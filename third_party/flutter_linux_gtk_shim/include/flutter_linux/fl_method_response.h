// SPDX-License-Identifier: MIT
#ifndef FL_METHOD_RESPONSE_H
#define FL_METHOD_RESPONSE_H

#include <glib-object.h>

#include "fl_value.h"

G_BEGIN_DECLS

#define FL_TYPE_METHOD_RESPONSE (fl_method_response_get_type())
G_DECLARE_FINAL_TYPE(FlMethodResponse, fl_method_response, FL, METHOD_RESPONSE, GObject)

typedef enum {
    FL_METHOD_RESPONSE_SUCCESS,
    FL_METHOD_RESPONSE_ERROR,
    FL_METHOD_RESPONSE_NOT_IMPLEMENTED,
} FlMethodResponseType;

FlMethodResponseType fl_method_response_get_type_id(FlMethodResponse *response);

FlMethodResponse *fl_method_success_response_new(FlValue *result);
FlMethodResponse *fl_method_error_response_new(const gchar *error_code, const gchar *error_message, FlValue *details);
FlMethodResponse *fl_method_not_implemented_response_new(void);

FlValue *fl_method_response_get_result(FlMethodResponse *response);
const gchar *fl_method_response_get_error_code(FlMethodResponse *response);
const gchar *fl_method_response_get_error_message(FlMethodResponse *response);
FlValue *fl_method_response_get_error_details(FlMethodResponse *response);

G_END_DECLS

#endif  // FL_METHOD_RESPONSE_H
