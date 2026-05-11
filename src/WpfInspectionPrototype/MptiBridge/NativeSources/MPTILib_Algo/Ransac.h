#pragma once

#include "datatype.h"
#include "LeastSquare.h"

class CRansac
{
public:
	CRansac(void);
	virtual ~CRansac(void);
public:
	void Run(std::vector< jsl::Point2d<double> > & data, std::vector< jsl::Point2d<double> > & outlier, std::vector<double> & coeff, bool bSaveOut=false, int nOrder=2, double w=0.5, double dDist=6, int nSampling=2, double nSuccessScore=90.0, int nMaxLoop=50);
	void Run2d();

	jsl::LeastSquare	m_LeastSquare;
};

