#pragma once
#include <string.h>

enum FR_WP_DT
{
	FR_WP_DT_BTC = 0x01,
	FR_WP_DT_RemoveArea_WP = 0x02,
	FR_WP_DT_IM = 0x04,
	FR_WP_DT_AM = 0x08,

	FR_WP_DT_RemoveArea_IM = 0x10,
	FR_WP_DT_WP_Save3D = 0x20,
	FR_WP_DT_Not_Borad = 0x40,
	FR_WP_DT_Not_Hole = 0x80,

	FR_WP_DT_CorrectWP = 0x100,
	FR_WP_DT_CorrectWPSave = 0x200,
	FR_WP_DT_PseudoZero = 0x400,
	FR_WP_DT_Not_FR = 0x800,

	FR_WP_DT_CorrectWPSimpleSave = 0x1000,
	FR_WP_DT_CorrectWPAbsolute = 0x2000,
	FR_WP_DT_CorrectWPAuto = 0x4000,
	FR_WP_DT_Expression_Type_RemovePart = 0x8000,

	FR_WP_DT_Array_Type_2D = 0x10000,
	FR_WP_DT_CorrectWP_B = 0x20000,
};
enum FR_WP_N
{
	FR_WP_N_Data = 0,
	FR_WP_N_WPQuality,
	FR_WP_N_WPQualityPtt,
	FR_WP_N_Scale,
	FR_WP_N_ExceptArea,
	FR_WP_N_ExceptAreaIM,
	FR_WP_N_Total,
};
enum FR_WP_F
{
	FR_WP_F_AMIM_H = 0, // 사용하지 않음
	FR_WP_F_PseudoColorMin,
	FR_WP_F_PseudoColorMax,
	FR_WP_F_HeightMaxMin_WP,
	FR_WP_F_HeightMaxMin_WPM,
	FR_WP_F_HeightMaxMin_WPAM,
	FR_WP_F_Margin_T,
	FR_WP_F_Margin_B,
	FR_WP_F_Total,
};

typedef struct tagFR_WP
{
	int m_nArr[FR_WP_N_Total];
	float m_fArr[FR_WP_F_Total];
	void Init()
	{
		memset(m_nArr, 0, sizeof(int) * FR_WP_N_Total);
		memset(m_fArr, 0, sizeof(float) * FR_WP_F_Total);
	}
	void Clone(tagFR_WP &inspAlgo)
	{
		memcpy(inspAlgo.m_nArr, m_nArr, sizeof(int) * FR_WP_N_Total);
		memcpy(inspAlgo.m_fArr, m_fArr, sizeof(float) * FR_WP_F_Total);
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nArr[FR_WP_N_Data] & nType) == nType);
		return bRet;
	}
	int GetRemove(int nW, int nH, int nDivide, bool bAMIM = false)
	{
		int nExt = bAMIM ? m_nArr[FR_WP_N_ExceptAreaIM] : m_nArr[FR_WP_N_ExceptArea];
		int nExceptArea = nExt / nDivide;
		if (nExceptArea < 0) nExceptArea = 0;
		if (nExceptArea > ((nW / 2) - 2)) nExceptArea = (nW / 2) - 2;
		if (nExceptArea > ((nH / 2) - 2)) nExceptArea = (nH / 2) - 2;

		return nExceptArea;
	}
}FR_WP;