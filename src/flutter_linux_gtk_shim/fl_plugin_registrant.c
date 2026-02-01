// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_plugin_registry.h"

#include "fl_plugin_registrar_internal.h"
#include "flutterpi_shim.h"
#include "plugin_loader.h"

__attribute__((weak)) void fl_register_plugins(FlPluginRegistry *registry);

void flutterpi_register_gtk_plugins(struct flutterpi *flutterpi) {
    struct gtk_plugin_loader *loader = gtk_plugin_loader_load(flutterpi);
    if (loader != NULL) {
        flutterpi_set_gtk_plugin_loader(flutterpi, loader);
    }

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
