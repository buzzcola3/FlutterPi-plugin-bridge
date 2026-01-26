// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_plugin_registry.h"

FlPluginRegistrar *fl_plugin_registry_get_registrar_for_plugin(FlPluginRegistry *registry,
                                                               const gchar *name) {
    (void) name;
    if (registry == NULL) {
        return NULL;
    }
    return g_object_ref(registry);
}
