#pragma once

#include "datatype.h"
#include <vector>

namespace jsl
{
	typedef double typIV;
	typedef double typDV;
	typedef double typCoeff;

	class LeastSquare
	{
	public:
		LeastSquare(void);
		virtual ~LeastSquare(void);
	public:
		int		ValueCount2d(int nOrder);
		int		GetOrderFromCoeffCount2d(std::vector<double> & Coff);

		void	Run2d(int nOrder, std::vector< Point3d<double,double,double> > & Val, std::vector<double> & Coff);
		bool	Apply2d(int nOrder, std::vector< Point3d<double,double,double> > & Val, std::vector<double> & Coff);
		inline double	Apply2d(int nOrder, double IVx, double IVy, std::vector<double> & Coff);
		double Apply2d(double IVx, double IVy, std::vector<double> & Coff);

// 		void	Run(std::vector< Point2d<double> > & Val, std::vector<double> & Coff);
// 		bool	Apply(std::vector< Point2d<double> > & Val, std::vector<double> & Coff);
// 		inline double	Apply(double IV, std::vector<double> & Coff);

		void RunSvd(int nOrder, std::vector< Point2d<double> > & Val, std::vector<double> & Coff);
		double ApplySvd(double IVx, std::vector<double> & Coff);
		bool ApplySvd(std::vector< Point2d<double> > & Val, std::vector<double> & Coff);
	};
}

