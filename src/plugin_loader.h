// SPDX-License-Identifier: MIT
#ifndef _FLUTTERPI_SRC_PLUGIN_LOADER_H
#define _FLUTTERPI_SRC_PLUGIN_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

struct flutterpi;
struct gtk_plugin_loader;

struct gtk_plugin_loader *gtk_plugin_loader_load(const char *plugin_list_path, struct flutterpi *flutterpi);
void gtk_plugin_loader_destroy(struct gtk_plugin_loader *loader);

#ifdef __cplusplus
}
#endif

#endif  // _FLUTTERPI_SRC_PLUGIN_LOADER_H
