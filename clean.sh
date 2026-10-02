#!/bin/sh

find . -name "*.o"|xargs rm -f
find . -name "game*.dxe"|xargs rm -f
find . -name "game*.dll"|xargs rm -f
find . -name "ref_*.dxe"|xargs rm -f
find . -name "ref_*.dll"|xargs rm -f
find . -name "*.res"|xargs rm -f
rm -f q2*.exe
rm -f q2*.dbg
rm -rf _build
