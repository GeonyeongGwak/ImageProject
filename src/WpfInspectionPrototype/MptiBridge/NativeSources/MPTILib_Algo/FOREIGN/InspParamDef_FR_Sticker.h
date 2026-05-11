#pragma once
#include <string.h>

enum FR_Sticker_DT
{
	FR_Sticker_DT_TR = 0x01,
	FR_Sticker_DT_TG = 0x02,
	FR_Sticker_DT_TB = 0x04,
	FR_Sticker_DT_MR = 0x08,
	FR_Sticker_DT_MB = 0x10,
	FR_Sticker_DT_BR = 0x20,
	FR_Sticker_DT_BB = 0x40,
};
enum FR_Sticker_N
{
	FR_Sticker_N_Data = 0,
	FR_Sticker_N_MarginX,
	FR_Sticker_N_MarginY,
	FR_Sticker_N_MORPH_RECT,
	FR_Sticker_N_TH_TR,
	FR_Sticker_N_TH_TG,
	FR_Sticker_N_TH_TB,
	FR_Sticker_N_TH_TW,
	FR_Sticker_N_TH_MR,
	FR_Sticker_N_TH_MB,
	FR_Sticker_N_TH_BR,
	FR_Sticker_N_TH_BB,
	FR_Sticker_N_Max_Per,
	FR_Sticker_N_Size_Chk_W,
	FR_Sticker_N_Size_Chk_H,
	FR_Sticker_N_TH_Dir,
	FR_Sticker_N_Total,
};
enum FR_Sticker_L
{
	FR_Sticker_L_TW = 0,
	FR_Sticker_L_TR,
	FR_Sticker_L_TG,
	FR_Sticker_L_TB,
	FR_Sticker_L_MR,
	FR_Sticker_L_MB,
	FR_Sticker_L_BR,
	FR_Sticker_L_BB,
	FR_Sticker_L_Total,
};

typedef struct tagFR_Sticker
{
	int m_nArr[FR_Sticker_N_Total];
	RECT m_rcLabel;

	void Init()
	{
		memset(m_nArr, 0, sizeof(int) * FR_Sticker_N_Total);
	}
	void Clone(tagFR_Sticker &inspAlgo)
	{
		memcpy(inspAlgo.m_nArr, m_nArr, sizeof(int) * FR_Sticker_N_Total);
	}
	bool UseData(int nType)
	{
		bool bRet = ((m_nArr[FR_Sticker_N_Data] & nType) == nType);
		return bRet;
	}
}FR_Sticker;

typedef struct tagStickerData
{
	BOOL m_bSave;
	int m_nFOV;
	int m_nBin;
	int m_nMode;
	int m_nImgW;
	int m_nImgH;
	int m_nPartCnt;
	RECT m_rcArrROI[MAX_STRLEN];
	UCHAR* m_pDesImg;
	wchar_t m_sDir[MAX_STRLEN];

}StickerData;