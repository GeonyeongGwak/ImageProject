#include "alp_file.h"

class CMGrab;
class CMInspManager;
class CImgLoadSave
{
public:
	CImgLoadSave();
	~CImgLoadSave();

private:
	CMGrab *m_pMGrab;

	CMInspManager *m_pInspMng;

public:
	BOOL Initial(CString sPathDcf, UINT nFovSizeX, UINT nFovSizeY);
	void Destroy();

	void* GetGrabSystem();
	void* GetGrabApplication();
};

struct CPri_Header
{
	float fNumColumn;
	float fNumRow;
	float fSt_Col;
	float fSt_Row;
	float fWidth;
	float fLength;
	float fShiftX;
	float fShiftY;
	float fResol_x;
	float fResol_y;	
	float fDEPTH;

	CPri_Header()
	{
		fNumColumn	= 0.0f;
		fNumRow		= 0.0f;
		fSt_Col		= 0.0f;
		fSt_Row		= 0.0f;
		fWidth		= 0.0f;
		fLength		= 0.0f;
		fShiftX		= 0.0f;
		fShiftY		= 0.0f;
		fResol_x	= 0.0f;
		fResol_y	= 0.0f;	
		fDEPTH		= 0.0f;
	}
};

struct CPtr_Header
{
	UINT nNumRow;
	UINT nNumColumn;
	float fResol_x; 
	float fResol_y;
	UINT nBit;
	UINT nGap_x;
	UINT nGap_y;
	UINT nUnit;

	CPtr_Header()
	{
		nNumRow		= 0;
		nNumColumn	= 0;
		fResol_x	= 0.0f;
		fResol_y	= 0.0f;
		nBit		= 0;
		nGap_x		= 0;
		nGap_y		= 0;
		nUnit		= 0;
	}
};

extern CImgLoadSave *g_pImageFileLoadSave;

extern "C" __declspec(dllexport) BOOL ImageInit(wchar_t* filePath, UINT nFovSizeX, UINT nFovSizeY);
extern "C" __declspec(dllexport) BOOL ImageDestroy();

extern "C" __declspec(dllexport) int ImgLoadBuk(UCHAR* &pDest, wchar_t* filePath, int &nWidth, int &nHeight);
extern "C" __declspec(dllexport) int ImgSaveBuk(UCHAR *pSrc, wchar_t* filePath, LP_HEADER_BUK pHeader);

extern "C" __declspec(dllexport) int ImgLoadPtt(float* &ptrfVertexColorRed, float* &ptrfVertexColorGreend, float* &ptrfVertexColorBlue,
										float* &ptrfvRawData, wchar_t* filePath, int &nWidth, int &nHeight );
extern "C" __declspec(dllexport) int ImgAllLoadPtt(float* &ptrfVertexColor, float* &ptrfvRawData, wchar_t* filePath, int &nWidth, int &nHeight, LP_HEADER_PTT pHeader );
// extern "C" __declspec(dllexport) int ImageFileLoad_ptt(UCHAR* &ptrfVertexColorRed, UCHAR* &ptrfVertexColorGreend, UCHAR* &ptrfVertexColorBlue,
// 										UCHAR* &ptrfvRawData, wchar_t* filePath, int &nWidth, int &nHeight );

extern "C" __declspec(dllexport) int ImgAllSavePtt(float *pZmap, float *pImg32f, wchar_t* filePath, LP_HEADER_PTT pHeader);
extern "C" __declspec(dllexport) int ImgSavePtt(float *pZmap, float *pImg32f, wchar_t* filePath, LP_HEADER_PTT pHeader, 
															RECT roi, int zmapWidth);

extern "C" __declspec(dllexport) int ImgLoadTif(UCHAR* &pDest, wchar_t* filePath, int &nWidth, int &nHeight);
extern "C" __declspec(dllexport) int ImgSaveTif(UCHAR *pDest, wchar_t* filePath, int nWidth, int nHeight);

extern "C" __declspec(dllexport) int ImgLoadBmp(UCHAR* &pDest, wchar_t* filePath, int &nWidth, int &nHeight);

