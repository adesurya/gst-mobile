#ifndef _MEDIA_PLAYER_H_
#define _MEDIA_PLAYER_H_

#include <string>
#include <vector>
using namespace std;

#define INVALID_OPERATION -1
#define NO_MEMORY -2
#define BAD_VALUE -3

enum status_t {
    OK = 0,
};

enum audio_stream_type_t {
};

class MediaPlayerListener
{

};

class MediaPlayer : RefCounted<MediaPlayer>
{
public:
    MediaPlayer();
    virtual ~MediaPlayer();

    void setListener(MediaPlayerListener *listener);
    status_t setNextMediaPlayer(MediaPlayer *player);

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
    int getVideoheight(int *height);
    status_t getCurrentPosition(int *msec);
    status_t getDuration(int *msec);
    status_t reset(int *msec);
    status_t setAudioStreamType(audio_stream_type_t stype);
    status_t setLooping(bool looping);
    bool isLooping();
    status_t setVolume(float leftVolume, float rightVolume);

    void disconnect();

    status_t setAudioSessionId(int sessionId);
    int setAudioSessionId();
    status_t setAuxEffectSendLevel(float level);
    status_t attachAuxEffect(int effectId);

    int setRetransmitEndpoint(const char *addr, unsigned short port);
    void updateProxyConfig(const char *host, unsigned short port, const char *exclusionList);
};

#endif // _MEDIA_PLAYER_H_
