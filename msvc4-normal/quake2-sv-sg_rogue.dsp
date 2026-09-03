# Microsoft Developer Studio Project File - Name="quake2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=quake2 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "quake2-sv-sg_rogue.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "quake2-sv-sg_rogue.mak" CFG="quake2 - Win32 Release"
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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../win32/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "id386=1" /D "REF_HARD_LINKED" /D "GAME_HARD_LINKED" /D "CLIENT_SPLIT_NETFRAME" /D "GAMESPY" /D "GAMESPY_HARD_LINKED" /D "OGG_SUPPORT" /D "USE_CURL" /D "USE_JOYSTICK" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libcmtd.lib"
# SUBTRACT LINK32 /force

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../win32/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "id386=1" /D "REF_HARD_LINKED" /D "GAME_HARD_LINKED" /D "CLIENT_SPLIT_NETFRAME" /D "GAMESPY" /D "GAMESPY_HARD_LINKED" /D "OGG_SUPPORT" /D "USE_CURL" /D "USE_JOYSTICK" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcmt.lib"

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
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\gserver.c
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\gserverlist.c
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\gutil.c
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\hashtable.c
# End Source File
# Begin Source File

SOURCE=..\Goa\nonport.c
# End Source File
# End Group
# Begin Group "Client"

# PROP Default_Filter ""
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

SOURCE=..\CLIENT\x86.c
# End Source File
# End Group
# Begin Group "Ref_Soft"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\REF_SOFT\r_aclip.c
# End Source File
# Begin Source File

SOURCE=..\ref_soft\r_aclipa.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_aclipa.asm
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_aclipa.asm
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

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

SOURCE=..\ref_soft\r_draw16.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_draw16.asm
InputName=r_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_draw16.asm
InputName=r_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ref_soft\r_drawa.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_drawa.asm
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_drawa.asm
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_edge.c
# End Source File
# Begin Source File

SOURCE=..\ref_soft\r_edgea.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_edgea.asm
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_edgea.asm
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

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

SOURCE=..\ref_soft\r_polysa.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_polysa.asm
InputName=r_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_polysa.asm
InputName=r_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

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

SOURCE=..\ref_soft\r_scana.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_scana.asm
InputName=r_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_scana.asm
InputName=r_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ref_soft\r_spr8.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_spr8.asm
InputName=r_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_spr8.asm
InputName=r_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_sprite.c
# End Source File
# Begin Source File

SOURCE=..\REF_SOFT\r_surf.c
# End Source File
# Begin Source File

SOURCE=..\ref_soft\r_surf8.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_surf8.asm
InputName=r_surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_surf8.asm
InputName=r_surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ref_soft\r_varsa.asm

!IF  "$(CFG)" == "quake2 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\ref_soft\r_varsa.asm
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "quake2 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\ref_soft\r_varsa.asm
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WIN32\cd_win.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\in_win.c
# End Source File
# Begin Source File

SOURCE=..\win32\net_wins.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\q_shwin.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\rw_ddraw.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\rw_dib.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\rw_imp.c
# End Source File
# Begin Source File

SOURCE=..\WIN32\snd_win.c
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
# End Group
# Begin Group "Server"

# PROP Default_Filter ""
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
# End Group
# Begin Group "Rogue Game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\rsrc\dm_ball.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\dm_tag.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_ai.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_chase.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_cmds.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_combat.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_func.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_items.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_main.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_misc.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_monster.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_newai.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_newdm.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_newfnc.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_newtarg.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_newtrig.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_newweap.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_phys.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_save.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_spawn.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_sphere.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_target.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_trigger.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_turret.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_utils.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_weapon.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_actor.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_berserk.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_boss2.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_boss3.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_boss31.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_boss32.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_brain.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_carrier.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_chick.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_flash.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_flipper.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_float.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_flyer.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_gladiator.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_gunner.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_hover.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_infantry.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_insane.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_medic.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_move.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_mutant.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_parasite.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_soldier.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_stalker.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_supertank.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_tank.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_turret.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_widow.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_widow2.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\p_client.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\p_hud.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\p_trail.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\p_view.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\p_weapon.c
# End Source File
# Begin Source File

SOURCE=..\rsrc\q_shared.c
# End Source File
# End Group
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

SOURCE=..\CLIENT\keys.c
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

SOURCE=..\QCOMMON\pmove.c
# End Source File
# Begin Source File

SOURCE=..\win32\q2.rc
# End Source File
# Begin Source File

SOURCE=..\CLIENT\qmenu.c
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

SOURCE=..\Goa\CEngine\darray.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\g_local.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\game.h
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\goaceng.h
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\gserver.h
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\gutil.h
# End Source File
# Begin Source File

SOURCE=..\Goa\CEngine\hashtable.h
# End Source File
# Begin Source File

SOURCE=..\client\input.h
# End Source File
# Begin Source File

SOURCE=..\client\keys.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_actor.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_berserk.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_boss2.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_boss31.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_boss32.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_brain.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_carrier.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_chick.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_flipper.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_float.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_flyer.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_gladiator.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_gunner.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_hover.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_infantry.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_insane.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_medic.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_mutant.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_parasite.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_player.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_rider.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_soldier.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_stalker.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_supertank.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_tank.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_turret.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_widow.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\m_widow2.h
# End Source File
# Begin Source File

SOURCE=..\Goa\nonport.h
# End Source File
# Begin Source File

SOURCE=..\rsrc\q_shared.h
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

SOURCE=..\win32\rw_win.h
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

SOURCE=..\win32\winquake.h
# End Source File
# Begin Source File

SOURCE=..\dos\zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=..\win32\libcurl.lib
# End Source File
# Begin Source File

SOURCE=..\win32\libogg.lib
# End Source File
# Begin Source File

SOURCE=..\win32\libvorbis.lib
# End Source File
# End Target
# End Project
