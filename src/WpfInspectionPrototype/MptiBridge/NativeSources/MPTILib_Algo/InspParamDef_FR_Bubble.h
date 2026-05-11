#pragma once
#include <string.h>

enum FR_BUBBLE_DT
{
	FR_BUBBLE_DT_MeasureBubSize = 0x01,
	FR_BUBBLE_DT_BubNGImg = 0x02,
	FR_BUBBLE_DT_BubInspResult = 0x04,
	FR_BUBBLE_DT_LimitBubNGCnt_FOV = 0x08,

	FR_BUBBLE_DT_LimitBubNGCnt_SplitBRDArea = 0x10,
	FR_BUBBLE_DT_VisibleSplitBRDGrid = 0x20,
	FR_BUBBLE_DT_ChkBubMaxSize = 0x40,
	FR_BUBBLE_DT_USE = 0x80,
};
enum FR_BUBBLE_N
{
	FR_BUBBLE_N_Data = 0,
	FR_BUBBLE_N_BubNGLimitCnt_FOV,
	FR_BUBBLE_N_BubbleThreshold,
	FR_BUBBLE_N_BubNGLimitCnt_SplitBRDArea,
	FR_BUBBLE_N_ColorUVSelect,
	FR_BUBBLE_N_ExtraPixelCnt_Bubble,
	FR_BUBBLE_N_Total,
};
enum FR_BUBBLE_F
{
	FR_BUBBLE_F_Area2D_GrayBub = 0,
	FR_BUBBLE_F_BubbleExceptSize,
	FR_BUBBLE_F_BubBoardSplitSize,
	FR_BUBBLE_F_BubMaxSizeSpec,
	FR_BUBBLE_F_BubbleHeightMin,
	FR_BUBBLE_F_BubbleHeightMax,
	FR_BUBBLE_F_BubbleWidthMin,
	FR_BUBBLE_F_BubbleWidthMax,
	FR_BUBBLE_F_Total,
};

typedef struct tagFR_Bubble
{
	int m_nArr[FR_BUBBLE_N_Total];
	float m_fArr[FR_BUBBLE_F_Total];
	void Init()
	{
		memset(m_nArr, 0, sizeof(int) * FR_BUBBLE_N_Total);
		memset(m_fArr, 0, sizeof(float) * FR_BUBBLE_F_Total);
	}
	void Clone(tagFR_Bubble &inspAlgo)
	{
		memcpy(inspAlgo.m_nArr, m_nArr, sizeof(int) * FR_BUBBLE_N_Total);
		memcpy(inspAlgo.m_fArr, m_fArr, sizeof(float) * FR_BUBBLE_F_Total);
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nArr[FR_BUBBLE_N_Data] & nType) == nType);
		return bRet;
	}
}FR_Bubble;