#pragma once

#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

class CHistogramAnalysis
{
	friend class CDlgHistogram;
public:
	class PeakData
	{
	public:
		PeakData() :  nIndex(0), dValue(0.0), nStIdx(0), dStVal(0.0), nEdIdx(0), dEdVal(0.0), nMasterIdx(0), nPairIdx(0)
					, nStRmsIdx(0), dStRmsVal(0), nEdRmsIdx(0), dEdRmsVal(0), nUpIdx(0), dUpVal(0), nDnIdx(0), dDnVal(0)
					, dDnInt(0), dUpInt(0), dIndexSub(0.0)
		{

		}
		virtual ~PeakData()
		{

		}
	public:
		int		nIndex;
		double	dIndexSub;
		double	dValue;

		int		nStIdx;
		double	dStVal;
		int		nEdIdx;
		double	dEdVal;

		int		nStRmsIdx;
		double	dStRmsVal;
		int		nEdRmsIdx;
		double	dEdRmsVal;

		int		nUpIdx;
		double	dUpVal;
		double	dUpInt;
		int		nDnIdx;
		double	dDnVal;
		double	dDnInt;

		int		nMasterIdx;		// 하나의 산에 봉우리가 여러개
		int		nPairIdx;		// 두 산이 서로 붙어 있는 경우

		std::map<CString, double> addedParam;
	};

public:
	CHistogramAnalysis();
	CHistogramAnalysis(cv::Mat src);
	virtual ~CHistogramAnalysis(void);

public:
	void Export(CString sName);

	void PreProcess()
	{
		Reset();
		CalcHistogram();
		Normalization();

		if(m_nBlurSize==5)
			MovingAverage5();
		else
			MovingAverage3();

		DiffEqu();
		FindUpPeak();
		FindDnPeak();
		CorrectUpPeak();
		CorrectDnPeak();
		ReselectUpPeak();
		ReselectDnPeak();

		FindStEd_se(m_dStEdDiffThres);
	}
	void PreProcess(cv::Mat & hist)
	{
		m_nSizeHisto = hist.cols;

		Reset();
		// 		m_dMax = 0;
		// 		int * pHist = hist.ptr<int>();
		// 		for (int i = 0; i < m_nSizeHisto; i++)
		// 		{
		// 			m_HistogramOrg[i] = pHist[i];
		// 			m_dMax = __max(m_dMax, pHist[i]);
		// 		}
		// 		memcpy(&m_HistNorm[0], &m_HistogramOrg[0], sizeof(double)*m_nSizeHisto);
		// 		memcpy(&m_HistBlur[0], &m_HistogramOrg[0], sizeof(double)*m_nSizeHisto);

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

		DiffEqu();
		FindUpPeak();
		FindDnPeak();
		CorrectUpPeak();
		CorrectDnPeak();
		ReselectUpPeak();
		ReselectDnPeak();

		FindStEd_se(m_dStEdDiffThres);
	}
	void PreProcess(std::vector<int> & hist)
	{
		m_nSizeHisto = hist.size();

		Reset();
		m_dMax = 0;
		for(int i=0; i<m_nSizeHisto; i++)
		{
			m_HistogramOrg[i] = hist[i];
			m_dMax = __max(m_dMax, hist[i]);
		}
		Normalization();

		if(m_nBlurSize==5)
			MovingAverage5();
		else
			MovingAverage3();

		DiffEqu();
		FindUpPeak();
		FindDnPeak();
		CorrectUpPeak();
		CorrectDnPeak();
		ReselectUpPeak();
		ReselectDnPeak();

		FindStEd_se(m_dStEdDiffThres);
	}
	void PreProcess(BYTE * hist, int nSize)
	{
		m_nSizeHisto = nSize;

		Reset();
		m_dMax = 0;
		for(int i=0; i<m_nSizeHisto; i++)
		{
			m_HistogramOrg[i] = hist[i];
			m_dMax = __max(m_dMax, hist[i]);
		}
		Normalization();

		if(m_nBlurSize==5)
			MovingAverage5();
		else
			MovingAverage3();

		DiffEqu();
		FindUpPeak();
		FindDnPeak();
		CorrectUpPeak();
		CorrectDnPeak();
		ReselectUpPeak();
		ReselectDnPeak();

		FindStEd_se(m_dStEdDiffThres);
	}
	void PreProcess(std::vector<double> & hist)
	{
		m_nSizeHisto = hist.size();

		Reset();
		m_dMax = 0;
		for(int i=0; i<m_nSizeHisto; i++)
		{
			m_HistogramOrg[i] = hist[i];
			m_dMax = __max(m_dMax, hist[i]);
		}
		Normalization();

		if(m_nBlurSize==5)
			MovingAverage5();
		else
			MovingAverage3();

		DiffEqu();
		FindUpPeak();
		FindDnPeak();
		CorrectUpPeak();
		CorrectDnPeak();
		ReselectUpPeak();
		ReselectDnPeak();

		FindStEd_se(m_dStEdDiffThres);
	}
	virtual void PostProcess()
	{
		PeakAnalysis();
		FindMaster();
		CalcStEd_Rms();
		CalcUpDn();
	}

	// 파라메터
	int			m_nBlurSize;		// 3 또는 5픽셀 단위
	double		m_dRmsPos;			// 그 산의 실효 범위를 정할 최대값 기준 정규화 된 높이.
	double		m_dStEdDiffThres;	// 그 산의 최대 범위를 정할 최소 미분 값.
	double		m_dThDiffEqu;		// 작은산을 버릴 수 있지 않을까?
	double		m_dStdMaxValue;
	bool		m_bNormalized;

	int m_HistMaxIdx;
public:
	std::vector<PeakData>	m_PeakList;
	CString					m_sExportPath;

public:
	double					m_dZeroValueForMinMax;

	cv::Mat					m_image;

	double					m_dMax;
	int						m_nSizeHisto;
	std::vector<double>		m_HistogramOrg;
	std::vector<double>		m_HistNorm;
	std::vector<double>		m_HistBlur;
	std::vector<double>		m_HistDiffEqu;
	std::vector<double>		m_HistMax;
	std::vector<double>		m_HistMin;

//	std::vector<double>		m_DiffEquMinMax;
	std::vector<double>		m_DiffEquMin;
	std::vector<double>		m_DiffEquMax;

	std::vector<double>		m_DiffEquUpInt;
	std::vector<double>		m_DiffEquDnInt;

	std::vector<double>		m_HistStEd;
	std::vector<double>		m_HistStEdRms;

protected:
	virtual void Reset();

	// 전처리 항목
	void CalcHistogram();
	void Normalization();

	virtual void MovingAverage3();
	virtual void MovingAverage5();
	virtual void DiffEqu();

	void FindUpPeak();
	void FindDnPeak();

	void CorrectUpPeak();
	void CorrectDnPeak();
	void ReselectUpPeak();
	void ReselectDnPeak();

	void FindStEd(double dTh);
	void FindStEd_se(double dTh);

	// 후처리 항목
	void PeakAnalysis();
	void FindMaster();
	void CalcStEd_Rms();
	void CalcUpDn();
};

