#include "StdAfx.h"
#include "MPTI.h"
#include "InspManager.h"
#include "PInsp_Mount.h"
#include <math.h>
#include "ProcPil_Pattern.h"
#include "LeastSquare.h"
#include "GeoMatch_BodyBlob.h"

CPInsp_Mount::CPInsp_Mount(void)
{
	m_className = _T("CPInsp_Mount");

	m_procMil = NULL;
	m_procPatMil = NULL;
	//m_milApp = M_NULL;
	//m_milSys = M_NULL;

	m_inspParam_Mount = NULL;
	m_teachParam_Mount = NULL;

	m_fovImage_insp = NULL;
	m_fovImage_teach = NULL;

	m_bodyOffset = NULL;

	InitMntResultStruct(&m_rstInspMount);
	InitOffsetBuf();

	m_bUseImagePilLib = false;
}

CPInsp_Mount::~CPInsp_Mount(void)
{
	DeleteOffsetBuf();
	CloseDevice();
}

#define TomstoneAreaValue 3.0
#define TomstoneHeightValue 1.5
#define BillboardingAreaValue 2.0
#define BillboardingHeightValue 1.2
int CPInsp_Mount::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	m_bUseImagePilLib = bUseImagePilLib;

	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;
	 
	//m_procMil = new CProcMil_Mount();
	m_procMil = g_pMManager->pem_new<CProcMil_Mount>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	//m_procMil->InitMil(m_milApp, m_milSys);
	m_procMil->InitMil(milApp, milSys,bUseImagePilLib);
	m_procMil->SetResol(resolX, resolY, m_fovWidth);

	if(m_bUseImagePilLib == true)
		//m_procPatMil = new CProcPil_Pattern();
		m_procPatMil = g_pMManager->pem_new<CProcPil_Pattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	else
		//m_procPatMil = new CProcMil_Pattern();
		m_procPatMil = g_pMManager->pem_new<CProcMil_Pattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

	//m_procMil->InitMil(m_milApp, m_milSys);
	m_procPatMil->InitMil(milApp, milSys,bUseImagePilLib);
	m_procPatMil->SetResol(resolX, resolY, m_fovWidth);

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	if(m_pProcMilAlgo)
		m_pProcMilAlgo->InitMilAlgoBlob();	

	return ePART_SUCCESS;
}

int CPInsp_Mount::CloseDevice()
{
	if(m_procMil != NULL)
	{
		m_procMil->FreeMil();
		
		//delete m_procMil;
		g_pMManager->pem_delete(m_procMil, false);
		m_procMil = NULL;
	}

	if(m_procPatMil != NULL)
	{
		m_procPatMil->FreeMil();
		
		//delete m_procPatMil;
		g_pMManager->pem_delete(m_procPatMil, false);
		m_procPatMil = NULL;
	}

	if(m_pProcMilAlgo != NULL)
		m_pProcMilAlgo->FreeMilAlgoBlob();
	CPInsp::CloseDevice();

	return ePART_SUCCESS;	
}

int CPInsp_Mount::SetInspParam(void* itemParam, void* targetImg, ZmapData zmap, Coordinate cdn, int fovNum)
{
	m_inspParam_Mount = (InspParamMount*) itemParam;
	m_fovImage_insp = targetImg;
	m_inspZmapData =  zmap;
	m_inspCoordinate = cdn;

	m_fovIndex = fovNum;

	return ePART_SUCCESS;
}

int CPInsp_Mount::SetTeachParam(void* itemParam, void* targetImg, int teachType, ZmapData zmap, Coordinate cdn)
{
	m_teachParam_Mount = (TeachParamMount*) itemParam;
	m_fovImage_teach = targetImg;
	m_teachZmapData = zmap;
	m_teachCoordinate = cdn;

	return ePART_SUCCESS;
}


int CPInsp_Mount::InspProc_Mount()
{
	int ret = 0;

	ret = ProcAuto();
	
	return ret;
}


int CPInsp_Mount::ProcAuto()
{
	int ret = e_OK;


	int stepID = 0;
	int maxStepCnts = eMStepID_COUNTS;
	BOOL isIsnpPatternFlag = m_inspParam_Mount->inspPatternFlag;

	//step1(eMStepID_BLOB) : blob
	//step2(eMStepID_SIZE) : size (height X)
	//step5(eMStepID_POLARITY) : polarity
	//step6(eMStepID_ECCENTRICITY) : eccentricity
	//step7(eMStepID_ROTATE) : rotate
	//step8(eMStepID_GRADIENT) : gradient
	
	InitBlobResultStruct(&m_inspParam_Mount->retBlobRst);  //initialize blob result struct
	InitMntResultStruct(&m_inspParam_Mount->retInspMountResult);
	InitMntResultStruct(&m_rstInspMount);			

	stepID = 0;
	int rstTemp = e_OK;
	for(int i = 0; i < maxStepCnts; i++)
	{
		rstTemp = ProcStep(stepID);
		
		if(rstTemp == e_NG)
		{			
			ret = e_NG;
			if(isIsnpPatternFlag == FALSE)
			{
				if(stepID == eMStepID_BLOB)
					break;
			}
			else
			{
				if(stepID == eMStepID_PATTERN && ret == e_NG)
					break;
			}
		}
		else
		{
			if(stepID == eMStepID_PATTERN && ret == e_NG)
			{
				ret = e_OK;
				break;
			}
		}

		stepID++;
	}

	if(ret == e_NG)
		m_rstInspMount.ok = FALSE;

	return ret;
}


int CPInsp_Mount::ProcStep(int stepID)
{
	int ret = e_OK;
	
	switch(stepID)
	{
	case eMStepID_BLOB: // blob
		{
			ret = IsMountOK();
		}
		break;
	case eMStepID_PATTERN: // pattern
		{
			ret = MountPatternSearch();
		}
		break;
	case eMStepID_SIZE: // size
		{
			ret = ComponentSize();
		}
		break;
	case eMStepID_ECCENTRICITY: //eccentricity
		{	
			ret = Eccentricity();	
		}
		break;
	case eMStepID_ROTATE: //rotate
		{
			ret = Rotation();
		}
		break;
	case eMStepID_GRADIENT: //gradient
		{
			ret = Gradient();
		}
		break;		
	}

	return ret;
}



void CPInsp_Mount::GetInspRst(RstInspMount* ret)
{
	*ret = m_rstInspMount;
}


//////////////////////////////////////////////////////////////////////////
#pragma region _MOUNT FUNCTION_

int CPInsp_Mount::IsMountOK()
{
	int ret = e_OK;

	float* zmapData = m_inspZmapData.data;
	int zmapSizeX = m_inspZmapData.zmapSizeX;
	int zmapSizeY = m_inspZmapData.zmapSizeY;
	int cx = m_inspCoordinate.cx;
	int cy = m_inspCoordinate.cy;
	int roiSizeX = m_inspCoordinate.width;
	int roiSizeY = m_inspCoordinate.length;
	int range = m_inspParam_Mount->extractRange;   
	double wndAngle = m_inspCoordinate.angle;
	double stdHeight =_mm2micron(m_inspParam_Mount->teachData.hAvr);
	double range_hMax = m_inspParam_Mount->range_MaxHeight;
	double range_hMin = m_inspParam_Mount->range_MinHeight;
	int roiArea = roiSizeX * roiSizeY;

	int bdCx = m_inspCoordinate.anyAngleCx;
	int bdCy = m_inspCoordinate.anyAngleCy;
	int bdSizeX = m_inspCoordinate.anyAngleWidth;
	int bdSizeY = m_inspCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;



	if(cx < 1 || cy < 1 || roiSizeX < 1 || roiSizeY < 1 || zmapData == NULL)
		return ePART_FAIL;

	bool bIsUserDefCop	= m_inspParam_Mount->IsUserDefCop;
	//double* cxCop = new double[4];
	double* cxCop = g_pMManager->pem_new<double>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(cxCop, 0.0, sizeof(double) * 4);
	for(int s = 0; s < 4; s++)
		cxCop[s]	= m_inspParam_Mount->cxList[s];
	//double* cyCop = new double[4];
	double* cyCop = g_pMManager->pem_new<double>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(cyCop, 0.0, sizeof(double) * 4);
	for(int s = 0; s < 4; s++)
		cyCop[s]	= m_inspParam_Mount->cyList[s];
	//double* widthCop = new double[4];
	double* widthCop = g_pMManager->pem_new<double>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(widthCop, 0.0, sizeof(double) * 4);
	for(int s = 0; s < 4; s++)
		widthCop[s]	= m_inspParam_Mount->widthList[s];
	//double* heightCop = new double[4];
	double* heightCop = g_pMManager->pem_new<double>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(heightCop, 0.0, sizeof(double) * 4);
	for(int s = 0; s < 4; s++)
		heightCop[s]	= m_inspParam_Mount->heightList[s];

	float* zmapRoiData = NULL;	
	//////////////////////////////////////////////////////////////////////////
	BOOL isAnyAngle = IsAnyAngle(wndAngle);
	if(isAnyAngle)
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;
		float* bdryZmapTemp = NULL;
		m_proc3d.ProcAnyAngle_Zmap(zmapData, &bdryZmapTemp, zmapSizeX, zmapSizeY, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		m_procMil->SaveWorkImg_float(bdryZmapTemp, w, l, _T("MOUNT_boundary.bmp"));

		wndAngle = 0;
		int tempCx = cx -  (int)(bdCx - (w / 2.0));
		int tempCy = cy -  (int)(bdCy - (l / 2.0));

		//zmapRoiData = new float[roiArea];
		zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(bdryZmapTemp, zmapRoiData, w, l, tempCx, tempCy, roiSizeX, roiSizeY);

		//delete bdryZmapTemp;
		g_pMManager->pem_delete(bdryZmapTemp, true);
	}
	else
	{
		//zmapRoiData = new float[roiArea];
		zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(zmapData, zmapRoiData, zmapSizeX, zmapSizeY, cx, cy, roiSizeX, roiSizeY);//
	}
	
	//////////////////////////////////////////////////////////////////////////
	//binarize
	//UCHAR* binImg = new UCHAR[roiArea];
	UCHAR* binImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	
	float hThreshold = 0;
	hThreshold = (float)(stdHeight * ((double)(100 - range) / 100.0));

	if((stdHeight * range) < 100.0)
		hThreshold = (float)(stdHeight - 100);

	float hThreshold2 = 0;
	hThreshold2 = (float)(stdHeight * 1.8);

	//m_proc3d.MakeZmap2BinImg(zmapRoiData, roiSizeX, roiSizeY, hThreshold, binImg);	//for
	m_proc3d.GetBinImage(zmapRoiData, roiSizeX, roiSizeY, hThreshold, binImg);			//ipp
	//m_proc3d.GetBinImage2(zmapRoiData, roiSizeX, roiSizeY, hThreshold, hThreshold2, binImg);
	m_procMil->SaveWorkImg(binImg, roiSizeX, roiSizeY, _T("newMount.bmp"));

	//////////////////////////////////////////////////////////////////////////
	//huj 2013/12/12   solder 튀는 부분 제거..
	double stdBodyArea = _mm2pixel_x(1.7) * _mm2pixel_y(0.9);
	double inspBodyArea =  _mm2pixel_x(m_inspParam_Mount->teachData.width) * _mm2pixel_y(m_inspParam_Mount->teachData.length);
	if(stdBodyArea > inspBodyArea)
	{		
		//UCHAR* binImg_Noise = new UCHAR[roiArea];
		UCHAR* binImg_Noise = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		//m_proc3d.GetBinImage(zmapRoiData, roiSizeX, roiSizeY, hThreshold2, binImg_Noise);	

		BOOL noiseFlag = m_proc3d.MakeZmap2BinImg2(zmapRoiData, roiSizeX, roiSizeY, hThreshold2, binImg_Noise);	
		m_procMil->SaveWorkImg(binImg_Noise, roiSizeX, roiSizeY, _T("binImg_Noise.bmp"));

		if(noiseFlag == TRUE)
		{
			double marginX = _mm2pixel_x(m_inspParam_Mount->bdryMargin);
			double marginY = _mm2pixel_y(m_inspParam_Mount->bdryMargin);
			int cxTemp = (int)(roiSizeX / 2.0);
			int cyTemp = (int)(roiSizeY / 2.0);
			int wTemp_Half = (int)((roiSizeX - (marginX * 2.0)) / 2.0);
			int lTemp_Half = (int)((roiSizeY - (marginY * 2.0)) / 2.0);
			CRect wndRectTemp(cxTemp - wTemp_Half, cyTemp - lTemp_Half, cxTemp + wTemp_Half, cyTemp + lTemp_Half);
			CRect wndMarginRect(0, 0, roiSizeX, roiSizeY);

			m_procMil->RemakeMountBinImg(binImg, binImg_Noise, binImg, roiSizeX, roiSizeY, wndRectTemp, wndMarginRect);
		}
		//delete binImg_Noise;
		g_pMManager->pem_delete(binImg_Noise, true);
	}

	//////////////////////////////////////////////////////////////////////////
	//blob

	CRect partRect((int)(cx - (m_inspCoordinate.bdrWidth / 2.0)), (int)(cy - (m_inspCoordinate.bdrLength / 2.0)),
		(int)(cx + (m_inspCoordinate.bdrWidth / 2.0)), (int)(cy + (m_inspCoordinate.bdrLength / 2.0)));

	CRect wndRect((int)(cx - (m_inspCoordinate.width / 2.0)), (int)(cy - (m_inspCoordinate.length / 2.0)),
		(int)(cx + (m_inspCoordinate.width / 2.0)), (int)(cy + (m_inspCoordinate.length / 2.0)));

	CRect blobRect(0,0,0,0);
	int blobCx = 0;
	int blobCy = 0;
	//UCHAR* bodyImg = new UCHAR[roiArea];
	UCHAR* bodyImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	int count = m_procMil->MakeBodyImg(binImg, roiSizeX, roiSizeY, partRect, wndRect, bodyImg, &blobRect, &blobCx, &blobCy);
	m_procMil->SaveWorkImg(bodyImg, roiSizeX, roiSizeY, _T("newBlob.bmp"));
	//////////////////////////////////////////////////////////////////////////


	double bodyAngle = 0;
	double cogX_roi_pixel = 0.0;
	double cogY_roi_pixel = 0.0;
	CRect bodyRect;
	float hAvr = 0;
	double bodyWidth = 0;
	double bodyHeight = 0;
	double gradient = 0;

	CPoint nodalPoint[4];


	int tempW = 0;
	int tempH = 0;

	if(count > 0)
	{
		//bodyAngle = m_proc3d.ExtractBody4(bodyImg, cx, cy, roiSizeX, roiSizeY, blobRect, &cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &bodyRect, nodalPoint, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		bodyAngle = m_proc3d.ExtractBodyCv(bodyImg, cx, cy, roiSizeX, roiSizeY, blobRect, &cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &bodyRect, nodalPoint);
		m_procMil->DrawBody(roiSizeX, roiSizeY, nodalPoint);
		//////////////////////////////////////////////////////////////////////////
		
		tempW = (m_inspCoordinate.width > m_inspCoordinate.bdrWidth) ?      m_inspCoordinate.width : m_inspCoordinate.bdrWidth;
		tempH = (m_inspCoordinate.length > m_inspCoordinate.bdrLength) ?      m_inspCoordinate.length : m_inspCoordinate.bdrLength;

		CRect tempRt;
		int cnt = 0;
		int index = 0;

		tempRt.SetRect(0,0, tempW, tempH);
		cnt = CheckRect(tempRt, nodalPoint, 4, &index);

		if(cnt > 0)
		{
			bodyWidth = blobRect.Width();
			bodyHeight =  blobRect.Height();
			if(cnt == 4)
				bodyAngle = 0.0;

			cogX_roi_pixel = (cx - (roiSizeX / 2.0)) + ((blobRect.Width() / 2.0) + blobRect.left);
			cogY_roi_pixel = (cy - (roiSizeY / 2.0)) + ((blobRect.Height() / 2.0) + blobRect.top);

			RemakeNodalPt(nodalPoint, blobRect, 4, cnt, index, nodalPoint);
		}

		hAvr = m_procMil->CalcGradient4(zmapRoiData, roiSizeX, roiSizeY, (int)bodyWidth, (int)bodyHeight, nodalPoint, hThreshold, hThreshold2, (float)stdHeight, bodyAngle, &gradient, 
			bIsUserDefCop, cxCop, cyCop, widthCop, heightCop);
	}
	//////////////////////////////////////////////////////////////////////////


	//각도에따른 w와 h 변환..
	double rstW = bodyWidth;
	double rstH = bodyHeight;

	if(wndAngle == 90 || wndAngle == 270)
	{
		rstW = bodyHeight;
		rstH = bodyWidth;
	}

	//////////////////////////////////////////////////////////////////////////
	// 결과 비교및 저장...
	double cogX_board = 0.0;
	double cogY_board = 0.0;


	CvtPixelToBoard(m_inspCoordinate.fovCx, m_inspCoordinate.fovCy, cogX_roi_pixel, cogY_roi_pixel, &cogX_board, &cogY_board);

	WriteInspBlobResult(hAvr, cogX_board, cogY_board, rstW, rstH, bodyRect, bodyAngle , gradient, gradient);
	InitBlobResultStruct(&m_inspRst);

	m_inspRst.counts = 1;
	m_inspRst.hAvr = hAvr;
	m_inspRst.cogX = (double)cogX_board;
	m_inspRst.cogY = (double)cogY_board;
	m_inspRst.left = (double)bodyRect.left;
	m_inspRst.right = (double)bodyRect.right;
	m_inspRst.top = (double)bodyRect.top;
	m_inspRst.bottom = (double)bodyRect.bottom;
	m_inspRst.width = rstW;
	m_inspRst.length = rstH;
	m_inspRst.area = (double)(rstW *  rstH);
	m_inspRst.gradientAngle_v = gradient;
	m_inspRst.gradientAngle_h = gradient;
	m_inspRst.rotationAngle = bodyAngle;



	//////////////////////////////////////////////////////////////////////////
	//height

	double hMax = stdHeight * ((range_hMax - 100) / 100.0);
	if(hMax < 100)// 최소 100미크론
		hMax = 100;

	double deltaH = m_inspRst.hAvr - stdHeight;
	//////////////////////////////////////////////////////////////////////////
	if(m_inspRst.hAvr < 1)
	{
		ret = e_NG;
		m_rstInspMount.ok = FALSE;

		m_rstInspMount.list.existOK = FALSE;
		m_rstInspMount.list.heightOk = FALSE;

		//defect viewer....
		m_rstInspMount.width = _pixel2mm_x(m_inspRst.width);
		m_rstInspMount.length = _pixel2mm_y(m_inspRst.length); 	
	}
	else if(deltaH > 0 && deltaH > hMax)
	{
		ret = e_NG;
		m_rstInspMount.ok = FALSE;

		m_rstInspMount.list.heightOk = FALSE;

		//defect viewer....
		m_rstInspMount.width = _pixel2mm_x(m_inspRst.width);
		m_rstInspMount.length = _pixel2mm_y(m_inspRst.length); 	
	}
	else
	{
		m_rstInspMount.ok = TRUE;
		m_rstInspMount.list.heightOk = TRUE;
		m_rstInspMount.list.existOK = TRUE;
	}

	m_rstInspMount.bodyHeight = (float)_micron2mm(m_inspRst.hAvr);
	m_rstInspMount.isInsp = TRUE;


	//m_rstInspMount.wndDefectCode = ret;
	//////////////////////////////////////////////////////////////////////////

	/*delete zmapRoiData;
	delete binImg;
	delete bodyImg;

	delete cxCop;
	delete cyCop;
	delete widthCop; 
	delete heightCop;*/
	g_pMManager->pem_delete(zmapRoiData, true);
	g_pMManager->pem_delete(binImg, true);
	g_pMManager->pem_delete(bodyImg, true);
	g_pMManager->pem_delete(cxCop, true);
	g_pMManager->pem_delete(cyCop, true);
	g_pMManager->pem_delete(widthCop, true);
	g_pMManager->pem_delete(heightCop, true);

	return  ret;
}

int CPInsp_Mount::MountPatternSearch()
{
	int ret = e_OK;
	int searchCx = m_inspCoordinate.bdrCx;
	int searchCy = m_inspCoordinate.bdrCy;
	int searchSizeX = m_inspCoordinate.bdrWidth;
	int searchSizeY = m_inspCoordinate.bdrLength;
	
	double wndAngle = m_inspCoordinate.angle;	
	CString modelPath = m_inspParam_Mount->modelPath;
	BOOL isIsnpPatternFlag = m_inspParam_Mount->inspPatternFlag;

	int bdCx = m_inspCoordinate.anyAngleCx;
	int bdCy = m_inspCoordinate.anyAngleCy;
	int bdSizeX = m_inspCoordinate.anyAngleWidth;
	int bdSizeY = m_inspCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	if(isIsnpPatternFlag == FALSE || m_rstInspMount.list.existOK == TRUE)
		return ret;

	if(searchCx < 0 || searchCy < 0 || searchSizeX < 0 || searchSizeY < 0)
		return e_NG;

	if((searchSizeX % 2) != 0)
		searchSizeX += 1;

	if((searchSizeY % 2) != 0)
		searchSizeY += 1;

	int searchArea = searchSizeX * searchSizeY;

	UCHAR* clipSearchImg = NULL;
	//////////////////////////////////////////////////////////////////////////
	BOOL isAnyAngle = IsAnyAngle(wndAngle);
	if(isAnyAngle)
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;
		UCHAR* bdryImgTemp = NULL;
		m_procMil->ProcAnyAngle_Img(m_fovImage_insp, &bdryImgTemp, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		m_procMil->SaveWorkImg(bdryImgTemp, w, l, _T("MPATTERN_boundaryImg.bmp"));

		wndAngle = 0;
		searchCx -= (int)(bdCx - (w / 2.0));
		searchCy -= (int)(bdCy - (l / 2.0));

		//clipSearchImg = new UCHAR[searchArea];
		clipSearchImg = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipImage(bdryImgTemp, w, l, clipSearchImg, searchCx, searchCy, searchSizeX, searchSizeY);
		m_procMil->SaveWorkImg(clipSearchImg, searchSizeX, searchSizeY, _T("MPATTERN_clipSearchImg.bmp"));

		//delete bdryImgTemp;
		g_pMManager->pem_delete(bdryImgTemp, true);
	}
	else
	{
		//clipSearchImg = new UCHAR[searchArea];
		clipSearchImg = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipBuff(m_fovImage_teach, clipSearchImg, searchCx, searchCy, searchSizeX, searchSizeY);
	}


	
	//////////////////////////////////////////////////////////////////////////
	int searchCnt = 0;

	double angleTemp = 0.;
	int modelLoadRst = m_procPatMil->ModelLoad(modelPath);

	
	double cogX_board = 0.0;
	double cogY_board = 0.0;
	if(modelLoadRst == eMNT_SUCCESS)
	{
		angleTemp = 0 + wndAngle;
		if(angleTemp != 0)
		{
			m_procPatMil->SetSearchStartAngle(angleTemp);	
			m_procPatMil->PreprocModel(m_fovImage_insp);
		}
		//searchCnt =	m_procPatMil->SearchPattern(m_fovImage_insp, searchCx, searchCy, searchSizeX, searchSizeY);
		searchCnt =	m_procPatMil->SearchPattern(clipSearchImg, searchSizeX, searchSizeY);

		if(searchCnt > 0)
		{	
			double score = 0;
			double angle = 0;
			double posX = 0;
			double posY = 0;

			m_procPatMil->GetPatResult(&score, &angle, &posX, &posY);

			double cogX_pixel = posX;
			double cogY_pixel = posY;

			
			CvtPixelToBoard(m_inspCoordinate.fovCx, m_inspCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	if(searchCnt > 0)
	{
		if(m_rstInspMount.list.existOK == FALSE)
		{
			
			m_inspRst.cogX = (double)cogX_board;
			m_inspRst.cogY = (double)cogY_board;

			m_rstInspMount.ok = TRUE;
			m_rstInspMount.list.existOK = TRUE;
			m_rstInspMount.list.heightOk = TRUE;

			ret = e_OK;
		}	
	}
	else
	{
		if(m_rstInspMount.list.existOK == FALSE)
		{
			m_rstInspMount.list.mountPatternOK = FALSE;
			ret = e_NG;
		}
	}



	//////////////////////////////////////////////////////////////////////////
	//delete clipSearchImg;
	g_pMManager->pem_delete(clipSearchImg, true);

	return ret;
}

int CPInsp_Mount::ComponentSize()
{
	int ret = e_OK;
	
	double stdWidth = _mm2pixel_x(m_inspParam_Mount->teachData.width);
	double stdLength = _mm2pixel_y(m_inspParam_Mount->teachData.length);
	int minRange_W = m_inspParam_Mount->range_MinWidth;
	int maxRange_W = m_inspParam_Mount->range_MaxWidth;
	int minRange_L = m_inspParam_Mount->range_MinLength;
	int maxRange_L = m_inspParam_Mount->range_MaxLength;

	double tempMinWidth = stdWidth * (minRange_W / 100.0);
	double tempMinLength = stdLength * (minRange_L / 100.0);
	double tempMaxWidth = stdWidth * (maxRange_W / 100.0);
	double tempMaxLength = stdLength * (maxRange_L / 100.0);

	double curWidth = m_inspRst.width;
	double curLength = m_inspRst.length;




	//////////////////////////////////////////////////////////////////////////
/*	if(curWidth < tempMinWidth|| curWidth > tempMaxWidth || curLength < tempMinLength  || curLength > tempMaxLength) // small
	{
		ret = e_NG;
		m_rstInspMount.ok = FALSE;


		//defect viewer....
		if(curWidth < tempMinWidth || curWidth > tempMaxWidth)
		{
			m_rstInspMount.list.widthOk = FALSE;
		}

		if(curLength < tempMinLength || curLength > tempMaxLength)
		{
			m_rstInspMount.list.lengthOk = FALSE;
		}
	}
	else
	{
		if(m_rstInspMount.ok != FALSE)
			m_rstInspMount.ok = TRUE;
	}*/

	if(curWidth < tempMinWidth || curLength < tempMinLength) // small
	{
		ret = e_NG;
		m_rstInspMount.ok = FALSE;


		//defect viewer....
		if(curWidth < tempMinWidth)
		{
			m_rstInspMount.list.widthOk = FALSE;
		}

		if(curLength < tempMinLength)
		{
			m_rstInspMount.list.lengthOk = FALSE;
		}
	}
	else
	{
		if(m_rstInspMount.ok != FALSE)
			m_rstInspMount.ok = TRUE;
	}
	
	//////////////////////////////////////////////////////////////////////////

	m_rstInspMount.width = _pixel2mm_x(curWidth);
	m_rstInspMount.length = _pixel2mm_y(curLength);


	//////////////////////////////////////////////////////////////////////////

	return  ret;
}

int CPInsp_Mount::Eccentricity()
{
	int ret = e_OK;

	double stdCogX = m_inspParam_Mount->teachData.cogX; //mm
	double stdCogY = m_inspParam_Mount->teachData.cogY; //mm

	double range = m_inspParam_Mount->range_Eccentricity; //mm

	double curCogX = m_inspRst.cogX; //mm
	double curCogY = m_inspRst.cogY; //mm


	double offsetX;  //mm
	double offsetY;  //mm
	offsetX = curCogX - stdCogX;	
	offsetY = curCogY - stdCogY;



	//////////////////////////////////////////////////////////////////////////
	//huj 2013/10/18
	if(abs(offsetX) > range || abs(offsetY) > range)
	{
		ret = e_NG;
		m_rstInspMount.ok = FALSE;


		//defect viewer....
		if(abs(offsetX) > range)
			m_rstInspMount.list.offsetXOk = FALSE;

		if(abs(offsetY) > range)
			m_rstInspMount.list.offsetYOk = FALSE;
	}
	else
	{
		if(m_rstInspMount.ok != FALSE)
			m_rstInspMount.ok = TRUE;
	}
	//////////////////////////////////////////////////////////////////////////

	//huj 2013/10/18
	//m_rstInspMount.ok = TRUE;

	m_rstInspMount.offsetX = offsetX;
	m_rstInspMount.offsetY = offsetY;



	//////////////////////////////////////////////////////////////////////////
	//종속 윈도우 이동시킬 값...
	m_bodyOffset->offsetX = (int)_mm2pixel_x(offsetX);
	m_bodyOffset->offsetY = (int)_mm2pixel_y(offsetY);
	


	//////////////////////////////////////////////////////////////////////////

	return  ret;
}

int CPInsp_Mount::Rotation()
{
	int ret  = e_OK;

	double rotationAngle = m_inspRst.rotationAngle;
	double rotationRange = m_inspParam_Mount->range_RotationAngle;

	double maxRange = rotationRange;
	double minRange = rotationRange * -1.0;


	//////////////////////////////////////////////////////////////////////////
	if(rotationAngle > maxRange || rotationAngle < minRange )
	{
		ret = e_NG;
		m_rstInspMount.ok = FALSE;

		//defect viewer....
		m_rstInspMount.list.rotationOk = FALSE;
	}
	else
	{
		if(m_rstInspMount.ok != FALSE)
			m_rstInspMount.ok = TRUE;
	}

	m_rstInspMount.rotationAngle = rotationAngle;

	//m_rstInspMount.wndDefectCode = ret;

	//////////////////////////////////////////////////////////////////////////

	return ret;
}

int CPInsp_Mount::Gradient()
{
	int ret  = e_OK;

	double gradient_v = m_inspRst.gradientAngle_v;
	double gradient_h = m_inspRst.gradientAngle_h;
	double gradientRange = m_inspParam_Mount->range_GradientAngle;

	double maxRange = gradientRange;
	double minRange = gradientRange * -1.0;



	//////////////////////////////////////////////////////////////////////////
	if(gradient_v > maxRange || gradient_v < minRange || gradient_h > maxRange || gradient_h < minRange)
	{
		ret = e_NG;
		m_rstInspMount.ok = FALSE;


		//defect viewer....
		if(gradient_v > maxRange || gradient_v < minRange)
			m_rstInspMount.list.gradientOk_v = FALSE;
		if(gradient_h > maxRange || gradient_h < minRange)
			m_rstInspMount.list.gradientOk_h = FALSE;
	}
	else
	{
		if(m_rstInspMount.ok != FALSE)
			m_rstInspMount.ok = TRUE;
	}

	
	m_rstInspMount.gradientAngle_v = gradient_v;
	m_rstInspMount.gradientAngle_h = gradient_h;

	//m_rstInspMount.wndDefectCode = ret;

	//////////////////////////////////////////////////////////////////////////
	
	return ret;
}

void CPInsp_Mount::DeleteDoubleVal(double** pValue, int nCnt)
{
	if(*pValue != NULL)
	{
		//delete *pValue;
		g_pMManager->pem_delete(*pValue, true);
		*pValue = NULL;
	}
}
void CPInsp_Mount::InitDoubleVal(double** pValue, int nCnt)
{
	DeleteDoubleVal(pValue, nCnt);

	if(nCnt > 0)
	{
		//*pValue = new double[nCnt];
		*pValue = g_pMManager->pem_new<double>(true, nCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(*pValue, 0, sizeof(double) * nCnt);
	}
}

int CPInsp_Mount::Mount_Teach(UCHAR* retTrueImg, int width, int height)
{
	int ret = e_OK;

	float* zmapData = m_teachZmapData.data;
	int zmapSizeX = m_teachZmapData.zmapSizeX;
	int zmapSizeY = m_teachZmapData.zmapSizeY;
	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int roiSizeX = m_teachCoordinate.width;
	int roiSizeY = m_teachCoordinate.length;
	int range = m_teachParam_Mount->extractRange;
	double wndAngle = m_teachCoordinate.angle;
	int roiArea = roiSizeX * roiSizeY;

	int bdCx = m_teachCoordinate.anyAngleCx;
	int bdCy = m_teachCoordinate.anyAngleCy;
	int bdSizeX = m_teachCoordinate.anyAngleWidth;
	int bdSizeY = m_teachCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	BOOL isNoiseRemove = m_teachParam_Mount->noiseRemove;
	double hThresh2 = _mm2micron(m_teachParam_Mount->heightLimit);

	bool bIsUserDefCop	= m_teachParam_Mount->IsUserDefCop;
	//double* cxCop = new double[4];
	double* cxCop = g_pMManager->pem_new<double>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(cxCop, 0.0, sizeof(double) * 4);
	for(int s = 0; s < 4; s++)
		cxCop[s]	= m_teachParam_Mount->cxList[s];
	//double* cyCop = new double[4];
	double* cyCop = g_pMManager->pem_new<double>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(cyCop, 0.0, sizeof(double) * 4);
	for(int s = 0; s < 4; s++)
		cyCop[s]	= m_teachParam_Mount->cyList[s];
	//double* widthCop = new double[4];
	double* widthCop = g_pMManager->pem_new<double>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(widthCop, 0.0, sizeof(double) * 4);
	for(int s = 0; s < 4; s++)
		widthCop[s]	= m_teachParam_Mount->widthList[s];
	//double* heightCop = new double[4];
	double* heightCop = g_pMManager->pem_new<double>(true, 4, (PCHAR)__FUNCTION__, __LINE__);
	memset(heightCop, 0.0, sizeof(double) * 4);
	for(int s = 0; s < 4; s++)
		heightCop[s]	= m_teachParam_Mount->heightList[s];


	BOOL inspPatternFlag = m_teachParam_Mount->inspPatternFlag;

	if(cx < 1 || cy < 1 || roiSizeX < 1 || roiSizeY < 1 || zmapData == NULL)
		return ePART_FAIL;


	float* zmapRoiData = NULL;
	UCHAR* bdryImg = NULL;	
	//////////////////////////////////////////////////////////////////////////
	BOOL isAnyAngle = IsAnyAngle(wndAngle);

	if(isAnyAngle)
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;
		float* bdryZmapTemp = NULL;
		m_proc3d.ProcAnyAngle_Zmap(zmapData, &bdryZmapTemp, zmapSizeX, zmapSizeY, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		m_procMil->SaveWorkImg_float(bdryZmapTemp, w, l, _T("MOUNT_boundary.bmp"));
		
		UCHAR* bdryImgTemp = NULL;
		m_procMil->ProcAnyAngle_Img(m_fovImage_teach, &bdryImgTemp, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, NULL, NULL);
		m_procMil->SaveWorkImg(bdryImgTemp, w, l, _T("MOUNT_boundaryImg.bmp"));


		wndAngle = 0;
		int tempCx = cx -  (int)(bdCx - (w / 2.0));
		int tempCy = cy -  (int)(bdCy - (l / 2.0));

		//zmapRoiData = new float[roiArea];
		zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(bdryZmapTemp, zmapRoiData, w, l, tempCx, tempCy, roiSizeX, roiSizeY);

		//bdryImg = new UCHAR[roiArea];
		bdryImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipImage(bdryImgTemp, w, l, bdryImg, tempCx, tempCy, roiSizeX, roiSizeY);

		/*delete bdryZmapTemp;
		delete bdryImgTemp;*/
		g_pMManager->pem_delete(bdryZmapTemp, true);
		g_pMManager->pem_delete(bdryImgTemp, true);
	}
	else
	{
		//zmapRoiData = new float[roiArea];
		zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(zmapData, zmapRoiData, zmapSizeX, zmapSizeY, cx, cy, roiSizeX, roiSizeY);
	}

	//////////////////////////////////////////////////////////////////////////

	UCHAR* binImg = NULL;
	//binImg = new UCHAR[roiArea];
	binImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);

	float thresh = 0;
	BOOL flag = m_proc3d.GetRoughGrayImage(zmapRoiData, roiSizeX, roiSizeY, range, &thresh, binImg, NULL);
	m_procMil->SaveWorkImg(binImg, roiSizeX, roiSizeY, _T("boardGrayImg.bmp"));

	//////////////////////////////////////////////////////////////////////////
	//huj 2013/12/12   solder 튀는 부분 제거..
	if(isNoiseRemove)
	{	
		//UCHAR* binImg_Noise = new UCHAR[roiArea];
		UCHAR* binImg_Noise = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		BOOL noiseFlag = m_proc3d.MakeZmap2BinImg2(zmapRoiData, roiSizeX, roiSizeY, (float)hThresh2, binImg_Noise);	
		//m_procMil->SaveWorkImg(binImg_Noise, roiSizeX, roiSizeY, _T("binImg_Noise.bmp"));

		if(noiseFlag == TRUE)
		{
			double marginX = _mm2pixel_x(m_teachParam_Mount->bdryMargin);
			double marginY = _mm2pixel_y(m_teachParam_Mount->bdryMargin);
			int cxTemp = (int)(roiSizeX / 2.0);
			int cyTemp = (int)(roiSizeY / 2.0);
			int wTemp_Half = (int)((roiSizeX - (marginX * 2.0)) / 2.0);
			int lTemp_Half = (int)((roiSizeY - (marginY * 2.0)) / 2.0);
			CRect wndRectTemp(cxTemp - wTemp_Half, cyTemp - lTemp_Half, cxTemp + wTemp_Half, cyTemp + lTemp_Half);
			CRect wndMarginRect(0, 0, roiSizeX, roiSizeY);

			m_procMil->RemakeMountBinImg(binImg, binImg_Noise, binImg, roiSizeX, roiSizeY, wndRectTemp, wndMarginRect);
		}
		//delete binImg_Noise;
		g_pMManager->pem_delete(binImg_Noise, true);
	}

	//////////////////////////////////////////////////////////////////////////
	//blob

	CRect partRect((int)(cx - (m_teachCoordinate.bdrWidth / 2.0)), (int)(cy - (m_teachCoordinate.bdrLength / 2.0)),
		(int)(cx + (m_teachCoordinate.bdrWidth / 2.0)), (int)(cy + (m_teachCoordinate.bdrLength / 2.0)));

	CRect wndRect((int)(cx - (m_teachCoordinate.width / 2.0)), (int)(cy - (m_teachCoordinate.length / 2.0)),
		(int)(cx + (m_teachCoordinate.width / 2.0)), (int)(cy + (m_teachCoordinate.length / 2.0)));


	CRect blobRect(0,0,0,0);
	int blobCx = 0;
	int blobCy = 0;
	//UCHAR* bodyImg = new UCHAR[roiArea];
	UCHAR* bodyImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	int count = m_procMil->MakeBodyImg2(binImg, roiSizeX, roiSizeY, partRect, wndRect, bodyImg, &blobRect, &blobCx, &blobCy);//

	m_procMil->SaveWorkImg(bodyImg, roiSizeX, roiSizeY, _T("newBlob.bmp"));
	//////////////////////////////////////////////////////////////////////////

	double bodyAngle = 0;
	double cogX_roi_pixel = 0.0;
	double cogY_roi_pixel = 0.0;
	CRect bodyRect;
	float hAvr = 0;
	double bodyWidth = 0;
	double bodyHeight = 0;
	double gradient = 0;

	CPoint nodalPoint[4];

	int tempW = 0;
	int tempH = 0;

	if(count > 0)
	{
		//bodyAngle = m_proc3d.ExtractBody4(bodyImg, cx, cy, roiSizeX, roiSizeY, blobRect, &cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &bodyRect, nodalPoint, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		bodyAngle = m_proc3d.ExtractBodyCv(bodyImg, cx, cy, roiSizeX, roiSizeY, blobRect, &cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &bodyRect, nodalPoint);
		m_procMil->DrawBody(roiSizeX, roiSizeY, nodalPoint);
		//////////////////////////////////////////////////////////////////////////

		tempW = (m_teachCoordinate.width > m_teachCoordinate.bdrWidth) ?      m_teachCoordinate.width : m_teachCoordinate.bdrWidth;
		tempH = (m_teachCoordinate.length > m_teachCoordinate.bdrLength) ?      m_teachCoordinate.length : m_teachCoordinate.bdrLength;

		CRect tempRt;
		int cnt = 0;
		int index = 0;

		tempRt.SetRect(0,0, tempW, tempH);
		cnt = CheckRect(tempRt, nodalPoint, 4, &index);

		if(cnt > 0)
		{
			bodyWidth = blobRect.Width();
			bodyHeight =  blobRect.Height();

			cogX_roi_pixel = (cx - (roiSizeX / 2.0)) + ((blobRect.Width() / 2.0) + blobRect.left);
			cogY_roi_pixel = (cy - (roiSizeY / 2.0)) + ((blobRect.Height() / 2.0) + blobRect.top);

			RemakeNodalPt(nodalPoint, blobRect, 4, cnt, index, nodalPoint);
		}

		
		hAvr = m_procMil->CalcGradient4(zmapRoiData, roiSizeX, roiSizeY, (int)bodyWidth, (int)bodyHeight, nodalPoint, thresh, 0, 0, bodyAngle, &gradient, 
			bIsUserDefCop, cxCop, cyCop, widthCop, heightCop);
		//hAvr = m_procMil->CalcGradient4(zmapRoiData, roiSizeX, roiSizeY, (int)bodyWidth, (int)bodyHeight, nodalPoint, thresh, 0, 0, bodyAngle, &gradient);


		//////////////////////////////////////////////////////////////////////////
		//각도에따른 w와 h 변환..
		double rstW = bodyWidth;
		double rstH = bodyHeight;

		if(wndAngle == 90 || wndAngle == 270)
		{
			rstW = bodyHeight;
			rstH = bodyWidth;
		}

		//////////////////////////////////////////////////////////////////////////
		// 결과 비교및 저장...
		double cogX_board = 0.0;
		double cogY_board = 0.0;

		CvtPixelToBoard(m_teachCoordinate.fovCx, m_teachCoordinate.fovCy, cogX_roi_pixel, cogY_roi_pixel, &cogX_board, &cogY_board);

		WriteTeachBlobResult(hAvr, cogX_board, cogY_board, rstW, rstH, bodyRect, bodyAngle , gradient, gradient);

		if(inspPatternFlag == TRUE)
			MountPatternTeach(cogX_roi_pixel, cogY_roi_pixel);


		if(retTrueImg != NULL)
		{
			//UCHAR* maskImg = new UCHAR[roiArea];
			UCHAR* maskImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
			m_procMil->MakeBodyMaskImg(bodyImg, roiSizeX, roiSizeY, bodyRect, maskImg);

			//UCHAR* trueImg = new UCHAR[roiArea];
			UCHAR* trueImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);

			if(!isAnyAngle)
				m_procMil->MaskTrueImg(m_fovImage_teach, maskImg, trueImg, cx, cy, roiSizeX, roiSizeY);
			else
				m_procMil->MaskTrueImg(bdryImg, maskImg, trueImg, roiSizeX, roiSizeY);


			//UCHAR* reSIzeImg = new UCHAR[width * height];
			UCHAR* reSIzeImg = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
			double factor = m_procMil->GetResizeFactor(width, height, roiSizeX, roiSizeY);
			m_procMil->ResizeImg(trueImg, reSIzeImg, bodyRect, roiSizeX, roiSizeY, width, height, factor, TRUE, FALSE);


			double rotateAngle = GetRotateAngle(roiSizeX, roiSizeY, wndAngle, NULL, NULL);

			UCHAR* rotateImg = NULL;
			if(rotateAngle > 0)
			{
				//rotateImg = new UCHAR[width * height];
				rotateImg = g_pMManager->pem_new<UCHAR>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
				m_procMil->RotateImg(reSIzeImg, width, height, rotateAngle, rotateImg);
				memcpy_s(retTrueImg, sizeof(UCHAR) * width * height, rotateImg, sizeof(UCHAR) * width * height);

				//delete rotateImg;
				g_pMManager->pem_delete(rotateImg, true);
			}
			else
			{
				memcpy_s(retTrueImg, sizeof(UCHAR) * width * height, reSIzeImg, sizeof(UCHAR) * width * height);
			}


			/*delete maskImg;
			delete trueImg;
			delete reSIzeImg;*/
			g_pMManager->pem_delete(maskImg, true);
			g_pMManager->pem_delete(trueImg, true);
			g_pMManager->pem_delete(reSIzeImg, true);

		}
	}
	else
	{
		ret = e_NG;
	}
	
	/*delete cxCop;
	delete cyCop;
	delete widthCop; 
	delete heightCop;*/
	g_pMManager->pem_delete(cxCop, true);
	g_pMManager->pem_delete(cyCop, true);
	g_pMManager->pem_delete(widthCop, true);
	g_pMManager->pem_delete(heightCop, true);

	//////////////////////////////////////////////////////////////////////////

	if(zmapRoiData != NULL)
		//delete zmapRoiData;
		g_pMManager->pem_delete(zmapRoiData, true);

	if(binImg != NULL)
		//delete binImg;
		g_pMManager->pem_delete(binImg, true);

	if(bodyImg != NULL)
		//delete bodyImg;
		g_pMManager->pem_delete(bodyImg, true);

	if(bdryImg != NULL)
		//delete bdryImg;
		g_pMManager->pem_delete(bdryImg, true);

	return ret;
}

#if 0

int CPInsp_Mount::Mount_Teach(UCHAR* retTrueImg, int width, int height)
{
	int ret = e_OK;

	float* zmapData = m_teachZmapData.data;
	int zmapSizeX = m_teachZmapData.zmapSizeX;
	int zmapSizeY = m_teachZmapData.zmapSizeY;
	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int roiSizeX = m_teachCoordinate.width;
	int roiSizeY = m_teachCoordinate.length;
	int range = m_teachParam_Mount->extractRange;
	double wndAngle = m_teachCoordinate.angle;
	int roiArea = roiSizeX * roiSizeY;

	BOOL isNoiseRemove = m_teachParam_Mount->noiseRemove;
	double hThresh2 = _mm2micron(m_teachParam_Mount->heightLimit);

	BOOL inspPatternFlag = m_teachParam_Mount->inspPatternFlag;

	if(cx < 1 || cy < 1 || roiSizeX < 1 || roiSizeY < 1 || zmapData == NULL)
		return ePART_FAIL;

	float* zmapRoiData = new float[roiArea];
	m_proc3d.GetCropZmap(zmapData, zmapRoiData, zmapSizeX, zmapSizeY, cx, cy, roiSizeX, roiSizeY);


	UCHAR* binImg = NULL;
	binImg = new UCHAR[roiArea];

	float thresh = 0;
	BOOL flag = m_proc3d.GetRoughGrayImage(zmapRoiData, roiSizeX, roiSizeY, range, &thresh, binImg, NULL);
	m_procMil->SaveWorkImg(binImg, roiSizeX, roiSizeY, _T("boardGrayImg.bmp"));

	//////////////////////////////////////////////////////////////////////////
	//huj 2013/12/12   solder 튀는 부분 제거..
	if(isNoiseRemove)
	{	
		UCHAR* binImg_Noise = new UCHAR[roiArea];
		BOOL noiseFlag = m_proc3d.MakeZmap2BinImg2(zmapRoiData, roiSizeX, roiSizeY, (float)hThresh2, binImg_Noise);	
		//m_procMil->SaveWorkImg(binImg_Noise, roiSizeX, roiSizeY, _T("binImg_Noise.bmp"));

		if(noiseFlag == TRUE)
		{
			double marginX = _mm2pixel_x(m_teachParam_Mount->bdryMargin);
			double marginY = _mm2pixel_y(m_teachParam_Mount->bdryMargin);
			int cxTemp = (int)(roiSizeX / 2.0);
			int cyTemp = (int)(roiSizeY / 2.0);
			int wTemp_Half = (int)((roiSizeX - (marginX * 2.0)) / 2.0);
			int lTemp_Half = (int)((roiSizeY - (marginY * 2.0)) / 2.0);
			CRect wndRectTemp(cxTemp - wTemp_Half, cyTemp - lTemp_Half, cxTemp + wTemp_Half, cyTemp + lTemp_Half);
			CRect wndMarginRect(0, 0, roiSizeX, roiSizeY);

			m_procMil->RemakeMountBinImg(binImg, binImg_Noise, binImg, roiSizeX, roiSizeY, wndRectTemp, wndMarginRect);
		}
		delete binImg_Noise;
	}

	//////////////////////////////////////////////////////////////////////////
	//blob

	CRect partRect((int)(cx - (m_teachCoordinate.bdrWidth / 2.0)), (int)(cy - (m_teachCoordinate.bdrLength / 2.0)),
		(int)(cx + (m_teachCoordinate.bdrWidth / 2.0)), (int)(cy + (m_teachCoordinate.bdrLength / 2.0)));

	CRect wndRect((int)(cx - (m_teachCoordinate.width / 2.0)), (int)(cy - (m_teachCoordinate.length / 2.0)),
		(int)(cx + (m_teachCoordinate.width / 2.0)), (int)(cy + (m_teachCoordinate.length / 2.0)));


	CRect blobRect(0,0,0,0);
	int blobCx = 0;
	int blobCy = 0;
	UCHAR* bodyImg = new UCHAR[roiArea];
	int count = m_procMil->MakeBodyImg2(binImg, roiSizeX, roiSizeY, partRect, wndRect, bodyImg, &blobRect, &blobCx, &blobCy);

	m_procMil->SaveWorkImg(bodyImg, roiSizeX, roiSizeY, _T("newBlob.bmp"));
	//////////////////////////////////////////////////////////////////////////

	double bodyAngle = 0;
	double cogX_roi_pixel = 0.0;
	double cogY_roi_pixel = 0.0;
	CRect bodyRect;
	float hAvr = 0;
	double bodyWidth = 0;
	double bodyHeight = 0;
	double gradient = 0;

	CPoint nodalPoint[4];

	int tempW = 0;
	int tempH = 0;

	if(count > 0)
	{
		//bodyAngle = m_proc3d.ExtractBody4(bodyImg, cx, cy, roiSizeX, roiSizeY, blobRect, &cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &bodyRect, nodalPoint, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		bodyAngle = m_proc3d.ExtractBodyCv(bodyImg, cx, cy, roiSizeX, roiSizeY, blobRect, &cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &bodyRect, nodalPoint);
		m_procMil->DrawBody(roiSizeX, roiSizeY, nodalPoint);
		//////////////////////////////////////////////////////////////////////////

		tempW = (m_teachCoordinate.width > m_teachCoordinate.bdrWidth) ?      m_teachCoordinate.width : m_teachCoordinate.bdrWidth;
		tempH = (m_teachCoordinate.length > m_teachCoordinate.bdrLength) ?      m_teachCoordinate.length : m_teachCoordinate.bdrLength;

		CRect tempRt;
		int cnt = 0;
		int index = 0;

		tempRt.SetRect(0,0, tempW, tempH);
		cnt = CheckRect(tempRt, nodalPoint, 4, &index);

		if(cnt > 0)
		{
			bodyWidth = blobRect.Width();
			bodyHeight =  blobRect.Height();

			cogX_roi_pixel = (cx - (roiSizeX / 2.0)) + ((blobRect.Width() / 2.0) + blobRect.left);
			cogY_roi_pixel = (cy - (roiSizeY / 2.0)) + ((blobRect.Height() / 2.0) + blobRect.top);

			RemakeNodalPt(nodalPoint, blobRect, 4, cnt, index, nodalPoint);
		}


		hAvr = m_procMil->CalcGradient4(zmapRoiData, roiSizeX, roiSizeY, (int)bodyWidth, (int)bodyHeight, nodalPoint, thresh, 0, 0, &gradient);


		//////////////////////////////////////////////////////////////////////////
		//각도에따른 w와 h 변환..
		double rstW = bodyWidth;
		double rstH = bodyHeight;

		if(wndAngle == 90 || wndAngle == 270)
		{
			rstW = bodyHeight;
			rstH = bodyWidth;
		}

		//////////////////////////////////////////////////////////////////////////
		// 결과 비교및 저장...
		double cogX_board = 0.0;
		double cogY_board = 0.0;

		CvtPixelToBoard(m_teachCoordinate.fovCx, m_teachCoordinate.fovCy, cogX_roi_pixel, cogY_roi_pixel, &cogX_board, &cogY_board);

		WriteTeachBlobResult(hAvr, cogX_board, cogY_board, rstW, rstH, bodyRect, bodyAngle , gradient, gradient);

		if(inspPatternFlag == TRUE)
			MountPatternTeach(cogX_roi_pixel, cogY_roi_pixel);


		if(retTrueImg != NULL)
		{
			UCHAR* maskImg = new UCHAR[roiArea];
			m_procMil->MakeBodyMaskImg(bodyImg, roiSizeX, roiSizeY, bodyRect, maskImg);

			UCHAR* trueImg = new UCHAR[roiArea];
			m_procMil->MaskTrueImg(m_fovImage_teach, maskImg, trueImg, cx, cy, roiSizeX, roiSizeY);


			UCHAR* reSIzeImg = new UCHAR[width * height];
			double factor = m_procMil->GetResizeFactor(width, height, roiSizeX, roiSizeY);
			m_procMil->ResizeImg(trueImg, reSIzeImg, bodyRect, roiSizeX, roiSizeY, width, height, factor, TRUE);


			double rotateAngle = GetRotateAngle(roiSizeX, roiSizeY, wndAngle, NULL, NULL);

			UCHAR* rotateImg = NULL;
			if(rotateAngle > 0)
			{
				rotateImg = new UCHAR[width * height];
				m_procMil->RotateImg(reSIzeImg, width, height, rotateAngle, rotateImg);
				memcpy_s(retTrueImg, sizeof(UCHAR) * width * height, rotateImg, sizeof(UCHAR) * width * height);

				delete rotateImg;
			}
			else
			{
				memcpy_s(retTrueImg, sizeof(UCHAR) * width * height, reSIzeImg, sizeof(UCHAR) * width * height);
			}


			delete maskImg;
			delete trueImg;
			delete reSIzeImg;

		}
	}
	else
	{
		ret = e_NG;
	}



	//////////////////////////////////////////////////////////////////////////

	if(zmapRoiData != NULL)
		delete zmapRoiData;

	if(binImg != NULL)
		delete binImg;

	if(bodyImg != NULL)
		delete bodyImg;



	return ret;
}
#endif


int CPInsp_Mount::Mount_GetBodyRect(RECT* retRect)
{
	int ret = ePART_SUCCESS; 
	RECT rectTemp;

	float* zmapData = m_teachZmapData.data;
	int zmapSizeX = m_teachZmapData.zmapSizeX;
	int zmapSizeY = m_teachZmapData.zmapSizeY;
	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int roiSizeX = m_teachCoordinate.width;
	int roiSizeY = m_teachCoordinate.length;
	double range = 20.0;//m_teachParam_Mount->extractRange;
	double wndAngle = m_teachCoordinate.angle;
	int roiArea = roiSizeX * roiSizeY;


	int bdCx = m_teachCoordinate.anyAngleCx;
	int bdCy = m_teachCoordinate.anyAngleCy;
	int bdSizeX = m_teachCoordinate.anyAngleWidth;
	int bdSizeY = m_teachCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;


	if(roiSizeX < 1 || roiSizeY < 1 || zmapData == NULL)
		return ePART_FAIL;

	if(wndAngle != 0.0)
	{
		rectTemp.left = 0;
		rectTemp.right = 0;
		rectTemp.top = 0;
		rectTemp.bottom = 0;

		*retRect = rectTemp;

		return ePART_FAIL;
	}


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
		m_procMil->SaveWorkImg_float(bdryZmapTemp, w, l, _T("MOUNT_boundary.bmp"));

		wndAngle = 0;
		int tempCx = cx -  (int)(bdCx - (w / 2.0));
		int tempCy = cy -  (int)(bdCy - (l / 2.0));

		//zmapRoiData = new float[roiArea];
		zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(bdryZmapTemp, zmapRoiData, w, l, tempCx, tempCy, roiSizeX, roiSizeY);

		//delete bdryZmapTemp;
		g_pMManager->pem_delete(bdryZmapTemp, true);
	}
	else
	{
		//zmapRoiData = new float[roiArea];
		zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_proc3d.GetCropZmap(zmapData, zmapRoiData, zmapSizeX, zmapSizeY, cx, cy, roiSizeX, roiSizeY);
	}


	//////////////////////////////////////////////////////////////////////////



	UCHAR* binImg = NULL;
	//binImg = new UCHAR[roiArea];
	binImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);

	float thresh = 0;
	BOOL flag = m_proc3d.GetRoughGrayImage(zmapRoiData, roiSizeX, roiSizeY, range, &thresh, binImg, NULL);
	//m_procMil->SaveWorkImg(binImg, roiSizeX, roiSizeY, _T("boardGrayImg.bmp"));


	//////////////////////////////////////////////////////////////////////////
	//blob
	int partW = m_teachCoordinate.bdrWidth;
	int partH = m_teachCoordinate.bdrLength;
	CRect partRect((int)(cx - (partW / 2.0)), (int)(cy - (partH / 2.0)), (int)(cx + (partW / 2.0)), (int)(cy + (partH / 2.0)));

	int wndW = m_teachCoordinate.width;
	int wndH = m_teachCoordinate.length;
	CRect wndRect((int)(cx - (wndW / 2.0)), (int)(cy - (wndH / 2.0)), (int)(cx + (wndW / 2.0)), (int)(cy + (wndH / 2.0)));

	CRect blobRect(0,0,0,0);
	int blobCx = 0;
	int blobCy = 0;
	//UCHAR* bodyImg = new UCHAR[roiArea];
	UCHAR* bodyImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	int blobCnt = m_procMil->MakeBodyImg2(binImg, roiSizeX, roiSizeY, partRect, wndRect, bodyImg, &blobRect, &blobCx, &blobCy);

	//m_procMil->SaveWorkImg(bodyImg, roiSizeX, roiSizeY, _T("newBlob.bmp"));
	//////////////////////////////////////////////////////////////////////////

	double cogX_roi_pixel = 0.0;
	double cogY_roi_pixel = 0.0;
	CRect bodyRect;

	double bodyAngle = 0;
	double bodyWidth = 0;
	double bodyHeight = 0;
	if(blobCnt > 0)
	{
		//bodyAngle = m_proc3d.ExtractBody4(bodyImg, cx, cy, roiSizeX, roiSizeY, blobRect, &cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &bodyRect, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);	
		bodyAngle = m_proc3d.ExtractBodyCv(bodyImg, cx, cy, roiSizeX, roiSizeY, blobRect, &cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &bodyRect, NULL);
		int marginX = 5;//(int)((rt.Width() * 0.2) / 2.0); //half
		int marginY = 5;//(int)((rt.Height() * 0.2) / 2.0); //half

		rectTemp.left =(LONG)(bodyRect.left - marginX);
		rectTemp.right = (LONG)(bodyRect.right + marginX);
		rectTemp.top = (LONG)(bodyRect.top - marginY);
		rectTemp.bottom = (LONG)(bodyRect.bottom + marginY);
	}
	else
	{
		rectTemp.left = 0;
		rectTemp.right = 0;
		rectTemp.top = 0;
		rectTemp.bottom = 0;
	}


	//huj 2014/01/15
	if(rectTemp.left < 0 || rectTemp.left >= m_fovWidth || rectTemp.right <= 0 || rectTemp.right >= m_fovWidth || rectTemp.top < 0 || rectTemp.top >= m_fovLength || rectTemp.bottom <= 0 || rectTemp.bottom >= m_fovLength)
	{
		rectTemp.left = 0;
		rectTemp.right = 0;
		rectTemp.top = 0;
		rectTemp.bottom = 0;

		// 이 경우는 외부에서 실패로 처리하겠습니다.
		ret = ePART_FAIL;
	}

	*retRect = rectTemp;


//////////////////////////////////////////////////////////////////////////

	if(binImg !=NULL)
		//delete binImg;
		g_pMManager->pem_delete(binImg, true);

	if(bodyImg !=NULL)
		//delete bodyImg;
		g_pMManager->pem_delete(bodyImg, true);

	if(zmapRoiData !=NULL)
		//delete zmapRoiData;
		g_pMManager->pem_delete(zmapRoiData, true);

	

	return ret;
}


double CPInsp_Mount::GetRotateAngle(int wndW, int wndH, double wndAngle, int* retW, int* retH)
{
	int w = wndW;
	int h = wndH;
	double angle = (wndAngle == 0)?  0 : (360 - wndAngle);

	if(angle == 90 || angle == 270)
	{
		w = wndH;
		h = wndW;
	}

	if(retW != NULL)
		*retW = w;
	if(retH != NULL)
		*retH = h;

	return angle;
}

CRect CPInsp_Mount::RotateRect(CRect srcRect, double angle)
{
	CRect retRect = srcRect;

	if(angle < 0)
		return retRect;

	int w = srcRect.Width();
	int h = srcRect.Height();
	CPoint c = srcRect.CenterPoint();

	if(angle == 90 || angle == 270)
	{
		retRect.left = (LONG)(c.x - (h / 2.0));
		retRect.right = (LONG)(c.x + (h / 2.0));
		retRect.top = (LONG)(c.y - (w / 2.0));
		retRect.bottom = (LONG)(c.y + (w / 2.0));
	}

	return retRect;
}

void CPInsp_Mount::WriteTeachBlobResult(float hAvr, double cogX, double cogY, double width, double height, CRect rect, double rotAng, double graAng_v, double graAng_h)
{
	if(!m_teachParam_Mount)
		return;

	InitBlobResultStruct(&m_teachParam_Mount->retTeachResult); 

	m_teachParam_Mount->retTeachResult.counts = 1;
	m_teachParam_Mount->retTeachResult.hAvr	  = (float)_micron2mm(hAvr);
	m_teachParam_Mount->retTeachResult.cogX   = cogX;
	m_teachParam_Mount->retTeachResult.cogY   = cogY;
	m_teachParam_Mount->retTeachResult.area   = _pixel2mm_x(rect.Width()) * _pixel2mm_y(rect.Height());
	m_teachParam_Mount->retTeachResult.left   = _pixel2mm_x(rect.left);
	m_teachParam_Mount->retTeachResult.right  = _pixel2mm_x(rect.right);
	m_teachParam_Mount->retTeachResult.top    = _pixel2mm_y(rect.top);
	m_teachParam_Mount->retTeachResult.bottom = _pixel2mm_y(rect.bottom);
	m_teachParam_Mount->retTeachResult.width  = _pixel2mm_x(width);
	m_teachParam_Mount->retTeachResult.length = _pixel2mm_y(height);

	m_teachParam_Mount->retTeachResult.rotationAngle = rotAng;
	m_teachParam_Mount->retTeachResult.gradientAngle_v = graAng_v;
	m_teachParam_Mount->retTeachResult.gradientAngle_h = graAng_h;

}

void CPInsp_Mount::WriteInspBlobResult(float hAvr, double cogX, double cogY, double width, double height, CRect rect, double rotAng, double graAng_v, double graAng_h)
{
	m_inspRst.counts = 1;
	m_inspRst.hAvr	  = (float)_micron2mm(hAvr);
	m_inspRst.cogX   = cogX;
	m_inspRst.cogY   = cogY;
	m_inspRst.area   = _pixel2mm_x(rect.Width()) * _pixel2mm_y(rect.Height());
	m_inspRst.left   = _pixel2mm_x(rect.left);
	m_inspRst.right  = _pixel2mm_x(rect.right);
	m_inspRst.top    = _pixel2mm_y(rect.top);
	m_inspRst.bottom = _pixel2mm_y(rect.bottom);
	m_inspRst.width  = _pixel2mm_x(width);
	m_inspRst.length = _pixel2mm_y(height);

	m_inspRst.rotationAngle = rotAng;
	m_inspRst.gradientAngle_v = graAng_v;
	m_inspRst.gradientAngle_h = graAng_h;

	if(m_inspParam_Mount)
		m_inspParam_Mount->retBlobRst = m_inspRst;
}


void CPInsp_Mount::WriteTeachBlobResult(CMilBlobResult* rst, float hAvr)
{
	InitBlobResultStruct(&m_teachParam_Mount->retTeachResult); 

	double width = 0.0;
	double length = 0.0;

	CalcBlobSize(rst, &width, & length);

	m_teachParam_Mount->retTeachResult.counts = rst->count;
	m_teachParam_Mount->retTeachResult.hAvr	  = (rst->count != 0)?		 (float)_micron2mm(hAvr) : 0;
	m_teachParam_Mount->retTeachResult.cogX   = (rst->count != 0)?		 _pixel2mm_x(rst->cx[0]) : 0.0;
	m_teachParam_Mount->retTeachResult.cogY   = (rst->count != 0)?		 _pixel2mm_y(rst->cy[0]) : 0.0;
	m_teachParam_Mount->retTeachResult.area   = (rst->count != 0)?		 rst->area[0] : 0.0;	
	m_teachParam_Mount->retTeachResult.left   = (rst->count != 0)?		 rst->left[0] : 0.0;
	m_teachParam_Mount->retTeachResult.right  = (rst->count != 0)?		 rst->right[0] : 0.0;
	m_teachParam_Mount->retTeachResult.top    = (rst->count != 0)?		 rst->top[0] : 0.0;
	m_teachParam_Mount->retTeachResult.bottom = (rst->count != 0)?		 rst->bottom[0] : 0.0;
	m_teachParam_Mount->retTeachResult.width  = (rst->count != 0)?	     _pixel2mm_x(width) : 0.0;
	m_teachParam_Mount->retTeachResult.length = (rst->count != 0)?	     _pixel2mm_y(length) : 0.0;
}

void CPInsp_Mount::WriteInspBlobResult(CMilBlobResult* rst, float hAvr)
{	

	double width = 0.0;
	double length = 0.0;

	CalcBlobSize(rst, &width, & length);

	m_inspParam_Mount->retBlobRst.counts = rst->count;
	m_inspParam_Mount->retBlobRst.hAvr	  = (rst->count != 0)?		(float)_micron2mm(hAvr) : 0;
	m_inspParam_Mount->retBlobRst.cogX   = (rst->count != 0)?		 _pixel2mm_x(rst->cx[0]) : 0.0;
	m_inspParam_Mount->retBlobRst.cogY   = (rst->count != 0)?		 _pixel2mm_y(rst->cy[0]) : 0.0;
	m_inspParam_Mount->retBlobRst.area   = (rst->count != 0)?		 rst->area[0] : 0.0;
	m_inspParam_Mount->retBlobRst.left   = (rst->count != 0)?		 rst->left[0] : 0.0;
	m_inspParam_Mount->retBlobRst.right  = (rst->count != 0)?		 rst->right[0] : 0.0;
	m_inspParam_Mount->retBlobRst.top    = (rst->count != 0)?		 rst->top[0] : 0.0;
	m_inspParam_Mount->retBlobRst.bottom = (rst->count != 0)?		 rst->bottom[0] : 0.0;
	m_inspParam_Mount->retBlobRst.width  = (rst->count != 0)?		 _pixel2mm_x(width) : 0.0;
	m_inspParam_Mount->retBlobRst.length = (rst->count != 0)?		 _pixel2mm_y(length) : 0.0;
}

void CPInsp_Mount::CalcBlobSize(CMilBlobResult* blob, double* retWidth, double* retLength)
{
	double length = 0.0;
	double width = 0.0;
	double angle = blob->angle[0];


	if(angle >= -45.0 && angle <= 45.0)
	{
		width = blob->diameter[0];
		length = (blob->area[0] / width);
	}
	else if((angle < -45 && angle >= -90) || (angle > 45.0 && angle <= 90.0))
	{
		length = blob->diameter[0];
		width = (blob->area[0] / length);
	}
	else
	{
		width = blob->diameter[0];
		length = (blob->area[0] / width);
	}

	*retWidth = width;
	*retLength = length;
}


int CPInsp_Mount:: CheckRect(CRect rt, CPoint* nodalPt, int ptCount, int* retIndex)
{
	int cnt = 0;
	int index = -1;

	for(int i = 0; i < ptCount; i++)
	{
		if(!PtInRect(rt, nodalPt[i]))
		{
			cnt++;
			index = i;
		}
	}


	*retIndex = index;

	return cnt;
}

void CPInsp_Mount::RemakeNodalPt(CPoint* srcNodalPt, CRect srcRect, int ptCount, int errorCount, int errorIndex, CPoint* dstNodalPt)
{
	if(ptCount <= 0)
		return;

	CPoint* ptTemp = NULL;
	//ptTemp = new CPoint[ptCount];
	ptTemp = g_pMManager->pem_new<CPoint>(true, ptCount, (PCHAR)__FUNCTION__, __LINE__);
	memcpy_s(ptTemp, sizeof(CPoint) * ptCount, srcNodalPt, sizeof(CPoint) * ptCount);

	if(errorCount == 1)
	{
		if(ptCount >= 1 && errorIndex == 0)
		{
			ptTemp[0].x = srcNodalPt[1].x;
			ptTemp[0].y = srcNodalPt[3].y;
		}
		else if(ptCount >= 2 && errorIndex == 1)
		{
			ptTemp[1].x = srcNodalPt[0].x;
			ptTemp[1].y = srcNodalPt[2].y;
		}
		else if(ptCount >= 3 && errorIndex == 2)
		{
			ptTemp[2].x = srcNodalPt[3].x;
			ptTemp[2].y = srcNodalPt[1].y;
		}
		else if(ptCount >= 4 && errorIndex == 3)
		{
			ptTemp[3].x = srcNodalPt[2].x;
			ptTemp[3].y = srcNodalPt[0].y;
		}			
	}
	else if(errorCount > 1)
	{		
		ptTemp[0].x = srcRect.left;
		ptTemp[0].y = srcRect.top;

		ptTemp[1].x = srcRect.left;
		ptTemp[1].y = srcRect.bottom;

		ptTemp[2].x = srcRect.right;
		ptTemp[2].y = srcRect.bottom;

		ptTemp[3].x = srcRect.right;
		ptTemp[3].y = srcRect.top;		
	}

	if(dstNodalPt != NULL)
		memcpy_s(dstNodalPt, sizeof(CPoint) * ptCount, ptTemp, sizeof(CPoint) * ptCount);


	//delete ptTemp;
	g_pMManager->pem_delete(ptTemp, true);
}


void CPInsp_Mount::InitBlobResultStruct(MountBlobRst* rst)
{
	if(!rst)
		return;

	//MountBlobRst* temp = new MountBlobRst;
	MountBlobRst* temp = g_pMManager->pem_new<MountBlobRst>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, sizeof(MountBlobRst));

	*rst = *temp;

	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

void CPInsp_Mount::InitMntResultStruct(RstInspMount* rst)
{
	//RstInspMount* temp = new RstInspMount;
	RstInspMount* temp = g_pMManager->pem_new<RstInspMount>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, sizeof(RstInspMount));

	temp->ok = -1;
	temp->isInsp = FALSE;
	temp->list.existOK = TRUE;
	temp->list.heightOk = TRUE;
	temp->list.widthOk = TRUE;
	temp->list.lengthOk = TRUE;
	temp->list.offsetXOk = TRUE;
	temp->list.offsetYOk = TRUE;
	temp->list.rotationOk = TRUE;
	temp->list.gradientOk_v = TRUE;
	temp->list.gradientOk_h = TRUE;
	temp->list.mountPatternOK = TRUE;


	*rst = *temp;
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}


void CPInsp_Mount::InitOffsetBuf()
{
	DeleteOffsetBuf();

	//m_bodyOffset = new BodyOffset;
	m_bodyOffset = g_pMManager->pem_new<BodyOffset>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(m_bodyOffset, 0, sizeof(BodyOffset));
}

void CPInsp_Mount::DeleteOffsetBuf()
{
	if(m_bodyOffset != NULL)
	{
		//delete m_bodyOffset;
		g_pMManager->pem_delete(m_bodyOffset, false);
		m_bodyOffset = NULL;
	}
}

int CPInsp_Mount::GetBodyOffset(BodyOffset* rst)
{
	if(m_bodyOffset == NULL)
		return eMNT_FAIL;


	memcpy_s(rst, sizeof(BodyOffset), m_bodyOffset, sizeof(BodyOffset));

	return eMNT_SUCCESS;
}



#pragma endregion _MOUNT FUNCTION_




#pragma region pattern 

int CPInsp_Mount::MountPatternTeach(double cogX, double cogY)
{
	int ret = e_OK;

	int cx = (int)cogX;
	int cy = (int)cogY;
	double marginX = _mm2pixel_x(m_teachParam_Mount->bdryMargin);
	double marginY = _mm2pixel_x(m_teachParam_Mount->bdryMargin);
	int roiSizeX = (int)(m_teachCoordinate.width - (marginX * 2.0));
	int roiSizeY = (int)(m_teachCoordinate.length - (marginY * 2.0));

	int searchCx = m_teachCoordinate.bdrCx;
	int searchCy = m_teachCoordinate.bdrCy;
	int searchSizeX = m_teachCoordinate.bdrWidth;
	int searchSizeY = m_teachCoordinate.bdrLength;
	
	double wndAngle = m_teachCoordinate.angle;

	int bdCx = m_teachCoordinate.anyAngleCx;
	int bdCy = m_teachCoordinate.anyAngleCy;
	int bdSizeX = m_teachCoordinate.anyAngleWidth;
	int bdSizeY = m_teachCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;
	
	CString modelPath = m_teachParam_Mount->modelPath;

	if(cx < 0 || cy < 0 || roiSizeX < 0 || roiSizeY < 0 || searchCx < 0 || searchCy < 0 || searchSizeX < 0 || searchSizeY < 0)
		return eMNT_FAIL;


	if((roiSizeX % 2) != 0)
		roiSizeX += 1;

	if((roiSizeY % 2) != 0)
		roiSizeY += 1;

	if((searchSizeX % 2) != 0)
		searchSizeX += 1;

	if((searchSizeY % 2) != 0)
		searchSizeY += 1;

	int roiArea = roiSizeX * roiSizeY;
	int searchArea = searchSizeX * searchSizeY;

	UCHAR* clipSearchImg = NULL;
	UCHAR* clipModelImg = NULL;
	//////////////////////////////////////////////////////////////////////////
	BOOL isAnyAngle = IsAnyAngle(wndAngle);
	if(isAnyAngle)
	{
		double rotAngle = CalcRotateAngle(wndAngle);

		int w = 0;
		int l = 0;
		UCHAR* bdryImgTemp = NULL;
		m_procMil->ProcAnyAngle_Img(m_fovImage_teach, &bdryImgTemp, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
		m_procMil->SaveWorkImg(bdryImgTemp, w, l, _T("MPATTERN_boundaryImg.bmp"));

		wndAngle = 0;
		searchCx -= (int)(bdCx - (w / 2.0));
		searchCy -= (int)(bdCy - (l / 2.0));

		//clipSearchImg = new UCHAR[searchArea];
		clipSearchImg = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipImage(bdryImgTemp, w, l, clipSearchImg, searchCx, searchCy, searchSizeX, searchSizeY);
		m_procMil->SaveWorkImg(clipSearchImg, searchSizeX, searchSizeY, _T("MPATTERN_clipSearchImg.bmp"));

		cx -= (int)(bdCx - (w / 2.0));
		cy -= (int)(bdCy - (l / 2.0));

		//clipModelImg = new UCHAR[roiArea];
		clipModelImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipImage(bdryImgTemp, w, l, clipModelImg, cx, cy, roiSizeX, roiSizeY);
		m_procMil->SaveWorkImg(clipModelImg, roiSizeX, roiSizeY, _T("MPATTERN_clipModelImg.bmp"));

		//delete bdryImgTemp;
		g_pMManager->pem_delete(bdryImgTemp, true);
	}
	else
	{
		//clipSearchImg = new UCHAR[searchArea];
		clipSearchImg = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipBuff(m_fovImage_teach, clipSearchImg, searchCx, searchCy, searchSizeX, searchSizeY);

		//clipModelImg = new UCHAR[roiArea];
		clipModelImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipBuff(m_fovImage_teach, clipModelImg, cx, cy, roiSizeX, roiSizeY);
	}


	int w = 0;
	int h = 0;
	double rotateAngle = 0;
	if(wndAngle > 0)
		rotateAngle	= GetRotateAngle(roiSizeX, roiSizeY, wndAngle, &w, &h);


	//////////////////////////////////////////////////////////////////////////
	//aloc model
	BOOL rotMode = (rotateAngle == 0.0)?    FALSE : TRUE;

	int allocRst = m_procPatMil->AllocPatModel(clipModelImg, roiSizeX, roiSizeY, rotateAngle, w, h);
	m_procPatMil->SetAngleMode(5, 5, 0.5, rotMode);
	m_procPatMil->SetSearchStartAngle(0);
	m_procPatMil->PreprocModel(m_fovImage_teach, rotMode);
	//////////////////////////////////////////////////////////////////////////


	int searchCnt = 0;
	double score = 0;
	double angle = 0;
	double posX = 0;
	double posY = 0;
	if(allocRst == eMNT_SUCCESS)
	{
		//searchCnt = m_procPatMil->SearchPattern(m_fovImage_teach, searchCx, searchCy, searchSizeX, searchSizeY);
		searchCnt = m_procPatMil->SearchPattern(clipSearchImg, searchSizeX, searchSizeY);

		if(searchCnt > 0)	
		{	
			m_procPatMil->GetPatResult(&score, &angle, &posX, &posY);

			double cogX_pixel = (searchCx - (searchSizeX / 2.0)) + posX;
			double cogY_pixel = (searchCy - (searchSizeY / 2.0)) + posY;

			double cogX_board = 0.0;
			double cogY_board = 0.0;
			CvtPixelToBoard(m_teachCoordinate.fovCx, m_teachCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);

			//m_procMil->WritePatternModel(m_fovImage_teach, cx, cy, roiSizeX, roiSizeY, rotateAngle, modelPath);
			m_procPatMil->WritePatternModel(clipModelImg, roiSizeX, roiSizeY, rotateAngle, modelPath,_T(".tif"));
				
		}
	}


	/*delete clipModelImg;
	delete clipSearchImg;*/
	g_pMManager->pem_delete(clipModelImg, true);
	g_pMManager->pem_delete(clipSearchImg, true);

	return ret;
}
int CPInsp_Mount::InspBodyBlob( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg,
						InspRoiImgBuf &sInspImageData, BOOL bTeach, RstAlgoBodyBlob * pRstAlgo,
						CRect &rcBlobBody, POINTF ptWndCenter, TotalInspExceptArea stTieArea, UCHAR *pUcImgBlob)
{
	int nReturnRate = e_NG;

	int nLine = __LINE__;

	//When inspecting "excluding hidden areas", only foreign matter/height inspection is performed.
	//When inspecting "excluding hidden areas", the inspection is performed regardless of the Minimum size.
	//When inspecting "excluding hidden areas", the inspection is performed regardless of the Count of blobs.
	//When inspecting "excluding hidden areas", the 'BodyTip' option is ignored.
	//가려진 상태인지 확인하여 Flag 넣어야 함.
	InspPartInfo *pInspPartInfo = nullptr;
	POINTF *pPoHiddenAreaOfPartImgPx = nullptr;
	BOOL bUseHiddenArea = FALSE;
	UCHAR *pUcImgSrcTmp = nullptr;
	float *pfImgSrcTmp = nullptr;

	try
	{
		if(pRstAlgo)
		{
			memset(pRstAlgo, 0, (sizeof(RstAlgoBodyBlob)));
			pRstAlgo->Init();
		}

		double dShiftX = 0;
		double dShiftY = 0;
		rcBlobBody = CRect(0,0,0,0);

		if(sInspAlgo.m_eAlgoType != eAlgoBody_Blob)
			return nReturnRate;
		AlgoBodyBlob *pAlgoBodyBlob = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
		if(!pAlgoBodyBlob)
			return nReturnRate;
		CProcMil_Mount *pProcMilMount = m_procMil;
		if(!pProcMilMount)
			return nReturnRate;

		nLine = __LINE__;

		UCHAR *pUcImgSrc	= sWndAlgoImg.m_ucArr2D;
		float *pfImgSrc		= sWndAlgoImg.m_fArr3D;
		int nImgWidthClip		= sWndAlgoImg.m_nWidth;
		int nImgHeightClip		= sWndAlgoImg.m_nHeight;
		double dWndAngle = sWndAlgoImg.dAngle;
		if((pUcImgSrc == NULL) || (pfImgSrc == NULL) || (nImgWidthClip <= 0) || (nImgHeightClip <= 0))
			return nReturnRate;
#if _DEBUG
		cv::Mat imgpUcImgSrc(nImgHeightClip, nImgWidthClip, CV_8UC1, pUcImgSrc);
		cv::Mat imgpfImgSrc(nImgHeightClip, nImgWidthClip, CV_32FC1, pfImgSrc);
#endif

		//HiddenArea
		pInspPartInfo = g_pInspMng->GetInspPartInfo();
		if (pInspPartInfo != nullptr)
		{
			if (pInspPartInfo->bStickerInsp == TRUE)
			{
				pPoHiddenAreaOfPartImgPx = pInspPartInfo->pPoHiddenArea;
				bUseHiddenArea = TRUE;
			}
		}

		if (bUseHiddenArea)
		{
			//이물, 높이 검사를 제외하고 다른 검사는 하지 않도록 해야함.
			pAlgoBodyBlob->m_bUseBodyTip = FALSE; //가려진 경우 BodyTip이 없을 수 있기 때문에 해당 옵션 비활성화하여 검사 진행.
			pAlgoBodyBlob->m_bAreaIsUse = FALSE;
			pAlgoBodyBlob->m_bShiftIsUse = FALSE;
			pAlgoBodyBlob->m_bShiftXUse = FALSE;
			pAlgoBodyBlob->m_bShiftYUse = FALSE;
			pAlgoBodyBlob->m_bTeachWidthUse = FALSE;
			pAlgoBodyBlob->m_bTeachLengthUse = FALSE;
			pAlgoBodyBlob->m_bUseAngle = FALSE;
			pAlgoBodyBlob->m_bUseShape = FALSE;
			pAlgoBodyBlob->m_bUseDamage = FALSE;
			pAlgoBodyBlob->m_bUseHeight = TRUE;

			//가려진 영역이 있다면 영상 사본을 만들어 검사 진행.
			pUcImgSrcTmp = new UCHAR[nImgWidthClip * nImgHeightClip];
			pfImgSrcTmp = new float[nImgWidthClip * nImgHeightClip];
			memcpy(pUcImgSrcTmp, sWndAlgoImg.m_ucArr2D, nImgWidthClip * nImgHeightClip * sizeof(UCHAR));
			memcpy(pfImgSrcTmp, sWndAlgoImg.m_fArr3D, nImgWidthClip * nImgHeightClip * sizeof(float));
			pUcImgSrc = pUcImgSrcTmp;
			pfImgSrc = pfImgSrcTmp;
		}

		cv::Mat img2D = cv::Mat(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_MAKETYPE(CV_8U, 1), pUcImgSrc);
		cv::Mat img3D = cv::Mat(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_32FC1, pfImgSrc);

		//가려진 영역이 있다면 제외하고 검사 진행.
		if (pPoHiddenAreaOfPartImgPx != nullptr && bUseHiddenArea)
		{
			for (int nStcIdx = 0; nStcIdx < StickerCnt; nStcIdx++)
			{
				g_pInspMng->m_FR.FillPolygon(img2D, pPoHiddenAreaOfPartImgPx + (nStcIdx * StickerPo), StickerPo, cv::Scalar(0, 0, 0));
				g_pInspMng->m_FR.FillPolygon(img3D, pPoHiddenAreaOfPartImgPx + (nStcIdx * StickerPo), StickerPo, cv::Scalar(0, 0, 0));
			}
		}

		double dROICenterX = nImgWidthClip / 2.0;
		double dROICenterY = nImgHeightClip / 2.0;
		if ((dROICenterX < 0) || (dROICenterY < 0))
		{
			if (pUcImgSrcTmp != nullptr)
			{
				g_pMManager->pem_delete(pUcImgSrcTmp, true);
				pUcImgSrcTmp = nullptr;
			}
			if (pfImgSrcTmp != nullptr)
			{
				g_pMManager->pem_delete(pfImgSrcTmp, true);
				pfImgSrcTmp = nullptr;
			}
			return nReturnRate;
		}

		nLine = __LINE__;

		UCHAR * ucArrBlobDst = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobDst, nImgWidthClip * nImgHeightClip);
		double dCX = 0;
		double dCY = 0;
		double dArea = 0;
		double dMinBlobArea = ( pAlgoBodyBlob->m_dTeachWidth / m_resolX * pAlgoBodyBlob->m_dTeachLength / m_resolY) / 10.0;
		//When inspecting "excluding hidden areas", only foreign matter/height inspection is performed.
		if (bUseHiddenArea == TRUE) dMinBlobArea = 0;
		else if (dMinBlobArea < 10)	dMinBlobArea = 10;
		CRect rcBlob(0,0,0,0);

		int nSearchPerCnt = m_nSearchPer_DefaultCnt;
		double dArrSearchPer[BODYBLOB_DEFAULT];
		for (int a = 0; a < BODYBLOB_DEFAULT; a++)
		{
			dArrSearchPer[a] = m_dSearchPer_Default[a];
			if (dArrSearchPer[a] < 10) dArrSearchPer[a] = 10;
			if (dArrSearchPer[a] >= 80) dArrSearchPer[a] = 80;
			if (bUseHiddenArea == TRUE) dArrSearchPer[a] = 10;
		}
		if (pAlgoBodyBlob->m_fSearchDefault >= 10 && bUseHiddenArea == FALSE)
		{
			nSearchPerCnt = 1;
			dArrSearchPer[0] = pAlgoBodyBlob->m_fSearchDefault;
			if (dArrSearchPer[0] >= 80) dArrSearchPer[0] = 80;
		}
		if (nSearchPerCnt < 1) nSearchPerCnt = 1;
		if (nSearchPerCnt > BODYBLOB_DEFAULT) nSearchPerCnt = BODYBLOB_DEFAULT;

		float fSearchPer_BodyTip = m_dBodyBlobSearchPer_BodyTip;
		if(pAlgoBodyBlob->m_fSearchBodyTip >= 10)
			fSearchPer_BodyTip = pAlgoBodyBlob->m_fSearchBodyTip;
		if(fSearchPer_BodyTip < 10) fSearchPer_BodyTip = 10;
		if(fSearchPer_BodyTip >= 80) fSearchPer_BodyTip = 80;
		nLine = __LINE__;

		// Teaching Center 좌표는 현재 각도에 맞춰 돌려줘야한다.
		POINT ptTeachCenter;
		
		// Wnd Center 좌표는 이미 angle 적용 되어 있으므로 돌릴 필요가 없다.
		POINTF ptWndCenterTemp;

		// blob search 시 사용되는 wnd center 좌표
		double dCenTeachX = pAlgoBodyBlob->m_dTechCenterX;
		double dCenTeachY = pAlgoBodyBlob->m_dTechCenterY;
		
		// bodyblob 시 사용되는 wnd center 좌표는 angle을 돌려줘야 함.
		if((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
		{
			m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, dWndAngle,
				0, 0, &dCenTeachX, &dCenTeachY);
		}

		ptTeachCenter.x = RounD(dCenTeachX / m_resolX) + nImgWidthClip / 2;
		ptTeachCenter.y = nImgHeightClip / 2 - RounD(dCenTeachY / m_resolY);
		if(pRstAlgo)
		{
			double dTeachW = pAlgoBodyBlob->m_dTeachWidth / m_resolX;
			double dTeachH = pAlgoBodyBlob->m_dTeachLength / m_resolY;
			if (dWndAngle == 90 || dWndAngle == 270)
			{
				dTeachH = pAlgoBodyBlob->m_dTeachWidth / m_resolX;
				dTeachW = pAlgoBodyBlob->m_dTeachLength / m_resolY;
			}
			nLine = __LINE__;
			pRstAlgo->m_rcRect_T.left = ptTeachCenter.x - (dTeachW / 2.0);
			pRstAlgo->m_rcRect_T.right = pRstAlgo->m_rcRect_T.left + dTeachW;
			
			pRstAlgo->m_rcRect_T.top = ptTeachCenter.y - (dTeachH / 2.0);
			pRstAlgo->m_rcRect_T.bottom = pRstAlgo->m_rcRect_T.top + dTeachH;
			pRstAlgo->m_dWNDA = dWndAngle;
		}
		ptWndCenterTemp.x = RounD(ptWndCenter.x / m_resolX) + nImgWidthClip / 2;
		ptWndCenterTemp.y = nImgHeightClip / 2 - RounD(ptWndCenter.y / m_resolY);

		nLine = __LINE__;
		
		// UI 에서 그린 좌표와 실제 skip 되는 영역이 Teach center 때문에 차이가 나서 여기서 조정함



			// teach center 와 roi/2 가 일치 하지 않는 경우




		// 이곳은 하단까지 다 내려와서 만들필요가 없을 듯 하다.
		// 통합 검사/제외 영역에 넣어준다
		stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
		stTieArea.m_rcArrMaskingROI = sInspAlgo.m_rcArrMaskingROI;
		//for (int i = 0; i < MAX_MASKING_NUM; i++)
		//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
		//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
		//	stTieArea.m_rcArrMaskingROI.push_back(sInspAlgo.m_rcArrMaskingROI[i]);
		//stTieArea.m_rcArrMaskingROI.resize(sInspAlgo.m_nUsedMaskingValue);
		//for (int i = 0; i < sInspAlgo.m_nUsedMaskingValue; i++)
		//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
		stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
		stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
			stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];

		// 파트영역에서 지정 영역 skip 하는 부분
		// UI 상에서는 여기서 나온 img 를 출력함		
		if(bTeach)
		{
			// KIY 2020/04/23 : 폴리곤 통합
			if(pUcImgSrc != NULL)
				CPInsp::FillOutOfInspAreaCombine(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, 0, pUcImgSrc, NULL, stTieArea);

		}

		AlgoBlob algoBlob = CPInsp::SetAlgoBlob(sInspAlgo);
		if(pAlgoBodyBlob->m_b3dPerCheck == TRUE)
		{
			nLine = __LINE__;
			float fMarginX = 0.3f;
			float fMarginY = 0.3f;
			if(pAlgoBodyBlob->m_bShiftXUse)
				fMarginX = pAlgoBodyBlob->m_dShiftX;
			if(pAlgoBodyBlob->m_bShiftYUse)
				fMarginY = pAlgoBodyBlob->m_dShiftY;
			if(sWndAlgoImg.dAngle == 90 || sWndAlgoImg.dAngle == 270)
			{
				if(pAlgoBodyBlob->m_bShiftXUse)
					fMarginY = pAlgoBodyBlob->m_dShiftX;
				if(pAlgoBodyBlob->m_bShiftYUse)
					fMarginX = pAlgoBodyBlob->m_dShiftY;
			}
			float fMarginX_Pix = fMarginX / m_resolX;
			float fMarginY_Pix = fMarginY / m_resolY;
			RECT rcTeachROI;
			double dTeachW = pAlgoBodyBlob->m_dTeachWidth / m_resolX;
			double dTeachH = pAlgoBodyBlob->m_dTeachLength / m_resolY;
			if(bTeach == TRUE)
			{
				if(dTeachW == 0)
					dTeachW = nImgWidthClip / 2;
				if(dTeachH == 0)
					dTeachH = nImgHeightClip / 2;
			}
			rcTeachROI.left = ptTeachCenter.x - (dTeachW / 2.0) - fMarginX_Pix;
			rcTeachROI.right = rcTeachROI.left + dTeachW + fMarginX_Pix;
			rcTeachROI.top = ptTeachCenter.y - (dTeachH / 2.0) - fMarginY_Pix;
			rcTeachROI.bottom = rcTeachROI.top + dTeachH + fMarginY_Pix;
			float fDefault = pAlgoBodyBlob->m_f3dPerHeightAvg * 0.5;
			float f3DAvg = CPInsp::GetImageROIHeightAvg(pfImgSrc, nImgWidthClip, nImgHeightClip, rcTeachROI, fDefault);
			algoBlob.m_bInsp3D = pAlgoBodyBlob->m_b3dPerCheck;
			algoBlob.m_nTypeRange3D = pAlgoBodyBlob->m_n3dPerRange;
			algoBlob.m_dHeightRateMin = f3DAvg * pAlgoBodyBlob->m_f3dPerHeightMin / 100.0f;
			algoBlob.m_dHeightRateMax = f3DAvg * pAlgoBodyBlob->m_f3dPerHeightMax / 100.0f;
			if(pRstAlgo)
				pRstAlgo->m_dRstHeightMean_Per = f3DAvg;
			nLine = __LINE__;
		}
		UCHAR *ucColorImgDst = NULL;
		CPInsp_Color *pColorTeach = g_pInspMng->GetColorTeach();
		if(pColorTeach != NULL && pAlgoBodyBlob->m_sAlgoColorBase.m_bUseColor == TRUE)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucColorImgDst, sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			memset(ucColorImgDst, 0, sizeof(UCHAR) * sInspImageData.nImageSizeX * sInspImageData.nImageSizeY);
			pColorTeach->GetColorBaseBin(&pAlgoBodyBlob->m_sAlgoColorBase, sInspImageData, ucColorImgDst, bTeach);
			m_pProcMilAlgo->SaveWorkImg(ucColorImgDst, sInspImageData.nImageSizeX, sInspImageData.nImageSizeY, _T("OrgColor_Image.bmp"));
		}
#if _DEBUG
		cv::Mat imgBlobDst(nImgHeightClip, nImgWidthClip, CV_8UC1, ucArrBlobDst);
#endif

		int nCntBlob = 0;
		//When inspecting "excluding hidden areas", the inspection is performed regardless of the Count of blobs.
		if (bUseHiddenArea == TRUE)
		{
			double m_resolX = PIAL::PInspAlgo_Lib::m_resolX;
			double m_resolY = PIAL::PInspAlgo_Lib::m_resolY;
			double dAreaWPix = pAlgoBodyBlob->m_dTeachWidth * pAlgoBodyBlob->m_dTeachWidthRateMax * 0.01 / m_resolX;
			double dAreaHPix = pAlgoBodyBlob->m_dTeachLength * pAlgoBodyBlob->m_dTeachLengthRateMax * 0.01 / m_resolY;
			algoBlob.m_nTypeSelectBlob = pAlgoBodyBlob->m_nTypeSelectBlob = eSelectHiddenArea;
			nCntBlob = CPInsp::BlobImageStruct(algoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidthClip, nImgHeightClip, dMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucArrBlobDst, stTieArea, algoBlob.m_bFillHole, FALSE, ptTeachCenter.x, ptTeachCenter.y, 0, -1, dAreaWPix, dAreaHPix, 0);
		}
		else
		{
			algoBlob.m_nTypeSelectBlob = pAlgoBodyBlob->m_nTypeSelectBlob = eSelectPosition;		// eSelectCenter -> eSelectPosition  로 변경
			nCntBlob = CPInsp::BlobImageStruct(algoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidthClip, nImgHeightClip, dMinBlobArea, &dArea, &dCX, &dCY, &rcBlob, ucArrBlobDst, stTieArea, algoBlob.m_bFillHole, FALSE, ptTeachCenter.x, ptTeachCenter.y);
		}

		if (nCntBlob == 0 && bTeach)
		{
			algoBlob.m_nTypeSelectBlob = eSelectBigger;
			nCntBlob = CPInsp::BlobImageStruct(algoBlob, pUcImgSrc, pfImgSrc, ucColorImgDst, nImgWidthClip, nImgHeightClip, 4, &dArea, &dCX, &dCY, &rcBlob, ucArrBlobDst, stTieArea, algoBlob.m_bFillHole, FALSE, ptTeachCenter.x, ptTeachCenter.y);
		}
		BOOL bInspArea = ((m_nBodyBlobOPT & m_eBodyBlobOPT_INSP_AREA) == m_eBodyBlobOPT_INSP_AREA);
		if (bInspArea && pRstAlgo && bTeach == false && nCntBlob > 0)
		{
			INSP_SHIFT_AREA(pAlgoBodyBlob, dWndAngle, bTeach, pRstAlgo, nImgWidthClip, nImgHeightClip, 0, nCntBlob, ucArrBlobDst, dArrSearchPer[0]);
			if (((m_nBodyBlobOPT & m_eBodyBlobOPT_UNIT_FIND) == m_eBodyBlobOPT_UNIT_FIND))
			{
				int nFilter = (algoBlob.m_bFilterIsUse) ? algoBlob.m_nFilterStepNarrow : 0;
				nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucArrBlobDst, ucArrBlobDst, nImgWidthClip, nImgHeightClip, dMinBlobArea, FALSE, algoBlob.m_bFillHole, nFilter, algoBlob.m_nTypeSelectBlob, ptTeachCenter.x, ptTeachCenter.y);
			}
		}
		if (pAlgoBodyBlob->UseData(m_eAlgoBB_Find) && nCntBlob > 0)
		{
			BODY_RMV_DIV(pAlgoBodyBlob, dWndAngle, bTeach, ucArrBlobDst, nImgWidthClip, nImgHeightClip, fSearchPer_BodyTip);
			int nFilter = (algoBlob.m_bFilterIsUse) ? algoBlob.m_nFilterStepNarrow : 0;
			nCntBlob = m_pProcMilAlgo->CalcBlob_Select(ucArrBlobDst, ucArrBlobDst, nImgWidthClip, nImgHeightClip, dMinBlobArea, FALSE, algoBlob.m_bFillHole, nFilter, algoBlob.m_nTypeSelectBlob, ptTeachCenter.x, ptTeachCenter.y);
		}

		// 이름에 언더바 제거 - LWJ TEST_CODE
		pProcMilMount->SaveWorkImg(ucArrBlobDst, nImgWidthClip, nImgHeightClip, _T("_newBlob.bmp"));
		Delete_1DArray(&ucColorImgDst);
		//////////////////////////////////////////////////////////////////////////

		nLine = __LINE__;

		double dAngleBody = 0;

		double cogX_roi_pixel = 0.0;
		double cogY_roi_pixel = 0.0;

		double cogX_roi_pixel2 = 0.0;
		double cogY_roi_pixel2 = 0.0;

		CRect rcBody;
		CRect rcBody2;
		CRect rcBodyChipBW;	// Test

		float hAvr = 0;
		double bodyWidth = 0;
		double bodyHeight = 0;
		double dGradient = 0;

		double bodyWidth2 = 0;
		double bodyHeight2 = 0;

		CPoint nodalPoint[4];
		CPoint nodalPoint_Angle[4];
		double dRstDmagePer = 0;
		nLine = __LINE__;

		InitBlobResultStruct(&m_inspRst);

		nLine = __LINE__;
		double dNotUSE = 99999;
		double dOnlyBodyTipAngle = dNotUSE;
		double dOnlyBodyTipX = dNotUSE;
		double dOnlyBodyTipY = dNotUSE;
		double dOnlyBodyTipArea = dNotUSE;
		CRect rcTipBody;
		bool bOnlyBodyTip = pAlgoBodyBlob->m_bOnlyBodyTip;
		int nInspBodyTip = -1;
		BOOL bUNIT_FIND = FALSE;
		bool bInspOnlyBodyTip = false;
		bool bTipAngle = ((m_nBodyBlobOPT & m_eBodyBlobOPT_TipAngle) == m_eBodyBlobOPT_TipAngle);
		bool bTipShift = ((m_nBodyBlobOPT & m_eBodyBlobOPT_TipShift) == m_eBodyBlobOPT_TipShift);
		bool bTipArea = false;
		bool bTipSize = true;
		cv::Mat imgBodyTipRst(nImgHeightClip, nImgWidthClip, CV_8UC1, cv::Scalar(0));
		//When inspecting "excluding hidden areas", the 'BodyTip' option is ignored.
		if (pAlgoBodyBlob->m_bUseBodyTip && bOnlyBodyTip == false && bUseHiddenArea == FALSE)
		{
			bUNIT_FIND = ((m_nBodyBlobOPT & m_eBodyBlobOPT_UNIT_FIND) == m_eBodyBlobOPT_UNIT_FIND);
			bInspOnlyBodyTip = bTipAngle || bTipShift;
		}
		if (pAlgoBodyBlob->m_bUseBodyTip && (bOnlyBodyTip || bInspOnlyBodyTip) && bUseHiddenArea == FALSE)
		{
			nLine = __LINE__;
			UCHAR * rstImg = bOnlyBodyTip ? ucArrBlobDst : NULL;
			nInspBodyTip = OnlyBodyTip(pAlgoBodyBlob, pUcImgSrc, pfImgSrc, rstImg, nImgWidthClip, nImgHeightClip, dWndAngle, ptTeachCenter, fSearchPer_BodyTip, bTeach, &rcTipBody, &dOnlyBodyTipArea, stTieArea, &dOnlyBodyTipAngle, &dOnlyBodyTipX, &dOnlyBodyTipY, pRstAlgo, imgBodyTipRst);
			if (nInspBodyTip < 0)
				bOnlyBodyTip = false;
			if (bOnlyBodyTip)
			{
				nCntBlob = nInspBodyTip >= 0 ? 1 : 0;
				dArea = dOnlyBodyTipArea;
				if (rcTipBody.Width() > 3 && rcTipBody.Height() > 3)
				{
					rcBody2.left = rcTipBody.left;
					rcBody2.right = rcTipBody.right;
					rcBody2.top = rcTipBody.top;
					rcBody2.bottom = rcTipBody.bottom;
				}
			}
		}

		if(nCntBlob > 0)
		{
			UCHAR * ucArrBlobDst2 = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobDst2, nImgWidthClip * nImgHeightClip);
			IppStatus sts;
			IppiSize iSize = {nImgWidthClip, nImgHeightClip};
			int nOKPercent = 1;

			nLine = __LINE__;
#if _DEBUG
			cv::Mat imgBlobDst2(nImgHeightClip, nImgWidthClip, CV_8UC1, ucArrBlobDst2);
#endif
			if (bUseHiddenArea == TRUE) nOKPercent = 2;
			sts = ippiCopy_8u_C1R(ucArrBlobDst, nImgWidthClip, ucArrBlobDst2, nImgWidthClip, iSize);
			pProcMilMount->SaveWorkImg(ucArrBlobDst2, nImgWidthClip, nImgHeightClip, _T("newBlob2.bmp"));
			dAngleBody = m_proc3d.ExtractBodyCv(ucArrBlobDst2, 0, 0, nImgWidthClip, nImgHeightClip, rcBlob, 
				&cogX_roi_pixel2, &cogY_roi_pixel2, &bodyWidth2, &bodyHeight2, &rcBody2, nodalPoint, nOKPercent);
			if(bodyWidth2 <= 0 || bodyHeight2 <= 0)
			{
				dAngleBody = m_proc3d.ExtractBodyCv(ucArrBlobDst2, 0, 0, nImgWidthClip, nImgHeightClip, rcBlob, 
					&cogX_roi_pixel2, &cogY_roi_pixel2, &bodyWidth2, &bodyHeight2, &rcBody2, nodalPoint);
			}
			nLine = __LINE__;
			double dblobOrgW = rcBody2.right - rcBody2.left + 1;
			double dblobOrgH = rcBody2.bottom - rcBody2.top + 1;
			if(dblobOrgW > nImgWidthClip)
			{
				dblobOrgW = nImgWidthClip;
				rcBody2.right = rcBody2.left + dblobOrgW;
			}
			if(dblobOrgH > nImgHeightClip)
			{
				dblobOrgH = nImgHeightClip;
				rcBody2.bottom = rcBody2.top + dblobOrgH;
			}
			double dblobCX = (double)(rcBody2.left + rcBody2.right) / 2.0;
			double dblobCY = (double)(rcBody2.top + rcBody2.bottom) / 2.0;
			if(rcBody2.IsRectNull() || dblobOrgW <= 1 || dblobOrgH <= 1 || dblobCX < 0 || dblobCY < 0)
			{
				if(ucArrBlobDst2 != NULL)
					Delete_1DArray(&ucArrBlobDst2);
				if(ucArrBlobDst != NULL)
					Delete_1DArray(&ucArrBlobDst);
				
				if(IsUseInsp(pAlgoBodyBlob) == FALSE)
					nReturnRate = e_OK;
				else
					nReturnRate = e_NG;
				if (g_pMPTI && pRstAlgo && g_pMPTI->m_LogLevel == m_eLogLv_Blob)
				{
					CString sLog = _T("");
					sLog.Format(_T("[InspBodyBlob] [1]\t%.5f\t%.5f"), pRstAlgo->m_dRstWidth, pRstAlgo->m_dRstLength);
					g_pMPTI->AddLog_Dev(sLog);
				}
				if (pUcImgSrcTmp != nullptr)
				{
					g_pMManager->pem_delete(pUcImgSrcTmp, true);
					pUcImgSrcTmp = nullptr;
				}
				if (pfImgSrcTmp != nullptr)
				{
					g_pMManager->pem_delete(pfImgSrcTmp, true);
					pfImgSrcTmp = nullptr;
				}
				return nReturnRate;
			}
			//When inspecting "excluding hidden areas", the 'BodyTip' option is ignored.
			else if (pAlgoBodyBlob->m_bUseBodyTip && bOnlyBodyTip == false && bUseHiddenArea == FALSE)
			{
				nLine = __LINE__;
				SearchBodyTip(pAlgoBodyBlob, pUcImgSrc, pfImgSrc, ucArrBlobDst, nImgWidthClip, nImgHeightClip, dAngleBody, &rcBody2, &dArea, nodalPoint, fSearchPer_BodyTip, nInspBodyTip, imgBodyTipRst, &rcTipBody);
				if(nodalPoint[0].x < rcBody2.left)
				{
					nodalPoint[0].x = nodalPoint[1].x = rcBody2.left;
				}
				if(nodalPoint[0].y < rcBody2.top)
				{
					nodalPoint[0].y = nodalPoint[3].y = rcBody2.top;
				}
				if(nodalPoint[1].y > rcBody2.bottom)
				{
					nodalPoint[1].y = nodalPoint[2].y = rcBody2.bottom;
				}
				if(nodalPoint[2].x > rcBody2.right)
				{
					nodalPoint[2].x = nodalPoint[3].x = rcBody2.right;
				}
				for (int nX = 0; nX < nImgWidthClip; nX++)
				{
					if(pAlgoBodyBlob->m_bIsHorizon == TRUE && nX >= rcBody2.left && nX <= rcBody2.right)
						continue;
					for (int nY = 0; nY < nImgHeightClip; nY++)
					{
						if(pAlgoBodyBlob->m_bIsHorizon == FALSE && nY >= rcBody2.top && nY <= rcBody2.bottom)
							continue;
						int nIndex = (nY * nImgWidthClip) + nX;
						ucArrBlobDst[nIndex] = 0;
					}
				}
				dblobOrgW = rcBody2.right - rcBody2.left + 1;
				dblobOrgH = rcBody2.bottom - rcBody2.top + 1;
				if(dblobOrgW > nImgWidthClip)
				{
					dblobOrgW = nImgWidthClip;
					rcBody2.right = rcBody2.left + dblobOrgW;
				}
				if(dblobOrgH > nImgHeightClip)
				{
					dblobOrgH = nImgHeightClip;
					rcBody2.bottom = rcBody2.top + dblobOrgH;
				}
				dblobCX = (double)(rcBody2.left + rcBody2.right) / 2.0;
				dblobCY = (double)(rcBody2.top + rcBody2.bottom) / 2.0;
				pProcMilMount->SaveWorkImg(ucArrBlobDst, nImgWidthClip, nImgHeightClip, _T("newBlob3.bmp"));
			}
			nLine = __LINE__;
			// For Damage Insp. (Blob 4 Point)
			memcpy_s(nodalPoint_Angle, sizeof(nodalPoint), nodalPoint, sizeof(nodalPoint));
			int ndecreasePixel = 3;	// 4점을 3pixel 줄인다.(LT, LB, RB, RT)

			nodalPoint_Angle[0].x += ndecreasePixel;
			nodalPoint_Angle[0].y += ndecreasePixel;
			nodalPoint_Angle[1].x += ndecreasePixel;
			nodalPoint_Angle[1].y -= ndecreasePixel;
			nodalPoint_Angle[2].x -= ndecreasePixel;
			nodalPoint_Angle[2].y -= ndecreasePixel;
			nodalPoint_Angle[3].x -= ndecreasePixel;
			nodalPoint_Angle[3].y += ndecreasePixel;
			////
			
			nLine = __LINE__;

			//UCHAR * ucblob2 = new UCHAR[(int)(dblobOrgW * dblobOrgH)];
			UCHAR * ucblob2 = g_pMManager->pem_new<UCHAR>(true, (int)(dblobOrgW * dblobOrgH), (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetClipImage_LT(ucArrBlobDst, nImgWidthClip, nImgHeightClip, 
				ucblob2, rcBody2.left, rcBody2.top, dblobOrgW, dblobOrgH);
			pProcMilMount->SaveWorkImg(ucArrBlobDst, nImgWidthClip, nImgHeightClip, _T("BlobOrg1.bmp"));
			pProcMilMount->SaveWorkImg(ucblob2, dblobOrgW, dblobOrgH, _T("Blobclip1.bmp"));

#if _DEBUG
			cv::Mat imgblob2(dblobOrgH, dblobOrgW, CV_8UC1, ucblob2);
#endif
			nLine = __LINE__;

			// 		dAngleBody = m_proc3d.ExtractBodyCv(ucblob2, dblobCX, dblobCY, dblobOrgW, dblobOrgH, rcBlob, 
			// 			&cogX_roi_pixel, &cogY_roi_pixel, &bodyWidth, &bodyHeight, &rcBody, nodalPoint, 80);
			// 		rcBodyChipBW = rcBody;


			double dStdWidth = (pAlgoBodyBlob->m_dTeachWidth / m_resolX);
			double dStdHeight = (pAlgoBodyBlob->m_dTeachLength / m_resolY);
			if(bTeach == false && (dWndAngle == 90 || dWndAngle == 270))
			{
				double temp = dStdWidth;
				dStdWidth = dStdHeight;
				dStdHeight = temp;
			}

			nLine = __LINE__;

			bool bUNIT_W = pAlgoBodyBlob->m_bIsHorizon == FALSE;
			int nTeachX = ptTeachCenter.x - rcBody2.left;
			int nTeachY = ptTeachCenter.y - rcBody2.top;
			if (pAlgoBodyBlob->m_bNotUseBoundaryAngle == FALSE)
			{
				float angle = 0.0f;
				UCHAR * ucblobRot = g_pMManager->pem_new<UCHAR>(true, (int)(dblobOrgW * dblobOrgH), (PCHAR)__FUNCTION__, __LINE__);
				m_pProcMilAlgo->GetClipImage_LT(ucArrBlobDst, nImgWidthClip, nImgHeightClip,
					ucblobRot, rcBody2.left, rcBody2.top, dblobOrgW, dblobOrgH);
				std::vector<std::vector<cv::Point>> contours;
				std::vector<cv::Vec4i> hierarchy;
				cv::Mat blobRot((int)dblobOrgH, (int)dblobOrgW, CV_8UC1, ucblobRot);
				cv::Mat blobRot2 = blobRot.clone();

				cv::findContours(blobRot2, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

				if (contours.size() > 0)
				{
					int nMaxBlobidx = 0;
					int nMaxAreaSize = 0;
					for (int i = 0; i < contours.size(); i++)
					{
						int area = contours[i].size();
						if (area > nMaxAreaSize)
						{
							nMaxAreaSize = area;
							nMaxBlobidx = i;
						}
					}
					cv::RotatedRect rotRect = cv::minAreaRect(contours[nMaxBlobidx]);
					angle = rotRect.angle;
					if (rotRect.angle <= -45)
						angle = (90 + rotRect.angle);

					if (angle != 0)
					{
						cv::Mat M = cv::getRotationMatrix2D(rotRect.center, angle, 1);
						cv::warpAffine(blobRot, blobRot, M, cv::Size((int)dblobOrgW, (int)dblobOrgH));
					}
				}

				dAngleBody = m_proc3d.ExtractBodyCv(ucblobRot, rcBody2.left, rcBody2.top, dblobOrgW, dblobOrgH, rcBlob,
					NULL, NULL, &bodyWidth, &bodyHeight, &rcBody, nodalPoint, dArrSearchPer[0], false, bUNIT_FIND, bUNIT_W, nTeachX, nTeachY, &dArea) - angle;

				if (ucblobRot)
				{
					g_pMManager->pem_delete(ucblobRot, true);
					ucblobRot = NULL;
				}
			}
			else
			{
				dAngleBody = m_proc3d.ExtractBodyCv(ucblob2, rcBody2.left, rcBody2.top, dblobOrgW, dblobOrgH, rcBlob,
					NULL, NULL, &bodyWidth, &bodyHeight, &rcBody, nodalPoint, dArrSearchPer[0], false, bUNIT_FIND, bUNIT_W, nTeachX, nTeachY, &dArea);
			}

			nLine = __LINE__;

			if(rcBody.IsRectNull() || (rcBody.right - rcBody.left) <= 0 || (rcBody.bottom - rcBody.top) <= 0)
			{
				if(ucArrBlobDst2 != NULL)
					Delete_1DArray(&ucArrBlobDst2);
				if(ucArrBlobDst != NULL)
					Delete_1DArray(&ucArrBlobDst);
				if(ucblob2)
				{
					//delete[] ucblob2;
					g_pMManager->pem_delete(ucblob2, true);
					ucblob2 = NULL;
				}

				if(IsUseInsp(pAlgoBodyBlob) == FALSE)
					nReturnRate = e_OK;
				else
					nReturnRate = e_NG;
				if (g_pMPTI && pRstAlgo && g_pMPTI->m_LogLevel == m_eLogLv_Blob)
				{
					CString sLog = _T("");
					sLog.Format(_T("[InspBodyBlob] [2]\t%.5f\t%.5f"), pRstAlgo->m_dRstWidth, pRstAlgo->m_dRstLength);
					g_pMPTI->AddLog_Dev(sLog);
				}
				if (pUcImgSrcTmp != nullptr)
				{
					g_pMManager->pem_delete(pUcImgSrcTmp, true);
					pUcImgSrcTmp = nullptr;
				}
				if (pfImgSrcTmp != nullptr)
				{
					g_pMManager->pem_delete(pfImgSrcTmp, true);
					pfImgSrcTmp = nullptr;
				}
				return nReturnRate;
			}

			nLine = __LINE__;

			// Test
			if (m_proc3d.CheckBody(ucblob2, dblobOrgW, dblobOrgH, rcBody, bodyWidth, bodyHeight, dStdWidth, dStdHeight, dArrSearchPer[0]))
			{
				if (bUseHiddenArea == TRUE) dArrSearchPer[0] -= 10;
				m_proc3d.ExtractBodyCv(ucblob2, rcBody2.left, rcBody2.top, dblobOrgW, dblobOrgH, rcBlob,
					&cogX_roi_pixel, &cogY_roi_pixel, &dStdWidth, &dStdHeight, &rcBodyChipBW, NULL, dArrSearchPer[0] + 10, true, bUNIT_FIND, bUNIT_W, nTeachX, nTeachY, &dArea);
				if (nSearchPerCnt > 1)
				{
					double dSumW = dStdWidth;
					double dSumH = dStdHeight;
					int nRear = 1;
					for (int a = 1; a < nSearchPerCnt; a++)
					{
						bool bConti = false;
						for (int b = 0; b < a; b++)
						{
							if (dArrSearchPer[a] == dArrSearchPer[b])
							{
								bConti = true;
								break;
							}
						}
						if (bConti) continue;

						double dTempW = 0;
						double dTempH = 0;
						m_proc3d.ExtractBodyCv(ucblob2, rcBody2.left, rcBody2.top, dblobOrgW, dblobOrgH, rcBlob,
							NULL, NULL, &dTempW, &dTempH, NULL, NULL, dArrSearchPer[a] + 10);

						if (dTempW > 0 && dTempH > 0)
						{
							dSumW += dTempW;
							dSumH += dTempH;
							nRear++;
						}
					}
					dStdWidth = dSumW / (double)nRear;
					dStdHeight = dSumH / (double)nRear;
				}
			}
			else
			{
				dStdWidth = bodyWidth;
				dStdHeight = bodyHeight;
				rcBodyChipBW.left = rcBody.left;
				rcBodyChipBW.right = rcBody.right;
				rcBodyChipBW.top = rcBody.top;
				rcBodyChipBW.bottom = rcBody.bottom;
				cogX_roi_pixel = rcBody2.left + (rcBodyChipBW.left + (bodyWidth / 2.0));
				cogY_roi_pixel = rcBody2.top + (rcBodyChipBW.top + (bodyHeight / 2.0));
			}

			pProcMilMount->SaveWorkImg(ucblob2, dblobOrgW, dblobOrgH, _T("Blobclip1.bmp"));
			bool bCenter = true; // ((m_nBodyBlobOPT & m_eBodyBlobOPT_Center) == m_eBodyBlobOPT_Center)
			if (bCenter)
			{
				for (int y = 0; y < (int)dblobOrgH; y++)
				{
					for(int x = 0; x < (int)dblobOrgW; x++)
					{
						if (y >= rcBodyChipBW.top && y <= rcBodyChipBW.bottom &&
							x >= rcBodyChipBW.left && x <= rcBodyChipBW.right)
							continue;
						ucblob2[(y * (int)dblobOrgW) + x] = 0;
					}
				}
				pProcMilMount->SaveWorkImg(ucblob2, dblobOrgW, dblobOrgH, _T("Blobclip2.bmp"));
				if (m_pProcMilAlgo->CalcBlob_Select(ucblob2, NULL, dblobOrgW, dblobOrgH, 4, FALSE, FALSE, 0, eSelectBigger) == 1)
				{
					m_pProcMilAlgo->GetBlobResult(NULL, &cogX_roi_pixel, &cogY_roi_pixel, NULL);
					cogX_roi_pixel += rcBody2.left;
					cogY_roi_pixel += rcBody2.top;
				}
			}
			bodyWidth = dStdWidth;
			bodyHeight = dStdHeight;
			nLine = __LINE__;

			if(rcBodyChipBW.IsRectNull() || (rcBodyChipBW.right - rcBodyChipBW.left) <= 0 || (rcBodyChipBW.bottom - rcBodyChipBW.top) <= 0)
			{
				if(ucArrBlobDst2 != NULL)
					Delete_1DArray(&ucArrBlobDst2);
				if(ucArrBlobDst != NULL)
					Delete_1DArray(&ucArrBlobDst);
				if(ucblob2)
				{
					//delete[] ucblob2;
					g_pMManager->pem_delete(ucblob2, true);
					ucblob2 = NULL;
				}

				if(IsUseInsp(pAlgoBodyBlob) == FALSE)
					nReturnRate = e_OK;
				else
					nReturnRate = e_NG;
				if (g_pMPTI && pRstAlgo && g_pMPTI->m_LogLevel == m_eLogLv_Blob)
				{
					CString sLog = _T("");
					sLog.Format(_T("[InspBodyBlob] [3]\t%.5f\t%.5f"), pRstAlgo->m_dRstWidth, pRstAlgo->m_dRstLength);
					g_pMPTI->AddLog_Dev(sLog);
				}
				if (pUcImgSrcTmp != nullptr)
				{
					g_pMManager->pem_delete(pUcImgSrcTmp, true);
					pUcImgSrcTmp = nullptr;
				}
				if (pfImgSrcTmp != nullptr)
				{
					g_pMManager->pem_delete(pfImgSrcTmp, true);
					pfImgSrcTmp = nullptr;
				}
				return nReturnRate;
			}

			nLine = __LINE__;

			pProcMilMount->DrawBody(nImgWidthClip, nImgHeightClip, nodalPoint);

			nLine = __LINE__;

			if(ucblob2)
			{
				//delete[] ucblob2;
				g_pMManager->pem_delete(ucblob2, true);
				ucblob2 = NULL;
			}

			nLine = __LINE__;
			// 평균 높이 산출.
			double dHeightMeanBody = 0.0;
			float rate = pAlgoBodyBlob->m_nTeachHeightAreaRate * 0.01;
			if (rate == 0) rate = 1;

			float fWidth = (rcBody.Width() * sqrt(rate));
			float fHeight = (rcBody.Height() * sqrt(rate));
			int nLeft = floor(rcBody.left + (rcBody.Width() - fWidth) / 2.0 + 0.5);
			int nTop = floor(rcBody.top + (rcBody.Height() - fHeight) / 2.0 + 0.5);
			int nRight = floor(nLeft + fWidth + 0.5);
			int nBottom = floor(nTop + fHeight + 0.5);

			if (nLeft < 0) nLeft = 0;
			if (nTop < 0) nTop = 0;
			if (nRight < 0) nRight = 0;
			if (nBottom < 0) nBottom = 0;

			CRect rcBody3;
			rcBody3.SetRect(nLeft, nTop, nRight, nBottom);

			int nWidthBolb = rcBody3.Width();
			int nHeightBlob = rcBody3.Height();
			/*int nWidthBolb = rcBody.Width();
			int nHeightBlob = rcBody.Height();*/
			float *pfImgDst = NULL;

			nLine = __LINE__;

			// 2023/02/24 Body의 높이값 계산 변경 : body Rect 내 평균 높이 -> Rect 내 이진화된 영역의 평균 높이
			dHeightMeanBody = CPInsp::GetHeightAvgMask(pfImgSrc, ucArrBlobDst, nImgWidthClip, nImgHeightClip, rcBody2.left + nLeft, rcBody2.top + nTop, nWidthBolb, nHeightBlob);

			nLine = __LINE__;

			bool bPatInsp(false);
			double dPatAngle(0);

			if(false/*pAlgoBodyBlob->m_bUsePattern*/)
			{
				nLine = __LINE__;
				double wndAngle = dWndAngle;
				if(IsAnyAngle(wndAngle))
					wndAngle = 0;		// 일반각 일 경우 0도로 돌려진 영상에서 찾기 때문에 0도로 처리

				nLine = __LINE__;
				cv::Mat OrgImg1(nImgHeightClip, nImgWidthClip, CV_8UC1, ucArrBlobDst), OrgImg;
				cv::blur(OrgImg1, OrgImg, cv::Size(5,5));

				nLine = __LINE__;
				if(bTeach)
				{
					nLine = __LINE__;
					pProcMilMount->TeachPattern(OrgImg, nImgWidthClip, nImgHeightClip, wndAngle,
												bodyWidth, bodyHeight, cogX_roi_pixel, cogY_roi_pixel, 
												pAlgoBodyBlob->m_sModelPath);
					nLine = __LINE__;
				}
				else if(fabs(dAngleBody) < 20.0)
				{
					nLine = __LINE__;
					bPatInsp = pProcMilMount->InspPattern(OrgImg, nImgWidthClip, nImgHeightClip, wndAngle,
														cogX_roi_pixel, cogY_roi_pixel, 
														pAlgoBodyBlob->m_sModelPath, dPatAngle) == 0;
					nLine = __LINE__;
				}
			}

			nLine = __LINE__;

			//////////////////////////////////////////////////////////////////////////
			// 내부 데미지 검사	// LYS 2016/03/28
			bool bSaveWorkImg = false;

#ifdef _DEBUG
			bSaveWorkImg = true;
#endif
			nLine = __LINE__;

			if (pRstAlgo)
			{
				pRstAlgo->m_bOKDamage = TRUE;
				pRstAlgo->m_rcBodyDamageRect.left = 0;
				pRstAlgo->m_rcBodyDamageRect.right = 0;
				pRstAlgo->m_rcBodyDamageRect.top = 0;
				pRstAlgo->m_rcBodyDamageRect.bottom = 0;
				pRstAlgo->m_dRstDamageRate = 0;
				pRstAlgo->m_dRstDamageConditionRate = 0;
				pRstAlgo->m_dRstDamageRate_Per = 0;
			}

			nLine = __LINE__;

			if (pAlgoBodyBlob->m_bUseDamage)
			{
				nLine = __LINE__;

				cv::Mat imgDamage(nImgHeightClip, nImgWidthClip, CV_8UC1, cv::Scalar(0));
				cv::Mat imgDamageBin(nImgHeightClip, nImgWidthClip, CV_8UC1, cv::Scalar(0));
				nLine = __LINE__;

				if (pAlgoBodyBlob->m_bUseDamageArea == FALSE)
					pProcMilMount->DrawBodyLine(ucArrBlobDst, imgDamageBin.data, nImgWidthClip, nImgHeightClip, nodalPoint_Angle);
				else
					memcpy(imgDamageBin.data, ucArrBlobDst, sizeof(UCHAR) * nImgWidthClip * nImgHeightClip);

				pProcMilMount->FillHole(imgDamageBin.data, imgDamageBin.data, nImgWidthClip, nImgHeightClip);
				nLine = __LINE__;

				//가려진 영역이 있으면 제외하고 검사
				if (pPoHiddenAreaOfPartImgPx != nullptr && bUseHiddenArea)
				{
					for (int nStcIdx = 0; nStcIdx < StickerCnt; nStcIdx++)
					{
						g_pInspMng->m_FR.FillPolygon(imgDamageBin, pPoHiddenAreaOfPartImgPx + (nStcIdx * StickerPo), StickerPo, cv::Scalar(0, 0, 0));
					}
				}
				nLine = __LINE__;

				int nStart2DX = nImgWidthClip * nImgHeightClip;
				int nStart2DY = nImgWidthClip * nImgHeightClip;
				int nEnd2DX = 0;
				int nEnd2DY = 0;
				double dTargetH = pAlgoBodyBlob->m_dTeachHeight - pAlgoBodyBlob->m_dDamageHeight;
				int64 nTargetGRange = M_LESS_OR_EQUAL;
				int nTargetGMin = 40;
				int nTargetGMax = 255;
				if (pAlgoBodyBlob->m_bInsp2D)
				{
					nTargetGRange = m_pProcMilAlgo->GetTypeRange(pAlgoBodyBlob->m_nTypeRange2D, TRUE);
					nTargetGMin = (nTargetGRange == M_LESS_OR_EQUAL || nTargetGRange == M_GREATER_OR_EQUAL) ? pAlgoBodyBlob->m_nMaxBinary : pAlgoBodyBlob->m_nMinBinary;
					nTargetGMax = (nTargetGRange == M_LESS_OR_EQUAL || nTargetGRange == M_GREATER_OR_EQUAL) ? pAlgoBodyBlob->m_nMinBinary : pAlgoBodyBlob->m_nMaxBinary;
				}

				double dAreaInspCnt = 0;
				bool bDamageBlob = false;
				for (int y = 0; y < nImgHeightClip; y++)
				{
					for (int x = 0; x < nImgWidthClip; x++)
					{
						int nIndex = (y * nImgWidthClip) + x;
						if (imgDamageBin.data[nIndex] != 255)
							continue;

						dAreaInspCnt += 1.0;
						bool bIn = false;
						if (pfImgSrc[nIndex] < dTargetH)
						{
							if (pAlgoBodyBlob->m_bUseDamage2D)
							{
								UCHAR ucG = pUcImgSrc[nIndex];
								if ((nTargetGRange == M_IN_RANGE && (nTargetGMin <= ucG && ucG <= nTargetGMax)) ||
									(nTargetGRange == M_OUT_RANGE && (nTargetGMin >= ucG || ucG >= nTargetGMax)) ||
									(nTargetGRange == M_LESS_OR_EQUAL && nTargetGMin >= ucG) ||
									(nTargetGRange == M_GREATER_OR_EQUAL && ucG >= nTargetGMax))
								{
									bIn = true;
								}
							}
							else
								bIn = true;
						}

						if (bIn)
						{
							imgDamage.data[nIndex] = 255;
							bDamageBlob = true;
						}
					}
				}
				nLine = __LINE__;

				int nDamageCnt = 0;
				double dDamageAreaPer = pAlgoBodyBlob->m_dDamageArea / 100.0;
				double dDamageAreaT = pAlgoBodyBlob->m_dAreaCurrent * dDamageAreaPer;
				if (dAreaInspCnt > 0 && bDamageBlob)
				{
					cv::Mat imgDamageBlob(nImgHeightClip, nImgWidthClip, CV_8UC1, cv::Scalar(0));
					int nDamageMinArea = (int)(dDamageAreaT / m_resolX / m_resolY);
					nDamageCnt = m_pProcMilAlgo->CalcBlob_Select(imgDamage.data, imgDamageBlob.data, nImgWidthClip, nImgHeightClip, nDamageMinArea, FALSE, TRUE, 0, eSelectMix);
				}
				nLine = __LINE__;

				if (nDamageCnt > 0)
				{
					double *pDamagedArea = g_pMManager->pem_new<double>(true, nDamageCnt, (PCHAR)__FUNCTION__, __LINE__);
					CRect *pDamagedRect = g_pMManager->pem_new<CRect>(true, nDamageCnt, (PCHAR)__FUNCTION__, __LINE__);
					m_pProcMilAlgo->GetBlobResult_ALL(pDamagedArea, NULL, NULL, pDamagedRect);

					double dDamagedArea = 0;
					for (int c = 0; c < nDamageCnt; c++)
					{
						if (c == 0)
						{
							pRstAlgo->m_rcBodyDamageRect.left = pDamagedRect[c].left;
							pRstAlgo->m_rcBodyDamageRect.top = pDamagedRect[c].top;
							pRstAlgo->m_rcBodyDamageRect.right = pDamagedRect[c].right;
							pRstAlgo->m_rcBodyDamageRect.bottom = pDamagedRect[c].bottom;
						}
						else
						{
							if (pRstAlgo->m_rcBodyDamageRect.left > pDamagedRect[c].left)
								pRstAlgo->m_rcBodyDamageRect.left = pDamagedRect[c].left;

							if (pRstAlgo->m_rcBodyDamageRect.top > pDamagedRect[c].top)
								pRstAlgo->m_rcBodyDamageRect.top = pDamagedRect[c].top;

							if (pRstAlgo->m_rcBodyDamageRect.right < pDamagedRect[c].right)
								pRstAlgo->m_rcBodyDamageRect.right = pDamagedRect[c].right;

							if (pRstAlgo->m_rcBodyDamageRect.bottom < pDamagedRect[c].bottom)
								pRstAlgo->m_rcBodyDamageRect.bottom = pDamagedRect[c].bottom;
						}
						dDamagedArea += pDamagedArea[c];
					}

					pRstAlgo->m_dRstDamageRate = m_procMil->ChangePoint(m_nAlgoPoint, dDamagedArea * m_resolX * m_resolY);
					pRstAlgo->m_dRstDamageConditionRate = (dAreaInspCnt * dDamageAreaPer) * m_resolX * m_resolY;
					pRstAlgo->m_dRstDamageRate_Per = (pRstAlgo->m_dRstDamageRate / pAlgoBodyBlob->m_dAreaCurrent) * 100.0;
					pRstAlgo->m_bOKDamage = pRstAlgo->m_dRstDamageRate < dDamageAreaT;

					Delete_1DArray(&pDamagedArea);
					Delete_1DArray(&pDamagedRect);
				}
				nLine = __LINE__;
			}

			//Shape
			if(pAlgoBodyBlob->m_bUseShape == true)
			{
				nLine = __LINE__;
				nWidthBolb		= rcBody.Width();
				nHeightBlob		= rcBody.Height();
				UCHAR *ucShape = g_pMManager->pem_new<UCHAR>(true, nWidthBolb * nHeightBlob, (PCHAR)__FUNCTION__, __LINE__);
				UCHAR *ucShapeDst = g_pMManager->pem_new<UCHAR>(true, nWidthBolb * nHeightBlob, (PCHAR)__FUNCTION__, __LINE__);
				pfImgDst = g_pMManager->pem_new<float>(true, nWidthBolb * nHeightBlob, (PCHAR)__FUNCTION__, __LINE__);
				float fHeigheMax = pAlgoBodyBlob->m_fTeachShapeHeightMax;
				float fArea = pAlgoBodyBlob->m_fTeachShapeArea;

				nLine = __LINE__;
				m_proc3d.GetCropZmap_LT(pfImgSrc, pfImgDst, nImgWidthClip, nImgHeightClip, 
					rcBody2.left + rcBody.left, rcBody2.top + rcBody.top , nWidthBolb, nHeightBlob);
				m_pProcMilAlgo->SaveWorkImg_float(pfImgDst, nWidthBolb, nHeightBlob, _T("BodyBlob3D.bmp"));

				nLine = __LINE__;
				double dCXShape[dfBodyBlobShapeAreaCnt];
				double dCYShape[dfBodyBlobShapeAreaCnt];
				double dAreaShape[dfBodyBlobShapeAreaCnt];
				int nMinBlobAreaShape = 4;
				CRect rcBlobShape[dfBodyBlobShapeAreaCnt];
				RECT rcArrPARTMaskingROIShape[MAX_MASKING_NUM];
				memset(dCXShape, 0, dfBodyBlobShapeAreaCnt * sizeof(double));
				memset(dCYShape, 0, dfBodyBlobShapeAreaCnt * sizeof(double));
				memset(dAreaShape, 0, dfBodyBlobShapeAreaCnt * sizeof(double));
				memset(rcBlobShape, 0, dfBodyBlobShapeAreaCnt * sizeof(CRect));
				memset(rcArrPARTMaskingROIShape, 0, dfBodyBlobShapeAreaCnt * sizeof(RECT));
				AlgoBlackWhite algoBWShape;
				algoBWShape.m_bInvertCheck = FALSE;
				algoBWShape.m_b2dCheck = FALSE;
				algoBWShape.m_nRange = 1;
				algoBWShape.m_nMinValue = 0;
				algoBWShape.m_nMaxValue = 80;

				algoBWShape.m_b3dCheck = TRUE;
				algoBWShape.m_n3dRange = eTypeRangeUpper;
				algoBWShape.m_d3dHeightMin = 0;
				algoBWShape.m_d3dHeightMax = fHeigheMax;
				int nCntBlobShape = CPInsp::BlobImageStruct_BW(algoBWShape, ucShape, pfImgDst, nWidthBolb, nHeightBlob, nMinBlobAreaShape, dAreaShape, dCXShape, dCYShape, rcBlobShape, ucShapeDst,
					eSelectMix, FALSE, 0, stTieArea, FALSE, TRUE, NULL);
				m_procMil->SaveWorkImg(ucShapeDst, nWidthBolb, nHeightBlob, _T("Shape_Blob.bmp"));
				
				if(pRstAlgo)
				{
					pRstAlgo->m_bOKShape = TRUE;
					pRstAlgo->m_nShapeAreaCnt = 0;
					for (int n = 0; n < nCntBlob; n++)
					{
						double dShapeAreamm = (rcBlobShape[n].Width() * m_resolX) * (rcBlobShape[n].Height() * m_resolY);
						double dShapeAreammPer = 0;
						if(pAlgoBodyBlob->m_dTeachWidth * pAlgoBodyBlob->m_dTeachLength > 0)
							dShapeAreammPer = (dShapeAreamm / (pAlgoBodyBlob->m_dTeachWidth * pAlgoBodyBlob->m_dTeachLength)) * 100.0;
						if(dShapeAreamm > fArea && dfBodyBlobShapeAreaCnt > pRstAlgo->m_nShapeAreaCnt)
						{
							pRstAlgo->m_dRstShapeArea[pRstAlgo->m_nShapeAreaCnt] = m_procMil->ChangePoint(m_nAlgoPoint, dShapeAreamm);
							pRstAlgo->m_dRstShapeArea_Per[pRstAlgo->m_nShapeAreaCnt] = dShapeAreammPer;
							pRstAlgo->m_rcBodyShapeRect[pRstAlgo->m_nShapeAreaCnt].left = rcBlobShape[n].left + rcBody2.left;
							pRstAlgo->m_rcBodyShapeRect[pRstAlgo->m_nShapeAreaCnt].right = rcBlobShape[n].right + rcBody2.left;
							pRstAlgo->m_rcBodyShapeRect[pRstAlgo->m_nShapeAreaCnt].top = rcBlobShape[n].top + rcBody2.top;
							pRstAlgo->m_rcBodyShapeRect[pRstAlgo->m_nShapeAreaCnt].bottom = rcBlobShape[n].bottom + rcBody2.top;
							pRstAlgo->m_bOKShape = FALSE;
							pRstAlgo->m_nShapeAreaCnt++;
						}
						if (dfBodyBlobShapeAreaCnt == pRstAlgo->m_nShapeAreaCnt)
							break;
					}
				}

				g_pMManager->pem_delete(ucShape, true);
				g_pMManager->pem_delete(ucShapeDst, true);
				g_pMManager->pem_delete(pfImgDst, true);
			}

			//각도에따른 w와 h 변환..
			double rstW = bodyWidth;
			double rstH = bodyHeight;

			if(dWndAngle == 90 || dWndAngle == 270)
			{
				rstW = bodyHeight;
				rstH = bodyWidth;
			}

			if(pUcImgBlob != NULL)
			{

				nLine = __LINE__;

				rcBody.left += rcBody2.left;
				rcBody.top += rcBody2.top;
				rcBody.right += rcBody2.left;
				rcBody.bottom += rcBody2.top;
				double drcBodyW = rcBody.right - rcBody.left;
				double drcBodyH = rcBody.bottom - rcBody.top;
				for (int y = 0; y < nImgHeightClip; y++)
				{
					for (int x = 0; x < nImgWidthClip; x++)
					{
						if (rcBody.left <= x && x <= rcBody.right &&
							rcBody.top <= y && y <= rcBody.bottom)
							continue;
						int nIndex = (y * nImgWidthClip) + x;
						ucArrBlobDst[nIndex] = 0;
					}
				}
				//UCHAR* pUcImgMask = new UCHAR[nImgWidthClip * nImgHeightClip];
				UCHAR* pUcImgMask = g_pMManager->pem_new<UCHAR>(true, nImgWidthClip * nImgHeightClip, (PCHAR)__FUNCTION__, __LINE__);
				pProcMilMount->MakeBodyMaskImg(ucArrBlobDst, nImgWidthClip, nImgHeightClip, rcBody, pUcImgMask);

				nLine = __LINE__;

				//UCHAR* pUcImgTrue = new UCHAR[nImgWidthClip * nImgHeightClip];
				UCHAR* pUcImgTrue = g_pMManager->pem_new<UCHAR>(true, nImgWidthClip * nImgHeightClip, (PCHAR)__FUNCTION__, __LINE__);
				pProcMilMount->MaskTrueImg(pUcImgSrc, pUcImgMask, pUcImgTrue, nImgWidthClip, nImgHeightClip);
				m_procMil->SaveWorkImg(pUcImgTrue, nImgWidthClip, nImgHeightClip, _T("Body.bmp"));
				CPoint posCenBody = rcBody2.CenterPoint();

				nLine = __LINE__;

				BOOL bReMove = FALSE;
				if(sInspAlgo.m_nUsedMaskingValue > 0 || sInspAlgo.m_nUsedInspPolygon > 0)
					bReMove = TRUE;

				//UCHAR* pUcImgRealSize = new UCHAR[nImgWidthClip * nImgHeightClip];
				UCHAR* pUcImgRealSize = g_pMManager->pem_new<UCHAR>(true, nImgWidthClip * nImgHeightClip, (PCHAR)__FUNCTION__, __LINE__);
				if (rcBody.left == 0 && rcBody.top == 0 && rcBody.right == nImgWidthClip && rcBody.bottom == nImgHeightClip)
					memcpy_s(pUcImgRealSize, sizeof(UCHAR) * nImgWidthClip * nImgHeightClip, pUcImgTrue, sizeof(UCHAR) * nImgWidthClip * nImgHeightClip);
				else
				{
					double dFactor = pProcMilMount->GetResizeFactor(nImgWidthClip, nImgHeightClip, nImgWidthClip, nImgHeightClip);
					pProcMilMount->ResizeImg(pUcImgTrue, pUcImgRealSize, rcBody, nImgWidthClip, nImgHeightClip, nImgWidthClip, nImgHeightClip, dFactor, TRUE, bReMove);
				}
				m_pProcMilAlgo->SaveWorkImg(pUcImgTrue, nImgWidthClip, nImgHeightClip, _T("BodyBlob1.bmp"));
				m_pProcMilAlgo->SaveWorkImg(pUcImgRealSize, nImgWidthClip, nImgHeightClip, _T("BodyBlob2.bmp"));
				nLine = __LINE__;

				double dAngleRotate = GetRotateAngle(nImgWidthClip, nImgHeightClip, dWndAngle, NULL, NULL);

				nLine = __LINE__;

				UCHAR* pImgRotate = NULL;

				{
					memcpy_s(pUcImgBlob, sizeof(UCHAR) * nImgWidthClip * nImgHeightClip, pUcImgTrue, sizeof(UCHAR) * nImgWidthClip * nImgHeightClip);
				}
				m_pProcMilAlgo->SaveWorkImg(pUcImgBlob, nImgWidthClip, nImgHeightClip, _T("BodyBlob.bmp"));

				//delete pUcImgMask;
				g_pMManager->pem_delete(pUcImgMask, true);

				nLine = __LINE__;

				//delete pUcImgTrue;
				g_pMManager->pem_delete(pUcImgTrue, true);

				nLine = __LINE__;

				//delete pUcImgRealSize;
				g_pMManager->pem_delete(pUcImgRealSize, true);

				nLine = __LINE__;
			}
			rcBlobBody.left = rcBodyChipBW.left + rcBody2.left;
			rcBlobBody.top = rcBodyChipBW.top + rcBody2.top;
			rcBlobBody.right = rcBodyChipBW.right + rcBody2.left;
			rcBlobBody.bottom = rcBodyChipBW.bottom + rcBody2.top;
			//When inspecting "excluding hidden areas", the 'BodyTip' option is ignored.
			if (pAlgoBodyBlob->m_bUseBodyTip && nInspBodyTip >= 0 && bUseHiddenArea == FALSE)
			{
				if (bTipAngle)
				{
					if (dOnlyBodyTipAngle != dNotUSE)
					{
						double dA = abs(dOnlyBodyTipAngle + pAlgoBodyBlob->m_dFiduAngle);
						double dB = abs(dAngleBody + pAlgoBodyBlob->m_dFiduAngle);
						if (dB > dA)
							dAngleBody = dOnlyBodyTipAngle;
					}
				}
				if (bTipShift)
				{
					if (dOnlyBodyTipX != dNotUSE && dOnlyBodyTipY != dNotUSE)
					{
						double dAX = abs((cogX_roi_pixel*m_resolX - (double)nImgWidthClip / 2.0 * m_resolX) + (sWndAlgoImg.m_fPartRoundingErrX * m_resolX) - dCenTeachX);
						double dAY = abs(((double)nImgHeightClip / 2.0 * m_resolY - cogY_roi_pixel * m_resolY) - (sWndAlgoImg.m_fPartRoundingErrY * m_resolY) - dCenTeachY);
						double dBX = abs((dOnlyBodyTipX*m_resolX - (double)nImgWidthClip / 2.0 * m_resolX) + (sWndAlgoImg.m_fPartRoundingErrX * m_resolX) - dCenTeachX);
						double dBY = abs(((double)nImgHeightClip / 2.0 * m_resolY - dOnlyBodyTipY * m_resolY) - (sWndAlgoImg.m_fPartRoundingErrY * m_resolY) - dCenTeachY);
						if (dAX > dBX) cogX_roi_pixel = dOnlyBodyTipX;
						if (dAY > dBY) cogY_roi_pixel = dOnlyBodyTipY;
					}
				}
				if (bTipArea)
				{
					double dAreaT = pAlgoBodyBlob->m_dAreaCurrent / m_resolX / m_resolY;
					double dA = abs(dAreaT - dArea);
					double dB = abs(dAreaT - dOnlyBodyTipArea);
					if (dA > dB)
						dArea = dOnlyBodyTipArea;
				}
				if (bTipSize && rcTipBody.Width() > 3 && rcTipBody.Height() > 3)
				{
					double dWT = pAlgoBodyBlob->m_dTeachWidth / m_resolX;
					double dHT = pAlgoBodyBlob->m_dTeachLength / m_resolY;
					double dTipW = (dWndAngle == 90 || dWndAngle == 270) ? rcTipBody.Height() : rcTipBody.Width();
					double dTipH = (dWndAngle == 90 || dWndAngle == 270) ? rcTipBody.Width() : rcTipBody.Height();
					double dWA = abs(dWT - rstW);
					double dHA = abs(dHT - rstH);
					double dWB = abs(dWT - dTipW);
					double dHB = abs(dHT - dTipH);
					if (dWA > dWB)
					{
						rstW = dTipW;
						if (dWndAngle == 90 || dWndAngle == 270)
						{
							bodyHeight = dTipW;
							rcBlobBody.top = rcTipBody.top;
							rcBlobBody.bottom = rcTipBody.bottom;
						}
						else
						{
							bodyWidth = dTipW;
							rcBlobBody.left = rcTipBody.left;
							rcBlobBody.right = rcTipBody.right;
						}
					}
					if (dHA > dHB)
					{
						rstH = dTipH;
						if (dWndAngle == 90 || dWndAngle == 270)
						{
							bodyWidth = dTipH;
							rcBlobBody.left = rcTipBody.left;
							rcBlobBody.right = rcTipBody.right;
						}
						else
						{
							bodyHeight = dTipH;
							rcBlobBody.top = rcTipBody.top;
							rcBlobBody.bottom = rcTipBody.bottom;
						}
					}
				}
			}
			if(bTeach)
			{
				if(pRstAlgo)
				{
					nLine = __LINE__;
					pRstAlgo->m_dRstCenterX = (cogX_roi_pixel*m_resolX - (double)nImgWidthClip / 2.0 * m_resolX) + (sWndAlgoImg.m_fPartRoundingErrX * m_resolX);
					pRstAlgo->m_dRstCenterY = ((double)nImgHeightClip / 2.0 * m_resolY - cogY_roi_pixel * m_resolY) - (sWndAlgoImg.m_fPartRoundingErrY * m_resolY);
					if((dWndAngle == 90) || (dWndAngle == 180) || (dWndAngle == 270))
					{
						m_proc3d.CorrectCoordinate(pRstAlgo->m_dRstCenterX, pRstAlgo->m_dRstCenterY , 0, 0, -dWndAngle,
							0, 0, &pRstAlgo->m_dRstCenterX, &pRstAlgo->m_dRstCenterY);
					}

					nLine = __LINE__;

					pRstAlgo->m_dRstWidth = rstW * m_resolX;
					pRstAlgo->m_dRstLength = rstH * m_resolY;

					double dRstAngle = (bPatInsp) ? dPatAngle : dAngleBody;
					dRstAngle += pAlgoBodyBlob->m_dFiduAngle;
					pRstAlgo->m_dRstAngle = dRstAngle;
					pRstAlgo->m_dRstArea = dArea * m_resolX * m_resolY;

					pAlgoBodyBlob->m_dTeachHeight = pRstAlgo->m_dRstHeightMean = dHeightMeanBody;
				}
			}
			else
			{
				if(pRstAlgo)
				{
					nLine = __LINE__;

					pRstAlgo->m_rcBodyRect = rcBlobBody;

					nLine = __LINE__;
					pRstAlgo->m_dRstCenterX = (cogX_roi_pixel*m_resolX - (double)nImgWidthClip / 2.0 * m_resolX) + (sWndAlgoImg.m_fPartRoundingErrX * m_resolX);
					pRstAlgo->m_dRstCenterY = ((double)nImgHeightClip / 2.0 * m_resolY - cogY_roi_pixel * m_resolY) - (sWndAlgoImg.m_fPartRoundingErrY * m_resolY);

#if _DEBUG
					if (g_pMPTI)
					{
						CString sLog = _T("");
						sLog.Format(_T("[InspBodyBlob] [X]\t%.5f\t%.5f\t%d\t%.5f\t%.5f\t%.5f\t%.5f"), m_resolX, cogX_roi_pixel, nImgWidthClip, sWndAlgoImg.m_fPartRoundingErrX, dCenTeachX, pRstAlgo->m_dRstCenterX, pRstAlgo->m_dRstCenterX - dCenTeachX);
						g_pMPTI->AddLog_Dev(sLog);
						sLog.Format(_T("[InspBodyBlob] [Y]\t%.5f\t%.5f\t%d\t%.5f\t%.5f\t%.5f\t%.5f"), m_resolY, cogY_roi_pixel, nImgHeightClip, sWndAlgoImg.m_fPartRoundingErrY, dCenTeachY, pRstAlgo->m_dRstCenterY, pRstAlgo->m_dRstCenterY - dCenTeachY);
						g_pMPTI->AddLog_Dev(sLog);
					}
#endif
					if (g_pMPTI && pRstAlgo && g_pMPTI->m_LogLevel == m_eLogLv_Blob)
					{
						CString sLog = _T("");
						sLog.Format(_T("[InspBodyBlob] [4]\t%.5f\t%.5f\t%.5f\t%.5f"), pRstAlgo->m_dRstWidth, pRstAlgo->m_dRstLength, rstW, rstH);
						g_pMPTI->AddLog_Dev(sLog);
					}
					pRstAlgo->m_dRstWidth = pAlgoBodyBlob->m_dTeachWidth;
					pRstAlgo->m_dRstLength = pAlgoBodyBlob->m_dTeachLength;
					pRstAlgo->m_dRstArea = m_procMil->ChangePoint(m_nAlgoPoint,dArea * m_resolX * m_resolY);
					pRstAlgo->m_dRstAreaRate = pRstAlgo->m_dRstArea / ((nImgWidthClip * m_resolX) * (nImgHeightClip * m_resolY)) * 100;
					pRstAlgo->m_poDrawCenter.x = cogX_roi_pixel;
					pRstAlgo->m_poDrawCenter.y = cogY_roi_pixel;
					nLine = __LINE__;

					pRstAlgo->m_dRstHeightMean = m_procMil->ChangePoint(m_nAlgoPoint, dHeightMeanBody);


					// SHW 20150302 : Tombstone
					// TomstoneAreaValue : 3 / TomstoneHeightValue : 1.5
					pRstAlgo->m_bTombstone = FALSE;
					double dAreaValue = pAlgoBodyBlob->m_dAreaCurrent * (1 / TomstoneAreaValue);
					double dHeightValue = pAlgoBodyBlob->m_dTeachHeight * TomstoneHeightValue;

					if(pRstAlgo->m_dRstArea < dAreaValue && dHeightValue  < pRstAlgo->m_dRstHeightMean)
						pRstAlgo->m_bTombstone = TRUE;
					pRstAlgo->m_bBillboarding = FALSE;
					// SHW 20150302 : Billboarding
					// BillboardingAreaValue : 2 / BillboardingHeightValue : 1.2
					dAreaValue = pAlgoBodyBlob->m_dAreaCurrent * (1 / BillboardingAreaValue);
					dHeightValue = pAlgoBodyBlob->m_dTeachHeight * BillboardingHeightValue;

					double dWidth = m_procMil->ChangePoint(m_nAlgoPoint, rstW * m_resolX);
					double dLegth = m_procMil->ChangePoint(m_nAlgoPoint, rstH * m_resolY);

					double dDivisionRate = 1.5;
					double dTeachWidthRateForBillboarding = 0.8;
					double dTeachHeightRateForBillboarding = 1.1;

					if (pRstAlgo->m_dRstArea < dAreaValue && dHeightValue < pRstAlgo->m_dRstHeightMean)
					{
						pRstAlgo->m_bBillboarding = TRUE;
						if (dWidth != 0 && dLegth != 0)
						{
							double dDivisionValue = dLegth / dWidth;
							if (dWidth > dLegth || dDivisionValue < dDivisionRate)
							{
								pRstAlgo->m_bBillboarding = FALSE;
								pRstAlgo->m_bTombstone = TRUE;
							}
						}
					}
					else if (dWidth < pAlgoBodyBlob->m_dTeachWidth * dTeachWidthRateForBillboarding && pAlgoBodyBlob->m_dTeachHeight * dTeachHeightRateForBillboarding < pRstAlgo->m_dRstHeightMean)
					{
						pRstAlgo->m_bBillboarding = TRUE;
					}

					nLine = __LINE__;
					if(pAlgoBodyBlob->m_bUseShape == false)
						pRstAlgo->m_bOKShape = TRUE;

					if(pAlgoBodyBlob->m_bAreaIsUse && (pAlgoBodyBlob->m_dAreaCurrent > 0.0005))
					{
						double minRange = pAlgoBodyBlob->m_dAreaCurrent * pAlgoBodyBlob->m_dAreaMin / 100.;
						double maxRange = pAlgoBodyBlob->m_dAreaCurrent * pAlgoBodyBlob->m_dAreaMax / 100.;
						if(pRstAlgo->m_dRstArea > minRange && pRstAlgo->m_dRstArea < maxRange)
							pRstAlgo->m_bOKArea = TRUE;
					}
					else
						pRstAlgo->m_bOKArea = TRUE;
					if(pAlgoBodyBlob->m_bTeachWidthUse)
					{
						pRstAlgo->m_dRstWidth = m_procMil->ChangePoint(m_nAlgoPoint, rstW * m_resolX);
						double minRange = pAlgoBodyBlob->m_dTeachWidth * pAlgoBodyBlob->m_dTeachWidthRateMin / 100.;
						double maxRange = pAlgoBodyBlob->m_dTeachWidth * pAlgoBodyBlob->m_dTeachWidthRateMax / 100.;
						if(pRstAlgo->m_dRstWidth > minRange && pRstAlgo->m_dRstWidth < maxRange)
							pRstAlgo->m_bOKWidth = TRUE;
					}
					else
						pRstAlgo->m_bOKWidth = TRUE;
					if(pAlgoBodyBlob->m_bTeachLengthUse)
					{
						pRstAlgo->m_dRstLength = m_procMil->ChangePoint(m_nAlgoPoint, rstH * m_resolY);
						double minRange = pAlgoBodyBlob->m_dTeachLength * pAlgoBodyBlob->m_dTeachLengthRateMin / 100.;
						double maxRange = pAlgoBodyBlob->m_dTeachLength * pAlgoBodyBlob->m_dTeachLengthRateMax / 100.;
						if(pRstAlgo->m_dRstLength > minRange && pRstAlgo->m_dRstLength < maxRange)
							pRstAlgo->m_bOKLength = TRUE;
					}
					else
						pRstAlgo->m_bOKLength = TRUE;
									
					double dRectValue = (pRstAlgo->m_dRstCenterX - (bodyWidth * m_resolX) / 2.0) / m_resolX + nImgWidthClip / 2.0;
					pRstAlgo->m_rcInspBodyRect.left =	RounD(dRectValue);
					dRectValue = (pRstAlgo->m_dRstCenterX + (bodyWidth * m_resolX) / 2.0) / m_resolX + nImgWidthClip / 2.0;
					pRstAlgo->m_rcInspBodyRect.right =	RounD(dRectValue);
					dRectValue = (-pRstAlgo->m_dRstCenterY - (bodyHeight * m_resolY) / 2.0) / m_resolY + nImgHeightClip / 2.0;
					pRstAlgo->m_rcInspBodyRect.top =	RounD(dRectValue);
					dRectValue = (-pRstAlgo->m_dRstCenterY + (bodyHeight * m_resolY) / 2.0) / m_resolY + nImgHeightClip / 2.0;
					pRstAlgo->m_rcInspBodyRect.bottom =	RounD(dRectValue);

					nLine = __LINE__;

					if(pAlgoBodyBlob->m_bShiftIsUse)
					{
						pRstAlgo->m_dRstShiftX = m_procMil->ChangePoint(m_nAlgoPoint, (pRstAlgo->m_dRstCenterX - dCenTeachX) * -1.0);
						pRstAlgo->m_dRstShiftY = m_procMil->ChangePoint(m_nAlgoPoint, pRstAlgo->m_dRstCenterY - dCenTeachY);

						if(pAlgoBodyBlob->m_bShiftMaxUse)
						{
							int nSign = 1; 
							if(pRstAlgo->m_dRstShiftX < 0)
								nSign = -1;
							pRstAlgo->m_fRstShiftMaxX = m_procMil->ChangePoint(m_nAlgoPoint, fabs(pRstAlgo->m_dRstShiftX) + fabs(pRstAlgo->m_dRstLength / 2 *  sin(dAngleBody * (PI / 180.0))));
							pRstAlgo->m_fRstShiftMaxX *= nSign;

							nSign = 1; 
							if(pRstAlgo->m_dRstShiftY < 0)
								nSign = -1;
							pRstAlgo->m_fRstShiftMaxY = m_procMil->ChangePoint(m_nAlgoPoint, fabs(pRstAlgo->m_dRstShiftY) + fabs(pRstAlgo->m_dRstWidth / 2 *  sin(dAngleBody * (PI / 180.0))));
							pRstAlgo->m_fRstShiftMaxY *= nSign;
						}
						else
						{
							pRstAlgo->m_bOKShiftMaxX = TRUE;
							pRstAlgo->m_bOKShiftMaxY = TRUE;
						}

						double dShiftX_Max = pAlgoBodyBlob->m_dShiftX + pAlgoBodyBlob->m_dShiftX_SC;
						double dShiftY_Max = pAlgoBodyBlob->m_dShiftY + pAlgoBodyBlob->m_dShiftY_SC;
						double dShiftX_Min = (pAlgoBodyBlob->m_dShiftX * -1.0) + pAlgoBodyBlob->m_dShiftX_SC;
						double dShiftY_Min = (pAlgoBodyBlob->m_dShiftY * -1.0) + pAlgoBodyBlob->m_dShiftY_SC;
						if (dWndAngle == 90 || dWndAngle == 270)
						{
							dShiftX_Max = pAlgoBodyBlob->m_dShiftY + pAlgoBodyBlob->m_dShiftY_SC;
							dShiftY_Max = pAlgoBodyBlob->m_dShiftX + pAlgoBodyBlob->m_dShiftX_SC;
							dShiftX_Min = (pAlgoBodyBlob->m_dShiftY * -1.0) + pAlgoBodyBlob->m_dShiftY_SC;
							dShiftY_Min = (pAlgoBodyBlob->m_dShiftX * -1.0) + pAlgoBodyBlob->m_dShiftX_SC;
						}

						pRstAlgo->m_bOKShiftX = pRstAlgo->ShiftChk(dShiftX_Min, dShiftX_Max, true, false);
						pRstAlgo->m_bOKShiftY = pRstAlgo->ShiftChk(dShiftY_Min, dShiftY_Max, false, false);

						pRstAlgo->m_bOKShiftMaxX = pRstAlgo->ShiftChk(dShiftX_Min, dShiftX_Max, true, true);
						pRstAlgo->m_bOKShiftMaxY = pRstAlgo->ShiftChk(dShiftY_Min, dShiftY_Max, false, true);
					}
					else
					{
						pRstAlgo->m_dRstShiftX = 0.;
						pRstAlgo->m_dRstShiftY = 0.;
						pRstAlgo->m_bOKShiftX = TRUE;
						pRstAlgo->m_bOKShiftY = TRUE;
						pRstAlgo->m_fRstShiftMaxX = 0.;
						pRstAlgo->m_fRstShiftMaxY = 0.;
						pRstAlgo->m_bOKShiftMaxX = TRUE;
						pRstAlgo->m_bOKShiftMaxY = TRUE;
					}

					if (pAlgoBodyBlob->m_bUseOffsetDistance)
					{
						pRstAlgo->m_bOKOffsetDistance = FALSE;
						double dResDistance = sqrt(pow((pRstAlgo->m_dRstCenterX - dCenTeachX), 2.0) + pow((pRstAlgo->m_dRstCenterY - dCenTeachY), 2.0));
						pRstAlgo->m_fOffsetDistance = m_procMil->ChangePoint(m_nAlgoPoint, dResDistance);
						if (pAlgoBodyBlob->m_fOffsetDistance > pRstAlgo->m_fOffsetDistance)
							pRstAlgo->m_bOKOffsetDistance = TRUE;
					}
					else
					{
						pRstAlgo->m_fOffsetDistance = 0.0f;
						pRstAlgo->m_bOKOffsetDistance = TRUE;
					}
					if(pAlgoBodyBlob->m_bUseAngle)
					{
						double dRstAngle = (bPatInsp) ? dPatAngle : dAngleBody;
						dRstAngle += pAlgoBodyBlob->m_dFiduAngle;
						pRstAlgo->m_dRstAngle = m_procMil->ChangePoint(m_nAlgoPoint, dRstAngle);
						if((pRstAlgo->m_dRstAngle > -pAlgoBodyBlob->m_dTeachRotate) && (pRstAlgo->m_dRstAngle < pAlgoBodyBlob->m_dTeachRotate))
							pRstAlgo->m_bOKAngle = TRUE;
					}
					else
					{
						pRstAlgo->m_dRstAngle = 0.0;// pAlgoBodyBlob->m_dStandardRotate;
						pRstAlgo->m_bOKAngle = TRUE;
					}

					if (pAlgoBodyBlob->m_bUseHeight)
					{
						if ((pRstAlgo->m_dRstHeightMean > pAlgoBodyBlob->m_dTeachHeightMin) && (pRstAlgo->m_dRstHeightMean < pAlgoBodyBlob->m_dTeachHeightMax))
							pRstAlgo->m_bOKHeight = TRUE;
					}
					else
						pRstAlgo->m_bOKHeight = TRUE;

					nReturnRate = e_OK;
					if (!pRstAlgo->m_bOKHeight || !pRstAlgo->m_bOKAngle || !pRstAlgo->m_bOKWidth || !pRstAlgo->m_bOKLength || !pRstAlgo->m_bOKShiftX || !pRstAlgo->m_bOKShiftY ||
						!pRstAlgo->m_bOKArea || !pRstAlgo->m_bOKDamage || !pRstAlgo->m_bOKShiftMaxX || !pRstAlgo->m_bOKShiftMaxY || !pRstAlgo->m_bOKShape ||
						!pRstAlgo->m_bOKOffsetDistance)
						nReturnRate = e_NG;
					pRstAlgo->m_bShowRect = FALSE;
					if (pRstAlgo->m_bOKHeight && pRstAlgo->m_bOKAngle && pRstAlgo->m_bOKWidth && pRstAlgo->m_bOKLength && pRstAlgo->m_bOKShiftX && pRstAlgo->m_bOKShiftY &&
						pRstAlgo->m_bOKArea && pRstAlgo->m_bOKShiftMaxX && pRstAlgo->m_bOKShiftMaxY && (!pRstAlgo->m_bOKShape || !pRstAlgo->m_bOKDamage) && pRstAlgo->m_bOKOffsetDistance)
						pRstAlgo->m_bShowRect = TRUE;
				}

				nLine = __LINE__;

				m_inspRst.counts = 1;
				m_inspRst.hAvr = hAvr;
				//m_inspRst.cogX = (double)cogX_board;
				//m_inspRst.cogY = (double)cogY_board;
				m_inspRst.left = (double)rcBody.left;
				m_inspRst.right = (double)rcBody.right;
				m_inspRst.top = (double)rcBody.top;
				m_inspRst.bottom = (double)rcBody.bottom;
				m_inspRst.width = rstW;
				m_inspRst.length = rstH;
				m_inspRst.area = (double)(rstW *  rstH);
				m_inspRst.gradientAngle_v = dGradient;
				m_inspRst.gradientAngle_h = dGradient;
				m_inspRst.rotationAngle = dAngleBody;
				//////////////////////////////////////////////////////////////////////////
				//height
				double dHeightStandard =_mm2micron(pAlgoBodyBlob->m_fHeightAvg);
				double range_hMax = pAlgoBodyBlob->m_dHeightRateMax;
				double hMax = dHeightStandard * ((range_hMax - 100) / 100.0);
				if(hMax < 100)// 최소 100미크론
					hMax = 100;

				double deltaH = m_inspRst.hAvr - dHeightStandard;
				m_rstInspMount.bodyHeight = (float)_micron2mm(m_inspRst.hAvr);
				m_rstInspMount.isInsp = TRUE;
				//////////////////////////////////////////////////////////////////////////

			}

			nLine = __LINE__;

			Delete_1DArray(&ucArrBlobDst2);
		}
		else	// Blob 이 없는 경우
		{
			if(IsUseInsp(pAlgoBodyBlob) == TRUE)
			{
				nReturnRate = e_NG;
			}
			else
			{
				nReturnRate = e_OK;
			}
		}

		nLine = __LINE__;

		if(ucArrBlobDst != NULL)
			Delete_1DArray(&ucArrBlobDst);

	}
	catch(...)
	{
		if(g_pMPTI)
		{
			if (pUcImgSrcTmp != nullptr)
			{
				g_pMManager->pem_delete(pUcImgSrcTmp, true);
				pUcImgSrcTmp = nullptr;
			}
			if (pfImgSrcTmp != nullptr)
			{
				g_pMManager->pem_delete(pfImgSrcTmp, true);
				pfImgSrcTmp = nullptr;
			}
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Mount::InspBodyBlob(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}

	if (pUcImgSrcTmp != nullptr)
	{
		g_pMManager->pem_delete(pUcImgSrcTmp, true);
		pUcImgSrcTmp = nullptr;
	}
	if (pfImgSrcTmp != nullptr)
	{
		g_pMManager->pem_delete(pfImgSrcTmp, true);
		pfImgSrcTmp = nullptr;
	}

	return nReturnRate;
}

// 검사 하는 항목이 있는지 여부 리턴
BOOL CPInsp_Mount::IsUseInsp(AlgoBodyBlob *pAlgoBodyBlob)
{
	if(pAlgoBodyBlob->m_bUseAngle || pAlgoBodyBlob->m_bShiftIsUse || pAlgoBodyBlob->m_bAreaIsUse || pAlgoBodyBlob->m_bUseDamage || pAlgoBodyBlob->m_bTeachWidthUse || pAlgoBodyBlob->m_bTeachLengthUse || pAlgoBodyBlob->m_bUseHeight || pAlgoBodyBlob->m_bUseShape)
		return true;
	else
		return false;

}

#pragma endregion pattern

BOOL CPInsp_Mount::SearchBodyTip(AlgoBodyBlob *pAlgo, UCHAR * srcImg2d, float * srcImg3d, UCHAR * blobImg, int sizeX, int sizeY, double dAngle, CRect * rcBody, double * dArea, CPoint* dstNodalPt, float fSearchPer_BodyTip, int nInspBodyTip, cv::Mat imgBodyTipRst, CRect * rcTipBody)
{
	BOOL bRet = TRUE;
	int nBodyL = rcBody->left;
	int nBodyT = rcBody->top;
	int nBodyWidth = rcBody->Width();
	int nBodyHeight = rcBody->Height();
	if (nInspBodyTip == 1)
	{
		nBodyL = rcTipBody->left;
		nBodyT = rcTipBody->top;
		nBodyWidth = rcTipBody->Width();
		nBodyHeight = rcTipBody->Height();
	}

	cv::Mat imgBin(nBodyHeight, nBodyWidth, CV_8UC1, cv::Scalar(0));
	bool bEndLine = ((m_nBodyBlobOPT & m_eBodyBlobOPT_TIP_END) == m_eBodyBlobOPT_TIP_END);
#if _DEBUG
	cv::Mat imgsrcImg2d(sizeY, sizeX, CV_8UC1, srcImg2d);
	cv::Mat imgsrcImg3d(sizeY, sizeX, CV_32FC1, srcImg3d);
	cv::Mat imgblobImg(sizeY, sizeX, CV_8UC1, blobImg);
#endif
	// 1. Binarize
	int ox = 0, oy = 0, oIndex = 0;
	BOOL b2d = FALSE, b3d = FALSE;
	for(int y = 0; y < nBodyHeight; y++)
	{
		oy = y + nBodyT;
		for(int x = 0; x < nBodyWidth; x++)
		{
			ox = x + nBodyL;
			oIndex = oy * sizeX + ox;
			if (nInspBodyTip == 1)
			{
				imgBin.data[y*nBodyWidth + x] = imgBodyTipRst.data[oIndex];
				continue;
			}

			b2d = b3d = true;
			if(pAlgo->m_bTip2dCheck)
			{
				switch (pAlgo->m_nTip2dRange)
				{
				case eTypeRangeIn:
					b2d = ((pAlgo->m_nTip2dMinValue <= srcImg2d[oIndex]) && (srcImg2d[oIndex] <= pAlgo->m_nTip2dMaxValue));					
					break;
				case eTypeRangeOut:
					b2d = ((pAlgo->m_nTip2dMinValue >= srcImg2d[oIndex]) || (srcImg2d[oIndex] >= pAlgo->m_nTip2dMaxValue));
					break;
				case eTypeRangeUpper:
					b2d = (pAlgo->m_nTip2dMaxValue <= srcImg2d[oIndex]);
					break;
				case eTypeRangeLower:
					b2d = (pAlgo->m_nTip2dMinValue >= srcImg2d[oIndex]);
					break;
				}
			}
			if(pAlgo->m_bTip3dCheck)
			{
				switch (pAlgo->m_nTip3dRange)
				{
				case eTypeRangeIn:
					b3d = ((pAlgo->m_dTip3dHeightMin <= srcImg3d[oIndex]) && (srcImg3d[oIndex] <= pAlgo->m_dTip3dHeightMax));					
					break;
				case eTypeRangeOut:
					b3d = ((pAlgo->m_dTip3dHeightMin >= srcImg3d[oIndex]) || (srcImg3d[oIndex] >= pAlgo->m_dTip3dHeightMax));
					break;
				case eTypeRangeUpper:
					b3d = (pAlgo->m_dTip3dHeightMax <= srcImg3d[oIndex]);
					break;
				case eTypeRangeLower:
					b3d = (pAlgo->m_dTip3dHeightMin >= srcImg3d[oIndex]);
					break;
				}
			}

			if(b2d && b3d)
				imgBin.data[y*nBodyWidth + x] = 255;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(imgBin.data, nBodyWidth, nBodyHeight, _T("SearchBodyTip_bw.bmp"));
	cv::Mat imgBlob(nBodyHeight, nBodyWidth, CV_8UC1, cv::Scalar(0));
	int nMinBlob = (pAlgo->m_bIsHorizon) ? nBodyHeight * 3 : nBodyWidth * 3;
	nMinBlob *= 0.8;
	int nCNT = m_pProcMilAlgo->CalcBlob_Select(imgBin.ptr<UCHAR>(), imgBlob.ptr<UCHAR>(), nBodyWidth, nBodyHeight, nMinBlob, false, false, 0, eSelectMix);
	if (nCNT == 0)
		return FALSE;
	//cv::Mat imgBW(nBodyHeight, nBodyWidth, CV_8UC1, tipBWImg);
	//cv::Mat imgBlob(sizeY, sizeX, CV_8UC1, blobImg);
	//cv::Mat img2D(sizeY, sizeX, CV_8UC1, srcImg2d);
	// 2. Tip 찾기
	double dBodyHeigth_Search = (double)nBodyHeight * (fSearchPer_BodyTip / 100.0);
	double dBodyWidth_Search = (double)nBodyWidth * (fSearchPer_BodyTip / 100.0);
	int nx, ny;
	int nWhitePixels;
	if(pAlgo->m_bIsHorizon)		// 가로
	{
		int nGap = nBodyWidth / 5 * 2;
		int nTipPos_left = -1;
		int nTipPos_right = -1;
		for(nx = 0; nx < nGap; nx++)
		{
			nWhitePixels = 0;
			for(ny = 0; ny < nBodyHeight; ny++)
			{
				if(imgBlob.data[ny * nBodyWidth + nx] == 255)
					nWhitePixels++;
			}
			if(nWhitePixels > dBodyHeigth_Search || (bEndLine && nWhitePixels > 0))
			{
				nTipPos_left = nx;
				break;
			}
		}

		for(nx = nBodyWidth-1; nx > nBodyWidth - nGap; nx--)
		{
			nWhitePixels = 0;
			for(ny = 0; ny < nBodyHeight; ny++)
			{
				if(imgBlob.data[ny * nBodyWidth + nx] == 255)
					nWhitePixels++;
			}
			if(nWhitePixels > dBodyHeigth_Search || (bEndLine && nWhitePixels > 0))
			{
				nTipPos_right = nx;
				if(nx + 1 >= nBodyWidth)
					nTipPos_right = nx + 1;
				break;
			}
		}

		// 3. Body Rect 재계산
		if(nTipPos_left >= 0 && nTipPos_right >= 0)
		{
			rcBody->right = nBodyL + nTipPos_right;
			rcBody->left = nBodyL + nTipPos_left;
		}
		else if (nTipPos_left >= 0)
		{
			rcBody->left = nBodyL + nTipPos_left;
		}
		else if (nTipPos_right >= 0)
		{
			rcBody->right = nBodyL + nTipPos_right;
		}
		else
			bRet = FALSE;
	}
	else	// 세로
	{
		int nGap = nBodyHeight / 5 * 2;
		int nTipPos_top = -1;
		int nTipPos_bottom = -1;
		for(ny = 0; ny < nGap; ny++)
		{
			int nWhitePixels = 0;
			for(nx = 0; nx < nBodyWidth; nx++)
			{
				if(imgBlob.data[ny * nBodyWidth + nx] == 255)
					nWhitePixels++;
			}
			if(nWhitePixels > dBodyWidth_Search || (bEndLine && nWhitePixels > 0))
			{
				nTipPos_top = ny;
				break;
			}
		}

		for(ny = nBodyHeight-1; ny > nBodyHeight - nGap; ny--)
		{
			int nWhitePixels = 0;
			for(nx = 0; nx < nBodyWidth; nx++)
			{
				if(imgBlob.data[ny * nBodyWidth + nx] == 255)
					nWhitePixels++;
			}
			if(nWhitePixels > dBodyWidth_Search || (bEndLine && nWhitePixels > 0))
			{
				nTipPos_bottom = ny;
				if(ny + 1 >= nBodyHeight)
					nTipPos_bottom = ny + 1;
				break;
			}
		}

		// 3. Body Rect 재계산
		if(nTipPos_top >= 0 && nTipPos_bottom >= 0)
		{
			rcBody->bottom = nBodyT + nTipPos_bottom;
			rcBody->top = nBodyT + nTipPos_top;
		}
		else if (nTipPos_top >= 0)
		{
			rcBody->top = nBodyT + nTipPos_top;
		}
		else if (nTipPos_bottom >= 0)
		{
			rcBody->bottom = nBodyT + nTipPos_bottom;
		}
		else
			bRet = FALSE;
	}

	// 3. Area 재계산
	if(bRet)
	{
		nWhitePixels = 0;
		for(ny = rcBody->top; ny < rcBody->bottom; ny++)
		{
			for(nx = rcBody->left; nx < rcBody->right; nx++)
			{
				if(blobImg[ny * sizeX + nx] == 255)
					nWhitePixels++;
			}
		}

		*dArea = nWhitePixels;
	}

	//delete [] tipBWImg;

	return TRUE;
}

int CPInsp_Mount::OnlyBodyTip(AlgoBodyBlob *pAlgo, UCHAR * srcImg2d, float * srcImg3d, UCHAR * rstImg, int sizeX, int sizeY, double dAngle, POINT ptTeachCenter, float fSearchPer_BodyTip, bool bTeach, CRect * rcBody, double * dArea, TotalInspExceptArea stTieArea, double * dRstAngle, double * dRstX, double * dRstY, RstAlgoBodyBlob * pRstAlgo, cv::Mat imgBodyTipRst)
{
	int nOnlyBodyTipRet = -1;
	cv::Mat imgBW(sizeY, sizeX, CV_8UC1, cv::Scalar(0));
	cv::Mat imgBlob(sizeY, sizeX, CV_8UC1, cv::Scalar(0));
	// 1. Binarize
	for(int y = 0; y < sizeY; y++)
	{

		for(int x = 0; x < sizeX; x++)
		{
			int nIndex = (y * sizeX) + x;
			bool b2d = false, b3d = false;
			if(pAlgo->m_bTip2dCheck)
			{
				switch (pAlgo->m_nTip2dRange)
				{
				case eTypeRangeIn:
					b2d = ((pAlgo->m_nTip2dMinValue <= srcImg2d[nIndex]) && (srcImg2d[nIndex] <= pAlgo->m_nTip2dMaxValue));					
					break;
				case eTypeRangeOut:
					b2d = ((pAlgo->m_nTip2dMinValue >= srcImg2d[nIndex]) || (srcImg2d[nIndex] >= pAlgo->m_nTip2dMaxValue));
					break;
				case eTypeRangeUpper:
					b2d = (pAlgo->m_nTip2dMaxValue <= srcImg2d[nIndex]);
					break;
				case eTypeRangeLower:
					b2d = (pAlgo->m_nTip2dMinValue >= srcImg2d[nIndex]);
					break;
				}
			}
			else
				b2d = true;
			if(pAlgo->m_bTip3dCheck)
			{
				switch (pAlgo->m_nTip3dRange)
				{
				case eTypeRangeIn:
					b3d = ((pAlgo->m_dTip3dHeightMin <= srcImg3d[nIndex]) && (srcImg3d[nIndex] <= pAlgo->m_dTip3dHeightMax));					
					break;
				case eTypeRangeOut:
					b3d = ((pAlgo->m_dTip3dHeightMin >= srcImg3d[nIndex]) || (srcImg3d[nIndex] >= pAlgo->m_dTip3dHeightMax));
					break;
				case eTypeRangeUpper:
					b3d = (pAlgo->m_dTip3dHeightMax <= srcImg3d[nIndex]);
					break;
				case eTypeRangeLower:
					b3d = (pAlgo->m_dTip3dHeightMin >= srcImg3d[nIndex]);
					break;
				}
			}
			else
				b3d = true;
			if (b2d && b3d)
				imgBW.data[nIndex] = 255;
			else
				imgBW.data[nIndex] = 0;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(imgBW.data, sizeX, sizeY, _T("OnlyBodyTip_BW.bmp"));
	CPInsp::FillOutOfInspAreaCombine(sizeX, sizeY, 0, imgBW.data, NULL, stTieArea);
	m_pProcMilAlgo->SaveWorkImg(imgBW.data, sizeX, sizeY, _T("OnlyBodyTip_BW_Rst.bmp"));
	int nShiftX = pAlgo->m_dShiftX / m_resolX / 2.0;
	int nBodyWidth = (pAlgo->m_dTeachWidth / m_resolX);
	int nBodyWMin = pAlgo->m_dTeachWidth * pAlgo->m_dTeachWidthRateMin / 100.0 / m_resolX;
	int nBodyWMax = pAlgo->m_dTeachWidth * pAlgo->m_dTeachWidthRateMax / 100.0 / m_resolX;
	int nShiftY = pAlgo->m_dShiftY / m_resolY / 2.0;
	int nBodyHeight = (pAlgo->m_dTeachLength / m_resolY);
	int nBodyHMin = pAlgo->m_dTeachLength * pAlgo->m_dTeachLengthRateMin / 100.0 / m_resolY;
	int nBodyHMax = pAlgo->m_dTeachLength * pAlgo->m_dTeachLengthRateMax / 100.0 / m_resolY;
	if(bTeach == false && (dAngle == 90 || dAngle == 270))
	{
		nShiftX = pAlgo->m_dShiftY / m_resolY / 2.0;
		nBodyWidth = pAlgo->m_dTeachLength / m_resolY;
		nBodyWMin = pAlgo->m_dTeachLength * pAlgo->m_dTeachLengthRateMin / 100.0 / m_resolY;
		nBodyWMax = pAlgo->m_dTeachLength * pAlgo->m_dTeachLengthRateMax / 100.0 / m_resolY;

		nShiftY = pAlgo->m_dShiftX / m_resolX / 2.0;
		nBodyHeight = pAlgo->m_dTeachWidth / m_resolX;
		nBodyHMin = pAlgo->m_dTeachWidth * pAlgo->m_dTeachWidthRateMin / 100.0 / m_resolX;
		nBodyHMax = pAlgo->m_dTeachWidth * pAlgo->m_dTeachWidthRateMax / 100.0 / m_resolX;
	}
	int nMinBlob = (pAlgo->m_bIsHorizon) ? nBodyHeight * 3 : nBodyWidth * 3;
	nMinBlob *= 0.8;
	bool bFillHole = pAlgo->m_bFillHole;
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(imgBW.data, imgBlob.data, sizeX, sizeY, nMinBlob, false, bFillHole, 0, eSelectMix);
	if (bTeach == true && nCntBlob <= 1)
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(imgBW.data, imgBlob.data, sizeX, sizeY, 4, false, bFillHole, 0, eSelectMix);

	if (nCntBlob <= 0)
		return nOnlyBodyTipRet;

	BOOL bInspArea = ((m_nBodyBlobOPT & m_eBodyBlobOPT_INSP_AREA) == m_eBodyBlobOPT_INSP_AREA);
	if (bInspArea && pRstAlgo && bTeach == false && nCntBlob > 0)
	{
		INSP_SHIFT_AREA(pAlgo, dAngle, bTeach, pRstAlgo, sizeX, sizeY, nMinBlob, nCntBlob, imgBlob.data);
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(imgBlob.data, imgBlob.data, sizeX, sizeY, nMinBlob, false, bFillHole, 0, eSelectMix);
	}
	if (nCntBlob <= 0)
		return nOnlyBodyTipRet;

	double *dArrArea = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	double *dArrCX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	double *dArrCY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	CRect *rcArrBlob = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
	m_pProcMilAlgo->GetBlobResult_ALL(dArrArea, dArrCX, dArrCY, rcArrBlob);

	if (nCntBlob > 1)
	{
		int nRMV = BODY_TIP_RMV(pAlgo, dAngle, bTeach, imgBlob, sizeX, sizeY, nCntBlob, rcArrBlob, nMinBlob, fSearchPer_BodyTip);
		if (nRMV <= 0)
		{
			Delete_1DArray(&dArrArea);
			Delete_1DArray(&dArrCX);
			Delete_1DArray(&dArrCY);
			Delete_1DArray(&rcArrBlob);
			return nOnlyBodyTipRet;
		}
		if (nRMV != nCntBlob)
		{
			nCntBlob = nRMV;
			Delete_1DArray(&dArrArea);
			Delete_1DArray(&dArrCX);
			Delete_1DArray(&dArrCY);
			Delete_1DArray(&rcArrBlob);
			dArrArea = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			dArrCX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			dArrCY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			rcArrBlob = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
		}
		m_pProcMilAlgo->GetBlobResult_ALL(dArrArea, dArrCX, dArrCY, rcArrBlob);
	}

	if (nCntBlob == 1)
	{
		nCntBlob = BODY_TIP_DIV(pAlgo, dAngle, bTeach, imgBlob.data, sizeX, sizeY, rcArrBlob[0], fSearchPer_BodyTip, false);
		if (nCntBlob >= 1)
		{
			Delete_1DArray(&dArrArea);
			Delete_1DArray(&dArrCX);
			Delete_1DArray(&dArrCY);
			Delete_1DArray(&rcArrBlob);
			dArrArea = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			dArrCX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			dArrCY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			rcArrBlob = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetBlobResult_ALL(dArrArea, dArrCX, dArrCY, rcArrBlob);
		}
	}
	if (nCntBlob <= 1)
	{
		BOOL bRET = FALSE;
		if (nCntBlob == 1 && pAlgo->m_bOnlyBodyTip == FALSE)
		{
			if (rcArrBlob[0].Width() > nBodyWMin && rcArrBlob[0].Width() < nBodyWMax &&
				rcArrBlob[0].Height() > nBodyHMin && rcArrBlob[0].Height() < nBodyHMax &&
				dArrArea[0] > nBodyWMin * nBodyHMin && dArrArea[0] < nBodyWMax * nBodyHMax)
			{
				*dRstX = dArrCX[0];
				*dRstY = dArrCY[0];

				rcBody->left = rcArrBlob[0].left;
				rcBody->right = rcArrBlob[0].right;
				rcBody->top = rcArrBlob[0].top;
				rcBody->bottom = rcArrBlob[0].bottom;
				bRET = TRUE;
			}
		}
		Delete_1DArray(&dArrArea);
		Delete_1DArray(&dArrCX);
		Delete_1DArray(&dArrCY);
		Delete_1DArray(&rcArrBlob);
		return bRET ? 0 : nOnlyBodyTipRet;
	}

	int nRET_CHK = BODY_TIP_CHK(pAlgo, dAngle, bTeach, imgBlob, nCntBlob, dArrArea, dArrCX, dArrCY, rcArrBlob, dRstX, dRstY);
	if (nRET_CHK < 0)
	{
		Delete_1DArray(&dArrArea);
		Delete_1DArray(&dArrCX);
		Delete_1DArray(&dArrCY);
		Delete_1DArray(&rcArrBlob);
		return nOnlyBodyTipRet;
	}

	m_pProcMilAlgo->SaveWorkImg(imgBlob.data, sizeX, sizeY, _T("OnlyBodyTip_Blob.bmp"));
	bool bDelete = false;
	if (nCntBlob > 2 && pRstAlgo)
	{
		float fOrgCX = ptTeachCenter.x;
		float fOrgCY = ptTeachCenter.y;
		int nCntTemp = 0;
		int nArrID[10];
		double dArrA[10];
		double dArrCXTemp[10];
		double dArrCYTemp[10];
		double dArrAreaTemp[10];
		CRect rcArrBlobTemp[10];
		memset(nArrID, -1, sizeof(int) * 10);
		memset(dArrA, 0, sizeof(double) * 10);
		memset(dArrCXTemp, 0, sizeof(double) * 10);
		memset(dArrCYTemp, 0, sizeof(double) * 10);
		memset(dArrAreaTemp, 0, sizeof(double) * 10);
		memset(rcArrBlobTemp, 0, sizeof(CRect) * 10);

		int nROIW = pRstAlgo->m_rcRect_T.right - pRstAlgo->m_rcRect_T.left;
		int nROIH = pRstAlgo->m_rcRect_T.bottom - pRstAlgo->m_rcRect_T.top;
		int nROICX = pRstAlgo->m_rcRect_T.left + (nROIW / 2);
		int nROICY = pRstAlgo->m_rcRect_T.top + (nROIH / 2);
		for (int nRe = 0; nRe < 3; nRe++)
		{
			nROIW = (pRstAlgo->m_rcRect_T.right - pRstAlgo->m_rcRect_T.left) * (1.0 + (0.1 * (nRe + 1)));
			nROIH = (pRstAlgo->m_rcRect_T.bottom - pRstAlgo->m_rcRect_T.top) * (1.0 + (0.1 * (nRe + 1)));
			CRect rtROI;
			rtROI.left = nROICX - (nROIW / 2);
			rtROI.right = rtROI.left + nROIW;
			rtROI.top = nROICY - (nROIH / 2);
			rtROI.bottom = rtROI.top + nROIH;

			for (int a = 0; a < nCntBlob; a++)
			{
				for (int b = 0; b < nCntBlob; b++)
				{
					bool bIn = true;
					for (int c = 0; c < a; c++)
					{
						if (nArrID[c] == b)
						{
							bIn = false;
							break;
						}
					}
					if (bIn == false) continue;
					CRect rtBlob;
					rtBlob.left = rcArrBlob[b].left;
					rtBlob.right = rcArrBlob[b].right;
					rtBlob.top = rcArrBlob[b].top;
					rtBlob.bottom = rcArrBlob[b].bottom;
					CRect rtDst;
					if (rtDst.IntersectRect(rtROI, rtBlob) == FALSE)
						continue;
					double dA = rtDst.Width() * rtDst.Height();
					if (dA > dArrA[a])
					{
						dArrA[a] = dA;
						nArrID[a] = b;
						dArrCXTemp[a] = dArrCX[b];
						dArrCYTemp[a] = dArrCY[b];
						dArrAreaTemp[a] = dArrArea[b];
						rcArrBlobTemp[a].left = rcArrBlob[b].left;
						rcArrBlobTemp[a].right = rcArrBlob[b].right;
						rcArrBlobTemp[a].top = rcArrBlob[b].top;
						rcArrBlobTemp[a].bottom = rcArrBlob[b].bottom;
						nCntTemp = a + 1;
					}
				}
				if (nCntTemp >= 10) break;
			}
			if (nCntTemp >= 2)
				break;
		}
		if (nCntTemp > 0)
		{
			nCntBlob = nCntTemp;
			for (int a = 0; a < nCntTemp; a++)
			{
				dArrCX[a] = dArrCXTemp[a];
				dArrCY[a] = dArrCYTemp[a];
				dArrArea[a] = dArrAreaTemp[a];
				rcArrBlob[a].left = rcArrBlobTemp[a].left;
				rcArrBlob[a].right = rcArrBlobTemp[a].right;
				rcArrBlob[a].top = rcArrBlobTemp[a].top;
				rcArrBlob[a].bottom = rcArrBlobTemp[a].bottom;
			}
			bDelete = true;
		}
	}
	if (nCntBlob > 3)
	{
		float fOrgCX = ptTeachCenter.x;
		float fOrgCY = ptTeachCenter.y;
		int nArrID[3] = {-1, -1, -1};
		double dArrDist[3] = {sizeX * sizeY, sizeX * sizeY, sizeX * sizeY};
		double dArrCXTemp[3];
		double dArrCYTemp[3];
		double dArrAreaTemp[3];
		CRect rcArrBlobTemp[3];
		for (int a = 0; a < 3; a++)
		{
			for (int b = 0; b < nCntBlob; b++)
			{
				bool bIn = true;
				for (int c = 0; c < a; c++)
				{
					if (nArrID[c] == b)
					{
						bIn = false;
						break;
					}
				}
				if (bIn == false) continue;
				if (pAlgo->m_bIsHorizon == TRUE)
				{
					if (nBodyHMin > rcArrBlob[b].Height() || nBodyHMax < rcArrBlob[b].Height())
						continue;
					double dDistX = sqrt(pow((fOrgCX - dArrCX[b]), 2));
					if (nBodyWMin / 2 - nShiftX > dDistX || nBodyWMax / 2 + nShiftX < dDistX)
						continue;
				}
				else
				{
					if (nBodyWMin > rcArrBlob[b].Width() || nBodyWMax < rcArrBlob[b].Width())
						continue;
					double dDistY = sqrt(pow((fOrgCY - dArrCY[b]), 2));
					if (nBodyHMin / 2 - nShiftY > dDistY || nBodyHMax / 2 + nShiftY < dDistY)
						continue;
				}
				double dDist = sqrt(pow((fOrgCX - dArrCX[b]), 2) + pow((fOrgCY - dArrCY[b]), 2));
				if (dArrDist[a] > dDist)
				{
					dArrDist[a] = dDist;
					nArrID[a] = b;
					dArrCXTemp[a] = dArrCX[b];
					dArrCYTemp[a] = dArrCY[b];
					dArrAreaTemp[a] = dArrArea[b];
					rcArrBlobTemp[a].left = rcArrBlob[b].left;
					rcArrBlobTemp[a].right = rcArrBlob[b].right;
					rcArrBlobTemp[a].top = rcArrBlob[b].top;
					rcArrBlobTemp[a].bottom = rcArrBlob[b].bottom;
				}
			}
		}
		nCntBlob = 3;
		for (int a = 0; a < nCntBlob; a++)
		{
			dArrCX[a] = dArrCXTemp[a];
			dArrCY[a] = dArrCYTemp[a];
			dArrArea[a] = dArrAreaTemp[a];
			rcArrBlob[a].left = rcArrBlobTemp[a].left;
			rcArrBlob[a].right = rcArrBlobTemp[a].right;
			rcArrBlob[a].top = rcArrBlobTemp[a].top;
			rcArrBlob[a].bottom = rcArrBlobTemp[a].bottom;
		}
	}

	if (nCntBlob == 1)
	{
		nCntBlob = BODY_TIP_DIV(pAlgo, dAngle, bTeach, imgBlob.data, sizeX, sizeY, rcArrBlob[0], fSearchPer_BodyTip, true);
		if (nCntBlob == 2)
		{
			Delete_1DArray(&dArrArea);
			Delete_1DArray(&dArrCX);
			Delete_1DArray(&dArrCY);
			Delete_1DArray(&rcArrBlob);
			dArrArea = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			dArrCX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			dArrCY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			rcArrBlob = g_pMManager->pem_new<CRect>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
			m_pProcMilAlgo->GetBlobResult_ALL(dArrArea, dArrCX, dArrCY, rcArrBlob);
			bDelete = false;

			nRET_CHK = BODY_TIP_CHK(pAlgo, dAngle, bTeach, imgBlob, nCntBlob, dArrArea, dArrCX, dArrCY, rcArrBlob, dRstX, dRstY);
			if (nRET_CHK < 0)
			{
				Delete_1DArray(&dArrArea);
				Delete_1DArray(&dArrCX);
				Delete_1DArray(&dArrCY);
				Delete_1DArray(&rcArrBlob);
				return nOnlyBodyTipRet;
			}
		}
	}
	if (nCntBlob < 2)
	{
		Delete_1DArray(&dArrArea);
		Delete_1DArray(&dArrCX);
		Delete_1DArray(&dArrCY);
		Delete_1DArray(&rcArrBlob);
		return nOnlyBodyTipRet;
	}
	double dOrgX = ptTeachCenter.x;
	double dOrgY = ptTeachCenter.y;
	double dGapX = pAlgo->m_dShiftX * 1.5 / m_resolX;
	double dGapY = pAlgo->m_dShiftY * 1.5 / m_resolY;
	if (dGapX < 10) dGapX = 10;
	if (dGapY < 10) dGapY = 10;
	int nFindIdx_1 = -1;
	int nFindIdx_2 = -1;
	int nFindIdx_1_Temp = -1;
	int nFindIdx_2_Temp = -1;
	int nTotal = (nCntBlob == 2) ? 3 : 6;
	double dDistMin = sizeX * sizeY;
	double dDistMinXY = sizeX * sizeY;
	double dDistMin_Temp = sizeX * sizeY;
	double dDistMinXY_Temp = sizeX * sizeY;
	double dDMin = 0.08;
	for (int a = 0; a < nTotal; a++)
	{
		float fDstCX = 0.0f;
		float fDstCY = 0.0f;
		float fDstW = 0.0f;
		float fDstH = 0.0f;
		int nID1 = a;
		int nID2 = -1;
		if ((a == 3 && nTotal == 6) || (a == 2 && nTotal == 3))
		{
			nID1 = 0;
			nID2 = 1;
		}
		else if (a == 4)
		{
			nID1 = 0;
			nID2 = 2;
		}
		else if (a == 5)
		{
			nID1 = 1;
			nID2 = 2;
		}
		if ((pAlgo->m_bIsHorizon == FALSE && (dOrgX - dGapX > dArrCX[nID1] || dOrgX + dGapX < dArrCX[nID1])) ||
			(pAlgo->m_bIsHorizon == TRUE && (dOrgY - dGapY > dArrCY[nID1] || dOrgY + dGapY < dArrCY[nID1])))
			continue;
		if ((a < 3 && nTotal == 6) || (a < 2 && nTotal == 3))
		{
			double dAreaW = rcArrBlob[nID1].right - rcArrBlob[nID1].left;
			double dAreaH = rcArrBlob[nID1].bottom - rcArrBlob[nID1].top;
			if (pAlgo->m_bIsHorizon == FALSE)
			{
				if (nBodyWMin > dAreaW || dAreaW > nBodyWMax)
					continue;
			}
			else
			{
				if (nBodyHMin > dAreaH || dAreaH > nBodyHMax)
					continue;
			}
			fDstCX = dArrCX[nID1];
			fDstCY = dArrCY[nID1];
			fDstW = dAreaW;
			fDstH = dAreaH;
		}
		else
		{
			if (pAlgo->m_bIsHorizon == TRUE)
			{
				if (dOrgY - dGapY > dArrCY[nID2])
					continue;
				else if (dOrgY + dGapY < dArrCY[nID2])
					continue;
				else if (nBodyHMin > rcArrBlob[nID1].Height() || nBodyHMax < rcArrBlob[nID1].Height())
					continue;
				else if (nBodyHMin > rcArrBlob[nID2].Height() || nBodyHMax < rcArrBlob[nID2].Height())
					continue;
			}
			else
			{
				if (dOrgX - dGapX > dArrCX[nID2])
					continue;
				else if (dOrgX + dGapX < dArrCX[nID2])
					continue;
				else if (nBodyWMin > rcArrBlob[nID1].Width() || nBodyWMax < rcArrBlob[nID1].Width())
					continue;
				else if (nBodyWMin > rcArrBlob[nID2].Width() || nBodyWMax < rcArrBlob[nID2].Width())
					continue;
			}
			fDstCX = (dArrCX[nID1] + dArrCX[nID2]) / 2.0;
			fDstCY = (dArrCY[nID1] + dArrCY[nID2]) / 2.0;
			double dLTemp = (rcArrBlob[nID1].left > rcArrBlob[nID2].left) ? rcArrBlob[nID2].left : rcArrBlob[nID1].left;
			double dRTemp = (rcArrBlob[nID1].right < rcArrBlob[nID2].right) ? rcArrBlob[nID2].right : rcArrBlob[nID1].right;
			double dTTemp = (rcArrBlob[nID1].top > rcArrBlob[nID2].top) ? rcArrBlob[nID2].top : rcArrBlob[nID1].top;
			double dBTemp = (rcArrBlob[nID1].bottom < rcArrBlob[nID2].bottom) ? rcArrBlob[nID2].bottom : rcArrBlob[nID1].bottom;
			fDstW = dRTemp - dLTemp;
			fDstH = dBTemp - dTTemp;
			double dTempX = (dRTemp + dLTemp) / 2.0;
			double dTempY = (dBTemp + dTTemp) / 2.0;
			double dTempGapX = abs(dTempX - fDstCX);
			double dTempGapY = abs(dTempY - fDstCY);
			if (dTempGapX > fDstW * dDMin) fDstCX = dTempX;
			if (dTempGapY > fDstH * dDMin) fDstCY = dTempY;
		}
		if (fDstW <= 0 || fDstH <= 0)
			continue;

		if (dOrgX - dGapX > fDstCX || dOrgX + dGapX < fDstCX ||
			dOrgY - dGapY > fDstCY || dOrgY + dGapY < fDstCY)
			continue;

		if (nBodyWMin > fDstW || nBodyWMax < fDstW)
			continue;
		if (nBodyHMin > fDstH || nBodyHMax < fDstH)
			continue;

		double dBodyArea = nBodyWidth * nBodyHeight;
		double dDstArea = fDstW * fDstH;
		double dDstPer = (dDstArea / dBodyArea) * 100.0;

		double dDistW = nBodyWidth - fDstW;
		double dDistH = nBodyHeight - fDstH;
		if (dDistW == 0) dDistW = 1;
		if (dDistH == 0) dDistH = 1;
		double dAreaTemp = sqrt(pow((dDistW), 2) * pow((dDistH), 2));

		double dDistCX = dOrgX - fDstCX;
		double dDistCY = dOrgY - fDstCY;
		if (dDistCX == 0) dDistCX = 1;
		if (dDistCY == 0) dDistCY = 1;
		double dAreaTempXY = sqrt(pow((dDistCX), 2) * pow((dDistCY), 2));

		bool bFindDist = false;
		if (dDistMin > dAreaTemp)
			bFindDist = true;
		else if (pAlgo->m_dAreaMin < dDstPer && dDstPer < pAlgo->m_dAreaMax)
		{
			if (dDistMinXY > dAreaTempXY)
				bFindDist = true;
		}

		if (dDistMinXY_Temp > dAreaTempXY)
		{
			dDistMin_Temp = dAreaTemp;
			dDistMinXY_Temp = dAreaTempXY;
			nFindIdx_1_Temp = nID1;
			nFindIdx_2_Temp = nID2;
		}

		if (bFindDist)
		{
			dDistMin = dAreaTemp;
			dDistMinXY = dAreaTempXY;
			nFindIdx_1 = nID1;
			nFindIdx_2 = nID2;
			continue;
		}
	}
	if (nFindIdx_2 == -1)
	{
		if (nFindIdx_1_Temp != -1 && nFindIdx_2_Temp != -1)
		{
			nFindIdx_1 = nFindIdx_1_Temp;
			nFindIdx_2 = nFindIdx_2_Temp;
		}
	}

	if (nFindIdx_2 == -1)
	{
		if (nFindIdx_1 > -1)
		{
			if (rstImg != NULL)
			{
				memcpy_s(rstImg, sizeof(UCHAR) * sizeX * sizeY, imgBlob.data, sizeof(UCHAR) * sizeX * sizeY);
				m_pProcMilAlgo->SaveWorkImg(rstImg, sizeX, sizeY, _T("OnlyBodyTip_Blob_Image.bmp"));
			}
		}
		Delete_1DArray(&dArrArea);
		Delete_1DArray(&dArrCX);
		Delete_1DArray(&dArrCY);
		Delete_1DArray(&rcArrBlob);
		if(nFindIdx_1 > -1)
			return 0;
		else
			return nOnlyBodyTipRet;
	}
	if(nCntBlob > 2 || bDelete)
	{
		for (int y = 0; y < sizeY; y++)
		{
			for (int x = 0; x < sizeX; x++)
			{
				int nIndex = (y * sizeX) + x;
				if (imgBlob.data[nIndex] == 0) continue;
				bool bDelete1 = true;
				bool bDelete2 = true;
				if (rcArrBlob[nFindIdx_1].left < x && rcArrBlob[nFindIdx_1].right > x &&
					rcArrBlob[nFindIdx_1].top < y && rcArrBlob[nFindIdx_1].bottom > y)
					bDelete1 = false;
				if (rcArrBlob[nFindIdx_2].left < x && rcArrBlob[nFindIdx_2].right > x &&
					rcArrBlob[nFindIdx_2].top < y && rcArrBlob[nFindIdx_2].bottom > y)
					bDelete2 = false;
				if (bDelete1 && bDelete2)
					imgBlob.data[nIndex] = 0;
			}
		}
		m_pProcMilAlgo->SaveWorkImg(imgBlob.data, sizeX, sizeY, _T("OnlyBodyTip_Blob_Delete.bmp"));
	}
	double dA = 0, dB = 0;
	double dArrX[2], dArrY[2];
	if (nFindIdx_1 != nFindIdx_2 && nFindIdx_1 >= 0 && nFindIdx_2 >= 0)
	{
		memcpy(imgBodyTipRst.data, imgBlob.data, sizeof(UCHAR) * sizeX * sizeY);
		nOnlyBodyTipRet = 1;
	}
	else
		nOnlyBodyTipRet = 0;
	if (pAlgo->m_bIsHorizon)
	{
		int nA = rcArrBlob[nFindIdx_1].top;
		int nB = rcArrBlob[nFindIdx_1].bottom;
		if(nA > rcArrBlob[nFindIdx_2].top)
			nA = rcArrBlob[nFindIdx_2].top;
		if(nB > rcArrBlob[nFindIdx_2].bottom)
			nB = rcArrBlob[nFindIdx_2].bottom;
		int nGap = (nB - nA) / 2 + 1;
		dArrY[0] = rcArrBlob[nFindIdx_1].top;
		dArrY[1] = rcArrBlob[nFindIdx_2].top;
		dArrX[0] = dArrCX[nFindIdx_1];
		dArrX[1] = dArrCX[nFindIdx_2];
		CPInsp::GetGradient(dArrX, dArrY, 2, dA, dB, pAlgo->m_bIsHorizon);
		int nL_S = (dArrCX[nFindIdx_1] < dArrCX[nFindIdx_2]) ? dArrCX[nFindIdx_1] : dArrCX[nFindIdx_2];
		int nR_S = (dArrCX[nFindIdx_1] < dArrCX[nFindIdx_2]) ? dArrCX[nFindIdx_2] : dArrCX[nFindIdx_1];
		for(int x = nL_S; x < nR_S; x++)
		{
			int nY = dA * x + dB;
			if(nY < 0 || nY > sizeY)
				continue;
			for (int y = nY; y < nY + nGap; y++)
			{
				int nIndex = (y * sizeX) + x;
				if (nIndex < 0 || nIndex >= sizeX * sizeY)
					continue;
				imgBlob.data[nIndex] = 255;
			}
		}
		m_pProcMilAlgo->SaveWorkImg(imgBlob.data, sizeX, sizeY, _T("OnlyBodyTip_Blob_H_1.bmp"));
		dArrY[0] = rcArrBlob[nFindIdx_1].bottom;
		dArrY[1] = rcArrBlob[nFindIdx_2].bottom;
		CPInsp::GetGradient(dArrX, dArrY, 2, dA, dB, pAlgo->m_bIsHorizon);
		for (int x = nL_S; x < nR_S; x++)
		{
			int nY = dA * x + dB;
			if (nY < 0 || nY > sizeY)
				continue;
			for (int y = nY; y > nY - nGap; y--)
			{
				int nIndex = (y * sizeX) + x;
				if (nIndex < 0 || nIndex >= sizeX * sizeY)
					continue;
				imgBlob.data[nIndex] = 255;
			}
		}
		m_pProcMilAlgo->SaveWorkImg(imgBlob.data, sizeX, sizeY, _T("OnlyBodyTip_Blob_H_2.bmp"));
	}
	else
	{
		int nA = rcArrBlob[nFindIdx_1].left;
		int nB = rcArrBlob[nFindIdx_1].right;
		if(nA > rcArrBlob[nFindIdx_2].left)
			nA = rcArrBlob[nFindIdx_2].left;
		if(nB > rcArrBlob[nFindIdx_2].right)
			nB = rcArrBlob[nFindIdx_2].right;
		int nGap = (nB - nA) / 2 + 1;
		dArrX[0] = rcArrBlob[nFindIdx_1].left;
		dArrX[1] = rcArrBlob[nFindIdx_2].left;
		dArrY[0] = dArrCY[nFindIdx_1];
		dArrY[1] = dArrCY[nFindIdx_2];
		CPInsp::GetGradient(dArrX, dArrY, 2, dA, dB, pAlgo->m_bIsHorizon);
		int nL_S = (dArrCY[nFindIdx_1] < dArrCY[nFindIdx_2]) ? dArrCY[nFindIdx_1] : dArrCY[nFindIdx_2];
		int nR_S = (dArrCY[nFindIdx_1] < dArrCY[nFindIdx_2]) ? dArrCY[nFindIdx_2] : dArrCY[nFindIdx_1];
		for(int x = nL_S; x < nR_S; x++)
		{
			int nY = dA * x + dB;
			if(nY < 0 || nY > sizeX)
				continue;
			for(int y = nY; y < nY + nGap; y++)
			{
				int nIndex = (x * sizeX) + y;
				if (nIndex < 0 || nIndex >= sizeX * sizeY)
					continue;
				imgBlob.data[nIndex] = 255;
			}
		}
		m_pProcMilAlgo->SaveWorkImg(imgBlob.data, sizeX, sizeY, _T("OnlyBodyTip_Blob_V_1.bmp"));
		dArrX[0] = rcArrBlob[nFindIdx_1].right;
		dArrX[1] = rcArrBlob[nFindIdx_2].right;
		CPInsp::GetGradient(dArrX, dArrY, 2, dA, dB, pAlgo->m_bIsHorizon);
		for (int x = nL_S; x < nR_S; x++)
		{
			int nY = dA * x + dB;
			if (nY < 0 || nY > sizeX)
				continue;
			for (int y = nY; y > nY - nGap; y--)
			{
				int nIndex = (x * sizeX) + y;
				if (nIndex < 0 || nIndex >= sizeX * sizeY)
					continue;
				imgBlob.data[nIndex] = 255;
			}
		}
		m_pProcMilAlgo->SaveWorkImg(imgBlob.data, sizeX, sizeY, _T("OnlyBodyTip_Blob_V_2.bmp"));
	}
	int nWCnt = 0;
	rcBody->left = 0;
	rcBody->right = 0;
	rcBody->top = 0;
	rcBody->bottom = 0;
	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			int nIndex = (y * sizeX) + x;
			if (imgBlob.data[nIndex] == 0) continue;
			nWCnt++;
			if (rcBody->left == 0 && rcBody->right == 0 &&
				rcBody->top == 0 && rcBody->bottom == 0)
			{
				rcBody->left = x;
				rcBody->right = x;
				rcBody->top = y;
				rcBody->bottom = y;
			}
			if (rcBody->left > x) rcBody->left = x;
			if (rcBody->right < x) rcBody->right = x;
			if (rcBody->top > y) rcBody->top = y;
			if (rcBody->bottom < y) rcBody->bottom = y;
		}
	}
	double dArrGetGradientCX[2] = { dArrCX[nFindIdx_1], dArrCX[nFindIdx_2] };
	double dArrGetGradientCY[2] = { dArrCY[nFindIdx_1], dArrCY[nFindIdx_2] };
	double dGetGradientA = 0, dGetGradientB = 0;
	double dTheta = (pAlgo->m_bIsHorizon) ? -1.0 : 1.0;
	*dRstAngle = CPInsp::GetGradient(dArrGetGradientCX, dArrGetGradientCY, 2.0, dGetGradientA, dGetGradientB, pAlgo->m_bIsHorizon) * dTheta;
	*dArea = (double)nWCnt;
	*dRstX = (dArrCX[nFindIdx_1] + dArrCX[nFindIdx_2]) / 2.0;
	*dRstY = (dArrCY[nFindIdx_1] + dArrCY[nFindIdx_2]) / 2.0;
	nCntBlob = m_pProcMilAlgo->CalcBlob_Select(imgBlob.data, imgBW.data, sizeX, sizeY, nMinBlob, false, bFillHole, 0, eSelectMix);
	if (nCntBlob == 1)
	{
		Delete_1DArray(&dArrCX);
		Delete_1DArray(&dArrCY);
		dArrCX = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
		dArrCY = g_pMManager->pem_new<double>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);
		m_pProcMilAlgo->GetBlobResult_ALL(NULL, dArrCX, dArrCY, NULL);
		*dRstX = dArrCX[0];
		*dRstY = dArrCY[0];
	}
	m_pProcMilAlgo->SaveWorkImg(imgBlob.data, sizeX, sizeY, _T("OnlyBodyTip_Blob_Result.bmp"));
	if (rstImg != NULL)
	{
		memcpy_s(rstImg, sizeof(UCHAR) * sizeX * sizeY, imgBlob.data, sizeof(UCHAR) * sizeX * sizeY);
		m_pProcMilAlgo->SaveWorkImg(rstImg, sizeX, sizeY, _T("OnlyBodyTip_Blob_Image.bmp"));
	}
	Delete_1DArray(&dArrArea);
	Delete_1DArray(&dArrCX);
	Delete_1DArray(&dArrCY);
	Delete_1DArray(&rcArrBlob);
	return nOnlyBodyTipRet;
}

int CPInsp_Mount::ClearModelList(int nLane)
{
	if (m_procMil == NULL)
		return eMR_FAIL;
	return m_procMil->ClearModelList(nLane);
}

int CPInsp_Mount::LoadModelList(CString sPath)
{
	if (m_procMil == NULL)
		return eMR_FAIL;
	return m_procMil->LoadModelList(sPath);
}

int CPInsp_Mount::CheckModelList(CString path)
{
	if (m_procMil == NULL)
		return eMR_FAIL;
	return m_procMil->CheckModelList(path);
}

void CPInsp_Mount::INSP_SHIFT_AREA(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, RstAlgoBodyBlob * pRstAlgo, int nW, int nH, double dMinArea, int nCntBlob, UCHAR* ucArrBlobDst, float fSearchPer)
{
	int nLine = __LINE__;
	try
	{
		if (pAlgo == NULL || pRstAlgo == NULL || ucArrBlobDst == NULL)
			return;

		BOOL bUNIT_FIND = ((m_nBodyBlobOPT & m_eBodyBlobOPT_UNIT_FIND) == m_eBodyBlobOPT_UNIT_FIND);
		bool bROT = (dAngle == 90 || dAngle == 270);
		double dResolX = (bROT) ? m_resolY : m_resolX;
		double dResolY = (bROT) ? m_resolX : m_resolY;
		double dShiftX = (bROT) ? pAlgo->m_dShiftY : pAlgo->m_dShiftX;
		double dShiftY = (bROT) ? pAlgo->m_dShiftX : pAlgo->m_dShiftY;
		double dTeachW = (bROT && bTeach == false) ? pAlgo->m_dTeachLength : pAlgo->m_dTeachWidth;
		double dTeachL = (bROT && bTeach == false) ? pAlgo->m_dTeachWidth : pAlgo->m_dTeachLength;
		double dTeachWMin = (bROT) ? pAlgo->m_dTeachLengthRateMin : pAlgo->m_dTeachWidthRateMin;
		double dTeachLMin = (bROT) ? pAlgo->m_dTeachWidthRateMin : pAlgo->m_dTeachLengthRateMin;
		double dTeachWMax = (bROT) ? pAlgo->m_dTeachLengthRateMax : pAlgo->m_dTeachWidthRateMax;
		double dTeachLMax = (bROT) ? pAlgo->m_dTeachWidthRateMax : pAlgo->m_dTeachLengthRateMax;

		int nShiftX = (dShiftX / dResolX) + 1;
		int nShiftY = (dShiftY / dResolY) + 1;
		if (nShiftX <= 0 || nShiftY <= 0)
			return;
		int nL_T = pRstAlgo->m_rcRect_T.left - nShiftX;
		int nT_T = pRstAlgo->m_rcRect_T.top - nShiftY;
		int nR_T = pRstAlgo->m_rcRect_T.right + nShiftX;
		int nB_T = pRstAlgo->m_rcRect_T.bottom + nShiftY;
		if (nL_T < 0) nL_T = 0;
		if (nT_T < 0) nT_T = 0;
		if (nR_T < 0) nR_T = 0;
		if (nB_T < 0) nB_T = 0;
		if (nL_T >= nW) nL_T = nW - 1;
		if (nT_T >= nH) nT_T = nH - 1;
		if (nR_T >= nW) nR_T = nW;
		if (nB_T >= nH) nB_T = nH;
		int nW_T = nR_T - nL_T;
		int nH_T = nB_T - nT_T;
		if (nW_T <= 0 || nH_T <= 0)
			return;

		int nBodyW = (dTeachW / dResolX);
		int nBodyWMin = dTeachW * dTeachWMin / 100.0 / dResolX;
		int nBodyWMax = dTeachW * dTeachWMax / 100.0 / dResolX;
		int nBodyH = (dTeachL / dResolY);
		int nBodyHMin = dTeachL * dTeachLMin / 100.0 / dResolY;
		int nBodyHMax = dTeachL * dTeachLMax / 100.0 / dResolY;
		double dBodySearchW = (double)nBodyWMin * (fSearchPer / 100.0 / 2.0);
		double dBodySearchH = (double)nBodyHMin * (fSearchPer / 100.0 / 2.0);

		double dW_T = dTeachW / dResolX + 1;
		double dH_T = dTeachL / dResolY + 1;
		double dMinPer = dTeachWMin > dTeachLMin ? dTeachLMin : dTeachWMin;
		double dW_Per = dW_T * dTeachWMin / 100.0;
		double dH_Per = dH_T * dTeachLMin / 100.0;
		double dMinBlobArea = dMinArea > 0 ? dMinArea : (dW_Per * dH_Per) * dMinPer / 100.0;
		if (dMinBlobArea < 10)	dMinBlobArea = 10;

		cv::Mat imgDst(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgInspArea(nH, nW, CV_8UC1, cv::Scalar(0));
		imgInspArea(cv::Rect(nL_T, nT_T, nW_T, nH_T)).setTo(255);
		CMilBlobResult blobRes(nCntBlob);
		m_pProcMilAlgo->GetBlobResult(&blobRes);

		Im::PIL_ID milBlobResultId = m_pProcMilAlgo->GetBlobResultId();
		for (int a = 0; a < nCntBlob; a++)
		{
			cv::Mat imgTemp = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
			m_pProcMilAlgo->DrawBlob(milBlobResultId, (Im::PIL_ID)&imgTemp, blobRes.blobLabel[a]);

			if (pAlgo->m_bUseBodyTip == FALSE && bUNIT_FIND && fSearchPer > 10 && dBodySearchW > 10 && dBodySearchH > 10)
			{
				for (int y = 0; y < nH; y++)
				{
					int nWhite = 0;
					for (int x = 0; x < nW; x++)
					{
						int nIndex = (y * nW) + x;
						if (imgTemp.data[nIndex] == 255)
							nWhite++;
					}
					if (dBodySearchW > nWhite)
					{
						for (int x = 0; x < nW; x++)
						{
							int nIndex = (y * nW) + x;
							if (imgTemp.data[nIndex] == 255)
								imgTemp.data[nIndex] = 0;
						}
					}
				}

				for (int x = 0; x < nW; x++)
				{
					int nWhite = 0;
					for (int y = 0; y < nH; y++)
					{
						int nIndex = (y * nW) + x;
						if (imgTemp.data[nIndex] == 255)
							nWhite++;
					}
					if (dBodySearchH > nWhite)
					{
						for (int y = 0; y < nH; y++)
						{
							int nIndex = (y * nW) + x;
							if (imgTemp.data[nIndex] == 255)
								imgTemp.data[nIndex] = 0;
						}
					}
				}
			}

			cv::Mat imgAND;
			cv::bitwise_and(imgTemp, imgInspArea, imgAND);
			int nWhite = cv::countNonZero(imgAND);
			if (dMinBlobArea > nWhite)
				continue;

			cv::bitwise_or(imgDst, imgTemp, imgDst);
				}

		memcpy(ucArrBlobDst, imgDst.data, sizeof(UCHAR) * nW * nH);
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Mount::INSP_SHIFT_AREA(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
}
int CPInsp_Mount::BODY_TIP_DIV(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, UCHAR* pucBlob, int nW, int nH, CRect rcROI, float fSearchPer_BodyTip, bool bUnit)
{
	int nLine = __LINE__;
	int nCntBlob = 0;
	try
	{
		if (pAlgo == NULL || pucBlob == NULL)
			return nCntBlob;
		bool bROT = (dAngle == 90 || dAngle == 270);
		double dResolX = (bROT) ? m_resolY : m_resolX;
		double dResolY = (bROT) ? m_resolX : m_resolY;
		double dShiftX = (bROT) ? pAlgo->m_dShiftY : pAlgo->m_dShiftX;
		double dShiftY = (bROT) ? pAlgo->m_dShiftX : pAlgo->m_dShiftY;
		double dTeachW = (bROT && bTeach == false) ? pAlgo->m_dTeachLength : pAlgo->m_dTeachWidth;
		double dTeachL = (bROT && bTeach == false) ? pAlgo->m_dTeachWidth : pAlgo->m_dTeachLength;
		double dTeachWMin = (bROT) ? pAlgo->m_dTeachLengthRateMin : pAlgo->m_dTeachWidthRateMin;
		double dTeachLMin = (bROT) ? pAlgo->m_dTeachWidthRateMin : pAlgo->m_dTeachLengthRateMin;
		double dTeachWMax = (bROT) ? pAlgo->m_dTeachLengthRateMax : pAlgo->m_dTeachWidthRateMax;
		double dTeachLMax = (bROT) ? pAlgo->m_dTeachWidthRateMax : pAlgo->m_dTeachLengthRateMax;
		int nBodyW = (dTeachW / dResolX);
		int nBodyWMin = dTeachW * dTeachWMin / 100.0 / dResolX;
		int nBodyWMax = dTeachW * dTeachWMax / 100.0 / dResolX;
		int nBodyH = (dTeachL / dResolY);
		int nBodyHMin = dTeachL * dTeachLMin / 100.0 / dResolY;
		int nBodyHMax = dTeachL * dTeachLMax / 100.0 / dResolY;
		if (rcROI.Width() < nBodyWMin || rcROI.Width() > nBodyWMax ||
			rcROI.Height() < nBodyHMin || rcROI.Height() > nBodyHMax)
			return nCntBlob;

		cv::Mat imgBin(nH, nW, CV_8UC1, cv::Scalar(0));
		cv::Mat imgBlob(nH, nW, CV_8UC1, pucBlob);
		int nBlobW = rcROI.Width() + 1;
		int nBlobH = rcROI.Height() + 1;
		if (rcROI.left + nBlobW > nW) nBlobW = nW - rcROI.left;
		if (rcROI.top + nBlobH > nH) nBlobH = nH - rcROI.top;
		if (nBlobW <= 0 || nBlobH <= 0)
			return nCntBlob;

		imgBin(cv::Rect(rcROI.left, rcROI.top, nBlobW, nBlobH)).setTo(255);
		cv::bitwise_and(imgBin, imgBlob, imgBlob);

		BOOL bHO = pAlgo->m_bIsHorizon;
		BOOL bUNIT_FIND = ((m_nBodyBlobOPT & m_eBodyBlobOPT_UNIT_FIND) == m_eBodyBlobOPT_UNIT_FIND) && bUnit;
		int nS = (bHO) ? rcROI.left : rcROI.top;
		int nE = (bHO) ? rcROI.right : rcROI.bottom;
		int nST = (bHO) ? rcROI.top : rcROI.left;
		int nET = (bHO) ? rcROI.bottom : rcROI.right;
		double dBody = (bHO) ? nBodyH : nBodyW;
		double dBodySearch = dBody * (fSearchPer_BodyTip / 100.0);
		for (int i = nS; i <= nE; i++)
		{
			int nTotal = 0;
			int nSUM = 0;
			int nStart = -1;
			for (int j = nST; j <= nET; j++)
			{
				int nIndex = bHO ? (j * nW) + i : (i * nW) + j;
				if (imgBlob.data[nIndex] == 255)
				{
					nTotal++;
					nSUM++;
					if (nStart < 0)
						nStart = j;
				}
				else
				{
					if (bUNIT_FIND && nStart >= 0 && dBodySearch > nSUM)
					{
						for (int k = nStart; k <= j; k++)
						{
							int nIndex = bHO ? (k * nW) + i : (i * nW) + k;
							imgBlob.data[nIndex] = 0;
						}
					}
					nSUM = 0;
					nStart = -1;
				}
			}
			if (nTotal >= dBodySearch)
				continue;
			for (int j = nST; j <= nET; j++)
			{
				int nIndex = bHO ? (j * nW) + i : (i * nW) + j;
				imgBlob.data[nIndex] = 0;
			}
		}
		int nMinBlob = dBody * 3.0 * 0.8;
		bool bFillHole = pAlgo->m_bFillHole;
		nCntBlob = m_pProcMilAlgo->CalcBlob_Select(imgBlob.data, imgBlob.data, nW, nH, nMinBlob, false, bFillHole, 0, eSelectMix);
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Mount::BODY_TIP_DIV(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
	return nCntBlob;
}
int CPInsp_Mount::BODY_TIP_CHK(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, cv::Mat imgBlob, int nCntBlob, double *dArrArea, double *dArrCX, double *dArrCY, CRect *rcArrBlob, double * dRstX, double * dRstY)
{
	int nLine = __LINE__;
	try
	{
		if (pAlgo == NULL || imgBlob.empty())
			return -2;
		if (dArrArea == NULL || dArrCX == NULL || dArrCY == NULL || rcArrBlob == NULL)
			return -3;
		if (nCntBlob != 2)
			return 0;
		bool bROT = (dAngle == 90 || dAngle == 270);
		double dResolX = (bROT) ? m_resolY : m_resolX;
		double dResolY = (bROT) ? m_resolX : m_resolY;
		double dShiftX = (bROT) ? pAlgo->m_dShiftY : pAlgo->m_dShiftX;
		double dShiftY = (bROT) ? pAlgo->m_dShiftX : pAlgo->m_dShiftY;
		double dTeachW = (bROT && bTeach == false) ? pAlgo->m_dTeachLength : pAlgo->m_dTeachWidth;
		double dTeachL = (bROT && bTeach == false) ? pAlgo->m_dTeachWidth : pAlgo->m_dTeachLength;
		double dTeachWMin = (bROT) ? pAlgo->m_dTeachLengthRateMin : pAlgo->m_dTeachWidthRateMin;
		double dTeachLMin = (bROT) ? pAlgo->m_dTeachWidthRateMin : pAlgo->m_dTeachLengthRateMin;
		double dTeachWMax = (bROT) ? pAlgo->m_dTeachLengthRateMax : pAlgo->m_dTeachWidthRateMax;
		double dTeachLMax = (bROT) ? pAlgo->m_dTeachWidthRateMax : pAlgo->m_dTeachLengthRateMax;
		int nShiftX = (dShiftX / dResolX) + 1;
		int nShiftY = (dShiftY / dResolY) + 1;
		int nBodyW = (dTeachW / dResolX);
		int nBodyWMin = dTeachW * dTeachWMin / 100.0 / dResolX;
		int nBodyWMax = dTeachW * dTeachWMax / 100.0 / dResolX;
		int nBodyH = (dTeachL / dResolY);
		int nBodyHMin = dTeachL * dTeachLMin / 100.0 / dResolY;
		int nBodyHMax = dTeachL * dTeachLMax / 100.0 / dResolY;
		CRect rcROI;
		rcROI.left = (rcArrBlob[0].left > rcArrBlob[1].left) ? rcArrBlob[1].left : rcArrBlob[0].left;
		rcROI.right = (rcArrBlob[0].right < rcArrBlob[1].right) ? rcArrBlob[1].right : rcArrBlob[0].right;
		rcROI.top = (rcArrBlob[0].top > rcArrBlob[1].top) ? rcArrBlob[1].top : rcArrBlob[0].top;
		rcROI.bottom = (rcArrBlob[0].bottom < rcArrBlob[1].bottom) ? rcArrBlob[1].bottom : rcArrBlob[0].bottom;

		if (rcROI.Width() < nBodyWMin || rcROI.Width() > nBodyWMax ||
			rcROI.Height() < nBodyHMin || rcROI.Height() > nBodyHMax)
			return -4;

		double dArea = dArrArea[0] + dArrArea[1];
		if (dArea > nBodyWMin * nBodyHMin && dArea < nBodyWMax * nBodyHMax)
		{
			double dArea1 = dArrArea[0] / dArea;
			double dArea2 = dArrArea[1] / dArea;
			if (dArea1 >= 0.7 || dArea2 >= 0.7)
				return -5;
		}

		double dCX = (dArrCX[0] + dArrCX[1]) / 2.0;
		double dCY = (dArrCY[0] + dArrCY[1]) / 2.0;
		CPoint poC = rcROI.CenterPoint();
		double dGapX = abs(poC.x - dCX);
		double dGapY = abs(poC.y - dCY);
		//if (dGapX > 3 || dGapY > 3)
		//	return -6;

		return 1;
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Mount::BODY_TIP_CHK(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
		}
		return -1;
	}
}
int CPInsp_Mount::BODY_TIP_RMV(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, cv::Mat imgBlob, int nW, int nH, int nCntBlob, CRect *rcArrBlob, int nMinBlob, float fSearchPer_BodyTip)
{
	int nLine = __LINE__;
	try
	{
		if (pAlgo == NULL || imgBlob.empty())
			return -2;
		if (nCntBlob < 2 || rcArrBlob == NULL)
			return -3;

		bool bUNIT_FIND = ((m_nBodyBlobOPT & m_eBodyBlobOPT_UNIT_FIND) == m_eBodyBlobOPT_UNIT_FIND);
		CMilBlobResult blobRes(nCntBlob);
		m_pProcMilAlgo->GetBlobResult(&blobRes);

		std::vector<cv::Mat> vArrImg;
		Im::PIL_ID milBlobResultId = m_pProcMilAlgo->GetBlobResultId();
		for (int a = 0; a < nCntBlob; a++)
		{
			cv::Mat imgTemp = cv::Mat(nH, nW, CV_8UC1, cv::Scalar(0));
			m_pProcMilAlgo->DrawBlob(milBlobResultId, (Im::PIL_ID)&imgTemp, blobRes.blobLabel[a]);
			vArrImg.push_back(imgTemp);
		}

		CRect rcPart;
		rcPart.left = (rcArrBlob[0].left < rcArrBlob[1].left) ? rcArrBlob[0].left : rcArrBlob[1].left;
		rcPart.right = (rcArrBlob[0].right > rcArrBlob[1].right) ? rcArrBlob[0].right : rcArrBlob[1].right;
		rcPart.top = (rcArrBlob[0].top < rcArrBlob[1].top) ? rcArrBlob[0].top : rcArrBlob[1].top;
		rcPart.bottom = (rcArrBlob[0].bottom > rcArrBlob[1].bottom) ? rcArrBlob[0].bottom : rcArrBlob[1].bottom;
		double dWPer1 = (double)rcArrBlob[0].Width() / (double)rcPart.Width() * 100.0;
		double dHPer1 = (double)rcArrBlob[0].Height() / (double)rcPart.Height() * 100.0;
		double dWPer2 = (double)rcArrBlob[1].Width() / (double)rcPart.Width() * 100.0;
		double dHPer2 = (double)rcArrBlob[1].Height() / (double)rcPart.Height() * 100.0;
		double dWGap = abs(dWPer1 - dWPer2);
		double dHGap = abs(dHPer1 - dHPer2);

		BOOL bHO = pAlgo->m_bIsHorizon;
		int nShiftX = pAlgo->m_dShiftX / m_resolX / 2.0;
		int nBodyWidth = (pAlgo->m_dTeachWidth / m_resolX);
		int nBodyWMin = pAlgo->m_dTeachWidth * pAlgo->m_dTeachWidthRateMin / 100.0 / m_resolX;
		int nBodyWMax = pAlgo->m_dTeachWidth * pAlgo->m_dTeachWidthRateMax / 100.0 / m_resolX;
		int nShiftY = pAlgo->m_dShiftY / m_resolY / 2.0;
		int nBodyHeight = (pAlgo->m_dTeachLength / m_resolY);
		int nBodyHMin = pAlgo->m_dTeachLength * pAlgo->m_dTeachLengthRateMin / 100.0 / m_resolY;
		int nBodyHMax = pAlgo->m_dTeachLength * pAlgo->m_dTeachLengthRateMax / 100.0 / m_resolY;
		if (bTeach == false && (dAngle == 90 || dAngle == 270))
		{
			nShiftX = pAlgo->m_dShiftY / m_resolY / 2.0;
			nBodyWidth = pAlgo->m_dTeachLength / m_resolY;
			nBodyWMin = pAlgo->m_dTeachLength * pAlgo->m_dTeachLengthRateMin / 100.0 / m_resolY;
			nBodyWMax = pAlgo->m_dTeachLength * pAlgo->m_dTeachLengthRateMax / 100.0 / m_resolY;

			nShiftY = pAlgo->m_dShiftX / m_resolX / 2.0;
			nBodyHeight = pAlgo->m_dTeachWidth / m_resolX;
			nBodyHMin = pAlgo->m_dTeachWidth * pAlgo->m_dTeachWidthRateMin / 100.0 / m_resolX;
			nBodyHMax = pAlgo->m_dTeachWidth * pAlgo->m_dTeachWidthRateMax / 100.0 / m_resolX;
		}

		std::vector<int> vArrRemove;
		cv::Mat imgBin(nH, nW, CV_8UC1, cv::Scalar(0));
		for (int a = 0; a < nCntBlob; a++)
		{
			bool bContinue = false;
			for (int nID : vArrRemove)
			{
				if (nID == a)
				{
					bContinue = true;
					break;
				}
			}
			if (bContinue) continue;

			for (int b = 0; b < nCntBlob; b++)
			{
				bContinue = false;
				for (int nID : vArrRemove)
				{
					if (nID == b)
					{
						bContinue = true;
						break;
					}
				}
				if (a == b || bContinue)
					continue;

				CRect rtDst;
				if (rtDst.IntersectRect(rcArrBlob[a], rcArrBlob[b]) == FALSE)
					continue;

				double dAreaDst = rtDst.Width() * rtDst.Height();
				double dAreaRst = rcArrBlob[b].Width() * rcArrBlob[b].Height();
				double dPer = dAreaDst / dAreaRst;
				if (dPer >= 0.8)
				{
					cv::bitwise_or(imgBin, vArrImg[b], imgBin);
					vArrRemove.push_back(b);
				}
			}
		}

		cv::Mat imgInsp(nH, nW, CV_8UC1, cv::Scalar(0));
		for (int a = 0; a < nCntBlob; a++)
		{
			bool bContinue = false;
			for (int nID : vArrRemove)
			{
				if (nID == a)
				{
					bContinue = true;
					break;
				}
			}
			if (bContinue) continue;

			if (bUNIT_FIND && dWGap < 10 && dHGap < 10)
			{
				int nL = 0;
				int nR = 0;
				int nT = 0;
				int nB = 0;
				int nC = 0;
				for (int b = 0; b <= 8; b++)
				{
					cv::Mat imgClone = vArrImg[a].clone();
					float fSearch = fSearchPer_BodyTip + (b * 5.0f);
					if (b > 4)
						fSearch = fSearchPer_BodyTip - ((b - 4) * 5.0f);
					BODY_TIP_RMV_DIV(pAlgo, dAngle, bTeach, imgClone, nW, nH, rcArrBlob[a], fSearch);

					std::vector<std::vector<cv::Point>> contours;
					cv::findContours(imgClone, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

					int nTotal = contours.size();
					int nTempL = -1;
					int nTempR = -1;
					int nTempT = -1;
					int nTempB = -1;
					int nTempW = -1;
					int nTempH = -1;
					double dGapTemp = 1;
					for (int c = 0; c < nTotal; c++)
					{
						int nCnt = contours[c].size();
						if (nCnt < 4)
							continue;
						int nTempL2 = -1;
						int nTempR2 = -1;
						int nTempT2 = -1;
						int nTempB2 = -1;
						for (int d = 0; d < nCnt; d++)
						{
							if (d == 0 || nTempL2 > contours[c][d].x) nTempL2 = contours[c][d].x;
							if (d == 0 || nTempR2 < contours[c][d].x) nTempR2 = contours[c][d].x;
							if (d == 0 || nTempT2 > contours[c][d].y) nTempT2 = contours[c][d].y;
							if (d == 0 || nTempB2 < contours[c][d].y) nTempB2 = contours[c][d].y;
						}
						int nTempW2 = nTempR2 - nTempL2;
						int nTempH2 = nTempB2 - nTempT2;
						if (nTempW2 <= 1 || nTempH2 <= 1)
							continue;

						if (bHO)
						{
							if (nBodyHMin > nTempH2 || nBodyHMax < nTempH2)
								continue;
						}
						else
						{
							if (nBodyWMin > nTempW2 || nBodyWMax < nTempW2)
								continue;
						}

						double dGap = bHO ? (double)nTempH2 / (double)nBodyHeight : (double)nTempW2 / (double)nBodyWidth;
						double dGapC = 1.0 - dGap;
						if (dGapC < 0) dGapC *= -1.0;
						bool bInput = (c == 0 || dGapTemp > dGapC);
						if (bInput == false && nTempW2 * nTempH2 > nTempW * nTempH)
							bInput = true;
						if (bInput)
						{
							nTempL = nTempL2;
							nTempR = nTempR2;
							nTempT = nTempT2;
							nTempB = nTempB2;
							nTempW = nTempR - nTempL;
							nTempH = nTempB - nTempT;
						}
					}
					nTempW = nTempR - nTempL;
					nTempH = nTempB - nTempT;
					if (nTempL < 0 || nTempR <= 0 || nTempT < 0 || nTempB <= 0 ||
						nTempW <= 1 || nTempH <= 1)
						continue;
					nL += nTempL;
					nR += nTempR;
					nT += nTempT;
					nB += nTempB;
					nC++;
				}
				if (nC <= 0)
					continue;

				int nAvgL = (int)(RounD((double)nL / (double)nC));
				int nAvgR = (int)(RounD((double)nR / (double)nC));
				int nAvgT = (int)(RounD((double)nT / (double)nC));
				int nAvgB = (int)(RounD((double)nB / (double)nC));
				int nAvgW = nAvgR - nAvgL + 1;
				int nAvgH = nAvgB - nAvgT + 1;
				if (nAvgL < 0 || nAvgR <= 0 || nAvgT < 0 || nAvgB <= 0 ||
					nAvgW <= 1 || nAvgH <= 1)
					continue;
				cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(0));
				imgTemp(cv::Rect(nAvgL, nAvgT, nAvgW, nAvgH)).setTo(255);
				cv::bitwise_and(vArrImg[a], imgTemp, imgTemp);
				cv::bitwise_or(imgTemp, imgInsp, imgInsp);
			}
			else
			{
			BODY_TIP_RMV_DIV(pAlgo, dAngle, bTeach, vArrImg[a], nW, nH, rcArrBlob[a], fSearchPer_BodyTip);
			cv::bitwise_or(vArrImg[a], imgInsp, imgInsp);
			}
		}

		cv::Mat imgTemp(nH, nW, CV_8UC1, cv::Scalar(255));
		cv::bitwise_xor(imgBin, imgTemp, imgBin);
		cv::bitwise_and(imgTemp, imgInsp, imgBlob);
		bool bFillHole = pAlgo->m_bFillHole;
		int nBlob = m_pProcMilAlgo->CalcBlob_Select(imgBlob.data, imgBlob.data, nW, nH, nMinBlob, false, bFillHole, 0, eSelectMix);
		return nBlob;
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Mount::BODY_TIP_RMV(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
		}
		return -1;
	}

}
void CPInsp_Mount::BODY_TIP_RMV_DIV(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, cv::Mat imgBlob, int nW, int nH, CRect rcROI, float fSearchPer_BodyTip)
{
	int nLine = __LINE__;
	try
	{
		if (pAlgo == NULL || imgBlob.empty())
			return;

		bool bROT = (dAngle == 90 || dAngle == 270);
		double dResolX = (bROT) ? m_resolY : m_resolX;
		double dResolY = (bROT) ? m_resolX : m_resolY;
		double dShiftX = (bROT) ? pAlgo->m_dShiftY : pAlgo->m_dShiftX;
		double dShiftY = (bROT) ? pAlgo->m_dShiftX : pAlgo->m_dShiftY;
		double dTeachW = (bROT && bTeach == false) ? pAlgo->m_dTeachLength : pAlgo->m_dTeachWidth;
		double dTeachL = (bROT && bTeach == false) ? pAlgo->m_dTeachWidth : pAlgo->m_dTeachLength;
		double dTeachWMin = (bROT) ? pAlgo->m_dTeachLengthRateMin : pAlgo->m_dTeachWidthRateMin;
		double dTeachLMin = (bROT) ? pAlgo->m_dTeachWidthRateMin : pAlgo->m_dTeachLengthRateMin;
		double dTeachWMax = (bROT) ? pAlgo->m_dTeachLengthRateMax : pAlgo->m_dTeachWidthRateMax;
		double dTeachLMax = (bROT) ? pAlgo->m_dTeachWidthRateMax : pAlgo->m_dTeachLengthRateMax;

		int nBodyW = (dTeachW / dResolX);
		int nBodyWMin = dTeachW * dTeachWMin / 100.0 / dResolX;
		int nBodyWMax = dTeachW * dTeachWMax / 100.0 / dResolX;
		int nBodyH = (dTeachL / dResolY);
		int nBodyHMin = dTeachL * dTeachLMin / 100.0 / dResolY;
		int nBodyHMax = dTeachL * dTeachLMax / 100.0 / dResolY;
		BOOL bHO = pAlgo->m_bIsHorizon;

		int nS = (bHO) ? rcROI.left : rcROI.top;
		int nE = (bHO) ? rcROI.right : rcROI.bottom;
		int nST = (bHO) ? rcROI.top : rcROI.left;
		int nET = (bHO) ? rcROI.bottom : rcROI.right;
		double dBody = (bHO) ? nBodyH : nBodyW;
		if (fSearchPer_BodyTip < 10) fSearchPer_BodyTip = 10;
		if (fSearchPer_BodyTip > 99) fSearchPer_BodyTip = 99;
		double dBodySearch = dBody * (fSearchPer_BodyTip / 100.0);
		for (int i = nS; i <= nE; i++)
		{
			int nTotal = 0;
			for (int j = nST; j <= nET; j++)
			{
				int nIndex = bHO ? (j * nW) + i : (i * nW) + j;
				if (imgBlob.data[nIndex] == 255)
					nTotal++;
			}
			if (nTotal >= dBodySearch)
				continue;

			for (int j = nST; j <= nET; j++)
			{
				int nIndex = bHO ? (j * nW) + i : (i * nW) + j;
				imgBlob.data[nIndex] = 0;
			}
		}
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Mount::BODY_TIP_RMV_DIV(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
		}
	}
}
void CPInsp_Mount::BODY_RMV_DIV(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, UCHAR* ucArrBlobDst, int nW, int nH, float fSearchPer)
{
	int nLine = __LINE__;
	try
	{
		if (pAlgo == NULL || ucArrBlobDst == NULL)
			return;

		if (fSearchPer < 10) fSearchPer = 10;
		if (fSearchPer > 99) fSearchPer = 99;
		double dSearchPerMax = (100.0 - fSearchPer) + 100.0;
		cv::Mat imgBlobDst(nH, nW, CV_8UC1, ucArrBlobDst);
		bool bROT = (dAngle == 90 || dAngle == 270);
		double dResolX = (bROT) ? m_resolY : m_resolX;
		double dResolY = (bROT) ? m_resolX : m_resolY;

		double dTeachW = (bROT && bTeach == false) ? pAlgo->m_dTeachLength : pAlgo->m_dTeachWidth;
		double dTeachWMin = (bROT) ? pAlgo->m_dTeachLengthRateMin : pAlgo->m_dTeachWidthRateMin;
		double dTeachWMax = (bROT) ? pAlgo->m_dTeachLengthRateMax : pAlgo->m_dTeachWidthRateMax;

		double dTeachL = (bROT && bTeach == false) ? pAlgo->m_dTeachWidth : pAlgo->m_dTeachLength;
		double dTeachLMin = (bROT) ? pAlgo->m_dTeachWidthRateMin : pAlgo->m_dTeachLengthRateMin;
		double dTeachLMax = (bROT) ? pAlgo->m_dTeachWidthRateMax : pAlgo->m_dTeachLengthRateMax;

		double dBodyW = (dTeachW / dResolX);
		double dBodyWMin = dBodyW * (dTeachWMin / 100.0) * (fSearchPer / 100.0);
		double dBodyWMax = dBodyW * (dTeachWMax / 100.0) * (dSearchPerMax / 100.0);

		double dBodyH = (dTeachL / dResolY);
		double dBodyHMin = dBodyH * (dTeachLMin / 100.0) * (fSearchPer / 100.0);
		double dBodyHMax = dBodyH * (dTeachLMax / 100.0) * (dSearchPerMax / 100.0);
		for (int y = 0; y < nH; y++)
		{
			int nS = nH;
			int nE = 0;
			int nTotal = 0;
			for (int x = 0; x < nW; x++)
			{
				int nIndex = (y * nW) + x;
				if (ucArrBlobDst[nIndex] == 255)
				{
					nTotal++;
					if (nS > x) nS = x;
					if (nE < x) nE = x;
				}
			}
			if (nTotal == 0)
				continue;
			if (nTotal >= dBodyWMin)
				continue;

			for (int x2 = nS; x2 <= nE; x2++)
			{
				int nIndex = (y * nW) + x2;
				ucArrBlobDst[nIndex] = 0;
			}
		}

		for (int x = 0; x < nW; x++)
		{
			int nS = nW;
			int nE = 0;
			int nTotal = 0;
			for (int y = 0; y < nH; y++)
			{
				int nIndex = (y * nW) + x;
				if (ucArrBlobDst[nIndex] == 255)
				{
					nTotal++;
					if (nS > y) nS = y;
					if (nE < y) nE = y;
				}
			}
			if (nTotal == 0)
				continue;
			if (nTotal >= dBodyHMin)
				continue;

			for (int y2 = nS; y2 <= nE; y2++)
			{
				int nIndex = (y2 * nW) + x;
				ucArrBlobDst[nIndex] = 0;
			}
		}
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Mount::BODY_TIP_RMV_DIV(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
		}
	}
}