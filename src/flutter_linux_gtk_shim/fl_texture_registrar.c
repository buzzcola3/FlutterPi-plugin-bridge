// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_texture_registrar.h"

#include "fl_texture_registrar_internal.h"

#include <errno.h>
#include "flutter_drm_embedder_shim.h"
#include "texture_registry.h"
#include "flutter_linux/fl_texture_gl.h"

#ifdef HAVE_EGL_GLES2
#include "gles.h"
#endif

struct _FlTextureRegistrar {
    GObject parent_instance;
    struct flutter_drm_embedder *flutter_drm_embedder;
};

typedef struct {
    struct texture *texture;
    int64_t texture_id;
} FlTexturePrivate;

#ifdef HAVE_EGL_GLES2
static int fl_texture_gl_resolve_frame(size_t width, size_t height, void *userdata, struct texture_frame *frame_out) {
    FlTextureGL *texture_gl = userdata;
    uint32_t target = 0;
    uint32_t name = 0;
    uint32_t w = 0;
    uint32_t h = 0;
    GError *error = NULL;
    gboolean ok = fl_texture_gl_populate(texture_gl, &target, &name, &w, &h, &error);
    if (!ok) {
        if (error) {
            g_error_free(error);
        }
        return EIO;
    }
    (void) width;
    (void) height;
    frame_out->gl.target = target;
    frame_out->gl.name = name;
#ifdef GL_RGBA8_OES
    frame_out->gl.format = GL_RGBA8_OES;
#else
    frame_out->gl.format = GL_RGBA;
#endif
    frame_out->gl.width = w;
    frame_out->gl.height = h;
    return 0;
}

static void fl_texture_gl_destroy_frame(void *userdata) {
    FlTextureGL *texture_gl = userdata;
    g_object_unref(texture_gl);
}
#endif

G_DEFINE_TYPE(FlTextureRegistrar, fl_texture_registrar, G_TYPE_OBJECT)
G_DEFINE_TYPE_WITH_PRIVATE(FlTexture, fl_texture, G_TYPE_OBJECT)

static void fl_texture_registrar_class_init(FlTextureRegistrarClass *klass) {
    (void) klass;
}

static void fl_texture_registrar_init(FlTextureRegistrar *self) {
    (void) self;
}

static void fl_texture_class_init(FlTextureClass *klass) {
    (void) klass;
}

static void fl_texture_init(FlTexture *self) {
    FlTexturePrivate *priv = fl_texture_get_instance_private(self);
    priv->texture_id = -1;
    priv->texture = NULL;
}

FlTextureRegistrar *fl_texture_registrar_new_for_flutter_drm_embedder(struct flutter_drm_embedder *flutter_drm_embedder) {
    FlTextureRegistrar *registrar = g_object_new(FL_TYPE_TEXTURE_REGISTRAR, NULL);
    registrar->flutter_drm_embedder = flutter_drm_embedder;
    g_message("[texture_registrar] created registrar %p (GType=%lu, embedder=%p)",
             (void *)registrar, (unsigned long)FL_TYPE_TEXTURE_REGISTRAR, (void *)flutter_drm_embedder);
    return registrar;
}

gboolean fl_texture_registrar_register_texture(FlTextureRegistrar *registrar, FlTexture *texture) {
    g_message("[texture_registrar] register_texture called: registrar=%p texture=%p", (void *)registrar, (void *)texture);
    if (registrar != NULL) {
        GType actual_type = G_TYPE_FROM_INSTANCE(registrar);
        g_message("[texture_registrar]   registrar GType: actual=%lu expected=%lu name='%s' IS_TEXTURE_REGISTRAR=%d",
                 (unsigned long)actual_type, (unsigned long)FL_TYPE_TEXTURE_REGISTRAR,
                 g_type_name(actual_type), FL_IS_TEXTURE_REGISTRAR(registrar) ? 1 : 0);
    }
    g_return_val_if_fail(FL_IS_TEXTURE_REGISTRAR(registrar), FALSE);
    g_return_val_if_fail(FL_IS_TEXTURE(texture), FALSE);

#ifndef HAVE_EGL_GLES2
    (void) registrar;
    (void) texture;
    return FALSE;
#else
    if (!FL_IS_TEXTURE_GL(texture)) {
        return FALSE;
    }

    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    if (priv->texture != NULL) {
        return TRUE;
    }

    struct texture *native_texture = flutter_drm_embedder_create_texture(registrar->flutter_drm_embedder);
    if (native_texture == NULL) {
        return FALSE;
    }

    priv->texture = native_texture;
    priv->texture_id = texture_get_id(native_texture);
    g_message("[texture_registrar] registered texture: id=%" G_GINT64_FORMAT " native=%p", priv->texture_id, (void *)native_texture);
    return TRUE;
#endif
}

gboolean fl_texture_registrar_unregister_texture(FlTextureRegistrar *registrar, FlTexture *texture) {
    g_return_val_if_fail(FL_IS_TEXTURE_REGISTRAR(registrar), FALSE);
    g_return_val_if_fail(FL_IS_TEXTURE(texture), FALSE);

    (void) registrar;
    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    if (priv->texture) {
        texture_destroy(priv->texture);
        priv->texture = NULL;
        priv->texture_id = -1;
    }
    return TRUE;
}

gboolean fl_texture_registrar_mark_texture_frame_available(FlTextureRegistrar *registrar, FlTexture *texture) {
    if (registrar != NULL && !FL_IS_TEXTURE_REGISTRAR(registrar)) {
        GType actual_type = G_TYPE_FROM_INSTANCE(registrar);
        g_critical("[texture_registrar] mark_frame_available: TYPE MISMATCH! registrar=%p actual_type=%lu('%s') expected_type=%lu('%s')",
                  (void *)registrar, (unsigned long)actual_type, g_type_name(actual_type),
                  (unsigned long)FL_TYPE_TEXTURE_REGISTRAR, g_type_name(FL_TYPE_TEXTURE_REGISTRAR));
    }
    g_return_val_if_fail(FL_IS_TEXTURE_REGISTRAR(registrar), FALSE);
    g_return_val_if_fail(FL_IS_TEXTURE(texture), FALSE);
    (void) registrar;
#ifndef HAVE_EGL_GLES2
    (void) texture;
    return FALSE;
#else
    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    if (!priv->texture || !FL_IS_TEXTURE_GL(texture)) {
        return FALSE;
    }

    FlTextureGL *gl_texture = FL_TEXTURE_GL(texture);
    g_object_ref(gl_texture);

    struct unresolved_texture_frame frame = {
        .resolve = fl_texture_gl_resolve_frame,
        .destroy = fl_texture_gl_destroy_frame,
        .userdata = gl_texture,
    };

    texture_push_unresolved_frame(priv->texture, &frame);
    return TRUE;
#endif
}

int64_t fl_texture_get_id(FlTexture *texture) {
    g_return_val_if_fail(FL_IS_TEXTURE(texture), -1);
    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    return priv->texture_id;
}
