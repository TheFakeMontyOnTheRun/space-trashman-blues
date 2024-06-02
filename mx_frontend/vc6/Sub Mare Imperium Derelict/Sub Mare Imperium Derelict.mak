# Microsoft Developer Studio Generated NMAKE File, Based on Sub Mare Imperium Derelict.dsp
!IF "$(CFG)" == ""
CFG=Sub Mare Imperium Derelict - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Sub Mare Imperium Derelict - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Sub Mare Imperium Derelict - Win32 Release" && "$(CFG)" != "Sub Mare Imperium Derelict - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sub Mare Imperium Derelict.mak" CFG="Sub Mare Imperium Derelict - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sub Mare Imperium Derelict - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Sub Mare Imperium Derelict - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Sub Mare Imperium Derelict.exe"


CLEAN :
	-@erase "$(INTDIR)\CGDIRenderer.obj"
	-@erase "$(INTDIR)\Common.obj"
	-@erase "$(INTDIR)\Core.obj"
	-@erase "$(INTDIR)\Crawler.obj"
	-@erase "$(INTDIR)\CreditsScreen.obj"
	-@erase "$(INTDIR)\CTile3DProperties.obj"
	-@erase "$(INTDIR)\Derelict.obj"
	-@erase "$(INTDIR)\Dungeon.obj"
	-@erase "$(INTDIR)\EDirection_Utils.obj"
	-@erase "$(INTDIR)\Engine.obj"
	-@erase "$(INTDIR)\Events.obj"
	-@erase "$(INTDIR)\FixP.obj"
	-@erase "$(INTDIR)\GameMenu.obj"
	-@erase "$(INTDIR)\Globals.obj"
	-@erase "$(INTDIR)\HackingMinigameRules.obj"
	-@erase "$(INTDIR)\HackingScreen.obj"
	-@erase "$(INTDIR)\HelpScreen.obj"
	-@erase "$(INTDIR)\LoadBitmap.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\MainMenu.obj"
	-@erase "$(INTDIR)\MapWithCharKey.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\PackedFileReader.obj"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\Renderer.obj"
	-@erase "$(INTDIR)\RendererRasterization.obj"
	-@erase "$(INTDIR)\RendererTesselation.obj"
	-@erase "$(INTDIR)\Sub Mare Imperium.res"
	-@erase "$(INTDIR)\UI.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\Vec.obj"
	-@erase "$(INTDIR)\VisibilityStrategy.obj"
	-@erase "$(OUTDIR)\Sub Mare Imperium Derelict.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /O2 /I "c:\space-trashman-blues\common\include" /I "c:\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\core\include" /I "c:\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /D "CLI_BUILD" /D "SUPPORTS_HACKING_MINIGAME" /D "PAGE_FLIP_ANIMATION" /D "INCLUDE_ITEM_DESCRIPTIONS" /D "ENDIANESS_AWARE" /D "EMIT_QUIT_OPTION" /D XRES_FRAMEBUFFER=320 /D YRES_FRAMEBUFFER=200 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x816 /fo"$(INTDIR)\Sub Mare Imperium.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sub Mare Imperium Derelict.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib shell32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"$(OUTDIR)\Sub Mare Imperium Derelict.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CGDIRenderer.obj" \
	"$(INTDIR)\Common.obj" \
	"$(INTDIR)\Core.obj" \
	"$(INTDIR)\Crawler.obj" \
	"$(INTDIR)\CreditsScreen.obj" \
	"$(INTDIR)\CTile3DProperties.obj" \
	"$(INTDIR)\Derelict.obj" \
	"$(INTDIR)\Dungeon.obj" \
	"$(INTDIR)\EDirection_Utils.obj" \
	"$(INTDIR)\Engine.obj" \
	"$(INTDIR)\Events.obj" \
	"$(INTDIR)\FixP.obj" \
	"$(INTDIR)\GameMenu.obj" \
	"$(INTDIR)\Globals.obj" \
	"$(INTDIR)\HackingScreen.obj" \
	"$(INTDIR)\HelpScreen.obj" \
	"$(INTDIR)\LoadBitmap.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\MainMenu.obj" \
	"$(INTDIR)\MapWithCharKey.obj" \
	"$(INTDIR)\PackedFileReader.obj" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\UI.obj" \
	"$(INTDIR)\Vec.obj" \
	"$(INTDIR)\VisibilityStrategy.obj" \
	"$(INTDIR)\Sub Mare Imperium.res" \
	"$(INTDIR)\RendererTesselation.obj" \
	"$(INTDIR)\RendererRasterization.obj" \
	"$(INTDIR)\Renderer.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\HackingMinigameRules.obj"

"$(OUTDIR)\Sub Mare Imperium Derelict.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Sub Mare Imperium Derelict.exe"


CLEAN :
	-@erase "$(INTDIR)\CGDIRenderer.obj"
	-@erase "$(INTDIR)\Common.obj"
	-@erase "$(INTDIR)\Core.obj"
	-@erase "$(INTDIR)\Crawler.obj"
	-@erase "$(INTDIR)\CreditsScreen.obj"
	-@erase "$(INTDIR)\CTile3DProperties.obj"
	-@erase "$(INTDIR)\Derelict.obj"
	-@erase "$(INTDIR)\Dungeon.obj"
	-@erase "$(INTDIR)\EDirection_Utils.obj"
	-@erase "$(INTDIR)\Engine.obj"
	-@erase "$(INTDIR)\Events.obj"
	-@erase "$(INTDIR)\FixP.obj"
	-@erase "$(INTDIR)\GameMenu.obj"
	-@erase "$(INTDIR)\Globals.obj"
	-@erase "$(INTDIR)\HackingMinigameRules.obj"
	-@erase "$(INTDIR)\HackingScreen.obj"
	-@erase "$(INTDIR)\HelpScreen.obj"
	-@erase "$(INTDIR)\LoadBitmap.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\MainMenu.obj"
	-@erase "$(INTDIR)\MapWithCharKey.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\PackedFileReader.obj"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\Renderer.obj"
	-@erase "$(INTDIR)\RendererRasterization.obj"
	-@erase "$(INTDIR)\RendererTesselation.obj"
	-@erase "$(INTDIR)\Sub Mare Imperium.res"
	-@erase "$(INTDIR)\UI.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\Vec.obj"
	-@erase "$(INTDIR)\VisibilityStrategy.obj"
	-@erase "$(OUTDIR)\Sub Mare Imperium Derelict.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "c:\space-trashman-blues\common\include" /I "c:\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\core\include" /I "c:\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /D "CLI_BUILD" /D "SUPPORTS_HACKING_MINIGAME" /D "PAGE_FLIP_ANIMATION" /D "INCLUDE_ITEM_DESCRIPTIONS" /D "ENDIANESS_AWARE" /D "EMIT_QUIT_OPTION" /D XRES_FRAMEBUFFER=320 /D YRES_FRAMEBUFFER=200 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sub Mare Imperium.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sub Mare Imperium Derelict.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /out:"$(OUTDIR)\Sub Mare Imperium Derelict.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CGDIRenderer.obj" \
	"$(INTDIR)\Common.obj" \
	"$(INTDIR)\Core.obj" \
	"$(INTDIR)\Crawler.obj" \
	"$(INTDIR)\CreditsScreen.obj" \
	"$(INTDIR)\CTile3DProperties.obj" \
	"$(INTDIR)\Derelict.obj" \
	"$(INTDIR)\Dungeon.obj" \
	"$(INTDIR)\EDirection_Utils.obj" \
	"$(INTDIR)\Engine.obj" \
	"$(INTDIR)\Events.obj" \
	"$(INTDIR)\FixP.obj" \
	"$(INTDIR)\GameMenu.obj" \
	"$(INTDIR)\Globals.obj" \
	"$(INTDIR)\HackingScreen.obj" \
	"$(INTDIR)\HelpScreen.obj" \
	"$(INTDIR)\LoadBitmap.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\MainMenu.obj" \
	"$(INTDIR)\MapWithCharKey.obj" \
	"$(INTDIR)\PackedFileReader.obj" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\UI.obj" \
	"$(INTDIR)\Vec.obj" \
	"$(INTDIR)\VisibilityStrategy.obj" \
	"$(INTDIR)\Sub Mare Imperium.res" \
	"$(INTDIR)\RendererTesselation.obj" \
	"$(INTDIR)\RendererRasterization.obj" \
	"$(INTDIR)\Renderer.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\HackingMinigameRules.obj"

"$(OUTDIR)\Sub Mare Imperium Derelict.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Sub Mare Imperium Derelict.dep")
!INCLUDE "Sub Mare Imperium Derelict.dep"
!ELSE 
!MESSAGE Warning: cannot find "Sub Mare Imperium Derelict.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Release" || "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Debug"
SOURCE=..\..\base3d\src\Win32Version\CGDIRenderer.c

"$(INTDIR)\CGDIRenderer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\Common.c

"$(INTDIR)\Common.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\core\src\Core.c

"$(INTDIR)\Core.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\Crawler.c

"$(INTDIR)\Crawler.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\CreditsScreen.c

"$(INTDIR)\CreditsScreen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\CTile3DProperties.c

"$(INTDIR)\CTile3DProperties.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\core\src\Derelict.c

"$(INTDIR)\Derelict.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\Dungeon.c

"$(INTDIR)\Dungeon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\EDirection_Utils.c

"$(INTDIR)\EDirection_Utils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\Engine.c

"$(INTDIR)\Engine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\Events.c

"$(INTDIR)\Events.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\FixP.c

"$(INTDIR)\FixP.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\GameMenu.c

"$(INTDIR)\GameMenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\Globals.c

"$(INTDIR)\Globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\Common\Src\HackingMinigameRules.c

"$(INTDIR)\HackingMinigameRules.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\HackingScreen.c

"$(INTDIR)\HackingScreen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\HelpScreen.c

"$(INTDIR)\HelpScreen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\LoadBitmap.c

"$(INTDIR)\LoadBitmap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\Main.c

!IF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /O2 /I "c:\space-trashman-blues\common\include" /I "c:\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\core\include" /I "c:\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /D "CLI_BUILD" /D "SUPPORTS_HACKING_MINIGAME" /D "PAGE_FLIP_ANIMATION" /D "INCLUDE_ITEM_DESCRIPTIONS" /D "ENDIANESS_AWARE" /D "EMIT_QUIT_OPTION" /D XRES_FRAMEBUFFER=320 /D YRES_FRAMEBUFFER=200 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "c:\space-trashman-blues\common\include" /I "c:\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\core\include" /I "c:\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /D "CLI_BUILD" /D "SUPPORTS_HACKING_MINIGAME" /D "PAGE_FLIP_ANIMATION" /D "INCLUDE_ITEM_DESCRIPTIONS" /D "ENDIANESS_AWARE" /D "EMIT_QUIT_OPTION" /D XRES_FRAMEBUFFER=320 /D YRES_FRAMEBUFFER=200 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\..\..\common\src\MainMenu.c

"$(INTDIR)\MainMenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\MapWithCharKey.c

"$(INTDIR)\MapWithCharKey.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\Common\Src\Mesh.c

"$(INTDIR)\Mesh.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\PackedFileReader.c

"$(INTDIR)\PackedFileReader.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\core\src\Parser.c

"$(INTDIR)\Parser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\Base3d\Src\Renderer.c

"$(INTDIR)\Renderer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\Base3d\Src\RendererRasterization.c

"$(INTDIR)\RendererRasterization.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\Base3d\Src\RendererTesselation.c

"$(INTDIR)\RendererTesselation.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\UI.c

"$(INTDIR)\UI.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\Vec.c

"$(INTDIR)\Vec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\VisibilityStrategy.c

"$(INTDIR)\VisibilityStrategy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Sub Mare Imperium.rc"

"$(INTDIR)\Sub Mare Imperium.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

