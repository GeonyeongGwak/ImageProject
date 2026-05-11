#pragma once
#include <Windows.h>
//////////////////////////////////////////////////////////////////////////
// PZmapDef.h

#define PROJ_TYPE_ORIGINAL		0
#define PROJ_TYPE_8_4			1
#define PROJ_TYPE_12_4			2
#define PROJ_TYPE_16_0			3
#define PROJ_TYPE_8_0			4
#define PROJ_TYPE_12_0			5
#define PROJ_TYPE_4_NO_DUAL		6
#define PROJ_TYPE_2				7
#define PROJ_TYPE_TPI			8

#define DLP_TYPE_SINGLE			 0x01
#define DLP_TYPE_DUAL			 0x02
#define DLP_TYPE_TALLER			 0x04
#define DLP_TYPE_TALLER_2		 0x04
#define DLP_TYPE_TALLER_4		 0x08

#define GPU_MAX_CNT 4
#define LANE_MAX_CNT 2

#define MAX_BUCKETS	4

#ifdef _PZMAP_LEGACY_
#define NUM_SW	16			// single wavelength channel number
#else
#define NUM_SW	18			// single wavelength channel number
#endif
#define NUM_SW_FINE 8
#define NUM_SW_REDUCE 4

#define NUM_DW	12			// dual wavelength channel number
#define NUM_DW_FINE		4	// yjs 2019/01/17
#define NUM_DW_REDUCE	2

#define WIDE_RW	1000
#define WIDE_RH	1000
#define NUM_ROI 50
#define WIDTH_SECTION 400
#define FOREIGN_SECTION 500

#define Zero_flag_ 0xff
//#define Solder_flag_ 0x04

#define PI		3.141592f
#define PI_i    31416
#define PI2		6.283184f
#define PI2_i   62832
#define PI4		12.566368f

#ifndef HPIN
	#define HPIN	-(PI/2.f)
#endif
#ifndef HPIP
	#define HPIP	(PI/2.f)
#endif

#define _abs_value(_val)	((_val > 0) ? _val : -(_val));
#define _limit_phase(_phase, _min, _max, _min_offset, _max_offset)	((_phase > _max) ? (_phase+_max_offset) : ((_phase < _min) ? (_phase+_min_offset) : _phase))
#define	_limit_value(_val, _min, _max)	(_val < _min) ? _min : ((_val > _max) ? _max : _val)
#define _fufd_round(_val)	(_val >= 0) ? ((int)(_val + 0.5f)) : ((int)(_val - 0.5f))
//#define _fufd_round(_val)	(_val >= 0) ? (int)floor(_val) : (int)ceil(_val)

#define _modulation(_val, _min, _max)	((_val <= _min) ? 0 : ((_val >= _max) ? 0 : 1))
#define	_average_3(i1, i2, i3)	((float)(i1 + i2 + i3) / 3.f)
#define	_modulation_3(i1, i2, i3)	(fabs((float)(i1 - i2)) + fabs((float)(i2 - i3)) + fabs((float)(i1 - i3)))
#define	_average_4(i1, i2, i3, i4)	((float)(i1 + i2 + i3 + i4) / 4.f)
#define	_modulation_4(i1, i2, i3, i4)	(fabs((float)(i1 - i2)) + fabs((float)(i2 - i3)) + fabs((float)(i3 - i4)) + fabs((float)(i1 - i4)) + fabs((float)(i2 - i4)) + fabs((float)(i3 - i4)))

#define _threshold_upper(_val, _std)	((_val > _std) ? true : false)
#define _threshold_lower(_val, _std)	((_val < _std) ? true : false)
#define _threshold_inner(_val, _min, _max)	((_val < _min) ? false : ((_val > _max) ? false : true))
#define _threshold_outer(_val, _min, _max)	((_val > _min) ? false : ((_val < _max) ? false : true))

#define _matching_percent(_val)	((1.f - (_val * 0.5f)) * 100.f)

#define _correct_dx(_height, _std, _corrector)	(_height < _std) ? (_height + _corrector) : _height;
#define _IsAnyAngle(angle)	(((int)angle % 90 != 0) || (angle - ((int)angle / 90.0) * 90 ) != 0) ? true : false

#pragma region arc_tangent_lookup_table

// 3 bucket( theta: 0 / 120 / 240 )	
// 			SQRT3*(I2 - I3)
// 	atan2(	--------------- )
// 			2*I1 - I2 - I3
#define SQRT3			1.7320508f			// square root 3 =(sqrt(3.0))
#define ATL_B3_DI2		(1)					// denominator 1th coefficient of atan2
#define ATL_B3_DI3		(-1)				// denominator 2nd coefficient of atan2
#define ATL_B3_DPL		( 255 * 1 )			// positive side denominator length of atan2
#define ATL_B3_DNL		( 255 * 1 )			// negative side denominator length of atan2
#define ATL_B3_DL		( ATL_B3_DPL + ATL_B3_DNL + 1 )		// denominator length of atan2
#define ATL_B3_NI1		(2)					// numerator 1th coefficient of atan2
#define ATL_B3_NI2		(-1)				// numerator 2nd coefficient of atan2
#define ATL_B3_NI3		(-1)				// numerator 3rd coefficient of atan2
#define ATL_B3_NPL		( 255 * 2 )			// positive side numerator length of atan2
#define ATL_B3_NNL		( 255 * (1+1) )		// negative side numerator length of atan2
#define ATL_B3_NL		( ATL_B3_NPL + ATL_B3_NNL + 1 )		// numerator length of atan2

// 4 bucket ( theta: 0 / 90 / 180 / 270 )
// 			I2 - I4
// 	atan2(	------- )
// 			I1 - I3
#define ATL_B4_DPL		(255)
#define ATL_B4_DNL		(255)
#define ATL_B4_DL		( ATL_B4_DPL + ATL_B4_DNL + 1)
#define ATL_B4_NPL		(255)
#define ATL_B4_NNL		(255)
#define ATL_B4_NL		( ATL_B4_NPL + ATL_B4_NNL + 1)

#pragma endregion arc_tangent_lookup_table

#pragma region abs_lookup_table

#define ABSL_PL		(255)
#define ABSL_NL		(255)
#define ABSL_LEN	( ABSL_NL + ABSL_PL + 1 )

#pragma endregion abs_lookup_table

#pragma region bottom_zone

#define BZ1TH_DNUM	4
#define BZ2ND_SIZE	1500

#pragma endregion bottom_zone

#pragma region color_map

#define SIZE_CLRX	(1000)	// (CIE)	// 256 (RB)
#define SIZE_CLRY	(1000)

// CIE model - RGB to XYZ
#define CIE_XR	(0.490f)
#define CIE_XG	(0.310f)
#define CIE_XB	(0.200f)
#define CIE_YR	(0.177f)
#define CIE_YG	(0.813f)
#define CIE_YB	(0.011f)
#define CIE_ZR	(0.000f)
#define CIE_ZG	(0.010f)
#define CIE_ZB	(0.990f)

#define	LEN_CLR_THRES	256

#pragma endregion color_map

#define SDW_STAIRS	3


typedef enum _ZmapCalculateMode
{
	cm_normal = 0,
	cm_full,
	cm_calib,
	cm_auto,
	cm_normal_off,
	cm_auto_off,
	cm_auto_calib,
	cm_auto_pzt,
	cm_auto_calib_Direction,
	cm_auto_calib_Lambda_Proj,
	cm_auto_calib_LambdaCoe_Proj,
	cm_auto_calib_Lambda_DLP2,
	cm_auto_calib_LambdaCoe_DLP2,
	cm_auto_calib_Lambda_Taller,
	cm_auto_calib_LambdaCoe_Taller,
	cm_auto_calib_ref,
	cm_calib_report,
	cm_debug_save
}ZmapCalcMode;

typedef enum _ZmapForeignCalculateMode
{
	fcm_none = 0,
	fcm_full,
	fcm_only_flag,
} ZmapForeignCalculateMode;

typedef enum _ZmapBuckets
{
	bucket_3	=3,
	bucket_4	=4
}ZmapBuckets;

typedef enum _ZmapFilterMaskMedian
{
	maskMedian3x3	=3,
	maskMedian5x5	=5,
	maskMedian7x7	=7
}MaskMedian;

typedef enum _ZmapFilterMaskGaussian
{
	maskGauss3x3	=3,
	maskGauss5x5	=5
}MaskGauss;

typedef enum _ZmapWavelengthType
{
	zmap_ws	=0,
	zmap_wd	=1
}WaveType;

typedef enum _ZmapThresholdType
{
	thres_upper	=0,
	thres_lower	,
	thres_inner	,
	thres_outer
}ThresType;

typedef struct stZmapParamInit
{
	int VersionA;
	int VersionB;
	int VersionD;

	bool w1bw2;
	bool IsDualLane;
	bool IsRefAlloc;
	ZmapBuckets bucket;
	int lenX, lenY;
	int nProjType;
	int TypeDLP;
	int numPZT;
	int nGpuCnt;
	int* GpuIdx;
	double pxlResX, pxlResY;
	int IGBT_FilterSize;
	int etcFlag;

	stZmapParamInit()
	{
		VersionA	 = -1;
		VersionB	 = -1;
		VersionD	 = -1;

		w1bw2		 = false;
		IsDualLane	 = false;
		IsRefAlloc	 = false;
		bucket		 = bucket_4;
		lenX		 = 0;
		lenY		 = 0;
		nProjType	 = PROJ_TYPE_ORIGINAL;
		TypeDLP		 = 0x00;
		numPZT		 = 2;
		nGpuCnt		 = 0;
		GpuIdx		 = nullptr;
		pxlResX		 = 0;
		pxlResY		 = 0;
		IGBT_FilterSize = 0;
		etcFlag		 = 0;
	}
}STZPINIT, *LPSTZPINIT;

typedef struct stZmapParamCalibLamdaCxCy
{
	// input parameter
	float	stdHeightX1;
	float	stdHeightX2;
	float	stdHeightY1;
	float	stdHeightY2;
	float	curHeightX1;
	float	curHeightX2;
	float	curHeightY1;
	float	curHeightY2;
	float	posX1;
	float	posX2;
	float	posY1;
	float	posY2;

	// output parameter
	float*	resCx;
	float*	resCy;

	stZmapParamCalibLamdaCxCy()
	{
		stdHeightX1 = 0.f;
		stdHeightX2 = 0.f;
		stdHeightY1 = 0.f;
		stdHeightY2 = 0.f;
		curHeightX1 = 0.f;
		curHeightX2 = 0.f;
		curHeightY1 = 0.f;
		curHeightY2 = 0.f;
		posX1 = 0.f;
		posX2 = 0.f;
		posY1 = 0.f;
		posY2 = 0.f;

		resCx = NULL;
		resCy = NULL;
	}
}STZPCALIBCXCY, *LPSTZPCALIBCXCY;

typedef struct stZmapParamPhaseOffset
{
	float	s[NUM_SW];
	float	d[NUM_DW];

	stZmapParamPhaseOffset()
	{
		for (int i=0 ; i<NUM_SW ; i++)
		{
			s[i] = 0.f;
		}
		for (int i=0 ; i<NUM_DW ; i++)
		{
			d[i] = 0.f;
		}
	}
}STZPPOFFSET, *LPSTZPPOFFSET;

typedef struct stZmapParamShadowCondition
{
	int stairs;
	float irange[SDW_STAIRS];
	float modulation[SDW_STAIRS];
	int pass_count[SDW_STAIRS];

	stZmapParamShadowCondition()
	{
		stairs = SDW_STAIRS;
		for (int i=0 ; i<stairs ; i++)
		{
			irange[i] = 0.f;
			modulation[i] = 0.f;
			pass_count[i] = 0;
		}
	}

}STZPSDWC, *LPSTZPSDWC;

#define ZMAPFOV_TYPE_FOREIGN_TALL			 0x0001
#define ZMAPFOV_TYPE_CALC_WARPAGE			 0x0002
#define ZMAPFOV_TYPE_ALL_PROJENTION			 0x0004
#define ZMAPFOV_TYPE_FOREIGN_PART			 0x0008
#define ZMAPFOV_TYPE_JIG					 0x0010
#define ZMAPFOV_TYPE_FOREIGN_WITH_WARPAGE	 0x0020
#define ZMAPFOV_TYPE_PART_FOREIGN_INSP		 0x0040
#define ZMAPFOV_TYPE_CALC_FULLFORIEIGN		 0x0080
#define ZMAPFOV_TYPE_FLEXIBLE_BOARD			 0x0100
#define ZMAPFOV_TYPE_BTM_SIGNAL_STRENGTH	 0x0200
#define ZMAPFOV_TYPE_GAP					 0x0400
#define ZMAPFOV_TYPE_FOREIGN_QUARTER		 0x0800
#define ZMAPFOV_TYPE_INTERPOSER				 0x1000
#define ZMAPFOV_TYPE_FOREIGN_MODULE			 0x2000
#define ZMAPFOV_TYPE_LOWBTMPASSED			 0x4000
#define ZMAPFOV_TYPE_MULTICOLOR_OFFSET		 0x8000

#pragma region ZMROI_btTypeInsp

#define ZMAPROI_TYPE_EABLE					 0x01
#define ZMAPROI_TYPE_ENABLE					 0x01
#define ZMAPROI_TYPE_DUMMY					 0x02
#define ZMAPROI_TYPE_TALLER					 0x04
#define ZMAPROI_TYPE_TALLEST				 0x08
#define ZMAPROI_TYPE_BLACK					 0x010
#define ZMAPROI_TYPE_LEAD					 0x020					 // over lighting solder
#define ZMAPROI_TYPE_WIDE					 0x040
#define ZMAPROI_TYPE_ALLDUALMIX				 0x080
#define ZMAPROI_TYPE_GAP					 0x100
#define ZMAPROI_TYPE_ZOffset				 0x200
#define ZMAPROI_TYPE_ZeroSurface			 0x400
#define ZMAPROI_TYPE_LongPartUser			 0x800
#define ZMAPROI_TYPE_ADVANCED_LEAD			 0x1000
#define ZMAPROI_TYPE_NARROW_PART			 0x2000
#define ZMAPROI_TYPE_PLANARIZATION			 0x4000
#define ZMAPROI_TYPE_FILLAROUND				 0x8000
#define ZMAPROI_TYPE_PRECISE_TALLER			 0X10000
#define ZMAPROI_TYPE_ZShortOffset			 0X20000
#define ZMAPROI_TYPE_SMOOTH					 0X40000
#define ZMAPROI_TYPE_BIG_PART				 0x80000
#define ZMAPROI_TYPE_ROUND_FILTER			 0x100000
#define ZMAPROI_TYPE_ZMULTIPLEOFFSET		 0x200000
#define ZMAPROI_TYPE_LEAD_BRIDGE			 0x400000
#define ZMAPROI_TYPE_LEAD_FRAME				 0x800000
#define ZMAPROI_TYPE_DLP_ONLY				 0x1000000				 // DLP 가 없을 경우 또는 DLP 유무를 모를 때, Only Taller
#define ZMAPROI_TYPE_LOW_RESOLUTION			 0x2000000
#define ZMAPROI_TYPE_BALL					 0x4000000
#define ZMAPROI_TYPE_PART_QUARTER			 0x8000000				
#define ZMAPROI_TYPE_BIG_PART_FOREIGN		 0x10000000
#define ZMAPROI_TYPE_CHIP_DIVISION			 0x20000000
#define ZMAPROI_TYPE_ZERO_PART				 0x40000000

#define ZMAPROI_TYPE_GROUND_LEVEL			 0x0200000000000000
#define ZMAPROI_TYPE_CUT_HEIGHT				 0x0400000000000000
#define ZMAPROI_TYPE_ZOFFSET_MANUAL			 0x0800000000000000
#define ZMAPROI_TYPE_LEAD_SHOULDER_FILTER	 0x1000000000000000
#define ZMAPROI_TYPE_MANUAL_PROJ			 0x2000000000000000
#define ZMAPROI_TYPE_BOTTOM_COLOR			 0x4000000000000000
#define ZMAPROI_TYPE_BOTTOM					 0x8000000000000000

#define ZMAPROI_TYPE_LOW_PROJ				 0x04
#define ZMAPROI_TYPE_HIGH_PROJ				 0x08

#define ZMAPROI_TYPE_SPEEDMODE				 0x000
#pragma endregion ZMROI_btTypeInsp

typedef enum _ZmapQualitySpeedType    // 
{
	qs_HA			 = 0,					//  8Projection 4버킷
	qs_Speed		 = 1,					//  4Projection 4버킷
	qs_Speed3Buk	 = 2,					//  4Projection 3버킷
	qs_Normal3Buk	 = 3,					//  8Projection 3버킷
	qs_Speed8Way	 = 4,					// (8 + x) Projection 4 bucket
	qs_Speed8Way3Buk = 5					// (8 + x) Projection 3 bucket
}ZmapQualitySpeedType;

#pragma region ZMROI_roiTypeInsp

#define ZMAPROI_TYPE_THUR					 0x00000001
#define ZMAPROI_TYPE_PIN					 0x00000002
#define ZMAPROI_TYPE_BlackBody				 0x00000004
#define ZMAPROI_TYPE_RETOUCH				 0x00000008
#define ZMAPROI_TYPE_LED					 0x00000010
#define ZMAPROI_TYPE_WIRE					 0x00000020
#define ZMAPROI_TYPE_SMALLPARTS				 0x00000040
#define ZMAPROI_TYPE_CIRCLEPARTS			 0x00000080
#define ZMAPROI_TYPE_HIGHDENSITY_GENERAL	 0x00000100
#define ZMAPROI_TYPE_COINING				 0x00000200
#define ZMAPROI_TYPE_SPARKLINGCHIP			 0x00000400
#define ZMAPROI_TYPE_COMPLEXSURFACE			 0x00000800
#define ZMAPROI_TYPE_WHITESOCKET			 0x00001000
#define ZMAPROI_TYPE_MICROLED				 0x00002000
#define ZMAPROI_TYPE_TILT					 0x00004000
#define ZMAPROI_TYPE_PART_WARPAGE			 0x00008000
#define ZMAPROI_TYPE_CALC_CENTER_WARPAGE	 0x00010000
#define ZMAPROI_TYPE_OBSCURED_BOTTOM		 0x00020000
#define ZMAPROI_TYPE_PRIORITY_TALLER		 0x00040000
#define ZMAPROI_TYPE_GLUE					 0x00080000
#define ZMAPROI_TYPE_NARROW_INTERVAL		 0x00100000
#define ZMAPROI_TYPE_FIXED_BOTTOM			 0x00200000
#define ZMAPROI_TYPE_RECOVER				 0x00400000
#define ZMAPROI_TYPE_SOLDER_RETOUCH			 0x00800000
#define ZMAPROI_TYPE_EXCLUDE_PROJ			 0X01000000
#define ZMAPROI_TYPE_CHIP_SOLDER			 0X02000000
#define ZMAPROI_TYPE_NICKEL					 0X04000000
#define ZMAPROI_TYPE_SHIELDCAN				 0X08000000
#define ZMAPROI_TYPE_SOLDER_RECOVER			 0x10000000
#define ZMAPROI_TYPE_TALLEST_SHADOW			 0x20000000
#define ZMAPROI_TYPE_WIRE_DIVISION			 0x40000000


#define ZMAPROI_TYPE_RETOUCH_TYPE2			 0x80000000	 // 임시 명칭

#pragma endregion ZMROI_roiTypeInsp

#pragma region ZMAP_etcFlag

#define ZMAP_ETC_DUALLANE			0x001
#define ZMAP_ETC_PZT4				0x002
#define ZMAP_ETC_PZT8				0x004
#define ZMAP_ETC_REF_ALLOC			0x008
#define ZMAP_ETC_Apllon1stBtm		0x010
#define ZMAP_ETC_Moi_Machine		0x020
#define ZMAP_ETC_GPU_IMG_Buff_Alloc	0x040
#define ZMAP_ETC_SMA				0x080
#define ZMAP_ETC_SAMSUNG			0X100

#pragma endregion ZMAP_etcFlag

typedef struct stZmapGROI
{
	RECT	groupROI;
	int		groupIndex;
	int		groupPartNum;

	BYTE * pSaveBuffer;
	unsigned int szSaveBuf;

	stZmapGROI()
	{
		groupROI.left = 0;
		groupROI.top = 0;
		groupROI.right = 0;
		groupROI.bottom = 0;

		groupIndex = -1;
		groupPartNum = 0;
		pSaveBuffer = nullptr;
		szSaveBuf = 0;
	}
}ZMGROUPROI, *LPZMGROUPROI;

typedef struct stZmapBottomROI
{
	bool bIsReference;
	RECT ROI;
	float Surf_Coef[NUM_SW][3];
	float Surf_Offset[NUM_SW];
	BYTE	btCal3D;
	int		btTypeInsp;
	float * result_buffer;

	stZmapBottomROI()
	{
		bIsReference = false;

		ROI.left = 0;
		ROI.top = 0;
		ROI.right = 0;
		ROI.bottom = 0;

		for ( int i = 0 ; i < NUM_SW ; i++)
			for ( int j = 0 ; j < 3 ; j++)
				Surf_Coef[i][j] = 0.f;

		for ( int i = 0 ; i < NUM_SW ; i++)Surf_Offset[i] = 0;

		btTypeInsp = 0;
		result_buffer = nullptr; 
	}
}ZMBTMROI, *LPZMBTMROI;

typedef struct stZmapBTM
{
	int btmPass[NUM_SW*10];
	float Surf_Coef[NUM_SW][30];
	int Surf_Offset[NUM_SW*10];

	stZmapBTM()
	{
		for ( int i = 0 ; i < NUM_SW ; i++)
			for ( int j = 0 ; j < 30 ; j++)
				Surf_Coef[i][j] = 0.f;

		for ( int i = 0 ; i < NUM_SW*10 ; i++)
		{
			btmPass[i] = 0;
			Surf_Offset[i] = 0;
		}
	}
}ZMBTM, *LPZMBTM;

typedef struct stPolyPoint
{
	bool	bPolygon;
	int cntPt;
	float x[20];
	float y[20];

	stPolyPoint()
	{
		bPolygon = false;
		cntPt = 0;
		for (int i=0; i<20; i++)
		{
			x[i]=0.f;
			y[i]=0.f;
		}
	}
}ZMPOLYPOINT, *LPZMPOLYPOINT;

#define MAX_POLYGON_POINT 60

typedef struct stPolyPointUltra
{
	bool	bPolygon;
	int cntPt;
	float x[MAX_POLYGON_POINT];
	float y[MAX_POLYGON_POINT];

	stPolyPointUltra()
	{
		bPolygon = false;
		cntPt = 0;
		for (int i = 0; i < MAX_POLYGON_POINT; i++)
		{
			x[i] = 0.f;
			y[i] = 0.f;
		}
	}
}ZMPOLYPOINTULTRA, *LPZMPOLYPOINTULTRA;
typedef struct stZmapROI
{
	RECT	ROI;
	RECT BodyROI;
	//RECT orgROI;			// in the case that FOV(s) are merged; the position in original FOV (Due to Reference)
	float Surf_Coef[NUM_SW][3];
	float Surf_Offset[NUM_SW];
	int		idxColorXY;
	int offsetlimit_X,offsetlimit_Y;
	int PartIdx;
	int ModuleIdx;
	int Tallest_Height;
	int RoiLength, RoiWidth;
	//bool Flag_BodyRoi;
	int		btTypeInspOld;		//이 변수는 더이상 사용하지 않습니다.
	int		roiTypeInspOld;		//이 변수는 더이상 사용하지 않습니다.

	__int64		btTypeInsp;
	__int64		roiTypeInsp;  // ROI Type :   0x00 - default, 0x01 - THUR HOLE option
	
	BYTE	btCal3D;		// 3D 계산 전 : 0, 3D 계산 후 : 1
	BYTE	btCalForeign;		// 이물 3D 계산 전 : 0, 계산 후 : 1
	//BYTE	btNoFilter;		// 0x01:left, 0x02:top, 0x04:right, 0x08:bottom
	float	PartAngle;
	float	BodyHeight;

	bool bOutputPartInfo;
	int partID;

	int		nGroupIndex;
	BYTE 	btQnSType;     // 4/8 PROJECTION & 3/4 BUCKIT FLAG 2018.05.11 BY Lee. C. J.

	BYTE	BlackBodyLevel;

	// advanced Mode
	BOOL	adv_LeadSolderFilter;
	BOOL	adv_blackhole;
	BOOL	adv_gap;
	BOOL	adv_smallPart;
	BOOL	adv_colorXY;
	BOOL	adv_LPartFewBtm;
	BOOL	adv_preFilter;
	BOOL	adv_other;
	BOOL	adv_blackbody;

	float * result_buffer;
	int btmPass[NUM_SW*10];
	float btmCoef[NUM_SW][30];
	int btmOffset[NUM_SW*10];
	float slope_x[NUM_SW*10];
	float slope_y[NUM_SW*10];
	int btmPass2nd[NUM_SW*10];

	BOOL save_buffer_flag;
	BYTE * pSaveBuffer;
	unsigned int szSaveBuf;

	BOOL bUseVirtualBottom;
	float fHeightOffset;

	unsigned short nSelectProjection;
	int PartStep;
	float fResultPartOffset;	 // 파트 자세 계산 후 offset 결과
	float PTallerLB;			 // Priority Taller Lower Bound
	float PartOffsetPre;

	stZmapROI()
	{
		ROI.left = 0;
		ROI.top = 0;
		ROI.right = 0;
		ROI.bottom = 0;		

		BodyROI.left = 0;
		BodyROI.top = 0;
		BodyROI.right = 0;
		BodyROI.bottom = 0;

		/*orgROI.left = 0;
		orgROI.top = 0;
		orgROI.right = 0;
		orgROI.bottom = 0;*/

		idxColorXY = 0;
		btTypeInsp = ZMAPROI_TYPE_ENABLE;
		roiTypeInsp = 0x00;
		offsetlimit_X = 40;
		offsetlimit_Y = 40;
		PartIdx = -1;
		ModuleIdx = 0;
		PartAngle = 0.f;    // @angle
		Tallest_Height = 1300;
		BodyHeight = 0.f;
		RoiLength = 0;
		RoiWidth = 0;

		bOutputPartInfo = false;
		partID = 0;
		btCal3D = 0;
		btCalForeign = 0;

		nGroupIndex = -1;
		for ( int i = 0 ; i < NUM_SW ; i++)
			for ( int j = 0 ; j < 3 ; j++)
				Surf_Coef[i][j] = 0.f;

		for ( int i = 0 ; i < NUM_SW ; i++)Surf_Offset[i] = 0;

		btQnSType = qs_HA;

		BlackBodyLevel = 15;

		adv_LeadSolderFilter = true;
		adv_blackhole = true;
		adv_gap = true;
		adv_smallPart = true;
		adv_colorXY = true;
		adv_LPartFewBtm = true;
		adv_preFilter = true;
		adv_other = true;
		adv_blackbody = false;

		result_buffer = nullptr;

		save_buffer_flag = FALSE;
		pSaveBuffer = nullptr;
		szSaveBuf = 0;

		bUseVirtualBottom = FALSE;

		fHeightOffset = 0.f;

		for ( int i = 0 ; i < NUM_SW ; i++)
			for ( int j = 0 ; j < 30 ; j++)
				btmCoef[i][j] = 0.f;

		for ( int i = 0 ; i < NUM_SW*10 ; i++)
		{
			btmPass[i] = 0;
			btmOffset[i] = 0;
			slope_x[i] = 0.f;
			slope_y[i] = 0.f;
			btmPass2nd[i] = 0;
		}

		nSelectProjection = 0xffff;

		PartStep = 0x00010001;
		fResultPartOffset = 0.f;
		PTallerLB = 3500.f;
		PartOffsetPre = 0.f;
	}
}ZMROI, *LPZMROI;

#define ZMAPEXCEPTROI_EABLE				 0x00000001
#define ZMAPEXCEPTROI_MOUNT				 0x00000002
#define ZMAPEXCEPTROI_SOLDER			 0x00000004
#define ZMAPEXCEPTROI_LEAD				 0x00000008
#define ZMAPEXCEPTROI_BLACKBODY			 0x00000010
#define ZMAPEXCEPTROI_BODY				 0x00000020
#define ZMAPEXCEPTROI_GAP				 0x00000040
#define ZMAPEXCEPTROI_THUR				 0x00000080
#define ZMAPEXCEPTROI_RETOUCH			 0x00000100
#define ZMAPEXCEPTROI_GAPBLACKSURFACE	 0x00000200
#define ZMAPEXCEPTROI_BOTTOM			 0x00000400
#define ZMAPEXCEPTROI_SOLDER_STDROI		 0x00000800
#define ZMAPEXCEPTROI_NO_TALLER			 0x00001000
#define ZMAPEXCEPTROI_BTM_COLOR			 0x00002000
#define ZMAPEXCEPTROI_PLANARIZATION		 0x00004000
#define ZMAPEXCEPTROI_WIRE_BLACK		 0x00010000
#define ZMAPEXCEPTROI_WIRE_BLACKSTDAREA	 0x00020000
#define ZMAPEXCEPTROI_INSENSITIVE		 0x00040000
#define ZMAPEXCEPTROI_COMPLEXSURFACE	 0x00080000
#define ZMAPEXCEPTROI_SPARKLINGCHIP		 0x00100000
#define ZMAPEXCEPTROI_LOWSIGNAL			 0x00200000
#define ZMAPEXCEPTROI_LOWER				 0x00400000
#define ZMAPEXCEPTROI_ZOffset			 0x00800000
#define ZMAPEXCEPTROI_IGBT				 0x01000000
#define ZMAPEXCEPTROI_GROUND_LEVEL		 0x02000000
#define ZMAPEXCEPTROI_PRECISE_TALLER	 0x04000000

#define ZMAPEXCEPTROI_OVERLIGHTNG_SOLDER 0x80000000

enum ZmapDirection
{
	eDirLEFT = 0,
	eDirRIGHT ,
	eDirTOP ,
	eDirBOTTOM 
};

#define ZMAPFOREIGN_EXCEPTROI_SUBPCB		 0x01
#define ZMAPFOREIGN_EXCEPTROI_INTERPOSER	 0x02

#define ZMAP_HITMODULETYPE_MODULE_AREA		 0x01

#define ZMAP_FOREIGN_FLAG_FOREIGNEXCEPT		 0x0001
#define ZMAP_FOREIGN_FLAG_PARTEXCEPT		 0x0002
#define ZMAP_FOREIGN_FLAG_BOTTOMCOLOR		 0x0004
#define ZMAP_FOREIGN_FLAG_IM_AM				 0x0008
#define ZMAP_FOREIGN_FLAG_HOLE				 0x0010
#define ZMAP_FOREIGN_FLAG_STICKER			 0x0020

typedef struct stZmapExceptROI
{
	RECT	exceptROI;
	int		PartIndex;	
	int		ModuleIndex;
	float	m_nPartAngle;
	int		btTypeInsp;			//ZMAPEXCEPTROI_ // yjs 2019/01/17 change type -> int
	int		nLeadTipDirect;		//ZmapDirection

	ZMPOLYPOINT		PolygonPoint;

	unsigned short nSelectProj;

	stZmapExceptROI()
	{
		exceptROI.left = 0;
		exceptROI.top = 0;
		exceptROI.right = 0;
		exceptROI.bottom = 0;		

		PartIndex = -1;
		ModuleIndex = 0;
		m_nPartAngle = 0;
		btTypeInsp = 0x00;
		nLeadTipDirect = -1;
		nSelectProj = 0xffff;
	}
}ZMEXCEPTROI, *LPZMEXCEPTROI;

#define ZMAP_HITMODULE_FLAG_SKIP			 0x0001

typedef struct stZmapHitModuleROI
{
	RECT	 ROI;
	int		 ModuleIndex;
	float	 ModuleAngle;
	int		 ModuleFlag; // 미래에 사용할 수도 있는 변수

	ZMPOLYPOINTULTRA PolygonPoint;

	stZmapHitModuleROI()
	{
		ROI.left = 0;
		ROI.top = 0;
		ROI.right = 0;
		ROI.bottom = 0;

		ModuleIndex = 0;
		ModuleAngle = 0;
		ModuleFlag = 0x00;
	}
}STZMHITMODLEROI, *LPSTZMHITMODLEROI;

typedef struct stHighDensityAreaThresholdType
{
	bool enbThreshold;
	int type;
	UCHAR threshold1;
	UCHAR threshold2;

	stHighDensityAreaThresholdType()
	{
		enbThreshold = false;
		type = -1;
		threshold1 = 0;
		threshold2 = 255;
	}
}STHTHRESHOLDTYPE;

typedef struct stZmapParamGenerateZmap
{
	ZmapCalcMode cm; 

	BYTE*	img[NUM_SW][MAX_BUCKETS];
	BYTE*	imgR;
	BYTE*	imgG;
	BYTE*	imgB;
	BYTE*	imgBI;	// bottom illumination
	BYTE*	imgbR;
	BYTE*	imgmR;
	BYTE*	imgmB;
	BYTE*	imgTw;
	float	fitR;
	float	fitG;
	float	fitB;

	STZPSDWC c;         // modulation factor
	
	STZPPOFFSET po;     //phase offset

	float	fillShadow;
	float   Zoffset;
	bool	enbBtc;
	bool	ChildFovFlag;
	int		btcOrder;
	int		btcStepX;
	int		btcStepY;
	int		numBTC;

	int		numExceptROI;
	ZMEXCEPTROI*	exceptROIs;

	int		numExceptSubROI;
	ZMEXCEPTROI*	exceptSubROIs;			// 일단은 Solder정보만 추가

	int		numROI;
	ZMROI*	ROIs;
	int*	rstGenerateROIs;

	BYTE bInspforeign;			// 이물 검사 유무
	int		numWndROI;		// FOV내 Window 개수
	ZMEXCEPTROI*	wndROIs;				// FOV내 Window ROI List (FOV에 걸친 Window 포함)


	int		numWndROIGen;					// FOV내 Window 개수 일반검사용도
	ZMEXCEPTROI*	wndROIsGen;				// FOV내 Window ROI List (FOV에 걸친 Window 미포함) 일반검사용도

	int numHole;			// FOV내 Hole 개수 (이물 검사 용도, FOV에 걸친 Hole 포함)
	RECT* holeROIs;			// FOV내 Hole ROI List (이물 검사 용도, FOV에 걸친 Hole 포함)

	POINT positionFOV;		// PCB 내 FOV Center 위치 (기준점 : PCB 좌하단)  pixel좌표계 
	SIZE sizePCB;			// PCB Size pixel좌표계 
	SIZE sizeFOV;			// FOV Size pixel좌표계 
	RECT exceptBoardSide;	// 클램프 영역 제외 사이즈
							// 2021/10/01 : SIZE -> RECT 변경
		
	int		numHitModuleROI;		// FOV내에 겹친 Module 개수
	ZMEXCEPTROI*	 hitModuleROIs_Legacy;	 // 사용 중지 예정
	STZMHITMODLEROI* hitModuleROIs;			 // 이물검사 히트모듈 리스트

	bool bExceptBTC_F; // 이물검사시 바닥컬러 제외 여부 (true:제외)

	bool bAdvancedBTC;

	POINT mergeXY;

	int		numGroupROI;
	ZMGROUPROI*	groupROIs;
	int		numSoloParts;

	unsigned __int64 * dst_offset;

	int		numBottomROI;		// the number of Bottom ROI (for inspection of flatness)
	ZMBTMROI*	bottomROIs;		// Bottom ROI List (for inspection of flatness)

	int fov_QnsType;	// YJS 2019/05/07 : default=qs_HA
	int nFOVOption_3D;

	float fBoardOffset;			 // KHJ 2020/12/03 : FOV의 측정 범위 조절 변수
	int ForeignStep;
	int BtmSignalStrength;

	int fov_ModuleIdx;

	bool adv_LeadSolderFilter;
	bool adv_blackhole;
	bool adv_gap;
	bool adv_smallPart;
	bool adv_colorXY;		//multipolygon & threshold
	bool adv_LPartFewBtm;
	bool adv_preFilter;
	bool adv_other;			//offline(merge), ptt(randBtm)
	float RefZoffset; // 레퍼런스 면 대비 z축 높이

	RECT Margin;	// body window margin

	float WarpageSearchRange;

	stZmapParamGenerateZmap()
	{
		fov_QnsType = qs_HA;
		cm = cm_normal;

		for (int i=0 ; i<NUM_SW ; i++)
		{
			for (int j=0 ; j<MAX_BUCKETS ; j++)
			{
				img[i][j] = NULL;
			}
		}
		imgR = NULL;
		imgG = NULL;
		imgB = NULL;
		imgBI = NULL;
		imgbR = NULL;
		imgmR = NULL;
		imgmB = NULL;
		imgTw = NULL;
		fitR = 1.f;
		fitG = 1.f;
		fitB = 1.f;
		
		Zoffset = 0.0f;
		ChildFovFlag = false;
		fillShadow = 0.f;
		enbBtc = true;
		btcOrder = 2;
		btcStepX = 4;
		btcStepY = 4;
		numBTC = 100;

		numExceptROI = 0;
		exceptROIs = NULL;

		numExceptSubROI = 0;
		exceptSubROIs = NULL;

		numROI = 0;
		ROIs = NULL;
		rstGenerateROIs = NULL;

		bInspforeign = FALSE;
		numWndROI = 0;
		wndROIs = NULL;
		numWndROIGen =0;

		numHole = 0;
		wndROIsGen = NULL;

		sizePCB.cx = 0;			// PCB Size pixel좌표계 
		sizePCB.cy = 0;
		sizeFOV.cx = 0;			// FOV Size pixel좌표계 
		sizeFOV.cy = 0;
		positionFOV.x = 0;
		positionFOV.y = 0;
		exceptBoardSide.left = 5;
		exceptBoardSide.top = 10;
		exceptBoardSide.right = 5;
		exceptBoardSide.bottom = 10;

		numHitModuleROI = 0;
		hitModuleROIs_Legacy = NULL;
		hitModuleROIs = NULL;

		bExceptBTC_F = false;

		bAdvancedBTC = true;

		mergeXY.x = 0;
		mergeXY.y = 0;

		numGroupROI = 0;
		groupROIs = NULL;
		numSoloParts = 0;

		dst_offset = nullptr;

		numBottomROI = 0;
		bottomROIs = NULL;
		nFOVOption_3D = 0;

		fBoardOffset = 0.f;

		ForeignStep = 0x00020002;
		BtmSignalStrength = 0;

		fov_ModuleIdx = 0;

		adv_LeadSolderFilter = true;
		adv_blackhole = true;
		adv_gap = true;
		adv_smallPart = true;
		adv_colorXY = true;			//multipolygon & threshold
		adv_LPartFewBtm = true;
		adv_preFilter = true;
		adv_other = true;			//offline(merge), ptt(randBtm)
		RefZoffset = 0.f;

		Margin.left = 0;
		Margin.top = 0;
		Margin.right = 0;
		Margin.bottom = 0;

		WarpageSearchRange = 500.f;
	}
}STZPGENZMAP, *LPSTZPGENZMAP;

typedef struct stZmapParamGeneratePhaseReference
{
	int		cntBuk;

	BYTE*	img[NUM_SW][MAX_BUCKETS];
	
	stZmapParamGeneratePhaseReference()
	{
		cntBuk = 4;

		for (int i=0 ; i<NUM_SW ; i++)
		{
			for (int j=0 ; j<MAX_BUCKETS ; j++)
			{
				img[i][j] = NULL;
			}
		}
	}
}STPGETPHIREF, *LPSTPGETPHIREF;

typedef struct stZmapParamGenerateLamda
{
	float	lamda[NUM_SW];
	float	cx[NUM_SW];
	float	cy[NUM_SW];
	float	coeff[NUM_SW][10];

	bool NewCxCy;               // created at ver5.0 2017, 04, 24 ::  flag for new CxCy by CCH
	int Version;

	stZmapParamGenerateLamda()
	{
		NewCxCy = false;
		Version = 0;

		for (int i = 0; i < (NUM_SW); i++)
		{
			lamda[i] = 0.f;
			cx[i] = 0.f;
			cy[i] = 0.f;
		}

		for (int i = 0; i < (NUM_SW); i++)
		{
			for (int j = 0; j < 10; j++)
			{
				coeff[i][j] = 0.f;
			}
		}
	}
}STZPGENLAMDA, *LPSTZPGENLAMDA;

typedef struct stZmapParamGenerateLambda
{
	float	lambda[NUM_SW];
	float	cx[NUM_SW];
	float	cy[NUM_SW];
	float	coeff[NUM_SW][10];

	bool NewCxCy;               // created at ver5.0 2017, 04, 24 ::  flag for new CxCy by CCH
	int Version;

	stZmapParamGenerateLambda()
	{
		NewCxCy = false;
		Version = 0;

		for (int i = 0; i < (NUM_SW); i++)
		{
			lambda[i] = 0.f;
			cx[i] = 0.f;
			cy[i] = 0.f;
		}

		for (int i = 0; i < (NUM_SW); i++)
		{
			for (int j = 0; j < 10; j++)
			{
				coeff[i][j] = 0.f;
			}
		}
	}
}STZPGENLAMBDA, *LPSTZPGENLAMBDA;

typedef struct stZmapParamGetZmapS
{
	int fov;
	int sch;
	RECT roi;
	float *dst;
	int xIdxStDst;
	int yIdxStDst;
	int dstStride;
	
	stZmapParamGetZmapS()
	{
		fov = 0;
		sch = 0;
		roi.left = roi.right = roi.top = roi.bottom = 0;
		dst = NULL;
		xIdxStDst = 0;
		yIdxStDst = 0;
		dstStride = 0;
	}
}STZPGETZMAPS, *LPSTZPGETZMAPS;

typedef struct stZmapParamGetZmapD
{
	int fov;
	int dch;
	RECT roi;
	float *dst;
	int xIdxStDst;
	int yIdxStDst;
	int dstStride;

	stZmapParamGetZmapD()
	{
		fov = 0;
		dch = 0;
		roi.left = roi.right = roi.top = roi.bottom = 0;
		dst = NULL;
		xIdxStDst = 0;
		yIdxStDst = 0;
		dstStride = 0;
	}
}STZPGETZMAPD, *LPSTZPGETZMAPD;

typedef struct stZmapParamGetZmapSx
{
	int fov;
	RECT roi;
	float *dst;
	int xIdxStDst;
	int yIdxStDst;
	int dstStride;

	stZmapParamGetZmapSx()
	{
		fov = 0;
		roi.left = roi.right = roi.top = roi.bottom = 0;
		dst = NULL;
		xIdxStDst = 0;
		yIdxStDst = 0;
		dstStride = 0;
	}
}STZPGETZMAPSX, *LPSTZPGETZMAPSX;

typedef struct stZmapParamGetZmapDx
{
	int fov;
	RECT roi;
	float *dst;
	int xIdxStDst;
	int yIdxStDst;
	int dstStride;

	stZmapParamGetZmapDx()
	{
		fov = 0;
		roi.left = roi.right = roi.top = roi.bottom = 0;
		dst = NULL;
		xIdxStDst = 0;
		yIdxStDst = 0;
		dstStride = 0;
	}
}STZPGETZMAPDX, *LPSTZPGETZMAPDX;

typedef struct stZmapParamGetZmapMx
{
	int fov;
	RECT roi;
	float *dst;
	int xIdxStDst;
	int yIdxStDst;
	int dstStride;

	stZmapParamGetZmapMx()
	{
		fov = 0;
		roi.left = roi.right = roi.top = roi.bottom = 0;
		dst = NULL;
		xIdxStDst = 0;
		yIdxStDst = 0;
		dstStride = 0;
	}
}STZPGETZMAPMX, *LPSTZPGETZMAPMX;

typedef struct stZmapParamGenerateZmapCuda
{
	LPSTZPGENZMAP gz;

	// additional parameter
	float* zs[NUM_SW];
	float* zd[NUM_DW];
	float* sx;
	float* dx;
	float* mx;
	bool* btm;
	float* res;
	unsigned __int64 resBufSize;

	stZmapParamGenerateZmapCuda()
	{
		for (int i=0; i<NUM_SW; i++)
			zs[i] = nullptr;
		for (int i=0; i<NUM_DW; i++)
			zd[i] = nullptr;
		sx = nullptr;
		dx = nullptr;
		mx = nullptr;
		btm = nullptr;
		res = nullptr;
		resBufSize = 0;
	}

}STZPGENZMAPCU, *LPSTZPGENZMAPCU;

typedef struct stZmapParamBottomCorrector
{
	int order;	// maximum order is 4
	int numCoef;
	int offset;
	float offset_u;
	
	float coef[6 * 2];	// order:coefficients 1:3, 2:6, 3:10, 4:15

	stZmapParamBottomCorrector()
	{
		order = 0;
		numCoef = 0;
		offset = 0;
		offset_u = 0;
	
		for (int i = 0; i < 6 * 2; i++)
		{
			coef[i] = 0.f;
		}
	}

}STZPBTC, *LPSTZPBTC;

typedef struct stZmapParamMixDx
{
	int srcNum;
	float* dst;
	float* src[NUM_DW];
	BYTE* sdw[NUM_DW];
	float fillShadow;

	stZmapParamMixDx()
	{
		srcNum = NUM_DW;
		dst = NULL;
		for (int i=0 ; i<NUM_DW ; i++)
		{
			src[i] = NULL;
			sdw[i] = NULL;
		}
		fillShadow = 0.f;
	}

}STZPMIXDX, *LPSTZPMIXDX;

typedef struct stZmapParamMixSx
{
	int srcNum;
	float* dst;
	float* src[NUM_SW];
	BYTE* sdw[NUM_SW];
	float fillShadow;

	stZmapParamMixSx()
	{
		srcNum = NUM_SW;
		dst = NULL;
		for (int i=0 ; i<NUM_SW ; i++)
		{
			src[i] = NULL;
			sdw[i] = NULL;
		}
		fillShadow = 0.f;
	}

}STZPMIXSX, *LPSTZPMIXSX;


typedef struct stZmapRoiHeader
{
	enum {	TopRed=0x0001, TopGreen=0x0002, TopBlue=0x0004, 
			MiddleRed=0x0008, MiddleBlue=0x0010, 
			BottomRed=0x0020, BottomBlue=0x0040,
			TopWhite=0x0080, ROIFlag=0x0100 };

	unsigned short zeroFlag;
	unsigned short szHeader;
	unsigned short nProjection;
	unsigned short nBucket;
	unsigned short nSave2dFlag;
	unsigned short nOption3D;
			 short stX;
			 short stY;
	unsigned short width;
	unsigned short length;

	size_t Get2dCount()
	{
		size_t n2dCount(0);
		unsigned short nflag(1);
		if((nSave2dFlag & nflag) != 0)
			n2dCount++;
		for (int i=0; i<15; i++)
		{
			nflag <<= 1;
			if((nSave2dFlag & nflag) != 0)
				n2dCount++;
		}

		return n2dCount;
	}

	size_t CalcBufferSize()
	{
		size_t rwh = (size_t)(width) * (size_t)(length);

		size_t n2dCount = Get2dCount();

		return (sizeof(stZmapRoiHeader) + rwh * nBucket * nProjection + rwh * (n2dCount + 1));
	}

	stZmapRoiHeader()
	{
		zeroFlag = 0;
		szHeader = sizeof(stZmapRoiHeader);
		nProjection = 0;
		nBucket = 0;
		nSave2dFlag = TopRed | TopGreen | TopBlue | MiddleRed | MiddleBlue | BottomRed | BottomBlue | TopWhite | ROIFlag;
		nOption3D = 0;
		stX = 0;
		stY = 0;
		width = 0;
		length = 0;
	}
}STZMROIHEADER, * LPSTZMROIHEADER;

typedef struct stZmapBtmDebugHeader
{
	int PartWid;
	int PartLen;
	int nCalcBtmCnt;
	int		nMaxAreaLB[7], nMaxArea[7], nBodyArea[7];
	int		minX[7], minY[7], maxX[7], maxY[7];
	float	sumHeight[7];
	int		cntHeight[7];
	int		exceptArea[7];
	int		selectedLabel[7];
	int		selectedLabel2nd[7];
	int		nNumOfBottom;
	int		nUseMethod;
	int		nNumOfBottom_2nd;
	int		CalcRes;
	int		CalcRes2nd;

	stZmapBtmDebugHeader()
	{
		PartWid = PartLen = 0;
		nUseMethod = -1;
		nCalcBtmCnt=0;
		nNumOfBottom = 0;
		nNumOfBottom_2nd = 0;
		CalcRes = 0;
		CalcRes2nd = 0;
		for(int i=0; i<7; i++)
		{
			nMaxAreaLB[i]		= -1;
			nMaxArea[i]			= -1;
			nBodyArea[i]		= -1;
			minX[i]				= -1;
			minY[i]				= -1;
			maxX[i]				= -1;
			maxY[i]				= -1;
			sumHeight[i]		= -1;
			cntHeight[i]		= -1;
			exceptArea[i]		= -1;
			selectedLabel[i]	= -1;
			selectedLabel2nd[i]	= -1;
		}
	}
}STBTMDEBUGHEADER, * LPSTBTMDEBUGHEADER;

typedef struct stZmapParamPhaseShift
{
	float	 Rad[NUM_SW][MAX_BUCKETS];

	stZmapParamPhaseShift()
	{
		for (int i = 0; i < NUM_SW; i++)
		{
			for (int j = 0; j < MAX_BUCKETS; j++)
			{
				Rad[i][j] = (PI * 0.5f) * j;
			}
		}
	}
}STZPPSHIFT, *LPSTZPPSHIFT;

typedef struct stZmapParamPostProc3D
{
	RECT roi;
	int width, height;
	float* dst;
	float* src;
	float offset;
	float fillShadow;

	stZmapParamPostProc3D()
	{
		roi.left = 0;
		roi.top = 0;
		roi.right = 0;
		roi.bottom = 0;

		width = 0;
		height = 0;

		dst = nullptr;
		src = nullptr;

		offset = 0.f;
		fillShadow = 0.f;
	}
}STZPPOSTPROC3D, *LPSTZPPOSTPROC3D;

typedef struct stZmapParamCorrectWarpage
{
	int fw, fh; // 버퍼 전체 크기
	float* dst;
	float* src;

	BYTE* imgTR;
	BYTE* imgTG;
	BYTE* imgTB;

	float fitR, fitG, fitB;

	RECT roi;		 // 1st - 기울기 계산 용
	RECT roi2nd;	 // 2nd - 워피지 추가 계산 용

	int ModuleIdx;
	int ModuleType;
	
	int* FlagProp;

	int NumExcept;
	ZMEXCEPTROI* ExceptROIs;

	ZMPOLYPOINTULTRA PolygonPoint;

	stZmapParamCorrectWarpage()
	{
		fw = 0, fh = 0;
		dst = nullptr;
		src = nullptr;

		imgTR = nullptr;
		imgTG = nullptr;
		imgTB = nullptr;

		fitR = 0, fitG = 0, fitB = 0;

		roi.left = 0;
		roi.top = 0;
		roi.right = 0;
		roi.bottom = 0;

		roi2nd.left = 0;
		roi2nd.top = 0;
		roi2nd.right = 0;
		roi2nd.bottom = 0;

		ModuleIdx = 0;
		ModuleType = 0;

		FlagProp = nullptr;

		NumExcept = 0;
		ExceptROIs = nullptr;
	}
}STZPCORRWARPAGE, *LPSTZPCORRWARPAGE;
#ifdef _EXPORT_PZMAP_

#ifdef _RUN_CUDA_

#include <cuda.h>
#include <cuda_runtime.h>

#pragma comment(lib, "cudart.lib")

#endif	//_RUN_CUDA_

#endif //_EXPORT_PZMAP_

typedef void (CALLBACK *PZMAPLOG)(wchar_t* szMessage);

namespace Image2D
{
	enum _Image2D
	{
		BottomBlue = 0,
		BottomRed,
		MiddleBlue,
		MiddleRed,
		TopBlue,
		TopRed,
		TopGreen,
		TopWhite,
		NumImg2D
	};
}

namespace ForeignResult
{
	const float NotHitModule	 = -4200.f; // 1
	const float Hole			 = -4100.f; // 3
	const float BottomColor		 = -3600.f; // 4
	const float ExceptForeign	 = -3100.f; // 2
}

#define MAX_COLOR_POLY 8
#define MAX_COLOR_POLY_POINTS 7

typedef struct stZmapParamColorXY
{
	int numPolygon;
	POINT PolygonPts[MAX_COLOR_POLY][MAX_COLOR_POLY_POINTS];
	int numPolygonPts;
	bool enbThres[MAX_COLOR_POLY];
	int typeThres[MAX_COLOR_POLY];
	BYTE thres1[MAX_COLOR_POLY];
	BYTE thres2[MAX_COLOR_POLY];
	bool bMaster[MAX_COLOR_POLY];

	stZmapParamColorXY()
	{
		numPolygon = 0;
		numPolygonPts = 0;

		for (int i = 0; i < MAX_COLOR_POLY; i++)
		{
			for (int j = 0; j < MAX_COLOR_POLY_POINTS; j++)
			{
				PolygonPts[i][j].x = 0;
				PolygonPts[i][j].y = 0;
			}

			enbThres[i] = false;
			typeThres[i] = 0;
			thres1[i] = 0;
			thres2[i] = 0;
			bMaster[i] = false;
		}
	}
}STZPCOLORXY, *LPSTZPCOLORXY;

namespace PZmapMessage
{
	enum PZmapError
	{
		Success = 0,
		InitFail,
		OfflineNotsupported,
		NotSupportedMachine,
		DeviceError,
		InvalidSource,
		NotReady,
		Fail
	};

	typedef enum PZmapError PZmapError_t;
}
