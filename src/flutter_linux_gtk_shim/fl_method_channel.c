// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_method_channel.h"

#include <gio/gio.h>

#include "fl_method_call_internal.h"
#include "flutter_linux/fl_standard_method_codec.h"

struct _FlMethodChannel {
    GObject parent_instance;
    FlBinaryMessenger *messenger;
    gchar *name;
    FlMethodCodec *codec;
    FlMethodChannelMethodCallHandler handler;
    gpointer handler_user_data;
    GDestroyNotify handler_destroy_notify;
};

G_DEFINE_TYPE(FlMethodChannel, fl_method_channel, G_TYPE_OBJECT)

typedef struct {
    FlMethodChannel *channel;
    GTask *task;
} FlMethodChannelPendingCall;

static void fl_method_channel_finalize(GObject *object) {
    FlMethodChannel *self = FL_METHOD_CHANNEL(object);
    if (self->messenger) {
        g_object_unref(self->messenger);
    }
    if (self->codec) {
        g_object_unref(self->codec);
    }
    if (self->handler_destroy_notify && self->handler_user_data) {
        self->handler_destroy_notify(self->handler_user_data);
    }
    g_free(self->name);

    G_OBJECT_CLASS(fl_method_channel_parent_class)->finalize(object);
}

static void fl_method_channel_class_init(FlMethodChannelClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = fl_method_channel_finalize;
}

static void fl_method_channel_init(FlMethodChannel *self) {
    (void) self;
}

static void fl_method_channel_message_handler(FlBinaryMessenger *messenger,
                                              const gchar *channel,
                                              GBytes *message,
                                              FlBinaryMessengerResponseHandle *response_handle,
                                              gpointer user_data) {
    FlMethodChannel *self = FL_METHOD_CHANNEL(user_data);
    (void) channel;
    if (!self->handler) {
        fl_binary_messenger_send_response(messenger, response_handle, NULL, NULL);
        return;
    }

    gchar *method = NULL;
    FlValue *args = NULL;
    GError *error = NULL;

    if (!fl_method_codec_decode_method_call(self->codec, message, &method, &args, &error)) {
        if (error) {
            g_error_free(error);
        }
        fl_binary_messenger_send_response(messenger, response_handle, NULL, NULL);
        return;
    }

    FlMethodCall *call = fl_method_call_new(messenger, self->codec, method, args, response_handle);
    self->handler(self, call, self->handler_user_data);

    if (method) {
        g_free(method);
    }
    if (args) {
        g_object_unref(args);
    }
    g_object_unref(call);
}

FlMethodChannel *fl_method_channel_new(FlBinaryMessenger *messenger, const gchar *name, FlMethodCodec *codec) {
    g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), NULL);
    g_return_val_if_fail(name != NULL, NULL);

    FlMethodChannel *channel = g_object_new(FL_TYPE_METHOD_CHANNEL, NULL);
    channel->messenger = g_object_ref(messenger);
    channel->name = g_strdup(name);
    channel->codec = codec ? g_object_ref(codec) : FL_METHOD_CODEC(fl_standard_method_codec_new());
    return channel;
}

void fl_method_channel_set_method_call_handler(FlMethodChannel *channel,
                                               FlMethodChannelMethodCallHandler handler,
                                               gpointer user_data,
                                               GDestroyNotify destroy_notify) {
    g_return_if_fail(FL_IS_METHOD_CHANNEL(channel));

    if (channel->handler_destroy_notify && channel->handler_user_data) {
        channel->handler_destroy_notify(channel->handler_user_data);
    }

    channel->handler = handler;
    channel->handler_user_data = user_data;
    channel->handler_destroy_notify = destroy_notify;

    fl_binary_messenger_set_message_handler_on_channel(channel->messenger, channel->name,
                                                       handler ? fl_method_channel_message_handler : NULL,
                                                       handler ? g_object_ref(channel) : NULL,
                                                       handler ? g_object_unref : NULL);
}

static void fl_method_channel_on_invoke_response(GObject *source_object, GAsyncResult *result, gpointer user_data) {
    FlMethodChannelPendingCall *pending = user_data;
    GError *error = NULL;

    GBytes *response = fl_binary_messenger_send_on_channel_finish(FL_BINARY_MESSENGER(source_object), result, &error);
    if (error != NULL) {
        g_task_return_error(pending->task, error);
        g_object_unref(pending->task);
        g_free(pending);
        return;
    }

    FlValue *result_value = NULL;
    gchar *error_code = NULL;
    gchar *error_message = NULL;
    FlValue *error_details = NULL;

    gboolean ok = fl_method_codec_decode_response_envelope(pending->channel->codec, response,
                                                           &result_value, &error_code, &error_message, &error_details, &error);
    if (!ok) {
        if (error) {
            g_task_return_error(pending->task, error);
        } else {
            g_task_return_new_error(pending->task, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to decode method response");
        }
        g_object_unref(pending->task);
        g_bytes_unref(response);
        g_free(pending);
        return;
    }

    FlMethodResponse *method_response = NULL;
    if (error_code || error_message) {
        method_response = fl_method_error_response_new(error_code, error_message, error_details);
    } else {
        method_response = fl_method_success_response_new(result_value);
    }

    if (result_value) {
        g_object_unref(result_value);
    }
    if (error_details) {
        g_object_unref(error_details);
    }
    g_free(error_code);
    g_free(error_message);
    g_bytes_unref(response);

    g_task_return_pointer(pending->task, method_response, g_object_unref);
    g_object_unref(pending->task);
    g_free(pending);
}

void fl_method_channel_invoke_method(FlMethodChannel *channel,
                                     const gchar *method,
                                     FlValue *args,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer user_data) {
    g_return_if_fail(FL_IS_METHOD_CHANNEL(channel));

    GError *error = NULL;
    GBytes *message = fl_method_codec_encode_method_call(channel->codec, method, args, &error);
    if (!message) {
        if (error) {
            g_task_report_error(channel, callback, user_data, NULL, error);
        } else {
            g_task_report_new_error(channel, callback, user_data, NULL, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to encode method call");
        }
        return;
    }

    GTask *task = g_task_new(channel, cancellable, callback, user_data);

    FlMethodChannelPendingCall *pending = g_new0(FlMethodChannelPendingCall, 1);
    pending->channel = channel;
    pending->task = task;

    fl_binary_messenger_send_on_channel(channel->messenger, channel->name, message, cancellable,
                                        fl_method_channel_on_invoke_response, pending);

    g_bytes_unref(message);
}

FlMethodResponse *fl_method_channel_invoke_method_finish(FlMethodChannel *channel, GAsyncResult *result, GError **error) {
    g_return_val_if_fail(FL_IS_METHOD_CHANNEL(channel), NULL);
    return g_task_propagate_pointer(G_TASK(result), error);
}
