// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_texture_gl.h"

#include <gio/gio.h>
#include <stdio.h>
#include <stdint.h>

/*
 * Private instance data: caches the populate function pointer directly on the
 * instance so fl_texture_gl_populate() never goes through GType vtable lookup.
 * GType class dispatch (FL_TEXTURE_GL_GET_CLASS / G_TYPE_INSTANCE_GET_CLASS)
 * is unreliable across dlopen boundaries on ARM64 — it can return a corrupted
 * function pointer, causing SIGBUS.
 */
typedef struct {
    gboolean (*populate)(FlTextureGL *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, GError **);
} FlTextureGLPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(FlTextureGL, fl_texture_gl, FL_TYPE_TEXTURE)

static void fl_texture_gl_constructed(GObject *object) {
    /* Chain up to parent constructed — fl_texture_gl_parent_class is set by
     * G_DEFINE_TYPE and is a plain pointer, no GType lookup needed. */
    GObjectClass *parent = (GObjectClass *)fl_texture_gl_parent_class;
    if (parent != NULL && parent->constructed != NULL)
        parent->constructed(object);

    /* Read the populate pointer directly from the instance's g_class.
     * This is a raw struct dereference — no G_TYPE_INSTANCE_GET_CLASS,
     * no g_type_check_instance_cast, no GType validation at all. */
    FlTextureGLClass *klass =
        (FlTextureGLClass *)(((GTypeInstance *)object)->g_class);
    FlTextureGLPrivate *priv =
        fl_texture_gl_get_instance_private((FlTextureGL *)object);

    if (klass != NULL) {
        priv->populate = klass->populate;
    }

    fprintf(stderr,
            "[fl_texture_gl] constructed: g_class=%p populate=%p priv=%p\n",
            (void *)klass,
            klass ? (void *)((uintptr_t)priv->populate) : NULL,
            (void *)priv);
}

static void fl_texture_gl_class_init(FlTextureGLClass *klass) {
    GObjectClass *gobject_class = (GObjectClass *)klass;
    gobject_class->constructed = fl_texture_gl_constructed;
}

static void fl_texture_gl_init(FlTextureGL *self) {
    (void) self;
}

gboolean fl_texture_gl_populate(FlTextureGL *texture, uint32_t *target, uint32_t *name, uint32_t *width, uint32_t *height,
                                GError **error) {
    if (texture == NULL) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Texture is NULL");
        return FALSE;
    }

    /* Primary path: use the instance-local populate pointer that was captured
     * during constructed — completely bypasses GType vtable dispatch. */
    FlTextureGLPrivate *priv = fl_texture_gl_get_instance_private(texture);
    if (priv != NULL && priv->populate != NULL) {
        return priv->populate(texture, target, name, width, height, error);
    }

    /* Fallback: direct g_class dereference (still no GType macro). */
    FlTextureGLClass *klass =
        (FlTextureGLClass *)(((GTypeInstance *)texture)->g_class);
    if (klass != NULL && klass->populate != NULL) {
        fprintf(stderr,
                "[fl_texture_gl] WARNING: using vtable fallback, populate=%p\n",
                (void *)((uintptr_t)klass->populate));
        return klass->populate(texture, target, name, width, height, error);
    }

    fprintf(stderr, "[fl_texture_gl] ERROR: no populate function available\n");
    g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Texture populate not implemented");
    return FALSE;
}
