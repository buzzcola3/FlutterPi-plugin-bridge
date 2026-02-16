// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_texture_registrar.h"

#include "fl_texture_registrar_internal.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
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

    /* Zero the entire output struct so that frame_out->destroy and
     * frame_out->userdata are NULL.  Without this, counted_texture_frame_destroy
     * will jump to a garbage address when it checks frame->frame.destroy. */
    memset(frame_out, 0, sizeof(*frame_out));

    gboolean ok = fl_texture_gl_populate(texture_gl, &target, &name, &w, &h, &error);
    if (!ok) {
        if (error) {
            fprintf(stderr, "[fl_texture_gl] resolve_frame: populate failed: %s\n", error->message);
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
    fprintf(stderr, "[texture_registrar] created registrar %p (GType=%lu, embedder=%p)\n",
             (void *)registrar, (unsigned long)FL_TYPE_TEXTURE_REGISTRAR, (void *)flutter_drm_embedder);
    return registrar;
}

gboolean fl_texture_registrar_register_texture(FlTextureRegistrar *registrar, FlTexture *texture) {
    fprintf(stderr, "[texture_registrar] register_texture: registrar=%p texture=%p\n", (void *)registrar, (void *)texture);
    if (registrar == NULL || texture == NULL) {
        fprintf(stderr, "[texture_registrar] register_texture: NULL argument!\n");
        return FALSE;
    }

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
    fprintf(stderr, "[texture_registrar] registered texture: id=%" G_GINT64_FORMAT " native=%p\n", priv->texture_id, (void *)native_texture);
    return TRUE;
#endif
}

gboolean fl_texture_registrar_unregister_texture(FlTextureRegistrar *registrar, FlTexture *texture) {
    fprintf(stderr, "[texture_registrar] unregister_texture: registrar=%p texture=%p\n", (void *)registrar, (void *)texture);
    if (registrar == NULL || texture == NULL) {
        fprintf(stderr, "[texture_registrar] unregister_texture: NULL argument!\n");
        return FALSE;
    }

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
    if (registrar == NULL || texture == NULL) {
        fprintf(stderr, "[texture_registrar] mark_frame_available: NULL arg! registrar=%p texture=%p\n", (void *)registrar, (void *)texture);
        return FALSE;
    }
    (void) registrar;
#ifndef HAVE_EGL_GLES2
    (void) texture;
    return FALSE;
#else
    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    if (!priv->texture) {
        return FALSE;
    }

    /* Direct cast — avoids GType validation in FL_TEXTURE_GL() which is
     * unreliable across dlopen boundaries.  The FlTextureGL is always the
     * first member of any subclass (e.g. OAVideoTexture). */
    FlTextureGL *gl_texture = (FlTextureGL *)texture;
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
    if (texture == NULL) return -1;
    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    return priv->texture_id;
}
