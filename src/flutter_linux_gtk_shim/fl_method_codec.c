// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_method_codec.h"

G_DEFINE_TYPE(FlMethodCodec, fl_method_codec, G_TYPE_OBJECT)

static void fl_method_codec_class_init(FlMethodCodecClass *klass) {
    (void) klass;
}

static void fl_method_codec_init(FlMethodCodec *self) {
    (void) self;
}

GBytes *fl_method_codec_encode_method_call(FlMethodCodec *codec, const gchar *method, FlValue *args, GError **error) {
    g_return_val_if_fail(codec != NULL, NULL);
    FlMethodCodecClass *klass = FL_METHOD_CODEC_GET_CLASS(codec);
    g_return_val_if_fail(klass->encode_method_call != NULL, NULL);
    return klass->encode_method_call(codec, method, args, error);
}

gboolean fl_method_codec_decode_method_call(FlMethodCodec *codec, GBytes *message, gchar **method_out, FlValue **args_out,
                                            GError **error) {
    g_return_val_if_fail(codec != NULL, FALSE);
    FlMethodCodecClass *klass = FL_METHOD_CODEC_GET_CLASS(codec);
    g_return_val_if_fail(klass->decode_method_call != NULL, FALSE);
    return klass->decode_method_call(codec, message, method_out, args_out, error);
}

GBytes *fl_method_codec_encode_success_envelope(FlMethodCodec *codec, FlValue *result, GError **error) {
    g_return_val_if_fail(codec != NULL, NULL);
    FlMethodCodecClass *klass = FL_METHOD_CODEC_GET_CLASS(codec);
    g_return_val_if_fail(klass->encode_success_envelope != NULL, NULL);
    return klass->encode_success_envelope(codec, result, error);
}

GBytes *fl_method_codec_encode_error_envelope(FlMethodCodec *codec, const gchar *code, const gchar *message, FlValue *details,
                                              GError **error) {
    g_return_val_if_fail(codec != NULL, NULL);
    FlMethodCodecClass *klass = FL_METHOD_CODEC_GET_CLASS(codec);
    g_return_val_if_fail(klass->encode_error_envelope != NULL, NULL);
    return klass->encode_error_envelope(codec, code, message, details, error);
}

gboolean fl_method_codec_decode_response_envelope(FlMethodCodec *codec, GBytes *message, FlValue **result_out,
                                                  gchar **error_code_out, gchar **error_message_out, FlValue **error_details_out,
                                                  GError **error) {
    g_return_val_if_fail(codec != NULL, FALSE);
    FlMethodCodecClass *klass = FL_METHOD_CODEC_GET_CLASS(codec);
    g_return_val_if_fail(klass->decode_response_envelope != NULL, FALSE);
    return klass->decode_response_envelope(codec, message, result_out, error_code_out, error_message_out, error_details_out, error);
}
