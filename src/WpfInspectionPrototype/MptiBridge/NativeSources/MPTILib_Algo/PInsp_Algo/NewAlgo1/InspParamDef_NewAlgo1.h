#pragma once
#include "../InspParamDef_AlgoBase.h"

//★★ 코드 작성 필요
enum m_eNewAlgo1Data
{
	m_eNewAlgo1Data_Use2D = 0x01,
	m_eNewAlgo1Data_Use3D = 0x02,
	m_eNewAlgo1Data_Shift = 0x04,
	m_eNewAlgo1Data_Angle = 0x08,
	m_eNewAlgo1Data_Total = 0x0F,
};
enum m_eNewAlgo1N
{
	m_eNewAlgo1N_Data,
	m_eNewAlgo1N_Range2D,
	m_eNewAlgo1N_Range3D,
	m_eNewAlgo1N_Min2D,
	m_eNewAlgo1N_Max2D,
	m_eNewAlgo1N_Total,
};
enum m_eNewAlgo1F
{
	m_eNewAlgo1F_Min3D,
	m_eNewAlgo1F_Max3D,
	m_eNewAlgo1F_Angle,
	m_eNewAlgo1F_ShiftX,
	m_eNewAlgo1F_ShiftY,
	m_eNewAlgo1F_Total,
};
enum m_eNewAlgo1PO
{
	m_eNewAlgo1PO_Teach1,
	m_eNewAlgo1PO_Teach2,
	m_eNewAlgo1PO_Size,
	m_eNewAlgo1PO_Total,
};

typedef struct tagAlgoNewAlgo1
{
	//★★ 코드 수정 필요
	int m_ArrDataN[m_eNewAlgo1N_Total];
	float m_ArrDataF[m_eNewAlgo1F_Total];
	POINTF m_ArrDataPO[m_eNewAlgo1PO_Total];
	tagAlgoNewAlgo1()
	{
		memset(m_ArrDataN, 0, m_eNewAlgo1N_Total * sizeof(int));
		memset(m_ArrDataF, 0, m_eNewAlgo1F_Total * sizeof(float));
		memset(m_ArrDataPO, 0, m_eNewAlgo1PO_Total * sizeof(POINTF));
	}
}AlgoNewAlgo1;

typedef struct tagRstAlgoNewAlgo1 // NewAlgo1 검사 결과
{
	//★★ 코드 수정 필요
	BOOL m_bOKA;
	BOOL m_bOKShift;
	float m_fRstA;
	float m_fRstShiftX;
	float m_fRstShiftY;
	POINTF m_poXY[3];
	tagRstAlgoNewAlgo1()
	{
		m_bOKA = FALSE;
		m_bOKShift = FALSE;
		m_fRstA = 0.0f;
		m_fRstShiftX = 0.0f;
		m_fRstShiftY = 0.0f;
		memset(m_poXY, 0, 3 * sizeof(POINTF));
	}
	void Init()
	{
		m_bOKA = FALSE;
		m_bOKShift = FALSE;
		m_fRstA = 0.0f;
		m_fRstShiftX = 0.0f;
		m_fRstShiftY = 0.0f;
		memset(m_poXY, 0, 3 * sizeof(POINTF));
	}
}RstAlgoNewAlgo1;