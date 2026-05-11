#pragma once

namespace ext
{
	enum eMstSignal
	{
		eMstAlive = 0x0001,	//1. 프로그램 살아있는지
		eMstReady = 0x0002,	//2. 작업 대기중인지
		eMstAutoMode = 0x0004,	//3. 검사모드 Auto 검사
		eMstManualMode = 0x0008,	//4. 검사모드 Manual 검사
		eMstStop = 0x0010,	//5. 검사중지, 비상정지
		eMstPreStart= 0x0020 //6. 검사 직전 Model 등 사전 데이터 Loading
	};
	enum eSlvSignal
	{
		eSlvAlive = 0x0001,	//1. 프로그램 살아있는지
		eSlvReady = 0x0002,	//2. 작업 대기중인지
		eSlvAutoMode = 0x0004,	//3. 검사모드 Auto 검사
		eSlvManualMode = 0x0008,	//4. 검사모드 Manual 검사
		eSlvStop = 0x0010,	//5. 검사중지, 비상정지
		eSlvPreStartDone = 0x0020  // 6. 검사직전 Model Load 등 사전 데이터 Loading 완료
	};
	enum eLoopSignal
	{
		eLsReady = 0,
		eLsEnd = 1,
	};
	enum eProcessCheck
	{
		eTerminate = 0,
		eExistCheck = 1,
	};
	enum eImageChannel
	{
		eTR = 0,
		eTG,
		eTB,
		eTW,
		eMR,
		eMB,
		eBR,
		eBB,
		e3D,
		e3DRaw,
		/*eS1_R,
		eS1_G,
		eS1_B,
		eS2_R,
		eS2_G,
		eS2_B,
		eS3_R,
		eS3_G,
		eS3_B,
		eS4_R,
		eS4_G,
		eS4_B,*/
		eNUM
	};
	enum eBufferState
	{
		eBufferEmpty	= 0x0001,	// 1.버퍼 빈 상태, 준비 완료
		eBufferFull		= 0x0002,	// 2.버퍼 풀, 검사 준비 완료
		eBufferCalc		= 0x0004,	// 3.검사 중
		eBufferDone		= 0x0008,	// 4.검사 완료, 결과 Get 대기, 결과 가져가면 다시 Empty
	};

	enum LANE
	{
		eFront = 0,
		eRear = 1,
		eLaneCount,
	};
	enum eToolType
	{
		eNORMAL = 0,
		eBIG = 1,
		eForeignType =2 
	};
	enum eImageType
	{
		e2DImg = 0,
		e3DImg = 1,
		eForeign = 2,
	};
	enum eMachineType
	{
		eAOI = 0,
		eWIR = 1,
		eHS = 2,
	};
	enum eCommunicationType
	{
		eSinglePC = 0,
		eMultiPC = 1,
	};
	enum eForeignData
	{
		eForeignSturct_DataPtr = 2, // 1번째부터 읽어야함, 
		eForeign_XList,
		eForeign_YList,
		eForeign_WList,
		eForeign_HList,
		eForeign_InspModule,
		eForeign_DesImg	,
		eForeign_DesImgBin,
		eForeign_imgTopR,	
		eForeign_imgTopG,
		eForeign_imgTopB,
		eForeign_imgWPInspA,
		eForeign_p3Dm,
		eForeign_pimgFR,
		eForeign_pimgWP,
		eForeign_pimgSUB,
		eForeign_DataTotal,
	};
	enum eForeignData_FR2DData
	{
		eFR2DDataSturct_DataPtr = eForeign_DataTotal,
		eFR2DData_m_pFr1,
		eFR2DData_m_pFr2,
		eFR2DData_m_pFr3,
		eFR2DData_m_pFr4,
		eFR2DData_m_pucTR,
		eFR2DData_m_pucTG,
		eFR2DData_m_pucTB,
		eFR2DData_m_pucBR,
		eFR2DData_m_pucBB,
		eFR2DData_Total,
	};
	enum eForeignInfoData
	{
		eForeignInfoData_Struct = eFR2DData_Total,
		eForeignInspPartInfo_Struct,
		eForeignInspPartInfo_InspRoiImgBuf,
		eForeignInspPartInfo_imgTop_R,
		eForeignInspPartInfo_imgTop_G,
		eForeignInspPartInfo_imgTop_B,
		eForeignInspPartInfo_imgSide1_R,
		eForeignInspPartInfo_imgSide1_G,
		eForeignInspPartInfo_imgSide1_B,
		eForeignInspPartInfo_imgSide2_R,
		eForeignInspPartInfo_imgBottom_R,
		eForeignInspPartInfo_imgBottom_B,
		eForeignInspPartInfo_ZmapData,
		eForeignInspPartInfo_ForeParamROIStruct,
		eForeignInspPartInfo_arrROIStruct,
		eForeignInspPartInfo_Total,
	};

	enum eForeignResultData
	{
		eForeignResultData_Sturct  = 2, 
		eForeignResultData_AForeignResult,
		eForeignResultData_m_stForeignWP,

	};
	#define ExtNoUpdate false
}
