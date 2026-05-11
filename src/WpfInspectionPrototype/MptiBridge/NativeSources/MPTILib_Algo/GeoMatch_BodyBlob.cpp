#include "StdAfx.h"
#include "GeoMatch_BodyBlob.h"
#include <ppl.h>
#include "LeastSquare.h"

GeoMatch_BodyBlob::GeoMatch_BodyBlob(void)
{
}

GeoMatch_BodyBlob::~GeoMatch_BodyBlob(void)
{
}

void GeoMatch_BodyBlob::FindModel(GeoModel & model, GeoResult & result, int x, int y)
{
	x /= 2;
	y /= 2;

	GeoModel::typItemList ModelList = model._Models[GeoModel::enmDepth::spHalf];

	int nFoundAngle = result._AngleStepId[0];

	std::vector<int> mdTmpIdx;
	std::vector<GeoResult> mdTmpRes;
	std::vector<GeoModel::typItemPtr> mdTmpList;
	for (int i = 0; i < ModelList.size(); i+=1)
	{
		GeoModel::typItemList::iterator iter = ModelList.find(i);
		if(iter != ModelList.end())
		{
			mdTmpIdx.emplace_back(iter->first);
			mdTmpList.emplace_back(iter->second);
		}
	}

	mdTmpRes.resize(mdTmpList.size());
	GeoModel::typItemPtr * pmdTmpList = &mdTmpList[0];

	int nTargetImgW = _ProcImg->_half->cols;
	int nTargetImgL = _ProcImg->_half->rows;

#ifndef GeoNoUseMultiThread
	Concurrency::parallel_for((size_t)0, mdTmpList.size(), [&](size_t i)
#else
	for (int i=0; i<mdTmpList.size(); i++)
#endif
	{
		mdTmpRes[i].Alloc(1);

		_FindModel(pmdTmpList[i], mdTmpIdx[i], cv::Point(x,y), cv::Size(1,1), model.getAngle(), 0.0, 0.9, mdTmpRes[i], false, model.getMatchAlgo());
	}
#ifndef GeoNoUseMultiThread
	);
#endif
	// 각도와 점수를 fitting해서 실제 각도를 구한다.
	{
		jsl::LeastSquare ls;
		std::vector< jsl::Point2d<double> > Val;
		for (int i=0; i<mdTmpRes.size(); i++)
			Val.emplace_back( jsl::Point2d<double>(mdTmpRes[i]._Angle[0], mdTmpRes[i]._Equality[0]) );

		std::vector<double> Coff;
		ls.RunSvd(2, Val, Coff);
		if(Coff.empty() == false)
		{
			result._Angle[0] = -Coff[1] / (2.0 * Coff[0]);
		}
	}
}

void GeoMatch_BodyBlob::FindModelAtPos(GeoModel & model, GeoResult & result, float x, float y)
{
	int nAngleStep = model.getAngleStep();


	{
		// 여기서 주변 각도 별로 탐색한다.
		GeoModel::typItemList ModelList = model._Models[GeoModel::enmDepth::spOrg];

		//	int nFoundAngle = result._AngleStepId[0];

		std::vector<int> mdTmpIdx;
		std::vector<GeoResult> mdTmpRes;
		std::vector<GeoModel::typItemPtr> mdTmpList;
	//	for (int i = nFoundAngle-nAngleStep; i <= (nFoundAngle+nAngleStep); i++)
		for (int i = 0; i < ModelList.size(); i++)
		{
			GeoModel::typItemList::iterator iter = ModelList.find(i);
			if(iter != ModelList.end())
			{
				mdTmpIdx.emplace_back(iter->first);
				mdTmpList.emplace_back(iter->second);
			}
		}

		int cx = (int)(x + 0.49999);	//(int)(result._Center_y[0] + 0.4999);
		int cy = (int)(y + 0.49999);	//(int)(result._Center_x[0] + 0.4999);

		mdTmpRes.resize(mdTmpList.size());
		GeoModel::typItemPtr * pmdTmpList = &mdTmpList[0];
		for (int i=0; i<mdTmpList.size(); i++)
		{
			mdTmpRes[i].Alloc(1);
			_FindModel(pmdTmpList[i], mdTmpIdx[i], cv::Point(cx-4,cy-4), cv::Size(8,8), model.getAngle(), 0.0, 0.9, mdTmpRes[i], false, model.getMatchAlgo());
		}

		float maxScore(-HUGE_VAL), posX(0), posY(0);
		for (int i=0; i<mdTmpRes.size(); i++)
		{
			if(maxScore < mdTmpRes[i]._Equality[0])
			{
				maxScore = mdTmpRes[i]._Equality[0];
				posX = mdTmpRes[i]._Center_y[0];
				posY = mdTmpRes[i]._Center_x[0];
			}
		}
		result._Equality[0] = maxScore;
		result._Center_x[0] = posY;
		result._Center_y[0] = posX;

		// 각도와 점수를 fitting해서 실제 각도를 구한다.
		{
			jsl::LeastSquare ls;
			std::vector< jsl::Point2d<double> > Val;
			for (int i=0; i<mdTmpRes.size(); i++)
				Val.emplace_back( jsl::Point2d<double>(mdTmpRes[i]._Angle[0], mdTmpRes[i]._Equality[0]) );

			std::vector<double> Coff;
			ls.RunSvd(2, Val, Coff);
			if(Coff.empty() == false)
				result._Angle[0] = -Coff[1] / (2.0 * Coff[0]);
		}
	}
}