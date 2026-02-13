#ifndef _TEST_PLUGIN_H
#define _TEST_PLUGIN_H

#include <stdio.h>
#include <string.h>

#define TESTPLUGIN_CHANNEL_JSON "flutter-drm-embedder/testjson"
#define TESTPLUGIN_CHANNEL_STD "flutter-drm-embedder/teststd"
#define TESTPLUGIN_CHANNEL_PING "flutter-drm-embedder/ping"

extern int testp_init(void);
extern int testp_deinit(void);

#endif
