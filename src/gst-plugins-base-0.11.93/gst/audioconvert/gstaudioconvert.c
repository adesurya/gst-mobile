/* GStreamer
 * Copyright (C) 2003 Benjamin Otte <in7y118@public.uni-hamburg.de>
 * Copyright (C) 2005 Thomas Vander Stichele <thomas at apestaart dot org>
 * Copyright (C) 2011 Wim Taymans <wim.taymans at gmail dot com>
 *
 * gstaudioconvert.c: Convert audio to different audio formats automatically
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-audioconvert
 *
 * Audioconvert converts raw audio buffers between various possible formats.
 * It supports integer to float conversion, width/depth conversion,
 * signedness and endianness conversion and channel transformations.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m audiotestsrc ! audioconvert ! audio/x-raw,format=S8,channels=2 ! level ! fakesink silent=TRUE
 * ]| This pipeline converts audio to 8-bit.  The level element shows that
 * the output levels still match the one for a sine wave.
 * |[
 * gst-launch -v -m audiotestsrc ! audioconvert ! vorbisenc ! fakesink silent=TRUE
 * ]| The vorbis encoder takes float audio data instead of the integer data
 * generated by audiotestsrc.
 * </refsect2>
 *
 * Last reviewed on 2006-03-02 (0.10.4)
 */

/*
 * design decisions:
 * - audioconvert converts buffers in a set of supported caps. If it supports
 *   a caps, it supports conversion from these caps to any other caps it
 *   supports. (example: if it does A=>B and A=>C, it also does B=>C)
 * - audioconvert does not save state between buffers. Every incoming buffer is
 *   converted and the converted buffer is pushed out.
 * conclusion:
 * audioconvert is not supposed to be a one-element-does-anything solution for
 * audio conversions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "gstaudioconvert.h"
#include "gstchannelmix.h"
#include "gstaudioquantize.h"
#include "plugin.h"

GST_DEBUG_CATEGORY (audio_convert_debug);
GST_DEBUG_CATEGORY_STATIC (GST_CAT_PERFORMANCE);

/*** DEFINITIONS **************************************************************/

/* type functions */
static void gst_audio_convert_dispose (GObject * obj);

/* gstreamer functions */
static gboolean gst_audio_convert_get_unit_size (GstBaseTransform * base,
    GstCaps * caps, gsize * size);
static GstCaps *gst_audio_convert_transform_caps (GstBaseTransform * base,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter);
static GstCaps *gst_audio_convert_fixate_caps (GstBaseTransform * base,
    GstPadDirection direction, GstCaps * caps, GstCaps * othercaps);
static gboolean gst_audio_convert_set_caps (GstBaseTransform * base,
    GstCaps * incaps, GstCaps * outcaps);
static GstFlowReturn gst_audio_convert_transform (GstBaseTransform * base,
    GstBuffer * inbuf, GstBuffer * outbuf);
static void gst_audio_convert_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_audio_convert_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

/* AudioConvert signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
  ARG_DITHERING,
  ARG_NOISE_SHAPING,
};

#define DEBUG_INIT \
  GST_DEBUG_CATEGORY_INIT (audio_convert_debug, "audioconvert", 0, "audio conversion element"); \
  GST_DEBUG_CATEGORY_GET (GST_CAT_PERFORMANCE, "GST_PERFORMANCE");
#define gst_audio_convert_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstAudioConvert, gst_audio_convert,
    GST_TYPE_BASE_TRANSFORM, DEBUG_INIT);

/*** GSTREAMER PROTOTYPES *****************************************************/

#define STATIC_CAPS \
GST_STATIC_CAPS (GST_AUDIO_CAPS_MAKE (GST_AUDIO_FORMATS_ALL) \
    ", layout = (string) interleaved")

static GstStaticPadTemplate gst_audio_convert_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    STATIC_CAPS);

static GstStaticPadTemplate gst_audio_convert_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    STATIC_CAPS);

#define GST_TYPE_AUDIO_CONVERT_DITHERING (gst_audio_convert_dithering_get_type ())
static GType
gst_audio_convert_dithering_get_type (void)
{
  static GType gtype = 0;

  if (gtype == 0) {
    static const GEnumValue values[] = {
      {DITHER_NONE, "No dithering",
          "none"},
      {DITHER_RPDF, "Rectangular dithering", "rpdf"},
      {DITHER_TPDF, "Triangular dithering (default)", "tpdf"},
      {DITHER_TPDF_HF, "High frequency triangular dithering", "tpdf-hf"},
      {0, NULL, NULL}
    };

    gtype = g_enum_register_static ("GstAudioConvertDithering", values);
  }
  return gtype;
}

#define GST_TYPE_AUDIO_CONVERT_NOISE_SHAPING (gst_audio_convert_ns_get_type ())
static GType
gst_audio_convert_ns_get_type (void)
{
  static GType gtype = 0;

  if (gtype == 0) {
    static const GEnumValue values[] = {
      {NOISE_SHAPING_NONE, "No noise shaping (default)",
          "none"},
      {NOISE_SHAPING_ERROR_FEEDBACK, "Error feedback", "error-feedback"},
      {NOISE_SHAPING_SIMPLE, "Simple 2-pole noise shaping", "simple"},
      {NOISE_SHAPING_MEDIUM, "Medium 5-pole noise shaping", "medium"},
      {NOISE_SHAPING_HIGH, "High 8-pole noise shaping", "high"},
      {0, NULL, NULL}
    };

    gtype = g_enum_register_static ("GstAudioConvertNoiseShaping", values);
  }
  return gtype;
}


/*** TYPE FUNCTIONS ***********************************************************/
static void
gst_audio_convert_class_init (GstAudioConvertClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseTransformClass *basetransform_class = GST_BASE_TRANSFORM_CLASS (klass);

  gobject_class->dispose = gst_audio_convert_dispose;
  gobject_class->set_property = gst_audio_convert_set_property;
  gobject_class->get_property = gst_audio_convert_get_property;

  g_object_class_install_property (gobject_class, ARG_DITHERING,
      g_param_spec_enum ("dithering", "Dithering",
          "Selects between different dithering methods.",
          GST_TYPE_AUDIO_CONVERT_DITHERING, DITHER_TPDF,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, ARG_NOISE_SHAPING,
      g_param_spec_enum ("noise-shaping", "Noise shaping",
          "Selects between different noise shaping methods.",
          GST_TYPE_AUDIO_CONVERT_NOISE_SHAPING, NOISE_SHAPING_NONE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_audio_convert_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_audio_convert_sink_template));
  gst_element_class_set_static_metadata (element_class,
      "Audio converter", "Filter/Converter/Audio",
      "Convert audio to different formats", "Benjamin Otte <otte@gnome.org>");

  basetransform_class->get_unit_size =
      GST_DEBUG_FUNCPTR (gst_audio_convert_get_unit_size);
  basetransform_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_audio_convert_transform_caps);
  basetransform_class->fixate_caps =
      GST_DEBUG_FUNCPTR (gst_audio_convert_fixate_caps);
  basetransform_class->set_caps =
      GST_DEBUG_FUNCPTR (gst_audio_convert_set_caps);
  basetransform_class->transform =
      GST_DEBUG_FUNCPTR (gst_audio_convert_transform);

  basetransform_class->passthrough_on_same_caps = TRUE;
}

static void
gst_audio_convert_init (GstAudioConvert * this)
{
  this->dither = DITHER_TPDF;
  this->ns = NOISE_SHAPING_NONE;
  memset (&this->ctx, 0, sizeof (AudioConvertCtx));

  gst_base_transform_set_gap_aware (GST_BASE_TRANSFORM (this), TRUE);
}

static void
gst_audio_convert_dispose (GObject * obj)
{
  GstAudioConvert *this = GST_AUDIO_CONVERT (obj);

  audio_convert_clean_context (&this->ctx);

  G_OBJECT_CLASS (parent_class)->dispose (obj);
}

/*** GSTREAMER FUNCTIONS ******************************************************/

/* BaseTransform vmethods */
static gboolean
gst_audio_convert_get_unit_size (GstBaseTransform * base, GstCaps * caps,
    gsize * size)
{
  GstAudioInfo info;

  g_assert (size);

  if (!gst_audio_info_from_caps (&info, caps))
    goto parse_error;

  *size = info.bpf;
  GST_INFO_OBJECT (base, "unit_size = %" G_GSIZE_FORMAT, *size);

  return TRUE;

parse_error:
  {
    GST_INFO_OBJECT (base, "failed to parse caps to get unit_size");
    return FALSE;
  }
}

/* copies the given caps */
static GstCaps *
gst_audio_convert_caps_remove_format_info (GstCaps * caps, gboolean channels)
{
  GstStructure *st;
  gint i, n;
  GstCaps *res;
  guint64 channel_mask;

  res = gst_caps_new_empty ();

  n = gst_caps_get_size (caps);
  for (i = 0; i < n; i++) {
    gboolean remove_channels = FALSE;

    st = gst_caps_get_structure (caps, i);

    /* If this is already expressed by the existing caps
     * skip this structure */
    if (i > 0 && gst_caps_is_subset_structure (res, st))
      continue;

    st = gst_structure_copy (st);
    gst_structure_remove_field (st, "format");

    /* Only remove the channels and channel-mask for non-NONE layouts */
    if (gst_structure_get (st, "channel-mask", GST_TYPE_BITMASK, &channel_mask,
            NULL)) {
      if (channel_mask != 0)
        remove_channels = TRUE;
    } else {
      remove_channels = TRUE;
    }

    if (remove_channels && channels)
      gst_structure_remove_fields (st, "channel-mask", "channels", NULL);

    gst_caps_append_structure (res, st);
  }

  return res;
}

/* The caps can be transformed into any other caps with format info removed.
 * However, we should prefer passthrough, so if passthrough is possible,
 * put it first in the list. */
static GstCaps *
gst_audio_convert_transform_caps (GstBaseTransform * btrans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstCaps *tmp, *tmp2;
  GstCaps *result;

  /* Get all possible caps that we can transform to */
  tmp = gst_audio_convert_caps_remove_format_info (caps, TRUE);

  if (filter) {
    tmp2 = gst_caps_intersect_full (filter, tmp, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (tmp);
    tmp = tmp2;
  }

  result = tmp;

  GST_DEBUG_OBJECT (btrans, "transformed %" GST_PTR_FORMAT " into %"
      GST_PTR_FORMAT, caps, result);

  return result;
}

static const GstAudioChannelPosition default_positions[8][8] = {
  /* 1 channel */
  {
        GST_AUDIO_CHANNEL_POSITION_MONO,
      },
  /* 2 channels */
  {
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
      },
  /* 3 channels (2.1) */
  {
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_LFE1,
      },
  /* 4 channels (4.0) */
  {
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
      },
  /* 5 channels */
  {
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
      },
  /* 6 channels (5.1) */
  {
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE1,
      },
  /* 7 channels (6.1) */
  {
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE1,
        GST_AUDIO_CHANNEL_POSITION_REAR_CENTER,
      },
  /* 8 channels (7.1) */
  {
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE1,
        GST_AUDIO_CHANNEL_POSITION_SIDE_LEFT,
        GST_AUDIO_CHANNEL_POSITION_SIDE_RIGHT,
      }
};

static gint
n_bits_set (guint64 x)
{
  gint i;
  gint c = 0;
  guint64 y = 1;

  for (i = 0; i < 64; i++) {
    if (x & y)
      c++;
    y <<= 1;
  }

  return c;
}

static guint64
find_suitable_mask (guint64 mask, gint n_chans)
{
  guint64 intersection;
  gint i;

  i = 0;

  g_assert (n_bits_set (mask) >= n_chans);

  intersection = mask;
  do {
    intersection = intersection & ((~G_GUINT64_CONSTANT (0)) >> i);
    i++;
  } while (n_bits_set (intersection) > n_chans && i < 64);

  if (i < 64)
    return intersection;
  return 0;
}

static void
gst_audio_convert_fixate_channels (GstBaseTransform * base, GstStructure * ins,
    GstStructure * outs)
{
  gint in_chans, out_chans;
  guint64 in_mask = 0, out_mask = 0;
  gboolean has_in_mask = FALSE, has_out_mask = FALSE;

  if (!gst_structure_get_int (ins, "channels", &in_chans))
    return;                     /* this shouldn't really happen, should it? */

  if (!gst_structure_has_field (outs, "channels")) {
    /* we could try to get the implied number of channels from the layout,
     * but that seems overdoing it for a somewhat exotic corner case */
    gst_structure_remove_field (outs, "channel-mask");
    return;
  }

  /* ok, let's fixate the channels if they are not fixated yet */
  gst_structure_fixate_field_nearest_int (outs, "channels", in_chans);

  if (!gst_structure_get_int (outs, "channels", &out_chans)) {
    /* shouldn't really happen ... */
    gst_structure_remove_field (outs, "channel-mask");
    return;
  }

  /* get the channel layout of the output if any */
  has_out_mask = gst_structure_has_field (outs, "channel-mask");
  if (has_out_mask) {
    gst_structure_get (outs, "channel-mask", GST_TYPE_BITMASK, &out_mask, NULL);
  } else {
    /* channels == 1 => MONO */
    if (out_chans == 2) {
      out_mask =
          GST_AUDIO_CHANNEL_POSITION_MASK (FRONT_LEFT) |
          GST_AUDIO_CHANNEL_POSITION_MASK (FRONT_RIGHT);
      has_out_mask = TRUE;
      gst_structure_set (outs, "channel-mask", GST_TYPE_BITMASK, out_mask,
          NULL);
    }
  }

  /* get the channel layout of the input if any */
  has_in_mask = gst_structure_has_field (ins, "channel-mask");
  if (has_in_mask) {
    gst_structure_get (ins, "channel-mask", GST_TYPE_BITMASK, &in_mask, NULL);
  } else {
    /* channels == 1 => MONO */
    if (in_chans == 2) {
      in_mask =
          GST_AUDIO_CHANNEL_POSITION_MASK (FRONT_LEFT) |
          GST_AUDIO_CHANNEL_POSITION_MASK (FRONT_RIGHT);
      has_in_mask = TRUE;
    } else if (in_chans > 2)
      g_warning ("%s: Upstream caps contain no channel mask",
          GST_ELEMENT_NAME (base));
  }

  if (!has_out_mask && out_chans == 1 && (in_chans != out_chans
          || !has_in_mask))
    return;                     /* nothing to do, default layout will be assumed */

  if (in_chans == out_chans && (has_in_mask || in_chans == 1)) {
    /* same number of channels and no output layout: just use input layout */
    if (!has_out_mask) {
      /* in_chans == 1 handled above already */
      gst_structure_set (outs, "channel-mask", GST_TYPE_BITMASK, in_mask, NULL);
      return;
    }

    /* If both masks are the same we're done, this includes the NONE layout case */
    if (in_mask == out_mask)
      return;

    /* if output layout is fixed already and looks sane, we're done */
    if (n_bits_set (out_mask) == out_chans)
      return;

    if (n_bits_set (out_mask) < in_chans) {
      /* Not much we can do here, this shouldn't just happen */
      g_warning ("%s: Invalid downstream channel-mask with too few bits set",
          GST_ELEMENT_NAME (base));
    } else {
      guint64 intersection;

      /* if the output layout is not fixed, check if the output layout contains
       * the input layout */
      intersection = in_mask & out_mask;
      if (n_bits_set (intersection) >= in_chans) {
        gst_structure_set (outs, "channel-mask", GST_TYPE_BITMASK, in_mask,
            NULL);
        return;
      }

      /* output layout is not fixed and does not contain the input layout, so
       * just pick the first possibility */
      intersection = find_suitable_mask (out_mask, out_chans);
      if (intersection) {
        gst_structure_set (outs, "channel-mask", GST_TYPE_BITMASK, intersection,
            NULL);
        return;
      }
    }

    /* ... else fall back to default layout (NB: out_layout is NULL here) */
    GST_WARNING_OBJECT (base, "unexpected output channel layout");
  } else {
    guint64 intersection;

    /* number of input channels != number of output channels:
     * if this value contains a list of channel layouts (or even worse: a list
     * with another list), just pick the first value and repeat until we find a
     * channel position array or something else that's not a list; we assume
     * the input if half-way sane and don't try to fall back on other list items
     * if the first one is something unexpected or non-channel-pos-array-y */
    if (n_bits_set (out_mask) >= out_chans) {
      intersection = find_suitable_mask (out_mask, out_chans);
      gst_structure_set (outs, "channel-mask", GST_TYPE_BITMASK, intersection,
          NULL);
      return;
    }

    /* what now?! Just ignore what we're given and use default positions */
    GST_WARNING_OBJECT (base, "invalid or unexpected channel-positions");
  }

  /* missing or invalid output layout and we can't use the input layout for
   * one reason or another, so just pick a default layout (we could be smarter
   * and try to add/remove channels from the input layout, or pick a default
   * layout based on LFE-presence in input layout, but let's save that for
   * another day) */
  if (out_chans > 0 && out_chans <= G_N_ELEMENTS (default_positions[0])) {
    gint i;

    GST_DEBUG_OBJECT (base, "using default channel layout as fallback");

    out_mask = 0;
    for (i = 0; i < out_chans; i++)
      out_mask |= G_GUINT64_CONSTANT (1) << default_positions[out_chans - 1][i];

    gst_structure_set (outs, "channel-mask", GST_TYPE_BITMASK, out_mask, NULL);
  } else {
    GST_ERROR_OBJECT (base, "Have no default layout for %d channels",
        out_chans);
  }
}

/* try to keep as many of the structure members the same by fixating the
 * possible ranges; this way we convert the least amount of things as possible
 */
static GstCaps *
gst_audio_convert_fixate_caps (GstBaseTransform * base,
    GstPadDirection direction, GstCaps * caps, GstCaps * othercaps)
{
  GstStructure *ins, *outs;
  GstCaps *result;

  GST_DEBUG_OBJECT (base, "trying to fixate othercaps %" GST_PTR_FORMAT
      " based on caps %" GST_PTR_FORMAT, othercaps, caps);

  result = gst_caps_intersect (othercaps, caps);
  if (gst_caps_is_empty (result)) {
    GstCaps *removed;

    if (result)
      gst_caps_unref (result);
    /* try to preserve channels */
    removed = gst_audio_convert_caps_remove_format_info (caps, FALSE);
    result = gst_caps_intersect (othercaps, removed);
    gst_caps_unref (removed);
    if (gst_caps_is_empty (result)) {
      if (result)
        gst_caps_unref (result);
      result = othercaps;
    }
  } else {
    gst_caps_unref (othercaps);
  }

  GST_DEBUG_OBJECT (base, "now fixating %" GST_PTR_FORMAT, result);

  /* fixate remaining fields */
  result = gst_caps_make_writable (result);

  ins = gst_caps_get_structure (caps, 0);
  outs = gst_caps_get_structure (result, 0);

  gst_audio_convert_fixate_channels (base, ins, outs);
  result = gst_caps_fixate (result);

  GST_DEBUG_OBJECT (base, "fixated othercaps to %" GST_PTR_FORMAT, result);

  return result;
}

static gboolean
gst_audio_convert_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstAudioConvert *this = GST_AUDIO_CONVERT (base);
  GstAudioInfo in_info;
  GstAudioInfo out_info;

  GST_DEBUG_OBJECT (base, "incaps %" GST_PTR_FORMAT ", outcaps %"
      GST_PTR_FORMAT, incaps, outcaps);

  if (!gst_audio_info_from_caps (&in_info, incaps))
    goto invalid_in;
  if (!gst_audio_info_from_caps (&out_info, outcaps))
    goto invalid_out;

  if (!audio_convert_prepare_context (&this->ctx, &in_info, &out_info,
          this->dither, this->ns))
    goto no_converter;

  return TRUE;

  /* ERRORS */
invalid_in:
  {
    GST_ERROR_OBJECT (base, "invalid input caps");
    return FALSE;
  }
invalid_out:
  {
    GST_ERROR_OBJECT (base, "invalid output caps");
    return FALSE;
  }
no_converter:
  {
    GST_ERROR_OBJECT (base, "could not find converter");
    return FALSE;
  }
}

static GstFlowReturn
gst_audio_convert_transform (GstBaseTransform * base, GstBuffer * inbuf,
    GstBuffer * outbuf)
{
  GstFlowReturn ret;
  GstAudioConvert *this = GST_AUDIO_CONVERT (base);
  GstMapInfo srcmap, dstmap;
  gint insize, outsize;

  gint samples;

  /* get amount of samples to convert. */
  samples = gst_buffer_get_size (inbuf) / this->ctx.in.bpf;

  /* get in/output sizes, to see if the buffers we got are of correct
   * sizes */
  if (!audio_convert_get_sizes (&this->ctx, samples, &insize, &outsize))
    goto error;

  if (insize == 0 || outsize == 0)
    return GST_FLOW_OK;

  /* get src and dst data */
  gst_buffer_map (inbuf, &srcmap, GST_MAP_READ);
  gst_buffer_map (outbuf, &dstmap, GST_MAP_WRITE);

  /* check in and outsize */
  if (srcmap.size < insize)
    goto wrong_size;
  if (dstmap.size < outsize)
    goto wrong_size;

  /* and convert the samples */
  if (!GST_BUFFER_FLAG_IS_SET (inbuf, GST_BUFFER_FLAG_GAP)) {
    if (!audio_convert_convert (&this->ctx, srcmap.data, dstmap.data,
            samples, gst_buffer_is_writable (inbuf)))
      goto convert_error;
  } else {
    /* Create silence buffer */
    gst_audio_format_fill_silence (this->ctx.out.finfo, dstmap.data, outsize);
  }
  ret = GST_FLOW_OK;

done:
  gst_buffer_unmap (outbuf, &dstmap);
  gst_buffer_unmap (inbuf, &srcmap);

  return ret;

  /* ERRORS */
error:
  {
    GST_ELEMENT_ERROR (this, STREAM, FORMAT,
        (NULL), ("cannot get input/output sizes for %d samples", samples));
    return GST_FLOW_ERROR;
  }
wrong_size:
  {
    GST_ELEMENT_ERROR (this, STREAM, FORMAT,
        (NULL),
        ("input/output buffers are of wrong size in: %" G_GSIZE_FORMAT " < %d"
            " or out: %" G_GSIZE_FORMAT " < %d",
            srcmap.size, insize, dstmap.size, outsize));
    ret = GST_FLOW_ERROR;
    goto done;
  }
convert_error:
  {
    GST_ELEMENT_ERROR (this, STREAM, FORMAT,
        (NULL), ("error while converting"));
    ret = GST_FLOW_ERROR;
    goto done;
  }
}

static void
gst_audio_convert_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAudioConvert *this = GST_AUDIO_CONVERT (object);

  switch (prop_id) {
    case ARG_DITHERING:
      this->dither = g_value_get_enum (value);
      break;
    case ARG_NOISE_SHAPING:
      this->ns = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_audio_convert_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAudioConvert *this = GST_AUDIO_CONVERT (object);

  switch (prop_id) {
    case ARG_DITHERING:
      g_value_set_enum (value, this->dither);
      break;
    case ARG_NOISE_SHAPING:
      g_value_set_enum (value, this->ns);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}