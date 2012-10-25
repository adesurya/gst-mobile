if [ $# != 1 ]; then
    echo "usage: $0 module"
    exit 1
fi

mod=$1
ver="0.10"
modlist="gsth264parse gstsdpelem gstqtmux "

if [ $mod = "all" ]; then
    echo "[*] building all ...."
elif [ $mod = "clean" ]; then
    rm -f Android.mk Application.mk
    rm -rf ../obj/ ../libs/
    exit 0
else
    if [ ! -f ./Android-$mod.mk ]; then 
        echo "[*] no module $mod"
        exit 1
    fi
    modlist="$mod "
fi


for m in $modlist; 
do
    app=$m
    cat > Application.mk << EOF
APP_MODULES := $app
APP_OPTIM := release
APP_ABI := armeabi-v7a
APP_STL := stlport_static
EOF
    cp Android-$m.mk Android.mk
    ndk-build
done

exit 0
