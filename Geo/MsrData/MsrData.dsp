# Microsoft Developer Studio Project File - Name="MsrData" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MsrData - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MsrData.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MsrData.mak" CFG="MsrData - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MsrData - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MsrData - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MsrData - Win32 Release"

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
# ADD CPP /nologo /MT /W2 /GX /O2 /I "inc" /I "VExp/inc" /I "../CppStd/inc" /I "../Contour/inc" /I "../Approx/inc" /I "../../incOld/1.0" /I "../../inc/1.0" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\MsrData.lib"

!ELSEIF  "$(CFG)" == "MsrData - Win32 Debug"

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
# ADD CPP /nologo /MTd /W2 /Gm /GX /ZI /Od /I "inc" /I "VExp/inc" /I "../CppStd/inc" /I "../Contour/inc" /I "../Approx/inc" /I "../../incOld/1.0" /I "../../inc/1.0" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\MsrDatad.lib"

!ENDIF 

# Begin Target

# Name "MsrData - Win32 Release"
# Name "MsrData - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\VExp\src\db1.cpp
# End Source File
# Begin Source File

SOURCE=.\VExp\src\db2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DxfOut.cpp
# End Source File
# Begin Source File

SOURCE=.\VExp\src\ent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MsrCont.cpp
# End Source File
# Begin Source File

SOURCE=.\VExp\src\TRACE.CPP
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\VExp\inc\csarray.h
# End Source File
# Begin Source File

SOURCE=.\VExp\inc\db.h
# End Source File
# Begin Source File

SOURCE=.\inc\DxfOut.h
# End Source File
# Begin Source File

SOURCE=.\VExp\inc\ent.h
# End Source File
# Begin Source File

SOURCE=.\VExp\inc\layer.h
# End Source File
# Begin Source File

SOURCE=.\inc\MsrCont.h
# End Source File
# Begin Source File

SOURCE=.\VExp\inc\TRACE.H
# End Source File
# Begin Source File

SOURCE=.\VExp\inc\wfile.h
# End Source File
# End Group
# End Target
# End Project
