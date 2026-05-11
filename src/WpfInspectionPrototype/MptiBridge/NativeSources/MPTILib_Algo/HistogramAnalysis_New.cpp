#include "stdafx.h"
#include "HistogramAnalysis_New.h"


CHistogramAnalysis_New::CHistogramAnalysis_New()
{
}


CHistogramAnalysis_New::~CHistogramAnalysis_New()
{
}

void CHistogramAnalysis_New::Reset()
{
	CHistogramAnalysis::Reset();

	m_HistPeak.clear();
	m_HistPeak.resize(m_nSizeHisto);
}

void CHistogramAnalysis_New::CheckCrossPoints()
{
	// 피크점 찾기
	for (int i = 1; i < (m_nSizeHisto-1); i++)
		m_HistPeak[i] |= m_HistDiffEqu[i - 1] > 0.0 && m_HistDiffEqu[i + 1] < 0.0 ? ePointType::Peak : ePointType::None;
	if(m_HistMaxIdx > 0
		&& m_HistPeak[m_HistMaxIdx - 1] == ePointType::None
		&& m_HistPeak[m_HistMaxIdx] == ePointType::None
		&&(m_HistMaxIdx < 255 && m_HistPeak[m_HistMaxIdx + 1] == ePointType::None))
		m_HistPeak[m_HistMaxIdx] |= ePointType::Peak;

	if (m_HistBlur[0] > m_HistBlur[1] > m_HistBlur[2])
		m_HistPeak[0] |= ePointType::Peak;
	if (m_HistBlur[m_nSizeHisto - 1] > m_HistBlur[m_nSizeHisto - 2] > m_HistBlur[m_nSizeHisto - 3])
		m_HistPeak[m_nSizeHisto - 1] |= ePointType::Peak;

	for (int i = 0; i < m_nSizeHisto; i++)
	{
		if (m_HistPeak[i] == ePointType::Peak)
		{
			PeakData pd;

			// 좌로 찾고
			for (int j = 1; j < (m_nSizeHisto - 2); j++)
			{
				int idx = i - j;

				if (idx < 0 || m_HistDiffEqu[idx] < 0.0 || m_HistBlur[idx] == 0.0)
				{
					int nextidx = idx + 1;
					m_HistPeak[nextidx] |= ePointType::Start;
					pd.nStIdx = nextidx;
					pd.dStVal = m_HistBlur[nextidx];
					break;
				}
				else
					m_HistPeak[idx] = ePointType::Body;
			}

			// 우로 찾는다
			for (int j = 1; j < (m_nSizeHisto - 2); j++)
			{
				int idx = i + j;

				if (idx >= m_nSizeHisto || m_HistDiffEqu[idx] > 0.0 || m_HistBlur[idx] == 0.0)
				{
					int previdx = idx - 1;
					m_HistPeak[previdx] |= ePointType::End;
					pd.nEdIdx = previdx;
					pd.dEdVal = m_HistBlur[previdx];
					break;
				}
				else
					m_HistPeak[idx] = ePointType::Body;
			}

			int nPeakIdx(-1);
			double dPeakVal(-HUGE_VAL);
			for (int j = pd.nStIdx; j != pd.nEdIdx; j++)
			{
				if (dPeakVal < m_HistBlur[j])
				{
					dPeakVal = m_HistBlur[j];
					nPeakIdx = j;
				}
			}
			pd.dValue = dPeakVal;
			pd.nIndex = nPeakIdx;

			if(nPeakIdx < 0)
				continue;
			
			if (1 < nPeakIdx && nPeakIdx < m_nSizeHisto - 1)
			{
				if(m_HistDiffEqu[nPeakIdx - 1] != 0 && m_HistDiffEqu[nPeakIdx]!=0 && m_HistDiffEqu[nPeakIdx + 1]!=0)
				{
					if ((m_HistDiffEqu[nPeakIdx - 1] > 0 && m_HistDiffEqu[nPeakIdx] < 0) ||
						(m_HistDiffEqu[nPeakIdx - 1] < 0 && m_HistDiffEqu[nPeakIdx] > 0))
					{
						double dHv = fabs(m_HistDiffEqu[nPeakIdx - 1] - m_HistDiffEqu[nPeakIdx]);

						if (m_HistDiffEqu[nPeakIdx] > 0.0)
							pd.dIndexSub = (double)(nPeakIdx)-m_HistDiffEqu[nPeakIdx]/fabs(m_HistDiffEqu[nPeakIdx]- m_HistDiffEqu[nPeakIdx-1]);
						else
							pd.dIndexSub = (double)(nPeakIdx - 1) + m_HistDiffEqu[nPeakIdx] / fabs(m_HistDiffEqu[nPeakIdx] - m_HistDiffEqu[nPeakIdx - 1]);
					}
					else if ((m_HistDiffEqu[nPeakIdx + 1] > 0 && m_HistDiffEqu[nPeakIdx] < 0) ||
							 (m_HistDiffEqu[nPeakIdx + 1] < 0 && m_HistDiffEqu[nPeakIdx] > 0))
					{
						double dHv = fabs(m_HistDiffEqu[nPeakIdx + 1] - m_HistDiffEqu[nPeakIdx]);

						if (m_HistDiffEqu[nPeakIdx] > 0.0)
							pd.dIndexSub = (double)(nPeakIdx)+ m_HistDiffEqu[nPeakIdx] / fabs(m_HistDiffEqu[nPeakIdx] - m_HistDiffEqu[nPeakIdx]);
						else
							pd.dIndexSub = (double)(nPeakIdx + 1) - m_HistDiffEqu[nPeakIdx] / fabs(m_HistDiffEqu[nPeakIdx] - m_HistDiffEqu[nPeakIdx]);
					}
				}
				else
					pd.dIndexSub = pd.nIndex;
			}
			else
				pd.dIndexSub = pd.nIndex;
			

			double dRMS(m_dRmsPos);
			double dStRms = m_HistBlur[pd.nStIdx] + ((pd.dValue - m_HistBlur[pd.nStIdx]) * dRMS);
			for (int j = pd.nIndex; j != (pd.nStIdx-1); j--)
			{
				if ((m_HistBlur[j] < dStRms) )
				{
					pd.nStRmsIdx = j >= pd.nIndex ? j : j + 1;
					pd.dStRmsVal = m_HistBlur[pd.nStRmsIdx];
					break;
				}
				else if (j == 0)
				{
					pd.nStRmsIdx = j;
					pd.dStRmsVal = m_HistBlur[pd.nStRmsIdx];
				}
			}

			double dEdRms = m_HistBlur[pd.nEdIdx] + ((pd.dValue - m_HistBlur[pd.nEdIdx]) * dRMS);
			for (int j = pd.nIndex; j != (pd.nEdIdx+1); j++)
			{
				if ((m_HistBlur[j] < dEdRms))
				{
					pd.nEdRmsIdx = j <= pd.nIndex ? j : j - 1;
					pd.nEdRmsIdx = j - 1;
					if (pd.nEdRmsIdx >= 0 && pd.nEdRmsIdx < m_nSizeHisto)
					{
						pd.dEdRmsVal = m_HistBlur[pd.nEdRmsIdx];
					}
					break;
				}
				else if (j == (m_nSizeHisto - 1))
				{
					pd.nEdRmsIdx = j;
					if (pd.nEdRmsIdx >= 0 && pd.nEdRmsIdx < m_nSizeHisto)
						pd.dEdRmsVal = m_HistBlur[pd.nEdRmsIdx];
				}
			}

			// Peak List 생성
			m_PeakList.push_back(pd);
		}
	}
}