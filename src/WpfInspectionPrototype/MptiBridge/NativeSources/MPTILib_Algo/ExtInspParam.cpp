#include "stdafx.h"
#include "ExtInspParam.h"
#include "ExtLog.h"
#include "ipp.h"

namespace ext {
namespace Integ {
	aAlgo::aAlgo()
	{
	}
	bool aAlgo::Init()
	{
		return true;
	}
	void aAlgo::Exit()
	{
		
	}

	void aAlgo::Add(InspPartInfo * PartData, BYTE * pBuf)
	{
		InspPartParam * pWnd = PartData->pWindows;
		QWORD qwOfs(0);
		for (int w = 0; w < PartData->nWindowCount; w++)
		{
			int nAlgo = pWnd[w].nAlgorithmCnt;
			int szCommonAlgo = sizeof(InspAlgo) * pWnd[w].nAlgorithmCnt;
			memcpy(&pBuf[qwOfs], pWnd[w].vArrAlgoParam, szCommonAlgo);
			qwOfs += szCommonAlgo;
		}
		for (int w = 0; w < PartData->nWindowCount; w++)
		{
			int nAlgo = pWnd[w].nAlgorithmCnt;
			for (int a = 0; a < nAlgo; a++)
			{
				void * lpBuf = &pBuf[qwOfs];
				QWORD szAlgo = _AlgoClone(pWnd[w].vArrAlgoParam[a].m_eAlgoType, pWnd[w].vArrAlgoParam[a].m_ptrInspAlgoParam, lpBuf);
				qwOfs += szAlgo;
			}
		}
		for (int w = 0; w < PartData->nWindowCount; w++)
		{
			InspAlgo* inAlgo = (InspAlgo*)pWnd[w].vArrAlgoParam;
			int szMskAlgo = sizeof(RECT) * inAlgo->m_nUsedMaskingValue;
			memcpy(&pBuf[qwOfs], inAlgo->m_rcArrMaskingROI, szMskAlgo);
			qwOfs += szMskAlgo;
		}
	}
	void aAlgo::FromWnd(InspPartInfo * PartData, BYTE * pBuf)
	{
		InspPartParam * pWnd = PartData->pWindows;
		QWORD qwOfs(0);
		for (int w = 0; w < PartData->nWindowCount; w++)
		{
			int nAlgo = pWnd[w].nAlgorithmCnt;
			int szCommonAlgo = sizeof(InspAlgo) * pWnd[w].nAlgorithmCnt;
			ExtPrmPtr(pWnd[w].vArrAlgoParam) = (InspAlgo*)(&pBuf[qwOfs]);
			memcpy(&pBuf[qwOfs], pWnd[w].vArrAlgoParam, szCommonAlgo);
			qwOfs += szCommonAlgo;
		}
		for (int w = 0; w < PartData->nWindowCount; w++)
		{
			int nAlgo = pWnd[w].nAlgorithmCnt;
			for (int a = 0; a < nAlgo; a++)
			{
				void * lpBuf = &pBuf[qwOfs];
				ExtPrmPtr(pWnd[w].vArrAlgoParam[a].m_ptrInspAlgoParam) = lpBuf;
				QWORD szAlgo = _AlgoSize(pWnd[w].vArrAlgoParam[a].m_eAlgoType);
				qwOfs += szAlgo;
			}
		}
	}
	void aAlgo::Clear()
	{
		
	}
	int aAlgo::_AlgoClone(InspAlgoType eAlgoType, LPVOID lpSource, LPVOID& dst)
	{
		int stSize(0);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			memcpy_s(dst, sizeof(AlgoAlign), lpSource, stSize = sizeof(AlgoAlign));
			break;
		case eAlgoBW:
			memcpy_s(dst, sizeof(AlgoBW), lpSource, stSize = sizeof(AlgoBW));
			break;
		case eAlgoBlob:
			memcpy_s(dst, sizeof(AlgoBlob), lpSource, stSize = sizeof(AlgoBlob));
			break;
		case eAlgoBody_Blob:
			memcpy_s(dst, sizeof(AlgoBodyBlob), lpSource, stSize = sizeof(AlgoBodyBlob));
			break;
		case eAlgoTilt:
			memcpy_s(dst, sizeof(AlgoTilt), lpSource, stSize = sizeof(AlgoTilt));
			break;
		case eAlgoOCR:
			memcpy_s(dst, sizeof(AlgoOCR), lpSource, stSize = sizeof(AlgoOCR));
			break;
		case eAlgoPattern:
			memcpy_s(dst, sizeof(AlgoPattern), lpSource, stSize = sizeof(AlgoPattern));
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			memcpy_s(dst, sizeof(AlgoColor), lpSource, stSize = sizeof(AlgoColor));
			break;
		case eAlgoGray_Mean:
			memcpy_s(dst, sizeof(AlgoGrayMean), lpSource, stSize = sizeof(AlgoGrayMean));
			break;
		case eAlgoHeight_Mean:
			memcpy_s(dst, sizeof(AlgoHeightMean), lpSource, stSize = sizeof(AlgoHeightMean));
			break;
		case eAlgoGray_Diff:
			memcpy_s(dst, sizeof(AlgoGrayDiff), lpSource, stSize = sizeof(AlgoGrayDiff));
			break;
		case eAlgoHeight_Diff:
			memcpy_s(dst, sizeof(AlgoHeightDiff), lpSource, stSize = sizeof(AlgoHeightDiff));
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			memcpy_s(dst, sizeof(AlgoLeadSearch), lpSource, stSize = sizeof(AlgoLeadSearch));
			break;
		case eAlgoBridge:
			memcpy_s(dst, sizeof(AlgoBridge), lpSource, stSize = sizeof(AlgoBridge));
			break;
		case eAlgoLead_Tip:
			memcpy_s(dst, sizeof(AlgoLeadTip), lpSource, stSize = sizeof(AlgoLeadTip));
			break;
		case eAlgoLead_Lift:
			memcpy_s(dst, sizeof(AlgoLeadLift), lpSource, stSize = sizeof(AlgoLeadLift));
			break;
		case eAlgoLead_Solder:
			memcpy_s(dst, sizeof(AlgoLeadSolder), lpSource, stSize = sizeof(AlgoLeadSolder));
			break;
		case eAlgoLead_SideSolder:
			memcpy_s(dst, sizeof(AlgoLeadSideSolder), lpSource, stSize = sizeof(AlgoLeadSideSolder));
			break;
		case eAlgoWidth:
			memcpy_s(dst, sizeof(AlgoLength), lpSource, stSize = sizeof(AlgoLength));
			break;
		case eAlgoVolume:
			memcpy_s(dst, sizeof(AlgoVolume), lpSource, stSize = sizeof(AlgoVolume));
			break;
		case eAlgoTab:
			memcpy_s(dst, sizeof(AlgoTab), lpSource, stSize = sizeof(AlgoTab));
			break;
		case eAlgoGrid:
			memcpy_s(dst, sizeof(AlgoGrid), lpSource, stSize = sizeof(AlgoGrid));
			break;
		case eAlgoLine:
			memcpy_s(dst, sizeof(AlgoLine), lpSource, stSize = sizeof(AlgoLine));
			break;
		case eAlgoEdge:
			memcpy_s(dst, sizeof(AlgoEdge), lpSource, stSize = sizeof(AlgoEdge));
			break;
		case eAlgoSolderCone:
			memcpy_s(dst, sizeof(AlgoSolderCone), lpSource, stSize = sizeof(AlgoSolderCone));
			break;
		case eAlgoColorXY:
			memcpy_s(dst, sizeof(AlgoColorXY), lpSource, stSize = sizeof(AlgoColorXY));
			break;
		case eAlgoAlignEdge:
			memcpy_s(dst, sizeof(AlgoAlignEdge), lpSource, stSize = sizeof(AlgoAlignEdge));
			break;
		case eAlgoPadAlign:
			memcpy_s(dst, sizeof(AlgoPadAlign), lpSource, stSize = sizeof(AlgoPadAlign));
			break;
		case eAlgoPOCR:
			memcpy_s(dst, sizeof(AlgoPOCR), lpSource, stSize = sizeof(AlgoPOCR));
			break;
		case eAlgoWire:
			memcpy_s(dst, sizeof(AlgoWire), lpSource, stSize = sizeof(AlgoWire));
			break;
		case eAlgoFoot:
			memcpy_s(dst, sizeof(AlgoFoot), lpSource, stSize = sizeof(AlgoFoot));
			break;
		case eAlgoBarcode:
			memcpy_s(dst, sizeof(AlgoBarcode), lpSource, stSize = sizeof(AlgoBarcode));
			break;
		case eAlgoFillet:
			memcpy_s(dst, sizeof(AlgoFillet), lpSource, stSize = sizeof(AlgoFillet));
			break;
		case eAlgoBGA:
			memcpy_s(dst, sizeof(AlgoBGA), lpSource, stSize = sizeof(AlgoBGA));
			break;
		case eAlgoBump:
			memcpy_s(dst, sizeof(AlgoBump), lpSource, stSize = sizeof(AlgoBump));
			break;
		case eAlgoNGBlob:
			memcpy_s(dst, sizeof(AlgoNGBlob), lpSource, stSize = sizeof(AlgoNGBlob));
			break;
		case eAlgoBodyEdge:
			memcpy_s(dst, sizeof(AlgoBodyEdge), lpSource, stSize = sizeof(AlgoBodyEdge));
			break;
			//shw Delete Xcase eAlgoNewAlgo1:#@dst = g_pMManager->pem_new<AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);#@memcpy_s(dst, sizeof(AlgoNewAlgo1), lpSource, sizeof(AlgoNewAlgo1));#@break;
		default:
			break;
		}
		return stSize;
	}
	int aAlgo::_AlgoSize(InspAlgoType eAlgoType)
	{
		int stSize(0);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			stSize = sizeof(AlgoAlign);
			break;
		case eAlgoBW:
			stSize = sizeof(AlgoBW);
			break;
		case eAlgoBlob:
			stSize = sizeof(AlgoBlob);
			break;
		case eAlgoBody_Blob:
			stSize = sizeof(AlgoBodyBlob);
			break;
		case eAlgoTilt:
			stSize = sizeof(AlgoTilt);
			break;
		case eAlgoOCR:
			stSize = sizeof(AlgoOCR);
			break;
		case eAlgoPattern:
			stSize = sizeof(AlgoPattern);
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			stSize = sizeof(AlgoColor);
			break;
		case eAlgoGray_Mean:
			stSize = sizeof(AlgoGrayMean);
			break;
		case eAlgoHeight_Mean:
			stSize = sizeof(AlgoHeightMean);
			break;
		case eAlgoGray_Diff:
			stSize = sizeof(AlgoGrayDiff);
			break;
		case eAlgoHeight_Diff:
			stSize = sizeof(AlgoHeightDiff);
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			stSize = sizeof(AlgoLeadSearch);
			break;
		case eAlgoBridge:
			stSize = sizeof(AlgoBridge);
			break;
		case eAlgoLead_Tip:
			stSize = sizeof(AlgoLeadTip);
			break;
		case eAlgoLead_Lift:
			stSize = sizeof(AlgoLeadLift);
			break;
		case eAlgoLead_Solder:
			stSize = sizeof(AlgoLeadSolder);
			break;
		case eAlgoLead_SideSolder:
			stSize = sizeof(AlgoLeadSideSolder);
			break;
		case eAlgoWidth:
			stSize = sizeof(AlgoLength);
			break;
		case eAlgoVolume:
			stSize = sizeof(AlgoVolume);
			break;
		case eAlgoTab:
			stSize = sizeof(AlgoTab);
			break;
		case eAlgoGrid:
			stSize = sizeof(AlgoGrid);
			break;
		case eAlgoLine:
			stSize = sizeof(AlgoLine);
			break;
		case eAlgoEdge:
			stSize = sizeof(AlgoEdge);
			break;
		case eAlgoSolderCone:
			stSize = sizeof(AlgoSolderCone);
			break;
		case eAlgoColorXY:
			stSize = sizeof(AlgoColorXY);
			break;
		case eAlgoAlignEdge:
			stSize = sizeof(AlgoAlignEdge);
			break;
		case eAlgoPadAlign:
			stSize = sizeof(AlgoPadAlign);
			break;
		case eAlgoPOCR:
			stSize = sizeof(AlgoPOCR);
			break;
		case eAlgoWire:
			stSize = sizeof(AlgoWire);
			break;
		case eAlgoFoot:
			stSize = sizeof(AlgoFoot);
			break;
		case eAlgoBarcode:
			stSize = sizeof(AlgoBarcode);
			break;
		case eAlgoFillet:
			stSize = sizeof(AlgoFillet);
			break;
		case eAlgoBGA:
			stSize = sizeof(AlgoBGA);
			break;
		case eAlgoBump:
			stSize = sizeof(AlgoBump);
			break;
		case eAlgoNGBlob:
			stSize = sizeof(AlgoNGBlob);
			break;
		case eAlgoBodyEdge:
			stSize = sizeof(AlgoBodyEdge);
			break;
			//shw Delete Xcase eAlgoNewAlgo1:#@dst = g_pMManager->pem_new<AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);#@memcpy_s(dst, sizeof(AlgoNewAlgo1), lpSource, sizeof(AlgoNewAlgo1));#@break;
		default:
			break;
		}
		return stSize;
	}
	QWORD aAlgo::GetSize(InspPartInfo * PartData)
	{
		InspPartParam * pWnd = PartData->pWindows;
		QWORD szAlgo(0);
		for (int w = 0; w < PartData->nWindowCount; w++)
		{
			int nAlgo = pWnd[w].nAlgorithmCnt;
			for (int a = 0; a < nAlgo; a++)
			{
				szAlgo += sizeof(InspAlgo);
				szAlgo += _AlgoSize(pWnd[w].vArrAlgoParam[a].m_eAlgoType);
			}
		}
		return szAlgo;
	}


	Windows::Windows()
	{

	}
	bool Windows::Init()
	{
		_Algos.Init();
		return true;
	}	
	void Windows::Add(InspPartInfo * PartData, BYTE * pBuf)
	{
		QWORD szWnd = GetSize(PartData);
		memcpy(pBuf, PartData->pWindows, szWnd);
		_Algos.Add(PartData, pBuf + szWnd);

	}
	void Windows::FromPart(InspPartInfo * PartData, BYTE * pBuf)
	{
		QWORD szWnd = GetSize(PartData);
		ExtPrmPtr(PartData->pWindows) = (InspPartParam *)pBuf;
		_Algos.FromWnd(PartData, pBuf + szWnd);
	}
	QWORD Windows::GetSize(InspPartInfo * PartData)
	{
		return sizeof(InspPartParam) * PartData->nWindowCount;		
	}


	Part::Part()
		: _qwOffsetBytes(0)
	{

	}
	bool Part::Init()
	{
		ext::Log::add(_T("Part::Init() - Start"));
		
		QWORD szPart = sizeof(InspPartInfo) * env::nPartBufCount;
		QWORD szWindow = sizeof(InspPartParam) * env::nWindowBufCount;
		QWORD szAlgoC = sizeof(InspAlgo) * env::nAlgoBufCount;
		QWORD szAlgoE = env::nAlgoElemBufSzBytes;
		QWORD szAllBuf = szPart + szWindow + szAlgoC + szAlgoE;

		CString sMemName;
		sMemName.Format(_T("ExtAOIPart_%d"), env::nTool_id);
		if (_stream.AllocMemory(szAllBuf, sMemName) == false)
		{
			if (_stream.Open(szAllBuf, _T("ExtAOIPart"), FILE_MAP_ALL_ACCESS) == false)
			{
				return false;
			}
			if (_stream.MapViewAll(FILE_MAP_ALL_ACCESS) == false)
				return false;
		}

		ext::Log::add(_T("Part::Init() - End"));
		_Windows.Init();
		return true;
	}
	QWORD Part::GetSize(InspPartInfo * PartData)
	{
		QWORD szPart = sizeof(InspPartInfo);
		QWORD szWnd  = _Windows.GetSize(PartData);
		QWORD szAlgo = _Windows._Algos.GetSize(PartData);
		return szPart + szWnd + szAlgo;
	}
	QWORD Part::AddPart(InspPartInfo * PartData, InspPartParam * pWind, int nWndCnt)
	{
		PartData->pWindows = pWind;
		PartData->nWindowCount = nWndCnt;
		return Add(PartData);
	}
	QWORD Part::Add(InspPartInfo * PartData)
	{
		QWORD szParam = GetSize(PartData);

		if ((_qwOffsetBytes + szParam) >= _stream.m_nBytes)
			_qwOffsetBytes = 0;

		QWORD qwPartStart(_qwOffsetBytes);
		tagParamHead head;
		head.zero = 0;
		head.qwOffset = _qwOffsetBytes;
		head.sizebytes = szParam;
		BYTE * ptr = _stream.Byte(_qwOffsetBytes);
		int szHead = sizeof(tagParamHead);
		int szPart = sizeof(InspPartInfo);
		memcpy(ptr, &head, szHead);
		memcpy(ptr + szHead, PartData, szPart);
		_Windows.Add(PartData, ptr + szHead + szPart);
		_stream.Flush(ptr, sizeof(InspPartInfo) * szParam);

		_qwOffsetBytes += szParam;

		return qwPartStart;
	}





	PartBuff::PartBuff()
		: _qwCurrentPos(0)
	{
	}
	bool PartBuff::Init()
	{
		ext::Log::add(_T("Part::Init() - Start"));

		bool bInit = true;

		CString sMemName;
		sMemName.Format(_T("ExtTool_%d_PartIdx_%d"), env::nTool_id, nBuffIdx);
		if (_stream.AllocMemory(env::nTotalPartBufferSize, sMemName) == false)
		{
			if (_stream.Open(env::nTotalPartBufferSize, sMemName, FILE_MAP_ALL_ACCESS) == false)
				bInit = false;
			if (_stream.MapViewAll(FILE_MAP_ALL_ACCESS) == false)
				bInit = false;
		}

		ext::Log::add(_T("Part::Init() - End"));

		return bInit;

	}
	void PartBuff::AddPart(InspPartInfo * PartData, InspPartParam * pWind, int nWndCnt)
	{
		PartData->pWindows = pWind;
		PartData->nWindowCount = nWndCnt;
		Add(PartData);
	}
	void PartBuff::GetPart(InspPartInfo * PartData, InspPartParam * pWind)
	{
		_qwCurrentPos = 0;
		BYTE* ptr = _stream.Byte(_qwCurrentPos);
		
		//part
		memcpy(PartData, ptr, sizeof(InspPartInfo)); _qwCurrentPos += sizeof(InspPartInfo);

		for (int nWndIdx = 0; nWndIdx < PartData->nWindowCount; nWndIdx++)
		{
			//window
			memcpy(&pWind[nWndIdx], ptr + _qwCurrentPos, sizeof(InspPartParam)); _qwCurrentPos += sizeof(InspPartParam);
			for (int nAlgoIdx = 0; nAlgoIdx < pWind[nWndIdx].nAlgorithmCnt; nAlgoIdx++)
			{
				//commonalgo
				memcpy(&pWind[nWndIdx].vArrAlgoParam[nAlgoIdx], ptr + _qwCurrentPos, sizeof(InspPartParam)); _qwCurrentPos += sizeof(InspAlgo);

				//commonalgo
				int nAlgoSize = AlgoSize(pWind[nWndIdx].vArrAlgoParam[nAlgoIdx].m_eAlgoType);
				memcpy(&pWind[nWndIdx].vArrAlgoParam[nAlgoIdx].m_ptrInspAlgoParam, ptr + _qwCurrentPos, sizeof(InspPartParam)); _qwCurrentPos += nAlgoSize;
			}
		}
	}
	bool PartBuff::Add(InspPartInfo * PartData)
	{								
		_qwCurrentPos = 0;
		bool bAddSucces = true;						
								
		BYTE * ptr = _stream.Byte(_qwCurrentPos);

		//part
		memcpy(ptr + _qwCurrentPos, PartData, sizeof(InspPartInfo)); _qwCurrentPos += sizeof(InspPartInfo);
		
		for (int wnd = 0; wnd < PartData->nWindowCount; wnd++)
		{
			//window
			InspPartParam pWnd = PartData->pWindows[wnd];
			memcpy(ptr + _qwCurrentPos, &pWnd, sizeof(InspPartParam)); _qwCurrentPos += sizeof(InspPartParam);
			for (int algo = 0; algo < pWnd.nAlgorithmCnt; algo++)
			{
				//common algo
				InspAlgo pAlgo = pWnd.vArrAlgoParam[algo];
				memcpy(ptr + _qwCurrentPos, &pAlgo, sizeof(InspAlgo)); _qwCurrentPos += sizeof(InspAlgo);

				//algo
				int nAlgoSize = AlgoClone(pAlgo.m_eAlgoType, pAlgo.m_ptrInspAlgoParam, ptr + _qwCurrentPos); _qwCurrentPos += nAlgoSize;
			}
		}


		_stream.Flush(ptr, _qwCurrentPos);

		
		return bAddSucces;
	}
	int PartBuff::AlgoSize(InspAlgoType eAlgoType)
	{
		int stSize(0);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			stSize = sizeof(RstAlgoAlign);
			break;
		case eAlgoBW:
			stSize = sizeof(RstAlgoBlackWhite);
			break;
		case eAlgoBlob:
			stSize = sizeof(RstAlgoBlob);
			break;
		case eAlgoBody_Blob:
			stSize = sizeof(RstAlgoBodyBlob);
			break;
		case eAlgoTilt:
			stSize = sizeof(RstAlgoTilt);
			break;
		case eAlgoOCR:
			stSize = sizeof(RstAlgoOCR);
			break;
		case eAlgoPattern:
			stSize = sizeof(RstAlgoPattern);
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			stSize = sizeof(RstAlgoColor);
			break;
		case eAlgoGray_Mean:
			stSize = sizeof(RstAlgoGrayMean);
			break;
		case eAlgoHeight_Mean:
			stSize = sizeof(RstAlgoHeightMean);
			break;
		case eAlgoGray_Diff:
			stSize = sizeof(RstAlgoGrayDiff);
			break;
		case eAlgoHeight_Diff:
			stSize = sizeof(RstAlgoHeightDiff);
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			stSize = sizeof(RstAlgoLeadSearch);
			break;
		case eAlgoBridge:
			stSize = sizeof(RstAlgoBridge);
			break;
		case eAlgoLead_Tip:
			stSize = sizeof(RstAlgoLeadTip);
			break;
		case eAlgoLead_Lift:
			stSize = sizeof(RstAlgoLeadLift);
			break;
		case eAlgoLead_Solder:
			stSize = sizeof(RstAlgoLeadSolder);
			break;
		case eAlgoLead_SideSolder:
			stSize = sizeof(RstAlgoLeadSideSolder);
			break;
		case eAlgoWidth:
			stSize = sizeof(RstAlgoLength);
			break;
		case eAlgoVolume:
			stSize = sizeof(RstAlgoVolume);
			break;
		case eAlgoTab:
			stSize = sizeof(RstAlgoTab);
			break;
		case eAlgoGrid:
			stSize = sizeof(RstAlgoGrid);
			break;
		case eAlgoLine:
			stSize = sizeof(RstAlgoLine);
			break;
		case eAlgoEdge:
			stSize = sizeof(RstAlgoEdge);
			break;
		case eAlgoSolderCone:
			stSize = sizeof(RstAlgoSolderCone);
			break;
		case eAlgoColorXY:
			stSize = sizeof(RstAlgoColorXY);
			break;
		case eAlgoAlignEdge:
			stSize = sizeof(RstAlgoAlignEdge);
			break;
		case eAlgoPadAlign:
			stSize = sizeof(RstAlgoPadAlign);
			break;
		case eAlgoPOCR:
			stSize = sizeof(RstAlgoPOCR);
			break;
		case eAlgoWire:
			stSize = sizeof(RstAlgoWire);
			break;
		case eAlgoFoot:
			stSize = sizeof(RstAlgoFoot);
			break;
		case eAlgoBarcode:
			stSize = sizeof(RstAlgoBarcode);
			break;
		case eAlgoFillet:
			stSize = sizeof(RstAlgoFillet);
			break;
		case eAlgoBGA:
			stSize = sizeof(RstAlgoBGA);
			break;
		case eAlgoBump:
			stSize = sizeof(RstAlgoBump);
			break;
		case eAlgoNGBlob:
			stSize = sizeof(RstAlgoNGBlob);
			break;
		case eAlgoBodyEdge:
			stSize = sizeof(RstAlgoBodyEdge);
			break;
			//shw Delete Xcase eAlgoNewAlgo1:#@dst = g_pMManager->pem_new<AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);#@memcpy_s(dst, sizeof(AlgoNewAlgo1), lpSource, sizeof(AlgoNewAlgo1));#@break;
		default:
			break;
		}
		return stSize;
	}
	int PartBuff::AlgoClone(InspAlgoType eAlgoType, LPVOID& lpSource, BYTE* dst)
	{
		int stSize(0);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			memcpy_s(dst, sizeof(AlgoAlign), lpSource, stSize = sizeof(AlgoAlign));
			break;
		case eAlgoBW:
			memcpy_s(dst, sizeof(AlgoBW), lpSource, stSize = sizeof(AlgoBW));
			break;
		case eAlgoBlob:
			memcpy_s(dst, sizeof(AlgoBlob), lpSource, stSize = sizeof(AlgoBlob));
			break;
		case eAlgoBody_Blob:
			memcpy_s(dst, sizeof(AlgoBodyBlob), lpSource, stSize = sizeof(AlgoBodyBlob));
			break;
		case eAlgoTilt:
			memcpy_s(dst, sizeof(AlgoTilt), lpSource, stSize = sizeof(AlgoTilt));
			break;
		case eAlgoOCR:
			memcpy_s(dst, sizeof(AlgoOCR), lpSource, stSize = sizeof(AlgoOCR));
			break;
		case eAlgoPattern:
			memcpy_s(dst, sizeof(AlgoPattern), lpSource, stSize = sizeof(AlgoPattern));
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			memcpy_s(dst, sizeof(AlgoColor), lpSource, stSize = sizeof(AlgoColor));
			break;
		case eAlgoGray_Mean:
			memcpy_s(dst, sizeof(AlgoGrayMean), lpSource, stSize = sizeof(AlgoGrayMean));
			break;
		case eAlgoHeight_Mean:
			memcpy_s(dst, sizeof(AlgoHeightMean), lpSource, stSize = sizeof(AlgoHeightMean));
			break;
		case eAlgoGray_Diff:
			memcpy_s(dst, sizeof(AlgoGrayDiff), lpSource, stSize = sizeof(AlgoGrayDiff));
			break;
		case eAlgoHeight_Diff:
			memcpy_s(dst, sizeof(AlgoHeightDiff), lpSource, stSize = sizeof(AlgoHeightDiff));
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			memcpy_s(dst, sizeof(AlgoLeadSearch), lpSource, stSize = sizeof(AlgoLeadSearch));
			break;
		case eAlgoBridge:
			memcpy_s(dst, sizeof(AlgoBridge), lpSource, stSize = sizeof(AlgoBridge));
			break;
		case eAlgoLead_Tip:
			memcpy_s(dst, sizeof(AlgoLeadTip), lpSource, stSize = sizeof(AlgoLeadTip));
			break;
		case eAlgoLead_Lift:
			memcpy_s(dst, sizeof(AlgoLeadLift), lpSource, stSize = sizeof(AlgoLeadLift));
			break;
		case eAlgoLead_Solder:
			memcpy_s(dst, sizeof(AlgoLeadSolder), lpSource, stSize = sizeof(AlgoLeadSolder));
			break;
		case eAlgoLead_SideSolder:
			memcpy_s(dst, sizeof(AlgoLeadSideSolder), lpSource, stSize = sizeof(AlgoLeadSideSolder));
			break;
		case eAlgoWidth:
			memcpy_s(dst, sizeof(AlgoLength), lpSource, stSize = sizeof(AlgoLength));
			break;
		case eAlgoVolume:
			memcpy_s(dst, sizeof(AlgoVolume), lpSource, stSize = sizeof(AlgoVolume));
			break;
		case eAlgoTab:
			memcpy_s(dst, sizeof(AlgoTab), lpSource, stSize = sizeof(AlgoTab));
			break;
		case eAlgoGrid:
			memcpy_s(dst, sizeof(AlgoGrid), lpSource, stSize = sizeof(AlgoGrid));
			break;
		case eAlgoLine:
			memcpy_s(dst, sizeof(AlgoLine), lpSource, stSize = sizeof(AlgoLine));
			break;
		case eAlgoEdge:
			memcpy_s(dst, sizeof(AlgoEdge), lpSource, stSize = sizeof(AlgoEdge));
			break;
		case eAlgoSolderCone:
			memcpy_s(dst, sizeof(AlgoSolderCone), lpSource, stSize = sizeof(AlgoSolderCone));
			break;
		case eAlgoColorXY:
			memcpy_s(dst, sizeof(AlgoColorXY), lpSource, stSize = sizeof(AlgoColorXY));
			break;
		case eAlgoAlignEdge:
			memcpy_s(dst, sizeof(AlgoAlignEdge), lpSource, stSize = sizeof(AlgoAlignEdge));
			break;
		case eAlgoPadAlign:
			memcpy_s(dst, sizeof(AlgoPadAlign), lpSource, stSize = sizeof(AlgoPadAlign));
			break;
		case eAlgoPOCR:
			memcpy_s(dst, sizeof(AlgoPOCR), lpSource, stSize = sizeof(AlgoPOCR));
			break;
		case eAlgoWire:
			memcpy_s(dst, sizeof(AlgoWire), lpSource, stSize = sizeof(AlgoWire));
			break;
		case eAlgoFoot:
			memcpy_s(dst, sizeof(AlgoFoot), lpSource, stSize = sizeof(AlgoFoot));
			break;
		case eAlgoBarcode:
			memcpy_s(dst, sizeof(AlgoBarcode), lpSource, stSize = sizeof(AlgoBarcode));
			break;
		case eAlgoFillet:
			memcpy_s(dst, sizeof(AlgoFillet), lpSource, stSize = sizeof(AlgoFillet));
			break;
		case eAlgoBGA:
			memcpy_s(dst, sizeof(AlgoBGA), lpSource, stSize = sizeof(AlgoBGA));
			break;
		case eAlgoBump:
			memcpy_s(dst, sizeof(AlgoBump), lpSource, stSize = sizeof(AlgoBump));
			break;
		case eAlgoNGBlob:
			memcpy_s(dst, sizeof(AlgoNGBlob), lpSource, stSize = sizeof(AlgoNGBlob));
			break;
		case eAlgoBodyEdge:
			memcpy_s(dst, sizeof(AlgoBodyEdge), lpSource, stSize = sizeof(AlgoBodyEdge));
			break;
			//shw Delete Xcase eAlgoNewAlgo1:#@dst = g_pMManager->pem_new<AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);#@memcpy_s(dst, sizeof(AlgoNewAlgo1), lpSource, sizeof(AlgoNewAlgo1));#@break;
		default:
			break;
		}
		return stSize;
	}





	ImgBuff::ImgBuff()
		: _qwCurrentPos(0)
	{
	}
	bool ImgBuff::Init()
	{
		ext::Log::add(_T("Image::Init() - Start"));

		bool bInit = true;

		CString sMemName;
		sMemName.Format(_T("ExtTool_%d_ImageIdx_%d"), env::nTool_id, nBuffIdx);
		if (_stream.AllocMemory(env::nTotalImgBufferSize, sMemName) == false)
		{
			if (_stream.Open(env::nTotalImgBufferSize, sMemName, FILE_MAP_ALL_ACCESS) == false)
				bInit = false;
			if (_stream.MapViewAll(FILE_MAP_ALL_ACCESS) == false)
				bInit = false;
		}

		ext::Log::add(_T("Image::Init() - End"));

		return bInit;

	}

	bool ImgBuff::AddImage(InspRoiImgBuf * partImgBuf, ZmapData*  partZmapData)
	{
		bool bRet = true;
		IppStatus sts;

		tagImgHead head;
		head.nImgWidth = partImgBuf->nImageSizeX;
		head.nImgHeight = partImgBuf->nImageSizeY;

		IppiSize iSize = { head.nImgWidth, head.nImgHeight };
		QWORD qwImgSize = head.nImgWidth * head.nImgHeight;

		BYTE * ptr = _stream.Byte(0);

		//zero, 사이즈정보는 헤더에서 가지고있고 몸체는 이미지 데이터만 저장함.
		int szHead = sizeof(tagImgHead);
		memcpy(ptr, &head, szHead); _qwCurrentPos += szHead;

		//2D image
		sts = ippiCopy_8u_C1R(ptr + _qwCurrentPos, head.nImgWidth, partImgBuf->imgTop_R, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(ptr + _qwCurrentPos, head.nImgWidth, partImgBuf->imgTop_G, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(ptr + _qwCurrentPos, head.nImgWidth, partImgBuf->imgTop_B, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(ptr + _qwCurrentPos, head.nImgWidth, partImgBuf->imgTop_W, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(ptr + _qwCurrentPos, head.nImgWidth, partImgBuf->imgMiddle_R, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(ptr + _qwCurrentPos, head.nImgWidth, partImgBuf->imgMiddle_B, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(ptr + _qwCurrentPos, head.nImgWidth, partImgBuf->imgBottom_R, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(ptr + _qwCurrentPos, head.nImgWidth, partImgBuf->imgBottom_B, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;

		//3D Image
		//memcpy(ptr + _qwCurrentPos, partZmapData->data, qwImgSize * sizeof(float)); _qwCurrentPos += qwImgSize * sizeof(float);
		
		sts = ippiCopy_32f_C1R( partZmapData->data , head.nImgWidth * sizeof(ipp32f), (float*)(ptr + _qwCurrentPos), head.nImgWidth * sizeof(ipp32f), iSize); _qwCurrentPos += qwImgSize * sizeof(ipp32f);
		
		_stream.Flush(ptr, _qwCurrentPos);

		return bRet;
	}
	bool ImgBuff::GetImage(InspRoiImgBuf * partImgBuf, ZmapData*  partZmapData)
	{
		bool bRet = true;

		IppStatus sts;
		tagImgHead head;
		BYTE * ptr = _stream.Byte(0);
		_qwCurrentPos = 0;
		memcpy(&head, &ptr, sizeof(tagImgHead)); _qwCurrentPos += sizeof(tagImgHead);


		partImgBuf->nImageSizeX = head.nImgWidth;
		partImgBuf->nImageSizeY = head.nImgHeight;
		partZmapData->zmapSizeX = head.nImgWidth;
		partZmapData->zmapSizeY = head.nImgHeight;
		IppiSize iSize = { head.nImgWidth, head.nImgHeight };
		QWORD qwImgSize = partImgBuf->nImageSizeX * partImgBuf->nImageSizeY;
		
		//2D
		sts = ippiCopy_8u_C1R(partImgBuf->imgTop_R, head.nImgWidth, ptr + _qwCurrentPos, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(partImgBuf->imgTop_G, head.nImgWidth, ptr + _qwCurrentPos, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(partImgBuf->imgTop_B, head.nImgWidth, ptr + _qwCurrentPos, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(partImgBuf->imgTop_W, head.nImgWidth, ptr + _qwCurrentPos, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(partImgBuf->imgMiddle_R, head.nImgWidth, ptr + _qwCurrentPos, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(partImgBuf->imgMiddle_B, head.nImgWidth, ptr + _qwCurrentPos, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(partImgBuf->imgBottom_R, head.nImgWidth, ptr + _qwCurrentPos, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;
		sts = ippiCopy_8u_C1R(partImgBuf->imgBottom_B, head.nImgWidth, ptr + _qwCurrentPos, head.nImgWidth, iSize); _qwCurrentPos += qwImgSize;

		//3D
		sts = ippiCopy_32f_C1R((float*)(ptr + _qwCurrentPos), head.nImgWidth * sizeof(ipp32f), partZmapData->data, head.nImgWidth * sizeof(ipp32f), iSize); _qwCurrentPos += qwImgSize * sizeof(ipp32f);


		return bRet;
	}



	RstBuff::RstBuff()
		: _qwCurrentPos(0)
	{
	}
	bool RstBuff::Init()
	{
		ext::Log::add(_T("Result::Init() - Start"));

		bool bInit = true;

		CString sMemName;
		sMemName.Format(_T("ExtTool_%d_ResultIdx_%d"), env::nTool_id, nBuffIdx);
		if (_stream.AllocMemory(env::nTotalRstBufferSize, sMemName) == false)
		{
			if (_stream.Open(env::nTotalRstBufferSize, sMemName, FILE_MAP_ALL_ACCESS) == false)
				bInit = false;
			if (_stream.MapViewAll(FILE_MAP_ALL_ACCESS) == false)
				bInit = false;
		}

		ext::Log::add(_T("Result::Init() - End"));

		return bInit;

	}
	void RstBuff::AddRst(InspectionResult * PartData, int nWndCnt)
	{
		/*_qwCurrentPos = 0;
		BYTE* ptr = _stream.Byte(0);

		if (PartData->isInspMount)
		{
			InspWndResult * wnd = PartData->mountResult;
			memcpy(ptr + _qwCurrentPos, &PartData->mountResult, );
		}

		if (PartData->isInspAlign)
		{
			InspWndResult * wnd = _Windows.Add(ptr->alignResult, PartData->alignArraySize);
			ptr->alignResult_extif = (InspWndResult *)AddrCalc::Calc_Sub_Offset(_Windows.First(), wnd);
			InspParamTemp * tmp = _ParamTemp.Add(ptr->alignParamTemp, PartData->alignArraySize);
			ptr->alignParamTemp_extif = (InspParamTemp *)AddrCalc::Calc_Sub_Offset(_ParamTemp.First(), tmp);
		}

		if (PartData->isInspOcr)
		{
			InspWndResult * wnd = _Windows.Add(ptr->ocrResult, PartData->ocrArraySize);
			ptr->ocrResult_extif = (InspWndResult *)AddrCalc::Calc_Sub_Offset(_Windows.First(), wnd);
			InspParamTemp * tmp = _ParamTemp.Add(ptr->ocrParamTemp, PartData->ocrArraySize);
			ptr->ocrParamTemp_extif = (InspParamTemp *)AddrCalc::Calc_Sub_Offset(_ParamTemp.First(), tmp);
		}

		if (PartData->isInspLeadSolder)
		{
			InspWndResult * wnd = _Windows.Add(ptr->leadSolderResult, PartData->leadSolderArraySize);
			ptr->leadSolderResult_extif = (InspWndResult *)AddrCalc::Calc_Sub_Offset(_Windows.First(), wnd);
			InspParamTemp * tmp = _ParamTemp.Add(ptr->leadSolderParamTemp, PartData->leadSolderArraySize);
			ptr->leadSolderParamTemp_extif = (InspParamTemp *)AddrCalc::Calc_Sub_Offset(_ParamTemp.First(), tmp);
		}

		if (PartData->isInspSolder)
		{
			InspWndResult * wnd = _Windows.Add(ptr->solderResult, PartData->solderArraySize);
			PartData->solderResult_extif = (InspWndResult *)AddrCalc::Calc_Sub_Offset(_Windows.First(), wnd);
			InspParamTemp * tmp = _ParamTemp.Add(ptr->solderParamTemp, PartData->solderArraySize);
			PartData->solderParamTemp_extif = (InspParamTemp *)AddrCalc::Calc_Sub_Offset(_ParamTemp.First(), tmp);
		}

		if (PartData->isInspTab)
		{
			InspWndResult * wnd = _Windows.Add(ptr->tabResult, PartData->tabArraySize);
			ptr->tabResult_extif = (InspWndResult *)AddrCalc::Calc_Sub_Offset(_Windows.First(), wnd);
			InspParamTemp * tmp = _ParamTemp.Add(ptr->tabParamTemp, PartData->tabArraySize);
			ptr->tabParamTemp_extif = (InspParamTemp *)AddrCalc::Calc_Sub_Offset(_ParamTemp.First(), tmp);
		}

		if (PartData->isInspS_Ball)
		{
			InspWndResult * wnd = _Windows.Add(ptr->S_BallResult, PartData->S_BallArraySize);
			ptr->S_BallResult_extif = (InspWndResult *)AddrCalc::Calc_Sub_Offset(_Windows.First(), wnd);
			InspParamTemp * tmp = _ParamTemp.Add(ptr->S_BallParamTemp, PartData->S_BallArraySize);
			ptr->S_BallParamTemp_extif = (InspParamTemp *)AddrCalc::Calc_Sub_Offset(_ParamTemp.First(), tmp);
		}

		if (PartData->isInspPad)
		{
			InspWndResult * wnd = _Windows.Add(ptr->PadResult, PartData->PadArraySize);
			PartData->PadResult_extif = (InspWndResult *)AddrCalc::Calc_Sub_Offset(_Windows.First(), wnd);
			InspParamTemp * tmp = _ParamTemp.Add(ptr->PadParamTemp, PartData->PadArraySize);
			PartData->PadParamTemp_extif = (InspParamTemp *)AddrCalc::Calc_Sub_Offset(_ParamTemp.First(), tmp);
		}

		if (PartData->isInspBGA)
		{
			InspWndResult * wnd = _Windows.Add(ptr->BGAResult, PartData->BGAArraySize);
			ptr->BGAResult_extif = (InspWndResult *)AddrCalc::Calc_Sub_Offset(_Windows.First(), wnd);
			InspParamTemp * tmp = _ParamTemp.Add(ptr->BGAParamTemp, PartData->BGAArraySize);
			ptr->BGAParamTemp_extif = (InspParamTemp *)AddrCalc::Calc_Sub_Offset(_ParamTemp.First(), tmp);
		}

		_stream.Flush(ptr, sizeof(InspPartInfo)*Count);*/
	}
	void RstBuff::GetRst(InspectionResult * PartRst, int nWndCnt)
	{

	}
	void RstBuff::AddRstAlgo(InspAlgoResult* AlgoData)
	{

	}



	LightBuff::LightBuff()
		: _qwCurrentPos(0)
	{
	}
	bool LightBuff::Init()
	{
		ext::Log::add(_T("Light::Init() - Start"));

		bool bInit = true;

		CString sMemName;
		sMemName.Format(_T("ExtTool_%d_LightIdx_%d"), env::nTool_id, nBuffIdx);
		if (_stream.AllocMemory(env::nTotalLightBufferSize, sMemName) == false)
		{
			if (_stream.Open(env::nTotalLightBufferSize, sMemName, FILE_MAP_ALL_ACCESS) == false)
				bInit = false;
			if (_stream.MapViewAll(FILE_MAP_ALL_ACCESS) == false)
				bInit = false;
		}

		ext::Log::add(_T("Light::Init() - End"));

		return bInit;

	}
	bool LightBuff::AddLight(InspPartParam * pParamArray, int nWndCnt)
	{
		bool bRet = true;

		_qwCurrentPos = 0;
		BYTE* ptr = _stream.Byte(_qwCurrentPos);

		for (int wnd = 0; wnd < nWndCnt; wnd++)
		{
			for (int algo = 0; pParamArray[wnd].nAlgorithmCnt; algo++)
			{
				ExtAlgoLight stAlgoLight = Add(&pParamArray[wnd].vArrAlgoParam[algo], ptr, wnd, algo);
				//
				memcpy(ptr + _qwCurrentPos, &stAlgoLight, sizeof(ExtAlgoLight)); _qwCurrentPos += sizeof(ExtAlgoLight);
			}
		}

		return bRet;
	}
	bool LightBuff::GetLight(InspPartParam * pParamArray, int nWndCnt)
	{
		bool bRet = true;

		_qwCurrentPos = 0;
		BYTE* ptr = _stream.Byte(_qwCurrentPos);

		for (int wnd = 0; wnd < nWndCnt; wnd++)
		{
			for (int algo = 0; algo < pParamArray[wnd].nAlgorithmCnt; algo++)
			{
				Get(&pParamArray[wnd].vArrAlgoParam[algo]);
			}
		}

		return bRet;
	}
	void LightBuff::Get(InspAlgo * pAlgo)
	{
		memcpy(&stCurLightBuffer, _stream.Byte(_qwCurrentPos), sizeof(ExtAlgoLight));	_qwCurrentPos += sizeof(ExtAlgoLight);

		pAlgo->m_eLightType		= stCurLightBuffer.m_stLight.m_eLightType;
		pAlgo->m_nRedValue		= stCurLightBuffer.m_stLight.m_nRedValue;
		pAlgo->m_nGreenValue	= stCurLightBuffer.m_stLight.m_nGreenValue;
		pAlgo->m_nBlueValue		= stCurLightBuffer.m_stLight.m_nBlueValue;
		pAlgo->m_nWhiteValue	= stCurLightBuffer.m_stLight.m_nWhiteValue;

		pAlgo->m_nArrRedValue		= stCurLightBuffer.m_stLight.m_nArrRedValue;
		pAlgo->m_nArrGreenValue		= stCurLightBuffer.m_stLight.m_nArrGreenValue;
		pAlgo->m_nArrBlueValue		= stCurLightBuffer.m_stLight.m_nArrRedValue;
		pAlgo->m_nArrWhiteValue		= stCurLightBuffer.m_stLight.m_nArrRedValue;
		pAlgo->m_nArrCalculation	= stCurLightBuffer.m_stLight.m_nArrRedValue;
		pAlgo->m_nArrLightPosition	= stCurLightBuffer.m_stLight.m_nArrRedValue;

		if (pAlgo->m_nMixCount > 0)
		{
			for (int i = 0; i < pAlgo->m_nMixCount; pAlgo++)
			{
				pAlgo->InspAlgoLightsMix[i].m_eLightType			= stCurLightBuffer.m_stMixLight[i].m_eLightType;
				pAlgo->InspAlgoLightsMix[i].m_nImageNum				= stCurLightBuffer.m_stMixLight[i].m_nImageNum;
				pAlgo->InspAlgoLightsMix[i].m_nLightCnt				= stCurLightBuffer.m_stMixLight[i].m_nLightCnt;
				pAlgo->InspAlgoLightsMix[i].m_nArrRedValue			= stCurLightBuffer.m_stMixLight[i].m_nArrRedValue;
				pAlgo->InspAlgoLightsMix[i].m_nArrGreenValue		= stCurLightBuffer.m_stMixLight[i].m_nArrGreenValue;
				pAlgo->InspAlgoLightsMix[i].m_nBlueValue			= stCurLightBuffer.m_stMixLight[i].m_nBlueValue;
				pAlgo->InspAlgoLightsMix[i].m_nWhiteValue			= stCurLightBuffer.m_stMixLight[i].m_nWhiteValue;
				pAlgo->InspAlgoLightsMix[i].m_nArrCalculation		= stCurLightBuffer.m_stMixLight[i].m_nArrCalculation;
				pAlgo->InspAlgoLightsMix[i].m_nArrLightPosition		= stCurLightBuffer.m_stMixLight[i].m_nArrLightPosition;
			}
		}

	}
	ExtAlgoLight LightBuff::Add(InspAlgo * pAlgo, BYTE* ptr, int nWndID, int nAlgoID)
	{
		ExtAlgoLight stAlgoLight;

		stAlgoLight.m_nWndID = nWndID + 1;
		stAlgoLight.m_nAlgoID = nAlgoID + 1;
		stAlgoLight.m_stLight.CloneLight(pAlgo);

		stAlgoLight.m_nMixCnt = pAlgo->m_nMixCount;
		if (stAlgoLight.m_nMixCnt > 0)
		{
			for (int i = 0; i < stAlgoLight.m_nMixCnt; i++)
				stAlgoLight.m_stMixLight[i].CloneMixLight(&pAlgo->InspAlgoLightsMix[i]);
		}

		return stAlgoLight;
	}





	InspBuffer::InspBuffer()
	{
	}
	bool InspBuffer::Init()
	{
		bool bRst = true;

		SetBuffIdx();
		for (int i = 0; i < EXT_BUFFER_CNT; i++)
		{
			bRst = _part[i].Init();
			bRst = _img[i].Init();
			bRst = _rst[i].Init();
			bRst = _light[i].Init();
		}

		return bRst;
	}

	ClientCtrl::ClientCtrl()
		:_nTool_id(0)
	{

	}
	bool ClientCtrl::Init(int nTool_id)
	{
		_nTool_id = nTool_id;

		_ctrl.Init(nTool_id);

		_inspbuffer.Init();

		Execute();

		return true;
	}
	void ClientCtrl::Exit()
	{

	}
	void ClientCtrl::Execute()
	{

		assert(EnvVariable::sAlgoToolPath.IsEmpty() == false);
		CString sExecuteParam;
		sExecuteParam.Format(_T("INLINE_%d"), _nTool_id);
		//::ShellExecute(nullptr, _T("open"), EnvVariable::sAlgoToolPath, sExecuteParam, nullptr, SW_SHOW);

		SHELLEXECUTEINFO info = { sizeof(SHELLEXECUTEINFO) };

		info.lpVerb = TEXT("runas");

		info.lpFile = EnvVariable::sAlgoToolPath;
		info.lpParameters = sExecuteParam;

		info.nShow = SW_SHOW;

		ShellExecuteEx(&info);
	}


}
}