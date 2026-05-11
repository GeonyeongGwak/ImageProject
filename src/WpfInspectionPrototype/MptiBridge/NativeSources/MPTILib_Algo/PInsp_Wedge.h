#pragma once
#include "PInsp_Foot.h"


class CPInsp_Wedge:
	public CPInsp_Foot
{
public:
	CPInsp_Wedge();
	~CPInsp_Wedge();

#pragma region WedgeType

	bool FindCandidate_Wedge(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* dst, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg);
	bool FindCandidate_TailEndPosY(cv::Mat fHRoImg, bool bFindWedgeCandidates, cv::Mat WedgeCandidateImg, cv::Point& CenterPo, cv::Point& FootRoImgMargin, int &TailEndPosY, int &WedgeEndPosY);
	bool FindCandidate_WingStartAndWingEnd(cv::Mat* vImage, cv::Point& CenterPo, cv::Point& FootRoImgMargin, double dWingLengthMinimumMM, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, cv::Mat Img3D, bool bFindHill, bool bFindCandidateWedge, cv::Mat* cvWedgeCandidates);
	bool FindCandidate_Wing(CFoot_Model* pFoot, cv::Mat Img3D, cv::Mat BinWing, cv::Mat* dst, cv::Point& CenterPo);
	bool WingWedgeImage(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible = false);
	bool FindCandidate_TailAndWire_SobelXY(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat* cvFootImg, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, cv::Point& CenterPo, cv::Point& FootRoImgMargin, int &TailEndPosY, int &WireStartPosY);
	bool FindCandidate_Tail_v2(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat* cvFootImg, bool bFindWedgeCandidates, cv::Mat WedgeCandidateImg, cv::Point& CenterPo, cv::Point& FootRoImgMargin, int &TailEndPosY, int &WireStartPosY, int &WedgeEndPosY);
	bool ExtractLeftAndRightAreaOfFoot(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat roFootBWImg, cv::Mat WedgeCandidatesImg, cv::Mat WingCandidatesImg, cv::Mat *roFootBWRemovedFlowDown, cv::Mat* roFootLeftAreaBWImg, cv::Mat* roFootLeftAreafHImg, cv::Mat* roFootRightAreaBWImg, cv::Mat* roFootRightAreafHImg, CRect *rcArrClipedLeftFoot, CRect *rcArrClipedRightFoot, cv::Point& CenterPo, cv::Point& Margin);
	bool CalcFootHeightGradient_LeftAndRightAreas(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat inputFootLeftfH, cv::Mat inputFootRightfH, cv::Point& CenterPo, cv::Point& Margin, cv::Mat* LeftFootGradImg, cv::Mat* RightFootGradImg);
	bool DetectWireStartPos_UseLeftAndRightAreasOfFootHeight(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat FootBWRemovedFlowDown, cv::Mat inputFootLeftfH, cv::Mat inputFootRightfH, CRect rcLeftFootRefROI, CRect rcRightFootRefROI, cv::Point& CenterPo, cv::Point& Margin, bool bUseMeanSlopeAccum, int *nDetectedWireStartPosY);
	bool DetectWireStartPos_UseLeftAndRightAreasOfFootHeight_2(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat FootBWRemovedFlowDown, cv::Mat inputFootLeftfH, cv::Mat inputFootRightfH, CRect rcLeftFootRefROI, CRect rcRightFootRefROI, cv::Point& CenterPo, cv::Point& Margin, bool bUseMeanSlopeAccum, int *nDetectedWireStartPosY);
	bool DetectWireStartPos_UseFootHeightProfile(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat FootBWRemovedFlowDown, cv::Mat WingCandiImg, cv::Point& CenterPo, cv::Point& Margin, int *nDetectedWireStartPosY);

	bool getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection);
	bool Insp_FootPosition(cv::Mat* vImage, cv::Point* CenterPo, double* seta, cv::Mat FindImage, int nFootDirection, cv::Point2d & length);	//foot rotate
	bool RotateBackToOriginalPosition(CFoot_Model* pFoot, cv::Mat fHRoImg, cv::Mat cvFootImg, cv::Mat inputImg, cv::Point& CenterPo, cv::Point& Margin, double& seta, cv::Mat* ResultImg);

	//1유형검사
	bool FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible);

	//2. 형상검사
	void MaskFootDiffX(cv::Mat Img3D, cv::Mat BinWing, cv::Mat* dst, int nMin, int nMag); // wing 3D diffX
	bool Insp_FootPadLength(cv::Mat* vImage, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, cv::Mat Img3D, bool bFindHill); // 기존 Wing Start, end
	bool getFootLengthEndPoint(CFoot_Model* pFoot, cv::Mat* vImage, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, int CntX, float WedgeMinSpec = -1);
	bool getFootBondingStEndPoint(CFoot_Model* pFoot, cv::Mat* vImage, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, int CntX, float WedgeMinSpec);
	bool getFootBondingStEndPoint2(cv::Mat* vImage, cv::Mat* fHRectImg, cv::Point* pLSt, cv::Point* pLEd, cv::Point* pRSt, cv::Point* pREd, int CntX, int CntY, int isSt);

	//2, 4
	cv::Point FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible, cv::Mat *cv3DAvgFilter, cv::Mat *cvFindPadImg, cv::Mat *cvFindFootImg);
	int FootPointByType(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, FootPoins &fPoints, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, std::vector<cv::Mat> &Images, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible, cv::Mat *cv3DAvgFilter);

#pragma endregion

private:
};
