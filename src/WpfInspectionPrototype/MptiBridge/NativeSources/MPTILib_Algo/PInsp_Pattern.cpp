
#include "StdAfx.h"
//#include "InspParamDef.h"
#include "PInsp_Pattern.h"
#include "MPTI.h"
#include "math.h"
#include "ProcPil_Pattern.h"
#include "ExtInspSub.h"


#define PATTERN_LOCK		1
#define ALL_SAVE			TRUE



CPInsp_Pattern::CPInsp_Pattern(void)
{
	m_className = _T("CPInsp_Pattern");

	m_patWndCnts = 0;	
	m_patWndCnts_old = 0;

	m_pInspParam = &m_paramInspect;
	m_pTeachParam = &m_paramTeach;
	m_pInspPartInfo = &m_InspPartInfo;

	m_fovImage_insp = M_NULL;
	m_fovImage_teach = M_NULL;
	m_fovImage_insp_color = M_NULL;
	m_Mat_fovImage_teach = NULL;
	m_TeachingMask = M_NULL;
	SetAllocFovImg(FALSE);

	m_fovWidth = 0;
	m_fovLength = 0;
	m_resolX = 0;
	m_resolY = 0;

	m_bUseImagePilLib = false;

	m_pAlgoPattern = NULL;

	m_nInspectionMode = eMI_WindowTeaching;

	InitPatternResultStruct(&m_rstInspPattern);
	InitPatternResultStruct(&m_rstInspSimPattern);
	ptrFileAlgoPath =NULL;
	m_rstSimilarScore = 0;

	m_procMil = NULL;
}

CPInsp_Pattern::~CPInsp_Pattern(void)
{
	if(IsAllocFovImg())
	{
		m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_teach);
		if(m_fovImage_insp_color)
			m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp_color);
		m_fovImage_insp_color = M_NULL;		
		if(m_fovImage_insp)
			m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp);
		m_fovImage_insp = M_NULL;		
	}

	if(ptrFileAlgoPath!=NULL){

		//delete[] ptrFileAlgoPath;
		g_pMManager->pem_delete(ptrFileAlgoPath, true);
		ptrFileAlgoPath = NULL;
	}
	CloseDevice();
}

int CPInsp_Pattern::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	// Allocate a Application.
	//m_milApp = milApp;
	// Allocate a System.
	//m_milSys = milSys;

	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	m_bUseImagePilLib = bUseImagePilLib;

	if(m_procMil == nullptr)
		//m_procMil = new CProcPil_Pattern();
		m_procMil = g_pMManager->pem_new<CProcPil_Pattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

	m_procMil->InitMil(milApp, milSys,bUseImagePilLib);
	m_procMil->SetResol(resolX, resolY, m_fovWidth);

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY, true);


	return ePART_SUCCESS;
}

int CPInsp_Pattern::CloseDevice()
{
	if(m_procMil != NULL)
	{
		m_procMil->FreeMil();
		
		//delete m_procMil;
		g_pMManager->pem_delete(m_procMil, false);
		m_procMil = NULL;
	}

	CPInsp::CloseDevice();

	return ePART_SUCCESS;	
}

int CPInsp_Pattern::SetInspParam(void* itemParam, void* targetImg, Coordinate cdn)
{
	m_pInspParam = (InspParamPattern*)itemParam;
	m_fovImage_insp = targetImg;
	m_inspCoordinate = cdn;

	return ePART_SUCCESS;
}

int CPInsp_Pattern::SetInspParam( const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, int nPartWidth, int nPartHeight, TotalInspExceptArea stTieArea)
{
	if(!m_pInspParam)
		return ePART_FAIL;
	if(sInspAlgo.m_eAlgoType != eAlgoPattern)
		return ePART_FAIL;
	m_pAlgoPattern = (AlgoPattern *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pAlgoPattern)
		return ePART_FAIL;
	
	int widthStep = sWndAlgoImg.m_nWidth * 3;

	int nSizeX = widthStep % 4;
	if (nSizeX != 0)
		widthStep += 4 - nSizeX;
	//widthStep = g_pMPTI->nCalcWidthStep(false, widthStep);

	stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
	//for (int i = 0; i < MAX_MASKING_NUM; i++)
	//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
	stTieArea.m_rcArrMaskingROI = sInspAlgo.m_rcArrMaskingROI;
	stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
	stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
	for(int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];
	
	//cv::Mat src(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_MAKETYPE(8,3)/*CV_8UC3*/, sWndAlgoImg.m_ucArrCV, widthStep); // LWW 2017/05/10
	cv::Mat src = cv::Mat::zeros(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC3);
	m_Mat_fovImage_teach = cv::Mat::zeros(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC3);

	for (int y=0; y<sWndAlgoImg.m_nHeight; y++)
		memcpy(src.ptr(y), &sWndAlgoImg.m_ucArrCV[y*(sWndAlgoImg.m_nWidth*3)], sWndAlgoImg.m_nWidth*3);

	// KIY 2020/04/23 : 폴리곤 통합
	CPInsp::FillOutOfInspAreaCombine(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, 0, sWndAlgoImg.m_ucArr2D, NULL, stTieArea);
	CPInsp::FillOutOfInspAreaCombine_Color(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, 0, src.ptr(0), m_Mat_fovImage_teach.ptr(0), stTieArea);

	m_pAlgoPattern->GetParam(m_pInspParam);

	m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp);
	m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp_color);
	
	src.copyTo(m_Mat_fovImage_teach);
	
	Im::PIL_ID milSrc_color = m_procMil->AllocBuffColor(src.cols, src.rows);
	int64 M_width = Im::Buf::Inquire(milSrc_color, M_SIZE_X, M_NULL);
	int64 M_height = Im::Buf::Inquire(milSrc_color, M_SIZE_Y, M_NULL);
	Im::Buf::PutColor2d(milSrc_color, M_PACKED + M_BGR24, M_ALL_BANDS,0,0, M_width, M_height, &m_Mat_fovImage_teach.data[0]);

	m_fovImage_insp_color = (void*)milSrc_color;
	m_fovImage_insp = (void*)m_procMil->GetMilSrc(sWndAlgoImg);
	SetAllocFovImg(TRUE);

	m_inspCoordinate.SetParamROI(coordinateAlgo);
	
	m_pInspPartInfo->m_dPartWidth = (double)nPartWidth;
	m_pInspPartInfo->m_dPartHeight = (double)nPartHeight;

	return ePART_SUCCESS;
}

int CPInsp_Pattern::SetTeachParam(void* itemParam, void* targetImg, int teachType, Coordinate cdn)
{
	m_pTeachParam = (TeachParamPattern*) itemParam;
	m_fovImage_teach = targetImg;
	m_teachCoordinate = cdn;

	return ePART_SUCCESS;
}

int CPInsp_Pattern::SetTeachParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, WndInfo &sInspPartInfo)
{
	if(!m_pTeachParam)
		return ePART_FAIL;
	if(sInspAlgo.m_eAlgoType != eAlgoPattern)
		return ePART_FAIL;
	m_pAlgoPattern = (AlgoPattern *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pAlgoPattern)
		return ePART_FAIL;

	int width = sWndAlgoImg.m_nWidth;
	int height = sWndAlgoImg.m_nHeight;
	if((width <= 0) || (height <= 0))
		return eMR_FAIL;

	int widthStep = width * sWndAlgoImg.m_nChannel;

	/*int nSizeX = widthStep % 4;
	if (nSizeX!=0)
		widthStep += 4 - nSizeX;*/
	widthStep = g_pMPTI->nCalcWidthStep(false, widthStep);

	cv::Mat src(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_MAKETYPE(CV_8U,sWndAlgoImg.m_nChannel)/*CV_8UC3*/, sWndAlgoImg.m_ucArrCV, widthStep);
	cv::Mat src_gray;
	
	if(sWndAlgoImg.m_nChannel == 3)
		cv::cvtColor(src, src_gray , cv::COLOR_BGR2GRAY);

	// 실제 동작 안함
	TotalInspExceptArea stTieArea;
	stTieArea.m_nUsedMaskingValue = sInspAlgo.m_nUsedMaskingValue;
	//for (int i = 0; i < MAX_MASKING_NUM; i++)
	//	stTieArea.m_rcArrMaskingROI[i] = sInspAlgo.m_rcArrMaskingROI[i];
	stTieArea.m_rcArrMaskingROI = sInspAlgo.m_rcArrMaskingROI;
	stTieArea.m_nUsedInspPolygon = sInspAlgo.m_nUsedInspPolygon;
	stTieArea.m_bConvetExceptROI = sInspAlgo.m_bConvetExceptROI;
	for(int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		stTieArea.m_ptArrInspPolygon[i] = sInspAlgo.m_ptArrInspPolygon[i];

	m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_TeachingMask);
	m_TeachingMask = (void*)m_procMil->AllocBuff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight,255);

	cv::Mat teachMask = cv::Mat(sWndAlgoImg.m_nHeight,sWndAlgoImg.m_nWidth,CV_8UC1);
	teachMask.setTo(255);

	// KIY 2020/04/23 : 폴리곤 통합
	CPInsp::FillOutOfInspAreaCombine(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, 0, teachMask.ptr(0), NULL, stTieArea);

	m_pAlgoPattern->GetParam(m_pTeachParam);
	
	m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_teach);

	src.copyTo(m_Mat_fovImage_teach);
	//m_Mat_fovImage_teach = src;

	if(sWndAlgoImg.m_nChannel == 3)
	{
		//m_fovImage_teach 에 src_gray 넣어야 함.
		Im::PIL_ID milSrc_1;

		milSrc_1 = m_procMil->AllocBuff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);
		
		int64 M_width = Im::Buf::Inquire(milSrc_1, M_SIZE_X, M_NULL);
		int64 M_height = Im::Buf::Inquire(milSrc_1, M_SIZE_Y, M_NULL);
		Im::Buf::Put2d(milSrc_1, 0,0, M_width, M_height, src_gray.data);

		m_fovImage_teach = (void*)milSrc_1;
	}
	else	m_fovImage_teach = (void*)m_procMil->GetMilSrc(sWndAlgoImg);

	Im::PIL_ID milMask;
	milMask = m_procMil->AllocBuff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);
	int64 Mask_width = Im::Buf::Inquire(milMask, M_SIZE_X, M_NULL);
	int64 Mask_height = Im::Buf::Inquire(milMask, M_SIZE_Y, M_NULL);
	Im::Buf::Put2d(milMask, 0,0, Mask_width, Mask_height, teachMask.data);

	Im::Buf::Put(milMask, teachMask.data);
	m_TeachingMask = (void*)milMask;

	SetAllocFovImg(TRUE);

	m_teachCoordinate.SetParamROI(coordinateAlgo);

	SetInspPartInfo(sInspPartInfo);

	return ePART_SUCCESS;
}

int CPInsp_Pattern::InspProc_Pattern(int nInspectionMode, int nOffX_pix, int nOffY_pix)
{
	int ret = e_OK;

	m_nInspectionMode = nInspectionMode;
	Img_Proc = std::make_shared<ImgProcessing>();
	ret = ProcAuto(nOffX_pix, nOffY_pix);
	Img_Proc->Free();

	return ret;
}

int CPInsp_Pattern::ProcAuto(int nOffX_pix, int nOffY_pix)
{
	int ret = e_OK;
	int stepID = 0;
	int maxStepCnts = ePStepID_COUNTS;

	//step1(ePStepID_PATTERN_SEARCH) 

	InitPatternResultStruct(&m_rstInspPattern);
	InitPatternResultStruct(&m_pInspParam->retInspPatternResult);


	stepID = 0;
	for(int j = 0; j < maxStepCnts; j++)
	{
		ret = ProcStep(stepID, nOffX_pix, nOffY_pix);
		if(ret != e_OK)
			break;

		stepID++;
	}


	return ret;
}

int CPInsp_Pattern::ProcStep(int stepID, int nOffX_pix, int nOffY_pix)
{
	int ret = e_OK;

	switch(stepID)
	{
	case ePStepID_PATTERN_SEARCH:
		{
			ret = SearchPattern(nOffX_pix, nOffY_pix);
		}
		break;
	case ePStepID_SIMIALR_PAT_SEARCH:
		{
			ret = SearchSimilarPattern();
		}
		break;
	}

	return ret;
}

int CPInsp_Pattern::SearchPattern(int nOffX_pix, int nOffY_pix)
{
	int ret = e_NG;

	if(!m_pInspParam)
		return ret;
	
	// LMJ 2014/06/02	: Searching 영역 Part ROI -> Window로 변경
	int searchCx = m_inspCoordinate.cx;//m_inspCoordinate.bdrCx;
	int searchCy = m_inspCoordinate.cy;//m_inspCoordinate.bdrCy;
	int searchWidth = m_inspCoordinate.width;//m_inspCoordinate.bdrWidth;
	int searchHeight = m_inspCoordinate.length;//m_inspCoordinate.bdrLength;
	int searchArea = searchWidth * searchHeight;
	double wndAngle = m_inspCoordinate.angle;
	if(IsAnyAngle(wndAngle))
		wndAngle = 0;		// 일반각 일 경우 0도로 돌려진 영상에서 찾기 때문에 0도로 처리

	if(wndAngle > 360)
		wndAngle -= 360;
	
	if (wndAngle < 0) // 그룹파트일경우 음각으로 들어와 예외처리로 빠져서 검사안하는 문제 생김
		wndAngle += 360;

	//double stdCogX = m_pInspParam->stdCogX;
	//double stdCogY = m_pInspParam->stdCogY;
	double stdCogX[CNT_PATTERN_PATH];
	double stdCogY[CNT_PATTERN_PATH];

	memcpy(stdCogX, m_pInspParam->stdCogX, sizeof(double) * CNT_PATTERN_PATH);
	memcpy(stdCogY, m_pInspParam->stdCogY, sizeof(double) * CNT_PATTERN_PATH);

	double posLimit = 5;//m_inspParam->searchAngleRange_Max;
	double negLimit = 5;//m_inspParam->searchAngleRange_Min;

	int bdCx = m_inspCoordinate.anyAngleCx;
	int bdCy = m_inspCoordinate.anyAngleCy;
	int bdSizeX = m_inspCoordinate.anyAngleWidth;
	int bdSizeY = m_inspCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;


	if(searchCx < 0 || searchCy < 0 || searchWidth < 0 || searchHeight < 0 || wndAngle < 0 || wndAngle > 360)
		return ret;

	int nSelIndex = 0;	// 높은 스코어의 Index
	int nCntSearch = 0;

	double angleTemp = 0.;
	CString strFullPath = _T("");

	BOOL bFound = FALSE;
	double dAngleRotate = 0.0;

	BOOL bUseAlgoPattern = (m_pAlgoPattern && (m_pAlgoPattern->m_nCntPatternPath > 0));
	
	int nCntPatternPath = (bUseAlgoPattern) ? m_pAlgoPattern->m_nCntPatternPath : 1;

	RstInspPattern OrgResult;
	InitPatternResultStruct(&OrgResult);

	bool bDecision(true);

	m_rstInspPattern.score = 0;
	m_rstInspPattern.angle = 0;
	m_rstInspPattern.offsetX = 0;		/// 140207
	m_rstInspPattern.offsetY = 0;		/// 140207
	for(int i = 0; i < nCntPatternPath; ++i)
	{

		double* GapLnC = m_pInspParam->GapLnC[i];
		double* GapLnR = m_pInspParam->GapLnR[i];
		double* DiviScore = m_pInspParam->DiviScore[i];
		BOOL DetailSearch = m_pInspParam->DetailSearch[i];

		int zeroIdxC = m_pInspParam->DiviLnC[i];
		int zeroIdxR = m_pInspParam->DiviLnR[i];;
		
		if(zeroIdxC != 0 && zeroIdxC != 1)
			std::sort(m_pInspParam->GapLnC[i] ,m_pInspParam->GapLnC[i]+zeroIdxC);

		if(zeroIdxR != 0 && zeroIdxR != 1)
			std::sort(m_pInspParam->GapLnR[i] ,m_pInspParam->GapLnR[i]+zeroIdxR);

		if(bUseAlgoPattern)
		{
			//멀티쓰레드안쓸때.
			strFullPath.Format(_T("%s"), m_pAlgoPattern->GetModelPath(i));
			//strFullPath = m_pAlgoPattern->m_sArrPathModelInspect[i];
		}
		else
			strFullPath = m_pInspParam->modelPath;
		if(strFullPath.GetLength() < 1)
			continue;

		int modelLoadRst = eMPAT_FAIL;
		
		GeoModel model;
// 		if (g_pMPTI->m_bPattern_ForcedCharDiv)
// 			m_pInspParam->UseCharacter = true;

		if (m_nInspectionMode == eMI_Inspection)
		{
			
			modelLoadRst = m_procMil->ModelLoad(strFullPath);
			CString strFullPath_tif;
			strFullPath_tif.Format(_T("%s.tif"), strFullPath);
			
			//cv::Mat Loadimg = cv::imread(std::string(CT2A(strFullPath_tif)),CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
			
			//m_procMil->SetnChannel(Loadimg.channels());


			if(modelLoadRst==eMPAT_AllocedInternally)
			{
				modelLoadRst = eMPAT_SUCCESS;
				m_procMil->SetAngleMode(posLimit, negLimit, 0.5, FALSE);
				m_procMil->SetSearchStartAngle(0);
			}
			bool isSetOK = m_procMil->SetParam_DivArea(m_pInspParam->UseCharacter, &m_pInspParam->DiviLnC[i], &m_pInspParam->DiviLnR[i], &GapLnC, &GapLnR, &DiviScore, m_pInspParam->DetailSearch[i], m_pInspParam->ModelRect[i]);
			if(isSetOK == false)
				modelLoadRst == eMPAT_FAIL;
		}
		else if (m_nInspectionMode == eMI_SimilarInsp)
		{
			modelLoadRst = m_procMil->CodeSearchModelLoad(strFullPath);
			if(modelLoadRst==eMPAT_AllocedInternally)
			{
				modelLoadRst = eMPAT_SUCCESS;
				m_procMil->SetAngleMode(posLimit, negLimit, 0.5, FALSE);
				m_procMil->SetSearchStartAngle(0);
			}
			m_procMil->GetParam_DivArea(m_pInspParam->UseCharacter, m_pInspParam->DiviLnC[i], m_pInspParam->DiviLnR[i], m_pInspParam->GapLnC[i], m_pInspParam->GapLnR[i], m_pInspParam->DiviScore[i], m_pInspParam->DetailSearch[i],CRect());

		}

		if (modelLoadRst == eMPAT_FAIL)
		{
			modelLoadRst = m_procMil->ModelFileLoad(strFullPath);
			CString strFullPath_tif;
			strFullPath_tif.Format(_T("%s.tif"), strFullPath);

			//cv::Mat Loadimg = cv::imread(std::string(CT2A(strFullPath_tif)),CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
			
			//m_procMil->SetnChannel(Loadimg.channels());

			if(modelLoadRst==eMPAT_AllocedInternally)
			{
				modelLoadRst = eMPAT_SUCCESS;
				m_procMil->SetAngleMode(posLimit, negLimit, 0.5, FALSE);
				m_procMil->SetSearchStartAngle(0);
			}
			bool isSetOK = m_procMil->SetParam_DivArea(m_pInspParam->UseCharacter, &m_pInspParam->DiviLnC[i], &m_pInspParam->DiviLnR[i], &GapLnC, &GapLnR, &DiviScore, m_pInspParam->DetailSearch[i], m_pInspParam->ModelRect[i]);
			if(isSetOK == false)
				return e_NG;
		}

		// [0] : 0도 검사 결과, [1] : 180도 검사 결과
		double score[2] = {0, 0};
		double angle[2] = {0, 0};
		double posX[2] = {0, 0};
		double posY[2] = {0, 0};
		double dTempAngle = 0.;
		double offsetX = 0.;
		double offsetY = 0.;
		nCntSearch = 0;
		dAngleRotate = 0.0;
		nSelIndex = 0;	// 높은 스코어의 Index
		if(modelLoadRst == eMPAT_SUCCESS)
		{
			//1차: 0도에서 검사.
			dAngleRotate = 0.0;
			angleTemp = dAngleRotate + wndAngle;

			m_procMil->ModelPreproc(angleTemp,m_pAlgoPattern->m_SamplingAngle);

		//m_procMil->GetParam_DivArea(m_pInspParam->UseCharacter, m_pInspParam->DiviLnC[0], m_pInspParam->DiviLnR[0], m_pInspParam->GapLnC[0], m_pInspParam->GapLnR[0], m_pInspParam->DiviScore[0], m_pInspParam->DetailSearch[0],CRect());
// 			if(angleTemp != 0)
// 			{	
// 				m_procMil->SetSearchStartAngle(angleTemp);	
// 				m_procMil->PreprocModel(m_fovImage_insp);
// 			}
			//searchCnt =	m_procMil->SearchPattern(m_fovImage_insp, searchCx, searchCy, searchWidth, searchHeight);
			//nCntSearch =	 m_procMil->SearchPattern(clipSearchImg, searchWidth, searchHeight);
			// Window Image (m_fovImage_insp)가 Search 영역이므로 Clip 할 필요가 없다.
			
			
 			int nchannel = m_procMil->GetnChannel();
			if(nchannel!=3)
			{
				nchannel = 1;
				m_procMil->SetnChannel(1);
			}
			//milSrc = (MIL_ID)m_fovImage_insp;

			m_procMil->ImgProcess(m_fovImage_insp, m_fovImage_insp_color, searchWidth/2, searchHeight/2, searchWidth, searchHeight,Img_Proc);
			nCntSearch = m_procMil->SearchPattern(m_fovImage_insp, m_fovImage_insp_color, searchWidth/2, searchHeight/2, searchWidth, searchHeight);
			
			double dPatSubScore[2][CNT_PATTERN_SCORE] = {{0,}, {0,}};
			if(nCntSearch > 0)
			{
				m_procMil->GetPatResult(&score[0], &angle[0], &posX[0], &posY[0]);
				m_procMil->GetPatSubScore(dPatSubScore[0], CNT_PATTERN_DIVISION_C + 1);
			}
			int nSearchCnt(1);
	//		if(score[0] < 85)	// YJS 2017/02/20 70%=>85%로 변경
			{
				nSearchCnt = 2;
				//2차: 1차에서 실패하거나, score 70% 미만이면 180도 에서 다시 검사.
				dAngleRotate = 180.0;
				angleTemp = dAngleRotate + wndAngle;
				if(angleTemp >= 360)
					angleTemp = angleTemp - 360;

				m_procMil->ModelPreproc(angleTemp,m_pAlgoPattern->m_SamplingAngle);
// 				m_procMil->SetSearchStartAngle(angleTemp);	
// 				m_procMil->PreprocModel(m_fovImage_insp);

				int nCntSearch2 = m_procMil->SearchPattern(m_fovImage_insp, m_fovImage_insp_color, searchWidth/2, searchHeight/2, searchWidth, searchHeight);
				if(nCntSearch == 0)
					nCntSearch = nCntSearch2;

				m_procMil->GetPatResult(&score[1], &angle[1], &posX[1], &posY[1]);

				double maxscore = score[0]> score[1]? score[0]: score[1];
				CString savepath;
				m_procMil->GetPatSubScore(dPatSubScore[1], CNT_PATTERN_DIVISION_C + 1);
			}

			if(score[0] < score[1])
				nSelIndex = 1;

			bool bUseChar = m_pInspParam->UseCharacter;

			if(m_procMil->IsDivInspAvailable())
			{
				bool bDecisionList[2] = {false, };
				int nDivNumList[2] = {0, };
				double dResultScoreList[2] ={0, };
				if (m_pInspParam->DiviLnC[i] <= 0 && m_pInspParam->DiviLnR[i] <= 0 && bUseChar)
				{
					int nLimit = m_pInspParam->DiviLnC[i] > m_pInspParam->DiviLnR[i] ? m_pInspParam->DiviLnC[i]+1 : m_pInspParam->DiviLnR[i]+1;
					for (int x = 0; x < nLimit; x++)
						m_pInspParam->DiviScore[i][x] = m_pInspParam->stdScore;
				}
				for(int j=0; j<nSearchCnt; j++)
				{
					if(m_pInspParam->DiviLnC[i]>0 || m_pInspParam->DiviLnR[i]>0 || bUseChar)
					{
						int nDivNum(-1), nDivNgNum(-1);
						double DivScore(HUGE_VAL), DivNGScore(HUGE_VAL);

						int DiviArC = m_pInspParam->DiviLnC[i] + 1;
						int DiviArR = m_pInspParam->DiviLnR[i] + 1;
						int step = CNT_PATTERN_DIVISION_C + 1;
						// 					for (int y=0; y<DiviArR; y++)
						{
							for (int x=0; x<DiviArC; x++)
							{
								if(DivScore > dPatSubScore[j][/*y*step+*/x])
								{
									nDivNum = /*y*step+*/x;
									DivScore = dPatSubScore[j][/*y*step+*/x];
								}
								if(dPatSubScore[j][/*y*step+*/x] < m_pInspParam->DiviScore[i][/*y*step+*/x])
								{
									if(DivNGScore > dPatSubScore[j][/*y*step+*/x])
									{
										nDivNgNum = /*y*step+*/x;
										DivNGScore = dPatSubScore[j][/*y*step+*/x];
									}
								}
							}
							for (int x=0; x<DiviArR; x++)
							{
								if(DivScore > dPatSubScore[j][/*y*step+*/x])
								{
									nDivNum = /*y*step+*/x;
									DivScore = dPatSubScore[j][/*y*step+*/x];
								}
								if(dPatSubScore[j][/*y*step+*/x] < m_pInspParam->DiviScore[i][/*y*step+*/x])
								{
									if(DivNGScore > dPatSubScore[j][/*y*step+*/x])
									{
										nDivNgNum = /*y*step+*/x;
										DivNGScore = dPatSubScore[j][/*y*step+*/x];
									}
								}
							}
						}

						if(DivNGScore != HUGE_VAL)
						{
							bDecisionList[j] = false;
							if (m_pInspParam->DiviLnC[i] > 0 || m_pInspParam->DiviLnR[i] > 0)
								nDivNumList[j] = nDivNgNum + 1;
							dResultScoreList[j] = DivNGScore;
							// 						bDecision = false;
							// 						m_rstInspPattern.m_nDivisionNum = nDivNgNum + 1;
							// 						m_rstInspPattern.score = DivNGScore;
						}
						else if(DivScore != HUGE_VAL)
						{
							bDecisionList[j] = true;
							if(m_pInspParam->DiviLnC[i] > 0 || m_pInspParam->DiviLnR[i] > 0)
								nDivNumList[j] = nDivNum + 1;
							dResultScoreList[j] = DivScore;
							// 						bDecision = true;
							// 						m_rstInspPattern.m_nDivisionNum = nDivNum + 1;
							// 						m_rstInspPattern.score = DivScore;
						}
						else
						{
							bDecisionList[j] = true;
							nDivNumList[j] = 0;
							dResultScoreList[j] = 100.0;
							// 						bDecision = true;
							// 						m_rstInspPattern.m_nDivisionNum = 0;
						}
					}
					else
					{
						if(score[j] < m_pInspParam->stdScore)
						{
							bDecisionList[j] = false;
						}
						else
							bDecisionList[j] = true;
						dResultScoreList[j] = score[j];
						nDivNumList[j] = 0;
						// 					if(score[nSelIndex] < m_pInspParam->stdScore)
						// 					{
						// 						bDecision = false;
						// 					}
						// 					else
						// 						bDecision = true;
						//					m_rstInspPattern.score = score[nSelIndex];
						// 					m_rstInspPattern.m_nDivisionNum = 0;
					}
				}


				int nSelIdx(0);
				if(nSearchCnt>=2 && dResultScoreList[0] < dResultScoreList[1])
				{
					nSelIdx = 1;
					if(m_pInspParam->isPolarity)
					{
						bDecisionList[nSelIdx] = false;
					}
				}

				memcpy(m_rstInspPattern.m_dDiviScore, dPatSubScore[nSelIdx], sizeof(double)*CNT_PATTERN_SCORE);
				bDecision = bDecisionList[nSelIdx];
				m_rstInspPattern.score = dResultScoreList[nSelIdx];
				m_rstInspPattern.m_nDivisionNum = nDivNumList[nSelIdx];

				dTempAngle = (angle[nSelIdx] - wndAngle);

				if(dTempAngle <= -360)
				{
					while(dTempAngle <= -360)	dTempAngle += 360;
				}
				else if(dTempAngle >= 360)
				{
					while(dTempAngle >= 360)	dTempAngle -= 360;
				}

				nSelIndex = nSelIdx;	// YJS 2017/06/09

				m_rstInspPattern.angle = dTempAngle;

				double dCogX_pixel = ((searchCx - (searchWidth / 2.0)) + posX[nSelIndex]) - nOffX_pix;
				double dCogY_pixel = ((searchCy - (searchHeight / 2.0)) + posY[nSelIndex]) - nOffY_pix;
				/////
				double dCenTeachX = stdCogX[i];
				double dCenTeachY = stdCogY[i];

				if ((wndAngle == 90) || (wndAngle == 180) || (wndAngle == 270))
				{
					m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, wndAngle,
						0, 0, &dCenTeachX, &dCenTeachY);
				}

				double dshiftX = (dCogX_pixel * m_resolX) - ((m_pInspPartInfo->m_dPartWidth / 2) * m_resolX);
				double dshiftY = ((m_pInspPartInfo->m_dPartHeight / 2) * m_resolY) - (dCogY_pixel * m_resolY);

				offsetX = dCenTeachX - dshiftX;
				offsetY = (dCenTeachY - dshiftY) * -1;

				m_rstInspPattern.m_nModelNum = i + 1;// YJS 2017/03/11
				m_rstInspPattern.m_ptModelPos.x = posX[nSelIndex];
				m_rstInspPattern.m_ptModelPos.y = posY[nSelIndex];
				SIZE sz = m_procMil->getModelSz();
				m_rstInspPattern.ModelWidth = sz.cx;
				m_rstInspPattern.ModelHeight = sz.cy;
				// 		m_rstInspPattern.cogY = cogY_board;
				m_rstInspPattern.offsetX = offsetX;		/// 140207
				m_rstInspPattern.offsetY = offsetY;		/// 140207
			}
			else	//shkim 2017.07.26
			{
				if(m_pInspParam->stdScore > m_rstInspPattern.score)
					bDecision=false;
				else
					bDecision=true;
			}

			


			if(nCntSearch >= 1)
			{
				bFound = TRUE;

				if(bDecision==false)
				{
					continue;
				}
				else{
					//offset NG
					if(PATTERN_LOCK && m_pInspParam->bUseOffset && (abs(m_rstInspPattern.offsetX) > m_pInspParam->offsetRange_x || abs(m_rstInspPattern.offsetY) > m_pInspParam->offsetRange_y))
					{
						m_rstInspPattern.ok = FALSE;
						if(abs(m_rstInspPattern.offsetX) > m_pInspParam->offsetRange_x)
							m_rstInspPattern.list.offsetXOk = FALSE;
						if(abs(m_rstInspPattern.offsetX) > m_pInspParam->offsetRange_y)
							m_rstInspPattern.list.offsetYOk = FALSE;

						if(m_rstInspPattern.list.offsetXOk == FALSE || m_rstInspPattern.list.offsetYOk == FALSE )
							continue;
					}
					break;
				}
			}
			else
			{
				continue;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////	

	if(bFound == TRUE)
	{
		m_rstInspPattern.ok = TRUE;
		m_rstInspPattern.list.findOK = TRUE;

		if(nSelIndex == 1)
			m_rstInspPattern.isReverse = TRUE;
		else 
			m_rstInspPattern.isReverse = FALSE;
	}
	else
	{
		m_rstInspPattern.ok = FALSE;
		m_rstInspPattern.list.findOK = FALSE;		
	}


	m_rstInspPattern.isInsp = TRUE;


	ret = DecisionPatternInsp(bDecision);

	if(m_pAlgoPattern->m_bUseNGOpt == TRUE)
	{
		if(ret == e_NG)
			ret = e_OK;
		else
			ret = e_NG;
	}

	//////////////////////////////////////////////////////////////////////////

//	delete clipSearchImg;

	return ret;
}

#ifdef false
int CPInsp_Pattern::SearchPattern(int mode)
{
	int ret = e_OK;

// 	BOOL enabled = m_inspParam->enabled;
// 	if(!enabled)
// 	{
// 		return ret;
// 	}

	int searchCx = m_inspCoordinate.bdrCx;
	int searchCy = m_inspCoordinate.bdrCy;
	int searchWidth = m_inspCoordinate.bdrWidth;
	int searchHeight = m_inspCoordinate.bdrLength;
	double wndAngle = m_inspCoordinate.angle;
	CString strPath = m_pInspParam->modelPath;
	
	double stdCogX = m_pInspParam->stdCogX;
	double stdCogY = m_pInspParam->stdCogY;

	double posLimit = m_pInspParam->searchAngleRange_Max;
	double negLimit = m_pInspParam->searchAngleRange_Min;

	if(searchCx < 0 || searchCy < 0 || searchWidth < 0 || searchHeight < 0 || wndAngle < 0 || wndAngle > 360)
		return e_NG;

	UCHAR* clipBuff = NULL;
	clipBuff = new UCHAR[searchWidth * searchHeight];
	m_procMil->GetClipBuff(m_fovImage_insp, clipBuff, searchCx, searchCy, searchWidth, searchHeight);

	int w = 0;
	int h = 0;
	double rotateAngle = GetRotateAngle(searchWidth, searchHeight, wndAngle, &w, &h);

	UCHAR* rotateBuff = NULL;
	rotateBuff = new UCHAR[w * h];
	m_procMil->RotateImg(clipBuff, searchWidth, searchHeight, rotateAngle, rotateBuff);


	int searchCnt = 0;
	
	double angleTemp = 0.;
	CString strFullPath = _T("");
	
	strFullPath = strPath;
	int modelLoadRst = TRUE;
	if(!mode) //inspection mode
	{	
		modelLoadRst = m_procMil->ModelLoad(strPath);
	}

	if(modelLoadRst == eMPAT_SUCCESS)
	{
		//1차: 0도에서 검사.
		angleTemp = 0;
		m_procMil->SetAngleMode(posLimit, negLimit, 0.5);
 		m_procMil->SetSearchStartAngle(angleTemp);	
		searchCnt =	m_procMil->SearchPattern(rotateBuff, w, h);

		if(searchCnt < 1)
		{
			//2차: 1차에서 실패 할 경우 180도 에서 다시 검사.
			angleTemp = 180;
			m_procMil->SetSearchStartAngle(angleTemp);		
			searchCnt =	m_procMil->SearchPattern(rotateBuff, w, h);
		}
	}
	
	
	double score = 0;
	double angle = 0;
	double posX = 0;
	double posY = 0;

	BOOL isFound = FALSE;
	if(searchCnt > 0)
	{	
		m_procMil->GetPatResult(&score, &angle, &posX, &posY);
		//angle += angleTemp;
	

		double cogX_pixel = (searchCx - (w / 2.0)) + posX;
		double cogY_pixel = (searchCy - (h / 2.0)) + posY;

		double cogX_board = 0.0;
		double cogY_board = 0.0;
		CvtPixelToBoard(m_inspCoordinate.fovCx, m_inspCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);
		
		double offsetX = stdCogX - cogX_board;
		double offsetY = stdCogY - cogY_board;

		m_rstInspPattern.score = score;
		m_rstInspPattern.angle = angle;	
		m_rstInspPattern.cogX = cogX_board;
		m_rstInspPattern.cogY = cogY_board;
		m_rstInspPattern.offsetX = offsetX;
		m_rstInspPattern.offsetY = offsetY;
		

		isFound = TRUE;	
	}

	
//////////////////////////////////////////////////////////////////////////	

	if(isFound == TRUE)
	{
		m_rstInspPattern.ok = TRUE;
		m_rstInspPattern.list.findOK = TRUE;

		if(angleTemp == 180)
			m_rstInspPattern.isReverse = TRUE;
		else 
			m_rstInspPattern.isReverse = FALSE;
	}
	else
	{
		m_rstInspPattern.ok = FALSE;
		m_rstInspPattern.list.findOK = FALSE;		
	}
	

	m_rstInspPattern.isInsp = TRUE;


	ret = DecisionPatternInsp();

	//////////////////////////////////////////////////////////////////////////
	delete clipBuff;
	delete rotateBuff;

	return ret;
}
#endif

int CPInsp_Pattern::DecisionPatternInsp(bool bDecision)
{
	int ret = e_OK;

	double stdScore = m_pInspParam->stdScore;
	double curScroe = m_rstInspPattern.score;
	double angleRange = m_pInspParam->angleRange;
	double curAngle = m_rstInspPattern.angle;
	double offsetRange_x = m_pInspParam->offsetRange_x;
	double offsetRange_y = m_pInspParam->offsetRange_y;
	double curOffSetX = m_rstInspPattern.offsetX;
	double curOffSetY = m_rstInspPattern.offsetY;	

	if(m_inspCoordinate.angle == 90 || m_inspCoordinate.angle == 270)
	{
		double tmp = offsetRange_x;
		offsetRange_x = offsetRange_y;
		offsetRange_y = tmp;
	}

	BOOL isPolarity = m_pInspParam->isPolarity;
	if(m_rstInspPattern.list.findOK == TRUE) //찾았을 경우
	{
		ret = TRUE;


		//score NG
		if(bDecision == false) 
		{
			ret = e_NG;	
			m_rstInspPattern.ok = FALSE;

			m_rstInspPattern.list.scoreOk = FALSE;
		}

// 		if(curScroe < stdScore) 
// 		{
// 			ret = e_NG;	
// 			m_rstInspPattern.ok = FALSE;
// 
// 			m_rstInspPattern.list.scoreOk = FALSE;
// 		}
// 		else
// 		{
// 			m_rstInspPattern.list.scoreOk = TRUE;
// 		}
// 
// 		if (m_rstInspPattern.m_nModelNum > 0)
// 		{
// 			if ((m_pInspParam->DiviLnC[m_rstInspPattern.m_nModelNum - 1] > 0 || m_pInspParam->DiviLnR[m_rstInspPattern.m_nModelNum - 1] > 0))
// 			{
// 				if (m_rstInspPattern.m_nDivisionNum > 0)
// 				{
// 					ret = e_NG;
// 					m_rstInspPattern.list.scoreOk = FALSE;
// 				}
// 				else
// 				{
// 					ret = e_OK;
// 					m_rstInspPattern.list.scoreOk = TRUE;
// 				}
// 			}
// 		}


		//////////////////////////////////////////////////////////////////////////
		//angle NG

		if(curAngle < 0)
			curAngle = 360 + curAngle;


		double tempAngle[2] = {0.0, 180.0};

		int repeat = 2;

		if(isPolarity == TRUE)
			repeat = 1;

		BOOL bAngleOk = FALSE;
		for(int i = 0; i < repeat; i++)
		{
			double a1 = 0;
			double a2 = 0;
			double b1 = 0;
			double b2 = 0;


			a1 = tempAngle[i];
			a2 = tempAngle[i] + angleRange;
			if(a2 > 360)
				a2 -= 360;
			b1 = tempAngle[i] + 360;
			if(b1 > 360)
				b1 -= 360;
			b2 = (tempAngle[i] + 360) - angleRange;
			if(b2 > 360)
				b2 -= 360;

			if((curAngle >= a1 && curAngle <= a2) || (curAngle <= b1 && curAngle >= b2))
				bAngleOk = TRUE;
		}

	
		if(bAngleOk) 
		{
			m_rstInspPattern.list.angleOk = TRUE;				
		}
		else
		{
			ret = e_NG;
			m_rstInspPattern.ok = FALSE;
			m_rstInspPattern.list.angleOk = FALSE;			
		}
		//////////////////////////////////////////////////////////////////////////
		//offset NG
		if(PATTERN_LOCK && m_pInspParam->bUseOffset && (abs(curOffSetX) > offsetRange_x || abs(curOffSetY) > offsetRange_y))
		{
			ret = e_NG;
			m_rstInspPattern.ok = FALSE;
			if(abs(curOffSetX) > offsetRange_x)
				m_rstInspPattern.list.offsetXOk = FALSE;
			if(abs(curOffSetY) > offsetRange_y)
				m_rstInspPattern.list.offsetYOk = FALSE;
		}
		else
		{
			m_rstInspPattern.list.offsetXOk = TRUE;
			m_rstInspPattern.list.offsetYOk = TRUE;
		}
	}
	else//못찾았을 경우
	{
		ret = e_NG;
		m_rstInspPattern.ok = FALSE;

		m_rstInspPattern.list.findOK = FALSE;
		m_rstInspPattern.list.angleOk = FALSE;
		m_rstInspPattern.list.offsetXOk = FALSE;
		m_rstInspPattern.list.offsetYOk = FALSE;
		m_rstInspPattern.list.scoreOk = FALSE;
	}

//////////////////////////////////////////////////////////////////////////
	if(ret == e_NG)
	{
		m_rstInspPattern.ok = FALSE;

		if(isPolarity == TRUE)
		{
			if(m_rstInspPattern.list.findOK == FALSE || m_rstInspPattern.list.scoreOk == FALSE || m_rstInspPattern.list.angleOk == FALSE)
				m_rstInspPattern.list.polarityOK = FALSE;
			else
				m_rstInspPattern.list.polarityOK = TRUE;

		}
		else 
		{
			m_rstInspPattern.list.polarityOK = TRUE;	
		}
	}
	else
	{
		if(isPolarity == TRUE && m_rstInspPattern.isReverse == TRUE)
			m_rstInspPattern.list.polarityOK = FALSE;
		else 
			m_rstInspPattern.list.polarityOK = TRUE;
	}

	if(m_pAlgoPattern->m_bUseNGOpt == TRUE)
	{
		if(m_rstInspPattern.ok == FALSE)
			m_rstInspPattern.ok = TRUE;
		else
			m_rstInspPattern.ok = FALSE;

		if(m_rstInspPattern.list.scoreOk == FALSE)
			m_rstInspPattern.list.scoreOk = TRUE;
		else
			m_rstInspPattern.list.scoreOk = FALSE;
	}

	return ret;
}

void CPInsp_Pattern::GetPatternImage(WndAlgoImg &sWndAlgoImg, std::vector<UCHAR> & vMaskImg,int ModelIndex,const InspAlgo &sInspAlgo, int angle)
{

	CString strFullPath = _T("");
	AlgoPattern * pAlgo = NULL;
	if(ModelIndex >=0)
	{
		pAlgo = (AlgoPattern*)sInspAlgo.m_ptrInspAlgoParam;
		strFullPath.Format(_T("%s"), pAlgo->GetModelPath(ModelIndex));
	}
	int ret = eMPAT_SUCCESS;
	if(!strFullPath.IsEmpty() && strFullPath.MakeLower() != _T("(null)"))
	{
		ret = m_procMil->ModelFileLoad(strFullPath);
		m_procMil->ModelPreproc(angle,0);
	}
	vMaskImg.resize(sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight * sWndAlgoImg.m_nChannel);


	if(pAlgo==NULL || ret == eMPAT_FAIL)
		return;
	double* GapLnC = pAlgo->m_dGapLnC[0];
	double* GapLnR = pAlgo->m_dGapLnR[0];
	double* DiviScore = pAlgo->m_dDiviScore[0];
	//BOOL DetailSearch = m_pInspParam->DetailSearch[i];
	bool isSetOK = m_procMil->SetParam_DivArea(pAlgo->m_bUseCharacter, &pAlgo->m_nDiviLnC[0], &pAlgo->m_nDiviLnR[0], &GapLnC, &GapLnR, &DiviScore, pAlgo->m_bDetailSearch[0], pAlgo->m_rtModelRect[0]);
	m_procMil->SearchPattern_Draw(sWndAlgoImg.m_nChannel == 3 ? sWndAlgoImg.m_ucArrCV : sWndAlgoImg.m_ucArr2D , sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, &vMaskImg[0],sWndAlgoImg.m_nChannel);
// 	vMaskImg.resize(sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight * sWndAlgoImg.m_nChannel);
// 	m_procMil->SearchPattern_Draw(sWndAlgoImg.m_nChannel == 3 ? sWndAlgoImg.m_ucArrCV : sWndAlgoImg.m_ucArr2D , sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, &vMaskImg[0]);
}

int CPInsp_Pattern::Pattern_Teach(UCHAR* retImg, int sizeX, int sizeY, int use_algo, int nLight_Number, bool refPart) 
{
	int ret  = eMPAT_SUCCESS;

	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int width = m_teachCoordinate.width;
	int height = m_teachCoordinate.length;
	int roiArea;

	if(use_algo == 5) roiArea = width * height * 3;
	else roiArea = width * height;
		
	int searchCx = m_teachCoordinate.bdrCx;
	int searchCy = m_teachCoordinate.bdrCy;
	int searchWidth = m_teachCoordinate.bdrWidth;
	int searchHeight = m_teachCoordinate.bdrLength;
	int searchArea;

	if(use_algo == 5) searchArea = searchWidth * searchHeight *3;
	else searchArea = searchWidth * searchHeight;

	double wndAngle = m_teachCoordinate.angle;
	CString strPath = m_pTeachParam->modelPath;

	double posLimit = m_pTeachParam->SamplingAngle;// m_pTeachParam->searchAngleRange_Max;
	double negLimit = m_pTeachParam->SamplingAngle;// m_pTeachParam->searchAngleRange_Min;

	int bdCx = m_teachCoordinate.anyAngleCx;
	int bdCy = m_teachCoordinate.anyAngleCy;
	int bdSizeX = m_teachCoordinate.anyAngleWidth;
	int bdSizeY = m_teachCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	int nSX = 0;
	int nSY = 0;

	if(cx < 0 || cy < 0 || width < 0 || height < 0 || searchCx < 0 || searchCy < 0 || searchWidth < 0 || searchHeight < 0)
		return eMPAT_FAIL;

	int widthstep;
	
	int nChannel =1 ;
	if(use_algo == 5 && m_Mat_fovImage_teach.channels() == 3)
		nChannel = 3;
	else
		nChannel = 1;
	
	//////////////////////////////////////////////////////////////////////////
 	BOOL isAnyAngle = IsAnyAngle(wndAngle);
	
// 	if(isAnyAngle)
// 	{
// 		double rotAngle = CalcRotateAngle(wndAngle);
// 
// 		int w = 0;
// 		int l = 0;
// 		UCHAR* bdryImgTemp = NULL;
// 		m_procMil->ProcAnyAngle_Img(m_fovImage_teach, &bdryImgTemp, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
// 		m_procMil->SaveWorkImg(bdryImgTemp, w, l, _T("PATTERN_boundaryImg.bmp"));
// 
// 		wndAngle = 0;
// 		searchCx -= (int)RounD((double)bdCx - ((double)w / 2.0));
// 		searchCy -= (int)RounD((double)bdCy - ((double)l / 2.0));
// 
// 		nSX = RounD((double)searchCx - ((double)searchWidth / 2.0));
// 		nSY = RounD((double)searchCy - ((double)searchHeight / 2.0));
// 		if(nSX < 0)	nSX = 0;
// 		if(nSY < 0)	nSY = 0;
// 
// 		clipSearchImg = new UCHAR[searchArea];
// 		m_procMil->GetClipImage_LT(bdryImgTemp, w, l, clipSearchImg, nSX, nSY, searchWidth, searchHeight);
// 		m_procMil->SaveWorkImg(clipSearchImg, searchWidth, searchHeight, _T("PATTERN_clipSearchImg.bmp"));
// 
// 		cx -= (int)RounD((double)bdCx - ((double)w / 2.0));
// 		cy -= (int)RounD((double)bdCy - ((double)l / 2.0));
// 		nSX = RounD((double)cx - ((double)width / 2.0));
// 		nSY = RounD((double)cy - ((double)height / 2.0));
// 		if(nSX < 0)	nSX = 0;
// 		if(nSY < 0)	nSY = 0;
// 
// 		clipModelImg = new UCHAR[roiArea];
// 		m_procMil->GetClipImage_LT(bdryImgTemp, w, l, clipModelImg, nSX, nSY, width, height);
// 		m_procMil->SaveWorkImg(clipModelImg, width, height, _T("MPATTERN_clipModelImg.bmp"));
// 
// 		delete bdryImgTemp;
// 	}
// 	else
	/*UCHAR* clipSearchImg = new UCHAR[searchArea];
	UCHAR* clipModelImg = new UCHAR[roiArea];*/
	UCHAR* clipSearchImg = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipModelImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* clipModelMaskImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	{
		nSX = RounD((double)searchCx - ((double)searchWidth / 2.0));
		nSY = RounD((double)searchCy - ((double)searchHeight / 2.0));
		if(nSX < 0)	nSX = 0;
		if(nSY < 0)	nSY = 0;

		if(nChannel == 3)
		{
			cv::Rect rect(nSX, nSY, searchWidth, searchHeight);
			cv::Mat imageROI = m_Mat_fovImage_teach(rect);

			int nwidth = imageROI.cols;
			int nheight = imageROI.rows;

			for(int y=0; y< nheight ; y++)
				memcpy(&clipSearchImg[y*(nwidth * 3)], &imageROI.data[y*imageROI.step], nwidth * 3);
			m_procMil->SaveWorkImg(&clipSearchImg[0], nwidth, height, _T("MPATTERN_clipSearchImg.bmp"),nChannel);

		}
		else m_procMil->GetClipBuff_LT(m_fovImage_teach, clipSearchImg, nSX, nSY, searchWidth, searchHeight);

		nSX = RounD((double)cx - ((double)width / 2.0));
		nSY = RounD((double)cy - ((double)height / 2.0));
		if(nSX < 0)	nSX = 0;
		if(nSY < 0)	nSY = 0;

		m_procMil->GetnStep(width);

		if(nChannel == 3)
		{
			cv::Rect rect(nSX, nSY, width, height);
			cv::Mat imageROI = m_Mat_fovImage_teach(rect);

			width = imageROI.cols;
			height = imageROI.rows;
			for(int y=0; y< height ; y++)
				memcpy(&clipModelImg[y*(width * 3)], &imageROI.data[y*imageROI.step], width * 3);
		}
		else m_procMil->GetClipBuff_LT(m_fovImage_teach, clipModelImg, nSX, nSY, width, height);
		if(wndAngle==0)
			m_procMil->GetClipBuff_LT(m_TeachingMask, clipModelMaskImg, nSX, nSY, width, height);
		else
		{
			Im::PIL_ID rotMask_PILTmp = m_procMil->AllocNomalRotateBuff(((Im::PIL_ID)m_TeachingMask), 180);
			void* rotMask_voidTmp = (void*)rotMask_PILTmp;
			m_procMil->GetClipBuff_LT(rotMask_voidTmp, clipModelMaskImg, nSX, nSY, width, height);
			m_procMil->SaveWorkImg(&clipModelMaskImg[0], width, height, _T("MPATTERN_clipModelImgMaskRot.bmp"),nChannel);
			m_procMil->RotateImg(clipModelMaskImg,width, height,180,clipModelMaskImg, &width, &height);
		}

		m_procMil->SaveWorkImg(&clipModelImg[0], width, height, _T("MPATTERN_clipModelImg.bmp"),nChannel);
		m_procMil->SaveWorkImg(&clipModelMaskImg[0], width, height, _T("MPATTERN_clipModelImgMask.bmp"),nChannel);
	}

	//////////////////////////////////////////////////////////////////////////
	//UCHAR* imgTemp= new UCHAR[searchArea];
	UCHAR* imgTemp = g_pMManager->pem_new<UCHAR>(true, searchArea, (PCHAR)__FUNCTION__, __LINE__);

	if(wndAngle > 360)
		wndAngle -= 360;

	int w = 0;
	int h = 0;
	double rotateAngle = 0;
	if(wndAngle > 0 && !isAnyAngle)		// 일반각일 경우 0도로 처리
		rotateAngle	= GetRotateAngle(width, height, wndAngle, &w, &h);

	//////////////////////////////////////////////////////////////////////////
	//aloc model
	BOOL rotMode = (rotateAngle == 0.0)?    FALSE : TRUE;

	m_procMil->SetnChannel(nChannel == 3?3:1);

	int allocRst = m_procMil->AllocPatModel(clipModelImg, width, height, rotateAngle, w, h);
	float fAngleStp = 0.5f;
	if (posLimit + negLimit > 10)
	{
		fAngleStp = 1;
	}
	else if (posLimit + negLimit > 20)
	{
		fAngleStp = 2;
	}
	else if(posLimit + negLimit == 0)
	{
		rotateAngle = false;
		posLimit = negLimit = 1;
	}
	m_procMil->SetAngleMode(posLimit, negLimit, fAngleStp, rotMode, use_algo != 5);
	m_procMil->SetSearchStartAngle(0);
	m_procMil->SetFindSubPixel(rotMode,refPart);
	m_procMil->AllocModePatlexceptMask(clipModelMaskImg, width, height, rotateAngle, w, h);
	int nfilter = m_pAlgoPattern->m_nModelFilter;
	m_procMil->setModelFilterType(nfilter);
	m_procMil->PreprocModel(m_fovImage_teach, rotMode, use_algo, nLight_Number);
 	//////////////////////////////////////////////////////////////////////////

	//search
	int searchCnt = 0;
	double score = 0;
	double angle = 0;
	double posX = 0;
	double posY = 0;
	if(allocRst == eMPAT_SUCCESS)
	{
		bool isSetOK = m_procMil->SetParam_DivArea(false, 0, 0, 0, 0, 0, 0, RECT());
		Img_Proc = std::make_shared<ImgProcessing>();
		m_procMil->ImgProcess(clipSearchImg, searchWidth, searchHeight,Img_Proc);
		searchCnt = m_procMil->SearchPattern(clipSearchImg, searchWidth, searchHeight);

		if(searchCnt > 0)	
		{	
			m_procMil->GetPatResult(&score, &angle, &posX, &posY);
			double dX = posX + (searchCx - (searchWidth / 2.0));
			double dY = posY + (searchCy - (searchHeight / 2.0));
			
			double dCogX_pixel = m_pInspPartInfo->dCenterX - m_pInspPartInfo->dWidth / 2 + dX;
			double dCogY_pixel = m_pInspPartInfo->dCenterY - m_pInspPartInfo->dLength / 2 + dY;

			double dshiftX = (dCogX_pixel * m_resolX) - ((m_pInspPartInfo->m_dPartWidth / 2) * m_resolX);
			double dshiftY = ((m_pInspPartInfo->m_dPartHeight / 2) * m_resolY) - (dCogY_pixel * m_resolY);

			if((m_pInspPartInfo->dAngle == 90) || (m_pInspPartInfo->dAngle == 180) || (m_pInspPartInfo->dAngle == 270))
			{
				m_proc3d.CorrectCoordinate(dshiftX, dshiftY, 0, 0, -m_pInspPartInfo->dAngle,
					0, 0, &dshiftX, &dshiftY);
			}

// 			double cogX_board = 0.0;
// 			double cogY_board = 0.0;
			//CvtPixelToBoard(m_teachCoordinate.fovCx, m_teachCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);

			m_pTeachParam->retScore = score;
			m_pTeachParam->retAngle = angle;
			m_pTeachParam->retPosX =  posX;// - (searchCx - (searchWidth / 2.0));
			m_pTeachParam->retPosY =  posY;// - (searchCy - (searchHeight / 2.0));
			//m_pTeachParam->retCogX = cogX_board * m_resolX;		// 141007
			//m_pTeachParam->retCogY = cogY_board * m_resolY;		// 141007		
			m_pAlgoPattern->m_dRetCogX = dshiftX;
			m_pAlgoPattern->m_dRetCogY = dshiftY;
			if(m_pAlgoPattern)
				m_pAlgoPattern->SetParam(m_pTeachParam);
			//////////////////////////////////////////////////////////////////////////  display image
			if(retImg)
			{
				m_procMil->GetResultImg(clipSearchImg, searchWidth, searchHeight, imgTemp);

				double factor = m_procMil->GetResizeFactor(sizeX, sizeY, searchWidth, searchHeight);
				m_procMil->ResizeImg(imgTemp, retImg, searchWidth, searchHeight, sizeX, sizeY, 3, factor);
			}
			//////////////////////////////////////////////////////////////////////////
		}
		Img_Proc->Free();
	}
	if(clipSearchImg) //delete [] clipSearchImg;
		g_pMManager->pem_delete(clipSearchImg, true);
	if(clipModelImg) //delete [] clipModelImg;
		g_pMManager->pem_delete(clipModelImg, true);
 	if(imgTemp) //delete [] imgTemp;
		g_pMManager->pem_delete(imgTemp, true);
	if(clipModelMaskImg) //delete [] clipModelMaskImg;
		g_pMManager->pem_delete(clipModelMaskImg, true);
	
	return ret;
}

#ifdef false
int CPInsp_Pattern::Pattern_Teach(UCHAR* retImg, int sizeX, int sizeY)
{
	int ret  = eMPAT_SUCCESS;

	DWORD st, ed;
	st = GetTickCount();

	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int width = m_teachCoordinate.width;
	int height = m_teachCoordinate.length;
	int searchCx = m_teachCoordinate.bdrCx;
	int searchCy = m_teachCoordinate.bdrCy;
	int searchWidth = m_teachCoordinate.bdrWidth;
	int searchHeight = m_teachCoordinate.bdrLength;
	double wndAngle = m_teachCoordinate.angle;
	CString strPath = m_pTeachParam->modelPath;
	
	if(cx < 0 || cy < 0 || width < 0 || height < 0 || searchCx < 0 || searchCy < 0 || searchWidth < 0 || searchHeight < 0)
		return eMPAT_FAIL;

	UCHAR* clipSearchImg = NULL;
	clipSearchImg = new UCHAR[searchWidth * searchHeight];
	m_procMil->GetClipBuff(m_fovImage_teach, clipSearchImg, searchCx, searchCy, searchWidth, searchHeight);

	UCHAR* clipModelImg = NULL;
	clipModelImg = new UCHAR[width * height];
	m_procMil->GetClipBuff(m_fovImage_teach, clipModelImg, cx, cy, width, height);

	UCHAR* imgTemp = NULL;
	imgTemp = new UCHAR[searchWidth * searchHeight * 3];

	int w = 0;
	int h = 0;
	double rotateAngle = 0;
	if(wndAngle > 0)
		rotateAngle	= GetRotateAngle(width, height, wndAngle, &w, &h);

	//int allocRst = m_procMil->AllocPatModel(m_fovImage_teach, cx, cy, width, height);
	int allocRst = m_procMil->AllocPatModel(clipModelImg, width, height, rotateAngle, w, h);

	int searchCnt = 0;
	double score = 0;
	double angle = 0;
	double posX = 0;
	double posY = 0;
	if(allocRst == eMPAT_SUCCESS)
	{
		//searchCnt = m_procMil->SearchPattern(clipSearchImg, searchWidth, searchHeight);
		searchCnt = m_procMil->SearchPattern(m_fovImage_teach, searchCx, searchCy, searchWidth, searchHeight);

		if(searchCnt > 0)	
		{	
			m_procMil->GetPatResult(&score, &angle, &posX, &posY);

			//double cogX_pixel = (searchCx - (searchWidth / 2.0)) + posX;
			//double cogY_pixel = (searchCy - (searchHeight / 2.0)) + posY;
			double cogX_pixel = posX;
			double cogY_pixel = posY;

			double cogX_board = 0.0;
			double cogY_board = 0.0;
			CvtPixelToBoard(m_teachCoordinate.fovCx, m_teachCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);

			m_pTeachParam->retScore = score;
			m_pTeachParam->retAngle = angle;
			m_pTeachParam->retPosX = posX;
			m_pTeachParam->retPosY = posY;
			m_pTeachParam->retCogX = cogX_board;
			m_pTeachParam->retCogY = cogY_board;		

			//////////////////////////////////////////////////////////////////////////  display image
			m_procMil->GetResultImg(clipSearchImg, searchWidth, searchHeight, imgTemp);

 			double factor = m_procMil->GetResizeFactor(sizeX, sizeY, searchWidth, searchHeight);
 			m_procMil->ResizeImg(imgTemp, retImg, searchWidth, searchHeight, sizeX, sizeY, 3, factor);
			//////////////////////////////////////////////////////////////////////////
		}
	}

	delete clipSearchImg;
	delete clipModelImg;
	delete imgTemp;


	ed = GetTickCount();
	double time = ed-st;
	CString str;
	str.Format(_T("%f"), time);
	AfxMessageBox(str);

	return ret;
}

#endif

// int CPInsp_Pattern::LoadModelList(CString path)
// {
// 	int ret = eMPAT_SUCCESS;
// 	ret = m_procMil->ModelList_Load(path);
// 
// 	return ret;
// }

int CPInsp_Pattern::CheckModelList(CString path)
{
	int ret = eMPAT_SUCCESS;
	ret = m_procMil->ModelList_Check(path);

	return ret;
}

int CPInsp_Pattern::ClearModelList(int nLane)
{
	int ret = eMPAT_SUCCESS;
	ret = m_procMil->ModelList_Clear(nLane);

	return ret;
}

int CPInsp_Pattern::LoadModelList(fileAlgoPath* Path)
{
	int ret = eMPAT_SUCCESS;
	//memcpy(ptrFileAlgoPath,Path,sizeof(fileAlgoPath)*ptrFileAlgoPathCnt);
	if(ptrFileAlgoPathCnt == 0)
	{
		CString msg;
		msg.Format(_T("ptrFileAlgoPathCnt == 0"));
		ext::Log::add(msg);
		return ret;
		
	}
	CString Ocrmsg;
	Ocrmsg.Format(_T("%s"),Path[0].m_sPathModelTeach);
	ext::Log::add(Ocrmsg);
	for(int i= 0;i<ptrFileAlgoPathCnt;i++)
	{
		//ptrFileAlgoPath[i].m_sPathModelTeach.Format(_T("%s"), Path[i].m_sPathModelTeach);
		memcpy(ptrFileAlgoPath[i].m_sPathModelTeach, Path[i].m_sPathModelTeach, sizeof(wchar_t) * MAX_STRLEN);
		ptrFileAlgoPath[i].type = Path[i].type;
		memcpy(ptrFileAlgoPath[i].DiviLnR, Path[i].DiviLnR, sizeof(int) * CNT_PATTERN_PATH);
		memcpy(ptrFileAlgoPath[i].DiviLnC, Path[i].DiviLnC, sizeof(int) * CNT_PATTERN_PATH);

		memcpy(ptrFileAlgoPath[i].GapLnR, Path[i].GapLnR, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_DIVISION_R);
		memcpy(ptrFileAlgoPath[i].GapLnC, Path[i].GapLnC, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_DIVISION_C);

		memcpy(ptrFileAlgoPath[i].DiviScore, Path[i].DiviScore, sizeof(double) * CNT_PATTERN_PATH * CNT_PATTERN_SCORE);

		memcpy(ptrFileAlgoPath[i].DetailSearch, Path[i].DetailSearch, sizeof(BOOL) * CNT_PATTERN_PATH);
		ptrFileAlgoPath[i].m_bUseCharacter = Path[i].m_bUseCharacter;
	}

	ret = m_procMil->ModelList_Load(ptrFileAlgoPath,ptrFileAlgoPathCnt);

	return ret;
}

int CPInsp_Pattern::ExtLoadModelList(CString Path)
{
	int ret = eMPAT_SUCCESS;
	
	ret = m_procMil->ModelList_Load(ptrFileAlgoPath, ptrFileAlgoPathCnt);

	return ret;
}


void CPInsp_Pattern::PatternCnt(int cnt)
{
	ptrFileAlgoPathCnt = cnt;
	if(ptrFileAlgoPath!=NULL){

		//delete[] ptrFileAlgoPath;
		g_pMManager->pem_delete(ptrFileAlgoPath, true);
		ptrFileAlgoPath = NULL;
	}
	//ptrFileAlgoPath = new fileAlgoPath[ptrFileAlgoPathCnt];
	ptrFileAlgoPath = g_pMManager->pem_new<fileAlgoPath>(true, ptrFileAlgoPathCnt, (PCHAR)__FUNCTION__, __LINE__);
}

int CPInsp_Pattern::SavePatternModel(CString path, CString sExt)
{
	int ret = eMPAT_SUCCESS;

	int nChannel = m_Mat_fovImage_teach.channels();
	int cx = m_teachCoordinate.cx;
	int cy = m_teachCoordinate.cy;
	int width = m_teachCoordinate.width;
	int height = m_teachCoordinate.length;
	int roiArea;
	if(nChannel == 3) roiArea = width * height * 3;
	else roiArea = width * height;

	double wndAngle = m_teachCoordinate.angle;
	CString pathTemp = m_pTeachParam->modelPath;

	int bdCx = m_teachCoordinate.anyAngleCx;
	int bdCy = m_teachCoordinate.anyAngleCy;
	int bdSizeX = m_teachCoordinate.anyAngleWidth;
	int bdSizeY = m_teachCoordinate.anyAngleLength;
	int bdRoiArea;
	if(nChannel == 3) bdRoiArea = bdSizeX * bdSizeY * 3;
	else bdRoiArea = bdSizeX * bdSizeY;

	if(cx < 0 || cy < 0 || width < 0 || height < 0)
		return eMPAT_FAIL;
	
	//////////////////////////////////////////////////////////////////////////

	//UCHAR* clipModelImg = new UCHAR[roiArea];
	UCHAR* clipModelImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	
	//////////////////////////////////////////////////////////////////////////
	BOOL isAnyAngle = IsAnyAngle(wndAngle);

// 	if(isAnyAngle)
// 	{
// 		double rotAngle = CalcRotateAngle(wndAngle);
// 
// 		int w = 0;
// 		int l = 0;
// 		UCHAR* bdryImgTemp = NULL;
// 		m_procMil->ProcAnyAngle_Img(m_fovImage_teach, &bdryImgTemp, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
// 		m_procMil->SaveWorkImg(bdryImgTemp, w, l, _T("PATTERN_boundaryImg.bmp"));
// 
// 		wndAngle = 0;
// 		cx -= (int)RounD((double)bdCx - ((double)w / 2.0));
// 		cy -= (int)RounD((double)bdCy - ((double)l / 2.0));
// 		int nSX = RounD((double)cx - ((double)width / 2.0));
// 		int nSY = RounD((double)cy - ((double)height / 2.0));
// 
// 		clipModelImg = new UCHAR[roiArea];
// 		m_procMil->GetClipImage(bdryImgTemp, w, l, clipModelImg, nSX, nSY, width, height);
// 		m_procMil->SaveWorkImg(clipModelImg, width, height, _T("COLOR_clipModelImg.bmp"));
// 
// 		delete bdryImgTemp;
// 	}
// 	else
	
	{
		int nSX = RounD((double)cx - ((double)width / 2.0));
		int nSY = RounD((double)cy - ((double)height / 2.0));
		
		if(nChannel == 3)
		{
			cv::Rect rect(nSX, nSY, width, height);
			cv::Mat imageROI = m_Mat_fovImage_teach(rect);
			
			int width = imageROI.cols;
			int height = imageROI.rows;

			for(int y=0; y< height ; y++)
				memcpy(&clipModelImg[y*(width * 3)], &imageROI.data[y*imageROI.step], width * 3);
			m_procMil->SaveWorkImg(clipModelImg, width, height, _T("Pattern_clipModelImg.bmp"),nChannel);

		}
		else 
		{
			m_procMil->GetClipBuff_LT(m_fovImage_teach, clipModelImg, nSX, nSY, width, height); 
			m_procMil->SaveWorkImg(clipModelImg, width, height, _T("Pattern_clipModelImg.bmp"));
		}
	}
	//////////////////////////////////////////////////////////////////////////

	CString modelPath = _T("");		
	if(path.IsEmpty())
		modelPath = pathTemp;
	else
		modelPath = path;


	if(wndAngle > 360)
		wndAngle -= 360;

	int w = 0;
	int h = 0;
	double rotateAngle = 0;
	if(wndAngle > 0 && !isAnyAngle)
		rotateAngle	= GetRotateAngle(width, height, wndAngle, &w, &h);
	

	//클립 이미지 추가 해야함..
	//ret = m_procMil->WritePatternModel(m_fovImage_teach, cx, cy, width, height, modelPath);
	m_procMil->setModelFilterType(m_pAlgoPattern->m_nModelFilter);
	ret = m_procMil->WritePatternModel(clipModelImg, width, height, rotateAngle, modelPath, sExt);

	//delete [] clipModelImg;
	g_pMManager->pem_delete(clipModelImg, true);

	return ret;
}

void CPInsp_Pattern::InitPatternResultStruct(RstInspPattern* rst)
{
	//RstInspPattern* temp = new RstInspPattern;
	RstInspPattern* temp = g_pMManager->pem_new<RstInspPattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, sizeof(RstInspPattern));

	temp->ok = -1;
	temp->isInsp = FALSE;
	temp->list.findOK = TRUE;
	temp->list.scoreOk = TRUE;
	temp->list.angleOk = TRUE;
	temp->list.offsetXOk = TRUE;
	temp->list.offsetYOk = TRUE;
	temp->list.polarityOK = TRUE;

	*rst = *temp;
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

void CPInsp_Pattern::GetInspRst(RstInspPattern* dst)
{
	*dst = m_rstInspPattern;
}

void CPInsp_Pattern::GetInspRst( RstAlgoPattern* pDstAlgoPattern )
{
	if(!pDstAlgoPattern)
		return;

	pDstAlgoPattern->score = m_rstInspPattern.score;
//	if(m_rstInspPattern.angle<0)m_rstInspPattern.angle+=360;
	if(m_rstInspPattern.angle < -180)
		m_rstInspPattern.angle+=360;
	else if(m_rstInspPattern.angle > 180)
		m_rstInspPattern.angle-=360;
	pDstAlgoPattern->angle = m_rstInspPattern.angle;
	pDstAlgoPattern->cogX = m_rstInspPattern.cogX;
	pDstAlgoPattern->cogY = m_rstInspPattern.cogY;
	pDstAlgoPattern->offsetX = m_rstInspPattern.offsetX;
	pDstAlgoPattern->offsetY = m_rstInspPattern.offsetY;
	pDstAlgoPattern->isReverse = m_rstInspPattern.isReverse;
	pDstAlgoPattern->m_bOKFind = m_rstInspPattern.list.findOK;
	pDstAlgoPattern->m_bOKAngle = m_rstInspPattern.list.angleOk;
	pDstAlgoPattern->m_bOKOffsetX = m_rstInspPattern.list.offsetXOk;
	pDstAlgoPattern->m_bOKOffsetY = m_rstInspPattern.list.offsetYOk;
	pDstAlgoPattern->m_bOKScore = m_rstInspPattern.list.scoreOk;
	pDstAlgoPattern->m_bOKPolarity = m_rstInspPattern.list.polarityOK;

	pDstAlgoPattern->m_nDivisionNum = m_rstInspPattern.m_nDivisionNum;
	pDstAlgoPattern->m_nModelNum = m_rstInspPattern.m_nModelNum;
	pDstAlgoPattern->ModelWidth = m_rstInspPattern.ModelWidth;
	pDstAlgoPattern->ModelHeight = m_rstInspPattern.ModelHeight;
	pDstAlgoPattern->m_ptModelPos = m_rstInspPattern.m_ptModelPos;
	memcpy(pDstAlgoPattern->m_dDisiScore, m_rstInspPattern.m_dDiviScore, sizeof(double)*CNT_PATTERN_SCORE);
//	_tcscpy(pDstAlgoPattern->m_SimilarPartCode, m_rstInspPattern.m_SimilarPartCode);
	memcpy(pDstAlgoPattern->m_SimilarPartCode,m_rstInspPattern.m_SimilarPartCode,sizeof(wchar_t)*MAX_STRLEN);
	pDstAlgoPattern->m_dRstSimilarScore = m_rstInspPattern.m_dRstSimilarScore;
}

double CPInsp_Pattern::GetRotateAngle(int wndW, int wndH, double wndAngle, int* retW, int* retH)
{
	int w = wndW;
	int h = wndH;
	double angle = (wndAngle == 0)?  0 : (360 - wndAngle);

	if(angle == 90 || angle == 270)
	{
		w = wndH;
		h = wndW;
	}

	if(wndW != NULL)
		*retW = w;
	if(wndH != NULL)
		*retH = h;

	return angle;
}

void CPInsp_Pattern::SetInspPartInfo(WndInfo &sInspPartInfo)
{
	CSize imgSize = m_procMil->GetImageSize((Im::PIL_ID)m_fovImage_teach);

	m_pInspPartInfo->dAngle = sInspPartInfo.dAngle;
	m_pInspPartInfo->dLength = sInspPartInfo.dLength / m_resolY;
	m_pInspPartInfo->dWidth = sInspPartInfo.dWidth / m_resolX;
	m_pInspPartInfo->m_dPartHeight = sInspPartInfo.m_dPartHeight / m_resolY;
	m_pInspPartInfo->m_dPartWidth = sInspPartInfo.m_dPartWidth / m_resolX;
	m_pInspPartInfo->dCenterX = (double)m_pInspPartInfo->m_dPartWidth / 2. + (sInspPartInfo.dCenterX / m_resolX);
	m_pInspPartInfo->dCenterY = (double)m_pInspPartInfo->m_dPartHeight / 2. - (sInspPartInfo.dCenterY / m_resolY);
}

int CPInsp_Pattern::SearchSimilarPattern()
{
	int ret = e_OK;
	m_rstSimilarScore = 0;
	InspParamPattern paramSimilarInspect;

	if(!m_pInspParam)
		return ret;

	// LMJ 2014/06/02	: Searching 영역 Part ROI -> Window로 변경
	int searchCx = m_inspCoordinate.cx;//m_inspCoordinate.bdrCx;
	int searchCy = m_inspCoordinate.cy;//m_inspCoordinate.bdrCy;
	int searchWidth = m_inspCoordinate.width;//m_inspCoordinate.bdrWidth;
	int searchHeight = m_inspCoordinate.length;//m_inspCoordinate.bdrLength;
	int searchArea = searchWidth * searchHeight;
	double wndAngle = m_inspCoordinate.angle;
	if(IsAnyAngle(wndAngle))
		wndAngle = 0;		// 일반각 일 경우 0도로 돌려진 영상에서 찾기 때문에 0도로 처리

	//double stdCogX = m_pInspParam->stdCogX;
	//double stdCogY = m_pInspParam->stdCogY;
	double stdCogX[CNT_PATTERN_PATH];
	double stdCogY[CNT_PATTERN_PATH];

	memcpy(stdCogX, m_pInspParam->stdCogX, sizeof(double) * CNT_PATTERN_PATH);
	memcpy(stdCogY, m_pInspParam->stdCogY, sizeof(double) * CNT_PATTERN_PATH);

	double posLimit = 5;//m_inspParam->searchAngleRange_Max;
	double negLimit = 5;//m_inspParam->searchAngleRange_Min;

	int bdCx = m_inspCoordinate.anyAngleCx;
	int bdCy = m_inspCoordinate.anyAngleCy;
	int bdSizeX = m_inspCoordinate.anyAngleWidth;
	int bdSizeY = m_inspCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;


	if(searchCx < 0 || searchCy < 0 || searchWidth < 0 || searchHeight < 0 || wndAngle < 0 || wndAngle > 360)
		return ret;

	int nSelIndex = 0;	// 높은 스코어의 Index
	int nCntSearch = 0;
	double angleTemp = 0.;
	CString strFullPath = _T("");

	BOOL bFound = FALSE;
	double dAngleRotate = 0.0;
	
	RstInspPattern SimResult;
	InitPatternResultStruct(&SimResult);
//	AfxMessageBox(m_pAlgoPattern->m_sPathModelTeach[0]);
	int simID(0);
	int nSimPCodeIndex(0);
	int tmpSimID[5] = {0,0,0,0,0};
	int tmp(0);
	CString SimpPartCode;
	m_procMil->SimilarCntReset();

	for(int i =0;i<CNT_PATTERN_SIMILAR;i++)
	{
		bool rear(false);

		CString sSimilarModelPath = m_pAlgoPattern->GetSimilarModelPath(i);
		CString sModelPath = m_pAlgoPattern->GetModelPath(0);
		if(sSimilarModelPath != _T("") &&(sSimilarModelPath.IsEmpty() || sSimilarModelPath.Find(_T("(empty)")) != -1))
			return ret;

		if (sSimilarModelPath.Find(_T("TEMP_REAR")) == -1)
			rear = false;
		else
			rear = true;
		
        if(rear==false && sModelPath != _T("") && sModelPath.Find(_T("TEMP_REAR")) != -1 )
			rear = true;

		CString sPartName;
		sPartName.Format(_T("%s"), sSimilarModelPath);
		if(sPartName == "")
			break;
		simID = m_procMil->SimilarModelListLoad(sPartName,rear);
		if(simID>tmp)
		{
			tmpSimID[nSimPCodeIndex] = simID;
			tmp = simID;
			nSimPCodeIndex++;
		}
	}

	bool bDecision(true);

	for(int i = 0; i < simID; ++i)
	{
		int modelLoadRst = eMPAT_FAIL;

//		if (m_nInspectionMode == eMI_Inspection)
		{
			modelLoadRst = m_procMil->SimilarModelLoad(i);

			if(modelLoadRst==eMPAT_AllocedInternally)
			{
				modelLoadRst = eMPAT_SUCCESS;
				m_procMil->SetAngleMode(posLimit, negLimit, 0.5, FALSE);
				m_procMil->SetSearchStartAngle(0);
			}
		}
// 
// 		if (modelLoadRst == eMPAT_FAIL)
// 		{
// 			modelLoadRst = m_procMil->SimilarModelLoad(i);
// 			if(modelLoadRst==eMPAT_AllocedInternally)
// 			{
// 				modelLoadRst = eMPAT_SUCCESS;
// 				m_procMil->SetAngleMode(posLimit, negLimit, 0.5, FALSE);
// 				m_procMil->SetSearchStartAngle(0);
// 			}
// 		}
		// [0] : 0도 검사 결과, [1] : 180도 검사 결과
		double score[2] = {0, 0};
		double angle[2] = {0, 0};
		double posX[2] = {0, 0};
		double posY[2] = {0, 0};
		double dTempAngle = 0.;
		double offsetX = 0.;
		double offsetY = 0.;
		nCntSearch = 0;
		dAngleRotate = 0.0;
		nSelIndex = 0;	// 높은 스코어의 Index
		if(modelLoadRst == eMPAT_SUCCESS)
		{
			//1차: 0도에서 검사.
			dAngleRotate = 0.0;
			angleTemp = dAngleRotate + wndAngle;

			m_procMil->ModelPreproc(angleTemp,m_pAlgoPattern->m_SamplingAngle);
			m_procMil->SetParam_DivArea(true, 0, 0, 0, 0, 0, 0, CRect());
			m_procMil->GetParam_DivArea(paramSimilarInspect.UseCharacter, paramSimilarInspect.DiviLnC[0], paramSimilarInspect.DiviLnR[0], paramSimilarInspect.GapLnC[0], paramSimilarInspect.GapLnR[0], paramSimilarInspect.DiviScore[0], paramSimilarInspect.DetailSearch[0],CRect());
// 			if(angleTemp != 0)
// 			{	
// 				m_procMil->SetSearchStartAngle(angleTemp);	
// 				m_procMil->PreprocModel(m_fovImage_insp);
// 			}
			//searchCnt =	m_procMil->SearchPattern(m_fovImage_insp, searchCx, searchCy, searchWidth, searchHeight);
			//nCntSearch =	 m_procMil->SearchPattern(clipSearchImg, searchWidth, searchHeight);
			// Window Image (m_fovImage_insp)가 Search 영역이므로 Clip 할 필요가 없다.

			int nchannel = m_procMil->GetnChannel();
			if (nchannel != 3)
			{
				nchannel = 1;
				m_procMil->SetnChannel(1);
			}
			m_procMil->ImgProcess(m_fovImage_insp, m_fovImage_insp_color, searchWidth/2, searchHeight/2, searchWidth, searchHeight,Img_Proc);
			nCntSearch = m_procMil->SearchPattern(m_fovImage_insp, m_fovImage_insp_color, searchWidth/2, searchHeight/2, searchWidth, searchHeight);

			double dPatSubScore[2][CNT_PATTERN_SCORE] = {{0,}, {0,}};
			if(nCntSearch > 0)
			{
				m_procMil->GetPatResult(&score[0], &angle[0], &posX[0], &posY[0]);
				m_procMil->GetPatSubScore(dPatSubScore[0], CNT_PATTERN_DIVISION_C + 1);
			}

			int nSearchCnt(1);
			if(score[0] < 85)	// YJS 2017/02/20 70%=>85%로 변경
			{
				nSearchCnt = 2;
				//2차: 1차에서 실패하거나, score 70% 미만이면 180도 에서 다시 검사.
				dAngleRotate = 180.0;
				angleTemp = dAngleRotate + wndAngle;
				if(angleTemp >= 360)
					angleTemp = angleTemp - 360;

				m_procMil->ModelPreproc(angleTemp,m_pAlgoPattern->m_SamplingAngle);
// 				m_procMil->SetSearchStartAngle(angleTemp);	
// 				m_procMil->PreprocModel(m_fovImage_insp);

				//searchCnt =	m_procMil->SearchPattern(m_fovImage_insp, searchCx, searchCy, searchWidth, searchHeight);
				//nCntSearch =	 m_procMil->SearchPattern(clipSearchImg, searchWidth, searchHeight);
				int nCntSearch2 =	 m_procMil->SearchPattern(m_fovImage_insp, m_fovImage_insp_color, searchWidth/2, searchHeight/2, searchWidth, searchHeight);
				if(nCntSearch == 0)
					nCntSearch = nCntSearch2;

				m_procMil->GetPatResult(&score[1], &angle[1], &posX[1], &posY[1]);
				m_procMil->GetPatSubScore(dPatSubScore[1], CNT_PATTERN_DIVISION_C + 1);
			}

			if(score[0] < score[1])
				nSelIndex = 1;

			double dCogX_pixel = ((searchCx - (searchWidth / 2.0)) + posX[nSelIndex]);
			double dCogY_pixel = ((searchCy - (searchHeight / 2.0)) + posY[nSelIndex]);
			/////
			double dCenTeachX = stdCogX[i%CNT_PATTERN_PATH];
			double dCenTeachY = stdCogY[i%CNT_PATTERN_PATH];

			if((wndAngle == 90) || (wndAngle == 180) || (wndAngle == 270))
			{
				m_proc3d.CorrectCoordinate(dCenTeachX, dCenTeachY, 0, 0, wndAngle,
					0, 0, &dCenTeachX, &dCenTeachY);
			}

			double dshiftX = (dCogX_pixel * m_resolX) - ((m_pInspPartInfo->m_dPartWidth / 2) * m_resolX);
			double dshiftY = ((m_pInspPartInfo->m_dPartHeight / 2) * m_resolY) - (dCogY_pixel * m_resolY);

			/*double cogX_board = 0.0;
			double cogY_board = 0.0;
			CvtPixelToBoard(m_inspCoordinate.fovCx, m_inspCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);
			*/

			//double offsetX = stdCogX - cogX_board;
			//double offsetY = stdCogY - cogY_board;

// 			if(wndAngle == 90 || wndAngle == 270)
// 			{
// 				offsetX = (dCenTeachY - dshiftY) * -1;
// 				offsetY = dCenTeachX - dshiftX;
// 			}
// 			else
			{
				offsetX = dCenTeachX - dshiftX;
				offsetY = (dCenTeachY - dshiftY) * -1;
			}

			m_rstInspSimPattern.m_nModelNum = i + 1;// YJS 2017/03/11
			m_rstInspSimPattern.m_ptModelPos.x = posX[nSelIndex];
			m_rstInspSimPattern.m_ptModelPos.y = posY[nSelIndex];
			memcpy(m_rstInspSimPattern.m_dDiviScore, dPatSubScore[nSelIndex], sizeof(double)*CNT_PATTERN_SCORE);
			m_rstInspSimPattern.score = score[nSelIndex];
// 			dTempAngle = (angle[nSelIndex] - wndAngle);
// 
// 			if(dTempAngle <= -360)
// 			{
// 				while(dTempAngle <= -360)	dTempAngle += 360;
// 			}
// 			else if(dTempAngle >= 360)
// 			{
// 				while(dTempAngle >= 360)	dTempAngle -= 360;
// 			}
// 
// 			m_rstInspSimPattern.angle = dTempAngle;
			// 		m_rstInspSimPattern.cogY = cogY_board;
			m_rstInspSimPattern.offsetX = offsetX;		/// 140207
			m_rstInspSimPattern.offsetY = offsetY;		/// 140207

			bool bDecisionList[2] = {false, };
			int nDivNumList[2] = {2, };
			double dResultScoreList[2] ={0, };

			for(int j=0; j<nSearchCnt; j++)
			{
				if(paramSimilarInspect.DiviLnC[0]>0/* || paramSimilarInspect.DiviLnR[i]>0*/)
				{
					int nDivNum(-1), nDivNgNum(-1);
					double DivScore(HUGE_VAL), DivNGScore(HUGE_VAL);

					int DiviArC = paramSimilarInspect.DiviLnC[0] + 1;
					int DiviArR = paramSimilarInspect.DiviLnR[0] + 1;
					int step = CNT_PATTERN_DIVISION_C + 1;
// 					for (int y=0; y<DiviArR; y++)
					{
						for (int x=0; x<DiviArC; x++)
						{
							if(DivScore > dPatSubScore[j][/*y*step+*/x])
							{
								nDivNum = /*y*step+*/x;
								DivScore = dPatSubScore[j][/*y*step+*/x];
							}
							if(dPatSubScore[j][/*y*step+*/x] < paramSimilarInspect.DiviScore[0][/*y*step+*/x])
							{
								if(DivNGScore > dPatSubScore[j][/*y*step+*/x])
								{
									nDivNgNum = /*y*step+*/x;
									DivNGScore = dPatSubScore[j][/*y*step+*/x];
								}
							}
						}
					}

					if(DivNGScore != HUGE_VAL)
					{
						bDecisionList[j] = false;
						nDivNumList[j] = nDivNgNum + 1;
						dResultScoreList[j] = DivNGScore;
// 						bDecision = false;
// 						m_rstInspSimPattern.m_nDivisionNum = nDivNgNum + 1;
// 						m_rstInspSimPattern.score = DivNGScore;
					}
					else if(DivScore != HUGE_VAL)
					{
						bDecisionList[j] = true;
						nDivNumList[j] = nDivNum + 1;
						dResultScoreList[j] = DivScore;
// 						bDecision = true;
// 						m_rstInspSimPattern.m_nDivisionNum = nDivNum + 1;
// 						m_rstInspSimPattern.score = DivScore;
					}
					else
					{
						bDecisionList[j] = true;
						nDivNumList[j] = 0;
						dResultScoreList[j] = 100.0;
// 						bDecision = true;
// 						m_rstInspSimPattern.m_nDivisionNum = 0;
					}
				}
				else
				{
					if(score[j] < m_pInspParam->stdScore)
					{
						bDecisionList[j] = false;
					}
					else
						bDecisionList[j] = true;
					dResultScoreList[j] = score[j];
					nDivNumList[j] = 0;
// 					if(score[nSelIndex] < m_pInspParam->stdScore)
// 					{
// 						bDecision = false;
// 					}
// 					else
// 						bDecision = true;
//					m_rstInspSimPattern.score = score[nSelIndex];
// 					m_rstInspSimPattern.m_nDivisionNum = 0;
				}
			}


			int nSelIdx(0);
			if(nSearchCnt>=2 && dResultScoreList[0] < dResultScoreList[1])
				nSelIdx = 1;

			memcpy(m_rstInspSimPattern.m_dDiviScore, dPatSubScore[nSelIdx], sizeof(double)*CNT_PATTERN_SCORE);
			bDecision = bDecisionList[nSelIdx];
			m_rstInspSimPattern.score = dResultScoreList[nSelIdx];
			m_rstInspSimPattern.m_nDivisionNum = nDivNumList[nSelIdx];

			if(m_rstSimilarScore<m_rstInspSimPattern.score)
			{
				if(i<tmpSimID[0])
				{
					SimpPartCode.Format(_T("%s"),m_pAlgoPattern->m_sSimilarPartList1);
				}
				else if(i<tmpSimID[1])
				{
					SimpPartCode.Format(_T("%s"),m_pAlgoPattern->m_sSimilarPartList2);
				}
				else if(i<tmpSimID[2])
				{
					SimpPartCode.Format(_T("%s"),m_pAlgoPattern->m_sSimilarPartList3);
				}
				else if(i<tmpSimID[3])
				{
					SimpPartCode.Format(_T("%s"),m_pAlgoPattern->m_sSimilarPartList4);
				}
				else if(i<tmpSimID[4])
				{
					SimpPartCode.Format(_T("%s"),m_pAlgoPattern->m_sSimilarPartList5);
				}
				m_rstSimilarScore = m_rstInspSimPattern.score;
			}

			dTempAngle = (angle[nSelIdx] - wndAngle);

			if(dTempAngle <= -360)
			{
				while(dTempAngle <= -360)	dTempAngle += 360;
			}
			else if(dTempAngle >= 360)
			{
				while(dTempAngle >= 360)	dTempAngle -= 360;
			}

			m_rstInspSimPattern.angle = dTempAngle;

			if(nCntSearch >= 1)
			{
				bFound = TRUE;

				if(bDecision==false || m_rstSimilarScore < m_rstInspPattern.score)
				{
					continue;
				}
			}
			else
			{
				continue;
			}
			break;
		}
		else
		{
			m_rstInspSimPattern.score = score[nSelIndex];
			m_rstInspSimPattern.angle = dTempAngle;
			m_rstInspSimPattern.offsetX = offsetX;		/// 140207
			m_rstInspSimPattern.offsetY = offsetY;		/// 140207
		}
	}

	m_procMil->SimilarCntReset();
	//////////////////////////////////////////////////////////////////////////	

	if(bFound == TRUE)
	{
		m_rstInspSimPattern.ok = TRUE;
		m_rstInspSimPattern.list.findOK = TRUE;

		if(nSelIndex == 1)
			m_rstInspSimPattern.isReverse = TRUE;
		else 
			m_rstInspSimPattern.isReverse = FALSE;
	}
	else
	{
		m_rstInspSimPattern.ok = FALSE;
		m_rstInspSimPattern.list.findOK = FALSE;		
	}


	m_rstInspSimPattern.isInsp = TRUE;


	ret = DecisionSimPatternInsp(bDecision);

	if(ret == e_NG)
		ret = e_OK;
	else if(m_rstInspSimPattern.score>= m_rstInspPattern.score)
		ret = e_NG;
// 
// 	if(m_pAlgoPattern->m_bUseNGOpt == TRUE)
// 	{
// 		if(ret == e_NG)
// 			ret = e_OK;
// 		else
// 			ret = e_NG;
// 	}


	//////////////////////////////////////////////////////////////////////////

//	delete clipSearchImg;
	if(ret == e_NG)
	{
		_tcscpy(m_rstInspPattern.m_SimilarPartCode, SimpPartCode);
		m_rstInspPattern.m_dRstSimilarScore = m_rstSimilarScore;
		ext::Log::add(SimpPartCode);
		//g_pMPTI->AddLog_Dev(SimpPartCode);
	}
	return ret;
}

int CPInsp_Pattern::DecisionSimPatternInsp(bool bDecision)
{
	int ret = e_OK;

	double stdScore = m_pInspParam->stdScore;
	double curScroe = m_rstInspSimPattern.score;
	double angleRange = m_pInspParam->angleRange;
	double curAngle = m_rstInspSimPattern.angle;
	double offsetRange_x = m_pInspParam->offsetRange_x;
	double offsetRange_y = m_pInspParam->offsetRange_y;
	double curOffSetX = m_rstInspSimPattern.offsetX;
	double curOffSetY = m_rstInspSimPattern.offsetY;	

	BOOL isPolarity = m_pInspParam->isPolarity;
	if(m_rstInspSimPattern.list.findOK == TRUE) //찾았을 경우
	{
		ret = TRUE;


		//score NG
		if(bDecision == false) 
		{
			ret = e_NG;	
			m_rstInspSimPattern.ok = FALSE;

			m_rstInspSimPattern.list.scoreOk = FALSE;
		}

		// 		if(curScroe < stdScore) 
		// 		{
		// 			ret = e_NG;	
		// 			m_rstInspSimPattern.ok = FALSE;
		// 
		// 			m_rstInspSimPattern.list.scoreOk = FALSE;
		// 		}
		// 		else
		// 		{
		// 			m_rstInspSimPattern.list.scoreOk = TRUE;
		// 		}
		// 
		// 		if (m_rstInspSimPattern.m_nModelNum > 0)
		// 		{
		// 			if ((m_pInspParam->DiviLnC[m_rstInspSimPattern.m_nModelNum - 1] > 0 || m_pInspParam->DiviLnR[m_rstInspSimPattern.m_nModelNum - 1] > 0))
		// 			{
		// 				if (m_rstInspSimPattern.m_nDivisionNum > 0)
		// 				{
		// 					ret = e_NG;
		// 					m_rstInspSimPattern.list.scoreOk = FALSE;
		// 				}
		// 				else
		// 				{
		// 					ret = e_OK;
		// 					m_rstInspSimPattern.list.scoreOk = TRUE;
		// 				}
		// 			}
		// 		}


		//////////////////////////////////////////////////////////////////////////
		//angle NG

		if(curAngle < 0)
			curAngle = 360 + curAngle;


		double tempAngle[2] = {0.0, 180.0};

		int repeat = 2;

		if(isPolarity == TRUE)
			repeat = 1;

		BOOL bAngleOk = FALSE;
		for(int i = 0; i < repeat; i++)
		{
			double a1 = 0;
			double a2 = 0;
			double b1 = 0;
			double b2 = 0;


			a1 = tempAngle[i];
			a2 = tempAngle[i] + angleRange;
			if(a2 > 360)
				a2 -= 360;
			b1 = tempAngle[i] + 360;
			if(b1 > 360)
				b1 -= 360;
			b2 = (tempAngle[i] + 360) - angleRange;
			if(b2 > 360)
				b2 -= 360;

			if((curAngle >= a1 && curAngle <= a2) || (curAngle <= b1 && curAngle >= b2))
				bAngleOk = TRUE;
		}


		if(bAngleOk) 
		{
			m_rstInspSimPattern.list.angleOk = TRUE;				
		}
		else
		{
			ret = e_NG;
			m_rstInspSimPattern.ok = FALSE;
			m_rstInspSimPattern.list.angleOk = FALSE;			
		}
		//////////////////////////////////////////////////////////////////////////
		//offset NG
		if(PATTERN_LOCK && m_pInspParam->bUseOffset && (abs(curOffSetX) > offsetRange_x || abs(curOffSetY) > offsetRange_y))
		{
			ret = e_NG;
			m_rstInspSimPattern.ok = FALSE;
			if(abs(curOffSetX) > offsetRange_x)
				m_rstInspSimPattern.list.offsetXOk = FALSE;
			if(abs(curOffSetY) > offsetRange_y)
				m_rstInspSimPattern.list.offsetYOk = FALSE;
		}
		else
		{
			m_rstInspSimPattern.list.offsetXOk = TRUE;
			m_rstInspSimPattern.list.offsetYOk = TRUE;
		}
	}
	else//못찾았을 경우
	{
		ret = e_NG;
		m_rstInspSimPattern.ok = FALSE;

		m_rstInspSimPattern.list.findOK = FALSE;
		m_rstInspSimPattern.list.angleOk = FALSE;
		m_rstInspSimPattern.list.offsetXOk = FALSE;
		m_rstInspSimPattern.list.offsetYOk = FALSE;
		m_rstInspSimPattern.list.scoreOk = FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	if(ret == e_NG)
	{
		m_rstInspSimPattern.ok = FALSE;

		if(isPolarity == TRUE)
		{
			if(m_rstInspSimPattern.list.findOK == FALSE || m_rstInspSimPattern.list.scoreOk == FALSE || m_rstInspSimPattern.list.angleOk == FALSE)
				m_rstInspSimPattern.list.polarityOK = FALSE;
			else
				m_rstInspSimPattern.list.polarityOK = TRUE;

		}
		else 
		{
			m_rstInspSimPattern.list.polarityOK = TRUE;	
		}
	}
	else
	{
		if(isPolarity == TRUE && m_rstInspSimPattern.isReverse == TRUE)
			m_rstInspSimPattern.list.polarityOK = FALSE;
		else 
			m_rstInspSimPattern.list.polarityOK = TRUE;
	}

	if(m_pAlgoPattern->m_bUseNGOpt == TRUE)
	{
		if(m_rstInspSimPattern.ok == FALSE)
			m_rstInspSimPattern.ok = TRUE;
		else
			m_rstInspSimPattern.ok = FALSE;

		if(m_rstInspSimPattern.list.scoreOk == FALSE)
			m_rstInspSimPattern.list.scoreOk = TRUE;
		else
			m_rstInspSimPattern.list.scoreOk = FALSE;
	}

	return ret;
}

void CPInsp_Pattern::SetSimListParam( const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sInspPartInfo)
{
	int ret = e_OK;

	// 실제 언제 들어오는지 알 수 없음
	m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("InspArea_ABefore.bmp"));
	CPInsp::FillOutOfInspArea(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, sWndAlgoImg.dAngle, sWndAlgoImg.m_ucArr2D, NULL, sInspAlgo.m_nUsedInspPolygon, sInspAlgo.m_ptArrInspPolygon, 0, sInspAlgo.m_bConvetExceptROI);
	m_pProcMilAlgo->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("InspArea_After.bmp"));

	m_procMil->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Masking_ABefore.bmp"));

	CPInsp::FillMaskingROI(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, sWndAlgoImg.dAngle, sWndAlgoImg.m_ucArr2D, sInspAlgo.m_nUsedMaskingValue, sInspAlgo.m_rcArrMaskingROI);

	m_procMil->SaveWorkImg(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, _T("Masking_After.bmp"));

	m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp);
	m_fovImage_insp = (void*)m_procMil->GetMilSrc(sWndAlgoImg);

	m_procMil->SaveWorkImg((Im::PIL_ID)m_fovImage_insp,_T("SetSimListParam_m_fovImage_insp.bmp"));

	m_inspCoordinate.angle = sInspPartInfo.dAngle;
	m_inspCoordinate.cx = sInspPartInfo.dCenterX;
	m_inspCoordinate.cy = sInspPartInfo.dCenterY;
	m_inspCoordinate.length = sInspPartInfo.dLength;
	m_inspCoordinate.width = sInspPartInfo.dWidth;

	Img_Proc = std::make_shared<ImgProcessing>();
}

double CPInsp_Pattern::inspSimilarList(const InspAlgo &sInspAlgo,  SimilarPartInfo* listPartCode, int* listScore, int nlistCnt)
{
	int ret = e_OK;

	if(!m_pInspParam){

		return ePART_FAIL;
	}
	m_nInspectionMode = eMI_SimilarInsp;

	for(int i=0;i<nlistCnt;i++)
	{
		InitPatternResultStruct(&m_rstInspPattern);
		InitPatternResultStruct(&m_pInspParam->retInspPatternResult);

		m_pAlgoPattern = (AlgoPattern *)sInspAlgo.m_ptrInspAlgoParam;
		m_pAlgoPattern->GetParam(m_pInspParam);	
		for(int j=0;j<CNT_PATTERN_PATH;j++)
		{
			m_pInspParam->DiviLnC[j]=0;
			m_pInspParam->DiviLnR[j]=0;
		}

		_tcscpy(m_pInspParam->modelPath, listPartCode[i].m_sPartCode);
		//		memcpy(m_pAlgoPattern->m_sArrPathModelInspect[i],listPartCode[i].m_sPartCode,sizeof(listPartCode[i].m_sPartCode));
		m_pAlgoPattern->m_nCntPatternPath = 0;

		ret = ProcStep(ePStepID_PATTERN_SEARCH, 0, 0);

		listScore[i] = m_rstInspPattern.score;

		m_rstInspPattern.score = 0;
	}
	return ret;
}

void CPInsp_Pattern::GetMaskImage(CString sPath, UCHAR * ptr, int Wid, int Len, int Pitch)
{
	m_procMil->GetMaskImage(sPath, ptr, Wid, Len, Pitch);
	setInspLightbt(m_procMil->getPilLightbt());
	setInspMatchAlgo(m_procMil->getPilMatchAlgo());
	setInspModelnChannel(m_procMil->getPilModelnChannel());
	setModelFilterType(m_procMil->getModelFilterType());
}
void CPInsp_Pattern::SetMaskImage(UCHAR * ptr, int Wid, int Len, int Pitch, double dAngle)
{
	m_procMil->SetMaskImage(ptr, Wid, Len, Pitch,dAngle);
}
