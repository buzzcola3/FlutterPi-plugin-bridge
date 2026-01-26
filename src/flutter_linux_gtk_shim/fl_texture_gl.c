// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_texture_gl.h"

#include <gio/gio.h>

G_DEFINE_TYPE(FlTextureGL, fl_texture_gl, FL_TYPE_TEXTURE)

static void fl_texture_gl_class_init(FlTextureGLClass *klass) {
    (void) klass;
}

static void fl_texture_gl_init(FlTextureGL *self) {
    (void) self;
}

gboolean fl_texture_gl_populate(FlTextureGL *texture, uint32_t *target, uint32_t *name, uint32_t *width, uint32_t *height,
                                GError **error) {
    g_return_val_if_fail(FL_IS_TEXTURE_GL(texture), FALSE);

    FlTextureGLClass *klass = FL_TEXTURE_GL_GET_CLASS(texture);
    if (klass->populate == NULL) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Texture populate not implemented");
        return FALSE;
    }

    return klass->populate(texture, target, name, width, height, error);
}
