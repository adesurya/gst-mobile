#ifndef _GSTAPI_H_
#define _GSTAPI_H_

#include <gst/gst.h>

#include "refcount.h"
#include "zeroptr.h"

int ogg_player (int argc, char *argv[]);
int playbin2_player(int argc, char *argv[]);

namespace eau
{

typedef enum {
    GST_PLAY_FLAG_VIDEO         = (1 << 0), /* We want video output */
    GST_PLAY_FLAG_AUDIO         = (1 << 1), /* We want audio output */
    GST_PLAY_FLAG_TEXT          = (1 << 2)  /* We want subtitle output */
} GstPlayFlags;

class CGstPlayback : public RefCount
{
public:
    CGstPlayback();
    virtual ~CGstPlayback();

    bool Init(int argc, char *argv[]);
    bool Play();
    bool Pause();
    void Uninit();

protected:
    static gboolean handle_message (GstBus *bus, GstMessage *msg, void *data);
    gboolean HandleMessage(GstBus *bus, GstMessage *msg);
    void AnalyzeStreams();

private:
    GMainLoop *m_main_loop;
    GstElement *m_playbin2;
    GstBus *m_bus;

    gchar m_uri[1024];
    gint m_flags;
    GstStateChangeReturn m_state;

    gint m_numVideo; 
    gint m_numAudio;
    gint m_numText;

    gint m_curVideo;
    gint m_curAudio;
    gint m_curText;
};
typedef RefCounted<CGstPlayback> GstPlayback;

} // namespace eau

#endif
