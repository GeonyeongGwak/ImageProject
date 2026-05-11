// #include "Stdafx.h"
// #include "PInsp_Barcode.h"
// 
// CPInsp_Barcode::CPInsp_Barcode(void)
// {
// 
// }
// CPInsp_Barcode::~CPInsp_Barcode(void)
// {
// 
// }
// 
// bool CPInsp_Barcode::InspBarcode(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoBarcode *sRstAlgo)
// {
// 	bool bResult = false;
// 
// 	UCHAR *pucImgSrc = sWndAlgoImg.m_ucArr2D;
// 	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
// 	int nImgWidth = sWndAlgoImg.m_nWidth;
// 	int nImgHeight = sWndAlgoImg.m_nHeight;
// 	if (!m_pProcMilAlgo || (nImgWidth <= 0) || (nImgHeight <= 0)) // || sInspAlgo.m_eAlgoType != eAlgoBarcode
// 		return bResult;
// 
// 	AlgoBarcode *pInspAlgo = (AlgoBarcode *)sInspAlgo.m_ptrInspAlgoParam;
// 
// 	POINT st, ed;
// 	st.x = 0;
// 	st.y = 0;
// 	ed.x = nImgWidth - 1;
// 	ed.y = nImgHeight - 1;
// 
// 	int nInspType = 0;	//DTK : 0, MIL : 1, MIL + DTK : -1
// 	int nFlipX = 0, nFlipY = 0;
// 	int nCheckSum = 2;
// 	int nInspBarcodeCount = 1; //전달받은 영상 안에 같은 타입의 바코드 갯수
// 	int nBarcodeColor = 0; //먼저 검사 할 ForeGround 색상, 검정 : 0, 흰색 : 1
// 	int nBarcodeType = 14; //검사할 바코드 Type....ex)DataMatrix : 14, Code39 : 0, QRCode : 18, etc...
// 	int nThresholdType = 1; //DTK 라이브러리 사용할 경우 0 : Adaptive, 1: Automatic
// 	int nUseAngle = pInspAlgo->m_nArrData[(int)m_eAlgoBarUseAngle];
// 	bool bIsOrientationOK = true;	//pInspAlgo->m_nArrData[(int)m_eAlgoBarAngleOK];
// 	double dAngle = 0;
// 	int nStdAngle = pInspAlgo->m_nArrData[(int)m_eAlgoBarAngle];
// 
// 	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_MIL_Barcode) == (int)m_eAlgoBar_Data2_MIL_Barcode)
// 		nInspType = 1;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_All_Barcode) == (int)m_eAlgoBar_Data2_All_Barcode)
// 		nInspType = -1;
// 
// 	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_XFlip) == (int)m_eAlgoBar_Data_XFlip)
// 		nFlipX = 1;
// 	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_YFlip) == (int)m_eAlgoBar_Data_YFlip)
// 		nFlipY = 1;
// 
// 	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_ForeColor) == (int)m_eAlgoBar_Data_ForeColor)
// 		nBarcodeColor = 1;
// 
// 	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE39) == (int)m_eAlgoBar_Data_CODE39)
// 		nBarcodeType = 0;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE93) == (int)m_eAlgoBar_Data_CODE93)
// 		nBarcodeType = 1;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE128) == (int)m_eAlgoBar_Data_CODE128)
// 		nBarcodeType = 2;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODABAR) == (int)m_eAlgoBar_Data_CODABAR)
// 		nBarcodeType = 3;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_BC412) == (int)m_eAlgoBar_Data_BC412)
// 		nBarcodeType = 4;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_EAN8) == (int)m_eAlgoBar_Data_EAN8)
// 		nBarcodeType = 5;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_EAN13) == (int)m_eAlgoBar_Data_EAN13)
// 		nBarcodeType = 6;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_INTERLEAVED25) == (int)m_eAlgoBar_Data_INTERLEAVED25)
// 		nBarcodeType = 7;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PHARMACODE) == (int)m_eAlgoBar_Data_PHARMACODE)
// 		nBarcodeType = 8;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PLANET) == (int)m_eAlgoBar_Data_PLANET)
// 		nBarcodeType = 9;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_POSTNET) == (int)m_eAlgoBar_Data_POSTNET)
// 		nBarcodeType = 10;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_RSSCODE) == (int)m_eAlgoBar_Data_RSSCODE)
// 		nBarcodeType = 11;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UPC_A) == (int)m_eAlgoBar_Data_UPC_A)
// 		nBarcodeType = 12;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UPC_E) == (int)m_eAlgoBar_Data_UPC_E)
// 		nBarcodeType = 13;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_DATAMATRIX) == (int)m_eAlgoBar_Data_DATAMATRIX)
// 		nBarcodeType = 14;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_MAXICODE) == (int)m_eAlgoBar_Data_MAXICODE)
// 		nBarcodeType = 15;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PDF417) == (int)m_eAlgoBar_Data_PDF417)
// 		nBarcodeType = 16;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_MICROPDF417) == (int)m_eAlgoBar_Data_MICROPDF417)
// 		nBarcodeType = 17;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_QRCODE) == (int)m_eAlgoBar_Data_QRCODE)
// 		nBarcodeType = 18;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_COMPOSITECODE) == (int)m_eAlgoBar_Data_COMPOSITECODE)
// 		nBarcodeType = 19;
// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_GS1_128) == (int)m_eAlgoBar_Data_GS1_128)
// 		nBarcodeType = 20;
// 
// 	nThresholdType = 1;
// 	BCD_EncoderType_Order(nInspType, nFlipX, nFlipY, nCheckSum, nInspBarcodeCount);
// 
// 	CString sResultCode = _T("");
// 
// 	//int ret = BCD_Inspection(pucImgSrc, nImgWidth, nImgHeight, st, ed, nBarcodeType, nBarcodeColor, &sResultCode, nFlipX, nFlipY, nCheckSum, nInspBarcodeCount, nThresholdType, nUseAngle, &dAngle, nStdAngle, &bIsOrientationOK);
// 	int ret = 0;
// 	sRstAlgo->fAngle = dAngle;
// 	sRstAlgo->bIsOrientationOK = bIsOrientationOK;
// 	pInspAlgo->m_nArrData[(int)m_eAlgoBarAngleOK] = bIsOrientationOK;
// 
// 	if (ret)
// 	{
// 		sRstAlgo->m_bOK = TRUE;
// 		int nCompareBar = 1;
// 		CString sResult = sResultCode;
// 		CString sCompareBar = sResultCode;
// 		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UsePartialDisplay) == (int)m_eAlgoBar_Data2_UsePartialDisplay)
// 		{
// 			int nrPartialPosSt = pInspAlgo->m_nArrData[(int)m_eAlgoBarPartialFir] - 1;
// 			int nrPartialPosEd = pInspAlgo->m_nArrData[(int)m_eAlgoBarPartialSec] - 1;
// 			int nPartialLength = nrPartialPosEd - nrPartialPosSt + 1;
// 			sCompareBar = sResult.Mid(nrPartialPosSt, nPartialLength);
// 		}
// 		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseCharacterCount) == (int)m_eAlgoBar_Data2_UseCharacterCount)
// 		{
// 			int nCharCnt = sResult.GetLength();
// 			if (pInspAlgo->m_nArrData[(int)m_eAlgoBarCharCntMin] > nCharCnt || pInspAlgo->m_nArrData[(int)m_eAlgoBarCharCntMax] < nCharCnt)
// 				sRstAlgo->m_bOK = false;
// 		}
// 		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseEssentialWords) == (int)m_eAlgoBar_Data2_UseEssentialWords)
// 		{
// 			int nEssentialPos = pInspAlgo->m_nArrData[(int)m_eAlgoBarEssentialWordsPos] - 1;
// 			CString sEss;
// 			sEss.Format(_T("%s"), pInspAlgo->m_sEss);
// 			sResult = sCompareBar.Mid(nEssentialPos, sEss.GetLength());
// 
// 			if (sResult != sEss)
// 			{
// 				sRstAlgo->m_bOK = false;
// 			}
// 		}
// 
// 		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseStandardWords) == (int)m_eAlgoBar_Data2_UseStandardWords)
// 		{
// 			CString StandardWords;
// 			StandardWords.Format(_T("%s"), pInspAlgo->m_sStd);
// 
// 			if (sResultCode != StandardWords)
// 			{
// 				sRstAlgo->m_bOK = false;
// 			}
// 		}
// 
// 		if (sResultCode.GetLength() != 0)
// 		{
// 			memcpy(sRstAlgo->m_sBarcode, sResultCode.GetBuffer(), sResultCode.GetLength() * 2);
// 			sRstAlgo->m_sBarcode[sResultCode.GetLength() * 2] = 0;
// 
// 			if (sCompareBar.GetLength() != 0)
// 			{
// 				memcpy(sRstAlgo->m_sPartialBarcode, sCompareBar.GetBuffer(), sCompareBar.GetLength() * 2);
// 				sRstAlgo->m_sPartialBarcode[sCompareBar.GetLength() * 2] = 0;
// 			}
// 			else
// 			{
// 				memset(sRstAlgo->m_sPartialBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
// 			}
// 		}
// 		else
// 		{
// 			memset(sRstAlgo->m_sBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
// 		}
// 
// 		if (sRstAlgo->m_bOK != false)
// 			return true;
// 		else
// 			return false;
// 	}
// 	return bResult;
// }