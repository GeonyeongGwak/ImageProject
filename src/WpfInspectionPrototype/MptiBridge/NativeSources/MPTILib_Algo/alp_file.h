//////////////////////////////////////////////////////////////////////////
//
//	alp_file.h/cpp
//	- create by ATW
//
//	< history >
//	01. ATW 2013/09/23 create below
//		typedef struct headerBuk;
//		typedef struct headerPhi;
//		typedef struct headerNphi;
//		typedef struct headerZmapCW;
//		typedef struct headerZMapPtt;
//		int alpf_get_buk_info(CString filePath, LP_HEADER_BUK pHeader);
//		int alpf_load_buk(CString filePath, UCHAR *pDest);
//		int alpf_save_buk(CString filePath, LP_HEADER_BUK pHeader, UCHAR *pSrc);
//		int alpf_get_nphi_info(CString filePath, LP_HEADER_NPHI pHeader);
//		int alpf_load_nphi(CString filePath, float *pDest);
//		int alpf_save_nphi(CString filePath, LP_HEADER_NPHI pHeader, float *pSrc);
//		int alpf_get_phi_info(CString filePath, LP_HEADER_PHI pHeader);
//		int alpf_load_phi(CString filePath, float *pDest);
//		int alpf_convert_phi_to_nphi(CString filePath);		
//		int alpf_save_cw(CString filePath, LP_HEADER_CW pHeader, float *pZmap);
//		int alpf_save_cw(CString filePath, LP_HEADER_CW pHeader, RECT roi, int zmapWidth, float *pZmap);
//		int alpf_save_ptt(CString filePath, LP_HEADER_PTT pHeader, float *pZmap, float *pImg32f);
//		int alpf_save_ptt(CString filePath, LP_HEADER_PTT pHeader, RECT roi, int zmapWidth, float *pZmap, float *pImg32f);
//		int alpf_make_img32f(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, float factorR, float factorG, float factorB, int dstWidthStep, float *pDest);
//
//	02. HUJ 2013/09/25 create below
//      int alpf_make_img24n32f(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, float factorR, float factorG, float factorB, int dstWidthStep, float *pDest32, UCHAR* pDest24)
//////////////////////////////////////////////////////////////////////////

#pragma once


typedef struct headerBuk
{
	float startX;
	float startY;
	float sizeX;
	float sizeY;
	float camFovX;
	float camFovY;
	float sizeBit;

	headerBuk() 
	{
		startX = 0.f;
		startY = 0.f;
		sizeX = 0.f;
		sizeY = 0.f;
		camFovX = 0.f;
		camFovY = 0.f;
		sizeBit = 8.f;
	}
}HEADER_BUK, *LP_HEADER_BUK;

typedef struct headerPhi
{
	int left;
	int top;
	int width;
	int height;
	int res0;		// unknown
	int res1;		// unknown

	headerPhi() 
	{
		ZeroMemory(this, sizeof(*this)); 
	}
}HEADER_PHI, *LP_HEADER_PHI;

typedef struct headerNphi
{
	int width;
	int height;

	headerNphi() 
	{
		ZeroMemory(this, sizeof(*this)); 
	}
}HEADER_NPHI, *LP_HEADER_NPHI;

typedef struct headerZmapCW
{
	int width;
	int height;

	headerZmapCW() 
	{
		ZeroMemory(this, sizeof(*this)); 
	}
}HEADER_CW, *LP_HEADER_CW;

typedef struct headerZMapPtt
{
	unsigned int uiNumRow;
	unsigned int uiNumCol;
	float zResolX;
	float zResolY;
	unsigned int sizeBit;
	unsigned int gabX;
	unsigned int gabY;
	unsigned int sizeUnit;

	float fNumRow;
	float fNumCol;
	float targetStartX;
	float targetStartY;
	float targetShiftX;
	float targetShiftY;
	float targetWidth;
	float targetHeight;
	float pixelResX;
	float pixelResY;
	float depthBit;

	headerZMapPtt() 
	{
		this->uiNumRow = 0;
		this->uiNumCol = 0;
		this->zResolX = 1.f;
		this->zResolY = 1.f;
		this->sizeBit = 16;	//32
		this->gabX = 1;
		this->gabY = 1;
		this->sizeUnit = 1;
		this->fNumRow = 0;
		this->fNumCol = 0;
		this->targetStartX = 0;
		this->targetStartY = 0;
		this->targetShiftX = 0;
		this->targetShiftY = 0;
		this->targetWidth = 0;
		this->targetHeight = 0;
		this->pixelResX = 1;
		this->pixelResY = 1;
		this->depthBit = 32;
	}

}HEADER_PTT, *LP_HEADER_PTT;

typedef struct headerZMapPtr
{
	unsigned int uiNumRow;
	unsigned int uiNumCol;
	float zResolX;
	float zResolY;
	unsigned int sizeBit;
	unsigned int gabX;
	unsigned int gabY;
	unsigned int sizeUnit;

	headerZMapPtr()
	{
		this->uiNumRow = 0;
		this->uiNumCol = 0;
		this->zResolX = 1.f;
		this->zResolY = 1.f;
		this->sizeBit = 16;	//32
		this->gabX = 1;
		this->gabY = 1;
		this->sizeUnit = 1;
	}

}HEADER_PTR, *LP_HEADER_PTR;

typedef struct headerPot
{
	float fNumCol;
	float fNumRow;
	float fPixelResX;
	float fPixelResY;
	float fDepthBit;

	headerPot() 
	{
		this->fNumCol = 0;
		this->fNumRow = 0;
		this->fPixelResX = 0.015f;
		this->fPixelResY = 0.015f;
		this->fDepthBit = 8;
	}

}HEADER_POT, *LP_HEADER_POT;

int alpf_save_fbuk(CString filePath, LP_HEADER_BUK pHeader, int nImgCnt, PUINT8 *pArrSrc);
int alpf_load_fbuk(CString sFilePath, int nFovX, int nFovY, int nImgCnt, PUINT8 *pArrDest);

int alpf_get_buk_info(CString filePath, LP_HEADER_BUK pHeader);
int alpf_load_buk(CString filePath, UCHAR *pDest, int nDstW, int nDstH, int &nSizex, int &nSizey);

int alpf_save_buk(CString filePath, LP_HEADER_BUK pHeader, UCHAR *pSrc);

int alpf_get_nphi_info(CString filePath, LP_HEADER_NPHI pHeader);
int alpf_load_nphi(CString filePath, float *pDest);
int alpf_save_nphi(CString filePath, LP_HEADER_NPHI pHeader, float *pSrc);
int alpf_get_phi_info(CString filePath, LP_HEADER_PHI pHeader);
int alpf_load_phi(CString filePath, float *pDest);
int alpf_convert_phi_to_nphi(CString filePath);

int alpf_save_cw(CString filePath, LP_HEADER_CW pHeader, float *pZmap);
int alpf_save_cw(CString filePath, LP_HEADER_CW pHeader, RECT roi, int zmapWidth, float *pZmap);
int alpf_save_ptr(CString filePath, LP_HEADER_PTR pHeader, float *pZmap);
int alpf_save_ptt(CString filePath, LP_HEADER_PTT pHeader, float *pZmap, float *pImg32f);
int alpf_save_ptt(CString filePath, LP_HEADER_PTT pHeader, RECT roi, int zmapWidth, float *pZmap, float *pImg32f);
int alpf_make_img32f(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, float factorR, float factorG, float factorB, int dstWidthStep, float *pDest);
int alpf_make_img32f_CompositeBtm(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, UCHAR *pImgBR, UCHAR *pImgBB, float factorR, float factorG, float factorB, float factorBR, float factorBB, float compoBtmR, float compoBtmG, float compoBtmB, int dstWidthStep, float *pDest);
int alpf_make_img24n32f(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, float factorR, float factorG, float factorB, int dstWidthStep, float *pDest32, UCHAR* pDest24, int dstDst24WidthStep=0);	// SHKang 2017/07/20
int alpf_make_img24n32f_CompositeBtm(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, UCHAR *pImgBR, UCHAR *pImgBB, float factorR, float factorG, float factorB, float factorBR, float factorBB, float compoBtmR, float compoBtmG, float compoBtmB, int dstWidthStep, float *pDest32, UCHAR* pDest24, int dstDst24WidthStep=0);	// SHKang 2017/07/20
int alpf_make_img24(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, float factorR, float factorG, float factorB, int dstWidthStep, UCHAR* pDest24, int dstDst24WidthStep=0);	// LCJ 2017/08/28
int alpf_make_Miximg24_S4(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, UCHAR *pImgUV, float factorR, float factorG, float factorB, int dstWidthStep, UCHAR* pDest24, int dstDst24WidthStep=0);	// LCJ 2017/08/28

int alpf_save_pot(CString filePath, LP_HEADER_POT pHeader, UCHAR *pImgBB, UCHAR *pImgBR, UCHAR *pImgMB, UCHAR *pImgMR, UCHAR *pImgTW);
int alpf_save_pst(CString filePath, LP_HEADER_POT pHeader, UCHAR *pImgSR, UCHAR *pImgSG, UCHAR *pImgSB);

//#####################################################################################################################################
// SHKang 2017/03/30
int alpf_save_ptt(CString filePath, LP_HEADER_PTT pHeader, float *pZmap, BYTE *pImgR, BYTE *pImgG, BYTE *pImgB, float fFactorR, float fFactorG, float fFactorB, UCHAR* pDest24);

//#####################################################################################################################################=======
int alpf_save_put(CString filePath, LP_HEADER_POT pHeader, UCHAR *pImgTR, UCHAR *pImgTG, UCHAR *pImgTB, UCHAR *pImgBB, UCHAR *pImgBR = NULL);


int alpf_Write_Header(CArchive ar, LP_HEADER_BUK pHeader);
int alpf_Write_Body(CArchive ar, UCHAR *pSrc, int nSize);

int alpf_load_buk_to_savebmp(CString filePath, UCHAR **pDest,int* sizeX,int* sizeY);
int alpf_save_p3d(CString filePath, size_t szSaveBuf, BYTE *pSrc);