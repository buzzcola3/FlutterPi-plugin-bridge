// SPDX-License-Identifier: MIT
#ifndef FL_METHOD_CALL_INTERNAL_H
#define FL_METHOD_CALL_INTERNAL_H

#include "flutter_linux/fl_method_call.h"
#include "flutter_linux/fl_binary_messenger.h"
#include "flutter_linux/fl_method_codec.h"

FlMethodCall *fl_method_call_new(FlBinaryMessenger *messenger,
                                 FlMethodCodec *codec,
                                 const gchar *name,
                                 FlValue *args,
                                 FlBinaryMessengerResponseHandle *response_handle);

#endif  // FL_METHOD_CALL_INTERNAL_H
