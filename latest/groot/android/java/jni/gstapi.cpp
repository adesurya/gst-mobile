#include <gst/gst.h>
#include <glib.h>
#include "gstapi.h"
#include "ALog-priv.h"

extern "C" void gst_static_plugins(void);

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print ("End of stream\n");
            g_main_loop_quit (loop);
            break;
        case GST_MESSAGE_ERROR: 
        {
            gchar  *debug;
            GError *error;
            gst_message_parse_error (msg, &error, &debug);
            g_free (debug);
            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);
            g_main_loop_quit (loop);
            break;
        }
        default:
        break;
    }

    return TRUE;
}

static void on_pad_added(GstElement *element, GstPad *pad, gpointer data)
{
    GstPad *sinkpad;
    GstElement *decoder = (GstElement *) data;

    /* We can now link this pad with the vorbis-decoder sink pad */
    g_print ("Dynamic pad created, linking demuxer/decoder\n");
    sinkpad = gst_element_get_static_pad (decoder, "sink");
    gst_pad_link (pad, sinkpad);
    gst_object_unref (sinkpad);
}

int ogg_player (const char *location) {

    GMainLoop *loop;
    GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
    GstBus *bus;
    guint bus_watch_id;

    /* Initialisation */
    loop = g_main_loop_new (NULL, FALSE);

    /* Create gstreamer elements */
    pipeline = gst_pipeline_new ("audio-player");
    source   = gst_element_factory_make ("filesrc",       "file-source");
    returnv_assert(source, -1);
    demuxer  = gst_element_factory_make ("oggdemux",      "ogg-demuxer");
    returnv_assert(demuxer, -1);
    decoder  = gst_element_factory_make ("vorbisdec",     "vorbis-decoder");
    returnv_assert(decoder, -1);
    conv     = gst_element_factory_make ("audioconvert",  "converter");
    returnv_assert(conv, -1);
    sink     = gst_element_factory_make ("autoaudiosink", "audio-output");
    returnv_assert(sink, -1);

    if (!pipeline || !source || !demuxer || !decoder || !conv || !sink) {
        g_printerr ("One element could not be created. Exiting.\n");
        return -1;
    }

    /* Set up the pipeline */

    /* we set the input filename to the source element */
    g_object_set (G_OBJECT (source), "location", location, NULL);

    /* we add a message handler */
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    returnv_assert(bus, -1);
    bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    /* we add all elements into the pipeline */
    /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
    gst_bin_add_many (GST_BIN (pipeline), source, demuxer, decoder, conv, sink, NULL);

    /* we link the elements together */
    /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
    gst_element_link (source, demuxer);
    gst_element_link_many (decoder, conv, sink, NULL);
    g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), decoder);

    /* note that the demuxer will be linked to the decoder dynamically.
       The reason is that Ogg may contain various streams (for example
       audio and video). The source pad(s) will be created at run time,
       by the demuxer when it detects the amount and nature of streams.
       Therefore we connect a callback function which will be executed
       when the "pad-added" is emitted.*/

    /* Set the pipeline to "playing" state*/
    g_print ("Now playing: %s\n", location);
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* Iterate */
    g_print ("Running...\n");
    g_main_loop_run (loop);

    /* Out of the main loop, clean up nicely */
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);

    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));
    g_source_remove (bus_watch_id);
    g_main_loop_unref (loop);

    return 0;
}

int playbin2_player(const char *location) {

    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;
    gchar uri[512] = {0};

    /* playbin2 uri=http://docs.gstreamer.com/media/sintel_trailer-480p.webm */
    g_snprintf(uri, sizeof(uri), "playbin2 uri=%s", location);

    /* Build the pipeline */
    pipeline = gst_parse_launch (uri, NULL);

    /* Start playing */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Free resources */
    if (msg != NULL)
        gst_message_unref (msg);
    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
    return 0;
}

namespace eau
{

gboolean CGstPlayback::handle_message (GstBus *bus, GstMessage *msg, void *data)
{
    CGstPlayback *thiz = (CGstPlayback *)data;
    if (thiz)
        return thiz->HandleMessage(bus, msg);
    return false;
}

CGstPlayback::CGstPlayback()
{
    m_playbin = NULL;
    m_audio_sink = NULL;
    m_video_sink = NULL;
    m_main_loop = NULL;
    m_bus_msg_thread = NULL;
}

CGstPlayback::~CGstPlayback()
{
    Uninit();
}

bool CGstPlayback::Init()
{
    g_print("%s, begin", __func__);

    m_main_loop = g_main_loop_new (NULL, FALSE);
    returnb_assert(m_main_loop);

    m_playbin = gst_element_factory_make ("playbin", "playbin");
    returnb_assert(m_playbin);

    GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE(m_playbin));
    returnb_assert(bus);
    gst_bus_add_watch (bus, (GstBusFunc)handle_message, this);
    gst_object_unref (bus);

    m_bus_msg_thread = g_thread_create ((GThreadFunc)g_main_loop_run, m_main_loop, TRUE, NULL);
    returnb_assert(m_bus_msg_thread);

    //m_audio_sink = gst_element_factory_make("autoaudiosink", NULL);
    //returnb_assert(m_audio_sink);
    //g_object_set (GST_OBJECT(m_playbin), "audio-sink", m_audio_sink, NULL);

    //m_video_sink = gst_element_factory_make("eglglessink", NULL);
    //returnb_assert(m_video_sink);
    //g_object_set (GST_OBJECT(m_playbin), "video-sink", m_video_sink, NULL);

    g_print("%s, end", __func__);

    return true;
}

bool CGstPlayback::SetOption()
{
    returnb_assert(m_playbin);

    ALOGI("%s, set flags", __func__);
    g_object_get (G_OBJECT(m_playbin), "flags", &m_flags, NULL);
    m_flags |= GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_VIDEO;
    m_flags &= ~GST_PLAY_FLAG_TEXT;
    g_object_set (G_OBJECT(m_playbin), "flags", m_flags, NULL);
    //g_object_set (m_playbin, "connection-speed", 56, NULL);
    return true;
}

bool CGstPlayback::SetUri(const char *uri)
{
    returnb_assert(m_playbin);

    g_print("%s, set uri=%s", __func__, uri);
    g_object_set (G_OBJECT(m_playbin), "uri", uri, NULL);
    return true;
}

bool CGstPlayback::Play()
{
    returnb_assert(m_playbin);

    g_print("%s, begin", __func__);
    gint iret = gst_element_set_state (m_playbin, GST_STATE_PLAYING);
    if (iret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        return false;
    }

    //g_main_loop_run (m_main_loop);
    //gst_element_set_state (m_playbin, GST_STATE_NULL);

    return true;
}

bool CGstPlayback::Pause()
{
    returnb_assert(m_playbin);

    g_print("%s, begin", __func__);
    gint iret = gst_element_set_state (m_playbin, GST_STATE_PAUSED);
    if (iret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        return false;
    }
    return true;
}

bool CGstPlayback::Stop()
{
    returnb_assert(m_playbin);

    g_print("%s, begin", __func__);
    gint iret = gst_element_set_state (m_playbin, GST_STATE_READY);
    if (iret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        return false;
    }
    return true;
}

void CGstPlayback::Uninit()
{
    g_print("%s, begin", __func__);
    g_main_loop_unref (m_main_loop);
    gst_element_set_state (m_playbin, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT(m_playbin));
    gst_object_unref (GST_OBJECT(m_audio_sink));
    gst_object_unref (GST_OBJECT(m_video_sink));

    m_main_loop = NULL;
    m_playbin = NULL;
    m_audio_sink = NULL;
    m_video_sink = NULL;
    m_bus_msg_thread = NULL;
}

void CGstPlayback::AnalyzeStreams()
{
    return_assert(m_playbin);

    g_print("%s, begin", __func__);
    g_object_get (m_playbin, "n-video", &m_numVideo, NULL);
    g_object_get (m_playbin, "n-audio", &m_numAudio, NULL);
    g_object_get (m_playbin, "n-text", &m_numText, NULL);   

    for (gint i = 0; i < m_numVideo; i++) {
        GstTagList *tags = NULL;
        gchar *str = NULL;
        /* Retrieve the stream's video tags */
        g_signal_emit_by_name (m_playbin, "get-video-tags", i, &tags);
        if (tags) {
            g_print ("video stream %d:\n", i);
            gst_tag_list_get_string (tags, GST_TAG_VIDEO_CODEC, &str);
            g_print ("  codec: %s\n", str ? str : "unknown");
            g_free (str);
            gst_tag_list_free (tags);
        }
    }

    for (gint i = 0; i < m_numAudio; i++) {
        GstTagList *tags = NULL;
        gchar *str = NULL;
        guint rate = 0;
        /* Retrieve the stream's audio tags */
        g_signal_emit_by_name (m_playbin, "get-audio-tags", i, &tags);
        if (tags) {
            g_print ("audio stream %d:\n", i);
            if (gst_tag_list_get_string (tags, GST_TAG_AUDIO_CODEC, &str)) {
                g_print ("  codec: %s\n", str);
                g_free (str);
            }
            if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &str)) {
                g_print ("  language: %s\n", str);
                g_free (str);
            }
            if (gst_tag_list_get_uint (tags, GST_TAG_BITRATE, &rate)) {
                g_print ("  bitrate: %d\n", rate);
            }
            gst_tag_list_free (tags);
        }
    }

    for (gint i = 0; i < m_numText; i++) {
        GstTagList *tags = NULL;
        gchar *str = NULL;
        /* Retrieve the stream's subtitle tags */
        g_signal_emit_by_name (m_playbin, "get-text-tags", i, &tags);
        if (tags) {
            g_print ("subtitle stream %d:\n", i);
            if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &str)) {
                g_print ("  language: %s\n", str);
                g_free (str);
            }
            gst_tag_list_free (tags);
        }
    }

    g_object_get (GST_OBJECT(m_playbin), "current-video", &m_curVideo, NULL);
    g_object_get (GST_OBJECT(m_playbin), "current-audio", &m_curAudio, NULL);
    g_object_get (GST_OBJECT(m_playbin), "current-text", &m_curText, NULL);
}

gboolean CGstPlayback::HandleMessage(GstBus *bus, GstMessage *msg)
{
    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print ("End of stream");
            g_main_loop_quit (m_main_loop);
            break;
        case GST_MESSAGE_ERROR: 
        {
            gchar  *debug;
            GError *error;
            gst_message_parse_error (msg, &error, &debug);
            g_free (debug);
            g_printerr ("Error: %s", error->message);
            g_error_free (error);
            g_main_loop_quit (m_main_loop);
            break;
        }
        default:
        {
            gchar  *debug;
            GError *info;
            gst_message_parse_info (msg, &info, &debug);
            g_free (debug);
            g_print ("Info: %s", info->message);
            g_error_free (info);
            break;
        }
        break;
    }

    return TRUE;
}

} // namespace eau
