#include "StdAfx.h"
#include "PInsp_OCR.h"
#include "OCR_Dll/OcrDll.h"
#include "MPTI.h"

#include "ippi.h"
#include "ipps.h"
#pragma comment(lib, "ippi.lib")
#pragma comment(lib, "ipps.lib")

// #ifndef _DEBUG
#pragma comment(lib, "OCR_Dll/OCR_Dllx64.lib")
// #else
// #pragma comment(lib, "OCR_Dll/OCR_Dllx64D.lib")
// #endif
double doubeZeroBuf[MAX_STRLEN];
int intZeroBuf[MAX_STRLEN];
wchar_t wcharZeroBuf[MAX_STRLEN];

CPInsp_OCR::CPInsp_OCR(void)
{
	m_className = _T("CPInsp_OCR");

	m_fovImage_insp = M_NULL;
	for (int iLoopCount = 0; iLoopCount < 2; ++iLoopCount)	
		m_fovImage_insp_Mix[iLoopCount] = NULL;	//LJH 2016.05.26 ADD
	m_fovImage_teach = M_NULL;
	SetAllocFovImg(FALSE);

	m_procMil = NULL;
	//m_milSys = M_NULL;
	//m_milApp = M_NULL;

	m_mainLightIndex = 0;
	m_inspCount = 0;

	m_pInspAlgoOCR = NULL;
	m_pTeachParam = &m_paramTeach;
	m_pInspParam = &m_paramInspect;

	m_pInspImg = NULL;
	m_pInspImg_rotate = NULL;
	//m_pWndImg = NULL;

	//LJH 2016.05.26
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
	{
		m_pInspImg_Mix[iLoopCount] = NULL;
		m_pInspImg_Mix_rotate[iLoopCount] = NULL;
		//m_pWndImg_Mix[iLoopCount] = NULL;
	}

	m_roiSizeX = 0;
	m_roiSizeY = 0;

	for (int iLoopCount = 0; iLoopCount < 3; ++iLoopCount)
		m_pemtoThreshold[iLoopCount] = 0;

	InitStrResultStruct(&m_result);

	//InitStrResultTempStruct(); // shkim 초기화할 m_procMil의 ZeroBuffer가 생성되지 않아서 주석처리후 하단으로 변
	m_resultTemp.strCount = 0;
	m_resultTemp.charCount = 0;
	m_resultTemp.stringScore = 0.0;
	m_resultTemp.strPosX = 0.0;
	m_resultTemp.strPosY = 0.0;
	m_resultTemp.strAngle = 0.0;
	m_resultTemp.str = _T("");
	memset(m_resultTemp.charScore,0,sizeof(double)*MAX_STRLEN);
	memset(m_resultTemp.charX,0,sizeof(double)*MAX_STRLEN);
	memset(m_resultTemp.charY,0,sizeof(double)*MAX_STRLEN);
	memset(m_resultTemp.charWidth,0,sizeof(double)*MAX_STRLEN);
	memset(m_resultTemp.charHeight,0,sizeof(double)*MAX_STRLEN);
	memset(m_resultTemp.charangle,0,sizeof(double)*MAX_STRLEN);

	m_oldResultFlag = false;

	m_nInspectionMode = eMI_WindowTeaching;

	//m_Opencv_Functions = new COpenCV_Functions();
	m_Opencv_Functions = g_pMManager->pem_new<COpenCV_Functions>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

	m_nstringIndex = 0;

	OCR_Dll_CreateInterface();
	m_dCurrentPartAngle = 0.0;
	m_bIsHisto = false;
	m_bReRotate = false;
	m_bUseImgBufAlloc = true;

	m_nUserValueinsp = 0;
	m_nAutoValueInsp = 0;
	m_nPemtoValueInsp = 0;
	m_nPemToValueInsp_ImageMulti = 0;
	m_nPemToValueInsp_ImageMulti_auto = 0;
	m_nThresholdTracking = 0;

	m_nUserValueinsp_in=0;
	m_nAutoValueInsp_in=0;
	m_nPemtoValueInsp_in=0;
	m_nPemToValueInsp_ImageMulti_in=0;
	m_nPemToValueInsp_ImageMix_in=0;
	m_nThresholdTracking_in=0;
	m_OCRNGParamSave = false;
	consecute_tmp = 0;

	m_bstringMulti = true;

#if Timecheck
	Singleword_Insp = 0;
	StepID_IMGBUF_ALLOC = 0;
	StepID_USERVALUE = 0;
	StepID_USERVALUE_REV = 0;		
	StepID_AUTO = 0;
	StepID_PEMVALUE = 0;	
	StepID_AUTO_REV = 0;
	StepID_PEMVALUE_REV = 0;
	StepID_THRESHOLD_TRACKING = 0;			//2016.12.01 shkim threshold tracking Insp
	StepID_THRESHOLD_TRACKING_REV = 0;
	StepID_MIX_PEMVALUE = 0;		//LJH 2016.05.27	영상 합친 후 검사
	StepID_MIX_PEMVALUE_REV = 0;	//LJH 2016.05.27	영상 합친 후 검사 (영상 회전)
#endif

	memset(doubeZeroBuf,-1,sizeof(double)*MAX_STRLEN);
	memset(intZeroBuf,-1,sizeof(int)*MAX_STRLEN);
	memset(wcharZeroBuf,0,sizeof(wchar_t)*MAX_STRLEN);

	bIsPilBuff = false;
}


CPInsp_OCR::~CPInsp_OCR(void)
{
	if(IsAllocFovImg())
	{
		m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_teach);
		m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp);
		//LJH 2016.05.26 -> 2016.06.13
		//for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)	
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)	
			m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp_Mix[iLoopCount]);
	}

	DeleteImgBuf();
	OCR_Dll_DestroyInterface();
	CPInsp_OCR::CloseDevice();
	if(m_Opencv_Functions != NULL)
		//delete m_Opencv_Functions;
		g_pMManager->pem_delete(m_Opencv_Functions, false);
}

int CPInsp_OCR::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	// Allocate a Application.
	//m_milApp = milApp;
	// Allocate a System.
	//m_milSys = milSys;

	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	//m_procMil = new CProcMil_OCR();
	m_procMil = g_pMManager->pem_new<CProcMil_OCR>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	//m_procMil->InitMil(m_milApp, m_milSys);
	m_procMil->InitMil(milApp, milSys,bUseImagePilLib);
	m_procMil->SetResol(resolX, resolY, m_fovWidth);

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength ,resolX, resolY, g_pMPTI->isUseImagePilLib());

	bIsPilBuff = !bUseImagePilLib;
	return ePART_SUCCESS;
}

int CPInsp_OCR::CloseDevice()
{
	CPInsp::CloseDevice();

	if(m_procMil != NULL)
	{
		m_procMil->FreeMil();
		
		//delete m_procMil;
		g_pMManager->pem_delete(m_procMil, false);
		m_procMil = NULL;
	}

	//m_milSys = M_NULL;
	//m_milApp = M_NULL;

	return ePART_SUCCESS;	
}

int CPInsp_OCR::SetInspParam(void* itemParam, void* targetImg, Coordinate cdn)
{
	m_pInspParam = (InspParamOCR*) itemParam;
	m_fovImage_insp = targetImg;
	m_inspCoordinate = cdn;
	return ePART_SUCCESS;
}

int CPInsp_OCR::SetInspParam(void* itemParam, Coordinate cdn, InspImgBuf img, int mainImgIndex)
{
	m_pInspParam = (InspParamOCR*) itemParam;
	m_inspCoordinate = cdn;

	m_mainLightIndex = mainImgIndex;

	m_imgBuf[eImg_Top_R] = img.imgTop_R;
	m_imgBuf[eImg_Top_G] = img.imgTop_G;
	m_imgBuf[eImg_Top_B] = img.imgTop_B;

	m_imgBuf[eImg_Middle_R] = img.imgMiddle_R;
	m_imgBuf[eImg_Middle_B] = img.imgMiddle_B;

	m_imgBuf[eImg_Bottom_R] = img.imgBottom_R;
	m_imgBuf[eImg_Bottom_B] = img.imgBottom_B;

	//m_procMil->MakeImg(m_imgBuf[eImg_Top_R], m_imgBuf[eImg_Top_G], m_imgBuf[eImg_Top_B], m_imgBuf[eImg_Middle_R], m_imgBuf[eImg_Middle_B], m_imgBuf[eImg_Bottom_R], m_imgBuf[eImg_Bottom_B]);


	return ePART_SUCCESS;
}

int CPInsp_OCR::SetInspParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo )
{
	if(bIsPilBuff)
		return ePART_FAIL;
	if(!m_pInspParam)
		return ePART_FAIL;
	if(sInspAlgo.m_eAlgoType != eAlgoOCR)
		return ePART_FAIL;
	m_pInspAlgoOCR = (AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pInspAlgoOCR)
		return ePART_FAIL;

	if(m_dCurrentPartAngle<0)m_dCurrentPartAngle+=360;

	m_pInspAlgoOCR->GetParam(m_pInspParam);

	m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp);

	//LJH 2016.05.26 ADD
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
		m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_insp_Mix[iLoopCount]);

	if(m_dCurrentPartAngle<0)m_dCurrentPartAngle+=360;

	m_mainLightIndex = -1;
	m_fovImage_insp = (void*)m_procMil->GetMilSrc(sWndAlgoImg,TRUE,m_dCurrentPartAngle);
	//LJH 2016.05.26 ADD
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
		m_fovImage_insp_Mix[iLoopCount] = (void*)m_procMil->GetMilSrc(sWndAlgoImg, iLoopCount, TRUE,m_dCurrentPartAngle);

	m_procMil->SaveWorkImg((Im::PIL_ID)m_fovImage_insp, _T("OCROrgin.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) m_procMil->SaveWorkImg((Im::PIL_ID)m_fovImage_insp_Mix[0], _T("OCROrginMix1.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) m_procMil->SaveWorkImg((Im::PIL_ID)m_fovImage_insp_Mix[1], _T("OCROrginMix2.bmp"));
// 	m_procMil->SaveWorkImg((Im::PIL_ID)m_fovImage_insp, _T("OCROrgin.tif"));
	
	//m_Opencv_Functions->Histogram((Im::PIL_ID)m_fovImage_insp);
	//m_procMil->SaveWorkImg((Im::PIL_ID)m_fovImage_insp, _T("OCRAfterOpencv.bmp"));

	SetAllocFovImg(TRUE);

	m_inspCoordinate.SetParamROI(coordinateAlgo);

	m_roiSizeX = m_inspCoordinate.width;
	m_roiSizeY = m_inspCoordinate.length;

// 	GenAngle_left = m_pInspAlgoOCR->m_rtInspectArea.left;
// 	GenAngle_top = m_pInspAlgoOCR->m_rtInspectArea.top;

	//2016.11.03 shkim for General Angle Insp
	if((m_dCurrentPartAngle > 7 && m_dCurrentPartAngle < 30) || (m_dCurrentPartAngle<-7 && m_dCurrentPartAngle >-30)||(m_dCurrentPartAngle<353 && m_dCurrentPartAngle >330))
	{
		double angle = 360 - m_dCurrentPartAngle;
		if(angle>360)angle -=360; 
		double radian = angle * 3.141592654/180;
		double rx = abs(sWndAlgoImg.m_nWidth*cos(radian)) + abs(sWndAlgoImg.m_nHeight*sin(radian));
		double ry= abs(sWndAlgoImg.m_nWidth*sin(radian)) + abs(sWndAlgoImg.m_nHeight*cos(radian));
// 		if (g_pMPTI->m_bUseCognex == true)
// 		{
// 
// 			double centerx = (m_roiSizeX/2) + m_pInspAlgoOCR->m_rtInspectArea.left;
// 			double centery = (m_roiSizeY/2) + m_pInspAlgoOCR->m_rtInspectArea.top;
// 
// 			double rcenterx = rx*centerx/sWndAlgoImg.m_nWidth;
// 			double rcentery = ry*centery/sWndAlgoImg.m_nHeight;
// 
// 			GenAngle_left = rcenterx -centerx + m_pInspAlgoOCR->m_rtInspectArea.left;
// 			GenAngle_top = rcentery - centery + m_pInspAlgoOCR->m_rtInspectArea.top;
// 		}
// 		else
// 		{
			m_roiSizeX = rx;
			m_roiSizeY = ry;
// 		}

	}
	return ePART_SUCCESS;
}

int CPInsp_OCR::SetTeachParam(void* itemParam, void* targetImg, Coordinate cdn, int teachType)
{
	m_pTeachParam = (TeachParamOCR*) itemParam;
	m_fovImage_teach = targetImg;
	m_teachCoordinate = cdn;
	return ePART_SUCCESS;
}

int CPInsp_OCR::SetTeachParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo )
{
	if(bIsPilBuff)
		return ePART_FAIL;
	if(!m_pTeachParam)
		return ePART_FAIL;
	if(sInspAlgo.m_eAlgoType != eAlgoOCR)
		return ePART_FAIL;
	m_pInspAlgoOCR = (AlgoOCR *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pInspAlgoOCR)
		return ePART_FAIL;

	m_pInspAlgoOCR->GetParam(m_pTeachParam);

	m_procMil->FreeMilImageBuff((Im::PIL_ID*)&m_fovImage_teach);
	m_fovImage_teach = (void*)m_procMil->GetMilSrc(sWndAlgoImg);
	SetAllocFovImg(TRUE);


	m_teachCoordinate.SetParamROI(coordinateAlgo);

	return ePART_SUCCESS;
}

//LJH 2016.05.27
int CPInsp_OCR::Binarize(const UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nTypeRange, int nThreshold, BOOL bInvert, UCHAR* &pByImgDst)
{
	if(pByImgSrc == NULL || pByImgDst == NULL || nImgWidth <= 0 || nImgHeight <= 0 || nThreshold <= 0)
		return 0;
	if(bIsPilBuff)
		return eMR_FAIL;

	int nSizeX = nImgWidth % 4;
	int widthstep = nImgWidth;
	/*if (nSizeX!=0)
	{
		nSizeX = 4 - nSizeX;
		widthstep = nImgWidth + nSizeX;
	}*/
	widthstep = g_pMPTI->nCalcWidthStep(false, nImgWidth);

	for (int i = 0 ; i<nImgHeight ; i++)
	{
		for (int j = 0 ; j<nImgWidth ; j++)
		{
			if(pByImgSrc[(nImgWidth * i) + j] >= nThreshold)	pByImgDst[(nImgWidth * i) + j] = bInvert ? 0 : 255;
			else												pByImgDst[(nImgWidth * i) + j] = bInvert ? 255 : 0;
		}
	}

	return eMR_SUCCESS;
}

int CPInsp_OCR::BinarizeImageReverse(const UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, UCHAR* &pByImgDst)
{
	if(pByImgSrc == NULL || pByImgDst == NULL || nImgWidth <= 0 || nImgHeight <= 0)
		return 0;
	if(bIsPilBuff)
		return 0;

	//int nSizeX = nImgWidth % 4;
	int widthstep = nImgWidth;
	/*if (nSizeX!=0)
	{
		nSizeX = 4 - nSizeX;
		widthstep = nImgWidth + nSizeX;
	}*/
	widthstep = g_pMPTI->nCalcWidthStep(false, nImgWidth);

	for (int i = 0 ; i<nImgHeight ; i++)
	{
		for (int j = 0 ; j<nImgWidth ; j++)
		{
			if(pByImgSrc[(nImgWidth * i) + j] == 0)		pByImgDst[(nImgWidth * i) + j] = 255;
			else										pByImgDst[(nImgWidth * i) + j] = 0;
		}
	}
}

int CPInsp_OCR::InspProc_OCR(RstAlgoOCR * sRstAlgo, int nInspectionMode)
{
	int ret = e_NG;
	int nLine = __LINE__;
	try
	{
		if(bIsPilBuff)
			return e_NG;
		//shkim Img Alloc
		m_nInspectionMode = nInspectionMode;

		ret = ProcAuto();

		// ÀÓ½Ã (ÃßÈÄ º¯°æÇÒ °Í)
		//if(ret == e_OK)
		//	ret = m_result.ok;
		if(ret != e_OK)
			ret = e_NG;

		sRstAlgo->m_nCharCount = m_result.charCount;
		sRstAlgo->m_dStringScore = m_result.stringScore;

		memcpy(sRstAlgo->m_dArrCharScore,doubeZeroBuf,sizeof(double)*MAX_STRLEN);
		memcpy(sRstAlgo->m_cArrStr,wcharZeroBuf,sizeof(wchar_t)*MAX_STRLEN);
		memcpy(sRstAlgo->m_nCharScoreResult,intZeroBuf,sizeof(int)*MAX_STRLEN);

		for(int i = 0; i < sRstAlgo->m_nCharCount; i++)
		{
			sRstAlgo->m_dArrCharScore[i] = m_result.charScore[i];
			sRstAlgo->m_cArrStr[i] = m_result.str[i];
			sRstAlgo->m_nCharScoreResult[i] = m_result.charScoreResult[i];
		}
		sRstAlgo->m_dStrPosX = m_result.strPosX;
		sRstAlgo->m_dStrPosY = m_result.strPosY;
		sRstAlgo->m_dStrAngle = m_result.strAngle;
		sRstAlgo->m_nInspCount = m_result.inspCount;
		sRstAlgo->m_bIsReverse = m_result.isReverse;
		sRstAlgo->m_bOKString = m_result.list.stringOk;
		sRstAlgo->m_bOKScore = m_result.list.stringScoreOk;
		sRstAlgo->m_bOKPolarity = m_result.list.polarityOK;

		return ret;
	}
	catch(...)
	{
		if(g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CMInspManager::InspProc_OCR(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return ret;
}

int CPInsp_OCR::LoadModelList(CString path)
{
	int ret = eSTR_SUCCESS;
	ret = m_procMil->ModelList_Load(path);

	return ret;
}

int CPInsp_OCR::CheckModelList(CString path)
{
	int ret = eSTR_SUCCESS;
	ret = m_procMil->ModelList_Check(path);

	return ret;
}

int CPInsp_OCR::ClearModelList(int nLane)
{
	int ret = eSTR_SUCCESS;
	ret = m_procMil->ModelList_Clear(nLane);

	return ret;
}

void CPInsp_OCR::RemoveModel(CString sFontPath)
{
	m_procMil->ModelRemove(sFontPath);
}

int CPInsp_OCR::ProcAuto()
{
	int ret = e_NG;

	if(bIsPilBuff)
		return e_NG;
	m_oldResultFlag = false;
	InitStrResultStruct(&m_result);
	InitStrResultStruct(&m_pInspParam->retInspOcrResult);
	InitStrResultTempStruct();
	int fontCnts = 0;
	m_nFontSizeY = 0;
	int isLoad = LoadStrFont(m_pInspParam->fontPath, &fontCnts, &m_nFontSizeY, m_nInspectionMode);  

	if(m_nFontSizeY <= FONTSIZE_S)
		m_nThickenCnt = 0;
	else if(m_nFontSizeY <= FONTSIZE_M)
		m_nThickenCnt = 1;
	else
		m_nThickenCnt = 2;
	
	if(isLoad == eSTR_FAIL)
	{
		ret = ePROC_OCR_NOTFOUND_FILE;
		return ret;
	}
	if(fontCnts < 1 )
	{
		ret = ePROC_OCR_NOTFOUND_FONT;
		return ret;
	}	
	
	int stepID = 0;
	int maxStepCnts = eOStepID_COUNT;

	//step1(eOStepID_USERVALUE) : USER_DEFINED (user value)
	//step2(eOStepID_AUTO) : LOCAL -> LOCAL_WITH_RESEG -> USER_DEFINED (auto value for matrox)
	//step3(eOStepID_PEMVALUE) : USER_DEFINED (auto value for pemtron)
	
	//////////////////////////////////////////////////////////////////////////
	if(m_mainLightIndex > -1)
	{
		int imgIndex = m_mainLightIndex;
		int index[eImg_BufCnt] = {0,};
		int indexCount = 0;
		GetLightArray(imgIndex, index, &indexCount);

		m_procMil->StrAllocResult();
		//huj 2014/03/06  preprocess¸¦ ÇÑ¹ø¸¸ ÇÑ´Ù.
		if(AllocString() == e_NG){
			DecisionOcrInsp(e_NG);
			return e_NG;
		}
		for(int j = 0; j < indexCount; j++)
		{
			m_fovImage_insp = m_imgBuf[index[j]];
			//LJH 2016.05.26
			for(int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
				m_fovImage_insp_Mix[iLoopCount] = m_imgBuf[index[j]];

			stepID = 0;
			for(int i = 0; i < maxStepCnts; i++)
			{	
				ret = ProcStep(stepID);

				stepID++;
				if(ret == e_OK) 
					break;
			}

			if(ret == e_OK) 
				break;
		}

		//¼öÁ¤µÈ ¼Ò½º  (ÀÌ¹ÌÁö 7Àå)
		/*for(int j = 0; j < eImg_BufCnt; j++)
		{
		index = GetLightIndex(imgIndex);

		m_fovImage_insp = m_imgBuf[index];

		stepID = 0;
		for(int i = 0; i < maxStepCnts; i++)
		{	
		ret = ProcStep(stepID);

		stepID++;
		if(ret == e_OK) 
		break;
		}
		if(ret == e_OK) 
		break;


		if(m_mainLightIndex != 0)
		{
		imgIndex = j;
		if(j == m_mainLightIndex)
		{
		imgIndex = eImg_BufCnt - 1;
		}
		}	
		else
		{
		imgIndex = j + 1;
		}
		}*/
	}
	else if(m_fovImage_insp)
	{
		m_procMil->StrAllocResult();	// LMJ 2014/11/18
		if(AllocString() == e_NG){
			DecisionOcrInsp(e_NG);
			return e_NG;
		}

		for(int stepID = 0; stepID < maxStepCnts; stepID++)
		{	
			ret = ProcStep(stepID);

			//stepID++;	
			if(ret == e_OK) 
				break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG 
	if(ret == e_OK)
	{
		if(m_result.isReverse)
			m_procMil->DrawString(m_pInspImg_rotate, m_roiSizeX, m_roiSizeY);
		else
			m_procMil->DrawString(m_pInspImg, m_roiSizeX, m_roiSizeY);
	}
#endif
	//////////////////////////////////////////////////////////////////////////

	DecisionOcrInsp();

	if(m_pInspParam->m_bUseOCRNG == TRUE)
	{
		if(ret == e_NG)
			ret = e_OK;
		else
			ret = e_NG;
	}
	else
	{
		if(ret == e_OK)
			ret = m_result.ok;
	}

	//UnPreprocess();

	return ret;
}

int CPInsp_OCR::ProcStep(int stepID)
{
	int ret  = e_NG;
	if(bIsPilBuff)
		return e_NG;
	int thresholdMode =  m_pInspParam->threshMode;
	BOOL isPolarity = m_pInspParam->isPolarity;

#if Timecheck
	CQTimer qtm; 
#endif 
	//huj 2014/03/31  ±Ø¼º°Ë»ç¸¦ ÇÒ°æ¿ì´Â 180µµ È¸ÀüÇØ¼­ °Ë»çÇÏÁö ¾Ê´Â´Ù.
	switch(stepID)
	{
	case eOStepID_IMGBUF_ALLOC:
		{
			ImgBuffAlloc();
		}
		break;
	case eOStepID_USERVALUE:// USER_DEFINED (user value)
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			ret = UserValueInsp(FALSE);
		}
		break;
	case eOStepID_USERVALUE_REV: 
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			if(isPolarity == FALSE)
				ret = UserValueInsp(TRUE);			
		}
		break;
	case eOStepID_AUTO:// LOCAL -> LOCAL_WITH_RESEG -> USER_DEFINED (auto value for matrox)
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			ret = AutoValueInsp(FALSE);
		}
		break;
	case eOStepID_PEMVALUE: //USER_DEFINED (auto value for pemtron)
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			ret = PemtoValueInsp(FALSE);
		}
		break;
	case eOStepID_AUTO_REV: 
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			if(isPolarity == FALSE)
				ret = AutoValueInsp(TRUE);
		}
		break;
	case eOStepID_PEMVALUE_REV:
		{	
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			if(isPolarity == FALSE)
				ret = PemtoValueInsp(TRUE);
		}
		break;
		//LJH 2016.05.27 추가
	case eOStepID_MIX_PEMVALUE:
		{
			//LJH 2016.06.09	추가 영상이 있는 경우
			int nType = m_pInspParam->m_nUseImageMixType;
			if (m_pInspParam->m_nImageMixCount > 0)
			{
				if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MIX_INSP))		ret = PemToValueInsp_ImageMix(FALSE);			//영상을 합쳐서 사용할 경우
				if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MULTI_INSP))	ret = PemToValueInsp_ImageMulti(FALSE, FALSE);	//추가한 영상을 여러번 검사할 경우
				if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MULTI_INSP))	ret = PemToValueInsp_ImageMulti(FALSE, TRUE);	//추가한 영상을 여러번 검사할 경우
			}
		}
		break;

		///LJH 2016.05.27
	case eOStepID_MIX_PEMVALUE_REV:
		{
			if(isPolarity == FALSE)
			{
				//LJH 2016.06.09	추가 영상이 있는 경우
				int nType = m_pInspParam->m_nUseImageMixType;
				if (m_pInspParam->m_nImageMixCount > 0)
				{
					if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MIX_INSP))		ret = PemToValueInsp_ImageMix(TRUE);			//영상을 합쳐서 사용할 경우
					if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MULTI_INSP))	ret = PemToValueInsp_ImageMulti(TRUE, FALSE);	//추가한 영상을 여러번 검사할 경우
					if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MULTI_INSP))	ret = PemToValueInsp_ImageMulti(TRUE, TRUE);	//추가한 영상을 여러번 검사할 경우
				}
			}
		}
		break;
	}
	return ret;
}

void CPInsp_OCR::GetLightArray(int mainLight, int* retArr, int* retCount)
{
	if(retArr == NULL || mainLight < 0 || mainLight >= eImg_BufCnt)
		return;

	switch(mainLight)
	{
	case eM2C_TR:
		{
			retArr[0] = GetLightIndex(eM2C_TR);
			retArr[1] = GetLightIndex(eM2C_TG);
			retArr[2] = GetLightIndex(eM2C_TB);
			*retCount = 3;
		}
		break;
	case eM2C_TG:
		{
			retArr[0] = GetLightIndex(eM2C_TG);
			retArr[1] = GetLightIndex(eM2C_TR);
			retArr[2] = GetLightIndex(eM2C_TB);
			*retCount = 3;
		}
		break;
	case eM2C_TB:
		{
			retArr[0] = GetLightIndex(eM2C_TB);
			retArr[1] = GetLightIndex(eM2C_TR);
			retArr[2] = GetLightIndex(eM2C_TG);
			*retCount = 3;
		}
		break;
	case eM2C_MR:
		{
			retArr[0] = GetLightIndex(eM2C_MR);
			retArr[1] = GetLightIndex(eM2C_MB);

			retArr[2] = GetLightIndex(eM2C_BR);
			retArr[3] = GetLightIndex(eM2C_BB);
			*retCount = 4;
		}
		break;
	case eM2C_MB:
		{
			retArr[0] = GetLightIndex(eM2C_MB);
			retArr[1] = GetLightIndex(eM2C_MR);

			retArr[2] = GetLightIndex(eM2C_BR);
			retArr[3] = GetLightIndex(eM2C_BB);
			*retCount = 4;
		}
		break;
	case eM2C_BR: 
		{
			retArr[0] = GetLightIndex(eM2C_BR);
			retArr[1] = GetLightIndex(eM2C_BB);

			retArr[2] = GetLightIndex(eM2C_MR);
			retArr[3] = GetLightIndex(eM2C_MB);	
			*retCount = 4;
		}
		break;
	case eM2C_BB:
		{
			retArr[0] = GetLightIndex(eM2C_BB);
			retArr[1] = GetLightIndex(eM2C_BR);

			retArr[2] = GetLightIndex(eM2C_MR);
			retArr[3] = GetLightIndex(eM2C_MB);	
			*retCount = 4;
		}
		break;
	}
}

int CPInsp_OCR::ImgBuffAlloc()
{
	int ret  = e_OK;

	int cx = m_inspCoordinate.cx;
	int cy = m_inspCoordinate.cy;
	int sizeX = m_inspCoordinate.width;
	int sizeY = m_inspCoordinate.length;
	int partArea = sizeX * sizeY;
	int wndCx = m_inspCoordinate.bdrCx;
	int wndCy = m_inspCoordinate.bdrCy;
	int wndSizeX = m_inspCoordinate.bdrWidth;
	int wndSizeY = m_inspCoordinate.bdrLength;
	int wndArea = wndSizeX * wndSizeY;
	double wndAngle = m_inspCoordinate.angle;
	double fontAngle = m_pInspParam->fontAngle;
	if(IsAnyAngle(wndAngle))
		wndAngle = 0;

	int bdCx = m_inspCoordinate.anyAngleCx;
	int bdCy = m_inspCoordinate.anyAngleCy;
	int bdSizeX = m_inspCoordinate.anyAngleWidth;
	int bdSizeY = m_inspCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	//shk 20140421
	BOOL IsRemoveTmn = m_pInspParam->IsRemoveTmn;

	if(m_fovImage_insp == NULL || cx <= 0 || cy <= 0 || sizeX <= 0 || sizeY <= 0 || wndCx <= 0 || wndCy <= 0 || wndSizeX <= 0 || wndSizeY <= 0)
		return e_NG;

	//LJH 2016.05.26 ADD
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
	{
		if(m_fovImage_insp_Mix[iLoopCount] == NULL || cx <= 0 || cy <= 0 || sizeX <= 0 || sizeY <= 0 || wndCx <= 0 || wndCy <= 0 || wndSizeX <= 0 || wndSizeY <= 0)
			return e_NG;
	}
	//////////////////////////////////////////////////////////////////////////

	DeleteImgBuf();

//	m_pWndImg = new UCHAR[m_roiSizeX*m_roiSizeY];
	/*m_pInspImg = new UCHAR[m_roiSizeX*m_roiSizeY];
	m_pInspImg_rotate = new UCHAR[m_roiSizeX*m_roiSizeY];*/
	m_pInspImg = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	m_pInspImg_rotate = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	//LJH 2016.05.20 OCR 검사시 영상 취합할 버퍼 두개
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
	{
//		m_pWndImg_Mix[iLoopCount] = new UCHAR[m_roiSizeX*m_roiSizeY];
		/*m_pInspImg_Mix[iLoopCount] = new UCHAR[m_roiSizeX*m_roiSizeY];
		m_pInspImg_Mix_rotate[iLoopCount] = new UCHAR[m_roiSizeX*m_roiSizeY];*/
		m_pInspImg_Mix[iLoopCount] = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
		m_pInspImg_Mix_rotate[iLoopCount] = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	}

	//////////////////////////////////////////////////////////////////////////
	UCHAR* pInspImgTemp = NULL;
	UCHAR* pInspImgTemp_Mix[2] = {NULL, };	//LJH 2016.05.26 Mix 할 버퍼 추가

	//BOOL isAnyAngle = IsAnyAngle(wndAngle);
	//if(isAnyAngle)
	//{
	//	double rotAngle = CalcRotateAngle(wndAngle);

	//	int partW = 0;
	//	int partL = 0;
	//	UCHAR* bdryImgTemp_part = NULL;
	//	m_procMil->ProcAnyAngle_Img(m_fovImage_insp, &bdryImgTemp_part, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &partW, &partL);
	//	m_procMil->SaveWorkImg(bdryImgTemp_part, partW, partL, _T("OCR_boundaryImg.bmp"));

	//	wndAngle = 0;
	//	cx -= (int)(bdCx - (partW / 2.0));
	//	cy -= (int)(bdCy - (partL / 2.0));

	//	pInspImgTemp = new UCHAR[partArea];	
	//	m_procMil->GetClipImage(bdryImgTemp_part, partW, partL, pInspImgTemp, cx, cy, sizeX, sizeY);
	//	m_procMil->SaveWorkImg(pInspImgTemp, sizeX, sizeY, _T("OCR_clipPartImg.bmp"));
	//	
	//	wndCx -= (int)(bdCx - (partW / 2.0));
	//	wndCy -= (int)(bdCy - (partL / 2.0));

	//	m_procMil->GetClipImage(bdryImgTemp_part, partW, partL, m_pWndImg, wndCx, wndCy, wndSizeX, wndSizeY);
	//	m_procMil->SaveWorkImg(m_pWndImg, wndSizeX, wndSizeY, _T("OCR_clipWndImg.bmp"));

	//	delete bdryImgTemp_part;


	//}
	//else
	//{
	//	//main image(part area)
	//	pInspImgTemp = new UCHAR[partArea];
	//	m_procMil->GetClipBuff(m_fovImage_insp, pInspImgTemp, cx, cy, sizeX, sizeY);

	//	//window image
	//	m_procMil->GetClipBuff(m_fovImage_insp, m_pWndImg, wndCx, wndCy, wndSizeX, wndSizeY);
	//}

	//pInspImgTemp = new UCHAR[m_roiSizeX*m_roiSizeY];
	pInspImgTemp = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	m_procMil->GetClipBuff_LT(m_fovImage_insp, pInspImgTemp, 0, 0, m_roiSizeX, m_roiSizeY);
//	m_procMil->GetClipBuff_LT(m_fovImage_insp, m_pWndImg, 0, 0, m_roiSizeX, m_roiSizeY);

	//LJH 2016.05.26 Mix Buffer 할당
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
	{
		//pInspImgTemp_Mix[iLoopCount] = new UCHAR[m_roiSizeX*m_roiSizeY];
		pInspImgTemp_Mix[iLoopCount] = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
		m_procMil->GetClipBuff_LT(m_fovImage_insp_Mix[iLoopCount], pInspImgTemp_Mix[iLoopCount], 0, 0, m_roiSizeX,m_roiSizeY);
//		m_procMil->GetClipBuff_LT(m_fovImage_insp_Mix[iLoopCount], m_pWndImg_Mix[iLoopCount], 0, 0, m_roiSizeX, m_roiSizeY);
	}
	//////////////////////////////////////////////////////////////////////////

	double angle = CalcAngle(fontAngle, wndAngle, FALSE);

	//2016.06.14 Trunk에선 주석이 풀려있음
	//0, 90, 180, 270 각도를 제외한 각도가 들어가면 Rotate 한 이미지에서 메모리 에러 발생하는 듯
	if((angle != 0) && (angle != 90) && (angle != 180) && (angle != 270))
		angle = 0;

	int nRoiSizeX_ori = m_roiSizeX;
	int nRoiSizeY_ori = m_roiSizeY;

	//main image(part area)
	m_procMil->RotateImg_ipp(pInspImgTemp, nRoiSizeX_ori, nRoiSizeY_ori, angle, &m_pInspImg, &m_roiSizeX, &m_roiSizeY);	// LMJ 2013/11/25

	//LJH 2016.05.26 ADD
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
		m_procMil->RotateImg_ipp(pInspImgTemp_Mix[iLoopCount], nRoiSizeX_ori, nRoiSizeY_ori, angle, &(m_pInspImg_Mix[iLoopCount]), &m_roiSizeX, &m_roiSizeY);

	m_procMil->SaveWorkImg(pInspImgTemp, m_roiSizeX, m_roiSizeY, _T("pInspImgTemp0.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) m_procMil->SaveWorkImg(pInspImgTemp_Mix[0], m_roiSizeX, m_roiSizeY, _T("pInspImgTemp1.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) m_procMil->SaveWorkImg(pInspImgTemp_Mix[1], m_roiSizeX, m_roiSizeY, _T("pInspImgTemp2.bmp"));

	//shk 20140421
	int foreGround = m_pInspParam->foreGroundColor;
	if (IsRemoveTmn == TRUE)
		m_procMil->TopHatFiltering(m_pInspImg,m_pInspImg,m_roiSizeX,m_roiSizeY,foreGround);

	//LJH 2016.05.26
	if (IsRemoveTmn == TRUE)
	{
		for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
		{
			int nForeGround = m_pInspParam->AlgoMixCon[iLoopCount].m_nForeGround;
			//nForeGround = 1;
			m_procMil->TopHatFiltering(m_pInspImg_Mix[iLoopCount], m_pInspImg_Mix[iLoopCount], m_roiSizeX, m_roiSizeY, nForeGround);
		}
	}

	m_procMil->SaveWorkImg(m_pInspImg, m_roiSizeX, m_roiSizeY, _T("OCR1.bmp"));



	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) m_procMil->SaveWorkImg(m_pInspImg_Mix[0], m_roiSizeX, m_roiSizeY, _T("OCR_Mix1_1.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) m_procMil->SaveWorkImg(m_pInspImg_Mix[1], m_roiSizeX, m_roiSizeY, _T("OCR_Mix2_1.bmp"));

	//delete pInspImgTemp;
	g_pMManager->pem_delete(pInspImgTemp, false);
	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) //delete pInspImgTemp_Mix[0];
		g_pMManager->pem_delete(pInspImgTemp_Mix[0], false);
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) //delete pInspImgTemp_Mix[1];
		g_pMManager->pem_delete(pInspImgTemp_Mix[1], false);

	//rotate image(part area)
	m_procMil->RotateImg_ipp(m_pInspImg, m_roiSizeX, m_roiSizeY, 180, &m_pInspImg_rotate);		// LMJ 2013/11/25
	m_procMil->SaveWorkImg(m_pInspImg_rotate, m_roiSizeX, m_roiSizeY, _T("OCR2.bmp"));

	//LJH 2016.05.26
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
		m_procMil->RotateImg_ipp(m_pInspImg_Mix[iLoopCount], m_roiSizeX, m_roiSizeY, 180,&(m_pInspImg_Mix_rotate[iLoopCount]));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) m_procMil->SaveWorkImg(m_pInspImg_Mix_rotate[0], m_roiSizeX, m_roiSizeY, _T("OCR_Mix1_2.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) m_procMil->SaveWorkImg(m_pInspImg_Mix_rotate[1], m_roiSizeX, m_roiSizeY, _T("OCR_Mix2_2.bmp"));

	//window image
//	m_pemtoThreshold[0] = m_procMil->GetOtsuThreshold(m_pWndImg, m_roiSizeX,m_roiSizeY);
// 	m_procMil->SaveWorkImg(m_pWndImg, m_roiSizeX,m_roiSizeY, _T("OCR3.bmp"));
// 
// 	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
// 		m_pemtoThreshold[iLoopCount + 1] = m_procMil->GetOtsuThreshold(m_pWndImg_Mix[iLoopCount], m_roiSizeX,m_roiSizeY);
// 	if (m_pInspAlgoOCR->m_nImageMixCount >= 1)	m_procMil->SaveWorkImg(m_pWndImg_Mix[0], m_roiSizeX,m_roiSizeY, _T("OCR_Mix1_3.bmp"));
// 	if (m_pInspAlgoOCR->m_nImageMixCount >= 2)	m_procMil->SaveWorkImg(m_pWndImg_Mix[1], m_roiSizeX,m_roiSizeY, _T("OCR_Mix2_3.bmp"));

	return ret;
}
int CPInsp_OCR::ImgBuffAlloc2()
{
	int ret  = e_OK;

	int cx = m_inspCoordinate.cx;
	int cy = m_inspCoordinate.cy;
	int sizeX = m_inspCoordinate.width;
	int sizeY = m_inspCoordinate.length;
	int partArea = sizeX * sizeY;
	int wndCx = m_inspCoordinate.bdrCx;
	int wndCy = m_inspCoordinate.bdrCy;
	int wndSizeX = m_inspCoordinate.bdrWidth;
	int wndSizeY = m_inspCoordinate.bdrLength;
	int wndArea = wndSizeX * wndSizeY;
	double wndAngle = m_inspCoordinate.angle;
	double fontAngle = m_pInspParam->fontAngle;
	if(IsAnyAngle(wndAngle))
		wndAngle = 0;

	int bdCx = m_inspCoordinate.anyAngleCx;
	int bdCy = m_inspCoordinate.anyAngleCy;
	int bdSizeX = m_inspCoordinate.anyAngleWidth;
	int bdSizeY = m_inspCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("ImgBuffAlloc0.bmp"));
	//shk 20140421
	BOOL IsRemoveTmn = m_pInspParam->IsRemoveTmn;

	if(m_fovImage_insp == NULL || cx <= 0 || cy <= 0 || sizeX <= 0 || sizeY <= 0 || wndCx <= 0 || wndCy <= 0 || wndSizeX <= 0 || wndSizeY <= 0)
		return e_NG;

	//LJH 2016.05.26 ADD
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
	{
		if(m_fovImage_insp_Mix[iLoopCount] == NULL || cx <= 0 || cy <= 0 || sizeX <= 0 || sizeY <= 0 || wndCx <= 0 || wndCy <= 0 || wndSizeX <= 0 || wndSizeY <= 0)
			return e_NG;
	}
	//////////////////////////////////////////////////////////////////////////

// 	DeleteImgBuf();
// 
// 	//m_pWndImg = new UCHAR[m_roiSizeX*m_roiSizeY];
// 	m_pInspImg = new UCHAR[m_roiSizeX*m_roiSizeY];
// 	m_pInspImg_rotate = new UCHAR[m_roiSizeX*m_roiSizeY];
// 
// 	//LJH 2016.05.20 OCR 검사시 영상 취합할 버퍼 두개
// 	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
// 	{
// 		//m_pWndImg_Mix[iLoopCount] = new UCHAR[m_roiSizeX*m_roiSizeY];
// 		m_pInspImg_Mix[iLoopCount] = new UCHAR[m_roiSizeX*m_roiSizeY];
// 		m_pInspImg_Mix_rotate[iLoopCount] = new UCHAR[m_roiSizeX*m_roiSizeY];
// 	}

	//////////////////////////////////////////////////////////////////////////
	UCHAR* pInspImgTemp = NULL;
	UCHAR* pInspImgTemp_Mix[2] = {NULL,NULL };	//LJH 2016.05.26 Mix 할 버퍼 추가

	//BOOL isAnyAngle = IsAnyAngle(wndAngle);
	//if(isAnyAngle)
	//{
	//	double rotAngle = CalcRotateAngle(wndAngle);

	//	int partW = 0;
	//	int partL = 0;
	//	UCHAR* bdryImgTemp_part = NULL;
	//	m_procMil->ProcAnyAngle_Img(m_fovImage_insp, &bdryImgTemp_part, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &partW, &partL);
	//	m_procMil->SaveWorkImg(bdryImgTemp_part, partW, partL, _T("OCR_boundaryImg.bmp"));

	//	wndAngle = 0;
	//	cx -= (int)(bdCx - (partW / 2.0));
	//	cy -= (int)(bdCy - (partL / 2.0));

	//	pInspImgTemp = new UCHAR[partArea];	
	//	m_procMil->GetClipImage(bdryImgTemp_part, partW, partL, pInspImgTemp, cx, cy, sizeX, sizeY);
	//	m_procMil->SaveWorkImg(pInspImgTemp, sizeX, sizeY, _T("OCR_clipPartImg.bmp"));
	//	
	//	wndCx -= (int)(bdCx - (partW / 2.0));
	//	wndCy -= (int)(bdCy - (partL / 2.0));

	//	m_procMil->GetClipImage(bdryImgTemp_part, partW, partL, m_pWndImg, wndCx, wndCy, wndSizeX, wndSizeY);
	//	m_procMil->SaveWorkImg(m_pWndImg, wndSizeX, wndSizeY, _T("OCR_clipWndImg.bmp"));

	//	delete bdryImgTemp_part;


	//}
	//else
	//{
	//	//main image(part area)
	//	pInspImgTemp = new UCHAR[partArea];
	//	m_procMil->GetClipBuff(m_fovImage_insp, pInspImgTemp, cx, cy, sizeX, sizeY);

	//	//window image
	//	m_procMil->GetClipBuff(m_fovImage_insp, m_pWndImg, wndCx, wndCy, wndSizeX, wndSizeY);
	//}

// 	pInspImgTemp = new UCHAR[m_roiSizeX*m_roiSizeY];
// 	m_procMil->GetClipBuff_LT(m_fovImage_insp, pInspImgTemp, 0, 0, m_roiSizeX, m_roiSizeY);
// 	//m_procMil->GetClipBuff_LT(m_fovImage_insp, m_pWndImg, 0, 0, m_roiSizeX, m_roiSizeY);
// 
// 	//LJH 2016.05.26 Mix Buffer 할당
// 	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
// 	{
// 		pInspImgTemp_Mix[iLoopCount] = new UCHAR[m_roiSizeX*m_roiSizeY];
// 		m_procMil->GetClipBuff_LT(m_fovImage_insp_Mix[iLoopCount], pInspImgTemp_Mix[iLoopCount], 0, 0, m_roiSizeX,m_roiSizeY);
// 		//m_procMil->GetClipBuff_LT(m_fovImage_insp_Mix[iLoopCount], m_pWndImg_Mix[iLoopCount], 0, 0, m_roiSizeX, m_roiSizeY);
// 	}

	//////////////////////////////////////////////////////////////////////////
// 
// 	double angle = CalcAngle(fontAngle, wndAngle, FALSE);
// 
// 	//2016.06.14 Trunk에선 주석이 풀려있음
// 	//0, 90, 180, 270 각도를 제외한 각도가 들어가면 Rotate 한 이미지에서 메모리 에러 발생하는 듯
// 	if((angle != 0) && (angle != 90) && (angle != 180) && (angle != 270))
// 		angle = 0;
// 
// 	int nRoiSizeX_ori = m_roiSizeX;
// 	int nRoiSizeY_ori = m_roiSizeY;
// 
// 	//main image(part area)
// 	m_procMil->RotateImg_ipp(pInspImgTemp, nRoiSizeX_ori, nRoiSizeY_ori, angle, &m_pInspImg, &m_roiSizeX, &m_roiSizeY);	// LMJ 2013/11/25
// 
// 	//LJH 2016.05.26 ADD
// 	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
// 		m_procMil->RotateImg_ipp(pInspImgTemp_Mix[iLoopCount], nRoiSizeX_ori, nRoiSizeY_ori, angle, &(m_pInspImg_Mix[iLoopCount]), &m_roiSizeX, &m_roiSizeY);
// 
// 	m_procMil->SaveWorkImg(m_pInspImg, m_roiSizeX, m_roiSizeY, _T("pInspImgTemp0.bmp"));
// 	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) m_procMil->SaveWorkImg(m_pInspImg_Mix[0], m_roiSizeX, m_roiSizeY, _T("pInspImgTemp1.bmp"));
// 	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) m_procMil->SaveWorkImg(m_pInspImg_Mix[1], m_roiSizeX, m_roiSizeY, _T("pInspImgTemp2.bmp"));

	m_procMil->SaveWorkImg(m_pInspImg, m_roiSizeX, m_roiSizeY, _T("OCR0.bmp"));
	//shk 20140421
	int foreGround = m_pInspParam->foreGroundColor;
	if (IsRemoveTmn == TRUE)
		m_procMil->TopHatFiltering(m_pInspImg,m_pInspImg,m_roiSizeX,m_roiSizeY,foreGround);

	//LJH 2016.05.26
	if (IsRemoveTmn == TRUE)
	{
		for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
		{
			int nForeGround = m_pInspParam->AlgoMixCon[iLoopCount].m_nForeGround;
			//nForeGround = 1;
			m_procMil->TopHatFiltering(m_pInspImg_Mix[iLoopCount], m_pInspImg_Mix[iLoopCount], m_roiSizeX, m_roiSizeY, nForeGround);
		}
	}

	m_procMil->SaveWorkImg(m_pInspImg, m_roiSizeX, m_roiSizeY, _T("OCR1.bmp"));



	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) m_procMil->SaveWorkImg(m_pInspImg_Mix[0], m_roiSizeX, m_roiSizeY, _T("OCR_Mix1_1.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) m_procMil->SaveWorkImg(m_pInspImg_Mix[1], m_roiSizeX, m_roiSizeY, _T("OCR_Mix2_1.bmp"));

	//delete pInspImgTemp;
	g_pMManager->pem_delete(pInspImgTemp, false);
	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) //delete pInspImgTemp_Mix[0];
		g_pMManager->pem_delete(pInspImgTemp_Mix[0], false);
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) //delete pInspImgTemp_Mix[1];
		g_pMManager->pem_delete(pInspImgTemp_Mix[1], false);

	//rotate image(part area)
	m_procMil->RotateImg_ipp(m_pInspImg, m_roiSizeX, m_roiSizeY, 180, &m_pInspImg_rotate);		// LMJ 2013/11/25
	m_procMil->SaveWorkImg(m_pInspImg_rotate, m_roiSizeX, m_roiSizeY, _T("OCR2.bmp"));

	//LJH 2016.05.26
	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
		m_procMil->RotateImg_ipp(m_pInspImg_Mix[iLoopCount], m_roiSizeX, m_roiSizeY, 180,&(m_pInspImg_Mix_rotate[iLoopCount]));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 1) m_procMil->SaveWorkImg(m_pInspImg_Mix_rotate[0], m_roiSizeX, m_roiSizeY, _T("OCR_Mix1_2.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2) m_procMil->SaveWorkImg(m_pInspImg_Mix_rotate[1], m_roiSizeX, m_roiSizeY, _T("OCR_Mix2_2.bmp"));

	//window image
	m_pemtoThreshold[0] = m_procMil->GetOtsuThreshold(m_pInspImg, m_roiSizeX,m_roiSizeY);
	//m_procMil->SaveWorkImg(m_pInspImg_rotate, m_roiSizeX,m_roiSizeY, _T("OCR3.bmp"));

	for (int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
		m_pemtoThreshold[iLoopCount + 1] = m_procMil->GetOtsuThreshold(m_pInspImg_Mix[iLoopCount], m_roiSizeX,m_roiSizeY);
	if (m_pInspAlgoOCR->m_nImageMixCount >= 1)	m_procMil->SaveWorkImg(m_pInspImg_Mix[0], m_roiSizeX,m_roiSizeY, _T("OCR_Mix1_3.bmp"));
	if (m_pInspAlgoOCR->m_nImageMixCount >= 2)	m_procMil->SaveWorkImg(m_pInspImg_Mix[1], m_roiSizeX,m_roiSizeY, _T("OCR_Mix2_3.bmp"));

	return ret;
}

int CPInsp_OCR::AllocString()
{
	int ret  = e_OK;
	if(bIsPilBuff)
		return e_NG;

	if(Preprocess() == ePART_FAIL)
		return e_NG;

	return ret;

}

int CPInsp_OCR::UserValueInsp(BOOL reverse)
{
	int ret  = e_OK;
	int strRstTemp = eSTR_FAIL;

	int threshold = m_pInspParam->threshVal;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	UCHAR* procImg = NULL;
	if(!reverse)
		procImg = m_pInspImg;
	else
		procImg = m_pInspImg_rotate;

	//2016.06.13 이전 코드
	SetThresholdMode(eSTR_USER_DEFINED, threshold);
	/*
	//////////////////////////////////////////////////////////////////////////
	//LJH 2016.06.13
	//String Reader를 사용 하기 전에 이진화를 하여 검사 진행
	int nForeGround = 0;
	if (m_pInspParam->foreGroundColor == 1)		nForeGround = 0;
	else if (m_pInspParam->foreGroundColor == 0) nForeGround = 1;
	Binarize(procImg, m_roiSizeX, m_roiSizeY, 2, m_pInspParam->threshVal, nForeGround, procImg);

	SetThresholdMode(eSTR_USER_DEFINED, 0);		//LJH 2016.06.13 Threshold 값은 이미 이진화를 했으므로 0으로 사용
	*/
	for(int i = 0 ; i <= m_nThickenCnt ; i++)
	{
		m_procMil->SetThickenChar(i);
		strRstTemp = SearchString(procImg, m_roiSizeX, m_roiSizeY);
		if(strRstTemp > 0)	break;
		
	}

	m_inspCount++; //display

	if(strRstTemp == 0)
	{
		ret = e_NG;
	}
	else
	{
		if(!CheckScore(useCharScore, stdScore, stdCharCount, bUsePassFont, nPassFontCnt))
			ret = e_NG;
		else
		{
			BOOL isStringOK = FALSE;
			Decision(m_procMil->GetString(), &isStringOK);	// 타겟 문자열과 찾은 문자열이 같은지 체크
			if(!isStringOK)		
				ret = e_NG;
		}
	}

//////////////////////////////////////////////////////////////////////////
	if(ret == e_OK && reverse == TRUE)
		m_result.isReverse = TRUE;


	m_result.isInsp = TRUE;

	return ret;
}
int CPInsp_OCR::UserValueInsp2(BOOL reverse)
{
	int ret  = e_NG;
	int strRstTemp = eSTR_FAIL;

	int threshold = m_pInspParam->threshVal;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	UCHAR* procImg = NULL;
	if(!reverse)
		procImg = m_pInspImg;
	else
		procImg = m_pInspImg_rotate;


	SetThresholdMode(eSTR_USER_DEFINED, threshold);
	for(int i = 0 ; i <= m_nThickenCnt; i++)
	{
		m_procMil->SetThickenChar(i);
		strRstTemp = SearchString(procImg, m_roiSizeX, m_roiSizeY);

		if(strRstTemp >0)
		{
			ret = OKNGConfirm( strRstTemp, threshold);
			if(ret != e_NG)
			{
				m_nUserValueinsp++;
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	if(ret == e_OK && reverse == TRUE)
		m_result.isReverse = TRUE;


	m_result.isInsp = TRUE;

	m_inspCount++; //display

	return ret;
}

int CPInsp_OCR::AutoValueInsp(BOOL reverse)
{
	int ret  = e_OK;
	int strRstTemp = eSTR_FAIL;

	int threshold = m_pInspParam->threshVal;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	UCHAR* procImg = NULL;
	if(!reverse)
		procImg = m_pInspImg;
	else
		procImg = m_pInspImg_rotate;

	bool scoreFlag = false;

	for(int i = 0; i < eSTR_THRESMODE_COUNT; i++)  
	{
		SetThresholdMode(i); 

		for(int j = 0 ; j <= m_nThickenCnt ; j++)
		{
			m_procMil->SetThickenChar(j);
			strRstTemp = SearchString(procImg, m_roiSizeX, m_roiSizeY);
			if(strRstTemp > 0)	break;
		}

		if(strRstTemp > 0)
		{	
			if(CheckScore(useCharScore, stdScore, stdCharCount, bUsePassFont, nPassFontCnt))
			{

				BOOL isStringOK = FALSE;
				Decision(m_procMil->GetString(), &isStringOK);	// 타겟 문자열과 찾은 문자열이 같은지 체크
				if(isStringOK)		
				{
					scoreFlag = false;
					break;
				}
			}
			else
			{
				scoreFlag = true;
			}
		}	
	}
	m_inspCount++; //display

	if(strRstTemp == 0 || scoreFlag)
		ret = e_NG;

	//////////////////////////////////////////////////////////////////////////


	if(ret == e_OK && reverse	== TRUE)
		m_result.isReverse = TRUE;


	m_result.isInsp = TRUE;

	return ret;
}
int CPInsp_OCR::RESEGValueInsp(BOOL reverse)
{
	int ret  = e_NG;
	int strRstTemp = eSTR_FAIL;

	int threshold = m_pInspParam->threshVal;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	UCHAR* procImg = NULL;
	if(!reverse)
		procImg = m_pInspImg;
	else
		procImg = m_pInspImg_rotate;

	bool scoreFlag = false;

// 	for(int i = 0; i < eSTR_THRESMODE_COUNT; i++)  
// 	{
		SetThresholdMode(eSTR_LOCAL_WITH_RESEG); 

		for(int j = 0 ; j <= m_nThickenCnt ; j++)
		{
			m_procMil->SetThickenChar(j);
			strRstTemp = SearchString(procImg, m_roiSizeX, m_roiSizeY);
			if(strRstTemp > 0)	
			{	
				for(int index = 0;index<strRstTemp;index++)
				{

					if(CheckScore(useCharScore, stdScore, stdCharCount, bUsePassFont, nPassFontCnt,index))
					{

						BOOL isStringOK = FALSE;
						Decision(m_procMil->GetString(index), &isStringOK);	// 타겟 문자열과 찾은 문자열이 같은지 체크
						if(isStringOK)		
						{
							scoreFlag = false;
							m_nstringIndex = index;
							ret = e_OK;
							break;
						}
					}
					else
					{
						scoreFlag = true;
					}
				}
				if(ret == e_OK)
					break;
			}
		}
//	}
	m_inspCount++; //display

	if(strRstTemp < 1 || scoreFlag)
		ret = e_NG;

	//////////////////////////////////////////////////////////////////////////


	if(ret == e_OK && reverse	== TRUE)
		m_result.isReverse = TRUE;


	m_result.isInsp = TRUE;

	return ret;
}
int CPInsp_OCR::LocalValueInsp(BOOL reverse)
{
	int ret  = e_NG;
	int strRstTemp = eSTR_FAIL;

	int threshold = m_pInspParam->threshVal;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	UCHAR* procImg = NULL;
	if(!reverse)
		procImg = m_pInspImg;
	else
		procImg = m_pInspImg_rotate;

	bool scoreFlag = false;

		SetThresholdMode(eSTR_LOCAL); 

		for(int j = 0 ; j <= m_nThickenCnt ; j++)
		{
			m_procMil->SetThickenChar(j);
			strRstTemp = SearchString(procImg, m_roiSizeX, m_roiSizeY);
			if(strRstTemp > 0)	
			{	
				for(int index = 0;index<strRstTemp;index++)
				{

					if(CheckScore(useCharScore, stdScore, stdCharCount, bUsePassFont, nPassFontCnt,index))
					{

						BOOL isStringOK = FALSE;
						Decision(m_procMil->GetString(index), &isStringOK);	// 타겟 문자열과 찾은 문자열이 같은지 체크
						if(isStringOK)		
						{
							scoreFlag = false;
							m_nstringIndex = index;
							ret = e_OK;
							break;
						}
					}
					else
					{
						scoreFlag = true;
					}
				}
				if(ret == e_OK)
					break;
			}
		}

	m_inspCount++; //display

	if(strRstTemp < 1 || scoreFlag)
		ret = e_NG;

	//////////////////////////////////////////////////////////////////////////


	if(ret == e_OK && reverse	== TRUE)
		m_result.isReverse = TRUE;


	m_result.isInsp = TRUE;

	return ret;
}

int CPInsp_OCR::PemtoValueInsp(BOOL reverse)
{
	int ret  = e_OK;
	int strRstTemp = eSTR_FAIL;

	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	UCHAR* procImg = NULL;
	if(!reverse)
		procImg = m_pInspImg;
	else
		procImg = m_pInspImg_rotate;

	//2016.06.13 이전 코드
	SetThresholdMode(eSTR_USER_DEFINED, m_pemtoThreshold[0]);
	/*
	//////////////////////////////////////////////////////////////////////////
	//LJH 2016.06.13
	//String Reader를 사용 하기 전에 이진화를 하여 검사 진행
	int nForeGround = 0;
	if (m_pInspParam->foreGroundColor == 1)		nForeGround = 0;
	else if (m_pInspParam->foreGroundColor == 0) nForeGround = 1;
	Binarize(procImg, m_roiSizeX, m_roiSizeY, 2, m_pemtoThreshold[0], nForeGround, procImg);

	SetThresholdMode(eSTR_USER_DEFINED, 0);		//LJH 2016.06.13 Threshold 값은 이미 이진화를 했으므로 0으로 사용
	*/
	for(int i = 0 ; i <= m_nThickenCnt ; i++)
	{
		m_procMil->SetThickenChar(i);
		strRstTemp = SearchString(procImg, m_roiSizeX, m_roiSizeY);
		if(strRstTemp > 0)	break;
	}

	m_inspCount++; //display

	if(strRstTemp == 0)
	{
		ret = e_NG;
	}
	else
	{
		if(!CheckScore(useCharScore, stdScore, stdCharCount, bUsePassFont, nPassFontCnt))
			ret = e_NG;
		else
		{
			BOOL isStringOK = FALSE;
			Decision(m_procMil->GetString(), &isStringOK);	// 타겟 문자열과 찾은 문자열이 같은지 체크
			if(!isStringOK)
				ret = e_NG;
		}
	}
//////////////////////////////////////////////////////////////////////////

	if(ret == e_OK && reverse == TRUE)
		m_result.isReverse = TRUE;


	m_result.isInsp = TRUE;

	return ret;
}

//LJH 2016.05.27
int CPInsp_OCR::PemToValueInsp_ImageMix(BOOL reverse)
{
	int ret = e_OK;
	int strRstTemp = eSTR_FAIL;
	int threshold = m_pInspParam->threshVal;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL bUseCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	int nForeGround = 1;
	int nForeGrounds[3] = {1, 1, 1};
	int nThreshold[3] = {0, 0, 0};
	UCHAR* procImg[3] = { NULL, NULL, NULL };
	UCHAR* procImgDest[3] = { NULL, NULL, NULL };
	CString ImageName;

	nForeGrounds[0] = m_pInspParam->foreGroundColor;
	nThreshold[0] = m_pInspParam->threshVal;

	if (!reverse)
		procImg[0] = m_pInspImg;
	else
		procImg[0] = m_pInspImg_rotate;

	for (int iLoopCount = 1; iLoopCount <= m_pInspParam->m_nImageMixCount; ++iLoopCount)
	{
		nThreshold[iLoopCount] = m_pInspParam->AlgoMixCon[iLoopCount - 1].m_nThresholdValue;
		nForeGrounds[iLoopCount] = m_pInspParam->AlgoMixCon[iLoopCount - 1].m_nForeGround;

		if (!reverse)	
			procImg[iLoopCount] = m_pInspImg_Mix[iLoopCount - 1];
		else
			procImg[iLoopCount] = m_pInspImg_Mix_rotate[iLoopCount - 1];

		ImageName.Format(_T("OCR_Origin_%d.bmp"), (iLoopCount + 1));
		m_procMil->SaveWorkImg(procImg[iLoopCount], m_roiSizeX, m_roiSizeY, ImageName);
	}

	//String Reader를 사용 하기 전에 이진화를 하여 검사 진행
	for (int iLoopCount = 0; iLoopCount <= m_pInspParam->m_nImageMixCount; ++iLoopCount)
	{
		//nForeGround = nForeGrounds[iLoopCount];
		if (nForeGrounds[iLoopCount] == 1) nForeGround = 0;
		else if (nForeGrounds[iLoopCount] == 0) nForeGround = 1;

		//procImgDest[iLoopCount] = new UCHAR[m_roiSizeX * m_roiSizeY];
		procImgDest[iLoopCount] = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX * m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
		Binarize(procImg[iLoopCount], m_roiSizeX, m_roiSizeY, 2, nThreshold[iLoopCount], nForeGround, procImgDest[iLoopCount]);

		CString ImageName;
		ImageName.Format(_T("OCR_Binarize_%d.bmp"), (iLoopCount + 1));
		m_procMil->SaveWorkImg(procImgDest[iLoopCount], m_roiSizeX, m_roiSizeY, ImageName);
	}

	//LJH 2016.06.08 Filtering Check
	bool bFiltering = false;
	for (int iLoopCount = 1; iLoopCount <= m_pInspParam->m_nImageMixCount; ++iLoopCount)
	{
		if (nForeGrounds[iLoopCount - 1] == nForeGrounds[iLoopCount])
			bFiltering = true;
		else 
		{
			bFiltering = false;
			break;
		}
	}

	//영상 합치기 
	UCHAR* m_pInspDest = m_pInspImg;
	if(reverse)m_pInspDest = m_pInspImg_rotate;
	if (m_pInspAlgoOCR->m_nImageMixCount >= 1)
		m_pProcMilAlgo->BinarizeImagesCombine(procImgDest, m_pInspDest, m_roiSizeX, m_roiSizeY, m_pInspAlgoOCR->m_nImageMixCount + 1, m_pInspParam->m_nFilterType, FALSE);
	m_procMil->SaveWorkImg(m_pInspDest, m_roiSizeX, m_roiSizeY, _T("MixImage.bmp"));
	
	if (!reverse)	
		ImageName.Format(_T("OCR_Mix_SRC.bmp"));
	else
		ImageName.Format(_T("OCR_Mix_SRC_REV.bmp"));
	m_procMil->SaveWorkImg(m_pInspImg, m_roiSizeX, m_roiSizeY, ImageName);

	ret = UserValueInsp(reverse);

	if(ret == e_NG)
		ret = AutoValueInsp(reverse);

	if(ret == e_NG)
		ret = LocalValueInsp(reverse);
// 
// 	if(ret == e_NG)
// 		ret = PemtoValueInsp(reverse);

	if(ret == e_NG && m_bIsHisto && m_pInspParam->m_bThresholdtracking)
		ret = ThresholdTracking(reverse);


	for (int iLoopCount = 0; iLoopCount < 3; ++iLoopCount)
	{
		//delete procImgDest[iLoopCount];
		g_pMManager->pem_delete(procImgDest[iLoopCount], false);
		procImgDest[iLoopCount] = NULL;
	}

	if(ret != e_NG)m_nPemToValueInsp_ImageMix++;

	return ret;
}

//LJH 2016.06.09
int CPInsp_OCR::PemToValueInsp_ImageMulti(BOOL reverse, BOOL IsAutoThreshold)
{
	int ret = e_NG;
	if(bIsPilBuff)
		return ret;
	int strRstTemp = eSTR_FAIL;
	int threshold[3];
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL bUseCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	int nForeGround = 1;
	int nForeGrounds[3] = {1, 1, 1};
	int nThreshold[3] = {0, 0, 0};
	UCHAR* procImg[3] = { NULL, NULL, NULL };
	// 	UCHAR* procImgDest[3] = { NULL, NULL, NULL };
	CString ImageName;

	bool scoreFlag = false;
	nForeGrounds[0] = m_pInspParam->foreGroundColor;
	if (FALSE == IsAutoThreshold)		nThreshold[0] = m_pInspParam->threshVal;
	else if (TRUE == IsAutoThreshold)	nThreshold[0] = m_pemtoThreshold[0];


	//영상 할당
	if (!reverse)
		procImg[0] = m_pInspImg;
	else
		procImg[0] = m_pInspImg_rotate;

	for (int iLoopCount = 1; iLoopCount <= m_pInspParam->m_nImageMixCount; ++iLoopCount)
	{
		nForeGrounds[iLoopCount] = m_pInspParam->AlgoMixCon[iLoopCount - 1].m_nForeGround;
		//nThreshold[iLoopCount] = m_pInspParam->AlgoMixCon[iLoopCount - 1].m_nThresholdValue;
		if (FALSE == IsAutoThreshold)		nThreshold[iLoopCount] = m_pInspParam->AlgoMixCon[iLoopCount - 1].m_nThresholdValue;
		else if (TRUE == IsAutoThreshold)	nThreshold[iLoopCount] = m_pemtoThreshold[iLoopCount];

		if (!reverse)	
			procImg[iLoopCount] = m_pInspImg_Mix[iLoopCount - 1];
		else
			procImg[iLoopCount] = m_pInspImg_Mix_rotate[iLoopCount - 1];

		ImageName.Format(_T("OCR_Origin_%d.bmp"), (iLoopCount + 1));
		m_procMil->SaveWorkImg(procImg[iLoopCount], m_roiSizeX, m_roiSizeY, ImageName);
	}

	int fontCnts = 0;
	int InspFunc[3];
	InspFunc[0]= m_pInspParam->m_nInspFunc;
	InspFunc[1]= m_pInspParam->AlgoMixCon[0].m_nInspFunc;
	InspFunc[2]= m_pInspParam->AlgoMixCon[1].m_nInspFunc;

	threshold[0] = m_pInspParam->threshVal;
	threshold[1] = m_pInspParam->AlgoMixCon[0].m_nThresholdValue;
	threshold[2] = m_pInspParam->AlgoMixCon[1].m_nThresholdValue;
	if(InspFunc[1] != 2)threshold[1] = -1;
	if(InspFunc[2] != 2)threshold[2] = -1;
	for(int iLoopCount = 0;iLoopCount <= m_pInspParam->m_nImageMixCount; iLoopCount++)
	{
		if (!reverse)	
			ImageName.Format(_T("OCR_continue_SRC_%d.bmp"), (iLoopCount + 1));
		else
			ImageName.Format(_T("OCR_continue_SRC_REV_%d.bmp"), (iLoopCount + 1));
		m_procMil->SaveWorkImg(procImg[iLoopCount], m_roiSizeX, m_roiSizeY, ImageName);

 		if(ret == e_NG)
		{

			if(iLoopCount != 0 && nForeGrounds[iLoopCount-1] != nForeGrounds[iLoopCount])
			{
				if(nForeGrounds[iLoopCount] == 1)
					m_procMil->m_nUsecontinue = 1;
				else 
					m_procMil->m_nUsecontinue = 2;

				LoadStrFont(m_pInspParam->fontPath, &fontCnts, &m_nFontSizeY, m_nInspectionMode);
				if(m_procMil->GetStringModelCount() <=0 || m_procMil->GetStrForeGoundColor() != nForeGrounds[iLoopCount])
					Preprocess(nForeGrounds[iLoopCount]);

			}

			SetThresholdMode(InspFunc[iLoopCount], threshold[iLoopCount]);
			for(int i = 0 ; i <= m_nThickenCnt; i++)
			{
				m_procMil->SetThickenChar(i);
				strRstTemp = SearchString(procImg[iLoopCount], m_roiSizeX, m_roiSizeY);

				if(strRstTemp >0)
				{
					ret = OKNGConfirm( strRstTemp, threshold[iLoopCount]);
					if(ret != e_NG)
					{
						m_nPemToValueInsp_ImageMulti++;
						break;
					}

				}
			}
		}

		if(ret != e_NG)
		{
			break;
		}
	}


	m_procMil->m_nUsecontinue = 0;
	return ret;
}

int CPInsp_OCR::ThresholdTracking(BOOL reverse)
{
	int ret  = e_NG;
	if(bIsPilBuff)
		return ret;
	int strRstTemp = eSTR_FAIL;

	int threshold = m_pInspParam->threshVal;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;
	int nSearchThresold = m_pInspParam->m_searchThreshold;

	UCHAR* procImg = NULL;
	if(!reverse)
		procImg = m_pInspImg;
	else
		procImg = m_pInspImg_rotate;

		if(ret == e_NG)
		{
			SetThresholdMode(eSTR_USER_DEFINED, nSearchThresold);
			for(int i = 0 ; i <= m_nThickenCnt; i++)
			{
				m_procMil->SetThickenChar(i);
				strRstTemp = SearchString(procImg, m_roiSizeX, m_roiSizeY);

				if(strRstTemp > 0)
				{
					ret = OKNGConfirm( strRstTemp, nSearchThresold);
					if(ret != e_NG)
					{
						m_nThresholdTracking++;
						break;
					}
				}
			}

			int nCount = 20;
			int nTerm = -10;
			
			if(m_pInspParam->m_nAutoFlag)
				nCount = 10;
			for(int j=0; j<=nCount;j++)
			{
				if(ret != e_NG)
				{
					m_pInspParam->m_nAutoFlag++;
					break;
				}
				
				if(j%2) nTerm = -10;
				else nTerm = 10;

				nSearchThresold = m_pInspParam->m_searchThreshold+(nTerm*((j+1)/2));
				if(nSearchThresold > 230 || nSearchThresold < 30)
					continue;
				SetThresholdMode(eSTR_USER_DEFINED, nSearchThresold);
				for(int i = 0 ; i <= m_nThickenCnt; i++)
				{
					m_procMil->SetThickenChar(i);
					strRstTemp = SearchString(procImg, m_roiSizeX, m_roiSizeY);
						if(strRstTemp >0)
					{
						ret = OKNGConfirm( strRstTemp, nSearchThresold);
						if(ret != e_NG)
						{
							m_nThresholdTracking++;
							break;
						}
					}
				}
			}
		}
	//////////////////////////////////////////////////////////////////////////
	if(ret == e_OK && reverse == TRUE)
		m_result.isReverse = TRUE;


	m_result.isInsp = TRUE;
	/////////////////////////////////////////////////////////////////////

	m_inspCount++; //display


	return ret;
}


bool CPInsp_OCR::CheckScore(BOOL useCharScore, double stdScore, int charCount, BOOL bUseFontPass, int nFontPassCnt)
{
	if(bIsPilBuff)
		return false;
	bool ret = true;

	if(!useCharScore)
		return ret;

	if(charCount < 1)
		return false;

	double charScore[MAX_STRLEN] = {0.0,};
	m_procMil->GetCharScore(charScore);
	int nFontFailCnt = 0;
	if(nFontPassCnt > charCount)
		nFontPassCnt = charCount - 1;
	for(int i = 0; i < charCount; i++)
	{
		if(charScore[i] < stdScore)
		{
			if(bUseFontPass == TRUE)
			{
				nFontFailCnt++;
				if(nFontFailCnt >= nFontPassCnt)
				{
					ret = false;
					break;
				}
			}
			else
			{
				ret = false;
				break;
			}
		}
	}

	//score¿¡¼­ ½ÇÆÐÇÒ °æ¿ì µ¥ÀÌÅÍ ¹é¾÷
	if(!ret)
	{
		m_procMil->GetStringCount(&m_resultTemp.strCount);
		m_procMil->GetCharCount(&m_resultTemp.charCount);
		m_procMil->GetStringScore(&m_resultTemp.stringScore);
		m_procMil->GetCharScore(m_resultTemp.charScore);
		m_procMil->GetStringPos(&m_resultTemp.strPosX, &m_resultTemp.strPosY);
		m_procMil->GetStringAngle(&m_resultTemp.strAngle);
		m_oldResultFlag = true;
	}
	else
	{		
		m_oldResultFlag = false;
	}

	return ret;
}
bool CPInsp_OCR::CheckScore(BOOL useCharScore, double stdScore, int charCount, BOOL bUseFontPass, int nFontPassCnt, int index)
{
	if(bIsPilBuff)
		return false;
	bool ret = true;

	if(!useCharScore)
		return ret;

	if(charCount < 1)
		return false;

	double charScore[MAX_STRLEN] = {0.0,};
	double x[MAX_STRLEN] = {0.0,};
	double y[MAX_STRLEN] = {0.0,};
	double w[MAX_STRLEN] = {0.0,};
	double h[MAX_STRLEN] = {0.0,};
	double angle[MAX_STRLEN] = {0.0,};
	m_procMil->GetCharScore(charScore,index);
	int nFontFailCnt = 0;
	if(nFontPassCnt > charCount)
		nFontPassCnt = charCount - 1;
	for(int i = 0; i < charCount; i++)
	{
		if(charScore[i] < stdScore)
		{
			if(bUseFontPass == TRUE)
			{
				nFontFailCnt++;
				if(nFontFailCnt >= nFontPassCnt)
				{
					ret = false;
					break;
				}
			}
			else
			{
				ret = false;
				break;
			}
		}
	}

	//score¿¡¼­ ½ÇÆÐÇÒ °æ¿ì µ¥ÀÌÅÍ ¹é¾÷
	if(!ret)
	{
		m_oldResultFlag = true;
	}
	else
	{		
		m_oldResultFlag = false;
	}
	saveResultTemp(index);
	
	return ret;
}

double CPInsp_OCR::CalcAngle(double fontAngle, double wndAngle, BOOL isReverse)
{
	double angle = 0;

	if(wndAngle == 360)
		wndAngle = 0;

	angle = fontAngle + (360 - wndAngle);	
	if(angle >= 360)
		angle -= 360;

	if(isReverse)
	{
		angle = angle + 180;
	}

	if(angle >= 360)
		angle -= 360;

	return angle;
}

void CPInsp_OCR::DecisionOcrInsp(int ret)
{
	if(bIsPilBuff)
		return ;
	BOOL isPolarity = m_pInspParam->isPolarity;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;

	int strCount = 0;
	int charCount = 0;
	double stringScore = 0; 
	double charScore[MAX_STRLEN] = {0.0,};
	double strPosX = 0;
	double strPosY = 0;
	double strAngle = 0;
	CString strTemp = _T("");
	double x[MAX_STRLEN] = {0.0,};
	double y[MAX_STRLEN] = {0.0,};
	double w[MAX_STRLEN] = {0.0,};
	double h[MAX_STRLEN] = {0.0,};
	double angle[MAX_STRLEN] = {0.0,};
	
	//huj 2014/03/25
	if(!m_oldResultFlag && ret == e_OK)
	{
		m_procMil->GetStringCount(&strCount);
		m_procMil->GetCharCount(&charCount);
		m_procMil->GetStringScore(&stringScore,m_nstringIndex);
		m_procMil->GetCharScore(charScore,m_nstringIndex);
		m_procMil->GetStringPos(&strPosX, &strPosY,m_nstringIndex);
		m_procMil->GetStringAngle(&strAngle,m_nstringIndex);
		strTemp = m_procMil->GetString(m_nstringIndex);

		m_procMil->SetImgSize(m_roiSizeX,m_roiSizeY);
		m_procMil->GetCharPos(x, y,w,h,angle, m_nstringIndex,m_bReRotate);
	}
	else
	{
		strCount = m_resultTemp.strCount;
		charCount = m_resultTemp.charCount;
		stringScore = m_resultTemp.stringScore; 
		strPosX = m_resultTemp.strPosX;
		strPosY = m_resultTemp.strPosY;
		strAngle = m_resultTemp.strAngle;
		strTemp = m_resultTemp.str;
		memcpy(charScore,m_resultTemp.charScore,sizeof(double)*MAX_STRLEN);
// 		for(int i = 0; i < charCount; i++)
// 		{
// 			charScore[i] = m_resultTemp.charScore[i];
		// 		}
		memcpy(x,m_resultTemp.charX,sizeof(double)*MAX_STRLEN);
		memcpy(y,m_resultTemp.charY,sizeof(double)*MAX_STRLEN);
		memcpy(w,m_resultTemp.charWidth,sizeof(double)*MAX_STRLEN);
		memcpy(h,m_resultTemp.charHeight,sizeof(double)*MAX_STRLEN);
		memcpy(angle,m_resultTemp.charangle,sizeof(double)*MAX_STRLEN);
	}

	BOOL isStringOK = FALSE;
	Decision(strTemp, &isStringOK);
//////////////////////////////////////////////////////////////////////////

	m_result.strCount = strCount;

	if(strCount<1)m_result.charCount = (charCount/1); //2016.12.22 shkim 예외처리
	else m_result.charCount = (charCount/strCount);
	if(m_result.charCount == 0)
	{
		m_result.stringScore = 0;
		m_result.strPosX = 0;
		m_result.strPosY = 0;
		m_result.strAngle = 0;
		m_result.inspCount = m_inspCount;
		memcpy(m_result.charScore,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
	}
	else
	{
		m_result.stringScore = stringScore;
		m_result.strPosX = strPosX;
		m_result.strPosY = strPosY;
		m_result.strAngle = strAngle;
		m_result.inspCount = m_inspCount;
		memcpy(m_result.charScore,charScore,sizeof(double)*MAX_STRLEN);
	}
// 	for(int i = 0; i < charCount; i++)
// 	{
// 		m_result.charScore[i] = charScore[i];
	// 	}
	memcpy(m_result.charX,x,sizeof(double)*MAX_STRLEN);
	memcpy(m_result.charY,y,sizeof(double)*MAX_STRLEN);
	memcpy(m_result.charWidth,w,sizeof(double)*MAX_STRLEN);
	memcpy(m_result.charHeight,h,sizeof(double)*MAX_STRLEN);
	memcpy(m_result.charAngle,angle,sizeof(double)*MAX_STRLEN);

	wsprintf(m_result.str  ,T2W(strTemp.GetBuffer(0)));
//////////////////////////////////////////////////////////////////////////


	if(strCount > 0)//Ã£À½
	{
		m_result.ok = TRUE;
		m_result.list.existOK = TRUE;

		if(m_result.charCount != stdCharCount) //Ã£¾Ò´Âµ¥ ±ÛÀÚ¼ö ´Ù¸§
		{
			m_result.ok = FALSE;

			m_result.list.charCountOk = FALSE;			
		}
		else
		{
			m_result.list.charCountOk = TRUE;
		}	
		//////////////////////////////////////////////////////////////////////////

		if(isStringOK == FALSE) //Ã£¾Ò´Âµ¥ ¹®ÀÚ°¡ ´Ù¸§
		{
			m_result.ok = FALSE;

			m_result.list.stringOk = FALSE ;
		}
		else
		{
			m_result.list.stringOk = TRUE;
		}
		//////////////////////////////////////////////////////////////////////////
		//huj 2013/12/12
		if(useCharScore == TRUE)
		{
			double minCharScore = charScore[0];
			for(int i = 0; i < m_result.charCount; i++)
			{
				if(charScore[i] <= stdScore)
				{
					m_result.ok = FALSE;
					m_result.list.stringScoreOk = FALSE;

					m_result.charScoreResult[i] = 1;
				}

				if(minCharScore > charScore[i])
				{
					minCharScore = charScore[i];
				}
			}
			if(minCharScore>stdScore)
				m_result.list.stringScoreOk = TRUE;
			m_result.stringScore = minCharScore;
		}

		//////////////////////////////////////////////////////////////////////////

		if(isPolarity == TRUE)
		{
			if(m_result.isReverse == TRUE) //Ã£¾Ò´Âµ¥ ±Ø¼ºÀÌ ¹Ù²ñ
			{
				m_result.ok = FALSE;

				m_result.list.polarityOK = FALSE;
			}
			else
			{
				m_result.list.polarityOK = TRUE;
			}
		}
		else
		{
			m_result.list.polarityOK = TRUE;
		}
	}
	else//¸øÃ£À½
	{
		m_result.ok = FALSE;

		m_result.list.existOK = FALSE;
		m_result.list.charCountOk = FALSE;
		m_result.list.stringOk = FALSE;
		m_result.list.stringScoreOk = FALSE;

		if(isPolarity == TRUE)
			m_result.list.polarityOK = FALSE;
		else
			m_result.list.polarityOK = TRUE;  //±Ø¼º °Ë»ç check°¡ ¾ÈµÇ¾îÀÖÀ¸¸é ºÒ·®ÀÌ¾îµµ ÀÌÇ×¸ñÀº TRUE.
	}

	if(m_pInspParam->m_bUseOCRNG == TRUE)
	{
		if(m_result.ok == FALSE)
			m_result.ok = TRUE;
		else
			m_result.ok = FALSE;

		if(m_result.list.stringScoreOk == FALSE)
			m_result.list.stringScoreOk = TRUE;
		else
			m_result.list.stringScoreOk = FALSE;
	}
}

int CPInsp_OCR::SetFont(BOOL isNew, int* retThresh,int fontCount)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	double cx = m_teachCoordinate.cx;
	double cy = m_teachCoordinate.cy;
	double roiSizeX = m_teachCoordinate.width;
	double roiSizeY = m_teachCoordinate.length;
	int roiArea = roiSizeX * roiSizeY;
	int foreGroundColor = m_pTeachParam->foreGroundColor;
	CString targetString = m_pTeachParam->targetString;
	CString fontPath = m_pTeachParam->fontPath;
	int spaceWidthMode = m_pTeachParam->spaceWidthMode;
	int threshMode = m_pTeachParam->threshMode;
	int threshVal = m_pTeachParam->threshVal;
	double wndAngle = m_teachCoordinate.angle;
	double fontAngle = m_pTeachParam->fontAngle;

	int bdCx = m_teachCoordinate.anyAngleCx;
	int bdCy = m_teachCoordinate.anyAngleCy;
	int bdSizeX = m_teachCoordinate.anyAngleWidth;
	int bdSizeY = m_teachCoordinate.anyAngleLength;
	int bdRoiArea = bdSizeX * bdSizeY;

	if(m_fovImage_teach == NULL || cx < 0 || cy < 0 || roiSizeX < 0 || roiSizeY < 0	|| foreGroundColor < 0 || spaceWidthMode < 0 || targetString[0] == NULL)
		return ePART_FAIL;

	//////////////////////////////////////////////////////////////////////////
	UCHAR* wndImg = NULL;

// 	BOOL isAnyAngle = IsAnyAngle(wndAngle);
// 	if(isAnyAngle)
// 	{
// 		double rotAngle = CalcRotateAngle(wndAngle);
// 
// 		int w = 0;
// 		int l = 0;
// 		UCHAR* bdryImgTemp = NULL;
// 		m_procMil->ProcAnyAngle_Img(m_fovImage_teach, &bdryImgTemp, bdCx, bdCy, bdSizeX, bdSizeY, rotAngle, &w, &l);
// 		m_procMil->SaveWorkImg(bdryImgTemp, w, l, _T("OCR_boundaryImg.bmp"));
// 
// 		wndAngle = 0;
// 		cx -= (int)(bdCx - (w / 2.0));
// 		cy -= (int)(bdCy - (l / 2.0));
// 
// 		wndImg = new UCHAR[roiArea];	
// 		m_procMil->GetClipImage(bdryImgTemp, w, l, wndImg, cx, cy, roiSizeX, roiSizeY);
// 
// 		delete bdryImgTemp;
// 	}
// 	else
	{
		//main image(part area)
		//wndImg = new UCHAR[roiArea];
		wndImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		int nSX = RounD(cx - (roiSizeX / 2.0));
		int nSY = RounD(cy - (roiSizeY / 2.0));
		m_procMil->GetClipBuff_LT(m_fovImage_teach, wndImg, nSX, nSY, roiSizeX, roiSizeY);		
	}
	m_procMil->SaveWorkImg(wndImg, roiSizeX, roiSizeY, _T("OCR_clipPartImg.bmp"));


	//////////////////////////////////////////////////////////////////////////

	int ret = 0;
	int rstThresh = 0;
	
	if(isNew)
	{	
		// LMJ 2013/11/07
		ret = SetStrNewFont(wndImg, roiSizeX, roiSizeY, fontAngle, foreGroundColor, targetString, spaceWidthMode,m_pTeachParam->removeLineCnt, m_pTeachParam->removeLinePos, m_pTeachParam->removeLineWid, threshMode, threshVal, &rstThresh);
	}
	else
	{
// 		int fontCnts = 0;
// 		int isLoad = LoadStrFont(fontPath, &fontCnts);
// 
// 		if(isLoad == eSTR_FAIL)
// 		{
// 			ret = ePROC_OCR_NOTFOUND_FILE;
// 		}
// 		else
// 		{
// 			// LMJ 2013/11/07
// 			ret = SetStrFont(wndImg, roiSizeX, roiSizeY, fontAngle, foreGroundColor, targetString, fontCount, spaceWidthMode, m_pTeachParam->removeLineCnt, m_pTeachParam->removeLinePos, m_pTeachParam->removeLineWid, threshMode, threshVal, &rstThresh);
// 		}
		ret = SetStrFont(wndImg, roiSizeX, roiSizeY, fontAngle, foreGroundColor, targetString, fontCount, spaceWidthMode, m_pTeachParam->removeLineCnt, m_pTeachParam->removeLinePos, m_pTeachParam->removeLineWid, threshMode, threshVal, &rstThresh);
	}

	if(retThresh != NULL)
		*retThresh = rstThresh;


	//delete wndImg;
	g_pMManager->pem_delete(wndImg, false);

	return ret;
}

int CPInsp_OCR::SaveFont(CString path)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	CString temp = _T("");

	if(path.IsEmpty())
	{
		temp = m_pTeachParam->fontPath;
	}
	else
	{
		temp = path;
	}

	m_procMil->DeleteString();
	SaveStrFont(temp);

	return ePART_SUCCESS;
}

int CPInsp_OCR::LoadFont(CString path, int* retFontCount, long* retFontSize)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	int ret = LoadStrFont(path, retFontCount, retFontSize);
	

	if(ret)
		ret = ePART_FAIL;	
	else
		ret = ePART_SUCCESS;

	return ret;
}

int CPInsp_OCR::GetTeachResultImg(UCHAR* Img)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	int fontCount = m_procMil->GetFontCount();

	GetBuffDrawFont(Img,m_pTeachParam->rstImgSizeX,m_pTeachParam->rstImgSizeY, m_pTeachParam->fontAngle, m_pTeachParam->rstFontColor,fontCount-1);
	return ePART_SUCCESS;
}

int CPInsp_OCR::GetSerachResultImg(UCHAR* Img)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	int cx = m_inspCoordinate.cx;
	int cy = m_inspCoordinate.cy;
	int sizeX = m_inspCoordinate.width;
	int sizeY = m_inspCoordinate.length;	
	int margin = m_pInspParam->areaMargin;
	int resizeX = sizeX;// + margin;
	int resizeY = sizeY;// + margin;

	GetBuffDrawString(Img, cx, cy, resizeX, resizeY,
						m_pInspParam->rstImgSizeX, m_pInspParam->rstImgSizeY, m_pInspParam->fontAngle, m_pInspParam->rstStringColor);
	return ePART_SUCCESS;
}


int CPInsp_OCR::GetFontInfo(CString fontPath, CString tempPath, int imgSizeX, int imgSizeY, int* retFontCount, CString OKFont,int color)
{
	int ret = ePART_SUCCESS;
	if(bIsPilBuff)
		return ePART_FAIL;

	int fontCnts = 0;
	int isLoad = LoadStrFont(fontPath, &fontCnts);

	if(isLoad == eSTR_FAIL)
	{
		ret = ePROC_OCR_NOTFOUND_FILE;
	}
	else
	{
		int imgSizeX = 100;
		int imgSizeY = 80;
		int imgArea = imgSizeX * imgSizeY;

		UCHAR* fontImg = NULL;
		//fontImg = new UCHAR[imgArea * 3];
		fontImg = g_pMManager->pem_new<UCHAR>(true, imgArea * 3, (PCHAR)__FUNCTION__, __LINE__);

		for(int i = 0; i < fontCnts; i++)
		{
			CString imgName;
			CString fontCharVal = m_procMil->GetFontCharValue(i);

			imgName.Format(_T("%sfontImg%d.jpg"), tempPath, i);		//jpg 읽어오기

			GetBuffDrawDefineFont(fontImg, imgSizeX,imgSizeY, 0, color, i, fontCharVal);
			m_procMil->SaveTIFImage(fontImg, imgSizeX, imgSizeY, 3, imgName);

// 			CString reOKFont=_T("");
// 			CString reNGFont=_T("");
// 			 reNGFont = fontCharVal;
// 			for(int nOKFontnum = 0; nOKFontnum<OKFont.GetLength();nOKFontnum++)
// 			{
// 				int index = fontCharVal.Find(OKFont.GetAt(nOKFontnum));
// 				CString tmp  = _T("");
// 				tmp += OKFont.GetAt(nOKFontnum);
// 				if(index != -1)
// 				{
// 					//reOKFont.Format(_T("%s%c"),reOKFont,fontCharVal.GetAt(nOKFontnum));
// 					reOKFont += fontCharVal.GetAt(index);
// 					reNGFont.Remove(fontCharVal.GetAt(index));
// 				}
// 			}
// 
// 			int compareflag = fontCharVal.Compare(OKFont);
// 			imgName.Format(_T("%s%s_%d.tif"), tempPath, fontCharVal, i);
// 			imgName.Format(_T("%sOK@fontImg%d.tif"), tempPath, i);
// 			//imgName.Format(_T("%sfontImg%d.jpg"), tempPath, i);		//jpg 읽어오기
// 
// 			GetBuffDrawDefineFont(fontImg, imgSizeX,imgSizeY, 0, eCOLOR_GREEN, i, reOKFont);
// 			m_procMil->SaveTIFImage(fontImg, imgSizeX, imgSizeY, 3, imgName);
// 
// 			imgName.Format(_T("%sNG@fontImg%d.tif"), tempPath, i);
// 
// 			GetBuffDrawDefineFont(fontImg, imgSizeX,imgSizeY, 0, eCOLOR_GREEN, i, reNGFont);
// 			m_procMil->SaveTIFImage(fontImg, imgSizeX, imgSizeY, 3, imgName);


			memset(fontImg, 0, sizeof(UCHAR) * imgArea * 3);
		}

		//delete fontImg;
		g_pMManager->pem_delete(fontImg, false);
	}

	*retFontCount = fontCnts;

	return ret;
}


int CPInsp_OCR::DeleteFont(int fontIndex, CString fontPath)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	int ret = ePART_SUCCESS;

	int fontCount = 0;
	if(!m_procMil->ExistContext())	
	{
		int loadRst = m_procMil->LoadStrFont(fontPath, &fontCount);

		if(loadRst != eSTR_SUCCESS)
			ret = ePART_FAIL;
	}
	else
	{
		fontCount = m_procMil->GetFontCount();
	}

	if(fontCount <= 0 || fontIndex >= fontCount)
		ret = ePART_FAIL;



	if(ret == ePART_SUCCESS)
		m_procMil->DeleteFont(fontIndex);


	return ret;
}

int CPInsp_OCR::DeleteFont(int fontIndex)
{
	int ret = ePART_SUCCESS;
	if(bIsPilBuff)
		return ePART_FAIL;

	m_procMil->DeleteFont(fontIndex);
	
	return ret;
}



#pragma region _BASE FUNCTION_


int CPInsp_OCR::SetStrNewFont(UCHAR* srcImg, int sizeX, int sizeY, double angle, int fontColor, CString text, int spaceWidthMode, int rmLineCnt, int* rmLinePos, int* rmLineWid, int threshMode, int threshVal , int* retThresh)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	int rstThresh = 0;

	m_procMil->AddNewFont();
	int ret = m_procMil->SetEditFont(srcImg, sizeX, sizeY, angle, fontColor, text, 0, rmLineCnt, rmLinePos, rmLineWid, threshMode,  threshVal, &rstThresh);
	m_procMil->SetSpaceWidthFont(spaceWidthMode);

	if(retThresh != NULL)
		*retThresh = rstThresh;

	if(ret == eSTR_SUCCESS)
		return ePART_SUCCESS;
	else
		return ePART_FAIL;
}

int CPInsp_OCR::SetStrFont(UCHAR* srcImg, int sizeX, int sizeY, double angle, int fontColor, CString text,int fontCount, int spaceWidthMode, int rmLineCnt, int* rmLinePos, int* rmLineWid, int threshMode, int threshVal, int* retThresh)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	int rstThresh = 0;

	//int fontCount = m_procMil->GetFontCount(); //font add¸¦ ÇÏ±â ÀüÀÌ±â ¶§¹®¿¡ add ÈÄÀÇ index°¡ addÀüÀÇ count¿Í °°´Ù.
	
	//	m_procMil->AddFont();
	int ret = m_procMil->SetEditFont(srcImg, sizeX, sizeY, angle, fontColor, text, fontCount, rmLineCnt, rmLinePos, rmLineWid, threshMode, threshVal, &rstThresh);
	m_procMil->SetSpaceWidthFont(spaceWidthMode);

	if(retThresh != NULL)
		*retThresh = rstThresh;

	if(ret == eSTR_SUCCESS)
	{
		return ePART_SUCCESS;
	}
	else
	{
		//m_procMil->DeleteFont(fontCount);
		return ePART_FAIL;
	}
}

int CPInsp_OCR::GetCharType(wchar_t ch)
{
	int ret = 0;
	if(bIsPilBuff)
		return ret;

	switch(isalnum(ch))
	{
	case 1: //upper case
		{
			ret = eSTR_UPPERCASE;
		}
		break;
	case 2: //lower case
		{
			ret = eSTR_LOWERCASE;
		}
		break;
	case 4: //number
		{
			ret = eSTR_DIGIT;
		}
		break;
	case 0: //any..
		{
			ret = eSTR_ANY;
		}
		break;
	}


	return ret;
}

int CPInsp_OCR::Preprocess(int forground)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	CString text = m_pInspParam->targetString;
	/*
	//LJH 2016.06.13
	int foreGround = 1; //배경이 항상 일정하게 검정으로 이진화를 하기 때문에 1으로 Fix
	*/
	int foreGround = m_pInspParam->foreGroundColor;
	int charMinCount = m_pInspParam->charMinCount;
	int charMaxCount = m_pInspParam->charMaxCount;
	int consecutivespace = m_pInspParam->m_nconsecutiveSpace;
	BOOL mode = m_pInspParam->mode;
	int textLen = text.GetLength();
	int* type = NULL;
	CString temp = _T("");
	wchar_t wcharTemap[2];

	if(textLen < 1)
		return ePROC_OCR_NOTFOUND_FONT;

	//type = new int[textLen];
	type = g_pMManager->pem_new<int>(true, textLen, (PCHAR)__FUNCTION__, __LINE__);

	if(forground == 2)forground = foreGround;

	if(m_procMil->GetStringModelCount() <=0)
	{
		int fontCount = m_procMil->GetFontCount();

		if(fontCount < 1)
			return ePROC_OCR_NOTFOUND_FONT;

		m_procMil->AddString();
		if(m_bstringMulti)
			m_procMil->SetStringCount(M_ALL);
		else 
			m_procMil->SetStringCount(1);
		m_procMil->SetConsecutiveSpace(consecutivespace);
		m_procMil->SetSpaceWidthFont(eSTR_MAX);	// 자간 Max Font Width 로
		

		//huj 2014/03/24
		m_procMil->SetStrAcceptScore(10);
		m_procMil->SetStrTargetAcceptScore(10);
		m_procMil->SetCharAcceptScore(10);
		m_procMil->SetCharMinCount(charMinCount);
		m_procMil->SetCharMaxCount(charMaxCount);
		m_procMil->SetStrForeGoundColor(forground);
		if(m_nOCRSimilarityScore < 50)
			m_nOCRSimilarityScore = 50;
		if(m_nHomogeneityScore < 50)
			m_nHomogeneityScore = 50;
		m_procMil->SetCharSimilAcceptScore(m_nOCRSimilarityScore);
		m_procMil->SetCharHomoAcceptScore(m_nHomogeneityScore);
		m_procMil->SetCharScaleFactor(1.3, 0.8);
		m_procMil->SetCharAspectRatio(1.1, 0.9);
		m_procMil->SetStringScaleFactor(1.3, 0.8);
		//m_procMil->SetEnableCharAngle(true);
		m_procMil->SetFontText(text);

		if(m_pInspParam->m_bConstraint /*|| !m_bImproveOCRVer*/)
		for(int i=0; i<textLen; i++)
		{	
			temp = text.Mid(i,1);
			wsprintf(wcharTemap, T2W(temp.GetBuffer(0)));

			//wcharTemap[0] = text.GetAt(i);

			type[i] = GetCharType(wcharTemap[0]);		
			m_procMil->SetConstraint(i, type[i], wcharTemap[0]);

			//temp.ReleaseBuffer();
			//m_procMil->SetConstraint(i, type[i], (mode ? wcharTemap[0] : NULL));
		}

		UnPreprocess();
		m_procMil->Preprocess();
	}
	else
	{
		if(m_procMil->GetCharMinCount() != charMinCount)
			m_procMil->SetCharMinCount(charMinCount);
		if(m_procMil->GetCharMaxCount() != charMaxCount)
			m_procMil->SetCharMaxCount(charMaxCount);
		if(m_procMil->GetStrForeGoundColor() != forground)
			m_procMil->SetStrForeGoundColor(forground);
		if(m_nOCRSimilarityScore < 50)
			m_nOCRSimilarityScore = 50;
		if(m_nHomogeneityScore < 50)
			m_nHomogeneityScore = 50;
		if(m_procMil->GetCharSimilAcceptScore() != m_nOCRSimilarityScore)
			m_procMil->SetCharSimilAcceptScore(m_nHomogeneityScore);
		if(m_procMil->GetCharHomoAcceptScore() != m_nHomogeneityScore)
			m_procMil->SetCharHomoAcceptScore(m_nHomogeneityScore);

		if(m_procMil->GetFontText() != text)
		{
			m_procMil->SetFontText(text);
			if(m_pInspParam->m_bConstraint)
			for(int i=0; i<textLen; i++)
			{
				wcharTemap[0] = text.GetAt(i);

				type[i] = GetCharType(wcharTemap[0]);		
				m_procMil->SetConstraint(i, type[i], wcharTemap[0]);
				//m_procMil->SetConstraint(i, type[i], (mode ? wcharTemap[0] : NULL));
			}
		}

		if(m_procMil->IsPreprocess()==false)
		{
			m_procMil->Preprocess();
		}
	}
	if(!m_procMil->IsPreprocess())
	{
		return ePART_FAIL;
	}
	//delete [] type;
	g_pMManager->pem_delete(type, true);

	return ePART_SUCCESS;
}

int CPInsp_OCR::SetThresholdMode(int threshMode, int threshVal)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	m_procMil->SetThreshHoldMode(threshMode,threshVal);

	return ePART_SUCCESS;
}


int CPInsp_OCR::UnPreprocess()
{
	if(bIsPilBuff)
		return ePART_FAIL;
	if(m_procMil->IsPreprocess())
		m_procMil->UnPreprocess();

	return ePART_SUCCESS;
}

int CPInsp_OCR::SetString(CString text, int foreGround, int charMinCount, int charMaxCount, BOOL mode, int threshMode, int threshVal)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	int textLen = text.GetLength();
	int* type;
	CString temp = _T("");
	wchar_t wcharTemap[2];
	int fontCount = m_procMil->GetFontCount();

	if(fontCount < 1)
		return ePROC_OCR_NOTFOUND_FONT;

	if(textLen < 1)
		return ePROC_OCR_NOTFOUND_FONT;

	//type = new int[textLen];
	type = g_pMManager->pem_new<int>(true, textLen, (PCHAR)__FUNCTION__, __LINE__);

	m_procMil->AddString();
	m_procMil->SetStringCount(M_ALL);
	m_procMil->SetConsecutiveSpace(1);
	m_procMil->SetCharMinCount(charMinCount);
	m_procMil->SetCharMaxCount(charMaxCount);
	m_procMil->SetStrForeGoundColor(foreGround);
	m_procMil->SetThreshHoldMode(threshMode, threshVal);


	for(int i=0; i<textLen; i++)
	{	temp = text.Mid(i,1);
		wsprintf(wcharTemap,T2W(temp.GetBuffer(0)));
		
  		type[i] = GetCharType(wcharTemap[0]);		
  		m_procMil->SetConstraint(i,type[i],(mode ? wcharTemap[0] : NULL));
	}
	
	//delete type;
	g_pMManager->pem_delete(type, false);

	return ePART_SUCCESS;
}

int CPInsp_OCR::SearchString(void* targetImg, int cX, int cY, int sizeX, int sizeY, double angle, int foreGround)
{
	if(bIsPilBuff)
		return 0;
	return m_procMil->StringReadStart(targetImg, cX, cY, sizeX, sizeY, angle, foreGround);
}

int CPInsp_OCR::SearchString(UCHAR* targetImg, int sizeX, int sizeY)
{
	if(bIsPilBuff)
		return 0;
	return m_procMil->StringReadStart(targetImg, sizeX, sizeY);
}


void CPInsp_OCR::InitStrResultStruct(RstInspOCR* ret)
{
	//RstInspOCR* temp = new RstInspOCR;
	RstInspOCR* temp = g_pMManager->pem_new<RstInspOCR>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, sizeof(RstInspOCR));

	temp->ok = -1;
	temp->isInsp = FALSE;
	temp->isReverse = FALSE;
	temp->list.charCountOk = FALSE;
	temp->list.existOK = FALSE;
	temp->list.stringOk = FALSE;
	temp->list.polarityOK = FALSE;
	temp->list.stringScoreOk = FALSE;

	for(int i = 0; i<MAX_STRLEN; i++)
	{
		temp->charScoreResult[i] = 0;
	}

	
	*ret = *temp;
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}
void CPInsp_OCR::InitStrResultTempStruct()
{
	m_resultTemp.strCount = 0;
	m_resultTemp.charCount = 0;
	m_resultTemp.stringScore = 0.0;
	m_resultTemp.strPosX = 0.0;
	m_resultTemp.strPosY = 0.0;
	m_resultTemp.strAngle = 0.0;
	m_resultTemp.str = _T("");
	memcpy(m_resultTemp.charScore,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
	memcpy(m_resultTemp.charX,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
	memcpy(m_resultTemp.charY,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
	memcpy(m_resultTemp.charWidth,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
	memcpy(m_resultTemp.charHeight,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
	memcpy(m_resultTemp.charangle,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
}


void CPInsp_OCR::GetInspRst(RstInspOCR* ret)
{
	InitStrResultStruct(ret);

	*ret = m_result;
}

int CPInsp_OCR::GetBuffDrawFont(UCHAR* dst, int sizeX, int sizeY, double angle, int color,int fontIndex)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	double tempColor = 0.0;
	tempColor = _GetColor(color);

	m_procMil->GetBuffDrawFont(dst, sizeX, sizeY, tempColor, fontIndex);
	return ePART_SUCCESS;
}
int CPInsp_OCR::GetBuffDrawDefineFont(UCHAR* dst, int sizeX, int sizeY, double angle, int color,int fontIndex, CString OKFont)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	double tempColor = 0.0;
	tempColor = _GetColor(color);

	m_procMil->GetBuffDrawDefineFont(dst, sizeX, sizeY, tempColor, fontIndex,OKFont);
	return ePART_SUCCESS;
}

int CPInsp_OCR::GetBuffDrawString(UCHAR* dst, int cx, int cy, int srcWidth, int srcHeight, int dstWidth, int dstHeight, double angle, int color)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	double tempColor = 0.0;
	tempColor = _GetColor(color);

	m_procMil->GetBuffDrawString(m_fovImage_insp, dst, cx, cy, srcWidth, srcHeight, dstWidth, dstHeight, angle, tempColor);
	return ePART_SUCCESS;
}

int CPInsp_OCR::GetBuffDrawBox(UCHAR* dst, int sizeX, int sizeY, double angle, int color)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	double tempColor = 0.0;
	tempColor = _GetColor(color);

	m_procMil->GetBuffDrawBox(dst, sizeX, sizeY, tempColor);
	return ePART_SUCCESS;
}

int CPInsp_OCR::SaveStrFont(CString path)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	m_procMil->SaveStrFont(path);
	return ePART_SUCCESS;
}

int CPInsp_OCR::LoadStrFont(CString path, int* retFontCount, long * retFontSize, int nInspectionMode)
{
	if(bIsPilBuff)
		return ePART_FAIL;
	return m_procMil->LoadStrFont(path, retFontCount, retFontSize, nInspectionMode);
}

int CPInsp_OCR::Decision(CString str, BOOL* ok)
{
	if(bIsPilBuff)
		return 0;
	if(str.IsEmpty())
	{
		*ok = FALSE;
	}
	else
	{
		int result = wcscmp(str, m_pInspParam->targetString);


		if(result != 0)
			*ok = FALSE;
		else 
			*ok = TRUE;
	}

	return 0;
}


void CPInsp_OCR::DeleteImgBuf()
{
// 	m_roiSizeX = 0;
// 	m_roiSizeY = 0;

	if(m_pInspImg != NULL)
	{
		//delete m_pInspImg;
		g_pMManager->pem_delete(m_pInspImg, false);

		m_pInspImg = NULL;
	}

	if(m_pInspImg_rotate != NULL)
	{
		//delete m_pInspImg_rotate;
		g_pMManager->pem_delete(m_pInspImg_rotate, false);

		m_pInspImg_rotate = NULL;
	}

// 	if(m_pWndImg != NULL)
// 	{
// 		delete m_pWndImg;
// 		for (int iLoopCount = 0; iLoopCount < 2; ++iLoopCount)
// 			if(m_pWndImg == m_pWndImg_Mix[iLoopCount])
// 				m_pWndImg_Mix[iLoopCount] = NULL;
// 
// 		m_pWndImg = NULL;
// 	}

	//LJH 2016.05.26
	for (int iLoopCount = 0; iLoopCount < 2; ++iLoopCount)
	{
		if (m_pInspImg_Mix[iLoopCount] != NULL)
		{
			//delete m_pInspImg_Mix[iLoopCount];
			g_pMManager->pem_delete(m_pInspImg_Mix[iLoopCount], false);
			m_pInspImg_Mix[iLoopCount] = NULL;
		}

		if (m_pInspImg_Mix_rotate[iLoopCount] != NULL)
		{
			//delete m_pInspImg_Mix_rotate[iLoopCount];
			g_pMManager->pem_delete(m_pInspImg_Mix_rotate[iLoopCount], false);
			m_pInspImg_Mix_rotate[iLoopCount] = NULL;
		}

// 		if (m_pWndImg_Mix[iLoopCount] != NULL)
// 		{
// 			delete m_pWndImg_Mix[iLoopCount];
// 			m_pWndImg_Mix[iLoopCount] = NULL;
// 		}
	}
}

int CPInsp_OCR::ReadStringOCR( CString& sReadOCR, CString sPathOCR )
{
	if(bIsPilBuff)
		return eMR_FAIL;
	sReadOCR = OCR_Dll_ReadFileOCR(sPathOCR);
	if(sReadOCR == _T(""))
		return eMR_FAIL;
	return eMR_SUCCESS;
}

CString CPInsp_OCR::ReadStringOCR( UCHAR* ptrFontImg, int nSizeX, int nSizeY )
{
	if(bIsPilBuff)
		return _T("");
	int widthStep = nSizeX;
	/*if (widthStep % 4 != 0)
	{
		widthStep = nSizeX + 4 - (widthStep % 4);
	}*/
	widthStep = g_pMPTI->nCalcWidthStep(false, nSizeX);

	CString sReadOCR = _T("");
	UCHAR *ptrImg = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrImg, widthStep * nSizeY);
	memset(ptrImg, 0, sizeof(UCHAR)* widthStep * nSizeY);

	IppStatus sts;
	IppiSize iSize = {nSizeX, nSizeY};
	sts = ippiCopy_8u_C1R(ptrFontImg, nSizeX, ptrImg, widthStep, iSize);

	m_procMil->SaveWorkImg(ptrFontImg, nSizeX, nSizeY, _T("OrgStringOCR.bmp"));
	m_procMil->SaveWorkImg(ptrImg, widthStep, nSizeY, _T("ReadStringOCR.bmp"));

	sReadOCR = OCR_Dll_ReadOCR(ptrImg, widthStep, nSizeY);

	Delete_1DArray(&ptrImg);
	return sReadOCR;
}

int CPInsp_OCR::OKNGConfirm(int strRstTemp, int searchThresold)
{
	int ret = e_OK;
	if(bIsPilBuff)
		return e_NG;

	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;
	BOOL bUsePassFont = m_pInspParam->m_bUsePassFont;
	int nPassFontCnt = m_pInspParam->m_nPassFontCnt;

	for(int index = 0;index <strRstTemp ; index++)
	{
		if(!CheckScore(useCharScore, stdScore, stdCharCount, bUsePassFont, nPassFontCnt,index))
			ret = e_NG;
		else
		{
			BOOL isStringOK = FALSE;
			Decision(m_procMil->GetString(index), &isStringOK);	// 타겟 문자열과 찾은 문자열이 같은지 체크
			if(!isStringOK)		
				ret = e_NG;
			else 
			{
				if(searchThresold != -1)
					m_pInspParam->m_searchThreshold = searchThresold;
				m_nstringIndex = index;
				ret = e_OK;
				break;
			}

		}
	}


	return ret;
}
void CPInsp_OCR::SetHist(bool bIsHist)
{
	m_bIsHisto = bIsHist;
}
int CPInsp_OCR::InspROIImg()
{
	int ret = e_NG;
	if(bIsPilBuff)
		return e_NG;

	double x,y;
	UCHAR* tmpImg = NULL;
	//tmpImg = new UCHAR[m_roiSizeX*m_roiSizeY];
	tmpImg = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	UCHAR* backupImg = m_pInspImg;

	int OrgSizeX = m_roiSizeX;
	int OrgSizeY = m_roiSizeY;

	int roiSizeX = m_roiSizeX;
	int roiSizeY = m_roiSizeY;

	x = m_pInspParam->m_rtInspectArea.left-10>0?m_pInspParam->m_rtInspectArea.left-10:0;
	y = m_pInspParam->m_rtInspectArea.top-10>0?m_pInspParam->m_rtInspectArea.top-10:0;
	m_roiSizeX =m_pInspParam->m_rtInspectArea.right+20<roiSizeX?m_pInspParam->m_rtInspectArea.right-x+10:roiSizeX-x;
	m_roiSizeY = m_pInspParam->m_rtInspectArea.bottom+20<roiSizeY? m_pInspParam->m_rtInspectArea.bottom-y+10:roiSizeY-y;

	m_procMil->GetClipImage_LT(m_pInspImg, roiSizeX, roiSizeY, tmpImg, x, y, m_roiSizeX, m_roiSizeY, 1);
	m_procMil->SaveWorkImg(tmpImg,m_roiSizeX,m_roiSizeY,_T("OCR_ROI_tmpImg.bmp"));

	m_pInspImg = tmpImg;

	m_procMil->RotateImg_ipp(m_pInspImg, m_roiSizeX, m_roiSizeY, 180, &m_pInspImg_rotate, &m_roiSizeX, &m_roiSizeY);

	m_bUseImgBufAlloc = false;
	ret = ProcAuto2();

	m_roiSizeX = OrgSizeX;
	m_roiSizeY = OrgSizeY;
	DeleteImgBuf();
	m_pInspImg = backupImg;
	m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("OCR_ROI_backupImg.bmp"));

	// 	for(int i = 0; i < sRstAlgo->m_nCharCount; i++)
	// 	{
	// 		sRstAlgo->m_dArrCharScore[i] = m_result.charScore[i];
	// 		sRstAlgo->m_cArrStr[i] = m_result.str[i];
	// 		sRstAlgo->m_nCharScoreResult[i] = m_result.charScoreResult[i];
	// 	}

	if(ret == e_OK)
	{
		if(!m_bReRotate)
			for(int i = 0; i <m_result.charCount;i++)
			{
				m_result.charX[i] = m_result.charX[i] +x;
				m_result.charY[i] = m_result.charY[i] +y;
			}

		else
			for(int i = 0; i <m_result.charCount;i++)
			{
				m_result.charX[i] = roiSizeX-m_result.charX[i]-m_result.charWidth[i]-x;
				m_result.charY[i] = roiSizeY-m_result.charY[i]-m_result.charHeight[i]-y;
			}
	}

	return ret;
}
int CPInsp_OCR::ReInspRectimg()
{
	int ret = e_NG;
	if(bIsPilBuff)
		return e_NG;
	if(m_result.charCount >0) // shkim 2017.01.03 Draw Rect if failled ocr img
	{
		int cnt = 0;
		GetAllposition();
		if(m_result.strCount<1)
			cnt = 1;
		else cnt = m_result.charCount*m_result.strCount;

		double fontWidth = m_procMil->GetFontWidth();
		double stringWidth =m_result.charX[cnt-1] - m_result.charX[0]+fontWidth;
		// 			if(m_bReRotate)
		// 			{
		// 				delete m_pInspImg_rotate;
		// 				m_pInspImg_rotate = NULL;
		// 				m_pInspImg_rotate = new UCHAR[m_roiSizeY*m_roiSizeX];
		// 			}

		if(stringWidth<fontWidth *( m_result.charCount + m_pInspParam->m_nconsecutiveSpace + 1))
		{

			m_Opencv_Functions->DrawCharRect(m_pInspImg,m_roiSizeX,m_roiSizeY,m_result.charX,m_result.charY,m_result.charWidth,m_result.charHeight,cnt,m_pInspParam->foreGroundColor);

			m_procMil->RotateImg_ipp(m_pInspImg, m_roiSizeX, m_roiSizeY, 180, &m_pInspImg_rotate, &m_roiSizeX, &m_roiSizeY);
			m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("OCR_CharRect.bmp"));
			m_procMil->SaveWorkImg(m_pInspImg_rotate,m_roiSizeX,m_roiSizeY,_T("OCR_CharRect_rotate.bmp"));
			m_bUseImgBufAlloc = false;

			ret = ProcAuto2();
		}

	}

	return ret;
}
int CPInsp_OCR::ReInspLowScore()
{
	int ret = e_NG;
	if(bIsPilBuff)
		return e_NG;
	wchar_t targetstr[256];
	memcpy(targetstr,m_pInspParam->targetString,sizeof(wchar_t)*256);
	UCHAR* backupImg = m_pInspImg;
	UCHAR* backupImg_rotate = m_pInspImg_rotate;

	double x,y,width,height;
	width = 0; height = 0;

	if(m_pInspParam->charMaxCount<=1)
	{
		g_pMPTI->AddLog_Dev(_T("PInsp_OCR_ reInspLowScore_ m_pInspParam->charMaxCount<=1"));
		return 0;
	}

	width =(m_result.charX[m_result.charCount-1]-m_result.charX[0])+m_result.charWidth[m_result.charCount-1];
	for(int i = 0;i<m_result.charCount;i++)
	{
		height =height<m_result.charHeight[i]?m_result.charHeight[i]:height;
	}
	m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("OCRlowScore_Crop_SRC.bmp"));

	if(m_bReRotate)
	{
		//delete m_pInspImg;
		//m_pInspImg = m_pInspImg_rotate;
		m_pInspImg=NULL;
		m_procMil->RotateImg_ipp(backupImg, m_roiSizeX, m_roiSizeY, 180, &m_pInspImg, &m_roiSizeX, &m_roiSizeY);
	}

	int roiSizeX = m_roiSizeX;
	int roiSizeY = m_roiSizeY;

	x = m_result.charX[0]-10;
	y = m_result.charY[0]-10;
	m_roiSizeX = x+width+10<roiSizeX?width+20:roiSizeX-x;
	m_roiSizeY = y+height+10<roiSizeY?height+20:roiSizeY-y;
	if(x<0)
	{
		m_roiSizeX = roiSizeX + (x-10);
	}
	if(y<0)
	{
		y = 0;
		m_roiSizeY = roiSizeY + (y-10);
	}
	int SizeofImg = m_roiSizeX*m_roiSizeY;
	if(SizeofImg < 1)
		return ret;

	UCHAR* tmpImg = NULL;
	//tmpImg = new UCHAR[SizeofImg];
	tmpImg = g_pMManager->pem_new<UCHAR>(true, SizeofImg, (PCHAR)__FUNCTION__, __LINE__);
	m_procMil->GetClipImage_LT(m_pInspImg, roiSizeX, roiSizeY, tmpImg, x, y, m_roiSizeX, m_roiSizeY, 1);
	m_pInspImg = NULL;
	m_procMil->RotateImg_ipp(tmpImg, m_roiSizeX, m_roiSizeY, 180, &m_pInspImg, &m_roiSizeX, &m_roiSizeY);
	m_pInspImg_rotate = tmpImg;
	m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("OCRlowScore_Crop.bmp"));

	m_bUseImgBufAlloc = false;
	ret = ProcAuto2();
	m_roiSizeX = roiSizeX;
	m_roiSizeY = roiSizeY;
	DeleteImgBuf();
	m_pInspImg = backupImg;
	m_pInspImg_rotate = backupImg_rotate;

	return ret;
}

int CPInsp_OCR::ReInspLowSimilarity(RstAlgoOCR * sRstAlgo)
{
	if(bIsPilBuff)
		return e_NG;
	int ret = e_OK;
	int ret_tmp = e_OK;
	double similar[MAX_STRLEN];
	m_procMil->GetCharSIMILARITY(similar,m_nstringIndex);

	wchar_t targetstr[256];
	memcpy(targetstr,m_pInspParam->targetString,sizeof(wchar_t)*256);
	UCHAR* backupImg = m_pInspImg;

	int forCount = m_result.charCount;
	double charX_tmp[MAX_STRLEN];
	double charY_tmp[MAX_STRLEN];
	double charWidth_tmp[MAX_STRLEN];
	double charHeight_tmp[MAX_STRLEN];
	memcpy(charX_tmp,m_result.charX,sizeof(double)*MAX_STRLEN);
	memcpy(charY_tmp,m_result.charY,sizeof(double)*MAX_STRLEN);
	memcpy(charWidth_tmp,m_result.charWidth,sizeof(double)*MAX_STRLEN);
	memcpy(charHeight_tmp,m_result.charHeight,sizeof(double)*MAX_STRLEN);
	for(int i = 0;i<forCount;i++)
	{
		if(similar[i]<90/*&&(targetstr[i] != '1' && targetstr[i] != '0' && targetstr[i] != '8' && targetstr[i] != 'X' && targetstr[i] != 'H')*/)
		{
#if Timecheck
			CQTimer qtm; 
			qtm.StartTick();
#endif
			m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("OCR_Crop_SRC.bmp"));

			if(m_bReRotate)
			{
				//delete m_pInspImg;
				//m_pInspImg = m_pInspImg_rotate;
				m_pInspImg = NULL;
				m_procMil->RotateImg_ipp(backupImg, m_roiSizeX, m_roiSizeY, 180, &m_pInspImg, &m_roiSizeX, &m_roiSizeY);
			}

			int roiSizeX = m_roiSizeX;
			int roiSizeY = m_roiSizeY;
			double x,y;

			x = charX_tmp[i]-10;
			y = charY_tmp[i]-10;
			m_roiSizeX = charX_tmp[i]+charWidth_tmp[i]+10<roiSizeX?charWidth_tmp[i]+20:roiSizeX-x;
			m_roiSizeY = charY_tmp[i]+charHeight_tmp[i]+10<roiSizeY?charHeight_tmp[i]+20:roiSizeY-y;
			if(x<0)
			{
				x = 0;
				m_roiSizeX = roiSizeX + (charX_tmp[i]-10);
			}
			if(y<0)
			{
				y = 0;
				m_roiSizeY = roiSizeY + (charY_tmp[i]-10);
			}
			int SizeofImg = m_roiSizeX*m_roiSizeY;
			if(SizeofImg < 1)
				return e_NG;

			UCHAR* tmpImg = NULL;
			//tmpImg = new UCHAR[SizeofImg];
			tmpImg = g_pMManager->pem_new<UCHAR>(true, SizeofImg, (PCHAR)__FUNCTION__, __LINE__);
			m_procMil->GetClipImage_LT(m_pInspImg, roiSizeX, roiSizeY, tmpImg, x, y, m_roiSizeX, m_roiSizeY, 1);
			//				m_Opencv_Functions->ImgCrop(m_pInspImg, m_roiSizeX,m_roiSizeY,m_result.charX[i], m_result.charY[i], m_result.charWidth[i], m_result.charHeight[i],tmpImg);
			//delete m_pInspImg;
			m_pInspImg = tmpImg;
			m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("OCR_Crop.bmp"));

			m_pInspParam->charMinCount = 1;
			m_pInspParam->charMaxCount = 1;
			memset(m_pInspParam->targetString,0,sizeof(wchar_t)*256);
			m_pInspParam->targetString[0] = targetstr[i];


			if(m_bReRotate)
			{
				m_pInspImg_rotate = m_pInspImg;

				m_pInspImg = NULL;
				m_procMil->RotateImg_ipp(m_pInspImg_rotate, m_roiSizeX, m_roiSizeY, 180, &m_pInspImg, &m_roiSizeX, &m_roiSizeY);
				m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("OCR_Crop.bmp"));
				m_procMil->SaveWorkImg(m_pInspImg_rotate,m_roiSizeX,m_roiSizeY,_T("OCR_Crop_rotate.bmp"));
			}
			m_bUseImgBufAlloc = false;
			m_result.str[0]=0;
			ret_tmp = ProcAuto2();
			if(ret_tmp != e_OK){

				m_result.ok = FALSE;
				m_result.list.stringScoreOk = FALSE;
				m_result.list.stringOk = FALSE;
				m_result.stringScore = 0.0;
				m_result.charScoreResult[i] = 1;
				m_result.charScore[i] = 0.0;

				if(m_result.str[0]==0)
					sRstAlgo->m_cArrStr[i] = '?';
				else if(m_result.stringScore < 1)
					sRstAlgo->m_cArrStr[i] = '?';
				else
				sRstAlgo->m_cArrStr[i] = m_result.str[0];
				sRstAlgo->m_bOKString = m_result.list.stringOk;
				sRstAlgo->m_bOKScore = m_result.list.stringScoreOk;
				sRstAlgo->m_dStringScore = sRstAlgo->m_dStringScore>m_result.stringScore?m_result.stringScore:sRstAlgo->m_dStringScore;
				sRstAlgo->m_nCharScoreResult[i] = m_result.charScoreResult[i];
				sRstAlgo->m_dArrCharScore[i] =m_result.charScore[i];
				ret = e_NG;
			}
			else
			{
				sRstAlgo->m_cArrStr[i] = m_result.str[0];
				sRstAlgo->m_bOKString = m_result.list.stringOk;
				sRstAlgo->m_bOKScore = m_result.list.stringScoreOk;
				sRstAlgo->m_dStringScore = sRstAlgo->m_dStringScore>m_result.stringScore?m_result.stringScore:sRstAlgo->m_dStringScore;
				sRstAlgo->m_nCharScoreResult[i] = m_result.charScoreResult[0];
				sRstAlgo->m_dArrCharScore[i] =m_result.charScore[0];
			}
			m_roiSizeX = roiSizeX;
			m_roiSizeY = roiSizeY;
			DeleteImgBuf();
			m_pInspImg = backupImg;

#if Timecheck
			Singleword_Insp = (qtm.EndTick() * 1000.0) + Singleword_Insp;
#endif
		}
	}
	m_result.charCount =  forCount;
	memcpy(m_pInspParam->targetString,targetstr,sizeof(wchar_t)*256);

	return ret;
}

void CPInsp_OCR::saveResultTemp(int index)
{
	double strScore = 0;
	m_procMil->GetStringScore(&strScore,index);
	if(strScore>=m_resultTemp.stringScore)
	{

		double charScore[MAX_STRLEN] = {0.0,};
		double x[MAX_STRLEN] = {0.0,};
		double y[MAX_STRLEN] = {0.0,};
		double w[MAX_STRLEN] = {0.0,};
		double h[MAX_STRLEN] = {0.0,};
		double angle[MAX_STRLEN] = {0.0,};

		m_procMil->GetStringCount(&m_resultTemp.strCount);
		m_procMil->GetCharCount(&m_resultTemp.charCount);
		m_resultTemp.stringScore = strScore;
		m_procMil->GetCharScore(m_resultTemp.charScore,index);
		m_procMil->GetStringPos(&m_resultTemp.strPosX, &m_resultTemp.strPosY,index);
		m_procMil->GetStringAngle(&m_resultTemp.strAngle,index);
		m_resultTemp.str = m_procMil->GetString(index);

		m_procMil->SetImgSize(m_roiSizeX,m_roiSizeY);
		m_procMil->GetCharPos(x, y,w,h,angle, index,m_bReRotate);
		memcpy(m_resultTemp.charX,x,sizeof(double)*MAX_STRLEN);
		memcpy(m_resultTemp.charY,y,sizeof(double)*MAX_STRLEN);
		memcpy(m_resultTemp.charWidth,w,sizeof(double)*MAX_STRLEN);
		memcpy(m_resultTemp.charHeight,h,sizeof(double)*MAX_STRLEN);
		memcpy(m_resultTemp.charangle,angle,sizeof(double)*MAX_STRLEN);
	}
}
void CPInsp_OCR::SaveParam()
{
	CFileException ex;
	CStdioFile file;
	CString filePath;

	CTime t = CTime::GetCurrentTime();
	filePath.Format(_T("C:\\Eagle3D_64x\\TEMP\\OCR_Param_%s.csv"),t.Format(_T("%y.%m.%d_%Hh.%Mm.%Ss")));

	if(file.Open(filePath, CFile::modeCreate | CFile::modeReadWrite, nullptr, &ex)==TRUE)
	{
		CString sLine;

		sLine.Format(_T("foreGround %d,\r\n"), m_pInspParam->foreGroundColor);
		file.WriteString(sLine);
		sLine.Format(_T("charMinCount , %d ,\r\n"), m_pInspParam->charMinCount);
		file.WriteString(sLine);
		sLine.Format(_T("charMaxCount, %d ,\r\n"), m_pInspParam->charMaxCount);
		file.WriteString(sLine);


		sLine.Format(_T("m_nOCRSimilarityScore %d \r\n"), m_nOCRSimilarityScore);
		file.WriteString(sLine);
		sLine.Format(_T("m_nHomogeneityScore %d \r\n"), m_nHomogeneityScore);
		file.WriteString(sLine);
		sLine.Format(_T("m_bConstraint %d \r\n"), m_pInspParam->m_bConstraint);
		file.WriteString(sLine);
		sLine.Format(_T("m_bstringMulti %d \r\n"), m_bstringMulti);
		file.WriteString(sLine);
		sLine.Format(_T("consecutivespace %d \r\n"), consecute_tmp);
		file.WriteString(sLine);

		file.Close();
	}
}

int CPInsp_OCR::InspProc_OCR2(RstAlgoOCR * sRstAlgo, int nInspectionMode)
{
	if(bIsPilBuff)
		return e_NG;
	int ret = e_NG;
	int nLine = __LINE__;
	try
	{
		//shkim Img Alloc
		m_nInspectionMode = nInspectionMode;
		double wndAngle = m_inspCoordinate.angle;
		DeleteImgBuf();
		nLine = __LINE__;

		//m_pWndImg = new UCHAR[m_roiSizeX*m_roiSizeY];
		/*m_pInspImg = new UCHAR[m_roiSizeX*m_roiSizeY];
		m_pInspImg_rotate = new UCHAR[m_roiSizeX*m_roiSizeY];*/
		m_pInspImg = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
		m_pInspImg_rotate = g_pMManager->pem_new<UCHAR>(true, m_roiSizeX*m_roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

		if(IsAnyAngle(wndAngle))
			wndAngle = 0;
		double angle = CalcAngle(m_pInspParam->fontAngle, wndAngle, FALSE);

		nLine = __LINE__;
		//0, 90, 180, 270 각도를 제외한 각도가 들어가면 Rotate 한 이미지에서 메모리 에러 발생하는 듯
		if((angle != 0) && (angle != 90) && (angle != 180) && (angle != 270))
			angle = 0;

		m_procMil->GetClipBuff_LT(m_fovImage_insp, m_pInspImg, 0, 0, m_roiSizeX, m_roiSizeY);


		nLine = __LINE__;

		int SizeofImg = m_roiSizeX*m_roiSizeY;
		UCHAR* tmpImg = NULL;
		//tmpImg = new UCHAR[SizeofImg];
		tmpImg = g_pMManager->pem_new<UCHAR>(true, SizeofImg, (PCHAR)__FUNCTION__, __LINE__);


		nLine = __LINE__;
		m_procMil->RotateImg_ipp(m_pInspImg, m_roiSizeX, m_roiSizeY, angle, &tmpImg, &m_roiSizeX, &m_roiSizeY);
		//delete m_pInspImg;
		g_pMManager->pem_delete(m_pInspImg, false);
		m_procMil->SaveWorkImg(tmpImg,m_roiSizeX,m_roiSizeY,_T("OCR_ROI_img_save.bmp"));

		nLine = __LINE__;
		m_pInspImg = tmpImg;
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// shkim 2017.02.09 set tempResult
		InitStrResultTempStruct();
// 		m_resultTemp.strCount = 0;
// 		m_resultTemp.charCount = 0;
// 		m_resultTemp.stringScore = 0.0;
// 		m_resultTemp.strPosX = 0.0;
// 		m_resultTemp.strPosY = 0.0;
// 		m_resultTemp.strAngle = 0.0;
// 		m_resultTemp.str = _T("");
// 		memcpy(m_resultTemp.charScore,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
// 		memcpy(m_resultTemp.charX,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
// 		memcpy(m_resultTemp.charY,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
// 		memcpy(m_resultTemp.charWidth,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
// 		memcpy(m_resultTemp.charHeight,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
// 		memcpy(m_resultTemp.charangle,m_procMil->doubeZeroBuf,sizeof(double)*MAX_STRLEN);
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		nLine = __LINE__;

		//2017.01.23 shkim  Roi Img Insp
		if(m_pInspParam->m_rtInspectArea.right>0 && m_pInspParam->m_rtInspectArea.bottom>0)
		{
			ret = InspROIImg();
		}

		nLine = __LINE__;
		if(ret != e_OK)
		{
			m_bUseImgBufAlloc = true;
			ret = ProcAuto2();
		}
		nLine = __LINE__;
		// ÀÓ½Ã (ÃßÈÄ º¯°æÇÒ °Í)
		//if(ret == e_OK)
		//	ret = m_result.ok;
		if(ret != e_OK && (m_pInspParam->m_bUseOCRNG != TRUE))
		{
			ret = ReInspRectimg();
		}

		nLine = __LINE__;

		sRstAlgo->m_nCharCount = m_result.charCount;
		sRstAlgo->m_dStringScore = m_result.stringScore;
		// 	for(int i = 0; i < sRstAlgo->m_nCharCount; i++)
		// 	{
		// 		sRstAlgo->m_dArrCharScore[i] = m_result.charScore[i];
		// 		sRstAlgo->m_cArrStr[i] = m_result.str[i];
		// 		sRstAlgo->m_nCharScoreResult[i] = m_result.charScoreResult[i];
		// 	}
		sRstAlgo->m_dStrPosX = m_result.strPosX;
		sRstAlgo->m_dStrPosY = m_result.strPosY;
		sRstAlgo->m_dStrAngle = m_result.strAngle;
		sRstAlgo->m_nInspCount = m_result.inspCount;
		sRstAlgo->m_bIsReverse = m_result.isReverse;
		sRstAlgo->m_bOKString = m_result.list.stringOk;
		sRstAlgo->m_bOKScore = m_result.list.stringScoreOk;
		sRstAlgo->m_bOKPolarity = m_result.list.polarityOK;
		sRstAlgo->m_nSearchThreshold = m_pInspParam->m_searchThreshold;
		sRstAlgo->m_nAutoFlag = m_pInspParam->m_nAutoFlag;

		memcpy(sRstAlgo->m_dArrCharScore,m_result.charScore,sizeof(double)*sRstAlgo->m_nCharCount);
		memcpy(sRstAlgo->m_cArrStr,m_result.str,sizeof(wchar_t)*MAX_STRLEN);
		memcpy(sRstAlgo->m_nCharScoreResult,m_result.charScoreResult,sizeof(int)*sRstAlgo->m_nCharCount);

		memcpy(sRstAlgo->charX,m_result.charX,sizeof(double)*MAX_STRLEN);
		memcpy(sRstAlgo->charY,m_result.charY,sizeof(double)*MAX_STRLEN);
		memcpy(sRstAlgo->charWidth,m_result.charWidth,sizeof(double)*MAX_STRLEN);
		memcpy(sRstAlgo->charHeight,m_result.charHeight,sizeof(double)*MAX_STRLEN);
		memcpy(sRstAlgo->charAngle,m_result.charAngle,sizeof(double)*MAX_STRLEN);

		nLine = __LINE__;
		sRstAlgo->m_bRotate = m_bReRotate;
		if(ret == e_OK&&m_pInspParam->isPolarity != TRUE) // shkim 2017.01.16 if OKString score lower than standard score, rotate Insp
		{
			if(m_result.stringScore < m_nrotateInspScore)
			{
				ret = ReInspLowScore();

				nLine = __LINE__;
				if(ret == e_OK && sRstAlgo->m_dStringScore < m_result.stringScore){

					sRstAlgo->m_nCharCount = 0;
					sRstAlgo->m_dStringScore = 0;
					// 	for(int i = 0; i < sRstAlgo->m_nCharCount; i++)
					// 	{
					// 		sRstAlgo->m_dArrCharScore[i] = m_result.charScore[i];
					// 		sRstAlgo->m_cArrStr[i] = m_result.str[i];
					// 		sRstAlgo->m_nCharScoreResult[i] = m_result.charScoreResult[i];
					// 	}
					sRstAlgo->m_dStrPosX = 0;
					sRstAlgo->m_dStrPosY = 0;
					sRstAlgo->m_dStrAngle = 0;
					sRstAlgo->m_nInspCount = 0;
					sRstAlgo->m_bIsReverse = 0;
					sRstAlgo->m_bOKString = 0;
					sRstAlgo->m_bOKScore = 0;
					sRstAlgo->m_bOKPolarity = 0;
					sRstAlgo->m_nSearchThreshold = m_pInspParam->m_searchThreshold;
					sRstAlgo->m_nAutoFlag = m_pInspParam->m_nAutoFlag;

					memcpy(sRstAlgo->m_dArrCharScore,doubeZeroBuf,sizeof(double)*sRstAlgo->m_nCharCount);
					memcpy(sRstAlgo->m_cArrStr,wcharZeroBuf,sizeof(wchar_t)*sRstAlgo->m_nCharCount);
					memcpy(sRstAlgo->m_nCharScoreResult,intZeroBuf,sizeof(int)*sRstAlgo->m_nCharCount);

					memcpy(sRstAlgo->charX,doubeZeroBuf,sizeof(double)*MAX_STRLEN);
					memcpy(sRstAlgo->charY,doubeZeroBuf,sizeof(double)*MAX_STRLEN);
					memcpy(sRstAlgo->charWidth,doubeZeroBuf,sizeof(double)*MAX_STRLEN);
					memcpy(sRstAlgo->charHeight,doubeZeroBuf,sizeof(double)*MAX_STRLEN);
					memcpy(sRstAlgo->charAngle,doubeZeroBuf,sizeof(double)*MAX_STRLEN);
					ret = e_NG;
					nLine = __LINE__;
				}
				else
					ret = e_OK;
			}
		}

		nLine = __LINE__;
		if(ret == e_OK) // shkim 2017.01.04 low similarity score char in OK String
		{
			ret = ReInspLowSimilarity(sRstAlgo);
		}
		nLine = __LINE__;

		///////////////////////////////Save Param////////////////////////////////////

		if(m_OCRNGParamSave && ret == e_NG)
		{
			SaveParam();
		}
		nLine = __LINE__;
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	catch(...)
	{
		if(g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CMInspManager::InspProc_OCR2(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return ret;
}

int CPInsp_OCR::ProcAuto2()
{
	if(bIsPilBuff)
		return e_NG;
	int ret = e_NG;
	m_procMil->SaveWorkImg(m_pInspImg,m_roiSizeX,m_roiSizeY,_T("ProcAuto_Img.bmp"));

	m_oldResultFlag = false;
	InitStrResultStruct(&m_result);
	InitStrResultStruct(&m_pInspParam->retInspOcrResult);

	int fontCnts = 0;
	m_nFontSizeY = 0;
	int isLoad = 0;
	//if (m_pInspParam->m_nImageMixCount <= 0) 	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
		isLoad = LoadStrFont(m_pInspParam->fontPath, &fontCnts, &m_nFontSizeY, m_nInspectionMode);  

	if(m_nFontSizeY <= FONTSIZE_S)
		m_nThickenCnt = 0;
	else if(m_nFontSizeY <= FONTSIZE_M)
		m_nThickenCnt = 1;
	else
		m_nThickenCnt = 2;
	
	if(isLoad == eSTR_FAIL)
	{
		ret = ePROC_OCR_NOTFOUND_FILE;
		return ret;
	}
	if(fontCnts < 1 )
	{
		ret = ePROC_OCR_NOTFOUND_FONT;
		return ret;
	}	
	
	int stepID = m_pInspParam->m_nInspFunc;
	int maxStepCnts = eOStepID2_COUNT;
	//step1(eOStepID_USERVALUE) : USER_DEFINED (user value)
	//step2(eOStepID_AUTO) : LOCAL -> LOCAL_WITH_RESEG -> USER_DEFINED (auto value for matrox)
	//step3(eOStepID_PEMVALUE) : USER_DEFINED (auto value for pemtron)

	if(m_bUseImgBufAlloc == true)
	{
#if Timecheck
		qtm.StartTick();
		ImgBuffAlloc2();
		StepID_IMGBUF_ALLOC = (qtm.EndTick() * 1000.0) + StepID_IMGBUF_ALLOC;
#else
		ImgBuffAlloc2();
#endif
	}
	//////////////////////////////////////////////////////////////////////////
	if(m_mainLightIndex > -1)
	{
		int imgIndex = m_mainLightIndex;
		int index[eImg_BufCnt] = {0,};
		int indexCount = 0;
		GetLightArray(imgIndex, index, &indexCount);

		m_procMil->StrAllocResult();
		//huj 2014/03/06  preprocess¸¦ ÇÑ¹ø¸¸ ÇÑ´Ù.
		if(AllocString() == e_NG){
			DecisionOcrInsp(e_NG);
			return e_NG;
		}
		for(int j = 0; j < indexCount; j++)
		{
			m_fovImage_insp = m_imgBuf[index[j]];
			//LJH 2016.05.26
			for(int iLoopCount = 0; iLoopCount < m_pInspAlgoOCR->m_nImageMixCount; ++iLoopCount)
				m_fovImage_insp_Mix[iLoopCount] = m_imgBuf[index[j]];

			stepID = m_pInspParam->m_nInspFunc;

			for(int i = 0; i < maxStepCnts; i++)
			{	
				ret = ProcStep(stepID%maxStepCnts);

				stepID++;
				if(ret == e_OK) 
					break;
			}

			if(ret == e_OK) 
				break;
		}

		//¼öÁ¤µÈ ¼Ò½º  (ÀÌ¹ÌÁö 7Àå)
		/*for(int j = 0; j < eImg_BufCnt; j++)
		{
		index = GetLightIndex(imgIndex);

		m_fovImage_insp = m_imgBuf[index];

		stepID = 0;
		for(int i = 0; i < maxStepCnts; i++)
		{	
		ret = ProcStep(stepID);

		stepID++;
		if(ret == e_OK) 
		break;
		}
		if(ret == e_OK) 
		break;


		if(m_mainLightIndex != 0)
		{
		imgIndex = j;
		if(j == m_mainLightIndex)
		{
		imgIndex = eImg_BufCnt - 1;
		}
		}	
		else
		{
		imgIndex = j + 1;
		}
		}*/
	}
	else if(m_fovImage_insp)
	{
		m_procMil->StrAllocResult();	// LMJ 2014/11/18
		if(AllocString() == e_NG){
			DecisionOcrInsp(e_NG);
			return e_NG;
		}

			if(m_bUseImgBufAlloc == true){

#if Timecheck
				qtm.StartTick();
				ImgBuffAlloc2();
				StepID_IMGBUF_ALLOC = (qtm.EndTick() * 1000.0) + StepID_IMGBUF_ALLOC;
#else
				ImgBuffAlloc2();
#endif
			}
		for(int i = 0; i < maxStepCnts; i++)
		{	
			ret = ProcStep(stepID%maxStepCnts);

			stepID++;	
			if(ret == e_OK) 
				break;
		}
	}

	if(ret != e_OK)
	{
		//			eOStepID_THRESHOLD_TRACKING:
		{	
			if(m_pInspParam->m_bThresholdtracking != FALSE) 
				if(m_bIsHisto != false)					//2016.12.01 shkim image가 Histogram이 아닐경우 skip
#if Timecheck
					qtm.StartTick();
			ret = ThresholdTracking(FALSE);
			StepID_THRESHOLD_TRACKING = (qtm.EndTick() * 1000.0) + StepID_THRESHOLD_TRACKING;
			m_bReRotate = true;
#else
					ret = ThresholdTracking(FALSE);
			m_bReRotate = false;
#endif
			m_nThresholdTracking_in++;
		}
		//			eOStepID_THRESHOLD_TRACKING_REV:
		if(ret != e_OK)
		{	
			if(m_pInspParam->isPolarity != FALSE)
				if(m_pInspParam->m_bThresholdtracking != FALSE)
					if(m_bIsHisto != false) 					//2016.12.01 shkim image가 Histogram이 아닐경우 skip
					{
#if Timecheck

						qtm.StartTick();
						ret = ThresholdTracking(TRUE);
						StepID_THRESHOLD_TRACKING_REV = (qtm.EndTick() * 1000.0) + StepID_THRESHOLD_TRACKING_REV;
						m_bReRotate = true;
#else
						ret = ThresholdTracking(TRUE);
						m_bReRotate = true;
#endif
						m_nThresholdTracking_in++;
					}
		}
	}

	//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG 
	if(ret == e_OK)
	{
		if(m_result.isReverse)
			m_procMil->DrawString(m_pInspImg_rotate, m_roiSizeX, m_roiSizeY);
		else
			m_procMil->DrawString(m_pInspImg, m_roiSizeX, m_roiSizeY);
	}
#endif
	//////////////////////////////////////////////////////////////////////////

	DecisionOcrInsp(ret);

	if(m_pInspParam->m_bUseOCRNG == TRUE)
	{
		if(ret == e_NG)
			ret = e_OK;
		else
			ret = e_NG;
	}
	else
	{
		if(ret == e_OK)
			ret = m_result.ok;
	}

	//UnPreprocess();

	return ret;
}


int CPInsp_OCR::ProcStep2(int stepID2)
{
	if(bIsPilBuff)
		return e_NG;
	int ret  = e_NG;
	int thresholdMode =  m_pInspParam->threshMode;
	BOOL isPolarity = m_pInspParam->isPolarity;

#if Timecheck
	CQTimer qtm; 
#endif 
	//huj 2014/03/31  ±Ø¼º°Ë»ç¸¦ ÇÒ°æ¿ì´Â 180µµ È¸ÀüÇØ¼­ °Ë»çÇÏÁö ¾Ê´Â´Ù.
	switch(stepID2)
	{
	case eOStepID2_USERVALUE:// USER_DEFINED (user value)
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
#if Timecheck
			qtm.StartTick();
			ret = UserValueInsp(FALSE);
			m_bReRotate = false;
			StepID_USERVALUE = (qtm.EndTick() * 1000.0) + StepID_USERVALUE;
#else
			m_bReRotate = false;
			ret = UserValueInsp(FALSE);
#endif
			m_nUserValueinsp_in++;
		}
		break;
	case eOStepID2_USERVALUE_REV: 
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			if(isPolarity == FALSE)
			{
#if Timecheck
				qtm.StartTick();
				ret = UserValueInsp(TRUE);
				m_bReRotate = true;
				StepID_USERVALUE_REV = (qtm.EndTick() * 1000.0) + StepID_USERVALUE_REV;
#else
				m_bReRotate = true;
				ret = UserValueInsp(TRUE);
#endif
				m_nUserValueinsp_in++;
			}
		}
		break;
	case eOStepID2_RESEGVALUE:// LOCAL -> LOCAL_WITH_RESEG -> USER_DEFINED (auto value for matrox)
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
#if Timecheck
			qtm.StartTick();
			ret = AutoValueInsp(FALSE);
			StepID_AUTO = (qtm.EndTick() * 1000.0) + StepID_AUTO;
			m_bReRotate = false;
#else
			m_bReRotate = false;
			ret = RESEGValueInsp(FALSE);
#endif
			m_nAutoValueInsp_in++;
		}
		break;
	case eOStepID2_Local: // LOCAL -> LOCAL_WITH_RESEG -> USER_DEFINED (auto value for matrox)
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
#if Timecheck
			qtm.StartTick();
			ret = LocalValueInsp(FALSE);
			StepID_AUTO = (qtm.EndTick() * 1000.0) + StepID_AUTO;
			m_bReRotate = false;
#else
			m_bReRotate = false;
			ret = LocalValueInsp(FALSE);
#endif
			m_nAutoValueInsp_in++;
		}
		break;
	case eOStepID2_RESEGVALUE_REV: 
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			if(isPolarity == FALSE){

#if Timecheck
				qtm.StartTick();
				ret = AutoValueInsp(TRUE);
				StepID_AUTO_REV = (qtm.EndTick() * 1000.0) + StepID_AUTO_REV;
				m_bReRotate = true;
#else
				m_bReRotate = true;
				ret = AutoValueInsp(TRUE);
#endif
				m_nAutoValueInsp_in++;
			}
		}
		break;
	case eOStepID2_Local_REV: 
		{
			if (m_pInspParam->m_nImageMixCount > 0) break;	//LJH 2016.06.09 합칠 영상이 있는 경우 Skip
			if(isPolarity == FALSE){

#if Timecheck
				qtm.StartTick();
				ret = LocalValueInsp(TRUE);
				StepID_AUTO_REV = (qtm.EndTick() * 1000.0) + StepID_AUTO_REV;
				m_bReRotate = true;
#else
				m_bReRotate = true;
				ret = LocalValueInsp(TRUE);
#endif
				m_nAutoValueInsp_in++;
			}
		}
		//LJH 2016.05.27 추가
	case eOStepID2_MIX_PEMVALUE:
		{
			//LJH 2016.06.09	추가 영상이 있는 경우
			int nType = m_pInspParam->m_nUseImageMixType;
			if (m_pInspParam->m_nImageMixCount > 0)
			{
#if Timecheck
				qtm.StartTick();
#endif
				m_bReRotate = false;
				if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MIX_INSP))		{ret = PemToValueInsp_ImageMix(FALSE);m_nPemToValueInsp_ImageMix_in++;}			//영상을 합쳐서 사용할 경우
				if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MULTI_INSP))	{ret = PemToValueInsp_ImageMulti(FALSE, FALSE);m_nPemToValueInsp_ImageMulti_in;}	//추가한 영상을 여러번 검사할 경우
				if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MULTI_INSP))	{ret = PemToValueInsp_ImageMulti(FALSE, TRUE);m_nPemToValueInsp_ImageMulti_in;}	//추가한 영상을 여러번 검사할 경우
#if Timecheck
				StepID_MIX_PEMVALUE = (qtm.EndTick() * 1000.0) + StepID_MIX_PEMVALUE;
#endif
			}
		}
		break;

		///LJH 2016.05.27
	case eOStepID2_MIX_PEMVALUE_REV:
		{
			if(isPolarity == FALSE)
			{
				//LJH 2016.06.09	추가 영상이 있는 경우
				int nType = m_pInspParam->m_nUseImageMixType;
				if (m_pInspParam->m_nImageMixCount > 0)
				{
#if Timecheck
					qtm.StartTick();
#endif
					m_bReRotate = true;
					if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MIX_INSP))		{ret = PemToValueInsp_ImageMix(TRUE);m_nPemToValueInsp_ImageMix_in++;}			//영상을 합쳐서 사용할 경우
					if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MULTI_INSP))	{ret = PemToValueInsp_ImageMulti(TRUE, FALSE);m_nPemToValueInsp_ImageMulti_in;}	//추가한 영상을 여러번 검사할 경우
					if (ret == e_NG && (nType == TYPE_IMAGE_ALL_INSP || nType == TYPE_IMAGE_MULTI_INSP))	{ret = PemToValueInsp_ImageMulti(TRUE, TRUE);m_nPemToValueInsp_ImageMulti_in;}	//추가한 영상을 여러번 검사할 경우
#if Timecheck
					StepID_MIX_PEMVALUE_REV = (qtm.EndTick() * 1000.0) + StepID_MIX_PEMVALUE_REV;
#endif
				}
			}
		}
		break;
	}

	return ret;
}


int CPInsp_OCR::TeachingConSecutiveSpace(RstAlgoOCR * sRstAlgo, int nInspectionMode)
{
	if(bIsPilBuff)
		return e_NG;
	bool OKSpace[4] = {false,false,false,false};
	int ret = e_NG;
	RstAlgoOCR RstTmp;
	RstAlgoOCR NGRstTmp;
	int NGScore = 0;
	int sizex = m_roiSizeX;
	int sizey = m_roiSizeY;

	NGRstTmp.m_bOKString = false;
	NGRstTmp.m_bOKScore = false;
	NGRstTmp.charWidth[0] - 0; // 2017.02.03 shkim OCR result draw function use

	for(int i = 0;i<4;i++)
	{
		m_procMil->SetConsecutiveSpace(i);
		m_pInspParam->m_nconsecutiveSpace = i;
		//Preprocess();
		m_roiSizeX = sizex;
		m_roiSizeY = sizey;
		ret = InspProc_OCR2(sRstAlgo,nInspectionMode);

		if(ret != e_NG)
		{
			OKSpace[i] = true;
			int MaxWidth = 0;
			for(int j = 1;j<m_result.charCount;j++)
			{
				int width = m_result.charX[j] - (m_result.charX[j-1]+m_result.charWidth[j-1]);
				if(MaxWidth<width)
				{
					MaxWidth = width;
				}
			}
			double fontWidth = m_procMil->GetFontWidth();
			if(OKSpace[3]&&MaxWidth/2.7>fontWidth)
			{
				consecute_tmp = 3;
				memcpy(&RstTmp,sRstAlgo,sizeof(RstAlgoOCR));
			}
			else if(OKSpace[2]&&MaxWidth/1.7>fontWidth)
			{
				consecute_tmp = 2;
				memcpy(&RstTmp,sRstAlgo,sizeof(RstAlgoOCR));
			}
			else if(OKSpace[1]&&MaxWidth/0.7>fontWidth)
			{
				consecute_tmp = 1;
				memcpy(&RstTmp,sRstAlgo,sizeof(RstAlgoOCR));
			}
			else if(OKSpace[0])
			{
				consecute_tmp = 0;
				memcpy(&RstTmp,sRstAlgo,sizeof(RstAlgoOCR));
			}
		}
		else
		{
			for(int charCnt = 0;charCnt<sRstAlgo->m_nCharCount;charCnt++)
			{
				if(sRstAlgo->m_cArrStr[charCnt] == '?' || charCnt == 0)
					memcpy(&NGRstTmp,sRstAlgo,sizeof(RstAlgoOCR));
			}
		}
	}
	if(OKSpace[0]==1 || OKSpace[1]==1 || OKSpace[2]==1 || OKSpace[3]==1){

		memcpy(sRstAlgo,&RstTmp,sizeof(RstAlgoOCR));
		ret = e_OK;
	}
	else
	{
		memcpy(sRstAlgo,&NGRstTmp,sizeof(RstAlgoOCR));
		ret = e_NG;
	}
//	SaveFont(_T("D:\\Eagle3D_data\\OCR_FONT.msr"));
	sRstAlgo->m_nconsecutiveSpace = consecute_tmp;
	return ret;
}

//************************************
// Method:    CalcBlob
// FullName:  CPInsp_OCR::CalcBlob
// Access:    public 
// Returns:   int
// Qualifier: Read OCR 전에 Blob과 Morphology를 하기위한 함수
// Parameter: UCHAR * ptrFontImg >> blob을 할 이미지인 동시에 결과를 받을 배열
// Parameter: int nSizeX
// Parameter: int nSizeY
//************************************
int CPInsp_OCR::CalcBlob( UCHAR* pUcArrFontImg, int nSizeX, int nSizeY, int nFilterMor/*=-1*/ )
{
	int widthStep = nSizeX;
	/*if (widthStep % 4 != 0)
	{
		widthStep = nSizeX + 4 - (widthStep % 4);
	}*/
	widthStep = g_pMPTI->nCalcWidthStep(false, nSizeX);

	CString sReadOCR = _T("");


	Im::PIL_ID milOCR = m_procMil->AllocBuff(nSizeX, nSizeY);
	MbufPut(milOCR, pUcArrFontImg);

	int nAreaMin = nSizeX * nSizeY  * 0.01;
	int nCntBlob = m_procMil->CalcBlob(milOCR, nAreaMin, 0, FALSE, FALSE, milOCR, nFilterMor);

	MbufGet(milOCR, pUcArrFontImg);

	m_procMil->SaveWorkImg(pUcArrFontImg, nSizeX, nSizeY, _T("BlobResultOCR.bmp"));

	m_procMil->FreeMilImageBuff(&milOCR);	
	return nCntBlob;
}

int CPInsp_OCR::AddFontIndex(Im::PIL_ID milContext)
{
	if(bIsPilBuff)
		return 0;
	m_procMil->AddFont();
	return m_procMil->GetFontCount(milContext);
}

void CPInsp_OCR::GetAllposition()
{
	BOOL isPolarity = m_pInspParam->isPolarity;
	CString targetStr = m_pInspParam->targetString;
	int stdCharCount = targetStr.GetLength();
	double stdScore = m_pInspParam->stdStrScore;
	BOOL useCharScore = m_pInspParam->useCharScore;

	int strCount = 0;
	int charCount = 0;
	double stringScore = 0; 
	double charScore[MAX_STRLEN] = {0.0,};
	double strPosX = 0;
	double strPosY = 0;
	double strAngle = 0;
	CString strTemp = _T("");
	double x[MAX_STRLEN] = {0.0,};
	double y[MAX_STRLEN] = {0.0,};
	double w[MAX_STRLEN] = {0.0,};
	double h[MAX_STRLEN] = {0.0,};
	double angle[MAX_STRLEN] = {0.0,};

	m_procMil->GetStringCount(&strCount);
	m_procMil->GetCharCount(&charCount);
	m_procMil->GetStringScore(&stringScore,m_nstringIndex);
	m_procMil->GetCharScore(charScore,M_ALL);
	m_procMil->GetStringPos(&strPosX, &strPosY,m_nstringIndex);
	m_procMil->GetStringAngle(&strAngle,m_nstringIndex);
	strTemp = m_procMil->GetString(m_nstringIndex);

	m_procMil->SetImgSize(m_roiSizeX,m_roiSizeY);
	m_procMil->GetCharPos(x, y,w,h,angle, M_ALL,m_bReRotate);

	memcpy(m_result.charScore,charScore,sizeof(double)*charCount);

	memcpy(m_result.charX,x,sizeof(double)*MAX_STRLEN);
	memcpy(m_result.charY,y,sizeof(double)*MAX_STRLEN);
	memcpy(m_result.charWidth,w,sizeof(double)*MAX_STRLEN);
	memcpy(m_result.charHeight,h,sizeof(double)*MAX_STRLEN);
	memcpy(m_result.charAngle,angle,sizeof(double)*MAX_STRLEN);
}


#pragma endregion _BASE FUNCTION_