// SPDX-License-Identifier: MIT
#ifndef FLUTTERPI_SHIM_H
#define FLUTTERPI_SHIM_H

#include <stddef.h>
#include <stdint.h>

#include <flutter_embedder.h>

struct flutterpi;
struct texture;

struct plugin_registry;
struct texture_registry;

struct plugin_registry *flutterpi_get_plugin_registry(struct flutterpi *flutterpi);
int flutterpi_send_platform_message(struct flutterpi *flutterpi,
                                    const char *channel,
                                    const uint8_t *message,
                                    size_t message_size,
                                    FlutterPlatformMessageResponseHandle *responsehandle);
int flutterpi_respond_to_platform_message(const FlutterPlatformMessageResponseHandle *handle,
                                          const uint8_t *message,
                                          size_t message_size);

FlutterPlatformMessageResponseHandle *flutterpi_create_platform_message_response_handle(struct flutterpi *flutterpi,
                                                                                       FlutterDataCallback data_callback,
                                                                                       void *userdata);
void flutterpi_release_platform_message_response_handle(struct flutterpi *flutterpi,
                                                        FlutterPlatformMessageResponseHandle *handle);

struct texture *flutterpi_create_texture(struct flutterpi *flutterpi);

#endif  // FLUTTERPI_SHIM_H
