#pragma once
//MIL 

//#include "../PInspAlgo/OCVParamDef.h"
#include "PsrFileManager.h"
#include "ModelFile_OCV.h"
#include "ProcMil.h"
#include "MModelManager.h"


class CProcPil_POCR : public CProcMil
{
public:
	CProcPil_POCR(void);
	virtual ~CProcPil_POCR(void);

private:

	// 	MIL_ID m_milBlobFeature;	// YJS 2016/11/04 : Blob관련 오류 창 원인 제거
	// 	MIL_ID m_milBlobResult;


	std::shared_ptr<ModelFile_POCR> m_ModelFile;
	std::shared_ptr<CMModelManager<ModelFile_POCR>> m_ModelMng;

	cPOCRInfoParam m_POCRInfoParam[POCRFont::Num];
	void CreatePOCRInfo();

	POCRInspResult *m_rstPOCRInspResult;

public:
	int m_nUsecontinue;
	CString targetFont;
public:
	int ImgWidth;
	int ImgHeight;
	//mil init , free
	AlgoPOCR m_AlgoPOCR;

	std::shared_ptr<PsrModel>  FontModel;
	RstAlgoPOCR POCRresult;
	std::vector<RstAlgoPOCR> Cand_POCRresult;

	std::vector<std::vector<std::shared_ptr<rstPSRModel>>> arr;
public:
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int FreeMil();


	std::shared_ptr<ImgProcessing_POCR> ImgProc;
	
//#ifdef _DEBUG
	cv::Mat orgImage;

//#endif
	//string reader
// private:
// 	template<typename T>
//	int StrInquire(int index, int type, T * controlValue);
//	int StrControl(int index, int type, double controlValue);
//	int StrEditFont(int index, int operation, int operationMode, MIL_INT param1, const void *param2);

public:
	void Setstr(CString str);
	int FontRect(cv::Mat img,cv::Mat Grayimg,cv::Mat BinImage,int color,int threshValue, RECT* TeachRect,int nLength, std::vector<std::vector<cv::Point2f>> vPoint);
	int FontRectAuto(cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue, RECT* TeachRect, int nLength);
	bool AddFont(cv::Mat img,cv::Mat Grayimg,cv::Mat BinImage,int color,int threshValue, RECT TeachRect, RECT* rcFontRect,HWND Handle);
	bool AddFont_NG(cv::Mat img,cv::Mat Grayimg,cv::Mat BinImage,int color, CString ImgPath,int nThreshold, RECT TeachRect);
	bool SaveFontList(CString sPath,bool bAddMode);

	bool LoadFontList(CString sPath,int fontColor , CString readfonts = nullptr, CString destPath = nullptr,bool bOrg = false);

	bool DeleteFont(CString ch,int index);
	bool ClearFont();
	bool SearchPOCR(AlgoPOCR *InspAlgoPOCR);
//	bool Confirm(std::vector<std::shared_ptr<rstPSRModel>> rst,AlgoPOCR *InspAlgoPOCR);
	bool Decision(int fontIndex,AlgoPOCR *InspAlgoPOCR,std::shared_ptr<rstPSRModel> font);
	bool Decision(AlgoPOCR *InspAlgoPOCR); // shkim 2022.09.28 new Decision only 
	bool SingleDecision();
	bool Division_fail();
	bool Confirm(AlgoPOCR *InspAlgoPOCR);

	bool SetPOCRData(POCRInfoParam *pPOCRInfoParam);

	bool POCRInspTeach(POCRInspParam &sPOCRInspParam, int nImageCnt, CString ModePath, POCRFontFile *ImageFiles, CString ModelFont, int PsrListIndex);
	bool POCRGetInspResult(POCRInspResult *rstPOCRInspResult);

	int ModelList_Load(CString strPath);
	int ModelList_Check(CString strPath);
	int ModelList_Clear(int nLane);


	int ModelLoad(CString strPath);
	int ModelFileLoad(CString strPath);
	bool SearchPOCRDraw(UCHAR* userSrc,int width,int height,AlgoPOCR *InspAlgoPOCR,UCHAR * ucArrDstImgMask, int ModelIndex);
	bool SearchPOCRDraw3Ch(UCHAR* userSrc, int width, int height, AlgoPOCR *InspAlgoPOCR, UCHAR * ucArrDstImgMask, int ModelIndex);
	int POCRInitResultVal(int nImageCnt);

	void SetImgProc(UCHAR* userSrc,int width,int height,std::shared_ptr<ImgProcessing_POCR> ProcImg,AlgoPOCR *InspAlgoPOCR, bool rotate = false);
	int ModelCnt_Check(CString strPath);

public:
	//Ext MultiProcess
	void SetExtPOCRFontInfo();
	void ClearExtPOCRFontInfo();
//	int AddNewFont();   //Add a new empty user defined font to the context.
// 	int DeleteFont(int fontIndex = 0);
// 	int SetEditFont(UCHAR* srcImg, int sizeX, int sizeY, double angle, int foreGround, CString font, int fontIndex, 
// 		int rmLineCnt, int* rmLinePos, int* rmLineWid, int threshMode, int threshVal, int* retThresh = NULL);
// 	int EditFont(MIL_ID srcImg, CString font, int fontIndex, int foreGround);
// 
// 	bool IsPreprocess();
// 	void Preprocess();
// 	void UnPreprocess();
// 
// 	void MakeImg(void* srcT_R, void* srcT_G, void* srcT_B, void* srcM_R, void* srcM_B, void* srcB_R, void* srcB_B);
// 	
// 	int ModelList_Load(CString strPath);
// 	int ModelList_Check(CString strPath);
// 	int ModelLoad(CString strPath);
// 	int ModelList_Clear(int nLane);
// 	void ModelRemove(CString strPath);
// 	void SetFontText(CString sFontText);
// 	CString GetFontText();
// 
// 	int GetCharPos(double* x, double* y,double* width, double* height,double* angle,int index,bool m_bReRotate = false);
// 	int GetCharSIMILARITY(double* score, int index);
// 	double GetFontWidth();
// 	void SetImgSize(int width, int height);

};

