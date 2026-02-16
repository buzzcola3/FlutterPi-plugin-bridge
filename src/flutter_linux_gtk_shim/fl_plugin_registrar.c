// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_plugin_registrar.h"

#include "fl_binary_messenger_internal.h"
#include "fl_plugin_registrar_internal.h"
#include "fl_texture_registrar_internal.h"
#include "flutter_drm_embedder_shim.h"

struct _FlPluginRegistrar {
    GObject parent_instance;
    struct flutter_drm_embedder *flutter_drm_embedder;
    FlBinaryMessenger *messenger;
    FlTextureRegistrar *texture_registrar;
};

G_DEFINE_TYPE(FlPluginRegistrar, fl_plugin_registrar, G_TYPE_OBJECT)

static void fl_plugin_registrar_finalize(GObject *object) {
    FlPluginRegistrar *self = FL_PLUGIN_REGISTRAR(object);
    if (self->messenger) {
        g_object_unref(self->messenger);
    }
    if (self->texture_registrar) {
        g_object_unref(self->texture_registrar);
    }

    G_OBJECT_CLASS(fl_plugin_registrar_parent_class)->finalize(object);
}

static void fl_plugin_registrar_class_init(FlPluginRegistrarClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = fl_plugin_registrar_finalize;
}

static void fl_plugin_registrar_init(FlPluginRegistrar *self) {
    (void) self;
}

FlPluginRegistrar *fl_plugin_registrar_new_for_flutter_drm_embedder(struct flutter_drm_embedder *flutter_drm_embedder) {
    FlPluginRegistrar *registrar = g_object_new(FL_TYPE_PLUGIN_REGISTRAR, NULL);
    registrar->flutter_drm_embedder = flutter_drm_embedder;
    registrar->messenger = fl_binary_messenger_new_for_flutter_drm_embedder(flutter_drm_embedder);

    // Use a shared texture registrar that lives as long as the embedder,
    // matching real Flutter Linux API lifetime semantics.
    FlTextureRegistrar *shared = (FlTextureRegistrar *) flutter_drm_embedder_get_fl_texture_registrar(flutter_drm_embedder);
    if (shared == NULL) {
        shared = fl_texture_registrar_new_for_flutter_drm_embedder(flutter_drm_embedder);
        flutter_drm_embedder_set_fl_texture_registrar(flutter_drm_embedder, shared);
        g_message("[plugin_registrar] created NEW shared texture registrar %p", (void *)shared);
    } else {
        g_message("[plugin_registrar] reusing existing shared texture registrar %p", (void *)shared);
    }
    registrar->texture_registrar = g_object_ref(shared);
    g_message("[plugin_registrar] new plugin registrar %p (texture_registrar=%p, IS_TEXTURE_REGISTRAR=%d)",
             (void *)registrar, (void *)registrar->texture_registrar,
             FL_IS_TEXTURE_REGISTRAR(registrar->texture_registrar) ? 1 : 0);
    return registrar;
}

FlBinaryMessenger *fl_plugin_registrar_get_messenger(FlPluginRegistrar *registrar) {
    g_return_val_if_fail(FL_IS_PLUGIN_REGISTRAR(registrar), NULL);
    return registrar->messenger;
}

FlTextureRegistrar *fl_plugin_registrar_get_texture_registrar(FlPluginRegistrar *registrar) {
    g_return_val_if_fail(FL_IS_PLUGIN_REGISTRAR(registrar), NULL);
    g_message("[plugin_registrar] get_texture_registrar: returning %p (IS_TEXTURE_REGISTRAR=%d)",
             (void *)registrar->texture_registrar,
             registrar->texture_registrar ? FL_IS_TEXTURE_REGISTRAR(registrar->texture_registrar) : 0);
    return registrar->texture_registrar;
}
