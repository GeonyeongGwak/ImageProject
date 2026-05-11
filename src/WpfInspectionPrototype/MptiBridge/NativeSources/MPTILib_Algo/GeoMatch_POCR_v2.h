#pragma once

#include <afx.h>

//#include "geomatch.h"
//#include "OCV/OCVParamDef.h"
#include "GeoMatch_OCV.h"

typedef enum { ALGO_SIFT = 0, ALGO_AKAZE = 1, ALGO_ADDITIONAL = 2, ALGO_CUSTOM = 3 } AlgoType;

typedef struct node
{
	AlgoType type;
	double score;
	int size;
	int octave;
	double numerator; // Score (NCC) = numerator / sqrt(denominatorModel * denominatorSource)
	double denominatorModel;
	double denominatorSource;
	double numeratorModel;
	double numeratorSource;

	node()
	{
		type = ALGO_SIFT;
		octave = FALSE;
		size = 0;
		score = -1.0;
		numerator = 0;
		denominatorModel = 0;
		denominatorSource = 0;
		numeratorModel = 0;
		numeratorSource = 0;
	}

	node(int t, int o, int s, double* arr)
	{
		type = (AlgoType)t;
		octave = o;
		size = s;
		score = arr[0];
		numerator = arr[1];
		denominatorModel = arr[2];
		denominatorSource = arr[3];
		numeratorModel = arr[4];
		numeratorSource = arr[5];
	}
} Node;

class GeoModel_PntsPOCR : public GeoModel_POCR
{
public:
	GeoModel_PntsPOCR(void) :UniqueFPointsAdded(false) {}
	typedef std::vector<cv::KeyPoint> Points;

	// pt: position of point
	// size: size of point
	// class_id: algorithm type of point
	// octave: foreground point = TRUE(1), background point = FALSE(0)
	Points featurePoints;
	
private:
	cv::Mat _srcModelBlurred;
	cv::Mat _srcModelGray;
	cv::Mat backSearchModel;
	cv::Mat foreSearchModel;

	TCHAR _Ch;
	uchar backColor;
	uchar foreColor;
	double sizeThres;
	uchar colorThres;
	int srcGrayMean;
	int akazeCnt;
	bool UniqueFPointsAdded;
	bool isSmallFont;
	bool isDot;// jskim / 현재 사용되고 있지 않는 변수임 (21.09.03)

#ifdef DEBUG
	cv::Mat _srcModelWithPoints;

	void drawPoints();
#endif

protected:
	void _detect_SIFT(int thres);
	void _detect_AKAZE(int thres);
	void deleteBackAndFore(Points &pts);
	void deleteImgPnt(Points &pts, uchar val);
	int addToForeGround(int Cnt = 0);
	int addToBackGround(int Cnt = 0);
	void calcEmptySpace(cv::Mat &img, cv::Mat &output, int searchVal);
	int searchEmptySpace(cv::Mat &img, int searchVal, int cnt, bool isBack = true);
	int getKptsOTSU(Points &pts, int max, int &sum);
	int getKptsOTSU(Points &pts, int max);

	void pushAdditionalPoints(Points &additionalPoints);
	void resizeAdditionalPoints(Points &pnts, int width, int height, int newWidth, int newHeight);

public:
	static bool KeyCompare(cv::KeyPoint &p1, cv::KeyPoint &p2);
	typedef enum { SIFT_ONLY = 0, SIFT_AND_AKAZE = 1, AKAZE_ONLY = 2 } AlgoFlag;

	double getSizeThres() { return sizeThres; }
	int getMean() { return srcGrayMean; }
	void setMean(int val) { srcGrayMean = val; }
	int getAkazeCnt() { return akazeCnt; }
	void setAkazeCnt(int val) { akazeCnt = val; }
	uchar getColorThres() { return colorThres; }
	void setColorThres(uchar val) {  if(val > 0) colorThres = val; }
	uchar getBackColor() { return backColor; }
	cv::Mat GrayImage() { return _srcModelGray; }
	bool GrayEmpty() { return _srcModelGray.empty(); }
	bool IsSmallFont() { return isSmallFont; }
	TCHAR getModelChar() { return _Ch; }

	bool isUniqueFPointsAdded() { return UniqueFPointsAdded; }
	void setUniqueFPointsAdded(bool opt) { UniqueFPointsAdded = opt; }
	void PushAdditionalPoints();

	bool SaveFile_Pnts(CArchive & ar, double resX, double resY);
	bool LoadFile_Pnts(CArchive & ar, double resX, double resY);
	void Deduplication(Points &points, double threshold);
	void Detect(TCHAR ch, AlgoFlag algoflag = SIFT_AND_AKAZE);
	void _modifiedAlloc(cv::Mat src, cv::Mat grayImg,int forVal, int backVal, float fImgAngle = 0.0f);
	void copyTo(GeoModel_PntsPOCR &newModel);

	void reSizeModelPnt(std::shared_ptr<GeoModel_POCR> edge);
	void reSizeModelImgs(cv::Mat &target, double resX, double resY);

	static int getIndex(uchar ch);
	static uchar getIdxChar(int idx);
};

class GeoMatch_PntsPOCR : public GeoMatch_POCR
{
private:
	std::vector<::Node> kptScores;

public:
	static bool NodeCompare(::Node &n1, ::Node &n2) { return n1.score < n2.score; }


	void CalcDivScore(GeoModel_PntsPOCR & model, GeoResult_POCR & Result, cv::Point2i center = cv::Point2i(0), bool bIsSmallFont = false);
	void _CalcScore(GeoModel_PntsPOCR &model, GeoResult_POCR & Result);
	void templateMatching(cv::Mat &src, cv::Mat &tmplt, double *result, int srcThres, int tmpltThres);
	void borderMatching(cv::Mat &src, cv::Mat &tmplt, double* result, int srcThres, int tmpltThres, int borderSize = 2);
	double* _templateMatching(cv::Mat &src, cv::Mat &tmplt, int srcThres, int tmpltThres);
};

