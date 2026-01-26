// SPDX-License-Identifier: MIT
#include <flutter_linux/flutter_linux.h>

#ifdef FLUTTER_PLUGIN_EXPORT
#undef FLUTTER_PLUGIN_EXPORT
#endif
#include "openautoflutter/openautoflutter_plugin.h"

void fl_register_plugins(FlPluginRegistrar *registrar) {
    openautoflutter_plugin_register_with_registrar(registrar);
}
