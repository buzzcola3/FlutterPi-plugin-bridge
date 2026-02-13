// SPDX-License-Identifier: MIT
#ifndef FL_PLUGIN_REGISTRY_H
#define FL_PLUGIN_REGISTRY_H

#include <glib-object.h>

#include "fl_plugin_registrar.h"

G_BEGIN_DECLS

// Flutter Linux uses FlPluginRegistry as the entry point for registering plugins.
// For flutter-drm-embedder's GTK shim, treat the registrar itself as the registry.

typedef FlPluginRegistrar FlPluginRegistry;

FlPluginRegistrar *fl_plugin_registry_get_registrar_for_plugin(FlPluginRegistry *registry,
                                                               const gchar *name);

G_END_DECLS

#endif  // FL_PLUGIN_REGISTRY_H
