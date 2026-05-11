#include "PInsp_AlgoFillet.h"


CPInsp_AlgoFillet::CPInsp_AlgoFillet(void)
{
}


CPInsp_AlgoFillet::~CPInsp_AlgoFillet(void)
{
}

void CPInsp_AlgoFillet::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoFillet::GetInspAlgoData()
{
	return eSPCAlgoFillet;
}

int CPInsp_AlgoFillet::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
	nCurrentNgType = TypeSolderFilet;
	return nCurrentNgType;
}

bool CPInsp_AlgoFillet::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return true;
	return true;
}

BOOL CPInsp_AlgoFillet::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	int nWndDir = stAlgoParam.m_nWndDir;
	bResult = InspFillet(sInspAlgo, sWndAlgoImg, nWndDir, (RstAlgoFillet *)sRstAlgo);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;

	//Set Insp ROI 
	RstAlgoFillet * rstFillet = (RstAlgoFillet *)sRstAlgo;
	rstFillet->m_rcRect_I.left += stAlgoParam.m_dx;
	rstFillet->m_rcRect_I.right += stAlgoParam.m_dx + nImgWidth;
	rstFillet->m_rcRect_I.top += stAlgoParam.m_dy;
	rstFillet->m_rcRect_I.bottom += stAlgoParam.m_dy + nImgHeight;

	rstFillet->m_nDir = nWndDir;

	return bResult;
}

BOOL CPInsp_AlgoFillet::CreateFilletHeightLine(WndAlgoImg &sWndAlgoImg, std::vector<POINTF> &arrDstFilletLinePoint, int &nFilletLineCnt, int nTipDirection)
{
	BOOL bResult = FALSE;

	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	UCHAR *pUcOrgSrc = sWndAlgoImg.m_ucArr2D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;

	cv::Mat InspImage(nImgHeight, nImgWidth, CV_8UC1, pUcOrgSrc);
	cv::Mat fHRectImg(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);

	CString ImagePath;
#if _DEBUG
	ImagePath.Format(_T("D:\\testimage\\workimage\\InspFilletImg.bmp"));	//D:\testimage\workimage
	cv::imwrite(std::string(CT2A(ImagePath)), InspImage);

	cv::imwrite("D:\\testimage\\workimage\\InspFilletImage_3D.bmp", fHRectImg);
#endif

	//Save Insp Image
	//m_pProcMilAlgo->SaveWorkImg(pUcOrgSrc, nImgWidth, nImgHeight, _T("Org2D.bmp"));
	//m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Org3D.bmp"));

	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	int nW = nImgWidth;
	int nH = nImgHeight;


	int nSum = 0;
	double dAverage = 0;
	int nLineCnt = 0;

	arrDstFilletLinePoint.clear();

	nFilletLineCnt = 0;

	if (nTipDirection == 2 || nTipDirection == 3) // Top,Bottom  
	{
		for (int r = 0; r < nH; r++)
		{
			nSum = 0;
			dAverage = 0;
			nLineCnt = 0;

			int nLeft = (int)(nW * 0.3);
			int nRight = nW - nLeft;

			for (int c = nLeft; c < nRight; c++)
			{
				int nIndex = (r*nW) + c;
				if (nTipDirection == 3)
					nIndex = (nH - r - 1)*nW + c;

				nSum = nSum + pfImgSrc[nIndex];
				nLineCnt++;
			}

			if (nLineCnt > 0)
				dAverage = (double)(nSum / nLineCnt);

			POINTF FilletLinePos;
			FilletLinePos.x = r;
			FilletLinePos.y = dAverage;
			arrDstFilletLinePoint.push_back(FilletLinePos);

			nFilletLineCnt++;
		}
	}
	else
	{
		for (int r = 0; r < nW; r++)
		{
			nSum = 0;
			dAverage = 0;
			nLineCnt = 0;

			int nLeft = (int)(nH * 0.3);
			int nRight = nH - nLeft;

			for (int c = nLeft; c < nRight; c++)
			{
				int nIndex = (c*nW) + r;
				if (nTipDirection == 1)
					nIndex = (c*nW) + (nW - r - 1);

				nSum = nSum + pfImgSrc[nIndex];
				nLineCnt++;
			}

			if (nLineCnt > 0)
				dAverage = (double)(nSum / nLineCnt);

			POINTF FilletLinePos;
			FilletLinePos.x = r;
			FilletLinePos.y = dAverage;
			arrDstFilletLinePoint.push_back(FilletLinePos);

			nFilletLineCnt++;
		}

	}

	return bResult;
}
BOOL CPInsp_AlgoFillet::CreateFilletHighestHeightLine(WndAlgoImg& sWndAlgoImg, std::vector<POINTF>& arrDstFilletHighestLinePoint, int nTipDirection)
{
	BOOL bResult = FALSE;

	float* pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;

	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;

	int nW = nImgWidth;
	int nH = nImgHeight;


	int nMax = 0;
	double dMax = 0;
	int nLineCnt = 0;

	arrDstFilletHighestLinePoint.clear();

	
	if (nTipDirection == 2 || nTipDirection == 3) // Top,Bottom  
	{
		for (int r = 0; r < nH; r++)
		{
			nLineCnt = 0;
			nMax = -1000;
			dMax = 0;

			int nLeft = (int)(nW * 0.3);
			int nRight = nW - nLeft;

			for (int c = nLeft; c < nRight; c++)
			{
				int nIndex = (r * nW) + c;
				if (nTipDirection == 3)
					nIndex = (nH - r - 1) * nW + c;

				if (pfImgSrc[nIndex] > nMax)
				{
					nMax = pfImgSrc[nIndex];
				}
				nLineCnt++;
			}

			if (nLineCnt > 0)
				dMax = (double)nMax;

			POINTF FilletLinePos;
			FilletLinePos.x = r;
			FilletLinePos.y = dMax;
			arrDstFilletHighestLinePoint.push_back(FilletLinePos);
		}
	}
	else
	{
		for (int r = 0; r < nW; r++)
		{
			nLineCnt = 0;
			nMax = -1000;
			dMax = 0;

			int nLeft = (int)(nH * 0.3);
			int nRight = nH - nLeft;

			for (int c = nLeft; c < nRight; c++)
			{
				int nIndex = (c * nW) + r;
				if (nTipDirection == 1)
					nIndex = (c * nW) + (nW - r - 1);

				if (pfImgSrc[nIndex] > nMax)
				{
					nMax = pfImgSrc[nIndex];
				}
				nLineCnt++;
			}

			if (nLineCnt > 0)
				dMax = (double)nMax;

			POINTF FilletLinePos;
			FilletLinePos.x = r;
			FilletLinePos.y = dMax;
			arrDstFilletHighestLinePoint.push_back(FilletLinePos);
		}

	}

	return bResult;
}
//MesurementSpec();
BOOL CPInsp_AlgoFillet::MesurementSpec(AlgoFillet pInspAlgoFillet, WndAlgoImg &sWndAlgoImg, std::vector<POINTF> arrDstFilletLinePoint, std::vector<float> &arrRstFilletHeight, int nFilletLineCnt, int nTipDirection)
{
	//confirm height/angle value
	BOOL bResult = FALSE;

	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;


	cv::Mat InspROI3D_ORG(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);

	int nW = nImgWidth;
	int nH = nImgHeight;
	bool bIsHorizon = true;

	//pInspAlgoFillet.m_fArrHeightOptionValue[][]

	std::vector<float> vecFilletPointRstHeight;
	vecFilletPointRstHeight.clear();

	for (int nIdx = 0; nIdx < pInspAlgoFillet.m_nDivisionCounts; nIdx++)
	{
		float fHeightDefault = pInspAlgoFillet.m_fArrHeightOptionValue[nIdx][eMMD_Default];

		if (fHeightDefault == 0.0f)
		{
			break;
			return false;
		}

		float fRstHeight = 0;
		//float fRstHeight = sRstData->m_dArrRstHeight[nIdx];

		float fIndexX = 0.0f;
		for (int i = 0; i < nFilletLineCnt; i++)
		{
			POINTF FilletLinePo = arrDstFilletLinePoint[i];
			if (fHeightDefault == FilletLinePo.y)
			{
				fIndexX = FilletLinePo.x;
				vecFilletPointRstHeight.push_back(FilletLinePo.y);
			}

		}

	}

	//arrRstFilletHeight = vecFilletPointRstHeight;
	arrRstFilletHeight.assign(pInspAlgoFillet.m_nDivisionCounts, 0.0f);

	for (int a = 0; a < pInspAlgoFillet.m_nDivisionCounts; a++)
	{
		arrRstFilletHeight[a] = vecFilletPointRstHeight[a];
	}

	return bResult;
}

BOOL CPInsp_AlgoFillet::InspFillet(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, int nWndDir, RstAlgoFillet *sRstAlgo)
{
	BOOL bResult = FALSE;
	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	if ((pucImgSrc == NULL) || (pfImgSrc == NULL) || !m_pProcMilAlgo || (nImgWidth <= 0) || (nImgHeight <= 0) || sInspAlgo.m_eAlgoType != eAlgoFillet)
		return bResult;

	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, (sizeof(RstAlgoFillet)));
		sRstAlgo->Init();
	}

	AlgoFillet *pInspAlgoFillet = (AlgoFillet *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pInspAlgoFillet)
		return bResult;

	int nSizeImg = nImgWidth * nImgHeight;

	CRect rcInspFilletROI(0, 0, 0, 0);
	rcInspFilletROI.left = 0; rcInspFilletROI.right = 0; rcInspFilletROI.top = 0; rcInspFilletROI.bottom = 0;
	
	std::vector<POINTF> arrFilletLinePoint;
	std::vector<POINTF> arrFilletLineHighestPoint;
	int nFilletLineCnt = 0;


	CreateFilletHighestHeightLine(sWndAlgoImg, arrFilletLineHighestPoint, nWndDir);

	CreateFilletHeightLine(sWndAlgoImg, arrFilletLinePoint, nFilletLineCnt, nWndDir);

	int nW = nImgWidth;
	int nH = nImgHeight;
	bool bIsHorizon = true;

	// 장축이 아닌 단축을 분할하면 됨
	// 장축인지 단축인지에 대해 
	// 픽셀 값으로 저장을 하기 때문에 
	// Gap을 더하고, interval 만큼 측정하고
	int nSeperateLineWidth = 0;
	if (nW > nH)
		nSeperateLineWidth = nH;
	else
		nSeperateLineWidth = nW;

	//um단위로 받음
	double dGapPosX = pInspAlgoFillet->m_dGap;
	double dIntervalPosX = pInspAlgoFillet->m_dInterval;

	double dGapUM = dGapPosX;      //UM 단위로 입력
	double dGapMM = 0.0;
	int nGapPixel = 0.0;

	double dIntervalUM = dIntervalPosX;      //UM 단위로 입력
	double dIntervalMM = 0.0;
	int nIntervalPixel = 0.0;

	dGapMM = dGapUM / 1000.0;
	dIntervalMM = dIntervalUM / 1000.0;

	if (pInspAlgoFillet->m_nTipDirection == 0 || pInspAlgoFillet->m_nTipDirection == 1)
		nGapPixel = (int)RounD(dGapMM / m_resolX);	//um 단위로 저장
	else if (pInspAlgoFillet->m_nTipDirection == 2 || pInspAlgoFillet->m_nTipDirection == 3)
		nGapPixel = (int)RounD(dGapMM / m_resolY);

	if (pInspAlgoFillet->m_nTipDirection == 0 || pInspAlgoFillet->m_nTipDirection == 1)
		nIntervalPixel = (int)RounD(dIntervalMM / m_resolX);	//um 단위로 저장
	else if (pInspAlgoFillet->m_nTipDirection == 2 || pInspAlgoFillet->m_nTipDirection == 3)
		nIntervalPixel = (int)RounD(dIntervalMM / m_resolY);

	int nGapPixelPosX = nGapPixel;
	int nIntervalPosX = nIntervalPixel;

	int index = 0;
	int nSeperatePointCnt = 0;		//분할 포인트 지점 개수

	std::vector<POINTF> ptFilletPointPosList;
	std::vector<POINTF> ptFilletHighestPointPosList;
	POINTF poSeperatePointInfo;
	poSeperatePointInfo.x = 0.0f;
	poSeperatePointInfo.y = 0.0f;

	POINTF poSeperatePointHighestInfo;
	poSeperatePointHighestInfo.x = 0.0f;
	poSeperatePointHighestInfo.y = 0.0f;

	float fSamplingRate = 0.0f;
	int nSamplingRate = 1;

	int nPointCntAfterSampling = 0;
	bool bUseLineSampling = false;

	if (nFilletLineCnt > FILLET_LINE_COUNT)
	{
		bUseLineSampling = true;

		//Sampling 진행 - 일정한 간격으로
		fSamplingRate = (float)nFilletLineCnt / (float)FILLET_LINE_COUNT;
		nSamplingRate = RounDF(fSamplingRate);

		int nGap = nFilletLineCnt - FILLET_LINE_COUNT;
		for (int nIdx = 1; nIdx <= FILLET_LINE_COUNT; nIdx++)
		{
			if ((nSamplingRate)* nIdx - 1 > (nFilletLineCnt - 1))
				break;

			POINTF FilletLinePo = arrFilletLinePoint[(nSamplingRate)* nIdx - 1];
			POINTF FilletLineHighestPo = arrFilletLineHighestPoint[(nSamplingRate)*nIdx - 1];
			if (nWndDir == 0 || nWndDir == 1)
				sRstAlgo->m_fArrLineHeightAvg[nIdx - 1].x = FilletLinePo.x * m_resolX * 1000.0f;	//um 단위로 저장
			else if (nWndDir == 2 || nWndDir == 3)
				sRstAlgo->m_fArrLineHeightAvg[nIdx - 1].x = FilletLinePo.x * m_resolY * 1000.0f;	//um 단위로 저장

			sRstAlgo->m_fArrLineHeightAvg[nIdx - 1].y = FilletLinePo.y;

			if (nWndDir == 0 || nWndDir == 1)
				sRstAlgo->m_fArrLineHeightHighest[nIdx - 1].x = FilletLineHighestPo.x * m_resolX * 1000.0f;	//um 단위로 저장
			else if (nWndDir == 2 || nWndDir == 3)
				sRstAlgo->m_fArrLineHeightHighest[nIdx - 1].x = FilletLineHighestPo.x * m_resolY * 1000.0f;	//um 단위로 저장

			sRstAlgo->m_fArrLineHeightHighest[nIdx - 1].y = FilletLineHighestPo.y;

			nPointCntAfterSampling++;
		}
		sRstAlgo->m_nFilletLineCnt = nFilletLineCnt = nPointCntAfterSampling;
	}
	else
	{
		for (int nIdx = 0; nIdx < nFilletLineCnt; nIdx++)
		{
			POINTF FilletLinePo = arrFilletLinePoint[nIdx];
			POINTF FilletLineHighestPo = arrFilletLineHighestPoint[nIdx];

			if (nWndDir == 0 || nWndDir == 1)
				sRstAlgo->m_fArrLineHeightAvg[nIdx].x = FilletLinePo.x * m_resolX * 1000.0f;
			else if (nWndDir == 2 || nWndDir == 3)
				sRstAlgo->m_fArrLineHeightAvg[nIdx].x = FilletLinePo.x * m_resolY * 1000.0f;

			sRstAlgo->m_fArrLineHeightAvg[nIdx].y = FilletLinePo.y;

			if (nWndDir == 0 || nWndDir == 1)
				sRstAlgo->m_fArrLineHeightHighest[nIdx].x = FilletLineHighestPo.x * m_resolX * 1000.0f;	//um 단위로 저장
			else if (nWndDir == 2 || nWndDir == 3)
				sRstAlgo->m_fArrLineHeightHighest[nIdx].x = FilletLineHighestPo.x * m_resolY * 1000.0f;	//um 단위로 저장

			sRstAlgo->m_fArrLineHeightHighest[nIdx].y = FilletLineHighestPo.y;
		}
		sRstAlgo->m_nFilletLineCnt = nFilletLineCnt;
	}

	ptFilletPointPosList.clear();
	ptFilletHighestPointPosList.clear();
	
	//분할된 포인트 위치에서의 실제 높이 값 측정
	for (int i = 0; i < FILLET_LINE_COUNT; i++)
	{
		index = nFilletLineCnt - ((nGapPixelPosX / nSamplingRate) + ((nIntervalPosX / nSamplingRate)*i));		//첫 index 값은 (gap+itv)9+16, 다음이 9+32, 9+48.... 만약 index가 100을 넘어버리면 break;

		if (index < 0)
			break;
		if (index > nFilletLineCnt - 1)
			index = nFilletLineCnt - 1;


		float fFilletPointHighestHeight = sRstAlgo->m_fArrLineHeightHighest[index].y;	//fFilletPointHeight = sRst->m_fArrLineHeightAvg[i].y;

		poSeperatePointHighestInfo.x = sRstAlgo->m_fArrLineHeightHighest[index].x;

		poSeperatePointHighestInfo.y = fFilletPointHighestHeight;
	

		float fFilletPointHeight = sRstAlgo->m_fArrLineHeightAvg[index].y;	//fFilletPointHeight = sRst->m_fArrLineHeightAvg[i].y;

		poSeperatePointInfo.x = sRstAlgo->m_fArrLineHeightAvg[index].x;

		poSeperatePointInfo.y = fFilletPointHeight;

		ptFilletHighestPointPosList.emplace_back(poSeperatePointHighestInfo);
		ptFilletPointPosList.emplace_back(poSeperatePointInfo);
		nSeperatePointCnt++;
	}

	//분할 검사 line 정보가 없을 시, 검사 불가능
	if (nSeperatePointCnt == 0)
	{
		return bResult;
	}

	if (nSeperatePointCnt > 10)
		nSeperatePointCnt = 10;

	int nFilletDivCountSpec = pInspAlgoFillet->m_nDivisionCounts;
	if (nFilletDivCountSpec < nSeperatePointCnt)
		nSeperatePointCnt = nFilletDivCountSpec;

	int nCntInspectPos_Enabled = 0;
	std::vector<POINTF> ptFilletPointPosList_Enabled;
	ptFilletPointPosList_Enabled.clear();
	//높이 측정 결과 세팅
	for (int nIdx = 0; nIdx < nSeperatePointCnt; nIdx++)
	{
		if (pInspAlgoFillet->m_bArrUseHighestOption[nIdx] == true)
		{
			float fRstHeight = ptFilletHighestPointPosList[nIdx].y;
			sRstAlgo->m_fArrRstHeight[nIdx] = fRstHeight;
		}
		else
		{
			float fRstHeight = ptFilletPointPosList[nIdx].y;
			sRstAlgo->m_fArrRstHeight[nIdx] = fRstHeight;
		}
		if (pInspAlgoFillet->m_bArrUseAngleRangeHInsp[nIdx] == true)	//높이검사 여부확인
		{
			nCntInspectPos_Enabled++;
			if (pInspAlgoFillet->m_bArrUseHighestOption[nIdx] == true)
				ptFilletPointPosList_Enabled.emplace_back(ptFilletHighestPointPosList[nIdx]);
			else
				ptFilletPointPosList_Enabled.emplace_back(ptFilletPointPosList[nIdx]);

		}
	}

	std::vector<POINTF> ptFilletPosXAndAngleList;
	POINTF poAngleInfo;		//그래프 X좌표에서의 각도
	poAngleInfo.x = 0.0f;
	poAngleInfo.y = 0.0f;

	std::vector<POINTF> ptFilletPosXAndAngleListSub;
	POINTF poAngleInfoSub;		//그래프 X좌표에서의 각도
	poAngleInfoSub.x = 0.0f;
	poAngleInfoSub.y = 0.0f;

	for (int i = 0; i < nCntInspectPos_Enabled; i++)
	{
		if (i == nCntInspectPos_Enabled - 1)
		{
			break;
		}

		double ptrdLineX[2];
		double ptrdLineY[2];
		
		ptrdLineX[0] = (double)(ptFilletPointPosList_Enabled[i + 1].x);
		ptrdLineY[0] = (double)ptFilletPointPosList_Enabled[i + 1].y;

		ptrdLineX[1] = (double)(ptFilletPointPosList_Enabled[i].x);
		ptrdLineY[1] = (double)ptFilletPointPosList_Enabled[i].y;

		//double LeftA = 0;
		//double LeftB = 0;
		//m_pCPInsp_Algo->GetGradient(ptrdLineX, ptrdLineY, 2, LeftA, LeftB, true);
		//fRstAngle = m_pCPInsp_Algo->GetGradient(ptrdLineX, ptrdLineY, 2, LeftA, LeftB, true);

		float fRstAngle = 0.0f;
		double dDeltaX1 = 0.0, dDeltaY1 = 0.0;
		double dRad1 = 0.0;

		dDeltaX1 = ptrdLineX[1] - ptrdLineX[0];
		dDeltaY1 = ptrdLineY[0] - ptrdLineY[1];

		dRad1 = (double)atan2(dDeltaY1, dDeltaX1);     //각도(radian)

		double temp = dRad1;

		if (temp >= -PI)
		{
			while (temp >= PI)
			{
				temp -= PI * 2.0;
			}
		}
		else
		{
			while (temp < -PI)
			{
				temp += PI * 2.0;
			}
		}

		double dTheta = (temp * 180) / PI;
		/////////////////////////////////////////////////////////////

		fRstAngle = (float)(dTheta * (-1));

		poAngleInfoSub.x = ptFilletPointPosList_Enabled[i].x;
		poAngleInfoSub.y = fRstAngle;

		ptFilletPosXAndAngleListSub.emplace_back(poAngleInfoSub);

	}

	//각도 측정 결과 세팅
	for (int nIdx = 0; nIdx < nCntInspectPos_Enabled - 1; nIdx++)
	{
		float fRstAngle = ptFilletPosXAndAngleListSub[nIdx].y;
		sRstAlgo->m_fArrRstAngle[nIdx] = fRstAngle;
	}

	//divideFiletAndMeasurePoint();
	//measureHeightAndAngle();

	if (pInspAlgoFillet->m_bUseInspAngleRangeH == true && ptFilletPointPosList_Enabled.size() > 0)
	{
		//각도 범위 내에서만 검사하는 옵션사용 시

		// User가 설정한 각도범위 min, max 값 받아오기
		float fminAngle = 360;
		float fmaxAngle = 0;

		fminAngle = pInspAlgoFillet->m_fminAngle;
		fmaxAngle = pInspAlgoFillet->m_fmaxAngle;

		float fRstAvgHeight_inAngleRange = 0.0f;

		std::vector<POINTF> ptHeightList_inAngRange;

		float fStartXPos = 0;
		float fEndXPos = FILLET_LINE_COUNT - 1;

		//fillet 분할 point 에서 높이검사 사용여부 체크 목록 중, 가장 첫번째 지점-끝 지점 사이에서만 각도 검출
		fStartXPos = ptFilletPointPosList_Enabled[ptFilletPointPosList_Enabled.size() - 1].x;
		fEndXPos = ptFilletPointPosList_Enabled[0].x;

		//0. fillet 높이 배열을 반복 스캔하면서
		//분할된 포인트 위치에서의 실제 높이 값 측정
		for (int i = 0; i < FILLET_LINE_COUNT; i++)		//검사 ROI Point 배열 최대 개수만큼 순회
		{
			index = i + 2;

			if (index > nFilletLineCnt-2)
				break;

			float fFilletPointHeight = sRstAlgo->m_fArrLineHeightAvg[index].y;	//현재 포인트의 높이 정보

			if ((sRstAlgo->m_fArrLineHeightAvg[index].x < fStartXPos) || (sRstAlgo->m_fArrLineHeightAvg[index].x > fEndXPos))
			{
				continue;
			}

			POINTF poCurPointInfo;
			poCurPointInfo.x = 0.0f;			poCurPointInfo.y = 0.0f;
			poCurPointInfo.x = sRstAlgo->m_fArrLineHeightAvg[index].x;		//x위치 정보
			poCurPointInfo.y = fFilletPointHeight;

			float fFilletPointHeight_pre = sRstAlgo->m_fArrLineHeightAvg[index-2].y;	//이전 2px 포인트의 높이 정보
			POINTF poPrePointInfo;
			poPrePointInfo.x = 0.0f;			poPrePointInfo.y = 0.0f;
			poPrePointInfo.x = sRstAlgo->m_fArrLineHeightAvg[index-2].x;		//이전 2px 포인트 위치 정보
			poPrePointInfo.y = fFilletPointHeight_pre;

			float fFilletPointHeight_next = sRstAlgo->m_fArrLineHeightAvg[index+2].y;	//다음 2px 포인트의 높이 정보
			POINTF poNextPointInfo;
			poNextPointInfo.x = 0.0f;			poNextPointInfo.y = 0.0f;
			poNextPointInfo.x = sRstAlgo->m_fArrLineHeightAvg[index+2].x;		//다음 2px 포인트 위치 정보
			poNextPointInfo.y = fFilletPointHeight_next;

			//1. fillet 높이 배열에서 현재Point-2px과 현재Point+2px의 포인트 간 각도 구하기///////
			double dLineX[2];
			double dLineY[2];

			dLineX[0] = (double)poPrePointInfo.x;
			dLineY[0] = (double)poPrePointInfo.y;

			dLineX[1] = (double)poNextPointInfo.x;
			dLineY[1] = (double)poNextPointInfo.y;

			
			double dDeltaX1 = 0.0, dDeltaY1 = 0.0;
			double dRad1 = 0.0;

			dDeltaX1 = dLineX[1] - dLineX[0];
			dDeltaY1 = dLineY[0] - dLineY[1];

			dRad1 = (double)atan2(dDeltaY1, dDeltaX1);     //각도(radian)

			double temp = dRad1;

			if (temp >= -PI)
			{
				while (temp >= PI)
				{
					temp -= PI * 2.0;
				}
			}
			else
			{
				while (temp < -PI)
				{
					temp += PI * 2.0;
				}
			}

			double dTheta = (temp * 180) / PI;
			/////////////////////////////////////////////////////////////
			//2. 각도 구한값이 User설정각도 범위안에 드는 것들만 신규 높이 배열에 추가
			float fRstAngle_preNext = 0.0f;
			fRstAngle_preNext = (float)(dTheta * (-1));

			
			if (fRstAngle_preNext > fminAngle && fRstAngle_preNext < fmaxAngle)
			{
				POINTF poCurPoint_inAngR;
				poCurPoint_inAngR.x = 0.0f;			poCurPoint_inAngR.y = 0.0f;

				poCurPoint_inAngR.x = sRstAlgo->m_fArrLineHeightAvg[index].x;
				poCurPoint_inAngR.y = sRstAlgo->m_fArrLineHeightAvg[index].y;		//높이
				ptHeightList_inAngRange.emplace_back(poCurPoint_inAngR);

			}
			//////////////////////////////////////////////////////////////////////////////////////
		}

		//3. 추가된 신규배열을 순회하면서 point들의 높이 평균 구하기

		if (ptHeightList_inAngRange.size() != 0)
		{
			float fSumH_InAngR = 0;
			for (int i = 0; i < ptHeightList_inAngRange.size(); i++)
			{
				fSumH_InAngR = fSumH_InAngR + ptHeightList_inAngRange[i].y;
			}

			fRstAvgHeight_inAngleRange = fSumH_InAngR / ptHeightList_inAngRange.size();
		}

		//구해진 높이 평균 값을 검사 결과()에 세팅
		sRstAlgo->m_fRstAngleRangeH = fRstAvgHeight_inAngleRange;

	}
	else
	{
		sRstAlgo->m_fRstAngleRangeH = 0.0f;
	}

	if (sRstAlgo)
	{
		sRstAlgo->m_bOK = TRUE;

		for (int nIdx = 0; nIdx < nSeperatePointCnt; nIdx++)
		{
			//if (pInspAlgoFillet->m_bArrUseAngleRangeHInsp[nIdx] == true)
			{
				float fHeightMin = pInspAlgoFillet->m_fArrHeightOptionValue[nIdx][eMMD_Min];
				float fHeightMax = pInspAlgoFillet->m_fArrHeightOptionValue[nIdx][eMMD_Max];
				float fHeightDefault = pInspAlgoFillet->m_fArrHeightOptionValue[nIdx][eMMD_Default];

				float fRstHeight = sRstAlgo->m_fArrRstHeight[nIdx];

				if (fHeightMin < fRstHeight && fHeightMax > fRstHeight)
					sRstAlgo->m_bArrOKHeight[nIdx] = TRUE;
				else
					sRstAlgo->m_bArrOKHeight[nIdx] = FALSE;
			}
		}

		for (int nIdx = 0; nIdx < nCntInspectPos_Enabled - 1; nIdx++)	//높이 검사 지점 간의 Angle 검사
		{
			//if (pInspAlgoFillet->m_bArrUseAngleRangeHInsp[nIdx] == true)
			{
				float fAngleMin = pInspAlgoFillet->m_fArrAngleOptionValue[nIdx][eMMD_Min];
				float fAngleMax = pInspAlgoFillet->m_fArrAngleOptionValue[nIdx][eMMD_Max];
				float fAngleDefault = pInspAlgoFillet->m_fArrAngleOptionValue[nIdx][eMMD_Default];

				float fRstAngle = sRstAlgo->m_fArrRstAngle[nIdx];

				if (fAngleMin < fRstAngle && fAngleMax > fRstAngle)
					sRstAlgo->m_bArrOKAngle[nIdx] = TRUE;
				else
					sRstAlgo->m_bArrOKAngle[nIdx] = FALSE;
			}
		}

		if (pInspAlgoFillet->m_bUseInspAngleRangeH == true)
		{
			{
				float fAngleRangeHMin = pInspAlgoFillet->m_fminAngle_Spec;
				float fAngleRangeHMax = pInspAlgoFillet->m_fmaxAngle_Spec;
				//float fAngleRangeHDefault = pInspAlgoFillet->m_fArrAngleOptionValue[nIdx][eMMD_Default];

				float fRstAngleRangeH = sRstAlgo->m_fRstAngleRangeH;

				if (fAngleRangeHMin < fRstAngleRangeH && fAngleRangeHMax > fRstAngleRangeH)
					sRstAlgo->m_bOKAngleRangeH = TRUE;
				else
					sRstAlgo->m_bOKAngleRangeH = FALSE;
			}
		}

		bool bFlagHeightAllOK = true;
		bool bFlagAngleAllOK = true;
		for (int nIdx = 0; nIdx < nSeperatePointCnt; nIdx++)
		{
			if (sRstAlgo->m_bArrOKHeight[nIdx] == FALSE  && pInspAlgoFillet->m_bArrUseAngleRangeHInsp[nIdx] == true)
				bFlagHeightAllOK = false;
		}
		for (int nIdx = 0; nIdx < nCntInspectPos_Enabled - 1; nIdx++)
		{
			if (sRstAlgo->m_bArrOKAngle[nIdx] == FALSE /*&& pInspAlgoFillet->m_bArrUseAngleRangeHInsp[nIdx] == true */)
				bFlagAngleAllOK = false;
		}

		if (bFlagHeightAllOK == true && bFlagAngleAllOK == true)
		{
			sRstAlgo->m_bOK = TRUE;
			bResult = TRUE;
		}
		else
			sRstAlgo->m_bOK = FALSE;

		if (pInspAlgoFillet->m_bUseInspAngleRangeH == true)
		{
			if (sRstAlgo->m_bOKAngleRangeH == FALSE)
			{
				sRstAlgo->m_bOK = FALSE;
				bResult = FALSE;
			}
		}

		if (rcInspFilletROI.Width() > 0 && rcInspFilletROI.Height() > 0)
		{
			sRstAlgo->m_rcRect_I.left = rcInspFilletROI.left;
			sRstAlgo->m_rcRect_I.right = rcInspFilletROI.right;
			sRstAlgo->m_rcRect_I.top = rcInspFilletROI.top;
			sRstAlgo->m_rcRect_I.bottom = rcInspFilletROI.bottom;
		}

		//Delete_1DArray(&sRstData);
	}

	return bResult;
}

BOOL CPInsp_AlgoFillet::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoFillet::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoFillet::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}