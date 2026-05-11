#pragma once

//#include <opencv/cxcore.h>
#include <math.h>
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"

#include "OCVParamDef.h"
#include "GeoMatch_OCV.h"
#include "GeoMatch_POCR_v2.h"
#include "ExtInspLogic.h"
#if _DEBUG
	#define ImageSave 1
#else
	#define ImageSave 0
#endif

	struct rstPSRModel
	{
		double x;
		double y;
		double angle;

		char ch;

		double score;
		double div_score[MAX_POCR_DIVISITION_CNTS];

		int nCharWidth;
		int nCharHeight;

		double Parent_x;
		double Parent_y;
		double Parent_angle;
		int Parent_Width;
		int Parent_Height;

	double dContrastScore;
	double dUPScore;
	double dOPScore;
	double dBlobSizeValue;

		rstPSRModel()
		{
			x = 0;
			y = 0;
			angle = 0;
			score = 0;
			nCharWidth = 1;
			nCharHeight = 1;
			memset(div_score, 0, sizeof(double) * MAX_POCR_DIVISITION_CNTS);

			Parent_x = 0;
			Parent_y = 0;
			Parent_angle = 0;
			Parent_Width = 1;
			Parent_Height = 1;
		dContrastScore = 0.;
		dUPScore = 0.;
		dOPScore = 0.;
		dBlobSizeValue = 0;
		}

		void reset()
		{
			x = 0;
			y = 0;
			angle = 0;
			score = 0;
			nCharWidth = 1;
			nCharHeight = 1;

			Parent_x = 0;
			Parent_y = 0;
			Parent_angle = 0;
			Parent_Width = 1;
			Parent_Height = 1;
		dContrastScore = 0.;
		dUPScore = 0.;
		dOPScore = 0.;
		dBlobSizeValue = 0;
	}
};
struct algoPOCR_ParamContainer
{
	bool bUseContrast;
	bool bUseUP;
	bool bUseOP;
	bool bUseBlobSize;
	double dContrastTeachScore;
	double dUPTeachScore;
	double dOPTeachScore;
	int nBlobSizeTeachMaxCnt;
	double dBlobSizeTeachValue;
	int nPixelFilter;
	int fontArea;
	bool bImageLog;
	int m_nUseFontAngle;
	float m_fStdFontAngle;
	algoPOCR_ParamContainer()
	{
		bUseContrast = false;
		bUseUP = false;
		bUseOP = false;
		bUseBlobSize = false;
		dContrastTeachScore = 0;
		dUPTeachScore = 0;
		dOPTeachScore = 0;
		nBlobSizeTeachMaxCnt = 0;
		dBlobSizeTeachValue = 0;
		nPixelFilter = 3;
		fontArea = 0;
		bImageLog = false;
		m_nUseFontAngle = 0;
		m_fStdFontAngle = 6.f;
	}
	algoPOCR_ParamContainer(AlgoPOCR *algo)
	{
		bUseContrast = algo->m_bUseContrast;
		bUseUP = algo->m_bUseUP;
		bUseOP = algo->m_bUseOP;
		bUseBlobSize = algo->m_bUseBlobSize;
		dContrastTeachScore = algo->m_dContrast;
		dUPTeachScore = algo->m_dUP;
		dOPTeachScore = algo->m_dOP;
		nBlobSizeTeachMaxCnt = algo->m_nBlobSizeMaxCnt;
		dBlobSizeTeachValue = algo->m_dBlobSize;
		nPixelFilter = algo->m_nPixelFilter;
		fontArea = 0;
		bImageLog = algo->m_bImageLog == TRUE;
		m_nUseFontAngle = algo->m_bUseFontAngle == true ? 1 : 0;
		m_fStdFontAngle = algo->m_fStdFontAngle;
		}
	};

	struct clipData
	{
		int _ForVal;
		int _backVal;

		int* center_x;
		int* center_y;

		int* width;
		int* height;

		int nRectLength;

		cv::Mat _img;

		clipData()
		{
			_ForVal = 0;
			_backVal = 0;

			center_x = NULL;
			center_y = NULL;

			width = NULL;
			height = NULL;
			nRectLength = 0;
		}
		~clipData()
		{
			reset();
		}

		void reset()
		{
			_ForVal = 0;
			_backVal = 0;

			if (center_x != NULL) {
			//delete [] center_x;
			g_pMManager->pem_delete(center_x, true);
				center_x = NULL;
			}
			if (center_y != NULL) {
			//delete [] center_y;
			g_pMManager->pem_delete(center_y, true);
				center_y = NULL;
			}

			if (width != NULL) {
			//delete [] width;
			g_pMManager->pem_delete(width, true);
				width = NULL;
			}
			if (height != NULL) {
			//delete [] height;
			g_pMManager->pem_delete(height, true);
				height = NULL;
			}
		}
	};

	class CharClipper
	{
	public:
		std::vector<cv::Mat> MatList;
		std::vector<cv::Mat> MatOrgImgList;
		cv::Mat _Matrix;

		clipData* sClipData;

	public:
		bool searchArea(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue);
	bool searchArea(int strLength,cv::Mat src,cv::Mat Grayimg,cv::Mat BinImage,int color,int threshValue, RECT* FontRect,Im::PIL_ID* milApp, Im::PIL_ID* milSys);
		bool searchArea_NG(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, cv::Mat Model, RstAlgoPOCR result);
		bool searchArea_OrgImage(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue, std::vector<std::vector<cv::Point2f>> vPoint);
		bool searchArea_OrgImage_Old(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue, std::vector<std::vector<cv::Point2f>> vPoint);
		bool searchArea_OrgImage_AutoTeach(int strLength, cv::Mat src, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue);
		bool CalcAvgForBack(cv::Mat bin, cv::Mat Gray, cv::Mat& tmp);
		bool CalcAvgForBack(cv::Mat bin, cv::Mat Gray, cv::Mat& tmp, int threshold, int& backg);
		int GetRectArray(int strLength, RECT* ReArr);

		CharClipper()
		{
			MatList.clear();
			MatOrgImgList.clear();
		//sClipData = new clipData();
		sClipData = g_pMManager->pem_new<clipData>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		}
		~CharClipper()
		{
			if (sClipData != NULL)
			{
			g_pMManager->pem_delete(sClipData,false);
				sClipData = NULL;
			}
		}
	};

	typedef enum _ENM_GrayType { GtForeDard = 0, GtForeBright }enmGrayType;


	class sFontList;
	class PsrModel;

	class sFont
	{
	public:
		sFont(sFontList * pParent);
		virtual ~sFont();
	public:
		bool Alloc(TCHAR ch, cv::Mat img, cv::Mat Grayimg, int color);
		bool SaveToFile(CArchive & ar, double resX, double resY);
	bool ConvertFileVersion();
		bool LoadFromFile(CArchive & ar, double resX, double resY, CString path = nullptr, int fontColor = 1);
		void SaveFontImg(CString Path, int index, CString destPath = nullptr, bool bOrg = false);

		bool dataClear();
	std::vector<std::shared_ptr<rstPSRModel>> SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc, cPOCRInfoParam* vPOCRInspPtr, int cnt = 1, algoPOCR_ParamContainer* param = NULL);
	std::vector<std::shared_ptr<rstPSRModel>> SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc, cPOCRInfoParam* vPOCRInspPtr, cv::Point2f OfsChar, int cnt = 1, algoPOCR_ParamContainer* param = NULL);
	std::vector<std::shared_ptr<rstPSRModel>> SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc, cPOCRInfoParam* vPOCRInspPtr, std::shared_ptr<GeoModel_POCR> StrModel, GeoResult & StrRes,std::shared_ptr<GeoMatch_POCR> ptrEdge, algoPOCR_ParamContainer* param = NULL);
	std::vector<std::shared_ptr<rstPSRModel>> SearchFontSingle(cv::Mat srcimg, POCRInspParam& vPOCRInspPtr, algoPOCR_ParamContainer* param = NULL);
		bool Preprocess();
	bool RePreprocess();

		int getDataCount();
		cv::Mat CalcAvgForBack(cv::Mat bin, cv::Mat Gray);
	cv::Mat CalcAvgMaskImg(cv::Mat bin,cv::Mat Gray);

		void ThrowFontImg(int fListIndex, int fontIndex, CString destPath);
	void CalcShapeDiff(algoPOCR_ParamContainer* param, cv::Mat &model, cv::Mat &src, double nThreshold, bool isBin, double &contrast, double &up, double &op, double &blobsize);
	void CheckPOCRModelSize(GeoModel & model, std::vector<int> angleStepId);
	void(GeoMatch_POCR::*FunPtr_FindModel_Char_Angle[2])(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoModel * StringModel, GeoResult * StringResult, cv::Point2f OfsChar, GeoResult_POCR & result);
	public:
		TCHAR _ch;
		cv::Mat _Img;
		int _color;
		int _backVal;
		int _ForVal;
		int _center_x;
		int _center_y;
		int _width;
		int _height;
		int _Threshold;
	int _area;
		std::shared_ptr<GeoModel_POCR> _ModelEdge;
		std::shared_ptr<GeoModel_POCR> _ModelDiff;
	std::shared_ptr<GeoModel_PntsPOCR> _ModelPnts;		// jskim 2021.06.23		For POCR ver2

	public:
		sFontList* _pParent;
		bool _CheckThinFont(TCHAR ch);

	protected:
	void SaveHeader(CArchive & ar, double resX, double resY);
		void SaveModelEdge(CArchive & ar, double resX, double resY);
		void SaveModelDiff(CArchive & ar, double resX, double resY);

	void LoadHeader(CArchive & ar, double resX, double resY);
		void LoadModelEdge(CArchive & ar, double resX, double resY);
		void LoadModelDiff(CArchive & ar, double resX, double resY);

		POCRFont _GetFontEnum(TCHAR ch);
	};


	class sFontList
	{
	public:
		sFontList(PsrModel * pParent);
		virtual ~sFontList();
	public:
		bool AddFont(CString sStr, std::vector<cv::Mat> img, std::vector<cv::Mat> Grayimg, int color, clipData* sClipData, int nThreshold, RECT TeachRect);
	bool AddFont_ReTeaching(CString sStr, std::vector<cv::Mat> img, std::vector<cv::Mat> Grayimg, int color, clipData* sClipData, int nThreshold, RECT TeachRect, std::vector<std::shared_ptr<sFont>> vFontList);
		int getDataCount();
		bool SaveFile(CArchive &ar, bool bAddMode, CString path, int index, double resX, double resY);

		bool LoadFileReadFont(CArchive &ar, CString path, int index, CString readfonts, CString destPath, double resX, double resY, bool bOrg = false);	//shkim / POCREdit창에서사용.
		bool LoadFile(CArchive &ar, CString path, int index, double resX, double resY, int fontColor = 1);	//shkim / Teaching창에서사용.
		bool LoadFile(CArchive &ar, CString path, double resX, double resY);		//shkim //ModelFile_POCR.cpp에서 LoadFile시사용.
		bool dataClear();
		bool DeleteFont(CString ch);
	std::vector<std::shared_ptr<rstPSRModel>> SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc,CString TargetFont, cPOCRInfoParam* vPOCRInspPtr,std::shared_ptr<GeoMatch_POCR> ptrEdge,CString sSaveParam, int nForeGround, algoPOCR_ParamContainer* param = NULL);
		bool Preprocess();
	bool RePreprocess();
		bool SaveImageList(clipData* sClipData);
		void SearchFontDraw(cv::Mat src, bool rot, UCHAR * ucArrDstImgMask);
		void SearchFontDraw3Ch(cv::Mat src, bool rot, UCHAR * ucArrDstImgMask);
		void SaveImageList(CString sPath, std::vector<uchar>& vBuf);
	void ConvertFileVersion();
	void CheckPOCRModelSize(GeoModel & model, std::vector<int> angleStepId);

		CString _chfont;
		std::vector<std::shared_ptr<sFont>> _sfont;
		PsrModel * _pParent;
		std::shared_ptr<GeoModel_POCR> _OrgImage;
		cv::Point _teachPo;
	};

	class PsrModel
	{
	public:
		PsrModel(void);
		virtual ~PsrModel(void);
	private:
		bool Preprocessed;
	public:
	Im::PIL_ID* m_milApp;
	Im::PIL_ID* m_milSys;
		int nfontListCnt;
		std::vector<std::shared_ptr<sFontList>> _sfList;
		std::vector<std::shared_ptr<rstPSRModel>> rst;

		double m_dStdAddFontScore[256];

		bool AddFont(CString sStr, cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int thresh, int listIndex, cPOCRInfoParam* vPOCRInspPtr, RECT TeachRect, RECT* FontRect, HWND Handle);
		bool AddFont_NG(cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int listIndex, cPOCRInfoParam* vPOCRInspPtr, CString ImgPath, int nThreshold, RECT TeachRect);
	bool AddFont_ReTeaching(CString sStr, cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int thresh, int listIndex, cPOCRInfoParam* vPOCRInspPtr, RECT TeachRect, RECT* FontRect); 
		int dataCount();
		bool dataClear();
	bool SetPilParam(Im::PIL_ID* milApp, Im::PIL_ID* milSys);

		bool Save(CString sPath, bool bAddMode, double resX, double resY);
		bool Load(CString sPath, int fontColor, double resX, double resY, CString readfonts = nullptr, CString destPath = nullptr, bool bOrg = false);	//shkim / Teaching창에서사용.
		bool Load(CString sPath, double resX, double resY);		//shkim //ModelFile_POCR.cpp에서 LoadFile시사용.
		bool ExtLoad(int nIndex, double resX, double resY);
		bool deleteFont(CString ch, int index);
		bool deleteFontList(int index);

		bool SaveHeader(CArchive &ar);
		bool LoadHeader(CArchive &ar);
		bool Preprocess();
	bool RePreprocess();
		bool isPreprocess();

		//	bool SearchFont();
	std::vector<std::shared_ptr<rstPSRModel>> SearchFont(std::shared_ptr<ImgProcessing_POCR> ImgProc,CString TargetFont, cPOCRInfoParam* vPOCRInspPtr,CString sSaveParam, double* dStdScore, int nForeGround, algoPOCR_ParamContainer* param = NULL);
	std::vector<std::shared_ptr<rstPSRModel>> MatchingFont(std::shared_ptr<ImgProcessing_POCR> ImgProc, CString Targetfont, cPOCRInfoParam* vPOCRInspPtr, CString sSaveParam, double* dStdScore, int nForeGround, algoPOCR_ParamContainer* param = NULL);
	std::vector<std::shared_ptr<rstPSRModel>> SearchSingleFont(cv::Mat srcImg, CString ModelFont, int PsrListIndex, POCRInspParam& vPOCRInspPtr, algoPOCR_ParamContainer* param = NULL);
		void SearchFontDraw(cv::Mat srcImg, bool rot, UCHAR * ucArrDstImgMask, int ModelIndex);
		void SearchFontDraw3Ch(cv::Mat srcImg, bool rot, UCHAR * ucArrDstImgMask, int ModelIndex);
		CString GetModelStr(int idx);
	bool ConvertFileVersion();
	};


