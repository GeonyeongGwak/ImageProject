#include "StdAfx.h"
#include "PInsp_LeadSolder.h"

#include "PInsp_Color.h"		// LMJ 2014/01/03

#include "ipps.h"
#include "ippi.h"
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")

#define  EXRTW  2
#define  EXRTL  2
#define  SOLDERW_MARGIN		6

#define	COLD_VOL	70.0

BYTE* g_lutData_color; 

CPInsp_LeadSolder::CPInsp_LeadSolder(void)
{
	m_className = _T("CPInsp_LeadSolder");

	m_fovTargetImg = NULL; //shk 2014/04/28

	m_pLeadRect = NULL;
	m_pGapRect = NULL;
	m_pSolderRect = NULL;

	m_pZmapRoiData = NULL;
	m_pRoiImg_Top = NULL;
	m_pRoiImg_Bottom = NULL;
	m_pRoiImg_Mid = NULL; //shk 2014/01/16
	m_pRoiImg_Tar = NULL; //shk 20140425

	m_pImage_HV = NULL;
	m_pZmapBoundaryData = NULL;

	m_pLeadHeight = NULL;
	m_pLeadLiftRst = NULL;
	m_pLeadBridgeRst = NULL;
	m_pLeadPitch = NULL;


	m_pFiletLength = NULL;
	m_pSolderHeight = NULL;
	m_solderColorRate = NULL;
	m_solderVolRst = NULL;
	m_solderColorRst = NULL;
	m_solderVolume = NULL;
	m_solderRatio = NULL;
	//shk 20140318
	m_graysolderRatio = NULL;

	m_pRstSolderStartPos = NULL;

	m_solderCount = 0;

	m_extractW = 0;
	m_extractL = 0;

	m_leadAvrH = 0;

	m_pNgLeadInfo = NULL;
	m_wndAngle = 0;
	m_ngLeadRect = NULL;
	m_ngGapRect = NULL;
	m_ngSolderRect = NULL;

	m_bdryCx = 0;
	m_bdryCy = 0;
	m_bdryW = 0;
	m_bdryL = 0;
	m_isAnyAngle = FALSE;

	m_pProcMilLeadSolder = NULL;
	g_lutData_color = NULL;
	InitResultStruct(&m_inspResult);
	//InitMakeLUT_Color();		// LMJ 2014/01/10
}

CPInsp_LeadSolder::~CPInsp_LeadSolder(void)
{
	DeleteAllData();
	DeleteNgInfoBuff();
	CloseDevice();
// 	if(m_lutData_color)
// 		delete [] m_lutData_color;		// LMJ 2014/01/10
}


int CPInsp_LeadSolder::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY,bUseImagePilLib);

	//m_pProcMilLeadSolder = new CProcMil_LeadSolder();
	m_pProcMilLeadSolder = g_pMManager->pem_new<CProcMil_LeadSolder>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pProcMilLeadSolder->InitMil(milApp, milSys,bUseImagePilLib);
	m_pProcMilLeadSolder->SetResol(m_resolX, m_resolY, m_fovWidth);

	return ePART_SUCCESS;
}

int CPInsp_LeadSolder::CloseDevice()
{
	CPInsp::CloseDevice();

	if(m_pProcMilLeadSolder != NULL)
	{
		m_pProcMilLeadSolder->FreeMil();
		
		//delete m_pProcMilLeadSolder;
		g_pMManager->pem_delete(m_pProcMilLeadSolder, false);
		m_pProcMilLeadSolder = NULL;
	}

	return ePART_SUCCESS;	
}

// LMJ 2014/01/03
//int CPInsp_LeadSolder::SetInspParam(void* itemParam, void* targetImg_Top, void* targetImg_Bottom, ZmapData zmap, Coordinate cdn)
int CPInsp_LeadSolder::SetInspParam(void* itemParam,  void* targetImg, InspImgBuf img, ZmapData zmap, Coordinate cdn) //shk 20140425
{
	//shk 20140425
	m_fovTargetImg =  targetImg;
	
	m_inspParam = (InspParamLeadSolder*)itemParam;
	m_fovImageTop_insp = img.imgTop_R;
	m_fovImageBottom_insp = img.imgBottom_R;
	m_fovImageMid_insp = img.imgMiddle_R; //shk 2014/01/16

	m_inspZmapData =  zmap;
	m_inspCoordinate = cdn;

	m_fovImageTop_R = img.imgTop_R;
	m_fovImageTop_G = img.imgTop_G;
	m_fovImageTop_B = img.imgTop_B;

	return ePART_SUCCESS;
}

// LMJ 2014/01/03
//int CPInsp_LeadSolder::SetTeachParam(void* itemParam, void* targetImg_Top, void* targetImg_Bottom, int teachType, ZmapData zmap, Coordinate cdn)
int CPInsp_LeadSolder::SetTeachParam(void* itemParam, void* targetImg,InspImgBuf img, int teachType, ZmapData zmap, Coordinate cdn) //shk 20140425
{
	//shk 20140425
	m_fovTargetImg = targetImg;

	m_teachParam = (TeachParamLeadSolder*) itemParam;
	m_fovImageTop_teach = img.imgTop_R;
	m_fovImageBottom_teach  = img.imgBottom_R;
	m_fovImageMid_teach = img.imgMiddle_R; //shk 2014/01/16

	m_teachZmapData = zmap;
	m_teachCoordinate = cdn;

	m_fovImageTop_R = img.imgTop_R;
	m_fovImageTop_G = img.imgTop_G;
	m_fovImageTop_B = img.imgTop_B;

	return ePART_SUCCESS;
}



int CPInsp_LeadSolder::InspProc_LeadSolder()
{
	int ret = 0;

	ret = ProcAuto();

	return ret;
}

int CPInsp_LeadSolder::ProcAuto()
{
	int ret = e_OK;
	int stepID = 0;
	int maxStepCnts = eLSStepID_MAX_COUNT;

	//step1(eLSStepID_CALC_RECT) 
	//step2(eLSStepID_LEAD) 
	//step2(eLSStepID_SOLDER)

	DeleteAllData();
	InitResultStruct(&m_inspResult);


	stepID = 0;
	for(int j = 0; j < maxStepCnts; j++)
	{
		ret = ProcStep(stepID);
		stepID++;

// 		if(ret != e_OK) 
// 		{
// 			break;
// 		}
	}

	ret = DecisionSolderInsp();
	return ret;
}


int CPInsp_LeadSolder::ProcStep(int stepID)
{
	int ret = e_OK;

	BOOL isChipSolder = m_inspParam->isChipSolder;
	BOOL leadFlag = m_inspParam->leadFlag;
	BOOL solderFlag = m_inspParam->solderFlag;

	switch(stepID)
	{
	case eLSStepID_CALC_RECT:
		{
			CalcLeadnSolderArea();
		}
		break;
	case eLSStepID_SOLDER:
		{
			if(solderFlag == TRUE)
			{
				ret = InspSolder();
			}
		}
		break;
	case eLSStepID_LEAD:
		{
			if(leadFlag == TRUE && isChipSolder == FALSE)
			{
				ret = InspLead();
			}
		}
		break;
	
	}

	return ret;
}

int CPInsp_LeadSolder::CalcLeadnSolderArea()
{
	int ret = e_OK;
	float* zmapData = m_inspZmapData.data;
	int zmapSizeX = m_inspZmapData.zmapSizeX;
	int zmapSizeY = m_inspZmapData.zmapSizeY;
	int cx = m_inspCoordinate.bdrCx;
	int cy = m_inspCoordinate.bdrCy;
	int roiSizeX = m_inspCoordinate.bdrWidth;
	int roiSizeY = m_inspCoordinate.bdrLength;
	int leadPos = m_inspParam->leadPosition;
	int solderStartPos = m_inspParam->solderStartPos;
	int solderEndPos = m_inspParam->solderEndPos;
	double wndAngle = m_inspCoordinate.angle;
	int roiArea = roiSizeX * roiSizeY ;
	BOOL isChipSolder = m_inspParam->isChipSolder;
	BOOL inspHeightFlag = m_inspParam->inspHeightFlag;

	int bdCx = m_inspCoordinate.anyAngleCx;
	int bdCy = m_inspCoordinate.anyAngleCy;
	int bdSizeX = m_inspCoordinate.anyAngleWidth;
	int bdSizeY = m_inspCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	BOOL isContainer = m_inspParam->isContainer;
	BOOL isInspLeadLift = m_inspParam->inspHeightFlag;
	
	double dWidthLead = _mm2pixel_x(m_inspParam->stdLeadWidth);
	float dHeightLead = m_inspParam->stdLeadHeight;
	int dCntLead = m_inspParam->stdLeadCount;

	int threshold = m_inspParam->threshold;
	if(threshold > 255)		threshold = 255;
	else if(threshold < 0)		threshold = 0;

	BOOL use3D = m_inspParam->use3D;

	if(roiSizeX < 1 || roiSizeY < 1 || zmapData == NULL || solderStartPos < 0 || solderEndPos < 0 || solderEndPos < solderStartPos || leadPos < 0 || leadPos > 3)
		return e_NG;

	if(wndAngle < 0)
		wndAngle += 360 ;

	if(wndAngle > 360)
		wndAngle -= 360;

	DeleteBuffer();

	//////////////////////////////////////////////////////////////////////////
	float* zmapRoiDataTemp = NULL;
	UCHAR* fovImgTopTemp = NULL;
	UCHAR* fovImgBottomTemp = NULL;
	UCHAR* fovImgMidTemp = NULL; //shk 2014/01/16
	//shk 20140425
	UCHAR* fovImgTarTemp = NULL; 

	BOOL isAnyAngle = IsAnyAngle(wndAngle);
	m_isAnyAngle = isAnyAngle;
	if(isAnyAngle)
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;
		float* bdryZmapTemp = NULL;
		m_proc3d.ProcAnyAngle_Zmap(zmapData, &bdryZmapTemp, zmapSizeX, zmapSizeY, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		m_pProcMilLeadSolder->SaveWorkImg_float(bdryZmapTemp, w, l, _T("LEAD_boundary.bmp"));

		UCHAR* bdryImgTemp_top = NULL;
		m_pProcMilLeadSolder->ProcAnyAngle_Img(m_fovImageTop_insp, &bdryImgTemp_top, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
		m_pProcMilLeadSolder->SaveWorkImg(bdryImgTemp_top, w, l, _T("LEAD_boundaryImgTop.bmp"));

		UCHAR* bdryImgTemp_bottom = NULL;
		m_pProcMilLeadSolder->ProcAnyAngle_Img(m_fovImageBottom_insp, &bdryImgTemp_bottom, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
		m_pProcMilLeadSolder->SaveWorkImg(bdryImgTemp_bottom, w, l, _T("LEAD_boundaryImgBottom.bmp"));

		//shk 2014/01/16
		UCHAR* bdryImgTemp_mid = NULL;
		m_pProcMilLeadSolder->ProcAnyAngle_Img(m_fovImageMid_insp, &bdryImgTemp_mid, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
		m_pProcMilLeadSolder->SaveWorkImg(bdryImgTemp_mid, w, l, _T("LEAD_boundaryImgMid.bmp"));

		//shk 2014/01/16
		UCHAR* bdryImgTemp_Tar = NULL;
		m_pProcMilLeadSolder->ProcAnyAngle_Img(m_fovTargetImg, &bdryImgTemp_Tar, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
		m_pProcMilLeadSolder->SaveWorkImg(bdryImgTemp_Tar, w, l, _T("LEAD_boundaryImgMid.bmp"));

		wndAngle = 0;
		int tempCx = cx -  (int)(bdCx - (w / 2.0));
		int tempCy = cy -  (int)(bdCy - (l / 2.0));

		//zmapRoiDataTemp = new float[roiArea];
		zmapRoiDataTemp = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(bdryZmapTemp, zmapRoiDataTemp, w, l, tempCx, tempCy, roiSizeX, roiSizeY);
		m_pProcMilLeadSolder->SaveWorkImg_float(zmapRoiDataTemp, roiSizeX, roiSizeY, _T("LEAD_zmapRoiDataTemp.bmp"));

		//fovImgTopTemp = new UCHAR[roiArea];
		fovImgTopTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilLeadSolder->GetClipImage(bdryImgTemp_top, w, l, fovImgTopTemp, tempCx, tempCy, roiSizeX, roiSizeY);
		m_pProcMilLeadSolder->SaveWorkImg(fovImgTopTemp, roiSizeX, roiSizeY, _T("LEAD_fovImgTopTemp.bmp"));

		//fovImgBottomTemp = new UCHAR[roiArea];
		fovImgBottomTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilLeadSolder->GetClipImage(bdryImgTemp_bottom, w, l, fovImgBottomTemp, tempCx, tempCy, roiSizeX, roiSizeY);
		m_pProcMilLeadSolder->SaveWorkImg(fovImgBottomTemp, roiSizeX, roiSizeY, _T("LEAD_fovImgBottomTemp.bmp"));
		
		//shk 2014/01/16
		//fovImgMidTemp = new UCHAR[roiArea];
		fovImgMidTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilLeadSolder->GetClipImage(bdryImgTemp_mid, w, l, fovImgMidTemp, tempCx, tempCy, roiSizeX, roiSizeY);
		m_pProcMilLeadSolder->SaveWorkImg(fovImgMidTemp, roiSizeX, roiSizeY, _T("LEAD_fovImgMidTemp.bmp"));	

		//shk 2014/04/25
		//fovImgTarTemp = new UCHAR[roiArea];
		fovImgTarTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilLeadSolder->GetClipImage(bdryImgTemp_mid, w, l, fovImgTarTemp, tempCx, tempCy, roiSizeX, roiSizeY);
		m_pProcMilLeadSolder->SaveWorkImg(fovImgTarTemp, roiSizeX, roiSizeY, _T("LEAD_fovImgMidTemp.bmp"));	
		//////////////////////////////////////////////////////////////////////////

		m_bdryCx = tempCx;
		m_bdryCy = tempCy;
		m_bdryW = w;
		m_bdryL = l;
		//m_pZmapBoundaryData = new float[w * l];
		m_pZmapBoundaryData = g_pMManager->pem_new<float>(true, w * l, (PCHAR)__FUNCTION__, __LINE__);
		memcpy_s(m_pZmapBoundaryData, sizeof(float) * w * l, bdryZmapTemp, sizeof(float) * w * l);
		//////////////////////////////////////////////////////////////////////////

		//delete bdryZmapTemp;
		//delete bdryImgTemp_top;
		//delete bdryImgTemp_bottom;
		//delete bdryImgTemp_mid; //shk 2014/01/16
		//delete bdryImgTemp_Tar; //shk 20140425
		g_pMManager->pem_delete(bdryZmapTemp, false);
		g_pMManager->pem_delete(bdryImgTemp_top, false);
		g_pMManager->pem_delete(bdryImgTemp_bottom, false);
		g_pMManager->pem_delete(bdryImgTemp_mid, false);
		g_pMManager->pem_delete(bdryImgTemp_Tar, false);
	}
	else
	{
		//roi영역만큼 추출
		//zmapRoiDataTemp = new float[roiArea];
		zmapRoiDataTemp = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(zmapData, zmapRoiDataTemp, zmapSizeX, zmapSizeY, cx, cy, roiSizeX, roiSizeY);

		//fovImgTopTemp = new UCHAR[roiArea];
		fovImgTopTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilLeadSolder->GetClipBuff(m_fovImageTop_insp, fovImgTopTemp, cx, cy, roiSizeX, roiSizeY);

		//fovImgBottomTemp = new UCHAR[roiArea];
		fovImgBottomTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilLeadSolder->GetClipBuff(m_fovImageBottom_insp, fovImgBottomTemp, cx, cy, roiSizeX, roiSizeY);
		//shk 2014/01/16
		//fovImgMidTemp = new UCHAR[roiArea];
		fovImgMidTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilLeadSolder->GetClipBuff(m_fovImageMid_insp, fovImgMidTemp, cx, cy, roiSizeX, roiSizeY);
		//shk 2014/04/25
		//fovImgTarTemp = new UCHAR[roiArea];
		fovImgTarTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilLeadSolder->GetClipBuff(m_fovTargetImg, fovImgTarTemp, cx, cy, roiSizeX, roiSizeY);
	}

	//////////////////////////////////////////////////////////////////////////
	//bottom 위치가 아닐경우 bottom위치로 회전.
	int w = 0;
	int h = 0;	
	double angle = GetRotateAngle(leadPos, roiSizeX, roiSizeY, wndAngle, &w, &h);
	m_wndAngle = angle;

	//m_pZmapRoiData = new float[roiArea];
	m_pZmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	m_pProcMilLeadSolder->RotateZmap_ipp(zmapRoiDataTemp, m_pZmapRoiData, roiSizeX, roiSizeY, angle);		// LMJ 2013/11/25
	//delete zmapRoiDataTemp;
	g_pMManager->pem_delete(zmapRoiDataTemp, false);

	//m_pRoiImg_Top = new UCHAR[roiArea];
	m_pProcMilLeadSolder->RotateImg_ipp(fovImgTopTemp, roiSizeX, roiSizeY, angle, &m_pRoiImg_Top);	// LMJ 2013/11/25
	//delete fovImgTopTemp;
	g_pMManager->pem_delete(fovImgTopTemp, false);

	//m_pRoiImg_Bottom = new UCHAR[roiArea];
	m_pProcMilLeadSolder->RotateImg_ipp(fovImgBottomTemp, roiSizeX, roiSizeY, angle, &m_pRoiImg_Bottom);	// LMJ 2013/11/25
	//delete fovImgBottomTemp;
	g_pMManager->pem_delete(fovImgBottomTemp, false);

	//mid image //shk 2014/01/16
	//m_pRoiImg_Mid = new UCHAR[roiArea];
	m_pProcMilLeadSolder->RotateImg_ipp(fovImgMidTemp, roiSizeX, roiSizeY, angle, &m_pRoiImg_Mid);	
	//delete fovImgMidTemp;
	g_pMManager->pem_delete(fovImgMidTemp, false);

	//bottom image - top image
	//m_pImage_HV = new UCHAR[roiArea];
	m_pImage_HV = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
//	m_procMil->GetImage_HV(m_pRoiImg_Top, m_pRoiImg_Bottom, w, h, m_pImage_HV);
	//shk 2014/01/16
	m_pProcMilLeadSolder->GetImage_HV(m_pRoiImg_Top, m_pRoiImg_Mid, m_pRoiImg_Bottom, w, h, m_pImage_HV, threshold); 
	
	//target image //shk 20140425
	//m_pRoiImg_Tar = new UCHAR[roiArea];
	m_pProcMilLeadSolder->RotateImg_ipp(fovImgTarTemp, roiSizeX, roiSizeY, angle, &m_pRoiImg_Tar);	
	//delete fovImgTarTemp;
	g_pMManager->pem_delete(fovImgTarTemp, false);

	//////////////////////////////////////////////////////////////////////////

	m_extractW = EXRTW;
	m_extractL = EXRTL;

	int solderCnt = 0;
	if(!isChipSolder)
	{	
		solderCnt = CalcLeadSolderRect(m_pRoiImg_Top, m_pImage_HV, m_pZmapRoiData, w, h, 
									solderStartPos, solderEndPos, isContainer, isInspLeadLift, dHeightLead, dWidthLead, dCntLead, use3D, TRUE);	//shk 2014-01-03					
	}
	else
	{	
		solderCnt = 1;

		InitSolderRect(solderCnt);
		m_pSolderRect[0].top = (LONG)0;//(LONG)solderStartPos;
		m_pSolderRect[0].bottom = (LONG)h;
		m_pSolderRect[0].left = (LONG)0;
		m_pSolderRect[0].right = (LONG)w;
	}

	//////////////////////////////////////////////////////////////////////////

	m_solderCount = solderCnt;

	
	if(solderCnt < 1)
		ret = e_NG;

	m_inspResult.isInsp = TRUE;

	return ret;
}

int CPInsp_LeadSolder::InspLead()
{
	int ret = e_OK;

	float* zmapData = m_inspZmapData.data;
	int zmapSizeX = m_inspZmapData.zmapSizeX;
	int zmapSizeY = m_inspZmapData.zmapSizeY;

	int cx = m_inspCoordinate.bdrCx;
	int cy = m_inspCoordinate.bdrCy;
	int roiSizeX = m_inspCoordinate.bdrWidth;
	int roiSizeY = m_inspCoordinate.bdrLength;
	int leadCnt = m_solderCount;
	int gapCnt = m_solderCount - 1;
	int leadPos = m_inspParam->leadPosition;
	double range_Hmax = m_inspParam->range_MaxH;
	//double range_Hmin = m_inspParam->range_MinH;
	double wndAngle = m_inspCoordinate.angle;
	double stdH_Max = 0;

	BOOL inspHeightFlag = m_inspParam->inspHeightFlag;
	BOOL inspBridgeFlag = m_inspParam->inspBridgeFlag;
	BOOL bridge2dOnly = m_inspParam->bridgeOption_2dOnly;
	BOOL isContainer = m_inspParam->isContainer;
	bool isInspLeadPitch = m_inspParam->isInspLeadPitch;
	//double stdH_Min = 0;

	int solderStartPos = m_inspParam->solderStartPos;

	double stdLeadH = m_inspParam->stdLeadHeight;


	if(m_pZmapRoiData == NULL || m_pRoiImg_Tar ==NULL ||  m_pRoiImg_Top == NULL || m_pRoiImg_Bottom == NULL || m_pRoiImg_Mid == NULL || roiSizeX < 1 || roiSizeY < 1 || leadPos < 0 || leadPos > 3)
		return e_NG;

	if(wndAngle < 0)
		wndAngle += 360 ;

	if(wndAngle > 360)
		wndAngle -= 360;

	if(m_isAnyAngle)
		wndAngle = 0;

	int w = 0;
	int h = 0;	
	double angle = GetRotateAngle(leadPos, roiSizeX, roiSizeY, wndAngle, &w, &h);

	//////////////////////////////////////////////////////////////////////////
	//gap 검사 (bridge)
	BOOL totalGapRst = FALSE;

	if(gapCnt > 0 && isContainer == TRUE)
	{
		//BOOL* gapRst = new BOOL[gapCnt];
		BOOL* gapRst = g_pMManager->pem_new<BOOL>(true, gapCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(gapRst, 0 ,sizeof(BOOL) * gapCnt);

		if(inspBridgeFlag)
		{
			float stdHtemp = (float)stdLeadH;
			if(inspHeightFlag == TRUE)
				stdHtemp = 0;

			totalGapRst = m_pProcMilLeadSolder->InspGapArea2(m_pRoiImg_Mid, m_pRoiImg_Bottom, m_pZmapRoiData, w, h, m_pGapRect, (int)_mm2pixel_x(m_leadInfo.gapWidth), gapCnt, 0, bridge2dOnly, stdHtemp, gapRst);
		}
		else
		{
			totalGapRst = TRUE;
		}

		InitLeadBridgeVal(gapCnt);
		memcpy_s(m_pLeadBridgeRst, sizeof(BOOL) * gapCnt, gapRst, sizeof(BOOL) * gapCnt);	

		//delete gapRst;
		g_pMManager->pem_delete(gapRst, false);

		//////////////////////////////////////////////////////////////////////////

		//임시로 피치 검사를 브릿지 검사 결과에 넣음.. (defect code가 없고 spc에도 작업을 해줘야 하기 때문에...)
		if(isInspLeadPitch && m_pLeadPitch != NULL)
		{
			int subTemp = 0;
			int avrLeadPitch = (int)_mm2pixel_x(m_leadInfo.leadPitch);
			for(int i = 0; i < gapCnt; i++)
			{
				subTemp = abs((int)(m_pLeadPitch[i] - avrLeadPitch));
				if(subTemp > 5)
				{
					m_pLeadBridgeRst[i] = TRUE;	
					totalGapRst = TRUE;
				}
			}
		}	
		//////////////////////////////////////////////////////////////////////////
	}
	else //huj 2014/01/27
	{
		if(gapCnt > 0)
		{
			//BOOL* gapRst = new BOOL[gapCnt];
			BOOL* gapRst = g_pMManager->pem_new<BOOL>(true, gapCnt, (PCHAR)__FUNCTION__, __LINE__);
			memset(gapRst, 0 ,sizeof(BOOL) * gapCnt);

			InitLeadBridgeVal(gapCnt);
			memcpy_s(m_pLeadBridgeRst, sizeof(BOOL) * gapCnt, gapRst, sizeof(BOOL) * gapCnt);

			//delete gapRst;
			g_pMManager->pem_delete(gapRst, false);
		}

	}

	//////////////////////////////////////////////////////////////////////////
	//lead 검사 
	BOOL totalLeadRst = FALSE;

	if(leadCnt > 0)
	{
		//BOOL* leadRst = new BOOL[leadCnt];
		BOOL* leadRst = g_pMManager->pem_new<BOOL>(true, leadCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(leadRst, 0 ,sizeof(BOOL) * leadCnt);

		float totalAvrH = 0;
		//float* avrH = new float[leadCnt];
		float* avrH = g_pMManager->pem_new<float>(true, leadCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(avrH, 0 ,sizeof(float) * leadCnt);
		//float totalAvrH = m_proc3d.CalcLeadHeight(m_pZmapRoiData, w, h, m_pLeadRect, leadCnt, 0, avrH);  //line
	
		if(isContainer == FALSE)
		{
			if(inspHeightFlag)
			{
				//totalAvrH = m_proc3d.CalcLeadHeight(m_pZmapRoiData,w, h, EXRTW, EXRTL, m_pLeadRect, leadCnt, avrH);  //rect
				//shk 2014/04/10
				totalAvrH = m_proc3d.CalcLeadHeight2(m_pZmapRoiData,w, h, EXRTW, EXRTL, m_pLeadRect, leadCnt, avrH);  //rect
			}
			else
			{
				double sumTemp = 0;
				CRect leadRectTemp(0, 0, 0, 0);
				int wndLeft = 0;
				int wndTop = 0;
				if(!m_isAnyAngle)
				{
					wndLeft = (int)(cx - (roiSizeX / 2.0));
					wndTop = (int)(cy - (roiSizeY / 2.0));
				}
				else
				{
					wndLeft = (int)(m_bdryCx - (roiSizeX / 2.0));
					wndTop = (int)(m_bdryCy - (roiSizeY / 2.0));
				}

				for(int i = 0; i < leadCnt; i++)	
				{
					if(angle == 0)
					{
						leadRectTemp.left = (LONG)(wndLeft + m_pLeadRect[i].left);
						leadRectTemp.right = (LONG)(leadRectTemp.left + m_pLeadRect[i].Width());
						leadRectTemp.top = (LONG)wndTop;
						leadRectTemp.bottom = (LONG)(leadRectTemp.top + roiSizeY);
					}
					else if(angle == 90)
					{
						leadRectTemp.left = (LONG)wndLeft;
						leadRectTemp.right = (LONG)(leadRectTemp.left + roiSizeX);
						leadRectTemp.top = (LONG)(wndTop + m_pLeadRect[i].left);
						leadRectTemp.bottom = (LONG)(leadRectTemp.top + m_pLeadRect[i].Width());
					}
					else if(angle == 180)
					{
						leadRectTemp.left = (LONG)(wndLeft + (roiSizeX - m_pLeadRect[(leadCnt - 1) - i].right));
						leadRectTemp.right = (LONG)(leadRectTemp.left + m_pLeadRect[(leadCnt - 1) - i].Width());
						leadRectTemp.top = (LONG)wndTop;
						leadRectTemp.bottom = (LONG)(leadRectTemp.top + roiSizeY);
					}
					else if(angle == 270)
					{
						leadRectTemp.left = (LONG)wndLeft;
						leadRectTemp.right = (LONG)(leadRectTemp.left + roiSizeX);
						leadRectTemp.top = (LONG)(wndTop + (roiSizeY - m_pLeadRect[(leadCnt - 1) - i].right));
						leadRectTemp.bottom = (LONG)(leadRectTemp.top + m_pLeadRect[(leadCnt - 1) - i].Width());
					}


					if(!m_isAnyAngle)
						avrH[i] = GetChipBodyH(zmapData, leadRectTemp, zmapSizeX, zmapSizeY, EXRTW, EXRTL, angle, solderStartPos);
					else
						avrH[i] = GetChipBodyH(m_pZmapBoundaryData, leadRectTemp, m_bdryW, m_bdryL, EXRTW, EXRTL, angle, solderStartPos);

					sumTemp += avrH[i];

					//test
					// 				UCHAR* test = new UCHAR[leadRectTemp.Width() * leadRectTemp.Height()];
					// 
					// 				CPoint ctTemp = leadRectTemp.CenterPoint();
					// 				m_procMil->TestImageSave(m_fovImageTop_insp, zmapSizeX, zmapSizeY, ctTemp.x, ctTemp.y, leadRectTemp.Width(), leadRectTemp.Height(), _T("test.bmp"));
					// 
					// 				delete test; 				
				}

				totalAvrH = (float)(sumTemp / leadCnt);
			}
		}


	//////////////////////////////////////////////////////////////////////////
	
		m_leadAvrH = totalAvrH;

		//huj 2013/11/29
		stdH_Max = (double)(stdLeadH + range_Hmax);

		if(inspHeightFlag == TRUE && isContainer == FALSE)
		{
			for(int i = 0; i < leadCnt; i++)//일단 들뜸만 추가...  (추가로 min값보다 작으면 브로큰???인지는 판단하여 나중에 추가...)
			{
				if(avrH[i] > stdH_Max)
				{
					leadRst[i] = 1;
					totalLeadRst = TRUE;
				}
				else
				{
					leadRst[i] = 0;
				}

			}
		}

		InitLeadHeightVal(leadCnt);
		memcpy_s(m_pLeadHeight, sizeof(float) * leadCnt, avrH, sizeof(float) * leadCnt);	

		InitLeadLiftVal(leadCnt);
		memcpy_s(m_pLeadLiftRst, sizeof(BOOL) * leadCnt, leadRst, sizeof(BOOL) * leadCnt);	

		/*delete avrH;
		delete leadRst;*/
		g_pMManager->pem_delete(avrH, false);
		g_pMManager->pem_delete(leadRst, false);
	}

	//////////////////////////////////////////////////////////////////////////





	if(totalLeadRst == TRUE || totalGapRst == TRUE)
		ret = e_NG;

	//////////////////////////////////////////////////////////////////////////

	m_inspResult.leadInfo = m_leadInfo;
	m_inspResult.leadAvrHeight = m_leadAvrH;

	
	

	return ret;
}


int CPInsp_LeadSolder::InspSolder()
{
	int ret = e_OK;

	int cx = m_inspCoordinate.bdrCx;
	int cy = m_inspCoordinate.bdrCy;
	int roiSizeX = m_inspCoordinate.bdrWidth;
	int roiSizeY = m_inspCoordinate.bdrLength;
	int solderCnt = m_solderCount;
	int leadPos = m_inspParam->leadPosition;
	BOOL isChipSolder = m_inspParam->isChipSolder;
	double wndAngle = m_inspCoordinate.angle;
	double stdLeadH = m_inspParam->stdLeadHeight;

	//shk 20140424 
	BOOL IsUse2D = m_inspParam->IsUse2D;

	int solderStartPos = m_inspParam->solderStartPos;
	int solderEndPos = m_inspParam->solderEndPos;
	int solderInspLength = solderEndPos - solderStartPos;
	int stdPixel = (int)(solderInspLength * _mm2micron(m_resolX)); 

	double stdMaxVolume = m_inspParam->range_maxVolume;
	double stdMinVolume = m_inspParam->range_minVolume;
	double stdVoidVolume = 10.0;
	if(stdVoidVolume >= stdMinVolume)
	{
		stdVoidVolume = stdMinVolume - 1;	
		stdVoidVolume = stdVoidVolume < 0?   0:stdVoidVolume;
	}

	int threshold = m_inspParam->threshold;
	double stdRate = m_inspParam->stdRate;	
	float fitR = m_inspParam->redFator;
	float fitG = m_inspParam->greenFator;
	float fitB = m_inspParam->blueFator;

	int stdSolderArea = m_inspParam->stdSolderArea;


	BOOL isContainer = m_inspParam->isContainer;
	BOOL inspSolderFlag = m_inspParam->inspSolderFlag;

	//huj 2014/01/07
	if(isContainer == TRUE) //container일 경우 bridge 검사만 한다.
		inspSolderFlag = FALSE;


	if(m_pZmapRoiData == NULL || m_pRoiImg_Tar ==NULL || m_pRoiImg_Top == NULL || m_pRoiImg_Bottom == NULL || m_pImage_HV == NULL || roiSizeX < 1 || roiSizeY < 1 || leadPos < 0 || leadPos > 3)
		return e_NG;

	if(wndAngle < 0)
		wndAngle += 360 ;

	if(wndAngle > 360)
		wndAngle -= 360;

	if(m_isAnyAngle)
		wndAngle = 0;

	int w = 0;
	int h = 0;	
	double angle = GetRotateAngle(leadPos, roiSizeX, roiSizeY, wndAngle, &w, &h);


	//////////////////////////////////////////////////////////////////////////
	//solder 검사

	int totalRst = FALSE;
	double* solderVolum = NULL;
	double* solderColorRate = NULL;
	int* filetLength = NULL;
	int* filetStart = NULL;
	float* solderRatio = NULL;
	int solderArea = 0;
	int blobCount = 0;

	//shk 20140318
	float* graysolderRatio = NULL;

	//shk 20140430
	float* resultSolderRatio = NULL;

	int* leadEnd = NULL;


	if(solderCnt > 0 && m_pSolderRect != NULL)
	{
		//solderVolum = new double[solderCnt];
		solderVolum = g_pMManager->pem_new<double>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(solderVolum, 0, sizeof(double) * solderCnt);

		// LMJ 2014/01/20
		//solderColorRate = new double[solderCnt];
		solderColorRate = g_pMManager->pem_new<double>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(solderColorRate, 0, sizeof(double) * solderCnt);

		//filetLength = new int[solderCnt];
		filetLength = g_pMManager->pem_new<int>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(filetLength, 0, sizeof(int) * solderCnt);

		//solderRatio = new float[solderCnt];
		solderRatio = g_pMManager->pem_new<float>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(solderRatio, 0, sizeof(float) * solderCnt);

		//shk 20140318
		//graysolderRatio = new float[solderCnt];
		graysolderRatio = g_pMManager->pem_new<float>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(graysolderRatio, 0, sizeof(float) * solderCnt);

		if(!isChipSolder && inspSolderFlag)
		{
			//filetStart = new int[solderCnt];
			filetStart = g_pMManager->pem_new<int>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
			memset(filetStart, 0, sizeof(int) * solderCnt);

			//leadEnd = new int[solderCnt];
			leadEnd = g_pMManager->pem_new<int>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
			memset(leadEnd, 0, sizeof(int) * solderCnt);
		}
	
				
		if(inspSolderFlag)
		{
			//m_procMil->InspSolderArea_S2(m_pZmapRoiData, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, isChipSolder, solderInspLength, leadEnd, filetStart, solderRatio);
			//m_procMil->InspSolderArea_S3(m_pZmapRoiData,m_pRoiImg_Top, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, isChipSolder, solderInspLength, leadEnd, filetStart, solderRatio,graysolderRatio);
			
			//shk 20140424 // if 2d opetion was checked ,we will use middle image.
			//m_procMil->InspSolderArea_S4(m_pZmapRoiData,/*m_pRoiImg_Mid*/m_pRoiImg_Tar, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, isChipSolder, IsUse2D, solderInspLength, leadEnd, filetStart, solderRatio,graysolderRatio);

			//shk 20140430
			m_pProcMilLeadSolder->InspSolderArea_S5(m_pZmapRoiData,m_pRoiImg_Top, w, h, solderStartPos,m_pSolderRect, solderCnt, (float)stdLeadH, isChipSolder, IsUse2D, solderInspLength, leadEnd, filetStart, solderRatio,graysolderRatio);

			if(m_inspParam->inspColorFlag == TRUE && solderCnt > 0)
			{	
				RemakeSolderRect(solderCnt, filetStart);
				AnalysisColorCIE(roiSizeX, roiSizeY, fitR, fitG, fitB, m_inspCoordinate, m_pSolderRect, solderCnt, isChipSolder, solderColorRate, m_pZmapRoiData, (float)(stdLeadH / 2.0));
			}


			if(!isChipSolder)
			{
				RemakeLeadRect(solderCnt, leadEnd);
				//delete leadEnd;
				g_pMManager->pem_delete(leadEnd, false);
			}
		}	

		InitSolderVolumeVal(solderCnt);
		memcpy_s(m_solderVolume, sizeof(double) * solderCnt, solderVolum, sizeof(double) * solderCnt);

		InitSolderColorRateVal(solderCnt);
		memcpy_s(m_solderColorRate, sizeof(double) * solderCnt, solderColorRate, sizeof(double) * solderCnt);

		InitFiletLengthVal(solderCnt);
		memcpy_s(m_pFiletLength, sizeof(int) * solderCnt, filetLength, sizeof(int) * solderCnt);

		InitSolderRatioVal(solderCnt);
		memcpy_s(m_solderRatio, sizeof(float) * solderCnt, solderRatio, sizeof(float) * solderCnt);

		InitgraySolderRatioVal(solderCnt);
		memcpy_s(m_graysolderRatio, sizeof(float) * solderCnt, graysolderRatio, sizeof(float) * solderCnt);

		/*delete solderVolum;	
		delete solderColorRate;
		delete filetLength;
		delete solderRatio;
		delete graysolderRatio;*/
		g_pMManager->pem_delete(solderVolum, false);
		g_pMManager->pem_delete(solderColorRate, false);
		g_pMManager->pem_delete(filetLength, false);
		g_pMManager->pem_delete(solderRatio, false);
		g_pMManager->pem_delete(graysolderRatio, false);

		if(filetStart != NULL)
		{
			//delete filetStart;
			g_pMManager->pem_delete(filetStart, false);
			filetStart = NULL;
		}
	}
	else
	{
		ret = e_NG;
	}
	//////////////////////////////////////////////////////////////////////////

	//shk 20140428
// 	if(IsUse2D)
// 	{
// 		memcpy_s(m_solderRatio, sizeof(float) * solderCnt, m_graysolderRatio, sizeof(float) * solderCnt);
// 		//m_solderRatio = m_graysolderRatio;
// 	}

	//shk 20140430
	//  Step 1. 3D : 2D 중 검사 결과 기준 큰값이 Min volume에 만족하면 Step2 진행, 아니면 NG
	//  Step 2. 2D / 3D < 70% 이면 NG
	if(inspSolderFlag)
	{
		if(solderCnt > 0 && m_pFiletLength != NULL)
		{	
			//shk 20140430
			//resultSolderRatio = new float[solderCnt];
			resultSolderRatio = g_pMManager->pem_new<float>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
			memset(resultSolderRatio, 0, sizeof(float) * solderCnt);

			InitSolderResultVal(solderCnt);
			InitSolderColorResultVal(solderCnt);

			for(int i = 0; i < solderCnt; i++)
			{
				stdMaxVolume = 1000000;  //사용 안함

				if (!IsUse2D) // Normal 검사(3D + 2D)
				{
					//  Step 1. 3D : 2D 중 검사 결과 기준 큰값이 Min volume에 만족하면 Step2 진행, 아니면 NG
					resultSolderRatio[i] =  __max(m_solderRatio[i],m_graysolderRatio[i]);

					if(resultSolderRatio[i]  >= stdMaxVolume)
					{
						m_solderVolRst[i] = eSolder_Excessive; //과납
						ret = e_NG;
					}					
					else if(resultSolderRatio[i] <= stdMinVolume && resultSolderRatio[i] >= stdVoidVolume)
					{
						m_solderVolRst[i] = eSolder_Insufficient; //소납
						ret = e_NG;
					}
					else if(resultSolderRatio[i] < stdVoidVolume)
					{
						m_solderVolRst[i] = eSolder_Void; //미납
						ret = e_NG;
					}
					else
					{
						//m_solderVolRst[i] = eSolder_OK; //정상
						//  Step 2. 2D / 3D < 70% 이면 NG
						if( (int)m_graysolderRatio[i] != 0 && (int)m_solderRatio[i] != 0 )
						{
							if (m_graysolderRatio[i] >=  m_solderRatio[i])
							{
								resultSolderRatio[i] = 100.0f;
							}
							else
							{
								resultSolderRatio[i] = (m_graysolderRatio[i] /  m_solderRatio[i]);
								resultSolderRatio[i] = resultSolderRatio[i] * 100;
							}
						}
						else if( (int)m_graysolderRatio[i] == 0 && (int)m_solderRatio[i] != 0 )
						{
							resultSolderRatio[i] =  m_solderRatio[i];
						}
						else if( (int)m_graysolderRatio[i] != 0 && (int)m_solderRatio[i] == 0 )
						{
							resultSolderRatio[i] = m_graysolderRatio[i] ;
						}
						else
						{
							m_solderVolRst[i] = eSolder_Void; //미납
							ret = e_NG;
						}

						if( (int)m_graysolderRatio[i] != 0 || (int)m_solderRatio[i] != 0 )
						{
							if(resultSolderRatio[i] < COLD_VOL)
							{
								resultSolderRatio[i] = -resultSolderRatio[i];
								m_solderVolRst[i] = eSolder_Void; //미납
								ret = e_NG;
							}
							else
							{
								m_solderVolRst[i] = eSolder_OK; //정상
							}
						}
					}

				}
				else //3D only 검사일때
				{
					resultSolderRatio[i] = m_solderRatio[i];

					if(resultSolderRatio[i]  >= stdMaxVolume)
					{
						m_solderVolRst[i] = eSolder_Excessive; //과납
						ret = e_NG;
					}					
					else if(resultSolderRatio[i] <= stdMinVolume && resultSolderRatio[i] >= stdVoidVolume)
					{
						m_solderVolRst[i] = eSolder_Insufficient; //소납
						ret = e_NG;
					}
					else if(resultSolderRatio[i] < stdVoidVolume)
					{
						m_solderVolRst[i] = eSolder_Void; //미납
						ret = e_NG;
					}
					else
					{
						m_solderVolRst[i] = eSolder_OK; //정상
					}
				}

				//////////////////////////////////////////////////////////////////////////
				if(m_inspParam->inspColorFlag)
				{
					 if(m_solderColorRate[i] > stdRate)
					 {
					 	m_solderColorRst[i] = eSolder_Insufficient;
					 	ret = e_NG;
					 }
				}
			}

			m_inspResult.volumeRst = m_solderVolRst;
			m_inspResult.solderColorRst = m_solderColorRst;
		}
		else
		{
			ret = e_NG;
		}	
	}
	else
	{
		if(solderCnt > 0)
		{
			InitSolderResultVal(solderCnt);
			m_inspResult.volumeRst = m_solderVolRst;

			InitSolderColorResultVal(solderCnt);
			m_inspResult.solderColorRst = m_solderColorRst;
		}
	}

	m_inspResult.solderVolume = m_solderVolume;
	m_inspResult.filetLength = m_pFiletLength;
	m_inspResult.solderColorRate = m_solderColorRate;

	if(resultSolderRatio != NULL)
	{		
		memcpy_s(m_solderRatio, sizeof(float) * solderCnt, resultSolderRatio, sizeof(float) * solderCnt);
		m_inspResult.solderRatio = m_solderRatio;
		
	}
	

	m_inspResult.solderCnt = solderCnt;
	m_inspResult.rstSolderArea = solderArea;

	if(resultSolderRatio != NULL)
	{
		//delete resultSolderRatio;
		g_pMManager->pem_delete(resultSolderRatio, false);
	}


	return ret;
}



// int CPInsp_LeadSolder::InspSolder()
// {
// 	int ret = e_OK;
// 
// 	int cx = m_inspCoordinate.bdrCx;
// 	int cy = m_inspCoordinate.bdrCy;
// 	int roiSizeX = m_inspCoordinate.bdrWidth;
// 	int roiSizeY = m_inspCoordinate.bdrLength;
// 	int solderCnt = m_solderCount;
// 	int leadPos = m_inspParam->leadPosition;
// 	BOOL isChipSolder = m_inspParam->isChipSolder;
// 	double wndAngle = m_inspCoordinate.angle;
// 	double stdLeadH = m_inspParam->stdLeadHeight;
// 
// 	int solderStartPos = m_inspParam->solderStartPos;
// 	int solderEndPos = m_inspParam->solderEndPos;
// 	int solderInspLength = solderEndPos - solderStartPos;
// 	int stdPixel = (int)(solderInspLength * _mm2micron(m_resolX)); 
// 
// 	double stdMaxVolume = m_inspParam->range_maxVolume;
// 	double stdMinVolume = m_inspParam->range_minVolume;
// 	double stdVoidVolume = 10.0;
// 	if(stdVoidVolume >= stdMinVolume)
// 	{
// 		stdVoidVolume = stdMinVolume - 1;	
// 		stdVoidVolume = stdVoidVolume < 0?   0:stdVoidVolume;
// 	}
// 
// 	int threshold = m_inspParam->threshold;
// 	double stdRate = m_inspParam->stdRate;	
// 	float fitR = m_inspParam->redFator;
// 	float fitG = m_inspParam->greenFator;
// 	float fitB = m_inspParam->blueFator;
// 
// 	int stdSolderArea = m_inspParam->stdSolderArea;
// 
// 
// 	BOOL isContainer = m_inspParam->isContainer;
// 	BOOL inspSolderFlag = m_inspParam->inspSolderFlag;
// 
// 	//huj 2014/01/07
// 	if(isContainer == TRUE) //container일 경우 bridge 검사만 한다.
// 		inspSolderFlag = FALSE;
// 
// 
// 	if(m_pZmapRoiData == NULL || m_pRoiImg_Top == NULL || m_pRoiImg_Bottom == NULL || m_pImage_HV == NULL || roiSizeX < 1 || roiSizeY < 1 || leadPos < 0 || leadPos > 3)
// 		return e_NG;
// 
// 	if(wndAngle < 0)
// 		wndAngle += 360 ;
// 
// 	if(wndAngle > 360)
// 		wndAngle -= 360;
// 
// 	if(m_isAnyAngle)
// 		wndAngle = 0;
// 
// 	int w = 0;
// 	int h = 0;	
// 	double angle = GetRotateAngle(leadPos, roiSizeX, roiSizeY, wndAngle, &w, &h);
// 
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	//solder 검사
// 
// 	int totalRst = FALSE;
// 	double* solderVolum = NULL;
// 	double* solderColorRate = NULL;
// 	int* filetLength = NULL;
// 	int* filetStart = NULL;
// 	float* solderRatio = NULL;
// 	int solderArea = 0;
// 	int blobCount = 0;
// 
// 	int* leadEnd = NULL;
// 
// 
// 	if(solderCnt > 0 && m_pSolderRect != NULL)
// 	{
// 		solderVolum = new double[solderCnt];
// 		memset(solderVolum, 0, sizeof(double) * solderCnt);
// 
// 		// LMJ 2014/01/20
// 		solderColorRate = new double[solderCnt];
// 		memset(solderColorRate, 0, sizeof(double) * solderCnt);
// 
// 		filetLength = new int[solderCnt];
// 		memset(filetLength, 0, sizeof(int) * solderCnt);
// 
// 		solderRatio = new float[solderCnt];
// 		memset(solderRatio, 0, sizeof(float) * solderCnt);
// 
// 		if(!isChipSolder && inspSolderFlag)
// 		{
// 			filetStart = new int[solderCnt];
// 			memset(filetStart, 0, sizeof(int) * solderCnt);
// 		}
// 
// 		if(!isChipSolder && inspSolderFlag)
// 		{
// 			leadEnd = new int[solderCnt];
// 			memset(leadEnd, 0, sizeof(int) * solderCnt);
// 		}		
// 		//totalRst = m_procMil->InspSolderArea(m_pZmapRoiData, m_pRoiImg_Top, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, solderStartPos, solderEndPos, isChipSolder, filetLength, filetHeight, solderVolum, endLineOver);
// 		
// 		if(inspSolderFlag)
// 		{
// 			// LMJ 2014/01/20
// 			//m_procMil->InspSolderArea2(m_pZmapRoiData, m_pRoiImg_Top, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, isChipSolder, filetLength, filetStart, solderVolum);
// 			//m_procMil->InspSolderArea3(m_pZmapRoiData, m_pRoiImg_Top, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, isChipSolder, solderInspLength, filetStart, solderVolum, solderRatio);
// 			
// 			//송하전자 버전 (InspSolderArea4)
// 			//m_procMil->InspSolderArea4(m_pZmapRoiData, m_pRoiImg_Top, m_pRoiImg_Mid, m_pRoiImg_Bottom, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, stdSolderArea, isChipSolder, solderEndPos, SOLDERW_MARGIN, true, filetStart, &solderArea, &blobCount);
// 
// 			//m_procMil->InspSolderArea_S1(m_pZmapRoiData, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, isChipSolder, solderInspLength,leadEnd, filetStart, solderRatio);
// 			m_procMil->InspSolderArea_S2(m_pZmapRoiData, w, h, m_pSolderRect, solderCnt, (float)stdLeadH, isChipSolder, solderInspLength, leadEnd, filetStart, solderRatio);
// 			//huj 2014/01/06
// 			if(m_inspParam->inspColorFlag == TRUE && solderCnt > 0)
// 			{	
// 				RemakeSolderRect(solderCnt, filetStart);
// 				AnalysisColorCIE(roiSizeX, roiSizeY, fitR, fitG, fitB, m_inspCoordinate, m_pSolderRect, solderCnt, isChipSolder, solderColorRate, m_pZmapRoiData, (float)(stdLeadH / 2.0));
// 			}
// 		}
// 
// 		if(!isChipSolder && inspSolderFlag)
// 		{
// 			//RemakeLeadRect(solderCnt, filetStart);
// 			RemakeLeadRect(solderCnt, leadEnd);
// 			delete leadEnd;
// 
// 		}
// 
// 		InitSolderVolumeVal(solderCnt);
// 		memcpy_s(m_solderVolume, sizeof(double) * solderCnt, solderVolum, sizeof(double) * solderCnt);
// 
// 		InitSolderColorRateVal(solderCnt);
// 		memcpy_s(m_solderColorRate, sizeof(double) * solderCnt, solderColorRate, sizeof(double) * solderCnt);
// 
// 		InitFiletLengthVal(solderCnt);
// 		memcpy_s(m_pFiletLength, sizeof(int) * solderCnt, filetLength, sizeof(int) * solderCnt);
// 
// 		InitSolderRatioVal(solderCnt);
// 		memcpy_s(m_solderRatio, sizeof(float) * solderCnt, solderRatio, sizeof(float) * solderCnt);
// 
// // 		InitSolderStartRstVal(solderCnt);
// // 		memcpy_s(m_pRstSolderStartPos, sizeof(int) * solderCnt, filetStart, sizeof(int) * solderCnt);
// 
// 		delete solderVolum;	
// 		delete solderColorRate;
// 		delete filetLength;
// 		delete solderRatio;
// 
// 		if(filetStart != NULL)
// 		{
// 			delete filetStart;
// 			filetStart = NULL;
// 		}
// 	}
// 	else
// 	{
// 		ret = e_NG;
// 	}
// 	//////////////////////////////////////////////////////////////////////////
// 
// 
// 
// 
// 	if(inspSolderFlag)
// 	{
// 		if(solderCnt > 0 && m_pFiletLength != NULL)
// 		{			
// 			InitSolderResultVal(solderCnt);
// 			InitSolderColorResultVal(solderCnt);
// 
// 			for(int i = 0; i < solderCnt; i++)
// 			{
// // 				if(m_pFiletLength[i] * _mm2micron(m_resolX) < stdPixel) //l이 5pixel보다 작으면 소납
// // 				{
// // 					m_solderVolRst[i] = eSolder_Insufficient;
// // 					ret = e_NG;
// // 				}
// 
// //huj
// /*				double ratio = (double)solderArea / (double)stdSolderArea * 100.0;
// 				if(ratio < stdMinVolume)
// 				{
// 					m_solderVolRst[i] = eSolder_Insufficient; //소납
// 					ret = e_NG;
// 				}
// 				// 				else if(ratio > stdMaxVolume)
// 				// 				{
// 				// 					m_solderVolRst[i] = eSolder_Cold; //냉납
// 				// 					ret = e_NG;
// 				// 				}
// 
// 				// 				if(blobCount > 1)
// 				// 				{
// 				// 					m_solderVolRst[i] = eSolder_Cold; //냉납
// 				// 					ret = e_NG;
// 				// 				}
// */
// 
// 				//////////////////////////////////////////////////////////////////////////
// 
// 
// //shk
// 				stdMaxVolume = 1000000;  //사용 안함
// 				if(m_solderRatio[i]  >= stdMaxVolume)
// 				{
// 					m_solderVolRst[i] = eSolder_Excessive; //과납
// 					ret = e_NG;
// 				}					
// 				else if(m_solderRatio[i] <= stdMinVolume && m_solderRatio[i] >= stdVoidVolume)
// 				{
// 					m_solderVolRst[i] = eSolder_Insufficient; //소납
// 					ret = e_NG;
// 				}
// 				else if(m_solderRatio[i] < stdVoidVolume)
// 				{
// 					m_solderVolRst[i] = eSolder_Void; //미납
// 					ret = e_NG;
// 				}
// 				else
// 				{
// 					m_solderVolRst[i] = eSolder_OK; //정상
// 				}
// 
// 				//////////////////////////////////////////////////////////////////////////
// 				if(m_inspParam->inspColorFlag)
// 				{
// 					 if(m_solderColorRate[i] > stdRate)
// 					 {
// 					 	m_solderColorRst[i] = eSolder_Insufficient;
// 					 	ret = e_NG;
// 					 }
// 				}
// 			}
// 
// 			m_inspResult.volumeRst = m_solderVolRst;
// 			m_inspResult.solderColorRst = m_solderColorRst;
// 		}
// 		else
// 		{
// 			ret = e_NG;
// 		}	
// 	}
// 	else
// 	{
// 		if(solderCnt > 0)
// 		{
// 			InitSolderResultVal(solderCnt);
// 			m_inspResult.volumeRst = m_solderVolRst;
// 
// 			InitSolderColorResultVal(solderCnt);
// 			m_inspResult.solderColorRst = m_solderColorRst;
// 		}
// 	}
// 
// 
// 
// 	m_inspResult.solderVolume = m_solderVolume;
// 	m_inspResult.filetLength = m_pFiletLength;
// 	m_inspResult.solderColorRate = m_solderColorRate;
// 	m_inspResult.solderRatio = m_solderRatio;
// 	m_inspResult.solderCnt = solderCnt;
// 	m_inspResult.rstSolderArea = solderArea;
// 
// 	return ret;
// }

// LMJ 2014/01/03	: CIE 소납 검사
int CPInsp_LeadSolder::AnalysisColorCIE(int roiSizeX, int roiSizeY, float fitR, float fitG, float fitB, Coordinate coordi, CRect* solderRect, int solderCnt, int isChipsolder, double* retRate, float * zmap, float limitHeight)
{
	CPInsp_Color inspColor;
	int ret = e_OK;

	int roiArea = roiSizeX * roiSizeY ;

	UCHAR* fovImgTopRTemp = NULL;
	UCHAR* fovImgTopGTemp = NULL;
	UCHAR* fovImgTopBTemp = NULL;

	/*fovImgTopRTemp = new UCHAR[roiArea];
	fovImgTopGTemp = new UCHAR[roiArea];
	fovImgTopBTemp = new UCHAR[roiArea];*/
	fovImgTopRTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	fovImgTopGTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	fovImgTopBTemp = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	MakeProcImg(m_fovImageTop_R, m_fovImageTop_G, m_fovImageTop_B, fovImgTopRTemp, fovImgTopGTemp, fovImgTopBTemp, coordi);
	// 	m_procMil->SaveWorkImg(fovImgTopRTemp, roiSizeX, roiSizeY, _T("colorTest_R.bmp"));
	// 	m_procMil->SaveWorkImg(fovImgTopGTemp, roiSizeX, roiSizeY, _T("colorTest_G.bmp"));
	// 	m_procMil->SaveWorkImg(fovImgTopBTemp, roiSizeX, roiSizeY, _T("colorTest_B.bmp"));
	//bottom 위치가 아닐경우 bottom위치로 회전.
	int w = 0;
	int h = 0;	
	double angle = m_wndAngle;

	UCHAR * pRoiImg_Top_R = NULL; //new UCHAR[roiArea];
	m_pProcMilLeadSolder->RotateImg_ipp(fovImgTopRTemp, roiSizeX, roiSizeY, angle, &pRoiImg_Top_R, &w, & h);
	//delete [] fovImgTopRTemp;
	g_pMManager->pem_delete(fovImgTopRTemp, true);

	UCHAR * pRoiImg_Top_G = NULL; //new UCHAR[roiArea];
	m_pProcMilLeadSolder->RotateImg_ipp(fovImgTopGTemp, roiSizeX, roiSizeY, angle, &pRoiImg_Top_G, &w, & h);
	//delete [] fovImgTopGTemp;
	g_pMManager->pem_delete(fovImgTopGTemp, true);

	UCHAR * pRoiImg_Top_B = NULL; //new UCHAR[roiArea];
	m_pProcMilLeadSolder->RotateImg_ipp(fovImgTopBTemp, roiSizeX, roiSizeY, angle, &pRoiImg_Top_B, &w, & h);
	//delete [] fovImgTopBTemp;
	g_pMManager->pem_delete(fovImgTopBTemp, true);

	UCHAR* img_R = NULL;
	UCHAR* img_G = NULL;
	UCHAR* img_B = NULL;
	int width, length, cx, cy;

	double rate = 0.0;

	for(int i = 0; i < solderCnt; i++)
	{
		if(!isChipsolder)		// 칩솔더가 아닌 경우 솔더 영역만 대상
		{
			cx = solderRect[i].CenterPoint().x;
			cy = solderRect[i].CenterPoint().y;

			width = solderRect[i].Width();
			length = solderRect[i].Height();

			/*img_R = new UCHAR[width * length];
			img_G = new UCHAR[width * length];
			img_B = new UCHAR[width * length];*/
			img_R = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
			img_G = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
			img_B = g_pMManager->pem_new<UCHAR>(true, width * length, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilLeadSolder->GetClipImage(pRoiImg_Top_R, w, h, img_R, solderRect[i].CenterPoint().x, solderRect[i].CenterPoint().y, width, length);
			m_pProcMilLeadSolder->GetClipImage(pRoiImg_Top_G, w, h, img_G, solderRect[i].CenterPoint().x, solderRect[i].CenterPoint().y, width, length);
			m_pProcMilLeadSolder->GetClipImage(pRoiImg_Top_B, w, h, img_B, solderRect[i].CenterPoint().x, solderRect[i].CenterPoint().y, width, length);

			// 			m_procMil->SaveWorkImg(img_R, width, length, _T("colorTestLead_R.bmp"));
			// 			m_procMil->SaveWorkImg(img_G, width, length, _T("colorTestLead_G.bmp"));
			// 			m_procMil->SaveWorkImg(img_B, width, length, _T("colorTestLaed_B.bmp"));
			int nCntValid = 0;
			rate = inspColor.CompareCIE(img_R, img_G, img_B, fitR, fitG, fitB, g_lutData_color, width, length, nCntValid); //shk 2014/01/13
		}
		else		// 칩솔더인 경우 윈도우 ROI 영역 전체 대상
		{
			img_R = pRoiImg_Top_R;
			img_G = pRoiImg_Top_G;
			img_B = pRoiImg_Top_B;

			width = w;
			length = h;
			rate = inspColor.CompareCIE(img_R, img_G, img_B, fitR, fitG, fitB, g_lutData_color, width, length, zmap, limitHeight); //shk 2014/01/13
		}
		//m_procMil->SaveWorkImg(img_B, width, length, _T("COLOR_Clip.bmp"));

		if(retRate != NULL)
		{
			retRate[i] = rate;
		}

		if(!isChipsolder)
		{
			/*delete [] img_R;
			delete [] img_G;
			delete [] img_B;*/
			g_pMManager->pem_delete(img_R, true);
			g_pMManager->pem_delete(img_G, true);
			g_pMManager->pem_delete(img_B, true);
		}
	}

	/*delete [] pRoiImg_Top_R;
	delete [] pRoiImg_Top_G;
	delete [] pRoiImg_Top_B;*/
	g_pMManager->pem_delete(pRoiImg_Top_R, true);
	g_pMManager->pem_delete(pRoiImg_Top_G, true);
	g_pMManager->pem_delete(pRoiImg_Top_B, true);

	return ret;
}

// int CPInsp_LeadSolder::InspSolder()
// {
// 	int ret = e_OK;
// 
// 	int cx = m_inspCoordinate.bdrCx;
// 	int cy = m_inspCoordinate.bdrCy;
// 	int roiSizeX = m_inspCoordinate.bdrWidth;
// 	int roiSizeY = m_inspCoordinate.bdrLength;
// 	int solderCnt = m_solderCount;
// 	int leadPos = m_inspParam->leadPosition;
// 	BOOL isChipSolder = m_inspParam->isChipSolder;
// 	double wndAngle = m_inspCoordinate.angle;
// 
// 	double stdMaxVolume = m_inspParam->range_maxVolume;
// 	double stdMinVolume = m_inspParam->range_minVolume;
// 
// 	if(m_pZmapRoiData == NULL || m_pRoiImg_Top == NULL || m_pRoiImg_Bottom == NULL || m_pImage_HV == NULL || roiSizeX < 1 || roiSizeY < 1 || leadPos < 0 || leadPos > 3)
// 		return e_NG;
// 
// 	if(wndAngle < 0)
// 		wndAngle = 0;
// 
// 	if(wndAngle > 360)
// 		wndAngle = 0;
// 
// 	int w = 0;
// 	int h = 0;	
// 	double angle = GetRotateAngle(leadPos, roiSizeX, roiSizeY, wndAngle, &w, &h);
// 	
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	//solder 검사
// 
// 	int totalRst = FALSE;
// 	if(solderCnt > 0 && m_pSolderRect != NULL)
// 	{
// 		double* solderVolum = new double[solderCnt];
// 		memset(solderVolum, 0, sizeof(double) * solderCnt);
// 
// 		for(int i = 0; i < solderCnt; i++)
// 		{
// 			solderVolum[i] = Insp_CalcSolderArea(m_pRoiImg_Top, w, h, m_pSolderRect[i], isChipSolder);
// 		}
// 
// 		InitSolderVolumeVal(solderCnt);
// 		memcpy_s(m_solderVolume, sizeof(double) * solderCnt, solderVolum, sizeof(double) * solderCnt);	
// 
// 
// 		delete solderVolum;
// 	}
// 	else
// 	{
// 		ret = e_NG;
// 	}
// 	//////////////////////////////////////////////////////////////////////////
// 
// 
// 
// 	if(solderCnt > 0)
// 	{	
// 		m_inspResult.volumeRst = m_solderVolRst;
// 		if(totalRst == FALSE)
// 			ret = e_NG;
// 	}
// 	else
// 	{
// 		ret = e_NG;
// 	}
// 
// 
// 	int* solderVolumeRst = NULL;	
// 	if(solderCnt > 0 && m_solderVolume != NULL)
// 	{
// 		solderVolumeRst = new int[solderCnt];
// 		memset(solderVolumeRst, 0, sizeof(int) * solderCnt);
// 
// 		for(int i = 0; i < solderCnt; i++)
// 		{
// 			if(m_solderVolume[i] < stdMaxVolume) //ratio
// 			{		
// 				ret = e_NG;
// 				solderVolumeRst[i] = eSolder_Insufficient;
// 			}
// 			else
// 			{
// 				solderVolumeRst[i] = eSolder_OK;
// 			}
// 			/*
// 			if(m_solderVolume[i] > stdMaxVolume) //ratio
// 			{		
// 				ret = e_NG;
// 				solderVolumeRst[i] = eSolder_Excessive;
// 			}
// 			else if(m_solderVolume[i] < stdMinVolume) //height
// 			{		
// 				ret = e_NG;
// 				solderVolumeRst[i] = eSolder_Insufficient;				
// 			}
// 			else
// 			{
// 				solderVolumeRst[i] = eSolder_OK;
// 			}
// 			*/
// 		}
// 		InitSolderResultVal(solderCnt);
// 		memcpy_s(m_solderVolRst, sizeof(int) * solderCnt, solderVolumeRst, sizeof(int) * solderCnt);
// 		m_inspResult.volumeRst = m_solderVolRst;
// 
// 		delete solderVolumeRst;
// 	}
// 	else
// 	{
// 		ret = e_NG;
// 	}
// 
// 
// 	m_inspResult.solderVolume = m_solderVolume;
// 	m_inspResult.solderCnt = solderCnt;
// 
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	return ret;
// }





int CPInsp_LeadSolder::DecisionSolderInsp()
{
	int ret = e_OK;

	BOOL isContainer = m_inspParam->isContainer;
	//////////////////////////////////////////////////////////////////////////
	//solder
	int solderCnt = m_solderCount;


	if(m_inspParam->solderFlag == TRUE && isContainer == FALSE)
	{
		if(solderCnt > 0 && m_solderColorRate != NULL)
		{
			for(int i = 0; i < solderCnt; i++)
			{
// 				if(m_solderVolRst[i] == eSolder_Excessive)
// 				{
// 					ret = e_NG;
// 					m_inspResult.ok = FALSE;
// 					m_inspResult.list.solderFilletOk = FALSE;
// 				}
// 				else if(m_solderVolRst[i] == eSolder_Insufficient)
// 				{
// 					ret = e_NG;
// 					m_inspResult.ok = FALSE;
// 					m_inspResult.list.solderFilletOk = FALSE;
// 				}	
// 				else if(m_solderVolRst[i] == eSolder_Void)
// 				{
// 					ret = e_NG;
// 					m_inspResult.ok = FALSE;
// 					m_inspResult.list.solderFilletOk = FALSE;
// 				}	

				if(m_solderVolRst[i] != eSolder_OK)
				{
					ret = e_NG;
					m_inspResult.ok = FALSE;
					m_inspResult.list.solderFilletOk = FALSE;
				}

				if(m_solderColorRst[i] == eSolder_Insufficient)
				{
					ret = e_NG;
					m_inspResult.ok = FALSE;
					m_inspResult.list.solderColorOk = FALSE;
				}
			}
	
		}
		else
		{
			ret = e_NG;
			m_inspResult.ok = FALSE;
			m_inspResult.list.solderFilletOk = FALSE;
		}
	}

	m_inspResult.solderColorRate = m_solderColorRate;
	m_inspResult.solderCnt = solderCnt;

	//////////////////////////////////////////////////////////////////////////
	//lead
	if(m_inspParam->isChipSolder == FALSE && m_inspParam->leadFlag == TRUE)
	{

		int stdLeadCount = m_inspParam->stdLeadCount;

		//huj 2014/01/06
// 		if(m_inspResult.leadInfo.leadCounts != stdLeadCount)
// 		{
// 			ret = e_NG;
// 			m_inspResult.list.leadCountOk = FALSE;
// 		}

		for(int i = 0; i < m_inspResult.leadInfo.leadCounts; i++)
		{
			if(isContainer == FALSE)
			{
				if(m_pLeadLiftRst != NULL)
				{
					if(m_pLeadLiftRst[i] > 0)
					{
						ret = e_NG;
						m_inspResult.list.leadLiftOk = FALSE;
					}
				}
			}


			if(isContainer == TRUE)
			{
				if(i < m_inspResult.leadInfo.leadCounts - 1 && m_pLeadBridgeRst != NULL)
				{
					if(m_pLeadBridgeRst[i] > 0)
					{
						ret = e_NG;
						m_inspResult.list.leadBridgeOk = FALSE;
					}
				}
			}
		}


		m_inspResult.leadHeight = m_pLeadHeight;
		m_inspResult.leadBridgeRst = m_pLeadBridgeRst;
		m_inspResult.leadLiftRst = m_pLeadLiftRst;

	}	

	
	if(ret != e_NG)
		m_inspResult.ok = TRUE;
	else
		m_inspResult.ok = FALSE;
	
	

	m_inspResult.leadFlag = m_inspParam->leadFlag;
	m_inspResult.solderFlag = m_inspParam->solderFlag;

	//////////////////////////////////////////////////////////////////////////
	return ret;
}

// 20140917 SHW : 수정
void CPInsp_LeadSolder::GetBinaryImg(UCHAR* ptrucImg2D, int nSizeX, int nSizeY, UCHAR* ptrucDst, int nThreshold)
{
	if((ptrucImg2D == NULL) || (nSizeX < 0) || (nSizeY < 0))
		return;

	int nRoiSize = nSizeX * nSizeY; 
	UCHAR* ptrucTempTop = NULL;
	
	//ptrucTempTop = new UCHAR[nRoiSize];
	ptrucTempTop = g_pMManager->pem_new<UCHAR>(true, nRoiSize, (PCHAR)__FUNCTION__, __LINE__);
	
	m_pProcMilAlgo->GetBinaryImg_ipp(ptrucImg2D, nSizeX, nSizeY, ptrucTempTop, nThreshold, 0);

	Im::PIL_ID milId2D = M_NULL;
	Im::PIL_ID milIdDst = M_NULL;
	milIdDst = m_pProcMilAlgo->AllocBuff(nSizeX, nSizeY, 0);

	milId2D = m_pProcMilAlgo->AllocBuff(nSizeX, nSizeY);
	Im::Buf::Put(milId2D, ptrucTempTop);
	//delete ptrucTempTop;
	g_pMManager->pem_delete(ptrucTempTop, true);

	m_pProcMilAlgo->SaveWorkImg(milId2D, _T("mill_Image2D.bmp"));
	
	//	ArithImage(milBottom, milTop, milDst, M_SUB  + M_SATURATION);
	m_pProcMilAlgo->ArithImage(milId2D, milIdDst, milIdDst, M_ADD+ M_SATURATION);
	
	Im::Buf::Get(milIdDst, ptrucDst);

	m_pProcMilAlgo->FreeMilImageBuff(&milId2D);
	m_pProcMilAlgo->FreeMilImageBuff(&milIdDst);
}

bool CPInsp_LeadSolder::GetLeadRect( InspAlgo &sInspAlgo, CRect* pucrcLeadRect, CRect* pucrcGapRect )
{
	bool bResult = FALSE;

	AlgoLeadSearch *pInspAlgoSearch = (AlgoLeadSearch *)sInspAlgo.m_ptrInspAlgoParam;
	if(!pInspAlgoSearch)
		return bResult;

	if(pucrcLeadRect == NULL)
		return bResult;
	
	if((!m_solderCount) || (!m_pLeadRect))
		return bResult;

	for (int i=0; i < m_solderCount; i++)
	{
		pucrcLeadRect[i].left = m_pLeadRect[i].left;
		pucrcLeadRect[i].right = m_pLeadRect[i].right;
		pucrcLeadRect[i].top = 0;
		if(pInspAlgoSearch->m_bIsManualSearch == TRUE)
			pucrcLeadRect[i].bottom = m_pLeadRect[i].bottom;
		else
			pucrcLeadRect[i].bottom = m_pSolderRect[i].bottom;

		if(pucrcGapRect != NULL)
		{
			if(i < m_solderCount -1)
			{
				pucrcGapRect[i].left = m_pGapRect[i].left;
				pucrcGapRect[i].right = m_pGapRect[i].right;
				pucrcGapRect[i].top = m_pGapRect[i].top;
				pucrcGapRect[i].bottom = m_pGapRect[i].bottom;
			}
		}
	}

	return bResult = true;
}

// 20140917 SHW : leadsearch 수정
bool CPInsp_LeadSolder::LeadSolderTeach(const AlgoLeadSearch *pInspAlgoSearch, WndAlgoImg &sWndAlgoImg, UCHAR* pUcArrRetResultImg, LeadInfo* ptrResultLeadInfo, int &nLeadCnt, CRect* pArrLeadRect, BOOL bTabSearch, double dLeadStartPos)
{
	bool bResult = FALSE;

	if(!pInspAlgoSearch)
		return bResult;

	UCHAR *pUcImgSrc	= sWndAlgoImg.m_ucArr2D;
	float* pfImgSrc = sWndAlgoImg.m_fArr3D;
	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	double dWndAngle = sWndAlgoImg.dAngle;
	if((pUcImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidth <= 0) || (nImgHeight <= 0))
		return bResult;
	int nROIArea = nImgWidth * nImgHeight * 3;

	int nLeadPosition = pInspAlgoSearch->nLeadPosition;  //0:left, 1:right, 2:top, 3:bottom
	int nSolderStartPos = pInspAlgoSearch->nSolderStartPos;
	if((nLeadPosition < 0) || (nLeadPosition > 3) || (nSolderStartPos < 0))
		return bResult;
	int nSearchWidth = nImgWidth;
	if((nLeadPosition == 0) || (nLeadPosition == 1))
		nSearchWidth = nImgHeight;

	BOOL bIsContainer = pInspAlgoSearch->bIsContainer;

	int nThresholdValue = pInspAlgoSearch->nThresholdValue;
	if(nThresholdValue > 255)			nThresholdValue = 255;
	else if(nThresholdValue < 0)		nThresholdValue = 0;

	BOOL bUse3D = pInspAlgoSearch->bUse3D;
	BOOL bUse2D = pInspAlgoSearch->bUse2D;

	BOOL bInspHeightFlag = pInspAlgoSearch->bInspHeightFlag;

	BOOL bIsManual = pInspAlgoSearch->m_bIsManualSearch;
	BOOL bSameWidthPitch = pInspAlgoSearch->m_bSameWidthPitch;
	BOOL bGerber = pInspAlgoSearch->m_bUseGerber;
	BOOL bWideSize = pInspAlgoSearch->m_bWideSize;

	int nPadCnt = pInspAlgoSearch->m_nPadCnt;
	int nSolderCnt = pInspAlgoSearch->nSolderCnt;
	int nLeadWidth = RounD(_mm2pixel_x(pInspAlgoSearch->m_dLeadWidth));
	double dLeadPitch_mm = pInspAlgoSearch->m_dLeadPitch;
	//int nLeadPitch = RounD(_mm2pixel_x(pInspAlgoSearch->m_dLeadPitch));

	int nStartX = pInspAlgoSearch->m_poStartLead.x;
	//////////////////////////////////////////////////////////////////////////
	//bottom 위치가 아닐경우 bottom위치로 회전.
	int nWidth = 0;
	int nHeight = 0;	
	double dAngleValue = GetRotateAngle(nLeadPosition, nImgWidth, nImgHeight, dWndAngle, &nWidth, &nHeight);
	if((nWidth <= 0) || (nHeight <= 0))
		return bResult;

	int nSizeX = nWidth;//pInspAlgoSearch->nDisplayPannelSizeX;
	int nSizeY = nHeight;//pInspAlgoSearch->nDisplayPannelSizeY;
	if((nSizeX <= 0) || (nSizeY <= 0))
		return bResult;
	/*if((nSizeX % 4) != 0)
		nSizeX = nSizeX + (4 - (nSizeX % 4));*/

	CAtlArray<CRect> *pRcArrSearchTab = (!bIsManual && bTabSearch) ? &rcArrSearchTab : NULL;
	if(pRcArrSearchTab)
		pRcArrSearchTab->RemoveAll();

	//2D image
	UCHAR* ptrucROIImg2D = NULL;
	//ptrucROIImg2D = new UCHAR[nROIArea];
	m_pProcMilLeadSolder->RotateImg_ipp(pUcImgSrc, nImgWidth, nImgHeight, dAngleValue, &ptrucROIImg2D);	// LMJ 2013/11/25
	m_pProcMilLeadSolder->SaveWorkImg(pUcImgSrc, nImgWidth, nImgHeight, _T("Img2D_1.bmp"));
	m_pProcMilLeadSolder->SaveWorkImg(ptrucROIImg2D, nWidth, nHeight, _T("Img2D_2.bmp"));
	//////////////////////////////////////////////////////////////////////////
	//lead & solder 영역 계산
	if(bIsManual == TRUE)
	{
		if(nSolderCnt < 1)
			bResult = FALSE;

		InitLeadRect(nSolderCnt);
		int nIndex = 0;
		LONG lLeftValue = 0;

		int nLeadStartLeft = pInspAlgoSearch->m_nLeadStartLeft;
		if (pInspAlgoSearch->m_bIsManualType)
			nLeadStartLeft = RounD(_mm2pixel_x(dLeadStartPos));

		for(int i = 0; i < nSolderCnt; i++)
		{
			lLeftValue = (nLeadStartLeft + RounD(_mm2pixel_x(i * dLeadPitch_mm)) + nLeadWidth);
			if(lLeftValue > nSearchWidth)
				continue;
			
			m_pLeadRect[i].left = nLeadStartLeft + RounD(_mm2pixel_x(i * dLeadPitch_mm));
			m_pLeadRect[i].right = m_pLeadRect[i].left + nLeadWidth;
			m_pLeadRect[i].top = 0;
			m_pLeadRect[i].bottom = nHeight;

			if(i > 0)
			{
				m_pGapRect[i-1].left = m_pLeadRect[i-1].right + 3;
				m_pGapRect[i-1].right = m_pLeadRect[i].left - 3;
				m_pGapRect[i-1].top = 0;
				m_pGapRect[i-1].bottom = nHeight;
			}
			nIndex++;
		}
		nSolderCnt = nIndex;
	}
	else
	{
		//zmap
		int nGap = 4 - (nImgWidth % 4);

		cv::Mat window_image_3D(nImgHeight, nImgWidth, CV_32FC1, pfImgSrc);
		if (dAngleValue == 90)
			cv::rotate(window_image_3D, window_image_3D, cv::ROTATE_90_COUNTERCLOCKWISE);
		else if (dAngleValue == 270)
			cv::rotate(window_image_3D, window_image_3D, cv::ROTATE_90_CLOCKWISE);
		else if (dAngleValue == 180)
			cv::rotate(window_image_3D, window_image_3D, cv::ROTATE_180);


		m_pProcMilLeadSolder->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("3DImage_1.bmp"));

		//bottom image - top image
		UCHAR* ptrucImgHV = g_pMManager->pem_new<UCHAR>(true, nROIArea, (PCHAR)__FUNCTION__, __LINE__);
		GetBinaryImg(ptrucROIImg2D, nWidth, nHeight, ptrucImgHV, nThresholdValue); //shk 2014/01/16
		m_pProcMilLeadSolder->SaveWorkImg(ptrucImgHV, nWidth, nHeight, _T("hv.bmp"));
		m_pProcMilLeadSolder->SaveWorkImg(ptrucROIImg2D, nWidth, nHeight, _T("ROIImg2D.bmp"));

		if(nSolderStartPos >= nHeight)
			nSolderStartPos = nHeight -1;

		nSolderCnt = CalcLeadSolderRect(ptrucROIImg2D, ptrucImgHV, window_image_3D.ptr<float>(), nWidth, nHeight,
									nSolderStartPos, bIsContainer, bInspHeightFlag, 0, 0, 0, bUse3D, bUse2D, nThresholdValue, pRcArrSearchTab);

		if(ptrucImgHV)
			//delete [] ptrucImgHV;
			g_pMManager->pem_delete(ptrucImgHV, true);

		if(bSameWidthPitch == TRUE || bGerber == TRUE)
		{
			if(bSameWidthPitch == TRUE)
				nLeadWidth = _mm2pixel_x(m_leadInfo.leadWidth);
			if(bGerber == TRUE)
			{
				nSolderCnt = nPadCnt;
				InitLeadRect(nSolderCnt);
				if(pInspAlgoSearch->m_nPadCnt > 1)
				{
					double nPadPitch = (double)pInspAlgoSearch->m_rcArrPadRect[1].left - pInspAlgoSearch->m_rcArrPadRect[0].left;
					if(nPadPitch < 0)
						nPadPitch *= -1;
					if(nPadPitch > 0)
						m_leadInfo.leadPitch = _pixel2mm_x(nPadPitch);
				}
			}
			if(nSolderCnt < 1)
				bResult = FALSE;

			if(nLeadWidth <= 0)
				nLeadWidth = _mm2pixel_x(m_leadInfo.leadWidth);
			if(dLeadPitch_mm <= 0)
				dLeadPitch_mm = m_leadInfo.leadPitch;

			//int nStartPoint = m_pLeadRect[0].left;
			for(int i = 0; i < nSolderCnt; i++)
			{
				if(bGerber == FALSE)
					m_pLeadRect[i].left = m_pLeadRect[0].left + RounD(_mm2pixel_x(i * dLeadPitch_mm));
				else
				{
					int nPadW = pInspAlgoSearch->m_rcArrPadRect[i].right - pInspAlgoSearch->m_rcArrPadRect[i].left;
					int nPadH = pInspAlgoSearch->m_rcArrPadRect[i].bottom - pInspAlgoSearch->m_rcArrPadRect[i].top;
					int nCX = pInspAlgoSearch->m_rcArrPadRect[i].left + (nPadW / 2);
					int nCY = pInspAlgoSearch->m_rcArrPadRect[i].top + (nPadH / 2);
					if(bSameWidthPitch == FALSE)
						nLeadWidth = m_pLeadRect[0].right - m_pLeadRect[0].left;
					if(nLeadWidth <= 0)
						nLeadWidth = _mm2pixel_x(m_leadInfo.leadWidth);
					if(nLeadWidth <= 0)
						nLeadWidth = nPadW;

					m_pLeadRect[i].left = nCX - (nLeadWidth / 2);
				}
				m_pLeadRect[i].right = m_pLeadRect[i].left + nLeadWidth;
				m_pLeadRect[i].top = 0;
				m_pLeadRect[i].bottom = nHeight;
				if(i > 0)
				{
					if(bGerber == FALSE)
					{
						m_pGapRect[i-1].left = m_pLeadRect[i-1].right + 3;
						m_pGapRect[i-1].right = m_pLeadRect[i].left - 3;
						m_pGapRect[i-1].top = 0;
						m_pGapRect[i-1].bottom = nHeight;
					}
					else
					{
						m_pGapRect[i-1].left = pInspAlgoSearch->m_rcArrPadRect[i-1].right - 3;
						m_pGapRect[i-1].right = pInspAlgoSearch->m_rcArrPadRect[i].left + 3;
						m_pGapRect[i-1].top = pInspAlgoSearch->m_rcArrPadRect[i].top;
						m_pGapRect[i-1].bottom = pInspAlgoSearch->m_rcArrPadRect[i].bottom;
					}
				}
			}

			if(bSameWidthPitch == TRUE && bWideSize == TRUE && nSolderCnt > 1)
			{				
				int nPitch = m_pLeadRect[1].left - m_pLeadRect[0].left;
				int nWidth = m_pLeadRect[0].right - m_pLeadRect[0].left;
				int nPlusStartPos = (nPitch - nWidth) / 2;
				int nGapFirst = 0;
				int nGapLast = 0;
				int imgSize = 0;

				if (nLeadPosition == 0 || nLeadPosition == 1) // Left, Right
					imgSize = nImgHeight;
				else if (nLeadPosition == 2 || nLeadPosition == 3) // Top, Bottom
					imgSize = nImgWidth;

				if (m_pLeadRect[0].left - nPlusStartPos  < 0 || m_pLeadRect[nSolderCnt - 1].right + nPlusStartPos > imgSize)
					nPlusStartPos = m_pLeadRect[0].left;

				if (m_pLeadRect[0].right + nPlusStartPos > m_pLeadRect[1].left - nPlusStartPos)
					nGapFirst = (m_pLeadRect[0].right + nPlusStartPos) - (m_pLeadRect[1].left - nPlusStartPos);

				if (m_pLeadRect[nSolderCnt - 1].right + nPlusStartPos > imgSize)
					nGapLast = m_pLeadRect[nSolderCnt - 1].right + nPlusStartPos - imgSize;

				if ( nGapLast > nGapFirst)
					nPlusStartPos -= nGapFirst;
				else
					nPlusStartPos -= nGapLast;

				for(int i = 0; i < nSolderCnt; i++)
				{
					m_pLeadRect[i].left -= nPlusStartPos;
					m_pLeadRect[i].right += nPlusStartPos;
				}
			}
		}
	}

	if(pRcArrSearchTab)
		nSolderCnt = pRcArrSearchTab->GetCount();

	m_leadInfo.leadCounts = m_solderCount = nLeadCnt = nSolderCnt;
	m_leadInfo.gapCounts = m_leadInfo.leadCounts - 1;
	if(nSolderCnt < 1)
		bResult = FALSE;

	if(ptrResultLeadInfo)
	{
		*ptrResultLeadInfo = m_leadInfo;

		if(bGerber == TRUE)
		{
			for(int i = 0; i < nSolderCnt - 1; i++)
				ptrResultLeadInfo->m_rcArrGapRect[i] = m_pGapRect[i];
		}		
	}
	int nAngleWidth = nImgWidth;
	int nAngleHeight = nImgHeight;
	if ((nLeadPosition == 0) || (nLeadPosition == 1))
	{
		nAngleWidth = nImgHeight;
		nAngleHeight = nImgWidth;
	}

	CRect* pRcArrDisplay = NULL;
	if(nSolderCnt > 0)
	{
		//pRcArrDisplay = new CRect[nSolderCnt];
		pRcArrDisplay = g_pMManager->pem_new<CRect>(true, nSolderCnt, (PCHAR)__FUNCTION__, __LINE__);
		for(int i = 0; i < nSolderCnt; i++)
		{
			if(!pRcArrSearchTab)
			{
				pRcArrDisplay[i].left = m_pLeadRect[i].left;
				pRcArrDisplay[i].right = m_pLeadRect[i].right;
				pRcArrDisplay[i].top = 0;
				if(bIsManual == TRUE || bGerber == TRUE)
					pRcArrDisplay[i].bottom = m_pLeadRect[i].bottom;
				else
					pRcArrDisplay[i].bottom = m_pSolderRect[i].bottom;
			}
			else
			{
				pRcArrDisplay[i] = pRcArrSearchTab->GetAt(i);
				if(bSameWidthPitch)
				{
					double dWidth = _mm2pixel_x(m_leadInfo.leadWidth);
					double posCenX = pRcArrDisplay[i].left + (pRcArrDisplay[i].right - pRcArrDisplay[i].left) / 2.0;
					pRcArrDisplay[i].left = RounD(posCenX - dWidth/2.0);
					pRcArrDisplay[i].right = RounD(pRcArrDisplay[i].left + dWidth);
				}
			}

			if(ptrResultLeadInfo)
			{
				if(i==0)
				{
					ptrResultLeadInfo->m_nStartLeadLeft = pRcArrDisplay[i].left;
					ptrResultLeadInfo->m_poStartLead.x = pRcArrDisplay[i].left + RounD((pRcArrDisplay[i].right - pRcArrDisplay[i].left) / 2.0);
					ptrResultLeadInfo->m_poStartLead.y = pRcArrDisplay[i].top + RounD((pRcArrDisplay[i].bottom - pRcArrDisplay[i].top) / 2.0);
				}
				else if(i==nSolderCnt-1)
				{
					ptrResultLeadInfo->m_poEndLead.x = pRcArrDisplay[i].left + RounD((pRcArrDisplay[i].right - pRcArrDisplay[i].left) / 2.0);
					ptrResultLeadInfo->m_poEndLead.y = pRcArrDisplay[i].top + RounD((pRcArrDisplay[i].bottom - pRcArrDisplay[i].top) / 2.0);
				}
			}

			if(pArrLeadRect != NULL)
				pArrLeadRect[i] = pRcArrDisplay[i];

			if(pRcArrSearchTab)
				continue;

			if(i < nSolderCnt-1)
			{
				m_pGapRect[i].bottom = m_pGapRect[i].bottom - (nAngleHeight / 2);
				m_pGapRect[i].top = m_pGapRect[i].top - (nAngleHeight / 2);
				m_pGapRect[i].right = m_pGapRect[i].right - (nAngleWidth / 2);
				m_pGapRect[i].left = m_pGapRect[i].left - (nAngleWidth / 2);
			}
		}

		//delete[] pRcArrDisplay;
		g_pMManager->pem_delete(pRcArrDisplay, true);
	}

	if(pUcArrRetResultImg != NULL/* && nSolderCnt > 0*/)
	{
		//image display
		/*UCHAR* ptrucRstImg = new UCHAR[nWidth * nHeight * 3];
		UCHAR* ptrucRstImgReSize =  new UCHAR[nSizeX * nSizeY * 3];*/
		UCHAR* ptrucRstImg = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight * 3, (PCHAR)__FUNCTION__, __LINE__);
		UCHAR* ptrucRstImgReSize = g_pMManager->pem_new<UCHAR>(true, nSizeX * nSizeY * 3, (PCHAR)__FUNCTION__, __LINE__);

		m_pProcMilLeadSolder->SaveLeadProcImg(ptrucROIImg2D, nWidth, nHeight, NULL, NULL, NULL, nSolderCnt, ptrucRstImg);

		double dFactor = m_pProcMilLeadSolder->GetResizeFactor(nSizeX, nSizeY, nWidth, nHeight);
		m_pProcMilLeadSolder->ResizeImg_Left(ptrucRstImg, ptrucRstImgReSize, nWidth, nHeight, nSizeX, nSizeY, 3, dFactor);
		//delete ptrucRstImg;
		g_pMManager->pem_delete(ptrucRstImg, true);

		m_pProcMilLeadSolder->SaveWorkImg(ptrucRstImgReSize, nSizeX, nSizeY, _T("resize.bmp"), 3);

		IppiSize iSize = {nSizeX, nSizeY*3};
		ippiCopy_8u_C1R(ptrucRstImgReSize, nSizeX, pUcArrRetResultImg, nSizeX, iSize);

		m_pProcMilLeadSolder->SaveWorkImg(pUcArrRetResultImg, nSizeX, nSizeY, _T("pUcArrRetResultImg.bmp"), 3);

		//delete ptrucRstImgReSize;
		g_pMManager->pem_delete(ptrucRstImgReSize, true);
	}
	//////////////////////////////////////////////////////////////////////////	

	//delete ptrucROIImg2D;
	g_pMManager->pem_delete(ptrucROIImg2D, true);
	
	return bResult = TRUE;
}

void CPInsp_LeadSolder::ClacSolderAvrSize(CRect* solderRect, int solderCnt, int* retW, int* retH)
{
	float sumW = 0;
	float sumH = 0;

	for(int i = 0; i < solderCnt; i++)
	{
		sumW += solderRect[i].Width();
		sumH += solderRect[i].Height();
	}



	if(retW != NULL)
	{
		if(sumW != 0)
			*retW = (int)(sumW / solderCnt);
		else
			*retW = 0;
	}

	if(retH != NULL)
	{
		if(sumH != 0)
			*retH = (int)(sumH / solderCnt);
		else
			*retH = 0;	
	}
}

/*
	 
float CPInsp_LeadSolder::GetChipBodyH(float* zmapData,CRect* pSolRect,int orgSizeX, int orgSizeY,int sizeX,int sizeY, double angle)
{
	if(zmapData == NULL || pSolRect == NULL || orgSizeX <= 0 || orgSizeY <= 0 || sizeX <= 0 || sizeY < 0)
		return 0;

	float retH = 0;
	int cx = 0;
	int cy = 0;

	if(angle == 0)
	{
		cx = (int)(pSolRect[0].left + (pSolRect[0].Width() / 2.0));
		cy = (int)((pSolRect[0].top - (sizeY / 2.0)) - 1);
	}
	else if(angle == 90)
	{
		cx = (int)((pSolRect[0].right + (sizeX / 2.0)) + 1); 
		cy = (int)(pSolRect[0].top + (pSolRect[0].Height() / 2.0));
	}
	else if(angle == 180)
	{
		cx = (int)(pSolRect[0].left + (pSolRect[0].Width() / 2.0));
		cy = (int)((pSolRect[0].bottom + (sizeY / 2.0)) + 1);
	}
	else if(angle == 270)
	{
		cx = (int)((pSolRect[0].left - (sizeX / 2.0)) - 1);
		cy = (int)(pSolRect[0].top + (pSolRect[0].Height() / 2.0));
	}

	

	//특정 영역 평균높이 GetFilterHeight
	retH = m_proc3d.GetFilterHeight(zmapData, orgSizeX, orgSizeY, cx, cy, sizeX, sizeY);

	return retH;
}
*/

float CPInsp_LeadSolder::GetChipBodyH(float* zmapData, CRect pSolRect, int orgSizeX, int orgSizeY, int sizeX, int sizeY, double angle, int startPt)
{
	if(zmapData == NULL || orgSizeX <= 0 || orgSizeY <= 0 || sizeX <= 0 || sizeY < 0)
		return 0;

	float retH = 0;
	int cx = 0;
	int cy = 0;
	int w = sizeX;
	int h = sizeY;


	if(angle == 0)
	{
		cx = (int)(pSolRect.left + (pSolRect.Width() / 2.0));
		cy = (int)(((pSolRect.top + startPt) - (h / 2.0)) - 1);
	}
	else if(angle == 90)
	{
		w = sizeY;
		h = sizeX;
		cx = (int)(((pSolRect.right - startPt) + (w / 2.0)) + 1); 
		cy = (int)(pSolRect.top + (pSolRect.Height() / 2.0));
		
	}
	else if(angle == 180)
	{
		cx = (int)(pSolRect.left + (pSolRect.Width() / 2.0));
		cy = (int)(((pSolRect.bottom - startPt) + (h / 2.0)) + 1);
	}
	else if(angle == 270)
	{
		w = sizeY;
		h = sizeX;

		cx = (int)(((pSolRect.left + startPt) - (w / 2.0)) - 1);
		cy = (int)(pSolRect.top + (pSolRect.Height() / 2.0));		
	}

	//특정 영역 평균높이 GetFilterHeight
	retH = m_proc3d.GetFilterHeight(zmapData, orgSizeX, orgSizeY, cx, cy, w, h);

	return retH;
}

void CPInsp_LeadSolder::GetSolderH(float* zmapData,CRect* pSolRect,int leadCnt,int orgSizeX, int orgSizeY,int sizeX,int sizeY, BOOL isChipSolder,float* retSolderHeight)
{
	int cx = 0;
	int cy = 0;
	// 	int sizeX = 0;
	// 	int sizeY = 0;

	float* psolderHeight= NULL;
	//psolderHeight = new float[leadCnt];
	psolderHeight = g_pMManager->pem_new<float>(true, leadCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(psolderHeight, 0, sizeof(float) * leadCnt);

	int margin = 3;  


	// 각 solderRect : m_pSolderRect --> 각 solderRect clip
	for (int i=0;i<leadCnt;i++)
	{
		if(!isChipSolder)//lead solder
		{
			cx = (int)(pSolRect[i].left + (pSolRect[i].Width() / 2.0));
			cy = (int)((pSolRect[i].top + (sizeY / 2.0)) + margin);
		}
		else// chip solder
		{
			cx = (int)(pSolRect[i].Width() / 2.0);
			cy = (int)((sizeY / 2.0) + margin);
		}


		//특정 영역 평균높이 GetFilterHeight
		psolderHeight[i] = m_proc3d.GetFilterHeight(zmapData, orgSizeX, orgSizeY, cx, cy, sizeX, sizeY);
	}

	if(retSolderHeight  != NULL)
		memcpy_s(retSolderHeight, sizeof(float) * leadCnt, psolderHeight, sizeof(float) * leadCnt);


	//delete psolderHeight;
	g_pMManager->pem_delete(psolderHeight, false);
}


void CPInsp_LeadSolder::CalcDeltaH(float* solderH, float* leadH, int leadCnt, float* retDeltaH, double* retRatio)
{
	//각 lead의 평균높이 : m_pLeadHeight 와 retSolderHeight 비교 --> retDeltaH
	if(retDeltaH  != NULL)
	{
		for (int i=0 ; i < leadCnt ; i++)
		{
			retDeltaH[i] = leadH[i] - solderH[i]; 
		}
	}

	//각 lead의 평균높이 : m_pLeadHeight 와 retSolderHeight 비교 --> retDeltaH
	if(retRatio  != NULL)
	{
		for (int i=0 ; i < leadCnt ; i++)
		{
			if(solderH[i] != 0)
				retRatio[i] = (double)((solderH[i] / leadH[i]) * 100.0); 
			else
				retRatio[i] = 0.0;
		}
	}
}

void CPInsp_LeadSolder::GetDeltaH(float* zmapData, float* leadH, CRect* pSolRect, int leadCnt, int orgSizeX, int orgSizeY, int sizeX, int sizeY, BOOL isChipSolder, float* retSolderH, double* retRatio)
{
	if(leadCnt <= 0)
		return;

	float* avrSolderH= NULL;
	//avrSolderH = new float[leadCnt];
	avrSolderH = g_pMManager->pem_new<float>(true, leadCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(avrSolderH, 0, sizeof(float) * leadCnt);


	GetSolderH(zmapData, pSolRect, leadCnt, orgSizeX, orgSizeY, sizeX, sizeY, isChipSolder, avrSolderH);


// 	float* DeltaH= NULL;
// 	DeltaH = new float[leadCnt];
// 	memset(DeltaH, 0, sizeof(float) * leadCnt);

	double* Ratio= NULL;
	//Ratio = new double[leadCnt];
	Ratio = g_pMManager->pem_new<double>(true, leadCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(Ratio, 0, sizeof(double) * leadCnt);

	CalcDeltaH(avrSolderH, leadH, leadCnt, NULL, Ratio);

	if(retRatio != NULL)
		memcpy_s(retRatio, sizeof(double) * leadCnt, Ratio, sizeof(double) * leadCnt);

	if(retSolderH != NULL)
		memcpy_s(retSolderH, sizeof(float) * leadCnt, avrSolderH, sizeof(float) * leadCnt);

	//delete avrSolderH;
	g_pMManager->pem_delete(avrSolderH, false);
	//delete DeltaH;
	//delete Ratio;
	g_pMManager->pem_delete(Ratio, false);
}
/*
int CPInsp_LeadSolder::GetLeadPosition()
{
	int ret = -1;

	float* zmapData = m_teachZmapData.data;
	int zmapSizeX = m_teachZmapData.zmapSizeX;
	int zmapSizeY = m_teachZmapData.zmapSizeY;
	int cx = m_teachCoordinate.bdrCx;
	int cy = m_teachCoordinate.bdrCy;
	int roiSizeX = m_teachCoordinate.bdrWidth;
	int roiSizeY = m_teachCoordinate.bdrLength;


	if(roiSizeX < 1 || roiSizeY < 1 || zmapData == NULL)
		return ePART_FAIL;

	float* zmapRoiData = NULL;
	zmapRoiData = new float[roiSizeX * roiSizeY];
	m_procMil->ClipZmap(zmapData, zmapRoiData, zmapSizeX, zmapSizeY, cx, cy , roiSizeX, roiSizeY);

	float* temp1 = NULL;
	float* temp2 = NULL;
	int size = 0;
	int mode = 0;
	if(roiSizeX > roiSizeY)
	{
		mode = 1;
		size = roiSizeX;
		temp1 = new float[size];
		temp2 = new float[size];
		m_proc3d.MakeProfileData_H(zmapRoiData, temp1, roiSizeX, roiSizeY, 0, 0, size);
		m_proc3d.MakeProfileData_H(zmapRoiData, temp2, roiSizeX, roiSizeY, 0, roiSizeY-1, size);
	}
	else
	{
		mode = 2;
		size = roiSizeY;
		temp1 = new float[size];
		temp2 = new float[size];
		m_proc3d.MakeProfileData_V(zmapRoiData, temp1, roiSizeX, roiSizeY, 0, 0, size);
		m_proc3d.MakeProfileData_V(zmapRoiData, temp2, roiSizeX, roiSizeY, roiSizeX-1, 0, size);
	}

	float sum1 = 0;
	float sum2 = 0;
	float avr1 = 0;
	float avr2 = 0;
	for(int i = 0; i < size; i++)
	{
		sum1 += temp1[i];
		sum2 += temp2[i];
	}
	avr1 = sum1 / size;
	avr2 = sum2 / size;

	if(mode == 1)
	{
		if(avr1 > avr2)
			ret = e_BOTTOM;
		else
			ret = e_TOP;
	}
	else if(mode == 2)
	{
		if(avr1 > avr2)
			ret = e_RIGHT;
		else
			ret = e_LEFT;
	}

	delete temp1;
	delete temp2;
	delete zmapRoiData;

	return ret;
}
*/

int CPInsp_LeadSolder::GetLeadPosition()
{
	int ret = -1;

	float* zmapData = m_teachZmapData.data;
	int zmapSizeX = m_teachZmapData.zmapSizeX;
	int zmapSizeY = m_teachZmapData.zmapSizeY;
	int cx = m_teachCoordinate.bdrCx;
	int cy = m_teachCoordinate.bdrCy;
	int roiSizeX = m_teachCoordinate.bdrWidth;
	int roiSizeY = m_teachCoordinate.bdrLength;
	int roiArea = roiSizeX * roiSizeY;
	double wndAngle = m_teachCoordinate.angle;

	int bdCx = m_teachCoordinate.anyAngleCx;
	int bdCy = m_teachCoordinate.anyAngleCy;
	int bdSizeX = m_teachCoordinate.anyAngleWidth;
	int bdSizeY = m_teachCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	if(roiSizeX < 1 || roiSizeY < 1 || zmapData == NULL)
		return ePART_FAIL;
//////////////////////////////////////////////////////////////////////////
	float* zmapRoiData = NULL;

	BOOL isAnyAngle = IsAnyAngle(wndAngle);
	if(isAnyAngle)
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;
		float* bdryZmapTemp = NULL;
		m_proc3d.ProcAnyAngle_Zmap(zmapData, &bdryZmapTemp, zmapSizeX, zmapSizeY, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		m_pProcMilLeadSolder->SaveWorkImg_float(bdryZmapTemp, w, l, _T("LEAD_boundary.bmp"));

		wndAngle = 0;
		int tempCx = cx -  (int)(bdCx - (w / 2.0));
		int tempCy = cy -  (int)(bdCy - (l / 2.0));

		//zmapRoiData = new float[roiArea];
		zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(bdryZmapTemp, zmapRoiData, w, l, tempCx, tempCy, roiSizeX, roiSizeY);
		m_pProcMilLeadSolder->SaveWorkImg_float(zmapRoiData, roiSizeX, roiSizeY, _T("LEAD_zmapRoiDataTemp.bmp"));

		//delete bdryZmapTemp;
		g_pMManager->pem_delete(bdryZmapTemp, false);
	}
	else
	{
		//roi영역만큼 추출
		//zmapRoiData = new float[roiArea];
		zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(zmapData, zmapRoiData, zmapSizeX, zmapSizeY, cx, cy, roiSizeX, roiSizeY);
	}
	//////////////////////////////////////////////////////////////////////////
	

	float* temp1 = NULL;
	float* temp2 = NULL;
	float* temp3 = NULL;
	float* temp4 = NULL;

	int size = 0;

	float sum[4] = {0,};
	float avr[4] = {0,};

	size = roiSizeX;
	/*temp1 = new float[size];
	temp2 = new float[size];*/
	temp1 = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	temp2 = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	m_proc3d.MakeProfileData_H(zmapRoiData, temp1, roiSizeX, roiSizeY, 0, 0, size);
	m_proc3d.MakeProfileData_H(zmapRoiData, temp2, roiSizeX, roiSizeY, 0, roiSizeY-1, size);

	for(int i = 0; i < size; i++)
	{
		sum[0] += temp1[i];
		sum[1] += temp2[i];
	}
	avr[0] = sum[0] / size;
	avr[1] = sum[1] / size;

	size = roiSizeY;
	/*temp3 = new float[size];
	temp4 = new float[size];*/
	temp3 = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	temp4 = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	m_proc3d.MakeProfileData_V(zmapRoiData, temp3, roiSizeX, roiSizeY, 0, 0, size);
	m_proc3d.MakeProfileData_V(zmapRoiData, temp4, roiSizeX, roiSizeY, roiSizeX-1, 0, size);

	for(int i = 0; i < size; i++)
	{
		sum[2] += temp3[i];
		sum[3] += temp4[i];
	}
	avr[2] = sum[2] / size;
	avr[3] = sum[3] / size;


	int maxAvr = (int)avr[0];
	int maxIndex = 0;

	for(int i=1; i<4; i++)
	{
		if(maxAvr<avr[i])
		{
			maxAvr = (int)avr[i];
			maxIndex = i;
		}

	}

	switch (maxIndex)
	{
	case 0:
		ret = e_BOTTOM;
		break;
	case 1:
		ret = e_TOP;
		break;
	case 2:
		ret = e_RIGHT;
		break;
	case 3:
		ret = e_LEFT;
		break;
	}


	/*delete temp1;
	delete temp2;
	delete temp3;
	delete temp4;

	delete zmapRoiData;*/
	g_pMManager->pem_delete(temp1, false);
	g_pMManager->pem_delete(temp2, false);
	g_pMManager->pem_delete(temp3, false);
	g_pMManager->pem_delete(temp4, false);
	g_pMManager->pem_delete(zmapRoiData, false);

	return ret;
}


LeadInfo CPInsp_LeadSolder::LeadInforCalc(CRect* leadRect, int leadCnts, CRect* gapRect, int gapCnts, int leadPos)
{
	LeadInfo ret;
	int sum_leadW = 0;
	int sum_leadP = 0;
	for(int i = 0; i < leadCnts; i++)
	{
		if(leadPos == e_TOP || leadPos == e_BOTTOM)
		{
			sum_leadW += leadRect[i].Width();

			if(i < gapCnts)
				sum_leadP += gapRect[i].Width();
		}
		else if(leadPos == e_LEFT || leadPos == e_RIGHT)
		{
			sum_leadW += leadRect[i].Height();

			if(i < gapCnts)
				sum_leadP += gapRect[i].Height();
		}
	}	

	double avr_leadW = 0;
	double avr_leadP = 0;

	avr_leadW = (double)sum_leadW / (double)leadCnts;
	avr_leadP = (double)sum_leadP / (double)gapCnts;



	ret.leadCounts = leadCnts;
	ret.gapCounts = gapCnts;
	ret.leadWidth = _pixel2mm_x(avr_leadW);
	ret.leadPitch = _pixel2mm_x(avr_leadP);

	return ret;
}

double CPInsp_LeadSolder::GetRotateAngle(int leadPos, int width, int height, double wndAngle, int* retWidth, int* retHeight)
{
	double angle = 0;
	double constAngle = 0;
	int w = width;
	int h = height;

	if(leadPos == e_LEFT)
		constAngle = 90;
	else if(leadPos == e_RIGHT)
		constAngle = 270;
	else if(leadPos == e_TOP)
		constAngle = 180;
	else if(leadPos == e_BOTTOM) 
		constAngle = 0;
	else
		constAngle = 0;


	angle = constAngle - wndAngle;
	if(angle < 0)
		angle = 360 + angle;

	if(angle == 90 || angle == 270)
	{
		w = height;
		h = width;
	}

	if(retWidth != NULL)
		*retWidth = w;
	if(retHeight != NULL)
		*retHeight = h;

	return angle;
}


int CPInsp_LeadSolder::InitSolderRect(int cnt)
{
	int ret = eMLSD_SUCCESS;
	DeleteSolderRect();

	if(cnt > 0)
	{
		//m_pSolderRect = new CRect[cnt];
		m_pSolderRect = g_pMManager->pem_new<CRect>(true, cnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pSolderRect, 0, sizeof(CRect) * cnt);
	}
	else
	{
		ret = eMLSD_FAIL;
	}

	return ret;
}

int CPInsp_LeadSolder::InitLeadRect(int cnt)
{
	int ret = eMLSD_SUCCESS;
	DeleteLeadRect();

	if(cnt > 0)
	{
		//m_pLeadRect = new CRect[cnt];
		m_pLeadRect = g_pMManager->pem_new<CRect>(true, cnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pLeadRect, 0, sizeof(CRect) * cnt);
		
		if(cnt > 1)
		{
			//m_pGapRect = new CRect[cnt - 1];
			m_pGapRect = g_pMManager->pem_new<CRect>(true, cnt - 1, (PCHAR)__FUNCTION__, __LINE__);
			memset(m_pGapRect, 0, sizeof(CRect) * (cnt - 1));
		}
	}
	else
	{
		ret = eMLSD_FAIL;
	}

	return ret;
}

void CPInsp_LeadSolder::DeleteSolderRect()
{
	if(m_pSolderRect != NULL)
	{
		//delete [] m_pSolderRect;
		g_pMManager->pem_delete(m_pSolderRect, true);
		m_pSolderRect = NULL;
	}
};


void CPInsp_LeadSolder::DeleteLeadRect()
{
	if(m_pLeadRect != NULL)
	{
		//delete [] m_pLeadRect;
		g_pMManager->pem_delete(m_pLeadRect, true);
		m_pLeadRect = NULL;
	}

	if(m_pGapRect != NULL)
	{
		//delete []  m_pGapRect;
		g_pMManager->pem_delete(m_pGapRect, true);
		m_pGapRect = NULL;
	}
};

void CPInsp_LeadSolder::InitResultStruct(RstInspLeadSolder* rst)
{
	//RstInspLeadSolder* temp = new RstInspLeadSolder;
	RstInspLeadSolder* temp = g_pMManager->pem_new<RstInspLeadSolder>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, sizeof(RstInspLeadSolder));

	temp->ok = -1;
	temp->isInsp = FALSE;

	//lead
	temp->list.leadBridgeOk = TRUE;
	temp->list.leadLiftOk = TRUE;
	temp->list.leadCountOk = TRUE;

	//solder
	temp->list.solderFilletOk = TRUE;
	temp->list.solderColorOk = TRUE;
	
	*rst = *temp;
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}


void CPInsp_LeadSolder::GetInspRst(RstInspLeadSolder* dst)
{
	*dst = m_inspResult;
}


void CPInsp_LeadSolder::DeleteBuffer()
{
	if(m_pZmapRoiData != NULL)
	{
		//delete m_pZmapRoiData;
		g_pMManager->pem_delete(m_pZmapRoiData, false);
		m_pZmapRoiData = NULL;
	}

	if(m_pRoiImg_Top != NULL)
	{
		//delete m_pRoiImg_Top;
		g_pMManager->pem_delete(m_pRoiImg_Top, false);
		m_pRoiImg_Top = NULL;
	}

	if(m_pRoiImg_Bottom != NULL)
	{
		//delete m_pRoiImg_Bottom;
		g_pMManager->pem_delete(m_pRoiImg_Bottom, false);
		m_pRoiImg_Bottom = NULL;
	}

	//shk 2014/01/16
	if(m_pRoiImg_Mid != NULL)
	{
		//delete m_pRoiImg_Mid;
		g_pMManager->pem_delete(m_pRoiImg_Mid, false);
		m_pRoiImg_Mid = NULL;
	}

	//shk 2014/04/25
	if(m_pRoiImg_Tar != NULL)
	{
		//delete m_pRoiImg_Tar;
		g_pMManager->pem_delete(m_pRoiImg_Tar, false);
		m_pRoiImg_Tar = NULL;
	}

	if(m_pImage_HV != NULL)
	{
		//delete m_pImage_HV;
		g_pMManager->pem_delete(m_pImage_HV, false);
		m_pImage_HV = NULL;
	}

	if(m_pZmapBoundaryData != NULL)
	{
		//delete m_pZmapBoundaryData;
		g_pMManager->pem_delete(m_pZmapBoundaryData, false);
		m_pZmapBoundaryData = NULL;
	}

	m_solderCount = 0;
}


void CPInsp_LeadSolder::InitSolderResultVal(int solderCnt)
{
	DeleteSolderResultVal();

	if(solderCnt > 0)
	{
		//m_solderVolRst = new int[solderCnt];
		m_solderVolRst = g_pMManager->pem_new<int>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_solderVolRst, 0, sizeof(int) * solderCnt);
	}
}

void CPInsp_LeadSolder::DeleteSolderResultVal()
{
	if(m_solderVolRst != NULL)
	{
		//delete m_solderVolRst;
		g_pMManager->pem_delete(m_solderVolRst, false);
		m_solderVolRst = NULL;
	}

}

void CPInsp_LeadSolder::InitSolderColorResultVal(int solderCnt)
{
	DeleteSolderColorResultVal();

	if(solderCnt > 0)
	{
		//m_solderColorRst = new int[solderCnt];
		m_solderColorRst = g_pMManager->pem_new<int>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_solderColorRst, 0, sizeof(int) * solderCnt);
	}
}

void CPInsp_LeadSolder::DeleteSolderColorResultVal()
{
	if(m_solderColorRst != NULL)
	{
		//delete m_solderColorRst;
		g_pMManager->pem_delete(m_solderColorRst, false);
		m_solderColorRst = NULL;
	}

}

void CPInsp_LeadSolder::InitSolderVolumeVal(int solderCnt)
{
	DeleteSolderVolumeVal();

	if(solderCnt > 0)
	{
		//m_solderVolume = new double[solderCnt];
		m_solderVolume = g_pMManager->pem_new<double>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_solderVolume, 0, sizeof(double) * solderCnt);
	}
}

void CPInsp_LeadSolder::DeleteSolderVolumeVal()
{
	if(m_solderVolume != NULL)
	{
		//delete m_solderVolume;
		g_pMManager->pem_delete(m_solderVolume, false);
		m_solderVolume = NULL;
	}
}

void CPInsp_LeadSolder::InitSolderColorRateVal(int solderCnt)
{
	DeleteSolderColorRateVal();

	if(solderCnt > 0)
	{
		//m_solderColorRate = new double[solderCnt];
		m_solderColorRate = g_pMManager->pem_new<double>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_solderColorRate, 0, sizeof(double) * solderCnt);
	}
}

void CPInsp_LeadSolder::DeleteSolderColorRateVal()
{
	if(m_solderColorRate != NULL)
	{
		//delete m_solderColorRate;
		g_pMManager->pem_delete(m_solderColorRate, false);
		m_solderColorRate = NULL;
	}
}


void CPInsp_LeadSolder::InitSolderHeightVal(int solderCnt)
{
	DeleteSolderHeightVal();

	if(solderCnt > 0)
	{
		//m_pSolderHeight = new float[solderCnt];
		m_pSolderHeight = g_pMManager->pem_new<float>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pSolderHeight, 0, sizeof(float) * solderCnt);
	}

}

void CPInsp_LeadSolder::DeleteSolderHeightVal()
{
	if(m_pSolderHeight != NULL)
	{
		//delete m_pSolderHeight;
		g_pMManager->pem_delete(m_pSolderHeight, false);
		m_pSolderHeight = NULL;
	}
}

void CPInsp_LeadSolder::InitFiletLengthVal(int solderCnt)
{
	DeleteFiletLengthVal();

	if(solderCnt > 0)
	{
		//m_pFiletLength = new int[solderCnt];
		m_pFiletLength = g_pMManager->pem_new<int>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pFiletLength, 0, sizeof(int) * solderCnt);
	}

}

void CPInsp_LeadSolder::DeleteFiletLengthVal()
{
	if(m_pFiletLength != NULL)
	{
		//delete m_pFiletLength;
		g_pMManager->pem_delete(m_pFiletLength, false);
		m_pFiletLength = NULL;
	}
}

void CPInsp_LeadSolder::InitSolderRatioVal(int solderCnt)
{
	DeleteSolderRatioVal();

	if(solderCnt > 0)
	{
		//m_solderRatio = new float[solderCnt];
		m_solderRatio = g_pMManager->pem_new<float>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_solderRatio, 0, sizeof(float) * solderCnt);
	}

}

void CPInsp_LeadSolder::InitgraySolderRatioVal(int solderCnt)
{
	DeletegraySolderRatioVal();

	if(solderCnt > 0)
	{
		//m_graysolderRatio = new float[solderCnt];
		m_graysolderRatio = g_pMManager->pem_new<float>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_graysolderRatio, 0, sizeof(float) * solderCnt);
	}

}

void CPInsp_LeadSolder::DeleteSolderRatioVal()
{
	if(m_solderRatio != NULL)
	{
		//delete m_solderRatio;
		g_pMManager->pem_delete(m_solderRatio, false);
		m_solderRatio = NULL;
	}
}

void CPInsp_LeadSolder::DeletegraySolderRatioVal()
{
	if(m_graysolderRatio != NULL)
	{
		//delete m_graysolderRatio;
		g_pMManager->pem_delete(m_graysolderRatio, false);
		m_graysolderRatio = NULL;
	}
}


void CPInsp_LeadSolder::InitSolderStartRstVal(int solderCnt)
{
	DeleteSolderStartRstVal();

	if(solderCnt > 0)
	{
		//m_pRstSolderStartPos = new int[solderCnt];
		m_pRstSolderStartPos = g_pMManager->pem_new<int>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pRstSolderStartPos, 0, sizeof(int) * solderCnt);
	}

}

void CPInsp_LeadSolder::DeleteSolderStartRstVal()
{
	if(m_pRstSolderStartPos != NULL)
	{
		//delete m_pRstSolderStartPos;
		g_pMManager->pem_delete(m_pRstSolderStartPos, false);
		m_pRstSolderStartPos = NULL;
	}
}


void CPInsp_LeadSolder::InitLeadHeightVal(int LeadCnt)
{
	DeleteLeadHeightVal();

	if(LeadCnt > 0)
	{
		//m_pLeadHeight = new float[LeadCnt];
		m_pLeadHeight = g_pMManager->pem_new<float>(true, LeadCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pLeadHeight, 0, sizeof(float) * LeadCnt);
	}

}

void CPInsp_LeadSolder::DeleteLeadHeightVal()
{
	if(m_pLeadHeight != NULL)
	{
		//delete m_pLeadHeight;
		g_pMManager->pem_delete(m_pLeadHeight, false);
		m_pLeadHeight = NULL;
	}
}

void CPInsp_LeadSolder::InitLeadLiftVal(int LeadCnt)
{
	DeleteLeadLiftVal();

	if(LeadCnt > 0)
	{
		//m_pLeadLiftRst = new BOOL[LeadCnt];
		m_pLeadLiftRst = g_pMManager->pem_new<BOOL>(true, LeadCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pLeadLiftRst, 0, sizeof(BOOL) * LeadCnt);
	}

}

void CPInsp_LeadSolder::DeleteLeadLiftVal()
{
	if(m_pLeadLiftRst != NULL)
	{
		//delete m_pLeadLiftRst;
		g_pMManager->pem_delete(m_pLeadLiftRst, false);
		m_pLeadLiftRst = NULL;
	}
}

void CPInsp_LeadSolder::InitLeadBridgeVal(int gapCnt)
{
	DeleteLeadBridgeVal();

	if(gapCnt > 0)
	{
		//m_pLeadBridgeRst = new BOOL[gapCnt];
		m_pLeadBridgeRst = g_pMManager->pem_new<BOOL>(true, gapCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pLeadBridgeRst, 0, sizeof(BOOL) * gapCnt);
	}
}

void CPInsp_LeadSolder::DeleteLeadBridgeVal()
{
	if(m_pLeadBridgeRst != NULL)
	{
		//delete m_pLeadBridgeRst;
		g_pMManager->pem_delete(m_pLeadBridgeRst, false);
		m_pLeadBridgeRst = NULL;
	}
}

void CPInsp_LeadSolder::InitLeadPitchVal(int gapCnt)
{
	DeleteLeadPitchVal();

	if(gapCnt > 0)
	{
		//m_pLeadPitch = new int[gapCnt];
		m_pLeadPitch = g_pMManager->pem_new<int>(true, gapCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_pLeadPitch, 0, sizeof(int) * gapCnt);
	}
}

void CPInsp_LeadSolder::DeleteLeadPitchVal()
{
	if(m_pLeadPitch != NULL)
	{
		//delete m_pLeadPitch;
		g_pMManager->pem_delete(m_pLeadPitch, false);
		m_pLeadPitch = NULL;
	}
}

void CPInsp_LeadSolder::DeleteAllData()
{
	m_solderCount = 0;

	DeleteBuffer();

	DeleteSolderRect();
	DeleteLeadRect();

	DeleteSolderColorRateVal();
	DeleteSolderResultVal();
	DeleteSolderHeightVal();
	DeleteFiletLengthVal();
	DeleteSolderColorResultVal();
	DeleteSolderVolumeVal();
	DeleteSolderRatioVal();
	DeletegraySolderRatioVal();
	DeleteSolderStartRstVal();

	DeleteLeadBridgeVal();
	DeleteLeadLiftVal();
	DeleteLeadHeightVal();
	DeleteLeadPitchVal();
}

int CPInsp_LeadSolder::CalcLeadSolderRect(UCHAR* topImg, UCHAR* hvImage,float* ZmapRoiData, 
										int roiSizeX, int roiSizeY, int startPos, BOOL isContainer, 
										BOOL isInspLeadLift, float leadHeight, double stdLeadWidth, int stdLeadCount, 
										BOOL use3D, BOOL use2D, int nThresholdValue, CAtlArray<CRect> *pRcArrSearchTab)
{
	CRect* leadRect = NULL;
	int* leadPitch = NULL;
	int leadStartPos = 0;

	LeadInfoTemp leadInfo;
	if(!pRcArrSearchTab)
	{
		leadInfo = m_pProcMilLeadSolder->CalcLeadRect4(hvImage, ZmapRoiData, roiSizeX, roiSizeY, use2D, use3D, startPos, &leadRect, &leadPitch);
	}
	else
	{
		leadInfo = m_pProcMilLeadSolder->CalcLeadRectTab(hvImage, roiSizeX, roiSizeY, nThresholdValue, startPos, pRcArrSearchTab);
	}

	int leadCount = leadInfo.countLead;
	m_leadInfo.leadCounts = leadInfo.countLead;
	m_leadInfo.leadWidth = _pixel2mm_x(leadInfo.avgLead);
	m_leadInfo.maxLeadWidth = _pixel2mm_x(leadInfo.maxLead);
	m_leadInfo.minLeadWidth = _pixel2mm_x(leadInfo.minLead);

	m_leadInfo.gapCounts = leadInfo.countGap;
	m_leadInfo.gapWidth = _pixel2mm_x(leadInfo.avgGap);
	m_leadInfo.maxGap = _pixel2mm_x(leadInfo.maxGap);
	m_leadInfo.minGap = _pixel2mm_x(leadInfo.minGap);

	int gapCount = (int)leadInfo.countGap;
	if( leadPitch && (leadInfo.countGap > 0) )
	{
		m_leadInfo.leadPitch = _pixel2mm_x(leadInfo.leadPitch);
		InitLeadPitchVal(gapCount);
		memcpy_s(m_pLeadPitch, sizeof(int) * gapCount, leadPitch, sizeof(int) * gapCount);

		//delete[] leadPitch;
		g_pMManager->pem_delete(leadPitch, true);
		leadPitch = NULL;
	}
	else if(pRcArrSearchTab)
		m_leadInfo.leadPitch = m_leadInfo.leadWidth + m_leadInfo.gapWidth;
	else
	{
		m_leadInfo.leadPitch = 0;
	}


	if (leadRect == NULL) //shk 2014-01-03
	{
		//delete[] leadRect; //shk 2014-01-06
		return 0;
	}

	//huj 2013/12/14  리드 시작점을 모두 같게 통일..(제일 작은값으로..) 테스트후 결과 안좋으면 평균값으로 변경...
	int minLeadStartPos = leadRect[0].top;
	for(int i = 0; i < leadCount; i++)
	{
		if(minLeadStartPos > leadRect[i].top)
			minLeadStartPos = leadRect[i].top;
	}


	if(leadCount > 0)
	{
		int top = 0;
		top = minLeadStartPos >= startPos?  0 : minLeadStartPos;

		InitLeadRect(leadCount);
		InitSolderRect(leadCount);
		for(int i = 0; i < leadCount; i++)
		{
			m_pSolderRect[i].top = (LONG)startPos;
			m_pSolderRect[i].bottom = (LONG)roiSizeY;//(LONG)endPos;
			m_pSolderRect[i].left = leadRect[i].left;
			m_pSolderRect[i].right = leadRect[i].right;

			m_pLeadRect[i].top = leadRect[i].top; //top;  //huj 2014/04/09 시작점은 각각 다를 수 있음...   //huj 2013/12/14  리드 시작점을 모두 같게 통일..(제일 작은값으로..)
			m_pLeadRect[i].bottom = (LONG)startPos;
			m_pLeadRect[i].left = leadRect[i].left - 1;
			m_pLeadRect[i].right = leadRect[i].right + 1;

			if(i < leadCount - 1)
			{
				m_pGapRect[i].top = leadRect[i].top + 2;//top;
				m_pGapRect[i].bottom = (LONG)roiSizeY;
				m_pGapRect[i].left = leadRect[i].right + 3;
				m_pGapRect[i].right = leadRect[i + 1].left - 3;
			}
		}	
	}

	//delete[] leadRect;
	g_pMManager->pem_delete(leadRect, true);

	if(leadPitch != NULL)
		//delete leadPitch;
		g_pMManager->pem_delete(leadPitch, false);

	return leadCount;
}

int CPInsp_LeadSolder::CalcLeadSolderRect(UCHAR* topImg, UCHAR* hvImage,float* ZmapRoiData, 
										int roiSizeX, int roiSizeY, int startPos, int endPos, 
										BOOL isContainer, BOOL isInspLeadLift, float leadHeight, 
										double stdLeadWidth, int stdLeadCount, BOOL use3D, BOOL use2D)
{
	CRect* leadRect = NULL;
	int* leadPitch = NULL;
	int leadStartPos = 0;

	LeadInfoTemp leadInfo = m_pProcMilLeadSolder->CalcLeadRect3(topImg, hvImage,ZmapRoiData, roiSizeX, roiSizeY, &leadRect, &leadPitch, 
												isContainer, isInspLeadLift, startPos, leadHeight, stdLeadWidth, stdLeadCount, use3D, use2D);
	int leadCount = leadInfo.countLead;

	m_leadInfo.leadCounts = leadInfo.countLead;
	m_leadInfo.leadWidth = _pixel2mm_x(leadInfo.avgLead);
	m_leadInfo.maxLeadWidth = _pixel2mm_x(leadInfo.maxLead);
	m_leadInfo.minLeadWidth = _pixel2mm_x(leadInfo.minLead);

	m_leadInfo.gapCounts = leadInfo.countGap;
	m_leadInfo.gapWidth = _pixel2mm_x(leadInfo.avgGap);
	m_leadInfo.maxGap = _pixel2mm_x(leadInfo.maxGap);
	m_leadInfo.minGap = _pixel2mm_x(leadInfo.minGap);

	int gapCount = (int)leadInfo.countGap;
	if( leadPitch && (leadInfo.countGap > 0) )
	{
		m_leadInfo.leadPitch = _pixel2mm_x(leadInfo.leadPitch);
		InitLeadPitchVal(gapCount);
		memcpy_s(m_pLeadPitch, sizeof(int) * gapCount, leadPitch, sizeof(int) * gapCount);

		//delete[] leadPitch;
		g_pMManager->pem_delete(leadPitch, true);
		leadPitch = NULL;
	}
	else
	{
		m_leadInfo.leadPitch = 0;
	}


	if (leadRect == NULL) //shk 2014-01-03
	{
		//delete[] leadRect; //shk 2014-01-06
		return 0;
	}

	//huj 2013/12/14  리드 시작점을 모두 같게 통일..(제일 작은값으로..) 테스트후 결과 안좋으면 평균값으로 변경...
	int minLeadStartPos = leadRect[0].top;
	for(int i = 0; i < leadCount; i++)
	{
		if(minLeadStartPos > leadRect[i].top)
			minLeadStartPos = leadRect[i].top;
	}


	if(leadCount > 0)
	{
		int top = 0;
		top = minLeadStartPos >= startPos?  0 : minLeadStartPos;

		InitLeadRect(leadCount);
		InitSolderRect(leadCount);
		for(int i = 0; i < leadCount; i++)
		{
			m_pSolderRect[i].top = (LONG)startPos;
			m_pSolderRect[i].bottom = (LONG)roiSizeY;//(LONG)endPos;
			m_pSolderRect[i].left = leadRect[i].left;
			m_pSolderRect[i].right = leadRect[i].right;

			m_pLeadRect[i].top = leadRect[i].top; //top;  //huj 2014/04/09 시작점은 각각 다를 수 있음...   //huj 2013/12/14  리드 시작점을 모두 같게 통일..(제일 작은값으로..)
			m_pLeadRect[i].bottom = (LONG)startPos;
			m_pLeadRect[i].left = leadRect[i].left;
			m_pLeadRect[i].right = leadRect[i].right;

			if(i < leadCount - 1)
			{
				m_pGapRect[i].top = leadRect[i].top + 2;//top;
				m_pGapRect[i].bottom = (LONG)roiSizeY;
				m_pGapRect[i].left = leadRect[i].right;
				m_pGapRect[i].right = leadRect[i + 1].left;
			}
		}	
	}

	//delete[] leadRect;
	g_pMManager->pem_delete(leadRect, true);

	if(leadPitch != NULL)
		//delete leadPitch;
		g_pMManager->pem_delete(leadPitch, false);

	return leadCount;
}

double CPInsp_LeadSolder::Insp_CalcSolderArea(UCHAR* src,int sizeX,int sizeY,CRect rt,int isChipSolder, int threshold)
{
	double ret = 0;

	if (isChipSolder)//When isChipSolder is True
	{
		ret = m_pProcMilLeadSolder->CalcSolderArea(src, sizeX, sizeY, threshold);
	}
	else			//When isChipSolder is False
	{
		//전체 이미지(sizex,sizey)에서 rect 만큼 clip
		int roiArea = rt.Width() * rt.Height() ;
		int cx = rt.left +  (rt.Width()/2);
		int cy = rt.top +  (rt.Height()/2);

		//UCHAR* dstClip = new UCHAR[roiArea];
		UCHAR* dstClip = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		//memset(dstClip, 0, sizeof(UCHAR) * roiArea);

		m_pProcMilLeadSolder->GetClipImage(src, sizeX, sizeY, dstClip, cx, cy, rt.Width(), rt.Height());
		ret = m_pProcMilLeadSolder->CalcSolderArea(dstClip,  rt.Width() , rt.Height() ,threshold);

		//delete dstClip;
		g_pMManager->pem_delete(dstClip, false);
	}

	//////////////////////////////////////////////////////////////////////////
	return ret;
}


int CPInsp_LeadSolder::RemakeLeadRect(int leadCount, int* solderStartPoint)
{
	if(leadCount <= 0 || solderStartPoint == NULL || m_pLeadRect == NULL)
		return FALSE;

	int temp = 0;
	for(int i = 0; i < leadCount; i++)
	{
		temp = m_pLeadRect[i].bottom + solderStartPoint[i];

		if(temp > m_pLeadRect[i].top && temp > 0)
			m_pLeadRect[i].bottom = temp;	
	}

	return TRUE;
}

int CPInsp_LeadSolder::RemakeSolderRect(int solderCount, int* solderStartPoint)
{
	if(solderCount <= 0 || solderStartPoint == NULL || m_pSolderRect == NULL)
		return FALSE;

	int temp = 0;
	for(int i = 0; i < solderCount; i++)
	{
		temp = m_pSolderRect[i].top + solderStartPoint[i];
		if(temp < m_pSolderRect[i].bottom && temp > 0)
			m_pSolderRect[i].top = temp;	
	}

	return TRUE;
}

LeadInfo CPInsp_LeadSolder::GetLeadInfo()
{
	return m_leadInfo;
}

BOOL CPInsp_LeadSolder::IsLeadNg()
{
	return !m_inspResult.ok;
}


//defect image save use..
BOOL CPInsp_LeadSolder::GetLeadRectInfo(LeadNgRectInfo** leadInfo)
{
	BOOL ret = FALSE;

	if(m_inspResult.ok == TRUE)
		return !m_inspResult.ok;

	int cx = m_inspCoordinate.bdrCx;
	int cy = m_inspCoordinate.bdrCy;
	int roiSizeX = m_inspCoordinate.bdrWidth;
	int roiSizeY = m_inspCoordinate.bdrLength;
	int leadCount = m_solderCount;
	int gapCount = m_solderCount - 1;

	InitNgInfoBuff(leadCount);

	m_pNgLeadInfo->isOK = m_inspResult.ok;
	m_pNgLeadInfo->wndRoiforFOV.left = (int)(cx - (roiSizeX / 2.0));
	m_pNgLeadInfo->wndRoiforFOV.right = (int)(cx + (roiSizeX / 2.0));
	m_pNgLeadInfo->wndRoiforFOV.top = (int)(cy - (roiSizeY / 2.0));
	m_pNgLeadInfo->wndRoiforFOV.bottom = (int)(cy + (roiSizeY / 2.0));

	m_pNgLeadInfo->leadCount = leadCount;
	m_pNgLeadInfo->bridgeCount = gapCount;
	m_pNgLeadInfo->solderCount = leadCount;

	m_pNgLeadInfo->isLeadCountNg = !m_inspResult.list.leadCountOk;

	//////////////////////////////////////////////////////////////////////////

	if(m_wndAngle == 0)
	{
		m_pNgLeadInfo->position = e_BOTTOM;
	}
	else if(m_wndAngle == 90.0)
	{
		m_pNgLeadInfo->position = e_LEFT;
	}
	else if(m_wndAngle == 180.0)
	{
		m_pNgLeadInfo->position = e_TOP;
	}
	else if(m_wndAngle == 270.0)
	{
		m_pNgLeadInfo->position = e_RIGHT;
	}
	//////////////////////////////////////////////////////////////////////////
	int ngCount = 0;
	//lead
	if(m_pLeadLiftRst != NULL)
	{
		for(int i = 0; i < leadCount; i++)
		{
			if(m_pLeadLiftRst[i] == TRUE)
			{	
				m_ngLeadRect[ngCount] = RotateLeadRect(m_pLeadRect[i], cx, cy, roiSizeX, roiSizeY, m_wndAngle);
				ngCount++;				
			}
		}
		m_pNgLeadInfo->leadNgCount = ngCount;

		if(ngCount > 0)
			m_pNgLeadInfo->leadRect = m_ngLeadRect;
		else
			m_pNgLeadInfo->leadRect = NULL;
	}
	else
	{
		m_pNgLeadInfo->leadNgCount = 0;
		m_pNgLeadInfo->leadRect = NULL;
	}

	//solder
	ngCount = 0;
	if(m_solderVolRst != NULL)
	{
		for(int i = 0; i < leadCount; i++)
		{
			if(m_solderVolRst[i] > 0 || m_solderColorRst[i] > 0)
			{	
				m_ngSolderRect[ngCount] = RotateLeadRect(m_pSolderRect[i], cx, cy, roiSizeX, roiSizeY, m_wndAngle);
				ngCount++;				
			}
		}
		
		m_pNgLeadInfo->solderNgCount = ngCount;

		if(ngCount > 0)
			m_pNgLeadInfo->solderRect = m_ngSolderRect;
		else
			m_pNgLeadInfo->solderRect = NULL;
	}
	else
	{
		m_pNgLeadInfo->solderNgCount = 0;
		m_pNgLeadInfo->solderRect = NULL;
	}

	//bridge
	ngCount = 0;
	if(m_pLeadBridgeRst != NULL)
	{
		for(int i = 0; i < gapCount; i++)
		{
			if(m_pLeadBridgeRst[i] == TRUE)
			{	
				m_ngGapRect[ngCount] = RotateLeadRect(m_pGapRect[i], cx, cy, roiSizeX, roiSizeY, m_wndAngle);
				ngCount++;				
			}
		}	
		m_pNgLeadInfo->bridgeNgCount = ngCount;

		if(ngCount > 0)
			m_pNgLeadInfo->bridgeRect = m_ngGapRect;
		else
			m_pNgLeadInfo->bridgeRect = NULL;
	}
	else
	{
		m_pNgLeadInfo->bridgeNgCount = 0;
		m_pNgLeadInfo->bridgeRect = NULL;
	}


	*leadInfo = m_pNgLeadInfo;

	ret = !m_inspResult.ok;

	return ret;
}

CRect CPInsp_LeadSolder::RotateLeadRect(CRect srcLeadRect, int cx, int cy, int roiSizeX, int roiSizeY, double angle, BOOL isWnd)
{
	CRect leadRectTemp(0,0,0,0);

	int wndLeft = (int)(cx - (roiSizeX / 2.0));
	int wndTop = (int)(cy - (roiSizeY / 2.0));

	if(isWnd == TRUE)//true :  윈도우 기준 rect 좌표,  false : fov기준 rect 좌표
	{
		wndLeft = 0;
		wndTop = 0;
	}

	if(angle == 0)
	{
		leadRectTemp.left = (LONG)(wndLeft + srcLeadRect.left);
		leadRectTemp.right = (LONG)(leadRectTemp.left + srcLeadRect.Width());
		leadRectTemp.top = (LONG)(wndTop + srcLeadRect.top);
		leadRectTemp.bottom = (LONG)(leadRectTemp.top + srcLeadRect.Height());
	}
	else if(angle == 90)
	{
		leadRectTemp.left = (LONG)(wndLeft + (roiSizeX - srcLeadRect.bottom));
		leadRectTemp.right = (LONG)(leadRectTemp.left + srcLeadRect.Height());
		leadRectTemp.top = (LONG)(wndTop + srcLeadRect.left);
		leadRectTemp.bottom = (LONG)(leadRectTemp.top + srcLeadRect.Width());
	}
	else if(angle == 180)
	{
		leadRectTemp.left = (LONG)(wndLeft + (roiSizeX - srcLeadRect.right));
		leadRectTemp.right = (LONG)(leadRectTemp.left + srcLeadRect.Width());
		leadRectTemp.top = (LONG)(wndTop + (roiSizeY - srcLeadRect.bottom));
		leadRectTemp.bottom = (LONG)(leadRectTemp.top + srcLeadRect.Height());
	}
	else if(angle == 270)
	{
		leadRectTemp.left = (LONG)(wndLeft + srcLeadRect.top);
		leadRectTemp.right = (LONG)(leadRectTemp.left + srcLeadRect.Height());
		leadRectTemp.top = (LONG)(wndTop + (roiSizeY - srcLeadRect.right));
		leadRectTemp.bottom = (LONG)(leadRectTemp.top + srcLeadRect.Width());
	}

	return leadRectTemp;
}

void CPInsp_LeadSolder::InitNgInfoBuff(int leadCount)
{
	DeleteNgInfoBuff();

	//m_pNgLeadInfo = new LeadNgRectInfo();
	m_pNgLeadInfo = g_pMManager->pem_new<LeadNgRectInfo>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

	if(leadCount > 0)
	{
		//m_ngLeadRect = new CRect[leadCount];
		m_ngLeadRect = g_pMManager->pem_new<CRect>(true, leadCount, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_ngLeadRect, 0, sizeof(CRect) * leadCount);

		//m_ngSolderRect = new CRect[leadCount];
		m_ngSolderRect = g_pMManager->pem_new<CRect>(true, leadCount, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_ngSolderRect, 0, sizeof(CRect) * leadCount);
	}

	if(leadCount > 1)
	{
		//m_ngGapRect = new CRect[leadCount - 1];
		m_ngGapRect = g_pMManager->pem_new<CRect>(true, leadCount - 1, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_ngGapRect, 0, sizeof(CRect) * (leadCount - 1));
	}
}

void CPInsp_LeadSolder::DeleteNgInfoBuff()
{
	if(m_ngLeadRect != NULL)
	{
		//delete m_ngLeadRect;
		g_pMManager->pem_delete(m_ngLeadRect, false);
		m_ngLeadRect = NULL;
	}

	if(m_ngGapRect != NULL)
	{
		//delete m_ngGapRect;
		g_pMManager->pem_delete(m_ngGapRect, false);
		m_ngGapRect = NULL;
	}

	if(m_ngSolderRect != NULL)
	{
		//delete m_ngSolderRect;
		g_pMManager->pem_delete(m_ngSolderRect, false);
		m_ngSolderRect = NULL;
	}

	if(m_pNgLeadInfo != NULL)
	{
		//delete m_pNgLeadInfo;
		g_pMManager->pem_delete(m_pNgLeadInfo, false);
		m_pNgLeadInfo = NULL;
	}
}


//UI use
void CPInsp_LeadSolder::GetEachLeadCoordi(double* retLeftPos, double* retRightPos)
{
	if(m_pLeadRect == NULL || m_solderCount < 1)
		return;

	int cx = m_teachCoordinate.bdrCx;
	int cy = m_teachCoordinate.bdrCy;
	int roiSizeX = m_teachCoordinate.bdrWidth;
	int roiSizeY = m_teachCoordinate.bdrLength;

//	CRect* leadRectTemp = NULL;
// 	leadRectTemp = new CRect[m_solderCount];
// 	memset(leadRectTemp, 0, sizeof(CRect) * m_solderCount);

	CRect leadRectTemp(0, 0, 0, 0);

	int margin = SOLDERW_MARGIN;
	int temp = 0;
	for(int i = 0; i < m_solderCount; i++)
	{
		leadRectTemp = RotateLeadRect(m_pLeadRect[i], cx, cy, roiSizeX, roiSizeY, m_wndAngle, TRUE);	

		if(retLeftPos != NULL)
		{
			if(m_wndAngle == 90.0 || m_wndAngle == 270)
			{
				temp = (int)(leadRectTemp.top - margin);
				temp = temp < 0?  0 : temp;
			}
			else
			{
				temp = (int)(leadRectTemp.left - margin);
				temp = temp < 0?  0 : temp;
			}
			retLeftPos[i] = _pixel2mm_x(temp);
		}

		if(retRightPos != NULL)
		{
			if(m_wndAngle == 90.0 || m_wndAngle == 270)
			{
				temp = (int)(leadRectTemp.bottom + margin);
				temp = temp > roiSizeY?  roiSizeY : temp;
			}
			else
			{
				temp = (int)(leadRectTemp.right + margin);
				temp = temp > roiSizeX?  roiSizeX : temp;
			}
			retRightPos[i] = _pixel2mm_x(temp);
		}

		leadRectTemp.SetRect(0, 0, 0, 0);
	}

// 	if(leadRectTemp != NULL)
// 		delete leadRectTemp;
}









//////////////////////////////////////////////////////////////////////////
//solder color


void CPInsp_LeadSolder::MakeProcImg(void* src1, void* src2, void* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, Coordinate coordi)
{
	int cx = coordi.cx;
	int cy = coordi.cy;
	int roiSizeX = coordi.width;
	int roiSizeY = coordi.length;
	int roiArea = roiSizeX * roiSizeY;

	int bdCx = coordi.anyAngleCx;
	int bdCy = coordi.anyAngleCy;
	int bdSizeX = coordi.anyAngleWidth;
	int bdSizeY = coordi.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	double wndAngle = coordi.angle;

	if(src1 == NULL || dst1 == NULL)
		return;

	//////////////////////////////////////////////////////////////////////////
	BOOL isAnyAngle = IsAnyAngle(wndAngle);

	if(isAnyAngle)
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;

		UCHAR* bdryImgTemp_src1 = NULL;
		m_pProcMilLeadSolder->ProcAnyAngle_Img(src1, &bdryImgTemp_src1, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		m_pProcMilLeadSolder->SaveWorkImg(bdryImgTemp_src1, w, l, _T("COLOR_boundaryImg1.bmp"));

		wndAngle = 0;
		cx -= (int)(bdCx - (w / 2.0));
		cy -= (int)(bdCy - (l / 2.0));

		m_pProcMilLeadSolder->GetClipImage(bdryImgTemp_src1, w, l, dst1, cx, cy, roiSizeX, roiSizeY);
		//delete bdryImgTemp_src1;
		g_pMManager->pem_delete(bdryImgTemp_src1, false);


		if(src2 != NULL && dst2 != NULL)
		{
			UCHAR* bdryImgTemp_src2 = NULL;
			m_pProcMilLeadSolder->ProcAnyAngle_Img(src2, &bdryImgTemp_src2, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_pProcMilLeadSolder->SaveWorkImg(bdryImgTemp_src2, w, l, _T("COLOR_boundaryImg2.bmp"));

			m_pProcMilLeadSolder->GetClipImage(bdryImgTemp_src2, w, l, dst2, cx, cy, roiSizeX, roiSizeY);
			//delete bdryImgTemp_src2;
			g_pMManager->pem_delete(bdryImgTemp_src2, false);
		}


		if(src3 != NULL && dst3 != NULL)
		{
			UCHAR* bdryImgTemp_src3 = NULL;
			m_pProcMilLeadSolder->ProcAnyAngle_Img(src3, &bdryImgTemp_src3, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
			m_pProcMilLeadSolder->SaveWorkImg(bdryImgTemp_src3, w, l, _T("COLOR_boundaryImg3.bmp"));

			m_pProcMilLeadSolder->GetClipImage(bdryImgTemp_src3, w, l, dst3, cx, cy, roiSizeX, roiSizeY);
			//delete bdryImgTemp_src3;
			g_pMManager->pem_delete(bdryImgTemp_src3, false);
		}
	}
	else
	{
		m_pProcMilLeadSolder->GetClipBuff(src1, dst1, cx, cy, roiSizeX, roiSizeY);

		if(src2 != NULL && dst2 != NULL)
		{
			m_pProcMilLeadSolder->GetClipBuff(src2, dst2, cx, cy, roiSizeX, roiSizeY);
		}

		if(src3 != NULL && dst3 != NULL)
		{
			m_pProcMilLeadSolder->GetClipBuff(src3, dst3, cx, cy, roiSizeX, roiSizeY);
		}
	}
}
// 
// //LMJ 2014/01/10
// void CPInsp_LeadSolder::InitMakeLUT_Color()
// {
// 	CPInsp_Color inspColor;
// 	POINT pPt[] = {363, 663, 354, 795, 490, 791, 487, 542, 386, 540};
// 
// 	m_lutData_color = NULL;
// 	m_lutData_color = new BYTE[SIZE_CLRX * SIZE_CLRY];
// 	inspColor.MakeLUT_Poly(m_lutData_color, SIZE_CLRX, SIZE_CLRY, pPt, 5);
// }
