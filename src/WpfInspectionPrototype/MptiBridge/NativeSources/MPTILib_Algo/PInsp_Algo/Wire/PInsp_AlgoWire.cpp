#include "PInsp_AlgoWire.h"


CPInsp_AlgoWire::CPInsp_AlgoWire(void)
{
}


CPInsp_AlgoWire::~CPInsp_AlgoWire(void)
{
}

void CPInsp_AlgoWire::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoWire::GetInspAlgoData()
{
	return eSPCAlgoWire;
}

int CPInsp_AlgoWire::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoWire * rstAlgo = (RstAlgoWire *)vRstInspAlgo;
	//for (int a = 0; a < rstAlgo->m_nWireCnt; a++)
	{
		if (rstAlgo->m_bArrNGType/*[a]*/ == 1)
		{
			eWholeNgTypeTemp[TypeMountMissing] = e_NG;
			nCurrentNgType = TypeMountMissing;
		}
		else if (rstAlgo->m_bArrNGType/*[a]*/ == 2)
		{
			eWholeNgTypeTemp[TypeDamage] = e_NG;
			nCurrentNgType = TypeDamage;
		}
		else if (!rstAlgo->m_bArrOK/*[a]*/[eWIRE_Thickness] || !rstAlgo->m_bArrOK/*[a]*/[eWIRE_Distance] || !rstAlgo->m_bArrOK/*[a]*/[eWIRE_Interval] ||
			!rstAlgo->m_bOK)
		{
			eWholeNgTypeTemp[TypeMountWorng] = e_NG;
			nCurrentNgType = TypeMountWorng;
		}
		else if (!rstAlgo->m_bArrOK/*[a]*/[eWIRE_Height] || !rstAlgo->m_bArrOK/*[a]*/[eWIRE_Height2] ||
			!rstAlgo->m_bArrOK/*[a]*/[eWIRE_Highest] || !rstAlgo->m_bArrOK/*[a]*/[eWIRE_SteepSlope])
		{
			eWholeNgTypeTemp[TypeMountLift] = e_NG;
			nCurrentNgType = TypeMountLift;
		}
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoWire::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoWire::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	CPInsp_WireBonding* PInspWireBonding = g_pInspMng->GetWireBondingInsp();
	if (PInspWireBonding == nullptr)
		return bResult;

	if (stAlgoParam.m_bInspection == TRUE)
	{
		InspPartParam *pParamArray = g_pInspMng->GetInspPartParam();
		InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();
		if (pParamArray == nullptr || pInspBoardInfo == nullptr)
			return bResult;

		AlgoWire *pAlgoWire = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;

		int nWireRefAreaOpt = pAlgoWire->m_nWireRefAreaOpt;

		int nFootRstIdx = 0;
		POINTF poArrFoot[WIRE_DRAW_CNT];
		memset(&poArrFoot, 0, sizeof(POINTF) * 2);

		float fWireRefAreaHgt = 0.0f;
		float fWireRefAreaHgt_Sub = 0.0f;
		//Foot 결과 설정
		for (int nAlgo = 0; nAlgo < pParamArray[stAlgoParam.m_nWndIndex].nAlgorithmCnt; nAlgo++)
		{
			if (nFootRstIdx > 1)
				break;

			if (stAlgoParam.m_WndResult->m_vArrRstInspAlgo[nAlgo].m_bIsInsp == FALSE)
				continue;
			if (pParamArray[stAlgoParam.m_nWndIndex].vArrAlgoParam[nAlgo].m_eAlgoType != eAlgoFoot)
				continue;
			RstAlgoFoot *sRstAlgo = (RstAlgoFoot *)stAlgoParam.m_WndResult->m_vArrRstInspAlgo[nAlgo].m_vRstInspAlgo;
			if (sRstAlgo == NULL) continue;

			//FootPad 바닥높이 값 wire 검사함수에 넘기기 
			InspAlgo sInspAlgo = pParamArray[stAlgoParam.m_nWndIndex].vArrAlgoParam[nAlgo];
			AlgoFoot *pInspAlgoFoot = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;

			if (nAlgo == 0 && pAlgoWire->m_nWireRefAreaOpt == (int)m_eWire_RefAreaType::Foot1_pad)	//foot 1
			{
				fWireRefAreaHgt = sRstAlgo->m_fAreaHeight;
				pInspAlgoFoot->m_fPadAreaAvgHeight = fWireRefAreaHgt;
				
			}
			else if (nAlgo == 1 && pAlgoWire->m_nWireRefAreaOpt == (int)m_eWire_RefAreaType::Foot2_pad)	//foot 2
			{
				fWireRefAreaHgt = sRstAlgo->m_fAreaHeight;
				pInspAlgoFoot->m_fPadAreaAvgHeight = fWireRefAreaHgt;

			}
			else if(nAlgo == 1) // Pad2를 쓰지않을때 Sub으로 넣어준다
			{
				fWireRefAreaHgt_Sub = sRstAlgo->m_fAreaHeight;
				//pInspAlgoFoot->m_fPadAreaAvgHeight = fWireRefAreaHgt;
			}

			if (sRstAlgo->m_sEnd.x == 0 && sRstAlgo->m_sEnd.y == 0)
				continue;
			if (stAlgoParam.m_dx >= sRstAlgo->m_sEnd.x || stAlgoParam.m_dx + stAlgoParam.m_dInspW <= sRstAlgo->m_sEnd.x ||
				stAlgoParam.m_dy >= sRstAlgo->m_sEnd.y || stAlgoParam.m_dy + stAlgoParam.m_dInspH <= sRstAlgo->m_sEnd.y)
				continue;

			

			if (pInspAlgoFoot->m_nFootType == 1) 
			{
				//ribbon
				poArrFoot[nFootRstIdx].x = sRstAlgo->m_sEnd.x - stAlgoParam.m_dx;
				poArrFoot[nFootRstIdx].y = sRstAlgo->m_sEnd.y - stAlgoParam.m_dy;
			}
			else
			{
				poArrFoot[nFootRstIdx].x = (sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Wedge_L].x + sRstAlgo->m_poWire_Wnd.x) - stAlgoParam.m_dx;
				poArrFoot[nFootRstIdx].y = (sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Wedge_L].y + sRstAlgo->m_poWire_Wnd.y) - stAlgoParam.m_dy;
			}

			nFootRstIdx++;
		}

		sWndAlgoImg.m_nWidth3D = pInspBoardInfo->partImgBuf.nImageSizeX;
		sWndAlgoImg.m_nHeight3D = pInspBoardInfo->partImgBuf.nImageSizeY;
		if(nFootRstIdx == 2) 
			bResult = PInspWireBonding->InspWire(sInspAlgo, sWndAlgoImg, (RstAlgoWire *)sRstAlgo, stTieArea, stAlgoParam.m_dx, stAlgoParam.m_dy, *stAlgoParam.m_ptrAlgoColorOpt, nFootRstIdx == 2 ? poArrFoot : nullptr, -1, 0, ucArrDstImg, stAlgoParam.m_sAlignRes, fWireRefAreaHgt, fWireRefAreaHgt_Sub);
		else
			if (sRstAlgo)
			{
				memset(sRstAlgo, 0, sizeof(RstAlgoWire));
				((RstAlgoWire *)sRstAlgo)->m_poWire_Wnd.x = stAlgoParam.m_dx;
				((RstAlgoWire *)sRstAlgo)->m_poWire_Wnd.y = stAlgoParam.m_dy;
				((RstAlgoWire *)sRstAlgo)->m_nWireCnt = 0;
				((RstAlgoWire *)sRstAlgo)->m_fRefArea_H = 0.0f;
				((RstAlgoWire *)sRstAlgo)->m_rcWire.left = 0;
				((RstAlgoWire *)sRstAlgo)->m_rcWire.right = 0;
				((RstAlgoWire *)sRstAlgo)->m_rcWire.top = 0;
				((RstAlgoWire *)sRstAlgo)->m_rcWire.bottom = 0;
				
				((RstAlgoWire *)sRstAlgo)->m_nArrCnt = 0;
				((RstAlgoWire *)sRstAlgo)->m_bArrNGType = 0;
				((RstAlgoWire *)sRstAlgo)->m_bArrHo = 0;
			}
			else
				return FALSE;
	}
	else
	{
		sWndAlgoImg.m_nWidth3D = stAlgoParam.m_sPartAlgoImg->m_nWidth;
		sWndAlgoImg.m_nHeight3D = stAlgoParam.m_sPartAlgoImg->m_nHeight;
		float fWireRefAreaHgt = 0.0f;
		bResult = PInspWireBonding->InspWire(sInspAlgo, sWndAlgoImg, (RstAlgoWire *)sRstAlgo, stTieArea, stAlgoParam.m_nStartX, stAlgoParam.m_nStartY, *stAlgoParam.m_ptrAlgoColorOpt, NULL, stAlgoParam.m_nSelectArea, *stAlgoParam.m_pfHeightMean, ucArrDstImg, NULL, fWireRefAreaHgt);
	}
	return bResult;
}

BOOL CPInsp_AlgoWire::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoWire::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoWire::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}