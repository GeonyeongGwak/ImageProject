#pragma once

#include "pinsp.h"
#include "ProcMil_OCR.h"
#include "OpenCV_Functions.h"

#define FONTSIZE_S 15
#define FONTSIZE_M 25

#define Timecheck 0

enum proc_ocr_statuse
{
	
	ePROC_OCR_NG= 0,
	ePROC_OCR_OK,
	ePROC_OCR_NOTFOUND_FILE,
	ePROC_OCR_NOTFOUND_FONT
};



enum ocr_stepID2
{
	//eOStepID_IMGBUF_ALLOC = 0,

	eOStepID2_Local = 0,
	eOStepID2_RESEGVALUE,
	eOStepID2_USERVALUE,
//	eOStepID_PEMVALUE,	

	eOStepID2_Local_REV,
	eOStepID2_RESEGVALUE_REV,
	eOStepID2_USERVALUE_REV,		
	//eOStepID_PEMVALUE_REV,
// 
// 	eOStepID_THRESHOLD_TRACKING,			//2016.12.01 shkim threshold tracking Insp
// 	eOStepID_THRESHOLD_TRACKING_REV,		

	eOStepID2_MIX_PEMVALUE,		//LJH 2016.05.27	영상 합친 후 검사
	eOStepID2_MIX_PEMVALUE_REV,	//LJH 2016.05.27	영상 합친 후 검사 (영상 회전)

	eOStepID2_COUNT
};

enum ocr_stepID
{
	eOStepID_IMGBUF_ALLOC = 0,

	eOStepID_USERVALUE,
	eOStepID_USERVALUE_REV,		
	eOStepID_AUTO,
	eOStepID_PEMVALUE,	

	eOStepID_AUTO_REV,
	eOStepID_PEMVALUE_REV,

	eOStepID_MIX_PEMVALUE,		//LJH 2016.05.27	영상 합친 후 검사
	eOStepID_MIX_PEMVALUE_REV,	//LJH 2016.05.27	영상 합친 후 검사 (영상 회전)

	eOStepID_COUNT
};

typedef struct tagOCRResultTemp
{
	int strCount;
	int charCount;
	double stringScore; 
	double charScore[MAX_STRLEN];
	double strPosX;
	double strPosY;
	double strAngle;
	CString str;
	double charX[MAX_STRLEN];
	double charY[MAX_STRLEN];
	double charWidth[MAX_STRLEN];
	double charHeight[MAX_STRLEN];
	double charangle[MAX_STRLEN];
}OCRResultTemp;


// enum ocr_stepID
// {
// 	eOStepID_USERVALUE = 0,
// 	eOStepID_AUTO,
// 	eOStepID_PEMVALUE,
// 	eOStepID_USERVALUE_REV,
// 	eOStepID_AUTO_REV,
// 	eOStepID_PEMVALUE_REV,
// 	eOStepID_COUNT
// };



class CPInsp_OCR : public CPInsp
{
public:
	CPInsp_OCR(void);
	virtual ~CPInsp_OCR(void);

	CProcMil_OCR* m_procMil;
	bool m_bstringMulti;
	int m_nOCRSimilarityScore;
	int m_nHomogeneityScore;
	int m_nrotateInspScore;
	bool m_OCRNGParamSave;
	int consecute_tmp;
	bool m_bImproveOCRVer;

	double m_dCurrentPartAngle;
private:

	int m_mainLightIndex;
	void* m_imgBuf[eImg_BufCnt];
	//InspImgBuf m_imgBuf;
	void* m_fovImage_insp;
	void* m_fovImage_insp_Mix[2];
	void* m_fovImage_teach;
	Coordinate m_inspCoordinate;
	Coordinate m_teachCoordinate;

	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;

	int m_inspCount;

	UCHAR* m_pInspImg;
	UCHAR* m_pInspImg_rotate;
	//UCHAR* m_pWndImg;

	//LJH 2016.05.20 OCR 검사시 영상 취합할 버퍼 두개
	//UCHAR* m_pWndImg_Mix[2];
	UCHAR* m_pInspImg_Mix[2];
	UCHAR* m_pInspImg_Mix_rotate[2];

	int m_roiSizeX;
	int m_roiSizeY;

	int m_pemtoThreshold[3];

	BOOL m_bAllocFovImg;
	int m_nstringIndex;

public:
	int m_nUserValueinsp_in;
	int m_nAutoValueInsp_in;
	int m_nPemtoValueInsp_in;
	int m_nPemToValueInsp_ImageMulti_in;
	int m_nPemToValueInsp_ImageMix_in;
	int m_nThresholdTracking_in;

	int m_nUserValueinsp;
	int m_nAutoValueInsp;
	int m_nPemtoValueInsp;
	int m_nPemToValueInsp_ImageMulti;
	int m_nPemToValueInsp_ImageMulti_auto;
	int m_nPemToValueInsp_ImageMix;
	int m_nThresholdTracking;

#if Timecheck
	double Singleword_Insp;
	double 	StepID_IMGBUF_ALLOC;
	double  StepID_USERVALUE;
	double	StepID_USERVALUE_REV;		
	double	StepID_AUTO;
	double	StepID_PEMVALUE;	
	double	StepID_AUTO_REV;
	double	StepID_PEMVALUE_REV;
	double	StepID_THRESHOLD_TRACKING;			//2016.12.01 shkim threshold tracking Insp
	double	StepID_THRESHOLD_TRACKING_REV;
	double	StepID_MIX_PEMVALUE;		//LJH 2016.05.27	영상 합친 후 검사
	double	StepID_MIX_PEMVALUE_REV;	//LJH 2016.05.27	영상 합친 후 검사 (영상 회전)
	int dInspROIImg;
	int dProcAuto;
	int dReInspRectimg;
	int dReInspLowScore;
	int dReInspLowSimilarity;
#endif
private:
	InspParamOCR m_paramInspect;
	TeachParamOCR m_paramTeach;
	AlgoOCR *m_pInspAlgoOCR;

public:
	InspParamOCR* m_pInspParam;
private:
	TeachParamOCR* m_pTeachParam;

	RstInspOCR m_result;

	OCRResultTemp m_resultTemp;
	bool m_oldResultFlag;

	long m_nFontSizeY; // Load된 Font 사이즈 길이
	int m_nThickenCnt;	// 인식 실패시 M_THICKEN_CHAR 확장 픽셀(0,1,2)

	int m_nInspectionMode;
	bool m_bIsHisto;
	bool m_bReRotate;
	bool m_bUseImgBufAlloc;
public:
	COpenCV_Functions* m_Opencv_Functions;
	int CharSimscore[36];

	bool bIsPilBuff;

public:
	int SetInspParam(void* itemParam, void* targetImg, Coordinate cdn);
	int SetInspParam(void* itemParam, Coordinate cdn, InspImgBuf img, int mainImgIndex);
	int SetInspParam(const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo);
	int SetTeachParam(void* itemParam, void* targetImg, Coordinate cdn, int teachType = -1);
	int SetTeachParam(const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo);
	

	int SetFont(BOOL isNew, int* retThresh = NULL,int fontCount = 0);
	int SaveFont(CString path = _T(""));
	int LoadFont(CString path, int* retFontCount, long* retFontSize);
	int GetTeachResultImg(UCHAR* Img);
	int GetSerachResultImg(UCHAR* Img);

	int ClearModelList(int nLane);
	int LoadModelList(CString path);
	int CheckModelList(CString path);
	void RemoveModel(CString sFontPath);

	int InspProc_OCR(RstAlgoOCR * sRstAlgo, int nInspectionMode);
	int InspProc_OCR2(RstAlgoOCR * sRstAlgo, int nInspectionMode);
	int ProcAuto();
	int ProcAuto2();
	int ProcStep(int stepID);
	int ProcStep2(int stepID);

	void GetLightArray(int mainLight, int* retArr, int* retCount);

	int ImgBuffAlloc();
	int ImgBuffAlloc2();
	int UserValueInsp(BOOL reverse);
	int UserValueInsp2(BOOL reverse);
	int AutoValueInsp(BOOL reverse);
	int RESEGValueInsp(BOOL reverse);
	int LocalValueInsp(BOOL reverse);
	int PemtoValueInsp(BOOL reverse);
	int PemToValueInsp_ImageMix(BOOL reverse);									//LJH 2016.05.27
	int PemToValueInsp_ImageMulti(BOOL reverse, BOOL IsAutoThreshold = FALSE);	//LJH 2016.06.09, LJH 2016.06.30 수정
	int ThresholdTracking(BOOL reverse);
	
	//huj 2013/10/04
	int GetFontInfo(CString fontPath, CString tempPath, int imgSizeX, int imgSizeY, int* retFontCount, CString OKFont,int color);
	int DeleteFont(int fontIndex, CString fontPath);
	int DeleteFont(int fontIndex);

	BOOL IsAllocFovImg() const { return m_bAllocFovImg; }
	void SetAllocFovImg(BOOL bAllocFovImg) { m_bAllocFovImg = bAllocFovImg; }

	int ReadStringOCR(CString& sReadOCR, CString sPathOCR);
	CString ReadStringOCR(UCHAR* ptrFontImg, int nSizeX, int nSizeY);
	int CalcBlob(UCHAR* pUcArrFontImg, int nSizeX, int nSizeY, int nFilterMor=-1);
	int AddFontIndex(Im::PIL_ID milContext);
#pragma region _BASE FUNCTION_
public:
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	int CloseDevice();

	int GetBuffDrawFont(UCHAR* dst, int sizeX, int sizeY, double angle, int color, int fontIndex = 0);
	int GetBuffDrawDefineFont(UCHAR* dst, int sizeX, int sizeY, double angle, int color,int fontIndex, CString OKFont);
	void InitStrResultStruct(RstInspOCR* ret);
	void InitStrResultTempStruct();
	void GetInspRst(RstInspOCR* ret);
	void DecisionOcrInsp(int ret = e_OK);
private:
	int GetCharType(wchar_t ch);

	int SetStrNewFont(UCHAR* srcImg, int sizeX, int sizeY, double angle, int fontColor, CString text, int spaceWidthMode, int rmLineCnt, int* rmLinePos, int* rmLineWid, int threshMode, int threshVal , int* retThresh = NULL);
	int SetStrFont(UCHAR* srcImg, int sizeX, int sizeY, double angle, int fontColor, CString text,int fontCount, int spaceWidthMode, int rmLineCnt, int* rmLinePos, int* rmLineWid, int threshMode, int threshVal, int* retThresh = NULL);

	int SetString(CString text, int foreGround, int charMinCount, int charMaxCount, BOOL mode, int threshMode, int threshVal = -1);
	int SearchString(void* targetImg, int cX, int cY, int sizeX, int sizeY, double angle, int foreGround);
	int SearchString(UCHAR* targetImg, int sizeX, int sizeY);

	//huj 2014/03/04 속도개선..SetString 함수를 두개로 나눔(Preprocess, SetThresholdMode)
	int AllocString();
	int Preprocess(int forground = 2);
	int SetThresholdMode(int threshMode, int threshVal = -1);
	int UnPreprocess();

	int GetBuffDrawString(UCHAR* dst, int cx, int cy, int srcWidth, int srcHeight, int dstWidth, int dstHeight, double angle, int color);
	int GetBuffDrawBox(UCHAR* dst, int sizeX, int sizeY, double angle, int color);

	int SaveStrFont(CString path);
	int LoadStrFont(CString path, int* retFontCount, long* retFontSize = NULL, int nInspectionMode = eMI_WindowTeaching);

	int Decision(CString str, BOOL* ok);

	double CalcAngle(double fontAngle, double wndAngle, BOOL isReverse);

	void DeleteImgBuf();

	// LMJ 2013/11/07
	void SetDrawLine(UCHAR* img, int sizeX, int sizeY, int lineCnt, int * linePos, int * lineWidth, bool bWhite);

	bool CheckScore(BOOL useCharScore, double stdScore, int charCount, BOOL bUseFontPass, int nFontPassCnt);
	bool CheckScore(BOOL useCharScore, double stdScore, int charCount, BOOL bUseFontPass, int nFontPassCnt, int index);

	//LJH
	//void Binarize(const UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nTypeRange, int nThreshLow, int nThreshHigh, BOOL bInvert, UCHAR* &pByImgDst);
	int Binarize(const UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nTypeRange, int nThreshold, BOOL bInvert, UCHAR* &pByImgDst);
	int BinarizeImageReverse(const UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, UCHAR* &pByImgDst);

	//shkim 2017.02.09
	int OKNGConfirm(int strRstTemp,int searchThresold = 130);
	int InspCropImg(UCHAR* img);
	int InspROIImg();
	int ReInspRectimg();
	int ReInspLowScore();
	int ReInspLowSimilarity(RstAlgoOCR * sRstAlgo);
	void saveResultTemp(int index = 0);

	void SaveParam();
	bool CharSimilarityConform(int charCount, int index);

public:
	void SetHist(bool bIsHist);
	void GetAllposition();
	int TeachingConSecutiveSpace(RstAlgoOCR * sRstAlgo, int nInspectionMode);
	int LoadParam();
#pragma endregion _BASE FUNCTION_

};

