#include <gst/gst.h>
#include <glib.h>


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


int ogg_player (int argc, char *argv[]) {

    GMainLoop *loop;
    GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
    GstBus *bus;
    guint bus_watch_id;

    /* Initialisation */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    /* Check input arguments */
    if (argc != 2) {
        g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
        return -1;
    }

    /* Create gstreamer elements */
    pipeline = gst_pipeline_new ("audio-player");
    source   = gst_element_factory_make ("filesrc",       "file-source");
    demuxer  = gst_element_factory_make ("oggdemux",      "ogg-demuxer");
    decoder  = gst_element_factory_make ("vorbisdec",     "vorbis-decoder");
    conv     = gst_element_factory_make ("audioconvert",  "converter");
    sink     = gst_element_factory_make ("autoaudiosink", "audio-output");

    if (!pipeline || !source || !demuxer || !decoder || !conv || !sink) {
        g_printerr ("One element could not be created. Exiting.\n");
        return -1;
    }

    /* Set up the pipeline */

    /* we set the input filename to the source element */
    g_object_set (G_OBJECT (source), "location", argv[1], NULL);

    /* we add a message handler */
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
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
    g_print ("Now playing: %s\n", argv[1]);
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


int playbin2_player(int argc, char *argv[]) {

    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    gchar uri[512] = {0};
    if (argc != 2) {
        g_print ("playbin2 [options] uri\n");
        return -1;
    }
    /* playbin2 uri=http://docs.gstreamer.com/media/sintel_trailer-480p.webm */
    g_snprintf(uri, sizeof(uri), "playbin2 uri=%s", argv[1]);

    /* Initialize GStreamer */
    gst_init (&argc, &argv);

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

