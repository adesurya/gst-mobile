#ifndef _GSTAPI_H_
#define _GSTAPI_H_

#include <gst/gst.h>

#include "refcount.h"
#include "zeroptr.h"

int ogg_player (const char *location);
int playbin2_player(const char *location);

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

    bool Init();
    bool SetOption();
    bool SetUri(const char *uri);
    bool SetWindow(void *window);
    bool Prepare();
    bool Play();
    bool Pause();
    bool Stop();
    void Uninit();

protected:
    static gboolean handle_message (GstBus *bus, GstMessage *msg, void *data);
    gboolean HandleMessage(GstBus *bus, GstMessage *msg);
    void AnalyzeStreams();

private:
    GstElement  *m_playbin;
    GstElement  *m_audio_sink;
    GstElement  *m_video_sink;
    GMainLoop   *m_main_loop;
    GThread     *m_bus_msg_thread;

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
