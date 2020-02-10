# Microsoft Developer Studio Project File - Name="Contour" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Contour - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Contour.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Contour.mak" CFG="Contour - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Contour - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Contour - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Contour - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W2 /GX /O2 /I "inc" /I "../cppstd/inc" /I "../../incOld/1.0" /I "../../inc/1.0" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\Contour.lib"

!ELSEIF  "$(CFG)" == "Contour - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W2 /Gm /GX /ZI /Od /I "inc" /I "../cppstd/inc" /I "../../incOld/1.0" /I "../../inc/1.0" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\Contourd.lib"

!ENDIF 

# Begin Target

# Name "Contour - Win32 Release"
# Name "Contour - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\contisct1.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Contisct2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\contour.cpp
# End Source File
# Begin Source File

SOURCE=.\src\contouri.cpp
# End Source File
# Begin Source File

SOURCE=.\src\el_arc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\el_cir.cpp
# End Source File
# Begin Source File

SOURCE=.\src\el_line.cpp
# End Source File
# Begin Source File

SOURCE=.\src\elem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\geo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\isect.cpp
# End Source File
# Begin Source File

SOURCE=.\src\rect.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sub_rect.cpp
# End Source File
# Begin Source File

SOURCE=.\src\template.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\cntpanic.hi
# End Source File
# Begin Source File

SOURCE=.\src\contisct.hi
# End Source File
# Begin Source File

SOURCE=.\inc\contour.h
# End Source File
# Begin Source File

SOURCE=.\src\contouri.hi
# End Source File
# Begin Source File

SOURCE=.\inc\el_arc.h
# End Source File
# Begin Source File

SOURCE=.\inc\el_cir.h
# End Source File
# Begin Source File

SOURCE=.\inc\el_info.h
# End Source File
# Begin Source File

SOURCE=.\inc\el_line.h
# End Source File
# Begin Source File

SOURCE=.\inc\elem.h
# End Source File
# Begin Source File

SOURCE=.\inc\geo.h
# End Source File
# Begin Source File

SOURCE=.\inc\isect.h
# End Source File
# Begin Source File

SOURCE=.\inc\rect.h
# End Source File
# Begin Source File

SOURCE=.\src\sub_rect.hi
# End Source File
# End Group
# End Target
# End Project
