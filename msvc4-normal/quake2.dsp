# Microsoft Developer Studio Project File - Name="quake2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=quake2 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "quake2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "quake2.mak" CFG="quake2 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "quake2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "quake2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "quake2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../win32/include" /D "NDEBUG" /D id386=1 /D "WIN32" /D "_WINDOWS" /D "CLIENT_SPLIT_NETFRAME" /D "GAMESPY" /D "OGG_SUPPORT" /D "USE_CURL" /D "USE_JOYSTICK" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib libcurl.lib libvorbis.lib libogg.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libcmtd.lib" /out:".\Release/q2.exe" /libpath:"../win32/lib"

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../win32/include" /D "_DEBUG" /D id386=1 /D "WIN32" /D "_WINDOWS" /D "CLIENT_SPLIT_NETFRAME" /D "GAMESPY" /D "USE_CURL" /D "USE_JOYSTICK" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib libcurl.lib libvorbis.lib libogg.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMT" /out:".\Debug/q2.exe" /libpath:"../win32/lib"

!ENDIF 

# Begin Target

# Name "quake2 - Win32 Release"
# Name "quake2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Group "Gamespy"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Goa\CEngine\darray.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\gserver.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\gserverlist.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\gutil.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\hashtable.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\Goa\nonport.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "DOS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\dos\cd_dos.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\dos_v2.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\dxe.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\in_dos.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\net_watt.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\q_shdos.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\snd_gus.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\snd_pci.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\snd_sb.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\snddma_dos.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\snddma_wss.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\swimp_dos.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\sys_dos.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\vid_dos.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\vid_ext.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\vregset.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\dos\wss.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=..\WIN32\cd_win.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_cin.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_ents.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_fx.c
# End Source File
# Begin Source File

SOURCE=..\client\cl_http.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_input.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_inv.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_main.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_newfx.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_parse.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_pred.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_scrn.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_tent.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\cl_view.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\cmd.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\cmd_auto.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\cmodel.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\common.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\console.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\crc.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\cvar.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\files.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\in_win.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\keys.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_flash.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\md4.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\menu.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\net_chan.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\net_wins.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\pmove.c
# End Source File
# Begin Source File

SOURCE=..\GAME\q_shared.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\q_shwin.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\qmenu.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\snd_dma.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\snd_mem.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\snd_mix.c
# End Source File
# Begin Source File

SOURCE=..\client\snd_stream.c
# End Source File
# Begin Source File

SOURCE=..\client\snd_wavstream.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\snd_win.c
# End Source File
# Begin Source File

SOURCE=..\SERVER\sv_ccmds.c
# End Source File
# Begin Source File

SOURCE=..\SERVER\sv_ents.c
# End Source File
# Begin Source File

SOURCE=..\SERVER\sv_game.c
# End Source File
# Begin Source File

SOURCE=..\SERVER\sv_init.c
# End Source File
# Begin Source File

SOURCE=..\SERVER\sv_main.c
# End Source File
# Begin Source File

SOURCE=..\SERVER\sv_send.c
# End Source File
# Begin Source File

SOURCE=..\SERVER\sv_user.c
# End Source File
# Begin Source File

SOURCE=..\SERVER\sv_world.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\sys_win.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\vid_dll.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\vid_menu.c
# End Source File
# Begin Source File

SOURCE=..\win32\win_dedconsole.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\x86.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\client\adivtab.h
# End Source File
# Begin Source File

SOURCE=..\client\anorms.h
# End Source File
# Begin Source File

SOURCE=..\client\asm_i386.h
# End Source File
# Begin Source File

SOURCE=..\client\block16.h
# End Source File
# Begin Source File

SOURCE=..\client\block8.h
# End Source File
# Begin Source File

SOURCE=..\client\cdaudio.h
# End Source File
# Begin Source File

SOURCE=..\client\client.h
# End Source File
# Begin Source File

SOURCE=..\client\console.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\crc.h
# End Source File
# Begin Source File

SOURCE=..\client\gspy.h
# End Source File
# Begin Source File

SOURCE=..\client\input.h
# End Source File
# Begin Source File

SOURCE=..\client\keys.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=..\client\qmenu.h
# End Source File
# Begin Source File

SOURCE=..\client\ref.h
# End Source File
# Begin Source File

SOURCE=..\client\screen.h
# End Source File
# Begin Source File

SOURCE=..\server\server.h
# End Source File
# Begin Source File

SOURCE=..\client\snd_loc.h
# End Source File
# Begin Source File

SOURCE=..\client\snd_ogg.h
# End Source File
# Begin Source File

SOURCE=..\client\snd_wavstream.h
# End Source File
# Begin Source File

SOURCE=..\client\sound.h
# End Source File
# Begin Source File

SOURCE=..\client\vid.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\win32\q2.ico
# End Source File
# Begin Source File

SOURCE=..\win32\q2.rc
# End Source File
# End Group
# End Target
# End Project
