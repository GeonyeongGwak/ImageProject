#pragma once

#include "pinsp.h"

//#include "PInsp_Polar.h"
#include "Proc3D.h"
#include "ProcMil_Mount.h"
#include "ProcMil_Pattern.h"

#define BODYBLOB_DEFAULT 10
enum mount_stepID
{
	eMStepID_BLOB = 0,
	eMStepID_PATTERN,
	eMStepID_SIZE,
	eMStepID_POLARITY,
	eMStepID_ECCENTRICITY,
	eMStepID_ROTATE,
	eMStepID_GRADIENT,
	eMStepID_COUNTS
};
enum m_eBodyBlobOPT
{
	m_eBodyBlobOPT_TipAngle = 0x02,
	m_eBodyBlobOPT_BoundaryAngle = 0x04,
	m_eBodyBlobOPT_TIP_END = 0x08,
	m_eBodyBlobOPT_INSP_AREA = 0x10,
	m_eBodyBlobOPT_UNIT_FIND = 0x20,
	m_eBodyBlobOPT_HGT_DIFF_MASK = 0x40,
	m_eBodyBlobOPT_TipShift = 0x80,
};
class CPInsp_Mount : public CPInsp
{
public:
	CPInsp_Mount(void);
	virtual ~CPInsp_Mount(void);
	double m_dSearchPer_Default[BODYBLOB_DEFAULT];
	double m_dBodyBlobSearchPer_BodyTip;
	int m_nSearchPer_DefaultCnt;
	int m_nAlgoPoint;
	int m_nBodyBlobOPT;

private:
	double m_resolX;
	double m_resolY;
	int m_fovWidth;
	int m_fovLength;
		
private:
	//////////////////////
	//		Class		//
	//////////////////////
	
	//3D inspection Class
	CProc3D m_proc3d;

	CProcMil_Mount* m_procMil;
	CProcMil_Pattern* m_procPatMil;

private:
	//////////////////////////////////////////
	//	inspection image data & zmap data   //
	//////////////////////////////////////////

	//inspection image for mount
	void* m_fovImage_insp;
	void* m_fovImage_teach;


	//zmap data
	ZmapData m_inspZmapData;
	ZmapData m_teachZmapData;

private:
	/////////////////////////////
	//	inspection parameter   //
	/////////////////////////////

	//auto inspection parameter for mount
	InspParamMount* m_inspParam_Mount;


	//inspection coordinate for mount
	Coordinate m_inspCoordinate;


private:
	////////////////////////
	//	teach parameter   //
	////////////////////////

	//teach parameter for mount
	TeachParamMount* m_teachParam_Mount;
	
	
	//teach coordinate for mount
	Coordinate m_teachCoordinate;

private:
	//////////////////////////
	//	inspection result   //
	//////////////////////////
	RstInspMount m_rstInspMount;

	//blob result (mount)
	MountBlobRst m_inspRst;

	BodyOffset* m_bodyOffset;

	//debug
	int m_fovIndex;

	bool m_bUseImagePilLib;

public:
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	int CloseDevice();

	int SetInspParam(void* itemParam, void* targetImg, ZmapData zmap, Coordinate cdn, int fovNum = -1);
	int SetTeachParam(void* itemParam, void* targetImg, int teachType, ZmapData zmap, Coordinate cdn);


	int InspProc_Mount();
	int ProcAuto();
	int ProcStep(int stepID);

	int InspBodyBlob( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, InspRoiImgBuf &sInspImageData, BOOL bTeach, 
		RstAlgoBodyBlob * pRstAlgo, CRect &rcBlobBody, POINTF ptWndCenter, TotalInspExceptArea stTieArea, UCHAR *pUcImgBlob=NULL);

	int ClearModelList(int nLane);
	int LoadModelList(CString sPath);
	int CheckModelList(CString path);
	void INSP_SHIFT_AREA(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, RstAlgoBodyBlob * pRstAlgo, int nW, int nH, double dMinArea, int nCntBlob, UCHAR* ucArrBlobDst, float fSearchPer = 0.0f);
	int BODY_TIP_DIV(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, UCHAR* pucBlob, int nW, int nH, CRect rcROI, float fSearchPer_BodyTip, bool bUnit);
	int BODY_TIP_CHK(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, cv::Mat imgBlob, int nCntBlob, double *dArrArea, double *dArrCX, double *dArrCY, CRect *rcArrBlob, double * dRstX, double * dRstY);
	int BODY_TIP_RMV(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, cv::Mat imgBlob, int nW, int nH, int nCntBlob, CRect *rcArrBlob, int nMinBlob, float fSearchPer_BodyTip);
	void BODY_TIP_RMV_DIV(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, cv::Mat imgBlob, int nW, int nH, CRect rcROI, float fSearchPer_BodyTip);
	void BODY_RMV_DIV(AlgoBodyBlob *pAlgo, double dAngle, bool bTeach, UCHAR* ucArrBlobDst, int nW, int nH, float fSearchPer);
#pragma region _MOUNT FUNCTION_

private:
	void DeleteDoubleVal(double** pValue, int nCnt);
	void InitDoubleVal(double** pValue, int nCnt);
	BOOL IsUseInsp(AlgoBodyBlob *pAlgoBodyBlob);

public:
	int IsMountOK();
	//int IsMountOK2();
	int ComponentSize();
	int MountPatternSearch();
	int Eccentricity();
	int Gradient();
	int Rotation();
	int Mount_Teach(UCHAR* retTrueImg, int width, int height);
	//int Mount_Teach2(UCHAR* retTrueImg, int width, int height);
	int Mount_GetBodyRect(RECT* retRect);
	
	void GetInspRst(RstInspMount* ret);

	int GetBodyOffset(BodyOffset* rst);
	
private:
	void WriteTeachBlobResult(float hAvr, double cogX, double cogY, double width, double height, CRect rect, double rotAng, double graAng_v, double graAng_h);
	void WriteInspBlobResult(float hAvr, double cogX, double cogY, double width, double height, CRect rect, double rotAng, double graAng_v, double graAng_h);
	void WriteTeachBlobResult(CMilBlobResult* rst, float hAvr);
	void WriteInspBlobResult(CMilBlobResult* rst, float hAvr);	
	void CalcBlobSize(CMilBlobResult* blob, double* retWidth, double* retLength);

	void InitBlobResultStruct(MountBlobRst* rst);
	void InitMntResultStruct(RstInspMount* rst);

	void InitOffsetBuf();
	void DeleteOffsetBuf();

	double GetRotateAngle(int wndW, int wndH, double wndAngle, int* retW = NULL, int* retH = NULL);
	CRect RotateRect(CRect srcRect, double angle);

	int CheckRect(CRect rt, CPoint* nodalPt, int ptCount, int* retIndex);
	void RemakeNodalPt(CPoint* srcNodalPt, CRect srcRect, int ptCount, int errorCount, int errorIndex, CPoint* dstNodalPt);

	BOOL SearchBodyTip(AlgoBodyBlob *pAlgo, UCHAR * srcImg2d, float * srcImg3d, UCHAR * blobImg, int sizeX, int sizeY, double dAngle, CRect * rcBody, double * dArea, CPoint* dstNodalPt, float fSearchPer_BodyTip, int nInspBodyTip, cv::Mat imgBodyTipRst, CRect * rcTipBody);
	int OnlyBodyTip(AlgoBodyBlob *pAlgo, UCHAR * srcImg2d, float * srcImg3d, UCHAR * rstImg, int sizeX, int sizeY, double dAngle, POINT ptTeachCenter, float fSearchPer_BodyTip, bool bTeach, CRect * rcBody, double * dArea, TotalInspExceptArea stTieArea, double * dRstAngle, double * dRstX, double * dRstY, RstAlgoBodyBlob * pRstAlgo, cv::Mat imgBodyTipRst);
#pragma endregion _MOUNT FUNCTION_



#pragma region pattern 

	int MountPatternTeach(double cogX, double cogY);

#pragma endregion pattern

};

