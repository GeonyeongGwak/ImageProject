#pragma once

/*
#ifdef _MPTI_EXPORT
	#define __EXPORT_DLL__
#endif

#ifdef __EXPORT_DLL__
#define MPTIDLL __declspec(dllexport)
#else
#define MPTIDLL __declspec(dllimport)
#endif

#ifdef __cplusplus
#define USEDLL extern "C" __declspec(IESTATE)
#else
#define USEDLL __declspec(IESTATE)
#endif
*/


#ifdef _USRDLL
#define IESTATE dllexport
#else
#define IESTATE dllimport
#endif

#ifdef __cplusplus
#define MPTIDLL extern "C" __declspec(IESTATE)
#else
#define MPTIDLL __declspec(IESTATE)
#endif


//////////////////////////////////////////////////////////////////////////
//
// MPTI common
//
//////////////////////////////////////////////////////////////////////////

typedef void (CALLBACK *MPTISTATUS)(int code);

#define eMR_NULL	NULL

// SHKang 2017/05/17
enum MPTI_EUREGB_TOPOLOTY
{
	eMONO = 24,
	eMONO_DECA = 41
};

enum MPTI_CAMERA_TYPE
{
	Basler = 0,
	IOI,
	LaonPeople,
	Idule,
	ViewWorks_12MX,
	ViewWorks_4MC,
	SenTech,
	Adimec,
	Dahua,
	Basler_Color,
	Crevis_4M,
	Dahua_25M,
};

typedef struct MPTIInitParameter
{
	MPTISTATUS cbStatus;
	bool bUseBubbleInsp;		//NYJ 2019/06/17
	int nDioType;	
	int bucketNum;
	int projectionType;
	bool w1bw2;
	int machineflow;
	int sensorcontacttype;
	int machineType;
	bool useAreaSensor;
	int nGraberType;
	int nResetSWType;
	bool bUseInnerPower;
	int nW2W3Mode;
	BOOL bUseSideCamera;
	BOOL bUseBtmCamera;
	int nCXPVer;
	int nDiffMachine;
	bool bUseImagePilLib;
	int nEureGBTopology; // SHKang 2017/05/17 : add Euresys board topology option... only need R&D's HD machine 
	int nMotorCnt;
	bool bUseThicknessModule;
	int nReviewMotorType;
	int nDiffGrabMode;
	int nUV1Channel;
	int nExposureTime;
	int n10ProjDelayTime;
	bool bUseLaneShuttle;
	int nDualsideTop;
	int nReverseMachinePos;
	int nBadMarkerIOType;
	int nProtoTypeNum;
	int nPZTCnt;
	int nCameraType;
	int nAddDelayExTime;
	BOOL bTableStrobeIO_OldType;
	BOOL bRotateBuffer;
	bool bUseNewOfflineRun;		// LYS 2020/01/13   add for New OfflineRun Seq.

	int nStrobeType;
	int nADConverterType;
	int nZaxisBreakOnIOType; // PNS 2020/02/25
	bool bPZT_OUT1IOType; // PNS 2020/05/06
	int nNeedReset;
	int nUVSelectIdx;

	int nMotionType;
	int nSwitchConvertType; // JHJ

	int nStrobeType_2D;
	int nMCMicroscope;
	int n10ProjDelayTime_3buk;
	bool m_bInlineAreaSensor;
	int nGrabberWaitTime; // JHJ 2020/12/02
	int nConvD2Motor;
	int nLightChangeTW;
	int nSubDiffMachineType;
	bool bUseProjector;
	int nProjectorNum;

	MPTIInitParameter()
	{
		cbStatus = NULL;
		bUseBubbleInsp = false;
		nDioType = 0;		
		bucketNum = 3;
		w1bw2 = false;
		projectionType = 0;
		machineflow = 0;
		sensorcontacttype = 1;
		machineType = 0;
		useAreaSensor = true;
		nGraberType = 0;
		nResetSWType = 0;
		bUseInnerPower = true;
		nW2W3Mode = 0;
		bUseSideCamera = false;
		bUseBtmCamera = false;
		nCXPVer = 3;
		nDiffMachine = 0;
		bUseImagePilLib = false;
		nEureGBTopology=eMONO_DECA;	// SHKang 2017/05/17
		nMotorCnt = 2;
		bUseThicknessModule = false;
		nReviewMotorType = 0;
		nDiffGrabMode = 0;
		nUV1Channel = 8;
		nExposureTime = 0;
		n10ProjDelayTime = 0;
		bUseLaneShuttle = false;
		nDualsideTop = 1;
		nReverseMachinePos = 0;
		nBadMarkerIOType = 0;
		nZaxisBreakOnIOType = 0;
		bPZT_OUT1IOType = false;
		nProtoTypeNum = 0;
		nPZTCnt = 0;
		nCameraType = ViewWorks_12MX;
		nAddDelayExTime = 0;
		bTableStrobeIO_OldType = false;
		bUseNewOfflineRun = false;
		nProtoTypeNum = 0;
		nADConverterType = 0;
		nNeedReset = 0;
		nSwitchConvertType = 0; // JHJ
		nUVSelectIdx = 0;
		nStrobeType_2D = 0;
		nMCMicroscope = 0;
		n10ProjDelayTime_3buk = 0;
		m_bInlineAreaSensor = false;
		nGrabberWaitTime = 15000;
		nConvD2Motor = false;
		nLightChangeTW = 0;
		nSubDiffMachineType = -1;
		bUseProjector = false;
		nProjectorNum = 0;
	}

}MPTI_INIT, *LP_MPTI_INIT;

// typedef struct MPTIParamInit
// {
// 	BSTR mcfPath;
// 	BSTR camxmlPath;
// 	BSTR dcfPath;
// 	BSTR motPath;
// 	BSTR refPath;
// 	BSTR sProcessName;
// 	BSTR sEaglePath;
// 
// 	UINT nFovSizeX;
// 	UINT nFovSizeY;
// 
// 	double dpxlResX;
// 	double dpxlResY;
// 	double dHomeZOffsetPos; 
// 
// 	int nGpuCnt;
// 	int nGpuIdx;
// 	int nProjRotate;
// 	int machineType;
// 	int nModeAI;
// 
// 	BOOL IsPreAlloc3DImg;
// 	BOOL IsDualLane;
// 	BOOL bNetIO;
// 	BOOL bGpuLowerMemUses;
// 	BOOL bUseLensDistortion;
// 	BOOL bUseHomeZOffset;
// 	BOOL bOldNewEqual3D;
// 
// 	MPTIParamInit()
// 	{
// 		mcfPath = NULL;
// 		camxmlPath = NULL;
// 		dcfPath = NULL;
// 		motPath = NULL;
// 		refPath = NULL;
// 		sProcessName = NULL;
// 		sEaglePath = NULL;
// 
// 		nFovSizeX = 0;
// 		nFovSizeY = 0;
// 
// 		dpxlResX = 0.0;
// 		dpxlResY = 0.0;
// 		dHomeZOffsetPos = 0.0;
// 
// 		nGpuCnt = 0;
// 		nGpuIdx = 0;
// 		nProjRotate = 0;
// 		machineType = 0;
// 		nModeAI = 0;
// 
// 		IsPreAlloc3DImg = false;
// 		IsDualLane = false;
// 		bNetIO = false;
// 		bGpuLowerMemUses = false;
// 		bUseLensDistortion = false;
// 		bUseHomeZOffset = false;
// 		bOldNewEqual3D = false;
// 	}
// 
// };

enum MPTI_GRABER_TYPE
{
	MATROX = 0,
	EURESYS_CXP = 1,
	EURESYS_CL = 2,
	MATROX_CXP = 3,
	IMAWORX_CXP = 4,
	Count = 5,
};

enum MPTI_DIO_TYPE
{
	DIO_AJIN_RM = 0,
	DIO_NET_IO  = 1
};

enum MPTI_Return
{
	eMR_GRAB_FAIL = -2,
	eMR_SUCCESS =0,
	eMR_FAIL				=(-1),

	eMR_INIT_FAIL_IO		,
	eMR_INIT_FAIL_MOTOR		,
	eMR_INIT_FAIL_PZT		,
	eMR_INIT_FAIL_GRAB		,
	eMR_INIT_FAIL_CONV		,
	eMR_INIT_FAIL_FIDU		,
	eMR_INIT_FAIL_ZMAP		,
	eMR_INIT_FAIL_GRAB_SETCONFIGURATION	,

	eMR_HOME_MOVE_FAIL
};

enum MPTI_Status
{
	eMS_NONE				=0,
	eMS_INIT_IO				,
	eMS_INIT_MOTOR			,
	eMS_INIT_PZT			,
	eMS_REBOOT_CAMERA		,
	eMS_INIT_GRAB			,
	eMS_INIT_CONV			,
	eMS_INIT_FIDU			,
	eMS_INIT_ZMAP			,
	eMS_INIT_FAIL_IO		,
	eMS_INIT_FAIL_MOTOR		,
	eMS_INIT_FAIL_PZT		,
	eMS_INIT_FAIL_GRAB		,
	eMS_INIT_FAIL_GRAB_SETCONFIGURATION	,
	eMS_INIT_FAIL_CONV		,
	eMS_INIT_FAIL_FIDU		,
	eMS_INIT_FAIL_ZMAP		,
	eMS_SW_RESET			,
	eMS_SW_EMERGENCY		,
	eMS_SW_REMERGENCY		,
	eSM_ALARM_MOTORX		,
	eSM_ALARM_MOTORY		,
	eSM_RALARM_MOTORX		,
	eSM_RALARM_MOTORY		,
	eMS_ERR_CONV			,
	eMS_SEQ_START			,
	eMS_SEQ_STOP			,
	eMS_SEQ_FORCE_STOP		,
	eMS_END_ZMAP_THREAD		,
	eMS_FAIL_GENERATE_ZMAP	,
	eMS_OPEN_UNDER_DOOR		,
	eMS_CLOSE_UNDER_DOOR	,
	eMS_OPEN_UPPER_DOOR		,
	eMS_CLOSE_UPPER_DOOR	,
	eMS_ERR_AIR_SUPPLY		,
	eMS_ERR_TIMEOUT			,
	eMS_ERR_PCBPOS			,
	eMS_ERR_PUP				,
	eMS_ERR_SENSING			,
	eMS_ERR_WSTP			,
	eMS_ERR_WIDTH			,
	eMS_AREA_SENSOR_RAISED  ,
	eMS_AREA_SENSOR_RELEASED,
	eMS_ERR_TIMEOUT_REAR	,
	eMS_ERR_NETIO_COMMUNICATION	,
	eMS_ERR_CXP_TRIGGER_FAIL_ERROR,
	eMS_ERR_CXP_TRIGGER_TIMEOUT,
	eMS_ERR_CXPDLL_API_PUSH_ERROR,
	eMS_ERR_CXP_CAMERA_CONNECTION_ERROR,
	eMS_ERR_CXP_TRIGGER_LOSS,
	eMS_NUM
};

enum MPTI_InspectionMode
{
	eMI_Inspection = 0,
	eMI_WindowTeaching,
	eMI_RawDataInsp,
	eMI_RawDataInspEx,
	eMI_SimilarInsp
};

enum MPTI_SideCamNum
{
	eMSCN_COAXIAL = -1,
	eMSCN_SIDECAM1 = 0,
	eMSCN_SIDECAM2 = 1,
	eMSCN_SIDECAM3 = 2,
	eMSCN_SIDECAM4 = 3,
	eMSCN_NUM
};

enum MPTI_WarpPosition
{
	eMWP_TOPLEFT = 0,
	eMWP_TOPRIGHT = 1,
	eMWP_BOTTOMLEFT = 2,
	eMWP_BOTTOMRIGHT = 3,
	eMWP_NUM
};

enum MPTI_SideChannel
{
	eMSC_COLOR = 0,
	eMSC_RED = 1,
	eMSC_GREEN = 2,
	eMSC_BLUE = 3,
	eMSC_NUM
};


typedef struct MPTIMachineStatus
{
	int reset;
	int emergency;
	int homeAxisX;
	int homeAxisY;
	int servoAlarmX;
	int servoAlarmY;
	int convComplite;
	int convError;
	int underDoor;
	int upperDoor;

	// 컨베이어 에러 상태
	int convErrorType;

	MPTIMachineStatus()
	{
		reset = 1;
		emergency = 0;
		homeAxisX = 0;
		homeAxisY = 0;
		servoAlarmX = 0;
		servoAlarmY = 0;
		convComplite = 0;
		convError = 0;
		convErrorType = 0;
	}

}MMSTATUS, *LP_MMSTATUS;

enum MPTI_Machine_Flow
{
	eMMF_LR	=0,
	eMMF_RL
};

enum MPTI_Machine_Type
{
	eMMT_INLINE,
	eMMT_TABLE,
	eMMT_OFFLINE
};

enum MPTI_Light_Kinds
{
	eMLK_3D1=0,
	eMLK_3D2,
	eMLK_3D3,
	eMLK_3D4,
	eMLK_3D5,
	eMLK_3D6,
	eMLK_3D7,
	eMLK_3D8,
	eMLK_2DBB,
	eMLK_2DBR,
	eMLK_2DMB,
	eMLK_2DMR,
	eMLK_2DTB,
	eMLK_2DTR,
	eMLK_2DTG,
	eMLK_2DTW,
	eMLK_3D9,
	eMLK_3D10,
	eMLK_SIDE,
	eMLK_3D11,
	eMLK_3D12,
	eMLK_3D13,
	eMLK_3D14,
	eMLK_3D15,
	eMLK_3D16,
	eMLK_NUM
};

enum MPTI_CONV_FED
{
	eSTOP_IMMEDIATE = 0,
	eSTOP_DELAYED = 1,
};
enum MPTI_UV_Kinds
{
	eMUK_UV1 = 0,
	eMUK_UV2,
	eMUK_UV12,
	eMUK_UV1_Bub,
	eMUK_UV2_Bub,
	eMUK_UV12_Bub,
	eMUK_NUM
};


#define MPTI_GRAB_CURRENT	0xffff
#define MPTI_GRAB_CH3D1		0x0001
#define MPTI_GRAB_CH3D2		0x0002
#define MPTI_GRAB_CH3D3		0x0004
#define MPTI_GRAB_CH3D4		0x0008
#define MPTI_GRAB_CH3D5		0x0010
#define MPTI_GRAB_CH3D6		0x0020
#define MPTI_GRAB_CH3D7		0x0040
#define MPTI_GRAB_CH3D8		0x0080
#define MPTI_GRAB_CH2DBB	0x0100
#define MPTI_GRAB_CH2DBR	0x0200
#define MPTI_GRAB_CH2DMB	0x0400
#define MPTI_GRAB_CH2DMR	0x0800
#define MPTI_GRAB_CH2DTB	0x1000
#define MPTI_GRAB_CH2DTR	0x2000
#define MPTI_GRAB_CH2DTG	0x4000
#define MPTI_GRAB_CH2DTW	0x8000
#define MPTI_GRAB_CH3D9		0x10000
#define MPTI_GRAB_CH3D10	0x20000
#define MPTI_GRAB_SIDE		0x40000
#define MPTI_GRAB_CH3D11	0x80000
#define MPTI_GRAB_CH3D12	0x100000
#define MPTI_GRAB_CH3D13	0x200000
#define MPTI_GRAB_CH3D14	0x400000
#define MPTI_GRAB_CH3D15	0x800000
#define MPTI_GRAB_CH3D16	0x1000000

//////////////////////////////////////////////////////////////////////////
//
// regrading coordinate
//
//////////////////////////////////////////////////////////////////////////

#define		FIDU_NUM_BOARD		4
#define		FIDU_NUM_MODULE		100
#define		FIDU_NUM_PART		2

#define		MOTOR_PULSE_CNT		7

//************************************
// Qualifier: EuresysCXP Ver10 Grab Check Define	// LYS 2019/10/22
//************************************


enum MPTI_Coordinate_Anchor
{
	eMCA_LB	=0,		// left,bottom [x(+),y(+)]
	eMCA_RB	,		// right,bottom [x(-),y(+)]
	eMCA_LT	,		// left,top [x(+),y(-)]
	eMCA_RT			// right,top [x(-),y(-)]
};

typedef struct MPTICoordinateMachine
{
	double x;
	double y;

	MPTICoordinateMachine()
	{
		x = 0.0;
		y = 0.0;
	}

	MPTICoordinateMachine(double ix, double iy)
	{
		x = ix;
		y = iy;
	}
}MCOORDI_M, *LP_MCOORDI_M;

typedef struct MPTICoordinateBoard
{
	double x;
	double y;

	MPTICoordinateBoard()
	{
		x = 0.0;
		y = 0.0;
	}

	MPTICoordinateBoard(double ix, double iy)
	{
		x = ix;
		y = iy;
	}

	BOOL IsValidate()
	{
		if((x == 0.0) && (y == 0.0))
			return FALSE;

		return TRUE;
	}
}MCOORDI_B, *LP_MCOORDI_B;

typedef struct MPTICoordinateCorrector
{
	double stdTheta;	// degree
	double curTheta;
	double theta;
	double offsetX;
	double offsetY;
	double deltaX;
	double deltaY;
	double scaleX;
	double scaleY;
	double m_dBigJobLine;
	BOOL m_bGenerate;

public:
	BOOL IsGenerate() const { return m_bGenerate; }
	void SetGenerate(BOOL bGenerate) { m_bGenerate = bGenerate; }

	MPTICoordinateCorrector()
	{
		stdTheta = 0.0;
		curTheta = 0.0;
		theta = 0.0;
		offsetX = 0.0;
		offsetY = 0.0;
		deltaX = 0.0;
		deltaY = 0.0;
		scaleX = 1.0;
		scaleY = 1.0;
		SetGenerate(FALSE);
	}
}MCOORDI_COORECTOR, *LP_MCOORDI_COORECTOR;

//////////////////////////////////////////////////////////////////////////
//
// regrading conveyor
//
//////////////////////////////////////////////////////////////////////////

#define PCONV_PORT		2
#define PCONV_BAUD		9600
#define PCONV_COMM_ID	1

enum MPTI_Conveyor_RailIndex
{
	eMCRI_F	=0,	// front
	eMCRI_R,	// rear
	eMCRI_Num
};

enum MPTI_Conveyor_Table
{
	eMCT_PRE		=0,
	eMCT_WORK	,
	eMCT_NEXT	,
	eMCT_ALL
};

enum MPTI_Conveyor_RunSpeed
{
	eMCRS_FAST		=0,
	eMCRS_NORMAL	,
	eMCRS_SLOW
};

enum MPTI_Conveyor_Direction
{
	eMCD_REVERSE	=0,
	eMCD_FORWARD
};

enum MPTI_Conveyour_Section
{
	eMCS_S1		=0,		// sensor section
	eMCS_S2		,
	eMCS_S3		,
	eMCS_S4		,
	eMCS_S5		,
	eMCS_W				// width control
};

#define PCONV_WAIT_WSTP	3000

//////////////////////////////////////////////////////////////////////////
//
// regrading PZT
//
//////////////////////////////////////////////////////////////////////////

#define TAD10_IP	"217.1.1.10"
#define TAD10_PORT	5000

#define STAD_PORTNUM_1	"COM9"
#define STAD_PORTNUM_2	"COM8"
#define STAD_BAUDRATE	9600

#define PZT_N	2
#define PZT_1	0
#define PZT_2	1
#define PZT_3	2
#define PZT_4	3

#define PZT_ELAPSE	50
#define PZT_SOFFSET	4.0

// wait time for PZT and light channel control
#define WAIT_2D		(0.003)	//(0.003)	//(0.001)	// light
#define WAIT_2D3D	(0.003)	//(0.003)	//(0.01)	// PZT + light
#define WAIT_GRAB	(0.01)

// PZT movement
#define PZT_CH1		9.3
#define PZT_CH2		10.15
#define PZT_CH3		10.4
#define PZT_CH4		11
#define PZT_CH5		9.3
#define PZT_CH6		10.15
#define PZT_CH7		10.4
#define PZT_CH8		11

#define PZT_OFFSET	4.0


//////////////////////////////////////////////////////////////////////////
//
// regarding motion
//
//////////////////////////////////////////////////////////////////////////

//#define FILEPATH_MOTION_SET	"C:\\Mot\\AjinEIP.mot"
#define FILEPATH_MOTION_SET	"C:\\Mot\\AjinRM_2Axis.mot"

enum MPTI_Axis_Info
{
	eMXI_X	=0,
	eMXI_Y,
	eMXI_Z1,
	eMXI_Z2,
	eMXI_NUN
};

#define MTPI_AXIS_MODULE_X	0
#define MTPI_AXIS_MODULE_Y	1
#define MTPI_AXIS_MODULE_Z1	2
#define MTPI_AXIS_MODULE_Z2	3

#define DEF_VEL	200.0		// unit: mm/sec
#define DEF_ACC	2000.0
#define DEF_DEC	2000.0


//////////////////////////////////////////////////////////////////////////
//
// regarding input/output
//
//////////////////////////////////////////////////////////////////////////

/*
#define BITS_IN_MODULE	32
#define BITS_OUT_MODULE	32
#define MODULE_DINS		2
#define MODULE_DOUTS	2
#define BITS_DIN		(BITS_IN_MODULE * MODULE_DINS)
#define BITS_DOUT		(BITS_OUT_MODULE * MODULE_DOUTS)

enum MIO_Module_Index
{
	eMIMI_IN1	=0,		// in index has to continuous
	eMIMI_IN2	,
	eMIMI_OUT1	=2,		// out index has to continuous
	eMIMI_OUT2
};

enum MIO_Module_Byte
{
	eMIMB_1		=0,
	eMIMB_2		,
	eMIMB_3		,
	eMIMB_4		,
	eMIMB_NUM
};
*/

#define LIGHT_MIN			0
#define LIGHT_MAX			63
#define DEF_LLEVEL			LIGHT_MAX
#define LIGHT_LEVLE0_3D		0xfc0fffff
#define LIGHT_LEVLE0_2D		0xfc0fffff
#define LIGHT_ALLOFF_3D		0xfff0ffff
#define LIGHT_ALLOFF_2D		0xfff0ffff
#define LIGHT_INIT_3D		(LIGHT_ALLOFF_3D & LIGHT_LEVLE0_3D)
#define LIGHT_INIT_2D		(LIGHT_ALLOFF_2D & LIGHT_LEVLE0_2D)
#define BIT_IN_BYTE			8
#define LIGHT_SHFIT_CH3D	(BIT_IN_BYTE * 2)
#define LIGHT_SHFIT_CH2D	(BIT_IN_BYTE * 2)
#define LIGHT_SHFIT_BR3D	(BIT_IN_BYTE * 2 + 4)
#define LIGHT_SHFIT_BR2D	(BIT_IN_BYTE * 2 + 4)

// (0)off (1)on (-1)disable
typedef struct stInputChannel
{
	int pcb_in1;		// pcb sensor in1
	int pcb_in2;		// pcb sensor in2
	int pcb_w1;			// pcb sensor w1
	int pcb_w2;			// pcb sensor w2
	int pcb_w3;			// pcb sensor w3
	int pcb_out1;		// pcb sensor out1
	int pcb_out2;		// pcb sensor out2	

	int pstp_up;		// pre conveyor stopper up sensor
	int pstp_dn;		// pre conveyor stopper down sensor	
	int wstp_up;		// work conveyor stopper up sensor	
	int wstp_dn;		// work conveyor stopper down sensor
	
	int pup_s1;			// push-up plate sensor 1
	int pup_s2;			// push-up plate sensor 2	
	
	int safe_l;			// safety sensor left
	int safe_r;			// safety sensor right
	
	int upper_door;		// upper door sensor
	int under_door;		// under door sensor

	int smema_ba;		// SMEMA board available signal from pre-machine
	int smema_nb;		// SMEMA not busy signal from next-machine
	
	int ext_barcode;	// trigger input from extra barcode reader
	
	int pzt_in1;		// input 1 from PZT
	int pzt_in2;		// input 2 from PZT

	int sw_emer;		// emergency switch
	int sw_reset;		// reset switch
	
	int n_ng;			// N.NG signal feedback
	int p_ok;			// P.OK signal feedback
	int n_ba;			// N.BA signal feedback

	int air_press;		// air pressure sensor
	
	int conv_complete;	// complete signal from conveyor controller
	int conv_error;		// error signal from conveyor controller
	int conv_insert;	// pcb inserting from pre machine
	int conv_eject;		// pcb ejecting to next machine

	int shuttle_complete;	// complete signal from Shuttle PLC

	int smema_r_ok;			// R.OK signal feedback

	// For Table Type
	int sw_start		  ;
	int sw_stop			  ;
	int safety_area       ;

	int vacuumAir;

	int railDoor		;
	int tem_smo			;
	int ionizer_L		;
	int ionizer_R		;
	int air_press_Ionizer;
	int bup_up			;
	int bub_down		;

	/*
	int ilock_s1;		// interlock sensor
	int ilock_s2;
	int sdoor;			// stage door
	int ld_acks;		// SMEMA interface
	int uld_reqs;
	int pcb_reqs;	
	int sw_start;	
	int sw_stop;
	int iw3pl;			// unknown
	int iw3nl;
	*/

	stInputChannel()
	{
		memset(this, 0, sizeof(stInputChannel));
	}

}IN_CH, *LP_IN_CH;

// (0)off (1)on (-1)disable
typedef struct stOutputChannel
{
	int tl_1		;
	int tl_2		;
	int tl_3		;
	int buzzer		;

	int smema_nb	;
	int smema_ba	;
	int smema_ng	;

	int pstp		;
	int wstp		;

	int pup			;
	int bup			;

	int pzt_out1	;
	int pzt_out2	;
	int pzt_reset	;

	int enb_safety	;

	int l3d_ch0		;
	int l3d_ch1		;
	int l3d_ch2		;
	int l3d_ch3		;
	int l3d_br0		;
	int l3d_br1		;
	int l3d_br2		;
	int l3d_br3		;
	int l3d_br4		;
	int l3d_br5		;

	int l2d_ch0		;
	int l2d_ch1		;
	int l2d_ch2		;
	int l2d_ch3		;
	int l2d_br0		;
	int l2d_br1		;
	int l2d_br2		;
	int l2d_br3		;
	int l2d_br4		;
	int l2d_br5		;

	int ReleaseReset ;	// This IO release actual reset switch IO
	int RepairConfirmOK	;
	int RepairConfirmNG	;
	int StrobeON ;	// AOI 4(0~)
	int Z1AxisBreakOn ;
	int Z2AxisBreakOn ;

	int vacuumplate;
	int badmarkerCover;
	int badmarker;

	int smema_r_ba	;
	int smema_r_reset	;
	int smema_r_turnover	;
	int smema_r_FinalNGSignal ;

	int conv_reset;

	int msl_Shutter_R;
	int msl_Shutter_L;
	int PCB_Lane;

	int TL_Mid_Flash;
	int TL_BTM_Flash;
	int W3_Sensor	;
	int ionizer;

	/*
	int p_ok		;
	int n_ba		;
	int ng_sig		;
	*/

}OUT_CH, *LP_OUT_CH;

enum MPTI_EmSwitch_Status
{
	eESS_NONE		=0,
	eESS_PRESSED	,
	eESS_RELEASED
};


//////////////////////////////////////////////////////////////////////////
//
// regarding grab
//
//////////////////////////////////////////////////////////////////////////

#define DCF_FILEPATH _T("C:\\DCF\\basler4M_Trig_Strobe_CC2.DCF")

typedef void (CALLBACK *MPTILIVEGRAB)(int idxBuf, int lenGrab);
typedef void (CALLBACK *MPTISEQGRAB)(int sequence, int fovCnt, int fovIdx, int nCallIndex, int finish);
typedef void (CALLBACK *MPTISIDEWARP)(int fovCnt);

typedef void (CALLBACK *MPTIADDLOG)(wchar_t *  szMessage);
typedef void (CALLBACK *MPTIADDLOG_DEV)(wchar_t*  szMessage);


enum MPTI_3D_Channel
{
	eM3C_1=0,
	eM3C_2,
	eM3C_3,
	eM3C_4,
	eM3C_5,
	eM3C_6,
	eM3C_7,
	eM3C_8,
	eM3C_9,
	eM3C_10,
	eM3C_11,
	eM3C_12,
	eM3C_13,
	eM3C_14,
	eM3C_15,
	eM3C_16,
	eM3C_17,
	eM3C_18,
	eM3C_NUM
};

enum MPTI_3D_ALL_Channel
{
	eM3AC_1_1=0,
	eM3AC_1_2,
	eM3AC_1_3,
	eM3AC_1_4,
	/*eM3C_5,
	eM3C_6,
	eM3C_7,
	eM3C_8,
	eM3C_9,
	eM3C_10,
	eM3C_11,
	eM3C_12,
	eM3C_13,
	eM3C_14,
	eM3C_15,
	eM3C_16,*/
	eM3AC_NUM
};

enum MPTI_2D_Channel
{
	eM2C_BB=0,
	eM2C_BR,
	eM2C_MB,
	eM2C_MR,
	eM2C_TB,
	eM2C_TR,
	eM2C_TG,
	eM2C_TW,
	eM2C_NUM
};

enum MPTI_2D_UV_Channel
{
	eM2UC_TW=0,
	eM2UC_UV1,
	eM2UC_BUB,
	eM2UC_NUM
};

enum MPTI_2D_Channel_FiduAll
{
	eM2C_Fidu_BR=0,
	eM2C_Fidu_BB,
	eM2C_Fidu_MR,
	eM2C_Fidu_MB,
	eM2C_Fidu_TR,
	eM2C_Fidu_TB,
	eM2C_Fidu_TG,
	eM2C_Fidu_TW,
	eM2C_Fidu_NUM
};

enum MPTI_3D_BukMode
{
	eM3D_4BUK=0,
	eM3D_3BUK,
	eM3D_BUK_NUM
};

enum MPTI_PZT_CNT
{
	ePZT_2,
	ePZT_4,
	ePZT_8
};

enum ADConverter
{
	ADC_TCPIP_TAD10,
	ADC_SERIAL_STAD,
	ADC_TCPIP_STAD,
};
typedef struct MPTISeqLightLevel
{
	int bb;	// bottom blue
	int br;	// bottom red
	int mb;	// middle blue
	int mr;	// middle red
	int tb;	// top blue
	int tr;	// top red
	int tg;	// top green
	int tw;	// top white

	MPTISeqLightLevel()
	{
		bb = 0;
		br = 0;
		mb = 0;
		mr = 0;
		tb = 0;
		tr = 0;
		tg = 0;
		tw = 0;
	}

	MPTISeqLightLevel(int ibb, int ibr, int imb, int imr, int itb, int itr, int itg, int itw)
	{
		bb = ibb;
		br = ibr;
		mb = imb;
		mr = imr;
		tb = itb;
		tr = itr;
		tg = itg;
		tw = itw;
	}
}MSEQLIGHTVAL, *LP_MSEQLIGHTVAL;

enum MPTI_Grab_Sequence
{
	eMGS_FG		=0,		// full grab
	eMGS_FS		,		// full scan
	eMGS_NUM
};


//////////////////////////////////////////////////////////////////////////
//
// regarding fiducial
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
// regarding z-map
//
//////////////////////////////////////////////////////////////////////////

#define DEF_LAMDA		500.f
#define DEF_LAMDA_CX	0.f
#define DEF_LAMDA_CY	0.f

#define REF_FILEPATH	_T("C:\\ref\\r")
#define REF_FILEEXT		_T(".nphi")

enum single_wavelength_channel
{
	swc_1=0,
	swc_2,
	swc_3,
	swc_4,
	swc_5,
	swc_6,
	swc_7,
	swc_8,
	swc_9,
	swc_10,
	swc_11,
	swc_12,
	swc_num
};
enum dual_wavelength_channel_pair
{
	dwp_12=0,
	dwp_34,
	dwp_56,
	dwp_78,
	dwp_num
};
typedef struct stParamGetZmapSingle
{
	RECT	roi;
	int		imgWidthStep;
	BYTE*	pImg[4];
	BYTE*	pImgR;
	BYTE*	pImgB;

	stParamGetZmapSingle()
	{
		roi.left = roi.right = roi.top = roi.bottom = 0;
		imgWidthStep = 0;
		for (int i=0 ; i<4 ; i++)
		{
			pImg[i] = NULL;
		}
		pImgR = NULL;
		pImgB = NULL;
	}
}STPGETZMAPS, *LPSTPGETZMAPS;
typedef struct stParamGetZmapDual
{
	RECT	roi;
	int		imgWidthStep;
	BYTE*	pImg[2][4];
	BYTE*	pImgR;
	BYTE*	pImgB;

	stParamGetZmapDual()
	{
		roi.left = roi.right = roi.top = roi.bottom = 0;
		imgWidthStep = 0;
		for (int i=0 ; i<2 ; i++)
		{
			for (int j=0 ; j<4 ; j++)
			{
				pImg[i][j] = NULL;
			}
		}
		pImgR = NULL;
		pImgB = NULL;
	}
}STPGETZMAPD, *LPSTPGETZMAPD;
typedef struct stParamGetZmapDx
{
	RECT	roi;
	int		imgWidthStep;
	BYTE*	pImg[4][2][4];
	BYTE*	pImgR;
	BYTE*	pImgB;

	stParamGetZmapDx()
	{
		roi.left = roi.right = roi.top = roi.bottom = 0;
		imgWidthStep = 0;
		for (int i=0 ; i<4 ; i++)
		{
			for (int j=0 ; j<2 ; j++)
			{
				for (int k=0 ; k<4 ; k++)
				{
					pImg[i][j][k] = NULL;
				}					
			}
		}
		pImgR = NULL;
		pImgB = NULL;
	}
}STPGETZMAPDX, *LPSTPGETZMAPDX;
typedef struct stParamGetZmapSx
{
	RECT	roi;
	int		imgWidthStep;
	BYTE*	pImg[8][4];
	BYTE*	pImgR;
	BYTE*	pImgB;

	stParamGetZmapSx()
	{
		roi.left = roi.right = roi.top = roi.bottom = 0;
		imgWidthStep = 0;
		for (int i=0 ; i<8 ; i++)
		{
			for (int j=0 ; j<4 ; j++)
			{
				pImg[i][j] = NULL;
			}
		}
		pImgR = NULL;
		pImgB = NULL;
	}
}STPGETZMAPSX, *LPSTPGETZMAPSX;


//Zmap Margin (Inspection use)
#define  ZMAP_MARGIN		1   //mm  편측   (*2)

enum digitizer_grab_status
{
	eDGS_STOP=0,
	eDGS_SINGLE,
	eDGS_CONTINUE,
	eDGS_LIVE,
	eDGS_CONTINUE_SINGLE,
	eDGS_FIDUALL,
	eDGS_SINGLE_COLOR,
	eDGS_CONTINUE_COLOR,
	eDGS_CONTINUE_SINGLE_COLOR
};

enum measurement_mode
{
	eMM_NORMAL=0,
	eMM_REF_IMAGE_ADDITION
};

enum CalibType
{
	eCT_None = 0,
	eCT_CxCyLamda,
	eCT_Coeff,
	eCT_Pzt,
	eCT_CoeffRef,
	eCT_CxCyLambda_Dual,
	eCT_Coeff_Dual,
	eCT_Report,
	eCT_CxCyLambda_Taller,
	eCT_Coeff_Taller,
	eCT_Direction,
	eCT_NUM
};

enum euresys_grab_status
{
	eEureCXP_Ret_OK = 0,
	eEureCXP_Ret_Busy = 1,
	eEureCXP_Ret_Fail = 2,
	eEureCXP_Ret_NUM
};

enum MPTI_GRAB_ERROR_INFO_V10
{
	MPTI_V10_GInfo_Push,
	MPTI_V10_GInfo_Cable,	
	MPTI_V10_GInfo_Trigger,	
	MPTI_V10_GInfo_NewBufCallback,
	MPTI_V10_GInfo_NetIOCmd
};

enum InspRoiImgBuf_no
{
	eInspRoiImg_imgTop_R = 0,
	eInspRoiImg_imgTop_G,
	eInspRoiImg_imgTop_B,
	eInspRoiImg_imgTop_W,
	eInspRoiImg_imgMiddle_R,
	eInspRoiImg_imgMiddle_B,
	eInspRoiImg_imgBottom_R,
	eInspRoiImg_imgBottom_B,
	eInspRoiImg_imgSide1_R,
	eInspRoiImg_imgSide1_G,
	eInspRoiImg_imgSide1_B,
	eInspRoiImg_imgSide2_R,
	eInspRoiImg_imgSide2_G,
	eInspRoiImg_imgSide2_B,
	eInspRoiImg_imgSide3_R,
	eInspRoiImg_imgSide3_G,
	eInspRoiImg_imgSide3_B,
	eInspRoiImg_imgSide4_R,
	eInspRoiImg_imgSide4_G,
	eInspRoiImg_imgSide4_B,
	eInspRoiImg_imgAI,
	eInspRoiImg_NUM
};

enum RoiColorBuf_no
{
	eRoiColorBuf_img_R = 0,
	eRoiColorBuf_img_G,
	eRoiColorBuf_img_B,
	eRoiColorBuf_img_R_BtmSide,
	eRoiColorBuf_img_G_BtmSide,
	eRoiColorBuf_img_B_BtmSide,
	eRoiColorBuf_NUM
};

enum RawDataMachineType
{
	eRawDataNone = -1,
	eRawDataAOI = 0,
	eRawDataAOXI,
	eRawDataAXI,
	eRawDataNum
};