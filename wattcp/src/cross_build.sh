#!/bin/sh

. ../../cross_defs.dj

exec make -f djgpp.mak CROSS=1 $*
