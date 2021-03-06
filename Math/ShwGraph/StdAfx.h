#pragma once

#define OEMRESOURCE 

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include "targetver.h"

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#define _CRT_SECURE_NO_WARNINGS 1

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <MyUtil.h>
#include <Math.h>
#include <MFCUtil/WinTools.h>
#include <MFCUtil/CoordinateSystem/CoordinateSystem.h>
#include <Math/Polynomial.h>

#include "resource.h"
