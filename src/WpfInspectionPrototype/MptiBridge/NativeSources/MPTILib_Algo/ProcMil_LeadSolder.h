#pragma once
#include "procmil.h"
#include <vector>
#include <algorithm>

using namespace std;

typedef vector<int> POSITION_1D;
typedef vector<int> RUN_WIDTH;

typedef struct tagLeadInfoTemp
{	
	long		countLead;
	long		maxLead;
	long		minLead;
	double		avgLead;

	long		countGap;
	long		maxGap;
	long		minGap;
	double		avgGap;

	double leadPitch;
}LeadInfoTemp;


enum milprocleadsolder_result
{
	eMLSD_SUCCESS = 0,
	eMLSD_FAIL
};

enum solder_volume_result
{
	eSolder_OK = 0,
	eSolder_Excessive,
	eSolder_Insufficient,
	eSolder_Void,
	eSolder_Cold

};

class CProcMil_LeadSolder :	public CProcMil
{
public:
	CProcMil_LeadSolder(void);
	virtual ~CProcMil_LeadSolder(void);


private:
// 	Im::PIL_ID m_milBlobFeature;
// 	Im::PIL_ID m_milBlobResult;

	POSITION_1D	m_vtrLandPos; //land left position
	POSITION_1D	m_vtrGapPos;  //gap left position
	POSITION_1D m_vtrLandCenterPos; 
	RUN_WIDTH	m_vtrLand;  //land width
	RUN_WIDTH	m_vtrGap;  //gap width

	long		m_countLead;
	long		m_maxLead;
	long		m_minLead;
	double		m_avgLead;
	long		m_countGap;
	long		m_maxGap;
	long		m_minGap;
	double		m_avgGap;
	double		m_avgPitch;


public:
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int FreeMil();

	//solder
public:
	int CalcBlobM(Im::PIL_ID milBinSrc, int minArea, int maxArea, BOOL eraseBorderBlob, BOOL fillHole, Im::PIL_ID milDst);
	void GetBlobResult(CMilBlobResult* retResult);
	
	void DataSorting(CMilBlobResult* src, CRect* dst, BOOL flag);
	int CalcSolderVolume(float* pZmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float* hLimit, double* retSolderVolume);

	int CalcBlobRect(float* zmapRoiData, UCHAR* userSrc_Top, UCHAR* userSrc_Bottom, int sizeX, int sizeY, CMilBlobResult** retRst);
	int GetSolderRect(float* zmapRoiData, UCHAR* userSrc_Top, UCHAR* userSrc_Bottom, int sizeX, int sizeY, int solderStartPos, CRect** retBlobRect, CRect** retSolderRect);
	int BlobProc(Im::PIL_ID milSrc, int sizeX, int sizeY,  Im::PIL_ID milDst);
	int GetSolderRect_Chip(float* zmapRoiData, UCHAR* userSrc, int sizeX, int sizeY, CRect* retRect);
	int BlobProc_Chip(Im::PIL_ID milSrc, int sizeX, int sizeY, Im::PIL_ID milDst);

	int SelectBlob_MaxArea(int blobNum, Im::PIL_ID milDst);


	//lead
public:
	void SaveLeadProcImg(UCHAR* userSrc, int sizeX, int sizeY, CRect* leadRect, CRect* gapRect, CRect* solderRect, int leadCnts, UCHAR* retColorImg);
	BOOL InspGapArea(UCHAR* userSrc, float* zmapRoiData, int sizeX, int sizeY, CRect* gapRect, int avrGapW, int cnts, double angle, BOOL bridgeMode, float stdHeight, BOOL* retResult);  //bridgeMode  1: 2d Only / 0: 2d&3d
	BOOL InspGapArea2(UCHAR* userSrcMid, UCHAR* userSrcBot, float* zmapRoiData, int sizeX, int sizeY, CRect* gapRect, int avrGapW, int cnts, double angle, BOOL bridgeMode, float stdHeight, BOOL* retResult);
	int CalcBlob(UCHAR* userBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob, BOOL fillHole, UCHAR* userDst);
	int CalcBlob(Im::PIL_ID milBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob, BOOL fillHole, UCHAR* userDst);
	void GetBlobResultRect(CRect* retRect, double* area);


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	void GetImage_HV(UCHAR* topImg, UCHAR* bottomImg, int sizeX, int sizeY, UCHAR* dst);
	void GetImage_HV(UCHAR* topImg, UCHAR* midImg,UCHAR* bottomImg, int sizeX, int sizeY, UCHAR* dst, int threshold); //shk 2014/01/16

	LeadInfoTemp CalcLeadRect(UCHAR* topImg, UCHAR* hvImg, float* ZmapRoiData, int sizeX, int sizeY, CRect** retRect, int** retLeadPitch, BOOL isContainer, BOOL isInspLeadLift, int leadStartPos, float leadHeight, double leadWidth, BOOL use3D); //shk 2014-01-03;
	LeadInfoTemp CalcLeadRect2(UCHAR* topImg, UCHAR* bottomImg, UCHAR* hvImg, float* ZmapRoiData, int sizeX, int sizeY, CRect** retRect, int** retLeadPitch, BOOL isContainer, BOOL isInspLeadLift, int leadStartPos, float leadHeight, double stdLeadWidth, int stdLeadCount, BOOL use3D);
	LeadInfoTemp CalcLeadRect3(UCHAR* topImg, UCHAR* hvImg, float* ZmapRoiData, int sizeX, int sizeY, CRect** retRect, int** retLeadPitch, BOOL isContainer, BOOL isInspLeadLift, int leadStartPos, float leadHeight, double stdLeadWidth, int stdLeadCount, BOOL use3D, BOOL use2D);
	LeadInfoTemp CalcLeadRectTab_org(UCHAR* hvImage, int roiSizeX, int roiSizeY, int nThresholdValue, int startPos, CAtlArray<CRect> *pRcArrSearchTab);
	LeadInfoTemp CalcLeadRectTab(UCHAR* hvImage, int roiSizeX, int roiSizeY, int nThresholdValue, int startPos, CAtlArray<CRect> *pRcArrSearchTab);
	LeadInfoTemp CalcLeadRect4(UCHAR* ptruc2DImage, float* ptrf3DImage, int nImgW, int nImgH, BOOL bUse2D, BOOL bUse3D, int nLeadTipPos, CRect** retRect, int** retLeadPitch);
	void CalcRunLength(const long nCount, const long *line, const long min_land_size, BOOL widthFlag);
	void CalcRunLength2(const long nCount, const long *line, const long min_land_size, BOOL widthFlag, int stdLeadCount, double stdLeadWidth, int** retPitch);
	double CalcSolderArea(UCHAR* userSub, int sizeX, int sizeY, int threshold);


	//int InspSolderArea(float* zmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int stPoint, int endPoint, int isChipsolder, int* retSolderRst, double* retSolderRatio, double stdRatio = 0);
	int InspSolderArea(float* zmapData, UCHAR* img, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int stPoint, int endPoint, int isChipsolder, int* retFiletL, int* retFiletH, double* retSolderRatio, BOOL* retOverEndline);
	// LMJ 2014/01/20	: solderVolum 인자 추가
	//int InspSolderArea2(float* zmapData, UCHAR* img, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int* retFiletL, int* retFiletStart);
	int InspSolderArea2(float* zmapData, UCHAR* img, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int* retFiletL, int* retFiletStart, double* solderVolum);
	int InspSolderArea3(float* zmapData, UCHAR* img, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int solderInspLength, int* retFiletStart, double* retSolderVolum, float* retSolderRatio);
	int InspSolderArea4(float* zmapData, UCHAR* imgT, UCHAR* imgM, UCHAR* imgB, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int stdSolderArea, int isChipsolder, int solderInspLength, int solderWidthMargin, bool mode, int* retFiletStart, int* retSolderArea, int* retBlobCount);
	int InspSolderArea_S1(float* zmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio);
	int InspSolderArea_S2(float* zmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio);

	void DataSave(int* data, int dataSize, CString path);
	void DataSave(float* data, int dataSize, CString path);

	void TestImageSave(void* milSrc, int orgSIzeX, int orgSizeY, int cx, int cy, int w, int h, CString path);
	void TestImageSave(UCHAR* userSrc, int orgSIzeX, int orgSizeY, int cx, int cy, int w, int h, CString path);


	void MaskTrueImg(UCHAR * src, UCHAR * mask, UCHAR * dst, int sizeX, int sizeY);


	int SearchStartPos(UCHAR* srcImg, int sizeX, int sizeY);
	//shk 20140317
	int InspSolderArea_S3(float* zmapData, UCHAR* imgT,int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio,float* retGrayfRatio);
	//shk 20140424
	int InspSolderArea_S4(float* zmapData, UCHAR* imgT,int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int isUse2D, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio,float* retGrayfRatio);
	//shk 20140430
	int InspSolderArea_S5(float* zmapData, UCHAR* imgT,int roiSizeX, int roiSizeY, int solderStartPos, CRect* solderRect, int solderCnt, float avrLeadH, int isChipsolder, int isUse2D, int solderInspLength, int* retLeadEd, int* retSolderSt, float* retfiletRatio,float* retGrayfRatio);
};

