#include "stdafx.h"
#include "ExtInspRoot.h"
#include "ExtLog.h"
#include <iostream>
#include <fstream>
#include <cstdlib>  // for std::atoi
#include <string>  // std::getline 함수가 정의된 헤더 추가
#include <sstream> // std::istringstream 사용 시 필요
namespace ext {

	bool EnvVariable::bUseSystem = false;
	int EnvVariable::nSystemType = EXT_INSP_SERVER;
	CString EnvVariable::sAlgoToolPath = _T("C:\\Eagle3D_64x\\PROGRAM\\AlgoTool\\AlgorithmTool.exe");

	QWORD EnvVariable::nImgBufSize = (1024 * 1024 * 1024 * 2) - 1;
	int EnvVariable::nPartBufCount = 20;
	int EnvVariable::nWindowBufCount = 40;
	int EnvVariable::nAlgoBufCount = 40;
	QWORD EnvVariable::nAlgoElemBufSzBytes = 1024 * 1024;

	int EnvVariable::nTool_id = 0;
	int EnvVariable::nToolCount = 2;

	int EnvVariable::nTotalPartBufferSize = 1024 * 1024 * 128;
	int EnvVariable::nTotalImgBufferSize = (1024 * 1024 * 1024 * 2) - 1;
	int EnvVariable::nTotalRstBufferSize = 1024 * 1024 * 128;
	int EnvVariable::nTotalLightBufferSize = 1024 * 1024 * 128;

	int EnvVariable::nPartBufferSize = 1024 * 1024 * 128;
	int EnvVariable::nImgBufferSize2D = 1024 * 1024 * 25;
	int EnvVariable::nImgBufferSize3D = 1024 * 1024 * 100;
	int EnvVariable::nRstBufferSize = 1024 * 1024 * 2;
	int EnvVariable::nLightBufferSize = 1024 * 1024;

	int EnvVariable::nBigImgBufferSize2D = 1024 * 1024 * 70;
	int EnvVariable::nBigImgBufferSize3D = 1024 * 1024 * 280;
	int EnvVariable::nPOCRModelData = (1024 * 1024 * 1024 * 2) - 1;

	//Foreign Data 
	int EnvVariable::nForeignBufferSize2D = 1024 * 1024 * 25; // FOV Size
	int EnvVariable::nForeignBufferSize3D = nForeignBufferSize2D * sizeof(float); // FOV Size
	int EnvVariable::nForeigeDataWP = (1024 * 1024 * 1024 * 2) - 1; // Pointers 
	int EnvVariable::nForeigeData = (1024 * 1024 * 1024 * 1) ; // Pointers 
	int EnvVariable::nForeigeDataFovNum = 40; // FovCnt;

	
	/////////////////
	int EnvVariable::nSpareCoreCnt = 2;
	int EnvVariable::nApplicableCoreCnt = 3;
	int EnvVariable::nStringDataStructSize = 1024 * 10;

	int EnvVariable::nBigTypeToolCnt = 1;
	int EnvVariable::nForeignTypeToolCnt = 1;

	std::shared_ptr<CMutex> Sync::_Sync;
	Sync::Sync()
	{
		if (_Sync == nullptr)
		{
			_Sync = std::shared_ptr<CMutex>(new CMutex(FALSE, _T("AOI_EA_PANNEL")));
		}
		_Sync->Lock();
	}
	Sync::~Sync()
	{
		_Sync->Unlock();
	}

			// read or write xml file to save these values.
	bool EnvVariable::Init(int _nTool_id, int ToolCnt, CString sToolPath, int nSpareCnt, int nCoreCnt, int nMachineType, int nBigCnt, int nForeCnt)
	{
		CString strToolMemorySizePath = _T("MultiProcessShared.ini");
		// read xml using TinyXML2
		if (_nTool_id == EXT_SERVER_TOOL_ID)
		{
			sAlgoToolPath = _T("");
			sAlgoToolPath = sToolPath;
			sToolPath.Replace(_T("AlgorithmTool.exe"), _T("MultiProcessShared.ini"));
			strToolMemorySizePath = sToolPath;
			bUseSystem = true;
			nSystemType = EXT_INSP_SERVER;		// Server or Client

			nImgBufSize = (1024 * 1024 * 1024 * 2) - 1;
			nPartBufCount = 20;
			nWindowBufCount = 40;
			nAlgoBufCount = 40;
			nAlgoElemBufSzBytes = 1024 * 1024;

			nTool_id = EXT_SERVER_TOOL_ID;
			nToolCount = ToolCnt;
			nBigTypeToolCnt = nBigCnt;
			nForeignTypeToolCnt = nForeCnt;
			nTotalPartBufferSize = 1024 * 1024 * 128;
			nTotalImgBufferSize = (1024 * 1024 * 2) - 1;
			

			nTotalRstBufferSize = 1024 * 1024 * 128;
			nTotalLightBufferSize = 1024 * 1024 * 128;

			nPartBufferSize = 1024 * 1024 * 128;

			nImgBufferSize2D = 1024 * 1024 * 25; //Part Image Size Only One Channel 
			nImgBufferSize3D = nImgBufferSize2D * sizeof(float); //Part Image Size Only One Channel  * Float 

			nBigImgBufferSize2D = 1024 * 1024 * 70;
			nBigImgBufferSize3D = nBigImgBufferSize2D * sizeof(float);

			if (nMachineType == eMachineType::eWIR)
			{
				nImgBufferSize2D = 1024 * 1024 * 25; // WIR FOV Size 
				nImgBufferSize3D = 1024; // WIR 3D X Default Value

				nBigImgBufferSize2D = 1024;// WIR 3D Big Part Not Use
				nBigImgBufferSize3D = nBigImgBufferSize2D * sizeof(float);

			}

			nRstBufferSize = 1024 * 1024 * 2;
			nLightBufferSize = 1024 * 1024;

			
				nPOCRModelData = (1024 * 1024 * 1024 * 2) - 1; 

			nSpareCoreCnt = nSpareCnt;
			nApplicableCoreCnt = nCoreCnt;

			nStringDataStructSize = 1024 * 10;
		}
		else
		{
			sAlgoToolPath = sToolPath;
			sToolPath.Replace(_T("AlgorithmTool.exe"), _T("MultiProcessShared.ini"));
			strToolMemorySizePath = sToolPath;
			bUseSystem = true;
#ifdef _EXTMONITOR
			nSystemType = EXT_INSP_MONITOR;
#else
			nSystemType = EXT_INSP_CLIENT;		// Server or Client
#endif

			nImgBufSize = (1024 * 1024 * 1024 * 2) - 1;
			nPartBufCount = 20;
			nWindowBufCount = 40;
			nAlgoBufCount = 40;
			nAlgoElemBufSzBytes = 1024 * 1024;

			nTool_id = _nTool_id;
			nToolCount = ToolCnt;
			nBigTypeToolCnt = nBigCnt;
			nForeignTypeToolCnt = nForeCnt;
			
			nTotalPartBufferSize = 1024 * 1024 * 128;
			nTotalImgBufferSize = (1024 * 1024 * 2) - 1;
			nTotalRstBufferSize = 1024 * 1024 * 128;
			nTotalLightBufferSize = 1024 * 1024 * 128;

			nPartBufferSize = 1024 * 1024 * 128;
			nImgBufferSize2D = 1024 * 1024 * 25;
			nImgBufferSize3D = nImgBufferSize2D * sizeof(float);
			nRstBufferSize = 1024 * 1024 * 2;
			nLightBufferSize = 1024 * 1024;

			nBigImgBufferSize2D = 1024 * 1024 * 70;
			nBigImgBufferSize3D = nBigImgBufferSize2D * sizeof(float);


			if (nMachineType == eMachineType::eWIR)
			{
				nImgBufferSize2D = 1024 * 1024 * 25; // WIR FOV Size 
				nImgBufferSize3D = 1024; // WIR 3D X Default Value

				nBigImgBufferSize2D = 1024;// WIR 3D Big Part Not Use
				nBigImgBufferSize3D = nBigImgBufferSize2D * sizeof(float);

			}

			nPOCRModelData = (1024 * 1024 * 1024 * 2) - 1;
			nSpareCoreCnt = nSpareCnt;
			nApplicableCoreCnt = nCoreCnt;

			nStringDataStructSize = 1024 * 10;
		}

		if(readFile(strToolMemorySizePath))
			ext::Log::add(_T("Pass - Ext @EnvVariable@ Shared Init "));
		return true;
	}
	void EnvVariable::Exit()
	{
		// write xml using TinyXML2
	}

	// ????? ?? ??? ???? ??
	std::string EnvVariable::removeSpaces(const std::string& str) 
	{
		std::string result;
		std::remove_copy_if(str.begin(), str.end(), std::back_inserter(result), ::isspace);
		return result;
	}

	std::vector<std::string> EnvVariable::tokenize(const std::string& str, char delimiter)
	{
		std::string strR = removeSpaces(str);
		std::istringstream iss(strR);
		std::vector<std::string> tokens;
		std::string token;

		while (std::getline(iss, token, delimiter)) 
		{
			tokens.push_back(token);
		}

		return tokens;
	}


	bool EnvVariable::readFile(const CString & filename)
	{
		// CString? std::string?? ??
		CT2CA pszConvertedAnsiString(filename);
		std::string strStd(pszConvertedAnsiString);

		std::ifstream file(strStd);

		// ?? ?? ?? ??
		if (!file.is_open()) {
			ext::Log::add(_T("File Open Fail - Ext @EnvVariable@ Shared Init "));
			//std::cerr << "??? ? ? ????: " << strStd << std::endl;
			return false;
		}
		std::vector<std::vector<std::string>> readList;
		std::string line;
		while (std::getline(file, line)) 
		{
			std::vector<std::string> tokens = EnvVariable::tokenize(line, ',');
			readList.push_back(tokens);
			//std::cout << line << std::endl;
		}
		CString sLog; 
		if (readList[0].size() == readList[1].size())
		{
			ext::Log::add(_T(" ===========Read Shared Init =========== "));
			for (int i = 0; i<readList[1].size(); i++)
			{
				if ((1024 * 1024 * 1024 * 2) - 1 < std::stoll(readList[1][i]))
				{
					sLog.Format(_T("%s  : %s = Fail Size (Max : 2gigaByte -1) "), (CString)readList[0][i].c_str(), (CString)readList[1][i].c_str());
					ext::Log::add(sLog);
				}

				switch (i)
				{
				case 0: //nPartBufCount
					nPartBufCount = std::stoi(readList[1][i]);
					sLog.Format(_T("nPartBufCount,%s"), (CString)readList[1][i].c_str());
					break;
				case 1: //nWindowBufCount
					nWindowBufCount = std::stoi(readList[1][i]);
					sLog.Format(_T("nWindowBufCount,%s"), (CString)readList[1][i].c_str());
					break;
				case 2: //nAlgoBufCount
					nAlgoBufCount = std::stoi(readList[1][i]);
					sLog.Format(_T("nAlgoBufCount,%s"), (CString)readList[1][i].c_str());
					break;
				case 3: //nAlgoElemBufSzBytes
					nAlgoElemBufSzBytes = std::stoi(readList[1][i]);
					sLog.Format(_T("nAlgoElemBufSzBytes,%s"), (CString)readList[1][i].c_str());
					break;
				case 4: //nTotalPartBufferSize
					nTotalPartBufferSize = std::stoi(readList[1][i]);
					sLog.Format(_T("nTotalPartBufferSize,%s"), (CString)readList[1][i].c_str());
					break;
				case 5: //nTotalImgBufferSize
					nTotalImgBufferSize = std::stoi(readList[1][i]);
					sLog.Format(_T("nTotalImgBufferSize,%s"), (CString)readList[1][i].c_str());
					break;
				case 6: //nTotalRstBufferSize
					nTotalRstBufferSize = std::stoi(readList[1][i]);
					sLog.Format(_T("nTotalRstBufferSize,%s"), (CString)readList[1][i].c_str());
					break;
				case 7: //nTotalLightBufferSize
					nTotalLightBufferSize = std::stoi(readList[1][i]);
					sLog.Format(_T("nTotalLightBufferSize,%s"), (CString)readList[1][i].c_str());
					break;
				case 8: //nPartBufferSize
					nPartBufferSize = std::stoi(readList[1][i]);
					sLog.Format(_T("nPartBufferSize,%s"), (CString)readList[1][i].c_str());
					break;
				case 9: //nImgBufferSize2D
					nImgBufferSize2D = std::stoi(readList[1][i]);
					sLog.Format(_T("nImgBufferSize2D,%s"), (CString)readList[1][i].c_str());
					break;
				case 10: //nImgBufferSize3D
					nImgBufferSize3D = std::stoi(readList[1][i]);
					sLog.Format(_T("nImgBufferSize3D,%s"), (CString)readList[1][i].c_str());
					break;
				case 11: //nRstBufferSize
					nRstBufferSize = std::stoi(readList[1][i]);
					sLog.Format(_T("nRstBufferSize,%s"), (CString)readList[1][i].c_str());
					break;
				case 12: //nLightBufferSize
					nLightBufferSize = std::stoi(readList[1][i]);
					sLog.Format(_T("nLightBufferSize,%s"), (CString)readList[1][i].c_str());
					break;
				case 13: //nBigImgBufferSize2D
					nBigImgBufferSize2D = std::stoi(readList[1][i]);
					sLog.Format(_T("nBigImgBufferSize2D,%s"), (CString)readList[1][i].c_str());
					break;
				case 14: //nBigImgBufferSize3D
					nBigImgBufferSize3D = std::stoi(readList[1][i]);
					sLog.Format(_T("nBigImgBufferSize3D,%s"), (CString)readList[1][i].c_str());
					break;
				case 15: //nPOCRModelData
					nPOCRModelData = std::stoi(readList[1][i]);
					sLog.Format(_T("nPOCRModelData,%s"), (CString)readList[1][i].c_str());
					break;
				}
				ext::Log::add(sLog);

			}

				
		}
		else
		{
			sLog.Format(_T("Header Size %d  != Value Size %d "), readList[0].size() , readList[1].size());
			ext::Log::add(sLog);
		}
		ext::Log::add(_T(" ===========Read Shared End =========== "));
		
		file.close();

		return true;

	}

		Ctrler::Ctrler()
		{
			_TotalBuff = 0; 
		}

		bool Ctrler::Init()
		{
			int nAccessType = FILE_MAP_ALL_ACCESS;
			if (EnvVariable::nSystemType == EXT_INSP_CLIENT || EnvVariable::nSystemType == EXT_INSP_MONITOR)
				nAccessType = FILE_MAP_READ;
			Create(sizeof(CtrlServer), _T("AOI_ExtCtrler"), nAccessType);
			_TotalBuff = sizeof(CtrlServer);
			if (EnvVariable::nSystemType == EXT_INSP_SERVER)
			{
				SetSignal(eMstSignal::eMstAlive, true, ExtNoUpdate);
				SetSignal(eMstSignal::eMstAutoMode, false, ExtNoUpdate);
				First()->dwProcessID = GetCurrentProcessId();
				First()->nCurrToolIdx = 0;
				First()->nCurrBigToolIdx = 0;
				UpdateAll();
			}
			
			return true;
		}

		void Ctrler::InitSignal()
		{
			First()->signals &= (~ext::eMstAlive);
			First()->signals &= (~ext::eMstReady);
			First()->signals &= (~ext::eMstAutoMode);
			First()->signals &= (~ext::eMstManualMode);
			First()->signals &= (~ext::eMstStop);
		}

		void Ctrler::SetSignal(int sig, bool bOn, bool bUpdate)
		{
			if (bOn)
				First()->signals |= sig;
			else
				First()->signals &= (~sig);
			if (bUpdate)
				UpdateAll();
		}
		bool Ctrler::GetSignal(int sig)
		{
			return ((First()->signals & sig) != 0);
		}
		void Ctrler::Clear()
		{
			Memory<CtrlServer>::Clear();
			First()->nPartFullCnt = 0;
			First()->nCurrToolIdx = 0;
			First()->nCurrBigToolIdx = 0; 
			UpdateAll();
		}
		DWORD Ctrler::GetProcessID()
		{
			return First()->dwProcessID;
		}

		Ctrlee::Ctrlee()
		{
			_TotalBuff = 0;
		}

		bool Ctrlee::Init(int nTool_id)
		{
			int nAccessType = FILE_MAP_ALL_ACCESS;
			if (EnvVariable::nSystemType == EXT_INSP_SERVER || EnvVariable::nSystemType == EXT_INSP_MONITOR)
				nAccessType = FILE_MAP_READ;

			CString sMemName,sLog;
			sMemName.Format(_T("AOI_ExtCtrlee_%d"), nTool_id);


			sLog.Format(_T("%s_Start"), sMemName);

			bool bInit = Create(sizeof(CtrlClient), sMemName, nAccessType);
			_TotalBuff = sizeof(CtrlClient);
			if (!bInit)
			{
				sLog.Format(_T("%s_Init_Fail "), sMemName);
				ext::Log::add(sLog);
			}
			sLog.Format(_T("%s_End"), sMemName);
			if (EnvVariable::nSystemType == EXT_INSP_CLIENT)
			{
				First()->dwProcessID = GetCurrentProcessId();
				UpdateAll();
			}
			return true;
		}

		void Ctrlee::SetSignal(int sig, bool bOn, bool bUpdate)
		{
			if (bOn)
				First()->signals |= sig;
			else
				First()->signals &= (~sig);

			if (bUpdate)
				UpdateAll();
		}
		bool Ctrlee::GetSignal(int sig)
		{
			return ((First()->signals & sig) != 0);
		}
		DWORD Ctrlee::GetProcessID()
		{
			return First()->dwProcessID;
		}
		void Ctrlee:: Clear()
		{
			Memory<CtrlClient>::Clear();

			First()->CurrCalcPartIdx = 0;
			First()->CalcPartCount = 0;
			//First()->nOwnCalcCount = 0;
			UpdateAll();
		}

		CtrlBuffer::CtrlBuffer()
		{

		}

		bool CtrlBuffer::Init(int nTool_Id, int nMode)
		{
			// bMode true : Send, false : Receive 
			int nAccessType = FILE_MAP_ALL_ACCESS;
			if (EnvVariable::nSystemType == EXT_INSP_MONITOR)
				nAccessType = FILE_MAP_READ;
			CString sMemName, sLog;

			switch (nMode)
			{
			case ext::CtrlMode::eSend: 
				sMemName.Format(_T("AOI_ExtCtrBufferSend_%d_%d"), nTool_Id, m_nBuffIdx);
				break;
			case ext::CtrlMode::eReceive: 
				sMemName.Format(_T("AOI_ExtCtrBufferReceive_%d_%d"), nTool_Id, m_nBuffIdx);
				break;
			case ext::CtrlMode::eForeignSend: 
				sMemName.Format(_T("AOI_ExtForeignCtrBufferSend_%d_%d"), nTool_Id, m_nBuffIdx);
				break;
			case ext::CtrlMode::eForeignReceive: 
				sMemName.Format(_T("AOI_ExtForeignCtrBufferReceive_%d_%d"), nTool_Id, m_nBuffIdx);
				break;
			default:
				break;
			}
			
			sLog.Format(_T("%s_Start"), sMemName);
			ext::Log::add(sLog);
			bool bInit = Create(sizeof(CtrlBuffer), sMemName, nAccessType);

			if (!bInit)
			{
				sLog.Format(_T("%s_Init_Fail "), sMemName);
				ext::Log::add(sLog);
			}
			sLog.Format(_T("%s_End"), sMemName);
			ext::Log::add(sLog);
			if (EnvVariable::nSystemType == EXT_INSP_CLIENT)
			{
				UpdateAll();
			}
			return true;
		}
		void CtrlBuffer::Exit()
		{
			_stream.UnmapView();
		}
		void CtrlBuffer::SetFlag(int nFlag, bool bUpdate)
		{
			
			First()->status = nFlag;
			
			if (bUpdate)
				UpdateAll();
		}
		bool CtrlBuffer::GetFlag(int nFlag)
		{
			bool bRet = false;
			if (First()->status == nFlag)
				bRet = true;
			return bRet;
		}
		void CtrlBuffer::Clear()
		{
			First()->status = InspFlag::IF_Empty;
			UpdateAll();
		}

		InspPannel::InspPannel()
		{
			_TotalBuff = 0;
		}
		bool InspPannel::Init()
		{
			ext::Log::add(_T("InspPannel::Init() - Start"));
			bool bInit = Create(sizeof(InspStatus) , _T("AOI_ExtInspPannel"), FILE_MAP_ALL_ACCESS);
			memset(First(), 0, sizeof(InspStatus));
			UpdateAll();
			ext::Log::add(_T("InspPannel::Init() - End"));
			return bInit;
		}
		void InspPannel::Clear()
		{
			memset(First(), 0, sizeof(InspStatus));
		}
		void InspPannel::Update(int pos)
		{
			UpdateIdx(pos);
		}
		int InspPannel::ModelTotalCount(int nModelType)
		{
			int nTotalCount = -1 ;
			switch (nModelType)
			{
			case ModelBufferFlag::eExtModel_POCR:nTotalCount = First()->nModelIndex_POCR; break;
			case ModelBufferFlag::eExtModel_Pattern:nTotalCount = First()->nModelIndex_Pattern; break;
			case ModelBufferFlag::eExtModel_COB: nTotalCount = First()->nModelIndex_COB; break;
			case ModelBufferFlag::eExtModel_Exc: nTotalCount = First()->nModelIndex_Exc; break;
			case ModelBufferFlag::eExtModel_PadBW: nTotalCount = First()->nModelIndex_PadBW; break;
			case ModelBufferFlag::eExtModel_ShapeX: nTotalCount = First()->nModelIndex_ShapeX; break;
			case ModelBufferFlag::eExtModel_HalconPOCR: nTotalCount = First()->nModelIndex_HalconPOCR; break;
			case ModelBufferFlag::eExtModel_NGBlobExc: nTotalCount = First()->nModelIndex_NGBlobExc; break;
			}
			return nTotalCount;
		}
		ModelCtrl::ModelCtrl()
		{
			_TotalBuff = 0;

		}
		bool ModelCtrl::Init(int nModelIndex)
		{
			CString sLog; 
			sLog.Format(_T("ModelCtrl::Init() - Start_ ModelIndex = %d"), nModelIndex);
			ext::Log::add(sLog);
			int nAccessType = FILE_MAP_ALL_ACCESS;
			if (EnvVariable::nSystemType == EXT_INSP_CLIENT)
				nAccessType = FILE_MAP_READ;
			CString sModelName = _T("");
			sModelName.Format(_T("AOI_ExtModelCtrl_%d"),nModelIndex);
			bool bInit = Create(sizeof(ModelStatus) * EXT_BUFFER_POCR_CNT, sModelName, nAccessType);
			_TotalBuff = sizeof(ModelStatus) * EXT_BUFFER_POCR_CNT;
			sLog.Format(_T("ModelCtrl::Init() - End , Model  Cnt %d "), EXT_BUFFER_POCR_CNT); // ¹e¿­·I A￠±U °¡´E  
			ext::Log::add(sLog);
			return bInit;
		}
		void ModelCtrl::Clear()
		{
			memset(First(), 0, sizeof(ModelStatus) * EXT_BUFFER_POCR_CNT);
		}
		CompareStrurct::CompareStrurct()
		{
			_TotalBuff = 0;
		}
		bool CompareStrurct::Init()
		{
			ext::Log::add(_T("AlgoCompare::Init() - Start"));
			bool bInit = Create(sizeof(tagCompareStrurctData), _T("AOI_ExtAlgoCompare"), FILE_MAP_ALL_ACCESS);
			_TotalBuff = sizeof(tagCompareStrurctData);
			if (EnvVariable::nSystemType == EXT_INSP_CLIENT)
				this->First()->nStructFlag = 0; 
			ext::Log::add(_T("AlgoCompare::Init() - End"));
			return bInit;
		}
		void CompareStrurct::Exit()
		{
			_stream.UnmapView();
		}
		MachineSetting::MachineSetting()
		{
			_TotalBuff = 0;
		}
		bool MachineSetting::Init(int nTotalToolCnt)
		{
			ext::Log::add(_T("MachineSetting::Init() - Start"));
			bool bInit = Create(sizeof(tagMachineParam), _T("AOI_MachineSetting"), FILE_MAP_ALL_ACCESS);
			
			_TotalBuff = sizeof(tagMachineParam);
			if (EnvVariable::nSystemType == EXT_INSP_SERVER)
			{
				First()->nMachineType = (int)ext::eMachineType::eAOI;
				First()->nComType = (int)ext::eCommunicationType::eSinglePC;
				First()->nTotalToolCnt = nTotalToolCnt;
			}
			
			ext::Log::add(_T("MachineSetting::Init() - End"));
			return bInit;
		}

		PreStartParam::PreStartParam()
		{

		}
		bool PreStartParam::Init()
		{
			ext::Log::add(_T("PreStartParam::Init() - Start"));
			bool bInit = Create(sizeof(tagExtPreStartParam), _T("AOI_PreStartParam"), FILE_MAP_ALL_ACCESS);
			ext::Log::add(_T("PreStartParam::Init() - End"));
			return bInit;
		}

		bool PreStartParam::SetData(int nFovSizeX, int nFovSizeY, double dResolution_X, double dResolution_Y )
		{
			First()->nFovSizeX = nFovSizeX;
			First()->nFovSizeY = nFovSizeY;
			First()->resolX = dResolution_X;
			First()->resolY = dResolution_Y;
			UpdateAll();
			return true;
		}
		StartInit::StartInit()
		{

		}
		bool StartInit::Init()
		{
			ext::Log::add(_T("StartInitStruct::Init() - Start"));
			bool bInit = Create(sizeof(tagExtStartInitStruct), _T("MultiProcessInit"), FILE_MAP_ALL_ACCESS);
			ext::Log::add(_T("StartInitStruct::Init() - End"));
			return bInit;
		}

		bool StartInit::SetData(ExtStartInitStruct st)
		{
			CString str;
			First()->nBigPartTool = st.nBigPartTool;
			str.Format(_T("[EXT] BigPartTool Init : %d Count"), st.nBigPartTool);
			ext::Log::add(str);
			First()->nForeignTool = st.nForeignTool;
			str.Format(_T("[EXT] ForeignTool Init : %d Count"), st.nForeignTool);
			ext::Log::add(str);
			First()->nToolCnt = st.nToolCnt;
			str.Format(_T("[EXT] ToolCnt Init : %d Count"), st.nToolCnt);
			ext::Log::add(str);
			UpdateAll();
			return true;
		}

		bool StartInit::GetData(ExtStartInitStruct  * st)
		{
			CString str;

			st->nBigPartTool = First()->nBigPartTool;
			str.Format(_T("[EXT] BigPartTool Init : %d Count"), st->nBigPartTool);
			ext::Log::add(str);
			st->nForeignTool = First()->nForeignTool;
			str.Format(_T("[EXT] ForeignTool Init : %d Count"), st->nForeignTool);
			ext::Log::add(str);
			st->nToolCnt = First()->nToolCnt;
			str.Format(_T("[EXT] Tool Init : %d Count"), st->nToolCnt);
			ext::Log::add(str);
			return true;
		}
		bool FrameFlagWriter::Init()
		{
			ext::Log::add(_T("EXT_FrameWriter::Init() - Start"));
			bool bInit = Create(sizeof(tagFrameWriteFlag)*100, _T("EXT_FrameWriter"), FILE_MAP_ALL_ACCESS);
			ext::Log::add(_T("EXT_FrameWriter::Init() - End"));
			return bInit;
		}
		void FrameFlagWriter::RunProgram(CString sAlgoToolPath)
		{
			CString sExecuteParam;
			sExecuteParam.Format(_T("Simul"));
			//::ShellExecute(nullptr, _T("open"), EnvVariable::sAlgoToolPath, sExecuteParam, nullptr, SW_SHOW);

			SHELLEXECUTEINFO info = { sizeof(SHELLEXECUTEINFO) };

			info.lpVerb = TEXT("runas");

			info.lpFile = sAlgoToolPath;
			info.lpParameters = sExecuteParam;

			info.nShow = SW_SHOW;

			ShellExecuteEx(&info);
			return;
		}
	}
