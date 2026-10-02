@echo off

if not defined Q2DOSDEVBASE ( goto failedq2dosdevbase )
if not defined SEVENZIPPATH ( goto failed7zpath )

:startbuild
call setenv.bat
call makeallDXE.bat %1 %2 %3 %4 %5 %6 %7 %8 %9

cd /D "%Q2DOSDEVBASE%"
del Q2DOS_EXE_LATEST.7Z
rd /s /q release

md release
cd release
md baseq2
md ctf
md 3zb2
md ace
md action
md chaos
md dday
md rogue
md xatrix
md zaero
cd ..

copy q2.exe release
copy q2.dbg release
copy ref_soft\ref_soft.dxe release
copy ref_gl\ref_gl.dxe release
copy cwsdpmi.exe release
copy readme.txt release
copy readme.fx release
copy Goa\gamespy.dxe release
copy game\gamex86.dxe release\baseq2
copy ctf\gamex86.dxe release\ctf
copy 3zb2\gamex86.dxe release\3zb2
copy 3zb2\3ZB.cfg release\3zb2
copy acebot\gamex86.dxe release\ace
copy action\gamex86.dxe release\action
copy chaos\gamex86.dxe release\chaos
copy dday\gamex86.dxe release\dday
copy rsrc\gamex86.dxe release\rogue
copy xsrc\gamex86.dxe release\xatrix
copy zaero\gamex86.dxe release\zaero
xcopy dos\3rdparty\lib_dxe release /E

REM copy a default gl.dxe
copy dos\3rdparty\lib_dxe\opengl\fxmesa\gl.dxe release

"%SEVENZIPPATH%\7z.exe" a -mx9 -mmt "%Q2DOSDEVBASE%\Q2DOS_EXE_LATEST.7Z" "%Q2DOSDEVBASE%\RELEASE\*"

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
