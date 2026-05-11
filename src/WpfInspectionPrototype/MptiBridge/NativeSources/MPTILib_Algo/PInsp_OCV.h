#pragma once

#include "pinsp.h"
#include "ProcPil_OCV.h"
//#include "MPTI.h"

#define FONTSIZE_S 15
#define FONTSIZE_M 25

#define Timecheck 0
#define AI_ON 0


typedef struct tagPOCRResultTemp
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
}POCRResultTemp;


class CPInsp_POCR : public CPInsp
{
public:
	CPInsp_POCR(void);
	virtual ~CPInsp_POCR(void);
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);

	CProcPil_POCR* m_procPil;
	int m_nrotateInspScore;
	int consecute_tmp;

	double m_dCurrentPartAngle;
private:
	int m_mainLightIndex;
	Coordinate m_inspCoordinate;

	int m_fovWidth;
	int m_fovLength;
	double m_resolX;
	double m_resolY;

	int m_inspCount;

	int m_roiSizeX;
	int m_roiSizeY;

	int m_nstringIndex;

	bool UsePOCRTxt; 
	bool AISampleImageSaveNG;
	bool AISampleImageSaveOK;

#if AI_ON
	std::shared_ptr<csml::CRAFT_AI_AutoModule> m_CRAFT_AI;
	std::vector<std::vector<cv::Point2f>> m_CRAFT_AI_Result;
#endif
	
public:
	bool AIInputIamgeSave;
	int nAIOKSaveCount;
	int nAIMaxSaveCount;
private:
	AlgoPOCR *m_pInspAlgoPOCR;

private:
	RstAlgoPOCR m_result;

	POCRResultTemp m_resultTemp;
	
public:
	//COpenCV_Functions* m_Opencv_Functions;
	int m_nCurJob;
	CString m_sBoard;
	CString m_sPartCode;
	CString m_sRefID;
	CString m_sModulID;
	CString m_WndIdx;
	int m_AlgoIdx;
	
	std::shared_ptr<ImgProcessing_POCR> ImgProc;
	std::shared_ptr<ImgProcessing_POCR> ImgProc_ro;


	std::map<CString,RstAlgoPOCR> m_mInspRst[2];
	std::map<CString,CString> m_mSavePartList[2];
	std::map<CString,CString> m_mNGSavePartList[2];
	std::map<CString,CString> m_mInspWndNum[2];
	std::map<CString,CString> m_mInspAlgoNum[2];
	std::map<CString, RstAlgoPOCR> m_mInspParRst[2];
	std::map<CString, int> AIImageSavePartCnt;
	std::map<CString, std::pair<CString, std::shared_ptr<ImgProcessing_POCR>>> AIImageSaveList[2];
	std::map<CString, std::pair<CString, std::shared_ptr<ImgProcessing_POCR>>> AIImageSaveList_Forced[2];
	std::map<CString,std::shared_ptr<ImgProcessing_POCR>> AIOKImageList[2];
public:
//	int SetTeachParam(const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo);

//	bool AddFont();
	void Setstr(CString str);
	int SetRect(UCHAR* srcImg, UCHAR* GrayImg, UCHAR* BinOrg, int sizeX, int sizeY, int fontColor, int threshVal, RECT* TeachRect, int nLength);
	int SetRectAuto(UCHAR* srcImg, UCHAR* GrayImg, UCHAR* BinOrg, int sizeX, int sizeY, int fontColor, int threshVal, RECT* TeachRect, int nLength);
	int SetStrFont(UCHAR* srcImg,UCHAR* GrayImg,UCHAR* BinOrg, int sizeX, int sizeY, double angle, int fontColor,  int threshVal, RECT TeachRect, RECT* FontRect,HWND Handle);
	int SetStrFont_NGViewer(UCHAR* srcImg,UCHAR* GrayImg,UCHAR* BinOrg, int sizeX, int sizeY, double angle, int fontColor, CString ImgPath,int nThreshold, RECT TeachRect);
	int SetStrFont_ReTeaching(UCHAR* srcImg, UCHAR* GrayImg, UCHAR* BinOrg, int sizeX, int sizeY, double angle, int fontColor, int threshVal, RECT TeachRect, RECT* FontRect);
	int LoadPsrFile(CString path,int fontColor, CString readfonts, CString destPath,bool bOrg = false);
	int LoadPsrFile(CString path);
	int SavePsrFile(CString path,bool bAddMode = false);
	int DeletePsrFont(CString ch,int index);
	int ClearFont();
	int InspProc_POCR(UCHAR* userSrc,int width,int height, int nInspectionMode);
	int GetInspRst(RstAlgoPOCR* pDstAlgoPOCR);
	int SetInspParam( const InspAlgo &sInspAlgo ,double angle);
	int SetPOCRData(POCRInfoParam *pPOCRInfoParam);
	int POCRInspTeach(POCRInspParam &sPOCRInspParam, int nImageCnt, CString ModePath, POCRFontFile *ImageFiles, CString ModelFont, int PsrListIndex);
	int POCRGetInspResult(POCRInspResult *rstPOCRInspResult);
	int POCRGetTeachResImg(UCHAR* userSrc,int width,int height, UCHAR * ucArrDstImgMask, int ModelIndex);
	int POCRGetTeachResImg3Ch(UCHAR* userSrc, int width, int height, UCHAR * ucArrDstImgMask, int ModelIndex);

	int ClearModelList(int nLane);
	int LoadModelList(CString path);
	int CheckModelList(CString path);
	void SetImgProc(std::shared_ptr<ImgProcessing_POCR> ProcImg,std::shared_ptr<ImgProcessing_POCR> ProcImg_ro);
	int ReadStringOCR(CString& sReadOCR, CString sPathOCR);
	int CheckModelCnt(CString path);
	void SetUsePOCRTxt(bool UseTxt);

	//Ext MultiProcess
	void SetExtPOCRFontInfo();
#if AI_ON
	int ReadStringOCR(CString& sReadOCR, CString sPathOCR);
	int AIImageSave(POCRAISaveInfo PocrInfo);
	int SetAIImageParam(CString sBoard,CString sPartCodeNWndName, CString sRefId, CString sModuleID);
	int AItxtSave(CString sFullPath);
	double AItxtload(CString sFullPath, double* MinScore, double* MaxScore);
	void PartStruct_AI(CString moduleId, CString id, CString partCode, CString windowId, CString algoId, CString refId, int* arrChResult, int nAlgoCnt, int nCharCount, AUTO_INSP_AI_DATA* sInsp_POCR);
	void GetAI_Image(int nAlgoCnt, AUTO_INSP_AI_DATA* sInsp_POCR, int nOPT);
	void SetFontImgForAI(bool roFlag);
	void AIImageListSave(int nCurJob);
	void AIImageListSave_forced(int nCurJob);
	void AIOKImageSave(int nCurJob, CString sModulID, CString sPartCode, CString sRefID, bool bOnlyErase);
	void AIImageSave_Result(int nCurJob);
	int AiResultClear(int IdxJobCur);
	int AiResultSave(CString sKey, RstAlgoPOCR HeightRst, CString sPartKey);
	RstAlgoPOCR* AiResultLoad(CString sKey, double* MinScore, double* MaxScore);
	bool AiResultExist(CString ssKey, CString sPartKey);

	int AIImageSavePart(int nCurJob, CString sPartCode, CString sRefId, bool bOnlyNGSave);
	int AIImageSavePartListClear(int nCurJob);
	void CopyAIPosition(RstAlgoPOCR& Srcrst,RstAlgoPOCR& DstRst);

	void GetModelStr(int idx, SPOCRFontList* sStr);
#pragma endregion _BASE FUNCTION_
	bool ConvertFileVersion();
#endif
};

