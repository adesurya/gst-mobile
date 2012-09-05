/*
 * This example shows how to use interfaces and the tag subsystem.
 * It takes an mp3 file as input, and makes an ogg file out of it. While doing
 * this, it parses the filename and sets artist and title in the ogg file.
 * It assumes the filename to be "<artist> - <title>.mp3"
 *
 * Run the program as "retag <mp3 file>"
 *
 * To run this program, you need to have the gst-plugins package (specifically
 * the vorbis and mad plugins) installed.
 */

/* main header */
#include <gst/gst.h>
/* and a header we need for the string manipulation */
#include <string.h>

int
main (int argc, char *argv[])
{
  GstElement *bin, *filesrc, *tag_changer, *filesink;
  gchar *artist, *title, *ext, *filename;

  /* initialize GStreamer */
  gst_init (&argc, &argv);

  /* check that the argument is there */
  if (argc != 2) {
    g_print ("usage: %s <mp3 file>\n", argv[0]);
    return 1;
  }

  /* parse the mp3 name */
  artist = strrchr (argv[1], '/');
  if (artist == NULL)
    artist = argv[1];
  artist = g_strdup (artist);
  ext = strrchr (artist, '.');
  if (ext)
    *ext = '\0';
  title = strstr (artist, " - ");
  if (title == NULL) {
    g_print ("The format of the mp3 file is invalid.\n");
    return 1;
  }
  *title = '\0';
  title += 3;


  /* create a new bin to hold the elements */
  bin = gst_pipeline_new ("pipeline");
  g_assert (bin);

  /* create a file reader */
  filesrc = gst_element_factory_make ("filesrc", "disk_source");
  g_assert (filesrc);

  /* now it's time to get the tag_changer */
  tag_changer = gst_element_factory_make ("id3tag", "tag_changer");
  if (!tag_changer) {
    g_print ("could not find plugin \"mad\"");
    return 1;
  }

  /* and a file writer */
  filesink = gst_element_factory_make ("filesink", "filesink");
  g_assert (filesink);

  /* set the filenames */
  filename = g_strdup_printf ("%s.temp", argv[1]);      /* easy solution */
  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);
  g_object_set (G_OBJECT (filesink), "location", filename, NULL);

  /* make sure the tag setter uses our stuff
     (though that should already be default) */
  gst_tag_setter_set_merge_mode (GST_TAG_SETTER (tag_changer),
      GST_TAG_MERGE_KEEP);
  /* set the tagging information */
  gst_tag_setter_add (GST_TAG_SETTER (tag_changer), GST_TAG_MERGE_REPLACE,
      GST_TAG_ARTIST, artist, GST_TAG_TITLE, title, NULL);

  /* add objects to the main pipeline */
  gst_bin_add_many (GST_BIN (bin), filesrc, tag_changer, filesink, NULL);

  /* link the elements */
  if (!gst_element_link_many (filesrc, tag_changer, filesink, NULL))
    g_assert_not_reached ();

  /* start playing */
  gst_element_set_state (bin, GST_STATE_PLAYING);

  while (gst_bin_iterate (GST_BIN (bin)));

  /* stop the bin */
  gst_element_set_state (bin, GST_STATE_NULL);

  /* rename the file to the correct name and remove the old one */
  remove (argv[1]);
  rename (filename, argv[1]);
  g_free (filename);

  return 0;
}
