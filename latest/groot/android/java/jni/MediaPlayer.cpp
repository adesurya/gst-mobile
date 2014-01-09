#include "MediaPlayer.h"

namespace eau
{

CMediaPlayer::CMediaPlayer()
{}

CMediaPlayer::~CMediaPlayer()
{}

void CMediaPlayer::setListener(MediaPlayerListener *listener)
{}

status_t CMediaPlayer::setNextMediaPlayer(CMediaPlayer *player)
{
    return OK;
}

status_t CMediaPlayer::setDataSource(const string &path, const vector<string> &headers)
{
    return OK;
}

status_t CMediaPlayer::setDataSource(int fd, long offset, long length)
{
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
    return OK;
}

status_t CMediaPlayer::stop()
{
    return OK;
}

status_t CMediaPlayer::pause()
{
    return OK;
}

bool CMediaPlayer::isPlaying()
{
    return false;
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
