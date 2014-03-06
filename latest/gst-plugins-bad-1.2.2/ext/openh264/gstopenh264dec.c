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
#include <dlfcn.h>

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
close_decoder(GstOpenH264Dec *decoder)
{
  if (!decoder) return;

  if (decoder->svc_dec) {
    (*decoder->svc_dec)->Uninitialize(decoder->svc_dec);

#ifdef DL_LOAD_OPENH264
    void (* pfDestroyDecoder) (ISVCDecoder* ) = NULL;
    if (decoder->svc_handle)
        pfDestroyDecoder = dlsym(decoder->svc_handle, "DestroyDecoder");
    if (pfDestroyDecoder)
        pfDestroyDecoder (decoder->svc_dec);
#else
    DestroyDecoder (decoder->svc_dec);
#endif
    decoder->svc_dec = NULL;
  }

#ifdef DL_LOAD_OPENH264
  if (decoder->svc_handle) {
    dlclose(decoder->svc_handle);
    decoder->svc_handle = NULL;
  }
#endif
}

static gboolean 
open_decoder(GstOpenH264Dec *decoder)
{
  gboolean bret = FALSE;
  int32_t color_fmt = videoFormatInternal;

  if (!decoder) return FALSE;

  do {
#ifdef DL_LOAD_OPENH264
    long (* pfCreateDecoder) (ISVCDecoder** ) = NULL;
    if (!decoder->svc_handle) {
      decoder->svc_handle = dlopen("libwels.so", RTLD_LAZY);
    }

    pfCreateDecoder = dlsym(decoder->svc_handle, "CreateDecoder");
    if(!pfCreateDecoder || pfCreateDecoder (&decoder->svc_dec) || (decoder->svc_dec == NULL)) {
      close_decoder(decoder);
      break;
    }
#else
    if(CreateDecoder (&decoder->svc_dec) || (decoder->svc_dec == NULL)) {
      close_decoder(decoder);
      break;
    }
#endif

    if((*decoder->svc_dec)->Initialize (decoder->svc_dec, &decoder->param)) {
      close_decoder(decoder);
      break;
    }

    if((*decoder->svc_dec)->SetOption (decoder->svc_dec, DECODER_OPTION_DATAFORMAT, (void *)&color_fmt)) {
      close_decoder(decoder);
      break;
    }

    bret = TRUE;
  }while(0);

  return bret;
}

static long
codec_decode(GstOpenH264Dec *decoder, const unsigned char *src, int src_len, void **dst, SBufferInfo* dst_info)
{
  long status = -1;
  if (decoder && decoder->svc_dec && src && dst && dst_info) {
    status = (*decoder->svc_dec)->DecodeFrame2 (decoder->svc_dec, src, src_len, dst, dst_info);
  }
  return status;
}

static void
codec_dec_send_tags (GstOpenH264Dec * dec)
{
  GstTagList *list;

  list = gst_tag_list_new_empty ();
  gst_tag_list_add (list, GST_TAG_MERGE_REPLACE,
      GST_TAG_VIDEO_CODEC, "OpenH264 video", NULL);

  gst_pad_push_event (GST_VIDEO_DECODER_SRC_PAD (dec),
      gst_event_new_tag (list));
}


static void
codec_dec_image_to_buffer (GstOpenH264Dec * dec, const SBufferInfo* img_info, void** img_data,
    GstBuffer * buffer)
{
  int deststride, srcstride, height, width, line, comp;
  guint8 *dest, *src;
  GstVideoFrame frame;
  GstVideoInfo *info = &dec->output_state->info;

  if (!gst_video_frame_map (&frame, info, buffer, GST_MAP_WRITE)) {
    GST_ERROR_OBJECT (dec, "Could not map video buffer");
  }

  for (comp = 0; comp < 3; comp++) {
    dest = GST_VIDEO_FRAME_COMP_DATA (&frame, comp);
    src = img_data[comp];
    width = GST_VIDEO_FRAME_COMP_WIDTH (&frame, comp);
    height = GST_VIDEO_FRAME_COMP_HEIGHT (&frame, comp);
    deststride = GST_VIDEO_FRAME_COMP_STRIDE (&frame, comp);
    srcstride = img_info->UsrData.sSystemBuffer.iStride[(comp==0?0:1)];

    for (line = 0; line < height; line++) {
      memcpy (dest, src, width);
      dest += deststride;
      src += srcstride;
    }
  }

  gst_video_frame_unmap (&frame);
}

static void
gst_openh264_dec_init (GstOpenH264Dec * openh264_dec)
{
  GstVideoDecoder *decoder = (GstVideoDecoder *) openh264_dec;

  GST_DEBUG_OBJECT (openh264_dec, "gst_openh264_dec_init");
  gst_video_decoder_set_packetized (decoder, TRUE);

  openh264_dec->input_state = NULL;
  openh264_dec->output_state = NULL;

  openh264_dec->svc_handle = NULL;
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

  close_decoder(openh264_dec);

  return TRUE;
}

static gboolean
gst_openh264_dec_set_format (GstVideoDecoder * decoder, GstVideoCodecState * state)
{
  GstOpenH264Dec *openh264_dec = GST_OPENH264_DEC(decoder);

  GST_DEBUG_OBJECT (openh264_dec, "set_format");

  close_decoder(openh264_dec);

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

  close_decoder(openh264_dec);

  return TRUE;
}

static GstFlowReturn
gst_openh264_dec_handle_frame (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstOpenH264Dec *dec;
  GstFlowReturn ret = GST_FLOW_OK;
  //long decoder_deadline = 0;
  GstClockTimeDiff deadline;
  GstMapInfo minfo;
  long status;

  void* pdata[3] = {NULL};
  SBufferInfo dst_info;

  GST_DEBUG_OBJECT (decoder, "handle_frame");

  dec = GST_OPENH264_DEC (decoder);

  if (!dec->svc_dec) {
    if(!open_decoder(dec)) {
      GST_ERROR_OBJECT (decoder, "Failed to open_decoder");
      return GST_FLOW_ERROR;
    }
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
  status = codec_decode(dec, minfo.data, minfo.size, pdata, &dst_info);
  gst_buffer_unmap (frame->input_buffer, &minfo);

  if (status != dsErrorFree) {
    GST_VIDEO_DECODER_ERROR (decoder, 1, LIBRARY, ENCODE,
        ("Failed to decode frame"), ("%ld", (status)), ret);
    return ret;
  }

  if (dec->output_state == NULL) {
    GstVideoCodecState *state = dec->input_state;
    g_assert(dec->input_state != NULL);
    dec->output_state = gst_video_decoder_set_output_state (GST_VIDEO_DECODER (dec),
        GST_VIDEO_FORMAT_I420, dst_info.UsrData.sSystemBuffer.iWidth, dst_info.UsrData.sSystemBuffer.iHeight, state);
    gst_video_decoder_negotiate (GST_VIDEO_DECODER (dec));
    codec_dec_send_tags(dec);
  }

  if (dst_info.iBufferStatus == 1) {
    if (deadline < 0) {
      GST_LOG_OBJECT (dec, "Skipping late frame (%f s past deadline)",
          (double) -deadline / GST_SECOND);
      gst_video_decoder_drop_frame (decoder, frame);
    }else {
      ret = gst_video_decoder_allocate_output_frame (decoder, frame);
      if (ret == GST_FLOW_OK) {
        codec_dec_image_to_buffer (dec, &dst_info, pdata, frame->output_buffer);
        ret = gst_video_decoder_finish_frame (decoder, frame);
      } else {
        gst_video_decoder_finish_frame (decoder, frame);
      }
    }
  }else {
    /* Invisible frame */
    GST_VIDEO_CODEC_FRAME_SET_DECODE_ONLY (frame);
    gst_video_decoder_finish_frame (decoder, frame);
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

