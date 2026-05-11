#include "PInsp_AlgoDistance.h"

//★★ 코드 작성 필요
CPInsp_AlgoDistance::CPInsp_AlgoDistance(void)
{
	m_pInspectionResult = NULL;
}


CPInsp_AlgoDistance::~CPInsp_AlgoDistance(void)
{
}

void CPInsp_AlgoDistance::InitAlgo()
{
	InitVariable();

	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = m_pCPInsp_Algo->GetProc3D();
	m_pProcMilAlgo = m_pCPInsp_Algo->GetProcMil();
	m_resolX = m_pCPInsp_Algo->GetResolX();
	m_resolY = m_pCPInsp_Algo->GetResolY();
}

void CPInsp_AlgoDistance::InitVariable()
{
	m_bUseX = false;
	m_bUseY = false;
	m_bUseDist = false;
	m_bUseAngle = false;
	m_bUseTargetAxes = false;

	m_nAnchorCnt = 0;
	m_nAnchorMode = 0;

	m_dTeachX = 0.;
	m_dTeachY = 0.;
	m_dTeachDist = 0.;

	m_dTeachMinX = 0.;
	m_dTeachMaxX = 0.;
	m_dTeachMinY = 0.;
	m_dTeachMaxY = 0.;
	m_dTeachMinDist = 0.;
	m_dTeachMaxDist = 0.;

	m_dTeachOffsetX = 0.;
	m_dTeachOffsetY = 0.;
	m_dTeachOffsetDist = 0.;
	// 다른 알고리즘의 정보 가리키는 포인터라서 delete하면 안됨
	m_pTargetInspAlgo = nullptr;
	m_pTargetAlgoRst = nullptr;

	m_pTargetAlignRst.Init();
	for (int i = 0; i < DISTANCE_ANCHOR_LIMIT; i++)
	{
		// 다른 알고리즘의 정보 가리키는 포인터라서 delete하면 안됨
		m_pArrAnchorInspAlgo[i] = nullptr;
		m_arrAnchorAlgoRst[i] = nullptr;

		m_arrAnchorAlignRst[i].Init();
	}
	m_nPartAngle = 0;
	m_ptTargetPnt.x = m_ptTargetPnt.y = 0;
}

unsigned long long CPInsp_AlgoDistance::GetInspAlgoData()
{
	unsigned long long ret = -1;
	ret = eSPCAlgoDistance;
	//shw Delete Xret = eSPCAlgoDistance;
	return ret;
}

int CPInsp_AlgoDistance::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	//★★ 코드 수정 필요
	int nCurrentNgType = 0;
	if (nWndType == eINSP_ALIGN)
	{
		eWholeNgTypeTemp[TypeAlignWrong] = e_NG;
		nCurrentNgType = TypeAlignWrong;
	}
	else
	{
		RstAlgoDistance * rstAlgo = (RstAlgoDistance *)vRstInspAlgo;

		if (!rstAlgo->m_bOKTarget && !rstAlgo->m_bArrOKAnchor[0] && !rstAlgo->m_bArrOKAnchor[1] // DISTANCE_ANCHOR_LIMIT 값 변경 시 반영 필요
			&& !rstAlgo->m_bOKX && !rstAlgo->m_bOKY && !rstAlgo->m_bOKDist && !rstAlgo->m_bOKAngle)
		{
			eWholeNgTypeTemp[TypeMountMissing] = e_NG;
			nCurrentNgType = TypeMountMissing;
		}

		else if (!rstAlgo->m_bOKX || !rstAlgo->m_bOKY || !rstAlgo->m_bOKDist)
		{
			eWholeNgTypeTemp[TypeMountShift] = e_NG;
			nCurrentNgType = TypeMountShift;
		}
		else if (!rstAlgo->m_bOKAngle)
		{
			eWholeNgTypeTemp[TypeMountAngle] = e_NG;
			nCurrentNgType = TypeMountAngle;
		}
	}

	return nCurrentNgType;
}

bool CPInsp_AlgoDistance::InspWindowArea(int nType)
{
	//★★ 코드 수정 필요
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoDistance::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	//★★ 코드 수정 필요
	BOOL bResult = FALSE;

	RstAlgoDistance * rst = (RstAlgoDistance *)sRstAlgo;
	if (!m_pProcMilAlgo || rst == NULL)
		return bResult;
	if (sInspAlgo.m_eAlgoType != eAlgoDistance)
		return bResult;

	//shw Delete Xif (sInspAlgo.m_eAlgoType != eAlgoDistance)#@return bResult;
	memset(rst, 0, (sizeof(RstAlgoDistance)));
	AlgoDistance * pInsp = (AlgoDistance *)sInspAlgo.m_ptrInspAlgoParam;

	if (g_pInspMng->GetInspectionResultRef(m_pInspectionResult) == eIMSG_FAIL)
		return bResult;

	m_nParamArraySize = g_pInspMng->GetInspPartParamSize();
	m_pParamArray = g_pInspMng->GetInspPartParam();

	double angle = g_pInspMng->GetInspPartInfo()->angle;
	if (angle < 0)
		angle += 360;
	else if (angle >= 360)
		angle -= 360;
	m_nPartAngle = int(angle + 0.5);
	if (m_nPartAngle % 90 == 0)
		m_nPartAngle /= 90;
	else
		m_nPartAngle = 0;
	bool bParamSetResult = SetArrDataToParams(pInsp->m_ArrDataN, pInsp->m_ArrDataF, rst);
	if (!bParamSetResult)
		return bResult;

	bool bFindTargetAnchorResult = GetTargetAnchorInfo(pInsp, stAlgoParam.m_arrAlignRes, rst);
	if (!bFindTargetAnchorResult)
		return bResult;

	if (m_nAnchorMode == m_eDistanceAnchorMode_LineDistance && m_pTargetInspAlgo->m_eAlgoType != eAlgoLine && m_pArrAnchorInspAlgo[0]->m_eAlgoType != eAlgoLine)
	{
		rst->m_bOKTarget = rst->m_bArrOKAnchor[0] = false;
		return bResult;
	}
	if (m_bUseTargetPnt)
	{
		if (stAlgoParam.m_nAlignCnt > 0)
		{
			double dOffsetX(0), dOffsetY(0);
			CorrectCoordinate(stAlgoParam.m_nWndIndex, stAlgoParam.m_sAlignRes, sInspAlgo, stAlgoParam.m_nAlignCnt, dOffsetX, dOffsetY);
			m_ptTargetPnt.x += (float)dOffsetX;
			m_ptTargetPnt.y += (float)dOffsetY;
		}
	}


	bool bGetResultOK = SetRstItems(pInsp, stAlgoParam.m_arrAlignRes, rst);
	if (!bGetResultOK)
		return bResult;

	std::vector<PIAL::_MCOORDI_B> poModel, poObject;
	poModel.reserve(m_nAnchorCnt);
	poObject.reserve(m_nAnchorCnt);
	bool nonZeroExist = false;

	double dstX(0), dstY(0);
	for (int i = 0; i < m_nAnchorCnt; i++)
	{
		POINTF poAnchorT =
		{
			m_arrAnchorAlignRst[i].centerX,
			m_arrAnchorAlignRst[i].centerY
		};

		poObject.emplace_back(PIAL::_MCOORDI_B(poAnchorT.x * 1000.0, poAnchorT.y * 1000.0));
		poModel.emplace_back(PIAL::_MCOORDI_B(pInsp->m_arrAnchorPos[i].x, pInsp->m_arrAnchorPos[i].y));

		//GetAngleAppliedCoord(poAnchorT.x, poAnchorT.y, dstX, dstY, m_nPartAngle);
		rst->m_pArrAnchor[i] = poAnchorT;

		if (!nonZeroExist && (pInsp->m_arrAnchorPos[i].x != 0 || pInsp->m_arrAnchorPos[i].y != 0))
			nonZeroExist = true;
	}

	double dTheta(0), dDeltaX(0), dDeltaY(0), dScaleX(0), dScaleY(0);
	if (nonZeroExist)
	{
		// nCnt = Fiducial Count
		// Arr_O = Fiducial Original Position
		std::vector<double> dArrX_O(m_nAnchorCnt);
		std::vector<double> dArrY_O(m_nAnchorCnt);
		std::vector<double> dArrD_O(m_nAnchorCnt);
		std::vector<double> dArrX_T(m_nAnchorCnt);
		std::vector<double> dArrY_T(m_nAnchorCnt);
		std::vector<double> dArrD_T(m_nAnchorCnt);
		PIAL::_MCOORDI_B poCenter = PIAL::FiducialCoord::GetCenterPos(m_nAnchorCnt, poModel);	// 무게 중심 좌표
		PIAL::_MCOORDI_B poCenter_T = PIAL::FiducialCoord::GetCenterPos(m_nAnchorCnt, poObject);	// Fiducial Real Position 무게 중심 좌표
		double dSumODistSqr(0), dSumTDistSqr(0), dRatio(0);
		int nZeroCntX(0), nZeroCntY(0);
		for (int n = 0; n < m_nAnchorCnt; n++)
		{
			dArrX_O[n] = poModel[n].x - poCenter.x;
			dArrY_O[n] = poModel[n].y - poCenter.y;
			dArrD_O[n] = sqrt((dArrX_O[n] * dArrX_O[n]) + (dArrY_O[n] * dArrY_O[n]));
			dSumODistSqr += dArrD_O[n];
			dArrX_T[n] = poObject[n].x - poCenter_T.x;
			dArrY_T[n] = poObject[n].y - poCenter_T.y;
			dArrD_T[n] = sqrt((dArrX_T[n] * dArrX_T[n]) + (dArrY_T[n] * dArrY_T[n]));
			dSumTDistSqr += dArrD_T[n];
		}
		dRatio = (dSumTDistSqr) / (dSumODistSqr);
		for (int n = 0; n < m_nAnchorCnt; n++)
		{
			dArrX_O[n] *= dRatio;
			dArrY_O[n] *= dRatio;
			dTheta += std::asin((dArrX_T[n] * dArrY_O[n] - dArrY_T[n] * dArrX_O[n]) / (dArrD_O[n] * dRatio * dArrD_T[n])) * 180.0 / PI;
		}
		dTheta /= (double)m_nAnchorCnt;
	}
	else
	{
		//dTheta = g_pMPTI->GetTheta_BoardFiduOrg(); //Fidu 정보 가져올수없음.
	}
		

	POINTF poTarget =
	{
		m_pTargetAlignRst.centerX * 1000.0,
		m_pTargetAlignRst.centerY * 1000.0
	};

	//GetAngleAppliedCoord(poTarget.x / 1000.0, poTarget.y / 1000.0, dstX, dstY, (int)m_nPartAngle);
	rst->m_pTarget = { (float)m_pTargetAlignRst.centerX, (float)m_pTargetAlignRst.centerY };

	POINTF poAnchor = { poObject[0].x, poObject[0].y };

	rst->m_pArrAnchorRotated[0] = poAnchor;

	switch (m_nAnchorMode)
	{
	// Anchor와 Target간의 평면상의 거리
#pragma region Anchor
	case (int)m_eDistanceAnchorMode_Anchor:
	{
		double dTargetX, dTargetY;
		m_proc3d.CorrectCoordinate(poTarget.x, poTarget.y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dTargetX, &dTargetY);

		rst->m_pTargetRotated.x = dTargetX;
		rst->m_pTargetRotated.y = dTargetY;

		if (m_bUseTargetAxes)
		{
			rst->m_dRstX = poAnchor.x - dTargetX;
			rst->m_dRstY = poAnchor.y - dTargetY;
		}
		else
		{
			rst->m_dRstX = dTargetX - poAnchor.x;
			rst->m_dRstY = dTargetY - poAnchor.y;
		}

		rst->m_dRstDist = sqrt(rst->m_dRstX * rst->m_dRstX + rst->m_dRstY * rst->m_dRstY);

		bResult = true;
		break;
	}
#pragma endregion

	// Anchor와 Target간의 평면상의 거리.
	// 단, Anchor - Anchor2 둘을 잇는 직선을 y축, Anchor를 중심으로 하도록 평행이동, 회전변환해서 변환된 x, y 거리를 계산
#pragma region TwoAnchor
	case (int)m_eDistanceAnchorMode_TwoAnchor:
	{
		POINTF poAnchor2, vecY, vecNewTarget;

		poAnchor2.x = poObject[1].x;
		poAnchor2.y = poObject[1].y;

		// vecY는 Anchor[0]으로부터 Anchor[1]까지의 벡터 (Y축)
		// vecNewTarget은 Anchor[0]으로부터 Target까지의 벡터 (측정할 거리)
		vecY.x = poAnchor2.x - poAnchor.x;
		vecY.y = poAnchor2.y - poAnchor.y;
		vecNewTarget.x = poTarget.x - poAnchor.x;
		vecNewTarget.y = poTarget.y - poAnchor.y;

		// vecY와 vecNewTarget의 내적 값을 vecY의 크기로 나누면, vecNewTarget을 vecY에 수직으로 정사영 시킨 벡터의 크기 값 도출
		// (= 새로운 Y축에 Target좌표에서 내린 수선의 발의 y좌표 = Target의 새로운 y좌표)
		double dot = vecY.x * vecNewTarget.x + vecY.y * vecNewTarget.y;
		rst->m_dRstDist = sqrt(vecNewTarget.x * vecNewTarget.x + vecNewTarget.y * vecNewTarget.y);

		// 내젹이 0이면 두 벡터가 수직 또는 둘 중 하나의 벡터의 크기가 0
		// 어느 경우든 Y축 좌표는 0으로 확정됨 (Anchor 둘이 같은 경우는 Y축이 특정되지 않으므로 고려하지 않음)
		if (dot == 0)
		{
			rst->m_dRstX = rst->m_dRstDist;
			rst->m_dRstY = 0;
		}
		else
		{
			// 위에서 구한 내적으로부터 새로운 X, Y값 계산
			double scalarY = sqrt(vecY.x * vecY.x + vecY.y * vecY.y);
			double distY = dot / scalarY;
			double distX = sqrt(rst->m_dRstDist * rst->m_dRstDist - distY * distY);
			
			rst->m_pArrAnchorRotated[0].x = 0;
			rst->m_pArrAnchorRotated[0].y = 0;
			rst->m_pArrAnchorRotated[1].x = 0;
			rst->m_pArrAnchorRotated[1].y = scalarY;

			// vecY와 수직이면서 변환된 Y축(vecY)에 대응되는 X축 방향의 벡터는 vecY를 vecY이 시작점을 중심으로 -90도 회전하면 되고
			// 따라서 VecY = (a, b)라고 할 때 -> vecX(X축) = (b, -a)이므로
			// vecX와 vecNewTarget의 내적 값이 양수이면, vecNewTarget의 새로운 X좌표가 x축 양의 방향(양수), 음수이면 x축 음의 방향(음수)이다.
			double dotForX = vecY.y * vecNewTarget.x - vecY.x * vecNewTarget.y;
			if (dotForX < 0)
				distX *= -1;

			rst->m_dRstX = distX;
			rst->m_dRstY = distY;

			rst->m_pTargetRotated.x = distX;
			rst->m_pTargetRotated.y = distY;
		}

		bResult = true;
		break;
	}
#pragma endregion

	// Anchor와 Anchor2의 중간점과 Target간의 거리
#pragma region TwoAnchorCenter
	case (int)m_eDistanceAnchorMode_TwoAnchorCenter:
	{
		double CenterX = 0;
		double CenterY = 0;
		double dAnchor2X, dAnchor2Y;
		double dTargetX, dTargetY;

		// Anchor2를 Anchor 기준으로 Board 틀어진 만큼 보정
		m_proc3d.CorrectCoordinate(poObject[1].x, poObject[1].y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dAnchor2X, &dAnchor2Y);

		rst->m_pArrAnchorRotated[1].x = dAnchor2X;
		rst->m_pArrAnchorRotated[1].y = dAnchor2Y;

		double dCenterX = ((poAnchor.x + dAnchor2X) / 2);
		double dCenterY = ((poAnchor.y + dAnchor2Y) / 2);

		// Target을 Anchor1 기준으로 Board 틀어진 만큼 보정
		m_proc3d.CorrectCoordinate(poTarget.x, poTarget.y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dTargetX, &dTargetY);

		rst->m_pTargetRotated.x = dTargetX;
		rst->m_pTargetRotated.y = dTargetY;

		rst->m_dRstX = dTargetX - dCenterX;
		rst->m_dRstY = dTargetY - dCenterY;
		rst->m_dRstDist = sqrt(rst->m_dRstX * rst->m_dRstX + rst->m_dRstY * rst->m_dRstY);

		bResult = true;
		break;
	}
#pragma endregion

	// Anchor로 이루어진 rect의 중점과 Target 간의 거리
#pragma region RectAnchor
	case (int)m_eDistanceAnchorMode_RectAnchor:
	{
		double CenterX = 0;
		double CenterY = 0;

		POINTF *poAnchorT = rst->m_pArrAnchorRotated;
		double dAnchor2X, dAnchor2Y;
		double dTargetX, dTargetY;

		// Anchor2를 Anchor 기준으로 Board 틀어진 만큼 보정
		m_proc3d.CorrectCoordinate(poObject[1].x, poObject[1].y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dAnchor2X, &dAnchor2Y);

		poAnchorT[1].x = dAnchor2X;
		poAnchorT[1].y = dAnchor2Y;

		POINTF slopeVec = { dAnchor2X - poAnchor.x, dAnchor2Y - poAnchor.y };

		double rectWidth = 0, rectHeight = 0, rectTheta = 0;
		bool bWrongAnchorPos = false;

		// 변수명이 (...)R 인 것들은 밑변 직선과 수직인 직선과 관련된 변수임
		if (slopeVec.x != 0 && slopeVec.y != 0)	// 밑변 직선이 수직이나 수평이 아닌 경우
		{
			//slopeVec 벡터의 크기와 기울기 값
			double slopeSize = sqrt(slopeVec.x * slopeVec.x + slopeVec.y * slopeVec.y);
			double slope = slopeVec.y / slopeVec.x, slopeR = -slopeVec.x / slopeVec.y;

			// y절편 계산
			double yIntercept = dAnchor2Y - slope * dAnchor2X;
			double yIntercept1R = poAnchor.y - slopeR * poAnchor.x, yIntercept2R = dAnchor2Y - slopeR * dAnchor2X;

			double minYIntercept = yIntercept, maxYIntercept = yIntercept;
			double minYInterceptR, maxYInterceptR;

			if (yIntercept1R >= yIntercept2R)
			{
				maxYInterceptR = yIntercept1R;
				minYInterceptR = yIntercept2R;
			}
			else
			{
				maxYInterceptR = yIntercept2R;
				minYInterceptR = yIntercept1R;
			}

			for (int i = 2; i < m_nAnchorCnt; i++)
			{
				double dAnchorTX, dAnchorTY, yInterceptT, yInterceptTR;

				// Anchor2를 Anchor 기준으로 Board 틀어진 만큼 보정
				m_proc3d.CorrectCoordinate(poObject[i].x, poObject[i].y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dAnchorTX, &dAnchorTY);

				poAnchorT[i].x = dAnchorTX;
				poAnchorT[i].y = dAnchorTY;

				yInterceptT = dAnchorTY - slope * dAnchorTX;
				yInterceptTR = dAnchorTY - slopeR * dAnchorTX;

				if (yInterceptT > maxYIntercept)
					maxYIntercept = yInterceptT;
				else if (yInterceptT < minYIntercept)
					minYIntercept = yInterceptT;

				if (yInterceptTR > maxYInterceptR)
					maxYInterceptR = yInterceptTR;
				else if (yInterceptTR < minYInterceptR)
					minYInterceptR = yInterceptTR;
			}

			if (maxYIntercept != yIntercept && minYIntercept != yIntercept)
				bWrongAnchorPos = true;

			rectWidth = (maxYInterceptR - minYInterceptR) * slopeVec.y / slopeSize;
			rectHeight = (maxYIntercept - minYIntercept) * slopeVec.x / slopeSize;

			if (rectWidth < 0) rectWidth = -rectWidth;
			if (rectHeight < 0) rectHeight = -rectHeight;

			CenterX = (maxYInterceptR + minYInterceptR - (maxYIntercept + minYIntercept)) / 2.0 / (slope - slopeR);
			CenterY = slope * CenterX + (maxYIntercept + minYIntercept) / 2.0;
			rectTheta = atan(slope) * 180.0 / PI;
		}
		else if (slopeVec.x == 0 && slopeVec.y != 0)	// 수직
		{
			double minX = dAnchor2X, maxX = dAnchor2X;
			double minY, maxY;

			if (slopeVec.y > 0)
			{
				minY = poAnchor.y;
				maxY = dAnchor2Y;
			}
			else
			{
				maxY = poAnchor.y;
				minY = dAnchor2Y;
			}

			for (int i = 2; i < m_nAnchorCnt; i++)
			{
				double dAnchorTX, dAnchorTY;

				// Anchor2를 Anchor 기준으로 Board 틀어진 만큼 보정
				m_proc3d.CorrectCoordinate(poObject[i].x, poObject[i].y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dAnchorTX, &dAnchorTY);

				poAnchorT[i].x = dAnchorTX;
				poAnchorT[i].y = dAnchorTY;

				if (minX > dAnchorTX)
					minX = dAnchorTX;
				if (maxX < dAnchorTX)
					maxX = dAnchorTX;

				if (minY > dAnchorTY)
					minY = dAnchorTY;
				if (maxY < dAnchorTY)
					maxY = dAnchorTY;
			}

			if (minX != dAnchor2X && maxX != dAnchor2X)
				bWrongAnchorPos = true;

			rectHeight = maxX - minX;
			rectWidth = maxY - minY;
			CenterX = (maxX + minX) / 2.0;
			CenterY = (maxY + minY) / 2.0;
			rectTheta = 90.0;
		}
		else if (slopeVec.x != 0 && slopeVec.y == 0)	// 수평
		{
			double minY = dAnchor2Y, maxY = dAnchor2Y;
			double minX, maxX;

			if (slopeVec.x > 0)
			{
				minX = poAnchor.x;
				maxX = dAnchor2X;
			}
			else
			{
				maxX = poAnchor.x;
				minX = dAnchor2X;
			}

			for (int i = 2; i < m_nAnchorCnt; i++)
			{
				double dAnchorTX, dAnchorTY;

				// Anchor2를 Anchor 기준으로 Board 틀어진 만큼 보정
				m_proc3d.CorrectCoordinate(poObject[i].x, poObject[i].y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dAnchorTX, &dAnchorTY);

				poAnchorT[i].x = dAnchorTX;
				poAnchorT[i].y = dAnchorTY;

				if (minX > dAnchorTX)
					minX = dAnchorTX;
				if (maxX < dAnchorTX)
					maxX = dAnchorTX;

				if (minY > dAnchorTY)
					minY = dAnchorTY;
				if (maxY < dAnchorTY)
					maxY = dAnchorTY;
			}

			if (minY != dAnchor2Y && maxY != dAnchor2Y)
				bWrongAnchorPos = true;

			rectWidth = maxX - minX;
			rectHeight = maxY - minY;
			CenterX = (maxX + minX) / 2.0;
			CenterY = (maxY + minY) / 2.0;
			rectTheta = 0.0;
		}
		else  // slopeVec.x == 0 && slopeVec.y == 0 -> 밑변이 특정되지 않기 때문에 검사불가. NG
		{
			bWrongAnchorPos = true;
			for (int i = 2; i < m_nAnchorCnt; i++)
			{
				double dAnchorTX, dAnchorTY;

				// Anchor2를 Anchor 기준으로 Board 틀어진 만큼 보정
				m_proc3d.CorrectCoordinate(poObject[i].x, poObject[i].y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dAnchorTX, &dAnchorTY);

				poAnchorT[i].x = dAnchorTX;
				poAnchorT[i].y = dAnchorTY;
			}
		}
		// Target을 Anchor1 기준으로 Board 틀어진 만큼 보정
		m_proc3d.CorrectCoordinate(poTarget.x, poTarget.y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dTargetX, &dTargetY);

		rst->m_pTargetRotated.x = dTargetX;
		rst->m_pTargetRotated.y = dTargetY;

		double targetTheta = m_pTargetAlignRst.theta - dTheta;

		if(!bWrongAnchorPos)	// Rect가 제대로 형성된 경우
		{
			double dx = dTargetX - CenterX;
			double dy = dTargetY - CenterY;
			rst->m_dRstDist = sqrt(dx * dx + dy * dy);
			rst->m_dRstAngle = targetTheta - rectTheta;

			if (rst->m_dRstAngle > 90.0)
				rst->m_dRstAngle -= 180.0;

			else if (rst->m_dRstAngle <= -90.0)
				rst->m_dRstAngle += 180.0;

			if (m_bUseTargetAxes)
			{
				double dTmpTheta = targetTheta * PI / 180.0;
				rst->m_dRstX = (dx * cos(dTmpTheta) + dy * sin(dTmpTheta));
				rst->m_dRstY = (dx * -sin(dTmpTheta) + dy * cos(dTmpTheta));
			}
			else
			{
				double dTmpTheta = rectTheta * PI / 180.0;
				rst->m_dRstX = dx * cos(dTmpTheta) + dy * sin(dTmpTheta);
				rst->m_dRstY = dx * -sin(dTmpTheta) + dy * cos(dTmpTheta);
			}

			m_proc3d.CorrectCoordinate(CenterX, CenterY, poAnchor.x, poAnchor.y, -dTheta, 0, 0, &dstX, &dstY);
			GetAngleAppliedCoord(dstX / 1000.0, dstY / 1000.0, dstX, dstY, m_nPartAngle);

			rst->m_dRstRectCenter[0] = { (float)dstX, (float)dstY };
			rst->m_dRstRectCenter[1] = { (float)CenterX, (float)CenterY };
			rst->m_dRstRectWidth = rectWidth / 1000.0;
			rst->m_dRstRectHeight = rectHeight / 1000.0;
			rst->m_dRstRectTheta = rectTheta;

			bResult = true;
		}
		else
		{
			rst->m_dRstRectWidth = rst->m_dRstRectHeight = -1;

			rst->m_bOKX = rst->m_bOKY = rst->m_bOKDist = rst->m_bOKAngle = TRUE;

			if (m_bUseX) rst->m_bOKX = FALSE;
			if (m_bUseY) rst->m_bOKY = FALSE;
			if (m_bUseDist) rst->m_bOKDist = FALSE;
			if (m_bUseAngle) rst->m_bOKAngle = FALSE;

			return FALSE;
		}

		break;
	}
#pragma endregion
#pragma region LineTheta
	case (int)m_eDistanceAnchorMode_LineTheta:
	{
		RstAlgoLine *targetRst = (RstAlgoLine*)m_pTargetAlgoRst->m_vRstInspAlgo;
		RstAlgoLine *anchorRst = (RstAlgoLine*)m_arrAnchorAlgoRst[0]->m_vRstInspAlgo;
		double rstAngle = targetRst->m_dRstAngle - anchorRst->m_dRstAngle;
		if (rstAngle < 0) rstAngle *= -1;
		if (rstAngle > 90) rstAngle = 180 - rstAngle;
		rst->m_dRstAngle = rstAngle;
		bResult = true;
		break;
	}
#pragma endregion
#pragma region LineDistance
	case (int)m_eDistanceAnchorMode_LineDistance:
	{
		double CenterX = 0;
		double CenterY = 0;
		double dAnchor2X, dAnchor2Y;
		double dTargetX, dTargetY;

		// Anchor2를 Anchor 기준으로 Board 틀어진 만큼 보정
		m_proc3d.CorrectCoordinate(poObject[1].x, poObject[1].y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dAnchor2X, &dAnchor2Y);

		rst->m_pArrAnchorRotated[1].x = dAnchor2X;
		rst->m_pArrAnchorRotated[1].y = dAnchor2Y;

		POINTF slopeVec = { dAnchor2X - poAnchor.x, dAnchor2Y - poAnchor.y };

		// Target을 Anchor1 기준으로 Board 틀어진 만큼 보정
		m_proc3d.CorrectCoordinate(poTarget.x, poTarget.y, poAnchor.x, poAnchor.y, dTheta, 0, 0, &dTargetX, &dTargetY);

		rst->m_pTargetRotated.x = dTargetX;
		rst->m_pTargetRotated.y = dTargetY;

		POINTF A1toTVec = { dTargetX - poAnchor.x, dTargetY - poAnchor.y };
		if (slopeVec.x != 0 && slopeVec.y != 0) // 기울어진 직선인 경우
		{
			//slopeVec 벡터의 크기와 기울기 값
			double slopeSize = sqrt(slopeVec.x * slopeVec.x + slopeVec.y * slopeVec.y);
			double A1toTSize = sqrt(A1toTVec.x * A1toTVec.x + A1toTVec.y * A1toTVec.y);
			double innerProd = (slopeVec.y * A1toTVec.x - slopeVec.x * A1toTVec.y) / slopeSize;

			rst->m_dRstDist = innerProd > 0 ? innerProd : -innerProd;
			rst->m_dRstX = (slopeVec.y > 0 ? slopeVec.y : -slopeVec.y) / slopeSize * rst->m_dRstDist;
			rst->m_dRstY = (slopeVec.x > 0 ? slopeVec.x : -slopeVec.x) / slopeSize * rst->m_dRstDist;
		}
		else if (slopeVec.x == 0 && slopeVec.y != 0) // y축에 평행인 경우
		{
			//slopeVec 벡터의 크기와 기울기 값
			double A1toTSize = dTargetX - poAnchor.x;

			rst->m_dRstX = (A1toTSize > 0 ? A1toTSize : -A1toTSize);
			rst->m_dRstY = 0;
			rst->m_dRstDist = rst->m_dRstX;
		}
		else if (slopeVec.x != 0 && slopeVec.y == 0) // x축에 평행인 경우
		{
			//slopeVec 벡터의 크기와 기울기 값
			double A1toTSize = dTargetY - poAnchor.y;

			rst->m_dRstX = 0;
			rst->m_dRstY = (A1toTSize > 0 ? A1toTSize : -A1toTSize);
			rst->m_dRstDist = rst->m_dRstY;
		}
		else
		{
			rst->m_dRstX = -1;
			rst->m_dRstY = -1;
			rst->m_dRstDist = -1;
		}

		bResult = true;
		break;
	}
#pragma endregion
	default:
		break;
	}
	rst->m_dRstX += m_dTeachOffsetX;
	rst->m_dRstY += m_dTeachOffsetY;
	rst->m_dRstDist += m_dTeachOffsetDist;

	rst->m_bOKX = rst->m_bOKY = rst->m_bOKDist = rst->m_bOKAngle = TRUE;

	if (m_bUseX)
	{
		if (rst->m_dRstX < m_dTeachMinX || rst->m_dRstX > m_dTeachMaxX)
			rst->m_bOKX = FALSE;
	}
	if (m_bUseY)
	{
		if (rst->m_dRstY < m_dTeachMinY || rst->m_dRstY > m_dTeachMaxY)
			rst->m_bOKY = FALSE;
	}
	if (m_bUseDist)
	{
		if (rst->m_dRstDist < m_dTeachMinDist || rst->m_dRstDist > m_dTeachMaxDist)
			rst->m_bOKDist = FALSE;
	}
	if (m_bUseAngle)
	{
		if (rst->m_dRstAngle < -m_dTeachAngle || rst->m_dRstAngle > m_dTeachAngle)
			rst->m_bOKAngle = FALSE;
	}

	if ((pInsp->m_ArrDataN[m_eDistanceN_Data] & m_eDistanceData_CoordinateLog) == m_eDistanceData_CoordinateLog)
	{
		CFile file;
		CFileException fe;
		CMemFile CMFile;
		if (file.Open(_T("D:\\Distance_Coordinate_LOG.txt"), CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate, NULL, &fe) == FALSE)
			return false;

		if(file.GetLength() > 0)
			file.SeekToEnd();

		CArchive ar(&file, CArchive::store);

		CString sLog = _T(""), sTmp = _T(""), partID = _T(""), modelName = g_pInspMng->GetInspPartInfo()->modelName;

		sLog.Format(_T("PartID:\t%d\t"), g_pInspMng->GetInspPartInfo()->nPartIDOrg);
		sTmp.Format(_T("ModelName:\t%s\t"), modelName);
		sLog += sTmp;
		sTmp.Format(_T("Target:\t%f\t%f\t"), rst->m_pTargetRotated.x, rst->m_pTargetRotated.y);
		sLog += sTmp;
		for (int i = 0; i < m_nAnchorCnt; i++)
		{
			sTmp.Format(_T("Anchor_%d:\t%f\t%f\t"), i + 1, rst->m_pArrAnchorRotated[i].x, rst->m_pArrAnchorRotated[i].y);
			sLog += sTmp;
		}
		sLog += _T("\r\n");

		file.Write(sLog.GetString(), sLog.GetLength()*2);
		//ar << sLog;
		//ar << _T("\r\n");

		ar.Close();
		file.Close();
	}

	if (!rst->m_bOKX || !rst->m_bOKY || !rst->m_bOKDist || !rst->m_bOKAngle)
		bResult = FALSE;

	return bResult;
}

BOOL CPInsp_AlgoDistance::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	//★★ 코드 수정 필요
	BOOL bRet = FALSE;

	//RstAlgoDistance * rst = (RstAlgoDistance *)sRstAlgo;
	//pAlignRes->offsetX = (double)-rst->m_fRstShiftX;
	//pAlignRes->offsetY = (double)rst->m_fRstShiftY;
	//pAlignRes->theta = (double)rst->m_fRstA;
	bRet = TRUE;

	return bRet;
}

bool CPInsp_AlgoDistance::SetArrDataToParams(int* ArrDataN, float* ArrDataF, RstAlgoDistance* sRstAlgo)
{
	bool bResult = false;
	try
	{
		m_bUseX = (ArrDataN[m_eDistanceN_Data] & m_eDistanceData_UseX) == m_eDistanceData_UseX;
		m_bUseY = (ArrDataN[m_eDistanceN_Data] & m_eDistanceData_UseY) == m_eDistanceData_UseY;
		m_bUseDist = (ArrDataN[m_eDistanceN_Data] & m_eDistanceData_UseDist) == m_eDistanceData_UseDist;
		m_bUseAngle = (ArrDataN[m_eDistanceN_Data] & m_eDistanceData_UseAngle) == m_eDistanceData_UseAngle;
		m_bUseTargetAxes = (ArrDataN[m_eDistanceN_Data] & m_eDistanceData_UseTargetAxes) == m_eDistanceData_UseTargetAxes;
		m_bUseTargetPnt = (ArrDataN[m_eDistanceN_Data] & m_eDistanceData_UseTargetPnt) == m_eDistanceData_UseTargetPnt;

		m_nAnchorCnt = ArrDataN[m_eDistanceN_AnchorCnt];
		m_nAnchorMode = ArrDataN[m_eDistanceN_AnchorMode];

		switch (m_nAnchorMode)
		{
		case m_eDistanceAnchorMode_Anchor:
			if (m_nAnchorCnt < 1)
				return bResult;
			for(int i = m_nAnchorCnt; i < DISTANCE_ANCHOR_LIMIT; i++)
				sRstAlgo->m_bArrOKAnchor[i] = true;
			break;
		case m_eDistanceAnchorMode_TwoAnchor:
		case m_eDistanceAnchorMode_TwoAnchorCenter:
			if (m_nAnchorCnt < 2)
				return bResult;
			for (int i = m_nAnchorCnt; i < DISTANCE_ANCHOR_LIMIT; i++)
				sRstAlgo->m_bArrOKAnchor[i] = true;
			break;
		case m_eDistanceAnchorMode_RectAnchor:
			if (m_nAnchorCnt < 3)
				return bResult;
			for (int i = m_nAnchorCnt; i < DISTANCE_ANCHOR_LIMIT; i++)
				sRstAlgo->m_bArrOKAnchor[i] = true;
			break;
		case m_eDistanceAnchorMode_LineTheta:
			if (m_nAnchorCnt < 1)
				return bResult;
			for (int i = m_nAnchorCnt; i < DISTANCE_ANCHOR_LIMIT; i++)
				sRstAlgo->m_bArrOKAnchor[i] = true;
			break;
		case m_eDistanceAnchorMode_LineDistance:
			if (m_nAnchorCnt < 1)
				return bResult;
			for (int i = m_nAnchorCnt; i < DISTANCE_ANCHOR_LIMIT; i++)
				sRstAlgo->m_bArrOKAnchor[i] = true;
			break;
		case m_eDistanceAnchorMode_NoneAnchorMode:
			for (int i = m_nAnchorCnt; i < DISTANCE_ANCHOR_LIMIT; i++)
				sRstAlgo->m_bArrOKAnchor[i] = true;
			return bResult;
			break;
		}

		m_dTeachX = ArrDataF[m_eDistanceF_TeachX];
		m_dTeachY = ArrDataF[m_eDistanceF_TeachY];
		m_dTeachDist = ArrDataF[m_eDistanceF_TeachDist];
		m_dTeachAngle = ArrDataF[m_eDistanceF_TeachAngle];

		m_dTeachMinX = m_dTeachX - ArrDataF[m_eDistanceF_TeachTolerX];
		m_dTeachMaxX = m_dTeachX + ArrDataF[m_eDistanceF_TeachTolerX];
		m_dTeachMinY = m_dTeachY - ArrDataF[m_eDistanceF_TeachTolerY];
		m_dTeachMaxY = m_dTeachY + ArrDataF[m_eDistanceF_TeachTolerY];
		m_dTeachMinDist = m_dTeachDist - ArrDataF[m_eDistanceF_TeachTolerDist];
		m_dTeachMaxDist = m_dTeachDist + ArrDataF[m_eDistanceF_TeachTolerDist];

		m_dTeachOffsetX = ArrDataF[m_eDistanceF_TeachOffsetX];
		m_dTeachOffsetY = ArrDataF[m_eDistanceF_TeachOffsetY];
		m_dTeachOffsetDist = ArrDataF[m_eDistanceF_TeachOffsetDist];
		bResult = true;
	}
	catch (exception e)
	{

	}
	catch (...)
	{

	}
	return bResult;
}

bool CPInsp_AlgoDistance::GetTargetAnchorInfo(AlgoDistance *algo, AlignResult* arrAlignRes, RstAlgoDistance* sRstAlgo)
{
	int wndCnt, nFoundAnchorCnt(0);
	if (m_bUseTargetPnt)
	{
		if (m_nPartAngle == 0) 
		{
		m_ptTargetPnt = algo->m_ptTargetPnt;
		}
		else if (m_nPartAngle == 1) // 90
		{
			m_ptTargetPnt.x = algo->m_ptTargetPnt.y;
			m_ptTargetPnt.y = m_pParamArray[0].anyAngleLength - algo->m_ptTargetPnt.x;
		} 
		else if (m_nPartAngle == 2) // 180
		{
			m_ptTargetPnt.x = m_pParamArray[0].anyAngleWidth  - algo->m_ptTargetPnt.x;
			m_ptTargetPnt.y = m_pParamArray[0].anyAngleLength - algo->m_ptTargetPnt.y;
		}
		else if (m_nPartAngle == 3) // 270
		{
			m_ptTargetPnt.x = m_pParamArray[0].anyAngleWidth - algo->m_ptTargetPnt.y;
			m_ptTargetPnt.y = algo->m_ptTargetPnt.x;
		}
		m_ptTargetPnt.x -= m_pParamArray[0].anyAngleWidth / 2.0;
		m_ptTargetPnt.y -= m_pParamArray[0].anyAngleLength / 2.0;
		sRstAlgo->m_bOKTarget = true;
	}

#pragma region MOUNT

	wndCnt = m_pInspectionResult->mountArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->mountParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result

			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->mountResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->mountResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

#pragma endregion MOUNT

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;

	// === ALIGN Window ===

	wndCnt = m_pInspectionResult->alignArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->alignParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result

			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->alignResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->alignResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

	// === ALIGN Window ===

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;

	// === OCR Window ===

	wndCnt = m_pInspectionResult->ocrArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->ocrParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result

			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->ocrResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->ocrResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

	// === OCR Window ===

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;

	// === LEADSOLDER Window ===

	wndCnt = m_pInspectionResult->leadSolderArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->leadSolderParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result

			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->leadSolderResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->leadSolderResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

	// === LEADSOLDER Window ===

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;

	// === SOLDER Window ===

	wndCnt = m_pInspectionResult->solderArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->solderParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result
			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->solderResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->solderResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

	// === SOLDER Window ===

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;

	// === TAB Window ===

	wndCnt = m_pInspectionResult->tabArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->tabParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result

			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->tabResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->tabResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

	// === TAB Window ===

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;

	// === S_BALL Window ===

	wndCnt = m_pInspectionResult->S_BallArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->S_BallParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result

			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->S_BallResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->S_BallResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

	// === S_BALL Window ===

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;

	// === PAD Window ===

	wndCnt = m_pInspectionResult->PadArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->PadParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result

			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->PadResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->PadResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

	// === PAD Window ===

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;

	// === BGA Window ===

	wndCnt = m_pInspectionResult->BGAArraySize;
	for (int i = 0; i < wndCnt; i++)
	{
		if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
			break;
		InspParamTemp *wndParam = &m_pInspectionResult->BGAParamTemp[i];

		CString wndID;
		wndID.Format(wndParam->wndName);
		wndID.Delete(0, 6); // Delete "window"

		int nWndID = _ttoi(wndID);
		int anchorIdx(0);
		bool isTarget = false, isAnchor = false;
		if (!sRstAlgo->m_bOKTarget && algo->m_nTargetWindowID == nWndID)
			isTarget = true;
		else if (nFoundAnchorCnt < m_nAnchorCnt)
		{
			for (int j = 0; j < m_nAnchorCnt; j++)
			{
				if (algo->m_arrAnchorWindowID[j] == nWndID)
				{
					isAnchor = true;
					anchorIdx = j;
					break;
				}
			}
		}
		if (!isTarget && !isAnchor)
			continue;

		int nBeforeAnchorCnt = nFoundAnchorCnt;
		int algoCnt = wndParam->nAlgorithmCnt;			// Algorithm 개수
		for (int j = 0; j < algoCnt; j++)
		{
			InspAlgo *algoResult = &wndParam->vArrAlgoParam[j];			// Algorithm Result

			if (isTarget && algo->m_nTargetAlgoID == algoResult->m_nAlgoId)
			{
				m_pTargetInspAlgo = algoResult;
				m_pTargetAlgoRst = &m_pInspectionResult->BGAResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bOKTarget = true;
				break;
			}
			else if (isAnchor && algo->m_arrAnchorAlgoID[anchorIdx] == algoResult->m_nAlgoId)
			{
				m_pArrAnchorInspAlgo[anchorIdx] = algoResult;
				m_arrAnchorAlgoRst[anchorIdx] = &m_pInspectionResult->BGAResult[i].m_vArrRstInspAlgo[j];
				sRstAlgo->m_bArrOKAnchor[anchorIdx] = true;
				nFoundAnchorCnt++;
				break;
			}
		}

		if (isTarget && !sRstAlgo->m_bOKTarget || isAnchor && nFoundAnchorCnt == nBeforeAnchorCnt)
			return false;
	}

	// === BGA Window ===

	if (sRstAlgo->m_bOKTarget && nFoundAnchorCnt == m_nAnchorCnt)
		return true;
	
	return false;
}

bool CPInsp_AlgoDistance::SetRstItems(AlgoDistance *algo, AlignResult* arrAlignRes, RstAlgoDistance* sRstAlgo)
{
	if (arrAlignRes == NULL)
		return false;

	bool targetX(false), targetY(false), checkOK(true);
	bool isOK = false;
	if (m_bUseTargetPnt)
	{
		targetX = targetY = isOK = true;
		m_pTargetAlignRst.centerX = m_ptTargetPnt.x;
		m_pTargetAlignRst.centerY = -m_ptTargetPnt.y;
	}
	else
		isOK = IsAppropriateResult(m_pTargetAlgoRst, targetX, targetY, -1);
	if (!isOK)
	{
		sRstAlgo->m_bOKTarget = FALSE;
		if (m_bUseX)
			sRstAlgo->m_bOKX = targetX;
		if (m_bUseY)
			sRstAlgo->m_bOKY = targetY;
		if (m_bUseDist)
			sRstAlgo->m_bOKDist = targetX && targetY;

		checkOK = false;
	}

	bool AnchorX(false), AnchorY(false);
	for (int a = 0; a < m_nAnchorCnt; a++)
	{
		isOK = IsAppropriateResult(m_arrAnchorAlgoRst[a], AnchorX, AnchorY, a);
		if (!isOK)
		{
			sRstAlgo->m_bArrOKAnchor[a] = FALSE;
			if (m_bUseX)
				sRstAlgo->m_bOKX &= AnchorX;
			if (m_bUseY)
				sRstAlgo->m_bOKY &= AnchorY;
			if (m_bUseDist)
				sRstAlgo->m_bOKDist &= AnchorX && AnchorY;

			checkOK = false;
		}
	}

	int nTotalWndCnt = g_pInspMng->GetInspPartParamSize();

	int nTargetWndID = algo->m_nTargetWindowID;

	int nSize = _msize(arrAlignRes) / sizeof(arrAlignRes[0]);
	bool targetGet = m_bUseTargetPnt;
	bool anchorGet[DISTANCE_ANCHOR_LIMIT]{ false };
	int nAnchorGetCnt(0);
	for (int a = 0; a < nSize; a++)
	{
		if (!targetGet && GetAlignResultData(arrAlignRes, m_pTargetAlignRst, nTargetWndID, nSize, a))
		{
			targetGet = true;
		}
		else if(nAnchorGetCnt < m_nAnchorCnt)
		{
			for (int b = 0; b < m_nAnchorCnt; b++)
			{
				if (!anchorGet[b] && GetAlignResultData(arrAlignRes, m_arrAnchorAlignRst[b], algo->m_arrAnchorWindowID[b], nSize, a))
				{
					anchorGet[b] = true;
					nAnchorGetCnt++;
					break;
				}
			}
		}

		if (targetGet && nAnchorGetCnt == m_nAnchorCnt)
			break;
	}
	if (m_nAnchorMode == m_eDistanceAnchorMode_LineDistance)
	{
		if (m_arrAnchorAlgoRst[0]->m_nAlgoType == eAlgoLine)
		{
			m_arrAnchorAlignRst[0].centerX = m_arrAnchorAlignRst[0].m_dCenterX_T;
			m_arrAnchorAlignRst[0].centerY = m_arrAnchorAlignRst[0].m_dCenterY_T;
			m_arrAnchorAlignRst[1].centerX = m_arrAnchorAlignRst[0].m_dCenterX_R;
			m_arrAnchorAlignRst[1].centerY = m_arrAnchorAlignRst[0].m_dCenterY_R;
			m_nAnchorCnt = 2;
		}
		else if (m_pTargetAlgoRst->m_nAlgoType == eAlgoLine)
		{
			auto newTarget = m_arrAnchorAlignRst[0];
			m_arrAnchorAlignRst[0].centerX = m_pTargetAlignRst.m_dCenterX_T;
			m_arrAnchorAlignRst[0].centerY = m_pTargetAlignRst.m_dCenterY_T;
			m_arrAnchorAlignRst[1].centerX = m_pTargetAlignRst.m_dCenterX_R;
			m_arrAnchorAlignRst[1].centerY = m_pTargetAlignRst.m_dCenterY_R;
			m_nAnchorCnt = 2;
			m_pTargetAlignRst = newTarget;
		}
		else
			return false;
	}

	return checkOK;
}

bool CPInsp_AlgoDistance::GetAlignResultData(AlignResult *arrAlignRes, AlignResult &sumAlignRes, int curWndID, int nSize, int a)
{
	if (curWndID == arrAlignRes[a].nWindowID)
	{
		sumAlignRes.nAlignWndID = arrAlignRes[a].nAlignWndID;
		sumAlignRes.nWindowID = arrAlignRes[a].nWindowID;

		int alignWndID = sumAlignRes.nAlignWndID;
		bool hasAlign = alignWndID > 0;
		while (hasAlign)
		{
			hasAlign = false;
			for (int b = 0; b < nSize; b++)
			{
				if (alignWndID == arrAlignRes[b].nWindowID)
				{
					alignWndID = arrAlignRes[b].nAlignWndID;
					hasAlign = alignWndID > 0;
					sumAlignRes.theta -= arrAlignRes[b].theta;
					//if (sumAlignRes.theta == 0)
					//{
					//	sumAlignRes.theta = arrAlignRes[b].theta;

					//	double corr_x = 0, corr_y = 0;
					//	int nCorrectCoordinate = g_pInspMng->Getproc3d().CorrectCoordinate(arrAlignRes[a].centerX, arrAlignRes[a].centerY,
					//		arrAlignRes[b].centerX, arrAlignRes[b].centerY, arrAlignRes[b].theta, arrAlignRes[b].offsetX, arrAlignRes[b].offsetY, &corr_x, &corr_y);
					//	if (hasAlign)
					//	{
					//		arrAlignRes[a].centerX = corr_x;
					//		arrAlignRes[a].centerY = corr_y;
					//	}
					//	fStartX = (fPartWidth) / 2. + corr_x / dResolX;//m_resolX;
					//	fStartY = (fPartHeight) / 2. - corr_y / dResolY;//m_resolY;
					//	coordinateAlgo.dROICenterX = fStartX;
					//	coordinateAlgo.dROICenterY = fStartY;

					//	//apply PartClip Rounding Err  NYJ 2020.12
					//	float fDX, fDY;
					//	fDX = fStartX - (coordinateAlgo.dROIWidth / 2.);
					//	fDY = fStartY - (coordinateAlgo.dROILength / 2.);

					//	fDX -= m_pInspBoardInfo->fPartRoundingErrX;
					//	fDY -= m_pInspBoardInfo->fPartRoundingErrY;

					//	dX = (float)RounD(fDX);
					//	dY = (float)RounD(fDY);
					//	if (dX < 0) dX = 0;
					//	if (dY < 0) dY = 0;

					//	//save WindowClip Round err
					//	m_pInspBoardInfo->fWndRoundingErrX = (float)dX - (float)fDX;
					//	m_pInspBoardInfo->fWndRoundingErrY = (float)dY - (float)fDY;

					//	//apply WindowClip Round Err 
					//	coordinateAlgo.dROICenterX = coordinateAlgo.dROICenterX + m_pInspBoardInfo->fWndRoundingErrX;
					//	coordinateAlgo.dROICenterY = coordinateAlgo.dROICenterY + m_pInspBoardInfo->fWndRoundingErrY;
					//	////////////////////////////////////////////////////

					//	nOffX_pix = RounD(((fPartWidth) / 2. + corr_x / dResolX) - ((fPartWidth) / 2. + dCx / dResolX));	// Align결과에 의해 offset된 pixel수
					//	nOffY_pix = RounD(((fPartHeight) / 2. + dCy / dResolY) - ((fPartHeight) / 2. + corr_y / dResolY));

					//	sumAlignRes.centerX -= arrAlignRes[b].offsetX;
					//	sumAlignRes.centerY += arrAlignRes[b].offsetY;
					//}
					break;
				}
			}
		}

		// window center originated coordinate
		if (sumAlignRes.centerX == 0 && sumAlignRes.centerY == 0)
		{
			sumAlignRes.centerX = sumAlignRes.TeachCenterX + sumAlignRes.offsetX;
			sumAlignRes.centerY = sumAlignRes.TeachCenterY + sumAlignRes.offsetY;
		}
		//sumAlignRes.offsetX = arrAlignRes[a].offsetX;
		//sumAlignRes.offsetY = arrAlignRes[a].offsetY;
		//sumAlignRes.TeachCenterX = arrAlignRes[a].TeachCenterX;
		//sumAlignRes.TeachCenterY = arrAlignRes[a].TeachCenterY;
		//sumAlignRes.theta = 0;
		//sumAlignRes.rcBodyRect = arrAlignRes[a].rcBodyRect;

		return true;
	}
	return false;
}

bool CPInsp_AlgoDistance::IsAppropriateResult(InspAlgoResult *algoRst, bool &bIsOKX, bool &bIsOKY, int AnchorIdx)
{
	bool ret = false;

	AlignResult *alignRst;
	void *rst = algoRst->m_vRstInspAlgo, *algo;
	if (AnchorIdx < 0)
	{
		alignRst = &m_pTargetAlignRst;
		algo = m_pTargetInspAlgo->m_ptrInspAlgoParam;
	}
	else
	{
		alignRst = &m_arrAnchorAlignRst[AnchorIdx];
		algo = m_pArrAnchorInspAlgo[AnchorIdx]->m_ptrInspAlgoParam;
	}

	bIsOKX = bIsOKY = false;
	if (!algoRst->m_bIsInsp)
		return false;

	switch (algoRst->m_nAlgoType)
	{
	case eAlgoBlob:
		{
			if (((AlgoBlob*)algo)->m_bUseBlobSizeDistance)
				break;
			if (!algoRst->m_bOk)
				break;
			bIsOKX = ((RstAlgoBlob*)rst)->m_bOKShiftX;
			bIsOKY = ((RstAlgoBlob*)rst)->m_bOKShiftY;
			alignRst->offsetX = -((RstAlgoBlob*)rst)->m_dRstShiftX;	// resol(o)
			alignRst->offsetY = ((RstAlgoBlob*)rst)->m_dRstShiftY;	// resol(o)
			alignRst->TeachCenterX = ((AlgoBlob*)algo)->m_dTechCenterX;	// resol(o), partCoord(o)
			alignRst->TeachCenterY = ((AlgoBlob*)algo)->m_dTechCenterY;	// resol(o), partCoord(o)
			alignRst->centerX = ((RstAlgoBlob*)rst)->m_poDrawCenter.x - g_pInspMng->GetInspPartInfo()->partCx;
			alignRst->centerY = ((RstAlgoBlob*)rst)->m_poDrawCenter.y - g_pInspMng->GetInspPartInfo()->partCy;
			alignRst->theta = 0;
			ret = true;
			break;
		}
	case eAlgoBodyEdge:
		{
			if ((((AlgoBodyEdge*)algo)->narrdata[BodyEdge_N_Data] & BodyEdge_Data_UseDistance) == BodyEdge_Data_UseDistance)
				break;
			if (!algoRst->m_bOk)
				break;
			bIsOKX = ((RstAlgoBodyEdge*)rst)->m_bOKShiftX;
			bIsOKY = ((RstAlgoBodyEdge*)rst)->m_bOKShiftY;
			alignRst->offsetX = -((RstAlgoBodyEdge*)rst)->m_dRstOffset_x;	// resol(o)
			alignRst->offsetY = ((RstAlgoBodyEdge*)rst)->m_dRstOffset_y;	// resol(o)
			alignRst->TeachCenterX = ((AlgoBodyEdge*)algo)->farrdata[BodyEdge_F_TechCenterX];	// resol(o), partCoord(o)
			alignRst->TeachCenterY = ((AlgoBodyEdge*)algo)->farrdata[BodyEdge_F_TechCenterY];	// resol(o), partCoord(o)
			alignRst->centerX = ((RstAlgoBodyEdge*)rst)->m_dRstCenterX;
			alignRst->centerY = ((RstAlgoBodyEdge*)rst)->m_dRstCenterY;
			alignRst->theta = ((RstAlgoBodyEdge*)rst)->m_dRstRealTheta;
			ret = true;
			break;
		}
	case eAlgoEdge:
		{
			if (!algoRst->m_bOk)
				break;
			bIsOKX = ((RstAlgoEdge*)rst)->m_bOKShiftX;
			bIsOKY = ((RstAlgoEdge*)rst)->m_bOKShiftY;
			alignRst->offsetX = -((RstAlgoEdge*)rst)->m_dRstShiftX;	// resol(o)
			alignRst->offsetY = ((RstAlgoEdge*)rst)->m_dRstShiftY;	// resol(o)
			alignRst->centerX = ((RstAlgoEdge*)rst)->m_poDrawCenter.x * m_resolX - m_pParamArray[0].anyAngleWidth / 2.0;
			alignRst->centerY = ((RstAlgoEdge*)rst)->m_poDrawCenter.y * m_resolY - m_pParamArray[0].anyAngleLength / 2.0;
			alignRst->TeachCenterX = 0; // alignRst->centerX + ((AlgoEdge*)algo)->m_poArrSetTeachCenter[1].x * m_resolX;
			alignRst->TeachCenterY = 0; //alignRst->centerY - ((AlgoEdge*)algo)->m_poArrSetTeachCenter[0].y * m_resolY;
			alignRst->theta = 0;
			ret = true;
			break;
		}
	case eAlgoLine:
	{
		if (!algoRst->m_bOk)
			break;

		double partCx = g_pInspMng->GetInspPartInfo()->partWidth / 2.0;
		double partCy = g_pInspMng->GetInspPartInfo()->partHeight / 2.0;

		bIsOKX = bIsOKY = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoLine*)rst)->m_bOKAngle;
		bIsOKX = ((RstAlgoLine*)rst)->m_bOKShiftX;
		bIsOKY = ((RstAlgoLine*)rst)->m_bOKShiftY;

		alignRst->m_dCenterX_T = (((RstAlgoLine*)rst)->m_poDrawLine[0].x) * m_resolX - partCx;
		alignRst->m_dCenterY_T = -(((RstAlgoLine*)rst)->m_poDrawLine[0].y) * m_resolY + partCy;
		alignRst->m_dCenterX_R = (((RstAlgoLine*)rst)->m_poDrawLine[1].x) * m_resolX - partCx;
		alignRst->m_dCenterY_R = -(((RstAlgoLine*)rst)->m_poDrawLine[1].y) * m_resolY + partCy;
		alignRst->centerX = (alignRst->m_dCenterX_T + alignRst->m_dCenterX_R) / 2.0;
		alignRst->centerY = (alignRst->m_dCenterY_T + alignRst->m_dCenterY_R) / 2.0;
		ret = true;
		break;
	}
		/*
	case eAlgoBody_Blob:
		{
			if (!algoRst->m_bOk)
				break;
			bIsOKX = ((RstAlgoBodyBlob*)rst)->m_bOKShiftX;
			bIsOKY = ((RstAlgoBodyBlob*)rst)->m_bOKShiftY;
			alignRst->offsetX = -((RstAlgoBodyBlob*)rst)->m_dRstShiftX;
			alignRst->offsetY = ((RstAlgoBodyBlob*)rst)->m_dRstShiftY;
			alignRst->TeachCenterX = ((AlgoBodyBlob*)algo)->m_dTechCenterX;
			alignRst->TeachCenterY = ((AlgoBodyBlob*)algo)->m_dTechCenterY;
			alignRst->centerX = ((RstAlgoBodyBlob*)rst)->m_poDrawCenter.x * m_resolX;
			alignRst->centerY = ((RstAlgoBodyBlob*)rst)->m_poDrawCenter.y * m_resolY;
			ret = true;
			break;
		}
	case eAlgoAlign:
		{
			bIsOKX = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoAlign*)rst)->m_bOKShiftX;
			bIsOKY = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoAlign*)rst)->m_bOKShiftY;
			alignRst->offsetX += -((RstAlgoAlign*)rst)->m_dOffset_x;
			alignRst->offsetY += ((RstAlgoAlign*)rst)->m_dOffset_y;
			alignRst->TeachCenterX = ((AlgoAlign*)algo)->m_dShiftX;
			alignRst->TeachCenterY = ((AlgoAlign*)algo)->m_dShiftX;
			ret = true;
			break;
		}
	case eAlgoAlignEdge:
		{
			bIsOKX = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoAlignEdge*)rst)->m_bOKShiftX;
			bIsOKY = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoAlignEdge*)rst)->m_bOKShiftY;
			alignRst->offsetX += -((RstAlgoAlignEdge*)rst)->m_dRstShiftX;
			alignRst->offsetY += ((RstAlgoAlignEdge*)rst)->m_dRstShiftY;
			alignRst->TeachCenterX = ((AlgoAlignEdge*)algo)->sArrAlgoEdge[0].m_dTechCenterX;
			alignRst->TeachCenterY = ((AlgoAlignEdge*)algo)->sArrAlgoEdge[0].m_dTechCenterY;
			ret = true;
			break;
		}
	case eAlgoPadAlign:
		{
			bIsOKX = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoPadAlign*)rst)->m_bOKShiftX;
			bIsOKY = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoPadAlign*)rst)->m_bOKShiftY;
			alignRst->offsetX += -((RstAlgoPadAlign*)rst)->m_dRstShiftX;
			alignRst->offsetY += ((RstAlgoPadAlign*)rst)->m_dRstShiftY;
			alignRst->TeachCenterX = ((AlgoPadAlign*)algo)->m_sSearchPoint.x;
			alignRst->TeachCenterY = ((AlgoPadAlign*)algo)->m_sSearchPoint.y;
			ret = true;
			break;
		}
	case eAlgoPOCR:
		{
// 			bIsOKX = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoPOCR*)rst)->bRstShiftX;
// 			bIsOKY = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoPOCR*)rst)->bRstShiftY;
// 			alignRst->offsetX += -((RstAlgoPOCR*)rst)->dRstShiftX;
// 			alignRst->offsetY += ((RstAlgoPOCR*)rst)->dRstShiftY;
// 			alignRst->TeachCenterX = ((RstAlgoPOCR*)rst)->ModelX - ((RstAlgoPOCR*)rst)->ModelWidth / 2.0;
// 			alignRst->TeachCenterY = ((RstAlgoPOCR*)rst)->ModelY - ((RstAlgoPOCR*)rst)->ModelHeight / 2.0;
// 			ret = true;
			break;
		}
	case eAlgoPattern:
		{
			bIsOKX = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoPattern*)rst)->m_bOKOffsetX;
			bIsOKY = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoPattern*)rst)->m_bOKOffsetX;
			alignRst->offsetX += -((RstAlgoPattern*)rst)->offsetX;
			alignRst->offsetY += ((RstAlgoPattern*)rst)->offsetY;
			alignRst->TeachCenterX = ((RstAlgoPattern*)rst)->cogX;
			alignRst->TeachCenterY = ((RstAlgoPattern*)rst)->cogY;
			ret = true;
			break;
		}
	case eAlgoLine:
		{
			bIsOKX = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoLine*)rst)->m_bOKShiftX;
			bIsOKY = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoLine*)rst)->m_bOKShiftY;
			alignRst->offsetX += -((RstAlgoLine*)rst)->m_dRstShiftX;
			alignRst->offsetY += ((RstAlgoLine*)rst)->m_dRstShiftY;
			alignRst->TeachCenterX = ((AlgoLine*)algo)->m_dTechCenterX;
			alignRst->TeachCenterY = ((AlgoLine*)algo)->m_dTechCenterY;
			ret = true;
			break;
		}
	case eAlgoBump:
		{
			bIsOKX = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoBump*)rst)->m_bOKShiftX;
			bIsOKY = algoRst->m_bIsInsp && algoRst->m_bOk && ((RstAlgoBump*)rst)->m_bOKShiftY;
			alignRst->offsetX += -((RstAlgoBump*)rst)->fRstShiftX;
			alignRst->offsetY += ((RstAlgoBump*)rst)->fRstShiftY;
			alignRst->TeachCenterX = ((AlgoBump*)algo)->farrdata[Bump_F_TechCenterX];
			alignRst->TeachCenterY = ((AlgoBump*)algo)->farrdata[Bump_F_TechCenterY];
			ret = true;
			break;
		}
		*/
	default:
		{
			break;
		}
	}

	double tmp;
	switch (m_nPartAngle)
	{
	case 1:
		tmp = alignRst->offsetX;
		alignRst->offsetX = alignRst->offsetY;
		alignRst->offsetY = -tmp;
		break;
	case 2:
		alignRst->offsetX *= -1;
		alignRst->offsetY *= -1;
		break;
	case 3:
		tmp = alignRst->offsetX;
		alignRst->offsetX = -alignRst->offsetY;
		alignRst->offsetY = tmp;
		break;
	default:
		break;
	}

	return ret;
}

void CPInsp_AlgoDistance::GetAngleAppliedCoord(double srcX, double srcY, double &dstX, double &dstY, int nAngle)
{
	switch (nAngle)
	{
	case 1:
		dstY = srcX;
		dstX = -srcY;
		break;
	case 2:
		dstX = -srcX;
		dstY = -srcY;
		break;
	case 3:
		dstY = -srcX;
		dstX = srcY;
		break;
	case 0:
	default:
		dstX = srcX;
		dstY = srcY;
		break;
	}
}

int CPInsp_AlgoDistance::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}

bool CPInsp_AlgoDistance::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}
void CPInsp_AlgoDistance::CorrectCoordinate(int nCurWndIndex, AlignResult * sptrAlignRes, InspAlgo sInspAlgo, int nAlignTotalCnt, double &dOffsetX, double &dOffsetY)
{
	InspAlgoType eAlgoType = eAlgoDistance;
	int nWndinspType = eINSP_MOUNT;
	double dResolX = m_resolX;
	double dResolY = m_resolY;
	if (g_pMPTI->m_bSideOriginalSize == true)
	{
		dResolX = g_pMPTI->m_dBtmSideResX;
		dResolY = g_pMPTI->m_dBtmSideResY;
	}
	int nWidth, nHeight;
	double dX, dY;

	InspPartInfo *pInspBoardInfo = g_pInspMng->GetInspPartInfo();
	std::map<InspAlgoType, IPINSP_ALGO*> &dicInspAlgorithm = g_pInspMng->m_dicInspAlgorithm;

	float fPartWidth = pInspBoardInfo->partWidth / dResolX;//m_resolX;
	float fPartHeight = pInspBoardInfo->partHeight / dResolY;//m_resolY;
	float fWndWidth = (float)(m_pParamArray[nCurWndIndex].width / dResolX);//m_resolX);
	float fWndHeight = (float)(m_pParamArray[nCurWndIndex].length / dResolY);//m_resolY);
	double dCx = m_pParamArray[nCurWndIndex].cx;
	double dCy = m_pParamArray[nCurWndIndex].cy;

	double corr_x = 0, corr_y = 0;

	int nWndWidth = RounD(fWndWidth);
	int nWndHeight = RounD(fWndHeight);

	for (int nAlgo = 0; nAlgo < m_pParamArray[nCurWndIndex].nAlgorithmCnt; nAlgo++)
	{
		InspAlgo sInspAlgo = m_pParamArray[nCurWndIndex].vArrAlgoParam[nAlgo];
	}

	int nTotalCnt = nAlignTotalCnt;
	if (nTotalCnt <= 0) nTotalCnt = 1;
	for (int nAlignIdx = 0; nAlignIdx < nTotalCnt; nAlignIdx++)
	{
		double dAlignRes_centerX = 0;
		double dAlignRes_centerY = 0;
		double dAlignRes_theta = 0;
		double dAlignRes_offsetX = 0;
		double dAlignRes_offsetY = 0;
		int nAlignRes_Width = 0;
		int nAlignRes_Height = 0;
		LONG lAlignRes_left = 0;
		LONG lAlignRes_right = 0;
		LONG lAlignRes_top = 0;
		LONG lAlignRes_bottom = 0;
		if (nAlignTotalCnt > nAlignIdx && sptrAlignRes)
		{
			dAlignRes_centerX = sptrAlignRes[nAlignIdx].centerX;
			dAlignRes_centerY = sptrAlignRes[nAlignIdx].centerY;
			dAlignRes_theta = sptrAlignRes[nAlignIdx].theta;
			dAlignRes_offsetX = sptrAlignRes[nAlignIdx].offsetX;
			dAlignRes_offsetY = sptrAlignRes[nAlignIdx].offsetY;
			nAlignRes_Width = sptrAlignRes[nAlignIdx].rcBodyRect.Width();
			nAlignRes_Height = sptrAlignRes[nAlignIdx].rcBodyRect.Height();
			lAlignRes_left = sptrAlignRes[nAlignIdx].rcBodyRect.left;
			lAlignRes_right = sptrAlignRes[nAlignIdx].rcBodyRect.right;
			lAlignRes_top = sptrAlignRes[nAlignIdx].rcBodyRect.top;
			lAlignRes_bottom = sptrAlignRes[nAlignIdx].rcBodyRect.bottom;

			if (sptrAlignRes[nAlignIdx].m_bAlgoCenter == TRUE)
			{
				if (sptrAlignRes[nAlignIdx].m_dCenterX_R > 0 &&
					sptrAlignRes[nAlignIdx].m_dCenterY_R > 0 &&
					sptrAlignRes[nAlignIdx].m_dCenterX_R < pInspBoardInfo->partWidth &&
					sptrAlignRes[nAlignIdx].m_dCenterY_R < pInspBoardInfo->partHeight)
				{
					dAlignRes_centerX = sptrAlignRes[nAlignIdx].m_dCenterX_R - (pInspBoardInfo->partWidth / 2.0);
					dAlignRes_centerY = sptrAlignRes[nAlignIdx].m_dCenterY_R - (pInspBoardInfo->partHeight / 2.0);
				}
			}
		}

		if (nAlignTotalCnt > nAlignIdx && sptrAlignRes)
		{
			// 회전 중심점 변경 : Part Center -> Align Window Center
			m_proc3d.CorrectCoordinate(dCx, dCy, dAlignRes_centerX, dAlignRes_centerY, dAlignRes_theta, dAlignRes_offsetX, dAlignRes_offsetY, &corr_x, &corr_y);
			dCx = corr_x;
			dCy = corr_y;
		}
	}

	dOffsetX = dCx - m_pParamArray[nCurWndIndex].cx;
	dOffsetY = m_pParamArray[nCurWndIndex].cy - dCy;
}
