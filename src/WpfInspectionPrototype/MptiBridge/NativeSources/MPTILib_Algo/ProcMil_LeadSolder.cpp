#include "StdAfx.h"
#include "ProcMil_LeadSolder.h"
#include <math.h>
#include "MemoryManager.h"

//shk 2014-01-03
#include "ipps.h"
#include "ippi.h"
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")

//shk 20140416
#define LEAD_SOLDER_DOWNSTEP	0.35

CProcMil_LeadSolder::CProcMil_LeadSolder(void)
{
	m_className = _T("CProcMil_LeadSolder");

	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;
}


CProcMil_LeadSolder::~CProcMil_LeadSolder(void)
{
	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;
}



int CProcMil_LeadSolder::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	if(milApp == M_NULL || milSys == M_NULL)
	{
		return eMLSD_FAIL;	
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

	return eMLSD_SUCCESS;
}

int CProcMil_LeadSolder::FreeMil()
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

	return eMLSD_SUCCESS;
}



int CProcMil_LeadSolder::CalcBlobM(Im::PIL_ID milBinSrc, int minArea, int maxArea, BOOL eraseBorderBlob, BOOL fillHole, Im::PIL_ID milDst)
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

void CProcMil_LeadSolder::GetBlobResult(CMilBlobResult* retResult)
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
		g_pMManager->pem_delete(label, false);

		retResult->Copy(blobResult,0);
		//delete blobResult;
		g_pMManager->pem_delete(blobResult, false);
	}	
}

void CProcMil_LeadSolder::DataSorting(CMilBlobResult* src, CRect* dst, BOOL flag)
{
	int cnt = src->count;
	//CRect* rtTemp = new CRect[cnt];
	CRect* rtTemp = g_pMManager->pem_new<CRect>(true, cnt, (PCHAR)__FUNCTION__, __LINE__);

	for(int i = 0; i < cnt; i++)
	{
		rtTemp[i].left = (LONG)(src->left[i]);
		rtTemp[i].right = (LONG)(src->right[i]);
		rtTemp[i].top = (LONG)(src->top[i]);
		rtTemp[i].bottom = (LONG)(src->bottom[i]);
	}

	if(flag == TRUE)//소팅 할것인지..
	{
		CRect temp = 0;
		for(int i = 0; i < cnt; i++)
		{
			for(int j = i+1; j < cnt; j++)
			{
				if(rtTemp[i].left > rtTemp[j].left)
				{
					temp = rtTemp[i];
					rtTemp[i] = rtTemp[j];
					rtTemp[j] = temp;
				}
			}
		}
	}

	memcpy_s(dst, sizeof(CRect) * cnt, rtTemp, sizeof(CRect) * cnt);
	//delete rtTemp;
	g_pMManager->pem_delete(rtTemp, false);
}

int CProcMil_LeadSolder::GetSolderRect(float* zmapRoiData, UCHAR* userSrc_Top, UCHAR* userSrc_Bottom, int sizeX, int sizeY, int solderStartPos, CRect** retBlobRect, CRect** retSolderRect)
{
	int solderCnt = 0;

	CMilBlobResult* blobRst = NULL;
	solderCnt = CalcBlobRect(zmapRoiData, userSrc_Top, userSrc_Bottom, sizeX, sizeY, &blobRst);

	if(solderCnt <= 0)
		return 0;

	//CRect* rectSolt = new CRect[solderCnt];
	CRect* rectSolt = g_pMManager->pem_new<CRect>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
	DataSorting(blobRst, rectSolt, TRUE);


	//CRect* rectTemp = new CRect[solderCnt];
	CRect* rectTemp = g_pMManager->pem_new<CRect>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
	for(int i = 0; i < solderCnt; i++)
	{
		rectTemp[i].left = rectSolt[i].left;
		rectTemp[i].right = rectSolt[i].right;
		rectTemp[i].top = solderStartPos;
		rectTemp[i].bottom = (rectSolt[i].bottom <= solderStartPos)? (solderStartPos + 1) : rectSolt[i].bottom;

		if(rectTemp[i].Width() <=  0 || rectTemp[i].Height() <= 0 || rectTemp[i].left < 0 || rectTemp[i].right < 0 || rectTemp[i].top < 0 || rectTemp[i].bottom < 0)
		{
			/*delete rectSolt;
			delete blobRst;
			delete rectTemp;*/
			g_pMManager->pem_delete(rectSolt, false);
			g_pMManager->pem_delete(blobRst, false);
			g_pMManager->pem_delete(rectTemp, false);
			return 0;
		}
	}

	//*retBlobRect = new CRect[solderCnt];
	*retBlobRect = g_pMManager->pem_new<CRect>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
	memcpy_s(*retBlobRect, sizeof(CRect) * solderCnt, rectSolt, sizeof(CRect) * solderCnt);

	//*retSolderRect = new CRect[solderCnt];
	*retSolderRect = g_pMManager->pem_new<CRect>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
	memcpy_s(*retSolderRect, sizeof(CRect) * solderCnt, rectTemp, sizeof(CRect) * solderCnt);

	/*delete blobRst;
	delete rectTemp;
	delete rectSolt;*/
	g_pMManager->pem_delete(blobRst, false);
	g_pMManager->pem_delete(rectTemp, false);
	g_pMManager->pem_delete(rectSolt, false);

	return solderCnt;
}

int CProcMil_LeadSolder::CalcBlobRect(float* zmapRoiData, UCHAR* userSrc_Top, UCHAR* userSrc_Bottom, int sizeX, int sizeY, CMilBlobResult** retRst)
{
	int blobCnt = 0;

	Im::PIL_ID milArithImg = M_NULL;
	milArithImg = AllocBuff(sizeX, sizeY, 0);

	Im::PIL_ID milClip_Top = M_NULL;
	milClip_Top = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milClip_Top, userSrc_Top);

	Im::PIL_ID milClip_Bottom = M_NULL;
	milClip_Bottom = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milClip_Bottom, userSrc_Bottom);

	ArithImage(milClip_Top, milClip_Bottom, milArithImg, M_OR);
	SaveWorkImg(milArithImg, _T("milArithImg.bmp"));

	Im::PIL_ID milBlobImg = M_NULL;
	milBlobImg = AllocBuff(sizeX, sizeY);
	blobCnt = BlobProc(milArithImg, sizeX, sizeY, milBlobImg);


	////////////////////////////////////////////////////////////////////////// //높이 데이터 없는 부분은 제외
	int area = sizeX * sizeY;

	Im::PIL_ID selectBlob = M_NULL;
	selectBlob = AllocBuff(sizeX, sizeY);

	UCHAR* userSelectBlobImg = NULL;
	//userSelectBlobImg = new UCHAR[area];
	userSelectBlobImg = g_pMManager->pem_new<UCHAR>(true, area, (PCHAR)__FUNCTION__, __LINE__);
	//memset(userSelectBlobImg, 0, sizeof(UCHAR) * area);

	//float* hAvrTemp = new float[blobCnt];
	float* hAvrTemp = g_pMManager->pem_new<float>(true, blobCnt, (PCHAR)__FUNCTION__, __LINE__);
	double avrTempSum = 0;

	//double* label = new double[blobCnt];
	double* label = g_pMManager->pem_new<double>(true, blobCnt, (PCHAR)__FUNCTION__, __LINE__);
	Im::Blob::blobGetResult(m_milBlobResult,  M_LABEL_VALUE   , label);

	for(int i = 0; i < blobCnt; i++)
	{
		Im::Buf::Copy(milBlobImg, selectBlob);
		Im::Blob::blobSelect(m_milBlobResult, M_INCLUDE_ONLY, M_LABEL_VALUE , M_EQUAL , label[i], M_NULL);		
		Im::Blob::blobFill(m_milBlobResult, selectBlob, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
		Im::Blob::blobFill(m_milBlobResult, selectBlob, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw
		Im::Buf::Get(selectBlob, userSelectBlobImg);

		int index = 0;
		double sum = 0;
		int cnt = 0;
		for(int y = 0; y < sizeY; y++)
		{
			for(int x = 0; x < sizeX; x++)
			{
				index = (y * sizeX) + x;
				if(userSelectBlobImg[index] > 0)
				{
					sum += zmapRoiData[index];
					cnt++;
				}

			}
		}
		if(sum == 0 || cnt ==0)
		{
			hAvrTemp[i] = 0;
		}
		else
		{
			hAvrTemp[i] = (float)(sum / cnt);
		}		
		avrTempSum += hAvrTemp[i];



		// 		CString name = _T("");
		// 		name.Format(_T("test_%d.bmp"), i+1);
		// 		SaveWorkImg(selectBlob, name);

		Im::Buf::Clear(selectBlob, 0);	
		memset(userSelectBlobImg, 0, sizeof(UCHAR) * area);
	}


	float avrTempAvr = 0;
	if(avrTempSum == 0 || blobCnt ==0)
	{
		avrTempAvr = 0;
	}
	else
	{
		avrTempAvr = (float)(avrTempSum / blobCnt);
	}	

	float stdHAvr = (float)(avrTempAvr * (50 / 100.0));
	for(int i = 0; i < blobCnt; i++)
	{
		if(hAvrTemp[i] > stdHAvr)
		{
			Im::Blob::blobSelect(m_milBlobResult, M_INCLUDE, M_LABEL_VALUE , M_EQUAL , label[i], M_NULL);
		}
		else
		{
			Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_LABEL_VALUE , M_EQUAL , label[i], M_NULL);
			blobCnt--;
		}
	}


	Im::Blob::blobFill(m_milBlobResult, selectBlob, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
	Im::Blob::blobFill(m_milBlobResult, selectBlob, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw

	SaveWorkImg(selectBlob, _T("finalBlob.bmp"));

	/*delete hAvrTemp;
	delete userSelectBlobImg;*/
	g_pMManager->pem_delete(hAvrTemp, false);
	g_pMManager->pem_delete(userSelectBlobImg, false);
	FreeMilImageBuff(&selectBlob);
	//////////////////////////////////////////////////////////////////////////

	*retRst = new CMilBlobResult(blobCnt);
	g_pMManager->pem_new_check(*retRst, (PCHAR)__FUNCTION__, __LINE__);
	GetBlobResult(*retRst);

	////////////////////////////////////////////////////////////////////////// 이부분까지 솔더영역 따내는 부분...(리드검사에 사용해도 됨)



	FreeMilImageBuff(&milBlobImg);
	FreeMilImageBuff(&milClip_Top);
	FreeMilImageBuff(&milClip_Bottom);
	FreeMilImageBuff(&milArithImg);
	//delete label;
	g_pMManager->pem_delete(label, false);

	return blobCnt;
}

int CProcMil_LeadSolder::BlobProc(Im::PIL_ID milSrc, int sizeX, int sizeY, Im::PIL_ID milDst)
{
	int blobCnt = 0;

	Im::PIL_ID milBin = M_NULL;
	milBin = AllocBuff(sizeX, sizeY);

	int threshold = GetThreshold(milSrc);
	Binarize(milSrc, milBin, threshold, FALSE);
	SaveWorkImg(milBin, _T("solderBin.bmp"));

	MorClose(milBin, milBin, 4);
	SaveWorkImg(milBin, _T("milBin_Close.bmp"));

	MorOpen(milBin, milBin, 1);
	SaveWorkImg(milBin, _T("milBin_Open.bmp"));

	Im::PIL_ID milSeed = M_NULL;
	milSeed = AllocBuff(sizeX, sizeY, 0);

	Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
	Im::Gra::RectFill(M_DEFAULT, milSeed, 0, 0, sizeX, sizeY / 2);
	SaveWorkImg(milSeed, _T("milSeed.bmp"));

	Im::Blob::blobReconstruct(milBin, milSeed, milBin, M_RECONSTRUCT_FROM_SEED, M_8_CONNECTED);

	Im::PIL_ID milBlob = M_NULL;
	milBlob = AllocBuff(sizeX, sizeY);
	blobCnt = CalcBlobM(milBin,100, 0, FALSE, FALSE, milBlob);
	SaveWorkImg(milBlob, _T("milBlob1.bmp"));

	double minblobSize = 100;
	if(blobCnt > 1)
	{
		//double* area = new double[blobCnt];
		double* area = g_pMManager->pem_new<double>(true, blobCnt, (PCHAR)__FUNCTION__, __LINE__);
		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);

		double sum = 0;
		for(int i = 0; i < blobCnt; i++)
		{
			sum += area[i];
		}
		double avr = sum / blobCnt;

		minblobSize = avr * (30 / 100.0);
		//delete area;
		g_pMManager->pem_delete(area, false);
	}



	blobCnt = CalcBlobM(milBlob, (int)minblobSize, 0, FALSE, FALSE, milBlob);
	SaveWorkImg(milBlob, _T("milBlob2.bmp"));

	if(milDst != M_NULL)
		Im::Buf::Copy(milBlob, milDst);

	FreeMilImageBuff(&milBin);
	FreeMilImageBuff(&milBlob);
	FreeMilImageBuff(&milSeed);


	return blobCnt;
}

int CProcMil_LeadSolder::GetSolderRect_Chip(float* zmapRoiData, UCHAR* userSrc, int sizeX, int sizeY, CRect* retRect)
{
	int solderCnt = 0;

	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSrc);

	Im::PIL_ID milBlobImg = M_NULL;
	milBlobImg = AllocBuff(sizeX, sizeY);
	solderCnt = BlobProc_Chip(milSrc, sizeX, sizeY, milBlobImg);

	CRect rectTemp;

	if(solderCnt > 0)
	{
		CMilBlobResult* blobRst = NULL;
		blobRst = new CMilBlobResult(solderCnt);
		g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

		GetBlobResult(blobRst);


		rectTemp.left = (LONG)blobRst->left[0];
		rectTemp.right = (LONG)blobRst->right[0];
		rectTemp.top = (LONG)blobRst->top[0];
		rectTemp.bottom = (LONG)blobRst->bottom[0];

		//delete blobRst;
		g_pMManager->pem_delete(blobRst, false);
	}
	else
	{
		rectTemp.left = 0;
		rectTemp.right = 0;
		rectTemp.top = 0;
		rectTemp.bottom = 0;
	}



	*retRect = rectTemp;

	FreeMilImageBuff(&milBlobImg);
	FreeMilImageBuff(&milSrc);


	return solderCnt;
}



int CProcMil_LeadSolder::BlobProc_Chip(Im::PIL_ID milSrc, int sizeX, int sizeY, Im::PIL_ID milDst)
{
	int blobCnt = 0;

	Im::PIL_ID milBin = M_NULL;
	milBin = AllocBuff(sizeX, sizeY);

	Binarize(milSrc, milBin, 1, FALSE);
	//SaveWorkImg(milBin, _T("solderBin_chip.bmp"));


	Im::PIL_ID milBlob = M_NULL;
	milBlob = AllocBuff(sizeX, sizeY);
	blobCnt = CalcBlobM(milBin,100, 0, FALSE, TRUE, milBlob);
	//SaveWorkImg(milBlob, _T("milBlob1_Chip.bmp"));

	MorClose(milBlob, milBlob, 3);
	//SaveWorkImg(milBin, _T("milBin_Close_chip.bmp"));

	blobCnt = CalcBlobM(milBlob,100, 0, FALSE, FALSE, milBlob);
	//SaveWorkImg(milBlob, _T("milBlob2_Chip.bmp"));

	if(blobCnt > 1)
	{
		SelectBlob_MaxArea(blobCnt, milBlob);
		blobCnt = CalcBlobM(milBlob,100, 0, FALSE, FALSE, milBlob);
	}

	if(milDst != M_NULL)
		Im::Buf::Copy(milBlob, milDst);

	FreeMilImageBuff(&milBin);
	FreeMilImageBuff(&milBlob);



	return blobCnt;
}






int CProcMil_LeadSolder::CalcSolderVolume(float* pZmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float* hLimit, double* retSolderVolume)
{
	int ret = TRUE;

	if(pZmapData == NULL || roiSizeX <= 0 || roiSizeY <= 0 || solderRect == NULL || solderCnt == 0 || hLimit == NULL)
		return 0;

	float* zmapClipBuf = NULL;

	int w = 0;
	int h = 0;
	int cx = 0;
	int cy = 0;
	int area = 0;
	float data = 0;

	//double* solderVol = new double[solderCnt];
	double* solderVol = g_pMManager->pem_new<double>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(solderVol, 0, sizeof(double) * solderCnt);

	for(int i = 0; i < solderCnt; i++)
	{	
		w = solderRect[i].Width();
		h = solderRect[i].Height();		
		cx = (int)(solderRect[i].left + (w / 2.0));
		cy = (int)(solderRect[i].top + (h / 2.0));

		if(w < 1 || h < 1 || cx < 0 || cy < 0 || w > roiSizeX || h > roiSizeY)
			break;
	
		area = w * h;
		//zmapClipBuf = new float[area];
		zmapClipBuf = g_pMManager->pem_new<float>(true, area, (PCHAR)__FUNCTION__, __LINE__);
		ClipZmap(pZmapData, zmapClipBuf, roiSizeX, roiSizeY, cx, cy, w, h);

		for(int j = 0; j < area; j++)
		{
			data = zmapClipBuf[j];

			if(data > 0)
			{
				if(data > hLimit[i])
					data = hLimit[i];

				solderVol[i] += data;
			}
		}

		//delete zmapClipBuf;
		g_pMManager->pem_delete(zmapClipBuf, false);
	}

	if(retSolderVolume != NULL)
	{
		memcpy_s(retSolderVolume, sizeof(double) * solderCnt, solderVol, sizeof(double) * solderCnt);
	}

	//delete solderVol;
	g_pMManager->pem_delete(solderVol, false);

	return ret;
}


int CProcMil_LeadSolder::SelectBlob_MaxArea(int blobNum,  Im::PIL_ID milDst)
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




void CProcMil_LeadSolder::SaveLeadProcImg(UCHAR* userSrc, int sizeX, int sizeY, CRect* leadRect, CRect* gapRect, CRect* solderRect, int leadCnts, UCHAR* retColorImg)
{
	Im::PIL_ID milBuf = M_NULL;
	milBuf = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milBuf, userSrc);
	Im::PIL_ID milColor = AllocBuffColor(sizeX, sizeY);

	Im::Buf::CopyColor(milBuf, milColor, M_ALL_BANDS);

	int leadThickness = 4;
	int gapThickness = 4;
	int solderThickness = 4;

	if(leadRect != NULL)
	{
		Im::Gra::Color(M_DEFAULT, M_COLOR_GREEN);
		for(int i = 0; i < leadCnts; i++)
		{	
			for(int j = 0; j < leadThickness; j++)
			{
				Im::Gra::Rect(M_DEFAULT, milColor, (MIL_DOUBLE)(leadRect[i].left + j), (MIL_DOUBLE)(leadRect[i].top + j), (MIL_DOUBLE)(leadRect[i].right - j), (MIL_DOUBLE)(leadRect[i].bottom - j));
			}
		}
	}

	if(solderRect != NULL)
	{
		Im::Gra::Color(M_DEFAULT, M_COLOR_RED);
		for(int i = 0; i < leadCnts; i++)
		{
			for(int j = 0; j < solderThickness; j++)
			{
				Im::Gra::Rect(M_DEFAULT, milColor, (MIL_DOUBLE)(solderRect[i].left + j), (MIL_DOUBLE)(solderRect[i].top + j), (MIL_DOUBLE)(solderRect[i].right - j), (MIL_DOUBLE)(solderRect[i].bottom - j));
			}
		}
	}

	if(gapRect != NULL)
	{
		Im::Gra::Color(M_DEFAULT, M_COLOR_BLUE);
		for(int i = 0; i < leadCnts - 1; i++)
		{
			for(int j = 0; j < gapThickness; j++)
			{
				Im::Gra::Rect(M_DEFAULT, milColor, (MIL_DOUBLE)((gapRect[i].left + 1) + j), (MIL_DOUBLE)((gapRect[i].top) + j), (MIL_DOUBLE)((gapRect[i].right - 1) - j), (MIL_DOUBLE)((gapRect[i].bottom) - j));
			}
		}
	}

 	if(retColorImg != NULL)
 		Im::Buf::GetColor(milColor, M_PACKED + M_BGR24, M_ALL_BANDS, retColorImg);

	SaveWorkImg(milColor, _T("result.bmp"));

	FreeMilImageBuff(&milColor);
	FreeMilImageBuff(&milBuf);
}



BOOL CProcMil_LeadSolder::InspGapArea(UCHAR* userSrc, float* zmapRoiData, int sizeX, int sizeY, CRect* gapRect, int avrGapW, int cnts, double angle, BOOL bridgeMode, float stdHeight, BOOL* retResult)
{
	BOOL ret = FALSE;
	Im::PIL_ID milTemp = M_NULL;
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milRotate = M_NULL;
	Im::PIL_ID milBin = M_NULL;

	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSrc);
	SaveWorkImg(milSrc, _T("clip.bmp"));

	int rtCx = 0;
	int rtCy = 0;
	int rtWidth = 0;
	int rtHeight = 0;
	int rotateWidth = 0;
	int rotateHeight = 0;

	int gapCnts = cnts;
	BOOL* result = NULL;
	//result = new BOOL[gapCnts];
	result = g_pMManager->pem_new<BOOL>(true, gapCnts, (PCHAR)__FUNCTION__, __LINE__);

	int searchMargin = 0;
	double per = 0.2;
	for(int i = 0; i < gapCnts; i++)
	{
		if(bridgeMode) //bridge mode = 2d only
			per = 0.5;

		searchMargin = (int)(gapRect[i].Width() * per);
		if(searchMargin > 10)
			searchMargin = 10;

		rtCx = (int)(gapRect[i].left + (gapRect[i].Width() / 2));
		rtCy = (int)(gapRect[i].top + (gapRect[i].Height() / 2));
		rtWidth = (int)(gapRect[i].Width()-searchMargin);
		if(rtWidth <= 0) rtWidth = gapRect[i].Width();	// JACKY 2013/9/30
		rtHeight = (int)(gapRect[i].Height());

		milTemp = AllocClipBuff(milSrc, rtCx, rtCy, rtWidth, rtHeight);
		SaveWorkImg(milTemp, _T("milTemp.bmp"));

		milRotate = AllocRotateBuff(milTemp, angle);

		CSize sz = GetImageSize(milRotate);
		rotateWidth = sz.cx;
		rotateHeight = sz.cy;

		SaveWorkImg(milRotate, _T("milRotate.bmp"));
		//////////////////////////////////////////////////////////////////////////
		milBin = AllocBuff(rotateWidth, rotateHeight);

		int t = GetOtsuThreshold(milRotate);
		int thresh = 90;//GetThreshold(milRotate);
// 		if(thresh < 100)
// 			thresh = 100;

		Binarize(milRotate, milBin, thresh, FALSE);
		SaveWorkImg(milBin, _T("bin.bmp"));


		//////////////////////////////////////////////////////////////////////////
		BOOL rst = FALSE;
		//UCHAR* userBlob = new UCHAR[rotateWidth * rotateHeight];
		UCHAR* userBlob = g_pMManager->pem_new<UCHAR>(true, rotateWidth * rotateHeight, (PCHAR)__FUNCTION__, __LINE__);

		int blobCnts = CalcBlob(milBin, rotateWidth, rotateHeight, 0, -1, FALSE, FALSE, userBlob);
		SaveWorkImg(userBlob, rotateWidth, rotateHeight, _T("Blob.bmp"));


		//CRect* blobRect = new CRect[blobCnts];
		CRect* blobRect = g_pMManager->pem_new<CRect>(true, blobCnts, (PCHAR)__FUNCTION__, __LINE__);
		memset(blobRect, 0, sizeof(CRect) * blobCnts);

		//double* blobArea = new double[blobCnts];
		double* blobArea = g_pMManager->pem_new<double>(true, blobCnts, (PCHAR)__FUNCTION__, __LINE__);
		memset(blobArea, 0, sizeof(double) * blobCnts);

		GetBlobResultRect(blobRect, blobArea);


		int stdBlobW = (int)(rotateWidth * 0.7); //일반적인 blob의 가로 size
		int stdBlobH = (int)(rotateHeight * 0.7);  //일반적인 blob의 세로 size
		BOOL bridgeFlag = FALSE; 

		int calcTempH = (int)(rotateHeight * 0.09);
		int stdBlobH_ForTop = calcTempH;// >= 5 ?   5 : calcTempH;

		for(int j = 0; j < blobCnts; j++)
		{
			int w = blobRect[j].Width();
			int h = blobRect[j].Height();

			if(blobRect[j].top != 0)	// ㅡ
			{//Blob이 top에 붙어있지 않는 경우
				if(w >= stdBlobW) // Blob의 폭
				{
					bridgeFlag = TRUE;
				}
			}
			else
			{//Blob이 top에 붙어있는 경우 블랍크기가 stdBlobH_ForTop 10% 보다 크고 Blob 면적이 rect 면적의 70% 이상 이면 Bridge.
				if(w >= stdBlobW) // Blob의 폭
				{
					if(h > stdBlobH_ForTop) //Blob의 높이
					{
						int blobRectArea = (int)((w * h) * 0.7);
						if(blobRectArea != 0 && (int)blobArea[j] >= blobRectArea) //면적
						{
							bridgeFlag = TRUE;
						}
					}
				}
			}
			
			if(blobRect[j].left != 0 && blobRect[j].right != rotateWidth)	// ㅣ
			{
				if(h >= stdBlobH)
				{
					//bridgeFlag = TRUE;
				}
			}

			if(bridgeFlag)
			{
				if(!bridgeMode)
				{
					float hThreshold = 60; //60um
					//huj 2014/01/16
					float hThreshold_High = (float)(stdHeight * 0.8);
					if(stdHeight == 0)
						hThreshold_High = 1000000;

					int stY = 0;
					int endY = 0;
					int stX = 0;
					int endX = 0;

					stX = gapRect[i].left + blobRect[j].left;
					endX = gapRect[i].left + blobRect[j].right;

					stY = gapRect[i].top + blobRect[j].top;
					endY = gapRect[i].top + blobRect[j].bottom;

					int pixelCount = 0;
					int heightCount = 0;
					int zmapIndex = 0;
					int imageIndex = 0;
					float zmapData = 0;
					UCHAR imgData = 0;
					for(int y = stY; y < endY; y++)
					{
						for(int x = stX; x < endX; x++)
						{
							zmapIndex = (sizeX * y) + x;
							imageIndex = (rotateWidth * (y - gapRect[i].top)) + (x - gapRect[i].left);
							zmapData = zmapRoiData[zmapIndex];
							imgData = userBlob[imageIndex];

							if(imgData == 255)
							{
								pixelCount++;
								if(zmapData >= hThreshold/* && zmapData <= hThreshold_High*/)
								{
									heightCount++;
								}
							}
						}
					}

					double ratio = ((double)heightCount / (double)pixelCount) * 100.0;

					if(ratio > 50) //50%이상이면 실제 bridge
					{
						rst = TRUE;
						break;
					}
				}
				else
				{
					rst = TRUE;
					break;
				}
			}

			//huj 2014/01/14
			bridgeFlag = FALSE;
		}

		//////////////////////////////////////////////////////////////////////////

		result[i] = rst;

		if(rst == TRUE)
			ret = TRUE;


		/*delete userBlob;
		delete blobRect;
		delete blobArea;*/
		g_pMManager->pem_delete(userBlob, false);
		g_pMManager->pem_delete(blobRect, false);
		g_pMManager->pem_delete(blobArea, false);

		FreeMilImageBuff(&milTemp);
		FreeMilImageBuff(&milBin);
		FreeMilImageBuff(&milRotate);
	}
	memcpy_s(retResult, (sizeof(BOOL) * gapCnts), result, (sizeof(BOOL) * gapCnts));

	FreeMilImageBuff(&milSrc);
	

	//delete result;
	g_pMManager->pem_delete(result, false);

	return ret;
}


BOOL CProcMil_LeadSolder::InspGapArea2(UCHAR* userSrcMid, UCHAR* userSrcBot, float* zmapRoiData, int sizeX, int sizeY, CRect* gapRect, int avrGapW, int cnts, double angle, BOOL bridgeMode, float stdHeight, BOOL* retResult)
{
	BOOL ret = FALSE;
	Im::PIL_ID milTemp = M_NULL;
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milSrc_Mid = M_NULL;
	Im::PIL_ID milSrc_Bot = M_NULL;
	Im::PIL_ID milRotate = M_NULL;
	Im::PIL_ID milBin = M_NULL;

	milSrc_Mid = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc_Mid, userSrcMid);
	SaveWorkImg(milSrc_Mid, _T("clip_mid.bmp"));

	milSrc_Bot = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc_Bot, userSrcBot);
	SaveWorkImg(milSrc_Bot, _T("clip_bot.bmp"));

	milSrc = AllocBuff(sizeX, sizeY);
// 	ArithImage(milSrc_Bot, milSrc_Mid, milSrc, M_ADD, TRUE);
// 	SaveWorkImg(milSrc, _T("clip_milSrc.bmp"));

	//////////////////////////////////////////////////////////////////////////
	//UCHAR* userTest = new UCHAR[sizeX * sizeY];
	UCHAR* userTest = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(userTest, 0, sizeof(UCHAR) * sizeX * sizeY);

	int index = 0;
	int val = 0;
	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			index = (y * sizeX) + x;
			
			if(userSrcBot[index] >= 50)  //50이상 되는 pixel 은 mid+bot
			{
// 				val = userSrcBot[index] + userSrcMid[index];
// 				if(val > 255)
// 					val = 255;
// 
// 				userTest[index] = (UCHAR)val;		
				userTest[index] = userSrcBot[index] >= userSrcMid[index] ?  userSrcBot[index] : userSrcMid[index];
			}
			else //50이하는 mid와 bot중 작은값
			{
				userTest[index] = userSrcBot[index] < userSrcMid[index] ?  userSrcBot[index] : userSrcMid[index];
			}

			
		}
	}

	Im::Buf::Put(milSrc, userTest);

	SaveWorkImg(userTest, sizeX, sizeY, _T("clip_milSrc2.bmp"));
	//delete userTest;
	g_pMManager->pem_delete(userTest, false);
	//////////////////////////////////////////////////////////////////////////




	int rtCx = 0;
	int rtCy = 0;
	int rtWidth = 0;
	int rtHeight = 0;
	int rotateWidth = 0;
	int rotateHeight = 0;

	int gapCnts = cnts;
	BOOL* result = NULL;
	//result = new BOOL[gapCnts];
	result = g_pMManager->pem_new<BOOL>(true, gapCnts, (PCHAR)__FUNCTION__, __LINE__);

	int searchMargin = 0;
	double per = 0.2;
	for(int i = 0; i < gapCnts; i++)
	{
		if(bridgeMode) //bridge mode = 2d only
			per = 0.5;

		searchMargin = (int)(gapRect[i].Width() * per);
		if(searchMargin > 10)
			searchMargin = 10;

		rtCx = (int)(gapRect[i].left + (gapRect[i].Width() / 2));
		rtCy = (int)(gapRect[i].top + (gapRect[i].Height() / 2));
		rtWidth = (int)(gapRect[i].Width()-searchMargin);
		if(rtWidth <= 0) rtWidth = gapRect[i].Width();	// JACKY 2013/9/30
		rtHeight = (int)(gapRect[i].Height());

		milTemp = AllocClipBuff(milSrc, rtCx, rtCy, rtWidth, rtHeight);
		SaveWorkImg(milTemp, _T("milTemp.bmp"));

		milRotate = AllocRotateBuff(milTemp, angle);

		CSize sz = GetImageSize(milRotate);
		rotateWidth = sz.cx;
		rotateHeight = sz.cy;

		SaveWorkImg(milRotate, _T("milRotate.bmp"));
		//////////////////////////////////////////////////////////////////////////
		milBin = AllocBuff(rotateWidth, rotateHeight);

		int t = GetOtsuThreshold(milRotate);
		int thresh = 90;//GetThreshold(milRotate);
		// 		if(thresh < 100)
		// 			thresh = 100;
		if (t > thresh)
		{
			thresh = t;
		}

// 		CString tmp ;
// 		tmp.Format(_T("t: %d   thres: %d"),t,thresh);
// 		OutputDebugString(tmp);

		Binarize(milRotate, milBin, thresh, FALSE);
		SaveWorkImg(milBin, _T("bin.bmp"));


		//////////////////////////////////////////////////////////////////////////
		BOOL rst = FALSE;
		//UCHAR* userBlob = new UCHAR[rotateWidth * rotateHeight];
		UCHAR* userBlob = g_pMManager->pem_new<UCHAR>(true, rotateWidth * rotateHeight, (PCHAR)__FUNCTION__, __LINE__);

		int blobCnts = CalcBlob(milBin, rotateWidth, rotateHeight, 0, -1, FALSE, FALSE, userBlob);
		SaveWorkImg(userBlob, rotateWidth, rotateHeight, _T("Blob.bmp"));


		//CRect* blobRect = new CRect[blobCnts];
		CRect* blobRect = g_pMManager->pem_new<CRect>(true, blobCnts, (PCHAR)__FUNCTION__, __LINE__);
		memset(blobRect, 0, sizeof(CRect) * blobCnts);

		//double* blobArea = new double[blobCnts];
		double* blobArea = g_pMManager->pem_new<double>(true, blobCnts, (PCHAR)__FUNCTION__, __LINE__);
		memset(blobArea, 0, sizeof(double) * blobCnts);

		GetBlobResultRect(blobRect, blobArea);


		int stdBlobW = (int)(rotateWidth * 0.7); //일반적인 blob의 가로 size
		int stdBlobH = (int)(rotateHeight * 0.7);  //일반적인 blob의 세로 size
		BOOL bridgeFlag = FALSE; 

		int calcTempH = (int)(rotateHeight * 0.09);
		int stdBlobH_ForTop = calcTempH;// >= 5 ?   5 : calcTempH;

		for(int j = 0; j < blobCnts; j++)
		{
			int w = blobRect[j].Width();
			int h = blobRect[j].Height();

			if(blobRect[j].top != 0)	// ㅡ
			{//Blob이 top에 붙어있지 않는 경우
				if(w >= stdBlobW) // Blob의 폭
				{
					bridgeFlag = TRUE;
				}
			}
			else
			{//Blob이 top에 붙어있는 경우 블랍크기가 stdBlobH_ForTop 10% 보다 크고 Blob 면적이 rect 면적의 70% 이상 이면 Bridge.
				if(w >= stdBlobW) // Blob의 폭
				{
					if(h > stdBlobH_ForTop) //Blob의 높이
					{
						int blobRectArea = (int)((w * h) * 0.65);
						if(blobRectArea != 0 && (int)blobArea[j] >= blobRectArea) //면적
						{
							bridgeFlag = TRUE;
						}
					}
				}
			}

			if(blobRect[j].left != 0 && blobRect[j].right != rotateWidth)	// ㅣ
			{
				if(h >= stdBlobH)
				{
					//bridgeFlag = TRUE;
				}
			}

			if(bridgeFlag)
			{
				if(!bridgeMode)
				{
					float hThreshold = 60; //60um
					//huj 2014/01/16
					float hThreshold_High = (float)(stdHeight * 0.8);
					if(stdHeight == 0)
						hThreshold_High = 1000000;

					int stY = 0;
					int endY = 0;
					int stX = 0;
					int endX = 0;

					stX = gapRect[i].left + blobRect[j].left;
					endX = gapRect[i].left + blobRect[j].right;

					stY = gapRect[i].top + blobRect[j].top;
					endY = gapRect[i].top + blobRect[j].bottom;

					int pixelCount = 0;
					int heightCount = 0;
					int zmapIndex = 0;
					int imageIndex = 0;
					float zmapData = 0;
					UCHAR imgData = 0;
					for(int y = stY; y < endY; y++)
					{
						for(int x = stX; x < endX; x++)
						{
							zmapIndex = (sizeX * y) + x;
							imageIndex = (rotateWidth * (y - gapRect[i].top)) + (x - gapRect[i].left);
							zmapData = zmapRoiData[zmapIndex];
							imgData = userBlob[imageIndex];

							if(imgData == 255)
							{
								pixelCount++;
								if(zmapData >= hThreshold/* && zmapData <= hThreshold_High*/)
								{
									heightCount++;
								}
							}
						}
					}

					double ratio = ((double)heightCount / (double)pixelCount) * 100.0;

					if(ratio > 50) //50%이상이면 실제 bridge
					{
						rst = TRUE;
						break;
					}
				}
				else
				{
					rst = TRUE;
					break;
				}
			}

			//huj 2014/01/14
			bridgeFlag = FALSE;
		}

		//////////////////////////////////////////////////////////////////////////

		result[i] = rst;

		if(rst == TRUE)
			ret = TRUE;


		/*delete userBlob;
		delete blobRect;
		delete blobArea;*/
		g_pMManager->pem_delete(userBlob, false);
		g_pMManager->pem_delete(blobRect, false);
		g_pMManager->pem_delete(blobArea, false);

		FreeMilImageBuff(&milTemp);
		FreeMilImageBuff(&milBin);
		FreeMilImageBuff(&milRotate);
	}
	memcpy_s(retResult, (sizeof(BOOL) * gapCnts), result, (sizeof(BOOL) * gapCnts));


	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milSrc_Mid);
	FreeMilImageBuff(&milSrc_Bot);


	//delete result;
	g_pMManager->pem_delete(result, false);

	return ret;
}


int CProcMil_LeadSolder::CalcBlob(UCHAR* userBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob, BOOL fillHole, UCHAR* userDst)
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


int CProcMil_LeadSolder::CalcBlob(Im::PIL_ID milBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob, BOOL fillHole, UCHAR* userDst)
{
	MIL_INT count = 0;
	Im::PIL_ID milDst = M_NULL;

	if(fillHole)
	{
		FillHoleBlob(milBinSrc, milBinSrc, m_milBlobResult);
	}

	if(eraseBorderBlob)
	{
		EraseBorderBlob(milBinSrc, milBinSrc, m_milBlobResult);
	}


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
		milDst = AllocBuff(sizeX, sizeY);
		Im::Buf::Copy(milBinSrc,milDst);
		Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
		Im::Blob::blobFill(m_milBlobResult, milDst, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw

		Im::Buf::Get(milDst, userDst);
		SaveWorkImg(milDst, _T("Blob_milDst.bmp"));
	}


	//get result
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);

	FreeMilImageBuff(&milDst);

	return (int)count;
}


void CProcMil_LeadSolder::GetBlobResultRect(CRect* retRect, double* area)
{
	MIL_INT blobCount = 0;
	Im::Blob::blobGetNumber(m_milBlobResult,  &blobCount);

	//CRect* rect = new CRect[(int)blobCount];
	CRect* rect = g_pMManager->pem_new<CRect>(true, (int)blobCount, (PCHAR)__FUNCTION__, __LINE__);
	memset(rect, 0, sizeof(CRect) * (int)blobCount);

	//double* boxLeft = new double[(int)blobCount];
	double* boxLeft = g_pMManager->pem_new<double>(true, (int)blobCount, (PCHAR)__FUNCTION__, __LINE__);
	memset(boxLeft, 0, sizeof(double) * (int)blobCount);
	//double* boxRight = new double[(int)blobCount];
	double* boxRight = g_pMManager->pem_new<double>(true, (int)blobCount, (PCHAR)__FUNCTION__, __LINE__);
	memset(boxRight, 0, sizeof(double) * (int)blobCount);
	//double* boxTop = new double[(int)blobCount];
	double* boxTop = g_pMManager->pem_new<double>(true, (int)blobCount, (PCHAR)__FUNCTION__, __LINE__);
	memset(boxTop, 0, sizeof(double) * (int)blobCount);
	//double* boxBottom = new double[(int)blobCount];
	double* boxBottom = g_pMManager->pem_new<double>(true, (int)blobCount, (PCHAR)__FUNCTION__, __LINE__);
	memset(boxBottom, 0, sizeof(double) * (int)blobCount);

	Im::Blob::blobGet_BOX(m_milBlobResult, boxLeft,boxTop,boxRight,boxBottom);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, boxLeft);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, boxRight);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, boxTop);
// 	Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, boxBottom);

	for(int i = 0; i <  blobCount; i++)
	{
		rect[i].left = (LONG)boxLeft[i];
		rect[i].right = (LONG)boxRight[i] + 1;  //huj 2013/12/02
		rect[i].top = (LONG)boxTop[i];
		rect[i].bottom = (LONG)boxBottom[i] + 1;  //huj 2013/12/02
	}

	memcpy_s(retRect, sizeof(CRect) * (int)blobCount, rect, sizeof(CRect) * (int)blobCount);


	if(area != NULL)
		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);	


	/*delete boxLeft;
	delete boxRight;
	delete boxTop;
	delete boxBottom;
	delete rect;*/
	g_pMManager->pem_delete(boxLeft, false);
	g_pMManager->pem_delete(boxRight, false);
	g_pMManager->pem_delete(boxTop, false);
	g_pMManager->pem_delete(boxBottom, false);
	g_pMManager->pem_delete(rect, false);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CProcMil_LeadSolder::GetImage_HV(UCHAR* topImg, UCHAR* bottomImg, int sizeX, int sizeY, UCHAR* dst)
{
	Im::PIL_ID milTop = M_NULL;
	Im::PIL_ID milBottom = M_NULL;
	Im::PIL_ID milDst = M_NULL;
	milDst = AllocBuff(sizeX, sizeY, 0);

	milTop = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milTop, topImg);
	milBottom = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milBottom, bottomImg);

	//ArithImage(milBottom, milTop, milDst, M_SUB  + M_SATURATION);
	ArithImage(milTop, milBottom, milDst, M_ADD+ M_SATURATION); //shk 2014/01/16 OR->ADD
	Im::Buf::Get(milDst, dst);


	FreeMilImageBuff(&milTop);
	FreeMilImageBuff(&milBottom);
	FreeMilImageBuff(&milDst);

	/*UCHAR* dstTemp = new UCHAR[sizeX * sizeY];
	memset(dstTemp, 0, sizeof(UCHAR) * sizeX * sizeY);

	int index = 0;
	UCHAR topData = 0;
	UCHAR bottomData = 0;
	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			index = (y * sizeX) + x;
			topData = topImg[index];
			bottomData = bottomImg[index];

			if(topData >= bottomData)
			{
				dstTemp[index] = 0;
			}
			else if(abs(bottomData - topData) <= 50)
			{
				dstTemp[index] = 0;
			}
			else
			{
				dstTemp[index] = bottomData;
			}
			
		}
	}

	memcpy_s(dst, sizeof(UCHAR) * sizeX * sizeY, dstTemp, sizeof(UCHAR) * sizeX * sizeY);
	delete dstTemp;*/
}

//shk 2014/01/16
void CProcMil_LeadSolder::GetImage_HV(UCHAR* topImg, UCHAR* midImg,UCHAR* bottomImg, int sizeX, int sizeY, UCHAR* dst, int threshold)
{
	//int threshold = 90;

	int roiSize = sizeX * sizeY; 
	UCHAR* tempTop = NULL;
	UCHAR* tempMid = NULL;
	UCHAR* tempBot = NULL;

	/*tempTop = new UCHAR[roiSize];
	tempMid = new UCHAR[roiSize];
	tempBot = new UCHAR[roiSize];*/
	tempTop = g_pMManager->pem_new<UCHAR>(true, roiSize, (PCHAR)__FUNCTION__, __LINE__);
	tempMid = g_pMManager->pem_new<UCHAR>(true, roiSize, (PCHAR)__FUNCTION__, __LINE__);
	tempBot = g_pMManager->pem_new<UCHAR>(true, roiSize, (PCHAR)__FUNCTION__, __LINE__);

	GetBinaryImg_ipp(topImg, sizeX, sizeY, tempTop, threshold, 0);
	GetBinaryImg_ipp(midImg, sizeX, sizeY, tempMid, threshold, 0);
	GetBinaryImg_ipp(bottomImg, sizeX, sizeY, tempBot, threshold, 0);

	//////////////////////////////////////////////////////////////////////////
	Im::PIL_ID milTop = M_NULL;
	Im::PIL_ID milMid = M_NULL;
	Im::PIL_ID milBottom = M_NULL;
	Im::PIL_ID milDst = M_NULL;
	milDst = AllocBuff(sizeX, sizeY, 0);

	milTop = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milTop, tempTop);
	//delete tempTop;
	g_pMManager->pem_delete(tempTop, false);

	milMid = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milMid, tempMid);
	//delete tempMid;
	g_pMManager->pem_delete(tempMid, false);

	milBottom = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milBottom, tempBot);
	//delete tempBot;
	g_pMManager->pem_delete(tempBot, false);

	SaveWorkImg(milTop, _T("milTop.bmp"));
	SaveWorkImg(milMid, _T("milMid.bmp"));
	SaveWorkImg(milBottom, _T("milBottom.bmp"));

//	ArithImage(milBottom, milTop, milDst, M_SUB  + M_SATURATION);
	ArithImage(milTop, milMid, milDst, M_ADD+ M_SATURATION);
	ArithImage(milDst, milBottom, milDst, M_ADD+ M_SATURATION);

	Im::Buf::Get(milDst, dst);

	FreeMilImageBuff(&milTop);
	FreeMilImageBuff(&milMid);
	FreeMilImageBuff(&milBottom);
	FreeMilImageBuff(&milDst);

}

#if 0
//shk 2014/01/16
void CProcMil_LeadSolder::GetImage_HV(UCHAR* topImg, UCHAR* midImg,UCHAR* bottomImg, int sizeX, int sizeY, UCHAR* dst)
{
	Im::PIL_ID milTop = M_NULL;
	Im::PIL_ID milMid = M_NULL;
	Im::PIL_ID milBottom = M_NULL;
	Im::PIL_ID milDst = M_NULL;
	milDst = AllocBuff(sizeX, sizeY, 0);

	milTop = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milTop, topImg);

	milMid = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milMid, midImg);

	milBottom = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milBottom, bottomImg);

	SaveWorkImg(milTop, _T("milTop.bmp"));
	SaveWorkImg(milMid, _T("milMid.bmp"));
	SaveWorkImg(milBottom, _T("milBottom.bmp"));

	//ArithImage(milBottom, milTop, milDst, M_SUB  + M_SATURATION);
	ArithImage(milTop, milMid, milDst, M_ADD+ M_SATURATION);
	ArithImage(milDst, milBottom, milDst, M_ADD+ M_SATURATION);

	Im::Buf::Get(milDst, dst);

	FreeMilImageBuff(&milTop);
	FreeMilImageBuff(&milMid);
	FreeMilImageBuff(&milBottom);
	FreeMilImageBuff(&milDst);

}
#endif


#if 0
LeadInfoTemp CProcMil_LeadSolder::CalcLeadRect(UCHAR* topImg, UCHAR* bottomImg, UCHAR* hvImg, float* ZmapRoiData, int sizeX, int sizeY, CRect** retRect, BOOL isContainer, BOOL isInspLeadLift, int leadStartPos, float leadHeight, double leadWidth) //shk 2014-01-03
{
	LeadInfoTemp leadInfo;
	//////////////////////////////////////////////////////////////////////////
	leadInfo.countLead = 0;
	leadInfo.avgLead = 0;
	leadInfo.maxLead = 0;
	leadInfo.minLead = 0;

	leadInfo.countGap = 0;
	leadInfo.avgGap = 0;
	leadInfo.maxGap = 0;
	leadInfo.minGap = 0;

	leadInfo.leadPitch = 0;
	//////////////////////////////////////////////////////////////////////////
	CRect* leadRect = NULL;

	Im::PIL_ID milSrcTop = M_NULL;
	milSrcTop = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrcTop, topImg);

	Im::PIL_ID milSrcBottom = M_NULL;
	milSrcBottom = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrcBottom, bottomImg);

	Im::PIL_ID milSrcHV = M_NULL;
	milSrcHV = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrcHV, hvImg);

	MedianFilter(milSrcTop, milSrcTop, TRUE);
	SaveWorkImg(milSrcTop, _T("medianTop.bmp"));

	MorUserErode_H(milSrcTop, milSrcTop, 2, TRUE);
	MorUserDilate_H(milSrcTop, milSrcTop, 2, TRUE);
	SaveWorkImg(milSrcTop, _T("morTop.bmp"));


	MedianFilter(milSrcBottom, milSrcBottom, TRUE);
	SaveWorkImg(milSrcBottom, _T("medianBottom.bmp"));

	MorUserErode_H(milSrcBottom, milSrcBottom, 2, TRUE);
	MorUserDilate_H(milSrcBottom, milSrcBottom, 2, TRUE);
	SaveWorkImg(milSrcBottom, _T("morBottom.bmp"));

	MedianFilter(milSrcHV, milSrcHV, TRUE);
	SaveWorkImg(milSrcHV, _T("medianHV.bmp"));

	MorUserErode_H(milSrcHV, milSrcHV, 2, TRUE);
	MorUserDilate_H(milSrcHV, milSrcHV, 2, TRUE);
	SaveWorkImg(milSrcHV, _T("morHV.bmp"));

	////////////////////////////////////////////////////////////////////////// 좌우..
	long* prjList = new long[sizeX];
	//memset(prjList, 0, sizeof(long)* sizeX);

	//GetProjection_H(milSrcBottom, prjList);
	GetProjection_H(milSrcHV, prjList);

	double sumTemp = 0.;
	int avrTemp = 0;

	for(int i = 0; i < sizeX; i++)
	{
		sumTemp += prjList[i];
		prjList[i] /= sizeY;
	}

	avrTemp = (int)(sumTemp / sizeX);
	BOOL widthFlag = TRUE;

	if(avrTemp < 6000)
		widthFlag = FALSE;

	long* prjHisto = new long[256];
	memset(prjHisto, 0, sizeof(long)* 256);

	GetHistoValue(prjList, sizeX, 256, prjHisto);
	long heightThreshold = CalcAutoThreshold(prjHisto, 256, 0, 255);
	heightThreshold = (long)(heightThreshold * 0.9);

	for (int index = 0; index < sizeX; index ++)
	{
		if (prjList[index] < heightThreshold)
			prjList[index] = 0;	
		else
			prjList[index] = 255L;
	}

	//////////////////////////////////////////////////////////////////////////

		CalcRunLength(sizeX, prjList, 0, widthFlag);
	int leadCount = m_countLead;

	//shk 2014/01/13 위치이동 ( leadCount < 1 일때 문제 생김 )
	delete prjList;
	delete prjHisto;

	if(leadCount < 1)
	{
		FreeMilImageBuff(&milSrcTop);
		FreeMilImageBuff(&milSrcBottom);
		FreeMilImageBuff(&milSrcHV);
		return leadInfo;
	}

	leadRect = new CRect[leadCount];
	memset(leadRect, 0, sizeof(CRect) * leadCount);

	for(int i = 0; i < leadCount; i++)
	{
		leadRect[i].left = (LONG)m_vtrLandPos.at(i);
		leadRect[i].right = (LONG)(leadRect[i].left + m_vtrLand.at(i));
		leadRect[i].top = 0; //아직 모름..
		leadRect[i].bottom = sizeY;	//아직 모름..
	}



	////////////////////////////////////////////////////////////////////////// 상
	int clipCx = 0;
	int clipCy = 0;


	Im::PIL_ID milClip_Top = M_NULL;
	Im::PIL_ID milClip_Bottom = M_NULL;
	

	long* prjListTop = NULL;
	long* prjListBottom = NULL;
	int size = 0;
	for(int i = 0; i < leadCount; i++)
	{
		double sum_top = 0;
		double sum_bottom = 0;
		long avr_top = 0;
		long avr_bottom = 0;

		clipCx = (int)(leadRect[i].left + (leadRect[i].Width() / 2.0));
		clipCy = (int)(leadRect[i].top + (leadRect[i].Height() / 2.0));

		milClip_Top = AllocClipBuff(milSrcTop, clipCx, clipCy,  leadRect[i].Width(), leadRect[i].Height());
		//milClip_Bottom = AllocClipBuff(milSrcBottom, clipCx, clipCy,  leadRect[i].Width(), leadRect[i].Height());
		milClip_Bottom = AllocClipBuff(milSrcHV, clipCx, clipCy,  leadRect[i].Width(), leadRect[i].Height());

 		size = leadRect[i].Height();
 		prjListTop = new long[size];
		prjListBottom = new long[size];

		GetProjection_V(milClip_Top, prjListTop);
		GetProjection_V(milClip_Bottom, prjListBottom);

		for(int j = 0; j < size; j++)
		{
			prjListTop[j] /= leadRect[i].Width();
			sum_top += prjListTop[j];

			prjListBottom[j] /= leadRect[i].Width();
			sum_bottom += prjListBottom[j];
		}

		avr_top = (long)(sum_top / size);
		avr_bottom = (long)(sum_bottom / size);


		//각 값에서 평균을 뺀 값이 양수인 지점 찾기...

		long leadStartPoint = 0;
		long dataTemp_top = 0;
		long dataTemp_bottom = 0;
		for(int j = 0; j < size; j++)
		{
			dataTemp_top = prjListTop[j] - avr_top;
			dataTemp_bottom = prjListBottom[j] - avr_bottom;

			if(dataTemp_top >= 0 || dataTemp_bottom >= 0)
			{
				leadStartPoint = j;
				break;
			}
		}

		leadRect[i].top = (LONG)leadStartPoint;
		

/*
#ifdef FALSE
		//////////////////////////////////////////////////////////////////////////
		FILE	*fp;
		wchar_t	fileName[256];
		wsprintf(fileName, _T("d:\\testimage\\data\\leadData%d.csv"), i);

		DeleteFile(fileName);
		_wfopen_s(&fp,fileName, _T("at"));
		//fp = _wfopen(fileName, _T("at"));
		if (!fp) 
		{
			AfxMessageBox(_T("already opened..."));
			return leadInfo;
		}


		int temp = 0;

		fwprintf(fp, _T("Index,top,bottom\n"));	
		for(int k = 0; k < size ;k++)
		{
			fwprintf(fp, _T("%d,%d,%d\n"),k , prjListTop[k], prjListBottom[k] );			
		}



		fclose(fp);

		//////////////////////////////////////////////////////////////////////////
#endif // 저장*/
		

		FreeMilImageBuff(&milClip_Top);
		FreeMilImageBuff(&milClip_Bottom);
		delete prjListTop;
		delete prjListBottom;	
	}
	
	
	//////////////////////////////////////////////////////////////////////////

	*retRect = new CRect[leadCount];
	memcpy_s(*retRect, sizeof(CRect) * leadCount, leadRect, sizeof(CRect) * leadCount);

	

	leadInfo.countLead = m_countLead;
	leadInfo.avgLead = m_avgLead;
	leadInfo.maxLead = m_maxLead;
	leadInfo.minLead = m_minLead;

	leadInfo.countGap = m_countGap;
	leadInfo.avgGap = m_avgGap;
	leadInfo.maxGap = m_maxGap;
	leadInfo.minGap = m_minGap;
	

	double sum = 0;
	for(int i = 0; i < leadCount - 1; i++)
	{
		sum += (m_vtrLandCenterPos.at(i + 1) - m_vtrLandCenterPos.at(i));
	}

	leadInfo.leadPitch = sum / (leadCount - 1);
	//////////////////////////////////////////////////////////////////////////



	delete leadRect;
	FreeMilImageBuff(&milSrcTop);
	FreeMilImageBuff(&milSrcBottom);
	FreeMilImageBuff(&milSrcHV);

	return leadInfo;	
}
#endif



LeadInfoTemp CProcMil_LeadSolder::CalcLeadRect(UCHAR* topImg, UCHAR* hvImg, float* ZmapRoiData, int sizeX, int sizeY, CRect** retRect, int** retLeadPitch, BOOL isContainer, BOOL isInspLeadLift, int leadStartPos, float leadHeight, double leadWidth, BOOL use3D) //shk 2014-01-03
{
	LeadInfoTemp leadInfo;
	//////////////////////////////////////////////////////////////////////////
	leadInfo.countLead = 0;
	leadInfo.avgLead = 0;
	leadInfo.maxLead = 0;
	leadInfo.minLead = 0;

	leadInfo.countGap = 0;
	leadInfo.avgGap = 0;
	leadInfo.maxGap = 0;
	leadInfo.minGap = 0;

	leadInfo.leadPitch = 0;
	//////////////////////////////////////////////////////////////////////////
	CRect* leadRect = NULL;

	Im::PIL_ID milSrcTop = M_NULL;
	milSrcTop = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrcTop, topImg);

	Im::PIL_ID milSrcHV = M_NULL;
	milSrcHV = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrcHV, hvImg);

	MedianFilter(milSrcTop, milSrcTop, TRUE);
	SaveWorkImg(milSrcTop, _T("medianTop.bmp"));	

	MorUserErode_H(milSrcTop, milSrcTop, 2, TRUE);
	MorUserDilate_H(milSrcTop, milSrcTop, 2, TRUE);
	SaveWorkImg(milSrcTop, _T("morTop.bmp"));

	MedianFilter(milSrcHV, milSrcHV, TRUE);
	SaveWorkImg(milSrcHV, _T("medianHV.bmp"));

	MorUserErode_H(milSrcHV, milSrcHV, 2, TRUE);
	MorUserDilate_H(milSrcHV, milSrcHV, 2, TRUE);
	SaveWorkImg(milSrcHV, _T("morHV.bmp"));
	//////////////////////////////////////////////////////////////////////////
	//2014-01-02 shk
	BOOL useZmapFlag = TRUE;
	float* prjZList = NULL;
	Im::PIL_ID milSrcZmap = M_NULL;
	if(use3D == TRUE)
	{	
		int zmapSizeX = sizeX;
		int zmapSizeY = sizeY;

		//huj 2014/01/12
		float zmapPrjThreshold = 100;

		//huj 2014/01/12
		if(isInspLeadLift == TRUE) //들뜸 검사 하는 경우 영역 그대로   //  들뜸 검사 안하는경우 lead start line 부터 .. (들뜸검사를 안하는 경우의 src 영역은 윈도우 전체 이기 떄문에..)
		{
			milSrcZmap = AllocBuff_float(zmapSizeX, zmapSizeY);
			ippsThreshold_LTVal_32f(ZmapRoiData, ZmapRoiData, sizeX * sizeY, 0, 0);
			Im::Buf::Put(milSrcZmap, ZmapRoiData);
		}
		else
		{	
			int zmapClipCx = 0;
			int zmapClipCy = 0;
			zmapSizeY = sizeY - leadStartPos;

			milSrcZmap = AllocBuff_float(zmapSizeX, zmapSizeY);
			ippsThreshold_LTVal_32f(ZmapRoiData, ZmapRoiData, sizeX * sizeY, 0, 0);

			zmapClipCx = (int)(zmapSizeX / 2.0);
			zmapClipCy = (int)(zmapSizeY / 2.0 + leadStartPos);

			//float* zmapTemp = new float[zmapSizeX * zmapSizeY];	
			float* zmapTemp = g_pMManager->pem_new<float>(true, zmapSizeX * zmapSizeY, (PCHAR)__FUNCTION__, __LINE__);
			GetCropZmap(ZmapRoiData, zmapTemp, sizeX, sizeY, zmapClipCx, zmapClipCy, zmapSizeX, zmapSizeY);	
			Im::Buf::Put(milSrcZmap, zmapTemp);

			//delete zmapTemp;
			g_pMManager->pem_delete(zmapTemp, false);
		}
//////////////////////////////////////////////////////////////////////////

		int allowDataCnt = 0;
		//prjZList = new float[zmapSizeX];
		prjZList = g_pMManager->pem_new<float>(true, zmapSizeX, (PCHAR)__FUNCTION__, __LINE__);
		//memset(prjZList, 0, sizeof(float)* zmapSizeX);

		GetProjection_H(milSrcZmap, prjZList);
		for(int i = 0; i < zmapSizeX; i++)
		{
			prjZList[i] /= zmapSizeY;
			if (prjZList[i] < zmapPrjThreshold)
				prjZList[i] = 0;
			else
				allowDataCnt++;
		}

		//huj 2014/01/16

		if((((double)allowDataCnt / (double)zmapSizeX) * 100.0) <= 30.0)
			useZmapFlag = FALSE;


	}

	////////////////////////////////////////////////////////////////////////// 좌우..
	//long* prjList = new long[sizeX];
	long* prjList = g_pMManager->pem_new<long>(true, sizeX, (PCHAR)__FUNCTION__, __LINE__);
	//memset(prjList, 0, sizeof(long)* sizeX);

	GetProjection_H(milSrcHV, prjList);

	double sumTemp = 0.;
	int avrTemp = 0;

	for(int i = 0; i < sizeX; i++)
	{
		sumTemp += prjList[i];
		prjList[i] /= sizeY;
	}

	avrTemp = (int)(sumTemp / sizeX);
	BOOL widthFlag = TRUE;

	if(avrTemp < 6000)
		widthFlag = FALSE;

	//long* prjHisto = new long[256];
	long* prjHisto = g_pMManager->pem_new<long>(true, 256, (PCHAR)__FUNCTION__, __LINE__);
	memset(prjHisto, 0, sizeof(long)* 256);

	GetHistoValue(prjList, sizeX, 256, prjHisto);
	long heightThreshold = CalcAutoThreshold(prjHisto, 256, 0, 255);
	heightThreshold = (long)(heightThreshold * 0.9);

	for (int index = 0; index < sizeX; index ++)
	{
		if (prjList[index] < heightThreshold)
			prjList[index] = 0;	
		else
			prjList[index] = 255L;
	}

	//////////////////////////////////////////////////////////////////////////
	//shk 2014-01-03
	//huj 2014/01/09
	if(use3D == TRUE && useZmapFlag == TRUE)
	{
		int stp = -1;
		int edp = 0;
		if(isContainer == TRUE)
		{
			for(int i = 0; i < sizeX; i++)
			{
				if (prjList[i] == 0 || (long)prjZList[i] == 0)
				{
					prjList[i] = 0L;
				}
				else if(prjList[i] > 0 && (long)prjZList[i] > 0 )
				{
					prjList[i] = 255L;
				}	
			}
		}
		else //리드가 무조건 한개! (양끝점만 찾는다..)
		{	
			for(int i = 0; i < sizeX; i++)
			{
				if(prjList[i] > 0 && (long)prjZList[i] > 0)
				{
					if(stp == -1)
						stp = i;

					edp = i;
				}	

			}

			//계산된 리드 폭이 터무니 없이 작을때..
			if(leadWidth != 0)
			{
				int tempW = edp - stp;
				int stdW = (int)(leadWidth * 0.3);

				if(tempW <= stdW)
				{
					int tempMargin = (int)((sizeX - leadWidth) / 2.0);
					if(tempMargin > 0)
					{
						stp = tempMargin;
						edp = sizeX - stp;
					}
					else
					{
						stp = (int)(sizeX * 0.2);
						edp = sizeX - stp;
					}
				}
			}




			for(int i = 0; i < sizeX; i++)
			{
				if(i >= stp && i <= edp)
					prjList[i] = 255L;
				else
					prjList[i] = 0L;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	CalcRunLength(sizeX, prjList, 0, widthFlag);
	int leadCount = m_countLead;

	//shk 2014/01/13 위치이동 ( leadCount < 1 일때 문제 생김 )
	/*delete prjList;
	delete prjHisto;*/
	g_pMManager->pem_delete(prjList, false);
	g_pMManager->pem_delete(prjHisto, false);

	if(prjZList != NULL)
		//delete prjZList;
		g_pMManager->pem_delete(prjZList, false);

	if(leadCount < 1)
	{
		FreeMilImageBuff(&milSrcTop);
		//FreeMilImageBuff(&milSrcBottom);  //huj 2014/02/24   bottom 영상은 사용 하지 않아 주석 처리함.
		FreeMilImageBuff(&milSrcHV);
		
		if(milSrcZmap != M_NULL)
			FreeMilImageBuff(&milSrcZmap); //shk 2014-01-03

		return leadInfo;
	}

	//leadRect = new CRect[leadCount];
	leadRect = g_pMManager->pem_new<CRect>(true, leadCount, (PCHAR)__FUNCTION__, __LINE__);
	memset(leadRect, 0, sizeof(CRect) * leadCount);

	for(int i = 0; i < leadCount; i++)
	{
		leadRect[i].left = (LONG)m_vtrLandPos.at(i);
		leadRect[i].right = (LONG)(leadRect[i].left + m_vtrLand.at(i));
		leadRect[i].top = 0; //아직 모름..
		leadRect[i].bottom = sizeY;	//아직 모름..
	}



	////////////////////////////////////////////////////////////////////////// 상
	int clipCx = 0;
	int clipCy = 0;


	Im::PIL_ID milClip_Top = M_NULL;
	Im::PIL_ID milClip_Bottom = M_NULL;
	

	long* prjListTop = NULL;
	long* prjListBottom = NULL;
	int size = 0;
	for(int i = 0; i < leadCount; i++)
	{
		double sum_top = 0;
		double sum_bottom = 0;
		long avr_top = 0;
		long avr_bottom = 0;

		clipCx = (int)(leadRect[i].left + (leadRect[i].Width() / 2.0));
		clipCy = (int)(leadRect[i].top + (leadRect[i].Height() / 2.0));

		milClip_Top = AllocClipBuff(milSrcTop, clipCx, clipCy,  leadRect[i].Width(), leadRect[i].Height());
		milClip_Bottom = AllocClipBuff(milSrcHV, clipCx, clipCy,  leadRect[i].Width(), leadRect[i].Height());

 		size = leadRect[i].Height();
 		/*prjListTop = new long[size];
		prjListBottom = new long[size];*/
		prjListTop = g_pMManager->pem_new<long>(true, size, (PCHAR)__FUNCTION__, __LINE__);
		prjListBottom = g_pMManager->pem_new<long>(true, size, (PCHAR)__FUNCTION__, __LINE__);

		GetProjection_V(milClip_Top, prjListTop);
		GetProjection_V(milClip_Bottom, prjListBottom);

		for(int j = 0; j < size; j++)
		{
			prjListTop[j] /= leadRect[i].Width();
			sum_top += prjListTop[j];

			prjListBottom[j] /= leadRect[i].Width();
			sum_bottom += prjListBottom[j];
		}

		avr_top = (long)(sum_top / size);
		avr_bottom = (long)(sum_bottom / size);


		//각 값에서 평균을 뺀 값이 양수인 지점 찾기...

		long leadStartPoint = 0;
		long dataTemp_top = 0;
		long dataTemp_bottom = 0;
		for(int j = 0; j < size; j++)
		{
			dataTemp_top = prjListTop[j] - avr_top;
			dataTemp_bottom = prjListBottom[j] - avr_bottom;

			if(dataTemp_top >= 0 || dataTemp_bottom >= 0)
			{
				leadStartPoint = j;
				break;
			}
		}

		leadRect[i].top = (LONG)leadStartPoint;
		
		FreeMilImageBuff(&milClip_Top);
		FreeMilImageBuff(&milClip_Bottom);
		/*delete prjListTop;
		delete prjListBottom;	*/
		g_pMManager->pem_delete(prjListTop, false);
		g_pMManager->pem_delete(prjListBottom, false);
	}
	
	
	//////////////////////////////////////////////////////////////////////////

	//*retRect = new CRect[leadCount];
	*retRect = g_pMManager->pem_new<CRect>(true, leadCount, (PCHAR)__FUNCTION__, __LINE__);
	memcpy_s(*retRect, sizeof(CRect) * leadCount, leadRect, sizeof(CRect) * leadCount);

	

	leadInfo.countLead = m_countLead;
	leadInfo.avgLead = m_avgLead;
	leadInfo.maxLead = m_maxLead;
	leadInfo.minLead = m_minLead;

	leadInfo.countGap = m_countGap;
	leadInfo.avgGap = m_avgGap;
	leadInfo.maxGap = m_maxGap;
	leadInfo.minGap = m_minGap;

	if(m_countGap > 0)
	{
		//int* leadPitch = new int[m_countGap];
		int* leadPitch = g_pMManager->pem_new<int>(true, m_countGap, (PCHAR)__FUNCTION__, __LINE__);
		double sum = 0;
		for(int i = 0; i < m_countGap; i++)
		{
			leadPitch[i] = (m_vtrLandCenterPos.at(i + 1) - m_vtrLandCenterPos.at(i));
			sum += leadPitch[i];
		}

		leadInfo.leadPitch = sum / m_countGap;

		//*retLeadPitch = new int[m_countGap];
		*retLeadPitch = g_pMManager->pem_new<int>(true, m_countGap, (PCHAR)__FUNCTION__, __LINE__);
		memcpy_s(*retLeadPitch, sizeof(int) * m_countGap, leadPitch, sizeof(int) * m_countGap);
		//delete leadPitch;
		g_pMManager->pem_delete(leadPitch, false);
	}
	else
	{
		leadInfo.leadPitch = 0;
	}
	//////////////////////////////////////////////////////////////////////////

	//delete leadRect;
	g_pMManager->pem_delete(leadRect, false);
	FreeMilImageBuff(&milSrcTop);
	//FreeMilImageBuff(&milSrcBottom);  //huj 2014/02/24   bottom 영상은 사용 하지 않아 주석 처리함.
	FreeMilImageBuff(&milSrcHV);

	if(milSrcZmap != M_NULL)
		FreeMilImageBuff(&milSrcZmap);//shk 2014-01-03

	return leadInfo;	
}

int CompareCRectX( const void* pElement1, const void* pElement2 )
{
	CRect *pElem1 = (CRect*) pElement1;
	CRect *pElem2 = (CRect*) pElement2;

	CPoint posElem1 = pElem1->CenterPoint();
	CPoint posElem2 = pElem2->CenterPoint();

	if( posElem1.x < posElem2.x )
	{
		return -1;
	} else if ( posElem1 == posElem2 ) {
		return 0;
	} else {
		return 1;
	}

	return 0;
}

LeadInfoTemp CProcMil_LeadSolder::CalcLeadRect2(UCHAR* topImg, UCHAR* bottomImg, UCHAR* hvImg, float* ZmapRoiData, int sizeX, int sizeY, CRect** retRect, int** retLeadPitch, BOOL isContainer, BOOL isInspLeadLift, int leadStartPos, float leadHeight, double stdLeadWidth, int stdLeadCount, BOOL use3D) //shk 2014-01-03
{
	LeadInfoTemp leadInfo;
	//////////////////////////////////////////////////////////////////////////
	leadInfo.countLead = 0;
	leadInfo.avgLead = 0;
	leadInfo.maxLead = 0;
	leadInfo.minLead = 0;

	leadInfo.countGap = 0;
	leadInfo.avgGap = 0;
	leadInfo.maxGap = 0;
	leadInfo.minGap = 0;

	leadInfo.leadPitch = 0;
	//////////////////////////////////////////////////////////////////////////
	CRect* leadRect = NULL;

	Im::PIL_ID milSrcTop = M_NULL;
	milSrcTop = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrcTop, topImg);

	//huj 2014/02/24   bottom 영상은 사용 하지 않아 주석 처리함.
	// 	Im::PIL_ID milSrcBottom = M_NULL;
	// 	milSrcBottom = AllocBuff(sizeX, sizeY);
	// 	Im::Buf::Put(milSrcBottom, bottomImg);

	Im::PIL_ID milSrcHV = M_NULL;
	milSrcHV = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrcHV, hvImg);

	MedianFilter(milSrcTop, milSrcTop, TRUE);
	SaveWorkImg(milSrcTop, _T("medianTop.bmp"));	

	MorUserErode_H(milSrcTop, milSrcTop, 2, TRUE);
	MorUserDilate_H(milSrcTop, milSrcTop, 2, TRUE);
	SaveWorkImg(milSrcTop, _T("morTop.bmp"));

	//huj 2014/02/24   bottom 영상은 사용 하지 않아 주석 처리함.
	// 	MedianFilter(milSrcBottom, milSrcBottom, TRUE);
	// 	SaveWorkImg(milSrcBottom, _T("medianBottom.bmp"));
	// 
	// 	MorUserErode_H(milSrcBottom, milSrcBottom, 2, TRUE);
	// 	MorUserDilate_H(milSrcBottom, milSrcBottom, 2, TRUE);
	// 	SaveWorkImg(milSrcBottom, _T("morBottom.bmp"));


	MedianFilter(milSrcHV, milSrcHV, TRUE);
	SaveWorkImg(milSrcHV, _T("medianHV.bmp"));

	MorUserErode_H(milSrcHV, milSrcHV, 2, TRUE);
	MorUserDilate_H(milSrcHV, milSrcHV, 2, TRUE);
	SaveWorkImg(milSrcHV, _T("morHV.bmp"));
	//////////////////////////////////////////////////////////////////////////
	//2014-01-02 shk
	BOOL useZmapFlag = TRUE;
	float* prjZList = NULL;
	Im::PIL_ID milSrcZmap = M_NULL;
	if(use3D == TRUE)
	{	
		float* zmapSrc = NULL;
		//zmapSrc = new float[sizeX * sizeY];
		zmapSrc = g_pMManager->pem_new<float>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
		ippsThreshold_LTVal_32f(ZmapRoiData, zmapSrc, sizeX * sizeY, 0, 0);

		int zmapClipCx = 0;
		int zmapClipCy = 0;
		int zmapSizeX = sizeX;
		int zmapSizeY = sizeY;

		//huj 2014/01/12
		float zmapPrjThreshold = 100;

		//huj 2014/01/12
		double sumTemp_top = 0;
		float avrHTemp_top = 0;
		if(isInspLeadLift == TRUE) //들뜸 검사 하는 경우 0부터 start line까지  //  들뜸 검사 안하는경우 lead start line 부터 .. (들뜸검사를 안하는 경우의 src 영역은 윈도우 전체 이기 떄문에..)
		{

			//////////////////////////////////////////////////////////////////////////	
			//검사영역안에 body가 포함되었는지 여부를 판단하기 위한 작업들.. (body가 들어가면 projection 할때 데이터가 뭉게질 수 있음)
			for(int i = 0; i < sizeX; i++)
			{
				sumTemp_top += zmapSrc[i];
			}
			avrHTemp_top = (float)(sumTemp_top / sizeX * 0.9);

			sumTemp_top = 0;
			for(int i = 0; i < sizeX; i++)
			{
				if(zmapSrc[i] >= avrHTemp_top)
					sumTemp_top++;
			}

			bool bodyDelFlag = false;
			if(sumTemp_top >= sizeX * 0.8)
				bodyDelFlag = true;

			int bodyTipIndex = 0;
			if(bodyDelFlag)//검사 영역에 포함 되어있는 body를 제거하기 위해 body위치를 찾는다. (포함 되어있다 -> bodyDelFlag=true)
			{
				Im::PIL_ID milTemp = AllocBuff_float(sizeX, sizeY);
				Im::Buf::Put(milTemp, zmapSrc);

				//float* proFile = new float[sizeY];
				float* proFile = g_pMManager->pem_new<float>(true, sizeY, (PCHAR)__FUNCTION__, __LINE__);
				GetProjection_V(milTemp, proFile);

				proFile[0] /= sizeX;
				for(int i = 1; i < sizeY; i++)
				{
					proFile[i] /= sizeX;
					if(proFile[i-1] >= avrHTemp_top)
					{
						if(proFile[i] < avrHTemp_top)
						{
							bodyTipIndex = i-1;
							break;
						}
					}
				}

				FreeMilImageBuff(&milTemp);
				//delete proFile;
				g_pMManager->pem_delete(proFile, false);
			}
			//////////////////////////////////////////////////////////////////////////
			if(bodyTipIndex != 0)
				bodyTipIndex += 10;

			zmapSizeY = leadStartPos - bodyTipIndex;
			if(zmapSizeY <= 0)
			{
				zmapSizeY = leadStartPos;
				bodyTipIndex = 0;
			}
			 
			zmapClipCx = (int)(zmapSizeX / 2.0);
			zmapClipCy = (int)((zmapSizeY / 2.0) + bodyTipIndex);

//녹색선 위로 10픽셀만 본다.
// 			zmapSizeY = 10;
// 			int tempVal = leadStartPos - zmapSizeY; 
// 
// 			if(tempVal < 0)
// 			{
// 				zmapSizeY = leadStartPos;
// 				zmapClipCy = (int)(zmapSizeY / 2.0);
// 			}
// 			else
// 			{			
// 				zmapClipCy = (int)(zmapSizeY / 2.0 + tempVal);
// 			}	
// 
// 			zmapClipCx = (int)(zmapSizeX / 2.0);
			//////////////////////////////////////////////////////////////////////////
		}
		else
		{				
			zmapSizeY = sizeY - leadStartPos;			

			zmapClipCx = (int)(zmapSizeX / 2.0);
			zmapClipCy = (int)(zmapSizeY / 2.0 + leadStartPos);
		}
			

		//float* zmapTemp = new float[zmapSizeX * zmapSizeY];	
		float* zmapTemp = g_pMManager->pem_new<float>(true, zmapSizeX * zmapSizeY, (PCHAR)__FUNCTION__, __LINE__);
		GetCropZmap(zmapSrc, zmapTemp, sizeX, sizeY, zmapClipCx, zmapClipCy, zmapSizeX, zmapSizeY);	

		milSrcZmap = AllocBuff_float(zmapSizeX, zmapSizeY);
		Im::Buf::Put(milSrcZmap, zmapTemp);

		/*delete zmapTemp;
		delete zmapSrc;*/
		g_pMManager->pem_delete(zmapTemp, false);
		g_pMManager->pem_delete(zmapSrc, false);
		//////////////////////////////////////////////////////////////////////////

		
		int allowDataCnt = 0;
		//prjZList = new float[zmapSizeX];
		prjZList = g_pMManager->pem_new<float>(true, zmapSizeX, (PCHAR)__FUNCTION__, __LINE__);
		//memset(prjZList, 0, sizeof(float)* zmapSizeX);

		GetProjection_H(milSrcZmap, prjZList);
		FreeMilImageBuff(&milSrcZmap);
		
		Im::PIL_ID milPrj = M_NULL;
		milPrj = AllocBuff_float(zmapSizeX, 1);
		Im::Buf::Put(milPrj, prjZList);
		zmapPrjThreshold = (float)GetThreshold(milPrj);
		FreeMilImageBuff(&milPrj);

		zmapPrjThreshold /= zmapSizeY;

		for(int i = 0; i < zmapSizeX; i++)
		{
			if(prjZList[i] != 0 && zmapSizeY != 0)
				prjZList[i] /= zmapSizeY;

			if (prjZList[i] < zmapPrjThreshold)
				prjZList[i] = 0;
			else
				allowDataCnt++;
		}

		//huj 2014/01/16

		if((((double)allowDataCnt / (double)zmapSizeX) * 100.0) <= 30.0)
			useZmapFlag = FALSE;
	}

	////////////////////////////////////////////////////////////////////////// 좌우..
	//long* prjList = new long[sizeX];
	long* prjList = g_pMManager->pem_new<long>(true, sizeX, (PCHAR)__FUNCTION__, __LINE__);
	GetProjection_H(milSrcHV, prjList);

	double sumTemp = 0.;
	int avrTemp = 0;
	for(int i = 0; i < sizeX; i++)
	{
		sumTemp += prjList[i];
		prjList[i] /= sizeY;
	}

	avrTemp = (int)(sumTemp / sizeX);
	BOOL widthFlag = TRUE;

	if(avrTemp < 6000)
		widthFlag = FALSE;

	//long* prjHisto = new long[256];
	long* prjHisto = g_pMManager->pem_new<long>(true, 256, (PCHAR)__FUNCTION__, __LINE__);
	memset(prjHisto, 0, sizeof(long)* 256);

	GetHistoValue(prjList, sizeX, 256, prjHisto);
	long heightThreshold = CalcAutoThreshold(prjHisto, 256, 0, 255);
	heightThreshold = (long)(heightThreshold * 0.9);

	for (int index = 0; index < sizeX; index ++)
	{
		if (prjList[index] < heightThreshold)
			prjList[index] = 0;	
		else
			prjList[index] = 255L;
	}

	//////////////////////////////////////////////////////////////////////////
	//shk 2014-01-03
	//huj 2014/01/09
	if(use3D == TRUE && useZmapFlag == TRUE)
	{
		int stp = -1;
		int edp = 0;
		if(isContainer == TRUE)
		{
			for(int i = 0; i < sizeX; i++)
			{
				if (prjList[i] == 0 || (long)prjZList[i] == 0)
				{
					prjList[i] = 0L;
				}
				else if(prjList[i] > 0 && (long)prjZList[i] > 0 )
				{
					prjList[i] = 255L;
				}	
			}
		}
		else //리드가 무조건 한개! (양끝점만 찾는다..)
		{	
			for(int i = 0; i < sizeX; i++)
			{
				if(prjList[i] > 0 && (long)prjZList[i] > 0)
				{
					if(stp == -1)
						stp = i;

					edp = i;
				}	

			}

			//계산된 리드 폭이 터무니 없이 작을때..
			if(stdLeadWidth != 0)
			{
				int tempW = edp - stp;
				int stdW = (int)(stdLeadWidth * 0.3);

				if(tempW <= stdW)
				{
					int tempMargin = (int)((sizeX - stdLeadWidth) / 2.0);
					if(tempMargin > 0)
					{
						stp = tempMargin;
						edp = sizeX - stp;
					}
					else
					{
						stp = (int)(sizeX * 0.2);
						edp = sizeX - stp;
					}
				}
			}


			for(int i = 0; i < sizeX; i++)
			{
				if(i >= stp && i <= edp)
					prjList[i] = 255L;
				else
					prjList[i] = 0L;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	
	int* leadPitchTemp = NULL;
	CalcRunLength2(sizeX, prjList, 0, widthFlag, stdLeadCount,stdLeadWidth, &leadPitchTemp);
	int leadCount = m_countLead;

	//shk 2014/01/13 위치이동 ( leadCount < 1 일때 문제 생김 )
	/*delete prjList;
	delete prjHisto;*/
	g_pMManager->pem_delete(prjList, false);
	g_pMManager->pem_delete(prjHisto, false);

	if(prjZList != NULL)
		//delete prjZList;
		g_pMManager->pem_delete(prjZList, false);

	if(leadCount < 1)
	{
		FreeMilImageBuff(&milSrcTop);
		//FreeMilImageBuff(&milSrcBottom);  //huj 2014/02/24   bottom 영상은 사용 하지 않아 주석 처리함.
		FreeMilImageBuff(&milSrcHV);

		return leadInfo;
	}

	//leadRect = new CRect[leadCount];
	leadRect = g_pMManager->pem_new<CRect>(true, leadCount, (PCHAR)__FUNCTION__, __LINE__);
	memset(leadRect, 0, sizeof(CRect) * leadCount);

	for(int i = 0; i < leadCount; i++)
	{
		leadRect[i].left = (LONG)m_vtrLandPos.at(i);
		leadRect[i].right = (LONG)(leadRect[i].left + m_vtrLand.at(i));
		leadRect[i].top = 0; //아직 모름..
		leadRect[i].bottom = sizeY;	//아직 모름..
	}



	////////////////////////////////////////////////////////////////////////// 상
	int clipCx = 0;
	int clipCy = 0;


	Im::PIL_ID milClip_Top = M_NULL;
	Im::PIL_ID milClip_Bottom = M_NULL;


	long* prjListTop = NULL;
	long* prjListBottom = NULL;
	int size = 0;
	for(int i = 0; i < leadCount; i++)
	{
		double sum_top = 0;
		double sum_bottom = 0;
		long avr_top = 0;
		long avr_bottom = 0;

		clipCx = (int)(leadRect[i].left + (leadRect[i].Width() / 2.0));
		clipCy = (int)(leadRect[i].top + (leadRect[i].Height() / 2.0));

		milClip_Top = AllocClipBuff(milSrcTop, clipCx, clipCy,  leadRect[i].Width(), leadRect[i].Height());
		milClip_Bottom = AllocClipBuff(milSrcHV, clipCx, clipCy,  leadRect[i].Width(), leadRect[i].Height());

		size = leadRect[i].Height();
		/*prjListTop = new long[size];
		prjListBottom = new long[size];*/
		prjListTop = g_pMManager->pem_new<long>(true, size, (PCHAR)__FUNCTION__, __LINE__);
		prjListBottom = g_pMManager->pem_new<long>(true, size, (PCHAR)__FUNCTION__, __LINE__);

		GetProjection_V(milClip_Top, prjListTop);
		GetProjection_V(milClip_Bottom, prjListBottom);

		for(int j = 0; j < size; j++)
		{
			prjListTop[j] /= leadRect[i].Width();
			sum_top += prjListTop[j];

			prjListBottom[j] /= leadRect[i].Width();
			sum_bottom += prjListBottom[j];
		}

		avr_top = (long)(sum_top / size);
		avr_bottom = (long)(sum_bottom / size);


		//각 값에서 평균을 뺀 값이 양수인 지점 찾기...

		long leadStartPoint = 0;
		long dataTemp_top = 0;
		long dataTemp_bottom = 0;
		for(int j = 0; j < size; j++)
		{
			dataTemp_top = prjListTop[j] - avr_top;
			dataTemp_bottom = prjListBottom[j] - avr_bottom;

			if(dataTemp_top >= 0 || dataTemp_bottom >= 0)
			{
				leadStartPoint = j;
				break;
			}
		}

		leadRect[i].top = (LONG)leadStartPoint;

		FreeMilImageBuff(&milClip_Top);
		FreeMilImageBuff(&milClip_Bottom);
		/*delete prjListTop;
		delete prjListBottom;*/	
		g_pMManager->pem_delete(prjListTop, false);
		g_pMManager->pem_delete(prjListBottom, false);
	}


	//////////////////////////////////////////////////////////////////////////

	//*retRect = new CRect[leadCount];
	*retRect = g_pMManager->pem_new<CRect>(true, leadCount, (PCHAR)__FUNCTION__, __LINE__);
	memcpy_s(*retRect, sizeof(CRect) * leadCount, leadRect, sizeof(CRect) * leadCount);



	leadInfo.countLead = m_countLead;
	leadInfo.avgLead = m_avgLead;
	leadInfo.maxLead = m_maxLead;
	leadInfo.minLead = m_minLead;

	leadInfo.countGap = m_countGap;
	leadInfo.avgGap = m_avgGap;
	leadInfo.maxGap = m_maxGap;
	leadInfo.minGap = m_minGap;

	if(m_countGap > 0)
	{
		//int* leadPitch = new int[m_countGap];
		int* leadPitch = g_pMManager->pem_new<int>(true, m_countGap, (PCHAR)__FUNCTION__, __LINE__);
		double sum = 0;
		for(int i = 0; i < m_countGap; i++)
		{
			leadPitch[i] = (m_vtrLandCenterPos.at(i + 1) - m_vtrLandCenterPos.at(i));
			sum += leadPitch[i];
		}

		leadInfo.leadPitch = sum / m_countGap;

		//*retLeadPitch = new int[m_countGap];
		*retLeadPitch = g_pMManager->pem_new<int>(true, m_countGap, (PCHAR)__FUNCTION__, __LINE__);
		memcpy_s(*retLeadPitch, sizeof(int) * m_countGap, leadPitch, sizeof(int) * m_countGap);
		//delete leadPitch;
		g_pMManager->pem_delete(leadPitch, false);
	}
	else
	{
		leadInfo.leadPitch = 0;
	}
	//////////////////////////////////////////////////////////////////////////

	//delete leadRect;
	g_pMManager->pem_delete(leadRect, false);
	FreeMilImageBuff(&milSrcTop);
	//FreeMilImageBuff(&milSrcBottom);  //huj 2014/02/24   bottom 영상은 사용 하지 않아 주석 처리함.
	FreeMilImageBuff(&milSrcHV);

	return leadInfo;	
}

//2014.10.23 SHW 2D, 3D 분리
LeadInfoTemp CProcMil_LeadSolder::CalcLeadRect3(UCHAR* topImg, UCHAR* hvImg, float* ZmapRoiData, int sizeX, int sizeY, CRect** retRect, int** retLeadPitch, 
	BOOL isContainer, BOOL isInspLeadLift, int leadStartPos, float leadHeight, double stdLeadWidth, int stdLeadCount, BOOL use3D, BOOL use2D)
{
	LeadInfoTemp leadInfo;
	//////////////////////////////////////////////////////////////////////////
	leadInfo.countLead = 0;
	leadInfo.avgLead = 0;
	leadInfo.maxLead = 0;
	leadInfo.minLead = 0;

	leadInfo.countGap = 0;
	leadInfo.avgGap = 0;
	leadInfo.maxGap = 0;
	leadInfo.minGap = 0;

	leadInfo.leadPitch = 0;
	//////////////////////////////////////////////////////////////////////////
	try
	{
		CRect* leadRect = NULL;

		Im::PIL_ID milSrcTop = M_NULL;
		milSrcTop = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(milSrcTop, topImg);

		MedianFilter(milSrcTop, milSrcTop, TRUE);
		SaveWorkImg(milSrcTop, _T("medianTop.bmp"));	

		MorUserErode_H(milSrcTop, milSrcTop, 2, TRUE);
		MorUserDilate_H(milSrcTop, milSrcTop, 2, TRUE);
		SaveWorkImg(milSrcTop, _T("morTop.bmp"));


		Im::PIL_ID milSrcHV = M_NULL;
		milSrcHV = AllocBuff(sizeX, sizeY);
		Im::Buf::Put(milSrcHV, hvImg);

		MedianFilter(milSrcHV, milSrcHV, TRUE);
		SaveWorkImg(milSrcHV, _T("medianHV.bmp"));

		MorUserErode_H(milSrcHV, milSrcHV, 2, TRUE);
		MorUserDilate_H(milSrcHV, milSrcHV, 2, TRUE);
		SaveWorkImg(milSrcHV, _T("morHV.bmp"));

		//////////////////////////////////////////////////////////////////////////
		//3D
		float* prjZList = NULL;
		bool useZmapFlag = true;
		bool splitFlag = false;
		bool nodataBody = false;
		int bodyTipIndex[2] = {0,};

		if(sizeX >= 500)//검사영역의 폭이 500이상이면 반씩 나눠서 시작점을 찾는다.(부품이 기울어지면 양끝의 시작점이 차이가 생기기 때문.)
			splitFlag = true;

		if(use3D == TRUE) //3D를 이용하여 바디 시작점을 찾고 zmap data도 추출한다.
		{
			float zmapPrjThreshold = 0;
			int zmapClipCx = 0;
			int zmapClipCy = 0;
			int zmapSizeX = sizeX;
			int zmapSizeY = sizeY;

			float* zmapSrc = NULL;
			//zmapSrc = new float[sizeX * sizeY];
			zmapSrc = g_pMManager->pem_new<float>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippsThreshold_LTVal_32f(ZmapRoiData, zmapSrc, sizeX * sizeY, 0, 0);


			//검사영역안에 body가 포함되었는지 여부를 판단하기 위한 작업들.. (body가 들어가면 projection 할때 데이터가 뭉게질 수 있음)
			double sumTemp_top = 0;
			float avrHTemp_top = 0;
			double sumTemp_Line = 0;
			float avrHTemp_Line = 0;
			for(int i = 0; i < sizeX; i++)
			{
				sumTemp_top += zmapSrc[i];
				sumTemp_Line += zmapSrc[(sizeX * leadStartPos) + i];
			}
			avrHTemp_top = (float)(sumTemp_top / sizeX * 0.9);
			avrHTemp_Line = (float)(sumTemp_Line / sizeX * 0.9);

			sumTemp_top = 0;
			sumTemp_Line = 0;
			for(int i = 0; i < sizeX; i++)
			{
				if(zmapSrc[i] >= avrHTemp_top)
					sumTemp_top++;

				if(zmapSrc[(sizeX * leadStartPos) + i] >= avrHTemp_Line)
					sumTemp_Line++;
			}

			bool bodyDelFlag = false; //true: 바디가 포함된 경우. 바디부분 삭제 해야함
			bool areaFlag = false; //true: 녹색선이 바디에 있는경우. 즉, 리드가 없어 리드 팁에 설정하지 않고 바디에 설정한 경우
			if(sumTemp_top >= sizeX * 0.8)
				bodyDelFlag = true;

			if(avrHTemp_top <= 170)
			{
				bodyDelFlag = false;
				nodataBody = true;
			}

			if(sumTemp_Line >= sizeX * 0.8)
				areaFlag = true;
			if(isContainer == FALSE)
				areaFlag = false;

			if(bodyDelFlag)//검사 영역에 포함 되어있는 body를 제거하기 위해 body위치를 찾는다. (포함 되어있다 -> bodyDelFlag=true)
			{
				Im::PIL_ID milTemp = AllocBuff_float(sizeX, sizeY);
				Im::Buf::Put(milTemp, zmapSrc);

				if(!splitFlag)//나누지 않고 전체영역에서 찾음.
				{
					//float* proFile = new float[sizeY];
					float* proFile = g_pMManager->pem_new<float>(true, sizeY, (PCHAR)__FUNCTION__, __LINE__);
					GetProjection_V(milTemp, proFile);

					proFile[0] /= sizeX;
					for(int i = 1; i < sizeY; i++)
					{
						proFile[i] /= sizeX;
						if(proFile[i-1] >= avrHTemp_top)
						{
							if(proFile[i] < avrHTemp_top)
							{
								bodyTipIndex[0] = i-1;
								break;
							}
						}
					}

					//delete proFile;
					g_pMManager->pem_delete(proFile, false);
				}
				else// 반씩 나눠서 각각 찾는다.
				{
					bool halfFlag1 = false;
					bool halfFlag2 = false;

					int tempSizeX = (int)(sizeX / 2.0);
					int tempSizeY = sizeY;
					int tempCx = (int)(tempSizeX / 2.0);
					int tempCy = (int)(tempSizeY / 2.0);
					Im::PIL_ID zmapSrcHalf1 = AllocClipBuff_float(milTemp, tempCx, tempCy ,tempSizeX, tempSizeY);

					tempCx = (int)(tempSizeX / 2.0) + tempSizeX;
					Im::PIL_ID zmapSrcHalf2 = AllocClipBuff_float(milTemp, tempCx, tempCy ,tempSizeX, tempSizeY);

					//float* proFileHalf1 = new float[tempSizeY];
					float* proFileHalf1 = g_pMManager->pem_new<float>(true, tempSizeY, (PCHAR)__FUNCTION__, __LINE__);
					GetProjection_V(zmapSrcHalf1, proFileHalf1);

					//float* proFileHalf2 = new float[tempSizeY];
					float* proFileHalf2 = g_pMManager->pem_new<float>(true, tempSizeY, (PCHAR)__FUNCTION__, __LINE__);
					GetProjection_V(zmapSrcHalf2, proFileHalf2);


					proFileHalf1[0] /= tempSizeX;
					proFileHalf2[0] /= tempSizeX;
					for(int i = 1; i < tempSizeY; i++)
					{
						proFileHalf1[i] /= tempSizeX;
						if(!halfFlag1)
						{
							if(proFileHalf1[i-1] >= avrHTemp_top)
							{
								if(proFileHalf1[i] < avrHTemp_top)
								{
									bodyTipIndex[0] = i-1;
									halfFlag1 = true;
								}
							}
						}

						proFileHalf2[i] /= tempSizeX;
						if(!halfFlag2)
						{
							if(proFileHalf2[i-1] >= avrHTemp_top)
							{
								if(proFileHalf2[i] < avrHTemp_top)
								{
									bodyTipIndex[1] = i-1;
									halfFlag2 = true;
								}
							}
						}

						if(halfFlag1 && halfFlag2)
							break;
					}


					FreeMilImageBuff(&zmapSrcHalf1);
					FreeMilImageBuff(&zmapSrcHalf2);
					/*delete proFileHalf1;
					delete proFileHalf2;*/
					g_pMManager->pem_delete(proFileHalf1, false);
					g_pMManager->pem_delete(proFileHalf2, false);
				}

				FreeMilImageBuff(&milTemp);
			}

			if(leadStartPos - (bodyTipIndex[0] + 10) < 10)
				areaFlag = true;

			areaFlag = true;
			if(!areaFlag)// 0부터 green line까지
			{
				int bodyTipIndexTemp = bodyTipIndex[0];
				if(bodyTipIndexTemp != 0)
					bodyTipIndexTemp += 10;

				zmapSizeY = leadStartPos - bodyTipIndexTemp;
				if(zmapSizeY <= 0) 
				{
					zmapSizeY = leadStartPos;
					bodyTipIndex[0] = 0;
					bodyTipIndex[1] = 0;
					bodyTipIndexTemp = 0;
				}

				zmapClipCx = (int)(zmapSizeX / 2.0);
				zmapClipCy = (int)((zmapSizeY / 2.0) + bodyTipIndexTemp);
			}
			else //green line부터 rect 끝까지
			{
				zmapSizeY = sizeY - leadStartPos;			

				zmapClipCx = (int)(zmapSizeX / 2.0);
				zmapClipCy = (int)(zmapSizeY / 2.0 + leadStartPos);
			}
			//////////////////////////////////////////////////////////////////////////

			//float* zmapTemp = new float[zmapSizeX * zmapSizeY];	
			float* zmapTemp = g_pMManager->pem_new<float>(true, zmapSizeX * zmapSizeY, (PCHAR)__FUNCTION__, __LINE__);
			GetCropZmap(zmapSrc, zmapTemp, sizeX, sizeY, zmapClipCx, zmapClipCy, zmapSizeX, zmapSizeY);	


			Im::PIL_ID milSrcZmap = M_NULL;
			milSrcZmap = AllocBuff_float(zmapSizeX, zmapSizeY);
			Im::Buf::Put(milSrcZmap, zmapTemp);

			/*delete zmapTemp;
			delete zmapSrc;*/
			g_pMManager->pem_delete(zmapTemp, false);
			g_pMManager->pem_delete(zmapSrc, false);
			//////////////////////////////////////////////////////////////////////////


			int allowDataCnt = 0;
			//prjZList = new float[zmapSizeX];
			prjZList = g_pMManager->pem_new<float>(true, zmapSizeX, (PCHAR)__FUNCTION__, __LINE__);
			//memset(prjZList, 0, sizeof(float)* zmapSizeX);

			GetProjection_H(milSrcZmap, prjZList);
			FreeMilImageBuff(&milSrcZmap);

			Im::PIL_ID milPrj = M_NULL;
			milPrj = AllocBuff_float(zmapSizeX, 1);
			Im::Buf::Put(milPrj, prjZList);
			zmapPrjThreshold = (float)GetThreshold(milPrj);
			FreeMilImageBuff(&milPrj);

			zmapPrjThreshold /= zmapSizeY;

			for(int i = 0; i < zmapSizeX; i++)
			{
				if(prjZList[i] != 0 && zmapSizeY != 0)
					prjZList[i] /= zmapSizeY;

				if (prjZList[i] < zmapPrjThreshold)
					prjZList[i] = 0;
				else
					allowDataCnt++;
			}

			if((((double)allowDataCnt / (double)zmapSizeX) * 100.0) <= 30.0)
				useZmapFlag = false;
		}

		if(use3D == FALSE || nodataBody == true)//2D를 이용하여 바디 시작점만 찾는다.
		{
			int bodyThreshold = 100;
			int stdCount_H = (int)(sizeX * 0.8);
			int stdCount_L = (int)(sizeX * 0.3);

			UCHAR* userSrcHV = NULL;
			//userSrcHV = new UCHAR[sizeX * sizeY];
			userSrcHV = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
			Im::Buf::Get(milSrcHV,userSrcHV);
			GetBinaryImg_ipp(userSrcHV, sizeX, sizeY, userSrcHV, bodyThreshold, 0, bodyThreshold, 255);

			Im::PIL_ID milTemp = AllocBuff(sizeX, sizeY);
			Im::Buf::Put(milTemp, userSrcHV);

			int tempCx = 0;
			int tempCy = 0;
			int tempSizeX = sizeX;
			int tempSizeY = sizeY;
			int reCnt = 1;

			if(splitFlag)
			{
				reCnt = 2;
				tempSizeX = (int)(sizeX / 2.0);

				stdCount_H = (int)(tempSizeX * 0.8);
				stdCount_L = (int)(tempSizeY * 0.3);
			}

			Im::PIL_ID milClip = M_NULL;
			//long* proFile = new long[tempSizeY];
			long* proFile = g_pMManager->pem_new<long>(true, tempSizeY, (PCHAR)__FUNCTION__, __LINE__);
			for(int j = 0; j < reCnt; j++)
			{
				if(splitFlag)
				{
					tempCx = (int)((tempSizeX / 2.0) + (tempSizeX * j));
					tempCy = (int)(tempSizeY / 2.0);
					milClip = AllocClipBuff(milTemp, tempCx, tempCy, tempSizeX, tempSizeY);
				}
				else
				{
					milClip = AllocBuff(tempSizeX, tempSizeY);
					Im::Buf::Copy(milTemp, milClip);				
				}
				MorOpen(milClip, milClip, 4);
				GetProjection_V(milClip, proFile);

				bool isDarkBody = true;
				for(int i = 0; i < tempSizeY; i++)
				{
					proFile[i] /= 255;
					if(proFile[i] <= stdCount_H && proFile[i] >= stdCount_L)
					{
						bodyTipIndex[j] = i-1;
						break;
					}

					if(i == 0)
					{
						if(proFile[0] < stdCount_H)			
							break;
						else
							isDarkBody = false;
					}
				}

				if(isDarkBody)
				{
					SaveWorkImg(milClip, _T("leadTest1.bmp"));
					//UCHAR* userTemp = new UCHAR[tempSizeX * tempSizeY];
					UCHAR* userTemp = g_pMManager->pem_new<UCHAR>(true, tempSizeX * tempSizeY, (PCHAR)__FUNCTION__, __LINE__);
					Im::Buf::Get(milClip, userTemp);
					GetBinaryImg_ipp(userTemp, tempSizeX, tempSizeY, userTemp, 1, 255, 254, 0);
					Im::Buf::Put(milClip, userTemp);

					GetProjection_V(milClip, proFile);
					SaveWorkImg(userTemp, tempSizeX, tempSizeY, _T("leadTest2.bmp"));

					for(int i = 0; i < tempSizeY; i++)
					{
						proFile[i] /= 255;
						if(proFile[i] <= stdCount_H)
						{
							bodyTipIndex[j] = i-1;
							break;
						}
					}

					//delete userTemp;
					g_pMManager->pem_delete(userTemp, false);
				}

				FreeMilImageBuff(&milClip);
			}


			/*delete proFile;
			delete userSrcHV;*/
			g_pMManager->pem_delete(proFile, false);
			g_pMManager->pem_delete(userSrcHV, false);
			FreeMilImageBuff(&milTemp);	

		}

		if(bodyTipIndex[0] < 0)
			bodyTipIndex[0] = 0;
		if(bodyTipIndex[0] >= sizeY)
			bodyTipIndex[0] = sizeY - 1;

		if(bodyTipIndex[1] < 0)
			bodyTipIndex[1] = 0;
		if(bodyTipIndex[1] >= sizeY)
			bodyTipIndex[1] = sizeY - 1;
		//////////////////////////////////////////////////////////////////////////
		//2D
		long* prjList = NULL;
		//prjList = new long[sizeX];
		prjList = g_pMManager->pem_new<long>(true, sizeX, (PCHAR)__FUNCTION__, __LINE__);
		GetProjection_H(milSrcHV, prjList);

		double sumTemp = 0.;
		int avrTemp = 0;
		BOOL widthFlag = TRUE;

		if(use2D == TRUE)
		{
			for(int i = 0; i < sizeX; i++)
			{
				sumTemp += prjList[i];
				prjList[i] /= sizeY;
			}

			avrTemp = (int)(sumTemp / sizeX);

			if(avrTemp < 6000)
				widthFlag = FALSE;

			//long* prjHisto = new long[256];
			long* prjHisto = g_pMManager->pem_new<long>(true, 256, (PCHAR)__FUNCTION__, __LINE__);
			memset(prjHisto, 0, sizeof(long)* 256);

			GetHistoValue(prjList, sizeX, 256, prjHisto);
			long heightThreshold = CalcAutoThreshold(prjHisto, 256, 0, 255);
			heightThreshold = (long)(heightThreshold * 0.9);
			//delete prjHisto;
			g_pMManager->pem_delete(prjHisto, false);

			for (int index = 0; index < sizeX; index ++)
			{
				if (prjList[index] < heightThreshold)
					prjList[index] = 0;	
				else
					prjList[index] = 255L;
			}
		}
		else
		{
			if(use3D == TRUE)
			{
				for (int index = 0; index < sizeX; index ++)
				{
					prjList[index] = prjZList[index];
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////

		if(use3D == TRUE && useZmapFlag == TRUE)
		{
			int stp = -1;
			int edp = 0;
			if(isContainer == TRUE)
			{
				for(int i = 0; i < sizeX; i++)
				{
					if (prjList[i] == 0 || (long)prjZList[i] == 0)
					{
						prjList[i] = 0L;
					}
					else if(prjList[i] > 0 && (long)prjZList[i] > 0 )
					{
						prjList[i] = 255L;
					}	
				}
			}
			else //리드가 무조건 한개! (양끝점만 찾는다..)
			{	
				for(int i = 0; i < sizeX; i++)
				{
					if(prjList[i] > 0 && (long)prjZList[i] > 0)
					{
						if(stp == -1)
							stp = i;

						edp = i;
					}	

				}

				//계산된 리드 폭이 터무니 없이 작을때..
				if(stdLeadWidth != 0)
				{
					int tempW = edp - stp;
					int stdW = (int)(stdLeadWidth * 0.3);

					if(tempW <= stdW)
					{
						int tempMargin = (int)((sizeX - stdLeadWidth) / 2.0);
						if(tempMargin > 0)
						{
							stp = tempMargin;
							edp = sizeX - stp;
						}
						else
						{
							stp = (int)(sizeX * 0.2);
							edp = sizeX - stp;
						}
					}
				}


				for(int i = 0; i < sizeX; i++)
				{
					if(i >= stp && i <= edp)
						prjList[i] = 255L;
					else
						prjList[i] = 0L;
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////

		int* leadPitch = NULL;
		CalcRunLength2(sizeX, prjList, 0, widthFlag, stdLeadCount,stdLeadWidth, &leadPitch);
		int leadCount = m_countLead;

		if(prjList != NULL)
			//delete prjList;
			g_pMManager->pem_delete(prjList, false);

		if(prjZList != NULL)
			//delete prjZList;
			g_pMManager->pem_delete(prjZList, false);



		if(leadCount < 1)
		{
			FreeMilImageBuff(&milSrcTop);
			FreeMilImageBuff(&milSrcHV);

			return leadInfo;
		}

		//leadRect = new CRect[leadCount];
		leadRect = g_pMManager->pem_new<CRect>(true, leadCount, (PCHAR)__FUNCTION__, __LINE__);
		memset(leadRect, 0, sizeof(CRect) * leadCount);

		for(int i = 0; i < leadCount; i++)
		{
			leadRect[i].left = (LONG)m_vtrLandPos.at(i);
			leadRect[i].right = (LONG)(leadRect[i].left + m_vtrLand.at(i));
			leadRect[i].top = 0; //아직 모름..
			leadRect[i].bottom = sizeY;	//아직 모름..
		}

		////////////////////////////////////////////////////////////////////////// 상
		for(int i = 0; i < leadCount; i++)
		{
			if(!splitFlag)
			{
				leadRect[i].top = (LONG)bodyTipIndex[0];
			}
			else
			{
				if(i < (int)(leadCount / 2.0))
					leadRect[i].top = (LONG)bodyTipIndex[0];
				else
					leadRect[i].top = (LONG)bodyTipIndex[1];
			}
		}

		//////////////////////////////////////////////////////////////////////////

		//*retRect = new CRect[leadCount];
		*retRect = g_pMManager->pem_new<CRect>(true, leadCount, (PCHAR)__FUNCTION__, __LINE__);
		memcpy_s(*retRect, sizeof(CRect) * leadCount, leadRect, sizeof(CRect) * leadCount);



		leadInfo.countLead = m_countLead;
		leadInfo.avgLead = m_avgLead;
		leadInfo.maxLead = m_maxLead;
		leadInfo.minLead = m_minLead;

		leadInfo.countGap = m_countGap;
		leadInfo.avgGap = m_avgGap;
		leadInfo.maxGap = m_maxGap;
		leadInfo.minGap = m_minGap;

		if(m_countGap > 0)
		{		
			leadInfo.leadPitch = m_avgPitch;

			//*retLeadPitch = new int[m_countGap];
			*retLeadPitch = g_pMManager->pem_new<int>(true, m_countGap, (PCHAR)__FUNCTION__, __LINE__);
			memcpy_s(*retLeadPitch, sizeof(int) * m_countGap, leadPitch, sizeof(int) * m_countGap);

		}
		else
		{
			leadInfo.leadPitch = 0;
		}
		//////////////////////////////////////////////////////////////////////////

		if(leadPitch != NULL)
			//delete leadPitch;
			g_pMManager->pem_delete(leadPitch, false);

		//delete leadRect;
		g_pMManager->pem_delete(leadRect, false);
		FreeMilImageBuff(&milSrcTop);
		FreeMilImageBuff(&milSrcHV);
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}

	return leadInfo;	
}

LeadInfoTemp CProcMil_LeadSolder::CalcLeadRectTab_org(UCHAR* hvImage, int roiSizeX, int roiSizeY, int nThresholdValue, int startPos, CAtlArray<CRect> *pRcArrSearchTab)
{
	LeadInfoTemp sLeadInfo;

	sLeadInfo.countLead = 0;
	sLeadInfo.avgLead = 0;
	sLeadInfo.maxLead = 0;
	sLeadInfo.minLead = 0;

	sLeadInfo.countGap = 0;
	sLeadInfo.avgGap = 0;
	sLeadInfo.maxGap = 0;
	sLeadInfo.minGap = 0;
	sLeadInfo.leadPitch = 0;

	InitMilAlgoBlob();
	//UCHAR * pBlobImg = new UCHAR[roiSizeX * roiSizeY];
	UCHAR * pBlobImg = g_pMManager->pem_new<UCHAR>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	GetBinaryImg_ipp(hvImage, roiSizeX, roiSizeY, hvImage, nThresholdValue, 0, nThresholdValue, 255);

	int nCntBlob = CalcBlob_Select(hvImage, pBlobImg, roiSizeX, roiSizeY, 4, false, false, 1, eSelectMix);
	Delete_1DArray(&pBlobImg);
	if (nCntBlob == 0)
		return sLeadInfo;

	/*double *dArrArea = new double[nCntBlob];
	double *dArrCX = new double[nCntBlob];
	double *dArrCY = new double[nCntBlob];
	CRect *rcArrBlob = new CRect[nCntBlob];*/
	double *dArrArea = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	double *dArrCX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	double *dArrCY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	CRect *rcArrBlob = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);

	GetBlobResult_ALL(dArrArea, dArrCX, dArrCY, rcArrBlob);

	double dWidthAVG = 0.0;
	double dHeightAVG = 0.0;
	double dGapAVG = 0.0;
	if(nCntBlob > 0)
	{
		for(int i=0; i < nCntBlob; ++i)
		{
			int nWidht = rcArrBlob[i].Width();
			int nHeight = rcArrBlob[i].Height();
			if((nWidht <= 0) || (nHeight <= 0) || (rcArrBlob[i].top > startPos || rcArrBlob[i].bottom < startPos))
				continue;

			dWidthAVG += nWidht;
			dHeightAVG += nHeight;

			pRcArrSearchTab->Add(rcArrBlob[i]);
		}
		nCntBlob = pRcArrSearchTab->GetCount();
		qsort(pRcArrSearchTab->GetData(),  nCntBlob, sizeof(CRect), CompareCRectX); 

		dWidthAVG /= nCntBlob;
		sLeadInfo.avgLead = dWidthAVG;
		sLeadInfo.countLead = nCntBlob;

		dWidthAVG = (int)dWidthAVG-1;

		dHeightAVG /= nCntBlob;
		dHeightAVG *= 0.1;
		for(int i=0; i < pRcArrSearchTab->GetCount(); ++i)
		{
			CRect rcBlob = pRcArrSearchTab->GetAt(i);
			if(rcBlob.Width() < dWidthAVG)
			{
				pRcArrSearchTab->RemoveAt(i--);
				continue;
			}
			else if(rcBlob.Height() < dHeightAVG)
			{
				pRcArrSearchTab->RemoveAt(i--);
				continue;
			}
			else if(i == 0)
				continue;

			CRect rcBlobPre = pRcArrSearchTab->GetAt(i-1);
			dGapAVG += (rcBlob.left - rcBlobPre.right);
		}
		nCntBlob = pRcArrSearchTab->GetCount();
		if(nCntBlob - 1 != 0)
			dGapAVG /= (nCntBlob-1);
		sLeadInfo.avgGap = dGapAVG;
		sLeadInfo.countGap = nCntBlob - 1;

	}

	Delete_1DArray(&dArrArea);
	Delete_1DArray(&dArrCX);
	Delete_1DArray(&dArrCY);
	Delete_1DArray(&rcArrBlob);

	return sLeadInfo;
}

LeadInfoTemp CProcMil_LeadSolder::CalcLeadRectTab(UCHAR* hvImage, int roiSizeX, int roiSizeY, int nThresholdValue, int startPos, CAtlArray<CRect> *pRcArrSearchTab)
{
	if (startPos < 1 || roiSizeY < (startPos + 3))
		return CalcLeadRectTab_org(hvImage, roiSizeX, roiSizeY, nThresholdValue, startPos, pRcArrSearchTab);
	LeadInfoTemp sLeadInfo;

	sLeadInfo.countLead = 0;
	sLeadInfo.avgLead = 0;
	sLeadInfo.maxLead = 0;
	sLeadInfo.minLead = 0;

	sLeadInfo.countGap = 0;
	sLeadInfo.avgGap = 0;
	sLeadInfo.maxGap = 0;
	sLeadInfo.minGap = 0;
	sLeadInfo.leadPitch = 0;

	InitMilAlgoBlob();
	//UCHAR * pBlobImg = new UCHAR[roiSizeX * roiSizeY];
	UCHAR * pBlobImg = g_pMManager->pem_new<UCHAR>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	GetBinaryImg_ipp(hvImage, roiSizeX, roiSizeY, hvImage, nThresholdValue, 0, nThresholdValue, 255);
	cv::Mat mhvImage(roiSizeY, roiSizeX, CV_8UC1, hvImage);
	cv::Mat StartPosImg = mhvImage(cv::Rect(0, startPos - 1, roiSizeX, 3));
	UCHAR* ptr = StartPosImg.ptr();

	//int nCntBlob = CalcBlob_Select(hvImage, pBlobImg, roiSizeX, roiSizeY, 4, false, false, 1, eSelectMix);
	int nCntBlob = CalcBlob_Select(ptr, pBlobImg, StartPosImg.cols, StartPosImg.rows, 4, false, false, 1, eSelectMix);
	Delete_1DArray(&pBlobImg);
	if (nCntBlob == 0)
		return sLeadInfo;
	/*double *dArrArea = new double[nCntBlob];
	double *dArrCX = new double[nCntBlob];
	double *dArrCY = new double[nCntBlob];
	CRect *rcArrBlob = new CRect[nCntBlob];*/
	double *dArrArea = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	double *dArrCX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	double *dArrCY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	CRect *rcArrBlob = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);

	GetBlobResult_ALL(dArrArea, dArrCX, dArrCY, rcArrBlob);

	CAtlArray<CRect> pRcArrSearchTabTmp;

	double dWidthMX = 0.0;
	double dWidthAVG = 0.0;
	double dHeightAVG = 0.0;
	double dGapAVG = 0.0;
	if (nCntBlob > 0)
	{
		for (int i = 0; i < nCntBlob; ++i)
		{
			int nWidht = rcArrBlob[i].Width();
			int nHeight = rcArrBlob[i].Height();
			if ((nWidht <= 0) || (nHeight <= 0) /*|| (rcArrBlob[i].top > startPos || rcArrBlob[i].bottom < startPos)*/)
				continue;

			dWidthAVG += nWidht;
			dHeightAVG += nHeight;
			CRect addRect = rcArrBlob[i];
			addRect.top += (startPos - 1);
			addRect.bottom += (startPos - 1);
			pRcArrSearchTabTmp.Add(addRect);
			if (nWidht > dWidthMX)
			{
				dWidthMX = nWidht;
			}
		}
		nCntBlob = pRcArrSearchTabTmp.GetCount();
		qsort(pRcArrSearchTabTmp.GetData(), nCntBlob, sizeof(CRect), CompareCRectX);

		dWidthAVG = (int)(dWidthMX*0.8f);
		sLeadInfo.avgLead = dWidthAVG;
		sLeadInfo.countLead = nCntBlob;

		dWidthAVG = (int)dWidthAVG - 1;

		dHeightAVG /= nCntBlob;
		dHeightAVG *= 0.1;
		for (int i = 0; i < pRcArrSearchTabTmp.GetCount(); ++i)
		{
			CRect rcBlob = pRcArrSearchTabTmp.GetAt(i);
			if (rcBlob.Width() < dWidthAVG)
			{
				pRcArrSearchTabTmp.RemoveAt(i--);
				continue;
			}
			else if (rcBlob.Height() < dHeightAVG)
			{
				pRcArrSearchTabTmp.RemoveAt(i--);
				continue;
			}
			else if (i == 0)
				continue;

			CRect rcBlobPre = pRcArrSearchTabTmp.GetAt(i - 1);
			dGapAVG += (rcBlob.left - rcBlobPre.right);
		}
		nCntBlob = pRcArrSearchTabTmp.GetCount();
		if (nCntBlob - 1 != 0)
			dGapAVG /= (nCntBlob - 1);
		sLeadInfo.avgGap = dGapAVG;
		sLeadInfo.countGap = nCntBlob - 1;

		//contain
		pBlobImg = g_pMManager->pem_new<UCHAR>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
		nCntBlob = CalcBlob_Select(hvImage, pBlobImg, roiSizeX, roiSizeY, 4, false, false, 1, eSelectMix);
		Delete_1DArray(&pBlobImg);
		if (nCntBlob != 0)
		{
			double *dArrAreaTmp = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			double *dArrCXTmp = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			double *dArrCYTmp = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			CRect *rcArrBlobTmp = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);

			GetBlobResult_ALL(dArrAreaTmp, dArrCXTmp, dArrCYTmp, rcArrBlobTmp);
			dGapAVG = 0;
			for (int i = 0; i < pRcArrSearchTabTmp.GetCount(); ++i)
			{
				bool bContain = false;
				for (int idxBlob = 0; idxBlob < nCntBlob; idxBlob++)
				{
					if (rcArrBlobTmp[idxBlob].top <= pRcArrSearchTabTmp[i].top && rcArrBlobTmp[idxBlob].left <= pRcArrSearchTabTmp[i].left
						&& rcArrBlobTmp[idxBlob].bottom >= pRcArrSearchTabTmp[i].bottom && rcArrBlobTmp[idxBlob].right >= pRcArrSearchTabTmp[i].right)
					{
						bContain = true;
						pRcArrSearchTab->Add(rcArrBlobTmp[idxBlob]);
						dGapAVG += (rcArrBlobTmp[idxBlob].left - rcArrBlobTmp[idxBlob].right);
						break;
					}
				}
			}
			nCntBlob = pRcArrSearchTab->GetCount();
			if (nCntBlob - 1 != 0)
				dGapAVG /= (nCntBlob - 1);
			sLeadInfo.avgGap = dGapAVG;
			sLeadInfo.countGap = nCntBlob - 1;

			Delete_1DArray(&dArrAreaTmp);
			Delete_1DArray(&dArrCXTmp);
			Delete_1DArray(&dArrCYTmp);
			Delete_1DArray(&rcArrBlobTmp);
		}
	}

	Delete_1DArray(&dArrArea);
	Delete_1DArray(&dArrCX);
	Delete_1DArray(&dArrCY);
	Delete_1DArray(&rcArrBlob);

	return sLeadInfo;
}

void CProcMil_LeadSolder::CalcRunLength(const long nCount, const long *line, const long min_land_size, BOOL widthFlag)
{
	long k = 0;
	long land = 0, gap = 0;
	long land_min = 0, gap_min = 0, land_max = 0, gap_max = 0;
	long first_land, last_land;
	long land_sum = 0, gap_sum = 0;
	double land_av = 0.0, gap_av = 0.0;

	// 데이타 개수가 없으면 .....
	if (nCount <= 0) {
		return;
	}

	// 이전의 데이타를 모두 삭제
	m_vtrLandPos.clear();
	m_vtrGapPos.clear();
	m_vtrLandCenterPos.clear();
	m_vtrLand.clear();
	m_vtrGap.clear();

	long pc1 = 0L, pc2 = 0L;
	long index = 0L;

	pc1 = line[index];

	/* 첫번째 index의 projection intensity가 0이면 무시 한다 */
	if (pc1 == 0L) {
		do {
			index ++;
			if (index >= nCount) {
				break;
			}

			pc2 = line[index];
		} while (pc2 == pc1);

		k = 1;
	}
	else {
		k = 1;
		pc2 = pc1;
	}

	// land관련 정보
	land	 = 0;
	land_sum = 0;
	land_min = 0x7FFF;
	land_max = 0;
	// gap관련 정보
	gap 	 = 0;
	gap_sum  = 0;
	gap_min  = 0x7FFF;
	gap_max  = 0;

	int last_gap_size = 0;
	first_land = 0;
	last_land = 0;

	do {
		index ++;
		if (index >= nCount) {
			break;
		}

		pc1 = line[index];

		if ((pc1>0 && pc2>0) || (pc1==0 && pc2==0)) {
			// pc1과 pc2가 0보다 크면 lead
			// pc1과 pc2가 0이면 gap
			// pc1과 pc2가 다르면 lead에서 gap으로 혹은 gap에서 lead로의 변환임
			k ++;
		}
		else {
			if (pc2 > 0 && k > min_land_size) { // land에서 gap으로
				land ++;
				land_sum += k;
				m_vtrLandPos.push_back(index - k);
				m_vtrGapPos.push_back(index);
				m_vtrLandCenterPos.push_back(index - k/2);
				m_vtrLand.push_back(k);

				if (land > 1) {
					last_land = k;
				}
				else {
					first_land = k;
				}

				k = 1; // object or background pixel count, 초기값은 1.
			}
			else if (pc2 > 0) { // 최소 land size filtering
				k = last_gap_size + k + 1;
			}
			else { // gap에서 land로
				last_gap_size = k;

				gap ++;
				gap_sum += k;
				m_vtrGap.push_back(k);

				k = 1; // object or background pixel count, 초기값은 1.
			}
			pc2 = pc1;
		}
	} while (TRUE);

	if (pc2 > 0) {
		land ++;
		land_sum += k;
		m_vtrLandPos.push_back(index - k);
		m_vtrLandCenterPos.push_back(index - k/2);
		m_vtrGapPos.push_back(index);
		m_vtrLand.push_back(k);

		if (land > 1) {
			last_land = k;
		}
		else {
			first_land = k;
		}
	}

	if (land == 0) {
		land_av = 0;
		land_min = 0;
		land_max = 0;
	}
	else if (land == 1) {
		land_av = (double)land_sum;
		land_min = land_max = (long)land_av;
	}
	else if (land == 2) {
		if ( m_vtrLand.at(0) >= m_vtrLand.at(1) ) {
			land_max = m_vtrLand.at(0);
			land_min = m_vtrLand.at(1);
		}
		else {
			land_max = m_vtrLand.at(1);
			land_min = m_vtrLand.at(0);
		}
		double temp_land = (double)land_min + ((double)land_min * 0.5);
		if (land_max >= temp_land) { // 첫 번째 혹은 마지막 리드에 패드가 포함되는 경우
			land_av = (double)land_min;
		}
		else {
			land_av = (double)(land_min + land_max) / 2.;
		}
	}
	else 
	{ // if land >= 3
		double temp_land = (double)(land_sum - first_land - last_land)/(double)(land - 2);
		temp_land = temp_land + (temp_land * 0.5);

		RUN_WIDTH::iterator start, end, location;
		start = m_vtrLand.begin();
		end = m_vtrLand.end();

		if ( first_land >= (long)temp_land && last_land >= (long)temp_land ) {
			land_av = (double)(land_sum - first_land - last_land) / (double)(land - 2);
			location = min_element(start + 1, end - 1);
			land_min = *location;
			location = max_element(start + 1, end - 1);
			land_max = *location;
		}
		else if ( first_land >= (long)temp_land && last_land < (long)temp_land ) {
			land_av = (double)(land_sum - first_land) / (double)(land - 1);
			location = min_element(start + 1, end);
			land_min = *location;
			location = max_element(start + 1, end);
			land_max = *location;
		}
		else if ( first_land < (long)temp_land && last_land >= (long)temp_land ) {
			land_av = (double)(land_sum - last_land) / (double)(land - 1);
			location = min_element(start, end - 1);
			land_min = *location;
			location = max_element(start, end - 1);
			land_max = *location;
		}
		else {
			land_av = (double)(land_sum) / (double)(land);
			location = min_element(start, end);
			land_min = *location;
			location = max_element(start, end);
			land_max = *location;
		}

		//if (size != m_vtrLand.size()) {
		//	start		= m_vtrLand.begin();
		//	end			= m_vtrLand.end();
		//	land_sum	= accumulate(start, end, 0);
		//	land_av		= (double)land_sum / (double)m_vtrLand.size();
		//	location	= min_element(start, end);
		//	land_min	= *location;
		//	location	= max_element(start, end);
		//	land_max	= *location;
		//}
	}

	if (gap == 0) {
		gap_av = 0;
		gap_min = 0;
		gap_max = 0;
	}
	else if (gap == 1) {
		gap_av = (double)gap_sum/(double)gap;
		gap_min = gap_max = (long)gap_av;
	}
	else {
		RUN_WIDTH::iterator start, end, location;
		start = m_vtrGap.begin();
		end = m_vtrGap.end();

		gap_av = (double)(gap_sum)/(double)(gap);
		location = min_element(start, end);
		gap_min = *location;
		location = max_element(start, end);
		gap_max = *location;
	}


	//////////////////////////////////////////////////////////////////////////

	if(land > 1)
	{
		int countTemp = 0;
		int* indexTemp = NULL;

		double tempSumW = 0;
		double stdLandW = 0;
		for(int i = 0; i < land; i++)
		{
			tempSumW += m_vtrLand.at(i);
		}

		double per = 0.7;
		if(!widthFlag) //short lead
			per = 0.5;

		stdLandW = tempSumW / land;
		stdLandW *= per;


		//indexTemp = new int[land];
		indexTemp = g_pMManager->pem_new<int>(true, land, (PCHAR)__FUNCTION__, __LINE__);
		memset(indexTemp, 0, sizeof(int) * land);

		for(int i = 0; i < land; i++)
		{
			if(stdLandW > m_vtrLand.at(i))
			{
				indexTemp[countTemp] = i;
				countTemp++;

			}
		}
		
		//////////////////////////////////////////////////////////////////////////
		if(countTemp > 0)
		{
			for(int i = countTemp - 1; i >= 0; i--)
			{
				m_vtrLand.erase(m_vtrLand.begin() + indexTemp[i]);
				m_vtrLandPos.erase(m_vtrLandPos.begin() + indexTemp[i]);
				m_vtrLandCenterPos.erase(m_vtrLandCenterPos.begin() + indexTemp[i]);
			}

			land -= countTemp;
			gap -= countTemp;

			m_vtrGap.clear();
			m_vtrGapPos.clear();

			double sumTemp_lead = 0;
			double sumTemp_gap = 0;
			int minLead = 0;
			int minGap = 0;
			for(int i = 0; i < land; i++)
			{
				m_vtrGapPos.push_back(m_vtrLandPos.at(i) + m_vtrLand.at(i));
				if(i < land - 1)
				{
					m_vtrGap.push_back(m_vtrLandPos.at(i + 1) - m_vtrGapPos.at(i));
					sumTemp_gap += m_vtrGap.at(i);

					if(minGap >  m_vtrGap.at(i))
						minGap = m_vtrGap.at(i);
				}


				sumTemp_lead += m_vtrLand.at(i);


				if(minLead >  m_vtrLand.at(i))
					minLead = m_vtrLand.at(i);

			}

			land_av = sumTemp_lead / land;
			gap_av = sumTemp_gap / land;

			land_min = minLead;
			gap_min = minGap;
		}

		if(indexTemp != NULL)
			//delete indexTemp;
			g_pMManager->pem_delete(indexTemp, false);
	}
	//////////////////////////////////////////////////////////////////////////

	m_avgLead	= land_av; // land average width
	m_avgGap	= gap_av; // gap average width
	m_minLead	= land_min; // land의 최소값
	m_minGap	= gap_min; // gap의 최소값
	m_maxLead	= land_max; // land의 최대값
	m_maxGap	= gap_max; // gap의 최대값
	m_countLead	= land; // land count
	m_countGap	= gap; // gap count

}


void CProcMil_LeadSolder::CalcRunLength2(const long nCount, const long *line, const long min_land_size, BOOL widthFlag, int stdLeadCount, double stdLeadWidth, int** retPitch)
{
	long k = 0;
	long land = 0, gap = 0;
	long land_min = 0, gap_min = 0, land_max = 0, gap_max = 0;
	long first_land, last_land;
	long land_sum = 0, gap_sum = 0;
	double land_av = 0.0, gap_av = 0.0;

	// 데이타 개수가 없으면 .....
	if (nCount <= 0) {
		return;
	}

	// 이전의 데이타를 모두 삭제
	m_vtrLandPos.clear();
	m_vtrGapPos.clear();
	m_vtrLandCenterPos.clear();
	m_vtrLand.clear();
	m_vtrGap.clear();

	long pc1 = 0L, pc2 = 0L;
	long index = 0L;

	pc1 = line[index];

	/* 첫번째 index의 projection intensity가 0이면 무시 한다 */
	if (pc1 == 0L) {
		do {
			index ++;
			if (index >= nCount) {
				break;
			}

			pc2 = line[index];
		} while (pc2 == pc1);

		k = 1;
	}
	else {
		k = 1;
		pc2 = pc1;
	}

	// land관련 정보
	land	 = 0;
	land_sum = 0;
	land_min = 0x7FFF;
	land_max = 0;
	// gap관련 정보
	gap 	 = 0;
	gap_sum  = 0;
	gap_min  = 0x7FFF;
	gap_max  = 0;

	int last_gap_size = 0;
	first_land = 0;
	last_land = 0;

	do {
		index ++;
		if (index >= nCount) {
			break;
		}

		pc1 = line[index];

		if ((pc1>0 && pc2>0) || (pc1==0 && pc2==0)) {
			// pc1과 pc2가 0보다 크면 lead
			// pc1과 pc2가 0이면 gap
			// pc1과 pc2가 다르면 lead에서 gap으로 혹은 gap에서 lead로의 변환임
			k ++;
		}
		else {
			if (pc2 > 0 && k > min_land_size) { // land에서 gap으로
				land ++;
				land_sum += k;
				m_vtrLandPos.push_back(index - k);
				m_vtrGapPos.push_back(index);
				m_vtrLandCenterPos.push_back(index - k/2);
				m_vtrLand.push_back(k-1);

				if (land > 1) {
					last_land = k;
				}
				else {
					first_land = k;
				}

				k = 1; // object or background pixel count, 초기값은 1.
			}
			else if (pc2 > 0) { // 최소 land size filtering
				k = last_gap_size + k + 1;
			}
			else { // gap에서 land로
				last_gap_size = k;

				gap ++;
				gap_sum += k;
				m_vtrGap.push_back(k);

				k = 1; // object or background pixel count, 초기값은 1.
			}
			pc2 = pc1;
		}
	} while (TRUE);

	if (pc2 > 0) {
		land ++;
		land_sum += k;
		m_vtrLandPos.push_back(index - k);
		m_vtrLandCenterPos.push_back(index - k/2);
		m_vtrGapPos.push_back(index);
		m_vtrLand.push_back(k);

		if (land > 1) {
			last_land = k;
		}
		else {
			first_land = k;
		}
	}

	if (land == 0) {
		land_av = 0;
		land_min = 0;
		land_max = 0;
	}
	else if (land == 1) {
		land_av = (double)land_sum;
		land_min = land_max = (long)land_av;
	}
	else if (land == 2) {
		if ( m_vtrLand.at(0) >= m_vtrLand.at(1) ) {
			land_max = m_vtrLand.at(0);
			land_min = m_vtrLand.at(1);
		}
		else {
			land_max = m_vtrLand.at(1);
			land_min = m_vtrLand.at(0);
		}
		double temp_land = (double)land_min + ((double)land_min * 0.5);
		if (land_max >= temp_land) { // 첫 번째 혹은 마지막 리드에 패드가 포함되는 경우
			land_av = (double)land_min;
		}
		else {
			land_av = (double)(land_min + land_max) / 2.;
		}
	}
	else 
	{ // if land >= 3
		double temp_land = (double)(land_sum - first_land - last_land)/(double)(land - 2);
		temp_land = temp_land + (temp_land * 0.5);

		RUN_WIDTH::iterator start, end, location;
		start = m_vtrLand.begin();
		end = m_vtrLand.end();

		if ( first_land >= (long)temp_land && last_land >= (long)temp_land ) {
			land_av = (double)(land_sum - first_land - last_land) / (double)(land - 2);
			location = min_element(start + 1, end - 1);
			land_min = *location;
			location = max_element(start + 1, end - 1);
			land_max = *location;
		}
		else if ( first_land >= (long)temp_land && last_land < (long)temp_land ) {
			land_av = (double)(land_sum - first_land) / (double)(land - 1);
			location = min_element(start + 1, end);
			land_min = *location;
			location = max_element(start + 1, end);
			land_max = *location;
		}
		else if ( first_land < (long)temp_land && last_land >= (long)temp_land ) {
			land_av = (double)(land_sum - last_land) / (double)(land - 1);
			location = min_element(start, end - 1);
			land_min = *location;
			location = max_element(start, end - 1);
			land_max = *location;
		}
		else {
			land_av = (double)(land_sum) / (double)(land);
			location = min_element(start, end);
			land_min = *location;
			location = max_element(start, end);
			land_max = *location;
		}

		//if (size != m_vtrLand.size()) {
		//	start		= m_vtrLand.begin();
		//	end			= m_vtrLand.end();
		//	land_sum	= accumulate(start, end, 0);
		//	land_av		= (double)land_sum / (double)m_vtrLand.size();
		//	location	= min_element(start, end);
		//	land_min	= *location;
		//	location	= max_element(start, end);
		//	land_max	= *location;
		//}
	}

	if (gap == 0) {
		gap_av = 0;
		gap_min = 0;
		gap_max = 0;
	}
	else if (gap == 1) {
		gap_av = (double)gap_sum/(double)gap;
		gap_min = gap_max = (long)gap_av;
	}
	else {
		RUN_WIDTH::iterator start, end, location;
		start = m_vtrGap.begin();
		end = m_vtrGap.end();

		gap_av = (double)(gap_sum)/(double)(gap);
		location = min_element(start, end);
		gap_min = *location;
		location = max_element(start, end);
		gap_max = *location;
	}


	//////////////////////////////////////////////////////////////////////////

	if(land > 1)
	{
		int countTemp = 0;
		int* indexTemp = NULL;
		bool isInsp = false;

		if(stdLeadCount == 0 && stdLeadWidth == 0)//티칭시 실행 (기준값을 리드 폭의 평균으로 함)
		{
			isInsp = false;

			double tempSumW = 0;
			double stdLandW = 0;
			for(int i = 0; i < land; i++)
			{
				tempSumW += m_vtrLand.at(i);
			}

			double per = 0.7;
			if(!widthFlag) //short lead
				per = 0.5;

			stdLandW = tempSumW / land;
			stdLandW *= per;

			//indexTemp = new int[land];
			indexTemp = g_pMManager->pem_new<int>(true, land, (PCHAR)__FUNCTION__, __LINE__);
			memset(indexTemp, 0, sizeof(int) * land);

			for(int i = 0; i < land; i++)
			{
				if(stdLandW > m_vtrLand.at(i))
				{
					indexTemp[countTemp] = i;
					countTemp++;
				}
			}
		}
		else //검사시 실행  (기준값을 티칭된 리드의 폭으로 함)
		{
			isInsp = true;
			if(stdLeadCount < land)
			{
				double stdLandW = stdLeadWidth * 0.7;

				//indexTemp = new int[land];
				indexTemp = g_pMManager->pem_new<int>(true, land, (PCHAR)__FUNCTION__, __LINE__);
				memset(indexTemp, 0, sizeof(int) * land);

				for(int i = 0; i < land; i++)
				{
					if(m_vtrLand.at(i) < stdLandW)
					{
						indexTemp[countTemp] = i;
						countTemp++;
					}
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////
		if(countTemp > 0 && gap > countTemp) //폭이 작은 리드 제거   //SHK 20140429
		{
			for(int i = countTemp - 1; i >= 0; i--)
			{
				m_vtrLand.erase(m_vtrLand.begin() + indexTemp[i]);
				m_vtrLandPos.erase(m_vtrLandPos.begin() + indexTemp[i]);
				m_vtrLandCenterPos.erase(m_vtrLandCenterPos.begin() + indexTemp[i]);
			}

			land -= countTemp;
			gap -= countTemp;

			//gap정보 재구성
			m_vtrGap.clear();
			m_vtrGapPos.clear();

			double sumTemp_lead = 0;
			double sumTemp_gap = 0;
			int minLead = 100000;
			int minGap = 100000;
			for(int i = 0; i < land; i++)
			{
				m_vtrGapPos.push_back(m_vtrLandPos.at(i) + m_vtrLand.at(i));
				if(i < land - 1)
				{
					m_vtrGap.push_back(m_vtrLandPos.at(i + 1) - m_vtrGapPos.at(i));
					sumTemp_gap += m_vtrGap.at(i);

					if(minGap >  m_vtrGap.at(i))
						minGap = m_vtrGap.at(i);
				}


				sumTemp_lead += m_vtrLand.at(i);
				

				if(minLead >  m_vtrLand.at(i))
					minLead = m_vtrLand.at(i);
				
			}

			land_av = sumTemp_lead / land;
			gap_av = sumTemp_gap / land;

			land_min = minLead;
			gap_min = minGap;
		}


		if(indexTemp != NULL)
			//delete indexTemp;
			g_pMManager->pem_delete(indexTemp, false);



		//////////////////////////////////////////////////////////////////////////
		//lead영역 재구성 후 기준 리드 개수랑 다를경우..(검사시에만 실행)
		if(isInsp)//검사시 실행
		{
			if(stdLeadCount != land)
			{
				//평균 pitch계산
				double stdPitch = 0;
				for(int i = 0; i < gap; i++)
				{
					stdPitch += m_vtrLandCenterPos.at(i + 1) - m_vtrLandCenterPos.at(i);
				}
				stdPitch /= gap;

				if(stdLeadCount > land)//기준 리드개수 보다 계산된 리드의 개수가 작을때
				{	
					int landCountTemp = land;
					int gapCountTemp = gap;

					POSITION_1D	vtrLandTemp;
					POSITION_1D	vtrLandPosTemp;
					POSITION_1D vtrLandCenterPosTemp;
					
					vtrLandTemp.clear();
					vtrLandPosTemp.clear();
					vtrLandCenterPosTemp.clear();
					for(int i = 0; i < land; i++)
					{
						vtrLandPosTemp.push_back(m_vtrLandPos.at(i));						

						if(m_vtrLand.at(i) >= stdPitch)
						{
							//width
							int landTemp = (int)((m_vtrLand.at(i) - gap_av) / 2.0);
							vtrLandTemp.push_back(landTemp);
							vtrLandTemp.push_back(landTemp);

							//left pos
							vtrLandPosTemp.push_back((int)(m_vtrLandPos.at(i) + landTemp + gap_av));
							
							//center pos
							vtrLandCenterPosTemp.push_back(vtrLandPosTemp.at(i) + (int)(land_av / 2.0));
							vtrLandCenterPosTemp.push_back(vtrLandPosTemp.at(i+1) + (int)(land_av / 2.0));

							landCountTemp++;
							gapCountTemp++;
						}
						else
						{
							vtrLandTemp.push_back(m_vtrLand.at(i));
							vtrLandCenterPosTemp.push_back(m_vtrLandCenterPos.at(i));
						}
					}

					m_vtrLand.clear();
					m_vtrLandPos.clear();
					m_vtrLandCenterPos.clear();
					m_vtrLand = vtrLandTemp;
					m_vtrLandPos = vtrLandPosTemp;
					m_vtrLandCenterPos = vtrLandCenterPosTemp;

					land = landCountTemp;
					gap = gapCountTemp;



					//gap정보 재구성
					m_vtrGap.clear();
					m_vtrGapPos.clear();

					double sumTemp_lead = 0;
					double sumTemp_gap = 0;
					int minLead = 100000;
					int minGap = 100000;
					int maxLead = 0;
					int maxGap = 0;
					for(int i = 0; i < land; i++)
					{
						m_vtrGapPos.push_back(m_vtrLandPos.at(i) + m_vtrLand.at(i));
						if(i < land - 1)
						{
							m_vtrGap.push_back(m_vtrLandPos.at(i + 1) - m_vtrGapPos.at(i));
							sumTemp_gap += m_vtrGap.at(i);

							if(minGap >  m_vtrGap.at(i))
								minGap = m_vtrGap.at(i);

							if(maxGap < m_vtrGap.at(i))
								maxGap = m_vtrGap.at(i);
						}


						sumTemp_lead += m_vtrLand.at(i);


						if(minLead >  m_vtrLand.at(i))
							minLead = m_vtrLand.at(i);

						if(maxLead <  m_vtrLand.at(i))
							maxLead = m_vtrLand.at(i);
					}

					land_av = sumTemp_lead / land;
					gap_av = sumTemp_gap / land;

					land_min = minLead;
					gap_min = minGap;

					land_max = maxLead;
					gap_max = maxGap;
				}
			}
		}


		//////////////////////////////////////////////////////////////////////////
		//pitch 계산
		//int* leadPitch = new int[gap];
		int* leadPitch = g_pMManager->pem_new<int>(true, gap, (PCHAR)__FUNCTION__, __LINE__);
		double sum = 0;
		for(int i = 0; i < gap; i++)
		{
			leadPitch[i] = (m_vtrLandCenterPos.at(i + 1) - m_vtrLandCenterPos.at(i));
			sum += leadPitch[i];
		}

		m_avgPitch = sum / gap;

		//*retPitch = new int[gap];
		*retPitch = g_pMManager->pem_new<int>(true, gap, (PCHAR)__FUNCTION__, __LINE__);
		memcpy_s(*retPitch, sizeof(int) * gap, leadPitch, sizeof(int) * gap);
		//delete leadPitch;		
		g_pMManager->pem_delete(leadPitch, false);
		////////	



	}
	else
	{
		m_avgPitch = 0;
	}
	//////////////////////////////////////////////////////////////////////////

	m_avgLead	= land_av; // land average width
	m_avgGap	= gap_av; // gap average width
	m_minLead	= land_min; // land의 최소값
	m_minGap	= gap_min; // gap의 최소값
	m_maxLead	= land_max; // land의 최대값
	m_maxGap	= gap_max; // gap의 최대값
	m_countLead	= land; // land count
	m_countGap	= gap; // gap count

}


double CProcMil_LeadSolder::CalcSolderArea(UCHAR* userSub, int sizeX, int sizeY, int threshold)
{
	if(userSub == NULL || sizeX < 1 || sizeY < 1)
		return -1;

	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(sizeX, sizeY);
	Im::Buf::Put(milSrc, userSub);
	SaveWorkImg(milSrc, _T("solder.bmp"));

	//int threshold2 = GetOtsuThreshold(milSrc);
	int thres = threshold;//GetThreshold(milSrc);

	if(thres <= 0)
		thres = GetThreshold(milSrc);

	FreeMilImageBuff(&milSrc);
	//////////////////////////////////////////////////////////////////////////

	int count = 0;
	int nRect =0;

	nRect = sizeX * sizeY;
	for(int i=0; i<nRect; i++)
	{
		if( userSub[i] < threshold )
			count++;
	}

	double nWhitePer =0;
	nWhitePer = (double)((double)count / (double)nRect) * 100.0;


	return nWhitePer;
}



//////////////////////////////////////////////////////////////////////////


int CProcMil_LeadSolder::InspSolderArea(float* zmapData, UCHAR* img, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int stPoint, int endPoint, int isChipsolder, int* retFiletL, int* retFiletH, double* retSolderRatio, BOOL* retOverEndline)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	Im::PIL_ID milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
	Im::Buf::Put(milZmapSrc, zmapData);

	float thresholdH_st = (float)(avrLeadH * 0.8);
	float thresholdH_ed = (float)(avrLeadH * 0.2);

	//if(isChipsolder)//chip
	//{
		if(thresholdH_ed > 50)
			thresholdH_ed = 50;
		else if(thresholdH_ed < 30)
			thresholdH_ed = 30;
	//}

	int margin = 20;   //solder 시작점에서 margin만큼 뺀 점을 시작점으로 한다.
	int widthMargin = 4;  //폭을 좌루 2개씩 줄임
	Im::PIL_ID milClipZmap = M_NULL;

	int solderW = 0;
	int solderH = 0;
	int cx = 0;
	int cy = 0;
	int temp = 0;
	for(int i = 0; i < solderCnt; i++)
	{

		solderW = (int)(solderRect[i].Width() * 0.6);
		solderH = (int)solderRect[i].Height() + margin;


		//cx = (int)((solderRect[i].left + (widthMargin / 2.0)) + (solderW / 2.0));

		cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));

		temp = solderRect[i].top - margin;
		if(temp < 0)
		{
			solderH -= abs(temp);
			temp = 0;		
		}
		cy = (int)(temp + (solderH / 2.0));		


		milClipZmap = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH); //zmap에서 리드팁부분을 (margin만큼)포함한 솔더 영역 클립


		//long* prjData = new long[solderH];
		long* prjData = g_pMManager->pem_new<long>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(prjData, 0, sizeof(long) * solderH);
		GetProjection_V(milClipZmap, prjData);  //projection 한다.



		//////////////////////////////////////////////////////////////////////////
		//시작, 끝점 찾기

		//중간 기준점
		int st = 0;
		int ed = 0;
		int centerPt = 0;
		int centerPt2 = 0;
		BOOL centerFlag = TRUE;	
		if(isChipsolder)//chip solder
		{
			st = 0;
			ed = solderH;
			for(int j = st; j < ed; j++)
			{
				prjData[j] /= solderW;

				if(j > 0)
				{
					if(centerFlag == TRUE && prjData[j - 1] >= thresholdH_st && prjData[j] <= thresholdH_st)
					{	
						centerPt = j;
						centerFlag = FALSE;
					}
				}
			}

			centerPt2 = centerPt;

			long tempSum = 0;
			long tempAvr = 0;
			for(int j = 0; j < centerPt; j++)
			{
				tempSum += prjData[j];
			}

			if(centerPt != 0)
			{
				tempAvr = tempSum / centerPt;


				centerFlag = TRUE;	
				int subTemp1 = 0;
				int subTemp2 = 0;
				for(int j = st; j < ed; j++)
				{
					if(j > 0)
					{
						subTemp1 = prjData[j - 1] - tempAvr;
						subTemp2 = tempAvr - prjData[j];
						if(centerFlag == TRUE && (subTemp1 * subTemp2) >= 0)
						{	
							centerPt2 = j;
							centerFlag = FALSE;
						}
					}
				}
			}

		}
		else// lead solder
		{
			st = solderH - 1;
			ed = 0;
			for(int j = st; j >= ed; j--)
			{
				prjData[j] /= solderW;

				if(centerFlag == TRUE && prjData[j] > thresholdH_st)
				{
					centerPt2 = j;
					centerFlag = FALSE;
				}
			}

			centerPt = centerPt2;
		}



		//시작점
		double curArea = 0;

		int startPos = 0;
		int endPos = solderH;
		BOOL endFlag = FALSE;
		int startPosMargin = 0;


		if(centerPt2 == 0)
		{
			startPos = startPosMargin;			
		}
		else
		{
			int tempIndex = 0;
			long maxData =  prjData[centerPt2];
			startPos = centerPt2;

// 			if(!isChipsolder)
// 			{
				for(int j = 0; j < 5; j++)
				{
					tempIndex = centerPt2 - j;
					if(tempIndex < 0)
						break;

					if(maxData < prjData[tempIndex])
					{
						maxData = prjData[tempIndex];
						startPos = tempIndex + startPosMargin;
					}
				}
// 			}
// 			else
// 			{
// 
// 				for(int j = centerPt - 1; j >= 0; j--)
// 				{
// 					if(maxData < prjData[j])
// 					{
// 						maxData = prjData[j];
// 						startPos = j + startPosMargin;
// 					}
// 				}
// 			}
		}

		//끝점
		if(centerPt == solderH)
		{
			endPos = solderH;
			endFlag = TRUE;
		}
		else
		{
			for(int j = solderH - 1; j >= centerPt; j--)
			{	
				if(j - 2 >= 0)
				{
					if(prjData[j] <= thresholdH_ed && prjData[j - 1] <= thresholdH_ed && prjData[j - 2] >= thresholdH_ed)
					{
						endPos = j;
						endFlag = TRUE;
						break;
					}
				}
			}


// 			for(int j = centerPt; j < solderH - 1; j++)
// 			{				
// 				if(prjData[j] <= thresholdH_ed && prjData[j + 1] <= thresholdH_ed)
// 				{
// 					endPos = j;
// 					endFlag = TRUE;
// 					break;
// 				}
// 			}
		}



		for(int j = startPos; j < endPos; j++)
		{
			curArea += prjData[j];
		}

		//////////////////////////////////////////////////////////////////////////
		//2d 
/*		CRect solderRt;
		solderRt.left = (LONG)(cx - (solderW / 2.0));
		solderRt.right = (LONG)(cx + (solderW / 2.0));
		solderRt.top = (LONG)((cy - (solderH / 2.0)) + startPos);
		solderRt.bottom = (LONG)(solderRt.top + (endPos - startPos));
		CPoint ct = solderRt.CenterPoint();

		UCHAR* solderClip = NULL;
		solderClip = new UCHAR[solderRt.Width() * solderRt.Height()];
		GetClipImage(img, roiSizeX, roiSizeY, solderClip, ct.x, ct.y, solderRt.Width(), solderRt.Height());
		SaveWorkImg(img, roiSizeX, roiSizeY, _T("org.bmp"));
		SaveWorkImg(solderClip, solderRt.Width(), solderRt.Height(), _T("solder.bmp"));
		delete solderClip;*/
		//////////////////////////////////////////////////////////////////////////
		// Data return

		//Filet length
		int l = 0;
		if(retFiletL != NULL)
		{
			l = (int)(endPos - startPos);

			retFiletL[i] = l;
		}

		//Filet Height
		int startH = prjData[startPos];
		if(retFiletH != NULL)
		{		  
			retFiletH[i] = startH;
		}

		//Solder Ratio
		int endH = prjData[endPos];
		double tmpBase = (endPos - startPos)*endH;
		if(retSolderRatio != NULL)
		{
			double stdArea = ((startH - endH) * (endPos - startPos));// / 2.0; //직각 삼각형 면적
			double areaRatio = 0;
			if(stdArea > 0)
				areaRatio = ((curArea-tmpBase) / stdArea) * 100.0;

			retSolderRatio[i] = areaRatio;
		}


		if(retOverEndline != NULL)
		{
			retOverEndline[i] = endFlag;
		}

		//////////////////////////////////////////////////////////////////////////


		//delete prjData;
		g_pMManager->pem_delete(prjData, false);
		FreeMilImageBuff(&milClipZmap);	
	}

	FreeMilImageBuff(&milZmapSrc);

	return ret;
}


// LMJ 2014/01/20	: solderVolum 인자 추가
//int CProcMil_LeadSolder::InspSolderArea2(float* zmapData, UCHAR* img, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int* retFiletL, int* retFiletStart)
int CProcMil_LeadSolder::InspSolderArea2(float* zmapData, UCHAR* img, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int* retFiletL, int* retFiletStart, double* solderVolum)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	float thresholdH_st = (float)(avrLeadH * 0.8);

	if((avrLeadH * 0.2) >= 150)
		thresholdH_st = avrLeadH - 150;

	float thresholdH_st2 = (float)(avrLeadH * 1.15);

	float thresholdH_ed = 30;

	if(!isChipsolder)
		thresholdH_ed = 15;

	Im::PIL_ID milZmapSrc = M_NULL;
	Im::PIL_ID milClipZmap = M_NULL;
	float* clipZmap = NULL;

	if(!isChipsolder)
	{
		milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
		Im::Buf::Put(milZmapSrc, zmapData);
	}


	int startPoint = 0;
	int endPoint = 0;

	int* pixelCnt_st = NULL; 
	int* remakePixelCnt_st = NULL;
	int* pixelCnt_ed = NULL; 
	float data = 0;
	int index = 0;
	int cx = 0;
	int cy = 0;
	int margin = 10;
	int solderW = 0;
	int solderH = 0;
	int temp = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		solderW = solderRect[i].Width();
		solderH = solderRect[i].Height();

		//clip zmap
		if(isChipsolder)
		{
			clipZmap = zmapData;
		}
		else
		{		
			solderW = solderRect[i].Width();
			solderH = solderRect[i].Height() + margin;

			cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));
			cy = (int)((solderRect[i].top + (solderRect[i].Height() / 2.0)) - (margin / 2.0));

			temp = (int)(cy - (solderH / 2.0));
			if(temp < 0)
			{
				cy += abs(temp);
				solderH -= abs(temp);
			}


			milClipZmap = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH);
			//clipZmap = new float[solderW * solderH];
			clipZmap = g_pMManager->pem_new<float>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);

			Im::Buf::Get(milClipZmap, clipZmap);
		}

		//////////////////////////////////////////////////////////////////////////
		 //test image save
	/*	Im::PIL_ID milSrc = AllocBuff(roiSizeX, roiSizeY);
		Im::Buf::Put(milSrc, img);
		Im::PIL_ID milClipImg = AllocClipBuff(milSrc, cx, cy, solderW, solderH);

		CString strImgName;
		strImgName.Format(_T("solderImg_%d.bmp"), i);
		SaveWorkImg(milClipImg, strImgName);

		FreeMilImageBuff(&milSrc);
		FreeMilImageBuff(&milClipImg);*/
		//////////////////////////////////////////////////////////////////////////



		//data count
		//pixelCnt_st = new int[solderH];
		pixelCnt_st = g_pMManager->pem_new<int>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(pixelCnt_st, 0, sizeof(int) * solderH);

		//pixelCnt_ed = new int[solderH];
		pixelCnt_ed = g_pMManager->pem_new<int>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(pixelCnt_ed, 0, sizeof(int) * solderH);

		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				index = (y * solderW) + x;
				data = clipZmap[index];
				if(data >= thresholdH_st && data <= thresholdH_st2)
					pixelCnt_st[y]++;

				if(data >= thresholdH_ed)
					pixelCnt_ed[y]++;
			}
		}

		//remake data
		
		//remakePixelCnt_st = new int[solderH];
		remakePixelCnt_st = g_pMManager->pem_new<int>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(remakePixelCnt_st, 0, sizeof(int) * solderH);

		double per = isChipsolder?  0.5 : 0.3;
		int stdWidth = (int)(solderW * per);

		int dataMax = 0;
		int dataMaxIndex = 0;
		int st = 1;
		int ed = solderH - 1;

		if(!isChipsolder)
		{
			ed = solderH - 2;
		}

		for(int j = st; j < ed; j++)
		{
			if(pixelCnt_st[j - 1] * pixelCnt_st[j] * pixelCnt_st[j + 1] == 0)
			{
				remakePixelCnt_st[j] = 0;
			}
			else
			{
				if(isChipsolder)
					remakePixelCnt_st[j] = (2 * pixelCnt_st[j]) - pixelCnt_st[j - 1] - pixelCnt_st[j + 1];
				else
					remakePixelCnt_st[j] = (2 * pixelCnt_st[j]) - pixelCnt_st[j + 1]- pixelCnt_st[j + 2];
					//remakePixelCnt_st[j] = 2 * (pixelCnt_st[j] - pixelCnt_st[j + 1]);
			}


			if(pixelCnt_st[j] >= stdWidth)
			{
				if(dataMax <= remakePixelCnt_st[j])
				{
					dataMax = remakePixelCnt_st[j];
					dataMaxIndex = j;
				}
			}
		}


		//start point
		startPoint = dataMaxIndex;

		//huj 2014/01/14(일단 제거.. 문제 없음)
	/*	BOOL stFlag = FALSE;

		int subTemp = 0;

		if(isChipsolder)
		{
			for(int j = dataMaxIndex; j >= 0; j--)
			{		
				if(remakePixelCnt_st[j] <= -3)
				{	
					startPoint = j;
					break;
				}
			}
		}*/

		//////////////////////////////////////////////////////////////////////////
		//end point
		int reCnt = 3;
		endPoint = solderH - 1;
		for(int j = startPoint; j < solderH - (reCnt - 1); j++)
		{
			if(pixelCnt_ed[j] == 0)
			{	
				if(pixelCnt_ed[j + (reCnt - 2)] + pixelCnt_ed[j + (reCnt - 1)] == 0)
				{
					if(j != 0)
						endPoint = j - 1;
					else
						endPoint = j;

					break;
				}
			}
		}

	
		//data save
		/*CString stPath;
		CString edPath;

		stPath.Format(_T("d:\\testimage\\data\\start%d.csv"),i);
		DataSave(pixelCnt_st, solderH, stPath);

		edPath.Format(_T("d:\\testimage\\data\\end%d.csv"),i);
		DataSave(pixelCnt_ed, solderH, edPath);
		*/
		// data return
		int l = 0;
		if(retFiletL != NULL)
		{
			l = (int)(endPoint - startPoint);
			if(l < 0)//huj 2014/01/14
				l = 0;

			retFiletL[i] = l;
		}

		if(retFiletStart != NULL)
		{
			retFiletStart[i] = startPoint - margin;
		}

		// LMJ 2014/01/20
		// Solder Volume return <-----------------------------------
		double resol_x = m_resolX;
		double resol_y = m_resolY;
		double sum_vol = 0, vol;

		float thresholdH_st_temp = (float)(avrLeadH * 0.6);

		if(l > 0)
		{
			for(int y = startPoint; y <= endPoint; y++)
			{
				for(int x = 0; x <= solderW; x++)
				{
					if(clipZmap[y * solderW + x] >= thresholdH_ed && clipZmap[y * solderW + x] <= thresholdH_st_temp)
					{
						vol = _micron2mm(clipZmap[y * solderW + x]) *  resol_x * resol_y;
						sum_vol += vol;
					}
				}
			}
		}

		if(solderVolum)
		{
			if(sum_vol > 0)
			{
				solderVolum[i] = _round(sum_vol, 5);
			}
			else
				solderVolum[i] = 0;
		}
		//--------------------------------------------------------------->



		//delete pixelCnt_st;
		g_pMManager->pem_delete(pixelCnt_st, false);
		pixelCnt_st = NULL;

		//delete remakePixelCnt_st;
		g_pMManager->pem_delete(remakePixelCnt_st, false);
		remakePixelCnt_st = NULL;

		//delete pixelCnt_ed;
		g_pMManager->pem_delete(pixelCnt_ed, false);
		pixelCnt_ed = NULL;


		if(!isChipsolder)
		{
			if(clipZmap != NULL)
			{
				//delete clipZmap;
				g_pMManager->pem_delete(clipZmap, false);
				clipZmap = NULL;
			}

			FreeMilImageBuff(&milClipZmap);
		}
	}

	if(pixelCnt_st != NULL)
		//delete pixelCnt_st;
		g_pMManager->pem_delete(pixelCnt_st, false);

	if(remakePixelCnt_st != NULL)
		//delete remakePixelCnt_st;
		g_pMManager->pem_delete(remakePixelCnt_st, false);

	if(pixelCnt_ed != NULL)
		//delete pixelCnt_ed;
		g_pMManager->pem_delete(pixelCnt_ed, false);

	if(!isChipsolder)
	{
		FreeMilImageBuff(&milZmapSrc);

		if(clipZmap != NULL)
			//delete clipZmap;
			g_pMManager->pem_delete(clipZmap, false);
	}

	return ret;
}



int CProcMil_LeadSolder::InspSolderArea3(float* zmapData, UCHAR* img, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int solderInspLength, int* retFiletStart, double* retSolderVolum, float* retSolderRatio)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	float thresholdL_st = (float)(avrLeadH * 0.8);
	float thresholdH_st = (float)(avrLeadH * 1.2);

	Im::PIL_ID milZmapSrc = M_NULL;
	Im::PIL_ID milClipZmap = M_NULL;
	float* clipZmap = NULL;

	float* zmapDataTemp = NULL;
	//zmapDataTemp = new float[roiSizeX * roiSizeY];
	zmapDataTemp = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	ippsThreshold_LTVal_32f(zmapData, zmapDataTemp, roiSizeX * roiSizeY, 0, 0);

	if(!isChipsolder)
	{
		milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
		Im::Buf::Put(milZmapSrc, zmapDataTemp);	
	}
	//////////////////////////////////////////////////////////////////////////

	int cx = 0;
	int cy = 0;
	int margin = 10;
	
	int temp = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		int solderW = solderRect[i].Width();
		int solderH = solderRect[i].Height();

		//clip zmap
		if(isChipsolder)
		{
			clipZmap = zmapDataTemp;
			milClipZmap = AllocBuff_float(solderW, solderH);
			Im::Buf::Put(milClipZmap, zmapDataTemp);
		}
		else
		{		
			solderW = solderRect[i].Width();
			solderH = solderRect[i].Height() + margin;

			cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));
			cy = (int)((solderRect[i].top + (solderRect[i].Height() / 2.0)) - (margin / 2.0));

			temp = (int)(cy - (solderH / 2.0));
			if(temp < 0)
			{
				cy += abs(temp);
				solderH -= abs(temp);
				margin -= abs(temp);
			}


			milClipZmap = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH);
			//clipZmap = new float[solderW * solderH];
			clipZmap = g_pMManager->pem_new<float>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);

			Im::Buf::Get(milClipZmap, clipZmap);
		}

		//////////////////////////////////////////////////////////////////////////
// 		 //test image save
// 		if(!isChipsolder)
// 		{
// 			Im::PIL_ID milSrc = AllocBuff(roiSizeX, roiSizeY);
// 			Im::Buf::Put(milSrc, img);
// 			Im::PIL_ID milClipImg = AllocClipBuff(milSrc, cx, cy, solderW, solderH);
// 
// 			CString strImgName;
// 			strImgName.Format(_T("solderImg_%d.bmp"), i);
// 			SaveWorkImg(milClipImg, strImgName);
// 
// 			FreeMilImageBuff(&milSrc);
// 			FreeMilImageBuff(&milClipImg);
// 		}
// 		else
// 		{
// 			CString strImgName;
// 			strImgName.Format(_T("solderImg_%d.bmp"), i);
// 			SaveWorkImg(img, roiSizeX, roiSizeY, strImgName);
// 		}
		//////////////////////////////////////////////////////////////////////////
		//projection
		
		//thresholdH_st 이상값만 추출
		int zindex = 0;
		int xcnt = 0;

		float overZmap = 0;
		float avrZmap = 0;

		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				zindex = (y * solderW) + x;

				if(clipZmap[zindex] >= thresholdL_st && clipZmap[zindex] < thresholdH_st)
				{
					overZmap += clipZmap[zindex];
					xcnt++;
				} 
			}
		}

		if (xcnt == 0) xcnt = 1;

		avrZmap = overZmap / xcnt;

		//float* prjList = new float[solderH];
		float* prjList = g_pMManager->pem_new<float>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(prjList, 0, sizeof(float) * solderH);
		GetProjection_V(milClipZmap, prjList);

		//blob image
		//UCHAR* procSolderImg = new UCHAR[solderW * solderH];
		UCHAR* procSolderImg = g_pMManager->pem_new<UCHAR>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);

		int index = 0;
		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				index = (y * solderW) + x;

				if(clipZmap[index] >= thresholdL_st && clipZmap[index] < thresholdH_st)//avrZmap)
					procSolderImg[index] = 255;
				else
					procSolderImg[index] = 0;
			}
		}
		SaveWorkImg(procSolderImg, solderW, solderH, _T("procSolderImg.bmp"));

		int blobCnt = CalcBlob(procSolderImg, solderW, solderH, 0, 0, FALSE, FALSE, NULL);

		CMilBlobResult *blobRst = new CMilBlobResult(blobCnt);
		g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

		GetBlobResult(blobRst);

		//첫번째 지점.
		int firstPoint = 0;
		if(blobCnt == 1)
		{
			firstPoint = (int)blobRst->bottom[0];
		}
		else if(blobCnt > 1 )
		{
			int maxIndex = 0;
			double maxArea = blobRst->area[0];
			for(int j = 1; j < blobCnt; j++)
			{
				if(blobRst->area[j] > maxArea)
				{
					maxArea = blobRst->area[j];
					maxIndex = j;
				}
			}

			firstPoint = (int)blobRst->bottom[maxIndex];		
		}

		//////////////////////////////////////////////////////////////////////////
		//huj 2014/02/13
		//int* prjList_2d = new int[solderH];
		int* prjList_2d = g_pMManager->pem_new<int>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(prjList_2d, 0, sizeof(int) * solderH);

		UCHAR data = 0;
		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				index = (y * solderW) + x;
				data = procSolderImg[index];
				if(data > 0)
					prjList_2d[y]++;
			}
		}


		

		//int* remakePixelCnt_st = new int[solderH];
		int* remakePixelCnt_st = g_pMManager->pem_new<int>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(remakePixelCnt_st, 0, sizeof(int) * solderH);

		int minVal = procSolderImg[0];
		int minIndex = 0;
		int dataMax = 0;
		int dataMaxIndex = 0;
		int temp = 0;
		int endp = solderH - 2;
		if(isChipsolder)
			endp = solderH - 1;

		for(int j = 1; j < endp; j++)
		{
			if(prjList_2d[j - 1] * prjList_2d[j] * prjList_2d[j + 1] == 0)
			{
				remakePixelCnt_st[j] = 0;
			}
			else
			{
				if(isChipsolder)
					remakePixelCnt_st[j] = (2 * prjList_2d[j]) - prjList_2d[j - 1] - prjList_2d[j + 1];
				else
					remakePixelCnt_st[j] = (2 * prjList_2d[j]) - prjList_2d[j + 1]- prjList_2d[j + 2];
	
			}

			if(dataMax < remakePixelCnt_st[j])
			{
				dataMax = remakePixelCnt_st[j];
				dataMaxIndex = j;
			}

// 			temp = prjList_2d[j] - prjList_2d[j-1];
// 			if(prjList_2d[j] != 0 && temp < minVal)
// 			{
// 				minVal = temp;
// 				minIndex = j;
// 			}
		}
		firstPoint = dataMaxIndex;

		/*delete prjList_2d;
		delete remakePixelCnt_st;*/
		g_pMManager->pem_delete(prjList_2d, false);
		g_pMManager->pem_delete(remakePixelCnt_st, false);
		//////////////////////////////////////////////////////////////////////////

		//두번째 지점 (solder rect 시작부터 첫번째 지점까지 기울기가 가장 큰 부분을 찾는다.-값)
// 		int secPoint = 0; //최종적으로 구하는 시작점.
// 		float subVal = 0;
// 		float minVal = prjList[1] - prjList[0];
// 		for(int j = 0; j < firstPoint; j++)
// 		{
// 			subVal = prjList[j + 1] - prjList[j];
// 			if(subVal < minVal)
// 			{
// 				minVal = subVal;
// 				secPoint = j;
// 			}
// 
// 		}
// 		
		if(retFiletStart != NULL)
		{
			retFiletStart[i] = firstPoint - margin;
		}

		//////////////////////////////////////////////////////////////////////////
		//test image save
	/*	UCHAR* clipSolderImg = NULL;
		if(isChipsolder)
		{
			clipSolderImg = img;
		}	
		else
		{			
			clipSolderImg = new UCHAR[solderW * solderH];
			GetClipImage(img, roiSizeX, roiSizeY, clipSolderImg, cx, cy, solderW, solderH);
		}

		Im::PIL_ID milBuf = M_NULL;
		milBuf = AllocBuff(solderW, solderH);
		Im::Buf::Put(milBuf, clipSolderImg);
		Im::PIL_ID milTestImg = M_NULL;
		milTestImg = AllocBuffColor(solderW, solderH);
		Im::Buf::CopyColor(milBuf, milTestImg, M_ALL_BANDS);
		FreeMilImageBuff(&milBuf);

		Im::Gra::Color(M_DEFAULT, M_COLOR_GREEN);
		Im::Gra::Line(M_DEFAULT, milTestImg, 0, firstPoint, solderW-1, firstPoint);
		Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);

		int randNum = rand();
		CString testPath = _T("");
		testPath.Format(_T("test\\img_%d.bmp"), randNum);
		SaveWorkImg(milTestImg, testPath);

		FreeMilImageBuff(&milTestImg);
		

		if(!isChipsolder)
		{
			if(clipSolderImg != NULL)
			{
				delete clipSolderImg;
				clipSolderImg = NULL;
			}
		}*/

		
		//////////////////////////////////////////////////////////////////////////
		//shk  2014/02/10

		int startPoint = firstPoint + 1; //+5 magic number

		float* profile1 = NULL;

		int range = 0;

		range = solderH - startPoint;

		if (range <= 0)
		{
			 retFiletStart = 0;
			 retSolderVolum = 0;
			 retSolderRatio = 0;

			 /*delete procSolderImg;
			 delete blobRst;
			 delete prjList;*/
			 g_pMManager->pem_delete(procSolderImg, false);
			 g_pMManager->pem_delete(blobRst, false);
			 g_pMManager->pem_delete(prjList, false);
			 FreeMilImageBuff(&milClipZmap);

			 if(!isChipsolder)
			 {
				 if(clipZmap != NULL)
				 {
					 //delete clipZmap;
					 g_pMManager->pem_delete(clipZmap, false);
					 clipZmap = NULL;
				 }		

				 FreeMilImageBuff(&milZmapSrc);
			 }

			if(zmapDataTemp != NULL)
			{
				//delete zmapDataTemp;
				g_pMManager->pem_delete(zmapDataTemp, false);
				zmapDataTemp = NULL;
			}

			return ret;
		}

		//profile1 = new float[range];
		profile1 = g_pMManager->pem_new<float>(true, range, (PCHAR)__FUNCTION__, __LINE__);
		memset(profile1, 0, sizeof(float) * (range));

		float startpTemp = 0.f;
		float sumTemp12 = 0.f; // 1/2지점까지 ▲
		float sumTemp22 = 0.f; // 1/2 에서 2/2 지점까지 ▲

		float avrTempT = 0.f;
		float endTemp = 0.f; //최소 지점 ■ 면적값 --> 빼줄값

		int indexT = solderInspLength; //지정된 노란 점선 까지
		if(startPoint + solderInspLength >= solderH)
			indexT = range;
		
		if (indexT > 20)
		{
			indexT = 20;
		}

		int indexhalf = (int)(indexT / 2);

		int prIndex1 = 0;	
		for(int y = startPoint; y < solderH; y++)
		{
			for (int x = 0; x < solderW; x++)
			{
				prIndex1 = (y * solderW) + x;
				if( y-startPoint < indexT )
				{
					profile1[y-startPoint] += clipZmap[prIndex1];
				}
			
				//if(y == startPoint) startpTemp += clipZmap[prIndex1];
				if ( y-startPoint < indexhalf )
				{
					sumTemp12 += clipZmap[prIndex1];
				}
				else if( y-startPoint < (indexhalf*2) )
				{
					sumTemp22 += clipZmap[prIndex1];
				}
			}
			

		}
		ippsMin_32f(profile1,indexT,&endTemp);
		endTemp = endTemp*indexhalf; // 전체 ■에서 절반크기

		sumTemp12 = sumTemp12 - endTemp;
		sumTemp22 = sumTemp22 - endTemp;

		//totalTemp13 = (startpTemp * indexT) - endTemp ; //  ■
		if (sumTemp12 <= 0.f)
		{
			avrTempT = 0.f;
		}
		else
		{
			avrTempT = (sumTemp22 / sumTemp12) * 100; 
		}

		if(retSolderRatio != NULL)
			retSolderRatio[i] = avrTempT;

// 		if(avrTemp13 >= 70) cold[i] = TRUE; //과납
// 		if(avrTemp13 > 30 && avrTemp13 < 70 ) cold[i] = FALSE; //정상
// 		if(avrTemp13 < 30 && avrTemp13 > 15 ) cold[i] = TRUE; //소납
// 		if(avrTemp13 < 15 ) cold[i] = TRUE; //미납

		//delete profile1;
		g_pMManager->pem_delete(profile1, false);


		//////////////////////////////////////////////////////////////////////////
		/*delete procSolderImg;
		delete blobRst;
		delete prjList;*/
		g_pMManager->pem_delete(procSolderImg, false);
		g_pMManager->pem_delete(blobRst, false);
		g_pMManager->pem_delete(prjList, false);
		FreeMilImageBuff(&milClipZmap);

		if(!isChipsolder)
		{
			if(clipZmap != NULL)
			{
				//delete clipZmap;
				g_pMManager->pem_delete(clipZmap, false);
				clipZmap = NULL;
			}		
		}
		//////////////////////////////////////////////////////////////////////////		
	}

	if(!isChipsolder)
	{
		FreeMilImageBuff(&milZmapSrc);

		if(clipZmap != NULL)
			//delete clipZmap;
			g_pMManager->pem_delete(clipZmap, false);
	}


	if(zmapDataTemp != NULL)
	{
		//delete zmapDataTemp;
		g_pMManager->pem_delete(zmapDataTemp, false);
		zmapDataTemp = NULL;
	}

	return ret;
}


int CProcMil_LeadSolder::InspSolderArea4(float* zmapData, UCHAR* imgT, UCHAR* imgM, UCHAR* imgB, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int stdSolderArea, int isChipsolder, int solderInspLength, int solderWidthMargin, bool mode, int* retFiletStart, int* retSolderArea, int* retBlobCount)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	float threshold_Low = (float)(avrLeadH * 0.8);
	float threshold_High = (float)(avrLeadH * 1.2);

	Im::PIL_ID milZmapSrc = M_NULL;
	float* clipZmap = NULL;
	UCHAR* srcImg_Top = NULL;
	UCHAR* srcImg_Mid = NULL;
	UCHAR* srcImg_Bot = NULL;
	int solderArea = 0;

	float* zmapDataTemp = NULL;
	//zmapDataTemp = new float[roiSizeX * roiSizeY];
	zmapDataTemp = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	ippsThreshold_GTVal_32f(zmapData, zmapDataTemp, roiSizeX * roiSizeY, threshold_Low, 0);			//상한 제거
	ippsThreshold_LTVal_32f(zmapDataTemp, zmapDataTemp, roiSizeX * roiSizeY, 0, 0);

	float tempThresh = 0;
	float hMax = 0;
	float hMin = 0;
	float avr = GetZmapDataAvr(zmapData, roiSizeX, roiSizeY);

	GetHlimit(zmapData, roiSizeX, roiSizeY, &hMax, &hMin);
	float val = 0;
	val = GetZmapDataAvr(zmapData, roiSizeX, roiSizeY, hMin, avr);


	tempThresh = (float)(val * 1);
	if(isChipsolder)
	{
		float tempThresh = 0;
		float hMax = 0;
		float hMin = 0;
		float avr = GetZmapDataAvr(zmapData, roiSizeX, roiSizeY);

		GetHlimit(zmapData, roiSizeX, roiSizeY, &hMax, &hMin);
		float val = 0;
		val = GetZmapDataAvr(zmapData, roiSizeX, roiSizeY, hMin, avr);

		tempThresh = (float)(val * 1);

		if(tempThresh < 20)
			tempThresh = 20;

		ippsThreshold_LTVal_32f(zmapDataTemp, zmapDataTemp, roiSizeX * roiSizeY, tempThresh, 0);      //하한 제거
	}
	else
	{
		milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
		Im::Buf::Put(milZmapSrc, zmapDataTemp);	
	}
	


	//////////////////////////////////////////////////////////////////////////

	int cx = 0;
	int cy = 0;
	int margin = 10;
	
	int temp = 0;
	double totalSolderArea = 0;
	int avrSolderArea = 0;
	int maxArea = 0;
	int minArea = roiSizeX * roiSizeY;
	int remakeCount = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		int solderW = solderRect[i].Width();
		int solderH = solderRect[i].Height();
		solderArea = solderW * solderH;

		//clip zmap
		if(isChipsolder)
		{
			clipZmap = zmapDataTemp;
			srcImg_Top = imgT;
			srcImg_Mid = imgM;
			srcImg_Bot = imgB;
		}
		else
		{		
			//solderW = solderRect[i].Width() + (solderWidthMargin * 2);
			solderW = mode == true?   roiSizeX : solderRect[i].Width() + (solderWidthMargin * 2);  //mode=> false : teach mode   //   true: inspection mode
			solderH = solderRect[i].Height() + margin;
			solderArea = solderW * solderH;

			cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));
			cy = (int)((solderRect[i].top + (solderRect[i].Height() / 2.0)) - (margin / 2.0));

			temp = (int)(cy - (solderH / 2.0));
			if(temp < 0)
			{
				cy += abs(temp);
				solderH -= abs(temp);
				margin -= abs(temp);
			}

			//3d
			//clipZmap = new float[solderArea];
			clipZmap = g_pMManager->pem_new<float>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
			Im::PIL_ID milClipTemp = M_NULL;		
			milClipTemp = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH);			
			Im::Buf::Get(milClipTemp, clipZmap);
			FreeMilImageBuff(&milClipTemp);


			//2d
			Im::PIL_ID milImgTemp = AllocBuff(roiSizeX, roiSizeY);
			
			Im::Buf::Put(milImgTemp, imgT);
			milClipTemp = AllocClipBuff(milImgTemp, cx, cy, solderW, solderH);
			//srcImg_Top = new UCHAR[solderArea];
			srcImg_Top = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
			Im::Buf::Get(milClipTemp, srcImg_Top);
			FreeMilImageBuff(&milClipTemp);

			Im::Buf::Put(milImgTemp, imgM);
			milClipTemp = AllocClipBuff(milImgTemp, cx, cy, solderW, solderH);
			//srcImg_Mid = new UCHAR[solderArea];
			srcImg_Mid = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
			Im::Buf::Get(milClipTemp, srcImg_Mid);
			FreeMilImageBuff(&milClipTemp);

			Im::Buf::Put(milImgTemp, imgB);
			milClipTemp = AllocClipBuff(milImgTemp, cx, cy, solderW, solderH);
			//srcImg_Bot = new UCHAR[solderArea];
			srcImg_Bot = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
			Im::Buf::Get(milClipTemp, srcImg_Bot);
			FreeMilImageBuff(&milClipTemp);

			FreeMilImageBuff(&milImgTemp);

			//////////////////////////////////////////////////////////////////////////
		}


		//////////////////////////////////////////////////////////////////////////

		//shk 2014/02/21
		//thresholdL_st 이하값만 projection 
		int rectSize = solderH ;
		//float* Startprofile = new float[rectSize];
		float* Startprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Startprofile, 0, sizeof(float) * (rectSize));
		int zindex = 0;

		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				zindex = (y * solderW) + x;

				if(clipZmap[zindex] <= threshold_Low)
				{
					Startprofile[y] += clipZmap[zindex];
				} 
			}
		}

		float tempMaxVal = 0.f;
		int MaxInd = 0;

		ippsMaxIndx_32f(Startprofile,rectSize,&tempMaxVal,&MaxInd);

		int firstPoint = MaxInd;

		//delete Startprofile;
		g_pMManager->pem_delete(Startprofile, false);

	
		if(retFiletStart != NULL)
		{
			retFiletStart[i] = firstPoint - (int)(margin / 2.0);
		}
		//////////////////////////////////////////////////////////////////////////

		//huj 2014/02/24
		if(!isChipsolder)
		{
			float tempThresh = 0;
			float hMax = 0;
			float hMin = 0;
			float avr = GetZmapDataAvr(clipZmap, solderW, solderH);
			ippsThreshold_LTVal_32f(clipZmap, clipZmap, solderW * solderH, avr, 0); 
		}
		//////////////////////////////////////////////////////////////////////////
		//solder data count
		int dataCount = 0;
		UCHAR* grayImg = NULL;
		//grayImg = new UCHAR[solderArea];
		grayImg = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);

		UCHAR* binImg = NULL;
		//binImg = new UCHAR[solderArea];
		binImg = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
		
		float cntThreshold = 10;
		if(!isChipsolder)
			cntThreshold = 0;

		dataCount = MakeRoughGrayImg(clipZmap, grayImg, binImg, solderW, solderH, cntThreshold);
		SaveWorkImg(binImg, solderW, solderH, _T("solder\\test.bmp"));


		//////////////////////////////////////////////////////////////////////////
		if(!isChipsolder)
		{
			Im::PIL_ID milBin = AllocBuff(solderW, solderH);
			Im::Buf::Put(milBin, binImg);
			MorClose(milBin, milBin, 2);
			Im::Buf::Get(milBin, binImg);
			FreeMilImageBuff(&milBin);

			dataCount = 0;
			int index = 0;
			UCHAR data = 0;
			for(int y = 0; y < solderH; y++)
			{
				for(int x = 0; x < solderW; x++)
				{
					index = (y * solderW) + x;
					data = binImg[index];
					if(data > 0)
						dataCount++;	
				}
			}
		}
	/*	else
		{
			//test		
			int limit = firstPoint-3;
			if(limit < 0)
				limit = 0;

			int countTemp = 0;
			int index = 0;
			UCHAR data = 0;
			for(int y = 0; y < firstPoint-3; y++)
			{
				for(int x = 0; x < solderW; x++)
				{
					index = (y * solderW) + x;
					data = binImg[index];
					if(binImg[index] > 0)
						countTemp++;


					binImg[index] = 0;				
				}
			}

			dataCount -= countTemp;

			SaveWorkImg(binImg, solderW, solderH, _T("solder\\test2.bmp"));
		}*/
		//////////////////////////////////////////////////////////////////////////

		if(!mode)
		{
			dataCount = 0;

			int index = 0;
			UCHAR data = 0;
		

			int length = solderInspLength;
			if(!isChipsolder)
				length = solderInspLength - solderRect[i].top;

			if(length > solderH)
				length = solderH;

			for(int y = 0; y < solderH; y++)
			{
				for(int x = 0; x < solderW; x++)
				{
					index = (y * solderW) + x;
					if(y < length)
					{
						data = binImg[index];
						if(data > 0)
						{
							dataCount++;
						}
					}
					else
					{
						binImg[index] = 0;
					}
				}
			}
		}
	
	

		CString strName = _T("");
		strName.Format(_T("solder\\solderGrayImg_%d.bmp"),i);
		SaveWorkImg(grayImg, solderW, solderH, strName);

		strName.Format(_T("solder\\solderBinImg_%d.bmp"),i);
		SaveWorkImg(binImg, solderW, solderH, strName);	

		//////////////////////////////////////////////////////////////////////////
		//solder data blob
		UCHAR* blobImg = NULL;
		//blobImg = new UCHAR[solderArea];
		blobImg = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
	
		int stdMinBlobArea = 100;  //solderArea * 0.03  (3%)
		if(!isChipsolder)
			stdMinBlobArea = 20;	//solderArea * 0.03  (3%)

		int blobCnt = CalcBlob(binImg, solderW, solderH, stdMinBlobArea, 0, FALSE, FALSE, blobImg);
		
		if(blobCnt > 1)
		{		
			CPoint wndCenter(0 ,0);
			wndCenter.SetPoint((int)(solderW / 2.0) ,(int)(solderH / 2.0));

			int imgThreshold_T = 0;
			int imgThreshold_M = 0;
			int imgThreshold_B = 0;

			//UCHAR* tempImg = new UCHAR[solderArea];
			UCHAR* tempImg = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
			MaskTrueImg(srcImg_Top, binImg, tempImg, solderW, solderH);
			imgThreshold_T = GetOtsuThreshold(tempImg, solderW, solderH);

			MaskTrueImg(srcImg_Mid, binImg, tempImg, solderW, solderH);
			imgThreshold_M = GetOtsuThreshold(tempImg, solderW, solderH);

			MaskTrueImg(srcImg_Bot, binImg, tempImg, solderW, solderH);
			imgThreshold_B = GetOtsuThreshold(tempImg, solderW, solderH);
			//delete tempImg;
			g_pMManager->pem_delete(tempImg, false);
		

			CMilBlobResult *blobRst = new CMilBlobResult(blobCnt);
			g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

			GetBlobResult(blobRst);

			//bool* labelOk = new bool[blobCnt];
			bool* labelOk = g_pMManager->pem_new<bool>(true, blobCnt, (PCHAR)__FUNCTION__, __LINE__);
			memset(labelOk, 0, sizeof(bool) * blobCnt);

			Im::PIL_ID remakeImg = AllocBuff(solderW, solderH, 0);
			UCHAR* trueImg_T = NULL;
			UCHAR* trueImg_M = NULL;
			UCHAR* trueImg_B = NULL;
			for(int j = 0; j < blobCnt; j++)
			{
				Im::PIL_ID milEachBlobImg = AllocBuff(solderW, solderH, 0);
				Im::Gra::Color(M_DEFAULT, M_COLOR_WHITE);
				DrawBlob(m_milBlobResult, milEachBlobImg, blobRst->blobLabel[j], FALSE);

				UCHAR* maskImg = NULL;
				//maskImg = new UCHAR[solderArea];
				maskImg = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
				Im::Buf::Get(milEachBlobImg, maskImg);

				//trueImg_T = new UCHAR[solderArea];
				trueImg_T = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
				MaskTrueImg(srcImg_Top, maskImg, trueImg_T, solderW, solderH);

				//trueImg_M = new UCHAR[solderArea];
				trueImg_M = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
				MaskTrueImg(srcImg_Mid, maskImg, trueImg_M, solderW, solderH);

				//trueImg_B = new UCHAR[solderArea];
				trueImg_B = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
				MaskTrueImg(srcImg_Bot, maskImg, trueImg_B, solderW, solderH);


				strName.Format(_T("solder\\solderTrueImgT_%d_%d.bmp"), i, j);
				SaveWorkImg(trueImg_T, solderW, solderH, strName);
				strName.Format(_T("solder\\solderTrueImgM_%d_%d.bmp"), i, j);
				SaveWorkImg(trueImg_M, solderW, solderH, strName);
				strName.Format(_T("solder\\solderTrueImgB_%d_%d.bmp"), i, j);
				SaveWorkImg(trueImg_B, solderW, solderH, strName);

				int topCount = 0;
				int midCount = 0;
				int botCount = 0;

				int index = 0;
				UCHAR data_T = 0;
				UCHAR data_M = 0;
				UCHAR data_B = 0;

				bool isSizeRatioOk = true;
				bool isBlobOk = true;
				for(int y = 0; y < solderH; y++)
				{
					for(int x = 0; x < solderW; x++)
					{
						index = (y * solderW) + x;

						data_T = trueImg_T[index];
						data_M = trueImg_M[index];
						data_B = trueImg_B[index];

						if(/*data_T > imgThreshold_T || data_M > imgThreshold_M || */data_B > imgThreshold_B)
							topCount++;
					}
				}

				//1
				int stdBlobArea = (int)(blobRst->area[j] * 0.2);
				if(blobRst->top[j] >= 5)
				{
					if(topCount <= stdBlobArea)
						isBlobOk = false;
				}
						
				//2
				CRect tempBlobRect(0, 0, 0, 0);
				tempBlobRect.left = (LONG)blobRst->left[j];
				tempBlobRect.right = (LONG)blobRst->right[j];
				tempBlobRect.top = (LONG)blobRst->top[j];
				tempBlobRect.bottom = (LONG)blobRst->bottom[j];

				if(wndCenter.x < tempBlobRect.left || wndCenter.x > tempBlobRect.right)
					isBlobOk = false;
		

				isBlobOk = true;  //huj 2014/02/24
				if(isBlobOk)
				{
					ArithImage(remakeImg, milEachBlobImg, remakeImg, M_ADD);
					remakeCount++;	
					labelOk[j] = true;
				}
				else
				{
					dataCount -= (int)blobRst->area[j];
					labelOk[j] = false;
				}
				//////////////////////////////////////////////////////////////////////////
				/*delete trueImg_T;
				delete trueImg_M;
				delete trueImg_B;
				delete maskImg;*/
				g_pMManager->pem_delete(trueImg_T, false);
				g_pMManager->pem_delete(trueImg_M, false);
				g_pMManager->pem_delete(trueImg_B, false);
				g_pMManager->pem_delete(maskImg, false);
				FreeMilImageBuff(&milEachBlobImg);
						
			}

			int stdBolbArea = (int)(dataCount * 0.4);
			if(stdBolbArea > stdMinBlobArea)
			{
				for(int j = 0; j < blobCnt; j++)
				{
					if(labelOk[j])
					{
						if(blobRst->area[j] < stdBolbArea)	
							remakeCount--;
					}
				}
			}
			strName.Format(_T("solder\\remakeSolder_%d.bmp"), i);
			SaveWorkImg(remakeImg, strName);


			/*delete labelOk;
			delete blobRst;*/
			g_pMManager->pem_delete(labelOk, false);
			g_pMManager->pem_delete(blobRst, false);
			FreeMilImageBuff(&remakeImg);		
		}
		else
		{
			remakeCount = blobCnt;
		}


		/*delete binImg;
		delete blobImg;*/
		g_pMManager->pem_delete(binImg, false);
		g_pMManager->pem_delete(blobImg, false);
		
		//////////////////////////////////////////////////////////////////////////

		//delete grayImg;	
		g_pMManager->pem_delete(grayImg, false);
		if(!isChipsolder)
		{
			if(clipZmap != NULL)
			{
				//delete clipZmap;
				g_pMManager->pem_delete(clipZmap, false);
				clipZmap = NULL;
			}	

			if(srcImg_Top != NULL)
			{
				//delete srcImg_Top;
				g_pMManager->pem_delete(srcImg_Top, false);
				srcImg_Top = NULL;
			}	

			if(srcImg_Mid != NULL)
			{
				//delete srcImg_Mid;
				g_pMManager->pem_delete(srcImg_Mid, false);
				srcImg_Mid = NULL;
			}	

			if(srcImg_Bot != NULL)
			{
				//delete srcImg_Bot;
				g_pMManager->pem_delete(srcImg_Bot, false);
				srcImg_Bot = NULL;
			}	
		}	

		//////////////////////////////////////////////////////////////////////////
		totalSolderArea += dataCount;

		if(maxArea < dataCount)
			maxArea = dataCount;

		if(minArea > dataCount)
			minArea = dataCount;
	}

	//////////////////////////////////////////////////////////////////////////
	if(solderCnt >= 3)
	{
		totalSolderArea = totalSolderArea - maxArea - minArea;
		solderCnt = solderCnt - 2;
	}

	avrSolderArea = (int)(totalSolderArea / solderCnt);



	if(retSolderArea != NULL)
		*retSolderArea = avrSolderArea;

	if(retBlobCount != NULL)
		*retBlobCount = remakeCount;


	if(!isChipsolder)
	{
		FreeMilImageBuff(&milZmapSrc);

		if(clipZmap != NULL)
			//delete clipZmap;
			g_pMManager->pem_delete(clipZmap, false);
	}

	if(zmapDataTemp != NULL)
	{
		//delete zmapDataTemp;
		g_pMManager->pem_delete(zmapDataTemp, false);
		zmapDataTemp = NULL;
	}


	return ret;
}



int CProcMil_LeadSolder::InspSolderArea_S1(float* zmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	float threshold_Low = (float)(avrLeadH * 0.8);

	float* zmapDataTemp = NULL;
	//zmapDataTemp = new float[roiSizeX * roiSizeY];
	zmapDataTemp = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	int nthresLow = 20;

	if(!isChipsolder)
	{
		nthresLow = 10;
	}

	ippsThreshold_LTVal_32f(zmapData, zmapDataTemp, roiSizeX * roiSizeY, (Ipp32f)nthresLow,  (Ipp32f)0);

	Im::PIL_ID milZmapSrc = M_NULL;
	Im::PIL_ID milClipZmap = M_NULL;
	float* clipZmap = NULL;

	if(!isChipsolder)
	{
		milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
		Im::Buf::Put(milZmapSrc, zmapDataTemp);
	}

	int cx = 0;
	int cy = 0;
	int margin = 0;
	int solderW = 0;
	int solderH = 0;
	int temp = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		solderW = solderRect[i].Width();
		solderH = solderRect[i].Height();

		if(isChipsolder)
		{
			margin = 0;
			clipZmap = zmapDataTemp;
		}
		else
		{		
			margin = 10;
			solderW = solderRect[i].Width();
			solderH = solderRect[i].Height() + margin;

			cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));
			cy = (int)((solderRect[i].top + (solderRect[i].Height() / 2.0)) - (margin / 2.0));

			temp = (int)(cy - (solderH / 2.0));
			if(temp < 0)
			{
				cy += abs(temp);
				solderH -= abs(temp);
			}

			milClipZmap = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH);
			//clipZmap = new float[solderW * solderH];
			clipZmap = g_pMManager->pem_new<float>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);

			Im::Buf::Get(milClipZmap, clipZmap);

			FreeMilImageBuff(&milClipZmap);
			FreeMilImageBuff(&milZmapSrc);
		}

		int rectSize = solderH ;
		//float* Startprofile = new float[rectSize];
		float* Startprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Startprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile = new float[rectSize];
		float* Tempprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile2 = new float[rectSize];
		float* Tempprofile2 = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile2, 0, sizeof(float) * (rectSize));

		int zindex = 0;

		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				zindex = (y * solderW) + x;

				if(clipZmap[zindex] <= threshold_Low)
				{
					Startprofile[y] += clipZmap[zindex];
				} 
				else
				{
					Tempprofile[y] += clipZmap[zindex];
				}
			}
		}

		float tempMaxVal = 0.f;
		float tempMinVal = 0.f;
		int MaxInd = 0;
		int MinInd = 0;
		int solderSt = 0;
		int LeadEd = 0;



		ippsSub_32f(Tempprofile,Startprofile,Tempprofile2,rectSize);
		ippsMinIndx_32f(Tempprofile2,rectSize,&tempMinVal,&MinInd);
		ippsMaxIndx_32f(Startprofile+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);

		MaxInd = MaxInd + MinInd ;
		solderSt = MaxInd + 1;
		LeadEd = MaxInd - 4;

		//filet 계산
		//1. solderSt 부터 차이값 계산
		ippsSub_32f(Startprofile+MaxInd+1,Startprofile+MaxInd,Tempprofile2,rectSize-MaxInd-1);

		//2. 차이값  -1, +1로 치환		//3 . filet count 계산

		float filetCnt = 0.f;
		float filetCnt1 = 0.f;
		float filetCnt2 = 0.f;

		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-MaxInd-1, -2, -1, -2, 0); 
		ippsSum_32f(Tempprofile,rectSize-MaxInd-1,&filetCnt1,ippAlgHintNone);
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-MaxInd-1, 0, 0, 0, 1);
		ippsSum_32f(Tempprofile,rectSize-MaxInd-1,&filetCnt2,ippAlgHintNone);

		filetCnt = filetCnt1 + filetCnt2;
		//ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-MaxInd-1, 0, -1, 0, 1);
		//ippsSum_32f(Tempprofile,rectSize-MaxInd-1,&filetCnt,ippAlgHintNone);

		//4. filet ratio 계산
		float fratio = 0.f;
		if (filetCnt <= 0)
		{
			fratio = 0.f;
		}
		else
		{
			fratio =  (filetCnt / solderInspLength ) * 100;
		}

		/*delete Startprofile;
		delete Tempprofile;
		delete Tempprofile2;*/
		g_pMManager->pem_delete(Startprofile, false);
		g_pMManager->pem_delete(Tempprofile, false);
		g_pMManager->pem_delete(Tempprofile2, false);

		if(!isChipsolder)
		{
			//delete clipZmap;
			g_pMManager->pem_delete(clipZmap, false);
		}
		if(retSolderSt != NULL)
		{
			retSolderSt[i] = solderSt - margin;
		}
		if(retLeadEd != NULL)
		{
			retLeadEd[i] = LeadEd - margin;
		}
		if(retfiletRatio != NULL)
		{
			retfiletRatio[i] = fratio;
		}
	}

	//delete zmapDataTemp;
	g_pMManager->pem_delete(zmapDataTemp, false);

	return ret;
}


int CProcMil_LeadSolder::InspSolderArea_S2(float* zmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	float threshold_Low = (float)(avrLeadH * 0.8);

	float* zmapDataTemp = NULL;
	//zmapDataTemp = new float[roiSizeX * roiSizeY];
	zmapDataTemp = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	int nthresLow = 20;

	if(!isChipsolder)
	{
		nthresLow = 10;
	}

	ippsThreshold_LTVal_32f(zmapData, zmapDataTemp, roiSizeX * roiSizeY, (Ipp32f)nthresLow,  (Ipp32f)0);

	Im::PIL_ID milZmapSrc = M_NULL;
	Im::PIL_ID milClipZmap = M_NULL;
	float* clipZmap = NULL;

	if(!isChipsolder)
	{
		milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
		Im::Buf::Put(milZmapSrc, zmapDataTemp);
	}

	int cx = 0;
	int cy = 0;
	int margin = 0;
	int solderW = 0;
	int solderH = 0;
	int temp = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		solderW = solderRect[i].Width();
		solderH = solderRect[i].Height();

		if(isChipsolder)
		{
			margin = 0;
			clipZmap = zmapDataTemp;
		}
		else
		{		
			margin = 20;
			solderW = solderRect[i].Width();
			solderH = solderRect[i].Height() + margin;

			cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));
			cy = (int)((solderRect[i].top + (solderRect[i].Height() / 2.0)) - (margin / 2.0));

			temp = (int)(cy - (solderH / 2.0));
			if(temp < 0)
			{
				cy += abs(temp);
				solderH -= abs(temp);
			}

			milClipZmap = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH);
			//clipZmap = new float[solderW * solderH];
			clipZmap = g_pMManager->pem_new<float>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);

			Im::Buf::Get(milClipZmap, clipZmap);

			FreeMilImageBuff(&milClipZmap);

		}

		int rectSize = solderH ;
		//float* Startprofile = new float[rectSize];
		float* Startprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Startprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile = new float[rectSize];
		float* Tempprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile2 = new float[rectSize];
		float* Tempprofile2 = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile2, 0, sizeof(float) * (rectSize));

		//float* Total = new float[rectSize];
		float* Total = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Total, 0, sizeof(float) * (rectSize));

		int zindex = 0;

		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				zindex = (y * solderW) + x;

				if(clipZmap[zindex] <= threshold_Low)
				{
					Startprofile[y] += clipZmap[zindex];
				} 
				else
				{
					Tempprofile[y] += clipZmap[zindex];
				}

				Total[y] += clipZmap[zindex];
			}
		}

		float tempMaxVal = 0.f;
		float tempMinVal = 0.f;
		int MaxInd = 0;
		int MinInd = 0;
		int solderSt = 0;
		int LeadEd = 0;



		ippsSub_32f(Tempprofile,Startprofile,Tempprofile2,rectSize);
		//ippsMinIndx_32f(Tempprofile2,rectSize,&tempMinVal,&MinInd);
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile2, rectSize, 0, 0, 0, 1); 

		// 		if (isChipsolder)
		// 		{
		// 			ippsMaxIndx_32f(Tempprofile2+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);
		// 		}
		// 		else
		// 			//ippsMaxIndx_32f(Startprofile+MinInd,(int)(1.5*margin)-MinInd,&tempMaxVal,&MaxInd);
		ippsMaxIndx_32f(Tempprofile2+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);


		MaxInd = MaxInd + MinInd ;
		solderSt = MaxInd -3;//권팀장이 책임진다고 함...// + 1; 
		LeadEd = MaxInd - 4;
		if (solderSt < 0 )
		{
			solderSt = 0;
		}
		if (LeadEd < 0 )
		{
			LeadEd = 0;
		}

		//fillet 계산
		if (rectSize > solderSt+solderInspLength)  // MaxInd -> solderSt, Startprofile -> Total
		{
			ippsZero_32f(Total+solderSt+solderInspLength,rectSize-solderSt-solderInspLength);
		}

		//1. solderSt 부터 차이값 계산
		ippsSub_32f(Total+solderSt+1,Total+solderSt,Tempprofile2,rectSize-solderSt-1);

		//2. 차이값  -1, +1로 치환		//3 . filet count 계산

		float filetCnt = 0.f;
		float filetCnt1 = 0.f;
		float filetCnt2 = 0.f;

		// MaxInd -> solderSt
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1, -2, -1, -2, 0); 
		ippsSum_32f(Tempprofile,rectSize-solderSt-1,&filetCnt1,ippAlgHintNone);
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1, 0, 0, 0, 1);
		ippsSum_32f(Tempprofile,rectSize-solderSt-1,&filetCnt2,ippAlgHintNone);

		filetCnt = filetCnt1 + filetCnt2;
		//ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-MaxInd-1, 0, -1, 0, 1);
		//ippsSum_32f(Tempprofile,rectSize-MaxInd-1,&filetCnt,ippAlgHintNone);

		//4. fillet ratio 계산
		float fratio = 0.f;
		if (filetCnt <= 0)
		{
			fratio = 0.f;
		}
		else
		{
			fratio =  (filetCnt / solderInspLength ) * 100;
		}

		/*delete Startprofile;
		delete Tempprofile;
		delete Tempprofile2;

		delete Total;*/
		g_pMManager->pem_delete(Startprofile, false);
		g_pMManager->pem_delete(Tempprofile, false);
		g_pMManager->pem_delete(Tempprofile2, false);
		g_pMManager->pem_delete(Total, false);

		if(!isChipsolder)
		{
			//delete clipZmap;
			g_pMManager->pem_delete(clipZmap, false);
		}
		if(retSolderSt != NULL)
		{
			retSolderSt[i] = solderSt - margin;
		}
		if(retLeadEd != NULL)
		{
			retLeadEd[i] = LeadEd -margin;
		}
		if(retfiletRatio != NULL)
		{
			retfiletRatio[i] = fratio;
		}
	}
	if(!isChipsolder)
	{
		FreeMilImageBuff(&milZmapSrc);
	}

	//delete zmapDataTemp;
	g_pMManager->pem_delete(zmapDataTemp, false);

	return ret;
}


int CProcMil_LeadSolder::InspSolderArea_S3(float* zmapData, UCHAR* imgT,int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio,float* retGrayfRatio)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	UCHAR* srcImg_Top = NULL;

	float threshold_Low = (float)(avrLeadH * 0.8);

	float* zmapDataTemp = NULL;
	//zmapDataTemp = new float[roiSizeX * roiSizeY];
	zmapDataTemp = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	int nthresLow = 20;

	if(!isChipsolder)
	{
		nthresLow = 10;
	}

	ippsThreshold_LTVal_32f(zmapData, zmapDataTemp, roiSizeX * roiSizeY, (Ipp32f)nthresLow,  (Ipp32f)0);

	Im::PIL_ID milZmapSrc = M_NULL;
	Im::PIL_ID milClipZmap = M_NULL;
	float* clipZmap = NULL;

	if(!isChipsolder)
	{
		milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
		Im::Buf::Put(milZmapSrc, zmapDataTemp);
	}

	int cx = 0;
	int cy = 0;
	int margin = 0;
	int solderW = 0;
	int solderH = 0;
	int temp = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		solderW = solderRect[i].Width();
		solderH = solderRect[i].Height();

		if(isChipsolder)
		{
			margin = 0;
			clipZmap = zmapDataTemp;
			srcImg_Top = imgT;
		}
		else
		{		
			margin = 20;
			solderW = solderRect[i].Width();
			solderH = solderRect[i].Height() + margin;

			cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));
			cy = (int)((solderRect[i].top + (solderRect[i].Height() / 2.0)) - (margin / 2.0));

			temp = (int)(cy - (solderH / 2.0));
			if(temp < 0)
			{
				cy += abs(temp);
				solderH -= abs(temp);
			}

			milClipZmap = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH);
			//clipZmap = new float[solderW * solderH];
			clipZmap = g_pMManager->pem_new<float>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);

			Im::Buf::Get(milClipZmap, clipZmap);

			FreeMilImageBuff(&milClipZmap);

			//2d 
			Im::PIL_ID milImgTemp = AllocBuff(roiSizeX, roiSizeY);
			Im::PIL_ID milClipTemp = M_NULL;		
			int  solderArea = solderW* solderH;

			Im::Buf::Put(milImgTemp, imgT);
			milClipTemp = AllocClipBuff(milImgTemp, cx, cy, solderW, solderH);
			//srcImg_Top = new UCHAR[solderArea];
			srcImg_Top = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
			Im::Buf::Get(milClipTemp, srcImg_Top);

			SaveWorkImg(milImgTemp,_T("milImgTemp.bmp"));
			SaveWorkImg(milClipTemp,_T("milClipTemp.bmp"));
			FreeMilImageBuff(&milClipTemp);
			FreeMilImageBuff(&milImgTemp);
		}

		int rectSize = solderH ;
		//float* Startprofile = new float[rectSize];			//teaching height 80% under profile
		float* Startprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Startprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile = new float[rectSize];			//teaching  height 80% over profile
		float* Tempprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile2 = new float[rectSize];         // Tempprofile2 = Tempprofile - Tempprofile
		float* Tempprofile2 = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile2, 0, sizeof(float) * (rectSize));

		//float* Total = new float[rectSize];
		float* Total = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Total, 0, sizeof(float) * (rectSize));

		//short* ucenProfile = new short[solderH];			//2d profile
		short* ucenProfile = g_pMManager->pem_new<short>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(ucenProfile, 0, sizeof(short) * (solderH));

		int zindex = 0;

		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				zindex = (y * solderW) + x;

				if(clipZmap[zindex] <= threshold_Low)
				{
					Startprofile[y] += clipZmap[zindex];
				} 
				else
				{
					Tempprofile[y] += clipZmap[zindex];
				}

				Total[y] += clipZmap[zindex];

				//2d 계산
				if (x == (int)solderW/2)
				{
					ucenProfile[y] = srcImg_Top[zindex];
				}
			}	
		}

		float tempMaxVal = 0.f;
		float tempMinVal = 0.f;
		int MaxInd = 0;
		int MinInd = 0;
		int solderSt = 0;
		int LeadEd = 0;

		ippsSub_32f(Tempprofile,Startprofile,Tempprofile2,rectSize);
		ippsMinIndx_32f(Tempprofile2,rectSize,&tempMinVal,&MinInd);
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile2, rectSize, 0, 0, 0, 1); 

		// 		if (isChipsolder)
		// 		{
		// 			ippsMaxIndx_32f(Tempprofile2+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);
		// 		}
		// 		else
		// 			//ippsMaxIndx_32f(Startprofile+MinInd,(int)(1.5*margin)-MinInd,&tempMaxVal,&MaxInd);
		ippsMaxIndx_32f(Tempprofile2+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);

		

		MaxInd = MaxInd + MinInd ;
		solderSt = MaxInd -3;//권팀장이 책임진다고 함...// + 1; 
		LeadEd = MaxInd - 4;//20140318 권팀장이 전적인 책임을 진다고 함. -4 ==> -7
		if (solderSt < 0 )
		{
			solderSt = 0;
		}
		if (LeadEd < 0 )
		{
			LeadEd = 0;
		}


		//fillet 계산
		if (rectSize > solderSt+solderInspLength)  // MaxInd -> solderSt, Startprofile -> Total
		{
			ippsZero_32f(Total+solderSt+solderInspLength,rectSize-solderSt-solderInspLength);
		}

		//2d계산 ---> start기준선 위쪽으로 5pixel 포함
		int graySolderSt = 0;
		graySolderSt = solderSt;

		if (graySolderSt < 5)
		{
			graySolderSt = 5;
		}

		//shk 20140320 grayscale threshold 기준 ==> solder start에서 -5 지점에서 solder start까지의 평균값*0.9 이하를 threshold
		short pThreshold = 0;
		ippsMean_16s_Sfs(ucenProfile+graySolderSt-5,5,&pThreshold,0);
		pThreshold = (short)(pThreshold*0.9);	

		ippsThreshold_LTValGTVal_16s(ucenProfile+graySolderSt-5,ucenProfile+graySolderSt-5,rectSize-graySolderSt+5,pThreshold,1,pThreshold,0);// 50 ---> pThreshold
	
		if (rectSize > solderSt+solderInspLength)  // MaxInd -> solderSt, Startprofile -> Total
		{
			//2d 계산
			ippsZero_16s(ucenProfile+solderSt+solderInspLength,rectSize-solderSt-solderInspLength);
		}

		short TempgrayCnt = 0;
		ippsSum_16s_Sfs(ucenProfile+graySolderSt-5,rectSize-graySolderSt+5,&TempgrayCnt,ippAlgHintNone);

		int grayCnt = 0;
		grayCnt = (int)TempgrayCnt;

		//////////////////////////////////////////////////////////////////////////
		
		//1. solderSt 부터 차이값 계산
		ippsDivC_32f(Total, (Ipp32f)solderW, Total, rectSize);
		ippsSub_32f(Total + solderSt + 1, Total+solderSt, Tempprofile2, rectSize - solderSt - 1);

		//2. 차이값  -1, +1로 치환		//3 . filet count 계산

		float filetCnt = 0.f;
		float filetCnt1 = 0.f;
		float filetCnt2 = 0.f;

		// MaxInd -> solderSt
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1, -1, -1, -1, 0); //20130318 권팀장이  전반적으로 책임진다고함 -2 ===> -1
		ippsSum_32f(Tempprofile,rectSize-solderSt-1,&filetCnt1,ippAlgHintNone);
		
		//ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1, 1, 0, 1, 1);//
		//SHK 20140416
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1,(Ipp32f)LEAD_SOLDER_DOWNSTEP, 0, (Ipp32f)LEAD_SOLDER_DOWNSTEP, 1);

		ippsSum_32f(Tempprofile,rectSize-solderSt-1,&filetCnt2,ippAlgHintNone);

		filetCnt = filetCnt1 + filetCnt2;
		//ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-MaxInd-1, 0, -1, 0, 1);
		//ippsSum_32f(Tempprofile,rectSize-MaxInd-1,&filetCnt,ippAlgHintNone);

		//4. fillet ratio 계산
		float fratio = 0.f;
		if (filetCnt <= 0)
		{
			fratio = 0.f;
		}
		else
		{
			fratio =  (filetCnt / solderInspLength ) * 100;
		}

		//2d 계산
		float grayfratio = 0.f;
		if (grayCnt <= 0)
		{
			grayfratio = 0.f;
		}
		else
		{
			if (grayCnt >= solderInspLength )
			{
				grayfratio = 100.f;
			}
			else
			{
				grayfratio =  ((float)grayCnt / solderInspLength ) * 100;
			}
			
		}

		/*delete Startprofile;
		delete Tempprofile;
		delete Tempprofile2;

		delete Total;
		delete ucenProfile;*/
		g_pMManager->pem_delete(Startprofile, false);
		g_pMManager->pem_delete(Tempprofile, false);
		g_pMManager->pem_delete(Tempprofile2, false);
		g_pMManager->pem_delete(Total, false);
		g_pMManager->pem_delete(ucenProfile, false);

		if(!isChipsolder)
		{
			/*delete srcImg_Top;
			delete clipZmap;*/
			g_pMManager->pem_delete(srcImg_Top, false);
			g_pMManager->pem_delete(clipZmap, false);
		}
		if(retSolderSt != NULL)
		{
			retSolderSt[i] = solderSt - margin;
		}
		if(retLeadEd != NULL)
		{
			retLeadEd[i] = LeadEd -margin;
		}
		if(retfiletRatio != NULL)
		{
			retfiletRatio[i] = fratio;
		}
		if(retGrayfRatio != NULL)
		{
			retGrayfRatio[i] = grayfratio;
		}
		

	}
	if(!isChipsolder)
	{
		FreeMilImageBuff(&milZmapSrc);
	}

	//delete zmapDataTemp;
	g_pMManager->pem_delete(zmapDataTemp, false);

	return ret;
}

//shk 20140424 
int CProcMil_LeadSolder::InspSolderArea_S4(float* zmapData, UCHAR* imgT,int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int isUse2D, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio,float* retGrayfRatio)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	//

	UCHAR* srcImg_Top = NULL;

	float threshold_Low = (float)(avrLeadH * 0.8);
	float threshold_High = (float)(avrLeadH * 0.9);

	float* zmapDataTemp = NULL;
	//zmapDataTemp = new float[roiSizeX * roiSizeY];
	zmapDataTemp = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	int nthresLow = 20;

	if(!isChipsolder)
	{
		nthresLow = 10;
	}

	ippsThreshold_LTVal_32f(zmapData, zmapDataTemp, roiSizeX * roiSizeY, (Ipp32f)nthresLow,  (Ipp32f)0);

	Im::PIL_ID milZmapSrc = M_NULL;
	Im::PIL_ID milClipZmap = M_NULL;
	float* clipZmap = NULL;

	if(!isChipsolder)
	{
		milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
		Im::Buf::Put(milZmapSrc, zmapDataTemp);
	}

	int cx = 0;
	int cy = 0;
	int margin = 0;
	int solderW = 0;
	int solderH = 0;
	int temp = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		solderW = solderRect[i].Width();
		solderH = solderRect[i].Height();

		if(isChipsolder)
		{
			margin = 0;
			clipZmap = zmapDataTemp;
			srcImg_Top = imgT;
		}
		else
		{		
			margin = 20;
			solderW = solderRect[i].Width();
			solderH = solderRect[i].Height() + margin;

			cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));
			cy = (int)((solderRect[i].top + (solderRect[i].Height() / 2.0)) - (margin / 2.0));

			temp = (int)(cy - (solderH / 2.0));
			if(temp < 0)
			{
				cy += abs(temp);
				solderH -= abs(temp);
			}

			milClipZmap = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH);
			//clipZmap = new float[solderW * solderH];
			clipZmap = g_pMManager->pem_new<float>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);

			Im::Buf::Get(milClipZmap, clipZmap);

			FreeMilImageBuff(&milClipZmap);

			//2d 
			Im::PIL_ID milImgTemp = AllocBuff(roiSizeX, roiSizeY);
			Im::PIL_ID milClipTemp = M_NULL;		
			int  solderArea = solderW* solderH;

			Im::Buf::Put(milImgTemp, imgT);
			milClipTemp = AllocClipBuff(milImgTemp, cx, cy, solderW, solderH);
			//srcImg_Top = new UCHAR[solderArea];
			srcImg_Top = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
			Im::Buf::Get(milClipTemp, srcImg_Top);

			SaveWorkImg(milImgTemp,_T("milImgTemp.bmp"));
			SaveWorkImg(milClipTemp,_T("milClipTemp.bmp"));
			FreeMilImageBuff(&milClipTemp);
			FreeMilImageBuff(&milImgTemp);
		}

		int rectSize = solderH ;
		//float* Startprofile = new float[rectSize];			//teaching height 80% under profile
		float* Startprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Startprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile = new float[rectSize];			//teaching  height 80% over profile
		float* Tempprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile2 = new float[rectSize];         // Tempprofile2 = Tempprofile - Tempprofile
		float* Tempprofile2 = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile2, 0, sizeof(float) * (rectSize));

		//float* Total = new float[rectSize];
		float* Total = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Total, 0, sizeof(float) * (rectSize));

		//short* ucenProfile = new short[solderH];			//2d profile
		short* ucenProfile = g_pMManager->pem_new<short>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(ucenProfile, 0, sizeof(short) * (solderH));
//////////////////////////////////////////////////////////////////////////
		//shk 20140424 
		int cXfor2D = 0;
		int widthfor2D = 0;
		int bottomfor2D = 0;

		if(isUse2D)
		{
			//clipZmap Z맵을 이용하여 검출할 width 계산  dstZmapWidth
			float* dstZmapWidth = NULL;
			/*dstZmapWidth = new float[solderW * solderH];
			UCHAR* pDst = new UCHAR[solderW * solderH];*/
			dstZmapWidth = g_pMManager->pem_new<float>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);
			UCHAR* pDst = g_pMManager->pem_new<UCHAR>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);
			ippsThreshold_LTValGTVal_32f(clipZmap, dstZmapWidth,  solderW * solderH, (Ipp32f)threshold_High, 0,(Ipp32f)threshold_High, 1); 
			ippsConvert_32f8u_Sfs(dstZmapWidth,pDst,solderW*solderH,ippRndNear,0);

			int stdMinBlobArea = 100;
			if(!isChipsolder)
				stdMinBlobArea = 20;

			int blobCnt = CalcBlob(pDst, solderW, solderH, stdMinBlobArea, 0, FALSE, FALSE, pDst);

			if(blobCnt == 1)
			{		
				CMilBlobResult *blobRst = new CMilBlobResult(blobCnt);		
				g_pMManager->pem_new_check(blobRst, (PCHAR)__FUNCTION__, __LINE__);

				GetBlobResult(blobRst);
				cXfor2D = (int)(blobRst->cx[0]);
				widthfor2D = (int)(blobRst->width[0]);
				bottomfor2D = (int)(blobRst->bottom[0]);

				//delete blobRst;
				g_pMManager->pem_delete(blobRst, false);
			}

			/*delete dstZmapWidth;
			delete pDst;*/
			g_pMManager->pem_delete(dstZmapWidth, false);
			g_pMManager->pem_delete(pDst, false);
		}

//////////////////////////////////////////////////////////////////////////
		int zindex = 0;

		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				zindex = (y * solderW) + x;

				if(clipZmap[zindex] <= threshold_Low)
				{
					Startprofile[y] += clipZmap[zindex];
				} 
				else
				{
					Tempprofile[y] += clipZmap[zindex];
				}

				Total[y] += clipZmap[zindex];

				//2d 계산
				if (x == (int)solderW/2)
				{
					ucenProfile[y] = srcImg_Top[zindex];
				}
			}	
		}

		float tempMaxVal = 0.f;
		float tempMinVal = 0.f;
		int MaxInd = 0;
		int MinInd = 0;
		int solderSt = 0;
		int LeadEd = 0;

		ippsSub_32f(Tempprofile,Startprofile,Tempprofile2,rectSize);
		ippsMinIndx_32f(Tempprofile2,rectSize,&tempMinVal,&MinInd);
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile2, rectSize, 0, 0, 0, 1); 

		// 		if (isChipsolder)
		// 		{
		// 			ippsMaxIndx_32f(Tempprofile2+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);
		// 		}
		// 		else
		// 			//ippsMaxIndx_32f(Startprofile+MinInd,(int)(1.5*margin)-MinInd,&tempMaxVal,&MaxInd);
		ippsMaxIndx_32f(Tempprofile2+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);

		MaxInd = MaxInd + MinInd ;
		solderSt = MaxInd -3;//권팀장이 책임진다고 함...// + 1; 
		LeadEd = MaxInd - 4;//20140318 권팀장이 전적인 책임을 진다고 함. -4 ==> -7
		if (solderSt < 0 )
		{
			solderSt = 0;
		}
		if (LeadEd < 0 )
		{
			LeadEd = 0;
		}


		//fillet 계산
		if (rectSize > solderSt+solderInspLength)  // MaxInd -> solderSt, Startprofile -> Total
		{
			ippsZero_32f(Total+solderSt+solderInspLength,rectSize-solderSt-solderInspLength);
		}

		//2d계산 ---> start기준선 위쪽으로 5pixel 포함
		int graySolderSt = 0;
		graySolderSt = solderSt;

		if (graySolderSt < 5)
		{
			graySolderSt = 5;
		}

		float grayfratio = 0.f;
#if 0
		//shk 20140320 grayscale threshold 기준 ==> solder start에서 -5 지점에서 solder start까지의 평균값*0.9 이하를 threshold
		short pThreshold = 0;
		ippsMean_16s_Sfs(ucenProfile+graySolderSt-5,5,&pThreshold,0);
		pThreshold = (short)(pThreshold*0.9);	

		ippsThreshold_LTValGTVal_16s(ucenProfile+graySolderSt-5,ucenProfile+graySolderSt-5,rectSize-graySolderSt+5,pThreshold,1,pThreshold,0);// 50 ---> pThreshold

		if (rectSize > solderSt+solderInspLength)  // MaxInd -> solderSt, Startprofile -> Total
		{
			//2d 계산
			ippsZero_16s(ucenProfile+solderSt+solderInspLength,rectSize-solderSt-solderInspLength);
		}

		short TempgrayCnt = 0;
		ippsSum_16s_Sfs(ucenProfile+graySolderSt-5,rectSize-graySolderSt+5,&TempgrayCnt,ippAlgHintNone);

		int grayCnt = 0;
		grayCnt = (int)TempgrayCnt;
#endif
		//shk 20140425 2d only inspect
		//grayscale threshold 기준 ==> solder start에서 -5 지점에서 solder start까지의 평균값*0.9 이하를 threshold
		short pThreshold = 0;
		ippsMean_16s_Sfs(ucenProfile+graySolderSt-5,5,&pThreshold,0);
		pThreshold = (short)(pThreshold*0.9);

		pThreshold = 90;
		//solder Rect 재 계산

		if (isUse2D)  
		{
			int clipcX = 0;
			int clipcY = 0;
			int clipW = 0;
			int clipH = 0;

			if (rectSize < bottomfor2D+solderInspLength)  
			{
				int offSetDiff = 0;
				offSetDiff = (bottomfor2D+solderInspLength) - (rectSize);

				solderInspLength = solderInspLength - offSetDiff; 
			}

			clipcX = cXfor2D;
			clipcY = bottomfor2D+(int)(solderInspLength/2); 
			clipW = widthfor2D;
			clipH = solderInspLength;

			int clipSize = clipW * clipH;

			UCHAR* clipSolderImg = NULL;
		
			//clipSolderImg = new UCHAR[clipW * clipH];
			clipSolderImg = g_pMManager->pem_new<UCHAR>(true, clipW * clipH, (PCHAR)__FUNCTION__, __LINE__);
			GetClipImage(srcImg_Top, solderW, solderH, clipSolderImg, clipcX, clipcY, clipW, clipH);

			IppiSize roiSize = {clipW,clipH};

			//ippiThreshold_LTValGTVal_8u_C1R(clipSolderImg,clipW,clipSolderImg,clipW,roiSize,pThreshold,0,pThreshold,1);
			ippiThreshold_LTVal_8u_C1R(clipSolderImg,clipW,clipSolderImg,clipW,roiSize,pThreshold,0);
			ippiThreshold_GTVal_8u_C1R(clipSolderImg,clipW,clipSolderImg,clipW,roiSize,pThreshold-1,1);

			Ipp64f TempgrayCnt = 0;
			ippiSum_8u_C1R(clipSolderImg,clipW,roiSize,&TempgrayCnt);

			int grayCnt = 0;
			grayCnt = (int)TempgrayCnt;
			//2d 계산

			if (grayCnt <= 0)
			{
				grayfratio = 0.f;
			}
			else
			{
				if (grayCnt >= clipSize )
				{
					grayfratio = 100.f;
				}
				else
				{
					grayfratio =  ((float)grayCnt / clipSize ) * 100;
				}

			}

			grayfratio = 100 - grayfratio;

			//delete clipSolderImg;
			g_pMManager->pem_delete(clipSolderImg, false);
		}
		//////////////////////////////////////////////////////////////////////////

		//1. solderSt 부터 차이값 계산
		ippsDivC_32f(Total, (Ipp32f)solderW, Total, rectSize);
		ippsSub_32f(Total + solderSt + 1, Total+solderSt, Tempprofile2, rectSize - solderSt - 1);

		//2. 차이값  -1, +1로 치환		//3 . filet count 계산

		float filetCnt = 0.f;
		float filetCnt1 = 0.f;
		float filetCnt2 = 0.f;

		// MaxInd -> solderSt
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1, -1, -1, -1, 0); //20130318 권팀장이  전반적으로 책임진다고함 -2 ===> -1
		ippsSum_32f(Tempprofile,rectSize-solderSt-1,&filetCnt1,ippAlgHintNone);

		//ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1, 1, 0, 1, 1);//
		//SHK 20140416
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1,(Ipp32f)LEAD_SOLDER_DOWNSTEP, 0, (Ipp32f)LEAD_SOLDER_DOWNSTEP, 1);

		ippsSum_32f(Tempprofile,rectSize-solderSt-1,&filetCnt2,ippAlgHintNone);

		filetCnt = filetCnt1 + filetCnt2;
		//ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-MaxInd-1, 0, -1, 0, 1);
		//ippsSum_32f(Tempprofile,rectSize-MaxInd-1,&filetCnt,ippAlgHintNone);

		//4. fillet ratio 계산
		float fratio = 0.f;
		if (filetCnt <= 0)
		{
			fratio = 0.f;
		}
		else
		{
			fratio =  (filetCnt / solderInspLength ) * 100;
		}



		/*delete Startprofile;
		delete Tempprofile;
		delete Tempprofile2;

		delete Total;
		delete ucenProfile;*/
		g_pMManager->pem_delete(Startprofile, false);
		g_pMManager->pem_delete(Tempprofile, false);
		g_pMManager->pem_delete(Tempprofile2, false);
		g_pMManager->pem_delete(Total, false);
		g_pMManager->pem_delete(ucenProfile, false);

		if(!isChipsolder)
		{
			/*delete srcImg_Top;
			delete clipZmap;*/
			g_pMManager->pem_delete(srcImg_Top, false);
			g_pMManager->pem_delete(clipZmap, false);
		}
		if(retSolderSt != NULL)
		{
			retSolderSt[i] = solderSt - margin;
		}
		if(retLeadEd != NULL)
		{
			retLeadEd[i] = LeadEd -margin;
		}
		if(retfiletRatio != NULL)
		{
			retfiletRatio[i] = fratio;
		}
		if(retGrayfRatio != NULL)
		{
			retGrayfRatio[i] = grayfratio;
		}


	}
	if(!isChipsolder)
	{
		FreeMilImageBuff(&milZmapSrc);
	}

	//delete zmapDataTemp;
	g_pMManager->pem_delete(zmapDataTemp, false);

	return ret;
}


//shk 20140430
//  통상 검사일때
//  Step 1. 3D : 2D 중 검사 결과 기준 큰값이 Min volume에 만족하면 Step2 진행, 아니면 NG
//  Step 2. 2D / 3D < 70% 이면 NG
//  2D 검사시에는 User define Green line pos. 값을 쓴다.
//-->  chip일때는 solderRect .top  에서 solderStartPos이 Green line pos.
//-->  Lead 일때는 solderRect  기준에서 margin값 20이 무조껀 Green line pos.
//  3D only일때는 2D 무시
int CProcMil_LeadSolder::InspSolderArea_S5(float* zmapData, UCHAR* imgT,int roiSizeX, int roiSizeY,int solderStartPos, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int isUse2D, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio,float* retGrayfRatio)
{
	int ret = TRUE;

	if(zmapData == NULL || solderRect == NULL || solderCnt <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return ret;

	//

	UCHAR* srcImg_Top = NULL;

	float threshold_Low = (float)(avrLeadH * 0.8);
	float threshold_High = (float)(avrLeadH * 0.9);

	float* zmapDataTemp = NULL;
	//zmapDataTemp = new float[roiSizeX * roiSizeY];
	zmapDataTemp = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	int nthresLow = 20;

	if(!isChipsolder)
	{
		nthresLow = 10;
	}

	ippsThreshold_LTVal_32f(zmapData, zmapDataTemp, roiSizeX * roiSizeY, (Ipp32f)nthresLow,  (Ipp32f)0);

	Im::PIL_ID milZmapSrc = M_NULL;
	Im::PIL_ID milClipZmap = M_NULL;
	float* clipZmap = NULL;

	if(!isChipsolder)
	{
		milZmapSrc = AllocBuff_float(roiSizeX, roiSizeY);
		Im::Buf::Put(milZmapSrc, zmapDataTemp);
	}

	int cx = 0;
	int cy = 0;
	int margin = 0;
	int solderW = 0;
	int solderH = 0;
	int temp = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		solderW = solderRect[i].Width();
		solderH = solderRect[i].Height();

		if(isChipsolder)
		{
			margin = 0;
			clipZmap = zmapDataTemp;
			srcImg_Top = imgT;
		}
		else
		{		
			margin = 20;
			solderW = solderRect[i].Width();
			solderH = solderRect[i].Height() + margin;

			cx = (int)(solderRect[i].left + (solderRect[i].Width() / 2.0));
			cy = (int)((solderRect[i].top + (solderRect[i].Height() / 2.0)));// - (margin / 2.0));

			temp = (int)(cy - (solderH / 2.0));
			if(temp < 0)
			{
				cy += abs(temp);
				solderH -= abs(temp);
			}

			milClipZmap = AllocClipBuff_float(milZmapSrc, cx, cy, solderW, solderH);
			//clipZmap = new float[solderW * solderH];
			clipZmap = g_pMManager->pem_new<float>(true, solderW * solderH, (PCHAR)__FUNCTION__, __LINE__);

			Im::Buf::Get(milClipZmap, clipZmap);

			FreeMilImageBuff(&milClipZmap);

			//2d 
			Im::PIL_ID milImgTemp = AllocBuff(roiSizeX, roiSizeY);
			Im::PIL_ID milClipTemp = M_NULL;		
			int  solderArea = solderW* solderH;

			Im::Buf::Put(milImgTemp, imgT);
			milClipTemp = AllocClipBuff(milImgTemp, cx, cy, solderW, solderH);
			//srcImg_Top = new UCHAR[solderArea];
			srcImg_Top = g_pMManager->pem_new<UCHAR>(true, solderArea, (PCHAR)__FUNCTION__, __LINE__);
			Im::Buf::Get(milClipTemp, srcImg_Top);

			SaveWorkImg(milImgTemp,_T("milImgTemp.bmp"));
			SaveWorkImg(milClipTemp,_T("milClipTemp.bmp"));
			FreeMilImageBuff(&milClipTemp);
			FreeMilImageBuff(&milImgTemp);
		}

		int rectSize = solderH ;
		//float* Startprofile = new float[rectSize];			//teaching height 80% under profile
		float* Startprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Startprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile = new float[rectSize];			//teaching  height 80% over profile
		float* Tempprofile = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile, 0, sizeof(float) * (rectSize));

		//float* Tempprofile2 = new float[rectSize];         // Tempprofile2 = Tempprofile - Tempprofile
		float* Tempprofile2 = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Tempprofile2, 0, sizeof(float) * (rectSize));

		//float* Total = new float[rectSize];
		float* Total = g_pMManager->pem_new<float>(true, rectSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(Total, 0, sizeof(float) * (rectSize));

		//short* ucenProfile = new short[solderH];			//2d profile
		short* ucenProfile = g_pMManager->pem_new<short>(true, solderH, (PCHAR)__FUNCTION__, __LINE__);
		memset(ucenProfile, 0, sizeof(short) * (solderH));

		int zindex = 0;

		for(int y = 0; y < solderH; y++)
		{
			for(int x = 0; x < solderW; x++)
			{
				zindex = (y * solderW) + x;

				if(clipZmap[zindex] <= threshold_Low)
				{
					Startprofile[y] += clipZmap[zindex];
				} 
				else
				{
					Tempprofile[y] += clipZmap[zindex];
				}

				Total[y] += clipZmap[zindex];

				//2d 계산
				if (x == (int)solderW/2)
				{
					ucenProfile[y] = srcImg_Top[zindex];
				}
			}	
		}

		float tempMaxVal = 0.f;
		float tempMinVal = 0.f;
		int MaxInd = 0;
		int MinInd = 0;
		int solderSt = 0;
		int LeadEd = 0;

		ippsSub_32f(Tempprofile,Startprofile,Tempprofile2,rectSize);
		ippsMinIndx_32f(Tempprofile2,rectSize,&tempMinVal,&MinInd);
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile2, rectSize, 0, 0, 0, 1); 

		// 		if (isChipsolder)
		// 		{
		// 			ippsMaxIndx_32f(Tempprofile2+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);
		// 		}
		// 		else
		// 			//ippsMaxIndx_32f(Startprofile+MinInd,(int)(1.5*margin)-MinInd,&tempMaxVal,&MaxInd);
		ippsMaxIndx_32f(Tempprofile2+MinInd,rectSize-MinInd,&tempMaxVal,&MaxInd);

		MaxInd = MaxInd + MinInd ;
		solderSt = MaxInd -3;//권팀장이 책임진다고 함...// + 1; 
		LeadEd = MaxInd;// - 4;//20140318 권팀장이 전적인 책임을 진다고 함. -4 ==> -7
		if (solderSt < 0 )
		{
			solderSt = 0;
		}
		if (LeadEd < 0 )
		{
			LeadEd = 0;
		}


		//fillet 계산
		if (rectSize > solderSt+solderInspLength)  // MaxInd -> solderSt, Startprofile -> Total
		{
			ippsZero_32f(Total+solderSt+solderInspLength,rectSize-solderSt-solderInspLength);
		}


		//////////////////////////////////////////////////////////////////////////
		//2d계산 ---> start기준선 위쪽으로 5pixel 포함
		int graySolderSt = 0;

		if (isChipsolder)
		{
			graySolderSt = solderSt+2;//solderStartPos+2;
		}
		else
		{
			graySolderSt =  solderSt+2;//(int)(margin/2)+2; // margin = 20이다.
		}

		if (graySolderSt < 5)
		{
			graySolderSt = 5;
		}

		float grayfratio = 0.f;

		//shk 20140425 2d only inspect
		//grayscale threshold 기준 ==> solder start에서 -5 지점에서 solder start까지의 평균값*0.9 이하를 threshold
		short pThreshold = 0;
		ippsMean_16s_Sfs(ucenProfile+graySolderSt-5,5,&pThreshold,0);
		pThreshold = (short)(pThreshold*0.9);

		if(pThreshold > 145)			// LMJ 2014/05/30
		{
			pThreshold = 145;
		}
 		if (pThreshold < 127)
 		{
			pThreshold = 127;
		}
		//solder Rect 재 계산
		int clipcX = 0;
		int clipcY = 0;
		int clipW = 0;
		int clipH = 0;

		if (rectSize < graySolderSt+solderInspLength)  
		{
			int offSetDiff = 0;
			offSetDiff = (graySolderSt+solderInspLength) - (rectSize);

			solderInspLength = solderInspLength - offSetDiff; 
		}

		clipcX =  (int)(solderW / 2);
		clipcY = graySolderSt+(int)(solderInspLength/2); 
		clipW = (int)(solderW * 0.8); //일단 80%
		clipH = solderInspLength;

		int clipSize = clipW * clipH;

		UCHAR* clipSolderImg = NULL;

		//clipSolderImg = new UCHAR[clipW * clipH];
		clipSolderImg = g_pMManager->pem_new<UCHAR>(true, clipW * clipH, (PCHAR)__FUNCTION__, __LINE__);
		GetClipImage(srcImg_Top, solderW, solderH, clipSolderImg, clipcX, clipcY, clipW, clipH);

		//test
		SaveWorkImg(srcImg_Top, solderW, solderH,_T("srcImg_Top.bmp"));
		SaveWorkImg(clipSolderImg, clipW, clipH,_T("clipSolderImg.bmp"));

		IppiSize roiSize = {clipW,clipH};

		//ippiThreshold_LTValGTVal_8u_C1R(clipSolderImg,clipW,clipSolderImg,clipW,roiSize,pThreshold,0,pThreshold,1);
		ippiThreshold_LTVal_8u_C1R(clipSolderImg,clipW,clipSolderImg,clipW,roiSize,pThreshold,0);
		ippiThreshold_GTVal_8u_C1R(clipSolderImg,clipW,clipSolderImg,clipW,roiSize,pThreshold-1,1);

		Ipp64f TempgrayCnt = 0;
		ippiSum_8u_C1R(clipSolderImg,clipW,roiSize,&TempgrayCnt);

		int grayCnt = 0;
		grayCnt = (int)TempgrayCnt;
		//2d 계산

		if (grayCnt <= 0)
		{
			grayfratio = 0.f;
		}
		else
		{
			if (grayCnt >= clipSize )
			{
				grayfratio = 100.f;
			}
			else
			{
				grayfratio =  ((float)grayCnt / clipSize ) * 100;
			}

		}

		grayfratio = 100 - grayfratio;

		//delete clipSolderImg;
		g_pMManager->pem_delete(clipSolderImg, false);
		
		//////////////////////////////////////////////////////////////////////////

		//1. solderSt 부터 차이값 계산
		ippsDivC_32f(Total, (Ipp32f)solderW, Total, rectSize);
		ippsSub_32f(Total + solderSt + 1, Total+solderSt, Tempprofile2, rectSize - solderSt - 1);

		//2. 차이값  -1, +1로 치환		//3 . filet count 계산

		float filetCnt = 0.f;
		float filetCnt1 = 0.f;
		float filetCnt2 = 0.f;

		// MaxInd -> solderSt
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1, -1, -1, -1, 0); //20130318 권팀장이  전반적으로 책임진다고함 -2 ===> -1
		ippsSum_32f(Tempprofile,rectSize-solderSt-1,&filetCnt1,ippAlgHintNone);

		//ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1, 1, 0, 1, 1);//
		//SHK 20140416
		ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-solderSt-1,(Ipp32f)LEAD_SOLDER_DOWNSTEP, 0, (Ipp32f)LEAD_SOLDER_DOWNSTEP, 1);

		ippsSum_32f(Tempprofile,rectSize-solderSt-1,&filetCnt2,ippAlgHintNone);

		filetCnt = filetCnt1 + filetCnt2;
		//ippsThreshold_LTValGTVal_32f(Tempprofile2, Tempprofile, rectSize-MaxInd-1, 0, -1, 0, 1);
		//ippsSum_32f(Tempprofile,rectSize-MaxInd-1,&filetCnt,ippAlgHintNone);

		//4. fillet ratio 계산
		float fratio = 0.f;
		if (filetCnt <= 0)
		{
			fratio = 0.f;
		}
		else
		{
			fratio =  (filetCnt / solderInspLength ) * 100;
		}

		/*delete Startprofile;
		delete Tempprofile;
		delete Tempprofile2;

		delete Total;
		delete ucenProfile;*/
		g_pMManager->pem_delete(Startprofile, false);
		g_pMManager->pem_delete(Tempprofile, false);
		g_pMManager->pem_delete(Tempprofile2, false);
		g_pMManager->pem_delete(Total, false);
		g_pMManager->pem_delete(ucenProfile, false);

		if(!isChipsolder)
		{
			/*delete srcImg_Top;
			delete clipZmap;*/
			g_pMManager->pem_delete(srcImg_Top, false);
			g_pMManager->pem_delete(clipZmap, false);
		}
		if(retSolderSt != NULL)
		{
			retSolderSt[i] = solderSt - (int)(margin/2);
		}
		if(retLeadEd != NULL)
		{
			retLeadEd[i] = LeadEd - (int)(margin/2);
		}
		if(retfiletRatio != NULL)
		{
			retfiletRatio[i] = fratio;
		}
		if(retGrayfRatio != NULL)
		{
			retGrayfRatio[i] = grayfratio;
		}


	}
	if(!isChipsolder)
	{
		FreeMilImageBuff(&milZmapSrc);
	}

	//delete zmapDataTemp;
	g_pMManager->pem_delete(zmapDataTemp, false);

	return ret;
}

void CProcMil_LeadSolder::DataSave(int* data, int dataSize, CString path)
{
	FILE	*fp;
	wchar_t	fileName[256];
	wsprintf(fileName, _T("%s"), path);

	DeleteFile(fileName);
	_wfopen_s(&fp,fileName, _T("at"));
	//fp = _wfopen(fileName, _T("at"));
	if (!fp) 
	{
		AfxMessageBox(_T("already opened..."));
		return;
	}


	int temp = 0;

	for(int i = 0; i < dataSize ;i++)
	{
		fwprintf(fp, _T("%d,%d\n"),i , data[i] );			
	}



	fclose(fp);
}

void CProcMil_LeadSolder::DataSave(float* data, int dataSize, CString path)
{
	FILE	*fp;
	wchar_t	fileName[256];
	wsprintf(fileName, _T("%s"), path);

	DeleteFile(fileName);
	_wfopen_s(&fp,fileName, _T("at"));
	//fp = _wfopen(fileName, _T("at"));
	if (!fp) 
	{
		AfxMessageBox(_T("already opened..."));
		return;
	}


	int temp = 0;

	for(int i = 0; i < dataSize ;i++)
	{
		fwprintf(fp, _T("%d,%f\n"),i , data[i] );			
	}



	fclose(fp);
}





void CProcMil_LeadSolder::TestImageSave(void* milSrc, int orgSIzeX, int orgSizeY, int cx, int cy, int w, int h, CString path)
{
	Im::PIL_ID milImg = M_NULL;
	milImg = AllocBuffColor(orgSIzeX, orgSizeY);

	Im::Buf::Copy((Im::PIL_ID)milSrc, milImg);


	Im::Gra::Color(M_DEFAULT, M_COLOR_GREEN);
	Im::Gra::Rect(M_DEFAULT, milImg, cx - (w / 2.0), cy - (h / 2.0), cx + (w / 2.0), cy + (h / 2.0));

	Im::Buf::Export(_T("d:\\testimage\\workimage\\") + path, M_BMP, milImg);

	FreeMilImageBuff(&milImg);
}

void CProcMil_LeadSolder::TestImageSave(UCHAR* userSrc, int orgSIzeX, int orgSizeY, int cx, int cy, int w, int h, CString path)
{
	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(orgSIzeX, orgSizeY);
	Im::Buf::Put(milSrc, userSrc);

	Im::PIL_ID milImg = M_NULL;
	milImg = AllocBuffColor(orgSIzeX, orgSizeY);

	Im::Buf::Copy(milSrc, milImg);

	Im::Gra::Color(M_DEFAULT, M_COLOR_GREEN);
	Im::Gra::Rect(M_DEFAULT, milImg, cx - (w / 2.0), cy - (h / 2.0), cx + (w / 2.0), cy + (h / 2.0));

	Im::Buf::Export(_T("d:\\testimage\\workimage\\") + path, M_BMP, milImg);

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milImg);
}


void CProcMil_LeadSolder::MaskTrueImg(UCHAR * src, UCHAR * mask, UCHAR * dst, int sizeX, int sizeY)
{
	if( src == NULL || mask == NULL || dst == NULL) 
		return ;

	UCHAR* temp = NULL;
	//temp = new UCHAR[sizeX * sizeY];
	temp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, (sizeof(UCHAR) * sizeX * sizeY));

	int id = 0;
	for( int y = 0 ; y < sizeY; y ++)
	{
		for( int x = 0; x < sizeX; x ++)
		{
			id = (y * sizeX) + x;
			if(mask[id] > 0)
				temp[id] = src[id];
			else
				temp[id] = 0;
		}
	}

	memcpy_s(dst, (sizeof(UCHAR) * sizeX * sizeY), temp, (sizeof(UCHAR) * sizeX * sizeY));
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}


int CProcMil_LeadSolder::SearchStartPos(UCHAR* srcImg, int sizeX, int sizeY)
{
	int ret = 0;

	int cx = (int)(sizeX / 2.0);

	
	bool startFlag = false;
	int index = 0;

	UCHAR* tempSrc = srcImg;
	UCHAR data = 0;
	tempSrc += cx;
	for(int y = 0; y < sizeY; y++)
	{
		data = *tempSrc;
		if(data == 0)
			startFlag = true;

		if(startFlag == true)
		{
			if(data > 0)
			{
				index = y;
				break;
			}
		}
		tempSrc += sizeX;
	}

	ret = index;
	return ret;
}

LeadInfoTemp CProcMil_LeadSolder::CalcLeadRect4(UCHAR* ptruc2DImage, float* ptrf3DImage, int nImgW, int nImgH,
	BOOL bUse2D, BOOL bUse3D, int nLeadTipPos, CRect** retRect, int** retLeadPitch)
{
	LeadInfoTemp sLeadInfo;
	//////////////////////////////////////////////////////////////////////////
	sLeadInfo.countLead = 0;
	sLeadInfo.avgLead = 0;
	sLeadInfo.maxLead = 0;
	sLeadInfo.minLead = 0;

	sLeadInfo.countGap = 0;
	sLeadInfo.avgGap = 0;
	sLeadInfo.maxGap = 0;
	sLeadInfo.minGap = 0;

	sLeadInfo.leadPitch = 0;
	//////////////////////////////////////////////////////////////////////////
	try
	{
		if(bUse2D == FALSE && bUse3D == FALSE)
			return sLeadInfo;
		if(nImgW <= 0 || nImgH <= 0)
			return sLeadInfo;
		if(nLeadTipPos < 0)
			nLeadTipPos = nImgH / 2;

		double* ptrdDstAvg = g_pMManager->pem_new<double>(true, nImgW, (PCHAR)__FUNCTION__, __LINE__);
		memset(ptrdDstAvg, 0, sizeof(double) * nImgW);
		int nStartY = nLeadTipPos - 5;
		int nEndY = nLeadTipPos + 5;
		if(nStartY < 0)		nStartY = 0;
		if(nEndY > nImgH)	nEndY = nImgH;
		double dYTotal = nEndY - nStartY;

		if(bUse2D == TRUE)
		{
			for (int nX = 0; nX < nImgW; nX++)
			{
				double dYSum = 0;
				for (int nY = nStartY; nY < nEndY; nY++)
				{
					int nIdx = (nY * nImgW) + nX;
					dYSum += ptruc2DImage[nIdx];
				}
				double dYAvg = dYSum / dYTotal;
				if(dYAvg > 0)
					ptrdDstAvg[nX] = 1;
			}
		}

		if(bUse3D == TRUE)
		{
			int nHCnt = 0;
			double dHSum = 0;
			for (int nX = 0; nX < nImgW; nX++)
			{
				double dYSum = 0;
				for (int nY = nStartY; nY < nEndY; nY++)
				{
					int nIdx = (nY * nImgW) + nX;
					float fHeight = ptrf3DImage[nIdx];
					if(fHeight < 0) fHeight = 0.0f;
					dYSum += fHeight;
				}
				if((ptrdDstAvg[nX] == 1 && bUse2D == TRUE) || bUse2D == FALSE)
				{
					ptrdDstAvg[nX] = dYSum / dYTotal;
					dHSum += ptrdDstAvg[nX];
					nHCnt++;
				}
				else
					ptrdDstAvg[nX] = 0;
			}

			double dHeightAvg = 50;
			if (nHCnt > 0)
				dHeightAvg = dHSum / nHCnt * 0.7;
			if (dHeightAvg < 50)
				dHeightAvg = 50;

			for (int nX = 0; nX < nImgW; nX++)
			{
				if(ptrdDstAvg[nX] >= dHeightAvg)
					ptrdDstAvg[nX] = 1;
				else
					ptrdDstAvg[nX] = 0;
			}
		}

		CRect* cLeadRect = g_pMManager->pem_new<CRect>(true, nImgW, (PCHAR)__FUNCTION__, __LINE__);
		memset(cLeadRect, 0, sizeof(CRect) * nImgW);
		int nLeadCnt = 0;
		bool bLeftIn = false;

		double dAvgLead = 0;
		double dMaxLead = 0;
		double dMinLead = 0;
		double dAvgPitch = 0;
		for (int nX = 0; nX < nImgW; nX++)
		{
			if(ptrdDstAvg[nX] == 1 && bLeftIn == false)
			{
				cLeadRect[nLeadCnt].left = nX + 1;
				bLeftIn = true;
			}
			if(ptrdDstAvg[nX] == 0 && bLeftIn == true)
			{
				cLeadRect[nLeadCnt].right = nX - 1;
				double dLength = (double)(cLeadRect[nLeadCnt].right - cLeadRect[nLeadCnt].left);
				if(dLength < 5)
				{
					cLeadRect[nLeadCnt].left = 0;
					cLeadRect[nLeadCnt].right = 0;
					bLeftIn = false;
					continue;
				}
				cLeadRect[nLeadCnt].top = 0;
				cLeadRect[nLeadCnt].bottom = nImgH;
				if(nLeadCnt == 0)
				{
					dAvgLead = dLength;
					dMaxLead = dAvgLead;
					dMinLead = dAvgLead;
				}
				else
				{
					if(nLeadCnt == 1)
						dAvgPitch = cLeadRect[nLeadCnt].left - cLeadRect[nLeadCnt - 1].left;
					else
						dAvgPitch = (dAvgPitch + (cLeadRect[nLeadCnt].left - cLeadRect[nLeadCnt - 1].left)) / 2.0;
					dAvgLead = (dAvgLead + dLength) / 2.0;
					if(dLength > dMaxLead)
						dMaxLead = dLength;
					if(dLength < dMinLead)
						dMinLead = dLength;
				}
				bLeftIn = false;
				nLeadCnt++;
			}
		}
		if(nLeadCnt == 0)
		{
			Delete_1DArray(&ptrdDstAvg);
			Delete_1DArray(&cLeadRect);
			return sLeadInfo;
		}

		double dAvgGap = 0;
		double dMaxGap = 0;
		double dMinGap = 0;
		//int* ptrnLeadPitch = new int[nLeadCnt];
		int* ptrnLeadPitch = g_pMManager->pem_new<int>(true, nLeadCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(ptrnLeadPitch, nLeadCnt, sizeof(int) * nLeadCnt);
		for (int n = 0; n < nLeadCnt - 1; n++)
		{
			ptrnLeadPitch[n] = cLeadRect[n + 1].left - cLeadRect[n].right;
			if(n == 0)
			{
				dAvgGap = ptrnLeadPitch[n];
				dMaxGap = ptrnLeadPitch[n];
				dMinGap = ptrnLeadPitch[n];
			}
			else
			{
				dAvgGap = (dAvgGap + ptrnLeadPitch[n]) / 2.0;
				if(ptrnLeadPitch[n] > dMaxGap)
					dMaxGap = ptrnLeadPitch[n];
				if(ptrnLeadPitch[n] < dMinGap)
					dMinGap = ptrnLeadPitch[n];
			}
		}
		//////////////////////////////////////////////////////////////////////////

		//*retRect = new CRect[nLeadCnt];
		*retRect = g_pMManager->pem_new<CRect>(true, nLeadCnt, (PCHAR)__FUNCTION__, __LINE__);
		memcpy_s(*retRect, sizeof(CRect) * nLeadCnt, cLeadRect, sizeof(CRect) * nLeadCnt);
		
		sLeadInfo.countLead = nLeadCnt;
		sLeadInfo.avgLead = dAvgLead;
		sLeadInfo.maxLead = dMaxLead;
		sLeadInfo.minLead = dMinLead;

		sLeadInfo.countGap = nLeadCnt - 1;
		sLeadInfo.avgGap = dAvgGap;
		sLeadInfo.maxGap = dMaxGap;
		sLeadInfo.minGap = dMinGap;
		sLeadInfo.leadPitch = dAvgPitch;
		if(sLeadInfo.countGap > 0)
		{
			//*retLeadPitch = new int[sLeadInfo.countGap];
			*retLeadPitch = g_pMManager->pem_new<int>(true, sLeadInfo.countGap, (PCHAR)__FUNCTION__, __LINE__);
			memcpy_s(*retLeadPitch, sizeof(int) * sLeadInfo.countGap, ptrnLeadPitch, sizeof(int) * sLeadInfo.countGap);
		}
		//////////////////////////////////////////////////////////////////////////
		Delete_1DArray(&ptrdDstAvg);
		Delete_1DArray(&cLeadRect);
		Delete_1DArray(&ptrnLeadPitch);
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}

	return sLeadInfo;	
}

