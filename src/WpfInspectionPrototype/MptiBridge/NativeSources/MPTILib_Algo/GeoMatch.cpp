#include "StdAfx.h"
#include "GeoMatch.h"

#include <opencv2/opencv.hpp>
#include <ppl.h>
#include "LeastSquare.h"
#include <set>
#include <math.h>
#include "MemoryManager.h"
#include "ExtInspSub.h"
#include "ExtLog.h"
//#ifdef _DEBUG
//
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

// #ifndef RounDF
// 	#define RounDF(x) roundl(x)
// #endif


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


#ifdef _DEBUG
//	#define GeoSaveDebug		1
#endif


#ifndef GeoNoUseWriteLog
#include "MPTI.h"
#endif

#ifndef GeoNoUseWriteTimeLog
#include "QTimer.h"
#endif

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

void GeoWriteLog(CString sLog)
{
#ifndef GeoNoUseWriteLog
	if (g_pMPTI)
		g_pMPTI->AddLog_Dev(sLog);
#endif
}


GeoModel::Item::Item(GeoModel * parent)
{
	_noOfCordinates = 0;
	_coordX = _coordY = nullptr;
	_modelHeight = _modelWidth = 0;
	_edgeMagnitude = nullptr;
	_edgeDerivativeX = nullptr;
	_edgeDerivativeY = nullptr;
	_DivisionId = nullptr;
	_SubDivisionId = nullptr;
	_Flags = nullptr;
	_modelDefined = false;
	_Angle = 0.0f;
	_SobelSize = 3;
	_bOriginal = false;
	_PyrDown = 0;
	_Stdev = 0.0;
	_ParentPtr = parent;
	_Filter = 0;

	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
}
GeoModel::Item::Item(GeoModel * parent, cv::Mat & src, cv::Mat mask, int nPreProvLv, float maxContrast, float minContrast, bool _bEdgeEnhance, int nFilter)
{
	_noOfCordinates = 0;
	_coordX = _coordY = nullptr;
	_modelHeight = _modelWidth = 0;
	_edgeMagnitude = nullptr;
	_edgeDerivativeX = nullptr;
	_edgeDerivativeY = nullptr;
	_DivisionId = nullptr;
	_SubDivisionId = nullptr;
	_Flags = nullptr;
	_modelDefined = false;
	_Angle = 0.0f;
	_SobelSize = 3;
	_bOriginal = false;
	_Stdev = 0.0;
	_ParentPtr = parent;
	_Filter = nFilter;

	Calculate(src, mask, nPreProvLv, maxContrast, minContrast, _bEdgeEnhance);

	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
}
GeoModel::Item::~Item()
{
	_Free();

	g_pMManager->pem_delete_check(this);
}


void GeoModel::Item::_Alloc(int wid, int len)
{
	_Free();

	_modelWidth = wid;
	_modelHeight = len;

	/*_coordX = new float[_modelWidth*_modelHeight];
	_coordY = new float[_modelWidth*_modelHeight];
	_edgeMagnitude = new float[_modelWidth*_modelHeight];
	_edgeDerivativeX = new float[_modelWidth*_modelHeight];
	_edgeDerivativeY = new float[_modelWidth*_modelHeight];
	_DivisionId = new int[_modelWidth*_modelHeight];
	_SubDivisionId = new int[_modelWidth*_modelHeight];
	_Flags = new BYTE[_modelWidth*_modelHeight];*/

	_coordX = g_pMManager->pem_new<float>(true, _modelWidth*_modelHeight, (PCHAR)__FUNCTION__, __LINE__);
	_coordY = g_pMManager->pem_new<float>(true, _modelWidth*_modelHeight, (PCHAR)__FUNCTION__, __LINE__);
	_edgeMagnitude = g_pMManager->pem_new<float>(true, _modelWidth*_modelHeight, (PCHAR)__FUNCTION__, __LINE__);
	_edgeDerivativeX = g_pMManager->pem_new<float>(true, _modelWidth*_modelHeight, (PCHAR)__FUNCTION__, __LINE__);
	_edgeDerivativeY = g_pMManager->pem_new<float>(true, _modelWidth*_modelHeight, (PCHAR)__FUNCTION__, __LINE__);
	_DivisionId = g_pMManager->pem_new<int>(true, _modelWidth*_modelHeight, (PCHAR)__FUNCTION__, __LINE__);
	_SubDivisionId = g_pMManager->pem_new<int>(true, _modelWidth*_modelHeight, (PCHAR)__FUNCTION__, __LINE__);
	_Flags = g_pMManager->pem_new<BYTE>(true, _modelWidth*_modelHeight, (PCHAR)__FUNCTION__, __LINE__);

	memset(_DivisionId, 0xff, sizeof(int) * _modelWidth*_modelHeight);
	memset(_SubDivisionId, 0xff, sizeof(int) * _modelWidth*_modelHeight);
	memset(_Flags, 0x00, sizeof(BYTE) * _modelWidth*_modelHeight);
}
void GeoModel::Item::_Free()
{
	_modelHeight = _modelWidth = 0;
	if (_coordX) //delete [] _coordX ;
		g_pMManager->pem_delete(_coordX, true);
	if (_coordY) //delete [] _coordY ;
		g_pMManager->pem_delete(_coordY, true);
	if (_edgeMagnitude) //delete [] _edgeMagnitude;
		g_pMManager->pem_delete(_edgeMagnitude, true);
	if (_edgeDerivativeX) //delete [] _edgeDerivativeX;
		g_pMManager->pem_delete(_edgeDerivativeX, true);
	if (_edgeDerivativeY) //delete [] _edgeDerivativeY;
		g_pMManager->pem_delete(_edgeDerivativeY, true);
	if (_DivisionId) //delete [] _DivisionId;
		g_pMManager->pem_delete(_DivisionId, true);
	if (_SubDivisionId) //delete [] _SubDivisionId;
		g_pMManager->pem_delete(_SubDivisionId, true);
	if (_Flags) //delete [] _Flags;
		g_pMManager->pem_delete(_Flags, true);

	_coordX = _coordY = nullptr;
	_edgeMagnitude = nullptr;
	_edgeDerivativeX = nullptr;
	_edgeDerivativeY = nullptr;
	_DivisionId = nullptr;
	_SubDivisionId = nullptr;
	_Flags = nullptr;
}


void GeoModel::Item::SaveFile(CArchive & ar)
{
	ar << _modelHeight;
	ar << _modelWidth;
	ar << _noOfCordinates;
	ar << _modelDefined;

	if (_noOfCordinates > 0 && _modelHeight > 0 && _modelWidth > 0)
	{
		for (int i = 0; i < _noOfCordinates; i++)
		{
			ar << _coordX[i];
			ar << _coordY[i];
			ar << _edgeMagnitude[i];
			ar << _edgeDerivativeX[i];
			ar << _edgeDerivativeY[i];
		}
	}

	ar << _CogX;
	ar << _CogY;
	ar << _Angle;
	ar << _bOriginal;
	ar << _AvgEdges;
	ar << _SobelSize;
}
void GeoModel::Item::LoadFile(CArchive & ar)
{
	ar >> _modelHeight;
	ar >> _modelWidth;
	ar >> _noOfCordinates;
	ar >> _modelDefined;

	if (_noOfCordinates > 0 && _modelHeight > 0 && _modelWidth > 0)
	{
		_Alloc(_modelWidth, _modelHeight);

		if (_noOfCordinates >= (_modelWidth*_modelHeight))
		{
			CString sLog;
			sLog.Format(_T("GeoModel::Item::LoadFile => data too many : %s"), _ParentPtr->_sLoadedPath);
			GeoWriteLog(sLog);
		}

		for (int i = 0; i < _noOfCordinates; i++)
		{
			ar >> _coordX[i];
			ar >> _coordY[i];
			ar >> _edgeMagnitude[i];
			ar >> _edgeDerivativeX[i];
			ar >> _edgeDerivativeY[i];
		}
	}
	else
	{
		_coordX = nullptr;
		_coordY = nullptr;
		_edgeMagnitude = nullptr;
		_edgeDerivativeX = nullptr;
		_edgeDerivativeY = nullptr;
		{
			CString sLog;
			sLog.Format(_T("GeoModel::Item::LoadFile => data zero state : %s"), _ParentPtr->_sLoadedPath);
			GeoWriteLog(sLog);
		}

		_Free();
	}

	ar >> _CogX;
	ar >> _CogY;
	ar >> _Angle;
	ar >> _bOriginal;
	ar >> _AvgEdges;
	ar >> _SobelSize;
}

void GeoModel::Item::SaveFile_v2(CArchive & ar)
{
	ar << _MinPos.x;
	ar << _MinPos.y;

	ar << _MaxPos.x;
	ar << _MaxPos.y;

	ar << _Stdev;
}
void GeoModel::Item::LoadFile_v2(CArchive & ar, int ver)
{
	if (ver < 2)
		return;

	if (ar.IsBufferEmpty() == FALSE)
		ar >> _MinPos.x;

	if (ar.IsBufferEmpty() == FALSE)
		ar >> _MinPos.y;

	if (ar.IsBufferEmpty() == FALSE)
		ar >> _MaxPos.x;

	if (ar.IsBufferEmpty() == FALSE)
		ar >> _MaxPos.y;

	if (ar.IsBufferEmpty() == FALSE)
		ar >> _Stdev;
}

void GeoModel::Item::SaveFile_v3(CArchive & ar)
{
	if (_noOfCordinates > 0 && _modelHeight > 0 && _modelWidth > 0)
	{
		for (int i = 0; i < _noOfCordinates; i++)
		{
			ar << _Flags[i];
		}
	}
}
void GeoModel::Item::LoadFile_v3(CArchive & ar, int ver)
{
	if (_noOfCordinates > 0 && _modelHeight > 0 && _modelWidth > 0)
	{
		if (_noOfCordinates < (_modelWidth*_modelHeight))
		{
			for (int i = 0; i < _noOfCordinates; i++)
			{
				if (ver >= 3 && ar.IsBufferEmpty() == FALSE)
					ar >> _Flags[i];
				else
					_Flags[i] = 0;
			}
		}
	}
}


bool GeoModel::Item::Calculate(cv::Mat & src, cv::Mat mask, int nPreProvLv, float maxContrast, float minContrast, bool _bEdgeEnhance, int use_algo)
{
	if (CV_MAT_TYPE(src.type()) != CV_8UC1 && CV_MAT_TYPE(src.type()) != CV_32FC1)
		return false;


	// 초기화
	_noOfCordinates = 0;
	_modelDefined = false;


	// 버퍼생성
	_modelWidth = src.cols;
	_modelHeight = src.rows;
	cv::Size Ssize(src.cols, src.rows);
	_Alloc(Ssize.width, Ssize.height);
	cv::Mat nmsEdges = cv::Mat(Ssize.height, Ssize.width, CV_32F);
	/*int * orients = (int *)_aligned_malloc(Ssize.height*Ssize.width*sizeof(int), 16);
	float * magMat = (float *)_aligned_malloc(Ssize.height*Ssize.width*sizeof(float), 16);*/
	int * orients = (int *)g_pMManager->pem_aligned_malloc(Ssize.height*Ssize.width * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
	float * magMat = (float *)g_pMManager->pem_aligned_malloc(Ssize.height*Ssize.width * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);


	//필터링
	if (_Filter > 0)
	{
#if _DEBUG
		cv::imwrite("D:\\testimage\\workimage\\GeoMatchSrc.bmp", src);
#endif

		cv::Mat ImageStandard, NomalImg, MedianMatOrg, MedianMat, thTemp;
		double thre_value1 = cv::threshold(src, thTemp, 0, 255, cv::THRESH_OTSU);
		cv::medianBlur(src, MedianMatOrg, 3);
		MedianMatOrg.convertTo(MedianMat, CV_16SC1);

		src.convertTo(ImageStandard, CV_16SC1);
		cv::Mat OMSub = MedianMat - ImageStandard;
		cv::Mat ImgADDM = ImageStandard + OMSub;
		cv::normalize(ImgADDM, NomalImg, 0, 255, cv::NORM_MINMAX);
		NomalImg.convertTo(src, CV_8UC1);
		double thre_value2 = cv::threshold(src, thTemp, 0, 255, cv::THRESH_OTSU);

		src = src - (thre_value2 - thre_value1);
#if _DEBUG
		cv::imwrite("D:\\testimage\\workimage\\GeoMatchSrc_Filter.bmp", src);
#endif
	}

	// 초기연산
	cv::Mat src_W = src.clone();
	cv::Scalar mean, stddev; // LWW test
	cv::meanStdDev(src, mean, stddev);

	// 에지 추출
	cv::Mat dst;
	if (use_algo == 1) // bace 알고리즘
		dst = algo_3(src_W, mean, stddev, src_W.cols, src_W.rows);
	else if (use_algo == 2) // 노이즈 섞여있을 때 알고리즘
		dst = algo_2(src_W, mean, stddev, src_W.cols, src_W.rows);
	else if (use_algo == 3) // 음각
		dst = algo_4(src_W);
	else if (use_algo == 4) // 양각
		dst = algo_6(src_W);
	else if (use_algo == 5) // image matching
	{

	}
	cv::Mat FlageMat(src_W.rows, src_W.cols, src_W.type());
	if (use_algo == 3 || use_algo == 4)
	{
		cv::Mat dstOrg = dst.clone();
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(2, 2));
		cv::dilate(dst, dst, kernel);
		FlageMat = dst - dstOrg;
	}
	else
	{
		FlageMat.setTo(0);
	}
	if (dst.empty() == false)
		cv::bitwise_and(dst, mask, dst);
	if (CV_MAT_TYPE(src.type()) == CV_32FC1)
		dst.setTo(255);

#ifdef GeoSaveDebug
	CString sSrc_image_path;
	sSrc_image_path.Format(_T("d:\\testimage\\workimage\\create_pattern_dst.bmp"));
	cv::imwrite(std::string(CT2A(sSrc_image_path)), dst);
#endif

	if (use_algo == GeoModel::enmMatchAlgo::agOriginal)
	{
		cv::Mat gx, gy;
		_Sobel(src, gx, gy, mask);

		int low_th = 150;
		int high_th = low_th * 2;

		cv::Canny(src, dst, low_th, high_th);
		// 		cv::imwrite(std::string("d:\\source.bmp"), src);
		// 		cv::imwrite(std::string("d:\\canny.bmp"), dst);


		// 		_MinPos.x = INT_MAX;
		// 		_MinPos.y = INT_MAX;
		// 		_MaxPos.x = INT_MIN;
		// 		_MaxPos.y = INT_MIN;
		// 
		// 		cv::Mat gsx, gsy;
		// 		_Sobel(src, gsx, gsy, mask);
		// 		int RSum(0), CSum(0);
		// 		int curX(1), curY(1);
		// 		int nSumEdges(0), sSunCnt(0);
		// 		for(int i=1; i<src.rows-1; i++)
		// 		{
		// 			short * _sdx = gsx.ptr<short>(i);
		// 			short * _sdy = gsy.ptr<short>(i);
		// 			float * pEdges_n = nmsEdges.ptr<float>(i-1);
		// 			float * pEdges   = nmsEdges.ptr<float>(i);
		// 			float * pEdges_p = nmsEdges.ptr<float>(i+1);
		// 			for(int j=1; j<(src.cols-1); j++)
		// 			{
		// 				float fdx = _sdx[j], fdy = _sdy[j];
		// 				float MagG = sqrt(fdx*fdx + fdy*fdy); //Magnitude = Sqrt(gx^2 +gy^2)
		// 				float DirG = cvFastArctan((float)fdy,(float)fdx);	 //Direction = tan(y/x)
		// 				curX=i;	curY=j;
		// 				if(fdx!=0 || fdy!=0)
		// 				{		
		// 					sSunCnt ++;
		// 					nSumEdges += pEdges[j];
		// 					RSum=RSum+curX;	CSum=CSum+curY; // Row sum and column sum for center of gravity
		// 					if(dst.ptr(i)[j] != 0)
		// 					{
		// 						_MinPos.x = cv::min(_MinPos.x, curY);
		// 						_MinPos.y = cv::min(_MinPos.y, curX);
		// 
		// 						_MaxPos.x = cv::max(_MaxPos.x, curY);
		// 						_MaxPos.y = cv::max(_MaxPos.y, curX);
		// 
		// 						_Flags[_noOfCordinates] = 0;
		// 						_coordX[_noOfCordinates] = curX; //좌표
		// 						_coordY[_noOfCordinates] = curY;
		// 						_edgeDerivativeX[_noOfCordinates] = fdx; //sobal 결과 데이터
		// 						_edgeDerivativeY[_noOfCordinates] = fdy;
		// 						if(MagG!=0)
		// 							_edgeMagnitude[_noOfCordinates] = 1/MagG;  // gradient magnitude //NCC 미리 계산
		// 						else
		// 							_edgeMagnitude[_noOfCordinates] = 0;
		// 						_noOfCordinates++;
		// 					}
		// 				}
		// 			}
		// 		}
		// 		if(sSunCnt!=0.0)
		// 			_AvgEdges = nSumEdges / sSunCnt;
		// 		_CogX = (float)(Ssize.height) * 0.5f;
		// 		_CogY = (float)(Ssize.width) * 0.5f;
		// 		for(int m=0; m<_noOfCordinates; m++)
		// 		{
		// 			int temp;
		// 			temp = _coordX[m];
		// 			_coordX[m] = temp - _CogX;
		// 			temp = _coordY[m];
		// 			_coordY[m] = temp - _CogY;
		// 		}
		// 
		// 		_MinPos.x = RounDF( (float)(_MinPos.x) - _CogY );
		// 		_MinPos.y = RounDF( (float)(_MinPos.y) - _CogX );
		// 
		// 		_MaxPos.x = RounDF( (float)(_MaxPos.x) - _CogY );
		// 		_MaxPos.y = RounDF( (float)(_MaxPos.y) - _CogX );

		float MaxGradient = -HUGE_VAL;
		_Calc_Dir_Mag(gx, gy, magMat, orients, Ssize, MaxGradient);
		_NonMaximalSuppression(orients, magMat, nmsEdges, Ssize, MaxGradient);
		_HysterisisThreshold(gx, gy, magMat, nmsEdges, Ssize, maxContrast, minContrast);
	}
	else if (use_algo == GeoModel::enmMatchAlgo::agDefault || use_algo == GeoModel::enmMatchAlgo::agLowContrast)
	{
		_MinPos.x = INT_MAX;
		_MinPos.y = INT_MAX;
		_MaxPos.x = INT_MIN;
		_MaxPos.y = INT_MIN;

		cv::Mat gsx, gsy;
		_Sobel(src, gsx, gsy, mask);
		int RSum(0), CSum(0);
		int curX(1), curY(1);
		int nSumEdges(0), sSunCnt(0);
		for (int i = 1; i < src.rows - 1; i++)
		{
			short * _sdx = gsx.ptr<short>(i);
			short * _sdy = gsy.ptr<short>(i);
			float * pEdges_n = nmsEdges.ptr<float>(i - 1);
			float * pEdges = nmsEdges.ptr<float>(i);
			float * pEdges_p = nmsEdges.ptr<float>(i + 1);
			for (int j = 1; j < (src.cols - 1); j++)
			{
				float fdx = _sdx[j], fdy = _sdy[j];
				float MagG = sqrt(fdx*fdx + fdy * fdy); //Magnitude = Sqrt(gx^2 +gy^2)
				float DirG = cv::fastAtan2((float)fdy, (float)fdx);	 //Direction = tan(y/x)
				curX = i;	curY = j;
				if (fdx != 0 || fdy != 0)
				{
					sSunCnt++;
					nSumEdges += pEdges[j];
					RSum = RSum + curX;	CSum = CSum + curY; // Row sum and column sum for center of gravity
					if (dst.ptr(i)[j] != 0)
					{
						_MinPos.x = cv::min(_MinPos.x, curY);
						_MinPos.y = cv::min(_MinPos.y, curX);

						_MaxPos.x = cv::max(_MaxPos.x, curY);
						_MaxPos.y = cv::max(_MaxPos.y, curX);

						_Flags[_noOfCordinates] = 0;
						_coordX[_noOfCordinates] = curX; //좌표
						_coordY[_noOfCordinates] = curY;
						_edgeDerivativeX[_noOfCordinates] = fdx; //sobal 결과 데이터
						_edgeDerivativeY[_noOfCordinates] = fdy;
						if (MagG != 0)
							_edgeMagnitude[_noOfCordinates] = 1 / MagG;  // gradient magnitude //NCC 미리 계산
						else
							_edgeMagnitude[_noOfCordinates] = 0;
						_noOfCordinates++;
					}
				}
			}
		}
		if (sSunCnt != 0.0)
			_AvgEdges = nSumEdges / sSunCnt;
		_CogX = (float)(Ssize.height) * 0.5f;
		_CogY = (float)(Ssize.width) * 0.5f;
		for (int m = 0; m < _noOfCordinates; m++)
		{
			int temp;
			temp = _coordX[m];
			_coordX[m] = temp - _CogX;
			temp = _coordY[m];
			_coordY[m] = temp - _CogY;
		}

		_MinPos.x = RounDF((float)(_MinPos.x) - _CogY);
		_MinPos.y = RounDF((float)(_MinPos.y) - _CogX);

		_MaxPos.x = RounDF((float)(_MaxPos.x) - _CogY);
		_MaxPos.y = RounDF((float)(_MaxPos.y) - _CogX);
	}
	else if (use_algo == GeoModel::enmMatchAlgo::agIntaglio || use_algo == GeoModel::enmMatchAlgo::agRelief)
	{
		cv::Mat gl;
		src.copyTo(gl);

#ifdef GeoSaveDebug
		sSrc_image_path.Format(_T("d:\\testimage\\workimage\\create_pattern_laplacian.bmp"));
		cv::imwrite(std::string(CT2A(sSrc_image_path)), gl);
#endif
		_MinPos.x = INT_MAX;
		_MinPos.y = INT_MAX;
		_MaxPos.x = INT_MIN;
		_MaxPos.y = INT_MIN;

		double SubSum(0.0);

		int RSum(0), CSum(0);
		int curX(1), curY(1);
		int nSumEdges(0), sSunCnt(0);
		for (int i = 1; i < src.rows - 1; i++)
		{
			BYTE * _sdl = gl.ptr<BYTE>(i);
			float * pEdges_n = nmsEdges.ptr<float>(i - 1);
			float * pEdges = nmsEdges.ptr<float>(i);
			float * pEdges_p = nmsEdges.ptr<float>(i + 1);
			for (int j = 1; j < (src.cols - 1); j++)
			{
				float fdl(0.0), MagG(0.0);

				fdl = (float)(_sdl[j]) - mean.val[0];
				MagG = sqrt(fdl*fdl);

				curX = i;	curY = j;
				if (fdl != 0)
				{
					sSunCnt++;
					nSumEdges += pEdges[j];
					RSum = RSum + curX;	CSum = CSum + curY;
					if (dst.ptr(i)[j] != 0)
					{
						_MinPos.x = cv::min(_MinPos.x, curY);
						_MinPos.y = cv::min(_MinPos.y, curX);

						_MaxPos.x = cv::max(_MaxPos.x, curY);
						_MaxPos.y = cv::max(_MaxPos.y, curX);

						SubSum += (fdl * fdl);
						_Flags[_noOfCordinates] = FlageMat.ptr(curX)[curY] > 0 ? GeoModel::enmPtDataFlag::PdfDraw : 0;
						_coordX[_noOfCordinates] = curX;
						_coordY[_noOfCordinates] = curY;
						_edgeDerivativeX[_noOfCordinates] = fdl;
						_edgeDerivativeY[_noOfCordinates] = 0;
						if (MagG != 0)
							_edgeMagnitude[_noOfCordinates] = 1 / MagG;
						else
							_edgeMagnitude[_noOfCordinates] = 0;
						_noOfCordinates++;
					}
				}
			}
		}

		_Stdev = sqrt(SubSum / (double)(_noOfCordinates));

		if (sSunCnt != 0.0)
			_AvgEdges = nSumEdges / sSunCnt;
		_CogX = (float)(Ssize.height) * 0.5f;
		_CogY = (float)(Ssize.width) * 0.5f;
		for (int m = 0; m < _noOfCordinates; m++)
		{
			int temp;
			temp = _coordX[m];
			_coordX[m] = temp - _CogX;
			temp = _coordY[m];
			_coordY[m] = temp - _CogY;
		}

		_MinPos.x = RounDF((float)(_MinPos.x) - _CogY);
		_MinPos.y = RounDF((float)(_MinPos.y) - _CogX);

		_MaxPos.x = RounDF((float)(_MaxPos.x) - _CogY);
		_MaxPos.y = RounDF((float)(_MaxPos.y) - _CogX);
	}
	else if (use_algo == GeoModel::enmMatchAlgo::agValue)
	{
		cv::Mat gl;
		src.copyTo(gl);

		_MinPos.x = INT_MAX;
		_MinPos.y = INT_MAX;
		_MaxPos.x = INT_MIN;
		_MaxPos.y = INT_MIN;

		double SubSum(0.0);

		int RSum(0), CSum(0);
		int curX(1), curY(1);
		int nSumEdges(0), sSunCnt(0);
		for (int i = 0; i < src.rows; i++)
		{
			BYTE * _sdl = gl.ptr<BYTE>(i);
			for (int j = 0; j < src.cols; j++)
			{
				float fdl(0.0), MagG(0.0);

				fdl = _sdl[j];
				MagG = sqrt(fdl*fdl);

				curX = i;	curY = j;
				if (fdl > 0)
				{
					sSunCnt++;
					nSumEdges += fdl;
					RSum = RSum + curX;	CSum = CSum + curY;

					SubSum += (fdl * fdl);

					_Flags[_noOfCordinates] = 0;
					_coordX[_noOfCordinates] = curX;
					_coordY[_noOfCordinates] = curY;
					_edgeDerivativeX[_noOfCordinates] = fdl;
					_edgeDerivativeY[_noOfCordinates] = 0;
					if (MagG != 0)
						_edgeMagnitude[_noOfCordinates] = 1.f / MagG;
					else
						_edgeMagnitude[_noOfCordinates] = 0;
					_noOfCordinates++;
				}
				else
				{
					_MinPos.x = cv::min(_MinPos.x, curY);
					_MinPos.y = cv::min(_MinPos.y, curX);

					_MaxPos.x = cv::max(_MaxPos.x, curY);
					_MaxPos.y = cv::max(_MaxPos.y, curX);
				}
			}
		}

		_Stdev = sqrt(SubSum / (double)(_noOfCordinates));

		if (sSunCnt != 0.0)
			_AvgEdges = nSumEdges / sSunCnt;
		_CogX = Ssize.height / 2;//(float)(Ssize.height) * 0.5f;
		_CogY = Ssize.width / 2;//(float)(Ssize.width) * 0.5f;
		for (int m = 0; m < _noOfCordinates; m++)
		{
			int temp;
			temp = _coordX[m];
			_coordX[m] = temp - _CogX;
			temp = _coordY[m];
			_coordY[m] = temp - _CogY;
		}

		_MinPos.x = (float)(_MinPos.x) - _CogY;
		_MinPos.y = (float)(_MinPos.y) - _CogX;

		_MaxPos.x = (float)(_MaxPos.x) - _CogY;
		_MaxPos.y = (float)(_MaxPos.y) - _CogX;
	}

	/*_aligned_free(orients);
	_aligned_free(magMat);*/
	g_pMManager->pem_aligned_free(orients);
	g_pMManager->pem_aligned_free(magMat);

	_modelDefined = true;

	return _modelDefined;
}
bool GeoModel::Item::CalculateAngle(int modelWidth, int modelHeight, int nPreProvLv, float maxContrast, float minContrast, bool _bEdgeEnhance, std::shared_ptr<Item> Orgmd, float angle, int use_algo)
{
	// 	if(CV_MAT_TYPE( Orgmd._.type() ) != CV_8UC1)
	// 		return false;


		// 초기화
	_noOfCordinates = 0;
	_modelDefined = false;


	// 버퍼생성
	_modelWidth = modelWidth;
	_modelHeight = modelHeight;
	cv::Size Ssize(_modelWidth, _modelHeight);
	_Alloc(Ssize.width, Ssize.height);
	cv::Mat nmsEdges = cv::Mat(Ssize.height, Ssize.width, CV_32F);
	/*int * orients = (int *)_aligned_malloc(Ssize.height*Ssize.width*sizeof(int), 16);
	float * magMat = (float *)_aligned_malloc(Ssize.height*Ssize.width*sizeof(float), 16);*/
	int * orients = (int *)g_pMManager->pem_aligned_malloc(Ssize.height*Ssize.width * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
	float * magMat = (float *)g_pMManager->pem_aligned_malloc(Ssize.height*Ssize.width * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);

	//상수계산
	float _angleVal = angle - Orgmd->_Angle;
	float radian = _angleVal / 180.0f * M_PI;
	double _sin = sin(radian);
	double _cos = cos(radian);
	cv::Point2f center(_modelWidth / 2.0, _modelHeight / 2.0);

	// 	cv::Mat testImg(Ssize.height,Ssize.width,CV_8UC1);
	// 	cv::Mat testImgY(Ssize.height,Ssize.width,CV_8UC1);
	//		testImg.zeros(Ssize.height, Ssize.width, CV_8UC1);
	if (use_algo == GeoModel::enmMatchAlgo::agDefault || use_algo == GeoModel::enmMatchAlgo::agLowContrast || use_algo == GeoModel::enmMatchAlgo::agOriginal)
	{
		_MinPos.x = INT_MAX;
		_MinPos.y = INT_MAX;
		_MaxPos.x = INT_MIN;
		_MaxPos.y = INT_MIN;

		int RSum(0), CSum(0);
		int curX(1), curY(1);


		_CogX = (float)(Ssize.height) * 0.5f;
		_CogY = (float)(Ssize.width) * 0.5f;
		for (int i = 0; i < Orgmd->_noOfCordinates; i++)
		{
			curY = (Orgmd->_coordX[i]) * _sin + (Orgmd->_coordY[i]) * _cos;	//x
			curX = (Orgmd->_coordX[i]) * _cos - (Orgmd->_coordY[i]) * _sin;	//y

			_MinPos.x = cv::min(_MinPos.x, curY);
			_MinPos.y = cv::min(_MinPos.y, curX);

			_MaxPos.x = cv::max(_MaxPos.x, curY);
			_MaxPos.y = cv::max(_MaxPos.y, curX);

			_Flags[i] = Orgmd->_Flags[i];
			_coordX[i] = curX; //좌표
			_coordY[i] = curY;
			_edgeDerivativeX[i] = (Orgmd->_edgeDerivativeX[i]) * _cos + (Orgmd->_edgeDerivativeY[i]) * _sin; //sobal 결과 데이터
			_edgeDerivativeY[i] = -(Orgmd->_edgeDerivativeX[i]) * _sin + (Orgmd->_edgeDerivativeY[i]) * _cos;
			float MagG = sqrt(_edgeDerivativeX[i] * _edgeDerivativeX[i] + _edgeDerivativeY[i] * _edgeDerivativeY[i]); //Magnitude = Sqrt(gx^2 +gy^2)
			if (MagG != 0)
				_edgeMagnitude[i] = 1 / MagG;  // gradient magnitude //NCC 미리 계산
			else
				_edgeMagnitude[i] = 0;

			_noOfCordinates++;

			// 			UCHAR* chptrx = testImg.ptr(curX+_CogX);
			// 			chptrx[curY+(int)_CogY] = (UCHAR)(_edgeDerivativeX[i]/10);
			// 			UCHAR* chptrY = testImgY.ptr(curX+_CogX);
			// 			chptrY[curY+(int)_CogY] = (UCHAR)(_edgeDerivativeY[i]/10);
		}
		// 		if( _PyrDown == enmDepth::spOrg )
		// 		{
		// 			CString msg;
		// 			msg.Format(_T("D:\\testImgX_w%d_h%d_a%.1f_a%.1f.bmp"),Ssize.width,Ssize.height,angle,angle);
		// 			imwrite(std::string(CT2A(msg)),testImg);
		// 			msg.Format(_T("D:\\testImgY_w%d_h%d_a%.1f_a%.1f.bmp"),Ssize.width,Ssize.height,angle,angle);
		// 			imwrite(std::string(CT2A(msg)),testImgY);
		// 		}

		// 		_MinPos.x = RounDF( (float)(_MinPos.x) - _CogY );
		// 		_MinPos.y = RounDF( (float)(_MinPos.y) - _CogX );
		// 
		// 		_MaxPos.x = RounDF( (float)(_MaxPos.x) - _CogY );
		// 		_MaxPos.y = RounDF( (float)(_MaxPos.y) - _CogX );
	}
	else if (use_algo == GeoModel::enmMatchAlgo::agIntaglio || use_algo == GeoModel::enmMatchAlgo::agRelief)
	{

		_MinPos.x = INT_MAX;
		_MinPos.y = INT_MAX;
		_MaxPos.x = INT_MIN;
		_MaxPos.y = INT_MIN;

		double SubSum(0.0);

		int RSum(0), CSum(0);
		int curX(1), curY(1);
		int nSumEdges(0), sSunCnt(0);

		_CogX = (float)(Ssize.height) * 0.5f;
		_CogY = (float)(Ssize.width) * 0.5f;

		for (int i = 0; i < Orgmd->_noOfCordinates; i++)
		{
			float fdl(0.0), MagG(0.0);

			fdl = (float)(Orgmd->_edgeDerivativeX[_noOfCordinates]);
			MagG = sqrt(fdl*fdl);

			curY = (Orgmd->_coordX[i]) * _sin + (Orgmd->_coordY[i]) * _cos;	//x
			curX = (Orgmd->_coordX[i]) * _cos - (Orgmd->_coordY[i]) * _sin;	//y

			_MinPos.x = cv::min(_MinPos.x, curY);
			_MinPos.y = cv::min(_MinPos.y, curX);

			_MaxPos.x = cv::max(_MaxPos.x, curY);
			_MaxPos.y = cv::max(_MaxPos.y, curX);

			SubSum += (fdl * fdl);

			_Flags[_noOfCordinates] = Orgmd->_Flags[_noOfCordinates];
			_coordX[_noOfCordinates] = curX;
			_coordY[_noOfCordinates] = curY;
			_edgeDerivativeX[_noOfCordinates] = fdl;
			_edgeDerivativeY[_noOfCordinates] = 0;
			if (MagG != 0)
				_edgeMagnitude[_noOfCordinates] = 1 / MagG;
			else
				_edgeMagnitude[_noOfCordinates] = 0;
			_noOfCordinates++;
			// 			UCHAR* chptr = testImg.ptr(curX+_CogX);
			// 			chptr[curY+(int)_CogY] = 255;
		}

		_Stdev = sqrt(SubSum / (double)(_noOfCordinates));

		if (sSunCnt != 0.0)
			_AvgEdges = nSumEdges / sSunCnt;

		// 		if( _PyrDown == enmDepth::spOrg )
		// 		{
		// 
		// 			CString msg;
		// 			msg.Format(_T("D:\\testImg_w%d_h%d_a%.1f_a%.1f.bmp"),Ssize.width,Ssize.height,angle,_angleVal);
		// 			imwrite(std::string(CT2A(msg)),testImg);
		// 		}
		// 		for(int m=0; m<_noOfCordinates; m++)
		// 		{
		// 			int temp;
		// 			temp = _coordX[m];
		// 			_coordX[m] = temp - _CogX;
		// 			temp = _coordY[m];
		// 			_coordY[m] = temp - _CogY;
		// 		}
		//
		// 		_MinPos.x = RounDF( (float)(_MinPos.x) - _CogY );
		// 		_MinPos.y = RounDF( (float)(_MinPos.y) - _CogX );
		// 
		// 		_MaxPos.x = RounDF( (float)(_MaxPos.x) - _CogY );
		// 		_MaxPos.y = RounDF( (float)(_MaxPos.y) - _CogX );
	}
	else if (use_algo == GeoModel::enmMatchAlgo::agValue)
	{
		_MinPos.x = INT_MAX;
		_MinPos.y = INT_MAX;
		_MaxPos.x = INT_MIN;
		_MaxPos.y = INT_MIN;

		double SubSum(0.0);

		int RSum(0), CSum(0);
		int curX(1), curY(1);
		int nSumEdges(0), sSunCnt(0);

		_CogX = (float)(Ssize.height) * 0.5f;
		_CogY = (float)(Ssize.width) * 0.5f;

		for (int i = 0; i < Orgmd->_noOfCordinates; i++)
		{
			float fdl(0.0), MagG(0.0);

			fdl = (float)(Orgmd->_edgeDerivativeX[_noOfCordinates]);
			MagG = sqrt(fdl*fdl);

			curY = (Orgmd->_coordX[i]) * _sin + (Orgmd->_coordY[i]) * _cos;	//x
			curX = (Orgmd->_coordX[i]) * _cos - (Orgmd->_coordY[i]) * _sin;	//y

// 			_MinPos.x = cv::min(_MinPos.x, curY);
// 			_MinPos.y = cv::min(_MinPos.y, curX);
// 
// 			_MaxPos.x = cv::max(_MaxPos.x, curY);
// 			_MaxPos.y = cv::max(_MaxPos.y, curX);

			SubSum += (fdl * fdl);

			_Flags[_noOfCordinates] = Orgmd->_Flags[_noOfCordinates];
			_coordX[_noOfCordinates] = curX;
			_coordY[_noOfCordinates] = curY;
			_edgeDerivativeX[_noOfCordinates] = fdl;
			_edgeDerivativeY[_noOfCordinates] = 0;
			if (MagG != 0)
				_edgeMagnitude[_noOfCordinates] = 1 / MagG;
			else
				_edgeMagnitude[_noOfCordinates] = 0;
			_noOfCordinates++;
		}

		_MinPos.x = (Orgmd->_MinPos.y) * _sin + (Orgmd->_MinPos.x) * _cos;	//x
		_MinPos.y = (Orgmd->_MinPos.y) * _cos - (Orgmd->_MinPos.x) * _sin;	//y

		_MaxPos.x = (Orgmd->_MaxPos.y) * _sin + (Orgmd->_MaxPos.x) * _cos;	//x
		_MaxPos.y = (Orgmd->_MaxPos.y) * _cos - (Orgmd->_MaxPos.x) * _sin;	//y

		_Stdev = sqrt(SubSum / (double)(_noOfCordinates));

		if (sSunCnt != 0.0)
			_AvgEdges = nSumEdges / sSunCnt;
	}

	/*_aligned_free(orients);
	_aligned_free(magMat);*/
	g_pMManager->pem_aligned_free(orients);
	g_pMManager->pem_aligned_free(magMat);

	_modelDefined = true;

	return _modelDefined;
}
bool GeoModel::Item::ProCalculate(cv::Mat & src, int use_algo)
{
	if (CV_MAT_TYPE(src.type()) != CV_8UC1)
		return false;


	cv::Size Ssize(src.cols, src.rows);

	cv::Mat nmsEdges = cv::Mat(Ssize.height, Ssize.width, CV_32F);
	/*int * orients = (int *)_aligned_malloc(Ssize.height*Ssize.width*sizeof(int), 16);
	float * magMat = (float *)_aligned_malloc(Ssize.height*Ssize.width*sizeof(float), 16);*/
	int * orients = (int *)g_pMManager->pem_aligned_malloc(Ssize.height*Ssize.width * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
	float * magMat = (float *)g_pMManager->pem_aligned_malloc(Ssize.height*Ssize.width * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);

	cv::Mat src_W = src.clone();
	cv::Scalar mean, stddev; // LWW test
	cv::meanStdDev(src, mean, stddev);

#ifdef GeoSaveDebug
	CString sSrc_image_path;
	sSrc_image_path.Format(_T("d:\\testimage\\workimage\\create_pattern_dst.bmp"));
	cv::imwrite(std::string(CT2A(sSrc_image_path)), dst);
#endif


	if (use_algo == GeoModel::enmMatchAlgo::agDefault || use_algo == GeoModel::enmMatchAlgo::agLowContrast || use_algo == GeoModel::enmMatchAlgo::agOriginal)
	{
		cv::Mat gsx, gsy;
		_Sobel(src, gsx, gsy, cv::Mat());

		int gsxp = gsx.step1();
		int gsyp = gsy.step1();
		short * _sdx = gsx.ptr<short>();
		short * _sdy = gsy.ptr<short>();
		for (int z = 0; z < _noOfCordinates; z++)
		{
			int y = _coordX[z] + _CogX;
			int x = _coordY[z] + _CogY;
			int j = y * gsxp + x;

			float fdx = _sdx[j], fdy = _sdy[j];
			float MagG = sqrt(fdx*fdx + fdy * fdy);
			float DirG = cv::fastAtan2((float)fdy, (float)fdx);

			_edgeDerivativeX[z] = fdx;
			_edgeDerivativeY[z] = fdy;
			if (MagG != 0)
				_edgeMagnitude[z] = 1.0 / MagG;
			else
				_edgeMagnitude[z] = 0;
		}
	}
	else if (use_algo == GeoModel::enmMatchAlgo::agIntaglio || use_algo == GeoModel::enmMatchAlgo::agRelief)
	{
		cv::Mat gl;
		src.copyTo(gl);

		int glp = gl.step1();
		BYTE * _gl = gl.ptr<BYTE>();

		double SubSum(0.0);
		for (int z = 0; z < _noOfCordinates; z++)
		{
			int y = _coordX[z] + _CogX;
			int x = _coordY[z] + _CogY;
			int j = y * glp + x;

			float fdl(0.0), MagG(0.0);

			fdl = (float)(_gl[j]) - mean.val[0];
			MagG = sqrt(fdl*fdl);
			SubSum += fdl * fdl;
			_edgeDerivativeX[z] = fdl;
			_edgeDerivativeY[z] = 0;
			if (MagG != 0)
				_edgeMagnitude[z] = 1.0 / MagG;
			else
				_edgeMagnitude[z] = 0;
		}
		_Stdev = sqrt(SubSum / (double)(_noOfCordinates));
	}
	else if (use_algo == GeoModel::enmMatchAlgo::agValue)
	{
		cv::Mat gl;
		src.copyTo(gl);

		int glp = gl.step1();
		BYTE * _gl = gl.ptr<BYTE>();

		int nSumCnt(0);
		double SubSum(0.0), nSumEdges(0.0);
		// 		for(int z=0; z<_noOfCordinates; z++)
		// 		{
		// 			int y = _coordX[z] + _CogX;
		// 			int x = _coordY[z] + _CogY;
		// 			int j = y*glp+x;
		// 
		// 			float fdl(0.0), MagG(0.0);
		// 
		// 			fdl = gl.ptr<BYTE>(y)[x];
		// 			nSumEdges += fdl;
		// 			nSumCnt++;
		// 		}
		// 		nSumEdges /= nSumCnt;

		for (int z = 0; z < _noOfCordinates; z++)
		{
			int y = _coordX[z] + _CogX;
			int x = _coordY[z] + _CogY;
			int j = y * glp + x;

			float fdl(0.0), MagG(0.0);

			fdl = (float)(gl.ptr<BYTE>(y)[x]);
			MagG = sqrt(fdl*fdl);
			nSumEdges += fdl;
			SubSum += fdl * fdl;

			_edgeDerivativeX[z] = fdl;
			_edgeDerivativeY[z] = 0;
			if (MagG != 0)
				_edgeMagnitude[z] = 1.0 / MagG;
			else
				_edgeMagnitude[z] = 0;
		}

		_Stdev = sqrt(SubSum / (double)(_noOfCordinates));
		_AvgEdges = nSumEdges;
	}
	/*_aligned_free(orients);
	_aligned_free(magMat);*/
	g_pMManager->pem_aligned_free(orients);
	g_pMManager->pem_aligned_free(magMat);

	return _modelDefined;
}

void GeoModel::Item::_Sobel(cv::Mat & src, cv::Mat & gx, cv::Mat & gy, cv::Mat & mask)
{
	int ssz = _SobelSize;

	cv::Sobel(src, gx, CV_16S, 1, 0, ssz);
	cv::Sobel(src, gy, CV_16S, 0, 1, ssz);

	cv::Mat gxm, gym;
	if (mask.empty() == false)
	{
		gx.copyTo(gxm, mask);
		gy.copyTo(gym, mask);

		gx = gxm;
		gy = gym;
	}
}
void GeoModel::Item::_Calc_Dir_Mag(cv::Mat gx, cv::Mat gy, cv::Mat gsx, cv::Mat gsy, float * magMat, int * orients, cv::Size Ssize, float & MaxGradient)
{
	MaxGradient = -HUGE_VAL;
	int count = 0;
	for (int i = 1; i < Ssize.height - 1; i++)
	{
		short * _sdx = gx.ptr<short>(i);
		short * _sdy = gy.ptr<short>(i);

		short * _sdsx = gsx.ptr<short>(i);
		short * _sdsy = gsy.ptr<short>(i);

		for (int j = 1; j < Ssize.width - 1; j++)
		{
			float fdx = _sdx[j], fdy = _sdy[j];        // read x, y derivatives
			float fdsx = _sdsx[j], fdsy = _sdsy[j];        // read x, y derivatives

			float MagG = sqrt((float)(fdx*fdx) + (float)(fdy*fdy)); //Magnitude = Sqrt(gx^2 +gy^2)
			float MagSG = sqrt((float)(fdsx*fdsx) + (float)(fdsy*fdsy)); //Magnitude = Sqrt(gx^2 +gy^2)
			float direction = cv::fastAtan2((float)fdsx, (float)fdsy);	 //Direction = invtan (Gy / Gx)
			magMat[i*Ssize.width + j] = MagSG;

			if (MagG > MaxGradient)
				MaxGradient = MagG; // get maximum gradient value for normalizing.

			// get closest angle from 0, 45, 90, 135 set
			if ((direction > 0 && direction < 22.5) || (direction > 157.5 && direction < 202.5) || (direction > 337.5 && direction < 360))
				direction = 0;
			else if ((direction > 22.5 && direction < 67.5) || (direction > 202.5 && direction < 247.5))
				direction = 45;
			else if ((direction > 67.5 && direction < 112.5) || (direction > 247.5 && direction < 292.5))
				direction = 90;
			else if ((direction > 112.5 && direction < 157.5) || (direction > 292.5 && direction < 337.5))
				direction = 135;
			else
				direction = 0;

			orients[count] = (int)direction;
			count++;
		}
	}
}
void GeoModel::Item::_Calc_Dir_Mag(cv::Mat gx, cv::Mat gy, float * magMat, int * orients, cv::Size Ssize, float & MaxGradient)
{
	MaxGradient = -HUGE_VAL;
	int count = 0;
	for (int i = 1; i < Ssize.height - 1; i++)
	{
		short * _sdx = gx.ptr<short>(i);
		short * _sdy = gy.ptr<short>(i);

		for (int j = 1; j < Ssize.width - 1; j++)
		{
			float fdx = _sdx[j], fdy = _sdy[j];        // read x, y derivatives

			float MagG = sqrt((float)(fdx*fdx) + (float)(fdy*fdy)); //Magnitude = Sqrt(gx^2 +gy^2)
			float direction = cv::fastAtan2((float)fdx, (float)fdy);	 //Direction = invtan (Gy / Gx)
			magMat[i*Ssize.width + j] = MagG;

			if (MagG > MaxGradient)
				MaxGradient = MagG; // get maximum gradient value for normalizing.

			// get closest angle from 0, 45, 90, 135 set
			if ((direction > 0 && direction < 22.5) || (direction > 157.5 && direction < 202.5) || (direction > 337.5 && direction < 360))
				direction = 0;
			else if ((direction > 22.5 && direction < 67.5) || (direction > 202.5 && direction < 247.5))
				direction = 45;
			else if ((direction > 67.5 && direction < 112.5) || (direction > 247.5 && direction < 292.5))
				direction = 90;
			else if ((direction > 112.5 && direction < 157.5) || (direction > 292.5 && direction < 337.5))
				direction = 135;
			else
				direction = 0;

			orients[count] = (int)direction;
			count++;
		}
	}
}
void GeoModel::Item::_NonMaximalSuppression(int * orients, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float MaxGradient)
{
	int count(0);
	const int nDiff(1);
	for (int i = nDiff; i < Ssize.height - nDiff; i++)
	{
		float * pEdges = nmsEdges.ptr<float>(i);
		for (int j = nDiff; j < Ssize.width - nDiff; j++)
		{
			float leftPixel(0.f), rightPixel(0.f);

			switch (orients[count])
			{
			case 0:
				leftPixel = magMat[(i - nDiff)*Ssize.width + j];
				rightPixel = magMat[(i + nDiff)*Ssize.width + j];
				break;
			case 45:
				leftPixel = magMat[(i - nDiff)*Ssize.width + j - nDiff];
				rightPixel = magMat[(i + nDiff)*Ssize.width + j + nDiff];
				break;
			case 90:
				leftPixel = magMat[(i)*Ssize.width + j - nDiff];
				rightPixel = magMat[(i)*Ssize.width + j + nDiff];
				break;
			case 135:
				leftPixel = magMat[(i - nDiff)*Ssize.width + j + nDiff];
				rightPixel = magMat[(i + nDiff)*Ssize.width + j - nDiff];
				break;
			}
			// compare current pixels value with adjacent pixels
// 			if (( magMat[i*Ssize.width+j] < leftPixel ) || (magMat[i*Ssize.width+j] < rightPixel ) )
// 				pEdges[j]=0;
// 			else
// 			{
// 				float fMag = magMat[i*Ssize.width+j]/MaxGradient;
// 				if(fMag > 1.0f)
// 					fMag = 1.0f;
// 				pEdges[j]=(uchar)(fMag*255);
// 			}
			float fMag = magMat[i*Ssize.width + j] / MaxGradient;
			if (fMag > 1.0f)
				fMag = 1.0f;
			pEdges[j] = (uchar)(fMag * 255);
			count++;
		}
	}
}
void GeoModel::Item::_HysterisisThreshold(cv::Mat gx, cv::Mat gy, cv::Mat gsx, cv::Mat gsy, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float maxContrast, float minContrast)
{
	int RSum(0), CSum(0);
	int curX(1), curY(1);
	int flag(1);

	int nSumEdges(0), sSunCnt(0);
	for (int i = 1; i < Ssize.height - 1; i++)
	{
		short * _sdx = gsx.ptr<short>(i);
		short * _sdy = gsy.ptr<short>(i);
		float * pEdges_n = nmsEdges.ptr<float>(i - 1);
		float * pEdges = nmsEdges.ptr<float>(i);
		float * pEdges_p = nmsEdges.ptr<float>(i + 1);

		for (int j = 1; j < (Ssize.width - 1); j++)
		{
			float fdx = _sdx[j], fdy = _sdy[j];

			float MagG = sqrt(fdx*fdx + fdy * fdy); //Magnitude = Sqrt(gx^2 +gy^2)
			float DirG = cv::fastAtan2((float)fdy, (float)fdx);	 //Direction = tan(y/x)

			////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]= MagG;
			flag = 1;
			if ((pEdges[j]) < maxContrast)
			{
				if (pEdges[j] < minContrast)
				{
					pEdges[j] = 0;
					flag = 0; // remove from edge
					////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
				}
				else
				{   // if any of 8 neighboring pixel is not greater than max contraxt remove from edge
					if ((pEdges_n[j - 1] < maxContrast) &&
						(pEdges_n[j] < maxContrast) &&
						(pEdges_n[j + 1] < maxContrast) &&
						(pEdges[j - 1] < maxContrast) &&
						(pEdges[j + 1] < maxContrast) &&
						(pEdges_p[j - 1] < maxContrast) &&
						(pEdges_p[j] < maxContrast) &&
						(pEdges_p[j + 1] < maxContrast))
					{
						pEdges[j] = 0;
						flag = 0;
						////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
					}
				}

			}

			// save selected edge information
			curX = i;	curY = j;
			if (flag != 0)
			{
				if (fdx != 0 || fdy != 0)
				{
					sSunCnt++;
					nSumEdges += pEdges[j];
					RSum = RSum + curX;	CSum = CSum + curY; // Row sum and column sum for center of gravity

					_coordX[_noOfCordinates] = curX;
					_coordY[_noOfCordinates] = curY;
					_edgeDerivativeX[_noOfCordinates] = fdx;
					_edgeDerivativeY[_noOfCordinates] = fdy;

					_DivisionId[_noOfCordinates] = -1;
					_SubDivisionId[_noOfCordinates] = -1;

					//handle divide by zero
					if (MagG != 0)
						_edgeMagnitude[_noOfCordinates] = 1 / MagG;  // gradient magnitude 
					else
						_edgeMagnitude[_noOfCordinates] = 0;

					_noOfCordinates++;
				}
			}
		}
	}

	if (sSunCnt != 0.0)
		_AvgEdges = nSumEdges / sSunCnt;

	_CogX = (float)(Ssize.height) * 0.5f;
	_CogY = (float)(Ssize.width) * 0.5f;

	for (int m = 0; m < _noOfCordinates; m++)
	{
		int temp;

		temp = _coordX[m];
		_coordX[m] = temp - _CogX;
		temp = _coordY[m];
		_coordY[m] = temp - _CogY;
	}
}
void GeoModel::Item::_HysterisisThreshold(cv::Mat gx, cv::Mat gy, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float maxContrast, float minContrast)
{
	int RSum(0), CSum(0);
	int curX(1), curY(1);
	int flag(1);

	int nSumEdges(0), sSunCnt(0);
	for (int i = 1; i < Ssize.height - 1; i++)
	{
		short * _sdx = gx.ptr<short>(i);
		short * _sdy = gy.ptr<short>(i);
		float * pEdges_n = nmsEdges.ptr<float>(i - 1);
		float * pEdges = nmsEdges.ptr<float>(i);
		float * pEdges_p = nmsEdges.ptr<float>(i + 1);

		for (int j = 1; j < (Ssize.width - 1); j++)
		{
			float fdx = _sdx[j], fdy = _sdy[j];

			float MagG = sqrt(fdx*fdx + fdy * fdy); //Magnitude = Sqrt(gx^2 +gy^2)
			float DirG = cv::fastAtan2((float)fdy, (float)fdx);	 //Direction = tan(y/x)

			////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]= MagG;
			flag = 1;
			if ((pEdges[j]) < maxContrast)
			{
				if (pEdges[j] < minContrast)
				{
					pEdges[j] = 0;
					flag = 0; // remove from edge
					////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
				}
				else
				{   // if any of 8 neighboring pixel is not greater than max contraxt remove from edge
					if ((pEdges_n[j - 1] < maxContrast) &&
						(pEdges_n[j] < maxContrast) &&
						(pEdges_n[j + 1] < maxContrast) &&
						(pEdges[j - 1] < maxContrast) &&
						(pEdges[j + 1] < maxContrast) &&
						(pEdges_p[j - 1] < maxContrast) &&
						(pEdges_p[j] < maxContrast) &&
						(pEdges_p[j + 1] < maxContrast))
					{
						pEdges[j] = 0;
						flag = 0;
						////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
					}
				}

			}

			// save selected edge information
			curX = i;	curY = j;
			if (flag != 0)
			{
				if (fdx != 0 || fdy != 0)
				{
					sSunCnt++;
					nSumEdges += pEdges[j];
					RSum = RSum + curX;	CSum = CSum + curY; // Row sum and column sum for center of gravity

					_coordX[_noOfCordinates] = curX;
					_coordY[_noOfCordinates] = curY;
					_edgeDerivativeX[_noOfCordinates] = fdx;
					_edgeDerivativeY[_noOfCordinates] = fdy;

					_DivisionId[_noOfCordinates] = -1;
					_SubDivisionId[_noOfCordinates] = -1;

					//handle divide by zero
					if (MagG != 0)
						_edgeMagnitude[_noOfCordinates] = 1 / MagG;  // gradient magnitude 
					else
						_edgeMagnitude[_noOfCordinates] = 0;

					_noOfCordinates++;
				}
			}
		}
	}

	if (sSunCnt != 0.0)
		_AvgEdges = nSumEdges / sSunCnt;

	_CogX = (float)(Ssize.height) * 0.5f;
	_CogY = (float)(Ssize.width) * 0.5f;

	for (int m = 0; m < _noOfCordinates; m++)
	{
		int temp;

		temp = _coordX[m];
		_coordX[m] = temp - _CogX;
		temp = _coordY[m];
		_coordY[m] = temp - _CogY;
	}
}


void GeoModel::Item::_Laplacian(cv::Mat & src, cv::Mat & gl, cv::Mat & mask)
{
	cv::Mat gauss;
	cv::GaussianBlur(src, gauss, cv::Size(7, 7), 0);
	cv::Laplacian(gauss, gl, CV_8U, 5);

}
void GeoModel::Item::_Calc_Dir_Lap(cv::Mat gx, cv::Mat gy, cv::Mat gl, float * magMat, int * orients, cv::Size Ssize, float & MaxGradient)
{
	MaxGradient = -HUGE_VAL;
	int count = 0;
	for (int i = 1; i < Ssize.height - 1; i++)
	{
		short * psdx = gx.ptr<short>(i);
		short * psdy = gy.ptr<short>(i);
		BYTE * pgl = gl.ptr<BYTE>(i);
		for (int j = 1; j < Ssize.width - 1; j++)
		{
			float fdx = psdx[j], fdy = psdy[j];        // read x, y derivatives
			float MagG = pgl[j];
			float direction = cv::fastAtan2((float)fdx, (float)fdy);	 //Direction = invtan (Gy / Gx)
			magMat[i*Ssize.width + j] = MagG;
			if (MagG > MaxGradient)
				MaxGradient = MagG; // get maximum gradient value for normalizing.
			if ((direction > 0 && direction < 22.5) || (direction > 157.5 && direction < 202.5) || (direction > 337.5 && direction < 360))
				direction = 0;
			else if ((direction > 22.5 && direction < 67.5) || (direction > 202.5 && direction < 247.5))
				direction = 45;
			else if ((direction > 67.5 && direction < 112.5) || (direction > 247.5 && direction < 292.5))
				direction = 90;
			else if ((direction > 112.5 && direction < 157.5) || (direction > 292.5 && direction < 337.5))
				direction = 135;
			else
				direction = 0;
			orients[count] = (int)direction;
			count++;
		}
	}
	if (MaxGradient > 255.0)
		MaxGradient = 255.0;
}
void GeoModel::Item::_NonMaximalSuppression_Lap(int * orients, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float MaxGradient)
{
	int count(0);
	const int nDiff(1);
	for (int i = nDiff; i < Ssize.height - nDiff; i++)
	{
		float * pEdges = nmsEdges.ptr<float>(i);
		for (int j = nDiff; j < Ssize.width - nDiff; j++)
		{
			float leftPixel(0.f), rightPixel(0.f);
			switch (orients[count])
			{
			case 0:
				leftPixel = magMat[(i - nDiff)*Ssize.width + j];
				rightPixel = magMat[(i + nDiff)*Ssize.width + j];
				break;
			case 45:
				leftPixel = magMat[(i - nDiff)*Ssize.width + j - nDiff];
				rightPixel = magMat[(i + nDiff)*Ssize.width + j + nDiff];
				break;
			case 90:
				leftPixel = magMat[(i)*Ssize.width + j - nDiff];
				rightPixel = magMat[(i)*Ssize.width + j + nDiff];
				break;
			case 135:
				leftPixel = magMat[(i - nDiff)*Ssize.width + j + nDiff];
				rightPixel = magMat[(i + nDiff)*Ssize.width + j - nDiff];
				break;
			}
			if ((magMat[i*Ssize.width + j] < leftPixel) || (magMat[i*Ssize.width + j] < rightPixel))
				pEdges[j] = 0;
			else
			{
				float fMag = magMat[i*Ssize.width + j] / MaxGradient;
				if (fMag > 1.0f)
					fMag = 1.0f;
				pEdges[j] = magMat[i*Ssize.width + j];//(uchar)(fMag*255);
			}
			count++;
		}
	}
}
void GeoModel::Item::_HysterisisThreshold_Lap(cv::Mat gl, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float maxContrast, float minContrast)
{
	int RSum(0), CSum(0);
	int curX(1), curY(1);
	int flag(1);
	int nSumEdges(0), sSunCnt(0);
	for (int i = 1; i < Ssize.height - 1; i++)
	{
		BYTE * psdx = gl.ptr<BYTE>(i);
		float * pEdges_n = nmsEdges.ptr<float>(i - 1);
		float * pEdges = nmsEdges.ptr<float>(i);
		float * pEdges_p = nmsEdges.ptr<float>(i + 1);
		for (int j = 1; j < (Ssize.width - 1); j++)
		{
			float fdx = psdx[j];
			float MagG = sqrt(fdx*fdx);
			flag = 1;
			if ((pEdges[j]) < maxContrast)
			{
				if (pEdges[j] < minContrast)
				{
					pEdges[j] = 0;
					flag = 0;
				}
				else
				{
					if ((pEdges_n[j - 1] < maxContrast) &&
						(pEdges_n[j] < maxContrast) &&
						(pEdges_n[j + 1] < maxContrast) &&
						(pEdges[j - 1] < maxContrast) &&
						(pEdges[j + 1] < maxContrast) &&
						(pEdges_p[j - 1] < maxContrast) &&
						(pEdges_p[j] < maxContrast) &&
						(pEdges_p[j + 1] < maxContrast))
					{
						pEdges[j] = 0;
						flag = 0;
					}
				}
			}
			curX = i;	curY = j;
			if (flag != 0)
			{
				if (fdx != 0)
				{
					sSunCnt++;
					nSumEdges += pEdges[j];
					RSum = RSum + curX;	CSum = CSum + curY; // Row sum and column sum for center of gravity
					_coordX[_noOfCordinates] = curX;
					_coordY[_noOfCordinates] = curY;
					_edgeDerivativeX[_noOfCordinates] = fdx;
					_edgeDerivativeY[_noOfCordinates] = 0;
					_DivisionId[_noOfCordinates] = -1;
					_SubDivisionId[_noOfCordinates] = -1;
					if (MagG != 0)
						_edgeMagnitude[_noOfCordinates] = 1 / MagG;  // gradient magnitude 
					else
						_edgeMagnitude[_noOfCordinates] = 0;
					_noOfCordinates++;
				}
			}
		}
	}
	if (sSunCnt != 0.0)
		_AvgEdges = nSumEdges / sSunCnt;
	_CogX = (float)(Ssize.height) * 0.5f;
	_CogY = (float)(Ssize.width) * 0.5f;
	for (int m = 0; m < _noOfCordinates; m++)
	{
		int temp;
		temp = _coordX[m];
		_coordX[m] = temp - _CogX;
		temp = _coordY[m];
		_coordY[m] = temp - _CogY;
	}
}


cv::Mat GeoModel::Item::algo_1(cv::Mat srcP, double dLowthre, double dHighthre, cv::Scalar mean, cv::Scalar stddev, cv::Size D_size, cv::Size U_size)
{
	cv::Mat cnres, cnres_d, cnres_U, dst;
	CString sOutput, sCanny, sCanny_U, sSrc;
	int m_median_size(0);

	if ((dLowthre == 0) && (dHighthre == 0))
	{
		dLowthre = (mean[0] + (2.0* stddev[0]));
		dHighthre = (mean[0] + (3.0* stddev[0]));
		if (dHighthre >= 255.0)
			dHighthre = 255.0;
		if (dLowthre >= 255.0)
			dLowthre = 255.0;
	}

	if (stddev[0] > 25.0)
	{
		m_median_size = 3;
		cv::medianBlur(srcP, srcP, m_median_size);
	}
	else if (stddev[0] >= 20.0 && stddev[0] <= 24.999999)
	{
		m_median_size = 5;
		cv::medianBlur(srcP, srcP, m_median_size);
	}
	else if (stddev[0] < 20.0 && stddev[0] >= 14.0)
	{
		m_median_size = 7;
		cv::medianBlur(srcP, srcP, m_median_size);
	}
	else if (stddev[0] < 14.0)
	{
		m_median_size = 3;
		cv::medianBlur(srcP, srcP, m_median_size);
	}

	cv::Canny(srcP, cnres, dLowthre, dHighthre, 3, false);
	cv::pyrDown(srcP, cnres_d, D_size, 4);
	cv::Canny(cnres_d, cnres_d, dLowthre, dHighthre, 3, false);
	cv::pyrUp(cnres_d, cnres_U, U_size, 4);
	cv::bitwise_and(cnres, cnres_U, dst);

	return dst;
}
cv::Mat GeoModel::Item::algo_2(cv::Mat srcP, cv::Scalar mean, cv::Scalar stddev, int width, int height)
{

	int m_median_size(0);

	if (stddev[0] > 25.0)
	{
		m_median_size = 3;
		cv::medianBlur(srcP, srcP, m_median_size);
	}
	else if (stddev[0] >= 20.0 && stddev[0] <= 24.999999)
	{
		m_median_size = 5;
		cv::medianBlur(srcP, srcP, m_median_size);
	}
	else if (stddev[0] < 20.0 && stddev[0] >= 14.0)
	{
		m_median_size = 7;
		cv::medianBlur(srcP, srcP, m_median_size);
	}
	else if (stddev[0] < 14.0 && stddev[0] >= 10.0)
	{
		m_median_size = 9;
		cv::medianBlur(srcP, srcP, m_median_size);
	}

	if (CV_MAT_TYPE(srcP.type()) != CV_8UC1)
	{
		cv::normalize(srcP, srcP, 0, 255, cv::NORM_MINMAX);
		srcP.convertTo(srcP, CV_8UC1);
	}
	cv::threshold(srcP, srcP, 0, 255, cv::THRESH_OTSU);
	//cv::Canny(srcP,srcP,0,0,3,false);
	cv::Mat srcP_CannyDst, srcP_Canny;
	cv::Canny(srcP, srcP_Canny, 0, 0, 3, false);

	srcP_CannyDst = noise_remove(srcP_Canny, srcP_Canny.rows, srcP_Canny.cols);
	int nCnt = 0;
	for (int r = 0; r < srcP_CannyDst.rows; r++)
	{
		for (int c = 0; c < srcP_CannyDst.cols; c++)
		{
			if (srcP_CannyDst.ptr(r)[c] > 0)
				nCnt++;
		}
	}
	int nszMin = srcP_CannyDst.cols < srcP_CannyDst.rows ? srcP_CannyDst.cols / 2 : srcP_CannyDst.rows / 2;
	if ((nCnt < nszMin))
		srcP_CannyDst = srcP_Canny;
	return srcP_CannyDst;
}
cv::Mat GeoModel::Item::algo_3(cv::Mat srcP, cv::Scalar mean, cv::Scalar stddev, int width, int height)
{
	cv::Mat src_W;
	int m_median_size(0);

	if (CV_MAT_TYPE(srcP.type()) != CV_8UC1)
	{
		cv::normalize(srcP, srcP, 0, 255, cv::NORM_MINMAX);
		srcP.convertTo(srcP, CV_8UC1);
	}
	srcP.copyTo(src_W);
	if (srcP.rows > 50 && srcP.cols > 50)
	{
		if (stddev[0] > 25.0)
		{
			m_median_size = 3;
			cv::medianBlur(src_W, src_W, m_median_size);
		}
		else if (stddev[0] >= 20.0 && stddev[0] <= 24.999999)
		{
			m_median_size = 5;
			cv::medianBlur(src_W, src_W, m_median_size);
		}
		else if (stddev[0] < 20.0 && stddev[0] >= 14.0)
		{
			m_median_size = 7;
			cv::medianBlur(src_W, src_W, m_median_size);
		}
		else if (stddev[0] < 14.0 && stddev[0] >= 10.0)
		{
			m_median_size = 9;
			cv::medianBlur(src_W, src_W, m_median_size);
		}
		else
		{
			m_median_size = 3;
			cv::medianBlur(src_W, src_W, m_median_size);
		}
	}
	double thre_value = cv::threshold(src_W, src_W, 0, 255, cv::THRESH_OTSU);

	cv::threshold(srcP, srcP, thre_value, 255, cv::THRESH_BINARY);

	cv::Mat srcP_CannyDst;
	cv::Canny(srcP, srcP_CannyDst, 0, 0, 3, false);
	//	srcP = noise_remove(srcP,srcP.rows, srcP.cols);

	return srcP_CannyDst;
}
cv::Mat GeoModel::Item::algo_4(cv::Mat srcP)
{
	cv::Mat src_W, src_W_1;
	cv::Scalar mean_1, stddev_1; // LWW test
	srcP.copyTo(src_W);

	if (CV_MAT_TYPE(src_W.type()) != CV_8UC1)
	{
		cv::normalize(src_W, src_W, 0, 255, cv::NORM_MINMAX);
		src_W.convertTo(src_W, CV_8UC1);
	}

	cv::bitwise_not(src_W, src_W);
	cv::meanStdDev(src_W, mean_1, stddev_1);

	double thre_value = mean_1[0] + (0.5*stddev_1[0]);

	cv::threshold(src_W, src_W, thre_value, 255, cv::THRESH_BINARY);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(srcP, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	int idx = 0;

	for (contours.begin(); idx < contours.size(); idx++)
	{
		cv::Rect bbox = cv::boundingRect(contours[idx]);
		double area = cv::contourArea(contours[idx]);
		if (area < 6)
		{
			cv::Scalar color(0, 0, 0);
			drawContours(src_W, contours, idx, color, 1, 8, hierarchy);
		}
	}

	//	Thinning(src_W);
	return src_W;
}
cv::Mat GeoModel::Item::algo_6(cv::Mat srcP)
{
	cv::Mat src_W, src_W_1;
	cv::Scalar mean_1, stddev_1; // LWW test
	srcP.copyTo(src_W);

	if (CV_MAT_TYPE(src_W.type()) != CV_8UC1)
	{
		cv::normalize(src_W, src_W, 0, 255, cv::NORM_MINMAX);
		src_W.convertTo(src_W, CV_8UC1);
	}
	cv::meanStdDev(src_W, mean_1, stddev_1);

	double thre_value = mean_1[0] + (0.5*stddev_1[0]);

	cv::threshold(src_W, src_W, thre_value, 255, cv::THRESH_BINARY);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(srcP, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	int idx = 0;

	for (contours.begin(); idx < contours.size(); idx++)
	{
		cv::Rect bbox = cv::boundingRect(contours[idx]);
		double area = cv::contourArea(contours[idx]);
		if (area < 6)
		{
			cv::Scalar color(0, 0, 0);
			drawContours(src_W, contours, idx, color, 1, 8, hierarchy);
		}
	}

#ifdef GeoSaveDebug
	CString sSrc_image_path;
	sSrc_image_path.Format(_T("d:\\testimage\\workimage\\THRE_.bmp"));
	cv::imwrite(std::string(CT2A(sSrc_image_path)), src_W);
#endif

	//	Thinning(src_W);

	return src_W;
}
void GeoModel::Item::Thinning(cv::Mat src)
{
	register int x, y;
	int ca, cb, cc, cd, tot;
	int np1, sp1, hv;
	int cnt, chk, flag, cz;

	int nWidth = src.cols;
	int nHeight = src.rows;

	cv::Mat dst, res;
	src.copyTo(dst);
	src.copyTo(res);
	//dst.setTo(0.0);
	//res.setTo(0.0);

	BYTE* ptr = src.data;
	BYTE* dstptr = dst.data;
	BYTE* resptr = res.data;

	ca = cb = cc = cd = np1 = sp1 = cnt = chk = flag = cz = 0;

	//복사
	// 	for (y=0 ; y<nHeight ; y++)
	// 	{
	// 		for (x=0 ; x<nWidth ; x++)
	// 		{
	// 			resptr[y*nWidth+x]=ptr[y*nWidth+x];
	// 		}
	// 	}

	do
	{
		// 		for (y=0 ; y<nHeight ; y++) //초기화
		// 		{
		// 			for (x=0 ; x<nWidth ; x++)
		// 			{
		// 				dstptr[y*nWidth+x] = 0;
		// 			}
		// 		}
		dst.setTo(0.0);

		flag = 0;
		chk = cnt % 2;
		cnt++;

		for (y = 1; y < nHeight - 1; y++)
		{
			for (x = 1; x < nWidth - 1; x++)
			{
				if (resptr[y*nWidth + x] == 255)
				{
					ca = cb = cc = cd = sp1 = 0;

					np1 = resptr[(y - 1)*nWidth + (x - 1)] + resptr[(y - 1)*nWidth + x] + resptr[(y - 1)*nWidth + (x + 1)] +
						resptr[y*nWidth + (x - 1)] + resptr[y*nWidth + (x + 1)] +
						resptr[(y + 1)*nWidth + (x - 1)] + resptr[(y + 1)*nWidth + x] + resptr[(y + 1)*nWidth + (x + 1)];

					if (np1 >= 2 * 255 && np1 <= 6 * 255)	ca = 0;
					else ca = 1;

					if (resptr[(y - 1)*nWidth + x] == 0 && resptr[(y - 1)*nWidth + (x + 1)] == 255)	sp1++;
					if (resptr[(y - 1)*nWidth + (x + 1)] == 0 && resptr[(y)*nWidth + (x + 1)] == 255)	sp1++;
					if (resptr[(y)*nWidth + (x + 1)] == 0 && resptr[(y + 1)*nWidth + (x + 1)] == 255)	sp1++;
					if (resptr[(y + 1)*nWidth + (x + 1)] == 0 && resptr[(y + 1)*nWidth + (x)] == 255)	sp1++;
					if (resptr[(y + 1)*nWidth + (x)] == 0 && resptr[(y + 1)*nWidth + (x - 1)] == 255)	sp1++;
					if (resptr[(y + 1)*nWidth + (x - 1)] == 0 && resptr[(y)*nWidth + (x - 1)] == 255)	sp1++;
					if (resptr[(y)*nWidth + (x - 1)] == 0 && resptr[(y - 1)*nWidth + (x - 1)] == 255)	sp1++;
					if (resptr[(y - 1)*nWidth + (x - 1)] == 0 && resptr[(y - 1)*nWidth + (x)] == 255)	sp1++;

					if (sp1 == 1)	cb = 0;
					else	cb = 1;

					if (chk == 0)
					{
						cc = resptr[(y - 1)*nWidth + x] * resptr[y*nWidth + (x + 1)] * resptr[(y + 1)*nWidth + x];
						cd = resptr[y*nWidth + (x + 1)] * resptr[(y + 1)*nWidth + x] * resptr[y*nWidth + (x - 1)];
					}
					else
					{
						cc = resptr[(y - 1)*nWidth + x] * resptr[y*nWidth + (x + 1)] * resptr[y*nWidth + (x - 1)];
						cd = resptr[(y - 1)*nWidth + x] * resptr[(y + 1)*nWidth + x] * resptr[y*nWidth + (x - 1)];
					}

					tot = ca || cb || cc || cd;

					if (tot)	dstptr[y*nWidth + x] = 255;
					else	flag = 1;
				}
			}
		}

		for (y = 0; y < nHeight; y++)
		{
			for (x = 0; x < nWidth; x++)
			{
				resptr[y*nWidth + x] = dstptr[y*nWidth + x];
			}
		}

	} while (flag);

	for (y = 1; y < nHeight - 1; y++)
	{
		for (x = 1; x < nWidth - 1; x++)
		{
			hv = 0;

			if (resptr[y*nWidth + x] == 255)
			{
				if (resptr[(y - 1)*nWidth + x] == 255 && resptr[y*nWidth + (x + 1)] == 255)	hv++;
				if (resptr[y*nWidth + (x + 1)] == 255 && resptr[(y + 1)*nWidth + x] == 255)	hv++;
				if (resptr[(y + 1)*nWidth + x] == 255 && resptr[y*nWidth + (x - 1)] == 255)	hv++;
				if (resptr[y*nWidth + (x - 1)] == 255 && resptr[(y - 1)*nWidth + x] == 255)	hv++;

				if (hv == 1)	resptr[y*nWidth + x] = 0;
			}
		}
	}

	for (y = 0; y < nHeight; y++)
	{
		for (x = 0; x < nWidth; x++)
		{
			ptr[y*nWidth + x] = resptr[y*nWidth + x];
		}
	}
}
cv::Mat GeoModel::Item::noise_remove(cv::Mat srcP, int width, int height)
{
	if (srcP.rows <= 80 && srcP.cols <= 80)
	{
		return srcP;
	}

	cv::Mat src_W;
	srcP.copyTo(src_W);
	src_W.setTo(0);


	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	int limit_box_area = 50;
	cv::findContours(srcP, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	int idx = 0;

	if (limit_box_area > (width * height * 0.03))
		limit_box_area = 50;
	else if (300 < (width * height * 0.03))
		limit_box_area = 300;
	else
		limit_box_area = width * height * 0.05;


	for (contours.begin(); idx < contours.size(); idx++)
	{
		cv::Rect bbox = cv::boundingRect(contours[idx]);

		if (bbox.area() > limit_box_area)
		{
			cv::Scalar color(255.0, 255.0, 255.0);
			drawContours(src_W, contours, idx, color, 1, 8, hierarchy);
		}
	}

#ifdef GeoSaveDebug
	CString save_path;

	//save_path.Format(_T("d:\\testimage\\workimage\\models\\test\\canny\\contours_%d_%d.bmp"),width,height);
	//cv::imwrite(std::string(CT2A(save_path)), src_W);
#endif

	return src_W;
}


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =




GeoModel::GeoModel()
{
	_modelDefined = false;

	_bAngleFlag = true;
	_fAngleNeg = -5;
	_fAnglePos = 5;
	_fAngleStep = 2.0f;
	_fAngleStd = 0.0f;
	_fImgAngle = 0.0f;

	_Acceptance = 0.7f;
	_Speed = enmSpeed::spHigh;
	_Accuracy = enmAccuracy::acHigh;
	_Precision = enmPrecision::pcLow;

	_pyrDownStep = 1;

	_MaxScore = 1.0f;

	_OrgModelAngleIdx = -1;

	_FindSubPixel = true;

	_bEdgeEnhancement = true;

	_overlappedModelDist.x = 0;
	_overlappedModelDist.y = 0;

	for (int i = 0; i < enmDepth::spNum; i++)
		_PreProcLv[i] = 1;

	_Use_algo_model = enmMatchAlgo::agDefault;

	_bUseImgPreProc = true;

	_nPyraDownStepLimit = 4;

	_nAngleSearchStep = 1;

	_fSearchAngleSt = 0;
	_fSearchAngleEd = 0;

	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	setRes(0, 0);
	_Filter = 0;
}
GeoModel::~GeoModel()
{
	g_pMManager->pem_delete_check(this);
}


void GeoModel::CopyOf(GeoModel & md, float fImgAngle)
{
	Alloc(md._srcModel_color.empty() == true ? md.Image() : md.color_Image(), fImgAngle);
	setAcceptance(md.getAcceptance());
	setAccuracy(md.getAccuracy());
	setFindSubPixel(md.getFindSubPixel());
	setModelFilterType(md.getModelFilterType());



	setMatchAlgo(md.getMatchAlgo());
	bool bEn(false);
	float fdNeg(0.0f), fdPos(0.0f), fdStep(0.0f);
	md.getAngleRange(bEn, fdNeg, fdPos, fdStep);
	setAngleRange(bEn, fdNeg, fdPos, fdStep);
	setModelFilterType(md.getModelFilterType());
	_bUseImgPreProc = false;
	setModelnChannel(md.getModelnChannel());
	Preprocess();
}
void GeoModel::Alloc(cv::Mat src, float fImgAngle)
{
	_fImgAngle = fImgAngle;

	if (src.channels() == 3)
	{
		cv::Mat src_gray;
		cv::cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);

		_srcModel_color = src;
		_srcModel = src_gray;
	}
	else
		_srcModel = src;
	//src.channels() == 1 ? _srcModel = src :_srcModel_color = src ;
	_modelDefined = false;
}


bool GeoModel::SaveFile(CString sPath, bool bSzOptimiz)
{
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if (extension != _T(".ppd"))
		return false;

	CFile file;
	CFileException fe;
	if (file.Open(sPath, CFile::modeCreate | CFile::modeWrite, NULL, &fe) == FALSE)
		return false;

	CArchive ar(&file, CArchive::store);

	SaveFile(ar, bSzOptimiz);

	ar.Close();
	file.Close();

	return true;
}
bool GeoModel::SaveFile(CArchive& ar, bool bSzOptimiz, int nVersion)
{
	try
	{
		// 헤더
		CString sName(_T("pemtron"));
		int version = nVersion;

		ar << sName;

		// 기본 정보
		if (_srcModel.empty())
		{
			ar << 0;
			ar << 0;
		}
		else
		{
			ar << _srcModel.cols;
			ar << _srcModel.rows;

			for (int y = 0; y < _srcModel.rows; y++)
			{
				uchar * ptr = _srcModel.ptr(y);
				for (int x = 0; x < _srcModel.cols; x++)
					ar << ptr[x];
			}
		}

		if (bSzOptimiz)
			_modelDefined = false;

		ar << _pyrDownStep;
		ar << _modelDefined;

		ar << _fImgAngle;
		ar << _MaxScore;

		ar << _Acceptance;
		ar << (int)(_Speed);
		ar << (int)(_Accuracy);

		ar << _bAngleFlag;
		ar << _fAngleNeg;
		ar << _fAnglePos;
		ar << _fAngleStep;
		ar << _fAngleStd;


		// 세부 모델 정보
		int nZoomCount(_Models[enmDepth::spZoom].size()), nHalfCount(_Models[enmDepth::spHalf].size()), nOriginalCount(_Models[enmDepth::spOrg].size());

		if (bSzOptimiz)
		{
			nZoomCount = 0;
			nHalfCount = 0;
			nOriginalCount = 0;

			ar << nZoomCount;
			ar << nHalfCount;
			ar << nOriginalCount;
			_Models[enmDepth::spZoom].clear();
			_Models[enmDepth::spHalf].clear();
			_Models[enmDepth::spOrg].clear();
		}
		else
		{
			ar << nZoomCount;
			ar << nHalfCount;
			ar << nOriginalCount;

			for (auto pos = _Models[enmDepth::spZoom].begin(); pos != _Models[enmDepth::spZoom].end(); pos++)
			{
				ar << pos->first;
				pos->second->SaveFile(ar);
			}
			for (auto pos = _Models[enmDepth::spHalf].begin(); pos != _Models[enmDepth::spHalf].end(); pos++)
			{
				ar << pos->first;
				pos->second->SaveFile(ar);
			}
			for (auto pos = _Models[enmDepth::spOrg].begin(); pos != _Models[enmDepth::spOrg].end(); pos++)
			{
				ar << pos->first;
				pos->second->SaveFile(ar);
			}
		}

		ar << version;
		// 버전 별 추가 정보
		for (int i = 0; i < enmDepth::spNum; i++)
			ar << _PreProcLv[i];


		_SaveFile_v2(ar);
		_SaveFile_v3(ar);
		_SaveFile_v5(ar);
		_SaveFile_v6(ar);
		_SaveFile_v7(ar);
		_SaveFile_v8(ar);
		_SaveFile_v9(ar);
		_SaveFile_v10(ar);
		_SaveFile_v11(ar);
	}
	catch (CMemoryException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		csLog.Format(_T("Exception_GeoModel::SaveFile_%s"), szMsg);

		GeoWriteLog(csLog);
		e->Delete();
		return false;
	}
	catch (CFileException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("Exception_GeoModel::SaveFile_%s"), szMsg);


		GeoWriteLog(csLog);
		e->Delete();
		return false;
	}
	catch (CException* e)
	{
		CString csLog;

		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("Exception_GeoModel::SaveFile_%s"), szMsg);
	

		GeoWriteLog(csLog);
		e->Delete();
		return false;
	}
	catch (...)
	{

		GeoWriteLog(_T("Exception_GeoModel::SaveFile_UnknownError"));
		return false;
	}

	return true;
}
bool GeoModel::LoadFile(CString sPath, double resX, double resY)
{
	bool res = false;
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if (extension != _T(".ppd"))
		return res;

	_sLoadedPath = sPath;

	CFile file;
	CFileException fe;
	if (file.Open(sPath, CFile::modeRead, NULL, &fe) == FALSE)
		return false;


	CArchive ar(&file, CArchive::load, file.GetLength());

	res = LoadFile(ar, resX, resY);

	ar.Close();
	file.Close();

	return res;
}
bool GeoModel::ExtLoadFile(int nIndex, double resX, double resY)
{
	bool res = false;
	ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_PatternModelCtrl.First()[nIndex];
	CString sPath(pCtrl.sFilePath);
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if (extension != _T(".ppd"))
		return res;

	_sLoadedPath = sPath;

// 	CFile file;
// 	CFileException fe;
// 	if (file.Open(sPath, CFile::modeRead, NULL, &fe) == FALSE)
// 		return false;

	CMemFile file;
	CArchive arStore(&file, CArchive::store); // 공유메모리에서 가져와 셋팅.
	arStore.Write(ext::InspRoot_Server::get()->_PatternModelBuffer.Lock(pCtrl.img), pCtrl.img.imgSz/* static_cast<UINT>(fileSize)*/);
	arStore.Close();
	file.Flush();
	file.SeekToBegin();

	CArchive ar(&file, CArchive::load, file.GetLength());

	res = LoadFile(ar, resX, resY);

	ar.Close();
	file.Close();

	return res;
}
bool GeoModel::LoadFile(CArchive& ar, double resX, double resY)
{
	try
	{
		// 헤더
		CString sName(_T("pemtron")), sReadName;

		ar >> sReadName;
		if (sReadName != sName)
			return false;

		// 기본 정보 
		int Wid(0), Len(0);
		ar >> Wid;
		ar >> Len;
		if (Wid > 0 && Len > 0)
		{
			_srcModel = cv::Mat(Len, Wid, CV_8UC1);
			for (int y = 0; y < _srcModel.rows; y++)
			{
				uchar * ptr = _srcModel.ptr(y);
				for (int x = 0; x < _srcModel.cols; x++)
					ar >> ptr[x];
			}
		}
		else 
			return false;

		ar >> _pyrDownStep;
		ar >> _modelDefined;

		ar >> _fImgAngle;
		ar >> _MaxScore;

		ar >> _Acceptance;
		int nSpeed(0), nAccuracy(0);
		ar >> nSpeed;
		ar >> nAccuracy;

		_Speed = (enmSpeed)nSpeed;
		_Accuracy = (enmAccuracy)nAccuracy;

		ar >> _bAngleFlag;
		ar >> _fAngleNeg;
		ar >> _fAnglePos;
		ar >> _fAngleStep;
		ar >> _fAngleStd;

		_fSearchAngleSt = _fAngleNeg;
		_fSearchAngleEd = _fAnglePos;

		// 세부 모델 정보
		int nZoomCount(0), nHalfCount(0), nOriginalCount(0);

		ar >> nZoomCount;
		ar >> nHalfCount;
		ar >> nOriginalCount;
		if (nZoomCount == 0 && nHalfCount == 0 && nOriginalCount == 0)
		{
			_Models[enmDepth::spZoom].clear();
			_Models[enmDepth::spHalf].clear();
			_Models[enmDepth::spOrg].clear();
		}

		for (int i = 0; i < nZoomCount; i++)
		{

			typItemPtr item = CreateItem();//(new GeoModel::Item(this));
			int key(0);
			ar >> key;
			item->LoadFile(ar);
			item->_PyrDown = enmDepth::spZoom;
			_Models[enmDepth::spZoom][key] = item;
		}
		for (int i = 0; i < nHalfCount; i++)
		{

			typItemPtr item = CreateItem();//(new GeoModel::Item(this));
			int key(0);
			ar >> key;
			item->LoadFile(ar);
			item->_PyrDown = enmDepth::spHalf;
			_Models[enmDepth::spHalf][key] = item;
		}
		for (int i = 0; i < nOriginalCount; i++)
		{

			typItemPtr item = CreateItem();//(new GeoModel::Item(this));
			int key(0);
			ar >> key;
			item->LoadFile(ar);
			item->_PyrDown = enmDepth::spOrg;
			_Models[enmDepth::spOrg][key] = item;
			if (item->_bOriginal == true)
				_OrgModelAngleIdx = key;
		}

		// 버전 별 추가 정보
		int version(0);

		if (ar.IsBufferEmpty() == FALSE)
		{
			ar >> version;
			for (int i = 0; i < enmDepth::spNum; i++)
			{
				if (ar.IsBufferEmpty() == FALSE)
					ar >> _PreProcLv[i];
				else
					_PreProcLv[i] = 1;
			}


			_LoadFile_v2(ar, version);
			_LoadFile_v3(ar, version);
			_LoadFile_v4(ar, version, resX, resY);
			_LoadFile_v5(ar, version);
			_LoadFile_v6(ar, version);
			_LoadFile_v7(ar, version, resX, resY);
			_LoadFile_v8(ar, version);
			_LoadFile_v9(ar, version, resX, resY);
			_LoadFile_v10(ar, version);
			_LoadFile_v11(ar, version);
		}
		if (nZoomCount == 0 && nHalfCount == 0 && nOriginalCount == 0)
		{
			_modelDefined = false;
			Preprocess();
		}
		if (version < 8)
			Preprocess();

		return true;
	}
	catch (CMemoryException* e)
	{
		g_pMManager->pem_delete_check(this);
		CString csLog;
		csLog.Format(_T("Exception_GeoModel::LoadFile_CMemoryException"));
		GeoWriteLog(csLog);
		e->Delete();
		return false;
	}
	catch (CFileException* e)
	{
		g_pMManager->pem_delete_check(this);
		CString csLog;
		csLog.Format(_T("Exception_GeoModel::LoadFile_CFileException"));
		GeoWriteLog(csLog);
		e->Delete();
		return false;
	}
	catch(CArchiveException* e)
	{
		g_pMManager->pem_delete_check(this);
		CString csLog;
		csLog.Format(_T("Exception_GeoModel::LoadFile_CArchiveException"));
		GeoWriteLog(csLog);
		e->Delete();
		return false;
	}
	catch (CException* e)
	{
		g_pMManager->pem_delete_check(this);
		CString csLog;

		//TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		csLog.Format(_T("Exception_GeoModel::LoadFile_CException"));
		GeoWriteLog(csLog);
		e->Delete();
		return false;
	}
	catch(...)
	{
		g_pMManager->pem_delete_check(this);
		GeoWriteLog(_T("Exception_GeoModel::LoadFile_UnknownError"));
		return false;
	}
}

bool GeoModel::_SaveFile_v5(CArchive & ar)
{
	return true;
}

bool GeoModel::_LoadFile_v5(CArchive & ar, int version)
{
	return true;
}

bool GeoModel::_SaveFile_v6(CArchive & ar)
{
	ar << (int)(_Precision);
	return true;
}

bool GeoModel::_LoadFile_v6(CArchive & ar, int version)
{
	int nPrecision(enmPrecision::pcLow);
	if(version>5 && (ar.IsBufferEmpty() == FALSE))
		ar >> nPrecision;

	_Precision = (enmPrecision)nPrecision;
	return true;
}

bool GeoModel::_SaveFile_v7(CArchive & ar)
{
	ar << _resolX;
	ar << _resolY;
	return true;
}

bool GeoModel::_LoadFile_v7(CArchive & ar, int version, double resX, double resY)
{
	if (version > 6)
	{
		ar >> _resolX;
		ar >> _resolY;

		reSizeModel(resX, resY);
		Preprocess();
	}
	else
	{
		_resolX = 0;
		_resolY = 0;
	}

	return true;
}
bool GeoModel::_SaveFile_v8(CArchive & ar)
{
	return true;
}
bool GeoModel::_LoadFile_v8(CArchive & ar, int version)
{
	return true;
}
bool GeoModel::_SaveFile_v9(CArchive & ar)
{
	int type = _srcModel.type();
	ar << type;
	if (_srcModel.type() == CV_32FC1)
	{

		if (_srcModel.empty())
		{
			ar << 0;
			ar << 0;
		}
		else
		{
			ar << _srcModel.cols;
			ar << _srcModel.rows;

			for (int y = 0; y < _srcModel.rows; y++)
			{
				float * ptr = _srcModel.ptr<float>(y);
				for (int x = 0; x < _srcModel.cols; x++)
					ar << ptr[x];
			}
		}
		ar << _resolX;
		ar << _resolY;

	}
	return true;
}
bool GeoModel::_LoadFile_v9(CArchive & ar, int version, double resX, double resY)
{
	if (version > 8 && (ar.IsBufferEmpty() == FALSE))
	{
		int type;
		ar >> type;
		if (type == CV_32FC1)
		{

			int Wid(0), Len(0);
			ar >> Wid;
			ar >> Len;
			if (Wid > 0 && Len > 0 && !_srcModel.empty() && Wid == _srcModel.cols && Len == _srcModel.rows)
			{
				_srcModel = cv::Mat(Len, Wid, CV_32FC1);
				for (int y = 0; y < _srcModel.rows; y++)
				{
					float * ptr = _srcModel.ptr<float>(y);
					for (int x = 0; x < _srcModel.cols; x++)
						ar >> ptr[x];
				}
			}

			ar >> _resolX;
			ar >> _resolY;
			reSizeModel(resX, resY);
			Preprocess();
		}
	}
	return true;
}
bool GeoModel::_SaveFile_v10(CArchive & ar)
{
	ar << _Filter;
	return true;
}
bool GeoModel::_LoadFile_v10(CArchive & ar, int version)
{
	if (version > 9 && (ar.IsBufferEmpty() == FALSE))
	{
		ar >> _Filter;
		if (_Filter < 0 || _Filter>1)
			_Filter = 0;
	}
	else
	{
		_Filter = 0;
	}

	return true;
}
bool GeoModel::_SaveFile_v11(CArchive & ar)
{
	ar << _halfModel.cols;
	ar << _halfModel.rows;

	for (int y = 0; y < _halfModel.rows; y++)
	{
		uchar * ptr = _halfModel.ptr(y);
		ar.Write(ptr, _halfModel.cols);
	}

	ar << _pyrModel.cols;
	ar << _pyrModel.rows;

	for (int y = 0; y < _pyrModel.rows; y++)
	{
		uchar * ptr = _pyrModel.ptr(y);
		ar.Write(ptr, _pyrModel.cols);
	}

	return true;
}
bool GeoModel::_LoadFile_v11(CArchive & ar, int version)
{
	//0도 모델일때만 전처리된것그대로 로드 하기
	if (version < 11 || _modelDefined == false)
		return false;

	int hfWid = 0;
	int hfLen = 0;
	ar >> hfWid;
	ar >> hfLen;
	if (hfLen > 0 && hfWid > 0)
	{
		_halfModel = cv::Mat(hfLen, hfWid, CV_8UC1);
		for (int y = 0; y < _halfModel.rows; y++)
		{
			uchar * ptr = _halfModel.ptr(y);
			ar.Read(ptr, _halfModel.cols);
		}
	}

	int pyWid = 0;
	int pyLen = 0;
	ar >> pyWid;
	ar >> pyLen;
	if (pyLen > 0 && pyWid > 0)
	{
		_pyrModel = cv::Mat(pyLen, pyWid, CV_8UC1);
		for (int y = 0; y < _pyrModel.rows; y++)
		{
			uchar * ptr = _pyrModel.ptr(y);
			ar.Read(ptr, _pyrModel.cols);
		}
	}

	return true;
}


bool GeoModel::_SaveFile_v2(CArchive & ar)
{
	ar << _Use_algo_model; // version 2 에 추가
	ar << _FindSubPixel;

	for (auto pos = _Models[enmDepth::spZoom].begin(); pos != _Models[enmDepth::spZoom].end(); pos++)
		pos->second->SaveFile_v2(ar);
	for (auto pos = _Models[enmDepth::spHalf].begin(); pos != _Models[enmDepth::spHalf].end(); pos++)
		pos->second->SaveFile_v2(ar);
	for (auto pos = _Models[enmDepth::spOrg].begin(); pos != _Models[enmDepth::spOrg].end(); pos++)
		pos->second->SaveFile_v2(ar);

	return true;
}

bool GeoModel::_LoadFile_v2(CArchive & ar, int ver)
{

	if (ver >= 2 && ar.IsBufferEmpty() == FALSE)
	{
		ar >> _Use_algo_model; // version 2 에 추가
		setMatchAlgo(_Use_algo_model);
	}
	else
		_Use_algo_model = enmMatchAlgo::agDefault;
	if (ver >= 2 && ar.IsBufferEmpty() == FALSE)
		ar >> _FindSubPixel;
	else
		_FindSubPixel = false;

	for (auto pos = _Models[enmDepth::spZoom].begin(); pos != _Models[enmDepth::spZoom].end(); pos++)

		pos->second->LoadFile_v2(ar, ver);
	for (auto pos = _Models[enmDepth::spHalf].begin(); pos != _Models[enmDepth::spHalf].end(); pos++)

		pos->second->LoadFile_v2(ar, ver);
	for (auto pos = _Models[enmDepth::spOrg].begin(); pos != _Models[enmDepth::spOrg].end(); pos++)

		pos->second->LoadFile_v2(ar, ver);

	return true;
}

bool GeoModel::_SaveFile_v3(CArchive & ar)
{
	for (auto pos = _Models[enmDepth::spZoom].begin(); pos != _Models[enmDepth::spZoom].end(); pos++)
		pos->second->SaveFile_v3(ar);
	for (auto pos = _Models[enmDepth::spHalf].begin(); pos != _Models[enmDepth::spHalf].end(); pos++)
		pos->second->SaveFile_v3(ar);
	for (auto pos = _Models[enmDepth::spOrg].begin(); pos != _Models[enmDepth::spOrg].end(); pos++)
		pos->second->SaveFile_v3(ar);

	return true;
}
bool GeoModel::_LoadFile_v3(CArchive & ar, int ver)
{
	for (auto pos = _Models[enmDepth::spZoom].begin(); pos != _Models[enmDepth::spZoom].end(); pos++)
		pos->second->LoadFile_v3(ar, ver);
	for (auto pos = _Models[enmDepth::spHalf].begin(); pos != _Models[enmDepth::spHalf].end(); pos++)
		pos->second->LoadFile_v3(ar, ver);
	for (auto pos = _Models[enmDepth::spOrg].begin(); pos != _Models[enmDepth::spOrg].end(); pos++)
		pos->second->LoadFile_v3(ar, ver);

	return true;
}

bool GeoModel::_LoadFile_v4(CArchive & ar, int ver, double resX, double resY)
{
	if (ver < 4 && (getMatchAlgo() == enmMatchAlgo::agIntaglio || getMatchAlgo() == enmMatchAlgo::agRelief) && _sLoadedPath.IsEmpty() == FALSE)
	{
		_modelDefined = false;
		for (int i = 0; i < enmDepth::spNum; i++)
			_Models[i].clear();
		_bUseImgPreProc = false;
		Preprocess();
		setRes(resX,resY);
		SaveFile(_sLoadedPath, true);
	}

	return true;
}

cv::Mat GeoModel::_ImgPreProcess(cv::Mat src)
{
	cv::Mat SrcBlur;
	if (_bUseImgPreProc)
	{
		if (src.cols < 51 || src.rows < 51)
			return src;
		if (getMatchAlgo() == GeoModel::enmMatchAlgo::agIntaglio || getMatchAlgo() == GeoModel::enmMatchAlgo::agRelief)
			cv::GaussianBlur(src, SrcBlur, cv::Size(3, 3), 0);
		else
			cv::medianBlur(src, SrcBlur, 3);
	}
	else
		_srcModel.copyTo(SrcBlur);

	return SrcBlur;
}

void GeoModel::Preprocess()
{
	//2022.01.13 shkim ImageType Div Insp
	//if(_srcModel_color.empty() != true)
	if (getMatchAlgo() == enmMatchAlgo::agImage && _srcModel_color.empty() != true/*(_srcModel_color.empty() != true || _srcModel.empty() != true)*/)
	{
		cv::cvtColor(_srcModel_color, _srcModel, cv::COLOR_BGR2GRAY);
		// 		_modelDefined = true;
		// 		return;
	}
	if (_srcModel.empty() == true || _modelDefined == true)
		return;

#ifdef GeoSaveDebug // LWW 2017/03/20
	CString sSrc_image_path;
	sSrc_image_path.Format(_T("d:\\testimage\\workimage\\canny_src.bmp"));
	cv::imwrite(std::string(CT2A(sSrc_image_path)), _srcModel);
#endif

	//필터링
#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\GeoMatchSrc.bmp", _srcModel);
#endif
	if (getModelFilterType() > 0)
	{
		cv::Mat ImageStandard, NomalImg, MedianMatOrg, MedianMat, thTemp;
		double thre_value1 = cv::threshold(_srcModel, thTemp, 0, 255, cv::THRESH_OTSU);
		cv::medianBlur(_srcModel, MedianMatOrg, 3);
		MedianMatOrg.convertTo(MedianMat, CV_16SC1);

		_srcModel.convertTo(ImageStandard, CV_16SC1);
		cv::Mat OMSub = MedianMat - ImageStandard;
		cv::Mat ImgADDM = ImageStandard + OMSub;
		cv::normalize(ImgADDM, NomalImg, 0, 255, cv::NORM_MINMAX);
		NomalImg.convertTo(_srcModel, CV_8UC1);
		double thre_value2 = cv::threshold(_srcModel, thTemp, 0, 255, cv::THRESH_OTSU);

		_srcModel = _srcModel - (thre_value2 - thre_value1);
	}

	_modelDefined = false;

	float maxContrast = 20;
	float minContrast = 10;


	cv::Mat SrcBlur = _ImgPreProcess(_srcModel);

	cv::Mat md = SrcBlur;
	cv::Mat pyr;
	_pyrDownStep = 0;
	do
	{
		pyr.release();
		_GetPyrDown(md, pyr);
		md = pyr;
		_pyrDownStep++;
		if (_pyrDownStep == 1)
			_halfModel = pyr;

		if (_pyrDownStep >= _nPyraDownStepLimit)
			break;
	} while (_CheckPyrSz(pyr));
	_pyrModel = pyr;


	// 각도 별 모델 생성
	//if(getMatchAlgo() != enmMatchAlgo::agImage)
	{
		_OrgModelAngleIdx = -1;
		cv::Mat * srcList[3] = { &_pyrModel, &_halfModel, &SrcBlur };
		// 	#ifndef GeoNoUseMultiThread
		// 		Concurrency::parallel_for((size_t)0, (size_t)enmDepth::spNum, [&](size_t i)
		// 	#else
		for (int i = 0; i < enmDepth::spNum; i++)
			// 	#endif
		{
			_PreprocessAngle(*srcList[i], maxContrast, minContrast, _Models[i], i);
		}
		// 	#ifndef GeoNoUseMultiThread
		// 		);
		// 	#endif
	}



#ifdef GeoSaveDebug
	if (getMatchAlgo() != enmMatchAlgo::agImage)
	{
		GeoResult res(1);
		GeoMatch gm;
		gm._ImgProcess(SrcBlur, _PreProcLv[enmDepth::spOrg], enmDepth::spOrg, getMatchAlgo());
		gm._FindModel(SrcBlur, _Models[enmDepth::spOrg][getOrgAngleIdx()], getOrgAngleIdx(), cv::Point(SrcBlur.cols / 2 - 4, SrcBlur.rows / 2 - 4), cv::Size(8, 8), _fAngleStd, 0.0, 0.9, res, getMatchAlgo());
		res.setResCnt(1);

		CString sName, sName2, sdst_image_path;
		sName.Format(_T("d:\\model_%.0f.bmp"), getImageAngle());
		cv::imwrite(std::string(CT2A(sName)), SrcBlur);

		sName2.Format(_T("d:\\model_draw_%.0f.bmp"), getImageAngle());


		//sdst_image_path.Format(_T("d:\\testimage\\workimage\\models\\test\\canny\\dst.bmp"));



		cv::Mat draw;
		_srcModel.copyTo(draw);
		gm.DrawContours(draw, _Models[enmDepth::spOrg][getOrgAngleIdx()], res, cv::Scalar(255), 1);
		cv::imwrite(std::string(CT2A(sName2)), draw);

		//cv::imwrite(std::string(CT2A(sdst_image_path)), draw);
	}
#endif

	_modelDefined = true;
}
void GeoModel::RePreprocess()
{
	//if(_srcModel_color.empty() != true)
	if (getMatchAlgo() == enmMatchAlgo::agImage && (_srcModel_color.empty() != true || _srcModel.empty() != true))
	{
		_modelDefined = true;
		return;
	}
	if (_srcModel.empty() == true)
		return;

#ifdef GeoSaveDebug // LWW 2017/03/20
	CString sSrc_image_path;
	sSrc_image_path.Format(_T("d:\\testimage\\workimage\\canny_src.bmp"));
	cv::imwrite(std::string(CT2A(sSrc_image_path)), _srcModel);
#endif

	//필터링
#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\GeoMatchSrc.bmp", _srcModel);
#endif
	if (getModelFilterType() > 0)
	{
		cv::Mat ImageStandard, NomalImg, MedianMatOrg, MedianMat, thTemp;
		double thre_value1 = cv::threshold(_srcModel, thTemp, 0, 255, cv::THRESH_OTSU);
		cv::medianBlur(_srcModel, MedianMatOrg, 3);
		MedianMatOrg.convertTo(MedianMat, CV_16SC1);

		_srcModel.convertTo(ImageStandard, CV_16SC1);
		cv::Mat OMSub = MedianMat - ImageStandard;
		cv::Mat ImgADDM = ImageStandard + OMSub;
		cv::normalize(ImgADDM, NomalImg, 0, 255, cv::NORM_MINMAX);
		NomalImg.convertTo(_srcModel, CV_8UC1);
		double thre_value2 = cv::threshold(_srcModel, thTemp, 0, 255, cv::THRESH_OTSU);

		_srcModel = _srcModel - (thre_value2 - thre_value1);
	}

	_modelDefined = false;

	float maxContrast = 20;
	float minContrast = 10;


	cv::Mat SrcBlur = _ImgPreProcess(_srcModel);

	cv::Mat md = SrcBlur;
	cv::Mat pyr;
	_pyrDownStep = 0;
	if (md.rows > 2 && md.cols > 2)
	{
		do
		{
			pyr.release();
			_GetPyrDown(md, pyr);
			md = pyr;
			_pyrDownStep++;
			if (_pyrDownStep == 1)
				_halfModel = pyr;

			if (_pyrDownStep >= _nPyraDownStepLimit)
				break;
		} while (_CheckPyrSz(pyr));
	}
	_pyrModel = pyr;


	// 각도 별 모델 생성
	if (getMatchAlgo() != enmMatchAlgo::agImage)
	{
		_OrgModelAngleIdx = -1;

		cv::Mat * srcList[3] = { &_pyrModel, &_halfModel, &SrcBlur };
		// 	#ifndef GeoNoUseMultiThread
		// 		Concurrency::parallel_for((size_t)0, (size_t)enmDepth::spNum, [&](size_t i)
		// 	#else
		for (int i = 0; i < enmDepth::spNum; i++)
			// 	#endif
		{
			if (srcList[i]->empty() == true)
				continue;
			_PreprocessAngle(*srcList[i], maxContrast, minContrast, _Models[i], i);
		}
		// 	#ifndef GeoNoUseMultiThread
		// 		);
		// 	#endif
	}



#ifdef GeoSaveDebug
	if (getMatchAlgo() != enmMatchAlgo::agImage)
	{
		GeoResult res(1);
		GeoMatch gm;
		gm._ImgProcess(SrcBlur, _PreProcLv[enmDepth::spOrg], enmDepth::spOrg, getMatchAlgo());
		gm._FindModel(SrcBlur, _Models[enmDepth::spOrg][getOrgAngleIdx()], getOrgAngleIdx(), cvPoint(SrcBlur.cols / 2 - 4, SrcBlur.rows / 2 - 4), cvSize(8, 8), _fAngleStd, 0.0, 0.9, res, getMatchAlgo());
		res.setResCnt(1);

		CString sName, sName2, sdst_image_path;
		sName.Format(_T("d:\\model_%.0f.bmp"), getImageAngle());
		cv::imwrite(std::string(CT2A(sName)), SrcBlur);

		sName2.Format(_T("d:\\model_draw_%.0f.bmp"), getImageAngle());


		//sdst_image_path.Format(_T("d:\\testimage\\workimage\\models\\test\\canny\\dst.bmp"));



		cv::Mat draw;
		_srcModel.copyTo(draw);
		gm.DrawContours(draw, _Models[enmDepth::spOrg][getOrgAngleIdx()], res, cv::Scalar(255), 1);
		cv::imwrite(std::string(CT2A(sName2)), draw);

		//cv::imwrite(std::string(CT2A(sdst_image_path)), draw);
	}
#endif

	_modelDefined = true;
}
void GeoModel::ReCalculation(cv::Mat src)
{
	if (src.empty() == true || _modelDefined == false)
		return;

#ifdef GeoSaveDebug // LWW 2017/03/20
	CString sSrc_image_path;
	sSrc_image_path.Format(_T("d:\\testimage\\workimage\\canny_src.bmp"));
	cv::imwrite(std::string(CT2A(sSrc_image_path)), _srcModel);
#endif

	if (getModelFilterType() > 0)
	{
		cv::Mat ImageStandard, NomalImg, MedianMatOrg, MedianMat, thTemp;
		double thre_value1 = cv::threshold(src, thTemp, 0, 255, cv::THRESH_OTSU);
		cv::medianBlur(src, MedianMatOrg, 3);
		MedianMatOrg.convertTo(MedianMat, CV_16SC1);

		src.convertTo(ImageStandard, CV_16SC1);
		cv::Mat OMSub = MedianMat - ImageStandard;
		cv::Mat ImgADDM = ImageStandard + OMSub;
		cv::normalize(ImgADDM, NomalImg, 0, 255, cv::NORM_MINMAX);
		NomalImg.convertTo(src, CV_8UC1);
		double thre_value2 = cv::threshold(src, thTemp, 0, 255, cv::THRESH_OTSU);

		src = src - (thre_value2 - thre_value1);
	}
	float maxContrast = 100;
	float minContrast = 20;


	cv::Mat SrcBlur = _ImgPreProcess(src);

	cv::Mat md = SrcBlur;
	cv::Mat pyr;
	_pyrDownStep = 0;
	do
	{
		pyr.release();
		_GetPyrDown(md, pyr);
		md = pyr;
		_pyrDownStep++;
		if (_pyrDownStep == 1)
			_halfModel = pyr;

		if (_pyrDownStep >= _nPyraDownStepLimit)
			break;
	} while (_CheckPyrSz(pyr));
	_pyrModel = pyr;


	// 각도 별 모델 생성
	if (getMatchAlgo() != enmMatchAlgo::agImage)
	{
		cv::Mat * srcList[3] = { &_pyrModel, &_halfModel, &SrcBlur };

		for (int i = 0; i < enmDepth::spNum; i++)
			_PreprocessAngle(*srcList[i], maxContrast, minContrast, _Models[i], i, GeoModel::cmProProc);
	}
}

void GeoModel::setAngleRange(bool bEnable, float fDeltaAngNeg, float fDeltaAngPos, float fStepAng)
{
	if (_bAngleFlag != bEnable || _fAngleNeg != -fDeltaAngNeg || _fAnglePos != fDeltaAngPos || _fAngleStep != fStepAng)
		_modelDefined = false;

	_bAngleFlag = bEnable;
	_fSearchAngleSt = _fAngleNeg = -fDeltaAngNeg;
	_fSearchAngleEd = _fAnglePos = fDeltaAngPos;
	_fAngleStep = fStepAng;
}
void GeoModel::getAngleRange(bool & bEnable, float & fDeltaAngNeg, float & fDeltaAngPos, float & fStepAng)
{
	bEnable = _bAngleFlag;
	fDeltaAngNeg = -_fAngleNeg;
	fDeltaAngPos = _fAnglePos;
	fStepAng = _fAngleStep;
}


void GeoModel::_PreprocessAngle(cv::Mat src, float maxContrast, float minContrast, std::map<int, GeoModel::typItemPtr> & mdList, int nPyrDepth, enmCalcMode cMode)
{
	float stdAngle = _fImgAngle;

	cv::Mat mask = cv::Mat(src.rows, src.cols, CV_8UC1, cv::Scalar(255));
	if (_bAngleFlag == true)
	{
		float angle(0);
		int nStep(0);

		GeoModel::typItemPtr orgItem;
		if (cMode == enmCalcMode::cmPreProc)
			orgItem = _MakeModel(src, mask, stdAngle, -1, maxContrast, minContrast, mdList, nPyrDepth, true, cMode);
		else
			orgItem = _MakeModel(src, mask, stdAngle, _OrgModelAngleIdx, maxContrast, minContrast, mdList, nPyrDepth, true, cMode);

		for (angle = _fAngleNeg; angle <= 0.0f; angle += _fAngleStep, nStep++)
		{
			if (nPyrDepth == enmDepth::spOrg && (angle > -0.05f && angle < 0.05f))
				_OrgModelAngleIdx = nStep;
			_MakeModelAngle(orgItem, src, mask, stdAngle + angle, nStep, maxContrast, minContrast, mdList, nPyrDepth, false);
		}
		if (cMode == enmCalcMode::cmPreProc)
		{
			mdList[nStep] = orgItem;
			if (nPyrDepth == enmDepth::spOrg)
				_OrgModelAngleIdx = nStep;
			nStep++;
		}

		for (; angle <= _fAnglePos; angle += _fAngleStep, nStep++)
		{
			if (nPyrDepth == enmDepth::spOrg && (angle > -0.05f && angle < 0.05f))
				_OrgModelAngleIdx = nStep;
			_MakeModelAngle(orgItem, src, mask, stdAngle + angle, nStep, maxContrast, minContrast, mdList, nPyrDepth, false);
		}
	}
	else
	{
		_MakeModel(src, mask, stdAngle, 0, maxContrast, minContrast, mdList, nPyrDepth, _OrgModelAngleIdx == 0 ? true : false);
		// 		mdList[0] = CreateItem();//std::shared_ptr<Item>(new GeoModel::Item(this, src, cv::Mat(src.rows, src.cols, CV_8UC1, cv::Scalar(255)), _PreProcLv[enmDepth::spOrg], maxContrast, minContrast, _bEdgeEnhancement));
		// 		mdList[0]->Calculate(src, cv::Mat(src.rows, src.cols, CV_8UC1, cv::Scalar(255)), _PreProcLv[enmDepth::spOrg], maxContrast, minContrast, _bEdgeEnhancement, getMatchAlgo());
	}
}
bool GeoModel::_CheckPyrSz(cv::Mat & src)
{
	int r = src.rows * 0.5f;
	int c = src.cols * 0.5f;
	if (_Speed == enmSpeed::spHigh)
	{
		if (r < 16 || c < 16)
			return false;
	}
	else if (_Speed == enmSpeed::spMedium)
	{
		if (r < 26 || c < 26)
			return false;
	}
	else
	{
		if (r < 36 || c < 36)
			return false;
	}

	return true;
}
void GeoModel::_GetPyrDown(cv::Mat src, cv::Mat & des)
{
	double scaleVal = 0.5;
	des = cv::Mat(src.rows*scaleVal, src.cols*scaleVal, CV_MAT_TYPE(src.type()));
	//	cv::pyrDown(src, des, cv::Size(src.cols*scaleVal, src.rows*scaleVal));
	cv::resize(src, des, cv::Size(src.cols*scaleVal, src.rows*scaleVal));
}
GeoModel::typItemPtr GeoModel::_MakeModel(cv::Mat src, cv::Mat mask, float angle, int nStep, float maxContrast, float minContrast, std::map<int, GeoModel::typItemPtr> & mdList, int nPyrDepth, bool bOrgImg, enmCalcMode cMode)
{
	cv::Mat temp;
	src = _GetAngle(src, angle, true);
	temp = _GetAngle(mask, angle, false);

	if (bOrgImg == true && nPyrDepth == (enmDepth::spNum - 1))
	{
#ifdef GeoSaveDebug
		CString sName, sName2;
		sName.Format(_T("d:\\model_%.0f.bmp"), getImageAngle());
		cv::imwrite(std::string(CT2A(sName)), src);
		sName2.Format(_T("d:\\model_0.bmp"));
		cv::imwrite(std::string(CT2A(sName2)), _srcModel);
#endif
		_srcModel = src;
	}

	int dilation_size(1);
	if (nPyrDepth == enmDepth::spOrg)
		dilation_size = 2;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1), cv::Point(dilation_size, dilation_size));

	cv::Mat morpImg = cv::Mat(temp.rows + 8, temp.cols + 8, CV_8UC1, cv::Scalar(0));
	cv::Mat morpActive = cv::Mat(temp.rows, temp.cols, CV_8UC1, &morpImg.ptr<uchar>(4)[4], morpImg.step1());
	temp.copyTo(morpActive);
	cv::erode(morpImg, morpImg, kernel);

	GeoModel::typItemPtr item;
	if (cMode == enmCalcMode::cmPreProc)
	{
		item = CreateItem();//std::shared_ptr<Item>(new GeoModel::Item(this));
		item->_Angle = angle;
		item->_PyrDown = nPyrDepth;
		item->Calculate(src, morpActive, _PreProcLv[nPyrDepth], maxContrast, minContrast, _bEdgeEnhancement, getMatchAlgo());

		if (nStep >= 0)
			mdList[nStep] = item;
	}
	else
	{
		item = mdList[nStep];
		item->ProCalculate(src, getMatchAlgo());
	}

#ifdef GeoSaveDebug
	if (item)
	{
		GeoResult res(1);
		GeoMatch gm;
		cv::Point pt = cv::Point(src.cols / 2 - 4, src.rows / 2 - 4);
		if (pt.x < 1)
			pt.x = 1;
		if (pt.y < 1)
			pt.y = 1;

		cv::Size sz = cv::Size(8, 8);
		if (src.cols < (pt.x + sz.width))
			sz.width = src.cols - pt.x;
		if (src.rows < (pt.y + sz.height))
			sz.height = src.rows - pt.y;

		gm._FindModel(src, item, nStep, pt, sz, 0.0, 0.0, 0.9, res);
		_MaxScore = res._Equality[0];

		res.setResCnt(1);
		CString sName;
		sName.Format(_T("d:\\testimage\\workimage\\models\\%d_%.0f.bmp"), nPyrDepth, angle);
		_tmkdir(_T("d:\\testimage\\workimage\\models"));
		cv::Mat draw;
		src.copyTo(draw);
		gm.DrawContours(draw, item, res, cv::Scalar(255), 1);
		cv::imwrite(std::string(CT2A(sName)), draw);
	}
#endif

	return item;
}

void GeoModel::_MakeModelAngle(GeoModel::typItemPtr orgItem, cv::Mat src, cv::Mat mask, float angle, int nStep, float maxContrast, float minContrast, std::map<int, GeoModel::typItemPtr> & mdList, int nPyrDepth, bool bOrgImg)
{
	cv::Mat temp;
	src = _GetAngle(src, angle, true);
	temp = _GetAngle(mask, angle, false);

	if (bOrgImg == true && nPyrDepth == (enmDepth::spNum - 1))
	{
#ifdef GeoSaveDebug
		CString sName, sName2;
		sName.Format(_T("d:\\model_%.0f.bmp"), getImageAngle());
		cv::imwrite(std::string(CT2A(sName)), src);
		sName2.Format(_T("d:\\model_0.bmp"));
		cv::imwrite(std::string(CT2A(sName2)), _srcModel);
#endif
		_srcModel = src;
	}

	int dilation_size(1);
	if (nPyrDepth == enmDepth::spOrg)
		dilation_size = 2;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
		cv::Point(dilation_size, dilation_size));

	cv::Mat morpImg = cv::Mat(temp.rows + 8, temp.cols + 8, CV_8UC1, cv::Scalar(0));
	cv::Mat morpActive = cv::Mat(temp.rows, temp.cols, CV_8UC1, &morpImg.ptr<uchar>(4)[4], morpImg.step1());
	temp.copyTo(morpActive);
	cv::erode(morpImg, morpImg, kernel);


	std::shared_ptr<Item> item = CreateItem();//std::shared_ptr<Item>(new GeoModel::Item(this));
	mdList[nStep] = item;
	mdList[nStep]->_Angle = angle;
	mdList[nStep]->_PyrDown = nPyrDepth;
	if (mdList.size() > 0)
		mdList[nStep]->CalculateAngle(src.cols, src.rows, _PreProcLv[nPyrDepth], maxContrast, minContrast, _bEdgeEnhancement, orgItem, angle, getMatchAlgo());
	else
		mdList[nStep]->Calculate(src, morpActive, _PreProcLv[nPyrDepth], maxContrast, minContrast, _bEdgeEnhancement, getMatchAlgo());

#ifdef GeoSaveDebug
	GeoResult res(1);
	GeoMatch gm;
	cv::Point pt = cv::Point(src.cols / 2 - 4, src.rows / 2 - 4);
	if (pt.x < 1)
		pt.x = 1;
	if (pt.y < 1)
		pt.y = 1;

	cv::Size sz = cv::Size(8, 8);
	if (src.cols < (pt.x + sz.width))
		sz.width = src.cols - pt.x;
	if (src.rows < (pt.y + sz.height))
		sz.height = src.rows - pt.y;

	gm._FindModel(src, item, nStep, pt, sz, 0.0, 0.0, 0.9, res);
	_MaxScore = res._Equality[0];

	res.setResCnt(1);
	CString sName;
	sName.Format(_T("d:\\testimage\\workimage\\models\\%d_%.0f.bmp"), nPyrDepth, angle);
	_tmkdir(_T("d:\\testimage\\workimage\\models"));
	cv::Mat draw;
	src.copyTo(draw);
	gm.DrawContours(draw, item, res, cv::Scalar(255), 1);
	cv::imwrite(std::string(CT2A(sName)), draw);
#endif
}

std::shared_ptr<GeoModel::Item> GeoModel::MakeModel(cv::Mat src, cv::Mat mask, float angle, float maxContrast, float minContrast, int nPyrDepth)
{
	_srcModel = src;
	// 	cv::Mat SrcBlur;
	// 	cv::medianBlur(_srcModel, SrcBlur, 3);

	cv::Mat temp = mask;
	std::shared_ptr<Item> item = CreateItem();//std::shared_ptr<Item>(new GeoModel::Item(this));
	item->_Angle = angle;
	item->_PyrDown = nPyrDepth;
	item->Calculate(_srcModel, mask, _PreProcLv[nPyrDepth], maxContrast, minContrast, _bEdgeEnhancement, getMatchAlgo());
	return item;
}
// cv::Mat GeoModel::_GetAngle(cv::Mat & src, float angle, bool bInterpolation)
// {
// 	cv::Point2f center(src.cols/2.0, src.rows/2.0);
// 	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
// 
// 	float radian = angle / 180.0f * M_PI;
// 	float sinth = sin(radian);
// 	float costh = cos(radian);
// 
// 	float lt_x = (-center.x) * costh + ( center.y) * sinth;
// 	float lt_y =-(-center.x) * sinth + ( center.y) * costh;
// 
// 	float lb_x = (-center.x) * costh + (-center.y) * sinth;
// 	float lb_y =-(-center.x) * sinth + (-center.y) * costh;
// 
// 	float rt_x = ( center.x) * costh + ( center.y) * sinth;
// 	float rt_y =-( center.x) * sinth + ( center.y) * costh;
// 
// 	float rb_x = ( center.x) * costh + (-center.y) * sinth;
// 	float rb_y =-( center.x) * sinth + (-center.y) * costh;
// 
// 	float min_x = std::min(std::min(std::min(lt_x, lb_x), rt_x), rb_x);
// 	float max_x = std::max(std::max(std::max(lt_x, lb_x), rt_x), rb_x);
// 	float min_y = std::min(std::min(std::min(lt_y, lb_y), rt_y), rb_y);
// 	float max_y = std::max(std::max(std::max(lt_y, lb_y), rt_y), rb_y);
// 	float sz_x = max_x - min_x;
// 	float sz_y = max_y - min_y;
// 
// 	rot.at<double>(0,2) += sz_x/2.0 - center.x;
// 	rot.at<double>(1,2) += sz_y/2.0 - center.y;
// 
// 	cv::Mat dst;
// 	if(bInterpolation==true)
// 		cv::warpAffine(cv::Mat(src), dst, rot, cv::Size(sz_x+0.4999,sz_y+0.4999), cv::INTER_LINEAR, cv::BORDER_WRAP);
// 	else
// 		cv::warpAffine(cv::Mat(src), dst, rot, cv::Size(sz_x+0.4999,sz_y+0.4999), cv::INTER_NEAREST);
// 
// 	return dst;
// }

cv::Mat GeoModel::_GetAngle(cv::Mat & src, float angle, bool bInterpolation)
{
	cv::Mat dst;
	if (src.channels() != 3)
		RotateImg_ipp(src, angle, &dst, bInterpolation);
	else
		RotateImg_ipp_color(src, angle, &dst, bInterpolation);

	return dst;
}


cv::Point GeoModel::WarpAffine(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent, bool Rev)
{
	cv::Point Cent, NCenter, temp;
	cv::Point Margin(0, 0);
	int Maxsz = orgImg.cols > orgImg.rows ? orgImg.cols : orgImg.rows;
	Cent.x = (orgImg.cols - 1) / 2;// MaxI;
	Cent.y = (orgImg.rows - 1) / 2;// MaxI;
	cv::Mat Img = orgImg.clone();
#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_orgImg.bmp", orgImg);
#endif
	Margin.x = Center.x - orgImg.cols / 2;
	Margin.y = Center.y - orgImg.rows / 2;

	NCenter = Center;
	temp.x = 0;
	temp.y = 0;

	if (abs(Center.x - Cent.x) > 2 || abs(Center.y - Cent.y) > 2)
	{
		cv::Point TmpM;
		if (Rev)
		{
			TmpM.x = Center.x - Cent.x;// +Margin.x * 2;
			TmpM.y = Center.y - Cent.y;// +Margin.y * 2;
		}
		else
		{
			TmpM.x = Center.x - Cent.x;
			TmpM.y = Center.y - Cent.y;
		}

		int w = TmpM.x <= 0 ? Img.cols + (Img.cols - Center.x - Center.x) : Img.cols + (Center.x * 2 - Img.cols);
		int h = TmpM.y <= 0 ? Img.rows + (Img.rows - Center.y - Center.y) : Img.rows + (Center.y * 2 - Img.rows);

		cv::Mat TempImage(h, w, orgImg.type());
		TempImage.setTo(0);

		temp.x = TempImage.cols / 2 - Center.x;// +TmpM.x;
		temp.y = TempImage.rows / 2 - Center.y;// +TmpM.y;

		cv::Mat roi = TempImage(cv::Rect(temp.x, temp.y, Img.cols, Img.rows));

		orgImg.copyTo(roi);

		Img = TempImage;
		NCenter.x = Img.cols / 2;
		NCenter.y = Img.rows / 2;

	}
	cv::Mat RotImage;
	cv::Mat rotation = getRotationMatrix2D(NCenter, Seta, 1);

	if (CV_MAT_TYPE(orgImg.type()) == CV_32FC1)
		cv::warpAffine(Img, RotImage, rotation, Img.size(), cv::INTER_LINEAR);
	else
		cv::warpAffine(Img, RotImage, rotation, Img.size(), cv::INTER_NEAREST);

#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_RotImage.bmp", RotImage);
#endif

	int MarX(0), MarY(0);
	if (NOtCent)
	{
		MarX = Margin.x;
		MarY = Margin.y;
		if (Rev)
		{
			MarX += Margin.x*0.5;
			MarY += Margin.y*0.5;
		}
	}

	temp.x = RotImage.cols / 2 - Cent.x - MarX;
	temp.y = RotImage.rows / 2 - Cent.y - MarY;

	//Center가 이미지 중심에서 오른쪽에 있을 경우
	if (Center.x > Cent.x)
	{
		temp.x = RotImage.cols / 2 - Center.x - MarX;
		Margin.x = 0;
	}

	//Center가 이미지 중심에서 아래에 있을 경우
	if (Center.y > Cent.y)
	{
		temp.y = RotImage.rows / 2 - Center.y - MarY;
		Margin.y = 0;
	}

	cv::Mat Crop = RotImage;
	int x(0), y(0), w = orgImg.cols, h = orgImg.rows;
	if (temp.x < 0)
	{
		x = -temp.x;
		w = orgImg.cols + temp.x;

		temp.x = 0;

		if (temp.y < 0)
		{
			y = -temp.y;
			h = orgImg.rows + temp.y;

			temp.y = 0;
			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else if (temp.y + orgImg.rows > RotImage.rows)
		{
			y = temp.y;
			y = 0;
			//int diffh = (orgImg.rows + temp.y) - RotImage.rows;
			//h = (orgImg.rows - diffh);
			h = RotImage.rows - temp.y;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else
		{
			y = temp.y;
			h = orgImg.rows;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, 0, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
	}
	else if (temp.x + orgImg.cols > RotImage.cols)
	{
		x = temp.x;
		x = 0;

		w = RotImage.cols - temp.x;

		if (temp.y < 0)
		{
			y = -temp.y;
			h = orgImg.rows + temp.y;

			temp.y = 0;
			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else if (temp.y + orgImg.rows > RotImage.rows)
		{
			y = temp.y;
			y = 0;

			h = RotImage.rows - temp.y;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else
		{
			y = temp.y;
			h = orgImg.rows;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, 0, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
	}
	if (temp.y < 0)
	{
		y = -temp.y;
		h = orgImg.rows + temp.y;

		temp.y = 0;
		cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

		roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
		roImg.setTo(0);
		cv::Mat roi = roImg(cv::Rect(0, y, w, h));

		cropImg.copyTo(roi);
		return Margin;
	}
	else if (temp.y + orgImg.rows > RotImage.rows)
	{
		y = temp.y;
		y = 0;

		h = RotImage.rows - temp.y;

		cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

		roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
		roImg.setTo(0);
		cv::Mat roi = roImg(cv::Rect(0, y, w, h));

		cropImg.copyTo(roi);
		return Margin;
	}

	roImg = RotImage(cv::Rect(temp.x, temp.y, orgImg.cols, orgImg.rows)).clone();
#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_roImg.bmp", roImg);
#endif
	return Margin;
}

bool GeoModel::RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (bColor)
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * 3 * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C3R(userSrc, orgSizeX * 3, *userDst, orgSizeX * 3, srcSize);
		}
		else
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C1R(userSrc, orgSizeX, *userDst, orgSizeX, srcSize);
		}

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	if (bColor)
		numChannels = 3;


	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp8u* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	if (bUseOrgSize == false)
	{
		//dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
		//dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);
		dstSize.width = (int)(bound[1][0] - bound[0][0] + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}

	//Memory allocation for the intermediate images
	*userDst = g_pMManager->pem_new<uchar>(true, dstSize.width * numChannels * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);
	Ipp8u* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;


	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing

		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}

bool GeoModel::RotateImg_ipp2020(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear, bool bUseOrgSize)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (*userDst == NULL)
			*userDst = g_pMManager->pem_new<float>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
		ippiCopy_32f_C1R(userSrc, orgSizeX * 4, *userDst, orgSizeX * 4, srcSize);

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp32f* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	if (bUseOrgSize == false)
	{
		//dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
		//dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);
		dstSize.width = (int)(bound[1][0] - bound[0][0] + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}

	//Memory allocation for the intermediate images
	*userDst = g_pMManager->pem_new<float>(true, dstSize.width * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);
	Ipp32f* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;


	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (status >= ippStsNoErr) status = ippiWarpAffineLinear_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (status >= ippStsNoErr) status = ippiWarpAffineNearest_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}

void GeoModel::RotateImg_ipp(cv::Mat& userSrc, double angle, cv::Mat* userDst, bool bInterpolation)
{
	int retDstSizeX, retDstSizeY;
	IppiSize srcSize = { userSrc.cols, userSrc.rows };

	if (userDst != NULL)
	{
		userDst->release();
	}

	if (CV_MAT_TYPE(userSrc.type()) == CV_8UC1)
	{
		UCHAR *userDstArr = nullptr;

		bool ret = RotateImg_ipp2020(userSrc.data, &userDstArr, angle, srcSize.width, srcSize.height, &retDstSizeX, &retDstSizeY, false, bInterpolation, false);
		if (ret == false) return;

		*userDst = cv::Mat(retDstSizeY, retDstSizeX, CV_MAKETYPE(CV_8U, userSrc.channels()));
		memcpy(userDst->data, userDstArr, sizeof(UCHAR)*retDstSizeX*retDstSizeY);

		Delete_1DArray(&userDstArr);
	}
	else if (CV_MAT_TYPE(userSrc.type()) == CV_32FC1)
	{
		float *userDstArr = nullptr;
		bool ret = RotateImg_ipp2020(userSrc.ptr<float>(), &userDstArr, angle, srcSize.width, srcSize.height, &retDstSizeX, &retDstSizeY, bInterpolation, false);
		if (ret == false) return;

		*userDst = cv::Mat(retDstSizeY, retDstSizeX, userSrc.type());
		memcpy(userDst->data, userDstArr, sizeof(float)*retDstSizeX*retDstSizeY);
		Delete_1DArray(&userDstArr);
	}
}

void GeoModel::RotateImg_ipp_color(cv::Mat& userSrc, double angle, cv::Mat* userDst, bool bInterpolation)
{
	unsigned char* ucuserDst = NULL;
	int dstSizeX = 0;
	int dstSizeY = 0;

	if (userSrc.channels() != 3)
	{
		unsigned char* ucuserSrc;

		ucuserSrc = userSrc.data;

		double dangle = angle;
		int orgSizeX;
		int orgSizeY;
		orgSizeX = userSrc.cols;
		orgSizeY = userSrc.rows;

		bool bLinear = bInterpolation;
		bool bColor = false;

		RotateImg_ipp2020(ucuserSrc, &ucuserDst, dangle, orgSizeX, orgSizeY, &dstSizeX, &dstSizeY, bColor, bLinear, false);

	}
	else
	{
		unsigned char* ucuserSrc;

		ucuserSrc = userSrc.data;

		double dangle = angle;
		int orgSizeX;
		int orgSizeY;
		orgSizeX = userSrc.cols;
		orgSizeY = userSrc.rows;

		bool bLinear = bInterpolation;
		bool bColor = true;

		RotateImg_ipp2020(ucuserSrc, &ucuserDst, dangle, orgSizeX, orgSizeY, &dstSizeX, &dstSizeY, bColor, bLinear, false);
	}

	if (userSrc.channels() != 3)
	{
		*userDst = cv::Mat(dstSizeY, dstSizeX, CV_8UC1);
		memcpy(userDst->data, ucuserDst, sizeof(UCHAR)*dstSizeY*dstSizeX);
		delete[] ucuserDst;
		ucuserDst = NULL;
	}
	else
	{
		*userDst = cv::Mat(dstSizeY, dstSizeX, CV_8UC3);
		memcpy(userDst->data, ucuserDst, sizeof(UCHAR)*dstSizeY*dstSizeX * 3);
		delete[] ucuserDst;
		ucuserDst = NULL;
	}
}



std::shared_ptr<GeoModel::Item> GeoModel::CreateItem()
{

	return std::shared_ptr<Item>(new GeoModel::Item(this));
}


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


GeoResult::GeoResult()
{
	_Count = 0;
	_ResCnt = 0;
}
GeoResult::GeoResult(int cnt)
{
	Alloc(cnt);
}
GeoResult::~GeoResult()
{
	Free();
}

void GeoResult::Alloc(int cnt)
{
	if (cnt < 0)
		return;
	Free();

	_Center_x.resize(cnt);
	_Center_y.resize(cnt);
	_Equality.resize(cnt);
	_Angle.resize(cnt);
	_AngleStepId.resize(cnt);
	_Count = cnt;
}
void GeoResult::Free()
{
	_Center_x.clear();
	_Center_y.clear();
	_Equality.clear();
	_Angle.clear();
	_AngleStepId.clear();
	_Count = 0;
}


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


bool operator < (const GeoSortItem & lhs, const GeoSortItem & rhs)
{
	return (lhs._Score < rhs._Score) || (lhs._Score == rhs._Score && ((lhs._I < rhs._I) || ((lhs._I == rhs._I) && (lhs._J < rhs._J))));
}
bool operator > (const GeoSortItem & lhs, const GeoSortItem & rhs)
{
	return (lhs._Score > rhs._Score) || (lhs._Score == rhs._Score && ((lhs._I > rhs._I) || ((lhs._I == rhs._I) && (lhs._J > rhs._J))));
}


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//CString GeoMatch::_bSaveProcData = _T("");

GeoMatch::GeoMatch(void)
{
	_ProcImg = std::make_shared<ImgProcessing>();
	FunPtr_CalcCoefficient_sse = &GeoMatch::_CalcCoefficient_sse;
	FunPtr_CalcCoefficientImg_sse = &GeoMatch::_CalcCoefficientImg_sse;
	FunPtr_CalcCoefficientVal_sse = &GeoMatch::_CalcCoefficientVal_sse;

	bool bUseAVX = false;
	if (_MSC_FULL_VER >= 160040219)
	{
		int cpuInfo[4];
		__cpuid(cpuInfo, 1);

		bool bOsUseXSAVE_XRSTORE = cpuInfo[2] & (1 << 27) || false;
		bool bCpuAVXSuport = cpuInfo[2] & (1 << 28) || false;

		if (bOsUseXSAVE_XRSTORE && bCpuAVXSuport)
		{
			unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
			bUseAVX = (xcrFeatureMask & 0x6) || false;
		}

	}
	if (bUseAVX)
	{
		FunPtr_CalcCoefficient_sse = &GeoMatch::_CalcCoefficient_AVX;
		FunPtr_CalcCoefficientImg_sse = &GeoMatch::_CalcCoefficientImg_AVX;
		FunPtr_CalcCoefficientVal_sse = &GeoMatch::_CalcCoefficientVal_AVX;
	}
}
GeoMatch::~GeoMatch(void)
{
}


void GeoMatch::_CalcCoefficient_sse_inline(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pEdgeY, float * pMag, BYTE * pFlag, float costh, float sinth)
{
	register __m128 partialSum = _mm_setzero_ps(), partialSum2 = _mm_setzero_ps();
	register __m128 num1 = _mm_set_ps1(1);
	register __m128 numStd = _mm_set_ps1(Min_Meg_Inv);
	int nRealNoOfCordinates(0);

	for (int m = 0; m < noOfCordinates; m++)
	{
		if ((pFlag[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			continue;
		nRealNoOfCordinates++;
		//	int curX = i + ((-pCoordX[m])*costh - pCoordY[m]*sinth);
		//	int curY = j + (pCoordX[m])*sinth + pCoordY[m]*costh;
		int curX = i + pCoordX[m];// + 0.49f;
		int curY = j + pCoordY[m];// + 0.49f;

		float iTx = pEdgeX[m];
		float iTy = pEdgeY[m];
		float iMag = pMag[m];

		if (curX<0 || curY<0 || (curX)>ImgLen - 1 || (curY + 8)>(ImgWid - 1))
		{
			if ((curX) >= 0 && curX <= (ImgLen - 1) && (curY + 8) >= 0 && curY <= (ImgWid - 1))
			{
				short *_Sdx = Sdx.ptr<short>(curX);// (short *)(Sdx->data.ptr + Sdx->step*(curX));
				short *_Sdy = Sdy.ptr<short>(curX);// (short *)(Sdy->data.ptr + Sdy->step*(curX));

				int jj = 0;
				int jn = 0;
				int cntjj = 8;
				if (curY < 0)
				{
					jj = -curY;
					curY = 0;
				}
				else if ((curY + 7) > (ImgWid - 1))
					cntjj -= (curY + 7) - (ImgWid - 1);

				for (; jj < 4 && jj < cntjj; jj++, jn++)
				{
					float iSx = _Sdx[curY + jn];
					float iSy = _Sdy[curY + jn];

					if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0) && (1.0f / matGradMag[(curX)*ImgWid + curY + jn] > Min_Meg))
						partialSum.m128_f32[jj] = partialSum.m128_f32[jj] + ((iSx*iTx) + (iSy*iTy)) * (iMag*matGradMag[(curX)*ImgWid + curY + jn]);
				}
				for (; jj < cntjj; jj++, jn++)
				{
					float iSx = _Sdx[curY + jn];
					float iSy = _Sdy[curY + jn];

					if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0) && (1.0f / matGradMag[(curX)*ImgWid + curY + jn] > Min_Meg))
						partialSum2.m128_f32[jj - 4] = partialSum2.m128_f32[jj - 4] + ((iSx*iTx) + (iSy*iTy)) * (iMag*matGradMag[(curX)*ImgWid + curY + jn]);
				}
			}
			continue;
		}

		short *_Sdx = Sdx.ptr<short>(curX);// short* _Sdx = (short*)(Sdx->data.ptr + Sdx->step*(curX));
		short *_Sdy = Sdy.ptr<short>(curX);// short* _Sdy = (short*)(Sdy->data.ptr + Sdy->step*(curX));

		register __m128i pSdx, pSdy;
		register __m128 vTx, vTy, vMag;//, vSx, vSy;



		partialSum = _mm_add_ps(
			partialSum,
			// 			_mm_and_ps(
			// 				_mm_and_ps(
			// 					_mm_or_ps(
			// 						_mm_cmpneq_ps(vSx, _mm_setzero_ps()), 
			// 						_mm_cmpneq_ps(vSy, _mm_setzero_ps())
			// 					), 
			// 					_mm_or_ps(
			// 						_mm_cmpneq_ps(vTx, _mm_setzero_ps()), 
			// 						_mm_cmpneq_ps(vTy, _mm_setzero_ps())
			// 					)
			// 				)
			// 			,
			_mm_and_ps(
				_mm_mul_ps(
					_mm_add_ps(
						_mm_mul_ps(
							vTx = _mm_set_ps1(iTx),
							_mm_cvtepi32_ps(_mm_unpacklo_epi16(pSdx, _mm_cmplt_epi16(pSdx = _mm_loadu_si128((__m128i *)&_Sdx[curY]), _mm_setzero_si128())))
						),
						_mm_mul_ps(
							vTy = _mm_set_ps1(iTy),
							_mm_cvtepi32_ps(_mm_unpacklo_epi16(pSdy, _mm_cmplt_epi16(pSdy = _mm_loadu_si128((__m128i *)&_Sdy[curY]), _mm_setzero_si128())))
						)
					),
					_mm_mul_ps(
						vMag = _mm_set_ps1(iMag),
						_mm_loadu_ps(&matGradMag[curX*ImgWid + curY]))
				)
				, _mm_cmpgt_ps(numStd, _mm_loadu_ps(&matGradMag[curX*ImgWid + curY]))
			)
			// 			)
		);


		partialSum2 = _mm_add_ps(
			partialSum2,
			// 			_mm_and_ps(
			// 				_mm_and_ps(
			// 					_mm_or_ps(
			// 						_mm_cmpneq_ps(vSx, _mm_setzero_ps()), 
			// 						_mm_cmpneq_ps(vSy, _mm_setzero_ps())
			// 					), 
			// 					_mm_or_ps(
			// 						_mm_cmpneq_ps(vTx, _mm_setzero_ps()), 
			// 						_mm_cmpneq_ps(vTy, _mm_setzero_ps())
			// 					)
			// 				)
			// 			,
			_mm_and_ps(
				_mm_mul_ps(
					_mm_add_ps(
						_mm_mul_ps(
							vTx,
							_mm_cvtepi32_ps(_mm_unpackhi_epi16(pSdx, _mm_cmplt_epi16(pSdx, _mm_setzero_si128())))
						),
						_mm_mul_ps(
							vTy,
							_mm_cvtepi32_ps(_mm_unpackhi_epi16(pSdy, _mm_cmplt_epi16(pSdy, _mm_setzero_si128())))
						)
					),

					_mm_mul_ps(
						vMag,
						_mm_loadu_ps(&matGradMag[curX*ImgWid + curY + 4]))
				)
				, _mm_cmpgt_ps(numStd, _mm_loadu_ps(&matGradMag[curX*ImgWid + curY + 4])))
			// 			)
		);
	}
	int nResWid = resBuf.cols;
	int nResLen = resBuf.rows;
	if (nRealNoOfCordinates)
	{
		if (nResLen <= i)
			return;
		float * resPS = resBuf.ptr<float>(i);// (float*)(resBuf->data.ptr + resBuf->step*(i));
		if ((j + 4) <= nResWid)
			_mm_storeu_ps(&resPS[j], _mm_div_ps(partialSum, _mm_set_ps1(nRealNoOfCordinates)));
		else
		{
			__m128 ScoreRes = _mm_div_ps(partialSum, _mm_set_ps1(nRealNoOfCordinates));
			float * resPS = resBuf.ptr<float>(i);// (float*)(resBuf->data.ptr + resBuf->step*(i));
			for (int jj = j, n = 0; jj < nResWid && n < 4; jj++, n++)
			{
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}

		if ((j + 8) <= nResWid)
			_mm_storeu_ps(&resPS[j + 4], _mm_div_ps(partialSum2, _mm_set_ps1(nRealNoOfCordinates)));
		else
		{
			__m128 ScoreRes = _mm_div_ps(partialSum2, _mm_set_ps1(nRealNoOfCordinates));
			float * resPS = resBuf.ptr<float>(i);// (float*)(resBuf->data.ptr + resBuf->step*(i));
			for (int jj = j + 4, n = 0; jj < nResWid && n < 4; jj++, n++)
			{
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}
	}
	else if (nResLen > i && nResWid > j)
		resBuf.ptr<float>(i)[j] = 0;
}
void GeoMatch::_CalcCoefficient_sse(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;

	float minScore(0.0f);

	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;

	int ImgWid = Sdx.cols;
	int ImgLen = Sdx.rows;

	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;

	float fRadian = fAngleStd / 180.0f * M_PI;
	float sinth = sin(fRadian);
	float costh = cos(fRadian);


	if (bConcurrency == false)
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 8)
				_CalcCoefficient_sse_inline(Sdx, Sdy, matGradMag, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pEdgeY, pMag, pFlags, costh, sinth);
			// 			int j = StX;
			// 			for(; j+8 < Wid; j+=8 )
			//  			if(j<Wid&& Wid - 9>0)
			//  				_CalcCoefficient_sse_inline(Sdx, Sdy, matGradMag, resBuf, i, Wid-9, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pEdgeY, pMag, pFlags, costh, sinth);
		}
	}
	else
	{
		// #ifndef GeoNoUseMultiThread
		// 		Concurrency::parallel_for((size_t)StY, (size_t)Len, [&](size_t i)
		// #else
		for (int i = StY; i < Len; i++)
			// #endif
		{
			for (int j = StX; j < Wid; j += 8)
				_CalcCoefficient_sse_inline(Sdx, Sdy, matGradMag, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pEdgeY, pMag, pFlags, costh, sinth);
			// 			int j = StX;
			// 			for(; j+8 < Wid; j+=8 )
			// 				_CalcCoefficient_sse_inline(Sdx, Sdy, matGradMag, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pEdgeY, pMag, pFlags, costh, sinth);
			//  			if (j < Wid && Wid - 9>0)
			//  				_CalcCoefficient_sse_inline(Sdx, Sdy, matGradMag, resBuf, i, Wid - 9, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pEdgeY, pMag, pFlags, costh, sinth);
			// 			for (; j < Wid; j++)
			// 				_CalcCoefficient(Sdx, Sdy, matGradMag, resBuf, cvPoint(i, j), Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, bConcurrency);
		}
		// #ifndef GeoNoUseMultiThread
		// 		);
		// #endif
	}
}
void GeoMatch::_CalcCoefficient(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;

	float minScore(0.0f);

	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;

	int ImgWid = Sdx.cols;
	int ImgLen = Sdx.rows;

	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;

	for (int i = StY; i < Len; i++)
	{
		for (int j = StX; j < Wid; j++)
		{
			double partialSum(0);
			double partialScore(0);
			for (int m = 0, mm = 0; m < noOfCordinates; m++)
			{
				if ((pFlags[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;
				mm++;
				int curX = i + pCoordX[m];
				int curY = j + pCoordY[m];
				float iTx = pEdgeX[m];
				float iTy = pEdgeY[m];
				float iMag = pMag[m];

				if (curX<0 || curY<0 || curX>ImgLen - 1 || curY>ImgWid - 1)
					continue;

				short *_Sdx = Sdx.ptr<short>(curX);// short * _Sdx = (short *)(Sdx->data.ptr + Sdx->step*(curX));
				short *_Sdy = Sdy.ptr<short>(curX);// short * _Sdy = (short *)(Sdy->data.ptr + Sdy->step*(curX));

				float iSx = _Sdx[curY];
				float iSy = _Sdy[curY];

				if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0) && (1.0f / matGradMag[curX*ImgWid + curY]) > Min_Meg)
					partialSum = partialSum + ((iSx*iTx) + (iSy*iTy)) * (iMag*matGradMag[curX*ImgWid + curY]);

				float sumOfCoords = mm + 1;
				partialScore = partialSum / sumOfCoords;

				// 				if( partialScore < (MIN((minScore-1) + normGreediness*sumOfCoords, normMinScore*sumOfCoords)))
				// 					break;
			}

			//partialScore = partialSum / noOfCordinates;

			float * resPS = resBuf.ptr<float>(i);// (float*)(resBuf->data.ptr + resBuf->step*(i));
			resPS[j] = partialScore;
		}
	}
}
void GeoMatch::_CalcCoefficientLap(cv::Mat & Sdl, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;
	for (int i = StY; i < Len; i++)
	{
		for (int j = StX; j < Wid; j++)
		{
			double partialSum(0);
			double partialScore(0);
			for (int m = 0, mm = 0; m < noOfCordinates; m++)
			{
				if ((pFlags[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;
				mm++;
				int curX = i + pCoordX[m];
				int curY = j + pCoordY[m];
				float iTl = pEdgeX[m];
				float iMag = pMag[m];
				if (curX<0 || curY<0 || curX>ImgLen - 1 || curY>ImgWid - 1)
					continue;
				BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
				float iSl = _Sdl[curY];
				if ((iSl != 0) && (iTl != 0))
					partialSum = partialSum + (iSl*iTl) * (iMag*matGradMag[curX*ImgWid + curY]);
				float sumOfCoords = mm + 1;
				partialScore = partialSum / sumOfCoords;
			}
			float * resPS = resBuf.ptr<float>(i);// (float*)(resBuf->data.ptr + resBuf->step*(i));
			resPS[j] = partialScore;
		}
	}
}
void GeoMatch::_CalcCoefficientImg_sse_inline(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pMag, BYTE * pFlag, cv::Point2i MinPos, cv::Point2i MaxPos, float stdev)
{
	int nMinX = j + MinPos.x + 0;
	int nMaxX = j + MaxPos.x + 1;
	int nMinY = i + MinPos.y + 0;
	int nMaxY = i + MaxPos.y + 1;

	if (nMinX < 0)
		nMinX = 0;
	if (nMinY < 0)
		nMinY = 0;

	if (nMaxX < 1 || nMaxY < 1)
	{
		resBuf.ptr<float>(i)[j] = 0;
		return;
	}

	if (nMaxX >= Integ.cols)
		nMaxX = Integ.cols - 1;
	if (nMaxY >= Integ.rows)
		nMaxY = Integ.rows - 1;
	int SzBox;// = (nMaxX-nMinX) * (nMaxY-nMinY);
	int * nMinIteg = Integ.ptr<int>(nMinY);
	int * nMaxIteg = Integ.ptr<int>(nMaxY);
	register __m128 vMean[4];
	// 	int imgSt = 0;
	// 
	// 	for(;nMaxX+imgSt+4<Integ.cols && imgSt<16;imgSt +=4)
	// 	{
	// 		vMean[imgSt/4] = _mm_div_ps(
	// 			_mm_sub_ps(
	// 			_mm_sub_ps(_mm_cvtepi32_ps(_mm_loadu_si128((__m128i *)&nMaxIteg[nMaxX+imgSt])),_mm_cvtepi32_ps(_mm_loadu_si128((__m128i *)&nMinIteg[nMaxX+imgSt]))),//1
	// 			_mm_sub_ps(_mm_cvtepi32_ps(_mm_loadu_si128((__m128i *)&nMaxIteg[nMinX+imgSt])),_mm_cvtepi32_ps(_mm_loadu_si128((__m128i *)&nMinIteg[nMinX+imgSt]))) //2
	// 			),
	// 			_mm_set_ps1(SzBox)
	// 			);
	// 	}
	// 	for(;imgSt<16;imgSt++)
	// 	{
	// 		int offset = imgSt;
	// 		if(offset + nMaxX >= Integ.cols)
	// 			offset = Integ.cols-1-nMaxX;
	// 													//1	----------------------------------------		//2-----------------------------------------
	// 		vMean[imgSt/4].m128_f32[imgSt%4] = (float)(nMaxIteg[nMaxX+offset] - nMinIteg[nMaxX+offset] - nMaxIteg[nMinX+offset] + nMinIteg[nMinX+offset]) / (float)(SzBox);
	// 	}

	for (int imgSt = 0; imgSt < 16; imgSt++)
	{
		int offset = imgSt;

		int ntmpMinX = j + MinPos.x + 0 + offset;
		int ntmpMaxX = j + MaxPos.x + 1 + offset;

		if (ntmpMinX < 0)
			ntmpMinX = 0;
		if (ntmpMaxX >= Integ.cols)
			ntmpMaxX = Integ.cols - 1;

		SzBox = (ntmpMaxX - ntmpMinX) * (nMaxY - nMinY);

		vMean[imgSt / 4].m128_f32[imgSt % 4] = (float)(nMaxIteg[ntmpMaxX] - nMinIteg[ntmpMaxX] - nMaxIteg[ntmpMinX] + nMinIteg[ntmpMinX]) / (float)(SzBox);
	}

	int nRealNoOfCordinates(0);

	register __m128 partialStdev = _mm_setzero_ps(), partialStdev2 = _mm_setzero_ps(), partialStdev3 = _mm_setzero_ps(), partialStdev4 = _mm_setzero_ps();
	register __m128 partialSum = _mm_setzero_ps(), partialSum2 = _mm_setzero_ps(), partialSum3 = _mm_setzero_ps(), partialSum4 = _mm_setzero_ps();
	float fGeoEpsilon = GeoEpsilon;
	register __m128 vGeoEpsilon = _mm_set_ps1(fGeoEpsilon);

	for (int m = 0; m < noOfCordinates; m++)
	{
		if ((pFlag[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			continue;

		nRealNoOfCordinates++;

		int curX = i + pCoordX[m];
		int curY = j + pCoordY[m];
		float iTl = pEdgeX[m];
		float iMag = pMag[m];

		if (curX < 0 || curY < 0 || curX > ImgLen - 1 || (curY + 16) >(ImgWid - 1))
		{
			if (curX >= 0 && curX <= (ImgLen - 1) && (curY + 16) >= 0 && curY <= (ImgWid - 1))
			{
				BYTE *_Sdl = Sdl.ptr<BYTE>(curX);

				int jj = 0;
				int jn = 0;
				int cntjj = 16;
				if (curY < 0)
				{
					jj = -curY;
					curY = 0;
				}
				else if ((curY + 15) > (ImgWid - 1))
					cntjj -= (curY + 15) - (ImgWid - 1);

				for (; jj < 4 && jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]) - vMean[0].m128_f32[jj];	// 평균 값을 활용한다
					partialSum.m128_f32[jj] = partialSum.m128_f32[jj] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev.m128_f32[jj] = partialStdev.m128_f32[jj] + (iSl*iSl);
				}

				for (; jj < 8 && jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]) - vMean[1].m128_f32[jj - 4];	// 평균 값을 활용한다
					partialSum2.m128_f32[jj - 4] = partialSum2.m128_f32[jj - 4] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev2.m128_f32[jj - 4] = partialStdev2.m128_f32[jj - 4] + (iSl*iSl);
				}

				for (; jj < 12 && jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]) - vMean[2].m128_f32[jj - 8];	// 평균 값을 활용한다
					partialSum3.m128_f32[jj - 8] = partialSum3.m128_f32[jj - 8] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev3.m128_f32[jj - 8] = partialStdev3.m128_f32[jj - 8] + (iSl*iSl);
				}

				for (; jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]) - vMean[3].m128_f32[jj - 12];	// 평균 값을 활용한다
					partialSum4.m128_f32[jj - 12] = partialSum4.m128_f32[jj - 12] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev4.m128_f32[jj - 12] = partialStdev4.m128_f32[jj - 12] + (iSl*iSl);
				}
			}
			continue;
		}


		BYTE *_Sdl = Sdl.ptr<BYTE>(curX);

		register __m128i pSdl;
		register __m128 vSdI, vTI;


		partialSum = _mm_add_ps(
			partialSum,

			_mm_mul_ps(
				_mm_add_ps(
					vSdI = _mm_sub_ps(
						_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(pSdl = _mm_loadu_si128((__m128i *)&_Sdl[curY]), _mm_setzero_si128()), _mm_setzero_si128())),
						vMean[0]), vGeoEpsilon),
				_mm_add_ps(vTI = _mm_set_ps1(iTl), vGeoEpsilon)
			)
		);
		partialStdev = _mm_add_ps(partialStdev, _mm_mul_ps(vSdI, vSdI));

		partialSum2 = _mm_add_ps(
			partialSum2,
			_mm_mul_ps(
				_mm_add_ps(
					vSdI = _mm_sub_ps(
						_mm_cvtepi32_ps(_mm_unpackhi_epi16(_mm_unpacklo_epi8(pSdl, _mm_setzero_si128()), _mm_setzero_si128())),
						vMean[1]), vGeoEpsilon),
				_mm_add_ps(vTI, vGeoEpsilon)
			)
		);
		partialStdev2 = _mm_add_ps(partialStdev2, _mm_mul_ps(vSdI, vSdI));

		partialSum3 = _mm_add_ps(
			partialSum3,
			_mm_mul_ps(
				_mm_add_ps(
					vSdI = _mm_sub_ps(
						_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpackhi_epi8(pSdl, _mm_setzero_si128()), _mm_setzero_si128())),
						vMean[2]), vGeoEpsilon),
				_mm_add_ps(vTI, vGeoEpsilon)
			)
		);
		partialStdev3 = _mm_add_ps(partialStdev3, _mm_mul_ps(vSdI, vSdI));

		partialSum4 = _mm_add_ps(
			partialSum4,
			_mm_mul_ps(
				_mm_add_ps(
					vSdI = _mm_sub_ps(
						_mm_cvtepi32_ps(_mm_unpackhi_epi16(_mm_unpackhi_epi8(pSdl, _mm_setzero_si128()), _mm_setzero_si128())),
						vMean[3]), vGeoEpsilon),
				_mm_add_ps(vTI, vGeoEpsilon)
			)
		);
		partialStdev4 = _mm_add_ps(partialStdev4, _mm_mul_ps(vSdI, vSdI));
	}

	if (nRealNoOfCordinates)
	{
		float * resPS = resBuf.ptr<float>(i);
		if ((j + 4) <= ImgWid)
		{
			_mm_storeu_ps(&resPS[j],
				_mm_div_ps(_mm_div_ps(partialSum, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
					_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates))
			);
		}
		else
		{
			__m128 ScoreRes = _mm_div_ps(_mm_div_ps(partialSum, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
				_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates));

			for (int jj = j, n = 0; jj < ImgWid && n < 4; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}

		if ((j + 8) <= ImgWid)
		{
			_mm_storeu_ps(&resPS[j + 4],
				_mm_div_ps(_mm_div_ps(partialSum2, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev2, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
					_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates))
			);
		}
		else
		{
			__m128 ScoreRes = _mm_div_ps(_mm_div_ps(partialSum2, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev2, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
				_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates));
			for (int jj = j + 4, n = 0; jj < ImgWid && n < 4; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}

		if ((j + 12) <= ImgWid)
		{
			_mm_storeu_ps(&resPS[j + 8],
				_mm_div_ps(_mm_div_ps(partialSum3, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev3, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
					_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates))
			);
		}
		else
		{
			__m128 ScoreRes = _mm_div_ps(_mm_div_ps(partialSum3, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev3, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
				_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates));
			for (int jj = j + 8, n = 0; jj < ImgWid && n < 4; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}

		if ((j + 16) <= ImgWid)
		{
			_mm_storeu_ps(&resPS[j + 12],
				_mm_div_ps(_mm_div_ps(partialSum4, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev4, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
					_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates))
			);
		}
		else
		{
			__m128 ScoreRes = _mm_div_ps(_mm_div_ps(partialSum4, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev4, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
				_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates));
			for (int jj = j + 12, n = 0; jj < ImgWid && n < 4; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}
	}
	else
		resBuf.ptr<float>(i)[j] = 0;

}
void GeoMatch::_CalcCoefficientImg_sse(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;
	cv::Point2i MinPos = ModelItem->_MinPos;
	cv::Point2i MaxPos = ModelItem->_MaxPos;

	int nModelWid = MaxPos.x - MinPos.x + 1;
	int nModelLen = MaxPos.y - MinPos.y + 1;
	int nModelArea = nModelWid * nModelLen;

	float stdev = ModelItem->_Stdev;

	if (bConcurrency == false)
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 16)
			{

				_CalcCoefficientImg_sse_inline(Sdl, Integ, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pMag, pFlags, MinPos, MaxPos, stdev);
			}
		}
	}
	else
	{
		// #ifndef GeoNoUseMultiThread
		// 		Concurrency::parallel_for((size_t)StY, (size_t)Len, [&](size_t i)
		// #else
		for (int i = StY; i < Len; i++)
			// #endif
		{
			for (int j = StX; j < Wid; j += 16)
				_CalcCoefficientImg_sse_inline(Sdl, Integ, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pMag, pFlags, MinPos, MaxPos, stdev);
		}
		// #ifndef GeoNoUseMultiThread
		// 		);
		// #endif
	}
}
void GeoMatch::_CalcCoefficientImg(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;
	cv::Point2i MinPos = ModelItem->_MinPos;
	cv::Point2i MaxPos = ModelItem->_MaxPos;

	int nModelWid = MaxPos.x - MinPos.x + 1;
	int nModelLen = MaxPos.y - MinPos.y + 1;
	int nModelArea = nModelWid * nModelLen;

	float stdev = ModelItem->_Stdev;

	for (int i = StY; i < Len; i++)
	{
		for (int j = StX; j < Wid; j++)
		{
			int nMinX = j + MinPos.x + 0;
			int nMaxX = j + MaxPos.x + 1;
			int nMinY = i + MinPos.y + 0;
			int nMaxY = i + MaxPos.y + 1;

			if (nMinX < 0)
				nMinX = 0;
			if (nMinY < 0)
				nMinY = 0;

			if (nMaxX < 1 || nMaxY < 1)
			{
				resBuf.ptr<float>(i)[j] = 0;
				continue;
			}

			if (nMaxX >= Integ.cols)
				nMaxX = Integ.cols - 1;
			if (nMaxY >= Integ.rows)
				nMaxY = Integ.rows - 1;
			int SzBox = (nMaxX - nMinX) * (nMaxY - nMinY);
			int * nMinIteg = Integ.ptr<int>(nMinY);
			int * nMaxIteg = Integ.ptr<int>(nMaxY);
			float ImgMean = (float)(nMaxIteg[nMaxX] - nMinIteg[nMaxX] - nMaxIteg[nMinX] + nMinIteg[nMinX]) / (float)(SzBox);

			float partialStdev(0);
			float partialSum(0);
			float partialScore(0);
			int nRealNoOfCordinates(0);
			for (int m = 0; m < noOfCordinates; m++)
			{
				if ((pFlags[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;
				nRealNoOfCordinates++;
				int curX = i + pCoordX[m];
				int curY = j + pCoordY[m];
				float iTl = pEdgeX[m];
				float iMag = pMag[m];
				if (curX<0 || curY<0 || curX>ImgLen - 1 || curY>ImgWid - 1)
					continue;
				BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
				float iSl = (float)(_Sdl[curY]) - ImgMean;	// 평균 값을 활용한다
				partialSum += ((iSl + GeoEpsilon)*(iTl + GeoEpsilon));// * (iMag*(1.0/sqrt(iSl*iSl)));
				partialStdev += ((iSl) * (iSl));
				float sumOfCoords = m + 1;
			}
			if (nRealNoOfCordinates)
			{
				partialStdev = sqrt(partialStdev / (float)(nRealNoOfCordinates));
				partialScore = (partialSum / ((partialStdev + GeoEpsilon) * (stdev + GeoEpsilon))) / (float)(nRealNoOfCordinates);
				float * resPS = resBuf.ptr<float>(i);
				resPS[j] = partialScore;
			}
			else
				resBuf.ptr<float>(i)[j] = 0;
		}
	}
}

void GeoMatch::_CalcCoefficientVal_sse_inline(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pMag, BYTE * pFlag, cv::Point2i MinPos, cv::Point2i MaxPos, float stdev)
{
	int nRealNoOfCordinates(0);

	register __m128 partialStdev = _mm_setzero_ps(), partialStdev2 = _mm_setzero_ps(), partialStdev3 = _mm_setzero_ps(), partialStdev4 = _mm_setzero_ps();
	register __m128 partialSum = _mm_setzero_ps(), partialSum2 = _mm_setzero_ps(), partialSum3 = _mm_setzero_ps(), partialSum4 = _mm_setzero_ps();
	float fGeoEpsilon = GeoEpsilon;
	register __m128 vGeoEpsilon = _mm_set_ps1(fGeoEpsilon);

	for (int m = 0; m < noOfCordinates; m++)
	{
		if ((pFlag[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			continue;

		nRealNoOfCordinates++;

		int curX = i + pCoordX[m];
		int curY = j + pCoordY[m];
		float iTl = pEdgeX[m];
		float iMag = pMag[m];

		if (curX < 0 || curY < 0 || curX > ImgLen - 1 || (curY + 16) >(ImgWid - 1))
		{
			if (curX >= 0 && curX <= (ImgLen - 1) && (curY + 16) >= 0 && curY <= (ImgWid - 1))
			{
				BYTE *_Sdl = Sdl.ptr<BYTE>(curX);

				int jj = 0;
				int jn = 0;
				int cntjj = 16;
				if (curY < 0)
				{
					jj = -curY;
					curY = 0;
				}
				else if ((curY + 15) > (ImgWid - 1))
					cntjj -= (curY + 15) - (ImgWid - 1);

				for (; jj < 4 && jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]);
					partialSum.m128_f32[jj] = partialSum.m128_f32[jj] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev.m128_f32[jj] = partialStdev.m128_f32[jj] + (iSl*iSl);
				}

				for (; jj < 8 && jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]);
					partialSum2.m128_f32[jj - 4] = partialSum2.m128_f32[jj - 4] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev2.m128_f32[jj - 4] = partialStdev2.m128_f32[jj - 4] + (iSl*iSl);
				}

				for (; jj < 12 && jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]);
					partialSum3.m128_f32[jj - 8] = partialSum3.m128_f32[jj - 8] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev3.m128_f32[jj - 8] = partialStdev3.m128_f32[jj - 8] + (iSl*iSl);
				}

				for (; jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]);
					partialSum4.m128_f32[jj - 12] = partialSum4.m128_f32[jj - 12] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev4.m128_f32[jj - 12] = partialStdev4.m128_f32[jj - 12] + (iSl*iSl);
				}
			}
			continue;
		}


		BYTE *_Sdl = Sdl.ptr<BYTE>(curX);

		register __m128i pSdl;
		register __m128 vSdI, vTI, vMean;

		partialSum = _mm_add_ps(
			partialSum,
			_mm_mul_ps(
				_mm_add_ps(
					vSdI = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(pSdl = _mm_loadu_si128((__m128i *)&_Sdl[curY]), _mm_setzero_si128()), _mm_setzero_si128()))
					, vGeoEpsilon),
				_mm_add_ps(vTI = _mm_set_ps1(iTl), vGeoEpsilon)
			)
		);
		partialStdev = _mm_add_ps(partialStdev, _mm_mul_ps(vSdI, vSdI));

		partialSum2 = _mm_add_ps(
			partialSum2,
			_mm_mul_ps(
				_mm_add_ps(
					vSdI = _mm_cvtepi32_ps(_mm_unpackhi_epi16(_mm_unpacklo_epi8(pSdl = _mm_loadu_si128((__m128i *)&_Sdl[curY]), _mm_setzero_si128()), _mm_setzero_si128()))
					, vGeoEpsilon),
				_mm_add_ps(vTI, vGeoEpsilon)
			)
		);
		partialStdev2 = _mm_add_ps(partialStdev2, _mm_mul_ps(vSdI, vSdI));

		partialSum3 = _mm_add_ps(
			partialSum3,
			_mm_mul_ps(
				_mm_add_ps(
					vSdI = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpackhi_epi8(pSdl = _mm_loadu_si128((__m128i *)&_Sdl[curY]), _mm_setzero_si128()), _mm_setzero_si128()))
					, vGeoEpsilon),
				_mm_add_ps(vTI, vGeoEpsilon)
			)
		);
		partialStdev3 = _mm_add_ps(partialStdev3, _mm_mul_ps(vSdI, vSdI));

		partialSum4 = _mm_add_ps(
			partialSum4,
			_mm_mul_ps(
				_mm_add_ps(
					vSdI = _mm_cvtepi32_ps(_mm_unpackhi_epi16(_mm_unpackhi_epi8(pSdl = _mm_loadu_si128((__m128i *)&_Sdl[curY]), _mm_setzero_si128()), _mm_setzero_si128()))
					, vGeoEpsilon),
				_mm_add_ps(vTI, vGeoEpsilon)
			)
		);
		partialStdev4 = _mm_add_ps(partialStdev4, _mm_mul_ps(vSdI, vSdI));
	}

	if (nRealNoOfCordinates)
	{
		float * resPS = resBuf.ptr<float>(i);
		if ((j + 4) <= ImgWid)
		{
			_mm_storeu_ps(&resPS[j],
				_mm_div_ps(_mm_div_ps(partialSum, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
					_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates))
			);
		}
		else
		{
			__m128 ScoreRes = _mm_div_ps(_mm_div_ps(partialSum, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
				_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates));

			for (int jj = j, n = 0; jj < ImgWid && n < 4; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}

		if ((j + 8) <= ImgWid)
		{
			_mm_storeu_ps(&resPS[j + 4],
				_mm_div_ps(_mm_div_ps(partialSum2, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev2, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
					_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates))
			);
		}
		else
		{
			__m128 ScoreRes = _mm_div_ps(_mm_div_ps(partialSum2, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev2, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
				_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates));
			for (int jj = j + 4, n = 0; jj < ImgWid && n < 4; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}

		if ((j + 12) <= ImgWid)
		{
			_mm_storeu_ps(&resPS[j + 8],
				_mm_div_ps(_mm_div_ps(partialSum3, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev3, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
					_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates))
			);
		}
		else
		{
			__m128 ScoreRes = _mm_div_ps(_mm_div_ps(partialSum3, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev3, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
				_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates));
			for (int jj = j + 8, n = 0; jj < ImgWid && n < 4; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}

		if ((j + 16) <= ImgWid)
		{
			_mm_storeu_ps(&resPS[j + 12],
				_mm_div_ps(_mm_div_ps(partialSum4, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev4, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
					_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates))
			);
		}
		else
		{
			__m128 ScoreRes = _mm_div_ps(_mm_div_ps(partialSum4, _mm_mul_ps(_mm_add_ps(_mm_sqrt_ps(_mm_div_ps(partialStdev4, _mm_set_ps1(nRealNoOfCordinates))), vGeoEpsilon),
				_mm_add_ps(_mm_set_ps1(stdev), vGeoEpsilon))), _mm_set_ps1(nRealNoOfCordinates));
			for (int jj = j + 12, n = 0; jj < ImgWid && n < 4; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m128_f32[n];
			}
		}
	}
	else
		resBuf.ptr<float>(i)[j] = 0;

}
void GeoMatch::_CalcCoefficientVal_sse(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;
	cv::Point2i MinPos = ModelItem->_MinPos;
	cv::Point2i MaxPos = ModelItem->_MaxPos;

	int nModelWid = MaxPos.x - MinPos.x + 1;
	int nModelLen = MaxPos.y - MinPos.y + 1;
	int nModelArea = nModelWid * nModelLen;

	float stdev = ModelItem->_Stdev;

	if (bConcurrency == false)
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 16)
			{
				_CalcCoefficientVal_sse_inline(Sdl, Integ, IntegSq, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pMag, pFlags, MinPos, MaxPos, stdev);
			}
		}
	}
	else
	{
		// #ifndef GeoNoUseMultiThread
		// 		Concurrency::parallel_for((size_t)StY, (size_t)Len, [&](size_t i)
		// #else
		for (int i = StY; i < Len; i++)
			// #endif
		{
			for (int j = StX; j < Wid; j += 16)
				_CalcCoefficientVal_sse_inline(Sdl, Integ, IntegSq, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pMag, pFlags, MinPos, MaxPos, stdev);
		}
		// #ifndef GeoNoUseMultiThread
		// 		);
		// #endif
	}
}
void GeoMatch::_CalcCoefficientVal(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;
	cv::Point2i MinPos = ModelItem->_MinPos;
	cv::Point2i MaxPos = ModelItem->_MaxPos;

	int nModelWid = MaxPos.x - MinPos.x + 1;
	int nModelLen = MaxPos.y - MinPos.y + 1;
	int nModelArea = nModelWid * nModelLen;

	float stdev = ModelItem->_Stdev;

	std::vector<float> partialData(noOfCordinates);
	for (int i = StY; i < Len; i++)
	{
		for (int j = StX; j < Wid; j++)
		{
			float partialStdev(0);
			float partialSum(0);
			float partialScore(0);
			int nRealNoOfCordinates(0);

			// 			float partialMean(0.0f);
			// 			int partialCnt(0);
			// 			for(int m=0; m<noOfCordinates; m++)
			// 			{
			// 				if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			// 					continue;
			// 				nRealNoOfCordinates++;
			// 				int curX = i + pCoordX[m];
			// 				int curY = j + pCoordY[m];
			// 				if(curX<0 || curY<0 || curX>ImgLen-1 || curY>ImgWid-1)
			// 					continue;
			// 				
			// 				BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
			// 				float iSl = _Sdl[curY];
			// 				partialData[m] = iSl;
			// 				partialMean += iSl;
			// 				partialCnt++;
			// 			}
			// 			partialMean /= (float)(partialCnt);

			nRealNoOfCordinates = 0;
			for (int m = 0; m < noOfCordinates; m++)
			{
				if ((pFlags[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;
				nRealNoOfCordinates++;
				int curX = i + pCoordX[m];
				int curY = j + pCoordY[m];
				float iTl = pEdgeX[m];
				float iMag = pMag[m];
				if (curX<0 || curY<0 || curX>ImgLen - 1 || curY>ImgWid - 1)
					continue;
				float iSl = partialData[m];
				partialSum += ((iSl)+GeoEpsilon)*((iTl)+GeoEpsilon);
				partialStdev += ((iSl) * (iSl));
				float sumOfCoords = m + 1;
			}
			if (nRealNoOfCordinates)
			{
				partialStdev = sqrt(partialStdev / (float)(nRealNoOfCordinates));
				partialScore = (partialSum / ((partialStdev + GeoEpsilon) * (stdev + GeoEpsilon))) / (float)(nRealNoOfCordinates);
				float * resPS = resBuf.ptr<float>(i);
				resPS[j] = partialScore;
			}
			else
				resBuf.ptr<float>(i)[j] = 0;
		}
	}
}

void GeoMatch::_CalcCoefficient_DivSearch(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness)
{
	if (ModelItem->isProcessed() == false)
		return;

	float minScore(0.0f);

	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;

	_DivResBuf.clear();
	_DivResBuf.resize(ModelItem->_noOfCordinates);
	float * pResBuf = &_DivResBuf[0];

	int ImgWid = Sdx.cols;
	int ImgLen = Sdx.rows;

	int j = SStart.x;
	int i = SStart.y;

	for (int m = 0; m < noOfCordinates; m++)
	{
		if ((pFlags[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
		{
			pResBuf[m] = 0;
			continue;
		}

		int curX = i + pCoordX[m];
		int curY = j + pCoordY[m];
		float iTx = pEdgeX[m];
		float iTy = pEdgeY[m];
		float iMag = pMag[m];

		if (curX<0 || curY<0 || curX>ImgLen - 1 || curY>ImgWid - 1)
			continue;

		short *_Sdx = Sdx.ptr<short>(curX);
		short *_Sdy = Sdy.ptr<short>(curX);

		float iSx = _Sdx[curY];
		float iSy = _Sdy[curY];

		double partialSum(0);
		if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0) && (1.0 / matGradMag[curX*ImgWid + curY]) > 40)
			partialSum = ((iSx*iTx) + (iSy*iTy)) * (iMag*matGradMag[curX*ImgWid + curY]);

		pResBuf[m] = partialSum;
	}
}

void GeoMatch::_CalcResult(cv::Mat resBuf, int nModelIdx, GeoModel::typItemPtr ModelItem, cv::Point SStart, cv::Size Ssize, GeoResult & result)
{
	if (result.Count() <= 0)
		return;

	int nImgWid = resBuf.cols;
	int nImgHei = resBuf.rows;

	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = SStart.x + Ssize.width;
	int Len = SStart.y + Ssize.height;
	int nCount = result.Count();
	int fnd = 0;

	for (int searchCnt = 0; searchCnt < nCount; searchCnt++)
	{
		float equality(-HUGE_VAL);
		int centX(0), centY(0);
		for (int i = StY; i < Len; i++)
		{
			float * data = resBuf.ptr<float>(i);
			for (int j = StX; j < Wid; j++)
			{
				if (data[j] > equality)
				{
					equality = data[j];
					centX = i;
					centY = j;
				}
			}
		}

		// 		cv::Point	ptMin, ptMax;
		// 		double		Min(0.0), Max(0.0);
		// 		cv::minMaxLoc(resBuf, &Min, &Max, &ptMin, &ptMax);
		// 		result._Center_x[fnd] = ptMax.y;
		// 		result._Center_y[fnd] = ptMax.x;
		// 		result._Equality[fnd] = Max;

		// 		if(equality <= 0.0)
		// 			continue;

		result._Center_x[fnd] = centX;
		result._Center_y[fnd] = centY;
		result._Equality[fnd] = equality;
		result._Angle[fnd] = ModelItem->_Angle;
		result._AngleStepId[fnd] = nModelIdx;

		fnd++;

		int nEraseHei = ModelItem->_modelHeight * 0.75;
		int nEraseWid = ModelItem->_modelWidth * 0.75;

		cv::Point2i OvrDist = ModelItem->_ParentPtr->getOverlappedModelDist();
		if (OvrDist.x != 0)
			nEraseWid = OvrDist.x;
		if (OvrDist.y != 0)
			nEraseHei = OvrDist.y;

		// 		int nEraseStX = ptMax.y/*centX*/ - nEraseHei * 0.5;
		// 		int nEraseStY = ptMax.x/*centY*/ - nEraseWid * 0.5;

		int nEraseStX = centX - nEraseHei * 0.5;
		int nEraseStY = centY - nEraseWid * 0.5;

		if (nCount <= 1)
			continue;

		if (nEraseStX < 0)
		{
			nEraseHei += nEraseStX;
			nEraseStX = 0;
		}
		if (nEraseStY < 0)
		{
			nEraseWid += nEraseStY;
			nEraseStY = 0;
		}

		int nEraseEdX = nEraseStX + nEraseHei;
		int nEraseEdY = nEraseStY + nEraseWid;

		if (nEraseEdX > nImgHei)
		{
			nEraseHei -= nEraseEdX - nImgHei;
			nEraseEdX = nEraseStX + nEraseHei;
		}
		if (nEraseEdY > nImgWid)
		{
			nEraseWid -= nEraseEdY - nImgWid;
			nEraseEdY = nEraseStY + nEraseWid;
		}
		cv::Mat resRoi(resBuf, cv::Rect(nEraseStY, nEraseStX, nEraseWid, nEraseHei));
		resRoi = -HUGE_VAL;

		// 		for (int j = nEraseStY; j < nEraseEdY; j++)
		// 			resBuf.ptr<float>(nEraseStX)[j] = -HUGE_VAL;
		// 
		// 		for (int i=nEraseStX+1; i<nEraseEdX; i++)
		// 		{
		// 			float * data = resBuf.ptr<float>(i);
		// 			float * zerobuf = resBuf.ptr<float>(nEraseStX);
		// 			int j = nEraseStY;
		// 			memcpy(&data[j], &zerobuf[j], nEraseWid * sizeof(float));
		// 		}
	}

	result.setResCnt(fnd);
}
void GeoMatch::_CalcPrecisePos(cv::Mat resBuf, int nModelIdx, GeoModel::typItemPtr ModelItem, GeoResult & result)
{
	//	return;

	int nImgWid = resBuf.cols;
	int nImgHei = resBuf.rows;

	// 	CStdioFile file;
	// 	file.Open(_T("d:\\testimage\\workimage\\findcoeff.csv"), CFile::modeCreate | CFile::modeWrite);

#ifdef _DEBUG
	{
		//		CString sLog;
		//		sLog.Format(_T("try fitting - %d"), result.Count());
		// 		TRACE( sLog );
	}
#endif

	for (int fnd = 0; fnd < result.Count(); fnd++)
	{
		int x = result._Center_x[fnd];
		int y = result._Center_y[fnd];
		if (x >= 2 && x < (nImgHei - 2) &&
			y >= 2 && y < (nImgWid - 2))
		{
			std::vector<jsl::Point3d<double, double, double>> Val(5 * 5);
			int n(0);

			for (int iy = 0, i = -2; iy < 5; iy++, i++)
			{
				for (int ix = 0, j = -2; ix < 5; ix++, j++)
				{
					if (resBuf.ptr<float>(x + i)[y + j] == 0.0f)
						continue;
					Val[n++] = jsl::Point3d<double, double, double>(ix, iy, resBuf.ptr<float>(x + i)[y + j]);
				}
			}

			if (n < 25)
			{
#ifdef _DEBUG
				//				CString sLog;
				//				sLog.Format(_T("fitting fail[%d] - %d(%d,%d,%d,%d)"), fnd, n, x, y, nImgHei,nImgWid);
				// 				TRACE( sLog );
#endif				
				continue;
			}

			// 			file.WriteString(_T("\n"));

			jsl::LeastSquare ls;
			std::vector<double> Coe;
			ls.Run2d(2, Val, Coe);

			// 			{
			// 				CString sLine;
			// 				for (int i=0; i<Coe.size(); i++)
			// 				{
			// 					CString sData;
			// 					sData.Format(_T("%e,"), Coe[i]);
			// 					sLine += sData;
			// 				}
			// 				sLine += _T("\n");
			// 				file.WriteString(sLine);
			// 			}
			// 
			// 			file.WriteString(_T("\n"));

			cv::Mat lum(2, 2, CV_64FC1);
			cv::Mat inv(2, 2, CV_64FC1);
			cv::Mat vec(2, 1, CV_64FC1);
			cv::Mat coe(2, 1, CV_64FC1);

			lum.ptr<double>(0)[0] = 2.0f * Coe[0];
			lum.ptr<double>(0)[1] = Coe[2];
			lum.ptr<double>(1)[0] = Coe[2];
			lum.ptr<double>(1)[1] = 2.0f * Coe[1];
			vec.ptr<double>(0)[0] = -Coe[3];
			vec.ptr<double>(1)[0] = -Coe[4];

			cv::invert(lum, inv, cv::DECOMP_CHOLESKY);
			coe = inv * vec;

			std::vector<double> Coff(2);
			for (int j = 0; j < 2; j++)
				Coff[j] = coe.ptr<double>(j)[0];

			// 			{
			// 				CString sLine;
			// 				for (int i=0; i<Coff.size(); i++)
			// 				{
			// 					CString sData;
			// 					sData.Format(_T("%f,"), Coff[i]);
			// 					sLine += sData;
			// 				}
			// 				sLine += _T("\n");
			// 				file.WriteString(sLine);
			// 			}
			if (Coff[0] > 0 && Coff[1] > 0 && Coff[0] < 4 && Coff[1] < 4)
			{
				result._Center_x[fnd] = x + (2.0) - Coff[1];
				result._Center_y[fnd] = y + (2.0) - Coff[0];
			}
		}
		else
		{
#ifdef _DEBUG
			// 			for (int iy=0, i=-2; iy<5; iy++, i++)
			// 			{
			// 				CString sLog;
			// 				for (int ix=0, j=-2; ix<5; ix++, j++)
			// 				{
			// 					CString sTmp;
			// 					sTmp.Format(_T("%f, "), resBuf.ptr<float>(x+i)[y+j]);
			// 					sLog += sTmp;
			// 				}
			// 				TRACE( sLog );
			// 			}

			// 			CString sLog;
			// 			sLog.Format(_T("no fitting[%d] - (%d,%d,%d,%d)"), fnd, x, y, nImgHei, nImgWid);
			// 			TRACE( sLog );
#endif
		}
	}

	// 	file.Close();
}
void GeoMatch::_GetResult(GeoResult * resData, int nModelCount, int nResCnt, GeoResult & result)
{
	// 여기서 모델 크기의 75% 범위에서 Group화가 되어야 한다.

	std::set<GeoSortItem, std::greater<GeoSortItem>> SortedRes;
	for (int i = 0; i < nModelCount; i++)
	{
		for (int j = 0; j < nResCnt; j++)
		{
			if (resData[i]._Equality[j] <= 0.0)
				continue;//resData[i]._Equality[j] = 0;

			SortedRes.insert(GeoSortItem(resData[i]._Equality[j],
				resData[i]._AngleStepId[j],
				resData[i]._Angle[j],
				resData[i]._Center_x[j],
				resData[i]._Center_y[j],
				i, j));
		}
	}


	int n(0);
	for (auto iter = SortedRes.begin(); iter != SortedRes.end() && n < result.Count(); iter++, n++)
	{
		result._Equality[n] = iter->_Score;
		result._AngleStepId[n] = iter->_Id;
		result._Angle[n] = iter->_Angle;
		result._Center_x[n] = iter->_X;
		result._Center_y[n] = iter->_Y;
	}

	result.setResCnt(n);

	//std::vector<GeoSortItem> SortedRes(nModelCount*nResCnt);
	//for (int i = 0; i<nModelCount; i++)
	//{
	//	for (int j = 0; j < nResCnt; j++)
	//	{
	//		SortedRes[i*nResCnt+j] = GeoSortItem(resData[i]._Equality[j],
	//								resData[i]._AngleStepId[j],
	//								resData[i]._Angle[j],
	//								resData[i]._Center_x[j],
	//								resData[i]._Center_y[j]);
	//	}
	//}
	//
	//std::sort(SortedRes.begin(), SortedRes.end(), std::greater<GeoSortItem>());
	//
	//int n(0);
	//for (auto iter = SortedRes.begin(); iter != SortedRes.end() && n<result.Count(); iter++, n++)
	//{
	//	result._Equality[n] = iter->_Score;
	//	result._AngleStepId[n] = iter->_Id;
	//	result._Angle[n] = iter->_Angle;
	//	result._Center_x[n] = iter->_X;
	//	result._Center_y[n] = iter->_Y;
	//}
}

void GeoMatch::FindModel(GeoModel & model, GeoResult & result, bool bIsDiv)
{
	cv::Mat src = *_ProcImg->_OrgImage;
	cv::Mat MoImg = model.Image();
	if ((CV_MAT_TYPE(src.type()) != CV_MAT_TYPE(MoImg.type()) && !(model.getMatchAlgo() == GeoModel::enmMatchAlgo::agImage && model.getModelnChannel() == 3)) || model.isProcessed() == false)
		return;

	if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
	{
		_FindImgModel(src, model, result, *_ProcImg->_fSrc, *_ProcImg->_half, *_ProcImg->_pyr);
		return;
	}
#ifndef GeoNoUseWriteTimeLog
	CQTimer qtm;
	double dDtZoom(0), dDtHalf(0), dDtOrg(0);
#endif
	// 변수 초기화
	double minScore(0.0);
	double greediness = model.getAcceptance();

	if (&result == NULL)
		return;

	for (int i = 0; i < result.Count(); i++)
		result._Equality[i] = 0;

	// 영상의 해상도 축소
	cv::Mat img = *_ProcImg->_fSrc, half = *_ProcImg->_half, pyr = *_ProcImg->_pyr;//_ImgProcess된 영상을 대입함. //shkim
#ifndef GeoNoUseWriteTimeLog
	qtm.StartTick();
#endif

	int pyrDownStep = model._pyrDownStep;
	if ((model.Image().rows < 20 || model.Image().cols < 20) && bIsDiv)
		pyrDownStep = 0;

	// 영상 전체 탐색
	GeoResult outResult(result.Count() + model.getAccuracy());
	GeoResult zoomResult(model._Models[GeoModel::enmDepth::spZoom].size() * (result.Count() + model.getAccuracy()));
	//_ImgProcess(pyr, model.getPreProcLv(GeoModel::enmDepth::spZoom), GeoModel::enmDepth::spZoom, model.getMatchAlgo());
	if (pyrDownStep > 0)
		_FindModel(pyr, model, GeoModel::enmDepth::spZoom, minScore, greediness, zoomResult, result.Count() + model.getAccuracy());
#ifndef GeoNoUseWriteTimeLog
	dDtZoom = qtm.EndTick() * 1000.0; qtm.StartTick();
#endif
	// 탐색 결과 점수를 상위 축소 Step에서 다시 계산 및 낮은 점수 탈락
	float fAngleStd = model.getAngle();
	float rScale = (float)(img.rows) / (float)(pyr.rows);
	float cScale = (float)(img.cols) / (float)(pyr.cols);
	if (pyrDownStep > 1 && model.getAccuracy() != GeoModel::enmAccuracy::acLow)
	{
		GeoResult halfResult(zoomResult.Count());
		//_ImgProcess(half, model.getPreProcLv(GeoModel::enmDepth::spHalf), GeoModel::enmDepth::spHalf, model.getMatchAlgo());
		_TunningRes(half, pyr, model._Models[GeoModel::enmDepth::spHalf], fAngleStd, pyrDownStep, minScore, greediness, zoomResult, halfResult, false, model.getMatchAlgo(), model.getPrecision());
		GeoResult orgResult(result.Count() + model.getAccuracy());
		_GetResult(&halfResult, 1, halfResult.Count(), orgResult);
#ifndef GeoNoUseWriteTimeLog
		dDtHalf = qtm.EndTick() * 1000.0; qtm.StartTick();
#endif
		//_ImgProcess(fSrc, model.getPreProcLv(GeoModel::enmDepth::spOrg), GeoModel::enmDepth::spOrg, model.getMatchAlgo());
		_TunningRes(img, half, model._Models[GeoModel::enmDepth::spOrg], fAngleStd, pyrDownStep, minScore, greediness, orgResult, outResult, model.getFindSubPixel(), model.getMatchAlgo(), model.getPrecision());
		_GetResult(&outResult, 1, outResult.Count(), result);
	}
	else
	{
		if (pyrDownStep > 0 && model.getAccuracy() != GeoModel::enmAccuracy::acLow)
		{
			GeoResult halfResult(zoomResult.Count());
			//_ImgProcess(fSrc, model.getPreProcLv(GeoModel::enmDepth::spOrg), GeoModel::enmDepth::spOrg, model.getMatchAlgo());
			_TunningRes(img, pyr, model._Models[GeoModel::enmDepth::spOrg], fAngleStd, pyrDownStep, minScore, greediness, zoomResult, halfResult, model.getFindSubPixel(), model.getMatchAlgo(), model.getPrecision());
			_GetResult(&halfResult, 1, halfResult.Count(), result);

			// 			for(int j=0; j<halfResult.Count(); j++)
			// 			{
			// 				cv::Mat resImg;
			// 				fSrc.copyTo(resImg);
			// 				DrawContours(resImg, model, halfResult, cv::Scalar(255), 1, j);
			// 
			// 				CString sExportName;
			// 				sExportName.Format(_T("d:\\Test_%.0f_%.3f.bmp"), model._Models[GeoModel::enmDepth::spOrg][halfResult._AngleStepId[j]]->_Angle, halfResult._Equality[j]);
			// 				cv::imwrite(std::string(CT2A(sExportName)), resImg);
			// 			}
		}
		else
		{
			//_ImgProcess(fSrc, model.getPreProcLv(GeoModel::enmDepth::spOrg), GeoModel::enmDepth::spOrg, model.getMatchAlgo());
			//			_TunningRes(fSrc, img, model._Models[GeoModel::enmDepth::spOrg], fAngleStd, nPyrDownStep, minScore, greediness, zoomResult, result, model.getFindSubPixel(), model.getMatchAlgo());

			int cnt = model._Models[GeoModel::enmDepth::spOrg].size();
			_FindModel(model._Models[GeoModel::enmDepth::spOrg][cnt / 2], cnt / 2, cv::Point(0, 0), cv::Size(_ProcImg->_fSrc->cols, _ProcImg->_fSrc->rows), 0, minScore, greediness, result, false/*model.getFindSubPixel()*/, model.getMatchAlgo());
			result.setResCnt(1);
		}
	}
#ifndef GeoNoUseWriteTimeLog
	dDtOrg = qtm.EndTick() * 1000.0; qtm.StartTick();
#endif
	int nResCount(0.0);
	for (int i = 0; i < result.getResCnt(); i++)
		result._Equality[i] = result._Equality[i] * 100.0f;


#ifndef GeoNoUseWriteTimeLog
	CString sLog;
	sLog.Format(_T("d:\\EdgeMatchRes.txt"));
	CStdioFile file;
	if (file.Open(sLog, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate) == TRUE)
	{
		file.SeekToEnd();
		sLog.Format(_T("%.3fms,%.3fms,%.3fms\n"), dDtZoom, dDtHalf, dDtOrg);
		file.WriteString(sLog);
		file.Close();
	}
#endif

	if (_bSaveProcData.IsEmpty() == false)
	{
		cv::Mat draw, srcCopy;
		srcCopy = src.clone();
		cv::Mat ImgList[3] = { srcCopy, srcCopy, srcCopy };
		cv::merge(ImgList, 3, draw);
		DrawContours(draw, model, result, cv::Scalar(0, 0, 255), 1);

		SaveImg(draw, _T("FindModel"), _T("FindModelRes"));
	}
}

void GeoMatch::SaveImg(cv::Mat img, CString funcName, CString imgName)
{
	if (_bSaveProcData.IsEmpty() == false)
	{
		LARGE_INTEGER freq, curr;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&curr);

		CString sTime;
		sTime.Format(_T("_%.3f"), (double)(curr.QuadPart) / (double)(freq.QuadPart));

		std::string sPath;
		sPath = std::string("D:\\TestImage\\workimage\\") + std::string(CT2A(funcName)) + std::string("_") + std::string(CT2A(imgName)) + std::string(CT2A(_bSaveProcData)) + std::string(CT2A(sTime)) + std::string(".bmp");
		cv::imwrite(sPath, img);
	}
}

void GeoMatch::SetImgProcess(std::shared_ptr<ImgProcessing> ProcImg)
{
	_ProcImg = ProcImg;
}


void GeoMatch::_TunningRes(cv::Mat src, cv::Mat zoom, GeoModel::typItemList & model, float fAngleStd, int pyrDownStep, double minScore, double greediness, GeoResult & zoomResult, GeoResult & srcResult, bool bCalcPrecisePos, int use_algo_model, int Precision)
{
	float rScale = (float)(src.rows) / (float)(zoom.rows);
	float cScale = (float)(src.cols) / (float)(zoom.cols);

	int step = (int)pow(2.0f, pyrDownStep);
	double margin(4.0f);
	if (Precision == GeoModel::enmPrecision::pcMedium)
		margin = 6.0f;
	else if (Precision == GeoModel::enmPrecision::pcHigh)
		margin = 10.0f;
#ifndef GeoNoUseMultiThread
	Concurrency::parallel_for((size_t)0, (size_t)zoomResult.Count(), [&](size_t i)
#else

	for (int i = 0; i < zoomResult.getResCnt(); i++)
#endif
	{
		GeoModel::typItemPtr item = model[zoomResult._AngleStepId[i]];
		float rsx = zoomResult._Center_y[i] * cScale - (item->_modelWidth * 0.5f) - cScale;
		float rsy = zoomResult._Center_x[i] * rScale - (item->_modelHeight * 0.5f) - rScale;

		float rex = rsx + item->_modelWidth + cScale;
		float rey = rsy + item->_modelHeight + rScale;

		if (rsx < 0.0f)
			rsx = 0.0f;
		if (rsy < 0.0f)
			rsy = 0.0f;
		if (rex > src.cols)
			rex = src.cols;
		if (rey > src.rows)
			rey = src.rows;

		// 		int StX = rsx;
		// 		int StY = rsy;
		// 		int EdX = rex;
		// 		int EdY = rey;
		// 		int Wid = rex - rsx;
		// 		int Len = rey - rsy;
		// 
		// 		cv::Mat srcROI(Len, Wid, src.type(), &src.ptr(StY)[StX], src.step1());

		int fndStX = zoomResult._Center_y[i] * cScale - cScale * margin;// - StX;
		int fndStY = zoomResult._Center_x[i] * rScale - rScale * margin;// - StY;

		if (fndStX < 0.0f)
			fndStX = 0.0f;
		if (fndStY < 0.0f)
			fndStY = 0.0f;

		int fndEdX = fndStX + cScale * margin * 2;
		int fndEdY = fndStY + rScale * margin * 2;

		// 		if(fndEdX>Wid)
		// 			fndEdX = Wid;
		// 		if(fndEdY>Len)
		// 			fndEdY = Len;

		if (fndEdX > src.cols)
			fndEdX = src.cols;
		if (fndEdY > src.rows)
			fndEdY = src.rows;

		GeoResult eachResult(1);
		_FindModel(item, zoomResult._AngleStepId[i], cv::Point(fndStX, fndStY), cv::Size(fndEdX - fndStX, fndEdY - fndStY), fAngleStd, minScore, greediness, eachResult, bCalcPrecisePos, use_algo_model);

		srcResult._Equality[i] = eachResult._Equality[0];
		srcResult._Center_x[i] = /*StY + */eachResult._Center_x[0];
		srcResult._Center_y[i] = /*StX + */eachResult._Center_y[0];
		srcResult._Angle[i] = eachResult._Angle[0];
		srcResult._AngleStepId[i] = eachResult._AngleStepId[0];
	}
#ifndef GeoNoUseMultiThread
	);
#endif
}
void GeoMatch::_FindModel(cv::Mat src, GeoModel & model, GeoModel::enmDepth depth, double minScore, double greediness, GeoResult & result, int nResCnt, bool bCalcPrecisePos)
{
	GeoModel::typItemList & ModelList = model._Models[depth];

	int nModelCount = ModelList.size();

	cv::Size Ssize = cv::Size(src.cols, src.rows);

	cv::Mat Sdl = _ProcImg->_ProcBuf[depth].Sdl;
	cv::Mat Sdx = _ProcImg->_ProcBuf[depth].Sdx;
	cv::Mat Sdy = _ProcImg->_ProcBuf[depth].Sdy;
	float * matGradMag = _ProcImg->_ProcBuf[depth].matGradMag;

	float fAngleStd = model.getImageAngle();
	int nAngleStep = model.getAngleStep();

	float fSearchAngleSt(0), fSearchAngleEd(0);
	model.getSearchAngle(fSearchAngleSt, fSearchAngleEd);
	fSearchAngleSt -= FLT_EPSILON;
	fSearchAngleEd += FLT_EPSILON;

	// 여기서 찾을 각도에 드는 것만 모은다.
	GeoModel::typItemList tmpMdList;
	for (int i = 0; i < nModelCount; i++)
	{
		GeoModel::typItemList::iterator iter = ModelList.find(i);
		float fa = iter->second->_Angle - fAngleStd;

		if (fSearchAngleSt <= fa && fa <= fSearchAngleEd)
		{
			tmpMdList[i] = iter->second;
		}
	}

	std::vector<int> mdTmpIdx;
	std::vector<GeoModel::typItemPtr> mdTmpList;//( nModelCount );
	{
		int i = 0;
		for (auto iter = tmpMdList.begin(); iter != tmpMdList.end(); iter++, i++)
		{
			if (nAngleStep > 1 && (i%nAngleStep) != 0)
				continue;
			if (iter != tmpMdList.end())
			{
				mdTmpIdx.emplace_back(iter->first);
				mdTmpList.emplace_back(iter->second);
			}
		}
	}

	// 	for (int i = 0; i < nModelCount; i+=nAngleStep)
	// 	{
	// 		GeoModel::typItemList::iterator iter = ModelList.find(i);
	// 		
	// 		if(iter != ModelList.end())
	// 		{
	// 			mdTmpIdx.emplace_back( iter->first );
	// 			mdTmpList.emplace_back( iter->second );
	// 		}
	// 	}

	nModelCount = mdTmpList.size();

	//cv::Mat * resBuf = new cv::Mat[nModelCount];
	cv::Mat * resBuf = g_pMManager->pem_new<cv::Mat>(true, nModelCount, (PCHAR)__FUNCTION__, __LINE__);
	//GeoResult * resData = new GeoResult[nModelCount];
	GeoResult * resData = g_pMManager->pem_new<GeoResult>(true, nModelCount, (PCHAR)__FUNCTION__, __LINE__);
	GeoModel::typItemPtr * pmdTmpList = &mdTmpList[0];

#ifndef GeoNoUseMultiThread
	Concurrency::parallel_for((size_t)0, (size_t)nModelCount, [&](size_t i)
#else
	for (int i = 0; i < nModelCount; i++)
#endif
	{
		GeoModel::typItemPtr ModelItem = pmdTmpList[i];
		if (ModelItem->_noOfCordinates <= 0)
		{
			resData[i].Alloc(nResCnt);
#ifndef GeoNoUseMultiThread
			return;
#else
			continue;
#endif
		}

		resBuf[i] = cv::Mat(Ssize.height, Ssize.width, CV_32FC1);

		float normMinScore = minScore / ModelItem->_noOfCordinates;
		float normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / ModelItem->_noOfCordinates;
		if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agIntaglio || model.getMatchAlgo() == GeoModel::enmMatchAlgo::agRelief)
		{
#ifndef GeoNoUseSimd
			(this->*FunPtr_CalcCoefficientImg_sse)(Sdl, _ProcImg->_ProcBuf[depth].Integral, resBuf[i], cv::Point(0, 0), Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#else
			_CalcCoefficientImg(Sdl, _ProcImg->_ProcBuf[depth].Integral, resBuf[i], cv::Point(0, 0), Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#endif			
		}
		else if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agValue)
		{
#ifndef GeoNoUseSimd
			(this->*FunPtr_CalcCoefficientVal_sse)(Sdl, _ProcImg->_ProcBuf[depth].Integral, _ProcImg->_ProcBuf[depth].SqIntegral,
				resBuf[i], cv::Point(0, 0), Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#else
			_CalcCoefficientVal(Sdl, _ProcImg->_ProcBuf[depth].Integral, _ProcImg->_ProcBuf[depth].SqIntegral,
				resBuf[i], cv::Point(0, 0), Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#endif	
		}
		else
		{
#ifndef GeoNoUseSimd
			(this->*FunPtr_CalcCoefficient_sse)(Sdx, Sdy, matGradMag, resBuf[i], cv::Point(0, 0), Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#else
			_CalcCoefficient(Sdx, Sdy, matGradMag, resBuf[i], cv::Point(0, 0), Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#endif			
		}

		resData[i].Alloc(nResCnt);
		_CalcResult(resBuf[i], mdTmpIdx[i], ModelItem, cv::Point(0, 0), Ssize, resData[i]);

		if (bCalcPrecisePos == true)
			_CalcPrecisePos(resBuf[i], mdTmpIdx[i], ModelItem, resData[i]);
	}
#ifndef GeoNoUseMultiThread
	);
#endif

	_AllResultProcedure(pmdTmpList, resData, nModelCount, result, nResCnt);

	/*delete [] resBuf;
	delete [] resData;*/
	g_pMManager->pem_delete(resBuf, true);
	g_pMManager->pem_delete(resData, true);
}
void GeoMatch::_AllResultProcedure(GeoModel::typItemPtr * pmdTmpList, GeoResult * resData, int nModelCount, GeoResult & result, int nResCnt)
{
	_GetResult(resData, nModelCount, nResCnt, result);
}

void GeoMatch::_FindModel(GeoModel::typItemPtr ModelItem, int idx, cv::Point SStart, cv::Size Ssize, float fAngleStd, double minScore, double greediness, GeoResult & result, bool bCalcPrecisePos, int use_algo_model)
{
	if (ModelItem != nullptr)
	{
		if (ModelItem->_noOfCordinates <= 0)
		{
			result.Alloc(result.Count());
			return;
		}

		cv::Mat Sdl = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdl;
		cv::Mat Sdx = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdx;
		cv::Mat Sdy = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdy;
		float * matGradMag = _ProcImg->_ProcBuf[ModelItem->_PyrDown].matGradMag;

		cv::Mat resBuf;

		int nResCnt = result.Count();
		if (ModelItem->_PyrDown == (int)GeoModel::enmDepth::spOrg)
			resBuf = cv::Mat::zeros(_ProcImg->_fSrc->rows, _ProcImg->_fSrc->cols, CV_32FC1);
		else if (ModelItem->_PyrDown == (int)GeoModel::enmDepth::spHalf)
			resBuf = cv::Mat::zeros(_ProcImg->_half->rows, _ProcImg->_half->cols, CV_32FC1);
		else
			resBuf = cv::Mat::zeros(_ProcImg->_pyr->rows, _ProcImg->_pyr->cols, CV_32FC1);

		float normMinScore = minScore / ModelItem->_noOfCordinates;
		float normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / ModelItem->_noOfCordinates;
		if (use_algo_model == GeoModel::enmMatchAlgo::agIntaglio || use_algo_model == GeoModel::enmMatchAlgo::agRelief)
		{
			// 			_CalcCoefficientImg_sse(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, resBuf, SStart, Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
			// 
			// 			cv::Mat ResSave, ResSave2, ResSave3;
			// 			resBuf.convertTo(ResSave, CV_8U, 128);
			// 			cv::imwrite(std::string("d:\\CalcCoefficientImg_sse.bmp"), ResSave);
#ifndef GeoNoUseSimd
			(this->*FunPtr_CalcCoefficientImg_sse)(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, resBuf, SStart, Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#else
			_CalcCoefficientImg(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, resBuf, SStart, Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#endif
			// 			resBuf2.convertTo(ResSave2, CV_8U, 128);
			// 			cv::imwrite(std::string("d:\\CalcCoefficientImg_Ori.bmp"), ResSave2);
			// 
			// 			cv::absdiff(ResSave, ResSave2, ResSave3);
			// 			cv::imwrite(std::string("d:\\CalcCoefficientImg_Diff.bmp"), ResSave3);
			// 			cv::equalizeHist(ResSave3, ResSave3);
			// 			cv::imwrite(std::string("d:\\CalcCoefficientImg_Equal.bmp"), ResSave3);
		}
		else if (use_algo_model == GeoModel::enmMatchAlgo::agValue)
		{
#ifndef GeoNoUseSimd
			(this->*FunPtr_CalcCoefficientVal_sse)(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, _ProcImg->_ProcBuf[ModelItem->_PyrDown].SqIntegral,
				resBuf, SStart, Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#else
			_CalcCoefficientVal(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, _ProcImg->_ProcBuf[ModelItem->_PyrDown].SqIntegral,
				resBuf, SStart, Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#endif	
		}
		else
		{
#ifndef GeoNoUseSimd
			(this->*FunPtr_CalcCoefficient_sse)(Sdx, Sdy, matGradMag, resBuf, SStart, Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#else
			_CalcCoefficient(Sdx, Sdy, matGradMag, resBuf, SStart, Ssize, ModelItem, fAngleStd, normMinScore, normGreediness, false);
#endif
		}

		result.Alloc(nResCnt);
		_CalcResult(resBuf, idx, ModelItem, SStart, Ssize, result);

		if (bCalcPrecisePos == true)
			_CalcPrecisePos(resBuf, idx, ModelItem, result);
	}
}

void GeoMatch::_FindImgModel(cv::Mat src, GeoModel & model, GeoResult & result, cv::Mat& org, cv::Mat& half, cv::Mat& pyr)
{
#ifndef GeoNoUseWriteTimeLog
	CQTimer qtm;
	qtm.StartTick();
#endif

	for (int i = 0; i < result.Count(); i++)
		result._Equality[i] = 0;

	cv::Mat mdImg = model._GetAngle(src.channels() == 3 ? model.color_Image() : model.Image(), model.getImageAngle()), mdHalf;

	//_GetPyrDown(mdImg, mdHalf);
	double scaleVal = 0.5;
	mdHalf = cv::Mat(mdImg.rows*scaleVal, mdImg.cols*scaleVal, CV_MAKETYPE(CV_8U, mdImg.channels()));
	cv::resize(mdImg, mdHalf, cv::Size(mdImg.cols*scaleVal, mdImg.rows*scaleVal));

#ifdef GeoSaveDebug
	cv::imwrite(std::string("d:\\target.bmp"), src);
	cv::imwrite(std::string("d:\\model.bmp"), mdImg);

	cv::imwrite(std::string("d:\\model_org.bmp"), src.channels() == 3 ? model.color_Image() : model.Image());
#endif
	// 
	cv::Mat scoreMat;
	if (half.cols < mdHalf.cols || half.rows < mdHalf.rows)
		return;
	cv::matchTemplate(half, mdHalf, scoreMat, cv::TM_CCOEFF_NORMED);

#ifdef GeoSaveDebug
	imwrite("D:\\testimage\\patternMathTest\\ImMatinputImg_m_model.bmp", ImMatinputImg.m_model);
	imwrite("D:\\testimage\\patternMathTest\\ImMatModel_m_model.bmp", ImMatinputImg.m_model);
	imwrite("D:\\testimage\\patternMathTest\\scoreMat.bmp", scoreMat);
	cv::Mat ResSave;
	cv::Mat ResBuf = scoreMat.clone();//ResMat.clone();
	ResBuf += 1.0;
	ResBuf.convertTo(ResSave, CV_8U, 128);
	cv::imwrite(std::string("d:\\image.bmp"), ResSave);
#endif
	cv::Point	ptMin, ptMax;
	double		Min(0.0), Max(0.0);

	int nResCount(0.0);
	for (int i = 0; i < result.Count(); i++)
	{
		nResCount++;
		cv::minMaxLoc(scoreMat, &Min, &Max, &ptMin, &ptMax);
		Min *= 100.0;
		Max *= 100.0;
		result._Equality[i] = Max;
		result._Angle[i] = model.getImageAngle();
		result._AngleStepId[i] = i;
		//		result._Center_y[i] = ((double)(ptMax.x) + (double)(mdHalf.cols) * 0.5);
		//		result._Center_x[i] = ((double)(ptMax.y) + (double)(mdHalf.rows) * 0.5);
		result._Center_y[i] = ((double)(ptMax.x) + (double)(mdHalf.cols) * 0.5) * 2.0;
		result._Center_x[i] = ((double)(ptMax.y) + (double)(mdHalf.rows) * 0.5) * 2.0;
		// 		int rm_w = mdImg.cols / 4;
		// 		int rm_l = mdImg.rows / 4;
		int rm_w = mdHalf.cols / 4;
		int rm_l = mdHalf.rows / 4;

		int nStX = ptMax.x - rm_w;
		int nStY = ptMax.y - rm_l;
		int nSzW = rm_w;
		int nSzL = rm_l;

		if (nStX < 0)
		{
			nSzW += nStX;
			nStX = 0;
		}
		if (nStY < 0)
		{
			nSzL += nStY;
			nStY = 0;
		}

		if ((nStX + nSzW) > scoreMat.cols)
		{
			nSzW -= (nStX + nSzW) - scoreMat.cols;
		}

		if ((nStY + nSzL) > scoreMat.rows)
		{
			nSzW -= (nStY + nSzL) - scoreMat.rows;
		}

		cv::Mat rmRoi(scoreMat, cv::Rect(nStX, nStY, nSzW, nSzL));
		rmRoi = 0;
	}

#ifndef GeoNoUseWriteTimeLog
	double dt = qtm.EndTick() * 1000.0;
	CString sLog;
	sLog.Format(_T("d:\\EdgeMatchRes.txt"));
	CStdioFile file;
	if (file.Open(sLog, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate) == TRUE)
	{
		file.SeekToEnd();
		sLog.Format(_T("%.3fms\n"), dt);
		file.WriteString(sLog);
		file.Close();
	}
#endif

	result.setResCnt(nResCount);
}

void GeoMatch::DrawContours(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth, int i)
{
	if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
		return;

	float angle = model.getAngle() / 180.0f * M_PI;
	float sinth = sin(angle);
	float costh = cos(angle);

	cv::Point pt(Res._Center_x[i] + 0.5f, Res._Center_y[i] + 0.5f);

	GeoModel::typItemPtr ModelItem;
	GeoModel::typItemList::iterator iter = model._Models[GeoModel::enmDepth::spOrg].find(Res._AngleStepId[i]);
	if (iter != model._Models[GeoModel::enmDepth::spOrg].end())
	{
		ModelItem = iter->second;

		float * coordX = ModelItem->_coordX;
		float * coordY = ModelItem->_coordY;
		int * Id = ModelItem->_DivisionId;
		int * SubId = ModelItem->_SubDivisionId;
		BYTE * pFlags = ModelItem->_Flags;
		for (int j = 0; j < ModelItem->_noOfCordinates; j++)
		{
			if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
				continue;

			if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDraw) == GeoModel::enmPtDataFlag::PdfDraw)
				continue;
			cv::Point point;
			//	point.y = pt.x + ((-coordX[j])*costh - coordY[j]*sinth);
			//	point.x = pt.y + (( coordX[j])*sinth + coordY[j]*costh);
			point.y = coordX[j] + pt.x + 0.49f;
			point.x = coordY[j] + pt.y + 0.49f;
			if (Id[j] > 0)
				cv::line(source, point, point, cv::Scalar(10 * (Id[j] * GeoCharDivCnt + SubId[j])), lineWidth);
			else
				cv::line(source, point, point, color, lineWidth);
		}
	}
}

void GeoMatch::DrawContoursAngle(cv::Mat source, GeoModel& model, GeoResult& Res, cv::Scalar color, int lineWidth, float angle)
{
	if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
		return;

	//float angle = model.getAngle() / 180.0f * P_PI;
	float sinth = sin(angle);
	float costh = cos(angle);
	for (int i = 0; i < Res.getResCnt(); i++)
	{
		cv::Point pt(Res._Center_x[i] + 0.5f, Res._Center_y[i] + 0.5f);

		GeoModel::typItemPtr ModelItem;
		GeoModel::typItemList::iterator iter = model._Models[GeoModel::enmDepth::spOrg].find(Res._AngleStepId[i]);
		if (iter != model._Models[GeoModel::enmDepth::spOrg].end())
		{
			ModelItem = iter->second;

			float* coordX = ModelItem->_coordX;
			float* coordY = ModelItem->_coordY;
			int* Id = ModelItem->_DivisionId;
			int* SubId = ModelItem->_SubDivisionId;
			BYTE* pFlags = ModelItem->_Flags;
			for (int j = 0; j < ModelItem->_noOfCordinates; j++)
			{
				if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;
				if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDraw) == GeoModel::enmPtDataFlag::PdfDraw)
					continue;

				cv::Point point;
				//point.y = pt.x + ((-coordX[j])*costh - coordY[j]*sinth);
				//point.x = pt.y + (( coordX[j])*sinth + coordY[j]*costh);
				point.y = pt.x + ((coordX[j])*costh - coordY[j] * sinth);
				point.x = pt.y + ((coordX[j])*sinth + coordY[j] * costh);
				//point.y = coordX[j] + pt.x + 0.49f;
				//point.x = coordY[j] + pt.y + 0.49f;
				if (Id[j] > 0)
					cv::line(source, point, point, cv::Scalar(10 * (Id[j] * GeoCharDivCnt + SubId[j])), lineWidth);
				else
					cv::line(source, point, point, color, lineWidth);
			}
		}
	}
}
void GeoMatch::DrawContours(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth)
{
	if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
		return;

	float angle = model.getAngle() / 180.0f * M_PI;
	float sinth = sin(angle);
	float costh = cos(angle);
	for (int i = 0; i < Res.getResCnt(); i++)
	{
		cv::Point pt(Res._Center_x[i] + 0.5f, Res._Center_y[i] + 0.5f);

		GeoModel::typItemPtr ModelItem;
		GeoModel::typItemList::iterator iter = model._Models[GeoModel::enmDepth::spOrg].find(Res._AngleStepId[i]);
		if (iter != model._Models[GeoModel::enmDepth::spOrg].end())
		{
			ModelItem = iter->second;

			float * coordX = ModelItem->_coordX;
			float * coordY = ModelItem->_coordY;
			int * Id = ModelItem->_DivisionId;
			int * SubId = ModelItem->_SubDivisionId;
			BYTE * pFlags = ModelItem->_Flags;
			for (int j = 0; j < ModelItem->_noOfCordinates; j++)
			{
				if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;
				if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDraw) == GeoModel::enmPtDataFlag::PdfDraw)
					continue;

				cv::Point point;
				//	point.y = pt.x + ((-coordX[j])*costh - coordY[j]*sinth);
				//	point.x = pt.y + (( coordX[j])*sinth + coordY[j]*costh);
				point.y = coordX[j] + pt.x + 0.49f;
				point.x = coordY[j] + pt.y + 0.49f;
				if (Id[j] > 0)
					cv::line(source, point, point, cv::Scalar(10 * (Id[j] * GeoCharDivCnt + SubId[j])), lineWidth);
				else
					cv::line(source, point, point, color, lineWidth);
			}
		}
	}
}
void GeoMatch::DrawContours(cv::Mat source, GeoModel::typItemPtr ModelItem, GeoResult & Res, cv::Scalar color, int lineWidth)
{
	if (ModelItem == nullptr)
		return;

	float angle = ModelItem->_Angle / 180.0f * M_PI;
	float sinth = sin(angle);
	float costh = cos(angle);
	for (int i = 0; i < Res.getResCnt(); i++)
	{
		cv::Point pt(Res._Center_x[i], Res._Center_y[i]);

		float * coordX = ModelItem->_coordX;
		float * coordY = ModelItem->_coordY;
		BYTE * pFlags = ModelItem->_Flags;
		for (int j = 0; j < ModelItem->_noOfCordinates; j++)
		{
			if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
				continue;

			if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDraw) == GeoModel::enmPtDataFlag::PdfDraw)
				continue;

			cv::Point point;
			//	point.y = pt.x + ((-coordX[j])*costh - coordY[j]*sinth);
			//	point.x = pt.y + (( coordX[j])*sinth + coordY[j]*costh);
			point.y = coordX[j] + pt.x + 0.49f;
			point.x = coordY[j] + pt.y + 0.49f;
			cv::line(source, point, point, color, lineWidth);
		}
	}
}
cv::Mat GeoMatch::DrawContourColor(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth)
{
	if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
		return source.clone();

	cv::Mat src = cv::Mat::zeros(source.rows, source.cols, CV_8UC3);
	cv::cvtColor(source, src, cv::COLOR_GRAY2BGR);

	float angle = model.getAngle() / 180.0f * M_PI;
	float sinth = sin(angle);
	float costh = cos(angle);
	for (int i = 0; i < Res.getResCnt(); i++)
	{
		cv::Point pt(Res._Center_x[i] + 0.5f, Res._Center_y[i] + 0.5f);

		GeoModel::typItemPtr ModelItem;
		GeoModel::typItemList::iterator iter = model._Models[GeoModel::enmDepth::spOrg].find(Res._AngleStepId[i]);
		if (iter != model._Models[GeoModel::enmDepth::spOrg].end())
		{
			ModelItem = iter->second;

			float * coordX = ModelItem->_coordX;
			float * coordY = ModelItem->_coordY;
			int * Id = ModelItem->_DivisionId;
			int * SubId = ModelItem->_SubDivisionId;
			BYTE * pFlags = ModelItem->_Flags;
			for (int j = 0; j < ModelItem->_noOfCordinates; j++)
			{
				if ((pFlags[j] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;

				cv::Point point;
				//	point.y = pt.x + ((-coordX[j])*costh - coordY[j]*sinth);
				//	point.x = pt.y + (( coordX[j])*sinth + coordY[j]*costh);
				point.y = coordX[j] + pt.x + 0.49f;
				point.x = coordY[j] + pt.y + 0.49f;
				if (Id[j] > 0)
					cv::line(src, point, point, cv::Scalar(10 * (Id[j] * GeoCharDivCnt + SubId[j]), 0, 0), lineWidth);
				else
					cv::line(src, point, point, color, lineWidth);
			}
		}
	}

	return src;
}

bool GeoMatch::_PtInPoly(cv::Point2d * ptArray, int nCount, cv::Point2d point)
{
	bool oddNodes = false;
	int i, j = nCount - 1;

	for (i = 0; i < nCount; i++)
	{
		if ((ptArray[i].y < point.y && ptArray[j].y >= point.y) || (ptArray[j].y < point.y && ptArray[i].y >= point.y))
		{
			// (y-Y1)/(x-X1)과 (Y2-Y1)/(X2-X1)의 비교
			if (ptArray[i].x + (point.y - ptArray[i].y) / (ptArray[j].y - ptArray[i].y) * (ptArray[j].x - ptArray[i].x) < point.x)
			{
				oddNodes = !oddNodes;
			}
		}
		j = i;
	}

	return oddNodes;
}

// 소벨 함수를 사용한 아래 두 함수는 점수가 낮을수록 포커스가 더 잘 맞은 겁니다.
double GeoMatch::grad_1stOrder_focus_algo_1(cv::Mat src)
{
	cv::Mat gradX, gradY;
	cv::Sobel(src, gradX, CV_16S, 1, 0);
	cv::Sobel(src, gradY, CV_16S, 0, 1);
	cv::Mat sobel = cv::abs(gradX) + cv::abs(gradY);
	cv::Scalar sobelSum = cv::sum(sobel);
	double PixelCount = 2.3 / (src.rows * src.cols);
	double Sum = sobelSum.val[0];
	double Threshold = PixelCount * Sum;

	cv::Mat bin;
	cv::threshold(sobel, bin, Threshold, 65535, cv::THRESH_BINARY);
	return cv::countNonZero(bin);
}

double GeoMatch::grad_1stOrder_focus_algo_2(cv::Mat src, double a = 0.2/*0.0 ~ 0.5*/)
{
	// lower value focus
	cv::Mat gradX, gradY;
	cv::Sobel(src, gradX, CV_16S, 1, 0);
	cv::Sobel(src, gradY, CV_16S, 0, 1);
	cv::Mat sobel = cv::abs(gradX) + cv::abs(gradY);

	double dMinVal, dMaxVal;
	cv::Point loc;
	cv::minMaxLoc(sobel, &dMinVal, &dMaxVal, &loc);
	cv::Scalar mean, stddev;
	cv::meanStdDev(sobel, mean, stddev);

	double t = mean.val[0] + a * (dMaxVal - mean.val[0]);

	cv::Mat bin;
	cv::threshold(sobel, bin, t, 65535, cv::THRESH_BINARY);
	return cv::countNonZero(bin);
}

// 라플라시안을 사용한 아래 함수는 점수가 클수록 포커스가 더 잘 맞은 겁니다.
double GeoMatch::grad_2ndOrder_focus_algo_2(cv::Mat src, double a = 0.2/*0.0 ~ 0.5*/)
{
	// higher value focus
	cv::Mat grad, temp;
	cv::Laplacian(src, temp, CV_16S, 5);
	cv::convertScaleAbs(temp, grad);
	cv::Scalar gradSum = cv::sum(grad);
	return sqrt(gradSum.val[0]);
}
void GeoMatch::_CalcCoefficient_AVX_inline(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pEdgeY, float * pMag, BYTE * pFlag, float costh, float sinth)
{
	register __m256 partialSum = _mm256_setzero_ps();
	register __m256 num1 = _mm256_set1_ps(1);
	register __m256 numStd = _mm256_set1_ps(Min_Meg_Inv);

	int nRealNoOfCordinates(0);

	for (int m = 0; m < noOfCordinates; m++)
	{
		if ((pFlag[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			continue;
		nRealNoOfCordinates++;
		int curX = i + pCoordX[m];// + 0.49f;
		int curY = j + pCoordY[m];// + 0.49f;

		float iTx = pEdgeX[m];
		float iTy = pEdgeY[m];
		float iMag = pMag[m];

		register __m256i pSdx, pSdy;
		register __m256 vTx, vTy, vMag;//, vSx, vSy;
		__m256 pMatgrandmag;
		__m256 pMatgrandmagN;

		if (curX<0 || curY<0 || (curX)>ImgLen - 1 || (curY + 8)>(ImgWid - 1))
		{
			if ((curX) >= 0 && curX <= (ImgLen - 1) && (curY + 8) >= 0 && curY <= (ImgWid - 1))
			{

				short *_Sdx = Sdx.ptr<short>(curX);// (short *)(Sdx->data.ptr + Sdx->step*(curX));
				short *_Sdy = Sdy.ptr<short>(curX);// (short *)(Sdy->data.ptr + Sdy->step*(curX));

				int jj = 0;
				int jn = 0;
				int cntjj = 8;
				if (curY < 0)
				{
					jj = -curY;
					curY = 0;
				}
				else if ((curY + 7) > (ImgWid - 1))
					cntjj -= (curY + 7) - (ImgWid - 1);

				for (; jj < cntjj; jj++, jn++)
				{
					float iSx = _Sdx[curY + jn];
					float iSy = _Sdy[curY + jn];

					if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0) && (1.0f / matGradMag[(curX)*ImgWid + curY + jn] > Min_Meg))
						partialSum.m256_f32[jj] = partialSum.m256_f32[jj] + ((iSx*iTx) + (iSy*iTy)) * (iMag*matGradMag[(curX)*ImgWid + curY + jn]);
				}
			}
			continue;
		}
		short *_Sdx = Sdx.ptr<short>(curX);// short* _Sdx = (short*)(Sdx->data.ptr + Sdx->step*(curX));
		short *_Sdy = Sdy.ptr<short>(curX);// short* _Sdy = (short*)(Sdy->data.ptr + Sdy->step*(curX));
// 		pMatgrandmag = _mm256_loadu_ps(&matGradMag[curX*ImgWid + curY]);
// 		pMatgrandmagN = _mm256_loadu_ps(&matGradMag[curX*ImgWid + curY + 8]);

		partialSum = _mm256_add_ps(
			partialSum,
			_mm256_and_ps(
				_mm256_mul_ps(
					_mm256_add_ps(
						_mm256_mul_ps(
							vTx = _mm256_set1_ps(iTx),
							_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i*)&_Sdx[curY])))
						),
						_mm256_mul_ps(
							vTy = _mm256_set1_ps(iTy),
							_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i*)&_Sdy[curY])))
						)
					),
					_mm256_mul_ps(
						vMag = _mm256_set1_ps(iMag),
						_mm256_loadu_ps(&matGradMag[curX*ImgWid + curY]))
				)
				, _mm256_cmp_ps(numStd, _mm256_loadu_ps(&matGradMag[curX*ImgWid + curY]), _CMP_GT_OS)
			)
		);
	}
	int nResWid = resBuf.cols;
	int nResLen = resBuf.rows;
	if (nRealNoOfCordinates)
	{
		if (nResLen <= i)
			return;
		float * resPS = resBuf.ptr<float>(i);// (float*)(resBuf->data.ptr + resBuf->step*(i));
		if ((j + 8) <= nResWid)
			_mm256_storeu_ps(&resPS[j], _mm256_div_ps(partialSum, _mm256_set1_ps(nRealNoOfCordinates)));
		else
		{
			__m256 ScoreRes = _mm256_div_ps(partialSum, _mm256_set1_ps(nRealNoOfCordinates));
			float * resPS = resBuf.ptr<float>(i);// (float*)(resBuf->data.ptr + resBuf->step*(i));
			for (int jj = j, n = 0; jj < nResWid && n < 8; jj++, n++)
			{
				resPS[jj] = ScoreRes.m256_f32[n];
			}
		}
	}
	else if (nResLen > i && nResWid > j)
		resBuf.ptr<float>(i)[j] = 0;
}
void GeoMatch::_CalcCoefficient_AVX(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;

	float minScore(0.0f);

	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;

	int ImgWid = Sdx.cols;
	int ImgLen = Sdx.rows;

	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;

	float fRadian = fAngleStd / 180.0f * M_PI;
	float sinth = sin(fRadian);
	float costh = cos(fRadian);


	if (bConcurrency == false)
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 8)
				_CalcCoefficient_AVX_inline(Sdx, Sdy, matGradMag, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pEdgeY, pMag, pFlags, costh, sinth);
		}
	}
	else
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 8)
				_CalcCoefficient_AVX_inline(Sdx, Sdy, matGradMag, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pEdgeY, pMag, pFlags, costh, sinth);
		}
	}
}
void GeoMatch::_CalcCoefficientImg_AVX_inline(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pMag, BYTE * pFlag, cv::Point2i MinPos, cv::Point2i MaxPos, float stdev)
{
	int nMinX = j + MinPos.x + 0;
	int nMaxX = j + MaxPos.x + 1;
	int nMinY = i + MinPos.y + 0;
	int nMaxY = i + MaxPos.y + 1;

	if (nMinX < 0)
		nMinX = 0;
	if (nMinY < 0)
		nMinY = 0;

	if (nMaxX < 1 || nMaxY < 1)
	{
		resBuf.ptr<float>(i)[j] = 0;
		return;
	}

	if (nMaxX >= Integ.cols)
		nMaxX = Integ.cols - 1;
	if (nMaxY >= Integ.rows)
		nMaxY = Integ.rows - 1;
	int SzBox;// = (nMaxX-nMinX) * (nMaxY-nMinY);
	int * nMinIteg = Integ.ptr<int>(nMinY);
	int * nMaxIteg = Integ.ptr<int>(nMaxY);
	__m256 vMean[2];

	for (int imgSt = 0; imgSt < 16; imgSt++)
	{
		int offset = imgSt;

		int ntmpMinX = j + MinPos.x + 0 + offset;
		int ntmpMaxX = j + MaxPos.x + 1 + offset;

		if (ntmpMinX < 0)
			ntmpMinX = 0;
		if (ntmpMaxX >= Integ.cols)
			ntmpMaxX = Integ.cols - 1;

		SzBox = (ntmpMaxX - ntmpMinX) * (nMaxY - nMinY);

		vMean[imgSt / 8].m256_f32[imgSt % 8] = (float)(nMaxIteg[ntmpMaxX] - nMinIteg[ntmpMaxX] - nMaxIteg[ntmpMinX] + nMinIteg[ntmpMinX]) / (float)(SzBox);
	}

	int nRealNoOfCordinates(0);

	register __m256 partialStdev = _mm256_setzero_ps(), partialStdev2 = _mm256_setzero_ps();
	register __m256 partialSum = _mm256_setzero_ps(), partialSum2 = _mm256_setzero_ps();
	float fGeoEpsilon = GeoEpsilon;
	register __m256 vGeoEpsilon = _mm256_set1_ps(fGeoEpsilon);

	register __m128 partialStdev_128 = _mm_setzero_ps(), partialStdev2_128 = _mm_setzero_ps();
	register __m128 partialSum_128 = _mm_setzero_ps(), partialSum2_128 = _mm_setzero_ps();
	register __m128 vGeoEpsilon_128 = _mm_set_ps1(fGeoEpsilon);

	for (int m = 0; m < noOfCordinates; m++)
	{
		if ((pFlag[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			continue;

		nRealNoOfCordinates++;

		int curX = i + pCoordX[m];
		int curY = j + pCoordY[m];
		float iTl = pEdgeX[m];
		float iMag = pMag[m];

		if (curX < 0 || curY < 0 || curX > ImgLen - 1 || (curY + 16) >(ImgWid - 1))
		{
			if (curX >= 0 && curX <= (ImgLen - 1) && (curY + 16) >= 0 && curY <= (ImgWid - 1))
			{
				BYTE *_Sdl = Sdl.ptr<BYTE>(curX);

				int jj = 0;
				int jn = 0;
				int cntjj = 16;
				if (curY < 0)
				{
					jj = -curY;
					curY = 0;
				}
				else if ((curY + 15) > (ImgWid - 1))
					cntjj -= (curY + 15) - (ImgWid - 1);

				for (; jj < 8 && jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]) - vMean[0].m256_f32[jj];	// Æò±Õ °ªÀ» È°¿ëÇÑ´Ù
					partialSum.m256_f32[jj] = partialSum.m256_f32[jj] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev.m256_f32[jj] = partialStdev.m256_f32[jj] + (iSl*iSl);
				}

				for (; jj < cntjj; jj++, jn++)
				{
					float iSl = (float)(_Sdl[curY + jn]) - vMean[1].m256_f32[jj - 8];	// Æò±Õ °ªÀ» È°¿ëÇÑ´Ù
					partialSum2.m256_f32[jj - 8] = partialSum2.m256_f32[jj - 8] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
					partialStdev2.m256_f32[jj - 8] = partialStdev2.m256_f32[jj - 8] + (iSl*iSl);
				}

			}
			continue;
		}


		BYTE *_Sdl = Sdl.ptr<BYTE>(curX);

		register __m256i pSdl;
		register __m256 vSdI, vTI;
		__m128i pSdl128;

		partialSum = _mm256_add_ps(
			partialSum,

			_mm256_mul_ps(
				_mm256_add_ps(
					vSdI = _mm256_sub_ps(
						_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpacklo_epi8(pSdl128 = _mm_loadu_si128((__m128i*)&_Sdl[curY]), _mm_setzero_si128()))),
						vMean[0]), vGeoEpsilon),
				_mm256_add_ps(vTI = _mm256_set1_ps(iTl), vGeoEpsilon)
			)
		);
		partialStdev = _mm256_add_ps(partialStdev, _mm256_mul_ps(vSdI, vSdI));

		partialSum2 = _mm256_add_ps(
			partialSum2,
			_mm256_mul_ps(
				_mm256_add_ps(
					vSdI = _mm256_sub_ps(
						_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpackhi_epi8(pSdl128, _mm_setzero_si128()))),
						vMean[1]), vGeoEpsilon),
				_mm256_add_ps(vTI, vGeoEpsilon)
			)
		);
		partialStdev2 = _mm256_add_ps(partialStdev2, _mm256_mul_ps(vSdI, vSdI));

	}

	if (nRealNoOfCordinates)
	{
		float * resPS = resBuf.ptr<float>(i);
		if ((j + 8) <= ImgWid)
		{
			_mm256_storeu_ps(&resPS[j],
				_mm256_div_ps(_mm256_div_ps(partialSum, _mm256_mul_ps(_mm256_add_ps(_mm256_sqrt_ps(_mm256_div_ps(partialStdev, _mm256_set1_ps(nRealNoOfCordinates))), vGeoEpsilon),
					_mm256_add_ps(_mm256_set1_ps(stdev), vGeoEpsilon))), _mm256_set1_ps(nRealNoOfCordinates))
			);
		}
		else
		{
			__m256 ScoreRes = _mm256_div_ps(_mm256_div_ps(partialSum, _mm256_mul_ps(_mm256_add_ps(_mm256_sqrt_ps(_mm256_div_ps(partialStdev, _mm256_set1_ps(nRealNoOfCordinates))), vGeoEpsilon),
				_mm256_add_ps(_mm256_set1_ps(stdev), vGeoEpsilon))), _mm256_set1_ps(nRealNoOfCordinates));

			for (int jj = j, n = 0; jj < ImgWid && n < 8; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m256_f32[n];
			}
		}

		if ((j + 16) <= ImgWid)
		{
			_mm256_storeu_ps(&resPS[j + 8],
				_mm256_div_ps(_mm256_div_ps(partialSum2, _mm256_mul_ps(_mm256_add_ps(_mm256_sqrt_ps(_mm256_div_ps(partialStdev2, _mm256_set1_ps(nRealNoOfCordinates))), vGeoEpsilon),
					_mm256_add_ps(_mm256_set1_ps(stdev), vGeoEpsilon))), _mm256_set1_ps(nRealNoOfCordinates))
			);
		}
		else
		{
			__m256 ScoreRes = _mm256_div_ps(_mm256_div_ps(partialSum2, _mm256_mul_ps(_mm256_add_ps(_mm256_sqrt_ps(_mm256_div_ps(partialStdev2, _mm256_set1_ps(nRealNoOfCordinates))), vGeoEpsilon),
				_mm256_add_ps(_mm256_set1_ps(stdev), vGeoEpsilon))), _mm256_set1_ps(nRealNoOfCordinates));
			for (int jj = j + 8, n = 0; jj < ImgWid && n < 8; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m256_f32[n];
			}
		}
	}
	else
		resBuf.ptr<float>(i)[j] = 0;

}

void GeoMatch::_CalcCoefficientImg_AVX(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;
	cv::Point2i MinPos = ModelItem->_MinPos;
	cv::Point2i MaxPos = ModelItem->_MaxPos;

	int nModelWid = MaxPos.x - MinPos.x + 1;
	int nModelLen = MaxPos.y - MinPos.y + 1;
	int nModelArea = nModelWid * nModelLen;

	float stdev = ModelItem->_Stdev;

	if (bConcurrency == false)
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 16)
			{

				_CalcCoefficientImg_AVX_inline(Sdl, Integ, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pMag, pFlags, MinPos, MaxPos, stdev);
			}
		}
	}
	else
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 16)
				_CalcCoefficientImg_AVX_inline(Sdl, Integ, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pMag, pFlags, MinPos, MaxPos, stdev);
		}
	}
}

void GeoMatch::_CalcCoefficientVal_AVX_inline(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pMag, BYTE * pFlag, cv::Point2i MinPos, cv::Point2i MaxPos, float stdev)
{
	int nRealNoOfCordinates(0);

	register __m256 partialStdev = _mm256_setzero_ps(), partialStdev2 = _mm256_setzero_ps(), partialStdev3 = _mm256_setzero_ps(), partialStdev4 = _mm256_setzero_ps();//, partialStdev5 = _mm256_setzero_ps(), partialStdev6 = _mm256_setzero_ps(), partialStdev7 = _mm256_setzero_ps(), partialStdev8 = _mm256_setzero_ps();
	register __m256 partialSum = _mm256_setzero_ps(), partialSum2 = _mm256_setzero_ps(), partialSum3 = _mm256_setzero_ps(), partialSum4 = _mm256_setzero_ps();//, partialSum5 = _mm256_setzero_ps(), partialSum6 = _mm256_setzero_ps(), partialSum7 = _mm256_setzero_ps(), partialSum8 = _mm256_setzero_ps();
	float fGeoEpsilon = GeoEpsilon;
	register __m256 vGeoEpsilon = _mm256_set1_ps(fGeoEpsilon);

	for (int m = 0; m < noOfCordinates; m++)
	{
		if ((pFlag[m] & GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			continue;

		nRealNoOfCordinates++;

		int curX = i + pCoordX[m];
		int curY = j + pCoordY[m];
		float iTl = pEdgeX[m];
		float iMag = pMag[m];

		BYTE *_Sdl = Sdl.ptr<BYTE>(curX);

		register __m256i pSdl;
		register __m256 vSdI, vTI, vMean;

		if (curX < 0 || curY < 0 || curX > ImgLen - 1 || (curY + 16) >(ImgWid - 1))
		{
			if (curX >= 0 && curX <= (ImgLen - 1) && (curY + 16) >= 0 && curY <= (ImgWid - 1))
			{
				BYTE *_Sdl = Sdl.ptr<BYTE>(curX);

				int jj = 0;
				int jn = 0;
				if ((curY + 16) > (ImgWid - 1))
				{
					if ((curY + 16) >= 0)
					{
						int cntjj = 16;
						if (curY < 0)
						{
							jj = -curY;
							curY = 0;
						}
						else if ((curY + 15) > (ImgWid - 1))
							cntjj -= (curY + 15) - (ImgWid - 1);

						for (; jj < 8 && jj < cntjj; jj++, jn++)
						{
							float iSl = (float)(_Sdl[curY + jn]);
							partialSum.m256_f32[jj] = partialSum.m256_f32[jj] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
							partialStdev.m256_f32[jj] = partialStdev.m256_f32[jj] + (iSl*iSl);
						}

						for (; jj < cntjj; jj++, jn++)
						{
							float iSl = (float)(_Sdl[curY + jn]);
							partialSum2.m256_f32[jj - 8] = partialSum2.m256_f32[jj - 8] + ((iSl + GeoEpsilon) * (iTl + GeoEpsilon));
							partialStdev2.m256_f32[jj - 8] = partialStdev2.m256_f32[jj - 8] + (iSl*iSl);
						}
					}
				}
			}
			continue;
		}

		__m128i pSdl128;

		partialSum = _mm256_add_ps(
			partialSum,
			_mm256_mul_ps(
				_mm256_add_ps(
					vSdI = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpacklo_epi8(pSdl128 = _mm_loadu_si128((__m128i*)&_Sdl[curY]), _mm_setzero_si128())))
					, vGeoEpsilon),
				_mm256_add_ps(vTI = _mm256_set1_ps(iTl), vGeoEpsilon)
			)
		);
		partialStdev = _mm256_add_ps(partialStdev, _mm256_mul_ps(vSdI, vSdI));

		partialSum2 = _mm256_add_ps(
			partialSum2,
			_mm256_mul_ps(
				_mm256_add_ps(
					vSdI = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpackhi_epi8(pSdl128, _mm_setzero_si128())))
					, vGeoEpsilon),
				_mm256_add_ps(vTI, vGeoEpsilon)
			)
		);
		partialStdev2 = _mm256_add_ps(partialStdev2, _mm256_mul_ps(vSdI, vSdI));

	}

	if (nRealNoOfCordinates)
	{
		float * resPS = resBuf.ptr<float>(i);
		if ((j + 8) <= ImgWid)
		{
			_mm256_storeu_ps(&resPS[j],
				_mm256_div_ps(_mm256_div_ps(partialSum, _mm256_mul_ps(_mm256_add_ps(_mm256_sqrt_ps(_mm256_div_ps(partialStdev, _mm256_set1_ps(nRealNoOfCordinates))), vGeoEpsilon),
					_mm256_add_ps(_mm256_set1_ps(stdev), vGeoEpsilon))), _mm256_set1_ps(nRealNoOfCordinates))
			);
		}
		else
		{
			__m256 ScoreRes = _mm256_div_ps(_mm256_div_ps(partialSum, _mm256_mul_ps(_mm256_add_ps(_mm256_sqrt_ps(_mm256_div_ps(partialStdev, _mm256_set1_ps(nRealNoOfCordinates))), vGeoEpsilon),
				_mm256_add_ps(_mm256_set1_ps(stdev), vGeoEpsilon))), _mm256_set1_ps(nRealNoOfCordinates));

			for (int jj = j, n = 0; jj < ImgWid && n < 8; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m256_f32[n];
			}
		}

		if ((j + 16) <= ImgWid)
		{
			_mm256_storeu_ps(&resPS[j + 8],
				_mm256_div_ps(_mm256_div_ps(partialSum2, _mm256_mul_ps(_mm256_add_ps(_mm256_sqrt_ps(_mm256_div_ps(partialStdev2, _mm256_set1_ps(nRealNoOfCordinates))), vGeoEpsilon),
					_mm256_add_ps(_mm256_set1_ps(stdev), vGeoEpsilon))), _mm256_set1_ps(nRealNoOfCordinates))
			);
		}
		else
		{
			__m256 ScoreRes = _mm256_div_ps(_mm256_div_ps(partialSum2, _mm256_mul_ps(_mm256_add_ps(_mm256_sqrt_ps(_mm256_div_ps(partialStdev2, _mm256_set1_ps(nRealNoOfCordinates))), vGeoEpsilon),
				_mm256_add_ps(_mm256_set1_ps(stdev), vGeoEpsilon))), _mm256_set1_ps(nRealNoOfCordinates));
			for (int jj = j + 8, n = 0; jj < ImgWid && n < 8; jj++, n++)
			{
				float * resPS = resBuf.ptr<float>(i);
				resPS[jj] = ScoreRes.m256_f32[n];
			}
		}
	}
	else
		resBuf.ptr<float>(i)[j] = 0;

}

void GeoMatch::_CalcCoefficientVal_AVX(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency)
{
	if (ModelItem->isProcessed() == false)
		return;
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	BYTE * pFlags = ModelItem->_Flags;
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int StX = SStart.x;
	int StY = SStart.y;
	int Wid = StX + Ssize.width;
	int Len = StY + Ssize.height;
	cv::Point2i MinPos = ModelItem->_MinPos;
	cv::Point2i MaxPos = ModelItem->_MaxPos;

	int nModelWid = MaxPos.x - MinPos.x + 1;
	int nModelLen = MaxPos.y - MinPos.y + 1;
	int nModelArea = nModelWid * nModelLen;

	float stdev = ModelItem->_Stdev;

	if (bConcurrency == false)
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 16)
			{
				_CalcCoefficientVal_AVX_inline(Sdl, Integ, IntegSq, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pMag, pFlags, MinPos, MaxPos, stdev);
			}
		}
	}
	else
	{
		for (int i = StY; i < Len; i++)
		{
			for (int j = StX; j < Wid; j += 16)
				_CalcCoefficientVal_AVX_inline(Sdl, Integ, IntegSq, resBuf, i, j, ImgWid, ImgLen, noOfCordinates, pCoordX, pCoordY, pEdgeX, pMag, pFlags, MinPos, MaxPos, stdev);
		}
	}
}
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

void GeoMatch::imgMatch::copyTo(imgMatch& dst, cv::Rect re)
{
	dst.m_model = this->m_model(re);
	//	dst.m_Dev = this->m_Dev(cv::Rect(re.x,re.y,re.width,re.height));
	//	dst.m_Integ = this->m_Integ(cv::Rect(re.x,re.y,re.width,re.height));

	dst.m_dstdDevP = this->m_dstdDevP;
	dst.m_nAvgValue = this->m_nAvgValue;

	// 	int* npIntegStLn = dst.m_Integ.ptr<int>(0);
	// 	int* npIntegEdLn = dst.m_Integ.ptr<int>(dst.m_Integ.rows-1);
	// 	// 		npIntegStLn[0];//1
	// 	// 		npIntegStLn[dst.m_Integ.cols-1];//2
	// 	// 		npIntegEdLn[0];//3
	// 	// 		npIntegEdLn[dst.m_Integ.cols-1];//4
	// 	double dAll = npIntegEdLn[dst.m_Integ.cols-1]-npIntegEdLn[0]-npIntegStLn[dst.m_Integ.cols-1]+npIntegStLn[0];
	// 
	// 	dst.m_nAvgValue = dAll/((dst.m_Integ.rows-1)*(dst.m_Integ.cols-1));
	// 
	// 	double dPow = pow(dst.m_nAvgValue,2);
	// 
	// 	double* dpDevStLn = dst.m_Dev.ptr<double>(0);
	// 	double* dpDevEdLn = dst.m_Dev.ptr<double>(dst.m_Dev.rows-1);
	// 	// 		dpDevStLn[0];//1
	// 	// 		dpDevStLn[dst.m_Dev.cols-1];//2
	// 	// 		dpDevEdLn[0];//3
	// 	// 		dpDevEdLn[dst.m_Dev.cols-1];//4
	// 	double d = dpDevEdLn[dst.m_Integ.cols-1]-dpDevEdLn[0]-dpDevStLn[dst.m_Integ.cols-1]+dpDevStLn[0];
	// 	double sqAvg = d/((dst.m_Dev.rows-1)*(dst.m_Dev.cols-1));
	// 
	// 	double Div =  sqAvg-dPow;
	// 	double Dev = sqrt(Div);
	// 
	// 	dst.m_dstdDevP = Dev;
}

int GeoMatch::imgMatch::GetThresHold(UCHAR* src, int nSizeX, int nSizeY, int* thresh)
{
	float hist[256] = { 0, };

	GetHistogram(src, nSizeX, nSizeY, hist);

	int T = 0, Told = 0;

	float sum = 0.f;
	for (int i = 0; i < 256; i++)
	{
		sum += (i*hist[i]);
	}

	T = (int)sum;

	if (T <= 0)
	{
		*thresh = 0;
		return eMR_FAIL;
	}

	float a1, b1, u1, a2, b2, u2;

	do {
		Told = T;

		a1 = b1 = 0;

		for (int i = 0; i <= Told; i++)
		{
			a1 += (i*hist[i]);
			b1 += hist[i];
		}

		if (b1 == 0)
			b1 = 1.f;

		u1 = a1 / b1;

		a2 = b2 = 0;

		for (int i = Told + 1; i < 256; i++)
		{

			a2 += (i*hist[i]);
			b2 += hist[i];
		}

		if (b2 == 0)
			b2 = 1.f;

		u2 = a2 / b2;

		T = (int)((u1 + u2) / 2);
	} while (T != Told);


	*thresh = T;

	return eMR_SUCCESS;
}

//SHK 2013/11/04
int GeoMatch::imgMatch::GetHistogram(UCHAR* src, int sizeX, int sizeY, float* histo)
{
	int nTemp[256] = { 0, };

	// 	int nSizeX = sizeX % 4;
	// 	int widthstep = sizeX;
	// 	if (nSizeX!=0)
	// 	{
	// 		nSizeX = 4 -nSizeX;
	// 		widthstep = sizeX + nSizeX;
	// 	}


	for (int j = 0; j < sizeY; j++)
	{
		for (int i = 0; i < sizeX; i++)
		{
			int nData = src[sizeX*j + i];
			nTemp[nData]++;
		}
	}

	float area = (float)(sizeX*sizeY);

	for (int i = 0; i < 256; i++)
	{
		histo[i] = nTemp[i] / area;
	}

	return eMR_SUCCESS;
}

void GeoMatch::imgMatch::calcValue(cv::Mat model, int nThreshold)
{
	m_nAvgValue = nThreshold;
	cv::integral(model, m_Integ, m_Dev);
	if (nThreshold == 0)
	{
		int nAvg(0);
		GetThresHold(model.ptr(0), model.cols, model.rows, &nAvg);
		m_nAvgValue = (double)nAvg;
	}

	double srcf = model.ptr<uchar>((model.rows - 1), (model.cols - 1))[0];
	double d = m_Integ.ptr<int>((m_Integ.rows - 1), (m_Integ.cols - 1))[0];

	double dAvg = d / ((m_Integ.rows - 1)*(m_Integ.cols - 1));
	double dPow = pow(dAvg, 2);

	d = m_Dev.ptr<double>((m_Dev.rows - 1), (m_Dev.cols - 1))[0];
	double sqAvg = d / ((m_Dev.rows - 1)*(m_Dev.cols - 1));

	double Div = sqAvg - dPow;
	m_dstdDevP = sqrt(Div);
}

void GeoMatch::imgMatch::setImage(cv::Mat model)
{
	m_model = model.clone();
}

double GeoMatch::imgMatch::ImageMatching(imgMatch inputImg, int nPitch)
{
	float dScore(0);

	for (int i = 0; i < m_model.rows; i++)
	{
		uchar* modelptr = m_model.ptr<BYTE>(i);
		uchar* Imgptr = inputImg.m_model.ptr<BYTE>(i);
		for (int j = 0; j < m_model.cols; j++)
		{
			double modelValue = modelptr[j] - (float)m_nAvgValue == 0 ? EPSILON : modelptr[j] - (float)m_nAvgValue;
			double ImageValue = Imgptr[j] - (float)inputImg.m_nAvgValue == 0 ? EPSILON : Imgptr[j] - (float)inputImg.m_nAvgValue;
			dScore += (modelValue*ImageValue) / std::sqrt((double)(modelValue*modelValue)*(ImageValue*ImageValue));
		}
	}

	return dScore / (m_model.rows*m_model.cols);
}
double GeoMatch::imgMatch::ImageMatching_sse(imgMatch inputImg, int nPitch)
{
	return (this->*FunPtr_ImageMatching_sse)(inputImg, nPitch);
}
double GeoMatch::imgMatch::_ImageMatching_sse(imgMatch inputImg, int nPitch)
{
	float dScore(0);
	float fGeoEpsilon = GeoEpsilon;

	register __m128 vAvg = _mm_set_ps1(m_nAvgValue), vImgAvg = _mm_set_ps1(inputImg.m_nAvgValue), vGeoEpsilon = _mm_set_ps1(fGeoEpsilon);

	for (int i = 0; i < m_model.rows; i++)
	{
		uchar* modelptr = m_model.ptr<BYTE>(i);
		uchar* Imgptr = inputImg.m_model.ptr<BYTE>(i);

		register __m128 partialmodelValue = _mm_setzero_ps(), partialmodelValue2 = _mm_setzero_ps(), partialmodelValue3 = _mm_setzero_ps(), partialmodelValue4 = _mm_setzero_ps();
		register __m128 partialImageValue = _mm_setzero_ps(), partialImageValue2 = _mm_setzero_ps(), partialImageValue3 = _mm_setzero_ps(), partialImageValue4 = _mm_setzero_ps();
		register __m128 partialScore = _mm_setzero_ps();
		int j = 0;
		for (j = 0; (j + 16) < m_model.cols; j += 16)
		{
			register __m128i pModel, pImg;

			partialmodelValue = _mm_add_ps(_mm_sub_ps(
				_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(pModel = _mm_loadu_si128((__m128i *)&modelptr[j]), _mm_setzero_si128()), _mm_setzero_si128())),
				vAvg), vGeoEpsilon);

			partialmodelValue2 = _mm_add_ps(_mm_sub_ps(
				_mm_cvtepi32_ps(_mm_unpackhi_epi16(_mm_unpacklo_epi8(pModel = _mm_loadu_si128((__m128i *)&modelptr[j]), _mm_setzero_si128()), _mm_setzero_si128())),
				vAvg), vGeoEpsilon);

			partialmodelValue3 = _mm_add_ps(_mm_sub_ps(
				_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpackhi_epi8(pModel = _mm_loadu_si128((__m128i *)&modelptr[j]), _mm_setzero_si128()), _mm_setzero_si128())),
				vAvg), vGeoEpsilon);

			partialmodelValue4 = _mm_add_ps(_mm_sub_ps(
				_mm_cvtepi32_ps(_mm_unpackhi_epi16(_mm_unpackhi_epi8(pModel = _mm_loadu_si128((__m128i *)&modelptr[j]), _mm_setzero_si128()), _mm_setzero_si128())),
				vAvg), vGeoEpsilon);

			partialImageValue = _mm_add_ps(_mm_sub_ps(
				_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(pImg = _mm_loadu_si128((__m128i *)&Imgptr[j]), _mm_setzero_si128()), _mm_setzero_si128())),
				vImgAvg), vGeoEpsilon);

			partialImageValue2 = _mm_add_ps(_mm_sub_ps(
				_mm_cvtepi32_ps(_mm_unpackhi_epi16(_mm_unpacklo_epi8(pImg = _mm_loadu_si128((__m128i *)&Imgptr[j]), _mm_setzero_si128()), _mm_setzero_si128())),
				vImgAvg), vGeoEpsilon);

			partialImageValue3 = _mm_add_ps(_mm_sub_ps(
				_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpackhi_epi8(pImg = _mm_loadu_si128((__m128i *)&Imgptr[j]), _mm_setzero_si128()), _mm_setzero_si128())),
				vImgAvg), vGeoEpsilon);

			partialImageValue4 = _mm_add_ps(_mm_sub_ps(
				_mm_cvtepi32_ps(_mm_unpackhi_epi16(_mm_unpackhi_epi8(pImg = _mm_loadu_si128((__m128i *)&Imgptr[j]), _mm_setzero_si128()), _mm_setzero_si128())),
				vImgAvg), vGeoEpsilon);

			partialScore = _mm_add_ps(
				partialScore,
				_mm_add_ps(
					_mm_add_ps(
						_mm_div_ps(
							_mm_mul_ps(partialmodelValue, partialImageValue),
							_mm_sqrt_ps(_mm_mul_ps(_mm_mul_ps(partialmodelValue, partialmodelValue), _mm_mul_ps(partialImageValue, partialImageValue)))),
						_mm_div_ps(
							_mm_mul_ps(partialmodelValue2, partialImageValue2),
							_mm_sqrt_ps(_mm_mul_ps(_mm_mul_ps(partialmodelValue2, partialmodelValue2), _mm_mul_ps(partialImageValue2, partialImageValue2))))
					),
					_mm_add_ps(
						_mm_div_ps(
							_mm_mul_ps(partialmodelValue3, partialImageValue3),
							_mm_sqrt_ps(_mm_mul_ps(_mm_mul_ps(partialmodelValue3, partialmodelValue3), _mm_mul_ps(partialImageValue3, partialImageValue3)))),
						_mm_div_ps(
							_mm_mul_ps(partialmodelValue4, partialImageValue4),
							_mm_sqrt_ps(_mm_mul_ps(_mm_mul_ps(partialmodelValue4, partialmodelValue4), _mm_mul_ps(partialImageValue4, partialImageValue4))))
					)
				)
			);
		}

		dScore += partialScore.m128_f32[0] + partialScore.m128_f32[1] + partialScore.m128_f32[2] + partialScore.m128_f32[3];

		for (int jj = j; jj < m_model.cols; jj++)
		{
			double modelValue = modelptr[jj] - (float)m_nAvgValue == 0 ? EPSILON : modelptr[jj] - (float)m_nAvgValue;
			double ImageValue = Imgptr[jj] - (float)inputImg.m_nAvgValue == 0 ? EPSILON : Imgptr[jj] - (float)inputImg.m_nAvgValue;
			dScore += (modelValue*ImageValue) / std::sqrt((double)(modelValue*modelValue)*(ImageValue*ImageValue));
		}
	}

	return dScore / (m_model.rows*m_model.cols);
}
cv::Mat ImgProcessing::_ImgPreprocess(cv::Mat src, GeoModel::enmMatchAlgo MatchAlgo)
{
	cv::Mat fSrc;
	if (src.cols < 51 || src.rows < 51)
		return src;
	if (MatchAlgo == GeoModel::enmMatchAlgo::agIntaglio || MatchAlgo == GeoModel::enmMatchAlgo::agRelief)
		cv::GaussianBlur(src, fSrc, cv::Size(3, 3), 0);
	else
		cv::medianBlur(src, fSrc, 3);

	return fSrc;
}

void ImgProcessing::_ImgProcess(cv::Mat src, GeoModel &model)
{
	int use_algo_model = model.getMatchAlgo();
	// 영상 전처리

	//필터링
#if _DEBUG
	cv::imwrite("D:\\testimage\\workimage\\GeoMatchSrc.bmp", src);
#endif
	if (model.getModelFilterType() > 0)
	{
		cv::Mat ImageStandard, NomalImg, MedianMatOrg, MedianMat, thTemp;
		double thre_value1 = cv::threshold(src, thTemp, 0, 255, cv::THRESH_OTSU);
		cv::medianBlur(src, MedianMatOrg, 3);
		MedianMatOrg.convertTo(MedianMat, CV_16SC1);

		src.convertTo(ImageStandard, CV_16SC1);
		cv::Mat OMSub = MedianMat - ImageStandard;
		cv::Mat ImgADDM = ImageStandard + OMSub;
		cv::normalize(ImgADDM, NomalImg, 0, 255, cv::NORM_MINMAX);
		NomalImg.convertTo(src, CV_8UC1);
		double thre_value2 = cv::threshold(src, thTemp, 0, 255, cv::THRESH_OTSU);

		src = src - (thre_value2 - thre_value1);
	}

	cv::Mat fSrc = _ImgPreprocess(src, (GeoModel::enmMatchAlgo)model.getMatchAlgo());

	// 영상의 해상도 축소
	cv::Mat img = fSrc, half, pyr;
	int nPyrDownStep(0);
	do
	{
		pyr.release();
		_GetPyrDown(img, pyr);
		img = pyr;
		nPyrDownStep++;
		if (nPyrDownStep == 1)
			half = pyr;
	} while (nPyrDownStep < model._pyrDownStep);
	int nSobelSz(3);

	cv::Size OrgSsize = cv::Size(src.cols, src.rows);
	cv::Size HalfSsize = cv::Size(half.cols, half.rows);
	cv::Size ZoomSsize = cv::Size(pyr.cols, pyr.rows);
	// 	if(use_algo_model == GeoModel::enmMatchAlgo::agIntaglio)
	// 	{
	// 		cv::GaussianBlur(src, _ProcImg->_ProcBuf[buf].Sdl, cv::Size(7,7), 0);
	// 		cv::Laplacian(_ProcImg->_ProcBuf[buf].Sdl, _ProcImg->_ProcBuf[buf].Sdl, CV_8U, 5);
	// 		_CalcMagnitudeLap(_ProcImg->_ProcBuf[buf].Sdl, _ProcImg->_ProcBuf[buf].matGradMag, Ssize);
	// 	}
	if (use_algo_model == GeoModel::enmMatchAlgo::agIntaglio || use_algo_model == GeoModel::enmMatchAlgo::agRelief || use_algo_model == GeoModel::enmMatchAlgo::agImage)
	{
		IntaglioModelStep = model._pyrDownStep;
		fSrc.copyTo(_ProcBuf[GeoModel::enmDepth::spOrg].Sdl);
		ProcIntegral(fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Integral, _ProcBuf[GeoModel::enmDepth::spOrg].SqIntegral);

		half.copyTo(_ProcBuf[GeoModel::enmDepth::spHalf].Sdl);
		ProcIntegral(half, _ProcBuf[GeoModel::enmDepth::spHalf].Integral, _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral);
		if (pyr.rows != half.rows)
		{
			pyr.copyTo(_ProcBuf[GeoModel::enmDepth::spZoom].Sdl);
			ProcIntegral(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Integral, _ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral);
		}
		else
		{
			_ProcBuf[GeoModel::enmDepth::spZoom].Sdl = _ProcBuf[GeoModel::enmDepth::spHalf].Sdl;
			_ProcBuf[GeoModel::enmDepth::spZoom].Integral = _ProcBuf[GeoModel::enmDepth::spHalf].Integral;
			_ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral = _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral;

		}
	}
	else if (use_algo_model == GeoModel::enmMatchAlgo::agDefault || use_algo_model == GeoModel::enmMatchAlgo::agLowContrast || use_algo_model == GeoModel::enmMatchAlgo::agOriginal)
	{


		_ProcBuf[GeoModel::enmDepth::spOrg].Alloc(src);
		_ProcBuf[GeoModel::enmDepth::spHalf].Alloc(half);
		_ProcBuf[GeoModel::enmDepth::spZoom].Alloc(pyr);

		edgeModelStep = model._pyrDownStep;
		cv::Sobel(fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Sdx, CV_16S, 1, 0, nSobelSz);
		cv::Sobel(fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, CV_16S, 0, 1, nSobelSz);
#ifndef GeoNoUseSimd
		(this->*FunPtr_CalcMagnitude_sse)(_ProcBuf[GeoModel::enmDepth::spOrg].Sdx, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, _ProcBuf[GeoModel::enmDepth::spOrg].matGradMag, OrgSsize);
#else
		_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spOrg].Sdx, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, _ProcBuf[GeoModel::enmDepth::spOrg].matGradMag, OrgSsize);
#endif

		cv::Sobel(half, _ProcBuf[GeoModel::enmDepth::spHalf].Sdx, CV_16S, 1, 0, nSobelSz);
		cv::Sobel(half, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, CV_16S, 0, 1, nSobelSz);
#ifndef GeoNoUseSimd
		(this->*FunPtr_CalcMagnitude_sse)(_ProcBuf[GeoModel::enmDepth::spHalf].Sdx, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, _ProcBuf[GeoModel::enmDepth::spHalf].matGradMag, HalfSsize);
#else
		_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spHalf].Sdx, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, _ProcBuf[GeoModel::enmDepth::spHalf].matGradMag, HalfSsize);
#endif

		if (pyr.rows != half.rows)
		{
			cv::Sobel(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Sdx, CV_16S, 1, 0, nSobelSz);
			cv::Sobel(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, CV_16S, 0, 1, nSobelSz);
#ifndef GeoNoUseSimd
			(this->*FunPtr_CalcMagnitude_sse)(_ProcBuf[GeoModel::enmDepth::spZoom].Sdx, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, _ProcBuf[GeoModel::enmDepth::spZoom].matGradMag, ZoomSsize);
#else
			_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spZoom].Sdx, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, _ProcBuf[GeoModel::enmDepth::spZoom].matGradMag, ZoomSsize);
#endif
		}
		else
		{
			_ProcBuf[GeoModel::enmDepth::spZoom] = _ProcBuf[GeoModel::enmDepth::spHalf];
		}
	}
	else if (use_algo_model == GeoModel::enmMatchAlgo::agValue)
	{
		IntaglioModelStep = model._pyrDownStep;
		fSrc.copyTo(_ProcBuf[GeoModel::enmDepth::spOrg].Sdl);
		half.copyTo(_ProcBuf[GeoModel::enmDepth::spHalf].Sdl);
		pyr.copyTo(_ProcBuf[GeoModel::enmDepth::spZoom].Sdl);

		ProcIntegral(fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Integral, _ProcBuf[GeoModel::enmDepth::spOrg].SqIntegral);
		ProcIntegral(half, _ProcBuf[GeoModel::enmDepth::spHalf].Integral, _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral);
		if (half.rows != pyr.rows)
			ProcIntegral(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Integral, _ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral);
		else
		{
			_ProcBuf[GeoModel::enmDepth::spZoom].Integral = _ProcBuf[GeoModel::enmDepth::spHalf].Integral;
			_ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral = _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral;
		}
	}
	else
	{
		_ProcBuf[GeoModel::enmDepth::spOrg].Alloc(src);
		_ProcBuf[GeoModel::enmDepth::spHalf].Alloc(half);
		_ProcBuf[GeoModel::enmDepth::spZoom].Alloc(pyr);

		ProcIntegral(fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Integral, _ProcBuf[GeoModel::enmDepth::spOrg].SqIntegral);
		ProcIntegral(half, _ProcBuf[GeoModel::enmDepth::spHalf].Integral, _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral);
		if (half.rows != pyr.rows)
			ProcIntegral(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Integral, _ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral);
		else
		{
			_ProcBuf[GeoModel::enmDepth::spZoom].Integral = _ProcBuf[GeoModel::enmDepth::spHalf].Integral;
			_ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral = _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral;
		}
	}

	half.copyTo(*_half);
	pyr.copyTo(*_pyr);
	fSrc.copyTo(*_fSrc);
	src.copyTo(*_OrgImage);

	bIsImgProcessed = true;
}
void ImgProcessing::_ImgProcess(cv::Mat src, cv::Mat half, cv::Mat pyr, int modelStep, GeoModel::enmMatchAlgo MatchAlgo)
{
	//for Match POCR
	half.copyTo(*_half);
	pyr.copyTo(*_pyr);
	src.copyTo(*_fSrc);

	modelStep = modelStep;
	_ProcBuf[GeoModel::enmDepth::spOrg].Alloc(src);
	_ProcBuf[GeoModel::enmDepth::spHalf].Alloc(half);
	_ProcBuf[GeoModel::enmDepth::spZoom].Alloc(pyr);

	int nSobelSz(3);

	cv::Size OrgSsize = cv::Size(src.cols, src.rows);
	cv::Size HalfSsize = cv::Size(half.cols, half.rows);
	cv::Size ZoomSsize = cv::Size(pyr.cols, pyr.rows);
	// 	if(use_algo_model == GeoModel::enmMatchAlgo::agIntaglio)
	// 	{
	// 		cv::GaussianBlur(src, _ProcImg->_ProcBuf[buf].Sdl, cv::Size(7,7), 0);
	// 		cv::Laplacian(_ProcImg->_ProcBuf[buf].Sdl, _ProcImg->_ProcBuf[buf].Sdl, CV_8U, 5);
	// 		_CalcMagnitudeLap(_ProcImg->_ProcBuf[buf].Sdl, _ProcImg->_ProcBuf[buf].matGradMag, Ssize);
	// 	}
	if (MatchAlgo == GeoModel::enmMatchAlgo::agIntaglio || MatchAlgo == GeoModel::enmMatchAlgo::agRelief || MatchAlgo == GeoModel::enmMatchAlgo::agImage)
	{
		src.copyTo(_ProcBuf[GeoModel::enmDepth::spOrg].Sdl);
		ProcIntegral(src, _ProcBuf[GeoModel::enmDepth::spOrg].Integral, _ProcBuf[GeoModel::enmDepth::spOrg].SqIntegral);
		// 
		// 		half.copyTo(_ProcBuf[GeoModel::enmDepth::spHalf].Sdl);
		// 		ProcIntegral(half, _ProcBuf[GeoModel::enmDepth::spHalf].Integral, _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral);
		// 
		// 
		// 		if(pyr.rows != half.rows)
		// 		{
		// 			pyr.copyTo(_ProcBuf[GeoModel::enmDepth::spZoom].Sdl);
		// 			ProcIntegral(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Integral, _ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral);
		// 		}
		// 		else
		// 		{
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].Sdl = _ProcBuf[GeoModel::enmDepth::spHalf].Sdl;
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].Integral = _ProcBuf[GeoModel::enmDepth::spHalf].Integral;
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral = _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral;
		// 
		// 		}
	}
	else if (MatchAlgo == GeoModel::enmMatchAlgo::agDefault || MatchAlgo == GeoModel::enmMatchAlgo::agLowContrast || MatchAlgo == GeoModel::enmMatchAlgo::agOriginal)
	{
		cv::Sobel(src, _ProcBuf[GeoModel::enmDepth::spOrg].Sdx, CV_16S, 1, 0, nSobelSz);
		cv::Sobel(src, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, CV_16S, 0, 1, nSobelSz);
#ifndef GeoNoUseSimd
		(this->*FunPtr_CalcMagnitude_sse)(_ProcBuf[GeoModel::enmDepth::spOrg].Sdx, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, _ProcBuf[GeoModel::enmDepth::spOrg].matGradMag, OrgSsize);
#else
		_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spOrg].Sdx, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, _ProcBuf[GeoModel::enmDepth::spOrg].matGradMag, OrgSsize);
#endif

		cv::Sobel(half, _ProcBuf[GeoModel::enmDepth::spHalf].Sdx, CV_16S, 1, 0, nSobelSz);
		cv::Sobel(half, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, CV_16S, 0, 1, nSobelSz);
#ifndef GeoNoUseSimd
		(this->*FunPtr_CalcMagnitude_sse)(_ProcBuf[GeoModel::enmDepth::spHalf].Sdx, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, _ProcBuf[GeoModel::enmDepth::spHalf].matGradMag, HalfSsize);
#else
		_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spHalf].Sdx, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, _ProcBuf[GeoModel::enmDepth::spHalf].matGradMag, HalfSsize);
#endif

		if (pyr.rows != half.rows)
		{
			cv::Sobel(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Sdx, CV_16S, 1, 0, nSobelSz);
			cv::Sobel(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, CV_16S, 0, 1, nSobelSz);
#ifndef GeoNoUseSimd
			(this->*FunPtr_CalcMagnitude_sse)(_ProcBuf[GeoModel::enmDepth::spZoom].Sdx, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, _ProcBuf[GeoModel::enmDepth::spZoom].matGradMag, ZoomSsize);
#else
			_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spZoom].Sdx, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, _ProcBuf[GeoModel::enmDepth::spZoom].matGradMag, ZoomSsize);
#endif
		}
		else
		{
			_ProcBuf[GeoModel::enmDepth::spZoom] = _ProcBuf[GeoModel::enmDepth::spHalf];
		}
	}
	else
	{
		ProcIntegral(src, _ProcBuf[GeoModel::enmDepth::spOrg].Integral, _ProcBuf[GeoModel::enmDepth::spOrg].SqIntegral);
		// 		ProcIntegral(half, _ProcBuf[GeoModel::enmDepth::spHalf].Integral, _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral);
		// 
		// 		if(half.rows != pyr.rows)
		// 			ProcIntegral(pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Integral, _ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral);
		// 		else
		// 		{
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].Integral = _ProcBuf[GeoModel::enmDepth::spHalf].Integral;
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral = _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral;
		// 		}
	}

	//	src.copyTo(*_OrgImage);
	bIsImgProcessed = true;
}

void ImgProcessing::ProcIntegral(cv::Mat & src, cv::Mat & Integ, cv::Mat & SqInteg)
{
	cv::integral(src, Integ, SqInteg);
}
void ImgProcessing::_CalcMagnitude_sse(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Size Ssize)
{
	for (int i = 0; i < Ssize.height; i++)
	{
		__m128 onePS = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);

		float magTest[8];
		float magTest2[8];
		float * mag = &matGradMag[i*Ssize.width];
		int j = 0;
		for (j = 0; (j + 8) < Ssize.width; j += 8)
		{
			short *_Sdx = Sdx.ptr<short>(i);// (short *)(Sdx->data.ptr + Sdx->step*(i));
			short *_Sdy = Sdy.ptr<short>(i);// (short *)(Sdy->data.ptr + Sdy->step*(i));

			register __m128i pSdx, pSdy;
			register __m128 psSdx, psSdy, Div, PowX, PowY;

			_mm_storeu_ps(&mag[j],
				_mm_andnot_ps(
					_mm_cmpeq_ps(Div, _mm_setzero_ps()),
					_mm_div_ps(
						onePS,
						Div = _mm_sqrt_ps(
							_mm_add_ps(
								_mm_mul_ps(psSdx, psSdx = _mm_cvtepi32_ps(_mm_unpacklo_epi16(pSdx, _mm_cmplt_epi16(pSdx = _mm_loadu_si128((__m128i *)&_Sdx[j]), _mm_setzero_si128())))),
								_mm_mul_ps(psSdy, psSdy = _mm_cvtepi32_ps(_mm_unpacklo_epi16(pSdy, _mm_cmplt_epi16(pSdy = _mm_loadu_si128((__m128i *)&_Sdy[j]), _mm_setzero_si128()))))
							)
						)
					)
				)
			);

			_mm_storeu_ps(&mag[j + 4],
				_mm_andnot_ps(
					_mm_cmpeq_ps(Div, _mm_setzero_ps()),
					_mm_div_ps(
						onePS,
						Div = _mm_sqrt_ps(
							_mm_add_ps(
								_mm_mul_ps(psSdx, psSdx = _mm_cvtepi32_ps(_mm_unpackhi_epi16(pSdx, _mm_cmplt_epi16(pSdx, _mm_setzero_si128())))),
								_mm_mul_ps(psSdy, psSdy = _mm_cvtepi32_ps(_mm_unpackhi_epi16(pSdy, _mm_cmplt_epi16(pSdy, _mm_setzero_si128()))))
							)
						)
					)
				)
			);
		}

		for (int jj = j; jj < Ssize.width; jj++)
		{
			short*_Sdx = Sdx.ptr<short>(i);// (short*)(Sdx->data.ptr + Sdx->step*(i));
			short*_Sdy = Sdy.ptr<short>(i);// (short*)(Sdy->data.ptr + Sdy->step*(i));
			float iSx = _Sdx[jj];
			float iSy = _Sdy[jj];
			float gradMag = sqrt((iSx*iSx) + (iSy*iSy));

			if (gradMag != 0)
				mag[jj] = 1 / gradMag;
			else
				mag[jj] = 0;
		}
	}
}
void ImgProcessing::_CalcMagnitude(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Size Ssize)
{
	for (int i = 0; i < Ssize.height; i++)
	{
		float * mag = &matGradMag[i*Ssize.width];

		for (int j = 0; j < Ssize.width; j++)
		{
			short *_Sdx = Sdx.ptr<short>(i);// (short *)(Sdx->data.ptr + Sdx->step*(i));
			short *_Sdy = Sdy.ptr<short>(i);// (short *)(Sdy->data.ptr + Sdy->step*(i));
			float iSx = _Sdx[j];
			float iSy = _Sdy[j];
			float gradMag = sqrt((iSx*iSx) + (iSy*iSy));

			if (gradMag != 0)
				mag[j] = 1 / gradMag;
			else
				mag[j] = 0;
		}
	}
}
void ImgProcessing::_CalcMagnitudeLap(cv::Mat & Sdl, float * matGradMag, cv::Size Ssize)
{
	for (int i = 0; i < Ssize.height; i++)
	{
		float * mag = &matGradMag[i*Ssize.width];
		for (int j = 0; j < Ssize.width; j++)
		{
			BYTE *_Sdl = Sdl.ptr<BYTE>(i);
			float iSl = _Sdl[j];
			float gradMag = sqrt((iSl*iSl));
			if (gradMag != 0)
				mag[j] = 1 / gradMag;
			else
				mag[j] = 0;
		}
	}
}
void ImgProcessing::_GetPyrDown(cv::Mat src, cv::Mat & des)
{
	double scaleVal = 0.5;
	//des = cv::Mat(src.rows*scaleVal, src.cols*scaleVal, CV_8UC1);
	des = cv::Mat(src.rows*scaleVal, src.cols*scaleVal, CV_MAKETYPE(CV_8U, src.channels()));
	//	cv::pyrDown(src, des, cv::Size(src.cols*scaleVal, src.rows*scaleVal));

	cv::resize(src, des, cv::Size(src.cols*scaleVal, src.rows*scaleVal));
}
void ImgProcessing::_CalcMagnitude_AVX(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Size Ssize)
{
	for (int i = 0; i < Ssize.height; i++)
	{
		__m256 onePS = _mm256_set_ps(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

		float * mag = &matGradMag[i*Ssize.width];
		int j = 0;
		for (j = 0; (j + 8) < Ssize.width; j += 8)
		{
			short *_Sdx = Sdx.ptr<short>(i);// (short *)(Sdx->data.ptr + Sdx->step*(i));
			short *_Sdy = Sdy.ptr<short>(i);// (short *)(Sdy->data.ptr + Sdy->step*(i));

			register __m256 psSdx, psSdy, Div;
			psSdx = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i*)&_Sdx[j])));
			psSdy = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i*)&_Sdy[j])));

			_mm256_storeu_ps(&mag[j],
				_mm256_andnot_ps(
					_mm256_cmp_ps(Div, _mm256_setzero_ps(), _CMP_EQ_OS),
					_mm256_div_ps(
						onePS,
						Div = _mm256_sqrt_ps(
							_mm256_add_ps(
								_mm256_mul_ps(psSdx, psSdx),
								_mm256_mul_ps(psSdy, psSdy)
							)
						)
					)
				)
			);
		}

		for (int jj = j; jj < Ssize.width; jj++)
		{
			short*_Sdx = Sdx.ptr<short>(i);// (short*)(Sdx->data.ptr + Sdx->step*(i));
			short*_Sdy = Sdy.ptr<short>(i);// (short*)(Sdy->data.ptr + Sdy->step*(i));
			float iSx = _Sdx[jj];
			float iSy = _Sdy[jj];
			float gradMag = sqrt((iSx*iSx) + (iSy*iSy));

			if (gradMag != 0)
				mag[jj] = 1 / gradMag;
			else
				mag[jj] = 0;
		}
	}
}
double GeoMatch::imgMatch::_ImageMatching_AVX(imgMatch inputImg, int nPitch)
{
	float dScore(0);
	float fGeoEpsilon = GeoEpsilon;

	register __m256 vAvg = _mm256_set1_ps(m_nAvgValue), vImgAvg = _mm256_set1_ps(inputImg.m_nAvgValue), vGeoEpsilon = _mm256_set1_ps(fGeoEpsilon);

	for (int i = 0; i < m_model.rows; i++)
	{
		uchar* modelptr = m_model.ptr<BYTE>(i);
		uchar* Imgptr = inputImg.m_model.ptr<BYTE>(i);

		register __m256 partialmodelValue = _mm256_setzero_ps(), partialmodelValue2 = _mm256_setzero_ps();
		register __m256 partialImageValue = _mm256_setzero_ps(), partialImageValue2 = _mm256_setzero_ps();
		register __m256 partialScore = _mm256_setzero_ps();
		int j = 0;
		for (j = 0; (j + 16) < m_model.cols; j += 16)
		{
			__m128i pModel128, pImg128;
			// 			partialmodelValue = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpacklo_epi8(pModel128 = _mm_loadu_si128((__m128i*)&modelptr[j]), _mm_setzero_si128())));
			// 			partialmodelValue2 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpackhi_epi8(pModel128, _mm_setzero_si128())));

			partialmodelValue = _mm256_add_ps(_mm256_sub_ps(
				_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpacklo_epi8(pModel128 = _mm_loadu_si128((__m128i*)&modelptr[j]), _mm_setzero_si128()))),
				vImgAvg), vGeoEpsilon);
			partialmodelValue2 = _mm256_add_ps(_mm256_sub_ps(
				_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpackhi_epi8(pModel128, _mm_setzero_si128()))),
				vImgAvg), vGeoEpsilon);


			partialImageValue = _mm256_add_ps(_mm256_sub_ps(
				_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpacklo_epi8(pImg128 = _mm_loadu_si128((__m128i*)&Imgptr[j]), _mm_setzero_si128()))),
				vImgAvg), vGeoEpsilon);

			partialImageValue2 = _mm256_add_ps(_mm256_sub_ps(
				_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_unpackhi_epi8(pImg128, _mm_setzero_si128()))),
				vImgAvg), vGeoEpsilon);

			partialScore = _mm256_add_ps(
				partialScore,
				_mm256_add_ps(
					_mm256_div_ps(
						_mm256_mul_ps(partialmodelValue, partialImageValue),
						_mm256_sqrt_ps(_mm256_mul_ps(_mm256_mul_ps(partialmodelValue, partialmodelValue), _mm256_mul_ps(partialImageValue, partialImageValue)))),
					_mm256_div_ps(
						_mm256_mul_ps(partialmodelValue2, partialImageValue2),
						_mm256_sqrt_ps(_mm256_mul_ps(_mm256_mul_ps(partialmodelValue2, partialmodelValue2), _mm256_mul_ps(partialImageValue2, partialImageValue2))))
				)
			);
		}

		dScore += partialScore.m256_f32[0] + partialScore.m256_f32[1] + partialScore.m256_f32[2] + partialScore.m256_f32[3] + partialScore.m256_f32[4] + partialScore.m256_f32[5] + partialScore.m256_f32[6] + partialScore.m256_f32[7];

		for (int jj = j; jj < m_model.cols; jj++)
		{
			double modelValue = modelptr[jj] - (float)m_nAvgValue == 0 ? EPSILON : modelptr[jj] - (float)m_nAvgValue;
			double ImageValue = Imgptr[jj] - (float)inputImg.m_nAvgValue == 0 ? EPSILON : Imgptr[jj] - (float)inputImg.m_nAvgValue;
			dScore += (modelValue*ImageValue) / std::sqrt((double)(modelValue*modelValue)*(ImageValue*ImageValue));
		}
	}

	return dScore / (m_model.rows*m_model.cols);
}