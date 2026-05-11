#include "PInsp_AlgoShapeX.h"

CPInsp_AlgoShapeX::CPInsp_AlgoShapeX(void)
{
}


CPInsp_AlgoShapeX::~CPInsp_AlgoShapeX(void)
{
}

int CPInsp_AlgoShapeX::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	// Allocate a Application.
	//m_milApp = milApp;
	// Allocate a System.
	//m_milSys = milSys;

	//m_fovWidth = fovWidth;
	//m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	//m_bUseImagePilLib = bUseImagePilLib;

	m_procMil = g_pMManager->pem_new<CProcMil_Color>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_procMil->InitMil(milApp, milSys, bUseImagePilLib);
	m_procMil->SetResol(resolX, resolY, fovWidth);

	m_procMil->InitMilAlgoBlob();
	pInsp.InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());


	return ePART_SUCCESS;
}
void CPInsp_AlgoShapeX::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
	m_procMil = g_pMManager->pem_new<CProcMil_Color>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	//m_procMil->InitMil(m_milApp, m_milSys);
	m_procMil->InitMil(g_pMPTI->m_milApp, g_pMPTI->m_milSys, g_pMPTI->isUseImagePilLib());
	m_procMil->SetResol(m_resolX, m_resolY, g_pInspMng->GetFovWidth());

	m_procMil->InitMilAlgoBlob();
	pInsp.InitDevice(g_pMPTI->m_milApp, g_pMPTI->m_milSys, g_pInspMng->GetFovWidth(), g_pInspMng->GetFovLength(), m_resolX, m_resolY, g_pMPTI->isUseImagePilLib());
}

unsigned long long CPInsp_AlgoShapeX::GetInspAlgoData()
{
	return eSPCAlgoShapeX;
}

int CPInsp_AlgoShapeX::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeTab] = e_NG;
	nCurrentNgType = TypeTab;
	return nCurrentNgType;
}

bool CPInsp_AlgoShapeX::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoShapeX::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	//CPInsp_ShapeX *pShapeX = g_pInspMng->GetShapeXInsp();
	//InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();
	//if (pShapeX == nullptr)
	//	return bResult;
	if (sInspAlgo.m_eAlgoType != eAlgoShapeX)
		return FALSE;
	m_sInspImgBuf = *sInspImageData;
	AlgoShapeX* pAlgoColor = (AlgoShapeX *)sInspAlgo.m_ptrInspAlgoParam;
	RECT* rcLeadInsp = g_pMManager->pem_new<RECT>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	rcLeadInsp->left = 0;
	rcLeadInsp->right = 0;
	rcLeadInsp->top = 0;
	rcLeadInsp->bottom = 0;
	AlgoCoordinate coordinateAlgo = *stAlgoParam.m_coordinateAlgo;
	m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("AfterShapeX2D.bmp"));
	//pucImgSrc[0] = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int n2DWidth = sWndAlgoImg.m_nWidth; 
	int n3DWidth = sWndAlgoImg.m_nWidth3D;
	int n2DHeight = sWndAlgoImg.m_nHeight;
	int n3DHeight = sWndAlgoImg.m_nHeight3D;

#if _DEBUG
	cv::Mat _2DImge(n2DHeight, n2DWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D);
	cv::Mat _2DImge1(n2DHeight, n2DWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D_Mix[0]);
	cv::Mat _2DImge2(n2DHeight, n2DWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D_Mix[1]);
	cv::Mat _3DImge2(n2DHeight, n2DWidth, CV_32FC1, pfImgSrc);
#endif
	RstAlgoShapeX * rstAlgo = (RstAlgoShapeX *)sRstAlgo;

	UCHAR *ucWindTabImg = g_pMManager->pem_new<UCHAR>(true, n2DWidth * n2DHeight, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR *ucWindInspImg = g_pMManager->pem_new<UCHAR>(true, n2DWidth * n2DHeight, (PCHAR)__FUNCTION__, __LINE__);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
	//pShapeX->SetInspParam(sInspAlgo, sWndAlgoImg, coordinateAlgo, pInspBoardInfo, stAlgoParam.m_nLeadTipPos, rcLeadInsp, sWndAlgoImg.m_nLight_index, sInspImageData , &stTieArea, stAlgoParam.m_nSelectArea);

	//pShapeX->InspProc(ucArrDstImg, stAlgoParam.m_bTeach, (pAlgoColor->m_bUseShapeAreaOther && g_pInspMng->m_nShapeXAlgoIdx > 0));
	CalcHistoBW(sInspAlgo, sWndAlgoImg, rstAlgo, ucWindTabImg, ucWindInspImg, stAlgoParam.m_nSelectArea, true, false);

	if (ucArrDstImg != NULL)
	{
		memcpy(ucArrDstImg, ucWindInspImg, n2DWidth * n2DHeight);
		cv::Mat mTabImg(n2DHeight, n2DWidth, CV_8UC1, ucWindTabImg);
		cv::Mat DstImg(n2DHeight, n2DWidth, CV_8UC1, ucArrDstImg);
		if (m_nTabIndex > 0)
		{
			cv::Mat DstTabImg = mTabImg.clone();
			cv::dilate(DstTabImg, DstTabImg, kernel);
			cv::erode(DstTabImg, DstTabImg, kernel);
			cv::erode(DstTabImg, DstTabImg, kernel);
			DstImg = DstTabImg & DstImg;
		}
	}
	int nAlgo = stAlgoParam.m_nAlgo;
	rcLeadInsp->left += stAlgoParam.m_dx;
	rcLeadInsp->right += stAlgoParam.m_dx;
	rcLeadInsp->top += stAlgoParam.m_dy;
	rcLeadInsp->bottom += stAlgoParam.m_dy;
	
//	bResult = pShapeX->GetInspRst(rstAlgo/*,  ucArrDstImg*/);
	//if (pAlgoColor && pAlgoColor->IsUseMultiArea())
	//	g_pInspMng->ShapeNGRectChangePosition(rstAlgo, nAlgo, stAlgoParam.m_dx, stAlgoParam.m_dy);

	g_pMManager->pem_delete(rcLeadInsp, false);
	g_pMManager->pem_delete(ucWindTabImg, true);
	g_pMManager->pem_delete(ucWindInspImg, true);

	return bResult;
}
BOOL CPInsp_AlgoShapeX::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	if (InspAlgo.m_eAlgoType != eAlgoShapeX)
		return FALSE;
	if(InspAlgo.m_nMixCount<1)
		return FALSE;
	AlgoShapeX* pAlgoShapeX = (AlgoShapeX *)InspAlgo.m_ptrInspAlgoParam;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	cv::Mat POCRImg = pImg_buf->m_p2D->Mat();
	PIAL::_AlgoShapeX algo2;
	PIAL::_RstAlgoShapeX Rst2;
	RstAlgoShapeX * rstAlgo = (RstAlgoShapeX *)sRstAlgo;
	InspWrapper->ConvertAlgo(pAlgoShapeX, algo2);

	
/*	if (algoParam.m_bTeach)
	{
		InspWrapper->m_PInspAlgo->PInspAlgo::SearchShapeXROI(pImg_buf->m_p2D, pImg_buf->inspWndImage->m_p3D, algo2, &Rst2, 0.3f);
	}
	else */if (algoParam.m_bBin)
	{
		bool bGray = algoParam.m_nBinaryMode == 1 ? true : false;
		bool bBin = algoParam.m_bBin == TRUE ? true : false;

		if (pAlgoShapeX->m_nTypeSelectTarget != 1)
		InspWrapper->m_PInspAlgo->PInspAlgo::InspShapeX(algo2, *pImg_buf, &Rst2, PIALTieArea, algoParam.m_nSelectArea, Mask_buf, InspAlgo.m_nUsedMaskingValue > 0, Mask_buf, false, false);
		else
			InspWrapper->m_PInspAlgo->PInspAlgo::InspShapeX(algo2, *pImg_buf, &Rst2, PIALTieArea, vecAlignResult, algoParam.m_nSelectArea, Mask_buf, InspAlgo.m_nUsedMaskingValue > 0, Mask_buf, false , algoParam.m_nInspType , algo2.nWndId/*algoParam.m_nWndID*/, bGray, bBin);
	}
	else
	{
		if( pAlgoShapeX->m_nTypeSelectTarget != 1)
			InspWrapper->m_PInspAlgo->PInspAlgo::InspShapeX(algo2, *pImg_buf, &Rst2, PIALTieArea, -1, Mask_buf, InspAlgo.m_nUsedMaskingValue > 0, NULL, false, false);
		else
			InspWrapper->m_PInspAlgo->PInspAlgo::InspShapeX(algo2, *pImg_buf, &Rst2, PIALTieArea, vecAlignResult, -1, Mask_buf, InspAlgo.m_nUsedMaskingValue > 0, NULL, false, algoParam.m_nInspType, algo2.nWndId/*algoParam.m_nWndID*/, false);
	}
	InspWrapper->ConvertRstAlgo(&Rst2, rstAlgo);
	BOOL bRst = rstAlgo->isInsp&rstAlgo->isOK;
	g_pInspMng->TabNGRectChangePosition(rstAlgo, algoParam.m_dx, algoParam.m_dy);
	return bRst;
}
BOOL CPInsp_AlgoShapeX::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoShapeX::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoShapeX * pInspAlgo = (AlgoShapeX *)sInspAlgo.m_ptrInspAlgoParam;
	nData = pInspAlgo->m_sBlobBase_ShapeArea.m_sAlgoColorBase.GetColorData();
	nData |= pInspAlgo->m_sBlobBase_NG.m_sAlgoColorBase.GetColorData();
	nData |= pInspAlgo->m_sBlobBase_NG2.m_sAlgoColorBase.GetColorData();

	if ((nData & COLOR_DATA_USE) == COLOR_DATA_USE)
		nData |= COLOR_DATA_WND;
	//if (pInspAlgo->m_sBlobBase_ShapeArea.m_sAlgoColorBase.m_bUseAngleColor| pInspAlgo->m_sBlobBase_NG.m_sAlgoColorBase.m_bUseAngleColor| pInspAlgo->m_sBlobBase_NG2.m_sAlgoColorBase.m_bUseAngleColor)
	//	nData |= COLOR_DATA_AC;

	//AlgoColor * pInspAlgo = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
	//
	//
	//if (pInspAlgo->m_bUseColorMap2)
	//	nData |= COLOR_DATA_BIN_AC;
	//if (pInspAlgo->m_byColorLightType == 1)
	//	nData |= COLOR_DATA_MID;
	if ((pInspAlgo->nInspOption & SHAPEX_OPTION::UseStripeCorrection) == SHAPEX_OPTION::UseStripeCorrection)
		nData |= COLOR_DATA_USE;
	if ((pInspAlgo->nInspOption & SHAPEX_OPTION::UseGradationFilter) == SHAPEX_OPTION::UseGradationFilter)
		nData |= COLOR_DATA_USE;

	return nData;
}
bool CPInsp_AlgoShapeX::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}


bool CPInsp_AlgoShapeX::CalcHistoBW(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, RstAlgoShapeX* sRstAlgo, UCHAR* ucDstImg, UCHAR* ucDsAreatImg, int nTabIdx, bool bTeach, bool bUseTabAreaOther)
{
	if (sWndAlgoImg.m_ucArr2D == NULL || sWndAlgoImg.m_nWidth <= 0 || sWndAlgoImg.m_nHeight <= 0
		|| sWndAlgoImg.m_ucArr2D_Mix[0] == NULL || ucDstImg == NULL)
		return false;

	//if (sWndAlgoImg.m_fArr3D == NULL || sWndAlgoImg.m_nWidth <= 0 || sWndAlgoImg.m_nHeight <= 0
	//	|| sWndAlgoImg.m_fArr3D[0] == NULL || ucDstImg == NULL)

	if (sWndAlgoImg.m_nWidth <= 0 || sWndAlgoImg.m_nHeight <= 0)
		return false;
	if (ucDstImg == NULL)
		return false;
	//티칭 화면 초기화
	if (ucDstImg != NULL)
		memset(ucDstImg, 0, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

	AlgoShapeX* pAlgo = (AlgoShapeX*)sInspAlgo.m_ptrInspAlgoParam;
	int* nArrValue = g_pMManager->pem_new<int>(true, 255, (PCHAR)__FUNCTION__, __LINE__, true);

	UCHAR *pUcImgBinaryMask = g_pMManager->pem_new<UCHAR>(true, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR *ucColorBin = g_pMManager->pem_new<UCHAR>(true, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(ucColorBin, 255, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

	int nMixCnt = 1;
	bool bEnableNG2 = pAlgo->m_sBlobBase_NG2.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_eUse;
	bool bUse2D_NG2 = (pAlgo->m_sBlobBase_NG2.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2D) == m_enBlobBase_Data::m_enBlobBase_Data_e2D ? true : false;
	bool bUse3D_NG2 = (pAlgo->m_sBlobBase_NG2.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3D) == m_enBlobBase_Data::m_enBlobBase_Data_e3D ? true : false;
	bool bUseColor_NG2 = pAlgo->m_sBlobBase_NG2.m_sAlgoColorBase.m_bUseColor == 1 ? true : false;
	if (bEnableNG2 == true && (bUse2D_NG2 == true || bUse3D_NG2 == true || bUseColor_NG2 == true))
		nMixCnt = 2;

	//Teaching UI 에서 4번 탭 선택했는데 해당탭의 이진화 티징데이터가 없을때 예외처리
	if (nTabIdx == 2 && nMixCnt == 1)
	{
		//해제
		g_pMManager->pem_delete(nArrValue, true);
		g_pMManager->pem_delete(pUcImgBinaryMask, true);
		g_pMManager->pem_delete(ucColorBin, true);

		return false;
	}

	UCHAR* ucMix[2];
	for (int n = 0; n < nMixCnt; n++)
	{
		ucMix[n] = g_pMManager->pem_new<UCHAR>(true, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight, (PCHAR)__FUNCTION__, __LINE__);
		memset(ucMix[n], 0, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
	}

	POINT posCen;
	posCen.x = sWndAlgoImg.m_nWidth / 2;
	posCen.y = sWndAlgoImg.m_nHeight / 2;
	AlgoBlob algoBlob = pInsp.SetAlgoBlob(sInspAlgo);
	CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();

	TotalInspExceptArea stTieAreaNULL;
	stTieAreaNULL.m_nUsedInspPolygon = 0;
	stTieAreaNULL.m_nUsedMaskingValue = 0;
	stTieAreaNULL.m_nUsedWndPolygon = 0;
	double dCX = 0;
	double dCY = 0;
	double dArea = 0;
	CRect rcBlob(0, 0, 0, 0);
#if 0
	//형상이미지 ColorBW

	//형상 이미지 BW
	if (pAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase.m_bUseColor)
	{
		pColorTeach->GetColorBaseBinTab(&pAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase, m_sInspImgBuf, ucColorBin);

		cv::Mat binDstTab(m_sInspImgBuf.nImageSizeY, m_sInspImgBuf.nImageSizeX, CV_8UC1, ucColorBin);
		m_pProcMilAlgo->SaveWorkImg(ucColorBin, m_sInspImgBuf.nImageSizeX, m_sInspImgBuf.nImageSizeY, _T("OrgColor_Clip.bmp"));
	}
	int nCntBlob = CPInsp::BlobImageStruct(algoBlob, sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_fArr3D, ucColorBin,
		sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, 100/*pAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_MinArea]*/,
		&dArea, &dCX, &dCY, &rcBlob, pUcImgBinaryMask, stTieAreaNULL, algoBlob.m_bFillHole);
#else

	//형상이미지 ColorBW
	//형상 이미지 BW
	if (pAlgo->m_sBlobBase_ShapeArea.m_sAlgoColorBase.m_bUseColor)
	{
		pColorTeach->GetColorBaseBinTab(&pAlgo->m_sBlobBase_ShapeArea.m_sAlgoColorBase, m_sInspImgBuf, ucColorBin, CRect(0, 0, 0, 0), m_nTabIndex > -1, pAlgo->m_sBlobBase_ShapeArea.m_sAlgoColorBase.m_bUseAngleColor);

		cv::Mat binDstTab(m_sInspImgBuf.nImageSizeY, m_sInspImgBuf.nImageSizeX, CV_8UC1, ucColorBin);
		m_pProcMilAlgo->SaveWorkImg(ucColorBin, m_sInspImgBuf.nImageSizeX, m_sInspImgBuf.nImageSizeY, _T("OrgColor_Clip.bmp"));
	}
	cv::Mat TabArea(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1/*, pUcImgBinaryMask*/);

	if (bUseTabAreaOther && g_pInspMng->m_pucTabAreaTmp != nullptr)
	{
		TabArea.data = g_pInspMng->m_pucTabAreaTmp;
		memcpy(pUcImgBinaryMask, g_pInspMng->m_pucTabAreaTmp, sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
	}
	else
	{
		TabArea.data = pUcImgBinaryMask;
		TabArea.setTo(0);
		for (int cnt = 0; cnt < 1; cnt++)
		{

			CRect rt = CRect(0,0, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);

			if (rt.left < 0)rt.left = 0;
			if (rt.top < 0)rt.top = 0;
			if (rt.right > sWndAlgoImg.m_nWidth - 1)rt.right = sWndAlgoImg.m_nWidth - 1;
			if (rt.bottom > sWndAlgoImg.m_nHeight - 1)rt.bottom = sWndAlgoImg.m_nHeight - 1;

			int nWidth = rt.Width();
			int nHeight = rt.Height();
			if (nWidth < 1)
				continue;
			if (nHeight < 1)
				continue;
			UCHAR *pUcDst = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			UCHAR *pUcSrc = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_procMil->GetClipImage_LT(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, pUcSrc, rt.left, rt.top, nWidth, nHeight);
			FLOAT *pfSrc = NULL;
			if (algoBlob.m_bInsp3D)
			{
				pfSrc = g_pMManager->pem_new<float>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
				m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D, pfSrc, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, rt.left, rt.top, nWidth, nHeight);
			}
			UCHAR *pUcColorSrc = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			m_procMil->GetClipImage_LT(ucColorBin, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, pUcColorSrc, rt.left, rt.top, nWidth, nHeight);
			int nCntBlob = pInsp.BlobImageStruct(algoBlob, pUcSrc, pfSrc, pUcColorSrc,
				nWidth, nHeight, 100/*pAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_MinArea]*/,
				&dArea, &dCX, &dCY, &rcBlob, pUcDst, stTieAreaNULL, algoBlob.m_bFillHole);

			cv::Mat Rst2D(nHeight, nWidth, CV_8UC1, pUcDst);
			Rst2D.copyTo(TabArea(cv::Rect(rt.left, rt.top, nWidth, nHeight)));

			g_pMManager->pem_delete(pUcDst, true);
			g_pMManager->pem_delete(pUcSrc, true);
			if (algoBlob.m_bInsp3D)
				g_pMManager->pem_delete(pfSrc, true);
			g_pMManager->pem_delete(pUcColorSrc, true);
		}
	}
#endif
#if SAVE_TAB_IMG
	CString imgPath;
	cv::Mat GrayImage(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D_Mix[0]);
	imgPath.Format(_T("D:\\testimage\\%d_1GrayImg.bmp"), sInspAlgo.m_nAlgoId);
	cv::imwrite(std::string(CT2A(imgPath)), GrayImage);
	cv::Mat TabImage(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, pUcImgBinaryMask);
	imgPath.Format(_T("D:\\testimage\\%d_1TabImage.bmp"), sInspAlgo.m_nAlgoId);
	cv::imwrite(std::string(CT2A(imgPath)), TabImage);
	if (nMixCnt > 1)
	{
		cv::Mat GrayImage2(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D_Mix[1]);
		imgPath.Format(_T("D:\\testimage\\%d_1GrayImg2.bmp"), sInspAlgo.m_nAlgoId);
		cv::imwrite(std::string(CT2A(imgPath)), GrayImage2);
	}
#endif

	for (int i = 0; i < nMixCnt; i++)
	{
		cv::Mat MixImg(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucMix[i]);
		tagAlgoBlobBase stHistoBW;
		int nTeachHist = 0;
		if (sWndAlgoImg.m_ucArr2D_Mix[i] == NULL)
			continue;
		if (i == 0)
		{
			stHistoBW = pAlgo->m_sBlobBase_NG;
			nTeachHist = pAlgo->nHist1;
		}
		else
		{
			stHistoBW = pAlgo->m_sBlobBase_NG2;
			nTeachHist = pAlgo->nHist2;
		}
		UCHAR *ucColorImgDst = NULL;

		bool bUseColor = (pColorTeach != NULL && stHistoBW.m_sAlgoColorBase.m_bUseColor == TRUE) ? true : false;
		bool bUse2D = (stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2D) == m_enBlobBase_Data::m_enBlobBase_Data_e2D ? true : false;
		bool bUse3D = (stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3D) == m_enBlobBase_Data::m_enBlobBase_Data_e3D ? true : false;

		//for (int cnt = 0; cnt < pAlgo->m_nCntRect; cnt++)
		//{
		//	//버퍼 초기화
		//	CRect rt = pAlgo->m_rcArrTabRect[cnt];
		//	rt += posCen;

		//	if (rt.left < 0)rt.left = 0;
		//	if (rt.top < 0)rt.top = 0;
		//	if (rt.right > sWndAlgoImg.m_nWidth - 1)rt.right = sWndAlgoImg.m_nWidth - 1;
		//	if (rt.bottom > sWndAlgoImg.m_nHeight - 1)rt.bottom = sWndAlgoImg.m_nHeight - 1;

		//	int nWidth = rt.Width();
		//	int nHeight = rt.Height();

		//	if (nWidth < 1)
		//		continue;
		//	if (nHeight < 1)
		//		continue;

		//	UCHAR *pUcDst2D = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
		//	UCHAR *pUcDst3D = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);

		//	if (bUse2D)
		//	{
		//		//histo
		//		int nHistoValue = Histogram(sWndAlgoImg.m_ucArr2D_Mix[i], pUcImgBinaryMask, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, rt);

		//		UCHAR *pUcSrc = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
		//		m_procMil->GetClipImage_LT(sWndAlgoImg.m_ucArr2D_Mix[i], sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, pUcSrc, rt.left, rt.top, nWidth, nHeight);

		//		int nMix2D = stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Min2D] - nTeachHist + nHistoValue > 255 ? 255 : stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Min2D] - nTeachHist + nHistoValue;
		//		int nMax2D = stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Max2D] - nTeachHist + nHistoValue > 255 ? 255 : stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Max2D] - nTeachHist + nHistoValue;
		//		int nRange = eTypeRangeOut;
		//		if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeIN) != 0)
		//			nRange = eTypeRangeIn;
		//		else if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeUP) != 0)
		//			nRange = eTypeRangeUpper;
		//		else if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeLo) != 0)
		//			nRange = eTypeRangeLower;
		//		if (nTeachHist == -1)
		//		{
		//			nMix2D = stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Min2D];
		//			nMax2D = stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Max2D];
		//		}
		//		if (nMix2D < 0) nMix2D = 0;
		//		if (nMax2D < 0) nMax2D = 0;
		//		pInsp.Binarize(pUcSrc, nWidth, nHeight, nRange, nMix2D, nMax2D, false, pUcDst2D);

		//		if (bUse3D == false)
		//		{
		//			cv::Mat Rst2D(nHeight, nWidth, CV_8UC1, pUcDst2D);
		//			Rst2D.copyTo(MixImg(cv::Rect(rt.left, rt.top, nWidth, nHeight)));
		//		}

		//		if (i == 0)
		//			sRstAlgo->m_nArrMaxFreqValue1[cnt] = nHistoValue;
		//		else
		//			sRstAlgo->m_nArrMaxFreqValue2[cnt] = nHistoValue;

		//		g_pMManager->pem_delete(pUcSrc, true);
		//	}
		//	if (bUse3D)
		//	{
		//		FLOAT *pfSrc = g_pMManager->pem_new<float>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
		//		m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D, pfSrc, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, rt.left, rt.top, nWidth, nHeight);

		//		float fMin3D = stHistoBW.m_fArrValue[m_efBlobBase::m_efBlobBase_Min3D];
		//		float nMax3D = stHistoBW.m_fArrValue[m_efBlobBase::m_efBlobBase_Max3D];
		//		int nRange = eTypeRangeOut;
		//		if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeIN) != 0)
		//			nRange = eTypeRangeIn;
		//		else if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeUP) != 0)
		//			nRange = eTypeRangeUpper;
		//		else if ((stHistoBW.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeLo) != 0)
		//			nRange = eTypeRangeLower;

		//		pInsp.Binarize(pfSrc, nWidth, nHeight, nRange, fMin3D, nMax3D, false, pUcDst3D);

		//		if (bUse2D == false)
		//		{
		//			cv::Mat Rst3D(nHeight, nWidth, CV_8UC1, pUcDst3D);
		//			Rst3D.copyTo(MixImg(cv::Rect(rt.left, rt.top, nWidth, nHeight)));
		//		}

		//		g_pMManager->pem_delete(pfSrc, true);
		//	}

		//	if (bUse2D == true && bUse3D == true)
		//	{
		//		cv::Mat Rst2D(nHeight, nWidth, CV_8UC1, pUcDst2D);
		//		cv::Mat Rst3D(nHeight, nWidth, CV_8UC1, pUcDst3D);

		//		cv::bitwise_and(Rst2D, Rst3D, Rst2D);
		//		Rst2D.copyTo(MixImg(cv::Rect(rt.left, rt.top, nWidth, nHeight)));
		//	}


		//	g_pMManager->pem_delete(pUcDst2D, true);
		//	g_pMManager->pem_delete(pUcDst3D, true);
		//}
		//
		memset(ucColorBin, 0, sWndAlgoImg.m_nWidth* sWndAlgoImg.m_nHeight);
		cv::Mat ColorImg(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucColorBin);

		if (bUseColor)
		{
			pColorTeach->GetColorBaseBinTab(&stHistoBW.m_sAlgoColorBase, m_sInspImgBuf, ucColorBin, CRect(0, 0, 0, 0), m_nTabIndex > -1, stHistoBW.m_sAlgoColorBase.m_bUseAngleColor);

			if (bUse2D == true || bUse3D == true)	//2D, 3D 하나라도 사용하면 위에 이진화 결과에 컬러를 합쳐야함
				cv::bitwise_and(MixImg, ColorImg, MixImg);
			else                                    //컬러 단독으로 이진화 하는경우
				ColorImg.copyTo(MixImg);
		}

		/*if (pColorTeach != NULL && stHistoBW.m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			cv::Mat ColorBin(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucColorImgDst);
			cv::Mat GrayBin(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucMix[i]);
			if (bUse2D | bUse3D)
				cv::bitwise_and(GrayBin, ColorBin, GrayBin);
			else
				ColorBin.copyTo(GrayBin);
			Delete_1DArray(&ucColorImgDst);
		}*/
	}


	//1번 2번 mix 이미지 AND
	//TabIdx : 0 = 모두합친결과, 1 = Tab영역, 2 = 이물 조명1, 3 = 이물 조명2
	switch (nTabIdx)
	{
	case 3:
	case -1:
		if (nMixCnt > 1)
		{
			//1번 버퍼에 결과 몰아줌
			cv::Mat Rst1(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucMix[0]);
			cv::Mat Rst2(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1, ucMix[1]);
			cv::bitwise_and(Rst1, Rst2, Rst1);
		}
		//최종 BW 영상 return 
		memcpy(ucDstImg, ucMix[0], sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
		break;
	case 0:
		memcpy(ucDstImg, pUcImgBinaryMask, sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
		break;
	case 1:
		memcpy(ucDstImg, ucMix[0], sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
		break;
	case 2:
		memcpy(ucDstImg, ucMix[1], sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);
		break;
	}

	//Area 영상 return 
	memcpy(ucDsAreatImg, pUcImgBinaryMask, sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);

	if (g_pInspMng->m_nTabAlgoIdx == 0 && g_pInspMng->m_pucTabAreaTmp != nullptr)
		memcpy(g_pInspMng->m_pucTabAreaTmp, pUcImgBinaryMask, sWndAlgoImg.m_nHeight* sWndAlgoImg.m_nWidth);

	//해제
	g_pMManager->pem_delete(nArrValue, true);
	g_pMManager->pem_delete(pUcImgBinaryMask, true);
	g_pMManager->pem_delete(ucColorBin, true);
	for (int n = 0; n < nMixCnt; n++)
		g_pMManager->pem_delete(ucMix[n], true);

	return true;
}

int CPInsp_AlgoShapeX::Histogram(UCHAR* ucSrc, UCHAR* ucMask, int nWidth, int nHeight, CRect rtRoi)
{
	//roi 안넣으면 전체이미지 히스토그램 계산
	//roi 넣으면 부분만 히스토그램 계산
	int nMaxLoc = 0;

	if (ucSrc == NULL || ucMask == NULL
		|| nWidth <= 0 || nHeight <= 0)
		return nMaxLoc;

	int nStX = 0;
	int nStY = 0;
	int nRoiWidth = nWidth;
	int nRoiHeight = nHeight;

	std::vector<int> vArrValue;
	vArrValue.resize(256, 0);

	if (rtRoi.Width() >= 0 || rtRoi.Height() >= 0)
	{
		nStX = rtRoi.left, nStY = rtRoi.top;
		nRoiWidth = rtRoi.Width(), nRoiHeight = rtRoi.Height();
	}
	int nWhitePixel = 0;
	int nSum = 0;
	for (int y = nStY; y < nStY + nRoiHeight; y++)
	{
		for (int x = nStX; x < nStX + nRoiWidth; x++)
		{
			if (ucMask[y * nWidth + x] == 255)
			{
				int nIdx = ucSrc[y * nWidth + x];
				vArrValue[nIdx] += 1;
				nWhitePixel++;
				nSum += nIdx;
			}
		}
	}
	float fDiv;
	float avg = nWhitePixel == 0 ? 0 : nSum / nWhitePixel;
	for (int nidx = 0; nidx < vArrValue.size(); nidx++)
		fDiv = std::abs(avg - vArrValue[nidx]);
	if (nWhitePixel != 0)fDiv /= nWhitePixel;
	if (fDiv > 0.005)
		nMaxLoc = std::max_element(vArrValue.begin() + (int)avg, vArrValue.end()) - vArrValue.begin();
	else
		nMaxLoc = std::max_element(vArrValue.begin(), vArrValue.end()) - vArrValue.begin();

	return nMaxLoc;
}
BOOL CPInsp_AlgoShapeX::ShapeX_LoadImageFile(CString sPath)
{
	BOOL rst = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	rst = InspWrapper->m_PInspAlgo->ShapeX_LoadImageFile(sPath);
	return rst;
}
int CPInsp_AlgoShapeX::GetModelCnt(wchar_t* arrPath, double dResolX, double dResolY) 
{
	int rst = 0;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	rst = InspWrapper->m_PInspAlgo->GetModelCnt(arrPath, dResolX, dResolX);
	return rst;
}
BOOL CPInsp_AlgoShapeX::ModelMaskChange(cv::Mat& newMask, wchar_t* arrPath, int nIdx)
{
	BOOL Rst = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	Rst = InspWrapper->m_PInspAlgo->ModelMaskChange(newMask, arrPath, nIdx);
	return Rst;
}
BOOL CPInsp_AlgoShapeX::ModelDelete(wchar_t* arrPath, int nIdx)
{
	BOOL Rst = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	Rst = InspWrapper->m_PInspAlgo->ModelDelete(arrPath, nIdx);
	return Rst;
}
BOOL CPInsp_AlgoShapeX::ModelAdd(wchar_t* arrPath, PIAL::PI_Buff* Src, PIAL::PI_Buff* p3D, PIAL::PI_Buff* pBin, PIAL::_Bin bin)
{
	BOOL Rst = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	Rst = InspWrapper->m_PInspAlgo->ModelAdd(arrPath, Src, pBin, p3D, bin, 0.3f, FALSE);
	return Rst;
}

BOOL CPInsp_AlgoShapeX::ModelAddWithType(wchar_t* arrPath, PIAL::PI_Buff* Src, PIAL::PI_Buff* p3D, PIAL::PI_Buff* pBin, PIAL::_Bin bin, int targetType, bool searchByShape)
{
	BOOL Rst = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	//Rst = InspWrapper->m_PInspAlgo->PInspAlgo::ModelAddWithType(arrPath, Src, pBin, p3D, bin, g_pInspMng->m_dShapeXMarginX, targetType);
	Rst = InspWrapper->m_PInspAlgo->ModelAddWithType(arrPath, Src, pBin, p3D, bin, 0.3f, targetType, searchByShape, FALSE);
	return Rst;
}

BOOL CPInsp_AlgoShapeX::AlgoCnt(int nCnt)
{
	BOOL Rst = FALSE;
	m_nModelListCnt = nCnt;
	return Rst;
}
BOOL CPInsp_AlgoShapeX::LoadModelList(SShapeXfileAlgoPath* ShapeXPath, bool bMultiProcessLoad) // bMultiProcessLoad MultiProcess 가 로드할때 True
{
	BOOL Rst = FALSE;
	CString sLog;
	sLog.Format(_T("[ShapeX]LoadModelList start"));
	g_pMPTI->AddLog(sLog);
	PIAL::_sShapeXList* sFileList = nullptr;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	if (bMultiProcessLoad && ShapeXPath ==nullptr)
	{
		std::vector<std::tuple<BYTE*, int>> inputBuffer;
		int nModelCnt = ext::InspRoot_Server::get()->_InspStatus.ModelTotalCount(ext::ModelBufferFlag::eExtModel_ShapeX); // Front Rear의 개수 
		sFileList = new PIAL::_sShapeXList[nModelCnt];
		//PIAL::_sShapeXList* sFileList = new PIAL::_sShapeXList[nModelCnt];
		for (int i = 0; i < nModelCnt; i++)
		{
			ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_ShapeXModelCtrl.First()[i];
			memcpy(sFileList[i].m_sFilePath, pCtrl.sFilePath, sizeof(wchar_t) * MAX_STRLEN - 1);
			inputBuffer.push_back(std::make_tuple(ext::InspRoot_Server::get()->_ShapeXModelBuffer.Lock(pCtrl.img) , pCtrl.img.imgSz) );
		}
		Rst = InspWrapper->m_PInspAlgo->LoadExtFileList(sFileList, m_resolX, m_resolY, inputBuffer);
	}
	else
	{
		sFileList = new PIAL::_sShapeXList[m_nModelListCnt];
		//PIAL::_sShapeXList* sFileList = new PIAL::_sShapeXList[m_nModelListCnt];
		for (int i = 0; i < m_nModelListCnt; i++)
		{
			memcpy(sFileList[i].m_sFilePath, ShapeXPath[i].m_sPathModelPath, sizeof(wchar_t) * MAX_STRLEN);
		}

			if (g_pMPTI->GetUseMultiProcess())
			{
				auto wrapper = [](CFile* file, CString name, int mode) -> bool
				{
					return ext::InspRoot_Server::get()->ExtModelAdd(file, name, mode);// 람다로 멤버 함수 감싸기
				};
				Rst = InspWrapper->m_PInspAlgo->LoadFileList(sFileList, m_nModelListCnt, m_resolX, m_resolY, wrapper);
			}
			else
		Rst = InspWrapper->m_PInspAlgo->LoadFileList(sFileList, m_nModelListCnt, m_resolX, m_resolY, nullptr);
	}
	sLog.Format(_T("[ShapeX] LoadModelList End"));
	g_pMPTI->AddLog(sLog);
	delete[]sFileList;
	return Rst;
}
BOOL CPInsp_AlgoShapeX::CheckModelList(SShapeXfileAlgoPath* ShapeXPath , bool bMultiProcessLoad) // bMultiProcessLoad MultiProcess 가 로드할때 True
{
	BOOL Rst = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	PIAL::_sShapeXList* sFileList = nullptr;
	if (bMultiProcessLoad)
	{
		std::vector<std::tuple<BYTE*, int>> inputBuffer;
		int nModelCnt = ext::InspRoot_Server::get()->_InspStatus.ModelTotalCount(ext::ModelBufferFlag::eExtModel_ShapeX);
		sFileList = new PIAL::_sShapeXList[nModelCnt];
		for (int i = 0; i < nModelCnt; i++)
		{
			ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_ShapeXModelCtrl.First()[i];
			memcpy(sFileList[i].m_sFilePath, pCtrl.sFilePath, sizeof(wchar_t) * MAX_STRLEN - 1);
			inputBuffer.push_back(std::make_tuple(ext::InspRoot_Server::get()->_ShapeXModelBuffer.Lock(pCtrl.img), pCtrl.img.imgSz));
		}
	}
	else
	{
		sFileList = new PIAL::_sShapeXList[m_nModelListCnt];
	for (int i = 0; i < m_nModelListCnt; i++)
	{
		memcpy(sFileList[i].m_sFilePath, ShapeXPath[i].m_sPathModelPath, sizeof(wchar_t) * MAX_STRLEN);
	}
	if (g_pMPTI->GetUseMultiProcess())
	{
		auto wrapper = [](CFile* file, CString name, int mode) -> bool
		{
			return ext::InspRoot_Server::get()->ExtModelAdd(file, name, mode);// 람다로 멤버 함수 감싸기
		};
		Rst = InspWrapper->m_PInspAlgo->CheckFileList(sFileList, m_nModelListCnt, m_resolX, m_resolY, wrapper);
	}
	else
	Rst = InspWrapper->m_PInspAlgo->CheckFileList(sFileList, m_nModelListCnt, m_resolX, m_resolY, nullptr);
	}
	delete[]sFileList;
	return Rst;
}
BOOL CPInsp_AlgoShapeX::clearModelList(int nLane)
{
	BOOL Rst = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	Rst = InspWrapper->m_PInspAlgo->ClearFileList(nLane);
	return Rst;
}
BOOL CPInsp_AlgoShapeX::DrawShapeXModel(PIAL::PI_Buff* Src, PIAL::PI_Buff* dst, AlgoShapeX* palgoShapeX, int* ModelIdx, POINTF* poArrCenter)
{
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	PIAL::_AlgoShapeX algo2;
	InspWrapper->ConvertAlgo(palgoShapeX, algo2);
	InspWrapper->m_PInspAlgo->PInspAlgo::DrawShapeXModel(Src, dst, algo2, ModelIdx, poArrCenter);
	return true;
}
BOOL CPInsp_AlgoShapeX::DrawShapeXModelMatching(PIAL::PI_Buff* Src, PIAL::PI_Buff* dst, AlgoShapeX* palgoShapeX, int ModelIdx, int SeleteROI)
{
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	PIAL::_AlgoShapeX algo2;
	InspWrapper->ConvertAlgo(palgoShapeX, algo2);
	InspWrapper->m_PInspAlgo->PInspAlgo::DrawShapeXModelMatching(Src, dst, algo2, ModelIdx, SeleteROI);
	return true;
}
int CPInsp_AlgoShapeX::ShapeXModelKey(int ModelIndex)
{
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	return InspWrapper->m_PInspAlgo->PInspAlgo::ShapeXModelKey(ModelIndex);
}