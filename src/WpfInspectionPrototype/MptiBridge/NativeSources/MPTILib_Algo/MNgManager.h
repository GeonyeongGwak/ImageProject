#pragma once

#ifndef __MPTI_DEF_H__
#define __MPTI_DEF_H__

#include "MPTIDef.h"

#endif

//#ifdef _MPTI_EXPORT

#include "Manager.h"
#include "ProcMil.h"
#include "alp_file.h"
#include "InspParamDef.h"
#include "../jpeg_Dll/jpeg.h"
// SHKang 2017/04/04
//#include "jpeg/jpeg.h"
//#include "jpeg_Dll/jpeg.h"

#include "Proc3D.h"

enum m_eLightPosition
{
	m_eLightPosition_TOP,
	m_eLightPosition_MIDDLE,
	m_eLightPosition_BOTTOM,
	m_eLightPosition_USER,
	m_eLightPosition_SIDE,
	m_eLightPosition_UV,
	m_eLightPosition_REVIEW,
	m_eLightPosition_ColorMap2,
	m_eLightPosition_UserMap,
};

class CMNgManager : public CManager
{
public:
	CMNgManager(void);
	virtual ~CMNgManager(void);


	Im::PIL_ID temp_R;
	Im::PIL_ID temp_G;
	Im::PIL_ID temp_B;

private:
	CProcMil* m_milProc;

private:
	CString m_ngPath;
	NgParam m_ngParam;

	int m_roiMargin;

	InspPartInfo *m_pInspBoardInfo;
	InspPartInfo *m_pInspBoardInfo_Foreign;
	InspPartParam *m_pParamArray;
	int m_nParamArraySize;
	CProc3D m_proc3d;
	int m_nSave_ImgType;

public:



	int Init(/*Im::PIL_ID* milApp, Im::PIL_ID* milSys*/); //factor  =  micro meter / pixel
	void SetPath(CString path);
	int Exit();
	void SetResolution(int fovWidth, int fovLength, double resolX, double resolY);
	void SetInspItemInfo(int* inspItemCnts,	int** inspItemID, int** inspWndOrder);
	void SetInspGroupInfo(int* groupIndexCnts, int** groupIndex, int* groupID, int* groupWndCnts, int** groupWndID);
	void SetPartParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize);
	void SetPartParam(InspPartInfo* boardInfo);
	void SetPartParam_Foreign(InspPartInfo* boardInfo);


	void SetZmapMargin(int margin);
	void SetNgParam(NgParam param);

	CString MakeFilename_Disp(CString moduleName, CString partName);	// ATW 2013.04.05 change
	CString MakeFilename_Ng(CString moduleName, CString partName, CString wndName, int lightNum);
	CString MakeDirname();

	BOOL IsZmapUse(int index = -1);

	void ProcSaveNgImg(CString path, CString moduleName, CString partName);
	void ProcSaveDisplaynNg(CString dispPath, CString moduleName, CString partName, int *pArrUseSideImg = NULL, CString LinkagePath = NULL, bool bLinkage = false, CString RefID = NULL, int nAngleColor = 0, int exportSaveOption = 0);  // CKH  20160519 ???? ?? 
	void ProcSaveDisplaynForeignNg(CString dispPath, CString partName, AForeignResult vforeign, int nMesIndex, CString UserPath, CString Filename, int extension, int nDivide, int nGapX, int nGapY, float* pfOff3D);
	void SaveNgImage(void* srcMilImg, CRect roi, CString fullPath);

	BOOL IsDefect(int inspType);
	BOOL IsMountDefect();
	BOOL IsOCRDefect();

	BOOL IsInMount(int* retIndex = NULL);
	CRect GetRectRoi(InspPartParam param);
	CRect GetRectRoi();
	CRect GetRectBoundaryRoi();
	CRect GetRectRoi_3D();
	void CreateDir(CString Path);

	void SaveDispImage(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nWidthStep, int exportSaveOption = 0, BOOL bIsSideBtm = FALSE, BOOL bIsMixImage = FALSE);
	void SaveDispImage_Multi(CString fullPath, BOOL zmapUse, UCHAR * retColorImg, int nWIdthStep, int exportSaveOption, BOOL bIsSideBtm, BOOL bIsMixImage, RoiColorBuf partImgColorBuf, InspRoiImgBuf partImgBuf, ZmapData partZmapData);
	void SaveColorMap2UseImage(CString fullPath, int nWidthStep);
// 	void SaveDispImage_Foreign(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide);
// 	void SaveDispImage_Foreign_new(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide, float* pfOff3D);
	void ProcSaveNgImg(CString path, CString moduleName, CString partName, CRect roi, double angle, BOOL isLeadNg, CRect* leadNgRect, int leadNgCount, CRect* solderNgRect, int solderNgCount, CRect* gapNgRect, int gapNgCount, CRect* leadCountNgRect, int leadCountNgCount);
	float * Save3DImage(CString fullPath, int * nSizeX = NULL, int * nSizeY = NULL, int exportSaveOption = 0);

	int SaveInspPartImage_SharedMemory(CString fullPath, UCHAR* retColorImg, UCHAR *ucAngleColor, int nWidthStep, bool bSavePtt = true); // SHKang 2017/03/13
	int SaveInspPartImage_SharedMemory_Multi(CString fullPath, UCHAR * retColorImg, UCHAR * ucAngleColor, int nWidthStep, bool bSavePtt, RoiColorBuf partImgColorBuf, InspRoiImgBuf partImgBuf, ZmapData partZmapData);
	int SaveInspPartImage_AngleColor(UCHAR *ucAngleColor, int nWidthStep);
	int SavePotImage_SharedMemory(CString fullPath, int roiArea); // SHKang 2017/03/13
	int SavePotImage_SharedMemory_Multi(CString fullPath, int roiArea, InspRoiImgBuf partImgBuf);
	int SaveInspPartImage(SaveImgParamData tgParam, UCHAR * ucColorImg, bool bImageSave, UCHAR * ucColorImg_AC, bool bLeadOK, bool bSaveUserLight = true, int exportSaveOption = 0, InspPartInfo* pInsp = NULL, InspectionResult* m_inspectionResult = NULL); // SHKang 2017/03/13

	int SaveMultiInspPartImage(std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool ,int > TupleSaveImages);
	int SaveImageParamDelete(std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool , int> TupleSaveImages);
	int SaveMultiInspPartImage(SaveImgParamData tgParam, bool bImageSave, InspPartInfo * InspBoardInfo);
	void Check2DImage(UCHAR* ucImg2D, int nW, int nH);
	void Check2DImage_Multi(UCHAR * ucImg2D, int nW, int nH, InspRoiImgBuf partImgBuf);
	int GetROISize(int & nSizeX, int & nSizeY, int & nWidthStep); // SHKang 2017/07/06

	int SaveP3DImage_SharedMemory(CString fullPath);
	int SaveP3DImage_SharedMemory_Multi(CString fullPath, RoiByteBuf Save3DRawData);
	void GetUserMapImage(InspRoiImgBuf partImgBuf, UCHAR * ptrbyDst, int nType);

	void GetUserMapImage(UCHAR * ptrbyTopRedBuffer, UCHAR * ptrbyTopGreenBuffer, UCHAR * ptrbyTopBlueBuffer, UCHAR * ptrbyTopWhiteBuffer, UCHAR * ptrbyMidRedBuffer, UCHAR * ptrbyMidGreenBuffer, UCHAR * ptrbyMidBlueBuffer, UCHAR * ptrbyBotRedBuffer, UCHAR * ptrbyBotBlueBuffer, UCHAR * ptrbyDst, int nType, int nWidth, int nHeight);

	BOOL UserMapSet(int nUserMap, lightData * pRedLightData);

	void SavePotImage(CString strFullPath, int roiArea);
	void SavePotImage_Multi(CString strFullPath, int roiArea, InspRoiImgBuf partImgBuf);
	void SavePutImage(CString strFullPath, int roiArea);
	void SavePutImage_Multi(CString strFullPath, int roiArea, InspRoiImgBuf partImgBuf);
	void SavePstImage(CString strFullPath, int roiArea, int nSideCamNum);
	void SavePstImage_Multi(CString strFullPath, int roiArea, int nSideCamNum, InspRoiImgBuf partImgBuf);
	void SaveTifImage(UCHAR *uImgBuff, CString strFullPath, int nWidth, int nHeight, int nBand);

	UCHAR* CopyPVImage;
	UCHAR* CopyPVImage_Foreign;
	UCHAR* SavePVImage(void* ColorImage, int width, int height);
	UCHAR* SavePVImage_Foreign(void* ColorImage, int width, int height);
	UCHAR* GetPVImage(int* nWidth, int* nHeight, int* nWidthSrc);	
	UCHAR* GetPVImage_Foreign(int* nWidthStep, int* nHeight, int* nWidth);
	int CreateJpgBuffer(UCHAR * srcImage, int srcWidth, int srcHeight, int & dstWdith, int & dstHeight, UCHAR ** dstImage); // SHKang 2017/05/26

	int* PVSize;
	int* PVSize_Foreign;
	CProc3D Getproc3d() { return m_proc3d; }	

	// SHKang 2017/04/04
	//CJPEG m_Jpeg;
	void Set_JpegImage(jpeg_formatType type, jpeg_formatQuality quality);
	int Save_JpegImage_1ch(UCHAR * ucSrc, int width, int height, CString  file_path);
	int Save_JpegImage_3ch(UCHAR * srcR, UCHAR * srcG, UCHAR * srcB, int width, int height, CString  file_path);
	int Save_JpegImage(UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, CString  file_path, int nJpegQuality);


	int DefectImgCrop_AngleColor(InspPartInfo* pBoard, InspectionResult* inspectionResult, int roiSizeX, int roiSizeY, UCHAR* ucAngleColor, int roistep, int nMachineCode, int ColorMode, CString sJobPath = _T(""));
	int DefectImgCropRect(cv::Rect& DefectRect, cv::Rect& ClipRect, cv::Mat& PartACImg, int nIMGSzX, int nIMGSzY, float fCx, float fCy, int &nClipW, int &nClipH, int& nStx, int& nSty, int& nDeW, int& nDeH);
	void BasicAlgo_CalcROICompose(InspAlgo& sInspAlgo, cv::Mat& dst);
	BOOL PadBW_CalcRoiCompose(AlgoPadBW &pAlgoPadBW, int nSelectedLightIdx, BOOL bInspMode, PIAL::PI_Buff* RstCompose);
	void Processing_DS(cv::Mat& gray, cv::Mat& Color, cv::Mat& dst, bool bNomalize);
	void Processing_DSGray(cv::Mat& gray, cv::Mat& Color, cv::Mat& dst, bool bNomalize);
	void SaveImage_Segmentation_AI(InspPartInfo* pInspBoardInfo, CString sPath);
	void SaveImage_2DColor(InspPartInfo* pInspBoardInfo, CString sPath);
	std::map<int, void(CMNgManager::*)(cv::Mat& gray, cv::Mat& Color, cv::Mat& dst, bool bNomalize)> ImgProcessing;
public:
	CJPEG m_Jpeg;
	float m_fFatorR;
	float m_fFatorG;
	float m_fFatorB;
	float m_fFatorACR;
	float m_fFatorACG;
	float m_fFatorACB;
	float m_fFatorMGR;
	float m_fFatorMGB;
	float m_fFatorBR;
	float m_fFatorBB;
	float m_fFatorBGR;
	float m_fFatorBGB;
	lightData m_sLightData[3];
	byte m_byDefaultAC[2];

	//lightData m_sLightData[3];
	int m_nUserMap;
	lightData m_sUserLightData[3];

};

//#endif //_MPTI_EXPORT


#pragma region exposure_define
MPTIDLL int  MPTI_SetNgPath(wchar_t* path); 
MPTIDLL UCHAR* MPTI_SavePVImageNG(int* nWidth, int* nHeight, int* nWidthSrc);
MPTIDLL UCHAR* MPTI_SavePVImageNG_Foreign(int* nWidth, int* nHeight, int* nWidthSrc);

/*
// SHKang 2017/04/05
MPTIDLL int MPTI_Set_JpegImage(jpeg_formatType type, jpeg_formatQuality quality);
MPTIDLL int MPTI_Save_JpegImage_1ch(UCHAR * ucSrc, int width, int height, wchar_t* file_path);
MPTIDLL int MPTI_Save_JpegImage_3ch(UCHAR * srcR, UCHAR * srcG, UCHAR * srcB, int width, int height, wchar_t* file_path);
*/
MPTIDLL int MPTI_Save_JpegImage(UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, wchar_t* file_path, int nJpegQuality);
MPTIDLL float * MPTI_Save3DImage(wchar_t* path, int * nSizeX = NULL, int * nSizeY = NULL, int exportSaveOption = 0);
MPTIDLL void MPTI_SetData(UCHAR * ucSrc, UCHAR * ucDst, int width, int height, int nWidthStep, int nBand);
#pragma endregion exposure_define