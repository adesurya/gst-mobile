//#define LOG_NDEBUG 0
#define LOG_TAG "MediaPlayer-JNI"

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include "JniConstants.h"
#include "JNIHelp.h"
#include "MediaPlayer.h"
#include "ALog-priv.h"
#include "mutex.h"
#include "zeroptr.h"

// ----------------------------------------------------------------------------

using namespace eau;

// ----------------------------------------------------------------------------

struct fields_t {
    jfieldID    context;
    jfieldID    surface_texture;

    jmethodID   post_event;

    jmethodID   proxyConfigGetHost;
    jmethodID   proxyConfigGetPort;
    jmethodID   proxyConfigGetExclusionList;
};
static fields_t fields;

static Mutex sLock;

static JavaVM *sJavaVM = NULL;

static JNIEnv* getJNIEnv()
{
    JNIEnv* env;
    JavaVM* vm = sJavaVM;
    assert(vm != NULL);
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
        return NULL;
    return env;
}

// ----------------------------------------------------------------------------
// ref-counted object for callbacks
class JNIMediaPlayerListener : public MediaPlayerListener
{
public:
    JNIMediaPlayerListener(JNIEnv* env, jobject thiz, jobject weak_thiz);
    ~JNIMediaPlayerListener();

private:
    JNIMediaPlayerListener();
    jclass      mClass;     // Reference to MediaPlayer class
    jobject     mObject;    // Weak ref to MediaPlayer Java object to call on
};

JNIMediaPlayerListener::JNIMediaPlayerListener(JNIEnv* env, jobject thiz, jobject weak_thiz)
{
    // Hold onto the MediaPlayer class for use in calling the static method
    // that posts events to the application thread.
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("Can't find k2/media/MediaPlayer");
        jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    mClass = (jclass)env->NewGlobalRef(clazz);

    // We use a weak reference so the MediaPlayer object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
    mObject = env->NewGlobalRef(weak_thiz);
}

JNIMediaPlayerListener::~JNIMediaPlayerListener()
{
    // remove global references
    JNIEnv *env = getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
}

// ----------------------------------------------------------------------------

static zeroptr<MediaPlayer> getMediaPlayer(JNIEnv* env, jobject thiz)
{
    Autolock l(sLock);
    MediaPlayer* const p = (MediaPlayer*)env->GetIntField(thiz, fields.context);
    return zeroptr<MediaPlayer>(p);
}

static zeroptr<MediaPlayer> setMediaPlayer(JNIEnv* env, jobject thiz, const zeroptr<MediaPlayer>& player)
{
    Autolock l(sLock);
    zeroptr<MediaPlayer> old = (MediaPlayer*)env->GetIntField(thiz, fields.context);
    if (player.get()) {
        player->AddRef(); // Add extra ref count
    }
    if (old != 0) {
        old->Release();
    }
    env->SetIntField(thiz, fields.context, (int)player.get());
    return old;
}

// If exception is NULL and opStatus is not OK, this method sends an error
// event to the client application; otherwise, if exception is not NULL and
// opStatus is not OK, this method throws the given exception to the client
// application.
static void process_media_player_call(JNIEnv *env, jobject thiz, status_t opStatus, const char* exception, const char *message)
{
    if (exception == NULL) {  // Don't throw exception. Instead, send an event.
        if (opStatus != (status_t) OK) {
            zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
            if (mp != 0) {
                //TODO
            }
        }
    } else {  // Throw exception!
        if ( opStatus == (status_t) INVALID_OPERATION ) {
            jniThrowException(env, "java/lang/IllegalStateException", NULL);
        } else if ( opStatus == (status_t) PERMISSION_DENIED ) {
            jniThrowException(env, "java/lang/SecurityException", NULL);
        } else if ( opStatus != (status_t) OK ) {
            if (strlen(message) > 230) {
               // if the message is too long, don't bother displaying the status code
               jniThrowException( env, exception, message);
            } else {
               char msg[256];
                // append the status code to the message
               sprintf(msg, "%s: status=0x%X", message, opStatus);
               jniThrowException( env, exception, msg);
            }
        }
    }
}

static void k2_media_MediaPlayer_setDataSourceAndHeaders(
        JNIEnv *env, jobject thiz, jstring path,
        jobjectArray keys, jobjectArray values) 
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    if (path == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    const char *tmp = env->GetStringUTFChars(path, NULL);
    if (tmp == NULL) {  // Out of memory
        return;
    }
    ALOGV("setDataSource: path %s", tmp);

    string pathStr(tmp);
    env->ReleaseStringUTFChars(path, tmp);
    tmp = NULL;

    // We build a KeyedVector out of the key and val arrays
    // TODO for keys and values
    vector<string> headersVector;
    status_t opStatus = mp->setDataSource(pathStr, headersVector);
    process_media_player_call(env, thiz, opStatus, "java/io/IOException", "setDataSource failed." );
}

static void k2_media_MediaPlayer_setDataSourceFD(JNIEnv *env, jobject thiz, jobject fileDescriptor, jlong offset, jlong length)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    if (fileDescriptor == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }
    int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
    ALOGV("setDataSourceFD: fd %d", fd);
    process_media_player_call(env, thiz, mp->setDataSource(fd, offset, length), "java/io/IOException", "setDataSourceFD failed." );
}

static int getVideoSurfaceTexture(JNIEnv *env, jobject thiz)
{
    return env->GetIntField(thiz, fields.surface_texture);
}

static void setVideoSurface(JNIEnv *env, jobject thiz, jobject jsurface, jboolean mediaPlayerMustBeAlive)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        if (mediaPlayerMustBeAlive) {
            jniThrowException(env, "java/lang/IllegalStateException", NULL);
        }
        return;
    }

    env->SetIntField(thiz, fields.surface_texture, (int)jsurface);

    // This will fail if the media player has not been initialized yet. This
    // can be the case if setDisplay() on MediaPlayer.java has been called
    // before setDataSource(). The redundant call to setVideoSurfaceTexture()
    // in prepare/prepareAsync covers for this case.
    mp->setVideoSurfaceTexture((int)jsurface);
}

static void k2_media_MediaPlayer_setVideoSurface(JNIEnv *env, jobject thiz, jobject jsurface)
{
    setVideoSurface(env, thiz, jsurface, true /* mediaPlayerMustBeAlive */);
}

static void k2_media_MediaPlayer_prepare(JNIEnv *env, jobject thiz)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    // Handle the case where the display surface was set before the mp was
    // initialized. We try again to make it stick.
    int texture = getVideoSurfaceTexture(env, thiz);
    mp->setVideoSurfaceTexture(texture);
    process_media_player_call( env, thiz, mp->prepare(), "java/io/IOException", "Prepare failed." );
}

static void k2_media_MediaPlayer_prepareAsync(JNIEnv *env, jobject thiz)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    // Handle the case where the display surface was set before the mp was
    // initialized. We try again to make it stick.
    int texture = getVideoSurfaceTexture(env, thiz);
    mp->setVideoSurfaceTexture(texture);
    process_media_player_call( env, thiz, mp->prepareAsync(), "java/io/IOException", "Prepare Async failed." );
}

static void k2_media_MediaPlayer_start(JNIEnv *env, jobject thiz)
{
    ALOGV("start");
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->start(), NULL, NULL );
}

static void k2_media_MediaPlayer_stop(JNIEnv *env, jobject thiz)
{
    ALOGV("stop");
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->stop(), NULL, NULL );
}

static void k2_media_MediaPlayer_pause(JNIEnv *env, jobject thiz)
{
    ALOGV("pause");
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->pause(), NULL, NULL );
}

static jboolean k2_media_MediaPlayer_isPlaying(JNIEnv *env, jobject thiz)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }
    const jboolean is_playing = mp->isPlaying();

    ALOGV("isPlaying: %d", is_playing);
    return is_playing;
}

static void k2_media_MediaPlayer_seekTo(JNIEnv *env, jobject thiz, int msec)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    ALOGV("seekTo: %d(msec)", msec);
    process_media_player_call( env, thiz, mp->seekTo(msec), NULL, NULL );
}

static int k2_media_MediaPlayer_getVideoWidth(JNIEnv *env, jobject thiz)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int w;
    if (0 != mp->getVideoWidth(&w)) {
        ALOGE("getVideoWidth failed");
        w = 0;
    }
    ALOGV("getVideoWidth: %d", w);
    return w;
}

static int k2_media_MediaPlayer_getVideoHeight(JNIEnv *env, jobject thiz)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int h;
    if (0 != mp->getVideoHeight(&h)) {
        ALOGE("getVideoHeight failed");
        h = 0;
    }
    ALOGV("getVideoHeight: %d", h);
    return h;
}

static int k2_media_MediaPlayer_getCurrentPosition(JNIEnv *env, jobject thiz)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int msec;
    process_media_player_call( env, thiz, mp->getCurrentPosition(&msec), NULL, NULL );
    ALOGV("getCurrentPosition: %d (msec)", msec);
    return msec;
}

static int k2_media_MediaPlayer_getDuration(JNIEnv *env, jobject thiz)
{
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int msec;
    process_media_player_call( env, thiz, mp->getDuration(&msec), NULL, NULL );
    ALOGV("getDuration: %d (msec)", msec);
    return msec;
}

static void k2_media_MediaPlayer_reset(JNIEnv *env, jobject thiz)
{
    ALOGV("reset");
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->reset(), NULL, NULL );
}

static void k2_media_MediaPlayer_setAudioStreamType(JNIEnv *env, jobject thiz, int streamtype)
{
    ALOGV("setAudioStreamType: %d", streamtype);
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setAudioStreamType(streamtype) , NULL, NULL );
}

static void k2_media_MediaPlayer_setLooping(JNIEnv *env, jobject thiz, jboolean looping)
{
    ALOGV("setLooping: %d", looping);
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setLooping(looping), NULL, NULL );
}

static jboolean k2_media_MediaPlayer_isLooping(JNIEnv *env, jobject thiz)
{
    ALOGV("isLooping");
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }
    return mp->isLooping();
}

static void k2_media_MediaPlayer_setVolume(JNIEnv *env, jobject thiz, float leftVolume, float rightVolume)
{
    ALOGV("setVolume: left %f  right %f", leftVolume, rightVolume);
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setVolume(leftVolume, rightVolume), NULL, NULL );
}

// This function gets some field IDs, which in turn causes class initialization.
// It is called from a static block in MediaPlayer, which won't run until the
// first time an instance of this class is used.
static void k2_media_MediaPlayer_native_init(JNIEnv *env)
{
    jclass clazz;
    clazz = env->FindClass("k2/media/MediaPlayer");
    if (clazz == NULL) {
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.context == NULL) {
        return;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");
    if (fields.post_event == NULL) {
        return;
    }

    fields.surface_texture = env->GetFieldID(clazz, "mNativeSurfaceTexture", "I");
    if (fields.surface_texture == NULL) {
        return;
    }
#if 0
    fields.proxyConfigGetHost = env->GetMethodID(clazz, "getHost", "()Ljava/lang/String;");
    fields.proxyConfigGetPort = env->GetMethodID(clazz, "getPort", "()I");
    fields.proxyConfigGetExclusionList = env->GetMethodID(clazz, "getExclusionList", "()Ljava/lang/String;");
#endif
}

static void k2_media_MediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    ALOGV("native_setup");
    zeroptr<MediaPlayer> mp = new MediaPlayer();
    if (mp == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    // create new listener and give it to MediaPlayer
    zeroptr<JNIMediaPlayerListener> listener = new JNIMediaPlayerListener(env, thiz, weak_this);
    mp->setListener(listener);

    // Stow our new C++ MediaPlayer in an opaque field in the Java object.
    setMediaPlayer(env, thiz, mp);
}

static void k2_media_MediaPlayer_release(JNIEnv *env, jobject thiz)
{
    ALOGV("release");
    zeroptr<MediaPlayer> mp = setMediaPlayer(env, thiz, 0);
    if (mp != NULL) {
        // this prevents native callbacks after the object is released
        mp->setListener(0);
        mp->disconnect();
    }
}

static void k2_media_MediaPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
    ALOGV("native_finalize");
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp != NULL) {
        ALOGW("MediaPlayer finalized without being released");
    }
    k2_media_MediaPlayer_release(env, thiz);
}

static void k2_media_MediaPlayer_set_audio_session_id(JNIEnv *env,  jobject thiz, jint sessionId) {
    ALOGV("set_session_id(): %d", sessionId);
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setAudioSessionId(sessionId), NULL, NULL );
}

static jint k2_media_MediaPlayer_get_audio_session_id(JNIEnv *env,  jobject thiz) {
    ALOGV("get_session_id()");
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    return mp->getAudioSessionId();
}

static void k2_media_MediaPlayer_setAuxEffectSendLevel(JNIEnv *env, jobject thiz, jfloat level)
{
    ALOGV("setAuxEffectSendLevel: level %f", level);
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setAuxEffectSendLevel(level), NULL, NULL );
}

static void k2_media_MediaPlayer_attachAuxEffect(JNIEnv *env,  jobject thiz, jint effectId) {
    ALOGV("attachAuxEffect(): %d", effectId);
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->attachAuxEffect(effectId), NULL, NULL );
}

static jint k2_media_MediaPlayer_setRetransmitEndpoint(JNIEnv *env, jobject thiz,
                                                jstring addrString, jint port) {
    zeroptr<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return INVALID_OPERATION;
    }

    const char *cAddrString = NULL;
    if (NULL != addrString) {
        cAddrString = env->GetStringUTFChars(addrString, NULL);
        if (cAddrString == NULL) {  // Out of memory
            return NO_MEMORY;
        }
    }
    ALOGV("setRetransmitEndpoint: %s:%d", cAddrString ? cAddrString : "(null)", port);

    status_t ret;
    if (cAddrString && (port > 0xFFFF)) {
        ret = BAD_VALUE;
    } else {
        ret = mp->setRetransmitEndpoint(cAddrString,
                static_cast<uint16_t>(port));
    }

    if (NULL != addrString) {
        env->ReleaseStringUTFChars(addrString, cAddrString);
    }

    if (ret == INVALID_OPERATION ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
    }

    return ret;
}

static void k2_media_MediaPlayer_setNextMediaPlayer(JNIEnv *env, jobject thiz, jobject java_player)
{
    ALOGV("setNextMediaPlayer");
    zeroptr<MediaPlayer> thisplayer = getMediaPlayer(env, thiz);
    if (thisplayer == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", "This player not initialized");
        return;
    }
    zeroptr<MediaPlayer> nextplayer = (java_player == NULL) ? NULL : getMediaPlayer(env, java_player);
    if (nextplayer == NULL && java_player != NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", "That player not initialized");
        return;
    }

    if (nextplayer == thisplayer) {
        jniThrowException(env, "java/lang/IllegalArgumentException", "Next player can't be self");
        return;
    }
    // tie the two players together
    process_media_player_call(
            env, thiz, thisplayer->setNextMediaPlayer(nextplayer),
            "java/lang/IllegalArgumentException",
            "setNextMediaPlayer failed." );
    ;
}

static void k2_media_MediaPlayer_updateProxyConfig(JNIEnv *env, jobject thiz, jobject proxyProps)
{
    ALOGV("updateProxyConfig");
    zeroptr<MediaPlayer> thisplayer = getMediaPlayer(env, thiz);
    if (thisplayer == NULL) {
        return;
    }

    if (proxyProps == NULL) {
        thisplayer->updateProxyConfig(NULL /*host*/, 0 /*port*/, NULL /*exclusionList*/);
    } else {
        jstring hostObj = (jstring)env->CallObjectMethod(proxyProps, fields.proxyConfigGetHost);
        const char *host = env->GetStringUTFChars(hostObj, NULL);
        int port = env->CallIntMethod(proxyProps, fields.proxyConfigGetPort);
        jstring exclusionListObj = (jstring)env->CallObjectMethod(
                proxyProps, fields.proxyConfigGetExclusionList);
        const char *exclusionList = env->GetStringUTFChars(exclusionListObj, NULL);

        if (host != NULL && exclusionListObj != NULL) {
            thisplayer->updateProxyConfig(host, port, exclusionList);
        }

        if (exclusionList != NULL) {
            env->ReleaseStringUTFChars(exclusionListObj, exclusionList);
            exclusionList = NULL;
        }

        if (host != NULL) {
            env->ReleaseStringUTFChars(hostObj, host);
            host = NULL;
        }
    }
}

// ----------------------------------------------------------------------------

static JNINativeMethod gMethods[] = {
    {
        "_setDataSource",
        "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
        (void *)k2_media_MediaPlayer_setDataSourceAndHeaders
    },

    {"_setDataSource",       "(Ljava/io/FileDescriptor;JJ)V",    (void *)k2_media_MediaPlayer_setDataSourceFD},
    {"_setVideoSurface",    "(Landroid/view/Surface;)V",        (void *)k2_media_MediaPlayer_setVideoSurface},
    {"prepare",             "()V",                              (void *)k2_media_MediaPlayer_prepare},
    {"prepareAsync",        "()V",                              (void *)k2_media_MediaPlayer_prepareAsync},
    {"_start",              "()V",                              (void *)k2_media_MediaPlayer_start},
    {"_stop",               "()V",                              (void *)k2_media_MediaPlayer_stop},
    {"getVideoWidth",       "()I",                              (void *)k2_media_MediaPlayer_getVideoWidth},
    {"getVideoHeight",      "()I",                              (void *)k2_media_MediaPlayer_getVideoHeight},
    {"seekTo",              "(I)V",                             (void *)k2_media_MediaPlayer_seekTo},
    {"_pause",              "()V",                              (void *)k2_media_MediaPlayer_pause},
    {"isPlaying",           "()Z",                              (void *)k2_media_MediaPlayer_isPlaying},
    {"getCurrentPosition",  "()I",                              (void *)k2_media_MediaPlayer_getCurrentPosition},
    {"getDuration",         "()I",                              (void *)k2_media_MediaPlayer_getDuration},
    {"_release",            "()V",                              (void *)k2_media_MediaPlayer_release},
    {"_reset",              "()V",                              (void *)k2_media_MediaPlayer_reset},
    {"setAudioStreamType",  "(I)V",                             (void *)k2_media_MediaPlayer_setAudioStreamType},
    {"setLooping",          "(Z)V",                             (void *)k2_media_MediaPlayer_setLooping},
    {"isLooping",           "()Z",                              (void *)k2_media_MediaPlayer_isLooping},
    {"setVolume",           "(FF)V",                            (void *)k2_media_MediaPlayer_setVolume},
    {"native_init",         "()V",                              (void *)k2_media_MediaPlayer_native_init},
    {"native_setup",        "(Ljava/lang/Object;)V",            (void *)k2_media_MediaPlayer_native_setup},
    {"native_finalize",     "()V",                              (void *)k2_media_MediaPlayer_native_finalize},
    {"getAudioSessionId",   "()I",                              (void *)k2_media_MediaPlayer_get_audio_session_id},
    {"setAudioSessionId",   "(I)V",                             (void *)k2_media_MediaPlayer_set_audio_session_id},
    {"setAuxEffectSendLevel", "(F)V",                           (void *)k2_media_MediaPlayer_setAuxEffectSendLevel},
    {"attachAuxEffect",     "(I)V",                             (void *)k2_media_MediaPlayer_attachAuxEffect},
    {"native_setRetransmitEndpoint", "(Ljava/lang/String;I)I",  (void *)k2_media_MediaPlayer_setRetransmitEndpoint},
    {"setNextMediaPlayer",  "(Lk2/media/MediaPlayer;)V",   (void *)k2_media_MediaPlayer_setNextMediaPlayer},
};

static const char* const kClassPathName = "k2/media/MediaPlayer";

// This function only registers the native methods
static int register_k2_media_MediaPlayer(JNIEnv *env)
{
    return jniRegisterNativeMethods(env, kClassPathName, gMethods, NELEM(gMethods));
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_k2_media_MediaPlayer(env) < 0) {
        ALOGE("ERROR: MediaPlayer native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

    JniConstants::init(env);

bail:
    return result;
}

