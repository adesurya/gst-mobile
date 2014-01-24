#include "MediaPlayer.h"
#include "gstapi.h"
#include "ALog-priv.h"

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

static void init_glib_log_print()
{
    gint level = 0xff;
    g_set_print_handler(print_func); // g_print
    g_set_printerr_handler(print_func); // g_printerr
    g_log_set_handler("k2player", (GLogLevelFlags)level, log_func, NULL);
    g_log_set_default_handler(log_func, NULL);
}


CMediaPlayer::CMediaPlayer()
{
    m_pListener = NULL;
    m_bPlaying = false;

    init_glib_log_print();
    m_pPlayer = new GstPlayback();
}

CMediaPlayer::~CMediaPlayer()
{
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

void CMediaPlayer::setVideoSurfaceTexture(int texture)
{
}

status_t CMediaPlayer::prepare()
{
    ALOGI("%s, begin", __func__);
    return OK;
}

status_t CMediaPlayer::prepareAsync()
{
    return OK;
}

status_t CMediaPlayer::start()
{
    returnv_assert(!m_szPath.empty(), BAD_VALUE);

    ALOGI("%s, begin", __func__);

    int argc = 2;
    char *argv[2] = {NULL, NULL};
    argv[0] = (char *)"k2player";
    argv[1] = (char *)m_szPath.c_str();
    m_bPlaying = true;
    g_print("start with ogg_player\n");
    ogg_player(argc, argv);
    //playbin2_player(argc, argv);
    //m_pPlayer->Init(argc, argv);
    //m_pPlayer->Play();
    m_bPlaying = false;
    return OK;
}

status_t CMediaPlayer::stop()
{
    m_bPlaying = false;
    return OK;
}

status_t CMediaPlayer::pause()
{
    return OK;
}

bool CMediaPlayer::isPlaying()
{
    return m_bPlaying;
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

} // namespace eau
