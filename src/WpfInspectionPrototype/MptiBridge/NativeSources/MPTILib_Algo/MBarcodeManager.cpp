//#include "StdAfx.h"
//#include "MBarcodeManager.h"
//#include <afxwin.h>
//
//
//CMBarcodeManager* g_pInspBarcode = NULL;
//
//
//CMBarcodeManager::CMBarcodeManager(void)
//{
//	g_pInspBarcode = this;
//
//	m_inspParam = NULL;
//	m_fovImg = NULL;
//}
//
//
//CMBarcodeManager::~CMBarcodeManager(void)
//{
//	if(m_inspParam != NULL)
//		//delete [] m_inspParam;
//		g_pMManager->pem_delete(m_inspParam, false);
//	g_pInspBarcode = NULL;
//}
//
//int CMBarcodeManager::Init(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
//{
//	int ret = eBCD_SUCCESS;
//
//	m_milApp = milApp;	// mil application
//	m_milSys = milSys;	// mil system
//
//	//m_milProc = new CProcMil;
//	m_milProc = g_pMManager->pem_new<CProcMil>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
//	m_milProc->InitMil(m_milApp, m_milSys,bUseImagePilLib);
//	m_milProc->SetResol(m_resolX, m_resolY, m_fovWidth);
//
//	return ret;
//}
//
//int CMBarcodeManager::Exit()
//{
//	int ret = eBCD_SUCCESS;
//
//	if(m_milProc != NULL)
//	{
//		//JACKY 2013/09/27
//		m_milProc->FreeMil();
//
//		//delete m_milProc;
//		g_pMManager->pem_delete(m_milProc, false);
//		m_milProc = NULL;
//	}
//
//	return ret;
//}
//
//void CMBarcodeManager::SetResolution(int fovWidth, int fovLength, double resolX, double resolY)
//{
//	m_fovWidth = fovWidth;
//	m_fovLength = fovLength;
//	m_resolX = resolX;
//	m_resolY = resolY;
//}
//
//
//int CMBarcodeManager::SetBarcodeParam(InspBarcodeParam* param)
//{
//	int ret = eBCD_SUCCESS;
//
//// 	m_inspParam = param;
//
//	if(m_inspParam != NULL)
//		//delete [] m_inspParam;
//		g_pMManager->pem_delete(m_inspParam, false);
//	m_inspParam = NULL;
//	//m_inspParam = new InspBarcodeParam;
//	m_inspParam = g_pMManager->pem_new<InspBarcodeParam>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
//	m_inspParam->devType = param->devType;
//	m_inspParam->fovCx = param->fovCx;
//	m_inspParam->fovCy = param->fovCy;
//	m_inspParam->cx = param->cx;
//	m_inspParam->cy = param->cy;
//	m_inspParam->width = param->width;
//	m_inspParam->length = param->length;
//	m_inspParam->imgSizeX = param->imgSizeX;
//	m_inspParam->imgSizeY = param->imgSizeY;
//	m_inspParam->barcodeType = param->barcodeType;
//	m_inspParam->m_InspBarcodeType = param->m_InspBarcodeType;
//	m_inspParam->foreColor = param->foreColor;
//	m_inspParam->fovIndex = param->fovIndex;
//	m_inspParam->lightType = param->lightType;
//	m_inspParam->XFlip = param->XFlip;
//	m_inspParam->YFlip = param->YFlip;
//	m_inspParam->checkSum = param->checkSum;
//	m_inspParam->nCount = param->nCount;
//	m_inspParam->nThresholdType = param->nThresholdType;
//	m_inspParam->nUseModuleProgram = param->nUseModuleProgram;
//	memcpy(m_inspParam->sModuleBarcodeID, param->sModuleBarcodeID, sizeof(wchar_t) * MAX_STRLEN);
//
//	m_coordi = SetCoordi(param);
//
//	return ret;
//}
//
//int CMBarcodeManager::SetBarcodeParam_Img(UCHAR * fovImg)
//{
//	m_fovImg = fovImg;
//	return eBCD_SUCCESS;
//}
//
//Coordinate CMBarcodeManager::SetCoordi(InspBarcodeParam* pam)
//{
//	Coordinate ret;
//	CPoint pt;
//
//	//pt = CvtBoradToPixel(pam->fovCx, pam->fovCy, pam->cx, pam->cy);
//	double imgWidth = (pam->imgSizeX * m_resolX);  //unit : pixel -> mm
//	double imgLength = (pam->imgSizeY * m_resolY);  //unit : pixel -> mm
//	double rstXmm = -pam->fovCx + (imgWidth / 2.0) + pam->cx;  //unit : mm
//	double rstYmm = pam->fovCy + (imgLength / 2.0) - pam->cy;  //unit : mm]
//	pt.SetPoint(RounD(rstXmm / m_resolX), RounD(rstYmm / m_resolY));
//
//	ret.cx = pt.x;
//	ret.cy = pt.y;
//
//	ret.width = (int)((pam->width  / m_resolX));// + 0.5);
//	ret.length = (int)((pam->length  / m_resolY));// + 0.5);
//
//
//#if _OFFLINE_INSP
//	ret.cx = (int)pam->cx;
//	ret.cy = (int)pam->cy;
//
//	ret.width = (int)pam->width;
//	ret.length = (int)pam->length;
//
//#endif
//
//	ret.fovCx = pam->fovCx;
//	ret.fovCy = pam->fovCy;
//	ret.bdrCx = 0;
//	ret.bdrCy = 0;
//	ret.bdrWidth = 0;
//	ret.bdrLength = 0;
//
//	return ret;
//}
//
//// SHKang 2017/10/24 : return value 정리 (SPI 와 모듈 통합되면 변경될수 있음.) : 현재 DTK 일때와 MIL 일때 틀림...
//// 0 : 성공, 1 : 실패
//int CMBarcodeManager::InspBarcode(wchar_t** rst, UCHAR* pImg, UCHAR* pucInspImg, int nInspImgW, int nInspImgH, wchar_t* sBarcodeImageSavePath, wchar_t* sBarcodeImageSavePath_NewSeq, int InspType)
//{
//	UCHAR* pFovImg = NULL;
//	if(pImg != NULL)
//		pFovImg = pImg;
//	else if(m_fovImg != NULL)
//		pFovImg = m_fovImg;
//	
//
//	int ret = eBCD_FAIL;	// SHKang 2017/10/24 : default 는 Fail 로 들어가는게 편하다.
//
//	int XFlip = m_inspParam->XFlip;
//	int YFlip = m_inspParam->YFlip;
//
//	int checkSum = m_inspParam->checkSum;
//
//	int imgSizeX = m_inspParam->imgSizeX;
//	int imgSizeY = m_inspParam->imgSizeY;
//	int barcodeType = m_inspParam->barcodeType;
//	int foreColor = m_inspParam->foreColor;
//	int nCount = m_inspParam->nCount;
//	int nThresholdType = m_inspParam->nThresholdType;
//	int nUseAngle = m_inspParam->nUseAngle;
//	double dAngle = m_inspParam->dAngle;
//	int nStdAngle = m_inspParam->nStdAngle;
//	bool bIsOrientationOK = m_inspParam->bIsOrientationOK;
//
//	CPoint stPoint, edPoint;
//
//	stPoint.x = (LONG)(m_coordi.cx - (m_coordi.width / 2.0));
//	if(stPoint.x < 0)
//		stPoint.x = 0;
//	stPoint.y = (LONG)(m_coordi.cy - (m_coordi.length / 2.0));
//	if(stPoint.y < 0)
//		stPoint.y = 0;
//
//	edPoint.x = (LONG)(stPoint.x + m_coordi.width);
//	if(edPoint.x < 0)
//		edPoint.x = 0;
//	edPoint.y = (LONG)(stPoint.y + m_coordi.length);
//	if(edPoint.y < 0)
//		edPoint.y = 0;
//	int nClipWidth = edPoint.x - stPoint.x + 1;
//	int nClipHeight = edPoint.y - stPoint.y + 1;
//
//	CString stdStr2323 = _T("");
//	stdStr2323.Format(_T("%d - barcodeImg.bmp"), m_inspParam->fovIndex);
//
//	int fovArea = imgSizeX * imgSizeY;
//	m_milProc->SaveWorkImg2(pFovImg, imgSizeX, imgSizeY, stdStr2323);
//	CString strRst = _T("");
//// 	CString msg;
//// 	msg.Format(_T("Insp type: %d"),InspType);
//// 	AfxMessageBox(msg);
//// 	msg.Format(_T("strRst: %f",&strRst));
//// 	AfxMessageBox(msg);
//
// 	//cv::Mat src_img = cv::imread(std::string("D:\\Eagle3D_data\\Job\\kimchoen\\20170316_kimchoen\\20170316_kimchoen\\PPDB\\Load_Acbr.bmp"),cv::IMREAD_GRAYSCALE);
//// 	cv::Mat src_img = cv::imread(std::string("D:\\Work\\Barcode\\image\\Yekani\\1_20180207093047.bmp"),cv::IMREAD_GRAYSCALE);
////  	stPoint.x = 0; stPoint.y=0; edPoint.x=src_img.cols; edPoint.y=src_img.rows;
//	CString strFilePath = _T("");
//	int Path_max_Count = 1;
//	// BCD_Open(src_img.cols, src_img.rows, TRUE);
//
//	int nCheck = m_inspParam->nUseModuleProgram;
//
//	if (nCheck == 1)
//	{
//		BCD_EncoderType_Order(InspType, XFlip, YFlip, checkSum, nCount);
//		//int ninputCheck = BCD_ModuleProcessInput(pFovImg, imgSizeX, imgSizeY, stPoint, edPoint, barcodeType, foreColor, m_inspParam->sModuleBarcodeID, /*strFilePath, Path_max_Count, */XFlip, YFlip, checkSum, nCount, nThresholdType);
//		//int ninputCheck = BCD_ModuleProcessInput(pFovImg, imgSizeX, imgSizeY, stPoint, edPoint, barcodeType, foreColor, m_inspParam->sModuleBarcodeID.GetBuffer(), /*strFilePath, Path_max_Count, */XFlip, YFlip, checkSum, nCount, nThresholdType);
//	}
//	else
//	{
//		if (BCD_Use_NEW_Barcode_Get() == TRUE)
//		{
//			BCD_EncoderType_Order(InspType, XFlip, YFlip, checkSum, nCount);
//			ret = BCD_Inspection(pFovImg, imgSizeX, imgSizeY, stPoint, edPoint, barcodeType, foreColor, &strRst, /*strFilePath, Path_max_Count, */XFlip, YFlip, checkSum, nCount, nThresholdType, nUseAngle, &dAngle, nStdAngle, &bIsOrientationOK);
//		}
//	}
//
//	if (nInspImgW > 0 && nInspImgH > 0 && nClipWidth >= nInspImgW && nClipHeight >= nInspImgH &&
//		(stPoint.x + nInspImgW < imgSizeX) && (stPoint.y + nInspImgH < imgSizeY) && pucInspImg)
//		m_milProc->GetClipImage_LT(pFovImg, imgSizeX, imgSizeY, pucInspImg, stPoint.x, stPoint.y, nInspImgW, nInspImgH);
//// 	else
//// 	{
//// 		//ret = CItoP_Insp_Barcode(m_inspParam->img, imgSizeX, imgSizeY, stPoint, edPoint, barcodeType, foreColor, XFlip, YFlip, strRst, checkSum, nCount);
//// 		if(InspType == 0)	ret = CItoP_Insp_Barcode(m_inspParam->img, imgSizeX, imgSizeY, stPoint, edPoint, barcodeType, foreColor, XFlip, YFlip, strRst, checkSum, nCount, nThresholdType);
//// 		else if(InspType == 1)ret = ExInsp_Barcode(m_inspParam->img, imgSizeX, imgSizeY, stPoint, edPoint, barcodeType, foreColor, nCount, &strRst);
//// 	}
//
//	// CItoP_Insp_Barcode : 무조건 0 리턴한다., ExInsp_Barcode : 성공시에는 1로 리턴한다. 따라서 strRst 값을 보고 처리해야 한다. 향후 수정 필요해 보임.
//
//// 	msg.Format(_T("ret: %d"),ret);
//// 	AfxMessageBox(msg);
//	if(sBarcodeImageSavePath != NULL || sBarcodeImageSavePath_NewSeq != NULL)
//	{
//		if((stPoint.x + nClipWidth < imgSizeX) && (stPoint.y + nClipHeight < imgSizeY))
//		{
//			//UCHAR* imgBuf = new UCHAR[nClipWidth * nClipHeight]; 
//			UCHAR* imgBuf = g_pMManager->pem_new<UCHAR>(true, nClipWidth * nClipHeight, (PCHAR)__FUNCTION__, __LINE__);
//			m_milProc->GetClipImage_LT(pFovImg, imgSizeX, imgSizeY, 
//				imgBuf, stPoint.x, stPoint.y, nClipWidth, nClipHeight);
//			if(sBarcodeImageSavePath != NULL)
//				m_milProc->SaveTIFImage(imgBuf, nClipWidth, nClipHeight, 1, sBarcodeImageSavePath);
//			
//			if(sBarcodeImageSavePath_NewSeq != NULL) 
//			{
//				if(_tcscmp(sBarcodeImageSavePath_NewSeq,_T("")) != 0)
//				{
//					if(_taccess(sBarcodeImageSavePath_NewSeq, 0 )!=-1)
//					{
//						DeleteFile(sBarcodeImageSavePath_NewSeq);
//					}
//					m_milProc->SaveTIFImage(imgBuf, nClipWidth, nClipHeight, 1, sBarcodeImageSavePath_NewSeq);
//				}
//			}
//			//delete [] imgBuf;
//			g_pMManager->pem_delete(imgBuf, true);
//			imgBuf = NULL;
//		}
//	}
////	AfxMessageBox(msg);
//
//	// SHKang 2017/10/24 : strRst 가 문자가 있을때 처리하는 코드에 문제가 있음. 
//	wsprintf(*rst ,strRst);	
//	if(strRst != _T("")) ret = eBCD_SUCCESS;
//	else ret = eBCD_FAIL;
//// 	if(strRst != _T(""))
//// 	{
//// 		int nStrLength = strRst.GetLength();
//// 		int nMaxCnt = 50;
//// 		if(nStrLength > nMaxCnt)
//// 			strRst.Delete(nMaxCnt, nStrLength - nMaxCnt);
//// 		wsprintf(*rst ,strRst);
//// 	}
//	if(m_fovImg != NULL && m_inspParam->lightType == 8)
//	{
//		//delete [] m_fovImg;
//		g_pMManager->pem_delete(m_fovImg, true);
//		m_fovImg = NULL;
//	}
//
//	return ret;
//}
//
//bool CMBarcodeManager::InspBarcodeOutput(wchar_t** rst, wchar_t* sModuleBarcodeID, int nDelaytime)
//{
//	CString strRst = _T("");
//	std::clock_t m_Time = std::clock();
//	int nResultCheck = 0;
//	std::clock_t elapsed = 0;
//	while (elapsed < nDelaytime)
//	{
//		//algotool 에서 바코드 모듈프로그램은 실행 안하기에 주석처리
//		//nResultCheck = BCD_ModuleProcessIDSelectResult(sModuleBarcodeID, &strRst);
//		if (nResultCheck != 0)
//			break;
//// 		std::vector<std::vector<CString>> Result_Id_CodeString;
//// 		nResultCheck = BCD_ModuleProcessResult_Vector(Result_Id_CodeString);
//// 		
//// 		if (nResultCheck != 0)
//// 		{
//// 			strRst.Format(_T("%s"), Result_Id_CodeString[0][1]);
//// 			break;
//// 		}
//		
//		std::clock_t endTime = std::clock();
//		elapsed = endTime - m_Time;
//		Sleep(10);
//	}
//	if(strRst !=  _T(""))
//		wsprintf(*rst, strRst);
//
//	return nResultCheck == 1 ? true : false;
//}
//
//bool CMBarcodeManager::GetInspImg(UCHAR* pImg, int nImgW, int nImgH, UCHAR* pBigImg)
//{
//	if (pImg == NULL ||
//		nImgW <= 0 || nImgW != m_inspParam->imgSizeX ||
//		nImgH <= 0 || nImgH != m_inspParam->imgSizeY)
//		return false;
//	if (pBigImg != NULL)
//	{
//		int nIndex = 0;
//		for (int a = 0; a < nImgW * nImgH; a++)
//		{
//			for (int b = 0; b < 3; b++)
//			{
//				pImg[nIndex] = pBigImg[a];
//				nIndex++;
//			}
//		}
//	}
//	else
//	{
//		if (m_fovImg != NULL)
//			memcpy(pImg, m_fovImg, nImgW * nImgH * sizeof(UCHAR));
//	}
//	return true;
//}
////////////////////////////////////////////////////////////////////////////
//
//
//
//int  MPTI_InspBarcode(wchar_t* rst, UCHAR* pImg, UCHAR* pucInspImg, int nInspImgW, int nInspImgH, wchar_t* sBarcodeImageSavePath, wchar_t* sBarcodeImageSavePath_NewSeq, int InspType)
//{
//	if (g_pInspBarcode == NULL)
//		return eMR_FAIL;	// -1 일때는 class pointer 가 없음.
//
//
//	keybd_event(VK_F24, 0, 0, 0);                                      //VKey(Down)
//	keybd_event(VK_F24, 0, KEYEVENTF_KEYUP, 0);        //VKey(UP)
//
///*	AfxMessageBox(_T("in MPTI_InspBarcode"));*/
//	int nRet = g_pInspBarcode->InspBarcode(&rst, pImg, pucInspImg, nInspImgW, nInspImgH, sBarcodeImageSavePath, sBarcodeImageSavePath_NewSeq, InspType);
//
//	return nRet;	// 0: 성공, 1 : 검사 실패
//}
//bool MPTI_InspBarcodeOutput(wchar_t* rst, wchar_t* sModuleBarcodeID, int nDelaytime)
//{
//	return g_pInspBarcode->InspBarcodeOutput(&rst, sModuleBarcodeID, nDelaytime);
//}
//bool MPTI_MILBarcodeLicense()
//{
//	if (g_pInspBarcode == NULL)
//		return eMR_FAIL;
//	//return MILBarcodeLicense();
//	return BCD_Get_Mil_License();
//}
//
//bool MPTI_MILBarcodeStop()
//{
//	if (g_pInspBarcode == NULL)
//		return eMR_FAIL;
//	BCD_Inspect_Stop();
//	return true;
//}
//bool  MPTI_GetInspImg(UCHAR* pImg, int nImgW, int nImgH, UCHAR* pBigImg)
//{
//	if (g_pInspBarcode == NULL || pImg == NULL || nImgW <= 0 || nImgH <= 0)
//		return false;
//	return g_pInspBarcode->GetInspImg(pImg, nImgW, nImgH, pBigImg);
//}