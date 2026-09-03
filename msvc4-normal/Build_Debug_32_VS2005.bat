@echo off
set Q2DOSBINDIR=Debug

if not defined Q2DOSDEVBASE ( goto failed )

call "%Q2DOSDEVBASE%\clean.bat"
call vcvars32.bat
call "%VS80COMNTOOLS%vsvars32.bat"

cd /D "%Q2DOSDEVBASE%\msvc4-normal"

:startcopy
devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "q2ded"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "gamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "ref_soft"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "ref_gl"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "quake2"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "gamespysdk"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "3zb2gamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "acegamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "actiongamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "chaosgamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "coopgamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "ctfgamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "ddaygamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "roguegamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "xatrixgamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|Win32" /Project "zaerogamex86"

CALL mvd.bat
goto end

:failed
echo Q2DOSDEVBASE not defined!
pause
goto end

:end
set Q2DOSBINDIR=
