// SPDX-License-Identifier: MIT
#ifndef FL_TEXTURE_REGISTRAR_H
#define FL_TEXTURE_REGISTRAR_H

#include <glib-object.h>
#include <stdint.h>

G_BEGIN_DECLS

#define FL_TYPE_TEXTURE_REGISTRAR (fl_texture_registrar_get_type())
G_DECLARE_FINAL_TYPE(FlTextureRegistrar, fl_texture_registrar, FL, TEXTURE_REGISTRAR, GObject)

#define FL_TYPE_TEXTURE (fl_texture_get_type())
G_DECLARE_DERIVABLE_TYPE(FlTexture, fl_texture, FL, TEXTURE, GObject)

struct _FlTextureClass {
    GObjectClass parent_class;
};

gboolean fl_texture_registrar_register_texture(FlTextureRegistrar *registrar, FlTexture *texture);
gboolean fl_texture_registrar_unregister_texture(FlTextureRegistrar *registrar, FlTexture *texture);
gboolean fl_texture_registrar_mark_texture_frame_available(FlTextureRegistrar *registrar, FlTexture *texture);

int64_t fl_texture_get_id(FlTexture *texture);

G_END_DECLS

#endif  // FL_TEXTURE_REGISTRAR_H
