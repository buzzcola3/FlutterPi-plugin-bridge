// SPDX-License-Identifier: MIT
#ifndef _FLUTTER_DRM_EMBEDDER_SRC_PLUGIN_LOADER_H
#define _FLUTTER_DRM_EMBEDDER_SRC_PLUGIN_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

struct flutter_drm_embedder;
struct gtk_plugin_loader;

struct gtk_plugin_loader *gtk_plugin_loader_load(struct flutter_drm_embedder *flutter_drm_embedder);
void gtk_plugin_loader_destroy(struct gtk_plugin_loader *loader);

#ifdef __cplusplus
}
#endif

#endif  // _FLUTTER_DRM_EMBEDDER_SRC_PLUGIN_LOADER_H
