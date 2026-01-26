// SPDX-License-Identifier: MIT
#ifndef FL_TEXTURE_REGISTRAR_INTERNAL_H
#define FL_TEXTURE_REGISTRAR_INTERNAL_H

struct flutterpi;

#include "flutter_linux/fl_texture_registrar.h"

FlTextureRegistrar *fl_texture_registrar_new_for_flutterpi(struct flutterpi *flutterpi);

#endif  // FL_TEXTURE_REGISTRAR_INTERNAL_H
