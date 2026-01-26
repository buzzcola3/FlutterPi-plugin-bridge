// SPDX-License-Identifier: MIT
#ifndef FL_STANDARD_METHOD_CODEC_H
#define FL_STANDARD_METHOD_CODEC_H

#include "fl_method_codec.h"

G_BEGIN_DECLS

#define FL_TYPE_STANDARD_METHOD_CODEC (fl_standard_method_codec_get_type())
G_DECLARE_FINAL_TYPE(FlStandardMethodCodec, fl_standard_method_codec, FL, STANDARD_METHOD_CODEC, FlMethodCodec)

FlStandardMethodCodec *fl_standard_method_codec_new(void);

G_END_DECLS

#endif  // FL_STANDARD_METHOD_CODEC_H
