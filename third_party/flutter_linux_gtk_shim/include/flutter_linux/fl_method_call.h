// SPDX-License-Identifier: MIT
#ifndef FL_METHOD_CALL_H
#define FL_METHOD_CALL_H

#include <glib-object.h>

#include "fl_method_codec.h"
#include "fl_method_response.h"
#include "fl_value.h"

G_BEGIN_DECLS

#define FL_TYPE_METHOD_CALL (fl_method_call_get_type())
G_DECLARE_FINAL_TYPE(FlMethodCall, fl_method_call, FL, METHOD_CALL, GObject)

const gchar *fl_method_call_get_name(FlMethodCall *method_call);
FlValue *fl_method_call_get_args(FlMethodCall *method_call);

gboolean fl_method_call_respond(FlMethodCall *method_call, FlMethodResponse *response, GError **error);

G_END_DECLS

#endif  // FL_METHOD_CALL_H
