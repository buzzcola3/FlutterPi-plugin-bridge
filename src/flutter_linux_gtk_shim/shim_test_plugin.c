// SPDX-License-Identifier: MIT
#include "shim_test_plugin.h"

#include <string.h>

static void shim_test_handle_method_call(FlMethodChannel *channel, FlMethodCall *method_call, gpointer user_data) {
    (void) channel;
    (void) user_data;

    g_autoptr(FlMethodResponse) response = NULL;
    const gchar *method = fl_method_call_get_name(method_call);

    if (method != NULL && strcmp(method, "ping") == 0) {
        g_message("[gtk_shim_test] ping received");
        g_autoptr(FlValue) result = fl_value_new_string("pong");
        response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
    } else {
        response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
    }

    fl_method_call_respond(method_call, response, NULL);
}

void gtk_shim_test_plugin_register(FlPluginRegistrar *registrar) {
    g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
    g_autoptr(FlMethodChannel) channel = fl_method_channel_new(
        fl_plugin_registrar_get_messenger(registrar),
        "gtk_shim_test",
        FL_METHOD_CODEC(codec)
    );

    fl_method_channel_set_method_call_handler(channel, shim_test_handle_method_call, NULL, NULL);
}
