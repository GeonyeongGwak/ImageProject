#pragma once


//#include "./PInsp_Algo/InspParamDef_AlgoBase.h"
/*#include "InspManager.h"*/
//#include "InspParamDef.h"
#include "Manager.h"
//#include "Proc3D.h"
#include "AlgoFactory.h"

/*#include "ExcelManager.h"*/

enum decision_stepID
{
	eDCS_STEP1 = 0,
	eDCS_STEP2,
	eDCS_STEP3,
	eDCS_STEP4,
	eDCS_STEP5,
	eDCS_STEP6,
	eDCS_STEP7,
	eDCS_STEP_COUNT
};
class CPInspDecision
{
private:
	CProc3D m_proc3D;
	CString m_className;
private:
	InspectionResult* m_pInspResult;
	int** m_pOkNg;


	//RstInspBGA_Grp m_pInspRstBGA;


	int* m_groupIndexCnts; //검사 항목별 그룹 종류 개수.
	int** m_groupIndex; //검사 항목별 가지고 있는 그룹 인덱스
	int** m_groupWndID;//[MAX_GROUP_CNTS][MAX_WINDOW_CNTS];  //그룹 인덱스 별 윈도우 번호

	int* m_groupWndCnts; // 그룹에 속해있는 윈도우 개수.
	InspPartParam *m_pParamArray;
	int m_nArrPriorityCode[kindofWholeNGTypeTotalNum];
public:
	CPInspDecision(void);
	~CPInspDecision(void);
private:
	void SetInspResult(InspectionResult* result, int** okng);

	int MountDecision(RstInspMount* rst, int* isOK, int cnt);
	int OcrDecision(RstInspOCR* rst, int* isOK, int cnt);
	int ColorDecision(RstInspColor* rst, int* isOK, int cnt);
	int PatternDecision(RstInspPattern* rst, int* isOK, int cnt);

	int ProcStep(int stepID);

	int DecisionStep1();
	int DecisionStep2();
	int DecisionStep3();
	int DecisionStep4();
	int DecisionStep5();
	int DecisionStep6();
	int DecisionStep7();



	//void InitResultStruct(RstInspBGA_Grp* rst);


	//void CalcCoplanarity(InspParamBGA_Part param, InspectionResult* result, float* retMax, float* retMin, float* retT);

public:

	unsigned long long m_nSPCAlgoType[eAlgoNum];

	void SetParamInfo(int* groupIndexCnts, int* groupWndCnts, int** groupIndex, int** groupWndID);
	void SetParamInfo(InspPartParam *pParamArray);
	int InspDecision(InspectionResult* result, int** defectTemp, int &nUserSub, bool bRecheck = false);
	//int InspDecision_BGA(InspParamBGA_Part param, InspectionResult* result, int* defectTemp);

	void GetInspRst_BGA(RstInspBGA_Grp* dst);

	int PriorityNGDefectCode(int* arrDef, int* nkindofWholeNGType);
	int InitkindofWholeNGType(int* Source, int nTarget);
	int InitkindofWholeNGType(int* Source, int nTarget, NG_Type Specific);
	int MatchDefcodetoWholeNgType(int nDefCode, int* ngType);
	void SetPriorityCode(int* nArrCode);
	int GetPriorityCode(int* nType, int &nDefectType, BOOL bTipFaultNG);
	int GetDefectCode(int nCode, int &nDefectType, BOOL bTipFaultNG = TRUE);
	int GetPriorityCode(int nCodeOrg, int nCode);
	//excel
public:
	//void InitialExcel();
	//void CloseExcelDev();
	//void BGAResultSave_EXCEL(CString strFileName,int nSheetNum, InspectionResult* result, BOOL mode);
	//void BGAResultSave_CSV(InspectionResult* result);
	bool CheckDefectWnd(int nCurrentNg, int nBeforeNg);
	bool CheckDefectAlgoKind(int curNgType, int wndDefectType);
	int AlgoJudgment(int nWndType, InspAlgoType eAlgoType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect = false);
	unsigned long long GetInspAlgoData(InspAlgoType eAlgo);
};




