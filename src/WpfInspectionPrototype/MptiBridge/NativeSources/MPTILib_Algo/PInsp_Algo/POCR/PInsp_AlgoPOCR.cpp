#include "PInsp_AlgoPOCR.h"
#include <numeric> 

#define CharSpaceDebug		0	

CPInsp_AlgoPOCR::CPInsp_AlgoPOCR(void)
{
}


CPInsp_AlgoPOCR::~CPInsp_AlgoPOCR(void)
{
}

void CPInsp_AlgoPOCR::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoPOCR::GetInspAlgoData()
{
	return eSPCAlgoPOCR;
}

int CPInsp_AlgoPOCR::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeOcrWrong] = e_NG;
	nCurrentNgType = TypeOcrWrong;
	return nCurrentNgType;
}

bool CPInsp_AlgoPOCR::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoPOCR::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	CPInsp_POCR *pPOCR = g_pInspMng->GetPocrTeach();
	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();
	if (pPOCR == nullptr || pInspBoardInfo == nullptr)
		return bResult;

	pPOCR->SetInspParam(sInspAlgo, pInspBoardInfo->angle);

	bResult = pPOCR->InspProc_POCR(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, stAlgoParam.m_nInspectionMode);

	RstAlgoPOCR * rstPOCR = (RstAlgoPOCR *)sRstAlgo;
	pPOCR->GetInspRst(rstPOCR);
	SetRstROI(sInspAlgo, sWndAlgoImg, stAlgoParam, rstPOCR);
	return bResult;
}

BOOL CPInsp_AlgoPOCR::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoPOCR::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoPOCR::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}
void CPInsp_AlgoPOCR::SetRstROI(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspAlgoParam stAlgoParam, RstAlgoPOCR * rstPOCR)
{
	if (rstPOCR == NULL)
		return;
	memset(rstPOCR->m_rcROI, 0, sizeof(RECT)*MAX_STRLEN);
	memset(rstPOCR->m_rcROI_Ro, 0, sizeof(RECT)*MAX_STRLEN);
	if (rstPOCR->m_nCharCount <= 0 || rstPOCR->m_nCharCount >= MAX_STRLEN)
		return;

#if _DEBUG
	cv::Mat img(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D);
	cv::Mat img90;
	cv::Mat img180;
	cv::Mat img270;
	cv::rotate(img, img90, cv::ROTATE_90_CLOCKWISE);
	cv::rotate(img, img180, cv::ROTATE_180);
	cv::rotate(img, img270, cv::ROTATE_90_COUNTERCLOCKWISE);
#endif

	AlgoPOCR * InspAlgoPOCR = (AlgoPOCR *)sInspAlgo.m_ptrInspAlgoParam;
	if (InspAlgoPOCR == NULL)
		return;

	int wndAngle = ((int)InspAlgoPOCR->m_dWndAngle);
	if (wndAngle % 90 != 0)
		wndAngle = 0;
	double dAngle = InspAlgoPOCR->m_dFontAngle - wndAngle;
	double dPartWOrg = sWndAlgoImg.m_nWidth;
	double dPartHOrg = sWndAlgoImg.m_nHeight;
	for (int a = 0; a < 2; a++)
	{
		double dPartA = dAngle;
		if (a == 1)
			dPartA += 180;

		if (dPartA > 360)
			dPartA = dPartA - 360;
		else if (dPartA < 0)
			dPartA = dPartA + 360;

		bool bRO = (dPartA == 90 || dPartA == 270);
		double dPartW = bRO ? dPartHOrg : dPartWOrg;
		double dPartH = bRO ? dPartWOrg : dPartHOrg;

		double* charX = a == 0 ? rstPOCR->charX : rstPOCR->charX_Ro;
		double* charY = a == 0 ? rstPOCR->charY : rstPOCR->charY_Ro;
		double* charWidth = a == 0 ? rstPOCR->charWidth : rstPOCR->charWidth_Ro;
		double* charHeight = a == 0 ? rstPOCR->charHeight : rstPOCR->charHeight_Ro;
		double* charAngle = a == 0 ? rstPOCR->charAngle : rstPOCR->charAngle_Ro;
		if (charX == NULL || charY == NULL || charWidth == NULL || charHeight == NULL || charAngle == NULL)
			continue;

		for (int b = 0; b < rstPOCR->m_nCharCount; b++)
		{
			double dA = charAngle[b];
			double dCX = charX[b];
			double dCY = charY[b];
			if (dCX < 0 || dCY < 0)
				continue;

			double dPartCX = bRO ? dCY - (dPartH / 2) : dCX - (dPartW / 2);
			double dPartCY = bRO ? dCX - (dPartW / 2) : dCY - (dPartH / 2);
			if (dPartA == 90)
			{
				dPartCX *= -1;
			}
			else if (dPartA == 180)
			{
				dPartCX *= -1;
				dPartCY *= -1;
			}
			else if (dPartA == 270)
			{
				dPartCY *= -1;
			}
			double dX = dPartCX + (dPartWOrg / 2);
			double dY = dPartCY + (dPartHOrg / 2);

			double dW = bRO ? charHeight[b] : charWidth[b];
			double dH = bRO ? charWidth[b] : charHeight[b];
			if (dW <= 0 || dH <= 0)
				continue;

			double dL = (dX - (dW / 2.0));
			double dT = (dY - (dH / 2.0));
			if (a == 0)
			{
				rstPOCR->m_rcROI[b].left = dL + stAlgoParam.m_dx;
				rstPOCR->m_rcROI[b].top = dT + stAlgoParam.m_dy;
				rstPOCR->m_rcROI[b].right = rstPOCR->m_rcROI[b].left + dW;
				rstPOCR->m_rcROI[b].bottom = rstPOCR->m_rcROI[b].top + dH;
			}
			else
			{
				rstPOCR->m_rcROI_Ro[b].left = dL + stAlgoParam.m_dx;
				rstPOCR->m_rcROI_Ro[b].top = dT + stAlgoParam.m_dy;
				rstPOCR->m_rcROI_Ro[b].right = rstPOCR->m_rcROI_Ro[b].left + dW;
				rstPOCR->m_rcROI_Ro[b].bottom = rstPOCR->m_rcROI_Ro[b].top + dH;
			}
		}
	}
}


BOOL CPInsp_AlgoPOCR::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	int check = InspWrapper->m_PInspAlgo->PInspAlgo::CheckHalconLicense();

	int Line = __LINE__;

	if (check != 1)
	{
		if (g_pMPTI)
		{
			CString sLog = _T(""), sType = _T("");
			if (check == -2)
				sType.Format(_T("HALCON OCR Model Not Loaded."));
			else
				sType.Format(_T("HALCON License Error. (error code: %d)"), check);

			sLog.Format(_T("CPInsp_AlgoPOCR::InspAlgorithm_Dll(), Line : %d Pass!!! POCR - %s"), Line, sType);
			g_pMPTI->AddLog_Dev(sLog);

			sLog.Format(_T("ERROR : POCR - %s"), sType);
			AfxMessageBox(sLog);

			throw Line;
		}
	}

	AlgoPOCR *algo = (AlgoPOCR*)InspAlgo.m_ptrInspAlgoParam;
	PIAL::_RstAlgoPOCR pRstPOCR;
	PIAL::_AlgoPOCR pAlgoPOCR;
	InspWrapper->ConvertAlgo(algo, pAlgoPOCR);

	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();
	if (pInspBoardInfo == nullptr)
		return bResult;

	pAlgoPOCR.m_dFontAngle = pInspBoardInfo->angle;
	if ((int)ceil(pAlgoPOCR.m_dFontAngle) % 90 != 0 || (int)floor(pAlgoPOCR.m_dFontAngle) % 90 != 0)pAlgoPOCR.m_dFontAngle = 0;
	cv::Mat POCRImg = pImg_buf->m_p2D->Mat();

	bool useSpace = pAlgoPOCR.m_bUseCharSpace;
	if (count(pAlgoPOCR.m_sTargetFont, pAlgoPOCR.m_sTargetFont + pAlgoPOCR.m_nCharMaxCount, ' ') > 0)
		pAlgoPOCR.m_bUseCharSpace = TRUE;

	cv::bitwise_not(Mask_buf->Mat(), Mask_buf->Mat());
	//if (pAlgoPOCR.m_nClrForeGround == 1)
	//	pImg_buf->m_p2D->Mat() = pImg_buf->m_p2D->Mat() + Mask_buf->Mat();
	//else
	//	pImg_buf->m_p2D->Mat() = pImg_buf->m_p2D->Mat() - Mask_buf->Mat();
	bResult = InspWrapper->m_PInspAlgo->PInspAlgo::HalconInspPOCR(pAlgoPOCR, *pImg_buf, &pRstPOCR, vecAlignResult);
	memcpy(pRstPOCR.m_cArrTeachStr, pAlgoPOCR.m_sTargetFont, sizeof(wchar_t) * MAX_STRLEN);
	int wndNameLen(0);
	if (!bResult && algo->m_bUseWndNameInWindow && pAlgoPOCR.m_nCharMaxCount > 0)
	{
		PIAL::_RstAlgoPOCR pRstPOCR1;

		CString cstr(algo->m_sWndName);

		wndNameLen = cstr.GetLength();
		for (int a = pAlgoPOCR.m_nCharMaxCount + wndNameLen - 1; a >= wndNameLen; a--)
			pAlgoPOCR.m_sTargetFont[a] = pAlgoPOCR.m_sTargetFont[a - wndNameLen];

		memcpy(pAlgoPOCR.m_sTargetFont, algo->m_sWndName, sizeof(wchar_t) * wndNameLen);

		pAlgoPOCR.m_nCharMaxCount += wndNameLen;

		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::HalconInspPOCR(pAlgoPOCR, *pImg_buf, &pRstPOCR1, vecAlignResult);

		if (bResult)
			memcpy(&pRstPOCR, &pRstPOCR1, sizeof(PIAL::_RstAlgoPOCR));

		if (bResult)
		{
			if (wndNameLen == 0)
			{
				CString cstr(algo->m_sWndName);
				wndNameLen = cstr.GetLength();
			}

			if (pRstPOCR.m_nCharCount < wndNameLen)
				wndNameLen = pRstPOCR.m_nCharCount;

			pAlgoPOCR.m_nCharMaxCount -= wndNameLen;
			pRstPOCR.m_nCharCount -= wndNameLen;

			for (int a = 0; a < pRstPOCR.m_nCharCount; a++)
			{
				pAlgoPOCR.m_sTargetFont[a] = pAlgoPOCR.m_sTargetFont[a + wndNameLen];
				pRstPOCR.m_cArrStr[a] = pRstPOCR.m_cArrStr[a + wndNameLen];
				pRstPOCR.m_nCharScoreResult[a] = pRstPOCR.m_nCharScoreResult[a + wndNameLen];
				pRstPOCR.charX[a] = pRstPOCR.charX[a + wndNameLen];
				pRstPOCR.charY[a] = pRstPOCR.charY[a + wndNameLen];
				pRstPOCR.charWidth[a] = pRstPOCR.charWidth[a + wndNameLen];
				pRstPOCR.charHeight[a] = pRstPOCR.charHeight[a + wndNameLen];
				pRstPOCR.m_dArrCharScore[a] = pRstPOCR.m_dArrCharScore[a + wndNameLen];
			}

			memset(pAlgoPOCR.m_sTargetFont + pAlgoPOCR.m_nCharMaxCount, 0, sizeof(wchar_t) * (MAX_STRLEN - pAlgoPOCR.m_nCharMaxCount));
			memset(pRstPOCR.m_cArrStr + pRstPOCR.m_nCharCount, 0, sizeof(wchar_t) * (MAX_STRLEN - pRstPOCR.m_nCharCount));
			memset(pRstPOCR.m_nCharScoreResult + pRstPOCR.m_nCharCount, 0, sizeof(wchar_t) * (MAX_STRLEN - pRstPOCR.m_nCharCount));
			memset(pRstPOCR.charX + pRstPOCR.m_nCharCount, 0, sizeof(wchar_t) * (MAX_STRLEN - pRstPOCR.m_nCharCount));
			memset(pRstPOCR.charY + pRstPOCR.m_nCharCount, 0, sizeof(wchar_t) * (MAX_STRLEN - pRstPOCR.m_nCharCount));
			memset(pRstPOCR.charWidth + pRstPOCR.m_nCharCount, 0, sizeof(wchar_t) * (MAX_STRLEN - pRstPOCR.m_nCharCount));
			memset(pRstPOCR.charHeight + pRstPOCR.m_nCharCount, 0, sizeof(wchar_t) * (MAX_STRLEN - pRstPOCR.m_nCharCount));
			memset(pRstPOCR.m_dArrCharScore + pRstPOCR.m_nCharCount, 0, sizeof(wchar_t) * (MAX_STRLEN - pRstPOCR.m_nCharCount));

			if (pRstPOCR.m_nCharCount == 0)
			{
				bResult = FALSE;
				pRstPOCR.m_bOKString = FALSE;
				pRstPOCR.m_dStringScore = 0;
			}
		}
	}

	int nResCnt = pRstPOCR.m_nCharCount;

	if (nResCnt == 0)
	{
		//사용 되는 배열 초기화
		memset(pRstPOCR.m_cArrStr, 0, sizeof(wchar_t) * _MAX_STRLEN);
		memset(pRstPOCR.m_nCharScoreResult, 0, sizeof(int) * _MAX_STRLEN);
		memset(pRstPOCR.charX, 0, sizeof(double) * _MAX_STRLEN);
		memset(pRstPOCR.charY, 0, sizeof(double) * _MAX_STRLEN);
		memset(pRstPOCR.charWidth, 0, sizeof(double) * _MAX_STRLEN);
		memset(pRstPOCR.charHeight, 0, sizeof(double) * _MAX_STRLEN);
		memset(pRstPOCR.m_dArrCharScore, 0, sizeof(double) * _MAX_STRLEN);

		pRstPOCR.m_nCharCount = 1;
		pRstPOCR.m_cArrStr[0] = '?';
	}
	else
	{
		if (pAlgoPOCR.m_bUseCharSpace)
		{
			std::vector<double> vCharX = std::vector<double>(std::begin(pRstPOCR.charX), std::end(pRstPOCR.charX));
			std::vector<double> vCharY = std::vector<double>(std::begin(pRstPOCR.charY), std::end(pRstPOCR.charY));
			std::vector<double> vCharWidth = std::vector<double>(std::begin(pRstPOCR.charWidth), std::end(pRstPOCR.charWidth));
			std::vector<double> vCharHeight = std::vector<double>(std::begin(pRstPOCR.charHeight), std::end(pRstPOCR.charHeight));
			std::vector<double> vCharScore = std::vector<double>(std::begin(pRstPOCR.m_dArrCharScore), std::end(pRstPOCR.m_dArrCharScore));
			std::vector<int> vCharScoreResult = std::vector<int>(std::begin(pRstPOCR.m_nCharScoreResult), std::end(pRstPOCR.m_nCharScoreResult));
			std::vector<wchar_t> vChar = std::vector<wchar_t>(std::begin(pRstPOCR.m_cArrStr), std::end(pRstPOCR.m_cArrStr));

			if (!useSpace)
			{
				bool onlyRecognition = pAlgoPOCR.m_nCharMaxCount == 0;
				int nResCnt = pRstPOCR.m_nCharCount;

				for (int n = 1; n < nResCnt; n++)
				{
					if (pAlgoPOCR.m_sTargetFont[n] == ' ')
					{
						vector<wchar_t>::iterator it_wchar = vChar.begin() + n;
						vChar.insert(it_wchar, ' ');

						vector<int>::iterator it_int = vCharScoreResult.begin() + n;
						vCharScoreResult.insert(it_int, 0);

						double dCenX = (vCharX[n] + vCharX[n - 1]) / 2;
						double dCenY = vCharY[n - 1];

						vector<double>::iterator it_double = vCharX.begin() + n;
						vCharX.insert(it_double, dCenX);

						it_double = vCharY.begin() + n;
						vCharY.insert(it_double, dCenY);

						it_double = vCharWidth.begin() + n;
						vCharWidth.insert(it_double, (vCharX[n] - (vCharWidth[n] / 2)) - (vCharX[n - 1] + (vCharWidth[n - 1] / 2)));

						it_double = vCharHeight.begin() + n;
						vCharHeight.insert(it_double, vCharHeight[n - 1]);

						it_double = vCharScore.begin() + n;
						vCharScore.insert(it_double, 100.0);

						nResCnt++;
					}
				}

				if (0)
				{
					//사용 되는 배열 초기화
					memset(pRstPOCR.m_cArrStr, 0, sizeof(wchar_t) * _MAX_STRLEN);
					memset(pRstPOCR.m_nCharScoreResult, 0, sizeof(int) * _MAX_STRLEN);
					memset(pRstPOCR.charX, 0, sizeof(double) * _MAX_STRLEN);
					memset(pRstPOCR.charY, 0, sizeof(double) * _MAX_STRLEN);
					memset(pRstPOCR.charWidth, 0, sizeof(double) * _MAX_STRLEN);
					memset(pRstPOCR.charHeight, 0, sizeof(double) * _MAX_STRLEN);
					memset(pRstPOCR.m_dArrCharScore, 0, sizeof(double) * _MAX_STRLEN);
				}

				if (!onlyRecognition && nResCnt > pAlgoPOCR.m_nCharMaxCount)
					nResCnt = pAlgoPOCR.m_nCharMaxCount;

				pRstPOCR.m_nCharCount = nResCnt;
				std::copy(vChar.begin(), vChar.begin() + nResCnt, pRstPOCR.m_cArrStr);
				std::copy(vCharScoreResult.begin(), vCharScoreResult.begin() + nResCnt, pRstPOCR.m_nCharScoreResult);
				std::copy(vCharX.begin(), vCharX.begin() + nResCnt, pRstPOCR.charX);
				std::copy(vCharY.begin(), vCharY.begin() + nResCnt, pRstPOCR.charY);
				std::copy(vCharWidth.begin(), vCharWidth.begin() + nResCnt, pRstPOCR.charWidth);
				std::copy(vCharHeight.begin(), vCharHeight.begin() + nResCnt, pRstPOCR.charHeight);
				std::copy(vCharScore.begin(), vCharScore.begin() + nResCnt, pRstPOCR.m_dArrCharScore);
			}
			else
			{
				bool onlyRecognition = pAlgoPOCR.m_nCharMaxCount == 0;

				std::vector<std::pair<double, double>> vLeftRight;
				std::vector<std::pair<double, double>> vTopBottom;

				int nResCnt = pRstPOCR.m_nCharCount;
				double meanWidth(0), meanHeight(0);
				for (int n = 0; n < nResCnt; n++)
				{
					double dLeft = vCharX[n] - (vCharWidth[n] / 2);
					double dRight = vCharX[n] + (vCharWidth[n] / 2);
					vLeftRight.emplace_back(std::make_pair(dLeft, dRight));

					double dTop = vCharY[n] - (vCharHeight[n] / 2);
					double dBottom = vCharY[n] + (vCharHeight[n] / 2);
					vTopBottom.emplace_back(std::make_pair(dTop, dBottom));

					meanWidth += vCharWidth[n];
					meanHeight += vCharHeight[n];
				}
				meanWidth /= nResCnt;
				meanHeight /= nResCnt;

				double dMaxInterval = 0;
				std::vector<double> vCharXInterval = std::vector<double>();
				std::vector<double> vCharYInterval = std::vector<double>();
				std::vector<int> vEndlineIndex;

				if (onlyRecognition)
				{
					for (int n = 1; n < nResCnt; n++)
					{
						double dInterval_X = vLeftRight[n].first - vLeftRight[n - 1].second;
						double dInterval_Y = vTopBottom[n - 1].second - vTopBottom[n].first;

						dInterval_X *= 2 * vCharWidth[n - 1] * vCharWidth[n] / ((vCharWidth[n - 1] + vCharWidth[n]) * meanWidth);

						if (dMaxInterval < dInterval_X)
							dMaxInterval = dInterval_X;

						if (dInterval_X < 0)
							continue;

						if (dInterval_Y < 0)
						{
							vEndlineIndex.emplace_back(vCharXInterval.size());
							continue;
						}

						vCharXInterval.emplace_back(dInterval_X);
						vCharYInterval.emplace_back(dInterval_Y);
					}
					double dMean = .0, dStdDev = .0;
					GetMeanStdDev(vCharXInterval, &dMean, &dStdDev);

#if CharSpaceDebug
					{
						//pRstPOCR.m_cArrStr
						//dMean, dStdDev
						//vCharXInterval

						FILE *fw(nullptr);
						TCHAR fname[MAX_PATH];
						CString path = _T("D:\\testimage");
						CString pullpath;
						pullpath.Format(_T("%s\\CharSpace.txt"), path);
						wsprintf(fname, pullpath);

						fw = _tfopen(fname, _T("w"));

						CString sTemp;
						sTemp.Format(_T("Rst, %s, \nmean, %f , StdDev, %f\n"), pRstPOCR.m_cArrStr, dMean, dStdDev);
						_ftprintf(fw, sTemp);

						sTemp.Format(_T("이름, 값\n"));
						_ftprintf(fw, sTemp);
						for (int i = 0; i < vCharXInterval.size(); i++)
						{
							sTemp.Format(_T("[%d], %f\n"), i, vCharXInterval[i]);
							_ftprintf(fw, sTemp);
						}

						fclose(fw);
					}
#endif

					if (dStdDev > 7.0)
					{
						double dWeight = dStdDev < 13 ? 1.95 : 0.7;
						double dLimit = dMean + dStdDev * dWeight;

						int nEndlSize = vEndlineIndex.size();
						vector<double>::iterator it_XInterval = vCharXInterval.begin();
						vector<double>::iterator it_YInterval = vCharYInterval.begin();
						for (int i = nEndlSize - 1; i >= 0; i--)
						{
							vCharXInterval.insert(it_XInterval + vEndlineIndex[i], dLimit * 1.2);
							vCharYInterval.insert(it_YInterval + vEndlineIndex[i], meanHeight);
						}

						int nX_Cnt = vCharXInterval.size();
						int nFos = 0;
						int nNum = 0;
						for (int n = 0; n < nX_Cnt; n++)
						{
							if (vCharXInterval[n] > dLimit || pAlgoPOCR.m_sTargetFont[n + nNum + 1] == ' ')
							{
								nFos = n + 1 + nNum;
								nNum++;

								vector<wchar_t>::iterator it_wchar = vChar.begin() + nFos;
								vChar.insert(it_wchar, ' ');

								vector<int>::iterator it_int = vCharScoreResult.begin() + nFos;
								vCharScoreResult.insert(it_int, 0);

								double dCenX = (vCharX[nFos] + vCharX[nFos - 1]) / 2;
								double dCenY = vCharY[nFos - 1];

								vector<double>::iterator it_double = vCharX.begin() + nFos;
								vCharX.insert(it_double, dCenX);

								it_double = vCharY.begin() + nFos;
								vCharY.insert(it_double, dCenY);

								it_double = vCharWidth.begin() + nFos;
								vCharWidth.insert(it_double, vCharXInterval[n]);

								it_double = vCharHeight.begin() + nFos;
								vCharHeight.insert(it_double, vCharHeight[nFos - 1]);

								it_double = vCharScore.begin() + nFos;
								if (onlyRecognition && vCharXInterval[n] > dLimit || !onlyRecognition && pAlgoPOCR.m_sTargetFont[n + nNum] == ' ' && vCharXInterval[n] > dMean * 1.25)
									vCharScore.insert(it_double, 100.0);
								else
								{
									vCharScore.insert(it_double, 0.0);
									bResult = pRstPOCR.m_bOKScore = pRstPOCR.m_bOKString = FALSE;
									vCharScoreResult[nFos] = 1;
									pRstPOCR.m_dStringScore = 0;
								}
							}
						}
						nResCnt += nNum;
					}
				}
				else
				{
					for (int n = 1; n < nResCnt; n++)
					{
						double dInterval_X = vLeftRight[n].first - vLeftRight[n - 1].second;
						double dInterval_Y = vTopBottom[n - 1].second - vTopBottom[n].first;

						if (dMaxInterval < dInterval_X)
							dMaxInterval = dInterval_X;

						if (dInterval_Y < 0)
						{
							vEndlineIndex.emplace_back(vCharXInterval.size());
							continue;
						}

						vCharXInterval.emplace_back(dInterval_X);
						vCharYInterval.emplace_back(dInterval_Y);
					}
					double dMean = .0, dStdDev = .0;
					GetMeanStdDev(vCharXInterval, &dMean, &dStdDev);

#if CharSpaceDebug
					{
						//pRstPOCR.m_cArrStr
						//dMean, dStdDev
						//vCharXInterval

						FILE *fw(nullptr);
						TCHAR fname[MAX_PATH];
						CString path = _T("D:\\testimage");
						CString pullpath;
						pullpath.Format(_T("%s\\CharSpace.txt"), path);
						wsprintf(fname, pullpath);

						fw = _tfopen(fname, _T("w"));

						CString sTemp;
						sTemp.Format(_T("Rst, %s, \nmean, %f , StdDev, %f\n"), pRstPOCR.m_cArrStr, dMean, dStdDev);
						_ftprintf(fw, sTemp);

						sTemp.Format(_T("이름, 값\n"));
						_ftprintf(fw, sTemp);
						for (int i = 0; i < vCharXInterval.size(); i++)
						{
							sTemp.Format(_T("[%d], %f\n"), i, vCharXInterval[i]);
							_ftprintf(fw, sTemp);
						}

						fclose(fw);
					}
#endif

					double dLimit = dMean * 1.25;

					int nEndlSize = vEndlineIndex.size();
					vector<double>::iterator it_XInterval = vCharXInterval.begin();
					vector<double>::iterator it_YInterval = vCharYInterval.begin();
					for (int i = nEndlSize - 1; i >= 0; i--)
					{
						vCharXInterval.insert(it_XInterval + vEndlineIndex[i], dLimit * 1.2);
						vCharYInterval.insert(it_YInterval + vEndlineIndex[i], meanHeight);
					}

					int nX_Cnt = vCharXInterval.size();
					int nFos = 0;
					int nNum = 0;
					for (int n = 0; n < nX_Cnt; n++)
					{
						if (vCharXInterval[n] > dLimit && pAlgoPOCR.m_sTargetFont[n + nNum + 1] == ' ')
						{
							nFos = n + 1 + nNum;
							nNum++;

							vector<wchar_t>::iterator it_wchar = vChar.begin() + nFos;
							vChar.insert(it_wchar, ' ');

							vector<int>::iterator it_int = vCharScoreResult.begin() + nFos;
							vCharScoreResult.insert(it_int, 0);

							double dCenX = (vCharX[nFos] + vCharX[nFos - 1]) / 2;
							double dCenY = vCharY[nFos - 1];

							vector<double>::iterator it_double = vCharX.begin() + nFos;
							vCharX.insert(it_double, dCenX);

							it_double = vCharY.begin() + nFos;
							vCharY.insert(it_double, dCenY);

							it_double = vCharWidth.begin() + nFos;
							vCharWidth.insert(it_double, vCharXInterval[n]);

							it_double = vCharHeight.begin() + nFos;
							vCharHeight.insert(it_double, vCharHeight[nFos - 1]);

							it_double = vCharScore.begin() + nFos;
							vCharScore.insert(it_double, 100.0);
						}
					}
					nResCnt += nNum;
				}

				if (0)
				{
					//사용 되는 배열 초기화
					memset(pRstPOCR.m_cArrStr, 0, sizeof(wchar_t) * _MAX_STRLEN);
					memset(pRstPOCR.m_nCharScoreResult, 0, sizeof(int) * _MAX_STRLEN);
					memset(pRstPOCR.charX, 0, sizeof(double) * _MAX_STRLEN);
					memset(pRstPOCR.charY, 0, sizeof(double) * _MAX_STRLEN);
					memset(pRstPOCR.charWidth, 0, sizeof(double) * _MAX_STRLEN);
					memset(pRstPOCR.charHeight, 0, sizeof(double) * _MAX_STRLEN);
					memset(pRstPOCR.m_dArrCharScore, 0, sizeof(double) * _MAX_STRLEN);
				}

				if (!onlyRecognition && nResCnt > pAlgoPOCR.m_nCharMaxCount)
					nResCnt = pAlgoPOCR.m_nCharMaxCount;

				pRstPOCR.m_nCharCount = nResCnt;
				std::copy(vChar.begin(), vChar.begin() + nResCnt, pRstPOCR.m_cArrStr);
				std::copy(vCharScoreResult.begin(), vCharScoreResult.begin() + nResCnt, pRstPOCR.m_nCharScoreResult);
				std::copy(vCharX.begin(), vCharX.begin() + nResCnt, pRstPOCR.charX);
				std::copy(vCharY.begin(), vCharY.begin() + nResCnt, pRstPOCR.charY);
				std::copy(vCharWidth.begin(), vCharWidth.begin() + nResCnt, pRstPOCR.charWidth);
				std::copy(vCharHeight.begin(), vCharHeight.begin() + nResCnt, pRstPOCR.charHeight);
				std::copy(vCharScore.begin(), vCharScore.begin() + nResCnt, pRstPOCR.m_dArrCharScore);
			}
		}

		if (algo->m_bUseFontSpec && pRstPOCR.m_nCharCount != algo->m_nFontSpec)
		{
			bResult = 0;
		}
	}

	if (pAlgoPOCR.m_bUseOCV)
		bResult = pRstPOCR.m_bOKScore && bResult;

	InspWrapper->ConvertRstAlgo(pRstPOCR, (RstAlgoPOCR*)sRstAlgo);

	return bResult;
}

void CPInsp_AlgoPOCR::GetMeanStdDev(std::vector<double> vInput, double* dMean, double* dStdDev)
{
	*dMean = std::accumulate(vInput.begin(), vInput.end(), 0.0) / vInput.size();
	double dsum = std::inner_product(vInput.begin(), vInput.end(), vInput.begin(), 0.0, [](double const & x, double const & y) {return x + y; }, [dMean](double const & x, double const & y) {return (x - *dMean)*(y - *dMean); });
	*dStdDev = std::sqrt(dsum / (vInput.size() - 1));
}
