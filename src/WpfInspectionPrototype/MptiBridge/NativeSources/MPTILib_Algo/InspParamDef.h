#pragma once
#include "InspRstPoly.h"
#include "FOREIGN/InspParamDef_FR.h"
#include "PInsp_Algo/InspParamDef_Algo.h"
#include "opencv2/opencv.hpp"
#include "MPTIDef.h"
#include <list>
#include <map>
#include "ExtProductionInfo.h"
#include "ipps.h"
#include "ippi.h"
//////////////////////////////////////////////////////////////////////////
//define
//////////////////////////////////////////////////////////////////////////

#define MAX_POCR_DIVISITION_CNTS	16

#define INSP_MODE   FALSE     //TRUE: ng pass mode   FALSE: ng none pass

//max inspection window counts
#define MAX_WINDOW_CNTS		10500
#define MAX_GROUP_CNTS		10500

//color search area

//board origin location
#define  DOWN_LEFT    0
#define  DOWN_RIGHT   1
#define  UP_LEFT      2
#define  UP_RIGHT     3

//LJH 2016.06.09
#define TYPE_IMAGE_MULTI_INSP	0		//선택한 영상을 모두 검사하는 방식
#define TYPE_IMAGE_MIX_INSP		1		//선택한 영상을 합쳐서 검사하는 방식
#define TYPE_IMAGE_ALL_INSP		2		//선택한 영상을 모두 검사한 후 불량일 경우 합쳐서 검사하는 방식

//LJH 2016.06.15 Filter Type
#define FONT_THICK_FILTER		2
#define FONT_THIN_FILTER		1

#define MAX_MASKING_NUM 4
#define MAX_INSP_AREA_COUNT 20
#define nFilletDivisionCnt	2

#define PAT_INSP_MIN_SIZE 9
#define PAT_INSP_MIN_SIZE2 5
enum insp_type
{
	eINSP_MOUNT = 0,
	eINSP_ALIGN,	// ++
	eINSP_OCR,
	eINSP_LEADSOLDER,
	eINSP_SOLDER,		// ++
	eINSP_TAB,
	eINSP_S_BALL,
	eINSP_PAD,
	eINSP_USER,
	eINSP_BGA,
	eINSP_WND_CNT,
	eINSP_COLOR,		// X
	eINSP_PATTERN, /// X
	eINSP_Total,
};

enum NG_Type
{
	eOK = 0,
	eNgMount = 200,
	eNgAlign = 300,
	eNgOCR = 400,
	eNgLead = 500,
	eNgSolder = 600,
	eNGForeign = 700,
	eNgTab = 800,
	eNgBGA = 900,
	eNGS_Ball = 1000,
	eNGPAD = 1100,
	eNGTHI = 1200,
};

enum inspresult_ID
{
	e_NG = 0,
	e_OK,
	e_SKIP
};
enum defectCode
{
	dftCODE_OK = 100,

	dftCODE_MISSING = 200,
	dftCODE_WRONG = 201,
	dftCODE_ANGLE = 202,
	dftCODE_POLARITY = 203,
	dftCODE_SHIFT = 204,
	dftCODE_TILT = 205,
	dftCODE_LIFT = 206,
	dftCODE_UPSIDEDOWN = 207,
	dftCODE_TOMBSTONE = 208,
	dftCODE_BILLBOARDING = 209,
	dftCODE_LENGTH = 210,
	dftCODE_DAMAGE = 211,
	dftCODE_BODYWIDTH = 212,
	dftCODE_BODYLENGTH = 213,
	dftCODE_BODYAREA = 214,
	dftCODE_COLOR = 215,

	dftCODE_OCR = 250,

	//dftCODE_L_NUM = 300,
	//dftCODE_L_PITCH = 301,
	dftCODE_L_BRIDGE = 302,
	dftCODE_L_LIFT = 303,
	dftCODE_L_LEAD = 304,
	dftCODE_L_SHIFT = 305,

	dftCODE_S_FILLET = 400,
	dftCODE_S_NO = 401,
	dftCODE_S_EXCESS = 402,
	dftCODE_S_CONE = 403,

	defCODE_TAB = 440,
	defCODE_TAB_SPACE = 441,

	defCODE_WARPAGE = 447,
	defCODE_COPPER = 448,
	defCODE_FOREIGN = 449,
	defCODE_USER_DEFINE = 450,
	dftCODE_FOREIGN_A = 451,
	dftCODE_FOREIGN_B = 452,
	dftCODE_FOREIGN_C = 453,

	dftCODE_BGA_NG = 500,

	dftCODE_S_BALL = 600,

	dftCODE_COLD_JOINT = 700,

	dftCODE_PIN_SHIFT = 800,
	dftCODE_VOID = 801,
	dftCODE_THICKNESS = 900,

	dftCODE_LOW_COATING = 1000,
	dftCODE_HIGH_COATING = 1001,
	dftCODE_CI_BUBBLE = 1002,		//NYJ 2018/07/18
	dftCODE_DISCOLOR = 1100,
};


//////////////////////////////////////////////////////////////////////////

enum color
{
	eCOLOR_BLACK	= 0,				      
	eCOLOR_RED,					
	eCOLOR_GREEN,					
	eCOLOR_BLUE,					
	eCOLOR_YELLOW,				
	eCOLOR_MAGENTA,				                          
	eCOLOR_CYAN,				                           
	eCOLOR_WHITE,					                             
	eCOLOR_GRAY,				                                
	eCOLOR_BRIGHT_GRAY,						                   
	eCOLOR_LIGHT_GRAY,			                        
	eCOLOR_LIGHT_GREEN,			                        
	eCOLOR_LIGHT_BLUE,			                       
	eCOLOR_LIGHT_WHITE,			                        
	eCOLOR_DARK_RED,				                             
	eCOLOR_DARK_GREEN,		                          
	eCOLOR_DARK_BLUE,				                        
	eCOLOR_DARK_YELLOW,			                     
	eCOLOR_DARK_MAGENTA,		                     
	eCOLOR_DARK_CYAN,	
	eCOLOR_ORANGE
};

//////////////////////////////////////////////////////////////////////////
// Foreign
enum m_eLogLv
{
	m_eLogLv_None = 0,
	m_eLogLv_Lv1,
	m_eLogLv_Lv2,
	m_eLogLv_Lv3,
	m_eLogLv_Lv4,
	m_eLogLv_Fiducial,
	m_eLogLv_Teaching,
	m_eLogLv_FOV,
	m_eLogLv_TactTime,
	m_eLogLv_Foreign,
	m_eLogLv_Partition,
	m_eLogLv_AI_Module,
	m_eLogLv_OffEdit,
	m_eLogLv_Cali,
	m_eLogLv_TrendLine,
	m_eLogLv_ExBarcode,
	m_eLogLv_Blob,
	m_eLogLv_OCR,
	m_eLogLv_Sticker,
	m_eLogLv_Check3D,
	m_eLogLv_PolygonGerber,
	m_eLogLv_MultiProcess,
	m_eLogLv_Defect,
	m_eLogLv_Repair,
	m_eLogLv_DirConnMac,
	m_eLogLv_RVS,
	m_eLogLv_JobChange,
	m_eLogLv_BigPart3D,
	m_eLogLv_RunDraw,
	m_eLogLv_PAT,
	m_eLogLv_PAT_DEBUG,
	m_eLogLv_Check3D_2,
	m_eLogLv_FOV_3D,
	m_eLogLv_MES,
	m_eLogLv_Crack,
	m_eLogLv_Clamp,
	m_eLogLv_ZPOS,
	m_eLogLv_ThreadSeq,
	m_eLogLv_ALL,
};
enum m_eSaveImgParamData
{
	m_eSaveImgParamData_SaveUserLight = 0x01,
};
enum m_ePartPropType
{
	PartPropType_General = 0,
	PartPropType_BGA,
	PartPropType_Chip,
	PartPropType_PartFidu,
	PartPropType_Dummy,          // 검사 무시
	PartPropType_Barcode,
	PartPropType_BadMark,
	PartPropType_Foreign, // 이물파트
	PartPropType_Vendor,
	PartPropType_Bottom,    // 바닥 평탄도 검사용
	PartPropType_BtmSide, // 하부 카메라
	PartPropType_BtmSide2, // 하부 카메라2
	PartPropType_BtmSide3, // 하부 카메라3
	PartPropType_BtmSide4, // 하부 카메라4
	PartPropType_ViaHole,    // viaHole 파트
	PartPropType_PadBW,        // Pad 파트
	PartPropType_AreaBadMark,        //영역 배드마크 NYJ 2022/01/10
	PartPropType_Label,        // 하나마이크론 SSD Label 검사 , OCR , Pattern , 바코드 : 3D 검사 불필요 알고리즘은 3개 문자관련 추가될수 있음 // LYS 2023/01/05
};
enum InspPartInfo_DATA
{
	InspPartInfo_DATA_RE_3D = 0x01,
};
enum m_ePAT_POLY_D
{
	m_ePAT_POLY_D_Erod = 0x01,
};
enum m_eINSP_PAD_PAT
{
	m_eINSP_PAD_PAT_Calc3D = 0x01,
};
enum m_eAIModelName
{
	m_eAIModelName_Contamination,
	m_eAIModelName_Segmentation,
	m_eAIModelName_test1,
	m_eAIModelName_test2,
	Total_,
};

enum m_ePART_GEN
{
	m_ePART_GEN_UP = 0,
	m_ePART_GEN_DN,
};
enum m_eINSP_ALGO_DATA
{
	m_eINSP_ALGO_DATA_Gen2D_UP = 0x01,
	m_eINSP_ALGO_DATA_Gen2D_DN = 0x02,
};
//////////////////////////////////////////////////////////////////////////
//
// Main inspection parameter
//
//////////////////////////////////////////////////////////////////////////
const int WND_ARRAY_CNTS = 1000;
#pragma region _BGA PARAM_
typedef struct tagBGAdecisionList_Wnd
{
	BOOL offsetXOk;
	BOOL offsetYOk;
	BOOL diameterOk;
	BOOL heightOk;

}BGAdecisionList_Wnd;

typedef struct tagBGAdecisionList_Grp
{
	BOOL toleranceOk;

}BGAdecisionList_Grp;

typedef struct tagRstInspBGA_Wnd
{
	//common
	BOOL isInsp;
	BOOL ok;	// 1: GOOD ,  0: NG

	double cogX;
	double cogY;
	double cogX_pixel;
	double cogY_pixel;
	double dia;
	double height_Abs;
	double height_Rel;
	double offsetX;
	double offsetY;
	double width;
	double height;

	double coplanarity;

	//int wndDefectCode;
	BGAdecisionList_Wnd list;
}RstInspBGA_Wnd;

typedef struct tagRstInspBGA_Grp
{
	//common
	BOOL ok;	// 1: GOOD ,  0: NG

	double maxCalcHeight;
	double minCalcHeight;
	double tolerance;

	//int wndDefectCode;
	BGAdecisionList_Grp list;
}RstInspBGA_Grp;


typedef struct tagTeachParamBGA
{
	//[input]
	double wndAngle;
	int lowerLimit_ball;	//% 상위 lowerLimit_ball% 안에드는 높이 들의 평균
	int lowerLimit_board;	//%	상위 lowerLimit_board% 안에드는 높이 들의 평균


	//[output]
	double cogX;
	double cogY;
	double dia;
	double height_Abs;
	double height_Rel;


}TeachParamBGA;


typedef struct tagInspParamBGA
{
	//common
	//[input]
	double wndAngle;
	int lowerLimit_ball;	//% 상위 lowerLimit_ball% 안에드는 높이 들의 평균
	int lowerLimit_board;	//%	상위 lowerLimit_board% 안에드는 높이 들의 평균

	//기준 데이터
	double stdCogX;
	double stdCogY;
	double stdDia;
	double stdH_Abs;
	double stdH_Rel;

	//허용 범위
	double range_offset;
	double range_MaxDia;
	double range_MinDia;
	double range_MaxHeight;
	double range_MinHeight;

	//[output]
	RstInspBGA_Wnd retInspResult;
}InspParamBGA;


typedef struct tagInspParamBGA_Part
{
	//common
	//[input]
	double stdTolerance;
	void CopyData(tagInspParamBGA_Part sData)
	{
		stdTolerance = sData.stdTolerance;
	}
	void DeleteData()
	{
	}
}InspParamBGA_Part;


#pragma endregion _BGA PARAM_



#pragma region _ZMAP DATA_
typedef struct tagZmapData
{
	float* data;  
	int zmapSizeX;  
	int zmapSizeY; 
	float fResultPartOffset;	// Center Warpage 결과
	bool bGroundLevel;
	void CopyData(tagZmapData sData , bool bIPP = false)
	{
		zmapSizeX = sData.zmapSizeX;
		zmapSizeY = sData.zmapSizeY;

		data = g_pMManager->pem_new<float>(true, zmapSizeX * zmapSizeY, (PCHAR)__FUNCTION__, __LINE__);
		if (bIPP)
		{
			IppStatus sts;
			IppiSize iSize = { zmapSizeX,zmapSizeY };
		if (sData.data == NULL)
			memset(data, 0, sizeof(float) * zmapSizeX * zmapSizeY);
			else
				sts = ippiCopy_32f_C1R((float*)sData.data, zmapSizeX * sizeof(float), (float*)data, zmapSizeY * sizeof(float), iSize);
		}
		else
		{
			if (sData.data == NULL)
				memset(data, 0, sizeof(float) * zmapSizeX * zmapSizeY);
			else
			memcpy(data, sData.data, sizeof(float) * zmapSizeX * zmapSizeY);
		}
	}
	void DeleteData()
	{
		if (data)
		{
			g_pMManager->pem_delete(data, true);
			data = NULL;
		}
	}
}ZmapData;
#pragma endregion _ZMAP DATA_




#pragma region _Image Buffer_

enum InspImg_Kind
{
	eImg_Bottom_R = 0,
	eImg_Bottom_B,

	eImg_Middle_R,
	eImg_Middle_B,

	eImg_Top_R,
	eImg_Top_G,
	eImg_Top_B,
	eImg_Top_W,

	eImg_User_R,
	eImg_User_G,
	eImg_User_B,
	eImg_User_W,

	eImg_BufCnt
};

typedef struct tagInspImgBuf
{
	void* imgTop_R;
	void* imgTop_G;
	void* imgTop_B;
	void* imgTop_W;

	void* imgMiddle_R;
	void* imgMiddle_B;

	void* imgBottom_R;
	void* imgBottom_B;

	void* imgSide1_R;
	void* imgSide1_G;
	void* imgSide1_B;

	void* imgSide2_R;
	void* imgSide2_G;
	void* imgSide2_B;

	void* imgSide3_R;
	void* imgSide3_G;
	void* imgSide3_B;

	void* imgSide4_R;
	void* imgSide4_G;
	void* imgSide4_B;

	void*& GetPtrImg(int nChannel2D)
	{
		switch(nChannel2D)
		{
		case eM2C_BB:
			return imgBottom_B;
		case eM2C_BR:
			return imgBottom_R;
		case eM2C_MB:
			return imgMiddle_B;
		case eM2C_MR:
			return imgMiddle_R;
		case eM2C_TB:
			return imgTop_B;
		case eM2C_TR:
			return imgTop_R;
		case eM2C_TG:
			return imgTop_G;
		case eM2C_TW:
			return imgTop_W;
		}
	}

	void CopyData(tagInspImgBuf sData)
	{
	}
	void DeleteData()
	{
	}
}InspImgBuf;

typedef struct tagRoiImgBuf
{
	int nImageSizeX;
	int nImageSizeY;

	UCHAR* imgTop_R;
	UCHAR* imgTop_G;
	UCHAR* imgTop_B;
	UCHAR* imgTop_W;

	UCHAR* imgMiddle_R;
	UCHAR* imgMiddle_B;

	UCHAR* imgBottom_R;
	UCHAR* imgBottom_B;

	UCHAR* imgSide1_R;
	UCHAR* imgSide1_G;
	UCHAR* imgSide1_B;

	UCHAR* imgSide2_R;
	UCHAR* imgSide2_G;
	UCHAR* imgSide2_B;

	UCHAR* imgSide3_R;
	UCHAR* imgSide3_G;
	UCHAR* imgSide3_B;

	UCHAR* imgSide4_R;
	UCHAR* imgSide4_G;
	UCHAR* imgSide4_B;

	UCHAR* imgAI;
	tagRoiImgBuf()
	{
		imgTop_R = NULL;
		imgTop_G = NULL;
		imgTop_B = NULL;
		imgTop_W = NULL;

		imgMiddle_R = NULL;
		imgMiddle_B = NULL;

		imgBottom_R = NULL;
		imgBottom_B = NULL;

		imgSide1_R = NULL;
		imgSide1_G = NULL;
		imgSide1_B = NULL;

		imgSide2_R = NULL;
		imgSide2_G = NULL;
		imgSide2_B = NULL;

		imgSide3_R = NULL;
		imgSide3_G = NULL;
		imgSide3_B = NULL;

		imgSide4_R = NULL;
		imgSide4_G = NULL;
		imgSide4_B = NULL;

		imgAI = NULL;
	}

	UCHAR*& GetPtrImg(int nNum)
	{
		switch (nNum)
		{
		case eInspRoiImg_imgTop_R:
			return imgTop_R;
		case eInspRoiImg_imgTop_G:
			return imgTop_G;
		case eInspRoiImg_imgTop_B:
			return imgTop_B;
		case eInspRoiImg_imgTop_W:
			return imgTop_W;
		case eInspRoiImg_imgMiddle_R:
			return imgMiddle_R;
		case eInspRoiImg_imgMiddle_B:
			return imgMiddle_B;
		case eInspRoiImg_imgBottom_R:
			return imgBottom_R;
		case eInspRoiImg_imgBottom_B:
			return imgBottom_B;
		case eInspRoiImg_imgSide1_R:
			return imgSide1_R;
		case eInspRoiImg_imgSide1_G:
			return imgSide1_G;
		case eInspRoiImg_imgSide1_B:
			return imgSide1_B;
		case eInspRoiImg_imgSide2_R:
			return imgSide2_R;
		case eInspRoiImg_imgSide2_G:
			return imgSide2_G;
		case eInspRoiImg_imgSide2_B:
			return imgSide2_B;
		case eInspRoiImg_imgSide3_R:
			return imgSide3_R;
		case eInspRoiImg_imgSide3_G:
			return imgSide3_G;
		case eInspRoiImg_imgSide3_B:
			return imgSide3_B;
		case eInspRoiImg_imgSide4_R:
			return imgSide4_R;
		case eInspRoiImg_imgSide4_G:
			return imgSide4_G;
		case eInspRoiImg_imgSide4_B:
			return imgSide4_B;
		case eInspRoiImg_imgAI:
			return imgAI;
		}
	}

	void CopyData(tagRoiImgBuf sData, bool bIPP = false)
	{
		nImageSizeX = sData.nImageSizeX;
		nImageSizeY = sData.nImageSizeY;
		for (int a = 0; a < eInspRoiImg_NUM; a++)
		{
			UCHAR*& pSrc = sData.GetPtrImg(a);
			UCHAR*& pRst = GetPtrImg(a);
			if (nImageSizeX <= 0 || nImageSizeY <= 0)
			{
				pRst = NULL;
				continue;
			}
			pRst = g_pMManager->pem_new<UCHAR>(true, nImageSizeX * nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
			if (bIPP)
			{
				IppStatus sts;
				IppiSize iSize = { nImageSizeX,nImageSizeY };
			if (pSrc == NULL)
				memset(pRst, 0, sizeof(UCHAR) * nImageSizeX * nImageSizeY);
				else
					sts = ippiCopy_8u_C1R(pSrc, nImageSizeX, pRst, nImageSizeX, iSize);
			}
			else
			{
				if (pSrc == NULL)
					memset(pRst, 0, sizeof(UCHAR) * nImageSizeX * nImageSizeY);
				else
				memcpy(pRst, pSrc, sizeof(UCHAR) * nImageSizeX * nImageSizeY);
			}
		}
	}
	void DeleteData()
	{
		for (int a = 0; a < eInspRoiImg_NUM; a++)
		{
			UCHAR*& pRst = GetPtrImg(a);
			if (pRst)
			{
				g_pMManager->pem_delete(pRst, true);
				pRst = NULL;
			}
		}
	}
}InspRoiImgBuf;

typedef struct tagRoiColorBuf
{
	UCHAR *img_R;
	UCHAR *img_G;
	UCHAR *img_B;

	UCHAR *img_R_BtmSide;
	UCHAR *img_G_BtmSide;
	UCHAR *img_B_BtmSide;

	tagRoiColorBuf()
	{
		img_R = nullptr;
		img_G = nullptr;
		img_B = nullptr;

		img_R_BtmSide = nullptr;
		img_G_BtmSide = nullptr;
		img_B_BtmSide = nullptr;
	}

	UCHAR*& GetPtrImg(int nNum)
	{
		switch (nNum)
		{
		case eRoiColorBuf_img_R:
			return img_R;
		case eRoiColorBuf_img_G:
			return img_G;
		case eRoiColorBuf_img_B:
			return img_B;
		case eRoiColorBuf_img_R_BtmSide:
			return img_R_BtmSide;
		case eRoiColorBuf_img_G_BtmSide:
			return img_G_BtmSide;
		case eRoiColorBuf_img_B_BtmSide:
			return img_B_BtmSide;
		}
	}
	void CopyData(tagRoiColorBuf sData, int nImageSizeX, int nImageSizeY)
	{
		for (int a = 0; a < eRoiColorBuf_NUM; a++)
		{
			UCHAR*& pSrc = sData.GetPtrImg(a);
			UCHAR*& pRst = GetPtrImg(a);
			if (nImageSizeX <= 0 || nImageSizeY <= 0)
			{
				pRst = NULL;
				continue;
			}
			pRst = g_pMManager->pem_new<UCHAR>(true, nImageSizeX * nImageSizeY, (PCHAR)__FUNCTION__, __LINE__);
			if (pSrc == NULL)
				memset(pRst, 0, sizeof(UCHAR) * nImageSizeX * nImageSizeY);
			else
				memcpy(pRst, pSrc, sizeof(UCHAR) * nImageSizeX * nImageSizeY);
		}
	}
	void DeleteData()
	{
		for (int a = 0; a < eRoiColorBuf_NUM; a++)
		{
			UCHAR*& pRst = GetPtrImg(a);
			if (pRst)
			{
				g_pMManager->pem_delete(pRst, true);
				pRst = NULL;
			}
		}
	}

}RoiColorBuf;

typedef struct tagPartImgBuf
{
	std::vector<UCHAR> imgTop_R;
	std::vector<UCHAR> imgTop_G;
	std::vector<UCHAR> imgTop_B;
	std::vector<UCHAR> imgTop_W;

	std::vector<UCHAR> imgMiddle_R;
	std::vector<UCHAR> imgMiddle_B;

	std::vector<UCHAR> imgBottom_R;
	std::vector<UCHAR> imgBottom_B;

	std::vector<UCHAR> imgSide1_R;
	std::vector<UCHAR> imgSide1_G;
	std::vector<UCHAR> imgSide1_B;

	std::vector<UCHAR> imgSide2_R;
	std::vector<UCHAR> imgSide2_G;
	std::vector<UCHAR> imgSide2_B;

	std::vector<UCHAR> imgSide3_R;
	std::vector<UCHAR> imgSide3_G;
	std::vector<UCHAR> imgSide3_B;

	std::vector<UCHAR> imgSide4_R;
	std::vector<UCHAR> imgSide4_G;
	std::vector<UCHAR> imgSide4_B;

	std::vector<float> zmapData;

	std::vector<UCHAR> imgColor_R;
	std::vector<UCHAR> imgColor_G;
	std::vector<UCHAR> imgColor_B;

	~tagPartImgBuf()
	{
		imgTop_R.clear();
		imgTop_G.clear();
		imgTop_B.clear();
		imgTop_W.clear();

		imgMiddle_R.clear();
		imgMiddle_B.clear();

		imgBottom_R.clear();
		imgBottom_B.clear();

		imgSide1_R.clear();
		imgSide1_G.clear();
		imgSide1_B.clear();

		imgSide2_R.clear();
		imgSide2_G.clear();
		imgSide2_B.clear();

		imgSide3_R.clear();
		imgSide3_G.clear();
		imgSide3_B.clear();

		imgSide4_R.clear();
		imgSide4_G.clear();
		imgSide4_B.clear();

		zmapData.clear();

		imgColor_R.clear();
		imgColor_G.clear();
		imgColor_B.clear();
	}

	void resize_TMBZ(int nSize)
	{
		zmapData.resize(nSize);
		imgTop_R.resize(nSize);
		imgTop_G.resize(nSize);
		imgTop_B.resize(nSize);
		imgTop_W.resize(nSize);
		imgMiddle_R.resize(nSize);
		imgMiddle_B.resize(nSize);
		imgBottom_R.resize(nSize);
		imgBottom_B.resize(nSize);
	}
	void data_TMBZ(InspRoiImgBuf * partImgBuf, float ** zmap)
	{
		partImgBuf->imgTop_R = imgTop_R.data();
		partImgBuf->imgTop_G = imgTop_G.data();
		partImgBuf->imgTop_B = imgTop_B.data();
		partImgBuf->imgTop_W = imgTop_W.data();
		partImgBuf->imgMiddle_R = imgMiddle_R.data();
		partImgBuf->imgMiddle_B = imgMiddle_B.data();
		partImgBuf->imgBottom_R = imgBottom_R.data();
		partImgBuf->imgBottom_B = imgBottom_B.data();
		*zmap = zmapData.data();
	}
	void resize_Color(int nSize)
	{
		imgColor_R.resize(nSize);
		imgColor_G.resize(nSize);
		imgColor_B.resize(nSize);
	}
	void data_Color(RoiColorBuf * partImgColorBuf)
	{
		partImgColorBuf->img_R = imgColor_R.data();
		partImgColorBuf->img_G = imgColor_G.data();
		partImgColorBuf->img_B = imgColor_B.data();
	}
	void resize_Side1(int nSize, InspRoiImgBuf * partImgBuf)
	{
		imgSide1_R.resize(nSize);
		imgSide1_G.resize(nSize);
		imgSide1_B.resize(nSize);
		partImgBuf->imgSide1_R = imgSide1_R.data();
		partImgBuf->imgSide1_G = imgSide1_G.data();
		partImgBuf->imgSide1_B = imgSide1_B.data();
	}
	void resize_Side2(int nSize, InspRoiImgBuf * partImgBuf)
	{
		imgSide2_R.resize(nSize);
		imgSide2_G.resize(nSize);
		imgSide2_B.resize(nSize);
		partImgBuf->imgSide2_R = imgSide2_R.data();
		partImgBuf->imgSide2_G = imgSide2_G.data();
		partImgBuf->imgSide2_B = imgSide2_B.data();
	}
	void resize_Side3(int nSize, InspRoiImgBuf * partImgBuf)
	{
		imgSide3_R.resize(nSize);
		imgSide3_G.resize(nSize);
		imgSide3_B.resize(nSize);
		partImgBuf->imgSide3_R = imgSide3_R.data();
		partImgBuf->imgSide3_G = imgSide3_G.data();
		partImgBuf->imgSide3_B = imgSide3_B.data();
	}
	void resize_Side4(int nSize, InspRoiImgBuf * partImgBuf)
	{
		imgSide4_R.resize(nSize);
		imgSide4_G.resize(nSize);
		imgSide4_B.resize(nSize);
		partImgBuf->imgSide4_R = imgSide4_R.data();
		partImgBuf->imgSide4_G = imgSide4_G.data();
		partImgBuf->imgSide4_B = imgSide4_B.data();
	}
}PartImgBuf;

typedef struct tagRoiByteBuf
{
	BYTE * pBuffer;
	UINT szSize;
	tagRoiByteBuf()
	{
		pBuffer = nullptr;
		szSize = 0;
	}
	void CopyData(tagRoiByteBuf sData)
	{
		szSize = sData.szSize;
		if (szSize > 0)
		{
			pBuffer = g_pMManager->pem_new<BYTE>(true, szSize, (PCHAR)__FUNCTION__, __LINE__);
			if (sData.pBuffer)
				memcpy(pBuffer, sData.pBuffer, sizeof(BYTE) * szSize);
			else
				memset(pBuffer, 0, sizeof(BYTE) * szSize);
		}
	}
	void DeleteData()
	{
		if (pBuffer)
		{
			g_pMManager->pem_delete(pBuffer, true);
			pBuffer = NULL;
		}
	}
}RoiByteBuf;

typedef struct tagSideCamInfo
{
	int sideOriginalSize;
	int camWidth;
	int camHeight;

	tagSideCamInfo()
	{
		sideOriginalSize = 0;
		camWidth = 2592;
		camHeight = 2048;
	}

}SideCamInfo;

#pragma endregion _Image Buffer_

#pragma region _Algorithm_

typedef struct tagLeadInfo
{	
	int leadCounts;
	int gapCounts;
	double leadPitch;
	double leadWidth;
	double maxLeadWidth;
	double minLeadWidth;
	double maxGap;
	double minGap;
	double gapWidth;
	POINT m_poStartLead;
	POINT m_poEndLead;
	int m_nStartLeadLeft;
	BOOL manualType;

	RECT m_rcArrGapRect[BRIEDGE_CNT];
	CRect m_rcArrLead[1000];
}LeadInfo;

typedef struct tagWndInfo
{
	double dCenterX;
	double dCenterY;
	double dWidth;
	double dLength;
	double dAngle;

	double m_dPartWidth;
	double m_dPartHeight;
	double m_dAlignAngle;

	int m_nWndX;
	int m_nWndY;
	tagWndInfo()
	{
		dCenterX = 0;
		dCenterY = 0;
		dWidth = 0;
		dLength = 0;
		dAngle = 0;

		m_dPartWidth = 0.0;
		m_dPartHeight = 0.0;
		m_dAlignAngle = 0.0;

		m_nWndX = 0;
		m_nWndY = 0;
	}

}WndInfo;

typedef struct tagAlgoCoordinate
{
	double dROICenterX;
	double dROICenterY;
	double dROIWidth;
	double dROILength;
	double dROIAngle;

	double m_dFovCenterX;
	double m_dFovCenterY;

	double dWndX;
	double dWndY;
	double dPartW;
	double dPartH;
	tagAlgoCoordinate()
	{
		dROICenterX = 0.;
		dROICenterY = 0.;
		dROIWidth = 0.;
		dROILength = 0.;

		m_dFovCenterX = 0.;
		m_dFovCenterY = 0.;
		dWndX = 0;
		dWndY = 0;
		dPartW = 0;
		dPartH = 0;
	}

	void GetParam(WndInfo &wndInfo) const
	{
		wndInfo.dCenterX = dROICenterX;
		wndInfo.dCenterY = dROICenterY;
		wndInfo.dWidth = dROIWidth;
		wndInfo.dLength = dROILength;
		wndInfo.dAngle = dROIAngle;
	}

	void SetParam(const WndInfo &wndInfo)
	{
		dROICenterX = wndInfo.dCenterX;
		dROICenterY = wndInfo.dCenterY;
		dROIWidth = wndInfo.dWidth;
		dROILength = wndInfo.dLength;
		dROIAngle = wndInfo.dAngle;
	}

	void ToImage(double dResolX, double dResolY)
	{
		dROIWidth = int(dROIWidth / dResolX);
		dROILength = int(dROILength / dResolY);
		dROICenterX = int(dROICenterX / dResolX);
		dROICenterY = int(dROICenterY / dResolY);

		if ((dROICenterX <= 0) || (dROICenterY <= 0))
		{
			dROICenterX = dROIWidth / 2;
			dROICenterY = dROILength / 2;
		}
	}

}AlgoCoordinate;

enum EIPC_Class
{
	Class1,
	Class2,
	Class3,
};
typedef struct tagEdgePartImage
{
	float nImageSizeX;
	float nImageSizeY;

	UCHAR * img[16];
}EdgePartImage;

typedef struct tagWndAlgoImg
{
	BOOL m_bIs2dCV;	// TRUE : m_ucArr2D => OpenCV raw buffer
	int m_nWidth;
	int m_nHeight;
	int m_nWidth3D;
	int m_nHeight3D;
	UCHAR * m_ucArr2D;
	UCHAR * m_ucArr2D_Mix[2];	//LJH 2016.05.28 합칠 2D 영상 버퍼 추가(최대 2개)
	float * m_fArr3D;			// 2D Image 와 맞춰서 자른 3D Data (검사 시 사용)
	double dAngle;
	UCHAR * m_ucArrCV;
	float * m_fArr3D_part;		// C#에서 넘겨주는  Part 3D data
	int m_nChannel;
	int m_nLight_index;
	int m_Use_algo_model;
	int m_Use_model_nchannel;

	//rounding error correction value that occurs when calculating coordinates ( unit : pixel) NYJ 2020/12/16
	float m_fPartRoundingErrX;
	float m_fPartRoundingErrY;
	float m_fWndRoundingErrX;
	float m_fWndRoundingErrY;

	tagWndAlgoImg()
	{
		m_bIs2dCV = FALSE;
		m_nWidth = 0;
		m_nHeight = 0;
		m_ucArr2D = NULL;
		for (int iLoopCount = 0; iLoopCount < 2; ++iLoopCount)	m_ucArr2D_Mix[iLoopCount] = NULL;	//LJH 2016.05.28 
		m_fArr3D = NULL;
		m_fArr3D_part = NULL;
		dAngle = 0;
		m_ucArrCV = NULL;
		m_nWidth3D = 0;
		m_nHeight3D = 0;
		m_nChannel = 1;
		m_nLight_index = 0;
		m_Use_algo_model = 1;
		m_Use_model_nchannel = 1;

		m_fPartRoundingErrX = 0.0f;
		m_fPartRoundingErrY = 0.0f;
		m_fWndRoundingErrX = 0.0f;
		m_fWndRoundingErrY = 0.0f;
	}
	~tagWndAlgoImg()
	{
		Destroy();
	}

	void Destroy()
	{
		if (m_ucArr2D)
		{
			//delete [] m_ucArr2D;
			g_pMManager->pem_delete(m_ucArr2D, true);
			m_ucArr2D = NULL;
		}

		if (m_ucArrCV)
		{
			//delete [] m_ucArrCV;
			g_pMManager->pem_delete(m_ucArrCV, true);
			m_ucArrCV = NULL;
		}

		//LJH 2016.05.28
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		{
			if (m_ucArr2D_Mix[iLoopCount])
			{
				//delete m_ucArr2D_Mix[iLoopCount];
				g_pMManager->pem_delete(m_ucArr2D_Mix[iLoopCount], true);
				m_ucArr2D_Mix[iLoopCount] = NULL;
			}
		}

		if (m_fArr3D)
		{
			//delete [] m_fArr3D;
			g_pMManager->pem_delete(m_fArr3D, true);
			m_fArr3D = NULL;
		}
	}

	struct tagWndAlgoImg& operator =(const struct tagWndAlgoImg& wndAlgoImg)
	{
		int nSizeImg = wndAlgoImg.m_nWidth * wndAlgoImg.m_nHeight;
		if (m_ucArr2D)
		{
			Delete_1DArray(&m_ucArr2D);
			m_ucArr2D = NULL;
		}

		//LJH 2016.05.28
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		{
			Delete_1DArray(&m_ucArr2D_Mix[iLoopCount]);
			m_ucArr2D_Mix[iLoopCount] = NULL;
		}

		if (m_fArr3D)
		{
			Delete_1DArray(&m_fArr3D);
			m_fArr3D = NULL;
		}

		m_bIs2dCV = wndAlgoImg.m_bIs2dCV;	// TRUE : m_ucArr2D => OpenCV raw buffer
		m_nWidth = wndAlgoImg.m_nWidth;
		m_nHeight = wndAlgoImg.m_nHeight;
		m_nWidth3D = wndAlgoImg.m_nWidth3D;
		m_nHeight3D = wndAlgoImg.m_nHeight3D;
		dAngle = wndAlgoImg.dAngle;
		m_nChannel = wndAlgoImg.m_nChannel;
		m_nLight_index = wndAlgoImg.m_nLight_index;
		m_Use_algo_model = wndAlgoImg.m_Use_algo_model;
		m_Use_model_nchannel = wndAlgoImg.m_Use_model_nchannel;

		m_fPartRoundingErrX = wndAlgoImg.m_fPartRoundingErrX;		//part clip rounding error correction value (unit:pixel)
		m_fPartRoundingErrY = wndAlgoImg.m_fPartRoundingErrY;
		m_fWndRoundingErrX = wndAlgoImg.m_fWndRoundingErrX;		//Wnd clip rounding error correction value (unit:pixel)
		m_fWndRoundingErrY = wndAlgoImg.m_fWndRoundingErrY;

		if (wndAlgoImg.m_ucArr2D)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &m_ucArr2D, sizeof(UCHAR) * nSizeImg);
			memcpy_s(m_ucArr2D, sizeof(UCHAR) * nSizeImg, wndAlgoImg.m_ucArr2D, sizeof(UCHAR) * nSizeImg);
		}
		else
			m_ucArr2D = NULL;

		//LJH 2016.05.28
		for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		{
			if (wndAlgoImg.m_ucArr2D_Mix[iLoopCount])
			{
				Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &m_ucArr2D_Mix[iLoopCount], sizeof(UCHAR) * nSizeImg);
				memcpy_s(m_ucArr2D_Mix[iLoopCount], sizeof(UCHAR) * nSizeImg, wndAlgoImg.m_ucArr2D_Mix[iLoopCount], sizeof(UCHAR) * nSizeImg);
			}
			else
				m_ucArr2D_Mix[iLoopCount] = NULL;
		}

		if (wndAlgoImg.m_fArr3D)
		{
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &m_fArr3D, sizeof(float) * nSizeImg);
			memcpy_s(m_fArr3D, sizeof(float) * nSizeImg, wndAlgoImg.m_fArr3D, sizeof(float) * nSizeImg);
		}
		else
			m_fArr3D = NULL;
		// 		UCHAR * m_ucArrCV;
		// 		float * m_fArr3D_part;		// C#에서 넘겨주는  Part 3D data

		return *this;
	}
}WndAlgoImg;

typedef struct tagSimilarPartInfo
{
	CString m_sPartCode;
}SimilarPartInfo;

typedef struct tagfileAlgoPath
{
	wchar_t m_sPathModelTeach[MAX_STRLEN];
	int type;

	//2017.03.29 shkim Divide Area
	int DiviLnR[CNT_PATTERN_PATH];
	int DiviLnC[CNT_PATTERN_PATH];

	double GapLnR[CNT_PATTERN_PATH][CNT_PATTERN_DIVISION_R];
	double GapLnC[CNT_PATTERN_PATH][CNT_PATTERN_DIVISION_C];

	double DiviScore[CNT_PATTERN_PATH][CNT_PATTERN_SCORE];

	BOOL DetailSearch[CNT_PATTERN_PATH];
	BOOL m_bUseCharacter;

	int m_nModelFilter;
}fileAlgoPath;
typedef struct tagSPOCRfileAlgoPath
{
	wchar_t m_sPathModelPath[MAX_STRLEN];
}SPOCRfileAlgoPath;
typedef struct tagSPOCRFontList
{
	wchar_t m_sFontList[MAX_STRLEN];
}SPOCRFontList;
typedef struct tagSShapeXfileAlgoPath
{
	wchar_t m_sPathModelPath[MAX_STRLEN];
}SShapeXfileAlgoPath;

//LJH
typedef struct tagInspAlgoLight
{
	int m_nImageNum;
	InspLightType m_eLightType;
	int m_nRedValue;
	int m_nGreenValue;
	int m_nBlueValue;
	int m_nWhiteValue;

	int m_nLightCnt;
	int *m_nArrRedValue;
	int *m_nArrGreenValue;
	int *m_nArrBlueValue;
	int *m_nArrWhiteValue;
	int *m_nArrCalculation;
	int *m_nArrLightPosition;

	void Clone(tagInspAlgoLight &inspAlgoLight)
	{
		inspAlgoLight.m_nImageNum	= m_nImageNum;
		inspAlgoLight.m_eLightType	= m_eLightType;
		inspAlgoLight.m_nRedValue	= m_nRedValue;
		inspAlgoLight.m_nGreenValue	= m_nGreenValue;
		inspAlgoLight.m_nBlueValue	= m_nBlueValue;
		inspAlgoLight.m_nWhiteValue	= m_nWhiteValue;

		inspAlgoLight.m_nLightCnt			= m_nLightCnt;
		inspAlgoLight.m_nArrRedValue		= m_nArrRedValue;
		inspAlgoLight.m_nArrGreenValue		= m_nArrGreenValue;
		inspAlgoLight.m_nArrBlueValue		= m_nArrBlueValue;
		inspAlgoLight.m_nArrWhiteValue		= m_nArrWhiteValue;
		inspAlgoLight.m_nArrCalculation		= m_nArrCalculation;
		inspAlgoLight.m_nArrLightPosition	= m_nArrLightPosition;

		if(m_nLightCnt != 0)
		{
			if(!m_nArrRedValue)			return;
			if(!m_nArrGreenValue)		return;
			if(!m_nArrBlueValue)		return;
			if(!m_nArrWhiteValue)		return;
			if(!m_nArrCalculation)		return;
			if(!m_nArrLightPosition)	return;
		}
	}
}InspAlgoLight;

typedef struct tagTotalInspExceptArea
{
	int m_nUsedMaskingValue;
	//RECT m_rcArrMaskingROI[MAX_MASKING_NUM];
	RECT* m_rcArrMaskingROI;

	int m_nUsedInspPolygon;
	BOOL m_bConvetExceptROI;
	POINTF m_ptArrInspPolygon[MAX_INSP_AREA_COUNT];

	int m_nUsedWndPolygon;
	POINTF m_ptArrWndPolygon[MAX_INSP_AREA_COUNT];

	double dAngle;

	tagTotalInspExceptArea()
	{
		m_nUsedMaskingValue = 0;

		//RECT rcInit;
		//rcInit.left = 0; rcInit.bottom = 0; rcInit.right = 0; rcInit.top = 0;
		//for (int i = 0; i < MAX_MASKING_NUM; i++)
		//	m_rcArrMaskingROI[i] = rcInit;
		m_rcArrMaskingROI = NULL;

		// for Insp Area Polygon
		m_nUsedInspPolygon = 0;
		m_bConvetExceptROI = FALSE;

		POINTF ptInit;
		ptInit.x = 0; ptInit.y = 0;

		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
			m_ptArrInspPolygon[i] = ptInit;

		m_nUsedWndPolygon = 0;
		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
			m_ptArrWndPolygon[i] = ptInit;

		dAngle = 0;
	}
	void SetData(tagTotalInspExceptArea sData)
	{
		m_nUsedMaskingValue = sData.m_nUsedMaskingValue;

		//for (int i = 0; i < sData.m_nUsedMaskingValue; i++)
		//{
		//	m_rcArrMaskingROI[i].left = sData.m_rcArrMaskingROI[i].left;
		//	m_rcArrMaskingROI[i].right = sData.m_rcArrMaskingROI[i].right;
		//	m_rcArrMaskingROI[i].top = sData.m_rcArrMaskingROI[i].top;
		//	m_rcArrMaskingROI[i].bottom = sData.m_rcArrMaskingROI[i].bottom;
		//}
		m_rcArrMaskingROI = sData.m_rcArrMaskingROI;
		// for Insp Area Polygon
		m_nUsedInspPolygon = sData.m_nUsedInspPolygon;
		m_bConvetExceptROI = sData.m_bConvetExceptROI;
		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		{
			m_ptArrInspPolygon[i].x = sData.m_ptArrInspPolygon[i].x;
			m_ptArrInspPolygon[i].y = sData.m_ptArrInspPolygon[i].y;
		}

		m_nUsedWndPolygon = sData.m_nUsedWndPolygon;
		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		{
			m_ptArrWndPolygon[i].x = sData.m_ptArrWndPolygon[i].x;
			m_ptArrWndPolygon[i].y = sData.m_ptArrWndPolygon[i].y;
		}

		dAngle = sData.dAngle;
	}

}TotalInspExceptArea;

typedef struct tagInspAlgo
{
	int m_nAlgoId;
	InspAlgoType m_eAlgoType;
	BOOL m_bIsRequired;       // 필수 검사 여부 (FALSE이면 검사 실패시 다음 알고리즘으로 넘어가지만, TRUE이면 다음 알고리즘으로 넘어가지 않고 바로 NG 처리)
	BOOL m_bAlgoEnable;
	BOOL m_bAlgoGroup;
	InspLightType m_eLightType;
	int m_nRedValue;
	int m_nGreenValue;
	int m_nBlueValue;
	int m_nWhiteValue;

	BOOL m_bUsingManualDefectCode;
	int m_nManualDefectCode;
	int m_nManualSubDefectCode;

	void *m_ptrInspAlgoParam;

	int m_nLightCnt;
	int *m_nArrRedValue;
	int *m_nArrGreenValue;
	int *m_nArrBlueValue;
	int *m_nArrWhiteValue;
	int *m_nArrCalculation;
	int *m_nArrLightPosition;

	//LJH 2016.05.28 합칠 영상에 대한 조건 설정
	int m_nMixCount;
	InspAlgoLight InspAlgoLightsMix[2];

	int m_nUsedMaskingValue;
	//RECT m_rcArrMaskingROI[MAX_MASKING_NUM];
	RECT* m_rcArrMaskingROI;

	int m_nUsedInspPolygon;
	BOOL m_bConvetExceptROI;
	POINTF m_ptArrInspPolygon[MAX_INSP_AREA_COUNT];

	BOOL m_bUseAI;
	int m_AIModelID;
	//************************************
	// Qualifier:  멤버데이터 m_ptrInspAlgoParam를 새로 만들어 복사
	//************************************
	void Clone(tagInspAlgo &inspAlgo);

	int GetKindInspImg() const
	{
		int nTypeInspImg = eImg_Top_R;
		switch(m_eLightType)
		{
		case Top_Light:
			{
				if(m_nRedValue > 100)
					nTypeInspImg = eImg_Top_R;
				else if(m_nGreenValue > 100)
					nTypeInspImg = eImg_Top_G;
				else if(m_nBlueValue > 100)
					nTypeInspImg = eImg_Top_B;
				else if(m_nWhiteValue > 100)
					nTypeInspImg = eImg_Top_W;
			}
			break;
		case Middle_Light:
			{
				if(m_nRedValue > 100)
					nTypeInspImg = eImg_Middle_R;
				else if(m_nBlueValue > 100)
					nTypeInspImg = eImg_Middle_B;
			}
			break;
		case Bottom_Light:
			{
				if(m_nRedValue > 100)
					nTypeInspImg = eImg_Bottom_R;
				else if(m_nBlueValue > 100)
					nTypeInspImg = eImg_Bottom_B;
			}
			break;
		case User_Light:
			{
				for (int i = 0; i< m_nLightCnt; i++)
				{
					if(m_nArrRedValue[i] > 100)
						nTypeInspImg = eImg_User_R;
					else if(m_nArrGreenValue[i] > 100)
						nTypeInspImg = eImg_User_G;
					else if(m_nArrBlueValue[i] > 100)
						nTypeInspImg = eImg_User_B;
					else if(m_nArrWhiteValue[i] > 100)
						nTypeInspImg = eImg_User_W;
				}
			}
			break;
		}

		return nTypeInspImg;
	}
	void LightClone(tagInspAlgo &inspAlgo)
	{
		inspAlgo.m_eLightType = m_eLightType;
		inspAlgo.m_nLightCnt = m_nLightCnt;
		inspAlgo.m_nArrRedValue = m_nArrRedValue;
		inspAlgo.m_nArrGreenValue = m_nArrGreenValue;
		inspAlgo.m_nArrBlueValue = m_nArrBlueValue;
		inspAlgo.m_nArrWhiteValue = m_nArrWhiteValue;
		inspAlgo.m_nArrCalculation = m_nArrCalculation;
		inspAlgo.m_nArrLightPosition = m_nArrLightPosition;
	}
}InspAlgo;

typedef struct tagLightTypeBuf
{
	UCHAR* m_pucTRed;
	UCHAR* m_pucTGreen;
	UCHAR* m_pucTBlue;
	UCHAR* m_pucTWhite;

	UCHAR* m_pucMRed;
	UCHAR* m_pucMGreen;
	UCHAR* m_pucMBlue;
	UCHAR* m_pucMWhite;

	UCHAR* m_pucBRed;
	UCHAR* m_pucBGreen;
	UCHAR* m_pucBBlue;
	UCHAR* m_pucBWhite;

	int m_nImgWidth;
	int m_nImgHeight;

	int m_nImgCnt;
	int* m_pnRedValue;
	int* m_pnGreenValue;
	int* m_pnBlueValue;
	int* m_pnWhiteValue; 
	int* m_pnPosition;
	int* m_pnCalculation;

	int m_nROIImgWidth;
	int m_nROIImgHeight;
	double m_dROIX;
	double m_dROIY;

	tagLightTypeBuf()
	{
		Init();
	}
	~tagLightTypeBuf()
	{
		//Distory();
	}
	void Distory()
	{
		if (m_pnRedValue)
		{
			g_pMManager->pem_delete(m_pnRedValue, true);
			m_pnRedValue = NULL;
		}
		if (m_pnGreenValue)
		{
			g_pMManager->pem_delete(m_pnGreenValue, true);
			m_pnGreenValue = NULL;
		}
		if (m_pnBlueValue)
		{
			g_pMManager->pem_delete(m_pnBlueValue, true);
			m_pnBlueValue = NULL;
		}
		if (m_pnWhiteValue)
		{
			g_pMManager->pem_delete(m_pnWhiteValue, true);
			m_pnWhiteValue = NULL;
		}
		if (m_pnPosition)
		{
			g_pMManager->pem_delete(m_pnPosition, true);
			m_pnPosition = NULL;
		}
		if (m_pnCalculation)
		{
			g_pMManager->pem_delete(m_pnCalculation, true);
			m_pnCalculation = NULL;
		}
	}
	void Init()
	{
		m_pucTRed = NULL;
		m_pucTGreen = NULL;
		m_pucTBlue = NULL;
		m_pucTWhite = NULL;

		m_pucMRed = NULL;
		m_pucMGreen = NULL;
		m_pucMBlue = NULL;
		m_pucMWhite = NULL;

		m_pucBRed = NULL;
		m_pucBGreen = NULL;
		m_pucBBlue = NULL;
		m_pucBWhite = NULL;

		m_nImgWidth = 0;
		m_nImgHeight = 0;

		m_nImgCnt = 0;
		m_pnRedValue = NULL;
		m_pnGreenValue = NULL;
		m_pnBlueValue = NULL;
		m_pnWhiteValue = NULL;
		m_pnPosition = NULL;
		m_pnCalculation = NULL;

		m_nROIImgWidth = 0;
		m_nROIImgHeight = 0;
		m_dROIX = 0;
		m_dROIY = 0;
	}
}LightTypeBuf;

// Foreign Silk
typedef struct tagSilkLightBuf
{
	UCHAR* m_pucTWhite;
	UCHAR* m_pucBRed;
	UCHAR* m_pucBBlue;

	int m_nImgWidth;
	int m_nImgHeight;

	int m_nImgCnt;
	int* m_pnRedValue;
	int* m_pnGreenValue;
	int* m_pnBlueValue;
	int* m_pnWhiteValue; 
	int* m_pnPosition;
	int* m_pnCalculation;
}SilkLightBuf;
typedef struct tagRstWndArr
{
	byte m_byG;
	int m_nArrRectCnt;
	RECT m_rcArrRect[WND_ARRAY_CNTS];
	float m_fArrRstA[WND_ARRAY_CNTS];
	float m_fArrRstH[WND_ARRAY_CNTS];
	float m_fArrRstHH[WND_ARRAY_CNTS];
	float m_fArrRstW[WND_ARRAY_CNTS];
	float m_fArrRstL[WND_ARRAY_CNTS];
	float m_fArrRstG[WND_ARRAY_CNTS];
	float m_fArrRstV[WND_ARRAY_CNTS];
	tagRstWndArr()
	{
		m_byG = 0;
		m_nArrRectCnt = 0;
		memset(m_rcArrRect, 0, WND_ARRAY_CNTS * sizeof(RECT));
		memset(m_fArrRstA, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(m_fArrRstH, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(m_fArrRstHH, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(m_fArrRstW, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(m_fArrRstL, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(m_fArrRstG, 0.0, WND_ARRAY_CNTS * sizeof(float));
		memset(m_fArrRstV, 0.0, WND_ARRAY_CNTS * sizeof(float));
	}
}RstWndArr;
#pragma endregion _Algorithm_

typedef struct tagFullMapData
{
	int m_nCH;
	int m_nSrcW;
	int m_nSrcH;

	int m_nRstCH;
	int m_nRstW;
	int m_nRstH;

	int m_nW;
	int m_nH;
	RECT m_rcFOV;
}FullMapData;

typedef struct tagPortionRect
{
	double dLeft;
	double dTop;
	double dRight;
	double dBottom;
}PortionRect;

typedef struct tagPortionSize
{
	double CX;
	double CY;
	double Widht;
	double Height;
}PortionSize;

typedef struct tagRoiImgBuf_OCR
{
	cv::Point ptPos;

	cv::Mat imgTop_R;
	cv::Mat imgTop_G;
	cv::Mat imgTop_B;
	cv::Mat imgTop_W;

	cv::Mat imgMiddle_R;
	cv::Mat imgMiddle_B;

	cv::Mat imgBottom_R;
	cv::Mat imgBottom_B;

	cv::Mat imgSide1_R;
	cv::Mat imgSide1_G;
	cv::Mat imgSide1_B;

	cv::Mat imgSide2_R;
	cv::Mat imgSide2_G;
	cv::Mat imgSide2_B;

	cv::Mat imgSide3_R;
	cv::Mat imgSide3_G;
	cv::Mat imgSide3_B;

	cv::Mat imgSide4_R;
	cv::Mat imgSide4_G;
	cv::Mat imgSide4_B;

	cv::Mat& GetPtrImg(int nNum)
	{
		switch (nNum)
		{
		case eInspRoiImg_imgTop_R:
			return imgTop_R;
		case eInspRoiImg_imgTop_G:
			return imgTop_G;
		case eInspRoiImg_imgTop_B:
			return imgTop_B;
		case eInspRoiImg_imgTop_W:
			return imgTop_W;
		case eInspRoiImg_imgMiddle_R:
			return imgMiddle_R;
		case eInspRoiImg_imgMiddle_B:
			return imgMiddle_B;
		case eInspRoiImg_imgBottom_R:
			return imgBottom_R;
		case eInspRoiImg_imgBottom_B:
			return imgBottom_B;
		case eInspRoiImg_imgSide1_R:
			return imgSide1_R;
		case eInspRoiImg_imgSide1_G:
			return imgSide1_G;
		case eInspRoiImg_imgSide1_B:
			return imgSide1_B;
		case eInspRoiImg_imgSide2_R:
			return imgSide2_R;
		case eInspRoiImg_imgSide2_G:
			return imgSide2_G;
		case eInspRoiImg_imgSide2_B:
			return imgSide2_B;
		case eInspRoiImg_imgSide3_R:
			return imgSide3_R;
		case eInspRoiImg_imgSide3_G:
			return imgSide3_G;
		case eInspRoiImg_imgSide3_B:
			return imgSide3_B;
		case eInspRoiImg_imgSide4_R:
			return imgSide4_R;
		case eInspRoiImg_imgSide4_G:
			return imgSide4_G;
		case eInspRoiImg_imgSide4_B:
			return imgSide4_B;
		}
	}

}InspRoiImgBuf_OCR;

// 파트 분할에 의해 분할된 OCR 정보
typedef struct tagOCRPortion
{
	// Crop할 pixel좌표 저장
	CRect rcOCR;

	// rcOCR을 이용해 Crop한 이미지
	InspRoiImgBuf_OCR OcrImgBuf;

}OCRPortion, *LPOCRPortion;

typedef struct tagPortionItem
{
	double dBeforeInspCX;
	double dBeforeInspCY;

	PortionSize szPart;
	PortionSize szAngle;

	PortionRect rtFov;

	ZmapData partZmapData;
	InspRoiImgBuf partImgBuf;
	RoiColorBuf partImgColorBuf;
	RoiByteBuf Save3DRawData;

	// 주석 아래는 Merge 시 계산되어 값이 입력되는 항목
	// 겹쳐지는 시작점 (이전 Portion의 겹쳐진 부분에 대한 50%지점)
	int nMergeStartPosZmapX;
	int nMergeStartPosZmapY;

	// 짤라야할 시작부분 (현재 Portion에서 겹쳐진 부분의 50%를 짜른위치)
	int nMergeCuttingPosX;
	int nMergeCuttingPosY;

	bool bIgnoreItem;
	bool bIsInclude; // 사용 안함

	// 1개의 Part에 2개 이상의 OCR이 있는 경우가 있어 list 로 저장
	std::map<int, LPOCRPortion> map_PortionOCR;

}PortionedPart, *LPPortionedPart;

typedef struct tagPortionDataParam
{
	// key = PartID, 파트는 조각난 여러 list를 관리
	std::map<int, std::list<LPPortionedPart>*> map_PortionedPart;

	// 조각이 모두 모인경우 합산을 위한 버퍼 
	ZmapData mergePart_ZmapData;
	InspRoiImgBuf mergePart_ImgBuf;
	RoiColorBuf mergePart_ImgColorBuf;
	RoiByteBuf merge_Save3DRawData;

	PortionRect rtPart;
	
	// Part에 OCR 존재하는 경우 좌표저장
	// map<PartID, std::map<WindID, CRect>
	// CRect 실제 좌표, 소숫점 제거를 위해 *1000한 값
	std::map<int, std::map<int, CRect>*> map_OCRInfo;

}PortionData, *LPPortionData;

#pragma region _MAIN PARAMETER_
typedef struct tagBigPartInfo
{
	int m_nIndex;
	double m_dFovL;
	double m_dFovT;
	float m_fBigPartErrX;
	float m_fBigPartErrY;
	RECT m_rcROI;
	void CopyData(tagBigPartInfo sData)
	{
		m_nIndex = sData.m_nIndex;
		m_dFovL = sData.m_dFovL;
		m_dFovT = sData.m_dFovT;
		m_fBigPartErrX = sData.m_fBigPartErrX;
		m_fBigPartErrY = sData.m_fBigPartErrY;
		m_rcROI.left = sData.m_rcROI.left;
		m_rcROI.right = sData.m_rcROI.right;
		m_rcROI.top = sData.m_rcROI.top;
		m_rcROI.bottom = sData.m_rcROI.bottom;
	}
	bool BigPartRoundingErr(float fL, float fT, int nW, int nH, float &fErrX, float &fErrY)
	{
		float fR = fL + nW;
		float fB = fT + nH;
		float fROIL = m_rcROI.left;
		float fROIR = m_rcROI.right;
		float fROIT = m_rcROI.top;
		float fROIB = m_rcROI.bottom;
		if (fROIL <= fL && fROIR >= fL &&
			fROIL <= fR && fROIR >= fR &&
			fROIT <= fT && fROIB >= fT &&
			fROIT <= fB && fROIB >= fB)
		{
			fErrX = m_fBigPartErrX;
			fErrY = m_fBigPartErrY;
			return true;
		}
		if ((fROIL <= fL && fROIR >= fL) || (fROIL <= fR && fROIR >= fR))
		{
		}
		else
			return false;
		if ((fROIT <= fT && fROIB >= fT) || (fROIT <= fB && fROIB >= fB))
		{

		}
		else
			return false;
		float fTempL = fL > fROIL ? fL : fROIL;
		float fTempR = fR < fROIR ? fR : fROIR;
		float fTempT = fT > fROIT ? fT : fROIT;
		float fTempB = fB < fROIB ? fB : fROIB;
		float fTempW = fTempR - fTempL;
		float fTempH = fTempB - fTempT;
		float fPerW = fTempW / (float)nW;
		float fPerH = fTempH / (float)nH;
		if (fPerW >= 0.6 && fPerH >= 0.6)
		{
			fErrX = m_fBigPartErrX;
			fErrY = m_fBigPartErrY;
			return true;
		}
		return false;
	}
}BigPartInfo;
typedef struct tagInspBigPartInfo
{
	int nTotal;
	tagBigPartInfo m_sInfo[MAX_BIG_INFO];
	void CopyData(tagInspBigPartInfo sData)
	{
		nTotal = sData.nTotal;
		for (int a = 0; a < MAX_BIG_INFO; a++)
			m_sInfo[a].CopyData(sData.m_sInfo[a]);
	}
	void BigPartRoundingErr(float fL, float fT, int nW, int nH, float &fErrX, float &fErrY)
	{
		if (nTotal <= 1)
			return;
		for (int a = nTotal - 1; a >= 0; a--)
		{
			for (int b = 0; b < nTotal; b++)
			{
				if (m_sInfo[b].m_nIndex != a)
					continue;
				if (m_sInfo[b].BigPartRoundingErr(fL, fT, nW, nH, fErrX, fErrY))
					return;
				break;
			}
		}
	}
}InspBigPartInfo;
typedef struct tagPADPatternPoly
{
	int m_nPolyCnt;
	POINTF* m_ptArrPoly;	// Pad Polygon

	tagPADPatternPoly()
	{
		m_nPolyCnt = 0;
		m_ptArrPoly = NULL;
	}

	void Init()
	{
		m_nPolyCnt = 0;
		m_ptArrPoly = NULL;
	}

	void CopyData(tagPADPatternPoly* sData)
	{
		m_nPolyCnt = sData->m_nPolyCnt;
		m_ptArrPoly = g_pMManager->pem_new<POINTF>(true, m_nPolyCnt, (PCHAR)__FUNCTION__, __LINE__);
		if (sData->m_ptArrPoly == NULL)
			memset(m_ptArrPoly, 0, sizeof(POINTF) * m_nPolyCnt);
		else
			memcpy(m_ptArrPoly, sData->m_ptArrPoly, sizeof(POINTF) * m_nPolyCnt);
	}

	void DeleteData()
	{
		if (m_ptArrPoly)
		{
			g_pMManager->pem_delete(m_ptArrPoly, true);
			m_ptArrPoly = NULL;
		}
	}
}PADPatternPoly;
typedef struct tagInspPartParam
{
	int inspType;
	//	int lightType_main;		// X

	//	BOOL useZmap;			// X
	int InspWire;

	double cx;
	double cy;
	double width;
	double length;

	double bdrCx;		// X
	double bdrCy;		// X
	double bdrWidth;	// X
	double bdrLength;	// X

	double anyAngleCx;	// X
	double anyAngleCy;	// X
	double anyAngleWidth;	// X
	double anyAngleLength;	// X

	void* targetImg;			// X
	void* inspParam;			// X
	int nAlgorithmCnt;			// Algorithm 개수// ++
	InspAlgo * vArrAlgoParam;		// Algorithm Parameter // ++

	int lightType_red;   //color inspection only	X
	int lightType_blue;  //color inspection only	X
	void* targetImg_R;  //color inspection only	X
	void* targetImg_B;  //color inspection only	X

	int lightType_Top;	//solder inspection only 	X
	int lightType_Bottom;	//solder inspection only	X
	void* targetImg_Top;  //solder inspection only	X
	void* targetImg_Bottom;  //solder inspection only	X

	InspImgBuf imgBuf;				// X

	int wndIndex;

	int groupIndex;

	int nAlignWndID;		// ++
	int nParentWndID;

	int nInspCameraType; // For Side Camera Img
	byte WndInspType;
	int m_nAlignPartWnd;
	int m_nUsedWndPolygon;           // 설정 ROI 개수 (1~20)	
	POINTF m_ptArrWndPolygon[MAX_INSP_AREA_COUNT];

	// byte byUseZ2Axis;
	int Gen2D;
}InspPartParam;
typedef struct tagInspPartInfo
{
	//input for UI
	wchar_t modelName[MAX_STRLEN];
	int moduleIndex;
	int fovIndex;
	double fovCx;
	double fovCy;

	// 중첩파트의 FOV 중첩검사 -LWJ ModifyInsp
	double dFovLeft;
	double dFovRight;
	double dFovTop;
	double dFovBottom;

	// 검사전 파트 중심 값 (for sort) -LWJ ModifyInsp
	double dBeforeInspCx;
	double dBeforeInspCy;

	// 분할되기전 파트 크기 -LWJ ModifyInsp
	double dPtPartWidth;
	double dPtPartHeight;

	double partCx;
	double partCy;
	double partWidth;    // 회전 각도 적용된 사이즈
	double partHeight;

	//rounding error correction value that occurs when calculating coordinates. unit:pixel  NYJ 2020/12/16
	float fPartRoundingErrX;
	float fPartRoundingErrY;
	float fWndRoundingErrX;
	float fWndRoundingErrY;

	double anyAngleCx;	// 일반각일 경우 각도 적용된 Boundary 중심 좌표	// ++
	double anyAngleCy;		// ++
	double anyAngleWidth;	// 일반각일 경우 각도 적용된 Boundary 사이즈	// ++
	double anyAngleLength;		// ++

	ZmapData zmapData;			// FOV 3D Image
	InspImgBuf fovImgBuf;		// FOV 2D Image // ++
	ZmapData partZmapData;           // Part 3D Image // ++
	InspRoiImgBuf partImgBuf;	// Part 2D Image // ++
	RoiColorBuf partImgColorBuf;	// Color Map2 // ++

	ZmapData zmapForeignData;

	InspParamBGA_Part bgaParam; //BGA Inspection only..

	double angle;

	int partIndex;		// Fov 내 Part Index, 0~	
	int BtmCameraUse;
	int BtmCameraWidth;
	int BtmCameraHeight;
	int BtmCameraIdx;


	wchar_t ModuleNo[MAX_STRLEN];
	wchar_t PartNo[MAX_STRLEN];
	wchar_t ImagePath[MAX_STRLEN];
	wchar_t s2DImagePath[MAX_STRLEN];
	wchar_t s3DImagePath[MAX_STRLEN];
	// 분할된 파트를 관리하기 위한 ID -LWJ ModifyInsp
	BOOL bPortionData;   // 분할파트 여부
	BOOL bLastPortion;   // 분할마지막 파트 (병합)
	BOOL bFirstInsp;	 // Auto 검사 시 최초 Portion 데이터, 메모리 해지용
	BOOL bBigPartData;   // BigPart 인지 아닌지 구분하는 변수
	int nFovID;
	int nPartID;         // 머지 빅 파트 고유값 key로 사용
	int nPartIDOrg;		 // 가지고 있는 고유 파트 아이디

	RoiByteBuf Save3DRawData;	    // Part ROI 3D Raw Image
	byte byPCBPosInspLight;    // PCB 끝단 검사용 조명 0: PCB 끝단 검사 아님, 1: 3D 조명 옵션, 2: 2D 조명 옵션

	byte byUseAngleColor; // AngleColor 사용 유무

	BOOL bIsChip; // 파트가 Chip인지 아닌지 구분하는 변수

	InspPartParam * pWindows;
	int nWindowCount;

	wchar_t s3DRawName[MAX_STRLEN];
	int m_nLane;

	BOOL bPartGroundLevel;

	tagInspBigPartInfo m_sBigInfo;

	BOOL m_bPadPattern;
	PADPatternPoly m_sPadPoly;  // Pad Polygon
	int m_nOverlapPadPolyCnt;    // Overlap Pad Polygon 개수
	PADPatternPoly* m_sArrOverlapPadPoly; // Overlap Pad Polygon
	int m_nPartPropType;
	int m_nPOLY_W;
	int m_nPOLY_H;
	UCHAR* m_ucPOLY;
	void* m_sRstAlgo_P;
	int m_nTYPE;
	int m_nOPT;

	BOOL bStickerInsp;
	POINTF pPoHiddenArea[StickerCnt * StickerPo]; // PartImgPx

	BOOL bSync2DLgtPosWithJobConfig;

	int nUseAISegmentation;
	void CopyData(tagInspPartInfo* sData, bool bAll = false)
	{
		moduleIndex = sData->moduleIndex;
		fovIndex = sData->fovIndex;
		fovCx = sData->fovCx;
		fovCy = sData->fovCy;

		dFovLeft = sData->dFovLeft;
		dFovRight = sData->dFovRight;
		dFovTop = sData->dFovTop;
		dFovBottom = sData->dFovBottom;

		dBeforeInspCx = sData->dBeforeInspCx;
		dBeforeInspCy = sData->dBeforeInspCy;

		dPtPartWidth = sData->dPtPartWidth;
		dPtPartHeight = sData->dPtPartHeight;

		partCx = sData->partCx;
		partCy = sData->partCy;
		partWidth = sData->partWidth;
		partHeight = sData->partHeight;

		fPartRoundingErrX = sData->fPartRoundingErrX;
		fPartRoundingErrY = sData->fPartRoundingErrY;
		fWndRoundingErrX = sData->fWndRoundingErrX;
		fWndRoundingErrY = sData->fWndRoundingErrY;

		anyAngleCx = sData->anyAngleCx;
		anyAngleCy = sData->anyAngleCy;
		anyAngleWidth = sData->anyAngleWidth;
		anyAngleLength = sData->anyAngleLength;

		angle = sData->angle;

		partIndex = sData->partIndex;
		BtmCameraUse = sData->BtmCameraUse;
		BtmCameraWidth = sData->BtmCameraWidth;
		BtmCameraHeight = sData->BtmCameraHeight;

		bPortionData = sData->bPortionData;
		bLastPortion = sData->bLastPortion;
		bFirstInsp = sData->bFirstInsp;
		bBigPartData = sData->bBigPartData;
		nFovID = sData->nFovID;
		nPartID = sData->nPartID;
		nPartIDOrg = sData->nPartIDOrg;

		byPCBPosInspLight = sData->byPCBPosInspLight;
		byUseAngleColor = sData->byUseAngleColor;
		bIsChip = sData->bIsChip;
		nWindowCount = sData->nWindowCount;
		bPartGroundLevel = sData->bPartGroundLevel;

		memcpy(modelName, sData->modelName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(ModuleNo, sData->ModuleNo, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(PartNo, sData->PartNo, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(ImagePath, sData->ImagePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(s2DImagePath, sData->s2DImagePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(s3DImagePath, sData->s3DImagePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(s3DRawName, sData->s3DRawName, sizeof(wchar_t) * MAX_STRLEN);

		partZmapData.CopyData(sData->partZmapData);
		partImgBuf.CopyData(sData->partImgBuf);
		partImgColorBuf.CopyData(sData->partImgColorBuf, partImgBuf.nImageSizeX, partImgBuf.nImageSizeY);
		bgaParam.CopyData(sData->bgaParam);
		Save3DRawData.CopyData(sData->Save3DRawData);

		if (bAll)
		{
			zmapData.CopyData(sData->zmapData);
			fovImgBuf.CopyData(sData->fovImgBuf);
			zmapForeignData.CopyData(sData->zmapForeignData);
		}
		m_sBigInfo.CopyData(sData->m_sBigInfo);

		m_bPadPattern = sData->m_bPadPattern;
		m_sPadPoly.CopyData(&sData->m_sPadPoly);

		m_nOverlapPadPolyCnt = sData->m_nOverlapPadPolyCnt;
		if (m_nOverlapPadPolyCnt > 0)
		{
			m_sArrOverlapPadPoly = g_pMManager->pem_new<PADPatternPoly>(true, m_nOverlapPadPolyCnt, (PCHAR)__FUNCTION__, __LINE__);

			for (int i = 0; i < m_nOverlapPadPolyCnt; i++)
			{
				m_sArrOverlapPadPoly[i].CopyData(&sData->m_sArrOverlapPadPoly[i]);
			}
		}
		else
		{
			m_sArrOverlapPadPoly = NULL;
		}
		m_nPartPropType = sData->m_nPartPropType;
		bStickerInsp = sData->bStickerInsp;
		for (int nPointIdx = 0; nPointIdx < StickerCnt * StickerPo; nPointIdx++)
			pPoHiddenArea[nPointIdx] = sData->pPoHiddenArea[nPointIdx];
		nUseAISegmentation = sData->nUseAISegmentation;

		bSync2DLgtPosWithJobConfig = sData->bSync2DLgtPosWithJobConfig;
	}

	void DeleteData(bool bAll = false)
	{
		partZmapData.DeleteData();
		partImgBuf.DeleteData();
		partImgColorBuf.DeleteData();
		bgaParam.DeleteData();
		Save3DRawData.DeleteData();

		if (bAll)
		{
			zmapData.DeleteData();
			fovImgBuf.DeleteData();
			zmapForeignData.DeleteData();
		}

		m_sPadPoly.DeleteData();
		if (m_sArrOverlapPadPoly)
		{
			for (int i = 0; i < m_nOverlapPadPolyCnt; i++)
			{
				m_sArrOverlapPadPoly[i].DeleteData();
			}

			g_pMManager->pem_delete(m_sArrOverlapPadPoly, true);
			m_sArrOverlapPadPoly = NULL;
			m_nOverlapPadPolyCnt = 0;
		}
	}
	bool UseData(int nType)
	{
		if ((m_nOPT & nType) == nType)
			return true;
		return false;
	}
}InspPartInfo;


typedef struct tagTeachParam
{
	int teachType;
	int lightType_main;

	BOOL useZmap;

	double cx;
	double cy;
	double width;
	double length;

	double bdrCx;
	double bdrCy;
	double bdrWidth;
	double bdrLength;

	double anyAngleCx;
	double anyAngleCy;
	double anyAngleWidth;
	double anyAngleLength;

	void* targetImg;
	void* teachParam;

	int lightType_red;	//color inspection only
	int lightType_blue;	//color inspection only
	void* targetImg_R;  //color inspection only
	void* targetImg_B;  //color inspection only

	int lightType_Top;	//solder inspection only
	int lightType_Bottom;	//solder inspection only
	void* targetImg_Top;  //solder inspection only
	void* targetImg_Bottom;  //solder inspection only

	InspImgBuf imgBuf;

}TeachParam;

typedef struct tagInspParamTemp
{
	int inspType;

	double cx;
	double cy;
	double width;
	double length;

	double bdrCx;
	double bdrCy;
	double bdrWidth;
	double bdrLength;

	double partAngle;

	int nAlgorithmCnt;			// Algorithm 개수// ++
	InspAlgo * vArrAlgoParam;		// Algorithm Parameter // ++
	void* inspParam;	// X

	wchar_t wndName[MAX_STRLEN];

	int groupIndex;

// 	~tagInspParamTemp()
// 	{
// 		Destroy();
// 	}

	void Destroy()
	{
		int nCntAlgo = nAlgorithmCnt;
		for(int nAlgo = 0; nAlgo < nCntAlgo; nAlgo++)
		{
			InspAlgo &inspAlgoCopy = vArrAlgoParam[nAlgo];
			void *&ptrInspAlgoCopy = inspAlgoCopy.m_ptrInspAlgoParam;
			if(!ptrInspAlgoCopy)
				return;

			//delete ptrInspAlgoCopy;
			g_pMManager->pem_delete(ptrInspAlgoCopy, false);
			ptrInspAlgoCopy = NULL;
		}

		//delete [] vArrAlgoParam;
		g_pMManager->pem_delete(vArrAlgoParam, true);
		vArrAlgoParam = NULL;
	}
}InspParamTemp;



typedef struct tagCoordinate
{
	int cx;
	int cy;
	int width;
	int length;

	int bdrCx;
	int bdrCy;
	int bdrWidth;
	int bdrLength;

	int anyAngleCx;
	int anyAngleCy;
	int anyAngleWidth;
	int anyAngleLength;

	double fovCx;
	double fovCy;

	double angle;


	//************************************
	// Parameter: const AlgoCoordinate & coordinateAlgo : 이미지 좌표
	//************************************
	void SetParamROI(const AlgoCoordinate &coordinateAlgo)
	{
		angle = coordinateAlgo.dROIAngle;
		width	= int(RounD(coordinateAlgo.dROIWidth));
		length	= int(RounD(coordinateAlgo.dROILength));
		cx = int(RounD(coordinateAlgo.dROICenterX));
		cy = int(RounD(coordinateAlgo.dROICenterY));

		bdrCx = cx;
		bdrCy = cy;
		bdrWidth = width;
		bdrLength = length;

		fovCx = coordinateAlgo.m_dFovCenterX;
		fovCy = coordinateAlgo.m_dFovCenterY;
	}

	//************************************
	// Parameter: const AlgoCoordinate & coordinateAlgo : 보드 좌표
	//************************************
	void SetParamROI(const AlgoCoordinate &coordinateAlgo, double dResolX, double dResolY)
	{
		angle = coordinateAlgo.dROIAngle;
		width	= int(coordinateAlgo.dROIWidth / dResolX);
		length	= int(coordinateAlgo.dROILength / dResolY);
		cx = int(coordinateAlgo.dROIWidth / 2. + (coordinateAlgo.dROICenterX / dResolX));
		cy = int(coordinateAlgo.dROILength / 2. - (coordinateAlgo.dROICenterY / dResolY));

		if((cx <= 0) || (cy <= 0))
		{
			cx = width /2;
			cy = length /2;
		}

		bdrCx = cx;
		bdrCy = cy;
		bdrWidth = width;
		bdrLength = length;

		fovCx = coordinateAlgo.m_dFovCenterX;
		fovCy = coordinateAlgo.m_dFovCenterY;
	}
}Coordinate;


typedef struct tagBodyOffset
{
	int offsetX;
	int offsetY;

}BodyOffset;


// Align Window 검사 결과 구조체
typedef struct tagAlignResult
{
	int nAlignWndID;
	int nWindowID;
	double centerX;
	double centerY;
	double offsetX;
	double offsetY;
	double theta;
	double TeachCenterX;
	double TeachCenterY;

	CRect rcBodyRect;

	BOOL m_bAlgoCenter;
	double m_dCenterX_T;
	double m_dCenterY_T;
	double m_dCenterX_R;
	double m_dCenterY_R;

	tagAlignResult()
	{
		Init();
	}

	void Init()
	{
		nWindowID = -1;
		nAlignWndID = -1;
		centerX = 0;
		centerY = 0;
		offsetX = 0.;
		offsetY = 0.;
		theta = 0.;

		rcBodyRect = CRect(0, 0, 0, 0);

		TeachCenterX = 0.;
		TeachCenterY = 0.;

		m_bAlgoCenter = FALSE;
		m_dCenterX_T = 0.;
		m_dCenterY_T = 0.;
		m_dCenterX_R = 0.;
		m_dCenterY_R = 0.;
	}
	void clone(tagAlignResult& result)
	{
		result.nWindowID = nWindowID;
		result.centerX = centerX;
		result.centerY = centerY;
		result.offsetX = offsetX;
		result.offsetY = offsetY;
		result.theta = theta;
		result.TeachCenterX = TeachCenterX;
		result.TeachCenterY = TeachCenterY;

		result.rcBodyRect.CopyRect(rcBodyRect);

		result.m_bAlgoCenter = m_bAlgoCenter;
		result.m_dCenterX_T = m_dCenterX_T;
		result.m_dCenterY_T = m_dCenterY_T;
		result.m_dCenterX_R = m_dCenterX_R;
		result.m_dCenterY_R = m_dCenterY_R;
	}

	struct tagAlignResult& operator=(const struct tagAlignResult& result)
	{
		nAlignWndID = result.nAlignWndID;
		nWindowID = result.nWindowID;
		centerX = result.centerX;
		centerY = result.centerY;
		offsetX = result.offsetX;
		offsetY = result.offsetY;
		theta = result.theta;
		TeachCenterX = result.TeachCenterX;
		TeachCenterY = result.TeachCenterY;

		rcBodyRect = result.rcBodyRect;

		m_bAlgoCenter = result.m_bAlgoCenter;
		m_dCenterX_T = result.m_dCenterX_T;
		m_dCenterY_T = result.m_dCenterY_T;
		m_dCenterX_R = result.m_dCenterX_R;
		m_dCenterY_R = result.m_dCenterY_R;

		return *this;
	}
}AlignResult;

typedef struct tagNgParam
{
	void* targetImg_R;  //color inspection only
	void* targetImg_G;  //color inspection only
	void* targetImg_B;  //color inspection only

	float redFator;
	float greenFator;
	float blueFator;
	float btmRedfactor;
	float btmBluefactor;

}NgParam;


typedef struct tagCalcRoiParm
{
	double partCx;
	double partCy;
	double partW;
	double partH;

	BOOL existMount;
	double mountCx;
	double mountCy;
	double mountW;
	double mountH;

	double fovCx;
	double fovCy;
	double fovW;
	double fovH;

}CalcRoiParm;

typedef struct tagDefaultGrayMeanStd
{	
	// Percentage 2개 값.
	int nStdGrayMin_Coat;
	int nStdGrayMax_Coat;

	// Value 2개 값.
	int nStdGrayValueMin_Coat;
	int nStdGrayValueMax_Coat;

	BOOL bUseStdGray_Non;
	int nStdGrayMin_Non;
	int nStdGrayMax_Non;
	// Value
	int nStdGrayValueMin_Non;
	int nStdGrayValueMax_Non;

	BOOL bUseStdGray_High;
	int nStdGrayMin_High;
	int nStdGrayMax_High;
	// Value
	int nStdGrayValueMin_High;
	int nStdGrayValueMax_High;
}DefaultGrayMeanStd;

typedef struct tagDefaultColorXYStd
{	
	float fStdColorXYMinX;
	float fStdColorXYMaxX;

	float fStdColorXYMinY;
	float fStdColorXYMaxY;
}DefaultColorXYStd;


typedef struct tagAlgoDebuggingMode
{
	int bAlgoDebuggingReady;	// 전달 문제로 int로 사용
	int nAlgoDebuggingRemainCnt;
	int bIncludeAlgoNG;			// 전달 문제로 int로 사용
	int bReviewMode;			// 전달 문제로 int로 사용
	int bType;					// 0은 Value, 1은 Percentage
}AlgoDebuggingMode;


#pragma endregion _PARAMETER_

#pragma region _MultiProcess
typedef struct tagExtAlgoLightBase
{
	InspLightType m_eLightType;
	int m_nImageNum;
	int m_nRedValue;
	int m_nGreenValue;
	int m_nBlueValue;
	int m_nWhiteValue;

	int m_nLightCnt;
	int m_nArrRedValue[LIGHT_CNT];
	int m_nArrGreenValue[LIGHT_CNT];
	int m_nArrBlueValue[LIGHT_CNT];
	int m_nArrWhiteValue[LIGHT_CNT];
	int m_nArrCalculation[LIGHT_CNT];
	int m_nArrLightPosition[LIGHT_CNT];

	tagExtAlgoLightBase()
	{
		m_eLightType = Top_Light;
		m_nImageNum = 0;
		m_nRedValue = 100;
		m_nGreenValue = 0;
		m_nBlueValue = 0;
		m_nWhiteValue = 0;

		m_nLightCnt = 0;
		memset(m_nArrRedValue, 0, sizeof(int) * LIGHT_CNT);
		memset(m_nArrGreenValue, 0, sizeof(int) * LIGHT_CNT);
		memset(m_nArrBlueValue, 0, sizeof(int) * LIGHT_CNT);
		memset(m_nArrWhiteValue, 0, sizeof(int) * LIGHT_CNT);
		memset(m_nArrCalculation, 0, sizeof(int) * LIGHT_CNT);
		memset(m_nArrLightPosition, 0, sizeof(int) * LIGHT_CNT);
	}


	void CloneLight(InspAlgo * pAlgo)
	{
		m_eLightType = pAlgo->m_eLightType;
		m_nRedValue = pAlgo->m_nRedValue;
		m_nGreenValue = pAlgo->m_nGreenValue;
		m_nBlueValue = pAlgo->m_nBlueValue;
		m_nWhiteValue = pAlgo->m_nWhiteValue;
		m_nLightCnt = pAlgo->m_nLightCnt;
		for (int i = 0; i < m_nLightCnt; i++)
		{
			m_nArrRedValue[i] = pAlgo->m_nArrRedValue[i];
			m_nArrGreenValue[i] = pAlgo->m_nArrGreenValue[i];
			m_nArrBlueValue[i] = pAlgo->m_nArrBlueValue[i];
			m_nArrWhiteValue[i] = pAlgo->m_nArrWhiteValue[i];
			m_nArrCalculation[i] = pAlgo->m_nArrCalculation[i];
			m_nArrLightPosition[i] = pAlgo->m_nArrLightPosition[i];
		}
	}
	void CloneMixLight(InspAlgoLight * pAlgo)
	{
		m_eLightType = pAlgo->m_eLightType;
		m_nImageNum = pAlgo->m_nImageNum;
		m_nRedValue = pAlgo->m_nRedValue;
		m_nGreenValue = pAlgo->m_nGreenValue;
		m_nBlueValue = pAlgo->m_nBlueValue;
		m_nWhiteValue = pAlgo->m_nWhiteValue;
		m_nLightCnt = pAlgo->m_nLightCnt;
		for (int i = 0; i < m_nLightCnt; i++)
		{
			m_nArrRedValue[i] = pAlgo->m_nArrRedValue[i];
			m_nArrGreenValue[i] = pAlgo->m_nArrGreenValue[i];
			m_nArrBlueValue[i] = pAlgo->m_nArrBlueValue[i];
			m_nArrWhiteValue[i] = pAlgo->m_nArrWhiteValue[i];
			m_nArrCalculation[i] = pAlgo->m_nArrCalculation[i];
			m_nArrLightPosition[i] = pAlgo->m_nArrLightPosition[i];
		}
	}

}ExtAlgoLightBase;


typedef struct tagExtAlgoLight
{
	int m_nWndID;
	int m_nAlgoID;
	ExtAlgoLightBase m_stLight;

	int m_nMixCnt;
	ExtAlgoLightBase m_stMixLight[2];

	tagExtAlgoLight()
	{
		m_nWndID = 0;
		m_nAlgoID = 0;
		m_nMixCnt = 0;
	}

}ExtAlgoLight;
#pragma endregion _MultiProcess

//////////////////////////////////////////////////////////////////////////
//
// Part Search parameter	
//
//////////////////////////////////////////////////////////////////////////

#pragma region _PartSerach Parameter_

typedef struct tagPartSearchParam
{
	int fovIndex;
	double fovCx;
	double fovCy;

	double cx;
	double cy;
	double width;
	double length;

	double minHeight; //mm

	ZmapData zmapData;
}PartSearchParam;

typedef struct tagPartSearchRst
{
	int arraySize;

	double* cx;
	double* cy;
	double* w;
	double* h;

}PartSearchRst;



//c++에서만 사용...(MPTI..)
typedef struct tagSearchParm
{
	int cx;
	int cy;
	int width;
	int length;

	double minHeight; //mm
	int minArea;

	ZmapData zmapData;
}SearchParm;

#pragma endregion _PartSerach Parameter_


//////////////////////////////////////////////////////////////////////////
//
// Mount inspection parameter
//
//////////////////////////////////////////////////////////////////////////


#pragma region _MOUNT PARAM_

typedef struct tagMountDecisionList
{
	BOOL existOK;
	BOOL heightOk;	
	BOOL widthOk;
	BOOL lengthOk;
	BOOL offsetXOk;
	BOOL offsetYOk;
	BOOL rotationOk;
	BOOL gradientOk_v;
	BOOL gradientOk_h;
	BOOL mountPatternOK;

}MountDecisionList;

typedef struct tagMountBlobRst
{
	int counts;
	double cogX;
	double cogY;
	double area;
	float hAvr;    //mm

	double left;			 //rect.left
	double right;			 //rect.right
	double top;			 //rect.top
	double bottom;			 //rect.bottom
	double width;           //rect.width()
	double length;          //rect.height()

	double rotationAngle;
	double gradientAngle_v; //ㅣ
	double gradientAngle_h; //ㅡ

}MountBlobRst;

typedef struct tagRstInspMount
{
	float bodyHeight;
	double width;
	double length;
	double offsetX;
	double offsetY;
	double rotationAngle;
	double gradientAngle_v; //ㅣ
	double gradientAngle_h; //ㅡ

	BOOL isInsp;
	BOOL ok;	// 1: GOOD ,  0: NG
	//int wndDefectCode;
	MountDecisionList list;

}RstInspMount;

typedef struct tagTeachParamMount
{
	//[input]
	double wndAngle;
	double bdryMargin;
	int extractRange;
	BOOL inspPatternFlag;
	wchar_t modelPath[MAX_STRLEN];
	
	BOOL noiseRemove;
	double heightLimit;

	BOOL IsUserDefCop;
	double* cxList;
	double* cyList;
	double* widthList;
	double* heightList;


	//[output]
	MountBlobRst retTeachResult;

}TeachParamMount;


typedef struct tagInspParamMount
{
	//BOOL enabled;

	//[input]
	double wndAngle;
	double bdryMargin;
	int extractRange;
	BOOL inspPatternFlag;
	wchar_t modelPath[MAX_STRLEN];

	//standard value	
	MountBlobRst teachData;
	double stdRotationAngle;   //default  0
	double stdGradientAngle;   //default  0

	//allow range
	int range_MinWidth;  //%
	int range_MaxWidth;  //%
	int range_MinLength;  //%
	int range_MaxLength;  //%
	int range_MinHeight;  //%
	int range_MaxHeight;  //%
	double range_Eccentricity;  //mm
	double range_RotationAngle;  //degree   +-
	double range_GradientAngle;  //degree   +-

    BOOL IsUserDefCop;
    double* cxList;
    double* cyList;
    double* widthList;
    double* heightList;


	//[output]
	MountBlobRst retBlobRst;
	RstInspMount retInspMountResult; //teach test only
	
}InspParamMount;

#pragma endregion _MOUNT PARAM_



#pragma region _Lead&Solder PARAM_

typedef struct tagLeadNgRectInfo
{
	BOOL isOK; //TRUE : good   FALSE : ng
	int position;
	CRect wndRoiforFOV;

	BOOL isLeadCountNg;

	CRect* leadRect;
	int leadCount;
	int leadNgCount;

	CRect* solderRect;
	int solderCount;
	int solderNgCount;

	CRect* bridgeRect;
	int bridgeCount;
	int bridgeNgCount;	
}LeadNgRectInfo;

typedef struct tagLeadSolderDecisionList
{
	//lead
	BOOL leadBridgeOk;
	BOOL leadLiftOk;
	BOOL leadCountOk;
	BOOL leadPitchOk;


	//solder
	BOOL solderFilletOk;
	BOOL solderColorOk;
}LeadSolderDecisionList;

typedef struct tagRstInspLeadSolder
{
	//common
	BOOL isInsp;
	BOOL ok;	// 1: GOOD ,  0: NG

	//lead	
	int leadFlag;
	LeadInfo leadInfo;	
	float leadAvrHeight;
	float* leadHeight;
	BOOL* leadLiftRst;
	BOOL* leadBridgeRst;

	//solder
	int solderFlag;
	int solderCnt;
	double* solderVolume; 
	int* solderColorRst;
	int* volumeRst;
	int* filetLength;
	double* solderColorRate;  //color CIE rate
	float* solderRatio;
	int* rstSolderStartPos;
	int rstSolderArea;

	//int wndDefectCode;
	LeadSolderDecisionList list;
}RstInspLeadSolder;

typedef struct tagTeachParamLeadSolder
{
	//common
	//[input]
	double wndAngle;
	BOOL leadFlag;
	BOOL solderFlag;

	int leadPosition;  //0:left, 1:right, 2:top, 3:bottom
	int solderStartPos;
	int solderEndPos;
	BOOL isChipSolder;
	BOOL isContainer;
	
	BOOL use3D; //lead 추출할때 3D 사용할지 여부
	int threshold;  //2d

	BOOL inspHeightFlag;
	BOOL inspBridgeFlag;
	BOOL inspSolderFlag;

	//CIE 검사 (소납)	// huj 2014/01/07
	BOOL inspColorFlag;
	float redFator;
	float greenFator;
	float blueFator;

	//lead
	//[output]
	LeadInfo leadInfo;
	float leadAvrHeight;
	float* leadHeight;

	//solder
	//[output]
	int solderCnt;
	double* solderVolume;
	int* filetLength;
	double* solderColorRate;  //color CIE rate
	float* solderRatio;
	int* rstSolderStartPos;
	int rstSolderArea;

	BOOL IsUse2D; 

}TeachParamLeadSolder;


typedef struct tagInspParamLeadSolder
{
	//common
	//[input]
	double wndAngle;
	BOOL leadFlag;
	BOOL solderFlag;

	int leadPosition;  //0:left, 1:right, 2:top, 3:bottom
	int solderStartPos;
	int solderEndPos;
	BOOL isChipSolder;

	BOOL inspHeightFlag;
	BOOL inspBridgeFlag;
	BOOL inspSolderFlag;
	BOOL bridgeOption_2dOnly;

	BOOL isContainer;

	
	//lead	
	//[input]
	int stdLeadCount;
	float stdLeadHeight;
	double stdLeadPitch;
	double stdLeadWidth;
	double range_MaxH;  //%
	double range_MinH;  //%

	//solder
	//[input]
	double range_maxVolume;  //%
	double range_minVolume;  //%
	int stdSolderArea;


	BOOL use3D; //lead 추출할때 3D 사용할지 여부
	int threshold;  //2d

	//CIE 검사 (소납)	// LMJ 2014/01/03
	BOOL inspColorFlag;
	double stdRate;
	float redFator;
	float greenFator;
	float blueFator;

	//huj 2014/02/25
	BOOL isInspLeadPitch;
	
	BOOL IsUse2D; 

	RstInspLeadSolder retInspResult;
}InspParamLeadSolder;

#pragma endregion _Lead&Solder PARAM_


#pragma region _Inspection result_

// Algorithm 검사 결과 임시저장소
typedef struct tagInspAlgoTempResult
{
	// Lead Tip
	int * m_ptrnLeadTipPosResult;
	// Lead Lift
	bool m_bParentWnd;
	int m_nLeadLiftGroupCnt;
	int * m_ptrnLeadWndID;
	double * m_ptrdLeadLiftResult;

	tagInspAlgoTempResult(int nCnt)
	{
		m_bParentWnd = false;
		m_ptrnLeadWndID = NULL;
		m_ptrnLeadTipPosResult = NULL;
		m_nLeadLiftGroupCnt = 0;
		m_ptrdLeadLiftResult = NULL;

		// Lead Tip
		//m_ptrnLeadTipPosResult = new int[nCnt];
		m_ptrnLeadTipPosResult = g_pMManager->pem_new<int>(true, nCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_ptrnLeadTipPosResult, 0, sizeof(int)*nCnt);
		// Lead Lift
		//m_ptrnLeadWndID = new int[nCnt];
		m_ptrnLeadWndID = g_pMManager->pem_new<int>(true, nCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_ptrnLeadWndID, 0, sizeof(int)*nCnt);
		//m_ptrdLeadLiftResult = new double[nCnt];
		m_ptrdLeadLiftResult = g_pMManager->pem_new<double>(true, nCnt, (PCHAR)__FUNCTION__, __LINE__);
		memset(m_ptrdLeadLiftResult, 0, sizeof(double)*nCnt);
	}

	void Destroy()
	{
		// Lead Tip
		if (m_ptrnLeadTipPosResult)
		{
			//delete [] m_ptrnLeadTipPosResult;
			g_pMManager->pem_delete(m_ptrnLeadTipPosResult, true);
			m_ptrnLeadTipPosResult = NULL;
		}
		// Lead Lift
		if (m_ptrnLeadWndID)
		{
			//delete [] m_ptrnLeadWndID;
			g_pMManager->pem_delete(m_ptrnLeadWndID, true);
			m_ptrnLeadWndID = NULL;
		}
		if (m_ptrdLeadLiftResult)
		{
			//delete [] m_ptrdLeadLiftResult;
			g_pMManager->pem_delete(m_ptrdLeadLiftResult, true);
			m_ptrdLeadLiftResult = NULL;
		}
	}
}InspAlgoTempResult;

// Algorithm 검사 결과
typedef struct tagInspAlgoResult
{
	BOOL m_bIsInsp;
	BOOL m_bIsRequired;
	BOOL m_bOk;
	InspAlgoType m_nAlgoType;
	void * m_vRstInspAlgo;			// Algorithm 별 Result value

	BOOL m_bUsingManualDefectCode;
	int m_nManualDefectCode;
	int m_nManualSubDefectCode;
	int m_nDefectCode;
	tagInspAlgoResult()
	{
		m_bIsInsp = FALSE;
		m_bIsRequired = FALSE;
		m_bOk = TRUE;
		m_vRstInspAlgo = NULL;
		m_bUsingManualDefectCode = FALSE;
		m_nManualDefectCode = 0;
		m_nManualSubDefectCode = 0;
		m_nDefectCode = 0;
	}
}InspAlgoResult;

// Window 검사 결과
typedef struct tagInspWndResult
{
	BOOL m_bIsInsp;
	BOOL m_bOk;
	int m_nDefectCode;
	int m_nDefectCodeSub;
	int m_nAlgorithmCnt;			// Algorithm 개수
	InspAlgoResult *m_vArrRstInspAlgo;			// Algorithm Result
	unsigned long long m_nDefectAlgo;
	int m_nDefectAlgoID;

	tagInspWndResult()
	{
		m_bIsInsp = FALSE;
		m_bOk = TRUE;
		m_nDefectCode = 0;
		m_nAlgorithmCnt = 0;
		m_vArrRstInspAlgo = NULL;
		m_nDefectAlgo = 0;
		m_nDefectAlgoID = 0;
	}

	void Destroy()
	{
		if (m_vArrRstInspAlgo)
		{
			for (int i = 0; i < m_nAlgorithmCnt; ++i)
			{
				if (!m_vArrRstInspAlgo[i].m_vRstInspAlgo)
					continue;

				//delete m_vArrRstInspAlgo[i].m_vRstInspAlgo;
				g_pMManager->pem_delete(m_vArrRstInspAlgo[i].m_vRstInspAlgo, false);
				m_vArrRstInspAlgo[i].m_vRstInspAlgo = NULL;
			}
			//delete [] m_vArrRstInspAlgo;
			g_pMManager->pem_delete(m_vArrRstInspAlgo, true);
			m_vArrRstInspAlgo = NULL;

			m_nAlgorithmCnt = 0;
		}
	}
}InspWndResult;

// Part 전체 검사 결과
typedef struct tagInspectionResult
{
	int nDefectType;
	BOOL isInspMount;
	int mountArraySize;
	InspParamTemp* mountParamTemp;
	InspWndResult* mountResult;	// ++

	BOOL isInspAlign;
	int alignArraySize;
	InspParamTemp* alignParamTemp;
	InspWndResult* alignResult;

	BOOL isInspOcr;
	int ocrArraySize;
	InspParamTemp* ocrParamTemp;
	InspWndResult* ocrResult;

	BOOL isInspLeadSolder;
	int leadSolderArraySize;
	InspParamTemp* leadSolderParamTemp;
	InspWndResult* leadSolderResult;

	BOOL isInspSolder;
	int solderArraySize;
	InspParamTemp* solderParamTemp;
	InspWndResult* solderResult;

	BOOL isInspTab;
	int tabArraySize;
	InspParamTemp* tabParamTemp;
	InspWndResult* tabResult;

	BOOL isInspS_Ball;
	int S_BallArraySize;
	InspParamTemp* S_BallParamTemp;
	InspWndResult* S_BallResult;

	BOOL isInspPad;
	int PadArraySize;
	InspParamTemp* PadParamTemp;
	InspWndResult* PadResult;
	// User 추가

	//////////////////////////////////////////////////////////////////////////bga
	BOOL isInspBGA;
	int BGAArraySize;
	InspParamTemp* BGAParamTemp;
	InspWndResult* BGAResult;

	// ybji 200721 기존 BGA Window 제거
	/*int bgaWndCnt;
	InspParamTemp* bgaWndParamTemp;
	RstInspBGA_Wnd* bgaWndRst;
	int bgaGroupCnt;
	InspParamBGA_Part bgaGrpParam;
	RstInspBGA_Grp* bgaGroupRst;*/
	int m_nDefectWnd;   // SHW 160224
	unsigned long long m_nDefectAlgo;    // SHW 160224
	//////////////////////////////////////////////////////////////////////////
	int m_nPartID;

	double m_dInspTime;

	tagInspectionResult()
	{
		mountArraySize = 0;
		mountParamTemp = NULL;
		mountResult = NULL;	// ++

		alignArraySize = 0;
		alignParamTemp = NULL;
		alignResult = NULL;

		ocrArraySize = 0;
		ocrParamTemp = NULL;
		ocrResult = NULL;

		leadSolderArraySize = 0;
		leadSolderParamTemp = NULL;
		leadSolderResult = NULL;

		solderArraySize = 0;
		solderParamTemp = NULL;
		solderResult = NULL;

		BGAArraySize = 0;
		BGAParamTemp = NULL;
		BGAResult = NULL;
	/*	bgaWndCnt = 0;
		bgaWndParamTemp = NULL;
		bgaWndRst = NULL;
		bgaGroupCnt = 0;
		bgaGroupRst = NULL;*/

		m_nDefectWnd = 0;
		m_nDefectAlgo = 0;
		
		m_nPartID = 0;
		m_dInspTime = 0.0;
	}

	~tagInspectionResult()
	{
		Destroy();
	}

	void Destroy()
	{
		if(mountResult)
		{
			for(int i=0; i < mountArraySize; ++i)
			{
				mountResult[i].Destroy();
			}
			g_pMManager->pem_delete(mountResult, true);
			mountResult = NULL;
			mountArraySize = 0;
 		}

		if(alignResult)
		{
			for(int i=0; i < alignArraySize; ++i)
			{
				alignResult[i].Destroy();
			}
			g_pMManager->pem_delete(alignResult, true);
			alignResult = NULL;
			alignArraySize = 0;
		}

		if(ocrResult)
		{
			for(int i=0; i < ocrArraySize; ++i)
			{
				ocrResult[i].Destroy();
			}
			g_pMManager->pem_delete(ocrResult, true);
			ocrResult = NULL;
			ocrArraySize = 0;
		}

		if(leadSolderResult)
		{
			for(int i=0; i < leadSolderArraySize; ++i)
			{
				leadSolderResult[i].Destroy();
			}
			g_pMManager->pem_delete(leadSolderResult, true);
			leadSolderResult = NULL;
			leadSolderArraySize = 0;
		}

		if(solderResult)
		{
			for(int i=0; i < solderArraySize; ++i)
			{
				solderResult[i].Destroy();
			}
			g_pMManager->pem_delete(solderResult, true);
			solderResult = NULL;
			solderArraySize = 0;
		}

		if(tabResult)
		{
			for(int i=0; i < tabArraySize; ++i)
			{
				tabResult[i].Destroy();
			}
			g_pMManager->pem_delete(tabResult, true);
			tabResult = NULL;
			tabArraySize = 0;
		}

		if(S_BallResult)
		{
			for(int i=0; i < S_BallArraySize; ++i)
			{
				S_BallResult[i].Destroy();
			}
			g_pMManager->pem_delete(S_BallResult, true);
			S_BallResult = NULL;
			S_BallArraySize = 0;
		}

		if(PadResult)
		{
			for(int i=0; i < PadArraySize; ++i)
			{
				PadResult[i].Destroy();
			}
			g_pMManager->pem_delete(PadResult, true);
			PadResult = NULL;
			PadArraySize = 0;
		}

		if (BGAResult)
		{
			for (int i = 0; i < BGAArraySize; ++i)
			{
				BGAResult[i].Destroy();
			}
			g_pMManager->pem_delete(BGAResult, true);
			BGAResult = NULL;
			BGAArraySize = 0;
		}
	}
	void DestroyParam()
	{
		int n = 0;
		if (mountParamTemp != NULL)
		{
			for (n = 0; n < mountArraySize; n++)
				mountParamTemp[n].Destroy();
			g_pMManager->pem_delete(mountParamTemp, true);
			mountParamTemp = NULL;
		}

		if (alignParamTemp != NULL)
		{
			for (n = 0; n < alignArraySize; n++)
				alignParamTemp[n].Destroy();
			g_pMManager->pem_delete(alignParamTemp, true);
			alignParamTemp = NULL;
		}

		if (ocrParamTemp != NULL)
		{
			for (n = 0; n < ocrArraySize; n++)
				ocrParamTemp[n].Destroy();
			g_pMManager->pem_delete(ocrParamTemp, true);
			ocrParamTemp = NULL;
		}

		if (leadSolderParamTemp != NULL)
		{
			for (n = 0; n < leadSolderArraySize; n++)
				leadSolderParamTemp[n].Destroy();
			g_pMManager->pem_delete(leadSolderParamTemp, true);
			leadSolderParamTemp = NULL;
		}

		if (solderParamTemp != NULL)
		{
			for (n = 0; n < solderArraySize; n++)
				solderParamTemp[n].Destroy();
			g_pMManager->pem_delete(solderParamTemp, true);
			solderParamTemp = NULL;
		}

		if (tabParamTemp != NULL)
		{
			for (n = 0; n < tabArraySize; n++)
				tabParamTemp[n].Destroy();
			g_pMManager->pem_delete(tabParamTemp, true);
			tabParamTemp = NULL;
		}

		if (S_BallParamTemp != NULL)
		{
			for (n = 0; n < S_BallArraySize; n++)
				S_BallParamTemp[n].Destroy();
			g_pMManager->pem_delete(S_BallParamTemp, true);
			S_BallParamTemp = NULL;
		}

		if (PadParamTemp != NULL)
		{
			for (n = 0; n < PadArraySize; n++)
				PadParamTemp[n].Destroy();
			g_pMManager->pem_delete(PadParamTemp, true);
			PadParamTemp = NULL;
		}

		if (BGAParamTemp != NULL)
		{
			for (n = 0; n < BGAArraySize; n++)
				BGAParamTemp[n].Destroy();
			g_pMManager->pem_delete(BGAParamTemp, true);
			BGAParamTemp = NULL;
		}
	}
	void SetInsp(int wndType)
	{
		if (wndType == eINSP_MOUNT)
		{
			isInspMount = TRUE;
		}
		else if (wndType == eINSP_ALIGN)
		{
			isInspAlign = TRUE;
		}
		else if (wndType == eINSP_OCR)
		{
			isInspOcr = TRUE;
		}
		else if (wndType == eINSP_LEADSOLDER)
		{
			isInspLeadSolder = TRUE;
		}
		else if (wndType == eINSP_SOLDER)
		{
			isInspSolder = TRUE;
		}
		else if (wndType == eINSP_TAB)
		{
			isInspTab = TRUE;
		}
		else if (wndType == eINSP_S_BALL)
		{
			isInspS_Ball = TRUE;
		}
		else if (wndType == eINSP_PAD)
		{
			isInspPad = TRUE;
		}
		else if (wndType == eINSP_BGA)
		{
			isInspBGA = TRUE;
		}
	}
	int GetWndArraySize(int wndType, int nSetSize = -1, bool bTypeNG = false)
	{
		if (wndType == eINSP_MOUNT)
		{
			if (bTypeNG)
				return -1;
			else if (nSetSize > -1)
				mountArraySize = nSetSize;
			return mountArraySize;
		}
		else if (wndType == eINSP_ALIGN)
		{
			if (bTypeNG)
				return TypeAlignWrong;
			else if (nSetSize > -1)
				alignArraySize = nSetSize;
			return alignArraySize;
		}
		else if (wndType == eINSP_OCR)
		{
			if (bTypeNG)
				return TypeOcrWrong;
			else if (nSetSize > -1)
				ocrArraySize = nSetSize;
			return ocrArraySize;
		}
		else if (wndType == eINSP_LEADSOLDER)
		{
			if (bTypeNG)
				return -1;
			else if (nSetSize > -1)
				leadSolderArraySize = nSetSize;
			return leadSolderArraySize;
		}
		else if (wndType == eINSP_SOLDER)
		{
			if (bTypeNG)
				return TypeSolderFilet;
			else if (nSetSize > -1)
				solderArraySize = nSetSize;
			return solderArraySize;
		}
		else if (wndType == eINSP_TAB)
		{
			if (bTypeNG)
				return TypeTab;
			else if (nSetSize > -1)
				tabArraySize = nSetSize;
			return tabArraySize;
		}
		else if (wndType == eINSP_S_BALL)
		{
			if (bTypeNG)
				return TypeS_Ball;
			else if (nSetSize > -1)
				S_BallArraySize = nSetSize;
			return S_BallArraySize;
		}
		else if (wndType == eINSP_PAD)
		{
			if (bTypeNG)
				return TypeNoSolder;
			else if (nSetSize > -1)
				PadArraySize = nSetSize;
			return PadArraySize;
		}
		else if (wndType == eINSP_BGA)
		{
			if (bTypeNG)
				return TypeNoSolder;
			else if (nSetSize > -1)
				BGAArraySize = nSetSize;
			return BGAArraySize;
		}
		return 0;
	}
	InspParamTemp* GetWndParam(int wndType, int nSet = -1)
	{
		InspParamTemp** param = nullptr;

		if (wndType == eINSP_MOUNT)
			param = &mountParamTemp;
		else if (wndType == eINSP_ALIGN)
			param = &alignParamTemp;
		else if (wndType == eINSP_OCR)
			param = &ocrParamTemp;
		else if (wndType == eINSP_LEADSOLDER)
			param = &leadSolderParamTemp;
		else if (wndType == eINSP_SOLDER)
			param = &solderParamTemp;
		else if (wndType == eINSP_TAB)
			param = &tabParamTemp;
		else if (wndType == eINSP_S_BALL)
			param = &S_BallParamTemp;
		else if (wndType == eINSP_PAD)
			param = &PadParamTemp;
		else if (wndType == eINSP_BGA)
			param = &BGAParamTemp;
		else
			return nullptr;

		if (nSet > 0)
		{
			*param = g_pMManager->pem_new<InspParamTemp>(true, nSet, (PCHAR)__FUNCTION__, __LINE__);
			memset(*param, 0, (sizeof(InspParamTemp) * nSet));
		}

		if (*param != nullptr)
			return *param;
		else
			return nullptr;
	}
	InspWndResult* GetWndResult(int wndType, int nSet = -1)
	{
		InspWndResult** rsult = nullptr;

		if (wndType == eINSP_MOUNT)
			rsult = &mountResult;
		else if (wndType == eINSP_ALIGN)
			rsult = &alignResult;
		else if (wndType == eINSP_OCR)
			rsult = &ocrResult;
		else if (wndType == eINSP_LEADSOLDER)
			rsult = &leadSolderResult;
		else if (wndType == eINSP_SOLDER)
			rsult = &solderResult;
		else if (wndType == eINSP_TAB)
			rsult = &tabResult;
		else if (wndType == eINSP_S_BALL)
			rsult = &S_BallResult;
		else if (wndType == eINSP_PAD)
			rsult = &PadResult;
		else if (wndType == eINSP_BGA)
			rsult = &BGAResult;
		else
			return nullptr;

		if (nSet > 0)
		{
			*rsult = g_pMManager->pem_new<InspWndResult>(true, nSet, (PCHAR)__FUNCTION__, __LINE__);
			memset(*rsult, 0, (sizeof(InspWndResult) * nSet));

			InspParamTemp * wndParam = GetWndParam(wndType);
			if (wndParam != nullptr)
			{
				for (int n = 0; n < nSet; n++)
				{
					InspWndResult *pInspWndResult = &(*rsult)[n];
					int nCntAlgorithm = wndParam[n].nAlgorithmCnt;
					if (pInspWndResult && nCntAlgorithm > 0)
					{
						(pInspWndResult)->m_nAlgorithmCnt = nCntAlgorithm;	// 할 것, 알고리즘별 결과 m_vArrRstInspAlgo 받으면 주석 풀 것
						(pInspWndResult)->m_vArrRstInspAlgo = g_pMManager->pem_new<InspAlgoResult>(true, nCntAlgorithm, (PCHAR)__FUNCTION__, __LINE__);
						memset((pInspWndResult)->m_vArrRstInspAlgo, 0, (sizeof(InspAlgoResult) * nCntAlgorithm));
					}
				}
			}
		}

		if (*rsult != nullptr)
			return *rsult;
		else
			return nullptr;
	}
}InspectionResult;

#pragma endregion _Inspection result_



#pragma region _Barcode Parameter_

enum Barcode_Type_Id
{
	eB_CODE39 = 0,
	eB_CODE93, 
	eB_CODE128, 
	eB_CODABAR,
	eB_BC412,
	eB_EAN8,
	eB_EAN13, 
	eB_INTERLEAVED25, 
	eB_PHARMACODE,
	eB_PLANET, 
	eB_POSTNET,
	eB_RSSCODE,
	eB_UPC_A, 
	eB_UPC_E, 
	eB_DATAMATRIX,
	eB_MAXICODE,
	eB_PDF417,
	eB_MICROPDF417,
	eB_QRCODE,
	eB_COMPOSITECODE,
	eB_GS1_128,
	eB_BARCODE_TYPE_CNT
};


typedef struct tagInspBarcodeParam
{	
	int devType;
	//////////////////////////////////////////////////////////////////////////
	
	double fovCx;
	double fovCy;	
	//////////////////////////////////////////////////////////////////////////

	double cx;
	double cy;
	double width;
	double length;
	double dAngle;

	int imgSizeX;
	int imgSizeY;
// 	UCHAR* img;		// LMJ 2018/05/02 : 불필요해서 삭제

	int barcodeType;
	int m_InspBarcodeType;
	int foreColor;

	int fovIndex;
	int lightType;
	InspAlgo MixLight;

	int UseFlip;
	int XFlip;
	int YFlip;

	int checkSum;

	int nCount;

// 	wchar_t stdString[MAX_STRLEN];		// LMJ 2018/05/02 : 불필요해서 삭제
	int nThresholdType;
	int nUseAngle;
	int nUseModuleProgram;
	int nStdAngle;
	BOOL bIsOrientationOK;
	wchar_t sModuleBarcodeID[MAX_STRLEN];
	
}InspBarcodeParam;

#pragma endregion _Barcode Parameter_



// LMJ 2013/12/17
#pragma region _BadMark Parameter_

enum m_eMachineOptionDafault_Data
{
	m_eMachineOptionDafault_Data_BAD_NOT_Min = 0x01,
	m_eMachineOptionDafault_Data_BAD_NOT_Max = 0x02,
	m_eMachineOptionDafault_Data_NEWJOB_AutoArray = 0x04,
};
typedef struct tagInspBadMarkParam
{
	int devType;

	double fovCx;
	double fovCy;	

	int fovSizeX;
	int fovSizeY;
	UCHAR* fovImg;
	int fovIndex;

	int lightType;
	InspAlgo MixLight;
	double cx;
	double cy;
	double sizeX;
	double sizeY;
	int grayLevel;		// Grary 평균 조건값
	int grayRange;		// +- 허용값
	int grayMaxRange;
	int grayOPT;

// 	int * retGrayLevel;		// 결과 Gray 평균값
// 	BOOL * retIsBadMark;	//
}InspBadMarkParam;

#pragma endregion _BadMark Parameter_



#pragma region Parameter Info

typedef struct ParamInfo
{	
	int* pInspItemCnts;
	int inspItemCntsArrSize;
	int** pInspItemID;
	int inspItemIDCntsArrSize1;
	int inspItemIDCntsArrSize2;

	int* pGroupIndexCnts; //검사 항목별 그룹 종류 개수.
	int** pGroupIndex; //검사 항목별 가지고 있는 그룹 인덱스

	int* pGroupID;
	int* pGroupWndCnts; // 그룹에 속해있는 윈도우 개수.
	int** pGroupWndID;  //그룹 인덱스 별 윈도우 번호

}ParamInfo;

typedef struct tagForeign3DUserExcept
{	
	CRect *rtForeignFilledRect;
	int nForeignFilledCnt;

	double dUserSolderMin;
	CRect *rtSolderFilledRect;
	int nSolderFilledCnt;

}Foreign3DUserExcept;

typedef struct tagForeign3DExceptData
{	
	int nForeignExceptCnt;
	int nSolderExceptCnt;
	int dHeightLimitSolder;

	int nForeignPolygonExceptCnt;
	int nSolderPolygonExceptCnt;
	int nCopperPolygonExceptCnt;
	int nBubblePolygonExceptCnt;		//NYJ 2018/07/20
}Foreign3DExceptData;

typedef struct tagForeignPolygonPt
{
	POINTF m_ptArrForeignPolygon[MAX_INSP_AREA_COUNT];

	tagForeignPolygonPt()
	{
		POINTF ptInit;
		ptInit.x = 0; ptInit.y = 0;

		for(int i = 0; i < MAX_INSP_AREA_COUNT; i++)
			m_ptArrForeignPolygon[i] = ptInit;
	}

}ForeignPolygonPt;

typedef struct tagForeignPolygonArea
{
	int m_nForeignPolygonCnt;

	CRect* rtPolyBound;
	ForeignPolygonPt* m_stArrForeignPolygon;	

}ForeignPolygonArea;

typedef struct tagAlgoColorOpt
{
	int m_nCIEWidth;
	int m_nCIEHeight;
	double dAngle;

	int m_nInspType;
	InspRoiImgBuf m_sFovImg;
	InspRoiImgBuf m_sImgBuf;

	RECT  m_rcImageRect;
	BOOL m_bUseRangeBar;
	tagAlgoColorOpt()
	{
		m_nCIEWidth = 0;
		m_nCIEHeight = 0;
		dAngle = 0;
		m_bUseRangeBar = FALSE;
		m_nInspType = -1;
	}
	~tagAlgoColorOpt()
	{
		Destroy();
	}

	void Destroy()
	{
		Delete_1DArray(&m_sImgBuf.imgTop_R);
		Delete_1DArray(&m_sImgBuf.imgTop_G);
		Delete_1DArray(&m_sImgBuf.imgTop_B);
		Delete_1DArray(&m_sImgBuf.imgTop_W);
		Delete_1DArray(&m_sImgBuf.imgMiddle_R);
		Delete_1DArray(&m_sImgBuf.imgMiddle_B);
		Delete_1DArray(&m_sImgBuf.imgBottom_R);
		Delete_1DArray(&m_sImgBuf.imgBottom_B);
	}
}AlgoColorOpt;

typedef struct tagRstAlgoColorOpt
{
	double m_dRate;
	BOOL m_bUseRGB[COLORALGO_POLYGON_CNTS];
	byte m_byMax[COLORALGO_POLYGON_CNTS];
	tagRstAlgoColorOpt()
	{
		m_dRate = 0;
		for (int n = 0; n < COLORALGO_POLYGON_CNTS; n++)
		{
			m_bUseRGB[n] = FALSE;
			m_byMax[n] = 0;
		}
	}
}RstAlgoColorOpt;

// SHKang 2017/03/13 : 저장을 위한 옵션 구조체
typedef struct tagSaveImgParamData
{	
	UINT32 unNGType;					// NG types

	int m_nPartID;
	int m_nOPT;
	int nSave_2d;						// Is save 2d image
	int nMSPtImg;                       // Is Must Save Part Image
	int nSave_3d;						// Is save 3d image
	int nSave_3DRaw;					// Is save 3d Raw image
	int nSave_put;						// Is save put image
	int nSave_AOILink;					// Is save AOI Linkage image (1 : 3d, 2 : jpg)
	int nSave_NextMC;                 // Is save Next machine image (1 : 3d, 2 : jpg)
	int m_nSave_MES;					// Is save MES 2d image
	int m_nSave_APC;					// Is save APC 2d image
	int m_nSave_Color;					// Is save Color Map2 image
	int m_nSave_3DRaw;					// Is Save 3D Raw Image
	int m_nSave_ImgType;				// Image Save TYpe
	int nSaveAuto3DRaw;					// P3dAutoSave
	UINT32 unSaveOption;				// Save option
	int m_n2DSaveQuality;             // Image Save quality
	int exportSaveOption;
	int m_nSave_Hanwha;
	int m_nColorCheck;
	int m_nMachineCode;					//is save LFI machine Side (-1: default, 9011 : TOP, 9021 : Bottom)
	int m_nLinkageCheck;				//Image Save Linkage
	int m_nLinkageImgFormat;			//IS SAVE AOI Linkage Image ( 1: jpg , 2 : bmp)

	wchar_t arrNGCaption[MAX_STRLEN];	// NG caption
	wchar_t arrJpgFileName[MAX_STRLEN];		// path 2d(jpg) image
	wchar_t arr3dFileName[MAX_STRLEN];		// path 3d(ptt) image
	wchar_t arrPutFileName[MAX_STRLEN];		// path UV(put) image
	wchar_t arrAOILinkFileName[MAX_STRLEN]; // path 3d(ptt) or 2d(jpg) image for AOI Linkage
	wchar_t arrNextMCFileName[MAX_STRLEN]; // path 3d(ptt) or 2d(jpg) image for Next Machine
	wchar_t arrMesFileName[MAX_STRLEN];		// path MES image
	wchar_t arrAPCFileName[MAX_STRLEN];		// path APC image
	wchar_t arrColorMap2FileName[MAX_STRLEN];		// Color Map2 image
	wchar_t arr3dRawFileName[MAX_STRLEN];		// path 3D Raw Image
	wchar_t arr3dLeadOKFileName[MAX_STRLEN]; // path 3D(ptt) leadOK part image
	wchar_t arrHanwhaFileName[MAX_STRLEN]; // path Hanwha 2D(jpg) Image
	wchar_t arrJpgFilePath[MAX_STRLEN];		// path 2d(jpg) image
	wchar_t arr3dFilePath[MAX_STRLEN];		// path 3d(ptt) image
	wchar_t arr3dFilePathTemp[MAX_STRLEN];
	wchar_t arrPutFilePath[MAX_STRLEN];		// path UV(put) image
	wchar_t arrAOILinkFilePath[MAX_STRLEN]; // path 3d(ptt) or 2d(jpg) image for AOI Linkage
	wchar_t arrNextMCFilePath[MAX_STRLEN]; // path 3d(ptt) or 2d(jpg) image for AOI Linkage
	wchar_t arrMesFilePath[MAX_STRLEN];		// path MES image
	wchar_t arrAPCFilePath[MAX_STRLEN];		// path APC image
	wchar_t arrColorMap2FilePath[MAX_STRLEN];		// Color Map2 image
	wchar_t arr3dRawFilePath[MAX_STRLEN];		// path 3D Raw Image
	wchar_t arr3dLeadOKFilePath[MAX_STRLEN]; // path 3D leadOK part image
	wchar_t arrHanwhaFilePath[MAX_STRLEN]; // path Hanwha 2D(jpg) Image 
	wchar_t arrSaveAuto3DRawFilePath[MAX_STRLEN]; //
	int pArrSideImg[PST_SIDE_NUM];				// PST file save option 
	wchar_t arrJobPath[MAX_STRLEN];				// path master job
	wchar_t arrLinkageFileName[MAX_STRLEN]; // path Linkage 2D(jpg) Image
	wchar_t arrLinkageFilePath[MAX_STRLEN]; // path Hanwha 2D(jpg) Image 
	int ColorMode;							//haesungDS defect save mode 0:Color, 1: AlgoLight(gray)

	tagSaveImgParamData()
	{
		unNGType = 0;

		m_nPartID = 0;
		m_nOPT = 0;
		nSave_2d = FALSE;
		nMSPtImg = FALSE;
		nSave_3d = FALSE;
		nSave_3DRaw = FALSE;
		nSave_put = FALSE;
		nSave_AOILink = 0;
		nSave_NextMC = 0;
		m_nSave_3DRaw = 0;
		unSaveOption = 0x00000000;
		m_n2DSaveQuality = 75;
		exportSaveOption = 0;
		m_nMachineCode = -1;

		::wmemset(arrNGCaption, 0, MAX_STRLEN);
		::wmemset(arrJpgFileName, 0, MAX_STRLEN);
		::wmemset(arr3dFileName, 0, MAX_STRLEN);
		::wmemset(arrPutFileName, 0, MAX_STRLEN);
		::wmemset(arrAOILinkFileName, 0, MAX_STRLEN);
		::wmemset(arrNextMCFileName, 0, MAX_STRLEN);
		::wmemset(arrJpgFilePath, 0, MAX_STRLEN);
		::wmemset(arr3dFilePath, 0, MAX_STRLEN);
		::wmemset(arr3dFilePathTemp, 0, MAX_STRLEN);
		::wmemset(arrPutFilePath, 0, MAX_STRLEN);
		::wmemset(arrAOILinkFilePath, 0, MAX_STRLEN);
		::wmemset(arrNextMCFilePath, 0, MAX_STRLEN);
		::wmemset(arr3dLeadOKFileName, 0, MAX_STRLEN);
		::wmemset(arr3dLeadOKFilePath, 0, MAX_STRLEN);
		memset(pArrSideImg, 0, sizeof(int) * PST_SIDE_NUM);
		::wmemset(arrJobPath, 0, MAX_STRLEN);
		ColorMode = 0;
	}
	bool UseData(int nType)
	{
		if ((m_nOPT & nType) == nType)
			return true;
		return false;
	}
	void Copy(tagSaveImgParamData sData)
	{
		unNGType = sData.unNGType;
		m_nPartID = sData.m_nPartID;
		m_nOPT = sData.m_nOPT;
		nSave_2d = sData.nSave_2d;
		nMSPtImg = sData.nMSPtImg;
		nSave_3d = sData.nSave_3d;
		nSave_3DRaw = sData.nSave_3DRaw;
		nSave_put = sData.nSave_put;
		nSave_AOILink = sData.nSave_AOILink;
		nSave_NextMC = sData.nSave_NextMC;
		m_nSave_MES = sData.m_nSave_MES;
		m_nSave_APC = sData.m_nSave_APC;
		m_nSave_Color = sData.m_nSave_Color;
		m_nSave_3DRaw = sData.m_nSave_3DRaw;
		m_nSave_ImgType = sData.m_nSave_ImgType;
		unSaveOption = sData.unSaveOption;
		m_n2DSaveQuality = sData.m_n2DSaveQuality;
		exportSaveOption = sData.exportSaveOption;
		m_nSave_Hanwha = sData.m_nSave_Hanwha;
		m_nColorCheck = sData.m_nColorCheck;
		m_nMachineCode = sData.m_nMachineCode;

		memcpy(arrNGCaption, sData.arrNGCaption, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrJpgFileName, sData.arrJpgFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arr3dFileName, sData.arr3dFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrPutFileName, sData.arrPutFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrAOILinkFileName, sData.arrAOILinkFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrNextMCFileName, sData.arrNextMCFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrMesFileName, sData.arrMesFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrAPCFileName, sData.arrAPCFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrColorMap2FileName, sData.arrColorMap2FileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arr3dRawFileName, sData.arr3dRawFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arr3dLeadOKFileName, sData.arr3dLeadOKFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrHanwhaFileName, sData.arrHanwhaFileName, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrJpgFilePath, sData.arrJpgFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arr3dFilePath, sData.arr3dFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arr3dFilePathTemp, sData.arr3dFilePathTemp, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrPutFilePath, sData.arrPutFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrAOILinkFilePath, sData.arrAOILinkFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrNextMCFilePath, sData.arrNextMCFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrMesFilePath, sData.arrMesFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrAPCFilePath, sData.arrAPCFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrColorMap2FilePath, sData.arrColorMap2FilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arr3dRawFilePath, sData.arr3dRawFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arr3dLeadOKFilePath, sData.arr3dLeadOKFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrHanwhaFilePath, sData.arrHanwhaFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(arrLinkageFilePath, sData.arrLinkageFilePath, sizeof(wchar_t) * MAX_STRLEN);
		memcpy(pArrSideImg, sData.pArrSideImg, sizeof(int) * PST_SIDE_NUM);
		memcpy(arrJobPath, sData.arrJobPath, sizeof(wchar_t) * MAX_STRLEN);
		ColorMode = sData.ColorMode;
	}
	bool SaveData()
	{
		if (nSave_3d == 0 && nSave_2d == 0 && nSave_put == 0 && nMSPtImg == 0 && m_nSave_MES == 0 &&  m_nSave_APC == 0)
			return false;
		return true;
	}
} SaveImgParamData;

typedef struct tagPOCRAISaveInfo
{
	BOOL m_bOKSave;
	BOOL m_bNGSave;
	BOOL m_bAIInputImgSave;
	int m_nAIOKSaveCount;
	int m_nAIMaxSaveCount;
}POCRAISaveInfo;
typedef struct tagBlobSaveInfo
{
	BOOL m_bBlobNGImgSave;
}BlobSaveInfo;

// InspAlgo Param
typedef struct tagInspAlgoParam
{
	//
	int m_nAlgo;
	int m_nWndIndex;
	int m_nAlignCnt;
	int m_nCurAlignID;
	int m_nStartX;
	int m_nStartY;
	int m_nWndDir;
	int m_nSelectArea;
	int m_nLeadTipPos;
	int m_nInspType;
	int m_nOffX_pix;
	int m_nOffY_pix;
	int m_nInspCameraType;
	int m_nLeadTipPosIndex;
	int m_nTypeSelectBlob;

	// esjb++ 250403.[36] ~InspParamDef.h~
	int m_nTypeSelectTarget;
	// ++sjb 250403.[36]

	int m_nBlobMinArea;
	int m_nInspectionMode;
	//
	double m_dx;
	double m_dy;
	double m_dWndW;
	double m_dWndH;
	double m_dInspW;
	double m_dInspH;
	//
	byte m_bViewCJ;
	byte m_byWndInspType;
	//
	BOOL m_bInspection;
	BOOL m_bTeach;
	BOOL m_bUSeLeadAlign;
	BOOL m_bTeachingFunc;
	BOOL m_bBin;
	BOOL m_bUseBlobOption;
	BOOL m_bFillHole;
	//
	POINTF m_poCenter;
	POINTF m_ptWndCenter;
	RECT m_rcWND;
	//
	int *m_pnRet;
	int *m_pnResult;
	int *m_nReviewMin;
	int *m_nReviewMax;
	double *m_pdResultHeight;
	double *m_dUSENGOCR;
	double *m_dOCRNGScore;
	double *m_dOCROKScore;
	float *m_pfHeightMean;
	UCHAR* m_ucArrDstImgAC;
	//
	CRect* m_rcBlobBody;
	CAtlArray<CRect>* m_rcArrTab;
	CAtlArray<CRect>* m_rcArrTabBridge;
	//
	WndInfo* m_sWndInfo;
	WndInfo* m_sAlignWndInfo;
	AlignResult* m_sAlignRes;
	AlignResult* m_arrAlignRes;
	InspAlgoTempResult* m_sInspAlgoTempResult;
	AlgoCoordinate* m_coordinateAlgo;
	InspWndResult* m_WndResult;
	WndAlgoImg* m_sPartAlgoImg;
	AlgoColorOpt* m_ptrAlgoColorOpt;
	InspRoiImgBuf* m_sInspImageData;
	FootColor* m_fcBackground;
	FootColor* m_fcFoot;
	FootColor* m_fcWing;
	FootColor* m_fcWedge;
	FootColor* m_fcFindDBC;
	vector<InspRstPolyAlgo>* m_pvInspRstPoly;
	int m_nWndID;
	//
	BOOL m_bPadPattern;
	int m_nViewMode;
	PADPatternPoly m_sPadPoly;  // Pad Polygon
	int m_nOverlapPadPolyCnt;    // Overlap Pad Polygon 개수
	PADPatternPoly* m_sArrOverlapPadPoly; // Overlap Pad Polygon
	int m_nPOLY_W;
	int m_nPOLY_H;
	UCHAR* m_ucPOLY;
	void* m_sRstAlgo_P;
	int m_nTYPE;
	int m_nBinaryMode; //0:binary, 1:Gray
	tagInspAlgoParam()
	{
		m_nAlgo = 0;
		m_nWndIndex = 0;
		m_nAlignCnt = 0;
		m_nCurAlignID = 0;
		m_nStartX = 0;
		m_nStartY = 0;
		m_nWndDir = 0;
		m_nSelectArea = -1;
		m_nLeadTipPos = -1;
		m_nInspType = 0;
		m_nOffX_pix = 0;
		m_nOffY_pix = 0;
		m_nInspCameraType = 0;
		m_nTypeSelectBlob = 4;

		// esjb++ 250403.[34] ~InspParamDef.h~
		m_nTypeSelectTarget = 1;
		// ++sjb 250403.[34]

		m_nBlobMinArea = 0;
		m_nInspectionMode = 0;
		m_nViewMode = 0;

		m_dx = 0.0;
		m_dy = 0.0;
		m_dWndW = 0.0;
		m_dWndH = 0.0;
		m_dInspW = 0.0;
		m_dInspH = 0.0;

		m_bViewCJ = 0;
		m_byWndInspType = 0;

		m_bInspection = FALSE;
		m_bTeach = FALSE;
		m_bUSeLeadAlign = FALSE;
		m_bTeachingFunc = FALSE;
		m_bBin = FALSE;
		m_bUseBlobOption = FALSE;
		m_bFillHole = FALSE;

		m_poCenter.x = 0;
		m_poCenter.y = 0;
		m_ptWndCenter.x = 0;
		m_ptWndCenter.y = 0;

		m_pnRet = nullptr;
		m_pnResult = nullptr;
		m_nReviewMin = nullptr;
		m_nReviewMax = nullptr;
		m_pdResultHeight = nullptr;
		m_dUSENGOCR = nullptr;
		m_dOCRNGScore = nullptr;
		m_dOCROKScore = nullptr;
		m_pfHeightMean = nullptr;
		m_ucArrDstImgAC = nullptr;

		m_rcBlobBody = nullptr;
		m_rcArrTab = nullptr;
		m_rcArrTabBridge = nullptr;

		m_sWndInfo = nullptr;
		m_sAlignWndInfo = nullptr;
		m_sAlignRes = nullptr;
		m_arrAlignRes = nullptr;
		m_sInspAlgoTempResult = nullptr;
		m_coordinateAlgo = nullptr;
		m_WndResult = nullptr;
		m_sPartAlgoImg = nullptr;
		m_ptrAlgoColorOpt = nullptr;
		m_sInspImageData = nullptr;
		m_fcBackground = nullptr;
		m_fcFoot = nullptr;
		m_fcWing = nullptr;
		m_fcWedge = nullptr;
		m_fcFindDBC = nullptr;
		m_pvInspRstPoly = NULL;

		m_nWndID = 0;
		m_bPadPattern = FALSE;
		m_sPadPoly.Init();
		m_nOverlapPadPolyCnt = 0;
		m_sArrOverlapPadPoly = NULL;
		m_nPOLY_W = 0;
		m_nPOLY_H = 0;
		m_ucPOLY = NULL;
		m_sRstAlgo_P = nullptr;
		m_nTYPE = 0;
		m_nBinaryMode = 0;
	}
}InspAlgoParam;

// TeachAlgo Param
typedef struct tagTeachAlgoParam
{
	//
	BOOL m_bInspPart;
	BOOL m_bTeachingFunc;
	BOOL m_bTeach;
	BOOL m_bIsLoadFovRawDataImage;
	BOOL m_bBin;
	BOOL m_bWindowArea;
	BOOL m_bBlob;
	BOOL m_bFillHole;
	BOOL m_bImgRectChange;
	//
	byte m_byDir;
	byte m_byWndInspType;
	byte m_bViewCJ;
	//
	int m_nMaskViewingMode;
	int m_nSelectArea;
	int m_nAlignResCnt;
	int m_nLeadTipPos;
	int m_nTypeSelectBlob;

	// esjb++ 250403.[35] ~InspParamDef.h~
	int m_nTypeSelectTarget;
	// ++sjb 250403.[35]

	int m_nBlobMinArea;
	int m_nCurLayerIdx;
	//
	TotalInspExceptArea m_stTieArea;
	WndInfo m_sWndInfo;
	WndInfo m_sAlignWndInfo;
	FootColor m_fcBackground;
	FootColor m_fcFoot;
	FootColor m_fcWing;
	FootColor m_fcWedge;
	FootColor m_fcFindDBC;
	//
	UCHAR * m_ucArrDstImg;
	UCHAR * m_ucArrDstImgAC;
	CRect * m_rcBlobBody;
	int *m_nReviewMin;
	int *m_nReviewMax;
	AlignResult * m_sAlignRes;
	float *m_pfHeightMean;
	double *m_pdResultHeight;
	//
	int m_nViewMode;
	PADPatternPoly m_sPadPoly;  // Pad Polygon
	int m_nOverlapPadPolyCnt;    // Overlap Pad Polygon 개수
	PADPatternPoly* m_sArrOverlapPadPoly; // Overlap Pad Polygon
	int m_nBinaryMode; //0:binary, 1:Gray
	tagTeachAlgoParam()
	{
		m_bInspPart = FALSE;
		m_bTeachingFunc = FALSE;
		m_bTeach = FALSE;
		m_bIsLoadFovRawDataImage = FALSE;
		m_bBin = TRUE;
		m_bWindowArea = FALSE;
		m_bBlob = FALSE;
		m_bFillHole = FALSE;
		m_bImgRectChange = FALSE;
		//
		m_byDir = 0;
		m_byWndInspType = 0;
		m_bViewCJ = 0;
		//
		m_nMaskViewingMode = 0;
		m_nSelectArea = 0;
		m_nAlignResCnt = 0;
		m_nLeadTipPos = -1;
		m_nTypeSelectBlob = 0;

		// esjb++ 250403.[35] ~InspParamDef.h~
		m_nTypeSelectTarget = 0;
		// ++sjb 250403.[35] 

		m_nBlobMinArea = 4;
		//
		//
		m_ucArrDstImg = nullptr;
		m_ucArrDstImgAC = nullptr;
		m_rcBlobBody = nullptr;
		m_nReviewMin = nullptr;
		m_nReviewMax = nullptr;
		m_sAlignRes = nullptr;
		m_pfHeightMean = nullptr;
		m_pdResultHeight = nullptr;
		//
		m_nViewMode = 0;
		m_sPadPoly.Init();
		m_nOverlapPadPolyCnt = 0;
		m_sArrOverlapPadPoly = NULL;
		m_nBinaryMode = 0;
	}
}TeachAlgoParam;

typedef struct tagTeachAIParam
{
	// Teach
	int m_nWidth;
	int m_nHeight;
	float * m_fArr3D_part;

	// Rst
	int m_nRstL;
	int m_nRstT;
	int m_nRstW;
	int m_nRstH;
	UCHAR * m_ucImgR;
	UCHAR * m_ucImgG;
	UCHAR * m_ucImgB;
	float * m_fImg3D;

	BOOL m_bRot;
	int m_nFontCnt;
	wchar_t m_sArrFont[MAX_STRLEN];
	RECT m_rcArrROI[MAX_STRLEN];

	tagTeachAIParam()
	{
		m_nWidth = 0;
		m_nHeight = 0;
		m_fArr3D_part = NULL;

		m_nRstL = 0;
		m_nRstT = 0;
		m_nRstW = 0;
		m_nRstH = 0;
		m_ucImgR = NULL;
		m_ucImgG = NULL;
		m_ucImgB = NULL;
		m_fImg3D = NULL;
	}
}TeachAIParam;

typedef struct tagAI_LEADRst
{
	BOOL m_bRst;

	double m_dM;
	double m_dA;
	double m_dB;

	tagAI_LEADRst()
	{
		m_bRst = FALSE;

		m_dM = 0;
		m_dA = 0;
		m_dB = 0;
	}
}AI_LEADRst;
typedef struct tagAlgoLeadSearch_T
{
	WndInfo m_sWndInfo;

	int m_nPos;
	int m_nDir;
	int m_nSolderCnt;

	int m_nArrGap[3];
	double m_dArrT[3];
	int m_nArrTipPos[BRIEDGE_CNT];
	RECT m_arrRect[BRIEDGE_CNT];

	double m_dArrRst[3];

	tagAlgoLeadSearch_T()
	{
		m_nDir = 0;
		m_nSolderCnt = 0;
		memset(m_nArrGap, 0, sizeof(int) * 3);
		memset(m_dArrT, 0, sizeof(int) * 3);
		memset(m_nArrTipPos, 0, sizeof(int) * BRIEDGE_CNT);
		memset(m_arrRect, 0, sizeof(RECT) * BRIEDGE_CNT);
		memset(m_dArrRst, 0, sizeof(double) * 3);
	}
}AlgoLeadSearch_T;

typedef struct tagInsp_PCBSurface
{
	UCHAR* m_pucTR;
	UCHAR* m_pucTG;
	UCHAR* m_pucTB;
	UCHAR* m_pucTW;
	UCHAR* m_pucMR;
	UCHAR* m_pucMB;
	UCHAR* m_pucBR;
	UCHAR* m_pucBB;
	int m_nW;
	int m_nH;
	double m_dFactorR;
	double m_dFactorG;
	double m_dFactorB;

	int m_nIndex;
	int m_nVFIndex;
	RECT m_rcROI;

	byte m_byMin[eM2C_NUM];
	byte m_byMax[eM2C_NUM];
	byte m_byAvg[eM2C_NUM];
	POINTF m_ptArrPolygon[MAX_INSP_AREA_COUNT][POLYGON_BTM_POINT_CNTS];
	ColorXYInfoForeign m_ColorXYInfo[MAX_INSP_AREA_COUNT];

	tagInsp_PCBSurface()
	{
		m_pucTR = NULL;
		m_pucTG = NULL;
		m_pucTB = NULL;
		m_pucTW = NULL;
		m_pucMR = NULL;
		m_pucMB = NULL;
		m_pucBR = NULL;
		m_pucBB = NULL;
		m_nW = 1;
		m_nH = 1;
		m_dFactorR = 1.0;
		m_dFactorG = 1.0;
		m_dFactorB = 1.0;

		m_nIndex = -1;
		m_nVFIndex = -1;
		m_rcROI.left = 0;
		m_rcROI.right = 0;
		m_rcROI.top = 0;
		m_rcROI.bottom = 0;
		for (int n = 0; n < eM2C_NUM; n++)
		{
			m_byMin[n] = 0;
			m_byMax[n] = 255;
			m_byAvg[n] = 0;
		}

		for (int a = 0; a < MAX_INSP_AREA_COUNT; a++)
			memset(m_ptArrPolygon[a], 0, sizeof(POINTF) * POLYGON_BTM_POINT_CNTS);
		memset(m_ColorXYInfo, 0, sizeof(ColorXYInfoForeign) * MAX_INSP_AREA_COUNT);
	}
}Insp_PCBSurface;

typedef struct tagAUTO_INSP_AI_DATA
{
	int nAlgoType;
	RECT rcWndROI;
	int m_nImgW;
	int m_nImgH;
	int m_nImgWndW;
	int m_nImgWndH;

	int m_nArrLen;
	RECT m_rcArrROI[MAX_STRLEN];

	UCHAR* pucImage;
	UCHAR* pucImageBin;
	UCHAR* pucImageWnd;
	wchar_t m_sLogDir[MAX_STRLEN];

	tagAUTO_INSP_AI_DATA()
	{
		nAlgoType = 0;
		m_nImgW = 0;
		m_nImgH = 0;
		m_nImgWndW = 0;
		m_nImgWndH = 0;
		m_nArrLen = 0;
		memset(m_rcArrROI, 0, sizeof(RECT) * MAX_STRLEN);

		pucImage = NULL;
		pucImageBin = NULL;
		pucImageWnd = NULL;
	}
}AUTO_INSP_AI_DATA;
typedef struct tagAUTO_INSP_DataSave
{
	SaveImgParamData m_tgParam;
	InspPartInfo m_pInsp;
}AUTO_INSP_DataSave;
typedef struct tagSaveImgDefect
{
	int m_nROI;
	RECT m_rcROI[MAX_STRLEN];
	wchar_t arrDefect[MAX_STRLEN];
	wchar_t arrFileName[MAX_STRLEN];
	int *pArrSideImg;

	tagSaveImgDefect()
	{
		m_nROI = 0;
		::wmemset(arrFileName, 0, MAX_STRLEN);
		pArrSideImg = NULL;
	}
} SaveImgDefect;
typedef struct tagJob_3DData
{
	RECT m_rcPart3DWndMargin;

	tagJob_3DData()
	{
	}

	void Clone(tagJob_3DData &sData)
	{
		sData.m_rcPart3DWndMargin.left = m_rcPart3DWndMargin.left;
		sData.m_rcPart3DWndMargin.right = m_rcPart3DWndMargin.right;
		sData.m_rcPart3DWndMargin.top = m_rcPart3DWndMargin.top;
		sData.m_rcPart3DWndMargin.bottom = m_rcPart3DWndMargin.bottom;
	}
} Job_3DData;

enum DiagnosticCheck_LED_InspPos
{
	TL = 0,
	TR,
	CT,
	BL,
	BR,
	NUM,
};
enum DiagnosticCheck_PZT_InspType
{
	DC_InspType_Strength = 0,
	DC_InspType_PhaseDiff,
	DC_InspType_NUM,
};
typedef struct tagDiagnostic_LED
{
	int m_nW;
	int m_nH;

	RECT m_rcInspROI[DiagnosticCheck_LED_InspPos::NUM];

	// LED 2D
	UCHAR* m_pucGray[MPTI_2D_Channel::eM2C_NUM];
	byte m_byRange2D;
	int m_nMean2D[MPTI_2D_Channel::eM2C_NUM * DiagnosticCheck_LED_InspPos::NUM];
	byte m_byMean2DRst[MPTI_2D_Channel::eM2C_NUM * DiagnosticCheck_LED_InspPos::NUM];
	BOOL m_bOK2D[MPTI_2D_Channel::eM2C_NUM * DiagnosticCheck_LED_InspPos::NUM];

	// LED 3D
	UCHAR* m_pucProj[MPTI_3D_Channel::eM3C_NUM];
	int m_nProjNum;
	byte m_byBrightness3D;
	byte m_byGridThreshold3D;
	byte m_byRange3D;
	int m_nMean3D[MPTI_3D_Channel::eM3C_NUM * DiagnosticCheck_LED_InspPos::NUM];
	byte m_byMean3DRst[MPTI_3D_Channel::eM3C_NUM * DiagnosticCheck_LED_InspPos::NUM];
	BOOL m_bOK3D[MPTI_3D_Channel::eM3C_NUM * DiagnosticCheck_LED_InspPos::NUM];

	tagDiagnostic_LED()
	{
		m_nW = 0;
		m_nH = 0;

		RECT rcInit;
		rcInit.left = 0; rcInit.bottom = 0; rcInit.right = 0; rcInit.top = 0;
		for (int i = 0; i < DiagnosticCheck_LED_InspPos::NUM; i++)
		{
			m_rcInspROI[i] = rcInit;
		}

		m_nProjNum = 16;
		m_byRange2D = 0;
		m_byRange3D = 0;
		m_byGridThreshold3D = 20;
		m_byBrightness3D = 42;

		for (int i = 0; i < MPTI_2D_Channel::eM2C_NUM; i++)
		{
			m_pucGray[i] = NULL;

			for (int j = 0; j < DiagnosticCheck_LED_InspPos::NUM; j++)
			{
				m_nMean2D[j + i * MPTI_2D_Channel::eM2C_NUM] = -1;
				m_byMean2DRst[j + i * MPTI_2D_Channel::eM2C_NUM] = 0;
				m_bOK2D[j + i * MPTI_2D_Channel::eM2C_NUM] = FALSE;
			}
		}

		for (int i = 0; i < MPTI_3D_Channel::eM3C_NUM; i++)
		{
			m_pucProj[i] = NULL;

			for (int j = 0; j < DiagnosticCheck_LED_InspPos::NUM; j++)
			{
				m_nMean3D[j + i * MPTI_3D_Channel::eM3C_NUM] = -1;
				m_byMean3DRst[j + i * MPTI_3D_Channel::eM3C_NUM] = 0;
				m_bOK3D[j + i * MPTI_3D_Channel::eM3C_NUM] = FALSE;
			}
		}
	}
}Diagnostic_LED;

typedef struct tagDiagnostic_CameraFocus
{
	int m_nW;
	int m_nH;

	RECT m_rcInspROI;

	UCHAR* m_pucGrab;

	tagDiagnostic_CameraFocus()
	{
		m_nW = 0;
		m_nH = 0;

		m_rcInspROI.left = 0;
		m_rcInspROI.right = 0;
		m_rcInspROI.top = 0;
		m_rcInspROI.bottom = 0;

		m_pucGrab = NULL;
	}
}Diagnostic_CameraFocus;

typedef struct tagDiagnostic_PZT
{
	int m_nProjNum;

	int m_nW;
	int m_nH;

	UCHAR* m_pucGrab[MPTI_3D_Channel::eM3C_NUM * 4];	// MAX_BUCKETS = 4

	int m_nStrengthSubtraction;
	int m_nPhaseDiffRange;
	int m_nPhaseDiffRange_12;
	int m_nInspCropType;

	int m_nStandardStrength[MPTI_3D_Channel::eM3C_NUM];
	int m_nStandardPhaseDiff[MPTI_3D_Channel::eM3C_NUM];

	double m_dArrResultScore[MPTI_3D_Channel::eM3C_NUM * DiagnosticCheck_PZT_InspType::DC_InspType_NUM];
	BOOL m_bOK[MPTI_3D_Channel::eM3C_NUM * DiagnosticCheck_PZT_InspType::DC_InspType_NUM];

	tagDiagnostic_PZT()
	{
		m_nProjNum = 0;

		m_nW = 0;
		m_nH = 0;

		m_nStrengthSubtraction = 0;
		m_nPhaseDiffRange = 0;
		m_nPhaseDiffRange_12 = 0;
		m_nInspCropType = 0;

		for (int i = 0; i < MPTI_3D_Channel::eM3C_NUM; i++)
		{
			m_nStandardStrength[i] = 0;
			m_nStandardPhaseDiff[i] = 0;
			for (int j = 0; j < 4; j++)
			{
				m_pucGrab[j + i * MPTI_3D_Channel::eM3C_NUM] = NULL;
			}
			for (int j = 0; j < DiagnosticCheck_PZT_InspType::DC_InspType_NUM; j++)
			{
				m_dArrResultScore[j + i * MPTI_3D_Channel::eM3C_NUM] = 0;
				m_bOK[j + i * MPTI_3D_Channel::eM3C_NUM] = FALSE;
			}
		}
	}
}Diagnostic_PZT;

enum m_ePAT_BIG
{
	m_ePAT_BIG_LT = 0x01,
	m_ePAT_BIG_FIND = 0x02,
	m_ePAT_BIG_DIV = 0x04,
	m_ePAT_BIG_ONE = 0x08,

	m_ePAT_BIG_PG = 0x10,
};
typedef struct tagGB_PAT_BIG
{
	int m_nType;
	int m_nW;
	int m_nH;
	int m_nTotal;
	int m_nExtraPixel;
	int m_nMAX;

	double m_dPAT_PAD_DIV_X;
	double m_dPAT_PAD_DIV_Y;
	double m_dPatResX;
	double m_dPatResY;
	double m_dFovW;
	double m_dFovH;
	double m_dSmall;
	double m_dFovW_Max;
	double m_dFovH_Max;

	bool UseData(int nType)
	{
		return ((m_nType & nType) == nType);
	}
	bool CheckFail(int nRst, int nRst_W, int nRst_H)
	{
		if (nRst < 0 || nRst >= m_nTotal - 1)
			return false;
		else if (nRst_W <= m_dSmall || nRst_H <= m_dSmall)
			return false;
		else if (nRst_W >= m_dFovW_Max || nRst_H >= m_dFovH_Max)
			return false;

		return true;
	}
	void Init()
	{
		m_nType = 0;
		m_nW = 0;
		m_nH = 0;
		m_nTotal = 0;
		m_nExtraPixel = 0;
		m_nMAX = 0;

		m_dPAT_PAD_DIV_X = 0;
		m_dPAT_PAD_DIV_Y = 0;
		m_dPatResX = 0;
		m_dPatResY = 0;
		m_dFovW = 0;
		m_dFovH = 0;
		m_dSmall = 0;
		m_dFovW_Max = 0;
		m_dFovH_Max = 0;
	}
}GB_PAT_BIG;
typedef struct tagGB_PAT_DIV
{
	RECT m_rcROI;
	int m_nArea;
	int m_nX;
	int m_nY;

	int m_nPoly;
	POINTF m_po[ZMPOLYPOINT_ULTRA_CNTS];

	bool CheckFail(bool bPAT)
	{
		int nRst_W = m_rcROI.right - m_rcROI.left;
		int nRst_H = m_rcROI.bottom - m_rcROI.top;
		int nMin = PAT_INSP_MIN_SIZE;
		if (bPAT == false)
			nMin = PAT_INSP_MIN_SIZE2;
		if (m_rcROI.left < 0 || m_rcROI.right < 0 || m_rcROI.top < 0 || m_rcROI.bottom < 0)
			return false;
		else if (m_nX <= 0 || m_nY <= 0)
			return false;
		else if (m_nArea <= 0)
			return false;
		else if (nRst_W <= nMin || nRst_H <= nMin)
			return false;

		return true;
	}
	bool SetROI(int nL, int nT, int nW, int nH, bool bPAT)
	{
		m_rcROI.left = nL;
		m_rcROI.top = nT;
		m_rcROI.right = nL + nW;
		m_rcROI.bottom = nT + nH;

		return CheckFail(bPAT);
	}
	bool SetData(int nL, int nT, int nW, int nH, int nArea, int nX, int nY, bool bPAT)
	{
		m_rcROI.left = nL;
		m_rcROI.top = nT;
		m_rcROI.right = nL + nW;
		m_rcROI.bottom = nT + nH;
		m_nArea = nArea;
		m_nX = nX;
		m_nY = nY;
		m_nPoly = 0;
		for (int a = 0; a < ZMPOLYPOINT_ULTRA_CNTS; a++)
		{
			m_po[a].x = 0;
			m_po[a].y = 0;
		}
		return CheckFail(bPAT);
	}
	void SetPoly(std::vector<cv::Point> arrPoly, int nAddX = 0, int nAddY = 0)
	{
		m_nPoly = arrPoly.size();
		if (m_nPoly < 3 || m_nPoly > ZMPOLYPOINT_ULTRA_CNTS)
		{
			m_nPoly = 0;
			return;
		}

		for (int a = 0; a < m_nPoly; a++)
		{
			m_po[a].x = arrPoly[a].x + nAddX;
			m_po[a].y = arrPoly[a].y + nAddY;
		}
	}
}GB_PAT_DIV;
typedef struct tagGB_PAT_POLY
{
	int m_nType;
	int m_nW;
	int m_nH;
	int m_nMAX;
	int m_nExtraPixel;
	int m_nTotal;
	int m_nROI_CNT;
	double m_dMaxPer;

	RECT m_rcROI;

	bool UseData(int nType)
	{
		if ((m_nType & nType) == nType)
			return true;

		return false;
	}
}GB_PAT_POLY;
typedef struct tagRstCombinePAT
{
	int m_nW;
	int m_nH;
	double m_dCX;
	double m_dCY;

	UCHAR* m_pImgPat;

}RstCombinePAT;

typedef struct tagINSP_PAD_PAT
{
	int m_nData;
	int m_nTYPE;
	int m_nORG_W;
	int m_nORG_H;

	int m_nL;
	int m_nT;
	int m_nW;
	int m_nH;
	int m_nShiftX;
	int m_nShiftY;

	double m_dAngle;
	AlgoVolume m_sAlgo;

	PADPatternPoly m_sPadPoly;  // Pad Polygon

	int m_nOverlapPadPolyCnt;
	PADPatternPoly* m_sArrOverlapPadPoly; // Overlap Pad Polygon

	UCHAR* m_ucBIN;
	UCHAR* m_ucGRAY;
	float* m_f3D;

	int m_nUsedWndPolygon;           // 설정 ROI 개수 (1~20)	
	POINTF m_ptArrWndPolygon[MAX_INSP_AREA_COUNT];

	InspAlgo m_sInspAlgo;

	BOOL m_bPadPattern;

	tagINSP_PAD_PAT()
	{
		m_nData = 0;
		m_nTYPE = 0;
		m_nORG_W = 0;
		m_nORG_H = 0;

		m_nL = 0;
		m_nT = 0;
		m_nW = 0;
		m_nH = 0;
		m_nShiftX = 0;
		m_nShiftY = 0;

		m_dAngle = 0;

		m_nOverlapPadPolyCnt = 0;
		m_ucBIN = NULL;
		m_ucGRAY = NULL;
		m_f3D = NULL;

		m_nUsedWndPolygon = 0;

		POINTF ptInit;
		ptInit.x = 0; ptInit.y = 0;

		for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
			m_ptArrWndPolygon[i] = ptInit;

		m_bPadPattern = FALSE;
	}
	bool UseData(int nType)
	{
		if ((m_nData & nType) == nType)
			return true;

		return false;
	}
}INSP_PAD_PAT;
typedef struct tagPAD_2D_DATA
{
	int m_nFov;
	int m_nW;
	int m_nH;

	int m_nRange;
	int m_nMIN;
	int m_nMAX;

	UCHAR* m_pGRAY;
	UCHAR* m_pBIN;
	float* m_p3D;

	tagForignLight m_MixLight;

}PAD_2D_DATA;
#pragma endregion Parameter Info

#pragma region  VOL_TH
#define PAD_OUT_SIZE	5000
typedef struct tagVOL_TH_DATA
{
	int m_nMAX;
	int m_nTotal;

	int m_nC;
	double m_dSumX;
	double m_dSumY;
	double m_dSumD;
	double* m_dArrCX;
	double* m_dArrCY;

	int m_nIDX;
	int* m_nArrB;
	int* m_nArrS;
	int* m_nArrE;
	int* m_nArrC;

	int m_nMAX_SZ;
	int* m_nArrSZ;
	int* m_nArrSZ_CNT;

	tagVOL_TH_DATA()
	{
		m_nMAX = 0;
		m_nTotal = 0;

		m_nC = 0;
		m_dSumX = 0;
		m_dSumY = 0;
		m_dSumD = 0;
		m_dArrCX = NULL;
		m_dArrCY = NULL;

		m_nIDX = 0;
		m_nArrB = NULL;
		m_nArrS = NULL;
		m_nArrE = NULL;
		m_nArrC = NULL;

		m_nMAX_SZ = 0;
		m_nArrSZ = NULL;
		m_nArrSZ_CNT = NULL;
	}
	void SetData(int nW, int nH, bool bMemset = false)
	{
		m_nMAX = nW + nH;
		m_nTotal = nW * nH;

		m_dArrCX = g_pMManager->pem_new<double>(true, m_nTotal, (PCHAR)__FUNCTION__, __LINE__);
		m_dArrCY = g_pMManager->pem_new<double>(true, m_nTotal, (PCHAR)__FUNCTION__, __LINE__);

		m_nArrB = g_pMManager->pem_new<int>(true, m_nTotal, (PCHAR)__FUNCTION__, __LINE__);
		m_nArrS = g_pMManager->pem_new<int>(true, m_nTotal, (PCHAR)__FUNCTION__, __LINE__);
		m_nArrE = g_pMManager->pem_new<int>(true, m_nTotal, (PCHAR)__FUNCTION__, __LINE__);
		m_nArrC = g_pMManager->pem_new<int>(true, m_nTotal, (PCHAR)__FUNCTION__, __LINE__);

		m_nArrSZ = g_pMManager->pem_new<int>(true, m_nTotal, (PCHAR)__FUNCTION__, __LINE__);
		m_nArrSZ_CNT = g_pMManager->pem_new<int>(true, m_nTotal, (PCHAR)__FUNCTION__, __LINE__);

		ClearValue(bMemset, true);
	}
	void ClearValue(bool bMemset = false, bool bCNT = false)
	{
		ClearValueC(bMemset);

		m_nIDX = 0;
		if (bMemset)
		{
			memset(m_nArrB, 0, sizeof(int) * m_nTotal);
			memset(m_nArrS, 0, sizeof(int) * m_nTotal);
			memset(m_nArrE, 0, sizeof(int) * m_nTotal);
			memset(m_nArrC, 0, sizeof(int) * m_nTotal);
		}

		if (bMemset)
			memset(m_nArrSZ, 0, sizeof(int) * m_nTotal);

		int nSZ = m_nTotal;
		if (bCNT)
		{
			if (m_nMAX > 0)
				nSZ = m_nMAX;
		}
		else
			nSZ = m_nMAX_SZ;
		memset(m_nArrSZ_CNT, 0, sizeof(int) * nSZ);
		m_nMAX_SZ = 0;
	}
	void ClearValueC(bool bMemset = false)
	{
		m_nC = 0;
		m_dSumX = 0;
		m_dSumY = 0;
		m_dSumD = 0;
		if (bMemset)
		{
			memset(m_dArrCX, 0, sizeof(double) * m_nTotal);
			memset(m_dArrCY, 0, sizeof(double) * m_nTotal);
		}
	}
	void ClearData()
	{
		m_nMAX = 0;
		m_nTotal = 0;

		m_nC = 0;
		m_dSumX = 0;
		m_dSumY = 0;
		m_dSumD = 0;
		if (m_dArrCX != NULL) 
		{
			g_pMManager->pem_delete(m_dArrCX, true);
			m_dArrCX = NULL;
		}
		if (m_dArrCY != NULL)
		{
			g_pMManager->pem_delete(m_dArrCY, true);
			m_dArrCY = NULL;
		}

		m_nIDX = 0;
		if (m_nArrB != NULL)
		{
			g_pMManager->pem_delete(m_nArrB, true);
			m_nArrB = NULL;
		}
		if (m_nArrS != NULL)
		{
			g_pMManager->pem_delete(m_nArrS, true);
			m_nArrS = NULL;
		}
		if (m_nArrE != NULL)
		{
			g_pMManager->pem_delete(m_nArrE, true);
			m_nArrE = NULL;
		}
		if (m_nArrC != NULL)
		{
			g_pMManager->pem_delete(m_nArrC, true);
			m_nArrC = NULL;
		}

		m_nMAX_SZ = 0;
		if (m_nArrSZ != NULL)
		{
			g_pMManager->pem_delete(m_nArrSZ, true);
			m_nArrSZ = NULL;
		}
		if (m_nArrSZ_CNT != NULL)
		{
			g_pMManager->pem_delete(m_nArrSZ_CNT, true);
			m_nArrSZ_CNT = NULL;
		}
	}
	void AddData(double dX, double dY, double dDist)
	{
		m_dArrCX[m_nC] = dX;
		m_dArrCY[m_nC] = dY;
		m_dSumX += m_dArrCX[m_nC];
		m_dSumY += m_dArrCY[m_nC];
		if (dDist >= 0)
			m_dSumD += (dDist + 1);
		m_nC++;
	}
	double GetAVG(bool bX, bool bDist = false)
	{
		if (m_nC <= 0)
			return 0;
		if (bDist)
		{
			if (m_dSumD <= 0)
				return 0;
			else
				return m_dSumD / m_nC;
		}

		double dSum = bX ? m_dSumX : m_dSumY;
		if (dSum == 0)
			return 0;

		return dSum / m_nC;
	}
	double CheckValue(bool bX)
	{
		double dAVG = GetAVG(bX);
		double dAvgMax = 0;
		for (int a = 0; a < m_nC; a++)
		{
			double dVal = bX ? m_dArrCX[a] : m_dArrCY[a];
			double dDiff = abs(dAVG - dVal);
			if (dDiff > dAvgMax)
				dAvgMax = dDiff;
		}
		return dAvgMax;
	}
	void AddData(int nB, int nS, int nE, bool bCenter)
	{
		m_nArrB[m_nIDX] = nB;
		m_nArrS[m_nIDX] = nS;
		m_nArrE[m_nIDX] = nE;
		m_nArrC[m_nIDX] = (nE + nS) / 2;
		m_nIDX++;

		int nGAP = nE - nS + 1;
		if (bCenter)
			nGAP = (nE + nS) / 2;
		int nIDX = -1;
		for (int a = 0; a < m_nMAX_SZ; a++)
		{
			if (m_nArrSZ[a] == nGAP)
			{
				nIDX = a;
				break;
			}
		}
		if (nIDX >= 0 && m_nArrSZ[nIDX] == nGAP)
		{
			m_nArrSZ_CNT[nIDX] += 1;
		}
		else
		{
			m_nArrSZ[m_nMAX_SZ] = nGAP;
			m_nArrSZ_CNT[m_nMAX_SZ] += 1;
			m_nMAX_SZ++;
		}
	}
	void AddDataL(int nB, int nE)
	{
		m_nArrB[m_nIDX] = nB;
		m_nArrE[m_nIDX] = nE;
		m_nIDX++;

		int nIDX = -1;
		for (int a = 0; a < m_nMAX_SZ; a++)
		{
			if (m_nArrSZ[a] == nE)
			{
				nIDX = a;
				break;
			}
		}
		if (nIDX >= 0 && m_nArrSZ[nIDX] == nE)
		{
			m_nArrSZ_CNT[nIDX] += 1;
		}
		else
		{
			m_nArrSZ[m_nMAX_SZ] = nE;
			m_nArrSZ_CNT[m_nMAX_SZ] += 1;
			m_nMAX_SZ++;
		}
	}
	std::vector<int> GetMAX_LIST(int nFIND_GAP, int N_POLY_DIFF = -1)
	{
		std::vector<int> vecMAX;
		for (int a = 0; a < m_nMAX_SZ; a++)
		{
			int nMAX_GAP = GetMAX_GAP(vecMAX, nFIND_GAP);
			if (nMAX_GAP <= 0)
				return vecMAX;
			if (N_POLY_DIFF < 0)
			vecMAX.push_back(nMAX_GAP);
			else
			{
				if (vecMAX.size() == 0)
					vecMAX.push_back(nMAX_GAP);
				else
				{
					bool bSkip = false;
					for (int a = 0; a < vecMAX.size(); a++)
					{
						if (std::abs(nMAX_GAP - vecMAX[a]) == N_POLY_DIFF)
						{
							bSkip = true;
							break;
						}
					}
					if (bSkip == false)
						vecMAX.push_back(nMAX_GAP);
				}
			}
		}
		return vecMAX;
	}
	int GetMAX_GAP(std::vector<int> vecMAX, int nFIND_GAP)
	{
		int nMAX_GAP = -1;
		int nMAX_CNT = -1;
		for (int b = 0; b < m_nMAX_SZ; b++)
		{
			int nGAP = m_nArrSZ[b];
			int nCnt = m_nArrSZ_CNT[b];
			if (nCnt < nFIND_GAP)
				continue;

			auto it = std::find(vecMAX.begin(), vecMAX.end(), nGAP);
			if (it != vecMAX.end())
				continue;
			if (nCnt > nMAX_CNT)
			{
				nMAX_CNT = nCnt;
				nMAX_GAP = nGAP;
			}
		}
		if (nMAX_GAP <= 0 || nMAX_CNT <= 0)
			return 0;

		return nMAX_GAP;
	}
	void Set_IMG(cv::Mat imgSrc, cv::Mat img, int nMAX_GAP, int nPOLY_DIFF, bool bX, bool bCenter = false)
	{
		for (int a = 0; a < m_nIDX; a++)
		{
			int nB = m_nArrB[a];
			int nS = m_nArrS[a];
			int nE = m_nArrE[a];
			int nGAP = nE - nS + 1;
			if (bCenter)
				nGAP = (nE + nS) / 2;
			if (abs(nGAP - nMAX_GAP) > nPOLY_DIFF)
				continue;

			nGAP = nE - nS + 1;
			if (bX)
			{
				cv::Rect roi(nS, nB, nGAP, 1);
				if (roi.x < 0 || roi.y < 0 || roi.x + roi.width > imgSrc.cols || roi.y + roi.height > imgSrc.rows)
					continue;
				imgSrc(roi).copyTo(img(roi));
			}
			else
			{
				cv::Rect roi(nB, nS, 1, nGAP);
				if (roi.x < 0 || roi.y < 0 || roi.x + roi.width > imgSrc.cols || roi.y + roi.height > imgSrc.rows)
					continue;
				imgSrc(roi).copyTo(img(roi));
			}
		}
	}
	cv::Rect Set_IMG(cv::Mat imgSrc, cv::Mat img, int nSIZE, int nBIN_GAP_DIFF, bool bX, cv::Rect roiP, int* nPol)
	{
		int nRectL = img.cols * img.rows;
		int nRectR = 0;
		int nRectT = img.cols * img.rows;
		int nRectB = 0;
		int nSIZE_MIN = nSIZE - (nBIN_GAP_DIFF * 2);
		int nSIZE_MAX = nSIZE + (nBIN_GAP_DIFF * 2);
		*nPol = 0;
		for (int a = 0; a < m_nIDX; a++)
		{
			int nB = m_nArrB[a];
			int nS = m_nArrS[a];
			int nE = m_nArrE[a];
			int nGAP = nE - nS + 1;
			if (nGAP >= nSIZE_MAX)
				continue;
			cv::Rect roi = bX ? cv::Rect(nS, nB, nGAP, 1) : cv::Rect(nB, nS, 1, nGAP);
			cv::Rect intersection = roiP & roi;
			if (intersection.area() <= 0)
				continue;

			if (bX)
			{
				imgSrc(roi).copyTo(img(roi));
				if (nRectL > roi.x) nRectL = roi.x;
				if (nRectR < roi.x + roi.width) nRectR = roi.x + roi.width;
				if (nRectT > roi.y) nRectT = roi.y;
				if (nRectB < roi.y + roi.height) nRectB = roi.y + roi.height;
				(*nPol)++;
			}
			else
			{
				imgSrc(roi).copyTo(img(roi));
				if (nRectL > roi.x) nRectL = roi.x;
				if (nRectR < roi.x + roi.width) nRectR = roi.x + roi.width;
				if (nRectT > roi.y) nRectT = roi.y;
				if (nRectB < roi.y + roi.height) nRectB = roi.y + roi.height;
				(*nPol)++;
			}
		}
		cv::Rect rect = cv::boundingRect(img);
		return rect;
	}
	void DEBUG_POLY(cv::Mat img, double dA, double dB, double dAVGA, double dAVGB, bool bX)
	{
#if !_DEBUG
		return;
#endif
		cv::Mat imgRGB(img.rows, img.cols, CV_8UC3, cv::Scalar(0));
		const int RGB = 3;
		cv::Mat imgArr[RGB];
		for (int a = 0; a < RGB; a++)
			imgArr[a] = img.clone();

		int nW = img.cols;
		int nH = img.rows;
		for (int a = 0; a < m_nC; a++)
		{
			double dX = m_dArrCX[a];
			double dY = m_dArrCY[a];
			int nIDX = ((int)dY * img.cols) + (int)dX;

			double dX2 = bX ? (m_dArrCY[a] * dA) + dB : m_dArrCX[a];
			double dY2 = bX ? m_dArrCY[a] : (m_dArrCX[a] * dA) + dB;
			int nIDX2 = ((int)dY2 * img.cols) + (int)dX2;
			if (dX >= 0 && dX < nW && dY >= 0 && dY < nH)
			{
				imgArr[0].data[nIDX] = 255;
				imgArr[1].data[nIDX] = 0;
				imgArr[2].data[nIDX] = 0;
			}

			if (dX2 >= 0 && dX2 < nW && dY2 >= 0 && dY2 < nH)
			{
				imgArr[0].data[nIDX2] = 0;
				imgArr[1].data[nIDX2] = 0;
				imgArr[2].data[nIDX2] = 255;
				if (nIDX && nIDX)
					imgArr[0].data[nIDX] = 255;
			}
		}

		double dReA = -1.0 / dA;
		double dReB = bX ? dAVGA - (dReA * dAVGB) : dAVGB - (dReA * dAVGA);
		double dAVGA_2 = bX ? dReA * dAVGB + dReB : dReA * dAVGA + dReB;
		double dAVGB_2 = bX ? (dAVGA - dReB) / dReA : (dAVGB - dReB) / dReA;
		int nT = bX ? img.cols : img.rows;
		if (dA != 0)
		{
			bool bX1 = true;
			for (int a = 0; a < img.cols; a++)
			{
				double dX = bX1 ? a : a * dReA + dReB;
				double dY = bX1 ? a * dReA + dReB : a;
				if (dX < 0 || dX >= img.cols)
					continue;
				if (dY < 0 || dY >= img.rows)
					continue;
				int nIDX = ((int)dY * img.cols) + (int)dX;
				imgArr[0].data[nIDX] = 0;
				imgArr[1].data[nIDX] = 255;
				imgArr[2].data[nIDX] = 0;
			}

			bool bX2 = false;
			for (int a = 0; a < img.rows; a++)
			{
				double dX = bX2 ? a : a * dReA + dReB;
				double dY = bX2 ? a * dReA + dReB : a;
				if (dX < 0 || dX >= img.cols)
					continue;
				if (dY < 0 || dY >= img.rows)
					continue;
				int nIDX = ((int)dY * img.cols) + (int)dX;
				imgArr[0].data[nIDX] = 255;
				imgArr[1].data[nIDX] = 255;
				imgArr[2].data[nIDX] = 0;
			}
		}
		cv::merge(imgArr, RGB, imgRGB);

		if (RGB > 3)
		{

		}
	}
}VOL_TH_DATA;
typedef struct tagVOL_TH_RE
{
	UCHAR* m_puc2D;
	float* m_pf3D;
	int m_nW;
	int m_nH;
	AlgoVolume* m_pAlgo;
	tagVOL_TH_RE()
	{
	}
	void SetData(WndAlgoImg &sWndAlgoImg, AlgoVolume* pInspAlgoVolume)
	{
		m_puc2D = sWndAlgoImg.m_ucArr2D;
		m_pf3D = sWndAlgoImg.m_fArr3D;
		m_nW = sWndAlgoImg.m_nWidth;
		m_nH = sWndAlgoImg.m_nHeight;
		m_pAlgo = pInspAlgoVolume;
	}
	bool WhiteArea(int nIndex, float fPer = 0.15f)
	{
		if (nIndex < 0 || nIndex >= m_nW * m_nH)
			return false;
		if (m_pAlgo->m_b2dCheck == false && m_pAlgo->m_b3dCheck == false)
			return false;
		if (m_pAlgo->m_bInvertCheck)
			return false;

		bool b2D = m_pAlgo->m_b2dCheck == false;
		bool b3D = m_pAlgo->m_b3dCheck == false;
		if (m_pAlgo->m_b2dCheck)
		{
			float fVal = m_puc2D[nIndex];
			int nRange = m_pAlgo->m_nRange;
			float fMin = m_pAlgo->m_nMinValue;
			float fMax = m_pAlgo->m_nMaxValue;
			b2D = CheckW(nRange, fMin, fMax, fVal, fPer);
		}
		if (m_pAlgo->m_b3dCheck)
		{
			float fVal = m_pf3D[nIndex];
			int nRange = m_pAlgo->m_n3dRange;
			float fMin = m_pAlgo->m_d3dHeightMin;
			float fMax = m_pAlgo->m_d3dHeightMax;
			b3D = CheckW(nRange, fMin, fMax, fVal, fPer);
		}

		return b2D && b3D;
	}
	bool CheckW(int nRange, float fMinVal, float fMaxVal, float fVal, float fPer)
	{
		float fMinPer = 1.0f - fPer;
		float fMaxPer = 1.0f + fPer;
		switch (nRange)
		{
		case eTypeRangeIn:
		{
			float fMin = fMinVal * fMinPer - 1;
			float fMax = fMaxVal * fMaxPer + 1;
			if (fMin <= fVal && fMax >= fVal)
				return true;
			return false;
		}
		case eTypeRangeOut:
		{
			float fMin = fMinVal * fMinPer - 1;
			float fMax = fMaxVal * fMaxPer + 1;
			if (fMin >= fVal || fMax <= fVal)
				return true;
			return false;
		}
		case eTypeRangeUpper:
		{
			float fMax = fMaxVal * fMinPer - 1;
			if (fMax <= fVal)
				return true;
			return false;
		}
		case eTypeRangeLower:
		{
			float fMin = fMinVal * fMaxPer + 1;
			if (fMin >= fVal)
				return true;
			return false;
		}
		}
		return false;
	}
}VOL_TH_RE;

typedef struct tagWndLightParam
{
	int m_nLightType;
	RECT rtWnd;
	wchar_t* m_sPath;
	lightData WndLight;
}WndLightParam;
#pragma endregion
typedef struct tagMultiStruct	//
 {
	 int nRoiSizeX;
	 int nRoiSizeY;
	 int nWidthStep;
	 int nRoiArea;
	 tagMultiStruct()
	 {
		 nRoiSizeX = 0;
		 nRoiSizeY = 0;
		 nWidthStep = 0;
		 nRoiArea = 0;
	 }
 }MultiStruct;
#pragma endregion Parameter Info