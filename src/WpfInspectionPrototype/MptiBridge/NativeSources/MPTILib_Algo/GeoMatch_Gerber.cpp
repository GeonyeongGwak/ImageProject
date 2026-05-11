#include "StdAfx.h"
#include "GeoMatch_Gerber.h"
#include <opencv2/opencv.hpp>

GeoModel_Gerber::GeoModel_Gerber(void)
{
	_nPyraDownStepLimit = 2;
}


GeoModel_Gerber::~GeoModel_Gerber(void)
{
}

void GeoModel_Gerber::_GetPyrDown(cv::Mat src, cv::Mat & des)
{
	double scaleVal = 0.5;
	cv::resize(src, des, cv::Size(src.cols*scaleVal, src.rows*scaleVal), 0, 0, cv::INTER_LINEAR);
}

cv::Mat GeoModel_Gerber::_ImgPreProcess(cv::Mat src)
{
	cv::Mat SrcBlur;
	src.copyTo(SrcBlur);
	return SrcBlur;
}


GeoMatch_Gerber::GeoMatch_Gerber(void)
{
}


GeoMatch_Gerber::~GeoMatch_Gerber(void)
{
}

void GeoMatch_Gerber::FindModel_Gerb(GeoModel & Models, GeoResult & Result, double dLimitScore)
{
	FindModel(Models, Result,false);

	GeoModel::typItemList model = Models._Models[GeoModel::enmDepth::spOrg];

	// 한번 더 검사해서 포인트의 10% 이상이 0을 포함하는 경우 점수를 0으로 만든다
	for (size_t i = 0; i < Result.Count(); i++)
	{
		if(Result._Equality[i] < dLimitScore)
			continue;

		auto fndRes = model.find(Result._AngleStepId[i]);

		GeoModel::typItemPtr ModelItem = fndRes->second;

		cv::Mat Sdl = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdl;
		cv::Mat Sdx = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdx;
		cv::Mat Sdy = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdy;
		float * matGradMag = _ProcImg->_ProcBuf[ModelItem->_PyrDown].matGradMag;

		float normMinScore = 0.1 / ModelItem->_noOfCordinates;
		float normGreediness = ((1 - 0.9 * 0.1) / (1 - 0.9)) / ModelItem->_noOfCordinates;

		_DivResBuf.clear();
		_DivResBuf.resize(ModelItem->_noOfCordinates);

		_CalcCoefficient_DivSearch(Sdx, Sdy, matGradMag, cv::Point(Result._Center_y[i], Result._Center_x[i]), ModelItem, 0.0, 0.9);

		int nZeroCount(0);
		for (size_t j = 0; j < _DivResBuf.size(); j++)
		{
			if (_DivResBuf[j] == 0)
				nZeroCount++;
		}

		double Ratio(0);
		if(nZeroCount > 0)
			Ratio = (double)(nZeroCount) / (double)(ModelItem->_noOfCordinates);

		if (Ratio > 0.1)
			Result._Equality[i] = 0.0;
	}
}

void GeoMatch_Gerber::_AllResultProcedure(GeoModel::typItemPtr * pmdTmpList, GeoResult * resData, int nModelCount, GeoResult & result, int nResCnt)
{
	CAtlMap<double, int> angList;
	for (int i = 0; i < nModelCount; i++)
		angList.SetAt(pmdTmpList[i]->_Angle, i);

	// 1. 180도와 0도, 225도와 45도, 270도와 90도, 315도와 135도를 각각 찾아서 작은 각도에 중복된 데이터를 제거한다
	double FndList[4] = { 180.0, 225.0, 270.0, 315.0 };
	double DesList[4] = {   0.0,  45.0,  90.0, 135.0 };

	for (int a=0; a<4; a++)
	{
		bool bDest(false), bZero(false);
		int nFindId(0), nDestId(0), nZeroID(0);
		if (angList.Lookup(FndList[a], nFindId) == true && (bDest=angList.Lookup(DesList[a], nDestId)) == true)
		{
			for (int i = 0; i < nResCnt; i++)
			{
				for (int j = 0; j < nResCnt; j++)
				{
					float dx = fabs(resData[nFindId]._Center_x[i] - resData[nDestId]._Center_x[j]);
					float dy = fabs(resData[nFindId]._Center_y[i] - resData[nDestId]._Center_y[j]);
					float lx = pmdTmpList[nDestId]->_modelWidth * 0.5;
					float ly = pmdTmpList[nDestId]->_modelHeight * 0.5;

					if ( (dx < lx && dy < ly && (resData[nFindId]._Equality[i] < resData[nDestId]._Equality[j] || (resData[nDestId]._Equality[j] / resData[nFindId]._Equality[i]) > 0.85)) 
						)
					{
						resData[nFindId]._Equality[i] = 0.0f;
						break;
					}
				}
			}
		}
	}


	_AllResultProcedure(pmdTmpList, resData, nModelCount, result, nResCnt);
}