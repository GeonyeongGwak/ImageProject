#include "StdAfx.h"
#include "ProcMil_BGA.h"
#include "MemoryManager.h"


CProcMil_BGA::CProcMil_BGA(void)
{
	m_className = _T("CProcMil_BGA");

	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;
}


CProcMil_BGA::~CProcMil_BGA(void)
{
	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;
}


int CProcMil_BGA::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	if(milApp == M_NULL || milSys == M_NULL)
	{
		return eMBGA_FAIL;	
	}

	CProcMil :: InitMil(milApp, milSys,bUseImagePilLib);

	InitMilAlgoBlob();
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

	return eMBGA_SUCCESS;
}

int CProcMil_BGA::FreeMil()
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

	FreeMilAlgoBlob();

	CProcMil :: FreeMil();
#endif

	return eMBGA_SUCCESS;
}


int CProcMil_BGA::CalcBlobM(Im::PIL_ID milBinSrc, int minArea, int maxArea, BOOL eraseBorderBlob, BOOL fillHole, Im::PIL_ID milDst)
{
	MIL_INT count = 0;
	CSize sz = GetImageSize(milBinSrc);

	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sz.cx, sz.cy);
	Im::Buf::Copy(milBinSrc, milSrc);

	if(fillHole)
	{
		FillHoleBlob(milSrc, milSrc, m_milBlobResult);
	}

	if(eraseBorderBlob)
	{
		EraseBorderBlob(milSrc, milSrc, m_milBlobResult);
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
		Im::Blob::blobFill(m_milBlobResult, milDst, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw
	}

	//get result
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);


	FreeMilImageBuff(&milSrc);
	return (int)count;
}

int CProcMil_BGA::CalcBlobU(UCHAR* userBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob, BOOL fillHole, UCHAR* userDst)
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
	}

	//get result
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);



	FreeMilImageBuff(&milBinSrc);
	FreeMilImageBuff(&milDst);

	return (int)count;
}

void CProcMil_BGA::GetBlobResult(CMilBlobResult* retResult)
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
		Im::Blob::blobGetResult(m_milBlobResult, M_FERET_MEAN_DIAMETER + M_TYPE_DOUBLE, blobResult->diameter);
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
		g_pMManager->pem_delete(label, false);

		retResult->Copy(blobResult,0);
		//delete blobResult;
		g_pMManager->pem_delete(blobResult, false);
	}	
}

int CProcMil_BGA::BlobProc(UCHAR* userSrc, int sizeX, int sizeY, UCHAR* retUserDst, UCHAR* retUserLut)
{
	int blobCnt = 0;

	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSrc);

	Im::PIL_ID milBin = M_NULL;
	milBin = AllocBuff(sizeX, sizeY);

	int threshold = 90;//GetThreshold(milSrc);
	Binarize(milSrc, milBin, threshold, FALSE);
	SaveWorkImg(milBin, _T("bgaBin.bmp"));


	Im::PIL_ID milBlob = M_NULL;
	milBlob = AllocBuff(sizeX, sizeY);
	blobCnt = CalcBlobM(milBin,100, 0, FALSE, TRUE, milBlob);
	SaveWorkImg(milBlob, _T("milBlob.bmp"));



	//huj 2014/03/18
	//////////////////////////////////////////////////////////////////////////
	//make lut data 
	if(retUserLut != NULL)
	{
		Im::Buf::Get(milBlob, retUserLut);
	}

	//huj 2014/03/18
	//////////////////////////////////////////////////////////////////////////
	if(blobCnt > 1)
	{
		CMilBlobResult* blobRst = new CMilBlobResult(blobCnt);
		g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

		GetBlobResult(blobRst);

		int label = -1;
		CRect rt;
		CPoint pt((int)(sizeX / 2.0), (int)(sizeY / 2.0));

		//window center를 포함하는 blob만을 남긴다. (blob area가 아닌 blob box를 본다.)
		for(int i = 0; i < blobCnt; i++)
		{
			rt.SetRect((int)blobRst->left[i] ,(int)blobRst->top[i] ,(int)blobRst->right[i] ,(int)blobRst->bottom[i]);

			if(PtInRect(rt,pt))
			{
				label = blobRst->blobLabel[i];
				break;				
			}

		}

		//window center를 포함하는 blob이 없을경우... 제일 큰 blob을 찾는다. (window center에 제일 근접한 blob을 찾아야 하는 경우도 있을듯..)
		if(label == -1)
		{
			double max = blobRst->area[0];
			label = blobRst->blobLabel[0];
			for( int i = 1; i < blobCnt ; i ++)
			{
				if( max < blobRst->area[i])
				{
					max = blobRst->area[i];
					label = blobRst->blobLabel[i];
				}
			}
		}	


		//블랍을 그려줌..		
		Im::PIL_ID milDst = AllocBuff(sizeX, sizeY, 0);
		Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
		DrawBlob(m_milBlobResult, milDst, label, FALSE);

		Im::Buf::Clear(milBlob, 0);
		blobCnt = CalcBlobM(milDst,100, 0, FALSE, TRUE, milBlob);

		FreeMilImageBuff(&milDst);	


		//delete blobRst;
		g_pMManager->pem_delete(blobRst, false);
	}



	//////////////////////////////////////////////////////////////////////////

// 	if(blobCnt >= 2) //2개 이상일경우.. ball이 쪼개진 경우일 수 있음..
// 	{
// 		Im::Buf::Clear(milBin, 0);
// 		MorClose(milBlob, milBin, 3);
// 		blobCnt = CalcBlobM(milBin,100, 0, TRUE, TRUE, milBlob);
// 		
// 		if(blobCnt >= 2)
// 		{
// 			SelectBlob_MaxArea(blobCnt, milBlob);
// 			SaveWorkImg(milBlob, _T("milBlob2.bmp"));
// 		}
// 	}


	if(retUserDst != NULL)
		Im::Buf::Get(milBlob, retUserDst);


	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milBin);
	FreeMilImageBuff(&milBlob);
	

	return blobCnt;
}

int CProcMil_BGA::MakeLUT(UCHAR* userSrc, int sizeX, int sizeY, UCHAR* retUserDst)
{
	int blobCnt = 0;

	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSrc);

	Im::PIL_ID milBin = M_NULL;
	milBin = AllocBuff(sizeX, sizeY);

	int threshold = GetThreshold(milSrc);
	Binarize(milSrc, milBin, threshold, FALSE);

	Im::PIL_ID milBlob = M_NULL;
	milBlob = AllocBuff(sizeX, sizeY);
	blobCnt = CalcBlobM(milBin,100, 0, FALSE, TRUE, milBlob);

	SaveWorkImg(milBlob, _T("milLut.bmp"));

	if(retUserDst != NULL)
		Im::Buf::Get(milBlob, retUserDst);


	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milBin);
	FreeMilImageBuff(&milBlob);

	return blobCnt;
}

int CProcMil_BGA::SelectBlob_MaxArea(int blobNum,  Im::PIL_ID milDst)
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

void CProcMil_BGA::MakeImage_Sum(void* milImgT, void* milImgM, void* milImgB, UCHAR* userDst)
{
	if(milImgT == M_NULL || milImgM == M_NULL || milImgB == M_NULL)
		return;

	CSize size = GetImageSize((Im::PIL_ID)milImgT);
	Im::PIL_ID milDest = M_NULL;
	milDest = AllocBuff(size.cx, size.cy, 0);

	Im::PIL_ID milSrc_T = M_NULL;
	milSrc_T = AllocBuff(size.cx, size.cy, 0);
	Im::Buf::Copy((Im::PIL_ID)milImgT, milSrc_T);

	Im::PIL_ID milSrc_M = M_NULL;
	milSrc_M = AllocBuff(size.cx, size.cy, 0);
	Im::Buf::Copy((Im::PIL_ID)milImgM, milSrc_M);

	Im::PIL_ID milSrc_B = M_NULL;
	milSrc_B = AllocBuff(size.cx, size.cy, 0);
	Im::Buf::Copy((Im::PIL_ID)milImgB, milSrc_B);


	ArithImage(milSrc_B, milSrc_M, milDest, M_OR);

	if(userDst != NULL)
		Im::Buf::Get(milDest, userDst);

	FreeMilImageBuff(&milDest);
	FreeMilImageBuff(&milSrc_T);
	FreeMilImageBuff(&milSrc_M);
	FreeMilImageBuff(&milSrc_B);
}