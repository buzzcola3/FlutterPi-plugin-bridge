// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_texture_registrar.h"

#include "fl_texture_registrar_internal.h"
#include "flutter-pi.h"
#include "texture_registry.h"

struct _FlTextureRegistrar {
    GObject parent_instance;
    struct flutterpi *flutterpi;
};

struct _FlTexture {
    GObject parent_instance;
    int64_t texture_id;
};

G_DEFINE_TYPE(FlTextureRegistrar, fl_texture_registrar, G_TYPE_OBJECT)
G_DEFINE_TYPE(FlTexture, fl_texture, G_TYPE_OBJECT)

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
    self->texture_id = -1;
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

    (void) registrar;
    (void) texture;
    return -1;
}

void fl_texture_registrar_unregister_texture(FlTextureRegistrar *registrar, FlTexture *texture) {
    g_return_if_fail(FL_IS_TEXTURE_REGISTRAR(registrar));
    g_return_if_fail(FL_IS_TEXTURE(texture));
    (void) registrar;
    (void) texture;
}

void fl_texture_registrar_mark_texture_frame_available(FlTextureRegistrar *registrar, FlTexture *texture) {
    g_return_if_fail(FL_IS_TEXTURE_REGISTRAR(registrar));
    g_return_if_fail(FL_IS_TEXTURE(texture));
    (void) registrar;
    (void) texture;
}
