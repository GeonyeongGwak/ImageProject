#include "PInsp_AlgoGWire.h"

//★★ 코드 작성 필요
CPInsp_AlgoGWire::CPInsp_AlgoGWire(void)
{
}


CPInsp_AlgoGWire::~CPInsp_AlgoGWire(void)
{
}

void CPInsp_AlgoGWire::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoGWire::GetInspAlgoData()
{
	unsigned long long ret = -1;
	ret = eSPCAlgoGWire;
	//shw Delete Xret = eSPCAlgoGWire;
	return ret;
}

int CPInsp_AlgoGWire::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	//★★ 코드 수정 필요
	int nCurrentNgType = 0;

	RstAlgoGWire * rstAlgo = (RstAlgoGWire *)vRstInspAlgo;
	if (rstAlgo->m_bCntOK == FALSE)
	{
		eWholeNgTypeTemp[TypeUserDefine] = e_NG;
		nCurrentNgType = TypeUserDefine;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoGWire::InspWindowArea(int nType)
{
	//★★ 코드 수정 필요
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoGWire::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	//★★ 코드 수정 필요
	BOOL bResult = FALSE;
// 
// 	if (sInspAlgo.m_eAlgoType != eAlgoGWire)
// 		return bResult;
// 
// 	RstAlgoGWire * rst = (RstAlgoGWire *)sRstAlgo;
// 	memset(rst, 0, (sizeof(RstAlgoGWire)));
// 
// 	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
// 	PIAL::_RstAlgoGWire RstAlgo2;
// 	PIAL::_AlgoGWire Algo2;
// 
// 	InspWrapper->ConvertAlgo((AlgoGWire*)sInspAlgo.m_ptrInspAlgoParam, Algo2);
// 
// 	PIAL::Insp_Image Img_buf;
// 	int nImgWidth = sWndAlgoImg.m_nWidth;
// 	int nImgHeight = sWndAlgoImg.m_nHeight;
// 	Img_buf.inspPartImage->nImgSizeX = nImgWidth;
// 	Img_buf.inspPartImage->nImgSizeY = nImgHeight;
// 
// 	Img_buf.inspWndImage->nImgSizeX = nImgWidth;
// 	Img_buf.inspWndImage->nImgSizeY = nImgHeight;
// 	Img_buf.m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArr2D, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
// 	Img_buf.inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D, nImgWidth, nImgHeight, true);
// 	Img_buf.inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D, nImgWidth, nImgHeight, true);
// 
// 	Img_buf.inspPartImage->m_fPartRoundingErrX = sWndAlgoImg.m_fPartRoundingErrX;
// 	Img_buf.inspPartImage->m_fPartRoundingErrY = sWndAlgoImg.m_fPartRoundingErrY;
// 
// 	int nPartWidth = stAlgoParam.m_sPartAlgoImg->m_nWidth;
// 	int nPartHeight = stAlgoParam.m_sPartAlgoImg->m_nHeight;
// 	if (ucArrDstImg == NULL)
// 	{
// 		ucArrDstImg = g_pMManager->pem_new<uchar>(true, nPartWidth * nPartHeight, __FUNCTION__, __LINE__);
// 		memset(ucArrDstImg, 0, nPartWidth * nPartHeight * sizeof(uchar));
// 	}
// 
// 	PIAL::PI_Buff dst(ucArrDstImg, nPartWidth, nPartHeight, nPartWidth, 1, 1, true);
// 	PIAL::PI_Buff *tmpDst = dst.ClipBuff_LT(stAlgoParam.m_nStartX, stAlgoParam.m_nStartY, nImgWidth, nImgHeight);
// 
// #ifdef _DEBUG
// 	cv::Mat tmpDebug2D = Img_buf.m_p2D->Mat();
// 	cv::Mat tmpDebugDstImg = tmpDst->Mat();
// #endif
// 
// 	if (stAlgoParam.m_bInspection == TRUE)
// 	{
// 		std::vector<PIAL::_AlignResult> vecAlignResult;
// 		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::InspGWire(Algo2, Img_buf, &RstAlgo2, vecAlignResult);
// 		InspWrapper->ConvertRstAlgo(RstAlgo2, (RstAlgoGWire*)sRstAlgo);
// 	}
// 	else if (stAlgoParam.m_bTeach == TRUE)
// 	{
// 		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::TeachGWire(Algo2, Img_buf, &RstAlgo2, tmpDst);
// 		InspWrapper->ConvertRstAlgo(RstAlgo2, (RstAlgoGWire*)sRstAlgo);
// 	}
// 	else if (stAlgoParam.m_bBin == TRUE)
// 	{
// 		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::BinalizeGWire(Algo2, Img_buf, tmpDst);
// 	}
// 
// 	((RstAlgoGWire*)sRstAlgo)->m_nCenterMarginX = stAlgoParam.m_nStartX + nImgWidth / 2.0 + 0.5;
// 	((RstAlgoGWire*)sRstAlgo)->m_nCenterMarginY = stAlgoParam.m_nStartY + nImgHeight / 2.0 + 0.5;
// 	((RstAlgoGWire*)sRstAlgo)->m_nCenterMarginW = ((RstAlgoGWire*)sRstAlgo)->m_nCenterMarginH = Algo2.m_nCenterMargin * 2 + 1;

	return bResult;
}

BOOL CPInsp_AlgoGWire::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	//★★ 코드 수정 필요
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoGWire::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}

bool CPInsp_AlgoGWire::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}