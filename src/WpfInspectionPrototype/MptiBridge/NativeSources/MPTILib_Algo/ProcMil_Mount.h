#pragma once
//MIL 

#include "ProcMil.h"
#include "ModelManager_Geo.h"

enum milprocmount_result
{
	eMNT_SUCCESS = 0,
	eMNT_FAIL,
	eMNT_OVERWRITE
};



class CProcMil_Mount : public CProcMil
{
public:
	CProcMil_Mount(void);
	virtual ~CProcMil_Mount(void);

private:
//	Im::PIL_ID m_milPatModel;
//	Im::PIL_ID m_milPatResult;
//	Im::PIL_ID m_milPatModel_Rotate;

	//mil init , free
public:
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib=false);
	int FreeMil();

public:
	void Binarize(UCHAR* userSrc, int sizeX, int sizeY, UCHAR* userDst, int thresh, BOOL invert=FALSE);
	void HistoEqulaize(Im::PIL_ID milSrc, Im::PIL_ID milDst, double min, double max);

	void MaskTrueImg(void* milSrc, UCHAR * mask, UCHAR * dst, int cx, int cy, int sizeX, int sizeY);
	void MaskTrueImg(UCHAR * src, UCHAR * mask, UCHAR * dst, int sizeX, int sizeY);
	void MakeBodyMaskImg(UCHAR* bodyBlobImg, int sizeX, int sizeY, CRect bodyRect, UCHAR* retMaskImg);

	//CMilBlobResult* CalcBlobM(Im::PIL_ID milBinSrc, int minArea, int* retBlobCnts , BOOL eraseBorderBlob = FALSE, BOOL fillHole = FALSE, Im::PIL_ID milDst = M_NULL);
	void GetBlobResult(CMilBlobResult* retResult);
	int CalcBlob_ForCnt(Im::PIL_ID milBin);
	int SelectBlob_MaxArea(int blobNum, UCHAR* userDst, int sizeX, int sizeY);
	int SelectBlob_MaxArea(int blobNum, Im::PIL_ID milDst);

	CMilBlobResult* BlobProc(void* milSrc, UCHAR * mask, int cx, int cy, int sizeX, int sizeY, UCHAR* retTrueImg = NULL);
	int BlobProc(UCHAR* userSrc, int sizeX, int sizeY, BOOL fillOutside, UCHAR* blobImg, CString wndName = _T(""), int fovIndex = -1);
	int MakeBodyImg(UCHAR* userSrc, int sizeX, int sizeY, CRect partRect, CRect wndRect, UCHAR* userDst, CRect* bodyRect, int* retBlobCx, int* retBlobCy); //for inspection
	int MakeBodyImg2(UCHAR* userSrc, int sizeX, int sizeY, CRect partRect, CRect wndRect, UCHAR* userDst, CRect* bodyRect, int* retBlobCx, int* retBlobCy); //for teaching
	BOOL CheckBodyRect(CRect partRect, CRect wndRect, CRect blobRect);

	void GetEraseBoarderData(UCHAR* userSrcImg, int sizeX, int sizeY, UCHAR* retMask);


	void CalcSampleArea(CRect bodyRect, CRect* retSampleArea);

	void CalcSampleArea_UserDefCop(CPoint* nodalPoint, int bodyW, int bodyH, CRect* retSampleRect, int* retCx, int* retCy
		, double* cxCOP, double* cyCOP, double* widthCOP, double* heightCOP);

	void CalcSampleArea(CPoint* nodalPoint, int bodyW, int bodyH, CRect* retSampleRect, int* retCx = NULL, int* retCy = NULL);
	float CalcHeight(float* zmapRoiData, UCHAR* bodyImg, int roiSizeX, int roiSizeY, CRect bodyRect, UCHAR* retBodyLut = NULL);
	float CalcHeight2(float* zmapRoiData, UCHAR* bodyImg, int roiSizeX, int roiSizeY, CRect bodyRect, UCHAR* retBodyLut = NULL);
	double CalcGradient(float* zmapRoiData, int roiSizeX, int roiSizeY, CRect bodyRect, float hThresh, double* retVerAngle, double* retHorAngle);
	float CalcGradient2(float* zmapRoiData, int roiSizeX, int roiSizeY, CRect bodyRect, double bodyAngle, float hThresh, double* retVerAngle, double* retHorAngle);
	float CalcGradient3(float* zmapRoiData, int roiSizeX, int roiSizeY, CRect bodyRect, double bodyAngle, float hThresh, double* retGradient);

	float CalcGradient4(float* zmapRoiData, int roiSizeX, int roiSizeY, int bodyW, int bodyH, CPoint* nodalPoint, float hThresh, float hThresh2, float dftVal, double bodyAngle, double* retTiltAngle, 
		bool bIsUserDefCop, double* cxCOP, double* cyCOP, double* widthCOP, double* heightCOP);
	//float CalcGradient4(float* zmapRoiData, int roiSizeX, int roiSizeY, int bodyW, int bodyH, CPoint* nodalPoint, float hThresh, float hThresh2, float dftVal, double bodyAngle, double* retTiltAngle);

	//pattern
public:	
//	int AllocPatModel(void* milSrc, int cx, int cy, int width, int height, double rotateAngle, int rotateW, int rotateH);
//	int AllocPatModel(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH);
//	void DeleteModelBuf();
//
//	int WritePatternModel(void* milSrc, int cx, int cy, int width, int height, double rotateAngle, CString strPath);
//	int WritePatternModel(UCHAR* userSrc, int width, int height, double rotateAngle, CString strPath);
//	int ModelLoad(CString strPath);
//	void ModelSave(Im::PIL_ID milModel, CString strPath);
//
//	int SearchPattern(UCHAR* userSrc, int cx, int cy, int width, int height);
//	int GetPatResult(double* retScore, double* retAngle, double* retPosX, double* retPosY);
//
//
//	void SetAccuracy(int speedFactor, int accuracy);
//	void SetAngleMode(double posLimit, double negLimit, double accuracy);
//	void SetSearchStartAngle(double angle);


	void DrawBody(int sizeX, int sizeY, POINT* pt);
	void SelectLine(UCHAR* img, int sizeX, int sizeY, POINT** retPt, int* retCnt);
	void SaveIntImage(int* src, int sizeX, int sizeY, CString path);

	//huj 2013/12/12
	BOOL RemakeMountBinImg(UCHAR* src, UCHAR* mask, UCHAR* dst, int sizeX, int sizeY, CRect wndRect, CRect wndMarginRect);
	int CalcNoisePos(Im::PIL_ID milSrc, CRect blobRect, CRect wndRect, BOOL chipPos);

	void MorphologyOpen(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, int iteration);
	void MorphologyClose(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, int iteration);
	void FillHole(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight);
	void FillDot(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, bool* arrDot);
	void DrawBodyLine(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, POINT* pt);
	CMilBlobResult* GetDamageResult(UCHAR* ucSrc, UCHAR* ucDst,int nWidth, int nHeight, bool *ptrbResult);
	void DrawResultRect(UCHAR* ucSrc, UCHAR* ucDst, int nWidth, int nHeight, CRect rcRect);

	int TeachPattern(cv::Mat OrgImg, int nImgWid, int nImgLen, double wndAngle,
					double bodyWidth, double bodyHeight, double cogX_roi_pixel, double cogY_roi_pixel, 
					CString sModelPath);
	int InspPattern(cv::Mat OrgImg, int nImgWid, int nImgLen, double wndAngle, 
					double cogX_roi_pixel, double cogY_roi_pixel, 
					CString sModelPath, double & dPatAngle);
	int ClearModelList(int nLane);
	int LoadModelList(CString sPath);
	int CheckModelList(CString path);
	std::shared_ptr<ModelFile_Pat> m_ModelFile;
	std::shared_ptr<CMModelManager<ModelFile_Pat>> m_ModelMng;
};