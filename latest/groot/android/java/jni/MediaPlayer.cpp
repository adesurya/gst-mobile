#include "MediaPlayer.h"
#include "gstapi.h"
#include "ALog-priv.h"

extern "C" void gst_static_plugins(void);
extern GstDebugLevel _gst_debug_min;

namespace eau
{

static void print_func(const gchar *string)
{
    ALOGI("%s", string);
}

static void log_func(const gchar   *log_domain,
        GLogLevelFlags log_level,
        const gchar   *message,
        gpointer       user_data)
{
    switch(log_level) {
    case G_LOG_LEVEL_ERROR:
    case G_LOG_LEVEL_CRITICAL:
        ALOGE("%s", message);
        break;
    case G_LOG_LEVEL_WARNING:
        ALOGW("%s", message);
        break;
    case G_LOG_LEVEL_MESSAGE:
    case G_LOG_LEVEL_INFO:
        ALOGI("%s", message);
        break;
    case G_LOG_LEVEL_DEBUG:
    default:
        ALOGD("%s", message);
        break;
    }
}

static void init_gst()
{
    static bool gst_inited = false;
    if (gst_inited)
        return;

    ALOGI("%s, %lu, begin", __func__, time(0));
    gint level = 0xff;
    g_set_print_handler(print_func); // g_print
    g_set_printerr_handler(print_func); // g_printerr
    g_log_set_handler("k2player", (GLogLevelFlags)level, log_func, NULL);
    g_log_set_default_handler(log_func, NULL);

    if (!g_thread_supported ()) {
        g_thread_init (NULL);
    }

    gst_debug_set_active(true);
#if 0
    GST_DEBUG_CATEGORY_STATIC (my_category);// define category (statically)
    #define GST_CAT_DEFAULT my_category     // set as default
    GST_DEBUG_CATEGORY_INIT (my_category, "my category", 0, "This is my very own");
    gst_debug_category_set_threshold(my_category, GST_LEVEL_TRACE);
#endif

    ALOGI("%s, %lu, gst_init", __func__, time(0));
    int argc = 1;
    char *argvs[] = {(char *)"k2player", NULL};
    char **argv = argvs;
    gst_init (&argc, &argv);

    ALOGI("%s, %lu, gst_static_init", __func__, time(0));
    gst_static_plugins();

    gst_inited = true;
    ALOGI("%s, %lu, end", __func__, time(0));
}


CMediaPlayer::CMediaPlayer()
{
    ALOGI("%s, begin", __func__);
    m_fd = -1;
    m_pListener = NULL;
    m_pPlayer = NULL;
    m_pTexture = NULL;
    m_state = GST_STATE_NULL;

    init_gst();
}

CMediaPlayer::~CMediaPlayer()
{
    ALOGI("%s, begin", __func__);
    m_pListener = NULL;
    m_pPlayer = NULL;
    m_pTexture = NULL;
}

void CMediaPlayer::setListener(zeroptr<MediaPlayerListener> listener)
{
    if (listener.get()) {
        listener->AddRef();
    }
    m_pListener = listener;
}

status_t CMediaPlayer::setNextMediaPlayer(zeroptr<CMediaPlayer> player)
{
    return OK;
}

status_t CMediaPlayer::setDataSource(const string &path, const vector<string> &headers)
{
    ALOGI("%s, begin, path: %s", __func__, path.c_str());
    m_szPath = path;
    return OK;
}

status_t CMediaPlayer::setDataSource(int fd, long offset, long length)
{
    m_fd = fd;
    return OK;
}

void CMediaPlayer::setVideoSurfaceTexture(void * texture)
{
    m_pTexture = texture;
}

status_t CMediaPlayer::prepare()
{
    returnv_assert(m_state != GST_STATE_PLAYING, OK);

    if (!m_pPlayer.get()) {
        m_pPlayer = new GstPlayback();
        m_pPlayer->Init();
        m_pPlayer->SetOption();
    }

    m_pPlayer->SetUri(m_szPath.c_str());
    if (m_pTexture) {
        m_pPlayer->Prepare();
        m_pPlayer->SetWindow(m_pTexture);
    }

    return OK;
}

status_t CMediaPlayer::prepareAsync()
{
    return OK;
}

status_t CMediaPlayer::start()
{
    returnv_assert(m_state != GST_STATE_PLAYING, OK);
    m_pPlayer->Play();
    return OK;
}

status_t CMediaPlayer::stop()
{
    m_pPlayer->Stop();
    return OK;
}

status_t CMediaPlayer::pause()
{
    m_pPlayer->Pause();
    return OK;
}

bool CMediaPlayer::isPlaying()
{
    return (m_state == GST_STATE_PLAYING);
}

status_t CMediaPlayer::seekTo(int msec)
{
    return OK;
}

int CMediaPlayer::getVideoWidth(int *width)
{
    return 0;
}

int CMediaPlayer::getVideoHeight(int *height)
{
    return 0;
}

status_t CMediaPlayer::getCurrentPosition(int *msec)
{
    return OK;
}

status_t CMediaPlayer::getDuration(int *msec)
{
    return OK;
}

status_t CMediaPlayer::reset()
{
    return OK;
}

status_t CMediaPlayer::setAudioStreamType(int stype)
{
    return OK;
}

status_t CMediaPlayer::setLooping(bool looping)
{
    return OK;
}

bool CMediaPlayer::isLooping()
{
    return false;
}

status_t CMediaPlayer::setVolume(float leftVolume, float rightVolume)
{
    return OK;
}

void CMediaPlayer::disconnect()
{}

status_t CMediaPlayer::setAudioSessionId(int sessionId)
{
    return OK;
}

int CMediaPlayer::getAudioSessionId()
{
    return 0;
}

status_t CMediaPlayer::setAuxEffectSendLevel(float level)
{
    return OK;
}

status_t CMediaPlayer::attachAuxEffect(int effectId)
{
    return OK;
}

int CMediaPlayer::setRetransmitEndpoint(const char *addr, unsigned short port)
{
    return 0;
}

void CMediaPlayer::updateProxyConfig(const char *host, unsigned short port, const char *exclusionList)
{}

void CMediaPlayer::onPlayState(GstState state)
{
    m_state = state;
    switch(state) {
    case GST_STATE_READY:
        break;
    case GST_STATE_PAUSED:
        break;
    case GST_STATE_PLAYING:
        break;
    }
}

void CMediaPlayer::onCompletion()
{
    m_state = GST_STATE_NULL;
}

void CMediaPlayer::onBufferingUpdate(int percent)
{}

void CMediaPlayer::onSeekComplete()
{}

void CMediaPlayer::onVideoSizeChanged(int width, int height)
{}

void CMediaPlayer::onError(int code, const char *extra)
{}

void CMediaPlayer::onInfo(int code, const char *extra)
{}

} // namespace eau
