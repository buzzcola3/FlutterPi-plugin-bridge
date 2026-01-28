// SPDX-License-Identifier: MIT
#include "plugin_loader.h"

#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/logging.h"

#include <flutter_linux/flutter_linux.h>

#include "flutter_linux_gtk_shim/fl_plugin_registrar_internal.h"

struct plugin_entry {
    char *path;
    char *symbol;
};

struct gtk_plugin_loader {
    void **handles;
    size_t handle_count;
};

static const char *skip_ws(const char *p) {
    while (p && *p && isspace((unsigned char) *p)) {
        p++;
    }
    return p;
}

static bool parse_json_string(const char **p_inout, char **out_str) {
    const char *p = skip_ws(*p_inout);
    if (*p != '"') {
        return false;
    }
    p++;

    char *buf = malloc(strlen(p) + 1);
    if (buf == NULL) {
        return false;
    }

    size_t out_len = 0;
    bool escape = false;
    for (; *p; p++) {
        if (escape) {
            switch (*p) {
                case '"': buf[out_len++] = '"'; break;
                case '\\': buf[out_len++] = '\\'; break;
                case '/': buf[out_len++] = '/'; break;
                case 'b': buf[out_len++] = '\b'; break;
                case 'f': buf[out_len++] = '\f'; break;
                case 'n': buf[out_len++] = '\n'; break;
                case 'r': buf[out_len++] = '\r'; break;
                case 't': buf[out_len++] = '\t'; break;
                default:
                    free(buf);
                    return false;
            }
            escape = false;
            continue;
        }

        if (*p == '\\') {
            escape = true;
            continue;
        }

        if (*p == '"') {
            buf[out_len] = '\0';
            *out_str = buf;
            *p_inout = p + 1;
            return true;
        }

        buf[out_len++] = *p;
    }

    free(buf);
    return false;
}

static const char *find_object_end(const char *p) {
    bool in_string = false;
    bool escape = false;
    int depth = 0;

    for (; *p; p++) {
        if (escape) {
            escape = false;
            continue;
        }
        if (*p == '\\') {
            escape = true;
            continue;
        }
        if (*p == '"') {
            in_string = !in_string;
            continue;
        }
        if (in_string) {
            continue;
        }
        if (*p == '{') {
            depth++;
        } else if (*p == '}') {
            depth--;
            if (depth == 0) {
                return p;
            }
        }
    }

    return NULL;
}

static bool extract_key_string(const char *obj_start, const char *obj_end, const char *key, char **out_value) {
    size_t key_len = strlen(key);
    const char *p = obj_start;

    while (p < obj_end) {
        const char *key_pos = strstr(p, key);
        if (key_pos == NULL || key_pos >= obj_end) {
            return false;
        }

        if (key_pos > obj_start && key_pos[-1] == '"' && key_pos[key_len] == '"') {
            const char *after_key = key_pos + key_len + 1;
            after_key = skip_ws(after_key);
            if (*after_key != ':') {
                p = key_pos + 1;
                continue;
            }
            after_key = skip_ws(after_key + 1);
            if (!parse_json_string(&after_key, out_value)) {
                return false;
            }
            return true;
        }

        p = key_pos + 1;
    }

    return false;
}

static char *resolve_plugin_path(const char *list_path, const char *entry_path) {
    if (entry_path == NULL) {
        return NULL;
    }
    if (entry_path[0] == '/') {
        return strdup(entry_path);
    }

    const char *last_slash = strrchr(list_path, '/');
    if (last_slash == NULL) {
        return strdup(entry_path);
    }

    size_t dir_len = (size_t)(last_slash - list_path);
    size_t entry_len = strlen(entry_path);
    char *joined = malloc(dir_len + 1 + entry_len + 1);
    if (joined == NULL) {
        return NULL;
    }

    memcpy(joined, list_path, dir_len);
    joined[dir_len] = '/';
    memcpy(joined + dir_len + 1, entry_path, entry_len);
    joined[dir_len + 1 + entry_len] = '\0';
    return joined;
}

static bool parse_plugin_list_json(const char *list_path, const char *json, struct plugin_entry **entries_out, size_t *count_out) {
    const char *p = json;
    size_t capacity = 4;
    size_t count = 0;
    struct plugin_entry *entries = calloc(capacity, sizeof(*entries));
    if (entries == NULL) {
        return false;
    }

    while (*p) {
        p = strchr(p, '{');
        if (p == NULL) {
            break;
        }
        const char *obj_end = find_object_end(p);
        if (obj_end == NULL) {
            break;
        }

        char *path = NULL;
        char *symbol = NULL;
        bool has_path = extract_key_string(p, obj_end, "path", &path);
        bool has_symbol = extract_key_string(p, obj_end, "symbol", &symbol);

        if (has_path) {
            char *resolved = resolve_plugin_path(list_path, path);
            free(path);
            path = resolved;
        }

        if (!has_path || path == NULL) {
            LOG_ERROR("Plugin list entry missing 'path'.\n");
            free(path);
            free(symbol);
            p = obj_end + 1;
            continue;
        }

        if (!has_symbol || symbol == NULL) {
            LOG_ERROR("Plugin list entry missing 'symbol' for %s.\n", path);
            free(path);
            free(symbol);
            p = obj_end + 1;
            continue;
        }

        if (count == capacity) {
            capacity *= 2;
            struct plugin_entry *new_entries = realloc(entries, capacity * sizeof(*entries));
            if (new_entries == NULL) {
                free(path);
                free(symbol);
                break;
            }
            entries = new_entries;
        }

        entries[count++] = (struct plugin_entry){ .path = path, .symbol = symbol };
        p = obj_end + 1;
    }

    if (count == 0) {
        free(entries);
        return false;
    }

    *entries_out = entries;
    *count_out = count;
    return true;
}

static char *read_file_to_string(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        LOG_ERROR("Could not open plugin list file '%s': %s\n", path, strerror(errno));
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long len = ftell(file);
    if (len < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file);
    char *buf = malloc((size_t) len + 1);
    if (buf == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(buf, 1, (size_t) len, file);
    fclose(file);
    buf[read] = '\0';
    return buf;
}

struct gtk_plugin_loader *gtk_plugin_loader_load(const char *plugin_list_path, struct flutterpi *flutterpi) {
    if (plugin_list_path == NULL || *plugin_list_path == '\0') {
        return NULL;
    }

    char *json = read_file_to_string(plugin_list_path);
    if (json == NULL) {
        return NULL;
    }

    struct plugin_entry *entries = NULL;
    size_t entry_count = 0;
    if (!parse_plugin_list_json(plugin_list_path, json, &entries, &entry_count)) {
        LOG_ERROR("No valid plugins found in %s.\n", plugin_list_path);
        free(json);
        return NULL;
    }

    struct gtk_plugin_loader *loader = calloc(1, sizeof(*loader));
    if (loader == NULL) {
        free(json);
        for (size_t i = 0; i < entry_count; i++) {
            free(entries[i].path);
            free(entries[i].symbol);
        }
        free(entries);
        return NULL;
    }

    loader->handles = calloc(entry_count, sizeof(void *));
    if (loader->handles == NULL) {
        free(loader);
        free(json);
        for (size_t i = 0; i < entry_count; i++) {
            free(entries[i].path);
            free(entries[i].symbol);
        }
        free(entries);
        return NULL;
    }

    for (size_t i = 0; i < entry_count; i++) {
        void *handle = dlopen(entries[i].path, RTLD_NOW | RTLD_LOCAL);
        if (handle == NULL) {
            LOG_ERROR("Failed to load plugin %s: %s\n", entries[i].path, dlerror());
            continue;
        }

        typedef void (*register_func_t)(FlPluginRegistrar *registrar);
        register_func_t register_func = (register_func_t) dlsym(handle, entries[i].symbol);
        if (register_func == NULL) {
            LOG_ERROR("Failed to find symbol %s in %s: %s\n", entries[i].symbol, entries[i].path, dlerror());
            dlclose(handle);
            continue;
        }

        FlPluginRegistrar *registrar = fl_plugin_registrar_new_for_flutterpi(flutterpi);
        if (registrar == NULL) {
            LOG_ERROR("Failed to create GTK registrar for %s.\n", entries[i].path);
            dlclose(handle);
            continue;
        }

        register_func(registrar);
        g_object_unref(registrar);

        loader->handles[loader->handle_count++] = handle;
    }

    for (size_t i = 0; i < entry_count; i++) {
        free(entries[i].path);
        free(entries[i].symbol);
    }
    free(entries);
    free(json);

    if (loader->handle_count == 0) {
        gtk_plugin_loader_destroy(loader);
        return NULL;
    }

    LOG_DEBUG("Loaded %zu GTK plugins from %s.\n", loader->handle_count, plugin_list_path);
    return loader;
}

void gtk_plugin_loader_destroy(struct gtk_plugin_loader *loader) {
    if (loader == NULL) {
        return;
    }

    for (size_t i = 0; i < loader->handle_count; i++) {
        if (loader->handles[i] != NULL) {
            dlclose(loader->handles[i]);
        }
    }

    free(loader->handles);
    free(loader);
}
