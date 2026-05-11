#include "StdAfx.h"
#include "ProcPil_ForeignPattern.h"
#include "MPTI.h"
#include <math.h>

#define PAT_ACCURACY	M_HIGH//M_MEDIUM
#define PAT_SPEED		M_HIGH//M_MEDIUM 
#define  PAT_ATCSCORE		0.0

#ifdef _DEBUG
#define    _PROC_IMG_SAVE
#endif

using namespace std;

void LogModel(CString msg)
{
	
}

CProcPil_ForeignPattern::CProcPil_ForeignPattern(void)
{
	m_className = _T("CProcPil_ForeignPattern");

	m_ModelMng = std::shared_ptr<CMModelManager<ModelFile_Foreign>>(new CMModelManager<ModelFile_Foreign>(eAlgoBlob, _T(".pim"), LogModel));

	SetAndFlag = false;
	erodeSize = 5;

	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
}
CProcPil_ForeignPattern::~CProcPil_ForeignPattern(void)
{
	g_pMManager->pem_delete_check(this);
}

int CProcPil_ForeignPattern::ModelList_Load(CString strPath)
{

	int ret = ePART_SUCCESS;

	try
	{
		if (!m_ModelMng->LoadFileList(strPath,m_resolX,m_resolY))
			ret = ePART_FAIL;
	}
	catch(...)
	{
		CString Msg;
		Msg.Format(_T("CProcPil_ForeignPattern::ModelList_Load(CString strPath)!!!"));
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}

	return ret;
}

int CProcPil_ForeignPattern::ModelList_Load(fileAlgoPath* strPath, int cnt)
{

	int ret = ePART_SUCCESS;

	try
	{
		if (!m_ModelMng->LoadFileList(strPath,cnt,m_resolX,m_resolY))
			ret = ePART_FAIL;
	}
	catch(...)
	{
		CString Msg;
		Msg.Format(_T("CProcPil_ForeignPattern::ModelList_Load(fileAlgoPath* strPath, int cnt)!!!"));
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}
	return ret;
}

int CProcPil_ForeignPattern::ModelList_Check(CString strPath)
{

	int ret = ePART_SUCCESS;

	try
	{
		if (!m_ModelMng->CheckFileList(strPath,m_resolX,m_resolY))
			ret = ePART_FAIL;

	}
	catch(...)
	{
		CString Msg;
		Msg.Format(_T("CProcPil_ForeignPattern::ModelList_Check!!!"));
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}
	return ret;
}
int CProcPil_ForeignPattern::ModelList_Clear(int nLane)
{
	int ret = ePART_SUCCESS;

	try
	{
		ret = m_ModelMng->Clear_File(nLane);
	}
	catch(...)
	{
		CString Msg;
		Msg.Format(_T("CProcPil_ForeignPattern::ModelList_Clear!!!"));
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}

	return ret;
}
int CProcPil_ForeignPattern::ModelLoad(CString strPath)
{

	int ret = ePART_SUCCESS;
	int nLine = __LINE__;
	try
	{

		ClearModel();
		nLine = __LINE__;

		CString strModelPath = _T("");
		strModelPath = strPath + _T(".pim");
		strPath.MakeLower();
		nLine = __LINE__;
		m_gModelFile = m_ModelMng->GetModel(strPath);
		nLine = __LINE__;
		if(m_gModelFile.get() == NULL)
			return ePART_FAIL;
		nLine = __LINE__;
		m_gImModel = m_gModelFile->GetAngleModel(0);
	}
	catch(...)
	{
		CString Msg;
		Msg.Format(_T("CProcPil_ForeignPattern::ModelLoad error!!! Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}

	return ret;
}

int CProcPil_ForeignPattern::ModelFileLoad(CString strPath)
{

	int ret = ePART_SUCCESS;
	int nLine = __LINE__;
	try{

		CString strModelPath = _T("");
		strModelPath = strPath;// + _T(".psr");
		nLine = __LINE__;
		if(FileExists(strModelPath))
		{
			nLine = __LINE__;
			m_gModelFile = std::shared_ptr<ModelFile_Foreign>(new ModelFile_Foreign(eAlgoBlob));
			m_gModelFile->LoadFile(strModelPath,m_resolX,m_resolY);
			nLine = __LINE__;
		}
		else
		{
			nLine = __LINE__;
			ret = ePART_FAIL;
			return ret;
		}

		if(m_gModelFile!=nullptr && m_gModelFile->milModel != nullptr)
			m_gImModel = m_gModelFile->milModel;
	}
	catch(...)
	{
		CString Msg;
		Msg.Format(_T("CProcPil_ForeignPattern::ModelFileLoad error!!! Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(Msg);
		throw;
	}

	return ret;
}

int CProcPil_ForeignPattern::SearchForeign(cv::Mat src,bool flag,double dAngle)
{
	int re = e_NG;

	if (m_gImModel == nullptr)
		return re;

	UCHAR* matchingArray = NULL, *mergeArray = NULL;
	int InspImgwidth, InspImgheight;

	dAngle = 360 - dAngle;

	RotateImg_ipp(m_gImModel->_MatchingImg.data, m_gImModel->_MatchingImg.cols, m_gImModel->_MatchingImg.rows, dAngle, &matchingArray, &InspImgwidth, &InspImgheight);
	RotateImg_ipp(m_gImModel->_mergedModel.data, m_gImModel->_mergedModel.cols, m_gImModel->_mergedModel.rows, dAngle, &mergeArray, &InspImgwidth, &InspImgheight);

	cv::Mat _MatchingImg(InspImgheight, InspImgwidth, CV_8UC1, matchingArray), _mergedModel(InspImgheight, InspImgwidth, CV_8UC1, mergeArray);
	cv::Mat score_M, result, tmp;
	cv::Point	ptMin, ptMax;
	double		Min(0.0), Max(0.0);

	if (_MatchingImg.cols > src.cols || _MatchingImg.rows > src.rows)
	{
		if (matchingArray != NULL)
			g_pMManager->pem_delete(matchingArray, true);
		if (mergeArray != NULL)
			g_pMManager->pem_delete(mergeArray, true);

		return e_NG;
	}

	if (SetAndFlag)
	{
		ptMax.x = (src.cols - _mergedModel.cols) / 2;
		ptMax.y = (src.rows - _mergedModel.rows) / 2;
	}
	else
	{
		cv::matchTemplate(src, _MatchingImg, score_M, cv::TM_CCOEFF_NORMED);
		cv::minMaxLoc(score_M, &Min, &Max, &ptMin, &ptMax);
	}

	cv::Rect tmp_rect(ptMax.x, ptMax.y, _mergedModel.cols, _mergedModel.rows);

	result = src(tmp_rect);
	if (SetAndFlag && erodeSize > 0)
	{
		int sizemin = tmp_rect.width < tmp_rect.height ? tmp_rect.width : tmp_rect.height;
		int dilation_size = sizemin / 500 > 5 ? sizemin / 100 : 5;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
			cv::Point(dilation_size, dilation_size));
		int PntIdx = (int)(erodeSize / 2 + 0.3);
		cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(erodeSize, erodeSize), cv::Point(PntIdx, PntIdx));
		cv::erode(_mergedModel, _mergedModel, kernel);
		cv::erode(result, result, kernel2);
	}
#if _DEBUG
	cv::imwrite("D:\\testimage\\Pim\\searchImage_src.bmp", src);
	cv::imwrite("D:\\testimage\\Pim\\searchImage_mergedModel.bmp", _mergedModel);
	cv::imwrite("D:\\testimage\\Pim\\searchImage_template.bmp", result);
#endif

	if (_mergedModel.rows != result.rows || _mergedModel.cols != result.cols)
	{
		if (matchingArray != NULL)
			g_pMManager->pem_delete(matchingArray, true);
		if (mergeArray != NULL)
			g_pMManager->pem_delete(mergeArray, true);

		return e_NG;
	}

	if (SetAndFlag)
		tmp = result & _mergedModel;
	else
	{
		if (flag)
			tmp = _mergedModel - result;
		else
			tmp = result - _mergedModel;
	}


	src.setTo(0);
	tmp.copyTo(result);



#if _DEBUG
	cv::imwrite("D:\\testimage\\Pim\\searchImage_2tmp.bmp", tmp);
	cv::imwrite("D:\\testimage\\Pim\\searchImage_3src.bmp", result);
#endif

	if (matchingArray != NULL)
		g_pMManager->pem_delete(matchingArray, true);
	if (mergeArray != NULL)
		g_pMManager->pem_delete(mergeArray, true);

	return re;
}

void CProcPil_ForeignPattern::ClearModel()
{
	if(m_gImModel==nullptr)
		return;
	m_gImModel.reset();

	return;
}

int CProcPil_ForeignPattern::teachingModel(cv::Mat src,cv::Mat maskImg,CString sPath,bool bIsMin,double dAngle, int nMargin)
{
	int re = e_NG;
	int _Line = __LINE__;
	try
	{
		if(m_gModelFile!=nullptr && m_gModelFile->milModel != nullptr)
			m_gImModel = m_gModelFile->milModel;
		if((m_gImModel->_mergedModel.cols==0 && m_gImModel->_mergedModel.rows == 0) || m_gImModel->ModelCnt <1)
		{
			NewModel(src,sPath,dAngle, nMargin);
			return e_OK;
		}

		UCHAR* matchingArray = NULL,* MaskingArray = NULL;
		int InspImgwidth, InspImgheight;

		//dAngle = 360-dAngle;

		RotateImg_ipp(src.data, src.cols, src.rows, dAngle, &matchingArray, &InspImgwidth, &InspImgheight);
		cv::Mat _SRCImg(InspImgheight,InspImgwidth,CV_8UC1,matchingArray);

		RotateImg_ipp(maskImg.data, maskImg.cols, maskImg.rows, dAngle, &MaskingArray, &InspImgwidth, &InspImgheight);
		cv::Mat _maskImg(InspImgheight,InspImgwidth,CV_8UC1,MaskingArray);

		double cnt = m_gImModel->ModelCnt;
	//	m_gImModel->_mergedModel = cv::imread("D:\\testimage\\_wndAlgoImg.bmp");
		cv::Mat tempModel; 
		cv::Mat tmp = m_gImModel->_mergedModel;
		double dAlpha = cnt/(cnt+1);
		double dBeta = 1.0/(cnt+1);


		cv::Mat score_M,result;
		cv::Point	ptMin, ptMax;
		double		Min(0.0), Max(0.0);
		_Line = __LINE__;

		if(m_gImModel->_MatchingImg.cols>_SRCImg.cols || m_gImModel->_MatchingImg.rows >_SRCImg.rows)
		{
			return e_NG;
		}

		if (SetAndFlag)
		{
			ptMax.x = (_SRCImg.cols - tmp.cols) / 2;
			ptMax.y = (_SRCImg.rows - tmp.rows) / 2;
		}
		else
		{
			cv::matchTemplate(_SRCImg, m_gImModel->_MatchingImg, score_M, cv::TM_CCOEFF_NORMED);
			cv::minMaxLoc(score_M, &Min, &Max, &ptMin, &ptMax);
		}
		_Line = __LINE__;
		cv::Rect tmp_rect(ptMax.x,ptMax.y,m_gImModel->_MatchingImg.cols,m_gImModel->_MatchingImg.rows);
		_Line = __LINE__;
		result = _maskImg(tmp_rect).clone();
		_Line = __LINE__;
#if _DEBUG
		cv::imwrite("D:\\testimage\\Pim\\teachingModel_src.bmp",src);
		cv::imwrite("D:\\testimage\\Pim\\teachingModel_maskImg.bmp",_maskImg);
		cv::imwrite("D:\\testimage\\Pim\\teachingModel_template.bmp",result);
		cv::imwrite("D:\\testimage\\Pim\\teachingModel_merge.bmp",tmp);
#endif
		if(cnt<1)
		{
			tempModel = src(cv::Rect(_PIXEL_MARGIN,_PIXEL_MARGIN,src.cols-(_PIXEL_MARGIN*2),src.rows-(_PIXEL_MARGIN*2)));
			m_gImModel->_MatchingImg = tempModel.clone();
		}
		else
		{
			//test = tmp*dAlpha + src*dBeta;
			//cv::addWeighted(tmp,dAlpha,result,dBeta,0,tempModel);

			tempModel = result;
// 			tempModel = tmp.clone();
// 			imwrite("D:\\testimage\\Pim\\0_teachModel_result.bmp",result);
// 			imwrite("D:\\testimage\\Pim\\0_teachModel_tmp.bmp",tmp);
// 			imwrite("D:\\testimage\\Pim\\0_teachModel_tempModel.bmp",tempModel);

			for(int i=0;i<result.rows;i++)
			{
				for(int j=0;j<result.cols;j++)
				{
					if(SetAndFlag)
					{
						//max
// 						if(bIsMin == false)
// 						{
// 							if(result.data[result.step*i + j]==255 || tmp.data[tmp.step*i + j]==255)
// 								tempModel.data[tempModel.step*i + j] = 255;
// 							else
// 								tempModel.data[tempModel.step*i + j] = 0;
// 						}
// 						else //min
						{
							if(result.data[result.step*i + j]==255 /*&& tmp.data[tmp.step*i + j]==255*/)
								tempModel.data[tempModel.step*i + j] = 0;
							else if(tmp.data[tmp.step*i + j]==255)
								tempModel.data[tempModel.step*i + j] = 255;
						}
					}
					else
					{
						//max
						if(bIsMin == false)
						{
							if(result.data[result.step*i + j]<tmp.data[tmp.step*i + j])
								tempModel.data[tempModel.step*i + j] = tmp.data[result.step*i + j];
						}
						else //min
						{
							if(result.data[result.step*i + j]>tmp.data[tmp.step*i + j])
								tempModel.data[tempModel.step*i + j] = tmp.data[result.step*i + j];
						}
					}
				}
			}
		}
		_Line = __LINE__;

		UCHAR* _tempModelArray = NULL;

		dAngle = 360-dAngle;

		RotateImg_ipp(tempModel.data, tempModel.cols, tempModel.rows, dAngle, &_tempModelArray, &InspImgwidth, &InspImgheight);
		cv::Mat _tempModel(InspImgheight,InspImgwidth,CV_8UC1,_tempModelArray);


		m_gImModel->_mergedModel = tempModel;
		m_gImModel->ModelCnt = cnt+1;
		m_gImModel->setRes(m_resolX,m_resolY);
		m_gImModel->SaveFile(sPath);
		_Line = __LINE__;
	#ifdef _DEBUG
		m_gImModel->SaveFile(_T("D:\\testimage\\ForeignModel.pim"));
		CString ImgPath;
		ImgPath.Format(_T("%s.bmp"),sPath);
		imwrite(std::string(CT2A(ImgPath)),m_gImModel->_mergedModel);
	#endif

		if(matchingArray!=NULL)
			//delete [] matchingArray;
			g_pMManager->pem_delete(matchingArray, true);
		if(MaskingArray!=NULL)
			//delete [] MaskingArray;
			g_pMManager->pem_delete(MaskingArray, true);
		if(_tempModelArray!=NULL)
			//delete [] _tempModelArray;
			g_pMManager->pem_delete(_tempModelArray, true);
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("%d line"),_Line);
		AfxMessageBox(msg);
	}
	re = e_OK;
	return re;
}

int CProcPil_ForeignPattern::resultImage(cv::Mat src,CString sPath,bool flag,double dAngle)
{
	int re = e_OK;

	if(m_gModelFile!=nullptr && m_gModelFile->milModel != nullptr)
		m_gImModel = m_gModelFile->milModel;
	else if(m_gImModel==nullptr)
	{
		m_gImModel = std::shared_ptr<ImgModels>(new ImgModels());
	}

	UCHAR* matchingArray = NULL,* mergeArray = NULL;
	int InspImgwidth, InspImgheight;

	dAngle = 360-dAngle;

	RotateImg_ipp(m_gImModel->_MatchingImg.data, m_gImModel->_MatchingImg.cols, m_gImModel->_MatchingImg.rows, dAngle, &matchingArray, &InspImgwidth, &InspImgheight);
	RotateImg_ipp(m_gImModel->_mergedModel.data, m_gImModel->_mergedModel.cols, m_gImModel->_mergedModel.rows, dAngle, &mergeArray, &InspImgwidth, &InspImgheight);

	cv::Mat _MatchingImg(InspImgheight,InspImgwidth,CV_8UC1,matchingArray),_mergedModel(InspImgheight,InspImgwidth,CV_8UC1,mergeArray);
	cv::Mat score_M,result,tmp;
	cv::Point	ptMin, ptMax;
	double		Min(0.0), Max(0.0);

	if(_MatchingImg.cols>src.cols || _MatchingImg.rows >src.rows)
	{
		return e_NG;
	}

	if (SetAndFlag)
	{
		ptMax.x = (src.cols - _mergedModel.cols) / 2;
		ptMax.y = (src.rows - _mergedModel.rows) / 2;
	}
	else
	{
		cv::matchTemplate(src, _MatchingImg, score_M, cv::TM_CCOEFF_NORMED);
		cv::minMaxLoc(score_M, &Min, &Max, &ptMin, &ptMax);
	}

	cv::Rect tmp_rect(ptMax.x,ptMax.y,_mergedModel.cols,_mergedModel.rows);
	result = src(tmp_rect);
	if (SetAndFlag && erodeSize > 0)
	{
		int sizemin = tmp_rect.width < tmp_rect.height ? tmp_rect.width : tmp_rect.height;
		int dilation_size = sizemin / 500 > 5 ? sizemin / 100 : 5;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
			cv::Point(dilation_size, dilation_size));
		int PntIdx = (int)(erodeSize / 2 + 0.3);
		cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(erodeSize, erodeSize), cv::Point(PntIdx, PntIdx));
		cv::erode(result, result, kernel);
		cv::erode(_mergedModel, _mergedModel, kernel2);
	}

#if _DEBUG
	cv::imwrite("D:\\testimage\\Pim\\resultImage_template.bmp",result);
#endif


	if(SetAndFlag)
		tmp =result& _mergedModel;
	else
	{
		if (flag)
			tmp = _mergedModel - result;
		else
			tmp = result - _mergedModel;
	}

	src.setTo(0);
	tmp.copyTo(result);

#if _DEBUG
	cv::imwrite("D:\\testimage\\Pim\\resultImage_2tmp.bmp",tmp);
	cv::imwrite("D:\\testimage\\Pim\\resultImage_3src.bmp",src);
	cv::imwrite("D:\\testimage\\Pim\\resultImage_4result.bmp",result);
#endif
	if(matchingArray!=NULL)
		//delete [] matchingArray;
		g_pMManager->pem_delete(matchingArray, true);
	if(mergeArray!=NULL)
		//delete [] mergeArray;
		g_pMManager->pem_delete(mergeArray, true);
	return re;
}

int CProcPil_ForeignPattern::ModelImage(CString sPath,double dAngle)
{
	int re = e_OK;

	if(m_gModelFile==nullptr || m_gModelFile->milModel == nullptr)
		return e_NG;
	if (m_gModelFile->milModel->_mergedModel.data == NULL || m_gModelFile->milModel->_mergedModel.rows <= 0 || m_gModelFile->milModel->_mergedModel.cols <= 0)
		return e_NG;

	CString ImgPath;
	ImgPath.Format(_T("%s.bmp"),sPath);

	UCHAR* mArray = NULL;
	int InspImgwidth, InspImgheight;
	dAngle = 360-dAngle;
	cv::Mat _srcRect = m_gModelFile->milModel->_mergedModel.clone();
	RotateImg_ipp(_srcRect.data, _srcRect.cols, _srcRect.rows, dAngle, &mArray, &InspImgwidth, &InspImgheight);
	cv::Mat _SRCImg(InspImgheight,InspImgwidth,CV_8UC1,mArray);
	
	imwrite(std::string(CT2A(ImgPath)),_SRCImg);
#if _DEBUG

	ImgPath.Format(_T("%s_org.bmp"),sPath);
	imwrite(std::string(CT2A(ImgPath)),m_gModelFile->milModel->_mergedModel);
#endif
	if(mArray!=NULL)
		//delete [] mArray;
		g_pMManager->pem_delete(mArray, true);

	return re;
}

int CProcPil_ForeignPattern::NewModel(cv::Mat src,CString sPath,double dAngle, int nMargin)
{
	int re = e_NG;

	if(m_gImModel==nullptr)
		m_gImModel = std::shared_ptr<ImgModels>(new ImgModels());
	if(nMargin < 5) nMargin = 5;
	if(nMargin > _PIXEL_MARGIN) nMargin = _PIXEL_MARGIN;
	m_gModelFile = std::shared_ptr<ModelFile_Foreign>(new ModelFile_Foreign(eAlgoBlob)); 
	cv::Rect rec(nMargin,nMargin,src.cols-(nMargin*2),src.rows-(nMargin*2));

	UCHAR* mArray = NULL;
	int InspImgwidth, InspImgheight;
	//dAngle = 360-dAngle;
	cv::Mat _srcRect = src(rec).clone();
	RotateImg_ipp(_srcRect.data, _srcRect.cols, _srcRect.rows, dAngle, &mArray, &InspImgwidth, &InspImgheight);
	cv::Mat _SRCImg(InspImgheight,InspImgwidth,CV_8UC1,mArray);

	m_gImModel->_mergedModel = _SRCImg;
	m_gImModel->_MatchingImg = _SRCImg;
	m_gImModel->ModelCnt = 1;
	m_gImModel->setRes(m_resolX,m_resolY);
	m_gImModel->SaveFile(sPath);

#ifdef _DEBUG
	m_gImModel->SaveFile(_T("D:\\testimage\\ForeignModel.pim"));
	CString ImgPath;
	ImgPath.Format(_T("%s.bmp"),sPath);
	imwrite(std::string(CT2A(ImgPath)),m_gImModel->_mergedModel);
#endif

	if(mArray!=NULL)
		//delete [] mArray;
		g_pMManager->pem_delete(mArray, true);

	re = e_OK;
	return re;
}
int CProcPil_ForeignPattern::SearchForeignByMask(cv::Mat src, bool flag, double dAngle, POINTF* pFPMathcPos)
{
	int re = e_NG;

	if (m_gImModel == nullptr)
		return re;

	UCHAR* matchingArray = NULL, *mergeArray = NULL;
	int InspImgwidth, InspImgheight;

	dAngle = 360 - dAngle;


	RotateImg_ipp(m_gImModel->_MatchingImg.data, m_gImModel->_MatchingImg.cols, m_gImModel->_MatchingImg.rows, dAngle, &matchingArray, &InspImgwidth, &InspImgheight);
	RotateImg_ipp(m_gImModel->_mergedModel.data, m_gImModel->_mergedModel.cols, m_gImModel->_mergedModel.rows, dAngle, &mergeArray, &InspImgwidth, &InspImgheight);

	cv::Mat _MatchingImg(InspImgheight, InspImgwidth, CV_8UC1, matchingArray), _mergedModel(InspImgheight, InspImgwidth, CV_8UC1, mergeArray);
	cv::Mat score_M, result, tmp;
	cv::Point	ptMin, ptMax;
	double		Min(0.0), Max(0.0);
	bool bDisplayLine = m_bDispalyMatchPos;
	std::vector<cv::Point> vMatchArea;

	if (_MatchingImg.cols > src.cols || _MatchingImg.rows > src.rows)
	{
		return e_NG;
	}
	int nDilateSize = 3;
	bool bUseLine = m_bUseFilterUnit;
	if (bUseLine)
	{
		nDilateSize = m_nFilterUnitSize;
	}

	if (LineImg.empty() == FALSE)
	{
		int PntIdx = (int)(nDilateSize / 2 + 0.3);

		cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(nDilateSize, nDilateSize), cv::Point(PntIdx, PntIdx));

		if (LineImg.rows == src.rows && LineImg.cols == src.cols)
		{
			cv::dilate(LineImg, LineImg, kernel2);
			src = src - LineImg;

			LineImg.release();
		}
	}
	else
		return e_NG;

	int shiftW = g_pInspMng->FP_CenterPtr.x - (src.cols / 2);
	int shiftH = g_pInspMng->FP_CenterPtr.y - (src.rows / 2);


	cv::Mat CorrectionMat(_mergedModel.rows, _mergedModel.cols, CV_8UC1, cv::Scalar(0));

	int stX = (src.cols / 2) - (_mergedModel.cols / 2);
	int stY = (src.rows / 2) - (_mergedModel.rows / 2);

	cv::Mat _tmpMask(src.rows, src.cols, CV_8UC1, cv::Scalar(0));

	cv::Mat ModelDst(src.rows, src.cols, CV_8UC1, cv::Scalar(0));
	cv::Mat RoiMat;

	RoiMat = _tmpMask(cv::Rect(stX, stY, _mergedModel.cols, _mergedModel.rows));
	_mergedModel.copyTo(RoiMat);

	ForeignModelChange(_tmpMask, ModelDst);

	cv::Mat ClipMask(src.rows, src.cols, CV_8UC1, cv::Scalar(0));

	ModelDst.copyTo(ClipMask);

	result = src;


	if (SetAndFlag && erodeSize > 0)
	{
		int sizemin = src.cols < src.rows ? src.cols : src.rows;
		int dilation_size = sizemin / 500 > 5 ? sizemin / 100 : 5;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
			cv::Point(dilation_size, dilation_size));
		int PntIdx = (int)(erodeSize / 2 + 0.3);
		cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(erodeSize, erodeSize), cv::Point(PntIdx, PntIdx));
		cv::erode(ClipMask, ClipMask, kernel2);
	}

#if _DEBUG
	cv::imwrite("D:\\testimage\\Pim\\searchImage_src.bmp", src);
	cv::imwrite("D:\\testimage\\Pim\\searchImage_mergedModel.bmp",/*_mergedModel*/_tmpMask);
	cv::imwrite("D:\\testimage\\Pim\\searchImage_template.bmp", result);
#endif

	//line ±×¸®±â
	for (int i = 0; i < ClipMask.rows; i++)
	{
		for (int j = 0; j < ClipMask.cols; j++)
		{
			if (ClipMask.data[i * ClipMask.cols + j] > 200)
				vMatchArea.emplace_back(cv::Point(j, i));
		}

	}

	if (vMatchArea.size() > 0)
	{
		cv::RotatedRect rt = cv::minAreaRect(vMatchArea);

		cv::Point2f pts[4];
		rt.points(pts);

		for (int i = 0; i < 4; i++)
		{
			if (pts[i].x >= 0 && pts[i].x < ClipMask.cols)
			{
				pFPMathcPos[i].x = pts[i].x;
			}
			else
			{
				if (pts[i].x < 0)
					pFPMathcPos[i].x = 0.0;
				if (pts[i].x > ClipMask.cols)
					pFPMathcPos[i].x = ClipMask.cols - 1;
			}

			if (pts[i].y >= 0 && pts[i].y < ClipMask.rows)
			{
				pFPMathcPos[i].y = pts[i].y;
			}
			else
			{
				if (pts[i].y < 0)
					pFPMathcPos[i].y = 0.0;
				if (pts[i].y > ClipMask.rows)
					pFPMathcPos[i].y = ClipMask.rows - 1;;
			}
		}
		/*cv::Mat DrawingImg(ClipMask.rows, ClipMask.cols, CV_8UC1, cv::Scalar(0));

		for (int i = 0; i < 4; i++)
		{
		cv::line(DrawingImg, pts[i%4], pts[(i+1)%4], 255);
		}*/
	}

	if (_tmpMask.rows != result.rows || _tmpMask.cols != result.cols)
	{
		return e_NG;
	}


	tmp = result & ClipMask;


	src.setTo(0);
	tmp.copyTo(result);

#if _DEBUG
	cv::imwrite("D:\\testimage\\Pim\\searchImage_2tmp.bmp", tmp);
	cv::imwrite("D:\\testimage\\Pim\\searchImage_3src.bmp", result);
#endif

	if (matchingArray != NULL)
		//delete [] matchingArray;
		g_pMManager->pem_delete(matchingArray, true);
	if (mergeArray != NULL)
		//delete [] mergeArray;
		g_pMManager->pem_delete(mergeArray, true);

	return re;
}
void CProcPil_ForeignPattern::ForeignModelChange(cv::Mat src, cv::Mat& dst)
{
	std::vector<cv::Point> pt;

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			cv::Point tmpPt;
			if (src.data[i * src.cols + j] > 200)
			{
				tmpPt.x = j;
				tmpPt.y = i;
				pt.push_back(tmpPt);
			}
		}
	}

	cv::RotatedRect rt = cv::minAreaRect(pt);

	POINTF centerPtr;
	centerPtr.x = rt.center.x;
	centerPtr.y = rt.center.y;
	int shiftX = pLinCenter.x - centerPtr.x;
	int shiftY = pLinCenter.y - centerPtr.y;

	if (centerPtr.x > pLinCenter.x)
		shiftX = -shiftX;
	if (centerPtr.y > pLinCenter.y)
		shiftY = -shiftY;

	if (shiftX != 0 || shiftY != 0)
	{
		for (int i = 0; i < src.rows; i++)
		{
			for (int j = 0; j < src.cols; j++)
			{
				int dX = j + shiftX;
				int dY = i + shiftY;
				if (dY >= 0 && dY < src.rows - 1 && dX >= 0 && dX < src.cols - 1 && src.data[i * src.cols + j] > 0)
				{
					dst.data[dY * src.cols + dX] = 255;
				}
			}
		}
	}
	else
	{
		dst = src.clone();
	}

}