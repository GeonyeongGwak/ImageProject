#pragma once
#define NoSolderStandardValue 5
// 20140913 SHW : IPP 평균 구하기에 사용
#include "stdafx.h"
#include "ipps.h"
#include "ippi.h"
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")
#include "../MPTI.h"
#include <limits>
#include <float.h>
//#include "../ProcMil.h"
#include <immintrin.h>
#include<cmath>
#include<array>
#include "../Ransac.h"
#include <vector>

enum m_eInspWindowArea_Type
{
	m_eInspWindowArea_Type_None = 0,
	m_eInspWindowArea_Type_Lead,
	m_eInspWindowArea_Type_ChipTracking,
};

//#include "../PInsp.h"
//#include "../PInsp_Algo.h"

class IPINSP_ALGO
{
private:

public:
	IPINSP_ALGO();
	virtual ~IPINSP_ALGO();
	virtual void InitAlgo() abstract;
	virtual unsigned long long GetInspAlgoData() abstract;
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect) abstract;
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None) abstract;
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr) abstract;
	
	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr) abstract;
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true) abstract;
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1) abstract;
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType) abstract;

#pragma region  Common Func

	CPInsp_Algo* m_pCPInsp_Algo;
	CProc3D m_proc3d;
	CProcMil *m_pProcMilAlgo;
	double m_resolX;
	double m_resolY;

	int InspAC(AngleColorBase sAlgo, InspRoiImgBuf &sInspImg, UCHAR *pucBWImage, WndAlgoImg &sWndAlgo, int nDir, UCHAR *pucRstAC, float* fArrRstRGB, RECT *rcArea, bool bTeach, POINTF poCenter);
	float GetInspACCJ(AngleColorBase sAlgo, UCHAR *pucRstAC, WndAlgoImg &sWndAlgo, int nAC, RECT *rcArea, float* fArrRstRGB, int nDir, POINTF poCenter);
	int InspAC(int nInspAC_T, float* fArrRstRGB, bool bRsx = false);
	void InspAC(UCHAR *pucRstAC, int nIndex, float* fArrRstRGB);

#pragma endregion
};
