#pragma once
#include "StdAfx.h"
#include "ProcMil_Mount.h"
#include <math.h>
#include "GeoMatch_BodyBlob.h"
#include "MemoryManager.h"

#define  MOUNT_DEBUG		0

#define PAT_ACCURACY	M_HIGH
#define PAT_SPEED		M_LOW

#define MOUNT_BLOB_MIN_AREA			250   

enum noise_rocate
{
	eNOISE_SOLDER = 1,
	eNOISE_BODY,
};

void Log_MountModel(CString msg)
{
	//g_pMPTI->AddLog(msg);
}

CProcMil_Mount::CProcMil_Mount(void)
{
	m_className = _T("CProcMil_Mount");

	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;

//	m_milPatModel = M_NULL;
//	m_milPatResult = M_NULL;
//	m_milPatModel_Rotate = NULL;

	m_ModelMng = std::shared_ptr<CMModelManager<ModelFile_Pat>>(new CMModelManager<ModelFile_Pat>(eAlgoBody_Blob, _T(".ppd"), Log_MountModel));

}


CProcMil_Mount::~CProcMil_Mount(void)
{

}


int CProcMil_Mount::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys,bool bUseImagePilLib)
{
	if(milApp == M_NULL || milSys == M_NULL)
	{
		return eMNT_FAIL;	
	}

	CProcMil :: InitMil(milApp, milSys,bUseImagePilLib);

// 	Im::Blob::blobAllocFeatureList(*m_milSys, &m_milBlobFeature);   //Allocate blob feature list.
// 	Im::Blob::blobAllocResult(*m_milSys, &m_milBlobResult);         //Allocate blob result.
// 
// 	//add feature list.
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_CENTER_OF_GRAVITY);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_X_MAX);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_X_MIN);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_Y_MAX);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_Y_MIN);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_AREA);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_FERET_MIN_DIAMETER);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_FERET_MIN_ANGLE);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_LENGTH);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BREADTH); 
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_LABEL_VALUE);
	InitMilAlgoBlob();
	//CProcMil :: InitMil(milApp, milSys);

	/* Allocate result buffer. */
//	MpatAllocResult(*milSys, 1L, &m_milPatResult);

	return eMNT_SUCCESS;
}

int CProcMil_Mount::FreeMil()
{
#ifndef _DISABLE_MIL	
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


	//if(m_milPatResult != M_NULL)
	//{
	//	MpatFree(m_milPatResult);
	//	m_milPatResult = M_NULL;
	//}
	//DeleteModelBuf();
	FreeMilAlgoBlob();

	CProcMil :: FreeMil();	
#endif

	return eMNT_SUCCESS;
}

void CProcMil_Mount::Binarize(UCHAR* userSrc, int sizeX, int sizeY, UCHAR* userDst, int thresh, BOOL invert)
{	
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milDst = M_NULL;

	milSrc = AllocBuff(sizeX, sizeY);
	milDst = AllocBuff(sizeX, sizeY);

	Im::Buf::Put(milSrc, userSrc);

	CProcMil :: Binarize(milSrc, milDst, thresh, invert);

	Im::Buf::Get(milDst, userDst);

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milDst);
}


void CProcMil_Mount::HistoEqulaize(Im::PIL_ID milSrc, Im::PIL_ID milDst, double min, double max)
{
	MimHistogramEqualize(milSrc, milDst, M_UNIFORM , M_EXPONENTIAL, min, max) ;
}


void CProcMil_Mount::MaskTrueImg(void* milSrc, UCHAR * mask, UCHAR * dst, int cx, int cy, int sizeX, int sizeY)
{
	if( milSrc == M_NULL || mask == NULL || dst == NULL || sizeX < 0 || sizeY < 0) 
		return ;

	Im::PIL_ID tempMilBuf = M_NULL;
	tempMilBuf = AllocClipBuff((Im::PIL_ID)milSrc, cx, cy, sizeX, sizeY);

	//SaveWorkImg(tempMilBuf, _T("mount_clip.bmp"));
	//SaveWorkImg(mask, sizeX, sizeY, _T("mount_mask.bmp"));
	UCHAR* srcTemp = NULL;
	UCHAR* dstTemp = NULL;
	/*srcTemp = new UCHAR[sizeX * sizeY];
	dstTemp = new UCHAR[sizeX * sizeY];*/
	srcTemp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	dstTemp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(srcTemp, 0, (sizeof(UCHAR) * sizeX * sizeY));
	memset(dstTemp, 0, (sizeof(UCHAR) * sizeX * sizeY));

	Im::Buf::Get(tempMilBuf, srcTemp);

	int id = 0;
	for( int j = 0 ; j < sizeY; j ++)
	{
		for( int i = 0; i < sizeX; i ++)
		{
			if( mask[id + i])
				dstTemp[id + i] = srcTemp[id + i];
			else
				dstTemp[id + i] = 0;
		}
		id += sizeX;
	}

	memcpy_s(dst, (sizeof(UCHAR) * sizeX * sizeY), dstTemp, (sizeof(UCHAR) * sizeX * sizeY));

	SaveWorkImg(dst, sizeX, sizeY, _T("mount_body.bmp"));
	/*delete srcTemp;
	delete dstTemp;*/
	g_pMManager->pem_delete(srcTemp, true);
	g_pMManager->pem_delete(dstTemp, true);
	FreeMilImageBuff(&tempMilBuf);
}

void CProcMil_Mount::MaskTrueImg(UCHAR * src, UCHAR * mask, UCHAR * dst, int sizeX, int sizeY)
{
	if( src == NULL || mask == NULL || dst == NULL) 
		return ;

	UCHAR* temp = NULL;
	//temp = new UCHAR[sizeX * sizeY];
	temp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, (sizeof(UCHAR) * sizeX * sizeY));

	int id = 0;
	for( int j = 0 ; j < sizeY; j ++)
	{
		for( int i = 0; i < sizeX; i ++)
		{
			if( mask[id + i])
				temp[id + i] = src[id + i];
			else
				temp[id + i] = 0;
		}
		id += sizeX;
	}

	memcpy_s(dst, (sizeof(UCHAR) * sizeX * sizeY), temp, (sizeof(UCHAR) * sizeX * sizeY));
	//delete temp;
	g_pMManager->pem_delete(temp, true);
}

void CProcMil_Mount::MakeBodyMaskImg(UCHAR* bodyBlobImg, int sizeX, int sizeY, CRect bodyRect, UCHAR* retMaskImg)
{
	Im::PIL_ID milBodyRectLut = M_NULL;
	milBodyRectLut = AllocBuff(sizeX, sizeY, 0);

	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	Im::Gra::RectFill(M_DEFAULT, milBodyRectLut, (MIL_DOUBLE)bodyRect.left, (MIL_DOUBLE)bodyRect.top, (MIL_DOUBLE)bodyRect.right, (MIL_DOUBLE)bodyRect.bottom);

	Im::PIL_ID milBodyImg = M_NULL;
	milBodyImg = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milBodyImg, bodyBlobImg);

	Im::PIL_ID milRealBodyLut = M_NULL;
	milRealBodyLut = AllocBuff(sizeX, sizeY, 0);

	ArithImage(milBodyRectLut, milBodyImg, milRealBodyLut, M_AND );

	if(retMaskImg != NULL)
		Im::Buf::Get(milRealBodyLut, retMaskImg);

	FreeMilImageBuff(&milBodyRectLut);
	FreeMilImageBuff(&milRealBodyLut);
	FreeMilImageBuff(&milBodyImg);
}

/*
CMilBlobResult* CProcMil_Mount::CalcBlobM(Im::PIL_ID milBinSrc, int minArea, int* retBlobCnts, BOOL eraseBorderBlob , BOOL fillHole, Im::PIL_ID milDst)
{
	CMilBlobResult* blobResult = NULL;
	MIL_INT count = 0;

	if(fillHole)
	{
		FillHoleBlob(milBinSrc, milBinSrc);
	}
		
	if(eraseBorderBlob)
	{
		EraseBorderBlob(milBinSrc, milBinSrc);
	}
		
	Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	Im::Blob::blobCalculate(milBinSrc, M_NULL, m_milBlobFeature, m_milBlobResult);

	//blob select
	Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS, minArea, M_NULL);

	//redraw blob image
	if(milDst != M_NULL)
	{	
		Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
		Im::Blob::blobFill(m_milBlobResult, milDst, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw
	}

	//get result
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);
	*retBlobCnts = (int)count;

	if(count > (MIL_INT)0)
	{
		blobResult = new CMilBlobResult((int)count);
		MIL_INT32* label = new MIL_INT32[(int)count];

		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, blobResult->area);
		Im::Blob::blobGet_BOX(m_milBlobResult, blobResult->left,blobResult->top,blobResult->right,blobResult->bottom);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, blobResult->left);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, blobResult->right);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, blobResult->top);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, blobResult->bottom);
		Im::Blob::blobGet_Center(m_milBlobResult,blobResult->cx, blobResult->cy);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_X + M_TYPE_DOUBLE, blobResult->cx);	
// 		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_Y + M_TYPE_DOUBLE, blobResult->cy);
		Im::Blob::blobGetResult(m_milBlobResult, M_FERET_MIN_DIAMETER + M_TYPE_DOUBLE, blobResult->diameter);
		Im::Blob::blobGetResult(m_milBlobResult, M_FERET_MIN_ANGLE + M_TYPE_DOUBLE, blobResult->angle);
		// 		Im::Blob::blobGetResult(m_milBlobResult, M_BREADTH + M_TYPE_DOUBLE, blobResult->width);
		// 		Im::Blob::blobGetResult(m_milBlobResult, M_LENGTH + M_TYPE_DOUBLE, blobResult->length);
		for(int i = 0; i < (int)count; i++)
		{
			blobResult->width[i] = 0.0;
			blobResult->length[i] = 0.0;
		}
		Im::Blob::blobGetResult(m_milBlobResult, M_LABEL_VALUE + M_TYPE_MIL_INT32, label);

		memcpy_s(blobResult->blobLable,(sizeof(int)*count),label,(sizeof(MIL_INT32)*count));
		delete label;

	}

	return blobResult;
}*/


void CProcMil_Mount::GetBlobResult(CMilBlobResult* retResult)
{
	CMilBlobResult* blobResult = NULL;
	MIL_INT count = 0;

	Im::Blob::blobGetNumber(m_milBlobResult,  &count);

	if(count > (MIL_INT)0)
	{
		blobResult = new CMilBlobResult((int)count);
		g_pMManager->pem_new_check(blobResult, (PCHAR)__FUNCTION__, __LINE__);

		//MIL_INT32* label = new MIL_INT32[(int)count];
		MIL_INT32* label = g_pMManager->pem_new<MIL_INT32>(true, (int)count, (PCHAR)__FUNCTION__, __LINE__);

		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, blobResult->area);
		Im::Blob::blobGet_BOX(m_milBlobResult, blobResult->left,blobResult->top,blobResult->right,blobResult->bottom);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, blobResult->left);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, blobResult->right);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, blobResult->top);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, blobResult->bottom);
		Im::Blob::blobGet_Center(m_milBlobResult,blobResult->cx, blobResult->cy);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_X + M_TYPE_DOUBLE, blobResult->cx);	
// 		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_Y + M_TYPE_DOUBLE, blobResult->cy);
		Im::Blob::blobGetResult(m_milBlobResult, M_FERET_MIN_DIAMETER + M_TYPE_DOUBLE, blobResult->diameter);
		Im::Blob::blobGetResult(m_milBlobResult, M_FERET_MIN_ANGLE + M_TYPE_DOUBLE, blobResult->angle);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BREADTH + M_TYPE_DOUBLE, blobResult->width);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_LENGTH + M_TYPE_DOUBLE, blobResult->length);
		for(int i = 0; i < (int)count; i++)
		{
			blobResult->width[i] = blobResult->right[i] - blobResult->left[i];
			blobResult->length[i] = blobResult->bottom[i] - blobResult->top[i];
		}
		Im::Blob::blobGetResult(m_milBlobResult, M_LABEL_VALUE + M_TYPE_MIL_INT32, label);

		memcpy_s(blobResult->blobLabel,(sizeof(int)*count),label,(sizeof(MIL_INT32)*count));
		//delete label;
		g_pMManager->pem_delete(label, true);

		retResult->Copy(blobResult,0);
		//delete blobResult;
		g_pMManager->pem_delete(blobResult, false);
	}	
}

int CProcMil_Mount::CalcBlob_ForCnt(Im::PIL_ID milBin)
{
	MIL_INT count = 0;

	Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	Im::Blob::blobCalculate(milBin, M_NULL, m_milBlobFeature, m_milBlobResult);

	//get result
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);

	return (int)count;
}


int CProcMil_Mount::SelectBlob_MaxArea(int blobNum, UCHAR* userDst, int sizeX, int sizeY)
{
	if(blobNum < 1)
		return 0;

	Im::PIL_ID milTemp = M_NULL;
	milTemp = AllocBuff(sizeX, sizeY);

	Im::Buf::Put(milTemp, userDst);

	double * area;
	//area = (double *)malloc(sizeof( double) * blobNum);
	area = (double *)g_pMManager->pem_malloc(sizeof(double) * blobNum, (PCHAR)__FUNCTION__, __LINE__);

	Im::Blob::blobGetResult( m_milBlobResult, M_AREA, area);

	double max = area[0];
	int index = 0;
	for( int i = 1; i < blobNum ; i ++)
	{
		if( max < area[i])
		{
			index = i;
			max = area[i];
		}
	}

	Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_NOT_EQUAL, max, max); 
	Im::Blob::blobFill(m_milBlobResult, milTemp,  M_EXCLUDED_BLOBS, 0); 
	Im::Blob::blobFill(m_milBlobResult, milTemp,  M_INCLUDED_BLOBS, 255);

	Im::Buf::Get(milTemp, userDst);


	if(area)	
		//free(area);
		g_pMManager->pem_free(area);

	FreeMilImageBuff(&milTemp);

	return index;
}

int CProcMil_Mount::SelectBlob_MaxArea(int blobNum,  Im::PIL_ID milDst)
{
	double * area;
	//area = (double *)malloc(sizeof( double) * blobNum);
	area = (double *)g_pMManager->pem_malloc(sizeof(double) * blobNum, (PCHAR)__FUNCTION__, __LINE__);

	Im::Blob::blobGetResult( m_milBlobResult, M_AREA, area);

	double max = area[0];
	int index = 0;
	for( int i = 1; i < blobNum ; i ++)
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


CMilBlobResult* CProcMil_Mount::BlobProc(void* milSrc, UCHAR * mask, int cx, int cy, int sizeX, int sizeY, UCHAR* retTrueImg)
{
	if(sizeX < 0 || sizeY < 0)
		return NULL;

	UCHAR* realImg = NULL;
	//realImg = new UCHAR[sizeX * sizeY];
	realImg = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	MaskTrueImg(milSrc, mask, realImg, cx, cy, sizeX, sizeY);

	if(retTrueImg != NULL)
		memcpy_s(retTrueImg, (sizeof(UCHAR) * sizeX * sizeY), realImg, (sizeof(UCHAR) * sizeX * sizeY));

	UCHAR* binImg = NULL;
	//binImg = new UCHAR[sizeX * sizeY];
	binImg = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	Binarize(realImg, sizeX, sizeY, binImg, 1, FALSE);

	SaveWorkImg(binImg, sizeX, sizeY, _T("bin.bmp"));

	UCHAR* blobImg = NULL;
	//blobImg = new UCHAR[sizeX * sizeY];
	blobImg = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(blobImg, 0, sizeof(UCHAR) * sizeX * sizeY);

	int blobCnt = 0;
	CMilBlobResult* blobRst = NULL;
	int minArea = 0;
	int maxArea = 0;
	minArea = (int)((sizeX * sizeY) * (1 / 100.0));   //area  *  1%
	maxArea = (int)((sizeX * sizeY) * (90 / 100.0));  //area  *  90% 
	blobCnt = CalcBlob(binImg, sizeX, sizeY, minArea, maxArea, FALSE, TRUE); 
																							

	if(blobCnt > 1)
	{
		if(blobRst != NULL)
			//delete blobRst;
			g_pMManager->pem_delete(blobRst, false);
		
		SelectBlob_MaxArea(blobCnt, blobImg, sizeX, sizeY);
		
		blobCnt = CalcBlob(blobImg, sizeX, sizeY, minArea, maxArea, FALSE, TRUE);
		
 	}

 	blobRst = new CMilBlobResult(blobCnt);
	g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

 	GetBlobResult(blobRst);


	/*delete realImg;
	delete binImg;
	delete blobImg;*/
	g_pMManager->pem_delete(realImg, true);
	g_pMManager->pem_delete(binImg, true);
	g_pMManager->pem_delete(blobImg, true);

	return blobRst;

}


int CProcMil_Mount::BlobProc(UCHAR* userSrc, int sizeX, int sizeY, BOOL fillOutside, UCHAR* blobImg, CString wndName, int fovIndex)
{
	int blobCnt = 0;

	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSrc);

	Im::PIL_ID milBin = M_NULL;
	milBin = AllocBuff(sizeX, sizeY);
	Im::PIL_ID milBlob = M_NULL;
	milBlob = AllocBuff(sizeX, sizeY);

	int threshold = GetThreshold(milSrc);

	CProcMil::Binarize(milSrc, milBin, threshold, FALSE);
	blobCnt = CalcBlob(milBin, 300, 0, TRUE, TRUE, milBlob);

	
	//SaveWorkImg(milBin,_T("bin.bmp"));
	//SaveWorkImg(milBlob,_T("blob_1st.bmp"));
	

	if(blobCnt > 1)
	{
		SelectBlob_MaxArea(blobCnt, milBlob);
		blobCnt = CalcBlob(milBlob, 300, 0, TRUE, TRUE, milBlob);
//		SaveWorkImg(milBlob,_T("blob2.bmp"));
	}
	

	//////////////////////////////////////////////////////////////////////////
	BOOL flag = TRUE;

	if(flag)
	{
		CMilBlobResult* blobRst = NULL;
		blobRst = new CMilBlobResult(blobCnt);
		g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

		GetBlobResult(blobRst);

		double per = 20.0;//20%
		int marginX = (int)(blobRst->width[0] * (per / 100.0));  
		int marginY = (int)(blobRst->length[0] * (per / 100.0));
		int cx = (int)(blobRst->cx[0]);
		int cy = (int)(blobRst->cy[0]);
		int width = (int)(blobRst->width[0] - (marginX * 2.0));
		int length = (int)(blobRst->length[0] - (marginY * 2.0));
		int area = width * length;

		Im::PIL_ID clipTemp = M_NULL;
		clipTemp = AllocClipBuff(milBlob, cx, cy, width, length);
		//SaveWorkImg(clipTemp, _T("innerClip.bmp"));

		UCHAR* userClipTemp = NULL;
		//userClipTemp = new UCHAR[area];
		userClipTemp = g_pMManager->pem_new<UCHAR>(true, area, (PCHAR)__FUNCTION__, __LINE__);
		//memset(userClipTemp, 0, sizeof(UCHAR) * area);

		Im::Buf::Get(clipTemp, userClipTemp);

		float cnt = 0;

		UCHAR data = 0;
		for(int y = 0; y < length; y++)
		{
			for(int x = 0; x < width; x++)
			{
				data = userClipTemp[(y * width) + x];
				if(data == 255)
					cnt++;
			}
		}

		double factor = (cnt / area) * 100.0;

		if(factor >= 95.0)
		{
			MorOpen(milBlob, milBlob, 6);
			blobCnt = CalcBlob(milBlob, 0, 0, FALSE, FALSE, NULL); //오픈 연산후 블랍이 쪼개졌는지 보려고 개수만 가져온다.
		}

//		SaveWorkImg(milBlob,_T("MorOpen.bmp"));

		FreeMilImageBuff(&clipTemp);
		/*delete blobRst;
		delete userClipTemp;*/
		g_pMManager->pem_delete(blobRst, false);
		g_pMManager->pem_delete(userClipTemp, true);
	}
	//////////////////////////////////////////////////////////////////////////


	SelectBlob_MaxArea(blobCnt, milBlob);
	blobCnt = CalcBlob(milBlob, 300, 0, TRUE, TRUE, milBlob);


	if(fillOutside)
	{
		FillBlob_Outside(milBlob, milBlob);
		//SaveWorkImg(milBlob,_T("blobFill.bmp"));
	}

	Im::Buf::Get(milBlob, blobImg);

	if(MOUNT_DEBUG == 1)  //debug
	{		
		if(!wndName.IsEmpty() && fovIndex != -1)
		{
			CString day = _T("");
			CString time = _T("");
			CString path = _T("");		
			CString fullPath = _T("");
			CString fileName = _T("");
			CString fileName2 = _T("");
			CTime ct;
			ct = CTime::GetCurrentTime();

			day.Format(_T("%04d%02d%02d"), ct.GetYear(), ct.GetMonth(), ct.GetDay());
			time.Format(_T("%02d%02d%02d"), ct.GetHour(), ct.GetMinute(), ct.GetSecond());

			path.Format(_T("%s_%s"),day, time);

			
			fullPath.Format(_T("D:\\TestImage\\workimage\\fov_%d"),fovIndex);
			CreateDir(fullPath);

			fileName.Format(_T("\\%s_%s_blob.bmp"), wndName, path);
			fileName2.Format(_T("\\%s_%s_bin.bmp"), wndName, path);

			SaveBuff(milBlob, fullPath + fileName);
			SaveBuff(milBin, fullPath + fileName2);
						
		}
	}


	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milBin);
	FreeMilImageBuff(&milBlob);


	return blobCnt;
}

int CProcMil_Mount::MakeBodyImg(UCHAR* userSrc, int sizeX, int sizeY, CRect partRect, CRect wndRect, UCHAR* userDst, CRect* bodyRect, int* retBlobCx, int* retBlobCy)
{
	SaveWorkImg(userSrc, sizeX, sizeY, _T("bin.bmp"));
	int area = sizeX * sizeY;
	int minArea = MOUNT_BLOB_MIN_AREA; // blob 면적이 250 이하면 제거

	//UCHAR* blobImg = new UCHAR[area];
	UCHAR* blobImg = g_pMManager->pem_new<UCHAR>(true, area, (PCHAR)__FUNCTION__, __LINE__);

	//blob
	int blobCnt = CalcBlob(userSrc, sizeX, sizeY, minArea, 0 ,FALSE, TRUE, blobImg);
	SaveWorkImg(blobImg, sizeX, sizeY, _T("bin2.bmp"));

	CMilBlobResult* blobRst = new CMilBlobResult(blobCnt);
	g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

	GetBlobResult(blobRst);

	int label = -1;
	int index = 0;
	CRect rtTemp(0,0,0,0);

	if(blobCnt == 1)
	{	//blob이 1개 일경우 외곽 채운후 이미지 리턴

		//찾은 blob이 part영역 안에 없으면 missing처리...
		CRect bdRt((int)blobRst->left[0], (int)blobRst->top[0], (int)blobRst->right[0], (int)blobRst->bottom[0]);
		if(!CheckBodyRect(partRect, wndRect, bdRt))
		{
			blobCnt = 0;
		}

		if(blobCnt > 0 && userDst != NULL)
		{
			FillBlob_Outside(blobImg, sizeX, sizeY, blobImg);
			memcpy_s(userDst, sizeof(UCHAR) * area, blobImg, sizeof(UCHAR) * area);
		}
	}
	else if(blobCnt > 1)
	{	//blob이 2개 이상일경우...

		CRect rt;
		CPoint pt((int)(sizeX / 2.0), (int)(sizeY / 2.0));
		
		//mount window center를 포함하는 blob만을 남긴다. (blob area가 아닌 blob box를 본다.)
		
		for(int i = 0; i < blobCnt; i++)
		{
			rt.SetRect((int)blobRst->left[i] ,(int)blobRst->top[i] ,(int)blobRst->right[i] ,(int)blobRst->bottom[i]);

			if(PtInRect(rt,pt))
			{
				label = blobRst->blobLabel[i];
				index = i;
				break;				
			}

		}

		//mount window center를 포함하는 blob이 없을경우... 제일 큰 blob을 찾는다. (mount window center에 제일 근접한 blob을 찾아야 하는 경우도 있을듯..)
		if(label == -1)
		{
			double max = blobRst->area[0];
			label = blobRst->blobLabel[0];
			index = 0;
			for( int i = 1; i < blobCnt ; i ++)
			{
				if( max < blobRst->area[i])
				{
					max = blobRst->area[i];
					label = blobRst->blobLabel[i];
					index = i;
				}
			}

			//찾은 blob이 part영역 안에 없으면 missing처리...
			CRect bdRt((int)blobRst->left[index], (int)blobRst->top[index], (int)blobRst->right[index], (int)blobRst->bottom[index]);
			if(!CheckBodyRect(partRect, wndRect, bdRt))
			{
				blobCnt = 0;
			}
		}	
		

		//블랍을 그려줌..
		if(blobCnt > 0)
		{			
			Im::PIL_ID milDst = AllocBuff(sizeX, sizeY, 0);
			Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
			DrawBlob(m_milBlobResult, milDst, label, FALSE);

			FillBlob_Outside(milDst, milDst);

			Im::Buf::Get(milDst, userDst);
			FreeMilImageBuff(&milDst);

			blobCnt = 1;
		}
		
	}


	if(bodyRect != NULL && blobCnt != 0)
	{
		rtTemp.SetRect((int)blobRst->left[index] ,(int)blobRst->top[index] ,(int)blobRst->right[index] ,(int)blobRst->bottom[index]);
		*bodyRect = rtTemp;

		*retBlobCx = (int)blobRst->cx[index];
		*retBlobCy = (int)blobRst->cy[index];
	}

	/*delete blobImg;
	delete blobRst;*/
	g_pMManager->pem_delete(blobImg, true);
	g_pMManager->pem_delete(blobRst, false);

	return blobCnt;
}


int CProcMil_Mount::MakeBodyImg2(UCHAR* userSrc, int sizeX, int sizeY, CRect partRect, CRect wndRect, UCHAR* userDst, CRect* bodyRect, int* retBlobCx, int* retBlobCy)
{
	int area = sizeX * sizeY;
	int minArea = MOUNT_BLOB_MIN_AREA;//(int)(area * 0.1); //10%

	//UCHAR* blobImg = new UCHAR[area];
	UCHAR* blobImg = g_pMManager->pem_new<UCHAR>(true, area, (PCHAR)__FUNCTION__, __LINE__);

	//blob
	int blobCnt = CalcBlob(userSrc, sizeX, sizeY, minArea, 0 ,FALSE, TRUE, blobImg);

	CMilBlobResult* blobRst = new CMilBlobResult(blobCnt);
	g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

	GetBlobResult(blobRst);

	int label = -1;
	int index = 0;
	CRect rtTemp(0,0,0,0);

	if(blobCnt == 1)
	{	//blob이 1개 일경우 외곽 채운후 이미지 리턴

		//찾은 blob이 part영역 안에 없으면 missing처리...
		CRect bdRt((int)blobRst->left[0], (int)blobRst->top[0], (int)blobRst->right[0], (int)blobRst->bottom[0]);
		if(!CheckBodyRect(partRect, wndRect, bdRt))
		{
			blobCnt = 0;
		}

		if(blobCnt > 0 && userDst != NULL) //image return
		{
			Im::PIL_ID milDst = AllocBuff(sizeX, sizeY, 0);
			Im::Buf::Put(milDst, blobImg);

			////////////////////////////////////////////////////////////////////////// //외곽을 다듬어줌
			MorOpen(milDst, milDst, 6);
			int cnt = CalcBlob_ForCnt(milDst);
			if(cnt > 1)
			{
				SelectBlob_MaxArea(cnt, milDst);
			}
			//////////////////////////////////////////////////////////////////////////
			Im::Buf::Get(milDst, userDst);
			FreeMilImageBuff(&milDst);
		}
	}
	else if(blobCnt > 1)
	{	//blob이 2개 이상일경우...

		CRect rt;
		CPoint pt((int)(sizeX / 2.0), (int)(sizeY / 2.0));

		//mount window center를 포함하는 blob만을 남긴다. (blob area가 아닌 blob box를 본다.)

		for(int i = 0; i < blobCnt; i++)
		{
			rt.SetRect((int)blobRst->left[i] ,(int)blobRst->top[i] ,(int)blobRst->right[i] ,(int)blobRst->bottom[i]);

			if(PtInRect(rt,pt))
			{
				label = blobRst->blobLabel[i];
				index = i;
				break;
			}

		}

		//mount window center를 포함하는 blob이 없을경우... 제일 큰 blob을 찾는다. (mount window center에 제일 근접한 blob을 찾아야 하는 경우도 있을듯..)
		if(label == -1)
		{
			double max = blobRst->area[0];
			for( int i = 1; i < blobCnt ; i ++)
			{
				if( max < blobRst->area[i])
				{
					max = blobRst->area[i];
					label = blobRst->blobLabel[i];
					index = i;
				}
			}

			//찾은 blob이 part영역 안에 없으면 missing처리...
			CRect bdRt((int)blobRst->left[index], (int)blobRst->top[index], (int)blobRst->right[index], (int)blobRst->bottom[index]);
			if(!CheckBodyRect(partRect, wndRect, bdRt))
			{
				blobCnt = 0;
			}
		}	


		//블랍을 그려줌..
		if(blobCnt > 0)  //image return
		{			
			Im::PIL_ID milDst = AllocBuff(sizeX, sizeY, 0);
			Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
			DrawBlob(m_milBlobResult, milDst, label, FALSE);

			////////////////////////////////////////////////////////////////////////// //외곽을 다듬어줌
			MorOpen(milDst, milDst, 6);
			int cnt = CalcBlob_ForCnt(milDst);
			if(cnt > 1)
			{
				SelectBlob_MaxArea(cnt, milDst);
			}
			//////////////////////////////////////////////////////////////////////////
			Im::Buf::Get(milDst, userDst);
			FreeMilImageBuff(&milDst);

			blobCnt = 1;
		}

	}


	if(bodyRect != NULL && blobCnt != 0)
	{
		rtTemp.SetRect((int)blobRst->left[index] ,(int)blobRst->top[index] ,(int)blobRst->right[index] ,(int)blobRst->bottom[index]);
		*bodyRect = rtTemp;

		*retBlobCx = (int)blobRst->cx[index];
		*retBlobCy = (int)blobRst->cy[index];
	}

	/*delete blobImg;
	delete blobRst;*/
	g_pMManager->pem_delete(blobImg, true);
	g_pMManager->pem_delete(blobRst, false);

	return blobCnt;
}



BOOL CProcMil_Mount::CheckBodyRect(CRect partRect, CRect wndRect, CRect blobRect)
{
	BOOL ret = FALSE;

	CPoint pt[4];

	//left top
	pt[0].x = (LONG)(wndRect.left + blobRect.left);
	pt[0].y = (LONG)(wndRect.top + blobRect.top);
	//left bottom
	pt[1].x = pt[0].x;
	pt[1].y = (LONG)(pt[0].y + blobRect.Height());
	//right bottom
	pt[2].x = (LONG)(pt[0].x + blobRect.Width());
	pt[2].y = pt[1].y;
	//right top
	pt[3].x = pt[2].x;
	pt[3].y = pt[0].y;

	for(int i = 0; i < 4; i++)//사각형
	{
		if(PtInRect(partRect, pt[i]))
		{
			ret = TRUE;
			break;
		}
	}

	return ret;
}

void CProcMil_Mount::GetEraseBoarderData(UCHAR* userSrcImg, int sizeX, int sizeY, UCHAR* retMask)
{
	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSrcImg);

	Im::PIL_ID milDst = M_NULL;
	milDst = AllocBuff(sizeX, sizeY);
	Im::Buf::Copy(milSrc, milDst);

	//SaveWorkImg(milSrc, _T("111.bmp"));

	Im::Blob::blobReconstruct(milSrc, M_NULL, milSrc, M_ERASE_BORDER_BLOBS , M_8_CONNECTED );
	Im::Blob::blobCalculate( milSrc, M_NULL, m_milBlobFeature, m_milBlobResult);
	

	
	Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 255); //redraw excluded blob (value : 255) -> draw
	Im::Blob::blobFill(m_milBlobResult, milDst, M_INCLUDED_BLOBS, 0);   //redraw included blob (value : 0) -> erase

	//SaveWorkImg(milDst, _T("222.bmp"));

	Im::Buf::Get(milDst, retMask);

/*	float* dstZmap = new float[size];
	memset(dstZmap, 0, sizeof(float) * size);

	UCHAR data = 0;
	int index = 0;
	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			index = (sizeX * y) + x;
			data = mask[index];
			if(data == 0)
			{
				dstZmap[index] = srcZmap[index];
			}
			else
			{

			}
		}
	}
	delete dstZmap;*/

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milDst);
}

void CProcMil_Mount::CalcSampleArea(CRect bodyRect, CRect* retSampleArea)
{
	double marginPerLong = 12.0;
	double marginPerShort = 10.0;
	double lengthPer = 20.0;

	int w = bodyRect.Width();
	int h = bodyRect.Height();

//////////////////////////////////////////////////////////////////////////
	BOOL flag = FALSE; // h >= w
	int sub = h - w;
	int tempValue = w;
	if(h < w)  //길이가 작은쪽이 기준...
	{
		tempValue = h;
		flag = TRUE;
	}

	if(tempValue < 100)
		lengthPer = 30.0;

	int rectLength = (int)(tempValue * (lengthPer / 100.0));
//////////////////////////////////////////////////////////////////////////
	double marginPerW = marginPerShort;
	double marginPerH = marginPerLong;

	if(flag == TRUE)  //h < w
	{
		marginPerW = marginPerLong;
		marginPerH = marginPerShort;
		sub = w - h;
	}	

	if(sub <= 15)
	{
		marginPerW = marginPerShort;
		marginPerH = marginPerShort;
	}

	


	int marginW = (int)(w * (marginPerW / 100.0));
	int marginH = (int)(h * (marginPerH / 100.0));
	
//////////////////////////////////////////////////////////////////////////


	//CRect* retRt = new CRect[4];
	CRect* retRt = g_pMManager->pem_new<CRect>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(retRt, 0, sizeof(CRect) * 4);

	// top left
	retRt[0].left = bodyRect.left + marginW;
	retRt[0].right = retRt[0].left + rectLength;
	retRt[0].top = bodyRect.top + marginH;
	retRt[0].bottom = retRt[0].top + rectLength;
	// bottom left
	retRt[1].left = bodyRect.left + marginW;
	retRt[1].right = retRt[1].left + rectLength;
	retRt[1].top = bodyRect.bottom - (marginH + rectLength);
	retRt[1].bottom = retRt[1].top + rectLength;
	// bottom right
	retRt[2].left = bodyRect.right - (marginW + rectLength);
	retRt[2].right = retRt[2].left + rectLength;
	retRt[2].top = bodyRect.bottom - (marginH + rectLength);
	retRt[2].bottom = retRt[2].top + rectLength;
	// top right
	retRt[3].left = bodyRect.right - (marginW + rectLength);
	retRt[3].right = retRt[3].left + rectLength;
	retRt[3].top = bodyRect.top + marginH;
	retRt[3].bottom = retRt[3].top + rectLength;

/*	// top left
	retRt[0].left = bodyRect.left + marginW;
	retRt[0].right = bodyRect.right - marginW;
	retRt[0].top = bodyRect.top + marginH;
	retRt[0].bottom = retRt[0].top + rectLength;
	// bottom left
	retRt[1].left = bodyRect.left + marginW;
	retRt[1].right = bodyRect.right - marginW;
	retRt[1].top = bodyRect.bottom - (marginH + rectLength);
	retRt[1].bottom = retRt[1].top + rectLength;
	// bottom right
	retRt[2].left = bodyRect.left + marginW;
	retRt[2].right = retRt[2].left + rectLength;
	retRt[2].top = bodyRect.top + marginH;
	retRt[2].bottom = bodyRect.bottom - marginH;
	// top right
	retRt[3].left = bodyRect.right - (marginW + rectLength);
	retRt[3].right = retRt[3].left + rectLength;
	retRt[3].top = bodyRect.top + marginH;
	retRt[3].bottom = bodyRect.bottom - marginH;*/

	memcpy_s(retSampleArea, sizeof(CRect) * 4, retRt, sizeof(CRect) * 4);
	//delete retRt;
	g_pMManager->pem_delete(retRt, true);
}


//body all
float CProcMil_Mount::CalcHeight(float* zmapRoiData, UCHAR* bodyImg, int roiSizeX, int roiSizeY, CRect bodyRect, UCHAR* retBodyLut)
{
	float hAvr = 0;
	int roiArea = roiSizeX * roiSizeY;

	Im::PIL_ID milBodyRectLut = M_NULL;
	milBodyRectLut = AllocBuff(roiSizeX, roiSizeY, 0);
	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	Im::Gra::RectFill(M_DEFAULT, milBodyRectLut, (MIL_DOUBLE)bodyRect.left, (MIL_DOUBLE)bodyRect.top, (MIL_DOUBLE)bodyRect.right, (MIL_DOUBLE)bodyRect.bottom);

	Im::PIL_ID milBodyImg = M_NULL;
	milBodyImg = AllocBuff(roiSizeX, roiSizeY);
	Im::Buf::Put(milBodyImg, bodyImg);

	Im::PIL_ID milRealBodyLut = M_NULL;
	milRealBodyLut = AllocBuff(roiSizeX, roiSizeY, 0);

	ArithImage(milBodyRectLut, milBodyImg, milRealBodyLut, M_AND );
	if(retBodyLut != NULL)
		Im::Buf::Get(milRealBodyLut, retBodyLut);

	UCHAR* userMilRealBodyLut = NULL;
	//userMilRealBodyLut = new UCHAR[roiArea];
	userMilRealBodyLut = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	//memset(userMilRealBodyLut, 0, sizeof(UCHAR) * roiArea);
	Im::Buf::Get(milRealBodyLut, userMilRealBodyLut);

	int index = 0;
	UCHAR data = 0;
	double sum = 0;
	int cnt = 0;
	for(int y = 0; y < roiSizeY; y++)
	{
		for(int x = 0; x < roiSizeX; x++)
		{
			index = (roiSizeX * y) + x;
			data = userMilRealBodyLut[index];

			if(data > 0)
			{
				sum += zmapRoiData[index];
				cnt++;
			}
		}
	}

	hAvr = (float)(sum / cnt);

	FreeMilImageBuff(&milBodyRectLut);
	FreeMilImageBuff(&milBodyImg);
	FreeMilImageBuff(&milRealBodyLut);
	//delete userMilRealBodyLut;
	g_pMManager->pem_delete(userMilRealBodyLut, true);
	return hAvr;
}

//body 4 point
float CProcMil_Mount::CalcHeight2(float* zmapRoiData, UCHAR* bodyImg, int roiSizeX, int roiSizeY, CRect bodyRect, UCHAR* retBodyLut)
{
	float hAvr = 0;
	int roiArea = roiSizeX * roiSizeY;

	//////////////////////////////////////////////////////////////////////////
	//CRect* retRt = new CRect[4];
	CRect* retRt = g_pMManager->pem_new<CRect>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(retRt, 0, sizeof(CRect) * 4);

	CalcSampleArea(bodyRect, retRt);

	Im::PIL_ID milBodyRectLut_4Pt = M_NULL;
	milBodyRectLut_4Pt = AllocBuff(roiSizeX, roiSizeY, 0);

	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	for(int i = 0; i < 4; i++)
	{		
		Im::Gra::RectFill(M_DEFAULT, milBodyRectLut_4Pt, retRt[i].left, retRt[i].top, retRt[i].right, retRt[i].bottom);
	}
	
	//delete retRt;
	g_pMManager->pem_delete(retRt, true);
	
	//////////////////////////////////////////////////////////////////////////

	Im::PIL_ID milBodyImg = M_NULL;
	milBodyImg = AllocBuff(roiSizeX, roiSizeY);
	Im::Buf::Put(milBodyImg, bodyImg);

	Im::PIL_ID milRealBodyLut = M_NULL;
	milRealBodyLut = AllocBuff(roiSizeX, roiSizeY, 0);
	ArithImage(milBodyRectLut_4Pt, milBodyImg, milRealBodyLut, M_AND );

	if(retBodyLut != NULL)
	{
		Im::PIL_ID milBodyRectLut = M_NULL;
		milBodyRectLut = AllocBuff(roiSizeX, roiSizeY, 0);
		Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
		Im::Gra::RectFill(M_DEFAULT, milBodyRectLut, (MIL_DOUBLE)bodyRect.left, (MIL_DOUBLE)bodyRect.top, (MIL_DOUBLE)bodyRect.right, (MIL_DOUBLE)bodyRect.bottom);

		Im::PIL_ID milRealBodyLut_Return = M_NULL;
		milRealBodyLut_Return = AllocBuff(roiSizeX, roiSizeY, 0);
		ArithImage(milBodyRectLut, milBodyImg, milRealBodyLut_Return, M_AND );
		Im::Buf::Get(milRealBodyLut_Return, retBodyLut);

		FreeMilImageBuff(&milBodyRectLut);
		FreeMilImageBuff(&milRealBodyLut_Return);
	}
	

	UCHAR* userMilRealBodyLut = NULL;
	//userMilRealBodyLut = new UCHAR[roiArea];
	userMilRealBodyLut = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	//memset(userMilRealBodyLut, 0, sizeof(UCHAR) * roiArea);
	Im::Buf::Get(milRealBodyLut, userMilRealBodyLut);

	int index = 0;
	UCHAR data = 0;
	double sum = 0;
	int cnt = 0;
	for(int y = 0; y < roiSizeY; y++)
	{
		for(int x = 0; x < roiSizeX; x++)
		{
			index = (roiSizeX * y) + x;
			data = userMilRealBodyLut[index];

			if(data > 0)
			{
				sum += zmapRoiData[index];
				cnt++;
			}
		}
	}

	hAvr = (float)(sum / cnt);

	
	FreeMilImageBuff(&milBodyImg);
	FreeMilImageBuff(&milRealBodyLut);
	FreeMilImageBuff(&milBodyRectLut_4Pt);
	//delete userMilRealBodyLut;
	g_pMManager->pem_delete(userMilRealBodyLut, true);
	return hAvr;
}

double CProcMil_Mount::CalcGradient(float* zmapRoiData, int roiSizeX, int roiSizeY, CRect bodyRect, float hThresh, double* retVerAngle, double* retHorAngle)
{
	double retGradi = 0;
	
	//CRect* retRt = new CRect[4];
	CRect* retRt = g_pMManager->pem_new<CRect>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(retRt, 0, sizeof(CRect) * 4);
	CalcSampleArea(bodyRect, retRt);

	Im::PIL_ID milZmapData = M_NULL;
	milZmapData = AllocBuff_float(roiSizeX, roiSizeY);
	Im::Buf::Put(milZmapData, zmapRoiData);

	///////////////////////////////////////////////////////////////////////////////
	Im::PIL_ID milImage = M_NULL;
	milImage = AllocBuff(roiSizeX, roiSizeY, 0);
	Im::Gra::Rect(M_DEFAULT, milImage, bodyRect.left, bodyRect.top, bodyRect.right, bodyRect.bottom);

	for(int i = 0; i < 4; i++)
	{
		Im::Gra::Rect(M_DEFAULT, milImage, retRt[i].left, retRt[i].top, retRt[i].right, retRt[i].bottom);
	}
	

	SaveWorkImg(milImage, _T("rect.bmp"));
	Im::Buf::Free(milImage);
	milImage = M_NULL;
	///////////////////////////////////////////////////////////////////////////////


	//float* avrH = new float[4]; //0: topLeft,  1: bottomLeft,  2: bottomRight,  3: topRight
	float* avrH = g_pMManager->pem_new<float>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(avrH, 0, sizeof(float) * 4);

	int cx = 0;
	int cy = 0;
	int w = retRt[0].right - retRt[0].left;
	int h = retRt[0].bottom - retRt[0].top;
	int area = w * h;

	//float* userClipZmapData = new float[area];
	float* userClipZmapData = g_pMManager->pem_new<float>(true, area, (PCHAR)__FUNCTION__, __LINE__);
	//memset(userClipZmapData, 0, sizeof(float) * area);	
		
	Im::PIL_ID milClipZmapData = M_NULL;
	CString test = _T("");
	for(int i = 0; i < 4; i++)
	{		
		cx = (int)(retRt[i].left + (retRt[i].Width() / 2.0));
		cy = (int)(retRt[i].top + (retRt[i].Height() / 2.0));

		milClipZmapData = AllocClipBuff_float(milZmapData, cx, cy, w, h);
		Im::Buf::Get(milClipZmapData, userClipZmapData);
		FreeMilImageBuff(&milClipZmapData);

		double sum = 0.0;
		int cnt = 0;
		int index = 0;
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{
				index = (w * y) + x;

				if(userClipZmapData[index] >= hThresh)
				{
					sum += userClipZmapData[index];
					cnt++;
				}
			}
		}
		avrH[i] = (float)(sum / cnt);
		memset(userClipZmapData, 0, sizeof(float) * area);
	}
	FreeMilImageBuff(&milZmapData);	
//////////////////////////////////////////////////////////////////////////
/*	double avrH_left = 0;
	double avrH_right = 0;
	double avrH_top = 0;
	double avrH_bottom= 0;

	avrH_left = (avrH[0] + avrH[1]) / 2.0;
	avrH_right = (avrH[2] + avrH[3]) / 2.0;

	avrH_top = (avrH[0] + avrH[3]) / 2.0;
	avrH_bottom = (avrH[1] + avrH[2]) / 2.0;


	double verticalGradi = 0;	//ㅣ
	double horizontalGradi = 0;	//ㅡ
	double tempX = 0;	//밑변																																
	double tempY = 0;   //높이
	double angleTemp = 0;

	tempX = _pixel2mm((double)(retRt[1].top - retRt[0].top)); 
	tempY = _micron2mm((double)(avrH_bottom - avrH_top));  
	angleTemp = atan2(tempY, tempX);
	verticalGradi = _radian2degree(angleTemp);  //결과가 0보다 작을경우 -> 아래쪽이 낮음   //  결과가 0보다 클경우 -> 위쪽이 낮음

	tempX = _pixel2mm((double)(retRt[3].left - retRt[0].left));
	tempY = _micron2mm((double)(avrH_right - avrH_left));
	angleTemp = atan2(tempY, tempX);
	horizontalGradi = _radian2degree(angleTemp);  //결과가 0보다 작을경우 -> 오른쪽이 낮음   //  결과가 0보다 클경우 -> 왼쪽이 낮음

	*retVerAngle = verticalGradi;
	*retHorAngle = horizontalGradi;*/

	double verticalGradi_left = 0.0;
	double verticalGradi_right = 0.0;
	double horizontalGradi_top = 0.0;
	double horizontalGradi_bottom = 0.0;

	double tempX = 0;	//밑변																																
	double tempY = 0;   //높이
	double angleTemp = 0;

	tempX = _pixel2mm_x((double)(retRt[1].top - retRt[0].top)); 
	tempY = _micron2mm((double)(avrH[1] - avrH[0]));  
	angleTemp = atan2(tempY, tempX);
	verticalGradi_left = _radian2degree(angleTemp);  //결과가 0보다 작을경우 -> 아래쪽이 낮음   //  결과가 0보다 클경우 -> 위쪽이 낮음

	tempX = _pixel2mm_y((double)(retRt[2].top - retRt[3].top)); 
	tempY = _micron2mm((double)(avrH[2] - avrH[3]));  
	angleTemp = atan2(tempY, tempX);
	verticalGradi_right = _radian2degree(angleTemp);  //결과가 0보다 작을경우 -> 아래쪽이 낮음   //  결과가 0보다 클경우 -> 위쪽이 낮음

	tempX = _pixel2mm_x((double)(retRt[3].left - retRt[0].left)); 
	tempY = _micron2mm((double)(avrH[3] - avrH[0]));  
	angleTemp = atan2(tempY, tempX);
	horizontalGradi_top = _radian2degree(angleTemp);  //결과가 0보다 작을경우 -> 아래쪽이 낮음   //  결과가 0보다 클경우 -> 위쪽이 낮음

	tempX = _pixel2mm_y((double)(retRt[2].left - retRt[1].left));
	tempY = _micron2mm((double)(avrH[2] - avrH[1]));  
	angleTemp = atan2(tempY, tempX);
	horizontalGradi_bottom = _radian2degree(angleTemp);  //결과가 0보다 작을경우 -> 아래쪽이 낮음   //  결과가 0보다 클경우 -> 위쪽이 낮음


	if(abs(verticalGradi_left) < abs(verticalGradi_right))
		*retVerAngle = verticalGradi_left;
	else
		*retVerAngle = verticalGradi_right;

	if(abs(horizontalGradi_top) < abs(horizontalGradi_bottom))
		*retHorAngle = horizontalGradi_top;
	else
		*retHorAngle = horizontalGradi_bottom;


//////////////////////////////////////////////////////////////////////////
	/*delete userClipZmapData;
	delete retRt;
	delete avrH;*/
	g_pMManager->pem_delete(userClipZmapData, true);
	g_pMManager->pem_delete(retRt, true);
	g_pMManager->pem_delete(avrH, true);

	return retGradi;
}

float CProcMil_Mount::CalcGradient2(float* zmapRoiData, int roiSizeX, int roiSizeY, CRect bodyRect, double bodyAngle, float hThresh, double* retVerAngle, double* retHorAngle)
{
	float retAvrH = 0;
	
	//CRect* retRt = new CRect[4];
	CRect* retRt = g_pMManager->pem_new<CRect>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(retRt, 0, sizeof(CRect) * 4);
	CalcSampleArea(bodyRect, retRt);

	Im::PIL_ID milZmapDataTemp = M_NULL;
	milZmapDataTemp = AllocBuff_float(roiSizeX, roiSizeY);
	Im::Buf::Put(milZmapDataTemp, zmapRoiData);

	Im::PIL_ID milZmapData = M_NULL;
	if(abs(bodyAngle) >= 1)
	{
		milZmapData = AllocBuff_float(roiSizeX, roiSizeY, 0);
		MimRotate(milZmapDataTemp, milZmapData, (MIL_DOUBLE)bodyAngle * -1, M_DEFAULT , M_DEFAULT , M_DEFAULT, M_DEFAULT, M_BILINEAR);
	}
	else
	{
		milZmapData = milZmapDataTemp;
	}
	//Im::Buf::Export(_T("d:\\testimage\\workimage\\rotateZmap.bmp"), M_BMP, milZmapData);

	///////////////////////////////////////////////////////////////////////////////
	Im::PIL_ID milImage = M_NULL;
	milImage = AllocBuff(roiSizeX, roiSizeY, 0);
	Im::Gra::Rect(M_DEFAULT, milImage, bodyRect.left, bodyRect.top, bodyRect.right, bodyRect.bottom);


	for(int i = 0; i < 4; i++)
	{
		Im::Gra::Rect(M_DEFAULT, milImage, retRt[i].left, retRt[i].top, retRt[i].right, retRt[i].bottom);
	}
	

	//SaveWorkImg(milImage, _T("rect.bmp"));
	FreeMilImageBuff(&milImage);
	///////////////////////////////////////////////////////////////////////////////


	//float* avrH = new float[4]; //0: topLeft,  1: bottomLeft,  2: bottomRight,  3: topRight
	float* avrH = g_pMManager->pem_new<float>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(avrH, 0, sizeof(float) * 4);

	int cx = 0;
	int cy = 0;
	int w = retRt[0].right - retRt[0].left;
	int h = retRt[0].bottom - retRt[0].top;
	int area = w * h;

	if(area <= 0)
	{
		*retVerAngle = -1;
		*retHorAngle = -1;
		return -1;
	}

	//float* userClipZmapData = new float[area];
	float* userClipZmapData = g_pMManager->pem_new<float>(true, area, (PCHAR)__FUNCTION__, __LINE__);
	//memset(userClipZmapData, 0, sizeof(float) * area);	
		
	Im::PIL_ID milClipZmapData = M_NULL;
	double totalSum = 0;
	for(int i = 0; i < 4; i++)
	{		
		cx = (int)(retRt[i].left + (retRt[i].Width() / 2.0));
		cy = (int)(retRt[i].top + (retRt[i].Height() / 2.0));

		milClipZmapData = AllocClipBuff_float(milZmapData, cx, cy, w, h);
		//milClipZmapData = AllocChildImage(milZmapData, retRt[i].left, retRt[i].top, w, h);
		Im::Buf::Get(milClipZmapData, userClipZmapData);
		FreeMilImageBuff(&milClipZmapData);

		double sum = 0.0;
		int cnt = 0;
		int index = 0;
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{
				index = (w * y) + x;

				if(userClipZmapData[index] >= hThresh)
				{
					sum += userClipZmapData[index];
					cnt++;
				}
			}
		}
		avrH[i] = (float)(sum / cnt);
		memset(userClipZmapData, 0, sizeof(float) * area);

		totalSum += avrH[i];
	}



	//topLeft(0) <-> bottomRight(2)
	double diffH1 = 0.0;
	diffH1 = abs(avrH[0] - avrH[2]);

	//topRight(3) <-> bottomLeft(1)
	double diffH2 = 0.0;
	diffH2 = abs(avrH[3] - avrH[1]);



	if(diffH1 < diffH2)
	{
		*retVerAngle = diffH1;
		*retHorAngle = diffH1;
	}
	else
	{
		*retVerAngle = diffH2;
		*retHorAngle = diffH2;
	}



	retAvrH = (float)(totalSum / 4.0);

	
	FreeMilImageBuff(&milZmapDataTemp);

	if(abs(bodyAngle) >= 1)
	{
		FreeMilImageBuff(&milZmapData);	
	}

	/*delete userClipZmapData;
	delete retRt;
	delete avrH;*/
	g_pMManager->pem_delete(userClipZmapData, true);
	g_pMManager->pem_delete(retRt, true);
	g_pMManager->pem_delete(avrH, true);

	return retAvrH;
}


float CProcMil_Mount::CalcGradient3(float* zmapRoiData, int roiSizeX, int roiSizeY, CRect bodyRect, double bodyAngle, float hThresh, double* retGradient)
{
	float retAvrH = 0;

	//CRect* retRt = new CRect[4];
	CRect* retRt = g_pMManager->pem_new<CRect>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(retRt, 0, sizeof(CRect) * 4);
	CalcSampleArea(bodyRect, retRt);  //0: topLeft,  1: bottomLeft,  2: bottomRight,  3: topRight

	Im::PIL_ID milZmapDataTemp = M_NULL;
	milZmapDataTemp = AllocBuff_float(roiSizeX, roiSizeY);
	Im::Buf::Put(milZmapDataTemp, zmapRoiData);

	Im::PIL_ID milZmapData = M_NULL;
	milZmapData = AllocBuff_float(roiSizeX, roiSizeY, 0);
	MimRotate(milZmapDataTemp, milZmapData, (MIL_DOUBLE)bodyAngle * -1, M_DEFAULT , M_DEFAULT , M_DEFAULT, M_DEFAULT, M_BILINEAR);

	//float* procZmap = new float[roiSizeX * roiSizeY];
	float* procZmap = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	Im::Buf::Get(milZmapData ,procZmap);
	//////////////////////////////////////////////////////////////////////////

	int index = 0;
	float data = 0;
	double sum[4] = {0,};
	int count[4] = {0,};
	float avrH[4] = {0,};
	for(int y = 0; y < roiSizeY; y++)
	{
		for(int x = 0; x < roiSizeX; x++)
		{
			index = (roiSizeX * y) + x;
			data = procZmap[index];

			if(data >= hThresh)
			{
				if(y >= retRt[0].top && y <= retRt[0].bottom && x >= retRt[0].left && x <= retRt[0].right)//topLeft
				{
					sum[0] += data;
					count[0]++;
				}

				if(y >= retRt[1].top && y <= retRt[1].bottom && x >= retRt[1].left && x <= retRt[1].right)//bottomLeft
				{
					sum[1] += data;
					count[1]++;
				}

				if(y >= retRt[2].top && y <= retRt[2].bottom && x >= retRt[2].left && x <= retRt[2].right)//bottomRight
				{
					sum[2] += data;
					count[2]++;
				}

				if(y >= retRt[3].top && y <= retRt[3].bottom && x >= retRt[3].left && x <= retRt[3].right)//topRight
				{
					sum[3] += data;
					count[3]++;
				}
			}
		}
	}

	double totalSum = 0;
	for(int i = 0; i < 4; i++)
	{
		avrH[i] = (float)(sum[i] / count[i]);
		totalSum += avrH[i];
	}
	retAvrH = (float)(totalSum / 4.0);  
	//////////////////////////////////////////////////////////////////////////

	double diffH1 = 0.0;
	diffH1 = abs(avrH[0] - avrH[2]);

	//topRight(3) <-> bottomLeft(1)
	double diffH2 = 0.0;
	diffH2 = abs(avrH[3] - avrH[1]);


	if(diffH1 < diffH2)
		*retGradient = diffH1;
	else
		*retGradient = diffH2;

	//////////////////////////////////////////////////////////////////////////

	FreeMilImageBuff(&milZmapDataTemp);
	FreeMilImageBuff(&milZmapData);	
	/*delete procZmap;
	delete retRt;*/
	g_pMManager->pem_delete(procZmap, true);
	g_pMManager->pem_delete(retRt, true);

	return retAvrH;
}


float CProcMil_Mount::CalcGradient4(float* zmapRoiData, int roiSizeX, int roiSizeY, int bodyW, int bodyH, CPoint* nodalPoint, float hThresh, float hThresh2, float dftVal, double bodyAngle, double* retTiltAngle, 
		bool bIsUserDefCop, double* cxCOP, double* cyCOP, double* widthCOP, double* heightCOP)
{
	float retAvrH = 0;

	int ptX = 0;
	int ptY = 0;
	
	// CalcHeight2와 CalcGradient, CalcGradient2, CalcGradient3는 사용되지 않고 있어서 이 부분을 구현하지 않습니다. 차후에 사용한다면 붙여서 사용하시기 바랍니다.
	CRect clipRoi[4];
	if(bIsUserDefCop)
	{
		CalcSampleArea_UserDefCop(nodalPoint, bodyW, bodyH, clipRoi, &ptX, &ptY, 
			cxCOP, cyCOP, widthCOP, heightCOP);
	}
	else
	{
		CalcSampleArea(nodalPoint, bodyW, bodyH, clipRoi, &ptX, &ptY);
	}

	///////////////////////////////////////////////////////////////////////////////
	//test image save
/*	Im::PIL_ID milImage = M_NULL;
	milImage = AllocBuff(roiSizeX, roiSizeY, 0);

	Im::Gra::Line(M_DEFAULT, milImage, (MIL_DOUBLE)nodalPoint[0].x, (MIL_DOUBLE)nodalPoint[0].y, (MIL_DOUBLE)nodalPoint[1].x, (MIL_DOUBLE)nodalPoint[1].y);
	Im::Gra::Line(M_DEFAULT, milImage, (MIL_DOUBLE)nodalPoint[1].x, (MIL_DOUBLE)nodalPoint[1].y, (MIL_DOUBLE)nodalPoint[2].x, (MIL_DOUBLE)nodalPoint[2].y);
	Im::Gra::Line(M_DEFAULT, milImage, (MIL_DOUBLE)nodalPoint[2].x, (MIL_DOUBLE)nodalPoint[2].y, (MIL_DOUBLE)nodalPoint[3].x, (MIL_DOUBLE)nodalPoint[3].y);
	Im::Gra::Line(M_DEFAULT, milImage, (MIL_DOUBLE)nodalPoint[3].x, (MIL_DOUBLE)nodalPoint[3].y, (MIL_DOUBLE)nodalPoint[0].x, (MIL_DOUBLE)nodalPoint[0].y);

	for(int i = 0; i < 4; i++)
	{
		Im::Gra::Rect(M_DEFAULT, milImage, clipRoi[i].left, clipRoi[i].top, clipRoi[i].right, clipRoi[i].bottom);
	}


	SaveWorkImg(milImage, _T("rect.bmp"));
	Im::Buf::Free(milImage);
	milImage = M_NULL;*/
	///////////////////////////////////////////////////////////////////////////////
	//huj 2014/02/07
	bool rotateFlag = false;
	if(abs(bodyAngle) <= 360.0 && abs(bodyAngle) >= 10.0)
		rotateFlag = true;

	float* zmapRotateData = NULL;
	int limitSizeX = roiSizeX;
	int limitSizeY = roiSizeY;
	int dx = 0;
	int dy = 0;
	if(rotateFlag)
	{
		int bdryLeft = nodalPoint[0].x;
		int bdryRight = nodalPoint[0].x;
		int bdryTop = nodalPoint[0].y;
		int bdryBottom = nodalPoint[0].y;
		int margin = 10;
		for(int i = 0; i < 4; i++)
		{
			if(nodalPoint[i].x < bdryLeft)
				bdryLeft = nodalPoint[i].x - margin;

			if(nodalPoint[i].x > bdryRight)
				bdryRight = nodalPoint[i].x + margin;

			if(nodalPoint[i].y < bdryTop)
				bdryTop = nodalPoint[i].y - margin;

			if(nodalPoint[i].y > bdryBottom)
				bdryBottom = nodalPoint[i].y + margin;
		}

		int bdryW = bdryRight - bdryLeft;
		int bdryH = bdryBottom - bdryTop;
		//float* bdryClipZamp = new float[bdryW * bdryH];
		float* bdryClipZamp = g_pMManager->pem_new<float>(true, bdryW * bdryH, (PCHAR)__FUNCTION__, __LINE__);
		GetCropZmap(zmapRoiData, bdryClipZamp, roiSizeX, roiSizeY, ptX, ptY, bdryW, bdryH);

		int sizeX = 0;
		int sizeY = 0;
		RotateZmap_ipp2(bdryClipZamp, &zmapRotateData, bdryW, bdryH, bodyAngle * -1, &sizeX, &sizeY);

		dx = (int)(ptX - (sizeX / 2.0));
		dy = (int)(ptY - (sizeY / 2.0));

		//////////////////////////////////////////////////////////////////////////
		//test image save
		Im::PIL_ID milImage = M_NULL;
		milImage = AllocBuff_float(sizeX, sizeY, 0);
		Im::Buf::Put(milImage, zmapRotateData);

		for(int i = 0; i < 4; i++)
		{
			Im::Gra::Rect(M_DEFAULT, milImage, clipRoi[i].left - dx, clipRoi[i].top - dy, clipRoi[i].right - dx, clipRoi[i].bottom - dy);
		}

		SaveWorkImg_float(milImage, _T("recttest.bmp"));
		Im::Buf::Free(milImage);
		milImage = M_NULL;
		//////////////////////////////////////////////////////////////////////////

		//delete bdryClipZamp;
		g_pMManager->pem_delete(bdryClipZamp, true);
		
		limitSizeX = sizeX;
		limitSizeY = sizeY;
	}
	else
	{
		zmapRotateData = zmapRoiData;
	}
	//////////////////////////////////////////////////////////////////////////
	float avrH[4] = {0,};
	double sum = 0;
	int cnt = 0;
	double totalSum = 0;

	int index = 0;
	float data = 0;
	for(int i = 0; i < 4; i++)
	{
		CRect roi = clipRoi[i];

		sum = 0;
		cnt = 0;
		int stX = 0;
		int edX = 0;
		int stY = 0;
		int edY = 0;

		stY = (int)roi.top - dy;
		edY = (int)(stY + roi.Height());

		stX = (int)roi.left - dx;
		edX = (int)(stX + roi.Width());

		for(int y = stY; y < edY; y++)
		{
			for(int x = stX; x < edX; x++)
			{
				if(x < limitSizeX && y < limitSizeY)					
				{
					index = (limitSizeX * y) + x;
					data = zmapRotateData[index];

					//huj 2013/12/12
					if(hThresh2 > 0)// 상한선... 넘으면 티칭 높이로 대체...
					{
						if(data >= hThresh2)
						{
							data = dftVal;
						}
					}

					if(data >= hThresh)
					{
						sum += data;
						cnt++;
					}
				}
			}
		}
	
			if(cnt != 0)
				avrH[i] = (float)(sum / cnt);
			else
				avrH[i] = 0;


			totalSum += avrH[i];
	}

	retAvrH = (float)(totalSum / 4.0);  

	
	//////////////////////////////////////////////////////////////////////////

	double diffH1 = 0.0;
	diffH1 = abs(avrH[0] - avrH[2]);

	//topRight(3) <-> bottomLeft(1)
	double diffH2 = 0.0;
	diffH2 = abs(avrH[3] - avrH[1]);


	if(diffH1 < diffH2)
		*retTiltAngle = diffH1;
	else
		*retTiltAngle = diffH2;

	//////////////////////////////////////////////////////////////////////////

	if(rotateFlag)
	{
		if(zmapRotateData != NULL)
			//delete zmapRotateData;
			g_pMManager->pem_delete(zmapRotateData, true);
	}

	return retAvrH;
}

#if 0

float CProcMil_Mount::CalcGradient4(float* zmapRoiData, int roiSizeX, int roiSizeY, int bodyW, int bodyH, CPoint* nodalPoint, float hThresh, float hThresh2, float dftVal, double* retTiltAngle)
{
	float retAvrH = 0;
	
	CRect clipRoi[4];
	CalcSampleArea(nodalPoint, bodyW, bodyH, clipRoi);

	///////////////////////////////////////////////////////////////////////////////
/*	Im::PIL_ID milImage = M_NULL;
	milImage = AllocBuff(roiSizeX, roiSizeY, 0);

	Im::Gra::Line(M_DEFAULT, milImage, (MIL_DOUBLE)nodalPoint[0].x, (MIL_DOUBLE)nodalPoint[0].y, (MIL_DOUBLE)nodalPoint[1].x, (MIL_DOUBLE)nodalPoint[1].y);
	Im::Gra::Line(M_DEFAULT, milImage, (MIL_DOUBLE)nodalPoint[1].x, (MIL_DOUBLE)nodalPoint[1].y, (MIL_DOUBLE)nodalPoint[2].x, (MIL_DOUBLE)nodalPoint[2].y);
	Im::Gra::Line(M_DEFAULT, milImage, (MIL_DOUBLE)nodalPoint[2].x, (MIL_DOUBLE)nodalPoint[2].y, (MIL_DOUBLE)nodalPoint[3].x, (MIL_DOUBLE)nodalPoint[3].y);
	Im::Gra::Line(M_DEFAULT, milImage, (MIL_DOUBLE)nodalPoint[3].x, (MIL_DOUBLE)nodalPoint[3].y, (MIL_DOUBLE)nodalPoint[0].x, (MIL_DOUBLE)nodalPoint[0].y);

	for(int i = 0; i < 4; i++)
	{
		Im::Gra::Rect(M_DEFAULT, milImage, clipRoi[i].left, clipRoi[i].top, clipRoi[i].right, clipRoi[i].bottom);
	}


	SaveWorkImg(milImage, _T("rect.bmp"));
	Im::Buf::Free(milImage);
	milImage = M_NULL;*/
	///////////////////////////////////////////////////////////////////////////////

	float avrH[4] = {0,};
	double sum = 0;
	int cnt = 0;
	double totalSum = 0;

	int index = 0;
	float data = 0;
	for(int i = 0; i < 4; i++)
	{
		CRect roi = clipRoi[i];

		sum = 0;
		cnt = 0;
		int stX = 0;
		int edX = 0;
		int stY = 0;
		int edY = 0;

		stY = (int)roi.top;
		edY = (int)(roi.top + roi.Height());

		stX = (int)roi.left;
		edX = (int)(roi.left + roi.Width());

		for(int y = stY; y < edY; y++)
		{
			for(int x = stX; x < edX; x++)
			{
				if(x < roiSizeX && y < roiSizeY)					
				{
					index = (roiSizeX * y) + x;
					data = zmapRoiData[index];

					//huj 2013/12/12
					if(hThresh2 > 0)// 상한선... 넘으면 티칭 높이로 대체...
					{
						if(data >= hThresh2)
						{
							data = dftVal;
						}
					}

					if(data >= hThresh)
					{
						sum += data;
						cnt++;
					}
				}
			}
		}
	
			if(cnt != 0)
				avrH[i] = (float)(sum / cnt);
			else
				avrH[i] = 0;


			totalSum += avrH[i];
	}

	retAvrH = (float)(totalSum / 4.0);  

	
	//////////////////////////////////////////////////////////////////////////

	double diffH1 = 0.0;
	diffH1 = abs(avrH[0] - avrH[2]);

	//topRight(3) <-> bottomLeft(1)
	double diffH2 = 0.0;
	diffH2 = abs(avrH[3] - avrH[1]);


	if(diffH1 < diffH2)
		*retTiltAngle = diffH1;
	else
		*retTiltAngle = diffH2;

	//////////////////////////////////////////////////////////////////////////


	return retAvrH;
}
#endif

void CProcMil_Mount::CalcSampleArea_UserDefCop(CPoint* nodalPoint, int bodyW, int bodyH, CRect* retSampleRect, int* retCx, int* retCy
	, double* cxCOP, double* cyCOP, double* widthCOP, double* heightCOP)
{
	//****************** 필요한 부분 start ************************//
	double sumX = 0;
	double sumY = 0;
	int ptX = 0;
	int ptY = 0;

	sumX = nodalPoint[0].x + nodalPoint[1].x + nodalPoint[2].x + nodalPoint[3].x;
	sumY = nodalPoint[0].y + nodalPoint[1].y + nodalPoint[2].y + nodalPoint[3].y;


	ptX = (int)(sumX / 4.0);
	ptY = (int)(sumY / 4.0);
	//****************** 필요한 부분 end ************************//

	int cx[4] = {0,};
	int cy[4] = {0,};

	cx[0] = cxCOP[0];
	cy[0] = cyCOP[0];

	cx[1] = cxCOP[1];
	cy[1] = cyCOP[1];

	cx[2] = cxCOP[2];
	cy[2] = cyCOP[2];

	cx[3] = cxCOP[3];
	cy[3] = cyCOP[3];

	int stX = 0;
	int stY = 0;
	CRect clipRoi[4];

	stX = (int)(cx[0] - (widthCOP[0] / 2.0));
	stY = (int)(cy[0] - (heightCOP[0] / 2.0));
	clipRoi[0].SetRect(stX, stY, stX + widthCOP[0] , stY + heightCOP[0]);

	stX = (int)(cx[1] - (widthCOP[1] / 2.0));
	stY = (int)(cy[1] - (heightCOP[1] / 2.0));
	clipRoi[1].SetRect(stX, stY, stX + widthCOP[1] , stY + heightCOP[1]);

	stX = (int)(cx[2] - (widthCOP[2] / 2.0));
	stY = (int)(cy[2] - (heightCOP[2] / 2.0));
	clipRoi[2].SetRect(stX, stY, stX + widthCOP[2] , stY + heightCOP[2]);

	stX = (int)(cx[3] - (widthCOP[3] / 2.0));
	stY = (int)(cy[3] - (heightCOP[3] / 2.0));
	clipRoi[3].SetRect(stX, stY, stX + widthCOP[3] , stY + heightCOP[3]);

	memcpy_s(retSampleRect, sizeof(CRect) * 4, clipRoi, sizeof(CRect) * 4);
	
	// **************************** 필요한 부분 start **************************** //
	if(retCx != NULL)
		*retCx = ptX;

	if(retCy != NULL)
		*retCy = ptY;
	// **************************** 필요한 부분 end **************************** //
}

void CProcMil_Mount::CalcSampleArea(CPoint* nodalPoint, int bodyW, int bodyH, CRect* retSampleRect, int* retCx, int* retCy)
{
	double sizeRatio_Long = 0.15;
	double sizeRatio_Short = 0.15;

	int clipW = 0;
	int clipH = 0;

	if(bodyW > bodyH)
	{
		clipW = (int)(bodyW * sizeRatio_Long);
		clipH = (int)(bodyH * sizeRatio_Short);

	}
	else if(bodyW == bodyH)
	{
		clipW = (int)(bodyW * sizeRatio_Short);
		clipH = (int)(bodyH * sizeRatio_Short);

	}
	else
	{
		clipW = (int)(bodyW * sizeRatio_Short);
		clipH = (int)(bodyH * sizeRatio_Long);

	}


	double sumX = 0;
	double sumY = 0;
	int ptX = 0;
	int ptY = 0;

	sumX = nodalPoint[0].x + nodalPoint[1].x + nodalPoint[2].x + nodalPoint[3].x;
	sumY = nodalPoint[0].y + nodalPoint[1].y + nodalPoint[2].y + nodalPoint[3].y;


	ptX = (int)(sumX / 4.0);
	ptY = (int)(sumY / 4.0);


	int marginX = (int)(bodyW / 4.0);
	int marginY = (int)(bodyH / 4.0);

	int cx[4] = {0,};
	int cy[4] = {0,};

	cx[0] = (ptX - marginX);
	cy[0] = (ptY - marginY);

	cx[1] = (ptX - marginX);
	cy[1] = (ptY + marginY);

	cx[2] = (ptX + marginX);
	cy[2] = (ptY + marginY);

	cx[3] = (ptX + marginX);
	cy[3] = (ptY - marginY);

	int stX = 0;
	int stY = 0;
	CRect clipRoi[4];

	stX = (int)(cx[0] - (clipW / 2.0));
	stY = (int)(cy[0] - (clipH / 2.0));
	clipRoi[0].SetRect(stX, stY, stX + clipW , stY + clipH);

	stX = (int)(cx[1] - (clipW / 2.0));
	stY = (int)(cy[1] - (clipH / 2.0));
	clipRoi[1].SetRect(stX, stY, stX + clipW , stY + clipH);

	stX = (int)(cx[2] - (clipW / 2.0));
	stY = (int)(cy[2] - (clipH / 2.0));
	clipRoi[2].SetRect(stX, stY, stX + clipW , stY + clipH);

	stX = (int)(cx[3] - (clipW / 2.0));
	stY = (int)(cy[3] - (clipH / 2.0));
	clipRoi[3].SetRect(stX, stY, stX + clipW , stY + clipH);

	memcpy_s(retSampleRect, sizeof(CRect) * 4, clipRoi, sizeof(CRect) * 4);
	

	if(retCx != NULL)
		*retCx = ptX;

	if(retCy != NULL)
		*retCy = ptY;

//////////////////////////////////////////////////////////////////////////
/*	double sizeRatio_Long = 0.3;//0.15;
	double sizeRatio_Short = 0.3;

	double marginRatio_Long = 1.2;
	double marginRatio_Short = 0.4;
	int marginX = 0;
	int marginY = 0;

	int clipW = 0;
	int clipH = 0;

	if(bodyW > bodyH)
	{
		clipW = (int)(bodyW * sizeRatio_Long);
		clipH = (int)(bodyH * sizeRatio_Short);

		marginX = (int)(clipW * marginRatio_Long);
		marginY = (int)(clipH * marginRatio_Short);
	}
	else if(bodyW == bodyH)
	{
		clipW = (int)(bodyW * sizeRatio_Short);
		clipH = (int)(bodyH * sizeRatio_Short);

		marginX = (int)(clipW * marginRatio_Short);
		marginY = (int)(clipH * marginRatio_Short);
	}
	else
	{
		clipW = (int)(bodyW * sizeRatio_Short);
		clipH = (int)(bodyH * sizeRatio_Long);

		marginX = (int)(clipW * marginRatio_Short);
		marginY = (int)(clipH * marginRatio_Long);
	}

	CRect clipRoi[4];
	clipRoi[0].SetRect(nodalPoint[0].x + marginX, nodalPoint[0].y + marginY, (nodalPoint[0].x + clipW) + marginX, (nodalPoint[0].y + clipH) + marginY);
	clipRoi[1].SetRect(nodalPoint[1].x + marginX, (nodalPoint[1].y - clipH) - marginY, (nodalPoint[1].x + clipW) + marginX, nodalPoint[1].y - marginY);
	clipRoi[2].SetRect((nodalPoint[2].x - clipW) - marginX, (nodalPoint[2].y - clipH) - marginY, nodalPoint[2].x - marginX, nodalPoint[2].y - marginY);
	clipRoi[3].SetRect((nodalPoint[3].x - clipW) - marginX, nodalPoint[3].y + marginY, nodalPoint[3].x - marginX, (nodalPoint[3].y + clipH) + marginY);

	memcpy_s(retSampleRect, sizeof(CRect) * 4, clipRoi, sizeof(CRect) * 4);*/
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//pattern
//int CProcMil_Mount::AllocPatModel(void* milSrc, int cx, int cy, int width, int height, double rotateAngle, int rotateW, int rotateH)
//{
//	int ret  = eMNT_SUCCESS;
//
//	int stX = 0;
//	int stY = 0;
//
//	stX = (int)(cx - (width / 2.0));
//	stY = (int)(cy - (height / 2.0));
//
//	DeleteModelBuf();
//	Im::PIL_ID rst = MpatAllocModel(*m_milSys, (Im::PIL_ID)milSrc, (MIL_INT)stX, (MIL_INT)stY, (MIL_INT)width, (MIL_INT)height, M_NORMALIZED, &m_milPatModel);
//
//	if(rotateAngle > 0)
//	{
//		Im::PIL_ID milClip = AllocClipBuff((Im::PIL_ID)milSrc, cx, cy, width, height);
//		Im::PIL_ID rotateImg = AllocRotateBuff(milClip, rotateAngle);
//
//		Im::PIL_ID rst2 = MpatAllocModel(*m_milSys, rotateImg, (MIL_INT)0, (MIL_INT)0, (MIL_INT)rotateW, (MIL_INT)rotateH, M_NORMALIZED, &m_milPatModel_Rotate);
//
//		FreeMilImageBuff(&milClip);
//		FreeMilImageBuff(&rotateImg);
//
//		if(rst2 == M_NULL)
//			ret = eMNT_FAIL;
//	}
//
//	if(rst == M_NULL)
//		ret = eMNT_FAIL;	
//
//	return ret;
//}

//int CProcMil_Mount::AllocPatModel(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH)
//{
//	int ret  = eMNT_SUCCESS;
//
//	Im::PIL_ID milSrc = AllocBuff(width, height);
//	Im::Buf::Put(milSrc, userSrc);
//
//	DeleteModelBuf();
//	Im::PIL_ID rst = MpatAllocModel(*m_milSys, (Im::PIL_ID)milSrc, (MIL_INT)0, (MIL_INT)0, (MIL_INT)width, (MIL_INT)height, M_NORMALIZED, &m_milPatModel);
//
//	if(rotateAngle > 0)
//	{
//		//Im::PIL_ID rotateImg = AllocRotateBuff(milSrc, rotateAngle);
//		//Im::PIL_ID rst2 = MpatAllocModel(*m_milSys, rotateImg, (MIL_INT)0, (MIL_INT)0, (MIL_INT)rotateW, (MIL_INT)rotateH, M_NORMALIZED, &m_milPatModel_Rotate);
//		Im::PIL_ID rst2 = MpatAllocRotatedModel(*m_milSys, m_milPatModel, rotateAngle, M_BILINEAR, M_NORMALIZED, &m_milPatModel_Rotate);
//
//		//FreeMilImageBuff(&rotateImg);
//
//		if(rst2 == M_NULL)
//			ret = eMNT_FAIL;
//	}
//
//	if(rst == M_NULL)
//		ret = eMNT_FAIL;	
//
//
//	FreeMilImageBuff(&milSrc);
//
//	return ret;
//}

//void CProcMil_Mount::DeleteModelBuf()
//{
//	if(m_milPatModel != M_NULL)
//	{
//		MpatFree(m_milPatModel);
//		m_milPatModel = M_NULL;
//	}
//
//	if(m_milPatModel_Rotate != M_NULL)
//	{
//		MpatFree(m_milPatModel_Rotate);
//		m_milPatModel_Rotate = M_NULL;
//	}
//}

//int CProcMil_Mount::WritePatternModel(void* milSrc, int cx, int cy, int width, int height, double rotateAngle, CString strPath)
//{
//	int ret = eMNT_SUCCESS;
//	int stX = 0;
//	int stY = 0;
//
//	if(m_milPatModel == M_NULL)
//		return eMNT_FAIL;
//
//	stX = (int)(cx - (width / 2.0));
//	stY = (int)(cy - (height / 2.0));
//
//	CString strImagePath = _T("");
//	strImagePath = strPath + _T(".tif");
//
//	if(rotateAngle == 0)
//		ModelSave(m_milPatModel, strPath);
//	else
//		ModelSave(m_milPatModel_Rotate, strPath);
//
//	SaveClipImg((Im::PIL_ID)milSrc, 1, cx, cy, width, height, strImagePath);
//
//	return ret;
//}

//int CProcMil_Mount::WritePatternModel(UCHAR* userSrc, int width, int height, double rotateAngle, CString strPath)
//{
//	int ret = eMNT_SUCCESS;
//
//	if(userSrc == NULL || width < 0 || height < 0)
//		return eMNT_FAIL;
//
//	Im::PIL_ID milSrc = AllocBuff(width, height);
//	Im::Buf::Put(milSrc, userSrc);
//	if(m_milPatModel == M_NULL)
//		return eMNT_FAIL;
//
//	CString strImagePath = _T("");
//	strImagePath = strPath + _T(".tif");
//
//	if(rotateAngle == 0)
//	{ 
//		if(m_milPatModel != M_NULL)
//		{
//			ModelSave(m_milPatModel, strPath);
//			SaveBuff(milSrc, strImagePath);
//		}
//		else
//		{
//			ret = eMNT_FAIL;
//		}
//	}
//	else
//	{
//		if(m_milPatModel_Rotate != M_NULL)
//		{
//			Im::PIL_ID rotateImg = AllocRotateBuff(milSrc, rotateAngle);
//			ModelSave(m_milPatModel_Rotate, strPath);
//			SaveBuff(rotateImg, strImagePath);
//
//			FreeMilImageBuff(&rotateImg);
//		}
//		else
//		{
//			ret = eMNT_FAIL;
//		}
//	}
//
//
//
//
//	FreeMilImageBuff(&milSrc);
//	return ret;
//}


//void CProcMil_Mount::ModelSave(Im::PIL_ID milModel, CString strPath)
//{
//	if(milModel == M_NULL)
//		return;
//
//	CString strModelPath = _T("");
//	strModelPath = strPath + _T(".mod");
//
//	MpatSave(strModelPath, milModel);
//
//
//}

//int CProcMil_Mount::ModelLoad(CString strPath)
//{
//	int ret = eMNT_SUCCESS;
//	DeleteModelBuf();
//
//	CString strModelPath = _T("");
//	strModelPath = strPath + _T(".mod");
//
//
//	Im::PIL_ID temp = MpatRestore(*m_milSys, strModelPath, &m_milPatModel);
//
//	if(temp == M_NULL)
//		ret = eMNT_FAIL;
//
//	return ret;
//}


//int CProcMil_Mount::SearchPattern(UCHAR* userSrc, int cx, int cy, int width, int height)
//{
//	Im::PIL_ID milTemp = M_NULL;
//
//	SetAccuracy(PAT_SPEED, PAT_ACCURACY);
//
//	milTemp = AllocBuff(width, height);
//	Im::Buf::Put(milTemp, userSrc);
//
//	SaveWorkImg(milTemp, _T("mountPatSrc.bmp"));
//
//	MpatPreprocModel(milTemp, m_milPatModel, M_DEFAULT);
//
//	MpatFindModel(milTemp, m_milPatModel, m_milPatResult);
//
//	MIL_INT count = MpatGetNumber(m_milPatResult, M_NULL);
//
//
//	FreeMilImageBuff(&milTemp);
//	return (int)count;
//}

//int CProcMil_Mount::GetPatResult(double* retScore, double* retAngle, double* retPosX, double* retPosY)
//{
//	int ret = eMNT_SUCCESS;
//	MIL_INT count = MpatGetNumber(m_milPatResult, M_NULL);
//
//	if((int)count == 1)
//	{
//		double posX = 0;
//		double posY = 0;
//		double angle = 0;
//		double score = 0;
//
//		MpatGetResult(m_milPatResult, M_POSITION_X, &posX);  //cX
//		MpatGetResult(m_milPatResult, M_POSITION_Y, &posY);  //cY
//		MpatGetResult(m_milPatResult, M_ANGLE, &angle);
//		MpatGetResult(m_milPatResult, M_SCORE, &score);
//
//		*retScore = score;
//		*retAngle = angle;
//		*retPosX = posX;
//		*retPosY = posY;
//	}
//	else if((int)count > 1)
//	{
//		double*  posX = new double[(int)count];
//		double*  posY = new double[(int)count];
//		double*  angle = new double[(int)count];
//		double*  score = new double[(int)count];
//
//		MpatGetResult(m_milPatResult, M_POSITION_X, posX);
//		MpatGetResult(m_milPatResult, M_POSITION_Y, posY);
//		MpatGetResult(m_milPatResult, M_ANGLE, angle);
//		MpatGetResult(m_milPatResult, M_SCORE, score);
//
//		int maxIndex = 0;
//		double maxVal = 0;
//		for(int i = 0; i < (int)count; i++)
//		{
//			if(maxVal < score[i])
//			{
//				maxVal = score[i];
//				maxIndex = i;
//			}
//		}
//
//		*retScore = score[maxIndex];
//		*retAngle = angle[maxIndex];
//		*retPosX = posX[maxIndex];
//		*retPosY = posY[maxIndex];
//
//		delete posX;
//		delete posY;
//		delete angle;
//		delete score;
//	}
//	else
//	{
//		*retScore = 0;
//		*retAngle = 0;
//		*retPosX = 0;
//		*retPosY = 0;
//
//		ret = eMNT_FAIL;
//	}
//
//	return ret;
//}

//void CProcMil_Mount::SetAccuracy(int speedFactor, int accuracy)
//{
//	MpatSetSpeed(m_milPatModel, speedFactor);
//	MpatSetAccuracy(m_milPatModel, accuracy);
//}

//void CProcMil_Mount::SetAngleMode(double posLimit, double negLimit, double accuracy)
//{
//	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_MODE, M_ENABLE);
//	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_INTERPOLATION_MODE, M_BILINEAR);
//	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_DELTA_NEG, negLimit);
//	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_DELTA_POS, posLimit);
//	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_ACCURACY, 0.5);
//}

//void CProcMil_Mount::SetSearchStartAngle(double angle)
//{
//	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE, angle); //search start angle
//}

void CProcMil_Mount::DrawBody(int sizeX, int sizeY, POINT* pt)
{
	Im::PIL_ID milTemp = M_NULL;
	milTemp = AllocBuff(sizeX, sizeY, 0);

	Im::Gra::Line(M_DEFAULT, milTemp, (MIL_DOUBLE)pt[0].x, (MIL_DOUBLE)pt[0].y, (MIL_DOUBLE)pt[1].x, (MIL_DOUBLE)pt[1].y);
	Im::Gra::Line(M_DEFAULT, milTemp, (MIL_DOUBLE)pt[1].x, (MIL_DOUBLE)pt[1].y, (MIL_DOUBLE)pt[2].x, (MIL_DOUBLE)pt[2].y);
	Im::Gra::Line(M_DEFAULT, milTemp, (MIL_DOUBLE)pt[2].x, (MIL_DOUBLE)pt[2].y, (MIL_DOUBLE)pt[3].x, (MIL_DOUBLE)pt[3].y);
	Im::Gra::Line(M_DEFAULT, milTemp, (MIL_DOUBLE)pt[3].x, (MIL_DOUBLE)pt[3].y, (MIL_DOUBLE)pt[0].x, (MIL_DOUBLE)pt[0].y);

	SaveWorkImg(milTemp, _T("body.bmp"));
	FreeMilImageBuff(&milTemp);
}


void CProcMil_Mount::SelectLine(UCHAR* img, int sizeX, int sizeY, POINT** retPt, int* retCnt)
{
	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, img);

	Im::PIL_ID milBlob = M_NULL;
	milBlob = AllocBuff(sizeX, sizeY);


	int blobCnt = CalcBlob(milSrc, 0, 0, FALSE, FALSE, milBlob);

	POINT* pt = NULL;
	double* blobSize = NULL;
	int ptCount = 0;
	if(blobCnt > 0)
	{
		//SelectBlob_MaxArea(blobCnt, milBlob);
		SaveWorkImg(milBlob, _T("selectLine.bmp"));
		//blobSize = new double[blobCnt];
		blobSize = g_pMManager->pem_new<double>(true, blobCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(blobSize, 0, sizeof(double) * blobCnt);

		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, blobSize);
		for(int i = 0; i < blobCnt; i++)
		{
			ptCount += (int)blobSize[i];
		}

		//pt = new POINT[ptCount];
		pt = g_pMManager->pem_new<POINT>(true, ptCount, (PCHAR)__FUNCTION__, __LINE__);
		memset(pt, 0, sizeof(POINT) * ptCount);

		UCHAR* userBlob = NULL;
		//userBlob = new UCHAR[sizeX * sizeY];
		userBlob = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
		//memset(userBlob, 0, sizeof(UCHAR) * sizeX * sizeY);
		Im::Buf::Get(milBlob, userBlob);
		//Im::Buf::Inquire(milBlob, M_HOST_ADDRESS, &userBlob);

		int index = 0;
		int ptIndex = 0;
		UCHAR data = 0;
		for(int y = 0; y < sizeY; y++)
		{
			for(int x = 0; x < sizeX; x++)
			{
				index = (y * sizeX) + x;
				data = userBlob[index];
				if(data > 0)
				{
					pt[ptIndex].x = x;
					pt[ptIndex].y = y;
					ptIndex++;
				}
			}
		}

		//delete userBlob;
		g_pMManager->pem_delete(userBlob, true);
	}
	
	if(pt != NULL)
	{
		//*retPt = new POINT[ptCount];
		*retPt = g_pMManager->pem_new<POINT>(true, ptCount, (PCHAR)__FUNCTION__, __LINE__);
		//memset(*retPt, 0, sizeof(POINT) * ptCount);

		memcpy_s(*retPt, sizeof(POINT) * ptCount, pt, sizeof(POINT) * ptCount);
	}

	*retCnt = ptCount;

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milBlob);
	
	if(pt != NULL)
		//delete pt;
		g_pMManager->pem_delete(pt, true);

	if(blobSize != NULL)
		//delete blobSize;
		g_pMManager->pem_delete(blobSize, true);
	
		 
}


void CProcMil_Mount::SaveIntImage(int* src, int sizeX, int sizeY, CString path)
{
	int addData = 100;
	int size = sizeX * sizeY;

	//UCHAR* srcTemp = new UCHAR[size];
	UCHAR* srcTemp = g_pMManager->pem_new<UCHAR>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	memset(srcTemp, 0, sizeof(UCHAR) * size);

	UCHAR data = 0;
	int index = 0;
	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			index = (y * sizeX) + x;

			if(src[index] > 255)
			{
				data = 255;
			}
			else
			{
				data = (UCHAR)src[index];
				if(data != 0)
					data += addData;
			}

			srcTemp[index] = data;
		}

	}

	Im::PIL_ID milTemp = M_NULL;
	milTemp = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milTemp, srcTemp);

	SaveWorkImg(milTemp, path);

	//delete srcTemp;
	g_pMManager->pem_delete(srcTemp, true);
	FreeMilImageBuff(&milTemp);
}


BOOL CProcMil_Mount::RemakeMountBinImg(UCHAR* src, UCHAR* mask, UCHAR* dst, int sizeX, int sizeY, CRect wndRect, CRect wndMarginRect)
{
	BOOL ret = TRUE;

	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, src);

	Im::PIL_ID milMask_Temp = M_NULL;
	milMask_Temp = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milMask_Temp, mask);

	int count = CalcBlob(milMask_Temp, 0, 0, TRUE, TRUE, NULL);

	CMilBlobResult* blobRst = new CMilBlobResult(count);
	g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

	GetBlobResult(blobRst);

    if(count > 0)
	{
		//blob이 mount window(margin을 뺀 영역)안에 들어있는지 확인.. 들어있는 blob만 남긴다.
		int index = -1;
		int indexTemp = 0;
		CPoint pt;
		//int* indexArr = new int[count];
		int* indexArr = g_pMManager->pem_new<int>(true, count, (PCHAR)__FUNCTION__, __LINE__);
		for(int i = 0; i < count; i++)
		{
			pt.x = (LONG)blobRst->cx[i];
			pt.y = (LONG)blobRst->cy[i];
			if(PtInRect(wndRect, pt))
			{
				indexArr[indexTemp] = i;
				indexTemp++;
				index = i;
			}
		}
		
		//mount window 안에 blob이 한개 이상 존재 할경우 그중 가장 큰 blob만을 남긴다.
		if(indexTemp > 1)
		{
			double max = blobRst->area[indexArr[0]];
			for( int i = 1; i < indexTemp ; i ++)
			{
				if( max < blobRst->area[indexArr[i]])
				{
					max = blobRst->area[indexArr[i]];
					index = indexArr[i];
				}
			}
		}

		//하나 남은 blob을 그린다.
		if(index >= 0)
		{
			Im::PIL_ID milMask = M_NULL;
			milMask = AllocBuff(sizeX, sizeY, 0);

			double marginX = 0;
			double marginY = 0;
			BOOL chipPosition = FALSE; // true  I   false  -
			int noiseKind = 0; //1: solder , 2: body
			if(wndRect.Width() < wndRect.Height())
				chipPosition = TRUE;				
			else
				chipPosition = FALSE;
				
			
			//////////////////////////////////////////////////////////////////////////
			CPoint wndCog = wndMarginRect.CenterPoint();
			int noiseCx = (int)(blobRst->cx[index]);
			int noiseCy = (int)(blobRst->cy[index]);

			if(chipPosition) //l
			{
				if(noiseCy <= wndCog.y)
				{
					if(noiseCy < (wndCog.y / 2.0))
						noiseKind = eNOISE_SOLDER;
					else
						noiseKind = eNOISE_BODY;
				}
				else
				{
					if(noiseCy > (wndCog.y + (wndCog.y / 2.0)))
						noiseKind = eNOISE_SOLDER;
					else
						noiseKind = eNOISE_BODY;
				}
				
			}
			else //-
			{
				if(noiseCx <= wndCog.x)
				{
					if(noiseCx < (wndCog.x / 2.0))
						noiseKind = eNOISE_SOLDER;
					else
						noiseKind = eNOISE_BODY;
				}
				else
				{
					if(noiseCx > (wndCog.x + (wndCog.x / 2.0)))
						noiseKind = eNOISE_SOLDER;
					else
						noiseKind = eNOISE_BODY;
				}

			}
				
			//////////////////////////////////////////////////////////////////////////
			if(noiseKind == eNOISE_SOLDER)
			{
				if(chipPosition)
					marginX = wndRect.Width() / 2.0;
				else
					marginY = wndRect.Height() / 2.0;
			}
			else if(noiseKind == eNOISE_BODY)
			{
// 				if(chipPosition)
// 					marginY = wndRect.Height() / 2.0;				
// 				else
// 					marginX = wndRect.Width() / 2.0;

				marginX = 0;
				marginY = 0;
			}
			Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
			Im::Gra::RectFill(M_DEFAULT, milMask, blobRst->left[index] - marginX, blobRst->top[index] - marginY, blobRst->right[index] + marginX, blobRst->bottom[index] + marginY);
		

			SaveWorkImg(milMask, _T("milMask.bmp"));

			Im::PIL_ID milDst = M_NULL;
			milDst = AllocBuff(sizeX, sizeY);
			ArithImage(milSrc, milMask, milDst, M_SUB  + M_SATURATION);

			if(dst != NULL)
				Im::Buf::Get(milDst, dst);
			SaveWorkImg(milDst, _T("milDst.bmp"));

			FreeMilImageBuff(&milMask);
			FreeMilImageBuff(&milDst);
		}

		//delete indexArr;
		g_pMManager->pem_delete(indexArr, true);
		ret = FALSE;
	}
		
	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milMask_Temp);	
	//delete blobRst;
	g_pMManager->pem_delete(blobRst, false);

	return ret;

}

//BOOL chipPos  까지 추가.. 더 진행 해야함.
int CProcMil_Mount::CalcNoisePos(Im::PIL_ID milSrc, CRect blobRect, CRect wndRect, BOOL chipPos)  //chipPos
{
	if(milSrc == M_NULL)
		return 0;	
	
	CSize imgSize = GetImageSize(milSrc);
	//UCHAR* userSrc = new UCHAR[imgSize.cx * imgSize.cy];
	UCHAR* userSrc = g_pMManager->pem_new<UCHAR>(true, imgSize.cx * imgSize.cy, (PCHAR)__FUNCTION__, __LINE__);
	
	int allowPer = 50; // 각면에서 50% 이상 데이터가 있어야 인정..
	int margin = 1; //rect에서 margin pixel만큼 확장된 위치에서 pixel 검사.
	int searchSizeX = blobRect.Width();
	int searchSizeY = blobRect.Height();


	int index = 0;
	int searchLineCount = 0; //단순히 blob rect 주변에 흰 pixel 개수가 몇개 있는지 저장 하는 변수

	int searchLineStartPos = 0; 
	int searchLineLength = 0;

	//left	
	searchLineCount = searchSizeY;
	searchLineStartPos = (int)wndRect.left;
	searchLineLength = (int)(blobRect.left - wndRect.left);
	searchLineLength = searchLineLength < 0 ?    0 : searchLineLength;

	int dataCount = 0;
	int leftLineDataCount = 0; //한줄의 데이터 개수
	for(int i = 0; i < searchLineCount; i++)
	{
		index = (int)((imgSize.cx * (blobRect.top + i)) + (blobRect.left - margin));
		if(userSrc[index] > 0)
			dataCount += 1;	

		for(int j = 0; j < searchLineLength; j++)
		{
			index = (int)((imgSize.cx * (blobRect.top + i)) + (searchLineStartPos + j));
			if(userSrc[index] > 0)
				leftLineDataCount += 1;	
		}
	}
	double leftResult = ((double)dataCount / (double)searchLineCount) * 100.0;


	//right	
	searchLineCount = searchSizeY;
	searchLineStartPos = (int)blobRect.right;
	searchLineLength = (int)(wndRect.right - blobRect.right);
	searchLineLength = searchLineLength < 0 ?    0 : searchLineLength;

	dataCount = 0;
	int rightLineDataCount = 0;
	for(int i = 0; i < searchLineCount; i++)
	{
		index = (int)((imgSize.cx * (blobRect.top + i)) + (blobRect.right + margin));
		if(userSrc[index] > 0)
			dataCount += 1;	

		for(int j = 0; j < searchLineLength; j++)
		{
			index = (int)((imgSize.cx * (blobRect.top + i)) + (searchLineStartPos + j));
			if(userSrc[index] > 0)
				rightLineDataCount += 1;	
		}
	}
	double rightResult = ((double)dataCount / (double)searchLineCount) * 100.0;

	//top	
	searchLineCount = searchSizeX;
	searchLineStartPos = (int)wndRect.left;
	searchLineLength = (int)(blobRect.top - wndRect.top);
	searchLineLength = searchLineLength < 0 ?    0 : searchLineLength;

	dataCount = 0;
	int topLineDataCount = 0;
	for(int i = 0; i < searchLineCount; i++)
	{
		index = (int)((imgSize.cx * (blobRect.top - margin)) + (blobRect.left + i));
		if(userSrc[index] > 0)
			dataCount += 1;	

		for(int j = 0; j < searchLineLength; j++)
		{
			index = (int)((imgSize.cx * (blobRect.top + j)) + (searchLineStartPos + i));
			if(userSrc[index] > 0)
				topLineDataCount += 1;	
		}
	}
	double topResult = ((double)dataCount / (double)searchLineCount) * 100.0;

	//bottom	
	searchLineCount = searchSizeX;
	searchLineStartPos = (int)wndRect.left;
	searchLineLength = (int)(wndRect.bottom - blobRect.bottom);
	searchLineLength = searchLineLength < 0 ?    0 : searchLineLength;

	dataCount = 0;
	int bottomLineDataCount = 0;
	for(int i = 0; i < searchLineCount; i++)
	{
		index = (int)((imgSize.cx * (blobRect.bottom + margin)) + (blobRect.left + i));
		if(userSrc[index] > 0)
			dataCount += 1;	

		for(int j = 0; j < searchLineLength; j++)
		{
			index = (int)((imgSize.cx * (blobRect.bottom + j)) + (searchLineStartPos + i));
			if(userSrc[index] > 0)
				bottomLineDataCount += 1;	
		}
	}
	double bottomResult = ((double)dataCount / (double)searchLineCount) * 100.0;
	//////////////////////////////////////////////////////////////////////////

	int temp[4] = {0,};  //0: top , 1: left , 2: bottom , 3: right
	int tempCount = 0;

	if(topResult >= (double)allowPer)
		temp[0] = topLineDataCount;
	if(leftResult >= (double)allowPer)
		temp[1] = leftLineDataCount;
	if(bottomResult >= (double)allowPer)
		temp[2] = bottomLineDataCount;
	if(rightResult >= (double)allowPer)
		temp[3] = rightLineDataCount;


    return 0;

	//delete userSrc;
	g_pMManager->pem_delete(userSrc, true);
}

void CProcMil_Mount::MorphologyOpen(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, int iteration)
{
	Im::PIL_ID milSrc = M_NULL;

	milSrc = AllocBuff(nWidth, nHeight);

	Im::Buf::Put(milSrc, ucSrc);

	MorOpen(milSrc, milSrc, iteration);
	MorErode(milSrc, milSrc, iteration);
	
	Im::Buf::Get(milSrc, ucDst);

	FreeMilImageBuff(&milSrc);
}

void CProcMil_Mount::MorphologyClose(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, int iteration)
{
	Im::PIL_ID milSrc = M_NULL;

	milSrc = AllocBuff(nWidth, nHeight);

	Im::Buf::Put(milSrc, ucSrc);
	
	MorDilate(milSrc, milSrc, iteration);

	Im::Buf::Get(milSrc, ucDst);

	FreeMilImageBuff(&milSrc);
}

void CProcMil_Mount::DrawBodyLine(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, POINT* pt)
{
	Im::PIL_ID milSrc = M_NULL;

	milSrc = AllocBuff(nWidth, nHeight);
	Im::Buf::Clear(milSrc, 0L);
	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);

	if(pt != NULL)
	{
		Im::Gra::Line(M_DEFAULT, milSrc, (MIL_DOUBLE)pt[0].x, (MIL_DOUBLE)pt[0].y, (MIL_DOUBLE)pt[1].x, (MIL_DOUBLE)pt[1].y);
		Im::Gra::Line(M_DEFAULT, milSrc, (MIL_DOUBLE)pt[1].x, (MIL_DOUBLE)pt[1].y, (MIL_DOUBLE)pt[2].x, (MIL_DOUBLE)pt[2].y);
		Im::Gra::Line(M_DEFAULT, milSrc, (MIL_DOUBLE)pt[2].x, (MIL_DOUBLE)pt[2].y, (MIL_DOUBLE)pt[3].x, (MIL_DOUBLE)pt[3].y);
		Im::Gra::Line(M_DEFAULT, milSrc, (MIL_DOUBLE)pt[3].x, (MIL_DOUBLE)pt[3].y, (MIL_DOUBLE)pt[0].x, (MIL_DOUBLE)pt[0].y);
	}

	Im::Buf::Get(milSrc, ucDst);

	FreeMilImageBuff(&milSrc);
}

void CProcMil_Mount::FillHole(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight)
{
	Im::PIL_ID milSrc = M_NULL;

	milSrc = AllocBuff(nWidth, nHeight);
	Im::Buf::Put(milSrc, ucSrc);

	FillHoleBlob(milSrc, milSrc, m_milBlobResult);

	Im::Buf::Get(milSrc, ucDst);

	FreeMilImageBuff(&milSrc);
}

void CProcMil_Mount::FillDot(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, bool* arrDot)
{
	Im::PIL_ID milSrc = M_NULL;

	milSrc = AllocBuff(nWidth, nHeight);

	Im::Buf::Clear(milSrc, 0L);

	for(int j = 0 ; j < nHeight; j++)
	{
		for(int i = 0; i < nWidth; i++)
		{
			if(arrDot[(nWidth*j)+i] == true)
			{
				Im::Gra::Dot(M_DEFAULT, milSrc, i, j);
			}
		}
	}

	Im::Buf::Get(milSrc, ucDst);

	FreeMilImageBuff(&milSrc);
}

void CProcMil_Mount::DrawResultRect(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, CRect rcRect)
{
	Im::PIL_ID milSrc = M_NULL;

	milSrc = AllocBuff(nWidth, nHeight);
	Im::Buf::Put(milSrc, ucSrc);
	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);

	Im::Gra::Rect(M_DEFAULT, milSrc, (MIL_DOUBLE)rcRect.left, (MIL_DOUBLE)rcRect.top, (MIL_DOUBLE)rcRect.right, (MIL_DOUBLE)rcRect.bottom);

	Im::Buf::Get(milSrc, ucDst);

	FreeMilImageBuff(&milSrc);
}

CMilBlobResult* CProcMil_Mount::GetDamageResult(UCHAR* ucSrc, UCHAR* ucDst,int nWidth, int nHeight, bool *ptrbResult)
{
	*ptrbResult = FALSE;
	CMilBlobResult* blobRst = NULL;
	MIL_INT count = 0;

	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milBlob = M_NULL;
	MIL_INT blobCnt = 0;

	int minArea = 0;
	int maxArea = 0;
	minArea = (int)((nWidth * nHeight) * (1 / 100.0));   //area  *  1%
	maxArea = (int)((nWidth * nHeight) * (90 / 100.0));  //area  *  90% 
	blobCnt = CalcBlob(ucSrc, nWidth, nHeight, minArea, maxArea, FALSE, TRUE); 

	if(blobCnt > 1)
	{
		if(blobRst != NULL)
			//delete blobRst;
			g_pMManager->pem_delete(blobRst, false);

		SelectBlob_MaxArea(blobCnt, ucSrc, nWidth, nHeight);

		blobCnt = CalcBlob(ucSrc, nWidth, nHeight, minArea, maxArea, FALSE, TRUE);
		if(blobCnt > 1)
			*ptrbResult = true;
	}

	blobRst = new CMilBlobResult(blobCnt);
	g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

	GetBlobResult(blobRst);

	return blobRst;
}

int CProcMil_Mount::TeachPattern(cv::Mat OrgImg, int nImgWid, int nImgLen, double wndAngle,
								double bodyWidth, double bodyHeight, double cogX_roi_pixel, double cogY_roi_pixel, 
								CString sModelPath)
{
	double radAng = (-wndAngle) / 180.0 * 3.141592653589;
	double sinAng = sin(radAng);
	double cosAng = cos(radAng);

	double BodyWid = fabs(cosAng * bodyWidth - sinAng * bodyHeight);
	double BodyLen = fabs(sinAng * bodyWidth + cosAng * bodyHeight);

	int or = nImgLen / 2;
	int oc = nImgWid / 2;

	int bW = RounD(BodyWid);
	int bH = RounD(BodyLen);
	int cogX = RounD(cogX_roi_pixel - oc);
	int cogY = RounD(cogY_roi_pixel - or);
	int stX = RounD(cogX_roi_pixel - oc - BodyWid/2.0);
	int stY = RounD(cogY_roi_pixel - or - BodyLen/2.0);

	cv::Mat rotImg;
	int angle(-wndAngle);
	if (angle == 270 || angle == -90) {
		cv::transpose(OrgImg, rotImg);
		cv::flip(rotImg, rotImg, 0);
	}
	else if (angle == 180 || angle == -180) {
		cv::flip(OrgImg, rotImg, -1);
	}
	else if (angle == 90 || angle == -270) {
		cv::transpose(OrgImg, rotImg);
		cv::flip(rotImg, rotImg, 1);
	}
	else if (angle == 360 || angle == 0 || angle == -360) {
		OrgImg.copyTo(rotImg);
	}
	SaveWorkImg(rotImg, _T("rotImg-Rot.bmp"));

	int nSzMargin(0);
	cv::Mat ModelImg(BodyLen+nSzMargin, BodyWid+nSzMargin, CV_8UC1, cv::Scalar(255));

	GeoModel_Pattern model;
	model.Alloc(ModelImg);
	model.setAngleRange(true, 20, 20, 0.5);
	model.setFindSubPixel(true);
	model.setAngleStep(15);
	model.setMatchAlgo(GeoModel::enmMatchAlgo::agValue);
	model.Preprocess();

	float bodyw = nImgWid * m_resolX;
	float bodyl = nImgLen * m_resolY;

	model.setRes(m_resolX,m_resolY);
	model.SaveFile(sModelPath, false);
	
	int OrgCtX = OrgImg.cols / 2;
	int OrgCtY = OrgImg.rows / 2;
	double cogXn = cogX_roi_pixel - OrgCtX;
	double cogYn = cogY_roi_pixel - OrgCtY;
	double stdCogX = cosAng * cogXn - sinAng * cogYn;
	double stdCogY = sinAng * cogXn + cosAng * cogYn;


	int RotCtX = rotImg.cols / 2;
	int RotCtY = rotImg.rows / 2;
	GeoMatch_BodyBlob match;
	GeoResult_Pat resMatch(1);
	std::shared_ptr<ImgProcessing> _ProcImg = std::shared_ptr<ImgProcessing>(new ImgProcessing());
	_ProcImg->_ImgProcess(rotImg, model);
	match.SetImgProcess(_ProcImg);
	match.FindModel(model, resMatch, RotCtX+stdCogX, RotCtY-stdCogY);

//	cv::Mat drawImg = match.DrawContourColor(ColorImg, model, resMatch, cv::Scalar(255,0,0), 1);
//	SaveWorkImg(drawImg, _T("bodyblob.bmp"));

	return 0;
}

int CProcMil_Mount::InspPattern(cv::Mat OrgImg, int nImgWid, int nImgLen, double wndAngle, 
								double cogX_roi_pixel, double cogY_roi_pixel, 
								CString sModelPath, double & dPatAngle)
{
	int ImgCtX = nImgWid / 2;
	int ImgCtY = nImgLen / 2;

	double stdCogXp = cogX_roi_pixel - ImgCtX;
	double stdCogYp = cogY_roi_pixel - ImgCtY;

	std::shared_ptr<GeoModel_Pattern> model;
	m_ModelFile = m_ModelMng->GetModel(sModelPath);
	if(m_ModelFile != nullptr)
		model = m_ModelFile->GetAngleModel(0);

	bool bLoaded(false);
	if(model == nullptr)
	{
		model = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());
		bLoaded = model->LoadFile(sModelPath,m_resolX,m_resolY);
	}
	else
		bLoaded = true;

	if(bLoaded==true)
	{
		double radAng = (-wndAngle) / 180.0 * 3.141592653589;
		double sinAng = sin(radAng);
		double cosAng = cos(radAng);

		cv::Mat rotImg;
		int angle(-wndAngle);
		if (angle == 270 || angle == -90) {
			cv::transpose(OrgImg, rotImg);
			cv::flip(rotImg, rotImg, 0);
		}
		else if (angle == 180 || angle == -180) {
			cv::flip(OrgImg, rotImg, -1);
		}
		else if (angle == 90 || angle == -270) {
			cv::transpose(OrgImg, rotImg);
			cv::flip(rotImg, rotImg, 1);
		}
		else if (angle == 360 || angle == 0 || angle == -360) {
			OrgImg.copyTo(rotImg);
		}
		SaveWorkImg(rotImg, _T("rotImg-Rot.bmp"));

		int stdCogX = RounD(rotImg.cols/2 + (cosAng * stdCogXp - sinAng * stdCogYp));
		int stdCogY = RounD(rotImg.rows/2 + (sinAng * stdCogXp + cosAng * stdCogYp));

		if(stdCogX <= 2 || stdCogY<=2 || (rotImg.cols-2) >= stdCogX || (rotImg.rows-2)>=stdCogY)
			return 1;

		model->setAngleStep(15);
		GeoMatch_BodyBlob match;
		GeoResult_Pat resMatch(1);
		std::shared_ptr<ImgProcessing> _ProcImg = std::shared_ptr<ImgProcessing>(new ImgProcessing());
		_ProcImg->_ImgProcess(rotImg, *model);
		match.SetImgProcess(_ProcImg);
		match.FindModel(*model, resMatch, stdCogX, stdCogY);

		dPatAngle = resMatch._Angle[0];

		return 0;
	}
	return 1;
}

int CProcMil_Mount::ClearModelList(int nLane)
{
	return m_ModelMng->Clear_File(nLane);
}

int CProcMil_Mount::LoadModelList(CString sPath)
{
	return m_ModelMng->LoadFileList(sPath,m_resolX,m_resolY);
}

int CProcMil_Mount::CheckModelList(CString path)
{
	return m_ModelMng->CheckFileList(path,m_resolX,m_resolY);
}