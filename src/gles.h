// SPDX-License-Identifier: MIT
/*
 * Just a shim for including EGL headers, and disabling EGL function prototypes if EGL is not present
 *
 * Copyright (c) 2022, Hannes Winkler <hanneswinkler2000@web.de>
 */

#ifndef _FLUTTER_DRM_EMBEDDER_SRC_GLES_H
#define _FLUTTER_DRM_EMBEDDER_SRC_GLES_H

#include "config.h"

#if !defined(HAVE_GLES2)
    #error "gles.h was included but OpenGLES2 support is disabled."
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#endif  // _FLUTTER_DRM_EMBEDDER_SRC_GLES_H
