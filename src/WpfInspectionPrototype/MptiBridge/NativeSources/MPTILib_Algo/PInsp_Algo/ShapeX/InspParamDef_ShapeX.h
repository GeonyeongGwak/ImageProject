#pragma once
#include "../InspParamDef_AlgoBase.h"
#define nShapeXCnt 200
#define nShapeXModelCnt 10

enum SHAPEX_OPTION
{
	MinScarThick = 0x01,
	aspect_ratio = 0x02,
	CrossLine_Detect = 0x04,
	VerticalMaxLength = 0x08,
	HorizontalMaxLength = 0x10,
	MaxNGArea = 0x20,
	ChippingCriticalLine = 0x40,
	TieBarRateCheck = 0x80,
	DeleteExitArea = 0x100,
	InspExitInnerArea = 0x200,
	TabLineChipping = 0x400,
	NonMatchingMode = 0x800,
	TabIncludeSide = 0x1000,
	WrForeignCnt = 0x2000,
	WrForeignWidth = 0x4000,
	WrForeignLength = 0x8000,
	NGGrouping = 0x10000,
	UseContrast=0x20000,
	UseSwapX_Y = 0x40000,
	UseNG1SubtNG2 = 0x80000,
	UseStripeCorrection = 0x100000,
	UseGradationFilter = 0x200000,
	UseCurrentShape = 0x400000,      // CurrentShape 모드: 마스크 기반 검사
	UseInvMatching = 0x800000,
	UseOutLineSecond = 0x1000000,    // 외곽선 검사 2회 실시 여부
	UseAngleProfiling = 0x2000000,        // 각도 프로파일링 검사 실시 여부
	UseNonCriticalExist = 0x4000000,      // Non-Critical Exist 검사 실시 여부
//UseCurrentShapeMask = 0x800000,
};
struct ShapeXROI
{
public:
	RECT ROI;      //윈도우 이미지 내부의 알고리즘의 검사 ROI
	POINT pCt;     //윈도우 이미지 내부의 pad(tab)의 center좌표
	BOOL bUseShape;     //형상검사 (형상과 현재 이진화를 차연산하여 Area측정)
	BOOL bUseInner;     //내부검사
	BOOL bUseExist;     //존재유무검사 true존재영역에있는 부분 검사: false 존재영역에 있는 부분 don`t care
	BOOL bUseShift;     //shift검사
	
	float fShapeArea;       //형상검사 OK Area기준
	float fInnerArea;       //내부검사 OK Area기준
	float fExistArea;       //존재유무 OK Area기준
	float fShiftX;           //shiftX 허용량(mm단위)
	float fShiftY;           //shiftY 허용량(mm단위)	
	int nModelCnt;
	int ArrModel[nShapeXModelCnt];  //사용 할 File상 Model Index
    //차후 추가 변수
	BOOL bUseExistShape;
	float fExistShapeArea;

	// sjb++ 250414.[3] add in struct

	// sjb++ 250408.[40] ~InspParamDef.h~
	float fLocalLength;
	float fLocalLength2;
	float fDent;
	float fDentMin;
	float fDentMax;
	BOOL bUseLocalLength;
	BOOL bUseDent;

	BOOL bUseForeignWidth;
	BOOL bUseForeignLength;
	float fForeignWidth;
	float fForeignLength;

	BOOL bUseForeignWLAnd;

	ShapeXROI()
	{
		bUseShape = false;
		bUseInner = false;
		bUseExist = false;
		bUseShift = false;
		bUseForeignWidth = false;
		bUseForeignLength = false;
		fShapeArea = 0.0f;
		fInnerArea = 0.0f;
		fExistArea = 0.0f;
		fShiftX = 0.0f;
		fShiftY = 0.0f;
		fForeignWidth = 0.025f;
		fForeignLength = 0.025f;
		nModelCnt = 0;
		bUseExistShape = false;
		fExistShapeArea = 0.0f;
		fLocalLength = 0.0f;
		fLocalLength2 = 0.0f;
		fDent = 0.0f;
		fDentMin = 0.0f;
		fDentMax = 0.0f;
		bUseLocalLength = false;
		bUseDent = false;
		bUseForeignWLAnd = false;
	}


	// esjb++ 250408.[40]
	// ++sjb 250414.[3] ~InspParamDef_ShapeX.h~

};
typedef struct tagAlgoShapeX
{
public:
	double dPartangle;
	int m_nLeadTipDirection;        // Lead 방향
	
	// S_sjb++ 250414.[0] add in struct
	int m_nTypeSelectBlob;
	int m_nTypeSelectTarget;
	// E_sjb++ 250414.[0] ~InspParamDef_ShapeX.h~

	int ROICnt;
	   
	// S_sjb+c 250414.[1] revise type
	//int MatchSc;
	short MatchSc;
	// E_sjhb+c 250414.[1] ~InspParamDef_ShapeX.h~

	// S_sjb+c to short
	//int nHist1;
	//int nHist2;
	short nHist1;
	short nHistLow1;
	short nHistUp1;
	short nHist2;
	short nHistLow2;
	short nHistUp2;

	struct tagAlgoBlobBase m_sBlobBase_ShapeArea;
	struct tagAlgoBlobBase m_sBlobBase_NG;
	struct tagAlgoBlobBase m_sBlobBase_NG2;
	wchar_t ArrPathModel[MAX_STRLEN];
	ShapeXROI spROI[nShapeXCnt];
	int nInspOption;

	float Aspectratio;
	float MinScarThickness;
	float Verticalmaxlength;
	float Horizontalmaxlength;
	float MaxNGArea;
	float CriticalArea;
	float MaxChippingLength;
	int nStartIdx;
	float fTieBarRate;//default 0.6f
	short WrForeignCnt;
	float WrForeignWidth;
	float WrForeignLength;
	float NGGroupingMaxSize;
	float NGGroupingDistance;
	int ContrastValue;
	float Check3DMaxVal;
	float Check3DMinVal;

	bool CheckSearchByShape;

	int nWndId;
	enum ETypeGep
	{
		eShapeX,
		eShapeXSpace,
		eColor
	};

	tagAlgoShapeX()
	{
		// S_sjb+c 250414.[2] init
		m_nTypeSelectBlob = ETypeBlob::eSelectBigger;
		m_nTypeSelectTarget = 0;
		// E_sjhb+c 250414.[2] ~InspParamDef_ShapeX.h~

		Aspectratio = 0.f;
		CriticalArea = 0.f;
		Horizontalmaxlength = 0.f;
		MatchSc = 0;
		MaxChippingLength = 0.f;
		MaxNGArea = 0.f;
		MinScarThickness = 0.f;
		ROICnt = 0;
		Verticalmaxlength = 0.;
		dPartangle = 0.;
		fTieBarRate = 0.f;
		m_nLeadTipDirection = 0;
		nHist1 = 0;
		nHist2 = 0;
		nInspOption = 0;
		nStartIdx = 0;
		WrForeignCnt = 0;
		WrForeignWidth = 0.0f;
		WrForeignLength = 0.0f;
		NGGroupingMaxSize = 0.0f;
		NGGroupingDistance = 0.0f;
		ContrastValue = 0;
		Check3DMaxVal = 0.1f;
		Check3DMinVal = 0.0f;
		CheckSearchByShape = false;
		nWndId = 0;
	}

	BOOL IsUseMultiArea()
	{
		return TRUE;
	}
}AlgoShapeX;




typedef struct tagRstAlgoShapeX	// ShapeX 검사 결과
{
public:
	BOOL isInsp;//검사여부
	BOOL isOK;//Algo ok

	short nROICnt;      //shapePadROI개수
	short nNGAreaRoiCnt;//NG blob개수
	int nMatchingIdx[nShapeXCnt];    //Matching Model Index;
	float nMatchingSc[nShapeXCnt];   //Matching score

	BOOL ArrROIOK[nShapeXCnt];     //ROI검사결과
	BOOL ArrOKShape[nShapeXCnt];   //형상검사결과
	BOOL ArrOKInner[nShapeXCnt];   //내부검사결과
	BOOL ArrOKExist[nShapeXCnt];   //존재유무검사결과
	BOOL ArrOKShift[nShapeXCnt];   //shift검사결과
	BOOL ArrOKForeignW[nShapeXCnt];//이물단축검사결과
	BOOL ArrOKForeignL[nShapeXCnt];//이물장축검사결과
	float ArrShapeArea[nShapeXCnt];//형상검사 NG Area
	float ArrInnerArea[nShapeXCnt];//내부검사 NG Area
	float ArrExistArea[nShapeXCnt];//존재유무 NG Area
	float ArrShiftX[nShapeXCnt];    //shift 값
	float ArrShiftY[nShapeXCnt];    //shift 값
	float ArrForeignW[nShapeXCnt];  //이물단축 NG mm
	float ArrForeignL[nShapeXCnt];  //이물장축 NG mm
	RECT rcArrRect[nShapeXCnt];	//현재 shpaeArea의 Rect
	int m_rcArrNGArrRectCnt[nShapeXCnt];   //ROI별 NG개수
	RECT rcArrNGRect[nShapeXCnt];	//NG위치
	int m_nArrMaxFreqValue1[nShapeXCnt];   //foreign1 평균
	int m_nArrMaxFreqValue2[nShapeXCnt];   //foreign2 평균

	BOOL m_bUseExistShape[nShapeXCnt];	//존재검사영역 형상검사사용
	BOOL ArrOKExistShape[nShapeXCnt];   //존재검사영역 형상검사결과
	float ArrExistShapeArea[nShapeXCnt];//존재검사영역 형상검사 NG Area

	// sjb++ 250408.[21] ~PInspalgo_Def.h~
	float ArrLocalLength[nShapeXCnt];
	float ArrDent[nShapeXCnt];
	float ArrDent2[nShapeXCnt];
	BOOL ArrOKLocalLength[nShapeXCnt];
	BOOL ArrOKDent[nShapeXCnt];
	// esjb++

	POINTF poArrCenter[nShapeXCnt];	//shpaeArea 내부의 Matching Center
	BOOL bMatchOK[nShapeXCnt];   //Matching OK

	BOOL m_bUseShape[nShapeXCnt];	//형상검사사용
	BOOL m_bUseInner[nShapeXCnt];	//내부검사사용
	BOOL m_bUseExist[nShapeXCnt];	//존재유무검사사용

	POINTF ArrStartJointP[nShapeXCnt];
	POINTF ArrEndJointP[nShapeXCnt];

	BOOL RstOKWrForeignCnt;            // Warning Foreign Count 결과
	BOOL ArrOKWrForeignW[nShapeXCnt];  // Warning Foreign 단축검사결과
	BOOL ArrOKWrForeignL[nShapeXCnt];  // Warning Foreign 장축검사결과
	short RstWrForeignCnt;             // Warning Foreign Count 값 (Total)
	short ArrWrForeignCnt[nShapeXCnt]; // Warning Foreing Count 값 (Roi)
	float ArrWrForeignW[nShapeXCnt];   // Warning Foreign 단축 값
	float ArrWrForeignL[nShapeXCnt];   // Warning Foreign 장축 값

	// 추가: 형상용 배열 (Shape 전용 Rect 및 NG 개수)
	short nShapeNGCnt;//Shape NG blob개수
	RECT rcArrShapeRect[nShapeXCnt];   // 형상검사용 Rect
	int m_rcArrShapeNGArrRectCnt[nShapeXCnt]; // 형상검사용 ROI별 NG개수
	short StripeRectCnt;//Stripe Rect개수
	RECT rcArrStripeRect[nShapeXCnt];	//현재 shpaeArea의 Stripe 영역 위치

	BOOL bAIOK;      //Algorithm AI OK
	BOOL ArrAIOK[nShapeXCnt];   //AI OK
	float stdAIScore;   //AI OK Score standard
	float ArrAIScore[nShapeXCnt];   //AI OK Score
	BOOL ArrROIAIOK[nShapeXCnt];     //ROI별 AI검사결과

	tagRstAlgoShapeX()
	{
		isOK = RstOKWrForeignCnt = FALSE;
		nNGAreaRoiCnt = nROICnt = RstWrForeignCnt = 0;
		memset(nMatchingIdx, -1, sizeof(int) * nShapeXCnt);
		memset(nMatchingSc, 0, sizeof(float) * nShapeXCnt);
		memset(ArrROIOK, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrOKShape, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrOKInner, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrOKExist, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrOKShift, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrOKForeignW, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrOKForeignL, 0, sizeof(BOOL) * nShapeXCnt);

		memset(ArrShapeArea, 0, sizeof(float) * nShapeXCnt);
		memset(ArrInnerArea, 0, sizeof(float) * nShapeXCnt);
		memset(ArrExistArea, 0, sizeof(float) * nShapeXCnt);
		memset(ArrShiftX, 0, sizeof(float) * nShapeXCnt);
		memset(ArrShiftY, 0, sizeof(float) * nShapeXCnt);
		memset(ArrForeignW, 0, sizeof(float) * nShapeXCnt);
		memset(ArrForeignL, 0, sizeof(float) * nShapeXCnt);

		memset(rcArrRect, 0, sizeof(RECT) * nShapeXCnt);
		memset(m_rcArrNGArrRectCnt, 0, sizeof(int) * nShapeXCnt);
		memset(rcArrNGRect, 0, sizeof(RECT) * nShapeXCnt);
		memset(m_nArrMaxFreqValue1, 0, sizeof(int) * nShapeXCnt);
		memset(m_nArrMaxFreqValue2, 0, sizeof(int) * nShapeXCnt);

		//추가변수
		memset(ArrOKExistShape, 0, sizeof(BOOL) * nShapeXCnt);
		memset(m_bUseExistShape, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrExistShapeArea, 0, sizeof(float) * nShapeXCnt);

		// sjb++ 250408.[22] ~PInspalgo_Def.h~
		memset(ArrLocalLength, 0, sizeof(float) * nShapeXCnt);
		memset(ArrDent, 0, sizeof(float) * nShapeXCnt);
		memset(ArrDent2, 0, sizeof(float) * nShapeXCnt);
		memset(ArrOKLocalLength, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrOKDent, 0, sizeof(BOOL) * nShapeXCnt);
		// esjb++ 250408.[22]

		memset(poArrCenter, 0, sizeof(POINTF) * nShapeXCnt);
		memset(bMatchOK, 0, sizeof(BOOL) * nShapeXCnt);

		isInsp = FALSE;
		memset(m_bUseExist, 0, sizeof(BOOL) * nShapeXCnt);
		memset(m_bUseInner, 0, sizeof(BOOL) * nShapeXCnt);
		memset(m_bUseShape, 0, sizeof(BOOL) * nShapeXCnt);

		memset(ArrStartJointP, 0, sizeof(POINTF) * nShapeXCnt);
		memset(ArrEndJointP, 0, sizeof(POINTF) * nShapeXCnt);

		memset(ArrOKWrForeignW, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrOKWrForeignL, 0, sizeof(BOOL) * nShapeXCnt);
		memset(ArrWrForeignCnt, 0, sizeof(short) * nShapeXCnt);
		memset(ArrWrForeignW, 0, sizeof(float) * nShapeXCnt);
		memset(ArrWrForeignL, 0, sizeof(float) * nShapeXCnt);

		// 추가: 형상용 배열 초기화
		nShapeNGCnt = 0;
		memset(rcArrShapeRect, 0, sizeof(RECT) * nShapeXCnt);
		memset(m_rcArrShapeNGArrRectCnt, 0, sizeof(int) * nShapeXCnt);
		StripeRectCnt = 0;
		memset(rcArrStripeRect, 0, sizeof(RECT) * nShapeXCnt);

		bAIOK = FALSE;      //Algorithm AI OK
		memset(ArrAIOK, FALSE, sizeof(BOOL) * nShapeXCnt);
		stdAIScore = 0;   //AI OK Score standard
		memset(ArrAIScore, 0, sizeof(float) * nShapeXCnt);
		memset(ArrROIAIOK, FALSE, sizeof(BOOL) * nShapeXCnt);
	}
}RstAlgoShapeX;
//typedef struct tagRstAlgoShapeX	// ShapeX 검사 결과
//{
//	BOOL isInsp;//skip:FALSE, Insp:TRUE
//	BOOL isOK;//Algo ok
//	   	   
//	short nROICnt;      //shapePadROI개수
//	short nNGAreaRoiCnt;//NG blob개수
//	int nMatchingIdx[nShapeXCnt];    //Matching Model Index;
//	float nMatchingSc[nShapeXCnt];   //Matching score
//	BOOL ArrROIOK[nShapeXCnt];     //ROI검사결과
//	BOOL ArrOKShape[nShapeXCnt];   //형상검사결과
//	BOOL ArrOKInner[nShapeXCnt];   //내부검사결과
//	BOOL ArrOKExist[nShapeXCnt];   //존재유무검사결과
//	BOOL ArrOKShift[nShapeXCnt];   //shift검사결과
//	float ArrShapeArea[nShapeXCnt];//형상검사 NG Area
//	float ArrInnerArea[nShapeXCnt];//내부검사 NG Area
//	float ArrExistArea[nShapeXCnt];//존재유무 NG Area
//	float ArrShiftX[nShapeXCnt];    //shift 값
//	float ArrShiftY[nShapeXCnt];    //shift 값
//	RECT rcArrRect[nShapeXCnt];	//현재 shpaeArea의 Rect
//	int m_rcArrNGArrRectCnt[nShapeXCnt];   //ROI별 NG개수
//	RECT rcArrNGRect[nShapeXCnt];	//NG위치
//	int m_nArrMaxFreqValue1[nShapeXCnt];   //foreign1 평균
//	int m_nArrMaxFreqValue2[nShapeXCnt];   //foreign2 평균
//
//	BOOL ArrOKExistShape[nShapeXCnt];   //존재검사영역 형상검사결과
//	float ArrExistShapeArea[nShapeXCnt];//존재검사영역 형상검사 NG Area
//
//	POINTF poArrCenter[nShapeXCnt];	//shpaeArea 내부의 Matching Center
//	BOOL bMatchOK[nShapeXCnt];   //Matching OK
//
//	tagRstAlgoShapeX()
//	{
//		isOK = FALSE;
//		nNGAreaRoiCnt = nROICnt = 0;
//		memset(nMatchingIdx, -1, sizeof(int) * nShapeXCnt);
//		memset(nMatchingSc, 0, sizeof(float) * nShapeXCnt);
//		memset(ArrROIOK, 0, sizeof(BOOL) * nShapeXCnt);
//		memset(ArrOKShape, 0, sizeof(BOOL) * nShapeXCnt);
//		memset(ArrOKInner, 0, sizeof(BOOL) * nShapeXCnt);
//		memset(ArrOKExist, 0, sizeof(BOOL) * nShapeXCnt);
//		memset(ArrOKShift, 0, sizeof(BOOL) * nShapeXCnt);
//
//		memset(ArrShapeArea, 0, sizeof(float) * nShapeXCnt);
//		memset(ArrInnerArea, 0, sizeof(float) * nShapeXCnt);
//		memset(ArrExistArea, 0, sizeof(float) * nShapeXCnt);
//		memset(ArrShiftX, 0, sizeof(float) * nShapeXCnt);
//		memset(ArrShiftY, 0, sizeof(float) * nShapeXCnt);
//
//		memset(rcArrRect, 0, sizeof(RECT) * nShapeXCnt);
//		memset(m_rcArrNGArrRectCnt, 0, sizeof(int) * nShapeXCnt);
//		memset(rcArrNGRect, 0, sizeof(RECT) * nShapeXCnt);
//		memset(m_nArrMaxFreqValue1, 0, sizeof(int) * nShapeXCnt);
//		memset(m_nArrMaxFreqValue2, 0, sizeof(int) * nShapeXCnt);
//
//		memset(ArrOKExistShape, 0, sizeof(BOOL) * nShapeXCnt);
//		memset(ArrExistShapeArea, 0, sizeof(float) * nShapeXCnt);
//		memset(poArrCenter, 0, sizeof(POINTF) * nShapeXCnt);
//		memset(bMatchOK, 0, sizeof(BOOL) * nShapeXCnt);
//	}
//}RstAlgoShapeX;
