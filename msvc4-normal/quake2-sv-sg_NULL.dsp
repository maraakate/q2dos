# Microsoft Developer Studio Project File - Name="quake2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=quake2 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "quake2-sv-sg_NULL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "quake2-sv-sg_NULL.mak" CFG="quake2 - Win32 Release"
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
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "REF_HARD_LINKED" /D "GAME_HARD_LINKED" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "REF_HARD_LINKED" /D "GAME_HARD_LINKED" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib /nologo /subsystem:console /map /debug /machine:I386
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "quake2 - Win32 Release"
# Name "quake2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\null\cd_null.c
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

SOURCE=..\GAME\g_ai.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_chase.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_cmds.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_combat.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_func.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_items.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_main.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_misc.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_monster.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_phys.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_save.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_spawn.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_target.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_trigger.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_turret.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_utils.c
# End Source File
# Begin Source File

SOURCE=..\GAME\g_weapon.c
# End Source File
# Begin Source File

SOURCE=..\null\glimp_null.c
# End Source File
# Begin Source File

SOURCE=..\null\in_null.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\keys.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_actor.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_berserk.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_boss2.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_boss3.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_boss31.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_boss32.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_brain.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_chick.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_flash.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_flipper.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_float.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_flyer.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_gladiator.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_gunner.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_hover.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_infantry.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_insane.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_medic.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_move.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_mutant.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_parasite.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_soldier.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_supertank.c
# End Source File
# Begin Source File

SOURCE=..\GAME\m_tank.c
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

SOURCE=..\NULL\net_null.c
# End Source File
# Begin Source File

SOURCE=..\GAME\p_client.c
# End Source File
# Begin Source File

SOURCE=..\GAME\p_hud.c
# End Source File
# Begin Source File

SOURCE=..\GAME\p_trail.c
# End Source File
# Begin Source File

SOURCE=..\GAME\p_view.c
# End Source File
# Begin Source File

SOURCE=..\GAME\p_weapon.c
# End Source File
# Begin Source File

SOURCE=..\QCOMMON\pmove.c
# End Source File
# Begin Source File

SOURCE=..\GAME\q_shared.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\qmenu.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_aclip.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_alias.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_bsp.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_draw.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_edge.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_image.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_light.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_main.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_misc.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_model.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_part.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_poly.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_polyse.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_rast.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_scan.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_sprite.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_surf.c
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

SOURCE=..\null\snddma_null.c
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

SOURCE=..\null\swimp_null.c
# End Source File
# Begin Source File

SOURCE=..\null\sys_null.c
# End Source File
# Begin Source File

SOURCE=..\null\vid_null.c
# End Source File
# Begin Source File

SOURCE=..\CLIENT\x86.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\ref_soft\adivtab.h
# End Source File
# Begin Source File

SOURCE=..\client\anorms.h
# End Source File
# Begin Source File

SOURCE=..\ref_soft\anorms.h
# End Source File
# Begin Source File

SOURCE="C:\Program Files\Microsoft Visual Studio\VC98\Include\BASETSD.H"
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

SOURCE=..\game\g_local.h
# End Source File
# Begin Source File

SOURCE=..\game\game.h
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\goaceng.h
# End Source File
# Begin Source File

SOURCE=..\client\input.h
# End Source File
# Begin Source File

SOURCE=..\client\keys.h
# End Source File
# Begin Source File

SOURCE=..\game\m_actor.h
# End Source File
# Begin Source File

SOURCE=..\game\m_berserk.h
# End Source File
# Begin Source File

SOURCE=..\game\m_boss2.h
# End Source File
# Begin Source File

SOURCE=..\game\m_boss31.h
# End Source File
# Begin Source File

SOURCE=..\game\m_boss32.h
# End Source File
# Begin Source File

SOURCE=..\game\m_brain.h
# End Source File
# Begin Source File

SOURCE=..\game\m_chick.h
# End Source File
# Begin Source File

SOURCE=..\game\m_flipper.h
# End Source File
# Begin Source File

SOURCE=..\game\m_float.h
# End Source File
# Begin Source File

SOURCE=..\game\m_flyer.h
# End Source File
# Begin Source File

SOURCE=..\game\m_gladiator.h
# End Source File
# Begin Source File

SOURCE=..\game\m_gunner.h
# End Source File
# Begin Source File

SOURCE=..\game\m_hover.h
# End Source File
# Begin Source File

SOURCE=..\game\m_infantry.h
# End Source File
# Begin Source File

SOURCE=..\game\m_insane.h
# End Source File
# Begin Source File

SOURCE=..\game\m_medic.h
# End Source File
# Begin Source File

SOURCE=..\game\m_mutant.h
# End Source File
# Begin Source File

SOURCE=..\game\m_parasite.h
# End Source File
# Begin Source File

SOURCE=..\game\m_player.h
# End Source File
# Begin Source File

SOURCE=..\game\m_soldier.h
# End Source File
# Begin Source File

SOURCE=..\game\m_supertank.h
# End Source File
# Begin Source File

SOURCE=..\game\m_tank.h
# End Source File
# Begin Source File

SOURCE=..\game\q_shared.h
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

SOURCE=..\ref_soft\r_local.h
# End Source File
# Begin Source File

SOURCE=..\ref_soft\r_model.h
# End Source File
# Begin Source File

SOURCE=..\ref_soft\rand1k.h
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

SOURCE=..\client\sound.h
# End Source File
# Begin Source File

SOURCE=..\client\vid.h
# End Source File
# Begin Source File

SOURCE=..\dos\zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
