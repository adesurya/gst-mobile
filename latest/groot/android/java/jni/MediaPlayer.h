#ifndef _MEDIA_PLAYER_H_
#define _MEDIA_PLAYER_H_

#include "Errors.h"
#include "refcount.h"

#include <string>
#include <vector>
using namespace std;

namespace eau
{

class CMediaPlayerListener : public RefCount
{

};
typedef RefCounted<CMediaPlayerListener> MediaPlayerListener;

class CMediaPlayer : public RefCount
{
public:
    CMediaPlayer();
    virtual ~CMediaPlayer();

    void setListener(MediaPlayerListener *listener);
    status_t setNextMediaPlayer(CMediaPlayer *player);

public:
    status_t setDataSource(const string &path, const vector<string> &headers);
    status_t setDataSource(int fd, long offset, long length);
    void setVideoSurfaceTexture(int texture);

    status_t prepare();
    status_t prepareAsync();
    status_t start();
    status_t stop();
    status_t pause();

    bool isPlaying();
    status_t seekTo(int msec);
    int getVideoWidth(int *width);
    int getVideoHeight(int *height);
    status_t getCurrentPosition(int *msec);
    status_t getDuration(int *msec);
    status_t reset();
    status_t setAudioStreamType(int stype);
    status_t setLooping(bool looping);
    bool isLooping();
    status_t setVolume(float leftVolume, float rightVolume);

    void disconnect();

    status_t setAudioSessionId(int sessionId);
    int getAudioSessionId();
    status_t setAuxEffectSendLevel(float level);
    status_t attachAuxEffect(int effectId);

    int setRetransmitEndpoint(const char *addr, unsigned short port);
    void updateProxyConfig(const char *host, unsigned short port, const char *exclusionList);
};
typedef RefCounted<CMediaPlayer> MediaPlayer;

} // namespace eau

#endif // _MEDIA_PLAYER_H_
