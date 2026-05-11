#include "StdAfx.h"
#include "LeastSquare.h"
#include <opencv2\opencv.hpp>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif





namespace jsl
{
	LeastSquare::LeastSquare(void)
	{
	}

	LeastSquare::~LeastSquare(void)
	{
	}

	int LeastSquare::ValueCount2d(int nOrder)
	{
		int nStdOrder(2);
		int nStdValue(3);			// 1차원 변수
		int nIndValue(3);			// 2개의 독립변수를 가진다.
		int nValCount(nStdValue);

		for(int r=nStdOrder; r<=nOrder; r++)
			nValCount += (nIndValue+(r-nStdOrder));

		return nValCount;
	}

	int LeastSquare::GetOrderFromCoeffCount2d(std::vector<double> & Coff)
	{
		if(Coff.empty()==true)
			return 0;
		int i=0,n=0;
		
		while(n<Coff.size())
		{
			++i;
			n=ValueCount2d(i);
		}
		return n==Coff.size() ? i : 0;
	}

	void LeastSquare::RunSvd(int nOrder, std::vector< Point2d<double> > & Val, std::vector<double> & Coff)
	{
		int nInputCount(Val.size());
		int nValCount(nOrder+1);

		cv::Mat M	(nInputCount, nValCount, CV_64FC1);
		cv::Mat Mt	(nValCount, nInputCount, CV_64FC1);
		cv::Mat V	(nInputCount, 1, CV_64FC1);

		cv::Mat lum	(nValCount, nValCount, CV_64FC1);
		cv::Mat vec	(nValCount, 1, CV_64FC1);
		cv::Mat coe	(nValCount, 1, CV_64FC1);

		for(int i=0; i<nInputCount; i++)
		{
			int n(0);

			for(int r=nOrder; r>0; r--)
				M.ptr<double>(i)[n++] = pow(Val[i].x, (double)(r));

			M.ptr<double>(i)[n++] = 1;
			V.ptr<double>(i)[0] = Val[i].y;
		}

		cv::transpose(M, Mt);
		lum = Mt * M;
		vec = Mt * V;
		if(cv::solve(lum, vec, coe)==1)
		{
			Coff.resize(nValCount);
			for(int j=0; j<nValCount; j++)
				Coff[j] = coe.ptr<double>(j)[0];
		}
	}

	double LeastSquare::ApplySvd(double IVx, std::vector<double> & Coff)
	{
		if(Coff.empty()==true)
			return 0.0;

		int nOrder = Coff.size()-1;
		int n(0), v(0);
		double DVy(0);

		for(int r=nOrder; r>0; r--)
			DVy += Coff[n++] * pow(IVx, r);
		DVy += Coff[n++];

		return DVy;
	}

	bool LeastSquare::ApplySvd(std::vector< Point2d<double> > & Val, std::vector<double> & Coff)
	{
		for(int i=0; i<Val.size(); i++)
			Val[i].y=ApplySvd(Val[i].x, Coff);

		return true;
	}

	void LeastSquare::Run2d(int nOrder, std::vector< Point3d<double,double,double> > & Val, std::vector<double> & Coff)
	{
		int nInputCount(Val.size());
		int nValCount(ValueCount2d(nOrder));

		cv::Mat M	(nInputCount, nValCount, CV_64FC1);
		cv::Mat Mt	(nValCount, nInputCount, CV_64FC1);
		cv::Mat V	(nInputCount, 1, CV_64FC1);

		cv::Mat lum	(nValCount, nValCount, CV_64FC1);
		cv::Mat vec	(nValCount, 1, CV_64FC1);
		cv::Mat coe	(nValCount, 1, CV_64FC1);

		for(int i=0; i<nInputCount; i++)
		{
			int n(0);

			for(int r=nOrder; r>0; r--)
			{
				double dXval = pow(Val[i].x, (double)(r));
				double dYval = pow(Val[i].y, (double)(r));

				M.ptr<double>(i)[n++] = dXval;
				M.ptr<double>(i)[n++] = dYval;

				for (int e=r-1; e>0; e--)
				{
					int xr = e;
					int yr = r-e;

					double dval = pow(Val[i].x, (double)(xr)) * pow(Val[i].y, (double)(yr));
					M.ptr<double>(i)[n++] = dval;
				}
			}
			M.ptr<double>(i)[n++] = 1;
			V.ptr<double>(i)[0] = Val[i].z;
		}

		cv::transpose(M, Mt);
		lum = Mt * M;
		vec = Mt * V;
		if(cv::solve(lum, vec, coe)==1)
		{
			Coff.resize(nValCount);
			for(int j=0; j<nValCount; j++)
				Coff[j] = coe.ptr<double>(j)[0];
		}
	}

	bool LeastSquare::Apply2d(int nOrder, std::vector< Point3d<double,double,double> > & Val, std::vector<double> & Coff)
	{
		for(int i=0; i<Val.size(); i++)
			Val[i].z=Apply2d(nOrder, Val[i].x, Val[i].y, Coff);

		return true;
	}

	double LeastSquare::Apply2d(int nOrder, double IVx, double IVy, std::vector<double> & Coff)
	{
 		if(Coff.empty()==true)
 			return 0.0;

		int n(0), v(0);
		double DVz(0);
		for(int r=nOrder; r>0; r--)
		{
			DVz += Coff[n++] * pow(IVx, r);
			DVz += Coff[n++] * pow(IVy, r);
			for(int e=r-1; e>0; e--)
			{
				int xr = e;
				int yr = r-e;

				DVz += Coff[n++] * (pow(IVx, xr) * pow(IVy, yr));
			}
		}
		DVz += Coff[n++];

		return DVz;
	}

	double LeastSquare::Apply2d(double IVx, double IVy, std::vector<double> & Coff)
	{
		int nOrder = GetOrderFromCoeffCount2d(Coff);

		int n(0), v(0);
		double DVz(0);
		for(int r=nOrder; r>0; r--)
		{
			DVz += Coff[n++] * pow(IVx, r);
			DVz += Coff[n++] * pow(IVy, r);
			for(int e=r-1; e>0; e--)
			{
				int xr = e;
				int yr = r-e;

				DVz += Coff[n++] * (pow(IVx, xr) * pow(IVy, yr));
			}
		}
		DVz += Coff[n++];

		return DVz;
	}
 }
