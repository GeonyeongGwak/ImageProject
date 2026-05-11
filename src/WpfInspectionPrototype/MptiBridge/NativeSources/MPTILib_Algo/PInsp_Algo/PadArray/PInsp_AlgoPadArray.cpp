#include "PInsp_AlgoPadArray.h"

CPInsp_AlgoPadArray::CPInsp_AlgoPadArray(void)
{
}


CPInsp_AlgoPadArray::~CPInsp_AlgoPadArray(void)
{
}

void CPInsp_AlgoPadArray::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoPadArray::GetInspAlgoData()
{
	return eSPCAlgoPadArray;
}

int CPInsp_AlgoPadArray::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeForeign] = e_NG;

	RstAlgoPadArray * rstAlgo = (RstAlgoPadArray *)vRstInspAlgo;

	return nCurrentNgType;
}


bool CPInsp_AlgoPadArray::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoPadArray::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();
	
	PIAL::_AlgoPadArray algoPadArray;
	PIAL::_RstAlgoPadArray rstAlgo;
	PIAL::TeachPadArray teachPadArray;
	InspWrapper->ConvertAlgo((AlgoPadArray*)sInspAlgo.m_ptrInspAlgoParam, algoPadArray);

	PIAL::_TotalInspExceptArea TieArea;

	CPInsp_Algo &PInspAlgo = g_pInspMng->GetInspAlgo();
	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspPartImage->nImgSizeX = sWndAlgoImg.m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sWndAlgoImg.m_nHeight3D;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, *stAlgoParam.m_sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}

	if (sWndAlgoImg.m_ucArr2D)
	{
		pImg_buf->inspWndImage->nImgSizeX = nImgWidth;
		pImg_buf->inspWndImage->nImgSizeY = nImgHeight;
		pImg_buf->inspWndImage->nStartX = stAlgoParam.m_nStartX;
		pImg_buf->inspWndImage->nStartY = stAlgoParam.m_nStartY;

		pImg_buf->m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArr2D, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	}
	if (sWndAlgoImg.m_fArr3D)
	{
		if (pImg_buf->inspWndImage->m_p3D)
			delete pImg_buf->inspWndImage->m_p3D;

		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D, nImgWidth, nImgHeight, true);
		pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
	}
	InspWrapper->MakeBuffWnd(sWndClipAlgo, *stAlgoParam.m_sWndInfo, pImg_buf);
	InspWrapper->MakeROIImg(&stAlgoParam.m_ptrAlgoColorOpt->m_sFovImg, pImg_buf->inspPartImage->m_ImageBuffer);

	PIAL::PI_Buff* puImgDst = nullptr;
	if (ucArrDstImg)
		puImgDst = new PIAL::PI_Buff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);

	POINTF ptWndCenter;
	ptWndCenter.x = 0.f;
	ptWndCenter.y = 0.f;

	if (stAlgoParam.m_bInspection == TRUE)
	{
		bResult = InspWrapper->m_PInspAlgo->InspPadArray(algoPadArray, *pImg_buf, &rstAlgo);
	}
	else 
	{
		bResult = InspWrapper->m_PInspAlgo->TeachPadArray(algoPadArray, *pImg_buf, &teachPadArray, &rstAlgo, ptWndCenter, puImgDst);

		rstAlgo.dRstWidth[0] = teachPadArray.m_width;
		rstAlgo.dRstLength[0] = teachPadArray.m_length;
		if (teachPadArray.m_numPad == algoPadArray.narrdata[PadArray_by_Column] * algoPadArray.narrdata[PadArray_by_Row])
		{
			rstAlgo.dRstDistanceX[0] = teachPadArray.m_distanceX;
			rstAlgo.dRstDistanceY[0] = teachPadArray.m_distanceY;
		}
		rstAlgo.dRstCenterPointX = teachPadArray.m_centerPointX;
		rstAlgo.dRstCenterPointY = teachPadArray.m_centerPointY;
		rstAlgo.m_nArrRectCnt = teachPadArray.m_numPad;
		for (int i = 0; i < teachPadArray.m_numPad; i++)
		{
			rstAlgo.m_rcArrRect[i] = teachPadArray.pPad[i].m_rcRect_I;
			/*rstAlgo.dRstWidth[i] = teachPadArray.pPad[i].m_width;
			rstAlgo.dRstLength[i] = teachPadArray.pPad[i].m_Length;
			rstAlgo.dRstDistanceX[i] = teachPadArray.pPad[i].m_distanceX;
			rstAlgo.dRstDistanceY[i] = teachPadArray.pPad[i].m_distanceY;*/
		}

		//for (int i = 0; i < rstAlgo.m_nArrRectCnt; ++i)
		//{
		//	rstAlgo.m_rcArrRect[i].left += stAlgoParam.m_dx;
		//	rstAlgo.m_rcArrRect[i].right += stAlgoParam.m_dx;
		//	rstAlgo.m_rcArrRect[i].top += stAlgoParam.m_dy;
		//	rstAlgo.m_rcArrRect[i].bottom += stAlgoParam.m_dy;
		//}
		if (ucArrDstImg)
		{
			for (int nX = 0; nX < nImgWidth; nX++)
			{
				for (int nY = 0; nY < nImgHeight; nY++)
				{
					int nIndex = (nY + stAlgoParam.m_nStartY) * stAlgoParam.m_sPartAlgoImg->m_nWidth + nX + stAlgoParam.m_nStartX;
					ucArrDstImg[nIndex] = puImgDst->m_pData[nY * nImgWidth + nX];
				}
			}
		}
	}
	InspWrapper->ConvertRstAlgo(rstAlgo, (RstAlgoPadArray*)sRstAlgo);


	//g_pInspMng->m_procMil->SaveImage(ucArrDstImg, nImgWidth, nImgHeight, 1, L"D:\\testimage\\data2.bmp");
	return bResult;
}


BOOL CPInsp_AlgoPadArray::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	PIAL::_AlgoPadArray algoPadArray;
	PIAL::_RstAlgoPadArray rstAlgo;
	InspWrapper->ConvertAlgo((AlgoPadArray*)InspAlgo.m_ptrInspAlgoParam, algoPadArray);
	//std::vector<PIAL::PInspData*> pDataSet;

	//if (InspDataSet) pDataSet = InspDataSet->GetInspDataSet();

	bResult = InspWrapper->m_PInspAlgo->InspPadArray(algoPadArray, *pImg_buf, &rstAlgo);

	if (rstAlgo.m_nArrRectCnt > 200)
		rstAlgo.m_nArrRectCnt = 200;

	for (int i = 0; i < rstAlgo.m_nArrRectCnt; ++i)
	{
		rstAlgo.m_rcArrRect[i].left += algoParam.m_dx;
		rstAlgo.m_rcArrRect[i].right += algoParam.m_dx;
		rstAlgo.m_rcArrRect[i].top += algoParam.m_dy;
		rstAlgo.m_rcArrRect[i].bottom += algoParam.m_dy;
	}

	rstAlgo.m_poDrawCenter.x += pImg_buf->inspWndImage->nStartX;
	rstAlgo.m_poDrawCenter.y += pImg_buf->inspWndImage->nStartY;
	rstAlgo.m_poDrawCenter.y = pImg_buf->inspPartImage->nImgSizeY - rstAlgo.m_poDrawCenter.y;

	InspWrapper->ConvertRstAlgo(rstAlgo, (RstAlgoPadArray*)sRstAlgo);

	return bResult;
}

BOOL CPInsp_AlgoPadArray::TeachAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	PIAL::_AlgoPadArray algoPadArray;
	PIAL::_RstAlgoPadArray rstAlgo;
	InspWrapper->ConvertAlgo((AlgoPadArray*)InspAlgo.m_ptrInspAlgoParam, algoPadArray);
	std::vector<PIAL::PInspData*> pDataSet;

	if (InspDataSet) pDataSet = InspDataSet->GetInspDataSet();

	bResult = InspWrapper->m_PInspAlgo->InspPadArray(algoPadArray, *pImg_buf, &rstAlgo);

	return bResult;
}

BOOL CPInsp_AlgoPadArray::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoPadArray::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoPadArray::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}