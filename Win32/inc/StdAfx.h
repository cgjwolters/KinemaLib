// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__9E8DA168_9730_42B7_A278_273E88F40B2A__INCLUDED_)
#define AFX_STDAFX_H__9E8DA168_9730_42B7_A278_273E88F40B2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>

//#import "libid:{00020905-0000-0000-C000-000000000046}" raw_interfaces_only, \
//  named_guids, \
//  rename( "FindText", "WrdFindText" ), rename( "ExitWindows", "WrdExitWindows" ), \
//  rename( "RGB", "WrdRGB" ), rename( "DocumentProperties", "WrdDocumentProperties" )

#import "C:\Program Files\Microsoft Office\OFFICE11\MSWORD.OLB" raw_interfaces_only, \
  named_guids, \
  rename( "FindText", "WrdFindText" ), rename( "ExitWindows", "WrdExitWindows" ), \
  rename( "RGB", "WrdRGB" ), rename( "DocumentProperties", "WrdDocumentProperties" )

//#import "C:\Program Files\Microsoft Office\OFFICE\MSWORD9.OLB" raw_interfaces_only, \
//  named_guids, \
//  rename( "FindText", "WrdFindText" ), rename( "ExitWindows", "WrdExitWindows" ), \
//  rename( "RGB", "WrdRGB" ), rename( "DocumentProperties", "WrdDocumentProperties" )
//
// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9E8DA168_9730_42B7_A278_273E88F40B2A__INCLUDED_)
