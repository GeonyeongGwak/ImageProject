#include "StdAfx.h"
#include "HistogramAnalysis.h"
#include <fstream>

CHistogramAnalysis::CHistogramAnalysis(cv::Mat src)
	: m_nSizeHisto(256)
	, m_image(src)
	, m_nBlurSize(5)
	, m_dZeroValueForMinMax(0)
	, m_sExportPath(_T("d:\\"))
	, m_dStEdDiffThres(0.02)
	, m_dRmsPos(0.05)
	, m_dThDiffEqu(2)
	, m_dStdMaxValue(255)
	, m_bNormalized(true)
{
	Reset();
}

CHistogramAnalysis::CHistogramAnalysis()
	: m_nSizeHisto(256)
	, m_nBlurSize(5)
	, m_dZeroValueForMinMax(0)
	, m_sExportPath(_T("d:\\"))
	, m_dStEdDiffThres(0.02)
	, m_dRmsPos(0.125)
	, m_dThDiffEqu(2)
	, m_dStdMaxValue(255)
	, m_bNormalized(false)
{

}

CHistogramAnalysis::~CHistogramAnalysis(void)
{
}

void CHistogramAnalysis::Reset()
{
	m_HistogramOrg.clear();
	m_HistNorm.clear();
	m_HistBlur.clear();
	m_HistDiffEqu.clear();
	m_HistMax.clear();
	m_HistMin.clear();
	m_HistStEd.clear();
	m_HistStEdRms.clear();
	m_DiffEquMin.clear();
	m_DiffEquMax.clear();
	m_DiffEquUpInt.clear();
	m_DiffEquDnInt.clear();

	m_HistogramOrg.resize(m_nSizeHisto);
	m_HistNorm.resize(m_nSizeHisto);
	m_HistBlur.resize(m_nSizeHisto);
	m_HistDiffEqu.resize(m_nSizeHisto);
	m_HistMax.resize(m_nSizeHisto);
	m_HistMin.resize(m_nSizeHisto);
	m_HistStEd.resize(m_nSizeHisto);
	m_HistStEdRms.resize(m_nSizeHisto);
	m_DiffEquMin.resize(m_nSizeHisto);
	m_DiffEquMax.resize(m_nSizeHisto);
	m_DiffEquUpInt.resize(m_nSizeHisto);
	m_DiffEquDnInt.resize(m_nSizeHisto);

	//memset(&m_HistogramOrg[0], 0, sizeof(double)*m_nSizeHisto);
	//memset(&m_HistNorm[0], 0, sizeof(double)*m_nSizeHisto);
	//memset(&m_HistBlur[0], 0, sizeof(double)*m_nSizeHisto);
	//memset(&m_HistDiffEqu[0], 0, sizeof(double)*m_nSizeHisto);
	//memset(&m_DiffEquMin[0], 0, sizeof(double)*m_nSizeHisto);
	//memset(&m_DiffEquMax[0], 0, sizeof(double)*m_nSizeHisto);
	//memset(&m_DiffEquUpInt[0], 0, sizeof(double)*m_nSizeHisto);
	//memset(&m_DiffEquDnInt[0], 0, sizeof(double)*m_nSizeHisto);

	for(int i=0; i<m_nSizeHisto; i++)
	{
		m_HistMax[i] = m_dZeroValueForMinMax;
		m_HistMin[i] = m_dZeroValueForMinMax;
		m_HistStEd[i] = m_dZeroValueForMinMax;
		m_HistStEdRms[i] = m_dZeroValueForMinMax;
		m_DiffEquMin[i] = m_dZeroValueForMinMax;
		m_DiffEquMax[i] = m_dZeroValueForMinMax;
		m_DiffEquUpInt[i] = m_dZeroValueForMinMax;
		m_DiffEquDnInt[i] = m_dZeroValueForMinMax;
	}
}

void CHistogramAnalysis::CalcHistogram()
{
	std::vector<int>		histn(m_nSizeHisto);
	memset(&histn[0], 0, sizeof(int)*histn.size());

// 	MIL_ID HistResult = MimAllocResult(M_DEFAULT_HOST, m_nSizeHisto, M_HIST_LIST, M_NULL);
// 	MimHistogram(m_image->Handle, HistResult);
// 	MimGetResult(HistResult, M_VALUE, &histn[0]);
// 	MimFree(HistResult);

	int w=m_image.cols;
	int h=m_image.rows;
	for(int y=0; y<h; y++)
	{
		BYTE * ptr = m_image.ptr(y);
		for(int x=0; x<w; x++)
		{
			histn[ptr[x]]++;
		}
	}

	m_dMax = 0.0;
	for(int i=0; i<m_nSizeHisto; i++)
	{
		m_HistogramOrg[i] = histn[i];

		m_dMax = __max(m_dMax, m_HistogramOrg[i]);
	}
}

void CHistogramAnalysis::Normalization()
{
	if(m_bNormalized==true)
	{
		double nMaxValue = m_dStdMaxValue;
		for(int i=0; i<m_nSizeHisto; i++)
			m_HistNorm[i] = nMaxValue - ( ((m_dMax - m_HistogramOrg[i]) / m_dMax) * nMaxValue );
	}
	else
	{
		for(int i=0; i<m_nSizeHisto; i++)
			m_HistNorm[i] = m_HistogramOrg[i];
	}
}

void CHistogramAnalysis::DiffEqu()
{
	{
		int j(0);

		j=0;
		m_HistDiffEqu[j] = m_HistNorm[j+1] - m_HistNorm[j];

		j=m_nSizeHisto-1;
		m_HistDiffEqu[j] = m_HistNorm[j] - m_HistNorm[j-1];

		for(int i=1; i<(m_nSizeHisto-1); i++)
			m_HistDiffEqu[i] = m_HistNorm[i+1] - m_HistNorm[i-1];
	}
}

void CHistogramAnalysis::MovingAverage3()
{
	{
		int i(0);

		i = 0;
		m_HistBlur[0]	= (m_HistNorm[i] + m_HistNorm[i+1]) / 2.0;
		i = m_nSizeHisto-1;
		m_HistBlur[i]	= (m_HistNorm[i-1] + m_HistNorm[i-2]) / 2.0;
	}

	for(int i=1; i<(m_nSizeHisto-1); i++)
		m_HistBlur[i] = (m_HistNorm[i-1] + m_HistNorm[i] + m_HistNorm[i+1]) / 3.0;

	memcpy(&m_HistogramOrg[0], &m_HistNorm[0], sizeof(double)*m_nSizeHisto);
	memcpy(&m_HistNorm[0], &m_HistBlur[0], sizeof(double)*m_nSizeHisto);
}

void CHistogramAnalysis::MovingAverage5()
{
// 	{
// 		int i(0);
// 
// 		i = 0;
// 		m_HistBlur[0]	= (m_HistNorm[i] + m_HistNorm[i+1]*0.5 + m_HistNorm[i+2]*0.25) / 1.75;
// 		i = 1;
// 		m_HistBlur[i]	= (m_HistNorm[i-1]*0.5 + m_HistNorm[i] + m_HistNorm[i+1]*0.5 + m_HistNorm[i+2]*0.25) / 2.25;
// 		i = m_nSizeHisto-1;
// 		m_HistBlur[i]	= (m_HistNorm[i-2]*0.25 + m_HistNorm[i-1]*0.5 + m_HistNorm[i]) / 1.75;
// 		i = m_nSizeHisto-2;
// 		m_HistBlur[i]	= (m_HistNorm[i-2]*0.25 + m_HistNorm[i-1]*0.5 + m_HistNorm[i] + m_HistNorm[i+1]*0.5) / 2.25;
// 	}
// 	
// 	for(int i=2; i<(m_nSizeHisto-2); i++)
// 		m_HistBlur[i] = (m_HistNorm[i-2]*0.25 + m_HistNorm[i-1]*0.5 + m_HistNorm[i] + m_HistNorm[i+1]*0.5 + m_HistNorm[i+2]*0.25) / 2.5;
	m_HistMaxIdx = 0;
	double fMaxHistValue(0.0f);
	int nRange(4);
//	for(int i=nRange; i<(m_nSizeHisto-nRange); i++)
	for(int i=0; i<m_nSizeHisto; i++)
	{
		int nCnt(0);
		for(int j=-nRange; j<=nRange; j++)
		{
			int idx = i+j;
			if(idx<0 || idx>=m_nSizeHisto)
				continue;
			m_HistBlur[i] += m_HistNorm[i+j];
			nCnt++;
		}
		m_HistBlur[i] /= nCnt;//(nRange*2+1);
		if (m_HistBlur[i] > fMaxHistValue)
		{
			fMaxHistValue = m_HistBlur[i];
			m_HistMaxIdx = i;
		}
	}
	memcpy(&m_HistogramOrg[0], &m_HistNorm[0], sizeof(double)*m_nSizeHisto);
	memcpy(&m_HistNorm[0], &m_HistBlur[0], sizeof(double)*m_nSizeHisto);
}

void CHistogramAnalysis::FindUpPeak()
{
	bool	bFoundUp(false), bFoundDown(false), bFoundZero(false), bFoundOrigin(false);
	int		nIndexUp(0), nIndexDown(0);
	int		nIndexUpSt(0), nIndexDnEd(0);
	double	dUpMax(0), dDownMax(0);

	std::vector<int>	nIndexZero;
	std::vector<double>	dZeroValue;

	for(int i=0; i<m_nSizeHisto; i++)
	{
		// 오름산을 찾는다.
		// 너무 낮은 언덕일 경우 
		if(bFoundUp==false && m_HistDiffEqu[i] > 0)
		{
			dUpMax		= m_HistDiffEqu[i];
			nIndexUpSt	= i;
			nIndexUp	= i;
			bFoundUp	= true;
			bFoundDown	= false;
			bFoundZero	= false;
			bFoundOrigin= false;

			if(m_HistNorm[i]==0)
			{
				// 처음 산이 시작되는 부분에 시작점을 표시.
				m_HistMin[i] = 1;
				m_HistStEd[i] = 1;
			//	if((i+1) < m_nSizeHisto && m_HistNorm[i]>=0)
			//		m_HistMin[i+1+2] = m_HistogramOrg[i+1+2];
			}
		}
		else if(bFoundUp==true && bFoundDown==false)
		{
			if(m_HistDiffEqu[i] < 0 || (i+1)==m_nSizeHisto)
			{
				dDownMax	= m_HistDiffEqu[i];
				nIndexDown	= i;
				bFoundDown	= true;
			}
// 			else if(bFoundZero==true && m_HistDiffEqu[i] > 0)
// 			{
// 				// 등산하다가 생기는 평지
// 				dUpMax		= m_HistDiffEqu[i];
// 				nIndexUp	= i;
// 				bFoundUp	= true;
// 				bFoundDown	= false;
// 				bFoundZero	= false;
// 			}
// 			else if(m_HistDiffEqu[i]==0)
// 			{
// 				bFoundZero	= true;
// 			}
			else
			{
				// 최고 미분값을 찾는다
				if(m_HistDiffEqu[i] > dUpMax)
				{
					nIndexUp= i;
					dUpMax	= m_HistDiffEqu[i];
				}
			}
		}

		if(bFoundUp==true && bFoundDown==true)
		{
			if(bFoundOrigin==false)
			{
				if(	(m_HistDiffEqu[i] >= 0)												|| 
					((i+1)<m_nSizeHisto && m_HistDiffEqu[i+1] >= 0)						||
					(m_HistDiffEqu[i] == 0 && m_HistNorm[i]==0)							||
					((i+1)<m_nSizeHisto && m_HistDiffEqu[i+1]==0 && m_HistNorm[i+1]==0)	)
				{
					if(m_HistDiffEqu[i] < dDownMax)
					{
						nIndexDown	= i;
						dDownMax	= m_HistDiffEqu[i];
					}

					if(m_HistDiffEqu[i] >= 0)
						nIndexDnEd = i-1;
					else
						nIndexDnEd = i;

					m_HistMin[i] = 1;
					m_HistStEd[i] = 1;

					bFoundUp = false;
					bFoundOrigin = true;

					if( m_dThDiffEqu > fabs(dUpMax) && m_dThDiffEqu > fabs(dDownMax) )
						continue;

					m_DiffEquMax[nIndexUp] = dUpMax;
					m_DiffEquMin[nIndexDown] = dDownMax;

					// 여기서 원점 찾는 작업을 해야한다.
					double dZero(m_HistNorm[nIndexUp]);
					for(int j=nIndexUp; j<=nIndexDown; j++)
					{
						if(fabs(m_HistNorm[j]) > fabs(dZero))
						{
							dZero = m_HistNorm[j];
						}
					}

					// 최소값과 같은 값이 여러개면 그곳에 다 표시한다.
					for(int j=nIndexUp; j<=nIndexDown; j++)
					{
						if(fabs(m_HistNorm[j]) >= fabs(dZero))
						{
						//	m_HistMax[nIndexUpSt] = m_HistNorm[nIndexUpSt];
						//	m_HistMax[nIndexDnEd] = m_HistNorm[nIndexDnEd];
							m_HistMax[j] = m_HistNorm[j];
						}
					}

					double dUpInt(0), dDnInt(0);
					for(int j=nIndexUpSt; j<=nIndexDnEd; j++)
					{
						if(m_HistDiffEqu[j] > 0.0)
							dUpInt += m_HistDiffEqu[j];

						if(m_HistDiffEqu[j] < 0.0)
							dDnInt += m_HistDiffEqu[j];
					}

					m_DiffEquUpInt[nIndexUp]  = sqrt(4.0*dUpInt/3.14);
					m_DiffEquDnInt[nIndexDown]= -sqrt(4.0*fabs(dDnInt)/3.14);
				}
				else
				{
					if(m_HistDiffEqu[i] < dDownMax)
					{
						nIndexDown	= i;
						dDownMax	= m_HistDiffEqu[i];
					}
				}
			}
		}
	}
}

void CHistogramAnalysis::FindDnPeak()
{
	bool	bFoundUp(false), bFoundDown(false), bFoundZero(false), bFoundOrigin(false);
	int		nIndexUp(0), nIndexDown(0);
	double	dUpMax(0), dDownMax(0);

	std::vector<int>	nIndexZero;
	std::vector<double>	dZeroValue;

//	if(m_HistNorm[0] != 0)
		m_HistMin[0] = m_HistNorm[0];

//	if(m_HistNorm[m_nSizeHisto-1] != 0)
		m_HistMin[m_nSizeHisto-1] = m_HistNorm[m_nSizeHisto-1];

	for(int i=0; i<m_nSizeHisto; i++)
	{
		// 내림산을 찾는다.
		if(bFoundDown==false && m_HistDiffEqu[i] < 0)
		{
			dDownMax	= m_HistDiffEqu[i];
			nIndexDown	= i;
			bFoundDown	= true;
			bFoundUp	= false;
			bFoundZero	= false;
			bFoundOrigin= false;
		}
		// 찾은 내림산에서 다시 오름산을 찾는다
		else if(bFoundDown==true && bFoundUp==false)
		{
			// 다시 오름산을 만났는데 마지막 데이터라면...
			if(m_HistDiffEqu[i] > 0 || (i+1)==m_nSizeHisto)
			{
				dUpMax	 = m_HistDiffEqu[i];
				nIndexUp = i;
				bFoundUp = true;
			}
			// 영점을 만났다가 다시 내려가면...
			else if(bFoundZero==true && m_HistDiffEqu[i] < 0)
			{
				dDownMax	= m_HistDiffEqu[i];
				nIndexDown	= i;
				bFoundDown	= true;
				bFoundUp	= false;
				bFoundZero	= false;
			}
			// 영점을 만나면...
			else if((i+(m_nBlurSize/2+2))<m_nSizeHisto		&& 
					m_HistDiffEqu[i+(m_nBlurSize/2+2)]==0	&& 
					m_HistNorm[i+(m_nBlurSize/2+1)]==0.0	)
			{
				// 해당 산은 끝난 것이다.
				dUpMax	 = m_HistDiffEqu[i];
				nIndexUp = i;
				bFoundUp = true;
			}
			else
			{
				// 현재 최소값보다 더 작은 값이 있으면 갱신.
				if(m_HistDiffEqu[i] < dDownMax)
				{
					nIndexDown = i;
					dDownMax = m_HistDiffEqu[i];
				}
			}
		}

		// 둘 다 찾았다면 교차점을 찾는다.
		if(bFoundDown==true && bFoundUp==true)
		{
			// 교차점을 찾지 못했다면...
			if(bFoundOrigin==false)
			{
				// 다시 내림산을 만났다면 이제 교차점을 찾는다.
				if(	(m_HistDiffEqu[i] <= 0)																									||
					((i+1)<m_nSizeHisto && m_HistDiffEqu[i+1] <= 0)																			||
					((i+(m_nBlurSize/2+2))<m_nSizeHisto && m_HistDiffEqu[i+(m_nBlurSize/2+2)]==0 && m_HistNorm[i+(m_nBlurSize/2+1)]==0.0)	)
				{
					if(m_HistDiffEqu[i] > dUpMax)
					{
						nIndexUp= i;
						dUpMax	= m_HistDiffEqu[i];
					}

					// 0이 아닌 지점으로 되돌아가자.
					bFoundDown = false;
					bFoundOrigin = true;

					if( m_dThDiffEqu > fabs(dUpMax) && m_dThDiffEqu > fabs(dDownMax) )
						continue;

					// 여기서 원점 찾는 작업을 해야한다.
					double dZero(m_HistNorm[nIndexDown]);
					for(int j=nIndexDown; j<=nIndexUp; j++)
					{
						if(fabs(m_HistNorm[j])>0.0 && fabs(m_HistNorm[j]) < fabs(dZero))
						{
							dZero = m_HistNorm[j];
						}
					}

					// 최소값과 같은 값이 여러개면 그곳에 다 표시한다.
					for(int j=nIndexDown; j<=nIndexUp; j++)
					{
						if(fabs(m_HistNorm[j])>0.0 && fabs(m_HistNorm[j]) <= fabs(dZero))
						{
							m_HistMin[j] = m_HistNorm[j];
						}
					}
				}
				else
				{
					// 아직 오르는 중이라면 최대값을 갱신한다.
					if(m_HistDiffEqu[i] > dUpMax)
					{
						nIndexUp= i;
						dUpMax	= m_HistDiffEqu[i];
					}
				}
			}
		}
	}
}

void CHistogramAnalysis::CorrectUpPeak()
{
	bool	bStart(false), bEnd(false);
	int		nIndexSt(0), nIndexEd(0);
	for(int i=1; i<(m_nSizeHisto-1); i++)
	{
		if(bStart==false && m_HistMax[i]!=m_dZeroValueForMinMax)
		{
			bStart = true;
			nIndexSt = i;
		}

		if(bStart==true && bEnd==false && m_HistMax[i]==m_dZeroValueForMinMax)
		{
			bEnd = true;
			nIndexEd = i-1;
		}

		if(bStart==true && bEnd==true)
		{
			bStart=false;
			bEnd=false;

			if((nIndexEd-nIndexSt+1) >= 3)
			{
				int nSz = (nIndexEd-nIndexSt+1);
				int nCenterSt(0), nCenterEd(0);

				if((nSz%2) > 0)
				{
					nCenterSt = nIndexSt + (nIndexEd-nIndexSt+1) / 2;
					nCenterEd = nCenterSt;
				}
				else
				{
					nCenterSt= nIndexSt + (nIndexEd-nIndexSt) / 2;
					nCenterEd = nCenterSt+1;
				}

				for(int j=nIndexSt; j<=nIndexEd; j++)
				{
					if(j < nCenterSt || j > nCenterEd)
						m_HistMax[j] = m_dZeroValueForMinMax;
				}
			}
			else
			{
				bStart=false;
				bEnd=false;
			}
		}
	}
}

void CHistogramAnalysis::CorrectDnPeak()
{
	bool	bStart(false), bEnd(false);
	int		nIndexSt(0), nIndexEd(0);
	for(int i=1; i<(m_nSizeHisto-1); i++)
	{
		if(bStart==false && m_HistMin[i]!=m_dZeroValueForMinMax)
		{
			bStart = true;
			nIndexSt = i;
		}

		if(bStart==true && bEnd==false && m_HistMin[i]==m_dZeroValueForMinMax)
		{
			bEnd = true;
			nIndexEd = i-1;
		}

		if(bStart==true && bEnd==true)
		{
			bStart=false;
			bEnd=false;

			if((nIndexEd-nIndexSt+1) >= 3)
			{
				int nSz = (nIndexEd-nIndexSt+1);
				int nCenterSt(0), nCenterEd(0);

				if((nSz%2) > 0)
				{
					nCenterSt = nIndexSt + (nIndexEd-nIndexSt+1) / 2;
					nCenterEd = nCenterSt;
				}
				else
				{
					nCenterSt= nIndexSt + (nIndexEd-nIndexSt) / 2;
					nCenterEd = nCenterSt+1;
				}

				for(int j=nIndexSt; j<=nIndexEd; j++)
				{
					if(j < nCenterSt || j > nCenterEd)
						m_HistMin[j] = m_dZeroValueForMinMax;
				}
			}
			else
			{
				bStart=false;
				bEnd=false;
			}
		}
	}
}

void CHistogramAnalysis::ReselectUpPeak()
{
	for(int i=1; i<(m_nSizeHisto-1); i++)
	{
		if(m_HistMax[i] != m_dZeroValueForMinMax)
		{
			int nMaxIdx = i;
			double dMax = m_HistogramOrg[i];

			m_HistMax[i] = m_dZeroValueForMinMax;
			for(int j=i-1; j<=(i+1); j++)
			{
				if(m_HistogramOrg[j] > dMax)
				{
					nMaxIdx = j;
					dMax	= m_HistogramOrg[j];
				}
			}

			m_HistMax[nMaxIdx] = dMax;
		}
	}
}

void CHistogramAnalysis::ReselectDnPeak()
{
	for(int i=1; i<(m_nSizeHisto-1); i++)
	{
		if(m_HistMin[i] != m_dZeroValueForMinMax)
		{
			int nMinIdx = i;
			double dMin = m_HistogramOrg[i];

			if(m_HistogramOrg[i]==0 && m_HistogramOrg[i-1]==0 && m_HistogramOrg[i+1]==0)
				continue;

			m_HistMin[i] = m_dZeroValueForMinMax;
			for(int j=i-1; j<=(i+1); j++)
			{
				if(m_HistogramOrg[j]!=0.0 && m_HistogramOrg[j] < dMin)
				{
					nMinIdx = j;
					dMin	= m_HistogramOrg[j];
				}
				else if(m_HistogramOrg[j]==0.0 && m_HistNorm[j] < dMin)
				{
					nMinIdx = j;
					dMin	= m_HistNorm[j];
				}
			}

			m_HistMin[nMinIdx] = dMin;
		}
	}
}

void CHistogramAnalysis::Export(CString sName)
{
#ifdef _DEBUG
	CStringA sNameA, sPathA;
	sNameA = CT2CA(sName);
	sPathA = CT2CA(m_sExportPath);
	CStringA sPath;
	sPath.Format("%s\\%s_coeff.csv", sPathA, sNameA);
	std::ofstream file(sPath);
	if(file.is_open()==true)
	{
		file << "기준" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << i+1 << ",";

		file << std::endl << "원본" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_HistogramOrg[i] << ",";

		file << std::endl << "정규" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_HistNorm[i] << ",";

		file << std::endl << "보정" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_HistBlur[i] << ",";

		file << std::endl << "미분" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_HistDiffEqu[i] << ",";

		file << std::endl << "최대" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_HistMax[i] << ",";

		file << std::endl << "최소" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_HistMin[i] << ",";

		file << std::endl << "범위" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_HistStEd[i] << ",";

		file << std::endl << "RMS" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_HistStEdRms[i] << ",";

		file << std::endl << "미분Up" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_DiffEquMax[i] << ",";

		file << std::endl << "미분Dn" << ",";
		for(int i=0; i<m_nSizeHisto; i++)
			file << m_DiffEquMin[i] << ",";

		// 결과 데이터
		file	<< std::endl	<< std::endl
				<< "nIndex"		<< "," 
				<< "dValue"		<< "," 
				<< "nStIdx"		<< "," 
				<< "dStVal"		<< "," 
				<< "nEdIdx"		<< "," 
				<< "dEdVal"		<< "," 
				<< "nStRmsIdx"	<< "," 
				<< "dStRmsVal"	<< "," 
				<< "nEdRmsIdx"	<< "," 
				<< "dEdRmsVal"	<< "," 
				<< "Master"		<< "," 
				<< "nUpIdx"		<< "," 
				<< "dUpVal"		<< "," 
				<< "nDnIdx"		<< "," 
				<< "dDnVal"		<< std::endl;
		for(int i=0; i<m_PeakList.size(); i++)
		{
			file	<< m_PeakList[i].nIndex		<< "," 
					<< m_PeakList[i].dValue		<< "," 
					<< m_PeakList[i].nStIdx		<< "," 
					<< m_PeakList[i].dStVal		<< "," 
					<< m_PeakList[i].nEdIdx		<< "," 
					<< m_PeakList[i].dEdVal		<< "," 
					<< m_PeakList[i].nStRmsIdx	<< "," 
					<< m_PeakList[i].dStRmsVal	<< "," 
					<< m_PeakList[i].nEdRmsIdx	<< "," 
					<< m_PeakList[i].dEdRmsVal	<< "," 
					<< m_PeakList[i].nMasterIdx << "," 
					<< m_PeakList[i].nUpIdx		<< "," 
					<< m_PeakList[i].dUpVal		<< "," 
					<< m_PeakList[i].nDnIdx		<< "," 
					<< m_PeakList[i].dDnVal		<< std::endl;
		}

		file.close();
	}

#endif
}

void CHistogramAnalysis::FindStEd(double dTh)
{
	int sz = m_nSizeHisto-1;
	for(int i=0; i<m_nSizeHisto; i++)
	{
		if(m_HistMin[i] != m_dZeroValueForMinMax)
		{
			if(i==0)	// YJS 2017/03/10
			{
				m_HistStEd[i] = 1;
			}
			else
			{
				for(int j=i; j>=0; j--)
				{
					if(fabs(m_HistDiffEqu[j]) > dTh && ((j-1)<0 || ((j-1)>=0 && fabs(m_HistDiffEqu[j-1]) > dTh)))
					{
						m_HistStEd[j] = 1; //m_HistNorm[j];
						break;
					}
					else if((m_HistDiffEqu[j]==0.0 && (j+1)<m_nSizeHisto && m_HistNorm[j+1]==0.0)		||
						(m_HistDiffEqu[j]==0.0 && (j-1)>=0 && m_HistNorm[j-1]==0.0)					)
						break;
					else if(m_HistMax[j] != m_dZeroValueForMinMax)
						break;
				}
			}

			for(int j=i; j<m_nSizeHisto; j++)
			{
				if(fabs(m_HistDiffEqu[j]) > dTh && ((j+1)>=m_nSizeHisto || ((j+1)<m_nSizeHisto && fabs(m_HistDiffEqu[j+1]) > dTh)))
				{
					m_HistStEd[j] = 1; //m_HistNorm[j];
					break;
				}
				else if((m_HistDiffEqu[j]==0.0 && (j+1)<m_nSizeHisto && m_HistNorm[j+1]==0.0)		||
					(m_HistDiffEqu[j]==0.0 && (j-1)>=0 && m_HistNorm[j-1]==0.0)						)
					break;
				else if(m_HistMax[j] != m_dZeroValueForMinMax)
					break;
			}
		}
	}
}

void CHistogramAnalysis::FindStEd_se(double dTh)
{
	for(int i=0; i<m_nSizeHisto; i++)
	{
		if(m_HistMax[i] != m_dZeroValueForMinMax || (i==0 && m_HistMin[i] != m_dZeroValueForMinMax))	// YJS 2017/03/10
		{
			bool bExit(false);
			int nSeqStep(1);

			for(int j=i; j<m_nSizeHisto && bExit==false; j++)
			{
				switch(nSeqStep)
				{
				case 1:
					if(i==(m_nSizeHisto-1))
					{
						m_HistStEd[j] = 1;//m_HistNorm[j];
						bExit = true;
						break;
					}
					else
						nSeqStep++;
				case 2:
					if(abs(m_HistDiffEqu[j]) > dTh)
						nSeqStep++;
					else if(m_HistMin[j]!=m_dZeroValueForMinMax)
					{
						bExit = true;
						break;
					}
					else
						break;
				case 3:
					if(abs(m_HistDiffEqu[j]) < dTh && m_HistNorm[j] < 1.0)
					{
						m_HistStEd[j] = 1;//m_HistNorm[j];
						bExit = true;
						break;
					}
					else if(m_HistMin[j]!=m_dZeroValueForMinMax)
					{
						m_HistStEd[j] = 1;//m_HistNorm[j];
						bExit = true;
						break;
					}
					else
						break;
				case 4:
					break;
				case 5:
					break;
				}
			}

			bExit	= false;
			nSeqStep= 1;

			for(int j=i; j>=0 && bExit==false; j--)
			{
				switch(nSeqStep)
				{
				case 1:
					if(i==0)
					{
						m_HistStEd[j] = m_HistNorm[j];
						bExit = true;
						break;
					}
					else
						nSeqStep++;
				case 2:
					if(abs(m_HistDiffEqu[j]) > dTh)
						nSeqStep++;
					else if(m_HistMin[j]!=m_dZeroValueForMinMax)
					{
						bExit = true;
						break;
					}
					else
						break;
				case 3:
					if(abs(m_HistDiffEqu[j]) < dTh && m_HistNorm[j] < 1.0)
					{
						m_HistStEd[j] = m_HistNorm[j];
						bExit = true;
						break;
					}
					else if(m_HistMin[j]!=m_dZeroValueForMinMax)
					{
						m_HistStEd[j] = m_HistNorm[j];
						bExit = true;
						break;
					}
					else
						break;
				}
			}

		}
	}
}

void CHistogramAnalysis::PeakAnalysis()
{
	m_PeakList.clear();

	bool bFound(false);
	for(int i=0; i<m_nSizeHisto; i++)
	{
		if((m_HistMax[i]!=m_dZeroValueForMinMax || (i==0 && m_HistMin[i]!=m_dZeroValueForMinMax)) && bFound==false)
		{
			bool bExistSt(false), bExistEd(false), bFoundMin(false);
			PeakData pd;
			pd.nIndex = i;
			if(i==0 && m_HistMin[i]!=m_dZeroValueForMinMax)
				pd.dValue = m_HistMin[i];
			else
				pd.dValue = m_HistMax[i];

			for(int j=i; j>=0; j--)
			{
				if(j==255)		// YJS 2017/03/10
					continue;
				if(m_HistStEd[j]!=m_dZeroValueForMinMax)
				{
					pd.nStIdx = j;
					pd.dStVal = m_HistStEd[j];
					bExistSt = true;
					break;
				}
				else if(m_HistMin[j]!=m_dZeroValueForMinMax)
				{
					pd.nStIdx = j;
					pd.dStVal = m_HistStEd[j];
					bExistSt = true;
					break;
				}
			}

			for(int j=i; j<m_nSizeHisto && bExistSt==true; j++)
			{
				if(j==0)		// YJS 2017/03/10
					continue;
				if(m_HistStEd[j]!=m_dZeroValueForMinMax)
				{
					bExistEd = true;
					pd.nEdIdx = j;
					pd.dEdVal = m_HistStEd[j];
					break;
				}
				else if(m_HistMin[j]!=m_dZeroValueForMinMax && bFoundMin==false)
				{
					bFoundMin = true;
				}
				else if(m_HistMin[j]!=m_dZeroValueForMinMax && bFoundMin==true)
				{
					break;
				}
			}

			// 
			if(bExistEd==true)
			{
				m_PeakList.push_back(pd);
			}

			bFound = true;
		}
		else if(bFound==true && m_HistMax[i] == m_dZeroValueForMinMax)
		{
			bFound = false;
		}
	}
}

void CHistogramAnalysis::FindMaster()
{
	int sz = m_PeakList.size()-1;

	for(int i=0; i<sz; i++)
	{
		// 하나의 산에 있는 여러 봉유리중 가장 높은 것을 마스터로 묶는다
		if(	m_PeakList[i].dEdVal > 2.0 && m_PeakList[i].nEdIdx==m_PeakList[i+1].nStIdx	)
		{
			if(m_PeakList[i+1].dValue >= m_PeakList[i].dValue)
			{
				m_PeakList[i].nMasterIdx = m_PeakList[i+1].nIndex;
			}
			else
			{
				m_PeakList[i+1].nMasterIdx = m_PeakList[i].nIndex;
			}
		}
		// 페어로 묶을 산들을 지정한다.
		else if( (m_PeakList[i].dEdRmsVal / m_PeakList[i+1].dStRmsVal) > 0.80 )
		{
			if(m_PeakList[i+1].dValue >= m_PeakList[i].dValue)
			{
				m_PeakList[i].nPairIdx = m_PeakList[i+1].nIndex;
			}
			else
			{
				m_PeakList[i+1].nPairIdx = m_PeakList[i].nIndex;
			}
		}
	}
}

void CHistogramAnalysis::CalcStEd_Rms()
{
	double dRMS(m_dRmsPos);

	// 최고점과 해당 방향의 마지막 지점간의 value% 지점의 밝기를 구한다.
	int sz = m_PeakList.size();
	for(int i=0; i<sz; i++)
	{
		double dStRms = m_HistNorm[m_PeakList[i].nStIdx] + ((m_PeakList[i].dValue - m_HistNorm[m_PeakList[i].nStIdx]) * dRMS);
		for(int j=m_PeakList[i].nIndex; j>=0; j--)
		{
			if(	(m_HistNorm[j] < dStRms)				||
				(j == m_PeakList[i].nIndex && j == 0)	)
			{
				m_PeakList[i].nStRmsIdx = j>=m_PeakList[i].nIndex ? j : j+1;
				m_PeakList[i].dStRmsVal = m_HistNorm[m_PeakList[i].nStRmsIdx];
				m_HistStEdRms[m_PeakList[i].nStRmsIdx]		= m_HistNorm[m_PeakList[i].nStRmsIdx];
				break;
			}
		}

		double dEdRms = m_HistNorm[m_PeakList[i].nEdIdx] + ((m_PeakList[i].dValue - m_HistNorm[m_PeakList[i].nEdIdx]) * dRMS);
		for(int j=m_PeakList[i].nIndex; j<m_nSizeHisto; j++)
		{
			if(	(m_HistNorm[j] < dEdRms)								||
				(j == m_PeakList[i].nIndex && j == (m_nSizeHisto-1))	)
			{
				m_PeakList[i].nEdRmsIdx = j<=m_PeakList[i].nIndex ? j : j-1;
				m_PeakList[i].nEdRmsIdx = j-1;
				if(m_PeakList[i].nEdRmsIdx>=0 && m_PeakList[i].nEdRmsIdx < m_nSizeHisto)
				{
					m_PeakList[i].dEdRmsVal = m_HistNorm[m_PeakList[i].nEdRmsIdx];
					m_HistStEdRms[m_PeakList[i].nEdRmsIdx]		= m_HistNorm[m_PeakList[i].nEdRmsIdx];
				}
				break;
			}
		}
	}
}

void CHistogramAnalysis::CalcUpDn()
{
	// 최고점과 해당 방향의 마지막 지점간의 value% 지점의 밝기를 구한다.
	// 최대와 최소 계산을 해주어야 한다.
	int sz = m_PeakList.size();
	for(int i=0; i<sz; i++)
	{
		for(int j=m_PeakList[i].nIndex; j>=0; j--)
		{
			if(	m_DiffEquMax[j] != m_dZeroValueForMinMax	)
			{
				m_PeakList[i].nUpIdx = j;
				m_PeakList[i].dUpInt = m_DiffEquUpInt[j];
				m_PeakList[i].dUpVal = m_DiffEquMax[j];
				break;
			}
		}

		for(int j=m_PeakList[i].nIndex; j<m_nSizeHisto; j++)
		{
			if(	m_DiffEquMin[j] != m_dZeroValueForMinMax )
			{
				m_PeakList[i].nDnIdx = j;
				m_PeakList[i].dDnInt = m_DiffEquDnInt[j];
				m_PeakList[i].dDnVal = m_DiffEquMin[j];
				break;
			}
		}
	}
}