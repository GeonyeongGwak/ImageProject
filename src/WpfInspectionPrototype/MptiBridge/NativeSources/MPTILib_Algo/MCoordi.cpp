#include "StdAfx.h"
#include "MCoordi.h"
#include "MPTI.h"


#include <math.h>

#include "alp_alloc.h"
#define DEF_FIDU_COUNT	200
#define PI   3.141592
#define PI_2	1.570796	// KSH 2014/09/29
#define COORDIALGO 3	// SHKang 2018/07/31 : 좌표 계산방법 설정

#include "ReadWriteFile.h"
//#include "compute.h"

CMCoordi* g_pMCoordi = NULL;

CMCoordi::CMCoordi(void)
{
	g_pMCoordi = this;

	m_anchorMachineX = false;
	m_anchorMachineY = false;
	m_anchorBoardX = false;
	m_anchorBoardY = false;
	m_anchorAlineX = false;
	m_anchorAlineY = false;

	m_machineAreaMinX = 0.0;
	m_machineAreaMinY = 0.0;
	m_machineAreaMaxX = 0.0;
	m_machineAreaMaxY = 0.0;


	m_dirMachineX = 1.0;
	m_dirMachineY = 1.0;
	m_dirBoardX = 1.0;
	m_dirBoardY = 1.0;

	m_sizeBoardX = 0.0;
	m_sizeBoardY = 0.0;	

	m_fovPixelNumX = 0;
	m_fovPixelNumY = 0;
	m_fovPixelResolX = 1.0;
	m_fovPixelResolY = 1.0;
	m_fovSizeX = 0.0;
	m_fovSizeY = 0.0;
	
	//m_boardFiduNum = FIDU_NUM_BOARD;

	m_ppModuleFiduPosOrg = NULL;
	m_ppModuleFiduPosTeach = NULL;
	m_ppModuleFiduDelta = NULL;
	m_pModuleCoordiCorrector = NULL;

	m_pStdPartFiduPos = NULL;
	m_pPartFiduDelta = NULL;
	m_pPartCoordiCorrector = NULL;

	CreateBoardFiducials();

	m_moduleNum = 0;
	m_partNum = 0;
	m_moduleFiduNum = 0;// DEF_FIDU_COUNT;
	m_partFiduNum = FIDU_NUM_PART;
	CreateSubFiducials(0, FIDU_NUM_PART);

	m_eCalibCode = MC_NONE;
	m_calib_ref = NULL;
	m_calib_obj = NULL;
	m_calib_Rear_ref = NULL;
	m_calib_Rear_obj = NULL;

	for(int i = 0 ; i < 2; i++)
	{
		m_fRef_offsetX[i] = 0;
		m_fRef_offsetY[i] = 0;
		m_fObj_offsetX[i] = 0;
		m_fObj_offsetY[i] = 0;

	}

	m_dDistanceOfOut1 = 0.0;
	m_nMachineFlowType = (int)eMMF_LR;
	m_nCoordiAlgo = COORDIALGO;
	/*LoadMatrixCalibrationFile( L"C:\\Eagle3D_64x\\MOTION\\calib_o.dat", 0);
	LoadMatrixCalibrationFile( L"C:\\Eagle3D_64x\\MOTION\\calib_r.dat", 1);*/
	
// 	LoadMatrixCalibrationFile( TEXT("C:\\") + g_pMPTI->m_ProcessName + TEXT("_64x\\MOTION\\calib_r_aoi.dat"), 0, (int)eMCRI_F);
// 	LoadMatrixCalibrationFile( TEXT("C:\\") + g_pMPTI->m_ProcessName + TEXT("_64x\\MOTION\\calib_o_aoi.dat"), 1, (int)eMCRI_F);
// 	LoadMatrixCalibrationFile( TEXT("C:\\") + g_pMPTI->m_ProcessName + TEXT("_64x\\MOTION\\calib_Rear_r_aoi.dat"), 0, (int)eMCRI_R);
// 	LoadMatrixCalibrationFile( TEXT("C:\\") + g_pMPTI->m_ProcessName + TEXT("_64x\\MOTION\\calib_Rear_o_aoi.dat"), 1, (int)eMCRI_R);

	m_bNewCoordiAlgo = true;
}


CMCoordi::~CMCoordi(void)
{
	g_pMCoordi = NULL;

	DeleteBoardFiducials();
	DeleteSubFiducials();

	if(m_calib_ref) Delete_2DArray(&m_calib_ref, m_calibHeader[(int)eMCRI_F].row, m_calibHeader[(int)eMCRI_F].col);
	if(m_calib_obj) Delete_2DArray(&m_calib_obj, m_calibHeader[(int)eMCRI_F].row, m_calibHeader[(int)eMCRI_F].col);

	if(m_calib_Rear_ref) Delete_2DArray(&m_calib_Rear_ref, m_calibHeader[(int)eMCRI_R].row, m_calibHeader[(int)eMCRI_R].col);
	if(m_calib_Rear_obj) Delete_2DArray(&m_calib_Rear_obj, m_calibHeader[(int)eMCRI_R].row, m_calibHeader[(int)eMCRI_R].col);
}


int CMCoordi::SetAnchorMachine(int anchor)
{
	if((anchor < eMCA_LB) || (anchor > eMCA_RT)) return -1;

	switch (anchor)
	{
	case eMCA_LB:
		m_anchorMachineX = false;
		m_anchorMachineY = false;
		break;
	case eMCA_RB:

		m_anchorMachineX = true;
		m_anchorMachineY = false;
		break;
	case eMCA_LT:
		m_anchorMachineX = false;
		m_anchorMachineY = true;
		break;
	case eMCA_RT:
		m_anchorMachineX = true;
		m_anchorMachineY = true;
		break;
	}
	
	return 0;
}


int CMCoordi::SetAnchorBoard(int anchor)
{
	if((anchor < eMCA_LB) || (anchor > eMCA_RT)) return -1;

	switch (anchor)
	{
	case eMCA_LB:
		m_anchorBoardX = false;
		m_anchorBoardY = false;
		break;
	case eMCA_RB:
		m_anchorBoardX = true;
		m_anchorBoardY = false;
		break;
	case eMCA_LT:
		m_anchorBoardX = false;
		m_anchorBoardY = true;
		break;
	case eMCA_RT:
		m_anchorBoardX = true;
		m_anchorBoardY = true;
		break;
	}
	
	return 0;
}


int CMCoordi::SetAnchorAline(int anchor)
{
	if((anchor < eMCA_LB) || (anchor > eMCA_RT)) return -1;

	switch (anchor)
	{
	case eMCA_LB:
		m_anchorAlineX = false;
		m_anchorAlineY = false;
		break;
	case eMCA_RB:
		m_anchorAlineX = true;
		m_anchorAlineY = false;
		break;
	case eMCA_LT:
		m_anchorAlineX = false;
		m_anchorAlineY = true;
		break;
	case eMCA_RT:
		m_anchorAlineX = true;
		m_anchorAlineY = true;
		break;
	}

	return 0;
}


int CMCoordi::SetSizeBoard(double sizeX, double sizeY)
{
	m_sizeBoardX = sizeX;
	m_sizeBoardY = sizeY;

	return 0;
}


int CMCoordi::SetBoardAlinePos(MPTICoordinateMachine *pPos)
{
	m_boardAlinePos.x = pPos->x;
	m_boardAlinePos.y = pPos->y;
	
	return 0;
}

int CMCoordi::GetBoardAlinePos(MPTICoordinateMachine *pPos)
{
	pPos->x = m_boardAlinePos.x;
	pPos->y = m_boardAlinePos.y;

	return eMR_SUCCESS;
}

int CMCoordi::SetDistanceOfOut1(double dDistanceX, int nMachineFlowType)
{
	m_dDistanceOfOut1 = dDistanceX;
	m_nMachineFlowType = nMachineFlowType;

	return eMR_SUCCESS;
}


int CMCoordi::GetBoardSize(double *sizeX, double *sizeY)
{
	*sizeX = m_sizeBoardX;
	*sizeY = m_sizeBoardY;

	return 0;
}


int CMCoordi::GenerateCoordinate()
{
	//
	// machine coordinate X
	//
	if(m_anchorMachineX == false)
	{
		m_dirMachineX = 1.0;
	}
	else
	{
		m_dirMachineX = -1.0;
	}
	//
	// machine coordinate Y
	//
	if(m_anchorMachineY == false)
	{
		m_dirMachineY = 1.0;
	}
	else
	{
		m_dirMachineY = -1.0;
	}
	//
	// board coordinate X
	//
	if(m_anchorBoardX == false)
	{
		m_dirBoardX = 1.0;
	}
	else
	{
		m_dirBoardX = -1.0;
	}
	//
	// board coordinate Y
	//
	if(m_anchorBoardY == false)
	{
		m_dirBoardY = 1.0;
	}
	else
	{
		m_dirBoardY = -1.0;
	}
	//
	// board anchor X
	//
	if(m_anchorBoardX == m_anchorAlineX)
	{
		m_boardAnchorPos.x = m_boardAlinePos.x;
	}
	else
	{
		m_boardAnchorPos.x = m_boardAlinePos.x - (m_sizeBoardX * m_dirMachineX);
	}
	//
	// board anchor Y
	//
	if(m_anchorBoardY == m_anchorAlineY)
	{
		m_boardAnchorPos.y = m_boardAlinePos.y;
	}
	else
	{
		m_boardAnchorPos.y = m_boardAlinePos.y - (m_sizeBoardY * m_dirMachineY);
	}		
	
	return 0;
}


int CMCoordi::CreateBoardFiducials()
{
	DeleteBoardFiducials();

	/*alpa_make_array_1d(&m_pStdBoardFiduPos, m_boardFiduNum);*/
	/*alpa_make_array_1d(&m_pBoardFiduDelta, m_boardFiduNum);*/
	/*m_pStdBoardFiduPosOrg.SetCount(m_boardFiduNum);
	m_pStdBoardFiduPosTeach.SetCount(m_boardFiduNum);
	m_pBoardFiduDelta.SetCount(m_boardFiduNum);*/

	return 0;
}


int CMCoordi::CreateSubFiducials(int moduleNum, int partNum)
{
	DeleteSubFiducials();

	if(moduleNum > 0)
	{
		m_moduleNum = moduleNum;
		alpa_make_array_2d(&m_ppModuleFiduPosOrg, moduleNum, m_moduleFiduNum);
		alpa_make_array_2d(&m_ppModuleFiduPosTeach, moduleNum, m_moduleFiduNum);
		alpa_make_array_2d(&m_ppModuleFiduDelta, moduleNum, m_moduleFiduNum);
		alpa_make_array_1d(&m_pModuleCoordiCorrector, moduleNum);

		if(partNum > 0)
		{
			m_partNum = partNum;
			alpa_make_array_3d(&m_pStdPartFiduPos, moduleNum, partNum, m_partFiduNum);
			alpa_make_array_3d(&m_pPartFiduDelta, moduleNum, partNum, m_partFiduNum);
			alpa_make_array_2d(&m_pPartCoordiCorrector, moduleNum, partNum);
		}
	}

	return 0;
}


int CMCoordi::DeleteBoardFiducials()
{
	/*alpa_delete_array_1d(&m_pStdBoardFiduPos);*/
	/*alpa_delete_array_1d(&m_pBoardFiduDelta);*/

	m_pStdBoardFiduPosOrg.RemoveAll();
	m_pStdBoardFiduPosTeach.RemoveAll();
	m_pBoardFiduDelta.RemoveAll();

	// Module Fiducial 정보도 초기화해주어야 한다.
	if(m_ppModuleFiduPosOrg != NULL)
	{
		for(int nModel = 0; nModel < m_moduleNum; nModel++)
		{		
			memset(m_ppModuleFiduPosOrg[nModel], 0, sizeof(MPTICoordinateBoard)*m_moduleFiduNum);
			memset(m_ppModuleFiduPosTeach[nModel], 0, sizeof(MPTICoordinateBoard)*m_moduleFiduNum);
			memset(m_ppModuleFiduDelta[nModel], 0, sizeof(MPTICoordinateBoard)*m_moduleFiduNum);		
		}
	}
	if(m_pModuleCoordiCorrector != NULL)
		memset(m_pModuleCoordiCorrector, 0, sizeof(MPTICoordinateCorrector) * m_moduleNum);

	return 0;
}


int CMCoordi::DeleteSubFiducials()
{
	alpa_delete_array_2d(&m_ppModuleFiduPosOrg, m_moduleNum);
	alpa_delete_array_2d(&m_ppModuleFiduPosTeach, m_moduleNum);
	alpa_delete_array_2d(&m_ppModuleFiduDelta, m_moduleNum);
	alpa_delete_array_1d(&m_pModuleCoordiCorrector);

	alpa_delete_array_3d(&m_pStdPartFiduPos, m_moduleNum, m_partNum);
	alpa_delete_array_3d(&m_pPartFiduDelta, m_moduleNum, m_partNum);
	alpa_delete_array_2d(&m_pPartCoordiCorrector, m_moduleNum);

	return 0;
}


int CMCoordi::GetBoardFiducialNumber(int *fiduNum)
{
	*fiduNum = min(m_pStdBoardFiduPosOrg.GetCount(), m_pStdBoardFiduPosTeach.GetCount());

	return 0;
}


int CMCoordi::GetSubFiducialNumber(int *moduleNum, int *moduleFiduNum, int *partNum, int *partFiduNum)
{
	*moduleNum = m_moduleNum;
	*moduleFiduNum = m_moduleFiduNum;
	*partNum = m_partNum;
	*partFiduNum = m_partFiduNum;
	
	return 0;
}


int CMCoordi::SetStdBoardFiducialPos(int idxFidu, MPTICoordinateBoard *pPosOrg, MPTICoordinateBoard *pPosTeach)
{
	SetBoardFiducialGlobal(idxFidu, pPosOrg);
	SetBoardFiducialGlobal(idxFidu, pPosTeach, FALSE);

	return 0;
}

int CMCoordi::SetBoardFiducialGlobal( int idxFidu, MPTICoordinateBoard *pPos, BOOL bOrigin/*=TRUE */ )
{
	if(idxFidu < 0) return -1;
	if(pPos == NULL) return -1;

	CAtlArray <MPTICoordinateBoard> &pStdBoardFiduPos = (bOrigin) ? m_pStdBoardFiduPosOrg : m_pStdBoardFiduPosTeach;

	int nboardFiduNum = pStdBoardFiduPos.GetCount();
	if(idxFidu >= nboardFiduNum)
	{
		pStdBoardFiduPos.Add(*pPos);
	}
	else
	{
		pStdBoardFiduPos[idxFidu].x = pPos->x;
		pStdBoardFiduPos[idxFidu].y = pPos->y;
	}

	return 0;
}


int CMCoordi::SetStdModuleFiducialPos(int idxModule, int idxFidu, MPTICoordinateBoard *pPosOrg, MPTICoordinateBoard *pPosTeach)
{
	if((m_moduleNum <= 0) || (m_moduleFiduNum <= 0)) return -1;
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;
	if((idxFidu < 0) || (idxFidu >= m_moduleFiduNum)) return -1;
	if(m_ppModuleFiduPosOrg == NULL) return -1;
	if(m_ppModuleFiduPosTeach == NULL) return -1;
	if(pPosOrg == NULL) return -1;

	m_ppModuleFiduPosOrg[idxModule][idxFidu].x = pPosOrg->x;
	m_ppModuleFiduPosOrg[idxModule][idxFidu].y = pPosOrg->y;
	m_ppModuleFiduPosTeach[idxModule][idxFidu].x = pPosTeach->x;
	m_ppModuleFiduPosTeach[idxModule][idxFidu].y = pPosTeach->y;

	return 0;
}


int CMCoordi::SetStdPartFiducialPos(int idxModule, int idxPart, int idxFidu, MPTICoordinateBoard *pPos)
{
	if((m_partNum <= 0) || (m_partFiduNum <= 0)) return -1;
	if((idxModule < 0) || (idxModule >= m_partNum)) return -1;
	if((idxFidu < 0) || (idxFidu >= m_partFiduNum)) return -1;
	if(m_pStdPartFiduPos == NULL) return -1;
	if(pPos == NULL) return -1;

	m_pStdPartFiduPos[idxModule][idxPart][idxFidu].x = pPos->x;
	m_pStdPartFiduPos[idxModule][idxPart][idxFidu].y = pPos->y;

	return 0;
}


int CMCoordi::SetBoardFiducialDelta(int idxFidu, MPTICoordinateBoard *pDelta)
{
	int nboardFiduNum = m_pBoardFiduDelta.GetCount();
	if(idxFidu < 0) return -1;
	if(pDelta == NULL) return -1;

	if(idxFidu >= nboardFiduNum)
	{
		m_pBoardFiduDelta.Add(*pDelta);
	}
	else
	{
		m_pBoardFiduDelta[idxFidu].x = pDelta->x;
		m_pBoardFiduDelta[idxFidu].y = pDelta->y;
	}

	return 0;
}


int CMCoordi::SetModuleFiducialDelta(int idxModule, int idxFidu, MPTICoordinateBoard *pDelta)
{
	if((m_moduleNum <= 0) || (m_moduleFiduNum <= 0)) return -1;
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;
	if((idxFidu < 0) || (idxFidu >= m_moduleFiduNum)) return -1;
	if(m_ppModuleFiduDelta == NULL) return -1;
	if(pDelta == NULL) return -1;

	m_ppModuleFiduDelta[idxModule][idxFidu].x = pDelta->x;
	m_ppModuleFiduDelta[idxModule][idxFidu].y = pDelta->y;

	return 0;
}


int CMCoordi::SetPartFiducialDelta(int idxModule, int idxPart, int idxFidu, MPTICoordinateBoard *pDelta)
{
	if((m_partNum <= 0) || (m_partFiduNum <= 0)) return -1;
	if((idxModule < 0) || (idxModule >= m_partNum)) return -1;
	if((idxFidu < 0) || (idxFidu >= m_partFiduNum)) return -1;
	if(m_pPartFiduDelta == NULL) return -1;
	if(pDelta == NULL) return -1;

	m_pPartFiduDelta[idxModule][idxPart][idxFidu].x = pDelta->x;
	m_pPartFiduDelta[idxModule][idxPart][idxFidu].y = pDelta->y;

	return 0;
}


int CMCoordi::ClearAllCoordiCorrector()
{
	ClearBoardCoordiCorrector();

	for (int i=0 ; i<m_moduleNum ; i++)
	{
		ClearModuleCoordiCorrector(i);
	
		for (int j=0 ; j<m_partNum ; j++)
		{
			ClearPartCoordiCorrector(i, j);
		}
	}

	return 0;
}


int CMCoordi::ClearBoardCoordiCorrector()
{
	ZeroMemory(&m_boardCoordiCorrector, sizeof(MPTICoordinateCorrector));

	m_boardCoordiCorrector.SetGenerate(FALSE);

	return 0;
}


int CMCoordi::ClearModuleCoordiCorrector(int idxModule)
{
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;
	
	ZeroMemory(&m_pModuleCoordiCorrector[idxModule], sizeof(MPTICoordinateCorrector));
	m_pModuleCoordiCorrector[idxModule].SetGenerate(FALSE);
	return 0;
}


int CMCoordi::ClearPartCoordiCorrector(int idxModule, int idxPart)
{
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;
	if((m_partNum <= 0) || (idxPart <= m_partNum)) return -1;

	ZeroMemory(&m_pPartCoordiCorrector[idxModule][idxPart], sizeof(MPTICoordinateCorrector));
	m_pPartCoordiCorrector[idxModule][idxPart].SetGenerate(FALSE);
	return 0;
}


int CMCoordi::GenerateAllCoordiCorrector()
{
	GenerateBoardCoordiCorrector();

	for (int i=0 ; i<m_moduleNum ; i++)
	{
		GenerateModuleCoordiCorrector(i);

		for (int j=0 ; j<m_partNum ; j++)
		{
			GeneratePartCoordiCorrector(i, j);
		}
	}

	return 0;
}


int CMCoordi::GenerateBoardCoordiCorrector()
{
	if((m_pStdBoardFiduPosOrg.GetCount() == 0) || (m_pStdBoardFiduPosTeach.GetCount() == 0) || (m_pBoardFiduDelta.GetCount() == 0)) return -1;

	return CalcCorrector2(eTypeGlobal, &m_boardCoordiCorrector);
	//return CalcCorrector(m_pStdBoardFiduPos[0], m_pStdBoardFiduPos[1], m_pBoardFiduDelta[0], m_pBoardFiduDelta[1], &m_boardCoordiCorrector);
}


int CMCoordi::GenerateModuleCoordiCorrector(int idxModule)
{
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;

	return CalcCorrector2(eTypeModule, &m_pModuleCoordiCorrector[idxModule], idxModule);
	//return CalcCorrector(m_pStdModuleFiduPos[idxModule][0], m_pStdModuleFiduPos[idxModule][1], m_pModuleFiduDelta[idxModule][0], m_pModuleFiduDelta[idxModule][1], &m_pModuleCoordiCorrector[idxModule]);
}


int CMCoordi::GeneratePartCoordiCorrector(int idxModule, int idxPart)
{
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;
	if((m_partNum <= 0) || (idxPart <= m_partNum)) return -1;

	return CalcCorrector2(eTypePart, &m_pPartCoordiCorrector[idxModule][idxPart], idxModule, idxPart);
	//return CalcCorrector(m_pStdPartFiduPos[idxModule][idxPart][0], m_pStdPartFiduPos[idxModule][idxPart][1], m_pPartFiduDelta[idxModule][idxPart][0], m_pPartFiduDelta[idxModule][idxPart][1], &m_pPartCoordiCorrector[idxModule][idxPart]);
}


int CMCoordi::CalcCorrector(MCOORDI_B stdPos1, MCOORDI_B stdPos2, MCOORDI_B delta1, MCOORDI_B delta2, LP_MCOORDI_COORECTOR lpCorrector)
{
	double stdDistX, stdDistY;
	double curDistX, curDistY;
	double stdTheta, curTheta;
	double deltaTheta;

	double stdPosSx, stdPosSy, stdPosEx, stdPosEy;
	double curPosSx, curPosSy, curPosEx, curPosEy;
	double offsetX, offsetY, deltaX, deltaY;
	
	// sequence must be cross
	//if(((stdPosX1 > stdPosX2) && (stdPosY1 < stdPosY2)) || ((stdPosX1 < stdPosX2) && (stdPosY1 < stdPosY2)))	// start from bottom
	if((stdPos1.y < stdPos2.y) && (stdPos1.x != stdPos2.x))	// start from bottom
	{
		stdPosSx = 0.0;
		stdPosSy = 0.0;
		stdPosEx = stdPos2.x - stdPos1.x;
		stdPosEy = stdPos2.y - stdPos1.y;

		curPosSx = stdPosSx + delta1.x;
		curPosSy = stdPosSy + delta1.y;
		curPosEx = stdPosEx + delta2.x;
		curPosEy = stdPosEy + delta2.y;

		offsetX = stdPos1.x;
		offsetY = stdPos1.y;
		deltaX = delta1.x;
		deltaY = delta1.y;
	}
	//else if(((stdPosX1 < stdPosX2) && (stdPosY1 > stdPosY2)) || ((stdPosX1 > stdPosX2) && (stdPosY1 > stdPosY2)))	// start from top
	else if((stdPos1.y > stdPos2.y) && (stdPos1.x != stdPos2.x))	// start from top
	{
		stdPosSx = 0.0;
		stdPosSy = 0.0;
		stdPosEx = stdPos1.x - stdPos2.x;
		stdPosEy = stdPos1.y - stdPos2.y;

		curPosSx = stdPosSx + delta2.x;
		curPosSy = stdPosSy + delta2.y;
		curPosEx = stdPosEx + delta1.x;
		curPosEy = stdPosEy + delta1.y;

		offsetX = stdPos2.x;
		offsetY = stdPos2.y;
		deltaX = delta2.x;
		deltaY = delta2.y;
	}
	else
	{
		return -1;
	}

	stdDistX = stdPosEx - stdPosSx;
	stdDistY = stdPosEy - stdPosSy;
	stdTheta = (double)atan2(stdDistY, stdDistX);

	curDistX = curPosEx - curPosSx;
	curDistY = curPosEy - curPosSy;
	curTheta = (double)atan2(curDistY, curDistX);

	deltaTheta =  curTheta - stdTheta;
	/*
	if(deltaTheta > PI)
	{
		deltaTheta = deltaTheta - PI;
	}
	else if(deltaTheta < -PI)
	{
		deltaTheta = -PI - deltaTheta;
	}
	*/

	lpCorrector->stdTheta = stdTheta * (180.0 / PI);	// radian to degree
	lpCorrector->curTheta = curTheta * (180.0 / PI);
	lpCorrector->theta = deltaTheta * (180.0 / PI);
	lpCorrector->offsetX = offsetX;
	lpCorrector->offsetY = offsetY;
	lpCorrector->deltaX = deltaX;
	lpCorrector->deltaY = deltaY;

	return 0;
}


  int CMCoordi::CalcCorrector2(TYPECORRECT eTypeCorrect, LP_MCOORDI_COORECTOR lpCorrector, int idxModule/*=-1*/, int idxPart/*=-1*/)
  {
  	double stdDistX, stdDistY;
  	double curDistX, curDistY;
  	double stdTheta, curTheta;
  	double deltaTheta;
  
  	double stdPosSx, stdPosSy, stdPosEx, stdPosEy, stdPosCx, stdPosCy;
  	double curPosSx, curPosSy, curPosEx, curPosEy, curPosCx, curPosCy;
  	double offsetX = 0, offsetY = 0, deltaX, deltaY;
  
  	MCOORDI_B posSttOrg, posEndOrg; 
  	MCOORDI_B posSttTeach, posEndTeach;
  	MCOORDI_B deltaSttTeach, deltaEndTeach;
  	lpCorrector->SetGenerate(FALSE);
  
  	if(eTypeCorrect == eTypeGlobal)
  	{
  		return CalcCorrectorNew(lpCorrector);		
  	}
  	else if(eTypeCorrect == eTypeModule)
  	{
  		if(m_bNewCoordiAlgo != false)
  		{
  			int nFiduNum = 0;
  			for(int i = 0; i < m_moduleFiduNum; ++i)
  			{
  				if(!m_ppModuleFiduPosOrg[idxModule][i].IsValidate())
  					continue;
  				nFiduNum++;
  			}
  			int nIDX = 0;
  			MCOORDI_B poFiduOrg[DEF_FIDU_COUNT];
  			MCOORDI_B poFiduReal[DEF_FIDU_COUNT];
  			for(int a = 0; a < nFiduNum; a++)
  			{
//   				if (g_pMPTI && m_dInspRate > 0)
//   				{
//   					LP_FIDU_RESULT pFiduResult = g_pMPTI->GetFiduRst(idxModule + 1, a);
//   					if (pFiduResult->dScore < m_dInspRate)
//   						continue;
//   				}
  				poFiduReal[nIDX].x = m_ppModuleFiduPosTeach[idxModule][a].x + m_ppModuleFiduDelta[idxModule][a].x;
  				poFiduReal[nIDX].y = m_ppModuleFiduPosTeach[idxModule][a].y + m_ppModuleFiduDelta[idxModule][a].y;
  				poFiduOrg[nIDX].x = m_ppModuleFiduPosOrg[idxModule][a].x;
  				poFiduOrg[nIDX].y = m_ppModuleFiduPosOrg[idxModule][a].y;
  				offsetX += poFiduOrg[nIDX].x;
  				offsetY += poFiduOrg[nIDX].y;
  				nIDX++;
  			}
  			CalcCorrect_ThetaDeltaScale(nIDX, poFiduOrg, poFiduReal, &deltaTheta, &deltaX, &deltaY, &lpCorrector->scaleX, &lpCorrector->scaleY);
  
  			stdDistX = m_ppModuleFiduPosOrg[idxModule][0].x - m_ppModuleFiduPosOrg[idxModule][1].x;
  			stdDistY = m_ppModuleFiduPosOrg[idxModule][0].y - m_ppModuleFiduPosOrg[idxModule][1].y;
  			stdTheta = (double)atan2(stdDistY, stdDistX);
  
  			curDistX = poFiduReal[0].x - poFiduReal[1].x;
  			curDistY = poFiduReal[0].y - poFiduReal[1].y;
  			curTheta = (double)atan2(curDistY, curDistX);
  
  			lpCorrector->stdTheta = stdTheta * (180.0 / PI);	// radian to degree
  			lpCorrector->curTheta = curTheta * (180.0 / PI);
  			lpCorrector->theta = deltaTheta * (180.0 / PI);
  			lpCorrector->offsetX = offsetX / nIDX;
  			lpCorrector->offsetY = offsetY / nIDX;
  			lpCorrector->deltaX = deltaX;
  			lpCorrector->deltaY = deltaY;
  			lpCorrector->SetGenerate(TRUE);
  			return 0;
  		}
  
  		posSttOrg = m_ppModuleFiduPosOrg[idxModule][0], posEndOrg = m_ppModuleFiduPosOrg[idxModule][1];
  		posSttTeach = m_ppModuleFiduPosTeach[idxModule][0], posEndTeach = m_ppModuleFiduPosTeach[idxModule][1];
  		deltaSttTeach = m_ppModuleFiduDelta[idxModule][0], deltaEndTeach = m_ppModuleFiduDelta[idxModule][1];
  	}
  	else if(eTypeCorrect == eTypePart)
  	{
  		posSttTeach = m_pStdPartFiduPos[idxModule][idxPart][0], posEndTeach = m_pStdPartFiduPos[idxModule][idxPart][1];
  		deltaSttTeach = m_pPartFiduDelta[idxModule][idxPart][0], deltaEndTeach = m_pPartFiduDelta[idxModule][idxPart][1];
  	}
  
  	// sequence must be cross
  	if((posSttTeach.y < posEndTeach.y) && (posSttTeach.x != posEndTeach.x))	// start from bottom
  	{
  		curPosSx = posSttTeach.x + deltaSttTeach.x;
  		curPosSy = posSttTeach.y + deltaSttTeach.y;
  		curPosEx = posEndTeach.x + deltaEndTeach.x;
  		curPosEy = posEndTeach.y + deltaEndTeach.y;
  
  		if(eTypeCorrect == eTypeModule)
  		{
  			stdPosSx = posSttOrg.x;
  			stdPosSy = posSttOrg.y;
  			stdPosEx = posEndOrg.x;
  			stdPosEy = posEndOrg.y;
  		}
  		else
  		{
  			stdPosSx = posSttTeach.x;
  			stdPosSy = posSttTeach.y;
  			stdPosEx = posEndTeach.x;
  			stdPosEy = posEndTeach.y;
  		}
  	}
  	else if((posSttTeach.y > posEndTeach.y) && (posSttTeach.x != posEndTeach.x))	// start from top
  	{
  		curPosSx = posEndTeach.x + deltaEndTeach.x;
  		curPosSy = posEndTeach.y + deltaEndTeach.y;
  		curPosEx = posSttTeach.x + deltaSttTeach.x;
  		curPosEy = posSttTeach.y + deltaSttTeach.y;
  
  		if(eTypeCorrect == eTypeModule)
  		{
  			stdPosSx = posEndOrg.x;
  			stdPosSy = posEndOrg.y;
  			stdPosEx = posSttOrg.x;
  			stdPosEy = posSttOrg.y;
  		}
  		else
  		{
  			stdPosSx = posEndTeach.x;
  			stdPosSy = posEndTeach.y;
  			stdPosEx = posSttTeach.x;
  			stdPosEy = posSttTeach.y;
  		}
  
  	}
  	else
  	{
  		return -1;
  	}
  
  	stdPosCx = stdPosSx + (stdPosEx - stdPosSx) / 2.0;
  	stdPosCy = stdPosSy + (stdPosEy - stdPosSy) / 2.0;
  	curPosCx = curPosSx + (curPosEx - curPosSx) / 2.0;
  	curPosCy = curPosSy + (curPosEy - curPosSy) / 2.0;
  	offsetX = stdPosCx;
  	offsetY = stdPosCy;
  
  	stdDistX = stdPosEx - stdPosSx;
  	stdDistY = stdPosEy - stdPosSy;
  	stdTheta = (double)atan2(stdDistY, stdDistX);
  
  	curDistX = curPosEx - curPosSx;
  	curDistY = curPosEy - curPosSy;
  	curTheta = (double)atan2(curDistY, curDistX);
  
  	deltaTheta = curTheta - stdTheta;
  
  
  	if(eTypeCorrect == eTypeModule)
  	{
  		MPTICoordinateBoard posCenterOrg;
  		{
  			int nCntFiducial = 0;
  			for(int i = 0; i < m_moduleFiduNum; ++i)
  			{
  				MPTICoordinateBoard posFiducial = m_ppModuleFiduPosOrg[idxModule][i];
  				if(!posFiducial.IsValidate())
  					continue;
  
  				posCenterOrg.x += (posFiducial.x);
  				posCenterOrg.y += (posFiducial.y);
  
  				++nCntFiducial;
  			}
  			posCenterOrg.x = posCenterOrg.x / nCntFiducial; 
  			posCenterOrg.y = posCenterOrg.y / nCntFiducial; 
  			offsetX = posCenterOrg.x;
  			offsetY = posCenterOrg.y;
  		}
  		MPTICoordinateBoard posCenterTeach;
  		{
  			int nCntFiducial = 0;
  			for(int i = 0; i < m_moduleFiduNum; ++i)
  			{
  				MPTICoordinateBoard posFiducial = m_ppModuleFiduPosTeach[idxModule][i];
  				MPTICoordinateBoard posDelta = m_ppModuleFiduDelta[idxModule][i];
  				if(!posFiducial.IsValidate())
  					continue;
  
  				posCenterTeach.x += (posFiducial.x + posDelta.x);
  				posCenterTeach.y += (posFiducial.y + posDelta.y);
  
  				++nCntFiducial;
  			}
  			posCenterTeach.x = posCenterTeach.x / nCntFiducial;
  			posCenterTeach.y = posCenterTeach.y / nCntFiducial;
  
  			++nCntFiducial;
  		}
  		deltaX = posCenterTeach.x - posCenterOrg.x;
  		deltaY = posCenterTeach.y - posCenterOrg.y;
  	}
  	else
  	{
  		deltaX = curPosCx - stdPosCx;
  		deltaY = curPosCy - stdPosCy;
  	}
  
  	lpCorrector->stdTheta = stdTheta * (180.0 / PI);	// radian to degree
  	lpCorrector->curTheta = curTheta * (180.0 / PI);
  	lpCorrector->theta = deltaTheta * (180.0 / PI);
  	lpCorrector->offsetX = offsetX;
  	lpCorrector->offsetY = offsetY;
  	lpCorrector->deltaX = deltaX;
  	lpCorrector->deltaY = deltaY;
  
  	lpCorrector->SetGenerate(TRUE);
  
  	return 0;
  }
  
  int CMCoordi::CalcCorrectorNew( LP_MCOORDI_COORECTOR lpCorrector )
  {
  	double stdDistX, stdDistY;
  	double curDistX, curDistY;
  	double stdTheta, curTheta;
  	double deltaTheta;
  
  	double stdPosSx, stdPosSy, stdPosEx, stdPosEy/*, stdPosCx, stdPosCy*/;
  	double curPosSx, curPosSy, curPosEx, curPosEy/*, curPosCx, curPosCy*/;
  	double offsetX = 0, offsetY = 0, deltaX = 0, deltaY = 0, scaleX = 0, scaleY = 0;
  
  	if(m_bNewCoordiAlgo != false)
  	{
  		int nFiduNum = 2;
  		GetBoardFiducialNumber(&nFiduNum);
  		int nIDX = 0;
  		MCOORDI_B poFiduOrg[DEF_FIDU_COUNT];
  		MCOORDI_B poFiduReal[DEF_FIDU_COUNT];
  		for(int a = 0; a < nFiduNum; a++)
  		{
//   			if (g_pMPTI && m_dInspRate > 0)
//   			{
//   				LP_FIDU_RESULT pFiduResult = g_pMPTI->GetFiduRst(0, a);
//   				if (pFiduResult->dScore < m_dInspRate)
//   					continue;
//   			}
  			poFiduOrg[nIDX].x = m_pStdBoardFiduPosOrg[a].x;
  			poFiduOrg[nIDX].y = m_pStdBoardFiduPosOrg[a].y;
  			poFiduReal[nIDX].x = m_pStdBoardFiduPosTeach[a].x + m_pBoardFiduDelta[a].x;
  			poFiduReal[nIDX].y = m_pStdBoardFiduPosTeach[a].y + m_pBoardFiduDelta[a].y;
  			offsetX += poFiduOrg[nIDX].x;
  			offsetY += poFiduOrg[nIDX].y;
  			nIDX++;
  		}
  		CalcCorrect_ThetaDeltaScale(nIDX, poFiduOrg, poFiduReal, &deltaTheta, &deltaX, &deltaY, &lpCorrector->scaleX, &lpCorrector->scaleY);
  
  		stdDistX = poFiduOrg[0].x - poFiduOrg[1].x;
  		stdDistY = poFiduOrg[0].y - poFiduOrg[1].y;
  		stdTheta = (double)atan2(stdDistY, stdDistX);
  
  		curDistX = poFiduReal[0].x - poFiduReal[1].x;
  		curDistY = poFiduReal[0].y - poFiduReal[1].y;
  		curTheta = (double)atan2(curDistY, curDistX);
  
  		lpCorrector->stdTheta = stdTheta * (180.0 / PI);	// radian to degree
  		lpCorrector->curTheta = curTheta * (180.0 / PI);
  		lpCorrector->theta = deltaTheta * (180.0 / PI);
  		lpCorrector->offsetX = offsetX / nIDX;
  		lpCorrector->offsetY = offsetY / nIDX;
  		lpCorrector->deltaX = deltaX;
  		lpCorrector->deltaY = deltaY;
  		lpCorrector->SetGenerate(TRUE);
  		return 0;
  	}
  
  	int nIdxStt(0), nIdxEnd(1);
  	MCOORDI_B stdPos1, stdPos2; 
  	MCOORDI_B delta1, delta2;
  
  	stdPos1 = m_pStdBoardFiduPosTeach[nIdxStt], stdPos2 = m_pStdBoardFiduPosTeach[nIdxEnd];
  	delta1 = m_pBoardFiduDelta[nIdxStt], delta2 = m_pBoardFiduDelta[nIdxEnd];
  
  	// sequence must be cross
  	if((stdPos1.y < stdPos2.y) && (stdPos1.x != stdPos2.x))	// start from bottom
  	{
  		nIdxStt = 0;
  		nIdxEnd = 1;
  	}
  	else if((stdPos1.y > stdPos2.y) && (stdPos1.x != stdPos2.x))	// start from top
  	{
  		nIdxStt = 1;
  		nIdxEnd = 0;
  		stdPos1 = m_pStdBoardFiduPosTeach[nIdxStt], stdPos2 = m_pStdBoardFiduPosTeach[nIdxEnd];
  		delta1 = m_pBoardFiduDelta[nIdxStt], delta2 = m_pBoardFiduDelta[nIdxEnd];
  	}
  	else
  	{
  		return -1;
  	}
  
  	stdPosSx = m_pStdBoardFiduPosOrg[nIdxStt].x;
  	stdPosSy = m_pStdBoardFiduPosOrg[nIdxStt].y;
  	stdPosEx = m_pStdBoardFiduPosOrg[nIdxEnd].x;
  	stdPosEy = m_pStdBoardFiduPosOrg[nIdxEnd].y;
  	curPosSx = stdPos1.x + delta1.x;
  	curPosSy = stdPos1.y + delta1.y;
  	curPosEx = stdPos2.x + delta2.x;
  	curPosEy = stdPos2.y + delta2.y;
  
  	offsetX = stdPosSx + (stdPosEx - stdPosSx) / 2.0;
  	offsetY = stdPosSy + (stdPosEy - stdPosSy) / 2.0;
  
  	stdDistX = stdPosEx - stdPosSx;
  	stdDistY = stdPosEy - stdPosSy;
  	stdTheta = (double)atan2(stdDistY, stdDistX);
  
  	curDistX = curPosEx - curPosSx;
  	curDistY = curPosEy - curPosSy;
  	curTheta = (double)atan2(curDistY, curDistX);
  
  	deltaTheta = curTheta - stdTheta;
  
  	MPTICoordinateBoard posCenterOrg;
  	{
  		int nCntFiducial = m_pStdBoardFiduPosOrg.GetCount();
  		int nCntDelta = m_pBoardFiduDelta.GetCount();
  		nCntFiducial = min(nCntFiducial, nCntDelta);
  		for(int i = 0; i < nCntFiducial; ++i)
  		{
  			MPTICoordinateBoard posFiducial = m_pStdBoardFiduPosOrg.GetAt(i);
  			posCenterOrg.x += (posFiducial.x);// - stdPosSx);
  			posCenterOrg.y += (posFiducial.y);// - stdPosSy);
  		}
  		posCenterOrg.x = posCenterOrg.x / nCntFiducial;//stdPosSx + 
  		posCenterOrg.y = posCenterOrg.y / nCntFiducial;//stdPosSy + 
  		offsetX = posCenterOrg.x;
  		offsetY = posCenterOrg.y;
  	}
  	MPTICoordinateBoard posCenterTeach;
  	{
  		int nCntFiducial = m_pStdBoardFiduPosTeach.GetCount();
  		int nCntDelta = m_pBoardFiduDelta.GetCount();
  		nCntFiducial = min(nCntFiducial, nCntDelta);
  		for(int i = 0; i < nCntFiducial; ++i)
  		{
  			MPTICoordinateBoard posFiducial = m_pStdBoardFiduPosTeach.GetAt(i);
  			MPTICoordinateBoard posDelta = m_pBoardFiduDelta.GetAt(i);
  			posCenterTeach.x += (posFiducial.x + posDelta.x);// - curPosSx);
  			posCenterTeach.y += (posFiducial.y + posDelta.y);// - curPosSy);
  		}
  		posCenterTeach.x = posCenterTeach.x / nCntFiducial;//curPosSx + 
  		posCenterTeach.y = posCenterTeach.y / nCntFiducial;//curPosSy + 
  	}
  	deltaX = posCenterTeach.x - posCenterOrg.x;
  	deltaY = posCenterTeach.y - posCenterOrg.y;
  
  
  	lpCorrector->stdTheta = stdTheta * (180.0 / PI);	// radian to degree
  	lpCorrector->curTheta = curTheta * (180.0 / PI);
  	lpCorrector->theta = deltaTheta * (180.0 / PI);
  	lpCorrector->offsetX = offsetX;
  	lpCorrector->offsetY = offsetY;
  	lpCorrector->deltaX = deltaX;
  	lpCorrector->deltaY = deltaY;
  	lpCorrector->SetGenerate(TRUE);
  	return 0;
  }
  
  int CMCoordi::CorrectCoordinate(MCOORDI_B pos, MCOORDI_COORECTOR corrector, LP_MCOORDI_B lpPos)
  {
  // 	double cosTheta = cos(corrector.theta);
  // 	double sinTheta = sin(corrector.theta);
  	double cosTheta = cos(corrector.theta * (PI / 180.0));	// degree to radian
  	double sinTheta = sin(corrector.theta * (PI / 180.0));
  
  	// move to standard origin position
  	double mX = pos.x - corrector.offsetX;
  	double mY = pos.y - corrector.offsetY;
  
  	// correct rotate theta
  	double rX =  mX * cosTheta - mY * sinTheta;
  	double rY =  mX * sinTheta + mY * cosTheta;
  
  	// correct shift delta
  	mX = rX + corrector.deltaX;
  	mY = rY + corrector.deltaY;
  
  	// move to current origin position
  	lpPos->x = mX + corrector.offsetX;
  	lpPos->y = mY + corrector.offsetY;
  
  	return 0;
  }


int CMCoordi::ReverseCorrectCoordinate(MCOORDI_B pos, MCOORDI_COORECTOR corrector, LP_MCOORDI_B lpPos)
{
	double theta = corrector.theta * (PI / 180.0) * -1.0;	// degree to radian
	double cosTheta = cos(theta);
	double sinTheta = sin(theta);

	// move to standard origin position
	double mX = pos.x - corrector.offsetX;
	double mY = pos.y - corrector.offsetY;

	// correct rotate theta
	double rX =  mX * cosTheta - mY * sinTheta;
	double rY =  mX * sinTheta + mY * cosTheta;

	// correct shift delta
	mX = rX - corrector.deltaX;
	mY = rY - corrector.deltaY;
// 	mX = rX + corrector.deltaX;
// 	mY = rY + corrector.deltaY;	

	// move to current origin position
	lpPos->x = mX + corrector.offsetX;
	lpPos->y = mY + corrector.offsetY;
// 	lpPos->x = rX + corrector.offsetX;
// 	lpPos->y = rY + corrector.offsetY;

	return 0;
}

// SHKang 2018/07/12
int CMCoordi::CorrectCoordinate_fromfidu(int nIndex, POINT_64F FiduPos, POINT_64F delta, double dTheta, POINT_64F srcPos, POINT_64F & dstPos) 
{
	double dRotPosX = FiduPos.x;
	double dRotPosY = FiduPos.y;

	double delta_F_x = srcPos.x - dRotPosX;
	double delta_F_y = srcPos.y - dRotPosY;

 	double delta_X = delta.x;
 	double delta_Y = delta.y;


	double dst_posX = dRotPosX + delta_X + (cos(dTheta)*delta_F_x - sin(dTheta) * delta_F_y);
	double dst_posY = dRotPosY + delta_Y + (sin(dTheta)*delta_F_x + cos(dTheta) * delta_F_y);

	dstPos.x = dst_posX;
	dstPos.y = dst_posY;

	return 0;
}
// SHKang 2018/07/12
int CMCoordi::CorrectCoordinate_both(int nFiduGroupId, MCOORDI_B pos, LP_MCOORDI_B lpPos, double dInversion, BOOL bTheta)
{
	int nFiduModuleId = nFiduGroupId - 1;
	lpPos->x = pos.x;
	lpPos->y = pos.y;
	if (m_bRstPCBStopPos)
	{
		lpPos->x = pos.x + m_dRstPCBStopPos;
		return 0;
	}
	if(nFiduGroupId == 0)
	{ 
		if(m_pBoardFiduDelta.IsEmpty())
			return -1;
	}
	else
	{
		if(m_ppModuleFiduDelta == NULL || m_ppModuleFiduDelta == nullptr || nFiduModuleId < 0 ||
			m_ppModuleFiduDelta[nFiduModuleId] == NULL || m_ppModuleFiduDelta[nFiduModuleId] == nullptr)
			return -1;
	}
	if(m_bNewCoordiAlgo != false)
	{
		double deltaX, deltaY, theta, fiduCenter, scaleX, scaleY;
		if(nFiduGroupId == 0)
			CorrectCoordinate_Cal3(lpPos, &m_boardCoordiCorrector, dInversion, bTheta);
		else 
			CorrectCoordinate_Cal3(lpPos, &m_pModuleCoordiCorrector[nFiduModuleId], dInversion, bTheta);
		return 0;
	}
	int nFiduCnt = 2;
	const int nFiduTotalCnt = 4;
	POINTF poSrc[nFiduTotalCnt];
	POINTF poDelta[nFiduTotalCnt];
	POINTF poFidu[nFiduTotalCnt];
	double dTheta = 0.;
	if(nFiduGroupId == 0)
	{
		nFiduCnt = m_pStdBoardFiduPosTeach.GetCount();
		int nFiduCntBuf = m_pStdBoardFiduPosOrg.GetCount();
		if(nFiduCntBuf != nFiduCnt && nFiduCnt > nFiduCntBuf)
			nFiduCnt = nFiduCntBuf;
		nFiduCntBuf = m_pBoardFiduDelta.GetCount();
		if(nFiduCntBuf != nFiduCnt && nFiduCnt > nFiduCntBuf)
			nFiduCnt = nFiduCntBuf;
		if(nFiduCnt < 2) return -1;
		dTheta = m_boardCoordiCorrector.theta * dInversion * (PI / 180.0);
		for(int a = 0; a < nFiduCnt; a++)
		{
			poSrc[a].x = m_pStdBoardFiduPosTeach[a].x;
			poSrc[a].y = m_pStdBoardFiduPosTeach[a].y;
			poFidu[a].x = m_pStdBoardFiduPosOrg[a].x;
			poFidu[a].y = m_pStdBoardFiduPosOrg[a].y;
			poDelta[a].x = m_pBoardFiduDelta[a].x * dInversion; 
			poDelta[a].y = m_pBoardFiduDelta[a].y * dInversion; // 검사된 피듀셜 dx, dy
		}
	}
	else
	{
		nFiduCnt = _msize(m_ppModuleFiduPosTeach[nFiduModuleId]) / sizeof(m_ppModuleFiduPosTeach[nFiduModuleId][0]);
		int nFiduCntBuf = _msize(m_ppModuleFiduPosOrg[nFiduModuleId]) / sizeof(m_ppModuleFiduPosOrg[nFiduModuleId][0]);
		if(nFiduCntBuf != nFiduCnt && nFiduCnt > nFiduCntBuf)
			nFiduCnt = nFiduCntBuf;
		nFiduCntBuf = _msize(m_ppModuleFiduDelta[nFiduModuleId]) / sizeof(m_ppModuleFiduDelta[nFiduModuleId][0]);
		if(nFiduCntBuf != nFiduCnt && nFiduCnt > nFiduCntBuf)
			nFiduCnt = nFiduCntBuf;
		for(int a = 0; a < nFiduCnt; a++)
		{
			if((m_ppModuleFiduPosTeach[nFiduModuleId][a].x == 0 && m_ppModuleFiduPosTeach[nFiduModuleId][a].y == 0) ||
			   (m_ppModuleFiduPosOrg[nFiduModuleId][a].x == 0 && m_ppModuleFiduPosOrg[nFiduModuleId][a].y == 0))
			{
				nFiduCnt = a;
				break;
			}
		}
		if(nFiduCnt < 2) return -1;
		dTheta = m_pModuleCoordiCorrector[nFiduModuleId].theta * dInversion * (PI / 180.0);
		for(int a = 0; a < nFiduCnt; a++)
		{
			poSrc[a].x = m_ppModuleFiduPosTeach[nFiduModuleId][a].x;
			poSrc[a].y = m_ppModuleFiduPosTeach[nFiduModuleId][a].y;
			poFidu[a].x = m_ppModuleFiduPosOrg[nFiduModuleId][a].x;
			poFidu[a].y = m_ppModuleFiduPosOrg[nFiduModuleId][a].y;
			poDelta[a].x = m_ppModuleFiduDelta[nFiduModuleId][a].x * dInversion; 
			poDelta[a].y = m_ppModuleFiduDelta[nFiduModuleId][a].y * dInversion; // 검사된 피듀셜 dx, dy
		}
	}
	POINTF poDst;
	poDst.x = pos.x;
	poDst.y = pos.y;
	if(CorrectCoordinate_Cal(nFiduCnt, poSrc, poDelta, dTheta, &poDst, poFidu, NULL, dInversion) == 1)
	{
		lpPos->x = poDst.x;
		lpPos->y = poDst.y;
	}
	return 0;
}

int CMCoordi::CorrectCoordinate_Cal(int nFiduNum, POINTF* poSrc, POINTF* poDelta, double dTheta, POINTF* poDst, POINTF* poFidu, POINTF* poData, double dInversion, int nMode)
{
	int nFiduCnt = nFiduNum;
	if(nFiduCnt < 2) return -1;
	if(nFiduCnt >= 4) nFiduCnt = 3;
	POINT_64F fiduPos[4];
	POINT_64F delta[4];
	POINT_64F srcPos(poDst->x, poDst->y);
	POINT_64F srcFovCenter(poDst->x, poDst->y);
	POINT_64F dstPos[4];
	POINT_64F fiduPos_Org[4];
	int nArrIdx[4] = {0,1,2,3};
	if(nFiduNum == 4)
	{
		GetCorrectIDX(nFiduNum, poSrc, *poDst, nArrIdx, 1);
	}
	float fSumX = 0.0f;
	float fSumY = 0.0f;
	for(int a = 0; a < nFiduNum; a++)
	{
		fSumX += poFidu[nArrIdx[a]].x;
		fSumY += poFidu[nArrIdx[a]].y;
	}
	srcFovCenter.x = fSumX / nFiduNum;
	srcFovCenter.y = fSumY / nFiduNum;
	std::vector<cv::Point2f> inputQuad(nFiduCnt);
	std::vector<cv::Point2f> outputQuad(nFiduCnt);
	for(int a = 0; a < nFiduNum; a++)
	{
		fiduPos[a].x = poSrc[nArrIdx[a]].x;
		fiduPos[a].y = poSrc[nArrIdx[a]].y;
		delta[a].x = poDelta[nArrIdx[a]].x;
		delta[a].y = poDelta[nArrIdx[a]].y;
		fiduPos_Org[a].x = poFidu[nArrIdx[a]].x;
		fiduPos_Org[a].y = poFidu[nArrIdx[a]].y;
		CorrectCoordinate_fromfidu(a, fiduPos[a], delta[a], dTheta, srcPos, dstPos[a]);
		dstPos[a].x += ((fiduPos[a].x - fiduPos_Org[a].x) * dInversion);
		dstPos[a].y += ((fiduPos[a].y - fiduPos_Org[a].y) * dInversion);
		if(a < nFiduCnt)
		{
			inputQuad[a] = cv::Point2f(fiduPos[a].x, fiduPos[a].y);
			outputQuad[a] = cv::Point2f(dstPos[a].x, dstPos[a].y);
		}
	}
	if(nFiduCnt == 2)
	{
		// SHKang 2019/01/11 : L_X, L_Y 가 0 일때 예외처리. 기울기 alpha value 초기화 함.
		double L_X = fiduPos[1].x - fiduPos[0].x;
		double L_Y = fiduPos[1].y - fiduPos[0].y;
		double alpha_x1 = 0.5;
		double alpha_x2 = 0.5;
		double alpha_y1 = 0.5;
		double alpha_y2 = 0.5;
		if(L_X != 0)
		{
			alpha_x1 = 1. -  fabs((srcPos.x - fiduPos[0].x) / L_X);
			alpha_x2 = 1. -  fabs((srcPos.x - fiduPos[1].x) / L_X);	// dstPos[0] 으로 넣는게 맞는지 생각중.... 
		}
		if(L_Y != 0)
		{
			alpha_y1 = 1. -  fabs((srcPos.y - fiduPos[0].y) / L_Y); 
			alpha_y2 = 1. -  fabs((srcPos.y - fiduPos[1].y) / L_Y); // dstPos[0] 으로 넣는게 맞는지 생각중....  
		}
		if(alpha_x1<0.) { alpha_x1 = 0.; alpha_x2 = 1.; }
		if(alpha_x2<0.) { alpha_x2 = 0; alpha_x1 = 1.; }
		if(alpha_y1<0.) { alpha_y1 = 0.; alpha_y2 = 1.; }
		if(alpha_y2<0.) { alpha_y1 = 1.; alpha_y2 = 0.; }
		poDst->x = dstPos[0].x * alpha_x1 + dstPos[1].x * alpha_x2;
		poDst->y = dstPos[0].y * alpha_y1 + dstPos[1].y * alpha_y2;
	}
	else
	{
		cv::Mat Lamda;
		for (int a = 0; a < nFiduCnt; a++)
		{
			inputQuad[a] = cv::Point2f(fiduPos[a].x, fiduPos[a].y);
			outputQuad[a] = cv::Point2f(dstPos[a].x, dstPos[a].y);
		}
		if(nFiduCnt == 3)
			Lamda = cv::getAffineTransform(inputQuad, outputQuad);
		else
			Lamda = cv::getPerspectiveTransform(inputQuad, outputQuad);
		if(Lamda.empty() == true)
			return -1;
		cv::Mat inputQuad2(1, 1, CV_32FC2);
		cv::Mat outputQuad2(1, 1, CV_32FC2);
		inputQuad2.at<cv::Point2f>(0) = cv::Point2f(srcPos.x, srcPos.y);
		if(nFiduCnt == 3)
			cv::transform(inputQuad2, outputQuad2, Lamda);
		else
			cv::perspectiveTransform(inputQuad2, outputQuad2, Lamda);
		poDst->x = outputQuad2.at<cv::Point2f>(0).x;
		poDst->y = outputQuad2.at<cv::Point2f>(0).y;
	}
	return 1;
}
int CMCoordi::GetCorrectIDX(int nFiduCnt, POINTF* poSrc, POINTF poDst, int* nArrIDX, int nType)
{
	if(nFiduCnt <= 2) return 0;
	double dArrDist[4];
	double dMin = 0, dMax = 0;
	int nMinIdx = -1, nMaxIdx = -1;
	for (int a = 0; a < nFiduCnt; a++)
	{
		nArrIDX[a] = -1;
		double dX = poDst.x - poSrc[a].x;
		double dY = poDst.y - poSrc[a].y;
		dArrDist[a] = 0;
		if (dX != 0 && dY != 0)
			dArrDist[a] = sqrt(pow(dX, 2) + pow(dY, 2));
		if (nMinIdx == -1 || (nMinIdx > -1 && dMin > dArrDist[a]))
		{
			dMin = dArrDist[a];
			nMinIdx = a;
		}
		if (nMaxIdx == -1 || (nMaxIdx > -1 && dArrDist[a] > dMax))
		{
			dMax = dArrDist[a];
			nMaxIdx = a;
		}
	}
	int nTotalCnt = nFiduCnt - 2;
	if (nMinIdx == nMaxIdx || nType == 1)
		nMaxIdx = -1;
	else if (nMinIdx == nMaxIdx || nType == 2)
		nMinIdx = -1;
	if(nType == 2)
	{
		if (nMaxIdx >= 0)
			nArrIDX[0] = nMaxIdx;
		else
			nTotalCnt = nFiduCnt;
	}
	else
	{
		if (nMinIdx >= 0)
		{
			nArrIDX[0] = nMinIdx;
			if (nMaxIdx >= 0)
				nArrIDX[1] = nMaxIdx;
			else
				nTotalCnt = nFiduCnt - 1;
		}
		else
			nTotalCnt = nFiduCnt;
	}
	int nStartIdx = nFiduCnt - nTotalCnt;
	for (int a = nStartIdx; a < nStartIdx + nTotalCnt; a++)
	{
		dMin = 0;
		nMinIdx = -1;
		for (int b = 0; b < nFiduCnt; b++)
		{
			bool bCheck = true;
			for (int c = 0; c < nFiduCnt; c++)
			{
				if (nArrIDX[c] == b)
				{
					bCheck = false;
					break;
				}
			}
			if (bCheck == false)
				continue;
			if (nType != 2 && (nMinIdx == -1 || (nMinIdx > -1 && dMin > dArrDist[b])))
			{
				dMin = dArrDist[b];
				nMinIdx = b;
			}
			else if (nType == 2 && (nMinIdx == -1 || (nMinIdx > -1 && dMin < dArrDist[b])))
			{
				dMin = dArrDist[b];
				nMinIdx = b;
			}
		}
		if(nMinIdx == -1)
			continue;
		nArrIDX[a] = nMinIdx;
	}
	for (int a = 0; a < nFiduCnt; a++)
	{
		bool bReset = true;
		for (int b = 0; b < nFiduCnt; b++)
		{
			if (nArrIDX[b] == a)
			{
				bReset = false;
				break;
			}
		}
		if (bReset)
		{
			for (int b = 0; b < nFiduCnt; b++)
				nArrIDX[b] = b;
			break;
		}
	}
	return 1;
}
int CMCoordi::CorrectCoordinate_FOV(POINT_64F FiduPos, POINT_64F delta, double dTheta, POINT_64F srcCenterPos, POINT_64F & dstPos) 
{
	double cosTheta = cos(dTheta * (PI / 180.0));	// degree to radian
	double sinTheta = sin(dTheta * (PI / 180.0));

	double mX = FiduPos.x - srcCenterPos.x;
	double mY = FiduPos.y - srcCenterPos.y;

	// correct rotate theta
	double rX = mX * cosTheta - mY * sinTheta;
	double rY = mX * sinTheta + mY * cosTheta;

	// correct shift delta
	dstPos.x = rX + delta.x + srcCenterPos.x;
	dstPos.y = rY + delta.y + srcCenterPos.y;
	return 0;
}
void CMCoordi::CalcDelta(int nFiduCnt, POINTF* poArrFidu, POINTF* poDst, double dTheta, POINTF* poDelta)
{
	float fFiduSumX = 0.0f;
	float fFiduSumY = 0.0f;
	for (int a = 0; a < nFiduCnt; a++)
	{
		fFiduSumX += poArrFidu[a].x;
		fFiduSumY += poArrFidu[a].y;
		poDelta[a].x = 0;
		poDelta[a].y = 0;
	}
	float fFiduCX = fFiduSumX / nFiduCnt;
	float fFiduCY = fFiduSumY / nFiduCnt;
	POINT_64F srcFovCenter(fFiduCX, fFiduCY);
	POINT_64F delta(0, 0);
	for (int a = 0; a < nFiduCnt; a++)
	{
		POINT_64F srcPos(poDst[a].x, poDst[a].y);
		POINT_64F dstPos(0, 0);
		CorrectCoordinate_FOV(srcPos, delta, dTheta, srcFovCenter, dstPos);
		poDelta[a].x = poArrFidu[a].x - dstPos.x;
		poDelta[a].y = poArrFidu[a].y - dstPos.y;
	}
}
double CMCoordi::CalcTheta(POINTF poPos_T1, POINTF poPos_T2, POINTF poPos_R1, POINTF poPos_R2)
{
	bool bHorizon = true;
	double dHorizonX = poPos_T1.x - poPos_T2.x;
	double dHorizonY = poPos_T1.y - poPos_T2.y;
	if (dHorizonX < 0) dHorizonX *= -1;
	if (dHorizonY < 0) dHorizonY *= -1;
	if (dHorizonY > dHorizonX) bHorizon = false;

	double dA1 = 0, dB1 = 0, dA2 = 0, dB2 = 0;
	double dGradient1 = GetGradient(bHorizon, poPos_T1, poPos_T2, &dA1, &dB1);
	double dGradient2 = GetGradient(bHorizon, poPos_R1, poPos_R2, &dA2, &dB2);

	//double dCX = (poAB_2.Y - poAB_1.Y) / (poAB_1.X - poAB_2.X);
	//double dCY = poAB_1.X * dCX + poAB_1.Y;
	//dTheta = Math.Atan((insp_y2 - dCY) / (insp_x2 - dCX)) - Math.Atan((y2 - dCY) / (x2 - dCX));
	double dTheta = (dGradient1 - dGradient2) * -1.;
	return dTheta;
}
double CMCoordi::GetGradient(bool bHorizon, POINTF poPos1, POINTF poPos2, double *dA, double *dB)
{
	double x1 = poPos1.x;
	double y1 = poPos1.y;
	double x2 = poPos2.x;
	double y2 = poPos2.y;
	if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
		return 0;

	double dSigmaXY = 0;
	double dSigmaXSquare = 0;

	double dSigmaX = 0;
	double dSigmaY = 0;
	for(int a = 0; a < 2; a++)
	{
		double dXPos = x1;
		double dYPos = y1;
		if (a == 1)
		{
			dXPos = x2;
			dYPos = y2;
		}
		if (bHorizon == true)
		{
			dSigmaXY = dSigmaXY + (dXPos * dYPos);
			dSigmaXSquare = dSigmaXSquare + (dXPos * dXPos);
			dSigmaX = dSigmaX + dXPos;
			dSigmaY = dSigmaY + dYPos;
		}
		else
		{
			dSigmaXY = dSigmaXY + (dXPos * dYPos);
			dSigmaXSquare = dSigmaXSquare + (dYPos * dYPos);
			dSigmaX = dSigmaX + dYPos;
			dSigmaY = dSigmaY + dXPos;
		}
	}
	double dValue = (dSigmaX * dSigmaX) - (dSigmaXSquare * 2);
	double dValue1 = (dSigmaX * dSigmaXY) - (dSigmaXSquare * dSigmaY);
	if (dValue != 0 && dValue1 != 0)
		*dB = (float)(dValue1 / dValue);
	else
		*dB = 0;
	dValue = dSigmaXSquare;
	dValue1 = dSigmaXY - (dSigmaX * *dB);
	if (dValue != 0 && dValue1 != 0)
		*dA = dValue1 / dValue;
	else
		*dA = 0;
	return atan(*dA) * 180 / PI;
}
double CMCoordi::CalcDistence(POINTF SrcPos, POINTF DstPos)
{
	double dX = DstPos.x - SrcPos.x;
	double dY = DstPos.y - SrcPos.y;
	double dDist = sqrt(pow(dX, 2) + pow(dY, 2));
	return dDist;
}
// ################################################################################################################################
int CMCoordi::CorrectCoordinate_Cal3(MCOORDI_B* poDst, MPTICoordinateCorrector * lpCorrector, double dInversion, BOOL bTheta)
{
	if(lpCorrector->IsGenerate() == false)
		return 1;
	double inf = std::numeric_limits<double>::infinity();
	float fDeltaX = (float)lpCorrector->deltaX;
	float fDeltaY = (float)lpCorrector->deltaY;
	if (_isnan(fDeltaX) || _isnan(fDeltaY) || fDeltaX == inf || fDeltaY == inf)
		return 1;
	MCOORDI_B poFiduCenter;
	poFiduCenter.x = lpCorrector->offsetX;
	poFiduCenter.y = lpCorrector->offsetY;

	double dThetaRadian = lpCorrector->theta * (PI / 180.0);	// degree to radian
	if (bTheta == FALSE)
		dThetaRadian = 0;
	return CorrectCoordinate_Cal3(poDst, lpCorrector->deltaX, lpCorrector->deltaY, dThetaRadian, lpCorrector->scaleX, lpCorrector->scaleY, poFiduCenter, dInversion);
}
int CMCoordi::CorrectCoordinate_Cal3(MCOORDI_B* poDst, double dDeltaX, double dDeltaY, double dThetaRadian, double dScaleX, double dScaleY, MCOORDI_B poFiduCenter, double dInversion)
{   
	dThetaRadian *= dInversion;
	dDeltaX *= dInversion;
	dDeltaY *= dInversion;

	//x, y = (오리지널 파트 좌표 - 오리지널 피듀셜의 무게중심) * scale
	//x' = cos(Theta) * x + sin(Theta) * y + delta_x * scale_x
	//y' = -sin(Theta) * x + cos(Theta) * y + delta_y * scale_y
	//x', y' = x', y' + 오리지널 피듀셜의 무게중심

	if(dInversion > 0)
	{
		double dSrcX = (poDst->x - poFiduCenter.x) * dScaleX;
		double dSrcY = (poDst->y - poFiduCenter.y) * dScaleY;
		poDst->x = cos(dThetaRadian) * dSrcX + sin(dThetaRadian) * dSrcY + dDeltaX * dScaleX;
		poDst->y = -sin(dThetaRadian) * dSrcX + cos(dThetaRadian) * dSrcY + dDeltaY * dScaleY;
	}
	else
	{
		dDeltaX *= dScaleX;
		dDeltaY *= dScaleY;
		dScaleX = 1 / dScaleX;
		dScaleY = 1 / dScaleY;
		double dSrcX = poDst->x - poFiduCenter.x + dDeltaX;
		double dSrcY = poDst->y - poFiduCenter.y + dDeltaY;
		poDst->x = cos(dThetaRadian) * dSrcX + sin(dThetaRadian) * dSrcY;
		poDst->y = -sin(dThetaRadian) * dSrcX + cos(dThetaRadian) * dSrcY;
		poDst->x *= dScaleX;
		poDst->y *= dScaleY;
	}
	poDst->x += poFiduCenter.x; 
	poDst->y += poFiduCenter.y;

	return 1;
}

int CMCoordi::CalcCorrect_ThetaDeltaScale(int nCnt, MCOORDI_B* poArr_O, MCOORDI_B* poArr_T, double *dTheta_ref, double *dDeltaX_ref, double *dDeltaY_ref, double * dScaleX_ref, double * dScaleY_ref)
{
	// nCnt = Fiducial Count
	// poArr_O = Fiducial Original Position
	// poArr_T = Fiducial Real Position
	double dTheta = 0;
	if(nCnt == 0 || poArr_O == NULL || poArr_T == NULL)
		return dTheta;
	double * dArrX_O = NULL;
	double * dArrY_O = NULL;
	double * dArrX_T = NULL;
	double * dArrY_T = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrX_O, nCnt);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrY_O, nCnt);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrX_T, nCnt);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &dArrY_T, nCnt);

	MCOORDI_B poCenter = GetCenterPos(nCnt, poArr_O);	// 무게 중심 좌표

	// 무게 중심을 원점으로 이동
	for(int n = 0; n < nCnt; n++)
	{
		dArrX_O[n] = poArr_O[n].x - poCenter.x;
		dArrY_O[n] = poArr_O[n].y - poCenter.y;
		dArrX_T[n] = poArr_T[n].x - poCenter.x;
		dArrY_T[n] = poArr_T[n].y - poCenter.y;
	}

	// 1차 Theta 계산
	dTheta = CalTheta(nCnt, dArrX_O, dArrY_O, dArrX_T, dArrY_T);

	// 1차 Delta 계산
	double deltaX = 0;
	double deltaY = 0;
	MCOORDI_B poCenter_T = GetCenterPos(nCnt, poArr_T);	// Fiducial Real Position 무게 중심 좌표
	deltaX = poCenter.x - poCenter_T.x;
	deltaY = poCenter.y - poCenter_T.y;	

	// Delta 값을 더해준다
	// Fiducial Real Positon을 역 Theta 로 돌린다
	double dCos = cos(dTheta);
	double dSin = sin(dTheta);
	for(int n = 0; n < nCnt; n++)
	{		
		dArrX_T[n] += deltaX;
		dArrY_T[n] += deltaY;
		double rotX = dArrX_T[n] * dCos - dArrY_T[n] * dSin;
		double rotY = dArrX_T[n] * dSin + dArrY_T[n] * dCos;
		dArrX_T[n] = rotX;
		dArrY_T[n] = rotY;
	}

	// 1차 Scale 계산
	double dXT_XO = CalSigma(nCnt, dArrX_T, dArrX_O);
	double dYT_YO = CalSigma(nCnt, dArrY_T, dArrY_O);
	double dX2 = CalSigma(nCnt, dArrX_O, 2);
	double dY2 = CalSigma(nCnt, dArrY_O, 2);
	double dScaleX = 1;	// X방향 수팽창율
	double dScaleY = 1;	// Y방향 수팽창율
	if(m_bNotUseScale == FALSE && dXT_XO != 0 && dX2 > 10)	// 너무 짧은 거리일 경우 Scale 계산하지 않음
		dScaleX = dXT_XO / dX2;
	if(m_bNotUseScale == FALSE && dYT_YO != 0 && dY2 > 10)
		dScaleY = dYT_YO / dY2;

	// Fiducial Original Position에 Scale 적용 (2차 Theta 계산을 위해)
	for(int n = 0; n < nCnt; n++)
	{
		dArrX_O[n] *= dScaleX;
		dArrY_O[n] *= dScaleY;

		// Fiducial Real Position은 원점만 이동된 좌표로 원복 (2차를 위해)
		dArrX_T[n] = poArr_T[n].x - poCenter.x;
		dArrY_T[n] = poArr_T[n].y - poCenter.y;
	}

	// 2차 Theta 계산 (Scale 적용된 Original Position과 Real Position)
	dTheta = CalTheta(nCnt, dArrX_O, dArrY_O, dArrX_T, dArrY_T);

	// Delta값을 더해준다
	// Fiducial Real Positon을 역 2차 Theta 로 돌린다
	dCos = cos(dTheta);
	dSin = sin(dTheta);
	for(int n = 0; n < nCnt; n++)
	{
// 		dArrX_T[n] += deltaX;
// 		dArrY_T[n] += deltaY;
		double rotX = dArrX_T[n] * dCos - dArrY_T[n] * dSin;
		double rotY = dArrX_T[n] * dSin + dArrY_T[n] * dCos;
		dArrX_T[n] = rotX;
		dArrY_T[n] = rotY;

		// Fiducial Original Position은 원점만 이동된 좌표로 원복 (2차 Scale 계산을 위해)
		dArrX_O[n] = poArr_O[n].x - poCenter.x;
		dArrY_O[n] = poArr_O[n].y - poCenter.y;
	}

	// 2차 Theta 적용된 측정 피듀셜과 Fiducial Original Position을 이용하여 Scale x, y 계산
	dXT_XO = CalSigma(nCnt, dArrX_T, dArrX_O);
	dYT_YO = CalSigma(nCnt, dArrY_T, dArrY_O);
	if(m_bNotUseScale == FALSE && dXT_XO != 0 && dX2 > 10)	// 너무 짧은 거리일 경우 Scale 계산하지 않음
		dScaleX = dXT_XO / dX2;	// X방향 수팽창율
	if(m_bNotUseScale == FALSE && dYT_YO != 0 && dY2 > 10)
		dScaleY = dYT_YO / dY2;	// Y방향 수팽창율

	// Fiducial Original Position에 Scale 적용 (2차 Delta 계산을 위해)
	for(int n = 0; n < nCnt; n++)
	{
		dArrX_O[n] *= dScaleX;
		dArrY_O[n] *= dScaleY;

		// Fiducial Real Position은 원점만 이동된 좌표로 원복 (2차 Delta 계산을 위해)
		dArrX_T[n] = poArr_T[n].x - poCenter.x;
		dArrY_T[n] = poArr_T[n].y - poCenter.y;
	}

	// 2차 Delta 계산
	poCenter.x = CalSigma(nCnt, dArrX_O) / nCnt;	// Scale 적용된 Fiducial Original Position에 무게 중심 좌표
	poCenter.y = CalSigma(nCnt, dArrY_O) / nCnt;
	poCenter_T.x = CalSigma(nCnt, dArrX_T) / nCnt;	// 2차 Theta 적용된 Fiducial Real Position의 무게 중심 좌표
	poCenter_T.y = CalSigma(nCnt, dArrY_T) / nCnt;
	deltaX = poCenter.x - poCenter_T.x;
	deltaY = poCenter.y - poCenter_T.y;

	Delete_1DArray(&dArrX_O);
	Delete_1DArray(&dArrY_O);
	Delete_1DArray(&dArrX_T);
	Delete_1DArray(&dArrY_T);

	*dTheta_ref = dTheta;
	*dDeltaX_ref = -deltaX;
	*dDeltaY_ref = -deltaY;
	*dScaleX_ref = dScaleX;
	*dScaleY_ref = dScaleY;

	return 1;
}

// SHW 2019/04/11
MCOORDI_B CMCoordi::GetCenterPos(int nCnt, MCOORDI_B* poArrPos)
{
	MCOORDI_B poCenter;
	poCenter.x = 0;
	poCenter.y = 0;
	if(poArrPos == NULL || nCnt == 0)
		return poCenter;
	for (int a = 0; a < nCnt; a++)
	{
		poCenter.x += poArrPos[a].x;
		poCenter.y += poArrPos[a].y;
	}
	poCenter.x /= (double)nCnt;
	poCenter.y /= (double)nCnt;
	return poCenter;
}

double CMCoordi::CalSigma(int nCnt, double* fArrX, int nPow/* = 1*/)
{
	double dSum = 0;
	for(int n = 0; n < nCnt; n++)
	{
		dSum += pow(fArrX[n], nPow);
	}
	return dSum;
}

double CMCoordi::CalSigma(int nCnt, double* fArrX, double* fArrY)
{
	double dSum = 0;
	for(int n = 0; n < nCnt; n++)
	{
		dSum += fArrX[n] * fArrY[n];
	}
	return dSum;
}

double CMCoordi::CalTheta(int nCnt, double * dArrX_O, double * dArrY_O, double * dArrX_T, double * dArrY_T)
{
	double dTheta = 0;
	double dXO_YT = CalSigma(nCnt, dArrX_T, dArrY_O);
	double dXT_YO= CalSigma(nCnt, dArrX_O, dArrY_T);
	double dXO_XT = CalSigma(nCnt, dArrX_O, dArrX_T);
	double dYO_YT = CalSigma(nCnt, dArrY_O, dArrY_T);
	if ((dXO_XT + dYO_YT) == 0)
		return 0.0;
	dTheta = atan((dXO_YT - dXT_YO) / (dXO_XT + dYO_YT));

	return dTheta;
}
int CMCoordi::GetCorrectJobPosOff(MPTICoordinateCorrector sData, MPTICoordinateBoard SrcPos, MPTICoordinateBoard *pResPos, double dInversion)
{
	if(pResPos == NULL) return -1;
	pResPos->x = SrcPos.x;
	pResPos->y = SrcPos.y;
	CorrectCoordinate_Cal3(pResPos, &sData, dInversion);
	return 0;
};
// SHKang 2018/07/12
int CMCoordi::Get_CorrectJobPos(int nFiduGroupId, MPTICoordinateBoard SrcPos, MPTICoordinateBoard *pResPos, BOOL bTheta)
{
	if(nFiduGroupId < 0 || pResPos == NULL) return -1;
	return CorrectCoordinate_both(nFiduGroupId, SrcPos, pResPos, 1.0, bTheta);
};

int CMCoordi::Get_InverseJobPos(int nFiduGroupId, MPTICoordinateBoard SrcPos, MPTICoordinateBoard *pResPos)
{
	if(nFiduGroupId < 0 || pResPos == NULL) return -1;
	return CorrectCoordinate_both(nFiduGroupId, SrcPos, pResPos, -1.);	
};

int CMCoordi::CorrectBoardCoordinate(MPTICoordinateBoard *pPos, MPTICoordinateBoard *pResPos)
{
	if((pPos == NULL) || (pResPos == NULL)) return -1;

	int nRet = 0;
	int nCoordiAlgo = m_nCoordiAlgo;
	
	if(nCoordiAlgo == 3) // SHKang 2018/07/31
	{	
		int nFiduGroupId = 0;
		nRet = CorrectCoordinate_both(nFiduGroupId, *pPos, pResPos);
	}
	else
	{
		nRet = CorrectCoordinate(*pPos, m_boardCoordiCorrector, pResPos);
	}
	return nRet;
}

// SHKang 2018/07/31
int CMCoordi::ReverseCorrectBoardCoordinate(MPTICoordinateBoard *pPos, MPTICoordinateBoard *pResPos)
{
	if((pPos == NULL) || (pResPos == NULL)) return -1;

	int nCoordiAlgo = m_nCoordiAlgo;
	int nRet = 0;
	if(nCoordiAlgo == 3)
	{
		int nFiduGroupId = 0;
		nRet = CorrectCoordinate_both(nFiduGroupId, *pPos, pResPos, -1.);
	}
	else // if(nCoordiAlgo==2)
	{
		ReverseCorrectCoordinate(*pPos, m_boardCoordiCorrector, pResPos);
	}
	return nRet;
}


int CMCoordi::CorrectModuleCoordinate(int idxModule, MPTICoordinateBoard *pPos, MPTICoordinateBoard *pResPos)
{
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;
	if((pPos == NULL) || (pResPos == NULL)) return -1;

	int nRet = 0;
	int nCoordiAlgo = m_nCoordiAlgo;
	if(nCoordiAlgo == 3) // SHKang 2018/07/31
	{
		int nFiduGroupId = idxModule;
		nRet = CorrectCoordinate_both(nFiduGroupId, *pPos, pResPos);
	}
	else
	{
		nRet = CorrectCoordinate(*pPos, m_pModuleCoordiCorrector[idxModule], pResPos);
	}
	return nRet;
}


int CMCoordi::CorrectPartoordinate(int idxModule, int idxPart, MPTICoordinateBoard *pPos, MPTICoordinateBoard *pResPos)
{
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;
	if(idxPart <= m_partNum) return -1;
	if((pPos == NULL) || (pResPos == NULL)) return -1;

	return CorrectCoordinate(*pPos, m_pPartCoordiCorrector[idxModule][idxPart], pResPos);
}


bool CMCoordi::GetFiduInspCheck()
{
	if (m_boardCoordiCorrector.m_bGenerate)
		return true;

	return false;
}


int CMCoordi::GetBoardCoordiCorrector(MPTICoordinateCorrector *pCorrector)
{
	if(pCorrector == NULL) return -1;

// 	CopyMemory(pCorrector, &m_boardCoordiCorrector, sizeof(MPTICoordinateCorrector));
	pCorrector->stdTheta = m_boardCoordiCorrector.stdTheta;
	pCorrector->curTheta = m_boardCoordiCorrector.curTheta;
	pCorrector->theta = m_boardCoordiCorrector.theta;
	pCorrector->offsetX = m_boardCoordiCorrector.offsetX;
	pCorrector->offsetY = m_boardCoordiCorrector.offsetY;
	pCorrector->deltaX = m_boardCoordiCorrector.deltaX;
	pCorrector->deltaY = m_boardCoordiCorrector.deltaY;;
	pCorrector->scaleX = m_boardCoordiCorrector.scaleX;
	pCorrector->scaleY = m_boardCoordiCorrector.scaleY;
	pCorrector->scaleX = m_boardCoordiCorrector.scaleX;
	pCorrector->scaleY = m_boardCoordiCorrector.scaleY;
	pCorrector->m_bGenerate = m_boardCoordiCorrector.m_bGenerate;

	return 0;
}


int CMCoordi::GetModuleCoordiCorrector(int idxModule, MPTICoordinateCorrector *pCorrector)
{
	if(pCorrector == NULL) return -1;
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;

	// 	CopyMemory(pCorrector, &m_pModuleCoordiCorrector[idxModule], sizeof(MPTICoordinateCorrector));
	pCorrector->stdTheta = m_pModuleCoordiCorrector[idxModule].stdTheta;
	pCorrector->curTheta = m_pModuleCoordiCorrector[idxModule].curTheta;
	pCorrector->theta = m_pModuleCoordiCorrector[idxModule].theta;
	pCorrector->offsetX = m_pModuleCoordiCorrector[idxModule].offsetX;
	pCorrector->offsetY = m_pModuleCoordiCorrector[idxModule].offsetY;
	pCorrector->deltaX = m_pModuleCoordiCorrector[idxModule].deltaX;
	pCorrector->deltaY = m_pModuleCoordiCorrector[idxModule].deltaY;
	pCorrector->scaleX = m_pModuleCoordiCorrector[idxModule].scaleX;
	pCorrector->scaleY = m_pModuleCoordiCorrector[idxModule].scaleY;
	pCorrector->m_bGenerate = m_pModuleCoordiCorrector[idxModule].m_bGenerate;

	return 0;
}


int CMCoordi::GetPartCoordiCorrector(int idxModule, int idxPart, MPTICoordinateCorrector *pCorrector)
{
	if(pCorrector == NULL) return -1;
	if((idxModule < 0) || (idxModule >= m_moduleNum)) return -1;
	if((idxPart < 0) || (idxPart >= m_partNum)) return -1;

	CopyMemory(pCorrector, &m_pPartCoordiCorrector[idxModule][idxPart], sizeof(MPTICoordinateCorrector));

	return 0;
}


int CMCoordi::SetFovPixelNumber(int numX, int numY)
{
	m_fovPixelNumX = numX;
	m_fovPixelNumY = numY;

	return 0;
}


int CMCoordi::SetFovPixelResolution(double resolX, double resolY)
{
	m_fovPixelResolX = resolX;
	m_fovPixelResolY = resolY;

	m_fovSizeX = m_fovPixelNumX * m_fovPixelResolX;
	m_fovSizeY = m_fovPixelNumY * m_fovPixelResolY;

	return 0;
}


int CMCoordi::SetFovSize(double sizeX, double sizeY)
{
	m_fovSizeX = sizeX;
	m_fovSizeX = sizeY;

	m_fovPixelResolX = m_fovSizeX / (double)m_fovPixelNumX;
	m_fovPixelResolY = m_fovSizeY / (double)m_fovPixelNumY;

	return 0;
}


int CMCoordi::GetFovSize(double *sizeX, double *sizeY)
{
	*sizeX = m_fovSizeX;
	*sizeY = m_fovSizeY;

	return 0;
}


int CMCoordi::GetFovPixelNumber(int *numX, int *numY)
{
	*numX = m_fovPixelNumX;
	*numY = m_fovPixelNumY;

	return 0;
}


int CMCoordi::GetFovPixelResolution(double *resolX, double *resolY)
{
	*resolX = m_fovPixelResolX;
	*resolY = m_fovPixelResolY;

	return 0;
}


 int CMCoordi::BoardToMachine(MPTICoordinateBoard *pBoard, MPTICoordinateMachine *pMachine, int nLaneNum)
 {
//  	if (g_pMPTI->m_pMIO == NULL)
//  		return 0;
//  
//  	bool bState = false;
//  
//  	if(nLaneNum == eMCRI_F)
//  	{
//  		g_pMPTI->m_pMIO->GetInPcbOut1(&bState);		// Bigjob2StepPosition check
//  		if(bState == false)
//  			g_pMPTI->m_pMIO->GetInPcbOut2(&bState);
//  	}
//  	else if(nLaneNum == eMCRI_R)
//  	{
//  		g_pMPTI->m_pMIO->GetInPcbOut1_Slave(&bState);		// Bigjob2StepPosition check
//  		if(bState == false)
//  			g_pMPTI->m_pMIO->GetInPcbOut2_Slave(&bState);
//  	}
//  
//  
//  // 	pMachine->x = m_boardAnchorPos.x + (pBoard->x * m_dirBoardX * m_dirMachineX);
//  // 	pMachine->y = m_boardAnchorPos.y + (pBoard->y * m_dirBoardY * m_dirMachineY);
//  
//  	if(bState == true)
//  	{
//  		if(m_nMachineFlowType == (int)eMMF_LR)
//  		{
//  			pMachine->x = (m_boardAnchorPos.x - m_dDistanceOfOut1) + (pBoard->x * m_dirBoardX * m_dirMachineX);
//  			pMachine->y = m_boardAnchorPos.y + (pBoard->y * m_dirBoardY * m_dirMachineY);
//  		}
//  		else
//  		{
//  			pMachine->x = (m_boardAnchorPos.x + m_dDistanceOfOut1) + (pBoard->x * m_dirBoardX * m_dirMachineX);
//  			pMachine->y = m_boardAnchorPos.y + (pBoard->y * m_dirBoardY * m_dirMachineY);
//  		}
//  	}
//  	else
//  	{
//  		pMachine->x = m_boardAnchorPos.x + (pBoard->x * m_dirBoardX * m_dirMachineX);
//  		pMachine->y = m_boardAnchorPos.y + (pBoard->y * m_dirBoardY * m_dirMachineY);
//  	}
//  
//  	double dBufX = pMachine->x;
//  	double dBufY = pMachine->y;
//  	CalcObjToRef( dBufX, dBufY, &pMachine->x, &pMachine->y, nLaneNum);
//  			
 	// by. sangmin msgbox
 //  	CString strMsg = L"";
 //  	strMsg.Format( L"Ref(%.03f, %.03f)-> Obj(%.03f, %.03f)", dBufX, dBufY, pMachine->x, pMachine->y);
 //  	AfxMessageBox( strMsg);
 
 	return 0;
 }
 
 
 int CMCoordi::MachineToBoard(MPTICoordinateMachine *pMachine, MPTICoordinateBoard *pBoard, int nLaneNum)
 {
//  	if (g_pMPTI->m_pMIO == NULL)
//  		return 0;
//  
//  	bool bState = false;
//  
//  	if(nLaneNum == eMCRI_F)
//  	{
//  		g_pMPTI->m_pMIO->GetInPcbOut1(&bState);		// Bigjob2StepPosition check
//  		if(bState == false)
//  			g_pMPTI->m_pMIO->GetInPcbOut2(&bState);
//  	}
//  	else if(nLaneNum == eMCRI_R)
//  	{
//  		g_pMPTI->m_pMIO->GetInPcbOut1_Slave(&bState);		// Bigjob2StepPosition check
//  		if(bState == false)
//  			g_pMPTI->m_pMIO->GetInPcbOut2_Slave(&bState);
//  	}
//  
//  	double dBufX = pMachine->x;
//  	double dBufY = pMachine->y;
//  	
//  	CalcRefToObj( dBufX, dBufY, &pMachine->x, &pMachine->y, nLaneNum);
//  	
//  	// by. sangmin msgbox
//  // 	CString strMsg = L"";
//  // 	strMsg.Format( L"Ref (%.03f, %.03f)-> Obj(%.03f, %.03f)", dBufX, dBufY, pMachine->x, pMachine->y);
//  // 	AfxMessageBox( strMsg);
//  
//  // 	pBoard->x = ((pMachine->x - m_boardAnchorPos.x) * m_dirBoardX * m_dirMachineX);
//  // 	pBoard->y = ((pMachine->y - m_boardAnchorPos.y) * m_dirBoardY * m_dirMachineY);
//  
//  	if(bState == true)
//  	{
//  		if(m_nMachineFlowType == (int)eMMF_LR)
//  		{
//  			pBoard->x = ((pMachine->x - (m_boardAnchorPos.x - m_dDistanceOfOut1)) * m_dirBoardX * m_dirMachineX);
//  			pBoard->y = ((pMachine->y - m_boardAnchorPos.y) * m_dirBoardY * m_dirMachineY);
//  		}
//  		else
//  		{
//  			pBoard->x = ((pMachine->x - (m_boardAnchorPos.x + m_dDistanceOfOut1)) * m_dirBoardX * m_dirMachineX);
//  			pBoard->y = ((pMachine->y - m_boardAnchorPos.y) * m_dirBoardY * m_dirMachineY);
//  		}
//  	}
//  	else
//  	{
//  		pBoard->x = ((pMachine->x - m_boardAnchorPos.x) * m_dirBoardX * m_dirMachineX);
//  		pBoard->y = ((pMachine->y - m_boardAnchorPos.y) * m_dirBoardY * m_dirMachineY);
//  	}
 	
 	return 0;
 }


int CMCoordi::UnitBoardToMachine(double inX, double inY, double *outX, double *outY)
{
	*outX = inX * m_dirBoardX * m_dirMachineX;
	*outY = inY * m_dirBoardY * m_dirMachineY;

	return 0;
}


int CMCoordi::UnitMachineToBoard(double inX, double inY, double *outX, double *outY)
{
	*outX = inX * m_dirBoardX * m_dirMachineX;
	*outY = inY * m_dirBoardY * m_dirMachineY;

	return 0;
}


int CMCoordi::UnitScreenToBoard(double inX, double inY, double *outX, double *outY)
{
	*outX = inX * m_dirBoardX * m_dirMachineX;
	*outY = inY * m_dirBoardY * m_dirMachineY;

	return 0;
}


int CMCoordi::UnitPixelToMillimeter(double inX, double inY, double *outX, double *outY)
{
	*outX = inX * m_fovPixelResolX;
	*outY = inY * m_fovPixelResolY;

	return 0;
}

int CMCoordi::ReLoadMatrixCalibrationFile(int nLaneNum)
{
	if(nLaneNum == 0)
	{
		if(m_calib_ref) Delete_2DArray(&m_calib_ref, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
		if(m_calib_obj) Delete_2DArray(&m_calib_obj, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);

		/*LoadMatrixCalibrationFile( L"C:\\Eagle3D_64x\\MOTION\\calib_o.dat", 0);
		LoadMatrixCalibrationFile( L"C:\\Eagle3D_64x\\MOTION\\calib_r.dat", 1);*/


// 		LoadMatrixCalibrationFile(TEXT("C:\\") + g_pMPTI->m_ProcessName + TEXT("_64x\\MOTION\\calib_r_aoi.dat"), 0, nLaneNum);
// 		LoadMatrixCalibrationFile(TEXT("C:\\") + g_pMPTI->m_ProcessName + TEXT("_64x\\MOTION\\calib_o_aoi.dat"), 1, nLaneNum);
	}
	else if(nLaneNum == 1)
	{
		if(m_calib_Rear_ref) Delete_2DArray(&m_calib_Rear_ref, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
		if(m_calib_Rear_obj) Delete_2DArray(&m_calib_Rear_obj, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
				
// 		LoadMatrixCalibrationFile(TEXT("C:\\") + g_pMPTI->m_ProcessName + TEXT("_64x\\MOTION\\calib_Rear_r_aoi.dat"), 0, nLaneNum);
// 		LoadMatrixCalibrationFile(TEXT("C:\\") + g_pMPTI->m_ProcessName + TEXT("_64x\\MOTION\\calib_Rear_o_aoi.dat"), 1, nLaneNum);
	}
	

	return 0;
}
// 
int CMCoordi::InitMatrixCalibration(int nLaneNum)
{
	if (nLaneNum == 0)
	{
		if (m_calib_ref) Delete_2DArray(&m_calib_ref, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
		if (m_calib_obj) Delete_2DArray(&m_calib_obj, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
	}
	else if (nLaneNum == 1)
	{
		if (m_calib_Rear_ref) Delete_2DArray(&m_calib_Rear_ref, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
		if (m_calib_Rear_obj) Delete_2DArray(&m_calib_Rear_obj, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
	}

	return 0;
}

float CMCoordi::Interpolate(float pt_x, float pt_y, float *src, int row, int col, int inc=1)
{
	int minX = 0;
	int minY = 0;

	float **ppArrsrc;
	Make_2DArray(&ppArrsrc, row * 2,col * 2);
	int nArrIdx = 0;

	for(int nj = 0; nj < row * 2; nj++)
	{
		for(int ni = 0; ni < col * 2; ni++)
		{
			ppArrsrc[nj][ni] = src[nArrIdx];

			nArrIdx++;
		}
	}


	//	if(pt_x > 0.) 
	minX = (int)floor( (pt_x) ) - (inc -1);
	//	if(pt_y  > 0.) 
	minY = (int)floor( (pt_y) ) - (inc -1);

	if(minX<0) 
		minX = 0;
	else if(minX >= col-1) 
		minX = col-2;

	if(minY<0) 
		minY = 0;
	else if(minY >= row-1) 
		minY = row-2;


	//	if(pt_x > 1)
	//		minX = (int)floor( pt_x ) - 1; // ¼Ò¼öÁ¡ÀÌÇÏ °ªÀ» Á¦°ÅÇÑ ÁÂÇ¥
	//	else
	//		minX = (int)floor( pt_x ); // ¼Ò¼öÁ¡ÀÌÇÏ °ªÀ» Á¦°ÅÇÑ ÁÂÇ¥
	//	
	//	if(pt_y > 1)
	//		minY = (int)floor( pt_y ) - 1;
	//	else
	//		minY = (int)floor( pt_y );

	int maxX = minX+inc; // ¼Ò¼öÁ¡ ÁÂÇ¥¸¦ ¿Ã¸²ÇÑ ÁÂÇ¥°ª
	int maxY = minY+inc;

	float ** fInterpolation;
	//fInterpolation = new float*[4];
	fInterpolation = g_pMManager->pem_new<float*>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	int i;
	for(i=0; i<4; i++)
	{
		//fInterpolation[i] = new float[4];
		fInterpolation[i] = g_pMManager->pem_new<float>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	}

	float **h;
	Make_2DArray(&h, 4,1);

	float ResultData;	

	fInterpolation[0][0] = 1;
	fInterpolation[0][1] = static_cast<float>(minX);
	fInterpolation[0][2] = static_cast<float>(minY);
	fInterpolation[0][3] = static_cast<float>(minX * minY);

	fInterpolation[1][0] = 1;
	fInterpolation[1][1] = static_cast<float>(minX);
	fInterpolation[1][2] = static_cast<float>(maxY);
	fInterpolation[1][3] = static_cast<float>(minX * maxY);

	fInterpolation[2][0] = 1;
	fInterpolation[2][1] = static_cast<float>(maxX);
	fInterpolation[2][2] = static_cast<float>(maxY);
	fInterpolation[2][3] = static_cast<float>(maxX * maxY);

	fInterpolation[3][0] = 1;
	fInterpolation[3][1] = static_cast<float>(maxX);
	fInterpolation[3][2] = static_cast<float>(minY);
	fInterpolation[3][3] = static_cast<float>(maxX * minY);

// 	h[0][0] = src[minY][minX];
// 	h[1][0] = src[maxY][minX];
// 	h[2][0] = src[maxY][maxX];
// 	h[3][0] = src[minY][maxX];

	h[0][0] = ppArrsrc[minY][minX];
	h[1][0] = ppArrsrc[maxY][minX];
	h[2][0] = ppArrsrc[maxY][maxX];
	h[3][0] = ppArrsrc[minY][maxX];

	gaussj(fInterpolation, h);
	//	Gaussj(fInterpolation, 4, h, 1);

	ResultData = h[0][0] + h[1][0] * pt_x + h[2][0] * pt_y + h[3][0] * pt_x * pt_y;

	for(i = 0; i < 4; i++)
	{
		//delete [] (fInterpolation[i]);
		g_pMManager->pem_delete(fInterpolation[i], true);
	}
	//delete [] fInterpolation;
	g_pMManager->pem_delete(fInterpolation, true);
	Delete_2DArray(&h, 4,1);
	Delete_2DArray(&ppArrsrc, row * 2, col * 2);
	return ResultData;
}

void CMCoordi::LoadMatrixCalibrationFile( CString fileName, int mode, int nLaneNum )
{
	if( _waccess(fileName, 0 ) == -1 )
		return;

	CReadWriteFile strFile;
	strFile.Open(fileName);
	register int nCount = strFile.GetCount();
	register int i = 0;
	CString strTemp = L"";
	char cTemp;

	int strLength=0;

	for(i=0; i < nCount; i++)
	{
 		if(strFile[i])
 		{
 			strTemp = strFile[i];
 			cTemp = strTemp.GetAt(0);
 			if(cTemp == '@')
 			{
 				if(strTemp == "@HEADER")
 				{
 					m_eCalibCode = MC_CALIB_HEADER;
 				}
 				else if(strTemp == "@BODY X")
 				{
 					m_eCalibCode = MC_CALIB_BODY_X;
 				}
 				else if(strTemp == "@BODY Y")
 				{
 					m_eCalibCode = MC_CALIB_BODY_Y;
 				}
 			}
 			else
 			{
 				if(m_eCalibCode == MC_CALIB_HEADER)
 				{
 					Set_MC_CalibHeader(strFile[i], mode, nLaneNum);
 				}
 				else if(m_eCalibCode == MC_CALIB_BODY_X)
 				{
 					Set_Mc_Calib_BodyX(strFile[i], i, mode, nLaneNum);
 				}
 				else if(m_eCalibCode == MC_CALIB_BODY_Y)
 				{
 					Set_Mc_Calib_BodyY(strFile[i], i, mode, nLaneNum);
 				}
 			}
 		}
	}

	if(nLaneNum == 0)
	{
		if(mode == 0)
		{
			// 		m_fRef_offsetX = m_calib_ref[1][1].x;
			// 		m_fRef_offsetY = m_calib_ref[1][1].y;
			if(m_calib_ref)
			{
				if(m_calib_ref[1][1].x >= 0)
					m_fRef_offsetX[nLaneNum] = m_calib_ref[1][1].x;
				else
					m_fRef_offsetX[nLaneNum] = m_calib_ref[1][1].x -1.;

				if(m_calib_ref[1][1].y >= 0)
					m_fRef_offsetY[nLaneNum] = m_calib_ref[1][1].y;
				else
					m_fRef_offsetY[nLaneNum] = m_calib_ref[1][1].y -1.;
			}
		}
		else
		{
			// 		m_fObj_offsetX = m_calib_obj[1][1].x;
			// 		m_fObj_offsetY = m_calib_obj[1][1].y;
			if(m_calib_obj[1][1].x >= 0)
				m_fObj_offsetX[nLaneNum] = m_calib_obj[1][1].x;
			else
				m_fObj_offsetX[nLaneNum] = m_calib_obj[1][1].x -1.;

			if(m_calib_obj[1][1].y >= 0)
				m_fObj_offsetY[nLaneNum] = m_calib_obj[1][1].y;
			else
				m_fObj_offsetY[nLaneNum] = m_calib_obj[1][1].y -1.;
		}
	}
	else if(nLaneNum == 1)
	{
		if(mode == 0)
		{
			if(m_calib_Rear_ref)
			{
				if(m_calib_Rear_ref[1][1].x >= 0)
					m_fRef_offsetX[nLaneNum] = m_calib_Rear_ref[1][1].x;
				else
					m_fRef_offsetX[nLaneNum] = m_calib_Rear_ref[1][1].x -1.;

				if(m_calib_Rear_ref[1][1].y >= 0)
					m_fRef_offsetY[nLaneNum] = m_calib_Rear_ref[1][1].y;
				else
					m_fRef_offsetY[nLaneNum] = m_calib_Rear_ref[1][1].y -1.;
			}
		}
		else
		{
			if(m_calib_Rear_obj[1][1].x >= 0)
				m_fObj_offsetX[nLaneNum] = m_calib_Rear_obj[1][1].x;
			else
				m_fObj_offsetX[nLaneNum] = m_calib_Rear_obj[1][1].x -1.;

			if(m_calib_Rear_obj[1][1].y >= 0)
				m_fObj_offsetY[nLaneNum] = m_calib_Rear_obj[1][1].y;
			else
				m_fObj_offsetY[nLaneNum] = m_calib_Rear_obj[1][1].y -1.;
		}
	}
}

void CMCoordi::Set_MC_CalibHeader( char * strData, int mode, int nLaneNum )
{
	if(strData==NULL) return;
	char seps[] = " =\n";

	CString type;
	CString data;

	type = strtok(strData, seps);
	data = strtok( NULL, seps );

	CString strQuery1, strQuery2;
	if(type == "X")
	{
		m_calibHeader[nLaneNum].col = _wtoi(data);
	}
	else if(type == "Y")
	{
		m_calibHeader[nLaneNum].row = _wtoi(data);

		if(nLaneNum == 0)
		{
			if(mode==0)
				Make_2DArray(&m_calib_ref, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
			else if(mode==1)
				Make_2DArray(&m_calib_obj, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
		}
		else if(nLaneNum == 1)
		{
			if(mode==0)
				Make_2DArray(&m_calib_Rear_ref, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
			else if(mode==1)
				Make_2DArray(&m_calib_Rear_obj, m_calibHeader[nLaneNum].row, m_calibHeader[nLaneNum].col);
		}
		
	}
}

void CMCoordi::Set_Mc_Calib_BodyX( char * strData, int i, int mode, int nLaneNum )
{
	if(strData==NULL) return;
	char seps[] = " \t";

	CStringA data = strtok(strData, seps);
	int m = i-4;
	int n = 0;

	if(nLaneNum == 0)
	{
		if(mode == 0)
		{
			while(data != "")
			{
				m_calib_ref[m][n].x = atof(data);
				data = strtok(NULL, seps);
				n ++;
			}
		}
		else if(mode == 1)
		{
			while(data != "")
			{
				m_calib_obj[m][n].x = atof(data);
				data = strtok(NULL, seps);
				n ++;
			}
		}
	}
	else if(nLaneNum == 1)
	{
		if(mode == 0)
		{
			while(data != "")
			{
				m_calib_Rear_ref[m][n].x = atof(data);
				data = strtok(NULL, seps);
				n ++;
			}
		}
		else if(mode == 1)
		{
			while(data != "")
			{
				m_calib_Rear_obj[m][n].x = atof(data);
				data = strtok(NULL, seps);
				n ++;
			}
		}
	}
	
}

void CMCoordi::Set_Mc_Calib_BodyY( char * strData, int i, int mode, int nLaneNum )
{
	if(strData==NULL) return;
	char seps[] = " \t";

	CStringA data = strtok(strData, seps);
	int m = i-(5 + m_calibHeader[nLaneNum].row);
	int n = 0;

	if(nLaneNum == 0)
	{
		if(mode == 0)
		{
			while(data != "")
			{
				m_calib_ref[m][n].y = atof(data);
				data = strtok(NULL, seps);
				n ++;
			}
		}
		else if(mode == 1)
		{
			while(data != "")
			{
				m_calib_obj[m][n].y = atof(data);
				data = strtok(NULL, seps);
				n ++;
			}
		}
	}
	else if(nLaneNum == 1)
	{
		if(mode == 0)
		{
			while(data != "")
			{
				m_calib_Rear_ref[m][n].y = atof(data);
				data = strtok(NULL, seps);
				n ++;
			}
		}
		else if(mode == 1)
		{
			while(data != "")
			{
				m_calib_Rear_obj[m][n].y = atof(data);
				data = strtok(NULL, seps);
				n ++;
			}
		}
	}
	
}

void CMCoordi::Set_Mc_Calib_Clear_BodyX(char * strData, int i, int mode, int nLaneNum)
{
	if (strData == NULL) return;
	char seps[] = " \t";

	CStringA data = strtok(strData, seps);
	int m = i - 4;
	int n = 0;

	if (nLaneNum == 0)
	{
		if (mode == 0)
		{
			while (data != "")
			{
				m_calib_ref[m][n].x = 0;
				data = strtok(NULL, seps);
				n++;
			}
		}
		else if (mode == 1)
		{
			while (data != "")
			{
				m_calib_obj[m][n].x = 0;
				data = strtok(NULL, seps);
				n++;
			}
		}
	}
	else if (nLaneNum == 1)
	{
		if (mode == 0)
		{
			while (data != "")
			{
				m_calib_Rear_ref[m][n].x = 0;
				data = strtok(NULL, seps);
				n++;
			}
		}
		else if (mode == 1)
		{
			while (data != "")
			{
				m_calib_Rear_obj[m][n].x = 0;
				data = strtok(NULL, seps);
				n++;
			}
		}
	}

}
void CMCoordi::Set_Mc_Calib_Clear_BodyY(char * strData, int i, int mode, int nLaneNum)
{
	if (strData == NULL) return;
	char seps[] = " \t";

	CStringA data = strtok(strData, seps);
	int m = i - (5 + m_calibHeader[nLaneNum].row);
	int n = 0;

	if (nLaneNum == 0)
	{
		if (mode == 0)
		{
			while (data != "")
			{
				m_calib_ref[m][n].y = 0;
				data = strtok(NULL, seps);
				n++;
			}
		}
		else if (mode == 1)
		{
			while (data != "")
			{
				m_calib_obj[m][n].y = 0;
				data = strtok(NULL, seps);
				n++;
			}
		}
	}
	else if (nLaneNum == 1)
	{
		if (mode == 0)
		{
			while (data != "")
			{
				m_calib_Rear_ref[m][n].y = 0;
				data = strtok(NULL, seps);
				n++;
			}
		}
		else if (mode == 1)
		{
			while (data != "")
			{
				m_calib_Rear_obj[m][n].y = 0;
				data = strtok(NULL, seps);
				n++;
			}
		}
	}
}
int CMCoordi::CalcObjToRef( double srcX, double srcY, double* dstX, double* dstY, int nLaneNum )
{
	if(nLaneNum == 0)
	{
		if( m_calib_obj == NULL || m_calib_ref == NULL )
		{
			*dstX = srcX;
			*dstY = srcY;
			return -1;
		}
		int minX = 0;
		int minY = 0;

		double intervalX = m_calib_ref[1][2].x - m_calib_ref[1][1].x;	// Cal 데이터 X 간격 계산
		double intervalY = m_calib_ref[2][1].y - m_calib_ref[1][1].y;	// Cal 데이터 Y 간격 계산

		if(srcX - m_fRef_offsetX[nLaneNum] > 0.) 
			minX = (int)floor( (srcX - m_fRef_offsetX[nLaneNum]) / intervalX) + 1;
		if(srcY - m_fRef_offsetY[nLaneNum] > 0.) 
			minY = (int)floor( (srcY - m_fRef_offsetY[nLaneNum]) / intervalY) + 1;

		if(minX<0) 
			minX = 0;
		else if(minX >= m_calibHeader[nLaneNum].col-1) 
			minX = m_calibHeader[nLaneNum].col-2;

		if(minY<0) 
			minY = 0;
		else if(minY >= m_calibHeader[nLaneNum].row-1) 
			minY = m_calibHeader[nLaneNum].row-2;

		int maxX = minX+1;
		int maxY = minY+1;

		float **fInterpolation_x;
		float **fInterpolation_y;
		float **h_x;
		float **h_y;

		Make_2DArray(&fInterpolation_x, 4,4);
		Make_2DArray(&fInterpolation_y, 4,4);
		Make_2DArray(&h_x, 4,1);
		Make_2DArray(&h_y, 4,1);

		fInterpolation_x[0][0] = fInterpolation_y[0][0] = 1;
		fInterpolation_x[0][1] = fInterpolation_y[0][1] = (float)m_calib_ref[minY][minX].x;
		fInterpolation_x[0][2] = fInterpolation_y[0][2] = (float)m_calib_ref[minY][minX].y;
		fInterpolation_x[0][3] = fInterpolation_y[0][3] = (float)( m_calib_ref[minY][minX].x * m_calib_ref[minY][minX].y );

		fInterpolation_x[1][0] = fInterpolation_y[1][0] = 1;
		fInterpolation_x[1][1] = fInterpolation_y[1][1] = (float)m_calib_ref[maxY][minX].x;
		fInterpolation_x[1][2] = fInterpolation_y[1][2] = (float)m_calib_ref[maxY][minX].y;
		fInterpolation_x[1][3] = fInterpolation_y[1][3] = (float)( m_calib_ref[maxY][minX].x * m_calib_ref[maxY][minX].y );

		fInterpolation_x[2][0] = fInterpolation_y[2][0] = 1;
		fInterpolation_x[2][1] = fInterpolation_y[2][1] = (float)m_calib_ref[maxY][maxX].x;
		fInterpolation_x[2][2] = fInterpolation_y[2][2] = (float)m_calib_ref[maxY][maxX].y;
		fInterpolation_x[2][3] = fInterpolation_y[2][3] = (float)( m_calib_ref[maxY][maxX].x * m_calib_ref[maxY][maxX].y );

		fInterpolation_x[3][0] = fInterpolation_y[3][0] = 1;
		fInterpolation_x[3][1] = fInterpolation_y[3][1] = (float)m_calib_ref[minY][maxX].x;
		fInterpolation_x[3][2] = fInterpolation_y[3][2] = (float)m_calib_ref[minY][maxX].y;
		fInterpolation_x[3][3] = fInterpolation_y[3][3] = (float)( m_calib_ref[minY][maxX].x * m_calib_ref[minY][maxX].y );

		h_x[0][0] = (float)m_calib_obj[minY][minX].x;
		h_x[1][0] = (float)m_calib_obj[maxY][minX].x;
		h_x[2][0] = (float)m_calib_obj[maxY][maxX].x;
		h_x[3][0] = (float)m_calib_obj[minY][maxX].x;

		h_y[0][0] = (float)m_calib_obj[minY][minX].y;
		h_y[1][0] = (float)m_calib_obj[maxY][minX].y;
		h_y[2][0] = (float)m_calib_obj[maxY][maxX].y;
		h_y[3][0] = (float)m_calib_obj[minY][maxX].y;

		gaussj(fInterpolation_x, h_x);
		gaussj(fInterpolation_y, h_y);

		*dstX = h_x[0][0] + h_x[1][0] * srcX + h_x[2][0] * srcY + h_x[3][0] * srcX * srcY;
		*dstY = h_y[0][0] + h_y[1][0] * srcX + h_y[2][0] * srcY + h_y[3][0] * srcX * srcY;

		Delete_2DArray(&fInterpolation_x, 4,4);
		Delete_2DArray(&fInterpolation_y, 4,4);
		Delete_2DArray(&h_x, 4,1);
		Delete_2DArray(&h_y, 4,1);

		return 0;
	}
	else if(nLaneNum == 1)
	{
		if( m_calib_Rear_obj == NULL || m_calib_Rear_ref == NULL )
		{
			*dstX = srcX;
			*dstY = srcY;
			return -1;
		}
		int minX = 0;
		int minY = 0;

		double intervalX = m_calib_Rear_ref[1][2].x - m_calib_Rear_ref[1][1].x;	// Cal 데이터 X 간격 계산
		double intervalY = m_calib_Rear_ref[2][1].y - m_calib_Rear_ref[1][1].y;	// Cal 데이터 Y 간격 계산

		if(srcX - m_fRef_offsetX[nLaneNum] > 0.) 
			minX = (int)floor( (srcX - m_fRef_offsetX[nLaneNum]) / intervalX) + 1;
		if(srcY - m_fRef_offsetY[nLaneNum] > 0.) 
			minY = (int)floor( (srcY - m_fRef_offsetY[nLaneNum]) / intervalY) + 1;

		if(minX<0) 
			minX = 0;
		else if(minX >= m_calibHeader[nLaneNum].col-1) 
			minX = m_calibHeader[nLaneNum].col-2;

		if(minY<0) 
			minY = 0;
		else if(minY >= m_calibHeader[nLaneNum].row-1) 
			minY = m_calibHeader[nLaneNum].row-2;

		int maxX = minX+1;
		int maxY = minY+1;

		float **fInterpolation_x;
		float **fInterpolation_y;
		float **h_x;
		float **h_y;

		Make_2DArray(&fInterpolation_x, 4,4);
		Make_2DArray(&fInterpolation_y, 4,4);
		Make_2DArray(&h_x, 4,1);
		Make_2DArray(&h_y, 4,1);

		fInterpolation_x[0][0] = fInterpolation_y[0][0] = 1;
		fInterpolation_x[0][1] = fInterpolation_y[0][1] = (float)m_calib_Rear_ref[minY][minX].x;
		fInterpolation_x[0][2] = fInterpolation_y[0][2] = (float)m_calib_Rear_ref[minY][minX].y;
		fInterpolation_x[0][3] = fInterpolation_y[0][3] = (float)( m_calib_Rear_ref[minY][minX].x * m_calib_Rear_ref[minY][minX].y );

		fInterpolation_x[1][0] = fInterpolation_y[1][0] = 1;
		fInterpolation_x[1][1] = fInterpolation_y[1][1] = (float)m_calib_Rear_ref[maxY][minX].x;
		fInterpolation_x[1][2] = fInterpolation_y[1][2] = (float)m_calib_Rear_ref[maxY][minX].y;
		fInterpolation_x[1][3] = fInterpolation_y[1][3] = (float)( m_calib_Rear_ref[maxY][minX].x * m_calib_Rear_ref[maxY][minX].y );

		fInterpolation_x[2][0] = fInterpolation_y[2][0] = 1;
		fInterpolation_x[2][1] = fInterpolation_y[2][1] = (float)m_calib_Rear_ref[maxY][maxX].x;
		fInterpolation_x[2][2] = fInterpolation_y[2][2] = (float)m_calib_Rear_ref[maxY][maxX].y;
		fInterpolation_x[2][3] = fInterpolation_y[2][3] = (float)( m_calib_Rear_ref[maxY][maxX].x * m_calib_Rear_ref[maxY][maxX].y );

		fInterpolation_x[3][0] = fInterpolation_y[3][0] = 1;
		fInterpolation_x[3][1] = fInterpolation_y[3][1] = (float)m_calib_Rear_ref[minY][maxX].x;
		fInterpolation_x[3][2] = fInterpolation_y[3][2] = (float)m_calib_Rear_ref[minY][maxX].y;
		fInterpolation_x[3][3] = fInterpolation_y[3][3] = (float)( m_calib_Rear_ref[minY][maxX].x * m_calib_Rear_ref[minY][maxX].y );

		h_x[0][0] = (float)m_calib_Rear_obj[minY][minX].x;
		h_x[1][0] = (float)m_calib_Rear_obj[maxY][minX].x;
		h_x[2][0] = (float)m_calib_Rear_obj[maxY][maxX].x;
		h_x[3][0] = (float)m_calib_Rear_obj[minY][maxX].x;

		h_y[0][0] = (float)m_calib_Rear_obj[minY][minX].y;
		h_y[1][0] = (float)m_calib_Rear_obj[maxY][minX].y;
		h_y[2][0] = (float)m_calib_Rear_obj[maxY][maxX].y;
		h_y[3][0] = (float)m_calib_Rear_obj[minY][maxX].y;

		gaussj(fInterpolation_x, h_x);
		gaussj(fInterpolation_y, h_y);

		*dstX = h_x[0][0] + h_x[1][0] * srcX + h_x[2][0] * srcY + h_x[3][0] * srcX * srcY;
		*dstY = h_y[0][0] + h_y[1][0] * srcX + h_y[2][0] * srcY + h_y[3][0] * srcX * srcY;

		Delete_2DArray(&fInterpolation_x, 4,4);
		Delete_2DArray(&fInterpolation_y, 4,4);
		Delete_2DArray(&h_x, 4,1);
		Delete_2DArray(&h_y, 4,1);

		return 0;
	}

	
}

int CMCoordi::CalcRefToObj( double srcX, double srcY, double* dstX, double* dstY, int nLaneNum )
{
	if(nLaneNum == 0)
	{
		if( m_calib_obj == NULL || m_calib_ref == NULL )
		{
			*dstX = srcX;
			*dstY = srcY;
			return -1;
		}

		int minX = 0;
		int minY = 0;

		double intervalX = m_calib_ref[1][2].x - m_calib_ref[1][1].x;	// Cal 데이터 X 간격 계산
		double intervalY = m_calib_ref[2][1].y - m_calib_ref[1][1].y;	// Cal 데이터 Y 간격 계산

		if(srcX - m_fObj_offsetX[nLaneNum] > 0.) 
			minX = (int)floor((srcX - m_fObj_offsetX[nLaneNum]) / intervalX) + 1;
		if(srcY - m_fObj_offsetY[nLaneNum] > 0.) 
			minY = (int)floor( (srcY - m_fObj_offsetY[nLaneNum]) / intervalY) + 1;

		if(minX<0) 
			minX = 0;
		else if(minX >= m_calibHeader[nLaneNum].col-1) 
			minX = m_calibHeader[nLaneNum].col-2;

		if(minY<0) 
			minY = 0;
		else if(minY >= m_calibHeader[nLaneNum].row-1) 
			minY = m_calibHeader[nLaneNum].row-2;

		int maxX = minX+1;
		int maxY = minY+1;

		float **fInterpolation_x;
		float **fInterpolation_y;
		float **h_x;
		float **h_y;

		Make_2DArray(&fInterpolation_x, 4,4);
		Make_2DArray(&fInterpolation_y, 4,4);
		Make_2DArray(&h_x, 4,1);
		Make_2DArray(&h_y, 4,1);

		fInterpolation_x[0][0] = fInterpolation_y[0][0] = 1;
		fInterpolation_x[0][1] = fInterpolation_y[0][1] = (float)m_calib_obj[minY][minX].x;
		fInterpolation_x[0][2] = fInterpolation_y[0][2] = (float)m_calib_obj[minY][minX].y;
		fInterpolation_x[0][3] = fInterpolation_y[0][3] = (float)( m_calib_obj[minY][minX].x * m_calib_obj[minY][minX].y );

		fInterpolation_x[1][0] = fInterpolation_y[1][0] = 1;
		fInterpolation_x[1][1] = fInterpolation_y[1][1] = (float)m_calib_obj[maxY][minX].x;
		fInterpolation_x[1][2] = fInterpolation_y[1][2] = (float)m_calib_obj[maxY][minX].y;
		fInterpolation_x[1][3] = fInterpolation_y[1][3] = (float)( m_calib_obj[maxY][minX].x * m_calib_obj[maxY][minX].y );

		fInterpolation_x[2][0] = fInterpolation_y[2][0] = 1;
		fInterpolation_x[2][1] = fInterpolation_y[2][1] = (float)m_calib_obj[maxY][maxX].x;
		fInterpolation_x[2][2] = fInterpolation_y[2][2] = (float)m_calib_obj[maxY][maxX].y;
		fInterpolation_x[2][3] = fInterpolation_y[2][3] = (float)( m_calib_obj[maxY][maxX].x * m_calib_obj[maxY][maxX].y );

		fInterpolation_x[3][0] = fInterpolation_y[3][0] = 1;
		fInterpolation_x[3][1] = fInterpolation_y[3][1] = (float)m_calib_obj[minY][maxX].x;
		fInterpolation_x[3][2] = fInterpolation_y[3][2] = (float)m_calib_obj[minY][maxX].y;
		fInterpolation_x[3][3] = fInterpolation_y[3][3] = (float)( m_calib_obj[minY][maxX].x * m_calib_obj[minY][maxX].y );

		h_x[0][0] = (float)m_calib_ref[minY][minX].x;
		h_x[1][0] = (float)m_calib_ref[maxY][minX].x;
		h_x[2][0] = (float)m_calib_ref[maxY][maxX].x;
		h_x[3][0] = (float)m_calib_ref[minY][maxX].x;

		h_y[0][0] = (float)m_calib_ref[minY][minX].y;
		h_y[1][0] = (float)m_calib_ref[maxY][minX].y;
		h_y[2][0] = (float)m_calib_ref[maxY][maxX].y;
		h_y[3][0] = (float)m_calib_ref[minY][maxX].y;

		gaussj(fInterpolation_x, h_x);
		gaussj(fInterpolation_y, h_y);

		*dstX = h_x[0][0] + h_x[1][0] * srcX + h_x[2][0] * srcY + h_x[3][0] * srcX * srcY;
		*dstY = h_y[0][0] + h_y[1][0] * srcX + h_y[2][0] * srcY + h_y[3][0] * srcX * srcY;

		Delete_2DArray(&fInterpolation_x, 4,4);
		Delete_2DArray(&fInterpolation_y, 4,4);
		Delete_2DArray(&h_x, 4,1);
		Delete_2DArray(&h_y, 4,1);

		return 0;
	}
	else if(nLaneNum == 1)
	{
		if( m_calib_Rear_obj == NULL || m_calib_Rear_ref == NULL )
		{
			*dstX = srcX;
			*dstY = srcY;
			return -1;
		}

		int minX = 0;
		int minY = 0;

		double intervalX = m_calib_ref[1][2].x - m_calib_ref[1][1].x;	// Cal 데이터 X 간격 계산
		double intervalY = m_calib_ref[2][1].y - m_calib_ref[1][1].y;	// Cal 데이터 Y 간격 계산

		if(srcX - m_fObj_offsetX[nLaneNum] > 0.) 
			minX = (int)floor((srcX - m_fObj_offsetX[nLaneNum]) / intervalX) + 1;
		if(srcY - m_fObj_offsetY[nLaneNum] > 0.) 
			minY = (int)floor( (srcY - m_fObj_offsetY[nLaneNum]) / intervalY) + 1;

		if(minX<0) 
			minX = 0;
		else if(minX >= m_calibHeader[nLaneNum].col-1) 
			minX = m_calibHeader[nLaneNum].col-2;

		if(minY<0) 
			minY = 0;
		else if(minY >= m_calibHeader[nLaneNum].row-1) 
			minY = m_calibHeader[nLaneNum].row-2;

		int maxX = minX+1;
		int maxY = minY+1;

		float **fInterpolation_x;
		float **fInterpolation_y;
		float **h_x;
		float **h_y;

		Make_2DArray(&fInterpolation_x, 4,4);
		Make_2DArray(&fInterpolation_y, 4,4);
		Make_2DArray(&h_x, 4,1);
		Make_2DArray(&h_y, 4,1);

		fInterpolation_x[0][0] = fInterpolation_y[0][0] = 1;
		fInterpolation_x[0][1] = fInterpolation_y[0][1] = (float)m_calib_Rear_obj[minY][minX].x;
		fInterpolation_x[0][2] = fInterpolation_y[0][2] = (float)m_calib_Rear_obj[minY][minX].y;
		fInterpolation_x[0][3] = fInterpolation_y[0][3] = (float)( m_calib_Rear_obj[minY][minX].x * m_calib_Rear_obj[minY][minX].y );

		fInterpolation_x[1][0] = fInterpolation_y[1][0] = 1;
		fInterpolation_x[1][1] = fInterpolation_y[1][1] = (float)m_calib_Rear_obj[maxY][minX].x;
		fInterpolation_x[1][2] = fInterpolation_y[1][2] = (float)m_calib_Rear_obj[maxY][minX].y;
		fInterpolation_x[1][3] = fInterpolation_y[1][3] = (float)( m_calib_Rear_obj[maxY][minX].x * m_calib_Rear_obj[maxY][minX].y );

		fInterpolation_x[2][0] = fInterpolation_y[2][0] = 1;
		fInterpolation_x[2][1] = fInterpolation_y[2][1] = (float)m_calib_Rear_obj[maxY][maxX].x;
		fInterpolation_x[2][2] = fInterpolation_y[2][2] = (float)m_calib_Rear_obj[maxY][maxX].y;
		fInterpolation_x[2][3] = fInterpolation_y[2][3] = (float)( m_calib_Rear_obj[maxY][maxX].x * m_calib_Rear_obj[maxY][maxX].y );

		fInterpolation_x[3][0] = fInterpolation_y[3][0] = 1;
		fInterpolation_x[3][1] = fInterpolation_y[3][1] = (float)m_calib_Rear_obj[minY][maxX].x;
		fInterpolation_x[3][2] = fInterpolation_y[3][2] = (float)m_calib_Rear_obj[minY][maxX].y;
		fInterpolation_x[3][3] = fInterpolation_y[3][3] = (float)( m_calib_Rear_obj[minY][maxX].x * m_calib_Rear_obj[minY][maxX].y );

		h_x[0][0] = (float)m_calib_Rear_ref[minY][minX].x;
		h_x[1][0] = (float)m_calib_Rear_ref[maxY][minX].x;
		h_x[2][0] = (float)m_calib_Rear_ref[maxY][maxX].x;
		h_x[3][0] = (float)m_calib_Rear_ref[minY][maxX].x;

		h_y[0][0] = (float)m_calib_Rear_ref[minY][minX].y;
		h_y[1][0] = (float)m_calib_Rear_ref[maxY][minX].y;
		h_y[2][0] = (float)m_calib_Rear_ref[maxY][maxX].y;
		h_y[3][0] = (float)m_calib_Rear_ref[minY][maxX].y;

		gaussj(fInterpolation_x, h_x);
		gaussj(fInterpolation_y, h_y);

		*dstX = h_x[0][0] + h_x[1][0] * srcX + h_x[2][0] * srcY + h_x[3][0] * srcX * srcY;
		*dstY = h_y[0][0] + h_y[1][0] * srcX + h_y[2][0] * srcY + h_y[3][0] * srcX * srcY;

		Delete_2DArray(&fInterpolation_x, 4,4);
		Delete_2DArray(&fInterpolation_y, 4,4);
		Delete_2DArray(&h_x, 4,1);
		Delete_2DArray(&h_y, 4,1);

		return 0;
	}
	
}

void CMCoordi::GetFiducialInfo(int nFiduGroup, int nFiduCnt, double *nOrgX, double *nOrgY, double *nOffsetedX, double *nOffsetedY/*, double *nOffsetX, double *nOffsetY, double *nDelta*/)
{
	if (nFiduGroup ==  0)
	{
		*nOffsetedX = m_pStdBoardFiduPosTeach[nFiduCnt].x + m_pBoardFiduDelta[nFiduCnt].x;
		*nOffsetedY = m_pStdBoardFiduPosTeach[nFiduCnt].y + m_pBoardFiduDelta[nFiduCnt].y;
		*nOrgX = m_pStdBoardFiduPosOrg[nFiduCnt].x;
		*nOrgY = m_pStdBoardFiduPosOrg[nFiduCnt].y;
		// *nOffsetX = m_pBoardFiduDelta[nFiduCnt].x;
		// *nOffsetY = m_pBoardFiduDelta[nFiduCnt].y;
	}
	else
	{		
		int nID = nFiduGroup - 1;
		*nOffsetedX = m_ppModuleFiduPosTeach[nID][nFiduCnt].x + m_ppModuleFiduDelta[nID][nFiduCnt].x;
		*nOffsetedY = m_ppModuleFiduPosTeach[nID][nFiduCnt].y + m_ppModuleFiduDelta[nID][nFiduCnt].y;
		*nOrgX = m_ppModuleFiduPosOrg[nID][nFiduCnt].x;
		*nOrgY = m_ppModuleFiduPosOrg[nID][nFiduCnt].y;
		// *nOffsetX = m_ppModuleFiduDelta[nFiduGroup][nFiduCnt].x;
		// *nOffsetY = m_ppModuleFiduDelta[nFiduGroup][nFiduCnt].y;
	}
}

// 동일 Y좌표를 가진 2 Board Fiducial의 Scan당시 Theta와 현재 검사된 Theta 더해진 Theta 반환
double CMCoordi::GetTheta_BoardFiduOrg()
{
	double dResTheta = 0.0;
	int nFiduNum = 2;
	GetBoardFiducialNumber(&nFiduNum);
	if (nFiduNum < 2)
		return dResTheta;

	double fidu_x[2], fidu_y[2];
	double org_x[2], org_y[2];
	bool bFind = false;

	for (int a = 0; a < nFiduNum; a++)
	{
		for (int b = 0; b < nFiduNum; b++)
		{
			if (a != b && fabs(m_pStdBoardFiduPosOrg[a].y - m_pStdBoardFiduPosOrg[b].y) < 10)
			{
				fidu_x[0] = org_x[0] = m_pStdBoardFiduPosOrg[a].x;
				fidu_x[1] = org_x[1] = m_pStdBoardFiduPosOrg[b].x;
				fidu_y[0] = org_y[0] = org_y[1] = m_pStdBoardFiduPosOrg[a].y;
				fidu_y[1] = m_pStdBoardFiduPosOrg[b].y;
				bFind = true;
				break;
			}
		}
		if (bFind) break;
	}

	if (!bFind)
		return dResTheta;

	for (int a = 1; a >= 0; a--)
	{
		fidu_x[a] -= org_x[0];
		fidu_y[a] -= org_y[0];
		org_x[a] -= org_x[0];
		org_y[a] -= org_y[0];
	}

	dResTheta = CalTheta(2, org_x, org_y, fidu_x, fidu_y);
	dResTheta = dResTheta * (180.0 / PI);	// radian to degree;
	dResTheta += m_boardCoordiCorrector.theta;

	return dResTheta;
}



  int CMCoordi::CalcCorrector_Pos(MPTICoordinateCorrector *pCorrector, int nPosCnt, double *dArrX, double *dArrY, double *dArrX_2, double *dArrY_2)
  {
  	double stdDistX, stdDistY;
  	double curDistX, curDistY;
  	double stdTheta, curTheta;
  	double deltaTheta;
  
  	double stdPosSx, stdPosSy, stdPosEx, stdPosEy/*, stdPosCx, stdPosCy*/;
  	double curPosSx, curPosSy, curPosEx, curPosEy/*, curPosCx, curPosCy*/;
  	double offsetX = 0, offsetY = 0, deltaX = 0, deltaY = 0, scaleX = 0, scaleY = 0;
  
  	int nIDX = 0;
  	MCOORDI_B poFiduOrg[DEF_FIDU_COUNT];
  	MCOORDI_B poFiduReal[DEF_FIDU_COUNT];
  	for (int a = 0; a < nPosCnt; a++)
  	{
  		poFiduOrg[nIDX].x = dArrX[a];
  		poFiduOrg[nIDX].y = dArrY[a];
  		poFiduReal[nIDX].x = dArrX_2[a];
  		poFiduReal[nIDX].y = dArrY_2[a];
  		offsetX += poFiduOrg[nIDX].x;
  		offsetY += poFiduOrg[nIDX].y;
  		nIDX++;
  	}
  
  	double dScaleX = 1;
  	double dScaleY = 1;
  	CalcCorrect_ThetaDeltaScale(nIDX, poFiduOrg, poFiduReal, &deltaTheta, &deltaX, &deltaY, &dScaleX, &dScaleY);
  
  	stdDistX = poFiduOrg[0].x - poFiduOrg[1].x;
  	stdDistY = poFiduOrg[0].y - poFiduOrg[1].y;
  	stdTheta = (double)atan2(stdDistY, stdDistX);
  
  	curDistX = poFiduReal[0].x - poFiduReal[1].x;
  	curDistY = poFiduReal[0].y - poFiduReal[1].y;
  	curTheta = (double)atan2(curDistY, curDistX);
  
  	pCorrector->stdTheta = stdTheta * (180.0 / PI);	// radian to degree
  	pCorrector->curTheta = curTheta * (180.0 / PI);
  	pCorrector->theta = deltaTheta * (180.0 / PI);
  	pCorrector->offsetX = offsetX / nIDX;
  	pCorrector->offsetY = offsetY / nIDX;
  	pCorrector->deltaX = deltaX;
  	pCorrector->deltaY = deltaY;
  	pCorrector->scaleX = dScaleX;
  	pCorrector->scaleY = dScaleY;
  	pCorrector->SetGenerate(TRUE);
  	return 1;
  }

#pragma region exposure_implementation

int MPTI_SetAnchorMachine(int anchor)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetAnchorMachine(anchor);
}

int MPTI_SetAnchorBoard(int anchor)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetAnchorBoard(anchor);
}

int MPTI_SetAnchorAline(int anchor)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetAnchorAline(anchor);
}

int MPTI_SetBoardAlinePos(MPTICoordinateMachine *pPos)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetBoardAlinePos(pPos);
}

int MPTI_GetBoardAlinePos(MPTICoordinateMachine *pPos)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->GetBoardAlinePos(pPos);
}

int MPTI_SetDistanceOfOut1(double dDistanceX, int nMachineFlowType)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetDistanceOfOut1(dDistanceX, nMachineFlowType);
}

int MPTI_GetBoardSize(double *sizeX, double *sizeY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->GetBoardSize(sizeX, sizeY);
}

int MPTI_GenerateCoordinate()
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->GenerateCoordinate();
}

int MPTI_SetStdBoardFiducialPos(int idxFidu, MPTICoordinateBoard *pPosOrg, MPTICoordinateBoard *pPosTeach)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetStdBoardFiducialPos(idxFidu, pPosOrg, pPosTeach);
}

int MPTI_SetStdModuleFiducialPos(int idxModule, int idxFidu, MPTICoordinateBoard *pPos, MPTICoordinateBoard *pPosTeach)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	// C#에서의 Module Index는 Global을 포함한다.
	return g_pMCoordi->SetStdModuleFiducialPos(idxModule-1, idxFidu, pPos, pPosTeach);
}

int MPTI_DeleteFiducials()
{
	if(g_pMCoordi == NULL) return eMR_FAIL;
	g_pMCoordi->DeleteBoardFiducials();
	/*g_pMCoordi->DeleteSubFiducials();*/		//Never delete Module
	return eMR_SUCCESS;
}

int MPTI_SetBoardFiducialDelta(int idxFidu, MPTICoordinateBoard *pDelta)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetBoardFiducialDelta(idxFidu, pDelta);
}

int MPTI_SetModuleFiducialDelta(int idxModule, int idxFidu, MPTICoordinateBoard *pDelta)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	// C#에서의 Module Index는 Global을 포함한다.
	return g_pMCoordi->SetModuleFiducialDelta(idxModule-1, idxFidu, pDelta);
}

int MPTI_ClearBoardCoordiCorrector()
{
	if(g_pMCoordi == NULL) return eMR_FAIL;
	g_pMCoordi->m_bRstPCBStopPos = false;
	g_pMCoordi->m_dRstPCBStopPos = 0;
	return g_pMCoordi->ClearBoardCoordiCorrector();
}

int MPTI_ClearAllCoordiCorrector()
{
	if(g_pMCoordi == NULL) return eMR_FAIL;
	g_pMCoordi->m_bRstPCBStopPos = false;
	g_pMCoordi->m_dRstPCBStopPos = 0;
	return g_pMCoordi->ClearAllCoordiCorrector();
}

int MPTI_ClearModuleCoordiCorrector(int idxModule)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;
	g_pMCoordi->m_bRstPCBStopPos = false;
	g_pMCoordi->m_dRstPCBStopPos = 0;
	return g_pMCoordi->ClearModuleCoordiCorrector(idxModule-1);
}

int MPTI_GenerateBoardCoordiCorrector()
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->GenerateBoardCoordiCorrector();
}

int MPTI_GenerateModuleCoordiCorrector(int idxModule)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	// C#에서의 Module Index는 Global을 포함한다.
	return g_pMCoordi->GenerateModuleCoordiCorrector(idxModule-1);
}

bool MPTI_GetFiduInspCheck()
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	bool bRtn = g_pMCoordi->GetFiduInspCheck();

	Sleep(10);

	return bRtn;
}

int MPTI_GetBoardCoordiCorrector(MPTICoordinateCorrector *pCorrector)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->GetBoardCoordiCorrector(pCorrector);
}

int MPTI_GetModuleCoordiCorrector(int idxModule, MPTICoordinateCorrector *pCorrector)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	// C#에서의 Module Index는 Global을 포함한다.
	return g_pMCoordi->GetModuleCoordiCorrector(idxModule-1, pCorrector);
}

int MPTI_SetFovPixelNumber(int numX, int numY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetFovPixelNumber(numX, numY);
}

int MPTI_SetFovPixelResolution(double resolX, double resolY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetFovPixelResolution(resolX, resolY);
}

int MPTI_SetFovSize(double sizeX, double sizeY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->SetFovSize(sizeX, sizeY);
}

int MPTI_GetFovPixelNumber(int *numX, int *numY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->GetFovPixelNumber(numX, numY);
}

int MPTI_GetFovPixelResolution(double *resolX, double *resolY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->GetFovPixelResolution(resolX, resolY);
}

int MPTI_GetFovSize(double *sizeX, double *sizeY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->GetFovSize(sizeX, sizeY);
}

int MPTI_BoardToMachine(MPTICoordinateBoard *pBoard, MPTICoordinateMachine *pMachine, int nLaneNum)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->BoardToMachine(pBoard, pMachine, nLaneNum);
}

int MPTI_MachineToBoard(MPTICoordinateMachine *pMachine, MPTICoordinateBoard *pBoard, int nLaneNum)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->MachineToBoard(pMachine, pBoard, nLaneNum);
}

int MPTI_UnitBoardToMachine(double inX, double inY, double *outX, double *outY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->UnitBoardToMachine(inX, inY, outX, outY);
}

int MPTI_UnitMachineToBoard(double inX, double inY, double *outX, double *outY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->UnitMachineToBoard(inX, inY, outX, outY);
}

int MPTI_UnitScreenToBoard(double inX, double inY, double *outX, double *outY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->UnitScreenToBoard(inX, inY, outX, outY);
}

int MPTI_UnitPixelToMillimeter(double inX, double inY, double *outX, double *outY)
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->UnitPixelToMillimeter(inX, inY, outX, outY);
}

int MPTI_CalcRefToObj( double dSrcX, double dSrcY, double* outX, double* outY, int nLaneNum )
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	double dBufX = dSrcX;
	double dBufY = dSrcY;

	int nRtn = g_pMCoordi->CalcRefToObj(dSrcX, dSrcY, outX, outY, nLaneNum);

	// by. sangmin msgbox
// 	CString strMsg = L"";
// 	strMsg.Format( L"Ref (%.03f, %.03f)-> Obj(%.03f, %.03f)", dBufX, dBufY, *outX, *outY);
// 	AfxMessageBox( strMsg);

	return nRtn;
}

int MPTI_CalcObjToRef( double dSrcX, double dSrcY, double* outX, double* outY, int nLaneNum )
{
	if(g_pMCoordi == NULL) return eMR_FAIL;

	double dBufX = dSrcX;
	double dBufY = dSrcY;

	int nRtn = g_pMCoordi->CalcObjToRef(dSrcX, dSrcY, outX, outY, nLaneNum);

	// by. sangmin msgbox
	// 	CString strMsg = L"";
	// 	strMsg.Format( L"Ref (%.03f, %.03f)-> Obj(%.03f, %.03f)", dBufX, dBufY, *outX, *outY);
	// 	AfxMessageBox( strMsg);

	return nRtn;
}

void MPTI_GetFiducialInfo(int nFiduGroup, int nFiduCnt, double *nOrgX, double *nOrgY, double *nOffsetedX, double *nOffsetedY/*, double *nOffsetX, double *nOffsetY, double *nDelta*/)
{
	if(g_pMCoordi == NULL)
		return;

	g_pMCoordi->GetFiducialInfo(nFiduGroup, nFiduCnt, nOrgX, nOrgY, nOffsetedX, nOffsetedY/*, nOffsetX, nOffsetY, nDelta*/);

}

int MPTI_ReLoadMatrixCalibrationFile(int nLaneNum)
{
	int nRtn = g_pMCoordi->ReLoadMatrixCalibrationFile(nLaneNum);

	return nRtn;
}

int MPTI_InitMatrixCalibration(int nLaneNum)
{
	int nRtn = g_pMCoordi->InitMatrixCalibration(nLaneNum);

	return nRtn;
}

float MPTI_Interpolate(float pt_x, float pt_y, float *src, int row, int col, int inc)
{
	float nRtn = g_pMCoordi->Interpolate(pt_x, pt_y, src, row, col, inc);

	return nRtn;
}
// MPTIDLL int MPTI_GetCorrectJobPosOff(MPTICoordinateCorrector sData, MPTICoordinateBoard SrcJobPos, MPTICoordinateBoard *pDstRealPos, double dInversion)
// {
// 	if(pDstRealPos == NULL) return eMR_FAIL;
// 	return g_pMCoordi->GetCorrectJobPosOff(sData, SrcJobPos, pDstRealPos, dInversion);
// }
// SHKang 2018/07/12
// Job position -> real board position
int MPTI_GetRealPosition(int nFiduGroupId, MPTICoordinateBoard SrcJobPos, MPTICoordinateBoard *pDstRealPos, BOOL bTheta)
{
	if(nFiduGroupId < 0 || pDstRealPos == NULL) return eMR_FAIL;

	return g_pMCoordi->Get_CorrectJobPos(nFiduGroupId, SrcJobPos, pDstRealPos, bTheta);
}

// real board position -> job position
int MPTI_GetDataPosition(int nFiduGroupId, MPTICoordinateBoard SrcRealPos, MPTICoordinateBoard *pDstJobPos)
{
	if(nFiduGroupId < 0 || pDstJobPos == NULL) return eMR_FAIL;
	
	return g_pMCoordi->Get_InverseJobPos(nFiduGroupId, SrcRealPos, pDstJobPos);
}

int MPTI_CorrectCoordinate_both(int nFiduCnt, POINTF* poSrc, POINTF* poDelta, double dTheta, POINTF* poDst, POINTF* poCad)
{
	return g_pMCoordi->CorrectCoordinate_Cal(nFiduCnt, poSrc, poDelta, dTheta, poDst, poSrc, poCad);
}


int MPTI_CalcCorrect_ThetaDeltaScale(int nCnt, MCOORDI_B* poArr_O, MCOORDI_B* poArr_T, double *dTheta_ref, double *dDeltaX_ref, double *dDeltaY_ref, double * dScaleX_ref, double * dScaleY_ref)
{
	return g_pMCoordi->CalcCorrect_ThetaDeltaScale(nCnt, poArr_O, poArr_T, dTheta_ref, dDeltaX_ref, dDeltaY_ref, dScaleX_ref, dScaleY_ref);
}

int MPTI_CorrectCoordinate_Cal3(MCOORDI_B* poDst, double dDeltaX, double dDeltaY, double dTheta, double dScaleX, double dScaleY, MCOORDI_B poFiduCenter, double dInversion)
{
	return g_pMCoordi->CorrectCoordinate_Cal3(poDst, dDeltaX, dDeltaY, dTheta, dScaleX, dScaleY, poFiduCenter, dInversion);
}
// MPTIDLL void MPTI_SetPCBStop(BOOL bUse, double dRst)
// {
// 	g_pMCoordi->m_bRstPCBStopPos = bUse;
// 	g_pMCoordi->m_dRstPCBStopPos = (bUse) ? dRst : 0;
// }
// MPTIDLL void MPTI_SetFiduOPT(BOOL bNotUseScale, double dRate)
// {
// 	g_pMCoordi->m_bNotUseScale = bNotUseScale;
// 	g_pMCoordi->m_dInspRate = dRate;
// }
int MPTI_CalcCorrector_Pos(MPTICoordinateCorrector *pCorrector, int nPosCnt, double *dArrX, double *dArrY, double *dArrX_2, double *dArrY_2)
{
	if (g_pMCoordi == NULL) return eMR_FAIL;

	return g_pMCoordi->CalcCorrector_Pos(pCorrector, nPosCnt, dArrX, dArrY, dArrX_2, dArrY_2);
}
#pragma endregion exposure_implementation