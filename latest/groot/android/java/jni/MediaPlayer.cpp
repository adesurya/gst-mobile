#include "MediaPlayer.h"
#include "gstapi.h"

namespace eau
{

CMediaPlayer::CMediaPlayer()
{
    m_pListener = NULL;
    m_bPlaying = false;
}

CMediaPlayer::~CMediaPlayer()
{}

void CMediaPlayer::setListener(zeroptr<MediaPlayerListener> listener)
{
    m_pListener = listener;
}

status_t CMediaPlayer::setNextMediaPlayer(zeroptr<CMediaPlayer> player)
{
    return OK;
}

status_t CMediaPlayer::setDataSource(const string &path, const vector<string> &headers)
{
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
    return OK;
}

status_t CMediaPlayer::prepareAsync()
{
    return OK;
}

status_t CMediaPlayer::start()
{
    int argc = 2;
    char *argv[2] = {NULL, NULL};
    argv[0] = (char *)"k2player";
    argv[1] = (char *)m_szPath.c_str();
    m_bPlaying = true;
    playbin2_player(argc, argv);
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
