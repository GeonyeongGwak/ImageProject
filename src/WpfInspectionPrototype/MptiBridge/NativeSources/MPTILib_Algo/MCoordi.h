#pragma once

#ifndef __MPTI_DEF_H__
#define __MPTI_DEF_H__

 #include "MPTIDef.h"
// 
#endif

//#ifdef _MPTI_EXPORT

enum eCalibrationCode
{
	MC_NONE,
	MC_CALIB_HEADER,
	MC_CALIB_BODY_X,
	MC_CALIB_BODY_Y,
};

typedef struct tagMcCalib_Header{
	int		col;
	int		row;
} CALIB_HEADER;

typedef struct tagPOINT_64F{
	double x;
	double y;

	// SHKang 2018/07/12
	tagPOINT_64F()
	{
		x = y = 0.;
	}
	tagPOINT_64F(double sx, double sy)
	{
		x = sx;
		y = sy;
	}

} POINT_64F;


class CMCoordi
{
public:
	CMCoordi(void);
	~CMCoordi(void);
private:
	enum TYPECORRECT { eTypeGlobal, eTypeModule, eTypePart};
	// coordinate anchor
	bool m_anchorMachineX;	// false(left), true(right)
	bool m_anchorMachineY;	// false(bottom), true(top)
	bool m_anchorBoardX;	
	bool m_anchorBoardY;
	bool m_anchorAlineX;
	bool m_anchorAlineY;

	// software limit
	double m_machineAreaMinX;
	double m_machineAreaMinY;
	double m_machineAreaMaxX;
	double m_machineAreaMaxY;

	// anchor position in machine
	MPTICoordinateMachine m_boardAlinePos;
	MPTICoordinateMachine m_boardAnchorPos;

	// Distance of Out1 Sensor
	double m_dDistanceOfOut1;
	int m_nMachineFlowType;

	// fiducially absolute coordinate
	double m_dirMachineX;
	double m_dirMachineY;
	double m_dirBoardX;
	double m_dirBoardY;

	// board size
	double m_sizeBoardX;
	double m_sizeBoardY;
	
	// field of view
	int m_fovPixelNumX;
	int m_fovPixelNumY;
	double m_fovPixelResolX;
	double m_fovPixelResolY;
	double m_fovSizeX;
	double m_fovSizeY;

	// fiducial mark on board coordinate
	/*int m_boardFiduNum;*/
	/*MPTICoordinateBoard *m_pStdBoardFiduPos;*/
	/*MPTICoordinateBoard *m_pBoardFiduDelta;*/
	CAtlArray <MPTICoordinateBoard> m_pStdBoardFiduPosOrg;		// Job global origin fiducial 좌표
	CAtlArray <MPTICoordinateBoard> m_pStdBoardFiduPosTeach;	// Job global Teching fiducial 좌표
	CAtlArray <MPTICoordinateBoard> m_pBoardFiduDelta;
	MPTICoordinateCorrector m_boardCoordiCorrector;
	
	// regrading module fiducial
	int m_moduleNum;
	int m_moduleFiduNum;
	MPTICoordinateBoard **m_ppModuleFiduPosOrg;
	MPTICoordinateBoard **m_ppModuleFiduPosTeach;
	MPTICoordinateBoard **m_ppModuleFiduDelta;
	MPTICoordinateCorrector *m_pModuleCoordiCorrector;

	// regrading part fiducial
	int m_partNum;
	int m_partFiduNum;
	MPTICoordinateBoard ***m_pStdPartFiduPos;
	MPTICoordinateBoard ***m_pPartFiduDelta;
	MPTICoordinateCorrector **m_pPartCoordiCorrector;
private:
	eCalibrationCode m_eCalibCode;

public:
	int SetAnchorMachine(int anchor);
	int SetAnchorBoard(int anchor);
	int SetAnchorAline(int anchor);

	int SetSizeBoard(double sizeX, double sizeY);
	int SetBoardAlinePos(MPTICoordinateMachine *pPos);

	int SetDistanceOfOut1(double dDistanceX, int nMachineFlowType);

	int GetBoardSize(double *sizeX, double *sizeY);
	int GetBoardAlinePos(MPTICoordinateMachine *pPos);

	int GenerateCoordinate();
	void GetFiducialInfo(int nFiduGroup, int nFiduCnt, double *nOrgX, double *nOrgY, double *nOffsetedX, double *nOffsetedY/*, double *nOffsetX, double *nOffsetY, double *nDelta*/);

	double GetTheta_BoardFiduOrg();	// 동일 Y좌표를 가진 2 Board Fiducial의 Scan당시 Theta와 현재 검사된 Theta 더해진 Theta 반환
private:
	int CalcCorrector(MCOORDI_B stdPos1, MCOORDI_B stdPos2, MCOORDI_B delta1, MCOORDI_B delta2, LP_MCOORDI_COORECTOR lpCorrector);
	int CalcCorrector2(TYPECORRECT eTypeCorrect, LP_MCOORDI_COORECTOR lpCorrector, int idxModule=-1, int idxPart=-1);
	int CalcCorrectorNew(LP_MCOORDI_COORECTOR lpCorrector);
	int CorrectCoordinate(MCOORDI_B pos, MCOORDI_COORECTOR corrector, LP_MCOORDI_B lpPos);
	int ReverseCorrectCoordinate(MCOORDI_B pos, MCOORDI_COORECTOR corrector, LP_MCOORDI_B lpPos);

public:
	int CreateBoardFiducials();
	int DeleteSubFiducials();
	
	//////////////////////////////////////////////////////////////////////////
	// 사용 안함
	int GetBoardFiducialNumber(int *fiduNum);
	int GetSubFiducialNumber(int *moduleNum, int *moduleFiduNum, int *partNum, int *partFiduNum);
	//////////////////////////////////////////////////////////////////////////

	int SetBoardFiducialGlobal( int idxFidu, MPTICoordinateBoard *pPos, BOOL bOrigin=TRUE );

private:
	//////////////////////////////////////////////////////////////////////////
	// Matrix Calibration
	// by. sangmin 2014.08.25
	void LoadMatrixCalibrationFile(CString fileName, int mode, int nLaneNum);

	void Set_MC_CalibHeader(char * strData, int mode, int nLaneNum);
	void Set_Mc_Calib_BodyX(char * strData, int i, int mode, int nLaneNum);
	void Set_Mc_Calib_BodyY(char * strData, int i, int mode, int nLaneNum);
	void Set_Mc_Calib_Clear_BodyX(char * strData, int i, int mode, int nLaneNum);
	void Set_Mc_Calib_Clear_BodyY(char * strData, int i, int mode, int nLaneNum);

	//void CalcObjToRef( double srcX, double srcY, double* dstX, double* dstY );
	//int CalcRefToObj( double srcX, double srcY, double* dstX, double* dstY );

	CALIB_HEADER m_calibHeader[2];
	POINT_64F	**m_calib_ref;
	POINT_64F	**m_calib_obj;
	POINT_64F	**m_calib_Rear_ref;
	POINT_64F	**m_calib_Rear_obj;

	
	float m_fRef_offsetX[2];
	float m_fRef_offsetY[2];
	float m_fObj_offsetX[2];
	float m_fObj_offsetY[2];

	public:
		int CalcRefToObj( double srcX, double srcY, double* dstX, double* dstY, int nLaneNum );
		int CalcObjToRef( double srcX, double srcY, double* dstX, double* dstY, int nLaneNum );
	//////////////////////////////////////////////////////////////////////////

public:
	int CreateSubFiducials(int moduleNum, int partNum);
	int DeleteBoardFiducials();
	
	int SetStdBoardFiducialPos(int idxFidu, MPTICoordinateBoard *pPosOrg, MPTICoordinateBoard *pPosTeach);
	int SetStdModuleFiducialPos(int idxModule, int idxFidu, MPTICoordinateBoard *pPosOrg, MPTICoordinateBoard *pPosTeach);
	int SetStdPartFiducialPos(int idxModule, int idxPart, int idxFidu, MPTICoordinateBoard *pPos);
	
	int SetBoardFiducialDelta(int idxFidu, MPTICoordinateBoard *pDelta);
	int SetModuleFiducialDelta(int idxModule, int idxFidu, MPTICoordinateBoard *pDelta);	
	int SetPartFiducialDelta(int idxModule, int idxPart, int idxFidu, MPTICoordinateBoard *pDelta);

	int ClearAllCoordiCorrector();
	int ClearBoardCoordiCorrector();
	int ClearModuleCoordiCorrector(int idxModule);
	int ClearPartCoordiCorrector(int idxModule, int idxPart);

	int GenerateAllCoordiCorrector();
	int GenerateBoardCoordiCorrector();
	int GenerateModuleCoordiCorrector(int idxModule);
	int GeneratePartCoordiCorrector(int idxModule, int idxPart);

	// ################################################################################################################################
	// SHKang 2018/07/12
	int CorrectCoordinate_fromfidu(int nIndex, POINT_64F FiduPos, POINT_64F delta, double dTheta, POINT_64F srcPos, POINT_64F & dstPos);
	int CorrectCoordinate_both(int nFiduGroupId, MCOORDI_B pos, LP_MCOORDI_B lpPos, double dInversion = 1., BOOL bTheta = TRUE);
	int CorrectCoordinate_Cal(int nFiduNum, POINTF* poSrc, POINTF* poDelta, double dTheta, POINTF* poDst, POINTF* poFidu, POINTF* poData = NULL, double dInversion = 1., int nMode = 0);
	int GetCorrectIDX(int nFiduCnt, POINTF* poSrc, POINTF poDst, int* nArrIDX, int nType = 0);
	int CorrectCoordinate_FOV(POINT_64F FiduPos, POINT_64F delta, double dTheta, POINT_64F srcCenterPos, POINT_64F & dstPos);
	void CalcDelta(int nFiduCnt, POINTF* poArrFidu, POINTF* poDst, double dTheta, POINTF* poDelta);
	double CalcTheta(POINTF poPos_T1, POINTF poPos_T2, POINTF poPos_R1, POINTF poPos_R2);
	double GetGradient(bool bHorizon, POINTF poPos1, POINTF poPos2, double *dA, double *dB);
	double CalcDistence(POINTF SrcPos, POINTF DstPos);
	// ################################################################################################################################
	int CalcCorrect_ThetaDeltaScale(int nCnt, MCOORDI_B* poArr_O, MCOORDI_B* poArr_T, double *dTheta_ref, double *dDeltaX_ref, double *dDeltaY_ref, double * dScaleX_ref, double * dScaleY_ref);
	int CorrectCoordinate_Cal3(MCOORDI_B* poDst, MPTICoordinateCorrector * lpCorrector, double dInversion, BOOL bTheta = TRUE);
	int CorrectCoordinate_Cal3(MCOORDI_B* poDst, double dDeltaX, double dDeltaY, double dTheta, double dScaleX, double dScaleY, MCOORDI_B poFiduCenter, double dInversion);
	// SHW 2019/04/11
	MCOORDI_B GetCenterPos(int nCnt, MCOORDI_B* poArrPos);
	double CalSigma(int nCnt, double* fArrX, int nPow = 1);
	double CalSigma(int nCnt, double* fArrX, double* fArrY);
	double CalTheta(int nCnt, double * dArrX_O, double * dArrY_O, double * dArrX_T, double * dArrY_T);

	int GetCorrectJobPosOff(MPTICoordinateCorrector sData, MPTICoordinateBoard SrcPos, MPTICoordinateBoard *pResPos, double dInversion);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// synopsis : Get Correction Board position from Global or module fiducial
	// return value : 
	//	 integer : -1 is pos == NULL or lpPos == NULL , 0 is success
	// Parameters: 
	//   int nFIduGroupId : 0 - Global fiducial / 1,2,... - Module fiducial's group
	//   MPTICoordinateBoard *pPos : Job position (Board coordinate)
	//   MPTICoordinateBoard *pResPos : corrected position from fiducial group
	int Get_CorrectJobPos(int nFiduGroupId, MPTICoordinateBoard SrcPos, MPTICoordinateBoard *pResPos, BOOL bTheta);

	// Inversion function to Get_CorrectBoardPos
	int Get_InverseJobPos(int nFiduGroupId, MPTICoordinateBoard SrcPos, MPTICoordinateBoard *pResPos);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ################################################################################################################################

	int CorrectBoardCoordinate(MPTICoordinateBoard *pPos, MPTICoordinateBoard *pResPos);
	int CorrectModuleCoordinate(int idxModule, MPTICoordinateBoard *pPos, MPTICoordinateBoard *pResPos);
	int CorrectPartoordinate(int idxModule, int idxPart, MPTICoordinateBoard *pPos, MPTICoordinateBoard *pResPos);

	int ReverseCorrectBoardCoordinate(MPTICoordinateBoard *pPos, MPTICoordinateBoard *pResPos);

	bool GetFiduInspCheck();

	int GetBoardCoordiCorrector(MPTICoordinateCorrector *pCorrector);
	int GetModuleCoordiCorrector(int idxModule, MPTICoordinateCorrector *pCorrector);
	int GetPartCoordiCorrector(int idxModule, int idxPart, MPTICoordinateCorrector *pCorrector);

public:
	int SetFovPixelNumber(int numX, int numY);
	int SetFovPixelResolution(double resolX, double resolY);
	int SetFovSize(double sizeX, double sizeY);

	int GetFovPixelNumber(int *numX, int *numY);
	int GetFovPixelResolution(double *resolX, double *resolY);
	int GetFovSize(double *sizeX, double *sizeY);

public:
	int BoardToMachine(MPTICoordinateBoard *pBoard, MPTICoordinateMachine *pMachine, int nLaneNum);
	int MachineToBoard(MPTICoordinateMachine *pMachine, MPTICoordinateBoard *pBoard, int nLaneNum);
	
	// sign transform
	int UnitBoardToMachine(double inX, double inY, double *outX, double *outY);
	int UnitMachineToBoard(double inX, double inY, double *outX, double *outY);
	int UnitScreenToBoard(double inX, double inY, double *outX, double *outY);
	int UnitPixelToMillimeter(double inX, double inY, double *outX, double *outY);
	int UnitMillimeterToPixel(double inX, double inY, double *outX, double *outY);
	int CalcCorrector_Pos(MPTICoordinateCorrector *pCorrector, int nPosCnt, double *dArrX, double *dArrY, double *dArrX_2, double *dArrY_2);
public:
	int m_nCoordiAlgo;
	BOOL m_bNewCoordiAlgo;
	int ReLoadMatrixCalibrationFile(int nLaneNum);
	int InitMatrixCalibration(int nLaneNum);
	float Interpolate(float pt_x, float pt_y, float *src, int row, int col, int inc);
	BOOL m_bRstPCBStopPos;
	double m_dRstPCBStopPos;
	BOOL m_bNotUseScale;
	double m_dInspRate;
};

//#endif //_MPTI_EXPORT

#pragma region exposure_define

// MPTIDLL int MPTI_SetAnchorMachine(int anchor);
// MPTIDLL int MPTI_SetAnchorBoard(int anchor);
// MPTIDLL int MPTI_SetAnchorAline(int anchor);
// MPTIDLL int MPTI_SetBoardAlinePos(MPTICoordinateMachine *pPos);
// MPTIDLL int MPTI_GetBoardAlinePos(MPTICoordinateMachine *pPos);
// MPTIDLL int MPTI_SetDistanceOfOut1(double dDistanceX, int nMachineFlowType);
// MPTIDLL int MPTI_GetBoardSize(double *sizeX, double *sizeY);
// MPTIDLL int MPTI_GenerateCoordinate();
// MPTIDLL int MPTI_SetFiducialNumber(int boardFiduNum, int moduleNum, int moduleFiduNum, int partNum, int partFiduNum);
// MPTIDLL int MPTI_SetStdBoardFiducialPos(int idxFidu, MPTICoordinateBoard *pPosOrg, MPTICoordinateBoard *pPosTeach);
// MPTIDLL int MPTI_SetStdModuleFiducialPos(int idxModule, int idxFidu, MPTICoordinateBoard *pPos, MPTICoordinateBoard *pPosTeach);
// MPTIDLL int MPTI_DeleteFiducials();
// MPTIDLL int MPTI_SetBoardFiducialDelta(int idxFidu, MPTICoordinateBoard *pDelta);
// MPTIDLL int MPTI_SetModuleFiducialDelta(int idxModule, int idxFidu, MPTICoordinateBoard *pDelta);
// MPTIDLL int MPTI_ClearAllCoordiCorrector();
// MPTIDLL int MPTI_ClearBoardCoordiCorrector();	
// MPTIDLL int MPTI_ClearModuleCoordiCorrector(int idxModule);
// MPTIDLL int MPTI_GenerateBoardCoordiCorrector();
// MPTIDLL int MPTI_GenerateModuleCoordiCorrector(int idxModule);
// MPTIDLL int MPTI_GetBoardCoordiCorrector(MPTICoordinateCorrector *pCorrector);
// MPTIDLL int MPTI_GetModuleCoordiCorrector(int idxModule, MPTICoordinateCorrector *pCorrector);
// MPTIDLL int MPTI_SetFovPixelNumber(int numX, int numY);
// MPTIDLL int MPTI_SetFovPixelResolution(double resolX, double resolY);
// MPTIDLL int MPTI_SetFovSize(double sizeX, double sizeY);
// MPTIDLL int MPTI_GetFovPixelNumber(int *numX, int *numY);
// MPTIDLL int MPTI_GetFovPixelResolution(double *resolX, double *resolY);
// MPTIDLL int MPTI_GetFovSize(double *sizeX, double *sizeY);
// MPTIDLL int MPTI_BoardToMachine(MPTICoordinateBoard *pBoard, MPTICoordinateMachine *pMachine, int nLaneNum);
// MPTIDLL int MPTI_MachineToBoard(MPTICoordinateMachine *pMachine, MPTICoordinateBoard *pBoard, int nLaneNum);
// MPTIDLL int MPTI_CalcRefToObj( double dSrcX, double dSrcY, double* outX, double* outY, int nLaneNum );
// MPTIDLL int MPTI_CalcObjToRef( double dSrcX, double dSrcY, double* outX, double* outY, int nLaneNum );
// MPTIDLL int MPTI_UnitBoardToMachine(double inX, double inY, double *outX, double *outY);
// MPTIDLL int MPTI_UnitMachineToBoard(double inX, double inY, double *outX, double *outY);
// MPTIDLL int MPTI_UnitScreenToBoard(double inX, double inY, double *outX, double *outY);
// MPTIDLL int MPTI_UnitPixelToMillimeter(double inX, double inY, double *outX, double *outY);
// MPTIDLL int MPTI_ReLoadMatrixCalibrationFile(int nLaneNum);
// MPTIDLL int MPTI_InitMatrixCalibration(int nLaneNum);
// MPTIDLL float MPTI_Interpolate(float pt_x, float pt_y, float *src, int row, int col, int inc);
// 
// MPTIDLL int MPTI_GetCorrectJobPosOff(MPTICoordinateCorrector sData, MPTICoordinateBoard SrcJobPos, MPTICoordinateBoard *pDstRealPos, double dInversion);
// MPTIDLL bool MPTI_GetFiduInspCheck();
// 
// // SHKang 2018/07/12
// MPTIDLL int MPTI_GetRealPosition(int nFiduGroupId, MPTICoordinateBoard SrcDataPos, MPTICoordinateBoard *pDstRealPos, BOOL bTheta = TRUE);
// MPTIDLL int MPTI_GetDataPosition(int nFiduGroupId, MPTICoordinateBoard SrcRealPos, MPTICoordinateBoard *pDstDataPos);
// MPTIDLL int MPTI_CorrectCoordinate_both(int nFiduCnt, POINTF* poSrc, POINTF* poDelta, double dTheta, POINTF* poDst, POINTF* poCad);
// MPTIDLL int MPTI_CalcCorrect_ThetaDeltaScale(int nCnt, MCOORDI_B* poArr_O, MCOORDI_B* poArr_T, double *dTheta_ref, double *dDeltaX_ref, double *dDeltaY_ref, double * dScaleX_ref, double * dScaleY_ref);
// MPTIDLL int MPTI_CorrectCoordinate_Cal3(MCOORDI_B* poDst, double dDeltaX, double dDeltaY, double dTheta, double dScaleX, double dScaleY, MCOORDI_B poFiduCenter, double dInversion = 1);
// MPTIDLL void MPTI_SetPCBStop(BOOL bUse, double dRst);
// MPTIDLL void MPTI_SetFiduOPT(BOOL bNotUseScale, double dRate);
// MPTIDLL int MPTI_CalcCorrector_Pos(MPTICoordinateCorrector *pCorrector, int nPosCnt, double *dArrX, double *dArrY, double *dArrX_2, double *dArrY_2);
// #pragma endregion exposure_define
// 
// MPTIDLL void MPTI_GetFiducialInfo(int nFiduGroup, int nFiduCnt, double *nOrgX, double *nOrgY, double *nOffsetedX, double *nOffsetedY/*, double *nOffsetX, double *nOffsetY, double *nDelta*/);
