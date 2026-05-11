#pragma once
#include "pinsp.h"
#include "Proc3D.h"
#include "ProcMil_LeadSolder.h"


enum leadsolder_stepID
{
	eLSStepID_CALC_RECT = 0,	
	eLSStepID_SOLDER,
	eLSStepID_LEAD,	
	eLSStepID_MAX_COUNT

};



class CPInsp_LeadSolder : public CPInsp
{
public:
	CPInsp_LeadSolder(void);
	virtual ~CPInsp_LeadSolder(void);

private:
	//3D inspection Class
	CProc3D m_proc3d;

	CProcMil_LeadSolder* m_pProcMilLeadSolder;

private:
	//shk 20140425 
	void* m_fovTargetImg;

	void* m_fovImageTop_insp;
	void* m_fovImageBottom_insp;
	void* m_fovImageMid_insp; //shk 2014/01/16

	void* m_fovImageTop_teach;
	void* m_fovImageBottom_teach;
	void* m_fovImageMid_teach;	//shk 2014/01/16

	void* m_fovImageTop_R;		// LMJ 2014/01/03	: CIE검사 (소납)
	void* m_fovImageTop_G;
	void* m_fovImageTop_B;
	
	//BYTE* m_lutData_color;

	//zmap data
	ZmapData m_inspZmapData;
	ZmapData m_teachZmapData;

	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;


private:
	/////////////////////////////
	//	inspection parameter   //
	/////////////////////////////

	//auto inspection parameter for lead & solder
	InspParamLeadSolder* m_inspParam;
	//inspection coordinate for lead & solder
	Coordinate m_inspCoordinate;


private:
	////////////////////////
	//	teach parameter   //
	////////////////////////

	//teach parameter for lead
	TeachParamLeadSolder* m_teachParam;
	//teach coordinate for lead
	Coordinate m_teachCoordinate;

private:
	//////////////////////////
	//	inspection result   //
	//////////////////////////
	RstInspLeadSolder m_inspResult;


public:
	//solder
	int* m_pFiletLength;
	float* m_pSolderHeight;
	double* m_solderVolume;
	int* m_solderVolRst;
	int* m_solderColorRst;
	double* m_solderColorRate;
	float* m_solderRatio;
	//shk 20140318
	float* m_graysolderRatio;
	int* m_pRstSolderStartPos;  //2014/02/14  솔더 시작점 저장(단순 디스플레이 용)

	//lead
	float* m_pLeadHeight;
	BOOL* m_pLeadLiftRst;
	BOOL* m_pLeadBridgeRst;
	int* m_pLeadPitch;

	LeadInfo m_leadInfo;

private:
	
	CRect* m_pLeadRect;
	CRect* m_pGapRect;
	CRect* m_pSolderRect;
	
	float* m_pZmapRoiData;
	UCHAR* m_pRoiImg_Top;
	UCHAR* m_pRoiImg_Bottom;
	UCHAR* m_pRoiImg_Mid; //shk 2014/01/16
	UCHAR* m_pImage_HV;

	UCHAR* m_pRoiImg_Tar; // shk 20140425

	int m_solderCount;

	float m_leadAvrH;

	int m_extractW;
	int m_extractL;

	float* m_pZmapBoundaryData;
	int m_bdryCx;
	int m_bdryCy;
	int m_bdryW;
	int m_bdryL;
	BOOL m_isAnyAngle;

private:
	LeadNgRectInfo* m_pNgLeadInfo;
	double m_wndAngle; 
	CRect* m_ngLeadRect;
	CRect* m_ngGapRect;
	CRect* m_ngSolderRect;

	CAtlArray<CRect> rcArrSearchTab;
public:
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	int CloseDevice();

	// LMJ 2014/01/03
//	int SetInspParam(void* itemParam, void* targetImg_Top, void* targetImg_Bottom, ZmapData zmap, Coordinate cdn);
//	int SetTeachParam(void* itemParam, void* targetImg_Top, void* targetImg_Bottom, int teachType, ZmapData zmap, Coordinate cdn);
	//int SetInspParam(void* itemParam, InspImgBuf img, ZmapData zmap, Coordinate cdn);
	//shk 20140425
	int SetInspParam(void* itemParam, void* targetImg, InspImgBuf img, ZmapData zmap, Coordinate cdn);
	//int SetTeachParam(void* itemParam, InspImgBuf img, int teachType, ZmapData zmap, Coordinate cdn);
	//shk 20140425
	int SetTeachParam(void* itemParam, void* targetImg,InspImgBuf img, int teachType, ZmapData zmap, Coordinate cdn);

	int InspProc_LeadSolder();
	int ProcAuto();
	int ProcStep(int stepID);

	bool LeadSolderTeach(const AlgoLeadSearch *pInspAlgoSearch, WndAlgoImg &sWndAlgoImg, UCHAR* pUcArrRetResultImg, LeadInfo* ptrResultLeadInfo, int &nLeadCnt, CRect* pArrLeadRect, BOOL bTabSearch, double dLeadStartPos);
	int CalcLeadnSolderArea();
	int InspSolder();
	int InspLead();

	int GetLeadPosition();


	double GetRotateAngle(int leadPos, int width, int height, double wndAngle, int* retWidth, int* retHeight);


	void GetInspRst(RstInspLeadSolder* dst);

	void DeleteAllData();
private:
	//common	
	int DecisionSolderInsp();
	void InitResultStruct(RstInspLeadSolder* rst);
	void DeleteBuffer();

	
	//solder
	int InitSolderRect(int cnt);
	void DeleteSolderRect();

	void InitSolderResultVal(int solderCnt);
	void DeleteSolderResultVal();

	void InitSolderVolumeVal(int solderCnt);
	void DeleteSolderVolumeVal();

	void InitSolderColorRateVal(int solderCnt);
	void DeleteSolderColorRateVal();

	void InitSolderHeightVal(int solderCnt);
	void DeleteSolderHeightVal();

	void InitFiletLengthVal(int solderCnt);
	void DeleteFiletLengthVal();

	void InitSolderColorResultVal(int solderCnt);
	void DeleteSolderColorResultVal();

	void InitSolderRatioVal(int solderCnt);
	void DeleteSolderRatioVal();

	void InitSolderStartRstVal(int solderCnt);
	void DeleteSolderStartRstVal();
	
	void InitgraySolderRatioVal(int solderCnt);
	void DeletegraySolderRatioVal();

	void ClacSolderAvrSize(CRect* solderRect, int solderCnt, int* retW, int* retH);
	//float GetChipBodyH(float* zmapData,CRect* pSolRect,int orgSizeX, int orgSizeY,int sizeX,int sizeY, double angle);
	float GetChipBodyH(float* zmapData, CRect pSolRect, int orgSizeX, int orgSizeY, int sizeX, int sizeY, double angle, int startPt);

	void GetSolderH(float* zmapData, CRect* pSolRect, int leadCnt, int orgSizeX, int orgSizeY, int sizeX, int sizeY, BOOL isChipSolder, float* retSolderHeight);
	void CalcDeltaH(float* solderH, float* leadH, int leadCnt, float* retDeltaH, double* retRatio);
	void GetDeltaH(float* zmapData, float* leadH, CRect* pSolRect, int leadCnt, int orgSizeX, int orgSizeY, int sizeX, int sizeY, BOOL isChipSolder, float* retSolderH, double* retRatio);
	//void GetDeltaH(float* src,CRect* pSolRect,int leadCnt,int orgSizeX, int orgSizeY,int sizeX,int sizeY,float* retSolderHeight,float* retDeltaH,float* retRatio);

	//lead
	int InitLeadRect(int cnt);
	void DeleteLeadRect();

	void InitLeadHeightVal(int LeadCnt);
	void DeleteLeadHeightVal();

	void InitLeadLiftVal(int LeadCnt);
	void DeleteLeadLiftVal();

	void InitLeadBridgeVal(int gapCnt);
	void DeleteLeadBridgeVal();

	void InitLeadPitchVal(int gapCnt);
	void DeleteLeadPitchVal();

	LeadInfo LeadInforCalc(CRect* leadRect, int leadCnts, CRect* gapRect, int gapCnts, int leadPos);

//	void InitMakeLUT_Color();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	//int CalcLeadSolderRect(UCHAR* topImg, UCHAR* hvImage,float* ZmapRoiData, int roiSizeX, int roiSizeY, int startPos, BOOL isContainer, BOOL isInspLeadLift, float leadHeight, double stdLeadWidth, int stdLeadCount, BOOL use3D);
	int CalcLeadSolderRect(UCHAR* topImg, UCHAR* hvImage,float* ZmapRoiData, int roiSizeX, int roiSizeY, int startPos, int endPos, BOOL isContainer, BOOL isInspLeadLift, float leadHeight, double stdLeadWidth, int stdLeadCount, BOOL use3D, BOOL use2D);
	int CalcLeadSolderRect(UCHAR* topImg, UCHAR* hvImage,float* ZmapRoiData, int roiSizeX, int roiSizeY, int startPos, BOOL isContainer, BOOL isInspLeadLift, float leadHeight, double stdLeadWidth, int stdLeadCount, BOOL use3D, BOOL use2D, int nThresholdValue, CAtlArray<CRect> *pRcArrSearchTab);
	double Insp_CalcSolderArea(UCHAR* src,int sizeX,int sizeY,CRect rt,int isChipSolder, int threshold);
	int RemakeLeadRect(int leadCount, int* solderStartPoint);
	int RemakeSolderRect(int solderCount, int* solderStartPoint);

	LeadInfo GetLeadInfo();

	BOOL IsLeadNg();
	BOOL GetLeadRectInfo(LeadNgRectInfo** leadInfo);
	CRect RotateLeadRect(CRect srcLeadRect, int cx, int cy, int roiSizeX, int roiSizeY, double angle, BOOL isWnd = FALSE);
	void InitNgInfoBuff(int leadCount);
	void DeleteNgInfoBuff();


	void GetEachLeadCoordi(double* retLeftPos, double* retRightPos);

	//////////////////////////////////////////////////////////////////////////
	//solder color
	int AnalysisColorCIE(int roiSizeX, int roiSizeY, float fitR, float fitG, float fitB, Coordinate coordi, CRect* solderRect, int solderCnt, int isChipsolder, double* retRate, float * zmap, float limitHeight);		// LMJ 2014/01/03
	void MakeProcImg(void* src1, void* src2, void* src3, UCHAR* dst1, UCHAR* dst2, UCHAR* dst3, Coordinate coordi);
	void GetBinaryImg(UCHAR* ptrucImg2D, int nSizeX, int nSizeY, UCHAR* ptrucDst, int nThreshold);
	bool GetLeadRect( InspAlgo &sInspAlgo, CRect* pucrcLeadRect, CRect* pucrcGapRect );
};

