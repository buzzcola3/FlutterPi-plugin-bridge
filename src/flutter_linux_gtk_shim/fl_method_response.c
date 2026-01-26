// SPDX-License-Identifier: MIT
#include "flutter_linux/fl_method_response.h"

#include <string.h>

struct _FlMethodResponse {
    GObject parent_instance;
    FlMethodResponseType type;
    FlValue *result;
    gchar *error_code;
    gchar *error_message;
    FlValue *error_details;
};

G_DEFINE_TYPE(FlMethodResponse, fl_method_response, G_TYPE_OBJECT)

static void fl_method_response_finalize(GObject *object) {
    FlMethodResponse *self = FL_METHOD_RESPONSE(object);
    if (self->result) {
        g_object_unref(self->result);
    }
    if (self->error_details) {
        g_object_unref(self->error_details);
    }
    g_free(self->error_code);
    g_free(self->error_message);

    G_OBJECT_CLASS(fl_method_response_parent_class)->finalize(object);
}

static void fl_method_response_class_init(FlMethodResponseClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = fl_method_response_finalize;
}

static void fl_method_response_init(FlMethodResponse *self) {
    self->type = FL_METHOD_RESPONSE_NOT_IMPLEMENTED;
}

FlMethodResponseType fl_method_response_get_type_id(FlMethodResponse *response) {
    g_return_val_if_fail(FL_IS_METHOD_RESPONSE(response), FL_METHOD_RESPONSE_NOT_IMPLEMENTED);
    return response->type;
}

FlMethodResponse *fl_method_success_response_new(FlValue *result) {
    FlMethodResponse *response = g_object_new(FL_TYPE_METHOD_RESPONSE, NULL);
    response->type = FL_METHOD_RESPONSE_SUCCESS;
    response->result = result ? g_object_ref(result) : NULL;
    return response;
}

FlMethodResponse *fl_method_error_response_new(const gchar *error_code, const gchar *error_message, FlValue *details) {
    FlMethodResponse *response = g_object_new(FL_TYPE_METHOD_RESPONSE, NULL);
    response->type = FL_METHOD_RESPONSE_ERROR;
    response->error_code = g_strdup(error_code ? error_code : "");
    response->error_message = g_strdup(error_message ? error_message : "");
    response->error_details = details ? g_object_ref(details) : NULL;
    return response;
}

FlMethodResponse *fl_method_not_implemented_response_new(void) {
    FlMethodResponse *response = g_object_new(FL_TYPE_METHOD_RESPONSE, NULL);
    response->type = FL_METHOD_RESPONSE_NOT_IMPLEMENTED;
    return response;
}

FlValue *fl_method_response_get_result(FlMethodResponse *response) {
    g_return_val_if_fail(FL_IS_METHOD_RESPONSE(response), NULL);
    return response->result;
}

const gchar *fl_method_response_get_error_code(FlMethodResponse *response) {
    g_return_val_if_fail(FL_IS_METHOD_RESPONSE(response), NULL);
    return response->error_code;
}

const gchar *fl_method_response_get_error_message(FlMethodResponse *response) {
    g_return_val_if_fail(FL_IS_METHOD_RESPONSE(response), NULL);
    return response->error_message;
}

FlValue *fl_method_response_get_error_details(FlMethodResponse *response) {
    g_return_val_if_fail(FL_IS_METHOD_RESPONSE(response), NULL);
    return response->error_details;
}
