// SPDX-License-Identifier: MIT
#include <flutter_linux/flutter_linux.h>

#ifdef FLUTTER_PLUGIN_EXPORT
#undef FLUTTER_PLUGIN_EXPORT
#endif
#include "openautoflutter/openautoflutter_plugin.h"

#ifdef BUILD_GTK_SHIM_TEST_PLUGIN
#include "shim_test_plugin.h"
#endif

void fl_register_plugins(FlPluginRegistrar *registrar) {
    openautoflutter_plugin_register_with_registrar(registrar);
#ifdef BUILD_GTK_SHIM_TEST_PLUGIN
    gtk_shim_test_plugin_register(registrar);
#endif
}
