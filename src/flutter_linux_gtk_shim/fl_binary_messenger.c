// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_binary_messenger.h"

#include <gio/gio.h>
#include <string.h>

#include "fl_binary_messenger_internal.h"
#include "flutter-pi.h"
#include "platformchannel.h"
#include "pluginregistry.h"

typedef struct {
    FlBinaryMessenger *messenger;
    gchar *channel;
    FlBinaryMessengerMessageHandler handler;
    gpointer user_data;
    GDestroyNotify destroy_notify;
} FlBinaryMessengerHandler;

typedef struct {
    struct flutterpi *flutterpi;
    FlutterPlatformMessageResponseHandle *handle;
    GTask *task;
} FlBinaryMessengerPendingResponse;

struct _FlBinaryMessengerResponseHandle {
    GObject parent_instance;
    FlutterPlatformMessageResponseHandle *handle;
};

typedef struct _FlBinaryMessengerResponseHandleClass {
    GObjectClass parent_class;
} FlBinaryMessengerResponseHandleClass;

G_DEFINE_TYPE(FlBinaryMessengerResponseHandle, fl_binary_messenger_response_handle, G_TYPE_OBJECT)

struct _FlBinaryMessenger {
    GObject parent_instance;
    struct flutterpi *flutterpi;
    struct plugin_registry *plugin_registry;
    GHashTable *handlers;
};

G_DEFINE_TYPE(FlBinaryMessenger, fl_binary_messenger, G_TYPE_OBJECT)

static void fl_binary_messenger_handler_free(gpointer data) {
    FlBinaryMessengerHandler *handler = data;
    if (!handler) {
        return;
    }
    if (handler->destroy_notify && handler->user_data) {
        handler->destroy_notify(handler->user_data);
    }
    g_free(handler->channel);
    g_free(handler);
}

static void fl_binary_messenger_finalize(GObject *object) {
    FlBinaryMessenger *self = FL_BINARY_MESSENGER(object);
    if (self->handlers) {
        g_hash_table_destroy(self->handlers);
    }
    G_OBJECT_CLASS(fl_binary_messenger_parent_class)->finalize(object);
}

static void fl_binary_messenger_class_init(FlBinaryMessengerClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = fl_binary_messenger_finalize;
}

static void fl_binary_messenger_init(FlBinaryMessenger *self) {
    self->handlers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, fl_binary_messenger_handler_free);
}

static void fl_binary_messenger_response_handle_class_init(FlBinaryMessengerResponseHandleClass *klass) {
    (void) klass;
}

static void fl_binary_messenger_response_handle_init(FlBinaryMessengerResponseHandle *self) {
    (void) self;
}

static FlBinaryMessengerResponseHandle *fl_binary_messenger_response_handle_new(FlutterPlatformMessageResponseHandle *handle) {
    FlBinaryMessengerResponseHandle *self = g_object_new(fl_binary_messenger_response_handle_get_type(), NULL);
    self->handle = handle;
    return self;
}

static void on_platform_message(void *userdata, const FlutterPlatformMessage *message) {
    FlBinaryMessengerHandler *handler = userdata;
    if (!handler || !handler->handler) {
        platch_respond_not_implemented((FlutterPlatformMessageResponseHandle *) message->response_handle);
        return;
    }

    GBytes *bytes = NULL;
    if (message->message && message->message_size > 0) {
        bytes = g_bytes_new(message->message, message->message_size);
    } else {
        bytes = g_bytes_new(NULL, 0);
    }

    FlBinaryMessengerResponseHandle *response_handle = fl_binary_messenger_response_handle_new(
        (FlutterPlatformMessageResponseHandle *) message->response_handle
    );

    handler->handler(handler->messenger, message->channel, bytes, response_handle, handler->user_data);

    g_bytes_unref(bytes);
    g_object_unref(response_handle);
}

FlBinaryMessenger *fl_binary_messenger_new_for_flutterpi(struct flutterpi *flutterpi) {
    FlBinaryMessenger *messenger = g_object_new(FL_TYPE_BINARY_MESSENGER, NULL);
    messenger->flutterpi = flutterpi;
    messenger->plugin_registry = flutterpi_get_plugin_registry(flutterpi);
    return messenger;
}

void fl_binary_messenger_set_message_handler_on_channel(FlBinaryMessenger *messenger,
                                                        const gchar *channel,
                                                        FlBinaryMessengerMessageHandler handler,
                                                        gpointer user_data,
                                                        GDestroyNotify destroy_notify) {
    g_return_if_fail(FL_IS_BINARY_MESSENGER(messenger));
    g_return_if_fail(channel != NULL);

    if (handler == NULL) {
        if (messenger->plugin_registry) {
            plugin_registry_remove_receiver_v2(messenger->plugin_registry, channel);
        }
        g_hash_table_remove(messenger->handlers, channel);
        return;
    }

    FlBinaryMessengerHandler *handler_data = g_new0(FlBinaryMessengerHandler, 1);
    handler_data->messenger = messenger;
    handler_data->channel = g_strdup(channel);
    handler_data->handler = handler;
    handler_data->user_data = user_data;
    handler_data->destroy_notify = destroy_notify;

    g_hash_table_replace(messenger->handlers, g_strdup(channel), handler_data);

    if (messenger->plugin_registry) {
        plugin_registry_set_receiver_v2(messenger->plugin_registry, channel, on_platform_message, handler_data);
    }
}

static void fl_binary_messenger_on_response(const uint8_t *data, size_t data_size, void *user_data) {
    FlBinaryMessengerPendingResponse *pending = user_data;
    GBytes *bytes = NULL;

    if (data && data_size > 0) {
        bytes = g_bytes_new(data, data_size);
    } else {
        bytes = g_bytes_new(NULL, 0);
    }

    g_task_return_pointer(pending->task, bytes, (GDestroyNotify) g_bytes_unref);

    if (pending->handle) {
        flutterpi_release_platform_message_response_handle(pending->flutterpi, pending->handle);
    }

    g_object_unref(pending->task);
    g_free(pending);
}

void fl_binary_messenger_send_on_channel(FlBinaryMessenger *messenger,
                                         const gchar *channel,
                                         GBytes *message,
                                         GCancellable *cancellable,
                                         GAsyncReadyCallback callback,
                                         gpointer user_data) {
    g_return_if_fail(FL_IS_BINARY_MESSENGER(messenger));
    g_return_if_fail(channel != NULL);

    GTask *task = g_task_new(messenger, cancellable, callback, user_data);

    FlBinaryMessengerPendingResponse *pending = g_new0(FlBinaryMessengerPendingResponse, 1);
    pending->flutterpi = messenger->flutterpi;
    pending->task = task;

    FlutterPlatformMessageResponseHandle *response_handle = flutterpi_create_platform_message_response_handle(
        messenger->flutterpi,
        fl_binary_messenger_on_response,
        pending
    );

    if (response_handle == NULL) {
        g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to create platform response handle");
        g_object_unref(task);
        g_free(pending);
        return;
    }

    pending->handle = response_handle;

    gsize size = 0;
    const uint8_t *data = NULL;
    if (message) {
        data = g_bytes_get_data(message, &size);
    }

    int ok = flutterpi_send_platform_message(messenger->flutterpi, channel, data, size, response_handle);
    if (ok != 0) {
        g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to send platform message");
        flutterpi_release_platform_message_response_handle(messenger->flutterpi, response_handle);
        g_object_unref(task);
        g_free(pending);
    }
}

GBytes *fl_binary_messenger_send_on_channel_finish(FlBinaryMessenger *messenger, GAsyncResult *result, GError **error) {
    g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), NULL);
    return g_task_propagate_pointer(G_TASK(result), error);
}

void fl_binary_messenger_send_response(FlBinaryMessenger *messenger,
                                       FlBinaryMessengerResponseHandle *response_handle,
                                       GBytes *response,
                                       GError **error) {
    g_return_if_fail(FL_IS_BINARY_MESSENGER(messenger));
    g_return_if_fail(response_handle != NULL);

    gsize size = 0;
    const uint8_t *data = NULL;
    if (response) {
        data = g_bytes_get_data(response, &size);
    }

    int ok = flutterpi_respond_to_platform_message(response_handle->handle, data, size);
    if (ok != 0) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to send platform response");
    }
}

void fl_binary_messenger_send_on_channel_no_response(FlBinaryMessenger *messenger, const gchar *channel, GBytes *message) {
    g_return_if_fail(FL_IS_BINARY_MESSENGER(messenger));
    g_return_if_fail(channel != NULL);

    gsize size = 0;
    const uint8_t *data = NULL;
    if (message) {
        data = g_bytes_get_data(message, &size);
    }

    flutterpi_send_platform_message(messenger->flutterpi, channel, data, size, NULL);
}
