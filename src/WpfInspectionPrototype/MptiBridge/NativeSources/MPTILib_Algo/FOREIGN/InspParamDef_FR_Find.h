#pragma once
#include <string.h>

#define FR_Find_MAX	1000
#define FR_EXT_RST_MAX 50
enum FR_Find_DT
{
	FR_Find_DT_Width = 0x01,
	FR_Find_DT_Length = 0x02,
	FR_Find_DT_FullMap = 0x04,
	FR_Find_DT_Circle = 0x08,

	FR_Find_DT_Save = 0x10,
};
enum FR_Find_N
{
	FR_Find_N_Data = 0,
	FR_Find_N_Total,
};
enum FR_Find_F
{
	FR_Find_F_Width_Min = 0,
	FR_Find_F_Width_Max,
	FR_Find_F_Length_Min,
	FR_Find_F_Length_Max,
	FR_Find_F_Circle,
	FR_Find_F_Total,
};
enum FR_Find_RGB
{
	FR_Find_RGB_TopB = 0,
	FR_Find_RGB_TopG,
	FR_Find_RGB_TopR,
	FR_Find_RGB_Total,
};

typedef struct tagFR_Find
{
	int m_nArr[FR_Find_N_Total];
	float m_fArr[FR_Find_F_Total];

	tagFR_Bin sBin;
	ColorXYInfoForeign m_ColorXYInfo[FR_Find_RGB_Total];

	bool UseData(int nType)
	{
		bool bRet = ((m_nArr[FR_Find_N_Data] & nType) == nType);
		return bRet;
	}
}FR_Find;