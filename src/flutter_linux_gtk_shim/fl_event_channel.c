// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_event_channel.h"

#include <gio/gio.h>

#include "fl_binary_messenger_internal.h"
#include "fl_method_call_internal.h"
#include "flutter_linux/fl_standard_method_codec.h"

struct _FlEventChannel {
    GObject parent_instance;
    FlBinaryMessenger *messenger;
    gchar *name;
    FlMethodCodec *codec;
    FlEventChannelListenCallback on_listen;
    FlEventChannelCancelCallback on_cancel;
    gpointer user_data;
    GDestroyNotify destroy_notify;
    FlEventSinkData *sink_data;
};

G_DEFINE_TYPE(FlEventChannel, fl_event_channel, G_TYPE_OBJECT)

typedef struct {
    FlEventChannel *channel;
    FlEventSink sink;
    gpointer user_data;
} FlEventSinkData;

static gboolean fl_event_channel_send_event(FlValue *event, GError **error, gpointer user_data) {
    FlEventSinkData *data = user_data;
    if (!data || !data->channel) {
        return FALSE;
    }

    GBytes *encoded = fl_method_codec_encode_success_envelope(data->channel->codec, event, error);
    if (!encoded) {
        return FALSE;
    }

    fl_binary_messenger_send_on_channel_no_response(data->channel->messenger, data->channel->name, encoded);
    g_bytes_unref(encoded);
    return TRUE;
}

static void fl_event_channel_message_handler(FlBinaryMessenger *messenger,
                                             const gchar *channel,
                                             GBytes *message,
                                             FlBinaryMessengerResponseHandle *response_handle,
                                             gpointer user_data) {
    FlEventChannel *self = FL_EVENT_CHANNEL(user_data);
    (void) channel;
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

    gboolean ok = TRUE;
    if (g_strcmp0(method, "listen") == 0) {
        if (self->on_listen) {
            if (self->sink_data) {
                g_free(self->sink_data);
                self->sink_data = NULL;
            }
            self->sink_data = g_new0(FlEventSinkData, 1);
            self->sink_data->channel = self;
            ok = self->on_listen(self, args, fl_event_channel_send_event, self->sink_data, &error);
        }
    } else if (g_strcmp0(method, "cancel") == 0) {
        if (self->on_cancel) {
            ok = self->on_cancel(self, self->user_data, &error);
        }
        if (self->sink_data) {
            g_free(self->sink_data);
            self->sink_data = NULL;
        }
    }

    if (!ok && error) {
        GBytes *error_reply = fl_method_codec_encode_error_envelope(self->codec, "error", error->message, NULL, NULL);
        fl_binary_messenger_send_response(messenger, response_handle, error_reply, NULL);
        if (error_reply) {
            g_bytes_unref(error_reply);
        }
        g_error_free(error);
    } else {
        GBytes *success_reply = fl_method_codec_encode_success_envelope(self->codec, NULL, NULL);
        fl_binary_messenger_send_response(messenger, response_handle, success_reply, NULL);
        if (success_reply) {
            g_bytes_unref(success_reply);
        }
    }

    g_free(method);
    if (args) {
        g_object_unref(args);
    }
}

static void fl_event_channel_finalize(GObject *object) {
    FlEventChannel *self = FL_EVENT_CHANNEL(object);
    if (self->messenger) {
        g_object_unref(self->messenger);
    }
    if (self->codec) {
        g_object_unref(self->codec);
    }
    if (self->destroy_notify && self->user_data) {
        self->destroy_notify(self->user_data);
    }
    if (self->sink_data) {
        g_free(self->sink_data);
    }
    g_free(self->name);

    G_OBJECT_CLASS(fl_event_channel_parent_class)->finalize(object);
}

static void fl_event_channel_class_init(FlEventChannelClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = fl_event_channel_finalize;
}

static void fl_event_channel_init(FlEventChannel *self) {
    (void) self;
}

FlEventChannel *fl_event_channel_new(FlBinaryMessenger *messenger, const gchar *name, FlMethodCodec *codec) {
    g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), NULL);
    g_return_val_if_fail(name != NULL, NULL);

    FlEventChannel *channel = g_object_new(FL_TYPE_EVENT_CHANNEL, NULL);
    channel->messenger = g_object_ref(messenger);
    channel->name = g_strdup(name);
    channel->codec = codec ? g_object_ref(codec) : fl_standard_method_codec_new();

    fl_binary_messenger_set_message_handler_on_channel(channel->messenger, channel->name,
                                                       fl_event_channel_message_handler, channel, NULL);

    return channel;
}

void fl_event_channel_set_stream_handlers(FlEventChannel *channel,
                                          FlEventChannelListenCallback on_listen,
                                          FlEventChannelCancelCallback on_cancel,
                                          gpointer user_data,
                                          GDestroyNotify destroy_notify) {
    g_return_if_fail(FL_IS_EVENT_CHANNEL(channel));

    if (channel->destroy_notify && channel->user_data) {
        channel->destroy_notify(channel->user_data);
    }

    channel->on_listen = on_listen;
    channel->on_cancel = on_cancel;
    channel->user_data = user_data;
    channel->destroy_notify = destroy_notify;
}
