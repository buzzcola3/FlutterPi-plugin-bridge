// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_plugin_registry.h"

#include "fl_plugin_registrar_internal.h"
#include "flutterpi_shim.h"

__attribute__((weak)) void fl_register_plugins(FlPluginRegistry *registry);

void flutterpi_register_gtk_plugins(struct flutterpi *flutterpi) {
    if (fl_register_plugins == NULL) {
        return;
    }

    FlPluginRegistrar *registrar = fl_plugin_registrar_new_for_flutterpi(flutterpi);
    if (registrar == NULL) {
        return;
    }

    fl_register_plugins((FlPluginRegistry *) registrar);
    g_object_unref(registrar);
}
