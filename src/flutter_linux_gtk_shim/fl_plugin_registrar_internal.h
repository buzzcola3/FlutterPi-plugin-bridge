// SPDX-License-Identifier: MIT
#ifndef FL_PLUGIN_REGISTRAR_INTERNAL_H
#define FL_PLUGIN_REGISTRAR_INTERNAL_H

struct flutter_drm_embedder;

#include "flutter_linux/fl_plugin_registrar.h"

FlPluginRegistrar *fl_plugin_registrar_new_for_flutter_drm_embedder(struct flutter_drm_embedder *flutter_drm_embedder);

#endif  // FL_PLUGIN_REGISTRAR_INTERNAL_H
