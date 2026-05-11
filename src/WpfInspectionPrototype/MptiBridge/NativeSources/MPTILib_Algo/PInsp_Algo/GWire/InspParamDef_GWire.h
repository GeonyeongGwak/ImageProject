#pragma once
#include "../InspParamDef_AlgoBase.h"

//★★ 코드 작성 필요
enum m_eGWireData
{
	m_eGWireData_Use2D = 0x01,
	m_eGWireData_Use3D = 0x02,
	m_eGWireData_Filter = 0x04,
	m_eGWireData_FillHoll = 0x08,
	m_eGWireData_CrossFilter = 0x10,
	m_eGWireData_UseCenterMargin = 0x20,
	m_eGWireData_Total = 0xFF,
};
enum m_eGWireN
{
	m_eGWireN_Data,
	m_eGWireN_Range2D,
	m_eGWireN_Range3D,
	m_eGWireN_Min2D,
	m_eGWireN_Max2D,
	m_eGWireN_Filter,
	m_eGWireN_Type,
	m_eGWireN_Direction,
	m_eGWireN_WireCount,
	m_eGWireN_CenterMargin,
	m_eGWireN_Total,
};
enum m_eGWireF
{
	m_eGWireF_Min3D,
	m_eGWireF_Max3D,
	m_eGWireF_WireThickness,
	m_eGWireF_Total,
};

enum m_eGWireType
{
	m_eGWireType_Wire,
	m_eGWireType_Foot,
};

enum m_eGWireDirection
{
	m_eGWireDirection_Left,
	m_eGWireDirection_TL,
	m_eGWireDirection_Top,
	m_eGWireDirection_TR,
	m_eGWireDirection_Right,
	m_eGWireDirection_BR,
	m_eGWireDirection_Bottom,
	m_eGWireDirection_BL,
};

typedef struct tagAlgoGWire
{
	int narrdata[m_eGWireN_Total];
	float farrdata[m_eGWireF_Total];

	tagAlgoGWire()
	{
		memset(narrdata, 0, m_eGWireN_Total * sizeof(int));
		memset(farrdata, 0, m_eGWireF_Total * sizeof(float));
	}
}AlgoGWire;

typedef struct tagRstAlgoGWire // GWire 검사 결과
{
	BOOL m_bCntOK;
	int m_nRstCnt;

	int m_nCenterMarginX;
	int m_nCenterMarginY;
	int m_nCenterMarginW;
	int m_nCenterMarginH;

	tagRstAlgoGWire()
	{
		m_bCntOK = FALSE;
		m_nRstCnt = 0;

		m_nCenterMarginX = 0;
		m_nCenterMarginY = 0;
		m_nCenterMarginW = 0;
		m_nCenterMarginH = 0;
	}
}RstAlgoGWire;