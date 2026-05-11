#include "stdafx.h"
#include "algo_geometry.h"
#include <vector>
#include <tuple>
algo_geometry::algo_geometry()
{
}


algo_geometry::~algo_geometry()
{
}

double algo_geometry::DotProduct(POINTF pointA, POINTF pointB, POINTF pointC)
{
	POINTF AB;
	POINTF BC;
	AB.x = pointB.x - pointA.x;
	AB.y = pointB.y - pointA.y;
	BC.x = pointC.x - pointB.x;
	BC.y = pointC.y - pointB.y;
	double dot = AB.x * BC.x + AB.y * BC.y;

	return dot;
}

//Compute the cross product AB x AC
double algo_geometry::CrossProduct(POINTF pointA, POINTF pointB, POINTF pointC)
{
	POINTF AB;
	POINTF AC;
	AB.x = pointB.x - pointA.x;
	AB.y = pointB.y - pointA.y;
	AC.x = pointC.x - pointA.x;
	AC.y = pointC.y - pointA.y;
	double cross = AB.x * AC.y - AB.y * AC.x;

	return cross;
}

//Compute the distance from A to B
double algo_geometry::Distance(POINTF pointA, POINTF pointB)
{
	double d1 = pointA.x - pointB.x;
	double d2 = pointA.y - pointB.y;

	return sqrt(d1 * d1 + d2 * d2);
}

double algo_geometry::Distance(POINT pointA, POINT pointB)
{
	double d1 = pointA.x - pointB.x;
	double d2 = pointA.y - pointB.y;

	return sqrt(d1 * d1 + d2 * d2);
}

//Compute the distance from AB to C
//if isSegment is true, AB is a segment, not a line.
double algo_geometry::LineToPointDistance2D(POINTF pointA, POINTF pointB, POINTF pointC,
	bool isSegment)
{
	double dist = CrossProduct(pointA, pointB, pointC) / Distance(pointA, pointB);
	if (isSegment)
	{
		double dot1 = DotProduct(pointA, pointB, pointC);
		if (dot1 > 0)
			return Distance(pointB, pointC);

		double dot2 = DotProduct(pointB, pointA, pointC);
		if (dot2 > 0)
			return Distance(pointA, pointC);
	}
	return abs(dist);
}

double algo_geometry::RectToPointDistance2D(RECT rect, POINT point, BOOL & bContain)
{
	double dMin = DBL_MAX;
	double dValue = 0.;

	CRect rec = (CRect)rect;

	bContain = rec.PtInRect(point);

	POINTF pt = { (float)point.x , (float)point.y };

	POINTF ptTopLeft = { (float)rec.left, (float)rec.top };
	POINTF ptTopRight = { (float)rec.right, (float)rec.top };
	POINTF ptBottomLeft = { (float)rec.left, (float)rec.bottom };
	POINTF ptBottomRight = { (float)rec.right, (float)rec.bottom };

	dValue = LineToPointDistance2D(ptTopLeft, ptTopRight, pt, true);
	if (dMin > dValue) dMin = dValue;

	dValue = LineToPointDistance2D(ptTopRight, ptBottomRight, pt, true);
	if (dMin > dValue) dMin = dValue;

	dValue = LineToPointDistance2D(ptBottomRight, ptBottomLeft, pt, true);
	if (dMin > dValue) dMin = dValue;

	dValue = LineToPointDistance2D(ptBottomLeft, ptTopLeft, pt, true);
	if (dMin > dValue) dMin = dValue;

	return dValue;
}

int algo_geometry::RectCrossPoint(RECT* ptRect, int nNum, POINT point)
{
	int nIndex = -1;
	if (nNum == 0) return nIndex;
   
	typedef std::tuple<int, double> rectInfo;

	std::vector<rectInfo> vecInner;
	std::vector<rectInfo> vecOutter;
	BOOL bContain = false;
	
	for (int i = 0; i < nNum; i++)
	{
		double dValue = RectToPointDistance2D(ptRect[i], point, bContain);

		if (bContain) // 내부  박스
			vecInner.emplace_back(i, dValue);
		else
			vecOutter.emplace_back(i, dValue);
	}

	if (vecInner.size() > 0)
	{
		//가장 먼값
		double dValue = DBL_MIN;

		for (size_t i = 0; i < vecInner.size(); i++)
		{
			int index = std::get<0>(vecInner[i]);
			double distance = std::get<1>(vecInner[i]);

			if (dValue < distance)
				nIndex = index;
		}
	}
	else
	{
		//가장 가까운 값
		double dValue = DBL_MAX;

		for (size_t i = 0; i < vecOutter.size(); i++)
		{
			int index = std::get<0>(vecOutter[i]);
			double distance = std::get<1>(vecOutter[i]);

			if (dValue > distance)
				nIndex = index;
		}
	}

	return nIndex;
}