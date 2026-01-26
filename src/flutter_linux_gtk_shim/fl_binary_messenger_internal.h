// SPDX-License-Identifier: MIT
#ifndef FL_BINARY_MESSENGER_INTERNAL_H
#define FL_BINARY_MESSENGER_INTERNAL_H

struct flutterpi;

#include "flutter_linux/fl_binary_messenger.h"

FlBinaryMessenger *fl_binary_messenger_new_for_flutterpi(struct flutterpi *flutterpi);

void fl_binary_messenger_send_on_channel_no_response(FlBinaryMessenger *messenger, const gchar *channel, GBytes *message);

#endif  // FL_BINARY_MESSENGER_INTERNAL_H
