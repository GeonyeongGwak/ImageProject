#pragma once
#include "ExtInspRoot.h"
#include "InspParamDef.h"
#include "FOREIGN/InspParamDef_FR.h"
#include "PInsp.h"
#ifdef EXT_SERVER

#else
#include "PInsp_Algo/InspParamDef_Algo.h"
#endif
#include "./opencv2/opencv.hpp"
#include <unordered_map>
#include <optional>
#include "alp_file.h"
#define MAX_TIMELIST 20

// If it need, this and hierarchical classes can be changed in ring buffer structure.
namespace ext {
	struct tagSharedPartHeader
	{
		int m_nZerobyte;
		QWORD m_qwOffSet[10];
		tagSharedPartHeader()
		{
			m_nZerobyte = 0;
			for(int i = 0 ; i <10 ; i++ )
				m_qwOffSet[i] = 0;
		}
	};
	struct tagSharedForeignHeader
	{
		int m_nZerobyte;
		QWORD m_qwOffSet[100];
		tagSharedForeignHeader()
		{
			m_nZerobyte = 0;
			memset(m_qwOffSet, 0, sizeof(QWORD) * 100);
		}
	};
	struct tagSharedModelMap
	{
		QWORD vName[300];
		QWORD vData[300];
		int nStrLen[300];
		tagSharedModelMap()
		{
			for (int i = 0; i < 300; i++)
			{
				vName[i] = 0;
				vData[i] = 0;
				nStrLen[i] = 0;
			}
		}
	};
	typedef struct tagTackTimeTable
	{
		int nCurCnt;	//현재 검사 완료 횟수, 20회를 넘지 말아야함
		std::unordered_map<int, double> vList_Recent[MAX_TIMELIST];	//Recent 20회 시간, 매 검사 들어올때마다 원형큐 처럼 갱신
		std::unordered_map<int, double> vList_Mean;	//PartID, meanTack, Count
	}TackTimeTable;

	//new image
	class _ImageBuffer
	{
	public:
		_ImageBuffer();

		virtual bool Init(CString sSpaceName, int nImageType, int nToolType);
		void Push(void * ptr, int nW, int nH, int nCh);
		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		virtual void Exit();
		//void AddImages(InspRoiImgBuf * partImgBuf, ZmapData*  partZmapData);
		//virtual void Push(BYTE * ptr, int nW, int nH, int nCh); 
		virtual void Clear();

		QWORD _offset;
		QWORD _sizebytes;
		QWORD _qwCurPos;
		std::shared_ptr<CSharedMemory> _stream;
		int m_nChannel;
		int m_nToolType;
	};

	class _ImageMasterBuffer
	{
	public:
		_ImageMasterBuffer();

		bool Init(int nToolID,int nBufferIndex, int nToolType, int nMachineType);

		void AddImages(InspRoiImgBuf * partImgBuf, RoiColorBuf * partImgColorBuf, ZmapData * partZmapData, BYTE* by3DRawImg, int sz3DRaw,int nBufferIndex = -1);

		void GetImages(InspRoiImgBuf * partImgBuf, RoiColorBuf * partImgColorBuf, ZmapData * partZmapData, BYTE *& by3DRawImg, int sz3DRaw , int nBufferIndex = -1);

		void Exit();

		void Clear();

		int MachineTypeImageBufferCount(int nMachineType);

		_ImageBuffer _ChannelBuffer[(int)ext::eImageChannel::eNUM + 1];
		_ImageBuffer _ChannelAngleBuffer[(int)RoiColorBuf_no::eRoiColorBuf_img_R_BtmSide]; //AngleBuffer
		unsigned long long _TotalBuff;
	protected:
		int m_nTool_Id;
		int m_nToolType;
		int _nMachineType;
	};

	class LightParam : public Memory<tagExtAlgoLight>
	{
	public:
		LightParam();

		virtual bool Init(int nToolID);

		//virtual bool Init(int nToolID);

		virtual void Exit();
		int Get(InspAlgo* pAlgo);
		void AddLight(InspPartParam* pWnd, int nWnd);
		void GetLight(InspPartParam*& pWnd, int nWnd);
		void SetPosZero() { _qwCurPos = 0; };
	public:
		int m_nBuffIdx;
		unsigned int _TotalBuff;
	protected:
		int Add(InspAlgo* pAlgo);

		//ExtAlgoLight * Add();
		QWORD _qwCurPos;
	};
	class StringData
	{
	public:
		StringData();

		virtual bool Init(int nToolID);

		virtual void Exit();

		int Get(InspAlgo* pAlgo);

		QWORD AddData(CString sStr, int& nSize);

		wchar_t* GetData(QWORD qwName, tagSharedModelMap* stModel);
		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		virtual void Clear();

	public:
		int m_nBuffIdx;
	protected:
		QWORD _sizebytes;
		QWORD _qwCurPos;
		std::shared_ptr<CSharedMemory> _stream;
	};

// 	class StringMap
// 	{
// 	public:
// 		StringMap();
// 
// 		virtual bool Init(int nToolID);
// 
// 		virtual void Exit();
// 
// 		int Get(InspAlgo* pAlgo);
// 
// 		void AddStringPath(InspPartParam* pWnd, int nWindowCnt);
// 		void GetStringPath(InspPartParam* pWnd, int nWindowCnt);
// 
// 		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
// 		virtual void Clear();
// 		StringData _SData;
// 	public:
// 		int m_nBuffIdx;
// 	protected:
// 		QWORD _sizebytes;
// 		QWORD _qwCurPos;
// 		std::shared_ptr<CSharedMemory> _stream;
// 	};
	//new part
	class _Part : public Memory<InspPartInfo>
	{
	public:
		_Part();

		virtual bool Init(int nToolID);

		virtual void Exit();

		//virtual InspPartInfo & At(int pos);

		bool AddInspData(InspPartInfo* PartData, InspPartParam* pWnd, int nWndCnt);
		bool AddPart(InspPartInfo * PartData);
		bool AddWindow(InspPartParam* pWnd, int nWndCnt);
		bool AddCommonAlgo(InspPartParam* pWnd, int nWndCnt);
		bool AddArrMaskingROI(InspPartParam * pWnd, int nWnd);
		
		bool AddAlgo(InspPartParam* pWnd, int nWndCnt);
		int AlgoClone(InspAlgoType eAlgoType, LPVOID& lpSource);
		bool AddPartPad(InspPartInfo * PartData);

		bool GetPartData(InspPartInfo * pDstPart, InspPartParam *& pDstWindow);
		int SetAlgoData(InspAlgoType eAlgoType, LPVOID& lpDst, QWORD qwOffset);
		int SetArrMaskingROIData(InspAlgo * pAlgo, QWORD qwOffSet);
		void CopyHeader();
		LightParam _Light;
	//	StringMap _Map;
		QWORD GetMemOffSet(int nIdx = 0) { return m_qwArrOffset[nIdx]; };
	public:
		int m_nBuffIdx;
		unsigned int _TotalBuff;

	protected:
		//InspPartInfo * Add(InspPartInfo * PartData, int Count = 1);
		QWORD _qwCurPos;
		QWORD m_qwArrOffset[10];
	};
	class ModelImageBuffer
	{
	public:
		ModelImageBuffer();
		virtual void Create(QWORD sizebytes, CString sName, DWORD access);
		virtual bool Init(int nModelIndex);
		virtual void Exit();
		virtual bool ReInit(long long _llSize);
		ImgData Push(BYTE * ptr, int Wid, int Len, int Ch, int widthbytes);
		ImgData Push(BYTE * ptr, int _nSize, int widthbytes);
		ImgData FilePush(CFile * file);
		BYTE * Lock(ImgData pos);
		void Clear();
		QWORD _offset;
		QWORD _sizebytes;
		std::shared_ptr<CSharedMemory> _stream;
		unsigned int _TotalBuff;
	};
	class _ForeignDataResult
	{
	public:
		_ForeignDataResult();
		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		virtual bool Init(int nToolID);
		virtual void Exit();
		void AddForeignRstData(InspFovForeignResult * ForeignRst);
		void AddForeignRstHeader();
		void AddForeignRstFovForeign(InspFovForeignResult * ForeignRst , int ret);
		int GetForeignRstData(InspFovForeignResult *& ForeignRst);
		void GetForeignRstHeader();
		bool GetNext(InspFovForeignResult *& rst,/* int nCalcDoneCnt,*/ int * nExtDefectCode);
		void PointerPush(int nTyoeSize, int nDataSize, void * InputData, int nMemorySnapShotIndex);
		BYTE * GettingPointer(QWORD Value);
		QWORD _offset;
		QWORD _sizebytes; 
		QWORD _qwCurPos;// CurPosition
		QWORD m_qwArrOffset[100]; //Pointer Position
		std::shared_ptr<CSharedMemory> _stream;
		int m_nBuffIdx;
		int _nRecvRstCount;
	};

	class _ForeignData
	{
	public:
		_ForeignData();
		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		virtual bool Init(int nToolID);
		virtual void Exit();
		void AddInspForeign(InspPartInfo * _ForePartInfo, ForeignData _foreinData, FR2DData Fr2Data, InspForeignInfo m_ForeignInfoData, ForeignParamROI* _ForeParamROI, ForeignParamROI* arrRoi);
		void AddForeignInfoData(InspForeignInfo m_ForeignInfoData, InspPartInfo * _ForePartInfo, ForeignParamROI* _ForeParamROI, ForeignParamROI* arrROI);
		
		void AddForeignHeader();
		void AddForeignData(ForeignData _foreinData);
		void AddFR2DData(FR2DData Fr2Data);
		void AddArrROI(ForeignParamROI* arrROI,int nSize)
		{
			PointerPush(sizeof(ForeignParamROI), nSize, arrROI, eForeignInfoData::eForeignInspPartInfo_arrROIStruct);
		};
			
	

		void GetnspForeign(InspPartInfo * _ForePartInfo, ForeignData * _foreinData, FR2DData * Fr2Data, InspForeignInfo * m_ForeignInfoData, ForeignParamROI ** _ForeParamROI, ForeignParamROI ** arrROI);
		void GetForeignData(ForeignData* _foreinData);
		void GetFR2DData(FR2DData* Fr2Data);
		void GetNGTempData(UCHAR* TR, UCHAR* TG, UCHAR* TB, UCHAR* BR, UCHAR* BB);
		void GetForeignInfoData(InspForeignInfo * m_ForeignInfoData, InspPartInfo * _ForePartInfo, ForeignParamROI ** _ForeParamROI);
		void GetForeignPartInfoData(InspPartInfo * _ForePartInfo, bool bSave3D);
		void GetArrRoiData(ForeignParamROI ** arrROI)
		{
			*arrROI = &*(ForeignParamROI*)GettingPointer(m_qwArrOffset[eForeignInfoData::eForeignInspPartInfo_arrROIStruct]);
		};
		void GetForeignHeader();

		//Add Option 
		void PointerPush(int nTyoeSize, int nDataSize, void * InputData, int nMemorySnapShotIndex = -1);//SnapShot 비어있을경우 CurPos까지만 업데이트 
		BYTE * GettingPointer(QWORD Value);
		void PushForeign(void * Inputptr, int nW, int nH, int nCh, int nMemorySnapShotIndex = -1);
		//Get 

	protected:
		QWORD _qwCurPos;// CurPosition
		QWORD m_qwArrOffset[100]; //Pointer Position
	public:
		QWORD _offset;
		QWORD _sizebytes;
		std::shared_ptr<CSharedMemory> _stream;
		int m_nBuffIdx; 
	};

	class _ForeignDataWP
	{
	public:
		_ForeignDataWP();
		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		virtual bool Init(int nToolID);
		virtual void Exit();
		void AddForeignHeader();
		void AddInspWarpageImage(ForeignData _foreinData);
		void GetForeignHeader();
		void GetInspWarpageImage(ForeignData* _foreinData, bool bLastCpy = false);
		//Add Option 
		void PointerPush(int nTyoeSize, int nDataSize, void * InputData, int nMemorySnapShotIndex = -1);//SnapShot 비어있을경우 CurPos까지만 업데이트 
		BYTE * GettingPointer(QWORD Value);
		void PushForeign(void * Inputptr, int nW, int nH, int nCh, int nMemorySnapShotIndex = -1);
		void PushForeignCpy(void * Inputptr, int nW, int nH, int nCh, void * OutputPtr);
		//Get 

	protected:
		QWORD _qwCurPos;// CurPosition
		QWORD m_qwArrOffset[100]; //Pointer Position
	public:
		QWORD _offset;
		QWORD _sizebytes;
		std::shared_ptr<CSharedMemory> _stream;
		int m_nBuffIdx;
	};
	//new Result
	class _PartRst : public Memory<InspectionResult>
	{
	public:
		_PartRst();
	
		virtual bool Init(int nToolID);

		virtual void Exit();

		bool GetNext(InspectionResult *& rst, int nCalcDoneCnt,int *nExtDefectCode);

		int RemainCount();

		int AddResult(InspectionResult * PartData, int nDefectCode);
		int GetResult(InspectionResult * PartData);
		LPVOID* SetWndTempRst(int nType, InspectionResult * lpDst, QWORD qwOffSet, bool bTemp);
		int SetAlgoData(InspAlgoType eAlgoType, LPVOID & lpDst, QWORD qwOffSet);
		int SetArrMaskingROIData(InspAlgo * pAlgo, QWORD qwOffSet);
		int SetAlgoRstData(InspAlgoType eAlgoType, LPVOID & lpDst, QWORD qwOffSet);
		int AddPartResult(InspectionResult * PartData);
		int AddWndResult(InspWndResult * pWndRst, int nWndCnt);
		//int AddWnd_T(T * pWndRst, int nWndCnt);
		int AddCommonAlgoResult(InspWndResult * pWndRst, int nWndCnt);
		int AddAlgoResult(InspWndResult * pWndRst, int nWndCnt);

		

		
		int AddPartTempResult(InspParamTemp * pParamTemp, int nWndCnt);
		int AddCommonAlgo(InspParamTemp * pParamTemp, int nWndCnt);
		int AddAlgo(InspParamTemp * pParamTemp, int nWndCnt);
		int AddArrMaskingROI(InspParamTemp * pParamTemp, int nWndCnt);

		int CopyHeader();

		int InspDoneCnt();

		/*WindowRsts _Windows;
		ParamTempRst _ParamTemp;*/
	public:
		int m_nBuffIdx;
		int _nRecvRstCount;
		Ctrlee * _ctrl;
		unsigned int _TotalBuff;
	protected:
		//InspectionResult * Add(InspectionResult * PartData, int Count = 1);
		int AlgoCloneRst(InspAlgoType eAlgoType, LPVOID lpSource, LPVOID & lpTarget);
		int AlgoCloneTemp(InspAlgoType eAlgoType, LPVOID lpSource, LPVOID & lpTarget);
		QWORD _qwCurPos;
		QWORD m_qwArrOffset[10]; // 9 번째는 nDefectCode로 
	};
	
	class InspRoot
	{
	public:
		InspRoot();
		virtual bool Init();
		virtual void Exit();
		virtual void Clear()
		{

		}
		virtual bool bInitialized();
		
		void AddPannelCount();
		int GetCurrentPannelCount() { return m_nCurrentIdx % env::nPartBufCount; };
		
		void CheckStAlgoSize();

		//JobData _Job;
		ModelCtrl _POCRModelCtrl; // 배열로 사용가능.
		ModelImageBuffer _POCRModelBuffer;
		ModelCtrl _PatternModelCtrl; // 배열로 사용가능.
		ModelImageBuffer _PatternModelBuffer;
		ModelCtrl _COBModelCtrl; // ¹e¿­·I ≫c¿e°¡´E.
		ModelImageBuffer _COBModelBuffer;
		ModelCtrl _ExcModelCtrl; // ¹e¿­·I ≫c¿e°¡´E.
		ModelImageBuffer _ExcModelBuffer;
		ModelCtrl _PadBWModelCtrl; // ¹e¿­·I ≫c¿e°¡´E.
		ModelImageBuffer _PadBWModelBuffer;
		ModelCtrl _ShapeXModelCtrl; // ¹e¿­·I ≫c¿e°¡´E.
		ModelImageBuffer _ShapeXModelBuffer;
		ModelCtrl _HalconPOCRModelCtrl;// Halcon POCR 은 파일로부터 불러오기때문에 Buffer 를 따로가지고있지않음 
		ModelCtrl _ExcNGBlobModelCtrl; 
		ModelImageBuffer _ExcNGBlobModelBuffer;
		InspPannel _InspStatus;


		bool ExtModelAdd(CFile * fInput,CString strFileName, int nModelType = ext::ModelBufferFlag::eExtModel_POCR);
		void ModelClear(int nModelType);
		bool ExtMatImageAdd(cv::Mat * fInput, CString sName, int nModelType = ext::ModelBufferFlag::eExtModel_COB);
		bool GetExtMatImage(cv::Mat * fInput, CString sName, int nModelType = ext::ModelBufferFlag::eExtModel_COB);
		bool stringCompare(wchar_t * strSrc, ModelCtrl * Models, int nIndex, int  * nOutModelIndex = NULL);
		bool FileAndModelNameCompare(wchar_t * strSrc, ModelCtrl * Models, int nIndex, int fileSize);
		std::map<int, int> _PartIndex;
	public:
		// parameters...

	protected:
		bool _bInitialized;
		int m_nCurrentIdx;
		unsigned long long _TotalBuff;
	};

	class ClientCtrl 
	{
 	public:
 		typedef std::shared_ptr <ClientCtrl> Ptr;
	public:
		ClientCtrl();
		static Ptr get();
		bool Init(int nTool_id,int nMachineType);
		void Exit();

		void Exit_Part(int nToolType);
		void Exit_Foreign();

		void Execute();
		bool GetSharedData(InspPartInfo * pPart, InspPartParam *& pWnd, int nSelectedIdx, int nBufferIndex = -1);
		void SelectBuffer(wchar_t * sPath);
	
		void AddResultData(InspectionResult * PartData, int nSelectedIdx);
		void GetResultData(InspectionResult * PartData, int nSelectedIdx);
		bool AddSharedData(InspPartInfo* pPart, InspPartParam* pWnd, int nSelectedIdx, int nBufferIndex = -1);

		bool NormalTypeInit(int nTool_id,int nMachineType);
		bool BigTypeInit(int nTool_id, int nMachineType);
		bool ForeignTypeInit(int nTool_id, int nMachineType);

		void BufferClear();
		void BigBufferClear();

		Ctrlee  _ctrl;
		CompareStrurct _AlgoCompare;
		CtrlBuffer		_SndCtrl[EXT_BUFFER_CNT];
		CtrlBuffer		_RevCtrl[EXT_BUFFER_RST_CNT];

		_ImageMasterBuffer	_MasterBuffer[EXT_BUFFER_CNT];
		_Part			_PartData[EXT_BUFFER_CNT];
		_PartRst		_Rst[EXT_BUFFER_RST_CNT];
		LightParam		_light[EXT_BUFFER_CNT];

		//FOREIGN
		CtrlBuffer		_SndForeignCtrl[EXT_BUFFER_FOREIGN_CNT];
		CtrlBuffer		_RevForeignCtrl[EXT_BUFFER_FOREIGN_RST_CNT];
		_ForeignData    _ForeignShared[EXT_BUFFER_FOREIGN_CNT]; // FOV 갯수
		_ForeignDataWP    _ForeignWP; // Board 갯수 
		_ForeignDataResult _ForeignSheradRst[EXT_BUFFER_FOREIGN_RST_CNT];
		bool AddShared_ForeignData(InspPartInfo * _ForePartInfo, ForeignData _foreinData, FR2DData Fr2Data, InspForeignInfo foreignInfo, ForeignParamROI* _ForeParamROI, ForeignParamROI* arrROI, int nSelectedIdx);
		bool GetShared_ForeignData(InspPartInfo * _ForePartInfo, ForeignData * _foreinData, FR2DData * Fr2Data, InspForeignInfo * m_ForeignInfoData, ForeignParamROI ** _ForeParamROI, ForeignParamROI** arrROI, int nSelectedIdx);

		std::map<QWORD, CString> m_stExtModelPath;
		void ClearExtModelPath() { m_stExtModelPath.clear(); };
		void InsertExtModelPath(QWORD qwName, CString sPath);
		CString FindExtModelPath(QWORD qwName);

		double m_dExcpectInspTime;
		void AddExcpectTime(int nBufferIdx, int nLane, int nPartID);

		void GetImageDump(InspPartInfo * pPart, InspPartParam * pWnd, SaveImgParamData * tg, int nBufferIndex);
		void CreateDir(CString Path);
		BOOL IsExistDir(CString path);
		int m_nCurrBufferIdx;

		int _nTool_id;

		int m_nToolType;
		int _MachineType;
		int _InputBufferIndex; // Minidump용
		int _nW; //image Write 용 사용하지말것. Minidump용
		int _nH; //image Write 용 사용하지말것. Minidump용
		int InspFovGetIndex;
		std::vector< int > InspCalcFov; 

		//Server용 변수들
		ZmapData m_MultiTempZmapData[EXT_BUFFER_FOREIGN_CNT]; // MultiProcess 3D image Buffer 
		tagRoiImgBuf  m_MultiRoiImgBuf[EXT_BUFFER_FOREIGN_CNT];
		unsigned long long _TotalBuff;
	protected:
		static Ptr s_Obj;
	};

	class InspRoot_Server : public InspRoot
	{
	public:
		typedef std::shared_ptr <InspRoot_Server> Ptr;
	public:
		InspRoot_Server();
		virtual ~InspRoot_Server();
		static Ptr get();
		static bool isLive();

		virtual bool Init(int nMachineType);
		virtual void Exit();
		virtual void Clear();
		void SetCoreNum();
		int GetRemainRstCount();
		bool GetPartResult(int nLane,int* nExtDefectCode, InspectionResult* rst);
		int GetRemainForeignRstCount();
		bool GetClientDone();
		int GetInspDoneCnt();
		int GetInspImageSaveCnt();
		int GetForeignDoneCnt();
		int GetSaveCnt();
		int GetNextToolIndex(bool bIsBigPart, bool bUseSchdule, int nLane, int nPartID, double& dExpectTime, int& nBuffer);
		int GetScheduleIndex();

		bool GetForeignResult(int* nSelectBuffer, int* nExtDefectCode, InspFovForeignResult*& rst);
		Ctrler _Ctrl;
		bool bUseMultiProcess;
		std::vector<ClientCtrl> _Tools;
		MachineSetting _SMachineData;
		PreStartParam _stPreStartParam;
		std::vector<int> m_vToolIndexSort;
		std::vector<int> m_vToolForeign;
		int _nFov;

	protected:
		static Ptr s_Obj;
	};

	typedef InspRoot_Server irs;

	class InspRoot_Client : public InspRoot
	{
	public:
		typedef std::shared_ptr <InspRoot_Client> Ptr;
	public:
		InspRoot_Client();
		virtual ~InspRoot_Client();
		static Ptr get();

		virtual bool Init();
		virtual void Exit();

		Ctrler _CtrlServer;
		Ctrlee _Ctrl;
		//PartRst _Result;

		MachineSetting _CMachineData;
		unsigned long long _TotalBuff; 
	protected:
		static Ptr s_Obj;
	};

	typedef InspRoot_Client irc;

	class InspSchdule
	{
	public:
		typedef std::shared_ptr <InspSchdule> Ptr;
	public:
		void init();
		void Exit();
		void Clear(int nLane);
		void SetList(int nLane, vector<int> vPartID);
		void UpdatePartTime(int nLane, int nPartID, double dMiliSec);
		void UpdateExtInspDoneCnt(int nLane);
		double GetExpectTime(int nLane, int nPartID);
		BOOL GetFirstInsp(int nLane);

		static Ptr get();
	private:
		std::vector<TackTimeTable> m_vTimeList;
	protected:
		static Ptr s_Obj;
	};

	typedef InspSchdule Sch;
	void CopyParamToExt(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize);
	class ImageStack
	{
	public:
		void push(const std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool, int >  TupleSaveImages)
		{
			std::unique_lock<std::mutex> lock(mtx);
			images.push_back(TupleSaveImages);
			imagesBuffer.push_back(std::get<4>(TupleSaveImages)); //BufferIndex
			conv.notify_one();
		}

		std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool, int >  pop()
		{
			std::unique_lock<std::mutex> lock(mtx);
			conv.wait(lock, [this]() { return !images.empty(); });
			auto imageData = std::move(images.front()); // std::move를 사용하여 불필요한 복사 방지
			images.pop_front(); // deque의 앞쪽에서 제거
			imagesBuffer.pop_front();
			return imageData;
		}
		int Count()
		{
			std::lock_guard<std::mutex> lock(mtx);
			return images.size();
		}
		bool Contains(int needle) const 
		{
			std::lock_guard<std::mutex> lock(mtx);
			return std::find(imagesBuffer.begin(), imagesBuffer.end(), needle) != imagesBuffer.end();
		}
		void ImageBufferClear()
		{
			imagesBuffer.clear();
		}

	private:
		std::deque<std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool , int > > images;
		std::deque<int >  imagesBuffer;
		mutable std::mutex mtx;
		std::condition_variable conv;
	};

	struct stFrameHeader
	{
		int nFrameIndex;
		int nFrameCnt;
		wchar_t strFrameName[256];
		HEADER_BUK stHeader;
	};
	class FrameWriter
	{
	public:
		FrameWriter() {};
		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		virtual bool Init(int nFrameIndex);
		virtual void Exit() ;
		void GetData();
		void GetData(PUINT8* pFrames);
		void SetData(PUINT8* pFrame, int nFrameCnt, HEADER_BUK bukHeader);
		//void SetData();
		std::shared_ptr<CSharedMemory> _stream;
		QWORD _offset;
		QWORD _sizebytes;
		const int _nFamesCnt = 40;
		QWORD _qwCurPos;
		int _nBufferIndefx;
	};

	class FrameWriters
	{
	public: 
		FrameWriters() {};
		stFrameHeader _BukHeader[EXT_FRAMEBUFFER];
		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		virtual bool Init();
		void GetData(PUINT8 * pFrames, int nSelectBuffer);
		virtual void Exit() ;
		void SetData(int nFrameIndex, CString strFrameName, HEADER_BUK bukHeader, int nFrameCnt, PUINT8* pFrame);
		FrameWriter _FrameWriter[EXT_FRAMEBUFFER];
		QWORD _offset;
		QWORD _sizebytes;
		std::shared_ptr<CSharedMemory> _stream;
		QWORD _qwCurPos;
		typedef std::shared_ptr <FrameWriters> Ptr;
		static Ptr get();
	protected:
		static Ptr s_Obj;
	};
	struct stMatHeader
	{
		int nMatIndex;
		int nPartID;        // strMatName 대신 nPartID 사용
		int nWidth;
		int nHeight;
		int nChannels;
		int nType;          // CV_8UC1, CV_32FC1 등
		QWORD qwDataSize;
	};

	// 단일 cv::Mat 버퍼
	class MatData
	{
	public:
		MatData();
		virtual ~MatData() = default;

		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		bool Init(int nMatIndex);
		void Exit();

		void SetData(const cv::Mat& mat);
		void GetData(cv::Mat& mat, int nWidth, int nHeight, int nType);
		BYTE* GetDataPtr();

	protected:
		std::shared_ptr<CSharedMemory> _stream;
		QWORD _sizebytes;
		QWORD _qwCurPos;
		int _nBufferIndex;
		unsigned long long _TotalBuff;
	};

	// 다중 cv::Mat 관리 클래스
#define EXT_MAT_BUFFER 20  // 저장 가능한 Mat 개수

	class MatDatas
	{
	public:
		typedef std::shared_ptr<MatDatas> Ptr;
		static Ptr s_Obj;
		static Ptr get();

		MatDatas();
		virtual ~MatDatas() = default;

		virtual bool Create(QWORD sizebytes, CString sName, DWORD access);
		bool Init();
		void Exit();
		void Clear();

		// Mat 데이터 설정/획득 (nPartID 기반)
		bool SetData(int nMatIndex, int nPartID, const cv::Mat& mat);
		bool GetData(int nMatIndex, cv::Mat& mat);
		bool GetDataByPartID(int nPartID, cv::Mat& mat);
		// 헤더 정보 획득
		stMatHeader GetHeader(int nMatIndex);
		int GetMatCount() const { return _nMatCount; }
		int GetMatIndex() const
		{
			if (_nMatCount < 0 || _nMatCount >= EXT_MAT_BUFFER)
				return 0;
			return _nMatCount; 
		}
		// PartID로 인덱스 찾기
		int FindIndexByPartID(int nPartID);

	protected:
		std::shared_ptr<CSharedMemory> _stream;
		QWORD _sizebytes;
		QWORD _qwCurPos;

		MatData _MatData[EXT_MAT_BUFFER];
		stMatHeader _MatHeader[EXT_MAT_BUFFER];
		int _nMatCount;
		unsigned long long _TotalBuff;
	};
}