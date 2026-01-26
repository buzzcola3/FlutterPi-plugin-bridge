// SPDX-License-Identifier: MIT
#ifndef FL_METHOD_CODEC_H
#define FL_METHOD_CODEC_H

#include <glib-object.h>

#include "fl_value.h"

G_BEGIN_DECLS

#define FL_TYPE_METHOD_CODEC (fl_method_codec_get_type())
G_DECLARE_DERIVABLE_TYPE(FlMethodCodec, fl_method_codec, FL, METHOD_CODEC, GObject)

struct _FlMethodCodecClass {
    GObjectClass parent_class;

    GBytes *(*encode_method_call)(FlMethodCodec *codec, const gchar *method, FlValue *args, GError **error);
    gboolean (*decode_method_call)(FlMethodCodec *codec, GBytes *message, gchar **method_out, FlValue **args_out, GError **error);
    GBytes *(*encode_success_envelope)(FlMethodCodec *codec, FlValue *result, GError **error);
    GBytes *(*encode_error_envelope)(FlMethodCodec *codec, const gchar *code, const gchar *message, FlValue *details, GError **error);
    gboolean (*decode_response_envelope)(FlMethodCodec *codec, GBytes *message, FlValue **result_out, gchar **error_code_out,
                                         gchar **error_message_out, FlValue **error_details_out, GError **error);
};

GBytes *fl_method_codec_encode_method_call(FlMethodCodec *codec, const gchar *method, FlValue *args, GError **error);

gboolean fl_method_codec_decode_method_call(FlMethodCodec *codec, GBytes *message, gchar **method_out, FlValue **args_out,
                                            GError **error);

GBytes *fl_method_codec_encode_success_envelope(FlMethodCodec *codec, FlValue *result, GError **error);

GBytes *fl_method_codec_encode_error_envelope(FlMethodCodec *codec, const gchar *code, const gchar *message, FlValue *details,
                                              GError **error);

gboolean fl_method_codec_decode_response_envelope(FlMethodCodec *codec, GBytes *message, FlValue **result_out,
                                                  gchar **error_code_out, gchar **error_message_out, FlValue **error_details_out,
                                                  GError **error);

G_END_DECLS

#endif  // FL_METHOD_CODEC_H
