#include "StdAfx.h"
#include "PInsp_OCV.h"
#include "MPTI.h"

#include "ippi.h"
#include "ipps.h"
#pragma comment(lib, "ippi.lib")
#pragma comment(lib, "ipps.lib")
#include "ExtInspRoot.h"
#include "ExtLog.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

#define AI_IMGMARGIN 1.1f
CString AI_ImageSaveDir;

CPInsp_POCR::CPInsp_POCR(void)
{
	//m_procPil = new CProcPil_POCR();
	m_procPil = g_pMManager->pem_new<CProcPil_POCR>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_roiSizeX = 0;
	m_roiSizeY = 0;

	m_nstringIndex = 0;

	m_pInspAlgoPOCR = NULL;
// 
// 	g_pMPTI->_inferOCR = std::shared_ptr<csml::PyOCR_AI_AutoModule>(new csml::PyOCR_AI_AutoModule); //kji 21-05-10
// 
// 	OCR_Dll_CreateInterface();
// 	OCR_Dll_ReadFileOCR(_T("A1.bmp"));
	AI_ImageSaveDir = _T("D:\\Eagle3D_data\\AI_Sample");
	m_sBoard = _T("");
	m_sPartCode = _T("");
	m_sRefID = _T("");
	m_sModulID = _T("");
	AISampleImageSaveNG = false;
	AISampleImageSaveOK = false; 
#if AI_ON
	AiResultClear(1);
	AiResultClear(0);
	AIImageSavePartListClear(1);
	AIImageSavePartListClear(0);
#endif
}

CPInsp_POCR::~CPInsp_POCR(void)
{
	if(m_procPil != NULL)
	{
		//delete m_procPil;
		g_pMManager->pem_delete(m_procPil, false);
		m_procPil = NULL;
	}
// 	if(m_Opencv_Functions != NULL)
// 	{
// 		delete m_Opencv_Functions;
// 		m_Opencv_Functions = NULL;
	// 	}
#if AI_ON
	AiResultClear(1);
	AiResultClear(0);
	AIImageSavePartListClear(1);
	AIImageSavePartListClear(0);
	OCR_Dll_DestroyInterface();
#endif
}
int CPInsp_POCR::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	m_procPil->InitMil(milApp, milSys, bUseImagePilLib);

	return true;
}

int CPInsp_POCR::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	// Allocate a Application.
	//m_milApp = milApp;
	// Allocate a System.
	//m_milSys = milSys;

	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	m_procPil->InitMil(milApp, milSys,bUseImagePilLib);
	m_procPil->SetResol(resolX, resolY, m_fovWidth);

	CPInsp::InitDevice(milApp, milSys, fovWidth, fovLength, resolX, resolY,bUseImagePilLib);
	UsePOCRTxt= false;
	//m_sTrainingFilePath = _T("C:\\Eagle3D_64x\\init\\AI_POCR.cs2"); 
	//AI OCR °Ë»ç ¿¹½Ã

#if AI_ON
	m_CRAFT_AI = nullptr;
	if (g_pMPTI->m_nlnspIndexAI > 0)
	{
		m_CRAFT_AI = std::shared_ptr<csml::CRAFT_AI_AutoModule>(new csml::CRAFT_AI_AutoModule());
		std::vector<cv::Mat> _in;
		_in.emplace_back(cv::Mat(200, 200, CV_8UC3));
		m_CRAFT_AI_Result = m_CRAFT_AI->CRAFT_AI_returnROI(2, _in);
	}
#endif

	return ePART_SUCCESS;
}
void CPInsp_POCR::Setstr(CString str)
{
	m_procPil->Setstr(str);
}
int CPInsp_POCR::SetRect(UCHAR* srcImg, UCHAR* GrayImg, UCHAR* BinOrg, int sizeX, int sizeY, int fontColor, int threshVal, RECT* TeachRect, int nLength)
{
	try
	{
#if AI_ON
		m_CRAFT_AI_Result.clear();
#else
		std::vector<std::vector<cv::Point2f>> Po;
#endif
		cv::Mat srcimg(sizeY, sizeX, CV_8UC1, srcImg);
		cv::Mat Grayimg(sizeY, sizeX, CV_8UC1, GrayImg);
		cv::Mat BinOrgImg(sizeY, sizeX, CV_8UC1, BinOrg);
#if AI_ON
		return m_procPil->FontRect(srcimg, Grayimg, BinOrgImg, fontColor, threshVal, TeachRect, nLength, m_CRAFT_AI_Result);
#else
		return m_procPil->FontRect(srcimg, Grayimg, BinOrgImg, fontColor, threshVal, TeachRect, nLength, Po);
#endif
	}
	catch (CMemoryException* e)
	{
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return 0;
}
int CPInsp_POCR::SetRectAuto(UCHAR* srcImg, UCHAR* GrayImg, UCHAR* BinOrg, int sizeX, int sizeY, int fontColor, int threshVal, RECT* TeachRect, int nLength)
{
	try
	{
		cv::Mat srcimg(sizeY, sizeX, CV_8UC1, srcImg);
		cv::Mat Grayimg(sizeY, sizeX, CV_8UC1, GrayImg);
		cv::Mat BinOrgImg(sizeY, sizeX, CV_8UC1, BinOrg);
		return m_procPil->FontRectAuto(srcimg, Grayimg, BinOrgImg, fontColor, threshVal, TeachRect, nLength);
	}
	catch (CMemoryException* e)
	{
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return 0;
}
int CPInsp_POCR::SetStrFont(UCHAR* srcImg,UCHAR* GrayImg,UCHAR* BinOrg, int sizeX, int sizeY, double angle, int fontColor,  int threshVal, RECT TeachRect, RECT* FontRect,HWND Handle)
{
	cv::Mat srcimg(sizeY,sizeX,CV_8UC1,srcImg);
	cv::Mat Grayimg(sizeY,sizeX,CV_8UC1,GrayImg);
	cv::Mat BinOrgImg(sizeY,sizeX,CV_8UC1,BinOrg);
	bool flag = m_procPil->AddFont(srcimg,Grayimg,BinOrgImg,fontColor,threshVal, TeachRect, FontRect, Handle);

	return flag?ePART_SUCCESS:ePART_FAIL;
}
int CPInsp_POCR::SetStrFont_NGViewer(UCHAR* srcImg,UCHAR* GrayImg,UCHAR* BinOrg, int sizeX, int sizeY, double angle, int fontColor, CString ImgPath,int nThreshold, RECT TeachRect)
{
	cv::Mat srcimg(sizeY,sizeX,CV_8UC1, srcImg);
	cv::Mat Grayimg(sizeY,sizeX,CV_8UC1,GrayImg);
	cv::Mat BinOrgImg(sizeY,sizeX,CV_8UC1,BinOrg);

	bool flag = m_procPil->AddFont_NG(srcimg.clone(),Grayimg.clone(),BinOrgImg.clone(),fontColor,ImgPath, nThreshold,TeachRect);
	return flag?ePART_SUCCESS:ePART_FAIL;
}
int CPInsp_POCR::SetStrFont_ReTeaching(UCHAR* srcImg, UCHAR* GrayImg, UCHAR* BinOrg, int sizeX, int sizeY, double angle, int fontColor, int threshVal, RECT TeachRect, RECT* FontRect)
{
	//cv::Mat srcimg(sizeY, sizeX, CV_8UC1, srcImg);
	//cv::Mat Grayimg(sizeY, sizeX, CV_8UC1, GrayImg);
	//cv::Mat BinOrgImg(sizeY, sizeX, CV_8UC1, BinOrg);
	////bool flag = m_procPil->AddFont_ReTeaching(srcimg, Grayimg, BinOrgImg, fontColor, threshVal, TeachRect, FontRect);

	//return flag?ePART_SUCCESS:ePART_FAIL;
	return 0;
}

int CPInsp_POCR::LoadPsrFile(CString path,int fontColor, CString readfonts, CString destPath,bool bOrg)
{
	bool flag = m_procPil->LoadFontList(path, fontColor, readfonts, destPath,bOrg);

	return flag?ePART_SUCCESS:ePART_FAIL;
}

int CPInsp_POCR::LoadPsrFile(CString path)
{
	bool flag = m_procPil->ModelFileLoad(path);

	return flag?0:1;
}

int CPInsp_POCR::SavePsrFile(CString path,bool bAddMode)
{
	bool flag = m_procPil->SaveFontList(path,bAddMode);

	return flag?ePART_SUCCESS:ePART_FAIL;
}

int CPInsp_POCR::DeletePsrFont(CString ch,int index)
{
	bool flag = m_procPil->DeleteFont(ch,index);

	return flag?ePART_SUCCESS:ePART_FAIL;
}


int CPInsp_POCR::InspProc_POCR(UCHAR* userSrc,int width,int height, int nInspectionMode)
{
	CString strFullPath;
	strFullPath.Format(_T("%s"), m_pInspAlgoPOCR->m_sFontPath);
	int nr = strFullPath.Find(_T("InspectResult"));

	for(int i = 0; i < m_pInspAlgoPOCR->m_nCharMaxCount; i++)
	{
		if(m_pInspAlgoPOCR->m_dStdCharScore[i] <= 0)
			m_pInspAlgoPOCR->m_dStdCharScore[i] = 50.0;
	}

// 	CString sOwnTool;
// 	sOwnTool.Format(_T("Tool%d"), (int)ext::env::nTool_id);
// 	int nSubStrIdx = strFullPath.Find(_T("POCRFont"));
// 	strFullPath.Insert(nSubStrIdx + 8, sOwnTool);
	if(nr != -1){

		if(m_procPil->ModelFileLoad(strFullPath)== ePART_FAIL)
		{
			m_result.m_dStringScore = 0;
			m_result.m_bOKPolarity = FALSE;
			m_result.m_bOKScore = FALSE;
			m_result.m_bOKString = FALSE;
			return e_NG;
		}
	}
	else if(m_procPil->ModelLoad(strFullPath)==ePART_FAIL)
	{
		if(m_procPil->ModelFileLoad(strFullPath)== ePART_FAIL)
		{
			m_result.m_dStringScore = 0;
			m_result.m_bOKPolarity = FALSE;
			m_result.m_bOKScore = FALSE;
			m_result.m_bOKString = FALSE;
			return e_NG;
		}
	}
//#if BlidErrCode
	ImgProc    = std::make_shared<ImgProcessing_POCR>();
	ImgProc_ro = std::make_shared<ImgProcessing_POCR>();
//#endif
	m_procPil->SetImgProc(userSrc,width,height,ImgProc,m_pInspAlgoPOCR);
	bool flag = m_procPil->SearchPOCR(m_pInspAlgoPOCR);

	m_result = m_procPil->POCRresult;


#ifdef _DEBUG
	cv::Mat charRectImg = m_procPil->orgImage;
	CreateDirectory(_T("D:\\testimage\\POCRTest"), NULL);
	cv::imwrite("D:\\testimage\\POCRTest\\charRectImg.bmp", charRectImg);
	for(int i=0;i<m_procPil->POCRresult.m_nCharCount;i++)
	{
		int Rectx = m_procPil->POCRresult.charX[i]-m_procPil->POCRresult.charWidth[i]/2;
		int Recty = m_procPil->POCRresult.charY[i]-m_procPil->POCRresult.charHeight[i]/2;
		cv::rectangle(charRectImg,cv::Rect(Rectx,Recty,m_procPil->POCRresult.charWidth[i],m_procPil->POCRresult.charHeight[i]),cv::Scalar(255));
	}
	cv::imwrite("D:\\testimage\\POCRTest\\charRectImg_Rect.bmp",charRectImg);
#endif
	bool roFlag = false;
	int Cnt(0),roCnt(0);
	if(!flag && m_pInspAlgoPOCR->m_bUsePolarity == FALSE)
	{
		double meanRe(0),meanRoRe(0);
		m_procPil->SetImgProc(userSrc,width,height,ImgProc_ro,m_pInspAlgoPOCR,true);
		flag = m_procPil->SearchPOCR(m_pInspAlgoPOCR);

#ifdef _DEBUG
		charRectImg = m_procPil->orgImage;
		for(int i=0;i<m_procPil->POCRresult.m_nCharCount;i++)
		{
			int Rectx = m_procPil->POCRresult.charX[i]-m_procPil->POCRresult.charWidth[i]/2;
			int Recty = m_procPil->POCRresult.charY[i]-m_procPil->POCRresult.charHeight[i]/2;
			cv::rectangle(charRectImg,cv::Rect(Rectx,Recty,m_procPil->POCRresult.charWidth[i],m_procPil->POCRresult.charHeight[i]),cv::Scalar(255));
		}
		cv::imwrite("D:\\testimage\\POCRTest\\charRectImg_ro.bmp",charRectImg);
#endif
		for(int i=0;i<m_result.m_nCharCount;i++)
		{
			if(m_pInspAlgoPOCR->m_dStdCharScore[i]>m_result.m_dArrCharScore[i])
			{
				Cnt++;
			}
			if(m_pInspAlgoPOCR->m_dStdCharScore[i]>m_procPil->POCRresult.m_dArrCharScore[i])
			{
				roCnt++;
			}
			meanRe +=m_result.m_dArrCharScore[i];
			meanRoRe +=m_procPil->POCRresult.m_dArrCharScore[i];
		}
		if(Cnt > roCnt)
		{
#if AI_ON
			CopyAIPosition(m_result,m_procPil->POCRresult);
#endif
			m_result = m_procPil->POCRresult;
			roFlag = true;
			m_result.RstRotate = m_result.Insp_Ro = 1;
		}
		else if(Cnt == roCnt && meanRe/m_result.m_nCharCount<meanRoRe/m_procPil->POCRresult.m_nCharCount)
		{
#if AI_ON
			CopyAIPosition(m_result,m_procPil->POCRresult);
#endif
			m_result = m_procPil->POCRresult;
			roFlag = true;
			m_result.RstRotate = m_result.Insp_Ro = 1;
		}
#if AI_ON
		else
		{
			CopyAIPosition(m_procPil->POCRresult,m_result);
		}
#endif
		m_result.Insp_Ro = 1;
	}

// 	for(int i=0;i<m_pInspAlgoPOCR->m_nCharMaxCount;i++)
// 	{
// 		if(m_pInspAlgoPOCR->m_sTargetFont[i] != m_result.m_cArrStr[i])
// 		{
// 			m_result.m_bOKString = false;
// 		}
// 	}
	if(wcscmp(m_pInspAlgoPOCR->m_sTargetFont, m_result.m_cArrStr) != 0)
	{
		flag = m_result.m_bOKString = FALSE;
	}

	if(!flag)
	{
		m_result.m_bOKPolarity = FALSE;
	}

	bool bAdditionalInspOK = true;
	for (int i = 0; i < m_result.m_nCharCount; i++)
	{
		if ((m_pInspAlgoPOCR->m_bUseContrast && m_result.bContrastOK[i] == FALSE) || (m_pInspAlgoPOCR->m_bUseOP && m_result.bOPOK[i] == FALSE)
			|| (m_pInspAlgoPOCR->m_bUseUP && m_result.bUPOK[i] == FALSE) || (m_pInspAlgoPOCR->m_bUseBlobSize && m_result.bBlobSizeOK[i] == FALSE))
		{
			flag = false;
			bAdditionalInspOK = false;
			break;
		}
	}
#if AI_ON
	if(g_pMPTI->m_nlnspIndexAI > 0 && bAdditionalInspOK)
		SetFontImgForAI(roFlag);

#endif
	if(m_pInspAlgoPOCR->m_bUsePOCRNG)
	{
		flag = !flag;
	}

#if AI_ON
	//shkim AI Data Save
	if (AIImageSavePartCnt[m_sPartCode] < 0)
		AIImageSavePartCnt[m_sPartCode] = 0;
	if(g_pMPTI->m_nlnspIndexAI > 0 && (AIImageSavePartCnt[m_sPartCode] < nAIMaxSaveCount || nAIMaxSaveCount == 0))
	//if((AISampleImageSaveNG && flag == false)||(flag && AISampleImageSaveOK))
	{

		CString ssKey;
		ssKey.Format(_T("%s/%s/%s/%s/%d"),m_sModulID,m_sPartCode,m_sRefID,m_WndIdx,m_AlgoIdx);
		CString ssPartKey;
		ssPartKey.Format(_T("%s/%s/%s"),m_sModulID,m_sPartCode,m_sRefID);

 		double dMxSc(0.0f),dMnSc(100.0f);
 		AiResultLoad(ssPartKey,&dMnSc,&dMxSc);
		bool bSave = false;
		bool bImgSave = false;
		if((AISampleImageSaveNG && flag == false)||(flag && AISampleImageSaveOK))
		{
 			for (int i = 0; i < m_result.m_nCharCount; i++)
 			{
 				if (m_result.m_dArrCharScore[i] > dMxSc)
 					bImgSave = true;
  			}
		}
		if((m_mSavePartList[m_nCurJob][m_sRefID] == m_sPartCode))
			bSave = true;
		if(m_mNGSavePartList[m_nCurJob][m_sRefID] == m_sPartCode && flag == false)
			bSave = true;

		if(m_sBoard != _T("") && (bImgSave | bSave))
		{
			//image set
			std::shared_ptr<PIAL::ImgProcessing_POCR> ImgProcTmp = ImgProc;
			if(roFlag)
				ImgProcTmp = ImgProc_ro;
			//std::shared_ptr<cv::Mat> detectImage;
			//detectImage = std::make_shared< PIAL::ImgProcessing_POCR>(ImgProcTmp->edge->_OrgImage.get());

			//save
			AiResultSave(ssKey,m_result,ssPartKey);
			CString sFIlePath;
			CFileFind Finder;
			if (bImgSave)
			{
				//file path
// 				int fileExist = Finder.FindFile(m_sBoard);
// 				if (!fileExist)
// 					CreateDirectory(m_sBoard, NULL);
				sFIlePath.Format(_T("%s_%s_%s_%s_%d"), m_sModulID, m_sPartCode, m_sRefID, m_WndIdx, m_AlgoIdx);
				CString sImagePath;
				sImagePath.Format(_T("%s\\%s.bmp"), m_sBoard, sFIlePath);

				AIImageSaveList[m_nCurJob][ssPartKey].first = sImagePath;
				AIImageSaveList[m_nCurJob][ssPartKey].second = ImgProcTmp;
				AIImageSavePartCnt[m_sPartCode]++;
				//AIImageSaveList[m_nCurJob][ssPartKey].second = detectImage;
				//cv::imwrite(std::string(CT2A(sImagePath)), detectImage);
			}
			else if(bSave)
			{
				//file path
// 				int fileExist = Finder.FindFile(m_sBoard);
// 				if (!fileExist)
// 					CreateDirectory(m_sBoard, NULL);
				sFIlePath.Format(_T("%s_%s_%s_%s_%d"), m_sModulID, m_sPartCode, m_sRefID, m_WndIdx, m_AlgoIdx);
				CString sImagePath;
				sImagePath.Format(_T("%s\\%s.bmp"), m_sBoard, sFIlePath);
				AIImageSaveList_Forced[m_nCurJob][ssPartKey].first = sImagePath;
				AIImageSaveList_Forced[m_nCurJob][ssPartKey].second = ImgProcTmp;
				
			}
		}
	}
	else if (AIImageSavePartCnt[m_sPartCode] >= nAIMaxSaveCount && nAIMaxSaveCount != 0)
	{
		CString sSavePartCode;
		sSavePartCode.Format(_T("%s count over!! cnt %d / Max %d"), m_sPartCode, AIImageSavePartCnt[sSavePartCode], nAIMaxSaveCount);
		g_pMPTI->AddLog_OCR(sSavePartCode);
	}
	//shkim POCR result txt
	if(UsePOCRTxt && flag == false)
	{
		cv::Rect re(0,0,0,0);

		std::shared_ptr<PIAL::ImgProcessing_POCR> ImgProcTmp = ImgProc;

		if(roFlag)
			ImgProcTmp = ImgProc_ro;
		if(m_procPil->FontModel->_sfList.size()<1)
		{
			re.width = ImgProcTmp->edge->_OrgImage->cols;
			re.height = ImgProcTmp->edge->_OrgImage->rows;
		}
		else
		{
			re.x = m_result.charX[0]-m_result.charWidth[0]/2;
			re.y = m_result.charY[0]-m_result.charHeight[0]/2;
			re.width = m_result.charWidth[0];
			re.height = m_result.charHeight[0];
		}
		for(int idx=0;idx<m_procPil->FontModel->_sfList.size();idx++)
		{
			re.width = m_procPil->FontModel->_sfList[idx]->_OrgImage->Image().cols>re.width?m_procPil->FontModel->_sfList[idx]->_OrgImage->Image().cols:re.width;
			re.height = m_procPil->FontModel->_sfList[idx]->_OrgImage->Image().rows>re.height?m_procPil->FontModel->_sfList[idx]->_OrgImage->Image().rows:re.height;
		}

		re.y -= 10;
		re.height += 10;

		cv::Mat detectImage;
		detectImage = (*ImgProcTmp->edge->_OrgImage.get());
		//(m_procPil->ImgProc->edge->_OrgImage->rows,m_procPil->ImgProc->edge->_OrgImage->cols,m_procPil->ImgProc->edge->_OrgImage->data);
		if(re.x<0)
			re.x=0;
		if(re.width+re.x >= detectImage.cols)
			re.width = detectImage.cols - re.x-1;

		if(re.y<0)
			re.y=0;
		if(re.height+re.y >= detectImage.rows)
			re.height = detectImage.rows - re.y-1;

		cv::Mat tmp = detectImage(re).clone();
		CString sReadStr = OCR_Dll_ReadOCR(tmp.data,tmp.cols,tmp.rows);

		for(int i=0;i<sReadStr.GetLength();i++)
		{
			if(m_result.m_nCharScoreResult[i]==0)
				continue;
			m_result.m_cArrStr[i] = sReadStr.GetAt(i);
			m_result.m_dArrCharScore[i] = 0;
		}
		if(sReadStr.GetLength()>0)
		{
			//AfxMessageBox(sReadStr);
			for(int i=sReadStr.GetLength();i<m_result.m_nCharCount;i++)
				m_result.m_cArrStr[i] = _T('?');
		}
		m_result.m_dStringScore = 0;

		//cv::imwrite("D:\\Eagle3D_data\\Job\\M\\0854221GBA_Offline_1\\Master\\POCRFontImg\\detectImage_tmp.bmp",tmp);
	}
//  //shkim Read speed Test code
// 	CString sFilePath;
// 	CString imgName[7] = {_T("101408823@window5@Algo1@0.bmp"),_T("101408831@window3@Algo1@0.bmp"),_T("101408832@window5@Algo1@0.bmp"),_T("101408834@window5@Algo1@0.bmp")
// 		,_T("103063500@window4@Algo1@0.bmp"),_T("103063542@window4@Algo1@0.bmp"),_T("PartCode160@window5@Algo1@0.bmp")};
// 	cv::Mat vReadImg[6];
// 	for(int imgIdx=0;imgIdx<6;imgIdx++)
// 	{
// 		//sFilePath.Format(_T("D:\\Eagle3D_data\\Job\\M\\0854221GBA_Offline_1\\Master\\POCRFontImg\\Part%d.bmp"),(imgIdx+1));
// 		sFilePath.Format(_T("D:\\Eagle3D_data\\Job\\M\\0854221GBA_Offline_1\\Master\\POCRFontImg\\%s"),imgName[imgIdx]);
// 		vReadImg[imgIdx] = cv::imread(std::string(CT2A(sFilePath)),cv::IMREAD_GRAYSCALE);
// 	}
// 	CString sReadString[6];
// 	double dArrTime[6];
// 	double dTime(0);
// 	CQTimer qtm; 
// 	for(int imgIdx=0;imgIdx<6;imgIdx++)
// 	{
// 		qtm.StartTick();
// 		sReadString[imgIdx] = OCR_Dll_ReadOCR(vReadImg[imgIdx].data,vReadImg[imgIdx].cols,vReadImg[imgIdx].rows);
// 		dArrTime[imgIdx] = (qtm.EndTick() * 1000.0);
// 		dTime +=dArrTime[imgIdx];
// 	}
// 
// 	CString testTime;
// 	testTime.Format(_T("%.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f"),dArrTime[0],dArrTime[1],dArrTime[2],dArrTime[3],dArrTime[4],dArrTime[5],dTime);
// 	AfxMessageBox(testTime);
#endif
	return flag?e_OK:e_NG;
}

int CPInsp_POCR::GetInspRst(RstAlgoPOCR* pDstAlgoPOCR)
{
	pDstAlgoPOCR->m_dStringScore = m_result.m_dStringScore;

	for(int i=0;i<m_result.m_nCharCount;i++)
	{
		if(m_pInspAlgoPOCR->m_dStdCharScore[i]<=m_result.m_dArrCharScore[i])
			pDstAlgoPOCR->m_bOKScore = TRUE;
		else
		{

			pDstAlgoPOCR->m_bOKScore = FALSE;
			break;
		}
	}
	pDstAlgoPOCR->m_bOKString = m_result.m_bOKString;
	pDstAlgoPOCR->m_bOKPolarity = m_result.m_bOKPolarity;
	pDstAlgoPOCR->m_nCharCount = m_result.m_nCharCount;

	pDstAlgoPOCR->ModelX = m_result.ModelX;
	pDstAlgoPOCR->ModelY = m_result.ModelY;
	pDstAlgoPOCR->ModelWidth = m_result.ModelWidth;
	pDstAlgoPOCR->ModelHeight = m_result.ModelHeight;

	for(int i=0;i<m_result.m_nCharCount;i++)
	{
		pDstAlgoPOCR->charAngle[i] = m_result.charAngle[i];
		pDstAlgoPOCR->charX[i] = m_result.charX[i] ;
		pDstAlgoPOCR->charY[i] = m_result.charY[i];
		pDstAlgoPOCR->charWidth[i] = m_result.charWidth[i] ;
		pDstAlgoPOCR->charHeight[i] = m_result.charHeight[i];
		pDstAlgoPOCR->m_dArrCharScore[i] = m_result.m_dArrCharScore[i];
		pDstAlgoPOCR->m_nCharScoreResult[i] = m_result.m_nCharScoreResult[i];
		pDstAlgoPOCR->m_cArrStr[i] = m_result.m_cArrStr[i];

		pDstAlgoPOCR->dContrastScore[i] = m_result.dContrastScore[i];
		pDstAlgoPOCR->bContrastOK[i] = m_result.bContrastOK[i];
		pDstAlgoPOCR->dUPScore[i] = m_result.dUPScore[i];
		pDstAlgoPOCR->bUPOK[i] = m_result.bUPOK[i];
		pDstAlgoPOCR->dOPScore[i] = m_result.dOPScore[i];
		pDstAlgoPOCR->bOPOK[i] = m_result.bOPOK[i];
		pDstAlgoPOCR->dBlobSizeValue[i] = m_result.dBlobSizeValue[i];
		pDstAlgoPOCR->bBlobSizeOK[i] = m_result.bBlobSizeOK[i];
		pDstAlgoPOCR->bFontAngleOK[i] = m_result.bFontAngleOK[i];

		pDstAlgoPOCR->sFont_AI[i] = m_result.sFont_AI[i];
		pDstAlgoPOCR->Score_AI[i] = m_result.Score_AI[i];
		pDstAlgoPOCR->Score_AI_Sec[i] = m_result.Score_AI_Sec[i];
	}
	pDstAlgoPOCR->bAIOK = m_result.bAIOK;

	pDstAlgoPOCR->bRstShiftX = m_result.bRstShiftX;
	pDstAlgoPOCR->dRstShiftX = m_result.dRstShiftX;
	pDstAlgoPOCR->bRstShiftY = m_result.bRstShiftY;
	pDstAlgoPOCR->dRstShiftY = m_result.dRstShiftY;
	pDstAlgoPOCR->RstRotate = m_result.RstRotate;
	m_result.reset();
	return ePART_SUCCESS;
}

int CPInsp_POCR::SetInspParam( const InspAlgo &sInspAlgo,double angle)
{
	if(sInspAlgo.m_eAlgoType != eAlgoPOCR)
		return ePART_FAIL;
	m_pInspAlgoPOCR = (AlgoPOCR *)sInspAlgo.m_ptrInspAlgoParam;
	if(!m_pInspAlgoPOCR)
		return ePART_FAIL;
	m_pInspAlgoPOCR->m_dWndAngle = angle;
	m_WndIdx.Format(_T("%s"),m_pInspAlgoPOCR->m_sWindName);
	m_AlgoIdx = sInspAlgo.m_nAlgoId;
	return ePART_SUCCESS;
}
#if AI_ON
int CPInsp_POCR::SetAIImageParam(CString sBoard,CString sPartCode, CString sRefId, CString sModuleID)
{
	if(sBoard != _T(""))
		m_sBoard.Format(_T("%s"),sBoard);
	if(sPartCode != _T(""))
		m_sPartCode.Format(_T("%s"),sPartCode);
	m_sRefID = sRefId;
	m_sModulID = sModuleID;
	return ePART_SUCCESS;
}
#endif
int CPInsp_POCR::SetPOCRData(POCRInfoParam *pPOCRInfoParam)
{
	bool flag = m_procPil->SetPOCRData(pPOCRInfoParam);

	return flag?ePART_SUCCESS:ePART_FAIL;
}

int CPInsp_POCR::ClearFont()
{
	bool flag = m_procPil->ClearFont();

	return flag?ePART_SUCCESS:ePART_FAIL;
}

int CPInsp_POCR::POCRInspTeach(POCRInspParam &sPOCRInspParam, int nImageCnt, CString ModePath, POCRFontFile *ImageFiles, CString ModelFont, int PsrListIndex)
{
	bool flag = m_procPil->POCRInspTeach(sPOCRInspParam, nImageCnt, ModePath, ImageFiles, ModelFont, PsrListIndex);

	return flag?ePART_SUCCESS:ePART_FAIL;
}

int CPInsp_POCR::POCRGetInspResult(POCRInspResult *rstPOCRInspResult)
{
	bool flag = m_procPil->POCRGetInspResult(rstPOCRInspResult);

	return flag?ePART_SUCCESS:ePART_FAIL;
}

int CPInsp_POCR::POCRGetTeachResImg(UCHAR* userSrc,int width,int height, UCHAR * ucArrDstImgMask, int ModelIndex)
{
	bool flag = m_procPil->SearchPOCRDraw(userSrc,width,height,m_pInspAlgoPOCR,ucArrDstImgMask,ModelIndex);

	return flag?ePART_SUCCESS:ePART_FAIL;
}
int CPInsp_POCR::POCRGetTeachResImg3Ch(UCHAR* userSrc, int width, int height, UCHAR * ucArrDstImgMask, int ModelIndex)
{
	bool flag = m_procPil->SearchPOCRDraw3Ch(userSrc, width, height, m_pInspAlgoPOCR, ucArrDstImgMask, ModelIndex);

	return flag ? ePART_SUCCESS : ePART_FAIL;
}
int CPInsp_POCR::LoadModelList(CString path)
{
	int ret = ePART_SUCCESS;
	ret = m_procPil->ModelList_Load(path);

	return ret;
}

int CPInsp_POCR::CheckModelList(CString path)
{
	int ret = 0;
	ret = m_procPil->ModelList_Check(path);

	return ret;
}

int CPInsp_POCR::ClearModelList(int nLane)
{
	int ret = ePART_SUCCESS;
	ret = m_procPil->ModelList_Clear(nLane);

	return ret;
}
void CPInsp_POCR::SetImgProc(std::shared_ptr<ImgProcessing_POCR> ProcImg,std::shared_ptr<ImgProcessing_POCR> ProcImg_ro)
{
	ImgProc    =ProcImg;
	ImgProc_ro = ProcImg_ro;
}
void CPInsp_POCR::SetExtPOCRFontInfo()
{
	m_procPil->SetExtPOCRFontInfo();
}
#if AI_ON
int CPInsp_POCR::ReadStringOCR( CString& sReadOCR, CString sPathOCR )
{
	sReadOCR = OCR_Dll_ReadFileOCR(sPathOCR);
	if(sReadOCR == _T(""))
		return eMR_FAIL;
	return eMR_SUCCESS;
}
#endif
int CPInsp_POCR::CheckModelCnt(CString path)
{
	int ret = ePART_FAIL;
	ret = m_procPil->ModelCnt_Check(path);

	return ret;
}

void CPInsp_POCR::SetUsePOCRTxt(bool UseTxt)
{
	UsePOCRTxt = UseTxt;
}
#if AI_ON
int CPInsp_POCR::AIImageSave(POCRAISaveInfo PocrInfo)
{
	AISampleImageSaveOK = PocrInfo.m_bOKSave;
	AISampleImageSaveNG = PocrInfo.m_bNGSave;
	AIInputIamgeSave = PocrInfo.m_bAIInputImgSave;
	nAIOKSaveCount = PocrInfo.m_nAIOKSaveCount;
	nAIMaxSaveCount = PocrInfo.m_nAIMaxSaveCount;
	return eMR_SUCCESS;
}
int CPInsp_POCR::AItxtSave(CString sFullPath)
{
	int nLine = __LINE__;
	try
	{

		CFile file;
		CFileException fe;
		if(file.Open(sFullPath, CFile::modeCreate | CFile::modeWrite, NULL, &fe)==FALSE)
			return false;

		CArchive ar(&file, CArchive::store);

		CString sName(_T("pemtron"));
		ar << sName;

		ar << m_sModulID;
		ar << m_sPartCode;
		ar << m_sRefID;
		ar <<m_result.m_nCharCount;
//		CString sTemp = _T("");
		for(int i = 0; i < m_result.m_nCharCount; i++)
		{
			//image crop
			ar <<m_result.m_cArrStr[i];
			ar <<(float)m_result.charX[i];
			ar <<(float)m_result.charY[i];
			ar <<(float)m_result.charWidth[i];
			ar <<(float)m_result.charHeight[i];
			ar <<(float)m_result.m_dArrCharScore[i];
			ar << m_result.m_bOKScore;
// 
// 			sTemp.Format(_T("%s, %s, %s, %c, %.0f, %.0f, %.0f, %.0f, %.0f, %d\n"),m_sModulID,m_sPartCode,m_sRefID,m_result.m_cArrStr[i],m_result.charX[i],m_result.charY[i],m_result.charWidth[i],m_result.charHeight[i],m_result.m_bOKScore);
// 			ar << sTemp;
		}

		ar.Close();
		file.Close();
	}
	catch (CMemoryException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::AItxtSave CMemoryException Line: %d"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return eMR_FAIL;
	}
	catch (CFileException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::AItxtSave CFileException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return eMR_FAIL;
	}
	catch (CException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::AItxtSave CException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return eMR_FAIL;
	}

	return eMR_SUCCESS;
}
double CPInsp_POCR::AItxtload(CString sFullPath, double* MinScore, double* MaxScore)
{
	int nLine = __LINE__;
	try
	{

		CFile file;
		CFileException fe;
		if(file.Open(sFullPath, CFile::modeRead, NULL, &fe)==FALSE)
			return 0.0f;

		CArchive ar(&file, CArchive::load);

		CString sName(_T(""));
		ar >> sName;
		if(sName != _T("pemtron"))
			return 0.0f;
		wchar_t ch;
		float sc(0.0f);
		float tmp(0.0f);
		BOOL bDecision;

		CString sModuleID = _T("");
		CString sPartCode = _T("");
		CString sRefID = _T("");
		CString sTemp = _T("");
		int nCharCount(0);

		ar >> sModuleID;
		ar >> sPartCode;
		ar >> sRefID;
		ar >>nCharCount;

		for(int i = 0; i < nCharCount; i++)
		{
			//image crop
			ar >> ch;//<<m_result.m_cArrStr[i];
			ar >> tmp;//<<m_result.charX[i];
			ar >> tmp;//<<m_result.charY[i];
			ar >> tmp;//<<m_result.charWidth[i];
			ar >> tmp;//<<m_result.charHeight[i];
			ar >> sc;//<<m_result.m_dArrCharScore[i];
			ar >> bDecision;//<< m_result.m_bOKScore;
			if(sc > *MaxScore)
				*MaxScore = sc;
			if(sc < *MinScore)
				*MinScore = sc;
		}

		ar.Close();
		file.Close();
	}
	catch (CMemoryException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::AIImageSave CMemoryException Line: %d"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return eMR_FAIL;
	}
	catch (CFileException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::AIImageSave CFileException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return eMR_FAIL;
	}
	catch (CException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::AIImageSave CException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return eMR_FAIL;
	}

	return *MaxScore;
}

//clear
int CPInsp_POCR::AiResultClear(int IdxJobCur)
{
	m_nCurJob = g_pInspMng->m_nCurJob = IdxJobCur;
	m_mInspRst[m_nCurJob].clear();
	m_mInspWndNum[m_nCurJob].clear();
	m_mInspAlgoNum[m_nCurJob].clear();
	m_mInspParRst[m_nCurJob].clear();
	AIImageSaveList[IdxJobCur].clear();
	AIImageSaveList_Forced[IdxJobCur].clear();
	AIOKImageList[IdxJobCur].clear();
	return eMR_SUCCESS;
}
//save
int CPInsp_POCR::AiResultSave(CString sKey, RstAlgoPOCR HeightRst, CString sPartKey)
{
	m_mInspRst[m_nCurJob][sKey] = HeightRst;
	CString sStr;
	
	m_mInspWndNum[m_nCurJob][sPartKey] = m_WndIdx;
	sStr.Format(_T("Algo%d"),m_AlgoIdx);
	m_mInspAlgoNum[m_nCurJob][sPartKey] = sStr;
	m_mInspParRst[m_nCurJob][sPartKey] = HeightRst;
	return eMR_SUCCESS;
}
//load
RstAlgoPOCR* CPInsp_POCR::AiResultLoad(CString sKey, double* MinScore, double* MaxScore)
{
	*MinScore = 0;
	*MaxScore = -1;
	if (m_mInspParRst[m_nCurJob].size() < 1)
		return NULL;

	auto iter = m_mInspParRst[m_nCurJob].find(sKey);
	if (iter == m_mInspParRst[m_nCurJob].end())
		return NULL;

	RstAlgoPOCR* HeightRst = &iter->second;

	for (int i = 0; i < HeightRst->m_nCharCount; i++)
	{
		if (*MinScore > HeightRst->m_dArrCharScore[i])
			*MinScore = HeightRst->m_dArrCharScore[i];
		if (*MaxScore < HeightRst->m_dArrCharScore[i])
			*MaxScore = HeightRst->m_dArrCharScore[i];
	}

	return HeightRst;
}
bool CPInsp_POCR::AiResultExist(CString ssKey, CString sPartKey)
{
	CString sKey;
	sKey.Format(_T("%s/%s/%s"), sPartKey,m_mInspWndNum[m_nCurJob][sPartKey], m_mInspAlgoNum[m_nCurJob][sPartKey]);
	if (sKey == ssKey)
	{
		CString sLog;
		sLog.Format(_T("%s -> %s"), sKey, ssKey);
		g_pMPTI->AddLog_OCR(sLog);
		return true;
	}
	else
		return false;
}
void CPInsp_POCR::PartStruct_AI(CString moduleId, CString id, CString partCode, CString windowId, CString algoId, CString refId, int* arrChResult, int nAlgoCnt, int nCharCount, AUTO_INSP_AI_DATA* sInsp_POCR)
{
	int nLine = __LINE__;
	try
	{
		if (sInsp_POCR)
		{
			sInsp_POCR->m_nImgW = 0;
			sInsp_POCR->m_nImgH = 0;
			sInsp_POCR->m_nImgWndW = 0;
			sInsp_POCR->m_nImgWndH = 0;
		}
		std::vector<InspPOCRAI> tempBuff;

		InspPOCRAI stTmpbuf;
		stTmpbuf.sModuleId = moduleId;
		stTmpbuf.sId = id;
		stTmpbuf.sPartCode = partCode;
		stTmpbuf.sWindowNum = windowId;
		stTmpbuf.sAlgoId = algoId;
		stTmpbuf.sRefId = refId;
		nLine = __LINE__;
		if(g_pInspMng->pNGFontData[m_nCurJob].size() > nAlgoCnt)
		{
			nLine = __LINE__;
			if (g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt].size() > 0)
			{
				nLine = __LINE__;
				stTmpbuf.Font = g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt][0].Font;
				stTmpbuf.mNGImg = g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt][0].mNGImg.clone();
				stTmpbuf.mNGImgWnd = g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt][0].mNGImgWnd.clone();
				g_pInspMng->vInspPOCRAIBuf[m_nCurJob].push_back(stTmpbuf);

				if (sInsp_POCR && stTmpbuf.mNGImg.cols > 0 && stTmpbuf.mNGImg.rows > 0)
				{
					sInsp_POCR->m_nImgW = stTmpbuf.mNGImg.cols;
					sInsp_POCR->m_nImgH = stTmpbuf.mNGImg.rows;
					if (stTmpbuf.mNGImgWnd.cols > 0 && stTmpbuf.mNGImgWnd.rows > 0)
					{
						sInsp_POCR->m_nImgWndW = stTmpbuf.mNGImgWnd.cols;
						sInsp_POCR->m_nImgWndH = stTmpbuf.mNGImgWnd.rows;
					}

					int nFontLen = stTmpbuf.Font.size();
					for (int b = 0; b < nFontLen; b++)
					{
						int nIndex = stTmpbuf.Font[b].nFntIdx;
						if (sInsp_POCR->m_nArrLen > nIndex)
						{
							sInsp_POCR->m_rcArrROI[nIndex].left = stTmpbuf.Font[b].re.x;
							sInsp_POCR->m_rcArrROI[nIndex].top = stTmpbuf.Font[b].re.y;
							sInsp_POCR->m_rcArrROI[nIndex].right = stTmpbuf.Font[b].re.x + stTmpbuf.Font[b].re.width;
							sInsp_POCR->m_rcArrROI[nIndex].bottom = stTmpbuf.Font[b].re.y + stTmpbuf.Font[b].re.height;
						}
					}
				}

				stTmpbuf.mNGImg = g_pInspMng->PairAIInspImage[m_nCurJob][nAlgoCnt].first.clone();
				g_pInspMng->AIImage[m_nCurJob].push_back(stTmpbuf);
				nLine = __LINE__;

				if (g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt].size() > 1)
				{
					stTmpbuf.sAlgoId.Format(_T("-%s"), algoId);
					stTmpbuf.Font = g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt][1].Font;
					stTmpbuf.mNGImg = g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt][1].mNGImg.clone();
					g_pInspMng->vInspPOCRAIBuf[m_nCurJob].push_back(stTmpbuf);

					stTmpbuf.mNGImg = g_pInspMng->PairAIInspImage[m_nCurJob][nAlgoCnt].second.clone();
					g_pInspMng->AIImage[m_nCurJob].push_back(stTmpbuf);
				}
			}
		}
	}
	catch (CMemoryException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::PartStruct_AI CMemoryException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return;
	}
	catch (CFileException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::PartStruct_AI CFileException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return;
	}
	catch (CException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::PartStruct_AI CException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return;
	}
}

void CPInsp_POCR::GetAI_Image(int nAlgoCnt, AUTO_INSP_AI_DATA* sInsp_POCR, int nOPT)
{
	if (sInsp_POCR == NULL)
		return;
	int nW = sInsp_POCR->m_nImgW;
	int nH = sInsp_POCR->m_nImgH;
	if (nW <= 5 || nH <= 5 || sInsp_POCR->pucImage == NULL)
		return;

	cv::Mat ocrImg = g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt][0].mNGImg.clone();
	g_pInspMng->m_procMil->SaveWorkImg(ocrImg.ptr(0), nW, nH, _T("GetAI_Image_1_Src.bmp"));
	memcpy(sInsp_POCR->pucImage, ocrImg.ptr(0), sizeof(UCHAR) * nW * nH);
	g_pInspMng->m_procMil->SaveWorkImg(sInsp_POCR->pucImage, nW, nH, _T("GetAI_Image_1_Dst.bmp"));

	if (sInsp_POCR->m_nImgWndW >= nW && sInsp_POCR->m_nImgWndH >= nH && sInsp_POCR->pucImageWnd != NULL)
	{
		cv::Mat wndImg = g_pInspMng->pNGFontData[m_nCurJob][nAlgoCnt][0].mNGImgWnd.clone();
		memcpy(sInsp_POCR->pucImageWnd, wndImg.ptr(0), sizeof(UCHAR) * sInsp_POCR->m_nImgWndW * sInsp_POCR->m_nImgWndH);
	}

	if ((nOPT & m_eAIOPT_OCR_Backup) == m_eAIOPT_OCR_Backup && sInsp_POCR->m_sLogDir)
	{
		CString filename;
		filename.Format(_T("%s"), sInsp_POCR->m_sLogDir);
		if (filename.GetLength() > 0)
			cv::imwrite(std::string(CT2A(filename)), ocrImg);
	}
}
void CPInsp_POCR::SetFontImgForAI(bool roFlag)
{
	int nLine = __LINE__;
	try
	{
		///BJY For POCR AI
		std::shared_ptr<PIAL::ImgProcessing_POCR> ImgProcTmp = ImgProc;
		if(roFlag)
			ImgProcTmp = ImgProc_ro;
		std::vector<InspPOCRAI> InspAIVec;
		InspPOCRAI inspAI, inspAI_Ro;
		InspAIVec.clear();
		inspAI.isRotate = false;
		inspAI_Ro.isRotate = true;
		POCRAITFont tmpPir;
		cv::Mat mGrayImg = (*ImgProcTmp->edge->_OrgImage.get());
		std::vector<POCRAITFont> vTmpBuff;
		cv::Mat InspImage, inspImage_Ro;
		nLine = __LINE__;
		int ModelX = m_result.ModelX - m_result.ModelWidth / 2 - 10 > 0 ? m_result.ModelX - m_result.ModelWidth / 2 - 10 : 0;
		int ModelY = m_result.ModelY - m_result.ModelHeight / 2 > 0 ? m_result.ModelY - m_result.ModelHeight / 2 : 0;
		if (ModelX + m_result.ModelWidth + 10 >= mGrayImg.cols)
			ModelX = mGrayImg.cols - (m_result.ModelWidth + 20)>0? mGrayImg.cols - (m_result.ModelWidth + 20):0;
		if (ModelY + m_result.ModelHeight >= mGrayImg.rows)
			ModelY = mGrayImg.rows - m_result.ModelHeight;
		double ModelW = m_result.ModelWidth + 20 + ModelX >= mGrayImg.cols ? mGrayImg.cols - ModelX : m_result.ModelWidth + 20;
		cv::Rect FontRect(ModelX, ModelY, ModelW, m_result.ModelHeight);
		if (m_result.ModelWidth < 1 && m_result.ModelHeight < 1)
			FontRect = cv::Rect(0, 0, mGrayImg.cols, mGrayImg.rows);
		
		int ModelRight = FontRect.x + FontRect.width;
		cv::cvtColor(mGrayImg, InspImage, cv::COLOR_GRAY2BGR);
		nLine = __LINE__;
		for(int i=0;i<m_result.m_nCharCount;i++)
		{
			if (m_result.m_nCharScoreResult[i] != 1 && m_result.m_cArrStr[i] == m_pInspAlgoPOCR->m_sTargetFont[i])
				continue;
			int Rectx = m_result.charX[i]-m_result.charWidth[i]/2 - FontRect.x;
			int Recty = m_result.charY[i]-m_result.charHeight[i]/2 - FontRect.y;

			if(Rectx<0)
				Rectx = 0;

			if(Recty<0)
				Recty = 0;

			int nLeft	= Rectx;
			int nTop	= Recty;
			int nRight	= Rectx + m_result.charWidth[i];
			int nBottom	= Recty + m_result.charHeight[i];

			int nOffset = /*max(nRight - nLeft, nBottom - nTop)*/m_result.charWidth[i] * 0.2;
			if(nOffset < 3) nOffset = 2;

			if(nLeft - nOffset < 0)
				nLeft = 0;
			else nLeft = nLeft - nOffset;

			if(nTop - nOffset < 0)
				nTop = 0;
			else nTop = nTop - nOffset;

			if (nRight + nOffset > ModelRight - 1)
			{
				if (ModelRight < nRight)
				{
					ModelRight = nRight + nOffset + FontRect.x > mGrayImg.cols - 1 ? mGrayImg.cols - 1 : nRight + nOffset + FontRect.x;
					FontRect.width = ModelRight - FontRect.x;
					nRight = ModelRight;
				}
				if(nRight<nLeft)
				{
					if(nRight - FontRect.width>0&& nRight - FontRect.width< mGrayImg.cols)
						nLeft = nRight - FontRect.width;

					CString msg;
					msg.Format(_T("if (ModelRight < nRight + nOffset + FontRect.x):  cols: %d, rows: %d / x: %d, y: %d, w: %d, h: %d"), mGrayImg.cols, mGrayImg.rows, FontRect.x, FontRect.y, FontRect.width, FontRect.height);
					g_pMPTI->AddLog_OCR(msg);
				}
			}
			else nRight = nRight + nOffset;

			if(nBottom + nOffset > FontRect.height - 1)
				nBottom = FontRect.height - 1;
			else nBottom = nBottom + nOffset;

			int nWidth = nRight - nLeft;
			int nHeight = nBottom - nTop;

			if (nLeft + nWidth > FontRect.width - 1) {nWidth = (FontRect.width - 1) - nLeft; g_pMPTI->AddLog_OCR(_T("POCRAI nLeft + nWidth > FontRect.width - 1"));}
			if (nTop + nHeight > FontRect.height - 1){ nHeight = (FontRect.height - 1) - nTop; g_pMPTI->AddLog_OCR(_T("POCRAI nLeft + nWidth > FontRect.width - 1"));}
			if (nWidth < 1)
			{
				nWidth = 1;
				nLeft = FontRect.width - 2;
				if (nLeft < 0) nLeft = 0;
				g_pMPTI->AddLog_OCR(_T("POCRAI nWidth < 1"));
			}
			if (nHeight < 1)
			{
				nHeight = 1;
				nTop = FontRect.height - 2;
				if (nTop < 0) nTop = 0;
				g_pMPTI->AddLog_OCR(_T("POCRAI nWidth < 1"));
			}
			CString sAddLog;
			sAddLog.Format(_T("FontRect: %d,%d,%d,%d / nLeft, nTop, nWidth, nHeight: %d,%d,%d,%d"), FontRect.x, FontRect.y, FontRect.x + FontRect.width, FontRect.y + FontRect.height, nLeft, nTop, nWidth, nHeight);
			g_pMPTI->AddLog_OCR(sAddLog);
			cv::Rect vClip = cv::Rect(nLeft, nTop, nWidth, nHeight);
			tmpPir.cFont = m_pInspAlgoPOCR->m_sTargetFont[i];
			tmpPir.nFntIdx = i;
			tmpPir.re = vClip;

			if (tmpPir.cFont == _T('1') || tmpPir.cFont == _T('I'))
			{
				cv::Mat ModelImg = mGrayImg(FontRect);
				cv::Mat FontImg = ModelImg(vClip).clone();
				cv::Mat Intg, Dev;
				cv::integral(FontImg, Intg, Dev);

				double d = Intg.ptr<int>((Intg.rows - 1), (Intg.cols - 1))[0];

				double dAvg = d / ((Intg.rows - 1)*(Intg.cols - 1));
				double dPow = pow(dAvg, 2);

				d = Dev.ptr<double>((Dev.rows - 1), (Dev.cols - 1))[0];
				double sqAvg = d / ((Dev.rows - 1)*(Dev.cols - 1));

				double Div = sqAvg - dPow;
				int sqDiv = RounD(sqrt(Div));

				if (sqDiv < 20 || (sqDiv <= 30 && dAvg > 220))
				{
					tmpPir.cFont = 0;
				}
			}
			vTmpBuff.emplace_back(tmpPir);
			cv::rectangle(InspImage,cv::Rect(nLeft + FontRect.x, nTop + FontRect.y,m_result.charWidth[i] + nOffset * 2,m_result.charHeight[i] + nOffset * 2),cv::Scalar(0,0,255));
		}
		nLine = __LINE__;
		inspAI.mNGImg = mGrayImg(FontRect);
		inspAI.mNGImgWnd = mGrayImg;
		if (inspAI.mNGImg.rows < 1 || inspAI.mNGImg.cols < 1)
		{
			CString msg;
			msg.Format(_T("mNGImg cols: %d, rows: %d / x: %d, y: %d, w: %d, h: %d"), inspAI_Ro.mNGImg.cols, inspAI_Ro.mNGImg.rows, FontRect.x, FontRect.y, FontRect.width, FontRect.height);
			g_pMPTI->AddLog_OCR(msg);
			inspAI_Ro.mNGImg = mGrayImg;
		}
		inspAI.Font = vTmpBuff;
		InspAIVec.push_back(inspAI);
		nLine = __LINE__;
		if(m_result.Insp_Ro == 1)
		{
			vTmpBuff.clear();
			ImgProcTmp = ImgProc_ro;
			if(roFlag)
				ImgProcTmp = ImgProc;
			mGrayImg = (*ImgProcTmp->edge->_OrgImage.get());

			ModelX = m_result.ModelX_Ro - m_result.ModelWidth_Ro / 2 - 10 > 0 ? m_result.ModelX_Ro - m_result.ModelWidth_Ro / 2 - 10 : 0;
			ModelY = m_result.ModelY_Ro - m_result.ModelHeight_Ro / 2 > 0 ? m_result.ModelY_Ro - m_result.ModelHeight_Ro / 2 : 0;
			if (ModelX + m_result.ModelWidth_Ro + 20 >= mGrayImg.cols)
				ModelX = mGrayImg.cols - (m_result.ModelWidth_Ro + 20)>0? mGrayImg.cols - (m_result.ModelWidth_Ro + 20):0;
			if (ModelY + m_result.ModelHeight_Ro >= mGrayImg.rows)
				ModelY = mGrayImg.rows - m_result.ModelHeight_Ro;
			double ModelW = m_result.ModelWidth_Ro + 20 + ModelX >= mGrayImg.cols ? mGrayImg.cols - ModelX : m_result.ModelWidth_Ro + 20;
			if (m_result.ModelWidth_Ro > 0 && m_result.ModelHeight_Ro > 0)
				FontRect = cv::Rect(ModelX, ModelY, ModelW, m_result.ModelHeight_Ro);
			else
				FontRect = cv::Rect(0, 0, mGrayImg.cols, mGrayImg.rows);
			ModelRight = FontRect.x + FontRect.width;
			cv::cvtColor(mGrayImg, inspImage_Ro, cv::COLOR_GRAY2BGR);

			for(int i=0;i<m_result.m_nCharCount;i++)
			{
				if (m_result.m_nCharScoreResult[i] != 1 && m_result.m_cArrStr[i] == m_pInspAlgoPOCR->m_sTargetFont[i])
					continue;
				int Rectx = m_result.charX_Ro[i]-m_result.charWidth_Ro[i]/2 - FontRect.x;
				int Recty = m_result.charY_Ro[i]-m_result.charHeight_Ro[i]/2 - FontRect.y;

				if(Rectx<0)
					Rectx = 0;

				if(Recty<0)
					Recty = 0;

				int nLeft	= Rectx;
				int nTop	= Recty;
				int nRight	= Rectx + m_result.charWidth_Ro[i];
				int nBottom	= Recty + m_result.charHeight_Ro[i];

				int nOffset = /*max(nRight - nLeft, nBottom - nTop)*/m_result.charWidth_Ro[i] * 0.2;
				//int nOffset = max(nRight - nLeft , nBottom - nTop) * 0.2;
				if(nOffset < 3) nOffset = 2;

				if(nLeft - nOffset < 0)
					nLeft = 0;
				else nLeft = nLeft - nOffset;

				if(nTop - nOffset < 0)
					nTop = 0;
				else nTop = nTop - nOffset;

				if (nRight + nOffset > ModelRight - 1)
				{
					if (ModelRight < nRight)
					{
						ModelRight = nRight + nOffset + FontRect.x > mGrayImg.cols - 1 ? mGrayImg.cols - 1 : nRight + nOffset + FontRect.x;
						FontRect.width = ModelRight - FontRect.x;
						nRight = ModelRight;
					}
					if (nRight < nLeft)
					{
						if (nRight - FontRect.width > 0 && nRight - FontRect.width < mGrayImg.cols)
							nLeft = nRight - FontRect.width;
						CString msg;
						msg.Format(_T("if (ModelRight < nRight + nOffset + FontRect.x):  cols: %d, rows: %d / x: %d, y: %d, w: %d, h: %d"), mGrayImg.cols, mGrayImg.rows, FontRect.x, FontRect.y, FontRect.width, FontRect.height);
						g_pMPTI->AddLog_OCR(msg);
					}
				}
				else nRight = nRight + nOffset;

				if(nBottom + nOffset > FontRect.height - 1)
					nBottom = FontRect.height - 1;
				else nBottom = nBottom + nOffset;

				int nWidth = nRight - nLeft;
				int nHeight = nBottom - nTop;

				if (nLeft + nWidth > FontRect.width - 1) {nWidth = (FontRect.width - 1) - nLeft; g_pMPTI->AddLog_OCR(_T("POCRAI nLeft + nWidth > FontRect.width - 1"));}
				if (nTop + nHeight > FontRect.height - 1) {nHeight = (FontRect.height - 1) - nTop; g_pMPTI->AddLog_OCR(_T("POCRAI nTop + nHeight > FontRect.height - 1"));}
				cv::Rect vClip = cv::Rect(nLeft, nTop, nWidth, nHeight);

				if(vClip.width<1 || vClip.height<1)//shkim ROI가 벗어난 경우 검사하지 않는다.
				{
					g_pMPTI->AddLog_OCR(_T("POCRAI nTop + nHeight > FontRect.height - 1"));
					continue; 
				}
				CString sAddLog;
				sAddLog.Format(_T("FontRect: %d,%d,%d,%d / nLeft, nTop, nWidth, nHeight: %d,%d,%d,%d"), FontRect.x, FontRect.y, FontRect.x + FontRect.width, FontRect.y + FontRect.height, nLeft, nTop, nWidth, nHeight);
				g_pMPTI->AddLog_OCR(sAddLog);
				nLine = __LINE__;
				tmpPir.cFont = m_pInspAlgoPOCR->m_sTargetFont[i];
				tmpPir.nFntIdx = i;
				tmpPir.re = vClip;
				if (tmpPir.cFont == _T('1') || tmpPir.cFont == _T('I'))
				{
					cv::Mat ModelImg = mGrayImg(FontRect);
					cv::Mat FontImg = ModelImg(vClip).clone();
					cv::Mat Intg, Dev;
					cv::integral(FontImg, Intg, Dev);

					double d = Intg.ptr<int>((Intg.rows - 1), (Intg.cols - 1))[0];

					double dAvg = d / ((Intg.rows - 1)*(Intg.cols - 1));
					double dPow = pow(dAvg, 2);

					d = Dev.ptr<double>((Dev.rows - 1), (Dev.cols - 1))[0];
					double sqAvg = d / ((Dev.rows - 1)*(Dev.cols - 1));

					double Div = sqAvg - dPow;
					int sqDiv = RounD(sqrt(Div));

					if (sqDiv < 20 || (sqDiv <= 30 && dAvg > 220))
					{
						tmpPir.cFont = 0;
					}
				}
				vTmpBuff.emplace_back(tmpPir);
				cv::rectangle(inspImage_Ro,cv::Rect(nLeft+ FontRect.x, nTop + FontRect.y,m_result.charWidth_Ro[i] + nOffset * 2,m_result.charHeight_Ro[i] + nOffset * 2),cv::Scalar(0,0,255));
			}
			nLine = __LINE__;
			inspAI_Ro.mNGImg = mGrayImg(FontRect);
			inspAI_Ro.Font = vTmpBuff;
			if (inspAI_Ro.mNGImg.rows < 1 || inspAI_Ro.mNGImg.cols < 1)
			{
				CString msg;
				msg.Format(_T("mNGImg_Ro cols: %d, rows: %d / x: %d, y: %d, w: %d, h: %d"), inspAI_Ro.mNGImg.cols, inspAI_Ro.mNGImg.rows, FontRect.x, FontRect.y, FontRect.width, FontRect.height);
				g_pMPTI->AddLog_OCR(msg);
				inspAI_Ro.mNGImg = mGrayImg;
			}
			InspAIVec.push_back(inspAI_Ro);
			vTmpBuff.clear();
		}
		nLine = __LINE__;
		g_pInspMng->pNGFontData[m_nCurJob].push_back(InspAIVec);
		nLine = __LINE__;

		std::pair<cv::Mat,cv::Mat> tempPair;
		tempPair.first = InspImage;
		tempPair.second = inspImage_Ro;
		g_pInspMng->PairAIInspImage[m_nCurJob].push_back(tempPair);
	}
	catch (CMemoryException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::SetFontImgForAI CMemoryException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return;

	}
	catch (CFileException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::SetFontImgForAI CFileException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return;
	}
	catch (CException* e)
	{
		CString msg;
		msg.Format(_T("CPInsp_POCR::SetFontImgForAI CException Line: %d : %s"), nLine);
		g_pMPTI->AddLog_OCR(msg);
		return;
	}
}
void CPInsp_POCR::AIImageListSave(int nCurJob)
{
	for (auto vect = AIImageSaveList[nCurJob].begin(); vect != AIImageSaveList[nCurJob].end(); vect++)
	{
		cv::imwrite(std::string(CT2A(vect->second.first)), (*vect->second.second->edge->_OrgImage.get()));
	}
	for (auto vect = AIOKImageList[nCurJob].begin(); vect != AIOKImageList[nCurJob].end(); vect++)
	{
		cv::imwrite(std::string(CT2A(vect->first)), (*vect->second->edge->_OrgImage.get()));
	}
}
void CPInsp_POCR::AIImageListSave_forced(int nCurJob)
{
	if (AIImageSaveList_Forced[nCurJob].size() > 0)
	{
		CString sFIlePath;
		CFileFind Finder;
		int fileExist = Finder.FindFile(m_sBoard);
		if (!fileExist)
			CreateDirectory(m_sBoard, NULL);
	}
	for (auto vect = AIImageSaveList_Forced[nCurJob].begin(); vect != AIImageSaveList_Forced[nCurJob].end(); vect++)
	{
		cv::imwrite(std::string(CT2A(vect->second.first)), (*vect->second.second->edge->_OrgImage.get()));
	}
}
void CPInsp_POCR::AIOKImageSave(int nCurJob, CString sModulID, CString sPartCode, CString sRefID, bool bOnlyErase)
{

	CString ssPartKey;
	ssPartKey.Format(_T("%s/%s/%s"),sModulID,sPartCode,sRefID);
	if(bOnlyErase && m_mInspParRst[nCurJob].count(ssPartKey) != 0)
		m_mInspParRst[nCurJob].erase(ssPartKey);

	if(AIImageSaveList[nCurJob].count(ssPartKey) == 0)
		return;
	if (!bOnlyErase | AISampleImageSaveOK)
		AIOKImageList[nCurJob][AIImageSaveList[nCurJob][ssPartKey].first] = AIImageSaveList[nCurJob][ssPartKey].second;
		
	if(AIImageSaveList[nCurJob].count(ssPartKey) != 0)
		AIImageSaveList[nCurJob].erase(ssPartKey);
}
void CPInsp_POCR::AIImageSave_Result(int nCurJob)
{
	for (auto vect = AIImageSaveList[nCurJob].begin(); vect != AIImageSaveList[nCurJob].end(); vect++)
	{
		if(vect->second.second)
			cv::imwrite(std::string(CT2A(vect->second.first)), (*vect->second.second->edge->_OrgImage.get()));
	}
	int m_nRandCnt = nAIOKSaveCount;
	if(AIOKImageList[nCurJob].size()<=m_nRandCnt)
	{
		for (auto vect = AIOKImageList[nCurJob].begin(); vect != AIOKImageList[nCurJob].end(); vect++)
		{
			if(vect->first == _T(""))
				continue;
			cv::imwrite(std::string(CT2A(vect->first)), (*vect->second->edge->_OrgImage.get()));
		}
	}
	else if(m_nRandCnt > 0)
	{
		float fPitch = (AIOKImageList[nCurJob].size()-1)/m_nRandCnt;
		std::srand(static_cast<unsigned int>(std::time(0)));
		for(int i=0;i<m_nRandCnt;i++)
		{
			float nRanNum = std::rand();
			int nSaveIdx = (nRanNum/RAND_MAX) * fPitch + (fPitch * i);
			auto Piar = AIOKImageList[nCurJob].begin();
			std::advance(Piar, nSaveIdx);
			cv::imwrite(std::string(CT2A(Piar->first)), (*Piar->second->edge->_OrgImage.get()));
		}
	}
}
int CPInsp_POCR::AIImageSavePart(int nCurJob, CString sPartCode, CString sRefId, bool bOnlyNGSave)
{
	try
	{
		bool isContained = false;
		if(bOnlyNGSave == false)
		{
			if(m_mSavePartList[nCurJob].size()>0)
			{
				if(m_mSavePartList[nCurJob].count(sRefId))
					isContained = true;
			}

			if(isContained == false && bOnlyNGSave == false)
			{
				m_mSavePartList[nCurJob][sRefId] = sPartCode;
			}
		}
		else
		{
			if(m_mNGSavePartList[nCurJob].size()>0)
			{
				if(m_mNGSavePartList[nCurJob].count(sRefId))
					isContained = true;
			}
			if(isContained == false && bOnlyNGSave)
			{
				m_mNGSavePartList[nCurJob][sRefId] = sPartCode;

			}
		}
	}
	catch(...)
	{
		g_pMPTI->AddLog(_T("AIImageSavePart error"));
	}
	return eMR_SUCCESS;
}
int CPInsp_POCR::AIImageSavePartListClear(int nCurJob)
{
	m_mSavePartList[nCurJob].clear();
	m_mNGSavePartList[nCurJob].clear();

	return eMR_SUCCESS;
}
void CPInsp_POCR::CopyAIPosition(RstAlgoPOCR& Srcrst,RstAlgoPOCR& DstRst)
{
	DstRst.ModelX_Ro = Srcrst.ModelX;
	DstRst.ModelY_Ro = Srcrst.ModelY;
	DstRst.ModelWidth_Ro = Srcrst.ModelWidth;
	DstRst.ModelHeight_Ro = Srcrst.ModelHeight;
	for (int i=0;i<MAX_STRLEN;i++)
	{
		DstRst.m_dArrCharScore_Ro[i] = Srcrst.m_dArrCharScore[i];
		DstRst.m_nCharScoreResult_Ro[i] = Srcrst.m_nCharScoreResult[i];
		DstRst.charX_Ro[i] = Srcrst.charX[i];
		DstRst.charY_Ro[i] = Srcrst.charY[i];
		DstRst.charWidth_Ro[i] = Srcrst.charWidth[i];
		DstRst.charHeight_Ro[i] = Srcrst.charHeight[i];
		DstRst.charAngle_Ro[i] = Srcrst.charAngle[i];
	}
}
void CPInsp_POCR::GetModelStr(int idx, SPOCRFontList* sStr)
{
	m_procPil->GetModelStr(idx,sStr);
}
bool CPInsp_POCR::ConvertFileVersion()
{
	return m_procPil->FontModel->ConvertFileVersion();
}
#endif