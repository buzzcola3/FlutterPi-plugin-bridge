// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_method_call.h"

#include "fl_method_call_internal.h"

struct _FlMethodCall {
    GObject parent_instance;
    FlBinaryMessenger *messenger;
    FlMethodCodec *codec;
    gchar *name;
    FlValue *args;
    FlBinaryMessengerResponseHandle *response_handle;
};

G_DEFINE_TYPE(FlMethodCall, fl_method_call, G_TYPE_OBJECT)

static void fl_method_call_finalize(GObject *object) {
    FlMethodCall *self = FL_METHOD_CALL(object);
    if (self->messenger) {
        g_object_unref(self->messenger);
    }
    if (self->codec) {
        g_object_unref(self->codec);
    }
    if (self->args) {
        g_object_unref(self->args);
    }
    if (self->response_handle) {
        g_object_unref(self->response_handle);
    }
    g_free(self->name);

    G_OBJECT_CLASS(fl_method_call_parent_class)->finalize(object);
}

static void fl_method_call_class_init(FlMethodCallClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = fl_method_call_finalize;
}

static void fl_method_call_init(FlMethodCall *self) {
    (void) self;
}

FlMethodCall *fl_method_call_new(FlBinaryMessenger *messenger,
                                 FlMethodCodec *codec,
                                 const gchar *name,
                                 FlValue *args,
                                 FlBinaryMessengerResponseHandle *response_handle) {
    FlMethodCall *call = g_object_new(FL_TYPE_METHOD_CALL, NULL);
    call->messenger = messenger ? g_object_ref(messenger) : NULL;
    call->codec = codec ? g_object_ref(codec) : NULL;
    call->name = g_strdup(name ? name : "");
    call->args = args ? g_object_ref(args) : NULL;
    call->response_handle = response_handle ? g_object_ref(response_handle) : NULL;
    return call;
}

const gchar *fl_method_call_get_name(FlMethodCall *method_call) {
    g_return_val_if_fail(method_call != NULL, NULL);
    return method_call->name;
}

FlValue *fl_method_call_get_args(FlMethodCall *method_call) {
    g_return_val_if_fail(method_call != NULL, NULL);
    return method_call->args;
}

gboolean fl_method_call_respond(FlMethodCall *method_call, FlMethodResponse *response, GError **error) {
    g_return_val_if_fail(method_call != NULL, FALSE);
    g_return_val_if_fail(response != NULL, FALSE);

    GBytes *encoded = NULL;
    FlMethodResponseType type = fl_method_response_get_type_id(response);

    if (type == FL_METHOD_RESPONSE_SUCCESS) {
        encoded = fl_method_codec_encode_success_envelope(method_call->codec, fl_method_response_get_result(response), error);
    } else if (type == FL_METHOD_RESPONSE_ERROR) {
        encoded = fl_method_codec_encode_error_envelope(
            method_call->codec,
            fl_method_response_get_error_code(response),
            fl_method_response_get_error_message(response),
            fl_method_response_get_error_details(response),
            error
        );
    } else {
        encoded = NULL;
    }

    if (type == FL_METHOD_RESPONSE_NOT_IMPLEMENTED) {
        fl_binary_messenger_send_response(method_call->messenger, method_call->response_handle, NULL, error);
        return TRUE;
    }

    if (encoded == NULL) {
        return FALSE;
    }

    fl_binary_messenger_send_response(method_call->messenger, method_call->response_handle, encoded, error);
    g_bytes_unref(encoded);
    return TRUE;
}
