#include "stdafx.h"
#include <windows.h>
#include "ExtInspLogic.h"
#include "MPTI.h"
#include "ExtTools.h"
#include "QTimer.h"
#include "psapi.h"
#include <tlhelp32.h>
#include <csignal>
#include <signal.h>
namespace ext
{
	InspLogicRoot::Ptr InspLogicRoot::s_Obj;

	InspLogicRoot::Ptr InspLogicRoot::get()
	{
		if (s_Obj == nullptr)
		{
			s_Obj = std::shared_ptr<InspLogicRoot>(new InspLogicRoot());
		}
		return s_Obj;
	}

	bool InspLogicRoot::Init()
	{
		_Insp.Init();
		return true;
	}
	bool InspLogicRoot::Init(int n)
	{

		_Insp.Init(n);
		return true;
	}
	void InspLogicRoot::Exit()
	{
		_Insp.Exit();
	}

	void InspLogic::Begin()
	{
		_nMainSeqStep = SeqCase::ReadyToAct;
		ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvAlive, true);

		//InspPartInfo * pDstPart;
		//InspPartParam * pDstWindow;
		//	
		////ext::ClientCtrl::get()->_PartData[0].GetPartData(*&pDstPart, *&pDstWindow);
		////ext::ClientCtrl::get()->_PartData[0].GetLight(*&pDstWindow, pDstPart->nWindowCount);
		//ext::ClientCtrl::get()->GetSharedData(*&pDstPart, *&pDstWindow, 0);

 	//	 cv::Mat CR(pDstPart->partImgBuf.nImageSizeY,pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgTop_R);
 	//	 cv::Mat CB(pDstPart->partImgBuf.nImageSizeY,pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgTop_B);
 	//	 cv::Mat CG(pDstPart->partImgBuf.nImageSizeY,pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgTop_G);
 	//	 cv::Mat CW(pDstPart->partImgBuf.nImageSizeY,pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgTop_W);
 	//	 cv::Mat CMR(pDstPart->partImgBuf.nImageSizeY,pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgMiddle_R);
 	//	 cv::Mat CMB(pDstPart->partImgBuf.nImageSizeY,pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgMiddle_B);
 	//	 cv::Mat CBR(pDstPart->partImgBuf.nImageSizeY,pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgBottom_R);
 	//	 cv::Mat CBB(pDstPart->partImgBuf.nImageSizeY,pDstPart->partImgBuf.nImageSizeX, CV_8UC1, pDstPart->partImgBuf.imgBottom_B);
 	//	 cv::Mat CB3D(pDstPart->partImgBuf.nImageSizeY, pDstPart->partImgBuf.nImageSizeX, CV_32FC1, pDstPart->partZmapData.data);
 		 
	}
	void InspLogic::End()
	{
		ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvAlive, false);

	}

	void InspLogic::Proc()
	{

 		CtrlServer * ctrlServer = ext::irc::get()->_CtrlServer.First();
 
		ext::ClientCtrl::get()->_ctrl.First()->nMainAutoSeqStep = _nMainSeqStep;
 		
 		if (InspLogic::CheckEmergencyStop() == true)
 		{
 			NextStep(SeqCase::ReadyToAct);
 			return;
 		}
 		
 
 		switch (_nMainSeqStep)
 		{
 		case SeqCase::ReadyToAct:
			if (g_pMPTI->m_InspMng->m_bMultiProcessThreadSave)
			{
				ext::ClientCtrl::get()->_ctrl.First()->nImageSaveCnt = g_pMPTI->m_InspMng->m_ImgStack.Count();
				Sleep(1);
			}
 			if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAlive) == true ||
 				ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAutoMode) == false )
 			{
 				Ready();
 				NextStep(SeqCase::CheckToAuto);
 			}
 			break;
 		case SeqCase::CheckToAuto:
 			if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAutoMode) == true)
 			{
				if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstPreStart) == true)
				{
					// Auto 검사중에 바꾼다고 신호가 들어올때 대비.
					NextStep(SeqCase::PreStart);
					break;
				}
 				//Log Init
 				memset(g_pMPTI->m_InspMng->m_dAlgoListTack, 0, sizeof(double) * eAlgoNum);
				memset(g_pMPTI->m_InspMng->m_nAlgoCnt, 0, sizeof(int) * eAlgoNum);
				memset(g_pMPTI->m_InspMng->m_dSectorLog, 0, sizeof(double) * 10);
 				ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvReady, false, ExtNoUpdate);
 				ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvAutoMode, true);
 
 				g_pMPTI->m_InspMng->SetExtProductionInfo();
 
 				ext::Log::add(_T("SeqCase::CheckToAuto"));
 				NextStep(SeqCase::InspProc);
				break;
 			}
			if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstPreStart) == true)
			{
				NextStep(SeqCase::PreStart);
			}
 			break;
		case SeqCase::PreStart:
			if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstPreStart) == false)
			{
				ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvPreStartDone, false, true);
				NextStep(SeqCase::CheckToAuto);
			}
			else if(ext::ClientCtrl::get()->_ctrl.GetSignal(eSlvSignal::eSlvPreStartDone) == false)
			{
				PreStarting();
			}
			break;
 		case SeqCase::InspProc:
 		case SeqCase::InspProc1:
 		case SeqCase::InspProc2:
 		case SeqCase::InspProc3:
 		case SeqCase::InspProc4:
 		case SeqCase::InspProc5:
 		{
 
 			//Inspect();		
 			Inspect2();		
			ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvAutoMode, false);
 
 			int nAuto = ext::ClientCtrl::get()->_ctrl.GetSignal(eSlvSignal::eSlvAutoMode);
 			
 			ext::Log::add(_T("SeqCase::InspProc - End"));
 
 			NextStep(SeqCase::CheckToFinish);
 			break;
 		}
 
 		case SeqCase::CheckToFinish:
 			if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAutoMode) == false ||
 				ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstStop) == true ||
 				ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAlive) == false ||
 				ext::IsProcessRunning(ext::irc::get()->_CtrlServer.First()->dwProcessID)==false)
 			{
 				ext::Log::add(_T("SeqCase::CheckToFinish"));
 				
 				NextStep(SeqCase::ReadyToAct);
 			}
 			break;
 		}
	}

	bool InspLogic::CheckEmergencyStop()
	{
		if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAlive) == true)
		{
			if (!ext::IsProcessRunning(ext::irc::get()->_CtrlServer.First()->dwProcessID))
				return true;
		}

		return false;
	}

	void InspLogic::Ready()
	{
		ext::Log::add(_T("SeqCase::ReadyToAct"));
		ext::ClientCtrl::get()->_ctrl.First()->CurrCalcPartIdx = 0;
		ext::ClientCtrl::get()->_ctrl.First()->CalcPartCount = 0;
		ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvAutoMode, false);
		ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvReady, true);
		//ext::irc::get()->_Result.Clear();
	}

	bool InspLogic::Inspect2()
	{
		g_pMPTI->m_InspMng->ClearProcessedAIPartID();
		ext::IsProcessRunningMemoryView();
		if(ext::EnvVariable::nTool_id  == ext::EnvVariable::nToolCount) // 마지막 툴만 Eagle3D state Log남김
			ext::IsMainProcessCheck();
		//ext::ClientCtrl::get()->_ctrl.First()->nMainAutoSeqStep = _nMainSeqStep;
		CString strLogStack;
		strLogStack.Format(_T("SeqCase::ImageStack Start"));
		ext::Log::add(strLogStack);

		if (g_pMPTI->m_InspMng->m_bMultiProcessThreadSave)
			g_pMPTI->m_InspMng->m_ImgStack.ImageBufferClear(); // Stack Count 만 초기화

		if (g_pMPTI->m_InspMng->m_bMultiProcessThreadSave)
		{
			while (true)
			{
				if (g_pMPTI->m_InspMng->m_ImgStack.Count() != 0)
				{

					strLogStack.Format(_T("[STACK-S] %d"), g_pMPTI->m_InspMng->m_ImgStack.Count());  // strLogStack.Format(_T("SeqCase::ImageStack Count  = %d"), g_pMPTI->m_InspMng->m_ImgStack.Count());
					ext::Log::add(strLogStack);
					Sleep(1);
				}
				else
				{
					strLogStack.Format(_T("STACK-S END"));
					ext::Log::add(strLogStack);
					break;
				}

			}
		}

		ext::Log::add(_T("SeqCase::InspProc - Begin"));

		int NumOfClients(ext::irc::get()->_CtrlServer.First()->NumOfClients);

		int nCalcPartCnt = 0;
		int nOwnToolid = ext::env::nTool_id - 1;
		int nOwnToolType = ext::ClientCtrl::get()->m_nToolType;
		CString slog;
		CString strInspectStart = _T("");
		CQTimer qtm, qtmPart, qtmWait,qtmSaveImage;
		double dAlgoTack = 0, dPartTack = 0, dTotalTack = 0, dWaitTack = 0,dPartSaveTack = 0 ;
		double dForeignTack = 0 ;
		double dSaveImageTack = 0;
		bool bHalconLoad = false;
		CQTimer qtm1, qtm2, qtm3, qtm4, qtm5, qtm6, qtmEagleCheck;
		double dTack1 = 0, dTack2 = 0, dTack3 = 0, dTack4 = 0, dTack5 = 0,dTack6 = 0, dTackEagleCheck = 0;
		int nSleepInterval = 0;

		NextStep(SeqCase::InspProc1);
		int nBuffRstIdx = 0;
		int nBuffIdx = 0;

		int nBuffForeignRstIdx = 0;
		int nBuffForeignIdx = 0;

		int nMaxBufferCnt = EXT_BUFFER_CNT;
		int nMaxRstBufferCnt = EXT_BUFFER_RST_CNT;
	
		
		int nMaxForeignBufferCnt = EXT_BUFFER_FOREIGN_CNT;
		int nMaxForeignRstBufferCnt = EXT_BUFFER_FOREIGN_RST_CNT;
		switch (nOwnToolType)
		{
		case (int)ext::eToolType::eNORMAL:
			break;
		case (int)ext::eToolType::eBIG:
			nMaxBufferCnt = EXT_BIG_BUFFER_CNT;
			nMaxRstBufferCnt = EXT_BIG_BUFFER_RST_CNT;
			break;
		case (int)ext::eToolType::eForeignType: 
			break;
		}

		while(true)
		{
			if (nSleepInterval == 100)
			{
				Sleep(1);
				nSleepInterval = 0;
			}
			else
				Sleep(0);

			qtmWait.StartTick();
			
			qtm1.StartTick();

			nBuffIdx %= nMaxBufferCnt;
			nBuffRstIdx %= nMaxRstBufferCnt;

			bool bCalc = false;
			bool bCalcForeign = false;

			if (TryInspection())
				break;
			
			double dT1 = qtm1.EndTick()* 1000.0;
			qtm2.StartTick();
			
			if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstStop) == true ||
				ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAlive) == false ||
				ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAutoMode) == false)
			{
				Sleep(5); // ext::irs::get()->_Ctrl.InitSignal(); 시 초기화 되서 다시 재확인 
				if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstStop) == true ||
					ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAlive) == false ||
					ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAutoMode) == false)
				{
					int nFlag = -1;
					if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstStop) == true)
						nFlag = 0;
					else if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAlive) == false)
						nFlag = 1;
					else if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAutoMode) == false)
						nFlag = 2;
					dTack2 += qtm2.EndTick() * 1000.0;
					int nTotalDoneCnt = ext::irc::get()->_CtrlServer.First()->nInspDoneCnt;
					int nSkipPartCnt = ext::irc::get()->_CtrlServer.First()->nInspSkipCnt;
					int nRealPartCnt = ext::irc::get()->_CtrlServer.First()->prod.partTotalCount - nSkipPartCnt;
					slog.Format(_T("End Point2,Status = %d , nTotalDoneCnt = %d ,nSkipPartCnt =%d , nRealPartCnt =%d "), nFlag, nTotalDoneCnt, nSkipPartCnt, nRealPartCnt);
					ext::Log::add(slog);
				break;
				}
			}
			double dT2 = qtm2.EndTick() * 1000.0;

			qtm3.StartTick();
			NextStep(SeqCase::InspProc2);
			ext::ClientCtrl::get()->_ctrl.First()->nMainAutoSeqStep = _nMainSeqStep;

			bool bFlag = false;
			bool bFlagForeign = false;
			double dT3 = qtm3.EndTick() * 1000.0;

			qtm4.StartTick();
		
	
			int nForeignNextStep = 0;
			if (nOwnToolType == (int)ext::eToolType::eForeignType)
			{
				int nSizeVector = ext::ClientCtrl::get()->InspCalcFov.size();
				nForeignNextStep  = ext::ClientCtrl::get()->InspCalcFov[nBuffForeignIdx];
				if (nSizeVector <= nBuffForeignIdx && ext::irc::get()->_CtrlServer.First()->prod.nFovTotalCnt <= nForeignNextStep)
					continue;
				bFlagForeign = ext::ClientCtrl::get()->_SndForeignCtrl[nForeignNextStep].GetFlag(ext::BufferFlag::IF_ON);
			}
			else
				bFlag = ext::ClientCtrl::get()->_SndCtrl[nBuffIdx].GetFlag(ext::BufferFlag::IF_ON); // Big Part 든 normal Part 든
			dTack3 += qtm3.EndTick() * 1000.0;
			double dT4 = qtm4.EndTick() * 1000.0;

			qtm5.StartTick();
			if (bFlag == true)
			{
				NextStep(SeqCase::InspProc3);

				slog.Format(_T("IN:%d"), nBuffIdx);//Input Buffer 
				ext::Log::add(slog);
				//slog.Format(_T("Buffer Rstidx = %d"), nBuffRstIdx);
				//ext::Log::add(slog);
			/*	slog.Format(_T("Buffer Rst idx = %d"), nBuffRstIdx);
				ext::Log::add(slog);*/

				if (nOwnToolType == (int)ext::eToolType::eBIG && bHalconLoad == false)
				{ 
					// Buffer On 이 될때 Halcon Model Load
					int nTotalCnt = ext::InspRoot_Server::get()->_InspStatus.ModelTotalCount(ext::ModelBufferFlag::eExtModel_HalconPOCR); // COB안에 Add 된 Pattern or Mat 갯수 
					std::vector<PIAL::OCRFilePath> m_vsFilePath;
					for (int i = 0; i < nTotalCnt; i++)
					{
						ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_HalconPOCRModelCtrl.First()[i];
						CString curFile = pCtrl.sFilePath;
						CString strLogHalcon = _T("");
						strLogHalcon.Format(_T("Halcon OCR Model Load : %s"), curFile);
						ext::Log::add(strLogHalcon);
						if (curFile.Find(_T("HalconOCR")) >= 0)
						{
							PIAL::OCRFilePath tempOCRPath;
							memcpy(tempOCRPath.FilePath, pCtrl.sFilePath, sizeof(wchar_t) * MAX_STRLEN);
							m_vsFilePath.push_back(tempOCRPath);
							//if (ext::InspRoot_Server::get()->bUseMultiProcess)
							//	ext::InspRoot_Server::get()->ExtModelAdd(nullptr, Path[i].m_sPathModelPath, ext::ModelBufferFlag::eExtModel_HalconPOCR);
						}
					}
					g_pMPTI->m_InspMng->m_PInspWrapper->m_PInspAlgo->PInspAlgo::SetHalconFolderPath(m_vsFilePath);
					int check = g_pMPTI->m_InspMng->m_PInspWrapper->m_PInspAlgo->PInspAlgo::CheckHalconLicense();


					if (check != 1)
					{
						if (g_pMPTI)
						{
							CString sLog = _T("");
							sLog.Format(_T("POCR - HALCON License Error. (error code: %d)"), check);
							ext::Log::add(sLog);
						}
					}
					else
					{
						CString sLog = _T("");
						sLog.Format(_T("POCR - HALCON License Pass "), check);
						ext::Log::add(sLog);
					}
					bHalconLoad = true; // 한번만 할수있도록
				}

				bCalc = true;
			}
			else if (bFlagForeign) //이물 검사 Flag 
			{
				bCalcForeign = true;
				slog.Format(_T("ForeignBuffer idx = %d"), nForeignNextStep);
				ext::Log::add(slog);
			}
			else
			{
				dTack1 += dT1;
				dTack2 += dT2;
				dTack3 += dT3;
				dTack4 += dT4;
				dTack5 += qtm5.EndTick() * 1000.0;
				dWaitTack += qtmWait.EndTick() * 1000.0;
				nSleepInterval++;
				continue;
			}
			int partID = 0;
			//자신에게 할당된 파트 데이터까지 왔음
	
			if (bCalc == true)
			{
				if (g_pMPTI->m_InspMng->m_bMultiProcessThreadSave)
				{
					slog.Format(_T("[STACK-C]")); // Buffer Check 
					ext::Log::add(slog);
					while (true)
					{
						bool bPass = g_pMPTI->m_InspMng->m_ImgStack.Contains(nBuffIdx); // 존재하면 기다려줘야함.. 버퍼한바퀴돌고 기다리는거라..

						if (bPass)
							Sleep(1);
						else
							break;
					}

					slog.Format(_T("[STACK-C-End]")); // Buffer Check 
					ext::Log::add(slog);
				}
				
				qtmEagleCheck.StartTick();
				if (!ext::IsProcessRunning(ext::irc::get()->_CtrlServer.First()->dwProcessID))
				{
					slog.Format(_T("End Point1"));
					ext::Log::add(slog);
					return false;
				}
				dTackEagleCheck += qtmEagleCheck.EndTick() * 1000.0;

				qtmPart.StartTick();
				NextStep(SeqCase::InspProc4);

				if (nBuffIdx < 0)
				{
					slog.Format(_T("Tool%d, Fail find buffer."), env::nTool_id);
					ext::Log::add(ext::Format(slog));
					continue;;
				}

				NextStep(SeqCase::InspProc4);

				//  전달사항 앞에서 생성해도 널로 초기화
				InspPartInfo * pDstPart = nullptr;
				InspPartParam * pDstWindow = nullptr;
				pDstPart = new InspPartInfo();
				pDstWindow = new InspPartParam();

				ext::ClientCtrl::get()->_SndCtrl[nBuffIdx].SetFlag(ext::BufferFlag::IF_CALC, true);
				g_pMPTI->m_InspMng->SetExtInspParam(pDstPart, pDstWindow, nBuffIdx);

				qtm.StartTick();
				/*			slog.Format(_T("StartInsp"));
							ext::Log::add(slog);*/
				int ret = g_pMPTI->m_InspMng->InspNormal_Ver2(true);
				double dPartInspectionTime = qtm.EndTick() * 1000.0;
				/*	slog.Format(_T("EndInsp"));
					ext::Log::add(slog);*/
				dAlgoTack += dPartInspectionTime;

				//스케줄링을 위한 검사 속도갱신
				dPartTack = g_pMPTI->m_InspMng->m_inspectionResult->m_dInspTime = dPartInspectionTime;

				g_pMPTI->m_InspMng->m_inspectionResult->m_nPartID = g_pMPTI->m_InspMng->m_pInspBoardInfo->nPartIDOrg;
				partID = g_pMPTI->m_InspMng->m_inspectionResult->m_nPartID;

				//slog.Format(_T("Part RstID = %d, Defect Code = %d"), g_pMPTI->m_InspMng->m_pInspBoardInfo->nPartIDOrg, g_pMPTI->m_InspMng->m_inspectionResult->nDefectType);
				//ext::Log::add(slog);
				// Image Save
				qtmSaveImage.StartTick();
				g_pMPTI->m_InspMng->SaveInspPartImage(&strInspectStart, pDstPart, pDstWindow, nBuffIdx); //Image Save 이후 UnLock
				if (g_pMPTI->m_InspMng->m_bMultiProcessThreadSave)
				{
					strLogStack.Format(_T("[STACK-E] %d"), g_pMPTI->m_InspMng->m_ImgStack.Count());
					ext::Log::add(strLogStack);
				}
				
				dPartSaveTack = qtmSaveImage.EndTick() * 1000.0;
				dSaveImageTack += dPartSaveTack;
				/*slog.Format(_T("StartResult"));
				ext::Log::add(slog);*/
				// 다음 검사를 위해 Send Flag OFF로 바꿔줘야 Eagle3D에서 다음 검사 버퍼를 세팅할 수 있음.
				qtm6.StartTick();
				while (true)
				{
					if (!ext::IsProcessRunning(ext::irc::get()->_CtrlServer.First()->dwProcessID))
						break;

					if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstStop) == true ||
						ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAutoMode | eMstAlive) == false)
						break;
					bool bBreak = ext::ClientCtrl::get()->_RevCtrl[nBuffRstIdx].GetFlag(ext::BufferFlag::IF_OFF);

					if (bBreak)
					{
				
						break;
					}
				}
				dTack6 += qtm6.EndTick() * 1000.0;


				ext::ClientCtrl::get()->_Rst[nBuffRstIdx].AddResult(g_pMPTI->m_InspMng->m_inspectionResult, ret);
				slog.Format(_T("Code:%d,%d,%lld,%d"), g_pMPTI->m_InspMng->m_inspectionResult->nDefectType,//nDefectType, m_nDefectWnd,m_nDefectAlgo ,nDefectCode
					g_pMPTI->m_InspMng->m_inspectionResult->m_nDefectWnd,
					g_pMPTI->m_InspMng->m_inspectionResult->m_nDefectAlgo,
					ret);
				ext::Log::add(slog, ext::m_eLogLv::Lv1);
				g_pMPTI->m_InspMng->DeleteResultVal(g_pMPTI->m_InspMng->m_inspectionResult);
				if (!g_pMPTI->m_InspMng->m_bMultiProcessThreadSave)
				{
					//Image Buffer 관장하는 Flag Off로 변경시 이미지 버퍼, InspData 채우므로 MultiSave 를사용할때는 예외 Save Class 에서 Off진행
					ext::ClientCtrl::get()->_SndCtrl[nBuffIdx].SetFlag(ext::BufferFlag::IF_OFF, true);
				}
					


				ext::ClientCtrl::get()->_RevCtrl[nBuffRstIdx].First()->nCalcDoneCount++;
				ext::ClientCtrl::get()->_RevCtrl[nBuffRstIdx].SetFlag(ext::BufferFlag::IF_ON, true);
				ext::ClientCtrl::get()->_ctrl.First()->CalcPartCount++;
				ext::ClientCtrl::get()->_ctrl.UpdateAll();

				//slog.Format(_T("InspComplete CalcPartCnt = %d"), ext::ClientCtrl::get()->_ctrl.First()->CalcPartCount);
				//ext::Log::add(slog);
				//검사 완료 갯수 갱신
				nCalcPartCnt++;
				nBuffIdx++;
				nBuffRstIdx++;
				if (!g_pMPTI->m_InspMng->m_bMultiProcessThreadSave)
				{
					if (pDstPart) { delete pDstPart; pDstPart = nullptr; }
					if (pDstWindow) { delete pDstWindow; pDstWindow = nullptr; }
				}
				
			}
			if (nOwnToolType == (int)ext::eToolType::eForeignType )
			{
				//이물검사 
				if (bCalcForeign == true)
				{

					qtmEagleCheck.StartTick();
					if (!ext::IsProcessRunning(ext::irc::get()->_CtrlServer.First()->dwProcessID))
					{
						slog.Format(_T("End Point1"));
						ext::Log::add(slog);
						return false;
					}
					dTackEagleCheck += qtmEagleCheck.EndTick() * 1000.0;


					//Insp Calc State Change 
					ext::ClientCtrl::get()->_SndForeignCtrl[nForeignNextStep].SetFlag(ext::BufferFlag::IF_CALC, true);
			 
					//  전달사항 앞에서 생성해도 널로 초기화

					InspPartInfo * _ForeignInspInfo = nullptr; 
					ForeignData * _ForeignData = nullptr;
					InspForeignInfo * _ForeignInfoData = nullptr;
					ForeignParamROI * _ForeParamROI = nullptr;
					ForeignParamROI * _ArrROI = nullptr;
					FR2DData * _Fr2Data = nullptr;
					InspFovForeignResult* _retResult = nullptr; 
					_ForeignInspInfo = new InspPartInfo();
					_ForeignData = new ForeignData();
					_Fr2Data = new FR2DData();
					_ForeignInfoData = new InspForeignInfo();
					//_ForeParamROI = new ForeignParamROI(); //
					//_ArrROI = new ForeignParamROI();
					//
					//Insp Data Set
					g_pMPTI->m_InspMng->GetExtForeignInspParam(_ForeignInspInfo, _ForeignData, _Fr2Data, _ForeignInfoData, &_ForeParamROI, &_ArrROI, nForeignNextStep);
					//Inspection


					qtm.StartTick();
					int ret = g_pMPTI->m_InspMng->Inspection_Foreign_new(*_ForeignData, *_Fr2Data, _ArrROI);
					dForeignTack += qtm.EndTick() * 1000.0;
					
					
					//InspFovForeignResult * _ForeRest = new InspFovForeignResult(); 
					
					//delete foreignRes;/
					//Eagle3D Running Check 
					_retResult = new InspFovForeignResult();
					qtm6.StartTick();
					while (true)
					{
						if (!ext::IsProcessRunning(ext::irc::get()->_CtrlServer.First()->dwProcessID))
							break;

						if (ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstStop) == true ||
							ext::irc::get()->_CtrlServer.GetSignal(eMstSignal::eMstAutoMode | eMstAlive) == false)
							break;
						bool bBreak = ext::ClientCtrl::get()->_RevForeignCtrl[nForeignNextStep].GetFlag(ext::BufferFlag::IF_OFF);
						// Flag OFF(빈공간) 상태에서만 Write 가능 무한루프 가능성으로 확인이 필요 
						if (bBreak)
						{
							g_pMPTI->m_InspMng->m_FR.GetInspForeignResult(_retResult);
							ext::ClientCtrl::get()->_ForeignSheradRst[nForeignNextStep].AddForeignRstFovForeign(_retResult, ret);
							ext::ClientCtrl::get()->_RevForeignCtrl[nForeignNextStep].SetFlag(ext::BufferFlag::IF_CALCEND, true);
						//	ext::ClientCtrl::get()->_SndForeignCtrl[nBuffIdx].SetFlag(ext::BufferFlag::IF_CALCEND, true); //검사까지 끝났으니 가져가도된다라는 뜻 
							break;
						}
					}
					
					dTack6 += qtm6.EndTick() * 1000.0;
					
					ext::ClientCtrl::get()->_RevForeignCtrl[nForeignNextStep].First()->nCalcDoneCount++; // 
					nBuffForeignRstIdx++;
					nBuffForeignIdx++; // 검사를 하지않으면 증가되지않음.

					if (_ForeignInspInfo) { delete _ForeignInspInfo; _ForeignInspInfo = nullptr; }
					if (_ForeignData) { delete _ForeignData; _ForeignData = nullptr; }
					if (_Fr2Data) { delete _Fr2Data; _Fr2Data = nullptr; }
					if (_ForeignInfoData) { delete _ForeignInfoData; _ForeignInfoData = nullptr; }
					//if (_ForeParamROI) { delete _ForeParamROI; _ForeParamROI = nullptr; }
					//if (_retResult) { delete _retResult; _retResult = nullptr; }

					
				}

			}
			//TackTime
			//dPartTack = qtmPart.EndTick() * 1000.0;
			//dTotalTack += dPartTack;

			if (partID != 0)
			{
				slog.Format(_T("PR:%d,%d,%d"), partID, (int)round(dPartTack), (int)round(dPartSaveTack) );//PartID,InspectionTime,SaveTime
				ext::Log::add(slog, ext::m_eLogLv::Lv1);
			}

			NextStep(SeqCase::InspProc6);
		}
		CString strLogTemp;
		if (nOwnToolType == (int)ext::eToolType::eForeignType)
		{
			double dFAvgTack = dForeignTack / (double)nBuffForeignIdx;
			strLogTemp.Format(_T("Inspection_Foreign_new,TotalCnt,%d,Avg,%d,Total,%d, "), nBuffForeignIdx, (int)dFAvgTack, (int)dForeignTack);
			ext::Log::add(ext::Format(strLogTemp));
		}
		for (int AlgoLoop = 0; AlgoLoop < eAlgoNum; AlgoLoop++)
		{
			if (g_pMPTI->m_InspMng->m_dAlgoListTack[AlgoLoop] != 0.)
			{
				double dAvgTact = ((double)g_pMPTI->m_InspMng->m_dAlgoListTack[AlgoLoop] / (double)g_pMPTI->m_InspMng->m_nAlgoCnt[AlgoLoop]) * 1000.0;
				strLogTemp.Format(_T("Algorithm Type: %d / Tack: %f / Avg %d ms / Cnt %d "), AlgoLoop, g_pMPTI->m_InspMng->m_dAlgoListTack[AlgoLoop] , (int)round(dAvgTact) , g_pMPTI->m_InspMng->m_nAlgoCnt[AlgoLoop]);
				ext::Log::add(ext::Format(strLogTemp));
			}
		}
		for (int nSector = 0; nSector < 10; nSector++)
		{
			if (g_pMPTI->m_InspMng->m_dSectorLog[nSector] != 0.)
			{
				strLogTemp.Format(_T("Sector : %d / Tack: %f "), nSector, g_pMPTI->m_InspMng->m_dSectorLog[nSector]);
				ext::Log::add(ext::Format(strLogTemp));
			}
		}
		int nTotalAsr = 0;
		if (!(dTotalTack == 0 && nCalcPartCnt == 0))
			nTotalAsr = (int)round((double)(dTotalTack / nCalcPartCnt));
			
		slog.Format(_T("AvgTack,%d,TotalTack,%d,TotalCnt,%d,InspAlgoTime,%d,WaitTack,%d,ResultOffTack,%d, EagleCheckTack, %d,(ms)"), nTotalAsr, (int)round(dTotalTack), (int)round(nCalcPartCnt), (int)round(dAlgoTack), (int)round(dWaitTack), (int)round(dTack6), (int)round(dTackEagleCheck));
		ext::Log::add(ext::Format(slog));

		slog.Format(_T("Tack1,%d,Tack2,%d,Tack3,%d,Tack4,%d,Tack5,%d, (ms)"), (int)round(dTack1), (int)round(dTack2), (int)round(dTack3), (int)round(dTack4), (int)round(dTack5));
		ext::Log::add(ext::Format(slog));

		slog.Format(_T("dSaveImageTack %d ,(ms), InspectStart,%s "), (int)round(dSaveImageTack), strInspectStart);
		ext::Log::add(ext::Format(slog));
	
		ext::ClientCtrl::get()->_ctrl.First()->nImageSaveCnt = g_pMPTI->m_InspMng->m_ImgStack.Count();
		
	}
	bool InspLogic::TryInspection()
	{
		bool bResult = false; 
		int nPartCount = ext::irc::get()->_CtrlServer.First()->prod.partTotalCount;
		int nTotalDoneCnt = ext::irc::get()->_CtrlServer.First()->nInspDoneCnt;
		int nSkipPartCnt = ext::irc::get()->_CtrlServer.First()->nInspSkipCnt;
		int nRealPartCnt = nPartCount - nSkipPartCnt;

		bResult = nTotalDoneCnt == nRealPartCnt;
		bool bCalcForeign = true; 
		if (ext::ClientCtrl::get()->m_nToolType == (int)ext::eToolType::eForeignType) //Foreign 검사Tool에서만 진행
		{
			int nTotalDoneCnt = ext::irc::get()->_CtrlServer.First()->prod.nFovTotalCnt; //  총갯수
			int nForeignDoneCnt = ext::irc::get()->_CtrlServer.First()->nInspForeignDoneCnt; // 가져올때 서버에서 증가함
			int nForeignSkipPartCnt = ext::irc::get()->_CtrlServer.First()->nInspForeignSkipCnt; // BadMark 시에만,

			int nRealCnt = nTotalDoneCnt - nForeignSkipPartCnt;
			if (nRealCnt <= 0)// Slave 일경우
				bCalcForeign = true; 
			else
				bCalcForeign = (nForeignDoneCnt == (nRealCnt));
				
		}

		return bResult && bCalcForeign;

	}
	bool IsProcessRunning(DWORD pid)
	{
		HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
		DWORD ret = WaitForSingleObject(process, 0);
		CloseHandle(process);
		return ret == WAIT_TIMEOUT;
	}

	bool IsProcessRunningMemoryView()
	{
		//전체 Virtual Memory:
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);

		DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
		//지금 쓰고 있는 Virtual Memory :

		DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
		
		//이 프로세스가 쓰고 있는 Virtual Memory:

		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc));
		SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

		

		//전체 Physical Memory (RAM): 전체 Virtual Memory에 다음을 추가합니다
		DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
		//지금 쓰고 있는 Physical Memory : 전체 Virtual Memory에 다음을 추가합니다
		DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys; 
		//이 프로세스가 쓰고 있는 Physical Memory : 이 프로세스가 쓰고 있는 Virtual Memory에 다음을 추가합니다
		SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
		CString slog;
		int nMegaByte = 1048576;
		DWORD nOutHandleCnt = 0;
		GetProcessHandleCount(GetCurrentProcess(), &nOutHandleCnt);
		slog.Format(_T("totalVirtualMem,%lld,virtualMemUsed,%lld,virtualMemUsedByMe,%lld,totalPhysMem,%lld,physMemUsed,%lld,physMemUsedByMe,%lld,(MB),HandleCnt,%d"), totalVirtualMem / nMegaByte, virtualMemUsed / nMegaByte, virtualMemUsedByMe / nMegaByte, totalPhysMem / nMegaByte, physMemUsed / nMegaByte, physMemUsedByMe / nMegaByte, nOutHandleCnt);
		ext::Log::add(slog);

		return true;
	}
	bool InspLogic::PreStarting()
	{
	
		ext::Log::add(_T("SeqCase::PreStart - Begin"));
		if(!g_pMPTI->m_InspMng->m_bSetResolution)
		{
			CString sLog;
			//ext::irc::get()->_CtrlServer
			ExtPreStartParam *stPre = ext::irs::get()->_stPreStartParam.First();
			sLog.Format(_T("PreStart - Resolution,X,%0.17g,Y,%0.17g"), stPre->resolX , stPre->resolY);
			g_pMPTI->m_InspMng->SetResolution(stPre->nFovSizeX, stPre->nFovSizeY, stPre->resolX, stPre->resolY);
			ext::Log::add(sLog);
		}
		g_pMPTI->m_InspMng->SetModelLoad();
		ext::ClientCtrl::get()->_ctrl.SetSignal(eSlvSignal::eSlvPreStartDone, true, true);
		ext::Log::add(_T("SeqCase::PreStart - End"));
		return true;
	}
	bool IsMainProcessCheck()
	{
		//이 프로세스가 쓰고 있는 Virtual Memory:
		int nCnt = 0;
		bool bRet = true;
		BOOL bContinue = true;
		CString sProcessName = _T("Eagle3D");
		PROCESS_MEMORY_COUNTERS_EX pmc;
		DWORD nOutHandleCnt = 0;
		DWORD threadCount = 0; 
		sProcessName.MakeUpper(); // 문자열 비교를 하기전 강제로 모두 대문자로 바꿔줌.

		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if ((int)hSnapshot != -1)
		{
			PROCESSENTRY32 pe32;
			pe32.dwSize = sizeof(PROCESSENTRY32);
			CString tempProcessName;

			if (Process32First(hSnapshot, &pe32))
			{
				//프로세스 목록 검색 시작
				while (bContinue)
				{
					tempProcessName = pe32.szExeFile; //프로세스 목록 중 비교할 프로세스 이름;
					tempProcessName.MakeUpper();
					if ((tempProcessName.Find(sProcessName, 0) != -1))
					{
				
						HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID);
						GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc));
						GetProcessHandleCount(hProcess, &nOutHandleCnt);
					
						threadCount = pe32.cntThreads;
						CloseHandle(hProcess);
					}
					bContinue = Process32Next(hSnapshot, &pe32);
				}
			}
			CloseHandle(hSnapshot);

		}
		//이 프로세스가 쓰고 있는 Physical Memory : 이 프로세스가 쓰고 있는 Virtual Memory에 다음을 추가합니다
		SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
		CString slog;
		int nMegaByte = 1048576;
		

		slog.Format(_T("Eagle3D_Used,%lld,(MB),HandleCnt,%d,threadCount,%d,"),
			 physMemUsedByMe / nMegaByte, nOutHandleCnt, threadCount);
		ext::Log::add(slog);

		return true;
	}

	void InspLogic::Proc_SaveImage()
	{
		auto data = g_pMPTI->m_InspMng->m_ImgStack.pop();
		g_pMPTI->m_InspMng->m_NgManager->SaveMultiInspPartImage(data);
		g_pMPTI->m_InspMng->m_NgManager->SaveImageParamDelete(data);
		ext::ClientCtrl::get()->_ctrl.First()->nImageSaveCnt = g_pMPTI->m_InspMng->m_ImgStack.Count();

		if (ext::ClientCtrl::get()->_ctrl.First()->nMainAutoSeqStep >= SeqCase::InspProc5)
		{
			ext::ClientCtrl::get()->_ctrl.UpdateAll(); // Flush 를 ImgStackCount 재갱신을 위해 해준다
		}
		else
			Sleep(1);// 계속 잡아두면 문제생길수 있으니 Sleep(1) 을 줘서 CPU 우선순위에서 놔준다. 검사중일때만,
			
	}

	void InspLogic::Proc_FrameSave()
	{
		FrameWriteData();
	}
	void InspLogic::FrameWriteData()
	{
		ext::FrameWriteFlag* flag = ext::FrameFlagWriter::get()->First();
		for (int i = 0; i < 100; i++)
		{
			if (flag[i].nFlag == 1) // 1일때만 저장
			{
				//Sleep(10);// 데이터양이 많으므로 천천히 쓰기
				CString str;
				str.Format(_T("Frame Save Start Index = %d "), i);
				ext::Log::add(str);
				PUINT8 pFrame = nullptr;
				ext::FrameWriters::get()->GetData(&pFrame, i);

				CString sFolderPath = _T("");
				sFolderPath.Format(L"%s", ext::FrameWriters::get()->_BukHeader[i].strFrameName);
				ext::Log::add(sFolderPath);

				int nFrameCnt = ext::FrameWriters::get()->_BukHeader[i].nFrameCnt;
				PUINT8* pFrames = new PUINT8[nFrameCnt];

				int nX = ext::FrameWriters::get()->_BukHeader[i].stHeader.sizeX;
				int nY = ext::FrameWriters::get()->_BukHeader[i].stHeader.sizeY / nFrameCnt;

				int nSize = nX * nY;
				for (int j = 0; j < nFrameCnt; j++)
					pFrames[j] = pFrame + j * nSize;

				alpf_save_fbuk(sFolderPath, &ext::FrameWriters::get()->_BukHeader[i].stHeader, nFrameCnt, pFrames);
				flag[i].nFlag = 0; // 저장 완료[i] = 0; 
				delete[] pFrames;
				pFrames = nullptr;
			}
			else if (flag[i].nFlag == 2)
			{
				TerminateProcess(GetCurrentProcess(), 1); // 강제 종료(정리 없음)
				std::raise(SIGTERM);
				// 종료신호 
				break;
			}
			Sleep(1); // Thread 우선순위 놔주기 

		}

	}
	void InspLogic::FrameWriteData2()
	{
		ext::FrameWriteFlag* flag = ext::FrameFlagWriter::get()->First();
		for (int i = 0; i < 100; i++)
		{
			if (flag[i].nFlag == 1) // 1일때만 저장
			{
				//Sleep(10);// 데이터양이 많으므로 천천히 쓰기
				CString str;
				str.Format(_T("Frame Save Start Index = %d "), i);
				ext::Log::add(str);
				PUINT8 pFrame = nullptr;
				ext::FrameWriters::get()->GetData(&pFrame, i);

				CString sFolderPath = _T("");
				sFolderPath.Format(L"%s", ext::FrameWriters::get()->_BukHeader[i].strFrameName);
				ext::Log::add(sFolderPath);

				int nFrameCnt = ext::FrameWriters::get()->_BukHeader[i].nFrameCnt;
				int nX = ext::FrameWriters::get()->_BukHeader[i].stHeader.sizeX;
				int nY = ext::FrameWriters::get()->_BukHeader[i].stHeader.sizeY / nFrameCnt;
				int nSize = nX * nY;

				// zstd 압축 저장 시도
				bool bZstdSaved = false;
				ext::ZstdDll& zstd = ext::GetZstdDll();
				if (zstd.IsLoaded() || zstd.Load())
				{
					size_t totalSrcSize = (size_t)nSize * nFrameCnt;
					size_t boundSize = zstd.CompressBound(totalSrcSize);

					if (boundSize > 0)
					{
						std::vector<BYTE> compBuf(boundSize);
						size_t compSize = zstd.CompressMT(compBuf.data(), boundSize, pFrame, totalSrcSize, 3, 5);

						if (compSize > 0)
						{
							// .zst 확장자로 저장
							CString sZstPath = sFolderPath + _T(".zst");
							CT2CA szPath(sZstPath);
							std::string strPath(szPath);

							FILE* fp = nullptr;
							fopen_s(&fp, strPath.c_str(), "wb");
							if (fp)
							{
								// 헤더: 원본 크기(8바이트) + 프레임 수(4바이트) + sizeX(4바이트) + sizeY(4바이트)
								unsigned long long ullOrigSize = totalSrcSize;
								fwrite(&ullOrigSize, sizeof(ullOrigSize), 1, fp);
								fwrite(&nFrameCnt, sizeof(nFrameCnt), 1, fp);
								fwrite(&nX, sizeof(nX), 1, fp);
								fwrite(&nY, sizeof(nY), 1, fp);
								fwrite(compBuf.data(), 1, compSize, fp);
								fclose(fp);

								CString sLog;
								sLog.Format(_T("[ZSTD] Saved Index=%d, Orig=%llu, Comp=%llu, Ratio=%.1f%%"),
									i, (unsigned long long)totalSrcSize, (unsigned long long)compSize,
									(double)compSize / totalSrcSize * 100.0);
								ext::Log::add(sLog);
								bZstdSaved = true;
							}
						}
					}
				}

				// zstd 실패 시 기존 방식으로 저장
				if (!bZstdSaved)
				{
					// 메모리 릭 수정: std::vector 사용
					std::vector<PUINT8> vFrames(nFrameCnt);
					for (int j = 0; j < nFrameCnt; j++)
						vFrames[j] = pFrame + j * nSize;

					alpf_save_fbuk(sFolderPath, &ext::FrameWriters::get()->_BukHeader[i].stHeader, nFrameCnt, vFrames.data());
				}

				flag[i].nFlag = 0; // 저장 완료
			}
			else if (flag[i].nFlag == 2)
			{
				TerminateProcess(GetCurrentProcess(), 1); // 강제 종료(정리 없음)
				std::raise(SIGTERM);
				// 종료신호 
				break;
			}
			Sleep(1); // Thread 우선순위 놔주기 
		}
	}
}

