sh ant_jar.sh
cp -f /tmp/build/jar/k2player.jar demo/k2mediademo/libs/
mkdir demo/k2mediademo/libs/armeabi-v7a
(cd jni;ndk-build)
cp -f libs/armeabi-v7a/lib*.so demo/k2mediademo/libs/armeabi-v7a/
(cd demo/k2mediademo; ant debug)
cp demo/k2mediademo/bin/MediaActivity-debug.apk /tmp/

exit 0
