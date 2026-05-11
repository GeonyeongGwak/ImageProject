#include "PInsp_AlgoBodyEdge.h"

//★★ 코드 작성 필요
CPInsp_AlgoBodyEdge::CPInsp_AlgoBodyEdge(void)
{
}


CPInsp_AlgoBodyEdge::~CPInsp_AlgoBodyEdge(void)
{
}

void CPInsp_AlgoBodyEdge::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoBodyEdge::GetInspAlgoData()
{
	unsigned long long ret = -1;
	ret = eSPCAlgoBodyEdge;
	//shw Delete Xret = eSPCAlgoBodyEdge;
	return ret;
}

int CPInsp_AlgoBodyEdge::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	if (nWndType == eINSP_ALIGN)
	{
		eWholeNgTypeTemp[TypeAlignWrong] = e_NG;
		nCurrentNgType = TypeAlignWrong;
	}
	else
	{
		RstAlgoBodyEdge * rstAlgo = (RstAlgoBodyEdge *)vRstInspAlgo;

		if (!rstAlgo->m_bOKShiftX && !rstAlgo->m_bOKShiftY && !rstAlgo->m_bOKAngle && !rstAlgo->m_bOKWidth && !rstAlgo->m_bOKLength)
		{
			eWholeNgTypeTemp[TypeMountMissing] = e_NG;
			nCurrentNgType = TypeMountMissing;
		}

		if (!rstAlgo->m_bOKWidth || !rstAlgo->m_bOKLength)
		{
			eWholeNgTypeTemp[TypeMountWorng] = e_NG;
			nCurrentNgType = TypeMountWorng;
		}

		if (!rstAlgo->m_bOKAngle)
		{
			eWholeNgTypeTemp[TypeMountAngle] = e_NG;
			nCurrentNgType = TypeMountAngle;
		}

		if (!rstAlgo->m_bOKShiftX || !rstAlgo->m_bOKShiftY || !rstAlgo->m_bOKDistanceX || !rstAlgo->m_bOKDistanceY)
		{
			eWholeNgTypeTemp[TypeMountShift] = e_NG;
			nCurrentNgType = TypeMountShift;
		}
	}

	return nCurrentNgType;
}

bool CPInsp_AlgoBodyEdge::InspWindowArea(int nType)
{
	//★★ 코드 수정 필요
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoBodyEdge::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	if (sInspAlgo.m_eAlgoType != eAlgoBodyEdge)
		return bResult;

	RstAlgoBodyEdge * rst = (RstAlgoBodyEdge *)sRstAlgo;
	memset(rst, 0, (sizeof(RstAlgoBodyEdge)));

	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	PIAL::_RstAlgoBodyEdge RstAlgo2;
	PIAL::_AlgoBodyEdge Algo2;

	InspWrapper->ConvertAlgo((AlgoBodyEdge*)sInspAlgo.m_ptrInspAlgoParam, Algo2);

	PIAL::Insp_Image Img_buf;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	Img_buf.inspPartImage->nImgSizeX = nImgWidth;
	Img_buf.inspPartImage->nImgSizeY = nImgHeight;

	Img_buf.inspWndImage->nImgSizeX = nImgWidth;
	Img_buf.inspWndImage->nImgSizeY = nImgHeight;
	Img_buf.m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArr2D, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	Img_buf.inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D, nImgWidth, nImgHeight, true);
	Img_buf.inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D, nImgWidth, nImgHeight, true);

	Img_buf.inspPartImage->m_fPartRoundingErrX = sWndAlgoImg.m_fPartRoundingErrX;
	Img_buf.inspPartImage->m_fPartRoundingErrY = sWndAlgoImg.m_fPartRoundingErrY;

	PIAL::PI_Buff dst(ucArrDstImg, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);

	
	if (stAlgoParam.m_bTeach == TRUE)
	{
		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::TeachBodyEdge(Algo2, Img_buf, &RstAlgo2, &dst);
		InspWrapper->ConvertRstAlgo(RstAlgo2, (RstAlgoBodyEdge*)sRstAlgo);
	}
	else if (stAlgoParam.m_bInspection == TRUE)
	{
		std::vector<PIAL::_AlignResult> vecAlignResult;
		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::InspBodyEdge(Algo2, Img_buf, &RstAlgo2, vecAlignResult);
		InspWrapper->ConvertRstAlgo(RstAlgo2, (RstAlgoBodyEdge*)sRstAlgo);
	}
	else if(stAlgoParam.m_bBin == TRUE)
	{
		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::TeachBodyEdge(Algo2, Img_buf, &RstAlgo2, &dst);
		InspWrapper->ConvertRstAlgo(RstAlgo2, (RstAlgoBodyEdge*)sRstAlgo);
		//bResult = InspWrapper->m_PInspAlgo->PInspAlgo::BinalizeBodyEdge(Algo2, Img_buf, &dst);
	}

	rst->m_bOKDistanceX = rst->m_bOKDistanceY = TRUE;

	return bResult;
}
BOOL CPInsp_AlgoBodyEdge::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	BOOL bResult = FALSE;

	if (InspAlgo.m_eAlgoType != eAlgoBodyEdge)
		return bResult;

	RstAlgoBodyEdge * rst = (RstAlgoBodyEdge *)sRstAlgo;
	memset(rst, 0, (sizeof(RstAlgoBodyEdge)));

	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	PIAL::_RstAlgoBodyEdge RstAlgo2;
	PIAL::_AlgoBodyEdge Algo2;
	InspWrapper->ConvertAlgo((AlgoBodyEdge*)InspAlgo.m_ptrInspAlgoParam, Algo2);
#if RETEACHG
	int szX = pImg_buf->inspWndImage->m_p3D->Width();
	int szY = pImg_buf->inspWndImage->m_p3D->Length();
	float imgValue = ((255 << 16) | (255 << 8) | (255));
	//float* img32 = new float[szX * szY];
	float* img32 = g_pMManager->pem_new<float>(true, szX * szY, (PCHAR)__FUNCTION__, __LINE__);
	memset(img32, imgValue, sizeof(float)*szX * szY);

	HEADER_PTT hd;
	hd.uiNumRow = szX;
	hd.uiNumCol = szY;
	hd.fNumRow = (float)szX;
	hd.fNumCol = (float)szY;
	hd.pixelResX = (float)_mm2micron(g_pInspMng->GetResolX());
	hd.pixelResY = (float)_mm2micron(g_pInspMng->GetResolY());
	hd.zResolX = (float)_mm2micron(g_pInspMng->GetResolX());
	hd.zResolY = (float)_mm2micron(g_pInspMng->GetResolY());
	CString fullPath;
	fullPath.Format(_T("D:\\test\\bodyEdge.ptt"));
	alpf_save_ptt(fullPath, &hd, pImg_buf->inspWndImage->m_p3D->Mat().ptr<float>(), img32);
	g_pMManager->pem_delete(img32, true);
#endif
	//PIAL::PI_Buff dst(ucArrDstImg, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	if (algoParam.m_bTeach == TRUE)
	{
		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::TeachBodyEdge(Algo2, *pImg_buf, &RstAlgo2, nullptr);
		InspWrapper->ConvertRstAlgo(RstAlgo2, (RstAlgoBodyEdge*)sRstAlgo);
	}
	else if (algoParam.m_bInspection == TRUE)
	{
		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::InspBodyEdge(Algo2, *pImg_buf, &RstAlgo2, vecAlignResult, bodyInfo);
		InspWrapper->ConvertRstAlgo(RstAlgo2, (RstAlgoBodyEdge*)sRstAlgo);
	}
	else if (algoParam.m_bBin == TRUE)
	{
		bResult = InspWrapper->m_PInspAlgo->PInspAlgo::BinalizeBodyEdge(Algo2, *pImg_buf, nullptr);
	}

	rst->m_bOKDistanceX = rst->m_bOKDistanceY = TRUE;

	return bResult;
}
BOOL CPInsp_AlgoBodyEdge::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	//★★ 코드 수정 필요
	BOOL bRet = FALSE;

	if ((inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
	{
		AlgoBodyEdge *pAlgoBodyEdge = (AlgoBodyEdge *)sInspAlgo.m_ptrInspAlgoParam;
		pAlignRes->TeachCenterX = pAlgoBodyEdge->farrdata[BodyEdge_F_TechCenterX];
		pAlignRes->TeachCenterY = pAlgoBodyEdge->farrdata[BodyEdge_F_TechCenterY];

		RstAlgoBodyEdge * rst = (RstAlgoBodyEdge *)sRstAlgo;
		pAlignRes->offsetX = (double)-rst->m_dRstOffset_x;
		pAlignRes->offsetY = (double)rst->m_dRstOffset_y;
		pAlignRes->theta = (double)rst->m_dRstTheta;
		bRet = TRUE;
	}
	return bRet;
}
int CPInsp_AlgoBodyEdge::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoBodyEdge::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}