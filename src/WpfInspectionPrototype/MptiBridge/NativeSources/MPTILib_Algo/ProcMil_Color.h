#pragma once


#include "ProcMil.h"


enum milproccolor_result
{
	eMCOL_SUCCESS = 0,
	eMCOL_FAIL
};


class CProcMil_Color : public CProcMil
{
public:
	CProcMil_Color(void);
	virtual ~CProcMil_Color(void);

private:
// 	Im::PIL_ID m_milBlobFeature;
// 	Im::PIL_ID m_milBlobResult;

	//mil init , free
public:
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int FreeMil();


public:

 	void GetPtrClipBuf(void* milBuf, UCHAR* userDst, int cx, int cy, int width, int height);
	void CvtGrayToColor(UCHAR* userSrc, UCHAR* userDst, int width, int height, double color);
	void SumImageToColor(UCHAR* userSrc1, UCHAR* userSrc2, UCHAR* userDst, int width, int height, double color1 , double color2);

public:

	

	int CalcBlob(Im::PIL_ID milSrc, Im::PIL_ID milDst, int minArea);
	CRect GetBlobBox(Im::PIL_ID milSrc, int minArea);
	int GetBlobIndex_MaxArea(int cnts);
	void MakeBlobImg(UCHAR* src, UCHAR* dst, int width, int height, int iteration);
	void GetPolygonCoordi(UCHAR* src, int width, int height, int margin, POINT* retPt);
	void GetPolygonCoordi_Pt7(UCHAR* src, int width, int height, int margin, POINT* retPt);
	void GetPolygonCoordi_CIE(UCHAR* src, int width, int height, int offsetX, int offsetY, POINT* retPt);

	void ImageClip_CIE(UCHAR* src, int orgSizeX, int orgSizeY, int band, UCHAR* dst, int cx, int cy, int dstSizeX, int dstSizeY);
};

