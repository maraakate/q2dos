@echo off

if not defined Q2DOSDEVBASE ( goto failedq2dosdevbase )
if not defined SEVENZIPPATH ( goto failed7zpath )

:startbuild
call setenv.bat
cd /D "%Q2DOSDEVBASE%"

make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj USE_SSE=1
upx -9 q2sse.exe

make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj USE_SSE=1 REF_DXE=0 REF_STATIC_GL=1 REFGL_DRIVER=sage
copy /y q2fx.exe q2sage.exe
upx -9 q2sage.exe

make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj USE_SSE=1 REF_DXE=0 REF_STATIC_GL=1 REFGL_DRIVER=mesa
upx -9 q2fx.exe

cd ref_gl
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj USE_SSE=1
cd..

del Q2DOS_SSE_LATEST.7z
rd /s /q release

md release
cd release

@echo on
echo SSE is for Pentium 3 or later processors ONLY.  It takes advantage of additional CPU extensions that can speed up graphics processing.  These SSE builds are entirely UNSUPPORTED and provided only as a convenience!  Please do not send us bug reports from them, Thanks! > !READ_SSE.txt
@echo off
cd..

copy q2sse.exe release
copy q2fx.exe release
copy q2sage.exe release
copy ref_gl\ref_gl.dxe release
copy readme.fx release
copy readme.txt release

"%SEVENZIPPATH%\7z.exe" a -mx9 -mmt "%Q2DOSDEVBASE%\Q2DOS_SSE_LATEST.7Z" "%Q2DOSDEVBASE%\RELEASE\*"

cd /D "%Q2DOSDEVBASE%"
rd /s /q "%Q2DOSDEVBASE%\release"
goto end

:failedq2dosdevbase
echo Q2DOSDEVBASE not defined!
pause
goto end

:failed7zpath
echo SEVENZIPPATH not defined!
pause
goto end

:end
