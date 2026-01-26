// SPDX-License-Identifier: MIT
#ifndef FL_PLUGIN_REGISTRAR_INTERNAL_H
#define FL_PLUGIN_REGISTRAR_INTERNAL_H

struct flutterpi;

#include "flutter_linux/fl_plugin_registrar.h"

FlPluginRegistrar *fl_plugin_registrar_new_for_flutterpi(struct flutterpi *flutterpi);

#endif  // FL_PLUGIN_REGISTRAR_INTERNAL_H
