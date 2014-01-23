#!/bin/sh

if [ $# -ne 1 ]; then
    echo "usage: $0 libdir"
    exit 1
fi

libdir=$1
if test ! -d "$libdir"; then
    echo "error: $libdir is invalid"
    exit 1
fi

solist=`ls $libdir/lib*`
for so in $solist; do
    if test -h "$so"; then
        rm -f "$so"
    fi
done

solist=`ls $libdir/lib*.so.*`
for so in $solist; do
    new_so=${so%.so*}.so
    mv -f "$so" "$new_so"
done

exit 0
