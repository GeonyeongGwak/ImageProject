#pragma once
//#include "Geo_Point.h"

namespace PIAL
{
	typedef struct PINSPALGO_API _MPTICoordinateBoard
	{
	public:
		double x;
		double y;

		_MPTICoordinateBoard()
		{
			x = 0.0;
			y = 0.0;
		}

		_MPTICoordinateBoard(double ix, double iy)
		{
			x = ix;
			y = iy;
		}

		BOOL IsValidate()
		{
			/*if ((x == 0.0) && (y == 0.0))
				return FALSE;*/

			return TRUE;
		}
	}_MCOORDI_B, *_LP_MCOORDI_B;

	typedef struct PINSPALGO_API _MPTICoordinateCorrector
	{
	public:
		double stdTheta;	// degree
		double curTheta;
		double theta;
		double offsetX;
		double offsetY;
		double deltaX;
		double deltaY;
		double scaleX;
		double scaleY;
		BOOL m_bGenerate;


		BOOL IsGenerate() const { return m_bGenerate; }
		void SetGenerate(BOOL bGenerate) { m_bGenerate = bGenerate; }

		_MPTICoordinateCorrector()
		{
			stdTheta = 0.0;
			curTheta = 0.0;
			theta = 0.0;
			offsetX = 0.0;
			offsetY = 0.0;
			deltaX = 0.0;
			deltaY = 0.0;
			scaleX = 1.0;
			scaleY = 1.0;
			SetGenerate(FALSE);
		}
	}_MCOORDI_COORECTOR, *_LP_MCOORDI_COORECTOR;


	class PINSPALGO_API FiducialCoord
	{
	public:
		FiducialCoord();
		~FiducialCoord();

		static _MCOORDI_B GetCenterPos(int nCnt, std::vector<_MCOORDI_B>& poArrPos);
		static POINTF GetCenterPos(int nCnt, std::vector<POINTF>& poArrPos);
		static double CalSigma(int nCnt, double* fArrX, int nPow = 1);
		static double CalSigma(int nCnt, double* fArrX, double* fArrY);
		static double CalTheta(int nCnt, double * dArrX_O, double * dArrY_O, double * dArrX_T, double * dArrY_T);

		static _MPTICoordinateCorrector CalcCorrect_ThetaDeltaScale(int nCnt, std::vector<_MCOORDI_B>& poArr_O, std::vector<_MCOORDI_B>& poArr_T, bool bScale = false, bool calcNew = false);
		static int CalcCorrect_ThetaDeltaScale(int nCnt, std::vector<_MCOORDI_B>& poArr_O, std::vector<_MCOORDI_B>& poArr_T, double *dTheta_ref, double *dDeltaX_ref, double *dDeltaY_ref, double * dScaleX_ref, double * dScaleY_ref, bool bScale = false);
		static int CorrectCoordinate_Cal3(_MCOORDI_B* poDst, _MPTICoordinateCorrector * lpCorrector, double dInversion);
		static int CorrectCoordinate_Cal3(_MCOORDI_B* poDst, double dDeltaX, double dDeltaY, double dTheta, double dScaleX, double dScaleY,_MCOORDI_B poFiduCenter, double dInversion);
	
		static int CalcCoordCorrector_new(int nCnt, std::vector<_MCOORDI_B>& poArr_O, std::vector<_MCOORDI_B>& poArr_T, double *dTheta_ref, double *dDeltaX_ref, double *dDeltaY_ref, double * dScaleX_ref, double * dScaleY_ref);
		static int CalcCoordCorrector_new(int nCnt, std::vector<POINTF>& poArr_O, std::vector<POINTF>& poArr_T, double *dTheta_ref, double *dDeltaX_ref, double *dDeltaY_ref, double * dScaleX_ref, double * dScaleY_ref);
	};

}
