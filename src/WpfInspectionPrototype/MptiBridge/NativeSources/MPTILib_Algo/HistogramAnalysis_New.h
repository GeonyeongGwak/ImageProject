#pragma once
#include "HistogramAnalysis.h"

class CHistogramAnalysis_New :
	public CHistogramAnalysis
{
public:
	typedef enum _ePointType { None, Peak = 1, Start = 2, End = 4, Body = 8 } ePointType;

public:
	CHistogramAnalysis_New();
	virtual ~CHistogramAnalysis_New();

public:
	void Run(cv::Mat & hist, int nBlurCnt=1)
	{
		m_nSizeHisto = hist.cols;

		Reset();

		m_dMax = 0;
		int * pHist = hist.ptr<int>();
		for (int i = 0; i < m_nSizeHisto; i++)
		{
			m_HistogramOrg[i] = pHist[i];
			m_dMax = __max(m_dMax, pHist[i]);
		}
		Normalization();

		if (m_nBlurSize == 5)
			MovingAverage5();
		else
			MovingAverage3();

		for(int j=1; j< nBlurCnt; j++)
		{
			for (int i = 0; i < m_nSizeHisto; i++)
			{
				m_HistNorm[i] += m_HistBlur[i];
			}
			if (m_nBlurSize == 5)
				MovingAverage5();
			else
				MovingAverage3();
		}

		DiffEqu();
		CheckCrossPoints();
	}

protected:
	virtual void Reset();
	virtual void CheckCrossPoints();

	std::vector<int>		m_HistPeak;
};

