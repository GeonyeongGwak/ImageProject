#pragma once
#include "StdAfx.h"
#include "ProcMil.h"
#include "InspParamDef.h"
#include "Crypt.h"
#include "GeoMatch.h"
#include "PILibrary.h"
#include "MemoryManager.h"

//SHK 2013/11/08
//#include "ippm.h"
#include "ipps.h"
#include "ippi.h"

//#pragma comment(lib, "ippm.lib")
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")
//////////////////////////////////////////////////////////////////////////

//CMPTI *g_pMPTI = NULL;

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

#define SQR(x) ((x)*(x))
//#define    _INSP_IMG_SAVE

CProcMil::CProcMil(void)
{
	m_className = _T("CProcMil");

	// Allocate a Application.
	m_milApp = M_NULL;
	// Allocate a System.
	m_milSys = M_NULL;


	m_structElement_H = M_NULL;
	m_structElement_V = M_NULL;

	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;
}


CProcMil::~CProcMil(void)
{
	FreeMil();
	
	FreeMilAlgoBlob();
}

int CProcMil::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys,bool bUseImagePilLib)
{
	if(milApp == M_NULL || milSys == M_NULL)
	{
		return 1;	
	}

	// Allocate a Application.
	m_milApp = milApp;
	// Allocate a System.
	m_milSys = milSys;

 	if(bUseImagePilLib == true)
 		Im::Blob::Alloc(Im::eType::PIL);
 	else
		Im::Blob::Alloc(Im::eType::MIL);

	long  HStructArray[STRUCT_ELEM_WIDTH][STRUCT_ELEM_HEIGHT] = {
		{M_DONT_CARE, M_DONT_CARE, M_DONT_CARE},
		{		    1,           1,          1},
		{M_DONT_CARE, M_DONT_CARE, M_DONT_CARE} };

	long  VStructArray[STRUCT_ELEM_WIDTH][STRUCT_ELEM_HEIGHT] = {
		{M_DONT_CARE,			 1, M_DONT_CARE},
		{M_DONT_CARE,			 1, M_DONT_CARE},
		{M_DONT_CARE,			 1, M_DONT_CARE} };

		FreeMil();
		Im::Buf::Alloc2d(*m_milSys, STRUCT_ELEM_WIDTH, STRUCT_ELEM_HEIGHT, 32 + M_UNSIGNED, M_STRUCT_ELEMENT, &m_structElement_H);
		Im::Buf::Put(m_structElement_H, HStructArray);

		Im::Buf::Alloc2d(*m_milSys, STRUCT_ELEM_WIDTH, STRUCT_ELEM_HEIGHT, 32 + M_UNSIGNED, M_STRUCT_ELEMENT, &m_structElement_V);
		Im::Buf::Put(m_structElement_V, VStructArray);

	return 0;
}

int CProcMil::FreeMil()
{
// 	m_milSys = M_NULL;
// 	m_milApp = M_NULL;

#ifndef _DISABLE_MIL
	if(m_structElement_H > M_NULL)
	{
		Im::Buf::Free(m_structElement_H);
		m_structElement_H = M_NULL;
	}

	if(m_structElement_V > M_NULL)
	{
		Im::Buf::Free(m_structElement_V);
		m_structElement_V = M_NULL;
	}
#endif

	return 0;
}

void CProcMil::FreeMilAlgoBlob()
{
	if(m_milBlobFeature != M_NULL)
	{
		Im::Blob::blobFree(m_milBlobFeature);
		m_milBlobFeature = M_NULL;
	}

	if(m_milBlobResult != M_NULL)
	{
		Im::Blob::blobFree(m_milBlobResult);
		m_milBlobResult = M_NULL;
	}
}

Im::PIL_ID CProcMil::AllocBuff(int width, int height, int initVal)
{
	Im::PIL_ID milBuff = M_NULL;
	Im::Buf::Alloc2d(*m_milSys, (MIL_INT)width, (MIL_INT)height, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);

	if(initVal >= 0 && initVal <= 255)
		Im::Buf::Clear(milBuff, initVal);

	return milBuff;
}



Im::PIL_ID CProcMil::AllocBuffColor(int width, int height, double initVal)
{
	Im::PIL_ID milBuff = M_NULL;
	Im::Buf::AllocColor(*m_milSys ,3 ,(MIL_INT)width ,(MIL_INT)height ,8 + M_UNSIGNED, M_IMAGE + M_PROC  , &milBuff);

	if(initVal >= 0)
		Im::Buf::Clear(milBuff, initVal);

	return milBuff;
}

Im::PIL_ID CProcMil::AllocClipBuff(Im::PIL_ID milSrc, int cX, int cY, int width, int height)
{
	Im::PIL_ID milBuff = M_NULL;
	int startX = 0;
	int startY = 0;

	Im::Buf::Alloc2d(*m_milSys, width, height, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);
	Im::Buf::Clear(milBuff, 0);

	startX = cX - (width/2);
	startY = cY - (height/2);

	Im::Buf::CopyClip(milSrc, milBuff, (MIL_INT)-startX, (MIL_INT)-startY);

	return milBuff;
}

//SHK 2013/11/08

void CProcMil::GetClipBuff(void* milSrc, UCHAR* userDst, int cx, int cy, int roiSizeX, int roiSizeY)
{
	CSize imgSize = GetImageSize((Im::PIL_ID)milSrc);

	if(roiSizeX == imgSize.cx && roiSizeY == imgSize.cy || imgSize.cx == 0 && imgSize.cy == 0)
	{
		Im::Buf::Get2d((Im::PIL_ID)milSrc, 0, 0, roiSizeX, roiSizeY, userDst);
		return;
	}

	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int orgSizeX = Im::Buf::Inquire((Im::PIL_ID)milSrc, M_PITCH, M_NULL);//imgSize.cx;		//Width¿Í Pitch°ªÀÌ ´Ù¸¥°æ¿ì Width°ªÀ» ¾²¸é ¿À·ù¹ß»ý
#if _OFFLINE
	if(m_fovWidth == imgSize.cx)
		orgSizeX = imgSize.cx;	// offline¿¡¼­ Euresys Àåºñ JobÀÎ °æ¿ì pitch¸¦ »ç¿ëÇÏ¸é ÀÌ»óÇÏ´Ù... ÀÏ´Ü matrox ´Â width¿Í pitch °¡ µ¿ÀÏÇÔ.
#endif
	int orgSizeY = imgSize.cy;
	int stX = 0;
	int stY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	if(stX < 0)
		stX = 0;

	if(stY < 0)
		stY = 0;

	if(stX + width >= imgSize.cx)
		stX = imgSize.cx - width;

	if(stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	//UCHAR* userSrc = NULL;//new UCHAR[imgSize.cx*imgSize.cy];
	//Im::Buf::Inquire((Im::PIL_ID)milSrc, M_HOST_ADDRESS,&userSrc);
	UCHAR* userSrc = (UCHAR*)Im::Buf::Inquire((Im::PIL_ID)milSrc, M_HOST_ADDRESS, NULL);
	sts = ippiCopy_8u_C1R(userSrc + offX + offY * orgSizeX, orgSizeX * 1, userDst, width * 1, iSize);
	//delete userSrc;
}

void CProcMil::GetClipBuff_LT_LoadRawData(void* milSrc, UCHAR* userDst, int nSX, int nSY, int roiSizeX, int roiSizeY)
{
	CSize imgSize = GetImageSize((Im::PIL_ID)milSrc);

	if(roiSizeX == imgSize.cx && roiSizeY == imgSize.cy)
	{
		Im::Buf::Get2d((Im::PIL_ID)milSrc, 0, 0, roiSizeX, roiSizeY, userDst);
		return;
	}

	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int orgSizeX = Im::Buf::Inquire((Im::PIL_ID)milSrc, M_PITCH, M_NULL);//imgSize.cx;		//Width¿Í Pitch°ªÀÌ ´Ù¸¥°æ¿ì Width°ªÀ» ¾²¸é ¿À·ù¹ß»ý
	int orgSizeY = imgSize.cy;
	int stX = nSX;  // start point x
	int stY = nSY;  // start point Y
	int width = roiSizeX;
	int height = roiSizeY;

	if(stX < 0)
		stX = 0;

	if(stY < 0)
		stY = 0;

	if(stX + width >= imgSize.cx)
		stX = imgSize.cx - width;

	if(stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	UCHAR* userSrc = (UCHAR*)Im::Buf::Inquire((Im::PIL_ID)milSrc, M_HOST_ADDRESS, NULL);

	sts = ippiCopy_8u_C1R(userSrc + offX + offY * orgSizeX, orgSizeX * 1, userDst, width * 1, iSize);

	// 	SaveWorkImg((Im::PIL_ID)milSrc, _T("SourceImage.bmp"));
	// 	SaveWorkImg(userDst, width, height, _T("ClipImage.bmp"));
	//delete userSrc;
}

// SHW 2014/11/04
void CProcMil::GetClipBuff_LT(void* milSrc, UCHAR* userDst, int nSX, int nSY, int roiSizeX, int roiSizeY)
{
	CSize imgSize = GetImageSize((Im::PIL_ID)milSrc);

	if(roiSizeX == imgSize.cx && roiSizeY == imgSize.cy || imgSize.cx == 0 && imgSize.cy == 0)
	{
		Im::Buf::Get2d((Im::PIL_ID)milSrc, 0, 0, roiSizeX, roiSizeY, userDst);
		return;
	}
	
	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int orgSizeX = Im::Buf::Inquire((Im::PIL_ID)milSrc, M_PITCH, M_NULL);//imgSize.cx;		//Width¿Í Pitch°ªÀÌ ´Ù¸¥°æ¿ì Width°ªÀ» ¾²¸é ¿À·ù¹ß»ý
#if _OFFLINE
	if(m_fovWidth == imgSize.cx)
		orgSizeX = imgSize.cx;	// offline¿¡¼­ Euresys Àåºñ JobÀÎ °æ¿ì pitch¸¦ »ç¿ëÇÏ¸é ÀÌ»óÇÏ´Ù... ÀÏ´Ü matrox ´Â width¿Í pitch °¡ µ¿ÀÏÇÔ.
#endif
	int orgSizeY = imgSize.cy;
	int stX = nSX;  // start point x
	int stY = nSY;  // start point Y
	int width = roiSizeX;
	int height = roiSizeY;

	if(stX < 0)
		stX = 0;

	if(stY < 0)
		stY = 0;

	if(stX + width >= imgSize.cx)
		stX = imgSize.cx - width;

	if(stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	UCHAR* userSrc = (UCHAR*)Im::Buf::Inquire((Im::PIL_ID)milSrc, M_HOST_ADDRESS, NULL);

	sts = ippiCopy_8u_C1R(userSrc + offX + offY * orgSizeX, orgSizeX * 1, userDst, width * 1, iSize);

// 	SaveWorkImg((Im::PIL_ID)milSrc, _T("SourceImage.bmp"));
// 	SaveWorkImg(userDst, width, height, _T("ClipImage.bmp"));
	//delete userSrc;
}

// SHW 2014/11/03
void CProcMil::GetClipImage_LT(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int Sx, int Sy, int roiSizeX, int roiSizeY, int band, double dAngle)
{
	if (userSrc == NULL || userDst == NULL)
		return;
	if (srcSizeX <= 0 || srcSizeY <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return;
	int stX = Sx;  // start point x
	int stY = Sy;  // start point Y
	if (stX < 0) stX = 0;
	if (stY < 0) stY = 0;
	if (roiSizeX > srcSizeX || stX + roiSizeX > srcSizeX ||
		roiSizeY > srcSizeY || stY + roiSizeY > srcSizeY)
		return;

	IppStatus sts;
	IppiSize iSize = { roiSizeX, roiSizeY };
	if (srcSizeX == roiSizeX && srcSizeY == roiSizeY)
	{
		if (band == 1)
			sts = ippiCopy_8u_C1R(userSrc, srcSizeX * band, userDst, roiSizeX * band, iSize);
		else if (band == 3)
			sts = ippiCopy_8u_C3R(userSrc, srcSizeX * band, userDst, roiSizeX * band, iSize);
	}
	else if (dAngle != 0 && band == 1)
	{
		UCHAR* ptrSrc = userSrc;
		UCHAR* ptrbyBuffer_ro = NULL;
		int retDstSizeX(0), retDstSizeY(0);
		
		for (int r = 0; r < srcSizeY; r++)
		{
			UCHAR* srcPtr = &userSrc[r*srcSizeX];
			UCHAR* dstPtr = &ptrSrc[r*srcSizeX];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*srcSizeX);
		}

		RotateImg_ipp(ptrSrc, srcSizeX, srcSizeY, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);

		int GapX = RounD((retDstSizeX - srcSizeX) / 2);
		int GapY = RounD((retDstSizeY - srcSizeY) / 2);
		if ((GapX + srcSizeX) > retDstSizeX)
			GapX = 0;
		if ((GapY + srcSizeY) > retDstSizeY)
			GapY = 0;

		for (int r = 0; r < srcSizeY; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];

			UCHAR* dstPtr = &userSrc[r*srcSizeX];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*srcSizeX);

		}
		SaveWorkImg(ptrbyBuffer_ro, retDstSizeX, retDstSizeY, _T("RotateTestbuffer.bmp"));
		SaveWorkImg(userSrc, srcSizeX, srcSizeY, _T("RotateTestPtr.bmp"));

		if (ptrbyBuffer_ro != NULL)
			g_pMManager->pem_delete(ptrbyBuffer_ro, true);
		sts = ippiCopy_8u_C1R(userSrc + stX + stY * srcSizeX, srcSizeX * band, userDst, roiSizeX * band, iSize);
	}
	else
	{
		if (band == 1)
			sts = ippiCopy_8u_C1R(userSrc + stX + stY * srcSizeX, srcSizeX * band, userDst, roiSizeX * band, iSize);
		else if (band == 3)
			sts = ippiCopy_8u_C3R(userSrc + ((stX* band) + (stY * (srcSizeX * band))), srcSizeX * band, userDst, roiSizeX * band, iSize);
	}
}

// YJS 2016/11/04
void CProcMil::SetBackClipImage_LT(UCHAR* userSrc, int srcSizeX, int srcSizeY, int srcPitch, UCHAR* userDst, int Sx, int Sy, int roiSizeX, int roiSizeY, int band)
{
	if((userSrc == NULL))
		return;

	if(srcSizeX == roiSizeX && srcSizeY == roiSizeY)
	{
		IppStatus sts;
		IppiSize iSize = {roiSizeX, roiSizeY};

		if(band == 1)
			sts = ippiCopy_8u_C1R(userDst, roiSizeX * band, userSrc, srcPitch * band, iSize);
		else if(band == 3)
			sts = ippiCopy_8u_C3R(userDst, roiSizeX * band, userSrc, srcPitch * band, iSize);

		return;
	}

	if(roiSizeX > srcSizeX)
		roiSizeX = srcSizeX;
	if(roiSizeY > srcSizeY)
		roiSizeY = srcSizeY;

	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int stX = 0;
	int stY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = Sx;  // start point x
	stY = Sy;  // start point Y

	if(stX < 0)
		stX = 0;

	if(stY < 0)
		stY = 0;

	if(stX + width >= srcSizeX)
		stX = srcSizeX - width;

	if(stY + height > srcSizeY)
		stY = srcSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	if(band == 1)
		sts = ippiCopy_8u_C1R(userDst, width, userSrc + offX + offY * srcPitch, srcPitch, iSize);
	else if(band == 3)
		sts = ippiCopy_8u_C3R(userDst, width * band, userSrc + ((offX* band) + (offY * (srcPitch * band))), srcPitch * band, iSize);
}

void CProcMil::GetClipImage(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int cx, int cy, int roiSizeX, int roiSizeY, int band)
{
	if(srcSizeX == roiSizeX && srcSizeY == roiSizeY)
	{
		IppStatus sts;
		IppiSize iSize = {roiSizeX, roiSizeY};

		if(band == 1)
			sts = ippiCopy_8u_C1R(userSrc, srcSizeX * band, userDst, roiSizeX * band, iSize);
		else if(band == 3)
			sts = ippiCopy_8u_C3R(userSrc, srcSizeX * band, userDst, roiSizeX * band, iSize);

		return;
	}

	if(roiSizeX > srcSizeX)
		roiSizeX = srcSizeX;
	if(roiSizeY > srcSizeY)
		roiSizeY = srcSizeY;

	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int orgSizeX = srcSizeX;
	int orgSizeY = srcSizeY;
	int stX = 0;
	int stY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	if(stX < 0)
		stX = 0;

	if(stY < 0)
		stY = 0;

	if(stX + width >= orgSizeX)
		stX = orgSizeX - width;

	if(stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	if(band == 1)
		sts = ippiCopy_8u_C1R(userSrc + offX + offY * srcSizeX, srcSizeX * band, userDst, width * band, iSize);
	else if(band == 3)
		sts = ippiCopy_8u_C3R(userSrc + ((offX* band) + (offY * (srcSizeX * band))), srcSizeX * band, userDst, width * band, iSize);
}

float CProcMil::GetCropZmap(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY)
{
	if(src == NULL || roiSizeX < 0 || roiSizeY < 0)
		return -1;

	if(roiSizeX > orgSizeX)
		roiSizeX = orgSizeX;
	if(roiSizeY > orgSizeY)
		roiSizeY = orgSizeY;

	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int stX = 0;
	int stY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	if(stX < 0)
		stX = 0;

	if(stY < 0)
		stY = 0;

	if(stX + width >= orgSizeX)
		stX = orgSizeX - width;

	if(stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	sts = ippiCopy_32f_C1R(src + offX + offY * orgSizeX, orgSizeX * 4, dst, width * 4, iSize);

	return (float)sts;
}


//////////////////////////////////////////////////////////////////////////
#ifdef false
//±âÁ¸ MIL
void CProcMil::GetClipBuff(void* milSrc, UCHAR* userDst, int cX, int cY, int width, int height)
{
	Im::PIL_ID milBuff = M_NULL;
	int startX = 0;
	int startY = 0;

	Im::Buf::Alloc2d(*m_milSys, width, height, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);

	//SaveWorkImg((Im::PIL_ID)milSrc, _T("grab.bmp"));

	startX = cX - (width/2);
	startY = cY - (height/2);

	Im::Buf::CopyClip((Im::PIL_ID)milSrc, milBuff, (MIL_INT)-startX, (MIL_INT)-startY);
	Im::Buf::Get(milBuff, userDst);

	Im::Buf::Free(milBuff);
	milBuff = M_NULL;
}

void CProcMil::GetClipImage(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int cX, int cY, int width, int height)
{
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;
	int startX = 0;
	int startY = 0;

	milSrc = AllocBuff(srcSizeX, srcSizeY);
	Im::Buf::Put(milSrc, userSrc);

	milDst = AllocClipBuff(milSrc, cX, cY, width, height);
	Im::Buf::Get(milDst, userDst);

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milDst);
}
#endif

Im::PIL_ID CProcMil::AllocClipBuffColor(Im::PIL_ID milSrc, int cX, int cY, int width, int height)
{
	Im::PIL_ID milBuff = M_NULL;
	int startX = 0;
	int startY = 0;

	Im::Buf::AllocColor(*m_milSys ,3 ,(MIL_INT)width ,(MIL_INT)height ,8+M_UNSIGNED, M_IMAGE + M_PROC  , &milBuff);

	startX = cX - (width/2);
	startY = cY - (height/2);

	Im::Buf::CopyClip(milSrc, milBuff, (MIL_INT)-startX, (MIL_INT)-startY);

	return milBuff;
}


Im::PIL_ID CProcMil::AllocChildImage(Im::PIL_ID milSrc, int roiX, int roiY, int roiWidth, int roiHeight)
{
	Im::PIL_ID milDest = M_NULL;
	Im::Buf::Child2d(milSrc, roiX, roiY, roiWidth, roiHeight, &milDest);
	return milDest;
}

Im::PIL_ID CProcMil::AllocRotateBuff(Im::PIL_ID milSrc, double angle)
{
	Im::PIL_ID milBuff = M_NULL;
	MIL_INT sizeX = Im::Buf::Inquire(milSrc, M_SIZE_X, M_NULL);
	MIL_INT sizeY = Im::Buf::Inquire(milSrc, M_SIZE_Y, M_NULL);

	if(angle == 90 || angle == 270)
	{
		Im::Buf::Alloc2d(*m_milSys, (MIL_INT)sizeY, (MIL_INT)sizeX, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);	
	}
	else
	{
		Im::Buf::Alloc2d(*m_milSys, (MIL_INT)sizeX, (MIL_INT)sizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);
	}

	GetRotateImg(milSrc, milBuff, angle);

	return milBuff;

}

Im::PIL_ID CProcMil::AllocRotateBuff_Color(Im::PIL_ID milSrc, double angle)
{
	Im::PIL_ID milBuff = M_NULL;
	MIL_INT sizeX = Im::Buf::Inquire(milSrc, M_SIZE_X, M_NULL);
	MIL_INT sizeY = Im::Buf::Inquire(milSrc, M_SIZE_Y, M_NULL);

	if(angle == 90 || angle == 270)
	{
		Im::Buf::AllocColor(*m_milSys, 3, (MIL_INT)sizeY, (MIL_INT)sizeX, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);	
	}
	else
	{
		Im::Buf::AllocColor(*m_milSys, 3, (MIL_INT)sizeX, (MIL_INT)sizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);
	}

	GetRotateImg(milSrc, milBuff, angle);

	return milBuff;

}



UCHAR* CProcMil::GetPtrGrabBuf(void* milBuf)
{
	UCHAR *pBuf = (UCHAR*)Im::Buf::Inquire((Im::PIL_ID)milBuf, M_HOST_ADDRESS, NULL);
	return pBuf;
}

void CProcMil::GetPtrGrabBuf(void* milBuf, UCHAR* userDst)
{
	MIL_INT sizeX = Im::Buf::Inquire((Im::PIL_ID)milBuf, M_SIZE_X, M_NULL);
	MIL_INT sizeY = Im::Buf::Inquire((Im::PIL_ID)milBuf, M_SIZE_Y, M_NULL);

	if(sizeX < 0 || sizeY < 0)
		return;

	//UCHAR* temp = new UCHAR[sizeX * sizeY];
	UCHAR* temp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);

	Im::Buf::Get((Im::PIL_ID)milBuf, temp);
	memcpy_s(userDst, (sizeof(UCHAR) * sizeX * sizeY), temp, (sizeof(UCHAR) * sizeX * sizeY));

	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

void CProcMil::FreeMilImageBuff(Im::PIL_ID* milImage)
{
	if(*milImage != M_NULL)
	{
		Im::Buf::Free(*milImage);
		*milImage = M_NULL;
	}
}

void CProcMil::GetRotateImg(Im::PIL_ID milSrc, Im::PIL_ID milDst, double angle)
{
	Im::im::Rotate(milSrc, milDst, (MIL_DOUBLE)angle, M_DEFAULT , M_DEFAULT , M_DEFAULT, M_DEFAULT, M_BILINEAR);
}

void CProcMil::RotateImg(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR* userDst, int* retDstSizeX, int* retDstSizeY)
{
	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(orgSizeX, orgSizeY);
	Im::Buf::Put(milSrc, userSrc);

	Im::PIL_ID milRotate = M_NULL;
	milRotate = AllocRotateBuff(milSrc, angle);
	Im::Buf::Get(milRotate, userDst);

	CSize sz = GetImageSize(milRotate);

	if(retDstSizeX != NULL)
		*retDstSizeX = (int)sz.cx;
	if(retDstSizeY != NULL)
		*retDstSizeY = (int)sz.cy;

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milRotate);
}


// LMJ 2013/11/25
void CProcMil::RotateImg_ipp(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR** userDst, int* retDstSizeX, int* retDstSizeY)
{

	if (*userDst != NULL)
	{
		delete *userDst;
		*userDst = NULL;
	}

	bool bLinear = true;
	bool bColor = false;

	RotateImg_ipp2020(userSrc, userDst, angle, orgSizeX, orgSizeY, retDstSizeX, retDstSizeY, bColor, bLinear, false);
}

void CProcMil::RotateImg_ipp_color(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR** userDst, int* retDstSizeX, int* retDstSizeY)
{
	
	bool bLinear = true;
	bool bColor = true;

	RotateImg_ipp2020(userSrc, userDst, angle, orgSizeX, orgSizeY, retDstSizeX, retDstSizeY, bColor, bLinear, false);
}


void CProcMil::RotateImg_ipp2(UCHAR* userSrc, UCHAR** userDst, int orgSizeX, int orgSizeY, double angle, int* retDstSizeX, int* retDstSizeY, int band, int nInterPolation)
{
	bool bLinear = nInterPolation == 0 ? true : false;	//0 : Linear, 1 : Nearest neigborhood
	bool bColor;

	if (band == 1)
		bColor = false;
	else
		bColor = true;

	RotateImg_ipp2020(userSrc, userDst, angle, orgSizeX, orgSizeY, retDstSizeX, retDstSizeY, bColor, bLinear, false);

	/*
	cv::Mat *src;

	if(band == 1)
		src = new cv::Mat(orgSizeY, orgSizeX, CV_8UC1, userSrc);
	else
		src = new cv::Mat(orgSizeY, orgSizeX, CV_8UC3, userSrc);

	cv::Mat srcImg;
	src->copyTo(srcImg);

	cv::Mat *dstImg;

	cv::Point CenterPo(orgSizeX / 2, orgSizeY / 2);

	cv::Point Margin = WarpAffine(srcImg, *dstImg, angle, srcImg.size(), CenterPo, false, false);

	*userDst = dstImg->data;

	if (retDstSizeX != NULL)
		*retDstSizeX = (*dstImg).cols;

	if (retDstSizeY != NULL)
		*retDstSizeY = (*dstImg).rows;

	if (*userDst == NULL)
		*userDst = new UCHAR[orgSizeX * orgSizeY];

	*/
}


// LMJ 2013/11/25
void CProcMil::RotateZmap_ipp(float* zmapData, float* dst, int orgSizeX, int orgSizeY, double angle)
{
	/*
	IppiSize srcSize = { orgSizeX, orgSizeY };

	if (angle == 0)
	{
		ippiCopy_32f_C1R(zmapData, orgSizeX * 4, dst, orgSizeX * 4, srcSize);
		return;
	}

	IppiSize dstSize = { 0,0 };
	IppiRect srcROI, dstROI;
	srcROI.x = srcROI.y = 0;
	srcROI.width = orgSizeX;
	srcROI.height = orgSizeY;

	double bound[2][2];
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateBound(srcROI, bound, angle, xShift, yShift);

	dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
	dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);

	//	*dst = new float [dstSize.width * dstSize.height];		// 임의 각도일 경우 사이즈가 달라질 수 있음

	dstROI.x = dstROI.y = 0;
	dstROI.width = dstSize.width;
	dstROI.height = dstSize.height;

	xShift -= bound[0][0];
	yShift -= bound[0][1];

	IppStatus st = ippiRotate_32f_C1R(zmapData, srcSize, srcSize.width * sizeof(Ipp32f), srcROI, dst, dstROI.width * sizeof(Ipp32f), dstROI, angle, xShift, yShift, IPPI_INTER_LINEAR);
	*/

	IppiSize srcSize = { orgSizeX,  orgSizeY };
	if (angle == 0)
	{
		ippiCopy_32f_C1R(zmapData, orgSizeX * 4, dst, orgSizeX * 4, srcSize);
		return;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp32f* pSrc = zmapData;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	
	dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
	dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);
	

	//Memory allocation for the intermediate images
	//*userDst = new float[dstSize.width * dstSize.height];
	Ipp32f* pDstRoi = dst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	
	//변화된 크기 만큼 Shift
	coeffs[0][2] -= bound[0][0];
	coeffs[1][2] -= bound[0][1];
	

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;

	
	// Spec and init buffer sizes
	status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippLinear, direction, borderType,
		&specSize, &initSize);

	pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

	//Affine transform data initialization
	if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

	//Get work buffer size
	if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

	//allocate memory for work buffer
	pBuffer = ippsMalloc_8u(bufSize);

	//Affine transform processing
	if (status >= ippStsNoErr) status = ippiWarpAffineLinear_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

}


bool CProcMil::RotateZmap_ipp2020(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear, bool bUseOrgSize)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (*userDst == NULL)
			*userDst = g_pMManager->pem_new<float>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
		ippiCopy_32f_C1R(userSrc, orgSizeX * 4, *userDst, orgSizeX * 4, srcSize);

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp32f* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	if (bUseOrgSize == false)
	{
		//dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
		//dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);
		dstSize.width = (int)(bound[1][0] - bound[0][0] + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}
	
	//Memory allocation for the intermediate images
	//*userDst = new float[dstSize.width * dstSize.height];
	if (*userDst == NULL)
	*userDst = g_pMManager->pem_new<float>(true, dstSize.width * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);
	Ipp32f* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;

	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (status >= ippStsNoErr) status = ippiWarpAffineLinear_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (status >= ippStsNoErr) status = ippiWarpAffineNearest_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}

bool CProcMil::RotateZmap_ipp2020_2(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear, bool bUseOrgSize, cv::Point cvPoCenter)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (*userDst == NULL)
			*userDst = g_pMManager->pem_new<float>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
		ippiCopy_32f_C1R(userSrc, orgSizeX * 4, *userDst, orgSizeX * 4, srcSize);

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp32f* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	//ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);
	ippiGetRotateShift(cvPoCenter.x, cvPoCenter.y, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	if (bUseOrgSize == false)
	{
		/*dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
			dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);*/
		dstSize.width = (int)(bound[1][0] - bound[0][0] + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}

	//Memory allocation for the intermediate images
	//*userDst = new float[dstSize.width * dstSize.height];
	if (*userDst == NULL)
	*userDst = g_pMManager->pem_new<float>(true, dstSize.width * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);

	Ipp32f* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;

	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (status >= ippStsNoErr) status = ippiWarpAffineLinear_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (status >= ippStsNoErr) status = ippiWarpAffineNearest_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}


void CProcMil::RotateZmap_ipp2(float* zmapData, float** dst, int orgSizeX, int orgSizeY, double angle, int* retDstSizeX, int* retDstSizeY)
{
	bool bLinear = true;
	bool bUseOrgSize = false;

	bool ret = RotateZmap_ipp2020(zmapData, dst, angle, orgSizeX, orgSizeY, retDstSizeX, retDstSizeY, bLinear, bUseOrgSize);
}

void CProcMil::RotateImg_Color(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR* userDst, int* retDstSizeX, int* retDstSizeY)
{
	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuffColor(orgSizeX, orgSizeY);
	Im::Buf::PutColor(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS, userSrc);

	Im::PIL_ID milRotate = M_NULL;
	milRotate = AllocRotateBuff_Color(milSrc, angle);
	Im::Buf::GetColor(milRotate, M_PACKED + M_BGR24, M_ALL_BANDS, userDst);

	CSize sz = GetImageSize(milRotate);

	if(retDstSizeX != NULL)
		*retDstSizeX = (int)sz.cx;
	if(retDstSizeY != NULL)
		*retDstSizeY = (int)sz.cy;

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milRotate);
}

CSize CProcMil::GetImageSize(Im::PIL_ID milImg)
{
	if(milImg <= 0)// milImg id가 쓰레기 값인 경우
		return CSize((int)0, (int)0);

	MIL_INT width = Im::Buf::Inquire(milImg, M_SIZE_X, M_NULL);
	MIL_INT height = Im::Buf::Inquire(milImg, M_SIZE_Y, M_NULL);

	return CSize((int)width, (int)height);
}



void CProcMil::SaveWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, int bandSize, bool bSaveSide, bool bSaveR)
{
	bool bReturn = false;
	if (bSaveR == true)
		bReturn = true;

#ifdef _DEBUG
#ifdef _INSP_IMG_SAVE
	bReturn = true;
#endif
#endif

	if (bReturn == false)
		return;

	CString fullPath = _T("");
	if(bSaveSide == TRUE)	fullPath.Format(_T("%s"), fileName);
	else					fullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, fileName);
	CreateDir(WORK_IMAGE_PATH);

	if((buff == NULL) || (sizeX <= 0) || (sizeY <= 0) || (bandSize <= 0))
		return;

	Im::PIL_ID temp = M_NULL;
	if(bandSize == 1)
	{
		temp = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(temp, buff);
	}
	else if(bandSize == 3)
	{
		temp = AllocBuffColor(sizeX, sizeY);
		MIL_INT width = Im::Buf::Inquire(temp, M_SIZE_X, M_NULL);
		MIL_INT height = Im::Buf::Inquire(temp, M_SIZE_Y, M_NULL);
		Im::Buf::PutColor2d(temp, M_PACKED + M_BGR24, M_ALL_BANDS,0,0, width, height, buff);
	}

	Im::Buf::Export(fullPath, M_BMP, temp);

	FreeMilImageBuff(&temp);
}

void CProcMil::SaveWorkImg2(UCHAR* buff, int sizeX, int sizeY, CString fileName, int bandSize, bool bSaveSide, bool bSaveR)
{
	bool bReturn = false;
	if (bSaveR == true)
		bReturn = true;

	bReturn = true;

	if (bReturn == false)
		return;

	CString fullPath = _T("");
	if (bSaveSide == TRUE)	fullPath.Format(_T("%s"), fileName);
	else					fullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH2, fileName);
	CreateDir(WORK_IMAGE_PATH2);

	if ((buff == NULL) || (sizeX <= 0) || (sizeY <= 0) || (bandSize <= 0))
		return;

	Im::PIL_ID temp = M_NULL;
	if (bandSize == 1)
	{
		temp = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(temp, buff);
	}
	else if (bandSize == 3)
	{
		temp = AllocBuffColor(sizeX, sizeY);
		MIL_INT width = Im::Buf::Inquire(temp, M_SIZE_X, M_NULL);
		MIL_INT height = Im::Buf::Inquire(temp, M_SIZE_Y, M_NULL);
		Im::Buf::PutColor2d(temp, M_PACKED + M_BGR24, M_ALL_BANDS, 0, 0, width, height, buff);
	}

	Im::Buf::Export(fullPath, M_BMP, temp);

	FreeMilImageBuff(&temp);
}

void CProcMil::SaveReleaseWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, CString path, int bandSize)
{
	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), path, fileName);
	CreateDir(path);

	if((buff == NULL) || (sizeX <= 0) || (sizeY <= 0) || (bandSize <= 0))
		return;

	Im::PIL_ID temp = M_NULL;
	if(bandSize == 1)
	{
		temp = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(temp, buff);
	}
	else if(bandSize == 3)
	{
		temp = AllocBuffColor(sizeX, sizeY);
		Im::Buf::PutColor(temp, M_PACKED + M_BGR24, M_ALL_BANDS, buff);
	}

	Im::Buf::Export(fullPath, M_BMP, temp);

	FreeMilImageBuff(&temp);
}
void CProcMil::SaveReleaseWorkImg(Im::PIL_ID milBuff, CString fileName, CString path)
{
	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), path, fileName);
	CreateDir(path);
	Im::Buf::Export(fullPath, M_BMP, milBuff);
}

void CProcMil::SaveWorkImg(Im::PIL_ID milBuff, CString fileName)
{
#ifdef _DEBUG
#ifdef _INSP_IMG_SAVE
//#ifdef _OFFLINE_INSP
	if(milBuff == M_NULL)
		return;

	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, fileName);
	CreateDir(WORK_IMAGE_PATH);
	Im::Buf::Export(fullPath, M_BMP, milBuff);
#endif
#endif
}

void CProcMil::SaveWorkImg(cv::Mat milBuff, CString fileName)
{
#ifdef _DEBUG
#ifdef _INSP_IMG_SAVE
	//#ifdef _OFFLINE_INSP
	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, fileName);
	CreateDir(WORK_IMAGE_PATH);
	cv::imwrite(std::string(CT2A(fullPath)), milBuff);
	//#endif
#endif
#endif
}

void CProcMil::SaveWorkImg_float(Im::PIL_ID milSrc, CString fileName)
{
#ifdef _DEBUG
#ifdef _INSP_IMG_SAVE
	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, fileName);
	CreateDir(WORK_IMAGE_PATH);
	Im::Buf::Export(fullPath, M_BMP, milSrc);
#endif
#endif
}

void CProcMil::SaveWorkImg_float(float* src, int sizeX, int sizeY, CString fileName, int band, bool bSaveR)
{
	bool bReturn = false;
	if (bSaveR == true)
		bReturn = true;
#ifdef _DEBUG
#ifdef _INSP_IMG_SAVE
	bReturn = true;
#endif
#endif
	if (bReturn == false)
		return;
	Im::PIL_ID milSrc = M_NULL;
	
	if(band == 1)
	{
		milSrc = AllocBuff_float(sizeX, sizeY);
		Im::Buf::Put(milSrc, src);
	}
	else if(band == 3)
	{
		milSrc = AllocRotateBuff_Color(sizeX, sizeY);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR32 , M_ALL_BANDS, src);
	}

	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, fileName);
	CreateDir(WORK_IMAGE_PATH);

	Im::Buf::Export(fullPath, M_BMP, milSrc);

	FreeMilImageBuff(&milSrc);	
}

void CProcMil::SaveWorkImg_dword(DWORD* src, int sizeX, int sizeY, CString fileName, int band)
{
#ifdef _DEBUG
#ifdef _INSP_IMG_SAVE
	Im::PIL_ID milSrc = M_NULL;

	if(band == 1)
	{
		milSrc = AllocBuff_float(sizeX, sizeY);
		Im::Buf::Put(milSrc, src);
	}
	else if(band == 3)
	{
		milSrc = AllocBuffColor(sizeX, sizeY);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR32 , M_ALL_BANDS, src);
	}

	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), WORK_IMAGE_PATH, fileName);
	CreateDir(WORK_IMAGE_PATH);

	Im::Buf::Export(fullPath, M_BMP, milSrc);

	FreeMilImageBuff(&milSrc);	
#endif
#endif
}

void CProcMil::SaveDebugImg(UCHAR* buff, int sizeX, int sizeY, CString FullPath, int bandSize, BOOL bEnCrypt)
{
	if(FullPath == "NONE")
		return;
	if((buff == NULL) || (sizeX <= 0) || (sizeY <= 0) || (bandSize <= 0))
		return;

	Im::PIL_ID temp = M_NULL;
	if(bandSize == 1)
	{
		temp = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(temp, buff);
	}
	else if(bandSize == 3)
	{
		temp = AllocBuffColor(sizeX, sizeY);
		Im::Buf::PutColor(temp, M_PACKED + M_BGR24, M_ALL_BANDS, buff);
	}

	Im::Buf::Export(FullPath, M_BMP, temp);

	FreeMilImageBuff(&temp);	

	if(bEnCrypt)
	{
		EnCryptBMP(FullPath);
	}
}

void CProcMil::EnCryptBMP(CString FullPath)
{
	int nsize;

	OnOpenDocument(FullPath);

	CCrypt::Encrypt(m_InImg, m_InImg, m_dwWidth, m_dwHeight);

	CFile hFile;

	LPCTSTR lpszA = (LPCTSTR)FullPath;

	if(!hFile.Open(lpszA,CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
		return;

	//Á¤º¸ÀúÀå
	hFile.Write(&dibHf,sizeof(BITMAPFILEHEADER));

	hFile.Write(&dibHi,sizeof(BITMAPINFOHEADER));

	if(dibHi.biBitCount==8)
		hFile.Write(palRGB,sizeof(RGBQUAD)*256);

	hFile.Write(m_InImg,dibHi.biSizeImage);

	hFile.Close();

	//delete m_InImg;

	// MemoryLeak Fix -wjlee
	//g_pMManager->pem_delete(m_InImg, false);
	g_pMManager->pem_delete(m_InImg, true);
}

BOOL CProcMil::OnOpenDocument(LPCTSTR lpszPathName)
{
	//CFile¿¡¼­ Ã³¸®
	CFile hFile;        //ÆÄÀÏÀ» ÀÐ¾îµéÀÌ¸é ÀÌ Å¬·¡½º Çü½ÄÀ¸·Î ÀúÀå.

	int height;        //ÀÌ¹ÌÁöÀÇ ³ôÀÌ¸¦ ÀúÀåÇÒ º¯¼ö

	int width;        //ÀÌ¹ÌÁöÀÇ °¡·Î±æÀÌ¸¦ ÀúÀåÇÒ º¯¼ö

	hFile.Open(lpszPathName,CFile::modeRead | CFile::typeBinary);    //ÆÄÀÏ ¿­±â

	hFile.Read(&dibHf,sizeof(BITMAPFILEHEADER));                          //dibHf¿¡ ÆÄÀÏÇì´õ¸¦ ÀÐ¾îµéÀÎ´Ù.

	//ÀÌ ÆÄÀÏÀÌ BMPÆÄÀÏÀÎÁö °Ë»ç 
	if(dibHf.bfType!=0x4D42) 
	{
		AfxMessageBox(L"Not BMP file!!");                                        //ÇÁ·ÎÁ§Æ® »ý¼º½Ã À¯´ÏÄÚµå¸¦ »ç¿ëÇÏ°Ô ÇÒ °æ¿ì

		return FALSE;                                                                      //LÀ» ºÙ¿©ÁØ´Ù
	}

	hFile.Read(&dibHi,sizeof(BITMAPINFOHEADER));                             //¿µ»óÁ¤º¸ÀÇ header¸¦ ÀÐ±â

	if(dibHi.biBitCount != 8 && dibHi.biBitCount != 24)                                //8,24ºñÆ®°¡ ¾Æ´Ò°æ¿ì
	{
		AfxMessageBox(L"Gray/True Color Possible!!");

		return FALSE;
	}

	if(dibHi.biBitCount==8)		//8ºñÆ®ÀÇ °æ¿ì ÆÈ·¹Æ®¸¦ »ý¼ºÇØ ÁÖ¾î¾ß ÇÑ´Ù. ÃÑ 256°¡Áö »öÀÌ¹Ç·Î ±× ±æÀÌ¸¸Å­ ÀÐ¾îµéÀÎ´Ù	
		hFile.Read(palRGB,sizeof(RGBQUAD)*256);

	//¸Þ¸ð¸® ÇÒ´ç
	int ImgSize;

	if(dibHi.biBitCount == 8)
	{
		ImgSize = hFile.GetLength()-sizeof(BITMAPFILEHEADER)-sizeof(BITMAPINFOHEADER)-256*sizeof(RGBQUAD);    //ÀÌ¹ÌÁöÀÇ Å©±â´Â ÆÄÀÏ ÃÑ ±æÀÌ¿¡¼­, µÎ Çìµå¿Í ÆÈ·¹Æ®ÀÇ »çÀÌÁî¸¦ 
	}                                                                       //Á¦¿ÜÇÑ´Ù.

	else if(dibHi.biBitCount == 24) //ÄÃ·¯¿µ»ó
	{
		ImgSize = hFile.GetLength()-sizeof(BITMAPFILEHEADER)-sizeof(BITMAPINFOHEADER);
	}


	//ÀÌ¹ÌÁö¸¦ ÀúÀå, Ãâ·ÂÇÒ ¹è¿­»ý¼º.

	//m_InImg = new unsigned char [ImgSize];	
	m_InImg = g_pMManager->pem_new<unsigned char>(true, ImgSize, (PCHAR)__FUNCTION__, __LINE__);

	hFile.Read(m_InImg, ImgSize);

	hFile.Close();

	//ÀÌ¹ÌÁöÀÇ ±æÀÌÁ¤º¸

	m_dwHeight = dibHi.biHeight;

	m_dwWidth = dibHi.biWidth;
	return TRUE;

}

void CProcMil::SaveDebugImg(Im::PIL_ID milBuff, CString Fullpath)
{
	if(Fullpath == "NONE")
		return;
	Im::Buf::Export(Fullpath, M_BMP, milBuff);
}

void CProcMil::SaveDebugImg_float(Im::PIL_ID milSrc, CString Fullpath)
{
	if(Fullpath == "NONE")
		return;
	Im::Buf::Export(Fullpath, M_BMP, milSrc);
}

void CProcMil::SaveDebugImg_float(float* src, int sizeX, int sizeY, CString Fullpath, int band)
{
	if(Fullpath == "NONE")
		return;
	Im::PIL_ID milSrc = M_NULL;

	if(band == 1)
	{
		milSrc = AllocBuff_float(sizeX, sizeY);
		Im::Buf::Put(milSrc, src);
	}
	else if(band == 3)
	{
		milSrc = AllocRotateBuff_Color(sizeX, sizeY);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR32 , M_ALL_BANDS, src);
	}

	Im::Buf::Export(Fullpath, M_BMP, milSrc);

	FreeMilImageBuff(&milSrc);	
}

void CProcMil::CreateDir(CString Path)
{
	if(IsExistDir(Path))
		return;

	CString strPath = Path;
	CString strTemp[20];
	CString strFile;
	int nLength = strPath.GetLength();
	int j = 0;

	for(int i = 0 ; i<20 ; i++)
		strTemp[i].Empty();


	for(int i = 0; i < nLength; i++)
	{
		if(strPath.Mid(i,1) == '\\')
		{
			j++;
		}
		strTemp[j] = strTemp[j] + strPath.Mid(i,1);

	}

	strFile = strTemp[0];
	int i = 1;

	while(strTemp[i] != "")
	{
		strFile = strFile+strTemp[i];
		CreateDirectory(strFile,NULL);
		i++;
	}


}

BOOL CProcMil::IsExistDir(CString path)
{
	BOOL ret = FALSE;
	CFileFind fn;

	BOOL bWorking  = fn.FindFile(path);
	if(bWorking)
	{
		bWorking = fn.FindNextFileW();
		if(fn.IsDirectory())
			ret = TRUE;
	}


	return ret;
}

void CProcMil::SaveBuff(Im::PIL_ID milBuff, CString path)
{
	Im::Buf::Export(path, M_TIFF, milBuff);
	//Im::Buf::Export(path,  M_JPEG_LOSSY, milBuff);		//jpg ÀÐ¾î¿À±â
}

void CProcMil::SaveBuffTIF(Im::PIL_ID milBuff, CString path)
{
	Im::Buf::Export(path, M_TIFF, milBuff);
}

void CProcMil::SaveImage(UCHAR* userBuff, int sizeX, int sizeY, int band, CString path)
{
	if(userBuff == NULL)
		return;

	Im::PIL_ID milBuff = M_NULL;

	if(band == 1)
	{
		milBuff = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(milBuff ,userBuff);
	}
	else if(band == 3)
	{
		milBuff = AllocBuffColor(sizeX, sizeY);
		Im::Buf::PutColor(milBuff ,M_PACKED + M_BGR24, M_ALL_BANDS,userBuff);
	}

	//Im::PIL_ID ChildBuffer = Im::Buf::ChildColor2d(milBuff, M_ALL_BANDS, 0, 0, sizeX, sizeY, M_NULL);

	if(milBuff != M_NULL)		
		Im::Buf::Export(path, M_TIFF, milBuff);
		//Im::Buf::Export(path,  M_JPEG_LOSSY, milBuff);		//jpg ÀÐ¾î¿À±â

	FreeMilImageBuff(&milBuff);
}

void CProcMil::SaveTIFImage(UCHAR* userBuff, int sizeX, int sizeY, int band, CString path)
{
	if(userBuff == NULL)
		return;

	Im::PIL_ID milBuff = M_NULL;

	if(band == 1)
	{
		milBuff = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(milBuff ,userBuff);
	}
	else if(band == 3)
	{
		milBuff = AllocBuffColor(sizeX, sizeY);
		Im::Buf::PutColor(milBuff ,M_PACKED + M_BGR24, M_ALL_BANDS,userBuff);
	}

	//Im::PIL_ID ChildBuffer = Im::Buf::ChildColor2d(milBuff, M_ALL_BANDS, 0, 0, sizeX, sizeY, M_NULL);

	if(milBuff != M_NULL)		
		Im::Buf::Export(path, M_TIFF, milBuff);

	FreeMilImageBuff(&milBuff);
}

void CProcMil::SaveNGimg(UCHAR* userBuf, int orcWidth, int orcHeight, int cx, int cy, int width, int height, CString path)
{
	Im::PIL_ID milColor = M_NULL;
	Im::PIL_ID milTemp = M_NULL;
	CString fullPath = _T("");

	//fullPath.Format(_T("%s.tif"),path);
	fullPath.Format(_T("%s.jpg"),path);		//jpg ÀÐ¾î¿À±â
	milColor = AllocBuffColor(orcWidth, orcHeight);
	Im::Buf::PutColor(milColor,  M_PACKED+M_BGR24, M_ALL_BANDS, userBuf);

	milTemp = AllocClipBuffColor((Im::PIL_ID)milColor, cx, cy, width, height);
	SaveBuff(milTemp, fullPath);


	FreeMilImageBuff(&milColor);
	FreeMilImageBuff(&milTemp);
}

void CProcMil::SaveClipImg(Im::PIL_ID src, int band, int cx, int cy, int width, int height, CString strPath)
{
	Im::PIL_ID milTemp = M_NULL;

	if(band == 3)
	{
		milTemp = AllocClipBuffColor(src, cx, cy, width, height);
	}
	else
	{
		milTemp = AllocClipBuff(src, cx, cy, width, height);
	}

	SaveBuff(milTemp, strPath);


	FreeMilImageBuff(&milTemp);
}

void CProcMil::SaveClipImg_void(void* milSrc, int band, int cx, int cy, int width, int height, CString strPath)
{
	Im::PIL_ID milTemp = M_NULL;

	if(band == 3)
	{
		milTemp = AllocClipBuffColor((Im::PIL_ID)milSrc, cx, cy, width, height);
	}
	else
	{
		milTemp = AllocClipBuff((Im::PIL_ID)milSrc, cx, cy, width, height);
	}

	SaveBuff(milTemp, strPath);

	FreeMilImageBuff(&milTemp);
}

void CProcMil::SaveClipImg(UCHAR* userSrc, int band, int orcWidth, int orcHeight, int cx, int cy, int width, int height, CString strPath)
{
	Im::PIL_ID milTemp = M_NULL;
	Im::PIL_ID milSrc = M_NULL;

	if(band == 3)
	{
		milSrc = AllocBuffColor(orcWidth, orcHeight);
		Im::Buf::PutColor(milSrc,  M_PACKED+M_BGR24, M_ALL_BANDS, userSrc);
		milTemp = AllocClipBuffColor(milSrc, cx, cy, width, height);
	}
	else
	{
		milSrc = AllocBuff(orcWidth, orcHeight);
		Im::Buf::Put(milSrc, userSrc);
		milTemp = AllocClipBuff(milSrc, cx, cy, width, height);
	}

	SaveBuff(milTemp, strPath);


	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milTemp);
}


CString CProcMil::CreatePath(CString path, CString format)
{
	int count = -1;
	CString fullPath = _T("");
	CString temp = _T("");


	for(int i = 0; i<path.GetLength(); i++)
	{
		temp = path.Mid(i,1);
		if(temp == '.')
		{
			count = i;
		}
	}

	if(count == -1)
	{
		fullPath.Format(_T("%s.%s"),path, format);
	}
	else
	{	
		temp = path.Mid(count+1,3);
		if(temp == format)
		{
			fullPath = 	path;
		}
		else
		{
			temp = path.Left(count);
			fullPath.Format(_T("%s.%s"),temp, format); 
		}
	}

	return fullPath;
}


void CProcMil::MakeColorImage(void* milSrcR, void* milSrcG, void* milSrcB, UCHAR* userDst)
{
	Im::PIL_ID milColor = M_NULL;
	CSize imgSize = GetImageSize((Im::PIL_ID)milSrcR);

	milColor = AllocBuffColor((int)imgSize.cx, (int)imgSize.cy);

	Im::Buf::CopyColor((Im::PIL_ID)milSrcR, milColor, M_RED);
	Im::Buf::CopyColor((Im::PIL_ID)milSrcG, milColor, M_GREEN);
	Im::Buf::CopyColor((Im::PIL_ID)milSrcB, milColor, M_BLUE);

	Im::Buf::GetColor(milColor,  M_PACKED+M_BGR24, M_ALL_BANDS, userDst);

	FreeMilImageBuff(&milColor);
}




void CProcMil::Binarize(Im::PIL_ID milSrc, Im::PIL_ID milDst, int thresh, BOOL invert)
{
	if(invert==FALSE)
		Im::im::Binarize(milSrc, milDst, M_GREATER, (double)thresh, M_NULL);    
	else
		Im::im::Binarize(milSrc, milDst, M_LESS, (double)thresh, M_NULL);
}

void CProcMil::Binarize(Im::PIL_ID milSrc, Im::PIL_ID milDst, int threshLow, int threshHigh, BOOL invert)
{
	if (invert == FALSE)
		Im::im::Binarize(milSrc, milDst, M_IN_RANGE, (double)threshLow, (double)threshHigh);
	else
		Im::im::Binarize(milSrc, milDst, M_OUT_RANGE, (double)threshLow, (double)threshHigh);
}

void CProcMil::Binarize(Im::PIL_ID milSrc, Im::PIL_ID milDst, int nTypeRange, int threshLow, int threshHigh, BOOL binvert)
{
	MIL_INT milTypeRange = GetTypeRange(nTypeRange, binvert);
	if(milTypeRange == M_LESS_OR_EQUAL)
	{
		if(binvert)
		{
			threshLow = threshHigh;
			threshHigh = NULL;
		}
		else
			threshHigh = NULL;
	}
	else if(milTypeRange == M_GREATER_OR_EQUAL)
	{
		if(binvert)
			threshHigh = NULL;
		else
		{
			threshLow = threshHigh;
			threshHigh = NULL;
		}
	}

	Im::im::Binarize(milSrc, milDst, milTypeRange, (double)threshLow, (double)threshHigh);
}

void CProcMil::MorOpen(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration)
{
	Im::im::Open( milSrc, milDst, iteration, M_BINARY);	
}

void CProcMil::MorClose(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration)
{
	Im::im::Close( milSrc, milDst, iteration, M_BINARY);	
}

void CProcMil::MorDilate(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration)
{
	Im::im::Dilate( milSrc, milDst, iteration, M_BINARY);
}

void CProcMil::MorErode(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration)
{
	Im::im::Erode( milSrc, milDst, iteration, M_BINARY);	
}

void CProcMil::MorUserErode_V(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration, BOOL isGrayScale)
{
	if(isGrayScale)
		Im::im::Morphic(milSrc, milDst, m_structElement_V, M_ERODE, iteration, M_GRAYSCALE);
	else
		Im::im::Morphic(milSrc, milDst, m_structElement_V, M_ERODE, iteration, M_BINARY);
}

void CProcMil::MorUserErode_H(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration, BOOL isGrayScale)
{
	if(isGrayScale)
		Im::im::Morphic(milSrc, milDst, m_structElement_H, M_ERODE, iteration, M_GRAYSCALE);
	else
		Im::im::Morphic(milSrc, milDst, m_structElement_H, M_ERODE, iteration, M_BINARY);
}

void CProcMil::MorUserDilate_V(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration, BOOL isGrayScale)
{
	if(isGrayScale)
		Im::im::Morphic(milSrc, milDst, m_structElement_V, M_DILATE, iteration, M_GRAYSCALE);
	else
		Im::im::Morphic(milSrc, milDst, m_structElement_V, M_DILATE, iteration, M_BINARY);
}

void CProcMil::MorUserDilate_H(Im::PIL_ID milSrc, Im::PIL_ID milDst, int iteration, BOOL isGrayScale)
{
	if(isGrayScale)
		Im::im::Morphic(milSrc, milDst, m_structElement_H, M_DILATE, iteration, M_GRAYSCALE);
	else
		Im::im::Morphic(milSrc, milDst, m_structElement_H, M_DILATE, iteration, M_BINARY);
}

void CProcMil::MedianFilter(Im::PIL_ID milSrc, Im::PIL_ID milDst, BOOL isGrayScale)
{
	if(isGrayScale)
		Im::im::Rank(milSrc, milDst, M_3X3_RECT , M_MEDIAN, M_GRAYSCALE);
	else
		Im::im::Rank(milSrc, milDst, M_3X3_RECT , M_MEDIAN, M_BINARY);
}

void CProcMil::ArithImage(Im::PIL_ID milSrc1,Im::PIL_ID milSrc2, Im::PIL_ID milDst, int operation, BOOL isSaturation)
{
	MIL_INT type = 0;

	if(operation == M_ADD || operation == M_DIV || operation == M_MULT || operation == M_SUB || operation == M_SUB_ABS
		|| operation == M_ADD_CONST || operation == M_CONST_DIV || operation == M_DIV_CONST || operation == M_MULT_CONST 
		|| operation == M_CONST_SUB || operation == M_SUB_CONST || operation == M_EXP_CONST)
	{
		type = (MIL_INT)operation + (isSaturation ?     M_SATURATION : 0);
	}
	else
	{
		type = (MIL_INT)operation;
	}

	Im::im::Arith(milSrc1, milSrc2, milDst, type);
}



double CProcMil::GetResizeFactor(int viewX, int viewY, int imgX, int imgY, BOOL rotate)
{
	double factor = std::min((viewX * 1.0) / (imgX * 1.0), (viewY * 1.0) / (imgY * 1.0));
	return factor;
}


void CProcMil::ResizeImg(Im::PIL_ID milSrc, Im::PIL_ID milDst, double factor, BOOL imgCenter)
{
	if(imgCenter)
	{
		Im::PIL_ID tempBuf = M_NULL;
		CSize srcSize = GetImageSize(milSrc);
		CSize dstSize = GetImageSize(milDst);

		int resizeCtX = (int)(((srcSize.cx * factor) / 2.0) + 0.5);
		int resizeCtY = (int)(((srcSize.cy * factor) / 2.0) + 0.5);
		int dstCtX = (int)((dstSize.cx / 2.0) + 0.5);
		int dstCtY = (int)((dstSize.cy / 2.0) + 0.5);

		int dx = dstCtX - resizeCtX;
		int dy = dstCtY - resizeCtY;

		MIL_INT band = Im::Buf::Inquire(milSrc, M_SIZE_BAND, M_NULL);

		if(band > 1)
			tempBuf = AllocBuffColor((int)dstSize.cx, (int)dstSize.cy, 0);
		else if(band == 1)
			tempBuf = AllocBuff((int)dstSize.cx, (int)dstSize.cy, 0);

		Im::im::Resize(milSrc, tempBuf, factor, factor, M_DEFAULT);

		Im::Buf::CopyClip(tempBuf, milDst, dx, dy);

		FreeMilImageBuff(&tempBuf);
	}
	else
	{
		Im::im::Resize(milSrc, milDst, factor, factor, M_DEFAULT);
	}
}

void  CProcMil::ResizeImg(UCHAR* userSrc, UCHAR* userDst, CRect bodyRect, int srcWidth, int srcHeight, int dstWidth, int dstHeight, double factor, BOOL imgCenter, BOOL bUseMASK)
{
	Im::PIL_ID tempSrc = M_NULL;
	Im::PIL_ID tempDst = M_NULL;

	Im::PIL_ID remakeSrc = M_NULL;

	tempSrc = AllocBuff(srcWidth, srcHeight);
	Im::Buf::Put(tempSrc, userSrc);
	tempDst = AllocBuff(dstWidth, dstHeight);
	Im::Buf::Put(tempDst, userDst);

	remakeSrc = AllocBuff(srcWidth, srcHeight, 0);
	ArithImage(remakeSrc, tempSrc, remakeSrc, M_ADD);

	if(imgCenter)
	{
		Im::PIL_ID tempBuf = M_NULL;

		int resizeCtX = (int)(((bodyRect.left + (bodyRect.Width() / 2)) * factor) + 0.5);
		int resizeCtY = (int)(((bodyRect.top + (bodyRect.Height() / 2)) * factor) + 0.5);
		int dstCtX = (int)((dstWidth / 2.0) + 0.5);
		int dstCtY = (int)((dstHeight / 2.0) + 0.5);

		int dx = dstCtX - resizeCtX;
		int dy = dstCtY - resizeCtY;

		tempBuf = AllocBuff(dstWidth, dstHeight, 0);
		Im::im::Resize(remakeSrc, tempBuf, factor, factor, M_DEFAULT);


		MIL_DOUBLE stX, stY, edX, edY;
		stX = (MIL_DOUBLE)(bodyRect.left * factor);
		stY = (MIL_DOUBLE)(bodyRect.top * factor);
		edX = (MIL_DOUBLE)(bodyRect.right * factor);
		edY = (MIL_DOUBLE)(bodyRect.bottom * factor);
		Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
		Im::Gra::Rect(M_DEFAULT, tempBuf, stX, stY, edX, edY);

		stX = stX - 1;
		stY = stY - 1;
		edX = edX + 1;
		edY = edY + 1;
		Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
		Im::Gra::Rect(M_DEFAULT, tempBuf, stX, stY, edX, edY);

		// ¸¶½ºÅ© »ç¿ë½Ã¿¡´Â center Á¤·ÄÀ» ÇÏÁö ¾Ê´Â´Ù.
		if(bUseMASK)
		{
			Im::Buf::Get(tempBuf, userDst);
		}
		else
		{
			if (dx < 0) dx = 0;
			if (dy < 0) dy = 0;
			if (dx == 0 && dy == 0)
				dx = 1;
			Im::im::Translate(tempBuf, tempDst, dx, dy, M_BILINEAR + M_OVERSCAN_CLEAR);
			Im::Buf::Get(tempDst, userDst);
		}
	
		FreeMilImageBuff(&tempBuf);
	}
	else
	{
		Im::im::Resize(tempSrc, tempDst, factor, factor, M_DEFAULT);
	}

	FreeMilImageBuff(&tempSrc);
	FreeMilImageBuff(&tempDst);
	FreeMilImageBuff(&remakeSrc);
}

void CProcMil::ResizeImg(UCHAR* userSrc, UCHAR* userDst, int roiSizeX, int roiSizeY, int sizeX, int sizeY, int bandSize, double factor)
{
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	if(bandSize == 1)
	{
		milSrc = AllocBuff(roiSizeX, roiSizeY);
		Im::Buf::Put(milSrc, userSrc);

		milDst = AllocBuff(sizeX, sizeY, 0);
	}
	else if(bandSize == 3)
	{
		milSrc = AllocBuffColor(roiSizeX, roiSizeY);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS, userSrc);

		milDst = AllocBuffColor(sizeX, sizeY, 0);
	}

	int resizeCtX = (int)(((roiSizeX / 2) * factor) + 0.5);
	int resizeCtY = (int)(((roiSizeY / 2) * factor) + 0.5);
	int dstCtX = (int)((sizeX / 2.0) + 0.5);
	int dstCtY = (int)((sizeY / 2.0) + 0.5);

	int dx = dstCtX - resizeCtX;
	int dy = dstCtY - resizeCtY;

	Im::im::Resize(milSrc, milDst, factor, factor, M_DEFAULT);
	

	Im::im::Translate(milDst, milDst, dx, dy, M_BILINEAR + M_OVERSCAN_CLEAR);

	if(bandSize == 1)
	{
		Im::Buf::Get(milDst, userDst);
	}
	else if(bandSize == 3)
	{
		Im::Buf::GetColor(milDst, M_PACKED + M_BGR24, M_ALL_BANDS, userDst);
	}
	

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milDst);
}

void CProcMil::ResizeImg_Left(UCHAR* userSrc, UCHAR* userDst, int roiSizeX, int roiSizeY, int sizeX, int sizeY, int bandSize, double factor)
{
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	if(bandSize == 1)
	{
		milSrc = AllocBuff(roiSizeX, roiSizeY);
		Im::Buf::Put(milSrc, userSrc);

		milDst = AllocBuff(sizeX, sizeY, 0);
	}
	else if(bandSize == 3)
	{
		milSrc = AllocBuffColor(roiSizeX, roiSizeY);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS, userSrc);

		milDst = AllocBuffColor(sizeX, sizeY, 0);
	}

	int resizeCtX = (int)(((roiSizeX / 2) * factor) + 0.5);
	int resizeCtY = (int)(((roiSizeY / 2) * factor) + 0.5);
	int dstCtX = (int)((sizeX / 2.0) + 0.5);
	int dstCtY = (int)((sizeY / 2.0) + 0.5);

	int dx = dstCtX - resizeCtX;
	int dy = dstCtY - resizeCtY;

	Im::im::Resize(milSrc, milDst, factor, factor, M_DEFAULT);


	//Im::im::Translate(milDst, milDst, dx, dy, M_BILINEAR + M_OVERSCAN_CLEAR);

	if(bandSize == 1)
	{
		Im::Buf::Get(milDst, userDst);
	}
	else if(bandSize == 3)
	{
		Im::Buf::GetColor(milDst, M_PACKED + M_BGR24, M_ALL_BANDS, userDst);
	}


	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milDst);
}


void CProcMil::FillHoleBlob(Im::PIL_ID milBinSrc, Im::PIL_ID milDst, Im::PIL_ID BlobResId)
{
	if(BlobResId == NULL)
		Im::Blob::blobReconstruct(milBinSrc, M_NULL, milDst, M_FILL_HOLES, M_BINARY + M_8_CONNECTED);
	else
		Im::Blob::blobReconstruct(milBinSrc, BlobResId, milDst, M_FILL_HOLES, M_BINARY + M_8_CONNECTED);
}

void CProcMil::EraseBorderBlob(Im::PIL_ID milBinSrc, Im::PIL_ID milDst, Im::PIL_ID BlobResId)
{
	if(BlobResId == NULL)
		Im::Blob::blobReconstruct(milBinSrc, M_NULL, milDst, M_ERASE_BORDER_BLOBS, M_BINARY + M_8_CONNECTED);
	else
		Im::Blob::blobReconstruct(milBinSrc, BlobResId, milDst, M_ERASE_BORDER_BLOBS, M_BINARY + M_8_CONNECTED);
	
}



int CProcMil::GetThreshold(Im::PIL_ID milSrc)
{
	if(milSrc == M_NULL)
		return 0;

	MIL_INT autoThreshold = 0;

	autoThreshold = Im::im::Binarize(milSrc,M_NULL,M_DEFAULT,M_DEFAULT,M_DEFAULT);

	return (int)autoThreshold;
}

int CProcMil::GetThreshold(UCHAR* userSrc, int sizeX, int sizeY)
{
	if(userSrc == M_NULL)
		return 0;

	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSrc);

	MIL_INT autoThreshold = 0;

	autoThreshold = Im::im::Binarize(milSrc,M_NULL,M_DEFAULT,M_DEFAULT,M_DEFAULT);

	FreeMilImageBuff(&milSrc);

	return (int)autoThreshold;
}

int CProcMil::GetOtsuThreshold(Im::PIL_ID milSrc)
{
	int thresh = 0;
	CSize sz = GetImageSize(milSrc);

	//long* hisList = new long[256];
	long* hisList = g_pMManager->pem_new<long>(true, 256, (PCHAR)__FUNCTION__, __LINE__);
	memset(hisList, 0, sizeof(long) * 256);

	GetHistoValue(milSrc, hisList);

	double dataSize = sz.cx * sz.cy;
	thresh = (int)CalcAutoThreshold(hisList, dataSize, 0, 255);

	//delete hisList;
	g_pMManager->pem_delete(hisList, false);
	return thresh;
}

int CProcMil::GetOtsuThreshold(UCHAR* userSrc, int sizeX, int sizeY)
{
	int thresh = 0;

	if(userSrc == NULL || sizeX <= 0 || sizeY <= 0)
		return thresh;

	Im::PIL_ID milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSrc);

	thresh = GetOtsuThreshold(milSrc);

	FreeMilImageBuff(&milSrc);

	return thresh;
}

long CProcMil::CalcAutoThreshold(long* pHistList, double dataCnt, long minValue, long maxValue)
{
	double	vb, w0, w1, u0, u1, vbmax;
	long	k, k1, k2, count;

	long th = 0;

	k1 = minValue;
	while (pHistList[k1] == 0) {
		k1 ++;
	}

	k2 = maxValue;
	while (pHistList[k2] == 0) {
		k2 --;
	}

	vbmax = 0.0;

	for (k = k1; k < k2 - 1; k ++) {
		w0 = 0.0;
		w1 = 0.0;
		u0 = 0.0;
		u1 = 0.0;


		for (count = k1; count <= k; count ++) {
			w0 = w0 + (double)pHistList[count] / dataCnt;
		}
		for (count = k + 1; count <= k2; count ++) {
			w1 = w1 + (double)pHistList[count] / dataCnt;
		}

		
		for (count = k1; count <= k; count ++) {
			u0 = u0 + (double)count * (double)pHistList[count] / (w0 * dataCnt);
		}
		for (count = k + 1; count <= k2; count ++) {
			u1 = u1 + (double)count * (double)pHistList[count] / (w1 * dataCnt);
		}


		vb = w0 * w1 * (u1 - u0) * (u1 - u0);
		if (vb > vbmax) {
			vbmax = vb;
			th = k;
		}
	}

	return th;
}

void CProcMil::EdgeDetect(Im::PIL_ID milSrc, Im::PIL_ID milDst)
{
	Im::im::EdgeDetect( milSrc, milDst, M_NULL, M_SOBEL, M_FAST_EDGE_DETECT, 1);
}

void CProcMil::EdgeDetect(UCHAR* userSrc, UCHAR* userDst, int width, int height)
{
	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(width, height);

	Im::Buf::Put(milSrc, userSrc);

	Binarize(milSrc, milSrc, 1 , FALSE);

	Im::im::EdgeDetect( milSrc, milSrc, M_NULL, M_SOBEL + M_OVERSCAN_DISABLE , M_FAST_EDGE_DETECT, M_NULL);

	Im::Buf::Get(milSrc, userDst);

	FreeMilImageBuff(&milSrc);
}



void CProcMil::MakeBlob(Im::PIL_ID milSrc, Im::PIL_ID milDst)
{
	UCHAR* ucBuf;
	Im::PIL_ID tempBuf = M_NULL;

	int width = (int)Im::Buf::Inquire(milSrc,M_SIZE_X,M_NULL);
	int length = (int)Im::Buf::Inquire(milSrc,M_SIZE_Y,M_NULL);

	if(width < 0 || length < 0)
		return;

	//ucBuf = new UCHAR[width * length];
	ucBuf = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
	Im::Buf::Get(milSrc, ucBuf);


	//draw line
	for(int i=10; i<length-10; i++)
	{	
		int j = 0;
		ucBuf[width * i + j] = 255;

		j = width-1;
		ucBuf[width * i + j] = 255;
	}

	for(int j=10; j<width-10; j++)
	{	
		int i = 0;
		ucBuf[width * i + j] = 255;

		i = length-1;
		ucBuf[width * i + j] = 255;
	}

	tempBuf = AllocBuff(width, length);
	Im::Buf::Put(tempBuf, ucBuf);


	Im::Blob::blobReconstruct(tempBuf, M_NULL, tempBuf, M_FILL_HOLES, M_8_CONNECTED);
	Im::Buf::Get(tempBuf, ucBuf);


	// delete line
	for(int i=10; i<length-10; i++)
	{	
		int j = 0;
		ucBuf[width * i + j] = ucBuf[width * i + (j+1)];		// left

		j = width-1;
		ucBuf[width * i + j] = ucBuf[width * i + (j-1)];			// right
	}

	for(int j=10; j<width-10; j++)
	{	
		int i = 0;
		ucBuf[width * i + j] = ucBuf[width * (i+1) + j] ;		// top

		i = length-1;
		ucBuf[width * i + j] = ucBuf[width * (i-1) + j] ;		// bottom
	}

	Im::Buf::Put(milDst,ucBuf);

	FreeMilImageBuff(&tempBuf);
	//delete ucBuf;
	g_pMManager->pem_delete(ucBuf, false);
}



//////////////////////////////////////////////////////////////////////////zmap

Im::PIL_ID CProcMil::AllocBuff_float(int width, int height, int initVal)
{
	Im::PIL_ID milBuff = M_NULL;
	Im::Buf::Alloc2d(*m_milSys, (MIL_INT)width, (MIL_INT)height, 32 + M_FLOAT, M_IMAGE + M_PROC, &milBuff);

	if(initVal >= 0 && initVal <= 255)
		Im::Buf::Clear(milBuff, initVal);

	return milBuff;
}

Im::PIL_ID CProcMil::AllocClipBuff_float(Im::PIL_ID milSrc, int cX, int cY, int width, int height)
{
	Im::PIL_ID milBuff = M_NULL;
	int startX = 0;
	int startY = 0;

	Im::Buf::Alloc2d(*m_milSys, width, height, 32 + M_FLOAT, M_IMAGE + M_PROC, &milBuff);
	Im::Buf::Clear(milBuff, 0);

	startX = cX - (width/2);
	startY = cY - (height/2);

	Im::Buf::CopyClip(milSrc, milBuff, (MIL_INT)-startX, (MIL_INT)-startY);

	return milBuff;
}

Im::PIL_ID CProcMil::AllocRotateBuff_float(Im::PIL_ID milSrc, double angle)
{
	Im::PIL_ID milBuff = M_NULL;
	MIL_INT sizeX = Im::Buf::Inquire(milSrc, M_SIZE_X, M_NULL);
	MIL_INT sizeY = Im::Buf::Inquire(milSrc, M_SIZE_Y, M_NULL);

	if(angle == 90 || angle == 270)
	{
		Im::Buf::Alloc2d(*m_milSys, (MIL_INT)sizeY, (MIL_INT)sizeX, 32 + M_FLOAT, M_IMAGE + M_PROC, &milBuff);	
	}
	else
	{
		Im::Buf::Alloc2d(*m_milSys, (MIL_INT)sizeX, (MIL_INT)sizeY, 32 + M_FLOAT, M_IMAGE + M_PROC, &milBuff);
	}

	GetRotateImg(milSrc, milBuff, angle);

	return milBuff;

}

void CProcMil::ClipZmap(float* zmapData, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int width, int height)
{
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	milSrc = AllocBuff_float(orgSizeX, orgSizeY);
	Im::Buf::Put(milSrc, zmapData);

	milDst = AllocClipBuff_float(milSrc, cx, cy, width, height);
	Im::Buf::Get(milDst, dst);

	FreeMilImageBuff(& milSrc);
	FreeMilImageBuff(& milDst);
}

float CProcMil::ClipZmap_ipp(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY)
{
	if(src == NULL || roiSizeX < 0 || roiSizeY < 0)
		return -1;

	//SHK 2013/11/15 offset ¿¹¿ÜÃ³¸®
	int stX = 0;
	int stY = 0;
	int endX = 0;
	int endY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	BOOL flagX = FALSE;
	BOOL flagY = FALSE;

	if(stX < 0)
	{
		stX = 0;
		flagX = TRUE;
	}

	if(stY < 0)
	{
		stY = 0;
		flagY = TRUE;
	}

	endX = cx + (width / 2);  // end point x
	endY = cy + (height / 2);  // end point Y

	if(endX > orgSizeX)
	{
		endX = orgSizeX;
		flagX = TRUE;
	}

	if(endY > orgSizeY)
	{
		endY = orgSizeY;
		flagY = TRUE;
	}

	if(flagX == TRUE)
		width = endX - stX;

	if(flagY == TRUE)
		height = endY - stY;


	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	sts = ippiCopy_32f_C1R(src + offX + offY * orgSizeX, orgSizeX * 4, dst, width * 4, iSize);

	return (float)sts;
}

void CProcMil::RotateZmap(float* zmapData, float* dst, int orgSizeX, int orgSizeY, double angle)
{
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	milSrc = AllocBuff_float(orgSizeX, orgSizeY);
	Im::Buf::Put(milSrc, zmapData);

	milDst = AllocRotateBuff_float(milSrc, angle);
	Im::Buf::Get(milDst, dst);

	FreeMilImageBuff(& milSrc);
	FreeMilImageBuff(& milDst);
}


void CProcMil::GetHistoValue(Im::PIL_ID milSrc, int* retValue)
{
	Im::PIL_ID histResult;
	MIL_INT32 histValues[256] = {0,};
	MIL_INT histNum =  256;

	/* Allocate a histogram result buffer. */
	Im::im::AllocResult(*m_milSys, histNum, M_HIST_LIST, &histResult);

	/* Calculate the histogram. */
	Im::im::Histogram(milSrc, histResult);

	/* Get the results. */
	Im::im::GetResult(histResult, M_VALUE + M_TYPE_MIL_INT32, histValues);

	memcpy_s(retValue,(sizeof(int) * 256),histValues,(sizeof(MIL_INT32) * 256));
	Im::im::Free(histResult);
}

void CProcMil::GetHistoValue(Im::PIL_ID milSrc, long* retValue)
{
	Im::PIL_ID histResult;
	long histValues[256] = {0,};
	MIL_INT histNum =  256;

	/* Allocate a histogram result buffer. */
	Im::im::AllocResult(*m_milSys, histNum, M_HIST_LIST, &histResult);

	/* Calculate the histogram. */
	Im::im::Histogram(milSrc, histResult);

	/* Get the results. */
	Im::im::GetResult(histResult, M_VALUE + M_TYPE_LONG, histValues);

	memcpy_s(retValue,(sizeof(long) * 256),histValues,(sizeof(long) * 256));
	Im::im::Free(histResult);
}

void CProcMil::GetHistoValue(long* src, int srcCount, int retCount, long* retValue)
{
// 	for (int index = 0; index < retCount; index ++)
// 		retValue[index] = 0;

	long value = 0;
	for (int i = 0; i < srcCount; i++)
	{
		value = src[i];
		if(value < 0)
			value = 0;

		if(value >= retCount)
			value = retCount - 1;

		retValue[value] = retValue[value] + 1;
	}

}

// void CProcMil::GetHistoValue(void* milSrc, int* retValue)
// {
// 	Im::PIL_ID histResult;
// 	MIL_INT32 histValues[256] = {0,};
// 	MIL_INT histNum =  256;
// 
// 	/* Allocate a histogram result buffer. */
// 	Im::im::AllocResult(*m_milSys, histNum, M_HIST_LIST, &histResult);
// 
// 	/* Calculate the histogram. */
// 	Im::im::Histogram((Im::PIL_ID)milSrc, histResult);
// 
// 	/* Get the results. */
// 	Im::im::GetResult(histResult, M_VALUE + M_TYPE_MIL_INT32, histValues);
// 
// 	memcpy_s(retValue,(sizeof(int) * 256),histValues,(sizeof(MIL_INT32) * 256));
// 	Im::im::Free(histResult);
// }

void CProcMil::GetHistoValue(UCHAR* userSrc, int sizeX, int sizeY, int* retValue)
{
	Im::PIL_ID histResult;
	MIL_INT32 histValues[256] = {0,};
	MIL_INT histNum =  256;
	Im::PIL_ID milTemp = M_NULL;

	milTemp = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milTemp, userSrc);

	/* Allocate a histogram result buffer. */
	Im::im::AllocResult(*m_milSys, histNum, M_HIST_LIST, &histResult);

	/* Calculate the histogram. */
	Im::im::Histogram(milTemp, histResult);

	/* Get the results. */
	Im::im::GetResult(histResult, M_VALUE + M_TYPE_MIL_INT32, histValues);

	memcpy_s(retValue,(sizeof(int) * 256),histValues,(sizeof(MIL_INT32) * 256));
	Im::im::Free(histResult);

	FreeMilImageBuff(&milTemp);
}


void CProcMil::GetHistoValue( UCHAR* userSrc, int nSttX, int nSttY, int nSizeX, int nSizeY, int* retValue )
{
	Im::PIL_ID milTemp = AllocBuff(nSizeX, nSizeY);
	Im::Buf::Put2d(milTemp, nSttX, nSttY, nSizeX, nSizeY, userSrc);
	SaveWorkImg(milTemp, _T("Clip_copy.bmp"));

	GetHistoValue(milTemp, retValue);

	FreeMilImageBuff(&milTemp);
}

void  CProcMil::MakeHistogramImg(int* histo, int histoSize, UCHAR* userDst)
{
	//x = 64
	//y = 34
	
	int interval = 1;
	int textAreaX = 50;
	int marg = 14;
	int textAreaY = 20;
	int graInterval = 4;
	int textSt_L = 10;
	
	

	float maxData = (float)histo[0];
	for(int i = 0; i < histoSize; i++)
	{
		if(maxData < (float)histo[i])
			maxData = (float)histo[i];
	}


	int sizeX = histoSize * interval + textAreaX + marg;
	int sizeY = histoSize * interval + textAreaY + marg;


	Im::PIL_ID milTemp = M_NULL;
	milTemp = AllocBuffColor(sizeX, sizeY, 0);

	/*int64* stX = new int64[histoSize];
	int64* stY = new int64[histoSize];
	int64* edX = new int64[histoSize];
	int64* edY = new int64[histoSize];*/
	int64* stX = g_pMManager->pem_new<int64>(true, histoSize, (PCHAR)__FUNCTION__, __LINE__);
	int64* stY = g_pMManager->pem_new<int64>(true, histoSize, (PCHAR)__FUNCTION__, __LINE__);
	int64* edX = g_pMManager->pem_new<int64>(true, histoSize, (PCHAR)__FUNCTION__, __LINE__);
	int64* edY = g_pMManager->pem_new<int64>(true, histoSize, (PCHAR)__FUNCTION__, __LINE__);

	float temp = (sizeY - textAreaY -  marg) / maxData;
	//histo data
	for(int i = 0; i < histoSize; i++)
	{
		stX[i] = (int64)(i * interval + textAreaX);
		stY[i] = (int64)(sizeY - textAreaY);

		edX[i] = (int64)(i * interval + textAreaX);
		edY[i] = (int64)((sizeY - textAreaY) - (histo[i] * temp));
	}

	

	//////////////////////////////////////////////////////////////////////////
	//lable ¿µ¿ª »ö.
/*	Im::Gra::Color(M_DEFAULT, M_COLOR_GRAY);
	Im::Gra::Rect(M_DEFAULT, milTemp, 0, marg, textAreaX - 1, sizeY - textAreaY);
	Im::Gra::RectFill(M_DEFAULT, milTemp, 0, marg, textAreaX - 1, sizeY - textAreaY);

	Im::Gra::Rect(M_DEFAULT, milTemp, 0, sizeY - textAreaY + 1, sizeX - marg, sizeY);
	Im::Gra::RectFill(M_DEFAULT, milTemp, 0, sizeY - textAreaY + 1, sizeX - marg, sizeY);*/


	/*int64* graStX_L = new int64[graInterval];
	int64* graStY_L = new int64[graInterval];
	int64* graEdX_L = new int64[graInterval];
	int64* graEdY_L = new int64[graInterval];*/
	int64* graStX_L = g_pMManager->pem_new<int64>(true, graInterval, (PCHAR)__FUNCTION__, __LINE__);
	int64* graStY_L = g_pMManager->pem_new<int64>(true, graInterval, (PCHAR)__FUNCTION__, __LINE__);
	int64* graEdX_L = g_pMManager->pem_new<int64>(true, graInterval, (PCHAR)__FUNCTION__, __LINE__);
	int64* graEdY_L = g_pMManager->pem_new<int64>(true, graInterval, (PCHAR)__FUNCTION__, __LINE__);
	
	double num_L = maxData / (double)(graInterval + 1);


	/*int64* graStX_B = new int64[graInterval];
	int64* graStY_B = new int64[graInterval];
	int64* graEdX_B = new int64[graInterval];
	int64* graEdY_B = new int64[graInterval];*/
	int64* graStX_B = g_pMManager->pem_new<int64>(true, graInterval, (PCHAR)__FUNCTION__, __LINE__);
	int64* graStY_B = g_pMManager->pem_new<int64>(true, graInterval, (PCHAR)__FUNCTION__, __LINE__);
	int64* graEdX_B = g_pMManager->pem_new<int64>(true, graInterval, (PCHAR)__FUNCTION__, __LINE__);
	int64* graEdY_B = g_pMManager->pem_new<int64>(true, graInterval, (PCHAR)__FUNCTION__, __LINE__);
	double num_B = histoSize / (double)(graInterval + 1);


	int dist = (histoSize * interval) / (graInterval + 1);
	int graLen = sizeX - textAreaX - marg;

	CString strNum = _T("");
	for(int i = 0; i < graInterval; i++)
	{
		//°¡·Î line data
		graStX_L[i] = (MIL_INT)textAreaX;
		graStY_L[i] = (MIL_INT)((sizeY  - ((i+1) * dist))  - textAreaY );
		
		graEdX_L[i] = (MIL_INT)(textAreaX + graLen);
		graEdY_L[i] = (MIL_INT)((sizeY - ((i+1) * dist))  - textAreaY );


		//¼¼·Î line data
		graStX_B[i] = (MIL_INT)textAreaX + (50 * (i + 1));//(textAreaX + ((i+1) * dist));
		graStY_B[i] = (MIL_INT)(sizeY - textAreaY);

		graEdX_B[i] = (MIL_INT)textAreaX + (50 * (i + 1));//(textAreaX + ((i+1) * dist));
		graEdY_B[i] = (MIL_INT)((sizeY - textAreaY) - graLen);

		//yÃà lable
		strNum.Format(_T("%.0f"), num_L * (i + 1));
		Im::Gra::Text(M_DEFAULT, milTemp, textSt_L, graStY_L[i] - 6, strNum);
		//xÃà lable
		strNum.Format(_T("%d"), 50 * (i + 1));
		Im::Gra::Text(M_DEFAULT, milTemp, graStX_B[i] - 10, (sizeY - textAreaY) + 5, strNum);
	}
	//¿øÁ¡
	Im::Gra::Text(M_DEFAULT, milTemp, textSt_L, (graStY_L[0] - 6) + dist, _T("0"));
	//yÃà max
	strNum.Format(_T("%.0f"), maxData);
	Im::Gra::Text(M_DEFAULT, milTemp, textSt_L, marg - 5, strNum);
	//xÃà max
	strNum.Format(_T("%d"), histoSize - 1);
	Im::Gra::Text(M_DEFAULT, milTemp, (graStX_B[graInterval - 1] + dist + marg) - 25, (sizeY - textAreaY) + 5, strNum);

	Im::Gra::Color(M_DEFAULT, M_COLOR_DARK_CYAN);

	//°¡·Î line
	Im::Gra::Lines(M_DEFAULT, milTemp, graInterval, graStX_L, graStY_L, graEdX_L, graEdY_L, M_DEFAULT);
	//¼¼·Î line
	Im::Gra::Lines(M_DEFAULT, milTemp, graInterval, graStX_B, graStY_B, graEdX_B, graEdY_B, M_DEFAULT);

	//rect
	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	Im::Gra::Rect(M_DEFAULT, milTemp, 0 + textAreaX - 1, marg, ((sizeX - 1) - marg) + 1, (sizeY - textAreaY));

	//histo
	Im::Gra::Color(M_DEFAULT, M_COLOR_RED);
	Im::Gra::Lines(M_DEFAULT, milTemp, histoSize, stX, stY, edX, edY, M_DEFAULT);

	SaveWorkImg(milTemp, _T("histo.bmp"));

	if(userDst != NULL)
		Im::Buf::GetColor(milTemp, M_PACKED+M_BGR24, M_ALL_BANDS, userDst);

	FreeMilImageBuff(&milTemp);

	/*delete stX;
	delete stY;
	delete edX;
	delete edY;

	delete graStX_L;
	delete graStY_L;
	delete graEdX_L;
	delete graEdY_L;

	delete graStX_B;
	delete graStY_B;
	delete graEdX_B;
	delete graEdY_B;*/
	g_pMManager->pem_delete(stX, true);
	g_pMManager->pem_delete(stY, true);
	g_pMManager->pem_delete(edX, true);
	g_pMManager->pem_delete(edY, true);

	g_pMManager->pem_delete(graStX_L, true);
	g_pMManager->pem_delete(graStY_L, true);
	g_pMManager->pem_delete(graEdX_L, true);
	g_pMManager->pem_delete(graEdY_L, true);

	g_pMManager->pem_delete(graStX_B, true);
	g_pMManager->pem_delete(graStY_B, true);
	g_pMManager->pem_delete(graEdX_B, true);
	g_pMManager->pem_delete(graEdY_B, true);

}


void CProcMil::FillBlob_Outside(Im::PIL_ID src, Im::PIL_ID dst)
{
	return;  //JACKY 2013/10/01

	if(src == M_NULL || dst == M_NULL)
		return ;

	UCHAR* ucBuf;

	int nWidth = (int)Im::Buf::Inquire(src,M_SIZE_X,M_NULL);
	int nLength = (int)Im::Buf::Inquire(src,M_SIZE_Y,M_NULL);



	//ucBuf = (UCHAR *)malloc(nWidth*nLength*sizeof(UCHAR));
	ucBuf = (UCHAR *)g_pMManager->pem_malloc(nWidth*nLength * sizeof(UCHAR), (PCHAR)__FUNCTION__, __LINE__);
	Im::Buf::Get2d(src,0,0,(MIL_INT)nWidth,(MIL_INT)nLength,ucBuf);

	int		nFillStart = 0;
	bool	flag = false;		
	for( int m=0; m<nLength; m++)		//°¡·Î¹æÇâ
	{
		nFillStart = 0;
		flag = false;
		for( int n=0; n<nWidth-1; n++)
		{
			if( ucBuf[ nWidth * m + n] == 255 && ucBuf[nWidth * m + (n+1)] == 0)
			{
				nFillStart = n+1;
				flag = true;
			}

			if( ucBuf[ nWidth * m + n] == 0 &&ucBuf[ nWidth * m + (n+1)] == 255)
			{
				if( flag == true)
				{
					for( int k=nFillStart; k<=n; k++)
					{
						ucBuf[ nWidth * m + k] = 255;
					}
					flag = false;
				}
			}
		}
	}


	nFillStart = 0;
	flag = false;
	for( int n=0; n<nWidth; n++)		  //¼¼·Î¹æÇâ
	{
		nFillStart = 0;
		flag = false;
		for( int m=0; m<nLength-1; m++)
		{
			if( ucBuf[ nWidth * m + n] == 255 && ucBuf[ nWidth * (m+1) + n] == 0)
			{
				nFillStart = m+1;
				flag = true;
			}

			if(ucBuf[ nWidth * m + n] == 0 && ucBuf[ nWidth * (m+1) + n] == 255)
			{
				if( flag == true)
				{
					for( int k=nFillStart; k<=m; k++)
					{
						ucBuf[ nWidth * k + n] = 255;
					}
					flag = false;
				}
			}
		}
	}

	Im::Buf::Put2d(dst,0,0,(MIL_INT)nWidth,(MIL_INT)nLength,ucBuf);

	//free(ucBuf) ;	
	g_pMManager->pem_free(ucBuf);
}


void CProcMil::FillBlob_Outside(UCHAR* userSrc, int sizeX, int sizeY, UCHAR* userDst)
{
	return;  //JACKY 2013/10/01

	if(userSrc == NULL || userDst == NULL || sizeX <= 0 || sizeY <= 0)
		return;

	int nWidth = sizeX;
	int nLength = sizeY;

	int		nFillStart = 0;
	bool	flag = false;		
	for( int m=0; m<nLength; m++)		//°¡·Î¹æÇâ
	{
		nFillStart = 0;
		flag = false;
		for( int n=0; n<nWidth-1; n++)
		{
			if( userSrc[ nWidth * m + n] == 255 && userSrc[nWidth * m + (n+1)] == 0)
			{
				nFillStart = n+1;
				flag = true;
			}

			if( userSrc[ nWidth * m + n] == 0 &&userSrc[ nWidth * m + (n+1)] == 255)
			{
				if( flag == true)
				{
					for( int k=nFillStart; k<=n; k++)
					{
						userDst[ nWidth * m + k] = 255;
					}
					flag = false;
				}
			}
		}
	}


	nFillStart = 0;
	flag = false;
	for( int n=0; n<nWidth; n++)		  //¼¼·Î¹æÇâ
	{
		nFillStart = 0;
		flag = false;
		for( int m=0; m<nLength-1; m++)
		{
			if( userSrc[ nWidth * m + n] == 255 && userSrc[ nWidth * (m+1) + n] == 0)
			{
				nFillStart = m+1;
				flag = true;
			}

			if(userSrc[ nWidth * m + n] == 0 && userSrc[ nWidth * (m+1) + n] == 255)
			{
				if( flag == true)
				{
					for( int k=nFillStart; k<=m; k++)
					{
						userDst[ nWidth * k + n] = 255;
					}
					flag = false;
				}
			}
		}
	}

}

void CProcMil::DrawSigleRect(Im::PIL_ID milDst, int sizeX, int sizeY, CRect rect, BOOL isSave, CString strPath)
{

	Im::PIL_ID milTemp = M_NULL;
	milTemp = AllocBuff(sizeX, sizeY, 0);


	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	Im::Gra::RectFill(M_DEFAULT, milTemp, rect.left, rect.top, rect.right, rect.bottom);


	if(milDst != M_NULL)
		Im::Buf::Copy(milTemp, milDst);

	if(isSave == TRUE)
	{
		if(strPath.IsEmpty())
			SaveWorkImg(milTemp, _T("DrawRect.bmp"));
		else
			Im::Buf::Export(strPath + _T(".bmp"), M_BMP, milTemp);
	}

	FreeMilImageBuff(&milTemp);
}


void CProcMil::DrawRectM(Im::PIL_ID milDst, int sizeX, int sizeY, CRect* rect, int cnt, BOOL isSave, CString strPath)
{

	Im::PIL_ID milTemp = M_NULL;
	milTemp = AllocBuff(sizeX, sizeY, 0);


	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	for(int i = 0; i < cnt; i++)
	{	
		Im::Gra::RectFill(M_DEFAULT, milTemp, rect[i].left, rect[i].top, rect[i].right, rect[i].bottom);
	}

	if(milDst != M_NULL)
		Im::Buf::Copy(milTemp, milDst);

	if(isSave == TRUE)
	{
		if(strPath.IsEmpty())
			SaveWorkImg(milTemp, _T("DrawRect.bmp"));
		else
			Im::Buf::Export(strPath + _T(".bmp"), M_BMP, milTemp);
	}
	
	FreeMilImageBuff(&milTemp);
}

void CProcMil::DrawRectSingleM(Im::PIL_ID milDst, int sizeX, int sizeY, CRect rect, BOOL isSave, CString strPath)
{
	Im::PIL_ID milTemp = M_NULL;
	milTemp = AllocBuff(sizeX, sizeY, 0);


	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	Im::Gra::RectFill(M_DEFAULT, milTemp, rect.left, rect.top, rect.right, rect.bottom);
	

	if(milDst != M_NULL)
		Im::Buf::Copy(milTemp, milDst);

	if(isSave == TRUE)
	{
		if(strPath.IsEmpty())
			SaveWorkImg(milTemp, _T("DrawRect.bmp"));
		else
			Im::Buf::Export(strPath + _T(".bmp"), M_BMP, milTemp);
	}

	FreeMilImageBuff(&milTemp);
}

void CProcMil::DrawRectU(UCHAR* userDst, int sizeX, int sizeY, CRect* rect, int cnt, BOOL isSave, CString strPath)
{
	Im::PIL_ID milTemp = M_NULL;

	if(userDst != NULL)
		milTemp = AllocBuff(sizeX, sizeY, 0);

	DrawRectM(milTemp, sizeX, sizeY, rect, cnt, isSave, strPath);

	if(userDst != NULL)
		Im::Buf::Get(milTemp, userDst);

	if(milTemp != M_NULL)
		FreeMilImageBuff(&milTemp);
}

void CProcMil::DrawRectSingleU(UCHAR* userDst, int sizeX, int sizeY, CRect rect, BOOL isSave, CString strPath)
{
	Im::PIL_ID milTemp = M_NULL;

	if(userDst != NULL)
		milTemp = AllocBuff(sizeX, sizeY, 0);

	DrawRectSingleM(milTemp, sizeX, sizeY, rect, isSave, strPath);

	if(userDst != NULL)
		Im::Buf::Get(milTemp, userDst);

	if(milTemp != M_NULL)
		FreeMilImageBuff(&milTemp);
}




void CProcMil::GetProjection_V(Im::PIL_ID milSrc, long* retList)
{
	CSize imgSize = GetImageSize(milSrc);
	int size = imgSize.cy;


	Im::PIL_ID milPrjList = M_NULL;
	Im::im::AllocResult(*m_milSys, size, M_PROJ_LIST, &milPrjList);

	Im::im::Project(milSrc, milPrjList, M_90_DEGREE );

	//long* prjList = new long[size];
	long* prjList = g_pMManager->pem_new<long>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	memset(prjList, 0, sizeof(long)* size);

	Im::im::GetResult(milPrjList, M_VALUE + M_TYPE_LONG , prjList);


	memcpy_s(retList, sizeof(long) * size, prjList, sizeof(long) * size);

	Im::im::Free(milPrjList);
	//delete prjList;
	g_pMManager->pem_delete(prjList, false);
}

void CProcMil::GetProjection_V(Im::PIL_ID milSrc, float* retList)
{
	CSize imgSize = GetImageSize(milSrc);
	int size = imgSize.cy;


	Im::PIL_ID milPrjList = M_NULL;
	Im::im::AllocResult(*m_milSys, size, M_PROJ_LIST, &milPrjList);

	Im::im::Project(milSrc, milPrjList, M_90_DEGREE );

	//float* prjList = new float[size];
	float* prjList = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	memset(prjList, 0, sizeof(float)* size);

	Im::im::GetResult(milPrjList, M_VALUE + M_TYPE_FLOAT , prjList);


	memcpy_s(retList, sizeof(float) * size, prjList, sizeof(float) * size);

	Im::im::Free(milPrjList);
	//delete prjList;
	g_pMManager->pem_delete(prjList, false);
}

void CProcMil::GetProjection_H(Im::PIL_ID milSrc, long* retList)
{
	CSize imgSize = GetImageSize(milSrc);
	int size = imgSize.cx;


	Im::PIL_ID milPrjList = M_NULL;
	Im::im::AllocResult(*m_milSys, size, M_PROJ_LIST, &milPrjList);

	Im::im::Project(milSrc, milPrjList, M_0_DEGREE );

	//long* prjList = new long[size];
	long* prjList = g_pMManager->pem_new<long>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	memset(prjList, 0, sizeof(long)* size);

	Im::im::GetResult(milPrjList, M_VALUE + M_TYPE_LONG , prjList);


	memcpy_s(retList, sizeof(long) * size, prjList, sizeof(long) * size);

	Im::im::Free(milPrjList);
	//delete prjList;
	g_pMManager->pem_delete(prjList, false);
}
//shk 2014-01-03
void CProcMil::GetProjection_H(Im::PIL_ID milSrc, float* retList)
{
	CSize imgSize = GetImageSize(milSrc);
	int size = imgSize.cx;


	Im::PIL_ID milPrjList = M_NULL;
	Im::im::AllocResult(*m_milSys, size, M_PROJ_LIST, &milPrjList);

	Im::im::Project(milSrc, milPrjList, M_0_DEGREE );

	//float* prjList = new float[size];
	float* prjList = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	memset(prjList, 0, sizeof(float)* size);

	Im::im::GetResult(milPrjList, M_VALUE + M_TYPE_FLOAT , prjList);


	memcpy_s(retList, sizeof(float) * size, prjList, sizeof(float) * size);

	Im::im::Free(milPrjList);
	//delete prjList;
	g_pMManager->pem_delete(prjList, false);
}


void CProcMil::DrawBlob(Im::PIL_ID milBlobResultId, Im::PIL_ID milDst, int label, BOOL reverse)
{
	if(milBlobResultId == M_NULL || milDst == M_NULL)
		return;

	if(!reverse)// blob: white , background: black
	{
		Im::Blob::graColor(M_DEFAULT, M_COLOR_WHITE);
		Im::Blob::blobDraw(M_DEFAULT , milBlobResultId, milDst, M_DRAW_BLOBS, label, M_DEFAULT);
	}
	else// blob: black , background: white
	{	
		Im::Buf::Clear(milDst, M_COLOR_WHITE);
		Im::Blob::graColor(M_DEFAULT, M_COLOR_BLACK);
		Im::Blob::blobDraw(M_DEFAULT , milBlobResultId, milDst, M_DRAW_BLOBS, label, M_DEFAULT);
		Im::Blob::graColor(M_DEFAULT, M_COLOR_WHITE); //¿øº¹~
	}
}
void CProcMil::DrawBlobLabels(Im::PIL_ID milBlobResultId, Im::PIL_ID milDst, BOOL reverse)
{
	if (milBlobResultId == M_NULL || milDst == M_NULL)
		return;


	Im::Blob::blobLabel(milBlobResultId, milDst, reverse);

}


void CProcMil::DrawLeadNgRect(UCHAR* userSrc, UCHAR* userDst, int sizeX, int sizeY, int srcColorBand, CRect wndRect, CRect* leadRect, int count, int marginX, int marginY, int color)
{
	//color = 0: lead   1: solder   2: gap
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	if(srcColorBand == 1)
	{
		milSrc = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(milSrc, userSrc);
	}
	else if(srcColorBand == 3)
	{
		milSrc = AllocBuffColor(sizeX, sizeY);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS, userSrc);
	}

	milDst = AllocBuffColor(sizeX, sizeY);
	Im::Buf::CopyColor(milSrc, milDst, M_ALL_BANDS);

	MIL_DOUBLE milColor = 0;
	if(color == 0)//lead
		milColor = M_COLOR_GREEN;
	if(color == 1)//solder
		milColor = M_COLOR_RED;
	if(color == 2)//gap
		milColor = M_COLOR_GRAY;
	if(color == 3)//lead wnd
		milColor = M_COLOR_GREEN;

	int lineStrong = 2;
	int stX = 0;
	int stY = 0;
	int edX = 0;
	int edY = 0;

	//huj 2014/01/15   ÀÌ¹ÌÁö¸¦ Ãà¼ÒÇÏ¸é ¶óÀÎÀÌ ¾Èº¸ÀÌ´Â °æ¿ì°¡ »ý°Ü Å©±â¿¡µû¶ó ¶óÀÎ ±½ÀÌ¸¦ º¯È­ ½ÃÅ²´Ù.
	int std = 256;
	if(sizeX < std && sizeY < std)
	{
		lineStrong = 1;
	}
	else
	{
		int temp1 =(int)((double)sizeX / (double)std);
		int temp2 =(int)((double)sizeY / (double)std);
		if(temp1 > 2 || temp2 > 2)
		{
			lineStrong = temp1 > temp2?   temp1 : temp2;
		}
	}


	for(int i = 0; i < count; i++)
	{
		stX = (leadRect[i].left - wndRect.left) + marginX;
		stY = (leadRect[i].top - wndRect.top) + marginY;
		edX = stX + leadRect[i].Width();//leadRect[i].right - wndRect.left;
		edY = stY + leadRect[i].Height();//leadRect[i].bottom - wndRect.top;

		Im::Gra::Color(M_DEFAULT, milColor);
		for(int j = 1; j <= lineStrong; j++)
		{
			Im::Gra::Rect(M_DEFAULT, milDst, stX-j, stY-j, edX+j, edY+j);
		}
	}

	Im::Buf::GetColor(milDst, M_PACKED + M_BGR24, M_ALL_BANDS, userDst);


	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milDst);	
}


//huj 2013/12/20
int CProcMil::ProcAnyAngle_Img(void* milSrc, UCHAR** dst, int bdryCx, int bdryCy, int bdrySizeX, int bdrySizeY, double angle, int* retDstSizeX, int* retDstSizeY)
{
	int ret = TRUE;
	int bdryRoiArea = bdrySizeX * bdrySizeY;
	//UCHAR* imgBoundary = new UCHAR[bdryRoiArea];
	UCHAR* imgBoundary = g_pMManager->pem_new<UCHAR>(true, bdryRoiArea, (PCHAR)__FUNCTION__, __LINE__);
	GetClipBuff(milSrc, imgBoundary, bdryCx, bdryCy, bdrySizeX, bdrySizeY);
	SaveWorkImg(imgBoundary, bdrySizeX, bdrySizeY,_T("clip.bmp"));
	RotateImg_ipp2(imgBoundary, dst, bdrySizeX, bdrySizeY, angle, retDstSizeX, retDstSizeY);

	//delete imgBoundary;
	g_pMManager->pem_delete(imgBoundary, true);


	return ret;
}

int CProcMil::ProcAnyAngle_Img(UCHAR* userSrc, UCHAR** dst, int OrgSizeX, int OrgSizeY, int bdryCx, int bdryCy, int bdrySizeX, int bdrySizeY, double angle, int band, int* retDstSizeX, int* retDstSizeY)
{
	int ret = TRUE;
	int bdryRoiArea = bdrySizeX * bdrySizeY;
	//UCHAR* imgBoundary = new UCHAR[bdryRoiArea];
	UCHAR* imgBoundary = g_pMManager->pem_new<UCHAR>(true, bdryRoiArea, (PCHAR)__FUNCTION__, __LINE__);
	GetClipImage(userSrc, OrgSizeX, OrgSizeY, imgBoundary, bdryCx, bdryCy, bdrySizeX, bdrySizeY, band);
	RotateImg_ipp2(imgBoundary, dst, bdrySizeX, bdrySizeY, angle, retDstSizeX, retDstSizeY, band);
	
	//delete imgBoundary;
	g_pMManager->pem_delete(imgBoundary, true);
	imgBoundary = NULL;
	return ret;
}

int CProcMil::ProcAnyAngle_Img(UCHAR* userSrc, UCHAR** dst, int sizeX, int sizeY, double angle, int band, int* retDstSizeX, int* retDstSizeY)
{
	int ret = TRUE;
	RotateImg_ipp2(userSrc, dst, sizeX, sizeY, angle, retDstSizeX, retDstSizeY, band);
	return ret;
}

//shk 2014/02/04
void CProcMil::GetBinaryImg_ipp(UCHAR* src, int orgSizeX,int orgSizeY, UCHAR* dst, int thresLT,int LTvalue,int thresGT,int GTvalue)
{
	IppiSize srcSize = {orgSizeX, orgSizeY};
	ippiThreshold_LTValGTVal_8u_C1R(src,orgSizeX,dst,orgSizeX,srcSize,thresLT,LTvalue,thresGT,GTvalue);

}



int CProcMil::MakeRoughGrayImg(float* src, UCHAR* dst, UCHAR* binDst, int sizeX, int sizeY, float countThreshold)
{
	int ret = 0;

	if(src == NULL || dst == NULL || sizeX < 0 || sizeY < 0)
		return ret;

	float hMax = 0;
	float hMin = 0;
	UCHAR* temp = NULL;
	//temp = new UCHAR[sizeX * sizeY];
	temp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, (sizeof(UCHAR) * sizeX * sizeY));


	UCHAR* binTemp = NULL;
	UCHAR* binDstTemp = NULL;
	if(binDst != NULL)
	{		
		//binTemp = new UCHAR[sizeX * sizeY];
		binTemp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
		memset(binTemp, 0, (sizeof(UCHAR) * sizeX * sizeY));
		binDstTemp = binTemp;
	}

	float stdMin = 0;
	float stdMax = 0;
	float avr = GetZmapDataAvr(src, sizeX, sizeY);
	GetHlimit(src, sizeX, sizeY, &hMax, &hMin);

	float val = 0;
	val = GetZmapDataAvr(src, sizeX, sizeY, hMin, avr);
	stdMin = val;
	val =  GetZmapDataAvr(src, sizeX, sizeY, avr, hMax);
	stdMax = hMax;//val;

	float* srcTemp = NULL;
	UCHAR* dstTemp = NULL;	
	srcTemp = src;
	dstTemp = temp;	

	float data = 0;
	UCHAR rst = 0;
	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{	
			data = *srcTemp++;
			if(data >= stdMin && data <= stdMax)
				rst = (UCHAR)(((data - stdMin) / (stdMax - stdMin)) * 255) / 1;
			else if(data < stdMin)
				rst = 0;
			else if(data > stdMax)
				rst = 255;
			
			*dstTemp++ = rst;

			if(binDst != NULL)
			{
				if(rst > countThreshold)
				{		
					*binDstTemp++ = 255;
					ret++;
				}
				else
				{
					*binDstTemp++ = 0;
				}
			}
		}
	}

	if(dst != NULL)
		memcpy_s(dst, (sizeof(UCHAR) * sizeX * sizeY), temp, (sizeof(UCHAR) * sizeX * sizeY));


	//delete temp;
	g_pMManager->pem_delete(temp, false);



	if(binDst != NULL)
	{		
		memcpy_s(binDst, (sizeof(UCHAR) * sizeX * sizeY), binTemp, (sizeof(UCHAR) * sizeX * sizeY));
		//delete binTemp;
		g_pMManager->pem_delete(binTemp, false);
	}

	return ret;
}


void CProcMil::GetHlimit(float* src, int sizeX, int sizeY, float* retHmax, float* retHmin)
{
	if(src == NULL || sizeX < 0 || sizeY < 0)
		return;

	float hMax = 0;
	float hMin = 0;
	float temp = 0;

	hMax = src[0];
	hMin = src[0];

	float* srcTemp = NULL;
	srcTemp = src;
	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{
			temp = *srcTemp++;
			if(temp > -10000)
			{	
				if(temp > hMax)
				{
					hMax = temp;
				}
				else if(temp < hMin)
				{
					hMin = temp;
				}
			}		
		}
	}

	*retHmax = hMax;
	*retHmin = hMin;
}



float CProcMil::GetZmapDataAvr(float* src, int sizeX, int sizeY, float noData)
{
	if(src == NULL || sizeX < 0 || sizeY < 0)
		return 1;

	int cnts = 0;
	double sum = 0;
	float avr = 0;

	for(int i = 0; i < sizeX * sizeY; i++)
	{
		if(noData >= 0)
		{
			if(noData != src[i])
			{
				sum += src[i];
				cnts++;
			}
		}
		else
		{
			sum += src[i];
			cnts++;
		}
	}

	avr = (float)(sum / cnts);

	return avr;
}

float CProcMil::GetZmapDataAvr(float* src, int sizeX, int sizeY, float minRange, float maxRange)
{
	if(src == NULL || sizeX < 0 || sizeY < 0)
		return 1;

	int cnts = 0;
	float sum = 0;
	float avr = 0;

	for(int i = 0; i < sizeX * sizeY; i++)
	{
		if(src[i] >= minRange && src[i] <= maxRange)
		{
			sum += src[i];
			cnts++;
		}
	}

	avr = sum / cnts;

	return avr;
}

//shk 2014/04/21
void CProcMil::GetMinMaxIdx(int* src,int size,int* minIdx,int* maxIdx)
{
	int minVal = 0;
	int maxVal = 0;
	ippsMinMaxIndx_32s(src,size,&minVal,minIdx,&maxVal,maxIdx);
}

MIL_INT CProcMil::GetTypeRange( int nTypeRange, BOOL binvert)
{
	if (nTypeRange == eTypeRangeIn)
		return binvert == FALSE ? M_IN_RANGE : M_OUT_RANGE;
	else if (nTypeRange == eTypeRangeOut)
		return binvert == FALSE ? M_OUT_RANGE : M_IN_RANGE;
	else if (nTypeRange == eTypeRangeUpper)
		return binvert == FALSE ? M_GREATER_OR_EQUAL : M_LESS_OR_EQUAL;
	else if (nTypeRange == eTypeRangeLower)
		return binvert == FALSE ? M_LESS_OR_EQUAL : M_GREATER_OR_EQUAL;
	else
		return M_IN_RANGE;
}

int CProcMil::CalcBlob( UCHAR* userBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob /*= FALSE*/, BOOL fillHole /*= FALSE*/, UCHAR* userDst /*= NULL*/ )
{

	MIL_INT count = 0;
	Im::PIL_ID milBinSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;


	milBinSrc = AllocBuff(sizeX, sizeY);
	milDst = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milBinSrc, userBinSrc);

	if(fillHole)
	{
		FillHoleBlob(milBinSrc, milBinSrc, m_milBlobResult);
	}

	if(eraseBorderBlob)
	{
		EraseBorderBlob(milBinSrc, milBinSrc, m_milBlobResult);
	}

	if(milDst != M_NULL)
		Im::Buf::Copy(milBinSrc,milDst);

	Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	Im::Blob::blobCalculate(milBinSrc, M_NULL, m_milBlobFeature, m_milBlobResult);

	//blob select
	if(minArea > 0)
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS, minArea, M_NULL);
	if(maxArea > 0)
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_GREATER , maxArea, M_NULL);


	//redraw blob image
	if(userDst != NULL)
	{
		Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
		Im::Blob::blobFill(m_milBlobResult, milDst, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw

		Im::Buf::Get(milDst, userDst);
		SaveWorkImg(milDst, _T("Blob_milDst.bmp"));
	}

	//get result
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);


	FreeMilImageBuff(&milBinSrc);
	FreeMilImageBuff(&milDst);

	return (int)count;

}

int CProcMil::CalcBlob( Im::PIL_ID milBinSrc, int minArea, int maxArea , BOOL eraseBorderBlob /*= FALSE*/, BOOL fillHole /*= FALSE*/, Im::PIL_ID milDst /*= NULL*/, int nFilter/* = 0*/)
{
	MIL_INT count = 0;
	CSize sz = GetImageSize(milBinSrc);
	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sz.cx, sz.cy);
	Im::Buf::Copy(milBinSrc, milSrc);
	if (fillHole)
	{
		FillHoleBlob(milSrc, milSrc, m_milBlobResult);
		//FillHoleBlob(milSrc, milSrc);
	}
	if(eraseBorderBlob)
		EraseBorderBlob(milSrc, milSrc, m_milBlobResult);
	if(nFilter >= 0)
	{
		MorOpen(milSrc, milSrc, nFilter);
		MorClose(milSrc, milSrc, nFilter);
	}
	
	if(milDst != M_NULL)
		Im::Buf::Copy(milSrc,milDst);

	Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	Im::Blob::blobCalculate(milSrc, M_NULL, m_milBlobFeature, m_milBlobResult);

	//blob select
	if(minArea > 0)
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS, minArea, M_NULL);
	if(maxArea > 0)
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_GREATER , maxArea, M_NULL);

	//redraw blob image
	if(milDst != M_NULL)
	{
		Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
//		Im::Blob::blobFill(m_milBlobResult, milDst, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw

	SaveWorkImg(milDst, _T("Blob_milDst.bmp"));
	}

	//get result
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);


	FreeMilImageBuff(&milSrc);
	return (int)count;
}
int CProcMil::CalcBlob_New(Im::PIL_ID milBinSrc, int minArea, int maxArea, BOOL eraseBorderBlob /*= FALSE*/, BOOL fillHole /*= FALSE*/, Im::PIL_ID milDst /*= NULL*/, int nFilter/* = 0*/)
{
	MIL_INT count = 0;

	if (fillHole)
		FillHoleBlob(milBinSrc, milBinSrc, m_milBlobResult);
	if (eraseBorderBlob)
		EraseBorderBlob(milBinSrc, milBinSrc, m_milBlobResult);
	if (nFilter >= 0)
	{
		MorOpen(milBinSrc, milBinSrc, nFilter);
		MorClose(milBinSrc, milBinSrc, nFilter);
	}

	if (milDst != M_NULL)
		Im::Buf::Copy(milBinSrc, milDst);

	Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	Im::Blob::blobCalculate(milBinSrc, M_NULL, m_milBlobFeature, m_milBlobResult);

	//blob select
	if (minArea > 0)
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS, minArea, M_NULL);
	if (maxArea > 0)
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_GREATER, maxArea, M_NULL);

	//redraw blob image
	if (milDst != M_NULL)
	{
		Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
//		Im::Blob::blobFill(m_milBlobResult, milDst, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw

		SaveWorkImg(milDst, _T("Blob_milDst.bmp"));
	}

	//get result
	Im::Blob::blobGetNumber(m_milBlobResult, &count);

	return (int)count;
}

int CProcMil::InitMilAlgoBlob()
{
	if(m_milSys == M_NULL)
		return M_FAILURE;

	FreeMilAlgoBlob();

	Im::Blob::blobAllocFeatureList(*m_milSys, &m_milBlobFeature);   //Allocate blob feature list.
	Im::Blob::blobAllocResult(*m_milSys, &m_milBlobResult);         //Allocate blob result.

	//add feature list.
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_CENTER_OF_GRAVITY);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_X_MAX);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_X_MIN);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_Y_MAX);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_Y_MIN);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_AREA);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_FERET_MIN_DIAMETER);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_FERET_MIN_ANGLE);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_LENGTH);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BREADTH); 
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_LABEL_VALUE);

	return M_SUCCESS;
}

int CProcMil::GetBlobResult( CMilBlobResult* pBlobResult )
{
	if(!pBlobResult)
		return M_FAILURE;

	CMilBlobResult* pResultTemp = NULL;
	MIL_INT nCntBlob = 0;

	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
	if(nCntBlob <= (MIL_INT)0)
		return M_FAILURE;

	pResultTemp = new CMilBlobResult((int)nCntBlob);
	g_pMManager->pem_new_check(pResultTemp, (PCHAR)__FUNCTION__, __LINE__);
	//MIL_INT32 *pMilLabel = new MIL_INT32[(int)nCntBlob];
	MIL_INT32 *pMilLabel = g_pMManager->pem_new<MIL_INT32>(true, (int)nCntBlob, (PCHAR)__FUNCTION__, __LINE__);

	Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, pResultTemp->area);
	Im::Blob::blobGet_BOX(m_milBlobResult, pResultTemp->left,pResultTemp->top,pResultTemp->right,pResultTemp->bottom);	
// 	Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, pResultTemp->left);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, pResultTemp->right);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, pResultTemp->top);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, pResultTemp->bottom);
	Im::Blob::blobGet_Center(m_milBlobResult,pResultTemp->cx, pResultTemp->cy);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_X + M_TYPE_DOUBLE, pResultTemp->cx);	
// 	Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_Y + M_TYPE_DOUBLE, pResultTemp->cy);
	Im::Blob::blobGetResult(m_milBlobResult, M_FERET_MIN_DIAMETER + M_TYPE_DOUBLE, pResultTemp->diameter);
	Im::Blob::blobGetResult(m_milBlobResult, M_FERET_MIN_ANGLE + M_TYPE_DOUBLE, pResultTemp->angle);
	//Im::Blob::blobGetResult(m_milBlobResult, M_BREADTH + M_TYPE_DOUBLE, blobResult->width);
	//Im::Blob::blobGetResult(m_milBlobResult, M_LENGTH + M_TYPE_DOUBLE, blobResult->length);
	for(int i = 0; i < (int)nCntBlob; i++)
	{
		pResultTemp->width[i] = pResultTemp->right[i] - pResultTemp->left[i];
		pResultTemp->length[i] = pResultTemp->bottom[i] - pResultTemp->top[i];
	}
	Im::Blob::blobGetResult(m_milBlobResult, M_LABEL_VALUE + M_TYPE_MIL_INT32, pMilLabel);

	memcpy_s(pResultTemp->blobLabel,(sizeof(int)*nCntBlob),pMilLabel,(sizeof(MIL_INT32)*nCntBlob));
	//delete [] pMilLabel;
	g_pMManager->pem_delete(pMilLabel, true);

	pBlobResult->Copy(pResultTemp,0);
	//delete pResultTemp;
	g_pMManager->pem_delete(pResultTemp, false);

	return M_SUCCESS;
}

int CProcMil::SelectBlob_MaxArea(int blobCnt,  Im::PIL_ID milDst)
{
	double * area;
	//area = (double *)malloc(sizeof( double) * blobCnt);
	area = (double *)g_pMManager->pem_malloc(sizeof(double) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);

	Im::Blob::blobGetResult( m_milBlobResult, M_AREA, area);

	double max = area[0];
	int index = 0;
	for( int i = 1; i < blobCnt ; i ++)
	{
		if( max < area[i])
		{
			index = i;
			max = area[i];
		}
	}

	Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_NOT_EQUAL, max, max); 
	Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0); 

	if(area)	//free(area);
		g_pMManager->pem_free(area);

	return index;
}
int CProcMil::SelectBlob_MaxArea(int blobCnt, Im::PIL_ID milDst, int nMaxCnt)
{
	double * area = (double *)g_pMManager->pem_malloc(sizeof(double) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);
	MIL_INT32 * label = (MIL_INT32 *)g_pMManager->pem_malloc(sizeof(MIL_INT32) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);

	Im::Blob::blobGetResult(m_milBlobResult, M_LABEL_VALUE + M_TYPE_MIL_INT32, label);
	Im::Blob::blobGetResult(m_milBlobResult, M_AREA, area);
	jsl::Blob * pb = (jsl::Blob*)m_milBlobResult;
	std::vector<std::pair<double, int>> vLabelList;	//first:area, second:label

	for (int i = 0; i < blobCnt; i++)
		vLabelList.push_back(std::make_pair(area[i], i));

	std::sort(vLabelList.begin(), vLabelList.end());

	//int nCalcCnt = vLabelList.size() - nMaxCnt
	for (int i = 0; i < vLabelList.size() - nMaxCnt; i++)
	{
		int nLabelNum = vLabelList[i].second;
		//Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_LABEL_VALUE, M_EQUAL, label[nLabelNum], label[nLabelNum]);
		pb->RemoveLabel(label[nLabelNum]);
	}

	Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);

	if (area)
		g_pMManager->pem_free(area);
	if (label)
		g_pMManager->pem_free(label);
	vLabelList.clear();

	return nMaxCnt;
}

int CProcMil::SelectBlob_Center(int blobCnt,  Im::PIL_ID milDst, double center_x, double center_y)
{
	double * cx = NULL;
	double * cy = NULL;
	MIL_INT32 * label = NULL;
	double * left = NULL;
	double * right = NULL;
	double * top = NULL;
	double * bottom = NULL;
	/*cx = (double *)malloc(sizeof( double) * blobCnt);
	cy = (double *)malloc(sizeof( double) * blobCnt);
	label = (MIL_INT32 *)malloc(sizeof( MIL_INT32) * blobCnt);*/
	cx = (double *)g_pMManager->pem_malloc(sizeof(double) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);
	cy = (double *)g_pMManager->pem_malloc(sizeof(double) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);
	label = (MIL_INT32 *)g_pMManager->pem_malloc(sizeof(MIL_INT32) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);

	left = (double *)g_pMManager->pem_malloc(sizeof(double) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);
	right = (double *)g_pMManager->pem_malloc(sizeof(double) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);
	top = (double *)g_pMManager->pem_malloc(sizeof(double) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);
	bottom = (double *)g_pMManager->pem_malloc(sizeof(double) * blobCnt, (PCHAR)__FUNCTION__, __LINE__);

	Im::Blob::blobGet_Center(m_milBlobResult, cx, cy);
	Im::Blob::blobGet_BOX(m_milBlobResult, left, top, right, bottom);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_X + M_TYPE_DOUBLE, cx);	
// 	Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_Y + M_TYPE_DOUBLE, cy);
	Im::Blob::blobGetResult(m_milBlobResult, M_LABEL_VALUE + M_TYPE_MIL_INT32, label);

	double dist = 0;
	double min_dist = HUGE_VAL;
	int index = -1;
	int Rectindex = -1;
	std::vector<int> vRectindex;
	std::vector<double> vRectdiff;
	double min_Rectdist = HUGE_VAL;

	//shkim center점이 속해있는블랍이 아닌 거리상 가까운 노이즈를 추출하는 경우 예외처리
	for (int i = 0; i < blobCnt; i++)
	{
		if (center_x >= left[i] && center_x <= right[i]
			&& center_y >= top[i] && center_y <= bottom[i])
		{
			dist = pow(center_x - cx[i], 2) + pow(center_y - cy[i], 2);
			vRectindex.push_back(i);
			if (min_Rectdist > dist)
			{
				Rectindex = i;
				min_Rectdist = dist;
			}
		}
	}

	for( int i = 0; i < blobCnt ; i ++)
	{
		dist = pow(center_x - cx[i], 2) + pow(center_y - cy[i], 2);

		if( min_dist > dist)
		{
			index = i;
			min_dist = dist;
		}
	}
	if (vRectindex.size() > 0)
	{
		//for문 안에 index가 존재하는경우
		for (int i = 0; i < vRectindex.size(); i++)
		{
			if (vRectindex[i] == index)
			{

				Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_LABEL_VALUE, M_NOT_EQUAL, label[index], label[index]);
				Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);
				if (cx)	//free(cx);
					g_pMManager->pem_free(cx);
				if (cy)	//free(cy);
					g_pMManager->pem_free(cy);
				if (label)	//free(label);
					g_pMManager->pem_free(label);
				if (left)
					g_pMManager->pem_free(left);
				if (right)
					g_pMManager->pem_free(right);
				if (top)
					g_pMManager->pem_free(top);
				if (bottom)
					g_pMManager->pem_free(bottom);
				return index;
			}
		}
		//for문 안에 index가 존재하지않는 경우
		if (Rectindex >= 0)
		{
			Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_LABEL_VALUE, M_NOT_EQUAL, label[Rectindex], label[Rectindex]);
			Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);
		}
	}
	else if(index >= 0) //속한 블랍이 없는경우
	{
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_LABEL_VALUE, M_NOT_EQUAL, label[index], label[index]); 
		Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0); 
	}

	if(cx)	//free(cx);
		g_pMManager->pem_free(cx);
	if(cy)	//free(cy);
		g_pMManager->pem_free(cy);
	if(label)	//free(label);
		g_pMManager->pem_free(label);
	if (left)
		g_pMManager->pem_free(left);
	if (right)
		g_pMManager->pem_free(right);
	if (top)
		g_pMManager->pem_free(top);
	if (bottom)
		g_pMManager->pem_free(bottom);

	return index;
}

// ±âÁ¸ HeightMeanÀ» »ç¿ëÇÏÁö ¾Ê°í ÁöÁ¤ÇÑ ROI¿µ¿ªÀÇ Height Æò±ÕÀ» ±¸ÇÑ´Ù.
void CProcMil::CalcHeightMean(UCHAR * pUcImgBinary, float * pfImgSrc, CRect *rcBlob, double *dHeight, int nCntBlob, int nWidth)
{
	if(nCntBlob < 0)
		return;

	//CRect *rtLocal = new CRect[nCntBlob];
	CRect *rtLocal = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);

	for(int i=0; i<nCntBlob; i++)
		rtLocal[i] = rcBlob[i];

	float fSum = 0.f;
	int nSumCount = 0;
	// height avg°Ë»ç
	for(int i=0; i < nCntBlob; i++)
	{
		fSum = 0;
		nSumCount = 0;
		for(int j = rtLocal[i].top; j < rtLocal[i].bottom; j++)
		{
			for(int k = rtLocal[i].left; k < rtLocal[i].right; k++)
			{
				int nIdx = j*nWidth+k;
				if(pUcImgBinary[nIdx] == 255)
				{
					fSum += pfImgSrc[nIdx];
					nSumCount++;

				}
			}
		}

		if(nSumCount != 0)
			dHeight[i] = (double)(fSum / nSumCount);
		else
			dHeight[i] = 0.f;

	}

	//delete [] rtLocal;
	g_pMManager->pem_delete(rtLocal, true);
}

bool CProcMil::PtInPolygon(POINT pt, POINT *pts, int ptNum)
{
	POINT spt, ept;				// start, end point
	bool oddNodes = false;		// false: even(out), true: odd(in)

	for(int i=0 ; i<ptNum ; i++)
	{
		spt = pts[i];
		ept = pts[(i + 1) % ptNum];		// % prevent over index

		if(((spt.y <= pt.y) && (ept.y > pt.y))			// an upward crossing
			|| ((spt.y > pt.y) && (ept.y <=  pt.y)))	// a downward crossing
		{
			// pt.x is in left of edge
			oddNodes ^= ((float)pt.x < ((float)spt.x + ((float)(pt.y - spt.y) / (float)(ept.y - spt.y) * (float)(ept.x - spt.x))));
		}
	}

	return oddNodes;
}

// Æú¸®°ï ¿µ¿ª¿¡¼­ Èò»öÀÎ ¿µ¿ªÀÇ ³ôÀÌ Æò±ÕÀ» ±¸ÇØ ³õ´Â´Ù.
void CProcMil::CalcHeightMean_Polygon(int nWidth, int nHeight, UCHAR * pUcImgBinary, float * pfImgSrc, ForeignPolygonArea pForeignPolygonArea, double *dHeight)
{
	if(pForeignPolygonArea.m_nForeignPolygonCnt < 0)
		return;

	// FOV ¾ÈÀÇ ÀüÃ¼ Æú¸®°ï(Solder)
	for(int i = 0; i < pForeignPolygonArea.m_nForeignPolygonCnt; i++)
	{
		float fSum = 0.f;
		int nSumCount = 0;

		POINT ptDstPoint[20];
		for (int a = 0; a < 20; a++)
		{
			ptDstPoint[a].x = pForeignPolygonArea.m_stArrForeignPolygon[i].m_ptArrForeignPolygon[a].x;
			ptDstPoint[a].y = pForeignPolygonArea.m_stArrForeignPolygon[i].m_ptArrForeignPolygon[a].y;
		}

		for(int j = 0; j < nHeight; j++)
		{
			for(int k = 0; k < nWidth; k++)
			{
				POINT cur;
				cur.x = k;
				cur.y = j;

				if(PtInPolygon(cur, ptDstPoint, 20) == TRUE)
				{
					int nIdx = j*nWidth+k;
					if(pUcImgBinary[nIdx] == 255)	// Èò»ö ¿µ¿ª¸¸
					{
						fSum += pfImgSrc[nIdx];
						nSumCount++;
					}
				}
			}
		}

		if(nSumCount != 0)
			dHeight[i] = (double)(fSum / nSumCount);
		else
			dHeight[i] = 0.f;
	}
}
void CProcMil::GetPolygon(UCHAR * uArrDst, int nWidth, int nHeight, int nStartX, int nStartY, vector<vector<POINTF>>* vPolygons, vector<POINTF>* vPolyCenter)
{
	((jsl::Blob *)m_milBlobResult)->GetPolygon(uArrDst, nWidth, nHeight, nStartX, nStartY, vPolygons, vPolyCenter);
}
int CProcMil::CalcBlob_Select(UCHAR * uArrBinSrc, UCHAR * uArrDst, int nWidth, int nHeight, int minArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType, int nTeachX/*= -1*/, int nTeachY/*= -1*/, int nBlobType, double dArea, double dAreaWPix, double dAreaHPix, int nAreaCnt, bool bUseMultiMaxBlob, int nMaxCnt)
{
	Im::PIL_ID milBin = M_NULL;
	milBin = AllocBuff(nWidth, nHeight);
	Im::Buf::Put(milBin, uArrBinSrc);
	Im::PIL_ID milDst = M_NULL;
	milDst = AllocBuff(nWidth, nHeight);
	int nCntBlob = CalcBlob(milBin, minArea, 0, eraseBorderBlob, fillHole, milDst, nFilter);
	int nAllBlobRst = 0;
	CRect rcRect;
	if(nCntBlob > 0)
	{
		if(nSelectType == eSimilarArea)		// TeachX, TeachY¿¡ °¡±î¿î  Blob 1°³¸¸ Select
		{
			int nIndex = SelectBlob_Area(nCntBlob, milDst, dArea, dAreaWPix, dAreaHPix, nAreaCnt);
			nCntBlob = nIndex >= 0 ? 1 : 0;
		}
		else if (nSelectType == eSelectHiddenArea)
		{
			nCntBlob = SelectBlob_HiddenArea(nCntBlob, milDst, nTeachX, nTeachY);
		}
		else if(nCntBlob == 1 || nSelectType == eSelectMix)		// ¸ðµç Blob
		{
			if(nBlobType == 1)
				nAllBlobRst = GetBlobResult_Rect(nTeachX, nTeachY, &rcRect);
		}
		else 
		{
			if (nSelectType == eSelectBigger)			// ¸éÀûÀÌ °¡Àå Å« Blob 1°³¸¸ Selct
			{
				if (bUseMultiMaxBlob && nCntBlob > nMaxCnt)
				{
					SelectBlob_MaxArea(nCntBlob, milDst, nMaxCnt);
					nCntBlob = CalcBlob(milDst, minArea, 0, eraseBorderBlob, fillHole, milDst, nFilter);
				}
				else if (!bUseMultiMaxBlob)
				{
					SelectBlob_MaxArea(nCntBlob, milDst);
					nCntBlob = CalcBlob(milDst, minArea, 0, eraseBorderBlob, fillHole, milDst, nFilter);
					if (nCntBlob > 1 && SelectBlob_Center(nCntBlob, milDst, nWidth / 2., nHeight / 2.) >= 0)
						nCntBlob = 1;
				}
			}
			else if(nSelectType == eSelectCenter || nTeachX < 0)		// Áß½É À§Ä¡¿¡ °¡Àå °¡±î¿î Blob 1°³¸¸ Select
			{
				if(SelectBlob_Center(nCntBlob, milDst, nWidth / 2., nHeight / 2.) >= 0)
					nCntBlob = 1;
			}
			else if(nSelectType == eSelectPosition)		// TeachX, TeachY¿¡ °¡±î¿î  Blob 1°³¸¸ Select
			{
				if (SelectBlob_Center(nCntBlob, milDst, nTeachX, nTeachY) >= 0)
					nCntBlob = 1;
			}
		}
	}
	if(uArrDst)
	{
		Im::Buf::Get(milDst, uArrDst);
		if(nSelectType == eSelectMix && nBlobType == 1 && nAllBlobRst == M_SUCCESS)
		{
			for (int y = 0; y < nHeight; y++)
			{
				int nWidthIdx = y * nWidth;
				for (int x = 0; x < nWidth; x++)
				{
					int nIndex = nWidthIdx + x;
					if(x < rcRect.left || x > rcRect.right || y < rcRect.top || y > rcRect.bottom)
						uArrDst[nIndex] = 0;
				}
			}
		}
	}

	FreeMilImageBuff(&milBin);
	FreeMilImageBuff(&milDst);
	return nCntBlob;
}
int CProcMil::CalcBlob_Select_new(cv::Mat& Src, cv::Mat& Dst, int nWidth, int nHeight, int minArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType, int nTeachX/*= -1*/, int nTeachY/*= -1*/, int nBlobType, double dArea, double dAreaWPix, double dAreaHPix, int nAreaCnt, bool bUseMultiMaxBlob, int nMaxCnt)
{
	Im::PIL_ID milBin = M_NULL;
	Im::PIL_ID milDst = M_NULL;
	milBin = (Im::PIL_ID)&Src;
	milDst = (Im::PIL_ID)&Dst;
	int nCntBlob = CalcBlob_New(milBin, minArea, 0, eraseBorderBlob, fillHole, milDst, nFilter);
	int nAllBlobRst = 0;
	CRect rcRect;
	if (nCntBlob > 0)
	{
		if (nSelectType == eSimilarArea)		// TeachX, TeachY¿¡ °¡±î¿î  Blob 1°³¸¸ Select
		{
			int nIndex = SelectBlob_Area(nCntBlob, milDst, dArea, dAreaWPix, dAreaHPix, nAreaCnt);
			nCntBlob = nIndex >= 0 ? 1 : 0;
		}
		else if (nCntBlob == 1 || nSelectType == eSelectMix)		// ¸ðµç Blob
		{
			if (nBlobType == 1)
				nAllBlobRst = GetBlobResult_Rect(nTeachX, nTeachY, &rcRect);
		}
		else
		{
			if (nSelectType == eSelectBigger)			// ¸éÀûÀÌ °¡Àå Å« Blob 1°³¸¸ Selct
			{
				if (bUseMultiMaxBlob && nCntBlob > nMaxCnt)
				{
					SelectBlob_MaxArea(nCntBlob, milDst, nMaxCnt);
					nCntBlob = CalcBlob(milDst, minArea, 0, eraseBorderBlob, fillHole, milDst, nFilter);
				}
				else if (!bUseMultiMaxBlob)
				{
					SelectBlob_MaxArea(nCntBlob, milDst);
					nCntBlob = CalcBlob(milDst, minArea, 0, eraseBorderBlob, fillHole, milDst, nFilter);
					if (nCntBlob > 1 && SelectBlob_Center(nCntBlob, milDst, nWidth / 2., nHeight / 2.) >= 0)
						nCntBlob = 1;
				}
			}
			else if (nSelectType == eSelectCenter || nTeachX < 0)		// Áß½É À§Ä¡¿¡ °¡Àå °¡±î¿î Blob 1°³¸¸ Select
			{
				if (SelectBlob_Center(nCntBlob, milDst, nWidth / 2., nHeight / 2.) >= 0)
					nCntBlob = 1;
			}
			else if (nSelectType == eSelectPosition)		// TeachX, TeachY¿¡ °¡±î¿î  Blob 1°³¸¸ Select
			{
				if (SelectBlob_Center(nCntBlob, milDst, nTeachX, nTeachY) >= 0)
					nCntBlob = 1;
			}
		}
	}
	if (Dst.data != NULL)
	{
		if (nSelectType == eSelectMix && nBlobType == 1 && nAllBlobRst == M_SUCCESS)
		{
			for (int y = 0; y < nHeight; y++)
			{
				int nWidthIdx = y * nWidth;
				UCHAR* uArrDst = Dst.ptr(nWidthIdx);
				for (int x = 0; x < nWidth; x++)
				{
					int nIndex = nWidthIdx + x;
					if (x < rcRect.left || x > rcRect.right || y < rcRect.top || y > rcRect.bottom)
						uArrDst[nIndex] = 0;
				}
			}
		}
	}
	
	return nCntBlob;
}
#define ForeignAreaCnt	1000
int CProcMil::GetForeignBlobResult(double *dArea, double *dCx, double *dCy, CRect *rcRect)
{
	if(m_milBlobResult == M_NULL)
		return M_FAILURE;
	MIL_INT nCntBlob = 0;
	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
	if(nCntBlob <= (MIL_INT)0)
		return M_FAILURE;
	int nTotalCnt = (nCntBlob > ForeignAreaCnt) ? ForeignAreaCnt : nCntBlob;
	if(dArea)
	{
		if(dArea)
			*dArea = 0;
		double * area = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &area, nCntBlob);
		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);
		for(int i =0; i<nTotalCnt; i++)
			dArea[i] = area[i];
		Delete_1DArray(&area);
	}
	if(rcRect)
	{
		double * left = NULL;
		double * right = NULL;
		double * top = NULL;
		double * bottom = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &left, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &right, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &top, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &bottom, nCntBlob);
		Im::Blob::blobGet_BOX(m_milBlobResult, left,top,right,bottom);
		for(int i = 0; i < nTotalCnt; i++)
		{							
			rcRect[i].left = left[i];
			rcRect[i].top = top[i];
			rcRect[i].right = right[i];
			rcRect[i].bottom = bottom[i];
			if(dCx)
			{
				double dHalfWidth = (rcRect[i].right - rcRect[i].left) / 2;
				dCx[i] = rcRect[i].left + dHalfWidth;
			}
			if(dCy)
			{
				double dHalfHeight = (rcRect[i].bottom - rcRect[i].top) / 2;
				dCy[i] = rcRect[i].top + dHalfHeight;
			}
		}
		Delete_1DArray(&left);
		Delete_1DArray(&right);
		Delete_1DArray(&top);
		Delete_1DArray(&bottom);
	}
	return M_SUCCESS;
}

#define BigAreaCnt	3
int CProcMil::GetBlobResult(double *dArea, double *dCx, double *dCy, CRect *rcRect, bool bAllBlob)
{
	if(m_milBlobResult == M_NULL)
		return M_FAILURE;

	MIL_INT nCntBlob = 0;

	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
	if(nCntBlob <= (MIL_INT)0)
		return M_FAILURE;

	int nBigAreaCnt = 1;
	if(bAllBlob)
	{
		nBigAreaCnt = BigAreaCnt;
		if(nBigAreaCnt > nCntBlob)
			nBigAreaCnt = nCntBlob;
	}
	int * nArrBigAreaIndex = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &nArrBigAreaIndex, nCntBlob);
	for (int a = 0; a < nCntBlob; a++)
		nArrBigAreaIndex[a] = -1;
	double * area = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &area, nCntBlob);
	Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);

	if (nBigAreaCnt == 1)
	{
		double dBigArea = 0.0;
		for (int n = 0; n < nCntBlob; n++)
		{
			if (area[n] > dBigArea)
			{
				dBigArea = area[n];
				nArrBigAreaIndex[0] = n;
			}
		}
	}
	else
	{
		for (int a = 0; a < nCntBlob; a++)
		{
			double dValue = 0;
			int nIdx = -1;
			for (int b = 0; b < nCntBlob; b++)
			{
				bool bCheck = true;
				for (int c = 0; c < nCntBlob; c++)
				{
					if (nArrBigAreaIndex[c] == b)
					{
						bCheck = false;
						break;
					}
				}
				if (bCheck == false)
					continue;
				if (nIdx == -1 || (nIdx > -1 && dValue < area[b]))
				{
					dValue = area[b];
					nIdx = b;
				}
			}
			if (nIdx == -1) return 0;
			nArrBigAreaIndex[a] = nIdx;
		}
		for (int a = 0; a < nCntBlob; a++)
		{
			bool bReset = true;
			for (int b = 0; b < nCntBlob; b++)
			{
				if (nArrBigAreaIndex[b] == a)
				{
					bReset = false;
					break;
				}
			}
			if (bReset)
			{
				for (int b = 0; b < nCntBlob; b++)
					nArrBigAreaIndex[b] = b;
				break;
			}
		}
	}

	if (dArea)
	{
		double cx_sum = 0;
		for (int a = 0; a < nBigAreaCnt; a++)
		{
			for (int n = 0; n < nCntBlob; n++)
			{
				if (bAllBlob == false)
					cx_sum += area[n];
				else
				{
					if (nArrBigAreaIndex[a] == n)
						dArea[a] = area[n];
				}
			}
		}
		if (bAllBlob == false)
			*dArea = cx_sum;
	}
	Delete_1DArray(&area);
	
	if(dCx)
	{
		double * cx = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &cx, nCntBlob);
		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_X + M_TYPE_DOUBLE, cx);

		double cx_sum = 0;
		for (int a =0; a< nBigAreaCnt; a++)
		{
			for(int n = 0; n < nCntBlob; n++)
			{
				if(bAllBlob == false)
					cx_sum += cx[n];
				else
				{
					if(nArrBigAreaIndex[a] == n)
						dCx[a] = cx[n];
				}
			}
		}
		if(bAllBlob == false)
			*dCx = cx_sum / nCntBlob;
		Delete_1DArray(&cx);
	}

	if(dCy)
	{
		double * cy = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &cy, nCntBlob);
		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_Y + M_TYPE_DOUBLE, cy);

		double cy_sum = 0;
		for (int a =0; a< nBigAreaCnt; a++)
		{
			for(int n = 0; n < nCntBlob; n++)
			{
				if(bAllBlob == false)
					cy_sum += cy[n];
				else
				{
					if(nArrBigAreaIndex[a] == n)
						dCy[a] = cy[n];
				}
			}
		}
		if(bAllBlob == false)
			*dCy = cy_sum / nCntBlob;
		Delete_1DArray(&cy);
	}

	if(rcRect)
	{
		double * left = NULL;
		double * right = NULL;
		double * top = NULL;
		double * bottom = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &left, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &right, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &top, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &bottom, nCntBlob);

		Im::Blob::blobGet_BOX(m_milBlobResult, left,top,right,bottom);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, left);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, right);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, top);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, bottom);

		(*rcRect).left = left[0];
		(*rcRect).right = right[0];
		(*rcRect).top = top[0];
		(*rcRect).bottom = bottom[0];
		for (int a =0; a< nBigAreaCnt; a++)
		{
			for(int n = 0; n < nCntBlob; n++)
			{
				if(bAllBlob == false)
				{
					(*rcRect).left = std::min(left[n], (double)((*rcRect).left));
					(*rcRect).top = std::min(top[n], (double)((*rcRect).top));
					(*rcRect).right = std::max(right[n], (double)((*rcRect).right));
					(*rcRect).bottom = std::max(bottom[n], (double)((*rcRect).bottom));
				}
				else
				{
					if(nArrBigAreaIndex[a] == n)
					{
						rcRect[a].left = left[n];
						rcRect[a].top = top[n];
						rcRect[a].right = right[n];
						rcRect[a].bottom = bottom[n];
					}
				}
			}
		}
		Delete_1DArray(&left);
		Delete_1DArray(&right);
		Delete_1DArray(&top);
		Delete_1DArray(&bottom);
	}
	Delete_1DArray(&nArrBigAreaIndex);
	return M_SUCCESS;
}

bool desc(std::pair<int, int> a, std::pair<int, int> b) { return a.first > b.first; }



int CProcMil::GetBlobResult_Renewal(double *dArea, double *dCx, double *dCy, CRect *rcRect, bool bAllBlob)
{

	if (Im::_LibType == Im::eType::MIL)
		return GetBlobResult(dArea, dCx, dCy, rcRect, bAllBlob);

	if (m_milBlobResult == M_NULL)
		return M_FAILURE;
	MIL_INT nCntBlob = 0;
	Im::Blob::blobGetNumber(m_milBlobResult, &nCntBlob);
	if (nCntBlob <= (MIL_INT)0)
		return M_FAILURE;

	jsl::Blob * pb = (jsl::Blob*)m_milBlobResult;
	//Max 기준 최대 3개
	if (bAllBlob)
	{
		int nBigAreaCnt = 3;
		if (nBigAreaCnt > nCntBlob)
			nBigAreaCnt = nCntBlob;
		//Max Area 기준으로 정렬
		// First: Area , Second: index
		std::vector<std::pair<int, int>> vecIndexMaxArea;
		for (int i = 1; i <= pb->m_nBlobCount; i++)
		{
			//삭제된 Index는 제외
			if (pb->m_remove[i] != 0)
				continue;
			vecIndexMaxArea.emplace_back(std::make_pair(pb->m_area[i], i));
		}



		//Area 기준 내림차순 정렬
		std::sort(vecIndexMaxArea.begin(), vecIndexMaxArea.end(), desc);
		//할당
		for (size_t i = 0; i < nBigAreaCnt; i++)
		{
			int nIndex = vecIndexMaxArea[i].second;
			if (dArea)
				dArea[i] = (float)pb->m_area[nIndex];
			if (dCx)
				dCx[i] = (double)pb->m_cx[nIndex] / pb->m_area[nIndex];
			if (dCy)
				dCy[i] = (double)pb->m_cy[nIndex] / pb->m_area[nIndex];
			if (rcRect)
			{
				rcRect[i].left = pb->m_min_x[nIndex];
				rcRect[i].top = pb->m_min_y[nIndex];
				rcRect[i].right = pb->m_max_x[nIndex];
				rcRect[i].bottom = pb->m_max_y[nIndex];
			}
		}
	}

	else  //Blob 전체에 대한 결과
	{
		if (dArea)
		{
			double sum = 0;
			for (int i = 1; i <= pb->m_nBlobCount; i++)
			{
				if (pb->m_remove[i] != 0) continue;
				sum += (float)pb->m_area[i];
			}
			*dArea = sum  /* / nCntBlob*/;
		}
		if (dCx)
		{
			double sum = 0;
			for (int i = 1; i <= pb->m_nBlobCount; i++)
			{
				if (pb->m_remove[i] != 0) continue;
				sum += (double)pb->m_cx[i] / pb->m_area[i];
			}
			*dCx = sum / nCntBlob;
		}
		if (dCy)
		{
			double sum = 0;
			for (int i = 1; i <= pb->m_nBlobCount; i++)
			{
				if (pb->m_remove[i] != 0) continue;
				sum += (double)pb->m_cy[i] / pb->m_area[i];
			}
			*dCy = sum / nCntBlob;
		}
		if (rcRect)
		{
			bool bFirst = true;
			for (int i = 1; i <= pb->m_nBlobCount; i++)
			{
				if (pb->m_remove[i] != 0) continue;
				if (bFirst)
				{
					(*rcRect).left = pb->m_min_x[i];
					(*rcRect).top = pb->m_min_y[i];
					(*rcRect).right = pb->m_max_x[i];
					(*rcRect).bottom = pb->m_max_y[i];
					bFirst = false;
				}
				else
				{
					(*rcRect).left = MIN(pb->m_min_x[i], ((*rcRect).left));
					(*rcRect).top = MIN(pb->m_min_y[i], ((*rcRect).top));
					(*rcRect).right = MAX(pb->m_max_x[i], ((*rcRect).right));
					(*rcRect).bottom = MAX(pb->m_max_y[i], ((*rcRect).bottom));
				}
			}
		}
	}
	return M_SUCCESS;
}

int CProcMil::GetBlobResult_Area(std::vector<double> &vArea)
{
	if(m_milBlobResult == M_NULL)
		return M_FAILURE;

	MIL_INT nCntBlob = 0;

	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
	if(nCntBlob <= (MIL_INT)0)
		return M_FAILURE;

	vArea.resize(nCntBlob);
	Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, &vArea[0]);
	return M_SUCCESS;
}
Im::PIL_ID CProcMil::GetMilSrc( const WndAlgoImg &sWndAlgoImg, BOOL b2D/*=TRUE */,double angle )
{
	Im::PIL_ID milSrc = M_NULL;

	if(b2D)
	{
		if(sWndAlgoImg.m_nChannel == 3)
		{
			milSrc = AllocBuffColor(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);
			MIL_INT M_width = Im::Buf::Inquire(milSrc, M_SIZE_X, M_NULL);
			MIL_INT M_height = Im::Buf::Inquire(milSrc, M_SIZE_Y, M_NULL);
			Im::Buf::PutColor2d(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS,0,0, M_width, M_height, sWndAlgoImg.m_ucArrCV);
			
		}
		else
		{
			milSrc = AllocBuff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);
			Im::Buf::Put(milSrc, sWndAlgoImg.m_ucArr2D);
			
		}
		if((angle > 7 && angle < 30) || (angle<-7 && angle >-30)||(angle<353 && angle >330))
			return AllocNomalRotateBuff(milSrc,angle);
	}
	else
	{
		milSrc = AllocRotateBuff_Color(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR32 , M_ALL_BANDS, sWndAlgoImg.m_fArr3D);
	}

	return milSrc;
}
//KSH
Im::PIL_ID CProcMil::GetMilSrc( const WndAlgoImg &sWndAlgoImg, int nIndex, BOOL b2D,double angle)
{
	Im::PIL_ID milSrc = M_NULL;

	if(b2D)
	{
		milSrc = AllocBuff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);
		Im::Buf::Put(milSrc, sWndAlgoImg.m_ucArr2D_Mix[nIndex]);

		if((angle > 7 && angle < 30) || (angle<-7 && angle >-30)||(angle<353 && angle >330))
			return AllocNomalRotateBuff(milSrc,angle);
	}
	else
	{
		milSrc = AllocRotateBuff_Color(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR32 , M_ALL_BANDS, sWndAlgoImg.m_fArr3D);
	}


	return milSrc;
}

int CProcMil::Morphology(UCHAR* src, int sizeX, int sizeY, int nMorphologyMode, int nIteration, UCHAR* dest)
{
	if(src == NULL || dest == NULL || sizeX <= 0 || sizeY <= 0 )
		return 0;

	Im::PIL_ID milBinSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	milBinSrc = AllocBuff(sizeX, sizeY);
	milDst = AllocBuff(sizeX, sizeY);

	Im::Buf::Put(milBinSrc, src);

	switch(nMorphologyMode)
	{
	case 0:
		MorOpen(milBinSrc, milDst, nIteration);	
		break;
	case 1:
		MorClose(milBinSrc, milDst, nIteration);	
		break;
	case 2:
		MorDilate(milBinSrc, milDst, nIteration);	
		break;
	case 3:
		MorErode(milBinSrc, milDst, nIteration);	
		break;
	}
	
	Im::Buf::Get(milDst, dest);

	FreeMilImageBuff(&milBinSrc);
	FreeMilImageBuff(&milDst);

	return eMR_SUCCESS;
}
int CProcMil::FillHoleMode(UCHAR* src, int sizeX, int sizeY, int nFillHoleMode, int nIteration, UCHAR* dest)
{
	if(src == NULL || dest == NULL || sizeX <= 0 || sizeY <= 0 )
		return 0;

	Im::PIL_ID milBinSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	milBinSrc = AllocBuff(sizeX, sizeY);
	milDst = AllocBuff(sizeX, sizeY);

	Im::Buf::Put(milBinSrc, src);

	
	switch(nFillHoleMode)
	{
	case 0:
		MorErode(milBinSrc, milDst, nIteration);
		FillHoleBlob(milDst, milDst, m_milBlobResult);
		MorDilate(milDst, milDst, nIteration);	
		
		break;
	case 1:
		MorDilate(milBinSrc, milDst, nIteration);	
		FillHoleBlob(milDst, milDst, m_milBlobResult);
		
		MorErode(milDst, milDst, nIteration);
		break;	
	}
	
	SaveReleaseWorkImg(milBinSrc, _T("abv22.bmp"), _T("C:"));
	SaveReleaseWorkImg(milDst, _T("abv23.bmp"), _T("C:"));

	Im::Buf::Get(milDst, dest);

	SaveReleaseWorkImg(dest, sizeX, sizeY, _T("abv234.bmp"), _T("C:"));

	FreeMilImageBuff(&milBinSrc);
	FreeMilImageBuff(&milDst);

	return eMR_SUCCESS;
}

// find outline
int CProcMil::cvContour(UCHAR* src, int sizeX, int sizeY, POINTF* ptVector)
{
	if(src == NULL || sizeX <= 0 || sizeY <= 0 )
		return 0;

	cv::Mat bgr = cv::Mat(sizeY, sizeX, CV_8UC1);

	bgr.data = src;

	std::vector< std::vector<cv::Point> > contours;

	cv::findContours(bgr,  contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	cv::Mat mask = cv::Mat::zeros(sizeY, sizeX, CV_8UC1);
	
	drawContours(mask, contours, -1, cv::Scalar(255), 3);

	if(ptVector != NULL)
	{
		for(int cc = 0; cc < contours[0].size(); cc++)
		{
			ptVector[cc].x = contours[0][cc].x;
			ptVector[cc].y = contours[0][cc].y;
		}
	}
	
	//drawContours(mask, contours, 3, cv::Scalar(255), 3);

	//drawContours(mask, contours, 0, cv::Scalar(255), 3);

	//cv::imwrite(std::string("c:\\target3.bmp"), mask);

	return contours[0].size();
}

// add range
int CProcMil::Binarize(UCHAR* src, int sizeX, int sizeY, int threshmin, int threshmax, int nRangeMode, int mode, UCHAR* dest, int nChannel, BOOL bFillHole)
{
	if(src == NULL || dest == NULL || sizeX <= 0 || sizeY <= 0 )
		return 0;

	
	Im::PIL_ID milBinSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	milBinSrc = AllocBuff(sizeX, sizeY);
	milDst = AllocBuff(sizeX, sizeY);
	

	Im::Buf::Put(milBinSrc, src);
	
	Binarize(milBinSrc, milDst,nRangeMode, threshmin, threshmax, mode );

	if(bFillHole)
		FillHoleBlob(milDst, milDst, m_milBlobResult);

	Im::Buf::Get(milDst, dest);


	FreeMilImageBuff(&milBinSrc);
	FreeMilImageBuff(&milDst);

	return eMR_SUCCESS;
}

//LJH 2016.05.20 Image Combine
void CProcMil::BinarizeImagesCombine(UCHAR* srcImg[], UCHAR* &destImg, int nImgWidth, int nImgHeight, int nImageCount = 0, int nFiltering /*false*/, BOOL bTeach)
{
	Im::PIL_ID *milSrc;
	Im::PIL_ID milDest = M_NULL;
	//milSrc = new Im::PIL_ID[nImageCount];
	milSrc = g_pMManager->pem_new<Im::PIL_ID>(true, nImageCount, (PCHAR)__FUNCTION__, __LINE__);
	
	bool bIsNullImage = false;
	int nNormalImageNumber = 0;

	int nWidthStep = nImgWidth;
 	if(bTeach)
	{
	/*	int nWidthSize = ((int)nWidthStep % 4);
		if (nWidthSize != 0) nWidthStep +=  (4 - nWidthSize);*/
		nWidthStep = nCalcWidthStep(false, nImgWidth);
	}
// 	destImg = new UCHAR[nWidthStep * nImgHeight];
	
	//¿µ»óÀÌ Á¤»óÀûÀÎÁö Check
	for (int iLoopCount = 0; iLoopCount < nImageCount; ++iLoopCount)
	{
		if (srcImg[iLoopCount] == NULL)			bIsNullImage = true;
		else if (srcImg[iLoopCount] != NULL)	nNormalImageNumber = iLoopCount;
		//CString FileName;
		//FileName.Format(_T("OCR Source Image %d"), iLoopCount + 1);
		//SaveImage(srcImg[iLoopCount], nImgWidth, nImgHeight, 1, FileName);
	}

	//ºñÁ¤»ó ¿µ»óÀÌ ÀÖ´Â °æ¿ì Á¤»ó ÀÌ¹ÌÁö¸¦ Dest Image¿¡ ³Ñ°Ü¼­ Error ¹ß»ýÀ» ¾ø¾Ú
	if (true == bIsNullImage)
	{
		milDest = AllocBuff(nImgWidth, nImgHeight);
		Im::Buf::Put(milDest, srcImg[nNormalImageNumber]);
		Im::Buf::Get(milDest, destImg);
		SaveWorkImg(milDest, _T("milDest_Last.bmp"));
	}

	else if(false == bIsNullImage)
	{
		for (int iLoopCount = 0; iLoopCount < nImageCount; ++iLoopCount)
		{
			milSrc[iLoopCount] = M_NULL;
			milSrc[iLoopCount] = AllocBuff(nImgWidth, nImgHeight);
			Im::Buf::Put(milSrc[iLoopCount ], srcImg[iLoopCount]);
			SaveWorkImg(milSrc[iLoopCount], _T("OCR_Copy.bmp"));
		}
		milDest = AllocBuff(nImgWidth, nImgHeight);
		Im::Buf::Copy(milSrc[0], milDest);
		for(int iLoopCount = 1; iLoopCount < nImageCount; ++iLoopCount)	ArithImage(milSrc[iLoopCount], milDest, milDest, M_ADD);
		SaveWorkImg(milDest, _T("milDest.bmp"));

		if (FONT_THIN_FILTER == nFiltering)
		{
			Im::im::Erode(milDest, milDest, 1, M_BINARY);	SaveWorkImg(milDest, _T("milDest_E.bmp"));
			Im::im::Dilate(milDest, milDest, 1, M_BINARY);	SaveWorkImg(milDest, _T("milDest_ED.bmp"));
		}

		else if (FONT_THICK_FILTER == nFiltering)
		{
			Im::im::Dilate(milDest, milDest, 1, M_BINARY);	SaveWorkImg(milDest, _T("milDest_D.bmp"));
			Im::im::Erode(milDest, milDest, 1, M_BINARY);	SaveWorkImg(milDest, _T("milDest_DE.bmp"));
		}

		Im::Buf::Get(milDest, destImg);
		SaveWorkImg(milDest, _T("milDest_Last.bmp"));
	}

	for (int iLoopCount = 0; iLoopCount < nImageCount; ++iLoopCount)	
		FreeMilImageBuff(&milSrc[iLoopCount]);
	//delete[] milSrc;
	g_pMManager->pem_delete(milSrc, true);

	FreeMilImageBuff(&milDest);
}

int CProcMil::nCalcWidthStep(bool bUseWidthStep, int OrgImgWidth)       //NYJ 2020/10/19
{
	int nWidthStep = OrgImgWidth;     //widthStep : bytes per a line

	int nSizeX = OrgImgWidth % 4;

	if (bUseWidthStep)
	{
		if (nSizeX != 0)
			nWidthStep += 4 - nSizeX;

		return nWidthStep;   //4 바이트의 배수가 되도록 영상 너비 정렬 시, width 대신 widthStep 값 사용
	}
	else
		return nWidthStep;
}

//shkim
Im::PIL_ID CProcMil::AllocNomalRotateBuff(Im::PIL_ID milSrc, double angle)
{
	Im::PIL_ID milBuff = M_NULL;
	MIL_INT sizeX = Im::Buf::Inquire(milSrc, M_SIZE_X, M_NULL);
	MIL_INT sizeY = Im::Buf::Inquire(milSrc, M_SIZE_Y, M_NULL);

	angle = 360 - angle;
	if(angle>360)angle -=360; 
	double radian = angle * 3.141592654/180;

	MIL_INT roSizeX = abs(sizeX*cos(radian)) + abs(sizeY*sin(radian));
	MIL_INT roSizeY = abs(sizeX*sin(radian)) + abs(sizeY*cos(radian));

	Im::Buf::Alloc2d(*m_milSys, (MIL_INT)roSizeX, (MIL_INT)roSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &milBuff);

	GetRotateImg(milSrc, milBuff, angle);

	return milBuff;

}
int CProcMil::GetnChannel()
{
	return 0;
}
void CProcMil::SetnChannel(int nChannel)
{

}
void CProcMil::GetnStep(int step)
{

}
void CProcMil::GetClipModelImg(cv::Mat ClipModelImg)
{

}

double CProcMil::SearchFocus(UCHAR* userSrc, int width, int height, int nAlgo = 0, int nChannel = 1)
{
	Im::PIL_ID milTemp = M_NULL;

	double dMatching = 0.f;

	cv::Mat srcImg;
	srcImg = cv::Mat::zeros(height, width, CV_8UC1);

	if(nChannel == 3)
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*(width*3)], width*3);
	}
	else
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*width], width);
	}

//#ifdef _PROC_IMG_SAVE
//	CString sSrcName;
//	sSrcName.Format(_T("SearchPatternsrc_%.0f.bmp"), m_milPatModel->getImageAngle());
//	SaveWorkImg(srcImg, sSrcName);
//#endif

	GeoMatch gm;

	cv::Mat roi  = srcImg(cv::Rect(512-20, 384-20, 40, 40));

	if(nAlgo == 1)
		dMatching = gm.grad_1stOrder_focus_algo_2(srcImg, 0.2);
	else if(nAlgo == 2)
		dMatching = gm.grad_2ndOrder_focus_algo_2(srcImg, 0.2);
	else
		dMatching = gm.grad_1stOrder_focus_algo_1(srcImg);
	
	

//#ifdef _PROC_IMG_SAVE
//	CString sName, sName1;
//	sName.Format(_T("SearchPatterndraw_%.0f.bmp"), m_milPatModel->getImageAngle());
//	sName1.Format(_T("SearchPatterndraw1_%.0f.bmp"), m_milPatModel->getImageAngle());
//	cv::Mat draw, draw1;
//	srcImg.copyTo(draw);
//	draw1 = cv::Mat::zeros(srcImg.rows, srcImg.cols, srcImg.type());
//	gm.DrawContours(draw, *m_milPatModel, *m_milPatResult, cv::Scalar(255), 1);
//	gm.DrawContours(draw1, *m_milPatModel, *m_milPatResult, cv::Scalar(255), 1);
//	SaveWorkImg(draw, sName);
//	SaveWorkImg(draw1, sName1);
//#endif
	srcImg.release();


	return dMatching;
}
#include <dvec.h>
bool CProcMil::InspSolderBall(cv::Mat src, std::vector<CRect> & rcBlob, int* nMaxIndex, float* ptrResult, double* ptrArrRst)
{
	bool bReturn = true;

	double MaxScore(0);
	int MaxIndex(0);
	int szImgW = src.cols;
	int szImgL = src.rows;
	int nBlobCnt = rcBlob.size();
	int nWidth = (szImgW >= szImgL) ? szImgW : szImgL;

	for (int i = 0; i < nBlobCnt; i++)
	{
		cv::Rect rc(rcBlob[i].left, rcBlob[i].top, rcBlob[i].Width(), rcBlob[i].Height());

		rc.x = rc.x < 0 ? 0 : rc.x;
		rc.y = rc.y < 0 ? 0 : rc.y;
		if ((rc.x + rc.width) > szImgW)
			rc.width -= (rc.x + rc.width) - szImgW;
		if ((rc.y + rc.height) > szImgL)
			rc.height -= (rc.y + rc.height) - szImgL;

		cv::Mat clipROI = src(rc).clone();
		cv::Mat FullROI = cv::Mat::zeros(nWidth, nWidth, CV_8UC1);
		UCHAR* pucClipROI = clipROI.data;
		UCHAR* pucFullROI = FullROI.data;
		int nClipWidth = clipROI.cols;
		int nClipHeight = clipROI.rows;
		CPoint ptClipCen(nClipWidth / 2, nClipHeight / 2);
		CPoint ptFullCen(nWidth / 2, nWidth / 2);
		int dX = ptFullCen.x - ptClipCen.x;
		int dY = ptFullCen.y - ptClipCen.y;
		if (dX < 0) dX = 0;
		if (dY < 0) dY = 0;
		int nR = dX + nClipWidth;
		int nB = dY + nClipHeight;
		if (nR >= nWidth) nR = nWidth - 1;
		if (nB >= nWidth) nB = nWidth - 1;
		int nWndIDX = 0;
		for (int m = dY; m < nB; m++)
		{
			for (int n = dX; n < nR; n++)
			{
				int nIndex = (m * nWidth) + n;
				pucFullROI[nIndex] = pucClipROI[nWndIDX];
				nWndIDX++;
			}
		}
		double dErr(1.);

		cv::Mat Ecircle = FullROI.clone();

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		cv::findContours(Ecircle, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
		if (contours.size() > 0)
		{
			std::vector<cv::Point> selectedCon;
			for (int cCnt = 0; cCnt < contours.size(); cCnt++)
			{
				if (contours[cCnt].size() < 6)
					continue;

				if (contours[cCnt].size() < selectedCon.size())
					continue;

				selectedCon = contours[cCnt];
			}

			if (selectedCon.size() > 6)
			{
				double radi(0.0), dCirle_x(0.0), dCirle_y(0.0);

				cv::Rect rcBounding = cv::boundingRect(selectedCon);
				double dCircleRatio = (double)rcBounding.width / rcBounding.height;
				double dCircleTol = 2.0;
				bool bCircleSimilar = 1.0 / dCircleTol < dCircleRatio && dCircleRatio < dCircleTol;
				if (bCircleSimilar == true)
				GetSolderBallPos(selectedCon, &dCirle_x, &dCirle_y, &radi, &dErr);

			}
		}
		else
			bReturn = false;

		dErr = 1. - dErr;
		if (dErr < 0)
			dErr = 0.0;
		if (MaxScore < dErr)
		{
			MaxIndex = i;
			MaxScore = dErr;
		}
		if (ptrArrRst != NULL)
			ptrArrRst[i] = dErr;
	}

	if (nMaxIndex != NULL)
		*nMaxIndex = MaxIndex;
	if (ptrResult != NULL)
		*ptrResult = MaxScore;

	return bReturn;
}
void CProcMil::GetSolderBallPos(std::vector<cv::Point>&  Points, double *dCX, double *dCY, double *dRadius, double *dErr)
{
	int nBorderCnt = Points.size();

	double theta;
	double s_cos = 0, s_sin = 0;
	double xy = 0, x = 0, y = 0;
	int cnt = nBorderCnt;

	double err2 = 0;

	double ds_Xn2 = 0, ds_Yn2 = 0, ds_XnYn = 0, ds_Xn = 0, ds_Yn = 0;
	double dsub1 = 0, dsub2 = 0, dsub3 = 0;

	for (int i = 0; i < nBorderCnt; i++)
	{
		double x, y;
		x = Points[i].x;
		y = Points[i].y;

		ds_Xn2 += x * x;
		ds_Yn2 += y * y;
		ds_XnYn += x * y;
		ds_Xn += x;
		ds_Yn += y;

		dsub1 += x * (x*x + y * y);
		dsub2 += y * (x*x + y * y);
		dsub3 += x * x + y * y;
	}

	int matrixSize = 3;
	int matrixSize2 = matrixSize * matrixSize;

	double* org_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);
	double* inv_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);
	double* sub_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize, (PCHAR)__FUNCTION__, __LINE__);

	org_m[0] = ds_Xn2; org_m[1] = ds_XnYn; org_m[2] = ds_Xn;
	org_m[3] = ds_XnYn; org_m[4] = ds_Yn2; org_m[5] = ds_Yn;
	org_m[6] = ds_Xn; org_m[7] = ds_Yn; org_m[8] = cnt;

	sub_m[0] = dsub1; sub_m[1] = dsub2; sub_m[2] = dsub3;

	cv::Mat org(3, 3, CV_64FC1, org_m);
	cv::Mat sub(3, 1, CV_64FC1, sub_m);
	cv::Mat dst = org.inv() * sub;

	*dCX = (float)dst.at<double>(0, 0) / 2;
	*dCY = (float)dst.at<double>(1, 0) / 2;
	*dRadius = (float)((*dCX**dCX) + (*dCY**dCY) + dst.at<double>(2, 0));
	*dRadius = sqrt(*dRadius);

	if (*dRadius < 0) *dRadius = -*dRadius;


	int m_borderCnt = 0;
	for (int i = 0; i < nBorderCnt; i++)
	{
		m_borderCnt++;
		cv::Point pt = Points[i];

		double dist = sqrt(SQR(pt.x - *dCX) + SQR(pt.y - *dCY));
		double distdeviate = fabs(dist - *dRadius) / *dRadius;

		if (distdeviate > 0.07)
			err2++;
	}
	*dErr = err2 / nBorderCnt;

	if (org_m)
	{
		g_pMManager->pem_free(org_m);
		org_m = NULL;
	}

	if (inv_m)
	{
		g_pMManager->pem_free(inv_m);
		inv_m = NULL;
	}

	if (sub_m)
	{
		g_pMManager->pem_free(sub_m);
		sub_m = NULL;
	}

	return;
}

int CProcMil::GetBlobResult_Rect(double dTeachW, double dTeachH, CRect *rcRect)
{
	if(m_milBlobResult == M_NULL)
		return M_FAILURE;

	MIL_INT nCntBlob = 0;

	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
	if(nCntBlob <= (MIL_INT)0)
		return M_FAILURE;

	if(rcRect)
	{
		double * left = NULL;
		double * right = NULL;
		double * top = NULL;
		double * bottom = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &left, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &right, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &top, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &bottom, nCntBlob);

		Im::Blob::blobGet_BOX(m_milBlobResult, left,top,right,bottom);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, left);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, right);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, top);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, bottom);

		int nIndex = 0;
		double dWidthBuf = 0;
		double dHeightBuf = 0;
		for(int n = 0; n < nCntBlob; n++)
		{
			double dRcW = right[n] - left[n];
			double dRcH = bottom[n] - top[n];
			if(dRcW <= 0 || dRcH <= 0)
				continue;

			if(dWidthBuf == 0 || dHeightBuf == 0)
			{
				dWidthBuf = dRcW;
				dHeightBuf = dRcH;
				rcRect->left = left[n];
				rcRect->top = top[n];
				rcRect->right = right[n];
				rcRect->bottom = bottom[n];
			}
			else
			{
				double dGapW = dTeachW - dWidthBuf;
				double dGapH = dTeachH- dHeightBuf;
				if(dGapW < 0)	dGapW *= -1;
				if(dGapH < 0)	dGapH *= -1;

				double dGapW2 = dTeachW - dRcW;
				double dGapH2 = dTeachH - dRcH;
				if(dGapW2 < 0)	dGapW2 *= -1;
				if(dGapH2 < 0)	dGapH2 *= -1;
				if(dGapW > dGapW2 && dGapH > dGapH2)
				{
					dWidthBuf = dRcW;
					dHeightBuf = dRcH;
					rcRect->left = left[n];
					rcRect->top = top[n];
					rcRect->right = right[n];
					rcRect->bottom = bottom[n];
				}
			}
		}
		Delete_1DArray(&left);
		Delete_1DArray(&right);
		Delete_1DArray(&top);
		Delete_1DArray(&bottom);
	}

	return M_SUCCESS;
}

int CProcMil::GetBlobResult_RectsV(std::vector<CRect> & rcRect)
{
	if(m_milBlobResult == M_NULL)
		return M_FAILURE;

	MIL_INT nCntBlob = 0;

	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
	if(nCntBlob <= (MIL_INT)0)
		return M_FAILURE;

	rcRect.resize(nCntBlob);


	double * left = NULL;
	double * right = NULL;
	double * top = NULL;
	double * bottom = NULL;
	//double * area = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &left, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &right, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &top, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &bottom, nCntBlob);
	//Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &area, nCntBlob);

	Im::Blob::blobGet_BOX(m_milBlobResult, left, top, right, bottom);
	//MblobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, left);
	//MblobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, right);
	//MblobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, top);
	//MblobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, bottom);
	//MblobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);


	for (int i = 0; i<nCntBlob; i++)
	{
		rcRect[i] = CRect(left[i], top[i], right[i] + 1, bottom[i] + 1);
	}


	Delete_1DArray(&left);
	Delete_1DArray(&right);
	Delete_1DArray(&top);
	Delete_1DArray(&bottom);
	//Delete_1DArray(&area);

	return M_SUCCESS;
}

int CProcMil::GetBlobResult_Rects(RECT *rcRect, int nRectCnt)
{
	if(m_milBlobResult == M_NULL)
		return M_FAILURE;

	MIL_INT nCntBlob = 0;
	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
	if(nCntBlob <= (MIL_INT)0)
		return M_FAILURE;


	bool bSort(false);
	if(nCntBlob > nRectCnt)
		bSort = true;


	double * left = NULL;
	double * right = NULL;
	double * top = NULL;
	double * bottom = NULL;
	//double * area = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &left, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &right, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &top, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &bottom, nCntBlob);
	//Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &area, nCntBlob);
	Im::Blob::blobGet_BOX(m_milBlobResult, left,top,right,bottom);
	//MblobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, left);
	//MblobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, right);
	//MblobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, top);
	//MblobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, bottom);
	//MblobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);

	if(bSort)
	{
		for (int i=0; i<nRectCnt; i++)
		{
			rcRect[i] = CRect(left[i], top[i], right[i]+1, bottom[i]+1);
		}
	}
	else
	{
		for (int i=0; i<nCntBlob; i++)
		{
			rcRect[i] = CRect(left[i], top[i], right[i]+1, bottom[i]+1);
		}
	}

	Delete_1DArray(&left);
	Delete_1DArray(&right);
	Delete_1DArray(&top);
	Delete_1DArray(&bottom);
	//Delete_1DArray(&area);

	return M_SUCCESS;
}

void CProcMil::GetRotateBound(int nImgW, int nImgH, double dAngle, int* ptrDstSizeX, int* ptrDstSizeY)
{
	if(dAngle == 0 || ptrDstSizeX == NULL || ptrDstSizeY == NULL)
		return;
	
	IppiSize srcSize = {nImgW, nImgH};
	IppiRect srcROI, dstROI;
	srcROI.x = srcROI.y = 0;
	srcROI.width = srcSize.width;
	srcROI.height = srcSize.height;
	double bound[2][2];
	double xShift = 0.0, yShift = 0.0;
	double coeffs[2][3] = { 0 };	//affine transform coefficients

	ippiGetRotateShift(srcROI.width/2, srcROI.height / 2, dAngle, &xShift, &yShift);
	ippiGetRotateTransform(dAngle, xShift, yShift, coeffs);
	ippiGetAffineBound(srcROI, bound, coeffs);

	/**ptrDstSizeX = (int)((int)(bound[1][0]+0.5) - (int)(bound[0][0]-0.5) + 1.0);
	*ptrDstSizeY = (int)((int)(bound[1][1]+0.5) - (int)(bound[0][1]-0.5) + 1.0);*/
	*ptrDstSizeX = (int)(bound[1][0] - bound[0][0] + 1.0);
	*ptrDstSizeY = (int)(bound[1][1] - bound[0][1] + 1.0);
}

bool CProcMil::RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (bColor)
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * 3 * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C3R(userSrc, orgSizeX * 3, *userDst, orgSizeX * 3, srcSize);
		}
		else
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C1R(userSrc, orgSizeX, *userDst, orgSizeX, srcSize);
		}

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	if (bColor)
		numChannels = 3;


	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp8u* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	if (bUseOrgSize == false)
	{
		//dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
		//dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);
		dstSize.width = (int)(bound[1][0]- bound[0][0]  + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}
	

	//Memory allocation for the intermediate images
	//*userDst = new uchar[dstSize.width * numChannels * dstSize.height];
	if (*userDst == NULL)
	*userDst = g_pMManager->pem_new<uchar>(true, dstSize.width * numChannels * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);
	Ipp8u* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;


	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing

		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}

bool CProcMil::RotateImg_ipp2020_2(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize, cv::Point cvPoCenter)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (bColor)
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * 3 * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C3R(userSrc, orgSizeX * 3, *userDst, orgSizeX * 3, srcSize);
		}
		else
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C1R(userSrc, orgSizeX, *userDst, orgSizeX, srcSize);
		}

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	if (bColor)
		numChannels = 3;


	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp8u* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	//ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	ippiGetRotateShift(cvPoCenter.x, cvPoCenter.y, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	if (bUseOrgSize == false)
	{
		/*dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
		dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);*/
		dstSize.width = (int)(bound[1][0] - bound[0][0] + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}

	//Memory allocation for the intermediate images
	//*userDst = new uchar[dstSize.width * numChannels * dstSize.height];
	if (*userDst == NULL)
	*userDst = g_pMManager->pem_new<uchar>(true, dstSize.width * numChannels * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);

	Ipp8u* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;


	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing

		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}

void CProcMil::RotateImg_IPP(UCHAR* ucSrcImg, int nSrcW, int nSrcH, double dAngle, UCHAR* ucDstImg, int nDstW, int nDstH)
{
	if (ucSrcImg == NULL || ucDstImg == NULL || nSrcW <= 0 || nSrcH <= 0 || nDstW <= 0 || nDstH <= 0)
		return;
	bool bLinear = true;
	bool bColor = true;

	IppiSize srcSize = { nSrcW, nSrcH };
	double angle = dAngle;
	if (dAngle == 0 || nSrcW == nDstW || nSrcH == nDstH)
	{
		if (ucDstImg != NULL)
			ippiCopy_8u_C1R(ucSrcImg, nSrcW, ucDstImg, nSrcW, srcSize);
		return;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp8u* pSrc = ucSrcImg;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { nDstW , nDstH };

	//변화된 크기 만큼
	//Memory allocation for the intermediate images
	Ipp8u* pDstRoi = ucDstImg;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;


	// Spec and init buffer sizes
	status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippLinear, direction, borderType,
		&specSize, &initSize);

	pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

	//Affine transform data initialization
	if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

	//Get work buffer size
	if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

	//allocate memory for work buffer
	pBuffer = ippsMalloc_8u(bufSize);

	//Affine transform processing
	if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
	
    //Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

}
int CProcMil::GetBlobLabel(long *pLebel, USHORT* LabelImage, int nWidth, int nHeight)
{
	if (pLebel == NULL || LabelImage == NULL)
		return 0;
	if (m_milBlobResult == NULL)
		return 0;
	Im::Blob::blobGetResult(m_milBlobResult, M_LABEL_VALUE + M_TYPE_LONG, pLebel);

	Im::PIL_ID MImageLabel;
	MImageLabel = M_NULL;
	Im::Buf::Alloc2d(*m_milSys, nWidth, nHeight, 16 + M_UNSIGNED, M_IMAGE + M_PROC, &MImageLabel);
	Im::Buf::Clear(MImageLabel, 0L);
	Im::Blob::blobLabel(m_milBlobResult, MImageLabel, M_CLEAR);
	
	Im::Buf::Get(MImageLabel, LabelImage);

	if (MImageLabel)Im::Buf::Free(MImageLabel); MImageLabel = M_NULL;
	return 1;
}
int CProcMil::SelectBlob_Area(int blobCnt,  Im::PIL_ID milDst, double dArea, double dAreaWPix, double dAreaHPix, int nAreaCnt)
{
	double * area = NULL;
	double * left = NULL;
	double * right = NULL;
	double * top = NULL;
	double * bottom = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &area, blobCnt);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &left, blobCnt);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &right, blobCnt);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &top, blobCnt);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &bottom, blobCnt);
	Im::Blob::blobGetResult( m_milBlobResult, M_AREA, area);
	Im::Blob::blobGet_BOX(m_milBlobResult, left,top,right,bottom);
	double maxA = dArea * 1000;
	double maxW = dArea * 1000;
	double maxH = dArea * 1000;
	double avgA = 0.0;
	double avgW = 0.0;
	double avgH = 0.0;
	int index = -1;
	int indexA = -1;
	int indexW = -1;
	int indexH = -1;
	for( int i = 0; i < blobCnt ; i ++)
	{
		double dRst = dArea - area[i];
		double dRstW = dAreaWPix - (right[i] - left[i]);
		double dRstH = dAreaHPix - (bottom[i] - top[i]);
		if (dRst < 0) dRst *= -1.0;
		if (dRstW < 0) dRstW *= -1.0;
		if (dRstH < 0) dRstH *= -1.0;
		if (dRst == 0) dRst = 0.5;
		if (dRstW == 0) dRstW = 0.5;
		if (dRstH == 0) dRstH = 0.5;
		if(maxA * maxW * maxH > dRst * dRstW * dRstH)
		{
			index = i;
			maxA = dRst;
			maxW = dRstW;
			maxH = dRstH;
			avgA = area[i] / dArea * 100.0;
			avgW = (right[i] - left[i]) / dAreaWPix * 100.0;
			avgH = (bottom[i] - top[i]) / dAreaHPix * 100.0;
		}
	}
	if (avgA <= (95 - (5 * nAreaCnt)) || avgA >= (105 + (5 * nAreaCnt)) ||
		avgW <= (95 - (5 * nAreaCnt)) || avgW >= (105 + (5 * nAreaCnt)) ||
		avgH <= (95 - (5 * nAreaCnt)) || avgH >= (105 + (5 * nAreaCnt)))
		index = -1;
	if (index >= 0)
	{
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_NOT_EQUAL, area[index], area[index]); 
		Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0); 
	}
	Delete_1DArray(&area);
	Delete_1DArray(&left);
	Delete_1DArray(&right);
	Delete_1DArray(&top);
	Delete_1DArray(&bottom);
	return index;
}
int CProcMil::SelectBlob_HiddenArea(int blobCnt, Im::PIL_ID milDst, int nTeachX, int nTeachY)
{
	if (blobCnt <= 0)
		return -1;
	if (milDst == 0) return -1;
	jsl::Blob* pb = (jsl::Blob*)m_milBlobResult;
	int* left = pb->m_min_x;
	int* right = pb->m_max_x;
	int* top = pb->m_min_y;
	int* bottom = pb->m_max_y;

		int index1 = -1;
		int index2 = -1;
		double dMinRate1 = 2;//shkim 4배 내에서 찾는다
		double dMinRate2 = 2;
		for (int i = 1; i <= pb->m_nBlobCount; i++)
		{
			if (pb->m_remove[i] != 0)
				continue;

			double dRstW = right[i] - left[i] + 1.0f;
			double dRstH = bottom[i] - top[i] + 1.0f;
			if (dRstW < 0) dRstW *= -1.0;
			if (dRstH < 0) dRstH *= -1.0;

			double dRateX = abs((nTeachX - dRstW) / nTeachX);
			double dRateY = abs((nTeachY - dRstH) / nTeachY);
			double dRate = dRateX * dRateY;
			if (dRate >= dMinRate2)
			{
				pb->m_remove[i] = 1;
				pb->m_nBlobRealCount -= 1;
			}
			else if (dRate < dMinRate1)
			{
				// 기존 2순위를 제거
				if (index2 >= 0)
				{
					pb->m_remove[index2] = 1;
					pb->m_nBlobRealCount -= 1;
				}
				// 기존 1순위를 2순위로 이동
				index2 = index1;
				dMinRate2 = dMinRate1;
				// 새로운 1순위 설정
				index1 = i;
				dMinRate1 = dRate;
			}
			else
			{
				// 기존 2순위를 제거
				if (index2 >= 0)
				{
					pb->m_remove[index2] = 1;
					pb->m_nBlobRealCount -= 1;
				}
				// 새로운 2순위 설정
				index2 = i;
				dMinRate2 = dRate;
			}
		}
		if (index1 >= 0 || index2 >= 0)
		{
			Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);
		}
	return pb->m_nBlobRealCount;
}
uchar* CProcMil::MatToBytes(cv::Mat image, uchar * bytes)
{
	int size = image.total() * image.elemSize();
	//uchar * bytes = new byte[size]; 
	std::memcpy(bytes,image.data,size * sizeof(byte));
	 //uchar * bytes =image.data;
	return bytes;
}
cv::Mat CProcMil::BytesToMat(UCHAR* src, int sizeX, int sizeY, int colortype)
{
	if(colortype == 0)
	{
		cv::Mat image = cv::Mat(sizeY,sizeX,CV_8UC3,src).clone(); // make a copy
		return image;
	}
	else if(colortype == 1)
	{
		cv::Mat image = cv::Mat(sizeY,sizeX,CV_8UC1,src).clone(); // make a copy
		return image;
	}
}
double CProcMil::ChangePoint(int nAlgoPoint, double dData)
{
	double dRst = dData;
	if (dData == 0) return dData;
	int nPoint = nAlgoPoint / 10;
	int nType = nAlgoPoint % 10;
	if (nPoint < 0) nPoint = 0;
	if (nPoint > 4) nPoint = 4;
	if (nType < 0 || nType >= m_eAlgoPoint_Total)
		nType = 0;
	double dPos = pow(10.0, (nPoint + 2.0));
	double dPos2 = dPos * 10.0;
	double dGap = 5.0 / dPos2;
	double dGap2 = 9.0 / dPos2;
	long long nValue = (long long)((dData + dGap) * dPos);
	if (nType == m_eAlgoPoint_RoundDown)
		nValue = (long long)((dData - dGap) * dPos);
	else if (nType == m_eAlgoPoint_Up)
		nValue = (long long)((dData + dGap2) * dPos);
	else if (nType == m_eAlgoPoint_Down)
		nValue = (long long)(dData * dPos);
	else if (nType == m_eAlgoPoint_Auto)
	{
		long long nValue2 = (long long)(dData * dPos2) % 10;
		if (nValue2 < 0) nValue2 *= -1;
		nValue = (long long)(dData * dPos);
		if (nValue2 >= 5) nValue += 1;
		else nValue -= 1;
	}
	dRst = (double)nValue / dPos;
	return dRst;
}

int CProcMil::GetBlobResult_ALL(double *dArea, double *dCx, double *dCy, CRect *rcRect)
{
	if(m_milBlobResult == M_NULL)
		return M_FAILURE;
	MIL_INT nCntBlob = 0;
	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);
	if(nCntBlob <= (MIL_INT)0)
		return M_FAILURE;
	if(dArea)
		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, dArea);
	if(dCx)
		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_X + M_TYPE_DOUBLE, dCx);
	if(dCy)
		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_Y + M_TYPE_DOUBLE, dCy);
	if(rcRect)
	{
		double * left = NULL;
		double * right = NULL;
		double * top = NULL;
		double * bottom = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &left, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &right, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &top, nCntBlob);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &bottom, nCntBlob);
		Im::Blob::blobGet_BOX(m_milBlobResult, left,top,right,bottom);
		for(int n = 0; n < nCntBlob; n++)
		{
			rcRect[n].left = left[n];
			rcRect[n].top = top[n];
			rcRect[n].right = right[n];
			rcRect[n].bottom = bottom[n];
		}
		Delete_1DArray(&left);
		Delete_1DArray(&right);
		Delete_1DArray(&top);
		Delete_1DArray(&bottom);
	}
	return M_SUCCESS;
}

int CProcMil::FOVImageCompose(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer, 
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage)
{
	double dRedValue = (double)nRedValue / 100.0;
	double dGreenValue = (double)nGreenValue / 100.0;
	double dBlueValue = (double)nBlueValue / 100.0;
	double dWhiteValue = (double)nWhiteValue / 100.0;

	int nValueBuf = 0;
	double dRedImageValue = 0;
	double dGreenImageValue = 0;
	double dBlueImageValue = 0;
	double dWhiteImageValue = 0;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;

	int nMaxValue = nFullImageSize / (nFullImageWidth*nFullImageHeight);
	if(nMaxValue <= 0)	nMaxValue = 1;

	for (int i=0; i<nMaxValue; i++)
	{
		for (int y = nStartY; y < nFullImageHeight; y++)
		{
			for (int x = nStartX; x < nFullImageWidth; x++)
			{
				if(y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				if (!ptrbyRedBuffer) 
					dRedImageValue = 0;
				else
					dRedImageValue = ptrbyRedBuffer[(y * (int)nFullImageWidth) + x];
				if (!ptrbyGreenBuffer)
					dGreenImageValue = 0;
				else
					dGreenImageValue = ptrbyGreenBuffer[(y * (int)nFullImageWidth) + x];
				if (!ptrbyBlueBuffer)
					dBlueImageValue = 0;
				else
					dBlueImageValue = ptrbyBlueBuffer[(y * (int)nFullImageWidth) + x];
				if (!ptrbyWhiteBuffer)
					dWhiteImageValue = 0;
				else
					dWhiteImageValue = ptrbyWhiteBuffer[(y * (int)nFullImageWidth) + x];

				nValueBuf = (int)(dRedValue * dRedImageValue) + (int)(dGreenValue * dGreenImageValue) + (int)(dBlueValue * dBlueImageValue) + (int)(dWhiteValue * dWhiteImageValue);

				if (nValueBuf > 255) 
					nValueBuf = 255;
				ptrbyResultImage[nROIIndex++] = nValueBuf;
			}
		}
	}

	return M_SUCCESS;
}
int CProcMil::GetBlobResult_Center(double *dCenterX, double *dCenterY)
{
	if(m_milBlobResult == M_NULL)
		return M_FAILURE;

	MIL_INT nCntBlob = 0;
	Im::Blob::blobGetNumber(m_milBlobResult,  &nCntBlob);

	if (nCntBlob < 0)
	{
		return M_FAILURE;
	}

	double * CenterX = NULL;
	double * CenterY = NULL;

	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &CenterX, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &CenterY, nCntBlob);

	Im::Blob::blobGet_Center(m_milBlobResult, CenterX, CenterY);

	for (int i=0; i<nCntBlob; i++)
	{
		dCenterX[i] = CenterX[i];
		dCenterY[i] = CenterY[i];
	}

	Delete_1DArray(&CenterX);
	Delete_1DArray(&CenterY);

	return M_SUCCESS;
}

void CProcMil::GetDrawContours(UCHAR* dst, int nSizeX, int nSizeY, int nStep, int nValue)

{
	if (m_milBlobResult == M_NULL)
		return;
	jsl::Blob * pb = (jsl::Blob*)m_milBlobResult;
	pb->DrawContours(dst, nSizeX, nSizeY, nStep, nValue);
}

int CProcMil::GetBlobPerimeter(UCHAR* ucDst, USHORT* usDst, int nSizeX, int nSizeY, int nStep)
{
	if (m_milBlobResult == M_NULL)
		return 0;

	if (ucDst == NULL || usDst == NULL)
		return 0;

	jsl::Blob * pb = (jsl::Blob*)m_milBlobResult;
	int nWidth = 0;
	pb->CalcPerimeter(ucDst, usDst, nSizeX, nSizeY, nStep, nWidth);

	return nWidth;
}
float CProcMil::GetPerimeterLength()
{
	if (m_milBlobResult == M_NULL)
		return 0;

	jsl::Blob * pb = (jsl::Blob*)m_milBlobResult;
	float fPerimeter = 0;

	for (int i = 1; i <= pb->m_nBlobCount; i++)
	{
		if (pb->m_remove[i] != 0) continue;
		
		fPerimeter += pb->m_perimeter[i];
	}

	return fPerimeter;
}
float CProcMil::SolderBall_Test(cv::Mat img_1, float dia, float stdscore, std::vector<CRect> & rcBlob)
{
	if (RounD(dia) == 1)
		return 1.0f;

	dia = RounD(dia) + 1;
	int patSize = dia;
	if (img_1.cols < patSize || img_1.rows < patSize)
		return 0.f;

	cv::Mat model1 = cv::Mat(patSize, patSize, CV_8UC1, cv::Scalar(0));
	cv::circle(model1, cv::Point(patSize / 2, patSize / 2), dia / 2, cv::Scalar(255), cv::FILLED);

#ifdef _DEBUG
#ifdef _INSP_IMG_SAVE
	CString sPath = WORK_IMAGE_PATH + CString(_T("\\SolderBall_Test_Model.bmp"));
	cv::imwrite(std::string(CT2A(sPath)), model1);
	sPath = WORK_IMAGE_PATH + CString(_T("\\SolderBall_Test_Img.bmp"));
	cv::imwrite(std::string(CT2A(sPath)), img_1);
#endif
#endif

	double MaxScore(0);
	float nModelPixelCnt = 1.0f / (float)(cv::countNonZero(model1));

	if (nModelPixelCnt <= 0)
		return 0.f;

	cv::Mat img = img_1 & 1;
	model1 &= 1;
	cv::Mat model;
	model1.convertTo(model, CV_32F, nModelPixelCnt);

	cv::Mat result = cv::Mat::zeros(img.rows, img.cols, CV_32FC1);

	int ofsX = model.cols / 2;
	int ofsY = model.rows / 2;
	int szW = model.cols;
	int szL = model.rows;
	int szImgW = img.cols;
	int szImgL = img.rows;
	for (int i = 0; i < rcBlob.size(); i++)
	{
		cv::Rect rc(rcBlob[i].left, rcBlob[i].top, rcBlob[i].Width(), rcBlob[i].Height());
		rc.x -= ofsX;
		rc.y -= ofsY;
		rc.width += szW;
		rc.width += szL;

		rc.x = rc.x < 0 ? 0 : rc.x;
		rc.y = rc.y < 0 ? 0 : rc.y;
		if ((rc.x + rc.width) > szImgW)
			rc.width -= (rc.x + rc.width) - szImgW;
		if ((rc.y + rc.height) > szImgL)
			rc.height -= (rc.y + rc.height) - szImgL;

		cv::Mat ResROI = result(rc);
		cv::filter2D(img(rc), ResROI, CV_32F, model, cv::Point(0, 0), 0, cv::BORDER_CONSTANT);

		double Score(0);
		cv::minMaxLoc(ResROI, nullptr, &Score);

		if (MaxScore < Score)
			MaxScore = Score;
	}

	return MaxScore;
}

bool CProcMil::PCA_1D(std::vector<cv::Point> pint, float* a, float* b)
{
	cv::Mat matrixInsp(pint.size(), 2, CV_32F);
	//cv::Mat MatrixStd(4,2,CV_32F);
	if (pint.size() < 3)
	{
		*a = 0.0f;
		*b = 0.0f;
		return false;
	}
	for (int i = 0; i < pint.size(); i++)
	{
		matrixInsp.ptr<float>(i)[0] = pint[i].x;		matrixInsp.ptr<float>(i)[1] = pint[i].y;
	}

	cv::Mat covar, comeam, eigenVal, eigenMt;
	cv::calcCovarMatrix(matrixInsp, covar, comeam, cv::COVAR_NORMAL | cv::COVAR_ROWS, CV_32F);
	float* tmp1 = covar.ptr<float>(0);
	covar /= (matrixInsp.rows - 1);
	cv::eigen(covar, eigenVal, eigenMt);	//주성분(그 방향으로 데이터들의 분산이 가장 큰 방향벡터) 분석
	float* tmpV = eigenVal.ptr<float>(0);
	float* tmpM = eigenMt.ptr<float>(0);
	float tmpA = tmpM[2];
	float tmpB = tmpM[3];
	float tmpx1 = matrixInsp.ptr<float>(0)[0];
	float tmpy1 = matrixInsp.ptr<float>(0)[1];

	*a = -tmpA / tmpB;
	*b = tmpA / tmpB * tmpx1 + tmpy1;
	if (tmpB == 0)
		return false;
	return true;
}
void CProcMil::OLS_1D(std::vector<cv::Point> vPoint, float* a, float* b)
{

	if (vPoint.size() < 3)
	{
		*a = 0.0f;
		*b = 0.0f;
		return;
	}
	cv::Mat matrixX(vPoint.size(), 2, CV_32F), matrixXT, matrixY(vPoint.size(), 1, CV_32F);


	for (int i = 0; i < vPoint.size(); i++)
	{
		float* fptr = matrixX.ptr<float>(i);
		float* fptry = matrixY.ptr<float>(i);

		fptr[0] = vPoint[i].x;
		fptr[1] = 1;

		fptry[0] = vPoint[i].y;
	}

	cv::Mat Xt = matrixX.t();
	cv::Mat XX = Xt * matrixX;
	cv::Mat XY = Xt * matrixY;
	cv::Mat res = XX.inv() * XY;;

#if _DEBUG
	float* XXptr = XX.ptr<float>(0);
	float* XTptr = Xt.ptr<float>(0);
	float* XYptr = XY.ptr<float>(0);
#endif

	float* Reptr = res.ptr<float>(0);
	*a = Reptr[0];
	*b = Reptr[1];
}