//////////////////////////////////////////////////////////////////////////
//
// Getting the framework settings for this library/program
//
#pragma once

#define WIN32_LEAN_AND_MEAN                     // Exclude rarely-used stuff from Windows headers
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN                            // Exclude rarely-used stuff from Windows headers
#endif
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	    // some CString constructors will be explicit
// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

// Makes use of MFC
#include <afx.h>

//////////////////////////////////////////////////////////////////////////
//
// Can be extended beyond this point with extra MFC requirements
//
//////////////////////////////////////////////////////////////////////////

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxtempl.h>			  // MFC template classes
#include <afxsock.h>        // MFC Sockets
#include <afxdisp.h>        // Required by afxpriv.h
#include <afxpriv.h>        // to get access to the T2A macro
#include <afxdtctl.h>		    // MFC support for Internet Explorer 4 Common Controls
#include <afxcmn.h>			    // MFC support for Windows Common Controls
