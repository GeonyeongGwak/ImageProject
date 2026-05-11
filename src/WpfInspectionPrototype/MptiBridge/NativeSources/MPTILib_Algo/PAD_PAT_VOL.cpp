#include "stdafx.h"
#include "PAD_PAT_VOL.h"
#include "MPTI.h"
#include "InspManager.h"


PAD_PAT::PAD_PAT(void)
{
}
PAD_PAT::~PAD_PAT(void)
{
}

void PAD_PAT::InitAlgo(double dResX, double dResY)
{
	m_resolX = dResX;
	m_resolY = dResY;
}

#pragma region PARALLEL
int PAD_PAT::PAD_PARALLEL(INSP_PAD_PAT sInspData, INSP_PAD_PAT_RST* pPAT_RST, UCHAR* pucPOLY)
{
	int nORG_W = sInspData.m_nORG_W;
	int nORG_H = sInspData.m_nORG_H;
	if (sInspData.m_ucBIN == NULL || sInspData.m_ucGRAY == NULL)
		return -1;
	else if (sInspData.m_nORG_W <= PAT_INSP_MIN_SIZE || sInspData.m_nORG_H <= PAT_INSP_MIN_SIZE)
		return -2;
	else if (pPAT_RST == NULL)
		return -3;

	pPAT_RST->Init();
	cv::Mat imgGRAY_ORG(nORG_H, nORG_W, CV_8UC1, sInspData.m_ucGRAY);
	cv::Mat imgBIN_ORG(nORG_H, nORG_W, CV_8UC1, sInspData.m_ucBIN);
	bool bCalc3D = sInspData.UseData(m_eINSP_PAD_PAT_Calc3D);
	if (sInspData.m_nL < 0 || sInspData.m_nL >= nORG_W ||
		sInspData.m_nT < 0 || sInspData.m_nT >= nORG_H)
		return -4;
	else if (sInspData.m_nW <= PAT_INSP_MIN_SIZE || sInspData.m_nH <= PAT_INSP_MIN_SIZE)
		return -5;
	else if (sInspData.m_nL + sInspData.m_nW > nORG_W || sInspData.m_nT + sInspData.m_nH > nORG_H)
		return -6;

	cv::Rect roiCLIP(sInspData.m_nL, sInspData.m_nT, sInspData.m_nW, sInspData.m_nH);
	int nW = roiCLIP.width;
	int nH = roiCLIP.height;

	cv::Mat ImgPoly(nH, nW, CV_8UC1, cv::Scalar(0));
	std::vector<std::vector<cv::Point>> vPoly(1, std::vector<cv::Point>());
	for (int i = 0; i < sInspData.m_sPadPoly.m_nPolyCnt; i++)
	{
		int nPolyX = (int)std::round(sInspData.m_sPadPoly.m_ptArrPoly[i].x);
		int nPolyY = (int)std::round(sInspData.m_sPadPoly.m_ptArrPoly[i].y);
		cv::Point ptPoly(nPolyX, nPolyY);
		vPoly[0].push_back(ptPoly);
	}
	if (sInspData.m_sPadPoly.m_nPolyCnt > 0 && vPoly[0].size() > 3)
		cv::fillPoly(ImgPoly, vPoly, cv::Scalar(255, 255, 255));
	else
		return -7;

	if (200 > nW && 200 > nH)
	{
		cv::Rect roiP = cv::boundingRect(ImgPoly);
		if (roiP.x > N_GAP_ORG && roiP.br().x < nW - N_GAP_ORG &&
			roiP.y > N_GAP_ORG && roiP.br().y < nH - N_GAP_ORG)
		{
			if (sInspData.m_sArrOverlapPadPoly != NULL)
			{
				int nPolyW = roiP.width;
				int nPolyH = roiP.height;
				for (int i = 0; i < sInspData.m_nOverlapPadPolyCnt; i++)
				{
					int nROIL = nW * nH;
					int nROIR = 0;
					int nROIT = nW * nH;
					int nROIB = 0;
					for (int n = 0; n < sInspData.m_sArrOverlapPadPoly[i].m_nPolyCnt; n++)
					{
						int nPolyX = (int)std::round(sInspData.m_sArrOverlapPadPoly[i].m_ptArrPoly[n].x);
						int nPolyY = (int)std::round(sInspData.m_sArrOverlapPadPoly[i].m_ptArrPoly[n].y);
						if (nROIL > nPolyX) nROIL = nPolyX;
						if (nROIR < nPolyX) nROIR = nPolyX;
						if (nROIT > nPolyY) nROIT = nPolyY;
						if (nROIB < nPolyY) nROIB = nPolyY;
					}
					if (nROIL >= nROIR || nROIT >= nROIB)
						continue;
					if (nROIR <= 0 || nROIB <= 0)
						continue;
					if (nROIL >= nW || nROIT >= nH)
						continue;

					if (nROIL < 0) nROIL = 0;
					if (nROIR < 0) nROIR = 0;
					if (nROIT < 0) nROIT = 0;
					if (nROIB < 0) nROIB = 0;
					if (nROIL >= nW) nROIL = nW - 1;
					if (nROIR >= nW) nROIR = nW;
					if (nROIT >= nH) nROIT = nH - 1;
					if (nROIB >= nH) nROIB = nH;
					if (nROIL >= nROIR || nROIT >= nROIB)
						continue;
					if (nROIL < 0 || nROIR <= 0 || nROIT < 0 || nROIB <= 0)
						continue;
					if (nROIL >= nW || nROIR > nW || nROIT >= nH || nROIB > nH)
						continue;

					int nROIW = nROIR - nROIL;
					int nROIH = nROIB - nROIT;
					nPolyW -= nROIW;
					nPolyH -= nROIH;
				}
				if (roiP.width > nPolyW && roiP.height > nPolyH)
				{
					if (N_INSP_OUT >= nPolyW && N_INSP_OUT >= nPolyH)
					{
						return -8;
					}
				}
			}
		}
	}

	cv::Mat imgGRAY = imgGRAY_ORG(roiCLIP).clone();
	cv::Mat ImgBin = imgBIN_ORG(roiCLIP).clone();
	cv::Mat ImgTemp(nH, nW, CV_8UC1, cv::Scalar(0));
	WndAlgoImg sWndAlgoImg;
	sWndAlgoImg.m_ucArr2D = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	sWndAlgoImg.m_fArr3D = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
	sWndAlgoImg.m_nWidth = nW;
	sWndAlgoImg.m_nHeight = nH;
	sWndAlgoImg.dAngle = sInspData.m_dAngle;
	memcpy(sWndAlgoImg.m_ucArr2D, imgGRAY.data, sizeof(UCHAR) * nW * nH);
	memset(sWndAlgoImg.m_fArr3D, 0, sizeof(float) * nW * nH);

	AlgoVolume* pAlgo = &sInspData.m_sAlgo;
	if (INSP_SHIFT_AREA(pAlgo, sWndAlgoImg, ImgPoly, ImgBin.data, nW, nH) == false)
	{
		sWndAlgoImg.Destroy();
		return -9;
	}

	int nMinBlobArea = 4;
	int nTypeSelectBlob = pAlgo->m_nTypeSelectBlob;
	int nFilter = pAlgo->m_bFilterIsUse ? pAlgo->m_nFilterStepNarrow : 0;
	BOOL bFillHole = pAlgo->m_bFillHole;
	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(ImgBin, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	if (allcontour.size() == 0)
	{
		sWndAlgoImg.Destroy();
		return -10;
	}

	int nMAXID = -1;
	int nMAXA = 0;
	cv::Mat ImgBlob = ImgBin.clone();
	ImgBin.setTo(0);
	for (int a = 0; a < allcontour.size(); a++)
	{
		ImgTemp.setTo(0);
		cv::drawContours(ImgTemp, allcontour, a, cv::Scalar(255), cv::FILLED);
		if (bFillHole == FALSE)
			cv::bitwise_and(ImgTemp, ImgBlob, ImgTemp);
		int nArea = cv::countNonZero(ImgTemp);
		if (nArea <= nMinBlobArea)
			continue;

		if (nTypeSelectBlob == eSelectMix)
			cv::bitwise_or(ImgBin, ImgTemp, ImgBin);

		if (nArea > nMAXA)
		{
			nMAXA = nArea;
			nMAXID = a;
			if (nTypeSelectBlob == eSelectBigger)
				ImgBin = ImgTemp.clone();
		}
	}
	if (nMAXA <= nMinBlobArea || nMAXID < 0)
	{
		sWndAlgoImg.Destroy();
		return -11;
	}

	int nTYPE_P = sInspData.m_nTYPE;
	if (nTYPE_P <= m_ePAD_PARALLEL_None)
		nTYPE_P = m_ePAD_PARALLEL_None;
	double dRstShiftX = 0;
	double dRstShiftY = 0;
	double dRstShiftXPix = 0;
	double dRstShiftYPix = 0;
	InspAlgoParam stAlgoParam;
	stAlgoParam.m_nViewMode == 0;
	stAlgoParam.m_sPadPoly = sInspData.m_sPadPoly;
	stAlgoParam.m_nOverlapPadPolyCnt = sInspData.m_nOverlapPadPolyCnt;
	stAlgoParam.m_sArrOverlapPadPoly = sInspData.m_sArrOverlapPadPoly;
	if (nTYPE_P >= m_ePAD_PARALLEL_Type2)
	{
		dRstShiftX = 5 * m_resolX;
		dRstShiftY = 5 * m_resolY;
	}
	RstAlgoVolume sRstAlgo;
	sRstAlgo.Init();
	BOOL bRET = PADPatternCheck(sWndAlgoImg, pAlgo, stAlgoParam, &sRstAlgo, ImgBin.data, NULL, nW, nH, dRstShiftX, dRstShiftY, dRstShiftXPix, dRstShiftYPix, nTYPE_P, pPAT_RST, pucPOLY);

	pPAT_RST->m_bPAD_RET = bRET;
	pPAT_RST->m_nRstPAD_CHK = sRstAlgo.m_nRstPAD_CHK;
	pPAT_RST->m_nOKPadPatternCheck = sRstAlgo.m_nOKPadPatternCheck;
	pPAT_RST->m_nPadPatternCheck = sRstAlgo.m_nPadPatternCheck;
	pPAT_RST->m_fRstNotOverlapArea = sRstAlgo.m_dRstNotOverlapArea;
	pPAT_RST->m_fRst_TH_MIN = sRstAlgo.m_fArr[m_eVolumeR_Rst_TH_MIN];
	pPAT_RST->m_fRst_TH_MAX = sRstAlgo.m_fArr[m_eVolumeR_Rst_TH_MAX];
	for (int a = 0; a < sRstAlgo.m_nRstPAD_CHK; a++)
		pPAT_RST->AddPAT_CHK(sRstAlgo.m_sRstPAD_CHK[a]);
	for (int a = 0; a < VOLUME_INSP_CNTS; a++)
	{
		pPAT_RST->m_rcRect_TH[a].left = sRstAlgo.m_rcRect_TH[a].left;
		pPAT_RST->m_rcRect_TH[a].right = sRstAlgo.m_rcRect_TH[a].right;
		pPAT_RST->m_rcRect_TH[a].top = sRstAlgo.m_rcRect_TH[a].top;
		pPAT_RST->m_rcRect_TH[a].bottom = sRstAlgo.m_rcRect_TH[a].bottom;
	}
	sWndAlgoImg.Destroy();
	return 1;
}

void PAD_PAT::RoiImageCompose_LT(CProcMil *m_pProcMilAlgo, UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage, double dAngle)
{
	float fRedValue = (float)nRedValue / 100.0;
	float fGreenValue = (float)nGreenValue / 100.0;
	float fBlueValue = (float)nBlueValue / 100.0;
	float fWhiteValue = (float)nWhiteValue / 100.0;

	int nValueBuf = 0;
	float fRedImageValue = 0;
	float fGreenImageValue = 0;
	float fBlueImageValue = 0;
	float fWhiteImageValue = 0;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	int nMaxValue = nFullImageSize / (nFullImageWidth*nFullImageHeight);
	if (nMaxValue <= 0)	nMaxValue = 1;

	nRedValue = (int)(fRedValue * 128.0);
	nGreenValue = (int)(fGreenValue * 128.0);
	nBlueValue = (int)(fBlueValue * 128.0);
	nWhiteValue = (int)(fWhiteValue * 128.0);

	__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
	__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
	__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
	__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);
	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);


	for (int i = 0; i < nMaxValue; i++)
	{
		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * iptrbyResultImage = (&ptrbyResultImage[(y - nStartY)*nROIWidth]);
			int x = 0;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue;
				register __m128i fGreenImageValue;
				register __m128i fBlueImageValue;
				register __m128i fWhiteImageValue;

				if (!ptrbyRedBuffer)
					fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
				else
					fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyRedBuffer[FImgIdx]);
				if (!ptrbyGreenBuffer)
					fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
				else
					fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyGreenBuffer[FImgIdx]);
				if (!ptrbyBlueBuffer)
					fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
				else
					fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBlueBuffer[FImgIdx]);
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
				else
					fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyWhiteBuffer[FImgIdx]);

				_mm_storeu_si128(
					(__m128i *)(&iptrbyResultImage[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (!ptrbyRedBuffer)
					fRedImageValue = 0;
				else
					fRedImageValue = ptrbyRedBuffer[FImgIdx];
				if (!ptrbyGreenBuffer)
					fGreenImageValue = 0;
				else
					fGreenImageValue = ptrbyGreenBuffer[FImgIdx];
				if (!ptrbyBlueBuffer)
					fBlueImageValue = 0;
				else
					fBlueImageValue = ptrbyBlueBuffer[FImgIdx];
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = 0;
				else
					fWhiteImageValue = ptrbyWhiteBuffer[FImgIdx];

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				iptrbyResultImage[(x - nStartX)] = nValueBuf;
			}
		}
	}
	if (dAngle == 0)
		return;

	UCHAR* ptrSrc = ptrbyResultImage;
	UCHAR* ptrbyBuffer_ro = NULL;
	int retDstSizeX(0), retDstSizeY(0);
	int GapX = 0;
	int GapY = 0;
	if (ptrbyResultImage != NULL)
	{
		m_pProcMilAlgo->RotateImg_ipp(ptrbyResultImage, nROIWidth, nROIHeight, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);

		GapX = RounD((retDstSizeX - nROIWidth) / 2);
		GapY = RounD((retDstSizeY - nROIHeight) / 2);
		if ((GapX + nROIWidth) > retDstSizeX)
			GapX = 0;
		if ((GapY + nROIHeight) > retDstSizeY)
			GapY = 0;
		for (int r = 0; r < nROIHeight; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];
			UCHAR* dstPtr = &ptrSrc[r*nROIWidth];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*nROIWidth);
		}
		m_pProcMilAlgo->SaveWorkImg(ptrbyBuffer_ro, retDstSizeX, retDstSizeY, _T("ptrbyBuffer_ro Red.bmp"));
		m_pProcMilAlgo->SaveWorkImg(ptrSrc, nROIWidth, nROIHeight, _T("ptrbyRedBuffer.bmp"));

		if (ptrbyBuffer_ro != NULL)
			//delete [] ptrbyBuffer_ro;
			g_pMManager->pem_delete(ptrbyBuffer_ro, true);
	}

}
void PAD_PAT::FullImageClaculCompose(CProcMil *m_pProcMilAlgo, const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage)
{
	int nWidth = sLightImg.m_nImgWidth;
	int nHeight = sLightImg.m_nImgHeight;
	int nImgCnt = sLightImg.m_nImgCnt;
	if ((nWidth <= 0) || (nHeight <= 0) || (nImgCnt <= 0))
		return;
	int nImageSize = nWidth * nHeight;

	UCHAR* ptrbyTopRedBuffer = sLightImg.m_pucTRed;
	UCHAR* ptrbyTopGreenBuffer = sLightImg.m_pucTGreen;
	UCHAR* ptrbyTopBlueBuffer = sLightImg.m_pucTBlue;
	UCHAR* ptrbyTopWhiteBuffer = sLightImg.m_pucTWhite;

	UCHAR* ptrbyMidRedBuffer = sLightImg.m_pucMRed;
	UCHAR* ptrbyMidGreenBuffer = sLightImg.m_pucMGreen;
	UCHAR* ptrbyMidBlueBuffer = sLightImg.m_pucMBlue;
	UCHAR* ptrbyMidWhiteBuffer = sLightImg.m_pucMWhite;

	UCHAR* ptrbyBotRedBuffer = sLightImg.m_pucBRed;
	UCHAR* ptrbyBotGreenBuffer = sLightImg.m_pucBGreen;
	UCHAR* ptrbyBotBlueBuffer = sLightImg.m_pucBBlue;
	UCHAR* ptrbyBotWhiteBuffer = sLightImg.m_pucBWhite;

	int* pnRedValue = sLightImg.m_pnRedValue;
	int* pnGreenValue = sLightImg.m_pnGreenValue;
	int* pnBlueValue = sLightImg.m_pnBlueValue;
	int* pnWhiteValue = sLightImg.m_pnWhiteValue;

	int *ptrnPosition = sLightImg.m_pnPosition;
	int *nCalculation = sLightImg.m_pnCalculation;
	if ((nCalculation == NULL) || (ptrbyResultImage == NULL))
		return;

	int nValueBuf = 0;
	float fRedImageValue = 0.0;
	float fGreenImageValue = 0.0;
	float fBlueImageValue = 0.0;
	float fWhiteImageValue = 0.0;

	float fRedValue = 0.0;
	float fGreenValue = 0.0;
	float fBlueValue = 0.0;
	float fWhiteValue = 0.0;

	UCHAR** ppucComposeImageBuf = NULL;
	//ppucComposeImageBuf = new UCHAR*[nImgCnt];
	ppucComposeImageBuf = g_pMManager->pem_new<UCHAR*>(true, nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	for (int i = 0; i < nImgCnt; i++)
	{
		//ppucComposeImageBuf[i] = new UCHAR[nImageSize];
		ppucComposeImageBuf[i] = g_pMManager->pem_new<UCHAR>(true, nImageSize, (PCHAR)__FUNCTION__, __LINE__);
	}


	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);

	for (int a = 0; a < nImgCnt; a++)
	{
		nValueBuf = 0;

		fRedValue = (double)pnRedValue[a] / 100.0;
		fGreenValue = (double)pnGreenValue[a] / 100.0;
		fBlueValue = (double)pnBlueValue[a] / 100.0;
		fWhiteValue = (double)pnWhiteValue[a] / 100.0;

		int nRedValue = (int)(fRedValue * 128.0);
		int nGreenValue = (int)(fGreenValue * 128.0);
		int nBlueValue = (int)(fBlueValue * 128.0);
		int nWhiteValue = (int)(fWhiteValue * 128.0);

		__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
		__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
		__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
		__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);

		for (int y = 0; y < nHeight; y++)
		{
			UCHAR * ippucComposeImageBuf = (UCHAR *)(&ppucComposeImageBuf[a][(y)*nWidth]);
			int x;
			for (x = 0; (x + 16) < nWidth; x += 16)
			{


				int FImgIdx = y * nWidth + x;

				register __m128i fRedImageValue = zeroBuf;
				register __m128i fGreenImageValue = zeroBuf;
				register __m128i fBlueImageValue = zeroBuf;
				register __m128i fWhiteImageValue = zeroBuf;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopRedBuffer[FImgIdx]);
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopGreenBuffer[FImgIdx]);
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopBlueBuffer[FImgIdx]);
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidRedBuffer[FImgIdx]);
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidGreenBuffer[FImgIdx]);
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidBlueBuffer[FImgIdx]);
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotRedBuffer[FImgIdx]);
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotGreenBuffer[FImgIdx]);
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotBlueBuffer[FImgIdx]);
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotWhiteBuffer[FImgIdx]);
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				_mm_storeu_si128(
					(__m128i *)(&ippucComposeImageBuf[(x)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < nWidth; x++)
			{
				int FImgIdx = y * nWidth + x;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyTopRedBuffer[FImgIdx];
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyTopGreenBuffer[FImgIdx];
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyTopBlueBuffer[FImgIdx];
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyTopWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyMidRedBuffer[FImgIdx];
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyMidGreenBuffer[FImgIdx];
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyMidBlueBuffer[FImgIdx];
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyMidWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyBotRedBuffer[FImgIdx];
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyBotGreenBuffer[FImgIdx];
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyBotBlueBuffer[FImgIdx];
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyBotWhiteBuffer[FImgIdx];
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				if (nValueBuf < 0)
					nValueBuf = 0;
				ippucComposeImageBuf[x] = nValueBuf;
			}

		}
		m_pProcMilAlgo->SaveWorkImg(ppucComposeImageBuf[a], nWidth, nHeight, _T("ComposeImageBuf.bmp"));
	}

	// 	for (int a = 0; a< nImgCnt; a++)
	// 	{
	// 		nValueBuf = 0;
	// 
	// 		fRedValue = (float)pnRedValue[a] / 100.0;
	// 		fGreenValue = (float)pnGreenValue[a] / 100.0;
	// 		fBlueValue = (float)pnBlueValue[a] / 100.0;
	// 		fWhiteValue = (float)pnWhiteValue[a] / 100.0;
	// 
	// 		for (int b = 0; b < nImageSize; b++)
	// 		{
	// 			if(ptrnPosition[a] == 0)
	// 			{
	// 				if (!ptrbyTopRedBuffer)
	// 					fRedImageValue = 0;
	// 				else
	// 					fRedImageValue = ptrbyTopRedBuffer[b];
	// 				if (!ptrbyTopGreenBuffer)
	// 					fGreenImageValue = 0;
	// 				else
	// 					fGreenImageValue = ptrbyTopGreenBuffer[b];
	// 				if (!ptrbyTopBlueBuffer)
	// 					fBlueImageValue = 0;
	// 				else
	// 					fBlueImageValue = ptrbyTopBlueBuffer[b];
	// 				if (!ptrbyTopWhiteBuffer)
	// 					fWhiteImageValue = 0;
	// 				else
	// 					fWhiteImageValue = ptrbyTopWhiteBuffer[b];
	// 			}
	// 			else if(ptrnPosition[a] == 1)
	// 			{
	// 				if (!ptrbyMidRedBuffer)
	// 					fRedImageValue = 0;
	// 				else
	// 					fRedImageValue = ptrbyMidRedBuffer[b];
	// 				if (!ptrbyMidGreenBuffer)
	// 					fGreenImageValue = 0;
	// 				else
	// 					fGreenImageValue = ptrbyMidGreenBuffer[b];
	// 				if (!ptrbyMidBlueBuffer)
	// 					fBlueImageValue = 0;
	// 				else
	// 					fBlueImageValue = ptrbyMidBlueBuffer[b];
	// 				if (!ptrbyMidWhiteBuffer)
	// 					fWhiteImageValue = 0;
	// 				else
	// 					fWhiteImageValue = ptrbyMidWhiteBuffer[b];
	// 			}
	// 			else if(ptrnPosition[a] == 2)
	// 			{
	// 				if (!ptrbyBotRedBuffer)
	// 					fRedImageValue = 0;
	// 				else
	// 					fRedImageValue = ptrbyBotRedBuffer[b];
	// 				if (!ptrbyBotGreenBuffer)
	// 					fGreenImageValue = 0;
	// 				else
	// 					fGreenImageValue = ptrbyBotGreenBuffer[b];
	// 				if (!ptrbyBotBlueBuffer)
	// 					fBlueImageValue = 0;
	// 				else
	// 					fBlueImageValue = ptrbyBotBlueBuffer[b];
	// 				if (!ptrbyBotWhiteBuffer)
	// 					fWhiteImageValue = 0;
	// 				else
	// 					fWhiteImageValue = ptrbyBotWhiteBuffer[b];
	// 			}
	// 
	// 			nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));
	// 
	// 			if (nValueBuf > 255) 
	// 				nValueBuf = 255;
	// 			ppucComposeImageBuf[a][b] = nValueBuf;
	// 		}
	// 		m_pProcMilAlgo->SaveWorkImg(ppucComposeImageBuf[a], nWidth, nHeight, _T("ComposeImageBuf.bmp"));
	// 	}

	//UCHAR *pnImageValue = new UCHAR[nImageSize];
	UCHAR *pnImageValue = g_pMManager->pem_new<UCHAR>(true, nImageSize, (PCHAR)__FUNCTION__, __LINE__);
	int nImgValue = 0;
	for (int a = 0; a < nImgCnt; a++)
	{
		for (int b = 0; b < nImageSize; b++)
		{
			if (a == 0)
				nImgValue = ppucComposeImageBuf[a][b];
			else
			{
				if (nCalculation[a - 1] == 1)
					nImgValue = pnImageValue[b] + ppucComposeImageBuf[a][b];
				else if (nCalculation[a - 1] == 2)
					nImgValue = pnImageValue[b] - ppucComposeImageBuf[a][b];
			}

			if (nImgValue > 255)
				nImgValue = 255;
			if (nImgValue < 0)
				nImgValue = 0;

			pnImageValue[b] = nImgValue;
		}
		m_pProcMilAlgo->SaveWorkImg(pnImageValue, nWidth, nHeight, _T("ImageValue.bmp"));
	}
	for (int i = 0; i < nImageSize; i++)
	{
		ptrbyResultImage[i] = pnImageValue[i];
	}
	m_pProcMilAlgo->SaveWorkImg(ptrbyResultImage, nWidth, nHeight, _T("ResultImage.bmp"));
	if (pnImageValue)
	{
		//delete [] pnImageValue;
		g_pMManager->pem_delete(pnImageValue, true);
		pnImageValue = NULL;
	}
	if (ppucComposeImageBuf)
	{
		for (int i = 0; i < nImgCnt; i++)
		{
			//delete ppucComposeImageBuf[i];

			// MemoryLeak Fix -wjlee
			//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
			g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

			ppucComposeImageBuf[i] = NULL;
		}
		//delete [] ppucComposeImageBuf;
		g_pMManager->pem_delete(ppucComposeImageBuf, true);
		ppucComposeImageBuf = NULL;
	}
}
int PAD_PAT::Binarize(cv::Mat imgSrc, PAD_2D_DATA* pfRst)
{
	if (imgSrc.empty())
		return -4;
	else if (pfRst->m_pBIN == NULL)
		return -5;
	else if (pfRst->m_nW <= 0 || pfRst->m_nH <= 0)
		return -6;

	cv::Mat imgBIN(pfRst->m_nH, pfRst->m_nW, CV_8UC1, pfRst->m_pBIN);
	imgBIN.setTo(0);
	int nMIN = pfRst->m_nMIN;
	int nMAX = pfRst->m_nMAX;
	if (nMIN < 0) nMIN = 0;
	if (nMAX < 0) nMAX = 0;
	if (nMIN > 255) nMIN = 255;
	if (nMAX > 255) nMAX = 255;
	if (pfRst->m_nRange == eTypeRangeIn || pfRst->m_nRange == eTypeRangeOut)
	{
		if (nMIN > nMAX) nMAX = nMIN;
		cv::Mat imgMin;
		cv::Mat imgMax;
		int nMinT = pfRst->m_nRange == eTypeRangeOut ? nMAX : nMIN;
		int nMaxT = pfRst->m_nRange == eTypeRangeOut ? nMIN : nMAX;
		cv::threshold(imgSrc, imgMin, nMinT, 255, cv::THRESH_BINARY);
		cv::threshold(imgSrc, imgMax, nMaxT, 255, cv::THRESH_BINARY_INV);
		cv::bitwise_and(imgMin, imgMax, imgBIN);
	}
	else if (pfRst->m_nRange == eTypeRangeUpper)
	{
		cv::threshold(imgSrc, imgBIN, nMAX, 255, cv::THRESH_BINARY);
	}
	else if (pfRst->m_nRange == eTypeRangeLower)
	{
		cv::threshold(imgSrc, imgBIN, nMIN, 255, cv::THRESH_BINARY_INV);
	}
	else
		return -7;

	return 1;
}
#pragma endregion

bool PAD_PAT::INSP_SHIFT_AREA(AlgoVolume *pAlgo, WndAlgoImg &sWndAlgoImg, cv::Mat ImgPoly, UCHAR* pUcArrDstImg, int nW, int nH)
{
	int nLine = __LINE__;
	try
	{
		if (pAlgo == NULL || ImgPoly.empty() || pUcArrDstImg == NULL || nW <= 0 || nH <= 0)
			return false;
		nLine = __LINE__;

		cv::Mat imgRst(nH, nW, CV_8UC1, pUcArrDstImg);
		double dAngle = sWndAlgoImg.dAngle;
		bool bROT = (dAngle == 90 || dAngle == 270);
		double dResolX = (bROT) ? m_resolY : m_resolX;
		double dResolY = (bROT) ? m_resolX : m_resolY;
		double dShiftX = (bROT) ? pAlgo->GetData(m_eVolumeF_ShiftY) : pAlgo->GetData(m_eVolumeF_ShiftX);
		double dShiftY = (bROT) ? pAlgo->GetData(m_eVolumeF_ShiftX) : pAlgo->GetData(m_eVolumeF_ShiftY);
		int nShiftX = (dShiftX / dResolX) + 1;
		int nShiftY = (dShiftY / dResolY) + 1;
		if (nShiftX <= 0 || nShiftY <= 0)
			return false;
		nLine = __LINE__;

		std::vector<std::vector<cv::Point>> allcontour;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(imgRst, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		nLine = __LINE__;
		if (allcontour.size() == 0)
			return false;

		int nMorX = 1 + (nShiftX * 2);
		int nMorY = 1 + (nShiftY * 2);
		if (nMorX > 100) nMorX = 100;
		if (nMorY > 100) nMorY = 100;

		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nMorX, nMorY));
		cv::Mat ImgPolyD(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::dilate(ImgPoly, ImgPolyD, kernel);

		int nMorX1 = (int)(nMorX * 0.7);
		int nMorY1 = (int)(nMorY * 0.7);
		cv::Mat kernel1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nMorX1, nMorY1));
		cv::Mat ImgPolyD1(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::dilate(ImgPoly, ImgPolyD1, kernel1);

		int nPol = cv::countNonZero(ImgPoly);
		if (nPol <= 0)
			return true;

		cv::Mat imgDst(nH, nW, CV_8UC1, cv::Scalar(0));
		int nTotalCnt = 0;

		cv::Mat imgDst1(nH, nW, CV_8UC1, cv::Scalar(0));
		int nTotalCnt1 = 0;

		cv::Mat imgDst2(nH, nW, CV_8UC1, cv::Scalar(0));
		int nTotalCnt2 = 0;
		cv::Mat imgTemp_Poly(nH, nW, CV_8UC1, cv::Scalar(0));
		for (int a = 0; a < allcontour.size(); a++)
		{
			imgTemp_Poly.setTo(0);
			cv::drawContours(imgTemp_Poly, allcontour, a, cv::Scalar(255), cv::FILLED);
			cv::Mat imgTemp;
			cv::bitwise_and(imgTemp_Poly, imgRst, imgTemp);

			cv::Mat imgTempA;
			cv::bitwise_and(imgTemp, ImgPoly, imgTempA);
			int nSrcA = cv::countNonZero(imgTempA);
			if (nSrcA > 0 && nSrcA == nPol)
			{
				memcpy(pUcArrDstImg, imgTemp.data, nW * nH * sizeof(UCHAR));
				return true;
			}

			int nSrc = cv::countNonZero(imgTemp);
			int nFind = 0;
			if (nSrc > nTotalCnt2)
			{
				memcpy(imgDst2.data, imgTemp.data, nW * nH * sizeof(UCHAR));
				nTotalCnt2 = nSrc;
			}
			int nReCnt = 2;
			for (int nRe = 0; nRe < nReCnt; nRe++)
			{
				cv::Mat imgP = (nRe == 0) ? ImgPolyD : ImgPolyD1;
				nLine = __LINE__;

				cv::Mat imgAnd;
				cv::bitwise_and(imgTemp, imgP, imgAnd);
				int nCnt = cv::countNonZero(imgAnd);
				if (nCnt == 0)
					break;

				double dPer = (double)nCnt / (double)nPol;
				if (dPer < 0.005)
					continue;

				if (nSrc == nCnt && dPer > 0.95)
				{
					if (nSrc == nPol)
					{
						memcpy(pUcArrDstImg, imgTemp.data, nW * nH * sizeof(UCHAR));
						return true;
					}
					nFind += pow(2, nRe);
					imgDst.setTo(0);
					imgDst1.setTo(0);
				}
				else if (nSrc == nCnt || dPer > 0.9)
				{
					if (nSrc == nPol)
					{
						memcpy(pUcArrDstImg, imgTemp.data, nW * nH * sizeof(UCHAR));
						return true;
					}
					nFind += pow(2, nRe);
				}
				else if (dPer > 0.2)
					nFind += pow(2, nReCnt);
			}
			if (nFind > 0)
			{
				cv::bitwise_or(imgTemp, imgDst1, imgDst1);
				nTotalCnt1++;
			}
			if ((nFind & 1) == 1)
			{
				cv::bitwise_or(imgTemp, imgDst, imgDst);
				nTotalCnt++;
			}
		}
		if (nTotalCnt > 0)
			memcpy(pUcArrDstImg, imgDst.data, nW * nH * sizeof(UCHAR));
		else if (nTotalCnt1 > 0)
			memcpy(pUcArrDstImg, imgDst1.data, nW * nH * sizeof(UCHAR));
		else
			memcpy(pUcArrDstImg, imgDst2.data, nW * nH * sizeof(UCHAR));
		return true;
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("PAD_PAT::INSP_SHIFT_AREA(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
		return false;
	}
}
BOOL PAD_PAT::PADPatternCheck(WndAlgoImg &sWndAlgoImg, AlgoVolume* pInspAlgoVolume, InspAlgoParam stAlgoParam, RstAlgoVolume* sRstAlgo, UCHAR* pUcArrDstImg, UCHAR* ucArrDstImg, int nImgWidth, int nImgHeight, double dRstShiftX, double dRstShiftY, double dRstShiftXPix, double dRstShiftYPix, int nTYPE_P, INSP_PAD_PAT_RST* pPAT_RST, UCHAR* pucPOLY)
{
	DWORD st = GetTickCount();
	BOOL bResult = TRUE;

	int nLine = __LINE__;
	try
	{
		if (sRstAlgo == nullptr || sRstAlgo == NULL)
			return FALSE;

		sRstAlgo->m_nOKPadPatternCheck = 0;
		sRstAlgo->m_nPadPatternCheck = 0;
		sRstAlgo->m_dRstNotOverlapArea = 0;
		sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN] = 0;
		sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX] = 0;
		sRstAlgo->m_fArr[m_eVolumeR_Rst_PAT_X] = 0;
		sRstAlgo->m_fArr[m_eVolumeR_Rst_PAT_Y] = 0;
		sRstAlgo->m_nRstPAD_CHK = 0;
		for (int a = 0; a < VOLUME_DIVIDE_AREA_CNTS; a++)
			sRstAlgo->m_sRstPAD_CHK[a].Init();
		if (pInspAlgoVolume->UseData(m_eVolumeData_PAD_PAT_Check) == false)
			return bResult;

		if (stAlgoParam.m_sPadPoly.m_nPolyCnt <= 0 || stAlgoParam.m_sPadPoly.m_ptArrPoly == NULL)
		{
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAD_OUT_1] += 1;
			return bResult;
		}

		nLine = __LINE__;
		cv::Mat ImgBin(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		memcpy(ImgBin.data, pUcArrDstImg, sizeof(UCHAR) * nImgWidth * nImgHeight);
		int nBin = cv::countNonZero(ImgBin);
		if (nBin <= 0)
		{
			sRstAlgo->m_nPadPatternCheck |= m_eVOL_PAT_CHK_AERA;
			sRstAlgo->m_nOKPadPatternCheck |= m_eVOL_PAT_CHK_AERA;
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAD_OUT_2] += 1;
			return FALSE;
		}

		bool bInspSave = (g_pMPTI && g_pMPTI->m_LogLevel == m_eLogLv_Blob);
		bool bView_PAD = false;
		if (ucArrDstImg && stAlgoParam.m_nViewMode == m_eViewMode_Volume_View_PAD)
			bView_PAD = true;
#if _DEBUG
		bInspSave = true;
		bView_PAD = true;
#endif

		int nPolyAreaMargin = 50;
		cv::Mat ImgPolyArea(nImgHeight + (nPolyAreaMargin * 2), nImgWidth + (nPolyAreaMargin * 2), CV_8UC1, cv::Scalar(0));
		cv::Mat ImgPoly(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		cv::Mat ImgPolyFind(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		std::vector<cv::Point> polyPoints;
		polyPoints.reserve(stAlgoParam.m_sPadPoly.m_nPolyCnt); // 메모리 예약
		for (int i = 0; i < stAlgoParam.m_sPadPoly.m_nPolyCnt; i++)
		{
			int nPolyX = (int)std::round(stAlgoParam.m_sPadPoly.m_ptArrPoly[i].x) + nPolyAreaMargin;
			int nPolyY = (int)std::round(stAlgoParam.m_sPadPoly.m_ptArrPoly[i].y) + nPolyAreaMargin;
			polyPoints.push_back(cv::Point(nPolyX, nPolyY));
		}
		cv::fillPoly(ImgPolyArea, std::vector<std::vector<cv::Point>>{polyPoints}, cv::Scalar(255));
		cv::Rect rcPolyArea(nPolyAreaMargin, nPolyAreaMargin, nImgWidth, nImgHeight);
		ImgPolyArea(rcPolyArea).copyTo(ImgPoly);

		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_SET] += ((GetTickCount() - st) / 1000.0f);
		nLine = __LINE__;

		int nOUT_FIND = 0;
		bool bSH = false;
		RECT_F rcFIND_RST;
		rcFIND_RST.Init();
		int nGAPX = 5;
		int nGAPY = 5;
		int nPixX = 0;
		int nPixY = 0;
		std::vector<RECT_F> vecROI;
		double dRST_CNT = 0;
		double dRST_MAX_AREA = 0;
		double dRST_MAX_MOR = nImgWidth * nImgHeight;
		double dRstShiftX_PIX = dRstShiftX / m_resolX;
		double dRstShiftY_PIX = dRstShiftY / m_resolY;
		if (pInspAlgoVolume->UseData(m_eVolumeData_Shift))
		{
			double dPixX = dRstShiftX_PIX;
			double dPixY = dRstShiftY_PIX;
			if (pInspAlgoVolume->UseData_M(m_ePAT_ALGO_N_DATA_SHIFT))
			{
				if (abs(dRstShiftXPix) > abs(dPixX)) dPixX = dRstShiftXPix;
				if (abs(dRstShiftYPix) > abs(dPixY)) dPixY = dRstShiftYPix;
				bSH = true;
			}
			if (dPixX < 0) dPixX *= -1.0;
			if (dPixY < 0) dPixY *= -1.0;
			nPixX = (int)std::round(dPixX) + nGAPX;
			nPixY = (int)std::round(dPixY) + nGAPY;
			if (nPixX > 20) nPixX = 20;
			if (nPixY > 20) nPixY = 20;
			if (dPixX <= 1) nGAPX = 8;
			if (dPixY <= 1) nGAPY = 8;
		}

		cv::Rect roiP_ORG = cv::boundingRect(ImgPoly);
		cv::Rect roiB_ORG = cv::boundingRect(ImgBin);
		cv::Rect roiPB_ORG = roiP_ORG | roiB_ORG;
		bool bONE_ORG = false;
		if (roiPB_ORG.x > N_GAP_ORG && roiPB_ORG.br().x < nImgWidth - N_GAP_ORG &&
			roiPB_ORG.y > N_GAP_ORG && roiPB_ORG.br().y < nImgHeight - N_GAP_ORG)
		{
			bONE_ORG = true;
		}

		for (int a = 0; a < 4; a++)
		{
			if (bONE_ORG == true)
			{
				int nFIND_RST_X = (int)std::round(dRstShiftX_PIX * (-1.0));
				int nFIND_RST_Y = (int)std::round(dRstShiftY_PIX);
				rcFIND_RST.top = nFIND_RST_X;
				rcFIND_RST.bottom = nFIND_RST_Y;

				cv::Rect rcShiftedPolyArea(nPolyAreaMargin + nFIND_RST_X, nPolyAreaMargin + nFIND_RST_Y, nImgWidth, nImgHeight);
				cv::Rect rcImgPolyArea(0, 0, ImgPolyArea.cols, ImgPolyArea.rows);
				cv::Rect rcClippedArea = rcShiftedPolyArea & rcImgPolyArea;
				if (rcClippedArea.area() > 0)
				{
					ImgPoly.setTo(0);
					ImgPolyArea(rcClippedArea).copyTo(ImgPoly(cv::Rect(rcClippedArea.tl() - rcShiftedPolyArea.tl(), rcClippedArea.size())));
				}

				cv::Mat ImgTempA(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
				cv::Mat ImgTempXOR(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));

				cv::bitwise_and(ImgPoly, ImgBin, ImgTempA);
				int nPolyTemp = cv::countNonZero(ImgTempA);

				cv::bitwise_xor(ImgPoly, ImgBin, ImgTempXOR);
				int nMorTemp = cv::countNonZero(ImgTempXOR);

				rcFIND_RST.left = nPolyTemp;
				rcFIND_RST.right = nMorTemp;

				dRST_CNT = nBin;
				dRST_MAX_AREA = (double)rcFIND_RST.left / dRST_CNT;
				dRST_MAX_MOR = rcFIND_RST.right;

				ImgPolyFind.setTo(0);
				break;
			}

			double dX = 0;
			double dY = 0;
			bool bLAST = a == 3;
			if (a == 1)
			{
				if (bSH == false)
					continue;
				dX = dRstShiftXPix;
				dY = dRstShiftYPix;
				if (dX == 0 && dY == 0)
					continue;
			}
			else if (a == 2)
			{
				dX = dRstShiftX_PIX;
				dY = dRstShiftY_PIX;
				if (dX == 0 && dY == 0)
					continue;
				if (abs(dRstShiftXPix) == abs(dRstShiftX_PIX) && abs(dRstShiftYPix) == abs(dRstShiftY_PIX))
					continue;
				int nXPix_1 = dRstShiftXPix < 0 ? (dRstShiftXPix - 0.5) : (dRstShiftXPix + 0.5);
				int nYPix_1 = dRstShiftYPix < 0 ? (dRstShiftYPix - 0.5) : (dRstShiftYPix + 0.5);
				int nXPix_2 = dRstShiftX_PIX < 0 ? (dRstShiftX_PIX - 0.5) : (dRstShiftX_PIX + 0.5);
				int nYPix_2 = dRstShiftY_PIX < 0 ? (dRstShiftY_PIX - 0.5) : (dRstShiftY_PIX + 0.5);
				int nGAP_X = abs(abs(nXPix_1) - abs(nXPix_2));
				int nGAP_Y = abs(abs(nYPix_1) - abs(nYPix_2));
				if (nGAP_X <= N_POLY_DIFF && nGAP_Y <= N_POLY_DIFF)
					continue;
			}
			else if (bLAST)
			{
				double dMOR_PER = dRST_MAX_MOR / dRST_CNT;
				if (dRST_MAX_AREA >= D_OUT_PER && dMOR_PER <= D_MOR_PER)
					break;
			}
			int nXPix_ORG = dX < 0 ? (dX - 0.5) : (dX + 0.5);
			int nYPix_ORG = dY < 0 ? (dY - 0.5) : (dY + 0.5);
			if (a > 0 && bLAST == false)
			{
				if (nXPix_ORG == 0 && nYPix_ORG == 0)
					continue;
			}
			for (int b = 0; b < 4; b++)
			{
				if (b > 0)
				{
					if (nXPix_ORG == 0 && nYPix_ORG == 0)
						break;
					else if (bLAST)
						break;
				}
				RECT_F rcFIND_RST_TEMP;
				rcFIND_RST_TEMP.Init();

				int nXPix = nXPix_ORG;
				int nYPix = nYPix_ORG;
				if (b == 0 || b == 3)
					nXPix *= -1;
				if (b == 1 || b == 3)
					nYPix *= -1;

				RECT_F rcFIND;
				if (bLAST)
				{
					int nvecROIL = -nPixX;
					int nvecROIR = nPixX;
					int nvecROIT = -nPixY;
					int nvecROIB = nPixY;
					for (int c = 0; c < vecROI.size(); c++)
					{
						if (nvecROIL > vecROI[c].left) nvecROIL = vecROI[c].left;
						if (nvecROIR < vecROI[c].right) nvecROIR = vecROI[c].right;
						if (nvecROIT > vecROI[c].top) nvecROIT = vecROI[c].top;
						if (nvecROIB < vecROI[c].bottom) nvecROIB = vecROI[c].bottom;
					}
					nvecROIL -= 2;
					nvecROIR += 2;
					nvecROIT -= 2;
					nvecROIB += 2;
					rcFIND.SetROI(nvecROIL, nvecROIR, nvecROIT, nvecROIB);
				}
				else
					rcFIND.SetROI_GAP(nXPix, nYPix, nGAPX, nGAPY);

				double dBinCNT = 0;
				ImgPolyFind.setTo(0);

				if (a == 0 && stAlgoParam.m_bInspection == TRUE && pInspAlgoVolume->m_rstPadPatRst.CheckDATA() == true)
				{
					nOUT_FIND = pInspAlgoVolume->m_rstPadPatRst.m_nRST;
					dBinCNT = pInspAlgoVolume->m_rstPadPatRst.m_dCNT;
					dRST_MAX_AREA = pInspAlgoVolume->m_rstPadPatRst.m_dRST_MAX_AREA;
					dRST_MAX_MOR = pInspAlgoVolume->m_rstPadPatRst.m_dRST_MAX_MOR;
					rcFIND_RST.SetData(pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST);
					if (dRST_MAX_AREA <= 0)
						dRST_MAX_AREA = (double)rcFIND_RST.left / dBinCNT;
					if (dRST_MAX_MOR <= 0)
						dRST_MAX_MOR = (double)rcFIND_RST.right;

					int nFIND_RST_X = pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST.top;
					int nFIND_RST_Y = pInspAlgoVolume->m_rstPadPatRst.rcFIND_RST.bottom;
					cv::Rect rcShiftedPolyArea(nPolyAreaMargin + nFIND_RST_X, nPolyAreaMargin + nFIND_RST_Y, nImgWidth, nImgHeight);
					cv::Rect rcImgPolyArea(0, 0, ImgPolyArea.cols, ImgPolyArea.rows);
					cv::Rect rcClippedArea = rcShiftedPolyArea & rcImgPolyArea;
					ImgPoly.setTo(0);
					if (rcClippedArea.area() > 0)
					{
						ImgPolyArea(rcClippedArea).copyTo(ImgPoly(cv::Rect(rcClippedArea.tl() - rcShiftedPolyArea.tl(), rcClippedArea.size())));
					}
					vecROI.push_back(rcFIND);
					if (bInspSave)
					{
						CString sLog = _T("");
						sLog.Format(_T("[Volume][FIND_OFFSET_P]\t%d\t%d\t%.3f\t%.3f\t%d\t%d\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%d"),
							a, b, dX, dY, nXPix, nYPix, dBinCNT, rcFIND_RST.left, rcFIND_RST.right, rcFIND_RST.top, rcFIND_RST.bottom, dRST_MAX_AREA, dRST_MAX_MOR, nOUT_FIND);
						g_pMPTI->AddLog_Dev(sLog);
					}
					break;
				}
				else
					nOUT_FIND = FIND_OFFSET(ImgBin, ImgPolyFind, rcFIND, ImgPolyArea, nPolyAreaMargin, &rcFIND_RST_TEMP, vecROI, dBinCNT, dRST_MAX_AREA, dRST_MAX_MOR);
				if (bInspSave)
				{
					CString sLog = _T("");
					sLog.Format(_T("[Volume][FIND_OFFSET]\t%d\t%d\t%.3f\t%.3f\t%d\t%d\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%d"),
						a, b, dX, dY, nXPix, nYPix, dBinCNT, rcFIND_RST_TEMP.left, rcFIND_RST_TEMP.right, rcFIND_RST_TEMP.top, rcFIND_RST_TEMP.bottom, dRST_MAX_AREA, dRST_MAX_MOR, nOUT_FIND);
					g_pMPTI->AddLog_Dev(sLog);
				}
				vecROI.push_back(rcFIND);
				if (dBinCNT <= 0)
					continue;

				double dMaxPer = (double)rcFIND_RST_TEMP.left / dBinCNT;
				double dMaxMor = rcFIND_RST_TEMP.right;
				if (nOUT_FIND > 0 || (dMaxPer >= dRST_MAX_AREA && dRST_MAX_MOR > dMaxMor) || (dMaxPer >= D_MAX_PER && dRST_MAX_MOR > dMaxMor))
				{
					dRST_MAX_AREA = dMaxPer;
					dRST_MAX_MOR = dMaxMor;
					dRST_CNT = dBinCNT;
					rcFIND_RST.SetData(rcFIND_RST_TEMP);
					memcpy(ImgPoly.data, ImgPolyFind.data, sizeof(UCHAR) * nImgWidth * nImgHeight);
				}
				if (nOUT_FIND > 0)
					break;
			}
			if (nOUT_FIND > 0)
				break;
			if (pInspAlgoVolume->UseData_M(m_ePAT_ALGO_N_DATA_AREA_OUT_NOT))
			{
				double dMOR_PER = dRST_MAX_MOR / dRST_CNT;
				if (dRST_MAX_AREA >= D_OUT_PER && dMOR_PER <= D_MOR_PER)
					break;
			}
		}
		int nMaxX = rcFIND_RST.top;
		int nMaxY = rcFIND_RST.bottom;
		sRstAlgo->m_fArr[m_eVolumeR_Rst_PAT_X] = nMaxX * m_resolX;
		sRstAlgo->m_fArr[m_eVolumeR_Rst_PAT_Y] = nMaxY * m_resolY;
		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_FIND_OFFSET] += ((GetTickCount() - st) / 1000.0f);
		else
		{
			if (pPAT_RST && pPAT_RST != NULL)
			{
				pPAT_RST->m_dCNT = dRST_CNT;
				pPAT_RST->m_dRST_MAX_AREA = dRST_MAX_AREA;
				pPAT_RST->m_dRST_MAX_MOR = dRST_MAX_MOR;
				pPAT_RST->rcFIND_RST.SetData(rcFIND_RST);
				pPAT_RST->m_nRST = nOUT_FIND;
			}
			if (nTYPE_P == m_ePAD_PARALLEL_Type1)
			{
				g_pInspMng->m_fVolTact[m_eVOL_TACT_PAD_OUT_3] += 1;
				return (nOUT_FIND > 0);
			}

			if (pInspAlgoVolume->UseData_M(m_ePAT_ALGO_N_DATA_AREA_OUT_NOT))
			{
				if (dRST_MAX_AREA < D_OUT_PER)
				{
					if (bInspSave)
					{
						CString sLog = _T("");
						sLog.Format(_T("[Volume][FIND_OFFSET][FAIL]\t%d\t%.3f\t%.3f"), nTYPE_P, dRST_MAX_AREA, D_OUT_PER);
						g_pMPTI->AddLog_Dev(sLog);
					}
					if (nTYPE_P == m_ePAD_PARALLEL_Type2)
					{
						g_pInspMng->m_fVolTact[m_eVOL_TACT_PAD_OUT_4] += 1;
						return FALSE;
					}
					else if (dRST_MAX_AREA < D_MIN_PER)
					{
						g_pInspMng->m_fVolTact[m_eVOL_TACT_PAD_OUT_4] += 1;
						return FALSE;
					}
				}
			}
			else
			{
				if (dRST_MAX_AREA < D_MAX_PER)
				{
					if (bInspSave)
					{
						CString sLog = _T("");
						sLog.Format(_T("[Volume][FIND_OFFSET][FAIL2]\t%d\t%.3f\t%.3f"), nTYPE_P, dRST_MAX_AREA, D_MAX_PER);
						g_pMPTI->AddLog_Dev(sLog);
					}
					if (nTYPE_P == m_ePAD_PARALLEL_Type2)
					{
						g_pInspMng->m_fVolTact[m_eVOL_TACT_PAD_OUT_5] += 1;
						return FALSE;
					}
					else if (dRST_MAX_AREA < D_MIN_PER)
					{
						g_pInspMng->m_fVolTact[m_eVOL_TACT_PAD_OUT_5] += 1;
						return FALSE;
					}
				}
			}
			if (pPAT_RST && pPAT_RST != NULL && pucPOLY != NULL)
			{
				memcpy(pucPOLY, ImgPoly.data, nImgWidth * nImgHeight * sizeof(UCHAR));
				pPAT_RST->m_nPOLY_W = nImgWidth;
				pPAT_RST->m_nPOLY_H = nImgHeight;
			}
		}
		nLine = __LINE__;
		if (POLY_IMG_CHECK(ImgPoly) == false)
		{
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAD_OUT_6] += 1;
			return FALSE;
		}

		nLine = __LINE__;
		cv::Mat imgMix(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		if (bView_PAD)
		{
			uchar* mixData = imgMix.data;
			const uchar* polyData = ImgPoly.data;
			const uchar* binData = ImgBin.data;
			int totalPixels = nImgHeight * nImgWidth;
#pragma omp parallel for
			for (int i = 0; i < totalPixels; i++) {
				bool bPAD = polyData[i] == 255;
				bool bBIN = binData[i] == 255;

				if (bPAD && bBIN)
					mixData[i] = 255;
				else if (bPAD)
					mixData[i] = 170;
				else if (bBIN)
					mixData[i] = 85;
			}
			if (ucArrDstImg)
			{
				SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("VOL_ArrDstImg_2.bmp"));
				memcpy(ucArrDstImg, mixData, sizeof(UCHAR) * totalPixels);
			}
		}
		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_MIX] += ((GetTickCount() - st) / 1000.0f);
		nLine = __LINE__;

		if (bInspSave)
		{
			CString sLog = _T("");
			sLog.Format(_T("[Volume][FIND]\t%.5f\t%.5f\t%.5f\t%.5f\t%d\t%d\t%d\t%d\t%.5f\t%.5f"), dRstShiftX_PIX, dRstShiftY_PIX, dRstShiftXPix, dRstShiftYPix, nPixX, nPixY, nMaxX, nMaxY, dRST_MAX_AREA, dRST_MAX_MOR);
			g_pMPTI->AddLog_Dev(sLog);
		}
		nLine = __LINE__;

		PAT_OverlapArea(pInspAlgoVolume, stAlgoParam, sRstAlgo, ImgPoly, ImgBin, ucArrDstImg, nImgWidth, nImgHeight, imgMix);
		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_OverlapArea] += ((GetTickCount() - st) / 1000.0f);
		nLine = __LINE__;

		ImgPoly.copyTo(ImgPolyFind);
		cv::Rect roiP = cv::boundingRect(ImgPoly);
		bool bX = roiP.height > roiP.width;
		if (stAlgoParam.m_sArrOverlapPadPoly != NULL)
		{
			cv::Rect roiB = cv::boundingRect(ImgBin);
			cv::Rect roiPB = roiP | roiB;
			int nEX_W = abs(nMaxX);
			int nEX_H = abs(nMaxY);
			if (nEX_W < PAT_INSP_MIN_SIZE) nEX_W = PAT_INSP_MIN_SIZE;
			if (nEX_H < PAT_INSP_MIN_SIZE) nEX_H = PAT_INSP_MIN_SIZE;
			if (nEX_W > 20) nEX_W = 20;
			if (nEX_H > 20) nEX_H = 20;
			bool bONE = false;
			if (roiPB.x > N_GAP_ORG && roiPB.br().x < nImgWidth - N_GAP_ORG &&
				roiPB.y > N_GAP_ORG && roiPB.br().y < nImgHeight - N_GAP_ORG)
			{
				bONE = true;
			}
			for (int i = 0; i < stAlgoParam.m_nOverlapPadPolyCnt; i++)
			{
				int nROIL = nImgWidth * nImgHeight;
				int nROIR = 0;
				int nROIT = nImgWidth * nImgHeight;
				int nROIB = 0;
				for (int n = 0; n < stAlgoParam.m_sArrOverlapPadPoly[i].m_nPolyCnt; n++)
				{
					int nPolyX = (int)std::round(stAlgoParam.m_sArrOverlapPadPoly[i].m_ptArrPoly[n].x);
					int nPolyY = (int)std::round(stAlgoParam.m_sArrOverlapPadPoly[i].m_ptArrPoly[n].y);
					if (nROIL > nPolyX) nROIL = nPolyX;
					if (nROIR < nPolyX) nROIR = nPolyX;
					if (nROIT > nPolyY) nROIT = nPolyY;
					if (nROIB < nPolyY) nROIB = nPolyY;
				}
				if (nROIL >= nROIR || nROIT >= nROIB)
					continue;
				if (nROIR <= 0 || nROIB <= 0)
					continue;
				if (nROIL >= nImgWidth || nROIT >= nImgHeight)
					continue;

				nROIL -= nMaxX;
				nROIR -= nMaxX;
				nROIT -= nMaxY;
				nROIB -= nMaxY;

				nROIL -= nEX_W;
				nROIR += nEX_W;
				nROIT -= nEX_H;
				nROIB += nEX_H;
				if (bONE)
				{
					if (roiPB.width >= roiPB.height)
					{
						int nGAPA = abs(nROIL - roiPB.x);
						int nGAPB = abs(nROIR - roiPB.br().x);
						if (nGAPB > nGAPA)
						{
							nROIL -= N_GAP_ORG;
							nROIT -= N_GAP_ORG;
							nROIB += N_GAP_ORG;
						}
						else
						{
							nROIR += N_GAP_ORG;
							nROIT -= N_GAP_ORG;
							nROIB += N_GAP_ORG;
						}
					}
					else
					{
						int nGAPA = abs(nROIT - roiPB.y);
						int nGAPB = abs(nROIB - roiPB.br().y);
						if (nGAPB > nGAPA)
						{
							nROIL -= N_GAP_ORG;
							nROIR += N_GAP_ORG;
							nROIT -= N_GAP_ORG;
						}
						else
						{
							nROIL -= N_GAP_ORG;
							nROIR += N_GAP_ORG;
							nROIB += N_GAP_ORG;
						}
					}
				}

				if (nROIL < 0) nROIL = 0;
				if (nROIR < 0) nROIR = 0;
				if (nROIT < 0) nROIT = 0;
				if (nROIB < 0) nROIB = 0;
				if (nROIL >= nImgWidth) nROIL = nImgWidth - 1;
				if (nROIR >= nImgWidth) nROIR = nImgWidth;
				if (nROIT >= nImgHeight) nROIT = nImgHeight - 1;
				if (nROIB >= nImgHeight) nROIB = nImgHeight;
				if (nROIL >= nROIR || nROIT >= nROIB)
					continue;
				if (nROIL < 0 || nROIR <= 0 || nROIT < 0 || nROIB <= 0)
					continue;
				if (nROIL >= nImgWidth || nROIR > nImgWidth || nROIT >= nImgHeight || nROIB > nImgHeight)
					continue;

				int nROIW = nROIR - nROIL;
				int nROIH = nROIB - nROIT;
				int nROIGAP_W = nROIW > roiPB.width ? nROIW - roiPB.width : roiPB.width - nROIW;
				int nROIGAP_H = nROIH > roiPB.height ? nROIH - roiPB.height : roiPB.height - nROIH;
				if (20 > nROIGAP_W)
				{
					nROIL -= (nROIGAP_W / 2 + 1);
					nROIR += (nROIGAP_W / 2 + 1);
					if (nROIL < 0) nROIL = 0;
					if (nROIR < 0) nROIR = 0;
					if (nROIL >= nImgWidth) nROIL = nImgWidth - 1;
					if (nROIR >= nImgWidth) nROIR = nImgWidth;
				}
				if (20 > nROIGAP_H)
				{
					nROIT -= (nROIGAP_H / 2 + 1);
					nROIB += (nROIGAP_H / 2 + 1);
					if (nROIT < 0) nROIT = 0;
					if (nROIB < 0) nROIB = 0;
					if (nROIT >= nImgHeight) nROIT = nImgHeight - 1;
					if (nROIB >= nImgHeight) nROIB = nImgHeight;
				}

				cv::Rect boundRect(
					nROIL,                 // 왼쪽으로 확장
					nROIT,                 // 위쪽으로 확장
					nROIR - nROIL,         // 폭 증가
					nROIB - nROIT         // 높이 증가
				);
				cv::rectangle(ImgPoly, boundRect, cv::Scalar(0), -1);
				cv::rectangle(ImgBin, boundRect, cv::Scalar(0), -1);
			}
		}
		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_VIA] += ((GetTickCount() - st) / 1000.0f);
		nLine = __LINE__;

		SaveWorkImg(ImgBin.data, nImgWidth, nImgHeight, _T("VOL_PAT_BIN.bmp"));
		SaveWorkImg(ImgPoly.data, nImgWidth, nImgHeight, _T("VOL_PAT_POLY.bmp"));
		SaveWorkImg(pUcArrDstImg, nImgWidth, nImgHeight, _T("VOL_PAT_BIN_EO.bmp"));
		PAT_Thickness(sWndAlgoImg, pInspAlgoVolume, ImgPoly, ImgBin, sRstAlgo, bX, nTYPE_P);
		nLine = __LINE__;

		bResult = (sRstAlgo->m_nOKPadPatternCheck > 0) ? FALSE : TRUE;
		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_Thickness] += ((GetTickCount() - st) / 1000.0f);
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("PAD_PAT::PADPatternCheck(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
	return bResult;
}
BOOL PAD_PAT::PAT_OverlapArea(AlgoVolume* pInspAlgoVolume, InspAlgoParam stAlgoParam, RstAlgoVolume* sRstAlgo, cv::Mat ImgPoly, cv::Mat ImgBin, UCHAR* ucArrDstImg, int nImgWidth, int nImgHeight, cv::Mat imgMix)
{
	BOOL bResult = TRUE;

	int nLine = __LINE__;
	try
	{
		if (sRstAlgo == nullptr || sRstAlgo == NULL)
			return FALSE;

		sRstAlgo->m_nRstPAD_CHK = 0;
		sRstAlgo->m_dRstNotOverlapArea = 0;
		for (int a = 0; a < VOLUME_DIVIDE_AREA_CNTS; a++)
			sRstAlgo->m_sRstPAD_CHK[a].Init();
		if (pInspAlgoVolume->UseData(m_eVolumeData_PAD_PAT_Check) == false)
			return bResult;

		int nFind = 2;
		int nImgW = ImgPoly.cols;
		int nImgH = ImgPoly.rows;
		cv::Rect roiP = cv::boundingRect(ImgPoly);
		float fUM = 1000.0f;
		int nMOR = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_AREA_MOR);
		float fAREA_W_MAX = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_AREA_W_MAX) / fUM;
		float fAREA_W_MAX_X = fAREA_W_MAX / m_resolX;
		float fAREA_W_MAX_Y = fAREA_W_MAX / m_resolY;
		float fAREA_A_MAX_XY = fAREA_W_MAX / m_resolY / m_resolY;
		if (fAREA_W_MAX_X < nFind) fAREA_W_MAX_X = nFind;
		if (fAREA_W_MAX_Y < nFind) fAREA_W_MAX_Y = nFind;

		if (nMOR <= 0) nMOR = 0;
		if (nMOR > 10) nMOR = 10;
		int nMorErode = nMOR + 2;
		double dResolutionXY = m_resolX * m_resolY;
		float fAREA_A_MAX = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_AREA_A_MAX) / fUM / fUM;
		float fAREA_A_MAX_PIX = fAREA_A_MAX / dResolutionXY;
		if (fAREA_A_MAX <= 0)
			return bResult;

		std::vector<std::vector<cv::Point>> allcontour;
		std::vector<cv::Vec4i> hierarchy;
		nLine = __LINE__;
		sRstAlgo->m_nPadPatternCheck |= m_eVOL_PAT_CHK_AERA;
		if (ucArrDstImg && stAlgoParam.m_nViewMode == m_eViewMode_Volume_View_PatternCheck)
		{
			cv::Mat imgV;
			cv::bitwise_xor(ImgBin, ImgPoly, imgV);
			memcpy(ucArrDstImg, imgV.data, sizeof(UCHAR) * nImgWidth * nImgHeight);
			SaveWorkImg(ucArrDstImg, nImgWidth, nImgHeight, _T("VOL_ArrDstImg_3.bmp"));
		}

		nLine = __LINE__;
		int nLONG_OUT = 5;
		cv::Mat imgROI(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		cv::Mat imgClip(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nMorErode, nMorErode));
		for (int a = 0; a < 2; a++)
		{
			cv::Mat imgBIN_BIN(nImgHeight, nImgWidth, CV_8UC1, cv::Scalar(0));
			if (a > 0)
			{
				if (pInspAlgoVolume->UseData_M(m_ePAT_ALGO_N_DATA_AREA_OUT_NOT))
					break;

				allcontour.clear();
				hierarchy.clear();
				cv::findContours(ImgBin, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
				for (int b = 0; b < allcontour.size(); b++)
					cv::drawContours(imgBIN_BIN, allcontour, b, cv::Scalar(255), cv::FILLED);
			}
			else
				imgBIN_BIN = ImgBin.clone();

			allcontour.clear();
			hierarchy.clear();
			cv::Mat imgTemp = a == 0 ? ImgPoly - imgBIN_BIN : imgBIN_BIN - ImgPoly;
			cv::findContours(imgTemp, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			nLine = __LINE__;
			int nDIFF_T = allcontour.size();
			if (nDIFF_T <= 0)
				continue;

			for (int b = 0; b < nDIFF_T; b++)
			{
				cv::Rect roi1 = cv::boundingRect(allcontour[b]);
				if (roi1.tl().x < 0 || roi1.tl().y < 0)
					continue;
				if (roi1.br().x <= 0 || roi1.br().y <= 0)
					continue;
				if (roi1.width <= fAREA_W_MAX_X || roi1.height <= fAREA_W_MAX_Y)
					continue;

				imgROI.setTo(0);
				cv::drawContours(imgROI, allcontour, b, cv::Scalar(255), cv::FILLED);
				cv::Rect roiA = FIND_MAX_ROI(allcontour, b, imgROI);
				if (roiA.tl().x < 0 || roiA.tl().y < 0)
					continue;
				if (roiA.br().x <= 0 || roiA.br().y <= 0)
					continue;
				if (roiA.width <= fAREA_W_MAX_X || roiA.height <= fAREA_W_MAX_Y)
					continue;

				if (nMOR > 0)
				{
					cv::Mat imgTemp_CLIP = imgTemp(roiA);
					cv::Mat imgErode(imgTemp_CLIP.rows, imgTemp_CLIP.cols, CV_8UC1, cv::Scalar(0));
					cv::erode(imgTemp_CLIP, imgErode, kernel);
					double dErode = cv::countNonZero(imgErode);
					if (dErode <= nFind)
						continue;
					cv::Rect roiErode = cv::boundingRect(imgErode);
					if (roiErode.width <= N_POLY_DIFF || roiErode.height <= N_POLY_DIFF)
						continue;

					int nL = roiErode.x + roiA.x - nFind;
					int nT = roiErode.y + roiA.y - nFind;
					int nW = roiErode.width * (nFind * 2);
					int nH = roiErode.height * (nFind * 2);
					if (nL < 0) nL = 0;
					if (nT < 0) nT = 0;
					if (nL + nW > nImgW) nW = nImgW - nL;
					if (nT + nH > nImgH) nH = nImgH - nT;
					if (nW <= N_POLY_DIFF || nH <= N_POLY_DIFF)
						continue;
					roiA = cv::Rect(nL, nT, nW, nH);
				}

				imgClip.setTo(0);
				imgTemp(roiA).copyTo(imgClip(roiA));
				int nRst = cv::countNonZero(imgClip);
				double dRst = nRst * dResolutionXY;
				if (dRst < fAREA_A_MAX || nRst < fAREA_A_MAX_PIX)
					continue;

				cv::Rect roiRST = cv::boundingRect(imgClip);
				if (roiRST.width <= fAREA_W_MAX_X || roiRST.height <= fAREA_W_MAX_Y)
					continue;

				double dRST_W = roiRST.width;
				double dRST_H = roiRST.height;
				double dGAP = 5;
				double dRstPER = nRst / (dRST_W * dRST_H);
				bool bCheck = (dRstPER < 0.3);
				if (dRST_W <= 20 || dRST_H <= 20)
				{
					if (dRST_W > dRST_H * dGAP || dRST_H > dRST_W * dGAP)
						bCheck = true;
				}
				if (bCheck)
				{
					bool bX = dRST_W > dRST_H;
					int nROISIZE = bX ? nImgW : nImgH;
					int nROIA = bX ? roiRST.width : roiRST.height;
					int nROIA_S = bX ? roiRST.x : roiRST.y;
					int nROIA_E = nROIA_S + nROIA;
					int nROIB = bX ? roiRST.height : roiRST.width;
					int nROIB_S = bX ? roiRST.y : roiRST.x;
					int nROIB_E = nROIB_S + nROIB;
					double dSUM = 0;
					double dCNT = 0;
					for (int d = nROIA_S; d <= nROIA_E; d++)
					{
						if (d < 0 || d >= nROISIZE)
							continue;

						cv::Rect roiR;
						if (bX)
							roiR = cv::Rect(d, roiRST.y, 1, roiRST.height);
						else
							roiR = cv::Rect(roiRST.x, d, roiRST.width, 1);

						cv::Mat imgClipR = imgClip(roiR);
						int nWCNT = cv::countNonZero(imgClipR);
						if (nWCNT <= 0)
							continue;

						dSUM += nWCNT;
						dCNT++;
						if (nWCNT <= fAREA_W_MAX_Y)
							imgClip(roiR).setTo(0);
					}

					roiRST = cv::boundingRect(imgClip);
					dRST_W = roiRST.width;
					dRST_H = roiRST.height;
					if (dCNT > 0)
					{
						if (bX)
							dRST_H = dSUM / dCNT;
						else
							dRST_W = dSUM / dCNT;
					}
				}

				if (dRST_W <= fAREA_W_MAX_X || dRST_H <= fAREA_W_MAX_Y)
					continue;

				RstPAT_CHK sRst;
				sRst.m_fROIW = dRST_W * m_resolX;
				sRst.m_fROIH = dRST_H * m_resolY;
				sRst.m_fROIA = dRst;
				sRst.m_rcRect.left = roiRST.x;
				sRst.m_rcRect.right = roiRST.x + roiRST.width;
				sRst.m_rcRect.top = roiRST.y;
				sRst.m_rcRect.bottom = roiRST.y + roiRST.height;
				if (dRst > sRstAlgo->m_dRstNotOverlapArea)
					sRstAlgo->m_dRstNotOverlapArea = dRst;
				sRstAlgo->AddPAT_CHK(sRst);
			}
			nLine = __LINE__;
		}
		nLine = __LINE__;
		if (sRstAlgo->m_dRstNotOverlapArea >= fAREA_A_MAX)
		{
			bResult = FALSE;
			sRstAlgo->m_nOKPadPatternCheck |= m_eVOL_PAT_CHK_AERA;
		}
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("PAD_PAT::PAT_OverlapArea(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return bResult;
}
cv::Rect PAD_PAT::FIND_MAX_ROI(std::vector<std::vector<cv::Point>> arrBin, int nIDX, cv::Mat imgMor)
{
	cv::Rect roiA = cv::boundingRect(arrBin[nIDX]);
	if (roiA.tl().x < 0 || roiA.tl().y < 0)
		return roiA;
	if (roiA.br().x <= 0 || roiA.br().y <= 0)
		return roiA;
	if (roiA.width <= N_POLY_DIFF || roiA.height <= N_POLY_DIFF)
		return roiA;

	int nW = imgMor.cols;
	int nH = imgMor.rows;
	int nROIW = roiA.width;
	int nROIH = roiA.height;
	bool bChange = false;
	bool bX = nROIW > nROIH;
	for (int a = 0; a < 2; a++)
	{
		if (a == 0 && nROIW != nW)
			continue;
		if (a == 1 && nROIH != nH)
			continue;

		bX = a == 1;
		int nROIA = bX ? nROIH : nROIW;
		int nA_S = bX ? roiA.tl().y : roiA.tl().x;
		int nA_E = bX ? roiA.br().y : roiA.br().x;
		int nB_S = bX ? roiA.tl().x : roiA.tl().y;
		int nB_E = bX ? roiA.br().x : roiA.br().y;
		int nROIA_MIN = ((double)nROIA * 0.008);
		for (int b = nB_S; b < nB_E; b++)
		{
			int nNON = 0;
			int nWhiteCnt = 0;
			for (int a = nA_S; a < nA_E; a++)
			{
				int nIndex = bX ? (a * nW) + b : (b * nW) + a;
				if (nIndex < 0 || nIndex >= nW * nH)
					continue;
				if (imgMor.data[nIndex] == 255)
					nWhiteCnt++;
				else
				{
					nNON++;
					if (nNON > nROIA_MIN)
						break;
				}
			}
			if (nWhiteCnt < nROIA - nROIA_MIN)
				continue;
			for (int a = nA_S; a < nA_E; a++)
			{
				int nIndex = bX ? (a * nW) + b : (b * nW) + a;
				if (nIndex < 0 || nIndex >= nW * nH)
					continue;
				imgMor.data[nIndex] = 0;
				bChange = true;
			}
		}
	}

	double dMAX_LINE = 20;
	double dBin = cv::countNonZero(imgMor);
	double dPer = dBin / (double)(nROIW * nROIH);
	bX = nROIW > nROIH;
	double dROI_PER = nROIW > nROIH ? (double)nROIW / (double)nROIH : (double)nROIH / (double)nROIW;
	if (0.4 > dPer && (nROIW >= dMAX_LINE && nROIH >= dMAX_LINE) || (nROIW == nW || nROIH == nH) || (0.02 > dPer) || (dROI_PER > 80))
	{
		double dROIA = bX ? nROIH : nROIW;
		double dROIB = bX ? nROIW : nROIH;
		double dROI_PER = dROIA / dROIB;

		int nA_S = bX ? roiA.tl().y : roiA.tl().x;
		int nA_E = bX ? roiA.br().y : roiA.br().x;
		int nB_S = bX ? roiA.tl().x : roiA.tl().y;
		int nB_E = bX ? roiA.br().x : roiA.br().y;
		int nRemove = dROIB * 0.006;
		if (nRemove < 2) nRemove = 2;
		if (0.15 > dPer) nRemove = 3;
		for (int b = nB_S; b <= nB_E; b++)
		{
			int nWhiteCnt = 0;
			for (int a = nA_S; a <= nA_E; a++)
			{
				int nIndex = bX ? (a * nW) + b : (b * nW) + a;
				bool bLast = true;
				if (nIndex >= 0 && nIndex < nW * nH)
				{
					if (imgMor.data[nIndex] == 255)
					{
						nWhiteCnt++;
						bLast = false;
					}
				}
				if (a == nA_E)
					bLast = true;
				if (bLast == false)
					continue;

				if (nWhiteCnt >= 1 && nWhiteCnt <= nRemove)
				{
					for (int c = 1; c <= nWhiteCnt; c++)
					{
						int nIndex1 = bX ? ((a - c) * nW) + b : (b * nW) + (a - c);
						if (nIndex1 >= 0 && nIndex1 < nW * nH)
						{
							if (imgMor.data[nIndex1] == 255)
								imgMor.data[nIndex1] = 0;
						}
					}
					bChange = true;
				}
				nWhiteCnt = 0;
			}
		}
	}

	if (bChange)
	{
		std::vector<std::vector<cv::Point>> allcontour;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(imgMor, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		if (allcontour.size() <= 0)
			return cv::Rect();

		int nMAX_ID = -1;
		int nMAXArea = 0;
		for (int a = 0; a < allcontour.size(); a++)
		{
			int nCNT = cv::contourArea(allcontour[a]);
			if (nCNT > nMAXArea)
			{
				nMAXArea = nCNT;
				nMAX_ID = a;
			}
		}
		if (nMAX_ID < 0 || nMAXArea == 0)
			return cv::Rect();

#if _DEBUG
		cv::Mat imgTemp2(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::drawContours(imgTemp2, allcontour, nMAX_ID, cv::Scalar(255), cv::FILLED);
#endif
		cv::Rect roiB = cv::boundingRect(allcontour[nMAX_ID]);
		return roiB;
	}
	else
		return roiA;

}
int PAD_PAT::FIND_OFFSET(cv::Mat ImgBin, cv::Mat ImgPoly, RECT_F roi, cv::Mat ImgPolyArea, int nPolyAreaMargin, RECT_F* pRST, std::vector<RECT_F> vecROI, double& dBinCNT, double dRST_MAX_AREA, double dRST_MAX_MOR)
{
	int nW = ImgBin.cols;
	int nH = ImgBin.rows;
	pRST->Init();
	pRST->right = nW * nH;
	int nBinCnt = cv::countNonZero(ImgBin);
	dBinCNT = 0;
	if (nBinCnt < PAT_INSP_MIN_SIZE)
		return m_eINSP_PAD_PAT_OUT_FIND_FAIL;

	RECT_F rcMaxB;
	rcMaxB.Init();
	rcMaxB.right = nW * nH;
	double dMaxPerB = 0;
	double dMorPerB = nW * nH;
	cv::Mat ImgTempB(nH, nW, CV_8UC1, cv::Scalar(0));

	RECT_F rcMaxP;
	rcMaxP.Init();
	rcMaxP.right = nW * nH;
	double dMaxPerP = 0;
	double dMorPerP = nW * nH;
	cv::Mat ImgTempP(nH, nW, CV_8UC1, cv::Scalar(0));
	int nPolyCnt = 0;

	cv::Mat ImgTemp_ORG(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat ImgTemp(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat ImgTempA(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat ImgTempXOR(nH, nW, CV_8UC1, cv::Scalar(0));
	int nSTART_X = (roi.left + roi.right) / 2;
	int nSTART_Y = (roi.top + roi.bottom) / 2;

	double dRST_PER_TEMP = D_MIN_PER;
	if (dRST_MAX_AREA >= D_MAX_PER)
		dRST_PER_TEMP = D_MAX_PER;

	std::vector<std::vector<cv::Point>> allcontour_BIN;
	std::vector<cv::Vec4i> hierarchy_BIN;
	cv::findContours(ImgBin, allcontour_BIN, hierarchy_BIN, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;

	for (int nY = 0; nY < 2; nY++)
	{
		int nY_S = nSTART_Y;
		int nY_E = roi.bottom;
		int nY_A = 1;
		if (nY == 1)
		{
			nY_S = nSTART_Y - 1;
			nY_E = roi.top;
			nY_A = -1;
		}
		for (int y = nY_S; (nY == 0 && y <= nY_E) || (nY == 1 && y >= nY_E); y += nY_A)
		{
			for (int nX = 0; nX < 2; nX++)
			{
				int nX_S = nSTART_X;
				int nX_E = roi.right;
				int nX_A = 1;
				if (nX == 1)
				{
					nX_S = nSTART_X - 1;
					nX_E = roi.left;
					nX_A = -1;
				}
				for (int x = nX_S; (nX == 0 && x <= nX_E) || (nX == 1 && x >= nX_E); x += nX_A)
				{
					bool bConti = false;
					for (int a = 0; a < vecROI.size(); a++)
					{
						if (vecROI[a].ContainXY(x, y))
						{
							bConti = true;
							break;
						}
					}
					if (bConti)
						continue;

					cv::Rect rcShiftedPolyArea(nPolyAreaMargin + x, nPolyAreaMargin + y, nW, nH);
					cv::Rect rcImgPolyArea(0, 0, ImgPolyArea.cols, ImgPolyArea.rows);
					cv::Rect rcClippedArea = rcShiftedPolyArea & rcImgPolyArea;
					ImgTemp.setTo(0);
					ImgTemp_ORG.setTo(0);
					if (rcClippedArea.area() > 0)
					{
						ImgPolyArea(rcClippedArea).copyTo(ImgTemp_ORG(cv::Rect(rcClippedArea.tl() - rcShiftedPolyArea.tl(), rcClippedArea.size())));

						if (allcontour_BIN.size() == 1)
						{
							allcontour.clear();
							hierarchy.clear();
							cv::findContours(ImgTemp_ORG, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
							int nIMG_CNT = allcontour.size();
							if (nIMG_CNT > 1)
							{
								int nMAXID = 0;
								int nMAX_A = 0;
								for (int a = 0; a < nIMG_CNT; a++)
								{
									int nArea = cv::contourArea(allcontour[a]);
									if (nArea > nMAX_A)
									{
										nMAX_A = nArea;
										nMAXID = a;
									}
								}

								cv::drawContours(ImgTemp, allcontour, nMAXID, cv::Scalar(255), cv::FILLED);
							}
							else
							{
								ImgTemp_ORG.copyTo(ImgTemp);
							}
						}
						else if (allcontour_BIN.size() > 1)
						{
							ImgTemp_ORG.copyTo(ImgTemp);
						}
					}

					int nTemp = cv::countNonZero(ImgTemp);
					if (nTemp == 0)
						continue;

					cv::bitwise_and(ImgTemp, ImgBin, ImgTempA);
					int nPolyTemp = cv::countNonZero(ImgTempA);
					double dPerB = (double)nPolyTemp / (double)nBinCnt;
					double dPerP = (double)nPolyTemp / (double)nTemp;
					if (dPerB > dRST_PER_TEMP)
					{
						cv::bitwise_xor(ImgTemp, ImgBin, ImgTempXOR);
						int nMorTemp = cv::countNonZero(ImgTempXOR);
						if (dRST_MAX_MOR > nMorTemp)
						{
							if ((nPolyTemp >= rcMaxB.left && rcMaxB.right > nMorTemp) || (dPerB >= D_MIN_PER && rcMaxB.right > nMorTemp))
							{
								rcMaxB.left = nPolyTemp;
								rcMaxB.right = nMorTemp;
								rcMaxB.top = x;
								rcMaxB.bottom = y;
								dMaxPerB = dPerB;
								dMorPerB = (double)rcMaxB.right / (double)nBinCnt;
								memcpy(ImgTempB.data, ImgTemp.data, sizeof(UCHAR) * nW * nH);
							}
							if ((rcMaxB.left > 0 && dMorPerB == 0) || (dMaxPerB >= D_MAX_PER && dMorPerB < D_MOR_PER))
							{
								pRST->left = rcMaxB.left;
								pRST->right = rcMaxB.right;
								pRST->top = rcMaxB.top;
								pRST->bottom = rcMaxB.bottom;
								memcpy(ImgPoly.data, ImgTempB.data, sizeof(UCHAR) * nW * nH);
								dBinCNT = nBinCnt;
								return m_eINSP_PAD_PAT_OUT_FIND_FIND_OK;
							}
						}
					}
					if (dPerP > dRST_PER_TEMP)
					{
						cv::bitwise_xor(ImgTemp, ImgTempA, ImgTempXOR);
						int nMorTemp = cv::countNonZero(ImgTempXOR);
						if (dRST_MAX_MOR > nMorTemp)
						{
							if ((nPolyTemp >= rcMaxP.left && rcMaxP.right > nMorTemp) || (dPerP >= D_OUT_PER && rcMaxP.right > nMorTemp))
							{
								rcMaxP.left = nPolyTemp;
								rcMaxP.right = nMorTemp;
								rcMaxP.top = x;
								rcMaxP.bottom = y;
								dMaxPerP = dPerP;
								dMorPerP = (double)rcMaxP.right / (double)nTemp;
								memcpy(ImgTempP.data, ImgTemp.data, sizeof(UCHAR) * nW * nH);
								nPolyCnt = nTemp;
							}
						}
					}
				}
			}
		}
	}

	bool bP = false;
	bool bB = false;
	bool bP_RST = (dMaxPerP >= D_MAX_PER && dMorPerP < D_MOR_PER);
	bool bB_RST = (dMaxPerB >= D_MAX_PER && dMorPerB < D_MOR_PER);
	if (bB_RST == false)
	{
		if (rcMaxB.right < 100)
			bB_RST = true;
		else
		{
			cv::bitwise_xor(ImgTempB, ImgBin, ImgTempXOR);
			ImgTemp.setTo(0);
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
			cv::erode(ImgTempXOR, ImgTemp, kernel);
			int nERO = cv::countNonZero(ImgTemp);
			if (nERO <= 10)
				bB_RST = true;
		}
	}
	if ((nBinCnt == rcMaxB.left) || (rcMaxB.left > 0 && dMorPerB == 0) || bB_RST)
		bB = true;
	else if (bP_RST)
		bP = true;
	else
	{
		if (dMaxPerP > dRST_PER_TEMP && dMaxPerB > dRST_PER_TEMP)
		{
			if (dMaxPerP > dMaxPerB)
				bP = true;
			else
				bB = true;
		}
		else if (dMaxPerP > dRST_PER_TEMP)
			bP = true;
		else if (dMaxPerB > dRST_PER_TEMP)
			bB = true;
	}

	if (bB)
	{
		pRST->SetData(rcMaxB);
		memcpy(ImgPoly.data, ImgTempB.data, sizeof(UCHAR) * nW * nH);
		dBinCNT = nBinCnt;
		if (bB_RST)
			return m_eINSP_PAD_PAT_OUT_FIND_BIN_OK;
		else
			return m_eINSP_PAD_PAT_OUT_FIND_BIN;
	}
	if (bP)
	{
		pRST->SetData(rcMaxP);
		memcpy(ImgPoly.data, ImgTempP.data, sizeof(UCHAR) * nW * nH);
		dBinCNT = nPolyCnt;
		if (bP_RST)
			return m_eINSP_PAD_PAT_OUT_FIND_POLY_OK;
		else
			return m_eINSP_PAD_PAT_OUT_FIND_POLY;
	}

	return m_eINSP_PAD_PAT_OUT_FIND_NOT;
}
bool PAD_PAT::POLY_IMG_CHECK(cv::Mat ImgPoly)
{
	int nPoly = cv::countNonZero(ImgPoly);
	if (nPoly < 3)
		return false;

	int nW = ImgPoly.cols;
	int nH = ImgPoly.rows;
	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(ImgPoly, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int nTotal = allcontour.size();
	if (nTotal == 0)
		return false;
	else if (nTotal == 1)
		return true;

	int nMAXWID = -1;
	int nMAXW = -1;
	int nMAXW_H = -1;
	int nMAXW_A = -1;

	int nMAXHID = -1;
	int nMAXH = -1;
	int nMAXH_W = -1;
	int nMAXH_A = -1;

	int nMAX_XYID = -1;
	int nMAX_XY = nW * nH;
	int nMAX_XY_W = -1;
	int nMAX_XY_H = -1;
	int nMAX_XY_A = -1;
	double dCX = nW / 2.0;
	double dCY = nH / 2.0;
	for (int n = 0; n < nTotal; n++)
	{
		cv::Rect roiM = cv::boundingRect(allcontour[n]);
		int nArea = cv::contourArea(allcontour[n]);
		if (roiM.width <= PAT_INSP_MIN_SIZE || roiM.height <= PAT_INSP_MIN_SIZE)
			continue;

		if (nTotal > 1)
		{
			int nOUT = 0;
			if (roiM.x == 0)
				nOUT++;
			if (roiM.y == 0)
				nOUT++;
			if (roiM.br().x == nW)
				nOUT++;
			if (roiM.br().y == nH)
				nOUT++;
			if (nOUT >= 3)
			{
				double dPER = (double)nArea / (double)(roiM.width * roiM.height);
				if (0.05 > dPER)
					continue;
			}
		}

		double dROI_CX = roiM.x + (roiM.width / 2.0);
		double dROI_CY = roiM.y + (roiM.height / 2.0);
		double dDist = sqrt(pow(dCX - dROI_CX, 2.0) + pow(dCY - dROI_CY, 2.0));
		if (nMAX_XY > dDist)
		{
			nMAX_XYID = n;
			nMAX_XY = dDist;
			nMAX_XY_W = roiM.width;
			nMAX_XY_H = roiM.height;
			nMAX_XY_A = nArea;
		}

		if (roiM.width > nMAXW)
		{
			nMAXW = roiM.width;
			nMAXW_H = roiM.height;
			nMAXW_A = nArea;
			nMAXWID = n;
		}
		if (roiM.height > nMAXH)
		{
			nMAXH = roiM.height;
			nMAXH_W = roiM.width;
			nMAXH_A = nArea;
			nMAXHID = n;
		}
		int nGAPW = abs(roiM.width - nW);
		int nGAPH = abs(roiM.height - nH);
		if (nGAPW < 1 && nGAPH < 1)
		{
			nMAXW = roiM.width;
			nMAXH = roiM.height;
			nMAXWID = n;
			nMAXHID = n;
			break;
		}
	}
	if (nMAXW <= 0 || nMAXH <= 0)
		return false;

	int nID = -1;
	if (nMAXWID == nMAXHID)
		nID = nMAXWID;
	else
	{
		int nGAPW = abs(nMAXW - nW);
		int nGAPH = abs(nMAXH - nH);
		if (nGAPW < 1 && nGAPH < 1)
		{
			if (nMAXW_A > nMAXH_A)
				nID = nMAXWID;
			else
				nID = nMAXHID;
		}
		else if (nMAX_XYID == nMAXWID)
			nID = nMAXWID;
		else if (nMAX_XYID == nMAXHID)
			nID = nMAXHID;
		else if (nGAPW < 1)
			nID = nMAXWID;
		else if (nGAPH < 1)
			nID = nMAXHID;
		else
		{
			if (nMAXW_A > nMAXH_A)
				nID = nMAXWID;
			else
				nID = nMAXHID;
		}
	}
	if (nID < 0)
		return false;

	cv::Mat contourImage(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::drawContours(contourImage, allcontour, nID, cv::Scalar(255), cv::FILLED);
	cv::bitwise_and(ImgPoly, contourImage, ImgPoly);
	return true;
}

#pragma region OLD
BOOL PAD_PAT::PAT_Thickness_Ver2(WndAlgoImg &sWndAlgoImg, AlgoVolume* pInspAlgoVolume, cv::Mat ImgPoly, cv::Mat ImgPolyORG, cv::Mat ImgBin, RstAlgoVolume* sRstAlgo, cv::Mat imgMix, UCHAR* pUcArrDstImg, bool bX, int nTYPE_P)
{
	BOOL bResult = TRUE;
	int nLine = __LINE__;
	try
	{
		DWORD st = GetTickCount();
		if (sRstAlgo == nullptr || sRstAlgo == NULL)
			return FALSE;
		if (pInspAlgoVolume->UseData(m_eVolumeData_PAD_PAT_Check) == false)
			return bResult;

		float fUM = 1000.0f;
		float fPAT_Min = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_TH_D_MIN);
		float fPAT_Max = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_TH_D_MAX);
		if (fPAT_Min <= 0 && fPAT_Max <= 0)
			return bResult;

		float fTH_Min = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_TH_Min) / fUM;
		float fTH_Max = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_TH_Max) / fUM;
		if (fTH_Min <= 0 && fTH_Max <= 0)
			return bResult;

		nLine = __LINE__;
		UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nW = sWndAlgoImg.m_nWidth;
		int nH = sWndAlgoImg.m_nHeight;
		if (pUcArrDstImg == NULL || pucImgSrc == NULL || pfImgSrc == NULL || nW <= 0 || nH <= 0)
			return bResult;

		nLine = __LINE__;
		std::vector<std::vector<cv::Point>> arrPoly;
		std::vector<std::vector<cv::Point>> arrBin;
		std::vector<cv::Vec4i> hierarchy;
		const int FIND_POLY = 3;
		int nMIN_MAX_FIND = 0;
		int nMIN = 0;
		int nMAX = 0;
#if _DEBUG
		cv::Mat imgSrc2D(nH, nW, CV_8UC1, pucImgSrc);
		cv::Mat imgSrc3D(nH, nW, CV_32FC1, pfImgSrc);
#endif
		cv::Mat imgPOLY_BIN = ImgPoly.clone();
		cv::Mat imgORG(nH, nW, CV_8UC1, pUcArrDstImg);
		cv::Mat imgBIN_BIN(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgBIN_ORG(nH, nW, CV_8UC1, cv::Scalar(0));
		if (pInspAlgoVolume->UseData_M(m_ePAT_ALGO_N_DATA_FILL))
		{
			arrBin.clear();
			hierarchy.clear();
			cv::findContours(imgORG, arrBin, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			for (int b = 0; b < arrBin.size(); b++)
				cv::drawContours(imgBIN_ORG, arrBin, b, cv::Scalar(255), cv::FILLED);

			arrBin.clear();
			hierarchy.clear();
			cv::findContours(ImgBin, arrBin, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			for (int b = 0; b < arrBin.size(); b++)
				cv::drawContours(imgBIN_BIN, arrBin, b, cv::Scalar(255), cv::FILLED);
		}
		else
		{
			imgORG.copyTo(imgBIN_ORG);
			ImgBin.copyTo(imgBIN_BIN);
		}
		SaveWorkImg(imgPOLY_BIN.data, nW, nH, _T("VOL_PAD_TH_POLY.bmp"));
		SaveWorkImg(imgBIN_BIN.data, nW, nH, _T("VOL_PAD_TH_BIN.bmp"));

		bool bONE = false;
		bool bONE_FIND = false;
		cv::Rect roiORG_P = cv::boundingRect(ImgPolyORG);
		if (roiORG_P.x >= N_ONE_CHK && roiORG_P.br().x <= nW - N_ONE_CHK &&
			roiORG_P.y >= N_ONE_CHK && roiORG_P.br().y <= nH - N_ONE_CHK)
		{
			bONE = true;
			int nPOL_TEMP = 0;
			bool bX_TEMP = FIND_LINE_XY(imgPOLY_BIN, &nPOL_TEMP);
			float fPol2 = nPOL_TEMP * fUM * m_resolX;
			if (fPAT_Min <= fPol2 && fPAT_Max >= fPol2)
			{
				bONE_FIND = true;
				bX = bX_TEMP;
				CHANGE_ONE_IMG(ImgPoly, ImgPolyORG, imgBIN_BIN, imgBIN_ORG, nPOL_TEMP, bX);
				imgPOLY_BIN = ImgPoly.clone();
			}
		}
		cv::Rect roiP2 = cv::boundingRect(ImgPoly);
		if (N_INSP_OUT > roiP2.width && N_INSP_OUT > roiP2.height)
			return bResult;

		const uchar* polyData = ImgPoly.data;
		const uchar* binData = imgBIN_ORG.data;
		VOL_TH_DATA sDataP;
		VOL_TH_DATA sDataB;
		VOL_TH_DATA sDataT;
		sDataP.SetData(nW, nH);
		sDataB.SetData(nW, nH);
		sDataT.SetData(nW, nH);
		int nExceptArea = 2;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nExceptArea, nExceptArea));
		cv::Mat imgPoly_Temp(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgBin_Temp(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgTempP(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgTempB(nH, nW, CV_8UC1, cv::Scalar(0));
		VOL_TH_RE sRE;
		sRE.SetData(sWndAlgoImg, pInspAlgoVolume);
		for (int nRot = 0; nRot < 2; nRot++)
		{
			if (nTYPE_P <= m_ePAD_PARALLEL_None)
			{
				if (nRot == 0)
					g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_TH_SET] += ((GetTickCount() - st) / 1000.0f);
				else
					g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_TH_1] += ((GetTickCount() - st) / 1000.0f);
			}

			if (nRot > 0)
			{
				sDataP.ClearValue();
				sDataB.ClearValue();
				if (bONE_FIND)
					continue;
			}
			bool bFIND_POS = true;
			if (nRot > 0)
			{
				bX = !bX;
				if (FIND_POS_ROT(imgPOLY_BIN, ImgPoly, bX, fPAT_Min, fPAT_Max, &sDataP))
					bFIND_POS = FIND_POS_ROT(imgBIN_BIN, imgBIN_ORG, bX, fPAT_Min, fPAT_Max, &sDataB);
				else
					continue;
			}
			else
				bFIND_POS = FIND_POS(imgPOLY_BIN, imgBIN_BIN, ImgPoly, imgBIN_ORG, bX, fPAT_Min, fPAT_Max, &sDataP, &sDataB);
			if (bFIND_POS == false)
				continue;

			double dRes = bX ? m_resolX : m_resolY;
			float fPAT_Min_pix = fPAT_Min / fUM / dRes;
			float fPAT_Max_pix = fPAT_Max / fUM / dRes;
			int nBIN_GAP_DIFF = fTH_Max / dRes - 2;
			if (nBIN_GAP_DIFF < 5) nBIN_GAP_DIFF = 5;
			if (sDataP.m_nIDX <= N_POLY_DIFF || sDataB.m_nIDX <= N_POLY_DIFF ||
				sDataP.m_nMAX_SZ == 0)
				continue;

			std::vector<int> vecMAX = sDataP.GetMAX_LIST(fPAT_Min_pix);
			for (int a = 0; a < vecMAX.size(); a++)
			{
				int nMAX_GAP = vecMAX[a];
				if (nMAX_GAP <= 0)
					break;

				imgPoly_Temp.setTo(0);
				sDataP.Set_IMG(imgPOLY_BIN, imgPoly_Temp, nMAX_GAP, N_POLY_DIFF, bX);
				arrPoly.clear();
				hierarchy.clear();
				cv::findContours(imgPoly_Temp, arrPoly, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
				if (arrPoly.size() == 0)
					continue;

				for (int b = 0; b < arrPoly.size(); b++)
				{
					cv::Rect roiP = cv::boundingRect(arrPoly[b]);
					if (roiP.width < fPAT_Min_pix || roiP.height < fPAT_Min_pix)
						continue;
					if (roiP.width >= fPAT_Max_pix && roiP.height >= fPAT_Max_pix)
						continue;

					int nCHECK_POLY_BIN = CHECK_POLY_BIN(imgPoly_Temp, roiP, bX, N_POLY_DIFF);
					if (bONE == false)
					{
						if (nCHECK_POLY_BIN <= 0)
							REMOVE_POLY_NOT_XY(imgPoly_Temp, arrPoly, b, bX, sDataP, N_POLY_DIFF);
					}
				}
				arrPoly.clear();
				hierarchy.clear();
				cv::findContours(imgPoly_Temp, arrPoly, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
				if (arrPoly.size() == 0)
					continue;

				for (int b = 0; b < arrPoly.size(); b++)
				{
#if _DEBUG
					cv::Mat imgDEBUG(nH, nW, CV_8UC1, cv::Scalar(0));
					cv::drawContours(imgDEBUG, arrPoly, b, cv::Scalar(255), cv::FILLED);
					cv::imwrite(std::string("D:\\POLY_IMG.bmp"), imgTempP);
#endif
					cv::Rect roiP = cv::boundingRect(arrPoly[b]);
					bool bRemove = false;
					if (roiP.width < fPAT_Min_pix || roiP.height < fPAT_Min_pix)
						continue;

					if (roiP.width >= fPAT_Max_pix && roiP.height >= fPAT_Max_pix)
						continue;

					int nCHECK_POLY_BIN = CHECK_POLY_BIN(imgPoly_Temp, roiP, bX, N_POLY_DIFF);
					if (bONE == false)
					{
						if (nCHECK_POLY_BIN <= 0)
							continue;
						else if (bX && nCHECK_POLY_BIN < roiP.width - 3)
							continue;
						else if (bX == false && nCHECK_POLY_BIN < roiP.height - 3)
							continue;

						if (nRot == 0)
						{
							if (bX == true && roiP.width > roiP.height ||
								bX == false && roiP.height > roiP.width)
							{
								imgBin_Temp.setTo(0);
								int nPol = 0;
								cv::Rect roiB = sDataB.Set_IMG(imgBIN_ORG, imgBin_Temp, nMAX_GAP, nBIN_GAP_DIFF, bX, roiP, &nPol);

								cv::Mat imgPOLYROI_BIN(nH, nW, CV_8UC1, cv::Scalar(0));
								cv::drawContours(imgPOLYROI_BIN, arrPoly, b, cv::Scalar(255), cv::FILLED);

								cv::bitwise_xor(imgPOLY_BIN, imgPOLYROI_BIN, imgPOLY_BIN);
								cv::bitwise_xor(imgBIN_BIN, imgBin_Temp, imgBIN_BIN);
								nMIN_MAX_FIND++;
								continue;
							}
						}
					}

					int nPOLY_C = bX ? roiP.x + roiP.width / 2 : roiP.y + roiP.height / 2;
					int nSIZE = bX ? roiP.width : roiP.height;
					double dMinPER = 0.8;
					int dSIZE_DIFF = abs(nMAX_GAP - nSIZE);
					if (bONE == false && dSIZE_DIFF > (N_BIN_DIFF * 2))
					{
						bool bPAT_TH_LINE = false;
						if (bX)
						{
							double dROI_P_PER = roiP.height / roiP.width;
							if (roiP.height > roiP.width && roiP.height > 100 && dROI_P_PER > 2)
								bPAT_TH_LINE = true;
						}
						else
						{
							double dROI_P_PER = roiP.width / roiP.height;
							if (roiP.width > roiP.height && roiP.width > 100 && dROI_P_PER > 2)
								bPAT_TH_LINE = true;
						}
						if (bPAT_TH_LINE && dSIZE_DIFF <= (N_BIN_DIFF * 10))
						{
							double dLINE_A = PAT_TH_LINE(imgPoly_Temp, arrPoly, b, sDataT);
							if (abs(dLINE_A) > 0.1)
								continue;
							dMinPER = 0.6;
						}
						else
							continue;
					}

					imgTempP.setTo(0);
					DRAW_Contour(arrPoly, b, imgTempP, imgPOLY_BIN);
					roiP = cv::boundingRect(imgTempP);
					double dPcnt = cv::countNonZero(imgTempP);
					double dPcntPer = dPcnt / (roiP.width * roiP.height);
					if (bONE == false && dPcntPer < dMinPER)
					{
						if (bX)
							dPcntPer = dPcnt / (nCHECK_POLY_BIN * roiP.height);
						else
							dPcntPer = dPcnt / (roiP.width * nCHECK_POLY_BIN);
						if (dPcntPer < dMinPER)
							continue;
					}
					if (roiP.x == 0 || roiP.y == 0 || roiP.br().x == nW || roiP.br().y == nH)
					{
						int nLINE_CNT = 0;
						int nLINE_MIN = nMAX_GAP * 0.5;
						int nLINE_OUT = 6;
						int nOUT_SIZE = N_INSP_OUT;
						if (bX)
						{
							for (int c = 0; c < 2; c++)
							{
								for (int d = 1; d <= 5; d++)
								{
									int nY = (c == 0) ? roiP.y - d : roiP.br().y + d;
									if (nY < 0 || nY >= nH)
										break;

									int nLINE = 0;
									for (int x = roiP.x; x < roiP.br().x; x++)
									{
										int nIDX = (nY * nW) + x;
										if (imgPOLY_BIN.data[nIDX] == 255)
											nLINE++;
									}
									if (nLINE >= nLINE_MIN)
										nLINE_CNT++;
								}
							}
							if (roiP.width <= N_SMALL_L && roiP.height <= nOUT_SIZE)
							{
								double dPER_P = (double)roiP.width / (double)roiP.height;
								if (dPER_P > D_OUT_LINE_PER)
									continue;
							}
						}
						else
						{
							for (int c = 0; c < 2; c++)
							{
								for (int d = 1; d <= 5; d++)
								{
									int nX = (c == 0) ? roiP.x - d : roiP.br().x + d;
									if (nX < 0 || nX >= nW)
										break;

									int nLINE = 0;
									for (int y = roiP.y; y < roiP.br().y; y++)
									{
										int nIDX = (y * nW) + nX;
										if (imgPOLY_BIN.data[nIDX] == 255)
											nLINE++;
									}
									if (nLINE >= nLINE_MIN)
										nLINE_CNT++;
								}
							}
							if (roiP.width <= nOUT_SIZE && roiP.height <= N_SMALL_L)
							{
								double dPER_P = (double)roiP.height / (double)roiP.width;
								if (dPER_P > D_OUT_LINE_PER)
									continue;
							}
						}
						if (roiP.width > nOUT_SIZE && roiP.height > nOUT_SIZE)
						{
							double dPER_W = (double)roiP.width / (double)roiP.height;
							double dPER_H = (double)roiP.height / (double)roiP.width;
							if (bX)
							{
								if (dPER_H > D_OUT_LINE_PER2)
									nLINE_CNT = 0;
							}
							else
							{
								if (dPER_W > D_OUT_LINE_PER2)
									nLINE_CNT = 0;
							}
						}
						if (nLINE_CNT >= nLINE_OUT)
							continue;
					}

					imgBin_Temp.setTo(0);
					int nPol2 = 0;
					cv::Rect roiB = sDataB.Set_IMG(imgBIN_ORG, imgBin_Temp, nMAX_GAP, nBIN_GAP_DIFF, bX, roiP, &nPol2);
					if (roiB.width <= N_BIN_DIFF || roiB.height <= N_BIN_DIFF)
						continue;
					if (roiB.width >= nW && roiB.height <= N_INSP_LINE_OUT)
					{
						if (bX)
							continue;
					}
					if (roiB.height >= nH && roiB.width <= N_INSP_LINE_OUT)
					{
						if (bX == false)
							continue;
					}

					if (CHK_BIN_POLY_XOR(imgBin_Temp, imgPoly_Temp, kernel, roiB, roiP) == false)
					{
						cv::bitwise_xor(imgPOLY_BIN, imgTempP, imgPOLY_BIN);
						cv::bitwise_xor(imgBIN_BIN, imgBin_Temp, imgBIN_BIN);
						nMIN_MAX_FIND++;
						continue;
					}

					RECT_F roiMAX;
					RECT_F roiMIN;
					roiMAX.Init();
					roiMIN.Init();
					float fPol = nMAX_GAP * dRes * fUM;
					int nRET_PAT_TH_L = PAT_TH_L(sRE, fPol, bX, imgBIN_BIN, imgBin_Temp, roiB, &roiMAX, &roiMIN);
					bool bRET_MISSING = (nRET_PAT_TH_L & m_eVOL_TH_FAIL_MISSING) == m_eVOL_TH_FAIL_MISSING;
					bool bRET_MIN = (nRET_PAT_TH_L & m_eVOL_TH_FAIL_MIN) == m_eVOL_TH_FAIL_MIN;
					bool bRET_MAX = (nRET_PAT_TH_L & m_eVOL_TH_FAIL_MAX) == m_eVOL_TH_FAIL_MAX;
					float fMAX_GAP = roiMAX.CALC_DIST(m_resolX, m_resolY) * fUM;
					float fMIN_GAP = roiMIN.CALC_DIST(m_resolX, m_resolY) * fUM;
					if ((bRET_MIN == false && fPol > fMIN_GAP) || bRET_MISSING)
					{
						float fGap2 = (bRET_MISSING) ? fPol : fPol - fMIN_GAP;
						if (bRET_MISSING == false && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN])
						{
							float fPolTemp = CHECK_LINE(ImgPoly, roiMIN, bX);
							if (fPolTemp > fMIN_GAP && fPAT_Min <= fPolTemp && fPAT_Max >= fPolTemp)
							{
								float fGap3 = fPolTemp - fMIN_GAP;
								if (fGap2 > fGap3)
									fGap2 = fGap3;
							}
						}

						if (fGap2 > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN])
						{
							sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN] = fGap2;
							sRstAlgo->m_rcRect_TH[0].left = roiMIN.left;
							sRstAlgo->m_rcRect_TH[0].top = roiMIN.top;
							sRstAlgo->m_rcRect_TH[0].right = roiMIN.right;
							sRstAlgo->m_rcRect_TH[0].bottom = roiMIN.bottom;
							nMIN++;
						}
					}
					if (bRET_MAX == false && fPol < fMAX_GAP)
					{
						float fGap2 = fMAX_GAP - fPol;
						if (fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX])
						{
							float fPolTemp = CHECK_LINE(ImgPoly, roiMAX, bX);
							if (fMAX_GAP > fPolTemp && fPAT_Min <= fPolTemp && fPAT_Max >= fPolTemp)
							{
								float fGap3 = fMAX_GAP - fPolTemp;
								if (fGap2 > fGap3)
									fGap2 = fGap3;
							}
						}

						if (fGap2 > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX])
						{
							sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX] = fGap2;
							sRstAlgo->m_rcRect_TH[1].left = roiMAX.left;
							sRstAlgo->m_rcRect_TH[1].top = roiMAX.top;
							sRstAlgo->m_rcRect_TH[1].right = roiMAX.right;
							sRstAlgo->m_rcRect_TH[1].bottom = roiMAX.bottom;
							nMAX++;
						}
					}

					cv::bitwise_and(imgPOLY_BIN, imgTempP, imgTempP);
					cv::bitwise_and(imgBin_Temp, imgBIN_BIN, imgBin_Temp);

					cv::bitwise_xor(imgPOLY_BIN, imgTempP, imgPOLY_BIN);
					cv::bitwise_xor(imgBIN_BIN, imgBin_Temp, imgBIN_BIN);
					nMIN_MAX_FIND++;
				}
			}
		}
		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_TH_2] += ((GetTickCount() - st) / 1000.0f);
		nLine = __LINE__;

		END_LINE_REMOVE(ImgPoly, imgPOLY_BIN, imgBIN_BIN);

		if (nMIN_MAX_FIND <= 1)
			POLY_ROI_LINE(ImgPoly, ImgPolyORG, imgPOLY_BIN, imgBIN_BIN, imgBIN_ORG, kernel, fPAT_Min, fPAT_Max, sDataP, sRE, nMIN, nMAX, sRstAlgo);

		arrPoly.clear();
		hierarchy.clear();
		cv::findContours(imgPOLY_BIN, arrPoly, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		arrBin.clear();
		hierarchy.clear();
		cv::findContours(imgBIN_BIN, arrBin, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		for (int a = 0; a < arrPoly.size(); a++)
		{
			PAT_TH_AB(a, ImgPoly, ImgPolyORG, imgPOLY_BIN, imgBIN_BIN, imgBIN_ORG, kernel,
				arrPoly, arrBin, fPAT_Min, fPAT_Max, sDataP, sRE, nMIN, nMAX, sRstAlgo);
		}
		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_TH_AB] += ((GetTickCount() - st) / 1000.0f);
		nLine = __LINE__;
		bResult = TRUE;
		if (fTH_Min > 0)
		{
			sRstAlgo->m_nPadPatternCheck |= m_eVOL_PAT_CHK_TH_IN;
			if (nMIN > 0 && sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN] > fTH_Min * fUM)
			{
				bResult = FALSE;
				sRstAlgo->m_nOKPadPatternCheck |= m_eVOL_PAT_CHK_TH_IN;
			}
		}
		if (fTH_Max > 0)
		{
			sRstAlgo->m_nPadPatternCheck |= m_eVOL_PAT_CHK_TH_OUT;
			if (nMAX > 0 && sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX] > fTH_Max * fUM)
			{
				bResult = FALSE;
				sRstAlgo->m_nOKPadPatternCheck |= m_eVOL_PAT_CHK_TH_OUT;
			}
		}
		sDataP.ClearData();
		sDataB.ClearData();
		sDataT.ClearData();
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("PAD_PAT::PAT_Thickness_Ver2(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
	return bResult;
}
void PAD_PAT::DRAW_Contour(std::vector<std::vector<cv::Point>> arr, int nIDX, cv::Mat img, cv::Mat imgSRC)
{
	cv::drawContours(img, arr, nIDX, cv::Scalar(255), cv::FILLED);
	cv::bitwise_and(img, imgSRC, img);
}
int PAD_PAT::FIND_TH_BIN(VOL_TH_RE sRE, cv::Rect rectP, cv::Rect rectB, cv::Mat img, cv::Mat imgSrc, bool bX, double dAValue, double dBValue, RECT_F* pMAX, RECT_F* pMIN, RECT_F* pMAX_F, RECT_F* pMIN_F, float fPol_ALL, bool bCheck)
{
	int nRet = 0;
	int nW = img.cols;
	int nH = img.rows;
	const uchar* puc = imgSrc.data;
	bool bXR = bX;
	int nA_T = bX ? nW : nH;
	int nB_T = bX ? nH : nW;
	int nB_S = bX ? rectB.y : rectB.x;
	int nB_E = bX ? rectB.y + rectB.height : rectB.x + rectB.width;
	int nMin = -1;
	float fMin_L = -1;
	float fMin_T = -1;
	float fMin_R = -1;
	float fMin_B = -1;
	int nMax = -1;
	float fMax_L = -1;
	float fMax_T = -1;
	float fMax_R = -1;
	float fMax_B = -1;

	float fMin_L_F = -1;
	float fMin_T_F = -1;
	float fMin_R_F = -1;
	float fMin_B_F = -1;

	float fMax_L_F = -1;
	float fMax_T_F = -1;
	float fMax_R_F = -1;
	float fMax_B_F = -1;

	int nFind_Gap = 1;
	int nFIND_MIN_MAX = 1;
	int nA_MIN = bXR ? rectB.x : rectB.y;
	int nA_MAX = bXR ? rectB.br().x : rectB.br().y;
	int nB_MIN = bXR ? rectB.y : rectB.x;
	int nB_MAX = bXR ? rectB.br().y : rectB.br().x;
	nA_MIN += nFIND_MIN_MAX;
	nA_MAX -= nFIND_MIN_MAX;
	nB_MIN += nFIND_MIN_MAX;
	nB_MAX -= nFIND_MIN_MAX;
#if _DEBUG
	cv::Mat imgTemp = imgSrc.clone();
#endif
	int nGAP_END = 5;
	int nGAP_END_X = rectB.width * 0.05;
	int nGAP_END_Y = rectB.height * 0.05;
	if (nGAP_END_X < 2) nGAP_END_X = 2;
	if (nGAP_END_Y < 2) nGAP_END_Y = 2;
	nB_S -= nGAP_END;
	nB_E += nGAP_END;
	for (int b = nB_S; b <= nB_E; b++)
	{
		if (b < 0 || b >= nB_T)
			continue;

		double dAVGA = bX ? (dAValue * b) + dBValue : b;
		double dAVGB = bX ? b : (dAValue * b) + dBValue;
		if (bCheck)
		{
			if (bX)
			{
				if (dAVGA < 0 || dAVGA >= nW)
					continue;
				if (dAVGB < 0 || dAVGB >= nH)
					continue;
			}
			else
			{
				if (dAVGA < 0 || dAVGA >= nH)
					continue;
				if (dAVGB < 0 || dAVGB >= nW)
					continue;
			}
		}

		double dReA = -1.0 / dAValue;
		double dReB = bX ? dAVGB - (dReA * dAVGA) : dAVGA - (dReA * dAVGB);

		double dS_A = dAVGA;
		double dS_B = dAVGB;
		double dE_A = dAVGA;
		double dE_B = dAVGB;

		double dS_A_FAIL = -1;
		double dS_B_FAIL = -1;
		double dE_A_FAIL = -1;
		double dE_B_FAIL = -1;
		int nFind = 0;
		int nADD = abs(dBValue) > abs(dReB) ? abs(dReB) : abs(dBValue);
		int nA_S = 0;
		int nA_E = bXR ? nA_T + nADD : nB_T + nADD;
		nA_S = 0;
		for (int a = nA_S; a < nA_E; a++)
		{
			double dX = bXR ? a : a * dReA + dReB;
			double dY = bXR ? a * dReA + dReB : a;
			if (dX >= nW && dY >= nH)
				break;
			if (dX < 0 || dX >= nW)
				continue;
			if (dY < 0 || dY >= nH)
				continue;

			if (rectB.x - nGAP_END > dX || rectB.br().x + nGAP_END < dX ||
				rectB.y - nGAP_END > dY || rectB.br().y + nGAP_END < dY)
				continue;

			bool bWhite = false;
			for (int y = 0; y <= 2; y++)
			{
				int nY = (int)dY;
				if (y == 1)
					nY = (int)(dY - 0.5);
				else if (y == 2)
					nY = (int)(dY + 0.5);
				for (int x = 0; x <= 2; x++)
				{
					int nX = (int)dX;
					if (x == 1)
						nX = (int)(dX - 0.5);
					else if (x == 2)
						nX = (int)(dX + 0.5);

					int nIndex = (nY * sRE.m_nW) + nX;
					if (nIndex < 0 || nX < 0 || nY < 0)
						continue;
					if (nIndex >= sRE.m_nW * sRE.m_nH || nX >= sRE.m_nW || nY >= sRE.m_nH)
						continue;
					if (puc[nIndex] != 255)
						continue;
					bWhite = true;
#if _DEBUG
					imgTemp.data[nIndex] = 126;
#endif
					break;
				}
				if (bWhite) break;
			}

			if (bWhite)
			{
				if (nFind == 0)
				{
					dS_A = bXR ? dX : dY;
					dS_B = bXR ? dY : dX;
					if (a > 0)
					{
						double dPre = a - 1;
						dE_A_FAIL = bXR ? dPre : dPre * dReA + dReB;
						dE_B_FAIL = bXR ? dPre * dReA + dReB : dPre;
					}
				}
				dE_A = bXR ? dX : dY;
				dE_B = bXR ? dY : dX;
				nFind++;
			}
			else if (nFind > 0)
			{
				dE_A_FAIL = bXR ? dX : dY;
				dE_B_FAIL = bXR ? dY : dX;
				break;
			}
		}
		if (nFind <= nFind_Gap)
			continue;

		double dA_SZ = abs(dE_A - dS_A);
		double dB_SZ = abs(dE_B - dS_B);
		if (dA_SZ <= nFind_Gap || dB_SZ <= nFind_Gap)
			continue;

		int nWhite = sqrt(pow(dA_SZ, 2.0f) + pow(dB_SZ, 2.0f));
		if (fPol_ALL > PAT_INSP_MIN_SIZE)
		{
			float fMIN_PER = 0.3;
			float fMAX_PER = 2.0;
			float fPol_ALL_MIN = fPol_ALL * fMIN_PER;
			float fPol_ALL_MAX = fPol_ALL * fMAX_PER;
			if (nWhite <= fPol_ALL_MIN || nWhite >= fPol_ALL_MAX)
				continue;
		}

		if (bXR)
		{
			if (rectB.x <= dS_A && rectB.br().x >= dS_A &&
				rectB.x <= dE_A && rectB.br().x >= dE_A)
			{
				if (dS_A <= nGAP_END || dS_B <= nGAP_END || dE_A <= nGAP_END || dE_B <= nGAP_END)
					continue;
				else if (dS_A >= nW - nGAP_END || dS_B >= nH - nGAP_END || dE_A >= nW - nGAP_END || dE_B >= nH - nGAP_END)
					continue;
				else if (rectB.y + nGAP_END_Y > dS_B || rectB.y + nGAP_END_Y > dE_B)
					continue;
				else if (rectB.br().y - nGAP_END_Y < dS_B || rectB.br().y - nGAP_END_Y < dE_B)
					continue;
				else if (rectP.y + nGAP_END_Y > dS_B || rectP.y + nGAP_END_Y > dE_B)
					continue;
				else if (rectP.br().y - nGAP_END_Y < dS_B || rectP.br().y - nGAP_END_Y < dE_B)
					continue;
			}
			else
				continue;
		}
		else
		{
			if (rectB.y <= dS_A && rectB.br().y >= dS_A &&
				rectB.y <= dE_A && rectB.br().y >= dE_A)
			{
				if (dS_A <= nGAP_END || dS_B <= nGAP_END || dE_A <= nGAP_END || dE_B <= nGAP_END)
					continue;
				else if (dS_A >= nH - nGAP_END || dS_B >= nW - nGAP_END || dE_A >= nH - nGAP_END || dE_B >= nW - nGAP_END)
					continue;
				else if (rectB.x + nGAP_END_X > dS_B || rectB.x + nGAP_END_X > dE_B)
					continue;
				else if (rectB.br().x - nGAP_END_X < dS_B || rectB.br().x - nGAP_END_X < dE_B)
					continue;
				else if (rectP.x + nGAP_END_X > dS_B || rectP.x + nGAP_END_X > dE_B)
					continue;
				else if (rectP.br().x - nGAP_END_X < dS_B || rectP.br().x - nGAP_END_X < dE_B)
					continue;
			}
			else
				continue;
		}

		if (nMin == -1 || nMin > nWhite)
		{
			nMin = nWhite;
			fMin_L = bXR ? dS_A : dS_B;
			fMin_T = bXR ? dS_B : dS_A;
			fMin_R = bXR ? dE_A : dE_B;
			fMin_B = bXR ? dE_B : dE_A;

			fMin_L_F = bXR ? dS_A_FAIL : dS_B_FAIL;
			fMin_T_F = bXR ? dS_B_FAIL : dS_A_FAIL;
			fMin_R_F = bXR ? dE_A_FAIL : dE_B_FAIL;
			fMin_B_F = bXR ? dE_B_FAIL : dE_A_FAIL;
		}
		if (nMax == -1 || nMax < nWhite)
		{
			nMax = nWhite;
			fMax_L = bXR ? dS_A : dS_B;
			fMax_T = bXR ? dS_B : dS_A;
			fMax_R = bXR ? dE_A : dE_B;
			fMax_B = bXR ? dE_B : dE_A;

			fMax_L_F = bXR ? dS_A_FAIL : dS_B_FAIL;
			fMax_T_F = bXR ? dS_B_FAIL : dS_A_FAIL;
			fMax_R_F = bXR ? dE_A_FAIL : dE_B_FAIL;
			fMax_B_F = bXR ? dE_B_FAIL : dE_A_FAIL;
		}
	}
	if (nMin == -1)
		nRet |= m_eVOL_TH_FAIL_MIN;
	if (nMax == -1)
		nRet |= m_eVOL_TH_FAIL_MAX;
	pMAX->SetROI(fMax_L, fMax_R, fMax_T, fMax_B);
	pMIN->SetROI(fMin_L, fMin_R, fMin_T, fMin_B);

	if (nMax > 0)
		pMAX_F->SetROI(fMax_L_F, fMax_R_F, fMax_T_F, fMax_B_F);
	if (nMin > 1)
		pMIN_F->SetROI(fMin_L_F, fMin_R_F, fMin_T_F, fMin_B_F);
	return nRet;
}
float PAD_PAT::FIND_TH_POLY(cv::Rect rectP, cv::Mat img, bool bX, double* dAValue, double* dBValue, VOL_TH_DATA sData)
{
	int nW = img.cols;
	int nH = img.rows;
	const uchar* puc = img.data;
	int nA_T = bX ? nH : nW;
	int nA_S = bX ? rectP.y : rectP.x;
	int nA_E = bX ? rectP.y + rectP.height : rectP.x + rectP.width;
	int nB_T = bX ? nW : nH;
	int nB_S = bX ? rectP.x : rectP.y;
	int nB_E = bX ? rectP.x + rectP.width : rectP.y + rectP.height;
	int nGAP = 4;
	nA_S += nGAP;
	nB_S -= nGAP;
	nA_E -= nGAP;
	nB_E += nGAP;
	sData.ClearValueC();
	for (int a = nA_S; a <= nA_E; a++)
	{
		if (a < 0 || a >= nA_T)
			continue;
		int nS = -1;
		int nE = -1;
		for (int b = nB_S; b <= nB_E; b++)
		{
			if (b < 0 || b >= nB_T)
				continue;
			int nIndex = bX ? (a * nW) + b : (b * nW) + a;
			if (puc[nIndex] == 255)
			{
				nS = b;
				break;
			}
		}
		if (nS < 0) continue;
		for (int b = nB_E; b >= nB_S; b--)
		{
			if (b < 0 || b >= nB_T)
				continue;
			int nIndex = bX ? (a * nW) + b : (b * nW) + a;
			if (puc[nIndex] == 255)
			{
				nE = b;
				break;
			}
		}
		if (nE < 0) continue;
		double dB_C = (nS + nE) / 2.0f;
		double dDist = nE - nS;
		if (bX)
			sData.AddData(dB_C, a, dDist);
		else
			sData.AddData(a, dB_C, dDist);
	}
	float fDist = 0;
	float fUM = 1000.0f;
	double dAVGB = sData.GetAVG(bX);
	double dAVGD = sData.GetAVG(bX, true);
	if (sData.m_nC <= 0 || dAVGB <= 0 || dAVGD <= 0)
	{
		*dAValue = 0;
		*dBValue = 0;
		return fDist;
	}

	double dAngle = GetGradient(sData.m_dArrCX, sData.m_dArrCY, sData.m_nC, *dAValue, *dBValue, !bX);
	double dAVGA = (dAVGB - *dBValue) / *dAValue;
	double dAvgMax = sData.CheckValue(bX);
	sData.DEBUG_POLY(img, *dAValue, *dBValue, dAVGA, dAVGB, bX);
	if (dAvgMax <= N_BIN_DIFF * 2)
	{
		*dAValue = 0;
		*dBValue = 0;
		return fDist;
	}
	if (*dAValue == 0)
	{
		*dAValue = 0;
		*dBValue = 0;
		fDist = bX ? rectP.width * m_resolX * fUM : rectP.height * m_resolY * fUM;
	}
	else if (abs(*dAValue) <= 0.2)
	{
		*dAValue = 0;
		*dBValue = 0;
		fDist = bX ? dAVGD * m_resolX * fUM : dAVGD * m_resolY * fUM;
		if (fDist <= 0)
			fDist = bX ? rectP.width * m_resolX * fUM : rectP.height * m_resolY * fUM;
	}
	else
	{
#if _DEBUG
		cv::Mat imgD = img.clone();
#endif
		double dS_X = bX ? dAVGB : dAVGA;
		double dS_Y = bX ? dAVGA : dAVGB;
		RECT_F rcROI;
		rcROI.left = dS_X;
		rcROI.right = dS_X;
		rcROI.top = dS_Y;
		rcROI.bottom = dS_Y;
		fDist = CHECK_LINE(img, rcROI, bX, *dAValue, *dBValue, 20, 400);
		if (fDist <= 0)
		{
			*dAValue = 0;
			*dBValue = 0;
			return -100;
		}
	}
	return fDist;
}
int PAD_PAT::FIND_TH(VOL_TH_RE sRE, cv::Rect roi, cv::Mat img, cv::Mat imgSrc, bool bX, RECT_F* pMAX, RECT_F* pMIN)
{
	int nRet = 0;
	int nW = img.cols;
	int nROIA = img.cols;
	int nROIB = img.rows;
	int nGAP = ((double)roi.height * 0.008 + 0.5);
	if (nGAP < N_BIN_DIFF) nGAP = N_BIN_DIFF;
	int nROIA_S = roi.x;
	int nROIA_E = roi.x + roi.width - 1;
	int nROIB_S = roi.y + nGAP;
	int nROIB_E = roi.y + roi.height - 1 - nGAP;
	if (bX == false)
	{
		nROIA = img.rows;
		nROIB = img.cols;
		nGAP = ((double)roi.width * 0.008 + 0.5);
		if (nGAP < N_BIN_DIFF) nGAP = N_BIN_DIFF;
		nROIA_S = roi.y;
		nROIA_E = roi.y + roi.height - 1;
		nROIB_S = roi.x + nGAP;
		nROIB_E = roi.x + roi.width - 1 - nGAP;
	}
	const uchar* puc = imgSrc.data;
	int nMin = -1;
	int nMinB = -1;
	int nMinS = -1;
	int nMinE = -1;
	int nMax = -1;
	int nMaxB = -1;
	int nMaxS = -1;
	int nMaxE = -1;
	for (int nB = nROIB_S; nB < nROIB_E; nB++)
	{
		if (nB < 0 || nB >= nROIB)
			continue;

		int nWhite = 0;
		int nWhiteS = -1;
		int nWhiteE = -1;
		int nWhiteS_PRE = -1;
		int nWhiteE_PRE = -1;
		for (int nA = nROIA_S; nA < nROIA_E; nA++)
		{
			if (nA < 0 || nA >= nROIA)
				continue;
			int nIndex = bX ? (nB * nW) + nA : (nA * nW) + nB;
			if (puc[nIndex] == 255)
			{
				nWhite++;
				if (nWhiteS_PRE == -1) nWhiteS_PRE = nA;
				if (nWhiteE_PRE < nA) nWhiteE_PRE = nA;
			}
			else
			{
				if (nWhiteS_PRE < 0 || nWhiteE_PRE < 0)
					continue;
				if (nWhiteS == -1 || nWhiteE == -1)
				{
					nWhiteS = nWhiteS_PRE;
					nWhiteE = nWhiteE_PRE;
				}
				else
				{
					int nW_R = nWhiteE - nWhiteS;
					int nW_S = nWhiteE_PRE - nWhiteS_PRE;
					if (nW_S > nW_R)
					{
						nWhiteS = nWhiteS_PRE;
						nWhiteE = nWhiteE_PRE;
					}
				}
				nWhiteS_PRE = -1;
				nWhiteE_PRE = -1;
			}
		}
		if (nWhiteS_PRE >= 0 && nWhiteE_PRE >= 0)
		{
			if (nWhiteS == -1 || nWhiteE == -1)
			{
				nWhiteS = nWhiteS_PRE;
				nWhiteE = nWhiteE_PRE;
			}
			else
			{
				int nW_R = nWhiteE - nWhiteS;
				int nW_S = nWhiteE_PRE - nWhiteS_PRE;
				if (nW_S > nW_R)
				{
					nWhiteS = nWhiteS_PRE;
					nWhiteE = nWhiteE_PRE;
				}
			}
		}

		bool bMISSING = ((nRet & m_eVOL_TH_FAIL_MISSING) == m_eVOL_TH_FAIL_MISSING);
		if (bMISSING == false && (nMin == -1 || nMin > nWhite || nWhite == 0))
		{
			nMin = nWhite;
			nMinB = nB;
			nMinS = nWhiteS;
			nMinE = nWhiteE;
			if (nWhite == 0)
			{
				nMinS = nROIA_S;
				nMinE = nROIA_E;
				nRet |= m_eVOL_TH_FAIL_MISSING;
			}
		}
		if (nMax == -1 || nMax < nWhite)
		{
			nMax = nWhite;
			nMaxB = nB;
			nMaxS = nWhiteS;
			nMaxE = nWhiteE;
		}
	}
	if (nMin == -1)
		nRet |= m_eVOL_TH_FAIL_MIN;
	if (nMax == -1)
		nRet |= m_eVOL_TH_FAIL_MAX;
	if (bX)
	{
		if (nMax >= 0)
		{
			pMAX->SetROI(nMaxS, nMaxE, nMaxB, nMaxB);
		}
		if (nMin >= 0)
		{
			pMIN->SetROI(nMinS, nMinE, nMinB, nMinB);
		}
	}
	else
	{
		if (nMax >= 0)
		{
			pMAX->SetROI(nMaxB, nMaxB, nMaxS, nMaxE);
		}
		if (nMin >= 0)
		{
			pMIN->SetROI(nMinB, nMinB, nMinS, nMinE);
		}
	}
	return nRet;
}
float PAD_PAT::CHECK_LINE(cv::Mat ImgPoly, RECT_F rcROI, bool bX, double dA_VAL, double dB_VAL, float fPAT_Min, float fPAT_Max, bool bROT)
{
	float fPOL = 0;
	if (dA_VAL == 0)
	{
		fPOL = CHECK_LINE(ImgPoly, rcROI, bX);
		if (fPAT_Min > fPOL || fPAT_Max < fPOL)
			return 0;
		return fPOL;
	}

	int nW = ImgPoly.cols;
	int nH = ImgPoly.rows;
	double dCX = (rcROI.right + rcROI.left) / 2.0;
	double dCY = (rcROI.bottom + rcROI.top) / 2.0;
	const uchar* puc = ImgPoly.data;
	int nA_T = bX ? nW : nH;

	bool bXR = bROT ? !bX : bX;
	double dReA = -1.0 / dA_VAL;
	double dReB = dCY - (dReA * dCX);
	double dS_X = -1;
	double dS_Y = -1;
	double dE_X = -1;
	double dE_Y = -1;

	int nFind = 0;
	int nFind_Gap = 1;
	int nFIND_MIN_MAX = 1;
#if _DEBUG
	cv::Mat imgTemp = ImgPoly.clone();
#endif
	for (int a = 0; a < nA_T; a++)
	{
		double dX = bXR ? a : a * dReA + dReB;
		double dY = bXR ? a * dReA + dReB : a;
		if (dX < 0 || dX >= nW)
			continue;
		if (dY < 0 || dY >= nH)
			continue;

		int nIDX = ((int)dY * nW) + (int)dX;
		if (puc[nIDX] == 255)
		{
#if _DEBUG
			imgTemp.data[nIDX] = 63;
#endif
			if (nFind == 0)
			{
				dS_X = dX;
				dS_Y = dY;
			}
			dE_X = dX;
			dE_Y = dY;
			nFind++;
		}
		else if (nFind > 0)
			break;
	}
	if (nFind <= nFind_Gap)
		return fPOL;

	double dCX_R = (dE_X + dS_X) / 2.0;
	double dCY_R = (dE_Y + dS_Y) / 2.0;
	double dDIST_X = abs(dCX - dCX_R);
	double dDIST_Y = abs(dCY - dCY_R);
	double dDIST = sqrt(pow(dDIST_X, 2.0f) + pow(dDIST_Y, 2.0f));
	if (dDIST > PAT_INSP_MIN_SIZE)
		return fPOL;

	float fUM = 1000.0f;
	double dDistX = abs(dE_X - dS_X + 1);
	double dDistY = abs(dE_Y - dS_Y + 1);
	float fX = dDistX * m_resolX;
	float fY = dDistY * m_resolY;
	fPOL = sqrt(pow(fX, 2.0f) + pow(fY, 2.0f)) * fUM;

	return fPOL;
}
void PAD_PAT::PAT_TH_AB(int nIDX, cv::Mat ImgPoly, cv::Mat ImgPolyORG, cv::Mat imgPOLY_BIN, cv::Mat imgBIN_BIN, cv::Mat imgBIN_ORG, cv::Mat kernel,
	std::vector<std::vector<cv::Point>> arrPoly, std::vector<std::vector<cv::Point>> arrBin, float fPAT_Min, float fPAT_Max,
	VOL_TH_DATA sDataP, VOL_TH_RE sRE, int &nMIN, int &nMAX, RstAlgoVolume* sRstAlgo, bool bFIND_LINE, bool bFIND_LINE_X, double dFIND_LINE_A, double dFIND_LINE_B)
{
	int nOUT_SIZE = N_INSP_OUT;
	cv::Rect rectP = cv::boundingRect(arrPoly[nIDX]);
	if (rectP.width <= nOUT_SIZE || rectP.height <= nOUT_SIZE)
		return;

	int nW = imgPOLY_BIN.cols;
	int nH = imgPOLY_BIN.rows;
	int nOUT_SIZE2 = nOUT_SIZE + PAT_INSP_MIN_SIZE;
	if (rectP.width <= nOUT_SIZE2)
	{
		if (rectP.br().x == nW || rectP.x == 0)
			return;
	}
	if (rectP.height <= nOUT_SIZE2)
	{
		if (rectP.br().y == nH || rectP.y == 0)
			return;
	}

	cv::Mat imgTempP(nH, nW, CV_8UC1, cv::Scalar(0));
	DRAW_Contour(arrPoly, nIDX, imgTempP, imgPOLY_BIN);
	Check_End_Bin(rectP, imgTempP);
	cv::Rect roiP;
	double dW_mm = nW * m_resolX;
	double dH_mm = nH * m_resolY;
	double dW_Min_mm = 1.1; //200pix * 0.0055
	double dH_Min_mm = 1.1; //200pix * 0.0055
	double dVal_A = 0;
	double dVal_B = 0;
	double dPerX = (double)rectP.height / (double)rectP.width;
	bool bX = dPerX > 0.7;
	double dRes = bX ? m_resolY : m_resolX;
	float fPol = 0;
	float fPol2 = 0;
	float fPol_ALL = -1;
	float fUM = 1000.0f;
	int nPOLY_ORG = 0;
	bool bPOLY_ORG_T = false;
	bool bCheckBin = true;
	bool bSMALL = dW_mm <= dW_Min_mm && dH_mm <= dH_Min_mm;
	int nPOLY_AREA = cv::countNonZero(imgTempP);
	double dPOLY_PER = (double)nPOLY_AREA / (double)(rectP.width * rectP.height);
	if (bFIND_LINE == false && ((nMIN == 0 && nMAX == 0) || bSMALL))
	{
		cv::Rect rectPORG = cv::boundingRect(ImgPolyORG);
		bool bX_TEMP = bX;
		fPol2 = FIND_TH_POLY(rectPORG, ImgPolyORG, bX_TEMP, &dVal_A, &dVal_B, sDataP);
		if (fPol2 == -100)
		{
			dVal_A = 0;
			dVal_B = 0;
			bX_TEMP = !bX;
			fPol2 = FIND_TH_POLY(rectPORG, ImgPolyORG, bX_TEMP, &dVal_A, &dVal_B, sDataP);
		}
		bPOLY_ORG_T = true;
		if (nMIN == 0 && nMAX == 0)
			bCheckBin = false;
		if (rectPORG.x > N_GAP_ORG && rectPORG.br().x < nW - N_GAP_ORG &&
			rectPORG.y > N_GAP_ORG && rectPORG.br().y < nH - N_GAP_ORG)
		{
			if (fPol2 > 0)
			{
				nPOLY_ORG = 1;
				bX = bX_TEMP;
				fPol = fPol2;
				ImgPolyORG.copyTo(imgTempP);
				rectP = rectPORG;
				fPol_ALL = fPol / fUM / dRes;
			}
		}
	}
	if (bFIND_LINE && dFIND_LINE_A != 0)
	{
		bX = bFIND_LINE_X;
		dVal_A = dFIND_LINE_A;
		dVal_B = dFIND_LINE_B;
		RECT_F rcROI_CHECK_LINE;
		rcROI_CHECK_LINE.left = rectP.x;
		rcROI_CHECK_LINE.top = rectP.y;
		rcROI_CHECK_LINE.right = rectP.br().x;
		rcROI_CHECK_LINE.bottom = rectP.br().y;
		float fPol2 = CHECK_LINE(imgTempP, rcROI_CHECK_LINE, bFIND_LINE_X, dFIND_LINE_A, dFIND_LINE_B, fPAT_Min, fPAT_Max);
		if (fPol2 > 0)
		{
			nPOLY_ORG = 2;
			bX = bFIND_LINE_X;
			dVal_A = dFIND_LINE_A;
			dVal_B = dFIND_LINE_B;
			fPol = fPol2;
			fPol_ALL = fPol / fUM / dRes;
		}
	}
	if (rectP.width <= N_SMALL_AB || rectP.height <= N_SMALL_AB)
	{
		int nPOL_TEMP = 0;
		bool bX_TEMP = FIND_LINE_XY(imgTempP, &nPOL_TEMP);
		float fPol2 = nPOL_TEMP * fUM * dRes;
		if (fPAT_Min <= fPol2 && fPAT_Max >= fPol2)
		{
			nPOLY_ORG = 3;
			bX = bX_TEMP;
			dVal_A = 0;
			dVal_B = 0;
			fPol = fPol2;
			fPol_ALL = nPOL_TEMP;
		}
	}
	if (nPOLY_ORG <= 0 || fPol <= 0)
	{
		dVal_A = 0;
		dVal_B = 0;
		fPol = FIND_TH_POLY(rectP, imgTempP, bX, &dVal_A, &dVal_B, sDataP);
		if (fPol == -100)
		{
			bX = !bX;
			fPol = FIND_TH_POLY(rectP, imgTempP, bX, &dVal_A, &dVal_B, sDataP);
		}
		if (fPol == -100)
			return;
	}
	if (fPol <= fPAT_Min || fPol >= fPAT_Max)
		return;
	if (dVal_A == 0)
	{
		if (CHECK_POLY_LINE(imgTempP, bX, rectP, fPol) == false)
			return;
	}

	float fRET_POL_MIN = fPol;
	float fRET_POL_MAX = fPol;
	if (fRET_POL_MIN <= N_BIN_DIFF)
		return;

	int nRectL = nW * nH;
	int nRectR = 0;
	int nRectT = nW * nH;
	int nRectB = 0;
	cv::Mat imgTempB(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgBin_Temp(nH, nW, CV_8UC1, cv::Scalar(0));
	if (nPOLY_ORG == 1)
	{
		imgBIN_ORG.copyTo(imgTempB);
		cv::Rect rectT = cv::boundingRect(imgTempB);
		if (nRectL > rectT.x) nRectL = rectT.x;
		if (nRectR < rectT.x + rectT.width) nRectR = rectT.x + rectT.width;
		if (nRectT > rectT.y) nRectT = rectT.y;
		if (nRectB < rectT.y + rectT.height) nRectB = rectT.y + rectT.height;
	}
	else
	{
		for (int b = 0; b < arrBin.size(); b++)
		{
			imgBin_Temp.setTo(0);
			DRAW_Contour(arrBin, b, imgBin_Temp, imgBIN_BIN);

			cv::Mat imgTempA;
			cv::bitwise_and(imgTempP, imgBin_Temp, imgTempA);
			double dBin_Cnt = cv::countNonZero(imgBin_Temp);
			double dA_Cnt = cv::countNonZero(imgTempA);
			if (dA_Cnt == 0)
				continue;

			double dPer = dA_Cnt / dBin_Cnt;
			if (dPer < 0.3)
				continue;

			cv::bitwise_or(imgBin_Temp, imgTempB, imgTempB);
			cv::Rect rectT = cv::boundingRect(arrBin[b]);
			if (nRectL > rectT.x) nRectL = rectT.x;
			if (nRectR < rectT.x + rectT.width) nRectR = rectT.x + rectT.width;
			if (nRectT > rectT.y) nRectT = rectT.y;
			if (nRectB < rectT.y + rectT.height) nRectB = rectT.y + rectT.height;
		}
	}
	int nRectW = nRectR - nRectL + 1;
	int nRectH = nRectB - nRectT + 1;
	if (nRectW <= N_BIN_DIFF || nRectH <= N_BIN_DIFF)
		return;

	cv::Rect rectB(nRectL, nRectT, nRectW, nRectH);
	Check_End_Bin(rectB, imgTempB);
	if (CHK_BIN_POLY_XOR(imgTempB, imgTempP, kernel, rectB, rectP) == false)
	{
		cv::bitwise_xor(imgPOLY_BIN, imgTempP, imgPOLY_BIN);
		cv::bitwise_xor(imgBIN_BIN, imgTempB, imgBIN_BIN);
		return;
	}

	RECT_F roiMAX;
	RECT_F roiMIN;
	RECT_F roiMAX_PRE;
	RECT_F roiMIN_PRE;
	roiMAX.Init();
	roiMIN.Init();
	roiMAX_PRE.Init();
	roiMIN_PRE.Init();
	bool bRET_BIN_MIN = true;
	bool bRET_BIN_MAX = true;
	bool bRET_BIN_MISSING = false;

	RECT_F roiMAX_TEMP_F;
	RECT_F roiMIN_TEMP_F;
	roiMAX_TEMP_F.Init();
	roiMIN_TEMP_F.Init();
	for (int c = 0; c < 2; c++)
	{
		RECT_F roiMAX_TEMP;
		RECT_F roiMIN_TEMP;
		roiMAX_TEMP.Init();
		roiMIN_TEMP.Init();
		bool bWA = c == 1;
		int nRET_TEMP = 0;
		if (bWA)
			nRET_TEMP = FIND_TH_WA(sRE, imgBIN_ORG, bX, dVal_A, dVal_B, roiMAX, roiMIN, &roiMAX_TEMP, &roiMIN_TEMP, &roiMAX_TEMP_F, &roiMIN_TEMP_F);
		else
		{
			if (dVal_A == 0)
				nRET_TEMP = FIND_TH(sRE, rectB, imgTempB, imgBIN_ORG, bX, &roiMAX_TEMP, &roiMIN_TEMP);
			else
				nRET_TEMP = FIND_TH_BIN(sRE, rectP, rectB, imgTempB, imgBIN_ORG, bX, dVal_A, dVal_B, &roiMAX_TEMP, &roiMIN_TEMP, &roiMAX_TEMP_F, &roiMIN_TEMP_F, fPol_ALL, bCheckBin);
		}
		bool bMissing = (nRET_TEMP & m_eVOL_TH_FAIL_MISSING) == m_eVOL_TH_FAIL_MISSING;
		bool bRET_TEMP_MAX = (nRET_TEMP & m_eVOL_TH_FAIL_MAX) == m_eVOL_TH_FAIL_MAX;
		bool bRET_TEMP_MIN = (nRET_TEMP & m_eVOL_TH_FAIL_MIN) == m_eVOL_TH_FAIL_MIN;
		bool bTEMP_MAX = roiMAX_TEMP.COMPARE_VALUE(bRET_BIN_MAX, bRET_TEMP_MAX, roiMAX, m_resolX, m_resolY, fRET_POL_MAX, fPol);
		bool bTEMP_MIN = false;
		if (bFIND_LINE && nPOLY_ORG == 2)
		{
			if (bRET_TEMP_MIN || bRET_TEMP_MAX)
				return;
		}
		if (bRET_BIN_MISSING == false)
		{
			if (bMissing == false)
				bTEMP_MIN = roiMIN_TEMP.COMPARE_VALUE(bRET_BIN_MIN, bRET_TEMP_MIN, roiMIN, m_resolX, m_resolY, fRET_POL_MIN, fPol);
			else
			{
				bTEMP_MIN = true;
				bRET_BIN_MISSING = bMissing;
			}
			if (bRET_TEMP_MIN || bRET_TEMP_MAX)
			{
				SaveWorkImg(imgTempP.data, nW, nH, _T("PAT_TH_AB_P.bmp"));
				SaveWorkImg(imgTempB.data, nW, nH, _T("PAT_TH_AB_B.bmp"));
			}
			if (dVal_A != 0 && bRET_TEMP_MIN && bRET_TEMP_MAX && c == 0)
			{
				cv::Mat imgTempP_Temp(nH, nW, CV_8UC1, cv::Scalar(0));
				if (nPOLY_ORG == 1)
				{
					DRAW_Contour(arrPoly, nIDX, imgTempP_Temp, imgPOLY_BIN);
					Check_End_Bin(rectP, imgTempP_Temp);
				}
				else
					imgTempP.copyTo(imgTempP_Temp);
				int nPOL_TEMP = 0;
				bool bX_TEMP = FIND_LINE_XY(imgTempP_Temp, &nPOL_TEMP);
				int nRET_TEMP_TEMP = FIND_TH(sRE, rectB, imgTempB, imgBIN_ORG, bX_TEMP, &roiMAX_TEMP, &roiMIN_TEMP);
				bool bMissing_TEMP = (nRET_TEMP_TEMP & m_eVOL_TH_FAIL_MISSING) == m_eVOL_TH_FAIL_MISSING;
				bool bRET_TEMP_TEMP_MAX = (nRET_TEMP_TEMP & m_eVOL_TH_FAIL_MAX) == m_eVOL_TH_FAIL_MAX;
				bool bRET_TEMP_TEMP_MIN = (nRET_TEMP_TEMP & m_eVOL_TH_FAIL_MIN) == m_eVOL_TH_FAIL_MIN;
				float fPol_TEMP = bX_TEMP ? (float)nPOL_TEMP * m_resolX * fUM : (float)nPOL_TEMP * m_resolY * fUM;
				if (fPol_TEMP >= fPAT_Min && fPol_TEMP <= fPAT_Max && bMissing_TEMP == false && bRET_TEMP_TEMP_MAX == false && bRET_TEMP_TEMP_MIN == false)
				{
					fPol = bX_TEMP ? (float)nPOL_TEMP * m_resolX * fUM : (float)nPOL_TEMP * m_resolY * fUM;
					fRET_POL_MIN = fPol;
					fRET_POL_MAX = fPol;
					bX = bX_TEMP;
					dVal_A = 0;
					dVal_B = 0;
					nRET_TEMP = nRET_TEMP_TEMP;
					bRET_TEMP_MAX = bRET_TEMP_TEMP_MAX;
					bRET_TEMP_MIN = bRET_TEMP_TEMP_MIN;
					if (nPOLY_ORG == 1)
						imgTempP_Temp.copyTo(imgTempP);
					bTEMP_MIN = roiMIN_TEMP.COMPARE_VALUE(bRET_BIN_MIN, bRET_TEMP_MIN, roiMIN, m_resolX, m_resolY, fRET_POL_MIN, fPol);
					bTEMP_MAX = roiMAX_TEMP.COMPARE_VALUE(bRET_BIN_MAX, bRET_TEMP_MAX, roiMAX, m_resolX, m_resolY, fRET_POL_MAX, fPol);
				}
				else
					break;
			}
		}
		if (bTEMP_MAX)
		{
			roiMAX.SetData(roiMAX_TEMP);
			if (c == 0)
				roiMAX_PRE.SetData(roiMAX_TEMP);
			bRET_BIN_MAX = (nRET_TEMP & m_eVOL_TH_FAIL_MAX) == m_eVOL_TH_FAIL_MAX;
		}
		if (bTEMP_MIN)
		{
			roiMIN.SetData(roiMIN_TEMP);
			if (c == 0)
				roiMIN_PRE.SetData(roiMIN_TEMP);
			bMissing = (nRET_TEMP & m_eVOL_TH_FAIL_MISSING) == m_eVOL_TH_FAIL_MISSING;
			bRET_BIN_MIN = (nRET_TEMP & m_eVOL_TH_FAIL_MIN) == m_eVOL_TH_FAIL_MIN;
		}
	}

	float fBIN_MAX = roiMAX.CALC_DIST(m_resolX, m_resolY);
	float fBIN_MIN = roiMIN.CALC_DIST(m_resolX, m_resolY);
	float fMAX_GAP = fBIN_MAX * fUM;
	float fMIN_GAP = fBIN_MIN * fUM;
	float fBIN_MAX_PRE = roiMAX_PRE.CALC_DIST(m_resolX, m_resolY);
	float fBIN_MIN_PRE = roiMIN_PRE.CALC_DIST(m_resolX, m_resolY);
	float fMAX_GAP_PRE = fBIN_MAX_PRE * fUM;
	float fMIN_GAP_PRE = fBIN_MIN_PRE * fUM;
	if (bRET_BIN_MIN == false && (fRET_POL_MIN > fMIN_GAP || bRET_BIN_MISSING))
	{
		float fGap2 = bRET_BIN_MISSING ? fRET_POL_MIN : fRET_POL_MIN - fMIN_GAP;
		if (fMIN_GAP_PRE > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN])
		{
			float fGap3 = abs(fRET_POL_MIN - fMIN_GAP_PRE);
			if (fMIN_GAP < fMIN_GAP_PRE) fGap3 = 0;
			if (fGap2 > fGap3)
			{
				fGap2 = fGap3;
				roiMIN.SetData(roiMIN_PRE);
			}
		}
		if (bPOLY_ORG_T)
		{
			if (fPol2 > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN])
			{
				float fGap3 = abs(fPol2 - fMIN_GAP);
				if (fPol2 < fMIN_GAP) fGap3 = 0;
				if (fGap2 > fGap3)
					fGap2 = fGap3;
			}
			for (int nR = 0; nR < 2; nR++)
			{
				float fPolTemp = CHECK_LINE(ImgPoly, roiMIN, bX, dVal_A, dVal_B, fPAT_Min, fPAT_Max, nR > 0);
				if (fPolTemp > 0)
				{
					float fGap3 = abs(fPolTemp - fMIN_GAP);
					if (fGap2 > fGap3)
						fGap2 = fGap3;
				}
			}
		}
		if (bRET_BIN_MISSING == false && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN])
		{
			for (int nR = 0; nR < 2; nR++)
			{
				float fPolTemp = CHECK_LINE(ImgPoly, roiMIN, bX, dVal_A, dVal_B, fPAT_Min, fPAT_Max, nR > 0);
				if (fPolTemp > 0)
				{
					float fGap3 = abs(fPolTemp - fMIN_GAP);
					if (fPolTemp < fMIN_GAP) fGap3 = 0;
					if (fGap2 > fGap3)
						fGap2 = fGap3;
				}
				fPolTemp = CHECK_LINE(imgTempP, roiMIN, bX, dVal_A, dVal_B, fPAT_Min, fPAT_Max, nR > 0);
				if (fPolTemp > 0)
				{
					float fGap3 = abs(fPolTemp - fMIN_GAP);
					if (fPolTemp < fMIN_GAP) fGap3 = 0;
					if (fGap2 > fGap3)
						fGap2 = fGap3;
				}
				fPolTemp = CHECK_LINE(ImgPolyORG, roiMIN, bX, dVal_A, dVal_B, fPAT_Min, fPAT_Max, nR > 0);
				if (fPolTemp > 0)
				{
					float fGap3 = abs(fPolTemp - fMIN_GAP);
					if (fPolTemp < fMIN_GAP) fGap3 = 0;
					if (fGap2 > fGap3)
						fGap2 = fGap3;
				}
			}
		}
		if (fGap2 > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN])
		{
			sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN] = fGap2;
			sRstAlgo->m_rcRect_TH[0].left = roiMIN.left;
			sRstAlgo->m_rcRect_TH[0].top = roiMIN.top;
			sRstAlgo->m_rcRect_TH[0].right = roiMIN.right;
			sRstAlgo->m_rcRect_TH[0].bottom = roiMIN.bottom;
			nMIN++;
		}
	}
	if (bRET_BIN_MAX == false && fRET_POL_MAX < fMAX_GAP)
	{
		float fGap2 = fMAX_GAP - fRET_POL_MAX;
		if (bPOLY_ORG_T)
		{
			if (fPol2 > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX])
			{
				float fGap3 = abs(fPol2 - fMAX_GAP);
				if (fGap2 > fGap3)
					fGap2 = fGap3;
			}
			for (int nR = 0; nR < 2; nR++)
			{
				float fPolTemp = CHECK_LINE(ImgPoly, roiMAX, bX, dVal_A, dVal_B, fPAT_Min, fPAT_Max, nR > 0);
				if (fPolTemp > 0)
				{
					float fGap3 = abs(fPolTemp - fMAX_GAP);
					if (fGap2 > fGap3)
						fGap2 = fGap3;
				}
			}
		}
		if (fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX])
		{
			for (int nR = 0; nR < 2; nR++)
			{
				float fPolTemp = CHECK_LINE(ImgPoly, roiMAX, bX, dVal_A, dVal_B, fPAT_Min, fPAT_Max, nR > 0);
				if (fPolTemp > 0)
				{
					float fGap3 = abs(fPolTemp - fMAX_GAP);
					if (fGap2 > fGap3)
						fGap2 = fGap3;
				}
				fPolTemp = CHECK_LINE(imgTempP, roiMAX, bX, dVal_A, dVal_B, fPAT_Min, fPAT_Max, nR > 0);
				if (fPolTemp > 0)
				{
					float fGap3 = abs(fPolTemp - fMAX_GAP);
					if (fGap2 > fGap3)
						fGap2 = fGap3;
				}
				fPolTemp = CHECK_LINE(ImgPolyORG, roiMAX, bX, dVal_A, dVal_B, fPAT_Min, fPAT_Max, nR > 0);
				if (fPolTemp > 0)
				{
					float fGap3 = abs(fPolTemp - fMAX_GAP);
					if (fGap2 > fGap3)
						fGap2 = fGap3;
				}
			}
		}

		if (fGap2 > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX])
		{
			sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX] = fGap2;
			sRstAlgo->m_rcRect_TH[1].left = roiMAX.left;
			sRstAlgo->m_rcRect_TH[1].top = roiMAX.top;
			sRstAlgo->m_rcRect_TH[1].right = roiMAX.right;
			sRstAlgo->m_rcRect_TH[1].bottom = roiMAX.bottom;
			nMAX++;
		}
	}

	cv::bitwise_xor(imgPOLY_BIN, imgTempP, imgPOLY_BIN);
	cv::bitwise_xor(imgBIN_BIN, imgTempB, imgBIN_BIN);
}
double PAD_PAT::PAT_TH_LINE(cv::Mat imgSrc, std::vector<std::vector<cv::Point>> arrPoly, int nIDX, VOL_TH_DATA sData)
{
	int nW = imgSrc.cols;
	int nH = imgSrc.rows;
	cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::drawContours(imgTemp, arrPoly, nIDX, cv::Scalar(255), cv::FILLED);
	cv::Rect rectP = cv::boundingRect(arrPoly[nIDX]);
	const uchar* puc = imgTemp.data;
	double dPerX = (double)rectP.height / (double)rectP.width;
	bool bX = dPerX > 0.7;
	int nA_T = bX ? nH : nW;
	int nA_S = bX ? rectP.y : rectP.x;
	int nA_E = bX ? rectP.y + rectP.height : rectP.x + rectP.width;
	int nB_T = bX ? nW : nH;
	int nB_S = bX ? rectP.x : rectP.y;
	int nB_E = bX ? rectP.x + rectP.width : rectP.y + rectP.height;
	int nGAP = 4;
	nA_S += nGAP;
	nB_S -= nGAP;
	nA_E -= nGAP;
	nB_E += nGAP;
	sData.ClearValue();
	for (int a = nA_S; a <= nA_E; a++)
	{
		if (a < 0 || a >= nA_T)
			continue;
		int nS = -1;
		int nE = -1;
		for (int b = nB_S; b <= nB_E; b++)
		{
			if (b < 0 || b >= nB_T)
				continue;
			int nIndex = bX ? (a * nW) + b : (b * nW) + a;
			if (puc[nIndex] == 255)
			{
				nS = b;
				break;
			}
		}
		if (nS < 0) continue;
		for (int b = nB_E; b >= nB_S; b--)
		{
			if (b < 0 || b >= nB_T)
				continue;
			int nIndex = bX ? (a * nW) + b : (b * nW) + a;
			if (puc[nIndex] == 255)
			{
				nE = b;
				break;
			}
		}
		if (nE < 0) continue;
		double dB_C = (nS + nE) / 2.0f;
		double dDist = nE - nS;
		if (bX)
			sData.AddData(dB_C, a, dDist);
		else
			sData.AddData(a, dB_C, dDist);
	}

	double dAValue = 0;
	double dBValue = 0;
	double dAVGB = sData.GetAVG(bX);
	if (sData.m_nC > 0 && dAVGB > 0)
	{
		double dAngle = GetGradient(sData.m_dArrCX, sData.m_dArrCY, sData.m_nC, dAValue, dBValue, !bX);
		double dAVGA = (dAVGB - dBValue) / dAValue;
		sData.DEBUG_POLY(imgSrc, dAValue, dBValue, dAVGA, dAVGB, bX);
	}
	return dAValue;
}
int PAD_PAT::FIND_TH_WA(VOL_TH_RE sRE, cv::Mat img, bool bX, double dAValue, double dBValue, RECT_F pMAX_ORG, RECT_F pMIN_ORG, RECT_F* pMAX, RECT_F* pMIN, RECT_F* pMAX_F, RECT_F* pMIN_F)
{
	int nRet = 0;
	pMAX->SetData(pMAX_ORG);
	pMIN->SetData(pMIN_ORG);
	for (int a = 0; a < 2; a++)
	{
		int nWA = 0;
		for (int b = 0; b < 2; b++)
		{
			float fX = -1;
			float fY = -1;
			if (a == 0)
			{
				if (b == 0)
				{
					fX = pMAX_ORG.left;
					fY = pMAX_ORG.top;
				}
				else
				{
					fX = pMAX_ORG.right;
					fY = pMAX_ORG.bottom;
				}
			}
			else
			{
				if (b == 0)
				{
					fX = pMIN_ORG.left;
					fY = pMIN_ORG.top;
				}
				else
				{
					fX = pMIN_ORG.right;
					fY = pMIN_ORG.bottom;
				}
			}
			if (dAValue == 0)
			{
				if (bX)
				{
					if (b == 0)
						fX -= 1;
					else
						fX += 1;
				}
				else
				{
					if (b == 0)
						fY -= 1;
					else
						fY += 1;
				}
			}
			else
			{
				bool bFind = true;
				if (a == 0)
				{
					if (pMAX_F->left != pMAX_F->right && pMAX_F->top != pMAX_F->bottom &&
						pMAX_F->left >= 0 && pMAX_F->top >= 0 &&
						pMAX_F->right > 0 && pMAX_F->bottom > 0)
					{
						bFind = false;
						if (b == 0)
						{
							fX = pMAX_F->left;
							fY = pMAX_F->top;
						}
						else
						{
							fX = pMAX_F->right;
							fY = pMAX_F->bottom;
						}
					}
				}
				else
				{
					if (pMIN_F->left != pMIN_F->right && pMIN_F->top != pMIN_F->bottom &&
						pMIN_F->left >= 0 && pMIN_F->top >= 0 &&
						pMIN_F->right > 0 && pMIN_F->bottom > 0)
					{
						bFind = false;
						if (b == 0)
						{
							fX = pMIN_F->left;
							fY = pMIN_F->top;
						}
						else
						{
							fX = pMIN_F->right;
							fY = pMIN_F->bottom;
						}
					}
				}
				if (bFind)
				{
					bool bXR = bX;
					double dReA = -1.0 / dAValue;
					double dReB = (double)fY - (dReA * (double)fX);
					for (int c = 0; c < 2; c++)
					{
						int nAdd = c == 0 ? 1 : -1;
						double dX = bXR ? fX + nAdd : ((fY + nAdd) - dReB) / dReA;
						double dY = bXR ? (fX + nAdd) * dReA + dReB : (fY + nAdd);
						int nX2 = (int)RounD(dX);
						int nY2 = (int)RounD(dY);
						if (nX2 < 0 || nX2 >= img.cols)
							continue;
						if (nY2 < 0 || nY2 >= img.rows)
							continue;

						int nIDX = (nY2 * img.cols) + nX2;
						if (img.data[nIDX] == 255)
							continue;

						fX = dX;
						fY = dY;
						break;
					}
				}
			}

			bool bWhite = false;
			for (int y = 0; y <= 2; y++)
			{
				int nY = (int)fY;
				if (y == 1)
					nY = (int)(fY - 0.5);
				else if (y == 2)
					nY = (int)(fY + 0.5);
				for (int x = 0; x <= 2; x++)
				{
					int nX = (int)fX;
					if (x == 1)
						nX = (int)(fX - 0.5);
					else if (x == 2)
						nX = (int)(fX + 0.5);

					int nIndex = (nY * sRE.m_nW) + nX;
					if (nIndex < 0 || nX < 0 || nY < 0)
						continue;
					if (nIndex >= sRE.m_nW * sRE.m_nH || nX >= sRE.m_nW || nY >= sRE.m_nH)
						continue;
					if (sRE.WhiteArea(nIndex) == false)
						continue;
					bWhite = true;
					break;
				}
				if (bWhite) break;
			}
			if (bWhite == false)
				continue;

			nWA++;
			if (a == 0)
			{
				if (b == 0)
				{
					pMAX->left = fX;
					pMAX->top = fY;
				}
				else
				{
					pMAX->right = fX;
					pMAX->bottom = fY;
				}
			}
			else
			{
				if (b == 0)
				{
					pMIN->left = fX;
					pMIN->top = fY;
				}
				else
				{
					pMIN->right = fX;
					pMIN->bottom = fY;
				}
			}
		}
		if (nWA > 0)
			continue;

		if (a == 0)
			nRet |= m_eVOL_TH_FAIL_MAX;
		else
			nRet |= m_eVOL_TH_FAIL_MIN;
	}
	return nRet;
}
bool PAD_PAT::FIND_POS(cv::Mat imgPOLY_BIN, cv::Mat imgBIN_BIN, cv::Mat ImgPoly, cv::Mat imgBIN_ORG, bool bX, float fPAT_Min, float fPAT_Max, VOL_TH_DATA* sDataP, VOL_TH_DATA* sDataB)
{
	int nW = imgPOLY_BIN.cols;
	int nH = imgPOLY_BIN.rows;
	cv::Rect roiPOLY = cv::boundingRect(imgPOLY_BIN);
	cv::Rect roiBIN = cv::boundingRect(imgBIN_BIN);
	int nAll_L = std::min(roiPOLY.x, roiBIN.x) - N_POLY_DIFF;
	int nAll_T = std::min(roiPOLY.y, roiBIN.y) - N_POLY_DIFF;
	int nAll_R = std::max(roiPOLY.x + roiPOLY.width, roiBIN.x + roiBIN.width) + N_POLY_DIFF;
	int nAll_B = std::max(roiPOLY.y + roiPOLY.height, roiBIN.y + roiBIN.height) + N_POLY_DIFF;
	int nOUT = cv::countNonZero(imgPOLY_BIN);
	if (nOUT <= 2)
		return false;

	const int FIND_CNT = 2;
	float fUM = 1000.0f;
	double dRes = m_resolX;
	int nROIA = nW;
	int nROIA_S = nAll_L;
	int nROIA_E = nAll_R;
	int nROIB = nH;
	int nROIB_S = nAll_T;
	int nROIB_E = nAll_B;
	if (bX == false)
	{
		dRes = m_resolY;
		nROIA = nH;
		nROIA_S = nAll_T;
		nROIA_E = nAll_B;
		nROIB = nW;
		nROIB_S = nAll_L;
		nROIB_E = nAll_R;
	}
	double dPer = (double)nROIB / (double)nROIA;
	float fPAT_Min_pix = fPAT_Min / fUM / dRes;
	float fPAT_Max_pix = fPAT_Max / fUM / dRes;
	const uchar* polyData = ImgPoly.data;
	const uchar* binData = imgBIN_ORG.data;
	for (int nB = nROIB_S; nB <= nROIB_E; nB++)
	{
		if (nB < 0 || nB >= nROIB)
			continue;
		bool bFind[FIND_CNT] = { false, false };
		int nS[FIND_CNT] = { 0, 0 };
		int nE[FIND_CNT] = { 0, 0 };
		for (int nA = nROIA_S; nA <= nROIA_E; nA++)
		{
			if (nA < 0 || nA >= nROIA)
				continue;
			for (int a = 0; a < FIND_CNT; a++)
			{
				int nIndex = bX ? (nB * nW) + nA : (nA * nW) + nB;
				const UCHAR* pucBIN = a == 0 ? polyData : binData;
				if (pucBIN[nIndex] == 255)
				{
					if (bFind[a] == false)
					{
						bFind[a] = true;
						nS[a] = nA;
						nE[a] = nA;
					}
					else
					{
						if (nS[a] > nA) nS[a] = nA;
						if (nE[a] < nA) nE[a] = nA;
					}
				}
				else
				{
					if (bFind[a] == false)
						continue;

					int nGAP = nE[a] - nS[a] + 1;
					if (nGAP > 1 && fPAT_Min_pix <= nGAP && fPAT_Max_pix >= nGAP)
					{
						if (a == 0)
							sDataP->AddData(nB, nS[a], nE[a], false);
						else
							sDataB->AddData(nB, nS[a], nE[a], false);
					}
					bFind[a] = false;
					nS[a] = 0;
					nE[a] = 0;
				}
			}
		}
		for (int a = 0; a < FIND_CNT; a++)
		{
			if (bFind[a] == false)
				continue;

			const UCHAR* pucBIN = a == 0 ? polyData : binData;
			int nGAP = nE[a] - nS[a] + 1;
			if (nGAP > 1 && fPAT_Min_pix <= nGAP && fPAT_Max_pix >= nGAP)
			{
				if (a == 0)
					sDataP->AddData(nB, nS[a], nE[a], false);
				else
					sDataB->AddData(nB, nS[a], nE[a], false);
			}
		}
	}
	return true;
}
bool PAD_PAT::FIND_POS_ROT(cv::Mat img, cv::Mat imgSRC, bool bX, float fPAT_Min, float fPAT_Max, VOL_TH_DATA* sData)
{
	int nW = imgSRC.cols;
	int nH = imgSRC.rows;
	int nOUT = cv::countNonZero(img);
	if (nOUT <= 2)
		return false;

	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(img, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	if (allcontour.size() <= 0)
		return false;

	cv::Rect roiSrc = cv::boundingRect(imgSRC);
	float fUM = 1000.0f;
	double dRes = bX ? m_resolX : m_resolY;
	float fPAT_Min_pix = fPAT_Min / fUM / dRes;
	float fPAT_Max_pix = fPAT_Max / fUM / dRes;
	const uchar* pucBIN = imgSRC.data;
	for (int a = 0; a < allcontour.size(); a++)
	{
#if _DEBUG
		cv::Mat imgDEBUG(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::drawContours(imgDEBUG, allcontour, a, cv::Scalar(255), cv::FILLED);
#endif

		cv::Rect roi = cv::boundingRect(allcontour[a]);
		int nROIA = nW;
		int nROIA_S = roiSrc.x;
		int nROIA_E = roiSrc.br().x;
		int nROIB = nH;
		int nROIB_S = roi.y;
		int nROIB_E = roi.y + roi.height;
		if (bX == false)
		{
			nROIA = nH;
			nROIA_S = roiSrc.y;
			nROIA_E = roiSrc.br().y;
			nROIB = nW;
			nROIB_S = roi.x;
			nROIB_E = roi.x + roi.width;
		}

		for (int nB = nROIB_S; nB <= nROIB_E; nB++)
		{
			if (nB < 0 || nB >= nROIB)
				continue;

			bool bFind = false;
			int nS = 0;
			int nE = 0;
			for (int nA = nROIA_S; nA <= nROIA_E; nA++)
			{
				if (nA < 0 || nA >= nROIA)
					continue;
				int nIndex = bX ? (nB * nW) + nA : (nA * nW) + nB;
				if (pucBIN[nIndex] == 255)
				{
					if (bFind == false)
					{
						bFind = true;
						nS = nA;
						nE = nA;
					}
					else
					{
						if (nS > nA) nS = nA;
						if (nE < nA) nE = nA;
					}
				}
				else
				{
					if (bFind == false)
						continue;

					int nGAP = nE - nS + 1;
					if (nGAP > 1 && fPAT_Min_pix <= nGAP && fPAT_Max_pix >= nGAP)
					{
						sData->AddData(nB, nS, nE, false);
					}
					bFind = false;
					nS = 0;
					nE = 0;
				}
			}
			if (bFind == false)
				continue;

			int nGAP = nE - nS + 1;
			if (nGAP > 1 && fPAT_Min_pix <= nGAP && fPAT_Max_pix >= nGAP)
				sData->AddData(nB, nS, nE, false);
		}
	}
	return true;
}
double PAD_PAT::GetGradient(double *ptrnLineX, double *ptrnLineY, double nTotalCnt, double &pdA, double &pdB, bool bHorizon)
{
	double dGradient = 0.0;
	if (nTotalCnt == 0)
		return dGradient;
	// 1: -[Sig]xy + a[Sig]x^2 + b[Sig]x = 0
	// 2: -[Sig]y + a[Sig]x + bn = 0
	// n = nTotalCnt
	// a, b를 계산
	int nStartIndex = 0;
	double dSigmaXY = 0;
	double dSigmaXSquare = 0;

	double dSigmaX = 0;
	double dSigmaY = 0;
	while (nStartIndex < nTotalCnt)
	{
		double dXPos = ptrnLineX[nStartIndex];
		double dYPos = ptrnLineY[nStartIndex];
		//if(dXPos < 0 || dYPos < 0)
		//{
		//	nStartIndex++;
		//	continue;
		//}

		if (bHorizon == true)
		{
			dSigmaXY = dSigmaXY + (dXPos * dYPos);
			dSigmaXSquare = dSigmaXSquare + (dXPos * dXPos);
			dSigmaX = dSigmaX + dXPos;
			dSigmaY = dSigmaY + dYPos;
		}
		else
		{
			dSigmaXY = dSigmaXY + (dXPos * dYPos);
			dSigmaXSquare = dSigmaXSquare + (dYPos * dYPos);
			dSigmaX = dSigmaX + dYPos;
			dSigmaY = dSigmaY + dXPos;
		}

		nStartIndex++;
	}
	// 	pdB = (nSigmaXY - nSigmaY) / (nSigmaX - nTotalCnt);
	// 	pdA = (nSigmaXY + (pdB * nSigmaX)) / nSigmaXSquare;
	double dValue = (dSigmaX * dSigmaX) - (dSigmaXSquare * nTotalCnt);
	double dValue1 = (dSigmaX * dSigmaXY) - (dSigmaXSquare * dSigmaY);
	if (dValue != 0 && dValue1 != 0)
		pdB = dValue1 / dValue;
	else
		pdB = 0;
	dValue = dSigmaXSquare;
	dValue1 = dSigmaXY - (dSigmaX * pdB);
	if (dValue != 0 && dValue1 != 0)
		pdA = dValue1 / dValue;
	else
		pdA = 0;

	//Gradient = Atan(pnA)
	dGradient = atan((double)pdA) * 180 / PI;

	return dGradient;
}
void PAD_PAT::Check_End_Bin(cv::Rect rect, cv::Mat Img)
{
	int nW = Img.cols;
	int nH = Img.rows;
	if (rect.x == 0 || rect.br().x == nW)
	{
		for (int y = rect.y; y <= rect.br().y; y++)
		{
			if (y < 0 || y >= nH)
				continue;

			int nWCnt = 0;
			int nIdx = -1;
			if (rect.x == 0)
				nIdx = (y * nW);
			else if (rect.br().x == nW)
				nIdx = (y * nW) + (nW - 1);
			else
				continue;
			if (Img.data[nIdx] != 255)
				continue;
			for (int x = rect.x; x <= rect.br().x; x++)
			{
				if (x < 0 || x >= nW)
					continue;
				nIdx = (y * nW) + x;
				if (Img.data[nIdx] == 255)
					nWCnt++;
			}
			if (nWCnt > PAT_INSP_MIN_SIZE)
				continue;
			for (int x = rect.x; x <= rect.br().x; x++)
			{
				if (x < 0 || x >= nW)
					continue;
				int nIdx = (y * nW) + x;
				if (Img.data[nIdx] == 255)
					Img.data[nIdx] = 0;
			}
		}
	}
	if (rect.y == 0 || rect.br().y == nH)
	{
		for (int x = rect.x; x <= rect.br().x; x++)
		{
			if (x < 0 || x >= nW)
				continue;

			int nWCnt = 0;
			int nIdx = -1;
			if (rect.y == 0)
				nIdx = x;
			else if (rect.br().y == nH)
				nIdx = ((nH - 1) * nW) + x;
			else
				continue;
			if (Img.data[nIdx] != 255)
				continue;
			for (int y = rect.y; y <= rect.br().y; y++)
			{
				if (y < 0 || y >= nH)
					continue;
				nIdx = (y * nW) + x;
				if (Img.data[nIdx] == 255)
					nWCnt++;
			}
			if (nWCnt > PAT_INSP_MIN_SIZE)
				continue;
			for (int y = rect.y; y <= rect.br().y; y++)
			{
				if (y < 0 || y >= nH)
					continue;
				int nIdx = (y * nW) + x;
				if (Img.data[nIdx] == 255)
					Img.data[nIdx] = 0;
			}
		}
	}
}
void PAD_PAT::POLY_ROI_LINE(cv::Mat ImgPoly, cv::Mat ImgPolyORG, cv::Mat imgPOLY_BIN,
	cv::Mat imgBIN_BIN, cv::Mat imgBIN_ORG, cv::Mat kernel,
	float fPAT_Min, float fPAT_Max, VOL_TH_DATA sDataP, VOL_TH_RE sRE, int &nMIN, int &nMAX, RstAlgoVolume* sRstAlgo)
{
	int nW = ImgPoly.cols;
	int nH = ImgPoly.rows;
	std::vector<std::vector<cv::Point>> arrPoly;
	std::vector<std::vector<cv::Point>> arrBin;
	std::vector<cv::Vec4i> hierarchy;

	cv::Mat imgTEMP_P(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTEMP_B(nH, nW, CV_8UC1, cv::Scalar(0));
#if _DEBUG
	cv::imwrite(std::string("D:\\POLY_ROI_LINE_P.bmp"), ImgPoly);
	cv::imwrite(std::string("D:\\POLY_ROI_LINE_B.bmp"), imgBIN_BIN);
#endif
	bool bONE = false;
	cv::Rect roiP = cv::boundingRect(ImgPolyORG);
	cv::Rect roiB = cv::boundingRect(imgBIN_ORG);
	cv::Rect roiPB = roiP | roiB;
	if (roiPB.x > N_GAP_ORG && roiPB.br().x < nW - N_GAP_ORG &&
		roiPB.y > N_GAP_ORG && roiPB.br().y < nH - N_GAP_ORG)
	{
		arrPoly.clear();
		hierarchy.clear();
		cv::findContours(ImgPoly, arrPoly, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		if (arrPoly.size() == 1)
			bONE = true;
	}

	for (int i = 0; i < 5; i++)
	{
		double dA = 0;
		double dB = 0;
		bool bX = false;
		bool bEdge = i == 0;
		imgTEMP_P.setTo(0);
		RECT_F roiP;
		roiP.Init();
		bool bRetP = FIND_LINE(imgPOLY_BIN, imgTEMP_P, sDataP, &dA, &dB, false, &roiP, &bX, bONE, bEdge);
		if (bRetP == false || abs(dA) <= 0.2 || abs(dA) >= 5)
			break;

		imgTEMP_B.setTo(0);
		bool bRetB = FIND_LINE(imgBIN_BIN, imgTEMP_B, sDataP, &dA, &dB, true, &roiP, &bX, bONE, bEdge);
		if (bRetB == false)
			break;

		arrPoly.clear();
		hierarchy.clear();
		cv::findContours(imgTEMP_P, arrPoly, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		arrBin.clear();
		hierarchy.clear();
		cv::findContours(imgTEMP_B, arrBin, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		int nPOLY_BIN = cv::countNonZero(imgPOLY_BIN);
		for (int a = 0; a < arrPoly.size(); a++)
		{
			PAT_TH_AB(a, ImgPoly, ImgPolyORG, imgPOLY_BIN, imgBIN_BIN, imgBIN_ORG, kernel,
				arrPoly, arrBin, fPAT_Min, fPAT_Max, sDataP, sRE, nMIN, nMAX, sRstAlgo, true, bX, dA, dB);
		}
		int nRST_POLY_BIN = cv::countNonZero(imgPOLY_BIN);
		if (nPOLY_BIN == nRST_POLY_BIN)
			break;
	}
}
bool PAD_PAT::FIND_LINE(cv::Mat ImgSrc, cv::Mat ImgRst, VOL_TH_DATA sDataP, double* dAValue, double* dBValue, bool bBIN, RECT_F* roiPOLY, bool* pbX, bool bONE, bool bEdge)
{
	int nW = ImgSrc.cols;
	int nH = ImgSrc.rows;
	cv::Rect roiSRC = cv::boundingRect(ImgSrc);
	if (roiSRC.width <= N_INSP_OUT || roiSRC.height <= N_INSP_OUT)
		return false;

	int nTotal = 1;
	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat imgTempP(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTempP2(nH, nW, CV_8UC1, cv::Scalar(0));
	std::vector<cv::Vec4i> lines;
	cv::Mat edge = ImgSrc.clone();
	if (bBIN == false)
	{
		if (bEdge)
			cv::Canny(ImgSrc, edge, 50, 150);

		double rho = 50;
		double theta = 50;
		int threshold = 10;
		for (int a = 0; a < 50; a++)
		{
			rho = 50 - a;
			for (int b = 0; b < 50; b++)
			{
				theta = 50 - b;
				HoughLinesP(edge, lines, 1, CV_PI / 180, rho, theta, threshold);
				if (lines.size() > 2)
					break;
			}
			if (lines.size() > 2)
				break;
		}
		if (lines.size() > 2)
		{
			sort(lines.begin(), lines.end(), [](const cv::Vec4i& a, const cv::Vec4i& b)
			{
				return cv::norm(cv::Point(a[0], a[1]) - cv::Point(a[2], a[3])) >
					cv::norm(cv::Point(b[0], b[1]) - cv::Point(b[2], b[3]));
			});
		}
		nTotal = lines.size();
	}
	for (int n = 0; n < nTotal; n++)
	{
		sDataP.ClearValue();
		double dX1 = 0;
		double dY1 = 0;
		double dX2 = 0;
		double dY2 = 0;
		if (bBIN)
		{
			dX1 = roiPOLY->left;
			dY1 = roiPOLY->top;
			dX2 = roiPOLY->right;
			dY2 = roiPOLY->bottom;
		}
		else
		{
			dX1 = lines[n][0];
			dY1 = lines[n][1];
			dX2 = lines[n][2];
			dY2 = lines[n][3];
			roiPOLY->left = dX1;
			roiPOLY->top = dY1;
			roiPOLY->right = dX2;
			roiPOLY->bottom = dY2;
		}
		double dMINX = dX2 > dX1 ? dX1 : dX2;
		double dMAXX = dX2 > dX1 ? dX2 : dX1;
		double dMINY = dY2 > dY1 ? dY1 : dY2;
		double dMAXY = dY2 > dY1 ? dY2 : dY1;
		double dA = (dY2 - dY1) / (dX2 - dX1);
		double dB = dY1 - (dA * dX1);
		double dDistX = abs(dX2 - dX1);
		double dDistY = abs(dY2 - dY1);
		bool bX = dDistX >= dDistY;
		if (bBIN)
		{
			dA = *dAValue;
			dB = *dBValue;
			bX = *pbX;
		}
		else
		{
			dMINX = roiSRC.x;
			dMAXX = roiSRC.br().x;
			dMINY = roiSRC.y;
			dMAXY = roiSRC.br().y;
		}
		int nA_T = bX ? nH : nW;
		int nB_T = bX ? nW : nH;
		int nA_S = bX ? dMINY : dMINX;
		int nA_E = bX ? dMAXY : dMAXX;
		int nB_S = bX ? dMINX : dMINY;
		int nB_E = bX ? dMAXX : dMAXY;
		imgTempP.setTo(0);
		for (int b = nB_S; b < nB_E; b++)
		{
			if (b < 0 || b >= nB_T)
				continue;
			double dX = bX ? b : (b - dB) / dA;
			double dY = bX ? (dA * b) + dB : b;
			bool bWhite = false;
			for (int y = 0; y <= 2; y++)
			{
				int nY = (int)dY;
				if (y == 1)
					nY = (int)(dY - 0.5);
				else if (y == 2)
					nY = (int)(dY + 0.5);
				for (int x = 0; x <= 2; x++)
				{
					int nX = (int)dX;
					if (x == 1)
						nX = (int)(dX - 0.5);
					else if (x == 2)
						nX = (int)(dX + 0.5);

					int nIndex = (nY * nW) + nX;
					if (nIndex < 0 || nX < 0 || nY < 0)
						continue;
					if (nIndex >= nW * nH || nX >= nW || nY >= nH)
						continue;
					if (ImgSrc.data[nIndex] != 255)
						continue;
					bWhite = true;
					break;
				}
				if (bWhite) break;
			}
			if (bWhite == false)
				continue;

			bool bXR = bX;
			double dReA = -1.0 / dA;
			double dReB = dY - (dReA * dX);
			int nW_CNT = 0;
			int nC_S = -1;
			int nC_E = -1;
			for (int c = 0; c < nB_T; c++)
			{
				double dX2 = bXR ? c : c * dReA + dReB;
				double dY2 = bXR ? c * dReA + dReB : c;
				if (dX2 < 0 || dX2 >= nW)
					continue;
				if (dY2 < 0 || dY2 >= nH)
					continue;
				if (dX2 < dMINX || dX2 > dMAXX)
					continue;
				if (dY2 < dMINY || dY2 > dMAXY)
					continue;

				bool bWhiteR = false;
				for (int y = 0; y <= 2; y++)
				{
					int nY = (int)dY2;
					if (y == 1)
						nY = (int)(dY2 - 0.5);
					else if (y == 2)
						nY = (int)(dY2 + 0.5);
					for (int x = 0; x <= 2; x++)
					{
						int nX = (int)dX2;
						if (x == 1)
							nX = (int)(dX2 - 0.5);
						else if (x == 2)
							nX = (int)(dX2 + 0.5);

						int nIndex = (nY * nW) + nX;
						if (nIndex < 0 || nX < 0 || nY < 0)
							continue;
						if (nIndex >= nW * nH || nX >= nW || nY >= nH)
							continue;
						if (ImgSrc.data[nIndex] != 255)
							continue;
						bWhiteR = true;
						imgTempP.data[nIndex] = 255;
						nW_CNT++;
						if (bXR)
						{
							if (nC_S == -1 || nC_S > nY)
								nC_S = nY;
							if (nC_E == -1 || nC_E < nY)
								nC_E = nY;
						}
						else
						{
							if (nC_S == -1 || nC_S > nX)
								nC_S = nX;
							if (nC_E == -1 || nC_E < nX)
								nC_E = nX;
						}
						break;
					}
					if (bWhiteR) break;
				}
			}
			if (nW_CNT <= 5)
				continue;
			sDataP.AddDataL(b, nW_CNT);
		}
		int nMAX_GAP = bBIN ? 4 : 2;
		std::vector<int> vecMAX = sDataP.GetMAX_LIST(nMAX_GAP);
		if (vecMAX.size() <= 0)
			continue;

		int nMAX_CNT = vecMAX[0];
		for (int i = 0; i < sDataP.m_nIDX; i++)
		{
			int nGAP = sDataP.m_nArrE[i];
			if (nGAP <= nMAX_CNT + (nMAX_GAP * 2))
				continue;

			int nB = sDataP.m_nArrB[i];
			double dX = bX ? nB : (nB - dB) / dA;
			double dY = bX ? (dA * nB) + dB : nB;
			bool bXR = bX;
			double dReA = -1.0 / dA;
			double dReB = dY - (dReA * dX);
			int nW_CNT = 0;
			int nC_S = -1;
			int nC_E = -1;
			for (int c = 0; c < nB_T; c++)
			{
				double dX2 = bXR ? c : c * dReA + dReB;
				double dY2 = bXR ? c * dReA + dReB : c;
				if (dX2 < 0 || dX2 >= nW)
					continue;
				if (dY2 < 0 || dY2 >= nH)
					continue;
				if (bXR)
				{
					if (dY2 < dMINY || dY2 > dMAXY)
						continue;
				}
				else
				{
					if (dX2 < dMINX || dX2 > dMAXX)
						continue;
				}

				bool bWhiteR = false;
				for (int y = 0; y <= 2; y++)
				{
					int nY = (int)dY2;
					if (y == 1)
						nY = (int)(dY2 - 0.5);
					else if (y == 2)
						nY = (int)(dY2 + 0.5);
					for (int x = 0; x <= 2; x++)
					{
						int nX = (int)dX2;
						if (x == 1)
							nX = (int)(dX2 - 0.5);
						else if (x == 2)
							nX = (int)(dX2 + 0.5);

						int nIndex = (nY * nW) + nX;
						if (nIndex < 0 || nX < 0 || nY < 0)
							continue;
						if (nIndex >= nW * nH || nX >= nW || nY >= nH)
							continue;
						if (imgTempP.data[nIndex] != 255)
							continue;
						bWhiteR = true;
						imgTempP.data[nIndex] = 0;
						nW_CNT++;
						if (bXR)
						{
							if (nC_S == -1 || nC_S > nY)
								nC_S = nY;
							if (nC_E == -1 || nC_E < nY)
								nC_E = nY;
						}
						else
						{
							if (nC_S == -1 || nC_S > nX)
								nC_S = nX;
							if (nC_E == -1 || nC_E < nX)
								nC_E = nX;
						}
						break;
					}
					if (bWhiteR) break;
				}
			}
		}

		allcontour.clear();
		hierarchy.clear();
		cv::findContours(imgTempP, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		int nIMG_CNT = allcontour.size();
		int nMAXID = 0;
		if (nIMG_CNT <= 0)
			continue;
		else if (bONE == false && nIMG_CNT > 1)
		{
			int nMAX_A = 0;
			for (int n = 0; n < nIMG_CNT; n++)
			{
				cv::Rect roiM = cv::boundingRect(allcontour[n]);
				int nArea = cv::contourArea(allcontour[n]);
				if (nArea > nMAX_A)
				{
					nMAX_A = nArea;
					nMAXID = n;
				}
			}
			if (nMAX_A <= 0)
				continue;
			else if (nMAXID < 0 || nMAXID >= nIMG_CNT)
				continue;

			imgTempP2.setTo(0);
			cv::drawContours(imgTempP2, allcontour, nMAXID, cv::Scalar(255), cv::FILLED);
			cv::bitwise_and(ImgSrc, imgTempP2, imgTempP);
		}

		cv::Rect roiTempP = cv::boundingRect(imgTempP);
		if (roiTempP.width <= N_INSP_OUT || roiTempP.height <= N_INSP_OUT)
			continue;

		imgTempP2.setTo(0);
		allcontour.clear();
		hierarchy.clear();
		cv::findContours(imgTempP, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		nIMG_CNT = allcontour.size();
		for (int n = 0; n < nIMG_CNT; n++)
		{
			cv::Mat imgTempP3(nH, nW, CV_8UC1, cv::Scalar(0));
			cv::drawContours(imgTempP3, allcontour, n, cv::Scalar(255), cv::FILLED);
			cv::bitwise_or(imgTempP3, imgTempP2, imgTempP2);
		}
		cv::bitwise_and(imgTempP2, ImgSrc, imgTempP2);

		allcontour.clear();
		hierarchy.clear();
		cv::findContours(imgTempP2, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		nIMG_CNT = allcontour.size();
		if (nIMG_CNT <= 0)
			continue;
		else if (nIMG_CNT > 1)
		{
			nMAXID = -1;
			int nMAX_A = 0;
			for (int n = 0; n < nIMG_CNT; n++)
			{
				cv::Rect roiM = cv::boundingRect(allcontour[n]);
				int nArea = cv::contourArea(allcontour[n]);
				if (nArea > nMAX_A)
				{
					nMAX_A = nArea;
					nMAXID = n;
				}
			}
			if (nMAX_A <= 0)
				continue;
			if (nMAXID < 0 || nMAXID >= nIMG_CNT)
				continue;
			cv::drawContours(ImgRst, allcontour, nMAXID, cv::Scalar(255), cv::FILLED);
		}
		else
			imgTempP2.copyTo(ImgRst);

		cv::bitwise_and(ImgSrc, ImgRst, ImgRst);
		int nRST_CNT = cv::countNonZero(ImgRst);
		if (nRST_CNT <= N_GAP_ORG)
			continue;

		if (bBIN == false)
		{
			*dAValue = dA;
			*dBValue = dB;
			*pbX = bX;
		}
		return true;
	}
	return false;
}
bool PAD_PAT::CHECK_POLY_LINE(cv::Mat Img, bool bX, cv::Rect roi, float fPol)
{
	const uchar* puc = Img.data;
	int nW = Img.cols;
	int nH = Img.rows;
	int nA_T = bX ? nH : nW;
	int nA_S = bX ? roi.y : roi.x;
	int nA_E = bX ? roi.y + roi.height : roi.x + roi.width;
	int nB_T = bX ? nW : nH;
	int nB_S = bX ? roi.x : roi.y;
	int nB_E = bX ? roi.x + roi.width : roi.y + roi.height;

	float fUM = 1000.0f;
	float fRes = bX ? m_resolX : m_resolY;
	float fPOL_PIX = fPol / fUM / fRes;
	int nLINE_CNT = 0;
	std::vector<int> vecLine;
	for (int a = nA_S; a < nA_E; a++)
	{
		if (a < 0 || a >= nA_T)
			continue;

		int nWCNT = 0;
		for (int b = nB_S; b < nB_E; b++)
		{
			if (b < 0 || b >= nB_T)
				continue;
			int nX = bX ? b : a;
			int nY = bX ? a : b;
			int nIDX = (nY * nW) + nX;
			if (puc[nIDX] == 255)
				nWCNT++;
		}
		vecLine.push_back(nWCNT);
		if (fPOL_PIX - N_BIN_DIFF > nWCNT || fPOL_PIX + N_BIN_DIFF < nWCNT)
			continue;
		nLINE_CNT++;
	}
	if (nLINE_CNT <= N_BIN_DIFF)
		return false;

	float fPER = (float)nLINE_CNT / (float)nA_T;
	if (fPER < 0.5)
		return false;

	return true;
}
bool PAD_PAT::FIND_LINE_XY(cv::Mat Img, int* pnPOL)
{
	const uchar* puc = Img.data;
	cv::Rect roi = cv::boundingRect(Img);
	int nW = Img.cols;
	int nH = Img.rows;
	int nMAX[2] = { 0, 0 };
	int nMAX_CNT[2] = { 0, 0 };
	std::map<int, int> mapData[2];
	for (int n = 0; n < 2; n++)
	{
		mapData[n].clear();
		bool bX = n == 0;
		int nA_T = bX ? nH : nW;
		int nA_S = bX ? roi.y : roi.x;
		int nA_E = bX ? roi.y + roi.height : roi.x + roi.width;
		int nB_T = bX ? nW : nH;
		int nB_S = bX ? roi.x : roi.y;
		int nB_E = bX ? roi.x + roi.width : roi.y + roi.height;
		vector<int> vec;
		for (int a = nA_S; a < nA_E; a++)
		{
			if (a < 0 || a >= nA_T)
				continue;

			int nWCNT = 0;
			for (int b = nB_S; b < nB_E; b++)
			{
				if (b < 0 || b >= nB_T)
					continue;
				int nX = bX ? b : a;
				int nY = bX ? a : b;
				int nIDX = (nY * nW) + nX;
				if (puc[nIDX] == 255)
					nWCNT++;
			}
			vec.push_back(nWCNT);
		}
		int nMax_Temp = 0;
		int nMax_Cnt_Temp = 0;
		for (int i = 0; i < vec.size(); i++)
		{
			int nID = vec[i];
			auto it = mapData[n].find(nID);
			if (it != mapData[n].end())
				continue;
			int nCNT = 0;
			for (int j = 0; j < vec.size(); j++)
			{
				if (nID == vec[j])
					nCNT++;
			}
			if (nCNT <= 0)
				continue;
			mapData[n][nID] = nCNT;
			if (nCNT > nMax_Cnt_Temp)
			{
				nMax_Cnt_Temp = nCNT;
				nMax_Temp = nID;
			}
		}

		nMAX[n] = nMax_Temp;
		for (int i = -1; i <= 1; i++)
		{
			int nID = nMax_Temp + i;
			auto it = mapData[n].find(nID);
			if (it != mapData[n].end())
				nMAX_CNT[n] += mapData[n][nID];
		}
	}
	int nGAP_X = abs(nMAX_CNT[0] - roi.height);
	int nGAP_Y = abs(nMAX_CNT[1] - roi.width);
	double dGAP_X = (double)nMAX_CNT[0] / (double)roi.height;
	double dGAP_Y = (double)nMAX_CNT[1] / (double)roi.width;
	if (nGAP_Y == 0 || dGAP_Y > dGAP_X)
	{
		*pnPOL = nMAX[1];
		return false;
	}

	*pnPOL = nMAX[0];
	return true;
}
void PAD_PAT::END_LINE_REMOVE(cv::Mat ImgPoly, cv::Mat imgPOLY_BIN, cv::Mat imgBIN_BIN)
{
	int nW = ImgPoly.cols;
	int nH = ImgPoly.rows;
	cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTemp2(nH, nW, CV_8UC1, cv::Scalar(0));
	cv::bitwise_or(imgPOLY_BIN, imgBIN_BIN, imgTemp);
	cv::Rect roi = cv::boundingRect(imgTemp);
	int nOUT = PAT_INSP_MIN_SIZE * 2;
	if (roi.x > PAT_INSP_MIN_SIZE && roi.br().x < nW - PAT_INSP_MIN_SIZE &&
		roi.y > PAT_INSP_MIN_SIZE && roi.br().y < nH - PAT_INSP_MIN_SIZE)
		return;

	for (int nDir = 0; nDir < 4; nDir++)
	{
		bool bX = nDir > 1;
		int nA_T = bX ? nH : nW;
		int nA_S = bX ? roi.y : roi.x;
		int nA_E = bX ? roi.y + roi.height : roi.x + roi.width;
		int nB_T = bX ? nW : nH;
		int nB_S = bX ? roi.x : roi.y;
		int nB_E = bX ? roi.x + roi.width : roi.y + roi.height;
		bool bLT = nDir == 0 || nDir == 2;
		if (bLT)
		{
			if (bX)
			{
				if (roi.x > PAT_INSP_MIN_SIZE)
					continue;
			}
			else
			{
				if (roi.y > PAT_INSP_MIN_SIZE)
					continue;
			}
		}
		else
		{
			if (bX)
			{
				if (roi.br().x < nW - PAT_INSP_MIN_SIZE)
					continue;
			}
			else
			{
				if (roi.br().y < nH - PAT_INSP_MIN_SIZE)
					continue;
			}
		}
		for (int a = nA_S; a <= nA_E; a++)
		{
			if (a < 0 || a >= nA_T)
				continue;

			int nWCnt = 0;
			for (int b = 0; b <= nOUT; b++)
			{
				int nB = bLT ? b : nB_T - 1 - b;
				if (b < 0 || b >= nB_T)
					continue;
				int nIndex = bX ? (a * nW) + nB : (nB * nW) + a;
				if (imgTemp.data[nIndex] == 255)
					nWCnt++;
				if (nWCnt > PAT_INSP_MIN_SIZE)
					break;
			}
			if (nWCnt <= 0 || nWCnt > PAT_INSP_MIN_SIZE)
				continue;

			for (int b = 0; b <= nWCnt; b++)
			{
				int nB = bLT ? b : nB_T - 1 - b;
				if (b < 0 || b >= nB_T)
					continue;
				int nIndex = bX ? (a * nW) + nB : (nB * nW) + a;
				if (imgTemp.data[nIndex] == 255)
					imgTemp2.data[nIndex] = 255;
			}
		}
	}

	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(imgTemp2, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	for (int a = 0; a < allcontour.size(); a++)
	{
		cv::Rect roi = cv::boundingRect(allcontour[a]);
		if (roi.width > PAT_INSP_MIN_SIZE && roi.height > PAT_INSP_MIN_SIZE)
			continue;
		else if (roi.width > PAT_INSP_MIN_SIZE)
		{
			double dPer = roi.width / roi.height;
			if (dPer < 2)
				continue;
		}
		else if (roi.height > PAT_INSP_MIN_SIZE)
		{
			double dPer = roi.height / roi.width;
			if (dPer < 2)
				continue;
		}
		imgTemp.setTo(0);
		cv::drawContours(imgTemp, allcontour, a, cv::Scalar(255), cv::FILLED);
		cv::bitwise_and(imgTemp, imgTemp2, imgTemp);
		ImgPoly = ImgPoly - imgTemp;
		imgPOLY_BIN = imgPOLY_BIN - imgTemp;
		imgBIN_BIN = imgBIN_BIN - imgTemp;
	}
}
void PAD_PAT::CHANGE_ONE_IMG(cv::Mat ImgPoly, cv::Mat ImgPolyORG, cv::Mat ImgBIN, cv::Mat ImgBINORG, int nPOL, bool bX)
{
	int nW = ImgPoly.cols;
	int nH = ImgPoly.rows;
	cv::Rect roiP = cv::boundingRect(ImgPoly);
	cv::Rect roiB = cv::boundingRect(ImgBIN);
	cv::Rect roi = roiP | roiB;
	int nA_T = bX ? nH : nW;
	int nA_S = bX ? roi.y : roi.x;
	int nA_E = bX ? roi.y + roi.height : roi.x + roi.width;
	int nB_T = bX ? nW : nH;
	int nB_S = bX ? roi.x : roi.y;
	int nB_E = bX ? roi.x + roi.width : roi.y + roi.height;
	for (int a = nA_S; a < nA_E; a++)
	{
		if (a < 0 || a >= nA_T)
			continue;

		int nWCNT = 0;
		for (int b = nB_S; b < nB_E; b++)
		{
			if (b < 0 || b >= nB_T)
				continue;
			int nX = bX ? b : a;
			int nY = bX ? a : b;
			int nIDX = (nY * nW) + nX;
			if (ImgPoly.data[nIDX] == 255)
				nWCNT++;
		}
		if (nPOL - N_POLY_DIFF > nWCNT || nPOL + N_POLY_DIFF < nWCNT)
			continue;
		nA_S = a;
		break;
	}
	for (int a = nA_E; a >= nA_S; a--)
	{
		if (a < 0 || a >= nA_T)
			continue;

		int nWCNT = 0;
		for (int b = nB_S; b < nB_E; b++)
		{
			if (b < 0 || b >= nB_T)
				continue;
			int nX = bX ? b : a;
			int nY = bX ? a : b;
			int nIDX = (nY * nW) + nX;
			if (ImgPoly.data[nIDX] == 255)
				nWCNT++;
		}
		if (nPOL - N_POLY_DIFF > nWCNT || nPOL + N_POLY_DIFF < nWCNT)
			continue;
		nA_E = a;
		break;
	}
	cv::Rect roiA;
	int nA = nA_E - nA_S;
	if (bX)
		roiA = cv::Rect(roi.x, nA_S, roi.width, nA);
	else
		roiA = cv::Rect(nA_S, roi.y, nA, roi.height);
	ImgPoly.setTo(0);
	ImgBIN.setTo(0);
	ImgPolyORG(roiA).copyTo(ImgPoly(roiA));
	ImgBINORG(roiA).copyTo(ImgBIN(roiA));
}
void PAD_PAT::REMOVE_POLY_NOT_XY(cv::Mat Img, std::vector<std::vector<cv::Point>> arrPoly, int nIDX, bool bX, VOL_TH_DATA sDataP, int nPOLY_DIFF)
{
	cv::Rect roi = cv::boundingRect(arrPoly[nIDX]);
	int nW = Img.cols;
	int nH = Img.rows;
	int nA_T = bX ? nH : nW;
	int nA_S = bX ? roi.y : roi.x;
	int nA_E = bX ? roi.y + roi.height : roi.x + roi.width;
	int nB_T = bX ? nW : nH;
	int nB_S = bX ? roi.x : roi.y;
	int nB_E = bX ? roi.x + roi.width : roi.y + roi.height;

	std::vector<int> vecC;
	std::map<int, int> mapC;
	for (int i = 0; i < sDataP.m_nIDX; i++)
	{
		if (sDataP.m_nArrB[i] < nA_S || sDataP.m_nArrB[i] > nA_E)
			continue;
		if (sDataP.m_nArrS[i] < nB_S || sDataP.m_nArrE[i] > nB_E)
			continue;
		if (sDataP.m_nArrC[i] < nB_S || sDataP.m_nArrC[i] > nB_E)
			continue;

		vecC.push_back(sDataP.m_nArrC[i]);
		if (mapC.find(sDataP.m_nArrB[i]) == mapC.end())
			mapC.insert(std::pair<int, int>(sDataP.m_nArrB[i], sDataP.m_nArrC[i]));
	}
	if (vecC.empty())
		return;

	std::sort(vecC.begin(), vecC.end());
	int nMostFrequentElement = vecC[0];
	int nMaxFrequency = 1;
	int nCurrentFrequency = 1;
	for (int i = 1; i < vecC.size(); ++i)
	{
		if (vecC[i] == vecC[i - 1])
		{
			nCurrentFrequency++;
		}
		else
		{
			if (nCurrentFrequency > nMaxFrequency)
			{
				nMaxFrequency = nCurrentFrequency;
				nMostFrequentElement = vecC[i - 1];
			}
			nCurrentFrequency = 1;
		}
	}
	if (nCurrentFrequency > nMaxFrequency)
	{
		nMaxFrequency = nCurrentFrequency;
		nMostFrequentElement = vecC.back();
	}
	if (nMaxFrequency < (nPOLY_DIFF * 4))
		return;

	for (int a = nA_S; a < nA_E; a++)
	{
		if (a < 0 || a >= nA_T)
			continue;
		if (mapC.find(a) == mapC.end())
			continue;
		if (mapC[a] < nMostFrequentElement - nPOLY_DIFF || mapC[a] > nMostFrequentElement + nPOLY_DIFF)
		{
			for (int b = nB_S; b < nB_E; b++)
			{
				if (b < 0 || b >= nB_T)
					continue;
				int nX = bX ? b : a;
				int nY = bX ? a : b;
				int nIndex = (nY * nW) + nX;
				if (Img.data[nIndex] == 255)
					Img.data[nIndex] = 0;
			}
		}
	}
}
int PAD_PAT::PAT_TH_L(VOL_TH_RE sRE, float fPol, bool bX, cv::Mat imgBIN_BIN, cv::Mat imgBin_Temp, cv::Rect roiB, RECT_F* roiMAX, RECT_F* roiMIN)
{
	roiMAX->Init();
	roiMIN->Init();
	bool bRET_MISSING = false;
	bool bRET_MIN = false;
	bool bRET_MAX = false;
	RECT_F roiMAX_TEMP_F;
	RECT_F roiMIN_TEMP_F;
	roiMAX_TEMP_F.Init();
	roiMIN_TEMP_F.Init();
	for (int c = 0; c < 2; c++)
	{
		bool bWA = c == 1;
		RECT_F roiMAX_TEMP;
		RECT_F roiMIN_TEMP;
		roiMAX_TEMP.Init();
		roiMIN_TEMP.Init();
		int nRET_TEMP = 0;
		if (bWA)
			nRET_TEMP = FIND_TH_WA(sRE, imgBIN_BIN, bX, 0, 0, *roiMAX, *roiMIN, &roiMAX_TEMP, &roiMIN_TEMP, &roiMAX_TEMP_F, &roiMIN_TEMP_F);
		else
			nRET_TEMP = FIND_TH(sRE, roiB, imgBin_Temp, imgBIN_BIN, bX, &roiMAX_TEMP, &roiMIN_TEMP);
		bool bMissing = (nRET_TEMP & m_eVOL_TH_FAIL_MISSING) == m_eVOL_TH_FAIL_MISSING;
		bool bRET_TEMP_MIN = (nRET_TEMP & m_eVOL_TH_FAIL_MIN) == m_eVOL_TH_FAIL_MIN;
		bool bRET_TEMP_MAX = (nRET_TEMP & m_eVOL_TH_FAIL_MAX) == m_eVOL_TH_FAIL_MAX;
		bool bTEMP_MAX = (c == 0);
		bool bTEMP_MIN = (c == 0);
		if (c == 0)
		{
			bRET_MISSING = bMissing;
			if (bRET_TEMP_MIN && bRET_TEMP_MAX)
			{
				bRET_MAX = true;
				bRET_MIN = true;
				break;
			}
		}
		else
		{
			bTEMP_MAX = roiMAX_TEMP.COMPARE_VALUE(bRET_MAX, bRET_TEMP_MAX, *roiMAX, m_resolX, m_resolY, fPol, fPol);
			if (bMissing == false)
				bTEMP_MIN = roiMIN_TEMP.COMPARE_VALUE(bRET_MIN, bRET_TEMP_MIN, *roiMIN, m_resolX, m_resolY, fPol, fPol);
			if (bRET_MISSING == false && bMissing)
				bTEMP_MIN = true;
		}

		if (bTEMP_MAX)
		{
			roiMAX->SetData(roiMAX_TEMP);
			bRET_MAX = (nRET_TEMP & m_eVOL_TH_FAIL_MAX) == m_eVOL_TH_FAIL_MAX;
		}
		if (bTEMP_MIN)
		{
			roiMIN->SetData(roiMIN_TEMP);
			bRET_MIN = (nRET_TEMP & m_eVOL_TH_FAIL_MIN) == m_eVOL_TH_FAIL_MIN;
		}
	}

	int nRET = 0;
	if (bRET_MISSING) nRET += m_eVOL_TH_FAIL_MISSING;
	if (bRET_MIN) nRET += m_eVOL_TH_FAIL_MIN;
	if (bRET_MAX) nRET += m_eVOL_TH_FAIL_MAX;
	return nRET;
}
#pragma endregion

#pragma region TH
BOOL PAD_PAT::PAT_Thickness(WndAlgoImg &sWndAlgoImg, AlgoVolume* pInspAlgoVolume, cv::Mat ImgPoly, cv::Mat ImgBin, RstAlgoVolume* sRstAlgo, bool bX, int nTYPE_P)
{
	BOOL bResult = TRUE;
	int nLine = __LINE__;
	try
	{
		DWORD st = GetTickCount();
		if (sRstAlgo == nullptr || sRstAlgo == NULL)
			return FALSE;
		if (pInspAlgoVolume->UseData(m_eVolumeData_PAD_PAT_Check) == false)
			return bResult;

		float fUM = 1000.0f;
		float fPAT_Min = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_TH_D_MIN);
		float fPAT_Max = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_TH_D_MAX);
		if (fPAT_Min <= 0 && fPAT_Max <= 0)
			return bResult;

		float fTH_Min = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_TH_Min) / fUM;
		float fTH_Max = pInspAlgoVolume->GetData_M(m_ePAT_ALGO_TH_Max) / fUM;
		if (fTH_Min <= 0 && fTH_Max <= 0)
			return bResult;

		nLine = __LINE__;
		UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
		float *pfImgSrc = sWndAlgoImg.m_fArr3D;
		int nW = sWndAlgoImg.m_nWidth;
		int nH = sWndAlgoImg.m_nHeight;
		if (pucImgSrc == NULL || pfImgSrc == NULL || nW <= 0 || nH <= 0)
			return bResult;

		nLine = __LINE__;
		std::vector<std::vector<cv::Point>> arrPoly;
		std::vector<std::vector<cv::Point>> arrBin;
		std::vector<cv::Vec4i> hierarchy;
		int nMIN = 0;
		int nMAX = 0;
#if _DEBUG
		cv::Mat imgSrc2D(nH, nW, CV_8UC1, pucImgSrc);
		cv::Mat imgSrc3D(nH, nW, CV_32FC1, pfImgSrc);
#endif
		cv::Mat imgBIN_BIN(nH, nW, CV_8UC1, cv::Scalar(0));
		if (pInspAlgoVolume->UseData_M(m_ePAT_ALGO_N_DATA_FILL))
		{
			arrBin.clear();
			hierarchy.clear();
			cv::findContours(ImgBin, arrBin, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			for (int b = 0; b < arrBin.size(); b++)
				cv::drawContours(imgBIN_BIN, arrBin, b, cv::Scalar(255), cv::FILLED);
		}
		else
		{
			ImgBin.copyTo(imgBIN_BIN);
		}
		SaveWorkImg(ImgPoly.data, nW, nH, _T("VOL_PAD_TH_POLY.bmp"));
		SaveWorkImg(imgBIN_BIN.data, nW, nH, _T("VOL_PAD_TH_BIN.bmp"));

		cv::Rect roiP2 = cv::boundingRect(ImgPoly);
		if (N_INSP_OUT > roiP2.width && N_INSP_OUT > roiP2.height)
			return bResult;

		VOL_TH_DATA sDataP;
		VOL_TH_DATA sDataB;
		sDataP.SetData(nW, nH);
		sDataB.SetData(nW, nH);
		int nExceptArea = 2;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(nExceptArea, nExceptArea));
		cv::Mat imgPoly_Temp(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgBin_Temp(nH, nW, CV_8UC1, cv::Scalar(0));
		VOL_TH_RE sRE;
		sRE.SetData(sWndAlgoImg, pInspAlgoVolume);
		for (int nRot = 0; nRot < 2; nRot++)
		{
			if (nTYPE_P <= m_ePAD_PARALLEL_None)
			{
				if (nRot == 0)
					g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_TH_SET] += ((GetTickCount() - st) / 1000.0f);
				else
					g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_TH_1] += ((GetTickCount() - st) / 1000.0f);
			}

			if (nRot > 0)
			{
				bX = !bX;
				sDataP.ClearValue();
				sDataB.ClearValue();
			}

			bool bFIND_POS = FIND_POS2(imgBIN_BIN, ImgPoly, bX, fPAT_Min, fPAT_Max, &sDataP, &sDataB);
			if (bFIND_POS == false)
				continue;

			double dRes = bX ? m_resolX : m_resolY;
			float fPAT_Min_pix = fPAT_Min / fUM / dRes;
			float fPAT_Max_pix = fPAT_Max / fUM / dRes;
			int nBIN_GAP_DIFF = fTH_Max / dRes - 2;
			if (nBIN_GAP_DIFF < 5) nBIN_GAP_DIFF = 5;
			if (sDataP.m_nIDX <= N_POLY_DIFF || sDataB.m_nIDX <= N_POLY_DIFF ||
				sDataP.m_nMAX_SZ == 0)
				continue;

			std::vector<int> vecMAX = sDataP.GetMAX_LIST(fPAT_Min_pix, N_POLY_DIFF);
			for (int a = 0; a < vecMAX.size(); a++)
			{
				int nMAX_GAP = vecMAX[a];
				if (nMAX_GAP <= 0)
					break;

				imgPoly_Temp.setTo(0);
				sDataP.Set_IMG(ImgPoly, imgPoly_Temp, nMAX_GAP, N_POLY_DIFF, bX);
				arrPoly.clear();
				hierarchy.clear();
				cv::findContours(imgPoly_Temp, arrPoly, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
				if (arrPoly.size() == 0)
					continue;

				for (int b = 0; b < arrPoly.size(); b++)
				{
#if _DEBUG
					cv::Mat imgDEBUG(nH, nW, CV_8UC1, cv::Scalar(0));
					cv::drawContours(imgDEBUG, arrPoly, b, cv::Scalar(255), cv::FILLED);
#endif
					cv::Rect roiP = cv::boundingRect(arrPoly[b]);
					if (roiP.width < fPAT_Min_pix || roiP.height < fPAT_Min_pix)
						continue;

					int nCHECK_POLY_BIN = CHECK_POLY_BIN(imgPoly_Temp, roiP, bX, N_POLY_DIFF);
					bool bAB = nCHECK_POLY_BIN < 0;

					if ((bX && roiP.width >= fPAT_Max_pix) || (bX == false && roiP.height >= fPAT_Max_pix))
					{
						if (nCHECK_POLY_BIN > 0)
							continue;
					}

					if (nRot > 0)
					{
						double dWHPer = bX ? (double)roiP.width / (double)roiP.height : (double)roiP.height / (double)roiP.width;
						if (dWHPer > D_WH_PER) // W, H 비율이 어느정도 차이나면 한 방향만 검사
							continue;
					}

					imgBin_Temp.setTo(0);
					int nPol = 0;
					cv::Rect roiB = sDataB.Set_IMG(imgBIN_BIN, imgBin_Temp, nMAX_GAP, nBIN_GAP_DIFF, bX, roiP, &nPol);
					if (roiB.width <= N_BIN_MIN || roiB.height <= N_BIN_MIN)
						continue;
					if (roiB.width >= nW && roiB.height <= N_INSP_LINE_OUT)
					{
						if (bX)
							continue;
					}
					if (roiB.height >= nH && roiB.width <= N_INSP_LINE_OUT)
					{
						if (bX == false)
							continue;
					}
					if (nPol <= N_BIN_MIN)
						continue;

					if (CHK_BIN_POLY_XOR(imgBin_Temp, imgPoly_Temp, kernel, roiB, roiP) == false)
					{
						continue;
					}

					RECT_F roiMAX;
					RECT_F roiMIN;
					roiMAX.Init();
					roiMIN.Init();
					float fPol = nMAX_GAP * dRes * fUM;
					int nRET_PAT_TH = PAT_TH(sRE, fPol, bX, imgBin_Temp, roiB, &roiMAX, &roiMIN, bAB);
					bool bRET_MISSING = (nRET_PAT_TH & m_eVOL_TH_FAIL_MISSING) == m_eVOL_TH_FAIL_MISSING;
					bool bRET_MIN = (nRET_PAT_TH & m_eVOL_TH_FAIL_MIN) == m_eVOL_TH_FAIL_MIN;
					bool bRET_MAX = (nRET_PAT_TH & m_eVOL_TH_FAIL_MAX) == m_eVOL_TH_FAIL_MAX;
					float fMAX_GAP = roiMAX.CALC_DIST(m_resolX, m_resolY) * fUM;
					float fMIN_GAP = roiMIN.CALC_DIST(m_resolX, m_resolY) * fUM;
					if ((bRET_MIN == false && fPol > fMIN_GAP) || bRET_MISSING)
					{
						float fGap2 = (bRET_MISSING) ? fPol : fPol - fMIN_GAP;
						if (bRET_MISSING == false && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN])
						{
							float fPolTemp = CHECK_LINE(ImgPoly, roiMIN, bX);
							if (fPolTemp > fMIN_GAP && fPAT_Min <= fPolTemp && fPAT_Max >= fPolTemp)
							{
								float fGap3 = fPolTemp - fMIN_GAP;
								if (fGap2 > fGap3)
									fGap2 = fGap3;
							}
						}

						if (fGap2 > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN])
						{
							sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN] = fGap2;
							sRstAlgo->m_rcRect_TH[0].left = roiMIN.left;
							sRstAlgo->m_rcRect_TH[0].top = roiMIN.top;
							sRstAlgo->m_rcRect_TH[0].right = roiMIN.right;
							sRstAlgo->m_rcRect_TH[0].bottom = roiMIN.bottom;
							nMIN++;
						}
					}
					if (bRET_MAX == false && fPol < fMAX_GAP)
					{
						float fGap2 = fMAX_GAP - fPol;
						if (fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX])
						{
							float fPolTemp = CHECK_LINE(ImgPoly, roiMAX, bX);
							if (fMAX_GAP > fPolTemp && fPAT_Min <= fPolTemp && fPAT_Max >= fPolTemp)
							{
								float fGap3 = fMAX_GAP - fPolTemp;
								if (fGap2 > fGap3)
									fGap2 = fGap3;
							}
						}

						if (fGap2 > 0 && fGap2 > sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX])
						{
							sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX] = fGap2;
							sRstAlgo->m_rcRect_TH[1].left = roiMAX.left;
							sRstAlgo->m_rcRect_TH[1].top = roiMAX.top;
							sRstAlgo->m_rcRect_TH[1].right = roiMAX.right;
							sRstAlgo->m_rcRect_TH[1].bottom = roiMAX.bottom;
							nMAX++;
						}
					}
				}
			}
		}
		if (nTYPE_P <= m_ePAD_PARALLEL_None)
			g_pInspMng->m_fVolTact[m_eVOL_TACT_PAT_TH_2] += ((GetTickCount() - st) / 1000.0f);
		nLine = __LINE__;
		bResult = TRUE;
		if (fTH_Min > 0)
		{
			sRstAlgo->m_nPadPatternCheck |= m_eVOL_PAT_CHK_TH_IN;
			if (nMIN > 0 && sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MIN] > fTH_Min * fUM)
			{
				bResult = FALSE;
				sRstAlgo->m_nOKPadPatternCheck |= m_eVOL_PAT_CHK_TH_IN;
			}
		}
		if (fTH_Max > 0)
		{
			sRstAlgo->m_nPadPatternCheck |= m_eVOL_PAT_CHK_TH_OUT;
			if (nMAX > 0 && sRstAlgo->m_fArr[m_eVolumeR_Rst_TH_MAX] > fTH_Max * fUM)
			{
				bResult = FALSE;
				sRstAlgo->m_nOKPadPatternCheck |= m_eVOL_PAT_CHK_TH_OUT;
			}
		}
		sDataP.ClearData();
		sDataB.ClearData();
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("PAD_PAT::PAT_Thickness(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
	return bResult;
}
bool PAD_PAT::CHK_BIN_POLY_XOR(cv::Mat imgBIN, cv::Mat imgPOLY, cv::Mat kernel, cv::Rect roiB, cv::Rect roiP)
{
	if (imgBIN.cols != imgPOLY.cols || imgBIN.rows != imgPOLY.rows)
		return true;

	int nROIW = roiB.width > roiP.width ? roiB.width : roiP.width;
	int nROIH = roiB.height > roiP.height ? roiB.height : roiP.height;
	int nL = roiB.x;
	int nR = roiB.x + nROIW;
	int nT = roiB.y;
	int nB = roiB.y + nROIH;
	if (nL < 0 || nR <= 0 ||
		nT < 0 || nB <= 0)
		return true;
	if (nL >= imgBIN.cols || nR >= imgBIN.cols ||
		nT >= imgBIN.rows || nB >= imgBIN.rows)
		return true;

	cv::Rect roi(nL, nT, nR - nL, nB - nT);
	cv::Mat imgB = imgBIN(roi);

	nL = roiP.x;
	nR = roiP.x + nROIW;
	nT = roiP.y;
	nB = roiP.y + nROIH;
	if (nL < 0 || nR <= 0 ||
		nT < 0 || nB <= 0)
		return true;
	if (nL >= imgBIN.cols || nR >= imgBIN.cols ||
		nT >= imgBIN.rows || nB >= imgBIN.rows)
		return true;

	roi = cv::Rect(nL, nT, nR - nL, nB - nT);
	cv::Mat imgP = imgPOLY(roi);

	cv::Mat imgXOR;
	cv::bitwise_xor(imgB, imgP, imgXOR);
	double dXOR = cv::countNonZero(imgXOR);
	if (dXOR <= 2)
		return false;

	cv::Rect roiXOR = cv::boundingRect(imgXOR);
	if (roiXOR.width <= 1 || roiXOR.height <= 1)
		return false;

	cv::Mat imgErode;
	cv::erode(imgXOR, imgErode, kernel);
	double dErode = cv::countNonZero(imgErode);
	if (dErode <= 5)
		return false;

	return true;
}
float PAD_PAT::CHECK_LINE(cv::Mat ImgPoly, RECT_F rcROI, bool bX)
{
	float fPOL = 0;
	int nW = ImgPoly.cols;
	int nH = ImgPoly.rows;
	int nOUT = 1;
	if (rcROI.left <= nOUT || rcROI.top <= nOUT || rcROI.right <= nOUT || rcROI.bottom <= nOUT)
		return fPOL;
	if (rcROI.left >= nW - nOUT || rcROI.top >= nH - nOUT || rcROI.right >= nW - nOUT || rcROI.bottom >= nH - nOUT)
		return fPOL;
	if ((bX && rcROI.top != rcROI.bottom) || (bX == false && rcROI.left != rcROI.right))
		return fPOL;

	int nFind = PAT_INSP_MIN_SIZE;
	int nS = -1;
	int nE = -1;
	int nA_T = bX ? nW : nH;
	int nA_S = bX ? rcROI.left : rcROI.top;
	int nA_E = bX ? rcROI.right : rcROI.bottom;
	int nB = bX ? rcROI.top : rcROI.left;
	nA_S += nFind;
	nA_E -= nFind;
	for (int a = nA_S; a >= 0; a--)
	{
		if (a < 0 || a >= nA_T)
			continue;
		int nIDX = bX ? (nB * nW) + a : (a * nW) + nB;
		if (ImgPoly.data[nIDX] == 255)
		{
			if (nS == -1 || nS > a)
				nS = a;
		}
		else
			break;
	}
	for (int a = nA_E; a < nA_T; a++)
	{
		if (a < 0 || a >= nA_T)
			continue;
		int nIDX = bX ? (nB * nW) + a : (a * nW) + nB;
		if (ImgPoly.data[nIDX] == 255)
		{
			if (nE == -1 || nE < a)
				nE = a;
		}
		else
			break;
	}
	if (nS < 0 || nE < 0 || nS >= nE)
		return fPOL;

	float fUM = 1000.0f;
	double dRes = bX ? m_resolX : m_resolY;
	fPOL = (nE - nS + 1) * dRes * fUM;
	return fPOL;
}
int PAD_PAT::PAT_TH(VOL_TH_RE sRE, float fPol, bool bX, cv::Mat imgBin_Temp, cv::Rect roiB, RECT_F* roiMAX, RECT_F* roiMIN, bool bAB)
{
	roiMAX->Init();
	roiMIN->Init();

	RECT_F roiMAX_TEMP;
	RECT_F roiMIN_TEMP;
	roiMAX_TEMP.Init();
	roiMIN_TEMP.Init();
	int nRET_TEMP = FIND_TH2(roiB, imgBin_Temp, bX, &roiMAX_TEMP, &roiMIN_TEMP, bAB);
	bool bRET_MISSING = (nRET_TEMP & m_eVOL_TH_FAIL_MISSING) == m_eVOL_TH_FAIL_MISSING;
	bool bRET_MIN = (nRET_TEMP & m_eVOL_TH_FAIL_MIN) == m_eVOL_TH_FAIL_MIN;
	bool bRET_MAX = (nRET_TEMP & m_eVOL_TH_FAIL_MAX) == m_eVOL_TH_FAIL_MAX;

	roiMAX->SetData(roiMAX_TEMP);
	roiMIN->SetData(roiMIN_TEMP);

	int nRET = 0;
	if (bRET_MISSING) nRET += m_eVOL_TH_FAIL_MISSING;
	if (bRET_MIN) nRET += m_eVOL_TH_FAIL_MIN;
	if (bRET_MAX) nRET += m_eVOL_TH_FAIL_MAX;
	return nRET;
}
int PAD_PAT::FIND_TH2(cv::Rect roi, cv::Mat img, bool bX, RECT_F* pMAX, RECT_F* pMIN, bool bAB)
{
	int nRet = 0;
	int nW = img.cols;
	int nROIA = img.cols;
	int nROIB = img.rows;
	int nGAP = ((double)roi.height * 0.08 + 0.5);
	if (nGAP < N_BIN_MIN) nGAP = N_BIN_MIN;
	int nROIA_S = roi.x;
	int nROIA_E = roi.x + roi.width - 1;
	int nROIB_S = roi.y + nGAP;
	int nROIB_E = roi.y + roi.height - 1 - nGAP;
	if (bX == false)
	{
		nROIA = img.rows;
		nROIB = img.cols;
		nGAP = ((double)roi.width * 0.008 + 0.5);
		if (nGAP < N_BIN_MIN) nGAP = N_BIN_MIN;
		nROIA_S = roi.y;
		nROIA_E = roi.y + roi.height - 1;
		nROIB_S = roi.x + nGAP;
		nROIB_E = roi.x + roi.width - 1 - nGAP;
	}
	if (nROIB_S >= nROIB_E)
	{
		nROIB_S = (nROIB_E + nROIB_S) / 2;
		nROIB_E = nROIB_S + 1;
		
		bool bPol = false;
		for (int nB = nROIB_S; nB < nROIB_E; nB++)
		{
			if (nB < 0 || nB >= nROIB)
				continue;
			for (int nA = nROIA_S; nA < nROIA_E; nA++)
			{
				if (nA < 0 || nA >= nROIA)
					continue;
				int nIndex = bX ? (nB * nW) + nA : (nA * nW) + nB;
				if (img.data[nIndex] == 255)
				{
					bPol = true;
					break;
				}
			}
		}
		if (bPol == false)
		{
			int nGAP2 = N_BIN_DIFF;
			nROIB_S = roi.x + nGAP2;
			nROIB_E = roi.x + roi.width - 1 - nGAP2;
		}
	}
	const uchar* puc = img.data;
	int nMin = -1;
	int nMinB = -1;
	int nMinS = -1;
	int nMinE = -1;
	int nMax = -1;
	int nMaxB = -1;
	int nMaxS = -1;
	int nMaxE = -1;
	int nPol = 0;
	for (int nB = nROIB_S; nB < nROIB_E; nB++)
	{
		if (nB < 0 || nB >= nROIB)
			continue;

		int nWhite = 0;
		int nWhiteS = -1;
		int nWhiteE = -1;
		int nWhiteS_PRE = -1;
		int nWhiteE_PRE = -1;
		for (int nA = nROIA_S; nA < nROIA_E; nA++)
		{
			if (nA < 0 || nA >= nROIA)
				continue;
			int nIndex = bX ? (nB * nW) + nA : (nA * nW) + nB;
			if (puc[nIndex] == 255)
			{
				nWhite++;
				if (nWhiteS_PRE == -1) nWhiteS_PRE = nA;
				if (nWhiteE_PRE < nA) nWhiteE_PRE = nA;
			}
			else
			{
				if (nWhiteS_PRE < 0 || nWhiteE_PRE < 0)
					continue;
				if (nWhiteS == -1 || nWhiteE == -1)
				{
					nWhiteS = nWhiteS_PRE;
					nWhiteE = nWhiteE_PRE;
				}
				else
				{
					int nW_R = nWhiteE - nWhiteS;
					int nW_S = nWhiteE_PRE - nWhiteS_PRE;
					if (nW_S > nW_R)
					{
						nWhiteS = nWhiteS_PRE;
						nWhiteE = nWhiteE_PRE;
					}
				}
				nWhiteS_PRE = -1;
				nWhiteE_PRE = -1;
			}
		}
		if (nWhiteS_PRE >= 0 && nWhiteE_PRE >= 0)
		{
			if (nWhiteS == -1 || nWhiteE == -1)
			{
				nWhiteS = nWhiteS_PRE;
				nWhiteE = nWhiteE_PRE;
			}
			else
			{
				int nW_R = nWhiteE - nWhiteS;
				int nW_S = nWhiteE_PRE - nWhiteS_PRE;
				if (nW_S > nW_R)
				{
					nWhiteS = nWhiteS_PRE;
					nWhiteE = nWhiteE_PRE;
				}
			}
		}

		if (nWhite > 0)
			nPol++;
		if (nWhite > 0 && (nMin == -1 || nMin > nWhite))
		{
			nMin = nWhite;
			nMinB = nB;
			nMinS = nWhiteS;
			nMinE = nWhiteE;
		}
		if (nWhite > 0 && (nMax == -1 || nMax < nWhite))
		{
			nMax = nWhite;
			nMaxB = nB;
			nMaxS = nWhiteS;
			nMaxE = nWhiteE;
		}
	}
	if (nPol == 0)
	{
		for (int nB = nROIB_S; nB >= 0; nB--)
		{
			if (nB < 0 || nB >= nROIB)
				continue;

			int nWhite = 0;
			int nWhiteS = -1;
			int nWhiteE = -1;
			int nWhiteS_PRE = -1;
			int nWhiteE_PRE = -1;
			for (int nA = nROIA_S; nA < nROIA_E; nA++)
			{
				if (nA < 0 || nA >= nROIA)
					continue;
				int nIndex = bX ? (nB * nW) + nA : (nA * nW) + nB;
				if (puc[nIndex] == 255)
				{
					nWhite++;
					if (nWhiteS_PRE == -1) nWhiteS_PRE = nA;
					if (nWhiteE_PRE < nA) nWhiteE_PRE = nA;
				}
				else
				{
					if (nWhiteS_PRE < 0 || nWhiteE_PRE < 0)
						continue;
					if (nWhiteS == -1 || nWhiteE == -1)
					{
						nWhiteS = nWhiteS_PRE;
						nWhiteE = nWhiteE_PRE;
					}
					else
					{
						int nW_R = nWhiteE - nWhiteS;
						int nW_S = nWhiteE_PRE - nWhiteS_PRE;
						if (nW_S > nW_R)
						{
							nWhiteS = nWhiteS_PRE;
							nWhiteE = nWhiteE_PRE;
						}
					}
					nWhiteS_PRE = -1;
					nWhiteE_PRE = -1;
				}
			}
			if (nWhiteS_PRE >= 0 && nWhiteE_PRE >= 0)
			{
				if (nWhiteS == -1 || nWhiteE == -1)
				{
					nWhiteS = nWhiteS_PRE;
					nWhiteE = nWhiteE_PRE;
				}
				else
				{
					int nW_R = nWhiteE - nWhiteS;
					int nW_S = nWhiteE_PRE - nWhiteS_PRE;
					if (nW_S > nW_R)
					{
						nWhiteS = nWhiteS_PRE;
						nWhiteE = nWhiteE_PRE;
					}
				}
			}

			if (nWhite > 0)
				nPol++;
			if (nWhite > 0 && (nMin == -1 || nMin > nWhite))
			{
				nMin = nWhite;
				nMinB = nB;
				nMinS = nWhiteS;
				nMinE = nWhiteE;
			}
			if (nWhite > 0 && (nMax == -1 || nMax < nWhite))
			{
				nMax = nWhite;
				nMaxB = nB;
				nMaxS = nWhiteS;
				nMaxE = nWhiteE;
			}
		}

		for (int nB = nROIB_E; nB < nROIB; nB++)
		{
			if (nB < 0 || nB >= nROIB)
				continue;

			int nWhite = 0;
			int nWhiteS = -1;
			int nWhiteE = -1;
			int nWhiteS_PRE = -1;
			int nWhiteE_PRE = -1;
			for (int nA = nROIA_S; nA < nROIA_E; nA++)
			{
				if (nA < 0 || nA >= nROIA)
					continue;
				int nIndex = bX ? (nB * nW) + nA : (nA * nW) + nB;
				if (puc[nIndex] == 255)
				{
					nWhite++;
					if (nWhiteS_PRE == -1) nWhiteS_PRE = nA;
					if (nWhiteE_PRE < nA) nWhiteE_PRE = nA;
				}
				else
				{
					if (nWhiteS_PRE < 0 || nWhiteE_PRE < 0)
						continue;
					if (nWhiteS == -1 || nWhiteE == -1)
					{
						nWhiteS = nWhiteS_PRE;
						nWhiteE = nWhiteE_PRE;
					}
					else
					{
						int nW_R = nWhiteE - nWhiteS;
						int nW_S = nWhiteE_PRE - nWhiteS_PRE;
						if (nW_S > nW_R)
						{
							nWhiteS = nWhiteS_PRE;
							nWhiteE = nWhiteE_PRE;
						}
					}
					nWhiteS_PRE = -1;
					nWhiteE_PRE = -1;
				}
			}
			if (nWhiteS_PRE >= 0 && nWhiteE_PRE >= 0)
			{
				if (nWhiteS == -1 || nWhiteE == -1)
				{
					nWhiteS = nWhiteS_PRE;
					nWhiteE = nWhiteE_PRE;
				}
				else
				{
					int nW_R = nWhiteE - nWhiteS;
					int nW_S = nWhiteE_PRE - nWhiteS_PRE;
					if (nW_S > nW_R)
					{
						nWhiteS = nWhiteS_PRE;
						nWhiteE = nWhiteE_PRE;
					}
				}
			}

			if (nWhite > 0)
				nPol++;
			if (nWhite > 0 && (nMin == -1 || nMin > nWhite))
			{
				nMin = nWhite;
				nMinB = nB;
				nMinS = nWhiteS;
				nMinE = nWhiteE;
			}
			if (nWhite > 0 && (nMax == -1 || nMax < nWhite))
			{
				nMax = nWhite;
				nMaxB = nB;
				nMaxS = nWhiteS;
				nMaxE = nWhiteE;
			}
		}

		if (nPol == 0)
		{
			nMinS = nROIA_S;
			nMinE = nROIA_E;
			nRet |= m_eVOL_TH_FAIL_MISSING;
		}
	}
	if (nMin == -1)
		nRet |= m_eVOL_TH_FAIL_MIN;
	if (nMax == -1)
		nRet |= m_eVOL_TH_FAIL_MAX;
	if (bX)
	{
		if (nMax >= 0)
		{
			pMAX->SetROI(nMaxS, nMaxE, nMaxB, nMaxB);
		}
		if (nMin >= 0)
		{
			pMIN->SetROI(nMinS, nMinE, nMinB, nMinB);
		}
	}
	else
	{
		if (nMax >= 0)
		{
			pMAX->SetROI(nMaxB, nMaxB, nMaxS, nMaxE);
		}
		if (nMin >= 0)
		{
			pMIN->SetROI(nMinB, nMinB, nMinS, nMinE);
		}
	}
	return nRet;
}
bool PAD_PAT::FIND_POS2(cv::Mat imgBIN_BIN, cv::Mat ImgPoly, bool bX, float fPAT_Min, float fPAT_Max, VOL_TH_DATA* sDataP, VOL_TH_DATA* sDataB)
{
	int nW = ImgPoly.cols;
	int nH = ImgPoly.rows;
	cv::Rect roiPOLY = cv::boundingRect(ImgPoly);
	cv::Rect roiBIN = cv::boundingRect(imgBIN_BIN);
	int nAll_L = std::min(roiPOLY.x, roiBIN.x) - N_POLY_DIFF;
	int nAll_T = std::min(roiPOLY.y, roiBIN.y) - N_POLY_DIFF;
	int nAll_R = std::max(roiPOLY.x + roiPOLY.width, roiBIN.x + roiBIN.width) + N_POLY_DIFF;
	int nAll_B = std::max(roiPOLY.y + roiPOLY.height, roiBIN.y + roiBIN.height) + N_POLY_DIFF;
	int nOUT = cv::countNonZero(ImgPoly);
	if (nOUT <= 2)
		return false;

	const int FIND_CNT = 2;
	float fUM = 1000.0f;
	double dRes = m_resolX;
	int nROIA = nW;
	int nROIA_S = nAll_L;
	int nROIA_E = nAll_R;
	int nROIB = nH;
	int nROIB_S = nAll_T;
	int nROIB_E = nAll_B;
	if (bX == false)
	{
		dRes = m_resolY;
		nROIA = nH;
		nROIA_S = nAll_T;
		nROIA_E = nAll_B;
		nROIB = nW;
		nROIB_S = nAll_L;
		nROIB_E = nAll_R;
	}
	double dPer = (double)nROIB / (double)nROIA;
	float fPAT_Min_pix = fPAT_Min / fUM / dRes;
	float fPAT_Max_pix = fPAT_Max / fUM / dRes;
	const uchar* polyData = ImgPoly.data;
	const uchar* binData = imgBIN_BIN.data;
	for (int nB = nROIB_S; nB <= nROIB_E; nB++)
	{
		if (nB < 0 || nB >= nROIB)
			continue;
		bool bFind[FIND_CNT] = { false, false };
		int nS[FIND_CNT] = { 0, 0 };
		int nE[FIND_CNT] = { 0, 0 };
		for (int nA = nROIA_S; nA <= nROIA_E; nA++)
		{
			if (nA < 0 || nA >= nROIA)
				continue;
			for (int a = 0; a < FIND_CNT; a++)
			{
				int nIndex = bX ? (nB * nW) + nA : (nA * nW) + nB;
				const UCHAR* pucBIN = a == 0 ? polyData : binData;
				if (pucBIN[nIndex] == 255)
				{
					if (bFind[a] == false)
					{
						bFind[a] = true;
						nS[a] = nA;
						nE[a] = nA;
					}
					else
					{
						if (nS[a] > nA) nS[a] = nA;
						if (nE[a] < nA) nE[a] = nA;
					}
				}
				else
				{
					if (bFind[a] == false)
						continue;

					int nGAP = nE[a] - nS[a] + 1;
					if (nGAP > 1 && fPAT_Min_pix <= nGAP && fPAT_Max_pix >= nGAP)
					{
						if (a == 0)
							sDataP->AddData(nB, nS[a], nE[a], false);
						else
							sDataB->AddData(nB, nS[a], nE[a], false);
					}
					bFind[a] = false;
					nS[a] = 0;
					nE[a] = 0;
				}
			}
		}
		for (int a = 0; a < FIND_CNT; a++)
		{
			if (bFind[a] == false)
				continue;

			const UCHAR* pucBIN = a == 0 ? polyData : binData;
			int nGAP = nE[a] - nS[a] + 1;
			if (nGAP > 1 && fPAT_Min_pix <= nGAP && fPAT_Max_pix >= nGAP)
			{
				if (a == 0)
					sDataP->AddData(nB, nS[a], nE[a], false);
				else
					sDataB->AddData(nB, nS[a], nE[a], false);
			}
		}
	}
	return true;
}
int PAD_PAT::CHECK_POLY_BIN(cv::Mat Img, cv::Rect roi, bool bX, int nPOLY_DIFF)
{
	const uchar* puc = Img.data;
	int nW = Img.cols;
	int nH = Img.rows;
	int nA_T = bX ? nH : nW;
	int nA_S = bX ? roi.y : roi.x;
	int nA_E = bX ? roi.y + roi.height : roi.x + roi.width;
	int nB_T = bX ? nW : nH;
	int nB_S = bX ? roi.x : roi.y;
	int nB_E = bX ? roi.x + roi.width : roi.y + roi.height;
	for (int nB = 0; nB < 2; nB++)
	{
		int b = nB == 0 ? nB_S : nB_E - 1;
		if (b < 0 || b >= nB_T)
			continue;

		int nOUT = 1;
		int nBCNT = 0;
		for (int a = nA_S; a < nA_E; a++)
		{
			if (a < 0 || a >= nA_T)
				continue;

			int nX = bX ? b : a;
			int nY = bX ? a : b;
			int nIDX = (nY * nW) + nX;
			if (puc[nIDX] == 255)
				nBCNT++;
			if (nBCNT > nOUT)
				break;
		}
		if (nBCNT == nOUT)
		{
			if (nB == 0)
				nB_S++;
			else
				nB_E--;
		}
	}

	for (int a = nA_S; a < nA_E; a++)
	{
		if (a < 0 || a >= nA_T)
			continue;

		int nBCNT = 0;
		for (int b = nB_S; b < nB_E; b++)
		{
			if (b < 0 || b >= nB_T)
				continue;

			int nX = bX ? b : a;
			int nY = bX ? a : b;
			int nIDX = (nY * nW) + nX;
			if (puc[nIDX] != 255)
				nBCNT++;
			if (nBCNT > (nPOLY_DIFF * 4))
				return -1;
		}
	}
	return nB_E - nB_S;
}
#pragma endregion

void PAD_PAT::SaveWorkImg(UCHAR* puc, int nW, int nH, CString fileName)
{
	if (puc == NULL || nW <= 0 || nH <= 0)
		return;

	bool bInspSave = (g_pMPTI && g_pMPTI->m_LogLevel == m_eLogLv_Blob);
#if _DEBUG
	bInspSave = true;
#endif
	if (bInspSave == false)
		return;

	CString strDir = _T("");
	strDir.Format(_T("%s\\PAT_IMG"), WORK_IMAGE_PATH);
	CreateDir(strDir);

	cv::Mat img(nH, nW, CV_8UC1, puc);
	CString fullPath = _T("");
	fullPath.Format(_T("%s\\%s"), strDir, fileName);
	cv::imwrite(std::string(CT2A(fullPath)), img);
}
void PAD_PAT::CreateDir(CString Path)
{
	if (IsExistDir(Path))
		return;

	CString strPath = Path;
	CString strTemp[20];
	CString strFile;
	int nLength = strPath.GetLength();
	int j = 0;

	for (int i = 0; i < 20; i++)
		strTemp[i].Empty();


	for (int i = 0; i < nLength; i++)
	{
		if (strPath.Mid(i, 1) == '\\')
		{
			j++;
		}
		strTemp[j] = strTemp[j] + strPath.Mid(i, 1);

	}

	strFile = strTemp[0];
	int i = 1;

	while (strTemp[i] != "")
	{
		strFile = strFile + strTemp[i];
		CreateDirectory(strFile, NULL);
		i++;
	}
}
BOOL PAD_PAT::IsExistDir(CString path)
{
	BOOL ret = FALSE;
	CFileFind fn;

	BOOL bWorking = fn.FindFile(path);
	if (bWorking)
	{
		bWorking = fn.FindNextFileW();
		if (fn.IsDirectory())
			ret = TRUE;
	}
	return ret;
}