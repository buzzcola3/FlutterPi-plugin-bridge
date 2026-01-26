// SPDX-License-Identifier: MIT
#ifndef FL_PLUGIN_REGISTRAR_H
#define FL_PLUGIN_REGISTRAR_H

#include <glib-object.h>

#include "fl_binary_messenger.h"
#include "fl_texture_registrar.h"

G_BEGIN_DECLS

#if defined(_WIN32)
    #define FLUTTER_PLUGIN_EXPORT __declspec(dllexport)
#else
    #define FLUTTER_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#define FL_TYPE_PLUGIN_REGISTRAR (fl_plugin_registrar_get_type())
G_DECLARE_FINAL_TYPE(FlPluginRegistrar, fl_plugin_registrar, FL, PLUGIN_REGISTRAR, GObject)

FlBinaryMessenger *fl_plugin_registrar_get_messenger(FlPluginRegistrar *registrar);
FlTextureRegistrar *fl_plugin_registrar_get_texture_registrar(FlPluginRegistrar *registrar);

G_END_DECLS

#endif  // FL_PLUGIN_REGISTRAR_H
