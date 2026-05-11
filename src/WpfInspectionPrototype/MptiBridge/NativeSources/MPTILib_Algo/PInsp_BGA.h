#pragma once
#include "pinsp.h"
#include "Proc3D.h"
#include "ProcMil_BGA.h"

enum BGA_stepID
{
	eBGAStepID_HEIGHT = 0,
	eBGAStepID_ECCENTRICITY,
	eBGAStepID_SIZE,
	eBGAStepID_COUNTS
};


class CPInsp_BGA : public CPInsp
{
public:
	CPInsp_BGA(void);
	virtual ~CPInsp_BGA(void);


private:
	//3D inspection Class
	CProc3D m_proc3d;
	CProcMil_BGA* m_procMil;

private:
	void* m_fovImage_insp;
	void* m_fovImage_teach;


	void* m_fovImage_T;
	void* m_fovImage_M;
	void* m_fovImage_B;

	//zmap data
	ZmapData m_inspZmapData;
	ZmapData m_teachZmapData;

	double m_resolX;
	double m_resolY;
	int m_fovWidth;
	int m_fovLength;


private:
	/////////////////////////////
	//	inspection parameter   //
	/////////////////////////////

	//auto inspection parameter for BGA
	InspParamBGA* m_inspParam;
	//inspection coordinate for BGA
	Coordinate m_inspCoordinate;


private:
	////////////////////////
	//	teach parameter   //
	////////////////////////

	//teach parameter for BGA
	TeachParamBGA* m_teachParam;
	//teach coordinate for BGA
	Coordinate m_teachCoordinate;

private:
	//////////////////////////
	//	inspection result   //
	//////////////////////////
	RstInspBGA_Wnd m_inspResult;


public:
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	int CloseDevice();

	int SetInspParam(void* itemParam, void* targetImg, ZmapData zmap, Coordinate cdn);
	int SetInspParam(void* itemParam, InspImgBuf img, ZmapData zmap, Coordinate cdn);
	int SetTeachParam(void* itemParam, void* targetImg, int teachType, ZmapData zmap, Coordinate cdn);
	int SetTeachParam(void* itemParam, InspImgBuf img, int teachType, ZmapData zmap, Coordinate cdn);

	int InspProc_BGA();
	int ProcAuto();
	int ProcStep(int stepID);
	void InitResultStruct(RstInspBGA_Wnd* rst);
	void GetInspRst(RstInspBGA_Wnd* dst);

private:

	int BGA_Calculate();
	int Eccentricity();
	int ComponentSize();

	void GetHeight(float* zmapData, UCHAR* blobImg, UCHAR* lutImg, int sizeX, int sizeY, int blobCx, int blobCy, int sampleRectSize, double hPercent, float* retAbsH, float* retRelH);
	void GetHeight2(float* zmapData, UCHAR* blobImg, UCHAR* lutImg, int sizeX, int sizeY, int blobCx, int blobCy, int sampleRectSize, double hPercent, float* retAbsH, float* retRelH);

	float GetHeight_REL(float* zmapData, int sizeX, int sizeY, UCHAR* blobImg);
	void CalcCog(double angle, int cx, int cy, int sizeX, int sizeY, double blobCx, double blobCy, double* retCogX, double* retCogY);

	void DataSort(float* srcData, int dataCnt, float* dstData);

};

