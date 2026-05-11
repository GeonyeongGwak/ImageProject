#pragma once


#ifndef __MPTI_DEF_H__
#define __MPTI_DEF_H__

#include "MPTIDef.h"

#endif

//#ifdef _MPTI_EXPORT
//#include "CItoP_Def.h"

#include "manager.h"
#include "ProcMil.h"

//SHK 2013/11/14 MIL barcode 사용 안함
#define  _MIL_BARCODE 0 // 첫번째꺼
#define  _DTKDLL_BARCODE 0 // 두번째꺼
#define  _MILDLL_BARCODE 0 // 세번째꺼
#define  _NEW_BARCODE 1 // 네번째꺼

#if _MIL_BARCODE
#ifdef _DEBUG
#pragma comment(lib, "CItoP_x64d.lib") 
#else
#pragma comment(lib, "CItoP_x64.lib")
#endif
extern "C" __declspec(dllimport) BOOL CItoP_Insp_Barcode(UCHAR * src, int imgSizeX, int imgSizeY, POINT st, POINT ed, int type, int color, CString& codeString);
#else

#if _DTKDLL_BARCODE
#ifdef _DEBUG
#pragma comment(lib, "DTK_Barcode_x64d.lib") 
#else
#pragma comment(lib, "DTK_Barcode_x64.lib")
extern "C" __declspec(dllimport) BOOL CItoP_Insp_Barcode(UCHAR * src, int imgSizeX, int imgSizeY, POINT st, POINT ed, int type, int color, int XFlip,int YFlip,CString& codeString, int checkSum, int nCount, int nThresholdType);
#endif
#endif

#if _MILDLL_BARCODE
#ifdef _DEBUG
#pragma comment(lib, "MIL_Barcode_x64d.lib") 
#else
#pragma comment(lib, "MIL_Barcode_x64.lib")
extern "C" __declspec(dllimport) int ExInsp_Barcode(UCHAR * src, int imgSizeX, int imgSizeY, POINT st, POINT ed, int type, int color, int nCount, CString *codeString);
extern "C" __declspec(dllimport) bool MILBarcodeLicense();
#endif
#endif

#if _NEW_BARCODE
#ifdef _DEBUG
#pragma comment(lib, "Barcode_x64d.lib") 
#else
#pragma comment(lib, "Barcode_x64.lib")
#endif
//#include "common_dll/include/Barcode_Def.h"
//#include "New_Barcode/Barcode_Def.h"
#endif

#endif

//LWW 2018/02/13 이전 DLL 호출X
// #ifdef _DEBUG
// #pragma comment(lib, "DTK_Barcode_x64d.lib") 
// #else
// #pragma comment(lib, "DTK_Barcode_x64.lib")
// #endif
// extern "C" __declspec(dllimport) BOOL CItoP_Insp_Barcode(UCHAR * src, int imgSizeX, int imgSizeY, POINT st, POINT ed, int type, int color, int XFlip,int YFlip,CString& codeString, int checkSum, int nCount, int nThresholdType);
// #endif
// 
// #ifdef _DEBUG
// #pragma comment(lib, "MIL_Barcode_x64d.lib") 
// #else
// #pragma comment(lib, "MIL_Barcode_x64.lib")
// #endif
// extern "C" __declspec(dllimport) int ExInsp_Barcode(UCHAR * src, int imgSizeX, int imgSizeY, POINT st, POINT ed, int type, int color, int nCount, CString *codeString);
// extern "C" __declspec(dllimport) bool MILBarcodeLicense();



enum barcode_rst
{
	eBCD_SUCCESS = 0,
	eBCD_FAIL,
};

enum barcode_device_type
{
	eBDEV_CAM = 0,
	eBDEV_BARCODE_READER,
};

class CMBarcodeManager : public CManager
{
public:
	CMBarcodeManager(void);
	virtual ~CMBarcodeManager(void);


private:
	InspBarcodeParam* m_inspParam;
	Coordinate m_coordi;
	UCHAR * m_fovImg;

	CProcMil* m_milProc;

public:
	int Init(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int Exit();
	void SetResolution(int fovWidth, int fovLength, double resolX, double resolY);


	int SetBarcodeParam(InspBarcodeParam* param);
	int SetBarcodeParam_Img(UCHAR * fovImg);
	Coordinate SetCoordi(InspBarcodeParam* pam);


	int InspBarcode(wchar_t** rst, UCHAR* pImg, UCHAR* pucInspImg, int nInspImgW, int nInspImgH, wchar_t* sBarcodeImageSavePath = NULL, wchar_t* sBarcodeImageSavePath_NewSeq = NULL, int InspType=0);
	bool InspBarcodeOutput(wchar_t** rst, wchar_t* sModuleBarcodeID, int nDelaytime);
	bool GetInspImg(UCHAR* pImg, int nImgW, int nImgH, UCHAR* pBigImg);
};

//#endif //_MPTI_EXPORT



//extern "C" MPTIDLL int MPTI_InspBarcode(wchar_t* rst, UCHAR* pImg, UCHAR* pucInspImg, int nInspImgW, int nInspImgH, wchar_t* sBarcodeImageSavePath = NULL, wchar_t* sBarcodeImageSavePath_NewSeq = NULL, int InspType = 0);
//extern "C" MPTIDLL bool MPTI_MILBarcodeLicense();
//extern "C" MPTIDLL bool MPTI_MILBarcodeStop();
//extern "C" MPTIDLL bool MPTI_GetInspImg(UCHAR* pImg, int nImgW, int nImgH, UCHAR* pBigImg);
//extern "C" MPTIDLL bool MPTI_InspBarcodeOutput(wchar_t* rst, wchar_t* sModuleBarcodeID, int nDelaytime);