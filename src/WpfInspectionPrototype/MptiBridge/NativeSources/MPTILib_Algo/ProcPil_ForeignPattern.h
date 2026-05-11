#pragma once
//MIL 

#include "ProcMil.h"
#include "ImageModels.h"
#include "MModelManager.h"
#include "ModelFile_Foreign.h"


class CProcPil_ForeignPattern : public CProcMil
{
public:
	CProcPil_ForeignPattern(void);
	virtual ~CProcPil_ForeignPattern(void);

	int ModelList_Load(CString strPath);
	int ModelList_Load(fileAlgoPath* strPath, int cnt);
	int ModelList_Check(CString strPath);
	int ModelList_Clear(int nLane);

	int ModelLoad(CString strPath);
	int ModelFileLoad(CString strPath);

	void ClearModel();
	int teachingModel(cv::Mat src,cv::Mat maskImg,CString sPath,bool bIsMin,double dAngle, int nMargin);
	int SearchForeign(cv::Mat src,bool flag,double dAngle);
	int resultImage(cv::Mat src,CString sPath,bool flag,double dAngle);
	int ModelImage(CString sPath,double dAngle);
	int SearchForeignByMask(cv::Mat src, bool flag, double dAngle, POINTF* pFPMathcPos);
	void ForeignModelChange(cv::Mat src, cv::Mat& dst);


	int NewModel(cv::Mat src,CString sPath,double dAngle, int nMargin);
	std::shared_ptr<ModelFile_Foreign> m_gModelFile;

	std::shared_ptr<ImgModels>  m_gImModel;
	std::shared_ptr<CMModelManager<ModelFile_Foreign>> m_ModelMng;
	bool SetAndFlag;
	int erodeSize;
	cv::Mat LineImg;
	cv::Point pLinCenter;
	bool m_bUseFilterUnit;
	int m_nFilterUnitSize;
	bool m_bDispalyMatchPos;
};

