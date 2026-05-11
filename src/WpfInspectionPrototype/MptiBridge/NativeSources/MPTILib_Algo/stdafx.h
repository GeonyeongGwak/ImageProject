// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT



#include <atlcoll.h>
#include <thread>

#pragma warning(disable: 4800)		// warning : down size type casting
#pragma warning(disable: 4819)		// warning : charter type

#define  _USE_MATH_DEFINES	1;		// YJS 2017/01/04
//#define ALGORITHMTOOL_CPP_USE		//NYJ 2021/11/26

#include "compute.h"
#include "PInsp_Algo/InspParamDef_AlgoBase.h"

#if _MSC_VER < 1900
#ifdef _DEBUG
	// #pragma comment( lib, "libjasperd.lib" )
	// #pragma comment( lib, "libjpegd.lib" )
	// #pragma comment( lib, "libpngd.lib" )
	// #pragma comment( lib, "libtiffd.lib" )
	// #pragma comment( lib, "zlibd.lib" )
#pragma comment( lib, "opencv_core2410d.lib" )
//#pragma comment( lib, "opencv_gpu2410d.lib" )
#pragma comment( lib, "opencv_highgui2410d.lib" )
#pragma comment( lib, "opencv_imgproc2410d.lib" )
#pragma comment( lib, "opencv_objdetect2410d.lib" )
#pragma comment( lib, "opencv_ocl2410d.lib" )
#pragma comment( lib, "opencv_features2d2410d.lib" )
#pragma comment( lib, "opencv_stitching2410d.lib" )
#else
	//#pragma comment( lib, "IlmImf.lib" )
	// #pragma comment( lib, "libjasper.lib" )
	// #pragma comment( lib, "libjpeg.lib" )
	// #pragma comment( lib, "libpng.lib" )
	// #pragma comment( lib, "libtiff.lib" )
	// #pragma comment( lib, "zlib.lib" )
#pragma comment( lib, "opencv_core2410.lib" )
//#pragma comment( lib, "opencv_gpu2410.lib" )
#pragma comment( lib, "opencv_highgui2410.lib" )
#pragma comment( lib, "opencv_imgproc2410.lib" )
#pragma comment( lib, "opencv_objdetect2410.lib" )
#pragma comment( lib, "opencv_ocl2410.lib" )
#pragma comment( lib, "opencv_features2d2410.lib" )
#pragma comment( lib, "opencv_stitching2410.lib" )
#endif
#else

#ifdef _DEBUG
#pragma comment( lib, "opencv_core440d.lib" )
#pragma comment( lib, "opencv_highgui440d.lib" )
#pragma comment( lib, "opencv_imgproc440d.lib" )
#pragma comment( lib, "opencv_imgcodecs440d.lib" )
#pragma comment( lib, "opencv_stitching440d.lib" )
#pragma comment( lib, "opencv_features2d440d.lib" )
#pragma comment( lib, "opencv_objdetect440d.lib" )
#else
#pragma comment( lib, "opencv_core440.lib" )
#pragma comment( lib, "opencv_highgui440.lib" )
#pragma comment( lib, "opencv_imgproc440.lib" )
#pragma comment( lib, "opencv_imgcodecs440.lib" )
#pragma comment( lib, "opencv_stitching440.lib" )
#pragma comment( lib, "opencv_features2d440.lib" )
#pragma comment( lib, "opencv_objdetect440.lib" )
#endif

#endif

// SHKang 2017/04/04
#pragma comment( lib, "ippi.lib" )
#pragma comment( lib, "ipps.lib" )
#pragma comment( lib, "ippcore.lib" )
//#pragma comment( lib, "ippch.lib" )
//#pragma comment( lib, "ippcc.lib" )
//
#include "QTimer.h"

#define _AlgoTool  1; 
#define MultiProcessFunc 1;
#pragma comment(lib,"../jpeg_Dll/JPGIPPLibrary.lib")
#ifdef _DEBUG
#pragma comment(lib, "CSML_x64d.lib")
#else
#pragma comment(lib, "CSML_x64.lib")
#endif // _DEBUG)
