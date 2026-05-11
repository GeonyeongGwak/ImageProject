#include "PInsp_AlgoBarcode.h"
CPInsp_AlgoBarcode::CPInsp_AlgoBarcode(void)
{
}


CPInsp_AlgoBarcode::~CPInsp_AlgoBarcode(void)
{
}

void CPInsp_AlgoBarcode::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoBarcode::GetInspAlgoData()
{
	return eSPCAlgoBarcode;
}

int CPInsp_AlgoBarcode::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeMountWorng] = e_NG;
	nCurrentNgType = TypeMountWorng;
	return nCurrentNgType;
}

bool CPInsp_AlgoBarcode::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoBarcode::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	bResult = InspBarcode(sInspAlgo, sWndAlgoImg, (RstAlgoBarcode *)sRstAlgo);

	return bResult;
}

BOOL CPInsp_AlgoBarcode::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	BOOL bResult = FALSE;
	bResult = InspBarcode_Halcon(InspAlgo, pImg_buf, vecAlignResult, (RstAlgoBarcode *)sRstAlgo);
	SetInspectWindowPosition(algoParam, (RstAlgoBarcode *)sRstAlgo);
	return bResult;
}

BOOL CPInsp_AlgoBarcode::InspBarcode(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoBarcode *sRstAlgo, PIAL::Insp_Image* pImg_buf)
{
	if (sRstAlgo)
	{
		memset(sRstAlgo, 0, (sizeof(RstAlgoBarcode)));
		sRstAlgo->Init();
	}
	BOOL bResult = FALSE;
	UCHAR *pucImgSrc;// = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc;// = sWndAlgoImg.m_fArr3D;
	int nImgWidth;// = sWndAlgoImg.m_nWidth;
	int nImgHeight;// = sWndAlgoImg.m_nHeight;
	if (pImg_buf != nullptr)
	{
		pucImgSrc = pImg_buf->m_p2D->m_pData;
		nImgWidth = pImg_buf->m_p2D->Width();
		nImgHeight = pImg_buf->m_p2D->Length();
	}
	else
	{
		pucImgSrc = sWndAlgoImg.m_ucArr2D;
		pfImgSrc = sWndAlgoImg.m_fArr3D;
		nImgWidth = sWndAlgoImg.m_nWidth;
		nImgHeight = sWndAlgoImg.m_nHeight;
	}
	if (!m_pProcMilAlgo || (nImgWidth <= 0) || (nImgHeight <= 0) || sInspAlgo.m_eAlgoType != eAlgoBarcode)
		return bResult;
	AlgoBarcode *pInspAlgo = (AlgoBarcode *)sInspAlgo.m_ptrInspAlgoParam;

	POINT st, ed;
	st.x = 0;
	st.y = 0;
	ed.x = nImgWidth - 1;
	ed.y = nImgHeight - 1;

	int nInspType = 0;//DTK : 0, MIL : 1, MIL + DTK : -1
	int nFlipX = 0, nFlipY = 0;
	int nCheckSum = 2;
	int nInspBarcodeCount = 1; //전달받은 영상 안에 같은 타입의 바코드 갯수
	int nBarcodeColor = 0; //먼저 검사 할 ForeGround 색상, 검정 : 0, 흰색 : 1
	int nBarcodeType = 14; //검사할 바코드 Type....ex)DataMatrix : 14, Code39 : 0, QRCode : 18, etc...
	int nThresholdType = 1; //DTK 라이브러리 사용할 경우 0 : Adaptive, 1: Automatic

	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_MIL_Barcode) == (int)m_eAlgoBar_Data2_MIL_Barcode)
		nInspType = 1;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_All_Barcode) == (int)m_eAlgoBar_Data2_All_Barcode)
		nInspType = -1;

	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_XFlip) == (int)m_eAlgoBar_Data_XFlip)
		nFlipX = 1;
	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_YFlip) == (int)m_eAlgoBar_Data_YFlip)
		nFlipY = 1;

	// 	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_B_UseCheckSum) == (int)m_eAlgoBar_Data_B_UseCheckSum)
	// 		nCheckSum = 1;
	// 	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_B_BOTH) == (int)m_eAlgoBar_Data_B_BOTH)
	// 		nCheckSum = 2;

	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_ForeColor) == (int)m_eAlgoBar_Data_ForeColor)
		nBarcodeColor = 1;

	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE39) == (int)m_eAlgoBar_Data_CODE39)
		nBarcodeType = 0;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE93) == (int)m_eAlgoBar_Data_CODE93)
		nBarcodeType = 1;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE128) == (int)m_eAlgoBar_Data_CODE128)
		nBarcodeType = 2;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODABAR) == (int)m_eAlgoBar_Data_CODABAR)
		nBarcodeType = 3;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_BC412) == (int)m_eAlgoBar_Data_BC412)
		nBarcodeType = 4;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_EAN8) == (int)m_eAlgoBar_Data_EAN8)
		nBarcodeType = 5;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_EAN13) == (int)m_eAlgoBar_Data_EAN13)
		nBarcodeType = 6;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_INTERLEAVED25) == (int)m_eAlgoBar_Data_INTERLEAVED25)
		nBarcodeType = 7;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PHARMACODE) == (int)m_eAlgoBar_Data_PHARMACODE)
		nBarcodeType = 8;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PLANET) == (int)m_eAlgoBar_Data_PLANET)
		nBarcodeType = 9;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_POSTNET) == (int)m_eAlgoBar_Data_POSTNET)
		nBarcodeType = 10;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_RSSCODE) == (int)m_eAlgoBar_Data_RSSCODE)
		nBarcodeType = 11;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UPC_A) == (int)m_eAlgoBar_Data_UPC_A)
		nBarcodeType = 12;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UPC_E) == (int)m_eAlgoBar_Data_UPC_E)
		nBarcodeType = 13;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_DATAMATRIX) == (int)m_eAlgoBar_Data_DATAMATRIX)
		nBarcodeType = 14;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_MAXICODE) == (int)m_eAlgoBar_Data_MAXICODE)
		nBarcodeType = 15;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PDF417) == (int)m_eAlgoBar_Data_PDF417)
		nBarcodeType = 16;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_MICROPDF417) == (int)m_eAlgoBar_Data_MICROPDF417)
		nBarcodeType = 17;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_QRCODE) == (int)m_eAlgoBar_Data_QRCODE)
		nBarcodeType = 18;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_COMPOSITECODE) == (int)m_eAlgoBar_Data_COMPOSITECODE)
		nBarcodeType = 19;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_GS1_128) == (int)m_eAlgoBar_Data_GS1_128)
		nBarcodeType = 20;
	else if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_MICRO_QR) == (int)m_eAlgoBar_Data_MICRO_QR)
		nBarcodeType = 21;

	// 	if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_Automatic) == (int)m_eAlgoBar_Data2_Automatic)
	nThresholdType = 1;
	// 	else
	// 		nThresholdType = 0;

	BCD_EncoderType_Order(nInspType, nFlipX, nFlipY, nCheckSum, nInspBarcodeCount);

	CString sResultCode = _T("");

	POINT *re_st, *re_ed;
	re_st = re_ed = NULL;

	RstBarcode _rstBarcode;
	
	TeachBarcode TeachBar;
	TeachBar.Src = pucImgSrc;
	TeachBar.imgSizeX = nImgWidth;
	TeachBar.imgSizeY = nImgHeight;
	TeachBar.st = st;
	TeachBar.ed = ed;
	TeachBar.type = nBarcodeType;
	TeachBar.color = nBarcodeColor;
	TeachBar.nFlipX = nFlipX;
	TeachBar.nFlipY = nFlipY;
	TeachBar.nCheckSum = nCheckSum;
	TeachBar.nCount = nInspBarcodeCount;
	TeachBar.nThresholdType = nThresholdType;
	float fReSize = .0f;

	{
		//multy 사용 시 CItoP_dll은 tool 넘버링 추가해야 함.
		TCHAR fname[MAX_PATH];
		int nAlgorithmToolIndex = 0;
		if (g_pMPTI->GetUseMultiProcess())
			nAlgorithmToolIndex = ext::env::nTool_id;

		CString sTemp = _T("");
		sTemp.Format(_T("C:\\barcode\\TestData\\CItoP_dll[%d].dat"), nAlgorithmToolIndex);
		_stprintf(fname, sTemp);
		TCHAR buff[MAX_PATH];

		CString strAppName = _T("");
		SetTypeName(nBarcodeType, strAppName);

		GetPrivateProfileString(strAppName, _T("DownSize"), _T("1.0"), buff, sizeof(buff), fname);
		fReSize = _tstof(buff);
	}


	int ret = BCD_Inspection(TeachBar, &_rstBarcode);

	if (ret)
	{
		sRstAlgo->m_bOK = TRUE;
		sRstAlgo->m_nBarType = SetDataType(TeachBar.type);
		int nCompareBar = 1;
		CString sResult = _rstBarcode.sResultCode;
		CString sCompareBar = _rstBarcode.sResultCode;

		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UsePartialDisplay) == (int)m_eAlgoBar_Data2_UsePartialDisplay)
		{
			int nrPartialPosSt = pInspAlgo->m_nArrData[(int)m_eAlgoBarPartialFir] - 1;
			int nrPartialPosEd = pInspAlgo->m_nArrData[(int)m_eAlgoBarPartialSec] - 1;
			int nPartialLength = nrPartialPosEd - nrPartialPosSt + 1;
			sCompareBar = sResult.Mid(nrPartialPosSt, nPartialLength);
		}


		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseEssentialWords) == (int)m_eAlgoBar_Data2_UseEssentialWords)
		{
			int nEssentialPos = pInspAlgo->m_nArrData[(int)m_eAlgoBarEssentialWordsPos] - 1;
			CString sEss;
			sEss.Format(_T("%s"), pInspAlgo->m_sEss);
			sResult = sCompareBar.Mid(nEssentialPos, sEss.GetLength());

			if (sResult != sEss)
			{
				sRstAlgo->m_bOK = false;
			}
		}

		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseStandardWords) == (int)m_eAlgoBar_Data2_UseStandardWords)
		{
			CString StandardWords;
			StandardWords.Format(_T("%s"), pInspAlgo->m_sStd);

			if (_rstBarcode.sResultCode != StandardWords && pInspAlgo->m_bUseBarcodeAlign != true)
			{
				sRstAlgo->m_bOK = false;
			}
		}

		if (_rstBarcode.sResultCode.GetLength() != 0)
		{
			memcpy(sRstAlgo->m_sBarcode, _rstBarcode.sResultCode.GetBuffer(), _rstBarcode.sResultCode.GetLength() * 2);
			sRstAlgo->m_sBarcode[_rstBarcode.sResultCode.GetLength() * 2] = 0;

			if (sCompareBar.GetLength() != 0)
			{
				memcpy(sRstAlgo->m_sPartialBarcode, sCompareBar.GetBuffer(), sCompareBar.GetLength() * 2);
				sRstAlgo->m_sPartialBarcode[sCompareBar.GetLength() * 2] = 0;
			}
			else
			{
				memset(sRstAlgo->m_sPartialBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
			}
		}
		else
		{
			memset(sRstAlgo->m_sBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
		}
		
		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UseCrossCheck1) == (int)m_eAlgoBar_Data_UseCrossCheck1)
		{
			sRstAlgo->IsCrossCheckOK = CompareTarget(pInspAlgo, sRstAlgo);
		}
		if ((pInspAlgo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UseCrossCheck2) == (int)m_eAlgoBar_Data_UseCrossCheck2)
		{
			sRstAlgo->IsCrossCheckOK2 = CompareTarget2(pInspAlgo, sRstAlgo);
		}

		for (int i = 0; i < 4; i++)
		{
			sRstAlgo->m_ptRst_Coner[i] = _rstBarcode.vCorner[i];
		}
		 
		if(pInspAlgo->m_bUseBarcodeAlign)
		{
			//_rstBarcode.ptCen, _rstBarcode.rtBoundary 은 검사 시 사용된 image의 결과값
			//Teach 대비 결과 Center 비교하여 Align 검출.
			//바코드 설정(DownSize)이 변경 안된다면 상관없음.
			//_rstBarcode.rtBoundary 는 image size로 변경.

			_rstBarcode.rtBoundary.left *= fReSize;
			_rstBarcode.rtBoundary.right *= fReSize;
			_rstBarcode.rtBoundary.top *= fReSize;
			_rstBarcode.rtBoundary.bottom *= fReSize;

			float dImageCenX = TeachBar.imgSizeX / 2.0;
			float dImageCenY = TeachBar.imgSizeY / 2.0;
			double dWndAngle = sWndAlgoImg.dAngle;
			sRstAlgo->m_bUseAlign = pInspAlgo->m_bUseBarcodeAlign;
			//if (dWndAngle == 90 || dWndAngle == 270)
			//{
			//	dImageCenX = TeachBar.imgSizeY / 2.0;
			//	dImageCenY = TeachBar.imgSizeX / 2.0;
			//}

			if(pInspAlgo->m_bUseTeachBarcodeAlign)
			{
				sRstAlgo->ptCen.x = (dImageCenX - (float)_rstBarcode.ptCen.x) * m_resolX;
				sRstAlgo->ptCen.y = (dImageCenY - (float)_rstBarcode.ptCen.y) * m_resolY;
				sRstAlgo->fAngle = _rstBarcode.fAngle;
			}
			else
			{
				if (pInspAlgo->m_ptTeachCen.x > 0)
					pInspAlgo->m_ptTeachCen.x *= -1;
				//pInspAlgo->m_ptTeachCen.x *= -1;

				float fTeachPtX = roundf(((float)pInspAlgo->m_ptTeachCen.x / m_resolX + dImageCenX));
				float fTeachPtY = roundf(( -1 * ((float)pInspAlgo->m_ptTeachCen.y / m_resolY) + dImageCenY));

				float fTempX = (fTeachPtX - (float)_rstBarcode.ptCen.x);
				float fTempY = (fTeachPtY - (float)_rstBarcode.ptCen.y);

				if (fTempX == 0.0f)
					fTempX = 1.0f;
				if (fTempY == 0.0f)
					fTempY = 1.0f;

				sRstAlgo->ptCen.x = fTempX * m_resolX;
				sRstAlgo->ptCen.y = fTempY * m_resolY;
				//sRstAlgo->fAngle = _rstBarcode.fAngle - pInspAlgo->m_fTeachAngle;
				sRstAlgo->fAngle = _rstBarcode.fAngle;
				sRstAlgo->m_rcROI = _rstBarcode.rtBoundary;
#if _DEBUG
				cv::Mat src = cv::Mat(TeachBar.imgSizeY, TeachBar.imgSizeX, CV_MAKETYPE(CV_8U, 1), sWndAlgoImg.m_ucArr2D);
				cv::Mat rstRect = cv::Mat::zeros(TeachBar.imgSizeY, TeachBar.imgSizeX, CV_MAKETYPE(CV_8U, 1));
				cv::Rect rt_Rst = cv::Rect(cv::Point(_rstBarcode.rtBoundary.left, _rstBarcode.rtBoundary.top), cv::Point(_rstBarcode.rtBoundary.right, _rstBarcode.rtBoundary.bottom));
				cv::Point pt_Rst = cv::Point((int)_rstBarcode.ptCen.x, (int)_rstBarcode.ptCen.y);
				cv::rectangle(rstRect, rt_Rst, cv::Scalar(255), cv::FILLED);

				cv::Mat pDst;
				cv::Mat pMerImg[3] = { cv::Mat::zeros(cv::Size(src.cols, src.rows), src.type()), src,  rstRect };
				cv::merge(pMerImg, 3, pDst);
#endif
			}
		}
		else
		{
			_rstBarcode.rtBoundary.left *= fReSize;
			_rstBarcode.rtBoundary.right *= fReSize;
			_rstBarcode.rtBoundary.top *= fReSize;
			_rstBarcode.rtBoundary.bottom *= fReSize;

			sRstAlgo->ptCen.x = _rstBarcode.rtBoundary.CenterPoint().x;
			sRstAlgo->ptCen.y = _rstBarcode.rtBoundary.CenterPoint().y;
			sRstAlgo->m_rcROI = _rstBarcode.rtBoundary;
			sRstAlgo->fAngle = _rstBarcode.fAngle;

#if _DEBUG
			cv::Mat src;
			if (pImg_buf != nullptr)
			{
				src = cv::Mat(TeachBar.imgSizeY, TeachBar.imgSizeX, CV_MAKETYPE(CV_8U, 1), pImg_buf->m_p2D->m_pData);
			}
			else
			{
				src = cv::Mat(TeachBar.imgSizeY, TeachBar.imgSizeX, CV_MAKETYPE(CV_8U, 1), sWndAlgoImg.m_ucArr2D);
			}
			cv::Mat rstRect = cv::Mat::zeros(TeachBar.imgSizeY, TeachBar.imgSizeX, CV_MAKETYPE(CV_8U, 1));
			cv::Rect rt_Rst = cv::Rect(cv::Point(_rstBarcode.rtBoundary.left, _rstBarcode.rtBoundary.top), cv::Point(_rstBarcode.rtBoundary.right, _rstBarcode.rtBoundary.bottom));
			cv::rectangle(rstRect, rt_Rst, cv::Scalar(255), cv::FILLED);
			cv::Mat pDst;
			cv::Mat pMerImg[3] = { cv::Mat::zeros(cv::Size(src.cols, src.rows), src.type()), src,  rstRect };
			cv::merge(pMerImg, 3, pDst);
#endif // _DEBUG
		}
		
		if (false)
		{
			//QuietZone 체크 예제 함수
			//전제조건 
			//라벨은 흰색으로 주변과 명확히 구분될수 있어야함.
			float fQZSize = 1.2f; //ex) 1.2mm 

			cv::Mat src;
			if (pImg_buf != nullptr)
			{
				src = cv::Mat(TeachBar.imgSizeY, TeachBar.imgSizeX, CV_MAKETYPE(CV_8U, 1), pImg_buf->m_p2D->m_pData);
			}
			else
			{
				src = cv::Mat(TeachBar.imgSizeY, TeachBar.imgSizeX, CV_MAKETYPE(CV_8U, 1), sWndAlgoImg.m_ucArr2D);
			}
			cv::Mat threImg, rstImg, mergeIng;
			cv::Mat convexHulling = cv::Mat::zeros(src.size(), src.type());
			cv::Mat redImg = cv::Mat::zeros(src.size(), src.type());
			cv::Mat rstRect = cv::Mat::zeros(TeachBar.imgSizeY, TeachBar.imgSizeX, CV_MAKETYPE(CV_8U, 1));

			cv::threshold(src, threImg, 0.0, 255.0, cv::THRESH_OTSU);

			CRect rtTemp = _rstBarcode.rtBoundary;
			rtTemp.left *= fReSize;
			rtTemp.right *= fReSize;
			rtTemp.top *= fReSize;
			rtTemp.bottom *= fReSize;

			cv::Rect rt_Rst = cv::Rect(cv::Point(rtTemp.left, rtTemp.top), cv::Point(rtTemp.right, rtTemp.bottom));
			cv::rectangle(rstRect, rt_Rst, cv::Scalar(255), cv::FILLED);

#if _DEBUG
			cv::Mat pDst;
			cv::Mat pMerImg[3] = { cv::Mat::zeros(cv::Size(src.cols, src.rows), src.type()), src,  rstRect };
			cv::merge(pMerImg, 3, pDst);
#endif

			cv::Mat xorImg;
			cv::bitwise_xor(threImg, rstRect, xorImg);

			cv::Mat topImg, bottomImg, leftImg, rightpImg;
			int nMargin = 5;

			//size에 대한 예외처리 있어야함.차후 수정

			xorImg(cv::Rect(cv::Point(rtTemp.left, 0), cv::Point(rtTemp.right, rtTemp.top - nMargin))).copyTo(topImg);
			xorImg(cv::Rect(cv::Point(rtTemp.left, rtTemp.bottom + nMargin), cv::Point(rtTemp.right, xorImg.rows - 1))).copyTo(bottomImg);
			xorImg(cv::Rect(cv::Point(0, rtTemp.top), cv::Point(rtTemp.left - nMargin, rtTemp.bottom))).copyTo(leftImg);
			xorImg(cv::Rect(cv::Point(rtTemp.right + nMargin, rtTemp.top), cv::Point(xorImg.cols - 1, rtTemp.bottom))).copyTo(rightpImg);

			//bTop
			//다른 이물 및 프린팅이 있으면 threshold 로 인하여 0 pixel 되고
			//0 pixel 찾는 코드.(opencv projection 대체용.)
			//가성발생으로 인하여 다른 방법 모색해야함.
			bool bTop = true, bBot = true, bLeft = true, bRight = true;

			cv::Point ptTopBlack = cv::Point();
			for (int y = topImg.rows - 1; y >= 0; y--)
			{ 
				for (int x = 0; x < topImg.cols; x++)
				{
					int pData = topImg.data[y*topImg.step + x];
					if (pData == 0)
					{ 
						ptTopBlack = cv::Point(x, y);
						break; 
					}
				}
				if (ptTopBlack != cv::Point())
					break;
			}
			float fTopSize = (topImg.rows - ptTopBlack.y) * m_resolY;
			if (fQZSize > fTopSize)
				bTop = false;

			cv::Point ptBotBlack = cv::Point();
			for (int y = 0; y < bottomImg.rows; y++)
			{
				for (int x = 0; x < bottomImg.cols; x++)
				{
					int pData = bottomImg.data[y*bottomImg.step + x];
					if (pData == 0)
					{
						ptBotBlack = cv::Point(x, y);
						break;
					}
				}
				if (ptBotBlack != cv::Point())
					break;
			}
			float fBotSize = (bottomImg.rows - ptBotBlack.y) * m_resolY;
			if (fQZSize > fBotSize)
				bBot = false;

			cv::Point ptLeftBlack = cv::Point();
			for (int y = 0; y < leftImg.rows; y++)
			{
				for (int x = leftImg.cols-1; x >= 0; x--)
				{
					int pData = leftImg.data[y*leftImg.step + x];
					if (pData == 0)
					{
						ptLeftBlack = cv::Point(x, y);
						break;
					}
				}
				if (ptLeftBlack != cv::Point())
					break;
			}
			float fLeftSize = (leftImg.cols - ptLeftBlack.x) * m_resolX;
			if (fQZSize > fLeftSize)
				bLeft = false;

			cv::Point ptRightBlack = cv::Point();
			for (int y = 0; y < rightpImg.rows; y++)
			{
				for (int x = 0; x < rightpImg.cols; x++)
				{
					int pData = rightpImg.data[y*rightpImg.step + x];
					if (pData == 0)
					{
						ptRightBlack = cv::Point(x, y);
						break;
					}
				}
				if (ptRightBlack != cv::Point())
					break;
			}
			float fRightSize = (rightpImg.cols - ptRightBlack.x) * m_resolX;
			if (fQZSize > fRightSize)
				bRight = false;

			bool bCheck = true;
			if (bTop != true || bBot != true || bLeft != true || bRight != true)
				bCheck = false;
				//sRstAlgo->IsQuietZoneOK = false;
		}


		if (sRstAlgo->m_bOK != false && sRstAlgo->IsCrossCheckOK != false && sRstAlgo->IsCrossCheckOK2 != false)
			return true;
		else
			return false;
	}
	return false;
}

//InspAlgorithm_Dll
BOOL CPInsp_AlgoBarcode::InspBarcode_Halcon(const InspAlgo &InspAlgo, PIAL::Insp_Image* pImg_buf, std::vector<PIAL::_AlignResult> vecAlignResult, RstAlgoBarcode *sRstAlgo) {
	BOOL bResult = FALSE;
	BOOL bQuality = FALSE;
	BOOL bQuality2DModule = FALSE;
	BOOL bPNValidate = FALSE;
	BOOL bSaveImage = FALSE;
	BOOL bPNImgSave = FALSE;
	UCHAR *pucImgSrc = pImg_buf->m_p2D->m_pData;
	int nImgWidth = pImg_buf->m_p2D->Width();
	int nImgHeight = pImg_buf->m_p2D->Length();
	int nBarcodeColor = 0; //먼저 검사 할 ForeGround 색상, 검정 : 0, 흰색 : 1
	int nBarcodeType = 30; //검사할 바코드 Type....ex)DataMatrix : 14, Code39 : 0, QRCode : 18, etc...
	int nDataType = 0;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	sRstAlgo->Init();
	int check = InspWrapper->m_PInspAlgo->PInspAlgo::CheckHalconLicense_Only();

	int Line = __LINE__;
	if (check != 1)	//할콘 라이센스 체크
	{
		if (g_pMPTI)
		{
			CString sLog = _T(""), sType = _T("");
			sType.Format(_T("HALCON License Error. (%d)"), check);

			sLog.Format(_T("[PAlgoBarcode_Dll], Line : %d, Barcode - %s"), Line, sType);
			g_pMPTI->AddLog_Dev(sLog);

			sLog.Format(_T("ERROR : Barcode - %s"), sType);
			AfxMessageBox(sLog);

			throw Line;
		}
	}
	AlgoBarcode *algo = (AlgoBarcode*)InspAlgo.m_ptrInspAlgoParam;
	if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_ForeColor) == (int)m_eAlgoBar_Data_ForeColor)
		nBarcodeColor = 1;

	if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE39) == (int)m_eAlgoBar_Data_CODE39)
		nBarcodeType = 0;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE93) == (int)m_eAlgoBar_Data_CODE93)
		nBarcodeType = 1;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODE128) == (int)m_eAlgoBar_Data_CODE128)
		nBarcodeType = 2;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_CODABAR) == (int)m_eAlgoBar_Data_CODABAR)
		nBarcodeType = 3;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_BC412) == (int)m_eAlgoBar_Data_BC412)
		nBarcodeType = 4;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_EAN8) == (int)m_eAlgoBar_Data_EAN8)
		nBarcodeType = 5;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_EAN13) == (int)m_eAlgoBar_Data_EAN13)
		nBarcodeType = 6;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_INTERLEAVED25) == (int)m_eAlgoBar_Data_INTERLEAVED25)
		nBarcodeType = 7;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PHARMACODE) == (int)m_eAlgoBar_Data_PHARMACODE)
		nBarcodeType = 8;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PLANET) == (int)m_eAlgoBar_Data_PLANET)
		nBarcodeType = 9;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_POSTNET) == (int)m_eAlgoBar_Data_POSTNET)
		nBarcodeType = 10;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_RSSCODE) == (int)m_eAlgoBar_Data_RSSCODE)
		nBarcodeType = 11;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UPC_A) == (int)m_eAlgoBar_Data_UPC_A)
		nBarcodeType = 12;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UPC_E) == (int)m_eAlgoBar_Data_UPC_E)
		nBarcodeType = 13;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_DATAMATRIX) == (int)m_eAlgoBar_Data_DATAMATRIX)
		nBarcodeType = 14;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_MAXICODE) == (int)m_eAlgoBar_Data_MAXICODE)
		nBarcodeType = 15;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_PDF417) == (int)m_eAlgoBar_Data_PDF417)
		nBarcodeType = 16;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_MICROPDF417) == (int)m_eAlgoBar_Data_MICROPDF417)
		nBarcodeType = 17;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_QRCODE) == (int)m_eAlgoBar_Data_QRCODE)
		nBarcodeType = 18;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_COMPOSITECODE) == (int)m_eAlgoBar_Data_COMPOSITECODE)
		nBarcodeType = 19;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_GS1_128) == (int)m_eAlgoBar_Data_GS1_128)
		nBarcodeType = 20;
	else if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_MICRO_QR) == (int)m_eAlgoBar_Data_MICRO_QR)
		nBarcodeType = 21;

	if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseBarcodeQuality) == (int)m_eAlgoBar_Data2_UseBarcodeQuality)
		bQuality = true;
	if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_Use2DQualityModule) == (int)m_eAlgoBar_Data2_Use2DQualityModule)
		bQuality2DModule = true;
	if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseMachinOptSaveImage) == (int)m_eAlgoBar_Data2_UseMachinOptSaveImage)
		bSaveImage = true;
	if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UsePNValidation) == (int)m_eAlgoBar_Data2_UsePNValidation)
		bPNValidate = true;
	if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseSplitFailImgSave) == (int)m_eAlgoBar_Data2_UseSplitFailImgSave)
		bPNImgSave = true;

	int nRetry = algo->m_nArrData[(int)m_eAlgoBarRetry] + 1; // 바코드 검사 NG일 때, OK나올때까지 Retry 설정 횟수만큼 추가 검사
	float fDefSpecWid = algo->m_fDefWidth / m_resolX;
	float fDefSpecLen = algo->m_fDefLength / m_resolY;
	int nDefSpecCnt = algo->m_nDefCount;

	PIAL::_AlgoBarcode pAlgoBarcode;	
	PIAL::_RstAlgoBarcode pRstBarcode;
	pAlgoBarcode.ucSrc = new UCHAR[pImg_buf->m_p2D->Width()*pImg_buf->m_p2D->Length()];

	memcpy(pAlgoBarcode.ucSrc, pucImgSrc, pImg_buf->m_p2D->Width()*pImg_buf->m_p2D->Length());

	//pAlgoBarcode.ucSrc = pucImgSrc;
	pAlgoBarcode.nImgSizeX = nImgWidth;
	pAlgoBarcode.nImgSizeY = nImgHeight;
	pAlgoBarcode.nColor = nBarcodeColor;
	pAlgoBarcode.nRetry = nRetry;
	pAlgoBarcode.nType = nBarcodeType;
	pAlgoBarcode.bQualityCheck = bQuality;
	pAlgoBarcode.bQuality2DModule = bQuality2DModule;
	pAlgoBarcode.nOffsetX = algo->m_nArrData[(int)m_eAlgoBarQuality1DOffsetX];
	pAlgoBarcode.nOffsetY = algo->m_nArrData[(int)m_eAlgoBarQuality1DOffsetY];
	pAlgoBarcode.nTolerance = algo->m_nArrData[(int)m_eAlgoBarQuality1DTolerrance];
	pAlgoBarcode.n2DTargetGrade = algo->m_nArrData[(int)m_eAlgoBarQuality2DGrade] + 1; //Index + 1
	pAlgoBarcode.n2DContrast = algo->m_nArrData[(int)m_eAlgoBarQuality2DContrast];
	//pAlgoBarcode.n2DDefectArea = algo->m_nArrData[(int)m_eAlgoBarQuality2DDefectArea];
	pAlgoBarcode.n2DDefectCnt = algo->m_nArrData[(int)m_eAlgoBarQuality2DDefectCnt];
	float f2DDefectArea = (algo->m_f2DDefArea / ((m_resolX * 1000) * (m_resolY * 1000)));
	pAlgoBarcode.n2DDefectArea = static_cast<int>(std::round(f2DDefectArea));
	pAlgoBarcode.bSaveImage = bSaveImage;

	nDataType = SetDataType(nBarcodeType, true);
	if (nDataType == 0)
	{
		return false;
	}
	sRstAlgo->m_nBarType = nDataType;

	float fReSize = 1.0f;
	{	//미사용
		//multy 사용 시 CItoP_dll은 tool 넘버링 추가해야 함.
		//TCHAR fname[MAX_PATH];
		//_stprintf(fname, _T("C:\\barcode\\TestData\\CItoP_dll.dat"));
		//TCHAR buff[MAX_PATH];

		//CString strAppName = _T("");
		//SetTypeName(nBarcodeType, strAppName);

		//GetPrivateProfileString(strAppName, _T("DownSize"), _T("1.0"), buff, sizeof(buff), fname);
		//fReSize = _tstof(buff);
	}

	bResult = InspWrapper->m_PInspAlgo->PInspAlgo::HalconInspBarcode(*pImg_buf, pAlgoBarcode, &pRstBarcode, vecAlignResult);
	delete pAlgoBarcode.ucSrc;
	
	//if (!bResult && pRstBarcode.sResultCode == "")
	//{	// Halcon에서 바코드 리딩 실패한경우 DTK로 재시도
	//	WndAlgoImg sWndAlgoImg;
	//	bResult = InspBarcode(InspAlgo, sWndAlgoImg, sRstAlgo, pImg_buf);
	//	if (bResult)
	//	{
	//		bReadByDTK = TRUE;
	//		pRstBarcode.sResultCode = sRstAlgo->m_sBarcode;
	//		pRstBarcode.fAngle = sRstAlgo->fAngle;
	//		for (int i = 0; i < 4; i++)
	//		{
	//			pRstBarcode.vCorner[i] = sRstAlgo->m_ptRst_Coner[i];
	//		}
	//		pRstBarcode.rtBoundary = sRstAlgo->m_rcROI;
	//		pRstBarcode.ptCen = sRstAlgo->ptCen;
	//		bool bValidate = InspWrapper->m_PInspAlgo->PInspAlgo::Validate1DBarcodeArea(pImg_buf->m_p2D, pRstBarcode.vCorner, 4);
	//		if (bQuality)
	//		{
	//			bValidate = InspWrapper->m_PInspAlgo->PInspAlgo::Validate1DBarcodeInsp(pAlgoBarcode, &pRstBarcode, pImg_buf->m_p2D);
	//		}
	//	}
	//}

	if (nDataType == 2)
		ValidateSplitData(pRstBarcode.sResultCode, nDataType);

	CString sLog;
	sLog.Format(_T("[PAlgoBarcode_Dll] %dD Result : %s"), nDataType, pRstBarcode.sResultCode);
	g_pMPTI->AddLog_Dev(sLog);

	if (bResult)
	{
		sRstAlgo->m_bOK = TRUE;
		int nCompareBar = 1;
		CString sResult = pRstBarcode.sResultCode;
		CString sCompareBar = pRstBarcode.sResultCode;
		if (nDataType == 1 && bQuality)
		{
			//if (bReadByDTK)
			//{
			//	sRstAlgo->m_fArrRst_QC1D[0] = 10000;
			//}
			// HALCON 품질 미사용으로 주석처리함.
			//else
			//{
			//	for (int i = 0; i < std::size(pRstBarcode.fArrBarcodeQC); i++)
			//	{	// 1D 바코드 품질평가 Rst
			//		sRstAlgo->m_fArrRst_QC1D[i] = pRstBarcode.fArrBarcodeQC[i];
			//	}
			//	if (sRstAlgo->m_fArrRst_QC1D[1] <= 0)
			//	{
			//		sRstAlgo->IsDecodeQualityOK = FALSE;
			//	}
			//	if (sRstAlgo->m_fArrRst_QC1D[6] <= 3.5)
			//	{
			//		sRstAlgo->IsDefectQualityOK = FALSE;
			//	}
			//}

			if (pRstBarcode.nInspRectCnt > 0)
			{
				sRstAlgo->nInspRectCnt = pRstBarcode.nInspRectCnt;
				memcpy(sRstAlgo->InspBarRect, pRstBarcode.InspBarRect, sizeof(RECT) * Barcode1DQualityRECT);
				memcpy(sRstAlgo->DetectBarRect, pRstBarcode.DetectBarRect, sizeof(RECT) * Barcode1DQualityRECT);
				for (int i = 0; i < pRstBarcode.nInspRectCnt; i++)
				{
					if (sRstAlgo->DetectBarRect[i].left != 0 && sRstAlgo->DetectBarRect[i].right != 0 &&
						sRstAlgo->DetectBarRect[i].top != 0 && sRstAlgo->DetectBarRect[i].bottom != 0)
						sRstAlgo->nDetectRectCnt++;
				}
			}
			if (pRstBarcode.nDefectRectCnt > 0)
			{
				if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseDefectCondition) == (int)m_eAlgoBar_Data2_UseDefectCondition) 
				{
					RECT rcDefect[Barcode1DDefectRECT];
					int nDefIdx[Barcode1DDefectRECT];
					int nDefCnt = 0;
					for (int i = 0; i < pRstBarcode.nDefectRectCnt; i++)
					{
						float fDefectRectWid = pRstBarcode.InspDefectRect[i].right - pRstBarcode.InspDefectRect[i].left;
						float fDefectRectHei = pRstBarcode.InspDefectRect[i].bottom - pRstBarcode.InspDefectRect[i].top;
						if (fDefectRectWid > fDefSpecWid || fDefectRectHei > fDefSpecLen)
						{
							rcDefect[nDefCnt] = pRstBarcode.InspDefectRect[i];
							nDefIdx[nDefCnt] = pRstBarcode.nDefectRectIndex[i];
							nDefCnt++;
						}
					}
					if (nDefCnt > 0 && nDefCnt >= nDefSpecCnt)
					{
						sRstAlgo->IsDefectQualityOK = FALSE;
						sRstAlgo->nDefectGrade = 0;
					}
					memcpy(sRstAlgo->InspDefectRect, rcDefect, sizeof(RECT) * Barcode1DDefectRECT);
					sRstAlgo->nDefectRectCnt = nDefCnt;
					memcpy(sRstAlgo->nDefectRectIndex, nDefIdx, sizeof(int) * Barcode1DDefectRECT);
				}
				else
				{
					sRstAlgo->IsDefectQualityOK = FALSE;
					sRstAlgo->nDefectGrade = 0;
					memcpy(sRstAlgo->InspDefectRect, pRstBarcode.InspDefectRect, sizeof(RECT) * Barcode1DDefectRECT);
					sRstAlgo->nDefectRectCnt = pRstBarcode.nDefectRectCnt;
					// defect Rect 가없으면 Copy 할 친구도없음.
					memcpy(sRstAlgo->nDefectRectIndex, pRstBarcode.nDefectRectIndex, sizeof(int) * Barcode1DDefectRECT);
				}
			}
			else 
			{
				sRstAlgo->nDefectGrade = 4;
			}
		}
		//	2D 바코드 품질검사 진행시 사용
		if (nDataType == 2 && (bQuality || bQuality2DModule))	//	nDataType == 2 && bQuality
		{
			sRstAlgo->nDefectGrade = pRstBarcode.nDefectGrade;
			if (bQuality && sRstAlgo->nDefectGrade <= algo->m_nArrData[(int)m_eAlgoBarQuality2DGrade])
			{
				sRstAlgo->IsDefectQualityOK = FALSE;
			}
			//for (int i = 0; i < std::size(pRstBarcode.fArrDataMatrixQC); i++)
			//{	// 2D 데이터매트릭스 품질평가 Rst
			//	sRstAlgo->m_fArrRst_QC2D[i] = pRstBarcode.fArrDataMatrixQC[i];
			//}
			// [3] -> Fixed Pattern Damage(QuietZone / Pattern Damaged) / [4] -> Decode([Success/Fail] -> [4/0])
			// 2D 데이터 코드 에러
			//if (sRstAlgo->m_fArrRst_QC2D[3] <= 3.5)
			//{
			//	sRstAlgo->IsDefectQualityOK = FALSE;
			//}
			//if (sRstAlgo->m_fArrRst_QC2D[4] <= 0)
			//{
			//	sRstAlgo->IsDecodeQualityOK = FALSE;
			//}
			if (bQuality2DModule)
				sRstAlgo->nInspRectCnt = pRstBarcode.nInspRectCnt;
			if (pRstBarcode.nDefectRectCnt > 0)
			{
				if ((bQuality2DModule && pRstBarcode.nInspRectCnt >= pAlgoBarcode.n2DDefectCnt) ||
					(bQuality && !bQuality2DModule))
					sRstAlgo->IsDefectQualityOK = FALSE;
				memcpy(sRstAlgo->InspDefectRect, pRstBarcode.InspDefectRect, sizeof(RECT) * Barcode1DDefectRECT);
				sRstAlgo->nDefectRectCnt = pRstBarcode.nDefectRectCnt;
			}
			//else
			//	sRstAlgo->m_fArrRst_QC2D[3] = 4;
		}

		float fConerX, fConerY;
		for (int i = 0; i < 4; i++)
			sRstAlgo->m_ptRst_Coner[i] = pRstBarcode.vCorner[i];
		//if (nDataType == 1)
		//{
		//	InspWrapper->m_PInspAlgo->PInspAlgo::Validate1DBarcodeArea(pImg_buf->m_p2D, sRstAlgo->m_ptRst_Coner, 4);
		//}
		//sRstAlgo->fAngle = pRstBarcode.fAngle;
		// Halcon angle -> DTK angle 맞춰주기 작업. 추후 필요시 수정.	
		float fHalconAngle = pRstBarcode.fAngle;
		float fAngleHalcon2DTK = fHalconAngle < 0 ? -(fHalconAngle) : (360 - fHalconAngle);
		if (fAngleHalcon2DTK < 45 && fAngleHalcon2DTK >= 315)
		{	// └ : BO_LeftToRight
			sRstAlgo->fAngle = fAngleHalcon2DTK - 360.0;
		}
		else if (fAngleHalcon2DTK < 135 && fAngleHalcon2DTK >= 45)
		{	// ┌ : BO_TopToBottom
			sRstAlgo->fAngle = fAngleHalcon2DTK - 90.0;
		}
		else if (fAngleHalcon2DTK <= 225 && fAngleHalcon2DTK > 135)
		{	// ┐ : BO_RightToLeft
			sRstAlgo->fAngle = fAngleHalcon2DTK;
		}
		else if (fAngleHalcon2DTK <= 315 && fAngleHalcon2DTK > 225)
		{	// ┘ : BO_BottomToTop
			sRstAlgo->fAngle = fAngleHalcon2DTK - 270.0;
		}

		if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UsePartialDisplay) == (int)m_eAlgoBar_Data2_UsePartialDisplay)
		{
			int nrPartialPosSt = algo->m_nArrData[(int)m_eAlgoBarPartialFir] - 1;
			int nrPartialPosEd = algo->m_nArrData[(int)m_eAlgoBarPartialSec] - 1;
			int nPartialLength = nrPartialPosEd - nrPartialPosSt + 1;
			sCompareBar = sResult.Mid(nrPartialPosSt, nPartialLength);
		}


		if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseEssentialWords) == (int)m_eAlgoBar_Data2_UseEssentialWords)
		{
			int nEssentialPos = algo->m_nArrData[(int)m_eAlgoBarEssentialWordsPos] - 1;
			CString sEss;
			sEss.Format(_T("%s"), algo->m_sEss);
			sResult = sCompareBar.Mid(nEssentialPos, sEss.GetLength());

			if (sResult != sEss)
			{
				sRstAlgo->m_bOK = FALSE;
			}
		}

		if ((algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseStandardWords) == (int)m_eAlgoBar_Data2_UseStandardWords)
		{
			CString StandardWords;
			StandardWords.Format(_T("%s"), algo->m_sStd);

			if (pRstBarcode.sResultCode != StandardWords/* && algo->m_bUseBarcodeAlign != true*/)
			{
				sRstAlgo->m_bOK = FALSE;
			}
		}

		if (pRstBarcode.sResultCode.GetLength() != 0)
		{
			memcpy(sRstAlgo->m_sBarcode, pRstBarcode.sResultCode.GetBuffer(), pRstBarcode.sResultCode.GetLength() * 2);
			sRstAlgo->m_sBarcode[pRstBarcode.sResultCode.GetLength() * 2] = 0;

			if (sCompareBar.GetLength() != 0)
			{
				memcpy(sRstAlgo->m_sPartialBarcode, sCompareBar.GetBuffer(), sCompareBar.GetLength() * 2);
				sRstAlgo->m_sPartialBarcode[sCompareBar.GetLength() * 2] = 0;
			}
			else
			{
				memset(sRstAlgo->m_sPartialBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
			}
		}
		else
		{
			memset(sRstAlgo->m_sBarcode, 0, sizeof(wchar_t) * MAX_STRLEN);
		}

		if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UseCrossCheck1) == (int)m_eAlgoBar_Data_UseCrossCheck1)
		{
			sRstAlgo->IsCrossCheckOK = CompareTarget(algo, sRstAlgo);
		}
		if ((algo->m_nArrData[(int)m_eAlgoBarData] & (int)m_eAlgoBar_Data_UseCrossCheck2) == (int)m_eAlgoBar_Data_UseCrossCheck2)
		{
			sRstAlgo->IsCrossCheckOK2 = CompareTarget2(algo, sRstAlgo);
		}

		if (algo->m_bUseBarcodeAlign)
		{
			//pRstBarcode.ptCen, pRstBarcode.rtBoundary 은 검사 시 사용된 image의 결과값
			//Teach 대비 결과 Center 비교하여 Align 검출.
			//바코드 설정(DownSize)이 변경 안된다면 상관없음.
			//pRstBarcode.rtBoundary 는 image size로 변경.

			pRstBarcode.rtBoundary.left *= fReSize;
			pRstBarcode.rtBoundary.right *= fReSize;
			pRstBarcode.rtBoundary.top *= fReSize;
			pRstBarcode.rtBoundary.bottom *= fReSize;

			float dImageCenX = pAlgoBarcode.nImgSizeX / 2.0;
			float dImageCenY = pAlgoBarcode.nImgSizeY / 2.0;
			//double dWndAngle = sWndAlgoImg.dAngle;
			//if (dWndAngle == 90 || dWndAngle == 270)
			//{
			//	dImageCenX = TeachBar.imgSizeY / 2.0;
			//	dImageCenY = TeachBar.imgSizeX / 2.0;
			//}
			sRstAlgo->m_bUseAlign = algo->m_bUseBarcodeAlign;

			if (algo->m_bUseTeachBarcodeAlign)
			{
				sRstAlgo->ptCen.x = (dImageCenX - (float)pRstBarcode.ptCen.x) * m_resolX;
				sRstAlgo->ptCen.y = (dImageCenY - (float)pRstBarcode.ptCen.y) * m_resolY;
			}
			else
			{
				if (algo->m_ptTeachCen.x > 0)
					algo->m_ptTeachCen.x *= -1;
				//pInspAlgo->m_ptTeachCen.x *= -1;

				float fTeachPtX = roundf(((float)algo->m_ptTeachCen.x / m_resolX + dImageCenX));
				float fTeachPtY = roundf((-1 * ((float)algo->m_ptTeachCen.y / m_resolY) + dImageCenY));

				float fTempX = (fTeachPtX - (float)pRstBarcode.ptCen.x);
				float fTempY = (fTeachPtY - (float)pRstBarcode.ptCen.y);

				if (fTempX == 0.0f)
					fTempX = 1.0f;
				if (fTempY == 0.0f)
					fTempY = 1.0f;

				sRstAlgo->ptCen.x = fTempX * m_resolX;
				sRstAlgo->ptCen.y = fTempY * m_resolY;
				//sRstAlgo->fAngle = pRstBarcode.fAngle - algo->m_fTeachAngle;	// 티칭 angle과 검사결과 angle 값 차이 확인용
				sRstAlgo->m_rcROI = pRstBarcode.rtBoundary;
#if _DEBUG
				cv::Mat src = cv::Mat(pAlgoBarcode.nImgSizeY, pAlgoBarcode.nImgSizeX, CV_MAKETYPE(CV_8U, 1), pImg_buf->m_p2D->m_pData/*sWndAlgoImg.m_ucArr2D*/);
				cv::Mat rstRect = cv::Mat::zeros(pAlgoBarcode.nImgSizeY, pAlgoBarcode.nImgSizeX, CV_MAKETYPE(CV_8U, 1));
				cv::Rect rt_Rst = cv::Rect(cv::Point(pRstBarcode.rtBoundary.left, pRstBarcode.rtBoundary.top), cv::Point(pRstBarcode.rtBoundary.right, pRstBarcode.rtBoundary.bottom));
				cv::Point pt_Rst = cv::Point((int)pRstBarcode.ptCen.x, (int)pRstBarcode.ptCen.y);
				cv::rectangle(rstRect, rt_Rst, cv::Scalar(255), cv::FILLED);

				cv::Mat pDst;
				cv::Mat pMerImg[3] = { cv::Mat::zeros(cv::Size(src.cols, src.rows), src.type()), src,  rstRect };
				cv::merge(pMerImg, 3, pDst);
#endif
			}
		}
		else
		{
			pRstBarcode.rtBoundary.left *= fReSize;
			pRstBarcode.rtBoundary.right *= fReSize;
			pRstBarcode.rtBoundary.top *= fReSize;
			pRstBarcode.rtBoundary.bottom *= fReSize;

			sRstAlgo->ptCen.x = pRstBarcode.rtBoundary.CenterPoint().x;
			sRstAlgo->ptCen.y = pRstBarcode.rtBoundary.CenterPoint().y;
			sRstAlgo->m_rcROI = pRstBarcode.rtBoundary;

#if _DEBUG
			cv::Mat src = cv::Mat(pAlgoBarcode.nImgSizeY, pAlgoBarcode.nImgSizeX, CV_MAKETYPE(CV_8U, 1), pImg_buf->m_p2D->m_pData/*sWndAlgoImg.m_ucArr2D*/);
			cv::Mat rstRect = cv::Mat::zeros(pAlgoBarcode.nImgSizeY, pAlgoBarcode.nImgSizeX, CV_MAKETYPE(CV_8U, 1));
			cv::Rect rt_Rst = cv::Rect(cv::Point(pRstBarcode.rtBoundary.left, pRstBarcode.rtBoundary.top), cv::Point(pRstBarcode.rtBoundary.right, pRstBarcode.rtBoundary.bottom));
			cv::rectangle(rstRect, rt_Rst, cv::Scalar(255), cv::FILLED);
			cv::Mat pDst;
			cv::Mat pMerImg[3] = { cv::Mat::zeros(cv::Size(src.cols, src.rows), src.type()), src,  rstRect };
			cv::merge(pMerImg, 3, pDst);
#endif // _DEBUG
		}

		if (bPNValidate && nDataType == 1)
		{
			BOOL bSplit = ValidateSplitData(pRstBarcode.sResultCode, nDataType);
			int nChk = 0;
			if (bSplit)
			{	// PN Value Validation
				CString sPNVal = m_MapBarcodeInspTarget.at(_T("PN"));
				memcpy(sRstAlgo->m_sPNValidation, sPNVal.GetBuffer(), sPNVal.GetLength() * 2);
				//if (sResult.Find(sPNVal) == -1)
				//	sRstAlgo->IsPNValidationOK = FALSE;
			}
		}
		if (nDataType == 2 && bPNImgSave && bSaveImage)
		{
			CString sPNCheck = pRstBarcode.sResultCode;
			int nPNL = sPNCheck.Find(_T("(L)"));
			int nPNS = sPNCheck.Find(_T("(S)"));
			int nPNP = sPNCheck.Find(_T("(P)"));
			int nPNC = sPNCheck.Find(_T("(c)"));
			if (nPNL == -1 || nPNS == -1 || nPNP == -1 || nPNC == -1)
			{
				cv::Mat PNImg(nImgHeight, nImgWidth, CV_8UC1, pucImgSrc);
				std::string sBarStr = CT2CA(sResult);
				std::string sImgPath = "D:\\testimage\\" + sBarStr +"_Splitfail.bmp";
				cv::imwrite(sImgPath, PNImg);
			}
		}

		if (sRstAlgo->m_bOK != false && sRstAlgo->IsCrossCheckOK != false && sRstAlgo->IsCrossCheckOK2 != false &&
			sRstAlgo->IsDecodeQualityOK != false && sRstAlgo->IsDefectQualityOK != false && sRstAlgo->IsPNValidationOK != false)
			return true;
		else
			return false;
	}

	bool bReadFailOK = (algo->m_nArrData[(int)m_eAlgoBarData2] & (int)m_eAlgoBar_Data2_UseReadFailOK) == (int)m_eAlgoBar_Data2_UseReadFailOK;
	if (pRstBarcode.sResultCode == "" && bReadFailOK)
	{
		sRstAlgo->m_bOK = TRUE;
		return true;
	}
	
	return false;
}

BOOL CPInsp_AlgoBarcode::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;
	
	RstAlgoBarcode * rst = (RstAlgoBarcode *)sRstAlgo;

	if (inspType == eINSP_MOUNT && rst->m_bUseAlign)
	{
		pAlignRes->offsetX = -rst->ptCen.x;
		pAlignRes->offsetY = rst->ptCen.y;
		pAlignRes->theta = rst->fAngle;
		bRet = TRUE;
	}
	
	return bRet;
}
int CPInsp_AlgoBarcode::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoBarcode::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}

BOOL CPInsp_AlgoBarcode::CompareTarget(AlgoBarcode *pInspAlgo, RstAlgoBarcode *sRstAlgo)
{
	//string sLog = "";
	BOOL bReturn = true;
	CString sMark = _T("");

	CString sSourceFilter = CString(pInspAlgo->m_sSourceFilter);
	CString sSourceRst = _T("");
	bReturn = CrossCheck(sSourceFilter, CString(sRstAlgo->m_sBarcode), sSourceRst);
	
	if (bReturn == false)
		return bReturn;
	

	sMark = CString(pInspAlgo->m_sCrossTarget);
	CString sTargetFilter = CString(pInspAlgo->m_sTargetFilter);
	CString sTargetRst = _T("");
	bReturn = CrossCheck(sTargetFilter, sMark, sTargetRst);
	
	if (bReturn == false)
		return bReturn;

	memcpy(sRstAlgo->m_sCrossSource, sSourceRst.GetBuffer(), sSourceRst.GetLength() * 2);
	memcpy(sRstAlgo->m_sCrossTarget, sTargetRst.GetBuffer(), sTargetRst.GetLength() * 2);

	//sLog = string.Format("SF :{0}, TF :{1}", sSourceRst, sTargetRst);
	//tb.AddLog(sLog);
	if (sSourceRst != sTargetRst)
		bReturn = false;
	
	return bReturn;
}

BOOL CPInsp_AlgoBarcode::CrossCheck(CString sFilter, CString sRst, CString &sResultF)
{
	int nRstSize = sRst.GetLength();
	std::vector<CString> vRst;
	for (int i = 0; i < nRstSize; i++)
		vRst.emplace_back(sRst.GetAt(i));

	sFilter = sFilter.TrimRight(',');
	sFilter = sFilter.TrimRight('/');

	bool bUseWave = false;
	for (int i = 0; i < sFilter.GetLength(); i++)
	{
		if ('~' == sFilter.GetAt(i))
			bUseWave = true;
	}

	std::vector<CString> vArrF;
	bool bUseSplit = false;
	int nSplitCnt = 0;
	for (int i = 0; i < sFilter.GetLength(); i++)
	{
		if ('/' == sFilter.GetAt(i))
		{
			nSplitCnt++;
			bUseSplit = true;
		}
	}

	if (bUseSplit == false)
		vArrF.emplace_back(sFilter);
	else
	{
		CString sTemp = _T("");
		
		for (int i = 0; i <= nSplitCnt; i++)
		{
			AfxExtractSubString(sTemp, sFilter, i, '/');

			if (sTemp != _T(""))
			{
				int nNum = _ttoi(sTemp);
				vArrF.emplace_back(sTemp);
			}
		}
	}

	if (vArrF[0] != "")
	{
		if (bUseWave)
		{
			std::vector<int> listNum = std::vector<int>();
			for (int i = 0; i < vArrF.size(); i++)
			{
				bool bContainWave = false;
				for (int j = 0; j < vArrF[i].GetLength(); j++)
				{
					if ('~' == vArrF[i].GetAt(j))
						bContainWave = true;
				}

				if (bContainWave)
				{
					std::vector<int> vWave;
					CString sTemp = _T("");
					for (int k = 0; k <= 1; k++)
					{
						AfxExtractSubString(sTemp, vArrF[i], k, '~');
					
						if (sTemp != _T(""))
						{
							int nNum = _ttoi(sTemp);
							vWave.emplace_back(nNum);
						}
					}

					int nFront = vWave[0];
					int nBack = vWave[1];
					
					for (int j = nFront; j <= nBack; j++)
					{
						if (sRst.GetLength() > j - 1)
							sResultF += sRst.GetAt(j - 1);
						else
							return false;
					}
				}
			}
		}
		else //기본
		{
			for (int i = 0; i < vArrF.size(); i++)
			{
				int nIndex = _ttoi(vArrF[i]) - 1;
				if (sRst.GetLength() > nIndex)
					sResultF += sRst.GetAt(nIndex);
				else
					return false;
			}
		}
	}

	return true;
}

BOOL CPInsp_AlgoBarcode::CompareTarget2(AlgoBarcode *pInspAlgo, RstAlgoBarcode *sRstAlgo)
{
	//string sLog = "";
	BOOL bReturn = true;
	CString sMark = _T("");

	CString sSourceFilter = CString(pInspAlgo->m_sSourceFilter2);
	CString sSourceRst = _T("");
	bReturn = CrossCheck(sSourceFilter, CString(sRstAlgo->m_sBarcode), sSourceRst);

	if (bReturn == false)
		return bReturn;


	sMark = CString(pInspAlgo->m_sCrossTarget2);
	CString sTargetFilter = CString(pInspAlgo->m_sTargetFilter2);
	CString sTargetRst = _T("");
	bReturn = CrossCheck(sTargetFilter, sMark, sTargetRst);

	if (bReturn == false)
		return bReturn;

	memcpy(sRstAlgo->m_sCrossSource2, sSourceRst.GetBuffer(), sSourceRst.GetLength() * 2);
	memcpy(sRstAlgo->m_sCrossTarget2, sTargetRst.GetBuffer(), sTargetRst.GetLength() * 2);

	//sLog = string.Format("SF :{0}, TF :{1}", sSourceRst, sTargetRst);
	//tb.AddLog(sLog);
	if (sSourceRst != sTargetRst)
		bReturn = false;

	return bReturn;
}

void CPInsp_AlgoBarcode::SetTypeName(int Type, CString &TypeName)
{
	switch (Type) {
	case 0: TypeName.Format(_T("Code39")); break;
	case 1: TypeName.Format(_T("Code93")); break;
	case 2: TypeName.Format(_T("Code128")); break;
	case 3: TypeName.Format(_T("Codabar")); break;
	case 4: TypeName.Format(_T("BC412")); break;
	case 5: TypeName.Format(_T("EAN8")); break;
	case 6: TypeName.Format(_T("EAN13")); break;
	case 7: TypeName.Format(_T("Inter2of5")); break;
	case 8: TypeName.Format(_T("Pharmacode")); break;
	case 9: TypeName.Format(_T("Planet")); break;
	case 10: TypeName.Format(_T("Postnet")); break;
	case 11: TypeName.Format(_T("RSSCODE")); break;
	case 12: TypeName.Format(_T("UPC-A")); break;
	case 13: TypeName.Format(_T("UPC_E")); break;
	case 14: TypeName.Format(_T("DATAMATRIX")); break;
	case 15: TypeName.Format(_T("Maxicode")); break;
	case 16: TypeName.Format(_T("PDF417")); break;
	case 17: TypeName.Format(_T("MICROPDF417")); break;
	case 18: TypeName.Format(_T("QRCode")); break;
	case 19: TypeName.Format(_T("COMPOSITECODE")); break;
	case 20: TypeName.Format(_T("UCC128")); break;
	case 21: TypeName.Format(_T("MICRO_QRCODE")); break;
	default: break;
	}
}

int CPInsp_AlgoBarcode::SetDataType(int type, bool bHALCON)
{	// Barcode Type 으로 구분. 1: 1D Barcode / 2: 2D DataMatrix / 0: HALCON 미지원 타입
	switch (type) {
	case 4: case 9:case 10: case 11:
		if (bHALCON == false)
		{
			return 1;
		}
	case 15:case 17: case 19:
		if (bHALCON == false)
		{
			return 2;
		}
		return 0;

	case 0: case 1: case 2: case 3: case 5: case 6:
	case 7:	case 8: case 12: case 13: case 20:
		return 1;

	case 14: case 16: case 18: case 21:
		return 2;

	default: return 0;
	}
}
int CPInsp_AlgoBarcode::BarcodeSplitInspTarget( RstAlgoBarcode *  rst)
{
    // 처음에 탈때 Clear 
	m_MapBarcodeInspTarget.clear(); 
	std::vector<CString> result;
	// 각 태그 시작 위치
	CString input = ((CString)rst->m_sBarcode); 
	
	if (rst->m_nBarType != 2 || input.IsEmpty()) // 혹시모를 2D 데이터코드가 아니거나 , 빈 값일경우 리턴
		return -1; 
	
	// 나눌 문자들
	int startL = input.Find(_T("(L)")) + 3;
	int startS = input.Find(_T("(S)"));
	int startP = input.Find(_T("(P)"));
	int startC = input.Find(_T("(c)"));

	int start = 0;
	int pos = 0;

	// 각 구간 추출
	CString FirString = input.Mid(startL, startS - startL);
	while ((pos = FirString.Find(_T(' '), start)) != -1 || (pos = FirString.Find(_T('\n'), start)) != -1)
	{
		// 현재 단어 추가
		result.push_back(FirString.Mid(start, pos - start));
		// 다음 단어 시작 위치
		start = pos + 1;
	}
	result.push_back(FirString.Mid(start));
	result.push_back(input.Mid(startS + 3, startP - (startS + 3)));
	result.push_back(input.Mid(startP + 3, startC - (startP + 3)));
	result.push_back(input.Mid(startP + 3, startC - (startP + 4)));
	result.push_back(input.Mid(startC + 3));

	if (result.size() < 7)
	{
		m_MapBarcodeInspTarget.clear();
		return -1; 
	}
		
	
	CString strTemp;
	strTemp.Format(_T("%s %s"), result[0], result[1]);
	m_MapBarcodeInspTarget.insert(std::make_pair(_T("DEVICE"), strTemp));
	m_MapBarcodeInspTarget.insert(std::make_pair(_T("JEDEC"), result[2]));
	m_MapBarcodeInspTarget.insert(std::make_pair(_T("SN"), result[3]));
	m_MapBarcodeInspTarget.insert(std::make_pair(_T("PN"), result[4]));
	m_MapBarcodeInspTarget.insert(std::make_pair(_T("CPN"), result[5]));
	m_MapBarcodeInspTarget.insert(std::make_pair(_T("PROCESSCODE"), result[6]));
	
	for (const auto& pair : m_MapBarcodeInspTarget)
	{
		CString sLog;
		sLog.Format(_T("[PAlgoBarcode_Dll] Key : %s, Value : %s"), pair.first, pair.second);
		g_pMPTI->AddLog_Dev(sLog);
	}

	return 0;

}
void CPInsp_AlgoBarcode::SetInspectWindowPosition(InspAlgoParam algoParam, RstAlgoBarcode * rst)
{
	rst->m_ptWndPos.x = algoParam.m_dx;
	rst->m_ptWndPos.y = algoParam.m_dy;
}
BOOL CPInsp_AlgoBarcode::ValidateSplitData(CString sRstBarcode, int nType)
{
	if (m_MapBarcodeInspTarget.size() < 6) 
	{	// 값이 제대로 들어가있지 않으면 Clear()
		m_MapBarcodeInspTarget.clear();
		return FALSE;
	}

	bool bData = false;
	CString sType;
	CString strCurrentSN;
	for (const auto& pair : m_MapBarcodeInspTarget)
	{	// 빈 값이 있는경우 Clear()
		if (pair.first == "SN") strCurrentSN = pair.second;
		if (pair.second == "") 
		{
			if (sType != "") sType.Append(_T(","));
			sType.Append(pair.first);
			bData = true;
		}
	}
	if (bData)
	{
		m_MapBarcodeInspTarget.clear();
		CString sLog;
		sLog.Format(_T("[PAlgoBarcode_Dll],[%s] Empty. Clear()"), sType);
		g_pMPTI->AddLog_Dev(sLog);
		return FALSE;
	}

	if (sRstBarcode.IsEmpty())
		return FALSE;
	// 바코드 리딩 실패하거나 SN/PN 없으면 리턴
	if (nType == 2)
	{
		CString strRstSN;
		int startS = sRstBarcode.Find(_T("(S)"));
		int startP = sRstBarcode.Find(_T("(P)"));
		if (startS == -1 || startP == -1)
			return FALSE;

		strRstSN = sRstBarcode.Mid(startS + 3, startP - (startS + 3));
		if (strRstSN != strCurrentSN)
		{	// 이전에 검사한 데이터 남아있으면 Clear()
			m_MapBarcodeInspTarget.clear();
		}
	}

	return TRUE;
}
