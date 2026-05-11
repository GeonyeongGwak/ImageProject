#include "PInsp_AlgoOCR.h"


CPInsp_AlgoOCR::CPInsp_AlgoOCR(void)
{
}


CPInsp_AlgoOCR::~CPInsp_AlgoOCR(void)
{
}

void CPInsp_AlgoOCR::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoOCR::GetInspAlgoData()
{
	return eSPCAlgoOCR;
}

int CPInsp_AlgoOCR::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeOcrWrong] = e_NG;
	nCurrentNgType = TypeOcrWrong;
	return nCurrentNgType;
}

bool CPInsp_AlgoOCR::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoOCR::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bAlgoOK = FALSE;

	CPInsp_OCR *pOcrInsp = g_pInspMng->GetOcrInsp();
	if (pOcrInsp == nullptr)
		return bAlgoOK;

#if Timecheck
	//qtm.StartTick();
#endif
	// PYJ 2018/03/12
	if (pOcrInsp == NULL)	// OCR¾µ °æ¿ì NG
	{
		bAlgoOK = e_NG;
		*stAlgoParam.m_dOCRNGScore = 100.0;
		return bAlgoOK;
	}

	if (g_pInspMng->LicenseCheck(M_LICENSE_STR) == false)
		return bAlgoOK;

	pOcrInsp->m_nOCRSimilarityScore = g_pInspMng->m_nOCRSimilarityScore;
	pOcrInsp->m_nHomogeneityScore = g_pInspMng->m_nHomogeneityScore;
	pOcrInsp->m_nrotateInspScore = g_pInspMng->m_nrotateInspScore;
	pOcrInsp->m_bstringMulti = g_pInspMng->m_bstringMulti;
	pOcrInsp->m_bImproveOCRVer = g_pInspMng->m_bImproveOCRVer;
	pOcrInsp->m_OCRNGParamSave = g_pInspMng->m_OCRNGParamSave;

	if (&stAlgoParam.m_sAlignRes[stAlgoParam.m_nCurAlignID] != NULL)
		pOcrInsp->m_dCurrentPartAngle = stAlgoParam.m_sAlignRes[stAlgoParam.m_nCurAlignID].theta;
	pOcrInsp->SetInspParam(sInspAlgo, sWndAlgoImg, *stAlgoParam.m_coordinateAlgo);

	
	if (pOcrInsp->m_bImproveOCRVer)
	{
// 		if (g_pInspMng->m_TeachingMode) bAlgoOK = pOcrInsp->TeachingConSecutiveSpace((RstAlgoOCR *)sRstAlgo, stAlgoParam.m_nInspectionMode);
// 		else 
			bAlgoOK = pOcrInsp->InspProc_OCR2((RstAlgoOCR *)sRstAlgo, stAlgoParam.m_nInspectionMode);
	}
	else
	{
		bAlgoOK = pOcrInsp->InspProc_OCR((RstAlgoOCR *)sRstAlgo, stAlgoParam.m_nInspectionMode);
	}
	

	pOcrInsp->m_dCurrentPartAngle = 0.0;
	//m_AlgoDt[_T("eAlgoOCR")] = (qtm.EndTick() * 1000.0) + m_AlgoDt[_T("eAlgoOCR")];
#if Timecheck
	OCR_insp = (qtm.EndTick() * 1000.0) + OCR_insp;
#endif
	//2016.11.10 shkim save ocr img
	if ((g_pInspMng->m_OCROKimgSave == true && g_pInspMng->m_OCROKimgSave == bAlgoOK) || (g_pInspMng->m_OCRNGimgSave == true && g_pInspMng->m_OCRNGimgSave != bAlgoOK))
	{
		CString Fullpath = ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_sFontPath;
		g_pInspMng->ReleaseImgSave(Fullpath, (bool)bAlgoOK, &sWndAlgoImg, false);
	}
	//m_AlgoDt[_T("eAlgoOCR")] = (qtm.EndTick() * 1000.0) + m_AlgoDt[_T("eAlgoOCR")];
	if (bAlgoOK == FALSE && (((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_bUseOCRNG == FALSE)
		&& ((((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_bStretching == TRUE) || (((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoErode.Use == TRUE) || (((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoDilate.Use == TRUE)))
	{
		
#if Timecheck
		//qtm.StartTick();
#endif
		double angle = 0;
		UCHAR* OCRimg = NULL;
		UCHAR* OCRimgMIX[2] = { NULL,NULL };
		int OCRimgWidth = sWndAlgoImg.m_nWidth;
		int OCRimgHeight = sWndAlgoImg.m_nHeight;
		angle = ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_dFontAngle + (360 - stAlgoParam.m_coordinateAlgo->dROIAngle);
		if (angle >= 360)
			angle -= 360;
		pOcrInsp->m_procMil->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("img_processing_0before.bmp"));
		pOcrInsp->m_procMil->RotateImg_ipp(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, angle, &OCRimg, &OCRimgWidth, &OCRimgHeight);	// LMJ 2013/11/25
		for (int i = 0; i < ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_nImageMixCount; i++)
			pOcrInsp->m_procMil->RotateImg_ipp(OCRimgMIX[i], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, angle, &OCRimgMIX[i], &OCRimgWidth, &OCRimgHeight);

		
		pOcrInsp->m_procMil->SaveWorkImg(OCRimg, OCRimgWidth, OCRimgHeight, _T("img_processing_1before.bmp"));
		for (int i = 0; i < ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_nImageMixCount; i++)
			pOcrInsp->m_procMil->SaveWorkImg(OCRimgMIX[i], OCRimgWidth, OCRimgHeight, _T("img_processing_1before_.bmp"));

		
		if (((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_bStretching == TRUE)
		{
			pOcrInsp->m_Opencv_Functions->Histogram(OCRimg, OCRimgWidth, OCRimgHeight, 5);
			for (int i = 0; i < ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_nImageMixCount; i++)
				pOcrInsp->m_Opencv_Functions->Histogram(OCRimgMIX[i], OCRimgWidth, OCRimgHeight, 5);
		}

		
		pOcrInsp->m_procMil->SaveWorkImg(OCRimg, OCRimgWidth, OCRimgHeight, _T("img_processing_Histogram_2After.bmp"));
		if (((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoErode.Use == TRUE)
		{
			pOcrInsp->m_Opencv_Functions->FilterErosion(OCRimg, OCRimgWidth, OCRimgHeight, ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoErode.filterSize, ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoErode.filterType);
			for (int i = 0; i < ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_nImageMixCount; i++)
				pOcrInsp->m_Opencv_Functions->FilterErosion(OCRimgMIX[i], OCRimgWidth, OCRimgHeight, ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoErode.filterSize, ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoErode.filterType);
		}

		
		pOcrInsp->m_procMil->SaveWorkImg(OCRimg, OCRimgWidth, OCRimgHeight, _T("img_processing_FilterErosion_2After.bmp"));
		if (((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoDilate.Use == TRUE)
		{
			pOcrInsp->m_Opencv_Functions->FilterDilation(OCRimg, OCRimgWidth, OCRimgHeight, ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoDilate.filterSize, ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoDilate.filterType);
			for (int i = 0; i < ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_nImageMixCount; i++)
				pOcrInsp->m_Opencv_Functions->FilterDilation(OCRimgMIX[i], OCRimgWidth, OCRimgHeight, ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoDilate.filterSize, ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->AlgoDilate.filterType);
		}

		
		pOcrInsp->m_procMil->SaveWorkImg(OCRimg, OCRimgWidth, OCRimgHeight, _T("img_processing_FilterDilation_2After.bmp"));

		pOcrInsp->m_procMil->SaveWorkImg(OCRimg, OCRimgWidth, OCRimgHeight, _T("img_processing_2After.bmp"));
		for (int i = 0; i < ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_nImageMixCount; i++)
			pOcrInsp->m_procMil->SaveWorkImg(OCRimgMIX[i], OCRimgWidth, OCRimgHeight, _T("img_processing_2After_.bmp"));

		angle = 360 - angle;
		if (angle < 0)
			angle += 360;

		pOcrInsp->m_procMil->RotateImg_ipp(OCRimg, OCRimgWidth, OCRimgHeight, angle, &sWndAlgoImg.m_ucArr2D, &sWndAlgoImg.m_nWidth, &sWndAlgoImg.m_nHeight);	// LMJ 2013/11/25

		
		for (int i = 0; i < ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_nImageMixCount; i++)
			pOcrInsp->m_procMil->RotateImg_ipp(OCRimgMIX[i], OCRimgWidth, OCRimgHeight, angle, &sWndAlgoImg.m_ucArr2D_Mix[i], &sWndAlgoImg.m_nWidth, &sWndAlgoImg.m_nHeight);

		pOcrInsp->m_procMil->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("img_processing_3Last.bmp"));

#if Timecheck
		OCR_imgprocessing = (qtm.EndTick() * 1000.0) + OCR_imgprocessing;
#endif
		//////////////////////////

#if Timecheck
					//qtm.StartTick();
#endif
		
		stAlgoParam.m_coordinateAlgo->dROIWidth = sWndAlgoImg.m_nWidth;
		stAlgoParam.m_coordinateAlgo->dROILength = sWndAlgoImg.m_nHeight;
		pOcrInsp->SetInspParam(sInspAlgo, sWndAlgoImg, *stAlgoParam.m_coordinateAlgo);
		pOcrInsp->SetHist(true);
		

		
		if (pOcrInsp->m_bImproveOCRVer)
		{
// 			if (g_pInspMng->m_TeachingMode) bAlgoOK = pOcrInsp->TeachingConSecutiveSpace((RstAlgoOCR *)sRstAlgo, stAlgoParam.m_nInspectionMode);
// 			else 
				bAlgoOK = pOcrInsp->InspProc_OCR2((RstAlgoOCR *)sRstAlgo, stAlgoParam.m_nInspectionMode);
		}
		else
		{
			bAlgoOK = pOcrInsp->InspProc_OCR((RstAlgoOCR *)sRstAlgo, stAlgoParam.m_nInspectionMode);
		}
		

		pOcrInsp->SetHist(false);
		if ((g_pInspMng->m_OCROKimgSave == true && g_pInspMng->m_OCROKimgSave == bAlgoOK) || (g_pInspMng->m_OCRNGimgSave == true && g_pInspMng->m_OCRNGimgSave != bAlgoOK))
		{
			CString Fullpath = ((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_sFontPath;
			g_pInspMng->ReleaseImgSave(Fullpath, (bool)bAlgoOK, &sWndAlgoImg, true);
		}
		
#if Timecheck
		OCR_insp_afterimgprocessing = (qtm.EndTick() * 1000.0) + OCR_insp_afterimgprocessing;
#endif
		//delete OCRimg;
		g_pMManager->pem_delete(OCRimg, false);
		OCRimg = NULL;
	}
	if (((AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam)->m_bUseOCRNG)
	{
		*stAlgoParam.m_dUSENGOCR = TRUE;
		*stAlgoParam.m_dOCRNGScore = ((RstAlgoOCR *)sRstAlgo)->m_dStringScore;
	}
	else
		*stAlgoParam.m_dOCROKScore = ((RstAlgoOCR *)sRstAlgo)->m_dStringScore;
	
	return bAlgoOK;
}

BOOL CPInsp_AlgoOCR::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoOCR::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoOCR::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}