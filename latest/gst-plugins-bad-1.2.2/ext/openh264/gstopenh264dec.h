/* GStreamer
 * Copyright (C) <2013> Sreerenj Balachandran <sreerenj.balachandran@intel.com>
 * Copyright (C) <2013> Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_OPENH264_DEC_H__
#define __GST_OPENH264_DEC_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideodecoder.h>
#include <gst/base/gstadapter.h>
#include <gst/base/gstbytereader.h>

#include <stdio.h>

#include <openh264/codec_api.h>

G_BEGIN_DECLS

#define GST_TYPE_OPENH264_DEC \
  (gst_openh264_dec_get_type())
#define GST_OPENH264_DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_OPENH264_DEC,GstOpenH264Dec))
#define GST_OPENH264_DEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_OPENH264_DEC,GstOpenH264DecClass))
#define GST_IS_OPENH264_DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_OPENH264_DEC))
#define GST_IS_OPENH264_DEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_OPENH264_DEC))

typedef struct _GstOpenH264Dec           GstOpenH264Dec;
typedef struct _GstOpenH264DecClass      GstOpenH264DecClass;

struct _GstOpenH264Dec {
  GstVideoDecoder decoder;

  GstVideoCodecState *input_state;
  GstVideoCodecState *output_state;

  ISVCDecoder* svc_dec;
  SDecodingParam param;
  gboolean decoder_inited;
  guint frame_size;
  gboolean use_threads;
};

struct _GstOpenH264DecClass {
  GstVideoDecoderClass decoder_class;
};

GType gst_openh264_dec_get_type (void);

#endif /* __GST_OPENH264_DEC_H__ */
