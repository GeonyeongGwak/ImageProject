#pragma once
#include "Insp_Wnd.h"
#include "PInspData.h"
#include "GeoMatch.h"
#include "ModelManager_Geo.h"

#ifdef _DEBUG
#pragma comment(lib, "opencv_video440d.lib")
#else
#pragma comment(lib, "opencv_video440.lib")
#endif

namespace PIAL
{
	enum insp_type
	{
		eINSP_MOUNT = 0,
		eINSP_ALIGN,	// ++
		eINSP_OCR,
		//eINSP_SOLDER,
		//eINSP_LEAD,
		eINSP_LEADSOLDER,
		eINSP_SOLDER,		// ++
		eINSP_TAB,
		eINSP_S_BALL,
		eINSP_PAD,
		eINSP_USER,
		eINSP_BGA,
		eINSP_WND_CNT,
		eINSP_COLOR,		// X
		eINSP_PATTERN, /// X
	};

	enum eBlobInspMode
	{
		eBlobInspMode_Origin = 0,
		eBlobInspMode_Solderball,
		eBlobInspMode_Coil,
	};

	class PINSPALGO_API  PInspAlgo
	{
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

		std::shared_ptr<CMModelManager<tagModelFile>> m_ModelMng;
		std::shared_ptr<tagModelFile> m_ModelFile;
		std::shared_ptr<GeoModel> m_milPatModel;

		void *m_PemHL;
		bool bIsHalconExist = false;

		int	m_ShapeXCnt;
		POINTF* m_ShapeXArrPoints;
		int* m_ShapeXArrMatchIdx;
		float* m_ShapeXArrMatchSc;
		cv::Mat m_ShapeXCurrentMask16;  // CurrentShape 16bit 레이블 마스크 (CV_16UC1)
	private :
		void(*m_MultiAddLog)(CString);
	public:
		// CurrentShape 마스크 외부 접근 (InspManager 등에서 사용)
		cv::Mat GetShapeXCurrentMask16() const { return m_ShapeXCurrentMask16; }
#ifdef DEBUG
		cv::Mat IMG4Debug;
#endif
		PInspAlgo();
		~PInspAlgo();
		
	    bool InspBlob(_AlgoBlob &pAlgoBlob,const Insp_Image& pImg_buf, const Insp_ROIImg&sInspImageData, _RstAlgoBlob* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL);
	
	    bool TeachBGA(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, TeachBGA* sTeachBGA, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL);
		bool TeachBGA_New(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, TeachBGA2* sTeachBGA, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL);
		bool TeachBGA_New_Pad(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, TeachBGA2* sTeachBGA, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL);
		bool TeachLQBGA(_AlgoLQBGA &pAlgoBGA, const Insp_Image& pImg_buf, TeachBGA2* sTeachBGA, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL);
		bool DrawLQBGABinary(_AlgoLQBGA &pAlgoBGA, const Insp_Image& pImg_buf, PI_Buff *pImgBlob );

		bool DrawBGABinary(_AlgoBGA &pAlgoBGA, Insp_Image& pImg_buf, const Insp_ROIImg&sInspImageData, PI_Buff *pImgBlob = NULL);
		bool DrawBGABinary_FluxCoating(_AlgoBGA& pAlgoBGA, Insp_Image& pImg_buf, PI_Buff* pImgBlob = NULL);
		bool DrawNGBlob(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, RECT bodyRect, std::vector<PIAL::PInspData*> pDataSet);
		bool DrawNGBlob(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, RECT bodyRect, std::vector<PIAL::PInspData*> pDataSet, _RstAlgoNGBlob* sRstAlgo,bool bInBubble=false);
		bool DrawNGBlob_renewal(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, RECT bodyRect, std::vector<PIAL::PInspData*> pDataSet, RECT* arrBumpMask, RECT* arrBodyMask, int* nArrMask, int* nHisto = nullptr, std::vector<cv::Rect2f> vForeignMask = std::vector<cv::Rect2f>(), std::vector<float> vForeignAngle = std::vector<float>(), bool bCoplDraw = true);
		bool DrawNGBlob_Pad(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, RECT bodyRect, std::vector<PIAL::PInspData*> pDataSet, std::vector< PadDrawShape>& pads, int* nHisto = nullptr, int* nArrMask = nullptr, RECT* arrBumpMask = nullptr, RECT* arrBodyMask = nullptr);
		bool DrawScratch(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, RECT* arrBumpMask, RECT* arrBodyMask, int* nArrMask);
		bool DrawNGBlob_renewal_Blob(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, RECT bodyRect, std::vector<PIAL::PInspData*> pDataSet, RECT* arrBumpMask, RECT* arrBodyMask, int* nArrMask, int* nHisto = nullptr, std::vector<cv::Rect2f> vForeignMask = std::vector<cv::Rect2f>(), std::vector<float> vForeignAngle = std::vector<float>(), bool bInspPadOutside = false);
		bool Histogram_NGBlob(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _TotalInspExceptArea stTieArea, RECT bodyRect, std::vector<PIAL::PInspData*> pDataSet, RECT* arrBumpMask, RECT* arrBodyMask, int* nArrMask,  uchar* pHistoImage, int *nRetRangeMin, int *nRetRangeMax, int *nRetRangeAvg, int* nRetMaxFreqValue);
		bool HistogramPad_NGBlob(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _TotalInspExceptArea stTieArea, RECT bodyRect, std::vector<PIAL::PInspData*> pDataSet, std::vector< PadDrawShape>& pads, uchar* pHistoImage, int *nRetRangeMin, int *nRetRangeMax, int *nRetRangeAvg, int* nRetMaxFreqValue, int* nArrMaskCnt);

		void GetBodyRect(std::vector<PIAL::PInspData*> pDataSet, CRect& cBodyRect);
		bool GetBodyEdge(std::vector<PIAL::PInspData*> pDataSet, std::vector<POINTF>& BodyEdge, int nOffsetX =0 , int nOffsetY =0);

		// new BGA
	    bool InspLQBGA(_AlgoLQBGA &pAlgoBGA, Insp_Image& pImg_buf, _RstAlgoLQBGA* sRstAlgo, std::vector<_AlignResult>& vecAlignResult, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);



		// Bump
	    bool InspBGA(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		bool InspQFN(_AlgoQFN &pAlgoQFN, const Insp_Image& pImg_buf, _RstAlgoQFN* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL); 
		
		bool InspBump(_AlgoBump &pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, int nOffX_pix, int nOffY_pix, std::vector<_AlignResult>& vecAlignResult, PI_Buff *pImgBlob = NULL, PI_Buff* pMask = NULL, bool bUseDiameter = true);
		// Only HanaMicron - Width/Length Rate삭제
		bool InspBumpAVI(_AlgoBump& pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, int nOffX_pix, int nOffY_pix, std::vector<_AlignResult>& vecAlignResult, PI_Buff* pImgBlob = NULL, bool bUseDiameter = true);
		//Coining: Coinning 검사는 제외됨, FlatBall 검사로 대체
		bool InspBGA_Coining(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		bool InspBump_Coining(_AlgoBump &pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, int nOffX_pix, int nOffY_pix, std::vector<_AlignResult>& vecAlignResult, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		//Precoat: 검사 제외
 		bool InspBGA_Precoat(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, const Insp_ROIImg&sInspImageData, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		bool InspBump_Precoat(_AlgoBump &pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, int nOffX_pix, int nOffY_pix, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);

		bool InspBGA_FlatBall(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		bool InspBump_FlatBall(_AlgoBump &pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, int nOffX_pix, int nOffY_pix, std::vector<_AlignResult>& vecAlignResult, PI_Buff *pImgBlob = NULL);

		bool InspBGA_Blob(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		bool InspBump_Blob(_AlgoBump& pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, int nOffX_pix, int nOffY_pix, std::vector<_AlignResult>& vecAlignResult, PI_Buff* pImgBlob = NULL, bool bUseRemovePadVol = false,double dRemovePadVol=0.0, cv::Mat& BGAMask=cv::Mat());
		//Only HanaMicron - Width/Length Rate삭제
		bool InspBump_Blob_AVI(_AlgoBump& pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, int nOffX_pix, int nOffY_pix, std::vector<_AlignResult>& vecAlignResult, PI_Buff* pImgBlob = NULL);

		bool InspBGA_Tiny(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		bool InspBump_Tiny(_AlgoBump &pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, std::vector<_AlignResult>& vecAlignResult, PI_Buff *pImgBlob = NULL);

		bool InspBGA_Pad(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		
		//Only HanaMicron - Width/Length Rate삭제
		bool InspBump_Pad(_AlgoBump &pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, std::vector<_AlignResult>& vecAlignResult, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		
		bool InspBump_New_Pad(_AlgoBump& pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, std::vector<_AlignResult>& vecAlignResult, PI_Buff* pImgBlob = NULL, PI_Buff* pMask = NULL);

		bool InspBGA_FluxCoating(_AlgoBGA& pAlgoBGA, Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult, std::vector<BumpLocation>& vecBumpLocation, PI_Buff* pImgBlob = NULL, PI_Buff* pMask = NULL,cv::Mat& BGAMask=cv::Mat(),bool bUseZoneBasedArea=false);
		bool InspBump_FluxCoating(_AlgoBump& pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, std::vector<_AlignResult>& vecAlignResult, PI_Buff* pImgBlob = NULL, bool bUseRemovePadVol = false, double dRemovePadVol = 0.0, cv::Mat& BGAMask = cv::Mat(), bool bUseZoneBasedArea = false, const std::vector<double>& vecTeachArea = {});


		bool InspBGA_Flux(_AlgoBGA& pAlgoBGA, Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult, std::vector<BumpLocation>& vecBumpLocation, std::vector<CRect>& vecBridgeRect, PI_Buff* pImgBlob = NULL, PI_Buff* pMask = NULL, UCHAR* pBridgebox = nullptr);
		bool InspBump_Flux(_AlgoBump& pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, std::vector<_AlignResult>& vecAlignResult,std::vector<CRect>&vecBridgeRect, PI_Buff* pImgBlob = NULL, std::vector<int>* vnBridgeIdx=nullptr,bool bUseInspAreaHOffset=false,double dInspAreaHOffset=0.0,bool bUseRemovePadVol=false,double dRemovePadVol=0.0);
		bool WarpageCorrect3DByGaussianBlur(_AlgoBGA& pAlgoBGA, Insp_Image& pImg_buf, std::vector<BumpLocation>& vecBumpLocation, std::vector<_AlignResult>& vecAlignResult, float nWidth_mm, float nLength_mm);
		bool WarpageCorrect3DByROI(_AlgoBGA& pAlgoBGA, Insp_Image& pImg_buf, std::vector<_AlignResult>& vecAlignResult);
		bool DrawFluxBridge(_AlgoBGA& pAlgoBGA,  Insp_Image& pImg_buf, PI_Buff* PImgBlob = NULL, _TotalInspExceptArea stTieArea = _TotalInspExceptArea(), _RstAlgoBGA* sRstAlgo = nullptr, BOOL bInspMode = false, UCHAR* ucBridgeImg = nullptr);
		bool DrawAlignedBGAMask(_AlgoBGA& pAlgoBGA,CString sPath, Insp_Image& pImg_buf, PI_Buff* PImgBlob = NULL, _TotalInspExceptArea stTieArea = _TotalInspExceptArea(), _RstAlgoBGA* sRstAlgo = nullptr, BOOL bInspMode = false, UCHAR* ucImage = nullptr, double dAlignX = 0.0, double dAlignY = 0.0);
		void LoadBGAMask(CString sPath, Insp_Image& pImg_buf, double AlignX, double AlignY, float m_resolX, float m_resolY, cv::Mat& alignedMask, bool& bMaskLoad);
		bool SelfAlignBump(_AlgoBGA &pAlgoBGA,  Insp_Image& pImg_buf, std::vector<BumpLocation>& vecBumpLocation);
		void AlignModelToInspImg(cv::Mat& inspMat, cv::Mat& PadMask);
		//Only BGA Window
		bool InspExtraBump(_AlgoNGBlob &pAlgoNGblob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, PI_Buff *pImgBlob = NULL);

		bool InspPicker(_AlgoNGBlob &pAlgoNGblob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, PI_Buff *pImgBlob = NULL);
		//Picker Mean
		void LogWrite_Mean(CString sLog);

		//Only BGA Window
		bool InspNGBlob(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, PI_Buff *pImgBlob = NULL);

		bool InspNGBlob(_AlgoNGBlob &pAlgoNGBlob, Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult, std::vector<PIAL::PInspData*>& pDataSet, PI_Buff *pMask = NULL, bool bGrayMask = false, void* stModel = nullptr);

		bool InspNGBlob_Blob(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult, std::vector<PIAL::PInspData*>& pDataSet, PI_Buff *pMask = NULL, bool bGrayMask = false);

		bool InspNGBlob_Blob_Pad(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult, std::vector<PIAL::PInspData*>& pDataSet, PI_Buff *pMask = NULL, bool bGrayMask = false);

		//Only KOVIS OVEN Warpage
		bool InspNGBlob_KOVIS(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult, std::vector<PIAL::PInspData*>& pDataSet, PI_Buff *pMask = NULL);

		//Only WIR
		bool InspNGBlob_WIR(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult, std::vector<PIAL::PInspData*> pDataSet, PI_Buff *pMask, _tagNGBlob_AISaveParam stAISaveParam,bool bGrayMask = false , int KernelSize = 1, int DownSample = 1, void * stModel = nullptr);

		bool InspNGBlob_BallWarpage(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, std::vector<AlgoSetBump>& vecBumpSet);

		bool InspNGBlob_BallDamage(_AlgoNGBlob &pAlgoNGBlob, const Insp_Image& pImg_buf, _RstAlgoNGBlob* sRstAlgo, std::vector<AlgoSetBump>& vecBumpSet, PI_Buff *pBumpImage = NULL, std::vector<_AlignResult>*vecAlignResult = nullptr, std::vector<PIAL::PInspData*>* pDataSet = nullptr, _TotalInspExceptArea stTieArea = _TotalInspExceptArea());

		bool InspTilt(_AlgoTilt &pAlgoTilt, const Insp_Image& pWnd_buff, _RstAlgoTilt* sRstAlgo, std::vector<_AlignResult>& vecAlignResult, _TotalInspExceptArea stTieArea);

		bool InspBGASecond(Insp_Image& pImg_buf, AlgoSetBGA* pBGASet, std::vector<AlgoSetBump>& vecBumpSet,PI_Buff* pBumpBin = nullptr, std::vector<PIAL::PInspData*>* pDataSet = nullptr, std::vector<_AlignResult>* vecAlignResult = nullptr, int nUseAVI = 0);
		bool InspCoiningSecond(Insp_Image& pImg_buf, AlgoSetBGA* pBGASet, std::vector<AlgoSetBump>& vecBumpSet);
		bool SaveBGARawData(AlgoSetBGA* pBGASet, std::vector<AlgoSetBump>& vecBumpSet, std::wstring path);
		bool AVGNearBump(std::vector<AlgoSetBump>& vecBumpSet, int NumNearBump = 7,double dMainRate = 0.5);

		int InspColorStep(_AlgoColor &algoColor, const Insp_Image& pImg_buf, _RstAlgoColor* sRstColor, PI_Buff* Buf_TR, PI_Buff* Buf_TG, PI_Buff* Buf_TB, PI_Buff* Buf_BR, PI_Buff* Buf_BB, int stepID);

		bool InspBW(_AlgoBW* algoBW, _AlgoBlackWhite algoBW2, PI_Buff* org2DSrc, PI_Buff* org3DSrc, _TotalInspExceptArea stTieArea, _RstAlgoBW* sRstBW, POINTF ptfOffset, Insp_Image* pImg_buf, PI_Buff* DstImg = nullptr);

		bool InspGrayMean(_AlgoGrayMean &algoGrayMean,const Insp_Image& pImg_buf, _RstAlgoGrayMean *sRstAlgo, _TotalInspExceptArea stTieArea, byte byInspType = 0, int *nReviewMin = 0, int *nRevieMax = 0, PI_Buff* DstImg = NULL, bool bTeach = false);

		bool InspHeightMean(_AlgoHeightMean &algoHeightMean, const Insp_Image& pImg_buf, const Insp_Image& pImg_buf2, _TotalInspExceptArea stTieArea, _RstAlgoHeightMean* sRstHeightMean, PI_Buff* puImgDst = NULL, int nStartX = 0, int nStartY = 0);
	
		bool InspGrayDiff(_AlgoGrayDiff &algoGrayDiff, const Insp_Image& pImg_buf, _RstAlgoGrayDiff *sRstAlgo, _AlignResult* sAlignRes, int nAlignResCnt);

		bool InspHeightDiff(_AlgoHeightDiff &algoHeightDiff, const Insp_Image& PImg_buf, _RstAlgoHeightDiff *sRstAlgo, _AlignResult* sAlignRes, int nAlignResCnt);

		bool GetHistogramGraph(PI_Buff *pSrcBuff, uchar* pHistoImage, int nLimitMin, int nLimitMax, int *nRetRangeMin, int *nRetRangeMax, int *nRetRangeAvg, int* nRetMaxFreqValue);

		//PadBW
		bool InspPadBW(_AlgoPadBW& pAlgoPadBW, const Insp_Image& pWnd_buff, Insp_ROIImg& sInspImageData, _RstAlgoPadBW* sRstAlgo, int nBufSizeW, int nBufSizeH, UCHAR** ucCalcBuf, UCHAR** ucRstBuf );
		bool InspPadBW(_AlgoPadBW &pAlgoPadBW, Insp_Image& pImg_buf, _RstAlgoPadBW* sRstAlgo, _TotalInspExceptArea stTieArea, int nInspMode, cv::Mat& Mask, BOOL bUseMask = FALSE, PI_Buff* puImgDst = NULL, int nSelectedLightIdx = 0, double dAlignAngle = 0, UCHAR* ucMaskOverlayImage = NULL,PInspData* PadBWWindowMask = nullptr, bool bGray = false, UCHAR* ucMaskOverlayNCriticalImage = NULL);
		bool InspPadBWShape(_AlgoPadBW &pAlgoPadBW, const Insp_Image &sWndAlgoImg, Insp_ROIImg &sInspImageData, _RstAlgoPadBW * sRstAlgo, cv::Mat& mGerberImg, int dX, int dY, cv::Mat fullgerber, UCHAR ** ucRstBuf);
		int CreatePadBWImage(PI_Buff* ucGrayImg, int RangeType, int nThMin, int nThMax, _BlobLabelInfo* stBlobLabelinfo, CString sSavePath);
		int CreatePadBWImage_direct(PI_Buff* BinImg, int RangeType, int nThMin, int nThMax, _BlobLabelInfo* stBlobLabelinfo, CString sSavePath);
		int CreatePadBWImage_3D(PI_Buff* GrayImg, PI_Buff *pSrc3, _AlgoPadBW algoPadBW, _BlobLabelInfo* stBlobLabelinfo, CString sSavePath);
		int CreatePadBWImage_3D(PI_Buff* GrayImg, PI_Buff *pSrc3, _AlgoPadBW algoPadBW, _BlobLabelInfo* stBlobLabelinfo, CString sSavePath, Insp_Image& pImg_buf);
		bool DrawPadBW(_AlgoPadBW& pAlgoPadBW, Insp_Image& pImg_buf, PI_Buff* pImgBlob, _TotalInspExceptArea stTieArea, _RstAlgoPadBW* sRstAlgo, BOOL bInspMode, int nSelectedLightIdx, cv::Mat& AlgoPolyMask, double dAlignAngle = 0, UCHAR* ucMaskOverlayImage = NULL, bool bGray = false, UCHAR* ucMaskOverlayNCriticalImage = NULL);
		bool BinPadBW(_AlgoPadBW &pAlgoPadBW, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, _RstAlgoPadBW* sRstAlgo, BOOL bInspMode, int nSelectedLightIdx);
		bool GetPadBWHistogramGraph(_AlgoPadBW& pAlgoPadBW, const Insp_Image& pImg_buf, uchar* pHistoImage, cv::Mat& AlgoPolyMask, int nCurLayerNum, int dx, int dy, int* nRetRangeMin, int* nRetRangeMax, int* nRetRangeAvg, int* nRetMaxFreqValue, BOOL bInspMode = TRUE);
		bool GetPadBWHistogramGraph_Smooth(_AlgoPadBW& pAlgoPadBW, const Insp_Image& pImg_buf, uchar* pHistoImage, cv::Mat& AlgoPolyMask, int nCurLayerNum, int dx, int dy, int* nRetRangeMin, int* nRetRangeMax, int* nRetRangeAvg, int* nRetMaxFreqValue, BOOL bInspMode = TRUE, double dAlignRstTheta = 0.0);
		bool GetPadBWHistogramGraph_1D_2Q(_AlgoPadBW& pAlgoPadBW, const Insp_Image& pImg_buf, uchar* pHistoImage, cv::Mat& AlgoPolyMask, int nCurLayerNum, int dx, int dy, int* nRetRangeMin, int* nRetRangeMax, int* nRetRangeAvg, int* nRetMaxFreqValue, BOOL bInspMode = TRUE, double dAlignRstTheta = 0.0);
		bool SyncPadBWMask(CString sSavePath, int* nMaskIndices, int nCount);

		//BodyBlob
		bool BinalizeBodyBlob(_AlgoBodyBlob & algoBodyBlob, const Insp_Image& pImg_buf, _RstAlgoBodyBlob * sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff* puImgDst = nullptr);
		bool TeachBodyBlob(_AlgoBodyBlob & algoBodyBlob, const Insp_Image& pImg_buf, _RstAlgoBodyBlob * sRstAlgo, POINTF ptWndCenter, _TotalInspExceptArea stTieArea, PI_Buff* puImgDst = nullptr);
		bool InspBodyBlob(_AlgoBodyBlob & algoBodyBlob, Insp_Image& pImg_buf, CRect &rcBlobBody, _RstAlgoBodyBlob * sRstAlgo, POINTF ptWndCenter, _TotalInspExceptArea stTieArea, std::vector<PIAL::_AlignResult> vecAlignResult, PI_Buff* pMask = nullptr, BodyInfo* pBody = nullptr, BOOL bUseHiddenArea = FALSE, POINTF* pPoHiddenAreaOfPartImgPx = nullptr, int nStickerPo = 0, int nStickerCnt = 0);
		
		//NewAlgo
		bool TeachPadArray(_AlgoPadArray & pAlgoPadArray, const Insp_Image& pImg_buf, PIAL::TeachPadArray* sTeachPadArray, _RstAlgoPadArray * sRstAlgo, POINTF ptWndCenter, PI_Buff* puImgDst = nullptr);
		bool InspPadArray(_AlgoPadArray & pAlgoPadArray, const Insp_Image& pImg_buf, _RstAlgoPadArray * sRstAlgo);

        //Edge 
		bool TeachEdge(_AlgoEdge& algoEdge,PI_Buff* pBuffPart2D , const Insp_Image& pImg_Windowbuf, _RstAlgoEdge * sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff* dstPart, int nStartX , int nStartY);
		bool InspEdge(_AlgoEdge& algoEdge, const Insp_Image& pImg_Windowbuf,  _RstAlgoEdge * sRstAlgo, _TotalInspExceptArea stTieArea, BodyInfo* pBody = nullptr);

		//Line
		bool TeachLine(_AlgoLine& algoLine, PI_Buff* pBuffPart2D, const Insp_Image& pImg_Windowbuf, _RstAlgoLine * sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff* dstPart, int nStartX, int nStartY);
		bool InspLine(_AlgoLine& algoLine, const Insp_Image& pImg_Windowbuf, _RstAlgoLine * sRstAlgo, _TotalInspExceptArea stTieArea, BodyInfo* pBody = nullptr);

		//Align Edge  
		//Binalize, Teach, Insp 통합 
		bool InspAlignEdge(_AlgoAlignEdge& algoLine, const Insp_Image& pImg_buf, _RstAlgoAlignEdge *sRstAlgo, std::vector<_AlignResult>& vecAlignResult, int nSelectArea = -1, PI_Buff* dstPart= nullptr, bool bTeach = false, BodyInfo* pBody = nullptr);
	
		bool TeachAlign(_AlgoAlign& algoAlign, const Insp_Image& pImg_buf, PI_Buff* dstPart = nullptr, bool bIsTeach  = false);
		bool InspAlign(_AlgoAlign& algoAlign, const Insp_Image& pImg_buf,CRect &rcBlobBody, _RstAlgoAlign * sRstAlgo, std::vector<_AlignResult>& vecAlignResult, PI_Buff* dstPart = nullptr, BodyInfo* pBody = nullptr);
		bool InspAlign_DPAMS(_AlgoAlign& algoAlign, const Insp_Image& pImg_buf, CRect &rcBlobBody, _RstAlgoAlign * sRstAlgo, std::vector<_AlignResult>& vecAlignResult, PI_Buff* dstPart);


		//Body Edge
		bool BinalizeBodyEdge(_AlgoBodyEdge& algoBodyEdge, const Insp_Image& pImg_buf, PI_Buff* puImgDst = nullptr); 
		bool TeachBodyEdge(_AlgoBodyEdge& algoBodyEdge, const Insp_Image& pImg_buf, _RstAlgoBodyEdge * sRstAlgo, PI_Buff* puImgDst = nullptr);
		bool InspBodyEdge(_AlgoBodyEdge& algoBodyEdge, const Insp_Image& pImg_buf, _RstAlgoBodyEdge * sRstAlgo, std::vector<_AlignResult>& vecAlignResult, BodyInfo* pBody = nullptr, bool bIsSkipPart = false);


		// Package Thickness
		bool InspPackageThickness(_AlgoPackageThickness* algoThickness, _RstAlgoPackageThickness* rstPackageThickness, PInspDataSet* InspDataSet);
		bool InspPickerCenterHeightMean(_AlgoPackageThickness &pAlgoThickness, const Insp_Image& pImg_buf, _RstAlgoPackageThickness* sRstAlgo, std::vector<PIAL::PInspData*>& pDataSet, PI_Buff *pMask, PI_Buff *pPlaneImg, PI_Buff *pClipImg, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult);
		bool DrawPackageThickness(_AlgoPackageThickness &pAlgoNGBlob, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, RECT bodyRect, std::vector<PIAL::PInspData*> pDataSet, RECT* arrBumpMask, RECT* arrBodyMask, int* nArrMask);

		//TimeCheck
		void LogWrite(CString sLog);
		float GetTime();
		void Savetime(int nSeq, bool bPassive = false);
		void WriteLog_Dev(CString sLog);
		void WriteLog_Dev(int nToolIndex, CString sLog);

		//ColorBase
		cv::Mat GetGreenImage(PI_Buff* pucR, PI_Buff* pucB, byte byColorLightType = 1);
		BOOL ColorSetValue(float fFatorR, float fFatorG, float fFatorB, float fFatorBR, float fFatorBB, float fFatorACR, float fFatorACG, float fFatorACB, float fFatorMGR, float fFatorMGB);

		//QFN
		bool TeachQFN(_AlgoQFN &pAlgoQFN, const Insp_Image& pImg_buf, PIAL::TeachQFN* sTeachQFN, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL);
		bool TeachQFNSecVer(_AlgoQFN &pAlgoQFN, const Insp_Image& pImg_buf, PIAL::TeachQFN* sTeachQFN, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL);
		bool TeachQFN_Test(_AlgoQFN &pAlgoQFN, const Insp_Image& pImg_buf, PIAL::TeachQFN* sTeachQFN, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL);

		//bool InspQFN(_AlgoBGA &pAlgoBGA, const Insp_Image& pImg_buf, _RstAlgoBGA* sRstAlgo, _TotalInspExceptArea stTieArea, PI_Buff *pImgBlob = NULL, PI_Buff *pMask = NULL);
		//bool InspUnit(_AlgoBump &pAlgoBlob, Insp_Image& pImg_buf, _RstAlgoBump* sRstAlgo, int nOffX_pix, int nOffY_pix, std::vector<_AlignResult>& vecAlignResult, PI_Buff *pImgBlob = NULL, bool bUseDiameter = true);

		//geo Match
		bool NGBlobExcepPatternMatching(_AlgoNGBlob & pAlgoNGBlob, const Insp_Image & pImg_buf, CRect rcROI, int nDir, _tagNGBlob_AISaveParam* stAISaveParam = nullptr, void * stModel= nullptr);
		int ModelList_Load(CString strPath);
		int ModelList_Check(CString strPath);
		int ModelLoad(CString strPath);
		int ModelList_Clear(int nLane);
		void DeleteModelBuf();

		

		bool NGBlobAIImageSave(PI_Buff* pGrayImg, PI_Buff* pAIMask, PI_Buff* pBWimg, CRect rcROI, int nCurBlobIndex, _NGBlob_AISaveParam stAISaveParam);

		// GWire
		bool TeachGWire(_AlgoGWire &pAlgoGWire, const Insp_Image& pImg_buf, _RstAlgoGWire * sRstAlgo, PI_Buff* puImgDst = nullptr, bool isInspRstImgSeq = false);
		bool InspGWire(_AlgoGWire &pAlgoGWire, const Insp_Image& pImg_buf, _RstAlgoGWire * sRstAlgo, std::vector<_AlignResult>& vecAlignResult, PI_Buff* dstPart = nullptr);
		bool BinarizeGWire(_AlgoGWire& algoBodyEdge, const Insp_Image& pImg_buf, PI_Buff* puImgDst);
		double BinarizeGWire_Die(_AlgoGWire& algoBodyEdge, const Insp_Image& pImg_buf, PI_Buff* puImgDst, cv::Point *ptCenter = NULL);

		// HALCON OCR
		bool InitHalcon();
		void SetResolutionHalcon(double resolX, double resolY);
		void GetSystemFontList(std::vector<CString>& m_vSystemFont);
		bool HalconInspPOCR(_AlgoPOCR &pAlgoPOCR, const Insp_Image& pImg_buf, _RstAlgoPOCR * sRstAlgo, std::vector<_AlignResult>& vecAlignResult);
		int CheckHalconLicense();
		int CheckHalconLicense_Only();
		//bool HalconInspBarcode(_AlgoBarcode &pAlgoBarcode, _RstAlgoBarcode * _rstBarcode);	//Offset 적용(임시)하기 위해 vecAlignResult사용
		bool HalconInspBarcode(const Insp_Image& pImg_buf, _AlgoBarcode &pAlgoBarcode, _RstAlgoBarcode * _rstBarcode, std::vector<_AlignResult>& vecAlignResult);
		//SetInspParamPattern();
		void HalconTrainOCR(std::vector<std::shared_ptr<cv::Mat>> &src, std::vector<TCHAR> &ch);
		void HalconTrainOCR2(std::vector<std::shared_ptr<cv::Mat>> &src, std::vector<TCHAR> &ch,CString cPath);
		void CreateOcvModel(std::vector<std::shared_ptr<cv::Mat>> &src, std::vector<TCHAR> &ch);
		bool SetHalconFolderPath(std::vector<OCRFilePath>& m_vsFilePath);
		bool AddHalconFolderPath(OCRFilePath m_vsFilePath);
		bool HalconModelLoad();
		void HalconTrainFIleCreate(CString CPath);

		//Pattern diff
		BOOL InspPatternDiff(_AlgoPatternDiff &pAlgoPatternDiff, const Insp_Image& pImg_buf, _RstAlgoPatternDiff* sRstAlgo, _TotalInspExceptArea stTieArea, POINTF ptOffset, int nInspMode, cv::Mat& LayerMask, void * stModel, cv::Mat& AlgoPolyMask, BOOL bUseMask = FALSE, PI_Buff* puImgDst = NULL, BOOL grouping = FALSE, double Maxsize = 0.0, double distance = 0.0, BOOL bUseAxisSize = false, _tagNGBlob_AISaveParam stAISaveParam = _tagNGBlob_AISaveParam(), CString sAIPath = _T(""));
		BOOL InspChippingMode(const Insp_Image& pImg_buf, _AlgoPatternDiff pAlgoPatternDiff, _RstAlgoPatternDiff* pRstAlgoPatternDiff, POINTF ptOffset, int nInspMode, PI_Buff* RstBWimg, cv::Rect rtDst, void * Models, BOOL Grouping = FALSE, double MaxSize = 0, double Distance = 0, _tagNGBlob_AISaveParam stAISaveParam = _tagNGBlob_AISaveParam(), CString sAIPath = _T(""));
		BOOL DrawPatternDiff(_AlgoPatternDiff &pAlgoPatternDiff, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, _RstAlgoPatternDiff* sRstAlgo, POINTF ptOffset,  BOOL bInspMode, int nCurLayer, int nSelectedModel, cv::Mat& AlgoPolyMask, void * stModel =NULL);
		BOOL BinPatternDiff(_AlgoPatternDiff &pAlgoPatternDiff, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, _RstAlgoPatternDiff* sRstAlgo, POINTF ptOffset, BOOL bInspMode, int nCurLayer, int nSelectedModel, cv::Mat& AlgoPolyMask, void * stModel = NULL);
		BOOL BinPatternDiff_Speed(_AlgoPatternDiff &pAlgoPatternDiff, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, _RstAlgoPatternDiff* sRstAlgo, POINTF ptOffset, BOOL bInspMode, int nCurLayer, int nSelectedModel, cv::Mat& AlgoPolyMask, void * stModel = NULL);
		BOOL Histogram_PatternDiff(_AlgoPatternDiff &pAlgoPatternDiff, const Insp_Image& pImg_buf, uchar* pHistoImage, int nCurLayerNum, int dx, int dy, int *nRetRangeMin, int *nRetRangeMax, int *nRetRangeAvg, int* nRetMaxFreqValue, cv::Mat& AlgoPolyMask);
		BOOL SetPatternDiffHistoData(_AlgoPatternDiff &pAlgoPatternDiff, PI_Buff* piGrayImg, int nCurLayerNum, int dx, int dy, int * nArrHisto, int nDownScale, double dPartAngle, cv::Mat AlgoPolyMask, void * stModel = NULL);
		BOOL BinPatternDiff_Chipping(_AlgoPatternDiff &pAlgoPatternDiff, const Insp_Image& pImg_buf, PI_Buff *pImgBlob, _TotalInspExceptArea stTieArea, _RstAlgoPatternDiff* sRstAlgo, POINTF ptOffset, BOOL bInspMode, int nCurLayer, int nSelectedModel, void * stModel = NULL);
		BOOL BinPatternDiff_MaskBW(_AlgoPatternDiff &pAlgoPatternDiff, PI_Buff *&pImgBlob, int nImgWidth, int nImgLength, int nSelectedModel, float dPartAngle);
		float CalcExceptScore(PI_Buff * OrgGrayImg, _AlgoPatternDiff pAlgoPatternDiff, CRect rcROI, int nDir, void * Models);
		//int CalcExceptScore(PI_Buff* OrgGrayImg, _AlgoPatternDiff pAlgoPatternDiff, CRect rcROI, int nDir);

		//Warpage Map
		bool Resize_Down_Min(PI_Buff* src, PI_Buff* dst);
		bool Resize_Down_Max(PI_Buff* src, PI_Buff* dst);
		bool Resize_Down_Max_Margin(PI_Buff* src, PI_Buff* dst, int nXmargin, int nYmargin);
		bool WaferWarpage(PI_Buff* src, float fthredHold, float* fWarpge, PI_Buff* dst = nullptr, float* fMax = nullptr, float* fMin = nullptr, POINT* ptMax = nullptr, POINT* ptMin = nullptr);

		bool WaferWarpage_2nd(PI_Buff* src, float fthredHold, float* fWarpge, PI_Buff* dst = nullptr, float* fMax = nullptr, float* fMin = nullptr, POINT* ptMax = nullptr, POINT* ptMin = nullptr, PI_Buff* mask = nullptr);

		bool ResizeToHalf_GrayMin_AVX(uchar* src, uchar* dst, int width, int length);
		bool MergePixelShiftImg(uchar* src1, uchar* src2, uchar* src3, uchar* src4, uchar *dst, int width, int length);

		bool ResizeImg_Linear_IPP(void* src, int srcStep, int srcWidth, int srcLength, int nElementSize, int nChannel, void* dst, int dstStep, int dstWidth, int dstLength);

		//ShapeX
		bool InspShapeX(_AlgoShapeX &pAlgoShapeX, const Insp_Image& pImg_buf, _RstAlgoShapeX* sRstAlgo, _TotalInspExceptArea stTieArea, int TabIndex, PIAL::PI_Buff* Mask_buf, BOOL bUseMask, PI_Buff* puImgDst, bool bTeach, bool bGray);
		bool InspShapeX(_AlgoShapeX &pAlgoShapeX, const Insp_Image& pImg_buf, _RstAlgoShapeX* sRstAlgo, _TotalInspExceptArea stTieArea, std::vector<_AlignResult>& vecAlignResult, int TabIndex, PIAL::PI_Buff* Mask_buf, BOOL bUseMask, PI_Buff* puImgDst, bool bTeach, int nInspType, int nWndID, bool bGray, bool bBin = false);
		//bool InspShapeXWithType(_AlgoShapeX &pAlgoShapeX, const Insp_Image& pImg_buf, _RstAlgoShapeX* sRstAlgo, cv::Mat d3_z0, _TotalInspExceptArea stTieArea, int TabIndex, PIAL::PI_Buff* Mask_buf, BOOL bUseMask, PI_Buff* puImgDst, bool bTeach);

		BOOL SearchShapeXROI(PI_Buff* Src, PI_Buff* p3D, _AlgoShapeX _AlgoShapeX, _RstAlgoShapeX* RstShapeX, double dShapeXMargin);
		BOOL AddShapeModel(PI_Buff* Src, PI_Buff* p3D, _AlgoShapeX algoShapeX, RECT RstShapeX, double dShapeXMargin);
		BOOL ShapeX_LoadImageFile(CString sPath);
		BOOL LoadFileList(_sShapeXList* sFileList, int nFileCnt, double dResolX, double dResolY, bool(*ExtModelLoad)(CFile *, CString, int));
		BOOL CheckFileList(_sShapeXList* sFileList, int nFileCnt, double dResolX, double dResolY , bool(*ExtModelLoad)(CFile *, CString, int));
		BOOL CheckExtFileList(_sShapeXList * sFileList, double dResolX, double dResolY, std::vector<std::tuple<BYTE*, int>> inputData);
		BOOL LoadExtFileList(_sShapeXList * sFileList, double dResolX, double dResolY, std::vector<std::tuple<BYTE*, int>> inputData);
		BOOL ClearFileList(int nLane);
		int GetModelCnt(wchar_t* arrPath, double dResolX, double dResolY);
		BOOL ModelMaskChange(cv::Mat& newMask, wchar_t* arrPath, int nIdx);
		BOOL ModelDelete(wchar_t* arrPath, int nIdx);
		BOOL ModelAdd(wchar_t* arrPath, PI_Buff* Src, PI_Buff* Bin, PI_Buff* p3D, _Bin bin, double dShapeXMargin, BOOL bHole);
		int ModelAdd2(PI_Buff* Src, PI_Buff* Bin, PI_Buff* p3D, _AlgoShapeX algoShapeX, _RstAlgoShapeX* RstShapeX, double dShapeXMargin, int i);
		_Bin AlgoToBin(_AlgoShapeX);
		BOOL DrawShapeXModel(PI_Buff* Src, PI_Buff* Dst, _AlgoShapeX& palgoShapeX, int* ModelIdx, POINTF* poArrCenter);
		BOOL DrawShapeXModelMatching(PI_Buff* Src, PI_Buff* Dst, _AlgoShapeX& palgoShapeX, int ModelIdx, int SeleteROI);
		void SetShapeXMatchingPoint(int Cnt, POINTF* ArrPoints, int* nArrMatchIdx, float* nArrMatchSc);
		void SetShapeXCurrentMask(cv::Mat mask16);
		void ClearShapeXCurrentMask();
		BOOL SaveShapeXCurrentMask(wchar_t* sPath);
		BOOL DrawShapeXCurrentMask(PI_Buff* Src, PI_Buff* Dst, _AlgoShapeX& palgoShapeX);
		BOOL CreateShapeXCurrentMask(cv::Mat& outMask16, _AlgoShapeX& algoShapeX, _RstAlgoShapeX& rstShapeX, int nImgWidth, int nImgHeight, PI_Buff* pMixMask = nullptr, PI_Buff* ucWindTabImg = nullptr);
		int ShapeXModelKey(int ModelIndex);
		//BOOL SearchShapeXCurrentMaskAndSave(PI_Buff* Src, PI_Buff* p3D, _AlgoShapeX _AlgoShapeX);

		// sjb++ 250403.[43] ~PInspAlgo.h~
		BOOL ModelAddWithType(wchar_t* arrPath, PI_Buff* Src, PI_Buff* Bin, PI_Buff* p3D, _Bin bin, double dShapeXMargin, int targetType, bool searchByShape, BOOL bHole, bool bUseNonCritical = false);
		// esjb++ 250403.[43]
		bool GetShapeXHistogramGraph(_AlgoShapeX& pAlgoShapeX, const Insp_Image& pImg_buf, uchar* pHistoImage, cv::Mat& AlgoPolyMask, int nCurLayerNum, int dx, int dy, int* nRetRangeMin, int* nRetRangeMax, int* nRetRangeAvg, int* nRetMaxFreqValue, BOOL bInspMode = TRUE);
		bool GetShapeXHistogramGraph_Smooth(_AlgoShapeX& pAlgoShapeX, const Insp_Image& pImg_buf, uchar* pHistoImage, int* nRetRangeMin, int* nRetRangeMax, int* nRetRangeAvg, int* nRetMaxFreqValue, int TabIndex, BOOL bInspMode = TRUE, double dAlignRstTheta = 0.0, int nTargetType = 0);

		BOOL ExistAreaReDraw(wchar_t* arrPath);

		//Barcode
		bool Validate1DBarcodeArea(PI_Buff* src, POINTF* pts, int nNum);
		bool Validate1DBarcodeInsp(_AlgoBarcode & pAlgoBarcode, _RstAlgoBarcode * _rstBarcode, PI_Buff * src);
		void LogFunc_Add(void(*InputFunc)(CString));
		void AddLog(CString sLog);

		//TestCode
		// 프로세스 내 동시 호출 안전(읽기 공유락)
		bool TryReadStripeRateFromIni(const std::string& iniPath, float& stripeRate);
		bool TryReadStripeRate(float& stripeRate);
		inline void TrimInPlace(std::string& s);
		inline bool IEquals(const std::string& a, const std::string& b);
		inline bool StartsWithI(const std::string& s, const std::string& prefix);
		inline bool TryParseFloatInvariant(const std::string& text, float& out);
	};
}
