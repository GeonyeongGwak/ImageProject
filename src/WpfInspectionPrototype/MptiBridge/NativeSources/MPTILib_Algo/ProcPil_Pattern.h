#pragma once

#include "ProcMil_Pattern.h"
#include "GeoMatch.h"
#include "ModelManager_Geo.h"
#include "../PInspAlgo/PAlgo.h"

class CProcPil_Pattern : public CProcMil_Pattern
{
public:
	CProcPil_Pattern(void);
	virtual ~CProcPil_Pattern(void);

public:
	virtual int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
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
//	virtual int GetResultImg(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst);
	virtual void SetAngleMode(double posLimit, double negLimit, double accuracy, BOOL rotMode, bool bAngleModel = true);
	virtual void SetSearchStartAngle(double angle);
	virtual void SetFindSubPixel(BOOL rotMode,bool bFindModel);

	virtual bool SetParam_DivArea(BOOL bUseCharDiv, int* DiviLnC, int* DiviLnR, double ** GapLnC, double ** GapLnR, double ** DiviScore, bool bDetailSearch, RECT modelRect);
	virtual void GetParam_DivArea(BOOL &bUseCharDiv, int & DiviLnC, int & DiviLnR, double * GapLnC, double * GapLnR, double * DiviScore, BOOL & bDetailSearch, RECT & modelRect);

	virtual int ExtModelList_Load();

	//	virtual int ModelList_Load(CString strPath);
	virtual int ModelList_Load(fileAlgoPath* Path, int ptrFileAlgoPathCnt);
	virtual int ExtModelList_Check();

	virtual int ModelList_Check(fileAlgoPath* strPath, int cnt);
	virtual int ModelLoad(CString strPath);
	virtual int CodeSearchModelLoad(CString strPath);
	virtual int SimilarModelListLoad(CString Parcode, bool rear);
	virtual void SimilarCntReset();
	virtual int SimilarModelLoad(int index);
	virtual int ModelFileLoad(CString strPath);
	virtual int ModelList_Clear(int nLane);

	virtual void ModelPreproc(double dAngle,int ModelPreproc);

	virtual void PreprocModel(UCHAR* userSrc, int sizeX, int sizeY);
	virtual void PreprocModel(void* milSrc, BOOL rotMode = FALSE, int use_algo=1, int nLight_Number = 0);

	virtual int RotateImage(CString sPathImageOld, CString sPathImageNew, double dAngRotate);

	virtual int GetMaskImage(CString sPath, UCHAR * Img, int Wid, int Len, int Pitch);
	virtual int SetMaskImage(UCHAR * Img, int Wid, int Len, int Pitch, double dAngle);
	virtual void SetnChannel(int nChannel);
	virtual int GetnChannel();
	virtual int GetModelAlgo();
	virtual void GetnStep(int step);
	virtual void GetClipModelImg(cv::Mat ClipModelImg);protected:
	virtual void DeleteModelBuf();
	virtual void ModelSave(CString strPath);
	void ModelSave(std::shared_ptr<GeoModel_Pattern> milModel, CString strPath);

	virtual void SetAccuracy(int speedFactor, int accuracy, BOOL rotMode);

	virtual bool IsDivInspAvailable();

	virtual void setPilLightbt(int val)	{_LightNumber = val;}
	virtual int getPilLightbt() { return _LightNumber; }

	virtual void setPilMatchAlgo(int val) { _Use_algo_model = val; }
	virtual int getPilMatchAlgo()	{ return _Use_algo_model; }
	
	virtual void setPilModelnChannel(int val) { _Use_model_nchannel = val; }
	virtual int getPilModelnChannel()	{ return _Use_model_nchannel; }

	virtual void setModelFilterType(int val) { _Use_model_nFilter = val; }
	virtual int getModelFilterType()	{ return _Use_model_nFilter; }

	virtual void ImgProcess(void* milSrc, void* milSrc_color, int cx, int cy, int width, int height,std::shared_ptr<ImgProcessing> Proc_Img);
	virtual void ImgProcess(UCHAR* userSrc, int width, int height,std::shared_ptr<ImgProcessing> Proc_Img);
	cv::Mat GetAngleImg(cv::Mat & src, float angle, bool bInterpolation);
	//void EdgeAlgoCheckback();

	virtual int AllocModePatlexceptMask(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH);
	virtual SIZE getModelSz();
private:
	std::shared_ptr<GeoModel_Pattern> m_milPatModel;
	std::shared_ptr<GeoModel_Pattern> m_milPatModel_Temp;
	std::shared_ptr<GeoModel_Pattern> m_milPatModel_Rotate;
	std::shared_ptr<GeoResult_Pat> m_milPatResult;

	std::shared_ptr<ModelFile_Pat> m_ModelFile;
	std::shared_ptr<CMModelManager<ModelFile_Pat>> m_ModelMng;
	std::vector< std::shared_ptr<ModelFile_Pat> > m_SimilarModelFile;
//	int m_SimilarModelFileCnt;

	std::shared_ptr<GeoMatchParam> m_SearchParam;
	
	int m_nChannel;
	int m_nStep;
	cv::Mat m_ClipModelImg;
	int _LightNumber;
	int _Use_algo_model;
	int _Use_model_nchannel;
	int _Use_model_nFilter;
	GeoMatch_Pattern gm;
	GeoMatch_Pattern gm_rotate;
	std::shared_ptr<ImgProcessing> ImgProc;
	std::shared_ptr<ImgProcessing> ImgProc_E1;
	std::shared_ptr<ImgProcessing> ImgProc_E2;
	std::shared_ptr<ImgProcessing> ImgProc_E3;
	std::shared_ptr<ImgProcessing> ImgProc_D1;
	std::shared_ptr<ImgProcessing> ImgProc_D2;
	std::shared_ptr<ImgProcessing> ImgProc_D3;
};

