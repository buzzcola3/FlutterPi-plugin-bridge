// SPDX-License-Identifier: MIT
#ifndef FL_BINARY_MESSENGER_H
#define FL_BINARY_MESSENGER_H

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define FL_TYPE_BINARY_MESSENGER (fl_binary_messenger_get_type())
G_DECLARE_FINAL_TYPE(FlBinaryMessenger, fl_binary_messenger, FL, BINARY_MESSENGER, GObject)

typedef struct _FlBinaryMessengerResponseHandle FlBinaryMessengerResponseHandle;

typedef void (*FlBinaryMessengerMessageHandler)(FlBinaryMessenger *messenger,
                                                const gchar *channel,
                                                GBytes *message,
                                                FlBinaryMessengerResponseHandle *response_handle,
                                                gpointer user_data);

void fl_binary_messenger_set_message_handler_on_channel(FlBinaryMessenger *messenger,
                                                        const gchar *channel,
                                                        FlBinaryMessengerMessageHandler handler,
                                                        gpointer user_data,
                                                        GDestroyNotify destroy_notify);

void fl_binary_messenger_send_on_channel(FlBinaryMessenger *messenger,
                                         const gchar *channel,
                                         GBytes *message,
                                         GCancellable *cancellable,
                                         GAsyncReadyCallback callback,
                                         gpointer user_data);

GBytes *fl_binary_messenger_send_on_channel_finish(FlBinaryMessenger *messenger,
                                                   GAsyncResult *result,
                                                   GError **error);

void fl_binary_messenger_send_response(FlBinaryMessenger *messenger,
                                       FlBinaryMessengerResponseHandle *response_handle,
                                       GBytes *response,
                                       GError **error);

G_END_DECLS

#endif  // FL_BINARY_MESSENGER_H
