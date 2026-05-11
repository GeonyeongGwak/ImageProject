#include "PInsp_AlgoTab.h"


CPInsp_AlgoTab::CPInsp_AlgoTab(void)
{
}


CPInsp_AlgoTab::~CPInsp_AlgoTab(void)
{
}

void CPInsp_AlgoTab::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoTab::GetInspAlgoData()
{
	return eSPCAlgoTab;
}

int CPInsp_AlgoTab::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeTab] = e_NG;
	nCurrentNgType = TypeTab;
	return nCurrentNgType;
}

bool CPInsp_AlgoTab::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoTab::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	CPInsp_Tab *pTab = g_pInspMng->GetTabInsp();
	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();
	if (pTab == nullptr)
		return bResult;
	if (sInspAlgo.m_eAlgoType != eAlgoTab)
		return FALSE;
	AlgoTab* pAlgoColor = (AlgoTab *)sInspAlgo.m_ptrInspAlgoParam;
	RECT* rcLeadInsp = g_pMManager->pem_new<RECT>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	rcLeadInsp->left = 0;
	rcLeadInsp->right = 0;
	rcLeadInsp->top = 0;
	rcLeadInsp->bottom = 0;
	AlgoCoordinate coordinateAlgo = *stAlgoParam.m_coordinateAlgo;
#if _DEBUG
	m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("AfterTab2D.bmp"));
	UCHAR *pucImgSrc[3];
	//pucImgSrc[0] = sWndAlgoImg.m_ucArr2D;
	float *pfImgSrc = sWndAlgoImg.m_fArr3D;
	int n2DWidth = sWndAlgoImg.m_nWidth; 
	int n3DWidth = sWndAlgoImg.m_nWidth3D;
	int n2DHeight = sWndAlgoImg.m_nHeight;
	int n3DHeight = sWndAlgoImg.m_nHeight3D;

	cv::Mat _2DImge(n2DHeight, n2DWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D);
	cv::Mat _2DImge1(n2DHeight, n2DWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D_Mix[0]);
	cv::Mat _2DImge2(n2DHeight, n2DWidth, CV_8UC1, sWndAlgoImg.m_ucArr2D_Mix[1]);
	cv::Mat _3DImge2(n2DHeight, n2DWidth, CV_32FC1, pfImgSrc);
#endif
	pTab->SetInspParam(sInspAlgo, sWndAlgoImg, coordinateAlgo, pInspBoardInfo, stAlgoParam.m_nLeadTipPos, rcLeadInsp, sWndAlgoImg.m_nLight_index, sInspImageData , &stTieArea, stAlgoParam.m_nSelectArea);

	pTab->InspProc(ucArrDstImg, stAlgoParam.m_bTeach, (pAlgoColor->m_bUseTabAreaOther && g_pInspMng->m_nTabAlgoIdx > 0));

	int nAlgo = stAlgoParam.m_nAlgo;
	rcLeadInsp->left += stAlgoParam.m_dx;
	rcLeadInsp->right += stAlgoParam.m_dx;
	rcLeadInsp->top += stAlgoParam.m_dy;
	rcLeadInsp->bottom += stAlgoParam.m_dy;
	RstAlgoTab * rstAlgo = (RstAlgoTab *)sRstAlgo;
	
	bResult = pTab->GetInspRst(rstAlgo/*,  ucArrDstImg*/);
	//cv::Mat _2DImgerst(n2DHeight, n2DWidth, CV_8UC1, ucArrDstImg);
	//pTab->GetInspRst(stAlgoParam.m_WndResult->m_vArrRstInspAlgo[nAlgo], *rcLeadInsp);
	if (pAlgoColor && pAlgoColor->IsUseMultiArea())
		g_pInspMng->TabNGRectChangePosition(rstAlgo, nAlgo, stAlgoParam.m_dx, stAlgoParam.m_dy);
	//bResult = stAlgoParam.m_WndResult->m_vArrRstInspAlgo[nAlgo].m_bOk;

	g_pMManager->pem_delete(rcLeadInsp, false);

	return bResult;
}

BOOL CPInsp_AlgoTab::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoTab::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoTab * pInspAlgo = (AlgoTab *)sInspAlgo.m_ptrInspAlgoParam;
	nData = pInspAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase.GetColorData();
	nData |= pInspAlgo->m_sBlobBase_NG.m_sAlgoColorBase.GetColorData();
	nData |= pInspAlgo->m_sBlobBase_NG2.m_sAlgoColorBase.GetColorData();

	if ((nData & COLOR_DATA_USE) == COLOR_DATA_USE)
		nData |= COLOR_DATA_WND;
	if (pInspAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase.m_bUseAngleColor| pInspAlgo->m_sBlobBase_NG.m_sAlgoColorBase.m_bUseAngleColor| pInspAlgo->m_sBlobBase_NG2.m_sAlgoColorBase.m_bUseAngleColor)
		nData |= COLOR_DATA_AC;

	//AlgoColor * pInspAlgo = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
	//
	//
	//if (pInspAlgo->m_bUseColorMap2)
	//	nData |= COLOR_DATA_BIN_AC;
	//if (pInspAlgo->m_byColorLightType == 1)
	//	nData |= COLOR_DATA_MID;

	return nData;
}
bool CPInsp_AlgoTab::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}

