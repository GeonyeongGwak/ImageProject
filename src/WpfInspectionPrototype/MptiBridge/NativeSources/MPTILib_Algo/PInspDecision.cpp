#include "StdAfx.h"
#include "AlgoFactory.h"
#include "PInspDecision.h"
#include <math.h>
#include "ExtInspRoot.h"

#define  EXCEL_ENABLED  0


CPInspDecision::CPInspDecision(void)
{
	m_className = _T("CPInspDecision");

	m_pInspResult = NULL;
	m_pOkNg = NULL;
	for (int a = 0; a < eAlgoNum; a++)
		m_nSPCAlgoType[a] = GetInspAlgoData((InspAlgoType)a);
	for (int a = 0; a < kindofWholeNGTypeTotalNum; a++)
		m_nArrPriorityCode[a] = a;
	//InitialExcel();
}


CPInspDecision::~CPInspDecision(void)
{
	m_pInspResult = NULL;
}

void CPInspDecision::SetParamInfo(int* groupIndexCnts, int* groupWndCnts, int** groupIndex, int** groupWndID)
{
	m_groupIndexCnts = groupIndexCnts;
	m_groupWndCnts = groupWndCnts;
	m_groupIndex = groupIndex;
	m_groupWndID = groupWndID;
}
void CPInspDecision::SetParamInfo(InspPartParam *pParamArray)
{
	m_pParamArray = pParamArray;
}

void CPInspDecision::SetInspResult(InspectionResult* result, int** okng)
{
	m_pInspResult = result;
	m_pOkNg = okng;
}


// InspDecision() 에서 사용
// NG 우선 순위에 따라 전체 NG type을 돌며 선정된 첫번째 NG defect code 를 리턴한다..
int CPInspDecision::PriorityNGDefectCode(int* arrDef, int* nkindofWholeNGType)
{
	int nPriorityCode = -1;
	int nDefectCode = (int)dftCODE_OK;
	if (arrDef)
	{
		for (int i = 0; i < kindofWholeNGTypeTotalNum; i++)
		{
			int nCode = m_nArrPriorityCode[i];
			if (nCode < 0 || nCode >= kindofWholeNGTypeTotalNum)
				nCode = i;
			if (arrDef[nCode] == e_NG)
			{
				nPriorityCode = nCode;
				break;
			}
		}
	}
	int nDefectType = 0;
	*nkindofWholeNGType = nPriorityCode;
	return GetDefectCode(nPriorityCode, nDefectType);
}

// InspDecision() 에서 사용
// kindofWholeNGType 로 선언된 변수 전체를 nTarget으로 초기화 한다.
int CPInspDecision::InitkindofWholeNGType(int* Source, int nTarget)
{
	if (!Source)
		return FALSE;

	for (int i = 0; i < kindofWholeNGTypeTotalNum; i++)
	{
		Source[i] = nTarget;
	}

	return TRUE;
}

// InspDecision() 에서 사용
// kindofWholeNGType 로 선언된 변수 중 일부를 nTarget으로 초기화 한다.
int CPInspDecision::InitkindofWholeNGType(int* Source, int nTarget, NG_Type Specific)
{
	if (!Source)
		return FALSE;

	switch (Specific)
	{
	case eNgMount:
	{
		//mountMissing = mountWorng = mountAngle = mountPolarity = mountShift = mountTilt = mountLift = ocrWrong =		e_SKIP;

		Source[TypeMountMissing] = nTarget;
		Source[TypeMountWorng] = nTarget;
		Source[TypeMountAngle] = nTarget;
		Source[TypeMountPolarity] = nTarget;
		Source[TypeMountShift] = nTarget;
		Source[TypeMountTilt] = nTarget;
		Source[TypeMountLift] = nTarget;
		Source[TypeOcrWrong] = nTarget;
	}
	break;
	case eNgAlign:
	{
		Source[TypeAlignWrong] = nTarget;
	}
	break;
	case eNgOCR:
	{
		Source[TypeOcrWrong] = nTarget;
	}
	break;
	case eNgLead:
	{
		Source[TypeLeadBridge] = nTarget;
		Source[TypeLeadTip] = nTarget;
		Source[TypeLeadShift] = nTarget;
		Source[TypeLeadLift] = nTarget;
		Source[TypeLeadSolder] = nTarget;
	}
	break;
	case eNgSolder:
	{
		Source[TypeSolderFilet] = nTarget;
		Source[TypeSolderCone] = nTarget;
	}
	break;
	case eNgTab:
	{
		Source[TypeTab] = nTarget;
		Source[TypeTabSpace] = nTarget;
	}
	break;
	case eNGForeign:
	{
		// not yet
	}
	break;
	case eNgBGA:
	{
		// not yet
	}
	break;
	case eNGS_Ball:
	{
		Source[TypeS_Ball] = nTarget;
	}
	break;
	}

	return TRUE;
}

// InspDecision() 에서 사용
// NG defect code 에 따라 배열로 만든 변수의 특정 위치에 e_NG를 넣어준다.
int CPInspDecision::MatchDefcodetoWholeNgType(int nDefCode, int* ngType)
{
	switch (nDefCode)
	{
	case dftCODE_MISSING:
		ngType[TypeMountMissing] = e_NG;
		break;
	case dftCODE_S_BALL:
		ngType[TypeS_Ball] = e_NG;
		break;
	case dftCODE_WRONG:
		ngType[TypeMountWorng] = e_NG;
		break;
	case dftCODE_OCR:
		ngType[TypeOcrWrong] = e_NG;
		break;
	case dftCODE_POLARITY:
		ngType[TypeMountPolarity] = e_NG;
		break;
	case dftCODE_TOMBSTONE:
		ngType[TypeTombstone] = e_NG;
		break;
	case dftCODE_BILLBOARDING:
		ngType[TypeBillboarding] = e_NG;
		break;
	case dftCODE_UPSIDEDOWN:
		ngType[TypeUpsidedown] = e_NG;
		break;
	case dftCODE_LENGTH:
		ngType[TypeLength] = e_NG;
		break;
	case dftCODE_SHIFT:
		ngType[TypeMountShift] = e_NG;
		ngType[TypeAlignWrong] = e_NG;
		break;
	case dftCODE_ANGLE:
		ngType[TypeMountAngle] = e_NG;
		break;
	case dftCODE_TILT:
		ngType[TypeMountTilt] = e_NG;
		break;
	case dftCODE_L_BRIDGE:
		ngType[TypeLeadBridge] = e_NG;
		break;
	case dftCODE_LIFT:
		ngType[TypeMountLift] = e_NG;
		break;
	case dftCODE_L_LIFT:
		ngType[TypeLeadLift] = e_NG;
		break;
	case dftCODE_L_LEAD:
		ngType[TypeLeadTip] = e_NG;
		break;
	case dftCODE_L_SHIFT:
		ngType[TypeLeadShift] = e_NG;
		break;
	case dftCODE_S_FILLET:
		ngType[TypeLeadSolder] = e_NG;
		ngType[TypeSolderFilet] = e_NG;
		break;
	case dftCODE_S_NO:
		ngType[TypeNoSolder] = e_NG;
		break;
	case dftCODE_S_EXCESS:
		ngType[TypeSolderExcess] = e_NG;
		break;
	case defCODE_FOREIGN:
		ngType[TypeForeign] = e_NG;
		break;
	case dftCODE_FOREIGN_A:
		ngType[TypeForeign_A] = e_NG;
		break;
	case dftCODE_FOREIGN_B:
		ngType[TypeForeign_B] = e_NG;
		break;
	case dftCODE_FOREIGN_C:
		ngType[TypeForeign_C] = e_NG;
		break;
	case defCODE_TAB:
		ngType[TypeTab] = e_NG;
		break;
	case defCODE_TAB_SPACE:
		ngType[TypeTabSpace] = e_NG;
		break;
	case defCODE_COPPER:
		ngType[TypeCOPPER] = e_NG;
		break;
	case defCODE_WARPAGE:
		ngType[TypeWarpage] = e_NG;
		break;
	case dftCODE_COLD_JOINT:
		ngType[TypeColdJoint] = e_NG;
		break;
	case dftCODE_DAMAGE:
		ngType[TypeDamage] = e_NG;
		break;
	case dftCODE_S_CONE:
		ngType[TypeSolderCone] = e_NG;
		break;
	case dftCODE_PIN_SHIFT:
		ngType[TypePinShift] = e_NG;
		break;
	case dftCODE_VOID:
		ngType[TypeVoid] = e_NG;
		break;
	case dftCODE_THICKNESS:
		ngType[TypeThickness] = e_NG;
		break;
	case defCODE_USER_DEFINE:
		ngType[TypeUserDefine] = e_NG;
		break;
	case dftCODE_LOW_COATING:
		ngType[TypeLowCoating] = e_NG;
		break;
	case dftCODE_HIGH_COATING:
		ngType[TypeHighCoating] = e_NG;
		break;

	case dftCODE_CI_BUBBLE:			//NYJ 2018/07/18
		ngType[TypeBubble] = e_NG;
		break;
	case dftCODE_BODYWIDTH:
		ngType[TypeBODYWIDTH] = e_NG;
		break;
	case dftCODE_BODYLENGTH:
		ngType[TypeBODYLENGTH] = e_NG;
		break;
	case dftCODE_BODYAREA:
		ngType[TypeBODYAREA] = e_NG;
		break;
	case dftCODE_COLOR:
		ngType[TypeCOLOR] = e_NG;
		break;
	}

	return TRUE;
}

bool CPInspDecision::CheckDefectAlgoKind(int curNgType, int wndDefectType)
{
	bool bReturn = false;

	if (wndDefectType > curNgType)
	{
		wndDefectType = curNgType;
		bReturn = true;
	}

	return bReturn;
}

int CPInspDecision::InspDecision(InspectionResult* result, int** defectTemp, int &nUserSub, bool bRecheck)
{
	int ret = dftCODE_OK;

	SetInspResult(result, defectTemp);

	int rstTemp = dftCODE_OK;

	// ng type 저장 변수
	int *eWholeNgTypeTemp = g_pMManager->pem_new<int>(true, kindofWholeNGTypeTotalNum, (PCHAR)__FUNCTION__, __LINE__);	// 중간중간 초기화 해가며 현재 ng 를 저장 할 변수
	int *eWholeNgType = g_pMManager->pem_new<int>(true, kindofWholeNGTypeTotalNum, (PCHAR)__FUNCTION__, __LINE__);	// 최종적으로 window ng 를 결정할 때 사용할 변수
	nUserSub = -1;
	// 전체 변수 초기화 작업
	InitkindofWholeNGType(eWholeNgTypeTemp, e_NG);
	InitkindofWholeNGType(eWholeNgType, e_OK);

	// NG Defect Wnd
	int nDefectWndID = 0;
	unsigned long long nDefectAlgo = 0;
	unsigned long long nCurDefectAlgo = 0;
	int nNgAlgoId = 0;
	int nNgType = kindofWholeNGTypeTotalNum + 10;
	BOOL bTipFaultNG = TRUE;
	for (int wndType = 0; wndType < eINSP_WND_CNT; wndType++)
	{
		if (wndType == eINSP_USER) continue;
		int groupCnt = m_groupIndexCnts[wndType];
		int wndStartIndex = 0;
		int nWndNgType = kindofWholeNGTypeTotalNum + 10;
		int nCurrentNgType = -1;
		BOOL bIsInsp = FALSE;
		std::vector<int> eAlgoNgType((int)kindofWholeNGTypeTotalNum); //알고리즘 별 DefectCode 우선순위
		int wndCount = m_pInspResult->GetWndArraySize(wndType);
		InspWndResult* inspRst = m_pInspResult->GetWndResult(wndType);
		InspParamTemp* paramTemp = m_pInspResult->GetWndParam(wndType);
		int nTypeNG = m_pInspResult->GetWndArraySize(wndType, -1, true);

		if (inspRst == nullptr || paramTemp == nullptr)
			continue;

		int nRstTemp = e_NG; // 해당 NG type 별 GROUP 에 ng 유무
		for (int i = 0; i < groupCnt; i++)
		{
			nRstTemp = e_NG;

			InitkindofWholeNGType(eWholeNgTypeTemp, e_OK);
			bool bGroupOK = false;
			int nGroupCnt = m_groupWndCnts[m_groupIndex[wndType][i]];
			for (int j = wndStartIndex; j < wndStartIndex + nGroupCnt; j++)
			{
				nCurrentNgType = -1;
				nWndNgType = kindofWholeNGTypeTotalNum + 10;

				if (j >= wndCount)
					break;

				if (wndType == eINSP_SOLDER)
				{
					if (bRecheck == false && (m_pOkNg == NULL || m_pOkNg[wndType][j] == e_SKIP))
						continue;
				}

				if (inspRst[j].m_bIsInsp == FALSE)
					continue;

				if (inspRst[j].m_bOk == TRUE)
				{
					nRstTemp = e_OK;
					bGroupOK = true;
					if (nTypeNG >= 0 && nTypeNG < kindofWholeNGTypeTotalNum)
						eWholeNgTypeTemp[nTypeNG] = e_OK;
					continue;
				}

				if (inspRst[j].m_vArrRstInspAlgo == NULL)
					continue;
				bool bCoiningInspect = false;
				bIsInsp = TRUE;
				int nUserSubTmp = -1;
				nRstTemp = e_NG; // 제거해야할 부분 - Group window 기능을 다시 쓸때 반드시 지울것
				int nInspOK = 0; // 1 : OK -1 : ESS NG
				for (int n = 0; n < inspRst[j].m_nAlgorithmCnt; ++n)
				{
					if (inspRst[j].m_vArrRstInspAlgo[n].m_bIsInsp == FALSE)
						continue;

					if (inspRst[j].m_vArrRstInspAlgo[n].m_bOk == TRUE)
					{
						if (wndType == eINSP_BGA)
							inspRst[j].m_vArrRstInspAlgo[n].m_nDefectCode = dftCODE_OK;
						if (nInspOK != -1) nInspOK = 1;
					}
					else
					{
						if (inspRst[j].m_vArrRstInspAlgo[n].m_bIsRequired)
							nInspOK = -1;

						for (size_t a = 0; a < (int)kindofWholeNGTypeTotalNum; a++)
							eAlgoNgType[a] = (int)e_OK;  //알고리즘 Defect 우선순위 초기화

						//if (wndType == eINSP_MOUNT)
						{
							// 여기서 기존 real NG 를 사용자 지정 NG 가 난 것처럼 교체한다.	
							nNgAlgoId = n + 1;
						}
						nCurDefectAlgo = m_nSPCAlgoType[inspRst[j].m_vArrRstInspAlgo[n].m_nAlgoType];

						// 사용자 지정 defectcode 사용
						if (inspRst[j].m_vArrRstInspAlgo[n].m_bUsingManualDefectCode && inspRst[j].m_vArrRstInspAlgo[n].m_nManualDefectCode > 100)
						{
							defectCode eUsercode = (defectCode)inspRst[j].m_vArrRstInspAlgo[n].m_nManualDefectCode;
							int nSub = inspRst[j].m_vArrRstInspAlgo[n].m_nManualSubDefectCode;
							if (nSub < 0) nSub = 0;
							if (eUsercode == defCODE_USER_DEFINE)
							{
								if (nUserSub == -1 || nUserSub > nSub) nUserSub = nSub;
								if (nUserSubTmp == -1 || nUserSubTmp > nSub) nUserSubTmp = nSub;
							}
							MatchDefcodetoWholeNgType(eUsercode, eWholeNgTypeTemp);
							if (wndType == eINSP_LEADSOLDER)
								MatchDefcodetoWholeNgType(eUsercode, eWholeNgType);	// 이곳은 특이하게 최종 ng 를 바로 넣는다.
						}
						else
						{
							if (wndType == eINSP_OCR || wndType == eINSP_S_BALL || wndType == eINSP_PAD)
							{
								eWholeNgTypeTemp[nTypeNG] = e_NG;
								nCurrentNgType = nTypeNG;
							}
							else
								nCurrentNgType = AlgoJudgment(wndType, inspRst[j].m_vArrRstInspAlgo[n].m_nAlgoType, paramTemp[j].vArrAlgoParam[n], inspRst[j].m_vArrRstInspAlgo[n].m_vRstInspAlgo, eWholeNgTypeTemp, eWholeNgType, nCurDefectAlgo, &eAlgoNgType[0], bTipFaultNG, bCoiningInspect);

							if (wndType == eINSP_BGA)
							{
								switch (inspRst[j].m_vArrRstInspAlgo[n].m_nAlgoType)
								{
								case eAlgoBump:
								{
									int nAlgoCurrentNGType = 0;
									inspRst[j].m_vArrRstInspAlgo[n].m_nDefectCode = PriorityNGDefectCode(&eAlgoNgType[0], &nAlgoCurrentNGType);
								}
								break;
								case eAlgoNGBlob: //NGBlob차후에 구현하자.
								{
									RstAlgoNGBlob* rstAlgo = (RstAlgoNGBlob *)inspRst[j].m_vArrRstInspAlgo[n].m_vRstInspAlgo;
									InspAlgo& inspAlgo = (InspAlgo)paramTemp[j].vArrAlgoParam[n];
									AlgoNGBlob* pAlgoNGBlob = (AlgoNGBlob*)inspAlgo.m_ptrInspAlgoParam;
									eWholeNgTypeTemp[TypeForeign] = e_NG;
									switch (pAlgoNGBlob->narrdata[NGBlob_by_inspType])
									{
									case 0: // FM (Default)
										if (!rstAlgo->bIsOKArea || !rstAlgo->bIsOKExtraPer || !rstAlgo->bIsOKExtraWidth || !rstAlgo->bIsOKLength || !rstAlgo->bIsOKWidth)
											inspRst[j].m_vArrRstInspAlgo[n].m_nDefectCode = defCODE_FOREIGN;
										break;
									case 1: // ExtramBump
										if (!rstAlgo->bIsOKArea || !rstAlgo->bIsOKExtraPer || !rstAlgo->bIsOKExtraWidth || !rstAlgo->bIsOKLength || !rstAlgo->bIsOKWidth)
											inspRst[j].m_vArrRstInspAlgo[n].m_nDefectCode = dftCODE_L_SHIFT;
										break;
									}
								}
								break;
								}
							}
						}

						PriorityNGDefectCode(eWholeNgTypeTemp, &nCurrentNgType);

						if (CheckDefectWnd(nCurrentNgType, nNgType) == true)
						{
							int nGroupIdx = m_groupIndex[wndType][i];
							if (nGroupIdx >= 0)
							{
								nDefectWndID = nGroupIdx;
								nNgType = nCurrentNgType;
								nDefectAlgo = nCurDefectAlgo;

								int nWndIdx = m_groupWndID[nGroupIdx][j - wndStartIndex];
								if (nWndIdx >= 0)
								{
									if (bRecheck)
									{
										CString str = _T("");
										str.Format(_T("[Recheck] InspDecision wndType : %d GroupID : %d WndIDX : %d"), wndType, nGroupIdx, nWndIdx);
										if (g_pInspMng)
											ext::Log::add(str);
									}
									if (m_pParamArray)
										nDefectWndID = m_pParamArray[nWndIdx].wndIndex;
								}
							}
						}
						if (CheckDefectWnd(nCurrentNgType, nWndNgType) == true)
						{
							nWndNgType = nCurrentNgType;
							inspRst[j].m_nDefectAlgo = nCurDefectAlgo;
							inspRst[j].m_nDefectAlgoID = nNgAlgoId;
						}
					}

					if (inspRst[j].m_vArrRstInspAlgo[n].m_nAlgoType == eAlgoBGA)
					{
						InspAlgo& inspAlgo = (InspAlgo)paramTemp[j].vArrAlgoParam[n];
						AlgoBGA* pAlgoBGA = (AlgoBGA*)inspAlgo.m_ptrInspAlgoParam;
						//Bump/UnCoining 구분
						if (pAlgoBGA->m_nInspType == 1)
							bCoiningInspect = true;

						inspRst[j].m_vArrRstInspAlgo[n].m_nDefectCode = 0; // BGA는 수량에 포함 않됨
					}
				}

				inspRst[j].m_nDefectCode = (nInspOK == 1) ? (int)dftCODE_OK : PriorityNGDefectCode(eWholeNgTypeTemp, &nCurrentNgType);
				inspRst[j].m_nDefectCodeSub = (nInspOK == 1) ? -1 : nUserSubTmp;
				if (nInspOK == 1)
				{
					nRstTemp = e_OK;
					if (nTypeNG >= 0 && nTypeNG < kindofWholeNGTypeTotalNum)
						eWholeNgTypeTemp[nTypeNG] = e_OK;
					continue;
				}
			}
			wndStartIndex += nGroupCnt;

			bool bGroup = true;
			if (groupCnt > 0 && nGroupCnt > 1)
			{
				if (bGroupOK)
					bGroup = false;
			}
			if (bGroup && nRstTemp == e_NG && bIsInsp == TRUE)
			{
				for (int i = 0; i < kindofWholeNGTypeTotalNum; i++)
				{
					if (eWholeNgTypeTemp[i] == e_NG)
						eWholeNgType[i] = e_NG;
				}
			}
		}

		if (groupCnt == 0) //검사를 하지 않았을 경우 결과에 영향을 미치지 않게 하려고..
		{
			if (wndType == eINSP_MOUNT)
				InitkindofWholeNGType(eWholeNgTypeTemp, e_SKIP, eNgMount);
			else
			{
				if (nTypeNG >= 0 && nTypeNG < kindofWholeNGTypeTotalNum && eWholeNgType[nTypeNG] != e_NG)
					eWholeNgType[nTypeNG] = e_SKIP;
			}
		}
	}

	if (eWholeNgTypeTemp != NULL)
	{
		//delete[] eWholeNgTypeTemp;
		g_pMManager->pem_delete(eWholeNgTypeTemp, true);
		eWholeNgTypeTemp = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	result->m_nDefectWnd = nDefectWndID;
	result->m_nDefectAlgo = nDefectAlgo;
	// Part Defect Code 결정 (우선 순위에 따라..)
	// Missing, Wrong, OCR, Polarity, Tombstone, Upsidedown, Shift, Angle, Tilt, Bridge, Lift, Solder(Lead), Lift(Lead Tip), No solder, Solder
	// 아래 우선순위가 변경되면 enum kindofWholeNGType순서도 변경해주세요
	int nDefectType = 0;
	ret = GetPriorityCode(eWholeNgType, nDefectType, bTipFaultNG);
	if (nUserSub < 0) nUserSub = 0;
	m_pInspResult->nDefectType = nDefectType;
	if (eWholeNgType != NULL)
	{
		//delete[] eWholeNgType;
		g_pMManager->pem_delete(eWholeNgType, true);
		eWholeNgType = NULL;
	}
	return ret;
}

bool CPInspDecision::CheckDefectWnd(int nCurrentNg, int nBeforeNg)
{
	bool bReturn = false;
	if (nCurrentNg < 0 || nBeforeNg < 0 || nCurrentNg > kindofWholeNGTypeTotalNum)
		return bReturn;

	if (nCurrentNg < nBeforeNg)
		bReturn = true;
	else
		bReturn = false;

	return bReturn;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifdef HideOldBGA
int CPInspDecision::InspDecision_BGA(InspParamBGA_Part param, InspectionResult* result, int* defectTemp)
{
	int ret = dftCODE_OK;

	if (result->bgaWndCnt < 1)
		return ret;

	float Max = 0;
	float Min = 0;
	float T = 0;
	CalcCoplanarity(param, result, &Max, &Min, &T);

	result->bgaGroupRst->maxCalcHeight = Max;
	result->bgaGroupRst->minCalcHeight = Min;
	result->bgaGroupRst->tolerance = T;


	if (T > param.stdTolerance)
	{
		ret = dftCODE_BGA_NG;

		result->bgaGroupRst->ok = FALSE;
		result->bgaGroupRst->list.toleranceOk = FALSE;

	}
	else
	{
		result->bgaGroupRst->ok = TRUE;
		result->bgaGroupRst->list.toleranceOk = TRUE;
	}


	//BGAResultSave_EXCEL(_T("BGA"), 1, result);
	//BGAResultSave_CSV(result);

	return ret;
}

void CPInspDecision::CalcCoplanarity(InspParamBGA_Part param, InspectionResult* result, float* retMax, float* retMin, float* retT)
{
	int count = result->bgaWndCnt;

	float* cogX = NULL;
	//cogX = new float[count];
	cogX = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
	memset(cogX, 0, sizeof(float) * count);

	float* cogY = NULL;
	//cogY = new float[count];
	cogY = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
	memset(cogY, 0, sizeof(float) * count);

	float* h = NULL;
	//h = new float[count];
	h = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
	memset(h, 0, sizeof(float) * count);

	for (int i = 0; i < count; i++)
	{
		cogX[i] = (float)result->bgaWndRst[i].cogX_pixel;
		cogY[i] = (float)result->bgaWndRst[i].cogY_pixel;
		h[i] = (float)result->bgaWndRst[i].height_Rel;
		//h[i] = (float)result->bgaWndRst[i].height_Abs;
		//h[i] = (float)_mm2micron(result->bgaWndRst[i].height_Abs);
	}

	float coefA = 0;
	float coefB = 0;
	float coefC = 0;
	m_proc3D.LeastSquareMethod_XY(cogX, cogY, h, count, &coefA, &coefB, &coefC);

	//////////////////////////////////////////////////////////////////////////
	float tempH = 0;
	float* calcH = NULL;
	//calcH = new float[count];
	calcH = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
	memset(calcH, 0, sizeof(float) * count);

	//old   (ax+by+c = z)
	/*for(int i = 0; i < count; i++)
	{

		tempH = (float)((coefA * cogX[i]) + (coefB * cogY[i]) + coefC);
		calcH[i] = h[i] - tempH;

		result->bgaWndRst[i].coplanarity = calcH[i];//////////////////
	}*/

	//new
	for (int i = 0; i < count; i++)
	{
		tempH = (float)((coefA * cogX[i]) + (coefB * cogY[i]) + (-1 * h[i]) + coefC);
		calcH[i] = tempH / sqrt((coefA * coefA) + (coefB * coefB) + (-1 * -1));

		result->bgaWndRst[i].coplanarity = calcH[i];//////////////////
	}

	float calcHmax = calcH[0];
	float calcHmin = calcH[0];
	for (int i = 0; i < count; i++)
	{
		if (calcHmax < calcH[i])
			calcHmax = calcH[i];

		if (calcHmin > calcH[i])
			calcHmin = calcH[i];
	}

	float calcTolerence = calcHmax - calcHmin;

	//////////////////////////////////////////////////////////////////////////
	*retMax = calcHmax;
	*retMin = calcHmin;
	*retT = calcTolerence;

	/*delete cogX;
	delete cogY;
	delete h;
	delete calcH;*/
	g_pMManager->pem_delete(cogX, false);
	g_pMManager->pem_delete(cogY, false);
	g_pMManager->pem_delete(h, false);
	g_pMManager->pem_delete(calcH, false);
}


void CPInspDecision::BGAResultSave_CSV(InspectionResult* result)
{
	CTime ct;
	CString strTime;
	ct = CTime::GetCurrentTime();
	strTime.Format(L"%04d%02d%02d_%02d%02d%02d", ct.GetYear(), ct.GetMonth(), ct.GetDay(), ct.GetHour(), ct.GetMinute(), ct.GetSecond());
	//////////////////////////////////////////////////////////////////////////
	FILE	*fp;
	wchar_t	path[256];
	CString fileName = _T("BGA_");

	wsprintf(path, _T("d:\\testimage\\BGAdata\\%s"), fileName + strTime + _T(".csv"));


	_wfopen_s(&fp, path, _T("at"));
	//fp = _wfopen(fileName, _T("at"));
	if (!fp)
	{
		AfxMessageBox(_T("already opened..."));
		return;
	}


	int count = result->bgaWndCnt;

	fwprintf(fp, _T("CalcH_Max,CalcH_Min,T\n"));
	fwprintf(fp, _T("%.6f,%.6f,%.6f\n\n\n"), result->bgaGroupRst[0].maxCalcHeight, result->bgaGroupRst[0].minCalcHeight, result->bgaGroupRst[0].tolerance);

	fwprintf(fp, _T("Index,Height_Abs,Height_Rel,CogX,CogY,Diameter,Coplanarity\n"));
	for (int i = 0; i < count; i++)
	{
		fwprintf(fp, _T("%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n"), i, result->bgaWndRst[i].height_Abs, result->bgaWndRst[i].height_Rel, result->bgaWndRst[i].cogX
			, result->bgaWndRst[i].cogY, result->bgaWndRst[i].dia, result->bgaWndRst[i].coplanarity);
	}


	fclose(fp);
}

void CPInspDecision::InitResultStruct(RstInspBGA_Grp* rst)
{
	//RstInspBGA_Grp* temp = new RstInspBGA_Grp;
	RstInspBGA_Grp* temp = g_pMManager->pem_new<RstInspBGA_Grp>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, sizeof(RstInspBGA_Grp));

	temp->ok = -1;

	temp->list.toleranceOk = TRUE;


	*rst = *temp;
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

void CPInspDecision::GetInspRst_BGA(RstInspBGA_Grp* dst)
{
	*dst = m_pInspRstBGA;
}
#endif
unsigned long long CPInspDecision::GetInspAlgoData(InspAlgoType eAlgo)
{
	long long nIndex = -1;
	CAlgoFactory algoFac;
	IPINSP_ALGO *iInslAlgo = algoFac.CreatePInspAlgo(eAlgo);
	if (iInslAlgo)
	{
		nIndex = iInslAlgo->GetInspAlgoData();
		g_pMManager->pem_delete(iInslAlgo, false);
		iInslAlgo = NULL;
	}
	return nIndex;
}
void CPInspDecision::SetPriorityCode(int* nArrCode)
{
	bool bInput = true;
	for (int a = 0; a < kindofWholeNGTypeTotalNum; a++)
	{
		m_nArrPriorityCode[a] = a;
		bool bIn = false;
		for (int b = 0; b < kindofWholeNGTypeTotalNum; b++)
		{
			if (nArrCode[b] == a)
			{
				bIn = true;
				break;
			}
		}
		if (bIn == false) bInput = false;
	}
	if (bInput == true)
	{
		for (int a = 0; a < kindofWholeNGTypeTotalNum; a++)
			m_nArrPriorityCode[a] = nArrCode[a];
	}
}
int CPInspDecision::GetPriorityCode(int* nType, int &nDefectType, BOOL bTipFaultNG)
{
	for (int a = 0; a < kindofWholeNGTypeTotalNum; a++)
	{
		int nCode = m_nArrPriorityCode[a];
		if (nCode < 0 || nCode >= kindofWholeNGTypeTotalNum)
			nCode = a;
		if (nType[nCode] == e_NG)
		{
			int nRet = GetDefectCode(nCode, nDefectType, bTipFaultNG);
			if (nRet == -1)
				continue;

			return nRet;
		}
	}
	return dftCODE_OK;
}
int CPInspDecision::GetDefectCode(int nCode, int &nDefectType, BOOL bTipFaultNG)
{
	if (nCode == TypeMountMissing)
	{
		nDefectType = eNgMount;
		return dftCODE_MISSING;
	}
	else if (nCode == TypeS_Ball)
	{
		nDefectType = eNGS_Ball;
		return dftCODE_S_BALL;
	}
	else if (nCode == TypeBillboarding)
	{
		nDefectType = eNgMount;
		return dftCODE_BILLBOARDING;
	}
	else if (nCode == TypeTombstone)
	{
		nDefectType = eNgMount;
		return dftCODE_TOMBSTONE;
	}
	else if (nCode == TypeMountWorng)
	{
		nDefectType = eNgMount;
		return dftCODE_WRONG;
	}
	else if (nCode == TypeOcrWrong)
	{
		nDefectType = eNgOCR;
		return dftCODE_OCR;
	}
	else if (nCode == TypeMountPolarity)
	{
		nDefectType = eNgMount;
		return dftCODE_POLARITY;
	}
	else if (nCode == TypeUpsidedown)
	{
		nDefectType = eNgMount;
		return dftCODE_UPSIDEDOWN;
	}
	else if (nCode == TypeLength)
	{
		nDefectType = eNgMount;
		return dftCODE_LENGTH;
	}
	else if (nCode == TypeMountShift)
	{
		nDefectType = eNgMount;
		return dftCODE_SHIFT;
	}
	else if (nCode == TypeAlignWrong)
	{
		nDefectType = eNgAlign;
		return dftCODE_SHIFT;
	}
	else if (nCode == TypeMountAngle)
	{
		nDefectType = eNgMount;
		return dftCODE_ANGLE;
	}
	else if (nCode == TypeMountTilt)
	{
		nDefectType = eNgMount;
		return dftCODE_TILT;
	}
	else if (nCode == TypeLeadBridge)
	{
		nDefectType = eNgLead;
		return dftCODE_L_BRIDGE;
	}
	else if (nCode == TypeMountLift)
	{
		nDefectType = eNgLead;
		return dftCODE_LIFT;
	}
	else if (nCode == TypeLeadSolder)
	{
		nDefectType = eNgLead;
		return dftCODE_S_FILLET;
	}
	else if (nCode == TypeNoSolder)
	{
		nDefectType = eNgSolder;
		return dftCODE_S_NO;
	}
	else if (nCode == TypeSolderFilet)
	{
		nDefectType = eNgSolder;
		return dftCODE_S_FILLET;
	}
	else if (nCode == TypeSolderExcess)
	{
		nDefectType = eNgSolder;
		return dftCODE_S_EXCESS;
	}
	else if (nCode == TypeLeadLift)
	{
		nDefectType = eNgLead;
		return dftCODE_L_LIFT;
	}
	else if (nCode == TypeLeadTip)
	{
		if (bTipFaultNG == TRUE)
		{
			nDefectType = eNgLead;
			return dftCODE_L_LEAD;
		}
		return -1;
	}
	else if (nCode == TypeLeadShift)
	{
		if (bTipFaultNG == TRUE)
		{
			nDefectType = eNgLead;
			return dftCODE_L_SHIFT;
		}
		return -1;
	}
	else if (nCode == TypeForeign)
	{
		nDefectType = eNgMount;
		return defCODE_FOREIGN;
	}
	else if (nCode == TypeForeign_A)
	{
		nDefectType = eNgMount;
		return dftCODE_FOREIGN_A;
	}
	else if (nCode == TypeForeign_B)
	{
		nDefectType = eNgMount;
		return dftCODE_FOREIGN_B;
	}
	else if (nCode == TypeForeign_C)
	{
		nDefectType = eNgMount;
		return dftCODE_FOREIGN_C;
	}
	else if (nCode == TypeWarpage)
	{
		nDefectType = eNgMount;
		return defCODE_WARPAGE;
	}
	else if (nCode == TypeBubble)
	{
		nDefectType = eNgMount;
		return dftCODE_CI_BUBBLE;
	}
	else if (nCode == TypeDamage)
	{
		nDefectType = eNgMount;
		return dftCODE_DAMAGE;
	}
	else if (nCode == TypeTab)
	{
		nDefectType = eNgTab;
		return defCODE_TAB;
	}
	else if (nCode == TypeTabSpace)
	{
		nDefectType = eNgTab;
		return defCODE_TAB_SPACE;
	}
	else if (nCode == TypeCOPPER)
	{
		nDefectType = eNgMount;
		return defCODE_COPPER;
	}
	else if (nCode == TypeColdJoint)
	{
		nDefectType = eNgLead;
		return dftCODE_COLD_JOINT;
	}
	else if (nCode == TypeSolderCone)
	{
		nDefectType = eNgSolder;
		return dftCODE_S_CONE;
	}
	else if (nCode == TypePinShift)
	{
		nDefectType = eNgSolder;
		return dftCODE_PIN_SHIFT;
	}
	else if (nCode == TypeVoid)
	{
		nDefectType = eNgSolder;
		return dftCODE_VOID;
	}
	else if (nCode == TypeThickness)
	{
		nDefectType = eNgMount;
		return dftCODE_THICKNESS;
	}
	else if (nCode == TypeUserDefine)
	{
		nDefectType = eNgMount;
		return defCODE_USER_DEFINE;
	}
	else if (nCode == TypeLowCoating)
	{
		nDefectType = eNgMount;
		return dftCODE_LOW_COATING;
	}
	else if (nCode == TypeHighCoating)
	{
		nDefectType = eNgMount;
		return dftCODE_HIGH_COATING;
	}
	else if (nCode == TypeBODYWIDTH)
	{
		nDefectType = eNgMount;
		return dftCODE_BODYWIDTH;
	}
	else if (nCode == TypeBODYLENGTH)
	{
		nDefectType = eNgMount;
		return dftCODE_BODYLENGTH;
	}
	else if (nCode == TypeBODYAREA)
	{
		nDefectType = eNgMount;
		return dftCODE_BODYAREA;
	}
	else if (nCode == TypeCOLOR)
	{
		nDefectType = eNgMount;
		return dftCODE_COLOR;
	}
	else
		return dftCODE_OK;
}
int CPInspDecision::GetPriorityCode(int nCodeOrg, int nCode)
{
	int nArr[kindofWholeNGTypeTotalNum];
	InitkindofWholeNGType(nArr, e_OK);
	MatchDefcodetoWholeNgType(nCodeOrg, nArr);
	MatchDefcodetoWholeNgType(nCode, nArr);
	int nDefectType = 0;
	return GetPriorityCode(nArr, nDefectType, FALSE);
}
int CPInspDecision::AlgoJudgment(int nWndType, InspAlgoType eAlgoType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;
	nCurDefectAlgo = GetInspAlgoData(eAlgoType);
	CAlgoFactory algoFac;
	IPINSP_ALGO *iInslAlgo = algoFac.CreatePInspAlgo(eAlgoType);
	if (iInslAlgo)
	{
		nCurrentNgType = iInslAlgo->AlgoJudgment(nWndType, vArrAlgoParam, vRstInspAlgo, eWholeNgTypeTemp, eWholeNgType, nCurDefectAlgo, eAlgoNgType, bTipFaultNG, bCoiningInspect);
		delete iInslAlgo; //g_pMManager->pem_delete(iInslAlgo, false);
		iInslAlgo = NULL;
	}

	switch (eAlgoType)
	{
	}
	return nCurrentNgType;
}