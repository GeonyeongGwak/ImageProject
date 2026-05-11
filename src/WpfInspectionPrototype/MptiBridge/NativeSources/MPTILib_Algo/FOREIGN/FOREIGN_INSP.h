#include "../stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "ipp.h"
#include "../InspParamDef.h"
#include "../jpeg_Dll/jpeg.h"

enum m_eFrTact
{
	m_eFrTact_Inspection_Foreign_Set = 0,
	m_eFrTact_Inspection_Foreign,

	m_eFrTact_InspForeign_3D,
	m_eFrTact_InspForeign_Sticker,
	m_eFrTact_InspForeign_InspEnd,
	m_eFrTact_InspForeign_CalcResult,
	m_eFrTact_InspForeign_END,

	m_eFrTact_Insp_Data,
	m_eFrTact_Insp_Module_Data,
	m_eFrTact_Insp_GetInspArea,
	m_eFrTact_Insp_FindBody,
	m_eFrTact_Insp_Set,
	m_eFrTact_Insp_Get2DFOV,
	m_eFrTact_Insp_TRACE,
	m_eFrTact_Insp_AREA,
	m_eFrTact_Insp_Foreign,
	m_eFrTact_Insp_Foreign2D,
	m_eFrTact_Insp_Gray,
	m_eFrTact_Insp_Copper,
	m_eFrTact_Insp_GrayBub,
	m_eFrTact_Insp_Warpage,
	m_eFrTact_Insp_END,

	m_eFrTact_GetInspArea_WP,
	m_eFrTact_GetInspArea_Sticker,
	m_eFrTact_GetInspArea_Manual,
	m_eFrTact_GetInspArea_Bin_Sub,
	m_eFrTact_GetInspArea_Bin_WP,
	m_eFrTact_GetInspArea_Bin_M,
	m_eFrTact_GetInspArea_Bin,
	m_eFrTact_GetInspArea_AMIM,
	m_eFrTact_GetInspArea_SOLDER,
	m_eFrTact_GetInspArea_WP_SkipPart,
	m_eFrTact_GetInspArea_WP_IMG,
	m_eFrTact_GetInspArea_Hole,
	m_eFrTact_GetInspArea_AMIM_EX,
	m_eFrTact_GetInspArea_IM_XOR,
	m_eFrTact_GetInspArea_HOLE_XOR,
	m_eFrTact_GetInspArea_Module,

	m_eFrTact_InspWP_Set,
	m_eFrTact_InspWP_Area,
	m_eFrTact_InspWP_Check,
	m_eFrTact_InspWP_SetWP,
	m_eFrTact_InspWP_Insp,
	m_eFrTact_InspWP_END,

	m_eFrTact_InspFR_SET,
	m_eFrTact_InspFR_Bin,
	m_eFrTact_InspFR_SizeChk,
	m_eFrTact_InspFR_Blob,
	m_eFrTact_InspFR_END,

	m_eFrTact_InspForeign_HA_SET,
	m_eFrTact_InspForeign_HA_Bin,
	m_eFrTact_InspForeign_HA_Rst,

	m_eFrTact_Total,
};
#pragma once
class FOREIGN_INSP
{
private:
#define CR_MAP_SIZE 1280
#define SQR(x) ((x)*(x))
#define FOREIGN2D_RSTCNT 4

	double m_resolX;
	double m_resolY;
	int m_fovWidth;
	int m_fovLength;
	int INSP_MIN_SIZE = 10;
	int m_nFR_TraceID = 100;
	float m_fFr_FINDBODY_ShiftX = 0.3f;
	float m_fFr_FINDBODY_ShiftY = 0.3f;
	float m_fFr_FINDBODY_FindX = 0.5f;
	float m_fFr_FINDBODY_FindY = 0.5f;
	CJPEG m_JP;
	InspForeignInfo m_ForeignData;
	ForeignParamROI *m_ForeignParamROI;
	InspPartInfo *m_pInspBoardInfo_Foreign;
	InspFovForeignResult* m_inspForeignResult;		// 현재 FOV의 이물 검사 결과

	UCHAR* m_ucForeignHalfBuffer[eM2C_NUM];
	UCHAR* m_ucForeignQuaterBuffer[eM2C_NUM];
	UCHAR* m_ucForeignHalfOrgBuffer[eM2C_NUM];
	UCHAR* m_ucForeignQuaterOrgBuffer[eM2C_NUM];

	UCHAR* m_ucForeignHalfRstBuffer[FOREIGN2D_RSTCNT];
	UCHAR* m_ucForeignQuaterRstBuffer[FOREIGN2D_RSTCNT];
	struct PseudoColor
	{
		int r;
		int g;
		int b;
	};
	PseudoColor m_crPseudo[CR_MAP_SIZE];

public:
	FOREIGN_INSP();
	virtual ~FOREIGN_INSP();

#pragma region FR
	float m_fFrTact[m_eFrTact_Total];
	void InitAlgo(int fovWidth, int fovLength, double resolX, double resolY);
	void Init_PseudoMap();
	void CloseDevice();
	InspForeignInfo GetForeignData() { return m_ForeignData; }
#pragma endregion FR

#pragma region MINSPM
	void SetForeignParam(InspForeignInfo ForeignParam, ForeignParamROI* vForeignParamROI);
	int FR_EXCEPT(FR_Bin sData, FR_EXT_SRC sInspData, FR_EXT_RST* vROI);
	int  FR_HIT_AREA(FR_FIND_HIT sInspData, POINT* poPoly);
	int SET_MODULE_AREA(cv::Mat imgBin, int nFovW, int nFovH, double dMinPer, POINT* poPoly, bool bBTM = false);
	std::vector<cv::Point> FIND_MODULE_AREA(cv::Mat imgBin, cv::Mat imgRst, int nFovW, int nFovH, double dMinPer);
	std::vector<cv::Point> FIND_MODULE_AREA_R(cv::Mat imgBin, int nFovW, int nFovH, double dMinPer);
	void GRAY_BIN(int nMin, int nMax, int nRange, cv::Mat imgGray, cv::Mat imgBin);
	std::vector<cv::Point> BIN_MAX_AREA(cv::Mat imgBin, cv::Mat imgMAX, int nFovW, int nFovH, double dMinPer, bool bArea = false, bool bONE = false);
	void CompareCIE_BTM(cv::Mat* imgRGB, cv::Mat imgBin, ColorXYInfoForeign vCol, POINTF* arrPo);
	int FR_AutoPseudo(float* pfSrc, UCHAR* pucWP, int nW, int nH, double dMin, double dMax, BOOL bSET);
	int CalcCorrWarpage(ForeignData sInspData, AForeignResultWP &retResult, float* pfArrTact);
	void SaveWarpageSUB(ForeignData sInspData, CString dir);
	void GetWP3D(CString sPath, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, double dPer);
	void GetWP3DM(CString sPath, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, IN UCHAR* retFullImgM, IN int nWidthM, IN int nHeightM, RECT rcClip, double dMin, double dMax, double dPer, double dFacX, double dFacY);
	void GetWP3DPTT(wchar_t* sPath, IN float* retFullImg, IN UCHAR* ptrTR, IN UCHAR* ptrTG, IN UCHAR* ptrTB, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, double dPer, double dFacW, double dFacH);
	void GetWP3D(float* pf3D, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, int nModuleID, CString sPath);
#pragma endregion MINSPM

#pragma region SAVE_NG
	void ForeignNG_Save(InspFR_Save sData, UCHAR * ucDst);
	void ForeignNG_Save_OFF(InspFR_Save sData, UCHAR * ucDst);
	void SaveDispImage_Foreign(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide);
	void SaveDispImage_Foreign_new(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide);
	void SaveDispImage_Foreign_new(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide, float* pfOff3D, int nGrabMode, int nRunSave);
	void SaveDispImage_Foreign(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide, float* pf3D, UCHAR* pucTR, UCHAR* pucTG, UCHAR* pucTB, UCHAR* pucBR, UCHAR* pucBB, int nFov2DW, int nFov2DH, int nFOVW, int nFOVH);
	int Save_JpegImage(UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, CString  file_path, int nJpegQuality);
	double GetResizeFactor(int viewX, int viewY, int imgX, int imgY, BOOL rotate = FALSE);
	void ResizeImg(UCHAR* userSrc, UCHAR* userDst, int nSrcW, int nSrcH, int nDstW, int nDstH, int bandSize, double factor);
	CString GetForeignDebugDataFullPath(int nFovCnt, CString csAddName, int nMode = -1, int nFormat = 0, bool bDEBUG = false);
#pragma endregion SAVE_NG
	
#pragma region COLOR
	void Get2DFOV(int projectionmode, const InspPartInfo *pInspBoardInfo, MPTI_InspectionMode vInspectionMod, cv::Mat *imgORG2D, int nDivide, int nFovID, bool bSave);
	void Get2DForeign(InspForeignInfo sData, FR_Bin sFRBin, int nMode, const InspPartInfo *pInspBoardInfo, UCHAR* pucColor, MPTI_InspectionMode vInspectionMod, cv::Mat *imgORG2D, int nDivide, int nFovID, BOOL bSave, UCHAR* pucGray = NULL);
	POINTF GetCIEPoint(cv::Mat*	pConvertMat, double dRed, double dGreen, double dBlue, int nPixelCount);
	bool pvProcPointInPolygon(POINTF* ptArray, int nArrayCts, POINTF point);
	bool IsBlack(double dR, double dG, double dB, int nThresholdType, double dMin, double dMax);
	void Get2DForeignGray(FR_Bin sData, int nMode, LightTypeBuf sLightImg, cv::Mat imgColorTemp, int nFovID, BOOL bSave, UCHAR* pucGray = NULL);
	void Get2DForeignColor(InspForeignInfo sData, FR_Bin sFRBin, int nMode, LightTypeBuf sLightImg, cv::Mat imgColorTemp, int nFovID, BOOL bSave);
	void RemakePoly(POINT* srcPoint, int count, int offsetX, int offsetY, POINT* retPoint);
	void GetAngleColorFR(InspRoiImgBuf sLightImg, lightData* sLightData, cv::Mat* img);
	void GetAngleColorFR(LightTypeBuf sLightTypeBuf, lightData sLightData, cv::Mat img);
	double CompareCIE_Foreign(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, int width, int length,
		float factorR, float factorG, float factorB, int ptPolyCnt, POINT pts[10][POLYGON_POINT_CNTS], ColorXYInfoForeign* vColorXYInfoForeign, UCHAR* MaskImg, bool bInvert);
	bool PtInPolygonForeign(POINT pt, POINT *pts, int ptNum);
	void Get2DFOV_Resize(InspRoiImgBuf sFov, FR2DData pfRst);
	int SetRGB(UCHAR * ucArrDstImg, long long nIndex, float fH, float fMin, float fMax);
	float GetRGBH(UCHAR ucR, UCHAR ucG, UCHAR ucB, double dMin, double dMax);
	int GetRGB_Index(UCHAR ucR, UCHAR ucG, UCHAR ucB);
	int GetRGB_Index_Find(UCHAR ucR, UCHAR ucG, UCHAR ucB);
	int GetColorRGB(IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT* rcRGB);
	int GetColorRGB(UCHAR* imgR, UCHAR* imgG, UCHAR* imgB, int nW, int nH, RECT* rcRGB);
	int FrFindArea(ForeignData sInspData, InspRoiImgBuf sFov, FR_Find sFind, RECT* rcROI);
	void SaveFind(CString cName, cv::Mat img, int nFovID, bool b3D = false);
	void GetGrayBin(cv::Mat imgGray, ColorXYInfoForeign vInfo, cv::Mat imgBin);
	double GetColorArea(InspRoiImgBuf sFov, FR_Bin sData, RECT rcROI);
	void Get2DFOV_Resize(int projectionmode, InspPartInfo *pInspBoardInfo, MPTI_InspectionMode vInspectionMod, int nDivide, int nFovID, BOOL bSave);
#pragma endregion COLOR

#pragma region COMM
	std::vector<std::vector<cv::Point>> FOREIGN_INSP::CalcBlob(cv::Mat imgSrc, cv::Mat imgDst, int nMinArea, bool bMaxBlob = false, bool bFillHole = false);
	void SaveReleaseWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, CString path, int bandSize = 1);
	void SaveReleaseWorkImg(cv::Mat img, CString fileName, CString path);
	void CreateDir(CString Path);
	BOOL IsExistDir(CString path);
	void SavePTR(CString str, cv::Mat img3D, int nBit = 32);
	void SaveZmapPTR(float *pZmap, CString filePath, int nW, int nH, int nBit = 32);
	void SaveWorkImg(UCHAR* buff, int sizeX, int sizeY, CString fileName, int bandSize = 1, bool bSaveR = false);
	void GetClipImage_LT(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int Sx, int sy, int roiSizeX, int roiSizeY, int band = 1, double bAngle = 0);
	float GetCropZmap_LT(float* src, float* dst, int orgSizeX, int orgSizeY, int nStX, int nStY, int roiSizeX, int roiSizeY);
	void RotateImg_ipp(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR** userDst, int* retDstSizeX = NULL, int* retDstSizeY = NULL);
	bool RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor = false, bool bLinear = true, bool bUseOrgSize = false);
	void GetClipImage(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst, int cx, int cy, int roiSizeX, int roiSizeY, int band = 1);
	float GetCropZmap(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY);
	void SaveDebugImg(cv::Mat img, CString FullPath);
	bool SizeChk(cv::Mat imgBin, double dArea, int nDivide, int nMode, int nNotSize);
#pragma endregion COMM

#pragma region INSP
	bool InspWarpage(InspForeignInfo sForeign, float* p3DWP, cv::Mat imgInspArea, int nW, int nH, int nModuleID, int nAM, ForeignData sInspData, InspFovForeignResult* ForeignResult, cv::Rect rcROI);
	void CheckWPMargin(InspForeignInfo sForeign, ForeignData sInspData, int nW, int nH, int nModuleID, cv::Rect &rcROI);
	void SetPartParam_Foreign(InspPartInfo* boardInfo);
	int ReleaseInspForeignResult();
	int GetInspForeignResult(InspFovForeignResult* retResult);
	void DeleteForeignImgBuffer();
	int Inspection_Foreign_new(ForeignData sData_ROI, FR2DData sFR2DData, ForeignParamROI* arrROI);
	int InspForeignAlgo(float* pfOrg3D, int nOrgW, int nOrgH, InspFovForeignResult* ForeignResult, ForeignData sInspData, int nFovIDX, FR2DData sFR2DData, ForeignParamROI* arrROI);
	byte ExceptForeignImage(int nMode, cv::Mat imgExcept, ForeignData sData_ROI);
	void ExceptForeign(int nType, cv::Mat imgExcept, ForeignData sData_ROI);
	void InspFRArea(ForeignData sInspData, cv::Mat imgArea);
	bool SetROI(ForeignData sInspData, int nIndex, cv::Mat img, int nW, int nH, double dFactorX, double dFactorY, byte byValue);
	bool SetExceptForeignImage(int nIndex, int nType, CRect rtFOV, UCHAR* pucDst, int nW, int nH, byte byValue = 0, bool bExtraPer = false, float fExtraPixelCnt = 0);
	int FillOutOfInspArea(RECT rcROI, int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue = 0, BOOL bConvertExceptROI = FALSE, bool bExtraPer = false, float fExtraPixelCnt = 0);
	bool PolygonAreaCalc();
	void FillPolygon(cv::Mat Dst, POINTF *pts, int nMaxCount, cv::Scalar color);
	void MorErode(cv::Mat Src, cv::Mat Dst, int nShape = 0, int ksize = 3);
	bool PtInPolygon(POINTF pt, POINTF *pts, int ptNum);
	void CalcForeignResult(InspFovForeignResult *arrTempRst, InspFovForeignResult* ForeignResult);
	void SetExtForeignParam(InspPartInfo * _ForePartInfo, ForeignData _ForeinData, FR2DData _Fr2Data, InspForeignInfo _ForeignInfo, ForeignParamROI * _ForeParamROI, int nSelectedIdx);
	BOOL InspBlobForForeign(InspForeignInfo sForeign, float* p3DFr, float* pf3D2, float* p3DWP, int* pnImg3DSub, int nW, int nH, InspFovForeignResult* ForeignResult, int nFovID, ForeignData sInspData, FR2DData sFR2DData, ForeignParamROI* arrROI);
	bool GetInspArea(InspForeignInfo sForeign, ForeignData sInspData, FR2DData sFR2DData, float* p3DFr, float* p3DWP, int* pnImg3DSub, cv::Mat *imgInspArea, cv::Mat *imgFR, int nW, int nH, int nModuleTotal, int nFovID, int* nArrModuleID, int* nArrModuleCnt);
	void ExceptArea(int nExceptArea, cv::Mat imgSrc, cv::Mat imgDst, bool bMorErode);
	bool InspWarpage(InspForeignInfo sForeign, float* p3DWP, cv::Mat imgInspArea, cv::Mat *imgFR, int nW, int nH, int nModuleID, int nFovID, int nAM, ForeignData sInspData, InspFovForeignResult* ForeignResult);
	bool InspForeign(InspForeignInfo sForeign, float* p3D, cv::Mat imgInspArea, cv::Mat *imgFR, cv::Mat *imgORG2D, int nW, int nH, int nMode, int nFovID, ForeignData sInspData, InspFovForeignResult* ForeignResult, FR2DData sFR2DData);
	BOOL ChekcForignROI(FR_Bin sData, CRect rcBlob, double dRstArea, int nAreaRootPixX, int nAreaRootPixY, int nDivide, bool bNotSizeChk);
	int GetForeignModuleNumber(InspForeignInfo sForeign, ForeignData sInspData, POINTF ptRst, int nDivide, MPTI_InspectionMode vInspectionMod, int nContainModuleNum);
	int GetForeignModuleNumber(InspForeignInfo sForeign, ForeignData sInspData, RECT rcROI, int nDivide, MPTI_InspectionMode vInspectionMod, int nContainModuleNum);
	void GetFindBody(float * pf3DOrg, float * pf3D2Org, cv::Mat imgInspArea, cv::Mat *imgFR, int nW_Org, int nH_Org, ForeignData sData, int nDivideOrg, int nFovID, bool bSaveImg);
	void FindROI(int nW, int nH, CRect rcPart, CRect* rcFind);
	void NotSizeChange(cv::Rect rtClip, CRect* rcFind);
	float GetBodyH(cv::Mat img3D, float f3DT, CRect rcROI, int nType, int nDivide);
	void Except3DArea(cv::Mat img, int nW, int nH, CRect rcROI, bool bExtPer, float fExtPix, bool bQ, bool bOrg);
	bool UseData(int nData, int nType);
	bool GetFindROI_Derivative(cv::Mat img3D2, cv::Mat imgArea, cv::Mat *imgFR, cv::Rect rtClip, CRect rcPart, int nDivide, CRect *rcFind_Temp);
	CRect GetFindCenter(cv::Mat imgBlob, CRect rcPart, std::vector<std::vector<cv::Point>> allRst, int nClipL, int nClipT, int nDivide, CRect* rcFind, CRect* rcFind2);
	void GetExceptFR(InspForeignInfo sForeign, ForeignData sInspData, cv::Mat imgFREXT, int nW, int nH, int nMode);
	void FR_INSP_TRACE(InspForeignInfo sForeign, ForeignData sInspData, cv::Mat *imgFR, int nW, int nH);
	void GetRGB(float fHei, UCHAR * ucArrDstImg, long long nIndex, float fPseudoColorMin, float fPseudoColorMax);
	void GetExceptFR(InspForeignInfo sForeign, ForeignData sInspData, InspFovForeignResult* ForeignResult, cv::Mat imgFREXT, int nW, int nH);
	bool InspForeign_HA(InspForeignInfo sForeign, float* p3D, cv::Mat imgInspArea, cv::Mat *imgFR, int nW, int nH, int nMode, int nFovID, ForeignData sInspData, InspFovForeignResult* ForeignResult, FR2DData sFR2DData);
	void GetInspAreaROI(InspForeignInfo sForeign, ForeignData sInspData, ForeignParamROI* arrROI, cv::Mat *imgInspArea, cv::Mat *imgFR, int nW, int nH, int nModuleTotal, int nFovID, int* nArrModuleID, int* nArrModuleCnt);
	void SET_IMG_DATA(cv::Mat img, ForeignParamROI roiFR, int nDivide, int nVal = 255);
	bool InspSolderBall(cv::Mat src, std::vector<CRect> & rcBlob, int* nMaxIndex = NULL, float* ptrResult = NULL, double* ptrArrRst = NULL);
	void GetSolderBallPos(std::vector<cv::Point>&  Points, double *dCX, double *dCY, double *dRadius, double *dErr);
	void AddLOG(CString sType, CString sFunc, CString sFunc2, CString sMSG, DWORD st = 0, m_eLogLv eLogLv = m_eLogLv::m_eLogLv_None);
	void InspWarpageGood(int nModuleID, int nAM, InspFovForeignResult* ForeignResult);
#pragma endregion INSP
};

#pragma region EX
MPTIDLL int MPTI_FR_EXCEPT(FR_Bin sData, FR_EXT_SRC sInspData, FR_EXT_RST* vROI);
MPTIDLL int MPTI_FR_HIT_AREA(FR_FIND_HIT sInspData, POINT* poPoly);
MPTIDLL void MPTI_Get2DFOV_Resize(InspRoiImgBuf sFov, FR2DData pfRst);
MPTIDLL int MPTI_FR_AutoPseudo(float* pfSrc, UCHAR* pucWP, int nW, int nH, double dMin, double dMax, BOOL bSET);
MPTIDLL int MPTI_CalcCorrWarpage(ForeignData sInspData, AForeignResultWP &retResult, float* pfArrTact);
MPTIDLL void MPTI_FrACImage(InspRoiImgBuf sLightImg, lightData sLightData, UCHAR* pucImage);
MPTIDLL void MPTI_Foreign_Value_RGB(InspRoiImgBuf sFov, RECT* rcROI);
MPTIDLL void MPTI_Foreign_FullMap(IN UCHAR* retFullImg, IN int nFullW, IN int nFullH, UCHAR* byR, UCHAR* byG, UCHAR* byB);
MPTIDLL POINTF MPTI_Foreign_Value(ForeignData sInspData);
MPTIDLL void MPTI_SaveZmapPTR(float *pZmap, wchar_t* filePath, int nW, int nH);
MPTIDLL void MPTI_SaveWarpageSUB(ForeignData sInspData, wchar_t* sPath);
MPTIDLL void MPTI_GetWP3D(wchar_t* sPath, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, double dPer);
MPTIDLL void MPTI_GetWP3DM(wchar_t* sPath, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, IN UCHAR* retFullImgM, IN int nWidthM, IN int nHeightM, RECT rcClip, double dMin, double dMax, double dPer, double dFacX, double dFacY);
MPTIDLL void MPTI_GetWP3DPTT(wchar_t* sPath, IN float* retFullImg, IN UCHAR* ptrTR, IN UCHAR* ptrTG, IN UCHAR* ptrTB, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, double dPer, double dFacW, double dFacH);
MPTIDLL void MPTI_GetWP3DIMG(float* pf3D, IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT rcClip, double dMin, double dMax, int nModuleID, wchar_t* sPath);
MPTIDLL int MPTI_GetColorRGB(InspRoiImgBuf sFov, RECT* rcRGB);
MPTIDLL int MPTI_GetFullColorRGB(IN UCHAR* retFullImg, IN int nWidth, IN int nHeight, RECT* rcRGB);
MPTIDLL int MPTI_SaveDisplaynForeignNg(InspFR_Save sData, UCHAR * ucDst);
MPTIDLL int MPTI_FrFindArea(ForeignData sInspData, InspRoiImgBuf sFov, FR_Find sFind, RECT* rcROI);
MPTIDLL double MPTI_GetColorArea(InspRoiImgBuf sFov, FR_Bin sBin, RECT rcROI);
#pragma endregion EX

#pragma region EX_COL
MPTIDLL int MPTI_GetFullColorCIEView_Foreign(IN UCHAR* retFullImg, IN int nFullW, IN int nFullH, UCHAR* retCie, int nCieW, int nCieH, UCHAR* retImg, int nWidth, int nHeight, POINT* pt, RECT rcROI);
MPTIDLL int MPTI_COLOR_GetColorHistogramView_Foreign(int nBin, int projectionmode, RECT rcROI, UCHAR* pucDstImg);
MPTIDLL int MPTI_GetColorCIEView_Foreign(InspRoiImgBuf sFov, UCHAR* retCie, int nCieW, int nCieH, UCHAR* retImg, int nWidth, int nHeight, POINT* pt, RECT rcROI);
#pragma endregion EX_COL

#pragma region EX_INSP
MPTIDLL void MPTI_SetForeignParam(InspForeignInfo ForeignParam, ForeignParamROI* vForeignParamROI);
MPTIDLL int MPTI_InspProc_Foreign(ForeignData sInspData, FR2DData sFR2DData, ForeignParamROI* arrROI);
MPTIDLL int MPTI_GetInspForeignResult(InspFovForeignResult* retResult);
MPTIDLL int MPTI_ReleaseInspForeignResult();
#pragma endregion EX_INSP