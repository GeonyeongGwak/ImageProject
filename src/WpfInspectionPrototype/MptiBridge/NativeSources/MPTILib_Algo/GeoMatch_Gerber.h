#pragma once
#include "geomatch.h"

class GeoModel_Gerber : public GeoModel
{
public:
	GeoModel_Gerber();
	virtual ~GeoModel_Gerber();
protected:
	virtual void _GetPyrDown(cv::Mat src, cv::Mat & des);
	virtual cv::Mat _ImgPreProcess(cv::Mat src);
};

class GeoMatch_Gerber : public GeoMatch
{
public:
	GeoMatch_Gerber(void);
	virtual ~GeoMatch_Gerber(void);

public:
	void FindModel_Gerb(GeoModel & Models, GeoResult & Result, double dLimitScore);

protected:
	virtual void _AllResultProcedure(GeoModel::typItemPtr * pmdTmpList, GeoResult * resData, int nModelCount, GeoResult & result, int nResCnt);
};

