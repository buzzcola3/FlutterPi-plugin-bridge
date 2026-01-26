// SPDX-License-Identifier: MIT
#ifndef FL_TEXTURE_GL_H
#define FL_TEXTURE_GL_H

#include <gio/gio.h>
#include <glib-object.h>
#include <stdint.h>

#include "fl_texture_registrar.h"

G_BEGIN_DECLS

#define FL_TYPE_TEXTURE_GL (fl_texture_gl_get_type())
G_DECLARE_DERIVABLE_TYPE(FlTextureGL, fl_texture_gl, FL, TEXTURE_GL, FlTexture)

struct _FlTextureGLClass {
    FlTextureClass parent_class;

    gboolean (*populate)(FlTextureGL *texture, uint32_t *target, uint32_t *name, uint32_t *width, uint32_t *height, GError **error);
};

gboolean fl_texture_gl_populate(FlTextureGL *texture, uint32_t *target, uint32_t *name, uint32_t *width, uint32_t *height,
                                GError **error);

G_END_DECLS

#endif  // FL_TEXTURE_GL_H
