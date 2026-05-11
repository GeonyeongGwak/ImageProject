#include "PInsp_AlgoBlob.h"

#define LineDraw 0
CPInsp_AlgoBlob::CPInsp_AlgoBlob(void)
{
}


CPInsp_AlgoBlob::~CPInsp_AlgoBlob(void)
{
}

void CPInsp_AlgoBlob::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
	AINgImageSave = false;
}

unsigned long long CPInsp_AlgoBlob::GetInspAlgoData()
{
	return eSPCAlgoBlob;
}

int CPInsp_AlgoBlob::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoBlob * rstAlgo = (RstAlgoBlob *)vRstInspAlgo;
	bool bMount = (nWndType == eINSP_MOUNT || nWndType == eINSP_OCR);
	if (nWndType == eINSP_SOLDER)
	{
		eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
		nCurrentNgType = TypeSolderFilet;
	}
	else
	{
		if (!rstAlgo->m_bOKArea_Reverse || rstAlgo->m_sInspAC.m_nOKInspAC > 0 || !rstAlgo->m_bOKArea ||
			!rstAlgo->m_bOKWidth || !rstAlgo->m_bOKLength || !rstAlgo->m_bOKDistance ||
			!rstAlgo->m_bOKHeight || !rstAlgo->m_bOKCoilRst || !rstAlgo->m_bOKThickMax
			|| !rstAlgo->m_bOKThickMin || !rstAlgo->m_bOKPinAngle)
		{
			if (bMount)
			{
				eWholeNgTypeTemp[TypeMountWorng] = e_NG;
				nCurrentNgType = TypeMountWorng;
			}
			else if (nWndType == eINSP_ALIGN)
			{
				eWholeNgTypeTemp[TypeAlignWrong] = e_NG;
				nCurrentNgType = TypeAlignWrong;
			}
		}

		if (!rstAlgo->m_bOKShiftX || !rstAlgo->m_bOKShiftY || !rstAlgo->m_bOKOffsetDistance)
		{
			eWholeNgTypeTemp[TypeMountShift] = e_NG;
			nCurrentNgType = TypeMountShift;
		}

		if (!rstAlgo->m_bOKArea)
		{
			if (rstAlgo->m_dRstArea == 0)
			{
				eWholeNgTypeTemp[TypeMountMissing] = e_NG;
				nCurrentNgType = TypeMountMissing;
			}
		}
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoBlob::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoBlob::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	CPInsp_ForeignPattern *pForeignPattern = g_pInspMng->GetForeignPattern();
	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();
	if (pForeignPattern == nullptr)
		return bResult;

	AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
	POINTF centerPtr;
	bool bSwitch = g_pMPTI->m_bLengthByAngel;
	if (pAlgoBlob)
	{
		if (pAlgoBlob->m_bUsePattern && pAlgoBlob->m_bUseFPBW == FALSE)
		{
			double dAngle = pInspBoardInfo ? pInspBoardInfo->angle : 0;
			pForeignPattern->SetInspAlgo(sInspAlgo, sWndAlgoImg, dAngle);
			bResult = pForeignPattern->InspForeign();

			if (dAngle > 360)
				dAngle = dAngle - 360;
			else if (dAngle < 0)
				dAngle = dAngle + 360;
			
			int bSuccess = m_pCPInsp_Algo->FillMaskingROI(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, sWndAlgoImg.dAngle, sWndAlgoImg.m_ucArr2D, sInspAlgo.m_nUsedMaskingValue, sInspAlgo.m_rcArrMaskingROI);
		}
	}

	if (stAlgoParam.m_bInspection == TRUE)
	{
		if (pInspBoardInfo == nullptr)
			return bResult;
		g_pInspMng->winid = stAlgoParam.m_nWndIndex;

		POINTF poCenter;
		poCenter.x = stAlgoParam.m_poCenter.x;
		poCenter.y = stAlgoParam.m_poCenter.y;
		if (pAlgoBlob->m_bInspCoil)
		{
			//19.11.18 KYH
			//coil용 데이터 생성이 안되어있을 경우 파트이미지 버퍼 준비
			//21.01.07 KYH 파트조명 데이터 Blob Mask이미지와 사이즈 일치..
			//int nTmpType = 2 * pAlgoBlob->m_sAlgoColorBase.m_bUseColor + pAlgoBlob->m_sAlgoColorBase.m_bUseAngleColor;
			bool tmp1 = pAlgoBlob->m_sAlgoColorBase.m_bUseColor;
			bool tmp2 = pAlgoBlob->m_sAlgoColorBase.m_bUseAngleColor;
			pAlgoBlob->m_sAlgoColorBase.m_bUseColor = true;
			pAlgoBlob->m_sAlgoColorBase.m_bUseAngleColor = true;
			g_pInspMng->ColorDataInput(sInspAlgo, stAlgoParam.m_dInspW, stAlgoParam.m_dInspH, stAlgoParam.m_dx, stAlgoParam.m_dy, stAlgoParam.m_dWndW, stAlgoParam.m_dWndH, sInspImageData, 0, stAlgoParam.m_nInspCameraType);
			//pAlgoBlob->m_sAlgoColorBase.m_bUseColor = (bool)(nTmpType / 2);
			//pAlgoBlob->m_sAlgoColorBase.m_bUseAngleColor = (bool)(nTmpType % 2);
			pAlgoBlob->m_sAlgoColorBase.m_bUseColor = tmp1;
			pAlgoBlob->m_sAlgoColorBase.m_bUseAngleColor = tmp2;
			bResult = InspBlob(sInspAlgo, sWndAlgoImg, *sInspImageData, stAlgoParam.m_coordinateAlgo, stAlgoParam.m_dInspW, stAlgoParam.m_dInspH, (RstAlgoBlob *)sRstAlgo, stTieArea, stAlgoParam.m_nInspType, stAlgoParam.m_nOffX_pix, stAlgoParam.m_nOffY_pix, poCenter, stAlgoParam.m_nInspectionMode);

		}
		else
		{
			//이 값 Blob 함수에 넘기기
			bResult = InspBlob(sInspAlgo, sWndAlgoImg, *sInspImageData, stAlgoParam.m_coordinateAlgo, stAlgoParam.m_dInspW, stAlgoParam.m_dInspH, (RstAlgoBlob *)sRstAlgo, stTieArea, stAlgoParam.m_nInspType, stAlgoParam.m_nOffX_pix, stAlgoParam.m_nOffY_pix, poCenter, stAlgoParam.m_nInspectionMode);
		}

		RstAlgoBlob * rst = (RstAlgoBlob *)sRstAlgo;
		rst->m_rcRect_I.left += stAlgoParam.m_dx;
		rst->m_rcRect_I.right += stAlgoParam.m_dx;
		rst->m_rcRect_I.top += stAlgoParam.m_dy;
		rst->m_rcRect_I.bottom += stAlgoParam.m_dy;
		rst->m_poDrawCenter.x += stAlgoParam.m_dx;	// part image 내 blob 중심 픽셀 좌표
		//rst->m_poDrawCenter.y = stAlgoParam.m_dInspH - (rst->m_poDrawCenter.y + stAlgoParam.m_dy);
		rst->m_poDrawCenter.y += stAlgoParam.m_dy;
		rst->m_poDrawCenterPix.x = rst->m_poDrawCenter.x;
		rst->m_poDrawCenterPix.y = rst->m_poDrawCenter.y;
		if (pAlgoBlob->m_nBlobSizeDistanceType == 1)
		{
			if (rst->m_pShiftSt.x != 0 || rst->m_pShiftSt.y != 0 || rst->m_pShiftEd.x != 0 || rst->m_pShiftEd.y != 0)
			{
				rst->m_pShiftSt.x += stAlgoParam.m_dx;
				rst->m_pShiftSt.y += stAlgoParam.m_dy;
				rst->m_pShiftEd.x += stAlgoParam.m_dx;
				rst->m_pShiftEd.y += stAlgoParam.m_dy;
			}
		}

		//MultiProcess 에선 절대 열지 말것
		CPoint rst_StPixel;
		double dPixel_StX = 0, dPixel_StY = 0;
		double part_x = pInspBoardInfo->partCx - pInspBoardInfo->partWidth / 2.;
		double part_y = pInspBoardInfo->partCy + pInspBoardInfo->partHeight / 2.;
		rst_StPixel = g_pInspMng->CvtBoradToPixel(pInspBoardInfo->fovCx, pInspBoardInfo->fovCy, part_x, part_y, pInspBoardInfo->partCx, pInspBoardInfo->partCy, dPixel_StX, dPixel_StY);

		double dPixel_CenterX = 0, dPixel_CenterY = 0;
		CPoint rst_CenterPixel = g_pInspMng->CvtBoradToPixel(pInspBoardInfo->fovCx, pInspBoardInfo->fovCy, pInspBoardInfo->partCx, pInspBoardInfo->partCy, pInspBoardInfo->partCx, pInspBoardInfo->partCy, dPixel_CenterX, dPixel_CenterY);
		//double dOffX_pixel = dPixel_CenterX  - (rst_StPixel.x + rst->m_poDrawCenter.x);
		//double dOffY_pixel = dPixel_CenterY  - (rst_StPixel.y + rst->m_poDrawCenter.y);

		double dOffX = rst_StPixel.x + rst->m_poDrawCenter.x;
		double dOffY = rst_StPixel.y + rst->m_poDrawCenter.y;

		//double dPixel_CenterXNew = dPixel_CenterX, dPixel_CenterYNew = dPixel_CenterY;
		//UndistortPoint(dPixel_CenterX, dPixel_CenterY, dPixel_CenterXNew, dPixel_CenterYNew);	// <- job
		POINT_64F ptCenter(dPixel_CenterX, dPixel_CenterY);
		POINT_64F ptCenter_New(dPixel_CenterX, dPixel_CenterY);
		if (g_pInspMng->m_CamInterpolation != nullptr && g_pMPTI->m_bUseLensDistortion)
			ptCenter_New = g_pInspMng->m_CamInterpolation->UndistortPoint(ptCenter);

		//double dNewOffX = dOffX, dNewOffY = dOffY;
		//UndistortPoint_Inv(dOffX, dOffY, dNewOffX, dNewOffY);	// <- inspect
		POINT_64F ptOff(dOffX, dOffY);
		POINT_64F ptOff_New(dOffX, dOffY);
		if (g_pInspMng->m_CamInterpolation != nullptr && g_pMPTI->m_bUseLensDistortion)
			ptOff_New = g_pInspMng->m_CamInterpolation->DistortPoint(ptOff);	// <- inspect

		double dBoardOffX = 0, dBoardOffY = 0;
		g_pInspMng->CvtPixelToBoard(pInspBoardInfo->fovCx, pInspBoardInfo->fovCy, (double)ptOff_New.x, (double)ptOff_New.y, &dBoardOffX, &dBoardOffY);

		if (g_pInspMng->IsAnyAngle(pInspBoardInfo->angle))	// 일반각일 경우 원래 각도로 좌표를 돌려준다
		{
			CProc pProc;
			pProc.CorrectCoordinate(dBoardOffX, dBoardOffY, pInspBoardInfo->partCx, pInspBoardInfo->partCy, pInspBoardInfo->angle, 0, 0, &dBoardOffX, &dBoardOffY);
		}

		rst->m_poDrawCenter.x = dBoardOffX;
		rst->m_poDrawCenter.y = dBoardOffY;

		if (rst->m_nArrRectCnt > 200)
			rst->m_nArrRectCnt = 200;

		rst->m_rcRectT.left += stAlgoParam.m_dx;
		rst->m_rcRectT.right += stAlgoParam.m_dx;
		rst->m_rcRectT.top += stAlgoParam.m_dy;
		rst->m_rcRectT.bottom += stAlgoParam.m_dy;
		for (int i = 0; i < rst->m_nArrRectCnt; ++i)
		{
			rst->m_rcArrRect[i].left += stAlgoParam.m_dx;
			rst->m_rcArrRect[i].right += stAlgoParam.m_dx;
			rst->m_rcArrRect[i].top += stAlgoParam.m_dy;
			rst->m_rcArrRect[i].bottom += stAlgoParam.m_dy;
		}
	}
	else
	{
		int nInspType = (stAlgoParam.m_bUseBlobOption) ? eINSP_S_BALL : 0;
		bool bUseLensDist = g_pMPTI->UseLensDistortion();

		bResult = TeachingBlob(sInspAlgo, *stAlgoParam.m_sPartAlgoImg, sWndAlgoImg, *sInspImageData, *stAlgoParam.m_coordinateAlgo, *stAlgoParam.m_sWndInfo, ucArrDstImg, (RstAlgoBlob *)sRstAlgo, stTieArea, nInspType, 0, stAlgoParam.m_ucArrDstImgAC, stAlgoParam.m_bTeach);

		RstAlgoBlob * rst = (RstAlgoBlob *)sRstAlgo;
		if (pAlgoBlob->m_bUseBlobNG && pAlgoBlob->m_bUsePattern && pAlgoBlob->m_bUseFPBW && bSwitch && stAlgoParam.m_bTeachingFunc)	//ForeignPattern TeachImg Invert
			g_pInspMng->InvertDisplay(ucArrDstImg, stAlgoParam.m_sPartAlgoImg->m_nWidth, stAlgoParam.m_sPartAlgoImg->m_nHeight, stAlgoParam.m_nStartX, stAlgoParam.m_nStartY, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, rst->m_pFPMatchPos);

		rst->m_poDrawCenterPix.x += stAlgoParam.m_nStartX;
		rst->m_poDrawCenterPix.y += stAlgoParam.m_nStartY;
		for (int i = 0; i < rst->m_nArrRectCnt; ++i)
		{
			rst->m_rcArrRect[i].left += stAlgoParam.m_nStartX;
			rst->m_rcArrRect[i].right += stAlgoParam.m_nStartX;
			rst->m_rcArrRect[i].top += stAlgoParam.m_nStartY;
			rst->m_rcArrRect[i].bottom += stAlgoParam.m_nStartY;
		}
	}

	return bResult;
}

void CPInsp_AlgoBlob::BlobDiagonalDistance(std::vector<CRect> blobRect, int cnt, std::vector<double>& rstD)
{
	for (int i = 0; i < cnt; i++)
	{
		double dSize_x = 0.0;
		double dSize_y = 0.0;
		if (blobRect[i].Width() > 0)
		{
			dSize_x = (blobRect[i].Width() - 1);
		}

		if (blobRect[i].Height() > 0)
		{
			dSize_y = (blobRect[i].Height() - 1);
		}

		double rstDist = sqrt((dSize_x * dSize_x) + (dSize_y * dSize_y));
		rstD.emplace_back(rstDist);
	}
}
BOOL CPInsp_AlgoBlob::InspBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, AlgoCoordinate* coordinateAlgo, double dPartW, double dPartH,
	RstAlgoBlob * sRstAlgo, TotalInspExceptArea stTieArea, int nInspType, int nOffX_pix, int nOffY_pix, POINTF poCenter, int nInspectionMode, UCHAR *pUcImgBlob)
{
	BOOL bResult = FALSE;

	int nLine = __LINE__;
	try
	{
		if (sRstAlgo)
		{
			memset(sRstAlgo, 0, (sizeof(RstAlgoBlob)));
			sRstAlgo->Init();
		}

		if (sInspAlgo.m_eAlgoType != eAlgoBlob)
			return bResult;
		if (!m_pProcMilAlgo)
			return bResult;
		AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
		if (!pAlgoBlob)
			return bResult;
		if (!pAlgoBlob->m_bInsp2D && !pAlgoBlob->m_bInsp3D && !pAlgoBlob->m_sAlgoColorBase.m_bUseColor)
			return bResult;

		int nSelectBlob = pAlgoBlob->m_nTypeSelectBlob;
		if (nSelectBlob < 0) nSelectBlob = 0;
		if (pAlgoBlob->m_bUseBlobNG || pAlgoBlob->m_bInspCoil)
		{
			if (nSelectBlob > eSelectMix)
				nSelectBlob = eSelectMix;
		}

		UCHAR *pUcImgSrc = sWndAlgoImg.m_ucArr2D;
		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;
		if ((nImgWidth <= 0) || (nImgHeight <= 0))
			return bResult;

		double dWndCX = coordinateAlgo->dROICenterX;
		double dWndCY = coordinateAlgo->dROICenterY;
		bool bInspSave = false;
		if (g_pMPTI)
		{
			bInspSave = g_pMPTI->m_LogLevel == m_eLogLv_Blob;
			if (bInspSave)
			{
				CString sLog = _T("");
				sLog.Format(_T("[Blob] [InspBlob] Color %d ColorAC %d AC %d _ %d"),
					pAlgoBlob->m_sAlgoColorBase.m_bUseColor, pAlgoBlob->m_sAlgoColorBase.m_bUseAngleColor,
					pAlgoBlob->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data], pAlgoBlob->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data2]);
				g_pMPTI->AddLog_Dev(sLog);
			}
		}
		nLine = __LINE__;

		// 통합 검사/제외 영역에 넣어준다
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
		stTieArea.dAngle = sWndAlgoImg.dAngle;

		double dCX = 0;
		double dCY = 0;
		double dArea = 0;
		int nMinBlobArea = pAlgoBlob->m_nMinBlobArea;
		//2020 0716 bjy : All blob & NG Blob일때   1 x 1 불량이 검출되어 주석처리
		//if(pAlgoBlob->m_nTypeSelectBlob == eSelectMix && pAlgoBlob->m_bUseBlobNG == TRUE)
		//	nMinBlobArea = 0;
		CRect rcBlob(0, 0, 0, 0);
		if (pUcImgBlob == NULL)
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pUcImgBlob, nImgWidth * nImgHeight);


		double dResolX = m_resolX;
		double dResolY = m_resolY;
		if (g_pMPTI->m_bSideOriginalSize == true)
		{
			dResolX = g_pMPTI->m_dBtmSideResX;
			dResolY = g_pMPTI->m_dBtmSideResY;
		}

		UCHAR *ucColorImgDst = NULL;
		CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
		if (pColorTeach != NULL && pAlgoBlob->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			pColorTeach->GetColorBaseBin(&pAlgoBlob->m_sAlgoColorBase, sInspImageData, ucColorImgDst, false, 0, 0, -1, sWndAlgoImg.m_nLight_index, NULL, bInspSave);
			m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, nImgWidth, nImgHeight, _T("OrgColor_Blob.bmp"), 1, false, bInspSave);
		}
		double dAreaPix = pAlgoBlob->m_dAreaCurrent / dResolX / dResolY;
		double dAreaWPix = pAlgoBlob->m_dTeachWidth / dResolX;
		double dAreaHPix = pAlgoBlob->m_dTeachLength / dResolY;

		double dTechCenterX = -1;
		double dTechCenterY = -1;

		double dWndAngle = sWndAlgoImg.dAngle;

		if (nSelectBlob == eSelectPosition)
		{
			double dCenTeachX = pAlgoBlob->m_dTechCenterX;
			double dCenTeachY = pAlgoBlob->m_dTechCenterY;
			if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
			{
				m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle,
					0, 0, &dCenTeachX, &dCenTeachY);
			}

			dTechCenterX = (dPartW / 2.) + (dCenTeachX / m_resolX);
			dTechCenterY = (dPartH / 2.) - (dCenTeachY / m_resolY);

			dTechCenterX -= (dWndCX - (nImgWidth / 2.));
			dTechCenterY -= (dWndCY - (nImgHeight / 2.));
		}

#if _DEBUG
		cv::Mat fImgSrc(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);
		cv::Mat imgpUcImgSrc(nImgHeight, nImgWidth, CV_8UC1, pUcImgSrc);
		//cv::Mat imgucColorImgDst(nImgHeight, nImgWidth, CV_8UC1, ucColorImgDst);
		cv::Mat imgpUcImgBlob(nImgHeight, nImgWidth, CV_8UC1, pUcImgBlob);
#endif
		int nCntBlob = m_pCPInsp_Algo->BlobImageStruct(*pAlgoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, pUcImgBlob, stTieArea, pAlgoBlob->m_bFillHole, 0, dTechCenterX, dTechCenterY, 0, dAreaPix, dAreaWPix, dAreaHPix);
		UCHAR* ucArrFillOutImg = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrFillOutImg, nImgWidth * nImgHeight);
		memset(ucArrFillOutImg, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		//Bin
		UCHAR*  ucBinImg = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucBinImg, nImgWidth * nImgHeight);
		memset(ucBinImg, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		//Mask2
		UCHAR*  ucMask = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucMask, nImgWidth * nImgHeight);
		memset(ucMask, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		int nFillOut = 0;
		nFillOut = m_pCPInsp_Algo->BlobFillOutImage(*pAlgoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, pUcImgBlob, stTieArea, pAlgoBlob->m_bFillHole, 0, -1, -1, 0, dAreaPix, dAreaWPix, dAreaHPix, 0, ucArrFillOutImg, ucBinImg, ucMask);
		if (nSelectBlob == eSimilarArea && nCntBlob == 0 && pAlgoBlob->m_bInsp2D)
		{
			for (int b = 0; b < 20; b++)
			{
				bool bOK = false;
				int nGapBinary = 0;
				int nMaxBinary = pAlgoBlob->m_nMaxBinary;
				for (int a = 0; a < 60; a++)
				{
					int nRstBinary = nMaxBinary;
					if (a % 2 == 0)
					{
						nGapBinary += 5;
						nRstBinary = nMaxBinary - nGapBinary;
					}
					else
						nRstBinary = nMaxBinary + nGapBinary;
					if (nRstBinary <= 0 || nRstBinary >= 255)
						continue;
					pAlgoBlob->m_nMaxBinary = nRstBinary;
					nCntBlob = m_pCPInsp_Algo->BlobImageStruct(*pAlgoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea,
						&dArea, &dCX, &dCY, &rcBlob, pUcImgBlob, stTieArea, pAlgoBlob->m_bFillHole, 0, -1, -1, 0, dAreaPix, dAreaWPix, dAreaHPix, b);
					if (nCntBlob == 0) continue;
					bOK = true;
					break;
				}
				if (bOK == true) break;
			}
		}
		Delete_1DArray(&ucColorImgDst);
		m_pProcMilAlgo->SaveWorkImg(pUcImgSrc, nImgWidth, nImgHeight, _T("Blob_2D.bmp"), 1, false, bInspSave);
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Blob_3D.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidth, nImgHeight, _T("Blob_Rst.bmp"), 1, false, bInspSave);

		UCHAR * ucArrBlobDst_except = NULL;
		double dArea_except(0.0f), dCX_except(0.0f), dCY_except(0.0f);
		CRect rcBlob_except;
		int nCntBlobExcept = 0;
		if (pAlgoBlob->m_bInsp2D_ExceptArea || pAlgoBlob->m_bInsp3D_ExceptArea)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobDst_except, nImgWidth * nImgHeight);
			memset(ucArrBlobDst_except, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
			AlgoBlob pAlgoBlob_except;
			pAlgoBlob_except.m_bInsp2D = pAlgoBlob->m_bInsp2D_ExceptArea;
			pAlgoBlob_except.m_nTypeRange2D = pAlgoBlob->m_nTypeRange2D_ExceptArea;
			pAlgoBlob_except.m_nMinBinary = pAlgoBlob->m_nMinBinary_ExceptArea;
			pAlgoBlob_except.m_nMaxBinary = pAlgoBlob->m_nMaxBinary_ExceptArea;
			pAlgoBlob_except.m_bInsp3D = pAlgoBlob->m_bInsp3D_ExceptArea;
			pAlgoBlob_except.m_nTypeRange3D = pAlgoBlob->m_nTypeRange3D_ExceptArea;
			pAlgoBlob_except.m_dHeightRateMin = pAlgoBlob->m_dHeightRateMin_ExceptArea;
			pAlgoBlob_except.m_dHeightRateMax = pAlgoBlob->m_dHeightRateMax_ExceptArea;
			pAlgoBlob_except.m_nTypeSelectBlob = eSelectMix;
			pAlgoBlob_except.m_nFilterStepNarrow = 0;
			pAlgoBlob_except.m_bFilterIsUse = FALSE;
			pAlgoBlob_except.m_bFillHole = FALSE;
			TotalInspExceptArea stTieArea_except;
			stTieArea_except.m_nUsedWndPolygon = 4;
			stTieArea_except.m_ptArrWndPolygon[0].x = rcBlob.left - (nImgWidth / 2);
			stTieArea_except.m_ptArrWndPolygon[0].y = rcBlob.top - (nImgHeight / 2);
			stTieArea_except.m_ptArrWndPolygon[1].x = rcBlob.right - (nImgWidth / 2);
			stTieArea_except.m_ptArrWndPolygon[1].y = rcBlob.top - (nImgHeight / 2);
			stTieArea_except.m_ptArrWndPolygon[2].x = rcBlob.right - (nImgWidth / 2);
			stTieArea_except.m_ptArrWndPolygon[2].y = rcBlob.bottom - (nImgHeight / 2);
			stTieArea_except.m_ptArrWndPolygon[3].x = rcBlob.left - (nImgWidth / 2);
			stTieArea_except.m_ptArrWndPolygon[3].y = rcBlob.bottom - (nImgHeight / 2);
			stTieArea_except.m_nUsedInspPolygon = 0;
			stTieArea_except.m_nUsedMaskingValue = 0;
			int nMinArea = ((int)dArea / 200);
			rcBlob_except.left = rcBlob_except.right = rcBlob_except.top = rcBlob_except.bottom = 0;
			nCntBlobExcept = m_pCPInsp_Algo->BlobImageStruct(pAlgoBlob_except, sWndAlgoImg.m_ucArr2D_Mix[1], pfImgSrc, NULL, nImgWidth, nImgHeight, nMinArea,
				&dArea_except, &dCX_except, &dCY_except, &rcBlob_except, ucArrBlobDst_except, stTieArea_except, pAlgoBlob->m_bFillHole, 0, -1, -1, 0, dAreaPix, dAreaWPix, dAreaHPix);
		}
		BlobNGImg Tmpbuf;
		double minVal = 0.0f, maxVal = 0.0f;
		RECT ThminRect, ThmaxRect;
		ThminRect.left = ThminRect.right = ThminRect.top = ThminRect.bottom = 0;
		ThmaxRect.left = ThmaxRect.right = ThmaxRect.top = ThmaxRect.bottom = 0;
		//ePartBodyRo_All = 0x00,
		//ePartBodyRo_Top = 0x01,
		//ePartBodyRo_Bottom = 0x02,
		//ePartBodyRo_Left = 0x04,
		//ePartBodyRo_Right = 0x08
		float fWndCenterX = (dWndCX - dPartW / 2);
		float fWndCenterY = (dWndCY - dPartH / 2);
		float fWndWidth = (nImgWidth / 2);
		float fWndLength = (nImgHeight / 2);
		if ((pAlgoBlob->m_bUseThickMax || pAlgoBlob->m_bUseThickMin) && sRstAlgo && nCntBlob > 0 && dArea > 5)
		{
			bool bTeach = false;
			nCntBlob = InspThickness2(pAlgoBlob, pUcImgSrc, pUcImgBlob, ucArrBlobDst_except, nImgWidth, nImgHeight, nCntBlobExcept, dArea, dArea_except, rcBlob, fWndCenterX, fWndCenterY, fWndWidth, fWndLength, ThminRect, ThmaxRect, minVal, maxVal, sRstAlgo,bTeach);
		}

		double dCircleX = 0;
		double dCircleY = 0;
		double dCircleR = 0;
		double dCircleErr = 0;
		if (nCntBlob == 1 && pAlgoBlob->m_bUseBlobNG == FALSE && pAlgoBlob->m_bInspCoil == FALSE && bInspSave)
		{
			int nFindPer = 80;
			double ini_radius = (((pAlgoBlob->m_dTeachWidth / dResolX) + (pAlgoBlob->m_dTeachLength / dResolY)) / 2.0) / 2.0;
			float fMin = pAlgoBlob->m_dTeachWidthRateMin;
			float fMax = pAlgoBlob->m_dTeachWidthRateMax;
			m_pCPInsp_Algo->CircleIntersection(pUcImgBlob, nImgWidth, nImgHeight, &dCircleX, &dCircleY, &dCircleR, &dCircleErr, ini_radius, nFindPer, fMin, fMax);
		}

		//jhj
		int nWinowIndex = g_pInspMng->winid + 1;
		if (BlobNgImageSave == TRUE && nInspectionMode == 0)
		{
			if (pAlgoBlob->m_bUseBlobNG)
			{
				std::vector<BlobNGImg> tempBuff;

				cv::Mat GrayImg2(nImgHeight, nImgWidth, CV_8UC1, pUcImgSrc);
				cv::Mat BinImg2(nImgHeight, nImgWidth, CV_8UC1, ucArrFillOutImg);
				cv::Mat GrayImg = GrayImg2.clone();
				cv::Mat BinImg = BinImg2.clone();

				int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImg2.ptr(0), BinImg.ptr(0), BinImg2.cols, BinImg2.rows, nMinBlobArea, 0, pAlgoBlob->m_bFillHole, pAlgoBlob->m_bFilterIsUse ? pAlgoBlob->m_nFilterStepNarrow : 0, nSelectBlob, -1, -1, -1, dAreaPix, dAreaWPix, dAreaHPix, 0);

				Tmpbuf.mGrayImg = GrayImg;
				Tmpbuf.mBinImg = BinImg;
				CString sBinImgPath, sGrayImgPath, sMaskImgPath;
				sBinImgPath.Format(_T("%s\\Bin%s@%s@%s@%d@%d.bmp"), BlobImgPath, m_sModuleID, m_sPartCode, m_sRefID, nWinowIndex, sInspAlgo.m_nAlgoId);
				sGrayImgPath.Format(_T("%s\\Gray%s@%s@%s@%d@%d.bmp"), BlobImgPath, m_sModuleID, m_sPartCode, m_sRefID, nWinowIndex, sInspAlgo.m_nAlgoId);

				if (nFillOut == 1)
				{
					cv::Mat MaskImg2(nImgHeight, nImgWidth, CV_8UC1, ucMask);//ucArrFillOutImg);
					cv::Mat MaskImg = MaskImg2.clone();
					Tmpbuf.mMask = MaskImg;
					sMaskImgPath.Format(_T("%s\\Mask%s@%s@%s@%d@%d.bmp"), BlobImgPath, m_sModuleID, m_sPartCode, m_sRefID, nWinowIndex, sInspAlgo.m_nAlgoId);
					Tmpbuf.sMask = sMaskImgPath;
					//cv::imwrite(std::string(CT2A(sMaskImgPath)), MaskImg);
				}

				Tmpbuf.sGray = sGrayImgPath;
				Tmpbuf.sBinary = sBinImgPath;
				//vBlobNGImg.push_back(Tmpbuf);

				//cv::imwrite(std::string(CT2A(sBinImgPath)),BinImg);
				//cv::imwrite(std::string(CT2A(sGrayImgPath)),GrayImg);
				//vBlobNGImg.clear();
			}
		}
		cv::Rect rtFPArea;
		if (pAlgoBlob->m_bUsePattern && pAlgoBlob->m_bUseFPBW)
		{
			nCntBlob = 0;
			BOOL IsBinImage = FALSE;

			int nTmpTypeSelectBlob = nSelectBlob;
			pAlgoBlob->m_nTypeSelectBlob = eSelectMix;
			nCntBlob = m_pCPInsp_Algo->BlobImageStruct(*pAlgoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, pUcImgBlob, stTieArea, pAlgoBlob->m_bFillHole);
			Delete_1DArray(&ucColorImgDst);
			ucColorImgDst = NULL;
			m_pProcMilAlgo->SaveWorkImg(pUcImgSrc, nImgWidth, nImgHeight, _T("Blob_2D_Allblob.bmp"));
			m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Blob_3D_Allblob.bmp"));
			m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidth, nImgHeight, _T("Blob_Rst_Allblob.bmp"), 1, false, bInspSave);

			POINTF centerPtr;
			bool bSwitch = g_pMPTI->m_bLengthByAngel;

			if (bSwitch == TRUE)
			{
				centerPtr = CalcForiegnPatternCenter(sInspAlgo, sWndAlgoImg, sInspImageData, dWndCX, dWndCY, dPartW, dPartH, stTieArea, 0, rtFPArea);
				g_pInspMng->FP_CenterPtr = centerPtr;
			}

			g_pInspMng->ForeignPatternSet(sInspAlgo, pUcImgBlob, nImgWidth, nImgHeight, sWndAlgoImg.dAngle, 0, sRstAlgo->m_pFPMatchPos);
			m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidth, nImgHeight, _T("Blob_FP.bmp"), 1, false, bInspSave);

			//blob setting
			nSelectBlob = nTmpTypeSelectBlob;
			int nFilter = 0;
			if (pAlgoBlob->m_bFilterIsUse)
				nFilter = pAlgoBlob->m_nFilterStepNarrow;
			pAlgoBlob->m_nTypeRange2D = 2;
			pAlgoBlob->m_nMinBinary = 0;
			pAlgoBlob->m_nMaxBinary = 200;
			pAlgoBlob->m_bInsp3D = false;
			pAlgoBlob->m_bInsp2D = true;

			dCX = 0, dCY = 0, dArea = 0;
			rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
			nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pUcImgBlob, pUcImgBlob, nImgWidth, nImgHeight, nMinBlobArea, FALSE, pAlgoBlob->m_bFillHole, nFilter, nSelectBlob);

			if (nCntBlob > 0)
			{
				m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
				m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidth, nImgHeight, _T("Blob_FP_Blob.bmp"), 1, false, bInspSave);
			}

			pUcImgSrc = pUcImgBlob;
			// 			if(pColorTeach != NULL && pAlgoBlob->m_sAlgoColorBase.m_bUseColor == TRUE)
			// 				memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			IsBinImage = TRUE;
			nCntBlob = m_pCPInsp_Algo->BlobImageStruct(*pAlgoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, pUcImgBlob, stTieArea, pAlgoBlob->m_bFillHole, 0, -1, -1, IsBinImage, dAreaPix, dAreaWPix, dAreaHPix);

			//Delete_1DArray(&ucColorImgDst);
			m_pProcMilAlgo->SaveWorkImg(pUcImgSrc, nImgWidth, nImgHeight, _T("Blob_2D.bmp"), 1, false, bInspSave);
			m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Blob_3D.bmp"));
			m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidth, nImgHeight, _T("Blob_Rst.bmp"), 1, false, bInspSave);
		}
		// 평균 높이 산출.
		double dHeightMeanBody = 0.0;
		float fPinHeight = 0.0f;
		float fPinAngle = 0.0f;
		if (sRstAlgo && nCntBlob > 0 && rcBlob.Width() > 0 && rcBlob.Height() > 0)
		{
			nLine = __LINE__;

			int nWidthBolb = rcBlob.Width();
			int nHeightBlob = rcBlob.Height();
			float *pfImgDst = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pfImgDst, nWidthBolb * nHeightBlob);

			m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst, nImgWidth, nImgHeight, rcBlob.left, rcBlob.top, nWidthBolb, nHeightBlob);
			m_pProcMilAlgo->SaveWorkImg_float(pfImgDst, nWidthBolb, nHeightBlob, _T("BodyBlob3D.bmp"));

			BOOL bCalcAvgMask = pAlgoBlob->m_bUseHeightAvgMask;
			if (bCalcAvgMask == FALSE)
				dHeightMeanBody = m_pCPInsp_Algo->GetHeightMean(pfImgDst, nWidthBolb, nHeightBlob);
			else
				dHeightMeanBody = m_pCPInsp_Algo->GetHeightAvgMask(pfImgSrc, pUcImgBlob, nImgWidth, nImgHeight, rcBlob.left, rcBlob.top, nWidthBolb, nHeightBlob);

			if (nCntBlob > 0 && pAlgoBlob->m_bUsePinAngle)
			{
				cv::Mat AngleHeight(nHeightBlob, nWidthBolb, CV_32FC1, pfImgDst);
				double dMin, dMax;
				cv::Point poMin, poMax;
				cv::minMaxLoc(AngleHeight, &dMin, &dMax, &poMin, &poMax);
				float poX = rcBlob.left + poMax.x;
				float poY = rcBlob.top + poMax.y;
				double dCenTeachX = pAlgoBlob->m_dTechCenterX;
				double dCenTeachY = pAlgoBlob->m_dTechCenterY;
				double rcBlob_left, rcBlob_right, rcBlob_top, rcBlob_bottom;
				if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
				{
					m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle,
						0, 0, &dCenTeachX, &dCenTeachY);
					m_proc3d.CorrectCoordinate(rcBlob_left, rcBlob_top, 0, 0, dWndAngle,
						0, 0, &rcBlob_left, &rcBlob_top);
					m_proc3d.CorrectCoordinate(rcBlob_right, rcBlob_bottom, 0, 0, dWndAngle,
						0, 0, &rcBlob_right, &rcBlob_bottom);
					if (rcBlob_bottom < rcBlob_top)
					{
						double dtmp = rcBlob_bottom;
						rcBlob_bottom = rcBlob_top;
						rcBlob_top = dtmp;
					}
					if (rcBlob_right < rcBlob_left)
					{
						double dtmp = rcBlob_right;
						rcBlob_right = rcBlob_left;
						rcBlob_left = dtmp;
					}
					if ((dWndAngle == 180) || (dWndAngle == 270))
					{
						double dtmp = rcBlob_bottom;
						rcBlob_bottom = rcBlob_right;
						rcBlob_right = dtmp;

						dtmp = rcBlob_top;
						rcBlob_top = rcBlob_left;
						rcBlob_left = dtmp;
					}
				}

				double dTechCenterX = (dPartW / 2.) + (dCenTeachX / m_resolX);
				double dTechCenterY = (dPartH / 2.) - (dCenTeachY / m_resolY);

				dTechCenterX -= (dWndCX - (nImgWidth / 2.));
				dTechCenterY -= (dWndCY - (nImgHeight / 2.));

				float fW = pAlgoBlob->m_dTeachWidth / 2 / dResolX;
				float fH = pAlgoBlob->m_dTeachLength / 2 / dResolY;
				//shiftX
				if (dCX - dTechCenterX < 0)//left
				{
					poX = rcBlob.left + fW;
				}
				else if (dCX - dTechCenterX > 0)//right
				{
					poX = rcBlob.right - fW;
				}
				//shiftY
				if (dCY - dTechCenterY > 0)//top
				{
					poY = rcBlob.bottom - fH;
				}
				else if (dCY - dTechCenterY < 0)//bottom
				{
					poY = rcBlob.top + fH;
				}
				float fShX = std::abs(dTechCenterX - poX) * m_resolX;
				float fShY = std::abs(dTechCenterY - poY) * m_resolY;
				float fSh = std::sqrtf(std::powf(fShX, 2) + std::powf(fShY, 2));
#if _DEBUG
				cv::Mat srcImg(nImgHeight, nImgWidth, CV_8UC1, pUcImgBlob);
				cv::Mat debugImg;
				cv::cvtColor(srcImg, debugImg, cv::COLOR_GRAY2BGR);
				cv::line(debugImg, cv::Point(poX - 1, poY), cv::Point(poX + 1, poY), cv::Scalar(0, 0, 255));
				cv::line(debugImg, cv::Point(poX, poY - 1), cv::Point(poX, poY + 1), cv::Scalar(0, 0, 255));
				cv::rectangle(debugImg, cv::Point(poX - fW, poY - fH), cv::Point(poX + fW, poY + fH), cv::Scalar(0, 0, 255));
				cv::rectangle(debugImg, cv::Point(dTechCenterX - fW, dTechCenterY - fH), cv::Point(dTechCenterX + fW, dTechCenterY + fH), cv::Scalar(255, 0, 0));
				cv::rectangle(debugImg, cv::Point(rcBlob.left, rcBlob.top), cv::Point(rcBlob.right, rcBlob.bottom), cv::Scalar(0, 255, 0));
				cv::imwrite("D:\\PintDebug.bmp",debugImg);
#endif
				int nMaxCnt(0);
				float fMax(0.0f);
				int rst = poMax.y - 1 > 0 ? poMax.y - 1 : 0;
				int red = poMax.y + 1 <= AngleHeight.rows ? poMax.y + 1 : AngleHeight.rows;
				int cst = poMax.x - 1 > 0 ? poMax.x - 1 : 0;
				int ced = poMax.x + 1 <= AngleHeight.cols ? poMax.x + 1 : AngleHeight.cols;
				for (int r = rst; r < red; r++)
				{
					float* fPtr = AngleHeight.ptr<float>(r);
					for (int c = cst; c < ced; c++)
					{
						fMax += fPtr[c];
						nMaxCnt++;
					}
				}
				fMax /= nMaxCnt;
				fPinHeight = pAlgoBlob->m_fPinHeight/1000;
				if (fPinHeight == 0 )
				{
					fPinAngle = 90;
				}
				else if (fPinHeight < fSh)
				{
					fPinAngle = 0;
				}
				else
				{
					float fRad = std::acosf(fSh / fPinHeight);
					fPinAngle = fRad * 180 / PI;
				}
			}
			Delete_1DArray(&pfImgDst);
			sRstAlgo->m_rcRect_I.left = rcBlob.left;
			sRstAlgo->m_rcRect_I.right = rcBlob.right;
			sRstAlgo->m_rcRect_I.top = rcBlob.top;
			sRstAlgo->m_rcRect_I.bottom = rcBlob.bottom;
			sRstAlgo->m_dRstHeightMean = dHeightMeanBody;
			sRstAlgo->m_nArrRectCnt = 0;

			if (pAlgoBlob->m_byNGCnt > 0 && pAlgoBlob->m_bUseBlobNG)
			{
				double *dArrA = NULL;
				CRect *cArrR = NULL;
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrA, nCntBlob);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &cArrR, nCntBlob);
				m_pProcMilAlgo->GetBlobResult_ALL(dArrA, NULL, NULL, cArrR);
				int nCnt = (nCntBlob > BLOB_RECT_CNTS) ? BLOB_RECT_CNTS : nCntBlob;
				for (int a = 0; a < nCnt; a++)
				{
					sRstAlgo->m_fArrRstA[a] = dArrA[a] * m_resolX * m_resolY;
					sRstAlgo->m_rcArrRect[a].left = cArrR[a].left;
					sRstAlgo->m_rcArrRect[a].right = cArrR[a].right;
					sRstAlgo->m_rcArrRect[a].top = cArrR[a].top;
					sRstAlgo->m_rcArrRect[a].bottom = cArrR[a].bottom;
					sRstAlgo->m_fArrRstW[a] = (sRstAlgo->m_rcArrRect[a].right - sRstAlgo->m_rcArrRect[a].left) * m_resolX;
					sRstAlgo->m_fArrRstL[a] = (sRstAlgo->m_rcArrRect[a].bottom - sRstAlgo->m_rcArrRect[a].top) * m_resolY;
					sRstAlgo->m_nArrRectCnt++;
				}
				Delete_1DArray(&dArrA);
				Delete_1DArray(&cArrR);
			}
			}
		Delete_1DArray(&ucArrBlobDst_except);
		Delete_1DArray(&ucArrFillOutImg);
		Delete_1DArray(&ucBinImg);
		Delete_1DArray(&ucMask);

		nLine = __LINE__;

		//2019.11.25 KYH 코일검사모드 추가
		eBlobInspMode BlobInspMode = (pAlgoBlob->m_bUseBlobNG) ? eBlobInspMode_Solderball : (pAlgoBlob->m_bInspCoil) ? eBlobInspMode_Coil : eBlobInspMode_Origin;
		////2019.11.18 KYH 코일검사모드

		int nInspAC = pAlgoBlob->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data];
		bool bInspAC = ((nInspAC & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
		bool bInspAC_Ess = (bInspAC && ((nInspAC & eDefaultAC_Data_Essential) == eDefaultAC_Data_Essential));
		bool bInspAC_WA = (bInspAC && ((nInspAC & eDefaultAC_Data_WA) == eDefaultAC_Data_WA));
		sRstAlgo->m_bOKArea_Reverse = TRUE;
		sRstAlgo->m_sInspAC.m_nOKInspAC = 0;
		sRstAlgo->m_bOKShiftX = TRUE;
		sRstAlgo->m_bOKShiftY = TRUE;
		sRstAlgo->m_bOKWidth = TRUE;
		sRstAlgo->m_bOKLength = TRUE;
		sRstAlgo->m_bOKDistance = TRUE;
		sRstAlgo->m_bOKArea = TRUE;
		sRstAlgo->m_bOKCoilRst = TRUE;
		sRstAlgo->m_bOKThickMax = TRUE;
		sRstAlgo->m_bOKThickMin = TRUE;
		sRstAlgo->m_bOKOffsetDistance = TRUE;
		if (BlobInspMode == eBlobInspMode_Origin)
		{
			if ((nCntBlob < 1) || (dArea < 1) || (rcBlob.Width() <= 0) || (rcBlob.Height() <= 0))
			{
				sRstAlgo->m_bOKArea_Reverse = FALSE;
				sRstAlgo->m_sInspAC.m_nOKInspAC = 0;
				sRstAlgo->m_bOKShiftX = FALSE;
				sRstAlgo->m_bOKShiftY = FALSE;
				sRstAlgo->m_bOKWidth = FALSE;
				sRstAlgo->m_bOKLength = FALSE;
				sRstAlgo->m_bOKDistance = FALSE;
				sRstAlgo->m_bOKArea = FALSE;
				sRstAlgo->m_bOKCoilRst = FALSE;
				sRstAlgo->m_bOKThickMax = FALSE;
				sRstAlgo->m_bOKThickMin = FALSE;
				sRstAlgo->m_bOKOffsetDistance = FALSE;

				//Mars Type이면 윈도우 Rect 추가
				if (bResult == FALSE && g_pMPTI->IsMachineTypeMars())
				{
					sRstAlgo->m_nArrRectCnt++;
					sRstAlgo->m_rcArrRect[0].left = 1;
					sRstAlgo->m_rcArrRect[0].top = 1;
					sRstAlgo->m_rcArrRect[0].right = nImgWidth - 2;
					sRstAlgo->m_rcArrRect[0].bottom = nImgHeight - 2;
				}

				nLine = __LINE__;
				if (pAlgoBlob->m_bUseThickMax)
				{
					sRstAlgo->m_fRstThickMax = maxVal * dResolX;
					if (sRstAlgo->m_fRstThickMax > pAlgoBlob->m_fThickMax)
					{
						sRstAlgo->m_bOKThickMax = FALSE;
					}
					else
					{
						bResult = sRstAlgo->m_bOKThickMax = TRUE;
					}
					sRstAlgo->m_nArrRectCnt = 0;
					if (sRstAlgo->m_nArrRectCnt < 200)
					{
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt] = ThmaxRect;
						sRstAlgo->m_nArrRectCnt++;
					}
				}
				nLine = __LINE__;
				if (pAlgoBlob->m_bUseThickMin)
				{
					sRstAlgo->m_fRstThickMin = minVal * dResolX;
					if (sRstAlgo->m_fRstThickMin < pAlgoBlob->m_fThickMin)
					{
						sRstAlgo->m_bOKThickMin = FALSE;
					}
					else
					{
						sRstAlgo->m_bOKThickMin = TRUE;
					}
					if (!pAlgoBlob->m_bUseThickMax)
						sRstAlgo->m_nArrRectCnt = 0;
					if (sRstAlgo->m_nArrRectCnt < 200)
					{
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt] = ThminRect;
						sRstAlgo->m_nArrRectCnt++;
					}
					bResult &= sRstAlgo->m_bOKThickMin;
				}


				Delete_1DArray(&pUcImgBlob);
				Delete_1DArray(&ucColorImgDst);
				return bResult;
			}

			nLine = __LINE__;

			m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidth, nImgHeight, _T("ClipBlob_O.bmp"), 1, false, bInspSave);
			double rstW = 0.0;
			double rstH = 0.0;
			double dWndAngle = sWndAlgoImg.dAngle;
			m_pCPInsp_Algo->GetBlobArea(pUcImgBlob, nImgWidth, nImgHeight, dCX, dCY, rcBlob.Width(), rcBlob.Height(), dWndAngle, &rstW, &rstH);
			UCHAR * pucACDst = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucACDst, nImgWidth * nImgHeight * 3);
			memset(pucACDst, 0, nImgWidth * nImgHeight * 3 * sizeof(UCHAR));
			sRstAlgo->m_sInspAC.m_nInspAC = InspAC(pAlgoBlob->m_sAngleColorBase, sInspImageData, pUcImgBlob, sWndAlgoImg, pAlgoBlob->m_byDir, pucACDst, sRstAlgo->m_sInspAC.m_fRstInspAC, &sRstAlgo->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ], false, poCenter);
			Delete_1DArray(&pucACDst);
			nLine = __LINE__;

			Delete_1DArray(&pUcImgBlob);
			if (rstW <= 0)
				rstW = rcBlob.Width();
			if (rstH <= 0)
				rstH = rcBlob.Height();

			double cogX_roi_pixel = 0.0;
			double cogY_roi_pixel = 0.0;

			if (sRstAlgo)
			{
				double dPartRstX = coordinateAlgo->dWndX + dCX - nOffX_pix;
				double dPartRstY = coordinateAlgo->dWndY + dCY - nOffY_pix;
				cogX_roi_pixel = dPartRstX * dResolX;
				cogY_roi_pixel = dPartRstY * dResolY;

				/////
				double dCenTeachX = pAlgoBlob->m_dTechCenterX;
				double dCenTeachY = pAlgoBlob->m_dTechCenterY;

				nLine = __LINE__;

				if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
				{
					m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle,
						0, 0, &dCenTeachX, &dCenTeachY);
				}

				nLine = __LINE__;

				sRstAlgo->m_dRstArea = dArea * dResolX * dResolY;

				if (pAlgoBlob->m_bAreaIsUse)
				{
					double minRange = pAlgoBlob->m_dAreaCurrent * pAlgoBlob->m_dAreaMin / 100.;
					double maxRange = pAlgoBlob->m_dAreaCurrent * pAlgoBlob->m_dAreaMax / 100.;
					if (sRstAlgo->m_dRstArea >= minRange && sRstAlgo->m_dRstArea <= maxRange)
						sRstAlgo->m_bOKArea = TRUE;
					else
						sRstAlgo->m_bOKArea = FALSE;
				}
				else
					sRstAlgo->m_bOKArea = TRUE;

				if (pAlgoBlob->m_bShiftIsUse)
				{
					double dShiftX = pAlgoBlob->m_dShiftX;
					double dShiftY = pAlgoBlob->m_dShiftY;
					bool bShiftXUse = pAlgoBlob->m_bShiftXUse;
					bool bShiftYUse = pAlgoBlob->m_bShiftYUse;
					if (dWndAngle == 90 || dWndAngle == 270)
					{
						dShiftX = pAlgoBlob->m_dShiftY;
						dShiftY = pAlgoBlob->m_dShiftX;
						bShiftXUse = pAlgoBlob->m_bShiftYUse;
						bShiftYUse = pAlgoBlob->m_bShiftXUse;
					}
					sRstAlgo->m_dRstShiftX = 0.;
					sRstAlgo->m_dRstShiftY = 0.;

					sRstAlgo->m_bOKShiftX = TRUE;
					sRstAlgo->m_bOKShiftY = TRUE;
					sRstAlgo->m_poDrawCenter.x = dCX;	// window image 내 blob 중심 픽셀 좌표
					sRstAlgo->m_poDrawCenter.y = dCY;
					sRstAlgo->m_poDrawCenterPix.x = dCX;
					sRstAlgo->m_poDrawCenterPix.y = dCY;

					double dErrX = sWndAlgoImg.m_fPartRoundingErrX * dResolX;
					double dErrY = sWndAlgoImg.m_fPartRoundingErrY * dResolY;
					double dErrWNDX = sWndAlgoImg.m_fWndRoundingErrX * dResolX;
					double dErrWNDY = sWndAlgoImg.m_fWndRoundingErrY * dResolY;
					double dRstX = cogX_roi_pixel - (coordinateAlgo->dPartW / 2.0) + dErrX + dErrWNDX;
					double dRstY = (coordinateAlgo->dPartH / 2.0) - cogY_roi_pixel - dErrY - dErrWNDY;
					if (bShiftXUse)
					{
						sRstAlgo->m_dRstShiftX = (dRstX - dCenTeachX) * -1;
						if (fabs(sRstAlgo->m_dRstShiftX) <= dShiftX)
							sRstAlgo->m_bOKShiftX = TRUE;
						else
							sRstAlgo->m_bOKShiftX = FALSE;
					}
					if (bShiftYUse)
					{
						sRstAlgo->m_dRstShiftY = (dRstY - dCenTeachY);
						if (fabs(sRstAlgo->m_dRstShiftY) <= dShiftY)
							sRstAlgo->m_bOKShiftY = TRUE;
						else
							sRstAlgo->m_bOKShiftY = FALSE;
					}
					if (bInspSave)
					{
						double dTeachX = dCenTeachX + (coordinateAlgo->dPartW / 2.0) - dErrX;
						double dTeachY = (dCenTeachY - (coordinateAlgo->dPartH / 2.0) + dErrY) * -1.0;
						double dTeachXPix = dTeachX / dResolX;
						double dTeachYPix = dTeachY / dResolY;
						double dTeachXWnd = dTeachXPix - coordinateAlgo->dWndX;
						double dTeachYWnd = dTeachYPix - coordinateAlgo->dWndY;
						double dGapXpix = dCX - dTeachXWnd;
						double dGapYpix = dCY - dTeachYWnd;
						double dCirXpix = dCircleX - dTeachXWnd;
						double dCirYpix = dCircleY - dTeachYWnd;
						double dCirX = dCirXpix * dResolX;
						double dCirY = dCirYpix * dResolY;

						CString sLog = _T("");
						sLog.Format(_T("[BLOB][InspBlob3] [X]\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f"),
							coordinateAlgo->dWndX, dCX, dPartRstX, sWndAlgoImg.m_fWndRoundingErrX, sWndAlgoImg.m_fPartRoundingErrX, dResolX,
							cogX_roi_pixel, coordinateAlgo->dPartW, dRstX, dCenTeachX, sRstAlgo->m_dRstShiftX,
							dTeachX, dTeachXPix, dTeachXWnd, dGapXpix, dCircleX, dCirXpix, dCirX);
						g_pMPTI->AddLog_Dev(sLog);
						sLog.Format(_T("[BLOB][InspBlob3] [Y]\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f"),
							coordinateAlgo->dWndY, dCY, dPartRstY, sWndAlgoImg.m_fWndRoundingErrY, sWndAlgoImg.m_fPartRoundingErrY, dResolY,
							cogY_roi_pixel, coordinateAlgo->dPartH, dRstY, dCenTeachY, sRstAlgo->m_dRstShiftY,
							dTeachY, dTeachYPix, dTeachYWnd, dGapYpix, dCircleY, dCirYpix, dCirY);
						g_pMPTI->AddLog_Dev(sLog);
					}
				}
				else
				{
					sRstAlgo->m_dRstShiftX = 0.;
					sRstAlgo->m_dRstShiftY = 0.;

					sRstAlgo->m_bOKShiftX = TRUE;
					sRstAlgo->m_bOKShiftY = TRUE;
				}

				if (pAlgoBlob->m_bUseOffsetDistance)
				{
					sRstAlgo->m_bOKOffsetDistance = FALSE;
					double dOffsetDistanceX = ((cogX_roi_pixel*dResolX - (float)dPartW / 2 * dResolX) - dCenTeachX);
					double dOffsetDistanceY = (((float)dPartH / 2 * dResolY - cogY_roi_pixel * dResolY) - dCenTeachY);
					sRstAlgo->m_fOffsetDistance = sqrt(pow(dOffsetDistanceX, 2.0) + pow(dOffsetDistanceY, 2.0));
					if (pAlgoBlob->m_fOffsetDistance > sRstAlgo->m_fOffsetDistance)
						sRstAlgo->m_bOKOffsetDistance = TRUE;
				}
				else
				{
					sRstAlgo->m_fOffsetDistance = 0.0f;
					sRstAlgo->m_bOKOffsetDistance = TRUE;
				}

				if (pAlgoBlob->m_bTeachWidthUse)
				{
					sRstAlgo->m_dRstWidth = rstW * dResolX;
					if (sRstAlgo->m_dRstWidth >= pAlgoBlob->m_dTeachWidth * pAlgoBlob->m_dTeachWidthRateMin / 100. && sRstAlgo->m_dRstWidth <= pAlgoBlob->m_dTeachWidth * pAlgoBlob->m_dTeachWidthRateMax / 100.)
						sRstAlgo->m_bOKWidth = TRUE;
					else
						sRstAlgo->m_bOKWidth = FALSE;
				}
				else
					sRstAlgo->m_bOKWidth = TRUE;

				if (pAlgoBlob->m_bTeachLengthUse)
				{
					sRstAlgo->m_dRstLength = rstH * dResolY;
					if (sRstAlgo->m_dRstLength >= pAlgoBlob->m_dTeachLength * pAlgoBlob->m_dTeachLengthRateMin / 100. && sRstAlgo->m_dRstLength <= pAlgoBlob->m_dTeachLength * pAlgoBlob->m_dTeachLengthRateMax / 100.)
						sRstAlgo->m_bOKLength = TRUE;
					else
						sRstAlgo->m_bOKLength = FALSE;
				}
				else
					sRstAlgo->m_bOKLength = TRUE;

				if (pAlgoBlob->m_bUseHeight)
				{
					if ((sRstAlgo->m_dRstHeightMean >= pAlgoBlob->m_dTeachHeightMin)
						&& (sRstAlgo->m_dRstHeightMean <= pAlgoBlob->m_dTeachHeightMax))
						sRstAlgo->m_bOKHeight = TRUE;
					else
						sRstAlgo->m_bOKHeight = FALSE;
				}
				else
					sRstAlgo->m_bOKHeight = TRUE;

				sRstAlgo->m_sInspAC.SetRstData(pAlgoBlob->m_sAngleColorBase);

				double dDstX = (dCenTeachX / m_resolX);
				double dDstY = (dCenTeachY / m_resolY);
				int nCX = (int)(dDstX + (dPartW / 2.) - dWndCX + (nImgWidth / 2.));
				int nCY = (int)((dPartH / 2.) - dDstY - dWndCY + (nImgHeight / 2.));
				int nRstW = (dWndAngle == 90 || dWndAngle == 270) ? dAreaHPix : dAreaWPix;
				int nRstH = (dWndAngle == 90 || dWndAngle == 270) ? dAreaWPix : dAreaHPix;
				sRstAlgo->m_rcRectT.left = nCX - (nRstW / 2);
				sRstAlgo->m_rcRectT.right = nCX + (nRstW / 2);
				sRstAlgo->m_rcRectT.top = nCY - (nRstH / 2);
				sRstAlgo->m_rcRectT.bottom = nCY + (nRstH / 2);

				nCX = dCX - nOffX_pix;
				nCY = dCY - nOffY_pix;
				nRstW = (dWndAngle == 90 || dWndAngle == 270) ? rstH : rstW;
				nRstH = (dWndAngle == 90 || dWndAngle == 270) ? rstW : rstH;
				if (BLOB_RECT_CNTS - 1 > sRstAlgo->m_nArrRectCnt)
				{
					sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].left = nCX - (nRstW / 2);
					sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].right = nCX + (nRstW / 2);
					sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].top = nCY - (nRstH / 2);
					sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].bottom = nCY + (nRstH / 2);
					sRstAlgo->m_nArrRectCnt++;
				}

				bResult = TRUE;

				nLine = __LINE__;
				if (pAlgoBlob->m_bUseThickMax)
				{
					sRstAlgo->m_fRstThickMax = maxVal * dResolX;
					if (sRstAlgo->m_fRstThickMax > pAlgoBlob->m_fThickMax)
					{
						sRstAlgo->m_bOKThickMax = FALSE;
					}
					else
					{
						sRstAlgo->m_bOKThickMax = TRUE;
					}
					sRstAlgo->m_nArrRectCnt = 0;
					if (sRstAlgo->m_nArrRectCnt < 200)
					{
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt] = ThmaxRect;
						sRstAlgo->m_nArrRectCnt++;
					}
				}
				nLine = __LINE__;
				if (pAlgoBlob->m_bUseThickMin)
				{
					sRstAlgo->m_fRstThickMin = minVal * dResolX;
					if (sRstAlgo->m_fRstThickMin < pAlgoBlob->m_fThickMin)
					{
						sRstAlgo->m_bOKThickMin = FALSE;
					}
					else
					{
						sRstAlgo->m_bOKThickMin = TRUE;
					}
					if (!pAlgoBlob->m_bUseThickMax)
						sRstAlgo->m_nArrRectCnt = 0;
					if (sRstAlgo->m_nArrRectCnt < 200)
					{
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt] = ThminRect;
						sRstAlgo->m_nArrRectCnt++;
					}
				}

				if (pAlgoBlob->m_bUsePinAngle)
				{
					sRstAlgo->m_fRstPinAngle = fPinAngle;
					sRstAlgo->m_fRstPinHeight = fPinHeight;
					if (std::abs(sRstAlgo->m_fRstPinAngle - pAlgoBlob->m_fPinAngle) <= pAlgoBlob->m_fPinAngleRange)
						sRstAlgo->m_bOKPinAngle = TRUE;
				}
				else
					sRstAlgo->m_bOKPinAngle = TRUE;
				if (!sRstAlgo->m_bOKArea || !sRstAlgo->m_bOKShiftX || !sRstAlgo->m_bOKShiftY || !sRstAlgo->m_bOKWidth || !sRstAlgo->m_bOKLength || !sRstAlgo->m_bOKHeight ||
					sRstAlgo->m_sInspAC.m_nOKInspAC > 0 || !sRstAlgo->m_bOKThickMax || !sRstAlgo->m_bOKThickMin || !sRstAlgo->m_bOKOffsetDistance || !sRstAlgo->m_bOKPinAngle)
					bResult = FALSE;
				if (bInspAC && bInspAC_Ess)
				{
					bResult = TRUE;
					if (sRstAlgo->m_sInspAC.m_nOKInspAC > 0)
						bResult = FALSE;
				}
				nLine = __LINE__;

				//Mars Type이면 윈도우 Rect 추가
				if (bResult == FALSE && g_pMPTI->IsMachineTypeMars())
				{
					sRstAlgo->m_nArrRectCnt++;
					sRstAlgo->m_rcArrRect[0].left = 1;
					sRstAlgo->m_rcArrRect[0].top = 1;
					sRstAlgo->m_rcArrRect[0].right = nImgWidth - 2;
					sRstAlgo->m_rcArrRect[0].bottom = nImgHeight - 2;
				}
			}
		}
		else if (BlobInspMode == eBlobInspMode_Solderball)
		{
			nLine = __LINE__;
			sRstAlgo->m_bOKArea_Reverse = FALSE;
			double dStandardArea = pAlgoBlob->m_dBlobSizeWidth * pAlgoBlob->m_dBlobSizeLength; // * m_resolX * m_resolY;		

			nLine = __LINE__;

			sRstAlgo->m_dTeachArea_Reverse = dStandardArea;

			if (nCntBlob < 1)
			{
				bResult = TRUE;	// blob 영역이 없으면 OK이다			

				sRstAlgo->m_bOKArea_Reverse = TRUE;
				Delete_1DArray(&pUcImgBlob);
				Delete_1DArray(&ucColorImgDst);
				Delete_1DArray(&ucArrFillOutImg);
				Delete_1DArray(&ucBinImg);
				Delete_1DArray(&ucMask);
				if (bResult == false && AINgImageSave)
					vBlobNGImg.push_back(Tmpbuf);
				return bResult;
			}

			m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidth, nImgHeight, _T("ClipBlob_S.bmp"), 1, false, bInspSave);

			double rstW = 0.0;
			double rstH = 0.0;
			double rstD = 0.0;


			double dWndAngle = sWndAlgoImg.dAngle;
			if (pAlgoBlob->m_bUseBlobSizeDistance)
			{
				std::vector<double> rstL;

				std::vector<CRect> rect;
				m_pProcMilAlgo->GetBlobResult_RectsV(rect);

				CMilBlobResult blobRes(rect.size());
				m_pProcMilAlgo->GetBlobResult(&blobRes);

				if (pAlgoBlob->m_nBlobSizeDistanceType == 0)
				{
					BlobDiagonalDistance(rect, nCntBlob, rstL);

					for (int i = 0; i < nCntBlob; i++)
					{
						rstD += rstL[i];
					}
					if (rstL.empty() == FALSE)rstL.clear();
				}
				else if (pAlgoBlob->m_nBlobSizeDistanceType == 2)
				{
					BlobGaussianLength(nImgWidth, nImgHeight, rect, blobRes.blobLabel, nCntBlob, rstL);

					for (int i = 0; i < nCntBlob; i++)
					{
						rstD += rstL[i];
					}
					if (rstL.empty() == FALSE)rstL.clear();
				}
				else if (pAlgoBlob->m_nBlobSizeDistanceType == 1 && nSelectBlob == 0)
				{
					double dRstL = 0.0;
					int nDir = 0;
					std::vector<cv::Point> tmpPtr;
					BlobShiftDistance(nImgWidth, nImgHeight, rect, blobRes.blobLabel, nCntBlob, dRstL, nDir, tmpPtr, pAlgoBlob->m_dBlobSizeDistance);

					if (dRstL <= 0)
					{
						rstD = 0.0;
					}
					else
					{
						if (tmpPtr.size() > 0)
						{
							sRstAlgo->m_nShiftDirIndex = nDir;

							sRstAlgo->m_pShiftSt.x = tmpPtr[0].x;
							sRstAlgo->m_pShiftSt.y = tmpPtr[0].y;
							sRstAlgo->m_pShiftEd.x = tmpPtr[1].x;
							sRstAlgo->m_pShiftEd.y = tmpPtr[1].y;
							rstD = dRstL;
						}
						else
							rstD = dRstL;
					}
				}
			}
			m_pCPInsp_Algo->GetBlobArea(pUcImgBlob, nImgWidth, nImgHeight, dCX, dCY, rcBlob.Width(), rcBlob.Height(), dWndAngle, &rstW, &rstH);
			UCHAR * pucACDst = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucACDst, nImgWidth * nImgHeight * 3);
			memset(pucACDst, 0, nImgWidth * nImgHeight * 3 * sizeof(UCHAR));
			sRstAlgo->m_sInspAC.m_nInspAC = InspAC(pAlgoBlob->m_sAngleColorBase, sInspImageData, pUcImgBlob, sWndAlgoImg, pAlgoBlob->m_byDir, pucACDst, sRstAlgo->m_sInspAC.m_fRstInspAC, &sRstAlgo->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ], false, poCenter);
			Delete_1DArray(&pucACDst);
			float fResult(0.0);
			double dMaxWidth(0.0);
			if (nInspType == eINSP_S_BALL)
			{
				double diameter = (pAlgoBlob->m_dBlobSizeWidth) / dResolX;
				std::vector<CRect> vBlobRects;
				m_pProcMilAlgo->GetBlobResult_RectsV(vBlobRects);
				cv::Mat model(nImgHeight, nImgWidth, CV_8UC1), m_Integ, m_Dev;
				memcpy(model.data, pUcImgBlob, sizeof(UCHAR)*nImgWidth*nImgHeight);
				int nMaxIndex(0);
				double *dArrScore = NULL;
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrScore, nCntBlob);
				memset(dArrScore, 0, sizeof(double)*nCntBlob);
				m_pProcMilAlgo->InspSolderBall(model, vBlobRects, &nMaxIndex, &fResult, dArrScore);
				cv::integral(model, m_Integ, m_Dev);
				int nCnt = (nCntBlob > BLOB_RECT_CNTS) ? BLOB_RECT_CNTS : nCntBlob;
				if (vBlobRects.size() > nMaxIndex)
					dMaxWidth = vBlobRects[nMaxIndex].Width() * m_resolX;
				for (int a = 0; a < nCnt; a++)
					sRstAlgo->m_fArrRstL[a] = dArrScore[a] * 100.0;
				Delete_1DArray(&pUcImgBlob);
				Delete_1DArray(&dArrScore);
			}

			nLine = __LINE__;

			if (rstW <= 0)
				rstW = rcBlob.Width();
			if (rstH <= 0)
				rstH = rcBlob.Height();

			double cogX_roi_pixel = 0.0;
			double cogY_roi_pixel = 0.0;

			if (sRstAlgo)
			{
				cogX_roi_pixel = dWndCX - (nImgWidth / 2.) + dCX;
				cogY_roi_pixel = dWndCY - (nImgHeight / 2.) + dCY;
				/////
				double dCenTeachX = pAlgoBlob->m_dTechCenterX;
				double dCenTeachY = pAlgoBlob->m_dTechCenterY;

				nLine = __LINE__;

				if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
				{
					m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle,
						0, 0, &dCenTeachX, &dCenTeachY);
				}

				nLine = __LINE__;

				sRstAlgo->m_dRstArea_Reverse = dArea * dResolX * dResolY;
				if (pAlgoBlob->m_byNGCnt > 0 && pAlgoBlob->m_bUseBlobNG)
				{
					sRstAlgo->m_nArrRectCnt = 0;
					sRstAlgo->m_bOKArea_Reverse = TRUE;
					sRstAlgo->m_bOKWidth = TRUE;
					sRstAlgo->m_bOKLength = TRUE;
					int nCnt = (nCntBlob > BLOB_RECT_CNTS) ? BLOB_RECT_CNTS : nCntBlob;
					float *fArrW = NULL;
					float *fArrA = NULL;
					float *fArrL = NULL;
					RECT *rArrR = NULL;
					Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrW, BLOB_RECT_CNTS);
					Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrA, BLOB_RECT_CNTS);
					Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrL, BLOB_RECT_CNTS);
					Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &rArrR, BLOB_RECT_CNTS);
					memcpy(fArrW, sRstAlgo->m_fArrRstW, BLOB_RECT_CNTS * sizeof(float));
					memcpy(fArrA, sRstAlgo->m_fArrRstA, BLOB_RECT_CNTS * sizeof(float));
					memcpy(fArrL, sRstAlgo->m_fArrRstL, BLOB_RECT_CNTS * sizeof(float));
					memcpy(rArrR, sRstAlgo->m_rcArrRect, BLOB_RECT_CNTS * sizeof(RECT));
					memset(sRstAlgo->m_fArrRstA, 0, BLOB_RECT_CNTS * sizeof(float));
					memset(sRstAlgo->m_fArrRstW, 0, BLOB_RECT_CNTS * sizeof(float));
					memset(sRstAlgo->m_fArrRstL, 0, BLOB_RECT_CNTS * sizeof(float));
					memset(sRstAlgo->m_rcArrRect, 0, BLOB_RECT_CNTS * sizeof(RECT));
					sRstAlgo->m_dRstArea_Reverse = 0;
					sRstAlgo->m_dRstWidth = 0;
					sRstAlgo->m_dRstLength = 0;
					int nACnt = 0, nWCnt = 0, nLCnt = 0;
					for (int a = 0; a < nCnt; a++)
					{
						if (fArrA[a] > sRstAlgo->m_dRstArea_Reverse) sRstAlgo->m_dRstArea_Reverse = fArrA[a];
						if (fArrW[a] > sRstAlgo->m_dRstWidth) sRstAlgo->m_dRstWidth = fArrW[a];
						if (fArrL[a] > sRstAlgo->m_dRstLength) sRstAlgo->m_dRstLength = fArrL[a];
						if (nInspType == eINSP_S_BALL)
						{
							if ((pAlgoBlob->m_bUseBlobSizeWidth && fArrW[a] >= pAlgoBlob->m_dBlobSizeWidth) ||
								(pAlgoBlob->m_bUseBlobSizeLength && fArrL[a] >= pAlgoBlob->m_dBlobSizeLength))
							{
								sRstAlgo->m_fArrRstA[sRstAlgo->m_nArrRectCnt] = 0.0f;
								sRstAlgo->m_fArrRstW[sRstAlgo->m_nArrRectCnt] = fArrW[a];
								sRstAlgo->m_fArrRstL[sRstAlgo->m_nArrRectCnt] = fArrL[a];
								sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].left = rArrR[a].left;
								sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].right = rArrR[a].right;
								sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].top = rArrR[a].top;
								sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].bottom = rArrR[a].bottom;
								sRstAlgo->m_nArrRectCnt++;
							}
							if (pAlgoBlob->m_bUseBlobSizeWidth && fArrW[a] >= pAlgoBlob->m_dBlobSizeWidth) nWCnt++;
							if (pAlgoBlob->m_bUseBlobSizeLength && fArrL[a] >= pAlgoBlob->m_dBlobSizeLength) nLCnt++;
						}
						else
						{
							if ((pAlgoBlob->m_bUseBlobNG && fArrA[a] >= dStandardArea) ||
								(pAlgoBlob->m_bUseBlobSizeWidth && fArrW[a] >= pAlgoBlob->m_dBlobSizeWidth) ||
								(pAlgoBlob->m_bUseBlobSizeLength && fArrL[a] >= pAlgoBlob->m_dBlobSizeLength))
							{
								sRstAlgo->m_fArrRstA[sRstAlgo->m_nArrRectCnt] = fArrA[a];
								sRstAlgo->m_fArrRstW[sRstAlgo->m_nArrRectCnt] = fArrW[a];
								sRstAlgo->m_fArrRstL[sRstAlgo->m_nArrRectCnt] = fArrL[a];
								sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].left = rArrR[a].left;
								sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].right = rArrR[a].right;
								sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].top = rArrR[a].top;
								sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].bottom = rArrR[a].bottom;
								sRstAlgo->m_nArrRectCnt++;
							}
							if (pAlgoBlob->m_bUseBlobNG && fArrA[a] >= dStandardArea) nACnt++;
							if (pAlgoBlob->m_bUseBlobSizeWidth && fArrW[a] >= pAlgoBlob->m_dBlobSizeWidth) nWCnt++;
							if (pAlgoBlob->m_bUseBlobSizeLength && fArrL[a] >= pAlgoBlob->m_dBlobSizeLength) nLCnt++;
						}
					}
					Delete_1DArray(&fArrW);
					Delete_1DArray(&fArrA);
					Delete_1DArray(&fArrL);
					Delete_1DArray(&rArrR);
					if (nInspType == eINSP_S_BALL)
					{
						if (pAlgoBlob->m_bUseBlobSizeWidth && sRstAlgo->m_dRstWidth >= pAlgoBlob->m_dBlobSizeWidth&& nWCnt >= pAlgoBlob->m_byNGCnt)
							sRstAlgo->m_bOKWidth = FALSE;
						if (pAlgoBlob->m_bUseBlobSizeLength && sRstAlgo->m_dRstLength >= pAlgoBlob->m_dBlobSizeLength&& nLCnt >= pAlgoBlob->m_byNGCnt)
							sRstAlgo->m_bOKLength = FALSE;
					}
					else
					{
						if (pAlgoBlob->m_bUseBlobNG && sRstAlgo->m_dRstArea_Reverse >= dStandardArea && nACnt >= pAlgoBlob->m_byNGCnt)
							sRstAlgo->m_bOKArea_Reverse = FALSE;
						if (pAlgoBlob->m_bUseBlobSizeWidth && sRstAlgo->m_dRstWidth >= pAlgoBlob->m_dBlobSizeWidth&& nWCnt >= pAlgoBlob->m_byNGCnt)
							sRstAlgo->m_bOKWidth = FALSE;
						if (pAlgoBlob->m_bUseBlobSizeLength && sRstAlgo->m_dRstLength >= pAlgoBlob->m_dBlobSizeLength&& nLCnt >= pAlgoBlob->m_byNGCnt)
							sRstAlgo->m_bOKLength = FALSE;
					}
				}
				else
				{
					if (pAlgoBlob->m_bUseBlobNG)
					{
						if (sRstAlgo->m_dRstArea_Reverse >= dStandardArea)
							sRstAlgo->m_bOKArea_Reverse = FALSE;
						else
							sRstAlgo->m_bOKArea_Reverse = TRUE;

					}
					else
						sRstAlgo->m_bOKArea_Reverse = TRUE;

					if (pAlgoBlob->m_bUseBlobSizeWidth)
					{
						sRstAlgo->m_dRstWidth = rstW * m_resolX;
						if (nInspType == eINSP_S_BALL)
							sRstAlgo->m_dRstWidth = dMaxWidth;
						if (sRstAlgo->m_dRstWidth < pAlgoBlob->m_dBlobSizeWidth)
							sRstAlgo->m_bOKWidth = TRUE;
						else
							sRstAlgo->m_bOKWidth = FALSE;
					}
					else
						sRstAlgo->m_bOKWidth = TRUE;

					if (pAlgoBlob->m_bUseBlobSizeLength)
					{
						sRstAlgo->m_dRstLength = rstH * m_resolY;
						if (nInspType == eINSP_S_BALL)
							sRstAlgo->m_dRstLength = fResult * 100; // %
						if (sRstAlgo->m_dRstLength < pAlgoBlob->m_dBlobSizeLength)
							sRstAlgo->m_bOKLength = TRUE;
						else
							sRstAlgo->m_bOKLength = FALSE;
					}
					else
						sRstAlgo->m_bOKLength = TRUE;

					int nCX = dCX;
					int nCY = dCY;
					int nRstW = (dWndAngle == 90 || dWndAngle == 270) ? rstH : rstW;
					int nRstH = (dWndAngle == 90 || dWndAngle == 270) ? rstW : rstH;
					if (BLOB_RECT_CNTS - 1 > sRstAlgo->m_nArrRectCnt)
					{
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].left = nCX - (nRstW / 2);
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].right = nCX + (nRstW / 2);
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].top = nCY - (nRstH / 2);
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].bottom = nCY + (nRstH / 2);
						sRstAlgo->m_nArrRectCnt++;
					}
				}
				bool bSwitch = g_pMPTI->m_bLengthByAngel;

				if (bSwitch && (g_pInspMng->FP_CenterPtr.x > 0 && g_pInspMng->FP_CenterPtr.x < nImgWidth && g_pInspMng->FP_CenterPtr.y > 0 && g_pInspMng->FP_CenterPtr.y < nImgHeight)
					&& nSelectBlob == 0)
				{
					float halfX = rtFPArea.width / 4;	//기본 25%
					float halfY = rtFPArea.height / 4;

					if (nCntBlob == 1)
					{
						double* dCenterX = NULL;
						double* dCenterY = NULL;
						Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dCenterX, nCntBlob);
						Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dCenterY, nCntBlob);
						m_pProcMilAlgo->GetBlobResult_Center(dCenterX, dCenterY);

						double cx = dCenterX[0];
						double cy = dCenterY[0];

						if ((rtFPArea.x < cx && cx < rtFPArea.x + halfX && rtFPArea.y + halfY < cy && cy < rtFPArea.y + (halfY * 3))
							|| rtFPArea.x + (halfX * 3) < cx && cx < rtFPArea.x + rtFPArea.width && rtFPArea.y + halfY < cy && cy < rtFPArea.y + (halfY * 3))
						{
							double tmpLength = sRstAlgo->m_dRstLength;
							sRstAlgo->m_dRstLength = sRstAlgo->m_dRstWidth;
							sRstAlgo->m_dRstWidth = tmpLength;

							if (sRstAlgo->m_dRstLength < pAlgoBlob->m_dBlobSizeLength)
								sRstAlgo->m_bOKLength = TRUE;
							else
								sRstAlgo->m_bOKLength = FALSE;

							if (sRstAlgo->m_dRstWidth < pAlgoBlob->m_dBlobSizeWidth)
								sRstAlgo->m_bOKWidth = TRUE;
							else
								sRstAlgo->m_bOKWidth = FALSE;
						}
						Delete_1DArray(&dCenterX);
						Delete_1DArray(&dCenterY);
					}
				}

				if (pAlgoBlob->m_bUseBlobSizeDistance)
				{
					rstD = int(rstD * 1000) / 1000.0;
					double dRes = int(((dResolX + dResolY) / 2) * 10000) / 10000.0;
					sRstAlgo->m_dRstDistance = rstD * dRes;
					if (sRstAlgo->m_dRstDistance <= pAlgoBlob->m_dBlobSizeDistance)
						sRstAlgo->m_bOKDistance = TRUE;
					else
						sRstAlgo->m_bOKDistance = FALSE;
				}
				else
					sRstAlgo->m_bOKDistance = TRUE;
				if (bInspAC)
				{
					sRstAlgo->m_sInspAC.m_nOKInspAC = 0;
					if ((nInspAC & eDefaultAC_Data_TA) == eDefaultAC_Data_TA)
					{
						if (sRstAlgo->m_sInspAC.m_fRstInspAC[m_eInspAC_R_TA] < pAlgoBlob->m_sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TAMin])
							sRstAlgo->m_sInspAC.m_nOKInspAC |= m_eInspAC_R_Data_TA;
						if (((nInspAC & eDefaultAC_Data_TAMax) == eDefaultAC_Data_TAMax) &&
							sRstAlgo->m_sInspAC.m_fRstInspAC[m_eInspAC_R_TA] > pAlgoBlob->m_sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TAMax])
							sRstAlgo->m_sInspAC.m_nOKInspAC |= m_eInspAC_R_Data_TA;

						if ((sRstAlgo->m_sInspAC.m_nInspAC & eDefaultAC_Data_R) != eDefaultAC_Data_R &&
							(nInspAC & eDefaultAC_Data_TA_R) == eDefaultAC_Data_TA_R &&
							sRstAlgo->m_sInspAC.m_fRstInspAC[m_eInspAC_R_R] > pAlgoBlob->m_sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TA])
							sRstAlgo->m_sInspAC.m_nOKInspAC |= m_eInspAC_R_Data_TA_R;
						if ((sRstAlgo->m_sInspAC.m_nInspAC & eDefaultAC_Data_G) != eDefaultAC_Data_G &&
							(nInspAC & eDefaultAC_Data_TA_G) == eDefaultAC_Data_TA_G &&
							sRstAlgo->m_sInspAC.m_fRstInspAC[m_eInspAC_R_G] > pAlgoBlob->m_sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TA])
							sRstAlgo->m_sInspAC.m_nOKInspAC |= m_eInspAC_R_Data_TA_G;
						if ((sRstAlgo->m_sInspAC.m_nInspAC & eDefaultAC_Data_B) != eDefaultAC_Data_B &&
							(nInspAC & eDefaultAC_Data_TA_B) == eDefaultAC_Data_TA_B &&
							sRstAlgo->m_sInspAC.m_fRstInspAC[m_eInspAC_R_B] > pAlgoBlob->m_sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TA])
							sRstAlgo->m_sInspAC.m_nOKInspAC |= m_eInspAC_R_Data_TA_B;
						if ((sRstAlgo->m_sInspAC.m_nInspAC & eDefaultAC_Data_Gray) != eDefaultAC_Data_Gray &&
							(nInspAC & eDefaultAC_Data_TA_Gr) == eDefaultAC_Data_TA_Gr &&
							sRstAlgo->m_sInspAC.m_fRstInspAC[m_eInspAC_R_Gr] > pAlgoBlob->m_sAngleColorBase.m_fArrInspAC[m_eInspAC_T_TA])
							sRstAlgo->m_sInspAC.m_nOKInspAC |= m_eInspAC_R_Data_TA_BW;
					}
					if ((nInspAC & eDefaultAC_Data_CJ) == eDefaultAC_Data_CJ)
					{
						if (sRstAlgo->m_sInspAC.m_fRstInspAC[m_eInspAC_R_CJ] >= pAlgoBlob->m_sAngleColorBase.m_fArrInspAC[m_eInspAC_T_CJ])
							sRstAlgo->m_sInspAC.m_nOKInspAC |= m_eInspAC_R_Data_CJ;
					}
				}
				else
				{
					sRstAlgo->m_sInspAC.m_nOKInspAC = 0;
					memset(sRstAlgo->m_sInspAC.m_fRstInspAC, 0, m_eInspAC_R_Total * sizeof(float));
				}
				bResult = TRUE;
				BOOL bSBResult = FALSE;
				if (BlobInspMode == eBlobInspMode_Solderball && pAlgoBlob->m_bUseBlobNG && nInspType == eINSP_S_BALL)
				{
					if ((pAlgoBlob->m_bUseBlobSizeWidth && sRstAlgo->m_bOKWidth) || (pAlgoBlob->m_bUseBlobSizeLength && sRstAlgo->m_bOKLength))
						bSBResult = TRUE;
				}
				if (!sRstAlgo->m_bOKArea || !sRstAlgo->m_bOKShiftX || !sRstAlgo->m_bOKShiftY || !sRstAlgo->m_bOKArea_Reverse || !sRstAlgo->m_bOKDistance ||
					((!sRstAlgo->m_bOKWidth || !sRstAlgo->m_bOKLength) && !bSBResult) || sRstAlgo->m_sInspAC.m_nOKInspAC > 0 || !sRstAlgo->m_bOKOffsetDistance)
					bResult = FALSE;
				if (bInspAC_Ess)
				{
					bResult = TRUE;
					if (sRstAlgo->m_sInspAC.m_nOKInspAC > 0)
						bResult = FALSE;
				}
				nLine = __LINE__;
			}
		}
		//else if (BlobInspMode == eBlobInspMode_Coil)
		//{
		//	CStdioFile _csvOpener;
		//	CString totmsg0, totmsg;
		//	double rstCoilW = 0.0;
		//	double rstCoilH = 0.0;
		//	double rstCoilD = 0.0;
		//	double dCoilWndAngle = sWndAlgoImg.dAngle;
		//	bResult = true;

		//	csml::tick tk1, tk2;
		//	float tAlgo(0.f);
		//	float tAlgoPrepare(0.f), tAlgoInner_Seg(0.f), tAlgoInner_Cls(0.f), tAlgoAfterWorks(0.f);
		//	tk1.start();
		//	tk2.start();
		//	//코일검사시작
		//	//검사시 망 재로드하도록 설정되어있는지 확인
		//	//if(g_pMPTI->_inferCoil->_bInit == true)
		//	//{
		//	//	CStdioFile readers;
		//	//	CString str = _T("");
		//	//	if(readers.Open(_T("D:\\coilConfig.txt"), CFile::modeRead))
		//	//	{
		//	//		readers.ReadString(str);
		//	//		g_pMPTI->_inferCoil->_bInit = _ttoi(str);
		//	//		readers.Close();
		//	//	}
		//	//}
		//	if (g_pMPTI->_inferCoil->_bInit == false)
		//	{
		//		g_pMPTI->_inferCoil->Init(BLOB_RECT_CNTS);
		//		if (g_pMPTI->_inferCoil->_bInit == false)
		//		{
		//			//MessageBox(NULL, _T("AI Coil Model Not Loaded!"), _T("Warning"), MB_OK | MB_SETFOREGROUND | MB_TOPMOST );
		//			sRstAlgo->m_bOKCoilRst = false;
		//			Delete_1DArray(&pUcImgBlob);
		//			Delete_1DArray(&ucColorImgDst);
		//			Delete_1DArray(&ucArrFillOutImg);
		//			Delete_1DArray(&ucBinImg);
		//			Delete_1DArray(&ucMask);
		//			if (bResult == false && AINgImageSave)
		//				vBlobNGImg.push_back(Tmpbuf);
		//			return false;
		//		}
		//	}

		//	/////////////////////////////////////////////////
		//	//
		//	//  입력 정보
		//	//
		//	//  srcBR : Bottom Red   조명 이미지
		//	//  srcTB : Top    Blue  조명 이미지
		//	//  srcTW : Top    White 조명 이미지
		//	//
		//	//  srcMask : 3D threshold 적용한 이진 이미지
		//	//  srcW, srcH : 입력 이미지의 width, height 정보
		//	//
		//	/////////////////////////////////////////////////


		//	uchar *srcBR, *srcTB, *srcTW, *srcMask;
		//	int srcW, srcH;

		//	//파트창 크기(sInspImageData.nImageSizeY, sInspImageData.nImageSizeX)

		//	srcW = sInspImageData.nImageSizeX;
		//	srcH = sInspImageData.nImageSizeY;

		//	srcBR = NULL;
		//	srcTB = NULL;
		//	srcTW = NULL;
		//	srcMask = NULL;
		//	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &srcBR, srcW*srcH);
		//	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &srcTB, srcW*srcH);
		//	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &srcTW, srcW*srcH);
		//	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &srcMask, nImgWidth*nImgHeight);

		//	memcpy(srcBR, sInspImageData.imgBottom_R, srcW*srcH);
		//	memcpy(srcTB, sInspImageData.imgTop_B, srcW*srcH);
		//	memcpy(srcTW, sInspImageData.imgTop_W, srcW*srcH);
		//	memcpy(srcMask, pUcImgBlob, nImgWidth*nImgHeight);

		//	std::vector<uchar> aiRes;
		//	//신경망 통과, threshold 적용
		//	//Area : 디폴트 검사
		//	//Width, Height, Size Dist : 체크박스 옵션에 따름
		//	g_pMPTI->_inferCoil->_blobLimitA = (pAlgoBlob->m_dBlobSizeLength / dResolY) * (pAlgoBlob->m_dBlobSizeWidth / dResolX);
		//	g_pMPTI->_inferCoil->_blobLimitW = pAlgoBlob->m_bUseBlobSizeWidth ? pAlgoBlob->m_dBlobSizeWidth / dResolX : 0;
		//	g_pMPTI->_inferCoil->_blobLimitH = pAlgoBlob->m_bUseBlobSizeLength ? pAlgoBlob->m_dBlobSizeLength / dResolY : 0;
		//	g_pMPTI->_inferCoil->_segTh = pAlgoBlob->m_nCoilThMax > pAlgoBlob->m_nCoilThMin ? pAlgoBlob->m_nCoilThMax : pAlgoBlob->m_nCoilThMin;
		//	double dRes = (dResolX + dResolY) / 2;
		//	g_pMPTI->_inferCoil->_blobLimitSzD = pAlgoBlob->m_bUseBlobSizeDistance ? pAlgoBlob->m_dBlobSizeDistance / dRes : 0;


		//	tAlgoPrepare = tk2.end();
		//	tk2.start();
		//	g_pMPTI->_inferCoil->CoilResult(srcBR, srcTB, srcTW, srcMask, srcW, srcH, cv::Rect(0, 0, nImgWidth, nImgHeight));
		//	totmsg0 = g_pMPTI->_inferCoil->_tempMsg_Seg;
		//	tAlgoInner_Seg = tk2.end();
		//	tk2.start();
		//	g_pMPTI->_inferCoil->CoilClsResult();
		//	totmsg0 = totmsg0 + g_pMPTI->_inferCoil->_tempMsg_Cls;
		//	tAlgoInner_Cls = tk2.end();
		//	tk2.start();
		//	//sRstAlgo->m_bOKCoilRst = bResult;

		//	/////////////////////////////////////////////////
		//	//
		//	//  출력 정보
		//	//
		//	// ai_result : 코일 검사 결과 이진 이미지
		//	//
		//	/////////////////////////////////////////////////

		//	if (nImgHeight*nImgWidth > 0)
		//	{

		//		g_pMPTI->m_coilH = srcH;//nImgHeight;
		//		g_pMPTI->m_coilW = srcW;//nImgWidth;
		//		g_pMPTI->m_coilEnumber = g_pMPTI->_inferCoil->_curSequence;

		//		//가장 큰 블랍 기준으로 결과 판정
		//		int nCoilBlobs = g_pMPTI->_inferCoil->_rcCoilDefects.size();
		//		sRstAlgo->m_nArrRectCnt = nCoilBlobs > BLOB_RECT_CNTS ? BLOB_RECT_CNTS : nCoilBlobs;

		//		//Coil Area + (Width | Height | Distance)

		//		if (nCoilBlobs >= pAlgoBlob->m_nCoilCntThresh)
		//		{
		//			memcpy(sRstAlgo->m_rcArrRect, &(g_pMPTI->_inferCoil->_rcCoilDefects)[0], sRstAlgo->m_nArrRectCnt * sizeof(RECT));
		//			//positioning max area rect to first vector
		//			std::vector<double> vArea = g_pMPTI->_inferCoil->_rctArea;
		//			int mIdx = max_element(vArea.begin(), vArea.end()) - vArea.begin();
		//			dArea = vArea[mIdx];
		//			RECT curRect = sRstAlgo->m_rcArrRect[0];
		//			sRstAlgo->m_rcArrRect[0] = sRstAlgo->m_rcArrRect[mIdx];
		//			sRstAlgo->m_rcArrRect[mIdx] = curRect;

		//			//Area Start
		//			sRstAlgo->m_dRstArea_Reverse = dArea * dResolX * dResolY;
		//			{
		//				double dStandardArea = pAlgoBlob->m_dBlobSizeWidth * pAlgoBlob->m_dBlobSizeLength; // * m_resolX * m_resolY;
		//				if (sRstAlgo->m_dRstArea_Reverse >= dStandardArea)
		//					sRstAlgo->m_bOKArea_Reverse = FALSE;
		//				else
		//					sRstAlgo->m_bOKArea_Reverse = TRUE;
		//			}
		//			//Area End

		//			//Width, Height Start
		//			if (pAlgoBlob->m_bUseBlobSizeWidth)
		//			{
		//				double rstCoilW = sRstAlgo->m_rcArrRect[0].right - sRstAlgo->m_rcArrRect[0].left;
		//				sRstAlgo->m_dRstWidth = rstCoilW * dResolX;
		//				if (sRstAlgo->m_dRstWidth < pAlgoBlob->m_dBlobSizeWidth)
		//					sRstAlgo->m_bOKWidth = TRUE;
		//				else
		//					sRstAlgo->m_bOKWidth = FALSE;
		//			}
		//			else
		//				sRstAlgo->m_bOKWidth = TRUE;
		//			if (pAlgoBlob->m_bUseBlobSizeLength)
		//			{
		//				double rstCoilH = sRstAlgo->m_rcArrRect[0].bottom - sRstAlgo->m_rcArrRect[0].top;
		//				sRstAlgo->m_dRstLength = rstCoilH * dResolY;
		//				if (sRstAlgo->m_dRstLength < pAlgoBlob->m_dBlobSizeLength)
		//					sRstAlgo->m_bOKLength = TRUE;
		//				else
		//					sRstAlgo->m_bOKLength = FALSE;
		//			}
		//			else
		//				sRstAlgo->m_bOKLength = TRUE;
		//			//Width, Height End

		//			//Distance Start
		//			if (pAlgoBlob->m_bUseBlobSizeDistance)
		//			{
		//				rstCoilD = g_pMPTI->_inferCoil->_rstD.size() > 0 ? g_pMPTI->_inferCoil->_rstD[mIdx] : 0;
		//				rstCoilD = int(rstCoilD * 1000) / 1000.0;
		//				double dRes = int(((dResolX + dResolY) / 2) * 10000) / 10000.0;
		//				sRstAlgo->m_dRstDistance = rstCoilD * dRes;
		//				if (sRstAlgo->m_dRstDistance <= pAlgoBlob->m_dBlobSizeDistance)
		//					sRstAlgo->m_bOKDistance = TRUE;
		//				else if (sRstAlgo->m_dRstWidth == 0.f && sRstAlgo->m_dRstLength == 0)
		//					sRstAlgo->m_bOKDistance = TRUE;
		//				else
		//					sRstAlgo->m_bOKDistance = FALSE;
		//			}
		//			//Distance End

		//			sRstAlgo->m_bOKCoilRst = bResult = sRstAlgo->m_bOKLength&&sRstAlgo->m_bOKWidth&&sRstAlgo->m_bOKDistance&&sRstAlgo->m_bOKArea_Reverse;
		//		}
		//		//Coil Area + (Width | Height | Distance)
		//		//Delete_1DArray(&coilData);
		//	}

		//	Delete_1DArray(&srcBR);
		//	Delete_1DArray(&srcTB);
		//	Delete_1DArray(&srcTW);
		//	Delete_1DArray(&srcMask);

		//	sRstAlgo->m_bOKShiftX = TRUE;
		//	sRstAlgo->m_bOKShiftY = TRUE;
		//	sRstAlgo->m_bOKOffsetDistance = TRUE;

		//	tAlgoAfterWorks = tk2.end();
		//	tAlgo = tk1.end();

		//	totmsg.Format(_T("%.3f,%.3f,%.3f,%d,%.3f,%.3f,%.3f"), tAlgo, tAlgoPrepare, tAlgoInner_Seg, g_pMPTI->_inferCoil->_segTh, tAlgoInner_Cls, g_pMPTI->_inferCoil->_clsTh, tAlgoAfterWorks);
		//	g_pMPTI->_inferCoil->WriteCSV(_T("D:\\Eagle3D_data\\log\\zLog_Coil.csv"), totmsg + totmsg0);

		//	//코일검사종료
		//}BN
		if (bResult == false && AINgImageSave)
			vBlobNGImg.push_back(Tmpbuf);

		Delete_1DArray(&pUcImgBlob);
		Delete_1DArray(&ucColorImgDst);
		Delete_1DArray(&ucArrFillOutImg);
		Delete_1DArray(&ucBinImg);
		Delete_1DArray(&ucMask);
	}
	catch (...)
	{

		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Algo::InspBlob(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}

	return bResult;
}

BOOL CPInsp_AlgoBlob::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	if ((inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
	{
		AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
		pAlignRes->TeachCenterX = pAlgoBlob->m_dTechCenterX;
		pAlignRes->TeachCenterY = pAlgoBlob->m_dTechCenterY;

		RstAlgoBlob * rst = (RstAlgoBlob *)sRstAlgo;
		pAlignRes->offsetX = -rst->m_dRstShiftX;
		pAlignRes->offsetY = rst->m_dRstShiftY;
		pAlignRes->theta = 0;
		bRet = TRUE;
	}

	return bRet;
}
int CPInsp_AlgoBlob::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoBlob * pInspAlgo = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;

	nData = pInspAlgo->m_sAlgoColorBase.GetColorData();
	nData |= COLOR_DATA_WND;

	if (pInspAlgo->m_bInspCoil)
		nData |= COLOR_DATA_AI;
	else
	{
		if (pInspAlgo->m_sAngleColorBase.UseAC())
			nData |= COLOR_DATA_AC;
	}

	return nData;
}

// Teaching function
BOOL CPInsp_AlgoBlob::TeachingBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndAlgoImg &sWndClipAlgo, InspRoiImgBuf &sInspImageData,
	const AlgoCoordinate &coordinateAlgo, WndInfo sWndInfo, UCHAR *puImgDst, RstAlgoBlob * sRstAlgo, TotalInspExceptArea stTieArea, int nInspType, int nBlobMode, UCHAR* ucArrDstImgAC, bool bTeach)
{
	BOOL bResult = FALSE;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, (sizeof(RstAlgoBlob)));
		sRstAlgo->Init();
	}
	if (sInspAlgo.m_eAlgoType != eAlgoBlob)
		return bResult;
	if (!m_pProcMilAlgo)
		return bResult;
	AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pAlgoBlob)
		return bResult;
	if (!pAlgoBlob->m_bInsp2D && !pAlgoBlob->m_bInsp3D && !pAlgoBlob->m_sAlgoColorBase.m_bUseColor)
		return bResult;

	bool bInspSave = false;
	if (g_pMPTI)
	{
		bInspSave = g_pMPTI->m_LogLevel == m_eLogLv_Blob;
		if (bInspSave)
		{
			CString sLog = _T("");
			sLog.Format(_T("[Blob] [TeachBlob] Color %d ColorAC %d AC %d _ %d"),
				pAlgoBlob->m_sAlgoColorBase.m_bUseColor, pAlgoBlob->m_sAlgoColorBase.m_bUseAngleColor,
				pAlgoBlob->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data], pAlgoBlob->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data2]);
			g_pMPTI->AddLog_Dev(sLog);
		}
	}

	double dResolX = m_resolX;
	double dResolY = m_resolY;

	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}

	double dClipWidth = (float)(sWndInfo.dWidth / dResolX/*m_resolX*/);// + 0.01;
	double dClipHeight = (float)(sWndInfo.dLength / dResolY/*m_resolY*/);// + 0.01;
	int nClipStartX = 0, nClipStartY = 0;

	float fWindowRoundingErrX = 0.0f, fWindowRoundingErrY = 0.0f;

	if (dClipWidth < sWndAlgoImg.m_nWidth && dClipHeight < sWndAlgoImg.m_nHeight)
	{
		double dClipCenterX2D = (double)sWndAlgoImg.m_nWidth / 2.0 + (sWndInfo.dCenterX / dResolX/*m_resolX*/);
		double dClipCenterY2D = (double)sWndAlgoImg.m_nHeight / 2.0 - (sWndInfo.dCenterY / dResolY/*m_resolY*/);
		if (dClipCenterX2D <= 0)
			dClipCenterX2D = dClipWidth / 2.0;
		if (dClipCenterY2D <= 0)
			dClipCenterY2D = dClipHeight / 2.0;

		nClipStartX = RounD(dClipCenterX2D - (dClipWidth / 2.0));
		nClipStartY = RounD(dClipCenterY2D - (dClipHeight / 2.0));
		if (nClipStartX < 0)
			nClipStartX = 0;
		if (nClipStartY < 0)
			nClipStartY = 0;
	}

	// 원래 이미지 2진화
	UCHAR *pUcOrgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfOrgSrc = sWndAlgoImg.m_fArr3D;
	int nOrgWidth = sWndAlgoImg.m_nWidth;
	int nOrgHeight = sWndAlgoImg.m_nHeight;
	if ((pUcOrgSrc == NULL) || (nOrgWidth <= 0) || (nOrgHeight <= 0))
		return bResult;
	if (puImgDst)
		memset(puImgDst, 0, sizeof(UCHAR)*nOrgWidth*nOrgHeight);
	if (ucArrDstImgAC)
	{
		int nWndIDXAC = 0;
		int widthStep = nOrgWidth;
		//	if ((widthStep % 4) != 0) widthStep += 4 - (widthStep % 4);
		widthStep = g_pMPTI->nCalcWidthStep(false, nOrgWidth);

		memset(ucArrDstImgAC, 0, widthStep * nOrgHeight * 3 * sizeof(UCHAR));
	}
	UCHAR *pUcImgSrc = sWndClipAlgo.m_ucArr2D;
	float *pfImgSrc = sWndClipAlgo.m_fArr3D;
	int nImgWidth = sWndClipAlgo.m_nWidth;
	int nImgHeight = sWndClipAlgo.m_nHeight;
	if ((nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

#if _DEBUG
	cv::Mat imgUcOrgSrc(nOrgHeight, nOrgWidth, CV_8UC1, pUcOrgSrc);
	cv::Mat imgUcImgSrc(nImgHeight, nImgWidth, CV_8UC1, pUcImgSrc);
#endif

	//Save coordiAlgo.dROICnt double type
	double dROICenterX = (double)(coordinateAlgo.dROICenterX);
	double dROICenterY = (double)(coordinateAlgo.dROICenterY);

	int nROICenterX = RounD(coordinateAlgo.dROICenterX);
	int nROICenterY = RounD(coordinateAlgo.dROICenterY);
	if ((nROICenterX < 0) || (nROICenterY < 0) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;
	int nRoiArea = nImgWidth * nImgHeight;
	double dWndAngle = coordinateAlgo.dROIAngle;

	UCHAR * ucArrBlobDst = NULL;
	if (puImgDst)
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobDst, nImgWidth * nImgHeight);
	memset(ucArrBlobDst, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);

	// 통합 검사/제외 영역에 넣어준다
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

	double dCX = 0;
	double dCY = 0;
	double dArea = 0;
	CRect rcBlob;
	int nMinBlobArea = pAlgoBlob->m_nMinBlobArea;
	//2020 0716 bjy : All blob & NG Blob일때   1 x 1 불량이 검출되어 주석처리
	//if(pAlgoBlob->m_nTypeSelectBlob == eSelectMix && pAlgoBlob->m_bUseBlobNG == TRUE)
	//	nMinBlobArea = 0;
	double cogX_roi_pixel = 0.0;
	double cogY_roi_pixel = 0.0;
	UCHAR *ucColorImgDst = NULL;
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	if (pColorTeach != NULL && pAlgoBlob->m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		if (sInspImageData.nImageSizeX <= 0 || sInspImageData.nImageSizeY <= 0)
		{
			if (ucArrBlobDst)
				Delete_1DArray(&ucArrBlobDst);
			return bResult;
		}
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		pColorTeach->GetColorBaseBin(&pAlgoBlob->m_sAlgoColorBase, sInspImageData, ucColorImgDst, true, 0, 0, -1, sWndAlgoImg.m_nLight_index);
		m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, nImgWidth, nImgHeight, _T("OrgColor_Image.bmp"));
	}
	double dAreaPix = pAlgoBlob->m_dAreaCurrent / dResolX / dResolY;
	double dAreaWPix = pAlgoBlob->m_dTeachWidth / dResolX;
	double dAreaHPix = pAlgoBlob->m_dTeachLength / dResolY;
	if (sWndClipAlgo.dAngle == 90 || sWndClipAlgo.dAngle == 270)
	{
		dAreaWPix = pAlgoBlob->m_dTeachLength / dResolY;
		dAreaHPix = pAlgoBlob->m_dTeachWidth / dResolX;
	}

	double dTechCenterX = -1;
	double dTechCenterY = -1;

	if (pAlgoBlob->m_nTypeSelectBlob == eSelectPosition)
	{
		double dCenTeachX = pAlgoBlob->m_dTechCenterX;
		double dCenTeachY = pAlgoBlob->m_dTechCenterY;
		if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
		{
			m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle,
				0, 0, &dCenTeachX, &dCenTeachY);
		}

		dTechCenterX = (nOrgWidth / 2.) + (dCenTeachX / m_resolX);
		dTechCenterY = (nOrgHeight / 2.) - (dCenTeachY / m_resolY);
		dTechCenterX -= nClipStartX;
		dTechCenterY -= nClipStartY;
	}

	int nCntBlob = 0;
	BOOL IsBinImage = FALSE;
	cv::Rect rtFPArea;
	if (pAlgoBlob->m_bUsePattern && pAlgoBlob->m_bUseFPBW)
	{
		int nTmpTypeSelectBlob = pAlgoBlob->m_nTypeSelectBlob;
		pAlgoBlob->m_nTypeSelectBlob = eSelectMix;
		nCntBlob = m_pCPInsp_Algo->BlobImageStruct(*pAlgoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucArrBlobDst, stTieArea, pAlgoBlob->m_bFillHole);
		Delete_1DArray(&ucColorImgDst);
		ucColorImgDst = NULL;
		m_pProcMilAlgo->SaveWorkImg(pUcImgSrc, nImgWidth, nImgHeight, _T("Blob_2D_Allblob.bmp"));
		if (g_pMPTI->m_bSideOriginalSize != true)
			m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Blob_3D_Allblob.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("Blob_Rst_Allblob.bmp"));

		POINTF centerPtr;
		bool bSwitch = g_pMPTI->m_bLengthByAngel;

		if (bSwitch && pAlgoBlob->m_bUsePattern)
		{
			centerPtr = CalcForiegnPatternCenter(sInspAlgo, sWndClipAlgo, sInspImageData, nROICenterX, nROICenterY, nImgWidth, nImgHeight, stTieArea, 0, rtFPArea);
			g_pInspMng->FP_CenterPtr = centerPtr;
		}

		g_pInspMng->ForeignPatternSet(sInspAlgo, ucArrBlobDst, nImgWidth, nImgHeight, sWndAlgoImg.dAngle, 0, sRstAlgo->m_pFPMatchPos);

		m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("Blob_FP.bmp"));

		//blob setting
		pAlgoBlob->m_nTypeSelectBlob = nTmpTypeSelectBlob;
		int nFilter = 0;
		if (pAlgoBlob->m_bFilterIsUse)
			nFilter = pAlgoBlob->m_nFilterStepNarrow;
		pAlgoBlob->m_nTypeRange2D = 2;
		pAlgoBlob->m_nMinBinary = 0;
		pAlgoBlob->m_nMaxBinary = 200;
		pAlgoBlob->m_bInsp3D = false;
		pAlgoBlob->m_bInsp2D = true;

		dCX = 0, dCY = 0, dArea = 0;
		rcBlob.left = 0; rcBlob.right = 0; rcBlob.top = 0; rcBlob.bottom = 0;
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucArrBlobDst, ucArrBlobDst, nImgWidth, nImgHeight, nMinBlobArea, FALSE, pAlgoBlob->m_bFillHole, nFilter, pAlgoBlob->m_nTypeSelectBlob);
		if (nCntBlob > 0)
		{
			m_pProcMilAlgo->GetBlobResult(&dArea, &dCX, &dCY, &rcBlob, FALSE);
			m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("Blob_FP_Blob.bmp"));
		}
		pUcImgSrc = ucArrBlobDst;
		// 		if(pColorTeach != NULL && pAlgoBlob->m_sAlgoColorBase.m_bUseColor == TRUE)
		// 			memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		IsBinImage = TRUE;
	}
	nCntBlob = m_pCPInsp_Algo->BlobImageStruct(*pAlgoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea,
		&dArea, &dCX, &dCY, &rcBlob, ucArrBlobDst, stTieArea, pAlgoBlob->m_bFillHole, 0, dTechCenterX, dTechCenterY, IsBinImage, dAreaPix, dAreaWPix, dAreaHPix);
	if (pAlgoBlob->m_nTypeSelectBlob == eSimilarArea && nCntBlob == 0 && pAlgoBlob->m_bInsp2D)
	{
		for (int b = 0; b < 20; b++)
		{
			int nGapBinary = 0;
			int nMaxBinary = pAlgoBlob->m_nMaxBinary;
			bool bOK = false;
			for (int a = 0; a < 60; a++)
			{
				int nRstBinary = nMaxBinary;
				if (a % 2 == 0)
				{
					nGapBinary += 5;
					nRstBinary = nMaxBinary - nGapBinary;
				}
				else
					nRstBinary = nMaxBinary + nGapBinary;
				if (nRstBinary <= 0 || nRstBinary >= 255)
					continue;
				pAlgoBlob->m_nMaxBinary = nRstBinary;
				nCntBlob = m_pCPInsp_Algo->BlobImageStruct(*pAlgoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea,
					&dArea, &dCX, &dCY, &rcBlob, ucArrBlobDst, stTieArea, pAlgoBlob->m_bFillHole, 0, -1, -1, 0, dAreaPix, dAreaWPix, dAreaHPix, b);
				if (nCntBlob == 0) continue;
				bOK = true;
				break;
			}
			if (bOK) break;
		}
	}
	if (pUcImgSrc)
		m_pProcMilAlgo->SaveWorkImg(pUcImgSrc, nImgWidth, nImgHeight, _T("Blob_2D.bmp"));
	if (pfImgSrc)
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Blob_3D.bmp"));
	if (ucArrBlobDst)
		m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("Blob_Rst.bmp"));
	// 평균 높이 산출.
	double dHeightMeanBody = 0.0;
	float fPinHeight = pAlgoBlob->m_fPinHeight;
	float fPinAngle = 0.0f;
	try
	{
		UCHAR * ucArrBlobDst_except = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobDst_except, nImgWidth * nImgHeight);
		memset(ucArrBlobDst_except, 0, sizeof(UCHAR) * nImgWidth * nImgHeight);
		AlgoBlob pAlgoBlob_except;
		pAlgoBlob_except.m_bInsp2D = pAlgoBlob->m_bInsp2D_ExceptArea;
		pAlgoBlob_except.m_nTypeRange2D = pAlgoBlob->m_nTypeRange2D_ExceptArea;
		pAlgoBlob_except.m_nMinBinary = pAlgoBlob->m_nMinBinary_ExceptArea;
		pAlgoBlob_except.m_nMaxBinary = pAlgoBlob->m_nMaxBinary_ExceptArea;
		pAlgoBlob_except.m_bInsp3D = pAlgoBlob->m_bInsp3D_ExceptArea;
		pAlgoBlob_except.m_nTypeRange3D = pAlgoBlob->m_nTypeRange3D_ExceptArea;
		pAlgoBlob_except.m_dHeightRateMin = pAlgoBlob->m_dHeightRateMin_ExceptArea;
		pAlgoBlob_except.m_dHeightRateMax = pAlgoBlob->m_dHeightRateMax_ExceptArea;
		pAlgoBlob_except.m_nTypeSelectBlob = eSelectMix;
		pAlgoBlob_except.m_nFilterStepNarrow = 0;
		pAlgoBlob_except.m_bFilterIsUse = FALSE;
		pAlgoBlob_except.m_bFillHole = FALSE;
		TotalInspExceptArea stTieArea_except;
		stTieArea_except.m_nUsedWndPolygon = 4;
		stTieArea_except.m_ptArrWndPolygon[0].x = rcBlob.left - (nImgWidth / 2);
		stTieArea_except.m_ptArrWndPolygon[0].y = rcBlob.top - (nImgHeight / 2);
		stTieArea_except.m_ptArrWndPolygon[1].x = rcBlob.right - (nImgWidth / 2);
		stTieArea_except.m_ptArrWndPolygon[1].y = rcBlob.top - (nImgHeight / 2);
		stTieArea_except.m_ptArrWndPolygon[2].x = rcBlob.right - (nImgWidth / 2);
		stTieArea_except.m_ptArrWndPolygon[2].y = rcBlob.bottom - (nImgHeight / 2);
		stTieArea_except.m_ptArrWndPolygon[3].x = rcBlob.left - (nImgWidth / 2);
		stTieArea_except.m_ptArrWndPolygon[3].y = rcBlob.bottom - (nImgHeight / 2);
		stTieArea_except.m_nUsedInspPolygon = 0;
		stTieArea_except.m_nUsedMaskingValue = 0;
		int nMinArea = ((int)dArea / 200);
		double dArea_except(0.0f), dCX_except(0.0f), dCY_except(0.0f);
		CRect rcBlob_except;
		rcBlob_except.left = rcBlob_except.right = rcBlob_except.top = rcBlob_except.bottom = 0;
		double dPartCenterX = sWndInfo.m_dPartWidth / 2;
		double dPartCenterY = sWndInfo.m_dPartHeight / 2;
		int nCntBlobExcept = 0;
		if (pAlgoBlob->m_bInsp2D_ExceptArea || pAlgoBlob->m_bInsp3D_ExceptArea)
		{
			nCntBlobExcept = m_pCPInsp_Algo->BlobImageStruct(pAlgoBlob_except, sWndClipAlgo.m_ucArr2D_Mix[1], pfImgSrc, NULL, nImgWidth, nImgHeight, nMinArea,
				&dArea_except, &dCX_except, &dCY_except, &rcBlob_except, ucArrBlobDst_except, stTieArea_except, pAlgoBlob->m_bFillHole, 0, -1, -1, 0, dAreaPix, dAreaWPix, dAreaHPix);
			cv::Mat exGray(nImgHeight, nImgWidth, CV_8UC1, sWndClipAlgo.m_ucArr2D_Mix[1]);
		}
		BlobNGImg Tmpbuf;
		double minVal = 0.0f, maxVal = 0.0f;
		RECT ThminRect, ThmaxRect;
		ThminRect.left = ThminRect.right = ThminRect.top = ThminRect.bottom = 0;
		ThmaxRect.left = ThmaxRect.right = ThmaxRect.top = ThmaxRect.bottom = 0;
		cv::Mat ContoursMaxImg(nImgHeight, nImgWidth, CV_8UC1, ucArrBlobDst);
		//ePartBodyRo_All = 0x00,
		//ePartBodyRo_Top = 0x01,
		//ePartBodyRo_Bottom = 0x02,
		//ePartBodyRo_Left = 0x04,
		//ePartBodyRo_Right = 0x08
		int nWndDir = 0;
		cv::Point poConnerLt(0, 0), poConnerRb(nImgWidth - 1, nImgHeight - 1);
		float fWndCenterX = sWndInfo.dCenterX / m_resolX;
		float fWndCenterY = -sWndInfo.dCenterY / m_resolY;
		float fWndWidth = sWndInfo.dWidth / 2 / m_resolX;
		float fWndLength = sWndInfo.dLength / 2 / m_resolY;
		bool bIsTop = fWndCenterY - fWndLength <= 0 ? true : false;
		bool bIsBot = fWndCenterY + fWndLength >= 0 ? true : false;
		if ((pAlgoBlob->m_bUseThickMax || pAlgoBlob->m_bUseThickMin) && sRstAlgo && nCntBlob > 0 && dArea > 5)
		{
			nCntBlob = InspThickness2(pAlgoBlob, pUcImgSrc, ucArrBlobDst, ucArrBlobDst_except, nImgWidth, nImgHeight, nCntBlobExcept, dArea, dArea_except, rcBlob, fWndCenterX, fWndCenterY, fWndWidth, fWndLength, ThminRect, ThmaxRect, minVal, maxVal, sRstAlgo,bTeach);
		}

		if (nCntBlob > 0 && rcBlob.Width() > 0 && rcBlob.Height() > 0 && g_pMPTI->m_bSideOriginalSize != true)
		{
			int nWidthBolb = rcBlob.Width();
			int nHeightBlob = rcBlob.Height();
			//float *pfImgDst = new float[nWidthBolb * nHeightBlob];
			float *pfImgDst = g_pMManager->pem_new<float>(true, nWidthBolb * nHeightBlob, (PCHAR)__FUNCTION__, __LINE__);

			if (pfImgSrc)
				m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst, nImgWidth, nImgHeight, rcBlob.left, rcBlob.top, nWidthBolb, nHeightBlob);
			if (pfImgDst && g_pMPTI->m_bSideOriginalSize != true)
				m_pProcMilAlgo->SaveWorkImg_float(pfImgDst, nWidthBolb, nHeightBlob, _T("BodyBlob3D.bmp"));

			BOOL bCalcAvgMask = pAlgoBlob->m_bUseHeightAvgMask;
			if (bCalcAvgMask == FALSE)
				dHeightMeanBody = m_pCPInsp_Algo->GetHeightMean(pfImgDst, nWidthBolb, nHeightBlob);
			else
				dHeightMeanBody = m_pCPInsp_Algo->GetHeightAvgMask(pfImgSrc, ContoursMaxImg.ptr(), nImgWidth, nImgHeight, rcBlob.left, rcBlob.top, nWidthBolb, nHeightBlob);

			if (nCntBlob > 0 && pAlgoBlob->m_bUsePinAngle)
			{
				cv::Mat AngleHeight(nHeightBlob, nWidthBolb, CV_32FC1, pfImgDst);
				double dMin, dMax;
				cv::Point poMin, poMax;
				cv::minMaxLoc(AngleHeight, &dMin, &dMax, &poMin, &poMax);
				float poX = dCX + poMax.x;
				float poY = dCY + poMax.y;
				double dCenTeachX = pAlgoBlob->m_dTechCenterX;
				double dCenTeachY = pAlgoBlob->m_dTechCenterY;
				double rcBlob_left, rcBlob_right, rcBlob_top, rcBlob_bottom;
				if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
				{
					m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle,
						0, 0, &dCenTeachX, &dCenTeachY);
					m_proc3d.CorrectCoordinate(rcBlob_left, rcBlob_top, 0, 0, dWndAngle,
						0, 0, &rcBlob_left, &rcBlob_top);
					m_proc3d.CorrectCoordinate(rcBlob_right, rcBlob_bottom, 0, 0, dWndAngle,
						0, 0, &rcBlob_right, &rcBlob_bottom);
					if (rcBlob_bottom < rcBlob_top)
					{
						double dtmp = rcBlob_bottom;
						rcBlob_bottom = rcBlob_top;
						rcBlob_top = dtmp;
					}
					if (rcBlob_right < rcBlob_left)
					{
						double dtmp = rcBlob_right;
						rcBlob_right = rcBlob_left;
						rcBlob_left = dtmp;
					}
					if ((dWndAngle == 180) || (dWndAngle == 270))
					{
						double dtmp = rcBlob_bottom;
						rcBlob_bottom = rcBlob_right;
						rcBlob_right = dtmp;

						dtmp = rcBlob_top;
						rcBlob_top = rcBlob_left;
						rcBlob_left = dtmp;
					}
				}
				//shiftX
				if (dCX - dCenTeachX > 0)//left
				{
					float fW = pAlgoBlob->m_dTeachWidth / 2 / dResolX;
					poX = rcBlob.left + fW;
				}
				else if (dCX - dCenTeachX < 0)//right
				{
					float fW = pAlgoBlob->m_dTeachWidth / 2 / dResolX;
					poX = rcBlob.right - fW;
				}
				//shiftY
				if (dCY - dCenTeachY > 0)//top
				{
					float fH = pAlgoBlob->m_dTeachLength / 2 / dResolY;
					poY = rcBlob.top + fH;
				}
				else if(dCY - dCenTeachY < 0)//bottom
				{
					float fH = pAlgoBlob->m_dTeachLength / 2 / dResolY;
					poY = rcBlob.top + fH;
				}
				float fShX = std::abs(dCenTeachX- poX) * m_resolX;
				float fShY = std::abs(dCenTeachY- poY) * m_resolY;
				float fSh = std::sqrtf(std::powf(fShX, 2) + std::powf(fShY, 2));
				
				int nMaxCnt(0);
				float fMax(0.0f);
				int rst = poMax.y - 1 > 0 ? poMax.y - 1 : 0;
				int red = poMax.y + 1 <= AngleHeight.rows ? poMax.y + 1 : AngleHeight.rows;
				int cst = poMax.x - 1 > 0 ? poMax.x - 1 : 0;
				int ced = poMax.x + 1 <= AngleHeight.cols ? poMax.x + 1 : AngleHeight.cols;
				for (int r = rst; r < red; r++)
				{
					float* fPtr = AngleHeight.ptr<float>(r);
					for (int c = cst; c < ced; c++)
					{
						fMax += fPtr[c];
						nMaxCnt++;
					}
				}
				fMax /= nMaxCnt;
				//fPinHeight = fMax;
				if (fPinHeight == 0)
					fPinHeight = (float)(fMax - dMin);
				if (fSh != 0)
				{
					float fRad = std::atanf(fPinHeight / fSh);
					fPinAngle = RounD(fRad * 180 / PI);
					if (fPinAngle < 0)
						fPinAngle += 180;
				}
				else
				{
					fPinAngle = 90;
				}
			}
			//delete pfImgDst;
			g_pMManager->pem_delete(pfImgDst, true);

			m_pProcMilAlgo->GetBlobResult_Rects(sRstAlgo->m_rcArrRect, BLOB_RECT_CNTS);

			// YJS 2017/11/23 start
			sRstAlgo->m_nArrRectCnt = nCntBlob > BLOB_RECT_CNTS ? BLOB_RECT_CNTS : nCntBlob;
			// YJS 2017/11/23 end
			if (pAlgoBlob->m_byNGCnt > 0 && pAlgoBlob->m_bUseBlobNG)
			{
				double *dArrA = NULL;
				CRect *cArrR = NULL;
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrA, nCntBlob);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &cArrR, nCntBlob);
				m_pProcMilAlgo->GetBlobResult_ALL(dArrA, NULL, NULL, cArrR);
				int nCnt = (nCntBlob > BLOB_RECT_CNTS) ? BLOB_RECT_CNTS : nCntBlob;
				for (int a = 0; a < nCnt; a++)
				{
					sRstAlgo->m_fArrRstA[a] = dArrA[a] * m_resolX * m_resolY;
					sRstAlgo->m_rcArrRect[a].left = cArrR[a].left;
					sRstAlgo->m_rcArrRect[a].right = cArrR[a].right;
					sRstAlgo->m_rcArrRect[a].top = cArrR[a].top;
					sRstAlgo->m_rcArrRect[a].bottom = cArrR[a].bottom;
					sRstAlgo->m_fArrRstW[a] = (sRstAlgo->m_rcArrRect[a].right - sRstAlgo->m_rcArrRect[a].left) * m_resolX;
					sRstAlgo->m_fArrRstL[a] = (sRstAlgo->m_rcArrRect[a].bottom - sRstAlgo->m_rcArrRect[a].top) * m_resolY;
				}
				Delete_1DArray(&dArrA);
				Delete_1DArray(&cArrR);
			}

			if (pAlgoBlob->m_bUseThickMax)
			{
				sRstAlgo->m_fRstThickMax = maxVal * dResolX;
				if (sRstAlgo->m_fRstThickMax > pAlgoBlob->m_fThickMax)
				{
					sRstAlgo->m_bOKThickMax = FALSE;
				}
				else
				{
					sRstAlgo->m_bOKThickMax = TRUE;
				}
				sRstAlgo->m_nArrRectCnt = 0;
				if (sRstAlgo->m_nArrRectCnt < 200)
				{
					sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt] = ThmaxRect;
					sRstAlgo->m_nArrRectCnt++;
				}
			}
			if (pAlgoBlob->m_bUseThickMin)
			{
				sRstAlgo->m_fRstThickMin = minVal * dResolX;
				if (sRstAlgo->m_fRstThickMin < pAlgoBlob->m_fThickMin)
				{
					sRstAlgo->m_bOKThickMin = FALSE;
				}
				else
				{
					sRstAlgo->m_bOKThickMin = TRUE;
				}
				if (!pAlgoBlob->m_bUseThickMax)
					sRstAlgo->m_nArrRectCnt = 0;
				if (sRstAlgo->m_nArrRectCnt < 200)
				{
					sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt] = ThminRect;
					sRstAlgo->m_nArrRectCnt++;
				}
			}
		}

		if (ucArrBlobDst_except)
			Delete_1DArray(&ucArrBlobDst_except);
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

	pAlgoBlob->m_dTeachHeight = dHeightMeanBody;
	if (sRstAlgo)
		sRstAlgo->m_dRstHeightMean = dHeightMeanBody;

	//2019.11.25 KYH 코일검사모드 추가
	eBlobInspMode BlobInspMode = (pAlgoBlob->m_bUseBlobNG) ? eBlobInspMode_Solderball : (pAlgoBlob->m_bInspCoil) ? eBlobInspMode_Coil : eBlobInspMode_Origin;
	int nInspAC = pAlgoBlob->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data];
	bool bInspAC = ((nInspAC & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
	bool bInspAC_Ess = (bInspAC && ((nInspAC & eDefaultAC_Data_Essential) == eDefaultAC_Data_Essential));
	bool bInspAC_WA = (bInspAC && ((nInspAC & eDefaultAC_Data_WA) == eDefaultAC_Data_WA));
	// 일반 blob 의 경우
	if (BlobInspMode == eBlobInspMode_Origin || BlobInspMode == eBlobInspMode_Coil)
	{
		if ((nCntBlob < 1) || (dArea < 1) || (rcBlob.Width() <= 1) || (rcBlob.Height() <= 1))
		{
			if (bInspAC_WA)
			{
				if (ucArrBlobDst)
					Delete_1DArray(&ucArrBlobDst);
				return bResult;
			}
		}

		m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("ClipBlob.bmp"));
		double rstW = 0.0;
		double rstH = 0.0;
		double rstD = 0.0;

		if (pAlgoBlob->m_bUseBlobSizeDistance)
		{
			std::vector<double> rstL;

			std::vector<CRect> rect;
			m_pProcMilAlgo->GetBlobResult_RectsV(rect);

			CMilBlobResult blobRes(rect.size());
			m_pProcMilAlgo->GetBlobResult(&blobRes);

			BlobGaussianLength(nImgWidth, nImgHeight, rect, blobRes.blobLabel, nCntBlob, rstL);

			for (int i = 0; i < nCntBlob; i++)
			{
				rstD += rstL[i];
			}
			if (rstL.empty() == FALSE)rstL.clear();
		}
		m_pCPInsp_Algo->GetBlobArea(ucArrBlobDst, nImgWidth, nImgHeight, dCX, dCY, rcBlob.Width(), rcBlob.Height(), dWndAngle, &rstW, &rstH);
		m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("GetBlobArea_1.bmp"));
		if (rstW < 0)
			rstW = rcBlob.Width();
		if (rstH < 0)
			rstH = rcBlob.Height();
		BlobTeachAC(nImgWidth, nImgHeight, sRstAlgo, pAlgoBlob, sInspImageData, ucArrBlobDst, sWndClipAlgo, ucArrDstImgAC, nOrgWidth, nOrgHeight, nImgWidth, nImgHeight, nClipStartX, dClipWidth, nClipStartY, dClipHeight, puImgDst);
		m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("BlobTeachAC_1.bmp"));
		if (puImgDst)
		{
			memset(puImgDst, 0, sizeof(UCHAR)*nOrgWidth*nOrgHeight);
			if (nOrgWidth > nImgWidth && nOrgWidth > nClipStartX + nImgWidth && nClipStartX >= 0 &&
				nOrgHeight > nImgHeight && nOrgHeight > nClipStartY + nImgHeight && nClipStartY >= 0)
			{
				IppiSize iSize = { nImgWidth, nImgHeight };
				IppStatus sts = ippiCopy_8u_C1R(ucArrBlobDst, nImgWidth, puImgDst + nClipStartX + nClipStartY * nOrgWidth, nOrgWidth, iSize);
				m_pProcMilAlgo->SaveWorkImg(puImgDst, nOrgWidth, nOrgHeight, _T("Origin_1.bmp"));
			}
			else
			{
				int nWidthBuf = nClipStartX + nImgWidth;
				int nHeightBuf = nClipStartY + nImgHeight;
				if (nWidthBuf > nOrgWidth)
					nWidthBuf = nOrgWidth;
				if (nHeightBuf > nOrgHeight)
					nHeightBuf = nOrgHeight;
				int nIdx = 0;
				for (int y = nClipStartY; y < nHeightBuf; y++)
				{
					for (int x = nClipStartX; x < nWidthBuf; x++)
					{
						int nIndex = (y * nOrgWidth) + x;
						puImgDst[nIndex] = ucArrBlobDst[nIdx];
						nIdx++;
					}
				}
			}
		}

		if (sRstAlgo)
		{
			cogX_roi_pixel = (coordinateAlgo.dWndX + dCX) * dResolX;
			cogY_roi_pixel = (coordinateAlgo.dWndY + dCY) * dResolY;

			if (BlobInspMode == eBlobInspMode_Origin)
			{
				double dCenTeachX = pAlgoBlob->m_dTechCenterX;
				double dCenTeachY = pAlgoBlob->m_dTechCenterY;
				if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
				{
					m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle,
						0, 0, &dCenTeachX, &dCenTeachY);
				}
				double dCenX = (nOrgWidth / 2.) + (dCenTeachX / dResolX);
				double dCenY = (nOrgHeight / 2.) - (dCenTeachY / dResolY);
				sRstAlgo->m_rcRectT.left = RounD(dCenX - (dAreaWPix / 2.0));
				sRstAlgo->m_rcRectT.top = RounD(dCenY - (dAreaHPix / 2.0));
				sRstAlgo->m_rcRectT.right = sRstAlgo->m_rcRectT.left + RounD(dAreaWPix);
				sRstAlgo->m_rcRectT.bottom = sRstAlgo->m_rcRectT.top + RounD(dAreaHPix);
				if (nBlobMode == 0 && !(pAlgoBlob->m_bUseThickMax || pAlgoBlob->m_bUseThickMin))
				{
					int nROI_W = rcBlob.Width();
					int nROI_H = rcBlob.Height();
					sRstAlgo->m_rcArrRect[0].left = dCX - (nROI_W / 2);
					sRstAlgo->m_rcArrRect[0].top = dCY - (nROI_H / 2);
					sRstAlgo->m_rcArrRect[0].right = sRstAlgo->m_rcArrRect[0].left + nROI_W;
					sRstAlgo->m_rcArrRect[0].bottom = sRstAlgo->m_rcArrRect[0].top + nROI_H;
				}
			}

			sRstAlgo->m_dRstShiftX = cogX_roi_pixel - (coordinateAlgo.dPartW / 2.0) + (sWndAlgoImg.m_fPartRoundingErrX * dResolX);
			sRstAlgo->m_dRstShiftY = (coordinateAlgo.dPartH / 2.0) - cogY_roi_pixel - (sWndAlgoImg.m_fPartRoundingErrY * dResolY);

			if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
			{
				m_proc3d.CorrectCoordinate(sRstAlgo->m_dRstShiftX, sRstAlgo->m_dRstShiftY, 0, 0, -dWndAngle,
					0, 0, &sRstAlgo->m_dRstShiftX, &sRstAlgo->m_dRstShiftY);
			}

			sRstAlgo->m_dRstArea = dArea * dResolX/*m_resolX*/ * dResolY/*m_resolY*/;
			sRstAlgo->m_dRstAreaRate = dArea / (nImgWidth * nImgHeight) * 100;

			if (pAlgoBlob->m_bAreaIsUse)
			{
				double minRange = pAlgoBlob->m_dAreaCurrent * pAlgoBlob->m_dAreaMin / 100.;
				double maxRange = pAlgoBlob->m_dAreaCurrent * pAlgoBlob->m_dAreaMax / 100.;
				if (sRstAlgo->m_dRstArea >= minRange && sRstAlgo->m_dRstArea <= maxRange)
					sRstAlgo->m_bOKArea = TRUE;
			}
			else
				sRstAlgo->m_bOKArea = TRUE;

			if (pAlgoBlob->m_bShiftIsUse)
			{
				if (fabs(sRstAlgo->m_dRstShiftX) <= pAlgoBlob->m_dShiftX)
					sRstAlgo->m_bOKShiftX = TRUE;
				if (fabs(sRstAlgo->m_dRstShiftY) <= pAlgoBlob->m_dShiftY)
					sRstAlgo->m_bOKShiftY = TRUE;
			}


			if (pAlgoBlob->m_bUseOffsetDistance)
			{
				sRstAlgo->m_bOKOffsetDistance = FALSE;
				sRstAlgo->m_fOffsetDistance = sqrt(pow(sRstAlgo->m_dRstShiftX, 2.0) + pow(sRstAlgo->m_dRstShiftY, 2.0));
				if (pAlgoBlob->m_fOffsetDistance > sRstAlgo->m_fOffsetDistance)
					sRstAlgo->m_bOKOffsetDistance = TRUE;
			}
			else
			{
				sRstAlgo->m_fOffsetDistance = 0.0f;
				sRstAlgo->m_bOKOffsetDistance = TRUE;
			}

			//////////////////////////////////////////////////////////////////////////
			sRstAlgo->m_dRstWidth = rstW * dResolX/*m_resolX*/;
			sRstAlgo->m_dRstLength = rstH * dResolY/*m_resolY*/;
			if (pAlgoBlob->m_bTeachWidthUse)
			{
				if (sRstAlgo->m_dRstWidth >= pAlgoBlob->m_dTeachWidth * pAlgoBlob->m_dTeachWidthRateMin / 100. && sRstAlgo->m_dRstWidth <= pAlgoBlob->m_dTeachWidth * pAlgoBlob->m_dTeachWidthRateMax / 100.)
					sRstAlgo->m_bOKWidth = TRUE;
			}
			if (pAlgoBlob->m_bTeachLengthUse)
			{
				if (sRstAlgo->m_dRstLength >= pAlgoBlob->m_dTeachLength * pAlgoBlob->m_dTeachLengthRateMin / 100. && sRstAlgo->m_dRstLength <= pAlgoBlob->m_dTeachLength * pAlgoBlob->m_dTeachLengthRateMax / 100.)
					sRstAlgo->m_bOKLength = TRUE;
			}

			double dRes = int(((dResolX + dResolY) / 2) * 10000) / 10000.0;
			sRstAlgo->m_dRstDistance = rstD * dRes;

			if (pAlgoBlob->m_bUsePinAngle)
			{
				sRstAlgo->m_fRstPinAngle = fPinAngle;
				sRstAlgo->m_fRstPinHeight = fPinHeight;
				if (std::abs(sRstAlgo->m_fRstPinAngle - pAlgoBlob->m_fPinAngle) <= pAlgoBlob->m_fPinAngleRange)
					sRstAlgo->m_bOKPinAngle = TRUE;
			}
		}
		bResult = TRUE;
		if (BlobInspMode == eBlobInspMode_Coil)
		{
			//신경망 초기화가 안되어있으면 초기화
			if (g_pMPTI->_inferCoil->_bInit == false)
			{
				g_pMPTI->_inferCoil->Init(BLOB_RECT_CNTS);
				if (g_pMPTI->_inferCoil->_bInit == false)
				{
					if (ucArrBlobDst)
						Delete_1DArray(&ucArrBlobDst);
					//AfxMessageBox(_T("AI Coil Model Not Loaded!"));
					return false;
				}
			}

			int nCoilBlobs = g_pMPTI->_inferCoil->_rcCoilDefects.size();
			sRstAlgo->m_nArrRectCnt = nCoilBlobs > BLOB_RECT_CNTS ? BLOB_RECT_CNTS : nCoilBlobs;

			for (int iters = 0; iters < sRstAlgo->m_nArrRectCnt; iters++)
				sRstAlgo->m_rcArrRect[iters] = g_pMPTI->_inferCoil->_rcCoilDefects[iters];

			if (nCoilBlobs > 0)
			{
				std::vector<double> vArea = g_pMPTI->_inferCoil->_rctArea;
				if (vArea.size() > 0)
				{
					int mIdx = max_element(vArea.begin(), vArea.end()) - vArea.begin();
					dArea = vArea[mIdx];
					RECT curRect = sRstAlgo->m_rcArrRect[0];
					sRstAlgo->m_rcArrRect[0] = sRstAlgo->m_rcArrRect[mIdx];
					sRstAlgo->m_rcArrRect[mIdx] = curRect;
				}
			}
		}
	}
	else if (BlobInspMode == eBlobInspMode_Solderball)	// solder ball algorithm
	{
		sRstAlgo->m_bOKShiftX = TRUE;
		sRstAlgo->m_bOKShiftY = TRUE;
		sRstAlgo->m_bOKWidth = TRUE;
		sRstAlgo->m_bOKLength = TRUE;
		sRstAlgo->m_bOKArea = TRUE;
		sRstAlgo->m_bOKDistance = TRUE;
		sRstAlgo->m_bOKOffsetDistance = TRUE;

		double dStandardArea = pAlgoBlob->m_dBlobSizeWidth * pAlgoBlob->m_dBlobSizeLength; // * m_resolX * m_resolY;		

		m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("ClipBlob.bmp"));

		double rstW = 0.0;
		double rstH = 0.0;
		m_pCPInsp_Algo->GetBlobArea(ucArrBlobDst, nImgWidth, nImgHeight, dCX, dCY, rcBlob.Width(), rcBlob.Height(), dWndAngle, &rstW, &rstH);
		BlobTeachAC(nImgWidth, nImgHeight, sRstAlgo, pAlgoBlob, sInspImageData, ucArrBlobDst, sWndClipAlgo, ucArrDstImgAC, nOrgWidth, nOrgHeight, nImgWidth, nImgHeight, nClipStartX, dClipWidth, nClipStartY, dClipHeight, puImgDst);

		bool bSwitch = g_pMPTI->m_bLengthByAngel;
		cv::Point blobCenter;

		float fResult(0.0);
		double dMaxWidth(0.0);
		if (nInspType == eINSP_S_BALL)
		{
			double diameter = (pAlgoBlob->m_dBlobSizeWidth) / dResolX;
			std::vector<CRect> vBlobRects;
			m_pProcMilAlgo->GetBlobResult_RectsV(vBlobRects);
			double *dArrScore = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrScore, nCntBlob);
			cv::Mat model(nImgHeight, nImgWidth, CV_8UC1), m_Integ, m_Dev;
			memcpy(model.data, ucArrBlobDst, sizeof(UCHAR)*nImgWidth*nImgHeight);
			int nMaxIndex(0);
			m_pProcMilAlgo->InspSolderBall(model, vBlobRects, &nMaxIndex, &fResult, dArrScore);
			int nCnt = (nCntBlob > BLOB_RECT_CNTS) ? BLOB_RECT_CNTS : nCntBlob;
			if (vBlobRects.size() > nMaxIndex)
				dMaxWidth = vBlobRects[nMaxIndex].Width() * m_resolX;
			for (int a = 0; a < nCnt; a++)
				sRstAlgo->m_fArrRstL[a] = dArrScore[a] * 100.0;
			cv::integral(model, m_Integ, m_Dev);
			Delete_1DArray(&dArrScore);

			if (sRstAlgo->m_nArrRectCnt > nMaxIndex && nInspType == eINSP_S_BALL && pAlgoBlob->m_byNGCnt == 0 && pAlgoBlob->m_bUseBlobNG)
			{
				RECT rcMax = sRstAlgo->m_rcArrRect[nMaxIndex];
				memset(sRstAlgo->m_rcArrRect, 0, BLOB_RECT_CNTS * sizeof(RECT));
				sRstAlgo->m_nArrRectCnt = 0;

				sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].left = rcMax.left;
				sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].right = rcMax.right;
				sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].top = rcMax.top;
				sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].bottom = rcMax.bottom;
				sRstAlgo->m_nArrRectCnt++;
			}
		}

		if (rstW < 0)
			rstW = rcBlob.Width();
		if (rstH < 0)
			rstH = rcBlob.Height();

		if (puImgDst)
		{
			memset(puImgDst, 0, sizeof(UCHAR)*nOrgWidth*nOrgHeight);
			if (nOrgWidth > nImgWidth && nOrgWidth > nClipStartX + nImgWidth && nClipStartX >= 0 &&
				nOrgHeight > nImgHeight && nOrgHeight > nClipStartY + nImgHeight && nClipStartY >= 0)
			{
				IppiSize iSize = { nImgWidth, nImgHeight };
				IppStatus sts = ippiCopy_8u_C1R(ucArrBlobDst, nImgWidth, puImgDst + nClipStartX + nClipStartY * nOrgWidth, nOrgWidth, iSize);
				m_pProcMilAlgo->SaveWorkImg(puImgDst, nOrgWidth, nOrgHeight, _T("Origin_1.bmp"));
			}
			else
			{
				int nWidthBuf = nClipStartX + nImgWidth;
				int nHeightBuf = nClipStartY + nImgHeight;
				if (nWidthBuf > nOrgWidth)
					nWidthBuf = nOrgWidth;
				if (nHeightBuf > nOrgHeight)
					nHeightBuf = nOrgHeight;
				int nIdx = 0;
				for (int y = nClipStartY; y < nHeightBuf; y++)
				{
					for (int x = nClipStartX; x < nWidthBuf; x++)
					{
						int nIndex = (y * nOrgWidth) + x;
						puImgDst[nIndex] = ucArrBlobDst[nIdx];
						nIdx++;
					}
				}
			}
		}

		if (nCntBlob < 1)
		{
			bResult = TRUE;	// blob 영역이 없으면 OK이다
			sRstAlgo->m_bOKArea_Reverse = TRUE;
			if (ucArrBlobDst)
				Delete_1DArray(&ucArrBlobDst);
			return bResult;
		}

		if (sRstAlgo)
		{
			cogX_roi_pixel = (double)dROICenterX - (nImgWidth / 2.) + dCX;
			cogY_roi_pixel = (double)dROICenterY - (nImgHeight / 2.) + dCY;

			sRstAlgo->m_dRstShiftX = (cogX_roi_pixel*dResolX - nOrgWidth / 2 * dResolX);
			sRstAlgo->m_dRstShiftY = (nOrgHeight / 2 * dResolY - cogY_roi_pixel * dResolY);

			if ((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
			{
				m_proc3d.CorrectCoordinate(sRstAlgo->m_dRstShiftX, sRstAlgo->m_dRstShiftY, 0, 0, -dWndAngle,
					0, 0, &sRstAlgo->m_dRstShiftX, &sRstAlgo->m_dRstShiftY);
			}

			sRstAlgo->m_dRstArea_Reverse = dArea * dResolX * dResolY;
			if (pAlgoBlob->m_byNGCnt > 0 && pAlgoBlob->m_bUseBlobNG)
			{
				sRstAlgo->m_nArrRectCnt = 0;
				sRstAlgo->m_bOKArea_Reverse = TRUE;
				sRstAlgo->m_bOKWidth = TRUE;
				sRstAlgo->m_bOKLength = TRUE;
				int nCnt = (nCntBlob > BLOB_RECT_CNTS) ? BLOB_RECT_CNTS : nCntBlob;
				float *fArrW = NULL;
				float *fArrA = NULL;
				float *fArrL = NULL;
				RECT *rArrR = NULL;
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrW, BLOB_RECT_CNTS);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrA, BLOB_RECT_CNTS);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &fArrL, BLOB_RECT_CNTS);
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &rArrR, BLOB_RECT_CNTS);
				memcpy(fArrW, sRstAlgo->m_fArrRstW, BLOB_RECT_CNTS * sizeof(float));
				memcpy(fArrA, sRstAlgo->m_fArrRstA, BLOB_RECT_CNTS * sizeof(float));
				memcpy(fArrL, sRstAlgo->m_fArrRstL, BLOB_RECT_CNTS * sizeof(float));
				memcpy(rArrR, sRstAlgo->m_rcArrRect, BLOB_RECT_CNTS * sizeof(RECT));
				memset(sRstAlgo->m_fArrRstA, 0, BLOB_RECT_CNTS * sizeof(float));
				memset(sRstAlgo->m_fArrRstW, 0, BLOB_RECT_CNTS * sizeof(float));
				memset(sRstAlgo->m_fArrRstL, 0, BLOB_RECT_CNTS * sizeof(float));
				memset(sRstAlgo->m_rcArrRect, 0, BLOB_RECT_CNTS * sizeof(RECT));
				sRstAlgo->m_dRstArea_Reverse = 0;
				sRstAlgo->m_dRstWidth = 0;
				sRstAlgo->m_dRstLength = 0;
				for (int a = 0; a < nCnt; a++)
				{
					if (fArrA[a] > sRstAlgo->m_dRstArea_Reverse) sRstAlgo->m_dRstArea_Reverse = fArrA[a];
					if (fArrW[a] > sRstAlgo->m_dRstWidth) sRstAlgo->m_dRstWidth = fArrW[a];
					if (fArrL[a] > sRstAlgo->m_dRstLength) sRstAlgo->m_dRstLength = fArrL[a];
					if ((pAlgoBlob->m_bUseBlobNG && fArrA[a] >= dStandardArea) ||
						(pAlgoBlob->m_bUseBlobSizeWidth && fArrW[a] >= pAlgoBlob->m_dBlobSizeWidth) ||
						(pAlgoBlob->m_bUseBlobSizeLength && fArrL[a] >= pAlgoBlob->m_dBlobSizeLength))
					{
						sRstAlgo->m_fArrRstA[sRstAlgo->m_nArrRectCnt] = fArrA[a];
						sRstAlgo->m_fArrRstW[sRstAlgo->m_nArrRectCnt] = fArrW[a];
						sRstAlgo->m_fArrRstL[sRstAlgo->m_nArrRectCnt] = fArrL[a];
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].left = rArrR[a].left;
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].right = rArrR[a].right;
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].top = rArrR[a].top;
						sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].bottom = rArrR[a].bottom;
						sRstAlgo->m_nArrRectCnt++;
					}
				}
				Delete_1DArray(&fArrW);
				Delete_1DArray(&fArrA);
				Delete_1DArray(&fArrL);
				Delete_1DArray(&rArrR);
				if (sRstAlgo->m_nArrRectCnt >= pAlgoBlob->m_byNGCnt)
				{
					if (pAlgoBlob->m_bUseBlobNG && sRstAlgo->m_dRstArea_Reverse >= dStandardArea)
						sRstAlgo->m_bOKArea_Reverse = FALSE;
					if (pAlgoBlob->m_bUseBlobSizeWidth && sRstAlgo->m_dRstWidth >= pAlgoBlob->m_dBlobSizeWidth)
						sRstAlgo->m_bOKWidth = FALSE;
					if (pAlgoBlob->m_bUseBlobSizeLength && sRstAlgo->m_dRstLength >= pAlgoBlob->m_dBlobSizeLength)
						sRstAlgo->m_bOKLength = FALSE;
				}
			}
			else
			{
				if (pAlgoBlob->m_bUseBlobNG)
				{
					if (sRstAlgo->m_dRstArea_Reverse >= dStandardArea)
						sRstAlgo->m_bOKArea_Reverse = FALSE;
					else
						sRstAlgo->m_bOKArea_Reverse = TRUE;
				}
				else
					sRstAlgo->m_bOKArea_Reverse = TRUE;

				if (pAlgoBlob->m_bUseBlobSizeWidth)
				{
					sRstAlgo->m_dRstWidth = rstW * dResolX;
					if (nInspType == eINSP_S_BALL)
						sRstAlgo->m_dRstWidth = dMaxWidth;
					if (sRstAlgo->m_dRstWidth < pAlgoBlob->m_dBlobSizeWidth)
						sRstAlgo->m_bOKWidth = TRUE;
					else
						sRstAlgo->m_bOKWidth = FALSE;
				}
				else
					sRstAlgo->m_bOKWidth = TRUE;

				if (pAlgoBlob->m_bUseBlobSizeLength)
				{
					sRstAlgo->m_dRstLength = rstH * dResolY;
					if (nInspType == eINSP_S_BALL)
						sRstAlgo->m_dRstLength = fResult * 100; // %
					if (sRstAlgo->m_dRstLength < pAlgoBlob->m_dBlobSizeLength)
						sRstAlgo->m_bOKLength = TRUE;
					else
						sRstAlgo->m_bOKLength = FALSE;
				}
				else
					sRstAlgo->m_bOKLength = TRUE;
			}

			double rstD = 0;
			if (pAlgoBlob->m_bUseBlobSizeDistance)
			{
				std::vector<CRect> rect;
				m_pProcMilAlgo->GetBlobResult_RectsV(rect);

				CMilBlobResult blobRes(rect.size());
				m_pProcMilAlgo->GetBlobResult(&blobRes);

				if (pAlgoBlob->m_nBlobSizeDistanceType == 1 && pAlgoBlob->m_nTypeSelectBlob == 0)
				{
					double dRstL = 0.0;
					int nDir = 0;
					std::vector<cv::Point> tmpPtr;
					BlobShiftDistance(nImgWidth, nImgHeight, rect, blobRes.blobLabel, nCntBlob, dRstL, nDir, tmpPtr, pAlgoBlob->m_dBlobSizeDistance);

					if (dRstL <= 0)
					{
						rstD = 0.0;
					}
					else
					{
						if (tmpPtr.size() > 0)
						{
							sRstAlgo->m_nShiftDirIndex = nDir;

							sRstAlgo->m_pShiftSt.x = tmpPtr[0].x;
							sRstAlgo->m_pShiftSt.y = tmpPtr[0].y;
							sRstAlgo->m_pShiftEd.x = tmpPtr[1].x;
							sRstAlgo->m_pShiftEd.y = tmpPtr[1].y;
							rstD = dRstL;
						}
						else
							rstD = dRstL;
					}
				}

			}


			if (bSwitch && (g_pInspMng->FP_CenterPtr.x > 0 && g_pInspMng->FP_CenterPtr.x < nImgWidth && g_pInspMng->FP_CenterPtr.y > 0 && g_pInspMng->FP_CenterPtr.y < nImgHeight)
				&& pAlgoBlob->m_nTypeSelectBlob == 0)
			{
				float halfX = rtFPArea.width / 4;	//기본 25%
				float halfY = rtFPArea.height / 4;

				if (nCntBlob == 1)
				{
					/*double* dCenterX= new double[nCntBlob];
					double* dCenterY= new double[nCntBlob];*/
					double* dCenterX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
					double* dCenterY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
					m_pProcMilAlgo->GetBlobResult_Center(dCenterX, dCenterY);

					double cx = dCenterX[0];
					double cy = dCenterY[0];

					if ((rtFPArea.x < cx && cx < rtFPArea.x + halfX && rtFPArea.y + halfY < cy && cy < rtFPArea.y + (halfY * 3))
						|| rtFPArea.x + (halfX * 3) < cx && cx < rtFPArea.x + rtFPArea.width && rtFPArea.y + halfY < cy && cy < rtFPArea.y + (halfY * 3))
					{
						double tmpLength = sRstAlgo->m_dRstLength;
						sRstAlgo->m_dRstLength = sRstAlgo->m_dRstWidth;
						sRstAlgo->m_dRstWidth = tmpLength;

						if (sRstAlgo->m_dRstLength < pAlgoBlob->m_dBlobSizeLength)
							sRstAlgo->m_bOKLength = TRUE;
						else
							sRstAlgo->m_bOKLength = FALSE;

						if (sRstAlgo->m_dRstWidth < pAlgoBlob->m_dBlobSizeWidth)
							sRstAlgo->m_bOKWidth = TRUE;
						else
							sRstAlgo->m_bOKWidth = FALSE;
					}

					/*delete [] dCenterX;
					delete [] dCenterY;*/
					g_pMManager->pem_delete(dCenterX, true);
					g_pMManager->pem_delete(dCenterY, true);
				}

			}
		}
		bResult = TRUE;
	}
	Delete_1DArray(&ucColorImgDst);
	if (ucArrBlobDst)
		Delete_1DArray(&ucArrBlobDst);
	return bResult;
}
POINTF CPInsp_AlgoBlob::CalcForiegnPatternCenter(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, double dWndCX, double dWndCY, double dPartW, double dPartH,
	TotalInspExceptArea stTieArea, int nInspType, cv::Rect& rtRst, UCHAR *pUcImgBlob)
{
	BOOL bResult = FALSE;

	int nLine = __LINE__;
	try
	{

		POINTF centerPtr;
		centerPtr.x = 0;
		centerPtr.y = 0;
		if (sInspAlgo.m_eAlgoType != eAlgoBlob)
			return centerPtr;
		if (!m_pProcMilAlgo)
			return centerPtr;
		AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
		if (!pAlgoBlob)
			return centerPtr;
		if (!pAlgoBlob->m_bInsp2D && !pAlgoBlob->m_bInsp3D && !pAlgoBlob->m_sAlgoColorBase.m_bUseColor)
			return centerPtr;

		UCHAR *pUcImgSrc;
		if (sWndAlgoImg.m_ucArr2D_Mix[0] != NULL)
			pUcImgSrc = sWndAlgoImg.m_ucArr2D_Mix[0]/*sWndAlgoImg.m_ucArr2D*/;
		else
			pUcImgSrc = sWndAlgoImg.m_ucArr2D;

		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nImgWidth = sWndAlgoImg.m_nWidth;
		int nImgHeight = sWndAlgoImg.m_nHeight;
		if ((nImgWidth <= 0) || (nImgHeight <= 0))
			return centerPtr;

		nLine = __LINE__;



		// 통합 검사/제외 영역에 넣어준다
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

		double dCX = 0;
		double dCY = 0;
		double dArea = 0;
		int nMinBlobArea = 4;
		if (pAlgoBlob->m_nTypeSelectBlob == eSelectMix && pAlgoBlob->m_bUseBlobNG == TRUE)
			nMinBlobArea = 0;
		CRect rcBlob(0, 0, 0, 0);
		if (pUcImgBlob == NULL)
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pUcImgBlob, nImgWidth * nImgHeight);

		UCHAR *ucColorImgDst = NULL;

		int nCntBlob = 0;
		BOOL IsBinImage = FALSE;

		AlgoBlob FP_Blob;
		FP_Blob = (AlgoBlob)*pAlgoBlob;
		FP_Blob.m_bInsp2D = pAlgoBlob->m_bInsp2D_pattern;
		FP_Blob.m_nMinBinary = pAlgoBlob->m_nMinBinary_pattern;
		FP_Blob.m_nMaxBinary = pAlgoBlob->m_nMaxBinary_pattern;
		FP_Blob.m_nTypeRange2D = pAlgoBlob->m_nTypeRange2D_pattern;
		FP_Blob.m_bInsp3D = pAlgoBlob->m_bInsp3D_pattern;
		FP_Blob.m_dHeightRateMin = pAlgoBlob->m_dHeightRateMin_pattern;
		FP_Blob.m_dHeightRateMax = pAlgoBlob->m_dHeightRateMax_pattern;
		FP_Blob.m_nTypeRange3D = pAlgoBlob->m_nTypeRange3D_pattern;

		FP_Blob.m_nTypeSelectBlob = eSelectBigger;
		nCntBlob = m_pCPInsp_Algo->BlobImageStruct(FP_Blob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidth, nImgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, pUcImgBlob, stTieArea, pAlgoBlob->m_bFillHole);

		Delete_1DArray(&ucColorImgDst);
		ucColorImgDst = NULL;
		m_pProcMilAlgo->SaveWorkImg(pUcImgSrc, nImgWidth, nImgHeight, _T("Blob_2D_Allblob.bmp"));
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Blob_3D_Allblob.bmp"));
		m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidth, nImgHeight, _T("Blob_Rst_Allblob.bmp"));

		cv::Mat Mat_res(nImgHeight, nImgWidth, CV_8UC1, pUcImgBlob);

		cv::Mat Center_res = Mat_res.clone();

		std::vector<cv::Point> pt;

		for (int i = 0; i < nImgHeight; i++)
		{
			for (int j = 0; j < nImgWidth; j++)
			{
				cv::Point tmpPt;
				if (Center_res.data[i * nImgWidth + j] > 200)
				{
					tmpPt.x = j;
					tmpPt.y = i;
					pt.push_back(tmpPt);
				}
			}
		}

		if (pt.size() <= 0)
		{
			if (pUcImgBlob != NULL)
				Delete_1DArray(&pUcImgBlob);
			if (!Center_res.empty())
				Center_res.release();
			return centerPtr;
		}

		cv::RotatedRect rt = cv::minAreaRect(pt);

		cv::Point2f pts[4];
		rt.points(pts);

		cv::Mat DrawingImg(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));

		float stX = nImgWidth * 10;
		float stY = nImgHeight * 10;
		float edX = 0;
		float edY = 0;

		for (int i = 0; i < 4; i++)
		{
			cv::line(DrawingImg, pts[i % 4], pts[(i + 1) % 4], 255);

			if (pts[i].x < stX)
				stX = pts[i].x;
			if (pts[i].y < stY)
				stY = pts[i].y;
			if (pts[i].x > edX)
				edX = pts[i].x;
			if (pts[i].y > edY)
				edY = pts[i].y;
		}

		if (stX < 0) stX = 0;
		if (stY < 0) stY = 0;
		if (edX >= nImgWidth) edX = nImgWidth - 1;
		if (edY >= nImgHeight) edY = nImgHeight - 1;

		centerPtr.x = rt.center.x;
		centerPtr.y = rt.center.y;

		cv::Point pCenter;
		pCenter.x = centerPtr.x + 0.4;
		pCenter.y = centerPtr.y + 0.4;
		g_pInspMng->ForeingLineImgSet(DrawingImg.data, DrawingImg.cols, DrawingImg.rows, pCenter);

		rtRst.x = stX;
		rtRst.y = stY;
		rtRst.width = edX - stX;
		rtRst.height = edY - stY;

		if (pt.size() > 0)
			pt.clear();

		if (pUcImgBlob != NULL)
			Delete_1DArray(&pUcImgBlob);
		Center_res.release();

		return centerPtr;
	}
	catch (...)
	{

		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Algo::CalcForiegnPatternCenter(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
}

void CPInsp_AlgoBlob::BlobGaussianLength(int nImgWidth, int nImgLength, std::vector<CRect> blobRect, int * nLabel, int cnt, std::vector<double>& rstD)
{
	for (int i = 0; i < cnt; i++)
	{
		cv::Mat BinImg = cv::Mat::zeros(nImgLength, nImgWidth, CV_8UC1);
		m_pProcMilAlgo->DrawBlob(m_pProcMilAlgo->GetBlobResultId(), (Im::PIL_ID)&BinImg, nLabel[i]);

		cv::Mat BinImgRoi = BinImg(cv::Rect(blobRect[i].left, blobRect[i].top, blobRect[i].Width(), blobRect[i].Height()));
		_GaussianLength(BinImgRoi.ptr(), BinImgRoi.cols, BinImgRoi.rows, BinImgRoi.step, rstD);

	}
}
void CPInsp_AlgoBlob::_GaussianLength(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch, std::vector<double>& rstD)
{
	cv::Mat src(nImgHeight, nImgWidth, CV_8UC1, pByImgSrc, nPitch);

	cv::Mat temp_src;
	src.copyTo(temp_src);
	int width = 0;

	cv::Mat thre_img;

	cv::Mat Mask_img(src.size(), CV_16SC1, cv::Scalar(0));
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(temp_src.ptr(), temp_src.ptr(), temp_src.cols, temp_src.rows, 4, FALSE, FALSE, FALSE, eSelectMix);

	cv::Mat result_img(temp_src.size(), CV_8UC1, cv::Scalar(0));
	width = m_pProcMilAlgo->GetBlobPerimeter(result_img.ptr(), Mask_img.ptr<USHORT>(), temp_src.cols, temp_src.rows, temp_src.step);

	int blobCount = nCntBlob;

	Mask_img.setTo(0);
	thre_img.setTo(0);
	result_img.setTo(0);

	//¿¹¿Ü Ã³¸®
	//Circle (°è»êµÈ Width°¡ Blob Width, HeightÀÇ 90% ÀÌ»ó)ÀÌ¸é Width(Áö¸§) ¸®ÅÏ
	if (width > 1 && width > nImgWidth*0.9 && width > nImgHeight *0.9)
	{
		rstD.push_back(width);
		return;
	}

	if (width % 2 == 0)width++;

	Mask_img = cv::Mat::zeros(src.rows + width * 2 + 4, src.cols + width * 2 + 4, CV_16SC1);
	thre_img = cv::Mat::zeros(src.rows + width * 2 + 4, src.cols + width * 2 + 4, CV_8UC1);
	result_img = cv::Mat::zeros(src.rows + width * 2 + 4, src.cols + width * 2 + 4, CV_8UC1);

	cv::Mat Extend = cv::Mat::zeros(src.rows + width * 2 + 4, src.cols + width * 2 + 4, CV_8UC1);
	cv::Mat ExtendROI = Extend(cv::Rect(width + 2, width + 2, src.cols, src.rows));
	src.copyTo(ExtendROI);

	cv::GaussianBlur(Extend, Extend, cv::Size(width, width), 0);
	cv::threshold(Extend, thre_img, 0, 255, cv::THRESH_BINARY);

	int tempWidth = width;

	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(thre_img.ptr(), thre_img.ptr(), thre_img.cols, thre_img.rows, 4, FALSE, FALSE, FALSE, eSelectMix);
	width = m_pProcMilAlgo->GetBlobPerimeter(result_img.ptr(), Mask_img.ptr<USHORT>(), thre_img.cols, thre_img.rows, thre_img.step);

	float rstLength = 0;
	rstLength = m_pProcMilAlgo->GetPerimeterLength();
	rstD.push_back(rstLength - tempWidth);

}
void CPInsp_AlgoBlob::BlobShiftDistance(int nImgWidth, int nImgLength, std::vector<CRect> blobRect, int * nLabel, int cnt, double& nRstD, int& nDir_Index, std::vector<cv::Point>& vLinePtr, double nStdLength)
{
	for (int i = 0; i < cnt; i++)
	{
		cv::Mat BinImg = cv::Mat::zeros(nImgLength, nImgWidth, CV_8UC1);
		m_pProcMilAlgo->DrawBlob(m_pProcMilAlgo->GetBlobResultId(), (Im::PIL_ID)&BinImg, nLabel[i]);

		//cv::imwrite("D:\\BinOrgImg.bmp", BinImg);

		int nWidth = blobRect[i].Width();
		int nHeight = blobRect[i].Height();
		int nStdL = (nStdLength * 1000) / 5;
		if (nWidth > nStdL || nHeight > nStdL)
		{
			cv::Mat BinImgRoi = BinImg(cv::Rect(blobRect[i].left, blobRect[i].top, blobRect[i].Width(), blobRect[i].Height()));
			_ShiftLength(BinImgRoi.ptr(), BinImgRoi.cols, BinImgRoi.rows, BinImgRoi.step, nRstD, nDir_Index, vLinePtr);
		}

		for (int j = 0; j < vLinePtr.size(); j++)
		{
			vLinePtr[j].x += blobRect[i].left;
			vLinePtr[j].y += blobRect[i].top;
		}

		if (nDir_Index == -1)	//´Ü¹æÇâ ½ÃÇÁÆ®ÀÇ °æ¿ì, ¹æÇâ ¸ø±¸ÇßÀ»¶§
		{
			double m_fDegree = (atan2f((float)nImgLength / 2 - (blobRect[i].top + blobRect[i].Height() / 2), (float)nImgWidth / 2 - (blobRect[i].left + blobRect[i].Width() / 2)) * 180 / 3.1415f) + 180;

			if (m_fDegree >= 45 && m_fDegree <= 135)			//Bottom
			{
				nDir_Index = 7;
			}
			else if (m_fDegree > 135 && m_fDegree < 225)		//Left
			{
				nDir_Index = 8;
			}
			else if (m_fDegree >= 225 && m_fDegree <= 315)	//Top
			{
				nDir_Index = 5;
			}
			else if ((m_fDegree > 315 && m_fDegree <= 360) || (m_fDegree >= 0 && m_fDegree < 45) || m_fDegree > 360)	//Right
			{
				nDir_Index = 6;
			}
		}
	}
}
void CPInsp_AlgoBlob::_ShiftLength(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch, double& nRstD, int& sDir_Index, std::vector<cv::Point>& vLinePtr)
{
	cv::Mat src(nImgHeight, nImgWidth, CV_8UC1, pByImgSrc, nPitch);

	bool bSkip = CheckBlobShiftImg(src.ptr(), nImgWidth, nImgHeight, nPitch);


	/////shift ¹æÇâ ÆÇ´Ü
	int nShiftCase = 2;	//¾ç¹æÇâ ÃÊ±âÈ­

	int max = nImgWidth;
	int min = nImgHeight;
	if (max < min)
	{
		max = nImgHeight;
		min = nImgWidth;
	}

	if ((max * 0.5) > min)
		nShiftCase = 1;		//´Ü¹æÇâ

	double rstShift = 0.0;

	if (bSkip == true)
	{
		cv::Mat temp_src(src.size(), src.type(), cv::Scalar(0));
		std::vector<cv::Point2f> vPoint;
		cv::Mat rect_img(src.size(), src.type(), cv::Scalar(0));
		int ret = BlobShiftImgSet(src.ptr(), nImgWidth, nImgHeight, temp_src.ptr(), rect_img.ptr(), src.step, vPoint, nShiftCase);


		if (ret == 1)	//1 = 계산 가능 형태, 2 = 계산가능 불가의 단방향
		{
			//Blob Sequence
			cv::Mat Mask_img(temp_src.size(), CV_16SC1, cv::Scalar(0));
			
			int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(temp_src.ptr(), temp_src.ptr(), temp_src.cols, temp_src.rows, 4, FALSE, FALSE, FALSE, eSelectBigger);

			cv::Mat result_img(temp_src.size(), CV_8UC1, cv::Scalar(0));
			m_pProcMilAlgo->GetDrawContours(result_img.ptr(), result_img.cols, result_img.rows, result_img.step, 255);

			//nRstD = rstShift;

			//Á¡°ú ¹æÇâ¼º Ã£±â
			cv::threshold(result_img, result_img, 0, 255, cv::THRESH_BINARY);
			int nLineIndex = SearchBlobShiftLine(result_img.ptr(), result_img.cols, result_img.rows, result_img.step, rect_img.ptr(), vPoint, nShiftCase);

			for (int i = 0; i < vPoint.size(); i++)
			{
				vLinePtr.emplace_back(vPoint[i]);
			}
			bool nShiftchange = false;
			if (nShiftCase == 2 && vLinePtr[0].x == vLinePtr[1].x)
				nShiftchange = true;
			if (nShiftCase == 2 && vLinePtr[0].y == vLinePtr[1].y)
				nShiftchange = true;

			if (nShiftchange == true && nShiftCase == 2)
				nShiftCase = 1;

			if (nShiftCase == 2)
			{
				if (vLinePtr[0].x > vLinePtr[1].x)
				{
					std::swap(vLinePtr[0], vLinePtr[1]);
				}
				cv::Mat mMap(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
				double x1 = vLinePtr[0].x;
				double y1 = vLinePtr[0].y;
				double x2 = vLinePtr[1].x;
				double y2 = vLinePtr[1].y;

				double ax = (y2 - y1) / (x2 - x1);
				double ay = y1 - (ax * x1);



				std::vector<cv::Point> pComparePtr;

				for (int x = 0; x < nImgWidth; x++)
				{
					double i = x;

					double y = ax * i + ay;

					if (y > 0 && y < nImgHeight)
					{
						int cnt = pComparePtr.size();
						bool bSame = false;
						for (int size = 0; size < cnt; size++)
						{
							if (cv::Point(x, (int)y) == pComparePtr[size])
							{
								bSame = true;
								break;
							}
						}

						if (bSame == false && src.data[(int)y * src.step + x] == 255)
							pComparePtr.emplace_back(cv::Point(x, y));
					}
				}
				if (pComparePtr.size() <= 0)
				{
					nRstD = max;
					vLinePtr.clear();
					return;
				}
				else
				{
					x1 = pComparePtr[0].x;
					y1 = pComparePtr[0].y;
					x2 = pComparePtr[pComparePtr.size() - 1].x;
					y2 = pComparePtr[pComparePtr.size() - 1].y;
					double euX = (x2 - x1);
					double euY = (y2 - y1);
					double euclipDist = sqrt((euX * euX) + (euY * euY));

					vLinePtr.clear();
					vLinePtr.emplace_back(cv::Point(x1, y1));
					vLinePtr.emplace_back(cv::Point(x2, y2));

					nRstD = euclipDist;
				}
			}
			else
			{
				double x1 = vLinePtr[0].x;
				double y1 = vLinePtr[0].y;
				double x2 = vLinePtr[1].x;
				double y2 = vLinePtr[1].y;
				double euX = (x2 - x1);
				double euY = (y2 - y1);
				double euclipDist = sqrt((euX * euX) + (euY * euY));
				nRstD = euclipDist;
			}


			sDir_Index = nLineIndex;
		}
		else if (ret == 2)	//Èò»ö ºñÀ²ÀÌ 90%¸¦ ³Ñ¾î°¡´Â ¹æÇâ¼ºÀ» ÆÇ´ÜÇÒ ¼ö ¾ø´Â ¿µ»ó
		{
			if (nImgHeight < nImgWidth)
				nRstD = nImgHeight;
			else if (nImgHeight > nImgWidth)
				nRstD = nImgWidth;

			//Æ÷ÀÎÆ®¿Í ¹æÇâ¼º ¸®ÅÏ
			for (int i = 0; i < vPoint.size(); i++)
			{
				vLinePtr.emplace_back(vPoint[i]);
			}

			sDir_Index = -1;		//¹æÇâÀ» ¸ø±¸Çß´Ù´Â ¶æ
		}

	}

}
bool CPInsp_AlgoBlob::CheckBlobShiftImg(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch)
{
	bool ret = true;
	cv::Mat srctmp(nImgHeight, nImgWidth, CV_8UC1, pByImgSrc, nPitch);
	cv::Mat src = srctmp.clone();
	bool bLeft = true, bRight = true, bTop = true, bBottom = true;

	int nCenterX = src.cols / 2;
	int nCenterY = src.rows / 2;

	if (src.data[nCenterY * src.cols + nCenterX] == 255)
	{
		if (src.empty() == FALSE)
			src.release();
		return true;
	}

	cv::Mat map(nImgHeight, nImgWidth, src.type(), cv::Scalar(0));
	//left
	for (int i = 0; i <= nCenterX; i++)
	{
		int dx = nCenterX - i;
		map.data[nCenterY * src.cols + dx] = 255;
		if (src.data[nCenterY * src.cols + dx] == 255)
		{
			bLeft = false;
			break;
		}
	}

	//right
	for (int i = nCenterX; i < src.cols; i++)
	{
		map.data[nCenterY * src.cols + i] = 255;
		if (src.data[nCenterY * src.cols + i] == 255)
		{
			bRight = false;
			break;
		}
	}

	//top
	for (int j = 0; j <= nCenterY; j++)
	{
		int dy = nCenterY - j;
		map.data[dy * src.cols + nCenterX] = 255;
		if (src.data[dy * src.cols + nCenterX] == 255)
		{
			bTop = false;
			break;
		}
	}

	//bottom
	for (int j = nCenterY; j < src.rows; j++)
	{
		map.data[j * src.cols + nCenterX] = 255;
		if (src.data[j * src.cols + nCenterX] == 255)
		{
			bBottom = false;
			break;
		}
	}

	if (bLeft == false && bRight == false && bTop == false && bBottom == false)
		ret = false;

	if (src.empty() == FALSE)
		src.release();

	return ret;
}
int CPInsp_AlgoBlob::BlobShiftImgSet(UCHAR* pByImgSrc, int nImgWidth, int nImgHeight, UCHAR *dstByimgSrc, UCHAR *rectByimgSrc, int nPicth, std::vector<cv::Point2f>& vPoint, int nShiftCase)
{
	int ret = 0;

	cv::Mat src(nImgHeight, nImgWidth, CV_8UC1, pByImgSrc, nPicth);
	cv::Mat dst(nImgHeight, nImgWidth, CV_8UC1, dstByimgSrc);
	cv::Mat rectimg(nImgHeight, nImgWidth, CV_8UC1, rectByimgSrc);

	cv::Mat element(5, 5, CV_8U, cv::Scalar(1));

	if (nShiftCase == 1)
	{
		cv::Mat Extend = cv::Mat::zeros(src.rows + 4, src.cols + 4, CV_8UC1);
		cv::Mat ExtendROI = Extend(cv::Rect(2, 2, src.cols, src.rows));
		src.copyTo(ExtendROI);
		cv::morphologyEx(Extend, Extend, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 1);//3,4,5
		src.setTo(0);
		src = ExtendROI.clone();
	}
	else
		cv::morphologyEx(src, src, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 1);//3,4,5

	double dRate = 0.0;

	cv::Mat OrgSizeMat(src.size(), src.type(), cv::Scalar(0));
	cv::bitwise_not(src, OrgSizeMat);

	cv::Mat _invMat(OrgSizeMat.rows + 2, OrgSizeMat.cols + 2, OrgSizeMat.type(), cv::Scalar(0));

	//¿øº»¿µ»ó Size+2 ¿µ»ó »ý¼º
	for (int i = 0; i < OrgSizeMat.rows; i++)
	{
		for (int j = 0; j < OrgSizeMat.cols; j++)
		{
			int dx = j + 1;
			int dy = i + 1;
			if (OrgSizeMat.data[i * OrgSizeMat.cols + j] > 200)
			{
				_invMat.data[dy * _invMat.cols + dx] = 255;
			}
			if (OrgSizeMat.data[i * OrgSizeMat.cols + j] == 0)
				dRate++;
		}
	}

	dRate = (dRate / (src.rows * src.cols)) * 100;

	if (dRate <= 90)
	{
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(_invMat, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);


		std::vector<std::vector<cv::Point> >hull(contours.size());
		for (size_t i = 0; i < contours.size(); i++)
		{
			convexHull(cv::Mat(contours[i]), hull[i], false);
		}

		//Max Blob °ËÃâ
		cv::Mat inv_maxBlob(_invMat.size(), CV_8UC1, cv::Scalar(0));

		int nMaxBlobidx = 0;
		int nMaxAreaSize = 0;
		for (int i = 0; i < contours.size(); i++)
		{
			int area = contours[i].size();			//Blob °´Ã¼ Å©±â(ÇÈ¼¿°³¼ö)
			if (area > nMaxAreaSize)
			{
				nMaxAreaSize = area;
				nMaxBlobidx = i;
			}
		}

		drawContours(inv_maxBlob, contours, nMaxBlobidx, cv::Scalar(255), cv::FILLED, 0, hierarchy);

		OrgSizeMat.setTo(0);

		//¿øº»¿µ»ó »çÀÌÁî·Î ´Ù½Ã ¸¸µë
		for (int i = 0; i < OrgSizeMat.rows; i++)
		{
			for (int j = 0; j < OrgSizeMat.cols; j++)
			{
				int dx = j + 1;
				int dy = i + 1;
				if (inv_maxBlob.data[dy * inv_maxBlob.cols + dx] > 200)
					OrgSizeMat.data[i * OrgSizeMat.cols + j] = 255;
			}
		}

		std::vector<cv::Point> pt;

		//MinRect Area
		for (int i = 0; i < OrgSizeMat.rows; i++)
		{
			for (int j = 0; j < OrgSizeMat.cols; j++)
			{
				cv::Point tmpPt;
				if (OrgSizeMat.data[i * OrgSizeMat.cols + j] > 200)
				{
					tmpPt.x = j;
					tmpPt.y = i;
					pt.push_back(tmpPt);
				}
			}
		}

		int nShortLength = OrgSizeMat.cols;
		if (OrgSizeMat.cols > OrgSizeMat.rows)
			nShortLength = OrgSizeMat.rows;
		if (pt.size() < nShortLength)
			return ret;

		cv::RotatedRect rt = cv::minAreaRect(pt);

		int dAngle_except = (abs)(rt.angle);
		dAngle_except %= 90;
		if (dAngle_except > 5)
			return ret;

		cv::Point2f pts[4];
		rt.points(pts);

		for (int i = 0; i < 4; i++)
		{
			cv::line(rectimg, pts[i % 4], pts[(i + 1) % 4], 255);
		}


		cv::Rect rect;

		int MinX = 10000;
		int MinY = 10000;
		int MaxX = 0;
		int MaxY = 0;

		for (int i = 0; i < 4; i++)
		{
			if (pts[i].x + 0.3 < MinX)
				MinX = pts[i].x + 0.3;
			if (pts[i].y + 0.3 < MinY)
				MinY = pts[i].y + 0.3;

			if (MaxX < pts[i].x + 0.3)
				MaxX = pts[i].x + 0.3;
			if (MaxY < pts[i].y + 0.3)
				MaxY = pts[i].y + 0.3;
		}

		int nWidth = 0;
		int nHeight = 0;

		if (MinX < 0)
			MinX = 0;
		if (MinY < 0)
			MinY = 0;

		nWidth = MaxX - MinX;
		nHeight = MaxY - MinY;

		if (nWidth + MinX > src.cols)
		{
			int gapX = src.cols - (MinX + nWidth);
			nWidth -= gapX;
		}
		if (nHeight + MinY > src.rows)
		{
			int gapY = src.rows - (MinY + nHeight);
			nHeight -= gapY;
		}

		rect.x = MinX;
		rect.y = MinY;
		rect.width = nWidth + 1;
		rect.height = nHeight + 1;

		cv::Mat whiteRect(rect.height, rect.width, CV_8UC1, cv::Scalar(255));
		cv::Mat BackgroundMat(src.size(), src.type(), cv::Scalar(0));
		cv::Mat RoiMat = BackgroundMat(rect);
		whiteRect.copyTo(RoiMat);

		cv::Mat WhiteBoard(BackgroundMat.size(), BackgroundMat.type(), cv::Scalar(255));
		dst = WhiteBoard - BackgroundMat;


		for (int i = 0; i < 4; i++)
		{
			cv::Point tmPtr;
			tmPtr.x = pts[i].x + 0.3;
			tmPtr.y = pts[i].y + 0.3;
			vPoint.emplace_back(tmPtr);
		}
		ret = 1;

		return ret;
	}
	else
	{
		if (nImgHeight < nImgWidth)
		{
			vPoint.emplace_back(cv::Point(src.cols / 2, 0));
			vPoint.emplace_back(cv::Point(src.cols / 2, src.rows - 1));
		}
		else if (nImgHeight > nImgWidth)
		{
			vPoint.emplace_back(cv::Point(0, src.rows / 2));
			vPoint.emplace_back(cv::Point(src.cols - 1, src.rows / 2));
		}

		ret = 2;

		return ret;
	}

}
int CPInsp_AlgoBlob::SearchBlobShiftLine(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch, UCHAR *rectByimgSrc, std::vector<cv::Point2f>& vPoint, int nShiiftCase)
{

	int nRetDir = 0;

	cv::Mat src(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));

	cv::Mat test(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
	std::vector<cv::Point> pt;

	std::vector<cv::Point> LinePoint;

	if (vPoint.size() <= 0)
	{
		return nRetDir;
	}

	for (int i = 0; i < 4; i++)
	{
		cv::Point tmpPtr;
		if (vPoint[i].x != 0 && vPoint[i].y != 0 && vPoint[i].x != src.cols - 1 && vPoint[i].y != src.rows - 1)
		{
			int x = vPoint[i].x;
			int y = vPoint[i].y;
			src.data[y * src.cols + x] = 255;
			tmpPtr.x = x;
			tmpPtr.y = y;
			pt.emplace_back(tmpPtr);
		}
	}

	cv::Mat LineImg(nImgHeight, nImgWidth, CV_8UC1, pByImgSrc);
	cv::Mat LineResult(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));

	if (pt.size() < 3)
	{
		int nLeft_Ratio = 0;
		int nTop_Ratio = 0;
		int nRight_Ratio = 0;
		int nBottom_Ratio = 0;

		cv::Point tmpPoint;

		for (int i = 0; i < LineImg.rows; i++)
		{
			int lx = 0;
			int rx = LineImg.cols - 1;
			if (LineImg.data[i * LineImg.cols + lx] == 255)
				nLeft_Ratio++;
			if (LineImg.data[i * LineImg.cols + rx] == 255)
				nRight_Ratio++;
		}

		for (int i = 0; i < LineImg.cols; i++)
		{
			int ty = 0;
			int by = src.rows - 1;
			if (LineImg.data[ty * LineImg.cols + i] == 255)
				nTop_Ratio++;
			if (LineImg.data[by * LineImg.cols + i] == 255)
				nBottom_Ratio++;
		}

		// À§¿¡¼­ ±¸ÇÑ ½ÃÀÛ Æ÷ÀÎÆ®¿Í ³¡³ª´Â Æ÷ÀÎÆ®, Index(1~4) ¹æÇâ ±¸ÇÔ
		if (pt.size() == 1)		//¾ç¹æÇâ shift
		{
			if (nShiiftCase == 2)	// ¾ç¹æÇâ µðÆå, ¾ç¹æÇâÀ¸·Î ÆÇ´Ü
			{
				LinePoint.emplace_back(pt[0]);	// ½ÃÀÛÆ÷ÀÎÆ®

				if (nLeft_Ratio == LineImg.rows && nTop_Ratio == LineImg.cols)
				{
					tmpPoint.x = 0;
					tmpPoint.y = 0;
					LinePoint.emplace_back(tmpPoint);
					nRetDir = 3;
				}
				if (nLeft_Ratio == LineImg.rows && nBottom_Ratio == LineImg.cols)
				{
					tmpPoint.x = 0;
					tmpPoint.y = LineImg.rows - 1;
					LinePoint.emplace_back(tmpPoint);
					nRetDir = 4;
				}
				if (nRight_Ratio == LineImg.rows && nTop_Ratio == LineImg.cols)
				{
					tmpPoint.x = LineImg.cols - 1;
					tmpPoint.y = 0;
					LinePoint.emplace_back(tmpPoint);
					nRetDir = 1;
				}
				if (nRight_Ratio == LineImg.rows && nBottom_Ratio == LineImg.cols)
				{
					tmpPoint.x = LineImg.cols - 1;
					tmpPoint.y = LineImg.rows - 1;
					LinePoint.emplace_back(tmpPoint);
					nRetDir = 2;
				}
			}
			else if (nShiiftCase == 1)	//¾ç¹æÇâ Shift, ´Ü¹æÇâÀ¸·Î ÆÇ´Ü
			{
				if (nImgHeight > nImgWidth)
				{
					tmpPoint.x = pt[0].x;
					tmpPoint.y = LineImg.rows / 2;
					LinePoint.emplace_back(tmpPoint);

					if (nLeft_Ratio == LineImg.rows)
					{
						tmpPoint.x = 0;
						tmpPoint.y = LineImg.rows / 2;
						LinePoint.emplace_back(tmpPoint);
						nRetDir = 8;
					}
					if (nRight_Ratio == LineImg.rows)
					{
						tmpPoint.x = LineImg.cols - 1;
						tmpPoint.y = LineImg.rows / 2;
						LinePoint.emplace_back(tmpPoint);
						nRetDir = 6;
					}


				}
				else if (nImgHeight < nImgWidth)
				{
					tmpPoint.x = LineImg.cols / 2;
					tmpPoint.y = pt[0].y;
					LinePoint.emplace_back(tmpPoint);

					if (nTop_Ratio == LineImg.cols)
					{
						tmpPoint.x = LineImg.cols / 2;
						tmpPoint.y = 0;
						LinePoint.emplace_back(tmpPoint);
						nRetDir = 5;
					}
					if (nBottom_Ratio == LineImg.cols)
					{
						tmpPoint.x = LineImg.cols / 2;
						tmpPoint.y = LineImg.rows - 1;
						LinePoint.emplace_back(tmpPoint);
						nRetDir = 7;
					}
				}
			}
		}
		else if (pt.size() == 2)					//´Ü¹æÇâ shift
		{
			int gapX = (abs)(pt[0].x - pt[1].x) / 2;
			int gapY = (abs)(pt[0].y - pt[1].y) / 2;
			cv::Point tmpPtr;

			int dx = pt[0].x;
			int dy = pt[0].y;
			if (pt[0].x > pt[1].x)
				dx = pt[1].x;
			if (pt[0].y > pt[1].y)
				dy = pt[1].y;

			dx += gapX;
			dy += gapY;
			tmpPtr.x = dx;
			tmpPtr.y = dy;
			LinePoint.emplace_back(tmpPtr);

			if (nImgHeight > nImgWidth)
			{

				if (nLeft_Ratio == LineImg.rows)
				{
					tmpPtr.x = 0;
					tmpPtr.y = dy;
					LinePoint.emplace_back(tmpPtr);
					nRetDir = 8;
				}
				if (nRight_Ratio == LineImg.rows)
				{
					tmpPtr.x = LineImg.cols - 1;
					tmpPtr.y = dy;
					LinePoint.emplace_back(tmpPtr);
					nRetDir = 6;
				}


			}
			else if (nImgHeight < nImgWidth)
			{

				if (nTop_Ratio == LineImg.cols)
				{
					tmpPtr.x = dx;
					tmpPtr.y = 0;
					LinePoint.emplace_back(tmpPtr);
					nRetDir = 5;
				}
				if (nBottom_Ratio == LineImg.cols)
				{
					tmpPtr.x = dx;
					tmpPtr.y = LineImg.rows - 1;
					LinePoint.emplace_back(tmpPtr);
					nRetDir = 7;
				}
			}

		}

		if (LinePoint.size() > 0)
		{
			vPoint.clear();
			for (int i = 0; i < LinePoint.size(); i++)
			{
				vPoint.emplace_back(LinePoint[i]);
				LineResult.data[LinePoint[i].y * LineResult.cols + LinePoint[i].x] = 255;
			}
		}
	}


	if (pt.size() > 0)
		pt.clear();
	if (LinePoint.size() > 0)
		LinePoint.clear();

	return nRetDir;
}
void CPInsp_AlgoBlob::BlobTeachAC(int nImgWidth, int nImgHeight, RstAlgoBlob * sRstAlgo, AlgoBlob * pAlgoBlob, InspRoiImgBuf & sInspImageData, UCHAR * ucArrBlobDst, WndAlgoImg & sWndClipAlgo, UCHAR* ucArrDstImgAC, int nOrgWidth, int nOrgHeight, int nROIWidth, int nROILength, int nClipStartX, double dClipWidth, int nClipStartY, double dClipHeight, UCHAR * puImgDst)
{
	if ((pAlgoBlob->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) != eDefaultAC_Data_Use)
		return;
	UCHAR * pucACDst = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucACDst, nImgWidth * nImgHeight * 3);
	memset(pucACDst, 0, nImgWidth * nImgHeight * 3 * sizeof(UCHAR));
	POINTF poCenter;
	poCenter.x = nImgWidth / 2;
	poCenter.y = nImgHeight / 2;
	sRstAlgo->m_sInspAC.m_nInspAC = InspAC(pAlgoBlob->m_sAngleColorBase, sInspImageData, ucArrBlobDst, sWndClipAlgo, pAlgoBlob->m_byDir, pucACDst, sRstAlgo->m_sInspAC.m_fRstInspAC, &sRstAlgo->m_sInspAC.m_rcArrRect[m_eInspAC_R_Rect_CJ], true, poCenter);
	bool bBlobImg = (ucArrBlobDst && ((pAlgoBlob->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Essential) == eDefaultAC_Data_Essential));
	if (ucArrDstImgAC || bBlobImg)
	{
		int nWndIDXAC = 0;
		int widthStep = nOrgWidth;
		//if ((widthStep % 4) != 0) widthStep += 4 - (widthStep % 4);
		widthStep = g_pMPTI->nCalcWidthStep(false, nOrgWidth);

		if (ucArrDstImgAC)
			memset(ucArrDstImgAC, 0, widthStep * nOrgHeight * 3 * sizeof(UCHAR));
		if (bBlobImg)
			memset(ucArrBlobDst, 0, nROIWidth * nROILength * sizeof(UCHAR));
		for (int y = 0; y < nOrgHeight; y++)
		{
			for (int x = 0; x < nOrgWidth; x++)
			{
				int nIndex = (y * nOrgWidth) + x;
				int nIndex1 = (y * widthStep) + x;
				int nIndex2 = ((y - nClipStartY) * nImgWidth) + (x - nClipStartX);
				if (x >= nClipStartX && x < nClipStartX + dClipWidth &&
					y >= nClipStartY && y < nClipStartY + dClipHeight &&
					x - nClipStartX < nImgWidth && y - nClipStartY < nImgHeight &&
					nIndex2 >= 0 && (nIndex2 * 3) + 2 < nImgWidth * nImgHeight * 3)
				{
					if (bBlobImg)
					{
						if (pucACDst[(nIndex2 * 3)] > 0 || pucACDst[(nIndex2 * 3) + 1] > 0 || pucACDst[(nIndex2 * 3) + 2] > 0)
							ucArrBlobDst[nIndex2] = 255;
					}
					if (ucArrDstImgAC)
					{
						ucArrDstImgAC[(nIndex1 * 3)] = pucACDst[nWndIDXAC++];
						ucArrDstImgAC[(nIndex1 * 3) + 1] = pucACDst[nWndIDXAC++];
						ucArrDstImgAC[(nIndex1 * 3) + 2] = pucACDst[nWndIDXAC++];
					}
				}
				else
				{
					if (ucArrDstImgAC)
					{
						ucArrDstImgAC[(nIndex1 * 3)] = puImgDst[nIndex];
						ucArrDstImgAC[(nIndex1 * 3) + 1] = puImgDst[nIndex];
						ucArrDstImgAC[(nIndex1 * 3) + 2] = puImgDst[nIndex];
					}
				}
			}
		}
		m_pProcMilAlgo->SaveWorkImg(pucACDst, nImgWidth, nImgHeight, _T("AngleColor_Wnd_Image.bmp"), 3);
		m_pProcMilAlgo->SaveWorkImg(ucArrBlobDst, nImgWidth, nImgHeight, _T("AngleColor_Wnd_BW.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ucArrDstImgAC, widthStep, nOrgHeight, _T("AngleColor_Part_Image.bmp"), 3);
	}
	if (bBlobImg)
	{
		int nFilter = (pAlgoBlob->m_bFilterIsUse) ? pAlgoBlob->m_nFilterStepNarrow : 0;
		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucArrBlobDst, ucArrBlobDst, nROIWidth, nROILength, 4, FALSE, pAlgoBlob->m_bFillHole, nFilter, pAlgoBlob->m_nTypeSelectBlob);
		memset(sRstAlgo->m_rcArrRect, 0, BLOB_RECT_CNTS * sizeof(RECT));
		sRstAlgo->m_nArrRectCnt = (nCntBlob > BLOB_RECT_CNTS) ? BLOB_RECT_CNTS : nCntBlob;
		if (sRstAlgo->m_nArrRectCnt > 0)
		{
			CRect *rcArrBlob = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetBlobResult_ALL(NULL, NULL, NULL, rcArrBlob);
			for (int a = 0; a < sRstAlgo->m_nArrRectCnt; a++)
			{
				sRstAlgo->m_rcArrRect[a].left = rcArrBlob[a].left;
				sRstAlgo->m_rcArrRect[a].right = rcArrBlob[a].right;
				sRstAlgo->m_rcArrRect[a].top = rcArrBlob[a].top;
				sRstAlgo->m_rcArrRect[a].bottom = rcArrBlob[a].bottom;
			}
			g_pMManager->pem_delete(rcArrBlob, true);
		}
	}
	Delete_1DArray(&pucACDst);
}
void CPInsp_AlgoBlob::WndArrayBlobImg(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, UCHAR *puImgDst, RstWndArr * sRstAlgo)
{
	if (!m_pProcMilAlgo || sInspAlgo.m_eAlgoType != eAlgoBlob || puImgDst == NULL || sRstAlgo == NULL)
		return;
	AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pAlgoBlob || (!pAlgoBlob->m_bInsp2D && !pAlgoBlob->m_bInsp3D && !pAlgoBlob->m_sAlgoColorBase.m_bUseColor))
		return;
	double dResolX = (g_pMPTI->m_bSideOriginalSize == false) ? m_resolX : g_pMPTI->m_dBtmSideResX;
	double dResolY = (g_pMPTI->m_bSideOriginalSize == false) ? m_resolY : g_pMPTI->m_dBtmSideResY;
	UCHAR *pSrc2D = sWndAlgoImg.m_ucArr2D;
	float *pSrc3D = sWndAlgoImg.m_fArr3D;
	int nOrgWidth = sWndAlgoImg.m_nWidth;
	int nOrgHeight = sWndAlgoImg.m_nHeight;
	if ((pAlgoBlob->m_bInsp2D && pSrc2D == NULL) || (pAlgoBlob->m_bInsp3D && pSrc3D == NULL) || nOrgWidth <= 0 || nOrgHeight <= 0)
		return;
	if (sRstAlgo->m_byG == 0)
		memset(sRstAlgo, 0, (sizeof(RstWndArr)));
	else
	{
		memset(sRstAlgo->m_fArrRstA, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(sRstAlgo->m_fArrRstH, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(sRstAlgo->m_fArrRstHH, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(sRstAlgo->m_fArrRstW, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(sRstAlgo->m_fArrRstL, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(sRstAlgo->m_fArrRstG, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(sRstAlgo->m_fArrRstV, 0.0, WND_ARRAY_CNTS * sizeof(float));
	}
	memset(puImgDst, 0, sizeof(UCHAR) * nOrgWidth * nOrgHeight);
	UCHAR *ucColorImgDst = NULL;
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
	if (pColorTeach != NULL && pAlgoBlob->m_sAlgoColorBase.m_bUseColor == TRUE)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
		pColorTeach->GetColorBaseBin(&pAlgoBlob->m_sAlgoColorBase, sInspImageData, ucColorImgDst, true, 0, 0, -1, sWndAlgoImg.m_nLight_index);
		m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, nOrgWidth, nOrgHeight, _T("OrgColor_Image.bmp"));
	}
	bool bGerber = ((sRstAlgo->m_byG & 0x01) == 0x01);
	bool bGerberBin = (bGerber && ((sRstAlgo->m_byG & 0x02) == 0x02));
	TotalInspExceptArea stTieArea;
	stTieArea.m_nUsedInspPolygon = 0;
	stTieArea.m_nUsedMaskingValue = 0;
	stTieArea.m_nUsedWndPolygon = 0;
	double dCX = 0;
	double dCY = 0;
	double dArea = 0;
	CRect rcBlob(0, 0, 0, 0);
	int nMinBlobArea = 4;
	int nMinW = pAlgoBlob->m_dTeachWidth / m_resolX;
	int nMinH = pAlgoBlob->m_dTeachLength / m_resolY;
	int nMarW = pAlgoBlob->m_dTeachWidthRateMin / m_resolX;
	int nMarH = pAlgoBlob->m_dTeachWidthRateMax / m_resolY;
	int nCntBlob = m_pCPInsp_Algo->BlobImageStruct(*pAlgoBlob, pSrc2D, pSrc3D, ucColorImgDst, nOrgWidth, nOrgHeight, nMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, puImgDst, stTieArea, pAlgoBlob->m_bFillHole);
	double *dArrA = NULL;
	CRect *cArrR = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrA, nCntBlob);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &cArrR, nCntBlob);
	m_pProcMilAlgo->GetBlobResult_ALL(dArrA, NULL, NULL, cArrR);
	int nCount = (WND_ARRAY_CNTS > nCntBlob) ? nCntBlob : WND_ARRAY_CNTS;
	if (nCount > 0 && bGerber == false)
	{
		sRstAlgo->m_nArrRectCnt = 0;
		m_pProcMilAlgo->GetBlobResult_ALL(dArrA, NULL, NULL, cArrR);
		for (int a = 0; a < nCount; a++)
		{
			int nW = cArrR[a].right - cArrR[a].left;
			int nH = cArrR[a].bottom - cArrR[a].top;
			int nHCnt = 0;
			float fSum = 0.0f;
			int nSum = 0;
			float fMaxH = 0.0f;
			float fV = 0.0f;
			for (int y = cArrR[a].top; y <= cArrR[a].bottom; y++)
			{
				for (int x = cArrR[a].left; x <= cArrR[a].right; x++)
				{
					int nIndex = (y * nOrgWidth) + x;
					if (nIndex < 0 || nIndex > nOrgWidth * nOrgHeight)
						continue;
					if ((nMinW > 0 && nMinW >= nW) || (nMinH > 0 && nMinH >= nH))
						puImgDst[nIndex] = 0;
					else if (puImgDst[nIndex] == 255)
					{
						fSum += pSrc3D[nIndex];
						nSum += pSrc2D[nIndex];
						nHCnt++;
						if (pSrc3D[nIndex] > fMaxH)
							fMaxH = pSrc3D[nIndex];
						fV += (m_resolX * m_resolY * pSrc3D[nIndex]);
					}
				}
			}
			if ((nMinW > 0 && nMinW >= nW) || (nMinH > 0 && nMinH >= nH))
				continue;
			sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].left = cArrR[a].left - nMarW;
			sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].right = cArrR[a].right + nMarW;
			sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].top = cArrR[a].top - nMarH;
			sRstAlgo->m_rcArrRect[sRstAlgo->m_nArrRectCnt].bottom = cArrR[a].bottom + nMarH;
			sRstAlgo->m_fArrRstA[sRstAlgo->m_nArrRectCnt] = dArrA[a] * m_resolX * m_resolY;
			sRstAlgo->m_fArrRstH[sRstAlgo->m_nArrRectCnt] = (nHCnt > 0) ? fSum / nHCnt : 0.0f;
			sRstAlgo->m_fArrRstHH[sRstAlgo->m_nArrRectCnt] = fMaxH;
			sRstAlgo->m_fArrRstW[sRstAlgo->m_nArrRectCnt] = (cArrR[a].right - cArrR[a].left) * m_resolX;
			sRstAlgo->m_fArrRstL[sRstAlgo->m_nArrRectCnt] = (cArrR[a].bottom - cArrR[a].top) * m_resolY;
			sRstAlgo->m_fArrRstG[sRstAlgo->m_nArrRectCnt] = (nHCnt > 0) ? nSum / nHCnt : 0;
			sRstAlgo->m_fArrRstV[sRstAlgo->m_nArrRectCnt] = fV;
			sRstAlgo->m_nArrRectCnt++;
		}
	}
	else if (bGerber == true)
	{
		for (int a = 0; a < sRstAlgo->m_nArrRectCnt; a++)
		{
			int nW = sRstAlgo->m_rcArrRect[a].right - sRstAlgo->m_rcArrRect[a].left;
			int nH = sRstAlgo->m_rcArrRect[a].bottom - sRstAlgo->m_rcArrRect[a].top;
			int nHCnt = 0;
			float fSum = 0.0f;
			int nSum = 0;
			float fMaxH = 0.0f;
			float fV = 0.0f;
			for (int y = sRstAlgo->m_rcArrRect[a].top; y <= sRstAlgo->m_rcArrRect[a].bottom; y++)
			{
				for (int x = sRstAlgo->m_rcArrRect[a].left; x <= sRstAlgo->m_rcArrRect[a].right; x++)
				{
					int nIndex = (y * nOrgWidth) + x;
					if (nIndex < 0 || nIndex > nOrgWidth * nOrgHeight)
						continue;
					if ((bGerberBin && puImgDst[nIndex] == 255) || (bGerberBin == false))
					{
						fSum += pSrc3D[nIndex];
						nSum += pSrc2D[nIndex];
						nHCnt++;
						if (pSrc3D[nIndex] > fMaxH)
							fMaxH = pSrc3D[nIndex];
						fV += (m_resolX * m_resolY * pSrc3D[nIndex]);
					}
				}
			}
			if (bGerberBin && nHCnt <= nMinW * nMinH * 0.8)
			{
				sRstAlgo->m_rcArrRect[a].left = 0;
				sRstAlgo->m_rcArrRect[a].right = 0;
				sRstAlgo->m_rcArrRect[a].top = 0;
				sRstAlgo->m_rcArrRect[a].bottom = 0;
			}
			else
			{
				sRstAlgo->m_rcArrRect[a].left -= nMarW;
				sRstAlgo->m_rcArrRect[a].right += nMarW;
				sRstAlgo->m_rcArrRect[a].top -= nMarH;
				sRstAlgo->m_rcArrRect[a].bottom += nMarH;
			}
			sRstAlgo->m_fArrRstA[a] = nHCnt * m_resolX * m_resolY;
			sRstAlgo->m_fArrRstH[a] = (nHCnt > 0) ? fSum / nHCnt : 0.0f;
			sRstAlgo->m_fArrRstHH[a] = fMaxH;
			sRstAlgo->m_fArrRstW[a] = nW * m_resolX;
			sRstAlgo->m_fArrRstL[a] = nH * m_resolY;
			sRstAlgo->m_fArrRstG[a] = (nHCnt > 0) ? nSum / nHCnt : 0;
			sRstAlgo->m_fArrRstV[a] = fV;
		}
	}
	Delete_1DArray(&ucColorImgDst);
	Delete_1DArray(&dArrA);
	Delete_1DArray(&cArrR);
}
int CPInsp_AlgoBlob::SetAIAlgoImageParam(CString sBoard, CString sPartCode, CString sRefId, CString sModuleID)
{
	if (sBoard != _T(""))
		m_sBoard.Format(_T("%s"), sBoard);
	if (sPartCode != _T(""))
		m_sPartCode.Format(_T("%s"), sPartCode);
	m_sRefID = sRefId;
	m_sModuleID = sModuleID;

	return ePART_SUCCESS;
}
void CPInsp_AlgoBlob::BlobImageSavePath(CString sTimePath)
{
	BlobImgPath = sTimePath;
}
void CPInsp_AlgoBlob::AIImageListSave(int nCurJob)
{
	for (auto vect = vBlobNGImg.begin(); vect != vBlobNGImg.end(); vect++)
	{
		if (!vect->mGrayImg.empty())
			cv::imwrite(std::string(CT2A(vect->sGray)), (vect->mGrayImg));
		if (!vect->mBinImg.empty())
			cv::imwrite(std::string(CT2A(vect->sBinary)), (vect->mBinImg));
		if (!vect->mMask.empty())
			cv::imwrite(std::string(CT2A(vect->sMask)), (vect->mMask));
	}
	vBlobNGImg.clear();
}
void CPInsp_AlgoBlob::AiResultClear(int nCurJob)
{
	vBlobNGImg.clear();
}
bool CPInsp_AlgoBlob::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}

std::vector<cv::Point> CPInsp_AlgoBlob::thickness(cv::Mat Image, int nWndDir, cv::Point Center, std::vector<std::vector<cv::Point>> contours, cv::Rect rcBlob, long lLebel)
{
	////좌측
	//nWndDir += 0x04;
	////우측
	//nWndDir += 0x08;
	////상측
	//nWndDir += 0x01;
	////하측
	//nWndDir += 0x02;
	cv::Point poStd(0,0);
	int nCalc = 1;
	if ((nWndDir & 0x04) == 0x04)
	{
		poStd.x = rcBlob.x;
	}
	else if ((nWndDir & 0x08) == 0x08)
	{
		poStd.x = rcBlob.x + rcBlob.width;
		nCalc *= -1;
	}
	if ((nWndDir & 0x01) == 0x01)
	{
		poStd.y = rcBlob.y;
	}
	else if ((nWndDir & 0x02) == 0x02)
	{
		poStd.y = rcBlob.y + rcBlob.height;
		nCalc *= -1;
	}

	cv::Mat img = Image.clone();
	cv::Mat distTr;
	cv::distanceTransform(Image, distTr, cv::DIST_C, 3);
	double dMin, dMax;
	cv::Point poMin, poMax;
	cv::minMaxLoc(distTr, &dMin, &dMax, &poMin, &poMax);
	distTr.convertTo(distTr, CV_8UC1);
	int nMaxTh = dMax * 0.8;
	cv::Mat bin;
	cv::threshold(distTr, bin, nMaxTh, 255, cv::THRESH_BINARY);
	cv::Mat thin = ImageThining(Image.clone());

	if (contours.size() == 0)
	{
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	}

	std::vector<std::vector<cv::Point>> blobPoints;
	blobPoints.resize(contours.size());
	double dMaxArea = 0;
	for (int sz = 0; sz < contours.size(); sz++)
	{
		//cv::Rect bbox = cv::boundingRect(contours[sz]);
		//double area = cv::contourArea(contours[sz]);
		//if (dMaxArea < area)
		//{
		//	dMaxArea = area;
		//	Center.x = bbox.x + bbox.width / 2;
		//	Center.y = bbox.y + bbox.height / 2;
		//}
		for (int i = 0; i < contours[sz].size(); i++)
		{
			//UCHAR* distPtr = distTr.ptr(contours[sz][i].y);
			//if (distPtr[contours[sz][i].x] == 1)
			//{
			//	if (blobPoints[sz].size() > 0 && blobPoints[sz][blobPoints[sz].size() - 1].x == contours[sz][i].x)
			//	{
			//		if (blobPoints[sz][blobPoints[sz].size() - 1].y < contours[sz][i].y)
			//		{
			//			for (int y = blobPoints[sz][blobPoints[sz].size() - 1].y; y < contours[sz][i].y; y++)
			//			{
			//				blobPoints[sz].push_back(cv::Point(contours[sz][i].x, y));
			//			}
			//		}
			//		else if (blobPoints[sz][blobPoints[sz].size() - 1].y > contours[sz][i].y)
			//		{
			//			for (int y = blobPoints[sz][blobPoints[sz].size() - 1].y; y > contours[sz][i].y; y--)
			//			{
			//				blobPoints[sz].push_back(cv::Point(contours[sz][i].x, y));
			//			}
			//		}
			//		blobPoints[sz].push_back(contours[sz][i]);
			//	}
			//	else if (blobPoints[sz].size() > 0 && blobPoints[sz][blobPoints[sz].size() - 1].y == contours[sz][i].y)
			//	{
			//		if (blobPoints[sz][blobPoints[sz].size() - 1].x < contours[sz][i].x)
			//		{
			//			for (int x = blobPoints[sz][blobPoints[sz].size() - 1].x; x < contours[sz][i].x; x++)
			//			{
			//				blobPoints[sz].push_back(cv::Point(x, contours[sz][i].y));
			//			}
			//		}
			//		else if (blobPoints[sz][blobPoints[sz].size() - 1].x > contours[sz][i].x)
			//		{
			//			for (int x = blobPoints[sz][blobPoints[sz].size() - 1].x; x > contours[sz][i].x; x--)
			//			{
			//				blobPoints[sz].push_back(cv::Point(x, contours[sz][i].y));
			//			}
			//		}
			//		blobPoints[sz].push_back(contours[sz][i]);
			//	}
			//	else
			//	{
			//		blobPoints[sz].push_back(contours[sz][i]);
			//	}
			//	blobPoints[sz].push_back(contours[sz][i]);
			//}
			blobPoints[sz].push_back(contours[sz][i]);
		}
	}
	std::vector<std::pair<cv::Point, double>> vPoAPair;
	std::vector<std::pair<cv::Point, cv::Point>> vPoPiar;
	for (int sz = 0; sz < blobPoints.size(); sz++)
	{
		for (int i = 0; i < blobPoints[sz].size(); i++)
		{
			std::vector< cv::Point> pint;
			int nEndCandj = i + 20 < blobPoints[sz].size() ? i + 20 : blobPoints[sz].size();
			for (int j = i - 20; j < nEndCandj; j++)
			{
				if (j < 0)
					continue;

				pint.push_back(blobPoints[sz][j]);
			}
			cv::Point st = pint[0];
			cv::Point ed = pint[pint.size() - 1];

			int dy = ed.y - st.y;
			int dx = ed.x - st.x;
			st = ed = blobPoints[sz][i];
			if (std::abs(dx) < 3)// | 세로선
			{
				//검증
				double dCenDiffX = Center.x - blobPoints[sz][i].x;
				double dCenDiffY = Center.y - blobPoints[sz][i].y;
				if (std::abs(dCenDiffY) < 1 || std::abs(dCenDiffY / dCenDiffX) < 1)
				{
					for (int x = blobPoints[sz][i].x; x > 0; x--)
					{
						int y = blobPoints[sz][i].y;
						if (Image.ptr(y)[x] != lLebel)
							break;
						st.x = x;
						st.y = y;
					}
					for (int x = blobPoints[sz][i].x; x < Image.cols; x++)
					{
						int y = blobPoints[sz][i].y;
						if (Image.ptr(y)[x] != lLebel)
							break;
						ed.x = x;
						ed.y = y;
					}

					//vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
				}
				vPoAPair.push_back(std::pair<cv::Point, double>(st, 9999));
			}
			else if (std::abs(dy) < 3)// - 가로선
			{
				//검증
				double dCenDiffX = Center.x - blobPoints[sz][i].x;
				double dCenDiffY = Center.y - blobPoints[sz][i].y;
				if (std::abs(dCenDiffX) < 1 || std::abs(dCenDiffY / dCenDiffX) > 1)
				{
					for (int y = blobPoints[sz][i].y; y > 0; y--)
					{
						int x = blobPoints[sz][i].x;
						if (Image.ptr(y)[x] != lLebel)
							break;
						st.x = x;
						st.y = y;
					}
					for (int y = blobPoints[sz][i].y; y < Image.rows; y++)
					{
						int x = blobPoints[sz][i].x;
						if (Image.ptr(y)[x] != lLebel)
							break;
						ed.x = x;
						ed.y = y;
					}

					//vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
				}
				vPoAPair.push_back(std::pair<cv::Point, double>(st, -9999));
			}
			else // X 사선
			{
				float A = (double)dy / dx;
				float A_ = -1 / A;
				float B = blobPoints[sz][i].y - (A_ * blobPoints[sz][i].x);
				m_pProcMilAlgo->PCA_1D(pint, &A, &B);
				A_ = -1 / A;
				B = blobPoints[sz][i].y - (A_ * blobPoints[sz][i].x);
				cv::Point org = blobPoints[sz][i];
				//검증
				double dCenDiffX = Center.x - blobPoints[sz][i].x;
				double dCenDiffY = Center.y - blobPoints[sz][i].y;
				double dCenA = dCenDiffY / dCenDiffX;
				double MnCondi(0.0f), MxCondi(0.0f);
				////좌측
				if ((nWndDir & 0x04) == 0x04)
				{
					////상측
					if ((nWndDir & 0x01) == 0x01)
					{
						MnCondi = ((double)1 / 2);
						MxCondi = 2;
					}
					////하측
					else if ((nWndDir & 0x02) == 0x02)
					{
						MnCondi = -2;
						MxCondi = -((double)1 / 2);
					}
				}
				////우측
				else if ((nWndDir & 0x08) == 0x08)
				{
					////상측
					if ((nWndDir & 0x01) == 0x01)
					{
						MnCondi = -2;
						MxCondi = -((double)1 / 2);
					}
					////하측
					else if ((nWndDir & 0x02) == 0x02)
					{
						MnCondi = ((double)1 / 2);
						MxCondi = 2;
					}
				}
				if (std::abs(dCenDiffX) > 0 && std::abs(dCenDiffY) > 0 && dCenA >= MnCondi && dCenA <= MxCondi)
				{
					for (int x = blobPoints[sz][i].x; x > 0; x--)
					{
						int y = A_ * x + B;
						if (y < 0 || y >= Image.rows || Image.ptr(y)[x] != lLebel)
							break;
						st.x = x;
						st.y = y;
					}
					for (int x = blobPoints[sz][i].x; x < Image.cols; x++)
					{
						int y = A_ * x + B;
						if (y < 0 || y >= Image.rows || Image.ptr(y)[x] != lLebel)
							break;
						ed.x = x;
						ed.y = y;
					}
					//vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
				}
				vPoAPair.push_back(std::pair<cv::Point, double>(blobPoints[sz][i], A));
			}
		}
	}
	std::vector<std::pair<cv::Point, double>> vPoAPair_exList;
	for (int sz = 0; sz < vPoAPair.size(); sz++)
	{
		cv::Point st = vPoAPair[sz].first;
		cv::Point ed = vPoAPair[sz].first;
		double A = vPoAPair[sz].second;
		if (A == 9999)
		{
			//검증
			double dCenDiffX = Center.x - vPoAPair[sz].first.x;
			double dCenDiffY = Center.y - vPoAPair[sz].first.y;
			double dCenA = dCenDiffY / dCenDiffX;
			double MnCondi(0.0f), MxCondi(0.0f);
			double t1 = dCenA, Rad = 0;
			double Angle = atan(dCenA) - Rad;
			int nUpDown = vPoAPair[sz].first.y - (vPoAPair[sz].first.y - (poStd.x - (vPoAPair[sz].first.x*nCalc)));
			if (((nWndDir & 0x01) == 0x01 && nUpDown < 0) || ((nWndDir & 0x02) == 0x02 && nUpDown > 0))
			{
				for (int x = vPoAPair[sz].first.x; x > 0; x--)
				{
					int y = vPoAPair[sz].first.y;
					if (Image.ptr(y)[x] != lLebel)
						break;
					st.x = x;
					st.y = y;
				}
				for (int x = vPoAPair[sz].first.x; x < Image.cols; x++)
				{
					int y = vPoAPair[sz].first.y;
					if (Image.ptr(y)[x] != lLebel)
						break;
					ed.x = x;
					ed.y = y;
				}
				vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
			}
		}
		else if (A == -9999)
		{
			//검증
			double dCenDiffX = Center.x - vPoAPair[sz].first.x;
			double dCenDiffY = Center.y - vPoAPair[sz].first.y;
			double dCenA = dCenDiffY / dCenDiffX;
			double MnCondi(0.0f), MxCondi(0.0f);
			double t1 = dCenA, Rad = 1.5708;
			double Angle = atan(dCenA) - Rad;
			int nUpDown = vPoAPair[sz].first.y - (vPoAPair[sz].first.y - (poStd.x - (vPoAPair[sz].first.x*nCalc)));
			if (((nWndDir & 0x01) == 0x01 && nUpDown > 0) || ((nWndDir & 0x02) == 0x02 && nUpDown < 0))
			{
				for (int y = vPoAPair[sz].first.y; y > 0; y--)
				{
					int x = vPoAPair[sz].first.x;
					if (Image.ptr(y)[x] != lLebel)
						break;
					st.x = x;
					st.y = y;
				}
				for (int y = vPoAPair[sz].first.y; y < Image.rows; y++)
				{
					int x = vPoAPair[sz].first.x;
					if (Image.ptr(y)[x] != lLebel)
						break;
					ed.x = x;
					ed.y = y;
				}

				vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
			}
		}
		else
		{
			double AAvg = A;
			AAvg = (AAvg * 4);
			int addCnt = 4;
			//if (sz - 2 > 0 && vPoAPair[sz - 2].second != std::abs(9999))
			//{
			//	AAvg += vPoAPair[sz - 2].second;
			//	addCnt++;
			//}
			//if (sz - 1 > 0 && vPoAPair[sz - 1].second != std::abs(9999))
			//{
			//	AAvg += (vPoAPair[sz - 1].second * 2);
			//	addCnt += 2;
			//}
			//if (sz + 1 < vPoAPair.size() && vPoAPair[sz + 1].second != std::abs(9999))
			//{
			//	AAvg += (vPoAPair[sz + 1].second * 2);
			//	addCnt += 2;
			//}
			//if (sz + 2 < vPoAPair.size() && vPoAPair[sz + 2].second != std::abs(9999))
			//{
			//	AAvg += vPoAPair[sz + 2].second;
			//	addCnt++;
			//}
			AAvg /= addCnt;
			double A_ = -1 / AAvg;
			double B = st.y - (A_ * st.x);
			cv::Point org = st;
			//검증
			double dCenDiffX = Center.x - vPoAPair[sz].first.x;
			double dCenDiffY = Center.y - vPoAPair[sz].first.y;
			double dCenA = dCenDiffY / dCenDiffX;
			double MnCondi(0.0f), MxCondi(0.0f);
			double t1 = dCenA, t2 = A_;
			double Angle = atan((dCenA - A_) / (1 + dCenA * A_));
			double AngleT = atan((t1 - t2) / (1 + t1 * t2));
			//dCenA*= AAvg;
			if (std::abs(dCenDiffX) > 0 && std::abs(dCenDiffY) > 0 && (Angle >= ((double)-0.785398f) && Angle <= ((double)0.785398f)))
			{
				for (int x = vPoAPair[sz].first.x; x > 0; x--)
				{
					int y = A_ * x + B;
					if (y < 0 || y >= Image.rows || Image.ptr(y)[x] != lLebel)
						continue;
					st.x = x;
					st.y = y;
				}
				for (int x = vPoAPair[sz].first.x; x < Image.cols; x++)
				{
					int y = A_ * x + B;
					if (y < 0 || y >= Image.rows || Image.ptr(y)[x] != lLebel)
						continue;
					ed.x = x;
					ed.y = y;
				}
				vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
			}
			else
			{
				vPoAPair_exList.push_back(vPoAPair[sz]);
			}
		}
	}
	double dMaxDist = 0;
	double dMinDist = 0;
	cv::Point MxPoSt, MxPoEd;
	for (int sz = 0; sz < vPoPiar.size(); sz++)
	{
		cv::Point st = vPoPiar[sz].first;
		cv::Point ed = vPoPiar[sz].second;

		double dDist = std::sqrt(std::pow((double)ed.x - st.x, 2) + std::pow((double)ed.y - st.y, 2));
		if (dDist > dMaxDist)
		{
			dMaxDist = dDist;
			MxPoSt = st;
			MxPoEd = ed;
		}
	}
#if _DEBUG
	cv::Mat draw;
	cv::cvtColor(Image, draw, cv::COLOR_GRAY2BGR);
	cv::line(draw, MxPoSt, MxPoEd, cv::Scalar(0, 0, 255), 1);
#endif // _DEBUG

	std::vector<cv::Point> vPo;
	vPo.push_back(MxPoSt);
	vPo.push_back(MxPoEd);
	return vPo;
}

std::vector<cv::Point> CPInsp_AlgoBlob::thickness2(cv::Mat Image, int nWndDir, cv::Point Center, std::vector<std::vector<cv::Point>> contours, cv::Rect rcBlob, std::vector<cv::Point> vLinePo, long lLebel)
{
	if (vLinePo.size() < 6)
	{
		return thickness(Image, nWndDir, Center, contours, rcBlob, lLebel);
	}
	////좌측
	//nWndDir += 0x04;
	////우측
	//nWndDir += 0x08;
	////상측
	//nWndDir += 0x01;
	////하측
	//nWndDir += 0x02;
	cv::Point poStd(0, 0);
	int nCalc = 1;
	//1 2
	//   3
	//    4
	//     5
	//     6
	std::vector<cv::Point> vLinePoSort;//12,23,34,45,56
	if ((nWndDir & 0x04) == 0x04)//left
	{
		poStd.x = rcBlob.x;
		//vLinePoSort.push_back(vLinePo[1]);//0
		//vLinePoSort.push_back(vLinePo[0]);//1
		//vLinePoSort.push_back(vLinePo[2]);//2
		//vLinePoSort.push_back(vLinePo[1]);//3
		//vLinePoSort.push_back(vLinePo[3]);//4
		//vLinePoSort.push_back(vLinePo[2]);//5
		//vLinePoSort.push_back(vLinePo[4]);//6
		//vLinePoSort.push_back(vLinePo[3]);//7
	}
	else if ((nWndDir & 0x08) == 0x08)//right
	{
		poStd.x = rcBlob.x + rcBlob.width;
		nCalc *= -1;
		//vLinePoSort.push_back(vLinePo[0]);//0
		//vLinePoSort.push_back(vLinePo[1]);//1
		//vLinePoSort.push_back(vLinePo[1]);//2
		//vLinePoSort.push_back(vLinePo[2]);//3
		//vLinePoSort.push_back(vLinePo[2]);//4
		//vLinePoSort.push_back(vLinePo[3]);//5
		//vLinePoSort.push_back(vLinePo[3]);//6
		//vLinePoSort.push_back(vLinePo[4]);//7
	}
	if ((nWndDir & 0x01) == 0x01)//top
	{
		poStd.y = rcBlob.y;
		//vLinePoSort.push_back(vLinePo[4]);//8
		//vLinePoSort.push_back(vLinePo[5]);//9
	}
	else if ((nWndDir & 0x02) == 0x02)//bot
	{
		poStd.y = rcBlob.y + rcBlob.height;
		nCalc *= -1;
		//vLinePoSort.push_back(vLinePo[5]);//8
		//vLinePoSort.push_back(vLinePo[4]);//9
	}
	if (vLinePo[0].x > vLinePo[1].x)
	{
		vLinePoSort.push_back(vLinePo[1]);//0
		vLinePoSort.push_back(vLinePo[0]);//1
	}
	else
	{
		vLinePoSort.push_back(vLinePo[0]);//0
		vLinePoSort.push_back(vLinePo[1]);//1
	}
	vLinePoSort.push_back(vLinePo[1]);//2
	vLinePoSort.push_back(vLinePo[2]);//3
	vLinePoSort.push_back(vLinePo[2]);//4
	vLinePoSort.push_back(vLinePo[3]);//5
	vLinePoSort.push_back(vLinePo[3]);//6
	vLinePoSort.push_back(vLinePo[4]);//7
	if (vLinePo[4].y > vLinePo[5].y)
	{
		vLinePoSort.push_back(vLinePo[5]);//8
		vLinePoSort.push_back(vLinePo[4]);//9
	}
	else
	{
		vLinePoSort.push_back(vLinePo[4]);//8
		vLinePoSort.push_back(vLinePo[5]);//9
	}
	if (vLinePo[0].y != poStd.y)
	{
		int y = vLinePo[0].y - poStd.y;
		for (int i = 0; i < vLinePo.size(); i++)
		{
			vLinePo[i].y -= y;
		}
	}
	int LinePOLast = vLinePo.size() - 1;
	if (vLinePo[LinePOLast].x != poStd.x)
	{
		int x = vLinePo[LinePOLast].x - poStd.x;
		for (int i = 0; i < vLinePo.size(); i++)
		{
			vLinePo[i].x -= x;
		}
	}
	cv::Mat img = Image.clone();
	cv::Mat distTr;
	cv::distanceTransform(Image, distTr, cv::DIST_C, 3);
	double dMin, dMax;
	cv::Point poMin, poMax;
	cv::minMaxLoc(distTr, &dMin, &dMax, &poMin, &poMax);
	distTr.convertTo(distTr, CV_8UC1);
	int nMaxTh = dMax * 0.8;
	cv::Mat bin;
	cv::threshold(distTr, bin, nMaxTh, 255, cv::THRESH_BINARY);
	cv::Mat thin = ImageThining(Image.clone());

	if (contours.size() == 0)
	{
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	}

	std::vector<std::vector<cv::Point>> blobPoints;
	blobPoints.resize(contours.size());
	double dMaxArea = 0;
	for (int sz = 0; sz < contours.size(); sz++)
	{
		for (int i = 0; i < contours[sz].size(); i++)
		{
			blobPoints[sz].push_back(contours[sz][i]);
		}
	}

	std::vector<std::pair<cv::Point, double>> vPoAPair;
	std::vector<std::pair<cv::Point, cv::Point>> vPoPiar;
	float fRad2 = std::atan2f(vLinePoSort[3].y - vLinePoSort[2].y, vLinePoSort[3].x - vLinePoSort[2].x);
	float fRad4 = std::atan2f(vLinePoSort[5].y - vLinePoSort[4].y, vLinePoSort[5].x - vLinePoSort[4].x);
	float fRad6 = std::atan2f(vLinePoSort[7].y - vLinePoSort[6].y, vLinePoSort[7].x - vLinePoSort[6].x);
	float fCosR2 = std::cos(fRad2) == 0 ? 1 : 1 / std::cos(fRad2);
	float fSinR2 = std::sin(fRad2) == 0 ? 1 : 1 / std::sin(fRad2);
	float fCosR4 = std::cos(fRad4) == 0 ? 1 : 1 / std::cos(fRad4);
	float fSinR4 = std::sin(fRad4) == 0 ? 1 : 1 / std::sin(fRad4);
	float fCosR6 = std::cos(fRad6) == 0 ? 1 : 1 / std::cos(fRad6);
	float fSinR6 = std::sin(fRad6) == 0 ? 1 : 1 / std::sin(fRad6);
	float fLineX2 = (vLinePoSort[2].x - vLinePoSort[2].x)*fCosR2 + (vLinePoSort[2].y - vLinePoSort[2].y)*fSinR2;
	float fLineX3 = (vLinePoSort[3].x - vLinePoSort[2].x)*fCosR2 + (vLinePoSort[3].y - vLinePoSort[2].y)*fSinR2;
	float fLineX4 = (vLinePoSort[4].x - vLinePoSort[4].x)*fCosR4 + (vLinePoSort[4].y - vLinePoSort[4].y)*fSinR4;
	float fLineX5 = (vLinePoSort[5].x - vLinePoSort[4].x)*fCosR4 + (vLinePoSort[5].y - vLinePoSort[4].y)*fSinR4;
	float fLineX6 = (vLinePoSort[6].x - vLinePoSort[6].x)*fCosR6 + (vLinePoSort[6].y - vLinePoSort[6].y)*fSinR6;
	float fLineX7 = (vLinePoSort[7].x - vLinePoSort[6].x)*fCosR6 + (vLinePoSort[7].y - vLinePoSort[6].y)*fSinR6;

	int dy = vLinePoSort[3].y - vLinePoSort[2].y;
	int dx = vLinePoSort[3].x - vLinePoSort[2].x;
	float A23 = dx == 0 ? 9999 : (float)dy / dx;
	dy = vLinePoSort[5].y - vLinePoSort[4].y;
	dx = vLinePoSort[5].x - vLinePoSort[4].x;
	float A45 = dx == 0 ? 9999 : (float)dy / dx;
	dy = vLinePoSort[7].y - vLinePoSort[6].y;
	dx = vLinePoSort[7].x - vLinePoSort[6].x;
	float A67 = dx == 0 ? 9999 : (float)dy / dx;
	for (int sz = 0; sz < blobPoints.size(); sz++)
	{
		for (int i = 0; i < blobPoints[sz].size(); i++)
		{
			cv::Point st;
			cv::Point ed;

			st = ed = blobPoints[sz][i];
			dy = 3;
			dx = 3;
			float A = nCalc;// (double)dy / dx;
			int calcX2 = (blobPoints[sz][i].x - vLinePoSort[2].x)*fCosR2 + (blobPoints[sz][i].y - vLinePoSort[2].y)*fSinR2;
			int calcX4 = (blobPoints[sz][i].x - vLinePoSort[4].x)*fCosR4 + (blobPoints[sz][i].y - vLinePoSort[4].y)*fSinR4;
			int calcX6 = (blobPoints[sz][i].x - vLinePoSort[6].x)*fCosR6 + (blobPoints[sz][i].y - vLinePoSort[6].y)*fSinR6;
			//-
			if (vLinePoSort[0].x <= blobPoints[sz][i].x && vLinePoSort[1].x >= blobPoints[sz][i].x)
			{
				A = -9999;
				dy = 0;
			}
			//|
			else if (vLinePoSort[8].y <= blobPoints[sz][i].y && vLinePoSort[9].y >= blobPoints[sz][i].y)
			{
				A = 9999;
				dx = 0;
			}
			//X
			else if (/*vLinePoSort[1].x <= blobPoints[sz][i].x &&*/ (fLineX2 <= calcX2 && fLineX3 >= calcX2))
			{
				A = A23;
			}
			else if (fLineX4 <= calcX4 && fLineX5 >= calcX4)
			{
				A = A45;
			}
			else if ((fLineX7 >= calcX6 && fLineX6 <= calcX6) /*&& vLinePoSort[7].x >= blobPoints[sz][i].x*/)
			{
				A = A67;
			}
			else
			{
				A *= (-1);
			}
			if (i == (blobPoints[sz].size() - 1))
			{
				dy = 0;
			}

			if (std::abs(dx) < 3)// | 세로선
			{
				vPoAPair.push_back(std::pair<cv::Point, double>(blobPoints[sz][i], 9999));
			}
			else if (std::abs(dy) < 3)// - 가로선
			{
				vPoAPair.push_back(std::pair<cv::Point, double>(blobPoints[sz][i], -9999));
			}
			else // X 사선
			{
				vPoAPair.push_back(std::pair<cv::Point, double>(blobPoints[sz][i], A));
			}
		}
	}

	for (int sz = 0; sz < vPoAPair.size(); sz++)
	{
		cv::Point st = vPoAPair[sz].first;
		cv::Point ed = vPoAPair[sz].first;
		double A = vPoAPair[sz].second;
		if (A == 9999)
		{
			for (int x = vPoAPair[sz].first.x; x > 0; x--)
			{
				int y = vPoAPair[sz].first.y;
				if (Image.ptr(y)[x] != lLebel)
					continue;
				st.x = x;
				st.y = y;
			}
			for (int x = st.x; x < Image.cols; x++)
			{
				int y = vPoAPair[sz].first.y;
				if (Image.ptr(y)[x] != lLebel)
					continue;
				ed.x = x;
				ed.y = y;
			}
			vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
		}
		else if (A == -9999)
		{
			for (int y = vPoAPair[sz].first.y; y > 0; y--)
			{
				int x = vPoAPair[sz].first.x;
				if (Image.ptr(y)[x] != lLebel)
					continue;
				st.x = x;
				st.y = y;
			}
			for (int y = st.y; y < Image.rows; y++)
			{
				int x = vPoAPair[sz].first.x;
				if (Image.ptr(y)[x] != lLebel)
					continue;
				ed.x = x;
				ed.y = y;
			}

			vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
		}
		else
		{
			double A_ = -1 / A;
			double B = st.y - (A_ * st.x);
			cv::Point org = st;
			for (int x = vPoAPair[sz].first.x; x > 0; x--)
			{
				int y = A_ * x + B;
				if (y < 0 || y >= Image.rows || Image.ptr(y)[x] != lLebel)
					continue;
				st.x = x;
				st.y = y;
			}
			for (int x = st.x; x < Image.cols; x++)
			{
				int y = A_ * x + B;
				if (y < 0 || y >= Image.rows || Image.ptr(y)[x] != lLebel)
					continue;
				ed.x = x;
				ed.y = y;
			}
			vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
		}
	}
	double dMaxDist = 0;
	double dMinDist = 0;
	cv::Point MxPoSt, MxPoEd;
	for (int sz = 0; sz < vPoPiar.size(); sz++)
	{
		cv::Point st = vPoPiar[sz].first;
		cv::Point ed = vPoPiar[sz].second;

		double dDist = std::sqrt(std::pow((double)ed.x - st.x, 2) + std::pow((double)ed.y - st.y, 2));
		if (dDist > dMaxDist)
		{
			dMaxDist = dDist;
			MxPoSt = st;
			MxPoEd = ed;
		}
	}
#if _DEBUG
	cv::Mat draw;
	cv::cvtColor(Image, draw, cv::COLOR_GRAY2BGR);
	cv::line(draw, MxPoSt, MxPoEd, cv::Scalar(0, 0, 255), 1);
	cv::imwrite("D:\\thickness2Img.bmp",draw);
#endif // _DEBUG

	std::vector<cv::Point> vPo;
	vPo.push_back(MxPoSt);
	vPo.push_back(MxPoEd);
	return vPo;
}

std::vector<cv::Point> CPInsp_AlgoBlob::thickness3(cv::Mat Image, int nWndDir, cv::Point Center, std::vector<std::vector<cv::Point>> contours, cv::Rect rcBlob, std::vector<cv::Point> vLinePo, long lLebel)
{
	if (vLinePo.size() < 6)
	{
		return thickness(Image, nWndDir, Center, contours, rcBlob, lLebel);
	}
	////좌측
	//nWndDir += 0x04;
	////우측
	//nWndDir += 0x08;
	////상측
	//nWndDir += 0x01;
	////하측
	//nWndDir += 0x02;
	cv::Point poStd(0, 0);
	int nCalc = 1;
	//1 2
	//   3
	//    4
	//     5
	//     6
	std::vector<cv::Point> vLinePoSort;//12,23,34,45,56
	if ((nWndDir & 0x04) == 0x04)//left
	{
		poStd.x = rcBlob.x;
	}
	else if ((nWndDir & 0x08) == 0x08)//right
	{
		poStd.x = rcBlob.x + rcBlob.width;
		nCalc *= -1;
	}
	if ((nWndDir & 0x01) == 0x01)//top
	{
		poStd.y = rcBlob.y;
	}
	else if ((nWndDir & 0x02) == 0x02)//bot
	{
		poStd.y = rcBlob.y + rcBlob.height;
		nCalc *= -1;
	}
	vLinePoSort.push_back(vLinePo[0]);//0
	vLinePoSort.push_back(vLinePo[1]);//1
	vLinePoSort.push_back(vLinePo[1]);//2
	vLinePoSort.push_back(vLinePo[2]);//3
	vLinePoSort.push_back(vLinePo[2]);//4
	vLinePoSort.push_back(vLinePo[3]);//5
	vLinePoSort.push_back(vLinePo[3]);//6
	vLinePoSort.push_back(vLinePo[4]);//7
	vLinePoSort.push_back(vLinePo[4]);//8
	vLinePoSort.push_back(vLinePo[5]);//9
	cv::Mat img = Image.clone();
	cv::Mat distTr;
	cv::distanceTransform(Image, distTr, cv::DIST_C, 3);
	double dMin, dMax;
	cv::Point poMin, poMax;
	cv::minMaxLoc(distTr, &dMin, &dMax, &poMin, &poMax);
	distTr.convertTo(distTr, CV_8UC1);
	int nMaxTh = dMax * 0.8;
	cv::Mat bin;
	cv::threshold(distTr, bin, nMaxTh, 255, cv::THRESH_BINARY);
	cv::Mat thin = ImageThining(Image.clone());

	if (contours.size() == 0)
	{
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	}

	std::vector<std::vector<cv::Point>> blobPoints;
	blobPoints.resize(contours.size());
	double dMaxArea = 0;
	for (int sz = 0; sz < contours.size(); sz++)
	{
		for (int i = 0; i < contours[sz].size(); i++)
		{
			blobPoints[sz].push_back(contours[sz][i]);
		}
	}

	std::vector<std::pair<cv::Point, double>> vPoAPair;
	std::vector<std::pair<cv::Point, cv::Point>> vPoPiar;
	float A = nCalc;// (double)dy / dx;
	for (int nPo = 0; nPo < vLinePoSort.size(); nPo += 2)
	{
		int dy = vLinePoSort[nPo + 1].y - vLinePoSort[nPo + 0].y;
		int dx = vLinePoSort[nPo + 1].x - vLinePoSort[nPo + 0].x;
		float ACal = dx == 0 ? 9999 : (float)dy / dx;

		if (std::abs(dx) < 3)// | 세로선
		{
			int dy = vLinePoSort[nPo + 1].y - vLinePoSort[nPo + 0].y;
			int dx = vLinePoSort[nPo + 1].x - vLinePoSort[nPo + 0].x;
			int st = vLinePoSort[nPo + 1].y < vLinePoSort[nPo + 0].y ? vLinePoSort[nPo + 1].y : vLinePoSort[nPo + 0].y;
			int ed = vLinePoSort[nPo + 1].y < vLinePoSort[nPo + 0].y ? vLinePoSort[nPo + 0].y : vLinePoSort[nPo + 1].y;
			if (st < 0) st = 0;
			if (ed > Image.rows - 1)ed = Image.rows - 1;
			for (int i = st; i <= ed; i++)
			{
				vPoAPair.push_back(std::pair<cv::Point, double>(cv::Point(vLinePoSort[nPo + 0].x, i), 9999));
			}

		}
		else if (std::abs(dy) < 3)// - 가로선
		{
			int dy = vLinePoSort[nPo + 1].y - vLinePoSort[nPo + 0].y;
			int dx = vLinePoSort[nPo + 1].x - vLinePoSort[nPo + 0].x;
			int st = vLinePoSort[nPo + 1].x < vLinePoSort[nPo + 0].x ? vLinePoSort[nPo + 1].x : vLinePoSort[nPo + 0].x;
			int ed = vLinePoSort[nPo + 1].x < vLinePoSort[nPo + 0].x ? vLinePoSort[nPo + 0].x : vLinePoSort[nPo + 1].x;
			if (st < 0) st = 0;
			if (ed > Image.cols - 1)ed = Image.cols - 1;
			for (int i = st; i <= ed; i++)
			{
				vPoAPair.push_back(std::pair<cv::Point, double>(cv::Point(i, vLinePoSort[nPo + 0].y), -9999));
			}
		}
		else
		{
			int dy = vLinePoSort[nPo + 1].y - vLinePoSort[nPo + 0].y;
			int dx = vLinePoSort[nPo + 1].x - vLinePoSort[nPo + 0].x;
			if (std::abs(dy) > std::abs(dx))
			{
				bool bNe = vLinePoSort[nPo + 1].y < vLinePoSort[nPo + 0].y;
				int st = bNe ? vLinePoSort[nPo + 1].y : vLinePoSort[nPo + 0].y;
				int ed = bNe ? vLinePoSort[nPo + 0].y : vLinePoSort[nPo + 1].y;
				float fAngle = (float)dx / dy;
				float fIntercept_x = bNe ? vLinePoSort[nPo + 1].x - (fAngle * st) : vLinePoSort[nPo + 0].x - (fAngle * st);
				if (st < 0) st = 0;
				if (ed > Image.rows - 1)ed = Image.rows - 1;
				for (int i = st; i <= ed; i++)
				{
					int _x = std::roundf(fAngle * i + fIntercept_x);
					vPoAPair.push_back(std::pair<cv::Point, double>(cv::Point(_x, i), ACal));
				}
			}
			else
			{
				bool bNe = vLinePoSort[nPo + 1].x < vLinePoSort[nPo + 0].x;
				int st = bNe ? vLinePoSort[nPo + 1].x : vLinePoSort[nPo + 0].x;
				int ed = bNe ? vLinePoSort[nPo + 0].x : vLinePoSort[nPo + 1].x;
				float fAngle = (float)dy / dx;
				float fIntercept_y = bNe ? vLinePoSort[nPo + 1].y - (fAngle * st) : vLinePoSort[nPo + 0].y - (fAngle * st);
				if (st < 0) st = 0;
				if (ed > Image.cols - 1)ed = Image.cols - 1;
				for (int i = st; i <= ed; i++)
				{
					int _y = std::roundf(fAngle * i + fIntercept_y);
					vPoAPair.push_back(std::pair<cv::Point, double>(cv::Point(i, _y), ACal));
				}
			}
		}
	}

#if _DEBUG
	cv::Mat drawPointImg;
	cv::cvtColor(Image, drawPointImg, cv::COLOR_GRAY2BGR);

	for (int sz = 1; sz < vPoAPair.size(); sz++)
	cv::line(drawPointImg, vPoAPair[sz].first, vPoAPair[sz].first, cv::Scalar(0, 0, 255), 1);
	cv::imwrite("D:\\thickness3PoImg.bmp", drawPointImg);
#endif // _DEBUG
	for (int sz = 0; sz < vPoAPair.size(); sz++)
	{
		cv::Point st(-1, -1);
		cv::Point ed = vPoAPair[sz].first;
		double A = vPoAPair[sz].second;
		if (A == 9999)
		{
			st.x = ed.x;
			st.y = ed.y;
			for (int x = vPoAPair[sz].first.x; x > 0; x--)
			{
				int y = vPoAPair[sz].first.y;
				if (Image.ptr(y)[x] != lLebel)
					continue;
				st.x = x;
				st.y = y;
				ed.x = x;
				ed.y = y;
			}
			int x = st.x > 0 ? st.x : 0;
			for (; x < Image.cols; x++)
			{
				int y = vPoAPair[sz].first.y;
				if (Image.ptr(y)[x] != lLebel)
					continue;
				if (st.x == vPoAPair[sz].first.x && st.y == vPoAPair[sz].first.y)
				{
					st.x = x;
					st.y = y;
				}
				ed.x = x;
				ed.y = y;
			}
			vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
		}
		else if (A == -9999)
		{
			st.x = ed.x;
			st.y = ed.y;
			for (int y = vPoAPair[sz].first.y; y > 0; y--)
			{
				int x = vPoAPair[sz].first.x;
				if (Image.ptr(y)[x] != lLebel)
					continue;
				st.x = x;
				st.y = y;
				ed.x = x;
				ed.y = y;
			}
			int y = st.y > 0 ? st.y : 0;
			for (; y < Image.rows; y++)
			{
				int x = vPoAPair[sz].first.x;
				if (Image.ptr(y)[x] != lLebel)
					continue;
				if (st.x == vPoAPair[sz].first.x && st.y == vPoAPair[sz].first.y)
				{
					st.x = x;
					st.y = y;
				}
				ed.x = x;
				ed.y = y;
			}

			vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
		}
		else
		{
			double A_ = -1 / A;
			double B = vPoAPair[sz].first.y - (A_ * vPoAPair[sz].first.x);
			cv::Point org = st;
			for (int x = vPoAPair[sz].first.x; x > 0; x--)
			{
				int y = A_ * x + B;
				if (y < 0 || y >= Image.rows || Image.ptr(y)[x] != lLebel)
					continue;
				st.x = x;
				st.y = y;
				ed.x = x;
				ed.y = y;
			}
			int x = st.x>0? st.x : 0;
			for (; x < Image.cols; x++)
			{
				int y = A_ * x + B;
				if (y < 0 || y >= Image.rows || Image.ptr(y)[x] != lLebel)
					continue;
				if (st.x == -1 && st.y == -1)
				{
					st.x = x;
					st.y = y;
				}
				ed.x = x;
				ed.y = y;
			}
			vPoPiar.push_back(std::pair<cv::Point, cv::Point>(st, ed));
		}
	}
	double dMaxDist = 0;
	double dMinDist = 0;
	cv::Point MxPoSt, MxPoEd;
	for (int sz = 0; sz < vPoPiar.size(); sz++)
	{
		cv::Point st = vPoPiar[sz].first;
		cv::Point ed = vPoPiar[sz].second;

		double dDist = std::sqrt(std::pow((double)ed.x - st.x, 2) + std::pow((double)ed.y - st.y, 2));
		if (dDist > dMaxDist)
		{
			dMaxDist = dDist;
			MxPoSt = st;
			MxPoEd = ed;
		}
	}
#if _DEBUG
	cv::Mat draw;
	cv::cvtColor(Image, draw, cv::COLOR_GRAY2BGR);
	cv::line(draw, MxPoSt, MxPoEd, cv::Scalar(0, 0, 255), 1);
	cv::imwrite("D:\\thickness3Img.bmp", draw);
#endif // _DEBUG

	std::vector<cv::Point> vPo;
	vPo.push_back(MxPoSt);
	vPo.push_back(MxPoEd);
	return vPo;
}

cv::Mat CPInsp_AlgoBlob::ImageThining(cv::Mat Img)
{
	//cv::Mat ReadImg = cv::imread(std::string(CT2A(imgPath)));
	//ReadImg.convertTo(Img,CV_8UC1);
	cv::threshold(Img, Img, 1, 255, cv::THRESH_OTSU);

	//cv::imwrite(std::string(CT2A(BinPath)),Img);

	cv::Mat tmp, temp1;

	//	cv::Canny(Img,Img,125,225);

	Img.copyTo(tmp);
	Img.copyTo(temp1);
	//	Img.copySize(tmp);
	//	tmp.setTo(0);
	bool flag = true;

	int cnt(0);
	int nStep(0);
	int nPoint[8] = { 0,0,0,0,0,0,0,0 };
	int ntempPoint[8] = { 0,0,0,0,0,0,0,0 };

	while (flag)
	{
		flag = false;
#if 0
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (temp1.data[r*Img.cols + c] == 0)
					continue;

				nPoint[0] = Img.data[(r - 1)*Img.cols + c - 1];
				nPoint[1] = Img.data[(r - 1)*Img.cols + c];
				nPoint[2] = Img.data[(r - 1)*Img.cols + c + 1];

				nPoint[3] = Img.data[(r)*Img.cols + c + 1];
				nPoint[4] = Img.data[(r + 1)*Img.cols + c + 1];

				nPoint[5] = Img.data[(r + 1)*Img.cols + c];
				nPoint[6] = Img.data[(r + 1)*Img.cols + c - 1];

				nPoint[7] = Img.data[(r)*Img.cols + c - 1];

				ntempPoint[0] = temp1.data[(r - 1)*Img.cols + c - 1];
				ntempPoint[1] = temp1.data[(r - 1)*Img.cols + c];
				ntempPoint[2] = temp1.data[(r - 1)*Img.cols + c + 1];
				ntempPoint[3] = temp1.data[(r)*Img.cols + c + 1];
				ntempPoint[4] = temp1.data[(r + 1)*Img.cols + c + 1];
				ntempPoint[5] = temp1.data[(r + 1)*Img.cols + c];
				ntempPoint[6] = temp1.data[(r + 1)*Img.cols + c - 1];
				ntempPoint[7] = temp1.data[(r)*Img.cols + c - 1];
				int nConnect(0);
				// 				int nL(0),nR(0),nT(0),nB(0),BR(0);
				// 				nL = nPoint[0]&nPoint[6]&nPoint[7]&1;
				// 				nR = nPoint[2]&nPoint[3]&nPoint[4]&1;
				// 				nT = nPoint[0]&nPoint[1]&nPoint[2]&1;
				// 				nB = nPoint[4]&nPoint[5]&nPoint[6]&1;
				// 				BR = nPoint[5]&nPoint[3]&nPoint[4]&1;
				// 				int nBCnt = nL+nR+nT+nB + BR;

				int nContin(0);
				bool bContin = false;

				for (int i = 1; i < 8; i++)
				{
					if (nPoint[i - 1] == 0 && nPoint[i] == 255)
						nConnect++;

					if (ntempPoint[i] == 255)
						nContin++;
					else
						nContin = 0;

					if (nContin > 2)
						bContin = true;
				}
				if (nPoint[7] == 0 && nPoint[0] == 255)
					nConnect++;

				if (ntempPoint[0] == 255)
					nContin++;
				else
					nContin = 0;

				if (nContin > 2)
					bContin = true;

				if (nConnect == 1 && bContin/*nBCnt>0*/)
				{
					temp1.data[r*Img.cols + c] = 0;
					flag = true;

					// 					CString SavePath;
					// 					SavePath.Format(_T("%s_step_%d.bmp"),sDirPath,nStep);
					// 					cv::imwrite(std::string(CT2A(SavePath)),temp1);
					// 					SavePath.Format(_T("%s_step_%d_Img.bmp"),sDirPath,nStep);
					// 					cv::imwrite(std::string(CT2A(SavePath)),Img);
					nStep++;
				}
			}
		}
		temp1.copyTo(Img);
		CString SavePath;
		SavePath.Format(_T("%s_temp_%d.bmp"), sDirPath, cnt);
		cv::imwrite(std::string(CT2A(SavePath)), temp1);
		cnt++;
#endif
#if 1

		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (temp1.data[r*Img.cols + c] == 0)
					continue;

				nPoint[0] = Img.data[(r - 1)*Img.cols + c - 1];
				nPoint[1] = Img.data[(r - 1)*Img.cols + c];
				nPoint[2] = Img.data[(r - 1)*Img.cols + c + 1];

				nPoint[3] = Img.data[(r)*Img.cols + c + 1];
				nPoint[4] = Img.data[(r + 1)*Img.cols + c + 1];

				nPoint[5] = Img.data[(r + 1)*Img.cols + c];
				nPoint[6] = Img.data[(r + 1)*Img.cols + c - 1];

				nPoint[7] = Img.data[(r)*Img.cols + c - 1];

				ntempPoint[0] = temp1.data[(r - 1)*Img.cols + c - 1];
				ntempPoint[1] = temp1.data[(r - 1)*Img.cols + c];
				ntempPoint[2] = temp1.data[(r - 1)*Img.cols + c + 1];
				ntempPoint[3] = temp1.data[(r)*Img.cols + c + 1];
				ntempPoint[4] = temp1.data[(r + 1)*Img.cols + c + 1];
				ntempPoint[5] = temp1.data[(r + 1)*Img.cols + c];
				ntempPoint[6] = temp1.data[(r + 1)*Img.cols + c - 1];
				ntempPoint[7] = temp1.data[(r)*Img.cols + c - 1];
				int nConnect(0);
				int nPixCnt(0);
				int nCndi1(0), nCndi2(0);

				for (int i = 1; i < 8; i++)
				{
					if (nPoint[i - 1] == 0 && nPoint[i] == 255)
						nConnect++;
					if (nPoint[i] == 255)
						nPixCnt++;
				}
				if (nPoint[7] == 0 && nPoint[0] == 255)
					nConnect++;
				if (nPoint[0] == 255)
					nPixCnt++;

				nCndi1 = nPoint[1] & nPoint[7] & nPoint[5];
				nCndi2 = nPoint[3] & nPoint[7] & nPoint[5];

				if (nConnect == 1 && (nPixCnt > 1 && nPixCnt < 7) && nCndi1 == 0 && nCndi2 == 0)
				{
					temp1.data[r*Img.cols + c] = 0;
					flag = true;

					nStep++;
				}
			}
		}
		temp1.copyTo(Img);
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (temp1.data[r*Img.cols + c] == 0)
					continue;

				nPoint[0] = Img.data[(r - 1)*Img.cols + c - 1];
				nPoint[1] = Img.data[(r - 1)*Img.cols + c];
				nPoint[2] = Img.data[(r - 1)*Img.cols + c + 1];

				nPoint[3] = Img.data[(r)*Img.cols + c + 1];
				nPoint[4] = Img.data[(r + 1)*Img.cols + c + 1];

				nPoint[5] = Img.data[(r + 1)*Img.cols + c];
				nPoint[6] = Img.data[(r + 1)*Img.cols + c - 1];

				nPoint[7] = Img.data[(r)*Img.cols + c - 1];
				int nConnect(0);
				int nPixCnt(0);
				int nCndi1(0), nCndi2(0);

				for (int i = 1; i < 8; i++)
				{
					if (nPoint[i - 1] == 0 && nPoint[i] == 255)
						nConnect++;
					if (nPoint[i] == 255)
						nPixCnt++;
				}
				if (nPoint[7] == 0 && nPoint[0] == 255)
					nConnect++;
				if (nPoint[0] == 255)
					nPixCnt++;

				nCndi1 = nPoint[1] & nPoint[3] & nPoint[5];
				nCndi2 = nPoint[1] & nPoint[3] & nPoint[7];

				if (nConnect == 1 && (nPixCnt > 1 && nPixCnt < 7) && nCndi1 == 0 && nCndi2 == 0)
				{
					temp1.data[r*Img.cols + c] = 0;
					flag = true;
					nStep++;
				}
			}
		}
		temp1.copyTo(Img);
		cnt++;
#else
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (Img.data[r*Img.cols + c] == 255)
				{
					int up(0), down(0), side(0), nLeft(0), nRight(0), Lt(0), Rt(0), Lb(0), Rb(0), upside(0), Line1(0), Line2(0), Line3(0), Line4(0);
					up += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					up += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					up += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					side += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					side += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;

					nLeft += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					nLeft += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					nLeft += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					nRight += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					nRight += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					nRight += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					upside += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					upside += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Lt += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Lt += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					Lt += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;

					Rt += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					Rt += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					Rt += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;

					Lb += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Lb += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					Lb += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Rb += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					Rb += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					Rb += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Line1 += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Line1 += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;

					Line2 += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					Line2 += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Line3 += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Line3 += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;

					Line4 += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Line4 += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;

					if ((up >= 1 && down >= 1 && side >= 1) || (nLeft >= 1 && nRight >= 1 && upside >= 1))
						if (up + down + side > 2 && up + down + side < 7)
						{
							int backg = Img.data[r*Img.cols + c - 1] * Img.data[(r + 1)*Img.cols + c] > 0 ? 1 : 0;
							if (Img.data[(r - 1)*Img.cols + c] * backg == 0)
							{
								if (Img.data[r*Img.cols + c + 1] * backg == 0)
								{
									temp1.data[r*Img.cols + c] = 0;
									flag = true;
								}
							}
						}
				}

			}
		}
		temp1.copyTo(Img);
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (Img.data[r*Img.cols + c] == 255)//현재 픽셀 == edge
				{
					int up(0), down(0), side(0), nLeft(0), nRight(0), Lt(0), Rt(0), Lb(0), Rb(0), upside(0);
					up += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					up += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					up += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					side += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					side += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;

					nLeft += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					nLeft += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					nLeft += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					nRight += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					nRight += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					nRight += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					upside += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					upside += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;


					Lt += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Lt += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					Lt += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;

					Rt += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					Rt += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					Rt += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;

					Lb += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Lb += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					Lb += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Rb += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					Rb += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					Rb += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					if ((up >= 1 && down >= 1 && side >= 1) || (nLeft >= 1 && nRight >= 1 && upside >= 1))
						if (up + down + side > 2 && up + down + side < 7)
						{
							int backg = Img.data[(r - 1)*Img.cols + c] * Img.data[r*Img.cols + c + 1] > 0 ? 1 : 0;
							if (Img.data[(r + 1)*Img.cols + c] * backg == 0)
							{
								if (Img.data[r*Img.cols + c - 1] * backg == 0)
								{
									temp1.data[r*Img.cols + c] = 0;
									flag = true;
								}
							}
						}
				}
			}
		}

		temp1.copyTo(Img);
		CString SavePath;
		SavePath.Format(_T("%s_temp_%d.bmp"), imgPath, cnt);
		cv::imwrite(std::string(CT2A(SavePath)), temp1);
		cnt++;
#endif
	}

	temp1.copyTo(Img);
	//cv::imwrite(std::string(CT2A(DstPath)), temp1);
	return temp1;
}

bool CPInsp_AlgoBlob::getLineIntersection(cv::Point2f a1, cv::Point2f a2, cv::Point2f b1, cv::Point2f b2, cv::Point2f& intersection)
{
	cv::Point2f r = a2 - a1;
	cv::Point2f s = b2 - b1;
	float denom = r.x * s.y - r.y * s.x;

	if (std::abs(denom) < 1e-6) return false; // 평행

	cv::Point2f diff = b1 - a1;
	float t = (diff.x * s.y - diff.y * s.x) / denom;
	float u = (diff.x * r.y - diff.y * r.x) / denom;

	if (t >= 0.0f && u >= 0.0f && u <= 1.0f)
	{
		intersection = a1 + t * r;
		return true;
	}
	return false;
}
void CPInsp_AlgoBlob::interpolHull(std::vector<cv::Point>& hull, std::vector<cv::Point2f>& interpolated, float spacing)
{
	for (size_t i = 0; i < hull.size(); ++i)
	{
		cv::Point2f A = hull[i];
		cv::Point2f B = hull[(i + 1) % hull.size()];
		cv::Point2f vec = B - A;
		float length = cv::norm(vec);
		int numSteps = std::max(1, static_cast<int>(length / spacing));
		for (int j = 0; j <= numSteps; ++j) {
			float t = j / static_cast<float>(numSteps);
			cv::Point2f pt = A + t * vec;
			interpolated.push_back(pt);
		}
	}

}


int CPInsp_AlgoBlob::InspThickness(AlgoBlob *pAlgoBlob, UCHAR* pUcImgSrc, UCHAR* pUcImgBlob, UCHAR* ucArrBlobDst_except, int nImgWidth, int nImgHeight, int nCntBlobExcept, double dArea, double dArea_except, CRect rcBlob, float fWndCenterX, float fWndCenterY, float fWndWidth, float fWndLength, RECT& ThminRect, RECT& ThmaxRect, double& minVal, double& maxVal, RstAlgoBlob * sRstAlgo)
{
	int nCntBlob(0);
	double dResolX = m_resolX;
	double dResolY = m_resolY;
	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}
	int nMinBlobArea = pAlgoBlob->m_nMinBlobArea;

	double dAreaPix = pAlgoBlob->m_dAreaCurrent / dResolX / dResolY;
	double dAreaWPix = pAlgoBlob->m_dTeachWidth / dResolX;
	double dAreaHPix = pAlgoBlob->m_dTeachLength / dResolY;
	int nWndDir = 0;
	bool bIsTop = fWndCenterY - fWndLength <= 0 ? true : false;
	bool bIsBot = fWndCenterY + fWndLength >= 0 ? true : false;
	bool bTeach = false;
	cv::Mat ContoursMaxImg(nImgHeight, nImgWidth, CV_8UC1, pUcImgBlob);
	UCHAR* ucArrBlobDst = pUcImgBlob;
	cv::Point poConnerLt(0, 0), poConnerRb(nImgWidth - 1, nImgHeight - 1);

	if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
	{
		//좌측
		nWndDir += 0x04;
	}
	else if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
	{
		//우측
		nWndDir += 0x08;
	}
	if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
	{
		//상측
		nWndDir += 0x01;
	}
	else if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
	{
		//하측
		nWndDir += 0x02;
	}
	POINTF RstPoTop;
	POINTF RstPoBot;
	POINTF RstPoLeft;
	POINTF RstPoRight;
	POINTF RstPoDiagLeft;
	POINTF RstPoDiagRight;
	if ((pAlgoBlob->m_bInsp2D_ExceptArea || pAlgoBlob->m_bInsp3D_ExceptArea) && nCntBlobExcept > 0 && dArea_except > 5)
	{
		cv::Mat GrayImg(nImgHeight, nImgWidth, CV_8UC1, pUcImgSrc);
		cv::Mat ExceptAreaImg(nImgHeight, nImgWidth, CV_8UC1, ucArrBlobDst_except);
		cv::Mat ExceptArea(nImgHeight, nImgWidth, CV_8UC1);
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
		cv::dilate(ExceptAreaImg, ExceptAreaImg, kernel);
		cv::erode(ExceptAreaImg, ExceptAreaImg, kernel);
		ExceptArea = ExceptAreaImg.clone();
		//UCHAR* ThiningBuf = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		//m_pCPInsp_Algo->ImageThining(nImgWidth, nImgHeight, ucArrBlobDst, ThiningBuf);
		//cv::Mat ThiningImgBlob(nImgHeight, nImgWidth, CV_8UC1, ThiningBuf);
		//cv::rectangle(ThiningImgBlob, cv::Rect(0, 0, ThiningImgBlob.cols, ThiningImgBlob.rows), cv::Scalar(0), 1);
		if (pAlgoBlob->m_nExceptAreaCal == 1)//and
		{

			//좌측
			if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
			{
				std::vector<cv::Point> vLeft;
				std::vector<int> vnLeftCnt;
				vnLeftCnt.resize(ContoursMaxImg.cols);
				for (int r = 0; r < ContoursMaxImg.rows; r++)
				{
					UCHAR* uPtrex = ExceptAreaImg.ptr(r);
					for (int c = 1; c < ContoursMaxImg.cols; c++)
					{
						if (uPtrex[c] > 0)
						{
							vLeft.push_back(cv::Point(r, c));
							vnLeftCnt[c]++;
							cv::line(ExceptArea, cv::Point(0, r), cv::Point(c - 1, r), cv::Scalar(0));
							break;
						}
					}
				}
				if (vLeft.size() > 3)
				{
					int nMxCnt = 0;
					int nMxIdx = 0;
					for (int c = 1; c < vnLeftCnt.size(); c++)//끝에 완전히 붙는경우 제외
					{
						if (nMxCnt < vnLeftCnt[c])
						{
							nMxCnt = vnLeftCnt[c];
							nMxIdx = c;
						}
					}
					cv::Rect whiteRect(nMxIdx, 0, ExceptArea.cols - nMxIdx, ExceptArea.rows);
					cv::Rect blackRect(0, 0, nMxIdx, ExceptArea.rows);
					for (int c = 0; c < vLeft.size(); c++)
					{
						if (std::abs(vLeft[c].y - nMxIdx) < 5)
						{
							blackRect.y = vLeft[c].x;
							break;
						}
					}
					for (int c = vLeft.size() - 1; c >= 0; c--)
					{
						if (std::abs(vLeft[c].y - nMxIdx) < 5)
						{
							blackRect.height = vLeft[c].x - blackRect.y + 1;
							break;
						}
					}
					std::vector<cv::Point> vLine;
					int nStLineX = vLeft[vLeft.size() - 1].y;
					int nEndLineX = 0;
					for (int c = 0; c < vLeft.size(); c++)
					{
						if (std::abs(vLeft[c].y - nMxIdx) < 5)
						{
							vLine.push_back(vLeft[c]);
							if (vLeft[c].y == nMxIdx)
							{
								nEndLineX = vLeft[c].x;
								if (nStLineX > vLeft[c].x)
									nStLineX = vLeft[c].x;
							}
						}
					}
					if (vLine.size() == 0)
						vLine = vLeft;
					if (nEndLineX == 0)
						nEndLineX = vLine[vLine.size() - 1].x;
					if (nStLineX == vLeft[vLeft.size() - 1].y)
						nStLineX = vLine[0].x;
					for (int c = 0; c < vLine.size(); c++)
					{
						if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
						{
							vLine.erase(vLine.begin() + c);
							c--;
						}
					}
					float a, b;
					if (!bIsBot && !bIsTop)
					{
						double dA, dB;
						double dAngle = 0;
						int nMeasureDirection = Left_Line;
						bool bIsHorizon = false;
						POINTF poDrawLine[2];
						memset(poDrawLine, 0, sizeof(POINTF) * 2);
						int nLineLength = 0;
						if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
							m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
						a = (float)dA;
						b = (float)dB;
					}
					else
						m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

#if LineDraw
					if (a != 0)
					{
						if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
						{
							b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
						}
						if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
						{
							for (int c = 0; c < vLine[0].x; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
							}
						}
						for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
						}
						if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
						{
							for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.rows; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
							}
						}
					}
#else

					std::vector<cv::Point> vDrawPo;
					for (int c = 0; c < vLeft.size(); c++)
					{
						if (vLine[0].x > vLeft[c].x)
							continue;
						else if (vLine[vLine.size() - 1].x < vLeft[c].x)
							break;
						int nStdX = (int)std::roundf(vLeft[c].x*a + b);
						if (std::abs(nStdX - vLeft[c].y) > 3)
						{
							vDrawPo.push_back(cv::Point(vLeft[c].x, nStdX));
						}
						else
						{
							vDrawPo.push_back(cv::Point(vLeft[c].x, vLeft[c].y));
						}
					}
					std::vector<cv::Point> vBlank;
					for (int l = 1; l < vDrawPo.size(); l++)
					{
						if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
						{
							float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
							float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
							for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
							{
								vBlank.push_back(cv::Point(x, fTmpA*x + fTmpB));
							}
						}
					}

					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						for (int c = 0; c < vLeft[0].x; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(0));
						}
					}
					//for (int c = 0; c < vLeft.size(); c++)
					//{
					//	//int nStdX = (int)std::roundf(c*a + b);
					//	cv::line(ExceptArea, cv::Point(0, vLeft[c].x), cv::Point(vLeft[c].y - 1, vLeft[c].x), cv::Scalar(0));
					//	cv::line(ExceptArea, cv::Point(vLeft[c].y, vLeft[c].x), cv::Point(ExceptArea.cols, vLeft[c].x), cv::Scalar(255));
					//}
					if (pAlgoBlob->m_nThickDir != 1)
					{
						for (int c = 0; c < vDrawPo.size(); c++)
						{
							//int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, vDrawPo[c].x), cv::Point(vDrawPo[c].y - 1, vDrawPo[c].x), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(vDrawPo[c].y, vDrawPo[c].x), cv::Point(ExceptArea.cols, vDrawPo[c].x), cv::Scalar(255));
						}
						for (int b = 0; b < vBlank.size(); b++)
						{
							cv::line(ExceptArea, cv::Point(0, vBlank[b].x), cv::Point(vBlank[b].y - 1, vBlank[b].x), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(vBlank[b].y, vBlank[b].x), cv::Point(ExceptArea.cols, vBlank[b].x), cv::Scalar(255));
						}
						if (/*(pAlgoBlob->m_nThickDir != 1) &&*/ bIsTop)
						{
							for (int c = vLeft[vLeft.size() - 1].x; c < ExceptArea.rows; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(0));
							}
						}
					}
#endif
					int nLeft = (int)std::roundf(vLine[0].x*a + b);
					poConnerLt.x = nLeft > 0 ? nLeft : 0;
					poConnerLt.y = vLine[0].x;
					nLeft = (int)std::roundf(vLine[vLine.size() - 1].x*a + b);
					poConnerRb.x = nLeft > 0 ? nLeft : 0;
					poConnerRb.y = vLine[vLine.size() - 1].x;
				}
			}
			//우측
			else if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
			{
				std::vector<cv::Point> vRight;
				std::vector<int> vnRightCnt;
				vnRightCnt.resize(ContoursMaxImg.cols);
				for (int r = 0; r < ContoursMaxImg.rows; r++)
				{
					UCHAR* uPtrex = ExceptAreaImg.ptr(r);
					for (int c = ContoursMaxImg.cols - 1; c >= 0; c--)
					{
						if (uPtrex[c] > 0)
						{
							vRight.push_back(cv::Point(r, c));
							vnRightCnt[c]++;
							cv::line(ExceptArea, cv::Point(c + 1, r), cv::Point(ExceptArea.cols - 1, r), cv::Scalar(0));
							break;
						}
					}
				}
				if (vRight.size() > 3)
				{
					int nMxCnt = 0;
					int nMxIdx = 0;
					for (int c = 0; c < vnRightCnt.size() - 1; c++)//끝에 완전히 붙는경우 제외
					{
						if (nMxCnt < vnRightCnt[c])
						{
							nMxCnt = vnRightCnt[c];
							nMxIdx = c;
						}
					}
					cv::Rect blackRect(nMxIdx, 0, ExceptArea.cols - nMxIdx, 0);
					cv::Rect whiteRect(0, 0, nMxIdx, ExceptArea.rows);
					for (int c = 0; c < vRight.size(); c++)
					{
						if (std::abs(vRight[c].y - nMxIdx) < 5)
						{
							blackRect.y = vRight[c].x;
							break;
						}
					}
					for (int c = vRight.size() - 1; c >= blackRect.x; c--)
					{
						if (std::abs(vRight[c].y - nMxIdx) < 5)
						{
							blackRect.height = vRight[c].x - blackRect.y + 1;
							break;
						}
					}
					std::vector<cv::Point> vLine;
					int nStLineX = vRight[vRight.size() - 1].y;
					int nEndLineX = 0;
					for (int c = 0; c < vRight.size(); c++)
					{
						if (std::abs(vRight[c].y - nMxIdx) < 5)
						{
							vLine.push_back(vRight[c]);
							if (vRight[c].y == nMxIdx)
							{
								nEndLineX = vRight[c].x;
								if (nStLineX > vRight[c].x)
									nStLineX = vRight[c].x;
							}
						}
					}
					if (vLine.size() == 0)
						vLine = vRight;
					if (nEndLineX == 0)
						nEndLineX = vLine[vLine.size() - 1].x;
					if (nStLineX == vRight[vRight.size() - 1].y)
						nStLineX = vLine[0].x;
					for (int c = 0; c < vLine.size(); c++)
					{
						if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
						{
							vLine.erase(vLine.begin() + c);
							c--;
						}
					}

					float a, b;
					if (!bIsBot && !bIsTop)
					{
						double dA, dB;
						double dAngle = 0;
						int nMeasureDirection = Right_Line;
						bool bIsHorizon = false;
						POINTF poDrawLine[2];
						memset(poDrawLine, 0, sizeof(POINTF) * 2);
						int nLineLength = 0;
						if (nMeasureDirection == Center_Line || nMeasureDirection == Right_Line || nMeasureDirection == Right_Line)
							m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
						a = (float)dA;
						b = (float)dB;
					}
					else
						m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

					std::vector<cv::Point> vDrawPo;
					for (int c = 0; c < vRight.size(); c++)
					{
						if (vLine[0].x > vRight[c].x)
							continue;
						else if (vLine[vLine.size() - 1].x < vRight[c].x)
							break;
						int nStdX = (int)std::roundf(vRight[c].x*a + b);
						if (std::abs(nStdX - vRight[c].y) > 3)
						{
							vDrawPo.push_back(cv::Point(vRight[c].x, nStdX));
						}
						else
						{
							vDrawPo.push_back(cv::Point(vRight[c].x, vRight[c].y));
						}
					}
					std::vector<cv::Point> vBlank;
					for (int l = 1; l < vDrawPo.size(); l++)
					{
						if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
						{
							float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
							float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
							for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
							{
								vBlank.push_back(cv::Point(x, fTmpA*x + fTmpB));
							}
						}
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
					}
#if LineDraw
					if (a != 0)
					{
						if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
						{
							for (int c = 0; c < vLine[0].x; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
							}
						}
						for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
						if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
						{
							for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.rows; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
							}
						}
					}
#else
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						for (int c = 0; c < vRight[0].x; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
					}
					//for (int c = 0; c < vRight.size(); c++)
					//{
					//	//int nStdX = (int)std::roundf(c*a + b);
					//	cv::line(ExceptArea, cv::Point(0, vRight[c].x), cv::Point(vRight[c].y, vRight[c].x), cv::Scalar(255));
					//	cv::line(ExceptArea, cv::Point(vRight[c].y + 1, vRight[c].x), cv::Point(ExceptArea.cols, vRight[c].x), cv::Scalar(0));
					//}
					if (pAlgoBlob->m_nThickDir != 1)
					{
						for (int c = 0; c < vDrawPo.size(); c++)
						{
							cv::line(ExceptArea, cv::Point(0, vDrawPo[c].x), cv::Point(vDrawPo[c].y, vDrawPo[c].x), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(vDrawPo[c].y + 1, vDrawPo[c].x), cv::Point(ExceptArea.cols, vDrawPo[c].x), cv::Scalar(0));
						}
						for (int b = 0; b < vBlank.size(); b++)
						{
							//int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, vBlank[b].x), cv::Point(vBlank[b].y, vBlank[b].x), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(vBlank[b].y + 1, vBlank[b].x), cv::Point(ExceptArea.cols, vBlank[b].x), cv::Scalar(0));
						}
						if (bIsTop)
						{
							for (int c = vRight[vRight.size() - 1].x; c < ExceptArea.rows; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
							}
						}
					}
#endif
					int nRight = (int)std::roundf(vLine[0].x*a + b);
					poConnerLt.x = nRight > 0 ? nRight : 0;
					poConnerLt.y = vLine[0].x;
					nRight = (int)std::roundf(vLine[vLine.size() - 1].x*a + b);
					poConnerRb.x = nRight > 0 ? nRight : 0;
					poConnerRb.y = vLine[vLine.size() - 1].x;
				}
			}
			//상측
			if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
			{
				std::vector<cv::Point> vTop;
				std::vector<int> vnTopCnt;
				vnTopCnt.resize(ContoursMaxImg.rows);
				int nCntSt = poConnerLt.x;
				int nCntEd = ContoursMaxImg.cols;
				if ((nWndDir & 0x08) == 0x08)
				{
					nCntSt = 0;
					nCntEd = poConnerLt.x;
				}
				for (int c = nCntSt; c < nCntEd; c++)
				{
					for (int r = 1; r < ContoursMaxImg.rows; r++)
					{
						UCHAR* uPtrex = ExceptAreaImg.ptr(r);
						if (uPtrex[c] > 0)
						{
							vTop.push_back(cv::Point(c, r));
							vnTopCnt[r]++;
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, r - 1), cv::Scalar(0));
							break;
						}
					}
				}
				if (vTop.size() > 3)
				{
					int nMxCnt = 0;
					int nMxIdx = 0;
					for (int r = 1; r < vnTopCnt.size(); r++)//끝에 완전히 붙는경우 제외
					{
						if (nMxCnt < vnTopCnt[r])
						{
							nMxCnt = vnTopCnt[r];
							nMxIdx = r;
						}
					}
					cv::Rect blackRect(0, 0, ExceptArea.cols, nMxIdx);
					cv::Rect whiteRect(0, nMxIdx, ExceptArea.cols, ExceptArea.rows);
					for (int r = 0; r < vTop.size(); r++)
					{
						if (std::abs(vTop[r].y - nMxIdx) < 5)
						{
							blackRect.x = vTop[r].x;
							break;
						}
					}
					for (int r = vTop.size() - 1; r >= 0; r--)
					{
						if (std::abs(vTop[r].y - nMxIdx) < 5)
						{
							blackRect.width = vTop[r].x - blackRect.x + 1;
							break;
						}
					}
					std::vector<cv::Point> vLine;
					int nStLineX = vTop[vTop.size() - 1].y;
					int nEndLineX = 0;
					for (int c = 0; c < vTop.size(); c++)
					{
						if (std::abs(vTop[c].y - nMxIdx) < 5)
						{
							vLine.push_back(vTop[c]);
							if (vTop[c].y == nMxIdx)
							{
								nEndLineX = vTop[c].x;
								if (nStLineX > vTop[c].x)
									nStLineX = vTop[c].x;
							}
						}
					}
					if (vLine.size() == 0)
						vLine = vTop;
					if (nEndLineX == 0)
						nEndLineX = vLine[vLine.size() - 1].x;
					if (nStLineX == vTop[vTop.size() - 1].y)
						nStLineX = vLine[0].x;
					for (int c = 0; c < vLine.size(); c++)
					{
						if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
						{
							vLine.erase(vLine.begin() + c);
							c--;
						}
					}
					float a, b;
					if (pAlgoBlob->m_nThickDir == 2)
					{
						double dA, dB;
						double dAngle = 0;
						int nMeasureDirection = Left_Line;
						bool bIsHorizon = true;
						POINTF poDrawLine[2];
						memset(poDrawLine, 0, sizeof(POINTF) * 2);
						int nLineLength = 0;
						if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
							m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
						a = (float)dA;
						b = (float)dB;
					}
					else
						m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

					if ((nWndDir & 0x08) == 0x08)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
					}
#if LineDraw
					if (a != 0)
					{
						if ((pAlgoBlob->m_nThickDir != 2) && bIsTop)
						{
							if ((nWndDir & 0x04) == 0x04)
							{
								for (int c = vLine[0].x; c < ExceptArea.cols; c++)
								{
									int nStdY = (int)std::roundf(c*a + b);
									cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
									cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
								}
							}
							if ((nWndDir & 0x08) == 0x08)
							{
								for (int c = 0; c < vLine[0].x; c++)
								{
									int nStdY = (int)std::roundf(c*a + b);
									cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
									cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
								}
							}
						}
						for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
						{
							int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
						}
						if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
						{
							for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
							}
						}
					}
#else
					std::vector<cv::Point> vDrawPo;
					for (int c = 0; c < vTop.size(); c++)
					{
						if (vLine[0].x > vTop[c].x)
							continue;
						else if (vLine[vLine.size() - 1].x < vTop[c].x)
							break;
						int nStdY = (int)std::roundf(vTop[c].x*a + b);
						if (std::abs(nStdY - vTop[c].y) > 3)
						{
							vDrawPo.push_back(cv::Point(vTop[c].x, nStdY));
						}
						else
						{
							vDrawPo.push_back(cv::Point(vTop[c].x, vTop[c].y));
						}
					}
					std::vector<cv::Point> vBlank;
					for (int l = 1; l < vDrawPo.size(); l++)
					{
						if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
						{
							float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
							float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
							for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
							{
								vBlank.push_back(cv::Point(x, fTmpA*x + fTmpB));
							}
						}
					}

					if ((pAlgoBlob->m_nThickDir != 2) && bIsTop)
					{
						if ((nWndDir & 0x04) == 0x04)
						{
							for (int c = vTop[vTop.size() - 1].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
							}
						}
						if ((nWndDir & 0x08) == 0x08)
						{
							for (int c = 0; c < vTop[0].x; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
							}
						}
					}
					//for (int c = 0; c < vTop.size(); c++)
					//{
					//	//int nStdY = (int)std::roundf(c*a + b);
					//	cv::line(ExceptArea, cv::Point(vTop[c].x, 0), cv::Point(vTop[c].x, vTop[c].y - 1), cv::Scalar(0));
					//	cv::line(ExceptArea, cv::Point(vTop[c].x, vTop[c].y), cv::Point(vTop[c].x, ExceptArea.rows), cv::Scalar(255));
					//}
					if (pAlgoBlob->m_nThickDir != 2)
					{
						for (int c = 0; c < vDrawPo.size(); c++)
						{
							cv::line(ExceptArea, cv::Point(vDrawPo[c].x, 0), cv::Point(vDrawPo[c].x, vDrawPo[c].y - 1), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(vDrawPo[c].x, vDrawPo[c].y), cv::Point(vDrawPo[c].x, ExceptArea.rows), cv::Scalar(255));
						}
						for (int b = 0; b < vBlank.size(); b++)
						{
							//int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(vBlank[b].x, 0), cv::Point(vBlank[b].x, vBlank[b].y - 1), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(vBlank[b].x, vBlank[b].y), cv::Point(vBlank[b].x, ExceptArea.rows), cv::Scalar(255));
						}
						if (fWndCenterX + fWndWidth <= 0)
						{
							for (int c = vTop[vTop.size() - 1].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
							}
						}
					}
#endif
					if ((nWndDir & 0x04) == 0x04)
					{
						poConnerRb.y = poConnerLt.y;
						int nTop = (int)std::roundf(vLine[0].x*a + b);
						poConnerRb.x = vLine[0].x;
						poConnerLt.y = nTop > 0 ? nTop : 0;
					}
					else if ((nWndDir & 0x08) == 0x08)
					{
						poConnerRb.x = poConnerLt.x;
						poConnerRb.y = poConnerLt.y;
						int nTop = (int)std::roundf(vLine[vLine.size() - 1].x*a + b);
						poConnerLt.x = vLine[vLine.size() - 1].x;
						poConnerLt.y = nTop > 0 ? nTop : 0;
					}
				}
			}
			//하측
			else if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
			{
				std::vector<cv::Point> vBot;
				std::vector<int> vnBotCnt;
				vnBotCnt.resize(ContoursMaxImg.rows);
				int nCntSt = poConnerLt.x;
				int nCntEd = ContoursMaxImg.cols;
				if ((nWndDir & 0x08) == 0x08)
				{
					nCntSt = 0;
					nCntEd = poConnerLt.x;
				}
				for (int c = nCntSt; c < nCntEd; c++)
				{
					for (int r = ContoursMaxImg.rows - 1; r >= 0; r--)
					{
						UCHAR* uPtrex = ExceptAreaImg.ptr(r);
						if (uPtrex[c] > 0)
						{
							vBot.push_back(cv::Point(c, r));
							vnBotCnt[r]++;
							cv::line(ExceptArea, cv::Point(c, r + 1), cv::Point(c, ExceptArea.rows - 1), cv::Scalar(0));
							break;
						}
					}
				}
				if (vBot.size() > 3)
				{
					int nMxCnt = 0;
					int nMxIdx = 0;
					for (int r = 0; r < vnBotCnt.size() - 1; r++)//끝에 완전히 붙는경우 제외
					{
						if (nMxCnt < vnBotCnt[r])
						{
							nMxCnt = vnBotCnt[r];
							nMxIdx = r;
						}
					}
					cv::Rect whiteRect(0, 0, ExceptArea.cols, nMxIdx);
					cv::Rect blackRect(0, nMxIdx, ExceptArea.cols, ExceptArea.rows);
					for (int r = 0; r < vBot.size(); r++)
					{
						if (std::abs(vBot[r].y - nMxIdx) < 5)
						{
							blackRect.x = vBot[r].x;
							break;
						}
					}
					for (int r = vBot.size() - 1; r >= blackRect.y; r--)
					{
						if (std::abs(vBot[r].y - nMxIdx) < 5)
						{
							blackRect.width = vBot[r].x - blackRect.x + 1;
							break;
						}
					}
					std::vector<cv::Point> vLine;
					int nStLineX = vBot[vBot.size() - 1].y;
					int nEndLineX = 0;
					for (int c = 0; c < vBot.size(); c++)
					{
						if (std::abs(vBot[c].y - nMxIdx) < 5)
						{
							vLine.push_back(vBot[c]);
							if (vBot[c].y == nMxIdx)
							{
								nEndLineX = vBot[c].x;
								if (nStLineX > vBot[c].x)
									nStLineX = vBot[c].x;
							}
						}
					}
					if (vLine.size() == 0)
						vLine = vBot;
					if (nEndLineX == 0)
						nEndLineX = vLine[vLine.size() - 1].x;
					if (nStLineX == vBot[vBot.size() - 1].y)
						nStLineX = vLine[0].x;
					for (int c = 0; c < vLine.size(); c++)
					{
						if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
						{
							vLine.erase(vLine.begin() + c);
							c--;
						}
					}
					float a, b;
					if (pAlgoBlob->m_nThickDir == 2)
					{
						double dA, dB;
						double dAngle = 0;
						int nMeasureDirection = Right_Line;
						bool bIsHorizon = true;
						POINTF poDrawLine[2];
						memset(poDrawLine, 0, sizeof(POINTF) * 2);
						int nLineLength = 0;
						if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
							m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
						a = (float)dA;
						b = (float)dB;
					}
					else
						m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

					if ((nWndDir & 0x08) == 0x08)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
					}
#if LineDraw
					if (a != 0)
					{
						if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
						{
							if ((nWndDir & 0x04) == 0x04)
							{
								for (int c = vLine[0].x; c < ExceptArea.cols; c++)
								{
									int nStdY = (int)std::roundf(c*a + b);
									cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
									cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
								}
							}
							else if ((nWndDir & 0x08) == 0x08)
							{
								for (int c = 0; c < vLine[0].x; c++)
								{
									int nStdY = (int)std::roundf(c*a + b);
									cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
									cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
								}
							}
						}
						for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
						{
							int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
						if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
						{
							for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
					}
#else
					std::vector<cv::Point> vDrawPo;
					for (int c = 0; c < vBot.size(); c++)
					{
						if (vLine[0].x > vBot[c].x)
							continue;
						else if (vLine[vLine.size() - 1].x < vBot[c].x)
							break;
						int nStdY = (int)std::roundf(vBot[c].x*a + b);
						if (std::abs(nStdY - vBot[c].y) > 3)
						{
							vDrawPo.push_back(cv::Point(vBot[c].x, nStdY));
						}
						else
						{
							vDrawPo.push_back(cv::Point(vBot[c].x, vBot[c].y));
						}
					}
					std::vector<cv::Point> vBlank;
					for (int l = 1; l < vDrawPo.size(); l++)
					{
						if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
						{
							float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
							float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
							for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
							{
								vBlank.push_back(cv::Point(x, fTmpA*x + fTmpB));
							}
						}
					}
					if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
					{
						if ((nWndDir & 0x04) == 0x04)
						{
							for (int c = vBot[vBot.size() - 1].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, vBot[c].y), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
						else if ((nWndDir & 0x08) == 0x08)
						{
							for (int c = 0; c < vBot[0].x; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
					}
					//for (int c = 0; c < vBot.size(); c++)
					//{
					//	//int nStdY = (int)std::roundf(c*a + b);
					//	cv::line(ExceptArea, cv::Point(vBot[c].x, 0), cv::Point(vBot[c].x, vBot[c].y), cv::Scalar(255));
					//	cv::line(ExceptArea, cv::Point(vBot[c].x, vBot[c].y + 1), cv::Point(vBot[c].x, ExceptArea.rows), cv::Scalar(0));
					//}

					if (pAlgoBlob->m_nThickDir != 2)
					{
						for (int c = 0; c < vDrawPo.size(); c++)
					{
						//int nStdY = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, 0), cv::Point(vDrawPo[c].x, vDrawPo[c].y), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(vDrawPo[c].x, vDrawPo[c].y + 1), cv::Point(vDrawPo[c].x, ExceptArea.rows), cv::Scalar(0));
						}
						for (int b = 0; b < vBlank.size(); b++)
						{
							cv::line(ExceptArea, cv::Point(vBlank[b].x, 0), cv::Point(vBlank[b].x, vBlank[b].y), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(vBlank[b].x, vBlank[b].y + 1), cv::Point(vBlank[b].x, ExceptArea.rows), cv::Scalar(0));
						}
						if (/*(pAlgoBlob->m_nThickDir != 2) &&*/ fWndCenterX + fWndWidth <= 0)
						{
							for (int c = vBot[vBot.size() - 1].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
					}
#endif
					if ((nWndDir & 0x04) == 0x04)
					{
						poConnerLt.y = poConnerRb.y;
						int nTop = (int)std::roundf(vLine[0].x*a + b);
						poConnerRb.x = vLine[0].x;
						poConnerRb.y = nTop > 0 ? nTop : 0;
					}
					else if ((nWndDir & 0x08) == 0x08)
					{
						poConnerLt.y = poConnerRb.y;
						int nTop = (int)std::roundf(vLine[vLine.size() - 1].x*a + b);
						poConnerLt.x = vLine[vLine.size() - 1].x;
						poConnerRb.y = nTop > 0 ? nTop : 0;
					}
				}
			}
			ContoursMaxImg &= ExceptArea;
		}
		else //subtract
		{
			if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
			{
				std::vector<cv::Point> vLeft;
				std::vector<int> vnLeftCnt;
				vnLeftCnt.resize(ContoursMaxImg.cols);
				//좌측
				//nWndDir += 0x04;
				for (int r = 0; r < ContoursMaxImg.rows; r++)
				{
					UCHAR* uPtr = ContoursMaxImg.ptr(r);
					int nmxC = ContoursMaxImg.cols;
					int nmnC = 0;
					for (int c = ContoursMaxImg.cols - 1; c > 0; c--)
					{
						if (nmxC == ContoursMaxImg.cols && uPtr[c] > 0)
						{
							nmxC = c;
						}
						else if (nmxC != ContoursMaxImg.cols && uPtr[c] == 0)
						{
							nmnC = c;
							break;
						}
					}
					if (nmxC == ContoursMaxImg.cols)
						nmxC = ContoursMaxImg.cols - 1;
					UCHAR* uPtrex = ExceptAreaImg.ptr(r);
					for (int c = nmxC; c >= nmnC; c--)
					{
						if (uPtrex[c] > 0)
						{
							vLeft.push_back(cv::Point(r, c));
							vnLeftCnt[c]++;
							nmnC = c;
							break;
						}
					}
					if (nmxC != ContoursMaxImg.cols - 1)
					{
						cv::line(ExceptArea, cv::Point(nmnC, r), cv::Point(ExceptArea.cols - 1, r), cv::Scalar(0));
					}
				}
				int nMinVecCnt = rcBlob.Height() / 3;
				vLeft.clear();
				if (vLeft.size() > nMinVecCnt)
				{
					int nMxCnt = 0;
					int nMxIdx = 0;
					for (int c = 0; c < vnLeftCnt.size() - 1; c++)//끝에 완전히 붙는경우 제외
					{
						if (nMxCnt < vnLeftCnt[c])
						{
							nMxCnt = vnLeftCnt[c];
							nMxIdx = c;
						}
					}
					cv::Rect blackRect(nMxIdx, 0, ExceptArea.cols - nMxIdx, 0);
					cv::Rect whiteRect(0, 0, nMxIdx, ExceptArea.rows);
					for (int c = 0; c < vLeft.size(); c++)
					{
						if (std::abs(vLeft[c].y - nMxIdx) < 5)
						{
							blackRect.y = vLeft[c].x;
							break;
						}
					}
					for (int c = vLeft.size() - 1; c >= blackRect.x; c--)
					{
						if (std::abs(vLeft[c].y - nMxIdx) < 5)
						{
							blackRect.height = vLeft[c].x - blackRect.y + 1;
							break;
						}
					}
					std::vector<cv::Point> vLine;
					int nStLineX = vLeft[vLeft.size() - 1].y;
					int nEndLineX = 0;
					for (int c = 0; c < vLeft.size(); c++)
					{
						if (std::abs(vLeft[c].y - nMxIdx) < 5)
						{
							vLine.push_back(vLeft[c]);
							if (vLeft[c].y == nMxIdx)
							{
								nEndLineX = vLeft[c].x;
								if (nStLineX > vLeft[c].x)
									nStLineX = vLeft[c].x;
							}
						}
					}
					if (vLine.size() == 0)
						vLine = vLeft;
					if (nEndLineX == 0)
						nEndLineX = vLine[vLine.size() - 1].x;
					if (nStLineX == vLeft[vLeft.size() - 1].y)
						nStLineX = vLine[0].x;
					for (int c = 0; c < vLine.size(); c++)
					{
						if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
						{
							vLine.erase(vLine.begin() + c);
							c--;
						}
					}

					float a, b;
					if (!bIsBot && !bIsTop)
					{
						double dA, dB;
						double dAngle = 0;
						int nMeasureDirection = Right_Line;
						bool bIsHorizon = false;
						POINTF poDrawLine[2];
						memset(poDrawLine, 0, sizeof(POINTF) * 2);
						int nLineLength = 0;
						if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
							m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
						a = (float)dA;
						b = (float)dB;
					}
					else
						m_pProcMilAlgo->OLS_1D(vLine, &a, &b);
					b = vLine[0].y - (vLine[0].x*a);

					std::vector<cv::Point> vDrawPo;
					for (int c = 0; c < vLeft.size(); c++)
					{
						if (vLine[0].x > vLeft[c].x)
							continue;
						else if (vLine[vLine.size() - 1].x < vLeft[c].x)
							break;
						int nStdX = (int)std::roundf(vLeft[c].x*a + b);
						if (std::abs(nStdX - vLeft[c].y) > 3)
						{
							vDrawPo.push_back(cv::Point(vLeft[c].x, nStdX));
						}
						else
						{
							vDrawPo.push_back(cv::Point(vLeft[c].x, vLeft[c].y));
						}
					}
					std::vector<cv::Point> vBlank;
					for (int l = 1; l < vDrawPo.size(); l++)
					{
						if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
						{
							float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
							float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
							for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
							{
								vBlank.push_back(cv::Point(x, fTmpA*x + fTmpB));
							}
						}
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
					}
#if LineDraw
					if (a != 0)
					{
						if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
						{
							for (int c = 0; c < vLine[0].x; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
							}
						}
						for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
						if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
						{
							for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.rows; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
							}
						}
					}
#else
					if (bIsBot)
					{
						for (int c = 0; c < vDrawPo[0].x; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
					}
					//for (int c = 0; c < vLeft.size(); c++)
					//{
					//	//int nStdX = (int)std::roundf(c*a + b);
					//	cv::line(ExceptArea, cv::Point(0, vLeft[c].x), cv::Point(vLeft[c].y, vLeft[c].x), cv::Scalar(255));
					//	cv::line(ExceptArea, cv::Point(vLeft[c].y + 1, vLeft[c].x), cv::Point(ExceptArea.cols, vLeft[c].x), cv::Scalar(0));
					//}
					for (int c = 0; c < vDrawPo.size(); c++)
					{
						cv::line(ExceptArea, cv::Point(0, vDrawPo[c].x), cv::Point(vDrawPo[c].y, vDrawPo[c].x), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(vDrawPo[c].y + 1, vDrawPo[c].x), cv::Point(ExceptArea.cols, vDrawPo[c].x), cv::Scalar(0));
					}
					for (int b = 0; b < vBlank.size(); b++)
					{
						//int nStdX = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(0, vBlank[b].x), cv::Point(vBlank[b].y, vBlank[b].x), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(vBlank[b].y + 1, vBlank[b].x), cv::Point(ExceptArea.cols, vBlank[b].x), cv::Scalar(0));
					}
					if (bIsTop)
					{
						for (int c = vDrawPo[vDrawPo.size() - 1].x; c < ExceptArea.rows; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
					}
#endif
					int nLeft = (int)std::roundf(vLine[0].x*a + b);
					poConnerLt.x = nLeft > 0 ? nLeft : 0;
					poConnerLt.y = vLine[0].x;
					nLeft = (int)std::roundf(vLine[vLine.size() - 1].x*a + b);
					poConnerRb.x = nLeft > 0 ? nLeft : 0;
					poConnerRb.y = vLine[vLine.size() - 1].x;
				}
			}
			else if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
			{
				//우측
				std::vector<cv::Point> vRight;
				std::vector<int> vnRightCnt;
				vnRightCnt.resize(ContoursMaxImg.cols);
				//nWndDir += 0x08;
				for (int r = 0; r < ContoursMaxImg.rows; r++)
				{
					UCHAR* uPtr = ContoursMaxImg.ptr(r);
					int nmxC = ContoursMaxImg.cols - 1;
					int nmnC = 0;
					for (int c = 0; c < ContoursMaxImg.cols; c++)
					{
						if (nmnC == 0 && uPtr[c] > 0)
						{
							nmnC = c;
						}
						else if (nmnC != 0 && uPtr[c] == 0)
						{
							nmxC = c;
							break;
						}
					}
					UCHAR* uPtrex = ExceptAreaImg.ptr(r);
					for (int c = nmnC; c < nmxC; c++)
					{
						if (uPtrex[c] > 0)
						{
							vRight.push_back(cv::Point(r, c));
							vnRightCnt[c]++;
							nmxC = c;
							break;
						}
					}
					if (nmxC != ContoursMaxImg.cols - 1)
					{
						cv::line(ExceptArea, cv::Point(0, r), cv::Point(nmxC, r), cv::Scalar(0));
					}
				}
				int nMinVecCnt = rcBlob.Height() / 3;
				vRight.clear();
				if (vRight.size() > nMinVecCnt)
				{
					int nMxCnt = 0;
					int nMxIdx = 0;
					for (int c = 1; c < vnRightCnt.size(); c++)//끝에 완전히 붙는경우 제외
					{
						if (nMxCnt < vnRightCnt[c])
						{
							nMxCnt = vnRightCnt[c];
							nMxIdx = c;
						}
					}
					cv::Rect whiteRect(nMxIdx, 0, ExceptArea.cols - nMxIdx, ExceptArea.rows);
					cv::Rect blackRect(0, 0, nMxIdx, ExceptArea.rows);
					for (int c = 0; c < vRight.size(); c++)
					{
						if (std::abs(vRight[c].y - nMxIdx) < 5)
						{
							blackRect.y = vRight[c].x;
							break;
						}
					}
					for (int c = vRight.size() - 1; c >= 0; c--)
					{
						if (std::abs(vRight[c].y - nMxIdx) < 5)
						{
							blackRect.height = vRight[c].x - blackRect.y + 1;
							break;
						}
					}
					std::vector<cv::Point> vLine;
					int nStLineX = vRight[vRight.size() - 1].y;
					int nEndLineX = 0;
					for (int c = 0; c < vRight.size(); c++)
					{
						if (std::abs(vRight[c].y - nMxIdx) < 5)
						{
							vLine.push_back(vRight[c]);
							if (vRight[c].y == nMxIdx)
							{
								nEndLineX = vRight[c].x;
								if (nStLineX > vRight[c].x)
									nStLineX = vRight[c].x;
							}
						}
					}
					if (vLine.size() == 0)
						vLine = vRight;
					if (nEndLineX == 0)
						nEndLineX = vLine[vLine.size() - 1].x;
					if (nStLineX == vRight[vRight.size() - 1].y)
						nStLineX = vLine[0].x;
					for (int c = 0; c < vLine.size(); c++)
					{
						if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
						{
							vLine.erase(vLine.begin() + c);
							c--;
						}
					}
					float a, b;
					if (!bIsBot && !bIsTop)
					{
						double dA, dB;
						double dAngle = 0;
						int nMeasureDirection = Left_Line;
						bool bIsHorizon = false;
						POINTF poDrawLine[2];
						memset(poDrawLine, 0, sizeof(POINTF) * 2);
						int nLineLength = 0;
						if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
							m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
						a = (float)dA;
						b = (float)dB;
					}
					else
						m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

					b = vLine[0].y - (vLine[0].x*a);

#if LineDraw
					if (a != 0)
					{
						if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
						{
							b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
						}
						if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
						{
							for (int c = 0; c < vLine[0].x; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
							}
						}
						for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
						}
						if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
						{
							for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.rows; c++)
							{
								int nStdX = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
							}
						}
					}
#else

					std::vector<cv::Point> vDrawPo;
					for (int c = 0; c < vRight.size(); c++)
					{
						if (vLine[0].x > vRight[c].x)
							continue;
						else if (vLine[vLine.size() - 1].x < vRight[c].x)
							break;
						int nStdX = (int)std::roundf(vRight[c].x*a + b);
						if (std::abs(nStdX - vRight[c].y) > 3)
						{
							vDrawPo.push_back(cv::Point(vRight[c].x, nStdX));
						}
						else
						{
							vDrawPo.push_back(cv::Point(vRight[c].x, vRight[c].y));
						}
					}
					std::vector<cv::Point> vBlank;
					for (int l = 1; l < vDrawPo.size(); l++)
					{
						if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
						{
							float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
							float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
							for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
							{
								vBlank.push_back(cv::Point(x, fTmpA*x + fTmpB));
							}
						}
					}

					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
					}
					if (bIsBot)
					{
						for (int c = 0; c < vDrawPo[0].x; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
						}
					}
					//for (int c = 0; c < vRight.size(); c++)
					//{
					//	//int nStdX = (int)std::roundf(c*a + b);
					//	cv::line(ExceptArea, cv::Point(0, vRight[c].x), cv::Point(vRight[c].y - 1, vRight[c].x), cv::Scalar(0));
					//	cv::line(ExceptArea, cv::Point(vRight[c].y, vRight[c].x), cv::Point(ExceptArea.cols, vRight[c].x), cv::Scalar(255));
					//}
					for (int c = 0; c < vDrawPo.size(); c++)
					{
						//int nStdX = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(0, vDrawPo[c].x), cv::Point(vDrawPo[c].y - 1, vDrawPo[c].x), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(vDrawPo[c].y, vDrawPo[c].x), cv::Point(ExceptArea.cols, vDrawPo[c].x), cv::Scalar(255));
					}
					for (int b = 0; b < vBlank.size(); b++)
					{
						cv::line(ExceptArea, cv::Point(0, vBlank[b].x), cv::Point(vBlank[b].y - 1, vBlank[b].x), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(vBlank[b].y, vBlank[b].x), cv::Point(ExceptArea.cols, vBlank[b].x), cv::Scalar(255));
					}
					if (bIsTop)
					{
						for (int c = vDrawPo[vDrawPo.size() - 1].x; c < ExceptArea.rows; c++)
						{
							int nStdX = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
						}
					}
#endif
					int nLeft = (int)std::roundf(vLine[0].x*a + b);
					poConnerLt.x = nLeft > 0 ? nLeft : 0;
					poConnerLt.y = vLine[0].x;
					nLeft = (int)std::roundf(vLine[vLine.size() - 1].x*a + b);
					poConnerRb.x = nLeft > 0 ? nLeft : 0;
					poConnerRb.y = vLine[vLine.size() - 1].x;
				}
			}
			if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
			{
				//상측
				std::vector<cv::Point> vTop;
				std::vector<int> vnTopCnt;
				vnTopCnt.resize(ContoursMaxImg.rows);
				//nWndDir += 0x01;
				int nCntSt = poConnerLt.x;
				int nCntEd = ContoursMaxImg.cols;
				if ((nWndDir & 0x08) == 0x08)
				{
					nCntSt = 0;
					nCntEd = poConnerLt.x;
				}
				for (int c = nCntSt; c < nCntEd; c++)
				{
					int nmnR = 0;
					int nmxR = ContoursMaxImg.rows - 1;
					for (int r = ContoursMaxImg.rows - 1; r > 0; r--)
					{
						UCHAR* uPtr = ContoursMaxImg.ptr(r);
						if (nmxR == ContoursMaxImg.rows - 1 && uPtr[c] > 0)
						{
							nmxR = r;
						}
						else if (nmxR != ContoursMaxImg.rows - 1 && uPtr[c] == 0)
						{
							nmnR = r;
							break;
						}
					}
					for (int r = nmxR; r >= nmnR; r--)
					{
						UCHAR* uPtrex = ExceptAreaImg.ptr(r);
						if (uPtrex[c] > 0)
						{
							vTop.push_back(cv::Point(c, r));
							vnTopCnt[r]++;
							nmnR = r;
							break;
						}
					}
					if (nmnR != 0)
					{
						cv::line(ExceptArea, cv::Point(c, nmnR), cv::Point(c, ExceptArea.rows - 1), cv::Scalar(0));
					}
				}
				int nMinVecCnt = rcBlob.Width() / 3;
				vTop.clear();
				if (vTop.size() > nMinVecCnt)
				{
					int nMxCnt = 0;
					int nMxIdx = 0;
					for (int r = 0; r < vnTopCnt.size() - 1; r++)//끝에 완전히 붙는경우 제외
					{
						if (nMxCnt < vnTopCnt[r])
						{
							nMxCnt = vnTopCnt[r];
							nMxIdx = r;
						}
					}
					cv::Rect whiteRect(0, 0, ExceptArea.cols, nMxIdx);
					cv::Rect blackRect(0, nMxIdx, ExceptArea.cols, ExceptArea.rows);
					for (int r = 0; r < vTop.size(); r++)
					{
						if (std::abs(vTop[r].y - nMxIdx) < 5)
						{
							blackRect.x = vTop[r].x;
							break;
						}
					}
					for (int r = vTop.size() - 1; r >= blackRect.y; r--)
					{
						if (std::abs(vTop[r].y - nMxIdx) < 5)
						{
							blackRect.width = vTop[r].x - blackRect.x + 1;
							break;
						}
					}
					std::vector<cv::Point> vLine;
					int nStLineX = vTop[vTop.size() - 1].y;
					int nEndLineX = 0;
					for (int c = 0; c < vTop.size(); c++)
					{
						if (std::abs(vTop[c].y - nMxIdx) < 5)
						{
							vLine.push_back(vTop[c]);
							if (vTop[c].y == nMxIdx)
							{
								nEndLineX = vTop[c].x;
								if (nStLineX > vTop[c].x)
									nStLineX = vTop[c].x;
							}
						}
					}
					if (vLine.size() == 0)
						vLine = vTop;
					if (nEndLineX == 0)
						nEndLineX = vLine[vLine.size() - 1].x;
					if (nStLineX == vTop[vTop.size() - 1].y)
						nStLineX = vLine[0].x;
					for (int c = 0; c < vLine.size(); c++)
					{
						if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
						{
							vLine.erase(vLine.begin() + c);
							c--;
						}
					}
					float a, b;
					if (pAlgoBlob->m_nThickDir == 2)
					{
						double dA, dB;
						double dAngle = 0;
						int nMeasureDirection = Right_Line;
						bool bIsHorizon = true;
						POINTF poDrawLine[2];
						memset(poDrawLine, 0, sizeof(POINTF) * 2);
						int nLineLength = 0;
						cv::Mat ClipImg = ExceptAreaImg(cv::Rect(vLine[0].x, 0, vLine[vLine.size() - 1].x - vLine[0].x, ExceptAreaImg.rows)).clone();
						if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
							m_pCPInsp_Algo->InspectionLine(ClipImg.cols, ClipImg.rows, ClipImg.ptr(), bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
						a = (float)dA;
						b = (float)dB;
					}
					else
						m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

					b = vLine[0].y - (vLine[0].x*a);

					if ((nWndDir & 0x08) == 0x08)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
					}
#if LineDraw
					if (a != 0)
					{
						if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
						{
							if ((nWndDir & 0x04) == 0x04)
							{
								for (int c = vLine[0].x; c < ExceptArea.cols; c++)
								{
									int nStdY = (int)std::roundf(c*a + b);
									cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
									cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
								}
							}
							else if ((nWndDir & 0x08) == 0x08)
							{
								for (int c = 0; c < vLine[0].x; c++)
								{
									int nStdY = (int)std::roundf(c*a + b);
									cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
									cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
								}
							}
						}
						for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
						{
							int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
						if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
						{
							for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
					}
#else
					std::vector<cv::Point> vDrawPo;
					for (int c = 0; c < vTop.size(); c++)
					{
						if (vLine[0].x > vTop[c].x)
							continue;
						else if (vLine[vLine.size() - 1].x < vTop[c].x)
							break;
						int nStdY = (int)std::roundf(vTop[c].x*a + b);
						if (std::abs(nStdY - vTop[c].y) > 3)
						{
							vDrawPo.push_back(cv::Point(vTop[c].x, nStdY));
						}
						else
						{
							vDrawPo.push_back(cv::Point(vTop[c].x, vTop[c].y));
						}
					}
					std::vector<cv::Point> vBlank;
					for (int l = 1; l < vDrawPo.size(); l++)
					{
						if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
						{
							float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
							float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
							for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
							{
								vBlank.push_back(cv::Point(x, fTmpA*x + fTmpB));
							}
						}
					}

					if ((nWndDir & 0x04) != 0x04)
					{
						for (int c = 0; c < vDrawPo[0].x; c++)
						{
							int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
					}
					//for (int c = 0; c < vTop.size(); c++)
					//{
					//	//int nStdY = (int)std::roundf(c*a + b);
					//	cv::line(ExceptArea, cv::Point(vTop[c].x, 0), cv::Point(vTop[c].x, vTop[c].y), cv::Scalar(255));
					//	cv::line(ExceptArea, cv::Point(vTop[c].x, vTop[c].y + 1), cv::Point(vTop[c].x, ExceptArea.rows), cv::Scalar(0));
					//}
					for (int c = 0; c < vDrawPo.size(); c++)
					{
						//int nStdY = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, 0), cv::Point(vDrawPo[c].x, vDrawPo[c].y), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, vDrawPo[c].y + 1), cv::Point(vDrawPo[c].x, ExceptArea.rows), cv::Scalar(0));
					}
					for (int b = 0; b < vBlank.size(); b++)
					{
						cv::line(ExceptArea, cv::Point(vBlank[b].x, 0), cv::Point(vBlank[b].x, vBlank[b].y), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(vBlank[b].x, vBlank[b].y + 1), cv::Point(vBlank[b].x, ExceptArea.rows), cv::Scalar(0));
					}
					if ((nWndDir & 0x08) != 0x08)
					{
						for (int c = vDrawPo[vDrawPo.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
					}
#endif
					if ((nWndDir & 0x04) == 0x04)
					{
						poConnerRb.y = poConnerLt.y;
						int nTop = (int)std::roundf(vLine[0].x*a + b);
						poConnerRb.x = vLine[0].x;
						poConnerLt.y = nTop > 0 ? nTop : 0;
					}
					else if ((nWndDir & 0x08) == 0x08)
					{
						poConnerRb.x = poConnerLt.x;
						poConnerRb.y = poConnerLt.y;
						int nTop = (int)std::roundf(vLine[vLine.size() - 1].x*a + b);
						poConnerLt.x = vLine[vLine.size() - 1].x;
						poConnerLt.y = nTop > 0 ? nTop : 0;
					}
				}
			}
			else if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
			{
				//하측
				std::vector<cv::Point> vBot;
				std::vector<int> vnBotCnt;
				vnBotCnt.resize(ContoursMaxImg.rows);
				//nWndDir += 0x02;
				int nCntSt = poConnerLt.x;
				int nCntEd = ContoursMaxImg.cols;
				if ((nWndDir & 0x08) == 0x08)
				{
					nCntSt = 0;
					nCntEd = poConnerLt.x;
				}
				for (int c = nCntSt; c < nCntEd; c++)
				{
					int nmnR = 0;
					int nmxR = ContoursMaxImg.rows - 1;
					for (int r = 0; r < ContoursMaxImg.rows; r++)
					{
						UCHAR* uPtr = ContoursMaxImg.ptr(r);
						if (nmnR == 0 && uPtr[c] > 0)
						{
							nmnR = r;
						}
						else if (nmnR != 0 && uPtr[c] == 0)
						{
							nmxR = r;
							break;
						}
					}
					for (int r = nmnR; r < nmxR; r++)
					{
						UCHAR* uPtrex = ExceptAreaImg.ptr(r);
						if (uPtrex[c] > 0)
						{
							vBot.push_back(cv::Point(c, r));
							vnBotCnt[r]++;
							nmxR = r;
							break;
						}
					}
					if (nmxR != ContoursMaxImg.rows - 1)
					{
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nmxR), cv::Scalar(0));
					}
				}
				int nMinVecCnt = rcBlob.Width() / 3;
				vBot.clear();
				if (vBot.size() > nMinVecCnt)
				{
					int nMxCnt = 0;
					int nMxIdx = 0;
					for (int r = 1; r < vnBotCnt.size(); r++)//끝에 완전히 붙는경우 제외
					{
						if (nMxCnt < vnBotCnt[r])
						{
							nMxCnt = vnBotCnt[r];
							nMxIdx = r;
						}
					}
					cv::Rect blackRect(0, 0, ExceptArea.cols, nMxIdx);
					cv::Rect whiteRect(0, nMxIdx, ExceptArea.cols, ExceptArea.rows);
					for (int r = 0; r < vBot.size(); r++)
					{
						if (std::abs(vBot[r].y - nMxIdx) < 5)
						{
							blackRect.x = vBot[r].x;
							break;
						}
					}
					for (int r = vBot.size() - 1; r >= 0; r--)
					{
						if (std::abs(vBot[r].y - nMxIdx) < 5)
						{
							blackRect.width = vBot[r].x - blackRect.x + 1;
							break;
						}
					}
					std::vector<cv::Point> vLine;
					int nStLineX = vBot[vBot.size() - 1].y;
					int nEndLineX = 0;
					for (int c = 0; c < vBot.size(); c++)
					{
						if (std::abs(vBot[c].y - nMxIdx) < 5)
						{
							vLine.push_back(vBot[c]);
							if (vBot[c].y == nMxIdx)
							{
								nEndLineX = vBot[c].x;
								if (nStLineX > vBot[c].x)
									nStLineX = vBot[c].x;
							}
						}
					}
					if (vLine.size() == 0)
						vLine = vBot;
					if (nEndLineX == 0)
						nEndLineX = vLine[vLine.size() - 1].x;
					if (nStLineX == vBot[vBot.size() - 1].y)
						nStLineX = vLine[0].x;
					for (int c = 0; c < vLine.size(); c++)
					{
						if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
						{
							vLine.erase(vLine.begin() + c);
							c--;
						}
					}
					float a, b;
					if (pAlgoBlob->m_nThickDir == 2)
					{
						double dA, dB;
						double dAngle = 0;
						int nMeasureDirection = Left_Line;
						bool bIsHorizon = true;
						POINTF poDrawLine[2];
						memset(poDrawLine, 0, sizeof(POINTF) * 2);
						int nLineLength = 0;
						cv::Mat ClipImg = ExceptAreaImg(cv::Rect(vLine[0].x, 0, vLine[vLine.size() - 1].x - vLine[0].x, ExceptAreaImg.rows)).clone();
						if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
							m_pCPInsp_Algo->InspectionLine(ClipImg.cols, ClipImg.rows, ClipImg.ptr(), bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
						a = (float)dA;
						b = (float)dB;
					}
					else
						m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

					b = vLine[0].y - (vLine[0].x*a);

					if ((nWndDir & 0x08) == 0x08)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x*a);
					}
#if LineDraw
					if (a != 0)
					{
						if ((pAlgoBlob->m_nThickDir != 2) && bIsBot)
						{
							if ((nWndDir & 0x04) == 0x04)
							{
								for (int c = vLine[0].x; c < ExceptArea.cols; c++)
								{
									int nStdY = (int)std::roundf(c*a + b);
									cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
									cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
								}
							}
							if ((nWndDir & 0x08) == 0x08)
							{
								for (int c = 0; c < vLine[0].x; c++)
								{
									int nStdY = (int)std::roundf(c*a + b);
									cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
									cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
								}
							}
						}
						for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
						{
							int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
						}
						if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
						{
							for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c*a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
							}
						}
					}
#else
					std::vector<cv::Point> vDrawPo;
					for (int c = 0; c < vBot.size(); c++)
					{
						if (vLine[0].x > vBot[c].x)
							continue;
						else if (vLine[vLine.size() - 1].x < vBot[c].x)
							break;
						int nStdY = (int)std::roundf(vBot[c].x*a + b);
						if (std::abs(nStdY - vBot[c].y) > 3)
						{
							vDrawPo.push_back(cv::Point(vBot[c].x, nStdY));
						}
						else
						{
							vDrawPo.push_back(cv::Point(vBot[c].x, vBot[c].y));
						}
					}
					std::vector<cv::Point> vBlank;
					for (int l = 1; l < vDrawPo.size(); l++)
					{
						if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
						{
							float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
							float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
							for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
							{
								vBlank.push_back(cv::Point(x, fTmpA*x + fTmpB));
							}
						}
					}

					if ((nWndDir & 0x04) != 0x04)
					{
						for (int c = 0; c < vDrawPo[0].x; c++)
						{
							int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
						}
					}
					//for (int c = 0; c < vBot.size(); c++)
					//{
					//	//int nStdY = (int)std::roundf(c*a + b);
					//	cv::line(ExceptArea, cv::Point(vBot[c].x, 0), cv::Point(vBot[c].x, vBot[c].y - 1), cv::Scalar(0));
					//	cv::line(ExceptArea, cv::Point(vBot[c].x, vBot[c].y), cv::Point(vBot[c].x, ExceptArea.rows), cv::Scalar(255));
					//}
					for (int c = 0; c < vDrawPo.size(); c++)
					{
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, 0), cv::Point(vDrawPo[c].x, vDrawPo[c].y - 1), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, vDrawPo[c].y), cv::Point(vDrawPo[c].x, ExceptArea.rows), cv::Scalar(255));
					}
					for (int b = 0; b < vBlank.size(); b++)
					{
						//int nStdY = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(vBlank[b].x, 0), cv::Point(vBlank[b].x, vBlank[b].y - 1), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(vBlank[b].x, vBlank[b].y), cv::Point(vBlank[b].x, ExceptArea.rows), cv::Scalar(255));
					}
					if ((nWndDir & 0x08) != 0x08)
					{
						for (int c = vDrawPo[vDrawPo.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c*a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
						}
					}
#endif
					if ((nWndDir & 0x04) == 0x04)
					{
						poConnerLt.y = poConnerRb.y;
						int nTop = (int)std::roundf(vLine[0].x*a + b);
						poConnerRb.x = vLine[0].x;
						poConnerRb.y = nTop > 0 ? nTop : 0;
					}
					else if ((nWndDir & 0x08) == 0x08)
					{
						poConnerLt.y = poConnerRb.y;
						int nTop = (int)std::roundf(vLine[vLine.size() - 1].x*a + b);
						poConnerLt.x = vLine[vLine.size() - 1].x;
						poConnerRb.y = nTop > 0 ? nTop : 0;
					}
				}
			}
			ContoursMaxImg -= ExceptArea;
		}
		//g_pMManager->pem_delete(ThiningBuf, true);
		//cv::imwrite("D:\\ExceptAreaTect.bmp", ContoursMaxImg(cv::Rect(poConnerLt.x, poConnerLt.y, poConnerRb.x - poConnerLt.x - 1, poConnerRb.y - poConnerLt.y - 1)));
		//이미 선택된 블랍이므로 all select //shkim 
		int nFilter = 0;
		if (pAlgoBlob->m_bFilterIsUse)
			nFilter = pAlgoBlob->m_nFilterStepNarrow;
		//if(pAlgoBlob->m_bFillHole)
		//작은 픽셀 삭제
		bool bMultiMaxBlob = !pAlgoBlob->m_bUseBlobNG && !pAlgoBlob->m_bInspCoil && pAlgoBlob->m_byNGCnt > 0;
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucArrBlobDst, ucArrBlobDst, nImgWidth, nImgHeight, nMinBlobArea, FALSE, pAlgoBlob->m_bFillHole, nFilter, pAlgoBlob->m_nTypeSelectBlob, -1, -1, -1, dAreaPix, dAreaWPix, dAreaHPix, -1, bMultiMaxBlob, pAlgoBlob->m_byNGCnt);

		cv::Mat teachImg = ExceptAreaImg.clone();
		std::vector<std::vector<cv::Point>> teachTmp;
		std::vector<cv::Vec4i> hierarchyTmp;
		cv::findContours(teachImg, teachTmp, hierarchyTmp, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
		POINTF PointFail;
		PointFail.x = PointFail.y = -1;
		RstPoTop = RstPoTop = RstPoTop = RstPoTop = PointFail;
		if (bTeach)
		{
			std::vector<cv::Point> Teachcontours;
			std::vector<int> vXCnt;
			std::vector<int> vYCnt;
			vXCnt.resize(ContoursMaxImg.cols);
			vYCnt.resize(ContoursMaxImg.rows);
			std::vector<cv::Point> vtmptach;
			RstPoTop.x = RstPoBot.x = RstPoLeft.x = RstPoRight.x = -1;
			RstPoTop.y = RstPoBot.y = RstPoLeft.y = RstPoRight.y = -1;
			for (int blobIdx = 0; blobIdx < teachTmp.size(); blobIdx++)
			{
				//zero index
				if (teachTmp[blobIdx].size() > 2)
				{
					int bPoI = 1;
					//When two points are located at the edge of the image
					if (((teachTmp[blobIdx][bPoI - 1].x == 0 && teachTmp[blobIdx][bPoI].x == 0) || (teachTmp[blobIdx][bPoI - 1].x >= teachImg.cols - 1 && teachTmp[blobIdx][bPoI].x >= teachImg.cols - 1))
						|| ((teachTmp[blobIdx][bPoI - 1].y == 0 && teachTmp[blobIdx][bPoI].y == 0) || (teachTmp[blobIdx][bPoI - 1].y >= teachImg.rows - 1 && teachTmp[blobIdx][bPoI].y >= teachImg.rows - 1)))
					{
					}
					else
					{
						vtmptach.push_back(teachTmp[blobIdx][bPoI - 1]);
						vXCnt[teachTmp[blobIdx][bPoI - 1].x]++;
						vYCnt[teachTmp[blobIdx][bPoI - 1].y]++;
					}
				}
				for (int bPoI = 1; bPoI < teachTmp[blobIdx].size(); bPoI++)
				{
					//When two points are located at the edge of the image
					if (((teachTmp[blobIdx][bPoI - 1].x == 0 && teachTmp[blobIdx][bPoI].x == 0) || (teachTmp[blobIdx][bPoI - 1].x >= teachImg.cols - 1 && teachTmp[blobIdx][bPoI].x >= teachImg.cols - 1))
						|| ((teachTmp[blobIdx][bPoI - 1].y == 0 && teachTmp[blobIdx][bPoI].y == 0) || (teachTmp[blobIdx][bPoI - 1].y >= teachImg.rows - 1 && teachTmp[blobIdx][bPoI].y >= teachImg.rows - 1)))
					{
						continue;
					}
					if (teachTmp[blobIdx][bPoI - 1].x == teachTmp[blobIdx][bPoI].x)
					{
						int sty = std::min(teachTmp[blobIdx][bPoI - 1].y, teachTmp[blobIdx][bPoI].y);
						int edy = std::max(teachTmp[blobIdx][bPoI - 1].y, teachTmp[blobIdx][bPoI].y);
						for (int poY = sty; poY <= edy; poY++)
						{
							vtmptach.push_back(cv::Point(teachTmp[blobIdx][bPoI].x, poY));
							vXCnt[teachTmp[blobIdx][bPoI].x]++;
							vYCnt[poY]++;
						}
					}
					else if (teachTmp[blobIdx][bPoI - 1].y == teachTmp[blobIdx][bPoI].y)
					{
						int stx = std::min(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
						int edx = std::max(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
						for (int pox = stx; pox <= edx; pox++)
						{
							vtmptach.push_back(cv::Point(pox, teachTmp[blobIdx][bPoI].y));
							vXCnt[pox]++;
							vYCnt[teachTmp[blobIdx][bPoI].y]++;
						}
					}
					else
					{
						float fDiffx = teachTmp[blobIdx][bPoI].x - teachTmp[blobIdx][bPoI - 1].x;
						float fDiffy = teachTmp[blobIdx][bPoI].y - teachTmp[blobIdx][bPoI - 1].y;
						float fa = fDiffy / fDiffx;
						float fb = teachTmp[blobIdx][bPoI - 1].y - fa * teachTmp[blobIdx][bPoI - 1].x;
						int stx = std::min(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
						int edx = std::max(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
						for (int pox = stx; pox <= edx; pox++)
						{
							float poY = fa * pox + fb;
							vtmptach.push_back(cv::Point(pox, poY));
							vXCnt[pox]++;
							vYCnt[poY]++;
						}
					}
					if (vtmptach.size() > 0)
					{
						Teachcontours.insert(Teachcontours.end(), vtmptach.begin(), vtmptach.end());
					}
				}
			}
			int nMX(0), nMY(0), nMXCnt(0), nMYCnt(0);
			for (int n = 0; n < vXCnt.size(); n++)
			{
				if (vXCnt[n] > nMXCnt)
				{
					nMX = n;
					nMXCnt = vXCnt[n];
				}
			}
			for (int n = 0; n < vYCnt.size(); n++)
			{
				if (vYCnt[n] > nMYCnt)
				{
					nMY = n;
					nMYCnt = vYCnt[n];
				}
			}
			if ((nWndDir & 0x01) == 0x01)//top
			{
				int nCntBlack = 0;
				for (int n = ContoursMaxImg.rows - 1; n > 0; n--)
				{
					int nWPixel = 0;
					for (int ncolIdx = nMX - 5; ncolIdx < (nMX + 5); ncolIdx++)
					{
						if (ncolIdx < 0) continue;
						if (ContoursMaxImg.ptr(n)[ncolIdx] > 0) nWPixel++;
					}
					if (nWPixel > 0)
					{
						RstPoTop.y = n;
						nCntBlack = 0;
						if (RstPoBot.y == -1)
							RstPoBot.y = n;
					}
					else if (RstPoTop.y > -1)
						nCntBlack++;
					if (nCntBlack > 5)
						break;
				}
				RstPoTop.x = RstPoBot.x = nMX;
				if ((nWndDir & 0x04) == 0x04)//left
				{
					int nCntBlack = 0;
					for (int n = ContoursMaxImg.cols - 1; n > 0; n--)
					{
						int nWPixel = 0;
						for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
						{
							if (nrowIdx < 0) continue;
							if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
						}
						if (nWPixel > 0)
						{
							RstPoLeft.x = n;
							nCntBlack = 0;
							if (RstPoRight.x == -1)
								RstPoRight.x = n;
						}
						else if (RstPoLeft.x > -1)
							nCntBlack++;
						if (nCntBlack > 5)
							break;
					}
					RstPoRight.y = RstPoLeft.y = nMY;
					float fy = RstPoLeft.y - RstPoTop.y;
					float fx = RstPoLeft.x - RstPoTop.x;
					float fTermX = fx / 3;
					float fTermY = fy / 3;
					RstPoDiagLeft.x = RstPoTop.x + fTermX;
					RstPoDiagLeft.y = RstPoTop.y + fTermY;
					RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
					RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
					cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
					for (int r = 0; r < RstPoDiagLeft.y; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y - r);
						if ((RstPoDiagLeft.x - r) >= 0 && uPtr[((int)RstPoDiagLeft.x - r)] > 0)
						{
							pTmp.x = RstPoDiagLeft.x - r;
							pTmp.y = RstPoDiagLeft.y - r;
							break;
						}
					}
					RstPoDiagLeft.x = pTmp.x;
					RstPoDiagLeft.y = pTmp.y;

					pTmp.x = RstPoDiagRight.x;
					pTmp.y = RstPoDiagRight.y;
					for (int r = 0; r < RstPoDiagRight.y; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagRight.y - r);
						if ((RstPoDiagRight.x - r) >= 0 && uPtr[((int)RstPoDiagRight.x - r)] > 0)
						{
							pTmp.x = RstPoDiagRight.x - r;
							pTmp.y = RstPoDiagRight.y - r;
							break;
						}
					}
					RstPoDiagRight.x = pTmp.x;
					RstPoDiagRight.y = pTmp.y;
				}
				else if ((nWndDir & 0x08) == 0x08)//right
				{
					int nCntBlack = 0;
					for (int n = 0; n < ContoursMaxImg.cols; n++)
					{
						int nWPixel = 0;
						for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
						{
							if (nrowIdx < 0) continue;
							if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
						}
						if (nWPixel > 0)
						{
							RstPoRight.x = n;
							nCntBlack = 0;
							if (RstPoLeft.x == -1)
								RstPoLeft.x = n;
						}
						else if (RstPoRight.x > -1)
							nCntBlack++;
						if (nCntBlack > 5)
							break;
					}
					RstPoRight.y = RstPoLeft.y = nMY;
					float fy = RstPoTop.y - RstPoRight.y;
					float fx = RstPoTop.x - RstPoRight.x;
					float fTermX = fx / 3;
					float fTermY = fy / 3;
					RstPoDiagLeft.x = RstPoRight.x + fTermX;
					RstPoDiagLeft.y = RstPoRight.y + fTermY;
					RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
					RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
					cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
					for (int r = 0; r < RstPoDiagLeft.y; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y - r);
						if ((RstPoDiagLeft.x + r) < teachImg.cols && uPtr[((int)RstPoDiagLeft.x + r)] > 0)
						{
							pTmp.x = RstPoDiagLeft.x + r;
							pTmp.y = RstPoDiagLeft.y - r;
							break;
						}
					}
					RstPoDiagLeft.x = pTmp.x;
					RstPoDiagLeft.y = pTmp.y;

					pTmp.x = RstPoDiagRight.x;
					pTmp.y = RstPoDiagRight.y;
					for (int r = 0; r < RstPoDiagRight.y; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagRight.y - r);
						if ((RstPoDiagRight.x + r) < teachImg.cols && uPtr[((int)RstPoDiagRight.x + r)] > 0)
						{
							pTmp.x = RstPoDiagRight.x + r;
							pTmp.y = RstPoDiagRight.y - r;
							break;
						}
					}
					RstPoDiagRight.x = pTmp.x;
					RstPoDiagRight.y = pTmp.y;
				}
			}
			else if ((nWndDir & 0x02) == 0x02)//bot
			{
				int nCntBlack = 0;
				for (int n = 0; n < ContoursMaxImg.rows; n++)
				{
					int nWPixel = 0;
					for (int ncolIdx = nMX - 5; ncolIdx < (nMX + 5); ncolIdx++)
					{
						if (ncolIdx < 0) continue;
						if (ContoursMaxImg.ptr(n)[ncolIdx] > 0) nWPixel++;
					}
					if (nWPixel > 0)
					{
						RstPoBot.y = n;
						nCntBlack = 0;
						if (RstPoTop.y == -1)
							RstPoTop.y = n;
					}
					else if (RstPoBot.y > -1)
						nCntBlack++;
					if (nCntBlack > 5)
						break;
				}
				RstPoTop.x = RstPoBot.x = nMX;
				if ((nWndDir & 0x04) == 0x04)//left
				{
					int nCntBlack = 0;
					for (int n = ContoursMaxImg.cols - 1; n > 0; n--)
					{
						int nWPixel = 0;
						for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
						{
							if (nrowIdx < 0) continue;
							if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
						}
						if (nWPixel > 0)
						{
							RstPoLeft.x = n;
							nCntBlack = 0;
							if (RstPoRight.x == -1)
								RstPoRight.x = n;
						}
						else if (RstPoLeft.x > -1)
							nCntBlack++;
						if (nCntBlack > 5)
							break;
					}
					RstPoRight.y = RstPoLeft.y = nMY;
					float fy = RstPoLeft.y - RstPoBot.y;
					float fx = RstPoLeft.x - RstPoBot.x;
					float fTermX = fx / 3;
					float fTermY = fy / 3;
					RstPoDiagLeft.x = RstPoBot.x + fTermX;
					RstPoDiagLeft.y = RstPoBot.y + fTermY;
					RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
					RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
					cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
					int nEdy = teachImg.rows - RstPoDiagLeft.y;
					for (int r = 0; r < nEdy; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y + r);
						if ((RstPoDiagLeft.x - r) >= 0 && uPtr[((int)RstPoDiagLeft.x - r)] > 0)
						{
							pTmp.x = RstPoDiagLeft.x - r;
							pTmp.y = RstPoDiagLeft.y + r;
							break;
						}
					}
					RstPoDiagLeft.x = pTmp.x;
					RstPoDiagLeft.y = pTmp.y;

					pTmp.x = RstPoDiagRight.x;
					pTmp.y = RstPoDiagRight.y;
					nEdy = teachImg.rows - RstPoDiagRight.y;
					for (int r = 0; r < nEdy; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagRight.y + r);
						if ((RstPoDiagRight.x - r) >= 0 && uPtr[((int)RstPoDiagRight.x - r)] > 0)
						{
							pTmp.x = RstPoDiagRight.x - r;
							pTmp.y = RstPoDiagRight.y + r;
							break;
						}
					}
					RstPoDiagRight.x = pTmp.x;
					RstPoDiagRight.y = pTmp.y;
				}
				else if ((nWndDir & 0x08) == 0x08)//right
				{
					int nCntBlack = 0;
					for (int n = 0; n < ContoursMaxImg.cols; n++)
					{
						int nWPixel = 0;
						for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
						{
							if (nrowIdx < 0) continue;
							if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
						}
						if (nWPixel > 0)
						{
							RstPoRight.x = n;
							nCntBlack = 0;
							if (RstPoLeft.x == -1)
								RstPoLeft.x = n;
						}
						else if (RstPoRight.x > -1)
							nCntBlack++;
						if (nCntBlack > 5)
							break;
					}
					RstPoRight.y = RstPoLeft.y = nMY;
					float fy = RstPoBot.y - RstPoRight.y;
					float fx = RstPoBot.x - RstPoRight.x;
					float fTermX = fx / 3;
					float fTermY = fy / 3;
					RstPoDiagLeft.x = RstPoRight.x + fTermX;
					RstPoDiagLeft.y = RstPoRight.y + fTermY;
					RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
					RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
					cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
					int nEdy = teachImg.rows - RstPoDiagLeft.y;
					for (int r = 0; r < nEdy; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y + r);
						if ((RstPoDiagLeft.x + r) < teachImg.cols && uPtr[((int)RstPoDiagLeft.x + r)] > 0)
						{
							pTmp.x = RstPoDiagLeft.x + r;
							pTmp.y = RstPoDiagLeft.y + r;
							break;
						}
					}
					RstPoDiagLeft.x = pTmp.x;
					RstPoDiagLeft.y = pTmp.y;

					pTmp.x = RstPoDiagRight.x;
					pTmp.y = RstPoDiagRight.y;
					nEdy = teachImg.rows - RstPoDiagRight.y;
					for (int r = 0; r < nEdy; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagRight.y + r);
						if ((RstPoDiagRight.x + r) < teachImg.cols && uPtr[((int)RstPoDiagRight.x + r)] > 0)
						{
							pTmp.x = RstPoDiagRight.x + r;
							pTmp.y = RstPoDiagRight.y + r;
							break;
						}
					}
					RstPoDiagRight.x = pTmp.x;
					RstPoDiagRight.y = pTmp.y;
				}
			}
			if (RstPoLeft.x < RstPoDiagLeft.x)//right
			{
				if (RstPoLeft.y > RstPoDiagLeft.y)//bot
				{
					sRstAlgo->Po1 = RstPoLeft;
					sRstAlgo->Po2 = RstPoRight;
					sRstAlgo->Po3 = RstPoDiagLeft;
					sRstAlgo->Po4 = RstPoDiagRight;
					sRstAlgo->Po5 = RstPoBot;
					sRstAlgo->Po6 = RstPoTop;
				}
				else//top
				{
					sRstAlgo->Po1 = RstPoLeft;
					sRstAlgo->Po2 = RstPoRight;
					sRstAlgo->Po3 = RstPoDiagLeft;
					sRstAlgo->Po4 = RstPoDiagRight;
					sRstAlgo->Po5 = RstPoTop;
					sRstAlgo->Po6 = RstPoBot;
				}
			}
			else//left
			{
				if (RstPoLeft.y > RstPoDiagLeft.y)//bot
				{
					sRstAlgo->Po1 = RstPoRight;
					sRstAlgo->Po2 = RstPoLeft;
					sRstAlgo->Po3 = RstPoDiagRight;
					sRstAlgo->Po4 = RstPoDiagLeft;
					sRstAlgo->Po5 = RstPoBot;
					sRstAlgo->Po6 = RstPoTop;
				}
				else//top
				{
					sRstAlgo->Po1 = RstPoRight;
					sRstAlgo->Po2 = RstPoLeft;
					sRstAlgo->Po3 = RstPoDiagRight;
					sRstAlgo->Po4 = RstPoDiagLeft;
					sRstAlgo->Po5 = RstPoTop;
					sRstAlgo->Po6 = RstPoBot;
				}
			}
			pAlgoBlob->Po1 = sRstAlgo->Po1;
			pAlgoBlob->Po2 = sRstAlgo->Po2;
			pAlgoBlob->Po3 = sRstAlgo->Po3;
			pAlgoBlob->Po4 = sRstAlgo->Po4;
			pAlgoBlob->Po5 = sRstAlgo->Po5;
			pAlgoBlob->Po6 = sRstAlgo->Po6;
			pAlgoBlob->m_nPoCnt = 6;
#if _DEBUG
			cv::Mat TechingcontoursMat = ContoursMaxImg.clone();
			cv::cvtColor(TechingcontoursMat, TechingcontoursMat, cv::COLOR_GRAY2BGR);
			for (int contIdx = 0; contIdx < Teachcontours.size(); contIdx++)
			{
				cv::line(TechingcontoursMat, Teachcontours[contIdx], Teachcontours[contIdx], cv::Scalar(0, 0, 255));
			}
			cv::imwrite("D:\\TechingcontoursMat.bmp", TechingcontoursMat);
			TechingcontoursMat = ContoursMaxImg.clone();
			cv::cvtColor(TechingcontoursMat, TechingcontoursMat, cv::COLOR_GRAY2BGR);
			cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po1.x, pAlgoBlob->Po1.y), cv::Point(pAlgoBlob->Po2.x, pAlgoBlob->Po2.y), cv::Scalar(0, 0, 255));
			cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po3.x, pAlgoBlob->Po3.y), cv::Point(pAlgoBlob->Po4.x, pAlgoBlob->Po4.y), cv::Scalar(0, 0, 255));
			cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po5.x, pAlgoBlob->Po5.y), cv::Point(pAlgoBlob->Po6.x, pAlgoBlob->Po6.y), cv::Scalar(0, 0, 255));
			cv::imwrite("D:\\TechingcontoursMat2.bmp", TechingcontoursMat);
#endif
		}
	}
	else if (dArea > 5)
	{
		int nFilter = 0;
		if (pAlgoBlob->m_bFilterIsUse)
			nFilter = pAlgoBlob->m_nFilterStepNarrow;
		bool bMultiMaxBlob = !pAlgoBlob->m_bUseBlobNG && !pAlgoBlob->m_bInspCoil && pAlgoBlob->m_byNGCnt > 0;
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucArrBlobDst, ucArrBlobDst, nImgWidth, nImgHeight, nMinBlobArea, FALSE, pAlgoBlob->m_bFillHole, nFilter, pAlgoBlob->m_nTypeSelectBlob, -1, -1, -1, dAreaPix, dAreaWPix, dAreaHPix, -1, bMultiMaxBlob, pAlgoBlob->m_byNGCnt);

		POINTF PointFail;
		PointFail.x = PointFail.y = -1;
		RstPoTop = RstPoTop = RstPoTop = RstPoTop = PointFail;
		cv::Mat teachImg = ContoursMaxImg.clone();
		std::vector<std::vector<cv::Point>> teachTmp;
		std::vector<cv::Vec4i> hierarchyTmp;
		cv::findContours(teachImg, teachTmp, hierarchyTmp, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
		if (bTeach)
		{
			std::vector<cv::Point> Teachcontours;
			std::vector<int> vXCnt;
			std::vector<int> vYCnt;
			vXCnt.resize(ContoursMaxImg.cols);
			vYCnt.resize(ContoursMaxImg.rows);
			std::vector<cv::Point> vtmptach;
			RstPoTop.x = RstPoBot.x = RstPoLeft.x = RstPoRight.x = -1;
			RstPoTop.y = RstPoBot.y = RstPoLeft.y = RstPoRight.y = -1;
			for (int blobIdx = 0; blobIdx < teachTmp.size(); blobIdx++)
			{
				//zero index
				if (teachTmp[blobIdx].size() > 2)
				{
					int bPoI = 1;
					//When two points are located at the edge of the image
					if (((teachTmp[blobIdx][bPoI - 1].x == 0 && teachTmp[blobIdx][bPoI].x == 0) || (teachTmp[blobIdx][bPoI - 1].x >= teachImg.cols - 1 && teachTmp[blobIdx][bPoI].x >= teachImg.cols - 1))
						|| ((teachTmp[blobIdx][bPoI - 1].y == 0 && teachTmp[blobIdx][bPoI].y == 0) || (teachTmp[blobIdx][bPoI - 1].y >= teachImg.rows - 1 && teachTmp[blobIdx][bPoI].y >= teachImg.rows - 1)))
					{
					}
					else
					{
						vtmptach.push_back(teachTmp[blobIdx][bPoI - 1]);
						vXCnt[teachTmp[blobIdx][bPoI - 1].x]++;
						vYCnt[teachTmp[blobIdx][bPoI - 1].y]++;
					}
				}
				for (int bPoI = 1; bPoI < teachTmp[blobIdx].size(); bPoI++)
				{
					//When two points are located at the edge of the image
					if (((teachTmp[blobIdx][bPoI - 1].x == 0 && teachTmp[blobIdx][bPoI].x == 0) || (teachTmp[blobIdx][bPoI - 1].x >= teachImg.cols - 1 && teachTmp[blobIdx][bPoI].x >= teachImg.cols - 1))
						|| ((teachTmp[blobIdx][bPoI - 1].y == 0 && teachTmp[blobIdx][bPoI].y == 0) || (teachTmp[blobIdx][bPoI - 1].y >= teachImg.rows - 1 && teachTmp[blobIdx][bPoI].y >= teachImg.rows - 1)))
					{
						continue;
					}
					if (teachTmp[blobIdx][bPoI - 1].x == teachTmp[blobIdx][bPoI].x)
					{
						int sty = std::min(teachTmp[blobIdx][bPoI - 1].y, teachTmp[blobIdx][bPoI].y);
						int edy = std::max(teachTmp[blobIdx][bPoI - 1].y, teachTmp[blobIdx][bPoI].y);
						for (int poY = sty; poY <= edy; poY++)
						{
							vtmptach.push_back(cv::Point(teachTmp[blobIdx][bPoI].x, poY));
							vXCnt[teachTmp[blobIdx][bPoI].x]++;
							vYCnt[poY]++;
						}
					}
					else if (teachTmp[blobIdx][bPoI - 1].y == teachTmp[blobIdx][bPoI].y)
					{
						int stx = std::min(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
						int edx = std::max(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
						for (int pox = stx; pox <= edx; pox++)
						{
							vtmptach.push_back(cv::Point(pox, teachTmp[blobIdx][bPoI].y));
							vXCnt[pox]++;
							vYCnt[teachTmp[blobIdx][bPoI].y]++;
						}
					}
					else
					{
						float fDiffx = teachTmp[blobIdx][bPoI].x - teachTmp[blobIdx][bPoI - 1].x;
						float fDiffy = teachTmp[blobIdx][bPoI].y - teachTmp[blobIdx][bPoI - 1].y;
						float fa = fDiffy / fDiffx;
						float fb = teachTmp[blobIdx][bPoI - 1].y - fa * teachTmp[blobIdx][bPoI - 1].x;
						int stx = std::min(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
						int edx = std::max(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
						for (int pox = stx; pox <= edx; pox++)
						{
							float poY = fa * pox + fb;
							vtmptach.push_back(cv::Point(pox, poY));
							vXCnt[pox]++;
							vYCnt[poY]++;
						}
					}
					if (vtmptach.size() > 0)
					{
						Teachcontours.insert(Teachcontours.end(), vtmptach.begin(), vtmptach.end());
					}
				}
			}
			int nMX(0), nMY(0), nMXCnt(0), nMYCnt(0);
			for (int n = 0; n < vXCnt.size(); n++)
			{
				if (vXCnt[n] > nMXCnt)
				{
					nMX = n;
					nMXCnt = vXCnt[n];
				}
			}
			for (int n = 0; n < vYCnt.size(); n++)
			{
				if (vYCnt[n] > nMYCnt)
				{
					nMY = n;
					nMYCnt = vYCnt[n];
				}
			}
			if ((nWndDir & 0x01) == 0x01)//top
			{
				int nCntBlack = 0;
				for (int n = ContoursMaxImg.rows - 1; n > 0; n--)
				{
					int nWPixel = 0;
					for (int ncolIdx = nMX - 5; ncolIdx < (nMX + 5); ncolIdx++)
					{
						if (ncolIdx < 0) continue;
						if (ContoursMaxImg.ptr(n)[ncolIdx] > 0) nWPixel++;
					}
					if (nWPixel > 0)
					{
						RstPoTop.y = n;
						nCntBlack = 0;
						if (RstPoBot.y == -1)
							RstPoBot.y = n;
					}
					else if (RstPoTop.y > -1)
						nCntBlack++;
					if (nCntBlack > 5)
						break;
				}
				RstPoTop.x = RstPoBot.x = nMX;
				if ((nWndDir & 0x04) == 0x04)//left
				{
					int nCntBlack = 0;
					for (int n = ContoursMaxImg.cols - 1; n > 0; n--)
					{
						int nWPixel = 0;
						for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
						{
							if (nrowIdx < 0) continue;
							if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
						}
						if (nWPixel > 0)
						{
							RstPoLeft.x = n;
							nCntBlack = 0;
							if (RstPoRight.x == -1)
								RstPoRight.x = n;
						}
						else if (RstPoLeft.x > -1)
							nCntBlack++;
						if (nCntBlack > 5)
							break;
					}
					RstPoRight.y = RstPoLeft.y = nMY;
					float fy = RstPoLeft.y - RstPoTop.y;
					float fx = RstPoLeft.x - RstPoTop.x;
					float fTermX = fx / 3;
					float fTermY = fy / 3;
					RstPoDiagLeft.x = RstPoTop.x + fTermX;
					RstPoDiagLeft.y = RstPoTop.y + fTermY;
					RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
					RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
					cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
					for (int r = 0; r < RstPoDiagLeft.y; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y - r);
						if ((RstPoDiagLeft.x - r) >= 0 && uPtr[((int)RstPoDiagLeft.x - r)] > 0)
						{
							pTmp.x = RstPoDiagLeft.x - r;
							pTmp.y = RstPoDiagLeft.y - r;
						}
					}
					RstPoDiagLeft.x = pTmp.x;
					RstPoDiagLeft.y = pTmp.y;

					pTmp.x = RstPoDiagRight.x;
					pTmp.y = RstPoDiagRight.y;
					for (int r = 0; r < RstPoDiagRight.y; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagRight.y - r);
						if ((RstPoDiagRight.x - r) >= 0 && uPtr[((int)RstPoDiagRight.x - r)] > 0)
						{
							pTmp.x = RstPoDiagRight.x - r;
							pTmp.y = RstPoDiagRight.y - r;
						}
					}
					RstPoDiagRight.x = pTmp.x;
					RstPoDiagRight.y = pTmp.y;
				}
				else if ((nWndDir & 0x08) == 0x08)//right
				{
					int nCntBlack = 0;
					for (int n = 0; n < ContoursMaxImg.cols; n++)
					{
						int nWPixel = 0;
						for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
						{
							if (nrowIdx < 0) continue;
							if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
						}
						if (nWPixel > 0)
						{
							RstPoRight.x = n;
							nCntBlack = 0;
							if (RstPoLeft.x == -1)
								RstPoLeft.x = n;
						}
						else if (RstPoRight.x > -1)
							nCntBlack++;
						if (nCntBlack > 5)
							break;
					}
					RstPoRight.y = RstPoLeft.y = nMY;
					float fy = RstPoTop.y - RstPoRight.y;
					float fx = RstPoTop.x - RstPoRight.x;
					float fTermX = fx / 3;
					float fTermY = fy / 3;
					RstPoDiagLeft.x = RstPoRight.x + fTermX;
					RstPoDiagLeft.y = RstPoRight.y + fTermY;
					RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
					RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
					cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
					for (int r = 0; r < RstPoDiagLeft.y; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y - r);
						if ((RstPoDiagLeft.x + r) < teachImg.cols && uPtr[((int)RstPoDiagLeft.x + r)] > 0)
						{
							pTmp.x = RstPoDiagLeft.x + r;
							pTmp.y = RstPoDiagLeft.y - r;
						}
					}
					RstPoDiagLeft.x = pTmp.x;
					RstPoDiagLeft.y = pTmp.y;

					pTmp.x = RstPoDiagRight.x;
					pTmp.y = RstPoDiagRight.y;
					for (int r = 0; r < RstPoDiagRight.y; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagRight.y - r);
						if ((RstPoDiagRight.x + r) < teachImg.cols && uPtr[((int)RstPoDiagRight.x + r)] > 0)
						{
							pTmp.x = RstPoDiagRight.x + r;
							pTmp.y = RstPoDiagRight.y - r;
						}
					}
					RstPoDiagRight.x = pTmp.x;
					RstPoDiagRight.y = pTmp.y;
				}
			}
			else if ((nWndDir & 0x02) == 0x02)//bot
			{
				int nCntBlack = 0;
				for (int n = 0; n < ContoursMaxImg.rows; n++)
				{
					int nWPixel = 0;
					for (int ncolIdx = nMX - 5; ncolIdx < (nMX + 5); ncolIdx++)
					{
						if (ncolIdx < 0) continue;
						if (ContoursMaxImg.ptr(n)[ncolIdx] > 0) nWPixel++;
					}
					if (nWPixel > 0)
					{
						RstPoBot.y = n;
						nCntBlack = 0;
						if (RstPoTop.y == -1)
							RstPoTop.y = n;
					}
					else if (RstPoBot.y > -1)
						nCntBlack++;
					if (nCntBlack > 5)
						break;
				}
				RstPoTop.x = RstPoBot.x = nMX;
				if ((nWndDir & 0x04) == 0x04)//left
				{
					int nCntBlack = 0;
					for (int n = ContoursMaxImg.cols - 1; n > 0; n--)
					{
						int nWPixel = 0;
						for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
						{
							if (nrowIdx < 0) continue;
							if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
						}
						if (nWPixel > 0)
						{
							RstPoLeft.x = n;
							nCntBlack = 0;
							if (RstPoRight.x == -1)
								RstPoRight.x = n;
						}
						else if (RstPoLeft.x > -1)
							nCntBlack++;
						if (nCntBlack > 5)
							break;
					}
					RstPoRight.y = RstPoLeft.y = nMY;
					float fy = RstPoLeft.y - RstPoBot.y;
					float fx = RstPoLeft.x - RstPoBot.x;
					float fTermX = fx / 3;
					float fTermY = fy / 3;
					RstPoDiagLeft.x = RstPoBot.x + fTermX;
					RstPoDiagLeft.y = RstPoBot.y + fTermY;
					RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
					RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
					cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
					int nEdy = teachImg.rows - RstPoDiagLeft.y;
					for (int r = 0; r < nEdy; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y + r);
						if ((RstPoDiagLeft.x - r) >= 0 && uPtr[((int)RstPoDiagLeft.x - r)] > 0)
						{
							pTmp.x = RstPoDiagLeft.x - r;
							pTmp.y = RstPoDiagLeft.y + r;
						}
					}
					RstPoDiagLeft.x = pTmp.x;
					RstPoDiagLeft.y = pTmp.y;

					pTmp.x = RstPoDiagRight.x;
					pTmp.y = RstPoDiagRight.y;
					nEdy = teachImg.rows - RstPoDiagRight.y;
					for (int r = 0; r < nEdy; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagRight.y + r);
						if ((RstPoDiagRight.x - r) >= 0 && uPtr[((int)RstPoDiagRight.x - r)] > 0)
						{
							pTmp.x = RstPoDiagRight.x - r;
							pTmp.y = RstPoDiagRight.y + r;
						}
					}
					RstPoDiagRight.x = pTmp.x;
					RstPoDiagRight.y = pTmp.y;
				}
				else if ((nWndDir & 0x08) == 0x08)//right
				{
					int nCntBlack = 0;
					for (int n = 0; n < ContoursMaxImg.cols; n++)
					{
						int nWPixel = 0;
						for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
						{
							if (nrowIdx < 0) continue;
							if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
						}
						if (nWPixel > 0)
						{
							RstPoRight.x = n;
							nCntBlack = 0;
							if (RstPoLeft.x == -1)
								RstPoLeft.x = n;
						}
						else if (RstPoRight.x > -1)
							nCntBlack++;
						if (nCntBlack > 5)
							break;
					}
					RstPoRight.y = RstPoLeft.y = nMY;
					float fy = RstPoBot.y - RstPoRight.y;
					float fx = RstPoBot.x - RstPoRight.x;
					float fTermX = fx / 3;
					float fTermY = fy / 3;
					RstPoDiagLeft.x = RstPoRight.x + fTermX;
					RstPoDiagLeft.y = RstPoRight.y + fTermY;
					RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
					RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
					cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
					int nEdy = teachImg.rows - RstPoDiagLeft.y;
					for (int r = 0; r < nEdy; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y + r);
						if ((RstPoDiagLeft.x + r) < teachImg.cols && uPtr[((int)RstPoDiagLeft.x + r)] > 0)
						{
							pTmp.x = RstPoDiagLeft.x + r;
							pTmp.y = RstPoDiagLeft.y + r;
						}
					}
					RstPoDiagLeft.x = pTmp.x;
					RstPoDiagLeft.y = pTmp.y;

					pTmp.x = RstPoDiagRight.x;
					pTmp.y = RstPoDiagRight.y;
					nEdy = teachImg.rows - RstPoDiagRight.y;
					for (int r = 0; r < nEdy; r++)
					{
						uchar* uPtr = teachImg.ptr(RstPoDiagRight.y + r);
						if ((RstPoDiagRight.x + r) < teachImg.cols && uPtr[((int)RstPoDiagRight.x + r)] > 0)
						{
							pTmp.x = RstPoDiagRight.x + r;
							pTmp.y = RstPoDiagRight.y + r;
						}
					}
					RstPoDiagRight.x = pTmp.x;
					RstPoDiagRight.y = pTmp.y;
				}
			}
			if (RstPoLeft.x < RstPoDiagLeft.x)//right
			{
				if (RstPoLeft.y > RstPoDiagLeft.y)//bot
				{
					sRstAlgo->Po1 = RstPoLeft;
					sRstAlgo->Po2 = RstPoRight;
					sRstAlgo->Po3 = RstPoDiagLeft;
					sRstAlgo->Po4 = RstPoDiagRight;
					sRstAlgo->Po5 = RstPoBot;
					sRstAlgo->Po6 = RstPoTop;
				}
				else//top
				{
					sRstAlgo->Po1 = RstPoLeft;
					sRstAlgo->Po2 = RstPoRight;
					sRstAlgo->Po3 = RstPoDiagLeft;
					sRstAlgo->Po4 = RstPoDiagRight;
					sRstAlgo->Po5 = RstPoTop;
					sRstAlgo->Po6 = RstPoBot;
				}
			}
			else//left
			{
				if (RstPoLeft.y > RstPoDiagLeft.y)//bot
				{
					sRstAlgo->Po1 = RstPoRight;
					sRstAlgo->Po2 = RstPoLeft;
					sRstAlgo->Po3 = RstPoDiagRight;
					sRstAlgo->Po4 = RstPoDiagLeft;
					sRstAlgo->Po5 = RstPoBot;
					sRstAlgo->Po6 = RstPoTop;
				}
				else//top
				{
					sRstAlgo->Po1 = RstPoRight;
					sRstAlgo->Po2 = RstPoLeft;
					sRstAlgo->Po3 = RstPoDiagRight;
					sRstAlgo->Po4 = RstPoDiagLeft;
					sRstAlgo->Po5 = RstPoTop;
					sRstAlgo->Po6 = RstPoBot;
				}
			}
			pAlgoBlob->Po1 = sRstAlgo->Po1;
			pAlgoBlob->Po2 = sRstAlgo->Po2;
			pAlgoBlob->Po3 = sRstAlgo->Po3;
			pAlgoBlob->Po4 = sRstAlgo->Po4;
			pAlgoBlob->Po5 = sRstAlgo->Po5;
			pAlgoBlob->Po6 = sRstAlgo->Po6;
			pAlgoBlob->m_nPoCnt = 6;
#if _DEBUG
			cv::Mat TechingcontoursMat = ContoursMaxImg.clone();
			cv::cvtColor(TechingcontoursMat, TechingcontoursMat, cv::COLOR_GRAY2BGR);
			for (int contIdx = 0; contIdx < Teachcontours.size(); contIdx++)
			{
				cv::line(TechingcontoursMat, Teachcontours[contIdx], Teachcontours[contIdx], cv::Scalar(0, 0, 255));
			}
			cv::imwrite("D:\\TechingcontoursMat.bmp", TechingcontoursMat);
			TechingcontoursMat = ContoursMaxImg.clone();
			cv::cvtColor(TechingcontoursMat, TechingcontoursMat, cv::COLOR_GRAY2BGR);
			cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po1.x, pAlgoBlob->Po1.y), cv::Point(pAlgoBlob->Po2.x, pAlgoBlob->Po2.y), cv::Scalar(0, 0, 255));
			cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po3.x, pAlgoBlob->Po3.y), cv::Point(pAlgoBlob->Po4.x, pAlgoBlob->Po4.y), cv::Scalar(0, 0, 255));
			cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po5.x, pAlgoBlob->Po5.y), cv::Point(pAlgoBlob->Po6.x, pAlgoBlob->Po6.y), cv::Scalar(0, 0, 255));
			cv::imwrite("D:\\TechingcontoursMat2.bmp", TechingcontoursMat);
#endif
		}
	}
#ifdef Thining
	UCHAR* ThiningBuf = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
	m_pCPInsp_Algo->ImageThining(nImgWidth, nImgHeight, pUcImgBlob, ThiningBuf);

	cv::Mat ThiningImg(nImgHeight, nImgWidth, CV_8UC1, ThiningBuf);
	cv::line(ThiningImg, cv::Point(0, 0), cv::Point(ThiningImg.cols - 1, 0), cv::Scalar(0));
	cv::line(ThiningImg, cv::Point(0, ThiningImg.rows - 1), cv::Point(ThiningImg.cols - 1, ThiningImg.rows - 1), cv::Scalar(0));
	cv::line(ThiningImg, cv::Point(0, 0), cv::Point(0, ThiningImg.rows), cv::Scalar(0));
	cv::line(ThiningImg, cv::Point(ThiningImg.cols - 1, 0), cv::Point(ThiningImg.cols - 1, ThiningImg.rows - 1), cv::Scalar(0));
#else
	cv::Mat ThiningImg(nImgHeight, nImgWidth, CV_8UC1);
#endif
	cv::Mat LabelImg(nImgHeight, nImgWidth, CV_16UC1);
	LabelImg.setTo(0);
	cv::Point MinPo, MaxPo;
	long pLebel[PTR_BLOB_MAX];
	memset(pLebel, 0, sizeof(long) * PTR_BLOB_MAX);
	m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImg.ptr<USHORT>(), nImgWidth, nImgHeight);
	LabelImg.convertTo(LabelImg, CV_8UC1);
	float fMinth = pAlgoBlob->m_nMinThickRemove;
	int nLabel = 0;
	float nminVal = (float)(-INFINITY);
	float nMaxVal = 0;
	RECT LabelRects[BLOB_RECT_CNTS];
	m_pProcMilAlgo->GetBlobResult_Rects(LabelRects, BLOB_RECT_CNTS);
	//rect표현1 simple
	ThminRect.left = MinPo.x - nminVal;
	ThminRect.top = MinPo.y - nminVal;
	ThminRect.right = MinPo.x + nminVal;
	ThminRect.bottom = MinPo.y + nminVal;

	ThmaxRect.left = MaxPo.x - nMaxVal;
	ThmaxRect.top = MaxPo.y - nMaxVal;
	ThmaxRect.right = MaxPo.x + nMaxVal;
	ThmaxRect.bottom = MaxPo.y + nMaxVal;
	RECT ThminRectTmp, ThmaxRectTmp;
	ThmaxRectTmp = ThmaxRect;
	while (pLebel[nLabel] != 0)
	{
		if (pAlgoBlob->m_nThickDir == 1)//Width
		{
			nMaxVal = 0;
			for (int r = 0; r < nImgHeight; r++)
			{
				UCHAR* uPtr = LabelImg.ptr(r);
				int nMinW = 0;
				int nMaxW = 0;
				bool bWhite = false;
				for (int c = 0; c < nImgWidth; c++)
				{
					if (uPtr[c] == pLebel[nLabel])
					{
						nMaxW = nMinW = c;
						bWhite = true;
						break;
					}
				}
				if (bWhite)
				{
					for (int c = nMinW + 1; c < nImgWidth; c++)
					{
						if (uPtr[c] == 0)
						{
							nMaxW = c;
							break;
						}
					}
				}
				if (nMaxW - nMinW > 0)
				{
					if (minVal > (nMaxW - nMinW))
					{
						nminVal = (nMaxW - nMinW);
						ThminRect.top = r - 1;
						ThminRect.bottom = r + 1;
						ThminRect.left = nMinW;
						ThminRect.right = nMaxW;
					}
					if (nMaxVal < (nMaxW - nMinW))
					{
						nMaxVal = (nMaxW - nMinW);
						ThmaxRectTmp.top = r - 1;
						ThmaxRectTmp.bottom = r + 1;
						ThmaxRectTmp.left = nMinW;
						ThmaxRectTmp.right = nMaxW;
					}
				}
			}
			if (minVal == nImgWidth && ThminRect.top == 0 && ThminRect.bottom == 0)
			{
				minVal = 0;
			}
			if ((float)(nMaxVal) < fMinth)
			{
				cv::Mat removeImg, removeImg_inv;
				cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
				cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
				removeImg = removeImg_inv & removeImg;
				ContoursMaxImg = ContoursMaxImg - removeImg;
			}
			else if (nMaxVal > maxVal)
			{

				maxVal = nMaxVal;
				ThmaxRect.top = ThmaxRectTmp.top;
				ThmaxRect.bottom = ThmaxRectTmp.bottom;
				ThmaxRect.left = ThmaxRectTmp.left;
				ThmaxRect.right = ThmaxRectTmp.right;
			}
			if (maxVal <= 0)
			{
				ThmaxRect.left = MaxPo.x - nMaxVal;
				ThmaxRect.top = MaxPo.y - nMaxVal;
				ThmaxRect.right = MaxPo.x + nMaxVal;
				ThmaxRect.bottom = MaxPo.y + nMaxVal;
			}
		}
		else if (pAlgoBlob->m_nThickDir == 2)//length
		{
			nMaxVal = 0;
			minVal = nImgHeight;
			for (int c = 0; c < nImgWidth; c++)
			{
				int nMinW = 0;
				int nMaxW = 0;
				bool bWhite = false;
				for (int r = 0; r < nImgHeight; r++)
				{
					UCHAR* uPtr = LabelImg.ptr(r);
					if (uPtr[c] == pLebel[nLabel])
					{
						nMaxW = nMinW = r;
						bWhite = true;
						break;
					}
				}
				if (bWhite)
				{
					for (int r = nMaxW + 1; r < nImgHeight; r++)
					{
						UCHAR* uPtr = LabelImg.ptr(r);
						if (uPtr[c] == 0)
						{
							nMaxW = r;
							break;
						}
					}
				}
				if (nMaxW - nMinW > 0)
				{
					if (minVal > (nMaxW - nMinW))
					{
						minVal = (nMaxW - nMinW);
						ThminRect.top = nMinW;
						ThminRect.bottom = nMaxW;
						ThminRect.left = c - 1;
						ThminRect.right = c + 1;
					}
					if (nMaxVal < (nMaxW - nMinW))
					{
						nMaxVal = (nMaxW - nMinW);
						ThmaxRectTmp.top = nMinW;
						ThmaxRectTmp.bottom = nMaxW;
						ThmaxRectTmp.left = c - 1;
						ThmaxRectTmp.right = c + 1;
					}
				}
			}
			if (minVal == nImgHeight && ThminRect.top == 0 && ThminRect.bottom == 0)
			{
				minVal = 0;
			}
			if ((float)(nMaxVal) < fMinth)
			{
				cv::Mat removeImg, removeImg_inv;
				cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
				cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
				removeImg = removeImg_inv & removeImg;
				ContoursMaxImg = ContoursMaxImg - removeImg;
			}
			else if (nMaxVal > maxVal)
			{

				maxVal = nMaxVal;
				ThmaxRect.top = ThmaxRectTmp.top;
				ThmaxRect.bottom = ThmaxRectTmp.bottom;
				ThmaxRect.left = ThmaxRectTmp.left;
				ThmaxRect.right = ThmaxRectTmp.right;
			}
			if (maxVal <= 0)
			{
				ThmaxRect.left = MaxPo.x - nMaxVal;
				ThmaxRect.top = MaxPo.y - nMaxVal;
				ThmaxRect.right = MaxPo.x + nMaxVal;
				ThmaxRect.bottom = MaxPo.y + nMaxVal;
			}
		}
		else if (pAlgoBlob->m_nThickDir == 3)// +1 정방향
		{
			nMaxVal = 0;
			minVal = nImgHeight;
			int a = 1;

			cv::Point PSt(0, 0), PEd(0, 0);
			if ((nWndDir & 0x01) == 0x01)//top
			{
				int b = LabelRects[nLabel].top - LabelRects[nLabel].left;
				int c = LabelRects[nLabel].left;
				for (; c < LabelRects[nLabel].right; c++)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = PSt.x = c;
						PEd.y = PSt.y = y;
						c++;
						break;
					}
				}
				for (; c < LabelRects[nLabel].right; c++)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = c;
						PEd.y = y;
					}
				}
				float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
				if (nMaxVal < fdist)
				{
					nMaxVal = fdist;
					ThmaxRectTmp.top = PSt.y;
					ThmaxRectTmp.bottom = PEd.y;
					ThmaxRectTmp.left = PSt.x;
					ThmaxRectTmp.right = PEd.x;
				}
				if (minVal > fdist)
				{
					minVal = fdist;
					ThminRectTmp.top = PSt.y;
					ThminRectTmp.bottom = PEd.y;
					ThminRectTmp.left = PSt.x;
					ThminRectTmp.right = PEd.x;
				}
			}
			else if ((nWndDir & 0x02) == 0x02)//bot
			{
				int b = LabelRects[nLabel].bottom - (LabelRects[nLabel].right - 1);
				int c = LabelRects[nLabel].right - 1;
				for (; c >= 0; c--)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = PSt.x = c;
						PEd.y = PSt.y = y;
						break;
					}
				}
				for (; c >= 0; c--)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PSt.x = c;
						PSt.y = y;
					}
				}
				float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
				if (nMaxVal < fdist)
				{
					nMaxVal = fdist;
					ThmaxRectTmp.top = PSt.y;
					ThmaxRectTmp.bottom = PEd.y;
					ThmaxRectTmp.left = PSt.x;
					ThmaxRectTmp.right = PEd.x;
				}
				if (minVal > fdist)
				{
					minVal = fdist;
					ThminRectTmp.top = PSt.y;
					ThminRectTmp.bottom = PEd.y;
					ThminRectTmp.left = PSt.x;
					ThminRectTmp.right = PEd.x;
				}
			}
			/*for (int r = nImgHeight - 1; r > 0; r--)
			{
				cv::Point PSt(0, 0), PEd(0, 0);
				int b = r;
				for (int c = 0; c < nImgWidth; c++)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = PSt.x = c;
						PEd.y = PSt.y = y;
						break;
					}
				}
				for (int c = PSt.x + 1; c < nImgWidth; c++)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = c;
						PEd.y = y;
					}
				}
				float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
				if (nMaxVal < fdist)
				{
					nMaxVal = fdist;
					ThmaxRectTmp.top = PSt.y;
					ThmaxRectTmp.bottom = PEd.y;
					ThmaxRectTmp.left = PSt.x;
					ThmaxRectTmp.right = PEd.x;
				}
				if (minVal > fdist)
				{
					minVal = fdist;
					ThminRectTmp.top = PSt.y;
					ThminRectTmp.bottom = PEd.y;
					ThminRectTmp.left = PSt.x;
					ThminRectTmp.right = PEd.x;
				}
			}
			for (int cOrg = 1; cOrg < nImgWidth; cOrg++)
			{
				cv::Point PSt(0, 0), PEd(0, 0);
				int b = (-1) * a * cOrg;
				for (int c = cOrg; c < nImgWidth; c++)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = PSt.x = c;
						PEd.y = PSt.y = y;
						break;
					}
				}
				for (int c = PSt.x + 1; c < nImgWidth; c++)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = c;
						PEd.y = y;
					}
				}
				float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
				if (nMaxVal < fdist)
				{
					nMaxVal = fdist;
					ThmaxRectTmp.top = PSt.y;
					ThmaxRectTmp.bottom = PEd.y;
					ThmaxRectTmp.left = PSt.x;
					ThmaxRectTmp.right = PEd.x;
				}
				if (minVal > fdist)
				{
					minVal = fdist;
					ThminRectTmp.top = PSt.y;
					ThminRectTmp.bottom = PEd.y;
					ThminRectTmp.left = PSt.x;
					ThminRectTmp.right = PEd.x;
				}
			}
*/
			if (minVal == nImgHeight && ThminRect.top == 0 && ThminRect.bottom == 0)
			{
				minVal = 0;
			}
			if ((float)(nMaxVal) < fMinth)
			{
				cv::Mat removeImg, removeImg_inv;
				cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
				cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
				removeImg = removeImg_inv & removeImg;
				ContoursMaxImg = ContoursMaxImg - removeImg;
			}
			else if (nMaxVal > maxVal)
			{

				maxVal = nMaxVal;
				ThmaxRect.top = ThmaxRectTmp.top < ThmaxRectTmp.bottom ? ThmaxRectTmp.top : ThmaxRectTmp.bottom;
				ThmaxRect.bottom = ThmaxRectTmp.top > ThmaxRectTmp.bottom ? ThmaxRectTmp.top : ThmaxRectTmp.bottom;
				ThmaxRect.left = ThmaxRectTmp.left < ThmaxRectTmp.right ? ThmaxRectTmp.left : ThmaxRectTmp.right;
				ThmaxRect.right = ThmaxRectTmp.left > ThmaxRectTmp.right ? ThmaxRectTmp.left : ThmaxRectTmp.right;
			}
			if (maxVal <= 0)
			{
				ThmaxRect.left = MaxPo.x - nMaxVal;
				ThmaxRect.top = MaxPo.y - nMaxVal;
				ThmaxRect.right = MaxPo.x + nMaxVal;
				ThmaxRect.bottom = MaxPo.y + nMaxVal;
			}
		}
		else if (pAlgoBlob->m_nThickDir == 4)// -1 역방향
		{
			nMaxVal = 0;
			minVal = nImgHeight;
			int a = -1;
			cv::Point PSt(0, 0), PEd(0, 0);
			if ((nWndDir & 0x01) == 0x01)//top
			{
				int b = (LabelRects[nLabel].right - 1) + LabelRects[nLabel].top;
				int c = LabelRects[nLabel].right - 1;
				for (; c >= LabelRects[nLabel].left; c--)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = PSt.x = c;
						PEd.y = PSt.y = y;
						break;
					}
				}
				for (; c >= LabelRects[nLabel].left; c--)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = c;
						PEd.y = y;
					}
				}
				float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
				if (nMaxVal < fdist)
				{
					nMaxVal = fdist;
					ThmaxRectTmp.top = PSt.y;
					ThmaxRectTmp.bottom = PEd.y;
					ThmaxRectTmp.left = PSt.x;
					ThmaxRectTmp.right = PEd.x;
				}
				if (minVal > fdist)
				{
					minVal = fdist;
					ThminRectTmp.top = PSt.y;
					ThminRectTmp.bottom = PEd.y;
					ThminRectTmp.left = PSt.x;
					ThminRectTmp.right = PEd.x;
				}
			}
			else if ((nWndDir & 0x02) == 0x02)//bot
			{
				int b = LabelRects[nLabel].bottom + LabelRects[nLabel].left;
				int c = LabelRects[nLabel].left;
				for (; c < LabelRects[nLabel].right; c++)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = PSt.x = c;
						PEd.y = PSt.y = y;
						break;
					}
				}
				for (; c < LabelRects[nLabel].right; c++)
				{
					int y = a * c + b;
					if (y < 0)
						continue;
					if (y >= nImgHeight)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PSt.x = c;
						PSt.y = y;
					}
				}
				float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
				if (nMaxVal < fdist)
				{
					nMaxVal = fdist;
					ThmaxRectTmp.top = PSt.y;
					ThmaxRectTmp.bottom = PEd.y;
					ThmaxRectTmp.left = PSt.x;
					ThmaxRectTmp.right = PEd.x;
				}
				if (minVal > fdist)
				{
					minVal = fdist;
					ThminRectTmp.top = PSt.y;
					ThminRectTmp.bottom = PEd.y;
					ThminRectTmp.left = PSt.x;
					ThminRectTmp.right = PEd.x;
				}
			}
			/*for (int r = 0; r < nImgHeight; r++)
			{
				cv::Point PSt(0, 0), PEd(0, 0);
				PEd.x = PSt.x = nImgWidth;
				PEd.y = PSt.y = nImgHeight;
				int b = r;
				for (int c = 0; c < nImgWidth; c++)
				{
					int y = a * c + b;
					if (y < 0)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = PSt.x = c;
						PEd.y = PSt.y = y;
						break;
					}
				}
				for (int c = PSt.x + 1; c < nImgWidth; c++)
				{
					int y = a * c + b;
					if (y < 0)
						break;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = c;
						PEd.y = y;
					}
				}
				float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
				if (nMaxVal < fdist)
				{
					nMaxVal = fdist;
					ThmaxRectTmp.top = PSt.y;
					ThmaxRectTmp.bottom = PEd.y;
					ThmaxRectTmp.left = PSt.x;
					ThmaxRectTmp.right = PEd.x;
				}
				if (minVal > fdist)
				{
					minVal = fdist;
					ThminRectTmp.top = PSt.y;
					ThminRectTmp.bottom = PEd.y;
					ThminRectTmp.left = PSt.x;
					ThminRectTmp.right = PEd.x;
				}
			}
			for (int cOrg = 1; cOrg < nImgWidth; cOrg++)
			{
				cv::Point PSt(0, 0), PEd(0, 0);
				PEd.x = PSt.x = nImgWidth;
				PEd.y = PSt.y = nImgHeight;
				int b = (-1) * a * cOrg + (nImgHeight - 1);
				for (int c = cOrg; c < nImgWidth; c++)
				{
					int y = a * c + b;
					if (y < 0)
						break;
					if (y >= nImgHeight)
						continue;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = PSt.x = c;
						PEd.y = PSt.y = y;
						break;
					}
				}
				for (int c = PSt.x + 1; c < nImgWidth; c++)
				{
					int y = a * c + b;
					if (y < 0)
						break;
					if (y >= nImgHeight)
						continue;
					UCHAR* uPtr = LabelImg.ptr(y);
					if (uPtr[c] == pLebel[nLabel])
					{
						PEd.x = c;
						PEd.y = y;
					}
				}
				float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
				if (nMaxVal < fdist)
				{
					nMaxVal = fdist;
					ThmaxRectTmp.top = PSt.y;
					ThmaxRectTmp.bottom = PEd.y;
					ThmaxRectTmp.left = PSt.x;
					ThmaxRectTmp.right = PEd.x;
				}
				if (minVal > fdist)
				{
					minVal = fdist;
					ThminRectTmp.top = PSt.y;
					ThminRectTmp.bottom = PEd.y;
					ThminRectTmp.left = PSt.x;
					ThminRectTmp.right = PEd.x;
				}
			}*/
			if (minVal == nImgHeight && ThminRect.top == 0 && ThminRect.bottom == 0)
			{
				minVal = 0;
			}
			if ((float)(nMaxVal) < fMinth)
			{
				cv::Mat removeImg, removeImg_inv;
				cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
				cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
				removeImg = removeImg_inv & removeImg;
				ContoursMaxImg = ContoursMaxImg - removeImg;
			}
			else if (nMaxVal > maxVal)
			{

				maxVal = nMaxVal;
				ThmaxRect.top = ThmaxRectTmp.top < ThmaxRectTmp.bottom ? ThmaxRectTmp.top : ThmaxRectTmp.bottom;
				ThmaxRect.bottom = ThmaxRectTmp.top > ThmaxRectTmp.bottom ? ThmaxRectTmp.top : ThmaxRectTmp.bottom;
				ThmaxRect.left = ThmaxRectTmp.left < ThmaxRectTmp.right ? ThmaxRectTmp.left : ThmaxRectTmp.right;
				ThmaxRect.right = ThmaxRectTmp.left > ThmaxRectTmp.right ? ThmaxRectTmp.left : ThmaxRectTmp.right;
			}
			if (maxVal <= 0)
			{
				ThmaxRect.left = MaxPo.x - nMaxVal;
				ThmaxRect.top = MaxPo.y - nMaxVal;
				ThmaxRect.right = MaxPo.x + nMaxVal;
				ThmaxRect.bottom = MaxPo.y + nMaxVal;
			}
		}
		else//all direction
		{
			cv::Point PartCenter;
			PartCenter.x = -fWndCenterX;
			PartCenter.y = -fWndCenterY;
			if (pAlgoBlob->m_nPoCnt > 0 && pAlgoBlob->m_bUseTeachTh)
			{
				std::vector<std::vector<cv::Point>> contours;
				std::vector<cv::Point> thickPoints;
				std::vector<cv::Point> vLinePo;
				vLinePo.push_back(cv::Point(pAlgoBlob->Po1.x, pAlgoBlob->Po1.y));
				vLinePo.push_back(cv::Point(pAlgoBlob->Po2.x, pAlgoBlob->Po2.y));
				vLinePo.push_back(cv::Point(pAlgoBlob->Po3.x, pAlgoBlob->Po3.y));
				vLinePo.push_back(cv::Point(pAlgoBlob->Po4.x, pAlgoBlob->Po4.y));
				vLinePo.push_back(cv::Point(pAlgoBlob->Po5.x, pAlgoBlob->Po5.y));
				vLinePo.push_back(cv::Point(pAlgoBlob->Po6.x, pAlgoBlob->Po6.y));
				for (int poidx = 1; poidx < vLinePo.size(); poidx++)
				{
					double diffX = vLinePo[poidx].x - vLinePo[poidx - 1].x;
					double diffY = vLinePo[poidx].y - vLinePo[poidx - 1].y;
					if (diffX == 0)
					{
						for (int r = vLinePo[poidx - 1].y; r < vLinePo[poidx].y; r++)
						{
							thickPoints.push_back(cv::Point(vLinePo[poidx].x, r));
						}
					}
					else if (std::abs(diffY) > std::abs(diffX))
					{
						double dTmpA = (double)(diffX) / diffY;
						double dTmpB = vLinePo[poidx].x - ((double)vLinePo[poidx].y * dTmpA);
						for (int r = vLinePo[poidx - 1].x; r < vLinePo[poidx].x; r++)
						{
							thickPoints.push_back(cv::Point(vLinePo[poidx].y*dTmpA + dTmpB, vLinePo[poidx].y));
						}
					}
					else
					{
						double dTmpA = (double)(diffY) / diffX;
						double dTmpB = vLinePo[poidx].y - ((double)vLinePo[poidx].x * dTmpA);
						for (int r = vLinePo[poidx - 1].x; r < vLinePo[poidx].x; r++)
						{
							thickPoints.push_back(cv::Point(vLinePo[poidx].x, vLinePo[poidx].x*dTmpA + dTmpB));
						}
					}
				}
				std::vector<cv::Point> dstPo = thickness3(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), vLinePo, pLebel[nLabel]);
				int stX = dstPo[0].x;
				int stY = dstPo[0].y;
				int edX = dstPo[1].x;
				int edY = dstPo[1].y;
#if _DEBUG
				cv::Mat debugImg = ContoursMaxImg.clone();
				cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
				for (int contIdx = 0; contIdx < contours.size(); contIdx++)
				{
					thickPoints = contours[contIdx];
					int nColorDrEn = thickPoints.size() - 1;
					for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
					{
						cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
					}
				}
				cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
				cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif

				float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
				if (dist < nminVal)
				{
					nminVal = dist;
					ThminRect.left = stX;
					ThminRect.top = stY;
					ThminRect.right = edX;
					ThminRect.bottom = edY;
				}

				if ((float)(dist) < fMinth)
				{
					cv::Mat removeImg, removeImg_inv;
					cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
					cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
					removeImg = removeImg_inv & removeImg;
					ContoursMaxImg = ContoursMaxImg - removeImg;
				}
				else if (dist > nMaxVal)
				{
					nMaxVal = dist;
					ThmaxRect.left = std::min(stX, edX);
					ThmaxRect.top = std::min(stY, edY);
					ThmaxRect.right = std::max(stX, edX);
					ThmaxRect.bottom = std::max(stY, edY);
				}
				maxVal = (double)nMaxVal;
				minVal = (double)nminVal;
			}
			else
			{
				if (nWndDir != 0)
				{
					std::vector<CRect> vBlobRects;
					m_pProcMilAlgo->GetBlobResult_RectsV(vBlobRects);
					if ((nWndDir & 0x01) == 0x01)//top
					{
						if ((nWndDir & 0x04) == 0x04)//left
						{
							//round
							std::vector<std::vector<cv::Point>> contours;
							std::vector<cv::Point> thickPoints;
							double dTmpA = 1.0f;// (double)PartCenter.y / PartCenter.x;
							//double dTmpB = 0;
							for (int nBlobIdx = nLabel; nBlobIdx < nLabel + 1; nBlobIdx++)
							{
								for (int r = vBlobRects[nBlobIdx].bottom - 1; r > vBlobRects[nBlobIdx].top; r--)
								{
									//double dTmpA = (PartCenter.y - r) / (PartCenter.x);
									//double dTmpB = r;
									double dTmpB = r - (dTmpA * vBlobRects[nBlobIdx].left);

									for (int c = vBlobRects[nBlobIdx].left; c < vBlobRects[nBlobIdx].right; c++)
									{
										int y = dTmpA * c + dTmpB;
										if (y < 0 || y >= vBlobRects[nBlobIdx].bottom)
											continue;
										UCHAR* uPtr = LabelImg.ptr(y);
										if (uPtr[c] == pLebel[nLabel])
										{
											thickPoints.push_back(cv::Point(c, y));
											break;
										}
									}
								}
								for (int c = vBlobRects[nBlobIdx].left; c < vBlobRects[nBlobIdx].right; c++)
								{
									//double dTmpA = (PartCenter.y) / (PartCenter.x - c);
									double dTmpB = -dTmpA * c + vBlobRects[nBlobIdx].top;

									for (int r = vBlobRects[nBlobIdx].top; r < vBlobRects[nBlobIdx].bottom; r++)
									{
										int x = (r - dTmpB) / dTmpA;
										if (x < 0 || x >= vBlobRects[nBlobIdx].right)
											continue;
										UCHAR* uPtr = LabelImg.ptr(r);
										if (uPtr[x] == pLebel[nLabel])
										{
											thickPoints.push_back(cv::Point(x, r));
											break;
										}
									}
								}
								contours.push_back(thickPoints);
								thickPoints.clear();
							}
							std::vector<cv::Point> dstPo = thickness(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), pLebel[nLabel]);
							int stX = dstPo[0].x;
							int stY = dstPo[0].y;
							int edX = dstPo[1].x;
							int edY = dstPo[1].y;
#if _DEBUG
							cv::Mat debugImg = ContoursMaxImg.clone();
							cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
							for (int contIdx = 0; contIdx < contours.size(); contIdx++)
							{
								thickPoints = contours[contIdx];
								int nColorDrEn = thickPoints.size() - 1;
								for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
								{
									cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
								}
							}
							cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
							cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif
							//if (edX != 0 && stX != 0 && edY != 0 && stY != 0)
							{
								float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
								if (dist < nminVal)
								{
									nminVal = dist;
									ThminRect.left = stX;
									ThminRect.top = stY;
									ThminRect.right = edX;
									ThminRect.bottom = edY;
								}

								if ((float)(dist) < fMinth)
								{
									cv::Mat removeImg, removeImg_inv;
									cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
									cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
									removeImg = removeImg_inv & removeImg;
									ContoursMaxImg = ContoursMaxImg - removeImg;
								}
								else if (dist > nMaxVal)
								{
									nMaxVal = dist;
									ThmaxRect.left = std::min(stX, edX);
									ThmaxRect.top = std::min(stY, edY);
									ThmaxRect.right = std::max(stX, edX);
									ThmaxRect.bottom = std::max(stY, edY);
								}
							}
							maxVal = (double)nMaxVal;
							minVal = (double)nminVal;
						}
						else if ((nWndDir & 0x08) == 0x08)//right
						{
							//round
							std::vector<std::vector<cv::Point>> contours;
							std::vector<cv::Point> thickPoints;
							double dTmpA = -1.0f;// (double)PartCenter.y / PartCenter.x;
							//double dTmpB = 0;
							for (int nBlobIdx = nLabel; nBlobIdx < nLabel + 1; nBlobIdx++)
							{
								for (int r = vBlobRects[nBlobIdx].bottom - 1; r > vBlobRects[nBlobIdx].top; r--)
								{
									//double dTmpA = (PartCenter.y - r) / (PartCenter.x - ContoursMaxImg.cols - 1);
									double dTmpB = -dTmpA * (vBlobRects[nBlobIdx].right - 1) + r;
									for (int c = vBlobRects[nBlobIdx].right - 1; c >= vBlobRects[nBlobIdx].left; c--)
									{
										int y = dTmpA * c + dTmpB;
										if (y < 0 || y >= vBlobRects[nBlobIdx].bottom)
											continue;
										UCHAR* uPtr = LabelImg.ptr(y);
										if (uPtr[c] == pLebel[nLabel])
										{
											thickPoints.push_back(cv::Point(c, y));
											break;
										}
									}
								}
								for (int c = vBlobRects[nBlobIdx].right - 1; c >= vBlobRects[nBlobIdx].left; c--)
								{
									//double dTmpA = (PartCenter.y) / (PartCenter.x - c);
									double dTmpB = -dTmpA * c + vBlobRects[nBlobIdx].top;

									for (int r = vBlobRects[nBlobIdx].top; r < vBlobRects[nBlobIdx].bottom; r++)
									{
										int x = (r - dTmpB) / dTmpA;
										if (x < 0 || x >= vBlobRects[nBlobIdx].right)
											continue;
										UCHAR* uPtr = LabelImg.ptr(r);
										if (uPtr[x] == pLebel[nLabel])
										{
											thickPoints.push_back(cv::Point(x, r));
											break;
										}
									}
								}
								contours.push_back(thickPoints);
								thickPoints.clear();
							}
							std::vector<cv::Point> dstPo = thickness(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), pLebel[nLabel]);
							int stX = dstPo[0].x;
							int stY = dstPo[0].y;
							int edX = dstPo[1].x;
							int edY = dstPo[1].y;
#if _DEBUG
							cv::Mat debugImg = ContoursMaxImg.clone();
							cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
							for (int contIdx = 0; contIdx < contours.size(); contIdx++)
							{
								thickPoints = contours[contIdx];
								int nColorDrEn = thickPoints.size() - 1;
								for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
								{
									cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
								}
							}
							cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
							cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif
							//if (edX != 0 && stX != 0 && edY != 0 && stY != 0)
							{
								float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
								if (dist < nminVal)
								{
									nminVal = dist;
									ThminRect.left = stX;
									ThminRect.top = stY;
									ThminRect.right = edX;
									ThminRect.bottom = edY;
								}
								if ((float)(dist) < fMinth)
								{
									cv::Mat removeImg, removeImg_inv;
									cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
									cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
									removeImg = removeImg_inv & removeImg;
									ContoursMaxImg = ContoursMaxImg - removeImg;
								}
								else if (dist > nMaxVal)
								{
									nMaxVal = dist;
									ThmaxRect.left = std::min(stX, edX);
									ThmaxRect.top = std::min(stY, edY);
									ThmaxRect.right = std::max(stX, edX);
									ThmaxRect.bottom = std::max(stY, edY);
								}
							}
							maxVal = (double)nMaxVal;
							minVal = (double)nminVal;
						}
					}
					else if ((nWndDir & 0x02) == 0x02)//bot
					{
						if ((nWndDir & 0x04) == 0x04)//left
						{
							//round
							std::vector<std::vector<cv::Point>> contours;
							std::vector<cv::Point> thickPoints;
							double dTmpA = -1.0f;//(double)(PartCenter.y - (ContoursMaxImg.rows - 1)) / PartCenter.x;
							//double dTmpB = ContoursMaxImg.rows - 1;
							for (int nBlobIdx = nLabel; nBlobIdx < nLabel + 1; nBlobIdx++)
							{
								for (int r = vBlobRects[nBlobIdx].top; r < vBlobRects[nBlobIdx].bottom; r++)
								{
									double dTmpB = r - (dTmpA * vBlobRects[nBlobIdx].left);
									for (int c = vBlobRects[nBlobIdx].left; c < vBlobRects[nBlobIdx].right - 1; c++)
									{
										int y = dTmpA * c + dTmpB;
										if (y < 0 || y >= vBlobRects[nBlobIdx].bottom)
											continue;
										UCHAR* uPtr = LabelImg.ptr(y);
										if (uPtr[c] == pLebel[nLabel])
										{
											thickPoints.push_back(cv::Point(c, y));
											break;
										}
									}
								}
								for (int c = vBlobRects[nBlobIdx].left; c < vBlobRects[nBlobIdx].right - 1; c++)
								{
									//double dTmpA = (PartCenter.y - ContoursMaxImg.rows - 1) / (PartCenter.x - c);
									double dTmpB = -dTmpA * c + (vBlobRects[nBlobIdx].bottom - 1);
									for (int r = vBlobRects[nBlobIdx].bottom - 1; r >= vBlobRects[nBlobIdx].top; r--)
									{
										int x = (r - dTmpB) / dTmpA;
										if (x < 0 || x >= vBlobRects[nBlobIdx].right)
											continue;
										UCHAR* uPtr = LabelImg.ptr(r);
										if (uPtr[x] == pLebel[nLabel])
										{
											thickPoints.push_back(cv::Point(x, r));
											break;
										}
									}
								}
								contours.push_back(thickPoints);
								thickPoints.clear();
							}
							std::vector<cv::Point> dstPo = thickness(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), pLebel[nLabel]);
							int stX = dstPo[0].x;
							int stY = dstPo[0].y;
							int edX = dstPo[1].x;
							int edY = dstPo[1].y;
#if _DEBUG
							cv::Mat debugImg = ContoursMaxImg.clone();
							cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
							for (int contIdx = 0; contIdx < contours.size(); contIdx++)
							{
								thickPoints = contours[contIdx];
								int nColorDrEn = thickPoints.size() - 1;
								for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
								{
									cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
								}
							}
							cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
							cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif
							//if (edX != 0 && stX != 0 && edY != 0 && stY != 0)
							{
								float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
								if (dist < nminVal)
								{
									nminVal = dist;
									ThminRect.left = stX;
									ThminRect.top = stY;
									ThminRect.right = edX;
									ThminRect.bottom = edY;
								}
								if ((float)(dist) < fMinth)
								{
									cv::Mat removeImg, removeImg_inv;
									cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
									cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
									removeImg = removeImg_inv & removeImg;
									ContoursMaxImg = ContoursMaxImg - removeImg;
								}
								else if (dist > nMaxVal)
								{
									nMaxVal = dist;
									ThmaxRect.left = std::min(stX, edX);
									ThmaxRect.top = std::min(stY, edY);
									ThmaxRect.right = std::max(stX, edX);
									ThmaxRect.bottom = std::max(stY, edY);
								}
							}
							maxVal = (double)nMaxVal;
							minVal = (double)nminVal;
						}
						else if ((nWndDir & 0x08) == 0x08)//right
						{
							//round
							std::vector<std::vector<cv::Point>> contours;
							std::vector<cv::Point> thickPoints;
							double dTmpA = 1.0f;//(double)(PartCenter.y - (ContoursMaxImg.rows - 1)) / (PartCenter.x - (ContoursMaxImg.cols - 1));
							//double dTmpB = -dTmpA * (ContoursMaxImg.cols - 1) + (ContoursMaxImg.rows - 1);
							for (int nBlobIdx = nLabel; nBlobIdx < nLabel + 1; nBlobIdx++)
							{
								for (int r = vBlobRects[nBlobIdx].top; r < vBlobRects[nBlobIdx].bottom; r++)
								{
									double dTmpB = -dTmpA * (vBlobRects[nBlobIdx].right - 1) + r;
									for (int c = vBlobRects[nBlobIdx].right - 1; c >= vBlobRects[nBlobIdx].left; c--)
									{
										int y = dTmpA * c + dTmpB;
										if (y < 0 || y >= vBlobRects[nBlobIdx].bottom)
											continue;
										UCHAR* uPtr = LabelImg.ptr(y);
										if (uPtr[c] == pLebel[nLabel])
										{
											thickPoints.push_back(cv::Point(c, y));
											break;
										}
									}
								}
								for (int c = vBlobRects[nBlobIdx].right - 1; c >= vBlobRects[nBlobIdx].left; c--)
								{
									//double dTmpA = (PartCenter.y - ContoursMaxImg.rows - 1) / (PartCenter.x - c);
									double dTmpB = -dTmpA * c + (vBlobRects[nBlobIdx].bottom - 1);
									for (int r = vBlobRects[nBlobIdx].bottom - 1; r >= vBlobRects[nBlobIdx].top; r--)
									{
										int x = (r - dTmpB) / dTmpA;
										if (x < 0 || x >= vBlobRects[nBlobIdx].right)
											continue;
										UCHAR* uPtr = LabelImg.ptr(r);
										if (uPtr[x] == pLebel[nLabel])
										{
											thickPoints.push_back(cv::Point(x, r));
											break;
										}
									}
								}
								contours.push_back(thickPoints);
								thickPoints.clear();
							}
							std::vector<cv::Point> dstPo = thickness(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), pLebel[nLabel]);
							int stX = dstPo[0].x;
							int stY = dstPo[0].y;
							int edX = dstPo[1].x;
							int edY = dstPo[1].y;
#if _DEBUG
							cv::Mat debugImg = ContoursMaxImg.clone();
							cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
							for (int contIdx = 0; contIdx < contours.size(); contIdx++)
							{
								thickPoints = contours[contIdx];
								int nColorDrEn = thickPoints.size() - 1;
								for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
								{
									cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
								}
							}
							cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
							cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif
							//if (edX != 0 && stX != 0 && edY != 0 && stY != 0)
							{
								float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
								if (dist < nminVal)
								{
									nminVal = dist;
									ThminRect.left = stX;
									ThminRect.top = stY;
									ThminRect.right = edX;
									ThminRect.bottom = edY;
								}
								if ((float)(dist) < fMinth)
								{
									cv::Mat removeImg, removeImg_inv;
									cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
									cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
									removeImg = removeImg_inv & removeImg;
									ContoursMaxImg = ContoursMaxImg - removeImg;
								}
								else if (dist > nMaxVal)
								{
									nMaxVal = dist;
									ThmaxRect.left = std::min(stX, edX);
									ThmaxRect.top = std::min(stY, edY);
									ThmaxRect.right = std::max(stX, edX);
									ThmaxRect.bottom = std::max(stY, edY);
								}
							}
							maxVal = (double)nMaxVal;
							minVal = (double)nminVal;
						}
					}

				}
				else
				{
					cv::Mat ThiningValue;
					cv::Mat distTrancf;
					double MinValTmp, MaxValTmp;
					cv::distanceTransform(LabelImg, distTrancf, cv::DIST_L2, 3);
					distTrancf.copyTo(ThiningValue, ThiningImg);
					cv::minMaxLoc(distTrancf, &MinValTmp, &MaxValTmp, &MinPo, &MaxPo);
					MinValTmp = MaxValTmp;
					MinPo = MaxPo;
					for (int r = 0; r < ThiningValue.rows; r++)
					{
						float* fPtr = ThiningValue.ptr<float>(r);
						for (int c = 0; c < ThiningValue.cols; c++)
						{
							if (fPtr[c] > 0 && fPtr[c] < MinValTmp)
							{
								MinValTmp = fPtr[c];
								MinPo.x = c;
								MinPo.y = r;
							}
						}
					}
					maxVal = std::roundf(MaxValTmp * 2 + 1);
					minVal = std::roundf(MinValTmp * 2 + 1);
					int nminVal = (int)std::ceil(MinValTmp);
					int nMaxVal = (int)std::ceil(MaxValTmp);
					//rect표현1 simple
					ThminRect.left = MinPo.x - nminVal;
					ThminRect.top = MinPo.y - nminVal;
					ThminRect.right = MinPo.x + nminVal;
					ThminRect.bottom = MinPo.y + nminVal;

					ThmaxRect.left = MaxPo.x - nMaxVal;
					ThmaxRect.top = MaxPo.y - nMaxVal;
					ThmaxRect.right = MaxPo.x + nMaxVal;
					ThmaxRect.bottom = MaxPo.y + nMaxVal;
				}
			}

		}
		nLabel++;
	}

#ifdef Thining
	g_pMManager->pem_delete(ThiningBuf, true);
#endif
	return nCntBlob;
}


int CPInsp_AlgoBlob::InspThickness2(AlgoBlob* pAlgoBlob, UCHAR* pUcImgSrc, UCHAR* pUcImgBlob, UCHAR* ucArrBlobDst_except, int nImgWidth, int nImgHeight, int nCntBlobExcept, double dArea, double dArea_except, CRect rcBlob, float fWndCenterX, float fWndCenterY, float fWndWidth, float fWndLength, RECT& ThminRect, RECT& ThmaxRect, double& minVal, double& maxVal, RstAlgoBlob* sRstAlgo,bool bTeach)
{
	int nCntBlob(0);
	double dResolX = m_resolX;
	double dResolY = m_resolY;
	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}
	int nMinBlobArea = pAlgoBlob->m_nMinBlobArea;
	double dAreaPix = pAlgoBlob->m_dAreaCurrent / dResolX / dResolY;
	double dAreaWPix = pAlgoBlob->m_dTeachWidth / dResolX;
	double dAreaHPix = pAlgoBlob->m_dTeachLength / dResolY;
	bool bIsTop = fWndCenterY - fWndLength <= 0 ? true : false;
	bool bIsBot = fWndCenterY + fWndLength >= 0 ? true : false;
	cv::Mat ContoursMaxImg(nImgHeight, nImgWidth, CV_8UC1, pUcImgBlob);
	UCHAR* ucArrBlobDst = pUcImgBlob;
	cv::Point poConnerLt(0, 0), poConnerRb(nImgWidth - 1, nImgHeight - 1);
	int nWndDir = 0;
	if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
	{
		//좌측
		nWndDir += 0x04;
	}
	else if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
	{
		//우측
		nWndDir += 0x08;
	}
	if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
	{
		//상측
		nWndDir += 0x01;
	}
	else if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
	{
		//하측
		nWndDir += 0x02;
	}

	int nCaseCode = convertDirectionFlag(nWndDir);
	cv::Mat GrayImg(nImgHeight, nImgWidth, CV_8UC1, pUcImgSrc);
	cv::Mat ExceptAreaImg(nImgHeight, nImgWidth, CV_8UC1, ucArrBlobDst_except);
	cv::Mat ExceptArea(nImgHeight, nImgWidth, CV_8UC1);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::dilate(ExceptAreaImg, ExceptAreaImg, kernel);
	cv::erode(ExceptAreaImg, ExceptAreaImg, kernel);
	ExceptArea = ExceptAreaImg.clone();

	int nFilter = 0;
	if (pAlgoBlob->m_bFilterIsUse)
		nFilter = pAlgoBlob->m_nFilterStepNarrow;
	bool bMultiMaxBlob = !pAlgoBlob->m_bUseBlobNG && !pAlgoBlob->m_bInspCoil && pAlgoBlob->m_byNGCnt > 0;
	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucArrBlobDst, ucArrBlobDst, nImgWidth, nImgHeight, nMinBlobArea, FALSE, pAlgoBlob->m_bFillHole, nFilter, pAlgoBlob->m_nTypeSelectBlob, -1, -1, -1, dAreaPix, dAreaWPix, dAreaHPix, -1, bMultiMaxBlob, pAlgoBlob->m_byNGCnt);


	POINTF RstPoTop;
	POINTF RstPoBot;
	POINTF RstPoLeft;
	POINTF RstPoRight;
	POINTF RstPoDiagLeft;
	POINTF RstPoDiagRight;
	cv::Mat teachImg = ExceptAreaImg.clone();
	std::vector<std::vector<cv::Point>> teachTmp;
	std::vector<cv::Vec4i> hierarchyTmp;
	cv::findContours(teachImg, teachTmp, hierarchyTmp, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	POINTF PointFail;
	PointFail.x = PointFail.y = -1;
	RstPoTop = RstPoTop = RstPoTop = RstPoTop = PointFail;
	if (bTeach)
	{
		std::vector<cv::Point> Teachcontours;
		std::vector<int> vXCnt;
		std::vector<int> vYCnt;
		vXCnt.resize(ContoursMaxImg.cols);
		vYCnt.resize(ContoursMaxImg.rows);
		std::vector<cv::Point> vtmptach;
		RstPoTop.x = RstPoBot.x = RstPoLeft.x = RstPoRight.x =RstPoDiagLeft.x=RstPoDiagRight.x= -1;
		RstPoTop.y = RstPoBot.y = RstPoLeft.y = RstPoRight.y = RstPoDiagLeft.y = RstPoDiagRight.y = -1;
		
		for (int blobIdx = 0; blobIdx < teachTmp.size(); blobIdx++)
		{
			//zero index
			if (teachTmp[blobIdx].size() > 2)
			{
				int bPoI = 1;
				//When two points are located at the edge of the image
				if (((teachTmp[blobIdx][bPoI - 1].x == 0 && teachTmp[blobIdx][bPoI].x == 0) || (teachTmp[blobIdx][bPoI - 1].x >= teachImg.cols - 1 && teachTmp[blobIdx][bPoI].x >= teachImg.cols - 1))
					|| ((teachTmp[blobIdx][bPoI - 1].y == 0 && teachTmp[blobIdx][bPoI].y == 0) || (teachTmp[blobIdx][bPoI - 1].y >= teachImg.rows - 1 && teachTmp[blobIdx][bPoI].y >= teachImg.rows - 1)))
				{
				}
				else
				{
					vtmptach.push_back(teachTmp[blobIdx][bPoI - 1]);
					vXCnt[teachTmp[blobIdx][bPoI - 1].x]++;
					vYCnt[teachTmp[blobIdx][bPoI - 1].y]++;
				}
			}
			for (int bPoI = 1; bPoI < teachTmp[blobIdx].size(); bPoI++)
			{
				//When two points are located at the edge of the image
				if (((teachTmp[blobIdx][bPoI - 1].x == 0 && teachTmp[blobIdx][bPoI].x == 0) || (teachTmp[blobIdx][bPoI - 1].x >= teachImg.cols - 1 && teachTmp[blobIdx][bPoI].x >= teachImg.cols - 1))
					|| ((teachTmp[blobIdx][bPoI - 1].y == 0 && teachTmp[blobIdx][bPoI].y == 0) || (teachTmp[blobIdx][bPoI - 1].y >= teachImg.rows - 1 && teachTmp[blobIdx][bPoI].y >= teachImg.rows - 1)))
				{
					continue;
				}
				if (teachTmp[blobIdx][bPoI - 1].x == teachTmp[blobIdx][bPoI].x)
				{
					int sty = std::min(teachTmp[blobIdx][bPoI - 1].y, teachTmp[blobIdx][bPoI].y);
					int edy = std::max(teachTmp[blobIdx][bPoI - 1].y, teachTmp[blobIdx][bPoI].y);
					for (int poY = sty; poY <= edy; poY++)
					{
						vtmptach.push_back(cv::Point(teachTmp[blobIdx][bPoI].x, poY));
						vXCnt[teachTmp[blobIdx][bPoI].x]++;
						vYCnt[poY]++;
					}
				}
				else if (teachTmp[blobIdx][bPoI - 1].y == teachTmp[blobIdx][bPoI].y)
				{
					int stx = std::min(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
					int edx = std::max(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
					for (int pox = stx; pox <= edx; pox++)
					{
						vtmptach.push_back(cv::Point(pox, teachTmp[blobIdx][bPoI].y));
						vXCnt[pox]++;
						vYCnt[teachTmp[blobIdx][bPoI].y]++;
					}
				}
				else
				{
					float fDiffx = teachTmp[blobIdx][bPoI].x - teachTmp[blobIdx][bPoI - 1].x;
					float fDiffy = teachTmp[blobIdx][bPoI].y - teachTmp[blobIdx][bPoI - 1].y;
					float fa = fDiffy / fDiffx;
					float fb = teachTmp[blobIdx][bPoI - 1].y - fa * teachTmp[blobIdx][bPoI - 1].x;
					int stx = std::min(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
					int edx = std::max(teachTmp[blobIdx][bPoI - 1].x, teachTmp[blobIdx][bPoI].x);
					for (int pox = stx; pox <= edx; pox++)
					{
						float poY = fa * pox + fb;
						vtmptach.push_back(cv::Point(pox, poY));
						vXCnt[pox]++;
						vYCnt[poY]++;
					}
				}
				if (vtmptach.size() > 0)
				{
					Teachcontours.insert(Teachcontours.end(), vtmptach.begin(), vtmptach.end());
				}
			}
		}
		int nMX(0), nMY(0), nMXCnt(0), nMYCnt(0);
		for (int n = 0; n < vXCnt.size(); n++)
		{
			if (vXCnt[n] > nMXCnt)
			{
				nMX = n;
				nMXCnt = vXCnt[n];
			}
		}
		for (int n = 0; n < vYCnt.size(); n++)
		{
			if (vYCnt[n] > nMYCnt)
			{
				nMY = n;
				nMYCnt = vYCnt[n];
			}
		}
		if ((nWndDir & 0x01) == 0x01)//top
		{
			int nCntBlack = 0;
			for (int n = ContoursMaxImg.rows - 1; n > 0; n--)
			{
				int nWPixel = 0;
				for (int ncolIdx = nMX - 5; ncolIdx < (nMX + 5); ncolIdx++)
				{
					if (ncolIdx < 0) continue;
					if (ContoursMaxImg.ptr(n)[ncolIdx] > 0) nWPixel++;
				}
				if (nWPixel > 0)
				{
					RstPoTop.y = n;
					nCntBlack = 0;
					if (RstPoBot.y == -1)
						RstPoBot.y = n;
				}
				else if (RstPoTop.y > -1)
					nCntBlack++;
				if (nCntBlack > 5)
					break;
			}
			RstPoTop.x = RstPoBot.x = nMX;
			if ((nWndDir & 0x04) == 0x04)//left
			{
				int nCntBlack = 0;
				for (int n = ContoursMaxImg.cols - 1; n > 0; n--)
				{
					int nWPixel = 0;
					for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
					{
						if (nrowIdx < 0) continue;
						if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
					}
					if (nWPixel > 0)
					{
						RstPoLeft.x = n;
						nCntBlack = 0;
						if (RstPoRight.x == -1)
							RstPoRight.x = n;
					}
					else if (RstPoLeft.x > -1)
						nCntBlack++;
					if (nCntBlack > 5)
						break;
				}
				RstPoRight.y = RstPoLeft.y = nMY;
				float fy = RstPoLeft.y - RstPoTop.y;
				float fx = RstPoLeft.x - RstPoTop.x;
				float fTermX = fx / 3;
				float fTermY = fy / 3;
				RstPoDiagLeft.x = RstPoTop.x + fTermX;
				RstPoDiagLeft.y = RstPoTop.y + fTermY;
				RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
				RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
				cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
				for (int r = 0; r < RstPoDiagLeft.y; r++)
				{
					uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y - r);
					if ((RstPoDiagLeft.x - r) >= 0 && uPtr[((int)RstPoDiagLeft.x - r)] > 0)
					{
						pTmp.x = RstPoDiagLeft.x - r;
						pTmp.y = RstPoDiagLeft.y - r;
						break;
					}
				}
				RstPoDiagLeft.x = pTmp.x;
				RstPoDiagLeft.y = pTmp.y;

				pTmp.x = RstPoDiagRight.x;
				pTmp.y = RstPoDiagRight.y;
				for (int r = 0; r < RstPoDiagRight.y; r++)
				{
					uchar* uPtr = teachImg.ptr(RstPoDiagRight.y - r);
					if ((RstPoDiagRight.x - r) >= 0 && uPtr[((int)RstPoDiagRight.x - r)] > 0)
					{
						pTmp.x = RstPoDiagRight.x - r;
						pTmp.y = RstPoDiagRight.y - r;
						break;
					}
				}
				RstPoDiagRight.x = pTmp.x;
				RstPoDiagRight.y = pTmp.y;
			}
			else if ((nWndDir & 0x08) == 0x08)//right
			{
				int nCntBlack = 0;
				for (int n = 0; n < ContoursMaxImg.cols; n++)
				{
					int nWPixel = 0;
					for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
					{
						if (nrowIdx < 0) continue;
						if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
					}
					if (nWPixel > 0)
					{
						RstPoRight.x = n;
						nCntBlack = 0;
						if (RstPoLeft.x == -1)
							RstPoLeft.x = n;
					}
					else if (RstPoRight.x > -1)
						nCntBlack++;
					if (nCntBlack > 5)
						break;
				}
				RstPoRight.y = RstPoLeft.y = nMY;
				float fy = RstPoTop.y - RstPoRight.y;
				float fx = RstPoTop.x - RstPoRight.x;
				float fTermX = fx / 3;
				float fTermY = fy / 3;
				RstPoDiagLeft.x = RstPoRight.x + fTermX;
				RstPoDiagLeft.y = RstPoRight.y + fTermY;
				RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
				RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
				cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
				for (int r = 0; r < RstPoDiagLeft.y; r++)
				{
					uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y - r);
					if ((RstPoDiagLeft.x + r) < teachImg.cols && uPtr[((int)RstPoDiagLeft.x + r)] > 0)
					{
						pTmp.x = RstPoDiagLeft.x + r;
						pTmp.y = RstPoDiagLeft.y - r;
						break;
					}
				}
				RstPoDiagLeft.x = pTmp.x;
				RstPoDiagLeft.y = pTmp.y;

				pTmp.x = RstPoDiagRight.x;
				pTmp.y = RstPoDiagRight.y;
				for (int r = 0; r < RstPoDiagRight.y; r++)
				{
					uchar* uPtr = teachImg.ptr(RstPoDiagRight.y - r);
					if ((RstPoDiagRight.x + r) < teachImg.cols && uPtr[((int)RstPoDiagRight.x + r)] > 0)
					{
						pTmp.x = RstPoDiagRight.x + r;
						pTmp.y = RstPoDiagRight.y - r;
						break;
					}
				}
				RstPoDiagRight.x = pTmp.x;
				RstPoDiagRight.y = pTmp.y;
			}
		}
		else if ((nWndDir & 0x02) == 0x02)//bot
		{
			int nCntBlack = 0;
			for (int n = 0; n < ContoursMaxImg.rows; n++)
			{
				int nWPixel = 0;
				for (int ncolIdx = nMX - 5; ncolIdx < (nMX + 5); ncolIdx++)
				{
					if (ncolIdx < 0) continue;
					if (ContoursMaxImg.ptr(n)[ncolIdx] > 0) nWPixel++;
				}
				if (nWPixel > 0)
				{
					RstPoBot.y = n;
					nCntBlack = 0;
					if (RstPoTop.y == -1)
						RstPoTop.y = n;
				}
				else if (RstPoBot.y > -1)
					nCntBlack++;
				if (nCntBlack > 5)
					break;
			}
			RstPoTop.x = RstPoBot.x = nMX;
			if ((nWndDir & 0x04) == 0x04)//left
			{
				int nCntBlack = 0;
				for (int n = ContoursMaxImg.cols - 1; n > 0; n--)
				{
					int nWPixel = 0;
					for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
					{
						if (nrowIdx < 0) continue;
						if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
					}
					if (nWPixel > 0)
					{
						RstPoLeft.x = n;
						nCntBlack = 0;
						if (RstPoRight.x == -1)
							RstPoRight.x = n;
					}
					else if (RstPoLeft.x > -1)
						nCntBlack++;
					if (nCntBlack > 5)
						break;
				}
				RstPoRight.y = RstPoLeft.y = nMY;
				float fy = RstPoLeft.y - RstPoBot.y;
				float fx = RstPoLeft.x - RstPoBot.x;
				float fTermX = fx / 3;
				float fTermY = fy / 3;
				RstPoDiagLeft.x = RstPoBot.x + fTermX;
				RstPoDiagLeft.y = RstPoBot.y + fTermY;
				RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
				RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
				cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
				int nEdy = teachImg.rows - RstPoDiagLeft.y;
				for (int r = 0; r < nEdy; r++)
				{
					uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y + r);
					if ((RstPoDiagLeft.x - r) >= 0 && uPtr[((int)RstPoDiagLeft.x - r)] > 0)
					{
						pTmp.x = RstPoDiagLeft.x - r;
						pTmp.y = RstPoDiagLeft.y + r;
						break;
					}
				}
				RstPoDiagLeft.x = pTmp.x;
				RstPoDiagLeft.y = pTmp.y;

				pTmp.x = RstPoDiagRight.x;
				pTmp.y = RstPoDiagRight.y;
				nEdy = teachImg.rows - RstPoDiagRight.y;
				for (int r = 0; r < nEdy; r++)
				{
					uchar* uPtr = teachImg.ptr(RstPoDiagRight.y + r);
					if ((RstPoDiagRight.x - r) >= 0 && uPtr[((int)RstPoDiagRight.x - r)] > 0)
					{
						pTmp.x = RstPoDiagRight.x - r;
						pTmp.y = RstPoDiagRight.y + r;
						break;
					}
				}
				RstPoDiagRight.x = pTmp.x;
				RstPoDiagRight.y = pTmp.y;
			}
			else if ((nWndDir & 0x08) == 0x08)//right
			{
				int nCntBlack = 0;
				for (int n = 0; n < ContoursMaxImg.cols; n++)
				{
					int nWPixel = 0;
					for (int nrowIdx = nMY - 5; nrowIdx < (nMY + 5); nrowIdx++)
					{
						if (nrowIdx < 0) continue;
						if (ContoursMaxImg.ptr(nrowIdx)[n] > 0) nWPixel++;
					}
					if (nWPixel > 0)
					{
						RstPoRight.x = n;
						nCntBlack = 0;
						if (RstPoLeft.x == -1)
							RstPoLeft.x = n;
					}
					else if (RstPoRight.x > -1)
						nCntBlack++;
					if (nCntBlack > 5)
						break;
				}
				RstPoRight.y = RstPoLeft.y = nMY;
				float fy = RstPoBot.y - RstPoRight.y;
				float fx = RstPoBot.x - RstPoRight.x;
				float fTermX = fx / 3;
				float fTermY = fy / 3;
				RstPoDiagLeft.x = RstPoRight.x + fTermX;
				RstPoDiagLeft.y = RstPoRight.y + fTermY;
				RstPoDiagRight.x = RstPoDiagLeft.x + fTermX;
				RstPoDiagRight.y = RstPoDiagLeft.y + fTermY;
				cv::Point pTmp(RstPoDiagLeft.x, RstPoDiagLeft.y);
				int nEdy = teachImg.rows - RstPoDiagLeft.y;
				for (int r = 0; r < nEdy; r++)
				{
					uchar* uPtr = teachImg.ptr(RstPoDiagLeft.y + r);
					if ((RstPoDiagLeft.x + r) < teachImg.cols && uPtr[((int)RstPoDiagLeft.x + r)] > 0)
					{
						pTmp.x = RstPoDiagLeft.x + r;
						pTmp.y = RstPoDiagLeft.y + r;
						break;
					}
				}
				RstPoDiagLeft.x = pTmp.x;
				RstPoDiagLeft.y = pTmp.y;

				pTmp.x = RstPoDiagRight.x;
				pTmp.y = RstPoDiagRight.y;
				nEdy = teachImg.rows - RstPoDiagRight.y;
				for (int r = 0; r < nEdy; r++)
				{
					uchar* uPtr = teachImg.ptr(RstPoDiagRight.y + r);
					if ((RstPoDiagRight.x + r) < teachImg.cols && uPtr[((int)RstPoDiagRight.x + r)] > 0)
					{
						pTmp.x = RstPoDiagRight.x + r;
						pTmp.y = RstPoDiagRight.y + r;
						break;
					}
				}
				RstPoDiagRight.x = pTmp.x;
				RstPoDiagRight.y = pTmp.y;
			}
		}
		if (RstPoLeft.x < RstPoDiagLeft.x)//right
		{
			if (RstPoLeft.y > RstPoDiagLeft.y)//bot
			{
				sRstAlgo->Po1 = RstPoLeft;
				sRstAlgo->Po2 = RstPoRight;
				sRstAlgo->Po3 = RstPoDiagLeft;
				sRstAlgo->Po4 = RstPoDiagRight;
				sRstAlgo->Po5 = RstPoBot;
				sRstAlgo->Po6 = RstPoTop;
			}
			else//top
			{
				sRstAlgo->Po1 = RstPoLeft;
				sRstAlgo->Po2 = RstPoRight;
				sRstAlgo->Po3 = RstPoDiagLeft;
				sRstAlgo->Po4 = RstPoDiagRight;
				sRstAlgo->Po5 = RstPoTop;
				sRstAlgo->Po6 = RstPoBot;
			}
		}
		else//left
		{
			if (RstPoLeft.y > RstPoDiagLeft.y)//bot
			{
				sRstAlgo->Po1 = RstPoRight;
				sRstAlgo->Po2 = RstPoLeft;
				sRstAlgo->Po3 = RstPoDiagRight;
				sRstAlgo->Po4 = RstPoDiagLeft;
				sRstAlgo->Po5 = RstPoBot;
				sRstAlgo->Po6 = RstPoTop;
			}
			else//top
			{
				sRstAlgo->Po1 = RstPoRight;
				sRstAlgo->Po2 = RstPoLeft;
				sRstAlgo->Po3 = RstPoDiagRight;
				sRstAlgo->Po4 = RstPoDiagLeft;
				sRstAlgo->Po5 = RstPoTop;
				sRstAlgo->Po6 = RstPoBot;
			}
		}
		pAlgoBlob->Po1 = sRstAlgo->Po1;
		pAlgoBlob->Po2 = sRstAlgo->Po2;
		pAlgoBlob->Po3 = sRstAlgo->Po3;
		pAlgoBlob->Po4 = sRstAlgo->Po4;
		pAlgoBlob->Po5 = sRstAlgo->Po5;
		pAlgoBlob->Po6 = sRstAlgo->Po6;
		pAlgoBlob->m_nPoCnt = 6;
#if _DEBUG
		cv::Mat TechingcontoursMat = ContoursMaxImg.clone();
		cv::cvtColor(TechingcontoursMat, TechingcontoursMat, cv::COLOR_GRAY2BGR);
		for (int contIdx = 0; contIdx < Teachcontours.size(); contIdx++)
		{
			cv::line(TechingcontoursMat, Teachcontours[contIdx], Teachcontours[contIdx], cv::Scalar(0, 0, 255));
		}
		cv::imwrite("D:\\TechingcontoursMat.bmp", TechingcontoursMat);
		TechingcontoursMat = ContoursMaxImg.clone();
		cv::cvtColor(TechingcontoursMat, TechingcontoursMat, cv::COLOR_GRAY2BGR);
		cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po1.x, pAlgoBlob->Po1.y), cv::Point(pAlgoBlob->Po2.x, pAlgoBlob->Po2.y), cv::Scalar(0, 0, 255));
		cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po3.x, pAlgoBlob->Po3.y), cv::Point(pAlgoBlob->Po4.x, pAlgoBlob->Po4.y), cv::Scalar(0, 0, 255));
		cv::line(TechingcontoursMat, cv::Point(pAlgoBlob->Po5.x, pAlgoBlob->Po5.y), cv::Point(pAlgoBlob->Po6.x, pAlgoBlob->Po6.y), cv::Scalar(0, 0, 255));
		cv::imwrite("D:\\TechingcontoursMat2.bmp", TechingcontoursMat);
#endif
	}

	if (pAlgoBlob->m_nExceptAreaCal == 1)//and
	{
		//좌측
		if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
		{
			std::vector<cv::Point> vLeft;
			std::vector<int> vnLeftCnt;
			vnLeftCnt.resize(ContoursMaxImg.cols);
			for (int r = 0; r < ContoursMaxImg.rows; r++)
			{
				UCHAR* uPtrex = ExceptAreaImg.ptr(r);
				for (int c = 1; c < ContoursMaxImg.cols; c++)
				{
					if (uPtrex[c] > 0)
					{
						vLeft.push_back(cv::Point(r, c));
						vnLeftCnt[c]++;
						cv::line(ExceptArea, cv::Point(0, r), cv::Point(c - 1, r), cv::Scalar(0));
						break;
					}
				}
			}
			if (vLeft.size() > 3)
			{
				int nMxCnt = 0;
				int nMxIdx = 0;
				for (int c = 1; c < vnLeftCnt.size(); c++)//끝에 완전히 붙는경우 제외
				{
					if (nMxCnt < vnLeftCnt[c])
					{
						nMxCnt = vnLeftCnt[c];
						nMxIdx = c;
					}
				}
				cv::Rect whiteRect(nMxIdx, 0, ExceptArea.cols - nMxIdx, ExceptArea.rows);
				cv::Rect blackRect(0, 0, nMxIdx, ExceptArea.rows);
				for (int c = 0; c < vLeft.size(); c++)
				{
					if (std::abs(vLeft[c].y - nMxIdx) < 5)
					{
						blackRect.y = vLeft[c].x;
						break;
					}
				}
				for (int c = vLeft.size() - 1; c >= 0; c--)
				{
					if (std::abs(vLeft[c].y - nMxIdx) < 5)
					{
						blackRect.height = vLeft[c].x - blackRect.y + 1;
						break;
					}
				}
				std::vector<cv::Point> vLine;
				int nStLineX = vLeft[vLeft.size() - 1].y;
				int nEndLineX = 0;
				for (int c = 0; c < vLeft.size(); c++)
				{
					if (std::abs(vLeft[c].y - nMxIdx) < 5)
					{
						vLine.push_back(vLeft[c]);
						if (vLeft[c].y == nMxIdx)
						{
							nEndLineX = vLeft[c].x;
							if (nStLineX > vLeft[c].x)
								nStLineX = vLeft[c].x;
						}
					}
				}
				if (vLine.size() == 0)
					vLine = vLeft;
				if (nEndLineX == 0)
					nEndLineX = vLine[vLine.size() - 1].x;
				if (nStLineX == vLeft[vLeft.size() - 1].y)
					nStLineX = vLine[0].x;
				for (int c = 0; c < vLine.size(); c++)
				{
					if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
					{
						vLine.erase(vLine.begin() + c);
						c--;
					}
				}
				float a, b;
				if (!bIsBot && !bIsTop)
				{
					double dA, dB;
					double dAngle = 0;
					int nMeasureDirection = Left_Line;
					bool bIsHorizon = false;
					POINTF poDrawLine[2];
					memset(poDrawLine, 0, sizeof(POINTF) * 2);
					int nLineLength = 0;
					if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
						m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
					a = (float)dA;
					b = (float)dB;
				}
				else
					m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

#if LineDraw
				if (a != 0)
				{
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						for (int c = 0; c < vLine[0].x; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
						}
					}
					for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
					{
						for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.rows; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
						}
					}
				}
#else

				std::vector<cv::Point> vDrawPo;
				for (int c = 0; c < vLeft.size(); c++)
				{
					if (vLine[0].x > vLeft[c].x)
						continue;
					else if (vLine[vLine.size() - 1].x < vLeft[c].x)
						break;
					int nStdX = (int)std::roundf(vLeft[c].x * a + b);
					if (std::abs(nStdX - vLeft[c].y) > 3)
					{
						vDrawPo.push_back(cv::Point(vLeft[c].x, nStdX));
					}
					else
					{
						vDrawPo.push_back(cv::Point(vLeft[c].x, vLeft[c].y));
					}
				}
				std::vector<cv::Point> vBlank;
				for (int l = 1; l < vDrawPo.size(); l++)
				{
					if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
					{
						float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
						float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
						for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
						{
							vBlank.push_back(cv::Point(x, fTmpA * x + fTmpB));
						}
					}
				}

				if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
				{
					b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
				}
				if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
				{
					for (int c = 0; c < vLeft[0].x; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(0));
					}
				}
				//for (int c = 0; c < vLeft.size(); c++)
				//{
				//	//int nStdX = (int)std::roundf(c*a + b);
				//	cv::line(ExceptArea, cv::Point(0, vLeft[c].x), cv::Point(vLeft[c].y - 1, vLeft[c].x), cv::Scalar(0));
				//	cv::line(ExceptArea, cv::Point(vLeft[c].y, vLeft[c].x), cv::Point(ExceptArea.cols, vLeft[c].x), cv::Scalar(255));
				//}
				if (pAlgoBlob->m_nThickDir != 1)
				{
					for (int c = 0; c < vDrawPo.size(); c++)
					{
						//int nStdX = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(0, vDrawPo[c].x), cv::Point(vDrawPo[c].y - 1, vDrawPo[c].x), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(vDrawPo[c].y, vDrawPo[c].x), cv::Point(ExceptArea.cols, vDrawPo[c].x), cv::Scalar(255));
					}
					for (int b = 0; b < vBlank.size(); b++)
					{
						cv::line(ExceptArea, cv::Point(0, vBlank[b].x), cv::Point(vBlank[b].y - 1, vBlank[b].x), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(vBlank[b].y, vBlank[b].x), cv::Point(ExceptArea.cols, vBlank[b].x), cv::Scalar(255));
					}
					if (/*(pAlgoBlob->m_nThickDir != 1) &&*/ bIsTop)
					{
						for (int c = vLeft[vLeft.size() - 1].x; c < ExceptArea.rows; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(0));
						}
					}
				}
#endif
				int nLeft = (int)std::roundf(vLine[0].x * a + b);
				poConnerLt.x = nLeft > 0 ? nLeft : 0;
				poConnerLt.y = vLine[0].x;
				nLeft = (int)std::roundf(vLine[vLine.size() - 1].x * a + b);
				poConnerRb.x = nLeft > 0 ? nLeft : 0;
				poConnerRb.y = vLine[vLine.size() - 1].x;
			}
		}
		//우측
		else if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
		{
			std::vector<cv::Point> vRight;
			std::vector<int> vnRightCnt;
			vnRightCnt.resize(ContoursMaxImg.cols);
			for (int r = 0; r < ContoursMaxImg.rows; r++)
			{
				UCHAR* uPtrex = ExceptAreaImg.ptr(r);
				for (int c = ContoursMaxImg.cols - 1; c >= 0; c--)
				{
					if (uPtrex[c] > 0)
					{
						vRight.push_back(cv::Point(r, c));
						vnRightCnt[c]++;
						cv::line(ExceptArea, cv::Point(c + 1, r), cv::Point(ExceptArea.cols - 1, r), cv::Scalar(0));
						break;
					}
				}
			}
			if (vRight.size() > 3)
			{
				int nMxCnt = 0;
				int nMxIdx = 0;
				for (int c = 0; c < vnRightCnt.size() - 1; c++)//끝에 완전히 붙는경우 제외
				{
					if (nMxCnt < vnRightCnt[c])
					{
						nMxCnt = vnRightCnt[c];
						nMxIdx = c;
					}
				}
				cv::Rect blackRect(nMxIdx, 0, ExceptArea.cols - nMxIdx, 0);
				cv::Rect whiteRect(0, 0, nMxIdx, ExceptArea.rows);
				for (int c = 0; c < vRight.size(); c++)
				{
					if (std::abs(vRight[c].y - nMxIdx) < 5)
					{
						blackRect.y = vRight[c].x;
						break;
					}
				}
				for (int c = vRight.size() - 1; c >= blackRect.x; c--)
				{
					if (std::abs(vRight[c].y - nMxIdx) < 5)
					{
						blackRect.height = vRight[c].x - blackRect.y + 1;
						break;
					}
				}
				std::vector<cv::Point> vLine;
				int nStLineX = vRight[vRight.size() - 1].y;
				int nEndLineX = 0;
				for (int c = 0; c < vRight.size(); c++)
				{
					if (std::abs(vRight[c].y - nMxIdx) < 5)
					{
						vLine.push_back(vRight[c]);
						if (vRight[c].y == nMxIdx)
						{
							nEndLineX = vRight[c].x;
							if (nStLineX > vRight[c].x)
								nStLineX = vRight[c].x;
						}
					}
				}
				if (vLine.size() == 0)
					vLine = vRight;
				if (nEndLineX == 0)
					nEndLineX = vLine[vLine.size() - 1].x;
				if (nStLineX == vRight[vRight.size() - 1].y)
					nStLineX = vLine[0].x;
				for (int c = 0; c < vLine.size(); c++)
				{
					if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
					{
						vLine.erase(vLine.begin() + c);
						c--;
					}
				}

				float a, b;
				if (!bIsBot && !bIsTop)
				{
					double dA, dB;
					double dAngle = 0;
					int nMeasureDirection = Right_Line;
					bool bIsHorizon = false;
					POINTF poDrawLine[2];
					memset(poDrawLine, 0, sizeof(POINTF) * 2);
					int nLineLength = 0;
					if (nMeasureDirection == Center_Line || nMeasureDirection == Right_Line || nMeasureDirection == Right_Line)
						m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
					a = (float)dA;
					b = (float)dB;
				}
				else
					m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

				std::vector<cv::Point> vDrawPo;
				for (int c = 0; c < vRight.size(); c++)
				{
					if (vLine[0].x > vRight[c].x)
						continue;
					else if (vLine[vLine.size() - 1].x < vRight[c].x)
						break;
					int nStdX = (int)std::roundf(vRight[c].x * a + b);
					if (std::abs(nStdX - vRight[c].y) > 3)
					{
						vDrawPo.push_back(cv::Point(vRight[c].x, nStdX));
					}
					else
					{
						vDrawPo.push_back(cv::Point(vRight[c].x, vRight[c].y));
					}
				}
				std::vector<cv::Point> vBlank;
				for (int l = 1; l < vDrawPo.size(); l++)
				{
					if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
					{
						float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
						float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
						for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
						{
							vBlank.push_back(cv::Point(x, fTmpA * x + fTmpB));
						}
					}
				}
				if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
				{
					b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
				}
#if LineDraw
				if (a != 0)
				{
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						for (int c = 0; c < vLine[0].x; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
					}
					for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
					{
						for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.rows; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
					}
				}
#else
				if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
				{
					for (int c = 0; c < vRight[0].x; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
					}
				}
				//for (int c = 0; c < vRight.size(); c++)
				//{
				//	//int nStdX = (int)std::roundf(c*a + b);
				//	cv::line(ExceptArea, cv::Point(0, vRight[c].x), cv::Point(vRight[c].y, vRight[c].x), cv::Scalar(255));
				//	cv::line(ExceptArea, cv::Point(vRight[c].y + 1, vRight[c].x), cv::Point(ExceptArea.cols, vRight[c].x), cv::Scalar(0));
				//}
				if (pAlgoBlob->m_nThickDir != 1)
				{
					for (int c = 0; c < vDrawPo.size(); c++)
					{
						cv::line(ExceptArea, cv::Point(0, vDrawPo[c].x), cv::Point(vDrawPo[c].y, vDrawPo[c].x), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(vDrawPo[c].y + 1, vDrawPo[c].x), cv::Point(ExceptArea.cols, vDrawPo[c].x), cv::Scalar(0));
					}
					for (int b = 0; b < vBlank.size(); b++)
					{
						//int nStdX = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(0, vBlank[b].x), cv::Point(vBlank[b].y, vBlank[b].x), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(vBlank[b].y + 1, vBlank[b].x), cv::Point(ExceptArea.cols, vBlank[b].x), cv::Scalar(0));
					}
					if (bIsTop)
					{
						for (int c = vRight[vRight.size() - 1].x; c < ExceptArea.rows; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
					}
				}
#endif
				int nRight = (int)std::roundf(vLine[0].x * a + b);
				poConnerLt.x = nRight > 0 ? nRight : 0;
				poConnerLt.y = vLine[0].x;
				nRight = (int)std::roundf(vLine[vLine.size() - 1].x * a + b);
				poConnerRb.x = nRight > 0 ? nRight : 0;
				poConnerRb.y = vLine[vLine.size() - 1].x;
			}
		}
		//상측
		if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
		{
			std::vector<cv::Point> vTop;
			std::vector<int> vnTopCnt;
			vnTopCnt.resize(ContoursMaxImg.rows);
			int nCntSt = poConnerLt.x;
			int nCntEd = ContoursMaxImg.cols;
			if ((nWndDir & 0x08) == 0x08)
			{
				nCntSt = 0;
				nCntEd = poConnerLt.x;
			}
			for (int c = nCntSt; c < nCntEd; c++)
			{
				for (int r = 1; r < ContoursMaxImg.rows; r++)
				{
					UCHAR* uPtrex = ExceptAreaImg.ptr(r);
					if (uPtrex[c] > 0)
					{
						vTop.push_back(cv::Point(c, r));
						vnTopCnt[r]++;
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, r - 1), cv::Scalar(0));
						break;
					}
				}
			}
			if (vTop.size() > 3)
			{
				int nMxCnt = 0;
				int nMxIdx = 0;
				for (int r = 1; r < vnTopCnt.size(); r++)//끝에 완전히 붙는경우 제외
				{
					if (nMxCnt < vnTopCnt[r])
					{
						nMxCnt = vnTopCnt[r];
						nMxIdx = r;
					}
				}
				cv::Rect blackRect(0, 0, ExceptArea.cols, nMxIdx);
				cv::Rect whiteRect(0, nMxIdx, ExceptArea.cols, ExceptArea.rows);
				for (int r = 0; r < vTop.size(); r++)
				{
					if (std::abs(vTop[r].y - nMxIdx) < 5)
					{
						blackRect.x = vTop[r].x;
						break;
					}
				}
				for (int r = vTop.size() - 1; r >= 0; r--)
				{
					if (std::abs(vTop[r].y - nMxIdx) < 5)
					{
						blackRect.width = vTop[r].x - blackRect.x + 1;
						break;
					}
				}
				std::vector<cv::Point> vLine;
				int nStLineX = vTop[vTop.size() - 1].y;
				int nEndLineX = 0;
				for (int c = 0; c < vTop.size(); c++)
				{
					if (std::abs(vTop[c].y - nMxIdx) < 5)
					{
						vLine.push_back(vTop[c]);
						if (vTop[c].y == nMxIdx)
						{
							nEndLineX = vTop[c].x;
							if (nStLineX > vTop[c].x)
								nStLineX = vTop[c].x;
						}
					}
				}
				if (vLine.size() == 0)
					vLine = vTop;
				if (nEndLineX == 0)
					nEndLineX = vLine[vLine.size() - 1].x;
				if (nStLineX == vTop[vTop.size() - 1].y)
					nStLineX = vLine[0].x;
				for (int c = 0; c < vLine.size(); c++)
				{
					if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
					{
						vLine.erase(vLine.begin() + c);
						c--;
					}
				}
				float a, b;
				if (pAlgoBlob->m_nThickDir == 2)
				{
					double dA, dB;
					double dAngle = 0;
					int nMeasureDirection = Left_Line;
					bool bIsHorizon = true;
					POINTF poDrawLine[2];
					memset(poDrawLine, 0, sizeof(POINTF) * 2);
					int nLineLength = 0;
					if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
						m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
					a = (float)dA;
					b = (float)dB;
				}
				else
					m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

				if ((nWndDir & 0x08) == 0x08)
				{
					b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
				}
#if LineDraw
				if (a != 0)
				{
					if ((pAlgoBlob->m_nThickDir != 2) && bIsTop)
					{
						if ((nWndDir & 0x04) == 0x04)
						{
							for (int c = vLine[0].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c * a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
							}
						}
						if ((nWndDir & 0x08) == 0x08)
						{
							for (int c = 0; c < vLine[0].x; c++)
							{
								int nStdY = (int)std::roundf(c * a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
							}
						}
					}
					for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
					{
						int nStdY = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
					}
					if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
					{
						for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
						}
					}
				}
#else
				std::vector<cv::Point> vDrawPo;
				for (int c = 0; c < vTop.size(); c++)
				{
					if (vLine[0].x > vTop[c].x)
						continue;
					else if (vLine[vLine.size() - 1].x < vTop[c].x)
						break;
					int nStdY = (int)std::roundf(vTop[c].x * a + b);
					if (std::abs(nStdY - vTop[c].y) > 3)
					{
						vDrawPo.push_back(cv::Point(vTop[c].x, nStdY));
					}
					else
					{
						vDrawPo.push_back(cv::Point(vTop[c].x, vTop[c].y));
					}
				}
				std::vector<cv::Point> vBlank;
				for (int l = 1; l < vDrawPo.size(); l++)
				{
					if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
					{
						float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
						float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
						for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
						{
							vBlank.push_back(cv::Point(x, fTmpA * x + fTmpB));
						}
					}
				}

				if ((pAlgoBlob->m_nThickDir != 2) && bIsTop)
				{
					if ((nWndDir & 0x04) == 0x04)
					{
						for (int c = vTop[vTop.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
						}
					}
					if ((nWndDir & 0x08) == 0x08)
					{
						for (int c = 0; c < vTop[0].x; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
						}
					}
				}
				//for (int c = 0; c < vTop.size(); c++)
				//{
				//	//int nStdY = (int)std::roundf(c*a + b);
				//	cv::line(ExceptArea, cv::Point(vTop[c].x, 0), cv::Point(vTop[c].x, vTop[c].y - 1), cv::Scalar(0));
				//	cv::line(ExceptArea, cv::Point(vTop[c].x, vTop[c].y), cv::Point(vTop[c].x, ExceptArea.rows), cv::Scalar(255));
				//}
				if (pAlgoBlob->m_nThickDir != 2)
				{
					for (int c = 0; c < vDrawPo.size(); c++)
					{
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, 0), cv::Point(vDrawPo[c].x, vDrawPo[c].y - 1), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, vDrawPo[c].y), cv::Point(vDrawPo[c].x, ExceptArea.rows), cv::Scalar(255));
					}
					for (int b = 0; b < vBlank.size(); b++)
					{
						//int nStdY = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(vBlank[b].x, 0), cv::Point(vBlank[b].x, vBlank[b].y - 1), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(vBlank[b].x, vBlank[b].y), cv::Point(vBlank[b].x, ExceptArea.rows), cv::Scalar(255));
					}
					if (fWndCenterX + fWndWidth <= 0)
					{
						for (int c = vTop[vTop.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
						}
					}
				}
#endif
				if ((nWndDir & 0x04) == 0x04)
				{
					poConnerRb.y = poConnerLt.y;
					int nTop = (int)std::roundf(vLine[0].x * a + b);
					poConnerRb.x = vLine[0].x;
					poConnerLt.y = nTop > 0 ? nTop : 0;
				}
				else if ((nWndDir & 0x08) == 0x08)
				{
					poConnerRb.x = poConnerLt.x;
					poConnerRb.y = poConnerLt.y;
					int nTop = (int)std::roundf(vLine[vLine.size() - 1].x * a + b);
					poConnerLt.x = vLine[vLine.size() - 1].x;
					poConnerLt.y = nTop > 0 ? nTop : 0;
				}
			}
		}
		//하측
		else if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
		{
			std::vector<cv::Point> vBot;
			std::vector<int> vnBotCnt;
			vnBotCnt.resize(ContoursMaxImg.rows);
			int nCntSt = poConnerLt.x;
			int nCntEd = ContoursMaxImg.cols;
			if ((nWndDir & 0x08) == 0x08)
			{
				nCntSt = 0;
				nCntEd = poConnerLt.x;
			}
			for (int c = nCntSt; c < nCntEd; c++)
			{
				for (int r = ContoursMaxImg.rows - 1; r >= 0; r--)
				{
					UCHAR* uPtrex = ExceptAreaImg.ptr(r);
					if (uPtrex[c] > 0)
					{
						vBot.push_back(cv::Point(c, r));
						vnBotCnt[r]++;
						cv::line(ExceptArea, cv::Point(c, r + 1), cv::Point(c, ExceptArea.rows - 1), cv::Scalar(0));
						break;
					}
				}
			}
			if (vBot.size() > 3)
			{
				int nMxCnt = 0;
				int nMxIdx = 0;
				for (int r = 0; r < vnBotCnt.size() - 1; r++)//끝에 완전히 붙는경우 제외
				{
					if (nMxCnt < vnBotCnt[r])
					{
						nMxCnt = vnBotCnt[r];
						nMxIdx = r;
					}
				}
				cv::Rect whiteRect(0, 0, ExceptArea.cols, nMxIdx);
				cv::Rect blackRect(0, nMxIdx, ExceptArea.cols, ExceptArea.rows);
				for (int r = 0; r < vBot.size(); r++)
				{
					if (std::abs(vBot[r].y - nMxIdx) < 5)
					{
						blackRect.x = vBot[r].x;
						break;
					}
				}
				for (int r = vBot.size() - 1; r >= blackRect.y; r--)
				{
					if (std::abs(vBot[r].y - nMxIdx) < 5)
					{
						blackRect.width = vBot[r].x - blackRect.x + 1;
						break;
					}
				}
				std::vector<cv::Point> vLine;
				int nStLineX = vBot[vBot.size() - 1].y;
				int nEndLineX = 0;
				for (int c = 0; c < vBot.size(); c++)
				{
					if (std::abs(vBot[c].y - nMxIdx) < 5)
					{
						vLine.push_back(vBot[c]);
						if (vBot[c].y == nMxIdx)
						{
							nEndLineX = vBot[c].x;
							if (nStLineX > vBot[c].x)
								nStLineX = vBot[c].x;
						}
					}
				}
				if (vLine.size() == 0)
					vLine = vBot;
				if (nEndLineX == 0)
					nEndLineX = vLine[vLine.size() - 1].x;
				if (nStLineX == vBot[vBot.size() - 1].y)
					nStLineX = vLine[0].x;
				for (int c = 0; c < vLine.size(); c++)
				{
					if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
					{
						vLine.erase(vLine.begin() + c);
						c--;
					}
				}
				float a, b;
				if (pAlgoBlob->m_nThickDir == 2)
				{
					double dA, dB;
					double dAngle = 0;
					int nMeasureDirection = Right_Line;
					bool bIsHorizon = true;
					POINTF poDrawLine[2];
					memset(poDrawLine, 0, sizeof(POINTF) * 2);
					int nLineLength = 0;
					if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
						m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
					a = (float)dA;
					b = (float)dB;
				}
				else
					m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

				if ((nWndDir & 0x08) == 0x08)
				{
					b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
				}
#if LineDraw
				if (a != 0)
				{
					if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
					{
						if ((nWndDir & 0x04) == 0x04)
						{
							for (int c = vLine[0].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c * a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
						else if ((nWndDir & 0x08) == 0x08)
						{
							for (int c = 0; c < vLine[0].x; c++)
							{
								int nStdY = (int)std::roundf(c * a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
					}
					for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
					{
						int nStdY = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
					}
					if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
					{
						for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
					}
				}
#else
				std::vector<cv::Point> vDrawPo;
				for (int c = 0; c < vBot.size(); c++)
				{
					if (vLine[0].x > vBot[c].x)
						continue;
					else if (vLine[vLine.size() - 1].x < vBot[c].x)
						break;
					int nStdY = (int)std::roundf(vBot[c].x * a + b);
					if (std::abs(nStdY - vBot[c].y) > 3)
					{
						vDrawPo.push_back(cv::Point(vBot[c].x, nStdY));
					}
					else
					{
						vDrawPo.push_back(cv::Point(vBot[c].x, vBot[c].y));
					}
				}
				std::vector<cv::Point> vBlank;
				for (int l = 1; l < vDrawPo.size(); l++)
				{
					if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
					{
						float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
						float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
						for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
						{
							vBlank.push_back(cv::Point(x, fTmpA * x + fTmpB));
						}
					}
				}
				if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
				{
					if ((nWndDir & 0x04) == 0x04)
					{
						for (int c = vBot[vBot.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, vBot[c].y), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
					}
					else if ((nWndDir & 0x08) == 0x08)
					{
						for (int c = 0; c < vBot[0].x; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
					}
				}
				//for (int c = 0; c < vBot.size(); c++)
				//{
				//	//int nStdY = (int)std::roundf(c*a + b);
				//	cv::line(ExceptArea, cv::Point(vBot[c].x, 0), cv::Point(vBot[c].x, vBot[c].y), cv::Scalar(255));
				//	cv::line(ExceptArea, cv::Point(vBot[c].x, vBot[c].y + 1), cv::Point(vBot[c].x, ExceptArea.rows), cv::Scalar(0));
				//}

				if (pAlgoBlob->m_nThickDir != 2)
				{
					for (int c = 0; c < vDrawPo.size(); c++)
					{
						//int nStdY = (int)std::roundf(c*a + b);
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, 0), cv::Point(vDrawPo[c].x, vDrawPo[c].y), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(vDrawPo[c].x, vDrawPo[c].y + 1), cv::Point(vDrawPo[c].x, ExceptArea.rows), cv::Scalar(0));
					}
					for (int b = 0; b < vBlank.size(); b++)
					{
						cv::line(ExceptArea, cv::Point(vBlank[b].x, 0), cv::Point(vBlank[b].x, vBlank[b].y), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(vBlank[b].x, vBlank[b].y + 1), cv::Point(vBlank[b].x, ExceptArea.rows), cv::Scalar(0));
					}
					if (/*(pAlgoBlob->m_nThickDir != 2) &&*/ fWndCenterX + fWndWidth <= 0)
					{
						for (int c = vBot[vBot.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
					}
				}
#endif
				if ((nWndDir & 0x04) == 0x04)
				{
					poConnerLt.y = poConnerRb.y;
					int nTop = (int)std::roundf(vLine[0].x * a + b);
					poConnerRb.x = vLine[0].x;
					poConnerRb.y = nTop > 0 ? nTop : 0;
				}
				else if ((nWndDir & 0x08) == 0x08)
				{
					poConnerLt.y = poConnerRb.y;
					int nTop = (int)std::roundf(vLine[vLine.size() - 1].x * a + b);
					poConnerLt.x = vLine[vLine.size() - 1].x;
					poConnerRb.y = nTop > 0 ? nTop : 0;
				}
			}
		}
		ContoursMaxImg &= ExceptArea;
	}
	else //subtract
	{
		if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
		{
			std::vector<cv::Point> vLeft;
			std::vector<int> vnLeftCnt;
			vnLeftCnt.resize(ContoursMaxImg.cols);
			//좌측
			//nWndDir += 0x04;
			for (int r = 0; r < ContoursMaxImg.rows; r++)
			{
				UCHAR* uPtr = ContoursMaxImg.ptr(r);
				int nmxC = ContoursMaxImg.cols;
				int nmnC = 0;
				for (int c = ContoursMaxImg.cols - 1; c > 0; c--)
				{
					if (nmxC == ContoursMaxImg.cols && uPtr[c] > 0)
					{
						nmxC = c;
					}
					else if (nmxC != ContoursMaxImg.cols && uPtr[c] == 0)
					{
						nmnC = c;
						break;
					}
				}
				if (nmxC == ContoursMaxImg.cols)
					nmxC = ContoursMaxImg.cols - 1;
				UCHAR* uPtrex = ExceptAreaImg.ptr(r);
				for (int c = nmxC; c >= nmnC; c--)
				{
					if (uPtrex[c] > 0)
					{
						vLeft.push_back(cv::Point(r, c));
						vnLeftCnt[c]++;
						nmnC = c;
						break;
					}
				}
				if (nmxC != ContoursMaxImg.cols - 1)
				{
					cv::line(ExceptArea, cv::Point(nmnC, r), cv::Point(ExceptArea.cols - 1, r), cv::Scalar(0));
				}
			}
			int nMinVecCnt = rcBlob.Height() / 3;
			vLeft.clear();
			if (vLeft.size() > nMinVecCnt)
			{
				int nMxCnt = 0;
				int nMxIdx = 0;
				for (int c = 0; c < vnLeftCnt.size() - 1; c++)//끝에 완전히 붙는경우 제외
				{
					if (nMxCnt < vnLeftCnt[c])
					{
						nMxCnt = vnLeftCnt[c];
						nMxIdx = c;
					}
				}
				cv::Rect blackRect(nMxIdx, 0, ExceptArea.cols - nMxIdx, 0);
				cv::Rect whiteRect(0, 0, nMxIdx, ExceptArea.rows);
				for (int c = 0; c < vLeft.size(); c++)
				{
					if (std::abs(vLeft[c].y - nMxIdx) < 5)
					{
						blackRect.y = vLeft[c].x;
						break;
					}
				}
				for (int c = vLeft.size() - 1; c >= blackRect.x; c--)
				{
					if (std::abs(vLeft[c].y - nMxIdx) < 5)
					{
						blackRect.height = vLeft[c].x - blackRect.y + 1;
						break;
					}
				}
				std::vector<cv::Point> vLine;
				int nStLineX = vLeft[vLeft.size() - 1].y;
				int nEndLineX = 0;
				for (int c = 0; c < vLeft.size(); c++)
				{
					if (std::abs(vLeft[c].y - nMxIdx) < 5)
					{
						vLine.push_back(vLeft[c]);
						if (vLeft[c].y == nMxIdx)
						{
							nEndLineX = vLeft[c].x;
							if (nStLineX > vLeft[c].x)
								nStLineX = vLeft[c].x;
						}
					}
				}
				if (vLine.size() == 0)
					vLine = vLeft;
				if (nEndLineX == 0)
					nEndLineX = vLine[vLine.size() - 1].x;
				if (nStLineX == vLeft[vLeft.size() - 1].y)
					nStLineX = vLine[0].x;
				for (int c = 0; c < vLine.size(); c++)
				{
					if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
					{
						vLine.erase(vLine.begin() + c);
						c--;
					}
				}

				float a, b;
				if (!bIsBot && !bIsTop)
				{
					double dA, dB;
					double dAngle = 0;
					int nMeasureDirection = Right_Line;
					bool bIsHorizon = false;
					POINTF poDrawLine[2];
					memset(poDrawLine, 0, sizeof(POINTF) * 2);
					int nLineLength = 0;
					if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
						m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
					a = (float)dA;
					b = (float)dB;
				}
				else
					m_pProcMilAlgo->OLS_1D(vLine, &a, &b);
				b = vLine[0].y - (vLine[0].x * a);

				std::vector<cv::Point> vDrawPo;
				for (int c = 0; c < vLeft.size(); c++)
				{
					if (vLine[0].x > vLeft[c].x)
						continue;
					else if (vLine[vLine.size() - 1].x < vLeft[c].x)
						break;
					int nStdX = (int)std::roundf(vLeft[c].x * a + b);
					if (std::abs(nStdX - vLeft[c].y) > 3)
					{
						vDrawPo.push_back(cv::Point(vLeft[c].x, nStdX));
					}
					else
					{
						vDrawPo.push_back(cv::Point(vLeft[c].x, vLeft[c].y));
					}
				}
				std::vector<cv::Point> vBlank;
				for (int l = 1; l < vDrawPo.size(); l++)
				{
					if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
					{
						float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
						float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
						for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
						{
							vBlank.push_back(cv::Point(x, fTmpA * x + fTmpB));
						}
					}
				}
				if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
				{
					b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
				}
#if LineDraw
				if (a != 0)
				{
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						for (int c = 0; c < vLine[0].x; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
					}
					for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
					{
						for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.rows; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(nStdX + 1, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
						}
					}
				}
#else
				if (bIsBot)
				{
					for (int c = 0; c < vDrawPo[0].x; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
					}
				}
				//for (int c = 0; c < vLeft.size(); c++)
				//{
				//	//int nStdX = (int)std::roundf(c*a + b);
				//	cv::line(ExceptArea, cv::Point(0, vLeft[c].x), cv::Point(vLeft[c].y, vLeft[c].x), cv::Scalar(255));
				//	cv::line(ExceptArea, cv::Point(vLeft[c].y + 1, vLeft[c].x), cv::Point(ExceptArea.cols, vLeft[c].x), cv::Scalar(0));
				//}
				for (int c = 0; c < vDrawPo.size(); c++)
				{
					cv::line(ExceptArea, cv::Point(0, vDrawPo[c].x), cv::Point(vDrawPo[c].y, vDrawPo[c].x), cv::Scalar(255));
					cv::line(ExceptArea, cv::Point(vDrawPo[c].y + 1, vDrawPo[c].x), cv::Point(ExceptArea.cols, vDrawPo[c].x), cv::Scalar(0));
				}
				for (int b = 0; b < vBlank.size(); b++)
				{
					//int nStdX = (int)std::roundf(c*a + b);
					cv::line(ExceptArea, cv::Point(0, vBlank[b].x), cv::Point(vBlank[b].y, vBlank[b].x), cv::Scalar(255));
					cv::line(ExceptArea, cv::Point(vBlank[b].y + 1, vBlank[b].x), cv::Point(ExceptArea.cols, vBlank[b].x), cv::Scalar(0));
				}
				if (bIsTop)
				{
					for (int c = vDrawPo[vDrawPo.size() - 1].x; c < ExceptArea.rows; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(0));
					}
				}
#endif
				int nLeft = (int)std::roundf(vLine[0].x * a + b);
				poConnerLt.x = nLeft > 0 ? nLeft : 0;
				poConnerLt.y = vLine[0].x;
				nLeft = (int)std::roundf(vLine[vLine.size() - 1].x * a + b);
				poConnerRb.x = nLeft > 0 ? nLeft : 0;
				poConnerRb.y = vLine[vLine.size() - 1].x;
			}
		}
		else if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
		{
			//우측
			std::vector<cv::Point> vRight;
			std::vector<int> vnRightCnt;
			vnRightCnt.resize(ContoursMaxImg.cols);
			//nWndDir += 0x08;
			for (int r = 0; r < ContoursMaxImg.rows; r++)
			{
				UCHAR* uPtr = ContoursMaxImg.ptr(r);
				int nmxC = ContoursMaxImg.cols - 1;
				int nmnC = 0;
				for (int c = 0; c < ContoursMaxImg.cols; c++)
				{
					if (nmnC == 0 && uPtr[c] > 0)
					{
						nmnC = c;
					}
					else if (nmnC != 0 && uPtr[c] == 0)
					{
						nmxC = c;
						break;
					}
				}
				UCHAR* uPtrex = ExceptAreaImg.ptr(r);
				for (int c = nmnC; c < nmxC; c++)
				{
					if (uPtrex[c] > 0)
					{
						vRight.push_back(cv::Point(r, c));
						vnRightCnt[c]++;
						nmxC = c;
						break;
					}
				}
				if (nmxC != ContoursMaxImg.cols - 1)
				{
					cv::line(ExceptArea, cv::Point(0, r), cv::Point(nmxC, r), cv::Scalar(0));
				}
			}
			int nMinVecCnt = rcBlob.Height() / 3;
			vRight.clear();
			if (vRight.size() > nMinVecCnt)
			{
				int nMxCnt = 0;
				int nMxIdx = 0;
				for (int c = 1; c < vnRightCnt.size(); c++)//끝에 완전히 붙는경우 제외
				{
					if (nMxCnt < vnRightCnt[c])
					{
						nMxCnt = vnRightCnt[c];
						nMxIdx = c;
					}
				}
				cv::Rect whiteRect(nMxIdx, 0, ExceptArea.cols - nMxIdx, ExceptArea.rows);
				cv::Rect blackRect(0, 0, nMxIdx, ExceptArea.rows);
				for (int c = 0; c < vRight.size(); c++)
				{
					if (std::abs(vRight[c].y - nMxIdx) < 5)
					{
						blackRect.y = vRight[c].x;
						break;
					}
				}
				for (int c = vRight.size() - 1; c >= 0; c--)
				{
					if (std::abs(vRight[c].y - nMxIdx) < 5)
					{
						blackRect.height = vRight[c].x - blackRect.y + 1;
						break;
					}
				}
				std::vector<cv::Point> vLine;
				int nStLineX = vRight[vRight.size() - 1].y;
				int nEndLineX = 0;
				for (int c = 0; c < vRight.size(); c++)
				{
					if (std::abs(vRight[c].y - nMxIdx) < 5)
					{
						vLine.push_back(vRight[c]);
						if (vRight[c].y == nMxIdx)
						{
							nEndLineX = vRight[c].x;
							if (nStLineX > vRight[c].x)
								nStLineX = vRight[c].x;
						}
					}
				}
				if (vLine.size() == 0)
					vLine = vRight;
				if (nEndLineX == 0)
					nEndLineX = vLine[vLine.size() - 1].x;
				if (nStLineX == vRight[vRight.size() - 1].y)
					nStLineX = vLine[0].x;
				for (int c = 0; c < vLine.size(); c++)
				{
					if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
					{
						vLine.erase(vLine.begin() + c);
						c--;
					}
				}
				float a, b;
				if (!bIsBot && !bIsTop)
				{
					double dA, dB;
					double dAngle = 0;
					int nMeasureDirection = Left_Line;
					bool bIsHorizon = false;
					POINTF poDrawLine[2];
					memset(poDrawLine, 0, sizeof(POINTF) * 2);
					int nLineLength = 0;
					if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
						m_pCPInsp_Algo->InspectionLine(nImgWidth, nImgHeight, ucArrBlobDst_except, bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
					a = (float)dA;
					b = (float)dB;
				}
				else
					m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

				b = vLine[0].y - (vLine[0].x * a);

#if LineDraw
				if (a != 0)
				{
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
					{
						for (int c = 0; c < vLine[0].x; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
						}
					}
					for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
					}
					if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
					{
						for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.rows; c++)
						{
							int nStdX = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX - 1, c), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
						}
					}
				}
#else

				std::vector<cv::Point> vDrawPo;
				for (int c = 0; c < vRight.size(); c++)
				{
					if (vLine[0].x > vRight[c].x)
						continue;
					else if (vLine[vLine.size() - 1].x < vRight[c].x)
						break;
					int nStdX = (int)std::roundf(vRight[c].x * a + b);
					if (std::abs(nStdX - vRight[c].y) > 3)
					{
						vDrawPo.push_back(cv::Point(vRight[c].x, nStdX));
					}
					else
					{
						vDrawPo.push_back(cv::Point(vRight[c].x, vRight[c].y));
					}
				}
				std::vector<cv::Point> vBlank;
				for (int l = 1; l < vDrawPo.size(); l++)
				{
					if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
					{
						float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
						float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
						for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
						{
							vBlank.push_back(cv::Point(x, fTmpA * x + fTmpB));
						}
					}
				}

				if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
				{
					b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
				}
				if (bIsBot)
				{
					for (int c = 0; c < vDrawPo[0].x; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
					}
				}
				//for (int c = 0; c < vRight.size(); c++)
				//{
				//	//int nStdX = (int)std::roundf(c*a + b);
				//	cv::line(ExceptArea, cv::Point(0, vRight[c].x), cv::Point(vRight[c].y - 1, vRight[c].x), cv::Scalar(0));
				//	cv::line(ExceptArea, cv::Point(vRight[c].y, vRight[c].x), cv::Point(ExceptArea.cols, vRight[c].x), cv::Scalar(255));
				//}
				for (int c = 0; c < vDrawPo.size(); c++)
				{
					//int nStdX = (int)std::roundf(c*a + b);
					cv::line(ExceptArea, cv::Point(0, vDrawPo[c].x), cv::Point(vDrawPo[c].y - 1, vDrawPo[c].x), cv::Scalar(0));
					cv::line(ExceptArea, cv::Point(vDrawPo[c].y, vDrawPo[c].x), cv::Point(ExceptArea.cols, vDrawPo[c].x), cv::Scalar(255));
				}
				for (int b = 0; b < vBlank.size(); b++)
				{
					cv::line(ExceptArea, cv::Point(0, vBlank[b].x), cv::Point(vBlank[b].y - 1, vBlank[b].x), cv::Scalar(0));
					cv::line(ExceptArea, cv::Point(vBlank[b].y, vBlank[b].x), cv::Point(ExceptArea.cols, vBlank[b].x), cv::Scalar(255));
				}
				if (bIsTop)
				{
					for (int c = vDrawPo[vDrawPo.size() - 1].x; c < ExceptArea.rows; c++)
					{
						int nStdX = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(0, c), cv::Point(nStdX, c), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(nStdX, c), cv::Point(ExceptArea.cols, c), cv::Scalar(255));
					}
				}
#endif
				int nLeft = (int)std::roundf(vLine[0].x * a + b);
				poConnerLt.x = nLeft > 0 ? nLeft : 0;
				poConnerLt.y = vLine[0].x;
				nLeft = (int)std::roundf(vLine[vLine.size() - 1].x * a + b);
				poConnerRb.x = nLeft > 0 ? nLeft : 0;
				poConnerRb.y = vLine[vLine.size() - 1].x;
			}
		}
		if ((pAlgoBlob->m_nThickDir != 1) && bIsTop)
		{
			//상측
			std::vector<cv::Point> vTop;
			std::vector<int> vnTopCnt;
			vnTopCnt.resize(ContoursMaxImg.rows);
			//nWndDir += 0x01;
			int nCntSt = poConnerLt.x;
			int nCntEd = ContoursMaxImg.cols;
			if ((nWndDir & 0x08) == 0x08)
			{
				nCntSt = 0;
				nCntEd = poConnerLt.x;
			}
			for (int c = nCntSt; c < nCntEd; c++)
			{
				int nmnR = 0;
				int nmxR = ContoursMaxImg.rows - 1;
				for (int r = ContoursMaxImg.rows - 1; r > 0; r--)
				{
					UCHAR* uPtr = ContoursMaxImg.ptr(r);
					if (nmxR == ContoursMaxImg.rows - 1 && uPtr[c] > 0)
					{
						nmxR = r;
					}
					else if (nmxR != ContoursMaxImg.rows - 1 && uPtr[c] == 0)
					{
						nmnR = r;
						break;
					}
				}
				for (int r = nmxR; r >= nmnR; r--)
				{
					UCHAR* uPtrex = ExceptAreaImg.ptr(r);
					if (uPtrex[c] > 0)
					{
						vTop.push_back(cv::Point(c, r));
						vnTopCnt[r]++;
						nmnR = r;
						break;
					}
				}
				if (nmnR != 0)
				{
					cv::line(ExceptArea, cv::Point(c, nmnR), cv::Point(c, ExceptArea.rows - 1), cv::Scalar(0));
				}
			}
			int nMinVecCnt = rcBlob.Width() / 3;
			vTop.clear();
			if (vTop.size() > nMinVecCnt)
			{
				int nMxCnt = 0;
				int nMxIdx = 0;
				for (int r = 0; r < vnTopCnt.size() - 1; r++)//끝에 완전히 붙는경우 제외
				{
					if (nMxCnt < vnTopCnt[r])
					{
						nMxCnt = vnTopCnt[r];
						nMxIdx = r;
					}
				}
				cv::Rect whiteRect(0, 0, ExceptArea.cols, nMxIdx);
				cv::Rect blackRect(0, nMxIdx, ExceptArea.cols, ExceptArea.rows);
				for (int r = 0; r < vTop.size(); r++)
				{
					if (std::abs(vTop[r].y - nMxIdx) < 5)
					{
						blackRect.x = vTop[r].x;
						break;
					}
				}
				for (int r = vTop.size() - 1; r >= blackRect.y; r--)
				{
					if (std::abs(vTop[r].y - nMxIdx) < 5)
					{
						blackRect.width = vTop[r].x - blackRect.x + 1;
						break;
					}
				}
				std::vector<cv::Point> vLine;
				int nStLineX = vTop[vTop.size() - 1].y;
				int nEndLineX = 0;
				for (int c = 0; c < vTop.size(); c++)
				{
					if (std::abs(vTop[c].y - nMxIdx) < 5)
					{
						vLine.push_back(vTop[c]);
						if (vTop[c].y == nMxIdx)
						{
							nEndLineX = vTop[c].x;
							if (nStLineX > vTop[c].x)
								nStLineX = vTop[c].x;
						}
					}
				}
				if (vLine.size() == 0)
					vLine = vTop;
				if (nEndLineX == 0)
					nEndLineX = vLine[vLine.size() - 1].x;
				if (nStLineX == vTop[vTop.size() - 1].y)
					nStLineX = vLine[0].x;
				for (int c = 0; c < vLine.size(); c++)
				{
					if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
					{
						vLine.erase(vLine.begin() + c);
						c--;
					}
				}
				float a, b;
				if (pAlgoBlob->m_nThickDir == 2)
				{
					double dA, dB;
					double dAngle = 0;
					int nMeasureDirection = Right_Line;
					bool bIsHorizon = true;
					POINTF poDrawLine[2];
					memset(poDrawLine, 0, sizeof(POINTF) * 2);
					int nLineLength = 0;
					cv::Mat ClipImg = ExceptAreaImg(cv::Rect(vLine[0].x, 0, vLine[vLine.size() - 1].x - vLine[0].x, ExceptAreaImg.rows)).clone();
					if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
						m_pCPInsp_Algo->InspectionLine(ClipImg.cols, ClipImg.rows, ClipImg.ptr(), bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
					a = (float)dA;
					b = (float)dB;
				}
				else
					m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

				b = vLine[0].y - (vLine[0].x * a);

				if ((nWndDir & 0x08) == 0x08)
				{
					b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
				}
#if LineDraw
				if (a != 0)
				{
					if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX - fWndWidth >= 0)
					{
						if ((nWndDir & 0x04) == 0x04)
						{
							for (int c = vLine[0].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c * a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
						else if ((nWndDir & 0x08) == 0x08)
						{
							for (int c = 0; c < vLine[0].x; c++)
							{
								int nStdY = (int)std::roundf(c * a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
								cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
							}
						}
					}
					for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
					{
						int nStdY = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
					}
					if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
					{
						for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
							cv::line(ExceptArea, cv::Point(c, nStdY + 1), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
						}
					}
				}
#else
				std::vector<cv::Point> vDrawPo;
				for (int c = 0; c < vTop.size(); c++)
				{
					if (vLine[0].x > vTop[c].x)
						continue;
					else if (vLine[vLine.size() - 1].x < vTop[c].x)
						break;
					int nStdY = (int)std::roundf(vTop[c].x * a + b);
					if (std::abs(nStdY - vTop[c].y) > 3)
					{
						vDrawPo.push_back(cv::Point(vTop[c].x, nStdY));
					}
					else
					{
						vDrawPo.push_back(cv::Point(vTop[c].x, vTop[c].y));
					}
				}
				std::vector<cv::Point> vBlank;
				for (int l = 1; l < vDrawPo.size(); l++)
				{
					if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
					{
						float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
						float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
						for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
						{
							vBlank.push_back(cv::Point(x, fTmpA * x + fTmpB));
						}
					}
				}

				if ((nWndDir & 0x04) != 0x04)
				{
					for (int c = 0; c < vDrawPo[0].x; c++)
					{
						int nStdY = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
					}
				}
				//for (int c = 0; c < vTop.size(); c++)
				//{
				//	//int nStdY = (int)std::roundf(c*a + b);
				//	cv::line(ExceptArea, cv::Point(vTop[c].x, 0), cv::Point(vTop[c].x, vTop[c].y), cv::Scalar(255));
				//	cv::line(ExceptArea, cv::Point(vTop[c].x, vTop[c].y + 1), cv::Point(vTop[c].x, ExceptArea.rows), cv::Scalar(0));
				//}
				for (int c = 0; c < vDrawPo.size(); c++)
				{
					//int nStdY = (int)std::roundf(c*a + b);
					cv::line(ExceptArea, cv::Point(vDrawPo[c].x, 0), cv::Point(vDrawPo[c].x, vDrawPo[c].y), cv::Scalar(255));
					cv::line(ExceptArea, cv::Point(vDrawPo[c].x, vDrawPo[c].y + 1), cv::Point(vDrawPo[c].x, ExceptArea.rows), cv::Scalar(0));
				}
				for (int b = 0; b < vBlank.size(); b++)
				{
					cv::line(ExceptArea, cv::Point(vBlank[b].x, 0), cv::Point(vBlank[b].x, vBlank[b].y), cv::Scalar(255));
					cv::line(ExceptArea, cv::Point(vBlank[b].x, vBlank[b].y + 1), cv::Point(vBlank[b].x, ExceptArea.rows), cv::Scalar(0));
				}
				if ((nWndDir & 0x08) != 0x08)
				{
					for (int c = vDrawPo[vDrawPo.size() - 1].x; c < ExceptArea.cols; c++)
					{
						int nStdY = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(255));
						cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(0));
					}
				}
#endif
				if ((nWndDir & 0x04) == 0x04)
				{
					poConnerRb.y = poConnerLt.y;
					int nTop = (int)std::roundf(vLine[0].x * a + b);
					poConnerRb.x = vLine[0].x;
					poConnerLt.y = nTop > 0 ? nTop : 0;
				}
				else if ((nWndDir & 0x08) == 0x08)
				{
					poConnerRb.x = poConnerLt.x;
					poConnerRb.y = poConnerLt.y;
					int nTop = (int)std::roundf(vLine[vLine.size() - 1].x * a + b);
					poConnerLt.x = vLine[vLine.size() - 1].x;
					poConnerLt.y = nTop > 0 ? nTop : 0;
				}
			}
		}
		else if ((pAlgoBlob->m_nThickDir != 1) && bIsBot)
		{
			//하측
			std::vector<cv::Point> vBot;
			std::vector<int> vnBotCnt;
			vnBotCnt.resize(ContoursMaxImg.rows);
			//nWndDir += 0x02;
			int nCntSt = poConnerLt.x;
			int nCntEd = ContoursMaxImg.cols;
			if ((nWndDir & 0x08) == 0x08)
			{
				nCntSt = 0;
				nCntEd = poConnerLt.x;
			}
			for (int c = nCntSt; c < nCntEd; c++)
			{
				int nmnR = 0;
				int nmxR = ContoursMaxImg.rows - 1;
				for (int r = 0; r < ContoursMaxImg.rows; r++)
				{
					UCHAR* uPtr = ContoursMaxImg.ptr(r);
					if (nmnR == 0 && uPtr[c] > 0)
					{
						nmnR = r;
					}
					else if (nmnR != 0 && uPtr[c] == 0)
					{
						nmxR = r;
						break;
					}
				}
				for (int r = nmnR; r < nmxR; r++)
				{
					UCHAR* uPtrex = ExceptAreaImg.ptr(r);
					if (uPtrex[c] > 0)
					{
						vBot.push_back(cv::Point(c, r));
						vnBotCnt[r]++;
						nmxR = r;
						break;
					}
				}
				if (nmxR != ContoursMaxImg.rows - 1)
				{
					cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nmxR), cv::Scalar(0));
				}
			}
			int nMinVecCnt = rcBlob.Width() / 3;
			vBot.clear();
			if (vBot.size() > nMinVecCnt)
			{
				int nMxCnt = 0;
				int nMxIdx = 0;
				for (int r = 1; r < vnBotCnt.size(); r++)//끝에 완전히 붙는경우 제외
				{
					if (nMxCnt < vnBotCnt[r])
					{
						nMxCnt = vnBotCnt[r];
						nMxIdx = r;
					}
				}
				cv::Rect blackRect(0, 0, ExceptArea.cols, nMxIdx);
				cv::Rect whiteRect(0, nMxIdx, ExceptArea.cols, ExceptArea.rows);
				for (int r = 0; r < vBot.size(); r++)
				{
					if (std::abs(vBot[r].y - nMxIdx) < 5)
					{
						blackRect.x = vBot[r].x;
						break;
					}
				}
				for (int r = vBot.size() - 1; r >= 0; r--)
				{
					if (std::abs(vBot[r].y - nMxIdx) < 5)
					{
						blackRect.width = vBot[r].x - blackRect.x + 1;
						break;
					}
				}
				std::vector<cv::Point> vLine;
				int nStLineX = vBot[vBot.size() - 1].y;
				int nEndLineX = 0;
				for (int c = 0; c < vBot.size(); c++)
				{
					if (std::abs(vBot[c].y - nMxIdx) < 5)
					{
						vLine.push_back(vBot[c]);
						if (vBot[c].y == nMxIdx)
						{
							nEndLineX = vBot[c].x;
							if (nStLineX > vBot[c].x)
								nStLineX = vBot[c].x;
						}
					}
				}
				if (vLine.size() == 0)
					vLine = vBot;
				if (nEndLineX == 0)
					nEndLineX = vLine[vLine.size() - 1].x;
				if (nStLineX == vBot[vBot.size() - 1].y)
					nStLineX = vLine[0].x;
				for (int c = 0; c < vLine.size(); c++)
				{
					if (!(vLine[c].x >= nStLineX && vLine[c].x <= nEndLineX))
					{
						vLine.erase(vLine.begin() + c);
						c--;
					}
				}
				float a, b;
				if (pAlgoBlob->m_nThickDir == 2)
				{
					double dA, dB;
					double dAngle = 0;
					int nMeasureDirection = Left_Line;
					bool bIsHorizon = true;
					POINTF poDrawLine[2];
					memset(poDrawLine, 0, sizeof(POINTF) * 2);
					int nLineLength = 0;
					cv::Mat ClipImg = ExceptAreaImg(cv::Rect(vLine[0].x, 0, vLine[vLine.size() - 1].x - vLine[0].x, ExceptAreaImg.rows)).clone();
					if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line || nMeasureDirection == Right_Line)
						m_pCPInsp_Algo->InspectionLine(ClipImg.cols, ClipImg.rows, ClipImg.ptr(), bIsHorizon, nMeasureDirection, &dAngle, poDrawLine, &nLineLength, &dA, &dB);
					a = (float)dA;
					b = (float)dB;
				}
				else
					m_pProcMilAlgo->OLS_1D(vLine, &a, &b);

				b = vLine[0].y - (vLine[0].x * a);

				if ((nWndDir & 0x08) == 0x08)
				{
					b = vLine[vLine.size() - 1].y - (vLine[vLine.size() - 1].x * a);
				}
#if LineDraw
				if (a != 0)
				{
					if ((pAlgoBlob->m_nThickDir != 2) && bIsBot)
					{
						if ((nWndDir & 0x04) == 0x04)
						{
							for (int c = vLine[0].x; c < ExceptArea.cols; c++)
							{
								int nStdY = (int)std::roundf(c * a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
							}
						}
						if ((nWndDir & 0x08) == 0x08)
						{
							for (int c = 0; c < vLine[0].x; c++)
							{
								int nStdY = (int)std::roundf(c * a + b);
								cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
								cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
							}
						}
					}
					for (int c = vLine[0].x; c < vLine[vLine.size() - 1].x; c++)
					{
						int nStdY = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
					}
					if ((pAlgoBlob->m_nThickDir != 2) && fWndCenterX + fWndWidth <= 0)
					{
						for (int c = vLine[vLine.size() - 1].x; c < ExceptArea.cols; c++)
						{
							int nStdY = (int)std::roundf(c * a + b);
							cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY - 1), cv::Scalar(0));
							cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
						}
					}
				}
#else
				std::vector<cv::Point> vDrawPo;
				for (int c = 0; c < vBot.size(); c++)
				{
					if (vLine[0].x > vBot[c].x)
						continue;
					else if (vLine[vLine.size() - 1].x < vBot[c].x)
						break;
					int nStdY = (int)std::roundf(vBot[c].x * a + b);
					if (std::abs(nStdY - vBot[c].y) > 3)
					{
						vDrawPo.push_back(cv::Point(vBot[c].x, nStdY));
					}
					else
					{
						vDrawPo.push_back(cv::Point(vBot[c].x, vBot[c].y));
					}
				}
				std::vector<cv::Point> vBlank;
				for (int l = 1; l < vDrawPo.size(); l++)
				{
					if (std::abs(vDrawPo[l].x - vDrawPo[l - 1].x) > 1)
					{
						float fTmpA = (vDrawPo[l].y - vDrawPo[l - 1].y) / (vDrawPo[l].x - vDrawPo[l - 1].x);
						float fTmpB = vDrawPo[l - 1].y - fTmpA * vDrawPo[l - 1].x;
						for (int x = vDrawPo[l - 1].x + 1; x < vDrawPo[l].x; x++)
						{
							vBlank.push_back(cv::Point(x, fTmpA * x + fTmpB));
						}
					}
				}

				if ((nWndDir & 0x04) != 0x04)
				{
					for (int c = 0; c < vDrawPo[0].x; c++)
					{
						int nStdY = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
					}
				}
				//for (int c = 0; c < vBot.size(); c++)
				//{
				//	//int nStdY = (int)std::roundf(c*a + b);
				//	cv::line(ExceptArea, cv::Point(vBot[c].x, 0), cv::Point(vBot[c].x, vBot[c].y - 1), cv::Scalar(0));
				//	cv::line(ExceptArea, cv::Point(vBot[c].x, vBot[c].y), cv::Point(vBot[c].x, ExceptArea.rows), cv::Scalar(255));
				//}
				for (int c = 0; c < vDrawPo.size(); c++)
				{
					cv::line(ExceptArea, cv::Point(vDrawPo[c].x, 0), cv::Point(vDrawPo[c].x, vDrawPo[c].y - 1), cv::Scalar(0));
					cv::line(ExceptArea, cv::Point(vDrawPo[c].x, vDrawPo[c].y), cv::Point(vDrawPo[c].x, ExceptArea.rows), cv::Scalar(255));
				}
				for (int b = 0; b < vBlank.size(); b++)
				{
					//int nStdY = (int)std::roundf(c*a + b);
					cv::line(ExceptArea, cv::Point(vBlank[b].x, 0), cv::Point(vBlank[b].x, vBlank[b].y - 1), cv::Scalar(0));
					cv::line(ExceptArea, cv::Point(vBlank[b].x, vBlank[b].y), cv::Point(vBlank[b].x, ExceptArea.rows), cv::Scalar(255));
				}
				if ((nWndDir & 0x08) != 0x08)
				{
					for (int c = vDrawPo[vDrawPo.size() - 1].x; c < ExceptArea.cols; c++)
					{
						int nStdY = (int)std::roundf(c * a + b);
						cv::line(ExceptArea, cv::Point(c, 0), cv::Point(c, nStdY), cv::Scalar(0));
						cv::line(ExceptArea, cv::Point(c, nStdY), cv::Point(c, ExceptArea.rows), cv::Scalar(255));
					}
				}
#endif
				if ((nWndDir & 0x04) == 0x04)
				{
					poConnerLt.y = poConnerRb.y;
					int nTop = (int)std::roundf(vLine[0].x * a + b);
					poConnerRb.x = vLine[0].x;
					poConnerRb.y = nTop > 0 ? nTop : 0;
				}
				else if ((nWndDir & 0x08) == 0x08)
				{
					poConnerLt.y = poConnerRb.y;
					int nTop = (int)std::roundf(vLine[vLine.size() - 1].x * a + b);
					poConnerLt.x = vLine[vLine.size() - 1].x;
					poConnerRb.y = nTop > 0 ? nTop : 0;
				}
			}
		}
		ContoursMaxImg -= ExceptArea;
	}

	cv::Mat gray = ContoursMaxImg.clone();

	cv::Mat labels, stats, centroids;
	int numLabels = cv::connectedComponentsWithStats(gray, labels, stats, centroids, 8, CV_32S);

	for (int label = 1; label < numLabels; ++label) {
		int area = stats.at<int>(label, cv::CC_STAT_AREA);
		if (area < 16) {
			gray.setTo(0, labels == label);
		}
	}

	std::vector<cv::Point> nonZeroPoints;
	cv::findNonZero(gray, nonZeroPoints);
	if (nonZeroPoints.empty())
		return nCntBlob;
	cv::Rect boundingBox = cv::boundingRect(nonZeroPoints);

	std::vector<cv::Point2f> dstPo;
	float finalminval;
	float finalmaxval;
	switch (nCaseCode)
	{
	case 0: ////Left-Right
	{
		float blockMaxThickness = 0.0f;
		cv::Point2f blockMaxFirstMeet(0, 0);
		cv::Point2f blockMaxLastInside(0, 0);

		cv::Point2f best_inter; float globalMaxThickness = 0.0f;
		cv::Point2f globalMaxFirstMeet(0, 0);
		cv::Point2f globalMaxLastInside(0, 0);

		float blockMinThickness = FLT_MAX;
		cv::Point2f blockMinFirstMeet(0, 0);
		cv::Point2f blockMinLastInside(0, 0);

		float globalMinThickness = FLT_MAX;
		cv::Point2f globalMinFirstMeet(0, 0);
		cv::Point2f globalMinLastInside(0, 0);

		int globalShiftYMax = 0;
		int globalShiftYMin = 0;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
		cv::Mat dilated = gray.clone();
		for (int i = 0; i < 5; ++i)
			cv::dilate(dilated, dilated, kernel);
		std::vector<std::vector<cv::Point>> allcontours;
		cv::findContours(gray, allcontours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
		std::vector<cv::Rect> blobBoxes;


		int targetHeight = static_cast<int>(gray.cols * 1.5f);//x1.5
		if (targetHeight <= 0) targetHeight = gray.rows;

		float overlapRatio = 0.1f;
		int overlap = static_cast<int>(targetHeight * overlapRatio);
		if (overlap >= targetHeight) overlap = targetHeight / 2;

		int numBlocks = (gray.rows - 1) / targetHeight;
		if (numBlocks == 0) numBlocks = 1;

		for (int b = 0; b < numBlocks; ++b)
		{
			int baseStart = b * targetHeight;
			int baseEnd = (b == numBlocks - 1) ? (gray.rows - 1) : (b + 1) * targetHeight;

			int y_start = baseStart - overlap;
			int y_end = baseEnd + overlap;

			if (b == 0)             y_start = 0; //first block
			if (b == numBlocks - 1) y_end = gray.rows - 1;  //last block

			y_start = std::max(0, y_start);
			y_end = std::min(gray.rows - 1, y_end);

			int height = y_end - y_start;
			if (height <= 0) continue;

			//Y block ROI
			cv::Rect blockRect(0, y_start, gray.cols, height);

			//block gray & dilated
			cv::Mat gray_block = gray(blockRect).clone();
			cv::Mat dilated_block = gray_block.clone();


			bool touchesBottom = cv::countNonZero(gray_block.row(gray_block.rows - 1)) > 0;
			bool touchesTop = cv::countNonZero(gray_block.row(0)) > 0;

			cv::Mat vis;
			cv::cvtColor(gray_block, vis, cv::COLOR_GRAY2BGR);

			for (int i = 0; i < 3; ++i)
				cv::dilate(dilated_block, dilated_block, kernel);

			//find all point in the block
			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(dilated_block, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			std::vector<cv::Point> allPoints;
			for (const auto& c : contours)
				allPoints.insert(allPoints.end(), c.begin(), c.end());
			if (allPoints.size() < 3) continue;

			//minRect
			cv::RotatedRect minRect = cv::minAreaRect(allPoints);

			float scale = 1.1f; //
			minRect.size.width *= scale;
			minRect.size.height *= scale;


			////visualize
			//cv::Point2f rectPts[4];
			//minRect.points(rectPts);

			//for (int i = 0; i < 4; ++i)
			//{
			//	cv::line(vis,
			//		rectPts[i],
			//		rectPts[(i + 1) % 4],
			//		cv::Scalar(0, 255, 128), // 연두색
			//		2,
			//		cv::LINE_AA);
			//}


			cv::Point2f vertices[4];
			minRect.points(vertices);

			//order the points
			int topLeftIdx = 0;
			float min_sum = vertices[0].x + vertices[0].y;
			for (int i = 1; i < 4; ++i) {
				float s = vertices[i].x + vertices[i].y;
				if (s < min_sum) { min_sum = s; topLeftIdx = i; }
			}
			std::vector<cv::Point2f> ordered(4);
			for (int i = 0; i < 4; ++i)
				ordered[i] = vertices[(topLeftIdx + i) % 4];

			//convex hull
			std::vector<cv::Point> hull;
			cv::convexHull(allPoints, hull);
			std::vector<std::pair<cv::Point2f, cv::Point2f>> convexEdges;
			for (size_t i = 0; i < hull.size(); ++i)
				convexEdges.emplace_back(hull[i], hull[(i + 1) % hull.size()]);


			////visualize convexHull
			//for (const auto& edge : convexEdges)
			//{
			//	cv::Point p1(cvRound(edge.first.x), cvRound(edge.first.y));
			//	cv::Point p2(cvRound(edge.second.x), cvRound(edge.second.y));
			//	cv::line(vis, p1, p2, cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
			//}

			//find the longest edges of the minRect
			std::vector<std::pair<cv::Point2f, cv::Point2f>> edges = 
			{
				{ordered[0], ordered[1]}, {ordered[1], ordered[2]},
				{ordered[2], ordered[3]}, {ordered[3], ordered[0]}
			};
			float maxLen = -1.0f, minLen = FLT_MAX;
			std::vector<std::pair<cv::Point2f, cv::Point2f>> longEdges;
			for (auto& e : edges) {
				float len = cv::norm(e.second - e.first);
				if (len < minLen) minLen = len;
				if (fabs(len - maxLen) < 1e-3f) longEdges.push_back(e);
				else if (len > maxLen) { maxLen = len; longEdges = { e }; }
			}

			auto averageDistanceToHull = [&](const std::pair<cv::Point2f, cv::Point2f>& edge)
				{
					cv::Point2f p1 = edge.first;
					cv::Point2f p2 = edge.second;
					cv::Point2f dir = p2 - p1;
					float len = cv::norm(dir);
					if (len < 1e-5f) return 1e6f;
					dir /= len;
					cv::Point2f normal(-dir.y, dir.x);

					double totalDist = 0.0;
					int count = 0;

					for (const auto& h : hull)
					{
						float dist = std::abs((h.x - p1.x) * normal.x + (h.y - p1.y) * normal.y);
						totalDist += dist;
						count++;
					}
					return (count > 0) ? static_cast<float>(totalDist / count) : 1e6f;
				};

			// projection
			cv::Point2f bottomP0=(0,0);
			cv::Point2f bottomP1(0,0);

			//compare two long edges(select the one with smaller average distance to hull)
			std::pair<cv::Point2f, cv::Point2f> chosenEdge = longEdges[0];
			if (longEdges.size() == 2)
			{
				float avgA = averageDistanceToHull(longEdges[0]);
				float avgB = averageDistanceToHull(longEdges[1]);
				chosenEdge = (avgA < avgB) ? longEdges[0] : longEdges[1];
			}
			bottomP0 = chosenEdge.first;
			bottomP1 = chosenEdge.second;

			


			//convex projection
			cv::Point2f edgeVec = bottomP1 - bottomP0;
			cv::Point2f edgeDir = edgeVec / cv::norm(edgeVec);
			cv::Point2f botNormal(-edgeDir.y, edgeDir.x);
			std::vector<cv::Point2f> botConvex;
			for (int t = 0; t <= maxLen; ++t) 
			{
				cv::Point2f base = bottomP0 + edgeDir * t;
				cv::Point2f rayEnd = base + botNormal * 1000.f;
				float minDist = FLT_MAX; cv::Point2f inter; bool found = false;
				for (auto& ce : convexEdges) 
				{
					cv::Point2f tmp;
					if (getLineIntersection(base, rayEnd, ce.first, ce.second, tmp)) 
					{
						float d = cv::norm(tmp - base);
						if (d > 0 && d < minDist) 
						{ 
							minDist = d; 
							inter = tmp;
							found = true; 
						}
					}
				}
				if (found)
				{
					//overlap covers these parts
					if (inter.y <= 3 || inter.y >= gray_block.rows - 3)
						continue;

					botConvex.push_back(inter);
				}
			}

			////visualize
			//// projection된 ray (하늘색 점)
			//for (size_t i = 0; i < botConvex.size(); i += 5)  // 5개당 1개만 표시 (너무 빽빽하지 않게)
			//{
			//	cv::circle(vis, botConvex[i], 2, cv::Scalar(255, 255, 0), -1);
			//}

			//// base 위치도 같이 시각화 (노란색)
			//for (int t = 0; t <= maxLen; t += 5)
			//{
			//	cv::Point2f base = bottomP0 + edgeDir * t;
			//	cv::circle(vis, base, 2, cv::Scalar(0, 255, 255), -1);
			//}
			

			//move convex
			std::vector<cv::Point2f> newConvex;
			for (auto& p : botConvex) {
				cv::Point2f moved = p + botNormal * minLen;
					//경계 보정 (y좌표)
					 if (moved.y <= 0)
						 moved.y = 0;
					 else if (moved.y >= gray_block.rows - 1)
						 moved.y = gray_block.rows - 1;

					 //경계 보정 (x좌표)
					 if (moved.x <= 0)
						 moved.x = 0;
					 else if (moved.x >= gray_block.cols - 1)
						 moved.x = gray_block.cols - 1;

					 newConvex.push_back(moved);
			}
			////visualize
			//for (size_t i = 0; i < newConvex.size(); i += 5) // 5개마다 1점 찍기 (너무 빽빽하지 않게)
			//{
			//	cv::circle(vis, newConvex[i], 2, cv::Scalar(0, 0, 255), -1);
			//}
			//cal thick with LineIterator
			float blockMaxThickness = -1.f, blockMinThickness = FLT_MAX;
			cv::Point2f blockMaxFirst, blockMaxLast, blockMinFirst, blockMinLast;

			for (int i = 0; i < botConvex.size(); ++i)
			{
				if (newConvex.empty())
					break; 

				cv::Point2f p0 = botConvex[i];
				cv::Point2f closest = newConvex[std::min(i, (int)newConvex.size() - 1)];
				cv::Point2f dir = closest - p0;
				if (cv::norm(dir) < 1e-5) continue;
				dir /= cv::norm(dir);

				cv::LineIterator it(gray_block,
					cv::Point(cvRound(p0.x), cvRound(p0.y)),
					cv::Point(cvRound(p0.x + dir.x * 1000.f),
						cvRound(p0.y + dir.y * 1000.f)), 8);

				cv::Point first(-1, -1), last(-1, -1);
				for (int k = 0; k < it.count; ++k, ++it) {
					cv::Point pt = it.pos();
					if (pt.x < 0 || pt.y < 0 || pt.x >= gray_block.cols || pt.y >= gray_block.rows)
						break;
					if (gray_block.at<uchar>(pt) > 0) {
						if (first.x < 0) first = pt;
						last = pt;
					}
				}

				if (first.x >= 0 && last.x >= 0) {
					float thick = cv::norm(first - last);
					if (first == last) thick = 0.5f;
					if (thick > blockMaxThickness) {
						blockMaxThickness = thick;
						blockMaxFirst = first; blockMaxLast = last;
					}
					if (thick < blockMinThickness) {
						blockMinThickness = thick;
						blockMinFirst = first; blockMinLast = last;
					}
				}
				else {
					//no blob found along this ray
					blockMinThickness = 0.f;
					blockMinFirst = p0;
					blockMinLast = p0 + dir * 1.f;
				}
			}
			if (!touchesTop || !touchesBottom)
			{
				blockMinThickness = 0.f;
			}
			//global update
			if (blockMaxThickness > globalMaxThickness) 
			{
				globalMaxThickness = blockMaxThickness;
				globalMaxFirstMeet = blockMaxFirst;
				globalMaxLastInside = blockMaxLast;
				globalShiftYMax = y_start;
			}
			if (blockMinThickness < globalMinThickness) 
			{
				globalMinThickness = blockMinThickness;
				globalMinFirstMeet = blockMinFirst;
				globalMinLastInside = blockMinLast;
				globalShiftYMin = y_start;
			}
		}
			if (pAlgoBlob->m_bUseThickMax)
			{ 
			finalmaxval = globalMaxThickness; 
			dstPo.push_back(cv::Point2f(globalMaxFirstMeet.x, globalMaxFirstMeet.y + globalShiftYMax));
			dstPo.push_back(cv::Point2f(globalMaxLastInside.x, globalMaxLastInside.y + globalShiftYMax)); 
			}
			if (pAlgoBlob->m_bUseThickMin)
			{
				finalminval = globalMinThickness;
			dstPo.push_back(cv::Point2f(globalMinFirstMeet.x, globalMinFirstMeet.y + globalShiftYMin));
			dstPo.push_back(cv::Point2f(globalMinLastInside.x, globalMinLastInside.y + globalShiftYMin));
			}
		}
		break;
		///Left-Top
		case 1:
		{
			bool touchesBottom = cv::countNonZero(gray.row(gray.rows - 1)) > 0;
			bool touchesRight = cv::countNonZero(gray.col(gray.cols - 1)) > 0;


			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));

			cv::Mat dilated = gray.clone();
			for (int i = 0; i < 3; ++i)
				cv::dilate(dilated, dilated, kernel);
			cv::Mat dilated2 = gray.clone();
			for (int i = 0; i < 4; ++i)
				cv::dilate(dilated2, dilated2, kernel);


			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<std::vector<cv::Point>> contours2;
			cv::findContours(dilated2, contours2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<cv::Point> aPoints;
			for (const auto& c : contours2)
				aPoints.insert(aPoints.end(), c.begin(), c.end());


			cv::RotatedRect minRect = cv::minAreaRect(aPoints);
			cv::Point2f vertices[4];
			minRect.points(vertices);


			int topLeftIdx = 0;
			float min_sum = vertices[0].x + vertices[0].y;
			for (int i = 1; i < 4; ++i)
			{
				float s = vertices[i].x + vertices[i].y;
				if (s < min_sum) 
				{
					min_sum = s;
					topLeftIdx = i;
				}
			}

			std::vector<cv::Point2f> ordered(4);
			for (int i = 0; i < 4; ++i) 
			{
				ordered[i] = vertices[(topLeftIdx + i) % 4];
			}

			std::vector<std::pair<cv::Point2f, cv::Point2f>> rectEdges;
			for (int i = 0; i < 4; ++i) 
			{
				rectEdges.emplace_back(ordered[i], ordered[(i + 1) % 4]);
			}

			// convex hull
			std::vector<cv::Point> allPoints;
			for (const auto& c : contours)
				allPoints.insert(allPoints.end(), c.begin(), c.end());


			std::vector<cv::Point> hull;
			cv::convexHull(allPoints, hull);
			std::vector<std::pair<cv::Point2f, cv::Point2f>> convexedges;
			for (size_t i = 0; i < hull.size(); ++i)
			{
				convexedges.emplace_back(hull[i], hull[(i + 1) % hull.size()]);
			}

			std::vector<cv::Point2f> intersectionPoints;
			//order edges
			cv::Point2f p0 = { 0,0 };
			cv::Point2f p1 = { 0,0 };
			std::vector<std::pair<cv::Point2f, cv::Point2f>> edges =
			{
				{ordered[0], ordered[1]},
				{ordered[1], ordered[2]},
				{ordered[2], ordered[3]},
				{ordered[3], ordered[0]}
			};

			float maxLen = -1.0f;
			float minLen = std::numeric_limits<float>::max();
			std::vector<std::pair<cv::Point2f, cv::Point2f>> longEdges;

			for (const auto& edge : edges)
			{
				const auto& p0 = edge.first;
				const auto& p1 = edge.second;

				float len = cv::norm(p1 - p0);

				if (len < minLen)
					minLen = len;

				if (std::abs(len - maxLen) < 1e-3f) 
				{
					longEdges.emplace_back(p0, p1);
				}
				else if (len > maxLen) 
				{
					maxLen = len;
					longEdges = { {p0, p1} };
				}
			}

			float bestYAvg = std::numeric_limits<float>::max();
			float worstYAvg = std::numeric_limits<float>::lowest();
			cv::Point2f topP0, topP1;
			cv::Point2f bottomP0, bottomP1;

			for (const auto& edge : longEdges) 
			{
				const auto& p0 = edge.first;
				const auto& p1 = edge.second;

				float yAvg = (p0.y + p1.y) * 0.5f;
				if (yAvg < bestYAvg)
				{
					bestYAvg = yAvg;
					topP0 = p0;
					topP1 = p1;
				}
				if (yAvg > worstYAvg)
				{
					worstYAvg = yAvg;
					bottomP0 = p0;
					bottomP1 = p1;
				}
			}
			std::vector<cv::Point2f> topConvex; 
			// select top convex
			cv::Point2f edgeVec = topP1 - topP0;
			cv::Point2f edgeDir = edgeVec / cv::norm(edgeVec);
			cv::Point2f topnormal(-edgeDir.y, edgeDir.x);  // 법선 벡터



			for (int t = 0; t <= maxLen; ++t)
			{
				cv::Point2f base = topP0 + edgeDir * t;
				cv::Point2f rayEnd = base + topnormal * 1000.0f;

				// find close points formconvexedge 
				float minDist = std::numeric_limits<float>::max();
				cv::Point2f firstIntersection;
				bool found = false;

				for (const auto& edges : convexedges) 
				{
					const auto& h1 = edges.first;
					const auto& h2 = edges.second;
					cv::Point2f inter;
					if (getLineIntersection(base, rayEnd, h1, h2, inter)) 
					{
						float dist = cv::norm(inter - base);
						if (dist > 0 && dist < minDist) 
						{
							minDist = dist;
							firstIntersection = inter;
							found = true;
						}
					}
				}

				if (found)
				{
					topConvex.push_back(firstIntersection);
				}
			}

			//copy and move convex
			std::vector<cv::Point2f> newConvex;
			float shiftAmount = minLen;

			for (int i = 0; i < topConvex.size(); ++i)
			{
				// shift to topnormal direction
				cv::Point2f moved = topConvex[i] + topnormal * shiftAmount;

				// is the point inside the image?
				if (moved.x > 0 && moved.x < gray.cols - 1 &&
					moved.y > 0 && moved.y < gray.rows - 1)
				{
					newConvex.push_back(moved);
				}
			}


			float maxThickness = -1.0f;
			float minThickness = FLT_MAX;
			if (!touchesBottom || !touchesRight)minThickness = 0;
			cv::Point maxFirstMeet, maxLastMeet;
			cv::Point minFirstMeet, minLastMeet;

			for (int i = 0; i < topConvex.size(); ++i)
			{
				cv::Point2f p0 = topConvex[i];

				float minDist = FLT_MAX;
				cv::Point2f closestNew;
				for (size_t j = 0; j < newConvex.size(); ++j) 
				{
					float dist = cv::norm(newConvex[j] - p0);
					if (dist < minDist)
					{
						minDist = dist;
						closestNew = newConvex[j];
					}
				}

				cv::Point2f dir = closestNew - p0;
				if (cv::norm(dir) < 1e-5)
					continue; 

				dir /= cv::norm(dir);

				//// 3. ray cast
				//cv::Point firstMeet(-1, -1), lastMeet(-1, -1);
				//float maxDist = 1000.0f;
				//for (float t = 0; t < maxDist; t += 1.0f) 
				//{
				//	cv::Point pt(cvRound(p0.x + dir.x * t), cvRound(p0.y + dir.y * t));
				//	if (pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
				//		break;

				//	uchar val = gray.at<uchar>(pt);
				//	if (val > 0 ) 
				//	{
				//		if (firstMeet.x < 0)
				//			firstMeet = pt;
				//		lastMeet = pt;
				//	}
				//}
				//Lineiterator
				cv::Point firstMeet(-1, -1), lastMeet(-1, -1);
				cv::Point2f end = p0 + dir * 1000.0f;

				cv::LineIterator it(gray,
					cv::Point(cvRound(p0.x), cvRound(p0.y)),
					cv::Point(cvRound(end.x), cvRound(end.y)),
					8);

				for (int k = 0; k < it.count; k++, ++it)
				{
					cv::Point pt = it.pos();
					if (pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
						break;

					uchar val = gray.at<uchar>(pt);
					if (val > 0)
					{
						if (firstMeet.x < 0)
							firstMeet = pt;  
						lastMeet = pt;        
					}
				}

				if (firstMeet.x >= 0 && lastMeet.x >= 0)
				{
					float thickness = cv::norm(firstMeet - lastMeet);
					if (firstMeet == lastMeet) thickness = 0.5f;
					if (thickness > maxThickness)
					{
						maxThickness = thickness;
						maxFirstMeet = firstMeet;
						maxLastMeet = lastMeet;
					}
					if (thickness < minThickness)
					{
						minThickness = thickness;
						minFirstMeet = firstMeet;
						minLastMeet = lastMeet;
					}
				}
				else
				{
					if (!(firstMeet.x >= 0) && !(lastMeet.x >= 0))
					{
						// no blob 
						minThickness = 0.0f;
						minFirstMeet = p0;        
						minLastMeet = p0 + dir * 1.0f;
					}
					else
					{
						minThickness = 0.5f;
						minFirstMeet = p0;     
						minLastMeet = p0 + dir * 1.0f;  
					}
				}
			}
			if (pAlgoBlob->m_bUseThickMax)
			{
				finalmaxval = maxThickness;
				dstPo.push_back(maxFirstMeet);
				dstPo.push_back(maxLastMeet);
			}
			if(pAlgoBlob->m_bUseThickMin)
			{
				finalminval = minThickness;
				dstPo.push_back(minFirstMeet);
				dstPo.push_back(minLastMeet);
			}
			break;

		}
		
		///Right-Top
		case 2:
		{

			bool touchesLeft = cv::countNonZero(gray.col(0)) > 0;
			bool touchesBottom = cv::countNonZero(gray.row(gray.rows - 1)) > 0;

			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));

			cv::Mat dilated = gray.clone();
			for (int i = 0; i < 3; ++i)
				cv::dilate(dilated, dilated, kernel);
			cv::Mat dilated2 = gray.clone();
			for (int i = 0; i < 4; ++i)
				cv::dilate(dilated2, dilated2, kernel);


			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<std::vector<cv::Point>> contours2;
			cv::findContours(dilated2, contours2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<cv::Point> aPoints;
			for (const auto& c : contours2)
				aPoints.insert(aPoints.end(), c.begin(), c.end());


			cv::RotatedRect minRect = cv::minAreaRect(aPoints);
			cv::Point2f vertices[4];
			minRect.points(vertices);



			int topLeftIdx = 0;
			float min_sum = vertices[0].x + vertices[0].y;
			for (int i = 1; i < 4; ++i) {
				float s = vertices[i].x + vertices[i].y;
				if (s < min_sum) 
				{
					min_sum = s;
					topLeftIdx = i;
				}
			}

			std::vector<cv::Point2f> ordered(4);
			for (int i = 0; i < 4; ++i) 
			{
				ordered[i] = vertices[(topLeftIdx + i) % 4];
			}

			std::vector<std::pair<cv::Point2f, cv::Point2f>> rectEdges;
			for (int i = 0; i < 4; ++i) 
			{
				rectEdges.emplace_back(ordered[i], ordered[(i + 1) % 4]);
			}

			// convex hull
			std::vector<cv::Point> allPoints;
			for (const auto& c : contours)
				allPoints.insert(allPoints.end(), c.begin(), c.end());


			std::vector<cv::Point> hull;
			cv::convexHull(allPoints, hull);
			std::vector<std::pair<cv::Point2f, cv::Point2f>> convexedges;
			for (size_t i = 0; i < hull.size(); ++i)
			{
				convexedges.emplace_back(hull[i], hull[(i + 1) % hull.size()]);
			}

			std::vector<cv::Point2f> intersectionPoints;
			// order edges
			cv::Point2f p0 = { 0,0 };
			cv::Point2f p1 = { 0,0 };
			std::vector<std::pair<cv::Point2f, cv::Point2f>> edges =
			{
				{ordered[0], ordered[1]},
				{ordered[1], ordered[2]},
				{ordered[2], ordered[3]},
				{ordered[3], ordered[0]}
			};

			float maxLen = -1.0f;
			float minLen = std::numeric_limits<float>::max();
			std::vector<std::pair<cv::Point2f, cv::Point2f>> longEdges;

			for (const auto& edge : edges)
			{
				const auto& p0 = edge.first;
				const auto& p1 = edge.second;

				float len = cv::norm(p1 - p0);

				if (len < minLen)
					minLen = len;

				if (std::abs(len - maxLen) < 1e-3f)
				{
					longEdges.emplace_back(p0, p1);
				}
				else if (len > maxLen) 
				{
					maxLen = len;
					longEdges = { {p0, p1} };
				}
			}

			float bestYAvg = std::numeric_limits<float>::max();
			float worstYAvg = std::numeric_limits<float>::lowest();
			cv::Point2f topP0, topP1;
			cv::Point2f bottomP0, bottomP1;

			for (const auto& edge : longEdges)
			{
				const auto& p0 = edge.first;
				const auto& p1 = edge.second;

				float yAvg = (p0.y + p1.y) * 0.5f;
				if (yAvg < bestYAvg)
				{
					bestYAvg = yAvg;
					topP0 = p0;
					topP1 = p1;
				}
				if (yAvg > worstYAvg)
				{
					worstYAvg = yAvg;
					bottomP0 = p0;
					bottomP1 = p1;
				}
			}
			std::vector<cv::Point2f> topConvex;  

			//select top convex
			cv::Point2f edgeVec = topP1 - topP0;
			cv::Point2f edgeDir = edgeVec / cv::norm(edgeVec);
			cv::Point2f topnormal(-edgeDir.y, edgeDir.x); 



			for (int t = 0; t <= maxLen; ++t)
			{
				cv::Point2f base = topP0 + edgeDir * t;
				cv::Point2f rayEnd = base + topnormal * 1000.0f;

				//finde close intersection from convexedge
				float minDist = std::numeric_limits<float>::max();
				cv::Point2f firstIntersection;
				bool found = false;

				for (const auto& edges : convexedges)
				{
					const auto& h1 = edges.first;
					const auto& h2 = edges.second;
					cv::Point2f inter;
					if (getLineIntersection(base, rayEnd, h1, h2, inter)) 
					{
						float dist = cv::norm(inter - base);
						if (dist > 0 && dist < minDist) {
							minDist = dist;
							firstIntersection = inter;
							found = true;
						}
					}
				}

				if (found)
				{
					topConvex.push_back(firstIntersection);
				}
			}

			//copy top convex and move
			std::vector<cv::Point2f> newConvex;
			float shiftAmount = minLen; 

			for (int i = 0; i < topConvex.size(); ++i)
			{
				// top normal direction
				cv::Point2f moved = topConvex[i] + topnormal * shiftAmount;

				// is the point inside the image?
				if (moved.x > 0 && moved.x < gray.cols - 1 &&
					moved.y > 0 && moved.y < gray.rows - 1)
				{
					newConvex.push_back(moved);
				}
			}

			float maxThickness = -1.0f;
			cv::Point maxFirstMeet, maxLastMeet;

			float minThickness = FLT_MAX;
			if (!touchesBottom || !touchesLeft)minThickness = 0;

			cv::Point minFirstMeet, minLastMeet;

			for (int i = 0; i < topConvex.size(); ++i)
			{
				cv::Point2f p0 = topConvex[i];

				float minDist = FLT_MAX;
				cv::Point2f closestNew;
				for (size_t j = 0; j < newConvex.size(); ++j)
				{
					float dist = cv::norm(newConvex[j] - p0);
					if (dist < minDist)
					{
						minDist = dist;
						closestNew = newConvex[j];
					}
				}

				cv::Point2f dir = closestNew - p0;
				if (cv::norm(dir) < 1e-5)
					continue; 

				dir /= cv::norm(dir);

				//// 3. ray cast
				//cv::Point firstMeet(-1, -1), lastMeet(-1, -1);
				//float maxDist = 1000.0f;
				//for (float t = 0; t < maxDist; t += 1.0f) 
				//{
				//	cv::Point pt(cvRound(p0.x + dir.x * t), cvRound(p0.y + dir.y * t));
				//	if (pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
				//		break;

				//	uchar val = gray.at<uchar>(pt);
				//	if (val > 0 )
				//	{
				//		if (firstMeet.x < 0)
				//			firstMeet = pt;
				//		lastMeet = pt;
				//	}
				//}
				//Lineiterator
				cv::Point firstMeet(-1, -1), lastMeet(-1, -1);
				cv::Point2f end = p0 + dir * 1000.0f;

				cv::LineIterator it(gray,
					cv::Point(cvRound(p0.x), cvRound(p0.y)),
					cv::Point(cvRound(end.x), cvRound(end.y)),
					8);

				for (int k = 0; k < it.count; k++, ++it)
				{
					cv::Point pt = it.pos();
					if (pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
						break;

					uchar val = gray.at<uchar>(pt);
					if (val > 0)
					{
						if (firstMeet.x < 0)
							firstMeet = pt;   
						lastMeet = pt;       
					}
				}

				if (firstMeet.x >= 0 && lastMeet.x >= 0)
				{
					float thickness = cv::norm(firstMeet - lastMeet);
					if (firstMeet == lastMeet) thickness = 0.5f;
					if (thickness > maxThickness)
					{
						maxThickness = thickness;
						maxFirstMeet = firstMeet;
						maxLastMeet = lastMeet;
					}
					if (thickness < minThickness)
					{
						minThickness = thickness;
						minFirstMeet = firstMeet;
						minLastMeet = lastMeet;
					}
				}
				else
				{
					if (!(firstMeet.x >= 0) && !(lastMeet.x >= 0))
					{
						minThickness = 0.0f;
						minFirstMeet = p0;       
						minLastMeet = p0 + dir * 1.0f;         
					}
					else
					{
						minThickness = 0.5f;
						minFirstMeet = p0;         
						minLastMeet = p0 + dir * 1.0f; 
					}
				}
			}
			if (pAlgoBlob->m_bUseThickMax)
			{
				finalmaxval = maxThickness;
				dstPo.push_back(maxFirstMeet);
				dstPo.push_back(maxLastMeet);
			}
			if (pAlgoBlob->m_bUseThickMin)
			{
				finalminval = minThickness;
				dstPo.push_back(minFirstMeet);
				dstPo.push_back(minLastMeet);
			}
			break;

		}
		
		///Left-Bottom
		case 3:
		{
			bool touchesTop = cv::countNonZero(gray.row(0)) > 0;
			bool touchesRight = cv::countNonZero(gray.col(gray.cols - 1)) > 0;

			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));

			cv::Mat dilated = gray.clone();
			for (int i = 0; i < 3; ++i)
				cv::dilate(dilated, dilated, kernel);
			cv::Mat dilated2 = gray.clone();
			for (int i = 0; i < 4; ++i)
				cv::dilate(dilated2, dilated2, kernel);


			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<std::vector<cv::Point>> contours2;
			cv::findContours(dilated2, contours2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<cv::Point> aPoints;
			for (const auto& c : contours2)
				aPoints.insert(aPoints.end(), c.begin(), c.end());


			cv::RotatedRect minRect = cv::minAreaRect(aPoints);
			cv::Point2f vertices[4];
			minRect.points(vertices);



			int topLeftIdx = 0;
			float min_sum = vertices[0].x + vertices[0].y;
			for (int i = 1; i < 4; ++i) 
			{
				float s = vertices[i].x + vertices[i].y;
				if (s < min_sum) 
				{
					min_sum = s;
					topLeftIdx = i;
				}
			}

			std::vector<cv::Point2f> ordered(4);
			for (int i = 0; i < 4; ++i) 
			{
				ordered[i] = vertices[(topLeftIdx + i) % 4];
			}

			std::vector<std::pair<cv::Point2f, cv::Point2f>> rectEdges;
			for (int i = 0; i < 4; ++i) 
			{
				rectEdges.emplace_back(ordered[i], ordered[(i + 1) % 4]);
			}

			// convex hull
			std::vector<cv::Point> allPoints;
			for (const auto& c : contours)
				allPoints.insert(allPoints.end(), c.begin(), c.end());


			std::vector<cv::Point> hull;
			cv::convexHull(allPoints, hull);
			std::vector<std::pair<cv::Point2f, cv::Point2f>> convexedges;
			for (size_t i = 0; i < hull.size(); ++i)
			{
				convexedges.emplace_back(hull[i], hull[(i + 1) % hull.size()]);
			}

			std::vector<cv::Point2f> intersectionPoints;
			// 2. 0~1 거리와 1~2 거리 비교
			cv::Point2f p0 = { 0,0 };
			cv::Point2f p1 = { 0,0 };
			std::vector<std::pair<cv::Point2f, cv::Point2f>> edges =
			{
				{ordered[0], ordered[1]},
				{ordered[1], ordered[2]},
				{ordered[2], ordered[3]},
				{ordered[3], ordered[0]}
			};

			float maxLen = -1.0f;
			float minLen = std::numeric_limits<float>::max();
			std::vector<std::pair<cv::Point2f, cv::Point2f>> longEdges;

			for (const auto& edge : edges) 
			{
				const auto& p0 = edge.first;
				const auto& p1 = edge.second;

				float len = cv::norm(p1 - p0);

				if (len < minLen)
					minLen = len;

				if (std::abs(len - maxLen) < 1e-3f) 
				{
					longEdges.emplace_back(p0, p1);
				}
				else if (len > maxLen)
				{
					maxLen = len;
					longEdges = { {p0, p1} };
				}
			}

			float bestYAvg = std::numeric_limits<float>::max();
			float worstYAvg = std::numeric_limits<float>::lowest();
			cv::Point2f topP0, topP1;
			cv::Point2f bottomP0, bottomP1;

			for (const auto& edge : longEdges) 
			{
				const auto& p0 = edge.first;
				const auto& p1 = edge.second;

				float yAvg = (p0.y + p1.y) * 0.5f;
				if (yAvg < bestYAvg)
				{
					bestYAvg = yAvg;
					topP0 = p0;
					topP1 = p1;
				}
				if (yAvg > worstYAvg)
				{
					worstYAvg = yAvg;
					bottomP0 = p0;
					bottomP1 = p1;
				}
			}

			std::vector<cv::Point2f> botConvex; 
			cv::Point2f edgeVec = bottomP1 - bottomP0;
			cv::Point2f edgeDir = edgeVec / cv::norm(edgeVec);
			cv::Point2f botnormal(-edgeDir.y, edgeDir.x);



			for (int t = 0; t <= maxLen; ++t)
			{
				cv::Point2f base = bottomP0 + edgeDir * t;
				cv::Point2f rayEnd = base + botnormal * 1000.0f;

				float minDist = std::numeric_limits<float>::max();
				cv::Point2f firstIntersection;
				bool found = false;

				for (const auto& edges : convexedges)
				{
					const auto& h1 = edges.first;
					const auto& h2 = edges.second;
					cv::Point2f inter;
					if (getLineIntersection(base, rayEnd, h1, h2, inter))
					{
						float dist = cv::norm(inter - base);
						if (dist > 0 && dist < minDist) {
							minDist = dist;
							firstIntersection = inter;
							found = true;
						}
					}
				}
				if (found)
					botConvex.push_back(firstIntersection);
			}

			std::vector<cv::Point2f> newConvex;
			float shiftAmount = minLen;

			for (int i = 0; i < botConvex.size(); ++i)
			{
				cv::Point2f moved = botConvex[i] + botnormal * shiftAmount;

				if (moved.x > 0 && moved.x < gray.cols - 1 &&
					moved.y > 0 && moved.y < gray.rows - 1)
				{
					newConvex.push_back(moved);
				}
			}


			float maxThickness = -1.0f;
			cv::Point maxFirstMeet, maxLastMeet;

			float minThickness = FLT_MAX;
			if (!touchesRight || !touchesTop)minThickness = 0;
			cv::Point minFirstMeet, minLastMeet;

			for (int i = 0; i < botConvex.size(); ++i)
			{
				cv::Point2f p0 = botConvex[i];

				float minDist = FLT_MAX;
				cv::Point2f closestNew;
				for (size_t j = 0; j < newConvex.size(); ++j) 
				{
					float dist = cv::norm(newConvex[j] - p0);
					if (dist < minDist) 
					{
						minDist = dist;
						closestNew = newConvex[j];
					}
				}

				cv::Point2f dir = closestNew - p0;
				if (cv::norm(dir) < 1e-5)
					continue;  

				dir /= cv::norm(dir);

				//// 3. ray cast
				//cv::Point firstMeet(-1, -1), lastMeet(-1, -1);
				//float maxDist = 1000.0f;
				//for (float t = 0; t < maxDist; t += 1.0f)
				//{
				//	cv::Point pt(cvRound(p0.x + dir.x * t), cvRound(p0.y + dir.y * t));
				//	if (pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
				//		break;

				//	uchar val = gray.at<uchar>(pt);
				//	if (val > 0) 
				//	{
				//		if (firstMeet.x < 0)
				//			firstMeet = pt;
				//		lastMeet = pt;
				//	}
				//}
				//LineIterator
				cv::Point firstMeet(-1, -1), lastMeet(-1, -1);
				cv::Point2f end = p0 + dir * 1000.0f;

				cv::LineIterator it(gray,
					cv::Point(cvRound(p0.x), cvRound(p0.y)),
					cv::Point(cvRound(end.x), cvRound(end.y)),
					8);

				for (int k = 0; k < it.count; k++, ++it)
				{
					cv::Point pt = it.pos();
					if (pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
						break;

					uchar val = gray.at<uchar>(pt);
					if (val > 0)
					{
						if (firstMeet.x < 0)
							firstMeet = pt;   
						lastMeet = pt;       
					}
				}

				if (firstMeet.x >= 0 && lastMeet.x >= 0)
				{
					float thickness = cv::norm(firstMeet - lastMeet);
					if (firstMeet == lastMeet) thickness = 0.5f;
					if (thickness > maxThickness)
					{
						maxThickness = thickness;
						maxFirstMeet = firstMeet;
						maxLastMeet = lastMeet;
					}
					if (thickness < minThickness)
					{
						minThickness = thickness;
						minFirstMeet = firstMeet;
						minLastMeet = lastMeet;
					}
				}
				else
				{
					if (!(firstMeet.x >= 0) && !(lastMeet.x >= 0))
					{
						minThickness = 0.0f;
						minFirstMeet = p0;         
						minLastMeet = p0 + dir * 1.0f;         
					}
					else
					{
						minThickness = 0.5f;
						minFirstMeet = p0;        
						minLastMeet = p0 + dir * 1.0f;
					}
				}
			}
			if (pAlgoBlob->m_bUseThickMax)
			{
				finalmaxval = maxThickness;
				dstPo.push_back(maxFirstMeet);
				dstPo.push_back(maxLastMeet);
			}
			if (pAlgoBlob->m_bUseThickMin)
			{
				finalminval = minThickness;
				dstPo.push_back(minFirstMeet);
				dstPo.push_back(minLastMeet);
			}
			break;

		}
		///Right-Bottom
		case 4:
		{
			bool touchesTop = cv::countNonZero(gray.row(0)) > 0;
			bool touchesLeft = cv::countNonZero(gray.col(0)) > 0;


			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));

			cv::Mat dilated = gray.clone();
			for (int i = 0; i < 3; ++i)
				cv::dilate(dilated, dilated, kernel);
			cv::Mat dilated2 = gray.clone();
			for (int i = 0; i < 5; ++i)
				cv::dilate(dilated2, dilated2, kernel);


			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<std::vector<cv::Point>> contours2;
			cv::findContours(dilated2, contours2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			std::vector<cv::Point> aPoints;
			for (const auto& c : contours2)
				aPoints.insert(aPoints.end(), c.begin(), c.end());


			cv::RotatedRect minRect = cv::minAreaRect(aPoints);
			cv::Point2f vertices[4];
			minRect.points(vertices);


			////visualize

			//cv::Mat vis;
			//cv::cvtColor(gray, vis, cv::COLOR_GRAY2BGR);


			int topLeftIdx = 0;
			float min_sum = vertices[0].x + vertices[0].y;
			for (int i = 1; i < 4; ++i)
			{
				float s = vertices[i].x + vertices[i].y;
				if (s < min_sum)
				{
					min_sum = s;
					topLeftIdx = i;
				}
			}

			std::vector<cv::Point2f> ordered(4);
			for (int i = 0; i < 4; ++i) 
			{
				ordered[i] = vertices[(topLeftIdx + i) % 4];
			}

			std::vector<std::pair<cv::Point2f, cv::Point2f>> rectEdges;
			for (int i = 0; i < 4; ++i) 
			{
				rectEdges.emplace_back(ordered[i], ordered[(i + 1) % 4]);
			}

			// convex hull
			std::vector<cv::Point> allPoints;
			for (const auto& c : contours)
				allPoints.insert(allPoints.end(), c.begin(), c.end());


			std::vector<cv::Point> hull;
			cv::convexHull(allPoints, hull);
			std::vector<std::pair<cv::Point2f, cv::Point2f>> convexedges;
			for (size_t i = 0; i < hull.size(); ++i) 
			{
				convexedges.emplace_back(hull[i], hull[(i + 1) % hull.size()]);
			}



			std::vector<cv::Point2f> intersectionPoints;
			
			cv::Point2f p0 = { 0,0 };
			cv::Point2f p1 = { 0,0 };
			std::vector<std::pair<cv::Point2f, cv::Point2f>> edges =
			{
				{ordered[0], ordered[1]},
				{ordered[1], ordered[2]},
				{ordered[2], ordered[3]},
				{ordered[3], ordered[0]}
			};

			float maxLen = -1.0f;
			float minLen = std::numeric_limits<float>::max();
			std::vector<std::pair<cv::Point2f, cv::Point2f>> longEdges;

			for (const auto& edge : edges) 
			{
				const auto& p0 = edge.first;
				const auto& p1 = edge.second;

				float len = cv::norm(p1 - p0);

				if (len < minLen)
					minLen = len;

				if (std::abs(len - maxLen) < 1e-3f) 
				{
					longEdges.emplace_back(p0, p1);
				}
				else if (len > maxLen) 
				{
					maxLen = len;
					longEdges = { {p0, p1} };
				}
			}

			float bestYAvg = std::numeric_limits<float>::max();
			float worstYAvg = std::numeric_limits<float>::lowest();
			cv::Point2f topP0, topP1;
			cv::Point2f bottomP0, bottomP1;

			for (const auto& edge : longEdges)
			{
				const auto& p0 = edge.first;
				const auto& p1 = edge.second;

				float yAvg = (p0.y + p1.y) * 0.5f;
				if (yAvg < bestYAvg)
				{
					bestYAvg = yAvg;
					topP0 = p0;
					topP1 = p1;
				}
				if (yAvg > worstYAvg)
				{
					worstYAvg = yAvg;
					bottomP0 = p0;
					bottomP1 = p1;
				}
			}

			std::vector<cv::Point2f> botConvex; 
			cv::Point2f edgeVec = bottomP1 - bottomP0;
			cv::Point2f edgeDir = edgeVec / cv::norm(edgeVec);
			cv::Point2f botnormal(-edgeDir.y, edgeDir.x); 

			for (int t = 0; t <= maxLen; ++t)
			{
				cv::Point2f base = bottomP0 + edgeDir * t;
				cv::Point2f rayEnd = base + botnormal * 1000.0f;

				float minDist = std::numeric_limits<float>::max();
				cv::Point2f firstIntersection;
				bool found = false;

				for (const auto& edges : convexedges)
				{
					const auto& h1 = edges.first;
					const auto& h2 = edges.second;
					cv::Point2f inter;
					if (getLineIntersection(base, rayEnd, h1, h2, inter))
					{
						float dist = cv::norm(inter - base);
						if (dist > 0 && dist < minDist) 
						{
							minDist = dist;
							firstIntersection = inter;
							found = true;
						}
					}
				}
				if (found)
					botConvex.push_back(firstIntersection); 
			}

			std::vector<cv::Point2f> newConvex;
			float shiftAmount = minLen;  

			for (int i = 0; i < botConvex.size(); ++i)
			{
				cv::Point2f moved = botConvex[i] + botnormal * shiftAmount;
				if (moved.x > 0 && moved.x < gray.cols - 1 &&
					moved.y > 0 && moved.y < gray.rows - 1)
				{
					newConvex.push_back(moved);
				}
			}

			//int step = 5; // 5개당 1점만 표시

			//for (int i = 0; i < botConvex.size(); i += step)
			//{
			//	const cv::Point2f& p = botConvex[i];
			//	if (p.x >= 0 && p.y >= 0 && p.x < vis.cols && p.y < vis.rows)
			//		cv::circle(vis, p, 2, cv::Scalar(0, 255, 255), -1); // 노란색
			//}

			//for (int i = 0; i < newConvex.size(); i += step)
			//{
			//	const cv::Point2f& p = newConvex[i];
			//	if (p.x >= 0 && p.y >= 0 && p.x < vis.cols && p.y < vis.rows)
			//		cv::circle(vis, p, 2, cv::Scalar(255, 255, 0), -1); // 하늘색
			//}


			float maxThickness = -1.0f;
			cv::Point maxFirstMeet, maxLastMeet;

			float minThickness = FLT_MAX;
			if (!touchesLeft || !touchesTop)minThickness = 0;

			cv::Point minFirstMeet, minLastMeet;

			for (int i = 0; i < botConvex.size(); ++i)
			{
				cv::Point2f p0 = botConvex[i];

				float minDist = FLT_MAX;
				cv::Point2f closestNew;
				for (size_t j = 0; j < newConvex.size(); ++j)
				{
					float dist = cv::norm(newConvex[j] - p0);
					if (dist < minDist)
					{
						minDist = dist;
						closestNew = newConvex[j];
					}
				}

				cv::Point2f dir = closestNew - p0;
				if (cv::norm(dir) < 1e-5)
					continue;  

				dir /= cv::norm(dir);

				//// 3. ray cast
				//cv::Point firstMeet(-1, -1), lastMeet(-1, -1);
				//float maxDist = 512.0f;
				//for (float t = 0; t < maxDist; t += 0.5f)
				//{
				//	cv::Point pt(cvRound(p0.x + dir.x * t), cvRound(p0.y + dir.y * t));
				//	if (pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
				//		break;

				//	uchar val = gray.at<uchar>(pt);
				//	if (val >0) 
				//	{
				//		if (firstMeet.x < 0)
				//			firstMeet = pt;
				//		lastMeet = pt;
				//	}
				//}
				//LineIterator
				cv::Point firstMeet(-1, -1), lastMeet(-1, -1);
				cv::Point2f end = p0 + dir * 1000.0f;

				cv::LineIterator it(gray,
					cv::Point(cvRound(p0.x), cvRound(p0.y)),
					cv::Point(cvRound(end.x), cvRound(end.y)),
					8);

				for (int k = 0; k < it.count; k++, ++it)
				{
					cv::Point pt = it.pos();
					if (pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
						break;

					uchar val = gray.at<uchar>(pt);
					if (val > 0)
					{
						if (firstMeet.x < 0)
							firstMeet = pt;  
						lastMeet = pt;        
					}
				}

				if (firstMeet.x >= 0 && lastMeet.x >= 0)
				{
					float thickness = cv::norm(firstMeet - lastMeet);
					if (firstMeet == lastMeet) thickness = 0.5f;
					if (thickness > maxThickness)
					{
						maxThickness = thickness;
						maxFirstMeet = firstMeet;
						maxLastMeet = lastMeet;
					}
					if (thickness < minThickness)
					{
						minThickness = thickness;
						minFirstMeet = firstMeet;
						minLastMeet = lastMeet;
					}
				}
				else
				{
					if (!(firstMeet.x >= 0) && !(lastMeet.x >= 0))
					{
						minThickness = 0.0f;
						minFirstMeet = p0;         
						minLastMeet = p0 + dir * 1.0f;
					}
					else
					{
						minThickness = 0.5f;
						minFirstMeet = p0;   
						minLastMeet = p0 + dir * 1.0f;
					}
				}
			}
			if (pAlgoBlob->m_bUseThickMax)
			{
				finalmaxval = maxThickness;
				dstPo.push_back(maxFirstMeet);
				dstPo.push_back(maxLastMeet);
			}
			if (pAlgoBlob->m_bUseThickMin)
			{
				finalminval = minThickness;
				dstPo.push_back(minFirstMeet);
				dstPo.push_back(minLastMeet);
			}
			break;

		}
		///Top-Bottom
		 case 5:
		 {
			 float blockMaxThickness = 0.0f;
			 cv::Point2f blockMaxFirstMeet(0, 0);
			 cv::Point2f blockMaxLastInside(0, 0);

			 cv::Point2f best_inter;
			 float globalMaxThickness = 0.0f;
			 cv::Point2f globalMaxFirstMeet(0, 0);
			 cv::Point2f globalMaxLastInside(0, 0);

			 float blockMinThickness = FLT_MAX;
			 cv::Point2f blockMinFirstMeet(0, 0);
			 cv::Point2f blockMinLastInside(0, 0);

			 float globalMinThickness = FLT_MAX;
			 cv::Point2f globalMinFirstMeet(0, 0);
			 cv::Point2f globalMinLastInside(0, 0);

			 int globalShiftXMax = 0;
			 int globalShiftXMin = 0;

			 cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
			 cv::Mat dilated = gray.clone();
			 for (int i = 0; i < 5; ++i)
				 cv::dilate(dilated, dilated, kernel);

			 int targetWidth = static_cast<int>(gray.rows * 1.5f);
			 if (targetWidth <= 0) targetWidth = gray.cols;

			 float overlapRatio = 0.1f;
			 int overlap = static_cast<int>(targetWidth * overlapRatio);
			 if (overlap >= targetWidth) overlap = targetWidth / 2;

			 int numBlocks = (gray.cols-1) / targetWidth;
			 if (numBlocks == 0) numBlocks = 1;

			 for (int b = 0; b < numBlocks; ++b)
			 {

				 int baseStart = b * targetWidth;
				 int baseEnd = (b == numBlocks - 1) ? gray.cols-1 : (b + 1) * targetWidth;

				 int x_start = baseStart - overlap;
				 int x_end = baseEnd + overlap;

				 if (b == 0)               x_start = 0;  
				 if (b == numBlocks - 1)   x_end = gray.cols-1;  

				 x_start = std::max(0, x_start);
				 x_end = std::min(gray.cols-1, x_end);

				 int width = x_end - x_start;
				 if (width <= 0) continue;

				 cv::Rect blockRect(x_start, 0, width, gray.rows);


				 cv::Mat gray_block = gray(blockRect).clone();
				 cv::Mat dilated_block = gray_block.clone();

				 bool touchesLeft = cv::countNonZero(gray_block.col(0)) > 0;
				 bool touchesRight = cv::countNonZero(gray_block.col(gray_block.cols - 1)) > 0;

				 //// 시각화용 이미지 생성
				 //cv::Mat vis;
				 //cv::cvtColor(gray_block, vis, cv::COLOR_GRAY2BGR);

				 for (int i = 0; i < 3; ++i)
					 cv::dilate(dilated_block, dilated_block, kernel);

				 // contour 추출
				 std::vector<std::vector<cv::Point>> contours;
				 cv::findContours(dilated_block, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
				 std::vector<cv::Point> allPoints;
				 for (const auto& c : contours)
					 allPoints.insert(allPoints.end(), c.begin(), c.end());
				 if (allPoints.size() < 3) continue;

				 //  minRect 계산 및 확장
				 cv::RotatedRect minRect = cv::minAreaRect(allPoints);
				 float scale = 1.1f;
				 minRect.size.width *= scale;
				 minRect.size.height *= scale;

				 //// minRect 시각화 (연두색)
				 //cv::Point2f rectPts[4];
				 //minRect.points(rectPts);
				 //for (int i = 0; i < 4; ++i)
					// cv::line(vis, rectPts[i], rectPts[(i + 1) % 4], cv::Scalar(0, 255, 128), 2, cv::LINE_AA);

				 std::vector<cv::Point> hull;
				 cv::convexHull(allPoints, hull);
				 std::vector<std::pair<cv::Point2f, cv::Point2f>> convexEdges;
				 for (size_t i = 0; i < hull.size(); ++i)
					 convexEdges.emplace_back(hull[i], hull[(i + 1) % hull.size()]);

				 // hull 시각화 (파란색)
				 //for (const auto& edge : convexEdges)
					// cv::line(vis, edge.first, edge.second, cv::Scalar(255, 0, 0), 1, cv::LINE_AA);

				 // 점 정렬
				 cv::Point2f vertices[4];
				 minRect.points(vertices);
				 int topLeftIdx = 0;
				 float min_sum = vertices[0].x + vertices[0].y;
				 for (int i = 1; i < 4; ++i)
				 {
					 float s = vertices[i].x + vertices[i].y;
					 if (s < min_sum) { min_sum = s; topLeftIdx = i; }
				 }
				 std::vector<cv::Point2f> ordered(4);
				 for (int i = 0; i < 4; ++i)
					 ordered[i] = vertices[(topLeftIdx + i) % 4];

				 // 긴 변 찾기
				 std::vector<std::pair<cv::Point2f, cv::Point2f>> edges =
				 {
					 {ordered[0], ordered[1]}, {ordered[1], ordered[2]},
					 {ordered[2], ordered[3]}, {ordered[3], ordered[0]}
				 };
				 float maxLen = -1.0f, minLen = FLT_MAX;
				 std::vector<std::pair<cv::Point2f, cv::Point2f>> longEdges;
				 for (auto& e : edges) {
					 float len = cv::norm(e.second - e.first);
					 if (len < minLen) minLen = len;
					 if (fabs(len - maxLen) < 1e-3f) longEdges.push_back(e);
					 else if (len > maxLen) { maxLen = len; longEdges = { e }; }
				 }

				 auto averageDistanceToHull = [&](const std::pair<cv::Point2f, cv::Point2f>& edge)
					 {
						 cv::Point2f p1 = edge.first;
						 cv::Point2f p2 = edge.second;
						 cv::Point2f dir = p2 - p1;
						 float len = cv::norm(dir);
						 if (len < 1e-5f) return 1e6f; 
						 dir /= len;
						 cv::Point2f normal(-dir.y, dir.x);

						 double totalDist = 0.0;
						 int count = 0;

						 for (const auto& h : hull)
						 {
							 float dist = std::abs((h.x - p1.x) * normal.x + (h.y - p1.y) * normal.y);
							 totalDist += dist;
							 count++;
						 }
						 return (count > 0) ? static_cast<float>(totalDist / count) : 1e6f;
					 };

				 cv::Point2f bottomP0 = (0, 0);
				 cv::Point2f bottomP1(0, 0);

				 std::pair<cv::Point2f, cv::Point2f> chosenEdge = longEdges[0];
				 if (longEdges.size() == 2)
				 {
					 float avgA = averageDistanceToHull(longEdges[0]);
					 float avgB = averageDistanceToHull(longEdges[1]);
					 chosenEdge = (avgA < avgB) ? longEdges[0] : longEdges[1];
				 }
				 bottomP0 = chosenEdge.first;
				 bottomP1 = chosenEdge.second;

				 cv::Point2f edgeVec = bottomP1 - bottomP0;
				 cv::Point2f edgeDir = edgeVec / cv::norm(edgeVec);
				 cv::Point2f botNormal(-edgeDir.y, edgeDir.x);
				 std::vector<cv::Point2f> botConvex;
				 for (int t = 0; t <= maxLen; ++t)
				 {
					 cv::Point2f base = bottomP0 + edgeDir * t;
					 cv::Point2f rayEnd = base + botNormal * 1000.f;
					 float minDist = FLT_MAX; cv::Point2f inter; bool found = false;
					 for (auto& ce : convexEdges)
					 {
						 cv::Point2f tmp;
						 if (getLineIntersection(base, rayEnd, ce.first, ce.second, tmp))
						 {
							 float d = cv::norm(tmp - base);
							 if (d > 0 && d < minDist)
							 {
								 minDist = d;
								 inter = tmp;
								 found = true;
							 }
						 }
					 }
					 if (found)
					 {
						 if (inter.x<= 3 || inter.x >= gray_block.cols - 3)
							 continue;

						 botConvex.push_back(inter);
					 }
				 }
				 //// projection된 ray (하늘색 점)
				 //for (size_t i = 0; i < botConvex.size(); i += 5)  // 5개당 1개만 표시 (너무 빽빽하지 않게)
				 //{
					// cv::circle(vis, botConvex[i], 2, cv::Scalar(255, 255, 0), -1);
				 //}

				 //// base 위치도 같이 시각화 (노란색)
				 //for (int t = 0; t <= maxLen; t += 5)
				 //{
					// cv::Point2f base = bottomP0 + edgeDir * t;
					// cv::circle(vis, base, 2, cv::Scalar(0, 255, 255), -1);
				 //}
	
				 std::vector<cv::Point2f> newConvex;
				 for (auto& p : botConvex) {
					 cv::Point2f moved = p + botNormal * minLen;
	
					 if (moved.y <= 0)
						 moved.y = 0;
					 else if (moved.y >= gray_block.rows - 1)
						 moved.y = gray_block.rows - 1;

					 if (moved.x <= 0)
						 moved.x = 0;
					 else if (moved.x >= gray_block.cols - 1)
						 moved.x = gray_block.cols - 1;

					 newConvex.push_back(moved);
				 }

				 //for (size_t i = 0; i < newConvex.size(); i += 5)
					// cv::circle(vis, newConvex[i], 2, cv::Scalar(0, 0, 255), -1);

				 // LineIterator
				 float blockMaxThickness = -1.f, blockMinThickness = FLT_MAX;
				 cv::Point2f blockMaxFirst, blockMaxLast, blockMinFirst, blockMinLast;

				 for (int i = 0; i < botConvex.size(); ++i)
				 {
					 if (newConvex.empty())
						 break;

					 cv::Point2f p0 = botConvex[i];
					 cv::Point2f closest = newConvex[std::min(i, (int)newConvex.size() - 1)];
					 cv::Point2f dir = closest - p0;
					 if (cv::norm(dir) < 1e-5) continue;
					 dir /= cv::norm(dir);

					 cv::LineIterator it(gray_block,
						 cv::Point(cvRound(p0.x), cvRound(p0.y)),
						 cv::Point(cvRound(p0.x + dir.x * 1000.f),
							 cvRound(p0.y + dir.y * 1000.f)), 8);

					 cv::Point first(-1, -1), last(-1, -1);
					 for (int k = 0; k < it.count; ++k, ++it) {
						 cv::Point pt = it.pos();
						 if (pt.x < 0 || pt.y < 0 || pt.x >= gray_block.cols || pt.y >= gray_block.rows)
							 break;
						 if (gray_block.at<uchar>(pt) > 0) {
							 if (first.x < 0) first = pt;
							 last = pt;
						 }
					 }
					 ////시각화: direction line (분홍색)
					 //if (first.x >= 0 && last.x >= 0)
					 //{
						// cv::line(
						//	 vis,
						//	 first,
						//	 last,
						//	 cv::Scalar(255, 0, 255),  // 분홍색 (magenta)
						//	 1,
						//	 cv::LINE_AA
						// );
					 //}
					 if (first.x >= 0 && last.x >= 0)
					 {
						 float thick = cv::norm(first - last);
						 if (first == last) thick = 0.5f;
						 if (thick > blockMaxThickness)
						 {
							 blockMaxThickness = thick;
							 blockMaxFirst = first; blockMaxLast = last;
						 }
						 if (thick < blockMinThickness)
						 {
							 blockMinThickness = thick;
							 blockMinFirst = first; blockMinLast = last;
						 }
					 }
					 else 
					 {
						 blockMinThickness = 0.f;
						 blockMinFirst = p0;
						 blockMinLast = p0 + dir * 1.f;
					 }
				 }

				 if (blockMaxThickness > globalMaxThickness)
				 {
					 globalMaxThickness = blockMaxThickness;
					 globalMaxFirstMeet = blockMaxFirst;
					 globalMaxLastInside = blockMaxLast;
					 globalShiftXMax = x_start;
				 }
				 if (blockMinThickness < globalMinThickness)
				 {
					 globalMinThickness = blockMinThickness;
					 globalMinFirstMeet = blockMinFirst;
					 globalMinLastInside = blockMinLast;
					 globalShiftXMin = x_start;
				 }
			 }

			 if (pAlgoBlob->m_bUseThickMax)
			 {
				 finalmaxval = globalMaxThickness;
				 dstPo.push_back(cv::Point2f(globalMaxFirstMeet.x + globalShiftXMax, globalMaxFirstMeet.y));
				 dstPo.push_back(cv::Point2f(globalMaxLastInside.x + globalShiftXMax, globalMaxLastInside.y));
			 }
			 if (pAlgoBlob->m_bUseThickMin)
			 {
				 finalminval = globalMinThickness;
				 dstPo.push_back(cv::Point2f(globalMinFirstMeet.x + globalShiftXMin, globalMinFirstMeet.y));
				 dstPo.push_back(cv::Point2f(globalMinLastInside.x + globalShiftXMin, globalMinLastInside.y));
			 }
		 }
		break;
	}
	if (!(pAlgoBlob->m_bUseTeachTh))
	{
		int stX = dstPo[0].x;
		int stY = dstPo[0].y;
		int edX = dstPo[1].x;
		int edY = dstPo[1].y;

		int stXmin = dstPo[2].x;
		int stYmin = dstPo[2].y;
		int edXmin = dstPo[3].x;
		int edYmin = dstPo[3].y;

		float nminVal = (float)(-INFINITY);
		float nMaxVal = 0;
		
		if (pAlgoBlob->m_bUseThickMin && !(pAlgoBlob->m_bUseThickMax))
		{
			ThminRect.left = stX;
			ThminRect.top = stY;
			ThminRect.right = edX;
			ThminRect.bottom = edY;
		}
		else if (pAlgoBlob->m_bUseThickMin && pAlgoBlob->m_bUseThickMax)
		{
			ThminRect.left = stXmin;
			ThminRect.top = stYmin;
			ThminRect.right = edXmin;
			ThminRect.bottom = edYmin;
		}
		

		if (pAlgoBlob->m_bUseThickMax) 
		{
			ThmaxRect.left = std::min(stX, edX);
			ThmaxRect.top = std::min(stY, edY);
			ThmaxRect.right = std::max(stX, edX);
			ThmaxRect.bottom = std::max(stY, edY);
		}
			
	
		if (finalminval == NAN)
		{
			maxVal = (double)nMaxVal;
			minVal = (double)nminVal;
		}
		else
		{
			maxVal = finalmaxval;
			minVal = finalminval;
		}
	}
	else
	{
#ifdef Thining
		UCHAR* ThiningBuf = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
		m_pCPInsp_Algo->ImageThining(nImgWidth, nImgHeight, pUcImgBlob, ThiningBuf);

		cv::Mat ThiningImg(nImgHeight, nImgWidth, CV_8UC1, ThiningBuf);
		cv::line(ThiningImg, cv::Point(0, 0), cv::Point(ThiningImg.cols - 1, 0), cv::Scalar(0));
		cv::line(ThiningImg, cv::Point(0, ThiningImg.rows - 1), cv::Point(ThiningImg.cols - 1, ThiningImg.rows - 1), cv::Scalar(0));
		cv::line(ThiningImg, cv::Point(0, 0), cv::Point(0, ThiningImg.rows), cv::Scalar(0));
		cv::line(ThiningImg, cv::Point(ThiningImg.cols - 1, 0), cv::Point(ThiningImg.cols - 1, ThiningImg.rows - 1), cv::Scalar(0));
#else
		cv::Mat ThiningImg(nImgHeight, nImgWidth, CV_8UC1);
#endif
		cv::Mat LabelImg(nImgHeight, nImgWidth, CV_16UC1);
		LabelImg.setTo(0);
		cv::Point MinPo, MaxPo;
		long pLebel[PTR_BLOB_MAX];
		memset(pLebel, 0, sizeof(long)* PTR_BLOB_MAX);
		m_pProcMilAlgo->GetBlobLabel(pLebel, LabelImg.ptr<USHORT>(), nImgWidth, nImgHeight);
		LabelImg.convertTo(LabelImg, CV_8UC1);
		float fMinth = pAlgoBlob->m_nMinThickRemove;
		int nLabel = 0;
		float nminVal = (float)(-INFINITY);
		float nMaxVal = 0;
		RECT LabelRects[BLOB_RECT_CNTS];
		m_pProcMilAlgo->GetBlobResult_Rects(LabelRects, BLOB_RECT_CNTS);
		//rect표현1 simple
		ThminRect.left = MinPo.x - nminVal;
		ThminRect.top = MinPo.y - nminVal;
		ThminRect.right = MinPo.x + nminVal;
		ThminRect.bottom = MinPo.y + nminVal;

		ThmaxRect.left = MaxPo.x - nMaxVal;
		ThmaxRect.top = MaxPo.y - nMaxVal;
		ThmaxRect.right = MaxPo.x + nMaxVal;
		ThmaxRect.bottom = MaxPo.y + nMaxVal;
		RECT ThminRectTmp, ThmaxRectTmp;
		ThmaxRectTmp = ThmaxRect;
		while (pLebel[nLabel] != 0)
		{
			if (pAlgoBlob->m_nThickDir == 1)//Width
			{
				nMaxVal = 0;
				for (int r = 0; r < nImgHeight; r++)
				{
					UCHAR* uPtr = LabelImg.ptr(r);
					int nMinW = 0;
					int nMaxW = 0;
					bool bWhite = false;
					for (int c = 0; c < nImgWidth; c++)
					{
						if (uPtr[c] == pLebel[nLabel])
						{
							nMaxW = nMinW = c;
							bWhite = true;
							break;
						}
					}
					if (bWhite)
					{
						for (int c = nMinW + 1; c < nImgWidth; c++)
						{
							if (uPtr[c] == 0)
							{
								nMaxW = c;
								break;
							}
						}
					}
					if (nMaxW - nMinW > 0)
					{
						if (minVal > (nMaxW - nMinW))
						{
							nminVal = (nMaxW - nMinW);
							ThminRect.top = r - 1;
							ThminRect.bottom = r + 1;
							ThminRect.left = nMinW;
							ThminRect.right = nMaxW;
						}
						if (nMaxVal < (nMaxW - nMinW))
						{
							nMaxVal = (nMaxW - nMinW);
							ThmaxRectTmp.top = r - 1;
							ThmaxRectTmp.bottom = r + 1;
							ThmaxRectTmp.left = nMinW;
							ThmaxRectTmp.right = nMaxW;
						}
					}
				}
				if (minVal == nImgWidth && ThminRect.top == 0 && ThminRect.bottom == 0)
				{
					minVal = 0;
				}
				if ((float)(nMaxVal) < fMinth)
				{
					cv::Mat removeImg, removeImg_inv;
					cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
					cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
					removeImg = removeImg_inv & removeImg;
					ContoursMaxImg = ContoursMaxImg - removeImg;
				}
				else if (nMaxVal > maxVal)
				{

					maxVal = nMaxVal;
					ThmaxRect.top = ThmaxRectTmp.top;
					ThmaxRect.bottom = ThmaxRectTmp.bottom;
					ThmaxRect.left = ThmaxRectTmp.left;
					ThmaxRect.right = ThmaxRectTmp.right;
				}
				if (maxVal <= 0)
				{
					ThmaxRect.left = MaxPo.x - nMaxVal;
					ThmaxRect.top = MaxPo.y - nMaxVal;
					ThmaxRect.right = MaxPo.x + nMaxVal;
					ThmaxRect.bottom = MaxPo.y + nMaxVal;
				}
			}
			else if (pAlgoBlob->m_nThickDir == 2)//length
			{
				nMaxVal = 0;
				minVal = nImgHeight;
				for (int c = 0; c < nImgWidth; c++)
				{
					int nMinW = 0;
					int nMaxW = 0;
					bool bWhite = false;
					for (int r = 0; r < nImgHeight; r++)
					{
						UCHAR* uPtr = LabelImg.ptr(r);
						if (uPtr[c] == pLebel[nLabel])
						{
							nMaxW = nMinW = r;
							bWhite = true;
							break;
						}
					}
					if (bWhite)
					{
						for (int r = nMaxW + 1; r < nImgHeight; r++)
						{
							UCHAR* uPtr = LabelImg.ptr(r);
							if (uPtr[c] == 0)
							{
								nMaxW = r;
								break;
							}
						}
					}
					if (nMaxW - nMinW > 0)
					{
						if (minVal > (nMaxW - nMinW))
						{
							minVal = (nMaxW - nMinW);
							ThminRect.top = nMinW;
							ThminRect.bottom = nMaxW;
							ThminRect.left = c - 1;
							ThminRect.right = c + 1;
						}
						if (nMaxVal < (nMaxW - nMinW))
						{
							nMaxVal = (nMaxW - nMinW);
							ThmaxRectTmp.top = nMinW;
							ThmaxRectTmp.bottom = nMaxW;
							ThmaxRectTmp.left = c - 1;
							ThmaxRectTmp.right = c + 1;
						}
					}
				}
				if (minVal == nImgHeight && ThminRect.top == 0 && ThminRect.bottom == 0)
				{
					minVal = 0;
				}
				if ((float)(nMaxVal) < fMinth)
				{
					cv::Mat removeImg, removeImg_inv;
					cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
					cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
					removeImg = removeImg_inv & removeImg;
					ContoursMaxImg = ContoursMaxImg - removeImg;
				}
				else if (nMaxVal > maxVal)
				{

					maxVal = nMaxVal;
					ThmaxRect.top = ThmaxRectTmp.top;
					ThmaxRect.bottom = ThmaxRectTmp.bottom;
					ThmaxRect.left = ThmaxRectTmp.left;
					ThmaxRect.right = ThmaxRectTmp.right;
				}
				if (maxVal <= 0)
				{
					ThmaxRect.left = MaxPo.x - nMaxVal;
					ThmaxRect.top = MaxPo.y - nMaxVal;
					ThmaxRect.right = MaxPo.x + nMaxVal;
					ThmaxRect.bottom = MaxPo.y + nMaxVal;
				}
			}
			else if (pAlgoBlob->m_nThickDir == 3)// +1 정방향
			{
				nMaxVal = 0;
				minVal = nImgHeight;
				int a = 1;

				cv::Point PSt(0, 0), PEd(0, 0);
				if ((nWndDir & 0x01) == 0x01)//top
				{
					int b = LabelRects[nLabel].top - LabelRects[nLabel].left;
					int c = LabelRects[nLabel].left;
					for (; c < LabelRects[nLabel].right; c++)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = PSt.x = c;
							PEd.y = PSt.y = y;
							c++;
							break;
						}
					}
					for (; c < LabelRects[nLabel].right; c++)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = c;
							PEd.y = y;
						}
					}
					float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
					if (nMaxVal < fdist)
					{
						nMaxVal = fdist;
						ThmaxRectTmp.top = PSt.y;
						ThmaxRectTmp.bottom = PEd.y;
						ThmaxRectTmp.left = PSt.x;
						ThmaxRectTmp.right = PEd.x;
					}
					if (minVal > fdist)
					{
						minVal = fdist;
						ThminRectTmp.top = PSt.y;
						ThminRectTmp.bottom = PEd.y;
						ThminRectTmp.left = PSt.x;
						ThminRectTmp.right = PEd.x;
					}
				}
				else if ((nWndDir & 0x02) == 0x02)//bot
				{
					int b = LabelRects[nLabel].bottom - (LabelRects[nLabel].right - 1);
					int c = LabelRects[nLabel].right - 1;
					for (; c >= 0; c--)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = PSt.x = c;
							PEd.y = PSt.y = y;
							break;
						}
					}
					for (; c >= 0; c--)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PSt.x = c;
							PSt.y = y;
						}
					}
					float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
					if (nMaxVal < fdist)
					{
						nMaxVal = fdist;
						ThmaxRectTmp.top = PSt.y;
						ThmaxRectTmp.bottom = PEd.y;
						ThmaxRectTmp.left = PSt.x;
						ThmaxRectTmp.right = PEd.x;
					}
					if (minVal > fdist)
					{
						minVal = fdist;
						ThminRectTmp.top = PSt.y;
						ThminRectTmp.bottom = PEd.y;
						ThminRectTmp.left = PSt.x;
						ThminRectTmp.right = PEd.x;
					}
				}
				/*for (int r = nImgHeight - 1; r > 0; r--)
				{
					cv::Point PSt(0, 0), PEd(0, 0);
					int b = r;
					for (int c = 0; c < nImgWidth; c++)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = PSt.x = c;
							PEd.y = PSt.y = y;
							break;
						}
					}
					for (int c = PSt.x + 1; c < nImgWidth; c++)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = c;
							PEd.y = y;
						}
					}
					float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
					if (nMaxVal < fdist)
					{
						nMaxVal = fdist;
						ThmaxRectTmp.top = PSt.y;
						ThmaxRectTmp.bottom = PEd.y;
						ThmaxRectTmp.left = PSt.x;
						ThmaxRectTmp.right = PEd.x;
					}
					if (minVal > fdist)
					{
						minVal = fdist;
						ThminRectTmp.top = PSt.y;
						ThminRectTmp.bottom = PEd.y;
						ThminRectTmp.left = PSt.x;
						ThminRectTmp.right = PEd.x;
					}
				}
				for (int cOrg = 1; cOrg < nImgWidth; cOrg++)
				{
					cv::Point PSt(0, 0), PEd(0, 0);
					int b = (-1) * a * cOrg;
					for (int c = cOrg; c < nImgWidth; c++)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = PSt.x = c;
							PEd.y = PSt.y = y;
							break;
						}
					}
					for (int c = PSt.x + 1; c < nImgWidth; c++)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = c;
							PEd.y = y;
						}
					}
					float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
					if (nMaxVal < fdist)
					{
						nMaxVal = fdist;
						ThmaxRectTmp.top = PSt.y;
						ThmaxRectTmp.bottom = PEd.y;
						ThmaxRectTmp.left = PSt.x;
						ThmaxRectTmp.right = PEd.x;
					}
					if (minVal > fdist)
					{
						minVal = fdist;
						ThminRectTmp.top = PSt.y;
						ThminRectTmp.bottom = PEd.y;
						ThminRectTmp.left = PSt.x;
						ThminRectTmp.right = PEd.x;
					}
				}
	*/
				if (minVal == nImgHeight && ThminRect.top == 0 && ThminRect.bottom == 0)
				{
					minVal = 0;
				}
				if ((float)(nMaxVal) < fMinth)
				{
					cv::Mat removeImg, removeImg_inv;
					cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
					cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
					removeImg = removeImg_inv & removeImg;
					ContoursMaxImg = ContoursMaxImg - removeImg;
				}
				else if (nMaxVal > maxVal)
				{

					maxVal = nMaxVal;
					ThmaxRect.top = ThmaxRectTmp.top < ThmaxRectTmp.bottom ? ThmaxRectTmp.top : ThmaxRectTmp.bottom;
					ThmaxRect.bottom = ThmaxRectTmp.top > ThmaxRectTmp.bottom ? ThmaxRectTmp.top : ThmaxRectTmp.bottom;
					ThmaxRect.left = ThmaxRectTmp.left < ThmaxRectTmp.right ? ThmaxRectTmp.left : ThmaxRectTmp.right;
					ThmaxRect.right = ThmaxRectTmp.left > ThmaxRectTmp.right ? ThmaxRectTmp.left : ThmaxRectTmp.right;
				}
				if (maxVal <= 0)
				{
					ThmaxRect.left = MaxPo.x - nMaxVal;
					ThmaxRect.top = MaxPo.y - nMaxVal;
					ThmaxRect.right = MaxPo.x + nMaxVal;
					ThmaxRect.bottom = MaxPo.y + nMaxVal;
				}
			}
			else if (pAlgoBlob->m_nThickDir == 4)// -1 역방향
			{
				nMaxVal = 0;
				minVal = nImgHeight;
				int a = -1;
				cv::Point PSt(0, 0), PEd(0, 0);
				if ((nWndDir & 0x01) == 0x01)//top
				{
					int b = (LabelRects[nLabel].right - 1) + LabelRects[nLabel].top;
					int c = LabelRects[nLabel].right - 1;
					for (; c >= LabelRects[nLabel].left; c--)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = PSt.x = c;
							PEd.y = PSt.y = y;
							break;
						}
					}
					for (; c >= LabelRects[nLabel].left; c--)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = c;
							PEd.y = y;
						}
					}
					float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
					if (nMaxVal < fdist)
					{
						nMaxVal = fdist;
						ThmaxRectTmp.top = PSt.y;
						ThmaxRectTmp.bottom = PEd.y;
						ThmaxRectTmp.left = PSt.x;
						ThmaxRectTmp.right = PEd.x;
					}
					if (minVal > fdist)
					{
						minVal = fdist;
						ThminRectTmp.top = PSt.y;
						ThminRectTmp.bottom = PEd.y;
						ThminRectTmp.left = PSt.x;
						ThminRectTmp.right = PEd.x;
					}
				}
				else if ((nWndDir & 0x02) == 0x02)//bot
				{
					int b = LabelRects[nLabel].bottom + LabelRects[nLabel].left;
					int c = LabelRects[nLabel].left;
					for (; c < LabelRects[nLabel].right; c++)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = PSt.x = c;
							PEd.y = PSt.y = y;
							break;
						}
					}
					for (; c < LabelRects[nLabel].right; c++)
					{
						int y = a * c + b;
						if (y < 0)
							continue;
						if (y >= nImgHeight)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PSt.x = c;
							PSt.y = y;
						}
					}
					float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
					if (nMaxVal < fdist)
					{
						nMaxVal = fdist;
						ThmaxRectTmp.top = PSt.y;
						ThmaxRectTmp.bottom = PEd.y;
						ThmaxRectTmp.left = PSt.x;
						ThmaxRectTmp.right = PEd.x;
					}
					if (minVal > fdist)
					{
						minVal = fdist;
						ThminRectTmp.top = PSt.y;
						ThminRectTmp.bottom = PEd.y;
						ThminRectTmp.left = PSt.x;
						ThminRectTmp.right = PEd.x;
					}
				}
				/*for (int r = 0; r < nImgHeight; r++)
				{
					cv::Point PSt(0, 0), PEd(0, 0);
					PEd.x = PSt.x = nImgWidth;
					PEd.y = PSt.y = nImgHeight;
					int b = r;
					for (int c = 0; c < nImgWidth; c++)
					{
						int y = a * c + b;
						if (y < 0)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = PSt.x = c;
							PEd.y = PSt.y = y;
							break;
						}
					}
					for (int c = PSt.x + 1; c < nImgWidth; c++)
					{
						int y = a * c + b;
						if (y < 0)
							break;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = c;
							PEd.y = y;
						}
					}
					float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
					if (nMaxVal < fdist)
					{
						nMaxVal = fdist;
						ThmaxRectTmp.top = PSt.y;
						ThmaxRectTmp.bottom = PEd.y;
						ThmaxRectTmp.left = PSt.x;
						ThmaxRectTmp.right = PEd.x;
					}
					if (minVal > fdist)
					{
						minVal = fdist;
						ThminRectTmp.top = PSt.y;
						ThminRectTmp.bottom = PEd.y;
						ThminRectTmp.left = PSt.x;
						ThminRectTmp.right = PEd.x;
					}
				}
				for (int cOrg = 1; cOrg < nImgWidth; cOrg++)
				{
					cv::Point PSt(0, 0), PEd(0, 0);
					PEd.x = PSt.x = nImgWidth;
					PEd.y = PSt.y = nImgHeight;
					int b = (-1) * a * cOrg + (nImgHeight - 1);
					for (int c = cOrg; c < nImgWidth; c++)
					{
						int y = a * c + b;
						if (y < 0)
							break;
						if (y >= nImgHeight)
							continue;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = PSt.x = c;
							PEd.y = PSt.y = y;
							break;
						}
					}
					for (int c = PSt.x + 1; c < nImgWidth; c++)
					{
						int y = a * c + b;
						if (y < 0)
							break;
						if (y >= nImgHeight)
							continue;
						UCHAR* uPtr = LabelImg.ptr(y);
						if (uPtr[c] == pLebel[nLabel])
						{
							PEd.x = c;
							PEd.y = y;
						}
					}
					float fdist = std::sqrtf(std::powl(PEd.x - PSt.x, 2) + std::powl(PEd.y - PSt.y, 2));
					if (nMaxVal < fdist)
					{
						nMaxVal = fdist;
						ThmaxRectTmp.top = PSt.y;
						ThmaxRectTmp.bottom = PEd.y;
						ThmaxRectTmp.left = PSt.x;
						ThmaxRectTmp.right = PEd.x;
					}
					if (minVal > fdist)
					{
						minVal = fdist;
						ThminRectTmp.top = PSt.y;
						ThminRectTmp.bottom = PEd.y;
						ThminRectTmp.left = PSt.x;
						ThminRectTmp.right = PEd.x;
					}
				}*/
				if (minVal == nImgHeight && ThminRect.top == 0 && ThminRect.bottom == 0)
				{
					minVal = 0;
				}
				if ((float)(nMaxVal) < fMinth)
				{
					cv::Mat removeImg, removeImg_inv;
					cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
					cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
					removeImg = removeImg_inv & removeImg;
					ContoursMaxImg = ContoursMaxImg - removeImg;
				}
				else if (nMaxVal > maxVal)
				{

					maxVal = nMaxVal;
					ThmaxRect.top = ThmaxRectTmp.top < ThmaxRectTmp.bottom ? ThmaxRectTmp.top : ThmaxRectTmp.bottom;
					ThmaxRect.bottom = ThmaxRectTmp.top > ThmaxRectTmp.bottom ? ThmaxRectTmp.top : ThmaxRectTmp.bottom;
					ThmaxRect.left = ThmaxRectTmp.left < ThmaxRectTmp.right ? ThmaxRectTmp.left : ThmaxRectTmp.right;
					ThmaxRect.right = ThmaxRectTmp.left > ThmaxRectTmp.right ? ThmaxRectTmp.left : ThmaxRectTmp.right;
				}
				if (maxVal <= 0)
				{
					ThmaxRect.left = MaxPo.x - nMaxVal;
					ThmaxRect.top = MaxPo.y - nMaxVal;
					ThmaxRect.right = MaxPo.x + nMaxVal;
					ThmaxRect.bottom = MaxPo.y + nMaxVal;
				}
			}
			else//all direction
			{
				cv::Point PartCenter;
				PartCenter.x = -fWndCenterX;
				PartCenter.y = -fWndCenterY;
				if (pAlgoBlob->m_nPoCnt > 0 && pAlgoBlob->m_bUseTeachTh)
				{
					std::vector<std::vector<cv::Point>> contours;
					std::vector<cv::Point> thickPoints;
					std::vector<cv::Point> vLinePo;
					vLinePo.push_back(cv::Point(pAlgoBlob->Po1.x, pAlgoBlob->Po1.y));
					vLinePo.push_back(cv::Point(pAlgoBlob->Po2.x, pAlgoBlob->Po2.y));
					vLinePo.push_back(cv::Point(pAlgoBlob->Po3.x, pAlgoBlob->Po3.y));
					vLinePo.push_back(cv::Point(pAlgoBlob->Po4.x, pAlgoBlob->Po4.y));
					vLinePo.push_back(cv::Point(pAlgoBlob->Po5.x, pAlgoBlob->Po5.y));
					vLinePo.push_back(cv::Point(pAlgoBlob->Po6.x, pAlgoBlob->Po6.y));
					for (int poidx = 1; poidx < vLinePo.size(); poidx++)
					{
						double diffX = vLinePo[poidx].x - vLinePo[poidx - 1].x;
						double diffY = vLinePo[poidx].y - vLinePo[poidx - 1].y;
						if (diffX == 0)
						{
							for (int r = vLinePo[poidx - 1].y; r < vLinePo[poidx].y; r++)
							{
								thickPoints.push_back(cv::Point(vLinePo[poidx].x, r));
							}
						}
						else if (std::abs(diffY) > std::abs(diffX))
						{
							double dTmpA = (double)(diffX) / diffY;
							double dTmpB = vLinePo[poidx].x - ((double)vLinePo[poidx].y * dTmpA);
							for (int r = vLinePo[poidx - 1].x; r < vLinePo[poidx].x; r++)
							{
								thickPoints.push_back(cv::Point(vLinePo[poidx].y * dTmpA + dTmpB, vLinePo[poidx].y));
							}
						}
						else
						{
							double dTmpA = (double)(diffY) / diffX;
							double dTmpB = vLinePo[poidx].y - ((double)vLinePo[poidx].x * dTmpA);
							for (int r = vLinePo[poidx - 1].x; r < vLinePo[poidx].x; r++)
							{
								thickPoints.push_back(cv::Point(vLinePo[poidx].x, vLinePo[poidx].x * dTmpA + dTmpB));
							}
						}
					}
					std::vector<cv::Point> dstPo = thickness3(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), vLinePo, pLebel[nLabel]);
					int stX = dstPo[0].x;
					int stY = dstPo[0].y;
					int edX = dstPo[1].x;
					int edY = dstPo[1].y;
#if _DEBUG
					cv::Mat debugImg = ContoursMaxImg.clone();
					cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
					for (int contIdx = 0; contIdx < contours.size(); contIdx++)
					{
						thickPoints = contours[contIdx];
						int nColorDrEn = thickPoints.size() - 1;
						for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
						{
							cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
						}
					}
					cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
					cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif

					float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
					if (dist < nminVal)
					{
						nminVal = dist;
						ThminRect.left = stX;
						ThminRect.top = stY;
						ThminRect.right = edX;
						ThminRect.bottom = edY;
					}

					if ((float)(dist) < fMinth)
					{
						cv::Mat removeImg, removeImg_inv;
						cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
						cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
						removeImg = removeImg_inv & removeImg;
						ContoursMaxImg = ContoursMaxImg - removeImg;
					}
					else if (dist > nMaxVal)
					{
						nMaxVal = dist;
						ThmaxRect.left = std::min(stX, edX);
						ThmaxRect.top = std::min(stY, edY);
						ThmaxRect.right = std::max(stX, edX);
						ThmaxRect.bottom = std::max(stY, edY);
					}
					maxVal = (double)nMaxVal;
					minVal = (double)nminVal;
				}
				else
				{
					if (nWndDir != 0)
					{
						std::vector<CRect> vBlobRects;
						m_pProcMilAlgo->GetBlobResult_RectsV(vBlobRects);
						if ((nWndDir & 0x01) == 0x01)//top
						{
							if ((nWndDir & 0x04) == 0x04)//left
							{
								//round
								std::vector<std::vector<cv::Point>> contours;
								std::vector<cv::Point> thickPoints;
								double dTmpA = 1.0f;// (double)PartCenter.y / PartCenter.x;
								//double dTmpB = 0;
								for (int nBlobIdx = nLabel; nBlobIdx < nLabel + 1; nBlobIdx++)
								{
									for (int r = vBlobRects[nBlobIdx].bottom - 1; r > vBlobRects[nBlobIdx].top; r--)
									{
										//double dTmpA = (PartCenter.y - r) / (PartCenter.x);
										//double dTmpB = r;
										double dTmpB = r - (dTmpA * vBlobRects[nBlobIdx].left);

										for (int c = vBlobRects[nBlobIdx].left; c < vBlobRects[nBlobIdx].right; c++)
										{
											int y = dTmpA * c + dTmpB;
											if (y < 0 || y >= vBlobRects[nBlobIdx].bottom)
												continue;
											UCHAR* uPtr = LabelImg.ptr(y);
											if (uPtr[c] == pLebel[nLabel])
											{
												thickPoints.push_back(cv::Point(c, y));
												break;
											}
										}
									}
									for (int c = vBlobRects[nBlobIdx].left; c < vBlobRects[nBlobIdx].right; c++)
									{
										//double dTmpA = (PartCenter.y) / (PartCenter.x - c);
										double dTmpB = -dTmpA * c + vBlobRects[nBlobIdx].top;

										for (int r = vBlobRects[nBlobIdx].top; r < vBlobRects[nBlobIdx].bottom; r++)
										{
											int x = (r - dTmpB) / dTmpA;
											if (x < 0 || x >= vBlobRects[nBlobIdx].right)
												continue;
											UCHAR* uPtr = LabelImg.ptr(r);
											if (uPtr[x] == pLebel[nLabel])
											{
												thickPoints.push_back(cv::Point(x, r));
												break;
											}
										}
									}
									contours.push_back(thickPoints);
									thickPoints.clear();
								}
								std::vector<cv::Point> dstPo = thickness(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), pLebel[nLabel]);
								int stX = dstPo[0].x;
								int stY = dstPo[0].y;
								int edX = dstPo[1].x;
								int edY = dstPo[1].y;
#if _DEBUG
								cv::Mat debugImg = ContoursMaxImg.clone();
								cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
								for (int contIdx = 0; contIdx < contours.size(); contIdx++)
								{
									thickPoints = contours[contIdx];
									int nColorDrEn = thickPoints.size() - 1;
									for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
									{
										cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
									}
								}
								cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
								cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif
								//if (edX != 0 && stX != 0 && edY != 0 && stY != 0)
								{
									float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
									if (dist < nminVal)
									{
										nminVal = dist;
										ThminRect.left = stX;
										ThminRect.top = stY;
										ThminRect.right = edX;
										ThminRect.bottom = edY;
									}

									if ((float)(dist) < fMinth)
									{
										cv::Mat removeImg, removeImg_inv;
										cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
										cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
										removeImg = removeImg_inv & removeImg;
										ContoursMaxImg = ContoursMaxImg - removeImg;
									}
									else if (dist > nMaxVal)
									{
										nMaxVal = dist;
										ThmaxRect.left = std::min(stX, edX);
										ThmaxRect.top = std::min(stY, edY);
										ThmaxRect.right = std::max(stX, edX);
										ThmaxRect.bottom = std::max(stY, edY);
									}
								}
								maxVal = (double)nMaxVal;
								minVal = (double)nminVal;
							}
							else if ((nWndDir & 0x08) == 0x08)//right
							{
								//round
								std::vector<std::vector<cv::Point>> contours;
								std::vector<cv::Point> thickPoints;
								double dTmpA = -1.0f;// (double)PartCenter.y / PartCenter.x;
								//double dTmpB = 0;
								for (int nBlobIdx = nLabel; nBlobIdx < nLabel + 1; nBlobIdx++)
								{
									for (int r = vBlobRects[nBlobIdx].bottom - 1; r > vBlobRects[nBlobIdx].top; r--)
									{
										//double dTmpA = (PartCenter.y - r) / (PartCenter.x - ContoursMaxImg.cols - 1);
										double dTmpB = -dTmpA * (vBlobRects[nBlobIdx].right - 1) + r;
										for (int c = vBlobRects[nBlobIdx].right - 1; c >= vBlobRects[nBlobIdx].left; c--)
										{
											int y = dTmpA * c + dTmpB;
											if (y < 0 || y >= vBlobRects[nBlobIdx].bottom)
												continue;
											UCHAR* uPtr = LabelImg.ptr(y);
											if (uPtr[c] == pLebel[nLabel])
											{
												thickPoints.push_back(cv::Point(c, y));
												break;
											}
										}
									}
									for (int c = vBlobRects[nBlobIdx].right - 1; c >= vBlobRects[nBlobIdx].left; c--)
									{
										//double dTmpA = (PartCenter.y) / (PartCenter.x - c);
										double dTmpB = -dTmpA * c + vBlobRects[nBlobIdx].top;

										for (int r = vBlobRects[nBlobIdx].top; r < vBlobRects[nBlobIdx].bottom; r++)
										{
											int x = (r - dTmpB) / dTmpA;
											if (x < 0 || x >= vBlobRects[nBlobIdx].right)
												continue;
											UCHAR* uPtr = LabelImg.ptr(r);
											if (uPtr[x] == pLebel[nLabel])
											{
												thickPoints.push_back(cv::Point(x, r));
												break;
											}
										}
									}
									contours.push_back(thickPoints);
									thickPoints.clear();
								}
								std::vector<cv::Point> dstPo = thickness(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), pLebel[nLabel]);
								int stX = dstPo[0].x;
								int stY = dstPo[0].y;
								int edX = dstPo[1].x;
								int edY = dstPo[1].y;
#if _DEBUG
								cv::Mat debugImg = ContoursMaxImg.clone();
								cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
								for (int contIdx = 0; contIdx < contours.size(); contIdx++)
								{
									thickPoints = contours[contIdx];
									int nColorDrEn = thickPoints.size() - 1;
									for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
									{
										cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
									}
								}
								cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
								cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif
								//if (edX != 0 && stX != 0 && edY != 0 && stY != 0)
								{
									float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
									if (dist < nminVal)
									{
										nminVal = dist;
										ThminRect.left = stX;
										ThminRect.top = stY;
										ThminRect.right = edX;
										ThminRect.bottom = edY;
									}
									if ((float)(dist) < fMinth)
									{
										cv::Mat removeImg, removeImg_inv;
										cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
										cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
										removeImg = removeImg_inv & removeImg;
										ContoursMaxImg = ContoursMaxImg - removeImg;
									}
									else if (dist > nMaxVal)
									{
										nMaxVal = dist;
										ThmaxRect.left = std::min(stX, edX);
										ThmaxRect.top = std::min(stY, edY);
										ThmaxRect.right = std::max(stX, edX);
										ThmaxRect.bottom = std::max(stY, edY);
									}
								}
								maxVal = (double)nMaxVal;
								minVal = (double)nminVal;
							}
						}
						else if ((nWndDir & 0x02) == 0x02)//bot
						{
							if ((nWndDir & 0x04) == 0x04)//left
							{
								//round
								std::vector<std::vector<cv::Point>> contours;
								std::vector<cv::Point> thickPoints;
								double dTmpA = -1.0f;//(double)(PartCenter.y - (ContoursMaxImg.rows - 1)) / PartCenter.x;
								//double dTmpB = ContoursMaxImg.rows - 1;
								for (int nBlobIdx = nLabel; nBlobIdx < nLabel + 1; nBlobIdx++)
								{
									for (int r = vBlobRects[nBlobIdx].top; r < vBlobRects[nBlobIdx].bottom; r++)
									{
										double dTmpB = r - (dTmpA * vBlobRects[nBlobIdx].left);
										for (int c = vBlobRects[nBlobIdx].left; c < vBlobRects[nBlobIdx].right - 1; c++)
										{
											int y = dTmpA * c + dTmpB;
											if (y < 0 || y >= vBlobRects[nBlobIdx].bottom)
												continue;
											UCHAR* uPtr = LabelImg.ptr(y);
											if (uPtr[c] == pLebel[nLabel])
											{
												thickPoints.push_back(cv::Point(c, y));
												break;
											}
										}
									}
									for (int c = vBlobRects[nBlobIdx].left; c < vBlobRects[nBlobIdx].right - 1; c++)
									{
										//double dTmpA = (PartCenter.y - ContoursMaxImg.rows - 1) / (PartCenter.x - c);
										double dTmpB = -dTmpA * c + (vBlobRects[nBlobIdx].bottom - 1);
										for (int r = vBlobRects[nBlobIdx].bottom - 1; r >= vBlobRects[nBlobIdx].top; r--)
										{
											int x = (r - dTmpB) / dTmpA;
											if (x < 0 || x >= vBlobRects[nBlobIdx].right)
												continue;
											UCHAR* uPtr = LabelImg.ptr(r);
											if (uPtr[x] == pLebel[nLabel])
											{
												thickPoints.push_back(cv::Point(x, r));
												break;
											}
										}
									}
									contours.push_back(thickPoints);
									thickPoints.clear();
								}
								std::vector<cv::Point> dstPo = thickness(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), pLebel[nLabel]);
								int stX = dstPo[0].x;
								int stY = dstPo[0].y;
								int edX = dstPo[1].x;
								int edY = dstPo[1].y;
#if _DEBUG
								cv::Mat debugImg = ContoursMaxImg.clone();
								cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
								for (int contIdx = 0; contIdx < contours.size(); contIdx++)
								{
									thickPoints = contours[contIdx];
									int nColorDrEn = thickPoints.size() - 1;
									for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
									{
										cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
									}
								}
								cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
								cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif
								//if (edX != 0 && stX != 0 && edY != 0 && stY != 0)
								{
									float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
									if (dist < nminVal)
									{
										nminVal = dist;
										ThminRect.left = stX;
										ThminRect.top = stY;
										ThminRect.right = edX;
										ThminRect.bottom = edY;
									}
									if ((float)(dist) < fMinth)
									{
										cv::Mat removeImg, removeImg_inv;
										cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
										cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
										removeImg = removeImg_inv & removeImg;
										ContoursMaxImg = ContoursMaxImg - removeImg;
									}
									else if (dist > nMaxVal)
									{
										nMaxVal = dist;
										ThmaxRect.left = std::min(stX, edX);
										ThmaxRect.top = std::min(stY, edY);
										ThmaxRect.right = std::max(stX, edX);
										ThmaxRect.bottom = std::max(stY, edY);
									}
								}
								maxVal = (double)nMaxVal;
								minVal = (double)nminVal;
							}
							else if ((nWndDir & 0x08) == 0x08)//right
							{
								//round
								std::vector<std::vector<cv::Point>> contours;
								std::vector<cv::Point> thickPoints;
								double dTmpA = 1.0f;//(double)(PartCenter.y - (ContoursMaxImg.rows - 1)) / (PartCenter.x - (ContoursMaxImg.cols - 1));
								//double dTmpB = -dTmpA * (ContoursMaxImg.cols - 1) + (ContoursMaxImg.rows - 1);
								for (int nBlobIdx = nLabel; nBlobIdx < nLabel + 1; nBlobIdx++)
								{
									for (int r = vBlobRects[nBlobIdx].top; r < vBlobRects[nBlobIdx].bottom; r++)
									{
										double dTmpB = -dTmpA * (vBlobRects[nBlobIdx].right - 1) + r;
										for (int c = vBlobRects[nBlobIdx].right - 1; c >= vBlobRects[nBlobIdx].left; c--)
										{
											int y = dTmpA * c + dTmpB;
											if (y < 0 || y >= vBlobRects[nBlobIdx].bottom)
												continue;
											UCHAR* uPtr = LabelImg.ptr(y);
											if (uPtr[c] == pLebel[nLabel])
											{
												thickPoints.push_back(cv::Point(c, y));
												break;
											}
										}
									}
									for (int c = vBlobRects[nBlobIdx].right - 1; c >= vBlobRects[nBlobIdx].left; c--)
									{
										//double dTmpA = (PartCenter.y - ContoursMaxImg.rows - 1) / (PartCenter.x - c);
										double dTmpB = -dTmpA * c + (vBlobRects[nBlobIdx].bottom - 1);
										for (int r = vBlobRects[nBlobIdx].bottom - 1; r >= vBlobRects[nBlobIdx].top; r--)
										{
											int x = (r - dTmpB) / dTmpA;
											if (x < 0 || x >= vBlobRects[nBlobIdx].right)
												continue;
											UCHAR* uPtr = LabelImg.ptr(r);
											if (uPtr[x] == pLebel[nLabel])
											{
												thickPoints.push_back(cv::Point(x, r));
												break;
											}
										}
									}
									contours.push_back(thickPoints);
									thickPoints.clear();
								}
								std::vector<cv::Point> dstPo = thickness(LabelImg, nWndDir, PartCenter, contours, cv::Rect(rcBlob.left, rcBlob.top, rcBlob.right - rcBlob.left + 1, rcBlob.bottom - rcBlob.top + 1), pLebel[nLabel]);
								int stX = dstPo[0].x;
								int stY = dstPo[0].y;
								int edX = dstPo[1].x;
								int edY = dstPo[1].y;
#if _DEBUG
								cv::Mat debugImg = ContoursMaxImg.clone();
								cv::cvtColor(debugImg, debugImg, cv::COLOR_GRAY2BGR);
								for (int contIdx = 0; contIdx < contours.size(); contIdx++)
								{
									thickPoints = contours[contIdx];
									int nColorDrEn = thickPoints.size() - 1;
									for (int clrIdx = 0; clrIdx < nColorDrEn; clrIdx++)
									{
										cv::line(debugImg, thickPoints[clrIdx], thickPoints[clrIdx], cv::Scalar(0, 0, 255));
									}
								}
								cv::line(debugImg, dstPo[0], dstPo[1], cv::Scalar(0, 255, 0));
								cv::imwrite("D:\\BlobThickness.bmp", debugImg);
#endif
								//if (edX != 0 && stX != 0 && edY != 0 && stY != 0)
								{
									float dist = std::sqrtf(std::powf((float)edY - (float)stY, 2) + std::powf((float)edX - (float)stX, 2));
									if (dist < nminVal)
									{
										nminVal = dist;
										ThminRect.left = stX;
										ThminRect.top = stY;
										ThminRect.right = edX;
										ThminRect.bottom = edY;
									}
									if ((float)(dist) < fMinth)
									{
										cv::Mat removeImg, removeImg_inv;
										cv::threshold(LabelImg, removeImg, pLebel[nLabel] - 1, 255, cv::THRESH_BINARY);
										cv::threshold(LabelImg, removeImg_inv, pLebel[nLabel] + 1, 255, cv::THRESH_BINARY_INV);
										removeImg = removeImg_inv & removeImg;
										ContoursMaxImg = ContoursMaxImg - removeImg;
									}
									else if (dist > nMaxVal)
									{
										nMaxVal = dist;
										ThmaxRect.left = std::min(stX, edX);
										ThmaxRect.top = std::min(stY, edY);
										ThmaxRect.right = std::max(stX, edX);
										ThmaxRect.bottom = std::max(stY, edY);
									}
								}
								maxVal = (double)nMaxVal;
								minVal = (double)nminVal;
							}
						}

					}
					else
					{
						cv::Mat ThiningValue;
						cv::Mat distTrancf;
						double MinValTmp, MaxValTmp;
						cv::distanceTransform(LabelImg, distTrancf, cv::DIST_L2, 3);
						distTrancf.copyTo(ThiningValue, ThiningImg);
						cv::minMaxLoc(distTrancf, &MinValTmp, &MaxValTmp, &MinPo, &MaxPo);
						MinValTmp = MaxValTmp;
						MinPo = MaxPo;
						for (int r = 0; r < ThiningValue.rows; r++)
						{
							float* fPtr = ThiningValue.ptr<float>(r);
							for (int c = 0; c < ThiningValue.cols; c++)
							{
								if (fPtr[c] > 0 && fPtr[c] < MinValTmp)
								{
									MinValTmp = fPtr[c];
									MinPo.x = c;
									MinPo.y = r;
								}
							}
						}
						maxVal = std::roundf(MaxValTmp * 2 + 1);
						minVal = std::roundf(MinValTmp * 2 + 1);
						int nminVal = (int)std::ceil(MinValTmp);
						int nMaxVal = (int)std::ceil(MaxValTmp);
						//rect표현1 simple
						ThminRect.left = MinPo.x - nminVal;
						ThminRect.top = MinPo.y - nminVal;
						ThminRect.right = MinPo.x + nminVal;
						ThminRect.bottom = MinPo.y + nminVal;

						ThmaxRect.left = MaxPo.x - nMaxVal;
						ThmaxRect.top = MaxPo.y - nMaxVal;
						ThmaxRect.right = MaxPo.x + nMaxVal;
						ThmaxRect.bottom = MaxPo.y + nMaxVal;
					}
				}

			}
			nLabel++;
		}
	}
	return nCntBlob;

	}
	
	int CPInsp_AlgoBlob::convertDirectionFlag(int dirFlag) {
		bool top = dirFlag & 1;
		bool bot = dirFlag & 2;
		bool left = dirFlag & 4;
		bool right = dirFlag & 8;

		bool vertical = top || bot;
		bool horizontal = left || right;

		if (horizontal && !vertical)
			return 0; // 좌우만

		if (left && top)
			return 1;
		if (right && top)
			return 2;
		if (left && bot)
			return 3;
		if (right && bot)
			return 4;

		if (!horizontal && vertical)
			return 5; // 상하만

		return -1; // 유효하지 않은 조합
	}