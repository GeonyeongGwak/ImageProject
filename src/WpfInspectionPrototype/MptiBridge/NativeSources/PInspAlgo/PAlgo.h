#pragma once

#include "PInspalgo_Def.h"
#include "PI_Buff.h"
#include "compositingImage.h"
//#include "Insp_Wnd.h"
//#include "MilWrapper.h"
namespace PIAL
{
	class PI_Blob_Res;
	class PINSPALGO_API PAlgo
	{
	public:
		PAlgo();
		~PAlgo();

		static _AlgoBlob ConvertToBlob(_AlgoBGA bga);
		static _AlgoBlob ConvertToBlob(_AlgoBodyBlob NGBlob);
		static _AlgoBlob ConvertToBlob(_AlgoEdge edge);
		static _AlgoBlob ConvertToBlob(_AlgoLine line);
		static _AlgoBlob ConvertToBlob(_AlgoAlign align);
		static _AlgoBlob ConvertToBlob(_AlgoBodyEdge bodyedge, bool bSub = false);
		static _AlgoBlob ConvertToBlob(_AlgoQFN qfn);
		static _AlgoBlob ConvertToBlob(_AlgoGWire gwire, bool bIsDie = false);
		static _AlgoBlob ConvertToBlob(_AlgoShapeX ShapeX);
		static _Blob ConvertTo_Blob(_AlgoNGBlob NGBlob);
		static _Blob ConvertTo_Blob(_AlgoPadArray PadArray);
		static _Blob ConvertTo_Blob(_AlgoPackageThickness Thickness);
		static _Blob ConvertTo_Blob(_AlgoBGA bga);

		//Common Method
		static void InvalidRectInImage(RECT & rect, int nWidth, int nLength);
		static bool IsAnyAngle(double angle);

		//Blob 관련
		static float GetCropZmap_LT(float* src, float*dst, int orgSizeX, int orgSizeY, int nStX, int nStY, int roiSizeX, int roiSizeY);
		static bool BinalizeImage(_Bin &bin, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff *srcColor, PI_Buff *dst, int nWidth, int nHeight);
		static bool BinalizeImageByHistogram(_Bin& bin, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff* srcColor, PI_Buff* Mask, PI_Buff* dst, int nWidth, int nHeight, int nRangeMin, int nRangeMax, int* nHisto = nullptr, PI_Buff* HistoMask = nullptr);
		static bool BinalizeImage_ex(_Blob &blob, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff *srcColor, PI_Buff *dst, int nWidth, int nHeight);
		static bool BinalizeImageByHistogram_ex(_Blob& blob, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff* srcColor, PI_Buff* Mask, PI_Buff* dst, int nWidth, int nHeight, int nRangeMin, int nRangeMax, std::vector<POINTF>& vecBodyEdge, int nGridX = 1, int nGridY = 1, int* nHisto = nullptr, PI_Buff* HistoMask = nullptr);
		static bool BinalizeImageByHistogram_Local(_Blob &blob, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff *srcColor, PI_Buff* Mask, PI_Buff *dst, int nRangeMin, int nRangeMax, std::vector<POINTF>& vecBodyEdge, std::vector<PI_Blob_Res>& blobs, float fAreaRate =1.5);
		static int Contrast_Local(_Blob &blob, PI_Buff* Src2D, PI_Buff* Mask, PI_Buff *dst,std::vector<POINTF>& vecBodyEdge, std::vector<PI_Blob_Res>& blobs, float fAreaRate = 1.5);
		static void Contrast(PI_Buff* Src2D, PI_Buff* Mask, PI_Buff *dst, std::vector<POINTF>& vecBodyEdge, std::vector<PI_Blob_Res>& blobs, float fAreaRate = 2);
		static void ContrastMorph(PI_Buff* Src2D, PI_Buff* Mask, PI_Buff *dst, std::vector<POINTF>& vecBodyEdge, std::vector<PI_Blob_Res>& blobs, float fAreaRate = 2);
		static void ContrastMorphBlob(PI_Buff* Src2D, PI_Buff* Mask, PI_Buff *dst, std::vector<POINTF>& vecBodyEdge, std::vector<PI_Blob_Res>& blobs, float fAreaRate = 2, bool bUseContrastExcept = false, int nContrastExceptMode = 0, int nContrastExceptNum = 0);
		static void ColorContrast(PI_Buff* pBuffR, PI_Buff* pBuffG, PI_Buff* pBuffB, PI_Buff* Mask, PI_Buff *dst, std::vector<POINTF>& vecBodyEdge, std::vector<PI_Blob_Res>& blobs, float fAreaRate = 2);

		static int AllBlob(_Blob &bin, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff *srcColor, PI_Buff *dst, int nWidth, int nHeight);
		static int AllBlob(_AlgoBlob &bin, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff *srcColor, PI_Buff *dst, int nWidth, int nHeight);
		static int BlobDirect(PI_Buff* Src);
		static int BlobDirect(PI_Buff* SrcDst, int MinArea);
		static int BlobRemove(PI_Buff* dst, std::vector<int>& vecID);

		
		static double CalcBWInspectArea(PI_Buff *pBWSrc, int *nImgW, int *nImgH, int nDir, int nInspectionAreaPer, int* pnROIValue);
		static double GetBWImageStruct(_AlgoBaseBW& AlgoBaseBW , PI_Buff *pInspAlgoBW, PI_Buff *pucImgSrc, int nWidth, int nHeight, PI_Buff *& ucArrDstImg, bool bUseColor = false, PI_Buff* ArrColorImg = NULL, int nDir = 10, int nInspAreaPer = 100);
		static int FillMaskingROI(int nWidth, int nHeight, double dAngle, PI_Buff *fArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue = 0, PI_Buff *ArrOverlapImg = NULL);
		static int FillMaskingROI(int nWidth, int nHeight, double dAngle, PI_Buff *fArrSrcImg, int nUsedMaskingValue, const std::vector<RECT> rcArrMaskingROI, int nFillValue = 0, PI_Buff *ArrOverlapImg = NULL);
		static int FillMaskingROIUsingOpenCV(int nWidth, int nHeight, double dAngle, PI_Buff *ucArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue = 0, PI_Buff *ucArrOverlapImg = NULL, int nChanel = 1);
		static int FillMaskingROIUsingOpenCV(int nWidth, int nHeight, double dAngle, PI_Buff *ucArrSrcImg, int nUsedMaskingValue, const std::vector<RECT> rcArrMaskingROI, int nFillValue = 0, PI_Buff *ucArrOverlapImg = NULL, int nChanel = 1);
		static void FillMaskingROIUsingOpenCVImg(int nWidth, int nHeight, double dAngle, PI_Buff *ucArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue = 0, PI_Buff *ucArrOverlapImg = NULL, int nChanel = 1, PI_Buff *ucArrFillOutImg = NULL, PI_Buff *ucMask = NULL);

		static bool AnglePointChange(const int nWidth, const int nHeight, const POINTF poSrcPoint, POINTF *poDstPoint);
		static int FillOutOfInspArea(int nWidth, int nHeight, double dAngle, PI_Buff *ArrSrcImg, PI_Buff *ArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue = 0, bool bConvertExceptROI = false, int nChanel = 1);
		static int FillOutOfInspAreaCombine(int nWidth, int nHeight, double dAngle, PI_Buff* src, PI_Buff* Overlap, _TotalInspExceptArea stTieArea);
		static int FillOutOfInspAreaCombine_Color(int nWidth, int nHeight, double dAngle, PI_Buff* ucArrSrcImg, PI_Buff* ucArrOverlapImg, _TotalInspExceptArea stTieArea);
		static void SaveWorkImg(PI_Buff* src, CString fileName, bool bSaveSide = false);
		static int BlobImageStruct_Base(_AlgoBlobBase* algo, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff* srcColor, int nWidth, int nHeight, _TotalInspExceptArea stTieArea, PI_Buff *DstImg);
		static int BlobImageStruct(_AlgoBlob &algoBlob, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff *srcColor, PI_Buff *dst,int nWidth, int nHeight, int nMinBlobArea,
			double *dArea, double *dCx, double *dCy, CRect *rcBlob, _TotalInspExceptArea stTieArea,
			bool bApplyFillHole, bool bAlignAlgo = false, int nTeachX = -1, int nTeachY = -1, bool eraseBorderBlob = false, double dAreaPix = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = 0);

		static int BlobImageStructWithType(_AlgoBlob& algoBlob, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff* srcColor, PI_Buff* dst, int nWidth, int nHeight, int nMinBlobArea,
			double* dArea, double* dCx, double* dCy, CRect* rcBlob, _TotalInspExceptArea stTieArea, int nSelectType,
			bool bApplyFillHole, bool bAlignAlgo = false, int nTeachX = -1, int nTeachY = -1, bool eraseBorderBlob = false, double dAreaPix = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = 0);


		static int BlobImageStruct_BW(_AlgoBaseBW &pInspAlgoBW, PI_Buff* Src2D, PI_Buff* src3D, int nWidth, int nHeight, int nMinBlobArea,
			double *dArea, double *dCx, double *dCy, CRect *rcBlob, PI_Buff* ucArrDstImg, int nTypeSelectBlob, bool bFilterIsUse, int nFilterStepNarrow, _TotalInspExceptArea stTieArea,
			BOOL bApplyFillHole, BOOL bAlignAlgo = FALSE, PI_Buff*  ucArrColorImg = NULL);

		//Detect Circle
		static int BlobCircleStruct(_AlgoBlob &algoBlob, PI_Buff* Src2D, PI_Buff* src3D, PI_Buff *srcColor, PI_Buff *dst, int nWidth, int nHeight, int nMinBlobArea, _TotalInspExceptArea stTieArea);

		static bool BlobCircleExcepDegreeArea(std::vector<cv::Point>& pts, POINTF ptBasisCenter, float fDegreeExcept, double& dCirle_x, double& dCirle_y, double* dRadius, double* dErrorRate);
		static bool BlobCircleRecoveryCenter(PI_Buff* src, double& dCirle_x, double& dCirle_y, double* dArea = NULL, double* dRadius = NULL, CRect* rect = NULL, double* dErrorRate = NULL);
		static bool BlobCircleRecoveryCenter_2nd(PI_Buff* src, double& dCirle_x, double& dCirle_y, double* dArea = NULL, double* dRadius = NULL, CRect* rect = NULL, double* dErrorRate = NULL);
		static bool BlobCircleRecoveryCenter_AVI(PI_Buff* src, double& dCirle_x, double& dCirle_y, double* dArea = NULL, double* dRadius = NULL, CRect* rect = NULL, double* dErrorRate = NULL);
		static bool BlobCircleCenter(PI_Buff* src, double& dCirle_x, double& dCirle_y,double* dArea = NULL,double* dRadius = NULL,CRect* rect = NULL, double* dErrorRate = NULL, bool bDetail = false);
		static bool BlobCircleCenterByBlob(double& dCirle_x, double& dCirle_y, double* dArea = NULL, double* dRadius = NULL, CRect* rect = NULL, double* dErrorRate = NULL, int nLabelID = -1, bool bDetailError = false);
		static bool BlobCircleCenterByBlob_SmoothError(double& dCirle_x, double& dCirle_y, double* dArea, double* dRadius, CRect* rect, double* dErrorRate, int nLabelID, bool bDetailError);
		static bool BlobCircleCenterByContour(double& dCirle_x, double& dCirle_y, double* dArea = NULL, double* dRadius = NULL, CRect* rect = NULL, double* dErrorRate = NULL, std::vector<cv::Point> contour = std::vector<cv::Point>() , bool bDetailError = false);
		static bool BlobCircleCenter_AVI(PI_Buff* src, double& dCirle_x, double& dCirle_y, double* dArea = NULL, double* dRadius = NULL, CRect* rect = NULL, double* dErrorRate = NULL, bool bDetail = false);

		static bool BlobCircleCenterByBlob_UnCoining(double& dCirle_x, double& dCirle_y, double* dArea = NULL, double* dMajoraxis = NULL, double* dMinoraxis = NULL, double* dRadius = NULL, CRect* rect = NULL, double* dErrorRate = NULL, int nLabelID = -1, bool bDetailError = false);
		static bool BlobCenterByBlob(PI_Buff* src, double& dCirle_x, double& dCirle_y, double* dArea, CRect* rect, int nLabelID);

		//detect ellipse
		static bool BlobEllipseFocusByBlob(double& dFocusX1, double& dFocusY1, double& dFocusX2, double& dFocusY2, double& major, double& minor, double& dAngle, CRect* rect, double* dErrorRate, int nLabelID);

		static bool GetConvexArea(std::vector<cv::Point>& pts, double& dConvexArea);

		static float GetHeightMean(PI_Buff *pfArrImgData,int nExceptCount = 0);
		static float GetHeightAvgMask(PI_Buff* p3D, PI_Buff* p2DMask);
		static float GetHeightAvgMask(PI_Buff* p3D, PI_Buff* p2DMask, int nStX, int nStY, int roiSizeX, int roiSizeY);
		static void GetHeightMinMax(float *pfArrImgData, int nImgWidth, int nImgHeight, float *fHeightMin, float *fHeightMax);

		static float GetMinMaxImg(float *pfArrImgData, int nImgWidth, int nImgHeight, int Min);
		static float _GetMinMaxImg(PI_Buff* p_fbuff, int nImgWidth, int nImgHeight, int Min);

		static float GetHeightMaxHist(float *pfArrImgData, int nImgWidth, int nImgHeight);
		static float _GetHeightMaxHist(PI_Buff* p_fbuff);

		static float GetHeightGradTest(PI_Buff* p3D, PI_Buff* p2DMask); //Test 용

		static bool GetHeightMinMax(PI_Buff* p3D, PI_Buff* p2DMask, float* fMax, float* fMin, POINT* ptMax, POINT* ptMin);

		static float GetMean_8u(PI_Buff* p_buff, PI_Buff* p_Mask);

		static float LeastSquarePlane(int cnt, float *height, POINTF* pint, double* A, double* B, double* C); //USE_PCA
		static float CalcTiltAngle(PI_Buff* p3D, double* A, double* B, double* C);
		//미분하여 해당 기울기 이내 인 것만의 평균으로 계산
		static float GetHeightGrad(PI_Buff* p3D, PI_Buff* p2DMask, float fPercent = 80, float Grad = 10);
		//평균의 해당 Percent 이상의 것만 평규으로 계산
		static float GetHeightPercent(PI_Buff* p3D, PI_Buff* p2DMask, float fPercent = 80);
		static float GetHeightPixel(PI_Buff* p_fbuff, int cx, int cy, int Pixel);
		static float GetMaxHeightPixel(PI_Buff* p_fbuff, PI_Buff* p2DMask, int cx, int cy, int Pixel);
		static float GetHeightOnSurface(PI_Buff* p_fbuff, CRect rect, int cx, int cy, int Pixel);
		static float GetHeightSurface(PI_Buff* p_fbuff, CRect rect, int cx, int cy, int Pixel);
		static float GetHeightReverseArea(PI_Buff* p_f3D,PI_Buff* p_2DMask, CRect rect);
		static float GetHeightReverseArea_FlatBall(PI_Buff* p_f3D, PI_Buff* p_2DMask, CRect rect, float fHeight);
		static float GetHeightReverseArea_Tiny(PI_Buff* p_f3D, PI_Buff* p_2DMask, PI_Buff* p_2D, CRect rect, BOOL& bMissing,float fBallHeight);
		static float GetHeightThresholdAvg(PI_Buff* p3D, PI_Buff* p2DMask);

		static bool GetHeightInGray(PI_Buff* p_3d, PI_Buff* p_Mask, CRect rect, float *fAvg, float* fMax = NULL, float* fMin = NULL);
		static bool BinalizeHeight(PI_Buff* p_3d, PI_Buff* p_dst, CRect rect, float fvalue);
		//Blob의 Width Length
		static void GetBlobArea(PI_Buff* src, int nSrcWidth, int nSrcHeight, double dSrcCx, double dSrcCy, double dBlobW, double dBlobH, double dWndAngle, double *rstW, double *rstH, int nOkPercent = 50);
		static void GetContourDefectSize(CRect rtLabel, int nLabelID, float& fWidth, float& fLength, float* fRstW = NULL, float* fRstL = NULL);
		static void GetContourDefectSize_WithContour(CRect rtLabel, int nLabelID, float& fWidth, float& fLength, std::vector<POINT>& contour);
		static void GetContourDefectSize_MaxSize(CRect rtLabel, int nLabelID, float& fWidth, float& fLength, float* fRstW, float* fRstL);

		static int CorrectCoordinate(double pos_x, double pos_y, double ct_x, double ct_y, double theta, double delta_x, double delta_y, double *corr_x, double *corr_y);
		static int CorrectCoordinate(POINTF pos, double ct_x, double ct_y, double theta, double delta_x, double delta_y, POINTF *corr);
		
		static float TestSolderBall(PI_Buff* src, float dia, float stdscore, std::vector<CRect> & rcBlob);
		static float TestSolderBall(PI_Buff* src, float dia, CRect rcBlob);

		static bool GetBlobByArea(CRect rcRect, PI_Blob_Res* results);
		static bool AngleRectChange(const int nAngle, const int nWidth, const int nHeight, const RECT rcSrcRect, RECT *rcDstRect);
		static bool AngleRectChange2(const double dAngle, const int nWidth, const int nHeight, const RECT rcSrcRect, RECT *rcDstRect);
		static bool AngleRectChange3(const int nAngle, const int nWidth, const int nHeight, const RECT rcSrcRect, RECT *rcDstRect, int nWndStartX, int nWndStartY);

		static float CalcTiltAngle(float *pfArrImgData, int nImgWidth, int nImgHeight, double* A, double* B, double* C);

		static void GetLineInterpolation(PI_Buff* bin, std::vector<cv::Point>& output);
		static void SetSubPixel(_AlgoBlob &algoBlob, PI_Buff* pOutput, PI_Buff* pGray, PI_Buff* p3D, PI_Buff* pColor, int nFactor = 3);

		static int GetHistogramCount(int rangeMode, int minRange, int maxRange, int *histogram);
		static void GetHistoValue(UCHAR* userSrc, int sizeX, int sizeY, int* retValue);
		static void GetHistogramResult(cv::Mat Histogram, int* rstHistogram);

		static void ChangeROI_CorrectCoordinate(_AlignResult* sAlignRes, double dROIWidth, double dROIHeight, double* dROIX, double* dROIY, int nAlignResCnt, int nImgWidth, int nImgHeight);

		static BOOL FileExists(CString strFilePath);
		static RECT GetBlobRect(long *pLebel, USHORT* LabelImage, int nCntBlob, int nW, int nH, int nCX, int nCY, int nRoiW, int nRoiH);

		static bool BinalizeImage_Histogram2D(_PadBin& bin, PI_Buff* Src2D, PI_Buff* srcColor, PI_Buff* dst, int nWidth, int nHeight, PI_Buff* Mask2D, int* histo, int nMinMargin = 20, int nMaxMargin = 230, bool bUse2Q = false);
		static bool BinalizeImage_Histogram2D(_PadBin& bin, PI_Buff* Src2D, PI_Buff* srcColor, PI_Buff* dst, int nWidth, int nHeight, int nBWThreshVal);
		static bool BinalizeImage_Histogram2D_Grid(_PadBin &bin, PI_Buff* Src2D, PI_Buff *srcColor, PI_Buff *dst, int nWidth, int nHeight, PI_Buff* Mask2D, int* histo, int nMinMargin = 20, int nMaxMargin = 230,int nGridX =1,int nGridY =1, bool bUse2Q = false);
		static int GetThresHoldORG(PI_Buff* src, int* thresh, PI_Buff* mask, int* histo, int nMinMargin, int nMaxMargin);
		static int GetThresHold(PI_Buff* src, int* thresh, PI_Buff* mask, int* histo, int nMinMargin, int nMaxMargin);
		static int GetThresHold_Smooth(PI_Buff* src, int* thresh, PI_Buff* mask, int* histo, int nMinMargin, int nMaxMargin);
		static void Smooth5(std::vector<double>& h);
		static int GetThresHold_valley(PI_Buff* src, int* thresh, PI_Buff* mask, int* histo, int nMinMargin, int nMaxMargin);
		static int GetThresHold_1D_2Q(PI_Buff* src, int* thresh, PI_Buff* mask, int* histo, int nMinMargin, int nMaxMargin);
		static double GetNormalHistPeak(const std::vector<double>& Hist);
		static int GetHistogram(PI_Buff* src, float* histo, PI_Buff* mask, int* nArrhisto, int nMinMargin, int nMaxMargin);
		static int Histogram_Smooth(PI_Buff* buffSrc, PI_Buff* buffMask, CRect rtRoi, int nMinMargin, int nMaxMargin);
		static int Histogram_Smooth_2(PI_Buff* buffSrc, PI_Buff* buffMask, CRect rtRoi, int nMinMargin, int nMaxMargin);
		static int Histogram_1D_2Q(PI_Buff* buffSrc, PI_Buff* buffMask, CRect rtRoi, int nMinMargin, int nMaxMargin, int* histo = NULL);

		static int GetThresHold(PI_Buff* src, int* thresh);
		static int GetHistogram(PI_Buff* src, float* histo);

		static void MakeHistoImage(int* histo, int histoSize, UCHAR* userDst);


		static void RotateImg_ipp(UCHAR* userSrc, int orgSizeX, int orgSizeY, double angle, UCHAR** userDst, int* retDstSizeX = NULL, int* retDstSizeY = NULL);
		static bool RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor = false, bool bLinear = true, bool bUseOrgSize = false, double centX = -1, double centY = -1);
		static bool RotateImg_ipp2020_2(unsigned char* userSrc, unsigned char* userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor = false, bool bLinear = true, bool bUseOrgSize = false);
		static bool RotateSImg_ipp2020(short* userSrc, short* userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor = false, bool bLinear = true, bool bUseOrgSize = false, double centX = -1, double centY = -1);
		static bool RotateSImg_ipp2020_2(short* userSrc, short* userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor = false, bool bLinear = true, bool bUseOrgSize = false);
		static bool GetRotateSize_ipp2020(double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY);
		static bool RotateZmap_ipp2020_2(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear, bool bUseOrgSize, int centX = -1, int centY = -1);
		//static void GetVoidMergeImage(Insp_Image* pImg_buf, PI_Buff* th_buf, PI_Buff* dst_buf);

		static void MorErode(PI_Buff* Src, PI_Buff* Dst, int nShape = 0, int ksize = 3);
		static void MorDilate(PI_Buff* Src, PI_Buff* Dst, int nShape = 0, int ksize = 3);

		static bool AngleShiftByFiducial(int nSize, POINTF* ref, POINTF* obj, float* fOffsetX, float* fOffsetY, float* fTheta, float* fShiftX = nullptr, float* fShiftY = nullptr);

		// 입력된 높이 만큼 Offset 만큼 올린 후 계산
		static float GetVolume_Flux(PI_Buff* p_f3D, PI_Buff* p_2DMask, CRect rect, float fHeightDiff);
		static float GetVolume(PI_Buff* p_f3D, PI_Buff* p_2DMask, CRect rect);

		static void HeightSubC(PI_Buff* p3DBuff, float value);

		// 수직, 수평방향으로 각각 연속되는 픽셀 중 nMaskSize 보다 적게 연속되는 픽셀 제거. nBorder만큼의 테두리는 제거 안함
		static void CrossEraseFilter(PI_Buff *src, int nMaskSize, int nBorder = 0, int nThres = 255, int nSetTo = 0);
		static cv::Mat FillPolygonImg(cv::Mat ucMask, cv::Mat binImg, int width, int height, POINT* ptDstPoint, bool isExceptROI = false, int nUsedPolygon = 0, int* cnt = nullptr, PI_Buff* ucArrOverlapImg = nullptr, int nChanel = 1, int nExcept = 1);
	
		static float MinHeightROI_IPP(PI_Buff* src, float fStartX, float fStartY, float fGapX, float fGapY);
		static float MaxHeightROI_IPP(PI_Buff* src, float fStartX, float fStartY, float fGapX, float fGapY);
		static float MaxHeightROI(PI_Buff* src, float fStartX, float fStartY, float fGapX, float fGapY);
		static bool Resize_Down_Min(PI_Buff* src, PI_Buff* dst);
		static bool Resize_Down_Max(PI_Buff* src, PI_Buff* dst);
		static bool Resize_Down_Max_Margin(PI_Buff* src, PI_Buff* dst, int nXmargin, int nYmargin);

		static bool ResizeToHalf_GrayMin_AVX(uchar* src, uchar *dst, int width, int length);
		static bool ResizeToHalf_GrayMax_AVX(uchar* src, uchar *dst, int width, int length);
		static bool ResizeToHalf_GrayMean_AVX(uchar* src, uchar* dst, int width, int length);
		static bool ResizeImg_Linear_IPP(void* src, int srcStep, int srcWidth, int srcLength, int nElementSize, int nChannel, void* dst, int dstStep, int dstWidth, int dstLength);

#pragma region ResizeGray_Linear_IPP_List
		// 위의 ResizeImg_Linear_IPP 함수 사용 추천. 해당 함수에서 이미지의 타입에 따라 분기하여 아래 함수로 진행
		static bool ResizeImg_Linear_IPP_8UC1(uchar* src, int srcStep, int srcWidth, int srcLength, uchar* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_8UC3(uchar* src, int srcStep, int srcWidth, int srcLength, uchar* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_8UC4(uchar* src, int srcStep, int srcWidth, int srcLength, uchar* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_16UC1(ushort* src, int srcStep, int srcWidth, int srcLength, ushort* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_16UC3(ushort* src, int srcStep, int srcWidth, int srcLength, ushort* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_16UC4(ushort* src, int srcStep, int srcWidth, int srcLength, ushort* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_16SC1(short* src, int srcStep, int srcWidth, int srcLength, short* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_16SC3(short* src, int srcStep, int srcWidth, int srcLength, short* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_16SC4(short* src, int srcStep, int srcWidth, int srcLength, short* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_32FC1(float* src, int srcStep, int srcWidth, int srcLength, float* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_32FC3(float* src, int srcStep, int srcWidth, int srcLength, float* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_32FC4(float* src, int srcStep, int srcWidth, int srcLength, float* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_64FC1(double* src, int srcStep, int srcWidth, int srcLength, double* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_64FC3(double* src, int srcStep, int srcWidth, int srcLength, double* dst, int dstStep, int dstWidth, int dstLength);
		static bool ResizeImg_Linear_IPP_64FC4(double* src, int srcStep, int srcWidth, int srcLength, double* dst, int dstStep, int dstWidth, int dstLength);
#pragma endregion

		static bool MergePixelShiftImg(uchar* src1, uchar* src2, uchar* src3, uchar* src4, uchar *dst, int width, int length);

		// n차 다항함수의 WarpageMap과 FlattenMap을 AVX를 활용해 생성하는 함수
		// 1, 2차는 따로 최적화된 함수로 진행
		// 3차 이상은 일반화된 함수로 개발되어 1, 2차에 비해서 비교적 최적화가 덜 진행됨
		static bool MakeWarpageMap_AVX(int nOrder, std::vector<double>& coeff, PI_Buff* dst);
		static bool MakeFlattenMap_AVX(int nOrder, PI_Buff* src, std::vector<double>& coeff, PI_Buff* dst, bool bExceptVal = false, float fExceptVal = 0.f);

		//Compose Light Image
		static void RoiCalcImageCompose(_LightTypeBuf& sLightBuf, UCHAR* ptrbyResultImage);

		static int correctIdx(const int &size, const int &targetIdx);
		static void FittingFeaturedPolygons_Img(IN PI_Buff &srcBIn, IN OUT std::vector<std::vector<POINTF>> &vTemplates);
		static void FittingFeaturedPolygon_Teach(IN std::vector<cv::Point> &vContour, IN OUT std::vector<FeaturedPoint> &vTemplate, IN int nImgWidth, IN int nImgLength);
		static void FittingFeaturedPolygon(IN std::vector<cv::Point> &vContour, IN OUT std::vector<FeaturedPoint> &vTemplate, IN int nImgWidth, IN int nImgLength, IN double dSearchRange = 7, IN int nIdxMargin = 25);

		static void ApplyAlgoPolyMask(cv::Mat& DstMask, cv::Mat& AlgoPolyMask);
		static void SetEdgeFilterParam(int nFilterLv, int& nMargin, double& dStandardExludeRate);
		static bool ExcludeEdgeBlob(PI_Buff* Mask, cv::Mat& LabelMask, int nMargin, double dExcludeRate, int nCurLabel, CRect rtRoi, int nCurArea);
		static int CalcEdgeDefectRate(std::vector<cv::Point>& vPtrList, PI_Buff* Mask, int nMargin);

		static double Find_LineAngle(PI_Buff& srcImg, CRect rtROI, int direction, bool sobelReverse, int nLineNum, _Blob stBlob, POINT &ptDstPos, bool useMorph = false, int nMinBlob = 0, bool useLabeling = false, PI_Buff* dstBuff = NULL, int *pCnt = NULL);



		static float GetHeightGrad_Hynix_Tiny(PI_Buff* pClip3D, PI_Buff* pClip2D, PI_Buff* pClip2DMask, float fGV = 50, float GradY = 5);
		static float GetHeight_Hynix_Tiny(PI_Buff* pClip3D, PI_Buff* pClip2D, PI_Buff* pClip2DMask, float fGV = 50, float GradY = 5);
		static float GetHeight_Hynix_Tiny2(PI_Buff* pClip3D, PI_Buff* pClip2D, PI_Buff* pClip2DMask);
		static float GetHeight_Hynix_Tiny3(PI_Buff* pClip3D, PI_Buff* pClip2D, PI_Buff* pClip2DMask, float fHeight = 25, float GradXY = 15);
		static float GetHeight_Hynix_Tiny4(PI_Buff* pClip3D, PI_Buff* pClip2D, PI_Buff* pClip2DMask, float fHeight = 20, int nCount = 4);
		static float GetHeight_Hynix_Tiny5(PI_Buff* pClip3D, PI_Buff* pClip2D, PI_Buff* pClip2DMask, float fHeight = 10);

		static void SideImage2EdgeSegment(PI_Buff* pClip2D, PI_Buff* pDst);
		static int Image2MeanStdDev(cv::Mat src, float& fMean, float& fStdDev);

		static bool MaxMinAVGStdDev(std::vector<float>& vec, float* fMax, float* fMin, float* fAVG, float* fStdDev);
		static int GradiantFilter(PI_Buff* pimg, PI_Buff* pMask, PI_Buff* pDst, int blurSize = 51);
		static int GradiantFilter_Fast(PI_Buff* pimg, PI_Buff* pMask, PI_Buff* pDst, int blurSize = 51);
		
		static int GradiantDorrect(PI_Buff* pimg, PI_Buff* pMask, PI_Buff* pDst);
		static int GradiantFilter2(PI_Buff* pimg, PI_Buff* pMask, PI_Buff* pDst, int blurSize = 51);
		static double MeanOfTopKMasked(const cv::Mat& depth, const cv::Mat& mask, int K);
		static void GetGradiantBinImage(cv::Mat& img, cv::Mat& Mask, cv::Mat& Dst, int blurSize = 51, int nthresh = 200);
		static bool AnglePosChange(const int nAngle, const int nWidth, const int nHeight, const POINTF poSrcPoint, POINTF* poDstPoint);

protected: 

		//Covnert Point
		static bool CT2LTPosChange(const int nWidth, const int nHeight, const POINTF poSrcPoint, POINTF *poDstPoint);
	
		static void FillPolygon(PI_Buff* binImg, int width, int height, std::vector<POINT>& ptDstPoint, bool isExceptROI, PI_Buff* ucArrOverlapImg);
		static void FillRect(PI_Buff* binImg, int width, int height, std::vector<POINT>& ptDstPoint, bool isExceptROI, PI_Buff* ucArrOverlapImg);

		static int PolygonMask(PI_Buff *ucArrSrcImg, PI_Buff *ucArrOverlapImg,int nWidth, int nHeight, double dAngle, int nPolyNum, const POINTF *pPolygon, bool bConvertExceptROI = false);
		static int RectMask(PI_Buff *ucArrSrcImg, PI_Buff *ucArrOverlapImg, int nWidth, int nHeight, double dAngle,  int nRectNum, const RECT *pRect, bool bConvertExceptROI = true);
		static int RectMask(PI_Buff *ucArrSrcImg, PI_Buff *ucArrOverlapImg, int nWidth, int nHeight, double dAngle, int nRectNum, const std::vector<RECT> arrRect, bool bConvertExceptROI = true);

		/*static int FillOutOfInspAreaCombine(int nWidth, int nHeight, double dAngle, PI_Buff* src, PI_Buff* Overlap, _TotalInspExceptArea stTieArea);*/

		static bool IsExistDir(CString path);
		static void CreateDir(CString Path);
	};
}
