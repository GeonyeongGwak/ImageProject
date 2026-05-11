#pragma once

#include <vector>
#include <memory>
#include <afxmt.h>
#include "SharedMemory.h"
#include "ExtProductionInfo.h"
#include "ExtInspRootDef.h"
#include <assert.h>
#include "PInsp_Algo/InspParamDef_AlgoBase.h"
#include "QTimer.h"

// static class
#define EXT_INSP_SERVER		1
#define EXT_INSP_CLIENT		2
#define EXT_INSP_MONITOR	3

#define EXT_SERVER_TOOL_ID	0

#define EXT_BUFFER_CNT 20
#define EXT_BUFFER_FOREIGN_CNT 40
#define EXT_BUFFER_FOREIGN_RST_CNT 40
#define EXT_BUFFER_RST_CNT 100
#define EXT_BUFFER_POCR_CNT 500 // POCR 모델 갯수 ( 차후 변경 )
#define EXT_BIG_BUFFER_CNT 5
#define EXT_BIG_BUFFER_RST_CNT 10

#define EXT_3D_BUFFER_INDEX 8
#define EXT_3D_RAW_BUFFER_INDEX 9
#define EXT_AI_BUFFER_INDEX 10
#define EXT_FRAMEBUFFER 100


#define EXT_FOVSIZE 30 //25MM + Mag
#define EXT_MERGEIMG_CNT 4
#define EXT_MAXCLIENT_CNT 6


#ifdef _MPTI_EXPORT
// eagle3d 표시
#define EXT_SERVER
#endif

#ifdef EXT_SERVER
	#define ExtPrmPtr
	#define ExtRstPtr
#else
	#define ExtPrmPtr 
	#define ExtPrmPtrIdx 
	#define ExtRstPtr 
#endif

#define MAX_PROCESS 4

namespace ext{

	class Sync
	{
	public:
		Sync();
		virtual ~Sync();

		static std::shared_ptr<CMutex> _Sync;
	};
typedef struct tagCtrlServer
{
	int signals;
	int NumOfClients;
	int nPartFullCnt;
	ExtProductionInfo prod;
	DWORD dwProcessID;
	int nMainAutoSeqStep;
	int nCurrToolIdx;	//다음 데이터 줘야하는 Tool 번호
	int nCurrBigToolIdx;	//다음 데이터 줘야하는 BigTool 번호
	int nInspDoneCnt;	//서버에서 계속 갱신되는 변수, 각 툴에서 검사 완료된 갯수 업데이트
	int nInspSkipCnt;
	int nInspForeignDoneCnt;
	int nInspForeignSkipCnt;
	ExtAlgoMachineInfo machineAlgo;
	ExtPOCRFont stPOCRInfo;
	lightData sAngleColorData[3];
	byte byDefaultAC[2];
}CtrlServer;

typedef struct tagCtrlClient
{
	int signals;
	int CurrCalcPartIdx;
	int CalcPartCount;
	int nThreadStatus;	// 1 ~ 4
	int nOwnCalcCount;  // Need To Calc
	DWORD dwProcessID;
	int nMainAutoSeqStep;
	int nImageSaveCnt;
}CtrlClient;
typedef struct tagCompareStrurctData
{
	int nStructSizeArray[200]; // 구조체갯수가 200개는 넘지않는것으로 
	int nAlgoCnt; // Algorithm 만 
	int nTotalCnt; // 전체 Struct ; 
	int nStructFlag; // 0: Not Struct Write 1: Write 

}CompareStrurctData;
typedef struct tagClientBufferCtrl
{
	int status;
	int nCalcDoneCount;	//Tool[0~2] 각 버퍼가 계산 완료한 파트 갯수


}ClientBufferCtrl;

typedef struct tagImgData
{
	QWORD imgStIdx;
	QWORD imgSz;
}ImgData;

typedef struct tagFrameWriteFlag
{
	int nFlag; 

}FrameWriteFlag;

typedef enum enmInspFlag
{
	IF_Empty = 0,
	IF_BufFull = 1,
	IF_BufCalc = 2,
	IF_BufDone = 3
}InspFlag;

typedef enum enmBufferFlag
{
	IF_OFF = 0,
	IF_ON = 1,
	IF_CALC = 2,
	IF_CALCEND =3 , 
}BufferFlag;
typedef enum enmCtrlMode
{
	eSend = 0,
	eReceive = 1,
	eForeignSend = 2,
	eForeignReceive = 3,
}CtrlMode;
typedef enum enmModelBufferFlag
{
	eExtModel_POCR= 0,
	eExtModel_Pattern ,
	eExtModel_COB,
	eExtModel_Exc,
	eExtModel_PadBW,
	eExtModel_ShapeX,
	eExtModel_HalconPOCR,
	eExtModel_NGBlobExc,
	eExtModel_Total  
}ModelBufferFlag;

typedef struct tagInspStatus
{
	int partIDOrg;
	ImgData img;
	int nTool_id;
	int result;
	int flag;
	int SubBufferIdx;
	int nModelIndex_POCR;
	int nModelIndex_Pattern;
	int nModelIndex_COB;
	int nModelIndex_Exc;		//PatternDiff -> ExceptModel
	int nModelIndex_PadBW;
	int nModelIndex_ShapeX;
	int nModelIndex_HalconPOCR;
	int nModelIndex_NGBlobExc;		//PatternDiff -> ExceptModel
}InspStatus;

typedef struct tagModelStatus
{
	int partIDOrg;
	ImgData img;
	wchar_t sFilePath[255];
	int nWidth; 
	int nHeight;
	int ch;
}ModelStatus;
typedef struct tagMachineParam
{
	int nMachineType;
	int nComType;
	int nTotalToolCnt;
}MachineParam;

class AddrCalc
{
public:
	static LPVOID Calc_Sub_Offset(LPVOID org, LPVOID pos)
	{
		BYTE * OrgBytes = (BYTE *)org;
		BYTE * PosBytes = (BYTE *)pos;

		QWORD orgAddr = (QWORD)OrgBytes;
		QWORD posAddr = (QWORD)PosBytes;

		return (LPVOID)(posAddr - orgAddr);
	}
	static LPVOID Calc_Add_Offset(LPVOID org, LPVOID pos)
	{
		BYTE * OrgBytes = (BYTE *)org;
		BYTE * PosBytes = (BYTE *)pos;

		QWORD orgAddr = (QWORD)OrgBytes;
		QWORD posAddr = (QWORD)PosBytes;

		return (LPVOID)(posAddr + orgAddr);
	}
	static QWORD Calc_Index(LPVOID pos, QWORD szStruct)
	{
		BYTE * PosBytes = (BYTE *)pos;

		QWORD posAddr = (QWORD)PosBytes;

		return QWORD((posAddr) / szStruct);
	}
};

class EnvVariable
{
public:
	static bool bUseSystem;
	static int nSystemType;		// Server or Client
	static CString sAlgoToolPath;

	static QWORD nImgBufSize;
	static int nPartBufCount;
	static int nWindowBufCount;
	static int nAlgoBufCount;

	static QWORD nAlgoElemBufSzBytes;
	static int nTotalPartBufferSize;
	static int nTotalImgBufferSize;
	static int nTotalRstBufferSize;
	static int nTotalLightBufferSize;

	static int nTool_id;
	static int nToolCount;

	static int nPartBufferSize;
	static int nImgBufferSize2D;
	static int nImgBufferSize3D;
	static int nRstBufferSize;
	static int nLightBufferSize;

	static int nBigImgBufferSize2D;
	static int nBigImgBufferSize3D;
	static int nPOCRModelData;
	static int nForeignBufferSize2D; 
	static int nForeignBufferSize3D;
	static int nForeigeDataWP;
	static int nForeigeData;
	static int nForeigeDataFovNum;

	static int nSpareCoreCnt;
	static int nApplicableCoreCnt;

	static int nStringDataStructSize;
	static int nBigTypeToolCnt; 
	static int nForeignTypeToolCnt;
	// read or write xml file to save these values.
	static bool readFile(const CString & filename);
	static bool Init(int _nTool_id, int nToolCnt = 2,CString ToolPath=_T(""), int nSpareCnt = 2, int nCoreCnt = 3, int nMachineType = 0 , int nBigCnt = 1, int nForeCnt = 1 );
	
	static void Exit();
	static std::string removeSpaces(const std::string & str);
	static std::vector<std::string> tokenize(const std::string & str, char delimiter);
};

typedef EnvVariable env;


template<typename T>
class Memory
{
public:
	Memory()
		: _sizebytes(0)
		, _UseCount(0)
		, _CurStIdx(0)
	{

	}
	Memory(QWORD sizebytes, CString sName, DWORD access)
		: _sizebytes(sizebytes)
		, _UseCount(0)
		, _CurStIdx(0)
	{
		Create(sizebytes, sName, access);
	}

	virtual ~Memory()
	{

	}

	virtual bool Create(QWORD sizebytes, CString sName, DWORD access)
	{
		_sizebytes = sizebytes;
		if (_stream.Open(_sizebytes,sName, access) == false)
			_stream.AllocMemory(_sizebytes, sName);

		return _stream.MapViewAll(access);
	}

	virtual bool Init()
	{
		return true;
	}

	virtual void Exit()
	{
		_stream.UnmapView();
	}

	virtual T & At(int pos)
	{
		if (pos >= GetMaxSize())
			pos = pos % GetMaxSize();
	//	assert(pos < GetMaxSize());
		return ((T*)_stream.Byte())[pos];
	}

	T & operator [] (int pos)
	{
		return At(pos);
	}

	virtual T * Add(T * Data, int Count = 1)
	{
		int NewIndex = _UseCount;
		int nStartIndex(NewIndex);
		T * ptr = (T *)_stream.Byte();

		if ((NewIndex+ Count-1) >= GetMaxSize())
			NewIndex = 0;

		nStartIndex = NewIndex;

		for(int i=0; i<Count; i++)
		{			
			memcpy(&ptr[NewIndex], &Data[i], sizeof(T));
			NewIndex++;
		}
		_UseCount = NewIndex;
		_CurStIdx = nStartIndex;
		return &ptr[nStartIndex];
	}
	virtual void Clear()
	{
		_UseCount = 0;
	}

	//T * Next()
	//{
	//	assert(_UseCount < GetMaxSize());
	//	ExtMemory & This = *this;
	//	return &This[_UseCount];
	//}
	T * First()
	{
		return ((T*)_stream.Byte());
	}
	T *  FirstSize()
	{
		return ((T*)_stream.GetMemSize());
	}
	//T * Translate(T* data)
	//{
	//	return (T *)AddrCalc::Calc_Add_Offset(First(), data);
	//}
	//T * Translate(LPVOID data)
	//{
	//	return (T *)AddrCalc::Calc_Add_Offset(First(), data);
	//}

	virtual void UpdateAll()
	{
		_stream.Flush();
	}
	virtual void UpdateIdx(int pos)
	{
		if (pos >= GetMaxSize())
			pos = pos % GetMaxSize();
		_stream.Flush(&operator[](pos), sizeof(T));
	}
	virtual void UpdatePtr(T * ptr_extif)
	{
		int pos = GetIdxFromPtr(ptr_extif);

		_stream.Flush(&operator[](pos), sizeof(T));
	}

	int GetIdxFromPtr(T * ptr_extif)
	{
		return AddrCalc::Calc_Index(ptr_extif, sizeof(T));
	}
	T * GetItem(T * ptr_extif)
	{
		int pos = GetIdxFromPtr(ptr_extif);
		return &At(pos);
	}

protected:
	int GetMaxSize()
	{
		return _sizebytes / sizeof(T);
	}

	int _UseCount;
	int _CurStIdx;
	CSharedMemory _stream;
	QWORD _sizebytes;
};

class Ctrler : public Memory<CtrlServer>
{
public:
	Ctrler();

	virtual bool Init();

	void InitSignal();
	void SetSignal(int sig, bool bOn, bool bUpdate = true);
	bool GetSignal(int sig);
	DWORD GetProcessID();

	virtual void Clear();
	unsigned int _TotalBuff; 
};

class CtrlBuffer : public Memory<ClientBufferCtrl>
{
public:
	CtrlBuffer();

	bool Init(int nBufferCnt, int nMode);
	void Exit();
	virtual void Clear();

	void SetFlag(int sig, bool bUpdate = true);
	bool GetFlag(int sig);
	int m_nBuffIdx;
};

class Ctrlee : public Memory<CtrlClient>
{
public:
	Ctrlee();

	virtual bool Init(int nTool_id);

	void SetSignal(int sig, bool bOn, bool bUpdate = true);
	bool GetSignal(int sig);
	DWORD GetProcessID();
	unsigned int _TotalBuff;
	virtual void Clear();
};

class InspPannel : public Memory<InspStatus>
{
public:
	InspPannel();


	virtual bool Init();
	virtual void Clear();
	int ModelTotalCount(int nModelType = ModelBufferFlag::eExtModel_POCR);
	void Update(int pos);
	unsigned int _TotalBuff;
};
class ModelCtrl : public Memory<ModelStatus>
{
public:
	ModelCtrl(); //Ctrl 할 Class Eagle3D 소속 ( Tool 쪽은 Read 만 )
	virtual bool Init(int nModelIndex);
	virtual void Clear();
	unsigned int _TotalBuff;
};

class InspLogicBase
{
public:
	InspLogicBase();
	virtual bool Init();
	virtual bool Init(int nStep);
	virtual void Exit();

	int _MainSleep;
	int _WaitForDelete;

	virtual int GetCurrentStep();

protected:
	virtual void Begin();
	virtual void Proc();
	virtual void End();
	virtual void Proc_SaveImage();
	virtual void Proc_FrameSave();
	void NextStep(int Step);
	static UINT thProcedure(LPVOID param);
	static UINT thSaveImage(LPVOID param);
	static UINT thSaveFrame(LPVOID param);
	bool _bRunThread;
	CWinThread *_pThread;
	CWinThread *_pThreadImageSave;
	int _nMainSeqStep;
};

class CompareStrurct : public Memory<tagCompareStrurctData>
{
public:
	CompareStrurct();


	virtual bool Init();
	virtual void Exit();
	//virtual bool RunCompare(); //Compare 실행 

	void CompareInit(bool bServer = false);
	unsigned int _TotalBuff; 
	//void Update(int pos);
};
class MachineSetting : public Memory<tagMachineParam>
{
public:
	MachineSetting();


	virtual bool Init(int nTotalToolCnt);
	unsigned int _TotalBuff; 
};


class PreStartParam : public Memory<tagExtPreStartParam>
{
public:
	PreStartParam();

	virtual bool Init();
	bool SetData(int nFovSizeX, int nFovSizeY,double dResolution_X,double dResolution_Y);
};
class StartInit : public  Memory<tagExtStartInitStruct>
{
public:
	typedef std::shared_ptr <StartInit> Ptr;
public:
	StartInit();
	virtual bool Init();
	static Ptr get();

	bool SetData(ExtStartInitStruct st);
	bool GetData(ExtStartInitStruct* st);
protected:
	static Ptr s_Obj;
};

class FrameFlagWriter : public Memory<tagFrameWriteFlag>
{
public:
	typedef std::shared_ptr <FrameFlagWriter> Ptr;
public : 
	FrameFlagWriter() {};
	virtual void RunProgram(CString sAlgoToolPath);
	static Ptr get();
	virtual bool Init();	
	//virtual void Exit();
protected:
	static Ptr s_Obj;
 };
}