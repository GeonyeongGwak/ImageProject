#pragma once
#include "StdAfx.h"
#include "GeoMatch_OCV.h"
#include <opencv2/opencv.hpp>
#include <set>
#include <fstream>
#include <functional>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif


	GeoModel_POCR::GeoModel_POCR(void)
	{
		//g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	GeoModel_POCR::~GeoModel_POCR(void)
	{
		//g_pMManager->pem_delete_check(this);
	}


	// = = = = = = = = = = = = = = = = = = = = = = = =


	GeoResult_POCR::GeoResult_POCR()
	{
		//g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	GeoResult_POCR::GeoResult_POCR(int cnt)
	{
		Alloc(cnt);

		//g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	GeoResult_POCR::~GeoResult_POCR()
	{
		//g_pMManager->pem_delete_check(this);
	}

	void GeoResult_POCR::Alloc(int cnt)
	{
		if (cnt < 0)
			return;
		GeoResult::Alloc(cnt);

		_Score.resize(cnt);
		_ResEdge.resize(cnt);
		_ResDiff.resize(cnt);
		_SelEngine.resize(cnt);
	}
	void GeoResult_POCR::Free()
	{
		GeoResult::Free();

		_Score.clear();
		_ResEdge.clear();
		_ResDiff.clear();
	}
	GeoModel_POCR::Item_POCR::Item_POCR(GeoModel * parent) : GeoModel::Item(parent)
	{

	}



	std::shared_ptr<GeoModel::Item> GeoModel_POCR::CreateItem()
	{
		return std::shared_ptr<Item>(new GeoModel_POCR::Item_POCR(this));
	}

	cv::Mat GeoModel_POCR::Item_POCR::algo_3(cv::Mat srcP, cv::Scalar mean, cv::Scalar stddev, int width, int height)
	{
		cv::Mat src_W;
		int m_median_size(0);

		srcP.copyTo(src_W);
		int minSize = src_W.rows;
		if (src_W.rows > src_W.cols)
			minSize = src_W.cols;

		cv::Mat srcP_CannyDst;

		if (minSize > 11)
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
			double thre_value = cv::threshold(src_W, src_W, 0, 255, cv::THRESH_OTSU);
			if (thre_value == 0)
			{
				srcP.copyTo(src_W);
				thre_value = cv::threshold(src_W, src_W, 0, 255, cv::THRESH_OTSU);
			}
			cv::threshold(srcP, srcP, thre_value, 255, cv::THRESH_BINARY);


			cv::Canny(srcP, srcP_CannyDst, 0, 0, 3, false);

			//cv::Canny(srcP,srcP,0,0,3,false);
			//	srcP = noise_remove(srcP,srcP.rows, srcP.cols);
		}
		else
		{
			cv::threshold(srcP, srcP, 0, 255, cv::THRESH_OTSU);

			cv::Canny(srcP, srcP_CannyDst, 0, 0, 3, false);
			//cv::Canny(srcP,srcP,0,0,3,false);
		}

		return srcP_CannyDst;
	}
	bool GeoModel_POCR::Item_POCR::Calculate(cv::Mat & src, cv::Mat mask, int nPreProvLv, float maxContrast, float minContrast, bool _bEdgeEnhance, int use_algo)
	{
		if (CV_MAT_TYPE(src.type()) != CV_8UC1)
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
		int * orients = (int *)_aligned_malloc(Ssize.height*Ssize.width*sizeof(int), 16);
		float * magMat = (float *)_aligned_malloc(Ssize.height*Ssize.width*sizeof(float), 16);
// 		int * orients = (int *)g_pMManager->pem_aligned_malloc(Ssize.height*Ssize.width * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
// 		float * magMat = (float *)g_pMManager->pem_aligned_malloc(Ssize.height*Ssize.width * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);

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
		if (dst.empty() == false)
			cv::bitwise_and(dst, mask, dst);

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

							_Flags[_noOfCordinates] = 0;
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

		_aligned_free(orients);
		_aligned_free(magMat);
// 		g_pMManager->pem_aligned_free(orients);
// 		g_pMManager->pem_aligned_free(magMat);

		_modelDefined = true;

		return _modelDefined;
	}

	// = = = = = = = = = = = = = = = = = = = = = = = =


	GeoMatch_POCR::GeoMatch_POCR(void)
	{
		//g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	this->FunPtr_FindModel_Char_Angle[0] = &GeoMatch_POCR::FindModel_Char_Angle;
	this->FunPtr_FindModel_Char_Angle[1] = &GeoMatch_POCR::FindModel_Char_Angle_AllAngle;
	}
	GeoMatch_POCR::~GeoMatch_POCR(void)
	{
		//g_pMManager->pem_delete_check(this);
	}

	// void GeoMatch_POCR::ProcIntegral(cv::Mat & src, cv::Mat & Integ, cv::Mat & SqInteg)
	// {
	// 	cv::integral(src, Integ, SqInteg);
	// }

	void GeoMatch_POCR::_GetResult(GeoResult_POCR * resData, int nModelCount, int nResCnt, GeoResult_POCR & result)
	{
		std::vector<GeoSortItem> CandidateRes;
		CandidateRes.reserve(nModelCount*nResCnt);

		for (int i = 0; i < nModelCount; i++)
		{
			for (int j = 0; j < nResCnt; j++)
			{
				if (resData[i]._Equality[j] <= 0.0)
					continue;//resData[i]._Equality[j] = 0;

				bool bFound(false);
				for (auto iter = CandidateRes.begin(); iter != CandidateRes.end(); iter++)
				{
					if (abs(iter->_X - resData[i]._Center_x[j]) < 5.0f && abs(iter->_Y - resData[i]._Center_y[j]) < 5.0f)
					{
						if (iter->_Score < resData[i]._Equality[j])
						{
							iter->_Score = resData[i]._Equality[j];
							iter->_Id = resData[i]._AngleStepId[j];
							iter->_Angle = resData[i]._Angle[j];
							iter->_X = resData[i]._Center_x[j];
							iter->_Y = resData[i]._Center_y[j];
							iter->_I = i;
							iter->_J = j;
						}

						bFound = true;
						break;
					}
				}

				if (bFound == false)
				{
					CandidateRes.push_back(GeoSortItem(resData[i]._Equality[j],
						resData[i]._AngleStepId[j],
						resData[i]._Angle[j],
						resData[i]._Center_x[j],
						resData[i]._Center_y[j],
						i, j));
				}
			}
		}

		std::set<GeoSortItem, std::greater<GeoSortItem>> SortedRes;
		for (auto iter = CandidateRes.begin(); iter != CandidateRes.end(); iter++)
			SortedRes.insert(*iter);


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
	}

	void GeoMatch_POCR::FindModel_Char(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoResult_POCR & result)
	{
		if (param == nullptr)
			return;

		if (_CheckFont(ch) == false)
			return;

		_EngineParam = param;
		_Ch = ch;
		_DestImg = *_ProcImg->_OrgImage;

		FindModel(model, result, false);
		_CalcDivScore(model, result);

		_EngineParam = nullptr;
	}
	void GeoMatch_POCR::FindModel_Char(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, cv::Point2f OfsChar, GeoResult_POCR & result)
	{
		if (param == nullptr)
			return;

		if (_CheckFont(ch) == false)
			return;

		_EngineParam = param;
		_Ch = ch;
		_DestImg = *_ProcImg->_OrgImage;

		// 문자열 이미지로 이미 찾은 위치 주변을 탐색한다.
		float rcx = OfsChar.x;
		float rcy = OfsChar.y;

		int nzeroIdx = (int)(model._Models[GeoModel::enmDepth::spOrg].size() / 2);
		GeoModel::typItemPtr FontItem = model._Models[GeoModel::enmDepth::spOrg][nzeroIdx];
		if (FontItem->_modelWidth == 0)
		{
			model.RePreprocess();
			FontItem = model._Models[GeoModel::enmDepth::spOrg][nzeroIdx];
		}
		//	FindModel(src, model, result);

		int sx = FontItem->_modelWidth / 2;
		int sy = sx;//ItemPtr->_modelHeight / 80 + 1;
		cv::Mat fSrc = *_ProcImg->_OrgImage;

		cv::Point stRoi = cv::Point(rcx - sx, rcy - sy);
		cv::Size szRoi = cv::Size(sx * 2 + 1, sy * 2 + 1);
		if ((stRoi.x) < 0)
		{
			szRoi.width = sx;
			stRoi.x = 0;
			if (szRoi.width < 0)
				szRoi.width = 1;
		}
		if ((stRoi.y) < 0)
		{
			szRoi.height = sy;
			stRoi.y = 0;
			if (szRoi.height < 0)
				szRoi.height = 1;
		}
		if ((stRoi.x + szRoi.width) >= fSrc.cols)
			szRoi.width -= (stRoi.x + szRoi.width) - fSrc.cols;
		if ((stRoi.y + szRoi.height) >= fSrc.rows)
			szRoi.height -= (stRoi.y + szRoi.height) - fSrc.rows;


		_FindModel(FontItem, nzeroIdx, stRoi, szRoi, model.getAngle(), 0.1, 0.9, result, false, model.getMatchAlgo());
		_CalcDivScore(model, result);

		_EngineParam = nullptr;


		//#ifdef ImageSave
		//	cv::Mat drawImg = cv::Mat::zeros(src.rows, src.cols, src.type());
		////	src.copyTo(drawImg);
		//	_DrawContours(drawImg, model, result, cv::Scalar(255), 1);
		//
		//	cv::imwrite(std::string("d:\\test_angle_find.bmp"), drawImg);
		//#endif	
	}

	void GeoMatch_POCR::FindModel_Char_Angle(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoModel * StringModel, GeoResult * StringResult, cv::Point2f OfsChar, GeoResult_POCR & result)
	{
		if (param == nullptr)
			return;

		if (_CheckFont(ch) == false)
			return;

		_EngineParam = param;
		_Ch = ch;
		_DestImg = *_ProcImg->_OrgImage;


		cv::Mat img = StringModel->Image();
		GeoModel::typItemPtr ItemPtr = StringModel->_Models[GeoModel::enmDepth::spOrg][StringResult->_AngleStepId[0]];

		// 	int sx = ItemPtr->_modelWidth / 80 + 1;
		// 	int sy = ItemPtr->_modelHeight / 80 + 1;

		double AngRad = ItemPtr->_Angle / 180.0 * PI;
		double radSin = sin(AngRad);
		double radCos = cos(AngRad);

		float rcx_ofs = OfsChar.x * radCos + OfsChar.y * radSin;
		float rcy_ofs = -OfsChar.x * radSin + OfsChar.y * radCos;

		// 문자열 이미지로 이미 찾은 위치 주변을 탐색한다.
		float rcx = StringResult->_Center_y[0] + rcx_ofs;
		float rcy = StringResult->_Center_x[0] + rcy_ofs;

		GeoModel::typItemPtr FontItem = model._Models[GeoModel::enmDepth::spOrg][StringResult->_AngleStepId[0]];
	if (FontItem->_modelWidth == 0)
	{
		model.RePreprocess();
		FontItem = model._Models[GeoModel::enmDepth::spOrg][StringResult->_AngleStepId[0]];
	}

	int sx = FontItem->_modelWidth/4;
	int sy = sx;//ItemPtr->_modelHeight / 80 + 1;
	cv::Mat fSrc = *_ProcImg->_OrgImage;

	cv::Point stRoi = cv::Point(rcx-sx, rcy-sy);
	cv::Size szRoi = cv::Size(sx*2+1,sy*2+1);
	if((stRoi.x) < 0)
	{
		szRoi.width = sx;
		stRoi.x = 0;
		if (szRoi.width < 0)
			szRoi.width = 1;
	}
	if((stRoi.y) < 0)
	{
		szRoi.height = sy;
		stRoi.y = 0;
		if (szRoi.height < 0)
			szRoi.height = 1;
	}
	if((stRoi.x + szRoi.width) >= fSrc.cols)
		szRoi.width -= (stRoi.x + szRoi.width) - fSrc.cols;
	if((stRoi.y + szRoi.height) >= fSrc.rows)
		szRoi.height -= (stRoi.y + szRoi.height) - fSrc.rows;


	_FindModel(FontItem, StringResult->_AngleStepId[0], stRoi, szRoi, model.getAngle(), 0.1, 0.9, result, false, model.getMatchAlgo());
	_CalcDivScore(model, result);

	_EngineParam = nullptr;
}
void GeoMatch_POCR::FindModel_Char_Angle_AllAngle(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoModel * StringModel, GeoResult * StringResult, cv::Point2f OfsChar, GeoResult_POCR & result)
{
	if (param == nullptr)
		return;

	if (_CheckFont(ch) == false)
		return;

	_EngineParam = param;
	_Ch = ch;
	_DestImg = *_ProcImg->_OrgImage;

	cv::Mat img = StringModel->Image();
	for each (pair<int, GeoModel::typItemPtr> itempair in model._Models[GeoModel::enmDepth::spOrg])
	{
		GeoResult_POCR Tempresult(1);
		//std::shared_ptr<GeoResult_POCR> Tempresult = std::shared_ptr<GeoResult_POCR>(new GeoResult_POCR(1));
		GeoModel::typItemPtr FontItem = itempair.second;
		// 	int sx = ItemPtr->_modelWidth / 80 + 1;
		// 	int sy = ItemPtr->_modelHeight / 80 + 1;

		double AngRad = FontItem->_Angle / 180.0 * M_PI;
		double radSin = sin(AngRad);
		double radCos = cos(AngRad);

		float rcx_ofs = OfsChar.x * radCos + OfsChar.y * radSin;
		float rcy_ofs = -OfsChar.x * radSin + OfsChar.y * radCos;

		// 문자열 이미지로 이미 찾은 위치 주변을 탐색한다.
		float rcx = StringResult->_Center_y[0] + rcx_ofs;
		float rcy = StringResult->_Center_x[0] + rcy_ofs;
		if (FontItem->_modelWidth == 0)
		{
			model.RePreprocess();
			FontItem = model._Models[GeoModel::enmDepth::spOrg][StringResult->_AngleStepId[0]];
		}
		//	FindModel(src, model, result);

		int sx = FontItem->_modelWidth / 4;
		int sy = sx;//ItemPtr->_modelHeight / 80 + 1;
		cv::Mat fSrc = *_ProcImg->_OrgImage;

		cv::Point stRoi = cv::Point(rcx - sx, rcy - sy);
		cv::Size szRoi = cv::Size(sx * 2 + 1, sy * 2 + 1);
		if ((stRoi.x) < 0)
		{
			szRoi.width = sx;
			stRoi.x = 0;
			if (szRoi.width < 0)
				szRoi.width = 1;
		}
		if ((stRoi.y) < 0)
		{
			szRoi.height = sy;
			stRoi.y = 0;
			if (szRoi.height < 0)
				szRoi.height = 1;
		}
		if ((stRoi.x + szRoi.width) >= fSrc.cols)
			szRoi.width -= (stRoi.x + szRoi.width) - fSrc.cols;
		if ((stRoi.y + szRoi.height) >= fSrc.rows)
			szRoi.height -= (stRoi.y + szRoi.height) - fSrc.rows;


		_FindModel(FontItem, StringResult->_AngleStepId[0], stRoi, szRoi, model.getAngle(), 0.1, 0.9, Tempresult, false, model.getMatchAlgo());
		_CalcDivScore(model, Tempresult);

		if (Tempresult._Equality[0] > result._Equality[0])
		{
			result = Tempresult;
		}
	}
	_EngineParam = nullptr;


	//#ifdef ImageSave
	//	cv::Mat drawImg = cv::Mat::zeros(src.rows, src.cols, src.type());
	////	src.copyTo(drawImg);
	//	_DrawContours(drawImg, model, result, cv::Scalar(255), 1);
	//
	//	cv::imwrite(std::string("d:\\test_angle_find.bmp"), drawImg);
	//#endif	
}
void GeoMatch_POCR::FindModel_Char_AngleFunc(int nCharAngleType, TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoModel * StringModel, GeoResult * StringResult, cv::Point2f OfsChar, GeoResult_POCR & result)
{
	if (nCharAngleType > 1)nCharAngleType = 0;
	(this->*FunPtr_FindModel_Char_Angle[nCharAngleType])(ch, model, param, StringModel, StringResult, OfsChar, result);
}
	void GeoMatch_POCR::CalcDivScore(GeoModel & model, GeoResult_POCR & Result, int nThreshold)
	{
		_DestImg = *_ProcImg->_OrgImage;
		//_ImgProcess(src, 0, GeoModel::enmDepth::spOrg, model.getMatchAlgo());
		if (model.getMatchAlgo() != GeoModel::enmMatchAlgo::agImage)
			_CalcDivScore(model, Result);
		else
			_CalcDivScore_Img(model, Result, nThreshold);
	}

	void GeoMatch_POCR::_CalcDivScore(GeoModel & model, GeoResult_POCR & Result)
	{
		for (int i = 0; i < Result.getResCnt(); i++)
			_FindModel_DivSearch(model._Models[GeoModel::enmDepth::spOrg][Result._AngleStepId[i]], i, cv::Point(Result._Center_y[i], Result._Center_x[i]), Result, model.getMatchAlgo());

		POCRFont fnt = _GetFontType(_Ch);
		if ((model.Image().rows <= 20 || model.Image().cols <= 20) && !_CheckThinFont(_Ch))
			fnt = POCRFont::SMall;
		for (int i = 0; i < Result.getResCnt(); i++)
		{
		if (model._Models[GeoModel::enmDepth::spOrg].find(Result._AngleStepId[i]) == model._Models[GeoModel::enmDepth::spOrg].end()
			|| model._Models[GeoModel::enmDepth::spOrg][Result._AngleStepId[i]] == nullptr 
			|| model._Models[GeoModel::enmDepth::spOrg][Result._AngleStepId[i]]->_noOfCordinates <= 0)
				continue;
			if (Result._Score[i].empty() == true)
				Result._Score[i].resize(_EngineParam[fnt].nPOCRInspCnt);

			for (int j = 0; j < _EngineParam[fnt].nPOCRInspCnt; j++)
			{
				cPOCRInspParam * param = _EngineParam[fnt]._vPOCRInspPtr[j]._vPOCRInspPtr;
				SetDivCnt2EngineParam(param);

				GeoModel::typItemPtr item = model._Models[GeoModel::enmDepth::spOrg][Result._AngleStepId[i]];
				_CalcPosSubArea(item, param, Result._Angle[i]);						// 1.문자 영역 나눠서 DivisionId(1~N)를 넣는다.
				_CalcAlgoSubScore(item, Result, param, i, j);	// 2.DivisionId(1~N) 별 알고리즘 점수 계산
			}
		}
	}

	void GeoMatch_POCR::_CalcDivScore_Img(GeoModel & model, GeoResult_POCR & Result, int nThreshold)
	{
		POCRFont fnt = _GetFontType(_Ch);
		if ((model.Image().rows <= 20 || model.Image().cols <= 20) && !_CheckThinFont(_Ch))
			fnt = POCRFont::SMall;
		for (int i = 0; i < Result.getResCnt(); i++)
		{
			if (Result._Score[i].empty() == true)
				Result._Score[i].resize(_EngineParam[fnt].nPOCRInspCnt);

			for (int j = 0; j < _EngineParam[fnt].nPOCRInspCnt; j++)
			{
				cPOCRInspParam * param = _EngineParam[fnt]._vPOCRInspPtr[j]._vPOCRInspPtr;
				SetDivCnt2EngineParam(param);
				_CalcPosSubArea(model, Result, param, i, j);
				_CalcAlgoSubScore(model, Result, param, i, j, nThreshold);
			}
		}
	}

	void GeoMatch_POCR::SetDivCnt2EngineParam(cPOCRInspParam * param)
	{
		switch (param->eDivisionArea)
		{
		case eHeight_2:
		case eWidth_2:
			param->nDivisionCnt = 2;
			break;
		case eLattice_4:
			param->nDivisionCnt = 4;
			break;
		case eLattice_9:
			param->nDivisionCnt = 9;
			break;
		case eLattice_6:
			param->nDivisionCnt = 6;
			break;
		case None:
			param->nDivisionCnt = 1;
			break;
		};
	}

	void GeoMatch_POCR::_CalcPosSubArea(GeoModel Model, GeoResult_POCR & Result, cPOCRInspParam * param, int iRes, int iEngine)
	{
		DivisionArea divArea = param->eDivisionArea;

		int margin(2);

		cv::Mat srcT = Model.Image();
		if (srcT.cols < 20 || srcT.rows < 20)
			margin = 0;
		cv::Mat src = srcT(cv::Rect(margin, margin, srcT.cols - margin * 2, srcT.rows - margin * 2));

		cv::Point2d ptList[4];
		ptList[0] = cv::Point2d(0, 0);
		ptList[1] = cv::Point2d(src.cols - 1, 0);
		ptList[2] = cv::Point2d(src.cols - 1, src.rows - 1);
		ptList[3] = cv::Point2d(0, src.rows - 1);

		if (divArea == DivisionArea::eHeight_2)
		{
			int dHalf = (int)((double)(src.cols - 1) / 2.0 + 0.5);

			Result._Score[iRes][iEngine]._Roi[0] = cv::Rect(0, 0, dHalf, src.rows);
			Result._Score[iRes][iEngine]._Roi[1] = cv::Rect(dHalf, 0, dHalf, src.rows);
		}
		else if (divArea == DivisionArea::eHeight_3)
		{
			int dHalfY = (int)((double)(src.cols) / 3.0 + 0.5);

			Result._Score[iRes][iEngine]._Roi[0] = cv::Rect(0, 0, dHalfY, src.rows);
			Result._Score[iRes][iEngine]._Roi[1] = cv::Rect(dHalfY, 0, dHalfY, src.rows);
			Result._Score[iRes][iEngine]._Roi[2] = cv::Rect(dHalfY*2.0, 0, dHalfY, src.rows);
		}
		else if (divArea == DivisionArea::eWidth_2)
		{
			int dHalf = (int)((double)(src.rows - 1) / 2.0 + 0.5);

			Result._Score[iRes][iEngine]._Roi[0] = cv::Rect(0, 0, src.cols, dHalf);
			Result._Score[iRes][iEngine]._Roi[1] = cv::Rect(0, dHalf, src.cols, dHalf);
		}
		else if (divArea == DivisionArea::eWidth_3)
		{
			int dHalfX = (int)((double)(src.rows) / 3.0 + 0.5);

			Result._Score[iRes][iEngine]._Roi[0] = cv::Rect(0, 0, src.cols, dHalfX);
			Result._Score[iRes][iEngine]._Roi[1] = cv::Rect(0, dHalfX, src.cols, dHalfX);
			Result._Score[iRes][iEngine]._Roi[2] = cv::Rect(0, dHalfX*2.0, src.cols, dHalfX);
		}
		else if (divArea == DivisionArea::eLattice_4)
		{
			int dHalfY = (int)((double)(src.cols - 1) / 2.0 + 0.5);
			int dHalfX = (int)((double)(src.rows - 1) / 2.0 + 0.5);

			Result._Score[iRes][iEngine]._Roi[0] = cv::Rect(0, 0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[1] = cv::Rect(dHalfY, 0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[2] = cv::Rect(0, dHalfX, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[3] = cv::Rect(dHalfY, dHalfX, dHalfY, dHalfX);
		}
		else if (divArea == DivisionArea::eLattice_9)
		{
			int dHalfY = (int)((double)(src.cols) / 3.0 + 0.5);
			int dHalfX = (int)((double)(src.rows) / 3.0 + 0.5);

			Result._Score[iRes][iEngine]._Roi[0] = cv::Rect(0, 0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[1] = cv::Rect(dHalfY, 0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[2] = cv::Rect(dHalfY*2.0, 0, dHalfY, dHalfX);

			Result._Score[iRes][iEngine]._Roi[3] = cv::Rect(0, dHalfX, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[4] = cv::Rect(dHalfY, dHalfX, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[5] = cv::Rect(dHalfY*2.0, dHalfX, dHalfY, dHalfX);

			Result._Score[iRes][iEngine]._Roi[6] = cv::Rect(0, dHalfX*2.0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[7] = cv::Rect(dHalfY, dHalfX*2.0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[8] = cv::Rect(dHalfY*2.0, dHalfX*2.0, dHalfY, dHalfX);
		}
		else if (divArea == DivisionArea::eLattice_6)
		{
			int dHalfY = (int)((double)(src.cols) / 2.0 + 0.5);
			int dHalfX = (int)((double)(src.rows) / 3.0 + 0.5);

			Result._Score[iRes][iEngine]._Roi[0] = cv::Rect(0, 0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[1] = cv::Rect(dHalfY, 0, dHalfY, dHalfX);

			Result._Score[iRes][iEngine]._Roi[2] = cv::Rect(0, dHalfX, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[3] = cv::Rect(dHalfY, dHalfX, dHalfY, dHalfX);

			Result._Score[iRes][iEngine]._Roi[4] = cv::Rect(0, dHalfX*2.0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[5] = cv::Rect(dHalfY, dHalfX*2.0, dHalfY, dHalfX);
		}
		else if (divArea == DivisionArea::eLatticeH_6)
		{
			int dHalfY = (int)((double)(src.cols) / 3.0 + 0.5);
			int dHalfX = (int)((double)(src.rows) / 2.0 + 0.5);

			Result._Score[iRes][iEngine]._Roi[0] = cv::Rect(0, 0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[1] = cv::Rect(dHalfY, 0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[2] = cv::Rect(dHalfY*2.0, 0, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[3] = cv::Rect(0, dHalfX, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[4] = cv::Rect(dHalfY, dHalfX, dHalfY, dHalfX);
			Result._Score[iRes][iEngine]._Roi[5] = cv::Rect(dHalfY*2.0, dHalfX, dHalfY, dHalfX);
		}
	}

	void GeoMatch_POCR::_setSubAreaId(GeoModel::typItemPtr item, cv::Point2d posList[][4], int nDivCnt)
	{
		for (int j = 0; j < item->_noOfCordinates; j++)
		{
			int areaX = (int)(item->_coordY[j]);
			int areaY = (int)(item->_coordX[j]);

			for (int i = 0; i < nDivCnt; i++)
			{
				if (_PtInPoly(posList[i], 4, cv::Point2d(areaX, areaY)))
				{
					item->_DivisionId[j] = i + 1;
					break;
				}
			}
		}
	}

	void GeoMatch_POCR::_CalcPosSubArea(GeoModel::typItemPtr item, cPOCRInspParam * param, double angle)
	{
		double AngRad = item->_Angle / 180.0 * PI;
		double radSin = sin(AngRad);
		double radCos = cos(AngRad);

		DivisionArea divArea = param->eDivisionArea;

		GeoModel * pModel = item->_ParentPtr;
		cv::Mat src = pModel->Image();
		float yCenter = item->_CogX;
		float xCenter = item->_CogY;

		cv::Point2d ptList[4];
		ptList[0] = cv::Point2d(-xCenter, -yCenter);
		ptList[1] = cv::Point2d(+xCenter, -yCenter);
		ptList[2] = cv::Point2d(+xCenter, +yCenter);
		ptList[3] = cv::Point2d(-xCenter, +yCenter);

		if (divArea == DivisionArea::eHeight_2)
		{
			double dHalf = ((double)(item->_modelWidth) / 2.0) - yCenter;

			const int nDivCnt(2);
			cv::Point2d rotSrc[nDivCnt][4], rotDst[nDivCnt][4];
			rotSrc[0][0] = ptList[0];
			rotSrc[0][1] = cv::Point2d(0.0, ptList[1].y);
			rotSrc[0][2] = cv::Point2d(0.0, ptList[2].y);
			rotSrc[0][3] = ptList[3];

			rotSrc[1][0] = cv::Point2d(0.0, ptList[0].y);
			rotSrc[1][1] = ptList[1];
			rotSrc[1][2] = ptList[2];
			rotSrc[1][3] = cv::Point2d(ptList[0].x, ptList[2].y);

			for (int pti = 0; pti < nDivCnt; pti++)
			{
				for (int ptrc = 0; ptrc < 4; ptrc++)
				{
					rotDst[pti][ptrc].x = (rotSrc[pti][ptrc].x) * radCos + (rotSrc[pti][ptrc].y) * radSin;
					rotDst[pti][ptrc].y = (-rotSrc[pti][ptrc].x) * radSin + (rotSrc[pti][ptrc].y) * radCos;
				}
			}

			_setSubAreaId(item, rotDst, nDivCnt);
		}
		else if (divArea == DivisionArea::eHeight_3)
		{
			double dHalfX = ((double)(item->_modelWidth) / 3.0) - xCenter;
			double dHalfXx2 = ((double)(item->_modelWidth) / 3.0 * 2.0) - xCenter;//dHalfY * 2.0;

			const int nDivCnt(3);
			cv::Point2d rotSrc[nDivCnt][4], rotDst[nDivCnt][4];
			rotSrc[0][0] = ptList[0];
			rotSrc[0][1] = cv::Point2d(dHalfX, ptList[1].y);
			rotSrc[0][2] = cv::Point2d(dHalfX, ptList[2].y);
			rotSrc[0][3] = ptList[3];

			rotSrc[1][0] = cv::Point2d(dHalfX, ptList[0].y);
			rotSrc[1][1] = cv::Point2d(dHalfXx2, ptList[1].y);
			rotSrc[1][2] = cv::Point2d(dHalfXx2, ptList[2].y);
			rotSrc[1][3] = cv::Point2d(dHalfX, ptList[3].y);

			rotSrc[2][0] = cv::Point2d(dHalfXx2, ptList[0].y);
			rotSrc[2][1] = ptList[1];
			rotSrc[2][2] = ptList[2];
			rotSrc[2][3] = cv::Point2d(dHalfXx2, ptList[3].y);

			for (int pti = 0; pti < nDivCnt; pti++)
			{
				for (int ptrc = 0; ptrc < 4; ptrc++)
				{
					rotDst[pti][ptrc].x = (rotSrc[pti][ptrc].x) * radCos + (rotSrc[pti][ptrc].y) * radSin;
					rotDst[pti][ptrc].y = (-rotSrc[pti][ptrc].x) * radSin + (rotSrc[pti][ptrc].y) * radCos;
				}
			}

#if 0
			int scaleVal(10);
			cv::Mat dsrc = cv::Mat::zeros(cv::Size(item->_modelWidth*scaleVal + scaleVal * 2, item->_modelHeight*scaleVal + scaleVal * 2), src.type());
			std::vector<cv::Point> TestDst[nDivCnt];
			int PtCnt(4);
			for (int pti = 0; pti < nDivCnt; pti++)
			{
				TestDst[pti].resize(4);
				for (int ptrc = 0; ptrc < PtCnt; ptrc++)
				{
					TestDst[pti][ptrc].x = (xCenter + rotDst[pti][ptrc].x) * (float)(scaleVal)+(float)(scaleVal);
					TestDst[pti][ptrc].y = (yCenter + rotDst[pti][ptrc].y) * (float)(scaleVal)+(float)(scaleVal);
				}
			}

			for (int pti = 0; pti < nDivCnt; pti++)
				cv::polylines(dsrc, TestDst[pti], true, cv::Scalar(30 + (pti + 1) * 10));

			cv::imwrite(std::string("d:\\test_rectangle.bmp"), dsrc);
			cv::imwrite(std::string("d:\\test_rectangle_model.bmp"), pModel->Image());
#endif

			_setSubAreaId(item, rotDst, nDivCnt);
		}
		else if (divArea == DivisionArea::eWidth_2)
		{
			double dHalf = ((double)(item->_modelHeight) / 2.0) - yCenter;

			const int nDivCnt(2);
			cv::Point2d rotSrc[nDivCnt][4], rotDst[nDivCnt][4];
			rotSrc[0][0] = ptList[0];
			rotSrc[0][1] = ptList[1];
			rotSrc[0][2] = cv::Point2d(ptList[2].x, 0.0);
			rotSrc[0][3] = cv::Point2d(ptList[3].x, 0.0);

			rotSrc[1][0] = cv::Point2d(ptList[0].x, 0.0);
			rotSrc[1][1] = cv::Point2d(ptList[1].x, 0.0);
			rotSrc[1][2] = ptList[2];
			rotSrc[1][3] = ptList[3];

			for (int pti = 0; pti < nDivCnt; pti++)
			{
				for (int ptrc = 0; ptrc < 4; ptrc++)
				{
					rotDst[pti][ptrc].x = (rotSrc[pti][ptrc].x) * radCos + (rotSrc[pti][ptrc].y) * radSin;
					rotDst[pti][ptrc].y = (-rotSrc[pti][ptrc].x) * radSin + (rotSrc[pti][ptrc].y) * radCos;
				}
			}

			_setSubAreaId(item, rotDst, nDivCnt);
		}
		else if (divArea == DivisionArea::eWidth_3)
		{
			double dHalfY = ((double)(item->_modelHeight) / 3.0) - yCenter;
			double dHalfYx2 = ((double)(item->_modelHeight) / 3.0 * 2.0) - yCenter;//dHalfY * 2.0;

			const int nDivCnt(3);
			cv::Point2d rotSrc[nDivCnt][4], rotDst[nDivCnt][4];
			rotSrc[0][0] = ptList[0];
			rotSrc[0][1] = ptList[1];
			rotSrc[0][2] = cv::Point2d(ptList[2].x, dHalfY);
			rotSrc[0][3] = cv::Point2d(ptList[3].x, dHalfY);

			rotSrc[1][0] = cv::Point2d(ptList[0].x, dHalfY);
			rotSrc[1][1] = cv::Point2d(ptList[1].x, dHalfY);
			rotSrc[1][2] = cv::Point2d(ptList[2].x, dHalfYx2);
			rotSrc[1][3] = cv::Point2d(ptList[3].x, dHalfYx2);

			rotSrc[2][0] = cv::Point2d(ptList[0].x, dHalfYx2);
			rotSrc[2][1] = cv::Point2d(ptList[1].x, dHalfYx2);
			rotSrc[2][2] = ptList[2];
			rotSrc[2][3] = ptList[3];

			for (int pti = 0; pti < nDivCnt; pti++)
			{
				for (int ptrc = 0; ptrc < 4; ptrc++)
				{
					rotDst[pti][ptrc].x = (rotSrc[pti][ptrc].x) * radCos + (rotSrc[pti][ptrc].y) * radSin;
					rotDst[pti][ptrc].y = (-rotSrc[pti][ptrc].x) * radSin + (rotSrc[pti][ptrc].y) * radCos;
				}
			}

			_setSubAreaId(item, rotDst, nDivCnt);
		}
		else if (divArea == DivisionArea::eLattice_4)
		{
			double dHalfX = ((double)(item->_modelWidth) / 2.0) - xCenter;
			double dHalfY = ((double)(item->_modelHeight) / 2.0) - yCenter;

			const int nDivCnt(4);
			cv::Point2d rotSrc[nDivCnt][4], rotDst[nDivCnt][4];
			rotSrc[0][0] = ptList[0];
			rotSrc[0][1] = cv::Point2d(0.0, ptList[1].y);
			rotSrc[0][2] = cv::Point2d(0.0, 0.0);
			rotSrc[0][3] = cv::Point2d(ptList[3].x, 0.0);

			rotSrc[1][0] = cv::Point2d(0.0, ptList[0].y);
			rotSrc[1][1] = ptList[1];
			rotSrc[1][2] = cv::Point2d(ptList[2].x, 0.0);
			rotSrc[1][3] = cv::Point2d(0.0, 0.0);

			rotSrc[2][0] = cv::Point2d(ptList[0].x, 0.0);
			rotSrc[2][1] = cv::Point2d(0.0, 0.0);
			rotSrc[2][2] = cv::Point2d(0.0, ptList[2].y);
			rotSrc[2][3] = ptList[3];

			rotSrc[3][0] = cv::Point2d(0.0, 0.0);
			rotSrc[3][1] = cv::Point2d(ptList[1].x, 0.0);
			rotSrc[3][2] = ptList[2];
			rotSrc[3][3] = cv::Point2d(0.0, ptList[3].y);


			for (int pti = 0; pti < nDivCnt; pti++)
			{
				for (int ptrc = 0; ptrc < 4; ptrc++)
				{
					rotDst[pti][ptrc].x = (rotSrc[pti][ptrc].x) * radCos + (rotSrc[pti][ptrc].y) * radSin;
					rotDst[pti][ptrc].y = (-rotSrc[pti][ptrc].x) * radSin + (rotSrc[pti][ptrc].y) * radCos;
				}
			}
#if 0
			int scaleVal(10);
			cv::Mat dsrc = cv::Mat::zeros(cv::Size(item->_modelWidth*scaleVal + scaleVal * 2, item->_modelHeight*scaleVal + scaleVal * 2), src.type());
			std::vector<cv::Point> TestDst[nDivCnt];
			int PtCnt(4);
			for (int pti = 0; pti < nDivCnt; pti++)
			{
				TestDst[pti].resize(4);
				for (int ptrc = 0; ptrc < PtCnt; ptrc++)
				{
					TestDst[pti][ptrc].x = (xCenter + rotDst[pti][ptrc].x) * (float)(scaleVal)+(float)(scaleVal);
					TestDst[pti][ptrc].y = (yCenter + rotDst[pti][ptrc].y) * (float)(scaleVal)+(float)(scaleVal);
				}
			}

			for (int pti = 0; pti < nDivCnt; pti++)
				cv::polylines(dsrc, TestDst[pti], true, cv::Scalar(30 + (pti + 1) * 10));

			cv::imwrite(std::string("d:\\test_rectangle.bmp"), dsrc);
			cv::imwrite(std::string("d:\\test_rectangle_model.bmp"), pModel->Image());
#endif
			_setSubAreaId(item, rotDst, nDivCnt);
			}
		else if (divArea == DivisionArea::eLattice_9)
		{
			double dHalfX = ((double)(item->_modelWidth) / 3.0) - xCenter;
			double dHalfY = ((double)(item->_modelHeight) / 3.0) - yCenter;
			double dHalfXx2 = ((double)(item->_modelWidth) / 3.0 * 2.0) - xCenter;
			double dHalfYx2 = ((double)(item->_modelHeight) / 3.0 * 2.0) - yCenter;

			const int nDivCnt(9);
			cv::Point2d rotSrc[nDivCnt][4], rotDst[nDivCnt][4];
			rotSrc[0][0] = ptList[0];
			rotSrc[0][1] = cv::Point2d(dHalfX, ptList[1].y);
			rotSrc[0][2] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[0][3] = cv::Point2d(ptList[3].x, dHalfY);

			rotSrc[1][0] = cv::Point2d(dHalfX, ptList[0].y);
			rotSrc[1][1] = cv::Point2d(dHalfXx2, ptList[0].y);
			rotSrc[1][2] = cv::Point2d(dHalfXx2, dHalfY);
			rotSrc[1][3] = cv::Point2d(dHalfX, dHalfY);

			rotSrc[2][0] = cv::Point2d(dHalfXx2, ptList[0].y);
			rotSrc[2][1] = cv::Point2d(ptList[1].x, ptList[0].y);
			rotSrc[2][2] = cv::Point2d(ptList[2].x, dHalfY);
			rotSrc[2][3] = cv::Point2d(dHalfXx2, dHalfY);



			rotSrc[3][0] = cv::Point2d(ptList[0].x, dHalfY);
			rotSrc[3][1] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[3][2] = cv::Point2d(dHalfX, dHalfYx2);
			rotSrc[3][3] = cv::Point2d(ptList[3].x, dHalfYx2);

			rotSrc[4][0] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[4][1] = cv::Point2d(dHalfXx2, dHalfY);
			rotSrc[4][2] = cv::Point2d(dHalfXx2, dHalfYx2);
			rotSrc[4][3] = cv::Point2d(dHalfX, dHalfYx2);

			rotSrc[5][0] = cv::Point2d(dHalfXx2, dHalfY);
			rotSrc[5][1] = cv::Point2d(ptList[1].x, dHalfY);
			rotSrc[5][2] = cv::Point2d(ptList[2].x, dHalfYx2);
			rotSrc[5][3] = cv::Point2d(dHalfXx2, dHalfYx2);



			rotSrc[6][0] = cv::Point2d(ptList[0].x, dHalfYx2);
			rotSrc[6][1] = cv::Point2d(dHalfX, dHalfYx2);
			rotSrc[6][2] = cv::Point2d(dHalfX, ptList[2].y);
			rotSrc[6][3] = ptList[3];

			rotSrc[7][0] = cv::Point2d(dHalfX, dHalfYx2);
			rotSrc[7][1] = cv::Point2d(dHalfXx2, dHalfYx2);
			rotSrc[7][2] = cv::Point2d(dHalfXx2, ptList[2].y);
			rotSrc[7][3] = cv::Point2d(dHalfX, ptList[3].y);

			rotSrc[8][0] = cv::Point2d(dHalfXx2, dHalfYx2);
			rotSrc[8][1] = cv::Point2d(ptList[1].x, dHalfYx2);
			rotSrc[8][2] = ptList[2];
			rotSrc[8][3] = cv::Point2d(dHalfXx2, ptList[3].y);


			for (int pti = 0; pti < nDivCnt; pti++)
			{
				for (int ptrc = 0; ptrc < 4; ptrc++)
				{
					rotDst[pti][ptrc].x = (rotSrc[pti][ptrc].x) * radCos + (rotSrc[pti][ptrc].y) * radSin;
					rotDst[pti][ptrc].y = (-rotSrc[pti][ptrc].x) * radSin + (rotSrc[pti][ptrc].y) * radCos;
				}
			}
#if 0
			int scaleVal(10);
			cv::Mat dsrc = cv::Mat::zeros(cv::Size(item->_modelWidth*scaleVal + scaleVal * 2, item->_modelHeight*scaleVal + scaleVal * 2), src.type());
			std::vector<cv::Point> TestDst[nDivCnt];
			int PtCnt(4);
			for (int pti = 0; pti < nDivCnt; pti++)
			{
				TestDst[pti].resize(4);
				for (int ptrc = 0; ptrc < PtCnt; ptrc++)
				{
					TestDst[pti][ptrc].x = (xCenter + rotDst[pti][ptrc].x) * (float)(scaleVal)+(float)(scaleVal);
					TestDst[pti][ptrc].y = (yCenter + rotDst[pti][ptrc].y) * (float)(scaleVal)+(float)(scaleVal);
				}
			}

			for (int pti = 0; pti < nDivCnt; pti++)
				cv::polylines(dsrc, TestDst[pti], true, cv::Scalar(30 + (pti + 1) * 10));

			cv::imwrite(std::string("d:\\test_rectangle.bmp"), dsrc);
			cv::imwrite(std::string("d:\\test_rectangle_model.bmp"), pModel->Image());
#endif
			_setSubAreaId(item, rotDst, nDivCnt);
			}
		else if (divArea == DivisionArea::eLattice_6)
		{
			double dHalfX = ((double)(item->_modelWidth) / 2.0) - xCenter;

			double dHalfY = ((double)(item->_modelHeight) / 3.0) - yCenter;
			double dHalfYx2 = ((double)(item->_modelHeight) / 3.0 * 2.0) - yCenter;

			const int nDivCnt(6);
			cv::Point2d rotSrc[nDivCnt][4], rotDst[nDivCnt][4];
			rotSrc[0][0] = ptList[0];
			rotSrc[0][1] = cv::Point2d(dHalfX, ptList[1].y);
			rotSrc[0][2] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[0][3] = cv::Point2d(ptList[3].x, dHalfY);

			rotSrc[1][0] = cv::Point2d(dHalfX, ptList[0].y);
			rotSrc[1][1] = ptList[1];
			rotSrc[1][2] = cv::Point2d(ptList[2].x, dHalfY);
			rotSrc[1][3] = cv::Point2d(dHalfX, dHalfY);



			rotSrc[2][0] = cv::Point2d(ptList[0].x, dHalfY);
			rotSrc[2][1] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[2][2] = cv::Point2d(dHalfX, dHalfYx2);
			rotSrc[2][3] = cv::Point2d(ptList[3].x, dHalfYx2);

			rotSrc[3][0] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[3][1] = cv::Point2d(ptList[1].x, dHalfY);
			rotSrc[3][2] = cv::Point2d(ptList[2].x, dHalfYx2);
			rotSrc[3][3] = cv::Point2d(dHalfX, dHalfYx2);



			rotSrc[4][0] = cv::Point2d(ptList[0].x, dHalfYx2);
			rotSrc[4][1] = cv::Point2d(dHalfX, dHalfYx2);
			rotSrc[4][2] = cv::Point2d(dHalfX, ptList[2].y);
			rotSrc[4][3] = ptList[3];

			rotSrc[5][0] = cv::Point2d(dHalfX, dHalfYx2);
			rotSrc[5][1] = cv::Point2d(ptList[1].x, dHalfYx2);
			rotSrc[5][2] = ptList[2];
			rotSrc[5][3] = cv::Point2d(dHalfX, ptList[3].y);



			for (int pti = 0; pti < nDivCnt; pti++)
			{
				for (int ptrc = 0; ptrc < 4; ptrc++)
				{
					rotDst[pti][ptrc].x = (rotSrc[pti][ptrc].x) * radCos + (rotSrc[pti][ptrc].y) * radSin;
					rotDst[pti][ptrc].y = (-rotSrc[pti][ptrc].x) * radSin + (rotSrc[pti][ptrc].y) * radCos;
				}
			}

#if 0
			int scaleVal(10);
			cv::Mat dsrc = cv::Mat::zeros(cv::Size(item->_modelWidth*scaleVal + scaleVal * 2, item->_modelHeight*scaleVal + scaleVal * 2), src.type());
			std::vector<cv::Point> TestDst[nDivCnt];
			int PtCnt(4);
			for (int pti = 0; pti < nDivCnt; pti++)
			{
				TestDst[pti].resize(4);
				for (int ptrc = 0; ptrc < PtCnt; ptrc++)
				{
					TestDst[pti][ptrc].x = (xCenter + rotDst[pti][ptrc].x) * (float)(scaleVal)+(float)(scaleVal);
					TestDst[pti][ptrc].y = (yCenter + rotDst[pti][ptrc].y) * (float)(scaleVal)+(float)(scaleVal);
				}
			}

			for (int pti = 0; pti < nDivCnt; pti++)
				cv::polylines(dsrc, TestDst[pti], true, cv::Scalar(30 + (pti + 1) * 10));

			cv::imwrite(std::string("d:\\test_rectangle.bmp"), dsrc);
			cv::imwrite(std::string("d:\\test_rectangle_model.bmp"), pModel->Image());
#endif

			_setSubAreaId(item, rotDst, nDivCnt);
		}
		else if (divArea == DivisionArea::eLatticeH_6)
		{
			double dHalfX = ((double)(item->_modelWidth) / 3.0) - xCenter;
			double dHalfXx2 = ((double)(item->_modelWidth) / 3.0 * 2.0) - xCenter;

			double dHalfY = ((double)(item->_modelHeight) / 2.0) - yCenter;

			const int nDivCnt(6);
			cv::Point2d rotSrc[nDivCnt][4], rotDst[nDivCnt][4];
			rotSrc[0][0] = ptList[0];
			rotSrc[0][1] = cv::Point2d(dHalfX, ptList[1].y);
			rotSrc[0][2] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[0][3] = cv::Point2d(ptList[3].x, dHalfY);

			rotSrc[1][0] = cv::Point2d(dHalfX, ptList[0].y);
			rotSrc[1][1] = cv::Point2d(dHalfXx2, ptList[1].y);
			rotSrc[1][2] = cv::Point2d(dHalfXx2, dHalfY);
			rotSrc[1][3] = cv::Point2d(dHalfX, dHalfY);

			rotSrc[2][0] = cv::Point2d(dHalfXx2, ptList[0].y);
			rotSrc[2][1] = ptList[1];
			rotSrc[2][2] = cv::Point2d(ptList[2].x, dHalfY);
			rotSrc[2][3] = cv::Point2d(dHalfXx2, dHalfY);

			rotSrc[3][0] = cv::Point2d(ptList[0].x, dHalfY);
			rotSrc[3][1] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[3][2] = cv::Point2d(dHalfX, ptList[2].y);
			rotSrc[3][3] = ptList[3];

			rotSrc[4][0] = cv::Point2d(dHalfX, dHalfY);
			rotSrc[4][1] = cv::Point2d(dHalfXx2, dHalfY);
			rotSrc[4][2] = cv::Point2d(dHalfXx2, ptList[2].y);
			rotSrc[4][3] = cv::Point2d(dHalfX, ptList[3].y);

			rotSrc[5][0] = cv::Point2d(dHalfXx2, dHalfY);
			rotSrc[5][1] = cv::Point2d(ptList[1].x, dHalfY);
			rotSrc[5][2] = ptList[2];
			rotSrc[5][3] = cv::Point2d(dHalfXx2, ptList[3].y);

			for (int pti = 0; pti < nDivCnt; pti++)
			{
				for (int ptrc = 0; ptrc < 4; ptrc++)
				{
					rotDst[pti][ptrc].x = (rotSrc[pti][ptrc].x) * radCos + (rotSrc[pti][ptrc].y) * radSin;
					rotDst[pti][ptrc].y = (-rotSrc[pti][ptrc].x) * radSin + (rotSrc[pti][ptrc].y) * radCos;
				}
			}

#if 0
			int scaleVal(10);
			cv::Mat dsrc = cv::Mat::zeros(cv::Size(item->_modelWidth*scaleVal + scaleVal * 2, item->_modelHeight*scaleVal + scaleVal * 2), src.type());
			std::vector<cv::Point> TestDst[nDivCnt];
			int PtCnt(4);
			for (int pti = 0; pti < nDivCnt; pti++)
			{
				TestDst[pti].resize(4);
				for (int ptrc = 0; ptrc < PtCnt; ptrc++)
				{
					TestDst[pti][ptrc].x = (xCenter + rotDst[pti][ptrc].x) * (float)(scaleVal)+(float)(scaleVal);
					TestDst[pti][ptrc].y = (yCenter + rotDst[pti][ptrc].y) * (float)(scaleVal)+(float)(scaleVal);
				}
			}

			for (int pti = 0; pti < nDivCnt; pti++)
				cv::polylines(dsrc, TestDst[pti], true, cv::Scalar(30 + (pti + 1) * 10));

			cv::imwrite(std::string("d:\\test_rectangle.bmp"), dsrc);
			cv::imwrite(std::string("d:\\test_rectangle_model.bmp"), pModel->Image());
#endif

			_setSubAreaId(item, rotDst, nDivCnt);
		}
		}

	void GeoMatch_POCR::_CalcAlgoSubScore(GeoModel model, GeoResult_POCR & Result, cPOCRInspParam * param, int iRes, int iEngine, int nThreshold)
	{
		cv::Mat src = *_ProcImg->_OrgImage;
		// _ROI를 이용해서 이미지 매칭
		for (int i = 0; i < param->nDivisionCnt; i++)
		{
			if (param->ePOCRAlgoType[i] != POCRAlgoType::eImage && param->ePOCRAlgoType[i] != POCRAlgoType::eImageMin)
				continue;

			cv::Mat tempT = model.Image();
			cv::Mat temp = tempT(cv::Rect(2, 2, tempT.cols - 4, tempT.rows - 4));
			cv::Rect roiTempl = Result._Score[iRes][iEngine]._Roi[i];

			if (roiTempl.x < 0)
				roiTempl.x = 0;
			if (roiTempl.y < 0)
				roiTempl.y = 0;
			if (roiTempl.x + roiTempl.width > temp.cols)
				roiTempl.width -= (roiTempl.x + roiTempl.width) - temp.cols;
			if (roiTempl.y + roiTempl.height > temp.rows)
				roiTempl.height -= (roiTempl.y + roiTempl.height) - temp.rows;

			int stRoiX = (int)(Result._Center_y[iRes] - (float)(temp.cols) * 0.5);
			int stRoiY = (int)(Result._Center_x[iRes] - (float)(temp.rows) * 0.5);

			int margin = 0;
			int MaxLength = temp.rows;
			if (temp.rows < temp.cols)
				MaxLength = temp.cols;

			if (MaxLength < 20)
				margin = 1;
			else if (MaxLength < 25)
				margin = 2;
			else if (MaxLength >= 25 && MaxLength < 50)
				margin = 4;
			else if (MaxLength >= 50 && MaxLength < 100)
				margin = 6;
			else if (MaxLength >= 100)
				margin = 8;

			int x = stRoiX + roiTempl.x - margin;
			int y = stRoiY + roiTempl.y - margin;
			int wid = roiTempl.width + (margin * 2);
			int len = roiTempl.height + (margin * 2);

			if (x < 0)
				x = 0;
			if (y < 0)
				y = 0;
			if (x + wid > src.cols)
			{
				roiTempl.width -= (x + wid) - src.cols;
				wid -= (x + wid) - src.cols;
			}
			if (y + len > src.rows)
			{
				roiTempl.height -= (y + len) - src.rows;
				len -= (y + len) - src.rows;
			}




			if (roiTempl.width < 1 || roiTempl.height < 1)
			{
				Result._Score[iRes][iEngine]._Score[i] = 0.f;
			}
			else
			{
				bool bOnlyBackValue = true;
				cv::Mat tmp = temp(roiTempl);
				//	imwrite("D:\\testimage\\patternMathTest\\model_Image_temp.bmp",tmp);
		//			uchar* tmpptr = tmp.data;
		// 			double tmpAvgValue =cv::mean(tmp).val[0];
		// 			for(int row = 0;row<roiTempl.height;row++)
		// 			{
		// 				for(int cols = 0;cols<roiTempl.width;cols++)
		// 				{
		// 					if(tmp.ptr<uchar>((row), (cols))[0] != tmpAvgValue)
		// 					{
		// 						bOnlyBackValue = false;
		// 						break;
		// 					}
		// 				}
		// 			}
				if (!bOnlyBackValue)
				{
					cv::Mat roi = src(cv::Rect(x, y, wid, len));
					cv::Mat res;


					SaveImg(roi, _T("CalcAlgoSubScore"), _T("Template_roi"));
					SaveImg(temp(roiTempl), _T("CalcAlgoSubScore"), _T("Template_roitemp"));

					cv::matchTemplate(roi, temp(roiTempl), res, cv::TM_CCOEFF_NORMED);
					cv::Point minP, maxP;
					double minV(0), maxV(0);
					cv::minMaxLoc(res, &minV, &maxV, &minP, &maxP);
					Result._Score[iRes][iEngine]._Score[i] = maxV * 100.0;
					Result._Score[iRes][iEngine]._Count[i] = roi.rows * roi.cols;
				}
				else
				{

					int nModelRoiW(temp.cols), nModelRoiL(temp.rows);
					int nFontRoiX(stRoiX), nFontRoiY(stRoiY), nFontRoiW(temp.cols), nFontRoiL(temp.rows);
					if (nFontRoiX < 0)
						nFontRoiX = 0;
					if (nFontRoiY < 0)
						nFontRoiY = 0;
					if (nFontRoiX + nFontRoiW >= src.cols)
					{
						nFontRoiW -= (nFontRoiX + nFontRoiW) - src.cols + 1;
						nModelRoiW -= (nFontRoiX + nFontRoiW) - src.cols + 1;
					}
					if (nFontRoiY + nFontRoiL >= src.rows)
					{
						nFontRoiL -= (nFontRoiY + nFontRoiL) - src.rows + 1;
						nModelRoiL -= (nFontRoiY + nFontRoiL) - src.rows + 1;
					}
					cv::Mat FontRoiImg = src(cv::Rect(nFontRoiX, nFontRoiY, nFontRoiW, nFontRoiL));
					cv::Mat ModelImg = temp(cv::Rect(0, 0, nModelRoiW, nModelRoiL));

					cv::Mat roi = src(cv::Rect(x, y, wid, len));

					cv::Mat modelImage = temp(roiTempl);

					imgMatch ImMatModel, ImMatinputImg, ImMatinsp;
					ImMatModel.calcValue(temp, nThreshold);
					ImMatinputImg.calcValue(FontRoiImg, nThreshold);

					ImMatModel.setImage(modelImage);
					ImMatinputImg.setImage(roi);

					int rows = roi.rows - modelImage.rows > 0 ? roi.rows - modelImage.rows : 1;
					int cols = roi.cols - modelImage.cols > 0 ? roi.cols - modelImage.cols : 1;

					cv::Mat scoreMat = cv::Mat(rows, cols, CV_32FC1);
					float* score = scoreMat.ptr<float>(0);
					for (int r = 0; r < rows; r++)
					{
						for (int j = 0; j < cols; j++)
						{
							ImMatinputImg.copyTo(ImMatinsp, cv::Rect(j, r, ImMatModel.m_model.cols, ImMatModel.m_model.rows));
#ifndef GeoNoUseSimd
							score[r*cols + j] = ImMatModel.ImageMatching_sse(ImMatinsp, ImMatinputImg.m_model.cols);
#else
							score[r*cols + j] = ImMatModel.ImageMatching(ImMatinsp, ImMatinputImg.m_model.cols);
#endif
						}
					}
#if ImageSave
					imwrite("D:\\testimage\\patternMathTest\\ImMatinputImg_m_model.bmp", ImMatModel.m_model);
					imwrite("D:\\testimage\\patternMathTest\\ImMatModel_m_model.bmp", ImMatinputImg.m_model);
					imwrite("D:\\testimage\\patternMathTest\\scoreMat.bmp", scoreMat);
#endif
					cv::Point minP, maxP;
					double minV(0), maxV(0);
					cv::minMaxLoc(scoreMat, &minV, &maxV, &minP, &maxP);
					/*if(_Ch == 'C')
					{
						Result._Score[iRes][iEngine]._Score[i] = minV * 100.0;
					}
					else
					{
						Result._Score[iRes][iEngine]._Score[i] = maxV * 100.0;
					}*/
					if (param->ePOCRAlgoType[i] == POCRAlgoType::eImageMin/* || _Ch == 'C'*/)
					{
						Result._Score[iRes][iEngine]._Score[i] = minV * 100.0;
					}
					else
					{
						Result._Score[iRes][iEngine]._Score[i] = maxV * 100.0;
					}
					Result._Score[iRes][iEngine]._Count[i] = roi.rows * roi.cols;
					}
				}
			}
		}

	void GeoMatch_POCR::_CalcAlgoSubScore(GeoModel::typItemPtr item, GeoResult_POCR & Result, cPOCRInspParam * param, int iRes, int iEngine)
	{
		GeoModel * pModel = item->_ParentPtr;

		Result._Score[iRes][iEngine]._AreaCount = param->nDivisionCnt;

		if (pModel->getMatchAlgo() == GeoModel::enmMatchAlgo::agDefault || pModel->getMatchAlgo() == GeoModel::enmMatchAlgo::agLowContrast)
		{
			float * List = &Result._ResEdge[iRes][0];

			for (int i = 0; i < param->nDivisionCnt; i++)
			{
				if (param->ePOCRAlgoType[i] == POCRAlgoType::eDefault || param->ePOCRAlgoType[i] == POCRAlgoType::eLowContrast)
				{
					int noOfCordinates = item->_noOfCordinates;
					int * pId = item->_DivisionId;

					double nDataCount(0), DataSum(0);
					for (int m = 0; m < noOfCordinates; m++)
					{
						if (pId[m] != (i + 1))
							continue;

						DataSum += List[m];
						nDataCount += 1.0;
					}

					Result._Score[iRes][iEngine]._Count[i] = nDataCount;

					if (nDataCount > 0)
						Result._Score[iRes][iEngine]._Score[i] = DataSum / nDataCount * 100.0;
					else
						Result._Score[iRes][iEngine]._Score[i] = 0.0;
				}
			}
		}
		else if (pModel->getMatchAlgo() == GeoModel::enmMatchAlgo::agRelief || pModel->getMatchAlgo() == GeoModel::enmMatchAlgo::agIntaglio)
		{
			float * List = &Result._ResDiff[iRes][0];

			for (int i = 0; i < param->nDivisionCnt; i++)
			{
				if (param->ePOCRAlgoType[i] == POCRAlgoType::eRelief || param->ePOCRAlgoType[i] == POCRAlgoType::eIntagrio)
				{
					int noOfCordinates = item->_noOfCordinates;
					int * pId = item->_DivisionId;
					float * pEdgeX = item->_edgeDerivativeX;

					double nDataCount(0), SrcSum(0), DstSum(0), DataSum(0);
					for (int m = 0; m < noOfCordinates; m++)
					{
						if (pId[m] != (i + 1))
							continue;

						SrcSum += pow(pEdgeX[m], 2);
						DstSum += pow(List[m], 2);

						DataSum += (List[m] + GeoEpsilon) * (pEdgeX[m] + GeoEpsilon);

						nDataCount += 1.0;
					}

					Result._Score[iRes][iEngine]._Count[i] = nDataCount;

					if (nDataCount)
					{
						DstSum = sqrt(DstSum / nDataCount);
						SrcSum = sqrt(SrcSum / nDataCount);

						if (nDataCount > 0)
							Result._Score[iRes][iEngine]._Score[i] = (DataSum / ((DstSum + GeoEpsilon)*(SrcSum + GeoEpsilon))) / nDataCount * 100.0;
						else
							Result._Score[iRes][iEngine]._Score[i] = 0.0;
					}
					else
					{
						Result._Score[iRes][iEngine]._Score[i] = 0.0;
					}
				}
			}
		}
	}

	void GeoMatch_POCR::CalcResScore(GeoResult_POCR & Result, bool isSmallFont)
	{
		float RequireScore(0.4);

		POCRFont fnt = _GetFontType(_Ch);
		if (isSmallFont)
			fnt = POCRFont::SMall;
		for (int i = 0; i < Result.getResCnt(); i++)
		{
			bool bRequireRes(false);
			int nMaxScoreIdx(INT_MAX);
			float MaxScore(-HUGE_VAL);
			for (int j = 0; j < _EngineParam[fnt].nPOCRInspCnt; j++)
			{
				cPOCRInspParam * param = _EngineParam[fnt]._vPOCRInspPtr[j]._vPOCRInspPtr;

				int nRequreAreaCnt(0), nRequrePassCnt(0);
				float SumScore(0.f), SumCoeff(0.f), MinScore(HUGE_VAL), RqMinScore(HUGE_VAL);
				for (int k = 0; k < param->nDivisionCnt; k++)
				{
					if (param->eRequiredArea[k] == RequiredType::RequiredType_eNG)
						continue;
					else if (param->eRequiredArea[k] == RequiredType::RequiredType_eOK)
					{
						nRequreAreaCnt++;
						if (Result._Score[i][j]._Score[k] > RequireScore)
						{
							if (Result._Score[i][j]._Count[k] > 5 && RqMinScore > Result._Score[i][j]._Score[k])
							{
								float WeightedScore = Result._Score[i][j]._Score[k] * (float)(param->nDivisionScore[k]);
								RqMinScore = WeightedScore;
							}
							nRequrePassCnt++;
						}
					}
					SumCoeff += (float)(param->nDivisionScore[k]);

					float WeightedScore = Result._Score[i][j]._Score[k] * (float)(param->nDivisionScore[k]);
					SumScore += WeightedScore;



					if (Result._Score[i][j]._Count[k] > 5 && MinScore > WeightedScore && param->nDivisionScore[k] > 0)
					{
						MinScore = WeightedScore;
					}
				}

				if (nRequreAreaCnt > 0 && nRequreAreaCnt == nRequrePassCnt)
				{
					Result._Score[i][j]._ResScore = RqMinScore * 0.01;
				}
				else
				{
					Result._Score[i][j]._ResScore = MinScore * 0.01;
				}


				// 			if(SumCoeff != 0.f)
				// 				Result._Score[i][j]._ResScore = SumScore / SumCoeff;
				// 			else
				// 				Result._Score[i][j]._ResScore = 0.0;

				if (MaxScore < Result._Score[i][j]._ResScore)
				{
					nMaxScoreIdx = j;
					MaxScore = Result._Score[i][j]._ResScore;
				}
			}

			Result._Equality[i] = MaxScore;
			Result._SelEngine[i] = nMaxScoreIdx;
		}

		SaveResScore(Result, isSmallFont);
	}

	void GeoMatch_POCR::SaveResScore(GeoResult_POCR & Result, bool isSmallFont)
	{
		if (_bSaveProcData.IsEmpty() == true)
			return;

		LARGE_INTEGER freq, curr;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&curr);

		CString sTime;
		sTime.Format(_T("_%.3f"), (double)(curr.QuadPart) / (double)(freq.QuadPart));

		CString sPath, funcName(_T("SaveResScore")), FontName;
		FontName.Format(_T("_%c"), _Ch);
		sPath = CString(_T("D:\\TestImage\\workimage\\")) + funcName + CString(_bSaveProcData) + FontName + sTime + CString(_T(".csv"));
		POCRFont fnt = _GetFontType(_Ch);
		if (isSmallFont)
			fnt = POCRFont::SMall;
		CStdioFile file(sPath, CFile::modeCreate | CFile::modeWrite);
		for (int i = 0; i < Result.getResCnt(); i++)
		{
			for (int j = 0; j < _EngineParam[fnt].nPOCRInspCnt; j++)
			{
				cPOCRInspParam * param = _EngineParam[fnt]._vPOCRInspPtr[j]._vPOCRInspPtr;
				CString sLog;
				for (int k = 0; k < param->nDivisionCnt; k++)
				{
					CString sTmp;
					sTmp.Format(_T("%.3f(%d),"), Result._Score[i][j]._Score[k], Result._Score[i][j]._Count[k]);
					sLog += sTmp;
				}
				sLog += _T("\n");
				file.WriteString(sLog);
			}
		}

		file.Close();
	}

	POCRFont GeoMatch_POCR::_GetFontType(TCHAR ch)
	{
		switch (ch)
		{
		case '0':	return POCRFont::_0;		break;
		case '1':	return POCRFont::_1;		break;
		case '2':	return POCRFont::_2;		break;
		case '3':	return POCRFont::_3;		break;
		case '4':	return POCRFont::_4;		break;
		case '5':	return POCRFont::_5;		break;
		case '6':	return POCRFont::_6;		break;
		case '7':	return POCRFont::_7;		break;
		case '8':	return POCRFont::_8;		break;
		case '9':	return POCRFont::_9;		break;

		case 'A':	return POCRFont::A;		break;
		case 'B':	return POCRFont::B;		break;
		case 'C':	return POCRFont::C;		break;
		case 'D':	return POCRFont::D;		break;
		case 'E':	return POCRFont::E;		break;
		case 'F':	return POCRFont::F;		break;
		case 'G':	return POCRFont::G;		break;
		case 'H':	return POCRFont::H;		break;
		case 'I':	return POCRFont::I;		break;
		case 'J':	return POCRFont::J;		break;

		case 'K':	return POCRFont::K;		break;
		case 'L':	return POCRFont::L;		break;
		case 'M':	return POCRFont::M;		break;
		case 'N':	return POCRFont::N;		break;
		case 'O':	return POCRFont::O;		break;
		case 'P':	return POCRFont::P;		break;
		case 'Q':	return POCRFont::Q;		break;
		case 'R':	return POCRFont::R;		break;
		case 'S':	return POCRFont::S;		break;
		case 'T':	return POCRFont::T;		break;

		case 'U':	return POCRFont::U;		break;
		case 'V':	return POCRFont::V;		break;
		case 'W':	return POCRFont::W;		break;
		case 'X':	return POCRFont::X;		break;
		case 'Y':	return POCRFont::Y;		break;
		case 'Z':	return POCRFont::Z;		break;


		case 'a':	return POCRFont::a;		break;
		case 'b':	return POCRFont::b;		break;
		case 'c':	return POCRFont::c;		break;
		case 'd':	return POCRFont::d;		break;
		case 'e':	return POCRFont::e;		break;
		case 'f':	return POCRFont::f;		break;
		case 'g':	return POCRFont::g;		break;
		case 'h':	return POCRFont::h;		break;
		case 'i':	return POCRFont::i;		break;
		case 'j':	return POCRFont::j;		break;

		case 'k':	return POCRFont::k;		break;
		case 'l':	return POCRFont::l;		break;
		case 'm':	return POCRFont::m;		break;
		case 'n':	return POCRFont::n;		break;
		case 'o':	return POCRFont::o;		break;
		case 'p':	return POCRFont::p;		break;
		case 'q':	return POCRFont::q;		break;
		case 'r':	return POCRFont::r;		break;
		case 's':	return POCRFont::s;		break;
		case 't':	return POCRFont::t;		break;

		case 'u':	return POCRFont::u;		break;
		case 'v':	return POCRFont::v;		break;
		case 'w':	return POCRFont::w;		break;
		case 'x':	return POCRFont::x;		break;
		case 'y':	return POCRFont::y;		break;
		case 'z':	return POCRFont::z;		break;
		}
	}

	bool GeoMatch_POCR::_CheckFont(TCHAR ch)
	{
		switch (ch)
		{
		case '0':	return true;		break;
		case '1':	return true;		break;
		case '2':	return true;		break;
		case '3':	return true;		break;
		case '4':	return true;		break;
		case '5':	return true;		break;
		case '6':	return true;		break;
		case '7':	return true;		break;
		case '8':	return true;		break;
		case '9':	return true;		break;

		case 'A':	return true;		break;
		case 'B':	return true;		break;
		case 'C':	return true;		break;
		case 'D':	return true;		break;
		case 'E':	return true;		break;
		case 'F':	return true;		break;
		case 'G':	return true;		break;
		case 'H':	return true;		break;
		case 'I':	return true;		break;
		case 'J':	return true;		break;

		case 'K':	return true;		break;
		case 'L':	return true;		break;
		case 'M':	return true;		break;
		case 'N':	return true;		break;
		case 'O':	return true;		break;
		case 'P':	return true;		break;
		case 'Q':	return true;		break;
		case 'R':	return true;		break;
		case 'S':	return true;		break;
		case 'T':	return true;		break;

		case 'U':	return true;		break;
		case 'V':	return true;		break;
		case 'W':	return true;		break;
		case 'X':	return true;		break;
		case 'Y':	return true;		break;
		case 'Z':	return true;		break;


		case 'a':	return true;		break;
		case 'b':	return true;		break;
		case 'c':	return true;		break;
		case 'd':	return true;		break;
		case 'e':	return true;		break;
		case 'f':	return true;		break;
		case 'g':	return true;		break;
		case 'h':	return true;		break;
		case 'i':	return true;		break;
		case 'j':	return true;		break;

		case 'k':	return true;		break;
		case 'l':	return true;		break;
		case 'm':	return true;		break;
		case 'n':	return true;		break;
		case 'o':	return true;		break;
		case 'p':	return true;		break;
		case 'q':	return true;		break;
		case 'r':	return true;		break;
		case 's':	return true;		break;
		case 't':	return true;		break;

		case 'u':	return true;		break;
		case 'v':	return true;		break;
		case 'w':	return true;		break;
		case 'x':	return true;		break;
		case 'y':	return true;		break;
		case 'z':	return true;		break;

		default:	return false;		break;
		}
	}
	bool GeoMatch_POCR::_CheckThinFont(TCHAR ch)
	{
		switch (ch)
		{
		case '1':	return true;		break;

		case 'I':	return true;		break;
		case 'i':	return true;		break;
		case 'j':	return true;		break;

		case 'l':	return true;		break;

		default:	return false;		break;
		}
	}

	void GeoMatch_POCR::_FindModel_DivSearch(GeoModel::typItemPtr ModelItem, int idx, cv::Point SStart, GeoResult_POCR & Result, int use_algo_model)
	{
		if (ModelItem != nullptr)
		{
			if (ModelItem->_noOfCordinates <= 0)
				return;

			GeoModel * pParent = (GeoModel *)ModelItem->_ParentPtr;

			cv::Mat Sdl = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdl;
			cv::Mat Sdx = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdx;
			cv::Mat Sdy = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdy;
			float * matGradMag = _ProcImg->_ProcBuf[ModelItem->_PyrDown].matGradMag;

			//	for(int i=0; i<Result.getResCnt(); i++)
			{
				if (use_algo_model == GeoModel::enmMatchAlgo::agIntaglio || use_algo_model == GeoModel::enmMatchAlgo::agRelief)
				{
					Result._ResDiff[idx].clear();
					Result._ResDiff[idx].resize(ModelItem->_noOfCordinates);
					_CalcCoefficient_Diff_Divide(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, SStart, ModelItem, &Result._ResDiff[idx][0]);
				}
				else if (use_algo_model == GeoModel::enmMatchAlgo::agDefault || use_algo_model == GeoModel::enmMatchAlgo::agLowContrast)
				{
					Result._ResEdge[idx].clear();
					Result._ResEdge[idx].resize(ModelItem->_noOfCordinates);
					_CalcCoefficient_Edge_Divide(Sdx, Sdy, matGradMag, SStart, ModelItem, &Result._ResEdge[idx][0]);
				}
			}
		}
	}

	void GeoMatch_POCR::_CalcCoefficient_Edge_Divide(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float * pResBuf)
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
		int * pId = ModelItem->_DivisionId;
		BYTE * pFlags = ModelItem->_Flags;

		int ImgWid = Sdx.cols;
		int ImgLen = Sdx.rows;

		int j = SStart.x;
		int i = SStart.y;


		for (int m = 0; m < noOfCordinates; m++)
		{
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

			if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
				pResBuf[m] = ((iSx*iTx) + (iSy*iTy)) * (iMag*matGradMag[curX*ImgWid + curY]);
		}
	}

	void GeoMatch_POCR::_CalcCoefficient_Diff_Divide(cv::Mat & Sdl, cv::Mat & Integ, cv::Point SStart, GeoModel::typItemPtr ModelItem, float * pResBuf)
	{
		if (ModelItem->isProcessed() == false)
			return;

		GeoModel * pModel = ModelItem->_ParentPtr;

		int noOfCordinates = ModelItem->_noOfCordinates;
		float * pCoordX = ModelItem->_coordX;
		float * pCoordY = ModelItem->_coordY;
		float * pEdgeX = ModelItem->_edgeDerivativeX;
		float * pMag = ModelItem->_edgeMagnitude;
		BYTE * pFlags = ModelItem->_Flags;

		cv::Mat img = pModel->Image();

		int ImgWid = img.cols;
		int ImgLen = img.rows;
		int j = SStart.x;
		int i = SStart.y;

		float half_W = (float)(ImgWid) * 0.5;
		float half_L = (float)(ImgLen) * 0.5;

		int nMinX = j - half_W + 1;
		int nMaxX = j + half_W + 2;
		int nMinY = i - half_L + 1;
		int nMaxY = i + half_L + 2;

		if (nMinX < 1)
			nMinX = 1;
		if (nMinY < 1)
			nMinY = 1;
		if (nMaxX >= Integ.cols)
			nMaxX = Integ.cols - 1;
		if (nMaxY >= Integ.rows)
			nMaxY = Integ.rows - 1;

		int SzBox = (nMaxX - nMinX) * (nMaxY - nMinY);
		int * nMinIteg = Integ.ptr<int>(nMinY);
		int * nMaxIteg = Integ.ptr<int>(nMaxY);
		float ImgMean = (double)(nMaxIteg[nMaxX] - nMinIteg[nMaxX] - nMaxIteg[nMinX] + nMinIteg[nMinX]) / (double)(SzBox);

		for (int m = 0; m < noOfCordinates; m++)
		{
			int curX = i + pCoordX[m];
			int curY = j + pCoordY[m];

			if (curX<0 || curY<0 || curX>Sdl.rows - 1 || curY>Sdl.cols - 1)
			{
				pResBuf[m] = 0.f;
				continue;
			}

			BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
			pResBuf[m] = (float)(_Sdl[curY]) - ImgMean;
		}
	}

	void GeoMatch_POCR::DrawContours(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth, bool wb)
	{
		if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
			return;

		int inspAlgo = GeoModel::enmMatchAlgo::agRelief;

		if (!wb)
			inspAlgo = GeoModel::enmMatchAlgo::agIntaglio;

		cv::Mat mdImg = model.Image();
		GeoModel newModel;
		newModel.Alloc(model.Image(), Res._Angle[0]);
		newModel.setMatchAlgo(inspAlgo);
		std::shared_ptr<GeoModel::Item> ItemPtr = newModel.MakeModel(newModel.Image(), cv::Mat(mdImg.rows, mdImg.cols, mdImg.type(), cv::Scalar(255)), 0, 0, 0, GeoModel::enmDepth::spOrg);


		float angle = model.getAngle() / 180.0f * PI;
		float sinth = sin(angle);
		float costh = cos(angle);
		if (0 < Res.getResCnt())
		{
			cv::Point pt(Res._Center_x[0] + 0.5f, Res._Center_y[0] + 0.5f);

			GeoModel::typItemPtr ModelItem;
			// 		GeoModel::typItemList::iterator iter = model._Models[GeoModel::enmDepth::spOrg].begin();
			// 		if(iter!=model._Models[GeoModel::enmDepth::spOrg].end())
			{
				// 			ModelItem = iter->second;
				ModelItem = ItemPtr;

				float * coordX = ModelItem->_coordX;
				float * coordY = ModelItem->_coordY;
				int * Id = ModelItem->_DivisionId;
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
						cv::line(source, point, point, cv::Scalar(10 * (Id[j])), lineWidth);
					else
						cv::line(source, point, point, color, lineWidth);
				}
			}
		}
	}

	void GeoMatch_POCR::_DrawContours(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth)
	{
		if (model.getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
			return;

		float angle = model.getAngle() / 180.0f * PI;
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
						cv::line(source, point, point, cv::Scalar(10 * Id[j]), lineWidth);
					else
						cv::line(source, point, point, color, lineWidth);
				}
			}
		}
	}

	void ImgProcessing_POCR::_ImgProcess(cv::Mat src, int modelStep, GeoModel::enmMatchAlgo MatchAlgo)
	{
		//	int use_algo_model = model.getMatchAlgo();
			// 영상 전처리
		cv::Mat diff_fSrc = _ImgPreprocess(src, GeoModel::enmMatchAlgo::agRelief);
		cv::Mat Edge_fSrc = _ImgPreprocess(src, GeoModel::enmMatchAlgo::agDefault);
		GeoModel model;
		// 영상의 해상도 축소
		cv::Mat Edge_img = Edge_fSrc, Edge_half, Edge_pyr, diff_img = diff_fSrc, diff_half, diff_pyr;
		int nPyrDownStep(0);
		do
		{
			Edge_pyr.release();
			_GetPyrDown(Edge_img, Edge_pyr);
			Edge_img = Edge_pyr;
			nPyrDownStep++;
			if (nPyrDownStep == 1)
				Edge_half = Edge_pyr;

		} while (nPyrDownStep < modelStep);

		*edge->_OrgImage = src;
		*diff->_OrgImage = src;
		*Img->_OrgImage = src;
		edge->_ImgProcess(Edge_fSrc, Edge_half, Edge_pyr, modelStep, GeoModel::enmMatchAlgo::agDefault);
		diff->_ImgProcess(diff_fSrc, diff_half, diff_pyr, modelStep, GeoModel::enmMatchAlgo::agRelief);
		Img->_ImgProcess(Edge_fSrc, Edge_half, Edge_pyr, modelStep, GeoModel::enmMatchAlgo::agImage);
		// 
		// 	int nSobelSz(3);
		// 
		// 	cv::Size OrgSsize = cv::Size(Edge_fSrc.cols, Edge_fSrc.rows);
		// 	cv::Size HalfSsize = cv::Size(Edge_half.cols, Edge_half.rows);
		// 	cv::Size ZoomSsize = cv::Size(Edge_pyr.cols, Edge_pyr.rows);
		// 
		// 	_ProcBuf[GeoModel::enmDepth::spOrg].Alloc(src);
		// 	_ProcBuf[GeoModel::enmDepth::spHalf].Alloc(Edge_half);
		// 	_ProcBuf[GeoModel::enmDepth::spZoom].Alloc(Edge_pyr);
		// 
		// 	if(MatchAlgo == GeoModel::enmMatchAlgo::agIntaglio || MatchAlgo == GeoModel::enmMatchAlgo::agRelief)
		// 	{
		// 		diff_fSrc.copyTo(_ProcBuf[GeoModel::enmDepth::spOrg].Sdl);
		// 		ProcIntegral(diff_fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Integral, _ProcBuf[GeoModel::enmDepth::spOrg].SqIntegral);
		// 
		// 		diff_half.copyTo(_ProcBuf[GeoModel::enmDepth::spHalf].Sdl);
		// 		ProcIntegral(diff_half, _ProcBuf[GeoModel::enmDepth::spHalf].Integral, _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral);
		// 		if(diff_pyr.rows != diff_half.rows)
		// 		{
		// 			diff_pyr.copyTo(_ProcBuf[GeoModel::enmDepth::spZoom].Sdl);
		// 			ProcIntegral(diff_pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Integral, _ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral);
		// 		}
		// 		else
		// 		{
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].Sdl = _ProcBuf[GeoModel::enmDepth::spHalf].Sdl;
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].Integral = _ProcBuf[GeoModel::enmDepth::spHalf].Integral;
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral = _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral;
		// 
		// 		}
		// 	}
		// 	else if(MatchAlgo == GeoModel::enmMatchAlgo::agDefault || MatchAlgo == GeoModel::enmMatchAlgo::agLowContrast)
		// 	{
		// 		cv::Sobel(Edge_fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Sdx, CV_16S, 1, 0, nSobelSz);
		// 		cv::Sobel(Edge_fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, CV_16S, 0, 1, nSobelSz);
		// #ifndef GeoNoUseSimd
		// 		_CalcMagnitude_sse(_ProcBuf[GeoModel::enmDepth::spOrg].Sdx, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, _ProcBuf[GeoModel::enmDepth::spOrg].matGradMag, OrgSsize);
		// #else
		// 		_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spOrg].Sdx, _ProcBuf[GeoModel::enmDepth::spOrg].Sdy, _ProcBuf[GeoModel::enmDepth::spOrg].matGradMag, OrgSsize);
		// #endif
		// 		
		// 
		// 		cv::Sobel(Edge_half, _ProcBuf[GeoModel::enmDepth::spHalf].Sdx, CV_16S, 1, 0, nSobelSz);
		// 		cv::Sobel(Edge_half, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, CV_16S, 0, 1, nSobelSz);
		// #ifndef GeoNoUseSimd
		// 		_CalcMagnitude_sse(_ProcBuf[GeoModel::enmDepth::spHalf].Sdx, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, _ProcBuf[GeoModel::enmDepth::spHalf].matGradMag, HalfSsize);
		// #else
		// 		_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spHalf].Sdx, _ProcBuf[GeoModel::enmDepth::spHalf].Sdy, _ProcBuf[GeoModel::enmDepth::spHalf].matGradMag, HalfSsize);
		// #endif
		// 		
		// 		if(Edge_pyr.rows != Edge_half.rows)
		// 		{
		// 			cv::Sobel(Edge_pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Sdx, CV_16S, 1, 0, nSobelSz);
		// 			cv::Sobel(Edge_pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, CV_16S, 0, 1, nSobelSz);
		// #ifndef GeoNoUseSimd
		// 			_CalcMagnitude_sse(_ProcBuf[GeoModel::enmDepth::spZoom].Sdx, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, _ProcBuf[GeoModel::enmDepth::spZoom].matGradMag, ZoomSsize);
		// #else
		// 			_CalcMagnitude(_ProcBuf[GeoModel::enmDepth::spZoom].Sdx, _ProcBuf[GeoModel::enmDepth::spZoom].Sdy, _ProcBuf[GeoModel::enmDepth::spZoom].matGradMag, ZoomSsize);
		// #endif
		// 		}
		// 		else
		// 		{
		// 			_ProcBuf[GeoModel::enmDepth::spZoom] = _ProcBuf[GeoModel::enmDepth::spHalf];
		// 		}
		// 	}
		// 	else
		// 	{
		// 		ProcIntegral(Edge_fSrc, _ProcBuf[GeoModel::enmDepth::spOrg].Integral, _ProcBuf[GeoModel::enmDepth::spOrg].SqIntegral);
		// 		ProcIntegral(Edge_half, _ProcBuf[GeoModel::enmDepth::spHalf].Integral, _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral);
		// 		if(Edge_half.rows != Edge_pyr.rows)
		// 			ProcIntegral(Edge_pyr, _ProcBuf[GeoModel::enmDepth::spZoom].Integral, _ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral);
		// 		else
		// 		{
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].Integral = _ProcBuf[GeoModel::enmDepth::spHalf].Integral;
		// 			_ProcBuf[GeoModel::enmDepth::spZoom].SqIntegral = _ProcBuf[GeoModel::enmDepth::spHalf].SqIntegral;
		// 		}
		// 	}
		//  
		//  	Edge_half.copyTo(*_half) ;
		//  	Edge_pyr.copyTo(*_pyr) ;
		//  	Edge_fSrc.copyTo(*_fSrc);
		//  	src.copyTo(*_OrgImage);
		bIsImgProcessed = true;
	}
