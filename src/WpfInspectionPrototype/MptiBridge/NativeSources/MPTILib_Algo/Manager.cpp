#include "StdAfx.h"
#include "Manager.h"
#include "MPTI.h"


CManager::CManager(void)
{
	m_milApp = M_NULL;
	m_milSys = M_NULL;

	m_inspItemCnts = NULL;
	m_inspItemID = NULL;

	m_groupIndexCnts = NULL;
	m_groupIndex = NULL;

	m_groupID = NULL;
	m_groupWndCnts = NULL;
	m_groupWndID = NULL;
}


CManager::~CManager(void)
{

}


int CManager::Init(Im::PIL_ID* milApp, Im::PIL_ID* milSys)
{
	return TRUE;
}
int CManager::Init(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	return TRUE;
}

int CManager::Exit()
{
	return TRUE;
}

void CManager::SetResolution(int fovWidth, int fovLength, double resolX, double resolY)
{

}

void CManager::SetInspItemInfo(int* inspItemCnts,	int** inspItemID, int** inspWndOrder)
{

}

void CManager::SetInspGroupInfo(int* groupIndexCnts, int** groupIndex, int* groupID, int* groupWndCnts, int** groupWndID)
{

}

void CManager::SetPartParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize)
{

}
void CManager::SetPartParam_Foreign(InspPartInfo* boardInfo)
{

}


Coordinate CManager::SetCoordinate(InspPartInfo* boardInfo)
{
	Coordinate ret;
	CPoint rst;
	CPoint rst2;

	rst = CvtBoradToPixel(boardInfo->fovCx, boardInfo->fovCy, boardInfo->partCx, boardInfo->partCy, boardInfo->partCx, boardInfo->partCy , boardInfo);
	ret.cx = rst.x;
	ret.cy = rst.y;

	double dResolX = m_resolX;
	double dResolY = m_resolY;

// 	if(boardInfo->BtmCameraUse == 1)
// 	{
// 		dResolX = g_pMPTI->m_dBtmSideResX;
// 		dResolY = g_pMPTI->m_dBtmSideResY;
// 	}

	ret.width = RounD(boardInfo->partWidth  / dResolX);
	ret.length = RounD(boardInfo->partHeight  / dResolY);
	rst = CvtBoradToPixel(boardInfo->fovCx, boardInfo->fovCy, boardInfo->anyAngleCx, boardInfo->anyAngleCy, boardInfo->anyAngleCx, boardInfo->anyAngleCy, boardInfo);
	ret.anyAngleCx = rst.x;
	ret.anyAngleCy = rst.y;
	ret.anyAngleWidth = RounD(boardInfo->anyAngleWidth  / dResolX);
	ret.anyAngleLength = RounD(boardInfo->anyAngleLength / dResolY);

	ret.fovCx = boardInfo->fovCx;
	ret.fovCy = boardInfo->fovCy;

	ret.angle = boardInfo->angle;

#ifdef _OFFLINE_INSP
	ret.cx = (int)boardInfo->.partCx;
	ret.cy = (int)boardInfo->.partCy;
	ret.width = (int)boardInfo->partWidth;
	ret.length = (int)boardInfo->partHeight;

	ret.anyAngleCx = (int)boardInfo->anyAngleCx;
	ret.anyAngleCy = (int)boardInfo->anyAngleCy;
	ret.anyAngleWidth = (int)boardInfo->anyAngleWidth;
	ret.anyAngleLength = (int)boardInfo->anyAngleLength;

	ret.fovCx = 0;
	ret.fovCy = 0;

	ret.angle = boardInfo->angle;
#endif

	return ret;
}

Coordinate CManager::SetCoordinate(InspPartInfo* boardInfo, InspPartParam param, AlignResult * sAlignRes)
{
	Coordinate ret;
	CPoint rst;
	CPoint rst2;

	double win_cx = param.cx;
	double win_cy = param.cy;
	if(sAlignRes)
	{
		CProc pProc;
		pProc.CorrectCoordinate(win_cx, win_cy, 0, 0, sAlignRes->theta, sAlignRes->offsetX, sAlignRes->offsetY, &win_cx, &win_cy);
	}
	rst = CvtBoradToPixel(boardInfo->fovCx, boardInfo->fovCy, boardInfo->partCx + win_cx, boardInfo->partCy + win_cy, boardInfo->partCx, boardInfo->partCy, boardInfo);
	ret.cx = rst.x;
	ret.cy = rst.y;
	ret.width = RounD(param.width  / m_resolX);
	ret.length = RounD(param.length  / m_resolY);

	rst = CvtBoradToPixel(boardInfo->fovCx, boardInfo->fovCy, param.bdrCx, param.bdrCy, param.bdrCx, param.bdrCy, boardInfo);
	ret.bdrCx = rst.x;
	ret.bdrCy = rst.y;
	ret.bdrWidth = RounD(param.bdrWidth  / m_resolX);
	ret.bdrLength = RounD(param.bdrLength / m_resolY);

	rst = CvtBoradToPixel(boardInfo->fovCx, boardInfo->fovCy, param.anyAngleCx, param.anyAngleCy, param.anyAngleCx, param.anyAngleCy, boardInfo);
	ret.anyAngleCx = rst.x;
	ret.anyAngleCy = rst.y;
	ret.anyAngleWidth = RounD(param.anyAngleWidth  / m_resolX);
	ret.anyAngleLength = RounD(param.anyAngleLength / m_resolY);

	ret.fovCx = boardInfo->fovCx;
	ret.fovCy = boardInfo->fovCy;

	ret.angle = boardInfo->angle;

#ifdef _OFFLINE_INSP
	ret.cx = (int)param.cx;
	ret.cy = (int)param.cy;
	ret.width = (int)param.width;
	ret.length = (int)param.length;

	ret.bdrCx = (int)param.bdrCx;
	ret.bdrCy = (int)param.bdrCy;
	ret.bdrWidth = (int)param.bdrWidth;
	ret.bdrLength = (int)param.bdrLength;

	ret.anyAngleCx = (int)param.anyAngleCx;
	ret.anyAngleCy = (int)param.anyAngleCy;
	ret.anyAngleWidth = (int)param.anyAngleWidth;
	ret.anyAngleLength = (int)param.anyAngleLength;

	ret.fovCx = 0;
	ret.fovCy = 0;

	ret.angle = boardInfo->angle;
#endif

	return ret;
}

Coordinate CManager::SetCoordinate(InspPartInfo* boardInfo, TeachParam* param)
{
	Coordinate ret;
	CPoint rst;
	CPoint rst2;
	
	rst = CvtBoradToPixel(boardInfo->fovCx, boardInfo->fovCy, param->cx, param->cy, param->cx, param->cy, boardInfo);
	ret.cx = rst.x;
	ret.cy = rst.y;
	ret.width = RounD(param->width / m_resolX);
	ret.length = RounD(param->length / m_resolY);

	rst = CvtBoradToPixel(boardInfo->fovCx, boardInfo->fovCy, param->bdrCx, param->bdrCy, param->bdrCx, param->bdrCy, boardInfo );
	ret.bdrCx = rst.x;
	ret.bdrCy = rst.y;
	ret.bdrWidth = RounD(param->bdrWidth / m_resolX);
	ret.bdrLength =  RounD(param->bdrLength / m_resolY);

	rst = CvtBoradToPixel(boardInfo->fovCx, boardInfo->fovCy, param->anyAngleCx, param->anyAngleCy, param->anyAngleCx, param->anyAngleCy , boardInfo );
	ret.anyAngleCx = rst.x;
	ret.anyAngleCy = rst.y;
	ret.anyAngleWidth = RounD(param->anyAngleWidth  / m_resolX);
	ret.anyAngleLength = RounD(param->anyAngleLength / m_resolY);

	ret.fovCx = boardInfo->fovCx;
	ret.fovCy = boardInfo->fovCy;

	ret.angle = boardInfo->angle;


#ifdef _OFFLINE_INSP
	ret.cx = (int)param->cx;
	ret.cy = (int)param->cy;
	ret.width = (int)param->width;
	ret.length = (int)param->length;

	ret.bdrCx = (int)param->bdrCx;
	ret.bdrCy = (int)param->bdrCy;
	ret.bdrWidth = (int)param->bdrWidth;
	ret.bdrLength = (int)param->bdrLength;

	ret.anyAngleCx = (int)param->anyAngleCx;
	ret.anyAngleCy = (int)param->anyAngleCy;
	ret.anyAngleWidth = (int)param->anyAngleWidth;
	ret.anyAngleLength = (int)param->anyAngleLength;

	ret.fovCx = 0;
	ret.fovCy = 0;

	ret.angle = boardInfo->angle;
#endif
	return ret;
}

Coordinate CManager::SetCoordinate(PartSearchParam* pam)
{
	Coordinate ret;
	CPoint rst;
	CPoint rst2;

	rst = CvtBoradToPixel(pam->fovCx, pam->fovCy, pam->cx, pam->cy, pam->cx, pam->cy);
	ret.cx = rst.x;
	ret.cy = rst.y;
	ret.width = RounD(pam->width / m_resolX);
	ret.length = RounD(pam->length / m_resolY);

	ret.fovCx = pam->fovCx;
	ret.fovCy = pam->fovCy;

	ret.angle = 0;
#ifdef _OFFLINE_INSP
	ret.cx = (int)pam->cx;
	ret.cy = (int)pam->cy;
	ret.width = (int)pam->width;
	ret.length = (int)pam->length;

	ret.fovCx = 0;
	ret.fovCy = 0;

	ret.angle = 0;

#endif

	return ret;
}

// ctRoiRealCx,y : ctRoiCx,y는 left, top을 넘기는 경우가 있어서, center 좌표로 렌즈왜곡 보정하기 위한 용도
CPoint CManager::CvtBoradToPixel(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double ctRoiRealCx, double ctRoiRealCy, InspPartInfo* boardInfo)
{
	double dRetPixelX, dRetPixelY;
	return CvtBoradToPixel(ctFovCx, ctFovCy, ctRoiCx, ctRoiCy, ctRoiRealCx, ctRoiRealCy, dRetPixelX, dRetPixelY, boardInfo);
}

CPoint CManager::CvtBoradToPixel(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double ctRoiRealCx, double ctRoiRealCy, double& dRetPixelX, double& dRetPixelY, InspPartInfo* boardInfo)
{
	CPoint ret;
	double rstXpixel = 0.0;
	double rstYpixel = 0.0;

	int nFovWidth = m_fovWidth;
	int nFoVHeight = m_fovLength;
	double dResolX = m_resolX;
	double dResolY = m_resolY;

	double dFovCx = ctFovCx;
	double dFovCy = ctFovCy;
	double dRoiCx = ctRoiCx;
	double dRoiCy = ctRoiCy;

// 	if (boardInfo != NULL && boardInfo->BtmCameraUse == 1)
// 	{
// 		nFovWidth = boardInfo->BtmCameraWidth;
// 		nFoVHeight = boardInfo->BtmCameraHeight;
// 
// 		dResolX = g_pMPTI->m_dBtmSideResX;
// 		dResolY = g_pMPTI->m_dBtmSideResY;
// 
// 		double fovWidth = 0;
// 		double fovLength = 0;
// 		double rstXmm = 0.0;
// 		double rstYmm = 0.0;
// 
// 		fovWidth = (nFovWidth/*m_fovWidth*/ * dResolX);  //unit : pixel -> mm
// 		fovLength = (nFoVHeight/*m_fovLength*/ * dResolY);  //unit : pixel -> mm
// 
// 
// 		double fovCX = ctFovCx; //unit : mm
// 		double fovCY = ctFovCy; //unit : mm
// 		//20141010 SHW : TEST 
// 		double srcPointX = ctRoiCx;
// 		double srcPointY = ctRoiCy;
// 
// 		rstXmm = -fovCX + (fovWidth / 2.0) + srcPointX;  //unit : mm
// 		rstYmm = fovCY + (fovLength / 2.0) - srcPointY;  //unit : mm
// 
// 		double rstXpixel = rstXmm * (/*m_fovWidth*/nFovWidth / fovWidth);   //unit : pixel
// 		double rstYpixel = rstYmm * (/*m_fovLength*/nFoVHeight / fovLength);  //unit : pixel
// 
// 		ret.SetPoint(RounD(rstXpixel), RounD(rstYpixel));
// 	}
// 	else
	{
		double dCamCenterX = ((double)nFovWidth - 1) / 2.0;
		double dCamCenterY = ((double)nFoVHeight - 1) / 2.0;

		double dOffsetX = (dFovCx - dRoiCx) / dResolX;
		double dOffsetY = (dFovCy - dRoiCy) / dResolY;

		rstXpixel = dCamCenterX - dOffsetX;
		rstYpixel = dCamCenterY + dOffsetY;

// 		if (g_pMPTI->m_bUseLensDistortion)
// 		{
// 			dOffsetX = dCamCenterX - (dFovCx - ctRoiRealCx) / dResolX;
// 			dOffsetY = dCamCenterY + (dFovCy - ctRoiRealCy) / dResolY;
// 
// 			double dstX = dOffsetX, dstY = dOffsetY;
// 			MPTI_LensCalcRefToObj(dOffsetX, dOffsetY, &dstX, &dstY);
// 
// 			dOffsetX = dstX - dOffsetX;
// 			dOffsetY = dstY - dOffsetY;
// 
// 			dRetPixelX = rstXpixel + dOffsetX;
// 			dRetPixelY = rstYpixel + dOffsetY;
// 		}
// 		else
		{
			dRetPixelX = rstXpixel;
			dRetPixelY = rstYpixel;
		}
		ret.SetPoint(RounD(dRetPixelX), RounD(dRetPixelY));
	}

	return ret;
}

void CManager::CvtPixelToBoard(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double* retX, double* retY, int mode) 
{

	double fovWidth = ((m_fovWidth) * m_resolX);  //unit : pixel -> mm
	double fovLength = ((m_fovLength) * m_resolY);  //unit : pixel -> mm

	double m = ctRoiCx * fovWidth / (m_fovWidth);
	double n = ctRoiCy * fovLength / (m_fovLength);

	double x = 0;
	double y = 0;

	x = ctFovCx - (fovWidth / 2.0) + m;
	y = ctFovCy + (fovLength / 2.0) - n;

	*retX = x;
	*retY = y;
}

Coordinate CManager::SetCoordinate_Offset(Coordinate input, AlignResult offset)
{
	Coordinate ret;

	//offset ¸¸Å­ ÀÌµ¿½ÃÄÑÁÜ..
	ret.cx = input.cx + offset.offsetX;
	ret.cy = input.cy + offset.offsetY;

	ret.bdrCx = input.bdrCx + offset.offsetX;
	ret.bdrCy = input.bdrCy + offset.offsetY;
	
	//////////////////////////////////////////////////////////////////////////

	ret.fovCx = input.fovCx;
	ret.fovCy = input.fovCy;

	ret.width = input.width;
	ret.length = input.length;
	ret.bdrWidth = input.bdrWidth;
	ret.bdrLength = input.bdrLength;

	ret.angle = input.angle;

	return ret;
}


BOOL CManager::IsAnyAngle(double angle)
{
	BOOL ret = FALSE;

	double temp = 0;
	temp = angle / 90.0;
	temp = angle - (int)temp * 90;

	if(temp != 0)
		ret = TRUE;

	return ret;
}

double CManager::CalcRotateAngle(double orgAngle)
{
	double retAngle = 0.0;

	double tempAngle = (orgAngle - (360.0 * (int)(orgAngle / 360.0)));
	if(tempAngle >= 0)
		retAngle = 360.0 - tempAngle;
	else
		retAngle = abs(tempAngle);

	return  retAngle;
}
