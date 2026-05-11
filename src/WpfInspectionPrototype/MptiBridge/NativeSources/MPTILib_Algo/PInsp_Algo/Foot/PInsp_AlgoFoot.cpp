//#include "stdafx.h"
#include "PInsp_AlgoFoot.h"
#include <ctime>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
////static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

//#define CheckValidationImage	1

CPInsp_AlgoFoot::CPInsp_AlgoFoot(void)
{
}


CPInsp_AlgoFoot::~CPInsp_AlgoFoot(void)
{
}

void CPInsp_AlgoFoot::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoFoot::GetInspAlgoData()
{
	return eSPCAlgoFoot;
}

int CPInsp_AlgoFoot::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoFoot * rstAlgo = (RstAlgoFoot *)vRstInspAlgo;
	if (rstAlgo->m_bFind == FALSE)
	{
		eWholeNgTypeTemp[TypeMountMissing] = e_NG;
		nCurrentNgType = TypeMountMissing;
	}
	//for (int a = 0; a < rstAlgo->m_nFootCnt; a++)
	{
		if (!rstAlgo->m_bArrOK[m_eFoot_Deviation] || !rstAlgo->m_bArrOK[m_eFoot_DeviationX] ||
			!rstAlgo->m_bArrOK[m_eFoot_DeviationY])
		{
			eWholeNgTypeTemp[TypeMountShift] = e_NG;
			nCurrentNgType = TypeMountShift;
		}
		if (!rstAlgo->m_bArrOK[m_eFoot_Damage] || !rstAlgo->m_bArrOK[m_eFoot_Width_1] || !rstAlgo->m_bArrOK[m_eFoot_Width_1_H]
			|| !rstAlgo->m_bArrOK[m_eFoot_Length_1]
			|| !rstAlgo->m_bArrOK[m_eFoot_Wedge_W] || !rstAlgo->m_bArrOK[m_eFoot_Wedge_L]
			|| !rstAlgo->m_bArrOK[m_eFoot_Tail_L] || !rstAlgo->m_bArrOK[m_eFoot_Tail_A]
			|| !rstAlgo->m_bArrOK[m_eFoot_WingArea] || !rstAlgo->m_bOK)
		{
			eWholeNgTypeTemp[TypeMountWorng] = e_NG;
			nCurrentNgType = TypeMountWorng;
		}
		if (!rstAlgo->m_bArrOK[m_eFoot_Height] || !rstAlgo->m_bArrOK[m_eFoot_HeelCrack])
		{
			eWholeNgTypeTemp[TypeMountLift] = e_NG;
			nCurrentNgType = TypeMountLift;
		}
		if (!rstAlgo->m_bFind)
		{
			eWholeNgTypeTemp[TypeMountMissing] = e_NG;
			nCurrentNgType = TypeMountMissing;
		}
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoFoot::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

cv::Mat CPInsp_AlgoFoot::FillPolygonImg(cv::Mat ucMask, cv::Mat binImg, int width, int height, POINT* ptDstPoint, bool isExceptROI, int nUsedInspPolygon, int* cnt, UCHAR* ucArrOverlapImg, int nChanel, int nExcept)
{
	cv::Scalar black(0, 0, 0);
	cv::Scalar white(255, 255, 255);

	cv::Mat img;
	cv::Mat mask;
	cv::Mat OverlapImg;
	vector<cv::Point> contour;

	CString str;

	for (int i = 0; i < nUsedInspPolygon; i++)
	{
		if ((ptDstPoint[i].x >= 0 && ptDstPoint[i].y >= 0) && (ptDstPoint[i].x <= width && ptDstPoint[i].y <= height))
			contour.push_back(cv::Point(ptDstPoint[i].x, ptDstPoint[i].y));
	}

	const cv::Point *pts3 = (cv::Point*) cv::Mat(contour).data;
	int npts3 = cv::Mat(contour).rows;
	int ZeroCnt = 0;
	bool isExceptROIRect = false;
	if (*cnt == 1)
	{
		isExceptROIRect = true;
		//npts3 = 4;
	}
	if (ucArrOverlapImg != NULL)
	{
		int nColorTye = nChanel == 3 ? 0 : 1;
		OverlapImg = m_pProcMilAlgo->BytesToMat(ucArrOverlapImg, width, height, nColorTye);
	}

	if (isExceptROI)
	{
		if (!isExceptROIRect)
		{
			img = cv::Mat(height, width, CV_8UC(nChanel), white);
			cv::fillPoly(img, &pts3, &npts3, 1, black);
			if (ucArrOverlapImg != NULL)
				cv::fillPoly(OverlapImg, &pts3, &npts3, 1, black);
		}
		else
		{
			img = cv::Mat(height, width, CV_8UC(nChanel), white);
			for (int i = 0; i < nUsedInspPolygon; i += 4)
			{
				cv::rectangle(img, pts3[i], pts3[i + 2], black, -1);
				if (ucArrOverlapImg != NULL)
					cv::rectangle(OverlapImg, pts3[i], pts3[i + 2], black, -1);
			}
		}
		// 		str.Format(_T("isExceptROI"));
		// 		g_pMPTI->AddLog_Dev(str);
	}
	else
	{
		img = cv::Mat(height, width, CV_8UC(nChanel), black);
		fillPoly(img, &pts3, &npts3, 1, white);
		if (ucArrOverlapImg != NULL)
			fillPoly(OverlapImg, &pts3, &npts3, 1, white);
		// 		str.Format(_T("polygon region"));
		// 		g_pMPTI->AddLog_Dev(str);
	}
	//ucMask = img.clone();
	if (ucMask.size == img.size)
	{
		mask = cv::Mat(height, width, CV_8UC(nChanel), black);
		mask = img.clone();
		if (nExcept == 1)
			cv::bitwise_and(img, ucMask, mask);
		// 		str.Format(_T("m_nUsedWndPolygon : %d, m_ptArrWndPolygon : %lf,%lf"), nUsedInspPolygon, ptDstPoint[0].x, ptDstPoint[0].y);
		// 		g_pMPTI->AddLog_Dev(str);
	}
	memcpy(ucMask.data, mask.data, width*height * sizeof(byte)*nChanel);

	if (binImg.size == img.size)
	{
		cv::bitwise_and(img, binImg, img);
		// 		str.Format(_T("m_nUsedWndPolygon : %d, m_ptArrWndPolygon : %lf,%lf"), nUsedInspPolygon, ptDstPoint[0].x, ptDstPoint[0].y);
		// 		g_pMPTI->AddLog_Dev(str);
	}
	if (ucArrOverlapImg != NULL)
		memcpy(ucArrOverlapImg, m_pProcMilAlgo->MatToBytes(OverlapImg, ucArrOverlapImg), width*height * sizeof(byte)*nChanel);
	*cnt = ZeroCnt;
	binImg = img;
	pts3 = nullptr;

	img.release();
	OverlapImg.release();

	return binImg;
}


BOOL CPInsp_AlgoFoot::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();

	CPInsp_WireBonding* PInspWireBonding = g_pInspMng->GetWireBondingInsp();

	AlgoFoot *pAlgoFoot = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;
	bool bUsePadAreaAutoteach = false;

	bUsePadAreaAutoteach = pAlgoFoot->m_bUsePadAreaAutoTeach;

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

	cv::Mat cvDBCPolygonImg2(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1);
	cvDBCPolygonImg2.setTo(0);

	if (stTieArea.m_nUsedInspPolygon > 0)
	{
		CString msg_SetDBCPolygon;
		msg_SetDBCPolygon.Format(_T("[FootInspAlgo]::InspAlgorithm()_MakeDBCPolygonImg"));
		g_pMPTI->AddLog(msg_SetDBCPolygon);

		POINT* ptDstPoint = g_pMManager->pem_new<POINT>(true, stTieArea.m_nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);
		POINTF* ptfDstPoint = g_pMManager->pem_new<POINTF>(true, stTieArea.m_nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);

		for (int a = 0; a < stTieArea.m_nUsedInspPolygon; a++)
		{
			ptfDstPoint[a].x = stTieArea.m_ptArrInspPolygon[a].x + (sWndAlgoImg.m_nWidth / 2.);
			ptfDstPoint[a].y = stTieArea.m_ptArrInspPolygon[a].y + (sWndAlgoImg.m_nHeight / 2.);

			if (ptfDstPoint[a].x < 0)
				ptfDstPoint[a].x = 0;
			if (ptfDstPoint[a].y < 0)
				ptfDstPoint[a].y = 0;
			if (ptfDstPoint[a].x > sWndAlgoImg.m_nWidth)
				ptfDstPoint[a].x = sWndAlgoImg.m_nWidth;
			if (ptfDstPoint[a].y > sWndAlgoImg.m_nHeight)
				ptfDstPoint[a].y = sWndAlgoImg.m_nHeight;

			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}

		int nColorType = 1;
		int nChanel = 1;
		cv::Mat src(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1);
		cv::Mat ucMask2(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1);
		bool bConvertExceptROI = false;
		int nUsedInspPolygon = stTieArea.m_nUsedInspPolygon;
		int *ZeroCnt = (int*)g_pMManager->pem_malloc(sizeof(int), (PCHAR)__FUNCTION__, __LINE__);
		*ZeroCnt = 1;

		
		int nExcept = 1;

		src = FillPolygonImg(ucMask2, src, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, ptDstPoint,
			bConvertExceptROI, nUsedInspPolygon, ZeroCnt, cvDBCPolygonImg2.data, nChanel, nExcept);

		if (ptDstPoint)
			//delete[] ptDstPoint;
			g_pMManager->pem_delete(ptDstPoint, true);
		ptDstPoint = NULL;

		if (ptfDstPoint)
			//delete[] ptfDstPoint;
			g_pMManager->pem_delete(ptfDstPoint, true);
		ptfDstPoint = NULL;

		g_pMManager->pem_free(ZeroCnt);
		ZeroCnt = NULL;

	}

	if (stAlgoParam.m_bInspection)
	{
		cv::Mat cvimg(0, 0, CV_32FC1);

		CString msg;
		msg.Format(_T("[FootInspAlgo]::InspAlgorithm()_MakeFoot()"));
		g_pMPTI->AddLog(msg);

		CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, sWndAlgoImg, stAlgoParam.m_dx, stAlgoParam.m_dy, *stAlgoParam.m_ptrAlgoColorOpt, &cvimg, &cvDBCPolygonImg2, true, bUsePadAreaAutoteach);
		if (pFoot == NULL)
			return bResult;


		CString sWndAlgoName=NULL;
		
#ifdef CheckValidationImage
/*#ifdef _DEBUG */
		{
			//window ROI 디버깅 이미지 저장 
			int nPartWidth = stAlgoParam.m_ptrAlgoColorOpt->m_sFovImg.nImageSizeX;
			int nPartHeight = stAlgoParam.m_ptrAlgoColorOpt->m_sFovImg.nImageSizeY;

			cv::Mat cvPartImage(nPartHeight, nPartWidth, CV_8UC1, stAlgoParam.m_ptrAlgoColorOpt->m_sFovImg.imgTop_R);
			cv::Mat cvPart_WindowROICheck(nPartHeight, nPartWidth, CV_8UC3);
			cvPart_WindowROICheck = cvPartImage.clone();

			int nWndStX = stAlgoParam.m_ptrAlgoColorOpt->m_rcImageRect.left;		//dx
			int nWndStY = stAlgoParam.m_ptrAlgoColorOpt->m_rcImageRect.top;			//dy
			int nWndWidth = stAlgoParam.m_ptrAlgoColorOpt->m_rcImageRect.right - stAlgoParam.m_ptrAlgoColorOpt->m_rcImageRect.left;
			int nWndHeight = stAlgoParam.m_ptrAlgoColorOpt->m_rcImageRect.bottom - stAlgoParam.m_ptrAlgoColorOpt->m_rcImageRect.top;

			cv::cvtColor(cvPart_WindowROICheck, cvPart_WindowROICheck, cv::COLOR_GRAY2RGB);
			cv::rectangle(cvPart_WindowROICheck, cv::Rect(nWndStX, nWndStY, nWndWidth, nWndHeight), cv::Scalar(0, 0, 255), 1, 8, 0);

			int nAlgoInspRectStX = pFoot->mImageRect.left;
			int nAlgoInspRectStY = pFoot->mImageRect.top;
			int nAlgoInspRectWidth = pFoot->mImageRect.right - pFoot->mImageRect.left;
			int nAlgoInspRectHeight = pFoot->mImageRect.bottom - pFoot->mImageRect.top;
			cv::rectangle(cvPart_WindowROICheck, cv::Rect(nAlgoInspRectStX, nAlgoInspRectStY, nAlgoInspRectWidth, nAlgoInspRectHeight), cv::Scalar(0, 255, 0), 1, 8, 0);

			int nAlgoSpecRectStX = pFoot->m_PadRect.left;
			int nAlgoSpecRectStY = pFoot->m_PadRect.top;
			int nAlgoSpecRectWidth = pFoot->m_PadRect.right - pFoot->m_PadRect.left;
			int nAlgoSpecRectHeight = pFoot->m_PadRect.bottom - pFoot->m_PadRect.top;
			cv::rectangle(cvPart_WindowROICheck, cv::Rect(nAlgoSpecRectStX, nAlgoSpecRectStY, nAlgoSpecRectWidth, nAlgoSpecRectHeight), cv::Scalar(255, 105, 180), 1, 8, 0);

			//create dir name
			time_t rawtime = time(nullptr);
			struct tm *curr_tm;
			curr_tm = localtime(&rawtime);

			int curr_year = curr_tm->tm_year + 1900;
			int curr_month = curr_tm->tm_mon + 1;
			int curr_day = curr_tm->tm_mday;
			int curr_hour = curr_tm->tm_hour, curr_minute = curr_tm->tm_min, curr_second = curr_tm->tm_sec;

			CString sValidateImgTime;
			sValidateImgTime.Format(_T("D:\\FootRst2"));
			if (PathFileExists(sValidateImgTime) == false)
				CreateDirectory(sValidateImgTime, NULL);

			CString sInspTime;
			sInspTime.Format(_T("D:\\FootRst2\\%d-%d-%d"), curr_year, curr_month, curr_day);
			if (PathFileExists(sInspTime) == false)
				CreateDirectory(sInspTime, NULL);

			CString sPartName = _T("");
			if (wcslen(pInspBoardInfo->PartNo) > 0)
			{
				sPartName.Format(_T("%s\\%s"), sInspTime, pInspBoardInfo->PartNo);
				if (PathFileExists(sPartName) == false)
					CreateDirectory(sPartName, NULL);
			}
			else
			{
				sPartName.Format(_T("%s\\%s"), sInspTime, L"null");
				if (PathFileExists(sPartName) == false)
					CreateDirectory(sPartName, NULL);
			}
			

			sWndAlgoName.Format(_T("%s\\%d_%d.bmp"), sPartName, stAlgoParam.m_nWndIndex, sInspAlgo.m_nAlgoId);

			CT2CA strWndAlgoName(sWndAlgoName);
			std::string strFootAlgoName(strWndAlgoName);

			cv::imwrite(strFootAlgoName, cvPart_WindowROICheck);
			////////////////////////////////
		}
/*#endif*/
#endif

		CString msg2;
		msg2.Format(_T("[FootInspAlgo]::InspAlgorithm()_InspFoot()"));
		g_pMPTI->AddLog(msg2);

		bResult = PInspWireBonding->InspFoot(pFoot, sWndAlgoImg, (RstAlgoFoot *)sRstAlgo, stTieArea, *stAlgoParam.m_ptrAlgoColorOpt, &cvimg, -1, NULL, stAlgoParam.m_sAlignRes, false, sWndAlgoName);
		//bResult = PInspWireBonding->InspFoot(pFoot, sWndAlgoImg, (RstAlgoFoot *)sRstAlgo, stTieArea, *stAlgoParam.m_ptrAlgoColorOpt, -1, NULL, stAlgoParam.m_sAlignRes, false, sWndAlgoName);

		if (pFoot)
		{
			g_pMManager->pem_delete(pFoot, false);
			pFoot = NULL;
		}

		RstAlgoFoot * sRst = (RstAlgoFoot *)sRstAlgo;
		if (sRst->m_sPoint.x > 0 || sRst->m_sPoint.y > 0)
		{
			sRst->m_sEnd.x = sRst->m_sPoint.x + stAlgoParam.m_dx;
			sRst->m_sEnd.y = sRst->m_sPoint.y + stAlgoParam.m_dy;
		}
	}
	else
	{
		int nTabIndex = stAlgoParam.m_nSelectArea;
		int nStartX = stAlgoParam.m_nStartX;
		int nStartY = stAlgoParam.m_nStartY;
		if (nTabIndex < m_eFootBin_Wedge)
		{
			cv::Mat cvimg(0, 0, CV_32FC1);
			if (true) 
			{
				CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, sWndAlgoImg, nStartX, nStartY, *stAlgoParam.m_ptrAlgoColorOpt, &cvimg, &cvDBCPolygonImg2, true, bUsePadAreaAutoteach);
				if (pFoot == nullptr) // MakeFoot ??? ?? ???
				{
					RstAlgoFoot * sRst = (RstAlgoFoot *)sRstAlgo;
					sRst->m_fArrRst[m_eFoot_Height] = -999; // NG ??? ??
					sRst->m_bArrOK[m_eFoot_Height] = FALSE;
					return false;
				}
			}

			bResult = PInspWireBonding->TeachFootColor(sInspAlgo, sWndAlgoImg, nStartX, nStartY, *stAlgoParam.m_ptrAlgoColorOpt, nTabIndex, *stAlgoParam.m_fcBackground, *stAlgoParam.m_fcFoot, *stAlgoParam.m_fcWing, *stAlgoParam.m_fcWedge, *stAlgoParam.m_fcFindDBC, ucArrDstImg, &cvimg);
		}
		else
		{
			cv::Mat cvimg(0, 0, CV_32FC1);
			CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, sWndAlgoImg, nStartX, nStartY, *stAlgoParam.m_ptrAlgoColorOpt, &cvimg, &cvDBCPolygonImg2, true, bUsePadAreaAutoteach);
			if (pFoot)
			{
				pFoot->m_rst_Color_Image = true;
				pFoot->m_fcBackground = *stAlgoParam.m_fcBackground;
				pFoot->m_fcFoot = *stAlgoParam.m_fcFoot;
				pFoot->m_fcWing = *stAlgoParam.m_fcWing;
				pFoot->m_fcWedge = *stAlgoParam.m_fcWedge;
				pFoot->m_fcFindDBC = *stAlgoParam.m_fcFindDBC;
				pFoot->bTeachBin = stAlgoParam.m_bTeach;

				bResult = PInspWireBonding->InspFoot(pFoot, sWndAlgoImg, (RstAlgoFoot *)sRstAlgo, stTieArea, *stAlgoParam.m_ptrAlgoColorOpt, &cvimg, nTabIndex, ucArrDstImg, NULL, stAlgoParam.m_bTeachingFunc);

				g_pMManager->pem_delete(pFoot, false);
				pFoot = NULL;
			}
			else if (pFoot == nullptr) // MakeFoot ??? ?? ???
			{
				((RstAlgoFoot *)sRstAlgo)->m_fArrRst[m_eFoot_Height] = -999; // NG ??? ??
				((RstAlgoFoot *)sRstAlgo)->m_bArrOK[m_eFoot_Height] = FALSE;
			}
		}
	}

	CString msg3;
	msg3.Format(_T("[FootInspAlgo]::InspAlgorithm()_EndAlgo()"));
	g_pMPTI->AddLog(msg3);

	return bResult;
}

BOOL CPInsp_AlgoFoot::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoFoot::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoFoot::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}