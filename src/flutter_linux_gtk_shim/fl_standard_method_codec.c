// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_standard_method_codec.h"

#include <gio/gio.h>
#include <string.h>

#include "flutter-pi.h"
#include "platformchannel.h"

struct _FlStandardMethodCodec {
    FlMethodCodec parent_instance;
};

G_DEFINE_TYPE(FlStandardMethodCodec, fl_standard_method_codec, FL_TYPE_METHOD_CODEC)

static int fl_value_to_std_value(FlValue *value, struct std_value *out);
static FlValue *std_value_to_fl_value(const struct std_value *value);

static void platch_free_std_value(struct std_value *value) {
    struct platch_obj obj = {
        .codec = kStandardMessageCodec,
        .std_value = *value,
    };
    platch_free_obj(&obj);
}

static int fl_list_to_std_value(FlValue *value, struct std_value *out) {
    size_t length = fl_value_get_length(value);
    out->type = kStdList;
    out->size = length;
    out->list = length ? calloc(length, sizeof(struct std_value)) : NULL;
    if (length && out->list == NULL) {
        return ENOMEM;
    }
    for (size_t i = 0; i < length; i++) {
        FlValue *child = fl_value_get_list_value(value, i);
        int ok = fl_value_to_std_value(child, &out->list[i]);
        if (ok != 0) {
            return ok;
        }
    }
    return 0;
}

static int fl_map_to_std_value(FlValue *value, struct std_value *out) {
    size_t length = fl_value_get_length(value);
    out->type = kStdMap;
    out->size = length;
    out->keys = length ? calloc(length, sizeof(struct std_value)) : NULL;
    out->values = length ? calloc(length, sizeof(struct std_value)) : NULL;
    if (length && (out->keys == NULL || out->values == NULL)) {
        return ENOMEM;
    }
    for (size_t i = 0; i < length; i++) {
        FlValue *key = fl_value_get_map_key(value, i);
        FlValue *val = fl_value_get_map_value(value, i);
        int ok = fl_value_to_std_value(key, &out->keys[i]);
        if (ok != 0) {
            return ok;
        }
        ok = fl_value_to_std_value(val, &out->values[i]);
        if (ok != 0) {
            return ok;
        }
    }
    return 0;
}

static int fl_value_to_std_value(FlValue *value, struct std_value *out) {
    if (value == NULL) {
        out->type = kStdNull;
        return 0;
    }

    switch (fl_value_get_type_id(value)) {
        case FL_VALUE_TYPE_NULL:
            out->type = kStdNull;
            return 0;
        case FL_VALUE_TYPE_BOOL:
            out->type = fl_value_get_bool(value) ? kStdTrue : kStdFalse;
            return 0;
        case FL_VALUE_TYPE_INT:
            out->type = kStdInt64;
            out->int64_value = fl_value_get_int(value);
            return 0;
        case FL_VALUE_TYPE_FLOAT:
            out->type = kStdFloat64;
            out->float64_value = fl_value_get_float(value);
            return 0;
        case FL_VALUE_TYPE_STRING:
            out->type = kStdString;
            out->string_value = g_strdup(fl_value_get_string(value));
            return out->string_value ? 0 : ENOMEM;
        case FL_VALUE_TYPE_UINT8_LIST: {
            size_t length = 0;
            const uint8_t *data = fl_value_get_uint8_list(value, &length);
            out->type = kStdUInt8Array;
            out->size = length;
            out->uint8array = length ? g_memdup2(data, length) : NULL;
            return (length && out->uint8array == NULL) ? ENOMEM : 0;
        }
        case FL_VALUE_TYPE_INT32_LIST: {
            size_t length = 0;
            const int32_t *data = fl_value_get_int32_list(value, &length);
            out->type = kStdInt32Array;
            out->size = length;
            out->int32array = length ? g_memdup2(data, sizeof(int32_t) * length) : NULL;
            return (length && out->int32array == NULL) ? ENOMEM : 0;
        }
        case FL_VALUE_TYPE_INT64_LIST: {
            size_t length = 0;
            const int64_t *data = fl_value_get_int64_list(value, &length);
            out->type = kStdInt64Array;
            out->size = length;
            out->int64array = length ? g_memdup2(data, sizeof(int64_t) * length) : NULL;
            return (length && out->int64array == NULL) ? ENOMEM : 0;
        }
        case FL_VALUE_TYPE_FLOAT_LIST: {
            size_t length = 0;
            const double *data = fl_value_get_float_list(value, &length);
            out->type = kStdFloat64Array;
            out->size = length;
            out->float64array = length ? g_memdup2(data, sizeof(double) * length) : NULL;
            return (length && out->float64array == NULL) ? ENOMEM : 0;
        }
        case FL_VALUE_TYPE_LIST:
            return fl_list_to_std_value(value, out);
        case FL_VALUE_TYPE_MAP:
            return fl_map_to_std_value(value, out);
        default:
            out->type = kStdNull;
            return 0;
    }
}

static FlValue *std_value_to_fl_value(const struct std_value *value) {
    if (value == NULL) {
        return fl_value_new_null();
    }

    switch (value->type) {
        case kStdNull:
            return fl_value_new_null();
        case kStdTrue:
            return fl_value_new_bool(TRUE);
        case kStdFalse:
            return fl_value_new_bool(FALSE);
        case kStdInt32:
            return fl_value_new_int(value->int32_value);
        case kStdInt64:
            return fl_value_new_int(value->int64_value);
        case kStdFloat64:
            return fl_value_new_float(value->float64_value);
        case kStdString:
            return fl_value_new_string(value->string_value ? value->string_value : "");
        case kStdUInt8Array:
            return fl_value_new_uint8_list(value->uint8array, value->size);
        case kStdInt32Array:
            return fl_value_new_int32_list(value->int32array, value->size);
        case kStdInt64Array:
            return fl_value_new_int64_list(value->int64array, value->size);
        case kStdFloat64Array:
            return fl_value_new_float_list(value->float64array, value->size);
        case kStdList: {
            FlValue *list = fl_value_new_list();
            for (size_t i = 0; i < value->size; i++) {
                FlValue *child = std_value_to_fl_value(&value->list[i]);
                fl_value_append(list, child);
                fl_value_unref(child);
            }
            return list;
        }
        case kStdMap: {
            FlValue *map = fl_value_new_map();
            for (size_t i = 0; i < value->size; i++) {
                FlValue *key = std_value_to_fl_value(&value->keys[i]);
                FlValue *val = std_value_to_fl_value(&value->values[i]);
                fl_value_set(map, key, val);
                fl_value_unref(key);
                fl_value_unref(val);
            }
            return map;
        }
        default:
            return fl_value_new_null();
    }
}

static GBytes *fl_standard_method_codec_encode_method_call(FlMethodCodec *codec, const gchar *method, FlValue *args, GError **error) {
    (void) codec;
    struct std_value std_arg = {0};
    int ok = fl_value_to_std_value(args, &std_arg);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to encode method call arguments");
        return NULL;
    }

    struct platch_obj obj = PLATCH_OBJ_STD_CALL(method, std_arg);
    uint8_t *buffer = NULL;
    size_t size = 0;
    ok = platch_encode(&obj, &buffer, &size);
    platch_free_std_value(&std_arg);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to encode method call");
        return NULL;
    }

    GBytes *bytes = g_bytes_new_take(buffer, size);
    return bytes;
}

static gboolean fl_standard_method_codec_decode_method_call(FlMethodCodec *codec, GBytes *message, gchar **method_out,
                                                            FlValue **args_out, GError **error) {
    (void) codec;
    gsize size = 0;
    const uint8_t *data = g_bytes_get_data(message, &size);
    struct platch_obj obj = {0};
    int ok = platch_decode(data, size, kStandardMethodCall, &obj);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to decode method call");
        return FALSE;
    }

    if (method_out) {
        *method_out = g_strdup(obj.method ? obj.method : "");
    }
    if (args_out) {
        *args_out = std_value_to_fl_value(&obj.std_arg);
    }

    platch_free_obj(&obj);
    return TRUE;
}

static GBytes *fl_standard_method_codec_encode_success_envelope(FlMethodCodec *codec, FlValue *result, GError **error) {
    (void) codec;
    struct std_value std_result = {0};
    int ok = fl_value_to_std_value(result, &std_result);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to encode success envelope");
        return NULL;
    }

    struct platch_obj obj = PLATCH_OBJ_STD_CALL_SUCCESS_RESPONSE(std_result);
    uint8_t *buffer = NULL;
    size_t size = 0;
    ok = platch_encode(&obj, &buffer, &size);
    platch_free_std_value(&std_result);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to encode success envelope");
        return NULL;
    }

    return g_bytes_new_take(buffer, size);
}

static GBytes *fl_standard_method_codec_encode_error_envelope(FlMethodCodec *codec, const gchar *code, const gchar *message,
                                                              FlValue *details, GError **error) {
    (void) codec;
    struct std_value std_details = {0};
    int ok = fl_value_to_std_value(details, &std_details);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to encode error envelope");
        return NULL;
    }

    struct platch_obj obj = PLATCH_OBJ_STD_CALL_ERROR_RESPONSE(code ? code : "", message ? message : "", std_details);
    uint8_t *buffer = NULL;
    size_t size = 0;
    ok = platch_encode(&obj, &buffer, &size);
    platch_free_std_value(&std_details);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to encode error envelope");
        return NULL;
    }

    return g_bytes_new_take(buffer, size);
}

static gboolean fl_standard_method_codec_decode_response_envelope(FlMethodCodec *codec, GBytes *message, FlValue **result_out,
                                                                  gchar **error_code_out, gchar **error_message_out,
                                                                  FlValue **error_details_out, GError **error) {
    (void) codec;
    if (result_out) {
        *result_out = NULL;
    }
    if (error_code_out) {
        *error_code_out = NULL;
    }
    if (error_message_out) {
        *error_message_out = NULL;
    }
    if (error_details_out) {
        *error_details_out = NULL;
    }
    gsize size = 0;
    const uint8_t *data = g_bytes_get_data(message, &size);
    struct platch_obj obj = {0};
    int ok = platch_decode(data, size, kStandardMethodCallResponse, &obj);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to decode response envelope");
        return FALSE;
    }

    if (obj.success) {
        if (result_out) {
            *result_out = std_value_to_fl_value(&obj.std_result);
        }
    } else {
        if (error_code_out) {
            *error_code_out = g_strdup(obj.error_code ? obj.error_code : "");
        }
        if (error_message_out) {
            *error_message_out = g_strdup(obj.error_msg ? obj.error_msg : "");
        }
        if (error_details_out) {
            *error_details_out = std_value_to_fl_value(&obj.std_error_details);
        }
    }

    platch_free_obj(&obj);
    return TRUE;
}

static void fl_standard_method_codec_class_init(FlStandardMethodCodecClass *klass) {
    FlMethodCodecClass *codec_class = FL_METHOD_CODEC_CLASS(klass);
    codec_class->encode_method_call = fl_standard_method_codec_encode_method_call;
    codec_class->decode_method_call = fl_standard_method_codec_decode_method_call;
    codec_class->encode_success_envelope = fl_standard_method_codec_encode_success_envelope;
    codec_class->encode_error_envelope = fl_standard_method_codec_encode_error_envelope;
    codec_class->decode_response_envelope = fl_standard_method_codec_decode_response_envelope;
}

static void fl_standard_method_codec_init(FlStandardMethodCodec *self) {
    (void) self;
}

FlStandardMethodCodec *fl_standard_method_codec_new(void) {
    return g_object_new(FL_TYPE_STANDARD_METHOD_CODEC, NULL);
}
