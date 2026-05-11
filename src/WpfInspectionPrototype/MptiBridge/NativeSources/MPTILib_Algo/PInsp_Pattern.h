#pragma once
#include "pinsp.h"
#include "ProcMil_Pattern.h"
//#include "MPTI.h"

enum proc_pattern_statuse
{

	ePROC_PAT_NG= 0,
	ePROC_PAT_OK,
};


enum pattern_stepID
{
	ePStepID_PATTERN_SEARCH = 0,
	ePStepID_SIMIALR_PAT_SEARCH,
	ePStepID_COUNTS,
};

class CPInsp_Pattern :	public CPInsp
{
public:
	CPInsp_Pattern(void);
	virtual ~CPInsp_Pattern(void);

private:
	void* m_fovImage_insp;
	void* m_fovImage_insp_color;
	void* m_fovImage_teach;
	cv::Mat m_Mat_fovImage_teach;
	void* m_TeachingMask;

	bool m_bUseImagePilLib;

	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;

	int m_patWndCnts;	//counts of pattern window
	int m_patWndCnts_old;	//counts of pattern window(old)

	
	//AlgoPattern *m_pAlgoPattern; 
	InspParamPattern m_paramInspect;
	TeachParamPattern m_paramTeach;
	WndInfo			  m_InspPartInfo;
	

	BOOL m_bAllocFovImg;

	int m_nInspectionMode;
	fileAlgoPath* ptrFileAlgoPath;
	int ptrFileAlgoPathCnt;
	
	
private:
	/////////////////////////////
	//	inspection parameter   //
	/////////////////////////////
	//auto inspection parameter for pattern
	InspParamPattern* m_pInspParam; //원복필요
	//inspection coordinate for pattern
	Coordinate m_inspCoordinate;

private:
	////////////////////////
	//	teach parameter   //
	////////////////////////

	//teach parameter for pattern
	TeachParamPattern* m_pTeachParam;
	//teach coordinate for pattern
	Coordinate m_teachCoordinate;
	//teach InspPartInfo for pattern
	WndInfo* m_pInspPartInfo;
	//teaching light bt index for pattern 
	int _LightNumber;
	int _Use_algo_model;
	int _Use_model_nchannel;
	int _Use_model_nFilter;

private:
	//////////////////////////
	//	inspection result   //
	//////////////////////////
	RstInspPattern m_rstInspPattern;
	RstInspPattern m_rstInspSimPattern;
	double m_rstSimilarScoreDiv[CNT_PATTERN_SCORE];
	double m_rstSimilarScore;
public:
	CProcMil_Pattern* m_procMil;
	AlgoPattern *m_pAlgoPattern; // 원복필요
	//InspParamPattern* m_pInspParam; //원복필요
	std::shared_ptr<ImgProcessing> Img_Proc;
public:
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	int CloseDevice();

	CProcMil_Pattern* GetProcMil() const { return m_procMil; }
public:
	int SetInspParam(void* itemParam, void* targetImg, Coordinate cdn);
	int SetInspParam( const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, int nPartWidth, int nPartHeight, TotalInspExceptArea stTieArea);
	int SetTeachParam(void* itemParam, void* targetImg, int teachType, Coordinate cdn);
	int SetTeachParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, WndInfo &sInspPartInfo);
	
	int InspProc_Pattern(int nInspectionMode, int nOffX_pix, int nOffY_pix);
	int ProcAuto(int nOffX_pix, int nOffY_pix);
	int ProcStep(int stepIDint, int nOffX_pix, int nOffY_pix); // TRUE = teach test mode    ,    FALSE = inspection mode 
	double inspSimilarList(const InspAlgo &sInspAlgo,  SimilarPartInfo* listPartCode, int* listScore, int nlistCnt);
	void SetSimListParam(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sInspPartInfo);

	int SearchPattern(int nOffX_pix, int nOffY_pix);
	int SearchSimilarPattern();
	int Pattern_Teach(UCHAR* retImg, int sizeX, int sizeY, int use_algo, int nLight_Number, bool refPart);
	int DecisionPatternInsp(bool bDecision);
	int DecisionSimPatternInsp(bool bDecision);


	void GetInspRst(RstInspPattern* dst);
	void GetInspRst(RstAlgoPattern* pDstAlgoPattern);
	void InitPatternResultStruct(RstInspPattern* rst);
	int SavePatternModel(CString path, CString sExt);

	int ClearModelList(int nLane);
//	int LoadModelList(CString path);
	int CheckModelList(CString path);
	int LoadModelList(fileAlgoPath* Path);
	int ExtLoadModelList(CString Path);
	void PatternCnt(int cnt);

	void SetInspPartInfo(WndInfo &sInspPartInfo);

	BOOL IsAllocFovImg() const { return m_bAllocFovImg; }
	void SetAllocFovImg(BOOL bAllocFovImg) { m_bAllocFovImg = bAllocFovImg; }

	void GetPatternImage(WndAlgoImg &sWndAlgoImg, std::vector<UCHAR> & vMaskImg, int ModelIndex,const InspAlgo &sInspAlgo,int angle = 0);

	void GetMaskImage(CString sPath, UCHAR * ptr, int Wid, int Len, int Pitch);
	void SetMaskImage(UCHAR * ptr, int Wid, int Len, int Pitch, double dAngle);

	virtual void setInspLightbt(int val)	{_LightNumber = val;}
	virtual int getInspLightbt() { return _LightNumber; }

	virtual void setInspMatchAlgo(int val)	{_Use_algo_model = val;}
	virtual int getInspMatchAlgo() { return _Use_algo_model; }

	virtual void setInspModelnChannel(int val) { _Use_model_nchannel = val; }
	virtual int getInspModelnChannel()	{ return _Use_model_nchannel; }

	void setModelFilterType(int val)	{_Use_model_nFilter = val;}
	int getModelFilterType() { return _Use_model_nFilter; }

	void SetImgProccess(std::shared_ptr<ImgProcessing> Proc_Img) { Img_Proc = Proc_Img; }

private:
	double GetRotateAngle(int wndW, int wndH, double wndAngle, int* retW, int* retH);
};

