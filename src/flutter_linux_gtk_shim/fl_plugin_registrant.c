// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_plugin_registry.h"

#include "fl_plugin_registrar_internal.h"
#include "flutter_drm_embedder_shim.h"
#include "plugin_loader.h"

__attribute__((weak)) void fl_register_plugins(FlPluginRegistry *registry);

void flutter_drm_embedder_register_gtk_plugins(struct flutter_drm_embedder *flutter_drm_embedder) {
    g_message("[plugin_registrant] registering GTK plugins for embedder %p", (void *)flutter_drm_embedder);

    struct gtk_plugin_loader *loader = gtk_plugin_loader_load(flutter_drm_embedder);
    if (loader != NULL) {
        g_message("[plugin_registrant] plugin_loader loaded plugins from bundle dir");
        flutter_drm_embedder_set_gtk_plugin_loader(flutter_drm_embedder, loader);
    } else {
        g_message("[plugin_registrant] plugin_loader found no plugins in bundle dir");
    }

    if (fl_register_plugins == NULL) {
        g_message("[plugin_registrant] fl_register_plugins weak symbol is NULL, skipping static registration");
        return;
    }

    g_message("[plugin_registrant] calling fl_register_plugins (static registration)");
    FlPluginRegistrar *registrar = fl_plugin_registrar_new_for_flutter_drm_embedder(flutter_drm_embedder);
    if (registrar == NULL) {
        g_warning("[plugin_registrant] failed to create plugin registrar");
        return;
    }

    fl_register_plugins((FlPluginRegistry *) registrar);
    g_message("[plugin_registrant] static registration complete, unreffing registrar %p", (void *)registrar);
    g_object_unref(registrar);
}
