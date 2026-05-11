#include "StdAfx.h"
#include "MPTI.h"
#include "ippi.h"

// 20140913 SHW : IPP 평균 구하기에 사용
#include "ipps.h"
#include "ippi.h"
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")

#include <limits>
#include <immintrin.h>
#include<cmath>
#include "PInsp_ForeignPattern.h"

using namespace std;

CPInsp_ForeignPattern::CPInsp_ForeignPattern(void)
{
	m_procPil = std::shared_ptr<CProcPil_ForeignPattern>(new CProcPil_ForeignPattern());
	m_pInspAlgo= NULL;
	InspImg = NULL;

	memset(m_pMatchPos, 0, 4 * sizeof(POINTF));
}

CPInsp_ForeignPattern::~CPInsp_ForeignPattern(void)
{
	if(InspImg!=NULL)
	{
		//delete [] InspImg;
		g_pMManager->pem_delete(InspImg, true);
		InspImg = NULL;
	}
}

int CPInsp_ForeignPattern::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	m_procPil->InitMil(milApp, milSys, bUseImagePilLib);

	return true;
}

int CPInsp_ForeignPattern::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	// Allocate a Application.
	//m_milApp = milApp;
	// Allocate a System.
	//m_milSys = milSys;

	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	m_procPil->InitMil(milApp, milSys,bUseImagePilLib);
	m_procPil->SetResol(resolX, resolY, m_fovWidth);

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY,bUseImagePilLib);


	return ePART_SUCCESS;
}
int CPInsp_ForeignPattern::SetInspAlgo(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, double dAngle)
{
	if(sInspAlgo.m_eAlgoType != eAlgoBlob)
		return ePART_FAIL;
	m_pInspAlgo = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pInspAlgo)
		return ePART_FAIL;

 	int  InspImgwidth, InspImgheight;
	InspImgwidth = sWndAlgoImg.m_nWidth;
	InspImgheight = sWndAlgoImg.m_nHeight;

	//m_procPil->RotateImg_ipp(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, dAngle, &InspImg, &InspImgwidth, &InspImgheight);

	_dAngle = dAngle;

	int widthStep = InspImgwidth;
	//int nSizeX = widthStep % 4;
	//if (nSizeX!=0)
	//	widthStep += 4 - nSizeX;
	widthStep = g_pMPTI->nCalcWidthStep(false, InspImgwidth);

	{m_mInspImg = cv::Mat(InspImgheight, InspImgwidth, CV_8UC1,sWndAlgoImg.m_ucArr2D/*,widthStep*/);}
	//cv::imwrite("D:\\testimage\\Pim\\set_m_mInspImg.bmp",m_mInspImg);
	return ePART_SUCCESS;
}
int CPInsp_ForeignPattern::SetInspAlgo(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, double dAngle,cv::Rect re,bool bIsMin)
{
	if(sInspAlgo.m_eAlgoType != eAlgoBlob)
		return ePART_FAIL;
	m_pInspAlgo = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pInspAlgo)
		return ePART_FAIL;

	if(InspImg!=NULL)
	{
		//delete [] InspImg;
		g_pMManager->pem_delete(InspImg, true);
		InspImg = NULL;
	}
	int  InspImgwidth, InspImgheight;
	InspImgwidth = sWndAlgoImg.m_nWidth;
	InspImgheight = sWndAlgoImg.m_nHeight;

	CPInsp::FillMaskingROI(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, sWndAlgoImg.dAngle, sWndAlgoImg.m_ucArr2D, sInspAlgo.m_nUsedMaskingValue, sInspAlgo.m_rcArrMaskingROI);

	dAngle = 360-dAngle;
	if(dAngle>360)
		dAngle = dAngle - 360;
	else if(dAngle<0)
		dAngle = dAngle + 360;

	m_procPil->RotateImg_ipp(sWndAlgoImg.m_ucArr2D, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight, dAngle, &InspImg, &InspImgwidth, &InspImgheight);
	_dAngle = dAngle;
	int widthStep = InspImgwidth;
	/*int nSizeX = widthStep % 4;
	if (nSizeX!=0)
		widthStep += 4 - nSizeX;*/
	widthStep = g_pMPTI->nCalcWidthStep(false, InspImgwidth);

	cv::Mat modelImg= cv::Mat(InspImgheight, InspImgwidth, CV_8UC1,InspImg,widthStep);

	if(modelImg.rows<=re.height+re.y || modelImg.cols <= re.width + re.x)
		m_mInspImg = modelImg.clone();
	else
		m_mInspImg = modelImg(re).clone();
	int FillValue = 0;
	if(bIsMin)
	{
		FillValue = 255;
	}
	CPInsp::FillMaskingROI(m_mInspImg.cols, m_mInspImg.rows, sWndAlgoImg.dAngle, m_mInspImg.data, sInspAlgo.m_nUsedMaskingValue, sInspAlgo.m_rcArrMaskingROI,FillValue);
//	cv::imwrite("D:\\Eagle3D_data\\Temp\\set_m_mInspImg.bmp",m_mInspImg);
	return ePART_SUCCESS;
}

int CPInsp_ForeignPattern::SetInspAlgo(const InspAlgo &sInspAlgo, UCHAR* ucImage, int nWidth, int nHeight, double dPartAngle, double dAngle)
{
	if(sInspAlgo.m_eAlgoType != eAlgoBlob)
		return ePART_FAIL;
	m_pInspAlgo = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pInspAlgo)
		return ePART_FAIL;

	int  InspImgwidth, InspImgheight;
	InspImgwidth = nWidth;
	InspImgheight = nHeight;

	//	dAngle = 360-dAngle;
	if(dAngle>360)
		dAngle = dAngle - 360;
	else if(dAngle<0)
		dAngle = dAngle + 360;

//	CPInsp::FillMaskingROI(nWidth, nHeight, dPartAngle, ucImage, sInspAlgo.m_nUsedMaskingValue, sInspAlgo.m_rcArrMaskingROI);

	_dAngle = dAngle;

	int widthStep = InspImgwidth;
	/*int nSizeX = widthStep % 4;
	if (nSizeX!=0)
		widthStep += 4 - nSizeX;*/
	widthStep = g_pMPTI->nCalcWidthStep(false, widthStep);

	{m_mInspImg = cv::Mat(InspImgheight, InspImgwidth, CV_8UC1,ucImage/*,widthStep*/);}
	//cv::imwrite("D:\\testimage\\Pim\\set_m_mInspImg.bmp",m_mInspImg);
	return ePART_SUCCESS;
}
int CPInsp_ForeignPattern::InspForeign(bool bCheckBW)
{
	int ret = e_NG;
	int nLine = __LINE__;
	if(m_pInspAlgo->m_bUsePattern == FALSE)
		return ret;
	if(bCheckBW && m_pInspAlgo->m_bUseFPBW == TRUE)
		return ret;

	memset(m_pMatchPos, 0, sizeof(POINTF) * 4);

	try
	{
		if(m_pInspAlgo == nullptr)
			return e_NG;
		CString strFullPath;
		strFullPath.Format(_T("%s"), m_pInspAlgo->m_sModelPath);
		int nr = strFullPath.Find(_T("InspectResult"));
		if(nr != -1){

			if(m_procPil->ModelFileLoad(strFullPath)== ePART_FAIL)
			{
				return e_NG;
			}
		}
		else if(m_procPil->ModelLoad(strFullPath)==ePART_FAIL)
		{
			if(m_procPil->ModelFileLoad(strFullPath)== ePART_FAIL)
			{
				return e_NG;
			}
		}

		if(g_pMPTI->m_bLengthByAngel && m_pInspAlgo->m_bUseFPBW)
			m_procPil->SearchForeignByMask(m_mInspImg, m_pInspAlgo->m_bIsModelSubImg, _dAngle, m_pMatchPos);
		else
			m_procPil->SearchForeign(m_mInspImg,m_pInspAlgo->m_bIsModelSubImg,_dAngle);
#ifdef _DEBUG
		CString temp;
		temp.Format(_T("%s.bmp"), m_pInspAlgo->m_sModelPath);
		imwrite(std::string(CT2A(temp)),m_mInspImg);
#endif
	}
	catch(...)
	{

		CString Msg;
		Msg.Format(_T("CPInsp_ForeignPattern::InspForeign error!!! Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}

	return ret;
}
int CPInsp_ForeignPattern::TeachingModel(cv::Mat src, CString sPath,bool isNew,bool bIsMin,double dAngle, int nMargin)
{
	int ret = e_OK;
	int nLine = __LINE__;
	try
	{
		if(isNew)
		{
			m_procPil->NewModel(src,sPath,dAngle,nMargin);
			return ret;
		}

		int nr = sPath.Find(_T("InspectResult"));
		if(nr != -1){

			if(m_procPil->ModelFileLoad(sPath)== ePART_FAIL)
			{
				m_procPil->m_gModelFile = std::shared_ptr<ModelFile_Foreign>(new ModelFile_Foreign(eAlgoBlob));
				m_procPil->m_gImModel = shared_ptr<ImgModels>(new ImgModels());
			}
		}
		else if(m_procPil->ModelLoad(sPath)==ePART_FAIL)
		{
			if(m_procPil->ModelFileLoad(sPath)== ePART_FAIL)
			{
				m_procPil->m_gModelFile = std::shared_ptr<ModelFile_Foreign>(new ModelFile_Foreign(eAlgoBlob));
				m_procPil->m_gImModel = shared_ptr<ImgModels>(new ImgModels());
			}
		}
		m_procPil->teachingModel(src,m_mInspImg.clone(),sPath,bIsMin,dAngle, nMargin);
	}
	catch(...)
	{

		CString Msg;
		Msg.Format(_T("CPInsp_ForeignPattern::TeachingModel error!!! Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}
	return ret;
}

int CPInsp_ForeignPattern::resultImage(cv::Mat src, CString sPath,bool flag,double dAngle)
{
	int ret = e_NG;
	int nLine = __LINE__;
	try
	{

		int nr = sPath.Find(_T("InspectResult"));
		if(nr != -1){

			if(m_procPil->ModelFileLoad(sPath)== ePART_FAIL)
			{
				return e_NG;
			}
		}
		else if(m_procPil->ModelLoad(sPath)==ePART_FAIL)
		{
			if(m_procPil->ModelFileLoad(sPath)== ePART_FAIL)
			{
				return e_NG;
			}
		}

		m_procPil->resultImage(src, sPath,flag,dAngle);
	}
	catch(...)
	{

		CString Msg;
		Msg.Format(_T("CPInsp_ForeignPattern::InspForeign error!!! Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}

	return ret;
}

int CPInsp_ForeignPattern::ModelImage_Load(CString sPath,double dAngle)
{
	int ret = e_NG;
	int nLine = __LINE__;
	try
	{

		int nr = sPath.Find(_T("InspectResult"));
		if(nr != -1){

			if(m_procPil->ModelFileLoad(sPath)== ePART_FAIL)
			{
				return e_NG;
			}
		}
		else if(m_procPil->ModelLoad(sPath)==ePART_FAIL)
		{
			if(m_procPil->ModelFileLoad(sPath)== ePART_FAIL)
			{
				return e_NG;
			}
		}

		m_procPil->ModelImage(sPath,dAngle);
	}
	catch(...)
	{

		CString Msg;
		Msg.Format(_T("CPInsp_ForeignPattern::ModelImage_Load error!!! Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}

	return ret;
}

void CPInsp_ForeignPattern::PatternModelAndOption(bool flag, int erodeSize)
{
	m_procPil->SetAndFlag = flag;
	m_procPil->erodeSize = erodeSize;
}
void CPInsp_ForeignPattern::SetForeingPattern_MaskParam(bool bUseLineFilter, int dilateSize, bool bUseDisplay)
{
	m_procPil->m_bUseFilterUnit = bUseLineFilter;
	m_procPil->m_nFilterUnitSize = dilateSize;
	m_procPil->m_bDispalyMatchPos = bUseDisplay;
}
void CPInsp_ForeignPattern::SetLineImg(cv::Mat LineMat, cv::Point pCenter)
{
	if(m_procPil->LineImg.empty() == false)
		m_procPil->LineImg.release();
	m_procPil->LineImg = LineMat.clone();
	m_procPil->pLinCenter = pCenter;
}
void CPInsp_ForeignPattern::GetForeignPatternMatchPos(POINTF* srcPtr)
{
	if(srcPtr == NULL)
		return;

	for (int i = 0; i < 4; i++)
	{
		srcPtr[i] = m_pMatchPos[i];
	}
}