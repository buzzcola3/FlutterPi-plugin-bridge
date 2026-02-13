// SPDX-License-Identifier: MIT
#include "plugin_loader.h"

#include <dirent.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "flutter-drm-embedder.h"
#include "util/logging.h"

#include <flutter_linux/flutter_linux.h>

#include "flutter_linux_gtk_shim/fl_plugin_registrar_internal.h"

struct gtk_plugin_loader {
    void **handles;
    size_t handle_count;
    size_t handle_capacity;
};

static bool has_so_suffix(const char *name) {
    size_t len;

    if (name == NULL) {
        return false;
    }

    len = strlen(name);
    return len > 3 && strcmp(name + len - 3, ".so") == 0;
}

static bool is_regular_file(const char *path) {
    struct stat st;

    if (path == NULL) {
        return false;
    }

    if (stat(path, &st) != 0) {
        return false;
    }

    return S_ISREG(st.st_mode);
}

static char *join_path(const char *dir, const char *name) {
    size_t dir_len;
    size_t name_len;
    char *joined;

    if (dir == NULL || name == NULL) {
        return NULL;
    }

    dir_len = strlen(dir);
    name_len = strlen(name);
    joined = malloc(dir_len + 1 + name_len + 1);
    if (joined == NULL) {
        return NULL;
    }

    memcpy(joined, dir, dir_len);
    joined[dir_len] = '/';
    memcpy(joined + dir_len + 1, name, name_len);
    joined[dir_len + 1 + name_len] = '\0';
    return joined;
}

static char *build_symbol_name(const char *filename) {
    size_t len;
    size_t base_len;
    const char *base_start;
    char *base;
    char *symbol;

    if (!has_so_suffix(filename)) {
        return NULL;
    }

    len = strlen(filename);
    base_len = len - 3;
    base = malloc(base_len + 1);
    if (base == NULL) {
        return NULL;
    }

    memcpy(base, filename, base_len);
    base[base_len] = '\0';

    base_start = base;
    if (strncmp(base, "lib", 3) == 0) {
        base_start = base + 3;
    }

    symbol = malloc(strlen(base_start) + strlen("_register_with_registrar") + 1);
    if (symbol == NULL) {
        free(base);
        return NULL;
    }

    strcpy(symbol, base_start);
    strcat(symbol, "_register_with_registrar");
    free(base);
    return symbol;
}

static char *get_plugins_dir(struct flutter_drm_embedder *flutter_drm_embedder) {
    const char *bundle_path;

    if (flutter_drm_embedder == NULL) {
        return NULL;
    }

    bundle_path = flutter_drm_embedder_get_bundle_path(flutter_drm_embedder);
    if (bundle_path == NULL) {
        return NULL;
    }

    return join_path(bundle_path, "plugins");
}

static bool ensure_handle_capacity(struct gtk_plugin_loader *loader) {
    size_t new_capacity;
    void **new_handles;

    if (loader->handle_count < loader->handle_capacity) {
        return true;
    }

    new_capacity = loader->handle_capacity == 0 ? 4 : loader->handle_capacity * 2;
    new_handles = realloc(loader->handles, new_capacity * sizeof(void *));
    if (new_handles == NULL) {
        return false;
    }

    loader->handles = new_handles;
    loader->handle_capacity = new_capacity;
    return true;
}

struct gtk_plugin_loader *gtk_plugin_loader_load(struct flutter_drm_embedder *flutter_drm_embedder) {
    struct gtk_plugin_loader *loader;
    struct dirent *entry;
    size_t found_count = 0;
    DIR *dir;
    char *plugins_dir;

    if (flutter_drm_embedder == NULL) {
        return NULL;
    }

    plugins_dir = get_plugins_dir(flutter_drm_embedder);
    if (plugins_dir == NULL) {
        return NULL;
    }

    dir = opendir(plugins_dir);
    if (dir == NULL) {
        LOG_DEBUG("No plugins found in %s.\n", plugins_dir);
        free(plugins_dir);
        return NULL;
    }

    loader = calloc(1, sizeof(*loader));
    if (loader == NULL) {
        closedir(dir);
        free(plugins_dir);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        char *path;
        char *symbol;
        void *handle;

        if (entry->d_name[0] == '.') {
            continue;
        }

        if (!has_so_suffix(entry->d_name)) {
            continue;
        }

        path = join_path(plugins_dir, entry->d_name);
        if (path == NULL) {
            continue;
        }

        if (!is_regular_file(path)) {
            free(path);
            continue;
        }

        found_count++;

        symbol = build_symbol_name(entry->d_name);
        if (symbol == NULL) {
            free(path);
            continue;
        }

        handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        if (handle == NULL) {
            LOG_ERROR("Failed to load plugin %s: %s\n", path, dlerror());
            free(symbol);
            free(path);
            continue;
        }

        typedef void (*register_func_t)(FlPluginRegistrar *registrar);
        register_func_t register_func = (register_func_t) dlsym(handle, symbol);
        if (register_func == NULL) {
            LOG_ERROR("Failed to find symbol %s in %s: %s\n", symbol, path, dlerror());
            dlclose(handle);
            free(symbol);
            free(path);
            continue;
        }

        FlPluginRegistrar *registrar = fl_plugin_registrar_new_for_flutter_drm_embedder(flutter_drm_embedder);
        if (registrar == NULL) {
            LOG_ERROR("Failed to create GTK registrar for %s.\n", path);
            dlclose(handle);
            free(symbol);
            free(path);
            continue;
        }

        register_func(registrar);
        g_object_unref(registrar);

        if (!ensure_handle_capacity(loader)) {
            LOG_ERROR("Failed to grow plugin handle list for %s.\n", path);
            dlclose(handle);
            free(symbol);
            free(path);
            continue;
        }

        loader->handles[loader->handle_count++] = handle;
        free(symbol);
        free(path);
    }

    closedir(dir);

    if (found_count == 0) {
        LOG_DEBUG("No plugins found in %s.\n", plugins_dir);
    } else {
        LOG_DEBUG("Found %zu plugins in %s.\n", found_count, plugins_dir);
    }

    free(plugins_dir);

    if (loader->handle_count == 0) {
        gtk_plugin_loader_destroy(loader);
        return NULL;
    }

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
