#pragma once
#include "PInsp_Mount.h"
#include "PInsp_Color.h"
#include "PInsp_OCR.h"
// #include "PInsp_Solder.h"
// #include "PInsp_Lead.h"
#include "PInsp_Pattern.h"
#include "PInsp_LeadSolder.h"
#include "PInsp_BGA.h"
#include "PInsp_Algo.h"
#include "PInsp_Grid.h"		// YJS 2016/11/04
#include "PInsp_OCV.h"
#include "PInsp_ForeignPattern.h"
#include "PInsp_Tab.h"
#include "PInsp_WireBonding.h" //YBJI 2019/10/01

#define sEnumString(a) #a 

class CManager
{
public:
	CManager(void);
	virtual ~CManager(void);

protected:
	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;

	CString m_className;

	int* m_inspItemCnts;//[eINSP_WND_CNT];
	int** m_inspItemID;//[eINSP_WND_CNT][MAX_WINDOW_CNTS];

	int* m_groupIndexCnts;//[eINSP_WND_CNT]; //검사 항목별 그룹 종류 개수.
	int** m_groupIndex;//[eINSP_WND_CNT][MAX_GROUP_CNTS]; //검사 항목별 가지고 있는 그룹 인덱스

	int* m_groupID;//[MAX_GROUP_CNTS];
	int* m_groupWndCnts;//[MAX_GROUP_CNTS]; // 그룹에 속해있는 윈도우 개수.
	int** m_groupWndID;//[MAX_GROUP_CNTS][MAX_WINDOW_CNTS];  //그룹 인덱스 별 윈도우 번호

	int** m_inspWndOrder; //실제 검사한 순서

protected:
	Im::PIL_ID* m_milApp;	// mil application
	Im::PIL_ID* m_milSys;	// mil system

public:
	virtual int Init(Im::PIL_ID* milApp, Im::PIL_ID* milSys);
	virtual int Init(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	virtual int Exit();
	virtual void SetResolution(int fovWidth, int fovLength, double resolX, double resolY);
	void SetFovWidth(int nFovWidth) { m_fovWidth = nFovWidth; }
	void SetFovLength(int nFovLength) { m_fovLength = nFovLength; }

	int GetFovWidth() { return m_fovWidth; }
	int GetFovLength() { return m_fovLength; }

	virtual void SetInspItemInfo(int* inspItemCnts,	int** inspItemID, int** inspWndOrder);
	virtual void SetInspGroupInfo(int* groupIndexCnts, int** groupIndex, int* groupID, int* groupWndCnts, int** groupWndID);

	virtual void SetPartParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize);
	virtual void SetPartParam_Foreign(InspPartInfo* boardInfo);
	
	double GetResolX() const { return m_resolX; }
	double GetResolY() const { return m_resolY; }
public:	
	Coordinate SetCoordinate(InspPartInfo* boardInfo);
	Coordinate SetCoordinate(InspPartInfo* boardInfo, InspPartParam param, AlignResult * sAlignRes = NULL);
	Coordinate SetCoordinate(InspPartInfo* boardInfo, TeachParam* param);
	Coordinate SetCoordinate(PartSearchParam* pam);
	CPoint CvtBoradToPixel(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double ctRoiRealCx, double ctRoiRealCy, InspPartInfo* boardInfo = NULL);
	CPoint CvtBoradToPixel(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double ctRoiRealCx, double ctRoiRealCy, double& dRetPixelX, double& dRetPixelY, InspPartInfo* boardInfo = NULL);
	void CvtPixelToBoard(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double* retX, double* retY, int mode = DOWN_LEFT);

	char* ReturnString(int id) ;
	CString GetDefectCodeName(int id);
	Coordinate SetCoordinate_Offset(Coordinate input, AlignResult offset);
	BOOL IsAnyAngle(double angle);
	double CalcRotateAngle(double orgAngle);
};

