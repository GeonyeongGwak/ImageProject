#include "PInsp_AlgoBodyBlob.h"


CPInsp_AlgoBodyBlob::CPInsp_AlgoBodyBlob(void)
{
}


CPInsp_AlgoBodyBlob::~CPInsp_AlgoBodyBlob(void)
{
}

void CPInsp_AlgoBodyBlob::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoBodyBlob::GetInspAlgoData()
{
	return eSPCAlgoBody_Blob;
}

int CPInsp_AlgoBodyBlob::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoBodyBlob * rstAlgo = (RstAlgoBodyBlob *)vRstInspAlgo;
	if (!rstAlgo->m_bOKWidth && !rstAlgo->m_bOKLength && !rstAlgo->m_dRstArea)
	{
		if (rstAlgo->m_dRstWidth < 0.01 && rstAlgo->m_dRstLength < 0.01)
		{
			eWholeNgTypeTemp[TypeMountMissing] = e_NG;
			nCurrentNgType = TypeMountMissing;
		}
	}

	if (!rstAlgo->m_bOKArea)
	{
		eWholeNgTypeTemp[TypeBODYAREA] = e_NG;
		nCurrentNgType = TypeBODYAREA;
	}
	BOOL bROI = (rstAlgo->m_dWNDA == 90 || rstAlgo->m_dWNDA == 270);
	BOOL bOKW = bROI ? rstAlgo->m_bOKLength : rstAlgo->m_bOKWidth;
	BOOL bOKH = bROI ? rstAlgo->m_bOKWidth : rstAlgo->m_bOKLength;

	if (!bOKW)
	{
		eWholeNgTypeTemp[TypeBODYWIDTH] = e_NG;
		nCurrentNgType = TypeBODYWIDTH;
	}
	if (!bOKH)
	{
		eWholeNgTypeTemp[TypeBODYLENGTH] = e_NG;
		nCurrentNgType = TypeBODYLENGTH;
	}

	if (!rstAlgo->m_bOKHeight)
	{
		eWholeNgTypeTemp[TypeMountLift] = e_NG;
		nCurrentNgType = TypeMountLift;
	}

	if (!rstAlgo->m_bOKAngle)
	{
		eWholeNgTypeTemp[TypeMountAngle] = e_NG;
		nCurrentNgType = TypeMountAngle;
	}

	if (!rstAlgo->m_bOKShiftX || !rstAlgo->m_bOKShiftY || !rstAlgo->m_bOKShiftMaxX || !rstAlgo->m_bOKShiftMaxY || !rstAlgo->m_bOKOffsetDistance)
	{
		eWholeNgTypeTemp[TypeMountShift] = e_NG;
		nCurrentNgType = TypeMountShift;
	}

	if (rstAlgo->m_bBillboarding)
	{
		eWholeNgTypeTemp[TypeBillboarding] = e_NG;
	}

	if (rstAlgo->m_bTombstone)
	{
		eWholeNgTypeTemp[TypeTombstone] = e_NG;
	}
	if (!rstAlgo->m_bOKDamage)
	{
		eWholeNgTypeTemp[TypeDamage] = e_NG;
		nCurrentNgType = TypeDamage;
	}
	if (!rstAlgo->m_bOKShape)
	{
		eWholeNgTypeTemp[TypeForeign] = e_NG;
		nCurrentNgType = TypeForeign;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoBodyBlob::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoBodyBlob::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	AlgoBodyBlob *pAlgoBodyBlob = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
	RstAlgoBodyBlob * pRstAlgo = (RstAlgoBodyBlob *)sRstAlgo;
	if (stAlgoParam.m_bInspection == FALSE)
	{
		if (stAlgoParam.m_bTeachingFunc)
			return TeachBodyBlob(sInspAlgo, sWndAlgoImg, ucArrDstImg, 0, *sInspImageData, pRstAlgo, stTieArea);

		pRstAlgo->m_dRstShiftX = pAlgoBodyBlob->m_dShiftX;
		pRstAlgo->m_dRstShiftY = pAlgoBodyBlob->m_dShiftY;
	}

	// inspType이 eINSP_MOUNT or eINSP_ALIGN 이면 arrAlignRes에 shift 결과값을 넣는다.
	// inspbodyblob 에서는 wnd center 좌표가 필요 없어서 기존에 없었으나, masking 제거시에 wnd center 가 필요해서 인자로 추가함.
	bResult = InspBodyBlob(sInspAlgo, sWndAlgoImg, stAlgoParam.m_bTeach, *stAlgoParam.m_rcBlobBody, *sInspImageData, (RstAlgoBodyBlob*)sRstAlgo, stAlgoParam.m_ptWndCenter, stTieArea, ucArrDstImg);

	if (stAlgoParam.m_bInspection == FALSE)
	{
		CPInsp_Algo *pAlgo = g_pInspMng->GetPtrInspAlgo();
		double dSearchMargin = pAlgoBodyBlob->m_dSearchMargin * 2;
		double dWndW = stAlgoParam.m_sWndInfo->dWidth + dSearchMargin;
		double dWndL = stAlgoParam.m_sWndInfo->dLength + dSearchMargin;
		double dPartW = sWndAlgoImg.m_nWidth * pAlgo->GetResolX();
		double dPartL = sWndAlgoImg.m_nHeight * pAlgo->GetResolY();
		dWndW = MIN(dWndW, dPartW);
		dWndL = MIN(dWndL, dPartL);
		pRstAlgo->CalcAreaRate(dWndW, dWndL, dSearchMargin);
	}

	return bResult;
}

BOOL CPInsp_AlgoBodyBlob::InspBodyBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg,
	BOOL bTeach, CRect &rcBlobBody, InspRoiImgBuf &sInspImageData,
	RstAlgoBodyBlob * sRstAlgo, POINTF ptWndCenter, TotalInspExceptArea stTieArea, UCHAR *pUcImgBlob/*=NULL*/)
{
	BOOL bRet = FALSE;

	if (sInspAlgo.m_eAlgoType != eAlgoBody_Blob || g_pInspMng->GetPtrInspAlgo() == NULL)
		return bRet;
	bRet = g_pInspMng->GetPtrInspAlgo()->m_inspMount.InspBodyBlob(sInspAlgo, sWndAlgoImg, sInspImageData,
		bTeach, sRstAlgo, rcBlobBody, ptWndCenter, stTieArea, pUcImgBlob);

	return bRet;
}

BOOL CPInsp_AlgoBodyBlob::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	PIAL::_RstAlgoBodyBlob pRstAlgo;
	PIAL::_AlgoBodyBlob pAlgoBodyBlob;
	InspWrapper->ConvertAlgo((AlgoBodyBlob*)InspAlgo.m_ptrInspAlgoParam, pAlgoBodyBlob);
	//가려진 상태인지 확인하여 Flag 넣어야 함.
	InspPartInfo *pInspPartInfo = nullptr;
	POINTF *pPoHiddenAreaOfPartImgPx = nullptr;
	BOOL bUseHiddenArea = FALSE;

	//HiddenArea
	pInspPartInfo = g_pInspMng->GetInspPartInfo();
	if (pInspPartInfo != nullptr)
	{
		if (pInspPartInfo->bStickerInsp == TRUE)
		{
			pPoHiddenAreaOfPartImgPx = pInspPartInfo->pPoHiddenArea;
			bUseHiddenArea = TRUE;
		}
	}
#if HIDDENAREA_TEST
	if (pInspPartInfo == nullptr)
		pPoHiddenAreaOfPartImgPx = new POINTF[8];
	else
		pPoHiddenAreaOfPartImgPx = pInspPartInfo->pPoHiddenArea;
	pPoHiddenAreaOfPartImgPx[0].x = 0;
	pPoHiddenAreaOfPartImgPx[1].x = 551;
	pPoHiddenAreaOfPartImgPx[2].x = 553;
	pPoHiddenAreaOfPartImgPx[3].x = 0;
	pPoHiddenAreaOfPartImgPx[0].y = 351;
	pPoHiddenAreaOfPartImgPx[1].y = 353;
	pPoHiddenAreaOfPartImgPx[2].y = 1026;
	pPoHiddenAreaOfPartImgPx[3].y = 1026;
	pPoHiddenAreaOfPartImgPx[4].x = 0;
	pPoHiddenAreaOfPartImgPx[5].x = 0;
	pPoHiddenAreaOfPartImgPx[6].x = 0;
	pPoHiddenAreaOfPartImgPx[7].x = 0;
	pPoHiddenAreaOfPartImgPx[4].y = 0;
	pPoHiddenAreaOfPartImgPx[5].y = 0;
	pPoHiddenAreaOfPartImgPx[6].y = 0;
	pPoHiddenAreaOfPartImgPx[7].y = 0;
	bUseHiddenArea = TRUE;
#endif

	//이물, 높이 검사를 제외하고 다른 검사는 하지 않도록 해야함.
	if (bUseHiddenArea)
	{
		pAlgoBodyBlob.m_bUseBodyTip = FALSE; //가려진 경우 BodyTip이 없을 수 있기 때문에 해당 옵션 비활성화하여 검사 진행.
		pAlgoBodyBlob.m_bAreaIsUse = FALSE;
		pAlgoBodyBlob.m_bShiftIsUse = FALSE;
		pAlgoBodyBlob.m_bShiftXUse = FALSE;
		pAlgoBodyBlob.m_bShiftYUse = FALSE;
		pAlgoBodyBlob.m_bTeachWidthUse = FALSE;
		pAlgoBodyBlob.m_bTeachLengthUse = FALSE;
		pAlgoBodyBlob.m_bUseAngle = FALSE;
		pAlgoBodyBlob.m_bUseShape = FALSE;
		pAlgoBodyBlob.m_bUseDamage = FALSE;
		pAlgoBodyBlob.m_bUseHeight = TRUE;
	}

	//pImg_buf의 영상을 복제하여 사용해야함.
	PIAL::PI_Buff* clone2D = pImg_buf->m_p2D->Clone();
	PIAL::PI_Buff* clone3D = pImg_buf->inspWndImage->m_p3D->Clone();
	PIAL::PI_Buff* org2D = pImg_buf->m_p2D;
	PIAL::PI_Buff* org3D = pImg_buf->inspWndImage->m_p3D;
	pImg_buf->m_p2D = clone2D;
	pImg_buf->inspWndImage->m_p3D = clone3D;

	//sticker 영역 받아와서 검사 이미지에 적용해줘야함.
	cv::Mat img2D = clone2D->Mat();
	cv::Mat img3D = clone3D->Mat();
	cv::Mat mat2D = org2D->Mat();
	cv::Mat mat3D = org3D->Mat();

	if (pPoHiddenAreaOfPartImgPx != nullptr && bUseHiddenArea)
	{
		for (int nStcIdx = 0; nStcIdx < StickerCnt; nStcIdx++)
		{
			g_pInspMng->m_FR.FillPolygon(img2D, pPoHiddenAreaOfPartImgPx + (nStcIdx * StickerPo), StickerPo, cv::Scalar(0, 0, 0));
			g_pInspMng->m_FR.FillPolygon(img3D, pPoHiddenAreaOfPartImgPx + (nStcIdx * StickerPo), StickerPo, cv::Scalar(0, 0, 0));
		}
	}

	if (algoParam.m_bInspection == FALSE)
	{
		if (algoParam.m_bTeachingFunc)
			return InspWrapper->m_PInspAlgo->TeachBodyBlob(pAlgoBodyBlob, *pImg_buf, &pRstAlgo, algoParam.m_ptWndCenter, PIALTieArea/*,매개확인*/);

		pRstAlgo.m_dRstShiftX = pAlgoBodyBlob.m_dShiftX;
		pRstAlgo.m_dRstShiftY = pAlgoBodyBlob.m_dShiftY;
	}

	bResult = InspBodyBlob_Dll(pAlgoBodyBlob, *pImg_buf, *algoParam.m_rcBlobBody, &pRstAlgo, algoParam.m_ptWndCenter, PIALTieArea, vecAlignResult, nullptr, bodyInfo, algoParam.m_dx, algoParam.m_dy);

	if (pAlgoBodyBlob.Passive)
	{
		pRstAlgo.m_rcBodyRect.left += algoParam.m_dx;
		pRstAlgo.m_rcBodyRect.right += algoParam.m_dx;
		pRstAlgo.m_rcBodyRect.top += algoParam.m_dy;
		pRstAlgo.m_rcBodyRect.bottom += algoParam.m_dy;

		pRstAlgo.m_rcInspBodyRect.left += algoParam.m_dx;
		pRstAlgo.m_rcInspBodyRect.right += algoParam.m_dx;
		pRstAlgo.m_rcInspBodyRect.top += algoParam.m_dy;
		pRstAlgo.m_rcInspBodyRect.bottom += algoParam.m_dy;

		if (pRstAlgo.m_rcBodyDamageRect.top != 0 && pRstAlgo.m_rcBodyDamageRect.left != 0 &&
			pRstAlgo.m_rcBodyDamageRect.bottom != 0 && pRstAlgo.m_rcBodyDamageRect.right != 0)
			pRstAlgo.m_rcBodyDamageRect.left += algoParam.m_dx;
		pRstAlgo.m_rcBodyDamageRect.right += algoParam.m_dx;
		pRstAlgo.m_rcBodyDamageRect.top += algoParam.m_dy;
		pRstAlgo.m_rcBodyDamageRect.bottom += algoParam.m_dy;

		InspDataSet->SetThickestPosition(pRstAlgo.m_dRstHeightMean, &pRstAlgo.m_rcBodyRect);
		// InspDataSet->SetThickestPosition(pRstAlgo.m_dRstHeightMean, &pRstAlgo.m_rcInspBodyRect);
	}

	if (algoParam.m_bInspection == FALSE)
	{
		CPInsp_Algo *pAlgo = g_pInspMng->GetPtrInspAlgo();
		double dSearchMargin = pAlgoBodyBlob.m_dSearchMargin * 2;
		double dWndW = algoParam.m_sWndInfo->dWidth + dSearchMargin;
		double dWndL = algoParam.m_sWndInfo->dLength + dSearchMargin;
		double dPartW = pImg_buf->inspWndImage->nImgSizeX * pAlgo->GetResolX();
		double dPartL = pImg_buf->inspWndImage->nImgSizeY * pAlgo->GetResolY();
		dWndW = MIN(dWndW, dPartW);
		dWndL = MIN(dWndL, dPartL);
		pRstAlgo.CalcAreaRate(dWndW, dWndL, dSearchMargin);
	}
	InspWrapper->ConvertRstAlgo(pRstAlgo, (RstAlgoBodyBlob*)sRstAlgo);

	pImg_buf->m_p2D = org2D;
	pImg_buf->inspWndImage->m_p3D = org3D;
	if (clone2D)
		delete clone2D;
	if (clone3D)
		delete clone3D;

#if HIDDENAREA_TEST
	if (pInspPartInfo == nullptr)
		delete[] pPoHiddenAreaOfPartImgPx;
#endif
	return bResult;
}

BOOL CPInsp_AlgoBodyBlob::InspBodyBlob_Dll(PIAL::_AlgoBodyBlob& algoBodyBlob, PIAL::Insp_Image& pImg_buf, CRect &rcBlobBody, PIAL::_RstAlgoBodyBlob * sRstAlgo, POINTF ptWndCenter, PIAL::_TotalInspExceptArea stTieArea, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::PI_Buff* pMask, PIAL::BodyInfo* pBodyInfo, double nDx, double nDy)
{
	BOOL bRet = FALSE;

	if (&algoBodyBlob == nullptr)
		return bRet;

	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	BOOL bUseHiddenArea = g_pInspMng->GetInspPartInfo()->bStickerInsp;
#if HIDDENAREA_TEST
	bUseHiddenArea = TRUE;
#endif
	bRet = InspWrapper->m_PInspAlgo->InspBodyBlob(algoBodyBlob, pImg_buf, rcBlobBody, sRstAlgo, ptWndCenter, stTieArea, vecAlignResult, pMask, pBodyInfo, bUseHiddenArea);
	if (pBodyInfo)
	{
		pBodyInfo->Passive = algoBodyBlob.Passive;
		pBodyInfo->BodyStartPos = new POINT();
		pBodyInfo->BodySize = new SIZE();
		int nMarginX = algoBodyBlob.PassiveMargin / PIAL::PInspAlgo_Lib::m_resolX;
		int nMarginY = algoBodyBlob.PassiveMargin / PIAL::PInspAlgo_Lib::m_resolY;

		if (rcBlobBody.Width() <= 0 || rcBlobBody.Height() <= 0)
		{
			pBodyInfo->BodySize->cx = pImg_buf.inspWndImage->nImgSizeX; // Width
			pBodyInfo->BodySize->cy = pImg_buf.inspWndImage->nImgSizeY; // Height
			pBodyInfo->BodyStartPos->x = pImg_buf.inspWndImage->nStartX;
			pBodyInfo->BodyStartPos->y = pImg_buf.inspWndImage->nStartY;
		}
		else
		{
			pBodyInfo->BodySize->cx = rcBlobBody.Width() + nMarginX * 2;
			pBodyInfo->BodySize->cy = rcBlobBody.Height() + nMarginY * 2;
			pBodyInfo->BodyStartPos->x = rcBlobBody.left + nDx - nMarginX;
			pBodyInfo->BodyStartPos->y = rcBlobBody.top + nDy - nMarginY;
		}
	}
	return bRet;
}

BOOL CPInsp_AlgoBodyBlob::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	AlgoBodyBlob *pAlgoBodyBlob = (AlgoBodyBlob *)ptrInspAlgoParam;
	RstAlgoBodyBlob * rst = (RstAlgoBodyBlob *)sRstAlgo;
	pAlignRes->offsetX = -rst->m_dRstShiftX;
	pAlignRes->offsetY = rst->m_dRstShiftY;
	pAlignRes->theta = rst->m_dRstAngle;// -pAlgoBodyBlob->m_dStandardRotate;
	bRet = TRUE;

	return bRet;
}
int CPInsp_AlgoBodyBlob::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoBodyBlob * pInspAlgo = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
	nData = pInspAlgo->m_sAlgoColorBase.GetColorData();

	return nData;
}
BOOL CPInsp_AlgoBodyBlob::TeachBodyBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* puImgDst, int nMaskViewingMode, InspRoiImgBuf &sInspImageData, RstAlgoBodyBlob *pRstAlgo, TotalInspExceptArea stTieArea)
{
	BOOL bResult = FALSE;
	int nLine = __LINE__;
	try
	{
		if (sInspAlgo.m_eAlgoType != eAlgoBody_Blob || !m_pProcMilAlgo || puImgDst == NULL)
			return bResult;
		AlgoBodyBlob *pInspAlgo = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
		if (!pInspAlgo)
			return bResult;
		nLine = __LINE__;
		// 원래 이미지 2진화
		UCHAR *pUcOrgSrc = sWndAlgoImg.m_ucArr2D;
		float *pfOrgSrc = sWndAlgoImg.m_fArr3D;
		int nOrgWidth = sWndAlgoImg.m_nWidth;
		int nOrgHeight = sWndAlgoImg.m_nHeight;
		if (nOrgWidth != sWndAlgoImg.m_nWidth3D && nOrgWidth > sWndAlgoImg.m_nWidth3D)
			nOrgWidth = sWndAlgoImg.m_nWidth3D;
		if (nOrgHeight != sWndAlgoImg.m_nHeight3D && nOrgHeight > sWndAlgoImg.m_nHeight3D)
			nOrgHeight = sWndAlgoImg.m_nHeight3D;
		if (pUcOrgSrc == NULL || pfOrgSrc == NULL || nOrgWidth <= 0 || nOrgHeight <= 0)
			return bResult;
		int nOrgArea = nOrgWidth * nOrgHeight;
		memset(puImgDst, 0, sizeof(UCHAR) * nOrgArea);
		nLine = __LINE__;
		m_pProcMilAlgo->SaveWorkImg(pUcOrgSrc, nOrgWidth, nOrgHeight, _T("Org2D.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(pfOrgSrc, nOrgWidth, nOrgHeight, _T("Org3D.bmp"));
		nLine = __LINE__;
		RECT rcArrPARTMaskingROI[MAX_MASKING_NUM];
		int nUsedPARTMaskingValue = 0;
		POINTF ptArrPARTInspPolygon[MAX_INSP_AREA_COUNT];
		int nUsedPARTInspPolygon = 0;
		nLine = __LINE__;
		/*UCHAR* puImgDefault = new UCHAR[nOrgArea];
		UCHAR* puImgTip = new UCHAR[nOrgArea];*/
		UCHAR* puImgDefault = g_pMManager->pem_new<UCHAR>(true, nOrgArea, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* puImgTip = g_pMManager->pem_new<UCHAR>(true, nOrgArea, (PCHAR)__FUNCTION__, __LINE__);
		memset(puImgDefault, 0, sizeof(UCHAR) * nOrgArea);
		memset(puImgTip, 0, sizeof(UCHAR) * nOrgArea);
		nLine = __LINE__;
		AlgoBlackWhite algoBW;
		algoBW.m_bInvertCheck = FALSE;
		double dCX = 0;
		double dCY = 0;
		double dArea = 0;
		int nMinBlobArea = 4;
		CRect rcBlob(0, 0, 0, 0);
		nLine = __LINE__;

		stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
		stTieArea.m_rcArrMaskingROI = sInspAlgo.m_rcArrMaskingROI;
		//for (int i = 0; i < MAX_MASKING_NUM; i++)
		//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
		//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
		//	stTieArea.m_rcArrMaskingROI.push_back(sInspAlgo.m_rcArrMaskingROI[i]);
		//stTieArea.m_rcArrMaskingROI.resize(sInspAlgo.m_nUsedMaskingValue);
		//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
		//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
		stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
		stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
			stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];

		if (pInspAlgo->m_bUseBodyTip)
		{
			algoBW.m_b2dCheck = pInspAlgo->m_bTip2dCheck;
			algoBW.m_nRange = pInspAlgo->m_nTip2dRange;
			algoBW.m_nMinValue = pInspAlgo->m_nTip2dMinValue;
			algoBW.m_nMaxValue = pInspAlgo->m_nTip2dMaxValue;
			algoBW.m_b3dCheck = pInspAlgo->m_bTip3dCheck;
			algoBW.m_n3dRange = pInspAlgo->m_nTip3dRange;
			algoBW.m_d3dHeightMin = pInspAlgo->m_dTip3dHeightMin;
			algoBW.m_d3dHeightMax = pInspAlgo->m_dTip3dHeightMax;
			m_pCPInsp_Algo->BlobImageStruct_BW(algoBW, sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_fArr3D, nOrgWidth, nOrgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, puImgTip,
				eSelectMix, FALSE, 0, stTieArea, pInspAlgo->m_bFillHole);
			m_pProcMilAlgo->SaveWorkImg(puImgTip, nOrgWidth, nOrgHeight, _T("BodyTip_BW.bmp"));
		}
		nLine = __LINE__;
		algoBW.m_b2dCheck = pInspAlgo->m_bInsp2D;
		algoBW.m_nRange = pInspAlgo->m_nTypeRange2D;
		algoBW.m_nMinValue = pInspAlgo->m_nMinBinary;
		algoBW.m_nMaxValue = pInspAlgo->m_nMaxBinary;
		if (pInspAlgo->m_b3dPerCheck)
		{
			algoBW.m_b3dCheck = pInspAlgo->m_b3dPerCheck;
			algoBW.m_n3dRange = pInspAlgo->m_n3dPerRange;
			algoBW.m_d3dHeightMin = pInspAlgo->m_f3dPerHeightAvg * pInspAlgo->m_f3dPerHeightMin / 100.0f;
			algoBW.m_d3dHeightMax = pInspAlgo->m_f3dPerHeightAvg * pInspAlgo->m_f3dPerHeightMax / 100.0f;
			algoBW.m_d3dAvgHeight = pInspAlgo->m_f3dPerHeightAvg;
		}
		else
		{
			algoBW.m_b3dCheck = pInspAlgo->m_bInsp3D;
			algoBW.m_n3dRange = pInspAlgo->m_nTypeRange3D;
			algoBW.m_d3dHeightMin = pInspAlgo->m_dHeightRateMin;
			algoBW.m_d3dHeightMax = pInspAlgo->m_dHeightRateMax;
			algoBW.m_d3dAvgHeight = pInspAlgo->m_fHeightAvg;
		}
		nLine = __LINE__;
		algoBW.m_sAlgoColorBase = pInspAlgo->m_sAlgoColorBase;
		UCHAR *ucColorImgDst = NULL;
		CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
		if (pColorTeach != NULL && algoBW.m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			nLine = __LINE__;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			nLine = __LINE__;
			memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			nLine = __LINE__;
			pColorTeach->GetColorBaseBin(&algoBW.m_sAlgoColorBase, sInspImageData, ucColorImgDst, true, 0, 0, -1, sWndAlgoImg.m_nLight_index);
			nLine = __LINE__;
			m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sInspImageData.nImageSizeX, sInspImageData.nImageSizeY, _T("OrgColor_Image.bmp"));
		}
		nLine = __LINE__;
		m_pCPInsp_Algo->GetBWImageStruct(algoBW, sWndAlgoImg, puImgDefault, stTieArea, ucColorImgDst);
		nLine = __LINE__;
		m_pProcMilAlgo->SaveWorkImg(puImgDefault, nOrgWidth, nOrgHeight, _T("Default_BW.bmp"));
		nLine = __LINE__;

		if (pInspAlgo->m_bUseBodyTip == TRUE && pInspAlgo->m_bOnlyBodyTip == TRUE)
		{
			memcpy(puImgDst, puImgTip, sizeof(UCHAR) * nOrgArea);
			dArea = std::count(puImgDst, puImgDst + nOrgArea, 255);
		}
		else
		{
			UCHAR* ucBlobBW = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucBlobBW, nOrgArea);
			memset(ucBlobBW, 0, sizeof(UCHAR) * nOrgArea);
			m_pProcMilAlgo->CalcBlob_Select(puImgDefault, ucBlobBW, nOrgWidth, nOrgHeight, nMinBlobArea, FALSE, pInspAlgo->m_bFillHole, 0, eSelectBigger);
			m_pProcMilAlgo->SaveWorkImg(ucBlobBW, nOrgWidth, nOrgHeight, _T("Default_BlobBW.bmp"));
			nLine = __LINE__;
			if (pInspAlgo->m_bUseBodyTip == TRUE && pInspAlgo->m_bOnlyBodyTip == FALSE)
			{
				for (int nIndex = 0; nIndex < nOrgArea; nIndex++)
				{
					UCHAR ucTip = puImgTip[nIndex];
					if (ucBlobBW[nIndex] == 255)
					{
						if (puImgTip[nIndex] == 255)
						{
							puImgDst[nIndex] = 255;
							dArea++;
						}
						else
							puImgDst[nIndex] = 150;
					}
					else
					{
						if (puImgTip[nIndex] == 255)
							puImgDst[nIndex] = 120;
					}
					if (puImgDst[nIndex] == 0)
					{
						if (puImgDefault[nIndex] == 255)
							puImgDst[nIndex] = 50;
					}
				}
			}
			else
			{
				for (int nIndex = 0; nIndex < nOrgArea; nIndex++)
				{
					if (ucBlobBW[nIndex] == 255)
					{
						puImgDst[nIndex] = 255;
						dArea++;
					}
					if (puImgDefault[nIndex] == 255 && ucBlobBW[nIndex] != 255)
						puImgDst[nIndex] = 120;
				}
			}

			Delete_1DArray(&ucBlobBW);
		}
		nLine = __LINE__;
		pRstAlgo->m_dRstArea = 0;
		if (dArea > 0 && pInspAlgo->m_dTeachWidth > 0 && pInspAlgo->m_dTeachLength > 0)
			pRstAlgo->m_dRstArea = (dArea / ((pInspAlgo->m_dTeachWidth / m_resolX) * (pInspAlgo->m_dTeachLength / m_resolY))) * 100.0;
		m_pProcMilAlgo->SaveWorkImg(puImgDst, nOrgWidth, nOrgHeight, _T("Result_BW.bmp"));
		nLine = __LINE__;
		Delete_1DArray(&puImgDefault);
		nLine = __LINE__;
		Delete_1DArray(&puImgTip);
		nLine = __LINE__;
		Delete_1DArray(&ucColorImgDst);
		nLine = __LINE__;
		bResult = TRUE;
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Mount::TeachBodyBlob(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
	return bResult;
}
bool CPInsp_AlgoBodyBlob::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}