#include "PInsp_AlgoPadBW.h"


CPInsp_AlgoPadBW::CPInsp_AlgoPadBW(void)
{
}


CPInsp_AlgoPadBW::~CPInsp_AlgoPadBW(void)
{
}

void CPInsp_AlgoPadBW::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoPadBW::GetInspAlgoData()
{
	unsigned long long ret = -1;
	ret = eSPCAlgoPadBW;
	//shw Delete Xret = eSPCAlgoPadBW;
	return ret;
}

int CPInsp_AlgoPadBW::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = e_OK;
	RstAlgoPadBW* rstAlgo = (RstAlgoPadBW *)vRstInspAlgo;

	bool bOK = true;

	//형상검사
	if (rstAlgo->m_bOKShapeArea== FALSE || rstAlgo->m_bOKArea == FALSE || rstAlgo->m_bOKShapeShiftX == FALSE || rstAlgo->m_bOKShapeShiftY == FALSE || rstAlgo->m_bMaskLoadSuccess == FALSE)
		bOK = false;

	//이물검사
	if (rstAlgo->m_bOKWidth== FALSE || rstAlgo->m_bOKLength == FALSE || rstAlgo->m_bOKArea == FALSE || rstAlgo->Option3DMinMaxOK == FALSE || rstAlgo->Option3DRangeOK == FALSE)
		bOK = false;

	if (bOK == false)
	{
		eWholeNgTypeTemp[TypeForeign] = e_NG;
		nCurrentNgType = TypeForeign;
	}
	
	return nCurrentNgType;
}

bool CPInsp_AlgoPadBW::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoPadBW::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	return bResult;
}
BOOL CPInsp_AlgoPadBW::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* dstImg)
{
	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	AlgoPadBW* pAlgoPadBW = (AlgoPadBW*)InspAlgo.m_ptrInspAlgoParam;
	PIAL::_AlgoPadBW algoPadBW;
	PIAL::_RstAlgoPadBW rstAlgo;
	InspWrapper->ConvertAlgo(pAlgoPadBW, algoPadBW);

	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();

	int nPartID = pInspBoardInfo->nPartIDOrg;
	bool bUseAISeg = ext::irc::get()->_CtrlServer.First()->prod.bUseAI && pAlgoPadBW->bUseAI_Segmentation;
#if MultiProcessFunc 
	if (bUseAISeg)
	{
		if (!g_pInspMng->ProcessAIDataByPartID(nPartID, bUseAISeg, pImg_buf, algoParam.m_dWndW, algoParam.m_dWndH))
			return FALSE;
	}
#endif

	memset(&rstAlgo, 0, sizeof(PIAL::_RstAlgoPadBW));

	//일반각 고려
	if ((int)pImg_buf->inspPartImage->m_Angle % 90 != 0)pImg_buf->inspPartImage->m_Angle = 0;

	//Auto 검사
	cv::Mat Mask;

	bool bMaskLoad = false;
	int nLane = 0;	//FRONT
	PIAL::PInspData* PadBWWindowMask = nullptr;
	if (algoParam.m_nInspType == 0)	//auto mode
	{
		//PadBW에서 만든 마스크 사용
		bMaskLoad = FindPreAllocMask(pAlgoPadBW, Mask);
		if (!bMaskLoad)
		{
			CString sLog; 
			sLog.Format(_T("%s - NG"), pAlgoPadBW->strGBMaskName);
			ext::Log::add(sLog);
		}
			
	}
	if (InspDataSet != nullptr)
	{
		PadBWWindowMask = InspDataSet->GetInspData(algoParam.m_nWndID);
	}

	// 통합 검사/제외 영역에 넣어준다
	PIALTieArea.dAngle = pImg_buf->inspPartImage->m_Angle;
	double dAlignAngle = 0;
	if (vecAlignResult.size() > 0)
		dAlignAngle = vecAlignResult[0].theta;

	if (algoParam.m_pvInspRstPoly)
	{
		pImg_buf->bUseDSI = true;
		pImg_buf->nWindowID = g_pInspMng->GetWindowID(algoParam.m_nWndIndex);
		pImg_buf->nAlgoID = InspAlgo.m_nAlgoId;
	}

	bResult = InspWrapper->m_PInspAlgo->InspPadBW(algoPadBW, *pImg_buf, &rstAlgo, PIALTieArea, 0, Mask, bMaskLoad, nullptr, -1 , dAlignAngle,NULL, PadBWWindowMask);

	if (algoParam.m_pvInspRstPoly)
	{
		for (size_t i = 0; i < pImg_buf->vecDSI.size(); i++)
		{
			InspRstPolyAlgo poly;
			InspWrapper->ConvertAlgo(&pImg_buf->vecDSI[i], &poly);
			algoParam.m_pvInspRstPoly->push_back(poly);
		}
	}

	//결과 이물 데이터를 파트 픽셀 좌표계로 변환
	for (int i = 0; i < rstAlgo.m_nArrRectCnt; ++i)
	{
		rstAlgo.m_rcArrRect_I[i].left += algoParam.m_dx;
		rstAlgo.m_rcArrRect_I[i].top += algoParam.m_dy;
		rstAlgo.m_rcArrRect_I[i].right += algoParam.m_dx;
		rstAlgo.m_rcArrRect_I[i].bottom += algoParam.m_dy;
	}

	//결과 형상좌표를 파트 좌표계로 변환
	for (int i = 0; i < rstAlgo.m_nArrShapeRectCnt; ++i)
	{
		rstAlgo.m_rcArrShapeRect_I[i].left += algoParam.m_dx;
		rstAlgo.m_rcArrShapeRect_I[i].top += algoParam.m_dy;
		rstAlgo.m_rcArrShapeRect_I[i].right += algoParam.m_dx;
		rstAlgo.m_rcArrShapeRect_I[i].bottom += algoParam.m_dy;
	}

	InspWrapper->ConvertRstAlgo(rstAlgo, (RstAlgoPadBW*)sRstAlgo);

	return bResult;
}

BOOL CPInsp_AlgoPadBW::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoPadBW::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	AlgoPadBW* pInspAlgo = (AlgoPadBW*)sInspAlgo.m_ptrInspAlgoParam;

	//nData = pInspAlgo->sArrInspPad[MAX_CNT_PAD_COMPOSED_LIGHT].stBin.m_sAlgoColorBase.GetColorData();
	//nData |= COLOR_DATA_WND;

	for (int i = 0;i < pInspAlgo->m_nTotLightCnt;i++)
	{
		nData |= pInspAlgo->sArrInspPad[i].stBin.m_sAlgoColorBase.GetColorData();
	}
	if(nData!= COLOR_DATA_Non)
	nData |= COLOR_DATA_WND;

	return nData;
}
bool CPInsp_AlgoPadBW::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}

void CPInsp_AlgoPadBW::ModelClear(int nLane)
{
	std::map<CString, cv::Mat>::iterator it;
	for (it = m_vArrModelList[nLane].begin(); it != m_vArrModelList[nLane].end(); ++it)
		it->second.release();
	if (g_pMPTI->GetUseMultiProcess())
	{
		ext::irs::get()->ModelClear(ext::ModelBufferFlag::eExtModel_PadBW);
	}
	m_vArrModelList[nLane].clear();
}
bool CPInsp_AlgoPadBW::PreAllocModel(int nLane, CString sPath, cv::Mat& Mask)
{
	bool bAllocSuccess = false;
	cv::Mat RotMask;
	Mask.copyTo(RotMask);

#if _DEBUG
	cv::Mat testimg;
	Mask.convertTo(testimg, CV_8UC1);
	cv::threshold(testimg, testimg, 0, 255, cv::THRESH_BINARY);
#endif

	m_vArrModelList[nLane].insert({ sPath, RotMask });

	if (g_pMPTI->GetUseMultiProcess())
	{
		sPath.Replace(_T(".png"), _T("._PadBWMask"));
		ext::irs::get()->ExtMatImageAdd(&RotMask, sPath, ext::ModelBufferFlag::eExtModel_PadBW);
	}
	return true;
}
bool CPInsp_AlgoPadBW::FindPreAllocMask(AlgoPadBW* algo, cv::Mat& Mask)
{
	bool bRet = false;

	CString sPath;
	sPath.Format(_T("%s"), algo->strGBMaskName);

	//Default FRONT
	int nLane = 0;

	//Confirm Lane
	if (sPath.GetLength() > 0)
	{
		if (sPath.Find('REAR') > 0)
			nLane = 1;	//REAR
	}
	if (g_pMPTI->GetUseMultiProcess())
	{
		sPath.Replace(_T(".png"), _T("._PadBWMask"));
		// Multi는 따로 Load 하는구문이 없음 공유메모리에서 직접적로드
		ext::InspRoot_Server::get()->GetExtMatImage(&Mask, sPath, ext::ModelBufferFlag::eExtModel_PadBW);

		if (Mask.data != nullptr && Mask.rows > 0 && Mask.cols > 0)
			bRet = TRUE;
	}
	else if (m_vArrModelList[nLane].size() > 0)
	{
		auto item = m_vArrModelList[nLane].find(sPath);
		if (item != m_vArrModelList[nLane].end())
		{
			Mask = item->second;
			bRet = true;
		}
	}

	return bRet;
}
