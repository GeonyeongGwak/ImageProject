#include "stdafx.h"
#include "ExtLog.h"

#include "ExtInspSub.h"
#include "ipp.h"
#include <iostream>
#include <string>
#include <locale>
#include <filesystem>
namespace ext {

	InspRoot::InspRoot()
		: _bInitialized(false)
	{
		_TotalBuff = 0; 
	}
	bool InspRoot::bInitialized()
	{
		return _bInitialized;
	}

	InspRoot_Server::Ptr InspRoot_Server::s_Obj;

	InspRoot_Server::Ptr InspRoot_Server::get()
	{
		if (s_Obj == nullptr)
		{
			s_Obj = std::shared_ptr<InspRoot_Server>(new InspRoot_Server());
		}
		return s_Obj;
	}

	InspRoot_Client::Ptr InspRoot_Client::s_Obj;

	InspRoot_Client::Ptr InspRoot_Client::get()
	{
		if (s_Obj == nullptr)
		{
			s_Obj = std::shared_ptr<InspRoot_Client>(new InspRoot_Client());
		}
		return s_Obj;
	}

	ClientCtrl::Ptr ClientCtrl::s_Obj;

	ClientCtrl::Ptr ClientCtrl::get()
	{
		
		if (s_Obj== nullptr)
		{
			s_Obj = std::shared_ptr<ClientCtrl>(new ClientCtrl());
		}
		return s_Obj;
	}
	StartInit::Ptr StartInit::s_Obj;

	StartInit::Ptr StartInit::get()
	{

		if (s_Obj == nullptr)
		{
			s_Obj = std::shared_ptr<StartInit>(new StartInit());
		}
		return s_Obj;
	}
	FrameFlagWriter::Ptr FrameFlagWriter::s_Obj;

	FrameFlagWriter::Ptr FrameFlagWriter::get()
	{

		if (s_Obj == nullptr)
		{
			s_Obj = std::shared_ptr<FrameFlagWriter>(new FrameFlagWriter());
		}
		return s_Obj;
	}

	FrameWriters::Ptr FrameWriters::s_Obj;

	FrameWriters::Ptr FrameWriters::get()
	{

		if (s_Obj == nullptr)
		{
			s_Obj = std::shared_ptr<FrameWriters>(new FrameWriters());
		}
		return s_Obj;
	}

	


	bool InspRoot_Server::isLive()
	{
		if (s_Obj == nullptr || s_Obj->bInitialized()==false)
		{
			return false;
		}
		return true;
	}




	int InspRoot_Server::GetRemainRstCount()
	{
		int nCount(0);
		int nNumofClient = _Ctrl.First()->NumOfClients;
		for (int t = 0; t < nNumofClient; t++)
		{
			int nRstLoopCnt = EXT_BUFFER_RST_CNT;
			switch (m_vToolIndexSort[t])
			{
			case  (int)eToolType::eNORMAL:
				break;
			case (int)eToolType::eBIG:
				nRstLoopCnt = EXT_BIG_BUFFER_RST_CNT;
				break;
			case (int)eToolType::eForeignType:
				continue;
				break;
			}

			//int nRstLoopCnt = EXT_BUFFER_RST_CNT;
			//if (t == nNumofClient - 1)
			//	nRstLoopCnt = EXT_BIG_BUFFER_CNT;

			for (int i = 0; i < nRstLoopCnt; i++)
			{
				nCount += _Tools[t]._Rst[i]._nRecvRstCount;
			}
		}
		int nPartTotalCnt = irs::get()->_Ctrl.First()->prod.partTotalCount;
		int nSkipPartCnt = irs::get()->_Ctrl.First()->nInspSkipCnt;
		return (nPartTotalCnt - nSkipPartCnt)- nCount;
	}

	bool InspRoot_Server::GetPartResult(int nLane,int *nExtDefectCode, InspectionResult* mRst)
	{
		if (ext::env::bUseSystem == false)
			return false;

		bool bRet = false;

		int nNumofClient = _Ctrl.First()->NumOfClients;
		for (int t = 0; t < nNumofClient; t++)
		{
			int nRstLoopCnt = EXT_BUFFER_RST_CNT;
			switch (m_vToolIndexSort[t])
			{
			case  (int)eToolType::eNORMAL:
				break;
			case (int)eToolType::eBIG:
				nRstLoopCnt = EXT_BIG_BUFFER_RST_CNT;
				break;
			case (int)eToolType::eForeignType:
				continue;
				break;
			}

			//Normal, Big 용 Tool은 버퍼 갯수가 서로 다르다.
			/*int nRstLoopCnt = EXT_BUFFER_RST_CNT;
			if (t == nNumofClient - 1)
				nRstLoopCnt = EXT_BIG_BUFFER_CNT;*/

			for (int n = 0; n < nRstLoopCnt; n++)
			{
				if (_Tools[t]._RevCtrl[n].GetFlag(BufferFlag::IF_ON) == true)
				{
					int nCalcCnt = _Tools[t]._RevCtrl[n].First()->nCalcDoneCount;
					bRet = _Tools[t]._Rst[n].GetNext(mRst, nCalcCnt, nExtDefectCode);
				}

				if (bRet)
				{
					//Receive 버퍼 상태 업데이트
					_Tools[t]._RevCtrl[n].SetFlag(BufferFlag::IF_OFF, true);
					break;
				}
			}
			if (bRet)
				break;
		}

		return bRet;
	}
	int InspRoot_Server::GetRemainForeignRstCount()
	{
		int nCount(0);
		//int nNumofClient = _Ctrl.First()->NumOfClients - 1; // 이물검사는 BigBuffer 에서만.


		for (std::vector<int>::iterator it = ext::irs::get()->m_vToolForeign.begin(); it != ext::irs::get()->m_vToolForeign.end(); ++it)
		{
			for (int n = 0; n < EXT_BUFFER_FOREIGN_RST_CNT; n++)
			{
				bool bToolEnd = _Tools[*it]._RevForeignCtrl[n].GetFlag(ext::BufferFlag::IF_CALCEND);
				bool bEagle3DGetEnd = _Tools[*it]._SndForeignCtrl[n].GetFlag(ext::BufferFlag::IF_CALCEND);

				if (bToolEnd && bEagle3DGetEnd)
					nCount++;
			}

		}

		
		int nPartTotalCnt = irs::get()->_Ctrl.First()->prod.nFovTotalCnt;
		int nSkipPartCnt = irs::get()->_Ctrl.First()->nInspForeignSkipCnt;
		return (nPartTotalCnt - nSkipPartCnt) - nCount;
	}
	bool InspRoot_Server::GetForeignResult(int* nSelectBuffer, int* nExtDefectCode, InspFovForeignResult*& rst)
	{
		if (ext::env::bUseSystem == false)
			return false;

		bool bRet = false;
	/*	CString sLog; 
		sLog.Format(_T("Start Get Foreign"));
		ext::Log::add(sLog);*/
		int nNumofClient = _Ctrl.First()->NumOfClients -1 ; // 이물검사는 BigBuffer 에서만.
		//for (int t = 0; t < nNumofClient; t++)
		//{
			//Normal, Big 용 Tool은 버퍼 갯수가 서로 다르다.
			/*int nRstLoopCnt = EXT_BUFFER_RST_CNT;
			if (t == nNumofClient - 1)
				nRstLoopCnt = EXT_BIG_BUFFER_CNT;*/

			for (int n = 0; n < EXT_BUFFER_FOREIGN_RST_CNT; n++)
			{

				nNumofClient = ext::irs::get()->m_vToolForeign[n % ext::env::nForeignTypeToolCnt];

				//CString sLogs;
				//sLogs.Format(_T("[Ext][Set][Foreign][Result][SUB] nTool %d, ForePartInfo->fovIndex %d ,  nFOV %d "), nNumofClient, n, ext::irs::get()->_nFov);
				//ext::Log::add(sLogs);


				bool bToolEnd = _Tools[nNumofClient]._RevForeignCtrl[n].GetFlag(ext::BufferFlag::IF_CALCEND);
				bool bEagle3DGetEnd = _Tools[nNumofClient]._SndForeignCtrl[n].GetFlag(ext::BufferFlag::IF_CALCEND);

				/*sLog.Format(_T("Get Foreign TOOL :%s, EAGLE: %s , index %d"), (bToolEnd ? _T("true") : _T("false"))  , (bEagle3DGetEnd ? _T("true") : _T("false")) ,n);
				ext::Log::add(sLog);*/

				if ((bToolEnd && bEagle3DGetEnd))
					continue;
				if (bToolEnd)
				{
					bRet = _Tools[nNumofClient]._ForeignSheradRst[n].GetNext(rst/*, nCalcCnt*/, nExtDefectCode);
					_Tools[nNumofClient]._SndForeignCtrl[n].SetFlag(ext::BufferFlag::IF_CALCEND, true);

					//sLog.Format(_T("Get Foreign TOOL :%s, EAGLE: %s ,Rst Index %d"), (bToolEnd ? _T("true") : _T("false")), (bEagle3DGetEnd ? _T("true") : _T("false")), n);
					//ext::Log::add(sLog);
					
				}
				else
				{

				}
				

				if (bRet)
				{
					*nSelectBuffer = n; 
					//Receive 버퍼 상태 업데이트// 이물검사는 버퍼를 비워주지않는다.
					//_Tools[t]._RevCtrl[n].SetFlag(BufferFlag::IF_OFF, true);
					break;
				}
			}
	/*		if (bRet)
				break;*/
		//}

			//sLog.Format(_T("End Foreign Rest :%s"), (bRet ? _T("true") : _T("false")));
			//ext::Log::add(sLog);
		return bRet;
	}
	bool InspRoot_Server::GetClientDone()
	{
		bool bRet = true;

		for (int i = 0; i < _Ctrl.First()->NumOfClients; i++)
		{
			if (_Tools[i]._ctrl.GetSignal(eSlvSignal::eSlvAutoMode) == true)
				bRet = false;
		}

		return bRet;
	}
	void CopyParamToExt(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize)
	{
		boardInfo->nWindowCount = nParamArraySize;
		boardInfo->pWindows = ExtPrmPtr(boardInfo->pWindows) = pParamArray; // ÆÄÆ® º° À©µµ¿ì Æ÷ÀÎÅÍ

		for (int w = 0; w < nParamArraySize; w++)
		{
			ExtPrmPtr(boardInfo->pWindows[w].vArrAlgoParam) = boardInfo->pWindows[w].vArrAlgoParam; // À©µµ¿ì º° ¾Ë°í¸®Áò Æ÷ÀÎÅÍ

			for (int a = 0; a < boardInfo->pWindows[w].nAlgorithmCnt; a++) // algo
			{
				// ¾Ë°í¸®Áò º° ½ÇÁ¦ ¾Ë°í¸®Áò Æ÷ÀÎÅÍ
				ExtPrmPtr(boardInfo->pWindows[w].vArrAlgoParam[a].m_ptrInspAlgoParam) = boardInfo->pWindows[w].vArrAlgoParam[a].m_ptrInspAlgoParam;
			}
		}
	}
	int InspRoot_Server::GetInspDoneCnt()
	{
		int nCnt(0);
		
		int nToolCnt = env::nToolCount;
		for (int i = 0; i < nToolCnt; i++)
		{
			nCnt += _Tools[i]._ctrl.First()->CalcPartCount;
		}
		return nCnt;
	}
	int InspRoot_Server::GetInspImageSaveCnt()
	{
		int nCnt(0);

		int nToolCnt = env::nToolCount;
		for (int i = 0; i < nToolCnt; i++)
		{
			nCnt += _Tools[i]._ctrl.First()->nImageSaveCnt;
		}
		CString sLog; 
		sLog.Format(_T("[EXT][ImageThread]Insp Thread Image Cnt : %d"), nCnt);
		ext::Log::add(sLog);
		return nCnt;
	}
	int InspRoot_Server::GetForeignDoneCnt()
	{
		int nCnt(0);


		for (std::vector<int>::iterator it = ext::irs::get()->m_vToolForeign.begin(); it != ext::irs::get()->m_vToolForeign.end(); ++it)
		{
			for (int i = 0; i < EXT_BUFFER_FOREIGN_RST_CNT; i++)
			{

				bool bToolEnd = _Tools[*it]._RevForeignCtrl[i].GetFlag(ext::BufferFlag::IF_CALCEND);

				if (bToolEnd)
				{
					nCnt++;
				}
			}
		}
		return nCnt;
	}
	int InspRoot_Server::GetSaveCnt()
	{
		int nCnt(0);
		int nToolCnt = env::nToolCount;
		for (int i = 0; i < nToolCnt; i++)
		{
			int nRstLoopCnt = EXT_BUFFER_RST_CNT;
			switch (m_vToolIndexSort[i])
			{
			case  (int)eToolType::eNORMAL:
				//_Tools[i].BufferClear();
				break;
			case (int)eToolType::eBIG:
				nRstLoopCnt = EXT_BIG_BUFFER_RST_CNT;
				break;
			case (int)eToolType::eForeignType:
				continue;
				break;
			}

			//int nRstLoopCnt = EXT_BUFFER_RST_CNT;
			//if (i == nToolCnt - 1)
			//	nRstLoopCnt = EXT_BIG_BUFFER_CNT;
			for (int j = 0; j < nRstLoopCnt; j++)
			{
				//// 수정 필요 
				int nRevCnt = _Tools[i]._Rst[j]._nRecvRstCount;
				////
				int nPartFullCnt = _Tools[i]._RevCtrl[j].First()->nCalcDoneCount;
				nCnt += nPartFullCnt - nRevCnt;
			}
		}

		return nCnt;
	}
	int InspRoot_Server::GetNextToolIndex(bool bIsBigPart, bool bUseSchdule, int nLane, int nPartID, double& dExpectTime, int& nBuffer)
	{
		int nNumofClient = _Ctrl.First()->NumOfClients;
		int nSelectedToolIdx = _Ctrl.First()->nCurrToolIdx % (nNumofClient- (env::nForeignTypeToolCnt + env::nBigTypeToolCnt));
		int nCurIndex = _Ctrl.First()->nCurrToolIdx;
		int nCurIN = _Ctrl.First()->nCurrBigToolIdx;

		int nNormalToolCnt = (nNumofClient - (env::nForeignTypeToolCnt + env::nBigTypeToolCnt));
	
		int totalToolCnt = env::nToolCount;
		vector<int> bigPartIndexSorting;
		//빅파트일 경우는 스케줄링 사용유무 상관없음.
		if (bIsBigPart)
		{
			int nCurBigIndex = 0;
			if ((env::nBigTypeToolCnt) != 0)
				nCurBigIndex = nCurIN % (env::nBigTypeToolCnt);
			int nBigPartTypeCnt = (nNumofClient - env::nForeignTypeToolCnt);
			for (int i = 0; i < m_vToolIndexSort.size(); i++)
			{
				if (m_vToolIndexSort[i] == (int)eToolType::eBIG)
					bigPartIndexSorting.push_back(i);
			}
			if(bigPartIndexSorting.size() != 0 )
			{
				nSelectedToolIdx = bigPartIndexSorting[nCurBigIndex];
				nBuffer = ext::irs::get()->_Tools[nSelectedToolIdx].m_nCurrBufferIdx;
				nBuffer %= EXT_BIG_BUFFER_CNT;
				return nSelectedToolIdx;
			}
			else
			{
				CString sLog;


				sLog.Format(_T("[Ext][GetNextToolIndex] lost BigPart Size : %d , "), m_vToolIndexSort.size());
				for (int i = 0;i < m_vToolIndexSort.size(); i++)
				{
					sLog.Append(_T(",%d"), m_vToolIndexSort[i]);
				}
				ext::Log::add(sLog);
			}
		}
		
		if (bUseSchdule)
		{
			dExpectTime = ext::Sch::get()->GetExpectTime(nLane, nPartID);
			nSelectedToolIdx = ext::irs::get()->GetScheduleIndex();
		}
		//nSelectedToolIdx = _Ctrl.First()->nCurrToolIdx % (nNumofClient - 1);

		nBuffer = ext::irs::get()->_Tools[nSelectedToolIdx].m_nCurrBufferIdx;
		nBuffer %= EXT_BUFFER_CNT;

		return nSelectedToolIdx;
	}
	int InspRoot_Server::GetScheduleIndex()
	{
		int nRet = 0;

		int nNumofClient = _Ctrl.First()->NumOfClients;
		
		//Tool 1개면 스케줄링 할 필요없다.
		if (nNumofClient == 1)
			return 0;

		std::vector<double> vArrExpectTime;
		for (int i = 0; i < nNumofClient; i++)
		{
			double dTime = 0.0;
			dTime =_Tools[i].m_dExcpectInspTime;

			vArrExpectTime.emplace_back(dTime);
		}
		
		nRet = min_element(vArrExpectTime.begin(), vArrExpectTime.end()) - vArrExpectTime.begin();

		return nRet;
	}

	int __nWriteLogState(0);

	// If it need, this and hierarchical classes can be changed in ring buffer structure.



	bool InspRoot::Init()
	{
		bool bInit(true);
		if (EnvVariable::bUseSystem == false)
			return false;

		if (_POCRModelCtrl.Init(ext::ModelBufferFlag::eExtModel_POCR) == false)
			bInit = false;
		if (_POCRModelBuffer.Init(ext::ModelBufferFlag::eExtModel_POCR) == false)
			bInit = false;
		if (_PatternModelCtrl.Init(ext::ModelBufferFlag::eExtModel_Pattern) == false)
			bInit = false;
		if (_PatternModelBuffer.Init(ext::ModelBufferFlag::eExtModel_Pattern) == false)
			bInit = false;
		if (_COBModelCtrl.Init(ext::ModelBufferFlag::eExtModel_COB) == false)
			bInit = false;
		if (_COBModelBuffer.Init(ext::ModelBufferFlag::eExtModel_COB) == false)
			bInit = false;
		if (_ExcModelCtrl.Init(ext::ModelBufferFlag::eExtModel_Exc) == false)
			bInit = false;
		if (_ExcModelBuffer.Init(ext::ModelBufferFlag::eExtModel_Exc) == false)
			bInit = false;
		if (_PadBWModelCtrl.Init(ext::ModelBufferFlag::eExtModel_PadBW) == false)
			bInit = false;
		if (_PadBWModelBuffer.Init(ext::ModelBufferFlag::eExtModel_PadBW) == false)
			bInit = false;
		if (_ShapeXModelCtrl.Init(ext::ModelBufferFlag::eExtModel_ShapeX) == false)
			bInit = false;
		if (_ShapeXModelBuffer.Init(ext::ModelBufferFlag::eExtModel_ShapeX) == false)
			bInit = false;
		if (_HalconPOCRModelCtrl.Init(ext::ModelBufferFlag::eExtModel_HalconPOCR) == false)
			bInit = false;
		if (_ExcNGBlobModelCtrl.Init(ext::ModelBufferFlag::eExtModel_NGBlobExc) == false)
			bInit = false;
		if (_ExcNGBlobModelBuffer.Init(ext::ModelBufferFlag::eExtModel_NGBlobExc) == false)
			bInit = false;
		if (_InspStatus.Init() == false)
			bInit = false;

		CheckStAlgoSize();

		_TotalBuff = _POCRModelCtrl._TotalBuff +
			_POCRModelBuffer._TotalBuff +
			_PatternModelCtrl._TotalBuff +
			_PatternModelBuffer._TotalBuff +
			_COBModelCtrl._TotalBuff +
			_COBModelBuffer._TotalBuff +
			_ExcModelCtrl._TotalBuff +
			_ExcModelBuffer._TotalBuff +
			_PadBWModelCtrl._TotalBuff +
			_PadBWModelBuffer._TotalBuff +
			_ShapeXModelCtrl._TotalBuff +
			_ShapeXModelBuffer._TotalBuff +
			_InspStatus._TotalBuff;

		return bInit;
	}
	void InspRoot::Exit()
	{
		_POCRModelCtrl.Exit();
		_POCRModelBuffer.Exit();
		_PatternModelCtrl.Exit();
		_PatternModelBuffer.Exit();
		_COBModelCtrl.Exit();
		_COBModelBuffer.Exit();
		_ExcModelCtrl.Exit();
		_ExcModelBuffer.Exit();
		_PadBWModelCtrl.Exit();
		_PadBWModelBuffer.Exit();
		_ShapeXModelCtrl.Exit();
		_ShapeXModelBuffer.Exit();
		_InspStatus.Exit();
	}
	void InspRoot::AddPannelCount()
	{
		m_nCurrentIdx += 1;
	}
	void InspRoot::CheckStAlgoSize()
	{
		//Excel 비교를 위한 수정 
		CString msgParam = _T("");
		int nStAddCnt = 0;
		ext::Log::add(_T("[Ext]Check InspParam Size."));
		msgParam.AppendFormat(_T("InspPartParam,%d,"), sizeof(InspPartParam));
		msgParam.AppendFormat(_T("InspPartInfo,%d,"), sizeof(InspPartInfo));
		msgParam.AppendFormat(_T("ZmapData,%d,"), sizeof(ZmapData));
		msgParam.AppendFormat(_T("TeachParam,%d,"), sizeof(TeachParam));
		msgParam.AppendFormat(_T("Coordinate,%d,"), sizeof(Coordinate));
		msgParam.AppendFormat(_T("BodyOffset,%d,"), sizeof(BodyOffset));
		msgParam.AppendFormat(_T("AlignResult,%d,"), sizeof(AlignResult));
		msgParam.AppendFormat(_T("NgParam,%d,"), sizeof(NgParam));
		msgParam.AppendFormat(_T("CalcRoiParm,%d,"), sizeof(CalcRoiParm));
		msgParam.AppendFormat(_T("DefaultGrayMeanStd,%d,"), sizeof(DefaultGrayMeanStd));
		msgParam.AppendFormat(_T("DefaultColorXYStd,%d,"), sizeof(DefaultColorXYStd));
		msgParam.AppendFormat(_T("AlgoDebuggingMode,%d,"), sizeof(AlgoDebuggingMode));
		msgParam.AppendFormat(_T("InspAlgo,%d,"), sizeof(InspAlgo));
		ext::Log::add(msgParam);

		CString msg = _T("");
		ext::Log::add(_T("[Ext]Check Algo Struct Size."));

		for (int i = 0; i < (int)eAlgoNum - 1; i++)
		{
			switch (i)
			{
			case eAlgoAlign:
				msg.AppendFormat(_T("AlgoAlign,%d,"), sizeof(AlgoAlign));
				break;
			case eAlgoBW:
				msg.AppendFormat(_T("AlgoBW,%d,"), sizeof(AlgoBW));
				break;
			case eAlgoBlob:
				msg.AppendFormat(_T("AlgoBlob,%d,"), sizeof(AlgoBlob));
				break;
			case eAlgoBody_Blob:
				msg.AppendFormat(_T("AlgoBodyBlob,%d,"), sizeof(AlgoBodyBlob));
				break;
			case eAlgoTilt:
				msg.AppendFormat(_T("AlgoTilt,%d,"), sizeof(AlgoTilt));
				break;
			case eAlgoOCR:
				msg.AppendFormat(_T("AlgoOCR,%d,"), sizeof(AlgoOCR));
				break;
			case eAlgoPattern:
				msg.AppendFormat(_T("AlgoPattern,%d,"), sizeof(AlgoPattern));
				break;
			case eAlgoColor:
				msg.AppendFormat(_T("AlgoColor,%d,"), sizeof(AlgoColor));
				break;
			case eAlgoLead_Color:
				msg.AppendFormat(_T("AlgoLeadColor,%d,"), sizeof(AlgoColor));
				break;
			case eAlgoGray_Mean:
				msg.AppendFormat(_T("AlgoGrayMean,%d,"), sizeof(AlgoGrayMean));
				break;
			case eAlgoHeight_Mean:
				msg.AppendFormat(_T("AlgoHeightMean,%d,"), sizeof(AlgoHeightMean));
				break;
			case eAlgoGray_Diff:
				msg.AppendFormat(_T("AlgoGrayDiff,%d,"), sizeof(AlgoGrayDiff));
				break;
			case eAlgoHeight_Diff:
				msg.AppendFormat(_T("AlgoHeightDiff,%d,"), sizeof(AlgoHeightDiff));
				break;
			case eAlgoLead_Search:
				msg.AppendFormat(_T("AlgoLeadSearch,%d,"), sizeof(AlgoLeadSearch));
				break;
			case eAlgoTab_Search:
				msg.AppendFormat(_T("AlgoTabSearch,%d,"), sizeof(AlgoLeadSearch));
				break;
			case eAlgoBridge:
				msg.AppendFormat(_T("AlgoBridge,%d,"), sizeof(AlgoBridge));
				break;
			case eAlgoLead_Tip:
				msg.AppendFormat(_T("AlgoLeadTip,%d,"), sizeof(AlgoLeadTip));
				break;
			case eAlgoLead_Lift:
				msg.AppendFormat(_T("AlgoLeadLift,%d,"), sizeof(AlgoLeadLift));
				break;
			case eAlgoLead_Solder:
				msg.AppendFormat(_T("AlgoLeadSolder,%d,"), sizeof(AlgoLeadSolder));
				break;
			case eAlgoLead_SideSolder:
				msg.AppendFormat(_T("AlgoLeadSideSolder,%d,"), sizeof(AlgoLeadSideSolder));
				break;
			case eAlgoWidth:
				msg.AppendFormat(_T("AlgoWidth,%d,"), sizeof(AlgoLength));
				break;
			case eAlgoVolume:
				msg.AppendFormat(_T("AlgoVolum,%d,"), sizeof(AlgoVolume));
				break;
			case eAlgoTab:
				msg.AppendFormat(_T("AlgoTab,%d,"), sizeof(AlgoTab));
				break;
			case eAlgoGrid:
				msg.AppendFormat(_T("AlgoGrid,%d,"), sizeof(AlgoGrid));
				break;
			case eAlgoLine:
				msg.AppendFormat(_T("AlgoLine,%d,"), sizeof(AlgoLine));
				break;
			case eAlgoEdge:
				msg.AppendFormat(_T("AlgoEdge,%d,"), sizeof(AlgoEdge));
				break;
			case eAlgoSolderCone:
				msg.AppendFormat(_T("AlgoSolderCone,%d,"), sizeof(AlgoSolderCone));
				break;
			case eAlgoColorXY:
				msg.AppendFormat(_T("AlgoColorXY,%d,"), sizeof(AlgoColorXY));
				break;
			case eAlgoAlignEdge:
				msg.AppendFormat(_T("AlgoAlignEdge,%d,"), sizeof(AlgoAlignEdge));
				break;
			case eAlgoPadAlign:
				msg.AppendFormat(_T("AlgoPadAlign,%d,"), sizeof(AlgoPadAlign));
				break;
			case eAlgoPOCR:
				msg.AppendFormat(_T("AlgoPOCR,%d,"), sizeof(AlgoPOCR));
				break;
			case eAlgoWire:
				msg.AppendFormat(_T("AlgoWire,%d,"), sizeof(AlgoWire));
				break;
			case eAlgoFoot:
				msg.AppendFormat(_T("AlgoFoot,%d,"), sizeof(AlgoFoot));
				break;
			case eAlgoBarcode:
				msg.AppendFormat(_T("AlgoBarcode,%d,"), sizeof(AlgoBarcode));
				break;
			case eAlgoFillet:
				msg.AppendFormat(_T("AlgoFillet,%d,"), sizeof(AlgoFillet));
				break;
			case eAlgoBGA:
				msg.AppendFormat(_T("AlgoBGA,%d,"), sizeof(AlgoBGA));
				break;
			case eAlgoBump:
				msg.AppendFormat(_T("AlgoBump,%d,"), sizeof(AlgoBump));
				break;
			case eAlgoNGBlob:
				msg.AppendFormat(_T("AlgoNGBlob,%d,"), sizeof(AlgoNGBlob));
				break;
			case eAlgoPadBW:
				msg.AppendFormat(_T("AlgoPadBW,%d,"), sizeof(AlgoPadBW));
				break;
			case eAlgoBodyEdge:
				msg.AppendFormat(_T("AlgoBodyEdge,%d,"), sizeof(AlgoBodyEdge));
				break;
			}

			// 			nStAddCnt++;
			// 			if (nStAddCnt % 5 == 0)
			// 			{
			// 				ext::Log::add(msg);
			// 				msg.Format(_T(""));
			// 				nStAddCnt = 0;
			// 			}

		}
		// 		if (nStAddCnt != 0)
		// 		{
		// 			nStAddCnt = 0;
		ext::Log::add(msg);
		/*		}*/


		CString msgParamRst = _T("");
		ext::Log::add(_T("[Ext]Rst InspParam Size."));
		msgParamRst.AppendFormat(_T("InspAlgoTempResult,%d,"), sizeof(InspAlgoTempResult));
		msgParamRst.AppendFormat(_T("InspAlgoResult,%d,"), sizeof(InspAlgoResult));
		msgParamRst.AppendFormat(_T("InspWndResult,%d,"), sizeof(InspWndResult));
		msgParamRst.AppendFormat(_T("InspectionResult,%d,"), sizeof(InspectionResult));
		msgParamRst.AppendFormat(_T("tagInspBarcodeParam,%d,"), sizeof(tagInspBarcodeParam));
		msgParamRst.AppendFormat(_T("ParamInfo,%d,"), sizeof(ParamInfo));

		ext::Log::add(msgParamRst);
		CString msgRst = _T("");
		ext::Log::add(_T("[Ext]Check RstAlgo Struct Size Start."));

		for (int i = 0; i < (int)eAlgoNum - 1; i++)
		{
			switch (i)
			{
			case eAlgoAlign:
				msgRst.AppendFormat(_T("RstAlgoAlign,%d,"), sizeof(RstAlgoAlign));
				break;
			case eAlgoBW:
				msgRst.AppendFormat(_T("RstAlgoBlackWhite,%d,"), sizeof(RstAlgoBlackWhite));
				break;
			case eAlgoBlob:
				msgRst.AppendFormat(_T("RstAlgoBlob,%d,"), sizeof(RstAlgoBlob));
				break;
			case eAlgoBody_Blob:
				msgRst.AppendFormat(_T("RstAlgoBodyBlob,%d,"), sizeof(RstAlgoBodyBlob));
				break;
			case eAlgoTilt:
				msgRst.AppendFormat(_T("RstAlgoTilt,%d,"), sizeof(RstAlgoTilt));
				break;
			case eAlgoOCR:
				msgRst.AppendFormat(_T("RstAlgoOCR,%d,"), sizeof(RstAlgoOCR));
				break;
			case eAlgoPattern:
				msgRst.AppendFormat(_T("RstAlgoPattern,%d,"), sizeof(RstAlgoPattern));
				break;
			case eAlgoColor:
				msgRst.AppendFormat(_T("RstAlgoColor,%d,"), sizeof(RstAlgoColor));
				break;
			case eAlgoLead_Color:
				msgRst.AppendFormat(_T("RstAlgoLeadColor,%d,"), sizeof(RstAlgoColor));
				break;
			case eAlgoGray_Mean:
				msgRst.AppendFormat(_T("RstAlgoGrayMean,%d,"), sizeof(RstAlgoGrayMean));
				break;
			case eAlgoHeight_Mean:
				msgRst.AppendFormat(_T("RstAlgoHeightMean,%d,"), sizeof(RstAlgoHeightMean));
				break;
			case eAlgoGray_Diff:
				msgRst.AppendFormat(_T("RstAlgoGrayDiff,%d,"), sizeof(RstAlgoGrayDiff));
				break;
			case eAlgoHeight_Diff:
				msgRst.AppendFormat(_T("RstAlgoHeightDiff,%d,"), sizeof(RstAlgoHeightDiff));
				break;
			case eAlgoLead_Search:
				msgRst.AppendFormat(_T("RstAlgoLeadSearch,%d,"), sizeof(RstAlgoLeadSearch));
				break;
			case eAlgoTab_Search:
				msgRst.AppendFormat(_T("RstAlgoTabSearch,%d,"), sizeof(RstAlgoLeadSearch));
				break;
			case eAlgoBridge:
				msgRst.AppendFormat(_T("RstAlgoBridge,%d,"), sizeof(RstAlgoBridge));
				break;
			case eAlgoLead_Tip:
				msgRst.AppendFormat(_T("RstAlgoLeadTip,%d,"), sizeof(RstAlgoLeadTip));
				break;
			case eAlgoLead_Lift:
				msgRst.AppendFormat(_T("RstAlgoLeadLift,%d,"), sizeof(RstAlgoLeadLift));
				break;
			case eAlgoLead_Solder:
				msgRst.AppendFormat(_T("RstAlgoLeadSolder,%d,"), sizeof(RstAlgoLeadSolder));
				break;
			case eAlgoLead_SideSolder:
				msgRst.AppendFormat(_T("RstAlgoLeadSideSolder,%d,"), sizeof(RstAlgoLeadSideSolder));
				break;
			case eAlgoWidth:
				msgRst.AppendFormat(_T("RstAlgoWidth,%d,"), sizeof(RstAlgoLength));
				break;
			case eAlgoVolume:
				msgRst.AppendFormat(_T("RstAlgoVolum,%d,"), sizeof(RstAlgoVolume));
				break;
			case eAlgoTab:
				msgRst.AppendFormat(_T("RstAlgoTab,%d,"), sizeof(RstAlgoTab));
				break;
			case eAlgoGrid:
				msgRst.AppendFormat(_T("RstAlgoGrid,%d,"), sizeof(RstAlgoGrid));
				break;
			case eAlgoLine:
				msgRst.AppendFormat(_T("RstAlgoLine,%d,"), sizeof(RstAlgoLine));
				break;
			case eAlgoEdge:
				msgRst.AppendFormat(_T("RstAlgoEdge,%d,"), sizeof(RstAlgoEdge));
				break;
			case eAlgoSolderCone:
				msgRst.AppendFormat(_T("RstAlgoSolderCone,%d,"), sizeof(RstAlgoSolderCone));
				break;
			case eAlgoColorXY:
				msgRst.AppendFormat(_T("RstAlgoColorXY,%d,"), sizeof(RstAlgoColorXY));
				break;
			case eAlgoAlignEdge:
				msgRst.AppendFormat(_T("RstAlgoAlignEdge,%d,"), sizeof(RstAlgoAlignEdge));
				break;
			case eAlgoPadAlign:
				msgRst.AppendFormat(_T("RstAlgoPadAlign,%d,"), sizeof(RstAlgoPadAlign));
				break;
			case eAlgoPOCR:
				msgRst.AppendFormat(_T("RstAlgoPOCR,%d,"), sizeof(RstAlgoPOCR));
				break;
			case eAlgoWire:
				msgRst.AppendFormat(_T("RstAlgoWire,%d,"), sizeof(RstAlgoWire));
				break;
			case eAlgoFoot:
				msgRst.AppendFormat(_T("RstAlgoFoot,%d,"), sizeof(RstAlgoFoot));
				break;
			case eAlgoBarcode:
				msgRst.AppendFormat(_T("RstAlgoBarcode,%d,"), sizeof(RstAlgoBarcode));
				break;
			case eAlgoFillet:
				msgRst.AppendFormat(_T("RstAlgoFillet,%d,"), sizeof(RstAlgoFillet));
				break;
			case eAlgoBGA:
				msgRst.AppendFormat(_T("RstAlgoBGA,%d,"), sizeof(RstAlgoBGA));
				break;
			case eAlgoBump:
				msgRst.AppendFormat(_T("RstAlgoBump,%d,"), sizeof(RstAlgoBump));
				break;
			case eAlgoNGBlob:
				msgRst.AppendFormat(_T("RstAlgoNGBlob,%d,"), sizeof(RstAlgoNGBlob));
				break;
			case eAlgoPadBW:
				msgRst.AppendFormat(_T("RstAlgoPadBW,%d,"), sizeof(RstAlgoPadBW));
				break;
			case eAlgoBodyEdge:
				msgRst.AppendFormat(_T("RstAlgoBodyEdge,%d,"), sizeof(RstAlgoBodyEdge));
				break;
			}

			// 			nStAddCnt++;
			// 			if (nStAddCnt % 5 == 0)
			// 			{
			// 				ext::Log::add(msg);
			// 				msg.Format(_T(""));
			// 				nStAddCnt = 0;
			// 			}

		}
		//	if (nStAddCnt != 0)
		ext::Log::add(msgRst);

	}
	bool InspRoot::ExtModelAdd(CFile * fInput,CString sFilename,int nModelType)
	{
		bool bResult = true; 
		bool bModelLoadCheck = false;
		int nIndexTemp = 0;
		int nFileSize = 0;
		wchar_t sTemp[255];
		wcscpy_s(sTemp, sFilename.GetLength() + 1, sFilename.GetBuffer(sFilename.GetLength()));
		if (fInput == nullptr)
			nFileSize = 0;
		else
			nFileSize = fInput->GetLength();
		ModelCtrl * pModel;
		ModelImageBuffer *pModelBuffer;
		int nModelIndex = 0;

		switch (nModelType)
		{
		case ext::ModelBufferFlag::eExtModel_POCR: 
			pModel = &_POCRModelCtrl;
			pModelBuffer = &_POCRModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_POCR;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_POCR++;
			break;
		case ext::ModelBufferFlag::eExtModel_Pattern:
			pModel = &_PatternModelCtrl;
			pModelBuffer = &_PatternModelBuffer; 
			nModelIndex = _InspStatus.First()->nModelIndex_Pattern;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_Pattern++;
			break;
		case ext::ModelBufferFlag::eExtModel_COB:
			pModel = &_COBModelCtrl;
			pModelBuffer = &_COBModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_COB;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_COB++;
			break;
		case ext::ModelBufferFlag::eExtModel_Exc:
			pModel = &_ExcModelCtrl;
			pModelBuffer = &_ExcModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_Exc;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_Exc++;
			break;
		case ext::ModelBufferFlag::eExtModel_PadBW:
			pModel = &_PadBWModelCtrl;
			pModelBuffer = &_PadBWModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_PadBW;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_PadBW++;
			break;
		case ext::ModelBufferFlag::eExtModel_ShapeX:
			pModel = &_ShapeXModelCtrl;
			pModelBuffer = &_ShapeXModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_ShapeX;
			if (!FileAndModelNameCompare(sTemp, pModel, nModelIndex, nFileSize))
				return false;
			_InspStatus.First()->nModelIndex_ShapeX++;
			break;
		case ext::ModelBufferFlag::eExtModel_HalconPOCR:
			pModel = &_HalconPOCRModelCtrl;
			pModelBuffer = nullptr;
			nModelIndex = _InspStatus.First()->nModelIndex_HalconPOCR;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_HalconPOCR++;
			break;
		case ext::ModelBufferFlag::eExtModel_NGBlobExc:
			pModel = &_ExcNGBlobModelCtrl;
			pModelBuffer = &_ExcNGBlobModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_NGBlobExc;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_NGBlobExc++;
			break;
		default:
			ext::Log::add(_T("[EXT] Invalid Model Type ExtModelAdd Fail"));
			return false;
			break;
		}
		sFilename.ReleaseBuffer();
		try
		{
			if (pModelBuffer == nullptr)
			{
				pModel->First()[nModelIndex].img.imgSz = 0;
				pModel->First()[nModelIndex].img.imgStIdx = 0;
				CString sLogs;
				sLogs.Format(_T("[EXTADD]ModelType: [%d] %s / Cnt : %d"), nModelType,sFilename,nModelIndex	);
				ext::Log::add(sLogs);
				wcscpy(pModel->First()[nModelIndex].sFilePath, sTemp);
			}
			else
			{
				pModel->First()[nModelIndex].img = pModelBuffer->FilePush(fInput);
				CString sLogs;
				sLogs.Format(_T("[EXTADD]ModelType: [%d] Image :%lld / Img Index : %lld , %s / Cnt : %d"), nModelType,
					pModel->First()[nModelIndex].img.imgSz,
					pModel->First()[nModelIndex].img.imgStIdx,
					sFilename,
					nModelIndex
				);
				ext::Log::add(sLogs);
				wcscpy(pModel->First()[nModelIndex].sFilePath, sTemp);
				//_InspStatus.First()->nModelIndex_POCR++;
			}

			_InspStatus.UpdateAll();
		}
		catch (...)
		{
			bResult = false;
		}
		return bResult;
	}
	void InspRoot::ModelClear(int nModelType)
	{
		CString sLogModel;
		sLogModel.Format(_T("[EXT_Model] Model Clear Type : %d"), nModelType);
		ext::Log::add(sLogModel);
		switch (nModelType)
		{
		case ext::ModelBufferFlag::eExtModel_POCR: 
		{
			_POCRModelCtrl.Clear();
			_POCRModelBuffer.Clear();
			_InspStatus.First()->nModelIndex_POCR = 0;
		}
			break;
		case ext::ModelBufferFlag::eExtModel_Pattern: 
		{
			_PatternModelCtrl.Clear();
			_PatternModelBuffer.Clear();
			_InspStatus.First()->nModelIndex_Pattern = 0;
		}
			break;
		case ext::ModelBufferFlag::eExtModel_COB:
		{
			_COBModelCtrl.Clear();
			_COBModelBuffer.Clear();
			_InspStatus.First()->nModelIndex_COB = 0;
		}
		break;
		case ext::ModelBufferFlag::eExtModel_Exc:
		{
			_ExcModelCtrl.Clear();
			_ExcModelBuffer.Clear();
			_InspStatus.First()->nModelIndex_Exc = 0;
		}
		break;
		case ext::ModelBufferFlag::eExtModel_PadBW:
		{
			_PadBWModelCtrl.Clear();
			_PadBWModelBuffer.Clear();
			_InspStatus.First()->nModelIndex_PadBW = 0;
		}
		break;
		case ext::ModelBufferFlag::eExtModel_ShapeX:
		{
			_ShapeXModelCtrl.Clear();
			_ShapeXModelBuffer.Clear();
			_InspStatus.First()->nModelIndex_ShapeX = 0;
		}
		break;
		case ext::ModelBufferFlag::eExtModel_HalconPOCR:
		{
			_HalconPOCRModelCtrl.Clear();
			_InspStatus.First()->nModelIndex_HalconPOCR = 0;
		}
		break;
		case ext::ModelBufferFlag::eExtModel_NGBlobExc:
		{
			_ExcNGBlobModelCtrl.Clear();
			_ExcNGBlobModelBuffer.Clear();
			_InspStatus.First()->nModelIndex_NGBlobExc = 0;
		}
		break;
		case ext::ModelBufferFlag::eExtModel_Total: 
			_POCRModelCtrl.Clear();
			_POCRModelBuffer.Clear();
			_PatternModelCtrl.Clear();
			_PatternModelBuffer.Clear();
			_ExcModelCtrl.Clear();
			_ExcModelBuffer.Clear();
			_COBModelCtrl.Clear();
			_COBModelBuffer.Clear();
			_PadBWModelCtrl.Clear();
			_PadBWModelBuffer.Clear();
			_ShapeXModelCtrl.Clear();
			_ShapeXModelBuffer.Clear();
			_HalconPOCRModelCtrl.Clear();
			_ExcNGBlobModelCtrl.Clear();
			_ExcNGBlobModelBuffer.Clear();
			_InspStatus.Clear();
			break;

		default:
			break;
		}
		_PatternModelCtrl.UpdateAll();
		_POCRModelCtrl.UpdateAll();
		_COBModelCtrl.UpdateAll();
		_ExcModelCtrl.UpdateAll();
		_ShapeXModelCtrl.UpdateAll();
		_HalconPOCRModelCtrl.UpdateAll();
		_InspStatus.UpdateAll();
		//_InspStatus.Clear();
	}
	bool InspRoot::ExtMatImageAdd(cv::Mat * cvInput, CString sName, int nModelType)
	{
		bool bResult = true;
		bool bModelLoadCheck = false;
		int nIndexTemp = 0;
		wchar_t sTemp[255];
		wcscpy_s(sTemp, sName.GetLength() + 1, sName.GetBuffer(sName.GetLength()));
		ModelCtrl * pModel;
		ModelImageBuffer *pModelBuffer;
		int nModelIndex = 0;
		switch (nModelType)
		{
		case ext::ModelBufferFlag::eExtModel_POCR:
			pModel = &_POCRModelCtrl;
			pModelBuffer = &_POCRModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_POCR;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_POCR++;
			break;
		case ext::ModelBufferFlag::eExtModel_Pattern:
			pModel = &_PatternModelCtrl;
			pModelBuffer = &_PatternModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_Pattern;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_Pattern++;
			break;
		case ext::ModelBufferFlag::eExtModel_COB:
			pModel = &_COBModelCtrl;
			pModelBuffer = &_COBModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_COB;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_COB++;
			break;
		case ext::ModelBufferFlag::eExtModel_Exc:
			pModel = &_ExcModelCtrl;
			pModelBuffer = &_ExcModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_Exc;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_Exc++;
			break;
		case ext::ModelBufferFlag::eExtModel_PadBW:
			pModel = &_PadBWModelCtrl;
			pModelBuffer = &_PadBWModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_PadBW;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_PadBW++;
			break;
		case ext::ModelBufferFlag::eExtModel_ShapeX:
			pModel = &_ShapeXModelCtrl;
			pModelBuffer = &_ShapeXModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_ShapeX;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_ShapeX++;
			break;
		case ext::ModelBufferFlag::eExtModel_HalconPOCR:
			pModel = &_HalconPOCRModelCtrl;
			pModelBuffer = nullptr;
			nModelIndex = _InspStatus.First()->nModelIndex_HalconPOCR;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_HalconPOCR++;
			break;
		case ext::ModelBufferFlag::eExtModel_NGBlobExc:
			pModel = &_ExcNGBlobModelCtrl;
			pModelBuffer = &_ExcNGBlobModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_NGBlobExc;
			if (!stringCompare(sTemp, pModel, nModelIndex))
				return false;
			_InspStatus.First()->nModelIndex_NGBlobExc++;
			break;
		default:
			ext::Log::add(_T("[EXT] Invalid Model Type ExtModelAdd Fail"));
			return false;
			break;
		}
		sName.ReleaseBuffer();
		try
		{
			if (cvInput->channels() == 1)
				pModel->First()[nModelIndex].img = pModelBuffer->Push((BYTE*)cvInput->data, cvInput->step *cvInput->size().height, 0);
			else
				pModel->First()[nModelIndex].img = pModelBuffer->Push((BYTE*)cvInput->data, (cvInput->size().width *cvInput->size().height) * 3, 0);
			pModel->First()[nModelIndex].nWidth = cvInput->size().width;
			pModel->First()[nModelIndex].nHeight = cvInput->size().height;
			pModel->First()[nModelIndex].ch = cvInput->channels();
			CString sLogs;
			sLogs.Format(_T("[EXTADD]ModelType: %d Image :%lld / Img Index : %lld , %s / Cnt : %d"), nModelType,
				pModel->First()[nModelIndex].img.imgSz,
				pModel->First()[nModelIndex].img.imgStIdx,
				sName,
				nModelIndex
			);
			ext::Log::add(sLogs);
			wcscpy(pModel->First()[nModelIndex].sFilePath, sTemp);
			_InspStatus.UpdateAll();
		}
		catch (...)
		{
			bResult = false;
		}
		return bResult;
	}
	bool InspRoot::GetExtMatImage(cv::Mat * cvInput, CString sName, int nModelType)
	{
		bool bResult = true;
		bool bModelLoadCheck = false;
		int nIndexTemp = 0;
		wchar_t sTemp[255];
		wcscpy_s(sTemp, sName.GetLength() + 1, sName.GetBuffer(sName.GetLength()));
		ModelCtrl * pModel;
		ModelImageBuffer *pModelBuffer;
		int nModelIndex = 0;
		switch (nModelType)
		{
		case ext::ModelBufferFlag::eExtModel_POCR:
			pModel = &_POCRModelCtrl;
			pModelBuffer = &_POCRModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_POCR;
			break;
		case ext::ModelBufferFlag::eExtModel_Pattern:
			pModel = &_PatternModelCtrl;
			pModelBuffer = &_PatternModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_Pattern;
			break;
		case ext::ModelBufferFlag::eExtModel_COB:
			pModel = &_COBModelCtrl;
			pModelBuffer = &_COBModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_COB;
			break;
		case ext::ModelBufferFlag::eExtModel_Exc:
			pModel = &_ExcModelCtrl;
			pModelBuffer = &_ExcModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_Exc;
			break;
		case ext::ModelBufferFlag::eExtModel_PadBW:
			pModel = &_PadBWModelCtrl;
			pModelBuffer = &_PadBWModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_PadBW;
			break;
		case ext::ModelBufferFlag::eExtModel_ShapeX:
			pModel = &_ShapeXModelCtrl;
			pModelBuffer = &_ShapeXModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_ShapeX;
			break;
		case ext::ModelBufferFlag::eExtModel_HalconPOCR:
			pModel = &_HalconPOCRModelCtrl;
			pModelBuffer = nullptr;
			nModelIndex = _InspStatus.First()->nModelIndex_HalconPOCR;
			break;
		case ext::ModelBufferFlag::eExtModel_NGBlobExc:
			pModel = &_ExcNGBlobModelCtrl;
			pModelBuffer = &_ExcNGBlobModelBuffer;
			nModelIndex = _InspStatus.First()->nModelIndex_NGBlobExc;
			break;
		default:
			ext::Log::add(_T("[EXT] Invalid Model Type Get ExtModelFail"));
			return false;
			break;
		}
		try
		{
			int nOutModel;
			if (!stringCompare(sTemp, pModel, nModelIndex, &nOutModel)) //if Model Same ?
			{
				cv::Size sz = cv::Size(pModel->First()[nOutModel].nWidth, pModel->First()[nOutModel].nHeight);
				*cvInput = cv::Mat::zeros(sz, CV_16SC1);
				memcpy(cvInput->data,
					pModelBuffer->Lock(pModel->First()[nOutModel].img),
					pModel->First()[nOutModel].img.imgSz);
			}
			sName.ReleaseBuffer();
		}
		catch (...)
		{
			bResult = false;
		}
		return bResult;
	}
	bool InspRoot::stringCompare(wchar_t * strSrc, ModelCtrl * Models, int nIndex, int * nOutModelIndex)
	{
		if (nIndex == 0)// 첫 스타트시 검사하지않음
			return true; 
		int nResult = -1; 

		for (int i = 0; i < nIndex; i++)
		{
			if (wcscmp(strSrc, Models->First()[i].sFilePath) == 0)// Model 가 같을때
			{
				//CString s(strSrc);
				//CString s2(Models->First()[i].sFilePath);
				//CString sLogs; 
				//sLogs.Format(_T("%s == %s "),s,s2);
				//ext::Log::add(sLogs);
				if (nOutModelIndex != NULL)
					* nOutModelIndex = i;
				nResult = 0; 
				break; 
			}
		}
		//CString sLog; 
		//sLog.Format(_T("Compare Reuslt = %d"), nResult);
		//ext::Log::add(sLog);
		return (nResult != 0) ? true : false;
	}
	bool InspRoot::FileAndModelNameCompare(wchar_t * strSrc, ModelCtrl * Models, int nIndex,int fileSize)
	{
		if (nIndex == 0)// 첫 스타트시 검사하지않음
			return true;
		int nResult = -1;

		for (int i = 0; i < nIndex; i++) // 공유메모리에 있는 모든 Path 비교 
		{
			if (wcscmp(strSrc, Models->First()[i].sFilePath) == 0)// Modelname이 같을때
			{
				if(Models->First()[i].img.imgSz == fileSize) // File 사이즈도 같을때
				nResult = 0; // 공유메모리에 올리지않는다.
				break;
			}
		}
		//CString sLog; 
		//sLog.Format(_T("Compare Reuslt = %d"), nResult);
		//ext::Log::add(sLog);
		return (nResult != 0) ? true : false;
	}
	void InspRoot_Server::Clear()
	{
		_Ctrl.First()->nPartFullCnt = 0;
// 		memset(_InspStatus.First(), 0, sizeof(InspStatus) * ext::env::nPartBufCount);
// 		_InspStatus.UpdateAll();
		//memset(_Job.parts.First(), 0, sizeof(InspPartInfo) *ext::env::nPartBufCount);
		//_Job.parts.UpdateAll();
		//_InspStatus.Clear();
		_PartIndex.clear();
		m_nCurrentIdx = 0;
		//_Job.parts.Clear();
		//_ImgBuffer.Clear();

		for (int i = 0; i < _Ctrl.First()->NumOfClients; i++)
		{
			switch (m_vToolIndexSort[i])
			{
				case  (int)eToolType::eNORMAL: 
					_Tools[i].BufferClear();
					break;
				case (int)eToolType::eBIG: 
					_Tools[i].BigBufferClear();
					break;
				case (int)eToolType::eForeignType: 
					_Tools[i].BigBufferClear(); 
					break;
			}
		
			//if(i == _Ctrl.First()->NumOfClients - 1)
			//	_Tools[i].BigBufferClear();
			//else
			//	_Tools[i].BufferClear();

		}
		_Ctrl.First()->nCurrToolIdx = 0;
		_Ctrl.First()->nCurrBigToolIdx = 0;
		_Ctrl.First()->nInspDoneCnt = 0;
		_Ctrl.First()->nInspSkipCnt = 0;
		_Ctrl.First()->nInspForeignSkipCnt = 0; 
		_Ctrl.First()->nInspForeignDoneCnt = 0;
		_Ctrl.Clear();
	}



	ClientCtrl::ClientCtrl()
		:_nTool_id(0)
	{

	}

	bool ClientCtrl::Init(int nTool_id, int nMachineType)
	{
		bool bRet = true;
		_nTool_id = nTool_id;
		_MachineType = nMachineType;
		
		int nForeignIndex = (int)eToolType::eNORMAL;
#ifdef EXT_SERVER
		int totalToolCnt = env::nToolCount;
#else
		int totalToolCnt = ext::irc::get()->_CMachineData.First()->nTotalToolCnt;
#endif
		m_nToolType = 0; 
		std::vector<int> nToolIndexSort;  // index 참조에 죽는경우 Eagle3D 가 켜져있는지 확인필요 

		for (int i = 0; i < totalToolCnt; i++)
			nToolIndexSort.push_back((int)eToolType::eNORMAL); // Normal 로 모든 Tool 

		if (env::nForeignTypeToolCnt != 0)
		{
			int nForeign = totalToolCnt - env::nForeignTypeToolCnt;
			for (int i = nToolIndexSort.size()-1; i >= nForeign; i--)
			{
				nToolIndexSort[i] = (int)eToolType::eForeignType; // ToolCnt 에서 마지막 부터  
				if(nTool_id - 1 <= i )
					nForeignIndex++;
			}
		}
			
		if (env::nBigTypeToolCnt != 0)
		{
			if (env::nForeignTypeToolCnt != 0)
			{
				int nForeign = totalToolCnt - env::nForeignTypeToolCnt-1;
				int nBig = nForeign - env::nBigTypeToolCnt; 
				for (int i = nForeign; i > nBig; i--)
				{
					nToolIndexSort[i] = (int)eToolType::eBIG; // Fore 과 Normal 사이 Big 
				}
			}
			else
			{
				int nBig = totalToolCnt - env::nBigTypeToolCnt;
				for (int i = nToolIndexSort.size()-1; i >= nBig; i--) //Fore 사용 안할시엔 Big 만 
				{
					nToolIndexSort[i] = (int)eToolType::eBIG; // ToolCnt 에서 마지막 부터  
					nForeignIndex++;
				}
			}
			
		}
		const int numGroups = env::nForeignTypeToolCnt;
		if (numGroups != 0)
		{
			std::vector<std::vector<int>> groups(numGroups);
			// 1~40 순회하며 그룹에 분배
			for (int i = 0; i < EXT_BUFFER_FOREIGN_CNT; ++i)
			{
				int groupIndex = (i) % numGroups;  // 0 ~ 3
				groups[groupIndex].push_back(i);
			}
			InspCalcFov = groups[nForeignIndex - 1];
			m_nToolType = nToolIndexSort[nTool_id - 1]; // Sort된 Tool 중 아이디 
		}
		else
			m_nToolType = nToolIndexSort[nTool_id - 1]; // Sort된 Tool 중 아이디 
		

		//if (totalToolCnt == nTool_id)
		//	m_nToolType = (int)eToolType::eBIG;
		//else
		//	m_nToolType = (int)eToolType::eNORMAL;

		Execute();
		CString sExcute;
		sExcute.Format(_T("[EXT]Excute_%d, ToolType_%d"), _nTool_id, m_nToolType);
		ext::Log::add(sExcute);
		if (nTool_id == 1) // 1번 알고툴에 데이터만 사용
		{
			_AlgoCompare.Init();
			 _TotalBuff = _AlgoCompare._TotalBuff;
#ifdef EXT_SERVER
			_AlgoCompare.CompareInit(true); // Server 일경우
#else
			_AlgoCompare.CompareInit();
#endif
		}

		if (_ctrl.Init(nTool_id) == false)
			bRet = false;
		_TotalBuff += _ctrl._TotalBuff;
		m_nCurrBufferIdx = 0;

		switch (m_nToolType)
		{
		case eToolType::eBIG:
			bRet = BigTypeInit(nTool_id, nMachineType);
			break;
		case eToolType::eNORMAL:
			bRet = NormalTypeInit(nTool_id, nMachineType);
			break;
		case eToolType::eForeignType:
			bRet = ForeignTypeInit(nTool_id, nMachineType);
			break;
		}

		for (int i = 0; i < EXT_BUFFER_FOREIGN_CNT; i++)
		{
			m_MultiTempZmapData[i].data = 0;
			m_MultiTempZmapData[i].zmapSizeX = 0;
			m_MultiTempZmapData[i].zmapSizeY = 0;
			for (int a = 0; a < eInspRoiImg_NUM; a++)
				m_MultiRoiImgBuf[i].GetPtrImg(a) = 0; // buffer 초기화
			m_MultiRoiImgBuf[i].nImageSizeY = 0;
			m_MultiRoiImgBuf[i].nImageSizeX = 0;
		}
		CString slog;
		int nMegaByte = 1048576;
		unsigned long long Size = _TotalBuff / nMegaByte;
		slog.Format(_T("[TotalBuff][Init] ClientCtrl :%llu (MB)"), _TotalBuff);
		ext::Log::add(slog);
		return bRet;
	}
	bool ClientCtrl::NormalTypeInit(int nTool_id, int nMachineType)
	{
		bool bRet = true;
		unsigned long long nSize = 0;
		for (int i = 0; i < EXT_BUFFER_CNT; i++)
		{
			_PartData[i].m_nBuffIdx = i;

			_light[i].m_nBuffIdx = i;
			_SndCtrl[i].m_nBuffIdx = i;
			_RevCtrl[i].m_nBuffIdx = i;

			if (_SndCtrl[i].Init(nTool_id, (int)ext::CtrlMode::eSend) == false)
				bRet = false;

			if (_MasterBuffer[i].Init(nTool_id, i, m_nToolType, nMachineType) == false)
				bRet = false;
			if (_PartData[i].Init(nTool_id) == false)
				bRet = false;
			nSize += (unsigned long long)_SndCtrl[i].FirstSize();
			nSize += (unsigned long long)_RevCtrl[i].FirstSize();

			nSize += (unsigned long long)_PartData[i].FirstSize();
		

		}

		for (int i = 0; i < EXT_BUFFER_RST_CNT; i++)
		{
			_RevCtrl[i].m_nBuffIdx = i;
			if (_RevCtrl[i].Init(nTool_id, (int)ext::CtrlMode::eReceive) == false)
				bRet = false;
			_Rst[i].m_nBuffIdx = i;
			if (_Rst[i].Init(nTool_id) == false)
				bRet = false;
			nSize += (unsigned long long)_Rst[i].FirstSize();
		}
		_TotalBuff += nSize;
		return bRet;
	}
	bool ClientCtrl::BigTypeInit(int nTool_id, int nMachineType)
	{
		bool bRet = true;
		unsigned long long nSize = 0 ;
		for (int i = 0; i < EXT_BIG_BUFFER_CNT; i++)
		{
			_PartData[i].m_nBuffIdx = i;
			_light[i].m_nBuffIdx = i;
			_SndCtrl[i].m_nBuffIdx = i;
			_RevCtrl[i].m_nBuffIdx = i;
			_Rst[i].m_nBuffIdx = i;

			if (_SndCtrl[i].Init(nTool_id, (int)ext::CtrlMode::eSend) == false)
				bRet &= false;
			
			if (_MasterBuffer[i].Init(nTool_id, i, m_nToolType, nMachineType) == false)
				bRet &= false;
			if (_PartData[i].Init(nTool_id) == false)
				bRet &= false;
			nSize += (unsigned long long)_SndCtrl[i].FirstSize();
			nSize += (unsigned long long)_PartData[i].FirstSize();
		}
		for (int i = 0; i < EXT_BIG_BUFFER_RST_CNT; i++)
		{
			_RevCtrl[i].m_nBuffIdx = i;
			_Rst[i].m_nBuffIdx = i;
			if (_RevCtrl[i].Init(nTool_id, (int)ext::CtrlMode::eReceive) == false)
				bRet &= false;
	
			if (_Rst[i].Init(nTool_id) == false)
				bRet &= false;
	
			nSize += (unsigned long long)_RevCtrl[i].FirstSize();
			nSize += (unsigned long long)_Rst[i].FirstSize();
		}
		_TotalBuff += nSize;
		return bRet;
	}
	bool ClientCtrl::ForeignTypeInit(int nTool_id, int nMachineType)
	{
		bool bRet = true;
		unsigned long long nSize = 0 ; 
		for (int i = 0; i < EXT_BUFFER_FOREIGN_CNT; i++)
		{
			_SndForeignCtrl[i].m_nBuffIdx = i;
			if (_SndForeignCtrl[i].Init(nTool_id, (int)ext::CtrlMode::eForeignSend) == false)
				bRet &= false;
			nSize += (unsigned long long)_SndForeignCtrl[i].FirstSize();
			// Insp Buffer 
			_ForeignShared[i].m_nBuffIdx = i;
			if (_ForeignShared[i].Init(nTool_id) == false)
				bRet &= false;
			nSize += _ForeignShared[i]._sizebytes;
		}
		if (_ForeignWP.Init(nTool_id) == false) // Board 갯수 ( 1개이기때문)
			bRet &= false;
		nSize += _ForeignWP._sizebytes;
		for (int i = 0; i < EXT_BUFFER_FOREIGN_RST_CNT; i++)
		{
			_RevForeignCtrl[i].m_nBuffIdx = i;
			if (_RevForeignCtrl[i].Init(nTool_id, (int)ext::CtrlMode::eForeignReceive) == false)
				bRet &= false;
			nSize += (unsigned long long)_RevForeignCtrl[i].FirstSize();
			// result Buffer 
			_ForeignSheradRst[i].m_nBuffIdx = i;
			if (_ForeignSheradRst[i].Init(nTool_id) == false)
				bRet &= false;
			nSize += (unsigned long long)_RevForeignCtrl[i].FirstSize();
		}
		_TotalBuff += nSize;
		return bRet;
	}
	void ClientCtrl::Exit()
	{

#ifdef EXT_SERVER
		Exit_Part(eToolType::eNORMAL);
		Exit_Part(eToolType::eBIG);
		Exit_Foreign();
#else

#endif
		switch (m_nToolType)
		{
		case eToolType::eBIG:
		case eToolType::eNORMAL:
			Exit_Part(m_nToolType);
			break;
		case eToolType::eForeignType:
			Exit_Foreign();
			break;
		}
	}
	void ClientCtrl::Exit_Part(int nToolType)
	{
		switch (nToolType)
		{
		case eToolType::eNORMAL:
			for (int i = 0; i < EXT_BUFFER_CNT; i++)
			{
				_MasterBuffer[i].Exit();
				_light[i].Exit();
				_SndCtrl[i].Exit();
				_PartData[i].Exit();
			}
			for (int i = 0; i < EXT_BUFFER_RST_CNT; i++)
			{
				_RevCtrl[i].Exit();
				_Rst[i].Exit();
			}
			break;
		case eToolType::eBIG: 
			for (int i = 0; i < EXT_BIG_BUFFER_CNT; i++)
			{
				_MasterBuffer[i].Exit();
				_light[i].Exit();
				_SndCtrl[i].Exit();
				_PartData[i].Exit();
			}
			for (int i = 0; i < EXT_BIG_BUFFER_RST_CNT; i++)
			{
				_RevCtrl[i].Exit();
				_Rst[i].Exit();
			}
			break;		
		}

	}
	void ClientCtrl::Exit_Foreign()
	{
		for (int i = 0; i < EXT_BUFFER_FOREIGN_CNT; i++)
		{
			_ForeignShared[i].Exit();
			_SndForeignCtrl[i].Exit();
		}
		for (int i = 0; i < EXT_BUFFER_FOREIGN_RST_CNT; i++)
		{
			_ForeignSheradRst[i].Exit();
			_RevForeignCtrl[i].Exit();
		}
		_ForeignWP.Exit();
	}
	void ClientCtrl::Execute()
	{
#ifdef EXT_SERVER
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
// 
// 		if (bState)
// 		{
// 			Sleep(10); // AlgoTool Init 하는 시간을 기다림. true 여도 
// 			_AlgoCompare.
// 		}
#else

#endif
	}
	bool ClientCtrl::AddSharedData(InspPartInfo* pPart, InspPartParam* pWnd, int nSelectedIdx, int nBufferIndex)
	{
		if (nSelectedIdx < 0 || nSelectedIdx > EXT_BUFFER_CNT || _SndCtrl[nSelectedIdx].GetFlag(ext::BufferFlag::IF_CALC))
		{
			CString sLog; 
			sLog.Format(_T("AddSharedData : Select Idx : %d, Calc: %d "), nSelectedIdx, _SndCtrl[nSelectedIdx].GetFlag(ext::BufferFlag::IF_CALC));
			ext::Log::add(sLog);
			return false;
		}
		if (ext::irs::get()->_Ctrl.First()->prod.bSharedInOutImageSave) // Input Image
		{
			CString str;
			str.Format(_T("C:\\Eagle3D_64x\\PROGRAM\\AlgoTool\\Test\\%d_A_Eagle3D.bmp"), pPart->nPartIDOrg);
			cv::Mat CR(pPart->partImgBuf.nImageSizeY, pPart->partImgBuf.nImageSizeX, CV_8UC1, pPart->partImgBuf.GetPtrImg(0) /*_MasterBuffer[nBufferIndex]._ChannelBuffer[0]._stream->Byte()*/);
			CT2CA strWndAlgoName(str);
			ext::Log::add(str);
			std::string strAlgoName(strWndAlgoName);
			imwrite(strAlgoName, CR);
		}		
		_MasterBuffer[nSelectedIdx].AddImages(&pPart->partImgBuf, &pPart->partImgColorBuf, &pPart->partZmapData, pPart->Save3DRawData.pBuffer, pPart->Save3DRawData.szSize, nBufferIndex);
		_PartData[nSelectedIdx].AddInspData(pPart, pWnd, pPart->nWindowCount);
		//_light[nSelectedIdx].AddLight(pWnd, pPart->nWindowCount);
		_PartData[nSelectedIdx]._Light.AddLight(pWnd, pPart->nWindowCount);
		//CString strLog1;
		//strLog1.Format(_T("[ext Image] SizeX :%d , SizeY :%d, Part ID :%d Set End - Index :%d ,Tool Num :%d "), pPart->partImgBuf.nImageSizeX, pPart->partImgBuf.nImageSizeY, pPart->nPartIDOrg, nSelectedIdx, _nTool_id);
		//ext::Log::add(strLog1);
		//InspPartInfo * pDstPart = new InspPartInfo();
		//InspPartParam * pDstWindow = new InspPartParam();

		//ext::irs::get()->_Tools[0].GetSharedData(pDstPart, pDstWindow, 0);

		//_PartData[nSelectedIdx]._Map.AddStringPath(pWnd, pPart->nWindowCount);
		
		//스케줄링 데이터 Set
		//AddExcpectTime(nSelectedIdx, 0/*pPart->m_nLane*/, pPart->nPartIDOrg);
		//_SndCtrl[nSelectedIdx].SetFlag(ext::BufferFlag::IF_ON, true);
		//delete pDstPart;
		//delete [] pDstWindow;
		return true;
	}
	bool ClientCtrl::GetSharedData(InspPartInfo* pPart, InspPartParam*& pWnd, int nSelectedIdx, int nBufferIndex)
	{
		if (nSelectedIdx < 0 || nSelectedIdx > EXT_BUFFER_CNT)
			return false;
		
		ClearExtModelPath();

		_PartData[nSelectedIdx].GetPartData(pPart, pWnd);
		_MasterBuffer[nSelectedIdx].GetImages(&pPart->partImgBuf, &pPart->partImgColorBuf, &pPart->partZmapData, pPart->Save3DRawData.pBuffer, pPart->Save3DRawData.szSize, nBufferIndex);
		_InputBufferIndex = nSelectedIdx;
		_nW = pPart->partImgBuf.nImageSizeX;
		_nH = pPart->partImgBuf.nImageSizeY;
	//	_PartData[nSelectedIdx]._Map.GetStringPath(pWnd, pPart->nWindowCount);

	}
	void ClientCtrl::SelectBuffer(wchar_t * sPath )
	{ 

		CString wideString(sPath);
		
		std::string str = std::string(CT2CA(wideString));
		//WIR은 W 데이터만 넣어주므로 
	cv::Mat img(_nH, _nW, CV_8UC1, _MasterBuffer[_InputBufferIndex]._ChannelBuffer[0]._stream->Byte());
	cv::imwrite(str, img);
		
	}
	bool ClientCtrl::AddShared_ForeignData(InspPartInfo* _ForePartInfo,ForeignData _foreinData, FR2DData Fr2Data, InspForeignInfo foreignInfo, ForeignParamROI* _ForeParamROI, ForeignParamROI* arrROI,int nSelectedIdx)
	{
		if (nSelectedIdx < 0 || nSelectedIdx > EXT_BUFFER_FOREIGN_CNT || _SndForeignCtrl[nSelectedIdx].GetFlag(ext::BufferFlag::IF_CALC))
		{
			CString sLog;
			sLog.Format(_T("AddSharedData : Select Idx : %d, Calc: %d "), nSelectedIdx, _SndCtrl[nSelectedIdx].GetFlag(ext::BufferFlag::IF_CALC));
			ext::Log::add(sLog);
			return false;
		}
			_ForeignShared[nSelectedIdx].AddInspForeign(_ForePartInfo,_foreinData, Fr2Data, foreignInfo, _ForeParamROI, arrROI);
			if(nSelectedIdx == 0 )
				_ForeignWP.AddInspWarpageImage(_foreinData);
		return true;
	}
	bool ClientCtrl::GetShared_ForeignData(InspPartInfo* _ForePartInfo, ForeignData* _foreinData, FR2DData* Fr2Data, InspForeignInfo* m_ForeignInfoData, ForeignParamROI ** _ForeParamROI , ForeignParamROI** arrROI,int nSelectedIdx)
	{
		//m_MultiTempZmapData[nSelectedIdx].DeleteData();
		//m_MultiRoiImgBuf[nSelectedIdx].DeleteData();s
		
		_ForeignShared[nSelectedIdx].GetnspForeign(_ForePartInfo, _foreinData, Fr2Data, m_ForeignInfoData, _ForeParamROI, arrROI);
		_ForeignWP.GetInspWarpageImage(_foreinData); // Board 당 한개 이므로
		if (_ForePartInfo ==NULL)
		{
			InspPartInfo* pBoardTemp = new InspPartInfo();
			_ForeignShared[nSelectedIdx].GetForeignPartInfoData(pBoardTemp, false);
			m_MultiTempZmapData[nSelectedIdx].CopyData(pBoardTemp->zmapForeignData, true);
			m_MultiRoiImgBuf[nSelectedIdx].CopyData(pBoardTemp->partImgBuf, true);
			delete pBoardTemp;
		}
		
		ext::irs::get()->_nFov = InspFovGetIndex = nSelectedIdx;
		return true;
	}
	void ClientCtrl::AddResultData(InspectionResult * PartData, int nSelectedIdx)
	{
		//int nIndex = 0;
		//_Rst[nIndex].AddResult(PartData);
	}
	void ClientCtrl::GetResultData(InspectionResult * PartData, int nSelectedIdx)
	{
		int nIndex = 0;
		_Rst[nIndex].GetResult( PartData);
	}
	void ClientCtrl::InsertExtModelPath(QWORD qwName, CString sPath)
	{
		m_stExtModelPath.insert(std::pair<QWORD, CString>(qwName, sPath));
	}
	CString ClientCtrl::FindExtModelPath(QWORD qwName)
	{
		CString ret = _T("");

		auto iter = m_stExtModelPath.find(qwName);
		if (iter != m_stExtModelPath.end())
		{
			ret = iter->second;
		}

		return ret;
	}
	void ClientCtrl::BufferClear()
	{
		m_nCurrBufferIdx = 0;

		if (m_nToolType == (int)ext::eToolType::eBIG)
		{
			BigBufferClear();
			return;
		}
		if (m_nToolType == (int)ext::eToolType::eForeignType)
		{
			for (int i = 0; i < EXT_BUFFER_FOREIGN_CNT; i++)
			{//EXT_BUFFER_FOREIGN_RST_CNT
				_SndForeignCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
				_RevForeignCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
				_ForeignSheradRst[i]._nRecvRstCount = 0;
				m_MultiTempZmapData[i].DeleteData();
				m_MultiRoiImgBuf[i].DeleteData();
			}
			return;
		}
		int nPrevCalcDoneCnt = 0;

		m_dExcpectInspTime = 0.0;

		for (int i = 0; i < EXT_BUFFER_CNT; i++)
		{
			//임시 확인용
			nPrevCalcDoneCnt += _RevCtrl[i].First()->nCalcDoneCount;

			_SndCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
			//_RevCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
			_SndCtrl[i].First()->nCalcDoneCount = 0;
			//_RevCtrl[i].First()->nCalcDoneCount = 0;
			_MasterBuffer[i].Clear();
			_PartData[i].Clear();
			_light[i].Clear();
		}
		for (int i = 0; i < EXT_BUFFER_RST_CNT; i++)
		{
			_RevCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
			_RevCtrl[i].First()->nCalcDoneCount = 0;
			_Rst[i].Clear();
			_Rst[i]._nRecvRstCount = 0;
		}

		//CString msg;
		//msg.Format(_T("[MultiProcess]Tool[%d] PrevCalcDoneCount, %d"), _nTool_id, nPrevCalcDoneCnt);
		//ext::Log::add(msg);
	}
	void ClientCtrl::BigBufferClear()
	{
		m_nCurrBufferIdx = 0;
		if (m_nToolType == (int)ext::eToolType::eNORMAL)
		{
			BufferClear();
			return;
		}
		if (m_nToolType == (int)ext::eToolType::eForeignType)
		{
			for (int i = 0; i < EXT_BUFFER_FOREIGN_CNT; i++)
			{//EXT_BUFFER_FOREIGN_RST_CNT
				_SndForeignCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
				_RevForeignCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
				_ForeignSheradRst[i]._nRecvRstCount = 0;
				m_MultiTempZmapData[i].DeleteData();
				m_MultiRoiImgBuf[i].DeleteData();
			}
			return;
		}

		int nPrevCalcDoneCnt = 0;
		m_dExcpectInspTime = 0.0;
		for (int i = 0; i < EXT_BIG_BUFFER_CNT; i++)
		{
			//임시 확인용
			nPrevCalcDoneCnt += _RevCtrl[i].First()->nCalcDoneCount;

			_SndCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
			//_RevCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
			_SndCtrl[i].First()->nCalcDoneCount = 0;
			//_RevCtrl[i].First()->nCalcDoneCount = 0;
			_MasterBuffer[i].Clear();
			_PartData[i].Clear();
			_light[i].Clear();
		}
		for (int i = 0; i < EXT_BIG_BUFFER_RST_CNT; i++)
		{
			_RevCtrl[i].SetFlag(BufferFlag::IF_OFF, true);
			_RevCtrl[i].First()->nCalcDoneCount = 0;
			_Rst[i].Clear();
			_Rst[i]._nRecvRstCount = 0;
		}


		//CString msg;
		//msg.Format(_T("[MultiProcess]Tool[%d] PrevCalcDoneCount, %d"), _nTool_id, nPrevCalcDoneCnt);
		//ext::Log::add(msg);
	}
	void ClientCtrl::AddExcpectTime(int nBufferIdx, int nLane, int nPartID)
	{
		//이전 평균 검사시간 취득
		double dExcpectTime = ext::Sch::get()->GetExpectTime(nLane, nPartID);
		
		//Tool buffer에 반영
		m_dExcpectInspTime += dExcpectTime;
	}

	void  ClientCtrl::GetImageDump(InspPartInfo* pPart, InspPartParam* pWnd, SaveImgParamData * tg, int nBufferIndex)
	{
		//2D 만 작업되어있음.
		int ret = 0;
	
		CString str2dFullPath = pPart->s2DImagePath;//_T("D:\\Eagle3D_data\\InspectResult\\Image\\mobis_Wire\\wire_1115\\20230308124734\\1@2");
		CString str3dFullPath = pPart->s3DImagePath;// _T("D:\\Eagle3D_data\\InspectResult\\Image\\mobis_Wire\\test\\wire_1115\\20230308124734\\1@3");//

		int nRIndex = str2dFullPath.ReverseFind('\\');
		int pos = 0;
		CString delimiter = _T("\\");
		CString token;
		std::vector< CString> strPath;
		while ((token = str2dFullPath.Tokenize(delimiter, pos)) != "") // Module 안넘겨줘
			strPath.emplace_back(token);
		if (strPath.size() == 0) return ;  // 2D 가 빈값으로 들어오면 문제가 있으니 여기서 걸러줌.
		CString sModuleNo = strPath[strPath.size() - 1].Mid(0, 1); //Module 
		CString sInspctStart = strPath[strPath.size() - 2];
		CString sRootPath = _T("C:\\Eagle3D_64x\\PROGRAM\\AlgoTool\\DumpMessage");
		sRootPath.AppendFormat(_T("\\%d_%d"),this->_nTool_id, nBufferIndex);

		CString s2DName = str2dFullPath.Mid(str2dFullPath.ReverseFind('\\') + 1);
		CString s2DPath = str2dFullPath.Left(str2dFullPath.ReverseFind('\\'));

		CString s3DName = str3dFullPath.Mid(str3dFullPath.ReverseFind('\\') + 1);
		CString s3DPath = str3dFullPath.Left(str3dFullPath.ReverseFind('\\'));


		s2DPath = s3DPath = sRootPath;
		int nSaveImage2D = 1;
		int nSaveImage3D = 1;
		bool b2DImageSave = true;
		SaveImgParamData tgparam;
		CString strRawTemp;
		strRawTemp.Format(_T("D:\\Temp\\Eagle3d\\rawdata3d\\%s@"), sInspctStart, sModuleNo);
		tgparam.unNGType = 100; // m_inspectionResult->nDefectType
		tgparam.nSave_2d = nSaveImage2D;// m_ExtProdInfo->nImgSave2DCheck;//m_ExtProdInfo->nImgSave2DCheck
		tgparam.nSave_3d = nSaveImage3D;
		tgparam.nMSPtImg = 0;
		tgparam.nSave_put = 0;
		tgparam.nSave_AOILink = 0;
		tgparam.nSave_NextMC = 0;
		tgparam.m_nSave_MES = 0;
		tgparam.m_nSave_APC = 0;
		tgparam.m_nSave_Color = pPart->s2DImagePath != _T("");
		// tgparam.m_nSave_Color = 1; // AngleColor 는 현재 사용 x 
		//tgparam.m_nSave_3DRaw = m_ExtProdInfo->m_nSave_3DRaw;
		tgparam.m_nSave_ImgType = -1;
		//tgparam.m_nSave_ImgType = -1; // uv 
		//tgparam.unSaveOption = m_ExtProdInfo->unSaveOption;
		tgparam.m_n2DSaveQuality = 75;
		wstring temp = L"";
		wcscpy(tgparam.arrNGCaption, temp.c_str());
		temp = s2DName.operator LPCWSTR();//L"1@2";
		wcscpy(tgparam.arrJpgFileName, temp.c_str());

		CString s3DRawName;
		s3DRawName.Format(_T("%s@%s"), sModuleNo, pPart->s3DRawName);
		wcscpy(tgparam.arr3dRawFileName, s3DRawName);

		//tgparam.m_nMachineCode = m_ExtProdInfo->m_nMachineCode;
	
	
		temp = s3DName.operator LPCWSTR();
		wcscpy(tgparam.arr3dFileName, temp.c_str());

		temp = (tgparam.unNGType == 0) ? s2DPath.operator LPCWSTR() : s3DPath.operator LPCWSTR();
		wcscpy(tgparam.arrJpgFilePath, temp.c_str());
		temp = (tgparam.unNGType == 0) ? s2DPath.operator LPCWSTR() : s3DPath.operator LPCWSTR();
		wcscpy(tgparam.arr3dFilePath, temp.c_str());

		wcscpy(tgparam.arr3dRawFilePath, temp.c_str());
		temp = strRawTemp.operator LPCWSTR();
		wcscpy(tgparam.arr3dRawFilePath, temp.c_str());

		memcpy(tg, &tgparam, sizeof(SaveImgParamData));
		
	}

	void ClientCtrl::CreateDir(CString Path)
	{
		if (IsExistDir(Path))
			return;

		CString strPath = Path;
		CString strTemp[20];
		CString strFile;
		int nLength = strPath.GetLength();
		int j = 0;

		for (int i = 0; i < 20; i++)
			strTemp[i].Empty();

		for (int i = 0; i < nLength; i++)
		{
			if (strPath.Mid(i, 1) == '\\')
			{
				j++;
			}
			strTemp[j] = strTemp[j] + strPath.Mid(i, 1);
		}

		strFile = strTemp[0];
		int i = 1;

		while (strTemp[i] != "")
		{
			strFile = strFile + strTemp[i];
			CreateDirectory(strFile, NULL);
			i++;
		}
	}
	BOOL ClientCtrl::IsExistDir(CString path)
	{
		BOOL ret = FALSE;
		CFileFind fn;

		BOOL bWorking = fn.FindFile(path);
		if (bWorking)
		{
			bWorking = fn.FindNextFileW();
			if (fn.IsDirectory())
				ret = TRUE;
		}
		return ret;
	}
	InspRoot_Server::InspRoot_Server()
	{
		_nFov = 0;
	}
	InspRoot_Server::~InspRoot_Server()
	{

	}

	bool InspRoot_Server::Init(int nMachineType)
	{
		bool bInit(true);
		int nF = env::nForeignTypeToolCnt;
		m_vToolForeign.clear();
		int totalToolCnt = env::nToolCount;
		std::vector<int> nToolIndexSort;  // 차후 변경 

		for (int i = 0; i < totalToolCnt; i++)
			nToolIndexSort.push_back((int)eToolType::eNORMAL); // Normal 로 모든 Tool 

		if (env::nForeignTypeToolCnt != 0)
		{
			int nForeign = totalToolCnt - env::nForeignTypeToolCnt;
			for (int i = nToolIndexSort.size() - 1; i >= nForeign; i--)
			{
				nToolIndexSort[i] = (int)eToolType::eForeignType; // ToolCnt 에서 마지막 부터  
				m_vToolForeign.push_back(i);
			}
		}

		if (env::nBigTypeToolCnt != 0)
		{
			if (env::nForeignTypeToolCnt != 0)
			{
				int nForeign = totalToolCnt - env::nForeignTypeToolCnt - 1;
				int nBig = nForeign - env::nBigTypeToolCnt;
				for (int i = nForeign; i > nBig; i--)
				{
					nToolIndexSort[i] = (int)eToolType::eBIG; // Fore 과 Normal 사이 Big 
				}

			}
			else
			{
				
				int nBig = totalToolCnt - env::nBigTypeToolCnt;
				for (int i = nToolIndexSort.size() - 1; i >= nBig; i--) //Fore 사용 안할시엔 Big 만 
				{
					nToolIndexSort[i] = (int)eToolType::eBIG; // ToolCnt 에서 마지막 부터  
					m_vToolForeign.push_back(i);
				}
				//std::vector<int> groups[nF];
			}

		}
		m_vToolIndexSort = nToolIndexSort;

		if (InspRoot::Init() == false)
			bInit = false;
		if (_stPreStartParam.Init() == false)
			bInit = false;
		if (EnvVariable::nSystemType != EXT_INSP_CLIENT)
		{
		if (_SMachineData.Init((int)EnvVariable::nToolCount) == false)
			bInit = false;


		_Tools.resize(EnvVariable::nToolCount);
		for (int i = 0; i < _Tools.size(); i++)
		{
				if (_Tools[i].Init(i + 1, nMachineType) == false)
				bInit = false;
		}

			if (_Ctrl.Init() == false)
			bInit = false;

		_bInitialized = bInit;
		}
	

		return _bInitialized;
	}
	void InspRoot_Server::Exit()
	{
		InspRoot::Exit();
		for (int i = 0; i < _Tools.size(); i++)
		{
			HANDLE hd;
			hd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, _Tools[i]._ctrl.GetProcessID()); //process ID 는 하나만 체크해도됨.
			TerminateProcess(hd, 0);
			_Tools[i].Exit();
		}
		_Ctrl.Exit();
		_SMachineData.Exit();
		_stPreStartParam.Exit();
	}
	void InspRoot_Server::SetCoreNum()
	{
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	}


	InspRoot_Client::InspRoot_Client()
	{
		_TotalBuff = 0;
	}
	InspRoot_Client::~InspRoot_Client()
	{

	}

	bool InspRoot_Client::Init()
	{
		bool bInit(true);
		ext::Log::add(_T("InspRoot_Client::Init() - Start"));
		if (InspRoot::Init() == false)
			bInit = false;
		if (_Ctrl.Init(env::nTool_id) == false)
			bInit = false;

		if (_CtrlServer.Init() == false)
			bInit = false;

		if (_CMachineData.Init(0) == false)
			bInit = false;
		/*if (_Result.Init(env::nTool_id, &_Ctrl, &_Job) == false)
			bInit = false;*/
		 unsigned long long TotalBuff = InspRoot::_TotalBuff+ _Ctrl._TotalBuff + _CtrlServer._TotalBuff + _CMachineData._TotalBuff;
		
		_bInitialized = bInit;
		CString sLog; 
		int nMegaByte = 1048576;
		unsigned long long Size = _TotalBuff / nMegaByte;
		sLog.Format(_T("[TotalBuff][Init] InspRoot_Client:[%llu (MB)] "), Size);
		ext::Log::add(sLog);
		ext::Log::add(_T("InspRoot_Client::Init() - End"));
		return _bInitialized;
	}
	void InspRoot_Client::Exit()
	{
		InspRoot::Exit();
		_Ctrl.Exit();
		_CtrlServer.Exit();
		_CMachineData.Exit();
		//_Result.Exit();
	}



	InspLogicBase::InspLogicBase()
		: _pThread(nullptr)
		, _bRunThread(false)
		, _WaitForDelete(100)
		, _MainSleep(10)
	{

	}
	bool InspLogicBase::Init()
	{
		_bRunThread = true;
		_pThread = AfxBeginThread(InspLogicBase::thProcedure, this);
		_pThreadImageSave = AfxBeginThread(InspLogicBase::thSaveImage, this);
		return true;
	}
	bool InspLogicBase::Init(int nStep)
	{
		_nMainSeqStep = nStep;
		_bRunThread = true;
		_pThread = AfxBeginThread(InspLogicBase::thSaveFrame, this);
		return true;
	}

	void InspLogicBase::Exit()
	{
		if (_pThread)
		{
			_bRunThread = false;
			HANDLE hThread = _pThread->m_hThread;
			DWORD dwRes = ::WaitForSingleObject(hThread, _WaitForDelete);

			if (WAIT_TIMEOUT)
				::TerminateThread(hThread, -1);
			_pThread = NULL;
		}
	}
	int InspLogicBase::GetCurrentStep()
	{
		return _nMainSeqStep;
	}
	void InspLogicBase::Begin()
	{

	}
	void InspLogicBase::Proc()
	{
	}
	void InspLogicBase::End()
	{

	}
	void InspLogicBase::Proc_SaveImage()
	{
	}
	void InspLogicBase::Proc_FrameSave()
	{

	}
	void InspLogicBase::NextStep(int Step)
	{
		_nMainSeqStep = Step;
		ext::ClientCtrl::get()->_ctrl.First()->nMainAutoSeqStep = Step;
	}
	UINT InspLogicBase::thProcedure(LPVOID param)
	{
		InspLogicBase * Owner = (InspLogicBase *)param;
		Owner->Begin();
		while (Owner->_bRunThread == true)
		{
			Owner->Proc();
			::Sleep(Owner->_MainSleep);
		}
		Owner->End();
		return 0;
	}
	UINT InspLogicBase::thSaveImage(LPVOID param)
	{
		InspLogicBase * Owner = (InspLogicBase *)param;
		while (Owner->_bRunThread == true)
		{
			Owner->Proc_SaveImage();
			::Sleep(Owner->_MainSleep);
		}
		return 0;
	}
	UINT InspLogicBase::thSaveFrame(LPVOID param)
	{
		InspLogicBase* Owner = (InspLogicBase*)param;
		while (Owner->_bRunThread == true)
		{
			Owner->Proc_FrameSave();
			::Sleep(Owner->_MainSleep);
		}
		return 0;
	}
	_ImageBuffer::_ImageBuffer()
		: _offset(0)
		, _sizebytes(0)
		, _qwCurPos(0)
	{

	}

	bool _ImageBuffer::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}

		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			_stream->AllocMemory(_sizebytes, sName);

		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit = false;
		return bInit;
	}

	bool _ImageBuffer::Init(CString sSpaceName, int nImageType, int nToolType)
	{

		CString strLog;
		bool bInit = true;

		strLog.Format(_T(" ImageBuffer::Init() - Start: %s"), sSpaceName);
		ext::Log::add(strLog);

		int access = FILE_MAP_ALL_ACCESS;

		if (env::nSystemType != EXT_INSP_SERVER)
			access = FILE_MAP_READ;

		int nBufferSize = 0;

		//이미지 타입, 툴 타입(노말파트용, 빅파트용)에 따라 이미지 초기화 사이즈가 달라짐
		switch (nImageType)
		{
		case (int)ext::eImageType::e2DImg:
			if (nToolType == (int)ext::eToolType::eNORMAL)
				nBufferSize = env::nImgBufferSize2D;
			else
				nBufferSize = env::nBigImgBufferSize2D;
			break;
		case (int)ext::eImageType::e3DImg:
			if (nToolType == (int)ext::eToolType::eNORMAL)
				nBufferSize = env::nImgBufferSize3D;
			else
				nBufferSize = env::nBigImgBufferSize3D;
			break;
		}

		bInit = Create(nBufferSize, sSpaceName, access);
		if (!bInit)
		{
			strLog.Format(_T("%s_Init_Fail"), sSpaceName);
			ext::Log::add(strLog);
		}
		
		strLog.Format(_T(" ImageBuffer::Init() - End: %s"), sSpaceName);
		ext::Log::add(strLog);
		return bInit;
	}
	void _ImageBuffer::Push(void * ptr, int nW, int nH, int nCh)
	{
		BYTE * buf = _stream->Byte();
		IppStatus sts;
		IppiSize iSize = { nW,nH };
		if (nCh == 1)
		{
			sts = ippiCopy_8u_C1R((BYTE *)ptr, nW, buf, nW, iSize);
			_stream->Flush(buf, nW*nH);
		}
		else
		{
			sts = ippiCopy_32f_C1R((float*)ptr, nW * sizeof(float), (float*)buf, nW * sizeof(float), iSize);
			_stream->Flush(buf, nW*nH* sizeof(float));
		}

	}
	void _ImageBuffer::Clear()
	{
		
	}
	void _ImageBuffer::Exit()
	{
		if (_stream != nullptr)
			_stream->Free();
	}


	_ImageMasterBuffer::_ImageMasterBuffer()
	{
		_TotalBuff = 0; 
	}
	bool _ImageMasterBuffer::Init(int nToolID , int nBufferIndex, int nToolType, int nMachineType)
	{
		m_nToolType = nToolType;
		_nMachineType = nMachineType;
		CString sSpaceName,sLog;
		if(nToolType == (int)ext::eToolType::eNORMAL)
			sLog.Format(_T("Tool idx_%d, Normal_Type ImageBuffer Init Start."), nToolID);
		else
			sLog.Format(_T("Tool idx_%d, Big_Type ImageBuffer Init Start."), nToolID);

		ext::Log::add(sLog);

		bool bInit = true; 
		for (int nNum = 0; nNum < (int)InspRoiImgBuf_no::eInspRoiImg_NUM; nNum++)  // 채널 별로, R , G, B ,W ..... 3D
		{
			if(nNum >= MachineTypeImageBufferCount(nMachineType))
				continue;
			sSpaceName.Format(_T("AOI_ExtPartImage_%d_%d_%d"), nToolID, nBufferIndex, nNum);
			bInit &= _ChannelBuffer[nNum].Init(sSpaceName, (int)ext::eImageType::e2DImg,  m_nToolType);
			_TotalBuff += _ChannelBuffer[nNum]._sizebytes;
		}
		for (int nNum = 0; nNum < RoiColorBuf_no::eRoiColorBuf_img_R_BtmSide; nNum++) // Color Angle 3ch 만
		{
			sSpaceName.Format(_T("AOI_ExtAnglePartImage_%d_%d_%d"), nToolID, nBufferIndex, nNum);
			bInit &= _ChannelAngleBuffer[nNum].Init(sSpaceName, (int)ext::eImageType::e2DImg, m_nToolType);
			_TotalBuff += _ChannelBuffer[nNum]._sizebytes;
		}
		if (nMachineType == ext::eMachineType::eAOI)
		{
		sSpaceName.Format(_T("AOI_ExtPartImage_3D_%d_%d"), nToolID, nBufferIndex);
		bInit &= _ChannelBuffer[EXT_3D_BUFFER_INDEX].Init(sSpaceName, (int)ext::eImageType::e3DImg, m_nToolType); // Side 안쓸경우 바꿔줘야함...
		_TotalBuff += _ChannelBuffer[EXT_3D_BUFFER_INDEX]._sizebytes;
		sSpaceName.Format(_T("AOI_ExtPartImage_3D_Raw_%d_%d"), nToolID, nBufferIndex);
		bInit &= _ChannelBuffer[EXT_3D_RAW_BUFFER_INDEX].Init(sSpaceName, (int)ext::eImageType::e3DImg, m_nToolType); // Side 안쓸경우 바꿔줘야함...
		_TotalBuff += _ChannelBuffer[EXT_3D_BUFFER_INDEX]._sizebytes;
		// imgAI 전용 버퍼 초기화
		sSpaceName.Format(_T("AOI_ExtPartImage_AI_%d_%d"), nToolID, nBufferIndex);
		bInit &= _ChannelBuffer[EXT_AI_BUFFER_INDEX].Init(sSpaceName, (int)ext::eImageType::e2DImg, m_nToolType);
		_TotalBuff += _ChannelBuffer[EXT_AI_BUFFER_INDEX]._sizebytes;
		}


		if (nToolType == (int)ext::eToolType::eNORMAL)
			sLog.Format(_T("Tool idx_%d, Normal_Type ImageBuffer Init End."), nToolID);
		else
			sLog.Format(_T("Tool idx_%d, Big_Type ImageBuffer Init End."), nToolID);

		ext::Log::add(sLog);

		return bInit;
	}
	void _ImageMasterBuffer::AddImages(InspRoiImgBuf * partImgBuf, RoiColorBuf *partImgColorBuf, ZmapData*  partZmapData, BYTE* by3DRawImg, int n3DRaw, int nBufferIndex )
	{
		for (int nNum = 0; nNum < (int)InspRoiImgBuf_no::eInspRoiImg_NUM; nNum++)  // 채널 별로, R , G, B ,W ..... 3D
		{
			if (nNum >= MachineTypeImageBufferCount(_nMachineType))
				continue;
			_ChannelBuffer[nNum].Push((_nMachineType == ext::eMachineType::eAOI)? (void *)partImgBuf->GetPtrImg(nNum) : (void *)partImgBuf->GetPtrImg(nBufferIndex),
				partImgBuf->nImageSizeX, partImgBuf->nImageSizeY, 1);
		}
		if ((_nMachineType == ext::eMachineType::eAOI))
		{
			for (int nNum = 0; nNum < RoiColorBuf_no::eRoiColorBuf_img_R_BtmSide; nNum++) // Color Angle 3ch 만 이미지 의 Width 와 Height 는 2D 에서 가져오자
			{
				_ChannelAngleBuffer[nNum].Push((void *)partImgColorBuf->GetPtrImg(nNum) ,partImgBuf->nImageSizeX, partImgBuf->nImageSizeY, 1);
			}
		}
		if (_nMachineType == ext::eMachineType::eAOI)
		{
		_ChannelBuffer[EXT_3D_BUFFER_INDEX].Push((void*)partZmapData->data, partImgBuf->nImageSizeX, partImgBuf->nImageSizeY, 4);

		if(n3DRaw > 0)
			_ChannelBuffer[EXT_3D_RAW_BUFFER_INDEX].Push((void*)by3DRawImg, n3DRaw, 1, 1);
		}
		// imgAI 데이터 Push
		if (partImgBuf->imgAI != nullptr)
		{
			_ChannelBuffer[EXT_AI_BUFFER_INDEX].Push((void*)partImgBuf->imgAI, partImgBuf->nImageSizeX, partImgBuf->nImageSizeY, 1);
		}
	}
	void _ImageMasterBuffer::GetImages(InspRoiImgBuf * partImgBuf, RoiColorBuf * partImgColorBuf, ZmapData*  partZmapData, BYTE *& by3DRawImg, int sz3DRaw, int nBufferIndex)
	{
		for (int nNum = 0; nNum < (int)InspRoiImgBuf_no::eInspRoiImg_NUM; nNum++)  // 채널 별로, R , G, B ,W ..... 3D
		{
			if (nNum >= MachineTypeImageBufferCount(_nMachineType))
				continue;
			partImgBuf->GetPtrImg(nNum) = _ChannelBuffer[nNum]._stream->Byte();
		}
		for (int nNum = 0; nNum < RoiColorBuf_no::eRoiColorBuf_img_R_BtmSide; nNum++) // Color Angle 3ch 만 이미지 의 Width 와 Height 는 2D 에서 가져오자
		{
			partImgColorBuf->GetPtrImg(nNum) = _ChannelAngleBuffer[nNum]._stream->Byte();
		}
		if (_nMachineType == ext::eMachineType::eAOI)
		{
			partZmapData->zmapSizeX = partImgBuf->nImageSizeX;
			partZmapData->zmapSizeY = partImgBuf->nImageSizeY;
			partZmapData->data = _ChannelBuffer[EXT_3D_BUFFER_INDEX]._stream->Float();

			if(sz3DRaw > 0)
				by3DRawImg = _ChannelBuffer[EXT_3D_RAW_BUFFER_INDEX]._stream->Byte(0);

			// imgAI 포인터 연결
			partImgBuf->imgAI = _ChannelBuffer[EXT_AI_BUFFER_INDEX]._stream->Byte();
			cv::Mat tmp(partImgBuf->nImageSizeY, partImgBuf->nImageSizeX, CV_8UC1, partImgBuf->imgAI);
		}
		else
		{
			UCHAR * bytTemp =  partImgBuf->GetPtrImg(0);//WIR 은 0번에 다가 넣어줄것 
			for (int nNum = 1; nNum <= (int)InspRoiImgBuf_no::eInspRoiImg_imgBottom_B; nNum++)
				partImgBuf->GetPtrImg(nNum) = bytTemp;
		}
	}
	void _ImageMasterBuffer::Exit()
	{
		for (int nNum = 0; nNum < (int)ext::eImageChannel::eNUM; nNum++)  // 채널 별로, R , G, B ,W ..... 3D
		{
			_ChannelBuffer[nNum].Exit();
		}
		for (int nNum = 0; nNum < RoiColorBuf_no::eRoiColorBuf_img_R_BtmSide; nNum++)
		{
			_ChannelAngleBuffer[nNum].Exit();
		}
	}
	void _ImageMasterBuffer::Clear()
	{
		for (int nNum = 0; nNum < (int)InspRoiImgBuf_no::eInspRoiImg_NUM; nNum++)  // 채널 별로, R , G, B ,W ..... 3D
		{
			if (nNum >= MachineTypeImageBufferCount(_nMachineType))
				continue;
			_ChannelBuffer[nNum].Clear();
		}
		if (_nMachineType == ext::eMachineType::eAOI)
		{
		_ChannelBuffer[EXT_3D_BUFFER_INDEX].Clear();
		_ChannelBuffer[EXT_3D_RAW_BUFFER_INDEX].Clear();
		_ChannelBuffer[EXT_AI_BUFFER_INDEX].Clear();
	}
	}
	int _ImageMasterBuffer::MachineTypeImageBufferCount(int nMachineType)
	{
		int ResultCnt = -1; // 
		switch (nMachineType)
		{
		case (int)ext::eMachineType::eAOI:
		{
			ResultCnt = EXT_3D_BUFFER_INDEX;
			break;
		}
		case (int)ext::eMachineType::eWIR:
		{
			ResultCnt = 1; // 조명에 따라 늘어날수도있음.
			break;
		}
		default:
			break;
		}
		return ResultCnt;

	}

	
	_Part::_Part()
		:_qwCurPos(0)
	{
		_TotalBuff = 0;
	}
	bool _Part::Init(int nToolID)
	{
		bool bInit = true;

		CString sName, sLog;
		sName.Format(_T("AOI_ExtParts_Tool_%d_Index_%d"), nToolID, m_nBuffIdx);

		sLog.Format(_T("Part Buffer Index_%d - Start()"), m_nBuffIdx);
		ext::Log::add(sLog);

		Memory<InspPartInfo>::Init();
		
		_Light.m_nBuffIdx = m_nBuffIdx;
		_Light.Init(nToolID);

// 		_Map.m_nBuffIdx = m_nBuffIdx;
// 		_Map.Init(nToolID);
		
		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;

		if (Create(EnvVariable::nPartBufferSize, sName, nAccessType) == false)
			bInit = false;
		if (!bInit)
		{
			sLog.Format(_T("%s_Init_Fail"), sName);
			ext::Log::add(sLog);
		}
			

		sLog.Format(_T("Part Buffer Index_%d - Start()"), m_nBuffIdx);
		ext::Log::add(sLog);

		return bInit;
	}
	void _Part::Exit()
	{
		_stream.Free();
	}

	bool _Part::AddInspData(InspPartInfo * PartData, InspPartParam * pWind, int nWndCnt)
	{
		bool bAddSuccess = true;

		//Offset 초기화
		_qwCurPos = 0;
		PartData->nWindowCount = nWndCnt;
		AddPart(PartData);
		AddWindow(pWind, nWndCnt);
		AddCommonAlgo(pWind, nWndCnt);
		AddAlgo(pWind, nWndCnt);
		AddArrMaskingROI(pWind, nWndCnt);
		AddPartPad(PartData);
		//패널버퍼 20개에 맞춰 현재 파트가 들어갈 index 갱신
		int nCurrentIdx = irs::get()->GetCurrentPannelCount();
		irs::get()->_PartIndex.insert(std::pair<int, int>(PartData->nPartIDOrg, nCurrentIdx));
		irs::get()->AddPannelCount();

		CopyHeader();

		return bAddSuccess;
	}

	bool _Part::AddPart(InspPartInfo * PartData)
	{
		
		bool bAddSuccess = true;

		BYTE * ptr = _stream.Byte(_qwCurPos);
		tagSharedPartHeader tgheader;
		tgheader.m_nZerobyte = 0;

		memcpy(ptr + _qwCurPos, &tgheader, sizeof(tagSharedPartHeader));
		_qwCurPos += sizeof(tagSharedPartHeader);
		//Part
		memcpy(ptr + _qwCurPos, PartData,  sizeof(InspPartInfo));
		_qwCurPos += sizeof(InspPartInfo);

		_stream.Flush(ptr, _qwCurPos);

		m_qwArrOffset[0] = _qwCurPos;

		return bAddSuccess;
	}
	bool _Part::AddPartPad(InspPartInfo * PartData)
	{

		bool bAddSuccess = true;

		BYTE * ptr = _stream.Byte();
		//Part
		//PADPatternPoly m_sPadPoly; 
		memcpy(ptr + _qwCurPos, PartData->m_sPadPoly.m_ptArrPoly, (PartData->m_sPadPoly.m_nPolyCnt * sizeof(POINTF)));
		_qwCurPos += (PartData->m_sPadPoly.m_nPolyCnt * sizeof(POINTF));

		memcpy(ptr + _qwCurPos, PartData->m_sArrOverlapPadPoly, (PartData->m_nOverlapPadPolyCnt * sizeof(PADPatternPoly)));
		_qwCurPos += (PartData->m_nOverlapPadPolyCnt * sizeof(PADPatternPoly));

		for (int i = 0; i < PartData->m_nOverlapPadPolyCnt; i++)
		{
			memcpy(ptr + _qwCurPos, PartData->m_sArrOverlapPadPoly[i].m_ptArrPoly, (PartData->m_sArrOverlapPadPoly[i].m_nPolyCnt * sizeof(POINTF)));
			_qwCurPos += (PartData->m_sArrOverlapPadPoly[i].m_nPolyCnt * sizeof(POINTF));
		}
		

		_stream.Flush(ptr, _qwCurPos);

		m_qwArrOffset[5] = _qwCurPos;

		return bAddSuccess;
	}
	bool _Part::AddWindow(InspPartParam * pWnd, int nWnd)
	{

		bool bAddSuccess = true;

		BYTE * ptr = _stream.Byte(_qwCurPos);

		//Window
		memcpy(ptr , pWnd, sizeof(InspPartParam) * nWnd);
		_qwCurPos += sizeof(InspPartParam) * nWnd;

		_stream.Flush(ptr, sizeof(InspPartParam) * nWnd);

		m_qwArrOffset[1] = _qwCurPos;

		return bAddSuccess;
	}
	bool _Part::AddCommonAlgo(InspPartParam * pWnd, int nWnd)
	{

		bool bAddSuccess = true;

		BYTE * ptr = _stream.Byte(_qwCurPos);
		int nTempIndex = 0; 
		int nTotalAlgoCnt = 0;
		for (int i = 0; i < nWnd; i++)
		{
			InspPartParam * pWndParam = &pWnd[i];
			for (int j = 0; j < pWndParam->nAlgorithmCnt; j++)
			{
				InspAlgo * pAlgo = &pWndParam->vArrAlgoParam[j];
				memcpy(ptr+ nTempIndex, pAlgo, sizeof(InspAlgo));
				nTempIndex += sizeof(InspAlgo);
				_qwCurPos += sizeof(InspAlgo);
				nTotalAlgoCnt++;
			}
		}
		
		_stream.Flush(ptr, nTotalAlgoCnt * sizeof(InspAlgo));

		m_qwArrOffset[2] = _qwCurPos;

		return bAddSuccess;
	}
	bool _Part::AddArrMaskingROI(InspPartParam * pWnd, int nWnd)
	{
		bool bAddSuccess = true;

		BYTE * ptr = _stream.Byte(_qwCurPos);
		int nTempIndex = 0;
		int nTotalMaskingROICnt = 0;
		for (int i = 0; i < nWnd; i++)
		{
			InspPartParam * pWndParam = &pWnd[i];
			for (int j = 0; j < pWndParam->nAlgorithmCnt; j++)
			{
				InspAlgo * pAlgo = &pWndParam->vArrAlgoParam[j];
				RECT* arrMaskingROI = pAlgo->m_rcArrMaskingROI;
				memcpy(ptr + nTempIndex, arrMaskingROI, sizeof(RECT) * pAlgo->m_nUsedMaskingValue);
				nTempIndex += sizeof(RECT) * pAlgo->m_nUsedMaskingValue;
				_qwCurPos += sizeof(RECT) * pAlgo->m_nUsedMaskingValue;
				nTotalMaskingROICnt += pAlgo->m_nUsedMaskingValue;
			}
		}

		_stream.Flush(ptr, nTotalMaskingROICnt * sizeof(RECT));

		m_qwArrOffset[4] = _qwCurPos;

		return bAddSuccess;
	}
	bool _Part::AddAlgo(InspPartParam * pWnd, int nWnd)
	{
		bool bAddSuccess = true;

		BYTE * ptr = _stream.Byte(_qwCurPos);
		QWORD qwTotalAlgoSize = 0;
		for (int i = 0; i < nWnd; i++)
		{
			InspPartParam * pWndParam = &pWnd[i];
			for (int j = 0; j < pWndParam->nAlgorithmCnt; j++)
			{
				InspAlgo * pAlgo = &pWndParam->vArrAlgoParam[j];
				int nSize = AlgoClone(pAlgo->m_eAlgoType, pAlgo->m_ptrInspAlgoParam);
				_qwCurPos += nSize;
				qwTotalAlgoSize += nSize;
			}
		}

		_stream.Flush(ptr , qwTotalAlgoSize);

		m_qwArrOffset[3] = _qwCurPos;

		return bAddSuccess;
	}

	int _Part::AlgoClone(InspAlgoType eAlgoType, LPVOID& lpSource)
	{
		int stSize(0);
		LPVOID* lpDst = (LPVOID*)_stream.Byte(_qwCurPos);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			memcpy_s(lpDst, sizeof(AlgoAlign), lpSource, stSize = sizeof(AlgoAlign));
			break;
		case eAlgoBW:
			memcpy_s(lpDst, sizeof(AlgoBW), lpSource, stSize = sizeof(AlgoBW));
			break;
		case eAlgoBlob:
			memcpy_s(lpDst, sizeof(AlgoBlob), lpSource, stSize = sizeof(AlgoBlob));
			break;
		case eAlgoBody_Blob:
			memcpy_s(lpDst, sizeof(AlgoBodyBlob), lpSource, stSize = sizeof(AlgoBodyBlob));
			break;
		case eAlgoTilt:
			memcpy_s(lpDst, sizeof(AlgoTilt), lpSource, stSize = sizeof(AlgoTilt));
			break;
		case eAlgoOCR:
			memcpy_s(lpDst, sizeof(AlgoOCR), lpSource, stSize = sizeof(AlgoOCR));
			break;
		case eAlgoPattern:
			memcpy_s(lpDst, sizeof(AlgoPattern), lpSource, stSize = sizeof(AlgoPattern));
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			memcpy_s(lpDst, sizeof(AlgoColor), lpSource, stSize = sizeof(AlgoColor));
			break;
		case eAlgoGray_Mean:
			memcpy_s(lpDst, sizeof(AlgoGrayMean), lpSource, stSize = sizeof(AlgoGrayMean));
			break;
		case eAlgoHeight_Mean:
			memcpy_s(lpDst, sizeof(AlgoHeightMean), lpSource, stSize = sizeof(AlgoHeightMean));
			break;
		case eAlgoGray_Diff:
			memcpy_s(lpDst, sizeof(AlgoGrayDiff), lpSource, stSize = sizeof(AlgoGrayDiff));
			break;
		case eAlgoHeight_Diff:
			memcpy_s(lpDst, sizeof(AlgoHeightDiff), lpSource, stSize = sizeof(AlgoHeightDiff));
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			memcpy_s(lpDst, sizeof(AlgoLeadSearch), lpSource, stSize = sizeof(AlgoLeadSearch));
			break;
		case eAlgoBridge:
			memcpy_s(lpDst, sizeof(AlgoBridge), lpSource, stSize = sizeof(AlgoBridge));
			break;
		case eAlgoLead_Tip:
			memcpy_s(lpDst, sizeof(AlgoLeadTip), lpSource, stSize = sizeof(AlgoLeadTip));
			break;
		case eAlgoLead_Lift:
			memcpy_s(lpDst, sizeof(AlgoLeadLift), lpSource, stSize = sizeof(AlgoLeadLift));
			break;
		case eAlgoLead_Solder:
			memcpy_s(lpDst, sizeof(AlgoLeadSolder), lpSource, stSize = sizeof(AlgoLeadSolder));
			break;
		case eAlgoLead_SideSolder:
			memcpy_s(lpDst, sizeof(AlgoLeadSideSolder), lpSource, stSize = sizeof(AlgoLeadSideSolder));
			break;
		case eAlgoWidth:
			memcpy_s(lpDst, sizeof(AlgoLength), lpSource, stSize = sizeof(AlgoLength));
			break;
		case eAlgoVolume:
			memcpy_s(lpDst, sizeof(AlgoVolume), lpSource, stSize = sizeof(AlgoVolume));
			break;
		case eAlgoTab:
			memcpy_s(lpDst, sizeof(AlgoTab), lpSource, stSize = sizeof(AlgoTab));
			break;
		case eAlgoGrid:
			memcpy_s(lpDst, sizeof(AlgoGrid), lpSource, stSize = sizeof(AlgoGrid));
			break;
		case eAlgoLine:
			memcpy_s(lpDst, sizeof(AlgoLine), lpSource, stSize = sizeof(AlgoLine));
			break;
		case eAlgoEdge:
			memcpy_s(lpDst, sizeof(AlgoEdge), lpSource, stSize = sizeof(AlgoEdge));
			break;
		case eAlgoSolderCone:
			memcpy_s(lpDst, sizeof(AlgoSolderCone), lpSource, stSize = sizeof(AlgoSolderCone));
			break;
		case eAlgoColorXY:
			memcpy_s(lpDst, sizeof(AlgoColorXY), lpSource, stSize = sizeof(AlgoColorXY));
			break;
		case eAlgoAlignEdge:
			memcpy_s(lpDst, sizeof(AlgoAlignEdge), lpSource, stSize = sizeof(AlgoAlignEdge));
			break;
		case eAlgoPadAlign:
			memcpy_s(lpDst, sizeof(AlgoPadAlign), lpSource, stSize = sizeof(AlgoPadAlign));
			break;
		case eAlgoPOCR:
			memcpy_s(lpDst, sizeof(AlgoPOCR), lpSource, stSize = sizeof(AlgoPOCR));
			break;
		case eAlgoWire:
			memcpy_s(lpDst, sizeof(AlgoWire), lpSource, stSize = sizeof(AlgoWire));
			break;
		case eAlgoFoot:
			memcpy_s(lpDst, sizeof(AlgoFoot), lpSource, stSize = sizeof(AlgoFoot));
			break;
		case eAlgoBarcode:
			memcpy_s(lpDst, sizeof(AlgoBarcode), lpSource, stSize = sizeof(AlgoBarcode));
			break;
		case eAlgoFillet:
			memcpy_s(lpDst, sizeof(AlgoFillet), lpSource, stSize = sizeof(AlgoFillet));
			break;
		case eAlgoBGA:
			memcpy_s(lpDst, sizeof(AlgoBGA), lpSource, stSize = sizeof(AlgoBGA));
			break;
		case eAlgoBump:
			memcpy_s(lpDst, sizeof(AlgoBump), lpSource, stSize = sizeof(AlgoBump));
			break;
		case eAlgoNGBlob:
			memcpy_s(lpDst, sizeof(AlgoNGBlob), lpSource, stSize = sizeof(AlgoNGBlob));
			break;
		case eAlgoPadBW:
			memcpy_s(lpDst, sizeof(AlgoPadBW), lpSource, stSize = sizeof(AlgoPadBW));
			break;
		case eAlgoBodyEdge:
			memcpy_s(lpDst, sizeof(AlgoBodyEdge), lpSource, stSize = sizeof(AlgoBodyEdge));
			break;
		case eAlgoDistance:
			memcpy_s(lpDst, sizeof(AlgoDistance), lpSource, stSize= sizeof(AlgoDistance));
			break;
		case eAlgoPatternDiff:
			memcpy_s(lpDst, sizeof(AlgoPatternDiff), lpSource, stSize = sizeof(AlgoPatternDiff));
			break;
		case eAlgoShapeX:
			memcpy_s(lpDst, sizeof(AlgoShapeX), lpSource, stSize = sizeof(AlgoShapeX));
			break;
			//shw Delete Xcase eAlgoNewAlgo1:#@dst = g_pMManager->pem_new<AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);#@memcpy_s(dst, sizeof(AlgoNewAlgo1), lpSource, sizeof(AlgoNewAlgo1));#@break;
		default:
			break;
		}
		return stSize;
	}
	void _Part::CopyHeader()
	{
		//heager set
		BYTE * ptr = _stream.Byte(0);
		tagSharedPartHeader* tgheader = (tagSharedPartHeader*)ptr;
		tgheader->m_nZerobyte = 0;
		tgheader->m_qwOffSet[0] = 0;
		tgheader->m_qwOffSet[0] = m_qwArrOffset[0];
		tgheader->m_qwOffSet[1] = m_qwArrOffset[1];
		tgheader->m_qwOffSet[2] = m_qwArrOffset[2];
		tgheader->m_qwOffSet[3] = m_qwArrOffset[3];
		tgheader->m_qwOffSet[4] = m_qwArrOffset[4];

		_stream.Flush(ptr, sizeof(tagSharedPartHeader));
	}

	bool _Part::GetPartData(InspPartInfo* pDstPart, InspPartParam *& pDstWindow)
	{
		bool bRet = true;

		BYTE* ptr = _stream.Byte(0);
		tagSharedPartHeader* stHeader = (tagSharedPartHeader*)ptr;

		if (stHeader->m_nZerobyte != 0)
		{
			//write log
			ext::Log::add(_T("Part header is wrong."));
		}
		
		//part
		*pDstPart = (*(InspPartInfo*)_stream.Byte(sizeof(tagSharedPartHeader)));

		//window
		pDstWindow = ((InspPartParam*)_stream.Byte(stHeader->m_qwOffSet[0]));

		QWORD qwAlgoCommonOffset = stHeader->m_qwOffSet[1];
		QWORD qwAlgoDataOffset = stHeader->m_qwOffSet[2];
		QWORD qwAlgoMaskOffset = stHeader->m_qwOffSet[3];
		QWORD qwAlgoPadOffset = stHeader->m_qwOffSet[4];
		//InspAlgo * pAlgo = pDstWindow[0].vArrAlgoParam = (InspAlgo*)_stream.Byte(qwAlgoCommonOffset);

		_Light.SetPosZero();

		for (int i = 0; i < pDstPart->nWindowCount; i++)
		{
			if(pDstWindow[i].nAlgorithmCnt <= 0)
				continue;
			//common algo
			InspAlgo * pAlgo = pDstWindow[i].vArrAlgoParam = (InspAlgo*)_stream.Byte(qwAlgoCommonOffset);
			

			qwAlgoCommonOffset += (sizeof(InspAlgo ) *pDstWindow[i].nAlgorithmCnt);

			for (int j = 0; j < pDstWindow[i].nAlgorithmCnt; j++)
			{
				//algo data
				int nSize = SetAlgoData(pAlgo[j].m_eAlgoType, pAlgo[j].m_ptrInspAlgoParam, qwAlgoDataOffset);
				qwAlgoDataOffset += nSize;

				//mask data
				nSize = SetArrMaskingROIData(&pAlgo[j], qwAlgoMaskOffset);
				qwAlgoMaskOffset += nSize;

				// light
				_Light.Get(&pAlgo[j]); //size랑 연관무.
			}
		}

		pDstPart->m_sPadPoly.m_ptArrPoly = (POINTF*)_stream.Byte(qwAlgoPadOffset);
		qwAlgoPadOffset += (sizeof(POINTF) * pDstPart->m_sPadPoly.m_nPolyCnt);
		pDstPart->m_sArrOverlapPadPoly = (PADPatternPoly*)_stream.Byte(qwAlgoPadOffset);
		qwAlgoPadOffset += (sizeof(PADPatternPoly) *  pDstPart->m_nOverlapPadPolyCnt);
		for (int j = 0; j < pDstPart->m_nOverlapPadPolyCnt; j++)
		{
			pDstPart->m_sArrOverlapPadPoly[j].m_ptArrPoly = (POINTF*)_stream.Byte(qwAlgoPadOffset);
			qwAlgoPadOffset += (sizeof(POINTF) *pDstPart->m_sArrOverlapPadPoly[j].m_nPolyCnt);
			
		}

		return bRet;
	}

	int _Part::SetAlgoData(InspAlgoType eAlgoType, LPVOID& lpDst, QWORD qwOffSet)
	{
		int stSize = 0;

		LPVOID* lpSource = (LPVOID*)_stream.Byte(qwOffSet);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			lpDst = (AlgoAlign*)lpSource;
			stSize = sizeof(AlgoAlign);
			break;
		case eAlgoBW:
			lpDst = (AlgoBW*)lpSource;
			stSize = sizeof(AlgoBW);
			break;
		case eAlgoBlob:
			lpDst = (AlgoBlob*)lpSource;
			stSize = sizeof(AlgoBlob);
			break;
		case eAlgoBody_Blob:
			lpDst = (AlgoBodyBlob*)lpSource;
			stSize = sizeof(AlgoBodyBlob);
			break;
		case eAlgoTilt:
			lpDst = (AlgoTilt*)lpSource;
			stSize = sizeof(AlgoTilt);
			break;
		case eAlgoOCR:
			lpDst = (AlgoOCR*)lpSource;
			stSize = sizeof(AlgoOCR);
			break;
		case eAlgoPattern:
			lpDst = (AlgoPattern*)lpSource;
			stSize = sizeof(AlgoPattern);
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			lpDst = (AlgoColor*)lpSource;
			stSize = sizeof(AlgoColor);
			break;
		case eAlgoGray_Mean:
			lpDst = (AlgoGrayMean*)lpSource;
			stSize = sizeof(AlgoGrayMean);
			break;
		case eAlgoHeight_Mean:
			lpDst = (AlgoHeightMean*)lpSource;
			stSize = sizeof(AlgoHeightMean);
			break;
		case eAlgoGray_Diff:
			lpDst = (AlgoGrayDiff*)lpSource;
			stSize = sizeof(AlgoGrayDiff);
			break;
		case eAlgoHeight_Diff:
			lpDst = (AlgoHeightDiff*)lpSource;
			stSize = sizeof(AlgoHeightDiff);
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			lpDst = (AlgoLeadSearch*)lpSource;
			stSize = sizeof(AlgoLeadSearch);
			break;
		case eAlgoBridge:
			lpDst = (AlgoBridge*)lpSource;
			stSize = sizeof(AlgoBridge);
			break;
		case eAlgoLead_Tip:
			lpDst = (AlgoLeadTip*)lpSource;
			stSize = sizeof(AlgoLeadTip);
			break;
		case eAlgoLead_Lift:
			lpDst = (AlgoLeadLift*)lpSource;
			stSize = sizeof(AlgoLeadLift);
			break;
		case eAlgoLead_Solder:
			lpDst = (AlgoLeadSolder*)lpSource;
			stSize = sizeof(AlgoLeadSolder);
			break;
		case eAlgoLead_SideSolder:
			lpDst = (AlgoLeadSideSolder*)lpSource;
			stSize = sizeof(AlgoLeadSideSolder);
			break;
		case eAlgoWidth:
			lpDst = (AlgoLength*)lpSource;
			stSize = sizeof(AlgoLength);
			break;
		case eAlgoVolume:
			lpDst = (AlgoVolume*)lpSource;
			stSize = sizeof(AlgoVolume);
			break;
		case eAlgoTab:
			lpDst = (AlgoTab*)lpSource;
			stSize = sizeof(AlgoTab);
			break;
		case eAlgoGrid:
			lpDst = (AlgoGrid*)lpSource;
			stSize = sizeof(AlgoGrid);
			break;
		case eAlgoLine:
			lpDst = (AlgoLine*)lpSource;
			stSize = sizeof(AlgoLine);
			break;
		case eAlgoEdge:
			lpDst = (AlgoEdge*)lpSource;
			stSize = sizeof(AlgoEdge);
			break;
		case eAlgoSolderCone:
			lpDst = (AlgoSolderCone*)lpSource;
			stSize = sizeof(AlgoSolderCone);
			break;
		case eAlgoColorXY:
			lpDst = (AlgoColorXY*)lpSource;
			stSize = sizeof(AlgoColorXY);
			break;
		case eAlgoAlignEdge:
			lpDst = (AlgoAlignEdge*)lpSource;
			stSize = sizeof(AlgoAlignEdge);
			break;
		case eAlgoPadAlign:
			lpDst = (AlgoPadAlign*)lpSource;
			stSize = sizeof(AlgoPadAlign);
			break;
		case eAlgoPOCR:
			lpDst = (AlgoPOCR*)lpSource;
			stSize = sizeof(AlgoPOCR);
			break;
		case eAlgoWire:
			lpDst = (AlgoWire*)lpSource;
			stSize = sizeof(AlgoWire);
			break;
		case eAlgoFoot:
			lpDst = (AlgoFoot*)lpSource;
			stSize = sizeof(AlgoFoot);
			break;
		case eAlgoBarcode:
			lpDst = (AlgoBarcode*)lpSource;
			stSize = sizeof(AlgoBarcode);
			break;
		case eAlgoFillet:
			lpDst = (AlgoFillet*)lpSource;
			stSize = sizeof(AlgoFillet);
			break;
		case eAlgoBGA:
			lpDst = (AlgoBGA*)lpSource;
			stSize = sizeof(AlgoBGA);
			break;
		case eAlgoBump:
			lpDst = (AlgoBump*)lpSource;
			stSize = sizeof(AlgoBump);
			break;
		case eAlgoNGBlob:
			lpDst = (AlgoNGBlob*)lpSource;
			stSize = sizeof(AlgoNGBlob);
			break;
		case eAlgoPadBW:
			lpDst = (AlgoPadBW*)lpSource;
			stSize = sizeof(AlgoPadBW);
			break;
		case eAlgoBodyEdge:
			lpDst = (AlgoBodyEdge*)lpSource;
			stSize = sizeof(AlgoBodyEdge);
			break;
		case eAlgoDistance:
			lpDst = (AlgoDistance*)lpSource;
			stSize = sizeof(AlgoDistance);
			break;
		case eAlgoPatternDiff:
			lpDst = (AlgoPatternDiff*)lpSource;
			stSize = sizeof(AlgoPatternDiff);
			break;
		case eAlgoShapeX:
			lpDst = (AlgoShapeX*)lpSource;
			stSize = sizeof(AlgoShapeX);
			break;
			//shw Delete Xcase eAlgoNewAlgo1:#@dst = g_pMManager->pem_new<AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);#@memcpy_s(dst, sizeof(AlgoNewAlgo1), lpSource, sizeof(AlgoNewAlgo1));#@break;
		default:
			break;
		}

		return stSize;
	}

	int _Part::SetArrMaskingROIData(InspAlgo * pAlgo, QWORD qwOffSet)
	{
		int stSize = 0;
		LPVOID* lpSource = (LPVOID*)_stream.Byte(qwOffSet);
		pAlgo->m_rcArrMaskingROI = (RECT*)lpSource;// new RECT[pAlgo->m_nUsedMaskingValue];

		stSize = sizeof(RECT)*pAlgo->m_nUsedMaskingValue;
		return stSize;
	}


	_PartRst::_PartRst()
		:_qwCurPos(0)
	{
		_TotalBuff = 0;
	}
	bool _PartRst::Init(int nToolID)
	{
		bool bInit = true;

		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;

		CString sName, sLog;
		sName.Format(_T("AOI_ExtPartRsts_Tool_%d_Index_%d"), nToolID, m_nBuffIdx);

		sLog.Format(_T("Result Buffer Index_%d - Start()"), m_nBuffIdx);
		ext::Log::add(sLog);

		if (Create(env::nRstBufferSize, sName, nAccessType) == false)
			bInit = false;

		if (!bInit)
		{
			sLog.Format(_T("%s_Init_Fail"), sName);
			ext::Log::add(sLog);
		}
			

		sLog.Format(_T("Result Buffer Index_%d - End()"), m_nBuffIdx);
		ext::Log::add(sLog);

		return bInit;

	}
	void _PartRst::Exit()
	{
		_stream.Free();
	}
	int _PartRst::AddResult(InspectionResult * PartData,int nDefectCode)
	{
		AddPartResult(PartData);

		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
			AddPartTempResult(PartData->GetWndParam(nWndType), PartData->GetWndArraySize(nWndType));
		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
			AddWndResult(PartData->GetWndResult(nWndType), PartData->GetWndArraySize(nWndType));
			
		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
			AddCommonAlgo(PartData->GetWndParam(nWndType), PartData->GetWndArraySize(nWndType));
		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
			AddCommonAlgoResult(PartData->GetWndResult(nWndType), PartData->GetWndArraySize(nWndType));
			
		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
			AddAlgo(PartData->GetWndParam(nWndType), PartData->GetWndArraySize(nWndType));
		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
			AddAlgoResult(PartData->GetWndResult(nWndType), PartData->GetWndArraySize(nWndType));

		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
			AddArrMaskingROI(PartData->GetWndParam(nWndType), PartData->GetWndArraySize(nWndType));
		

		m_qwArrOffset[9] = nDefectCode;
		CopyHeader();
		

		return 1;
	}
	int _PartRst::GetResult(InspectionResult * PartData)
	{
		int nExtDefectCode = 0; 
		QWORD qwGetOffset = 0;
		BYTE* ptr = _stream.Byte(qwGetOffset);
		tagSharedPartHeader* stHeader = (tagSharedPartHeader*)ptr;
		if (stHeader->m_nZerobyte != 0)
		{
			ext::Log::add(_T("Part header is wrong."));
		}
		*PartData = (*(InspectionResult*)_stream.Byte(sizeof(tagSharedPartHeader)));
		qwGetOffset = stHeader->m_qwOffSet[0];
		QWORD qwAlgoCommonOffset = stHeader->m_qwOffSet[2];
		QWORD qwAlgoDataOffset = stHeader->m_qwOffSet[4];
		QWORD qwAlgoMaskOffset = stHeader->m_qwOffSet[6];
		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
		{
			InspParamTemp *  wnd = (InspParamTemp *)SetWndTempRst(nWndType, PartData, qwGetOffset, true);
			qwGetOffset += sizeof(InspParamTemp) * PartData->GetWndArraySize(nWndType);
			for (int nWndCnt = 0; nWndCnt < PartData->GetWndArraySize(nWndType); nWndCnt++) // offset 필요없음 
			{
				if(wnd[nWndCnt].nAlgorithmCnt <= 0 )
					continue;
				InspAlgo * pAlgo = (InspAlgo*)_stream.Byte(qwAlgoCommonOffset);
				wnd[nWndCnt].vArrAlgoParam = pAlgo;
				qwAlgoCommonOffset += sizeof(InspAlgo) *wnd[nWndCnt].nAlgorithmCnt;
				for (int nAlgoCnt = 0; nAlgoCnt < wnd[nWndCnt].nAlgorithmCnt; nAlgoCnt++)
				{
					//InspAlgo * pAlgo = (InspAlgo*)wnd[nWndCnt].vArrAlgoParam = (InspAlgo*)_stream.Byte(qwAlgoCommonOffset);
					
					int nSize = SetAlgoData(pAlgo[nAlgoCnt].m_eAlgoType, pAlgo[nAlgoCnt].m_ptrInspAlgoParam, qwAlgoDataOffset);
					qwAlgoDataOffset += nSize;

					nSize = SetArrMaskingROIData(pAlgo, qwAlgoMaskOffset);
					qwAlgoMaskOffset += nSize;
				}
			}
		}
		qwGetOffset = stHeader->m_qwOffSet[1];
		QWORD qwAlgoCommonRstOffset = stHeader->m_qwOffSet[3];
		QWORD qwAlgoDataRstOffset = stHeader->m_qwOffSet[5];
		for (int nWndType = 0; nWndType < eINSP_Total; nWndType++)
		{
			InspWndResult *  wnd = (InspWndResult *)SetWndTempRst(nWndType, PartData, qwGetOffset, false);
			qwGetOffset += sizeof(InspWndResult) * PartData->GetWndArraySize(nWndType);
			for (int nWndCnt = 0; nWndCnt < PartData->GetWndArraySize(nWndType); nWndCnt++)
			{
				if (wnd[nWndCnt].m_nAlgorithmCnt <= 0)
					continue;
				InspAlgoResult * pAlgoRst = (InspAlgoResult*)wnd[nWndCnt].m_vArrRstInspAlgo = (InspAlgoResult *)_stream.Byte(qwAlgoCommonRstOffset);
				qwAlgoCommonRstOffset += sizeof(InspAlgoResult) * wnd[nWndCnt].m_nAlgorithmCnt;

				for (int nAlgoCnt = 0; nAlgoCnt < wnd[nWndCnt].m_nAlgorithmCnt; nAlgoCnt++)
				{
					int nSize = SetAlgoRstData(pAlgoRst[nAlgoCnt].m_nAlgoType, pAlgoRst[nAlgoCnt].m_vRstInspAlgo, qwAlgoDataRstOffset);
					qwAlgoDataRstOffset += nSize;
				}
			}
		}
		nExtDefectCode = stHeader->m_qwOffSet[9]; // DefectCode
		
		return nExtDefectCode;
	}
	LPVOID* _PartRst::SetWndTempRst(int nType , InspectionResult * lpDst, QWORD qwOffSet , bool bTemp )
	{
		int stSize = 0;
		LPVOID* lpSource = (LPVOID*)_stream.Byte(qwOffSet);
		switch (nType)
		{
		case eINSP_MOUNT:
			if (bTemp) lpDst->mountParamTemp = (InspParamTemp*)lpSource; 
			else lpDst->mountResult = (InspWndResult *)lpSource;
			break;
		case eINSP_ALIGN:
			if (bTemp) lpDst->alignParamTemp = (InspParamTemp*)lpSource;
			else lpDst->alignResult = (InspWndResult *)lpSource;
			break;
		case eINSP_OCR:
			if (bTemp) lpDst->ocrParamTemp = (InspParamTemp*)lpSource;
			else lpDst->ocrResult = (InspWndResult *)lpSource;
			break;
		case eINSP_LEADSOLDER:
			if (bTemp) lpDst->leadSolderParamTemp = (InspParamTemp*)lpSource;
			else lpDst->leadSolderResult = (InspWndResult *)lpSource;
			break;
		case eINSP_SOLDER:
			if (bTemp) lpDst->solderParamTemp = (InspParamTemp*)lpSource;
			else lpDst->solderResult = (InspWndResult *)lpSource;
		case eINSP_TAB:
			if (bTemp) lpDst->tabParamTemp = (InspParamTemp*)lpSource;
			else lpDst->tabResult = (InspWndResult *)lpSource;
			break;
		case eINSP_S_BALL:
			if (bTemp) lpDst->S_BallParamTemp = (InspParamTemp*)lpSource;
			else lpDst->S_BallResult = (InspWndResult *)lpSource;
			break;
		case eINSP_PAD:
			if (bTemp) lpDst->PadParamTemp = (InspParamTemp*)lpSource;
			else lpDst->PadResult = (InspWndResult *)lpSource;
			break;
		case eINSP_BGA:
			if (bTemp) lpDst->BGAParamTemp = (InspParamTemp*)lpSource;
			else lpDst->BGAResult = (InspWndResult *)lpSource;
			break;
		default:
			break;
		}
		return lpSource;
	}
	int _PartRst::SetAlgoData(InspAlgoType eAlgoType, LPVOID& lpDst, QWORD qwOffSet)
	{
		int stSize = 0;
		LPVOID* lpSource = (LPVOID*)_stream.Byte(qwOffSet);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			lpDst = (AlgoAlign*)lpSource;
			stSize = sizeof(AlgoAlign);
			break;
		case eAlgoBW:
			lpDst = (AlgoBW*)lpSource;
			stSize = sizeof(AlgoBW);
			break;
		case eAlgoBlob:
			lpDst = (AlgoBlob*)lpSource;
			stSize = sizeof(AlgoBlob);
			break;
		case eAlgoBody_Blob:
			lpDst = (AlgoBodyBlob*)lpSource;
			stSize = sizeof(AlgoBodyBlob);
			break;
		case eAlgoTilt:
			lpDst = (AlgoTilt*)lpSource;
			stSize = sizeof(AlgoTilt);
			break;
		case eAlgoOCR:
			lpDst = (AlgoOCR*)lpSource;
			stSize = sizeof(AlgoOCR);
			break;
		case eAlgoPattern:
			lpDst = (AlgoPattern*)lpSource;
			stSize = sizeof(AlgoPattern);
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			lpDst = (AlgoColor*)lpSource;
			stSize = sizeof(AlgoColor);
			break;
		case eAlgoGray_Mean:
			lpDst = (AlgoGrayMean*)lpSource;
			stSize = sizeof(AlgoGrayMean);
			break;
		case eAlgoHeight_Mean:
			lpDst = (AlgoHeightMean*)lpSource;
			stSize = sizeof(AlgoHeightMean);
			break;
		case eAlgoGray_Diff:
			lpDst = (AlgoGrayDiff*)lpSource;
			stSize = sizeof(AlgoGrayDiff);
			break;
		case eAlgoHeight_Diff:
			lpDst = (AlgoHeightDiff*)lpSource;
			stSize = sizeof(AlgoHeightDiff);
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			lpDst = (AlgoLeadSearch*)lpSource;
			stSize = sizeof(AlgoLeadSearch);
			break;
		case eAlgoBridge:
			lpDst = (AlgoBridge*)lpSource;
			stSize = sizeof(AlgoBridge);
			break;
		case eAlgoLead_Tip:
			lpDst = (AlgoLeadTip*)lpSource;
			stSize = sizeof(AlgoLeadTip);
			break;
		case eAlgoLead_Lift:
			lpDst = (AlgoLeadLift*)lpSource;
			stSize = sizeof(AlgoLeadLift);
			break;
		case eAlgoLead_Solder:
			lpDst = (AlgoLeadSolder*)lpSource;
			stSize = sizeof(AlgoLeadSolder);
			break;
		case eAlgoLead_SideSolder:
			lpDst = (AlgoLeadSideSolder*)lpSource;
			stSize = sizeof(AlgoLeadSideSolder);
			break;
		case eAlgoWidth:
			lpDst = (AlgoLength*)lpSource;
			stSize = sizeof(AlgoLength);
			break;
		case eAlgoVolume:
			lpDst = (AlgoVolume*)lpSource;
			stSize = sizeof(AlgoVolume);
			break;
		case eAlgoTab:
			lpDst = (AlgoTab*)lpSource;
			stSize = sizeof(AlgoTab);
			break;
		case eAlgoGrid:
			lpDst = (AlgoGrid*)lpSource;
			stSize = sizeof(AlgoGrid);
			break;
		case eAlgoLine:
			lpDst = (AlgoLine*)lpSource;
			stSize = sizeof(AlgoLine);
			break;
		case eAlgoEdge:
			lpDst = (AlgoEdge*)lpSource;
			stSize = sizeof(AlgoEdge);
			break;
		case eAlgoSolderCone:
			lpDst = (AlgoSolderCone*)lpSource;
			stSize = sizeof(AlgoSolderCone);
			break;
		case eAlgoColorXY:
			lpDst = (AlgoColorXY*)lpSource;
			stSize = sizeof(AlgoColorXY);
			break;
		case eAlgoAlignEdge:
			lpDst = (AlgoAlignEdge*)lpSource;
			stSize = sizeof(AlgoAlignEdge);
			break;
		case eAlgoPadAlign:
			lpDst = (AlgoPadAlign*)lpSource;
			stSize = sizeof(AlgoPadAlign);
			break;
		case eAlgoPOCR:
			lpDst = (AlgoPOCR*)lpSource;
			stSize = sizeof(AlgoPOCR);
			break;
		case eAlgoWire:
			lpDst = (AlgoWire*)lpSource;
			stSize = sizeof(AlgoWire);
			break;
		case eAlgoFoot:
			lpDst = (AlgoFoot*)lpSource;
			stSize = sizeof(AlgoFoot);
			break;
		case eAlgoBarcode:
			lpDst = (AlgoBarcode*)lpSource;
			stSize = sizeof(AlgoBarcode);
			break;
		case eAlgoFillet:
			lpDst = (AlgoFillet*)lpSource;
			stSize = sizeof(AlgoFillet);
			break;
		case eAlgoBGA:
			lpDst = (AlgoBGA*)lpSource;
			stSize = sizeof(AlgoBGA);
			break;
		case eAlgoBump:
			lpDst = (AlgoBump*)lpSource;
			stSize = sizeof(AlgoBump);
			break;
		case eAlgoNGBlob:
			lpDst = (AlgoNGBlob*)lpSource;
			stSize = sizeof(AlgoNGBlob);
			break;
		case eAlgoPadBW:
			lpDst = (AlgoPadBW*)lpSource;
			stSize = sizeof(AlgoPadBW);
			break;
		case eAlgoBodyEdge:
			lpDst = (AlgoBodyEdge*)lpSource;
			stSize = sizeof(AlgoBodyEdge);
			break;
		case eAlgoDistance:
			lpDst = (AlgoDistance*)lpSource;
			stSize = sizeof(AlgoDistance);
			break;
		case eAlgoPatternDiff:
			lpDst = (AlgoPatternDiff*)lpSource;
			stSize = sizeof(AlgoPatternDiff);
			break;
		case eAlgoShapeX:
			lpDst = (AlgoShapeX*)lpSource;
			stSize = sizeof(AlgoShapeX);
			break;
		default:
			break;
		}
		return stSize;
	}
	int _PartRst::SetArrMaskingROIData(InspAlgo * pAlgo, QWORD qwOffSet)
	{
		int stSize = 0;
		LPVOID* lpSource = (LPVOID*)_stream.Byte(qwOffSet);
		pAlgo->m_rcArrMaskingROI = (RECT*)lpSource;

		stSize = sizeof(RECT)*pAlgo->m_nUsedMaskingValue;
		return stSize;
	}
	int _PartRst::SetAlgoRstData(InspAlgoType eAlgoType, LPVOID& lpDst, QWORD qwOffSet)
	{
		int stSize = 0;
		LPVOID* lpSource = (LPVOID*)_stream.Byte(qwOffSet);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			lpDst = (RstAlgoAlign*)lpSource;
			stSize = sizeof(RstAlgoAlign);
			break;
		case eAlgoBW:
			lpDst = (RstAlgoBlackWhite*)lpSource;
			stSize = sizeof(RstAlgoBlackWhite);
			break;
		case eAlgoBlob:
			lpDst = (RstAlgoBlob*)lpSource;
			stSize = sizeof(RstAlgoBlob);
			break;
		case eAlgoBody_Blob:
			lpDst = (RstAlgoBodyBlob*)lpSource;
			stSize = sizeof(RstAlgoBodyBlob);
			break;
		case eAlgoTilt:
			lpDst = (RstAlgoTilt*)lpSource;
			stSize = sizeof(RstAlgoTilt);
			break;
		case eAlgoOCR:
			lpDst = (RstAlgoOCR*)lpSource;
			stSize = sizeof(RstAlgoOCR);
			break;
		case eAlgoPattern:
			lpDst = (RstAlgoPattern*)lpSource;
			stSize = sizeof(RstAlgoPattern);
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			lpDst = (RstAlgoColor*)lpSource;
			stSize = sizeof(RstAlgoColor);
			break;
		case eAlgoGray_Mean:
			lpDst = (RstAlgoGrayMean*)lpSource;
			stSize = sizeof(RstAlgoGrayMean);
			break;
		case eAlgoHeight_Mean:
			lpDst = (RstAlgoHeightMean*)lpSource;
			stSize = sizeof(RstAlgoHeightMean);
			break;
		case eAlgoGray_Diff:
			lpDst = (RstAlgoGrayDiff*)lpSource;
			stSize = sizeof(RstAlgoGrayDiff);
			break;
		case eAlgoHeight_Diff:
			lpDst = (RstAlgoHeightDiff*)lpSource;
			stSize = sizeof(RstAlgoHeightDiff);
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			lpDst = (RstAlgoLeadSearch*)lpSource;
			stSize = 0;
			break;
		case eAlgoBridge:
			lpDst = (RstAlgoBridge*)lpSource;
			stSize = sizeof(RstAlgoBridge);
			break;
		case eAlgoLead_Tip:
			lpDst = (RstAlgoLeadTip*)lpSource;
			stSize = sizeof(RstAlgoLeadTip);
			break;
		case eAlgoLead_Lift:
			lpDst = (RstAlgoLeadLift*)lpSource;
			stSize = sizeof(RstAlgoLeadLift);
			break;
		case eAlgoLead_Solder:
			lpDst = (RstAlgoLeadSolder*)lpSource;
			stSize = sizeof(RstAlgoLeadSolder);
			break;
		case eAlgoLead_SideSolder:
			lpDst = (RstAlgoLeadSideSolder*)lpSource;
			stSize = sizeof(RstAlgoLeadSideSolder);
			break;
		case eAlgoWidth:
			lpDst = (RstAlgoLength*)lpSource;
			stSize = sizeof(RstAlgoLength);
			break;
		case eAlgoVolume:
			lpDst = (RstAlgoVolume*)lpSource;
			stSize = sizeof(RstAlgoVolume);
			break;
		case eAlgoTab:
			lpDst = (RstAlgoTab*)lpSource;
			stSize = sizeof(RstAlgoTab);
			break;
		case eAlgoGrid:
			lpDst = (RstAlgoGrid*)lpSource;
			stSize = sizeof(RstAlgoGrid);
			break;
		case eAlgoLine:
			lpDst = (RstAlgoLine*)lpSource;
			stSize = sizeof(RstAlgoLine);
			break;
		case eAlgoEdge:
			lpDst = (RstAlgoEdge*)lpSource;
			stSize = sizeof(RstAlgoEdge);
			break;
		case eAlgoSolderCone:
			lpDst = (RstAlgoSolderCone*)lpSource;
			stSize = sizeof(RstAlgoSolderCone);
			break;
		case eAlgoColorXY:
			lpDst = (RstAlgoColorXY*)lpSource;
			stSize = sizeof(RstAlgoColorXY);
			break;
		case eAlgoAlignEdge:
			lpDst = (RstAlgoAlignEdge*)lpSource;
			stSize = sizeof(RstAlgoAlignEdge);
			break;
		case eAlgoPadAlign:
			lpDst = (RstAlgoPadAlign*)lpSource;
			stSize = sizeof(RstAlgoPadAlign);
			break;
		case eAlgoPOCR:
			lpDst = (RstAlgoPOCR*)lpSource;
			stSize = sizeof(RstAlgoPOCR);
			break;
		case eAlgoWire:
			lpDst = (RstAlgoWire*)lpSource;
			stSize = sizeof(RstAlgoWire);
			break;
		case eAlgoFoot:
			lpDst = (RstAlgoFoot*)lpSource;
			stSize = sizeof(RstAlgoFoot);
			break;
		case eAlgoBarcode:
			lpDst = (RstAlgoBarcode*)lpSource;
			stSize = sizeof(RstAlgoBarcode);
			break;
		case eAlgoFillet:
			lpDst = (RstAlgoFillet*)lpSource;
			stSize = sizeof(RstAlgoFillet);
			break;
		case eAlgoBGA:
			lpDst = (RstAlgoBGA*)lpSource;
			stSize = sizeof(RstAlgoBGA);
			break;
		case eAlgoBump:
			lpDst = (RstAlgoBump*)lpSource;
			stSize = sizeof(RstAlgoBump);
			break;
		case eAlgoNGBlob:
			lpDst = (RstAlgoNGBlob*)lpSource;
			stSize = sizeof(RstAlgoNGBlob);
			break;
		case eAlgoPadBW:
			lpDst = (RstAlgoPadBW*)lpSource;
			stSize = sizeof(RstAlgoPadBW);
			break;
		case eAlgoBodyEdge:
			lpDst = (RstAlgoBodyEdge*)lpSource;
			stSize = sizeof(RstAlgoBodyEdge);
			break;
		case eAlgoDistance:
			lpDst = (RstAlgoDistance*)lpSource;
			stSize = sizeof(RstAlgoDistance);
			break;
		case eAlgoPatternDiff:
			lpDst = (RstAlgoPatternDiff*)lpSource;
			stSize = sizeof(RstAlgoPatternDiff);
			break;
		case eAlgoShapeX:
			lpDst = (RstAlgoShapeX*)lpSource;
			stSize = sizeof(RstAlgoShapeX);
			break;
		default:
			break;
		}
		return stSize;
	}
	int _PartRst::AddPartResult(InspectionResult * PartData)
	{
		bool bAddSuccess = true;
		_qwCurPos = 0;
		BYTE * ptr = _stream.Byte(_qwCurPos);
		tagSharedPartHeader tgheader;
		tgheader.m_nZerobyte = 0;
		memcpy(ptr + _qwCurPos, &tgheader, sizeof(tagSharedPartHeader));
		_qwCurPos += sizeof(tagSharedPartHeader);
		//Part
		memcpy(ptr + _qwCurPos, PartData, sizeof(InspectionResult));
		_qwCurPos += sizeof(InspectionResult);

		_stream.Flush(ptr, _qwCurPos);

		m_qwArrOffset[0] = _qwCurPos;

		return bAddSuccess;
	}
	int _PartRst::AddWndResult(InspWndResult * pWndRst , int nWndCnt)
	{
		bool bAddSuccess = true;
		if (nWndCnt == 0)
			return bAddSuccess;
		BYTE * ptr = _stream.Byte(_qwCurPos);
		/*QWORD qwWireStartPos = _qwCurPos;*/
		//Window
		memcpy(ptr, pWndRst, sizeof(InspWndResult)*nWndCnt);
		_stream.Flush(ptr, sizeof(InspWndResult)*nWndCnt);
		_qwCurPos += sizeof(InspWndResult)*nWndCnt;
		m_qwArrOffset[2] = _qwCurPos;

		return bAddSuccess;
	}

	int _PartRst::AddCommonAlgoResult(InspWndResult * pWndRst, int nWndCnt)
	{
		bool bAddSuccess = true;
		int nTempIndex = 0;
		int nTotalAlgoCnt = 0;
		BYTE * ptr = _stream.Byte(_qwCurPos);

		for (int i = 0; i < nWndCnt; i++)
		{
			InspWndResult * pWndParam = &pWndRst[i];
			for (int j = 0; j < pWndParam->m_nAlgorithmCnt; j++)
			{
				InspAlgoResult * pAlgo = &pWndParam->m_vArrRstInspAlgo[j];
				memcpy(ptr + nTempIndex, pAlgo, sizeof(InspAlgoResult));
				nTempIndex += sizeof(InspAlgoResult);
				_qwCurPos += sizeof(InspAlgoResult);
				nTotalAlgoCnt++;
			}
		}
		_stream.Flush(ptr, nTotalAlgoCnt * sizeof(InspAlgoResult));
		m_qwArrOffset[4] = _qwCurPos;
		return bAddSuccess;
	}
	int _PartRst::AddAlgoResult(InspWndResult * pWndRst, int nWndCnt)
	{
		bool bAddSuccess = true;

		
		int nZero = 0; 
		for (int i = 0; i < nWndCnt; i++)
		{
			InspWndResult * pWndParam = &pWndRst[i];
			for (int j = 0; j < pWndParam->m_nAlgorithmCnt; j++)
			{
				void * ptr = _stream.Byte(_qwCurPos);
				InspAlgoResult * pAlgo = &pWndParam->m_vArrRstInspAlgo[j];
				int nSize = AlgoCloneRst(pAlgo->m_nAlgoType, pAlgo->m_vRstInspAlgo, ptr);
				_qwCurPos += nSize;
				_stream.Flush((BYTE *)ptr , nSize);
			}
		}
		m_qwArrOffset[6] = _qwCurPos;

		return bAddSuccess;
	}
	int _PartRst::AlgoCloneRst(InspAlgoType eAlgoType, LPVOID lpSource, LPVOID& lpTarget)
	{
		int stSize(0);

		switch (eAlgoType)
		{
		case eAlgoAlign:
			memcpy_s(lpTarget, sizeof(RstAlgoAlign), lpSource, stSize = sizeof(RstAlgoAlign));
			break;
		case eAlgoBW:
			memcpy_s(lpTarget, sizeof(RstAlgoBlackWhite), lpSource, stSize = sizeof(RstAlgoBlackWhite));
			break;
		case eAlgoBlob:
			memcpy_s(lpTarget, sizeof(RstAlgoBlob), lpSource, stSize = sizeof(RstAlgoBlob));
			break;
		case eAlgoBody_Blob:
			memcpy_s(lpTarget, sizeof(RstAlgoBodyBlob), lpSource, stSize = sizeof(RstAlgoBodyBlob));
			break;
		case eAlgoTilt:
			memcpy_s(lpTarget, sizeof(RstAlgoTilt), lpSource, stSize = sizeof(RstAlgoTilt));
			break;
		case eAlgoOCR:
			memcpy_s(lpTarget, sizeof(RstAlgoOCR), lpSource, stSize = sizeof(RstAlgoOCR));
			break;
		case eAlgoPattern:
			memcpy_s(lpTarget, sizeof(RstAlgoPattern), lpSource, stSize = sizeof(RstAlgoPattern));
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			memcpy_s(lpTarget, sizeof(RstAlgoColor), lpSource, stSize = sizeof(RstAlgoColor));
			break;
		case eAlgoGray_Mean:
			memcpy_s(lpTarget, sizeof(RstAlgoGrayMean), lpSource, stSize = sizeof(RstAlgoGrayMean));
			break;
		case eAlgoHeight_Mean:
			memcpy_s(lpTarget, sizeof(RstAlgoHeightMean), lpSource, stSize = sizeof(RstAlgoHeightMean));
			break;
		case eAlgoGray_Diff:
			memcpy_s(lpTarget, sizeof(RstAlgoGrayDiff), lpSource, stSize = sizeof(RstAlgoGrayDiff));
			break;
		case eAlgoHeight_Diff:
			memcpy_s(lpTarget, sizeof(RstAlgoHeightDiff), lpSource, stSize = sizeof(RstAlgoHeightDiff));
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			memcpy_s(lpTarget, sizeof(RstAlgoLeadSearch), lpSource, stSize = 0);
			break;
		case eAlgoBridge:
			memcpy_s(lpTarget, sizeof(RstAlgoBridge), lpSource, stSize = sizeof(RstAlgoBridge));
			break;
		case eAlgoLead_Tip:
			memcpy_s(lpTarget, sizeof(RstAlgoLeadTip), lpSource, stSize = sizeof(RstAlgoLeadTip));
			break;
		case eAlgoLead_Lift:
			memcpy_s(lpTarget, sizeof(RstAlgoLeadLift), lpSource, stSize = sizeof(RstAlgoLeadLift));
			break;
		case eAlgoLead_Solder:
			memcpy_s(lpTarget, sizeof(RstAlgoLeadSolder), lpSource, stSize = sizeof(RstAlgoLeadSolder));
			break;
		case eAlgoLead_SideSolder:
			memcpy_s(lpTarget, sizeof(RstAlgoLeadSideSolder), lpSource, stSize = sizeof(RstAlgoLeadSideSolder));
			break;
		case eAlgoWidth:
			memcpy_s(lpTarget, sizeof(RstAlgoLength), lpSource, stSize = sizeof(RstAlgoLength));
			break;
		case eAlgoVolume:
			memcpy_s(lpTarget, sizeof(RstAlgoVolume), lpSource, stSize = sizeof(RstAlgoVolume));
			break;
		case eAlgoTab:
			memcpy_s(lpTarget, sizeof(RstAlgoTab), lpSource, stSize = sizeof(RstAlgoTab));
			break;
		case eAlgoGrid:
			memcpy_s(lpTarget, sizeof(RstAlgoGrid), lpSource, stSize = sizeof(RstAlgoGrid));
			break;
		case eAlgoLine:
			memcpy_s(lpTarget, sizeof(RstAlgoLine), lpSource, stSize = sizeof(RstAlgoLine));
			break;
		case eAlgoEdge:
			memcpy_s(lpTarget, sizeof(RstAlgoEdge), lpSource, stSize = sizeof(RstAlgoEdge));
			break;
		case eAlgoSolderCone:
			memcpy_s(lpTarget, sizeof(RstAlgoSolderCone), lpSource, stSize = sizeof(RstAlgoSolderCone));
			break;
		case eAlgoColorXY:
			memcpy_s(lpTarget, sizeof(RstAlgoColorXY), lpSource, stSize = sizeof(RstAlgoColorXY));
			break;
		case eAlgoAlignEdge:
			memcpy_s(lpTarget, sizeof(RstAlgoAlignEdge), lpSource, stSize = sizeof(RstAlgoAlignEdge));
			break;
		case eAlgoPadAlign:
			memcpy_s(lpTarget, sizeof(RstAlgoPadAlign), lpSource, stSize = sizeof(RstAlgoPadAlign));
			break;
		case eAlgoPOCR:
			memcpy_s(lpTarget, sizeof(RstAlgoPOCR), lpSource, stSize = sizeof(RstAlgoPOCR));
			break;
		case eAlgoWire:
			memcpy_s(lpTarget, sizeof(RstAlgoWire), lpSource, stSize = sizeof(RstAlgoWire));
			break;
		case eAlgoFoot:
			memcpy_s(lpTarget, sizeof(RstAlgoFoot), lpSource, stSize = sizeof(RstAlgoFoot));
			break;
		case eAlgoBarcode:
			memcpy_s(lpTarget, sizeof(RstAlgoBarcode), lpSource, stSize = sizeof(RstAlgoBarcode));
			break;
		case eAlgoFillet:
			memcpy_s(lpTarget, sizeof(RstAlgoFillet), lpSource, stSize = sizeof(RstAlgoFillet));
			break;
		case eAlgoBGA:
			memcpy_s(lpTarget, sizeof(RstAlgoBGA), lpSource, stSize = sizeof(RstAlgoBGA));
			break;
		case eAlgoBump:
			memcpy_s(lpTarget, sizeof(RstAlgoBump), lpSource, stSize = sizeof(RstAlgoBump));
			break;
		case eAlgoNGBlob:
			memcpy_s(lpTarget, sizeof(RstAlgoNGBlob), lpSource, stSize = sizeof(RstAlgoNGBlob));
			break;
		case eAlgoPadBW:
			memcpy_s(lpTarget, sizeof(RstAlgoPadBW), lpSource, stSize = sizeof(RstAlgoPadBW));
			break;
		case eAlgoBodyEdge:
			memcpy_s(lpTarget, sizeof(RstAlgoBodyEdge), lpSource, stSize = sizeof(RstAlgoBodyEdge));
			break;
		case eAlgoDistance:
			memcpy_s(lpTarget, sizeof(RstAlgoDistance), lpSource, stSize = sizeof(RstAlgoDistance));
			break;
		case eAlgoPatternDiff:
			memcpy_s(lpTarget, sizeof(RstAlgoPatternDiff), lpSource, stSize = sizeof(RstAlgoPatternDiff));
			break;
		case eAlgoShapeX:
			memcpy_s(lpTarget, sizeof(RstAlgoShapeX), lpSource, stSize = sizeof(RstAlgoShapeX));
			break;
			//shw Delete Xcase eAlgoNewAlgo1:#@lpTarget = g_pMManager->pem_new<AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);#@memcpy_s(lpTarget, sizeof(AlgoNewAlgo1), lpSource, sizeof(AlgoNewAlgo1));#@break;
		default:
			break;
		}
		return stSize;
	}
	int _PartRst::AlgoCloneTemp(InspAlgoType eAlgoType, LPVOID lpSource, LPVOID& lpTarget)
	{
		int stSize(0);
		switch (eAlgoType)
		{
		case eAlgoAlign:
			memcpy_s(lpTarget, sizeof(AlgoAlign), lpSource, stSize = sizeof(AlgoAlign));
			break;
		case eAlgoBW:
			memcpy_s(lpTarget, sizeof(AlgoBW), lpSource, stSize = sizeof(AlgoBW));
			break;
		case eAlgoBlob:
			memcpy_s(lpTarget, sizeof(AlgoBlob), lpSource, stSize = sizeof(AlgoBlob));
			break;
		case eAlgoBody_Blob:
			memcpy_s(lpTarget, sizeof(AlgoBodyBlob), lpSource, stSize = sizeof(AlgoBodyBlob));
			break;
		case eAlgoTilt:
			memcpy_s(lpTarget, sizeof(AlgoTilt), lpSource, stSize = sizeof(AlgoTilt));
			break;
		case eAlgoOCR:
			memcpy_s(lpTarget, sizeof(AlgoOCR), lpSource, stSize = sizeof(AlgoOCR));
			break;
		case eAlgoPattern:
			memcpy_s(lpTarget, sizeof(AlgoPattern), lpSource, stSize = sizeof(AlgoPattern));
			break;
		case eAlgoColor:
		case eAlgoLead_Color:
			memcpy_s(lpTarget, sizeof(AlgoColor), lpSource, stSize = sizeof(AlgoColor));
			break;
		case eAlgoGray_Mean:
			memcpy_s(lpTarget, sizeof(AlgoGrayMean), lpSource, stSize = sizeof(AlgoGrayMean));
			break;
		case eAlgoHeight_Mean:
			memcpy_s(lpTarget, sizeof(AlgoHeightMean), lpSource, stSize = sizeof(AlgoHeightMean));
			break;
		case eAlgoGray_Diff:
			memcpy_s(lpTarget, sizeof(AlgoGrayDiff), lpSource, stSize = sizeof(AlgoGrayDiff));
			break;
		case eAlgoHeight_Diff:
			memcpy_s(lpTarget, sizeof(AlgoHeightDiff), lpSource, stSize = sizeof(AlgoHeightDiff));
			break;
		case eAlgoLead_Search:
		case eAlgoTab_Search:
			memcpy_s(lpTarget, sizeof(AlgoLeadSearch), lpSource, stSize = sizeof(AlgoLeadSearch));
			break;
		case eAlgoBridge:
			memcpy_s(lpTarget, sizeof(AlgoBridge), lpSource, stSize = sizeof(AlgoBridge));
			break;
		case eAlgoLead_Tip:
			memcpy_s(lpTarget, sizeof(AlgoLeadTip), lpSource, stSize = sizeof(AlgoLeadTip));
			break;
		case eAlgoLead_Lift:
			memcpy_s(lpTarget, sizeof(AlgoLeadLift), lpSource, stSize = sizeof(AlgoLeadLift));
			break;
		case eAlgoLead_Solder:
			memcpy_s(lpTarget, sizeof(AlgoLeadSolder), lpSource, stSize = sizeof(AlgoLeadSolder));
			break;
		case eAlgoLead_SideSolder:
			memcpy_s(lpTarget, sizeof(AlgoLeadSideSolder), lpSource, stSize = sizeof(AlgoLeadSideSolder));
			break;
		case eAlgoWidth:
			memcpy_s(lpTarget, sizeof(AlgoLength), lpSource, stSize = sizeof(AlgoLength));
			break;
		case eAlgoVolume:
			memcpy_s(lpTarget, sizeof(AlgoVolume), lpSource, stSize = sizeof(AlgoVolume));
			break;
		case eAlgoTab:
			memcpy_s(lpTarget, sizeof(AlgoTab), lpSource, stSize = sizeof(AlgoTab));
			break;
		case eAlgoGrid:
			memcpy_s(lpTarget, sizeof(AlgoGrid), lpSource, stSize = sizeof(AlgoGrid));
			break;
		case eAlgoLine:
			memcpy_s(lpTarget, sizeof(AlgoLine), lpSource, stSize = sizeof(AlgoLine));
			break;
		case eAlgoEdge:
			memcpy_s(lpTarget, sizeof(AlgoEdge), lpSource, stSize = sizeof(AlgoEdge));
			break;
		case eAlgoSolderCone:
			memcpy_s(lpTarget, sizeof(AlgoSolderCone), lpSource, stSize = sizeof(AlgoSolderCone));
			break;
		case eAlgoColorXY:
			memcpy_s(lpTarget, sizeof(AlgoColorXY), lpSource, stSize = sizeof(AlgoColorXY));
			break;
		case eAlgoAlignEdge:
			memcpy_s(lpTarget, sizeof(AlgoAlignEdge), lpSource, stSize = sizeof(AlgoAlignEdge));
			break;
		case eAlgoPadAlign:
			memcpy_s(lpTarget, sizeof(AlgoPadAlign), lpSource, stSize = sizeof(AlgoPadAlign));
			break;
		case eAlgoPOCR:
			memcpy_s(lpTarget, sizeof(AlgoPOCR), lpSource, stSize = sizeof(AlgoPOCR));
			break;
		case eAlgoWire:
			memcpy_s(lpTarget, sizeof(AlgoWire), lpSource, stSize = sizeof(AlgoWire));
			break;
		case eAlgoFoot:
			memcpy_s(lpTarget, sizeof(AlgoFoot), lpSource, stSize = sizeof(AlgoFoot));
			break;
		case eAlgoBarcode:
			memcpy_s(lpTarget, sizeof(AlgoBarcode), lpSource, stSize = sizeof(AlgoBarcode));
			break;
		case eAlgoFillet:
			memcpy_s(lpTarget, sizeof(AlgoFillet), lpSource, stSize = sizeof(AlgoFillet));
			break;
		case eAlgoBGA:
			memcpy_s(lpTarget, sizeof(AlgoBGA), lpSource, stSize = sizeof(AlgoBGA));
			break;
		case eAlgoBump:
			memcpy_s(lpTarget, sizeof(AlgoBump), lpSource, stSize = sizeof(AlgoBump));
			break;
		case eAlgoNGBlob:
			memcpy_s(lpTarget, sizeof(AlgoNGBlob), lpSource, stSize = sizeof(AlgoNGBlob));
			break;
		case eAlgoPadBW:
			memcpy_s(lpTarget, sizeof(AlgoPadBW), lpSource, stSize = sizeof(AlgoPadBW));
			break;
		case eAlgoBodyEdge:
			memcpy_s(lpTarget, sizeof(AlgoBodyEdge), lpSource, stSize = sizeof(AlgoBodyEdge));
			break;
		case eAlgoDistance:
			memcpy_s(lpTarget, sizeof(AlgoDistance), lpSource, stSize = sizeof(AlgoDistance));
			break;
		case eAlgoPatternDiff:
			memcpy_s(lpTarget, sizeof(AlgoPatternDiff), lpSource, stSize = sizeof(AlgoPatternDiff));
			break;
		case eAlgoShapeX:
			memcpy_s(lpTarget, sizeof(AlgoShapeX), lpSource, stSize = sizeof(AlgoShapeX));
			break;
			//shw Delete Xcase eAlgoNewAlgo1:#@lpTarget = g_pMManager->pem_new<AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);#@memcpy_s(lpTarget, sizeof(AlgoNewAlgo1), lpSource, sizeof(AlgoNewAlgo1));#@break;
		default:
			break;
		}
		return stSize;

	}
	int _PartRst::AddPartTempResult(InspParamTemp * pParamTemp , int nWndCnt)
	{
		bool bAddSuccess = true;
		if (nWndCnt == 0)
			return bAddSuccess;
		BYTE * ptr = _stream.Byte(_qwCurPos);
		//Window
		memcpy(ptr , pParamTemp, sizeof(InspParamTemp)* nWndCnt);
		_qwCurPos += sizeof(InspParamTemp)* nWndCnt;
		_stream.Flush(ptr , sizeof(InspParamTemp)* nWndCnt);
		m_qwArrOffset[1] = _qwCurPos;

		return bAddSuccess;
	}
	int _PartRst::AddCommonAlgo(InspParamTemp * pParamTemp, int nWndCnt)
	{
		bool bAddSuccess = true;

		BYTE * ptr = _stream.Byte(_qwCurPos);
		int nTempIndex = 0;
		int nTotalAlgoCnt = 0;

		for (int i = 0; i < nWndCnt; i++)
		{
			InspParamTemp * pWndParam = &pParamTemp[i];
			for (int j = 0; j < pWndParam->nAlgorithmCnt; j++)
			{
				InspAlgo * pAlgo = &pWndParam->vArrAlgoParam[j];
				memcpy(ptr + nTempIndex, pAlgo, sizeof(InspAlgo));
				nTempIndex += sizeof(InspAlgo);
				_qwCurPos += sizeof(InspAlgo);
				nTotalAlgoCnt++;
			}
		}

		_stream.Flush(ptr, nTotalAlgoCnt * sizeof(InspAlgo));

		m_qwArrOffset[3] = _qwCurPos;

		return bAddSuccess;
	}
	int _PartRst::AddAlgo(InspParamTemp * pParamTemp, int nWndCnt)
	{
		bool bAddSuccess = true;

		
		QWORD qwTotalAlgoSize = 0;
		for (int i = 0; i < nWndCnt; i++)
		{
			InspParamTemp * pWndParam = &pParamTemp[i];
			for (int j = 0; j < pWndParam->nAlgorithmCnt; j++)
			{
				void * ptr = _stream.Byte(_qwCurPos);
				InspAlgo * pAlgo = &pWndParam->vArrAlgoParam[j];
				int nSize = AlgoCloneTemp(pAlgo->m_eAlgoType, pAlgo->m_ptrInspAlgoParam,ptr);
				_qwCurPos += nSize;
				_stream.Flush(ptr, nSize);
			}
		}

	

		m_qwArrOffset[5] = _qwCurPos;
		return bAddSuccess;
	}
	int _PartRst::AddArrMaskingROI(InspParamTemp * pParamTemp, int nWndCnt)
	{
		bool bAddSuccess = true;

		BYTE * ptr = _stream.Byte(_qwCurPos);
		int nTempIndex = 0;
		int nTotalMaskingROICnt = 0;
		for (int i = 0; i < nWndCnt; i++)
		{
			InspParamTemp * pWndParam = &pParamTemp[i];
			for (int j = 0; j < pWndParam->nAlgorithmCnt; j++)
			{
				InspAlgo * pAlgo = &pWndParam->vArrAlgoParam[j];
				RECT* arrMaskingROI = pAlgo->m_rcArrMaskingROI;
				memcpy(ptr + nTempIndex, arrMaskingROI, sizeof(RECT) * pAlgo->m_nUsedMaskingValue);
				nTempIndex += sizeof(RECT) * pAlgo->m_nUsedMaskingValue;
				_qwCurPos += sizeof(RECT) * pAlgo->m_nUsedMaskingValue;
				nTotalMaskingROICnt += pAlgo->m_nUsedMaskingValue;
			}
		}

		_stream.Flush(ptr, nTotalMaskingROICnt * sizeof(RECT));

		m_qwArrOffset[7] = _qwCurPos;

		return bAddSuccess;
	}
	int _PartRst::CopyHeader()
	{
		BYTE * ptr = _stream.Byte(0);
		((tagSharedPartHeader*)ptr)->m_nZerobyte = 0;
		memcpy(((tagSharedPartHeader*)ptr)->m_qwOffSet, m_qwArrOffset, sizeof(((tagSharedPartHeader*)ptr)->m_qwOffSet));
		_stream.Flush(ptr, sizeof(tagSharedPartHeader));
		return 1;
	}
	bool _PartRst::GetNext(InspectionResult *& rst, int nCalcDoneCnt, int * nExtDefectCode)
	{
		bool ret = false;
		if (_nRecvRstCount < nCalcDoneCnt)
		{
			*nExtDefectCode = GetResult(rst);
			_nRecvRstCount++;
			ret = true;
		}
		else
			ret = false;

		return ret;
	}

	LightParam::LightParam()
		:_qwCurPos(0)
	{
		_TotalBuff = 0;
	}
	bool LightParam::Init(int nToolID)
	{
		bool bInit = true;

		CString sName, sLog;
		sName.Format(_T("AOI_ExtLightParam_Tool_%d_Index%d"), nToolID, m_nBuffIdx);

		sLog.Format(_T("LightParam Buffer Index_%d - Start()"), m_nBuffIdx);
		ext::Log::add(sLog);

		Memory<ExtAlgoLight>::Init();

		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;
		_TotalBuff = env::nLightBufferSize;
		if (Create(env::nLightBufferSize, sName, nAccessType) == false)
			bInit = false;
		if (!bInit)
		{
			sLog.Format(_T("%s_Init_Fail"), sName);
			ext::Log::add(sLog);
		}
			
		sLog.Format(_T("LightParam Buffer Index_%d - Start()"), m_nBuffIdx);
		ext::Log::add(sLog);

		return bInit;
	}
	void LightParam::Exit()
	{
		Memory<ExtAlgoLight>::Exit();
	}
	void LightParam::AddLight(InspPartParam* pWnd, int nWnd)
	{
		SetPosZero();
		BYTE* ptr = _stream.Byte(_qwCurPos);
		for (int i = 0; i < nWnd; i++)
		{
			InspPartParam * pParam = &pWnd[i];
			for (int j = 0; j < pParam->nAlgorithmCnt; j++)
			{
				int nSize = Add(&pParam->vArrAlgoParam[j]);
				_qwCurPos += nSize;
			}
		}
		_stream.Flush(ptr, _qwCurPos);
	}
	int LightParam::Add(InspAlgo* pAlgo)
	{
		BYTE* ptr = _stream.Byte(_qwCurPos);
		ExtAlgoLight stAlgo;
		stAlgo.m_stLight.CloneLight(pAlgo);
		stAlgo.m_nMixCnt = pAlgo->m_nMixCount;

		if (stAlgo.m_nMixCnt > 0)
		{
			for (int i = 0; i < stAlgo.m_nMixCnt; i++)
				stAlgo.m_stMixLight[i].CloneMixLight(&pAlgo->InspAlgoLightsMix[i]);
		}

		memcpy(ptr, &stAlgo, sizeof(ExtAlgoLight));

		return sizeof(ExtAlgoLight);
	}
	void LightParam::GetLight(InspPartParam*& pWnd, int nWnd)
	{
		_qwCurPos = 0;
		for (int wnd = 0; wnd < nWnd; wnd++)
		{
			InspPartParam * pParam = &pWnd[wnd];
			for (int algo = 0; algo < pParam->nAlgorithmCnt; algo++)
			{
				InspAlgo * pAlgo = &pParam->vArrAlgoParam[algo];
				Get(pAlgo);
			}
		}
	}
	int LightParam::Get(InspAlgo* pAlgo)
	{
		BYTE* ptr = _stream.Byte(_qwCurPos);
		ExtAlgoLight* stAlgo;

		stAlgo = (ExtAlgoLight*)ptr;

		pAlgo->m_nArrRedValue		= stAlgo->m_stLight.m_nArrRedValue;
		pAlgo->m_nArrGreenValue		= stAlgo->m_stLight.m_nArrGreenValue;
		pAlgo->m_nArrBlueValue		= stAlgo->m_stLight.m_nArrBlueValue;
		pAlgo->m_nArrWhiteValue		= stAlgo->m_stLight.m_nArrWhiteValue;
		pAlgo->m_nArrCalculation	= stAlgo->m_stLight.m_nArrCalculation;
		pAlgo->m_nArrLightPosition	= stAlgo->m_stLight.m_nArrLightPosition;

		if (pAlgo->m_nMixCount > 0)
		{
			for (int i = 0; i < pAlgo->m_nMixCount; i++)
			{
				pAlgo->InspAlgoLightsMix[i].m_nImageNum= stAlgo->m_stMixLight[i].m_nImageNum;
				pAlgo->InspAlgoLightsMix[i].m_eLightType = stAlgo->m_stMixLight[i].m_eLightType;
				pAlgo->InspAlgoLightsMix[i].m_nRedValue = stAlgo->m_stMixLight[i].m_nRedValue;
				pAlgo->InspAlgoLightsMix[i].m_nGreenValue = stAlgo->m_stMixLight[i].m_nGreenValue;
				pAlgo->InspAlgoLightsMix[i].m_nBlueValue = stAlgo->m_stMixLight[i].m_nBlueValue;
				pAlgo->InspAlgoLightsMix[i].m_nWhiteValue = stAlgo->m_stMixLight[i].m_nWhiteValue;

				pAlgo->InspAlgoLightsMix[i].m_nLightCnt = stAlgo->m_stMixLight[i].m_nLightCnt;
				pAlgo->InspAlgoLightsMix[i].m_nArrRedValue = stAlgo->m_stMixLight[i].m_nArrRedValue;
				pAlgo->InspAlgoLightsMix[i].m_nArrGreenValue = stAlgo->m_stMixLight[i].m_nArrGreenValue;
				pAlgo->InspAlgoLightsMix[i].m_nArrBlueValue = stAlgo->m_stMixLight[i].m_nArrBlueValue;
				pAlgo->InspAlgoLightsMix[i].m_nArrWhiteValue = stAlgo->m_stMixLight[i].m_nArrWhiteValue;
				pAlgo->InspAlgoLightsMix[i].m_nArrCalculation= stAlgo->m_stMixLight[i].m_nArrCalculation;
				pAlgo->InspAlgoLightsMix[i].m_nArrLightPosition= stAlgo->m_stMixLight[i].m_nArrLightPosition;
			}
		}

		_qwCurPos += sizeof(ExtAlgoLight);

		return sizeof(ExtAlgoLight);
	}
// 
// 	StringMap::StringMap()
// 		:_qwCurPos(0)
// 	{
// 
// 	}
// 	bool StringMap::Init(int nToolID)
// 	{
// 		bool bInit = true;
// 
// 		CString sName, sLog;
// 		sName.Format(_T("AOI_ExtStringMap_Tool_%d_Index%d"), nToolID, m_nBuffIdx);
// 
// 		sLog.Format(_T("StringMap Index_%d - Start()"), m_nBuffIdx);
// 		ext::Log::add(sLog);
// 
// 		int nAccessType = FILE_MAP_ALL_ACCESS;
// 		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
// 			nAccessType = FILE_MAP_READ;
// 
// 		if (Create(env::nStringDataStructSize, sName, nAccessType) == false)
// 			bInit = false;
// 		if (!bInit)
// 		{
// 			sLog.Format(_T("%s_Init_Fail"), sName);
// 			ext::Log::add(sLog); 
// 		}
// 
// 		_SData.m_nBuffIdx = m_nBuffIdx;
// 		_SData.Init(nToolID);
// 
// 		sLog.Format(_T("StringMap Index_%d - Start()"), m_nBuffIdx);
// 		ext::Log::add(sLog);
// 
// 		return bInit;
// 	}
// 	bool StringMap::Create(QWORD sizebytes, CString sName, DWORD access)
// 	{
// 		bool bInit = true;
// 		if (_stream == nullptr)
// 		{
// 			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
// 		}
// 
// 		_sizebytes = sizebytes;
// 		if (_stream->Open(_sizebytes, sName, access) == false)
// 			_stream->AllocMemory(_sizebytes, sName);
// 
// 		if (_stream->MapView(0, _sizebytes, access) == false)
// 			bInit = false;
// 		return bInit;
// 	}
// 	void StringMap::AddStringPath(InspPartParam* pWnd, int nWindowCnt)
// 	{
// 		_qwCurPos = 0;
// 		BYTE* ptr = _stream->Byte(_qwCurPos);
// 
// 		_SData.Clear();
// 
// 
// 
// 		tagSharedModelMap stModel;
// 		int nTotalModelCnt = 0;
// 		int nNextModelStIdx = 0;
// 		int nSize = 0;
// 		for (int i = 0; i < nWindowCnt; i++)
// 		{
// 			InspPartParam * pParam = &pWnd[i];
// 			InspAlgo* pAlgo = pParam->vArrAlgoParam;
// 			for (int j = 0; j < pParam->nAlgorithmCnt; j++)
// 			{
// 				if (pAlgo[j].m_eAlgoType == eAlgoPattern)
// 				{
// 					AlgoPattern* pPattern = (AlgoPattern*)pAlgo[j].m_ptrInspAlgoParam;
// 
// 					//model
// 					for (int modelcnt = 0; modelcnt < pPattern->m_nCntPatternPath; modelcnt++)
// 					{
// 						//data 먼저 처리
// 						CString str = _T("");
// 						str.Format(_T("%s"), pPattern->m_sArrPathModelInspect[modelcnt]);
// 
// 						QWORD qwRet = _SData.AddData(str, nSize);
// 
// 						//stModel.vName[modelcnt + nTotalModelCnt] = (QWORD)((void*)&pPattern->m_sArrPathModelInspect[modelcnt]);
// 						stModel.vName[modelcnt + nTotalModelCnt] = (QWORD)&*pPattern->m_sArrPathModelInspect[modelcnt];
// 						stModel.vData[modelcnt + nTotalModelCnt] = qwRet;
// 						stModel.nStrLen[modelcnt + nTotalModelCnt] = nSize;
// 
// 						nNextModelStIdx++;
// 					}
// 					nTotalModelCnt += nNextModelStIdx;
// 
// 					//SimilarModel
// 					for (int modelcnt = 0; modelcnt < CNT_PATTERN_SIMILAR; modelcnt++)
// 					{
// 						if (pPattern->m_sSimilarPartList[modelcnt] != _T(""))
// 						{
// 							//data 먼저 처리
// 							CString str = _T("");
// 							str.Format(_T("%s"), pPattern->m_sSimilarPartList[modelcnt]);
// 
// 							QWORD qwRet = _SData.AddData(str, nSize);
// 
// 							stModel.vName[modelcnt + nTotalModelCnt] = (QWORD)&*pPattern->m_sSimilarPartList[modelcnt];
// 							stModel.vData[modelcnt + nTotalModelCnt] = qwRet;
// 							stModel.nStrLen[modelcnt + nTotalModelCnt] = nSize;
// 							nNextModelStIdx++;
// 						}
// 					}
// 					nTotalModelCnt += nNextModelStIdx;
// 				}
// 
// 				if (pAlgo[j].m_eAlgoType == eAlgoNGBlob)
// 				{
// 					AlgoNGBlob* pNGBlob = (AlgoNGBlob*)pAlgo[j].m_ptrInspAlgoParam;
// 
// 					/*if (pNGBlob->m_nCntPatternPath > 0)
// 					{
// 						for (int modelcnt = 0; modelcnt < pNGBlob->m_nCntPatternPath; modelcnt++)
// 						{
// 							CString str = _T("");
// 							str.Format(_T("%s"), pNGBlob->m_sArrPathExceptModel[modelcnt]);
// 
// 							QWORD qwRet = _SData.AddData(str, nSize);
// 
// 							stModel.vName[modelcnt + nTotalModelCnt] = (QWORD)&*pNGBlob->m_sArrPathExceptModel[modelcnt];
// 							stModel.vData[modelcnt + nTotalModelCnt] = qwRet;
// 							stModel.nStrLen[modelcnt + nTotalModelCnt] = nSize;
// 							nNextModelStIdx++;
// 				}
// 						nTotalModelCnt += nNextModelStIdx;
// 					}*/
// 
// 			}
// 
// 		}
// 		}
// 		if (nTotalModelCnt > 0)
// 		{
// 			memcpy(ptr, &stModel, sizeof(tagSharedModelMap));
// 			_qwCurPos += sizeof(tagSharedModelMap);
// 			_stream->Flush(ptr, _qwCurPos);
// 		}
// 
// 	}
// 	void StringMap::GetStringPath(InspPartParam* pWnd, int nWindowCnt)
// 	{
// 		QWORD _qwGetCurPos = 0;
// 		BYTE* ptr = _stream->Byte(_qwGetCurPos);
// 
// 		tagSharedModelMap* stModel = (tagSharedModelMap*)ptr;
// 		
// 		for (int i = 0; i < nWindowCnt; i++)
// 		{
// 			InspPartParam * pParam = &pWnd[i];
// 			InspAlgo* pAlgo = pParam->vArrAlgoParam;
// 			for (int j = 0; j < pParam->nAlgorithmCnt; j++)
// 			{
// 				if (pAlgo[j].m_eAlgoType == (int)eAlgoPattern)
// 				{
// 					AlgoPattern* pPattern = (AlgoPattern*)pAlgo[j].m_ptrInspAlgoParam;
// 
// 					for (int cnt = 0; cnt < pPattern->m_nCntPatternPath; cnt++)
// 					{
// 						QWORD qwName = (QWORD)&*pPattern->m_sArrPathModelInspect[cnt];
// 						
// 						wchar_t* chAddres = _SData.GetData(qwName, stModel);
// 						ext::ClientCtrl::get()->InsertExtModelPath(qwName, (CString)chAddres);
// 			}
// 
// 					for (int cnt = 0; cnt < CNT_PATTERN_SIMILAR; cnt++)
// 					{
// 
// 						QWORD qwName = (QWORD)&*pPattern->m_sSimilarPartList[cnt];
// 
// 						wchar_t* chAddres = _SData.GetData(qwName, stModel);
// 						if (chAddres != 0)
// 							ext::ClientCtrl::get()->InsertExtModelPath(qwName, (CString)chAddres);
// 						else
// 						{
// 							wchar_t* sEmpty = L"empty";
// 							ext::ClientCtrl::get()->InsertExtModelPath(qwName, (CString)sEmpty);
// 		}
// 					}
// 				}
// 
// 				/*if (pAlgo[j].m_eAlgoType == (int)eAlgoNGBlob)
// 				{
// 					AlgoNGBlob* pNGBlob = (AlgoNGBlob*)pAlgo[j].m_ptrInspAlgoParam;
// 					for (int cnt = 0; cnt < pNGBlob->m_nCntPatternPath; cnt++)
// 					{
// 						QWORD qwName = (QWORD)&*pNGBlob->m_sArrPathExceptModel[cnt];
// 
// 						wchar_t* chAddres = _SData.GetData(qwName, stModel);
// 						if (chAddres != 0)
// 							ext::ClientCtrl::get()->InsertExtModelPath(qwName, (CString)chAddres);
// 	}
// 				}*/
// 			}
// 		}
// 
// 	}
// 	void StringMap::Clear()
// 	{
// 	}
// 	void StringMap::Exit()
// 	{
// 
// 	}
// 
// 	StringData::StringData()
// 		:_qwCurPos(0)
// 	{
// 
// 	}
// 	bool StringData::Init(int nToolID)
// 	{
// 		bool bInit = true;
// 
// 		CString sName, sLog;
// 		sName.Format(_T("AOI_ExtStringData_Tool_%d_Index%d"), nToolID, m_nBuffIdx);
// 
// 		sLog.Format(_T("StringData Index_%d - Start()"), m_nBuffIdx);
// 		ext::Log::add(sLog);
// 
// 		int nAccessType = FILE_MAP_ALL_ACCESS;
// 		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
// 			nAccessType = FILE_MAP_READ;
// 
// 		if (Create(env::nStringDataStructSize, sName, nAccessType) == false)
// 			bInit = false;
// 		if (!bInit)
// 		{
// 			sLog.Format(_T("%s_Init_Fail"), sName);
// 			ext::Log::add(sLog);
// 		}
// 
// 		sLog.Format(_T("StringData Index_%d - Start()"), m_nBuffIdx);
// 		ext::Log::add(sLog);
// 
// 		return bInit;
// 	}
// 	bool StringData::Create(QWORD sizebytes, CString sName, DWORD access)
// 	{
// 		bool bInit = true;
// 		if (_stream == nullptr)
// 		{
// 			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
// 		}
// 
// 		_sizebytes = sizebytes;
// 		if (_stream->Open(_sizebytes, sName, access) == false)
// 			_stream->AllocMemory(_sizebytes, sName);
// 
// 		if (_stream->MapView(0, _sizebytes, access) == false)
// 			bInit = false;
// 		return bInit;
// 	}
// 	QWORD StringData::AddData(CString sStr, int& nSize)
// 	{
// 		QWORD qwStartPos = _qwCurPos;
// 
// 		BYTE* ptr = _stream->Byte(_qwCurPos);
// 
// 		int nLength = sStr.GetLength() + 1;
// 		nSize = nLength;
// 
// 		wchar_t* c_str = sStr.LockBuffer();
// 
// 		memcpy(ptr, sStr, sizeof(wchar_t) * (nLength));
// 		_qwCurPos += sizeof(wchar_t) * (nLength);
// 		sStr.UnlockBuffer();
// 
// 		return qwStartPos;
// 	}
// 
// 	wchar_t* StringData::GetData(QWORD qwName, tagSharedModelMap* stModel)
// 	{
// 		CStringW sRet = _T("");
// 
// 		BYTE* ptr = _stream->Byte(0);
// 		
// 		for (int i = 0; i < 300; i++)
// 		{
// 			if (qwName == stModel->vName[i])
// 			{
// 				QWORD qwDataPos = stModel->vData[i];
// 				wchar_t* chr = (wchar_t*)_stream->Byte(stModel->vData[i]);
// 				return chr;
// 			}
// 		}
// 		return 0;
// 	}
// 	void StringData::Clear()
// 	{
// 		int nTotalIdx = _qwCurPos / (sizeof(wchar_t) * 256);
// 		for (int i = 0; i < nTotalIdx; i++)
// 		{
// 			BYTE* ptr = _stream->Byte((sizeof(wchar_t) * 256) * i);
// 			for (int j = 0; j < 256; j++)
// 			{
// 				wchar_t* str =(wchar_t*)ptr[j];
// 				str = _T("");
// 			}
// 		}
// 
// 		_qwCurPos = 0;
// 	}
// 	void StringData::Exit()
// 	{
// 
// 	}

	void CompareStrurct::CompareInit(bool bServer)
	{
		std::vector<int> nCompareStrurct;
		std::vector<CString> sName;

		if (EnvVariable::nSystemType == EXT_INSP_CLIENT)
			memset(this->First()->nStructSizeArray, 0, sizeof(int) * 200);
		else
		{
			CQTimer qtm;
			qtm.StartTick();
			CString sLogRst;
			while (true)
			{
				if (this->First()->nStructFlag == 1)
				{
					sLogRst.Format(_T("[EXTStructCompare] Flag On - Wait Time %d ms ") , (int)(qtm.EndTick()* 1000.0));
					ext::Log::add(sLogRst);
					break;
				}
				else if ((qtm.EndTick() * 1000.0) > 5000) // 5000Ms 일경우 Exit
				{
					sLogRst.Format(_T("[EXTStructCompare] Time Out"));
					ext::Log::add(sLogRst);
					break;
				}
			}
			
		}
		for (int i = 0; i < (int)eAlgoNum; i++)
		{
			switch (i)
			{
			case eAlgoAlign:
				nCompareStrurct.push_back(sizeof(AlgoAlign));
				sName.push_back(_T("AlgoAlign"));
				nCompareStrurct.push_back(sizeof(RstAlgoAlign));
				sName.push_back(_T("RstAlgoAlign"));
				break;
			case eAlgoBW:
				nCompareStrurct.push_back(sizeof(AlgoBW));
				sName.push_back(_T("AlgoBW"));
				nCompareStrurct.push_back(sizeof(RstAlgoBlackWhite));
				sName.push_back(_T("RstAlgoBlackWhite"));
				break;
			case eAlgoBlob:
				nCompareStrurct.push_back(sizeof(AlgoBlob));
				sName.push_back(_T("AlgoBlob"));
				nCompareStrurct.push_back(sizeof(RstAlgoBlob));
				sName.push_back(_T("RstAlgoBlob"));
				break;
			case eAlgoBody_Blob:
				nCompareStrurct.push_back(sizeof(AlgoBodyBlob));
				sName.push_back(_T("AlgoBodyBlob"));
				nCompareStrurct.push_back(sizeof(RstAlgoBodyBlob));
				sName.push_back(_T("RstAlgoBodyBlob"));
				break;
			case eAlgoTilt:
				nCompareStrurct.push_back(sizeof(AlgoTilt));
				sName.push_back(_T("AlgoTilt"));
				nCompareStrurct.push_back(sizeof(RstAlgoTilt));
				sName.push_back(_T("RstAlgoTilt"));
				break;
			case eAlgoOCR:
				nCompareStrurct.push_back(sizeof(AlgoOCR));
				sName.push_back(_T("AlgoOCR"));
				nCompareStrurct.push_back(sizeof(RstAlgoOCR));
				sName.push_back(_T("RstAlgoOCR"));
				break;
			case eAlgoPattern:
				nCompareStrurct.push_back(sizeof(AlgoPattern));
				sName.push_back(_T("AlgoPattern"));
				nCompareStrurct.push_back(sizeof(RstAlgoPattern));
				sName.push_back(_T("RstAlgoPattern"));
				break;
			case eAlgoColor:
				nCompareStrurct.push_back(sizeof(AlgoColor));
				sName.push_back(_T("AlgoColor"));
				nCompareStrurct.push_back(sizeof(RstAlgoColor));
				sName.push_back(_T("RstAlgoColor"));
				break;
			case eAlgoLead_Color:
				nCompareStrurct.push_back(sizeof(AlgoColor));
				sName.push_back(_T("AlgoColor"));
				nCompareStrurct.push_back(sizeof(RstAlgoColor));
				sName.push_back(_T("RstAlgoColor"));
				break;
			case eAlgoGray_Mean:
				nCompareStrurct.push_back(sizeof(AlgoGrayMean));
				sName.push_back(_T("AlgoGrayMean"));
				nCompareStrurct.push_back(sizeof(RstAlgoGrayMean));
				sName.push_back(_T("RstAlgoGrayMean"));
				break;
			case eAlgoHeight_Mean:
				nCompareStrurct.push_back(sizeof(AlgoHeightMean));
				sName.push_back(_T("AlgoHeightMean"));
				nCompareStrurct.push_back(sizeof(RstAlgoHeightMean));
				sName.push_back(_T("RstAlgoHeightMean"));
				break;
			case eAlgoGray_Diff:
				nCompareStrurct.push_back(sizeof(AlgoGrayDiff));
				sName.push_back(_T("AlgoGrayDiff"));
				nCompareStrurct.push_back(sizeof(RstAlgoGrayDiff));
				sName.push_back(_T("RstAlgoGrayDiff"));
				break;
			case eAlgoHeight_Diff:
				nCompareStrurct.push_back(sizeof(AlgoHeightDiff));
				sName.push_back(_T("AlgoHeightDiff"));
				nCompareStrurct.push_back(sizeof(RstAlgoHeightDiff));
				sName.push_back(_T("RstAlgoHeightDiff"));
				break;
			case eAlgoLead_Search:
				nCompareStrurct.push_back(sizeof(AlgoLeadSearch));
				sName.push_back(_T("AlgoLeadSearch"));
				nCompareStrurct.push_back(sizeof(RstAlgoLeadSearch));
				sName.push_back(_T("RstAlgoLeadSearch"));
				break;
			case eAlgoTab_Search:
				nCompareStrurct.push_back(sizeof(AlgoLeadSearch));
				sName.push_back(_T("AlgoLeadSearch"));
				nCompareStrurct.push_back(sizeof(RstAlgoLeadSearch));
				sName.push_back(_T("RstAlgoLeadSearch"));
				break;
			case eAlgoBridge:
				nCompareStrurct.push_back(sizeof(AlgoBridge));
				sName.push_back(_T("AlgoBridge"));
				nCompareStrurct.push_back(sizeof(RstAlgoBridge));
				sName.push_back(_T("RstAlgoBridge"));
				break;
			case eAlgoLead_Tip:
				nCompareStrurct.push_back(sizeof(AlgoLeadTip));
				sName.push_back(_T("AlgoLeadTip"));
				nCompareStrurct.push_back(sizeof(RstAlgoLeadTip));
				sName.push_back(_T("RstAlgoLeadTip"));
				break;
			case eAlgoLead_Lift:
				nCompareStrurct.push_back(sizeof(AlgoLeadLift));
				sName.push_back(_T("AlgoLeadLift"));
				nCompareStrurct.push_back(sizeof(RstAlgoLeadLift));
				sName.push_back(_T("RstAlgoLeadLift"));
				break;
			case eAlgoLead_Solder:
				nCompareStrurct.push_back(sizeof(AlgoLeadSolder));
				sName.push_back(_T("AlgoLeadSolder"));
				nCompareStrurct.push_back(sizeof(RstAlgoLeadSolder));
				sName.push_back(_T("RstAlgoLeadSolder"));
				break;
			case eAlgoLead_SideSolder:
				nCompareStrurct.push_back(sizeof(AlgoLeadSideSolder));
				sName.push_back(_T("AlgoLeadSideSolder"));
				nCompareStrurct.push_back(sizeof(RstAlgoLeadSideSolder));
				sName.push_back(_T("RstAlgoLeadSideSolder"));
				break;
			case eAlgoWidth:
				nCompareStrurct.push_back(sizeof(AlgoLength));
				sName.push_back(_T("AlgoLength"));
				nCompareStrurct.push_back(sizeof(RstAlgoLength));
				sName.push_back(_T("RstAlgoLength"));
				break;
			case eAlgoVolume:
				nCompareStrurct.push_back(sizeof(AlgoVolume));
				sName.push_back(_T("AlgoVolume"));
				nCompareStrurct.push_back(sizeof(RstAlgoVolume));
				sName.push_back(_T("RstAlgoVolume"));
				break;
			case eAlgoTab:
				nCompareStrurct.push_back(sizeof(AlgoTab));
				sName.push_back(_T("AlgoTab"));
				nCompareStrurct.push_back(sizeof(RstAlgoTab));
				sName.push_back(_T("RstAlgoTab"));
				break;
			case eAlgoGrid:
				nCompareStrurct.push_back(sizeof(AlgoGrid));
				sName.push_back(_T("AlgoGrid"));
				nCompareStrurct.push_back(sizeof(RstAlgoGrid));
				sName.push_back(_T("RstAlgoGrid"));
				break;
			case eAlgoLine:
				nCompareStrurct.push_back(sizeof(AlgoLine));
				sName.push_back(_T("AlgoLine"));
				nCompareStrurct.push_back(sizeof(RstAlgoLine));
				sName.push_back(_T("RstAlgoLine"));
				break;
			case eAlgoEdge:
				nCompareStrurct.push_back(sizeof(AlgoEdge));
				sName.push_back(_T("AlgoEdge"));
				nCompareStrurct.push_back(sizeof(RstAlgoEdge));
				sName.push_back(_T("RstAlgoEdge"));
				break;
			case eAlgoSolderCone:
				nCompareStrurct.push_back(sizeof(AlgoSolderCone));
				sName.push_back(_T("AlgoSolderCone"));
				nCompareStrurct.push_back(sizeof(RstAlgoSolderCone));
				sName.push_back(_T("RstAlgoSolderCone"));
				break;
			case eAlgoColorXY:
				nCompareStrurct.push_back(sizeof(AlgoColorXY));
				sName.push_back(_T("AlgoColorXY"));
				nCompareStrurct.push_back(sizeof(RstAlgoColorXY));
				sName.push_back(_T("RstAlgoColorXY"));
				break;
			case eAlgoAlignEdge:
				nCompareStrurct.push_back(sizeof(AlgoAlignEdge));
				sName.push_back(_T("AlgoAlignEdge"));
				nCompareStrurct.push_back(sizeof(RstAlgoAlignEdge));
				sName.push_back(_T("RstAlgoAlignEdge"));
				break;
			case eAlgoPadAlign:
				nCompareStrurct.push_back(sizeof(AlgoPadAlign));
				sName.push_back(_T("AlgoPadAlign"));
				nCompareStrurct.push_back(sizeof(RstAlgoPadAlign));
				sName.push_back(_T("RstAlgoPadAlign"));
				break;
			case eAlgoPOCR:
				nCompareStrurct.push_back(sizeof(AlgoPOCR));
				sName.push_back(_T("AlgoPOCR"));
				nCompareStrurct.push_back(sizeof(RstAlgoPOCR));
				sName.push_back(_T("RstAlgoPOCR"));
				break;
			case eAlgoWire:
				nCompareStrurct.push_back(sizeof(AlgoWire));
				sName.push_back(_T("AlgoWire"));
				nCompareStrurct.push_back(sizeof(RstAlgoWire));
				sName.push_back(_T("RstAlgoWire"));
				break;
			case eAlgoFoot:
				nCompareStrurct.push_back(sizeof(AlgoFoot));
				sName.push_back(_T("AlgoFoot"));
				nCompareStrurct.push_back(sizeof(RstAlgoFoot));
				sName.push_back(_T("RstAlgoFoot"));
				break;
			case eAlgoBarcode:
				nCompareStrurct.push_back(sizeof(AlgoBarcode));
				sName.push_back(_T("AlgoBarcode"));
				nCompareStrurct.push_back(sizeof(RstAlgoBarcode));
				sName.push_back(_T("RstAlgoBarcode"));
				break;
			case eAlgoFillet:
				nCompareStrurct.push_back(sizeof(AlgoFillet));
				sName.push_back(_T("AlgoFillet"));
				nCompareStrurct.push_back(sizeof(RstAlgoFillet));
				sName.push_back(_T("RstAlgoFillet"));
				break;
			case eAlgoBGA:
				nCompareStrurct.push_back(sizeof(AlgoBGA));
				sName.push_back(_T("AlgoBGA"));
				nCompareStrurct.push_back(sizeof(RstAlgoBGA));
				sName.push_back(_T("RstAlgoBGA"));
				break;
			case eAlgoBump:
				nCompareStrurct.push_back(sizeof(AlgoBump));
				sName.push_back(_T("AlgoBump"));
				nCompareStrurct.push_back(sizeof(RstAlgoBump));
				sName.push_back(_T("RstAlgoBump"));
				break;
			case eAlgoNGBlob:
				nCompareStrurct.push_back(sizeof(AlgoNGBlob));
				sName.push_back(_T("AlgoNGBlob"));
				nCompareStrurct.push_back(sizeof(RstAlgoNGBlob));
				sName.push_back(_T("RstAlgoNGBlob"));
				break;
			case eAlgoBodyEdge:
				nCompareStrurct.push_back(sizeof(AlgoBodyEdge));
				sName.push_back(_T("AlgoBodyEdge"));
				nCompareStrurct.push_back(sizeof(RstAlgoBodyEdge));
				sName.push_back(_T("RstAlgoBodyEdge"));
				break;
			case eAlgoColorBand_Search:
				// 
				break;
			case eAlgoDisColor:
				//
				break;
			case eAlgoPadBW:
				nCompareStrurct.push_back(sizeof(AlgoPadBW));
				sName.push_back(_T("AlgoPadBW"));
				nCompareStrurct.push_back(sizeof(RstAlgoPadBW));
				sName.push_back(_T("RstAlgoPadBW"));
				break;
			case eAlgoForeignOCV:
				//
				break;
			case eAlgoPackageThickness:
				//
				break;
			case eAlgoDistance:
				nCompareStrurct.push_back(sizeof(AlgoDistance));
				sName.push_back(_T("AlgoDistance"));
				nCompareStrurct.push_back(sizeof(RstAlgoDistance));
				sName.push_back(_T("RstAlgoDistance"));
				break;
			case eAlgoPatternDiff:
				nCompareStrurct.push_back(sizeof(AlgoPatternDiff));
				sName.push_back(_T("AlgoPatternDiff"));
				nCompareStrurct.push_back(sizeof(RstAlgoPatternDiff));
				sName.push_back(_T("RstAlgoPatternDiff"));
				break;
			case eAlgoShapeX:
				nCompareStrurct.push_back(sizeof(AlgoShapeX));
				sName.push_back(_T("AlgoShapeX"));
				nCompareStrurct.push_back(sizeof(RstAlgoShapeX));
				sName.push_back(_T("RstAlgoShapeX"));
				break;
			default:
			                                                                                                                                                    
				break;
			}
		}
		if(!bServer)
			this->First()->nAlgoCnt = nCompareStrurct.size() / 2 ;
		
		nCompareStrurct.push_back(sizeof(InspPartParam));			sName.push_back(_T("InspPartParam"));
		nCompareStrurct.push_back(sizeof(InspPartInfo));			sName.push_back(_T("InspPartInfo"));
		nCompareStrurct.push_back(sizeof(ZmapData));				sName.push_back(_T("ZmapData"));
		nCompareStrurct.push_back(sizeof(TeachParam));				sName.push_back(_T("TeachParam"));
		nCompareStrurct.push_back(sizeof(Coordinate));				sName.push_back(_T("Coordinate"));
		nCompareStrurct.push_back(sizeof(BodyOffset));				sName.push_back(_T("BodyOffset"));
		nCompareStrurct.push_back(sizeof(AlignResult));				sName.push_back(_T("AlignResult"));
		nCompareStrurct.push_back(sizeof(NgParam));					sName.push_back(_T("NgParam"));
		nCompareStrurct.push_back(sizeof(CalcRoiParm));				sName.push_back(_T("CalcRoiParm"));
		nCompareStrurct.push_back(sizeof(DefaultGrayMeanStd));		sName.push_back(_T("DefaultGrayMeanStd"));
		nCompareStrurct.push_back(sizeof(DefaultColorXYStd));		sName.push_back(_T("DefaultColorXYStd"));
		nCompareStrurct.push_back(sizeof(AlgoDebuggingMode));		sName.push_back(_T("AlgoDebuggingMode"));
		nCompareStrurct.push_back(sizeof(InspAlgo));				sName.push_back(_T("InspAlgo"));
		nCompareStrurct.push_back(sizeof(InspAlgoTempResult));		sName.push_back(_T("InspAlgoTempResult"));
		nCompareStrurct.push_back(sizeof(InspAlgoResult));			sName.push_back(_T("InspAlgoResult"));
		nCompareStrurct.push_back(sizeof(InspWndResult));			sName.push_back(_T("InspWndResult"));
		nCompareStrurct.push_back(sizeof(InspectionResult));		sName.push_back(_T("InspectionResult"));
		nCompareStrurct.push_back(sizeof(tagInspBarcodeParam));		sName.push_back(_T("tagInspBarcodeParam"));
		nCompareStrurct.push_back(sizeof(ParamInfo));				sName.push_back(_T("ParamInfo"));
		nCompareStrurct.push_back(sizeof(lightData));			sName.push_back(_T("lightData"));
		nCompareStrurct.push_back(sizeof(ExtProductionInfo));			sName.push_back(_T("ExtProductionInfo"));
		nCompareStrurct.push_back(sizeof(ExtAlgoMachineInfo));		sName.push_back(_T("ExtAlgoMachineInfo"));
		nCompareStrurct.push_back(sizeof(ExtPOCRFont));		sName.push_back(_T("ExtPOCRFont"));
		nCompareStrurct.push_back(sizeof(CtrlServer));			sName.push_back(_T("CtrlServer"));
		nCompareStrurct.push_back(sizeof(CtrlClient));		sName.push_back(_T("CtrlClient"));
		nCompareStrurct.push_back(sizeof(ClientBufferCtrl));		sName.push_back(_T("ClientBufferCtrl"));
		nCompareStrurct.push_back(sizeof(AlgoColorBase));		sName.push_back(_T("AlgoColorBase"));
		nCompareStrurct.push_back(sizeof(ForeignData));				sName.push_back(_T("ForeignData"));
		nCompareStrurct.push_back(sizeof(FR2DData));				sName.push_back(_T("FR2DData"));
		nCompareStrurct.push_back(sizeof(InspForeignInfo));			sName.push_back(_T("InspForeignInfo"));
		nCompareStrurct.push_back(sizeof(ForeignParamROI));			sName.push_back(_T("ForeignParamROI"));
		nCompareStrurct.push_back(sizeof(InspFovForeignResult));	sName.push_back(_T("InspFovForeignResult"));
		if (!bServer)
		{
			CString sLogRst;

			sLogRst.Format(_T("[EXTStructCompare] Struct Wirte Start") );
			ext::Log::add(sLogRst);

			this->First()->nTotalCnt = nCompareStrurct.size();
			std::copy(nCompareStrurct.begin(), nCompareStrurct.end(), this->First()->nStructSizeArray);
			this->First()->nStructFlag = 1; 
			this->_stream.Flush();

			sLogRst.Format(_T("[EXTStructCompare] Struct Wirte End"));
			ext::Log::add(sLogRst);

		}
		else
		{
			bool bCompare = true;
			int nSize = this->First()->nTotalCnt;
			std::vector<int> dest(std::begin(this->First()->nStructSizeArray), std::end(this->First()->nStructSizeArray));
			for (int i = 0; i < nCompareStrurct.size(); i++)
			{
				bCompare &= (nCompareStrurct[i] == dest[i]);
				if ((nCompareStrurct[i] == dest[i]))
				{

					CString sLog;
					sLog.Format(_T("[EXTStructCompare] %s [%d == %d] "), sName[i], nCompareStrurct[i], dest[i]);
					ext::Log::add(sLog);
				}
				else
				{
					CString sLog;
					sLog.Format(_T("[EXTStructCompare] %s [%d != %d] FALSE "), sName[i], nCompareStrurct[i], dest[i]);
					ext::Log::add(sLog);

				}
				
			}
			CString sLogRst;
			sLogRst.Format(_T("[EXTStructCompare] ResultCompare %s"), (bCompare) ? _T("TRUE") : _T("FALSE"));
			ext::Log::add(sLogRst);
		}

		
		
	}

	InspSchdule::Ptr InspSchdule::s_Obj;

	InspSchdule::Ptr InspSchdule::get()
	{

		if (s_Obj == nullptr)
		{
			s_Obj = std::shared_ptr<InspSchdule>(new InspSchdule());
		}
		return s_Obj;
	}
	void InspSchdule::init()
	{
		if (m_vTimeList.size() > 0)
			m_vTimeList.clear();

		m_vTimeList.resize(ext::LANE::eLaneCount);
	}
	void InspSchdule::Exit()
	{
	}
	void InspSchdule::Clear(int nLane)
	{
		m_vTimeList[nLane].nCurCnt = 0;
		m_vTimeList[nLane].vList_Mean.clear();
		for (int i = 0; i < MAX_TIMELIST; i++)
			m_vTimeList[nLane].vList_Recent[i].clear();
	}
	void InspSchdule::SetList(int nLane, vector<int> vPartID)
	{
		Clear(nLane);

		for (int i = 0; i < vPartID.size(); i++)
		{
			//Mean List 초기화
			m_vTimeList[nLane].vList_Mean.insert(std::unordered_map<int, double>::value_type(vPartID[i], 0.0));

			//Recent List 초기화
			for (int j = 0; j < MAX_TIMELIST; j++)
			{
				m_vTimeList[nLane].vList_Recent[j].insert(std::unordered_map<int, double>::value_type(vPartID[i], 0.0));
			}
		}

	}
	void InspSchdule::UpdateExtInspDoneCnt(int nLane)
	{
		int nCurCnt = m_vTimeList[nLane].nCurCnt + 1;
		m_vTimeList[nLane].nCurCnt = nCurCnt % MAX_TIMELIST;
	}
	void InspSchdule::UpdatePartTime(int nLane, int nPartID, double dMiliSec)
	{
		double dAvgTime = 0.0;
		int nAvgCnt = 0;
		int nCurCnt = m_vTimeList[nLane].nCurCnt;

		//현재 검사 Map에 업데이트
		auto it = m_vTimeList[nLane].vList_Recent[nCurCnt].find(nPartID);

		if (it != m_vTimeList[nLane].vList_Recent[nCurCnt].end())
			it->second = dMiliSec;
		else
			m_vTimeList[nLane].vList_Recent[nCurCnt].insert(std::unordered_map<int, double>::value_type(nPartID, dMiliSec));

		//평균 검사시간 갱신 
		//1.탐색
		for (int i = 0; i < MAX_TIMELIST; i++)
		{
			auto it = m_vTimeList[nLane].vList_Recent[i].find(nPartID);

			if (it != m_vTimeList[nLane].vList_Recent[i].end())
			{
				if (it->second > 0.0)
				{
					dAvgTime += it->second;
					nAvgCnt++;
				}
			}
		}

		//2.갱신
		if (nAvgCnt > 0)
		{
			auto it = m_vTimeList[nLane].vList_Mean.find(nPartID);
			if (it != m_vTimeList[nLane].vList_Mean.end())
				it->second = dAvgTime / (double)nAvgCnt;
		}

	}
	BOOL InspSchdule::GetFirstInsp(int nLane)
	{
		BOOL bFirst = TRUE;

		if (m_vTimeList[nLane].nCurCnt != 0)
			bFirst = FALSE;

		return bFirst;
	}
	double InspSchdule::GetExpectTime(int nLane, int nPartID)
	{
		double dRet = 0.0;

		auto it = m_vTimeList[nLane].vList_Mean.find(nPartID);
		if (it != m_vTimeList[nLane].vList_Mean.end())
			dRet = it->second;

		return dRet;
	}
	ModelImageBuffer::ModelImageBuffer()
		: _offset(0)
		, _sizebytes(0)
	{
		_TotalBuff = 0;
	}
	void ModelImageBuffer::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}
		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			_stream->AllocMemory(_sizebytes, sName);
		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit = false;
	}
	bool ModelImageBuffer::Init(int nModelIndex)
	{
		bool bInit = true;
		CString sLog;
		sLog.Format(_T("ModelImageBuffer::Init() - Start _ModelIndex = %d "), nModelIndex);
		ext::Log::add(sLog);
		int access = FILE_MAP_ALL_ACCESS;
		if (env::nSystemType != EXT_INSP_SERVER)
			access = FILE_MAP_READ;
		CString sModelName = _T("");
		sModelName.Format(_T("AOI_ExtModelImage_%d"), nModelIndex);
		
		int nSize = env::nPOCRModelData;
		switch (nModelIndex)
		{
		case eExtModel_POCR: break;
		case eExtModel_Pattern: break;
		case eExtModel_COB: break;
		case eExtModel_ShapeX: break;
		case eExtModel_PadBW: //nSize = (1024 * 1024 * 25); // 25M 한장들어갈 예정
			break;
		case eExtModel_Exc: break;
		case ext::ModelBufferFlag::eExtModel_NGBlobExc:
			break;
		default:
			break;
		}
		_TotalBuff = nSize; 
		Create(nSize, sModelName, access);
		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit = false;
		sLog.Format(_T("ModelImageBuffer::Init() - End _ModelIndex = %d "), nModelIndex);
		ext::Log::add(sLog);
		return bInit;
	}
	void ModelImageBuffer::Exit()
	{
		if(_stream !=nullptr)
		_stream->Free();
	}
	bool ModelImageBuffer::ReInit(long long _llSize) // 2g 이상 Size Over 됬을때 Page 생성 아직미구현
	{
		bool bInit = true;
		ext::Log::add(_T("POCRImageBuffer::ReInit() - Start"));
		_stream->UnmapView(true);
		double rstSize = (double)_llSize / (double)env::nPOCRModelData;
		double rstCeil = ceil(rstSize); // 무조건 올려서 갯수를 정함.
		int access = FILE_MAP_ALL_ACCESS;
		if (env::nSystemType != EXT_INSP_SERVER)
			access = FILE_MAP_READ;
		for (int i = 0; i < rstCeil; i++)
		{
			CString strCreate; 
			strCreate.Format(_T("AOI_ExtPOCRImageBuffer_%d"),i);
			Create(env::nPOCRModelData, strCreate, access); // 나눠서 생성 
			if (_stream->MapView(0, _sizebytes, access) == false)
				bInit = false;
		}
		ext::Log::add(_T("POCRImageBuffer::ReInit() - End"));
		return bInit;
	}
	ImgData ModelImageBuffer::Push(BYTE * ptr, int _nSize, int widthbytes)
	{
		ImgData ret;
		CString sLog;
		if (_offset >= _sizebytes || (_offset + (_nSize)) >= _sizebytes)
		{
			sLog.Format(_T("[EXT][Model_Fail] _offset %d ,_Size %d ,_sizebytes %d Reset Buffer "), _offset, _nSize, _sizebytes);
			ext::Log::add(sLog);
			_offset = 0;
		}
		sLog.Format(_T("_offset %d ,_Size %d ,_sizebytes %d"), _offset, _nSize, _sizebytes);
		ext::Log::add(sLog);
		_stream->MapToWrite(_offset, _nSize);
		ret.imgStIdx = _offset;
		BYTE * buf = _stream->Byte();
		memcpy(buf, ptr, _nSize);
		_stream->Flush(buf, _nSize);
		_stream->UnmapView();
		ret.imgStIdx = _offset;
		ret.imgSz = _nSize;
		_offset += _nSize;
		return ret;
	}
	ImgData ModelImageBuffer::FilePush(CFile * file)
	{
		ImgData ret;
		int _nSize = file->GetLength();
		CString sLog;
		if (_offset >= _sizebytes || (_offset + (_nSize)) >= _sizebytes)
		{
			sLog.Format(_T("[EXT][Model_Fail] _offset %d ,_Size %d ,_sizebytes %d Reset Buffer "), _offset, _nSize, _sizebytes);
			ext::Log::add(sLog);
			_offset = 0;
		}
		sLog.Format(_T("_offset %d ,_Size %d ,_sizebytes %d"),_offset,_nSize,_sizebytes);
		ext::Log::add(sLog);
		_stream->MapToWrite(_offset, _nSize);
		ret.imgStIdx = _offset;
		BYTE * buf = _stream->Byte();
		file->Read(buf, _nSize);
		file->SeekToBegin(); // Pointer를 처음으로 
		_stream->Flush(buf, _nSize);
		_stream->UnmapView();
		ret.imgStIdx = _offset;
		ret.imgSz = _nSize;
		_offset += _nSize;
		return ret;
	}
	BYTE * ModelImageBuffer::Lock(ImgData pos)
	{
		if (_stream->MapToRead(pos.imgStIdx, pos.imgSz) == false)
			return false;
		return _stream->Byte();
	}
	void ModelImageBuffer::Clear()
	{
		_offset = 0;
	}

	_ForeignData::_ForeignData() 
		:_offset(0)
		, _sizebytes(0)
		, _qwCurPos(0)
	{

	}
	bool _ForeignData::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}

		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			bInit &= _stream->AllocMemory(_sizebytes, sName);

		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit &= false;
		return bInit;
	}
	bool _ForeignData::Init(int nToolID)
	{
		bool bInit = true;

		CString sName, sLog;
		sName.Format(_T("AOI_ExtForeigeData_%d_Index_%d"), nToolID, m_nBuffIdx);

		sLog.Format(_T("ExtForeigeData Index_%d - Start()"), m_nBuffIdx);
		ext::Log::add(sLog);

		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;

		if (Create(EnvVariable::nForeigeData, sName, nAccessType) == false)
			bInit = false;
		if (!bInit)
		{
			sLog.Format(_T("%s_Init_Fail"), sName);
			ext::Log::add(sLog);
		}


		sLog.Format(_T("ExtForeigeData Index_%d - End()"), m_nBuffIdx);
		ext::Log::add(sLog);

		return bInit;
	}
	void _ForeignData::Exit()
	{
		if (_stream != nullptr)
		_stream->Free();
	}
	void _ForeignData::AddInspForeign(InspPartInfo* _ForePartInfo,ForeignData _foreinData , FR2DData Fr2Data , InspForeignInfo m_ForeignInfoData, ForeignParamROI* _ForeParamROI, ForeignParamROI* arrROI)
	{
		CString sLog;
		sLog.Format(_T("AddInspForeign Start_%d ") , m_nBuffIdx);
		ext::Log::add(sLog);


		_qwCurPos = 0;
		memset(m_qwArrOffset, 0,sizeof(QWORD) * 100);
		_qwCurPos += sizeof(tagSharedForeignHeader); // Header 정보를 제외하고 Add 하기위함
		AddForeignData(_foreinData);
		AddFR2DData(Fr2Data);
		AddForeignInfoData(m_ForeignInfoData, _ForePartInfo, _ForeParamROI, arrROI);
		AddArrROI(arrROI, _foreinData.m_nFRROI);
		AddForeignHeader();

		_stream->Flush();

		sLog.Format(_T("AddInspForeign End_%d "), m_nBuffIdx);
		ext::Log::add(sLog);

	}
	void _ForeignData::AddForeignHeader()
	{
		
		BYTE * ptr = _stream->Byte(0);
		tagSharedForeignHeader tgheader;
		tgheader.m_nZerobyte = 0;
		memcpy(tgheader.m_qwOffSet, m_qwArrOffset, sizeof(QWORD) * 100);
		memcpy(ptr, &tgheader, sizeof(tagSharedForeignHeader) );
		//PointerPush(sizeof(tagSharedForeignHeader), 1, &tgheader, 1);
		//_stream->Flush(ptr, sizeof(tagSharedForeignHeader));

	}

	void _ForeignData::AddForeignData(ForeignData _foreinData)
	{

		//SnapShot ==> 1Page의 Memory 공간 오픈이후 연달아서 쓸때 위치를 파악하기위한 포인터, 
		// Get으로 가져올때 참조의 위치가 된다
		PointerPush(sizeof(ForeignData), 1, &_foreinData, eForeignData::eForeignSturct_DataPtr); // 구조체 Copy
		PointerPush(sizeof(int), _foreinData.m_nTotalCnt, _foreinData.m_pXList, eForeignData::eForeign_XList); // 구조체 안의 Pointer Copy
		PointerPush(sizeof(int), _foreinData.m_nTotalCnt, _foreinData.m_pYList, eForeignData::eForeign_YList);
		PointerPush(sizeof(int), _foreinData.m_nTotalCnt, _foreinData.m_pWList, eForeignData::eForeign_WList);
		PointerPush(sizeof(int), _foreinData.m_nTotalCnt, _foreinData.m_pHList, eForeignData::eForeign_HList);
		PointerPush(sizeof(int), _foreinData.m_nModule, _foreinData.m_pInspModule, eForeignData::eForeign_InspModule );
		
		//PushForeign(_foreinData.m_pDesImg, _foreinData.m_nWP_W, _foreinData.m_nWP_H,sizeof(UCHAR),eForeignData::eForeign_DesImg);
		//PushForeign(_foreinData.m_pDesImgBin, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR),eForeignData::eForeign_DesImgBin);
		//PushForeign(_foreinData.m_pimgTopR, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR),eForeignData::eForeign_imgTopR);
		//PushForeign(_foreinData.m_pimgTopG, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR),eForeignData::eForeign_imgTopG);
		//PushForeign(_foreinData.m_pimgTopB, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR),eForeignData::eForeign_imgTopB);
		//PushForeign(_foreinData.m_pimgWPInspA, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR),eForeignData::eForeign_imgWPInspA);

		//PushForeign(_foreinData.m_p3D, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(float), eForeignData::eForeign_p3Dm);
		//PushForeign(_foreinData.m_pimgFR, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(float), eForeignData::eForeign_pimgFR);
		//PushForeign(_foreinData.m_pimgWP, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(float), eForeignData::eForeign_pimgWP);
		//PushForeign(_foreinData.m_pimgSUB, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(float), eForeignData::eForeign_pimgSUB);
		// ForeignDatga 는 구조를 유지하려면 FOV 별로 한개 씩 가지고있어야해서 해당부분에서 이미지를 불러와서 넣도록 변경 

		
	}

	void _ForeignData::AddFR2DData(FR2DData Fr2Data)
	{
		PointerPush(sizeof(FR2DData), 1, &Fr2Data, eForeignData_FR2DData::eFR2DDataSturct_DataPtr); // 구조체 Copy
		PushForeign(Fr2Data.m_pFr1, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pFr1);
		PushForeign(Fr2Data.m_pFr2, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pFr2);
		PushForeign(Fr2Data.m_pFr3, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pFr3);
		PushForeign(Fr2Data.m_pFr4, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pFr4);
		PushForeign(Fr2Data.m_pucTR, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pucTR);
		PushForeign(Fr2Data.m_pucTG, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pucTG);
		PushForeign(Fr2Data.m_pucTB, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pucTB);
		PushForeign(Fr2Data.m_pucBR, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pucBR);
		PushForeign(Fr2Data.m_pucBB, Fr2Data.m_nW, Fr2Data.m_nH, sizeof(UCHAR), eForeignData_FR2DData::eFR2DData_m_pucBB);
	}
	void _ForeignData::AddForeignInfoData(InspForeignInfo m_ForeignInfoData , InspPartInfo* _ForePartInfo , ForeignParamROI* _ForeParamROI, ForeignParamROI* arrROI)
	{
		PointerPush(sizeof(InspForeignInfo), 1, &m_ForeignInfoData, eForeignInfoData::eForeignInfoData_Struct); // 구조체 Copy
		
		//구조체에서 ZmapForeignData 만 사용하므로 해당데이터만 넣도록 
		PointerPush(sizeof(InspPartInfo), 1, _ForePartInfo, eForeignInfoData::eForeignInspPartInfo_Struct); // 구조체 Copy 
		PointerPush(sizeof(InspRoiImgBuf), 1, &_ForePartInfo->partImgBuf, eForeignInfoData::eForeignInspPartInfo_InspRoiImgBuf); // 구조체 Copy 
		PushForeign(_ForePartInfo->partImgBuf.imgTop_R, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgTop_R);
		PushForeign(_ForePartInfo->partImgBuf.imgTop_G, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgTop_G);
		PushForeign(_ForePartInfo->partImgBuf.imgTop_B, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgTop_B);
		PushForeign(_ForePartInfo->partImgBuf.imgSide1_R, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgSide1_R);
		PushForeign(_ForePartInfo->partImgBuf.imgSide1_G, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgSide1_G);
		PushForeign(_ForePartInfo->partImgBuf.imgSide1_B, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgSide1_B);
		PushForeign(_ForePartInfo->partImgBuf.imgSide2_R, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgSide2_R);
		PushForeign(_ForePartInfo->partImgBuf.imgBottom_R, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgBottom_R);
		PushForeign(_ForePartInfo->partImgBuf.imgBottom_B, _ForePartInfo->partImgBuf.nImageSizeX, _ForePartInfo->partImgBuf.nImageSizeY, sizeof(UCHAR), eForeignInfoData::eForeignInspPartInfo_imgBottom_B);

		PushForeign(_ForePartInfo->zmapForeignData.data, _ForePartInfo->zmapForeignData.zmapSizeX, _ForePartInfo->zmapForeignData.zmapSizeY, sizeof(float), eForeignInfoData::eForeignInspPartInfo_ZmapData);
		int nROITotalSize= m_ForeignInfoData.m_nArrData[FR_N_ForeignParamROI_Total];
		PointerPush(sizeof(ForeignParamROI), nROITotalSize, _ForeParamROI, eForeignInfoData::eForeignInspPartInfo_ForeParamROIStruct);
		
		

	}
	void _ForeignData::GetnspForeign(InspPartInfo* _ForePartInfo, ForeignData* _foreinData, FR2DData* Fr2Data, InspForeignInfo* m_ForeignInfoData, ForeignParamROI ** _ForeParamROI, ForeignParamROI ** arrRoi)
	{
		CString sLog;
		sLog.Format(_T("GetInspForeign Start "));
		ext::Log::add(sLog);
		//Get 으로 표기하고 Shared Memory 에 데이터들을 Pointer만 연결 해주는 작업
		//Copy 시 Program 성능저하 일어날 가능성이있음
		GetForeignHeader();
		GetForeignData(_foreinData);
		GetFR2DData(Fr2Data);
		GetForeignInfoData(m_ForeignInfoData, _ForePartInfo, _ForeParamROI);
		if(_ForePartInfo != NULL)
			GetArrRoiData(arrRoi);
		sLog.Format(_T("GetInspForeign End "));
		ext::Log::add(sLog);
	}
	void _ForeignData::GetForeignHeader()
	{
		BYTE* ptr = _stream->Byte(0);
		tagSharedForeignHeader* stHeader = (tagSharedForeignHeader*)ptr;

		if (stHeader->m_nZerobyte != 0)
		{
			//write log
			ext::Log::add(_T("Part header is wrong."));
		}
		memcpy(m_qwArrOffset, stHeader->m_qwOffSet, sizeof(QWORD)*100); // SnapShot Data 
	}
	void _ForeignData::GetForeignData(ForeignData* _foreinData)
	{
		
		*_foreinData = *(ForeignData*)GettingPointer(m_qwArrOffset[eForeignData::eForeignSturct_DataPtr]); // Ref 연결시에는 먼저 구조체 부터 연결
		_foreinData->m_pXList =(int *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_XList]);
		_foreinData->m_pYList = (int *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_YList]);
		_foreinData->m_pWList = (int *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_WList]);
		_foreinData->m_pHList = (int *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_HList]);
		_foreinData->m_pInspModule = (int *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_InspModule]);
		
		//_foreinData->m_pDesImg		= GettingPointer(m_qwArrOffset[eForeignData::eForeign_DesImg]);
		//_foreinData->m_pDesImgBin	= GettingPointer(m_qwArrOffset[eForeignData::eForeign_DesImgBin]);
		//_foreinData->m_pimgTopR		= GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopR]);
		//_foreinData->m_pimgTopG		= GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopG]);
		//_foreinData->m_pimgTopB		= GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopB]);
		//_foreinData->m_pimgWPInspA =  GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgWPInspA]);

		//_foreinData->m_p3D		= (float * )GettingPointer(m_qwArrOffset[eForeignData::eForeign_p3Dm]);
		//_foreinData->m_pimgFR = (float *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgFR]);
		//_foreinData->m_pimgWP = (float *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgWP]);
		//_foreinData->m_pimgSUB = (int *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgSUB]);
	}
	void _ForeignData::GetFR2DData(FR2DData* Fr2Data)
	{
		*Fr2Data = *(FR2DData *)GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DDataSturct_DataPtr]);
		Fr2Data->m_pFr1 = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pFr1]);
		Fr2Data->m_pFr2 = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pFr2]);
		Fr2Data->m_pFr3 = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pFr3]);
		Fr2Data->m_pFr4 = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pFr4]);
		Fr2Data->m_pucTR = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucTR]);
		Fr2Data->m_pucTG = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucTG]);
		Fr2Data->m_pucTB = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucTB]);
		Fr2Data->m_pucBR = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucBR]);
		Fr2Data->m_pucBB = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucBB]);
	}																	
	void _ForeignData::GetNGTempData(UCHAR* TR, UCHAR* TG , UCHAR* TB , UCHAR* BR , UCHAR* BB ) 
	{
	
		TR = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucTR]);
		TG = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucTG]);
		TB = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucTB]);
		BR = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucBR]);
		BB = GettingPointer(m_qwArrOffset[eForeignData_FR2DData::eFR2DData_m_pucBB]);
	}
	void _ForeignData::GetForeignInfoData(InspForeignInfo* m_ForeignInfoData, InspPartInfo * _ForePartInfo, ForeignParamROI ** _ForeParamROI)
	{
		if (m_ForeignInfoData == NULL && _ForePartInfo == NULL && _ForeParamROI == NULL)
			return;

		*_ForePartInfo = *(InspPartInfo*)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_Struct]);

		_ForePartInfo->partImgBuf = *(InspRoiImgBuf*)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_InspRoiImgBuf]);
		_ForePartInfo->partImgBuf.imgTop_R = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgTop_R]);
		_ForePartInfo->partImgBuf.imgTop_G = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgTop_G]);
		_ForePartInfo->partImgBuf.imgTop_B = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgTop_B]);
		_ForePartInfo->partImgBuf.imgSide1_R = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgSide1_R]);
		_ForePartInfo->partImgBuf.imgSide1_G = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgSide1_G]);
		_ForePartInfo->partImgBuf.imgSide1_B = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgSide1_B]);
		_ForePartInfo->partImgBuf.imgSide2_R = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgSide2_R]);
		_ForePartInfo->partImgBuf.imgBottom_R = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgBottom_R]);
		_ForePartInfo->partImgBuf.imgBottom_B = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgBottom_B]);

		_ForePartInfo->zmapForeignData.data = (float *)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_ZmapData]);

		*m_ForeignInfoData = *(InspForeignInfo*)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInfoData_Struct]);
		*_ForeParamROI = &*(ForeignParamROI*)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_ForeParamROIStruct]);
		//int nROITotalSize = m_ForeignInfoData->m_nArrData[FR_N_ForeignParamROI_Total];
		//ForeignParamROI * pPtr = (ForeignParamROI*)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_ForeParamROIStruct]);
		//for (int i = 0; i < nROITotalSize; i++)
		//	_ForeParamROI[i] = pPtr[i];
	}
	void _ForeignData::GetForeignPartInfoData(InspPartInfo * _ForePartInfo , bool bSave3D)
	{
		Sleep(1);
		GetForeignHeader();
		*_ForePartInfo = *(InspPartInfo*)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_Struct]);
		if (bSave3D)
		{
			_ForePartInfo->zmapForeignData.data = (float *)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_ZmapData]);
		}
		else
		{
			_ForePartInfo->partImgBuf = *(InspRoiImgBuf*)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_InspRoiImgBuf]);
			_ForePartInfo->partImgBuf.imgTop_R = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgTop_R]);
			_ForePartInfo->partImgBuf.imgTop_G = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgTop_G]);
			_ForePartInfo->partImgBuf.imgTop_B = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgTop_B]);
			_ForePartInfo->partImgBuf.imgSide1_R = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgSide1_R]);
			_ForePartInfo->partImgBuf.imgSide1_G = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgSide1_G]);
			_ForePartInfo->partImgBuf.imgSide1_B = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgSide1_B]);
			_ForePartInfo->partImgBuf.imgSide2_R = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgSide2_R]);
			_ForePartInfo->partImgBuf.imgBottom_R = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgBottom_R]);
			_ForePartInfo->partImgBuf.imgBottom_B = GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_imgBottom_B]);
			_ForePartInfo->zmapForeignData.data = (float *)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_ZmapData]);
		}
	}
	void _ForeignData::PushForeign(__in void * Inputptr, int nW, int nH, int nCh, int nMemorySnapShotIndex)// 이물은 한구조체에 이미지를 전부다 넣고 검사 
	{
		if (Inputptr == nullptr) // null 일경우 넣지않음.
			return ;
		QWORD old_qwCurPos = _qwCurPos;
		BYTE * buf = _stream->Byte(_qwCurPos);
		IppStatus sts;
		IppiSize iSize = { nW,nH };
		int nSize = 0; 
		if (nCh == 1)
		{
			sts = ippiCopy_8u_C1R((BYTE *)Inputptr, nW, buf, nW, iSize);
			nSize = nW * nH;
		}
		else
		{
			sts = ippiCopy_32f_C1R((float*)Inputptr, nW * sizeof(float), (float*)buf, nW * sizeof(float), iSize);
			nSize = nW *  nH * sizeof(float);
		}
		_qwCurPos += nSize;
		if (nMemorySnapShotIndex != -1)
			m_qwArrOffset[nMemorySnapShotIndex] = old_qwCurPos;
	}
	void _ForeignData::PointerPush(int nTyoeSize,int nDataSize,void * InputData,int nMemorySnapShotIndex )
	{
		if (InputData == nullptr) // null 일경우 넣지않음.
			return ;
		QWORD old_qwCurPos = _qwCurPos;
		BYTE * ptr = _stream->Byte(_qwCurPos);
		int nListSize = (nTyoeSize * nDataSize);
		memcpy(ptr, InputData, nListSize);
		_qwCurPos += nListSize;
		if (nMemorySnapShotIndex != -1)
			m_qwArrOffset[nMemorySnapShotIndex] = old_qwCurPos;

	}
	BYTE * _ForeignData::GettingPointer(QWORD Value)
	{
		if(Value == 0 ) // NULL 경우는 넣을필요가없습니다.
			return NULL;
					
		return _stream->Byte(Value);
		
	}
	_ForeignDataResult::_ForeignDataResult()
		:_offset(0)
		, _sizebytes(0)
		, _qwCurPos(0)
	{

	}
	bool _ForeignDataResult::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}

		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			bInit &= _stream->AllocMemory(_sizebytes, sName);

		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit &= false;
		return bInit;
	}
	bool _ForeignDataResult::Init(int nToolID)
	{
		bool bInit = true;

		CString sName, sLog;
		sName.Format(_T("AOI_Ext_ForeignDataResult_%d_Index_%d"), nToolID, m_nBuffIdx);

		sLog.Format(_T("Ext_ForeignDataResultIndex_%d - Start()"), m_nBuffIdx);
		ext::Log::add(sLog);

		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;

		if (Create(EnvVariable::nTotalRstBufferSize, sName, nAccessType) == false)
			bInit = false;
		if (!bInit)
		{
			sLog.Format(_T("%s_Init_Fail"), sName);
			ext::Log::add(sLog);
		}


		sLog.Format(_T("Ext_ForeignDataResultIndex_%d - End()"), m_nBuffIdx);
		ext::Log::add(sLog);

		return bInit;
	}
	void _ForeignDataResult::Exit()
	{
		if (_stream != nullptr)
		_stream->Free();
		
	}
	void _ForeignDataResult::AddForeignRstData(InspFovForeignResult * ForeignRst)
	{
		CString sLog;
		sLog.Format(_T("AddIInspFovForeignResultStart "));
		ext::Log::add(sLog);

		//InspFovForeignResult * ptr =(InspFovForeignResult*)_stream->Byte(0);

		//ForeignRst ->Clone(*ptr);

		sLog.Format(_T("AddIInspFovForeignResultEnd "));
		ext::Log::add(sLog);
	}
	void _ForeignDataResult::AddForeignRstHeader()
	{
		BYTE * ptr = _stream->Byte(0);
		tagSharedForeignHeader tgheader;
		tgheader.m_nZerobyte = 0;
		memcpy(tgheader.m_qwOffSet, m_qwArrOffset, sizeof(QWORD) * 100);
		memcpy(ptr, &tgheader, sizeof(tagSharedForeignHeader)); // Snap Shot 정보
		_stream->Flush(ptr, sizeof(tagSharedForeignHeader));
	}
	void _ForeignDataResult::AddForeignRstFovForeign(InspFovForeignResult * ForeignRst, int ret )
	{
		_qwCurPos = 0;
		memset(m_qwArrOffset, 0, sizeof(QWORD) * 100);
		_qwCurPos += sizeof(tagSharedForeignHeader); // Header 정보를 제외하고 Add 하기위함

		PointerPush(sizeof(InspFovForeignResult), 1, ForeignRst, eForeignResultData::eForeignResultData_Sturct); // 구조체 Copy 
		PointerPush(sizeof(AForeignResult), ForeignRst->m_nCountDefect, ForeignRst->m_stForeign, eForeignResultData::eForeignResultData_AForeignResult);
		PointerPush(sizeof(AForeignResultWP), ForeignRst->m_nCountDefectWP, ForeignRst->m_stForeignWP, eForeignResultData::eForeignResultData_m_stForeignWP);
		m_qwArrOffset[99] = ret;// 99 번째는 nDefectCode로 
		AddForeignRstHeader();
	}
	
	void _ForeignDataResult::PointerPush(int nTyoeSize, int nDataSize, void * InputData, int nMemorySnapShotIndex)
	{
		if (InputData == nullptr) // null 일경우 넣지않음.
			return;
		QWORD old_qwCurPos = _qwCurPos;
		BYTE * ptr = _stream->Byte(_qwCurPos);
		int nListSize = (nTyoeSize * nDataSize);
		memcpy(ptr, InputData, nListSize);
		_qwCurPos += nListSize;
		if (nMemorySnapShotIndex != -1)
			m_qwArrOffset[nMemorySnapShotIndex] = old_qwCurPos;

	}
	BYTE * _ForeignDataResult::GettingPointer(QWORD Value)
	{
		if (Value == 0) // NULL 경우는 넣을필요가없습니다.
			return NULL;

		return _stream->Byte(Value);

	}

	int _ForeignDataResult::GetForeignRstData(InspFovForeignResult *& ForeignRst)
	{
		GetForeignRstHeader();


		*ForeignRst = *(InspFovForeignResult*)GettingPointer(m_qwArrOffset[eForeignResultData::eForeignResultData_Sturct]);
		ForeignRst->m_stForeign = (AForeignResult*)GettingPointer(m_qwArrOffset[eForeignResultData::eForeignResultData_AForeignResult]);
		ForeignRst->m_stForeignWP = (AForeignResultWP*)GettingPointer(m_qwArrOffset[eForeignResultData::eForeignResultData_m_stForeignWP]);

		int ret = m_qwArrOffset[99];//(ForeignRst->m_bOK) ? dftCODE_OK : defCODE_FOREIGN;
		return ret;
		
	}
	void _ForeignDataResult::GetForeignRstHeader()
	{
		BYTE* ptr = _stream->Byte(0);
		tagSharedForeignHeader* stHeader = (tagSharedForeignHeader*)ptr;

		if (stHeader->m_nZerobyte != 0)
		{
			//write log
			ext::Log::add(_T("Part header is wrong."));
		}
		memcpy(m_qwArrOffset, stHeader->m_qwOffSet, sizeof(QWORD) * 100); // SnapShot Data 
	}
	bool _ForeignDataResult::GetNext(InspFovForeignResult *& rst, /*int nCalcDoneCnt,*/ int * nExtDefectCode)
	{
		bool ret = false;
		//if (_nRecvRstCount < nCalcDoneCnt)
		{
			*nExtDefectCode = GetForeignRstData(rst);
			//_nRecvRstCount++;
			ret = true;
		}
	/*	else
			ret = false;*/

		return ret;
	}

	_ForeignDataWP::_ForeignDataWP()
		:_offset(0)
		, _sizebytes(0)
		, _qwCurPos(0)
	{

	}
	bool _ForeignDataWP::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}

		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			bInit &= _stream->AllocMemory(_sizebytes, sName);

		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit &= false;
		return bInit;
	}
	bool _ForeignDataWP::Init(int nToolID)
	{
		bool bInit = true;
		// WP 는 한개의 보드에 대한 정보임
		CString sName, sLog;
		sName.Format(_T("AOI_Ext_WarpageImage"));

		sLog.Format(_T("AOI_Ext_WarpageImage - Start()"));
		ext::Log::add(sLog);

		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;

		if (Create(EnvVariable::nForeigeDataWP, sName, nAccessType) == false)
			bInit = false;
		if (!bInit)
		{
			sLog.Format(_T("%s_Init_Fail"), sName);
			ext::Log::add(sLog);
		}
		sLog.Format(_T("AOI_Ext_WarpageImage - End()"));
		ext::Log::add(sLog);

		return bInit;
	}
	void _ForeignDataWP::Exit()
	{
		if (_stream != nullptr)
		_stream->Free();
	}
	void _ForeignDataWP::AddForeignHeader()
	{
		BYTE * ptr = _stream->Byte(0);
		tagSharedForeignHeader tgheader;
		tgheader.m_nZerobyte = 0;
		memcpy(tgheader.m_qwOffSet, m_qwArrOffset, sizeof(QWORD) * 100);
		memcpy(ptr, &tgheader, sizeof(tagSharedForeignHeader));
		//PointerPush(sizeof(tagSharedForeignHeader), 1, &tgheader, 1);
		_stream->Flush(ptr, sizeof(tagSharedForeignHeader));

	}
	void _ForeignDataWP::AddInspWarpageImage(ForeignData _foreinData)
	{

		_qwCurPos = 0;
		memset(m_qwArrOffset, 0, sizeof(QWORD) * 100);
		_qwCurPos += sizeof(tagSharedForeignHeader); // Header 정보를 제외하고 Add 하기위함

		PushForeign(_foreinData.m_pDesImg, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR)*3, eForeignData::eForeign_DesImg);
		PushForeign(_foreinData.m_pDesImgBin, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR), eForeignData::eForeign_DesImgBin);
		PushForeign(_foreinData.m_pimgTopR, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR), eForeignData::eForeign_imgTopR);
		PushForeign(_foreinData.m_pimgTopG, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR), eForeignData::eForeign_imgTopG);
		PushForeign(_foreinData.m_pimgTopB, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR), eForeignData::eForeign_imgTopB);
		PushForeign(_foreinData.m_pimgWPInspA, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(UCHAR), eForeignData::eForeign_imgWPInspA);

		PushForeign(_foreinData.m_p3D, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(float), eForeignData::eForeign_p3Dm);
		PushForeign(_foreinData.m_pimgFR, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(float), eForeignData::eForeign_pimgFR);
		PushForeign(_foreinData.m_pimgWP, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(float), eForeignData::eForeign_pimgWP);
		PushForeign(_foreinData.m_pimgSUB, _foreinData.m_nWP_W, _foreinData.m_nWP_H, sizeof(float), eForeignData::eForeign_pimgSUB);

		AddForeignHeader();
	}
	void _ForeignDataWP::GetForeignHeader()
	{
		BYTE* ptr = _stream->Byte(0);
		tagSharedForeignHeader* stHeader = (tagSharedForeignHeader*)ptr;

		if (stHeader->m_nZerobyte != 0)
		{
			//write log
			ext::Log::add(_T("Part header is wrong."));
		}
		memcpy(m_qwArrOffset, stHeader->m_qwOffSet, sizeof(QWORD) * 100); // SnapShot Data 
	}
	void _ForeignDataWP::GetInspWarpageImage(ForeignData* _foreinData , bool LastCpy)
	{
		if (LastCpy == false)
		{
			GetForeignHeader();


			_foreinData->m_pDesImg = GettingPointer(m_qwArrOffset[eForeignData::eForeign_DesImg]);
			_foreinData->m_pDesImgBin = GettingPointer(m_qwArrOffset[eForeignData::eForeign_DesImgBin]);
			_foreinData->m_pimgTopR = GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopR]);
			_foreinData->m_pimgTopG = GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopG]);
			_foreinData->m_pimgTopB = GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopB]);
			_foreinData->m_pimgWPInspA = GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgWPInspA]);
			_foreinData->m_p3D = (float *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_p3Dm]);
			_foreinData->m_pimgFR = (float *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgFR]);
			_foreinData->m_pimgWP = (float *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgWP]);
			_foreinData->m_pimgSUB = (int *)GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgSUB]);


		}
		else
		{
			GetForeignHeader();
			int nWidth, nHeight; 
			nWidth = _foreinData->m_nWP_W; 
			nHeight = _foreinData->m_nWP_H;
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_DesImg]), nWidth, nHeight, sizeof(UCHAR)*3, _foreinData->m_pDesImg);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_DesImgBin]), nWidth, nHeight, sizeof(UCHAR), _foreinData->m_pDesImgBin);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopR]), nWidth, nHeight, sizeof(UCHAR), _foreinData->m_pimgTopR);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopG]), nWidth, nHeight, sizeof(UCHAR), _foreinData->m_pimgTopG);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgTopB]), nWidth, nHeight, sizeof(UCHAR), _foreinData->m_pimgTopB);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_imgWPInspA]), nWidth, nHeight, sizeof(UCHAR), _foreinData->m_pimgWPInspA);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_p3Dm]), nWidth, nHeight, sizeof(float), _foreinData->m_p3D);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgFR]), nWidth, nHeight, sizeof(float), _foreinData->m_pimgFR);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgWP]), nWidth, nHeight, sizeof(float), _foreinData->m_pimgWP);
			PushForeignCpy(GettingPointer(m_qwArrOffset[eForeignData::eForeign_pimgSUB]), nWidth, nHeight, sizeof(int ), _foreinData->m_pimgSUB);
			
		}
		

	}
	void _ForeignDataWP::PushForeign(__in void * Inputptr, int nW, int nH, int nCh, int nMemorySnapShotIndex)// 이물은 한구조체에 이미지를 전부다 넣고 검사 
	{
		if (Inputptr == nullptr) // null 일경우 넣지않음.
			return;
		QWORD old_qwCurPos = _qwCurPos;
		BYTE * buf = _stream->Byte(_qwCurPos);
		IppStatus sts;
		IppiSize iSize = { nW,nH };
		int nSize = 0;
		if (nCh == 1)
		{
			sts = ippiCopy_8u_C1R((BYTE *)Inputptr, nW, buf, nW, iSize);
			nSize = nW * nH;
		}
		else if (nCh == 3)
		{
			sts = ippiCopy_8u_C3R((BYTE *)Inputptr, nW*nCh, buf, nW*nCh, iSize);
			nSize = nW * nH* nCh;
		}
		else
		{
			sts = ippiCopy_32f_C1R((float*)Inputptr, nW * sizeof(float), (float*)buf, nW * sizeof(float), iSize);
			nSize = nW * nH * sizeof(float);
		}
		_qwCurPos += nSize;
		if (nMemorySnapShotIndex != -1)
			m_qwArrOffset[nMemorySnapShotIndex] = old_qwCurPos;
	}
	void _ForeignDataWP::PushForeignCpy(__in void * Inputptr, int nW, int nH, int nCh,__out void * OutputPtr)// 이물은 한구조체에 이미지를 전부다 넣고 검사 
	{
		if (Inputptr == nullptr) // null 일경우 넣지않음.
			return;
			
		IppStatus sts;
		IppiSize iSize = { nW,nH };
		if (nCh == 1)
		{
			sts = ippiCopy_8u_C1R((BYTE *)Inputptr, nW, (BYTE *)OutputPtr, nW, iSize);
		}
		else if (nCh == 3)
		{
			sts = ippiCopy_8u_C3R((BYTE *)Inputptr, nW*nCh, (BYTE *)OutputPtr, nW*nCh, iSize);
		}
		else
		{
			sts = ippiCopy_32f_C1R((float*)Inputptr, nW * sizeof(float), (float*)OutputPtr, nW * sizeof(float), iSize);
		}
	
	}
	void _ForeignDataWP::PointerPush(int nTyoeSize, int nDataSize, void * InputData, int nMemorySnapShotIndex)
	{
		if (InputData == nullptr) // null 일경우 넣지않음.
			return;
		QWORD old_qwCurPos = _qwCurPos;
		BYTE * ptr = _stream->Byte(_qwCurPos);
		int nListSize = (nTyoeSize * nDataSize);
		memcpy(ptr, InputData, nListSize);
		_qwCurPos += nListSize;
		if (nMemorySnapShotIndex != -1)
			m_qwArrOffset[nMemorySnapShotIndex] = old_qwCurPos;

	}
	BYTE * _ForeignDataWP::GettingPointer(QWORD Value)
	{
		if (Value == 0) // NULL 경우는 넣을필요가없습니다.
			return NULL;

		return _stream->Byte(Value);

	}
	bool FrameWriter::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}

		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			bInit &= _stream->AllocMemory(_sizebytes, sName);

		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit &= false;
		return bInit;
	}
	bool FrameWriter::Init(int nFrameIndex)
	{
		CString sLog, sSpaceName;
		sLog.Format(_T("Frame_ImageBuffer Init Start - %d."), nFrameIndex);
		ext::Log::add(sLog);

		bool bInit = true;
		_nBufferIndefx = nFrameIndex;
		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;
		//for (int i = 0; i < _nFamesCnt; i++)
		{
			sSpaceName.Format(_T("AOI_ExtFrame_%d"), nFrameIndex );
			if (Create((1024 * 1024 * 1024 * 1)+100 , sSpaceName, nAccessType) == false)
				bInit = false;
			if (bInit == false)
			{
				sLog.Format(_T("%s_Init_Fail"), sSpaceName);
				ext::Log::add(sLog);
			}
		}
		sLog.Format(_T("Frame_ImageBuffer Init End -  %d."), nFrameIndex);
		ext::Log::add(sLog);

		return bInit;
	}
	bool FrameWriters::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}

		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			bInit &= _stream->AllocMemory(_sizebytes, sName);

		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit &= false;
		return bInit;
	}
	bool FrameWriters::Init()
	{

		CString sLog, sSpaceName;
		bool bInit = true;
		for (int i = 0; i < EXT_FRAMEBUFFER; i++) // 100g..
		{
			
			int nAccessType = FILE_MAP_ALL_ACCESS;
			sSpaceName.Format(_T("AOI_ExtFrameH_%d"),  i); // Header 쪽
			if (Create(sizeof(stFrameHeader) * EXT_FRAMEBUFFER, sSpaceName, nAccessType) == false)
				bInit = false;
			if (bInit == false)
			{
				sLog.Format(_T("%s_Init_Fail"), sSpaceName);
				ext::Log::add(sLog);
			}
			if (_FrameWriter[i].Init(i) == false) //FOV 하나 크기 40개의 1기가 예상
				bInit = false;


		}
		return bInit;

	}
	void FrameWriters::GetData(PUINT8* pFrames,int nSelectBuffer)
	{
		memcpy(_BukHeader, _stream.get()->Byte(0), sizeof(_BukHeader));
		_FrameWriter[nSelectBuffer].GetData(pFrames);
	}
	void FrameWriters::SetData(int nFrameIndex, CString strFrameName, HEADER_BUK bukHeader, int nFrameCnt, PUINT8* pFrame)
	{
		BYTE* pData = _stream.get()->Byte(0);
		_qwCurPos = 0;

		//Hader Write 
		_BukHeader[nFrameIndex].nFrameIndex = nFrameIndex;
		_BukHeader[nFrameIndex].nFrameCnt = nFrameCnt;


		auto& dest = _BukHeader[nFrameIndex].strFrameName; // 배열 참조 유지가 중요
		const size_t cap = sizeof(dest) / sizeof(dest[0]);

		wcsncpy_s(dest, cap, strFrameName /*LPCWSTR or CString*/, _TRUNCATE);

		memcpy(&_BukHeader[nFrameIndex].stHeader, &bukHeader, sizeof(HEADER_BUK));

		_qwCurPos += sizeof(stFrameHeader)* nFrameIndex;
		memcpy(pData + _qwCurPos, &_BukHeader[nFrameIndex], sizeof(stFrameHeader));
		
		//
		//Input Frame

		_FrameWriter[nFrameIndex].SetData(pFrame, nFrameCnt, bukHeader);

		//
		// flush
		_stream->Flush(pData, sizeof(stFrameHeader));

	}
	void FrameWriters::Exit()
	{
		for (int i = 0; i < EXT_FRAMEBUFFER; i++)
		{
			_FrameWriter[i].Exit();
		}
		_stream->UnmapView();
	}
	void FrameWriter::GetData(PUINT8* pFrames)
	{
		*pFrames = reinterpret_cast<PUINT8>(_stream->Byte(0));

	}
	void FrameWriter::SetData(PUINT8* pFrame,int nFrameCnt, HEADER_BUK bukHeader)
	{
		BYTE* pData = _stream.get()->Byte(0);
		int sizeX = bukHeader.sizeX;
		int sizeY = bukHeader.sizeY/ nFrameCnt;
		//_qwCurPos = 0;
		_qwCurPos = (sizeX * sizeY);
		if (pFrame != nullptr)
		{
			for (int i = 0; i < nFrameCnt; i++)
			{
				//IppStatus sts;
				//IppiSize iSize = { sizeX,sizeY };
				//sts = ippiCopy_8u_C1R((BYTE*)pFrame[i], sizeX, pData + _qwCurPos*i, sizeX, iSize);
				////TEST 구문
				std::memcpy(pData + _qwCurPos * i, (BYTE*)pFrame[i], (sizeX * sizeY));
				////
			}
		}

		_stream->Flush();

	}

	void FrameWriter::Exit()
	{
		_stream->UnmapView();
	}
	// MatData 구현
	MatData::MatData()
		: _sizebytes(0)
		, _qwCurPos(0)
		, _nBufferIndex(0)
		, _TotalBuff(0)
	{
	}

	bool MatData::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}

		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			bInit &= _stream->AllocMemory(_sizebytes, sName);

		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit &= false;

		return bInit;
	}

	bool MatData::Init(int nMatIndex)
	{
		CString sLog, sSpaceName;
		sLog.Format(_T("MatData Init Start - %d."), nMatIndex);
		ext::Log::add(sLog);

		bool bInit = true;
		_nBufferIndex = nMatIndex;

		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (ext::EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;

		// 512MB 버퍼 할당
		QWORD nBufferSize = (QWORD)(1024 * 1024 * 512);
		sSpaceName.Format(_T("AOI_ExtMat_%d"), nMatIndex);

		if (Create(nBufferSize, sSpaceName, nAccessType) == false)
		{
			sLog.Format(_T("%s_Init_Fail"), sSpaceName);
			ext::Log::add(sLog);
			bInit = false;
		}

		_TotalBuff = nBufferSize;

		sLog.Format(_T("MatData Init End - %d."), nMatIndex);
		ext::Log::add(sLog);

		return bInit;
	}

	void MatData::Exit()
	{
		if (_stream != nullptr)
			_stream->UnmapView();
	}

	void MatData::SetData(const cv::Mat& mat)
	{
		if (_stream == nullptr || mat.empty())
			return;

		BYTE* pData = _stream->Byte(0);
		size_t dataSize = mat.total() * mat.elemSize();

		if (dataSize > _sizebytes)
		{
			CString sLog;
			sLog.Format(_T("[MatData] Data size overflow: %llu > %llu"), dataSize, _sizebytes);
			ext::Log::add(sLog);
			return;
		}

		if (mat.isContinuous())
		{
			std::memcpy(pData, mat.data, dataSize);
		}
		else
		{
			QWORD offset = 0;
			for (int row = 0; row < mat.rows; row++)
			{
				std::memcpy(pData + offset, mat.ptr(row), mat.cols * mat.elemSize());
				offset += mat.cols * mat.elemSize();
			}
		}

		_stream->Flush();
	}

	void MatData::GetData(cv::Mat& mat, int nWidth, int nHeight, int nType)
	{
		if (_stream == nullptr)
			return;

		
		BYTE* pData = _stream->Byte(0);
		mat = cv::Mat(nHeight, nWidth, nType, pData);
	}

	BYTE* MatData::GetDataPtr()
	{
		if (_stream == nullptr)
			return nullptr;

		return _stream->Byte(0);
	}

	// MatDatas 구현
	MatDatas::Ptr MatDatas::s_Obj;

	MatDatas::Ptr MatDatas::get()
	{
		if (s_Obj == nullptr)
		{
			s_Obj = std::shared_ptr<MatDatas>(new MatDatas());
		}
		return s_Obj;
	}

	MatDatas::MatDatas()
		: _sizebytes(0)
		, _qwCurPos(0)
		, _nMatCount(0)
		, _TotalBuff(0)
	{
		memset(_MatHeader, 0, sizeof(stMatHeader) * EXT_MAT_BUFFER);
	}

	bool MatDatas::Create(QWORD sizebytes, CString sName, DWORD access)
	{
		bool bInit = true;
		if (_stream == nullptr)
		{
			_stream = std::shared_ptr<CSharedMemory>(new CSharedMemory());
		}

		_sizebytes = sizebytes;
		if (_stream->Open(_sizebytes, sName, access) == false)
			bInit &= _stream->AllocMemory(_sizebytes, sName);

		if (_stream->MapView(0, _sizebytes, access) == false)
			bInit &= false;

		return bInit;
	}

	bool MatDatas::Init()
	{
		CString sLog, sSpaceName;
		bool bInit = true;

		ext::Log::add(_T("MatDatas::Init() - Start"));

		int nAccessType = FILE_MAP_ALL_ACCESS;
		if (ext::EnvVariable::nSystemType == EXT_INSP_MONITOR)
			nAccessType = FILE_MAP_READ;

		// 헤더 정보 저장용 공유 메모리
		sSpaceName.Format(_T("AOI_ExtMatHeader"));
		if (Create(sizeof(stMatHeader) * EXT_MAT_BUFFER, sSpaceName, nAccessType) == false)
		{
			sLog.Format(_T("%s_Init_Fail"), sSpaceName);
			ext::Log::add(sLog);
			bInit = false;
		}

		// 각 Mat 버퍼 초기화
		for (int i = 0; i < EXT_MAT_BUFFER; i++)
		{
			if (_MatData[i].Init(i) == false)
				bInit = false;
		}

		_nMatCount = 0;

		ext::Log::add(_T("MatDatas::Init() - End"));

		return bInit;
	}

	void MatDatas::Exit()
	{
		for (int i = 0; i < EXT_MAT_BUFFER; i++)
		{
			_MatData[i].Exit();
		}

		if (_stream != nullptr)
			_stream->UnmapView();
	}

	void MatDatas::Clear()
	{
		_nMatCount = 0;
		memset(_MatHeader, 0, sizeof(stMatHeader) * EXT_MAT_BUFFER);

		if (_stream != nullptr)
		{
			BYTE* pData = _stream->Byte(0);
			memset(pData, 0, sizeof(stMatHeader) * EXT_MAT_BUFFER);
			_stream->Flush();
		}
	}

	bool MatDatas::SetData(int nMatIndex, int nPartID, const cv::Mat& mat)
	{
		if (nMatIndex < 0 || nMatIndex >= EXT_MAT_BUFFER)
		{
			CString sLog;
			sLog.Format(_T("[EXT MatDatas] Invalid index: %d"), nMatIndex);
			ext::Log::add(sLog);
			return false;
		}

		if (mat.empty())
		{
			ext::Log::add(_T("[EXT MatDatas] Empty Mat"));
			return false;
		}

		// 헤더 정보 설정
		_MatHeader[nMatIndex].nMatIndex = nMatIndex;
		_MatHeader[nMatIndex].nPartID = nPartID;
		_MatHeader[nMatIndex].nWidth = mat.cols;
		_MatHeader[nMatIndex].nHeight = mat.rows;
		_MatHeader[nMatIndex].nChannels = mat.channels();
		_MatHeader[nMatIndex].nType = mat.type();
		_MatHeader[nMatIndex].qwDataSize = mat.total() * mat.elemSize();

		// 헤더 정보를 공유 메모리에 저장
		BYTE* pHeaderData = _stream->Byte(0);
		_qwCurPos = sizeof(stMatHeader) * nMatIndex;
		memcpy(pHeaderData + _qwCurPos, &_MatHeader[nMatIndex], sizeof(stMatHeader));
		_stream->Flush();

		// Mat 데이터 저장
		_MatData[nMatIndex].SetData(mat);

		if (nMatIndex >= _nMatCount)
			_nMatCount = nMatIndex + 1;

		CString sLog;
		sLog.Format(_T("[EXT MatDatas] SetData - Index:%d, PartID:%d, Size:%dx%d, Type:%d"),
			nMatIndex, nPartID, mat.cols, mat.rows, mat.type());
		ext::Log::add(sLog);

		return true;
	}

	bool MatDatas::GetData(int nMatIndex, cv::Mat& mat)
	{
		if (nMatIndex < 0 || nMatIndex >= EXT_MAT_BUFFER)
		{
			CString sLog;
			sLog.Format(_T("[EXT MatDatas] GetData - Invalid index: %d"), nMatIndex);
			ext::Log::add(sLog);
			return false;
		}

		// 헤더 정보 읽기
		BYTE* pHeaderData = _stream->Byte(0);
		memcpy(_MatHeader, pHeaderData, sizeof(stMatHeader) * EXT_MAT_BUFFER);

		stMatHeader& header = _MatHeader[nMatIndex];

		if (header.nWidth <= 0 || header.nHeight <= 0)
		{
			ext::Log::add(_T("[EXT MatDatas] GetData - Invalid header"));
			return false;
		}

		// Mat 데이터 읽기
		_MatData[nMatIndex].GetData(mat, header.nWidth, header.nHeight, header.nType);

		return true;
	}

	bool MatDatas::GetDataByPartID(int nPartID, cv::Mat& mat)
	{
		int nIndex = FindIndexByPartID(nPartID);
		if (nIndex < 0)
		{
			//CString sLog;
			//sLog.Format(_T("[MatDatas] GetDataByPartID - PartID not found: %d"), nPartID);
			//ext::Log::add(sLog);
			return false;
		}

		return GetData(nIndex, mat);
	}

	int MatDatas::FindIndexByPartID(int nPartID)
	{
		// 헤더 정보 읽기
		BYTE* pHeaderData = _stream->Byte(0);
		memcpy(_MatHeader, pHeaderData, sizeof(stMatHeader) * EXT_MAT_BUFFER);

		// PartID로 검색
		for (int i = 0; i < EXT_MAT_BUFFER; i++)
		{
			if (_MatHeader[i].nPartID == nPartID && _MatHeader[i].nWidth > 0)
			{
				return i;
			}
		}

		return -1;  // 찾지 못함
	}

	stMatHeader MatDatas::GetHeader(int nMatIndex)
	{
		if (nMatIndex < 0 || nMatIndex >= EXT_MAT_BUFFER)
			return stMatHeader();

		// 헤더 정보 읽기
		BYTE* pHeaderData = _stream->Byte(0);
		memcpy(_MatHeader, pHeaderData, sizeof(stMatHeader) * EXT_MAT_BUFFER);

		return _MatHeader[nMatIndex];
	}
}
