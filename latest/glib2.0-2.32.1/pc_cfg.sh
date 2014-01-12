
export LD_LIBRARY_PATH=/usr/local/lib

export LIBFFI_CFLAGS="-I/usr/local/lib/libffi-3.0.11/include"
export LIBFFI_LIBS="-L/usr/local/lib -lffi"

export CFLAGS="$CFLAGS $LIBFFI_CFLAGS"
export LDFLAGS="$LDFLAGS $LIBFFI_LIBS"

mkdir oldbld
cd oldbld
../configure 
