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
{}

status_t CMediaPlayer::setDataSource(const string &path, const vector<string> &headers)
{}

status_t CMediaPlayer::setDataSource(int fd, long offset, long length)
{}

void CMediaPlayer::setVideoSurfaceTexture(int texture)
{}

status_t CMediaPlayer::prepare()
{}

status_t CMediaPlayer::prepareAsync()
{}

status_t CMediaPlayer::start()
{}

status_t CMediaPlayer::stop()
{}

status_t CMediaPlayer::pause()
{}

bool CMediaPlayer::isPlaying()
{}

status_t CMediaPlayer::seekTo(int msec)
{}

int CMediaPlayer::getVideoWidth(int *width)
{}

int CMediaPlayer::getVideoHeight(int *height)
{}

status_t CMediaPlayer::getCurrentPosition(int *msec)
{}

status_t CMediaPlayer::getDuration(int *msec)
{}

status_t CMediaPlayer::reset()
{}

status_t CMediaPlayer::setAudioStreamType(int stype)
{}

status_t CMediaPlayer::setLooping(bool looping)
{}

bool CMediaPlayer::isLooping()
{}

status_t CMediaPlayer::setVolume(float leftVolume, float rightVolume)
{}

void CMediaPlayer::disconnect()
{}

status_t CMediaPlayer::setAudioSessionId(int sessionId)
{}

int CMediaPlayer::getAudioSessionId()
{}

status_t CMediaPlayer::setAuxEffectSendLevel(float level)
{}

status_t CMediaPlayer::attachAuxEffect(int effectId)
{}

int CMediaPlayer::setRetransmitEndpoint(const char *addr, unsigned short port)
{}

void CMediaPlayer::updateProxyConfig(const char *host, unsigned short port, const char *exclusionList)
{}

} // namespace eau
