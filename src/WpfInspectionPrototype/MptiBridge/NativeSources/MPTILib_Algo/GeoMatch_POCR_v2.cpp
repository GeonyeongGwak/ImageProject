#include "stdafx.h"
#include "MPTI.h"
#include "GeoMatch_POCR_v2.h"

#include <algorithm>

#define _MAX_(_x, _y) (((_x) > (_y)) ? (_x) : (_y))
#define _MIN_(_x, _y) (((_x) < (_y)) ? (_x) : (_y))

#ifdef DEBUG
void GeoModel_PntsPOCR::drawPoints()
{
	if (_srcModelBlurred.empty() == false)
		cv::drawKeypoints(_srcModelBlurred, featurePoints, _srcModelWithPoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	else
		cv::drawKeypoints(_srcModelGray, featurePoints, _srcModelWithPoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
}
#endif

bool blurOn = true;

//Get OTSU threshold value of the size of the feature points
//특징점들의 크기에 대한 OTSU threshold 구하는 함수
int GeoModel_PntsPOCR::getKptsOTSU(Points &pts, int max, int &sum)
{
	if (pts.size() == 0)
		return 0;

	int OTSU(0);

	try
	{
		int *hist = new int[max + 1]{ 0 };
		int arrSize = pts.size();

		int sumB = 0, wB = 0;
		double maximum = 0;
		sum = 0;

		for (int i = 0; i < arrSize; i++)
		{
			hist[(int)pts[i].size]++;
		}

		for (int i = 0; i <= max; i++)
			sum += i * hist[i];

		for (int i = 0; i <= max; i++)
		{
			int wF = arrSize - wB;
			if (wB > 0 && wF > 0)
			{
				int mF = (sum - sumB) / wF;
				double val = wB * wF * ((double)sumB / wB - mF) * ((double)sumB / wB - mF);
				if (val >= maximum)
				{
					OTSU = i;
					maximum = val;
				}
			}
			wB += hist[i];
			sumB += i * hist[i];
		}
		delete[] hist;
	}
	catch (cv::Exception e)
	{
		throw (std::exception)e;
	}

	return OTSU;
}

//Get OTSU threshold value of the size of the feature points
int GeoModel_PntsPOCR::getKptsOTSU(Points &pts, int max)
{
	int temp(0);
	return getKptsOTSU(pts, max, temp);
}

//Detect feature points using SIFT algorithm
void GeoModel_PntsPOCR::_detect_SIFT(int thres)
{
	double thresS = 0, sigma = _MAX_(1.0, (double)thres / 14);
	int m = thres / 4 - 1, min = thres / 10 + 1;
	int resized = 0;
	// Create Feature extractor
	cv::Ptr<cv::Feature2D> sift = cv::SIFT::create(0, 9, 0.001, 10, sigma);
	Points p1;

	//
	//SIFT
	//
	sift->detect(_srcModelBlurred, p1);

	Deduplication(p1, 0);

	int p1Size = p1.size();
	cv::Mat temp = _srcModelBlurred.clone();

	if (p1Size < 2)
		resized = 1;

	//특징점 개수가 너무 적을 시, 폰트 크기와 SIFT 매개변수 변경해서 다시 추출. 폰트 크기 최대 8배까지 확대
	while (p1Size < 2 && resized <= 8)
	{
		sift.release();
		p1.clear();

		cv::resize(temp, temp, cv::Size(), 2, 2);
		resized *= 2;
		sift = cv::SIFT::create(0, 9, 0.001, 10, sigma * log(resized));
		sift->detect(temp, p1);

		Deduplication(p1, 0);
		p1Size = p1.size();
	}

	if (p1Size == 0)
	{
		sift.release();
		temp.release();
		return;
	}

	for (int i = 0; i < p1.size(); i++)
	{
		p1[i].class_id = ALGO_SIFT;
		if (resized > 0)
		{
			p1[i].size /= resized;
			p1[i].pt.x /= resized;
			p1[i].pt.y /= resized;

			if (p1[i].size < min)
				p1[i].size = min;
		}

		p1[i].size = (int)p1[i].size;
 		p1[i].pt = cv::Point2i(p1[i].pt);

		if (p1[i].size > m) p1[i].size = m;
	}

	int sum;

	thresS = getKptsOTSU(p1, m, sum);

	double mean = (double)sum / p1Size;
/*
	if (thresS < 0)
		thresS = mean;
	else if (thresS > mean * 2)
		thresS = (thresS + mean) / 2;
*/
	sizeThres = p1[p1Size - 1].size;

	Deduplication(p1, min);

	//특징점 영역이 폰트 이미지 범위를 벗어나지 않도록 조정
	for (int i = 0; i < p1.size() && (/*isDot || */featurePoints.size() < 10 || p1[i].size >= min); i++)
	{
		if (p1[i].size * 2 >= _srcModel.cols - 4)
		{
			p1[i].pt.x = (_srcModel.cols - 1) / 2;
			p1[i].size = p1[i].pt.x - 2;
		}
		else if (p1[i].pt.x - p1[i].size - 2 < 0)
			p1[i].pt.x = p1[i].size + 2;
		else if (p1[i].pt.x + p1[i].size + 2 > _srcModel.cols - 1)
			p1[i].pt.x = _srcModel.cols - p1[i].size - 3;

		if (p1[i].size * 2 >= _srcModel.rows - 4)
		{
			p1[i].pt.y = (_srcModel.rows - 1) / 2;
			p1[i].size = p1[i].pt.y - 2;
		}
		else if (p1[i].pt.y - p1[i].size - 2 < 0)
			p1[i].pt.y = p1[i].size + 2;
		else if (p1[i].pt.y + p1[i].size + 2 > _srcModel.rows - 1)
			p1[i].pt.y = _srcModel.rows - p1[i].size - 3;

		featurePoints.push_back(p1[i]);
	}

	sift.release();
	p1.clear();
}

//Detect feature points using AKAZE algorithm
void GeoModel_PntsPOCR::_detect_AKAZE(int thres)
{
	cv::Mat src;
	//AKAZE는 Mask 사이즈 문제로 특징점 검출 영역 외부 테두리에 추가 여백이 필요함
	cv::copyMakeBorder(_srcModelBlurred, src, 31, 31, 31, 31, cv::BORDER_CONSTANT, cv::Scalar::all(backColor));

	cv::Ptr<cv::Feature2D> akaze = cv::AKAZE::create(cv::AKAZE::DESCRIPTOR_MLDB, 0, 3, 0.001F, 5, 7);
	std::vector<cv::KeyPoint> p3;

	//
	//AKAZE
	//

	akaze->detect(src, p3);

	if (p3.size() == 0)
	{
		akaze.release();
		src.release();
		return;
	}

	int m = thres / 4, thresA;

	for (int i = 0; i < p3.size(); i++)
	{
		if (p3[i].size > m)
			p3[i].size = m;
		else if (p3[i].size < sizeThres)
			p3[i].size = sizeThres;
	}

	thresA = getKptsOTSU(p3, m);

	Deduplication(p3, _MIN_(thresA, std::log10(thres) + 1));

	Points pnts = featurePoints;

	// 특징점 영역이 이미지 밖으로 나가지 않도록 좌표 조정
	for (int i = 0; i < p3.size(); i++)
	{
		p3[i].pt -= cv::Point2f(31, 31); // 추가된 여백만큼 좌표 추가 조정

		p3[i].size = (int)p3[i].size;
		p3[i].pt = cv::Point2i(p3[i].pt);

		if (p3[i].size * 2 >= _srcModel.cols - 4)
		{
			p3[i].pt.x = (_srcModel.cols - 1) / 2;
			p3[i].size = p3[i].pt.x - 2;
		}
		else if (p3[i].pt.x - p3[i].size - 2 < 0)
			p3[i].pt.x = p3[i].size + 2;
		else if (p3[i].pt.x + p3[i].size + 2 > _srcModel.cols - 1)
			p3[i].pt.x = _srcModel.cols - p3[i].size - 3;

		if (p3[i].size * 2 >= _srcModel.rows - 4)
		{
			p3[i].pt.y = (_srcModel.rows - 1) / 2;
			p3[i].size = p3[i].pt.y - 2;
		}
		else if (p3[i].pt.y - p3[i].size - 2 < 0)
			p3[i].pt.y = p3[i].size + 2;
		else if (p3[i].pt.y + p3[i].size + 2 > _srcModel.rows - 1)
			p3[i].pt.y = _srcModel.rows - p3[i].size - 3;


		p3[i].class_id = ALGO_AKAZE;
		pnts.push_back(p3[i]);
	}

	int sizeSum(0);

	Deduplication(pnts, 0);
	int pNum = pnts.size();

	for (int i = 0; i < pNum; i++)
	{
		if (pnts[i].class_id == ALGO_AKAZE)
		{
			sizeSum += pnts[i].size;
		}
	}
	// 모든 AKAZE 특징점의 영역 너비의 합이 매우 작으면 AKAZE 특징점을 사용하지 않음.
	if (sizeSum >= sizeThres * 2 || pnts[pNum - 1].size > 4)
		featurePoints = pnts;

	akaze.release();
	p3.clear();
	src.release();
}

void GeoModel_PntsPOCR::deleteBackAndFore(Points &pts)
{
	deleteImgPnt(pts, foreColor);
	deleteImgPnt(pts, backColor);
}

void GeoModel_PntsPOCR::deleteImgPnt(Points &pts, uchar val)
{
	int num = pts.size();
	cv::Mat *tmp = val == backColor ? &backSearchModel : &foreSearchModel;

	for (int i = 0; i < num; i++)
	{
		cv::KeyPoint kpt = pts[i];
		int x = kpt.pt.x;
		int y = kpt.pt.y;
		int size = kpt.size;

		tmp->rowRange(y - size > 0 ? y - size : 0, y + size + 1 < tmp->rows ? y + size + 1 : tmp->rows).col(x).setTo(val);
		tmp->row(y).colRange(x - size > 0 ? x - size : 0, x + size + 1 < tmp->cols ? x + size + 1 : tmp->cols).setTo(val);
	}
}

void GeoModel_PntsPOCR::calcEmptySpace(cv::Mat &img, cv::Mat &output, int searchVal)
{
	int cols = img.cols, rows = img.rows;
	uchar *srcPtr = img.data;
	long *tmpPtr = (long*)output.data;

	for (int x = 0; x < cols; x++)
	{
		int k(0);
		int _idx = x;
		for (int y = 0; y < rows; y++)
		{
			int idx = _idx + y * cols;
			int idxN = idx + cols;

			if (srcPtr[idx] != searchVal)
			{
				k++;
				if (y == rows - 1 || srcPtr[idxN] == searchVal)
				{
					int i = 0;
					while (k > 0)
					{
						tmpPtr[idx - i * cols] = _MIN_(i + 1, k);
						i++;
						k--;
					}
				}
			}
			else
			{
				k = 0;
				tmpPtr[idx] = 0;
			}
		}
	}
	for (int y = 0; y < rows; y++)
	{
		int k(0);
		int _idx = y * cols;
		for (int x = 0; x < cols; x++)
		{
			int idx = _idx + x;
			int idxN = idx + 1;

			if (srcPtr[idx] != searchVal)
			{
				k++;
				if (x == cols - 1 || srcPtr[idxN] == searchVal)
				{
					int i = 0;
					while (k > 0)
					{
						tmpPtr[idx - i] += _MIN_(i + 1, k);
						i++;
						k--;
					}
				}
			}
			else
				k = 0;
		}
	}

}

int GeoModel_PntsPOCR::searchEmptySpace(cv::Mat &img, int searchVal, int cnt, bool isBack)
{
	int rows = _srcModel.rows - 4, cols = _srcModel.cols - 4;
	int min = _MAX_(rows, cols) / 4;
	if (min < 5) min = 5;

	cv::Mat temp(rows, cols, CV_32SC1, cv::Scalar::all(0));
	long *tmpPtr = temp.ptr<long>(0);

	double max;
	cv::Point maxLoc;
	int _cnt = 0, flag;
	/*
		int *hist = new int[(rows + cols) / 2]{ 0 };
		for (int i = 0; i < rows * cols; i++) {
			hist[(int)tmpPtr[i]]++;
		}

		double sum = 0;
		for (int i = 0; i < (rows + cols) / 2; i++) {
			sum += i * hist[i];
		}
		double sumB = 0, wB = 0;
		double maximum = 0;

		for (int i = 0; i < (rows + cols) / 2; i++) {
			double wF = cols * rows - wB;
			if (wB > 0 && wF > 0) {
				double mF = (sum - sumB) / wF;
				double val = wB * wF * (sumB / wB - mF) * (sumB / wB - mF);
				if (val >= maximum) {
					flag = i;
					maximum = val;
				}
			}
			wB += hist[i];
			sumB += i * hist[i];
		}

		delete[] hist;
	*/
	int sizeT = sizeThres;
	int halfSize = sizeT / 2;
	int bound = sizeT * 4 / 3;
	int Max(0);
	cv::Rect roi(2, 2, cols, rows);
	int _rows = rows + 4, _cols = cols + 4;
	if (cnt <= 0)
		cnt = 3;
	else if (cnt > 6)
		cnt = 6;
	while (_cnt < cnt)
	{
		deleteImgPnt(featurePoints, searchVal);
		calcEmptySpace(isBack ? backSearchModel(roi) : foreSearchModel(roi), temp, searchVal);

		cv::minMaxLoc(temp, NULL, &max, NULL, &maxLoc);

		if (max < sizeT || max < min)
			break;

		int _x = maxLoc.x + 2, _y = maxLoc.y + 2;

		if (isBack)
			backSearchModel.rowRange(_y - 2, _y + 3).colRange(_x - 2, _x + 3).setTo(backColor);
		else
			foreSearchModel.rowRange(_y - 2, _y + 3).colRange(_x - 2, _x + 3).setTo(foreColor);

		if (_x - sizeT < 2 || _y - sizeT < 2)
		{
			_x = _MAX_(_x, sizeT + 2);
			_y = _MAX_(_y, sizeT + 2);
		}
		if (_x + sizeT > _cols - 3 || _y + sizeT > _rows - 3)
		{
			_x = _MIN_(_x, _cols - sizeT - 3);
			_y = _MIN_(_y, _rows - sizeT - 3);
		}

		cv::KeyPoint pt(cv::Point2f(_x, _y), sizeT);
		pt.class_id = ALGO_ADDITIONAL;

		featurePoints.push_back(pt);
		_cnt++;
	}

	if (_cnt > 0)
		Deduplication(featurePoints, 0);

	return _cnt;
}

int GeoModel_PntsPOCR::addToForeGround(int Cnt)
{
	return searchEmptySpace(backSearchModel, backColor, Cnt, true);
}

int GeoModel_PntsPOCR::addToBackGround(int Cnt)
{
	return searchEmptySpace(foreSearchModel, foreColor, Cnt, false);
}

// 메인 특징점 검출 함수
void GeoModel_PntsPOCR::Detect(TCHAR ch, GeoModel_PntsPOCR::AlgoFlag algoflag)
{
	int sizeT = _MIN_(_srcModel.rows - 4, _srcModel.cols - 4);
	int maxSize = _MAX_(_srcModel.rows - 4, _srcModel.cols - 4) / 2;
	int tmpSizeT = sizeT / 4;

	// 폰트가 너무 작으면 검출하지 않음. 검사도 다른 방식으로 진행됨.
	if (sizeT <= 6)
	{
		featurePoints.clear();
		UniqueFPointsAdded = false;
		return;
	}

	_Ch = ch;

	double ratio = (double)(_srcModel.rows - 4) / (_srcModel.cols - 4);
	if (ratio >= 2.5 || ratio <= 0.4)
		sizeT *= 2;

	featurePoints.clear();
	UniqueFPointsAdded = false;

	if (algoflag != AKAZE_ONLY)
		_detect_SIFT(sizeT);

	int siftCnt = featurePoints.size();
	if (siftCnt > 0)
	{
	sizeThres = featurePoints[siftCnt - 1].size;
	if (sizeThres > tmpSizeT)
		sizeThres = tmpSizeT;
	if (!isSmallFont && sizeThres < 3)
		sizeThres = 3;
	else if (isSmallFont && sizeThres < 1)
		sizeThres = 1;
	}
	else
		sizeThres = 3;

#ifdef DEBUG
	drawPoints();
#endif

	if (algoflag != SIFT_ONLY)
		_detect_AKAZE(sizeT);

#ifdef DEBUG
	drawPoints();
#endif
	//배경 여백 특징점 검출
	int backCnt = addToBackGround(maxSize / sizeThres);

#ifdef DEBUG
	drawPoints();
#endif
	//문자 위 여백에 특징점 검출
	backCnt += addToForeGround(maxSize / sizeThres);

#ifdef DEBUG
	drawPoints();
#endif
	
	int num = featurePoints.size();
	
	for (int i = 0; i < num; i++)
	{
		featurePoints[i].octave = _srcModel.at<uchar>(featurePoints[i].pt) == foreColor ? TRUE : FALSE;
	}

}

bool GeoModel_PntsPOCR::SaveFile_Pnts(CArchive &ar, double resX, double resY)
{
	GeoModel::setRes(resX, resY);
	GeoModel::SaveFile(ar, false);

	int version(0);
	ar << version;

	if (_srcModelGray.empty())
	{
		ar << (int)0;
		ar << (int)0;
		ar << (int)0;
		ar << (double)0;
		ar << (uchar)0;
		ar << (uchar)0;
		ar << (TCHAR)0;
	}
	else
	{
		ar << _srcModelGray.rows;
		ar << _srcModelGray.cols;
		ar << srcGrayMean;
		ar << sizeThres;
		ar << colorThres;
		ar << backColor;
		ar << _Ch;

		int size = _srcModelGray.rows * _srcModelGray.cols;
		uchar * pSrcGray = _srcModelGray.data;
		for (int i = 0; i < size; i++)
		{
			ar << pSrcGray[i];
		}
	}

	int byteSz(featurePoints.size());
	ar << byteSz;

	// for isDot option
	ar << isDot;

	for (int i = 0; i < featurePoints.size(); i++)
	{
		ar << featurePoints[i].pt.x
			<< featurePoints[i].pt.y
			<< featurePoints[i].size
			<< featurePoints[i].class_id/*
			<< featurePoints[i].octave*/;
	}

	return true;
}

bool GeoModel_PntsPOCR::LoadFile_Pnts(CArchive &ar, double resX, double resY)
{
	bool ret = LoadFile(ar, resX, resY);

	if (ar.IsBufferEmpty())
		return false;

	featurePoints.clear();
	UniqueFPointsAdded = false;
	if (ret == false)
		return false;
	int version(-1);
	ar >> version;

	if (version < 0 || version > 1)
		ret = false;

	int Wid(0), Len(0);

	ar >> Len;
	ar >> Wid;
	ar >> srcGrayMean;
	ar >> sizeThres;
	ar >> colorThres;
	ar >> backColor;
	ar >> _Ch;

	if (_Ch > 122 || _Ch < 48 || (_Ch > 57 && _Ch < 65) || (_Ch > 90 && _Ch < 97))
		return false;
	if (Wid > 0 && Len > 0)
	{
		_srcModelGray = cv::Mat(Len, Wid, CV_8UC1);
		int min(_MIN_(Len, Wid));
		if (min < 30)
		{
			isSmallFont = true;
			if (min <= 10)
				UniqueFPointsAdded = true;
		}
		else
			isSmallFont = false;

		int size = _srcModelGray.rows * _srcModelGray.cols;
		uchar * pSrcGray = _srcModelGray.data;
		for (int i = 0; i < size; i++)
		{
			ar >> pSrcGray[i];
		}
		reSizeModelImgs(_srcModelGray, resX, resY);
	}
	else
		ret = false;

	int byteSz(0);
	ar >> byteSz;


	ar >> isDot;
	if(byteSz> Wid*Len)
		return false;
	for (int i = 0; i < byteSz; i++)
	{
		if (ar.IsBufferEmpty())
			break;
		cv::KeyPoint point;
		ar >> point.pt.x
			>> point.pt.y
			>> point.size
			>> point.class_id/*
			>> point.octave*/;

		featurePoints.push_back(point);
	}

	if (!ret)
		return false;

	PushAdditionalPoints();

	return true;
}

// 문자 고유 특징점 추가 및 중복 제거
void GeoModel_PntsPOCR::pushAdditionalPoints(Points &additionalPoints)
{
	int num = additionalPoints.size();
	
	if(isSmallFont)
		featurePoints.clear();

	Points tmpList;

	for (int i = 0; i < num; i++)
	{
		cv::KeyPoint kpt = additionalPoints[i];
		kpt.class_id = ALGO_CUSTOM;
		kpt.octave = FALSE;
		tmpList.push_back(kpt);
	}

	tmpList.insert(tmpList.end(), featurePoints.begin(), featurePoints.end());
	featurePoints = tmpList;

	Deduplication(featurePoints, 0);
}

//실제 모델 폰트 크기에 맞게 고유 특징점 크기, 좌표 조정
void GeoModel_PntsPOCR::resizeAdditionalPoints(Points &pnts, int width, int height, int newWidth, int newHeight)
{
	int num = pnts.size();
	double widthRatio = (double)(newWidth - 4) / (width - 4);
	double heightRatio = (double)(newHeight - 4) / (height - 4);
	double sizeRatio = _MIN_(widthRatio, heightRatio);

	for (int i = 0; i < num; i++)
	{
		int x = int((pnts[i].pt.x - 2) * widthRatio) + 2;
		int y = int((pnts[i].pt.y - 2) * heightRatio) + 2;
		int size = int(pnts[i].size * sizeRatio + 0.5);

		if (!isSmallFont)
			size = _MAX_(size, 3);
		else
			size = _MAX_(size, 1);

		if (size * 2 >= newWidth - 4)
		{
			x = (newWidth - 1) / 2;
			size = x - 2;
		}
		else if (x - size < 2)
		{
			x = size + 2;
		}
		else if (x + size > newWidth - 3)
		{
			x = newWidth - size - 3;
		}

		if (size * 2 >= newHeight - 4)
		{
			y = (newHeight - 1) / 2;
			size = y - 2;
		}
		else if (y - size < 2)
		{
			y = size + 2;
		}
		else if (y + size > newHeight - 3)
		{
			y = newHeight - size - 3;
		}

		pnts[i].pt.x = x;
		pnts[i].pt.y = y;
		pnts[i].size = size;
	}
}

// 문자 고유 특징점 추가 함수
void GeoModel_PntsPOCR::PushAdditionalPoints()
{
	if (UniqueFPointsAdded)
		return;

	int idx = getIndex(_Ch);
	if (idx < 0 || idx > 61)
		return;

	if (isSmallFont)
		idx += 62;

	Points additionalPoints = g_pMPTI->m_arrUniqueFPoints[idx];

	if (additionalPoints.size() == 0)
	{
		UniqueFPointsAdded = false;
		return;
	}

	resizeAdditionalPoints(additionalPoints, g_pMPTI->m_nUniqueFPointWidth, g_pMPTI->m_nUniqueFPointHeight, _srcModelGray.cols, _srcModelGray.rows);
	pushAdditionalPoints(additionalPoints);

	akazeCnt = 0;

	for (int i = 0; i < featurePoints.size(); i++)
		if (featurePoints[i].class_id == ALGO_AKAZE)
			akazeCnt++;

	UniqueFPointsAdded = true;

#ifdef DEBUG
	drawPoints();
	int a(0);
#endif
}

bool GeoModel_PntsPOCR::KeyCompare(cv::KeyPoint &p1, cv::KeyPoint &p2)
{
	return p1.size > p2.size;
}

// 겹치는 특징점 제거, size가 threshold 값보다 작은 특징점 제거
void GeoModel_PntsPOCR::Deduplication(Points &points, double threshold)
{
	if (points.size() < 2) return;

	std::sort(points.begin(), points.end(), GeoModel_PntsPOCR::KeyCompare);

	int minGap = sizeThres > 1 ? sizeThres : 1;
	for (int i = 0; i < points.size() - 1; i++)
	{
		if (i == 0)
		{
			if (points[0].size < threshold)
			{
				points.erase(points.begin());
				i--;
				continue;
			}
		}
		for (int j = i + 1; j < points.size(); j++)
		{
			if (i == 0 && points[j].size < threshold)
			{
				points.erase(points.begin() + j);
				j--;
				continue;
			}

			/*if (!isSmallFont) {
				if (points[j].pt.x >= points[i].pt.x - points[i].size
					&& points[j].pt.x <= points[i].pt.x + points[i].size
					&& points[j].pt.y >= points[i].pt.y - points[i].size
					&& points[j].pt.y <= points[i].pt.y + points[i].size) {
					points.erase(points.begin() + j);
					j--;
				}
			}
			else */
			if(points[j].class_id != ALGO_CUSTOM){
				cv::Point2f pDist = points[i].pt - points[j].pt;
				double dist = pDist.x * pDist.x + pDist.y * pDist.y;
				double sizeSum = points[i].size + points[j].size - minGap;
				//상대적으로 크기가 큰 점을 기준으로 영역이 겹치는 작은 특징점을 제거
				if (dist < sizeSum * sizeSum)
				{
					points.erase(points.begin() + j);
					j--;
				}
			}
		}
	}
}

//GeoModel::Alloc함수를 대신해서 사용해야 함
void GeoModel_PntsPOCR::_modifiedAlloc(cv::Mat src, cv::Mat grayImg, int forVal, int backVal, float fImgAngle)
{
	int len = (_MIN_(src.cols, src.rows) / 30) * 2 + 1; // GaussianBlur Mask size

	if (len < 3)
	{
		len = 3;
		isSmallFont = true;
	}
	else
		isSmallFont = false;

	isDot = false;	// 사용 안함

	GeoModel::Alloc(src, fImgAngle);

	backSearchModel = src.clone();
	foreSearchModel = src.clone();

	if (grayImg.channels() == 3)
		cv::cvtColor(grayImg, _srcModelGray, cv::COLOR_BGR2GRAY);
	else
		_srcModelGray = grayImg.clone();

	cv::GaussianBlur(_srcModel, _srcModelBlurred, cv::Size(len, len), 3.5, 3.5, cv::BorderTypes::BORDER_REPLICATE);
	if (blurOn)
		cv::GaussianBlur(_srcModelGray, _srcModelGray, cv::Size(len, len), 3.5, 3.5, cv::BorderTypes::BORDER_REPLICATE);

	backColor = backVal;
	foreColor = forVal;

	colorThres = (backColor + foreColor) / 2;

	int thres(0);

	srcGrayMean = cv::mean(_srcModelGray)[0];
}

void GeoModel_PntsPOCR::copyTo(GeoModel_PntsPOCR &newModel)
{
	newModel.featurePoints = featurePoints;
	newModel.sizeThres = sizeThres;
	newModel.colorThres = colorThres;
}

// 장비 resol값에 맞춰 수정된 기존 POCR 에지모델 기반으로 모델 폰트 사이즈, 특징점 사이즈 및 좌표 조정
void GeoModel_PntsPOCR::reSizeModelPnt(std::shared_ptr<GeoModel_POCR> edge)
{
	if (edge == nullptr || edge->empty())
		return;

	cv::Size newsize(edge->Image().cols, edge->Image().rows);
	cv::Size oldsize(_srcModelGray.cols, _srcModelGray.rows);
	if (_srcModelGray.rows == newsize.height && _srcModelGray.cols == newsize.width)
		return;

	cv::resize(_srcModelGray, _srcModelGray, newsize);
	resizeAdditionalPoints(featurePoints, oldsize.width, oldsize.height, newsize.width, newsize.height);

	if (newsize.height < 30 || newsize.width < 30)
	{
		isSmallFont = true;
		PushAdditionalPoints();
	}
#ifdef DEBUG
	drawPoints();
#endif
}

void GeoModel_PntsPOCR::reSizeModelImgs(cv::Mat &target, double resX, double resY)
{
	if (resX == _resolX && resY == _resolY)
		return;
	if (resX == 0 || _resolX <= 0 || resY == 0 || _resolY <= 0)
		return;
	int nSizeX = (int)(target.cols*_resolX / resX);
	int nSizeY = (int)(target.rows*_resolY / resY);
	if (nSizeX == 0 || nSizeX <= 0 || nSizeX == target.cols || nSizeY == target.rows)
		return;
	cv::resize(target, target, cv::Size(nSizeX, nSizeY));
}
int GeoModel_PntsPOCR::getIndex(uchar ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'A' && ch <= 'Z')
		return ch - 'A' + 10;
	else if (ch >= 'a' && ch <= 'z')
		return ch - 'a' + 36;
	else
		return -1;
}
uchar GeoModel_PntsPOCR::getIdxChar(int idx)
{
	if (idx < 0 || idx >= 62)
		return 62;
	else if (idx < 10)
		return idx + '0';
	else if (idx < 36)
		return idx - 10 + 'A';
	else
		return idx - 36 + 'a';
}


//////////////////////////
//						//
//	GeoMatch_PntsPOCR	//
//						//
//////////////////////////

// 특징점 활용 POCR 검사
void GeoMatch_PntsPOCR::CalcDivScore(GeoModel_PntsPOCR & model, GeoResult_POCR & Result, cv::Point2i center, bool bIsSmallFont)
{
	cv::Mat Img = model.GrayImage();
	int len = (_MIN_(Img.cols, Img.rows) / 30) * 2 + 1;
	if (len < 3) len = 3;

	cv::Mat tmp;
	cv::copyMakeBorder(*_ProcImg->_OrgImage, tmp, 5, 5, 5, 5, cv::BorderTypes::BORDER_CONSTANT, model.getBackColor());

	GeoModel_PntsPOCR::Points points = model.featurePoints;
	int num = points.size();

	int centerY = Result._Center_x[0], centerX = Result._Center_y[0];

	if (centerY == 0 || centerX == 0)
	{
		centerX = center.x + _ProcImg->_OrgImage->cols / 2;
		centerY = center.y + _ProcImg->_OrgImage->rows / 2;
	}

	int width = Img.cols - 4, height = Img.rows - 4;
	cv::Mat img = Img(cv::Rect(2, 2, width, height));

	int tl_X = centerX - width / 2 + 5, tl_Y = centerY - height / 2 + 5;
	if (tl_X - 5 < 0)
		tl_X = 5;
	else if (tl_X + width + 5 > tmp.cols)
		tl_X = tmp.cols - 5 - width;

	if (tl_Y - 5 < 0)
		tl_Y = 5;
	else if (tl_Y + height + 5 > tmp.rows)
		tl_Y = tmp.rows - 5 - height;

	cv::Mat tmp2 = tmp(cv::Rect(tl_X - 3, tl_Y - 3, width + 6, height + 6));
	cv::Mat src;
	if (blurOn)
		cv::GaussianBlur(tmp2, src, cv::Size(len, len), 3.5, 3.5, cv::BorderTypes::BORDER_REPLICATE);

	
	cv::Mat tempSrc = src(cv::Rect(3, 3, width, height));

	int alpha = model.getMean(), beta = cv::mean(tempSrc)[0];
	int srcMean, imgMean = model.getColorThres();
	srcMean = imgMean - alpha + beta;

// 	int back = model.getBackColor();
// 	if (back < imgMean && back * 1.2 >= srcMean)
// 		srcMean = beta >= back * 1.2 ? beta : back * 1.2;
// 	else if (back > imgMean && back * 0.8 <= srcMean)
// 		srcMean = beta <= back * 0.8 ? beta : back * 0.8;

	if (srcMean > 255)
		srcMean = 255;
	else if (srcMean < 0)
		srcMean = 0;

	// 폰트 크기가 매우 작을 시, 특징점 영역이 너무 작기 때문에 폰트를 상하, 좌우로 2분할 하여 검사
	if (bIsSmallFont && num < 5)
	{
		int w = Img.cols / 3, h = Img.rows / 3;
		int W = src.cols / 3, H = src.rows / 3;
		cv::Rect roi, srcRoi;
		double score[6]{ 0 };

		for (int i = 0; i < 4; i++)
		{
			if (i < 2)
			{
				roi = cv::Rect(i * (w - 1), 0, 2 * w + 1, Img.rows);
				srcRoi = cv::Rect(i * (W - 1), 0, 2 * W + 1, src.rows);
			}
			else
			{
				roi = cv::Rect(0, (i - 2) * (h - 1), Img.cols, 2 * h + 1);
				srcRoi = cv::Rect(0, (i - 2) * (H - 1), src.cols, 2 * H + 1);
			}
			templateMatching(src(srcRoi), Img(roi), score, srcMean, imgMean);

			kptScores.push_back(::Node(ALGO_ADDITIONAL, FALSE, model.getSizeThres(), score));

		}

		w = Img.cols / 4; h = Img.rows / 4;
		W = src.cols / 4; H = src.rows / 4;
		roi = cv::Rect(w, h, Img.cols - w, Img.rows - h);
		srcRoi = cv::Rect(W, H, src.cols - W, src.rows - H);

		templateMatching(src(srcRoi), Img(roi), score, srcMean, imgMean);
		kptScores.push_back(::Node(ALGO_ADDITIONAL, FALSE, model.getSizeThres(), score));

#ifdef DEBUG
		std::vector<node> originalScores = kptScores;
#endif
		_CalcScore(model, Result);

		return;
	}

	double matched[25]{ 0 };

	// Object에서 정확한 모델의 위치 탐색 (5x5)
	for (int i = 0; i < num; i++)
	{
		cv::KeyPoint kpt = points[i];
		cv::Rect roi = cv::Rect(kpt.pt - cv::Point2f(kpt.size + 2, kpt.size + 2), kpt.pt + cv::Point2f(kpt.size - 1, kpt.size - 1));
		if (roi.x < 0)
			roi.x = 0;
		if (roi.y < 0)
			roi.y = 0;
		if (roi.x + roi.width > img.cols)
			roi.x = img.cols - roi.width;
		if (roi.y + roi.height > img.rows)
			roi.y = img.rows - roi.height;

		int tx = roi.tl().x + 1, ty = roi.tl().y + 1;

		cv::Rect srcRoi(cv::Point2i(tx, ty), cv::Point2i(4, 4) + cv::Point2i(tx + roi.width, ty + roi.height));
		double* tempPtr = _templateMatching(src(srcRoi), img(roi), srcMean, imgMean);

		for (int j = 0; j < 25; j++)
		{
			matched[j] += tempPtr[j];
		}

		delete[] tempPtr;
	}

	double max = -25.0;
	int _idx(0);

	for (int i = 0; i < 25; i++)
	{
		if (max < matched[i])
		{
			max = matched[i];
			_idx = i;
		}
	}

	int dx = _idx % 5 - 2, dy = _idx / 5 - 2;
	tmp2 = tmp(cv::Rect(tl_X - 3 + dx, tl_Y - 3 + dy, width + 6, height + 6));
	if (blurOn)
		cv::GaussianBlur(tmp2, src, cv::Size(len, len), 3.5, 3.5, cv::BorderTypes::BORDER_REPLICATE);

#ifdef DEBUG
	cv::Mat tempSrcImg = src(cv::Rect(2, 2, width + 2, height + 2));
#endif

	Result._Center_x[0] = centerY + dy;
	Result._Center_y[0] = centerX + dx;

	// 확정된 위치를 중심으로 각 특징점 영역별 Score 계산 (3x3, small font는 1x1)
	for (int i = 0; i < num; i++)
	{
		cv::KeyPoint kpt = points[i];
		cv::Rect roi = cv::Rect(kpt.pt - cv::Point2f(kpt.size + 2, kpt.size + 2), kpt.pt + cv::Point2f(kpt.size - 1, kpt.size - 1));
		if (roi.x < 0)
			roi.x = 0;
		if (roi.y < 0)
			roi.y = 0;
		if (roi.x + roi.width > img.cols)
			roi.x = img.cols - roi.width;
		if (roi.y + roi.height > img.rows)
			roi.y = img.rows - roi.height;

		int tx, ty;
		cv::Rect srcRoi;

		if (model.IsSmallFont())
		{
			tx = roi.tl().x + 3;
			ty = roi.tl().y + 3;
			srcRoi = cv::Rect(cv::Point2i(tx, ty), cv::Point2i(tx + roi.width, ty + roi.height));
		}
		else
		{
			tx = roi.tl().x + 2;
			ty = roi.tl().y + 2;
			srcRoi = cv::Rect(cv::Point2i(tx, ty), cv::Point2i(2, 2) + cv::Point2i(tx + roi.width, ty + roi.height));
		}

		cv::Mat temp;

		double score[6]{ 0 };
		templateMatching(src(srcRoi), img(roi), score, srcMean, imgMean);

		kptScores.push_back(::Node(kpt.class_id, kpt.octave, kpt.size, score));
	}

	// 특징점 검출이 잘 되지 않는 특정 문자들의 경우, 폰트 이미지 전체를 상하, 좌우로 2분할하여 추가 검사
	TCHAR ch = model.getModelChar();
	if (model.IsSmallFont() || ch == 'I' || ch == '1' || ch == 'l' || ch == 'i' || ch == 'X' || ch == 'x')
	{
		double w = Img.cols / 3.0, h = Img.rows / 4.0;
		double W = src.cols / 3.0, H = src.rows / 4.0;
		for (int i = 0; i < 4; i++)
		{
			cv::Rect roi, srcRoi;
			if (i < 2)
			{
				roi = cv::Rect(i * (2*w - 1), 0, w + 1, Img.rows);
				srcRoi = cv::Rect(i * (2*W - 1), 0, W + 1, src.rows);
			}
			else
			{
				roi = cv::Rect(0, (i - 2) * (3*h - 1), Img.cols, h + 1);
				srcRoi = cv::Rect(0, (i - 2) * (3*H - 1), src.cols, H + 1);
			}
			double score[6]{ 0 };
			templateMatching(src(srcRoi), Img(roi), score, srcMean, imgMean);

			kptScores.push_back(::Node(ALGO_ADDITIONAL, FALSE, model.getSizeThres(), score));
		}
	}
#ifdef DEBUG
	std::vector<node> originalScores = kptScores;
#endif
	_CalcScore(model, Result);
}

// 특징점 별로 Score가 계산된 후에 최종 POCR Score 계산
void GeoMatch_PntsPOCR::_CalcScore(GeoModel_PntsPOCR &model, GeoResult_POCR & result)
{
	int foreNum(0), foreNG(0);
	double scoreSum(0);
	if (!model.IsSmallFont())
	{
		int cnt = model.getAkazeCnt(), sizeSum(0);
		double sum(0), modelSqrSum(0), srcSqrSum(0), num(0), modelSum(0), srcSum(0);
		for (int i = kptScores.size() - 1; i > 0; i--)
		{
			//if (kptScores[i].octave == TRUE)
			//{
			//	scoreSum += kptScores[i].score;
			//	foreNum++;
			//	if (kptScores[i].score < 0)
			//		foreNG++;
			//}

			if (kptScores[i].type != ALGO_AKAZE) continue;
			if (num == 0 && kptScores[i].size >= 4) break;
			::Node node = kptScores[i];
			num++;

			sum += node.numerator;
			modelSqrSum += node.denominatorModel;
			srcSqrSum += node.denominatorSource;
			modelSum += node.numeratorModel;
			srcSum += node.numeratorSource;
			sizeSum += node.size;

			if (num == cnt)
			{
				if (modelSqrSum == 0 && srcSqrSum == 0)
					kptScores[i].score = 1;
				else if (modelSqrSum == 0)
					kptScores[i].score = srcSum / sqrt(sizeSum * sizeSum * 4 * srcSqrSum);
				else if (srcSqrSum == 0)
					kptScores[i].score = modelSum / sqrt(sizeSum * sizeSum * 4 * modelSqrSum);
				else
					kptScores[i].score = sum / sqrt(modelSqrSum * srcSqrSum);

				kptScores[i].size = model.getSizeThres();
				break;
			}
			else
			{
				kptScores.erase(kptScores.begin() + i);
				i--;
			}
		}
	}

	std::sort(kptScores.begin(), kptScores.end(), NodeCompare);

	//if (foreNG == 1 && kptScores[0].octave == TRUE && kptScores[0].score > -0.3)
	//	kptScores[0].score = scoreSum / foreNum;

	if (kptScores[0].size < model.getSizeThres() && kptScores[0].type != ALGO_CUSTOM)
		result._Equality[0] = (kptScores[1].score + kptScores[2].score) / 2.0 * 100.0;
	else
		result._Equality[0] = (kptScores[0].score + kptScores[1].score + kptScores[2].score) / 3.0 * 100.0;

	if (result._Equality[0] < 0) result._Equality[0] = 0;
}


// 기존의 템플릿 매칭에서 각 이미지의 평균값 대신 입력된 매개변수의 값을 기준으로 매칭
//result : Must be double array which length is at least 6.
void GeoMatch_PntsPOCR::templateMatching(cv::Mat &src, cv::Mat &tmplt, double* result, int srcThres, int tmpltThres)
{
	int rows = tmplt.rows, cols = tmplt.cols;
	int srcRows = src.rows, srcCols = src.cols;
	int height = srcRows - rows + 1;
	int width = srcCols - cols + 1;
	int idx = (1 + cols) * (1 + rows) - 1;
	int size = rows * cols;

	cv::Mat tempSrc, tempTmplt;
	tmplt.convertTo(tempTmplt, CV_64FC1);
	src.convertTo(tempSrc, CV_64FC1);

	tempTmplt -= tmpltThres;
	tempSrc -= srcThres;

	cv::Mat imgSum, imgSqrSum;
	cv::integral(tempTmplt, imgSum, imgSqrSum);

	cv::Mat srcSum, srcSqrSum, mul, mulSum;
	cv::integral(tempSrc, srcSum, srcSqrSum);

	double imgSqrSumVal = ((double*)imgSqrSum.data)[idx];
	double imgSumVal = ((double*)imgSum.data)[idx];

	double *srcSqrSumPtr = ((double*)srcSqrSum.data);
	double *srcSumPtr = ((double*)srcSum.data);

	result[0] = -1.0;

	int H = rows * (srcCols + 1);
	int srcIdx = H + cols;

	double preCalcScore = imgSumVal / sqrt(size * imgSqrSumVal);
	for (int i = 0; i < height; i++)
	{
		int _idx = i * (srcCols + 1);
		for (int j = 0; j < width; j++)
		{
			int __idx = _idx + j;

			cv::Rect roi(j, i, cols, rows);
			cv::Mat srcTemplate = tempSrc(roi);

			double srcSqrSumVal = srcSqrSumPtr[__idx + srcIdx] - srcSqrSumPtr[__idx + H] - srcSqrSumPtr[__idx + cols] + srcSqrSumPtr[__idx];
			double srcSumVal = srcSumPtr[__idx + srcIdx] - srcSumPtr[__idx + H] - srcSumPtr[__idx + cols] + srcSumPtr[__idx];

			cv::multiply(srcTemplate, tempTmplt, mul);
			cv::integral(mul, mulSum);

			double mulSumVal = ((double*)mulSum.data)[idx];
			double score(0);

			if (imgSqrSumVal == 0 && srcSqrSumVal == 0)
				score = 1;
			else if (imgSqrSumVal == 0)
				score = srcSumVal / sqrt(size * srcSqrSumVal);
			else if (srcSqrSumVal == 0)
				score = preCalcScore;
			else
				score = mulSumVal / sqrt(imgSqrSumVal * srcSqrSumVal);

			if (score > result[0])
			{
				result[0] = score;
				result[1] = mulSumVal;
				result[2] = imgSqrSumVal;
				result[3] = srcSqrSumVal;
				result[4] = imgSumVal;
				result[5] = srcSumVal;
			}
		}
	}
}

// return value: Same with opencv matchTemplate -> outputarray output.
//				 The return type is not cv::Mat but double*.
double* GeoMatch_PntsPOCR::_templateMatching(cv::Mat &src, cv::Mat &tmplt, int srcThres, int tmpltThres)
{
	int rows = tmplt.rows, cols = tmplt.cols;
	int srcRows = src.rows, srcCols = src.cols;
	int height = srcRows - rows + 1;
	int width = srcCols - cols + 1;
	int idx = (1 + cols) * (1 + rows) - 1;
	int size = rows * cols;

	double *resultPtr = new double[height * width]; // shared_ptr로 전환할 것

	cv::Mat tempSrc, tempTmplt;
	tmplt.convertTo(tempTmplt, CV_64FC1);
	src.convertTo(tempSrc, CV_64FC1);

	tempTmplt -= tmpltThres;
	tempSrc -= srcThres;

	cv::Mat imgSum, imgSqrSum;
	cv::integral(tempTmplt, imgSum, imgSqrSum);

	cv::Mat srcSum, srcSqrSum, mul, mulSum;
	cv::integral(tempSrc, srcSum, srcSqrSum);

	double imgSqrSumVal = ((double*)imgSqrSum.data)[idx];
	double imgSumVal = ((double*)imgSum.data)[idx];

	double *srcSqrSumPtr = ((double*)srcSqrSum.data);
	double *srcSumPtr = ((double*)srcSum.data);

	int H = rows * (srcCols + 1);
	int srcIdx = H + cols;

	double preCalcScore = imgSumVal / sqrt(size * imgSqrSumVal);
	for (int i = 0; i < height; i++)
	{
		int _idx = i * (srcCols + 1);
		for (int j = 0; j < width; j++)
		{
			int __idx = _idx + j;

			cv::Rect roi(j, i, cols, rows);
			cv::Mat srcTemplate = tempSrc(roi);

			double srcSqrSumVal = srcSqrSumPtr[__idx + srcIdx] - srcSqrSumPtr[__idx + H] - srcSqrSumPtr[__idx + cols] + srcSqrSumPtr[__idx];
			double srcSumVal = srcSumPtr[__idx + srcIdx] - srcSumPtr[__idx + H] - srcSumPtr[__idx + cols] + srcSumPtr[__idx];

			cv::multiply(srcTemplate, tempTmplt, mul);
			cv::integral(mul, mulSum);

			double mulSumVal = ((double*)mulSum.data)[idx];
			double score(0);

			if (imgSqrSumVal == 0 && srcSqrSumVal == 0)
				score = 1;
			else if (imgSqrSumVal == 0)
				score = srcSumVal / sqrt(size * srcSqrSumVal);
			else if (srcSqrSumVal == 0)
				score = preCalcScore;
			else
				score = mulSumVal / sqrt(imgSqrSumVal * srcSqrSumVal);

			resultPtr[i*height + j] = score;
		}
	}
	return resultPtr;
}

// src와 tmplt 이미지의 외각부분(border)에 대해 템플릿 매칭 실행
void GeoMatch_PntsPOCR::borderMatching(cv::Mat &src, cv::Mat &tmplt, double* result, int srcThres, int tmpltThres, int borderSize)
{
	int height = src.rows - tmplt.rows + 1;
	int width = src.cols - tmplt.cols + 1;
	int rows = tmplt.rows, cols = tmplt.cols;
	int idx = (1 + cols) * (1 + rows) - 1;
	cv::Rect interRoi(borderSize, borderSize, cols - borderSize * 2, rows - borderSize * 2);

	cv::Mat Src, tempTmplt;
	tmplt.convertTo(tempTmplt, CV_64FC1);
	src.convertTo(Src, CV_64FC1);

	tempTmplt -= tmpltThres;
	Src -= srcThres;

	tempTmplt(interRoi).setTo(0);

	cv::Mat imgSum, imgSqrSum;
	cv::integral(tempTmplt, imgSum, imgSqrSum);

	double imgSqrSumVal = imgSqrSum.ptr<double>(0)[idx];
	double imgSumVal = imgSum.ptr<double>(0)[idx];

	result[0] = -1.0;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			cv::Rect roi(j, i, cols, rows);
			cv::Mat tempSrc = Src(roi).clone();

			tempSrc(interRoi).setTo(0);

			cv::Mat srcSum, srcSqrSum, mul, mulSum;
			cv::integral(tempSrc, srcSum, srcSqrSum);

			cv::multiply(tempSrc, tempTmplt, mul);
			cv::integral(mul, mulSum);

			double srcSqrSumVal = srcSqrSum.ptr<double>(0)[idx];
			double mulSumVal = mulSum.ptr<double>(0)[idx];
			double srcSumVal = srcSum.ptr<double>(0)[idx];
			double score = mulSumVal / sqrt(imgSqrSumVal * srcSqrSumVal);

			if (imgSqrSumVal == 0 && srcSqrSumVal == 0)
				score = 1;
			else if (imgSqrSumVal == 0)
				score = srcSumVal / sqrt((rows * cols) * srcSqrSumVal);
			else if (srcSqrSumVal == 0)
				score = imgSumVal / sqrt((rows * cols) * imgSqrSumVal);

			if (score > result[0])
			{
				result[0] = score;
				result[1] = mulSumVal;
				result[2] = imgSqrSumVal;
				result[3] = srcSqrSumVal;
				result[4] = imgSumVal;
				result[5] = srcSumVal;
			}
		}
	}
}


#undef _MAX_(X, Y)
#undef _MIN_(X, Y)