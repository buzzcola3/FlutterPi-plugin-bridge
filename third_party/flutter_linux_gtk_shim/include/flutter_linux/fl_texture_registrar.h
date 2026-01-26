// SPDX-License-Identifier: MIT
#ifndef FL_TEXTURE_REGISTRAR_H
#define FL_TEXTURE_REGISTRAR_H

#include <glib-object.h>
#include <stdint.h>

G_BEGIN_DECLS

#define FL_TYPE_TEXTURE_REGISTRAR (fl_texture_registrar_get_type())
G_DECLARE_FINAL_TYPE(FlTextureRegistrar, fl_texture_registrar, FL, TEXTURE_REGISTRAR, GObject)

typedef struct _FlTexture FlTexture;

struct _FlTexture {
    GObject parent_instance;
};

#define FL_TYPE_TEXTURE (fl_texture_get_type())
G_DECLARE_FINAL_TYPE(FlTexture, fl_texture, FL, TEXTURE, GObject)

int64_t fl_texture_registrar_register_texture(FlTextureRegistrar *registrar, FlTexture *texture);
void fl_texture_registrar_unregister_texture(FlTextureRegistrar *registrar, FlTexture *texture);
void fl_texture_registrar_mark_texture_frame_available(FlTextureRegistrar *registrar, FlTexture *texture);

G_END_DECLS

#endif  // FL_TEXTURE_REGISTRAR_H
