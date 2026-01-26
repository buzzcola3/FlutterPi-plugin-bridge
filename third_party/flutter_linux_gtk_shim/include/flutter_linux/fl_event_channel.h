// SPDX-License-Identifier: MIT
#ifndef FL_EVENT_CHANNEL_H
#define FL_EVENT_CHANNEL_H

#include <glib-object.h>

#include "fl_binary_messenger.h"
#include "fl_method_codec.h"
#include "fl_value.h"

G_BEGIN_DECLS

#define FL_TYPE_EVENT_CHANNEL (fl_event_channel_get_type())
G_DECLARE_FINAL_TYPE(FlEventChannel, fl_event_channel, FL, EVENT_CHANNEL, GObject)

typedef gboolean (*FlEventSink)(FlValue *event, GError **error, gpointer user_data);

typedef gboolean (*FlEventChannelListenCallback)(FlEventChannel *channel,
                                                 FlValue *args,
                                                 FlEventSink sink,
                                                 gpointer user_data,
                                                 GError **error);

typedef gboolean (*FlEventChannelCancelCallback)(FlEventChannel *channel, gpointer user_data, GError **error);

FlEventChannel *fl_event_channel_new(FlBinaryMessenger *messenger, const gchar *name, FlMethodCodec *codec);

void fl_event_channel_set_stream_handlers(FlEventChannel *channel,
                                          FlEventChannelListenCallback on_listen,
                                          FlEventChannelCancelCallback on_cancel,
                                          gpointer user_data,
                                          GDestroyNotify destroy_notify);

G_END_DECLS

#endif  // FL_EVENT_CHANNEL_H
