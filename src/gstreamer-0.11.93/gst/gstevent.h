/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wim.taymans@chello.be>
 *                    2005 Wim Taymans <wim@fluendo.com>
 *
 * gstevent.h: Header for GstEvent subsystem
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


#ifndef __GST_EVENT_H__
#define __GST_EVENT_H__

typedef struct _GstEvent GstEvent;

/**
 * GstEventTypeFlags:
 * @GST_EVENT_TYPE_UPSTREAM:     Set if the event can travel upstream.
 * @GST_EVENT_TYPE_DOWNSTREAM:   Set if the event can travel downstream.
 * @GST_EVENT_TYPE_SERIALIZED:   Set if the event should be serialized with data
 *                               flow.
 * @GST_EVENT_TYPE_STICKY:       Set if the event is sticky on the pads.
 * @GST_EVENT_TYPE_STICKY_MULTI: Multiple sticky events can be on a pad, each
 *                               identified by the event name.
 *
 * #GstEventTypeFlags indicate the aspects of the different #GstEventType
 * values. You can get the type flags of a #GstEventType with the
 * gst_event_type_get_flags() function.
 */
typedef enum {
  GST_EVENT_TYPE_UPSTREAM       = 1 << 0,
  GST_EVENT_TYPE_DOWNSTREAM     = 1 << 1,
  GST_EVENT_TYPE_SERIALIZED     = 1 << 2,
  GST_EVENT_TYPE_STICKY         = 1 << 3,
  GST_EVENT_TYPE_STICKY_MULTI   = 1 << 4
} GstEventTypeFlags;

/**
 * GST_EVENT_TYPE_BOTH:
 *
 * The same thing as #GST_EVENT_TYPE_UPSTREAM | #GST_EVENT_TYPE_DOWNSTREAM.
 */
#define GST_EVENT_TYPE_BOTH \
    (GST_EVENT_TYPE_UPSTREAM | GST_EVENT_TYPE_DOWNSTREAM)

#define GST_EVENT_NUM_SHIFT     (8)

/**
 * GST_EVENT_MAKE_TYPE:
 * @num: the event number to create
 * @flags: the event flags
 *
 * when making custom event types, use this macro with the num and
 * the given flags
 */
#define GST_EVENT_MAKE_TYPE(num,flags) \
    (((num) << GST_EVENT_NUM_SHIFT) | (flags))

#define FLAG(name) GST_EVENT_TYPE_##name

/**
 * GstEventType:
 * @GST_EVENT_UNKNOWN: unknown event.
 * @GST_EVENT_FLUSH_START: Start a flush operation. This event clears all data
 *                 from the pipeline and unblock all streaming threads.
 * @GST_EVENT_FLUSH_STOP: Stop a flush operation. This event resets the
 *                 running-time of the pipeline.
 * @GST_EVENT_STREAM_START: Event to mark the start of a new stream. Sent before any
 *                 other serialized event and only sent at the start of a new stream,
 *                 not after flushing seeks.
 * @GST_EVENT_CAPS: #GstCaps event. Notify the pad of a new media type.
 * @GST_EVENT_STREAM_CONFIG: (unimplemented) contains configuration information for the stream,
 *                 for example stream-headers and codec-data.
 * @GST_EVENT_SEGMENT: A new media segment follows in the dataflow. The
 *                 segment events contains information for clipping buffers and
 *                 converting buffer timestamps to running-time and
 *                 stream-time.
 * @GST_EVENT_TAG: A new set of metadata tags has been found in the stream.
 * @GST_EVENT_BUFFERSIZE: Notification of buffering requirements. Currently not
 *                 used yet.
 * @GST_EVENT_SINK_MESSAGE: An event that sinks turn into a message. Used to
 *                          send messages that should be emitted in sync with
 *                          rendering.
 * @GST_EVENT_EOS: End-Of-Stream. No more data is to be expected to follow
 *                 without a SEGMENT event.
 * @GST_EVENT_SEGMENT_DONE: Marks the end of a segment playback.
 * @GST_EVENT_GAP: Marks a gap in the datastream.
 * @GST_EVENT_TOC: An event which indicates that a new table of contents (TOC)
 *                 was found or updated.
 * @GST_EVENT_QOS: A quality message. Used to indicate to upstream elements
 *                 that the downstream elements should adjust their processing
 *                 rate.
 * @GST_EVENT_SEEK: A request for a new playback position and rate.
 * @GST_EVENT_NAVIGATION: Navigation events are usually used for communicating
 *                        user requests, such as mouse or keyboard movements,
 *                        to upstream elements.
 * @GST_EVENT_LATENCY: Notification of new latency adjustment. Sinks will use
 *                     the latency information to adjust their synchronisation.
 * @GST_EVENT_STEP: A request for stepping through the media. Sinks will usually
 *                  execute the step operation.
 * @GST_EVENT_RECONFIGURE: A request for upstream renegotiating caps and reconfiguring.
 * @GST_EVENT_TOC_SELECT: A request for a new playback position based on TOC
 *                        entry's UID.
 * @GST_EVENT_CUSTOM_UPSTREAM: Upstream custom event
 * @GST_EVENT_CUSTOM_DOWNSTREAM: Downstream custom event that travels in the
 *                        data flow.
 * @GST_EVENT_CUSTOM_DOWNSTREAM_OOB: Custom out-of-band downstream event.
 * @GST_EVENT_CUSTOM_DOWNSTREAM_STICKY: Custom sticky downstream event.
 * @GST_EVENT_CUSTOM_BOTH: Custom upstream or downstream event.
 *                         In-band when travelling downstream.
 * @GST_EVENT_CUSTOM_BOTH_OOB: Custom upstream or downstream out-of-band event.
 *
 * #GstEventType lists the standard event types that can be sent in a pipeline.
 *
 * The custom event types can be used for private messages between elements
 * that can't be expressed using normal
 * GStreamer buffer passing semantics. Custom events carry an arbitrary
 * #GstStructure.
 * Specific custom events are distinguished by the name of the structure.
 */
/* NOTE: keep in sync with quark registration in gstevent.c */
typedef enum {
  GST_EVENT_UNKNOWN               = GST_EVENT_MAKE_TYPE (0, 0),

  /* bidirectional events */
  GST_EVENT_FLUSH_START           = GST_EVENT_MAKE_TYPE (10, FLAG(BOTH)),
  GST_EVENT_FLUSH_STOP            = GST_EVENT_MAKE_TYPE (20, FLAG(BOTH) | FLAG(SERIALIZED)),

  /* downstream serialized events */
  GST_EVENT_STREAM_START          = GST_EVENT_MAKE_TYPE (40, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_CAPS                  = GST_EVENT_MAKE_TYPE (50, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_STREAM_CONFIG         = GST_EVENT_MAKE_TYPE (60, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_SEGMENT               = GST_EVENT_MAKE_TYPE (70, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_TAG                   = GST_EVENT_MAKE_TYPE (80, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY) | FLAG(STICKY_MULTI)),
  GST_EVENT_BUFFERSIZE            = GST_EVENT_MAKE_TYPE (90, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_SINK_MESSAGE          = GST_EVENT_MAKE_TYPE (100, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY) | FLAG(STICKY_MULTI)),
  GST_EVENT_EOS                   = GST_EVENT_MAKE_TYPE (110, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_TOC                   = GST_EVENT_MAKE_TYPE (120, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY) | FLAG(STICKY_MULTI)),

  /* non-sticky downstream serialized */
  GST_EVENT_SEGMENT_DONE          = GST_EVENT_MAKE_TYPE (150, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),
  GST_EVENT_GAP                   = GST_EVENT_MAKE_TYPE (160, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),

  /* upstream events */
  GST_EVENT_QOS                   = GST_EVENT_MAKE_TYPE (190, FLAG(UPSTREAM)),
  GST_EVENT_SEEK                  = GST_EVENT_MAKE_TYPE (200, FLAG(UPSTREAM)),
  GST_EVENT_NAVIGATION            = GST_EVENT_MAKE_TYPE (210, FLAG(UPSTREAM)),
  GST_EVENT_LATENCY               = GST_EVENT_MAKE_TYPE (220, FLAG(UPSTREAM)),
  GST_EVENT_STEP                  = GST_EVENT_MAKE_TYPE (230, FLAG(UPSTREAM)),
  GST_EVENT_RECONFIGURE           = GST_EVENT_MAKE_TYPE (240, FLAG(UPSTREAM)),
  GST_EVENT_TOC_SELECT            = GST_EVENT_MAKE_TYPE (250, FLAG(UPSTREAM)),

  /* custom events start here */
  GST_EVENT_CUSTOM_UPSTREAM          = GST_EVENT_MAKE_TYPE (270, FLAG(UPSTREAM)),
  GST_EVENT_CUSTOM_DOWNSTREAM        = GST_EVENT_MAKE_TYPE (280, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),
  GST_EVENT_CUSTOM_DOWNSTREAM_OOB    = GST_EVENT_MAKE_TYPE (290, FLAG(DOWNSTREAM)),
  GST_EVENT_CUSTOM_DOWNSTREAM_STICKY = GST_EVENT_MAKE_TYPE (300, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY) | FLAG(STICKY_MULTI)),
  GST_EVENT_CUSTOM_BOTH              = GST_EVENT_MAKE_TYPE (310, FLAG(BOTH) | FLAG(SERIALIZED)),
  GST_EVENT_CUSTOM_BOTH_OOB          = GST_EVENT_MAKE_TYPE (320, FLAG(BOTH))
} GstEventType;
#undef FLAG

#include <gst/gstminiobject.h>
#include <gst/gstformat.h>
#include <gst/gstobject.h>
#include <gst/gstclock.h>
#include <gst/gststructure.h>
#include <gst/gsttaglist.h>
#include <gst/gstsegment.h>
#include <gst/gstsegment.h>
#include <gst/gstmessage.h>

G_BEGIN_DECLS

GST_EXPORT GType _gst_event_type;

#define GST_TYPE_EVENT                  (_gst_event_type)
#define GST_IS_EVENT(obj)               (GST_IS_MINI_OBJECT_TYPE (obj, GST_TYPE_EVENT))
#define GST_EVENT_CAST(obj)             ((GstEvent *)(obj))
#define GST_EVENT(obj)                  (GST_EVENT_CAST(obj))

/**
 * GST_EVENT_TYPE:
 * @event: the event to query
 *
 * Get the #GstEventType of the event.
 */
#define GST_EVENT_TYPE(event)           (GST_EVENT_CAST(event)->type)

/**
 * GST_EVENT_TYPE_NAME:
 * @event: the event to query
 *
 * Get a constant string representation of the #GstEventType of the event.
 */
#define GST_EVENT_TYPE_NAME(event)      (gst_event_type_get_name(GST_EVENT_TYPE(event)))

/**
 * GST_EVENT_TIMESTAMP:
 * @event: the event to query
 *
 * Get the #GstClockTime timestamp of the event. This is the time when the event
 * was created.
 */
#define GST_EVENT_TIMESTAMP(event)      (GST_EVENT_CAST(event)->timestamp)

/**
 * GST_EVENT_SEQNUM:
 * @event: the event to query
 *
 * The sequence number of @event.
 */
#define GST_EVENT_SEQNUM(event)         (GST_EVENT_CAST(event)->seqnum)

/**
 * GST_EVENT_IS_UPSTREAM:
 * @ev: the event to query
 *
 * Check if an event can travel upstream.
 */
#define GST_EVENT_IS_UPSTREAM(ev)       !!(GST_EVENT_TYPE (ev) & GST_EVENT_TYPE_UPSTREAM)
/**
 * GST_EVENT_IS_DOWNSTREAM:
 * @ev: the event to query
 *
 * Check if an event can travel downstream.
 */
#define GST_EVENT_IS_DOWNSTREAM(ev)     !!(GST_EVENT_TYPE (ev) & GST_EVENT_TYPE_DOWNSTREAM)
/**
 * GST_EVENT_IS_SERIALIZED:
 * @ev: the event to query
 *
 * Check if an event is serialized with the data stream.
 */
#define GST_EVENT_IS_SERIALIZED(ev)     !!(GST_EVENT_TYPE (ev) & GST_EVENT_TYPE_SERIALIZED)
/**
 * GST_EVENT_IS_STICKY:
 * @ev: the event to query
 *
 * Check if an event is sticky on the pads.
 */
#define GST_EVENT_IS_STICKY(ev)     !!(GST_EVENT_TYPE (ev) & GST_EVENT_TYPE_STICKY)

/**
 * gst_event_is_writable:
 * @ev: a #GstEvent
 *
 * Tests if you can safely write data into a event's structure or validly
 * modify the seqnum and timestamp field.
 */
#define         gst_event_is_writable(ev)     gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (ev))
/**
 * gst_event_make_writable:
 * @ev: (transfer full): a #GstEvent
 *
 * Makes a writable event from the given event. If the source event is
 * already writable, this will simply return the same event. A copy will
 * otherwise be made using gst_event_copy().
 *
 * Returns: (transfer full): a writable event which may or may not be the
 *     same as @ev
 */
#define         gst_event_make_writable(ev)   GST_EVENT_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (ev)))
/**
 * gst_event_replace:
 * @old_event: (inout) (transfer full): pointer to a pointer to a #GstEvent
 *     to be replaced.
 * @new_event: (allow-none) (transfer none): pointer to a #GstEvent that will
 *     replace the event pointed to by @old_event.
 *
 * Modifies a pointer to a #GstEvent to point to a different #GstEvent. The
 * modification is done atomically (so this is useful for ensuring thread safety
 * in some cases), and the reference counts are updated appropriately (the old
 * event is unreffed, the new one is reffed).
 *
 * Either @new_event or the #GstEvent pointed to by @old_event may be NULL.
 *
 * Returns: TRUE if @new_event was different from @old_event
 */
#ifdef _FOOL_GTK_DOC_
G_INLINE_FUNC gboolean gst_event_replace (GstEvent **old_event, GstEvent *new_event);
#endif

static inline gboolean
gst_event_replace (GstEvent **old_event, GstEvent *new_event)
{
  return gst_mini_object_replace ((GstMiniObject **) old_event, (GstMiniObject *) new_event);
}

/**
 * gst_event_steal:
 * @old_event: (inout) (transfer full): pointer to a pointer to a #GstEvent
 *     to be stolen.
 *
 * Atomically replace the #GstEvent pointed to by @old_event with NULL and
 * return the original event.
 *
 * Returns: the #GstEvent that was in @old_event
 */
#ifdef _FOOL_GTK_DOC_
G_INLINE_FUNC GstEvent * gst_event_steal (GstEvent **old_event);
#endif

static inline GstEvent *
gst_event_steal (GstEvent **old_event)
{
  return GST_EVENT_CAST (gst_mini_object_steal ((GstMiniObject **) old_event));
}

/**
 * gst_event_take:
 * @old_event: (inout) (transfer full): pointer to a pointer to a #GstEvent
 *     to be stolen.
 * @new_event: (allow-none) (transfer full): pointer to a #GstEvent that will
 *     replace the event pointed to by @old_event.
 *
 * Modifies a pointer to a #GstEvent to point to a different #GstEvent. This
 * function is similar to gst_event_replace() except that it takes ownership of
 * @new_event.
 *
 * Either @new_event or the #GstEvent pointed to by @old_event may be NULL.
 *
 * Returns: TRUE if @new_event was different from @old_event
 */
#ifdef _FOOL_GTK_DOC_
G_INLINE_FUNC gboolean gst_event_take (GstEvent **old_event, GstEvent *new_event);
#endif

static inline gboolean
gst_event_take (GstEvent **old_event, GstEvent *new_event)
{
  return gst_mini_object_take ((GstMiniObject **) old_event, (GstMiniObject *) new_event);
}

/**
 * GstQOSType:
 * @GST_QOS_TYPE_OVERFLOW: The QoS event type that is produced when downstream
 *    elements are producing data too quickly and the element can't keep up
 *    processing the data. Upstream should reduce their processing rate. This
 *    type is also used when buffers arrive early or in time.
 * @GST_QOS_TYPE_UNDERFLOW: The QoS event type that is produced when downstream
 *    elements are producing data too slowly and need to speed up their processing
 *    rate.
 * @GST_QOS_TYPE_THROTTLE: The QoS event type that is produced when the
 *    application enabled throttling to limit the datarate.
 *
 * The different types of QoS events that can be given to the
 * gst_event_new_qos() method.
 */
typedef enum {
  GST_QOS_TYPE_OVERFLOW        = 0,
  GST_QOS_TYPE_UNDERFLOW       = 1,
  GST_QOS_TYPE_THROTTLE        = 2
} GstQOSType;

/**
 * GstStreamConfigFlags:
 * @GST_STREAM_CONFIG_FLAG_NONE: no flags set
 *
 * GstStreamConfigFlags are flags passed with the stream config event, see
 * gst_event_new_stream_config().
 */
typedef enum {
  GST_STREAM_CONFIG_FLAG_NONE = 0
} GstStreamConfigFlags;

/**
 * GstEvent:
 * @mini_object: the parent structure
 * @type: the #GstEventType of the event
 * @timestamp: the timestamp of the event
 * @seqnum: the sequence number of the event
 *
 * A #GstEvent.
 */
struct _GstEvent {
  GstMiniObject mini_object;

  /*< public >*/ /* with COW */
  GstEventType  type;
  guint64       timestamp;
  guint32       seqnum;
};

const gchar*    gst_event_type_get_name         (GstEventType type);
GQuark          gst_event_type_to_quark         (GstEventType type);
GstEventTypeFlags
                gst_event_type_get_flags        (GstEventType type);


/* refcounting */
/**
 * gst_event_ref:
 * @event: The event to refcount
 *
 * Increase the refcount of this event.
 *
 * Returns: (transfer full): @event (for convenience when doing assignments)
 */
#ifdef _FOOL_GTK_DOC_
G_INLINE_FUNC GstEvent * gst_event_ref (GstEvent * event);
#endif

static inline GstEvent *
gst_event_ref (GstEvent * event)
{
  return (GstEvent *) gst_mini_object_ref (GST_MINI_OBJECT_CAST (event));
}

/**
 * gst_event_unref:
 * @event: (transfer full): the event to refcount
 *
 * Decrease the refcount of an event, freeing it if the refcount reaches 0.
 */
#ifdef _FOOL_GTK_DOC_
G_INLINE_FUNC void gst_event_unref (GstEvent * event);
#endif

static inline void
gst_event_unref (GstEvent * event)
{
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (event));
}

/* copy event */
/**
 * gst_event_copy:
 * @event: The event to copy
 *
 * Copy the event using the event specific copy function.
 *
 * Returns: (transfer full): the new event
 */
#ifdef _FOOL_GTK_DOC_
G_INLINE_FUNC GstEvent * gst_event_copy (const GstEvent * event);
#endif

static inline GstEvent *
gst_event_copy (const GstEvent * event)
{
  return GST_EVENT_CAST (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (event)));
}

GType           gst_event_get_type              (void);

/* custom event */
GstEvent*       gst_event_new_custom            (GstEventType type, GstStructure *structure) G_GNUC_MALLOC;

const GstStructure *
                gst_event_get_structure         (GstEvent *event);
GstStructure *  gst_event_writable_structure    (GstEvent *event);

gboolean        gst_event_has_name              (GstEvent *event, const gchar *name);

/* identifiers for events and messages */
guint32         gst_event_get_seqnum            (GstEvent *event);
void            gst_event_set_seqnum            (GstEvent *event, guint32 seqnum);

/* Stream start event */
GstEvent *      gst_event_new_stream_start      (const gchar *stream_id) G_GNUC_MALLOC;
void            gst_event_parse_stream_start    (GstEvent *event, const gchar **stream_id);

/* flush events */
GstEvent *      gst_event_new_flush_start       (void) G_GNUC_MALLOC;

GstEvent *      gst_event_new_flush_stop        (gboolean reset_time) G_GNUC_MALLOC;
void            gst_event_parse_flush_stop      (GstEvent *event, gboolean *reset_time);

/* EOS event */
GstEvent *      gst_event_new_eos               (void) G_GNUC_MALLOC;

/* GAP event */
GstEvent *      gst_event_new_gap               (GstClockTime   timestamp,
                                                 GstClockTime   duration) G_GNUC_MALLOC;

void            gst_event_parse_gap             (GstEvent     * event,
                                                 GstClockTime * timestamp,
                                                 GstClockTime * duration);

/* Caps events */
GstEvent *      gst_event_new_caps              (GstCaps *caps) G_GNUC_MALLOC;
void            gst_event_parse_caps            (GstEvent *event, GstCaps **caps);

/* Stream config */
GstEvent *      gst_event_new_stream_config                (GstStreamConfigFlags flags) G_GNUC_MALLOC;

void            gst_event_parse_stream_config              (GstEvent * event, GstStreamConfigFlags * flags);


void            gst_event_set_stream_config_setup_data     (GstEvent * event, GstBuffer  * buf);

gboolean        gst_event_parse_stream_config_setup_data   (GstEvent * event, GstBuffer ** buf);


void            gst_event_add_stream_config_header         (GstEvent * event, GstBuffer  * buf);

guint           gst_event_get_n_stream_config_headers      (GstEvent * event);

gboolean        gst_event_parse_nth_stream_config_header   (GstEvent * event, guint index, GstBuffer ** buf);

/* segment event */
GstEvent*       gst_event_new_segment           (const GstSegment *segment) G_GNUC_MALLOC;
void            gst_event_parse_segment         (GstEvent *event, const GstSegment **segment);
void            gst_event_copy_segment          (GstEvent *event, GstSegment *segment);

/* tag event */
GstEvent*       gst_event_new_tag               (GstTagList *taglist) G_GNUC_MALLOC;
void            gst_event_parse_tag             (GstEvent *event, GstTagList **taglist);

/* TOC event */
GstEvent*      gst_event_new_toc                (GstToc *toc, gboolean updated);
void           gst_event_parse_toc              (GstEvent *event, GstToc **toc, gboolean *updated);


/* buffer */
GstEvent *      gst_event_new_buffer_size       (GstFormat format, gint64 minsize, gint64 maxsize,
                                                 gboolean async) G_GNUC_MALLOC;
void            gst_event_parse_buffer_size     (GstEvent *event, GstFormat *format, gint64 *minsize,
                                                 gint64 *maxsize, gboolean *async);

/* sink message */
GstEvent*       gst_event_new_sink_message      (const gchar *name, GstMessage *msg) G_GNUC_MALLOC;
void            gst_event_parse_sink_message    (GstEvent *event, GstMessage **msg);

/* QOS events */
GstEvent*       gst_event_new_qos               (GstQOSType type, gdouble proportion,
                                                 GstClockTimeDiff diff, GstClockTime timestamp) G_GNUC_MALLOC;
void            gst_event_parse_qos             (GstEvent *event, GstQOSType *type,
                                                 gdouble *proportion, GstClockTimeDiff *diff,
                                                 GstClockTime *timestamp);
/* seek event */
GstEvent*       gst_event_new_seek              (gdouble rate, GstFormat format, GstSeekFlags flags,
                                                 GstSeekType start_type, gint64 start,
                                                 GstSeekType stop_type, gint64 stop) G_GNUC_MALLOC;
void            gst_event_parse_seek            (GstEvent *event, gdouble *rate, GstFormat *format,
                                                 GstSeekFlags *flags,
                                                 GstSeekType *start_type, gint64 *start,
                                                 GstSeekType *stop_type, gint64 *stop);

/* navigation event */
GstEvent*       gst_event_new_navigation        (GstStructure *structure) G_GNUC_MALLOC;

/* latency event */
GstEvent*       gst_event_new_latency           (GstClockTime latency) G_GNUC_MALLOC;
void            gst_event_parse_latency         (GstEvent *event, GstClockTime *latency);

/* step event */
GstEvent*       gst_event_new_step              (GstFormat format, guint64 amount, gdouble rate,
                                                 gboolean flush, gboolean intermediate) G_GNUC_MALLOC;
void            gst_event_parse_step            (GstEvent *event, GstFormat *format, guint64 *amount,
                                                 gdouble *rate, gboolean *flush, gboolean *intermediate);

/* renegotiate event */
GstEvent*       gst_event_new_reconfigure       (void) G_GNUC_MALLOC;

/* TOC select event */
GstEvent*       gst_event_new_toc_select        (const gchar *uid) G_GNUC_MALLOC;
void            gst_event_parse_toc_select      (GstEvent *event, gchar **uid);

/* segment-done event */
GstEvent*       gst_event_new_segment_done      (GstFormat format, gint64 position) G_GNUC_MALLOC;
void            gst_event_parse_segment_done    (GstEvent *event, GstFormat *format, gint64 *position);

G_END_DECLS

#endif /* __GST_EVENT_H__ */
