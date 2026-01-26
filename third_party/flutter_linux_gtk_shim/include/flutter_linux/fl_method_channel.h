// SPDX-License-Identifier: MIT
#ifndef FL_METHOD_CHANNEL_H
#define FL_METHOD_CHANNEL_H

#include <gio/gio.h>
#include <glib-object.h>

#include "fl_binary_messenger.h"
#include "fl_method_call.h"
#include "fl_method_codec.h"
#include "fl_method_response.h"

G_BEGIN_DECLS

#define FL_TYPE_METHOD_CHANNEL (fl_method_channel_get_type())
G_DECLARE_FINAL_TYPE(FlMethodChannel, fl_method_channel, FL, METHOD_CHANNEL, GObject)

typedef void (*FlMethodChannelMethodCallHandler)(FlMethodChannel *channel, FlMethodCall *method_call, gpointer user_data);

FlMethodChannel *fl_method_channel_new(FlBinaryMessenger *messenger, const gchar *name, FlMethodCodec *codec);

void fl_method_channel_set_method_call_handler(FlMethodChannel *channel,
                                               FlMethodChannelMethodCallHandler handler,
                                               gpointer user_data,
                                               GDestroyNotify destroy_notify);

void fl_method_channel_invoke_method(FlMethodChannel *channel,
                                     const gchar *method,
                                     FlValue *args,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer user_data);

FlMethodResponse *fl_method_channel_invoke_method_finish(FlMethodChannel *channel, GAsyncResult *result, GError **error);

G_END_DECLS

#endif  // FL_METHOD_CHANNEL_H
