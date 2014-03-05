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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>

#include "gstopenh264dec.h"

#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>

GST_DEBUG_CATEGORY_STATIC (gst_openh264dec_debug);
#define GST_CAT_DEFAULT gst_openh264dec_debug

#define MIN_WIDTH 4
#define MAX_WIDTH 16384
#define MIN_HEIGHT 4
#define MAX_HEIGHT 16384

enum
{
  PROP_0,
};

static void gst_openh264_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_openh264_dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_openh264_dec_start (GstVideoDecoder * decoder);
static gboolean gst_openh264_dec_stop (GstVideoDecoder * decoder);
static gboolean gst_openh264_dec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state);
static GstFlowReturn gst_openh264_dec_flush (GstVideoDecoder * decoder);
static GstFlowReturn gst_openh264_dec_handle_frame (GstVideoDecoder * decoder,
    GstVideoCodecFrame * frame);
static gboolean gst_openh264_dec_decide_allocation (GstVideoDecoder * decoder,
    GstQuery * query);

static GstStaticPadTemplate gst_openh264_dec_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264")
    );

static GstStaticPadTemplate gst_openh264_dec_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE("I420"))
    );

#define parent_class gst_openh264_dec_parent_class
G_DEFINE_TYPE (GstOpenH264Dec, gst_openh264_dec, GST_TYPE_VIDEO_DECODER);

static void
gst_openh264_dec_class_init (GstOpenH264DecClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;
  GstVideoDecoderClass *vdec_class;

  gobject_class = G_OBJECT_CLASS(klass);
  element_class = GST_ELEMENT_CLASS(klass);
  vdec_class = GST_VIDEO_DECODER_CLASS (klass);

  gobject_class->set_property = gst_openh264_dec_set_property;
  gobject_class->get_property = gst_openh264_dec_get_property;

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_openh264_dec_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_openh264_dec_sink_template));

  gst_element_class_set_static_metadata (element_class, 
      "Cisco OpenH264 Decoder",
      "Codec/Decoder/Video",
      "Decode H264 video streams",
      "cisco webex");

  vdec_class->start = GST_DEBUG_FUNCPTR (gst_openh264_dec_start);
  vdec_class->stop = GST_DEBUG_FUNCPTR (gst_openh264_dec_stop);
  vdec_class->flush = GST_DEBUG_FUNCPTR (gst_openh264_dec_flush);
  vdec_class->set_format = GST_DEBUG_FUNCPTR (gst_openh264_dec_set_format);
  vdec_class->handle_frame = GST_DEBUG_FUNCPTR (gst_openh264_dec_handle_frame);
  vdec_class->decide_allocation = gst_openh264_dec_decide_allocation;

  GST_DEBUG_CATEGORY_INIT (gst_openh264dec_debug, "openh264_dec", 0, "OpenH264 decoder");
}

static void
gst_openh264_close_dec(ISVCDecoder *decoder)
{
  if (decoder) {
    (*decoder)->Uninitialize(decoder);
    DestroyDecoder (decoder);
  }
}

static gboolean 
gst_openh264_open_dec(ISVCDecoder **decoder, const SDecodingParam* param)
{
  gboolean bret = FALSE;
  ISVCDecoder *svc_dec = NULL;
  int32_t color_fmt = videoFormatInternal;

  do {
    if(CreateDecoder (&svc_dec) || (svc_dec == NULL)) {
       break;
    }

    if((*svc_dec)->Initialize (svc_dec, param)) {
      gst_openh264_close_dec(svc_dec);
      break;
    }

    if((*svc_dec)->SetOption (svc_dec, DECODER_OPTION_DATAFORMAT, (void *)&color_fmt)) {
      gst_openh264_close_dec(svc_dec);
      break;
    }

    *decoder = svc_dec;
    bret = TRUE;
  }while(0);

  return bret;
}

static long
gst_openh264_decode(ISVCDecoder *decoder, const unsigned char *src, int src_len, void **dst, SBufferInfo* dst_info)
{
  long status = 1;
  if (decoder && src && dst && dst_info) {
    status = (*decoder)->DecodeFrame2 (decoder, src, src_len, dst, dst_info);
  }
  return status;
}

static void
gst_openh264_dec_init (GstOpenH264Dec * openh264_dec)
{
  GstVideoDecoder *decoder = (GstVideoDecoder *) openh264_dec;

  GST_DEBUG_OBJECT (openh264_dec, "gst_openh264_dec_init");
  gst_video_decoder_set_packetized (decoder, TRUE);
  openh264_dec->frame_size = 0;
  openh264_dec->use_threads = FALSE;
  openh264_dec->decoder_inited = FALSE;

  openh264_dec->svc_dec = NULL;
  memset(&openh264_dec->param, 0, sizeof(SDecodingParam));
  openh264_dec->param.iOutputColorFormat        = videoFormatI420;
  openh264_dec->param.uiTargetDqLayer           = (uint8_t) - 1;
  openh264_dec->param.uiEcActiveFlag            = 1;
  openh264_dec->param.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
}

static void
gst_openh264_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  //GstOpenH264Dec *dec;

  g_return_if_fail (GST_IS_OPENH264_DEC (object));
  //dec = GST_OPENH264_DEC (object);

  GST_DEBUG_OBJECT (object, "gst_openh264_dec_set_property");
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_openh264_dec_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  //GstOpenH264Dec *dec;

  g_return_if_fail (GST_IS_OPENH264_DEC (object));
  //dec = GST_OPENH264_DEC (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_openh264_dec_start (GstVideoDecoder * decoder)
{
  GstOpenH264Dec *openh264_dec = GST_OPENH264_DEC(decoder);

  GST_DEBUG_OBJECT (openh264_dec, "start");
  openh264_dec->decoder_inited = FALSE;

  return TRUE;
}

static gboolean
gst_openh264_dec_stop (GstVideoDecoder * decoder)
{
  GstOpenH264Dec *openh264_dec = GST_OPENH264_DEC(decoder);

  GST_DEBUG_OBJECT (openh264_dec, "stop");

  if (openh264_dec->output_state) {
    gst_video_codec_state_unref (openh264_dec->output_state);
    openh264_dec->output_state = NULL;
  }

  if (openh264_dec->input_state) {
    gst_video_codec_state_unref (openh264_dec->input_state);
    openh264_dec->input_state = NULL;
  }

  if (openh264_dec->decoder_inited){
    gst_openh264_close_dec(openh264_dec->svc_dec);
    openh264_dec->svc_dec = NULL;
  }
  openh264_dec->decoder_inited = FALSE;

  return TRUE;
}

static gboolean
gst_openh264_dec_set_format (GstVideoDecoder * decoder, GstVideoCodecState * state)
{
  GstOpenH264Dec *openh264_dec = GST_OPENH264_DEC(decoder);

  GST_DEBUG_OBJECT (openh264_dec, "set_format");

  if (openh264_dec->decoder_inited) {
    gst_openh264_close_dec(openh264_dec->svc_dec);
    openh264_dec->svc_dec = NULL;
  }
  openh264_dec->decoder_inited = FALSE;

  if (openh264_dec->input_state)
    gst_video_codec_state_unref (openh264_dec->input_state);
  openh264_dec->input_state = gst_video_codec_state_ref (state);

  return TRUE;
}

static gboolean
gst_openh264_dec_flush (GstVideoDecoder * decoder)
{
  GstOpenH264Dec *openh264_dec = GST_OPENH264_DEC (decoder);

  GST_DEBUG_OBJECT (decoder, "flush");

  if (openh264_dec->output_state) {
    gst_video_codec_state_unref (openh264_dec->output_state);
    openh264_dec->output_state = NULL;
  }

  if (openh264_dec->decoder_inited) {
    gst_openh264_close_dec(openh264_dec->svc_dec);
    openh264_dec->svc_dec = NULL;
  }
  openh264_dec->decoder_inited = FALSE;

  return TRUE;
}

static GstFlowReturn
gst_openh264_dec_handle_frame (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstOpenH264Dec *dec;
  GstFlowReturn ret = GST_FLOW_OK;
  //GstVideoFrame vframe;
  //long decoder_deadline = 0;
  GstClockTimeDiff deadline;
  GstMapInfo minfo;
  long status;

  void* pdata[3] = {NULL};
  SBufferInfo dst_info;

  GST_DEBUG_OBJECT (decoder, "handle_frame");

  dec = GST_OPENH264_DEC (decoder);

  if (!dec->decoder_inited) {
    if(!gst_openh264_open_dec(&dec->svc_dec, &dec->param)) {
      return GST_FLOW_ERROR;
    }
    dec->decoder_inited = TRUE;
  }

  deadline = gst_video_decoder_get_max_decode_time (decoder, frame);
#if 0
  if (deadline < 0) {
    decoder_deadline = 1;
  } else if (deadline == G_MAXINT64) {
    decoder_deadline = 0;
  } else {
    decoder_deadline = MAX (1, deadline / GST_MSECOND);
  }
#endif

  if (!gst_buffer_map (frame->input_buffer, &minfo, GST_MAP_READ)) {
    GST_ERROR_OBJECT (dec, "Failed to map input buffer");
    return GST_FLOW_ERROR;
  }

  pdata[0] = NULL; pdata[1] = NULL; pdata[2] = NULL;
  memset (&dst_info, 0, sizeof (SBufferInfo));
  status = gst_openh264_decode(dec->svc_dec, minfo.data, minfo.size, pdata, &dst_info);
  gst_buffer_unmap (frame->input_buffer, &minfo);

  if (status) {
    GST_VIDEO_DECODER_ERROR (decoder, 1, LIBRARY, ENCODE,
        ("Failed to decode frame"), ("%ld", (status)), ret);
    return ret;
  }

  if (dst_info.iBufferStatus == 1) {
#if 0
    int width, height;
    uint8_t* pdst[3] = {NULL};
    pdst[0] = (uint8_t*)pdata[0];
    pdst[1] = (uint8_t*)pdata[1];
    pdst[2] = (uint8_t*)pdata[2];
    int width  = dst_info.UsrData.sSystemBuffer.iWidth;
    int height = dst_info.UsrData.sSystemBuffer.iHeight;
#endif
  }

  // TODO: get decoded data from decoder
  if (0) {
    GST_VIDEO_CODEC_FRAME_SET_DECODE_ONLY (frame);
    gst_video_decoder_finish_frame (decoder, frame);
    return ret;
  }

  if (deadline < 0) {
    GST_LOG_OBJECT (dec, "Skipping late frame (%f s past deadline)",
        (double) -deadline / GST_SECOND);
    gst_video_decoder_drop_frame (decoder, frame);
  }else {
    ret = gst_video_decoder_allocate_output_frame (decoder, frame);
    if (ret == GST_FLOW_OK) {
      // TODO: copy decoded data into frame->output_buffer
      //gst_vp8_dec_image_to_buffer (dec, img, frame->output_buffer);
      ret = gst_video_decoder_finish_frame (decoder, frame);
    } else {
      gst_video_decoder_finish_frame (decoder, frame);
    }
  }

  return ret;
}

static gboolean
gst_openh264_dec_decide_allocation (GstVideoDecoder * bdec, GstQuery * query)
{
  GstBufferPool *pool = NULL;
  GstStructure *config;

  if (!GST_VIDEO_DECODER_CLASS (parent_class)->decide_allocation (bdec, query))
    return FALSE;

  g_assert (gst_query_get_n_allocation_pools (query) > 0);
  gst_query_parse_nth_allocation_pool (query, 0, &pool, NULL, NULL, NULL);
  g_assert (pool != NULL);

  config = gst_buffer_pool_get_config (pool);
  if (gst_query_find_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL)) {
    gst_buffer_pool_config_add_option (config,
        GST_BUFFER_POOL_OPTION_VIDEO_META);
  }
  gst_buffer_pool_set_config (pool, config);
  gst_object_unref (pool);

  return TRUE;
}

