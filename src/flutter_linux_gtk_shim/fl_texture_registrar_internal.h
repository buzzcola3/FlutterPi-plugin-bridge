// SPDX-License-Identifier: MIT
#ifndef FL_TEXTURE_REGISTRAR_INTERNAL_H
#define FL_TEXTURE_REGISTRAR_INTERNAL_H

struct flutter_drm_embedder;

#include "flutter_linux/fl_texture_registrar.h"

FlTextureRegistrar *fl_texture_registrar_new_for_flutter_drm_embedder(struct flutter_drm_embedder *flutter_drm_embedder);

#endif  // FL_TEXTURE_REGISTRAR_INTERNAL_H
