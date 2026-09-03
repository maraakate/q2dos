#!/bin/sh

PKG_NAME=Q2DOS_SSE_LATEST.7z
rm -f $PKG_NAME

./clean.sh

. ./cross_defs.dj

mkdir -p _build || exit 1

make CC=$TARGET-gcc -f Makefile.dj clean $*     || exit 1
make CC=$TARGET-gcc -f Makefile.dj USE_SSE=1 $*     || exit 1
upx -9 q2sse.exe || exit 1

make CC=$TARGET-gcc -f Makefile.dj clean $*     || exit 1
make CC=$TARGET-gcc -f Makefile.dj USE_SSE=1 REF_DXE=0 REF_STATIC_GL=1 REFGL_DRIVER=sage $*     || exit 1
cp q2fx.exe q2sage.exe
upx -9 q2sage.exe || exit 1

rm q2fx.exe
make CC=$TARGET-gcc -f Makefile.dj clean $*     || exit 1
make CC=$TARGET-gcc -f Makefile.dj USE_SSE=1 REF_DXE=0 REF_STATIC_GL=1 REFGL_DRIVER=mesa $*     || exit 1
upx -9 q2fx.exe || exit 1

make CC=$TARGET-gcc -C ref_gl -f Makefile.dj clean $* || exit 1
make CC=$TARGET-gcc -C ref_gl -f Makefile.dj USE_SSE=1 $* || exit 1

cp -p q2sse.exe _build
cp -p q2fx.exe _build
cp -p q2sage.exe _build
cp -p ref_gl/ref_gl.dxe _build
cp -p readme.txt _build
cp -p readme.fx _build

cd _build || exit 1
echo "SSE is for Pentium 3 or later processors ONLY.  It takes advantage of additional CPU extensions that can speed up graphics processing.  These SSE builds are entirely UNSUPPORTED and provided only as a convenience!  Please do not send us bug reports from them, Thanks!" > !READ_SSE.txt
unix2dos -k !READ_SSE.txt
unix2dos -k readme.txt
unix2dos -k readme.fx
find * -type d|xargs chmod 755
find * -type f|xargs chmod 644
7z a -mx9 -mmt ../$PKG_NAME *

