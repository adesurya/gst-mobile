package k2.media;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.AssetFileDescriptor;
import android.net.Proxy;
import android.net.ProxyProperties;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.PowerManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.Runnable;
import java.net.InetSocketAddress;
import java.util.Map;
import java.util.Scanner;
import java.util.Set;
import java.util.Vector;
import java.lang.ref.WeakReference;

/**
 * MediaPlayer can be used to control playback of audio/video files and streams. 
 */
public class MediaPlayer
{
    static {
        System.loadLibrary("k2player");
        native_init();
    }

    private final static String TAG = "MediaPlayer";

    private int mNativeContext; // accessed by native methods
    private int mNativeSurfaceTexture;  // accessed by native methods
    private int mListenerContext; // accessed by native methods
    private SurfaceHolder mSurfaceHolder;
    private EventHandler mEventHandler;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    /// -------------------------------------------------------------

    /**
     * All native methods 
     */ 
    private native void _setVideoSurface(Surface surface);
    private native void _setDataSource(String path, String[] keys, String[] values)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException;
    private native void _setDataSource(FileDescriptor fd, long offset, long length)
            throws IOException, IllegalArgumentException, IllegalStateException;

    public native void prepare() throws IOException, IllegalStateException;
    public native void prepareAsync() throws IllegalStateException;

    private native void _start() throws IllegalStateException;
    private native void _stop() throws IllegalStateException;
    private native void _pause() throws IllegalStateException;
    private native void _reset();
    private native void _release();

    public native int getVideoWidth();
    public native int getVideoHeight();
    public native boolean isPlaying();
    public native void seekTo(int msec) throws IllegalStateException;
    public native int getCurrentPosition();
    public native int getDuration();
    public native void setNextMediaPlayer(MediaPlayer next);
    public native void setAudioStreamType(int streamtype);
    public native void setLooping(boolean looping);
    public native boolean isLooping();
    public native void setVolume(float leftVolume, float rightVolume);
    public native void setAudioSessionId(int sessionId)  
            throws IllegalArgumentException, IllegalStateException;
    public native int getAudioSessionId();
    public native void attachAuxEffect(int effectId);
    public native void setAuxEffectSendLevel(float level);

    private native void updateProxyConfig(ProxyProperties props);
    private static native final void native_init();
    private native final void native_setup(Object mediaplayer_this);
    private native final void native_finalize();
    private native final int native_setRetransmitEndpoint(String addrString, int port);

    /// -------------------------------------------------------------

    /**
     * Default constructor. Consider using one of the create() methods for
     * synchronously instantiating a MediaPlayer from a Uri or resource.
     * <p>When done with the MediaPlayer, you should call  {@link #release()},
     * to free the resources. If not released, too many MediaPlayer instances may
     * result in an exception.</p>
     */
    public MediaPlayer() {
    }

    /**
     * Sets the {@link SurfaceHolder} to use for displaying the video
     * portion of the media.
     *
     * Either a surface holder or surface must be set if a display or video sink
     * is needed.  Not calling this method or {@link #setSurface(Surface)}
     * when playing back a video will result in only the audio track being played.
     * A null surface holder or surface will result in only the audio track being
     * played.
     *
     * @param sh the SurfaceHolder to use for video display
     */
    public void setDisplay(SurfaceHolder sh) {
        mSurfaceHolder = sh;
        Surface surface;
        if (sh != null) {
            surface = sh.getSurface();
        } else {
            surface = null;
        }
        _setVideoSurface(surface);
        updateSurfaceScreenOn();
    }

    /**
     * Sets the {@link Surface} to be used as the sink for the video portion of
     * the media. This is similar to {@link #setDisplay(SurfaceHolder)}, but
     * does not support {@link #setScreenOnWhilePlaying(boolean)}.  Setting a
     * Surface will un-set any Surface or SurfaceHolder that was previously set.
     * A null surface will result in only the audio track being played.
     *
     * If the Surface sends frames to a {@link SurfaceTexture}, the timestamps
     * returned from {@link SurfaceTexture#getTimestamp()} will have an
     * unspecified zero point.  These timestamps cannot be directly compared
     * between different media sources, different instances of the same media
     * source, or multiple runs of the same program.  The timestamp is normally
     * monotonically increasing and is unaffected by time-of-day adjustments,
     * but it is reset when the position is set.
     *
     * @param surface The {@link Surface} to be used for the video portion of
     * the media.
     */
    public void setSurface(Surface surface) {
        if (mScreenOnWhilePlaying && surface != null) {
            Log.w(TAG, "setScreenOnWhilePlaying(true) is ineffective for Surface");
        }
        mSurfaceHolder = null;
        _setVideoSurface(surface);
        updateSurfaceScreenOn();
    }

    /* Do not change these video scaling mode values below without updating
     * their counterparts in system/window.h! Please do not forget to update
     * {@link #isVideoScalingModeSupported} when new video scaling modes
     * are added.
     */
    /**
     * Specifies a video scaling mode. The content is stretched to the
     * surface rendering area. When the surface has the same aspect ratio
     * as the content, the aspect ratio of the content is maintained;
     * otherwise, the aspect ratio of the content is not maintained when video
     * is being rendered. Unlike {@link #VIDEO_SCALING_MODE_SCALE_TO_FIT_WITH_CROPPING},
     * there is no content cropping with this video scaling mode.
     */
    public static final int VIDEO_SCALING_MODE_SCALE_TO_FIT = 1;

    /**
     * Specifies a video scaling mode. The content is scaled, maintaining
     * its aspect ratio. The whole surface area is always used. When the
     * aspect ratio of the content is the same as the surface, no content
     * is cropped; otherwise, content is cropped to fit the surface.
     */
    public static final int VIDEO_SCALING_MODE_SCALE_TO_FIT_WITH_CROPPING = 2;
    /**
     * Sets video scaling mode. To make the target video scaling mode
     * effective during playback, this method must be called after
     * data source is set. If not called, the default video
     * scaling mode is {@link #VIDEO_SCALING_MODE_SCALE_TO_FIT}.
     *
     * <p> The supported video scaling modes are:
     * <ul>
     * <li> {@link #VIDEO_SCALING_MODE_SCALE_TO_FIT}
     * <li> {@link #VIDEO_SCALING_MODE_SCALE_TO_FIT_WITH_CROPPING}
     * </ul>
     *
     * @param mode target video scaling mode. Most be one of the supported
     * video scaling modes; otherwise, IllegalArgumentException will be thrown.
     *
     * @see MediaPlayer#VIDEO_SCALING_MODE_SCALE_TO_FIT
     * @see MediaPlayer#VIDEO_SCALING_MODE_SCALE_TO_FIT_WITH_CROPPING
     */
    public void setVideoScalingMode(int mode) {
        //TODO
    }

    /**
     * Convenience method to create a MediaPlayer for a given Uri.
     * On success, {@link #prepare()} will already have been called and must not be called again.
     * <p>When done with the MediaPlayer, you should call  {@link #release()},
     * to free the resources. If not released, too many MediaPlayer instances will
     * result in an exception.</p>
     *
     * @param context the Context to use
     * @param uri the Uri from which to get the datasource
     * @return a MediaPlayer object, or null if creation failed
     */
    public static MediaPlayer create(Context context, Uri uri) {
        return create (context, uri, null);
    }

    /**
     * Convenience method to create a MediaPlayer for a given Uri.
     * On success, {@link #prepare()} will already have been called and must not be called again.
     * <p>When done with the MediaPlayer, you should call  {@link #release()},
     * to free the resources. If not released, too many MediaPlayer instances will
     * result in an exception.</p>
     *
     * @param context the Context to use
     * @param uri the Uri from which to get the datasource
     * @param holder the SurfaceHolder to use for displaying the video
     * @return a MediaPlayer object, or null if creation failed
     */
    public static MediaPlayer create(Context context, Uri uri, SurfaceHolder holder) {
        try {
            MediaPlayer mp = new MediaPlayer();
            mp.setDataSource(context, uri);
            if (holder != null) {
                mp.setDisplay(holder);
            }
            mp.prepare();
            return mp;
        } catch (IOException ex) {
            Log.d(TAG, "create failed:", ex);
        } catch (IllegalArgumentException ex) {
            Log.d(TAG, "create failed:", ex);
        } catch (SecurityException ex) {
            Log.d(TAG, "create failed:", ex);
        }

        return null;
    }

    // Note no convenience method to create a MediaPlayer with SurfaceTexture sink.

    /**
     * Convenience method to create a MediaPlayer for a given resource id.
     * On success, {@link #prepare()} will already have been called and must not be called again.
     * <p>When done with the MediaPlayer, you should call  {@link #release()},
     * to free the resources. If not released, too many MediaPlayer instances will
     * result in an exception.</p>
     *
     * @param context the Context to use
     * @param resid the raw resource id (<var>R.raw.&lt;something></var>) for
     *              the resource to use as the datasource
     * @return a MediaPlayer object, or null if creation failed
     */
    public static MediaPlayer create(Context context, int resid) {
        try {
            AssetFileDescriptor afd = context.getResources().openRawResourceFd(resid);
            if (afd == null) return null;

            MediaPlayer mp = new MediaPlayer();
            mp.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
            afd.close();
            mp.prepare();
            return mp;
        } catch (IOException ex) {
            Log.d(TAG, "create failed:", ex);
        } catch (IllegalArgumentException ex) {
            Log.d(TAG, "create failed:", ex);
        } catch (SecurityException ex) {
            Log.d(TAG, "create failed:", ex);
        }
        return null;
    }

    /**
     * Sets the data source as a content Uri.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void setDataSource(Context context, Uri uri)
        throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        setDataSource(context, uri, null);
    }

    /**
     * Sets the data source as a content Uri.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play
     * @param headers the headers to be sent together with the request for the data
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void setDataSource(Context context, Uri uri, Map<String, String> headers)
        throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        disableProxyListener();

        String scheme = uri.getScheme();
        if(scheme == null || scheme.equals("file")) {
            setDataSource(uri.getPath());
            return;
        }

        AssetFileDescriptor fd = null;
        try {
            ContentResolver resolver = context.getContentResolver();
            fd = resolver.openAssetFileDescriptor(uri, "r");
            if (fd == null) {
                return;
            }
            // Note: using getDeclaredLength so that our behavior is the same
            // as previous versions when the content provider is returning
            // a full file.
            if (fd.getDeclaredLength() < 0) {
                setDataSource(fd.getFileDescriptor());
            } else {
                setDataSource(fd.getFileDescriptor(), fd.getStartOffset(), fd.getDeclaredLength());
            }
            return;
        } catch (SecurityException ex) {
        } catch (IOException ex) {
        } finally {
            if (fd != null) {
                fd.close();
            }
        }

        Log.d(TAG, "Couldn't open file on client side, trying server side");
        setDataSource(uri.toString(), headers);
        if (scheme.equalsIgnoreCase("http")
                || scheme.equalsIgnoreCase("https")) {
            setupProxyListener(context);
        }
    }

    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path the path of the file, or the http/rtsp URL of the stream you want to play
     * @throws IllegalStateException if it is called in an invalid state
     *
     * <p>When <code>path</code> refers to a local file, the file may actually be opened by a
     * process other than the calling application.  This implies that the pathname
     * should be an absolute path (as any other process runs with unspecified current working
     * directory), and that the pathname should reference a world-readable file.
     * As an alternative, the application could first open the file for reading,
     * and then use the file descriptor form {@link #setDataSource(FileDescriptor)}.
     */
    public void setDataSource(String path)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        setDataSource(path, null, null);
    }

    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path the path of the file, or the http/rtsp URL of the stream you want to play
     * @param headers the headers associated with the http request for the stream you want to play
     * @throws IllegalStateException if it is called in an invalid state
     * @hide pending API council
     */
    public void setDataSource(String path, Map<String, String> headers)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException
    {
        String[] keys = null;
        String[] values = null;
        if (headers != null) {
            keys = new String[headers.size()];
            values = new String[headers.size()];

            int i = 0;
            for (Map.Entry<String, String> entry: headers.entrySet()) {
                keys[i] = entry.getKey();
                values[i] = entry.getValue();
                ++i;
            }
        }
        setDataSource(path, keys, values);
    }

    private void setDataSource(String path, String[] keys, String[] values)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        disableProxyListener();

        final Uri uri = Uri.parse(path);
        if ("file".equals(uri.getScheme())) {
            path = uri.getPath();
        }

        final File file = new File(path);
        if (file.exists()) {
            FileInputStream is = new FileInputStream(file);
            FileDescriptor fd = is.getFD();
            setDataSource(fd);
            is.close();
        } else {
            _setDataSource(path, keys, values);
        }
    }

    /**
     * Sets the data source (FileDescriptor) to use. It is the caller's responsibility
     * to close the file descriptor. It is safe to do so as soon as this call returns.
     *
     * @param fd the FileDescriptor for the file you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void setDataSource(FileDescriptor fd)
            throws IOException, IllegalArgumentException, IllegalStateException {
        // intentionally less than LONG_MAX
        setDataSource(fd, 0, 0x7ffffffffffffffL);
    }

    /**
     * Sets the data source (FileDescriptor) to use.  The FileDescriptor must be
     * seekable (N.B. a LocalSocket is not seekable). It is the caller's responsibility
     * to close the file descriptor. It is safe to do so as soon as this call returns.
     *
     * @param fd the FileDescriptor for the file you want to play
     * @param offset the offset into the file where the data to be played starts, in bytes
     * @param length the length in bytes of the data to be played
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void setDataSource(FileDescriptor fd, long offset, long length)
            throws IOException, IllegalArgumentException, IllegalStateException {
        disableProxyListener();
        _setDataSource(fd, offset, length);
    }

    /**
     * Starts or resumes playback. If playback had previously been paused,
     * playback will continue from where it was paused. If playback had
     * been stopped, or never started before, playback will start at the
     * beginning.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void start() throws IllegalStateException {
        stayAwake(true);
        _start();
    }

    /**
     * Stops playback after playback has been stopped or paused.
     *
     * @throws IllegalStateException if the internal player engine has not been
     * initialized.
     */
    public void stop() throws IllegalStateException {
        stayAwake(false);
        _stop();
    }

    /**
     * Pauses playback. Call start() to resume.
     *
     * @throws IllegalStateException if the internal player engine has not been
     * initialized.
     */
    public void pause() throws IllegalStateException {
        stayAwake(false);
        _pause();
    }

    /**
     * Set the low-level power management behavior for this MediaPlayer.  This
     * can be used when the MediaPlayer is not playing through a SurfaceHolder
     * set with {@link #setDisplay(SurfaceHolder)} and thus can use the
     * high-level {@link #setScreenOnWhilePlaying(boolean)} feature.
     *
     * <p>This function has the MediaPlayer access the low-level power manager
     * service to control the device's power usage while playing is occurring.
     * The parameter is a combination of {@link android.os.PowerManager} wake flags.
     * Use of this method requires {@link android.Manifest.permission#WAKE_LOCK}
     * permission.
     * By default, no attempt is made to keep the device awake during playback.
     *
     * @param context the Context to use
     * @param mode    the power/wake mode to set
     * @see android.os.PowerManager
     */
    public void setWakeMode(Context context, int mode) {
        boolean washeld = false;
        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                washeld = true;
                mWakeLock.release();
            }
            mWakeLock = null;
        }

        PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(mode|PowerManager.ON_AFTER_RELEASE, MediaPlayer.class.getName());
        mWakeLock.setReferenceCounted(false);
        if (washeld) {
            mWakeLock.acquire();
        }
    }

    /**
     * Control whether we should use the attached SurfaceHolder to keep the
     * screen on while video playback is occurring.  This is the preferred
     * method over {@link #setWakeMode} where possible, since it doesn't
     * require that the application have permission for low-level wake lock
     * access.
     *
     * @param screenOn Supply true to keep the screen on, false to allow it
     * to turn off.
     */
    public void setScreenOnWhilePlaying(boolean screenOn) {
        if (mScreenOnWhilePlaying != screenOn) {
            if (screenOn && mSurfaceHolder == null) {
                Log.w(TAG, "setScreenOnWhilePlaying(true) is ineffective without a SurfaceHolder");
            }
            mScreenOnWhilePlaying = screenOn;
            updateSurfaceScreenOn();
        }
    }

    private void stayAwake(boolean awake) {
        if (mWakeLock != null) {
            if (awake && !mWakeLock.isHeld()) {
                mWakeLock.acquire();
            } else if (!awake && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }
        mStayAwake = awake;
        updateSurfaceScreenOn();
    }

    private void updateSurfaceScreenOn() {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
        }
    }

    /**
     * Releases resources associated with this MediaPlayer object.
     * It is considered good practice to call this method when you're
     * done using the MediaPlayer. In particular, whenever an Activity
     * of an application is paused (its onPause() method is called),
     * or stopped (its onStop() method is called), this method should be
     * invoked to release the MediaPlayer object, unless the application
     * has a special need to keep the object around. In addition to
     * unnecessary resources (such as memory and instances of codecs)
     * being held, failure to call this method immediately if a
     * MediaPlayer object is no longer needed may also lead to
     * continuous battery consumption for mobile devices, and playback
     * failure for other applications if no multiple instances of the
     * same codec are supported on a device. Even if multiple instances
     * of the same codec are supported, some performance degradation
     * may be expected when unnecessary multiple instances are used
     * at the same time.
     */
    public void release() {
        stayAwake(false);
        updateSurfaceScreenOn();
        mOnPreparedListener = null;
        mOnBufferingUpdateListener = null;
        mOnCompletionListener = null;
        mOnSeekCompleteListener = null;
        mOnErrorListener = null;
        mOnInfoListener = null;
        mOnVideoSizeChangedListener = null;
        mOnTimedTextListener = null;
        if (mTimeProvider != null) {
            mTimeProvider.close();
            mTimeProvider = null;
        }
        mOnSubtitleDataListener = null;
        _release();
    }

    /**
     * Resets the MediaPlayer to its uninitialized state. After calling
     * this method, you will have to initialize it again by setting the
     * data source and calling prepare().
     */
    public void reset() {
        mSelectedSubtitleTrackIndex = -1;
        synchronized(mOpenSubtitleSources) {
            for (final InputStream is: mOpenSubtitleSources) {
                try {
                    is.close();
                } catch (IOException e) {
                }
            }
            mOpenSubtitleSources.clear();
        }
        mOutOfBandSubtitleTracks.clear();
        mInbandSubtitleTracks = new SubtitleTrack[0];
        if (mSubtitleController != null) {
            mSubtitleController.reset();
        }
        if (mTimeProvider != null) {
            mTimeProvider.close();
            mTimeProvider = null;
        }

        stayAwake(false);
        _reset();
        // make sure none of the listeners get called anymore
        if (mEventHandler != null) {
            mEventHandler.removeCallbacksAndMessages(null);
        }

        disableProxyListener();
    }

    /**
     * Similar, excepts sets volume of all channels to same value.
     * @hide
     */
    public void setVolume(float volume) {
        setVolume(volume, volume);
    }

    /* Do not change these values without updating their counterparts
     * in include/media/stagefright/MediaDefs.h and media/libstagefright/MediaDefs.cpp!
     */
    /**
     * MIME type for SubRip (SRT) container. Used in addTimedTextSource APIs.
     */
    public static final String MEDIA_MIMETYPE_TEXT_SUBRIP = "application/x-subrip";

    /**
     * MIME type for WebVTT subtitle data.
     */
    public static final String MEDIA_MIMETYPE_TEXT_VTT = "text/vtt";

    /*
     * A helper function to check if the mime type is supported by media framework.
     */
    private static boolean availableMimeTypeForExternalSource(String mimeType) {
        if (mimeType == MEDIA_MIMETYPE_TEXT_SUBRIP) {
            return true;
        }
        return false;
    }

    private SubtitleController mSubtitleController;

    public void setSubtitleAnchor(
            SubtitleController controller,
            SubtitleController.Anchor anchor) {
        // TODO: create SubtitleController in MediaPlayer
        mSubtitleController = controller;
        mSubtitleController.setAnchor(anchor);
    }

    private SubtitleTrack[] mInbandSubtitleTracks;
    private int mSelectedSubtitleTrackIndex = -1;
    private Vector<SubtitleTrack> mOutOfBandSubtitleTracks;
    private Vector<InputStream> mOpenSubtitleSources;

    private OnSubtitleDataListener mSubtitleDataListener = new OnSubtitleDataListener() {
        @Override
        public void onSubtitleData(MediaPlayer mp, SubtitleData data) {
            int index = data.getTrackIndex();
            if (index >= mInbandSubtitleTracks.length) {
                return;
            }
            SubtitleTrack track = mInbandSubtitleTracks[index];
            if (track != null) {
                try {
                    long runID = data.getStartTimeUs() + 1;
                    // TODO: move conversion into track
                    track.onData(new String(data.getData(), "UTF-8"), true /* eos */, runID);
                    track.setRunDiscardTimeMs(
                            runID,
                            (data.getStartTimeUs() + data.getDurationUs()) / 1000);
                } catch (java.io.UnsupportedEncodingException e) {
                    Log.w(TAG, "subtitle data for track " + index + " is not UTF-8 encoded: " + e);
                }
            }
        }
    };

    /* TODO: Limit the total number of external timed text source to a reasonable number.
     */
    /**
     * Adds an external timed text source file.
     *
     * Currently supported format is SubRip with the file extension .srt, case insensitive.
     * Note that a single external timed text source may contain multiple tracks in it.
     * One can find the total number of available tracks using {@link #getTrackInfo()} to see what
     * additional tracks become available after this method call.
     *
     * @param path The file path of external timed text source file.
     * @param mimeType The mime type of the file. Must be one of the mime types listed above.
     * @throws IOException if the file cannot be accessed or is corrupted.
     * @throws IllegalArgumentException if the mimeType is not supported.
     * @throws IllegalStateException if called in an invalid state.
     */
    public void addTimedTextSource(String path, String mimeType)
            throws IOException, IllegalArgumentException, IllegalStateException {
        if (!availableMimeTypeForExternalSource(mimeType)) {
            final String msg = "Illegal mimeType for timed text source: " + mimeType;
            throw new IllegalArgumentException(msg);
        }

        File file = new File(path);
        if (file.exists()) {
            FileInputStream is = new FileInputStream(file);
            FileDescriptor fd = is.getFD();
            addTimedTextSource(fd, mimeType);
            is.close();
        } else {
            // We do not support the case where the path is not a file.
            throw new IOException(path);
        }
    }

    /**
     * Adds an external timed text source file (Uri).
     *
     * Currently supported format is SubRip with the file extension .srt, case insensitive.
     * Note that a single external timed text source may contain multiple tracks in it.
     * One can find the total number of available tracks using {@link #getTrackInfo()} to see what
     * additional tracks become available after this method call.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play
     * @param mimeType The mime type of the file. Must be one of the mime types listed above.
     * @throws IOException if the file cannot be accessed or is corrupted.
     * @throws IllegalArgumentException if the mimeType is not supported.
     * @throws IllegalStateException if called in an invalid state.
     */
    public void addTimedTextSource(Context context, Uri uri, String mimeType)
            throws IOException, IllegalArgumentException, IllegalStateException {
        String scheme = uri.getScheme();
        if(scheme == null || scheme.equals("file")) {
            addTimedTextSource(uri.getPath(), mimeType);
            return;
        }

        AssetFileDescriptor fd = null;
        try {
            ContentResolver resolver = context.getContentResolver();
            fd = resolver.openAssetFileDescriptor(uri, "r");
            if (fd == null) {
                return;
            }
            addTimedTextSource(fd.getFileDescriptor(), mimeType);
            return;
        } catch (SecurityException ex) {
        } catch (IOException ex) {
        } finally {
            if (fd != null) {
                fd.close();
            }
        }
    }

    /**
     * Adds an external timed text source file (FileDescriptor).
     *
     * It is the caller's responsibility to close the file descriptor.
     * It is safe to do so as soon as this call returns.
     *
     * Currently supported format is SubRip. Note that a single external timed text source may
     * contain multiple tracks in it. One can find the total number of available tracks
     * using {@link #getTrackInfo()} to see what additional tracks become available
     * after this method call.
     *
     * @param fd the FileDescriptor for the file you want to play
     * @param mimeType The mime type of the file. Must be one of the mime types listed above.
     * @throws IllegalArgumentException if the mimeType is not supported.
     * @throws IllegalStateException if called in an invalid state.
     */
    public void addTimedTextSource(FileDescriptor fd, String mimeType)
            throws IllegalArgumentException, IllegalStateException {
        // intentionally less than LONG_MAX
        addTimedTextSource(fd, 0, 0x7ffffffffffffffL, mimeType);
    }

    /**
     * Sets the target UDP re-transmit endpoint for the low level player.
     * Generally, the address portion of the endpoint is an IP multicast
     * address, although a unicast address would be equally valid.  When a valid
     * retransmit endpoint has been set, the media player will not decode and
     * render the media presentation locally.  Instead, the player will attempt
     * to re-multiplex its media data using the Android@Home RTP profile and
     * re-transmit to the target endpoint.  Receiver devices (which may be
     * either the same as the transmitting device or different devices) may
     * instantiate, prepare, and start a receiver player using a setDataSource
     * URL of the form...
     *
     * aahRX://&lt;multicastIP&gt;:&lt;port&gt;
     *
     * to receive, decode and render the re-transmitted content.
     *
     * setRetransmitEndpoint may only be called before setDataSource has been
     * called; while the player is in the Idle state.
     *
     * @param endpoint the address and UDP port of the re-transmission target or
     * null if no re-transmission is to be performed.
     * @throws IllegalStateException if it is called in an invalid state
     * @throws IllegalArgumentException if the retransmit endpoint is supplied,
     * but invalid.
     *
     * {@hide} pending API council
     */
    public void setRetransmitEndpoint(InetSocketAddress endpoint)
            throws IllegalStateException, IllegalArgumentException
    {
        String addrString = null;
        int port = 0;

        if (null != endpoint) {
            addrString = endpoint.getAddress().getHostAddress();
            port = endpoint.getPort();
        }

        int ret = native_setRetransmitEndpoint(addrString, port);
        if (ret != 0) {
            throw new IllegalArgumentException("Illegal re-transmit endpoint; native ret " + ret);
        }
    }

    @Override
    protected void finalize() { native_finalize(); }

    /* Do not change these values without updating their counterparts
     * in include/media/mediaplayer.h!
     */
    private static final int MEDIA_NOP = 0; // interface test message
    private static final int MEDIA_PREPARED = 1;
    private static final int MEDIA_PLAYBACK_COMPLETE = 2;
    private static final int MEDIA_BUFFERING_UPDATE = 3;
    private static final int MEDIA_SEEK_COMPLETE = 4;
    private static final int MEDIA_SET_VIDEO_SIZE = 5;
    private static final int MEDIA_STARTED = 6;
    private static final int MEDIA_PAUSED = 7;
    private static final int MEDIA_STOPPED = 8;
    private static final int MEDIA_SKIPPED = 9;
    private static final int MEDIA_TIMED_TEXT = 99;
    private static final int MEDIA_ERROR = 100;
    private static final int MEDIA_INFO = 200;
    private static final int MEDIA_SUBTITLE_DATA = 201;

    private TimeProvider mTimeProvider;

    public MediaTimeProvider getMediaTimeProvider() {
        if (mTimeProvider == null) {
            mTimeProvider = new TimeProvider(this);
        }
        return mTimeProvider;
    }

    private class EventHandler extends Handler
    {
        private MediaPlayer mMediaPlayer;

        public EventHandler(MediaPlayer mp, Looper looper) {
            super(looper);
            mMediaPlayer = mp;
        }

        @Override
        public void handleMessage(Message msg) {
            if (mMediaPlayer.mNativeContext == 0) {
                Log.w(TAG, "mediaplayer went away with unhandled events");
                return;
            }
            switch(msg.what) {
            case MEDIA_PREPARED:
                if (mOnPreparedListener != null)
                    mOnPreparedListener.onPrepared(mMediaPlayer);
                return;

            case MEDIA_PLAYBACK_COMPLETE:
                if (mOnCompletionListener != null)
                    mOnCompletionListener.onCompletion(mMediaPlayer);
                stayAwake(false);
                return;

            case MEDIA_STOPPED:
                if (mTimeProvider != null) {
                    mTimeProvider.onStopped();
                }
                break;

            case MEDIA_STARTED:
            case MEDIA_PAUSED:
                if (mTimeProvider != null) {
                    mTimeProvider.onPaused(msg.what == MEDIA_PAUSED);
                }
                break;

            case MEDIA_BUFFERING_UPDATE:
                if (mOnBufferingUpdateListener != null)
                    mOnBufferingUpdateListener.onBufferingUpdate(mMediaPlayer, msg.arg1);
                return;

            case MEDIA_SEEK_COMPLETE:
              if (mOnSeekCompleteListener != null) {
                  mOnSeekCompleteListener.onSeekComplete(mMediaPlayer);
              }
              // fall through

            case MEDIA_SKIPPED:
              if (mTimeProvider != null) {
                  mTimeProvider.onSeekComplete(mMediaPlayer);
              }
              return;

            case MEDIA_SET_VIDEO_SIZE:
              if (mOnVideoSizeChangedListener != null)
                  mOnVideoSizeChangedListener.onVideoSizeChanged(mMediaPlayer, msg.arg1, msg.arg2);
              return;

            case MEDIA_ERROR:
                Log.e(TAG, "Error (" + msg.arg1 + "," + msg.arg2 + ")");
                boolean error_was_handled = false;
                if (mOnErrorListener != null) {
                    error_was_handled = mOnErrorListener.onError(mMediaPlayer, msg.arg1, msg.arg2);
                }
                if (mOnCompletionListener != null && ! error_was_handled) {
                    mOnCompletionListener.onCompletion(mMediaPlayer);
                }
                stayAwake(false);
                return;

            case MEDIA_INFO:
                switch (msg.arg1) {
                case MEDIA_INFO_VIDEO_TRACK_LAGGING:
                    Log.i(TAG, "Info (" + msg.arg1 + "," + msg.arg2 + ")");
                    break;
                case MEDIA_INFO_METADATA_UPDATE:
                case MEDIA_INFO_EXTERNAL_METADATA_UPDATE:
                    msg.arg1 = MEDIA_INFO_METADATA_UPDATE;
                    // update default track selection
                    mSubtitleController.selectDefaultTrack();
                    break;
                }

                if (mOnInfoListener != null) {
                    mOnInfoListener.onInfo(mMediaPlayer, msg.arg1, msg.arg2);
                }
                // No real default action so far.
                return;
            case MEDIA_TIMED_TEXT:
                return;
            case MEDIA_SUBTITLE_DATA:
                return;
            case MEDIA_NOP: // interface test message - ignore
                break;
            default:
                Log.e(TAG, "Unknown message type " + msg.what);
                return;
            }
        }
    }

    /*
     * Called from native code when an interesting event happens.  This method
     * just uses the EventHandler system to post the event back to the main app thread.
     * We use a weak reference to the original MediaPlayer object so that the native
     * code is safe from the object disappearing from underneath it.  (This is
     * the cookie passed to native_setup().)
     */
    private static void postEventFromNative(Object mediaplayer_ref,
                                            int what, int arg1, int arg2, Object obj)
    {
        MediaPlayer mp = (MediaPlayer)((WeakReference)mediaplayer_ref).get();
        if (mp == null) {
            return;
        }

        if (what == MEDIA_INFO && arg1 == MEDIA_INFO_STARTED_AS_NEXT) {
            // this acquires the wakelock if needed, and sets the client side state
            mp.start();
        }
        if (mp.mEventHandler != null) {
            Message m = mp.mEventHandler.obtainMessage(what, arg1, arg2, obj);
            mp.mEventHandler.sendMessage(m);
        }
    }

    /**
     * Interface definition for a callback to be invoked when the media
     * source is ready for playback.
     */
    public interface OnPreparedListener
    {
        /**
         * Called when the media file is ready for playback.
         *
         * @param mp the MediaPlayer that is ready for playback
         */
        void onPrepared(MediaPlayer mp);
    }

    /**
     * Register a callback to be invoked when the media source is ready
     * for playback.
     *
     * @param listener the callback that will be run
     */
    public void setOnPreparedListener(OnPreparedListener listener)
    {
        mOnPreparedListener = listener;
    }

    private OnPreparedListener mOnPreparedListener;

    /**
     * Interface definition for a callback to be invoked when playback of
     * a media source has completed.
     */
    public interface OnCompletionListener
    {
        /**
         * Called when the end of a media source is reached during playback.
         *
         * @param mp the MediaPlayer that reached the end of the file
         */
        void onCompletion(MediaPlayer mp);
    }

    /**
     * Register a callback to be invoked when the end of a media source
     * has been reached during playback.
     *
     * @param listener the callback that will be run
     */
    public void setOnCompletionListener(OnCompletionListener listener)
    {
        mOnCompletionListener = listener;
    }

    private OnCompletionListener mOnCompletionListener;

    /**
     * Interface definition of a callback to be invoked indicating buffering
     * status of a media resource being streamed over the network.
     */
    public interface OnBufferingUpdateListener
    {
        /**
         * Called to update status in buffering a media stream received through
         * progressive HTTP download. The received buffering percentage
         * indicates how much of the content has been buffered or played.
         * For example a buffering update of 80 percent when half the content
         * has already been played indicates that the next 30 percent of the
         * content to play has been buffered.
         *
         * @param mp      the MediaPlayer the update pertains to
         * @param percent the percentage (0-100) of the content
         *                that has been buffered or played thus far
         */
        void onBufferingUpdate(MediaPlayer mp, int percent);
    }

    /**
     * Register a callback to be invoked when the status of a network
     * stream's buffer has changed.
     *
     * @param listener the callback that will be run.
     */
    public void setOnBufferingUpdateListener(OnBufferingUpdateListener listener)
    {
        mOnBufferingUpdateListener = listener;
    }

    private OnBufferingUpdateListener mOnBufferingUpdateListener;

    /**
     * Interface definition of a callback to be invoked indicating
     * the completion of a seek operation.
     */
    public interface OnSeekCompleteListener
    {
        /**
         * Called to indicate the completion of a seek operation.
         *
         * @param mp the MediaPlayer that issued the seek operation
         */
        public void onSeekComplete(MediaPlayer mp);
    }

    /**
     * Register a callback to be invoked when a seek operation has been
     * completed.
     *
     * @param listener the callback that will be run
     */
    public void setOnSeekCompleteListener(OnSeekCompleteListener listener)
    {
        mOnSeekCompleteListener = listener;
    }

    private OnSeekCompleteListener mOnSeekCompleteListener;

    /**
     * Interface definition of a callback to be invoked when the
     * video size is first known or updated
     */
    public interface OnVideoSizeChangedListener
    {
        /**
         * Called to indicate the video size
         *
         * The video size (width and height) could be 0 if there was no video,
         * no display surface was set, or the value was not determined yet.
         *
         * @param mp        the MediaPlayer associated with this callback
         * @param width     the width of the video
         * @param height    the height of the video
         */
        public void onVideoSizeChanged(MediaPlayer mp, int width, int height);
    }

    /**
     * Register a callback to be invoked when the video size is
     * known or updated.
     *
     * @param listener the callback that will be run
     */
    public void setOnVideoSizeChangedListener(OnVideoSizeChangedListener listener)
    {
        mOnVideoSizeChangedListener = listener;
    }

    private OnVideoSizeChangedListener mOnVideoSizeChangedListener;

    /**
     * Interface definition of a callback to be invoked when a
     * timed text is available for display.
     */
    public interface OnTimedTextListener
    {
        /**
         * Called to indicate an avaliable timed text
         *
         * @param mp             the MediaPlayer associated with this callback
         * @param text           the timed text sample which contains the text
         *                       needed to be displayed and the display format.
         */
        public void onTimedText(MediaPlayer mp, TimedText text);
    }

    /**
     * Register a callback to be invoked when a timed text is available
     * for display.
     *
     * @param listener the callback that will be run
     */
    public void setOnTimedTextListener(OnTimedTextListener listener)
    {
        mOnTimedTextListener = listener;
    }

    private OnTimedTextListener mOnTimedTextListener;

    /**
     * Interface definition of a callback to be invoked when a
     * track has data available.
     *
     * @hide
     */
    public interface OnSubtitleDataListener
    {
        public void onSubtitleData(MediaPlayer mp, SubtitleData data);
    }

    /**
     * Register a callback to be invoked when a track has data available.
     *
     * @param listener the callback that will be run
     *
     * @hide
     */
    public void setOnSubtitleDataListener(OnSubtitleDataListener listener)
    {
        mOnSubtitleDataListener = listener;
    }

    private OnSubtitleDataListener mOnSubtitleDataListener;

    /* Do not change these values without updating their counterparts
     * in include/media/mediaplayer.h!
     */
    /** Unspecified media player error.
     * @see android.media.MediaPlayer.OnErrorListener
     */
    public static final int MEDIA_ERROR_UNKNOWN = 1;

    /** Media server died. In this case, the application must release the
     * MediaPlayer object and instantiate a new one.
     * @see android.media.MediaPlayer.OnErrorListener
     */
    public static final int MEDIA_ERROR_SERVER_DIED = 100;

    /** The video is streamed and its container is not valid for progressive
     * playback i.e the video's index (e.g moov atom) is not at the start of the
     * file.
     * @see android.media.MediaPlayer.OnErrorListener
     */
    public static final int MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200;

    /** File or network related operation errors. */
    public static final int MEDIA_ERROR_IO = -1004;
    /** Bitstream is not conforming to the related coding standard or file spec. */
    public static final int MEDIA_ERROR_MALFORMED = -1007;
    /** Bitstream is conforming to the related coding standard or file spec, but
     * the media framework does not support the feature. */
    public static final int MEDIA_ERROR_UNSUPPORTED = -1010;
    /** Some operation takes too long to complete, usually more than 3-5 seconds. */
    public static final int MEDIA_ERROR_TIMED_OUT = -110;

    /**
     * Interface definition of a callback to be invoked when there
     * has been an error during an asynchronous operation (other errors
     * will throw exceptions at method call time).
     */
    public interface OnErrorListener
    {
        /**
         * Called to indicate an error.
         *
         * @param mp      the MediaPlayer the error pertains to
         * @param what    the type of error that has occurred:
         * <ul>
         * <li>{@link #MEDIA_ERROR_UNKNOWN}
         * <li>{@link #MEDIA_ERROR_SERVER_DIED}
         * </ul>
         * @param extra an extra code, specific to the error. Typically
         * implementation dependent.
         * <ul>
         * <li>{@link #MEDIA_ERROR_IO}
         * <li>{@link #MEDIA_ERROR_MALFORMED}
         * <li>{@link #MEDIA_ERROR_UNSUPPORTED}
         * <li>{@link #MEDIA_ERROR_TIMED_OUT}
         * </ul>
         * @return True if the method handled the error, false if it didn't.
         * Returning false, or not having an OnErrorListener at all, will
         * cause the OnCompletionListener to be called.
         */
        boolean onError(MediaPlayer mp, int what, int extra);
    }

    /**
     * Register a callback to be invoked when an error has happened
     * during an asynchronous operation.
     *
     * @param listener the callback that will be run
     */
    public void setOnErrorListener(OnErrorListener listener)
    {
        mOnErrorListener = listener;
    }

    private OnErrorListener mOnErrorListener;


    /* Do not change these values without updating their counterparts
     * in include/media/mediaplayer.h!
     */
    /** Unspecified media player info.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_UNKNOWN = 1;

    /** The player was started because it was used as the next player for another
     * player, which just completed playback.
     * @see android.media.MediaPlayer.OnInfoListener
     * @hide
     */
    public static final int MEDIA_INFO_STARTED_AS_NEXT = 2;

    /** The player just pushed the very first video frame for rendering.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_VIDEO_RENDERING_START = 3;

    /** The video is too complex for the decoder: it can't decode frames fast
     *  enough. Possibly only the audio plays fine at this stage.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_VIDEO_TRACK_LAGGING = 700;

    /** MediaPlayer is temporarily pausing playback internally in order to
     * buffer more data.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_BUFFERING_START = 701;

    /** MediaPlayer is resuming playback after filling buffers.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_BUFFERING_END = 702;

    /** Bad interleaving means that a media has been improperly interleaved or
     * not interleaved at all, e.g has all the video samples first then all the
     * audio ones. Video is playing but a lot of disk seeks may be happening.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_BAD_INTERLEAVING = 800;

    /** The media cannot be seeked (e.g live stream)
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_NOT_SEEKABLE = 801;

    /** A new set of metadata is available.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_METADATA_UPDATE = 802;

    /** A new set of external-only metadata is available.  Used by
     *  JAVA framework to avoid triggering track scanning.
     * @hide
     */
    public static final int MEDIA_INFO_EXTERNAL_METADATA_UPDATE = 803;

    /** Failed to handle timed text track properly.
     * @see android.media.MediaPlayer.OnInfoListener
     *
     * {@hide}
     */
    public static final int MEDIA_INFO_TIMED_TEXT_ERROR = 900;

    /** Subtitle track was not supported by the media framework.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_UNSUPPORTED_SUBTITLE = 901;

    /** Reading the subtitle track takes too long.
     * @see android.media.MediaPlayer.OnInfoListener
     */
    public static final int MEDIA_INFO_SUBTITLE_TIMED_OUT = 902;

    /**
     * Interface definition of a callback to be invoked to communicate some
     * info and/or warning about the media or its playback.
     */
    public interface OnInfoListener
    {
        /**
         * Called to indicate an info or a warning.
         *
         * @param mp      the MediaPlayer the info pertains to.
         * @param what    the type of info or warning.
         * <ul>
         * <li>{@link #MEDIA_INFO_UNKNOWN}
         * <li>{@link #MEDIA_INFO_VIDEO_TRACK_LAGGING}
         * <li>{@link #MEDIA_INFO_VIDEO_RENDERING_START}
         * <li>{@link #MEDIA_INFO_BUFFERING_START}
         * <li>{@link #MEDIA_INFO_BUFFERING_END}
         * <li>{@link #MEDIA_INFO_BAD_INTERLEAVING}
         * <li>{@link #MEDIA_INFO_NOT_SEEKABLE}
         * <li>{@link #MEDIA_INFO_METADATA_UPDATE}
         * <li>{@link #MEDIA_INFO_UNSUPPORTED_SUBTITLE}
         * <li>{@link #MEDIA_INFO_SUBTITLE_TIMED_OUT}
         * </ul>
         * @param extra an extra code, specific to the info. Typically
         * implementation dependent.
         * @return True if the method handled the info, false if it didn't.
         * Returning false, or not having an OnErrorListener at all, will
         * cause the info to be discarded.
         */
        boolean onInfo(MediaPlayer mp, int what, int extra);
    }

    /**
     * Register a callback to be invoked when an info/warning is available.
     *
     * @param listener the callback that will be run
     */
    public void setOnInfoListener(OnInfoListener listener)
    {
        mOnInfoListener = listener;
    }

    private OnInfoListener mOnInfoListener;

    /*
     * Test whether a given video scaling mode is supported.
     */
    private boolean isVideoScalingModeSupported(int mode) {
        return (mode == VIDEO_SCALING_MODE_SCALE_TO_FIT ||
                mode == VIDEO_SCALING_MODE_SCALE_TO_FIT_WITH_CROPPING);
    }

    private Context mProxyContext = null;
    private ProxyReceiver mProxyReceiver = null;

    private void setupProxyListener(Context context) {
        IntentFilter filter = new IntentFilter();
        filter.addAction(Proxy.PROXY_CHANGE_ACTION);
        mProxyReceiver = new ProxyReceiver();
        mProxyContext = context;

        Intent currentProxy =
            context.getApplicationContext().registerReceiver(mProxyReceiver, filter);

        if (currentProxy != null) {
            handleProxyBroadcast(currentProxy);
        }
    }

    private void disableProxyListener() {
        if (mProxyReceiver == null) {
            return;
        }

        Context appContext = mProxyContext.getApplicationContext();
        if (appContext != null) {
            appContext.unregisterReceiver(mProxyReceiver);
        }

        mProxyReceiver = null;
        mProxyContext = null;
    }

    private void handleProxyBroadcast(Intent intent) {
        ProxyProperties props =
            (ProxyProperties)intent.getExtra(Proxy.EXTRA_PROXY_INFO);

        if (props == null || props.getHost() == null) {
            updateProxyConfig(null);
        } else {
            updateProxyConfig(props);
        }
    }

    private class ProxyReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Proxy.PROXY_CHANGE_ACTION)) {
                handleProxyBroadcast(intent);
            }
        }
    }

    static class TimeProvider implements MediaPlayer.OnSeekCompleteListener,
            MediaTimeProvider {
        private static final String TAG = "MTP";
        private static final long MAX_NS_WITHOUT_POSITION_CHECK = 5000000000L;
        private static final long MAX_EARLY_CALLBACK_US = 1000;
        private static final long TIME_ADJUSTMENT_RATE = 2;  /* meaning 1/2 */
        private long mLastTimeUs = 0;
        private MediaPlayer mPlayer;
        private boolean mPaused = true;
        private boolean mStopped = true;
        private long mLastReportedTime;
        private long mTimeAdjustment;
        // since we are expecting only a handful listeners per stream, there is
        // no need for log(N) search performance
        private MediaTimeProvider.OnMediaTimeListener mListeners[];
        private long mTimes[];
        private long mLastNanoTime;
        private Handler mEventHandler;
        private boolean mRefresh = false;
        private boolean mPausing = false;
        private boolean mSeeking = false;
        private static final int NOTIFY = 1;
        private static final int NOTIFY_TIME = 0;
        private static final int REFRESH_AND_NOTIFY_TIME = 1;
        private static final int NOTIFY_STOP = 2;
        private static final int NOTIFY_SEEK = 3;
        private HandlerThread mHandlerThread;
        public boolean DEBUG = false;

        public TimeProvider(MediaPlayer mp) {
            mPlayer = mp;
            try {
                getCurrentTimeUs(true, false);
            } catch (IllegalStateException e) {
                // we assume starting position
                mRefresh = true;
            }

            Looper looper;
            if ((looper = Looper.myLooper()) == null &&
                (looper = Looper.getMainLooper()) == null) {
                // Create our own looper here in case MP was created without one
                mHandlerThread = new HandlerThread("MediaPlayerMTPEventThread",
                      Process.THREAD_PRIORITY_FOREGROUND);
                mHandlerThread.start();
                looper = mHandlerThread.getLooper();
            }
            mEventHandler = new EventHandler(looper);

            mListeners = new MediaTimeProvider.OnMediaTimeListener[0];
            mTimes = new long[0];
            mLastTimeUs = 0;
            mTimeAdjustment = 0;
        }

        private void scheduleNotification(int type, long delayUs) {
            // ignore time notifications until seek is handled
            if (mSeeking &&
                    (type == NOTIFY_TIME || type == REFRESH_AND_NOTIFY_TIME)) {
                return;
            }

            if (DEBUG) Log.v(TAG, "scheduleNotification " + type + " in " + delayUs);
            mEventHandler.removeMessages(NOTIFY);
            Message msg = mEventHandler.obtainMessage(NOTIFY, type, 0);
            mEventHandler.sendMessageDelayed(msg, (int) (delayUs / 1000));
        }

        public void close() {
            mEventHandler.removeMessages(NOTIFY);
            if (mHandlerThread != null) {
                mHandlerThread.quitSafely();
                mHandlerThread = null;
            }
        }

        protected void finalize() {
            if (mHandlerThread != null) {
                mHandlerThread.quitSafely();
            }
        }

        public void onPaused(boolean paused) {
            synchronized(this) {
                if (DEBUG) Log.d(TAG, "onPaused: " + paused);
                if (mStopped) { // handle as seek if we were stopped
                    mStopped = false;
                    mSeeking = true;
                    scheduleNotification(NOTIFY_SEEK, 0 /* delay */);
                } else {
                    mPausing = paused;  // special handling if player disappeared
                    mSeeking = false;
                    scheduleNotification(REFRESH_AND_NOTIFY_TIME, 0 /* delay */);
                }
            }
        }

        public void onStopped() {
            synchronized(this) {
                if (DEBUG) Log.d(TAG, "onStopped");
                mPaused = true;
                mStopped = true;
                mSeeking = false;
                scheduleNotification(NOTIFY_STOP, 0 /* delay */);
            }
        }

        @Override
        public void onSeekComplete(MediaPlayer mp) {
            synchronized(this) {
                mStopped = false;
                mSeeking = true;
                scheduleNotification(NOTIFY_SEEK, 0 /* delay */);
            }
        }

        public void onNewPlayer() {
            if (mRefresh) {
                synchronized(this) {
                    mStopped = false;
                    mSeeking = true;
                    scheduleNotification(NOTIFY_SEEK, 0 /* delay */);
                }
            }
        }

        private synchronized void notifySeek() {
            mSeeking = false;
            try {
                long timeUs = getCurrentTimeUs(true, false);
                if (DEBUG) Log.d(TAG, "onSeekComplete at " + timeUs);

                for (MediaTimeProvider.OnMediaTimeListener listener: mListeners) {
                    if (listener == null) {
                        break;
                    }
                    listener.onSeek(timeUs);
                }
            } catch (IllegalStateException e) {
                // we should not be there, but at least signal pause
                if (DEBUG) Log.d(TAG, "onSeekComplete but no player");
                mPausing = true;  // special handling if player disappeared
                notifyTimedEvent(false /* refreshTime */);
            }
        }

        private synchronized void notifyStop() {
            for (MediaTimeProvider.OnMediaTimeListener listener: mListeners) {
                if (listener == null) {
                    break;
                }
                listener.onStop();
            }
        }

        private int registerListener(MediaTimeProvider.OnMediaTimeListener listener) {
            int i = 0;
            for (; i < mListeners.length; i++) {
                if (mListeners[i] == listener || mListeners[i] == null) {
                    break;
                }
            }

            // new listener
            if (i >= mListeners.length) {
                MediaTimeProvider.OnMediaTimeListener[] newListeners =
                    new MediaTimeProvider.OnMediaTimeListener[i + 1];
                long[] newTimes = new long[i + 1];
                System.arraycopy(mListeners, 0, newListeners, 0, mListeners.length);
                System.arraycopy(mTimes, 0, newTimes, 0, mTimes.length);
                mListeners = newListeners;
                mTimes = newTimes;
            }

            if (mListeners[i] == null) {
                mListeners[i] = listener;
                mTimes[i] = MediaTimeProvider.NO_TIME;
            }
            return i;
        }

        public void notifyAt(
                long timeUs, MediaTimeProvider.OnMediaTimeListener listener) {
            synchronized(this) {
                if (DEBUG) Log.d(TAG, "notifyAt " + timeUs);
                mTimes[registerListener(listener)] = timeUs;
                scheduleNotification(NOTIFY_TIME, 0 /* delay */);
            }
        }

        public void scheduleUpdate(MediaTimeProvider.OnMediaTimeListener listener) {
            synchronized(this) {
                if (DEBUG) Log.d(TAG, "scheduleUpdate");
                int i = registerListener(listener);

                if (mStopped) {
                    scheduleNotification(NOTIFY_STOP, 0 /* delay */);
                } else {
                    mTimes[i] = 0;
                    scheduleNotification(NOTIFY_TIME, 0 /* delay */);
                }
            }
        }

        public void cancelNotifications(
                MediaTimeProvider.OnMediaTimeListener listener) {
            synchronized(this) {
                int i = 0;
                for (; i < mListeners.length; i++) {
                    if (mListeners[i] == listener) {
                        System.arraycopy(mListeners, i + 1,
                                mListeners, i, mListeners.length - i - 1);
                        System.arraycopy(mTimes, i + 1,
                                mTimes, i, mTimes.length - i - 1);
                        mListeners[mListeners.length - 1] = null;
                        mTimes[mTimes.length - 1] = NO_TIME;
                        break;
                    } else if (mListeners[i] == null) {
                        break;
                    }
                }

                scheduleNotification(NOTIFY_TIME, 0 /* delay */);
            }
        }

        private synchronized void notifyTimedEvent(boolean refreshTime) {
            // figure out next callback
            long nowUs;
            try {
                nowUs = getCurrentTimeUs(refreshTime, true);
            } catch (IllegalStateException e) {
                // assume we paused until new player arrives
                mRefresh = true;
                mPausing = true; // this ensures that call succeeds
                nowUs = getCurrentTimeUs(refreshTime, true);
            }
            long nextTimeUs = nowUs;

            if (mSeeking) {
                // skip timed-event notifications until seek is complete
                return;
            }

            if (DEBUG) {
                StringBuilder sb = new StringBuilder();
                sb.append("notifyTimedEvent(").append(mLastTimeUs).append(" -> ")
                        .append(nowUs).append(") from {");
                boolean first = true;
                for (long time: mTimes) {
                    if (time == NO_TIME) {
                        continue;
                    }
                    if (!first) sb.append(", ");
                    sb.append(time);
                    first = false;
                }
                sb.append("}");
                Log.d(TAG, sb.toString());
            }

            Vector<MediaTimeProvider.OnMediaTimeListener> activatedListeners =
                new Vector<MediaTimeProvider.OnMediaTimeListener>();
            for (int ix = 0; ix < mTimes.length; ix++) {
                if (mListeners[ix] == null) {
                    break;
                }
                if (mTimes[ix] <= NO_TIME) {
                    // ignore, unless we were stopped
                } else if (mTimes[ix] <= nowUs + MAX_EARLY_CALLBACK_US) {
                    activatedListeners.add(mListeners[ix]);
                    if (DEBUG) Log.d(TAG, "removed");
                    mTimes[ix] = NO_TIME;
                } else if (nextTimeUs == nowUs || mTimes[ix] < nextTimeUs) {
                    nextTimeUs = mTimes[ix];
                }
            }

            if (nextTimeUs > nowUs && !mPaused) {
                // schedule callback at nextTimeUs
                if (DEBUG) Log.d(TAG, "scheduling for " + nextTimeUs + " and " + nowUs);
                scheduleNotification(NOTIFY_TIME, nextTimeUs - nowUs);
            } else {
                mEventHandler.removeMessages(NOTIFY);
                // no more callbacks
            }

            for (MediaTimeProvider.OnMediaTimeListener listener: activatedListeners) {
                listener.onTimedEvent(nowUs);
            }
        }

        private long getEstimatedTime(long nanoTime, boolean monotonic) {
            if (mPaused) {
                mLastReportedTime = mLastTimeUs + mTimeAdjustment;
            } else {
                long timeSinceRead = (nanoTime - mLastNanoTime) / 1000;
                mLastReportedTime = mLastTimeUs + timeSinceRead;
                if (mTimeAdjustment > 0) {
                    long adjustment =
                        mTimeAdjustment - timeSinceRead / TIME_ADJUSTMENT_RATE;
                    if (adjustment <= 0) {
                        mTimeAdjustment = 0;
                    } else {
                        mLastReportedTime += adjustment;
                    }
                }
            }
            return mLastReportedTime;
        }

        public long getCurrentTimeUs(boolean refreshTime, boolean monotonic)
                throws IllegalStateException {
            synchronized (this) {
                // we always refresh the time when the paused-state changes, because
                // we expect to have received the pause-change event delayed.
                if (mPaused && !refreshTime) {
                    return mLastReportedTime;
                }

                long nanoTime = System.nanoTime();
                if (refreshTime ||
                        nanoTime >= mLastNanoTime + MAX_NS_WITHOUT_POSITION_CHECK) {
                    try {
                        mLastTimeUs = mPlayer.getCurrentPosition() * 1000;
                        mPaused = !mPlayer.isPlaying();
                        if (DEBUG) Log.v(TAG, (mPaused ? "paused" : "playing") + " at " + mLastTimeUs);
                    } catch (IllegalStateException e) {
                        if (mPausing) {
                            // if we were pausing, get last estimated timestamp
                            mPausing = false;
                            getEstimatedTime(nanoTime, monotonic);
                            mPaused = true;
                            if (DEBUG) Log.d(TAG, "illegal state, but pausing: estimating at " + mLastReportedTime);
                            return mLastReportedTime;
                        }
                        // TODO get time when prepared
                        throw e;
                    }
                    mLastNanoTime = nanoTime;
                    if (monotonic && mLastTimeUs < mLastReportedTime) {
                        /* have to adjust time */
                        mTimeAdjustment = mLastReportedTime - mLastTimeUs;
                        if (mTimeAdjustment > 1000000) {
                            // schedule seeked event if time jumped significantly
                            // TODO: do this properly by introducing an exception
                            mStopped = false;
                            mSeeking = true;
                            scheduleNotification(NOTIFY_SEEK, 0 /* delay */);
                        }
                    } else {
                        mTimeAdjustment = 0;
                    }
                }

                return getEstimatedTime(nanoTime, monotonic);
            }
        }

        private class EventHandler extends Handler {
            public EventHandler(Looper looper) {
                super(looper);
            }

            @Override
            public void handleMessage(Message msg) {
                if (msg.what == NOTIFY) {
                    switch (msg.arg1) {
                    case NOTIFY_TIME:
                        notifyTimedEvent(false /* refreshTime */);
                        break;
                    case REFRESH_AND_NOTIFY_TIME:
                        notifyTimedEvent(true /* refreshTime */);
                        break;
                    case NOTIFY_STOP:
                        notifyStop();
                        break;
                    case NOTIFY_SEEK:
                        notifySeek();
                        break;
                    }
                }
            }
        }
    }
}
