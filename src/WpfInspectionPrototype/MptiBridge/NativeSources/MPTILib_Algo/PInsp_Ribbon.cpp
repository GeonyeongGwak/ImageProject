#pragma once
#include "stdafx.h"
#include "Pinsp_Ribbon.h"
#include "MPTI.h"


CPInsp_Ribbon::CPInsp_Ribbon()
{
}
CPInsp_Ribbon::~CPInsp_Ribbon()
{
}

bool CPInsp_Ribbon::FindFootSubArea(CFoot_Model* pFoot, cv::Point& CenterPo, double& seta, cv::Mat* cvFootImg, cv::Mat* cvWedgeImg, cv::Mat* cvWing, cv::Mat* cv3DImg, cv::Mat* PadImg, UCHAR *ucArrDstImg, cv::Mat *WingImg_SecondCandi, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	//If the Foot cannot be found by pattern matching, the 2D and 3D image is used.
	bool ret = false;
	return ret;
}

cv::Point CPInsp_Ribbon::FootMeasure(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, std::vector<cv::Mat> &Images, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, cv::Point CenterPos, double matchAngle, cv::Point Margin, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{

	int nLine = __LINE__;
	cv::Point bottomPo;	//bottomPo: after rotate position
	try
	{
		cv::Mat* cvFootImg = &Images[0];
		cv::Mat* cvWedgeImg = &Images[1];
		cv::Mat* cv3DImg = &Images[2];
		cv::Mat* PadImg = &Images[3];

		//delete silk
		DeleteSilk(cvWedgeImg);

		std::vector<cv::Point> vPoSt((int)m_eFoot::m_eFoot_Total);
		std::vector<cv::Point> vPoEd((int)m_eFoot::m_eFoot_Total);
		cv::Point pSt, pEd;

		pSt.x = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Bonding1].x;
		pSt.y = sRstAlgo->rePoinSt[(int)m_eFoot::m_eFoot_Bonding1].y;
		pEd.x = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Bonding1].x;
		pEd.y = sRstAlgo->rePoinEd[(int)m_eFoot::m_eFoot_Bonding1].y;
#if _DEBUG
		cv::imwrite("D:\\cvFootImg.bmp", *cvFootImg);
		cv::imwrite("D:\\cvWedgeImg.bmp", *cvWedgeImg);
#endif
		cv::Point WingBondingEd;
		WingBondingEd.x = pEd.x;
		WingBondingEd.y = pEd.y;

		cv::Mat Img = cvFootImg->clone();

		bottomPo.x = Img.cols;
		bottomPo.y = Img.rows;
		nLine = __LINE__;
		//Width
		cv::Mat vLeft, vRight;
		vLeft = Img(cv::Rect(0, 0, CenterPos.x - Margin.x, Img.rows)).clone();
		vRight = Img(cv::Rect(CenterPos.x - Margin.x, 0, Img.cols - (CenterPos.x - Margin.x), Img.rows)).clone();

		cv::Point Re_LeftWidthSt(0, 0), Re_RightWidthSt(0, 0), Re_LeftWidthEd(0, 0), Re_RightWidthEd(0, 0);

		nLine = __LINE__;
		bool bWidthLeft = Insp_FootWidth(&vLeft, pSt, pEd, true, &Re_LeftWidthSt, &Re_LeftWidthEd);
		bool bWidthRight = Insp_FootWidth(&vRight, pSt, pEd, false, &Re_RightWidthSt, &Re_RightWidthEd);


		/*	Re_LeftWidthSt.y -= 2;
			Re_RightWidthSt.y -= 2;*/
		Re_LeftWidthSt.x = Re_LeftWidthSt.x;// -2;
		Re_RightWidthSt.x = Re_RightWidthSt.x + CenterPos.x - Margin.x;
		Re_LeftWidthEd.x = CenterPos.x - Margin.x;
		Re_LeftWidthEd.y = Re_LeftWidthSt.y;
		Re_RightWidthEd.x = CenterPos.x - Margin.x;
		Re_RightWidthEd.y = Re_RightWidthSt.y;
		vPoSt[(int)m_eFoot::m_eFoot_Width_1_H] = (Re_LeftWidthSt);
		vPoEd[(int)m_eFoot::m_eFoot_Width_1_H] = (Re_LeftWidthEd);

		vPoSt[(int)m_eFoot::m_eFoot_Width_1] = (Re_RightWidthSt);
		vPoEd[(int)m_eFoot::m_eFoot_Width_1] = (Re_RightWidthEd);
#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_L_Width.bmp"), &Img, vPoSt, vPoEd);
#endif
		nLine = __LINE__;

		//Foot Length inspection
		cv::Point LengRSt, LengREd;
		Img = (*cvFootImg);	//Foot Img를 타이트하게 자르기

		bool bLength = Insp_FootLength(&Img, pSt, pEd, &LengRSt, &LengREd);		//회전된 Foot Img로부터 Foot 길이 구하기

		vPoSt[(int)m_eFoot::m_eFoot_Length_1] = LengRSt;
		vPoEd[(int)m_eFoot::m_eFoot_Length_1] = LengREd;

		if (pEd.y >= Img.rows)
			pEd.y = Img.rows - 1;

		WingBondingEd.x = pEd.x;
		WingBondingEd.y = pEd.y;
		cv::Point rotateWingEdPnt = pEd;

#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_FootLength.bmp"), &Img, vPoSt, vPoEd);
#endif
		//tailLength
		Img = (*cvFootImg);
		cv::Point pTailSt, pTailEd;
		float fArea(0.0f);
		bool bTailLength = Insp_FootTailLength(Img, pSt, pSt, &pTailSt, &pTailEd, &fArea); // Tail End bonding 기준으로 변경

		vPoSt[(int)m_eFoot::m_eFoot_Tail_L] = (pTailSt);
		vPoEd[(int)m_eFoot::m_eFoot_Tail_L] = (pTailEd);
#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_L_W_Tail.bmp"), &Img, vPoSt, vPoEd);
#endif
		nLine = __LINE__;

		//Wedge
		cv::Mat Img3D = (*cv3DImg);
		Img = (*cvFootImg);
		cv::Mat fHImg = Img3D.clone();

		cv::Mat  padRectImg, padImage = (*PadImg).clone();
		cv::Mat LiftWRect;
		cv::Mat kernel;

		pFoot->GetPadBinImage(&padImage);


		kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(31, 31), cv::Point(1, 1));
		cv::erode(padImage, padImage, kernel);

		Img = (*cvFootImg);

		padRectImg = padImage.clone();
		nLine = __LINE__;
		cv::Mat InspImg, InspFindImg, Insp3D, InspPad;
		int nTop = vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y;	//pWedgeH_1.y;	
		nTop = vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y < vPoEd[(int)m_eFoot::m_eFoot_Wedge_L].y ? vPoSt[(int)m_eFoot::m_eFoot_Wedge_L].y : vPoEd[(int)m_eFoot::m_eFoot_Wedge_L].y;
		int nBot = pEd.y;

		
		InspImg = (*cvWedgeImg).clone();
		//WarpAffine(InspImg, RoInspImg, matchAngle, Img.size(), CenterPos);
		Insp3D = fHImg.clone();

		cv::Point pWedgeLeftSt, pWedgeLeftEd, pWedgeRightSt, pWedgeRightEd;
		bool bWedge = Insp_FootWedge(&InspImg, pSt, pEd, &pWedgeLeftSt, &pWedgeLeftEd, &pWedgeRightSt, &pWedgeRightEd);

		//Lift
		cv::Point pWedgeW_1, pWedgeW_2, pWedgeH_1, pWedgeH_2;
		pWedgeW_1.x = (pWedgeLeftSt.x + pWedgeLeftEd.x) / 2;
		pWedgeW_1.y = (pWedgeLeftSt.y + pWedgeLeftEd.y) / 2;
		pWedgeW_2.x = (pWedgeRightSt.x + pWedgeRightEd.x) / 2;
		pWedgeW_2.y = (pWedgeRightSt.y + pWedgeRightEd.y) / 2;
		pWedgeH_1.x = (pWedgeLeftSt.x + pWedgeRightSt.x) / 2;
		pWedgeH_1.y = (pWedgeLeftSt.y + pWedgeRightSt.y) / 2;
		pWedgeH_2.x = (pWedgeLeftEd.x + pWedgeRightEd.x) / 2;
		pWedgeH_2.y = (pWedgeLeftEd.y + pWedgeRightEd.y) / 2;

		vPoSt[(int)m_eFoot::m_eFoot_Wedge_W] = (pWedgeW_1);
		vPoEd[(int)m_eFoot::m_eFoot_Wedge_W] = (pWedgeW_2);
		vPoSt[(int)m_eFoot::m_eFoot_Wedge_L] = (pWedgeH_1);
		vPoEd[(int)m_eFoot::m_eFoot_Wedge_L] = (pWedgeH_2);

		nTop = 0;
		if (nBot - nTop < Img.rows)
		{
			InspFindImg = Img(cv::Rect(0, nTop, Img.cols, nBot - nTop)).clone();

			cv::Mat InspRoWImg(InspImg.rows, InspImg.cols, CV_8UC1);
			InspRoWImg.setTo(0);
			cv::rectangle(InspRoWImg, cv::Rect(0, pSt.y, InspRoWImg.cols, pEd.y), 255, cv::FILLED);
			InspRoWImg &= InspImg;
			InspImg = InspRoWImg(cv::Rect(0, nTop, Img.cols, nBot - nTop)).clone();
			Insp3D = fHImg(cv::Rect(0, nTop, fHImg.cols, nBot - nTop)).clone();
		}
		sRstAlgo->m_rcRefArea.left = 0;
		sRstAlgo->m_rcRefArea.right = 0;
		sRstAlgo->m_rcRefArea.top = 0;
		sRstAlgo->m_rcRefArea.bottom = 0;
		float fHightDiff(0.0f);

		InspPad = (padRectImg).clone();
		if (nBot - nTop < padRectImg.rows)
		{
			InspPad = (padRectImg)(cv::Rect(0, nTop, padRectImg.cols, nBot - nTop)).clone();
			InspPad -= InspFindImg;
		}

		Insp_FootLift(&InspImg, &Insp3D, &InspPad, &fHightDiff);

		//
		//UI에 표시하기 위한 이미지 (ucArrDstImg)생성
		if (ucArrDstImg != NULL)
		{
			std::vector<cv::Point> vPoDispSt;
			std::vector<cv::Point> vPoDispEd;
			CalcDisplayPnt(matchAngle, vPoSt, vPoEd, vPoDispSt, vPoDispEd);
			CalcRotPnt(CenterPos, -matchAngle, vPoDispSt, vPoDispEd, Margin);

			for (int i = 0; i < m_eFoot::m_eFoot_Total; i++)
			{
				sRstAlgo->DispPoinSt[i].x = vPoDispSt[i].x;
				sRstAlgo->DispPoinSt[i].y = vPoDispSt[i].y;

				sRstAlgo->DispPoinEd[i].x = vPoDispEd[i].x;
				sRstAlgo->DispPoinEd[i].y = vPoDispEd[i].y;
			}
#if _DEBUG
			ImagePointSave(_T("D:\\FootRst\\Bin2D_End_Disp.bmp"), &Img, vPoDispSt, vPoDispEd);
#endif
		}
#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_L_W_T_I_H_Lift.bmp"), &Img, vPoSt, vPoEd);
#endif

		CalcRotPnt(CenterPos, -matchAngle, vPoSt, vPoEd, Margin);
#if _DEBUG
		ImagePointSave(_T("D:\\FootRst\\Bin2D_End.bmp"), &Img, vPoSt, vPoEd);
#endif
		nLine = __LINE__;

		//shift
		//Insp_FootShift(pFoot, sRstAlgo, startPad, endPad, startPadY, endPadY, nMatLeft, nMatTop, nMatRight, nMatBottom, LeftIdx);
		//cv::Point cvPoNewCenter;
		//cvPoNewCenter.x = CenterPos.x - Margin.x;
		//cvPoNewCenter.y = CenterPos.y - Margin.y;
		//float fShift, fShiftX, fShiftY;
		//bool shiftRst = Insp_FootShift_ImgAnd(pFoot, sRstAlgo, &Img,PadImg, &padRectImg,cvPoNewCenter, fShift, fShiftX, fShiftY);

		//Shift 검사 NYJ 2020/02/06
		bool shiftRst = Insp_FootShift_removedWire(pFoot, sRstAlgo, &Img, &WingBondingEd);
		//result ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//for teaching
		POINTF CenterPoint;
		CenterPoint.x = CenterPos.x;
		CenterPoint.y = CenterPos.y;
		sRstAlgo->m_sPoint = CenterPoint;

		//PadLength
		//result /Left
		int footInspTypeIdx = (int)m_eFoot::m_eFoot_Bonding1;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		//result /Right
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Bonding2;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);

		//Length
		//result /Left
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Length_1;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);


		//wedge
		//result / length
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Wedge_L;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		//wedge result / width
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Wedge_W;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);

		//width
		//result / Left / Harf
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Width_1_H;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		float widthLeft = sRstAlgo->m_fArrRst[footInspTypeIdx];

		//result / Right / Harf
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Width_1;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		float widthright = sRstAlgo->m_fArrRst[footInspTypeIdx];
		float widthValue = widthLeft + widthright;
		//result /width
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Width_1;
		vPoEd[footInspTypeIdx] = vPoSt[(int)m_eFoot::m_eFoot_Width_1_H];
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, footInspTypeIdx, widthValue);
		//result / Right
		//footInspTypeIdx = (int)m_eFoot::m_eFoot_Width_2;
		//if (vPoSt[footInspTypeIdx].x != -1 || vPoSt[footInspTypeIdx].y != -1)
		//	vPoSt[footInspTypeIdx].x += (*left);
		//if (vPoSt[footInspTypeIdx].x != -1 || vPoSt[footInspTypeIdx].y != -1)
		//	vPoEd[footInspTypeIdx].x += (*left);
		//Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, footInspTypeIdx, widthValue);

		//tailLength result / Length
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Tail_L;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);

		//tailLength result / Area
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Tail_A;
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, footInspTypeIdx, fArea);

		//Foot Damage(Imprint) result
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Damage;
		Confim_OneFootAlgo(pFoot, sRstAlgo, vPoSt[footInspTypeIdx], vPoEd[footInspTypeIdx], footInspTypeIdx);


		//Lift
		footInspTypeIdx = (int)m_eFoot::m_eFoot_Height;
		Confim_OneFootAlgo_byValue(pFoot, sRstAlgo, footInspTypeIdx, fHightDiff);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	catch (...)
	{
		CString errorLog;
		errorLog.Format(_T("CPInsp_Foot::Insp_OneFoot() Line: %d Pass"), nLine);
		g_pMPTI->AddLog(errorLog);

		sRstAlgo->m_bOK = FALSE;
		sRstAlgo->m_bFind = FALSE;
	}

	return CenterPos;
}

bool CPInsp_Ribbon::Insp_Ribbon(CFoot_Model *pFoot, cv::Mat FootImg, cv::Mat WedgeImg, cv::Mat HImg, cv::Point& st, cv::Point& ed, double& seta,bool bMatchSuccess)
{
	bool ret = false;
	int nLine = __LINE__;

	try
	{
		std::vector< cv::RotatedRect> vDstRect;
		GerAlignRect(WedgeImg, vDstRect);
		if (vDstRect.size() > 0)
		{
			cv::RotatedRect vRect = vDstRect[0];
			int nStdSz = vRect.size.width*vRect.size.height;
			for (int i=1;i< vDstRect.size();i++)
			{
				int nSz = vDstRect[i].size.width*vDstRect[i].size.height;
				if (nSz > nStdSz)
				{
					vRect = vDstRect[i];
					nStdSz = nSz;
				}
			}
			double dRectAngle = vRect.angle < 0 ? 90 + vRect.angle : vRect.angle;

			if (std::abs(dRectAngle) > std::abs(90 - dRectAngle))
				seta = 90 - dRectAngle;
			else
				seta = vRect.angle < 0 ? -dRectAngle : dRectAngle;


			st.x =vRect.center.x - (vRect.size.width / 2);
			st.y =vRect.center.y - (vRect.size.height / 2);
			ed.x =vRect.center.x + (vRect.size.width / 2);
			ed.y =vRect.center.y + (vRect.size.height / 2);

			return true;
		}

		int stY(0), edY, stX(0), edX;
		cv::Mat localImg = FootImg.clone();
		localImg &= 1;
		//seta = 0;
		cv::Mat localWedgeImg = WedgeImg.clone();
		cv::Mat integ, Devi;

		localWedgeImg &= 1;
		cv::integral(localWedgeImg, integ, Devi);
		int bot = pFoot->m_BondingRect.bottom;
		for (int r = integ.rows - 1; r > 0; r--)
		{
			if (integ.ptr<int>(r)[integ.cols-1] - integ.ptr<int>(r - 1)[integ.cols - 1] > 0)
			{
				bot = r;
				break;
			}
		}
		if (!bMatchSuccess)
		{
			int top = 0;
			pFoot->m_BondingRect.bottom = bot;
			for (int r = 0; r < bot; r++)
			{
				if (integ.ptr<int>(r)[integ.cols - 1] > 0)
				{
					top = r;
					break;
				}
			}
			pFoot->m_BondingRect.top = top;
		}
		cv::Mat localImgH = HImg.clone();
		cv::Mat gyorg,gy, gyinv, gybin;
		int sobelsz = 3;
		cv::Sobel(localImgH, gyorg, CV_16S, 0, 1, sobelsz);
		gyinv = -gyorg;
		gyinv.convertTo(gyinv, CV_8UC1);
		cv::rectangle(gyinv, cv::Point(0, pFoot->m_BondingRect.top + 20), cv::Point(gyinv.cols, gyinv.rows-1), cv::Scalar(0), cv::FILLED);
		cv::rectangle(gyinv, cv::Point(0, 0), cv::Point(gyinv.cols, pFoot->m_BondingRect.top - 20), cv::Scalar(0), cv::FILLED);
		cv::threshold(gyinv, gyinv, 0, 255, cv::THRESH_OTSU);
		cv::Point topCenterPo(0, 0), botCenterPo(0, 0);
		double topSeta(0.0f), botSeta(0.0f);
		bool bTopOK = Insp_FootPos(&gyinv, &topCenterPo, &topSeta);
		if (!bTopOK)
		{
			topCenterPo.y = pFoot->m_BondingRect.top;
			topCenterPo.x = (pFoot->m_BondingRect.right + pFoot->m_BondingRect.left) / 2;
		}
		
		gyorg.convertTo(gy, CV_8UC1);
		cv::rectangle(gy, cv::Point(0, pFoot->m_BondingRect.bottom + 10), cv::Point(gy.cols-1, gy.rows-1), cv::Scalar(0), cv::FILLED);
		cv::rectangle(gy, cv::Point(0, 0), cv::Point(gy.cols, pFoot->m_BondingRect.bottom - 20), cv::Scalar(0), cv::FILLED);
		cv::threshold(gy, gybin, 0, 255, cv::THRESH_OTSU);
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
		cv::dilate(WedgeImg, localWedgeImg, kernel);
		cv::bitwise_and(gybin, localWedgeImg, gybin);
		if (!Insp_FootPos(&gybin, &botCenterPo, &botSeta))
		{
			botCenterPo.y = pFoot->m_BondingRect.bottom;
			botCenterPo.x = (pFoot->m_BondingRect.right + pFoot->m_BondingRect.left) / 2;
		}

		//shkim 2021.05.18 No Tail exception
		if (bTopOK)
		{
			gyorg.convertTo(gy, CV_8UC1);
			cv::rectangle(gy, cv::Point(0, pFoot->m_BondingRect.top + 10), cv::Point(gy.cols - 1, gy.rows - 1), cv::Scalar(0), cv::FILLED);
			cv::rectangle(gy, cv::Point(0, 0), cv::Point(gybin.cols, pFoot->m_BondingRect.top - 20), cv::Scalar(0), cv::FILLED);
			cv::threshold(gy, gybin, 0, 255, cv::THRESH_OTSU);
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
			cv::dilate(WedgeImg, localWedgeImg, kernel);
			cv::bitwise_and(gybin, localWedgeImg, gybin);
			cv::Point topCenterPo_tmp(0, 0);
			double topSeta_tmp(0.0f);
			if (Insp_FootPos(&gybin, &topCenterPo_tmp, &topSeta_tmp))
			{
				if (std::abs(topCenterPo_tmp.y - pFoot->m_BondingRect.top) < std::abs(topCenterPo.y - pFoot->m_BondingRect.top))
				{
					topCenterPo = topCenterPo_tmp;
					//topSeta = topSeta_tmp;
				}
			}

		}

		//seta = (topSeta + botSeta) / 2;
#if _DEBUG
		cv::imwrite("D:\\FootRst\\Insp_Ribbon_INPUT_Img_H.bmp", localImgH);
		cv::imwrite("D:\\FootRst\\Insp_Ribbon_INPUT_gy.bmp", gy);
		cv::imwrite("D:\\FootRst\\Insp_Ribbon_INPUT_gyinv.bmp", gyinv);
		cv::imwrite("D:\\FootRst\\Insp_Ribbon_INPUT_gybin.bmp", gybin);
		cv::Point CenterPo;
		CenterPo.x = (topCenterPo.x + botCenterPo.x) / 2;
		CenterPo.y = (topCenterPo.y + botCenterPo.y) / 2;
		cv::Point Margin = WarpAffine(localWedgeImg, localWedgeImg, seta, localWedgeImg.size(), CenterPo);		//Wing Bin Img 회전
		cv::imwrite("D:\\FootRst\\Insp_Ribbon_localWedgeImg.bmp", localWedgeImg);
		cv::rectangle(localWedgeImg,cv::Rect(0,0, localWedgeImg.cols-1, topCenterPo.y-1),cv::Scalar(0));
		cv::imwrite("D:\\FootRst\\Insp_Ribbon_localWedgeImg_Cut.bmp", localWedgeImg);
#endif

		//st.x = topCenterPo.x;
		//st.y = topCenterPo.y;
		ed.x = botCenterPo.x;
		ed.y = botCenterPo.y;

		return true;

		////old ver

		//init
		double dRandScore(0);
		bool bIsDetected_BondingPo = true;

		edY = pFoot->m_BondingRect.bottom;
		edX = stX = (pFoot->m_BondingRect.right + pFoot->m_BondingRect.left) / 2;
		stY = pFoot->m_BondingRect.top;


#pragma region Inspection Point

		std::vector<UCHAR*> stptr(localImg.rows);
		int nCntX(0), nSumX(0);

		for (int r = 0; r < localImg.rows; r++)
		{
			stptr[r] = localImg.ptr(r);
		}

		for (int c = 0; c < localImg.cols; c++)
		{
			if (stptr[stY][c] > 0)
			{
				nSumX += c;
				nCntX++;
			}
		}
		if (nCntX > 0)
			stX = nSumX / nCntX;

		std::vector<float> fAvgH(localImg.rows);
		int calcSz = 10;

		for (int r = stY + 5; r >= 0; r--)
		{
			float avg(0.0f);
			float fSum(0.0f);
			int nCnt(0);
			for (int c = 0; c < localImg.cols; c++)
			{
				if (stptr[r][c] > 0)
				{
					float* fStptr = HImg.ptr<float>(r);
					fSum += fStptr[c];
					nCnt++;
				}

			}
			avg = fSum / nCnt;
			if (nCnt == 0)
				avg = 0;
			fAvgH[r] = avg;
		}

		float fMaxVal = 0;

		for (int r = stY + 5; r >= calcSz; r--)
		{
			float fTmpMaxVal = 0;
			float fStptr = fAvgH[r];
			float fMaxH = fStptr;
			for (int i = 0; i < calcSz; i++)
			{
				//float* fStptr = HImg->ptr<float>(r + i);
				float fEdptr = fAvgH[r - i];

				if (fEdptr > fMaxH)
				{
					fMaxH = fEdptr;
				}
			}
			fTmpMaxVal = fMaxH - fStptr;

			if (fMaxVal < fTmpMaxVal)
			{
				fMaxVal = fTmpMaxVal;
				stY = r;
			}
		}

#pragma endregion

		st.x = stX;
		st.y = stY;
		ed.x = edX;
		ed.y = edY;

		ret = true;
		
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("CPInsp_Foot::InspFoot error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
	}
	return ret;
}
bool CPInsp_Ribbon::getFootCenterPosAndSeta(cv::Point& CenterPo, double& seta, cv::Mat &image, cv::Mat &mMaskImage, cv::Point2d &WingLen, int nFootDirection)
{
	bool ret = false;
	return ret;
}

int CPInsp_Ribbon::FootPointByType(CFoot_Model *pFoot, RstAlgoFoot * sRstAlgo, FootPoins &fPoints, cv::Mat * cvFootImg, cv::Mat * cvWedgeImg, cv::Mat * cvWing, cv::Mat * cvPadImg, UCHAR *ucArrDstImg, cv::Mat *wingCandiImgSec, std::vector<cv::Mat> &Images, int nStartX, int nStartY, int nUIFootBinIDX, bool bWingRstVisible)
{
	int ret = 0;
	int nLine = __LINE__;
	cv::Point CenterPo, bottomPo;	//bottomPo: after rotate position
	try
	{
		//initialize 
		int nWidth = pFoot->GetImageWidth();
		int nHeight = pFoot->GetImageLength();
		cv::Mat cv3DArea(nHeight, nWidth, CV_32FC1, pFoot->pf3D);

		int nAreaGapX = -5.;
		int nAreaGapY = -5.;
		int nSX = fPoints.rcArea.left + nAreaGapX;
		int nEX = fPoints.rcArea.right - nAreaGapX;
		int nSY = fPoints.rcArea.top + nAreaGapY;
		int nEY = fPoints.rcArea.bottom - nAreaGapY;

		if ((pFoot->m_nUseOption2 & m_eFOOT_Data2_Dir_Up) == m_eFOOT_Data2_Dir_Up)
			nSY = 0;
		else
			nEY = nHeight;
		if (nSX < 0) nSX = 0;
		if (nSY < 0) nSY = 0;
		if (nEX < 0) nEX = nWidth;
		if (nEY < 0) nEY = nHeight;
		if (nSX > nWidth) nSX = nWidth;
		if (nSY > nHeight) nSY = nHeight;
		if (nEX > nWidth) nEX = nWidth;
		if (nEY > nHeight) nEY = nHeight;

		SetBWForUIImage(pFoot, nSX, nSY, nEX, nEY, cvFootImg, cvWedgeImg, cvWing);		//NYJ 2020/07/01

		cv::Rect ImageRect(pFoot->mImageRect.left, pFoot->mImageRect.top, nWidth, nHeight);

		double seta(0.0), seta1(0.0);
		cv::Mat Img;
		cv::Mat fHImg;
		cv::Mat mMask;
		CString ImagePath;

		//티칭된 Foot Bin Img 예외처리 (이진 이미지가 All Black 일 경우 함수 리턴시킴)
		cv::Mat FootImg, retMatIntg, retDev;
		FootImg = (*cvFootImg).clone();
		cv::integral(FootImg, retMatIntg, retDev);
		UINT* unREPtr = retMatIntg.ptr<UINT>(retMatIntg.rows - 1);
		if (unREPtr[retMatIntg.cols - 1] <= 0)
			return -1;

		//Foot CenterPos, Seta, Lenght End 구하는 함수
		bottomPo.x = Img.cols;
		bottomPo.y = Img.rows;
		mMask = (*cvFootImg).clone();
		cv::Point2d WingLen;
		bool flag = fPoints.m_dSeta == -INFINITY ? false : true;
		if (fPoints.bMatchSuccess)
		{
			CenterPo.x = fPoints.m_pCenter.x;
			CenterPo.y = fPoints.m_pCenter.y;
			seta = fPoints.m_dSeta;
		}
		else
		{
			seta = calcSeta2(pFoot, FootImg, *cvWedgeImg, cv3DArea, &CenterPo);
			seta = calcSeta(pFoot, FootImg, *cvWedgeImg, &CenterPo);
		}

		sRstAlgo->m_bFind = TRUE;
		cv::Point CentLineSt(0, 0), CentLineEd(0.0);
		float a = std::tan(seta*PI / 180);
		float b = -a * CenterPo.x + CenterPo.y;

		CentLineSt.x = 0;
		CentLineSt.y = b;
		CentLineEd.x = ImageRect.width;
		CentLineEd.y = a * ImageRect.width + b;

		nLine = __LINE__;

//		fHImg = cv3DArea.clone();

		nLine = __LINE__;
		seta1 = (seta);
		sRstAlgo->m_fAngle = seta1;

		nLine = __LINE__;

		if (flag == false)	//missing
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bFind = FALSE;
			return -1;
		}
		//bonding Area
		Img = (*cvFootImg).clone();
		cv::Mat ImgWedge;
		cv::Mat roImg;
		cv::Point Margin = WarpAffine(Img, roImg, seta, Img.size(), CenterPo);		//Wing Bin Img 회전
		WarpAffine(*cvWedgeImg, ImgWedge, seta, Img.size(), CenterPo);
		WarpAffine(cv3DArea, fHImg, seta, Img.size(), CenterPo);
		cv::Mat CutImg = roImg;

		cv::Point pSt, pEd;

		int leftOrg = pFoot->m_BondingRect.left;
		int rightOrg = pFoot->m_BondingRect.right;
		int topOrg = pFoot->m_BondingRect.top;
		int botOrg = pFoot->m_BondingRect.bottom;

		pFoot->m_BondingRect.left -= Margin.x;
		pFoot->m_BondingRect.right -= Margin.x;
		pFoot->m_BondingRect.top -= Margin.y;
		pFoot->m_BondingRect.bottom -= Margin.y;

		if (pFoot->m_BondingRect.right > CutImg.cols)
			pFoot->m_BondingRect.right = CutImg.cols - 1;
		if (pFoot->m_BondingRect.bottom > CutImg.rows)
			pFoot->m_BondingRect.bottom = CutImg.rows - 1;

		pEd.y = pFoot->m_BondingRect.bottom;
		pEd.x = pSt.x = (pFoot->m_BondingRect.right + pFoot->m_BondingRect.left) / 2;
		pSt.y = pFoot->m_BondingRect.top;
		bool bBonding = Insp_Ribbon(pFoot, CutImg, ImgWedge, fHImg, pSt, pEd, seta, fPoints.bMatchSuccess);
		seta1 -= (seta);
		sRstAlgo->m_fAngle = seta1;

		Images.emplace_back(*cvFootImg);
		Images.emplace_back(*cvWedgeImg);
		Images.emplace_back(cv3DArea);
		Images.emplace_back(*cvPadImg);

		int footInspTypeIdx = (int)m_eFoot::m_eFoot_Bonding1;
		sRstAlgo->rePoinSt[footInspTypeIdx].x = pSt.x;
		sRstAlgo->rePoinSt[footInspTypeIdx].y = pSt.y;
		sRstAlgo->rePoinEd[footInspTypeIdx].x = pEd.x;
		sRstAlgo->rePoinEd[footInspTypeIdx].y = pEd.y;
#if _DEBUG
		cv::imwrite("D:\\cvFootRotImg.bmp", CutImg);
		WarpAffine(*cvWedgeImg, ImgWedge, seta1, Img.size(), CenterPo);
		cv::imwrite("D:\\cvImgWedge.bmp", ImgWedge);
#endif
		if (!fPoints.bMatchSuccess)
		{
			fPoints.m_pCenter.x = CenterPo.x;
			fPoints.m_pCenter.y = CenterPo.y;
			fPoints.m_dSeta = sRstAlgo->m_fAngle;
		}
		fPoints.m_dSeta = sRstAlgo->m_fAngle;


#pragma region Write UI_Image
		nLine = __LINE__;
		//
		//UI에 표시하기 위한 이미지 (ucArrDstImg)생성
		if (ucArrDstImg != NULL)
		{
			nLine = __LINE__;
			int nWidth = pFoot->GetImageWidth();
			int nHeight = pFoot->GetImageLength();
			nLine = __LINE__;

			cv::Mat mMask = *cvFootImg;
			m_eFootBin footbin = (m_eFootBin)nUIFootBinIDX;

			//shkim edge UI work
			cv::Mat cvPadUI = cvPadImg->clone();
			cv::Mat cvFootUI = cvFootImg->clone();
			cvPadUI = cvPadUI - cvFootUI;

			cv::Mat cvWedgeUI;
			cvWedgeUI = cvWedgeImg->clone();

			if (pEd.y - pSt.y < roImg.rows)
			{
				cv::Mat RoWedge;
				cv::Mat InspRoWImg(cvWedgeUI.rows, cvWedgeUI.cols, CV_8UC1);
				InspRoWImg.setTo(0);
				cv::rectangle(InspRoWImg, cv::Rect(0, pSt.y, InspRoWImg.cols, pEd.y), 255, cv::FILLED);

				cv::Point CenterPos_Revert = NULL;

				CenterPos_Revert.x = CenterPo.x - Margin.x;
				CenterPos_Revert.y = CenterPo.y - Margin.y;
				bool noCnt = false;
				bool bRev = true;
				WarpAffine(InspRoWImg, RoWedge, -seta1, InspRoWImg.size(), CenterPos_Revert);//최종 이미지 원상복귀를 위한 회전
				int mX = -Margin.x > 0 ? -Margin.x: Margin.x;
				int mY = -Margin.y > 0 ? -Margin.y : Margin.y;
				cv::Mat clIpImg = RoWedge(cv::Rect(mX, mY, RoWedge .cols- mX, RoWedge.rows - mY)).clone();
				RoWedge.setTo(0);
				cv::Mat tmp = RoWedge(cv::Rect(0, 0, RoWedge.cols - mX, RoWedge.rows - mY));
				tmp |= clIpImg;

				cvWedgeUI &= RoWedge;
			}

			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));

			cv::Mat cvPadEdge, cvFootEdge, cvWedgeEdge;
			cv::Canny(cvPadUI, cvPadEdge, 0, 0);
			cv::Canny(cvFootUI, cvFootEdge, 0, 0);
			//cv::erode(cvWedgeUI, cvWedgeEdge, kernel);

			cv::dilate(cvPadEdge, cvPadEdge, kernel);
			cv::dilate(cvFootEdge, cvFootEdge, kernel);
			//cv::bitwise_xor(cvWedgeUI, cvWedgeEdge, cvWedgeEdge);

#if _DEBUG
			cv::imwrite("D:\\cvFootEdge.bmp", cvFootEdge);
			//cv::imwrite("D:\\cvWedgeEdge.bmp", cvWedgeEdge);
#endif

			UCHAR* ptrPad = cvPadEdge.data;
			UCHAR* ptrFoot = cvFootEdge.data;
			UCHAR* ptrWedge = cvWedgeUI.data;

			for (int r = 0; r < cvFootImg->rows; r++)
			{
				for (int c = 0; c < cvFootImg->cols; c++)
				{
					int nIndex = r * nWidth + c;
					int nIndex2 = (nStartY + r) * pFoot->_WidthOrg + (nStartX + c);
					nIndex2 *= 3;

//					if (bWingRstVisible)
					{
						//Wedge
						if (ptrWedge[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcWedge, 3);

						//Foot
						else if (ptrFoot[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcFoot, 3);

						//Pad
						else if (ptrPad[nIndex])
							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcBackground, 3);
					}
// 					else
// 					{
// 						//Foot
// 						if (ptrFoot[nIndex])
// 							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcFoot, 3);
// 
// 						//Pad
// 						else if (ptrPad[nIndex])
// 							memcpy(&ucArrDstImg[nIndex2], &pFoot->m_fcBackground, 3);
// 					}

				}
			}
		}
#if _DEBUG
		cv::Mat ImgwindPanel(pFoot->_HeightOrg, pFoot->_WidthOrg, CV_8UC3, ucArrDstImg);
#endif

#pragma endregion

	}
	catch (const std::exception&)
	{
		CString msg;
		msg.Format(_T("FootPointByType error Line: %d"), nLine);
		g_pMPTI->AddLog(msg);
	}
	return ret;
}


void CPInsp_Ribbon::SetBWForUIImage(CFoot_Model *pFoot, int nSX, int nSY, int nEX, int nEY, cv::Mat* cvFootBW, cv::Mat* cvWedgeBW, cv::Mat* cvWingBW)	//UI Image
{
	int nImgHeight = pFoot->GetImageLength();
	int nImgWidth = pFoot->GetImageWidth();
	int imgSize = nImgWidth * nImgHeight;

	try
	{
		pFoot->GetImageMatrix(cvFootBW, (int)m_eFootBin::m_eFootBin_Foot);
		pFoot->GetImageMatrix(cvWedgeBW, (int)m_eFootBin::m_eFootBin_Wedge);

		UCHAR *pucImg = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg, imgSize);
		memset(pucImg, 0, sizeof(UCHAR) * imgSize);

		cv::Mat srcImg1 = *cvWedgeBW;

		UCHAR* pucImg1 = NULL;
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&pucImg1, imgSize);
		memset(pucImg1, 0, sizeof(UCHAR) * imgSize);

		//set BW Option
		bool bFind2D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
		int nFindRange = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nFindRange = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nFindRange = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nFindRange = 3;
		int nFindMin2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[m_eBin::m_eBin_Min2D];
		int nFindMax2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[m_eBin::m_eBin_Max2D];
		bool bFind3D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
		int nFindRange3D = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nFindRange3D = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nFindRange3D = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nFindRange3D = 3;
		float fFindMin3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Min];
		float fFindMax3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Foot].m_fArrH[m_eMMD::eMMD_Max];

		//set BW Option - Wedge Type Foot
		bool bWedge2D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
		int nWedgeRange = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nWedgeRange = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nWedgeRange = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nWedgeRange = 3;
		int nWedgeMin2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[m_eBin::m_eBin_Min2D];
		int nWedgeMax2D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[m_eBin::m_eBin_Max2D];
		bool bWedge3D = ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
		int nWedgeRange3D = 0;
		if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
			nWedgeRange3D = 1;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
			nWedgeRange3D = 2;
		else if ((pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			nWedgeRange3D = 3;
		float fWedgeMin3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_fArrH[m_eMMD::eMMD_Min];
		float fWedgeMax3D = pFoot->m_sArrBin[(int)m_eFootBin::m_eFootBin_Wedge].m_fArrH[m_eMMD::eMMD_Max];

		///////////////////////////////

		for (int y = nSY; y < nEY; y++)
		{
			for (int x = nSX; x < nEX; x++)
			{
				int nIdx = y * nImgWidth + x;
				float fH = pFoot->pf3D[y * nImgWidth + x];
				bool b2DIn = false;
				//foot binarization
				if (bFind2D)
				{
					UCHAR ucG = cvFootBW->data[y * nImgWidth + x];
					if ((nFindRange == 0 && (nFindMin2D <= ucG && ucG <= nFindMax2D)) ||
						(nFindRange == 1 && (nFindMin2D > ucG || ucG < nFindMax2D)) ||
						(nFindRange == 2 && ucG >= nFindMax2D) ||
						(nFindRange == 3 && nFindMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if ((nFindRange3D == 0 && (fFindMin3D <= fH && fH <= fFindMax3D)) ||
						(nFindRange3D == 1 && (fFindMin3D > fH || fH < fFindMax3D)) ||
						(nFindRange3D == 2 && fH >= fFindMax3D) ||
						(nFindRange3D == 3 && fFindMin3D >= fH))
						b2DIn = true;
				}
				if (b2DIn)
					pucImg[nIdx] = 255;

				//wedge binarization
				b2DIn = false;
				if (bWedge2D)
				{
					UCHAR ucG = srcImg1.data[y * nImgWidth + x];
					if ((nWedgeRange == 0 && (nWedgeMin2D <= ucG && ucG <= nWedgeMax2D)) ||
						(nWedgeRange == 1 && (nWedgeMin2D > ucG || ucG < nWedgeMax2D)) ||
						(nWedgeRange == 2 && ucG >= nWedgeMax2D) ||
						(nWedgeRange == 3 && nWedgeMin2D >= ucG))
						b2DIn = true;
				}
				else
					b2DIn = true;
				if (b2DIn)
				{
					b2DIn = false;
					if (bWedge3D)
					{
						if ((nWedgeRange3D == 0 && (fWedgeMin3D <= fH && fH <= fWedgeMax3D)) ||
							(nWedgeRange3D == 1 && (fWedgeMin3D > fH || fH < fWedgeMax3D)) ||
							(nWedgeRange3D == 2 && fH >= fWedgeMax3D) ||
							(nWedgeRange3D == 3 && fWedgeMin3D >= fH))
							b2DIn = true;
					}
					else
						b2DIn = true;
				}
				if (b2DIn) pucImg1[nIdx] = 255;

			}
		}

		//Remove overlapping area between wing and wedge.
		cv::Mat BinImage2(nImgHeight, nImgWidth, CV_8UC1, pucImg1);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage2.bmp", BinImage2);
#endif
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(BinImage2, BinImage2, kernel);
		cv::dilate(BinImage2, BinImage2, kernel);
#if _DEBUG
		cv::imwrite("D:\\FootRst\\GetBinImage_Mop2.bmp", BinImage2);
#endif

		cv::Mat BinImageFoot(nImgHeight, nImgWidth, CV_8UC1, pucImg);

		double dCX = 0, dCY = 0, dArea = 0;
		int nMinBlob = 100;
		nMinBlob = dArea * 0.1;
		if (100 > nMinBlob) nMinBlob = 100;
		int selectblob = eSelectMix;
		if (!pFoot->m_bUsePatternAngle)
			selectblob = eSelectBigger;
		kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7), cv::Point(1, 1));
		cv::dilate(BinImageFoot, BinImageFoot, kernel);
		cv::erode(BinImageFoot, BinImageFoot, kernel);
		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(BinImageFoot.data, cvFootBW->data, nImgWidth, nImgHeight, nMinBlob, FALSE, TRUE, 0, eSelectBigger);

		for (int y = 0; y < nImgHeight; y++)
		{
			for (int x = 0; x < nImgWidth; x++)
			{
				int nIdx = y * nImgWidth + x;
				cvWedgeBW->data[nIdx] = (cvFootBW->data[nIdx] == 255) ? BinImage2.data[nIdx] : 0;
			}
		}
#if _DEBUG
		cv::imwrite("D:\\FootRst\\Find_Wedge.bmp", *cvWedgeBW);
#endif
		Delete_1DArray(&pucImg1);

//		BinImageFoot.copyTo(*cvFootBW);

		Delete_1DArray(&pucImg);

	}
	catch (const std::exception&)
	{
		g_pMPTI->AddLog_Dev(_T("Ribbon SetBWForUIImage error"));
	}


}

double CPInsp_Ribbon::calcSeta(CFoot_Model *pFoot,cv::Mat FootImage, cv::Mat wedgeImage, cv::Point* CenterPo)
{
	double dSeta(0.0f);

	cv::Mat localWedgeImg(wedgeImage.rows, wedgeImage.cols, CV_8UC1);
	localWedgeImg.setTo(0);

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
	cv::erode(wedgeImage, localWedgeImg, kernel);
	cv::dilate(localWedgeImg, localWedgeImg, kernel);

	int nFilter = 0;
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(localWedgeImg.data, wedgeImage.data, localWedgeImg.cols, localWedgeImg.rows, 0, 0, 0, nFilter, eSelectBigger);
	localWedgeImg = wedgeImage.clone();

	//cv::imwrite("D:\\cvImglocalWedgeImg.bmp", localWedgeImg);
	cv::Mat integ, Devi;

	localWedgeImg &= 1;
	cv::integral(localWedgeImg, integ, Devi);
	int bot = pFoot->m_BondingRect.bottom;
	for (int r = integ.rows - 1; r > 0; r--)
	{
		if (integ.ptr<int>(r)[integ.cols - 1] - integ.ptr<int>(r - 1)[integ.cols - 1] > 0)
		{
			bot = r;
			break;
		}
	}
	int top = 0;
	for (int r = 0; r < bot; r++)
	{
		if (integ.ptr<int>(r)[integ.cols - 1] > 0)
		{
			top = r;
			break;
		}
	}
	int right = integ.cols;
	for (int c = right - 1; c > 0; c--)
	{
		if (integ.ptr<int>(integ.rows - 1)[c] - integ.ptr<int>(integ.rows - 1)[c - 1] > 0)
		{
			right = c;
			break;
		}
	}
	int left = 0;
	for (int c = 0; c < right; c++)
	{
		if (integ.ptr<int>(integ.rows - 1)[c] > 0)
		{
			left = c;
			break;
		}
	}

	CenterPo->x = (right - left + 1) / 2 + left - 0.5;
	CenterPo->y = (bot - top + 1) / 2 + top - 0.5;
	cv::Point RebCenterPo(0, 0);
	double RebSeta(0.0f);
	cv::Mat RoFoot;
	dSeta = -(pFoot->m_nFootAngle);
	WarpAffine(FootImage, RoFoot, dSeta, FootImage.size(), *CenterPo);
	Insp_FootPos(&RoFoot, &RebCenterPo, &RebSeta);
	dSeta += (RebSeta - 90);
	if (std::abs(std::abs(dSeta) - std::abs(pFoot->m_nFootAngle)) > 135)
		dSeta += 180;
	if (dSeta < 0)
		dSeta += 360;
	double teachSeta = pFoot->m_dTeachFootAngle < 0 ? pFoot->m_dTeachFootAngle + 360 : pFoot->m_dTeachFootAngle;
	double AngleDiff = std::abs(dSeta - teachSeta);
	if (pFoot->m_dTeachFootAngle != 0 && (AngleDiff > 45 && AngleDiff < 135/*|| (AngleDiff > 135 && AngleDiff <225)*/))
		dSeta += 90;

	return dSeta;
}

double CPInsp_Ribbon::calcSeta2(CFoot_Model *pFoot, cv::Mat FootImage, cv::Mat wedgeImage, cv::Mat Image3D, cv::Point* CenterPo)
{
	double dSeta = (0.0f);
	int nLine = __LINE__;
	try
	{
		//calc Point
		cv::Mat localWedgeImg(wedgeImage.rows, wedgeImage.cols, CV_8UC1);
		localWedgeImg.setTo(0);
		nLine = __LINE__;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(1, 1));
		cv::erode(wedgeImage, localWedgeImg, kernel);
		cv::dilate(localWedgeImg, localWedgeImg, kernel);
		nLine = __LINE__;
		int nFilter = 0;
		int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(localWedgeImg.data, wedgeImage.data, localWedgeImg.cols, localWedgeImg.rows, 0, 0, 0, nFilter, eSelectBigger);
		localWedgeImg = wedgeImage.clone();
		nLine = __LINE__;
		//cv::imwrite("D:\\cvImglocalWedgeImg.bmp", localWedgeImg);
		cv::Mat integ, Devi;

		localWedgeImg &= 1;
		cv::integral(localWedgeImg, integ, Devi);
		int bot = pFoot->m_BondingRect.bottom;
		for (int r = integ.rows - 1; r > 0; r--)
		{
			if (integ.ptr<int>(r)[integ.cols - 1] - integ.ptr<int>(r - 1)[integ.cols - 1] > 0)
			{
				bot = r;
				break;
			}
		}
		int top = 0;
		for (int r = 0; r < bot; r++)
		{
			if (integ.ptr<int>(r)[integ.cols - 1] > 0)
			{
				top = r;
				break;
			}
		}
		int right = integ.cols;
		for (int c = right - 1; c > 0; c--)
		{
			if (integ.ptr<int>(integ.rows - 1)[c] - integ.ptr<int>(integ.rows - 1)[c - 1] > 0)
			{
				right = c;
				break;
			}
		}
		int left = 0;
		for (int c = 0; c < right; c++)
		{
			if (integ.ptr<int>(integ.rows - 1)[c] > 0)
			{
				left = c;
				break;
			}
		}
		nLine = __LINE__;
		CenterPo->x = (right - left + 1) / 2 + left - 0.5;
		CenterPo->y = (bot - top + 1) / 2 + top - 0.5;
		nLine = __LINE__;

		//calc Angle
		nLine = __LINE__;
		cv::Point RebCenterPo(0, 0);
		double RebSeta(0.0f);
		cv::Mat RoFoot;
		dSeta = -(pFoot->m_nFootAngle);
		WarpAffine(FootImage, RoFoot, dSeta, FootImage.size(), *CenterPo);
		nLine = __LINE__;
		cv::Mat CenterImage(RoFoot.rows, RoFoot.cols, CV_8UC1);
		CenterImage.setTo(0);
		for (int r = 0; r < RoFoot.rows; r++)
		{
			uchar* uCenterPtr = CenterImage.ptr(r);
			uchar* uPtr = RoFoot.ptr(r);
			int CenterX = 0, nCnt = 0;
			for (int c = 0; c < RoFoot.cols; c++)
			{
				if (uPtr[c] > 0)
				{
					CenterX += c;
					nCnt++;
				}
			}
			if (nCnt > 1)
			{
				CenterX = CenterX / nCnt;
				uCenterPtr[CenterX] = 255;
			}
		}
		nLine = __LINE__;
		Insp_FootPos(&CenterImage, &RebCenterPo, &RebSeta);
		dSeta += (RebSeta - 90);
		if (std::abs(std::abs(dSeta) - std::abs(pFoot->m_nFootAngle)) > 135)
			dSeta += 180;
		double AngleDiff = std::abs(dSeta - pFoot->m_dTeachFootAngle);
		if (pFoot->m_dTeachFootAngle != 0 && AngleDiff > 45 && AngleDiff < 135)
			dSeta += 90;
	}
	catch (CMemoryException* e)
	{
		CString sLog;
		sLog.Format(_T("CPInsp_Ribbon::calcSeta2 : CMemoryException Line: %d"), nLine);
		g_pMPTI->AddLog(sLog);
		return 0.0f;
	}
	catch (CFileException* e)
	{
		CString sLog;
		sLog.Format(_T("CPInsp_Ribbon::calcSeta2 : CFileException Line: %d"), nLine);
		g_pMPTI->AddLog(sLog);
		return 0.0f;
	}
	catch (CException* e)
	{
		CString sLog;
		sLog.Format(_T("CPInsp_Ribbon::calcSeta2 : CException Line: %d"), nLine);
		g_pMPTI->AddLog(sLog);
		return 0.0f;
	}



	return dSeta;
}
BOOL CPInsp_Ribbon::InspFoot(CFoot_Model* pFoot, WndAlgoImg &sWndAlgoImg, RstAlgoFoot * sRstAlgo, TotalInspExceptArea stTieArea, AlgoColorOpt &ptrAlgoColorOpt, int nSelectFoot, UCHAR *ucArrDstImg, AlignResult * arrAlignRes, bool bWingRstVisible, cv::Mat *cv3DAvgFilter, CString sSaveDebugPath )
{
	BOOL ret = FALSE;
	int nLine = __LINE__;


	nLine = __LINE__;

	//USHORT* LabelImage = NULL;
	memset(&sRstAlgo->m_sEnd, 0, sizeof(POINTF));
	try
	{
#pragma region 1.Set Algo Values

		sRstAlgo->m_poWire_Wnd.x = pFoot->m_nWindowStartX;
		sRstAlgo->m_poWire_Wnd.y = pFoot->m_nWindowStartY;
		//Get Foot Search ROI Size
		int nWidth = pFoot->GetImageWidth();		//Foot Search Area 
		int nHeight = pFoot->GetImageLength();

		int nFootDirection = 0;
		nFootDirection = pFoot->m_nFootAngle;

		int nBlobCnt = 0;
		bool bIsFoundFoot = false;
		std::vector<cv::Rect> rcFootROIList;

		int nStartX = 0, nStartY = 0;
		nStartX = pFoot->mImageRect.left - (ptrAlgoColorOpt.m_sFovImg.nImageSizeX / 2 - ptrAlgoColorOpt.m_sImgBuf.nImageSizeX);
		nStartY = pFoot->mImageRect.top - (ptrAlgoColorOpt.m_sFovImg.nImageSizeY / 2 - ptrAlgoColorOpt.m_sImgBuf.nImageSizeY);
		if (ptrAlgoColorOpt.m_sImgBuf.nImageSizeX <= 0) nStartX = pFoot->mImageRect.left;
		if (ptrAlgoColorOpt.m_sImgBuf.nImageSizeY <= 0)nStartY = pFoot->mImageRect.top;
		if (nStartY < 0)
			nStartY = 0;

		cv::Rect ImageRect(pFoot->mImageRect.left, pFoot->mImageRect.top, nWidth, nHeight);
		cv::Mat Img(nHeight, nWidth, CV_8UC1), ImgWedge(nHeight, nWidth, CV_8UC1), ImgWing(nHeight, nWidth, CV_8UC1), Img3DArea(nHeight, nWidth, CV_32FC1);
		cv::Mat ImgTemp(nHeight, nWidth, CV_32FC1, pFoot->pf3D);
		ImgTemp.copyTo(Img3DArea);

		cv::Mat PadImg;
		Calc_padBinImage3(pFoot, sRstAlgo, ImageRect, &PadImg, Img3DArea, pFoot->bTeachBin);
		int nImgWidth = ImageRect.width;
		int nImgHeight = ImageRect.height;
		int imgSize = nImgWidth * nImgHeight;
		long pLebel[PTR_BLOB_MAX];
		//Make_1DArray((PCHAR)__FUNCTION__, __LINE__,&LabelImage, imgSize);
		memset(pLebel, 0, sizeof(long) * PTR_BLOB_MAX);
		//memset(LabelImage, 0, sizeof(USHORT) * imgSize);

		cv::Mat cvWingCandi_SecondImg;
		FootPoins fPoints;
		std::vector<cv::Mat> InspImages;
		InspImages.clear();
#pragma endregion
		nLine = __LINE__;
#pragma region 2. validity check

		nBlobCnt = FindFoot_Pat(pFoot, sRstAlgo, &Img, &ImgWedge, &ImgWing, &PadImg, fPoints, nSelectFoot);
		if (nBlobCnt > 0) bIsFoundFoot = true;

		if (nBlobCnt == 0)
		{
			sRstAlgo->m_bFind = FALSE;
			sRstAlgo->m_bArrOK[m_eFoot_Deviation] == FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);

				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;

			}
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("CPInsp_Foot::InspFoot BlobCount = 0 "));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			return false;
		}

		if (!bIsFoundFoot)
		{
			sRstAlgo->m_bFind = FALSE;
			sRstAlgo->m_bArrOK[m_eFoot_Deviation] == FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);

				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;

			}
			//Delete_1DArray(&LabelImage);
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("CPInsp_Foot::InspFoot 2. validity check !bIsFoundFoot"));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			return false;
		}
		sRstAlgo->m_nFindBin = 0;
		//sRstAlgo->m_bOK = TRUE;

		float fPadAreaAverageHgt = 0.0f;
		fPadAreaAverageHgt = pFoot->m_fPadArea3DAvgHeight;
		sRstAlgo->m_fAreaHeight = fPadAreaAverageHgt;

#if _DEBUG
		//디버깅 이미지 저장(sSaveDebugPath)  part 넘버,Win 순서, Algo 넘버 순으로
		if (sSaveDebugPath.IsEmpty() != TRUE)
		{
			cstDebugFolderPath = sSaveDebugPath;

			CT2CA convertedString(sSaveDebugPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_FootOrgImg.bmp"), Img);
		}
#endif

#pragma endregion
		nLine = __LINE__;
#pragma region 3. foot Type Inspection
		int nRet = FootPointByType(pFoot, sRstAlgo, fPoints, &Img, &ImgWedge, &ImgWing, &PadImg, ucArrDstImg, &cvWingCandi_SecondImg, InspImages, nStartX, nStartY, nSelectFoot, bWingRstVisible);		//Detect Points From Candidates images 
		if (nRet < 0)
		{
			sRstAlgo->m_bFind = FALSE;
			sRstAlgo->m_bArrOK[m_eFoot_Deviation] == FALSE;
			for (int footType = 0; footType < m_eFoot_Total; footType++)
			{
				int footDataType = convertToFootData(footType);

				if ((pFoot->m_nUseOption & footDataType) == footDataType)
					sRstAlgo->m_bArrOK[footType] = FALSE;

			}
			//Delete_1DArray(&LabelImage);
#if FOOT_RST_WRITE
			CString sLog;
			sLog.Format(_T("CPInsp_Foot::InspFoot 3. foot Type Inspection nRet < 0"));
			g_pMPTI->AddLog_OCR(sLog);
#endif
			return false;
		}
		cv::Point CenterPos(0, 0);
		CenterPos.x = fPoints.m_pCenter.x;
		CenterPos.y = fPoints.m_pCenter.y;
#pragma endregion
		nLine = __LINE__;

#if _DEBUG
		//디버깅 이미지 저장
		if (cstDebugFolderPath.IsEmpty() != TRUE)
		{
			CT2CA convertedString(cstDebugFolderPath);
			std::string strFootDebugImgPath = std::string(convertedString);

			size_t lastindex = strFootDebugImgPath.find_last_of(".");
			string rawname = strFootDebugImgPath.substr(0, lastindex);

			cv::imwrite(rawname + std::string("_BackGroundBWImg.bmp"), PadImg);
			cv::imwrite(rawname + std::string("_FootBWImg.bmp"), Img);
		}
#endif

#pragma region 4. create foot Images

		cv::Point Margin = FootImageByPoint(pFoot, fPoints, InspImages);		//Clip image using points (output images are real sub area)

		//GetBlob_BondingFloor(pFoot, sRstAlgo, nBlobCnt, pLebel, LabelImage, &Img, &ImgWedge, &ImgWing, &Img3DArea, rcFootROIList, nWidth, nHeight);

		//debug code
#if _DEBUG
		CString path = g_pMPTI->GetWireDebugDataFullPath(nSelectFoot, _T("FootRst"), _T("WedgeBin"), 0);
		m_pProcMilAlgo->SaveDebugImg(ImgWedge.ptr(), nWidth, nHeight, path, 1, FALSE);

		path = g_pMPTI->GetWireDebugDataFullPath(nSelectFoot, _T("FootRst"), _T("WingBin"), 0);
		m_pProcMilAlgo->SaveDebugImg(ImgWing.ptr(), nWidth, nHeight, path, 1, FALSE);

		path = g_pMPTI->GetWireDebugDataFullPath(nSelectFoot, _T("FootRst"), _T("FootBin"), 0);
		m_pProcMilAlgo->SaveDebugImg(Img.ptr(), nWidth, nHeight, path, 1, FALSE);
#endif

#pragma endregion
		nLine = __LINE__;
#pragma region 5. calculate items

		//InspImages.push_back(Img);//Deviation image
		//5 to 8
		sRstAlgo->m_nFindBin = 1;
		cv::Point Center = FootMeasure(pFoot, sRstAlgo, InspImages, ucArrDstImg, &cvWingCandi_SecondImg, CenterPos, fPoints.m_dSeta, Margin, nStartX, nStartY, nSelectFoot, bWingRstVisible);

#pragma endregion
		nLine = __LINE__;
#pragma region 6. end Function


		if (sRstAlgo->m_bFind) // Pad 영역 설정
		{

			POINT pt;
			pt.x = Center.x;
			pt.y = Center.y;

			CRect rect;
			rect.left = pFoot->m_PadRect.left;
			rect.top = pFoot->m_PadRect.top;
			rect.right = pFoot->m_PadRect.right;
			rect.bottom = pFoot->m_PadRect.bottom;
			if (rect.PtInRect(pt))
			{
				int nstX = pFoot->mImageRect.left - pFoot->mWindowImageRect.left; //Window 내의 Image ROI 시작 좌표
				int nstY = pFoot->mImageRect.top - pFoot->mWindowImageRect.top;

				sRstAlgo->m_rcSearchArea.left = rect.left + nstX;
				sRstAlgo->m_rcSearchArea.top = rect.top + nstY;
				sRstAlgo->m_rcSearchArea.right = rect.right + nstX;
				sRstAlgo->m_rcSearchArea.bottom = rect.bottom + nstY;
			}
		}


		nLine = __LINE__;
		//WIndow LT 기준으로 Position 변경
		int nImageStartXInWindowLT = pFoot->GetImageStartXInWindowLT();
		int nImageStartYInWindowLT = pFoot->GetImageStartYInWindowLT();

		for (int poIdx = 0; poIdx < m_eFoot::m_eFoot_Total; poIdx++)
		{

			if (sRstAlgo->rePoinSt[poIdx].x != 0 || sRstAlgo->rePoinSt[poIdx].y != 0
				|| sRstAlgo->rePoinEd[poIdx].x != 0 || sRstAlgo->rePoinEd[poIdx].y != 0)
			{
				//WindowLT 기준으로 변경
				sRstAlgo->rePoinSt[poIdx].x = sRstAlgo->rePoinSt[poIdx].x + nImageStartXInWindowLT;
				sRstAlgo->rePoinSt[poIdx].y = sRstAlgo->rePoinSt[poIdx].y + nImageStartYInWindowLT;

				sRstAlgo->rePoinEd[poIdx].x = sRstAlgo->rePoinEd[poIdx].x + nImageStartXInWindowLT;
				sRstAlgo->rePoinEd[poIdx].y = sRstAlgo->rePoinEd[poIdx].y + nImageStartYInWindowLT;


				sRstAlgo->DispPoinSt[poIdx].x = sRstAlgo->DispPoinSt[poIdx].x + nImageStartXInWindowLT;
				sRstAlgo->DispPoinSt[poIdx].y = sRstAlgo->DispPoinSt[poIdx].y + nImageStartYInWindowLT;

				sRstAlgo->DispPoinEd[poIdx].x = sRstAlgo->DispPoinEd[poIdx].x + nImageStartXInWindowLT;
				sRstAlgo->DispPoinEd[poIdx].y = sRstAlgo->DispPoinEd[poIdx].y + nImageStartYInWindowLT;
			}
		}

		if (sRstAlgo->m_sPoint.x != 0 || sRstAlgo->m_sPoint.y != 0)
		{
			sRstAlgo->m_sPoint.x = sRstAlgo->m_sPoint.x + nImageStartXInWindowLT;
			sRstAlgo->m_sPoint.y = sRstAlgo->m_sPoint.y + nImageStartYInWindowLT;
		}
		if (pFoot->m_RoRect != nullptr)
		{
			cv::Point2f padRect[4];
			pFoot->m_RoRect->center.x += pFoot->mWindowImageRect.left;
			pFoot->m_RoRect->center.y += pFoot->mWindowImageRect.top;
			pFoot->m_RoRect->points(padRect);

			sRstAlgo->m_nTeachRectLB.x = (int)std::roundf(padRect[0].x);
			sRstAlgo->m_nTeachRectLT.x = (int)std::roundf(padRect[1].x);
			sRstAlgo->m_nTeachRectRT.x = (int)std::roundf(padRect[2].x);
			sRstAlgo->m_nTeachRectRB.x = (int)std::roundf(padRect[3].x);

			sRstAlgo->m_nTeachRectLB.y = (int)std::roundf(padRect[0].y);
			sRstAlgo->m_nTeachRectLT.y = (int)std::roundf(padRect[1].y);
			sRstAlgo->m_nTeachRectRT.y = (int)std::roundf(padRect[2].y);
			sRstAlgo->m_nTeachRectRB.y = (int)std::roundf(padRect[3].y);
		}
		else
		{
			RECT FootShiftSpecArea = pFoot->m_PadRect;

			sRstAlgo->m_nTeachRectLT.x = (int)std::roundf(FootShiftSpecArea.left);
			sRstAlgo->m_nTeachRectLB.x = (int)std::roundf(FootShiftSpecArea.left);
			sRstAlgo->m_nTeachRectRB.x = (int)std::roundf(FootShiftSpecArea.right);
			sRstAlgo->m_nTeachRectRT.x = (int)std::roundf(FootShiftSpecArea.right);

			sRstAlgo->m_nTeachRectLT.y = (int)std::roundf(FootShiftSpecArea.top);
			sRstAlgo->m_nTeachRectLB.y = (int)std::roundf(FootShiftSpecArea.top);
			sRstAlgo->m_nTeachRectRB.y = (int)std::roundf(FootShiftSpecArea.bottom);
			sRstAlgo->m_nTeachRectRT.y = (int)std::roundf(FootShiftSpecArea.bottom);
		}

		nLine = __LINE__;
		for (int algoCnt = 0; algoCnt < (int)m_eFoot::m_eFoot_Bonding1; algoCnt++)
		{
			//if (sRstAlgo->m_bOK == FALSE)
				//break;
			int footDataType = convertToFootData(algoCnt);
			if ((pFoot->m_nUseOption & footDataType) != footDataType)
				continue;
			if (sRstAlgo->m_bArrOK[algoCnt] == FALSE)
				sRstAlgo->m_bOK = FALSE;
		}
		ret = sRstAlgo->m_bOK;

		//Delete_1DArray(&LabelImage);
#pragma endregion


	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("CPInsp_Foot::InspFoot error Line: %d pass"), nLine);
		g_pMPTI->AddLog(msg);
		//if(LabelImage != NULL)
		//	Delete_1DArray(&LabelImage);
		return ret;
	}

	return ret;
}