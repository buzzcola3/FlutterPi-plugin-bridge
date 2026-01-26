// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_texture_registrar.h"

#include "fl_texture_registrar_internal.h"

#include <errno.h>
#include "flutterpi_shim.h"
#include "texture_registry.h"
#include "flutter_linux/fl_texture_gl.h"

#ifdef HAVE_EGL_GLES2
#include "gles.h"
#endif

struct _FlTextureRegistrar {
    GObject parent_instance;
    struct flutterpi *flutterpi;
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

FlTextureRegistrar *fl_texture_registrar_new_for_flutterpi(struct flutterpi *flutterpi) {
    FlTextureRegistrar *registrar = g_object_new(FL_TYPE_TEXTURE_REGISTRAR, NULL);
    registrar->flutterpi = flutterpi;
    return registrar;
}

// TODO: Implement texture handling for flutter-pi in Phase 4.
int64_t fl_texture_registrar_register_texture(FlTextureRegistrar *registrar, FlTexture *texture) {
    g_return_val_if_fail(FL_IS_TEXTURE_REGISTRAR(registrar), -1);
    g_return_val_if_fail(FL_IS_TEXTURE(texture), -1);

#ifndef HAVE_EGL_GLES2
    (void) registrar;
    (void) texture;
    return -1;
#else
    if (!FL_IS_TEXTURE_GL(texture)) {
        return -1;
    }

    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    if (priv->texture != NULL) {
        return priv->texture_id;
    }

    struct texture *native_texture = flutterpi_create_texture(registrar->flutterpi);
    if (native_texture == NULL) {
        return -1;
    }

    priv->texture = native_texture;
    priv->texture_id = texture_get_id(native_texture);
    return priv->texture_id;
#endif
}

void fl_texture_registrar_unregister_texture(FlTextureRegistrar *registrar, FlTexture *texture) {
    g_return_if_fail(FL_IS_TEXTURE_REGISTRAR(registrar));
    g_return_if_fail(FL_IS_TEXTURE(texture));

    (void) registrar;
    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    if (priv->texture) {
        texture_destroy(priv->texture);
        priv->texture = NULL;
        priv->texture_id = -1;
    }
}

void fl_texture_registrar_mark_texture_frame_available(FlTextureRegistrar *registrar, FlTexture *texture) {
    g_return_if_fail(FL_IS_TEXTURE_REGISTRAR(registrar));
    g_return_if_fail(FL_IS_TEXTURE(texture));
    (void) registrar;
#ifndef HAVE_EGL_GLES2
    (void) texture;
#else
    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    if (!priv->texture || !FL_IS_TEXTURE_GL(texture)) {
        return;
    }

    FlTextureGL *gl_texture = FL_TEXTURE_GL(texture);
    g_object_ref(gl_texture);

    struct unresolved_texture_frame frame = {
        .resolve = fl_texture_gl_resolve_frame,
        .destroy = fl_texture_gl_destroy_frame,
        .userdata = gl_texture,
    };

    texture_push_unresolved_frame(priv->texture, &frame);
#endif
}

int64_t fl_texture_get_id(FlTexture *texture) {
    g_return_val_if_fail(FL_IS_TEXTURE(texture), -1);
    FlTexturePrivate *priv = fl_texture_get_instance_private(texture);
    return priv->texture_id;
}
