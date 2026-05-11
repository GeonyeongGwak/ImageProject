#pragma once
#include "procmil.h"

#include "MModelManager.h"
#include <memory>

enum milprocpattern_result
{
	eMPAT_SUCCESS = 0,
	eMPAT_FAIL,
	eMPAT_AllocedInternally
};

class CProcMil_Pattern : public CProcMil
{
public:
	CProcMil_Pattern(void);
	virtual ~CProcMil_Pattern(void);

private:
	Im::PIL_ID m_milPatModel_Temp;
	Im::PIL_ID m_milPatModel;
	Im::PIL_ID m_milPatResult;

	Im::PIL_ID m_milPatModel_Rotate;

	std::shared_ptr<tagModelFile> m_ModelFile;
	std::shared_ptr<CMModelManager<tagModelFile>> m_ModelMng;
	std::vector< std::shared_ptr<tagModelFile> > m_SimilarModelFile;

	int _LightNumber;
	int _Use_algo_model;
	int _Use_model_nchannel;
	int _Use_model_nFilter;
//	int m_SimilarModelFileCnt;

public:
	virtual int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib=false);
	virtual int FreeMil();

public:
	virtual int WritePatternModel(CString strPath);
	virtual int WritePatternModel(void* milSrc, int cx, int cy, int width, int height, CString strPath);
	virtual int WritePatternModel(UCHAR* userSrc, int width, int height, double rotateAngle, CString strPath, CString strExt);
	virtual int SearchPattern(UCHAR* userSrc, int width, int height);
	virtual int SearchPattern_Draw(UCHAR* userSrc, int width, int height, UCHAR* userDst,int Channel);
	virtual int SearchPattern(void* milSrc, void* milSrc_color, int cx, int cy, int width, int height);
	virtual int AllocPatModel(void* milSrc, int cx, int cy, int width, int height);
	virtual int AllocPatModel(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH);
	virtual int GetPatResult(double* retScore, double* retAngle, double* retPosX, double* retPosY);
	virtual void GetPatSubScore(double* retScore, int Pitch);
	virtual int GetResultImg(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst);
	virtual void SetAngleMode(double posLimit, double negLimit, double accuracy, BOOL rotMode, bool bAngleModel = true);
	virtual void SetSearchStartAngle(double angle);

	virtual bool SetParam_DivArea(BOOL bUseCharDiv, int* DiviLnC, int* DiviLnR, double ** GapLnC, double ** GapLnR, double ** DiviScore, bool bDetailSearch, RECT modelRect);
	virtual void GetParam_DivArea(BOOL& bUseCharDiv, int & DiviLnC, int & DiviLnR, double * GapLnC, double * GapLnR, double * DiviScore, BOOL & bDetailSearch, RECT & modelRect);

//	virtual int ModelList_Load(CString strPath);
	virtual int ModelList_Load(fileAlgoPath* Path, int ptrFileAlgoPathCnt);
	virtual int ExtModelList_Load();
	virtual int ModelList_Check(CString strPath);
	virtual int ExtModelList_Check();
	virtual int ModelLoad(CString strPath);
	virtual int CodeSearchModelLoad(CString strPath);
	virtual int SimilarModelListLoad(CString Parcode, bool rear);
	virtual void SimilarCntReset();
	virtual int SimilarModelLoad(int index);
	virtual int ModelFileLoad(CString strPath);
	virtual int ModelList_Clear(int nLane);

	virtual void ModelPreproc(double dAngle,int ModelPreproc);

	virtual void PreprocModel(UCHAR* userSrc, int sizeX, int sizeY);
	virtual void PreprocModel(void* milSrc, BOOL rotMode = FALSE, int use_algo =1, int nLight_Number = 0);

	virtual int RotateImage(CString sPathImageOld, CString sPathImageNew, double dAngRotate);
	virtual int GetnChannel();
	virtual int GetModelAlgo();
	virtual void SetnChannel(int nChannel);
	virtual void GetnStep(int step);
	virtual void GetClipModelImg(cv::Mat ClipModelImg);

	virtual int GetMaskImage(CString sPath, UCHAR * Img, int Wid, int Len, int Pitch);
	virtual int SetMaskImage(UCHAR * Img, int Wid, int Len, int Pitch, double dAngle);

	virtual bool IsDivInspAvailable();

	virtual void setPilLightbt(int val)	{_LightNumber = val;}
	virtual int getPilLightbt() { return _LightNumber; }

	virtual void setPilMatchAlgo(int val) { _Use_algo_model = val; }
	virtual int getPilMatchAlgo()	{ return _Use_algo_model; }

	virtual void setModelFilterType(int val) { _Use_model_nFilter = val; }
	virtual int getModelFilterType()	{ return _Use_model_nFilter; }

	virtual void setPilModelnChannel(int val) { _Use_model_nchannel = val; }
	virtual int getPilModelnChannel()	{ return _Use_model_nchannel; }
	virtual void ImgProcess(void* milSrc, void* milSrc_color, int cx, int cy, int width, int height,std::shared_ptr<ImgProcessing> Proc_Img);
	virtual void ImgProcess(UCHAR* userSrc, int width, int height,std::shared_ptr<ImgProcessing> Proc_Img);
	virtual void SetFindSubPixel(BOOL rotMode,bool bFindModel);

	virtual int AllocModePatlexceptMask(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH);
	virtual SIZE getModelSz();
protected:
	virtual void DeleteModelBuf();
	virtual void ModelSave(CString strPath);
	virtual void ModelSave(Im::PIL_ID milModel, CString strPath);
	
	virtual void SetAccuracy(int speedFactor, int accuracy, BOOL rotMode);
};

