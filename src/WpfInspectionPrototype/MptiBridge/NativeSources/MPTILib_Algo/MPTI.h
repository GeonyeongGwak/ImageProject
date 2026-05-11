
#pragma once

#ifndef __MPTI_DEF_H__
#define __MPTI_DEF_H__
#include <Windows.h>
#include "MPTIDef.h"

#endif
#include "PZmapDef.h"
#include "PILibrary.h"
#include "alp_file.h"


#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "PInsp_Algo.h"
#include "PInsp_WireBonding.h"
//#include "PInsp_Barcode.h"

#include "PInsp_Pattern.h"
#include "PInspAlgoWrapper.h"
#include "InspManager.h"
#include "HistogramAnalysis_New.h"

#include "CNN_Inference.h"
#include "BubbleThreadMng.h"

#include <random>
enum InspWndType : byte
{
	WndCoating,
	WndNonCoating,
	WndHighCoating
};
enum diffmachinetype
{
	DiffMachineNOT,
	DiffMachineUV,
	DiffMachineClient2dSOF,	/*SOF용*/
	DiffMachineClient3dSOF,	/*SOF용*/
	DiffMachineREVIEW,
	DiffMachineShuttle,
	DiffMachineDualside,	//양면 검사기
	DiffMachineDualsideCI,	// CI and dualside
	DiffMachineLeadTab,
	DiffMachineXRAY,
	DiffMachineBareBoard,
	DiffMachineMars,
	DiffMachineWire,
};

enum GrabTaller
{
	SingleGrabDefault = 0,
	SingleGrabPlus,
	SingleGrabFast,
	SingleGrabRGBPlus,
	SingleGrab3Buk,
	SingleGrab3BukPlus,
	SingleGrab3BukFast,
	SingleGrabOnly2D,
	SingleGrabPPZT_16,
	SingleGrabPPZT_16_Taller,
	SingleGrabPPZT_16_3buk,
	SingleGrabPPZT_16_3buk_Taller,
	SingleGrabPPZT_12,
	SingleGrabPPZT_12_Taller,
	SingleGrabPPZT_12_3buk,
	SingleGrabPPZT_12_3buk_Taller,
	SingleGrabPPZT_16_Zeus,
	SingleGrabPPZT_16_Zeus_3buk,
	SingleGrabPPZT_6,
	SingleGrabPPZT_6_3buk,
	SingleGrabPPZT_4,
	SingleGrabPPZT_4_3buk,
	SingleGrabColor_White,
	SingleGrabFast_4PZT,
	SingleGrab3BukFast_4PZT,
	SingleGrabShot,
	SingleGrabPPZT_DP,
	SingleGrabPPZT_DP_3buk,
	SingleGrabPPZT_DP_Taller,
	SingleGrabPPZT_DP_3buk_Taller,
	SingleGrabProjector4_8_12,
	SingleGrabProjector4_10_14,
	SingleGrabProjector4_4_8,
	SingleGrabProjector4_6_10,
	SingleGrabProjector8_4_12,
	SingleGrabProjector8_6_14,
	SingleGrabProjector12_4_16,
	SingleGrabProjector12_6_18,
	SingleGrabProjector4_12_16
};

enum Color2DImg_LIGHT_POS
{
	eColor2DImg_Light_Top_R = 0,
	eColor2DImg_Light_Top_G = 1,
	eColor2DImg_Light_Top_B = 2,
	eColor2DImg_Light_Top_W = 3,
	eColor2DImg_Light_Mid_R = 4,
	eColor2DImg_Light_Mid_B = 5,
	eColor2DImg_Light_Btm_R = 6,
	eColor2DImg_Light_Btm_B = 7,
};

#define dfUser_LightCnt	10		// User light 합성 개수
class CMPTI
{
public:
	CMPTI(void);
	~CMPTI(void);

public:
	int m_nPOCRAlgoVersion;
	int m_nUniqueFPointWidth;
	int m_nUniqueFPointHeight;
	std::vector<cv::KeyPoint> m_arrUniqueFPoints[124];
	/*jsl::Blob m_blob;
	jsl::Blob& GetjslBlob() { return m_blob; }*/
	double m_dBtmSideResX;
	double m_dBtmSideResY;
	int m_nlnspIndexAI;
	bool m_bUseAIModule;
	int m_SegThreshold;
	int m_nAIOption;
	bool bInspOne;
	bool m_bCheckWindowRotate;
	bool m_bPattern_ForcedCharDiv;

	Im::PIL_ID* m_milApp;	// mil application
	Im::PIL_ID* m_milSys;	// mil system

	bool isUseImagePilLib() {
		return true;
	};
	//int m_nCompositeLightMode;	// 0 Top, 1 Top + Bottom
	float *m_pfZmapData;
	int m_nZmapWidth;
	int m_nZmapHeight;

	UCHAR *m_parrPartBuf[eM2C_NUM];
	UCHAR *m_parrFovBuf[eM2C_NUM];
	int m_nSizeX_parrFovBuf[eM2C_NUM];
	int m_nSizeY_parrFovBuf[eM2C_NUM];
	Im::PIL_ID m_bufRawDataFov[eM2C_NUM];
	Im::PIL_ID m_bufSideDataFov[3];
	int m_nSizeXRawData;
	int m_nSizeYRawData;
	Im::PIL_ID m_bufRawDataFov_Side[4][3];
	UCHAR *m_pArrPartBuf_Side[4][3];	// Side Img R,G,B	

	//------------------------------------------------------------------------------------------
	// UI용... 4Byte Align 사용함
	int GetPttFileLoad_SPI(CString strPath, int* nWidth, int* nHeight, bool bCallSPI);
	int GetPotFileLoad_SPI(CString strPath, int* nWidth, int* nHeight, float* fPixelResX, float* fPixelResY);
	//------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------
	// 검사 용.. 4Byte 사용하지 않음.. Original
	int GetPstFileLoad_RawData(CString strPath, int* nWidth, int* nHeight);
	int GetPttFileLoad_RawData(CString strPath, int* nWidth, int* nHeight);
	//------------------------------------------------------------------------------------------

	int GetPotFileLoad_RawData(CString strPath, int* nWidth, int* nHeight, float* fPixelResX, float* fPixelResY);

	int SetRawDataFovInfo(CString filePathPtt, CString filePathPot, int nX, int nY, bool bSeparatedPart, int* nWidth, int* nHeight ,int nFovSizeX = 4096 , int nFovSizeY = 3076);
	void SetMachineOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax);
	UCHAR * GetPttFileChannel(int nChannel);

	UCHAR* GetRawDataFovBuffer(int nChannel);
	UCHAR* GetRawDataFovBuffer_Side(int nChannel, int nCamIdx);
	float* GetRawDataZmap();
	int GetRawDataZmapHeight();
	int GetRawDataZmapWidth();

	float m_factorR;
	float m_factorG;
	float m_factorB;
	float m_factorBR;
	float m_factorBB;

	float m_factorUVR;
	float m_factorUVG;
	float m_factorUVB;

	int SetColorFactor(float factorR, float factorG, float factorB, float factorBR, float factorBB);
	void SetCompositeLightMode(int LightMode, float compoBtmR, float compoBtmG, float compoBtmB);
	UCHAR *pUcAlgoImgLight;


	bool m_bSideOriginalSize;
	int m_nCompositeLightMode;	// 0 Top, 1 Top + Bottom
	int m_nCameraType;
	float m_fCompoBtmR;
	float m_fCompoBtmG;
	float m_fCompoBtmB;

	bool m_bUseImagePilLib;

	UCHAR* m_ucPadCalcBuffer[dfUser_LightCnt];
	UCHAR* m_ucPadRstBuffer[dfUser_LightCnt];

	int DeletePttArray();
	int DeletePotArray();
	int DeletePstArray();
	int m_sequence;					// sequence mode
	int m_bUseBubbleInsp;
private:
	UINT m_digitSizeX;
	UINT m_digitSizeY;

	
	int m_nSaveDebugData;
	int m_nSaveBubNGImg;		//NYJ 2018/07/24
	int m_nSaveBubInspResult;
	BOOL m_bUseLimitBubNGCnt_FOV;		//NYJ 2019/04/29
	int m_nBubNGLimitCnt_FOV;					//NYJ 2019/04/29
	int m_nExtMachineType;
	BOOL m_bUseLimitBubNGCnt_SplitBRDArea;		//NYJ 2019/08/07
	int m_nBubNGLimitCnt_SplitBRDArea;					//NYJ 2019/08/07
	double m_dBubBoardSplitSize;
	BOOL m_bUseVisibleSplitBRDGrid;

	BOOL m_bUseBubMaxSizeChk;		//NYJ 2019/04/29
	double m_dBubMaxSizeSpec;					//NYJ 2019/04/29
	
	int m_sizeBoardX;
	int m_sizeBoardY;
private:
	
public:
	int SetSizeBoard(double sizeX, double sizeY);
	SIZE GetSizeBoard();
	int GetExtMachineType() { return m_nExtMachineType; };
	int m_LogLevel =0;
	int GetCompositeLightMode() { 
		//m_nCompositeLightMode;
		//TRACE(_T("GetCompositeLightMode  == 0 "));
		return 0;
	}
	void AddLog(CString sMessge);
	void AddLog_Dev(CString sMessge, int nLogLv = 0);
	void AddLog_OCR(wchar_t  szMessage[]);
	void AddLog_OCR(CString sMessge);
	void SetLogLv(int nLogLv) {
		m_LogLevel = nLogLv;// nLogLv; 
	};
	BOOL IsMachineTypeMars()	{		return true;	}
	int GetLogLv() { return m_LogLevel; };
	bool CheckLogLv(int nLogLv);
	int nCalcWidthStep(bool bUseWidthStep, int OrgImgWidth);

	CString GetWireDebugDataFullPath(int nfootKind, CString csAddSubPath, CString csAddName, int nFormat);

	void SetResolution(int fovWidth, int fovLength, double resolX, double resolY);
	void SetFootOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax);

	BOOL IsMachineTypeDualSide();

	void SharedInit(int toolid , int nMachineType , int * nToolType);


	//Inspection
	//Wire Bonding

	CPInsp_Algo m_InspAlgo;
	CPInsp_WireBonding* m_pWireBondingInsp;
// 	void  * m_pPatternInsp;
// 	void  * m_pPOCRInsp;
// 	void  * m_pBarocdeInsp;
	InspManager* m_InspMng;
	CString timeCNNDataDir;		//c++ 현재시간 NYJ 2018/09/13
	CString m_sCurJobPath;		//NYJ 2018/09/13
	bool m_bLengthByAngel;
	int m_nCurInspFovIdx;
	CPInsp_Algo& GetInspAlgo() { return m_InspAlgo; }
	CPInsp_WireBonding* GetWireBondingInsp() { return m_pWireBondingInsp; } //YBJI 2019/10/01
// 	void* GetPatternInsp() { return m_pPatternInsp; }
// 	void* GetPocrTeach() { return m_pPOCRInsp;  }
// 	void* GetBarocdeInsp() { return m_pBarocdeInsp; }
	UINT GetDigitSizeX() const { return m_nSizeXRawData; }
	UINT GetDigitSizeY() const { return m_nSizeYRawData; }
	
	//PInspAlgoWrapper* GetPInspWrapper() { return m_InspMng->m_PInspWrapper; }

public:
	int GetSequence() const { return m_sequence; }
	void SetSequence(int nSequence) { m_sequence = nSequence; }
	void SetExtInspInit(int fovWidth, int fovLength, double resolX, double resolY);
	CString GetForeignDebugDataFullPath(int nFovCnt, CString csAddName, int nMode = -1, int nFormat = 0, bool bDEBUG = false);
	UCHAR * GetGrabOutputBuf_Color(int nCycleIdx, int nMode, int nChannel);
	BOOL IsMachineTypeUV();
	void* GetSeqGrabBufID(int sequence, int fovNo, int channel2D, int projectionmode = SingleGrabDefault, int nGen2D = 0);
	void* GetSeqGrabBufID_3D(int sequence, int fovNo, int channel3D, int projectionmode = SingleGrabDefault, int nGen2D = 0);


	int GetSaveDebugData() { return m_nSaveDebugData; }
	int GetReverseMachinePos();
	int GetSaveBubNGImg() { return m_nSaveBubNGImg; }
	int GetSaveBubInspResult() { return m_nSaveBubInspResult; }
	BOOL GetUseBubNGLimitOption() { return m_bUseLimitBubNGCnt_FOV; }		//NYJ 2019/04/29
	int GetBubNGLimitCount() { return m_nBubNGLimitCnt_FOV; }		//NYJ 2019/04/29
	BOOL GetUseBubMaxSizeCheckOption() { return m_bUseBubMaxSizeChk; }		//NYJ 2019/04/29
	double GetBubMaxSizeSpec() { return m_dBubMaxSizeSpec; }		//NYJ 2019/04/29
	PInspAlgoWrapper* GetPInspWrapper() { return m_InspMng->m_PInspWrapper; }
	int UseBubbleInsp() { return m_bUseBubbleInsp; }		//NYJ 2019/06/17
	bool GetUseMultiProcess() { return true; }
	// Machine Setting
	private:
		int m_nDIffMachine; 
		POINTF m_poCurFovCenter;
	public:
		bool m_bUseSideCamera = false; 
		bool m_bUseLensDistortion;	// NYJ 2021/03/02
		int UseLensDistortion() { return m_bUseLensDistortion; }		//NYJ 2021/06/15

		csml::BubbleThreadMng *m_pCSMLThreadMng;
		std::shared_ptr<csml::InferCoil> _inferCoil;
		//std::shared_ptr<csml::InferOCR> _inferOCR;
		std::shared_ptr<csml::PyOCR_AI_AutoModule> _inferOCR; //kji 2021/05/07
		int SetSideFovMilAlloc(int * nFOVSizeX, int * nFOVSizeY);
		bool IsForeignEnable(int nType = m_eForeignInsp_Total);
		CProcMil * GetProcMil();
		InspManager * GetMInspManager();
		POINTF GetForeignCurFovCenter() { return m_poCurFovCenter; }
		void ROIImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage, double dAngle = 0);
		void FrameWriterInit();
		void RoiImageCompose_LT(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
			int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
			int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
			int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage, double dAngle = 0);
		void FullImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage);
		void POCR_UseAI_Set(int nInspAIIdx, bool bInspOne, bool bUseAIModule, int nAIOption, int nThreshold);
	public:
		std::map<int, cv::Mat> m_vAIPart2DBuf;		//AI Segmentation 2D Image Buffer
		std::map<int, bool> m_vAIPart2DComplete;		//AI Segmentation result Complete Flag

		int m_nRedLgtPos_2DImg;		//0~7 범위의 조명 인덱스(Color2DImg_LIGHT_POS)
		int m_nGreenLgtPos_2DImg;
		int m_nBlueLgtPos_2DImg;
};



extern CMPTI *g_pMPTI;

MPTIDLL int MPTI_Init();
MPTIDLL int MPTI_Dispose();

MPTIDLL int MPTI_SetInspParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize);
MPTIDLL int MPTI_InspProc();
MPTIDLL int MPTI_GetInspectionResult(InspectionResult* retResult);

MPTIDLL int MPTI_GetPttFileLoad(BSTR filePath, int* nWidth, int* nHeight, bool bCallSPI);
MPTIDLL int MPTI_SetRawDataFovInfo(BSTR filePathPtt, BSTR filePathPot, int nX, int nY, bool bSeparatedPart, int* nWidth, int* nHeight);
MPTIDLL void MPTI_SetMachineOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax);

MPTIDLL void MPTI_SetWireOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax, float fDiffHigh, float fDiffLow, float fDiameterWire, int nFootPatternMatchStopScore);

MPTIDLL UCHAR* MPTI_GetPttFileChannel(int nChannel);
MPTIDLL UCHAR* MPTI_GetRawDataFovBuffer(int nChannel);
MPTIDLL UCHAR* MPTI_GetRawDataFovBuffer_Side(int nChannel, int nCamIdx);
MPTIDLL float* MPTI_GetRawDataZmap();
MPTIDLL int MPTI_GetRawDataZmapHeight();
MPTIDLL int MPTI_GetRawDataZmapWidth();

MPTIDLL void  MPTI_FULLImgCompose(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR *ptrbyWhiteBuff,
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	double dROICenterX, double dROICenterY, int nRoiWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage);

MPTIDLL void MPTI_FullImageClaculCompose(const LightTypeBuf &sLightImgBuf, UCHAR* ptrbyResultImage);

MPTIDLL int MPTI_SetColorFactor(float factorR, float factorG, float factorB, float factorBR, float factorBB);
MPTIDLL void MPTI_SetCompositeLightMode(int LightMode, float compoBtmR, float compoBtmG, float compoBtmB);
MPTIDLL BOOL MPTI_SetAngleColor(byte* byData);

MPTIDLL BOOL MPTI_TeachFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, int nSelectArea, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt);
MPTIDLL BOOL MPTI_InspFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, RstAlgoFoot &sRstAlgo, TotalInspExceptArea stTieArea, int nSelectArea, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt);


MPTIDLL BOOL MPTI_TeachFoot_Color(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, int nTabIndex);
MPTIDLL BOOL MPTI_InspFoot_Color(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, RstAlgoFoot &sRstAlgo, TotalInspExceptArea stTieArea, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, int nTabIndex);
MPTIDLL BOOL MPTI_LoadFootModelList(wchar_t* FolderPath);
MPTIDLL BOOL MPTI_CheckModelList(wchar_t* FolderPath);

MPTIDLL BOOL MPTI_InspWire(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, RstAlgoWire &sRstAlgo, TotalInspExceptArea stTieArea, int nSelectArea, float fTeachH, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt);

MPTIDLL BOOL MPTI_WireAutoSearch(AlgoFoot& foot1, AlgoFoot& foot2, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, int rect1Num, RECT*  pRect1, int Rect2Num, RECT* pRect2, WireSearch& footPos,int nSearchType);


MPTIDLL BOOL MPTI_InspWireShort(int nWireNum, WireRst* Wirerst, WndAlgoImg &sWndAlgoImg, UCHAR* ucArrDstImg);
MPTIDLL BOOL MPTI_HistogramFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo,TotalInspExceptArea stTieArea, int nSelectArea, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt);
MPTIDLL BOOL MPTI_InspBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, RstAlgoBlob &sRstAlgo, TotalInspExceptArea stTieArea, bool bDisplayInvert);
MPTIDLL BOOL MPTI_TeachBGA(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, PIAL::TeachBGA &sRstAlgo, TotalInspExceptArea stTieArea, bool bDisplayInvert);
MPTIDLL BOOL MPTI_DrawBGABinary(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, UCHAR *pUcImgBlob);
MPTIDLL void MPTI_DeleteBumps(struct tagTeachBump* pBump );
MPTIDLL BOOL MPTI_InspMeanGray(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, double &dResult, byte insptype, int &nReviewMin, int &nReviewMax, TotalInspExceptArea stTieArea);
MPTIDLL BOOL MPTI_InspPadBW(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR * pUcPadBWDst, RstAlgoPadBW &sRstAlgo, double dStartX, double dStartY);
            
MPTIDLL BOOL MPTI_InspBlob2(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, RstAlgoBlob &sRstAlgo, TotalInspExceptArea stTieArea, bool bDisplayInvert);

MPTIDLL BOOL MPTI_InspTilt(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgTilt, RstAlgoTilt &sRstAlgo, AlignResult * sAlignRes, int nAlignResCnt);

// MPTIDLL int MPTI_Color_GetColorCIEView(UCHAR* retCIEImg, int dstCieSizeX, int dstCieSizeY, UCHAR* retImg, int dstSizeX, int dstSizeY, POINT* retPt, WndInfo sWndInfo, AlgoColorOpt * ptrAlgoColorOpt, bool bIsLoadFovRawDataImage = false);
MPTIDLL int MPTI_GetCIEMapAboutImage(UCHAR* rstCIEMap, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, UCHAR* rstBin, AlgoColorOpt& ColorOpt, bool bUseColorMap2 = false);
MPTIDLL void MPTI_GetColorBuffer_Part(UCHAR* buf_R, UCHAR* buf_G, UCHAR* buf_B, int nWidth, int nHeight, bool bTeach = false, int nInspAC = 0);
MPTIDLL BOOL MPTI_InspTilt(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgTilt, RstAlgoTilt &sRstAlgo, AlignResult * sAlignRes, int nAlignResCnt);
MPTIDLL BOOL MPTI_ColorMapSetValue(lightData* pRedLightData);
MPTIDLL UINT MPTI_GetDigitSizeX();
MPTIDLL UINT MPTI_GetDigitSizeY();

MPTIDLL void MPTI_SetRawDataMachineType(int nType);
MPTIDLL int MPTI_GetRawDataMachineType();

MPTIDLL BOOL MPTI_BinalizeBodyBlob(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* ucArrDstImg, int nMaskViewingMode, AlgoColorOpt &ptrAlgoColorOpt, RstAlgoBodyBlob *pRstAlgo, TotalInspExceptArea stTieArea, bool bIsLoadFovRawDataImage);
MPTIDLL BOOL MPTI_InspBodyBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg,WndInfo &sWndInfo, BOOL bTeach, AlgoColorOpt &ptrAlgoColorOpt,
	RstAlgoBodyBlob *pRstAlgo, CRect &rcBlobBody, UCHAR *pUcImgBlob, TotalInspExceptArea stTieArea, bool bIsLoadFovRawDataImage);

MPTIDLL BOOL MPTI_DrawNGBlob(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, TotalInspExceptArea stTieArea);

MPTIDLL BOOL MPTI_NGBlobHistogram(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, TotalInspExceptArea stTieArea, UCHAR *retHistoImg, int *nRetRangeMin, int *nRetRangeMax, int *nRetRangeAvg, int* nRetMaxFreqValue);


MPTIDLL BOOL MPTI_BinalizeBodyEdge(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* ucArrDstImg, int nMaskViewingMode, AlgoColorOpt &ptrAlgoColorOpt, bool bIsLoadFovRawDataImage);
MPTIDLL BOOL MPTI_InspBodyEdge(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, BOOL bTeach, AlgoColorOpt &ptrAlgoColorOpt,
	RstAlgoBodyEdge *pRstAlgo, UCHAR *pUcImgBlob,bool bIsLoadFovRawDataImage);

MPTIDLL BOOL MPTI_TeachQFN(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, PIAL::TeachQFN &sRstAlgo, TotalInspExceptArea stTieArea, bool bDisplayInvert);
MPTIDLL void MPTI_DeleteUnits(struct tagTeachUnit* pUnit);

MPTIDLL BOOL MPTI_InspGWire(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, BOOL bTeach, AlgoColorOpt &ptrAlgoColorOpt, RstAlgoGWire *pRstAlgo, UCHAR *pUcImgBlob, bool bIsLoadFovRawDataImage);
MPTIDLL BOOL MPTI_BinalizeGWire(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* ucArrDstImg, int nMaskViewingMode, AlgoColorOpt &ptrAlgoColorOpt, bool bIsLoadFovRawDataImage);
MPTIDLL void MPTI_CheckerBoarderImageSet(int * nWidth, int * nHieght, int * nFootCnt , bool bFullImage);
MPTIDLL void MPTI_CheckerBoarderImageSet_forDebug(int * nWidth, int * nHieght, int * nFootCnt, bool bFullImage);

MPTIDLL void MPTI_GetFootPartResultImage(UCHAR *pUcImgFootPart, bool bCheckBoard, int nWidth, int nHeight);
MPTIDLL void MPTI_GetFootPartResultImage_forDebug(UCHAR *pUcImgFootPart, bool bCheckBoard, int nWidth, int nHeight);
MPTIDLL void MPTI_SetSaveImagePath_PartList(wchar_t * sSaveImagePath);
MPTIDLL void MPTI_SetSaveImagePath_PartList_forDebug(wchar_t * sSaveImagePath);

MPTIDLL cv::Mat FillPolygonImg(cv::Mat ucMask, cv::Mat binImg, int width, int height, POINT* ptDstPoint, bool isExceptROI, int nUsedInspPolygon, int* cnt, UCHAR* ucArrOverlapImg, int nChanel, int nExcept);
MPTIDLL uchar* MatToBytes(cv::Mat image, uchar * bytes);
MPTIDLL cv::Mat BytesToMat(UCHAR* src, int sizeX, int sizeY, int colortype);
//MPTIDLL void  MPTI_PatternCnt(int cnt);
//MPTIDLL int  MPTI_LoadModelList_algo(fileAlgoPath* filePath, wchar_t* OCRPath, wchar_t* POCRPath, wchar_t * modelPath_BodyBlob);
//MPTIDLL int  MPTI_CheckModelList(wchar_t* modelPath_Pattern, wchar_t* modelPath_OCR, wchar_t* modelPath_POCR, wchar_t * modelPath_BodyBlob, wchar_t * modelPath_Foot);
MPTIDLL BOOL MPTI_SharedFlagCheck();
MPTIDLL BOOL MPTI_SharedInit(int nAlgoToolIndex,int nMachineType , int * nTooType);
MPTIDLL void MPTI_SharedExit();
MPTIDLL BOOL MPTI_SharedInspection();
MPTIDLL void MPTI_AddLog(wchar_t * s);

MPTIDLL void MPTI_FrameWriterInit();
MPTIDLL void MPTI_Set2DChannel_LightPos(int nR, int nG, int nB);