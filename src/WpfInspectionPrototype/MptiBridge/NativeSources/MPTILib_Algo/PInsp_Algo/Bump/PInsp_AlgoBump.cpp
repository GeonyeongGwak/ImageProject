#include "PInsp_AlgoBump.h"

CPInsp_AlgoBump::CPInsp_AlgoBump(void)
{
}


CPInsp_AlgoBump::~CPInsp_AlgoBump(void)
{
}

void CPInsp_AlgoBump::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
}

unsigned long long CPInsp_AlgoBump::GetInspAlgoData()
{
	return eSPCAlgoBump;
}

int CPInsp_AlgoBump::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	RstAlgoBump* rstAlgo = (RstAlgoBump *)vRstInspAlgo;

	//Coining 검사
	if (bCoiningInspect)
	{
		if (!rstAlgo->bIsOKArea)
		{
			if (rstAlgo->fRstArea == 0)
			{
				//Missing bump
				eWholeNgTypeTemp[TypeMountMissing] = e_NG;
				nCurrentNgType = TypeMountMissing;
				eAlgoNgType[TypeMountMissing] = e_NG;
			}
			else
			{
				eWholeNgTypeTemp[TypeDamage] = e_NG;  //Coining 영역 검사
				nCurrentNgType = TypeDamage;
				eAlgoNgType[TypeDamage] = e_NG;
			}

		} // Missing, Coining

		if (!rstAlgo->m_bOKShiftX || !rstAlgo->m_bOKShiftY || !rstAlgo->m_bOKShiftR)
		{
			//Mis-alignment (ex 70% match, adjustable value) 
			eWholeNgTypeTemp[TypeMountShift] = e_NG;
			nCurrentNgType = TypeMountShift;
			eAlgoNgType[TypeMountShift] = e_NG;
		} // Shift

		if (!rstAlgo->bIsOKUnCoining)
		{
			InspAlgo& inspAlgo = vArrAlgoParam;
			AlgoBlob* pAlgoBlob = (AlgoBlob*)inspAlgo.m_ptrInspAlgoParam;

			//if (pAlgoBlob->m_bUseBlobNG == FALSE  && pAlgoBlob->m_bUseBlobSizeDistance)
			//{
				//UnCoining
			eWholeNgTypeTemp[TypeMountLift] = e_NG;
			nCurrentNgType = TypeMountLift;
			eAlgoNgType[TypeMountLift] = e_NG;
			//}
			//else
			//{
			//	//Extra bump 
			//	eWholeNgTypeTemp[TypeForeign] = e_NG;
			//	nCurrentNgType = TypeForeign;
			//	eAlgoNgType[TypeForeign] = e_NG;
			//} 현재 Bump 는  Extra Bump 사용 안함.
		} // Uncoining

		if (!rstAlgo->bIsOKCircleRate)
		{
			//Circle Rate(Damage) 
			eWholeNgTypeTemp[TypeForeign] = e_NG;
			nCurrentNgType = TypeForeign;
			eAlgoNgType[TypeForeign] = e_NG;
		} // CircleRate

		if (!rstAlgo->bIsOKNGBumpW || !rstAlgo->bIsOKNGBumpL)
		{
			//Circle Rate(Damage) 
			eWholeNgTypeTemp[TypeDamage] = e_NG;
			nCurrentNgType = TypeDamage;
			eAlgoNgType[TypeDamage] = e_NG;
		} // NGBump

		if (!rstAlgo->bIsOKBridge)
		{
			//Bridge bump 
			eWholeNgTypeTemp[TypeLeadBridge] = e_NG;
			nCurrentNgType = TypeLeadBridge;
			eAlgoNgType[TypeLeadBridge] = e_NG;

		} // Birdge

		if (!rstAlgo->bIsOKWidth || !rstAlgo->bIsOKLength || !rstAlgo->bIsOKHeight) // 이건 장축,단축,height 로 분리 되어야하나 ?
		{
			InspAlgo& inspAlgo = vArrAlgoParam;
			AlgoBump* pAlgoBump = (AlgoBump*)inspAlgo.m_ptrInspAlgoParam;

			if (!rstAlgo->bIsOKWidth)
			{
				//if (rstAlgo->fRstWidth </* pAlgoBump->farrdata[Bump_F_TeachWidth] **/ pAlgoBump->farrdata[Bump_F_WidthRateMin]/*/ 100.*/) //UnderSize
				if (rstAlgo->fRstWidth < pAlgoBump->farrdata[Bump_F_TeachWidth] * pAlgoBump->farrdata[Bump_F_WidthRateMin] / 100.) //UnderSize
				{
					eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
					nCurrentNgType = TypeSolderFilet;
					eAlgoNgType[TypeSolderFilet] = e_NG;
				}
				else //OverSize
				{
					eWholeNgTypeTemp[TypeSolderExcess] = e_NG;
					nCurrentNgType = TypeSolderExcess;
					eAlgoNgType[TypeSolderExcess] = e_NG;
				}
			}

			if (!rstAlgo->bIsOKLength)
			{
				if (rstAlgo->fRstLength < /*pAlgoBump->farrdata[Bump_F_TeachLength] **/ pAlgoBump->farrdata[Bump_F_LengthRateMin]/* / 100.*/) //UnderSize
				{
					eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
					nCurrentNgType = TypeSolderFilet;
					eAlgoNgType[TypeSolderFilet] = e_NG;
				}
				else //OverSize
				{
					eWholeNgTypeTemp[TypeSolderExcess] = e_NG;
					nCurrentNgType = TypeSolderExcess;
					eAlgoNgType[TypeSolderExcess] = e_NG;
				}

			}

			if (!rstAlgo->bIsOKHeight)
			{

				if (rstAlgo->fRstHeight < pAlgoBump->farrdata[Bump_F_HeightMin]) //UnderSize
				{
					eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
					nCurrentNgType = TypeSolderFilet;
					eAlgoNgType[TypeSolderFilet] = e_NG;
				}
				else //OverSize
				{
					eWholeNgTypeTemp[TypeMountLift] = e_NG;
					nCurrentNgType = TypeMountLift;
					eAlgoNgType[TypeMountLift] = e_NG;
				}

			}

		}

		if (!rstAlgo->bIsOKVolume)
		{
			InspAlgo& inspAlgo = vArrAlgoParam;
			AlgoBump* pAlgoBump = (AlgoBump*)inspAlgo.m_ptrInspAlgoParam;

			double minRange = pAlgoBump->farrdata[Bump_F_TeachVolume] * pAlgoBump->farrdata[Bump_F_VolumeRateMin] / 100.;
			double maxRange = pAlgoBump->farrdata[Bump_F_TeachVolume] * pAlgoBump->farrdata[Bump_F_VolumeRateMax] / 100.;
			if (rstAlgo->fRstVolume > maxRange)
			{
				eWholeNgTypeTemp[TypeSolderExcess] = e_NG;  //TypeOverSize
				nCurrentNgType = TypeSolderExcess;
				eAlgoNgType[TypeSolderExcess] = e_NG;
			}

			if (rstAlgo->fRstVolume < minRange)
			{
				eWholeNgTypeTemp[TypeMountMissing] = e_NG;  //TypeUnderSize
				nCurrentNgType = TypeMountMissing;
				eAlgoNgType[TypeMountMissing] = e_NG;
			}
		}


	}
	else // Bump 검사
	{
		if (!rstAlgo->bIsOKArea)
		{
			if (rstAlgo->fRstArea == 0)
			{
				//Missing bump
				eWholeNgTypeTemp[TypeMountMissing] = e_NG;
				nCurrentNgType = TypeMountMissing;
				eAlgoNgType[TypeMountMissing] = e_NG;
			}
			else
			{
				InspAlgo& inspAlgo = vArrAlgoParam;
				AlgoBump* pAlgoBump = (AlgoBump*)inspAlgo.m_ptrInspAlgoParam;

				double minRange = pAlgoBump->farrdata[Bump_F_Area] * pAlgoBump->farrdata[Bump_F_AreaMin] / 100.;
				double maxRange = pAlgoBump->farrdata[Bump_F_Area] * pAlgoBump->farrdata[Bump_F_AreaMax] / 100.;
				if (rstAlgo->fRstArea >= minRange)
				{
					eWholeNgTypeTemp[TypeSolderExcess] = e_NG;  //TypeOverSize
					nCurrentNgType = TypeSolderExcess;
					eAlgoNgType[TypeSolderExcess] = e_NG;
				}
				else
				{
					eWholeNgTypeTemp[TypeSolderFilet] = e_NG;  //TypeUnderSize
					nCurrentNgType = TypeSolderFilet;
					eAlgoNgType[TypeSolderFilet] = e_NG;
				}

			}
		}

		if (!rstAlgo->m_bOKShiftX || !rstAlgo->m_bOKShiftY || !rstAlgo->m_bOKShiftR || !rstAlgo->bIsOKPitch)
		{
			//Mis-alignment (ex 70% match, adjustable value) 
			eWholeNgTypeTemp[TypeMountShift] = e_NG;
			nCurrentNgType = TypeMountShift;
			eAlgoNgType[TypeMountShift] = e_NG;
		}

		//if (!rstAlgo->m_bOKArea_Reverse)
		//{
		//	//Extra bump 
		//	eWholeNgTypeTemp[TypeForeign] = e_NG;
		//	nCurrentNgType = TypeForeign;
		//	eAlgoNgType[TypeForeign] = e_NG;
		//} NGBUmp 이젠 무조건 Blob임.

		if (!rstAlgo->bIsOKCircleRate)
		{
			//Circle Rate(Damage) 
			eWholeNgTypeTemp[TypeDamage] = e_NG;
			nCurrentNgType = TypeDamage;
			eAlgoNgType[TypeDamage] = e_NG;
		}

		if (!rstAlgo->bIsOKNGBumpW || !rstAlgo->bIsOKNGBumpL)
		{
			//Circle Rate(Damage) 
			eWholeNgTypeTemp[TypeDamage] = e_NG;
			nCurrentNgType = TypeDamage;
			eAlgoNgType[TypeDamage] = e_NG;
		} // NGBump

		if (!rstAlgo->bIsOKBridge)
		{
			//Bridge bump 
			eWholeNgTypeTemp[TypeLeadBridge] = e_NG;
			nCurrentNgType = TypeLeadBridge;
			eAlgoNgType[TypeLeadBridge] = e_NG;
		}

		if (!rstAlgo->bIsOKContrast)
		{
			//Bridge bump 
			eWholeNgTypeTemp[TypeLowCoating] = e_NG;
			nCurrentNgType = TypeLowCoating;
			eAlgoNgType[TypeLowCoating] = e_NG;
		}

		if (!rstAlgo->bIsOKWidth || !rstAlgo->bIsOKLength || !rstAlgo->bIsOKHeight)
		{
			InspAlgo& inspAlgo = vArrAlgoParam;
			AlgoBump* pAlgoBump = (AlgoBump*)inspAlgo.m_ptrInspAlgoParam;

			if (!rstAlgo->bIsOKWidth)
			{
				if (rstAlgo->fRstWidth < pAlgoBump->farrdata[Bump_F_TeachWidth] * pAlgoBump->farrdata[Bump_F_WidthRateMin] / 100.) //UnderSize
				{
					eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
					nCurrentNgType = TypeSolderFilet;
					eAlgoNgType[TypeSolderFilet] = e_NG;
				}
				else //OverSize
				{
					eWholeNgTypeTemp[TypeSolderExcess] = e_NG;
					nCurrentNgType = TypeSolderExcess;
					eAlgoNgType[TypeSolderExcess] = e_NG;
				}
			}

			if (!rstAlgo->bIsOKLength)
			{
				if (rstAlgo->fRstLength < pAlgoBump->farrdata[Bump_F_TeachLength] * pAlgoBump->farrdata[Bump_F_LengthRateMin] / 100.) //UnderSize
				{
					eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
					nCurrentNgType = TypeSolderFilet;
					eAlgoNgType[TypeSolderFilet] = e_NG;
				}
				else //OverSize
				{
					eWholeNgTypeTemp[TypeSolderExcess] = e_NG;
					nCurrentNgType = TypeSolderExcess;
					eAlgoNgType[TypeSolderExcess] = e_NG;
				}

			}

			if (!rstAlgo->bIsOKHeight)
			{

				if (rstAlgo->fRstHeight < pAlgoBump->farrdata[Bump_F_HeightMin]) //UnderSize
				{
					eWholeNgTypeTemp[TypeSolderFilet] = e_NG;
					nCurrentNgType = TypeSolderFilet;
					eAlgoNgType[TypeSolderFilet] = e_NG;
				}
				else //OverSize
				{
					eWholeNgTypeTemp[TypeSolderExcess] = e_NG;
					nCurrentNgType = TypeSolderExcess;
					eAlgoNgType[TypeSolderExcess] = e_NG;
				}

			}

		}
		if (!rstAlgo->bIsOKHeightDev)
		{
			eWholeNgTypeTemp[TypeDamage] = e_NG;
			nCurrentNgType = TypeDamage;
			eAlgoNgType[TypeDamage] = e_NG;
		}
		if (!rstAlgo->bIsOKVolume)
		{
			InspAlgo& inspAlgo = vArrAlgoParam;
			AlgoBump* pAlgoBump = (AlgoBump*)inspAlgo.m_ptrInspAlgoParam;

			double minRange = pAlgoBump->farrdata[Bump_F_TeachVolume] * pAlgoBump->farrdata[Bump_F_VolumeRateMin] / 100.;
			double maxRange = pAlgoBump->farrdata[Bump_F_TeachVolume] * pAlgoBump->farrdata[Bump_F_VolumeRateMax] / 100.;
			if (rstAlgo->fRstVolume > maxRange)
			{
				eWholeNgTypeTemp[TypeSolderExcess] = e_NG;  //TypeOverSize
				nCurrentNgType = TypeSolderExcess;
				eAlgoNgType[TypeSolderExcess] = e_NG;
			}

			if (rstAlgo->fRstVolume < minRange)
			{
				eWholeNgTypeTemp[TypeMountMissing] = e_NG;  //TypeUnderSize
				nCurrentNgType = TypeMountMissing;
				eAlgoNgType[TypeMountMissing] = e_NG;
			}
		}
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoBump::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoBump::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	return bResult;
}

BOOL CPInsp_AlgoBump::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;
// 
// 	AlgoBump *pAlgo = (AlgoBump *)sInspAlgo.m_ptrInspAlgoParam;
// 	pAlignRes->TeachCenterX = pAlgo->farrdata[Bump_F_TechCenterX];
// 	pAlignRes->TeachCenterY = pAlgo->farrdata[Bump_F_TechCenterY];
// 
// 	RstAlgoBump * rst = (RstAlgoBump *)sRstAlgo;
// 	pAlignRes->offsetX = (double)-rst->fRstShiftX;
// 	pAlignRes->offsetY = (double)rst->fRstShiftY;
// 	pAlignRes->theta = 0.0;
// 	bRet = TRUE;

	return bRet;
}
int CPInsp_AlgoBump::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoBump::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}