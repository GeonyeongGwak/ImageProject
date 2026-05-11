#pragma once
#include "ExtInspSub.h"
#include "ExtLog.h"
namespace ext
{

class InspLogic : public InspLogicBase
{
public:
	enum SeqCase
	{
		ReadyToAct=100,
		CheckToAuto=200,
		PreStart = 210,
		InspProc =300,
		InspProc1 = 310,
		InspProc2 = 320,
		InspProc3 = 330,
		InspProc4 = 340,
		InspProc5 = 350,
		InspProc6 = 360,
		CheckToFinish = 400,
		LoopFrameWrier = 500,
	};
	enum AlgoStartEnd
	{
		eAlgoStart = 0 ,
		eAlgoEnd
	};
protected:

	virtual void Begin();
	virtual void Proc();
	virtual void Proc_SaveImage();
	virtual void Proc_FrameSave();
	void FrameWriteData();
	void FrameWriteData2();
	bool CheckEmergencyStop();
	void Ready();
	bool Inspect();
	bool PreStarting();
	bool Inspect2();	//가칭 추후 inspect1과 교체 예정
	bool TryInspection();
	bool InspectAlgoTypeTack(InspAlgoType algotype, AlgoStartEnd eStartEnd);
	virtual void End();


	bool DeleteManager();

};


class InspLogicRoot
{
public:
	typedef std::shared_ptr <InspLogicRoot> Ptr;
public:
	InspLogicRoot()
	{

	}
	virtual ~InspLogicRoot()
	{

	}
	static Ptr get();

public:
	bool Init();
	bool Init(int n);
	void Exit();

protected:

	InspLogic _Insp;
	static Ptr s_Obj;
};
typedef InspLogicRoot logic;


bool IsProcessRunning(DWORD pid);
bool IsProcessRunningMemoryView();
bool IsMainProcessCheck();
}

