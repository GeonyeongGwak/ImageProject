#pragma once
#include "StdAfx.h"
#include "ProcPil_OCV.h"
#include "PsrFileManager.h"
#include "MPTI.h"
#include <math.h>

#define PAT_ACCURACY	M_HIGH//M_MEDIUM
#define PAT_SPEED		M_HIGH//M_MEDIUM 
#define  PAT_ATCSCORE		0.0

#ifdef _DEBUG
#define    _PROC_IMG_SAVE
#endif

#define BlidErrCode 1

using namespace std;

void Log_PilOcvModel(CString msg)
{
	g_pMPTI->AddLog_OCR(msg);
}
CProcPil_POCR::CProcPil_POCR(void)
{
	m_className = _T("CProcPil_POCR");

#if BlidErrCode
	m_ModelMng = std::shared_ptr<CMModelManager<ModelFile_POCR>>(new CMModelManager<ModelFile_POCR>(eAlgoPOCR, _T(".psr"),Log_PilOcvModel));
#endif
	m_rstPOCRInspResult = NULL;

	CreatePOCRInfo();
}

CProcPil_POCR::~CProcPil_POCR(void)
{
	if (m_rstPOCRInspResult != NULL)
		m_rstPOCRInspResult->Destroy();

	m_rstPOCRInspResult = NULL;
	m_POCRInfoParam->free();
}

int CProcPil_POCR::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	CProcMil::InitMil(milApp, milSys,bUseImagePilLib);


	return ePART_SUCCESS;
}

int CProcPil_POCR::FreeMil()
{
#ifndef _DISABLE_MIL	
	CProcMil :: FreeMil();
#endif

//	m_milPatResult.reset();


	return ePART_SUCCESS;
}

// void CProcPil_POCR::DeleteModelBuf()
// {
// 	m_milPatModel.reset();
// 	m_milPatModel_Rotate.reset();
// }

void CProcPil_POCR::CreatePOCRInfo()
{
	for (int i = 0; i < POCRFont::Num; i++)
	{
		m_POCRInfoParam[i].vPOCRInspPtr = g_pMManager->pem_new<POCRInspPtr>(true, MAX_POCR_PARAM_CNT, (PCHAR)__FUNCTION__, __LINE__);
		m_POCRInfoParam[i]._vPOCRInspPtr = g_pMManager->pem_new<cPOCRInspPtr>(true, MAX_POCR_PARAM_CNT, (PCHAR)__FUNCTION__, __LINE__);
		for (int j = 0; j < MAX_POCR_PARAM_CNT; j++)
		{
			m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr = g_pMManager->pem_new<POCRInspParam>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
			m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr = g_pMManager->pem_new<cPOCRInspParam>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		}

	}
}

void CProcPil_POCR::Setstr(CString str)
{
	targetFont.Format(str);
}

int CProcPil_POCR::FontRect(cv::Mat img,cv::Mat Grayimg,cv::Mat BinImage,int color,int threshValue, RECT* TeachRect,int nLength, std::vector<std::vector<cv::Point2f>> vPoint)
{
	int FontCnt = 0;
	try
	{
		CharClipper clip;
		if(nLength>255)
			nLength = 255;

		bool flag = false;

#if BlidErrCode
		if(vPoint.size() > 0&& vPoint[0].size() > 0)
			flag = clip.searchArea_OrgImage(nLength, img, Grayimg, BinImage, color, threshValue, vPoint);
		else
			flag = clip.searchArea_OrgImage_Old(nLength, img, Grayimg, BinImage, color, threshValue, vPoint);
		FontCnt = clip.GetRectArray(nLength, TeachRect);

#endif
		for(int i = 0; i < FontCnt; i++)
		{
			TeachRect[i].left  = TeachRect[i].left   + (Grayimg.cols/2);
			TeachRect[i].top   = TeachRect[i].top    + (Grayimg.rows/2);
			TeachRect[i].right = TeachRect[i].right  + (Grayimg.cols/2);
			TeachRect[i].bottom =TeachRect[i].bottom + (Grayimg.rows/2);
		}

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
	return FontCnt;
}
int CProcPil_POCR::FontRectAuto(cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue, RECT* TeachRect, int nLength)
{
	int FontCnt = 0;
	try
	{
		CharClipper clip;
		if (nLength > 255)
			nLength = 255;

		bool flag = clip.searchArea_OrgImage_AutoTeach(nLength, img, Grayimg, BinImage, color, threshValue);
		FontCnt = clip.GetRectArray(nLength, TeachRect);

		for (int i = 0; i < FontCnt; i++)
		{
			TeachRect[i].left = TeachRect[i].left + (Grayimg.cols / 2);
			TeachRect[i].top = TeachRect[i].top + (Grayimg.rows / 2);
			TeachRect[i].right = TeachRect[i].right + (Grayimg.cols / 2);
			TeachRect[i].bottom = TeachRect[i].bottom + (Grayimg.rows / 2);
		}
		if (!flag) FontCnt = 0;
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
	return FontCnt;
}

bool CProcPil_POCR::AddFont(cv::Mat img,cv::Mat Grayimg,cv::Mat BinImage,int color,int threshValue, RECT TeachRect, RECT* rcFontRect, HWND Handle)
{

	if(FontModel==nullptr)
		FontModel = std::make_shared<PsrModel>();

	int mListIndex = FontModel->nfontListCnt;
	if (rcFontRect)
		FontModel->SetPilParam(m_milApp, m_milSys);
	bool isCropSuc = FontModel->AddFont(targetFont,img ,Grayimg,BinImage,color,threshValue,0,m_POCRInfoParam, TeachRect, rcFontRect, Handle);

	if(!isCropSuc)
		return false;

	return true;
}

bool CProcPil_POCR::AddFont_NG(cv::Mat img,cv::Mat Grayimg,cv::Mat BinImage,int color, CString ImgPath,int nThreshold, RECT TeachRect)
{
	if(FontModel==nullptr)
		FontModel = std::make_shared<PsrModel>();

	int mListIndex = FontModel->nfontListCnt;

//	bool isCropSuc = FontModel->searchArea(strLength,&img,color,threshValue);
//	SearchPOCR(Grayimg.data,Grayimg.cols,Grayimg.rows,InspAlgoPOCR,false);

	bool isCropSuc = FontModel->AddFont_NG(img ,Grayimg,BinImage,color,0,m_POCRInfoParam,ImgPath, nThreshold, TeachRect);

	if(!isCropSuc)
		return false;

	return true;
}
//bool CProcPil_POCR::AddFont_ReTeaching(cv::Mat img, cv::Mat Grayimg, cv::Mat BinImage, int color, int threshValue, RECT TeachRect, RECT* FontRect)
//{
//	if (FontModel == nullptr)
//		FontModel = std::shared_ptr<PsrModel>(new PsrModel);
//
//	int mListIndex = FontModel->nfontListCnt;
//	FontModel->SetPilParam(m_milApp, m_milSys);
//	bool isCropSuc = FontModel->AddFont_ReTeaching(targetFont, img, Grayimg, BinImage, color, threshValue, 0, m_POCRInfoParam, TeachRect, FontRect);
//	return isCropSuc;
//}
bool CProcPil_POCR::SaveFontList(CString sPath,bool bAddMode)
{
	if(FontModel==nullptr)
		return false;

	try
	{
		bool flag = FontModel->Save(sPath,bAddMode,m_resolX,m_resolY);

		if(!flag)
			return false;
	}
	catch (CMemoryException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SaveFontList_path: %s : %s"),sPath,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SaveFontList_path: %s : %s"),sPath,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SaveFontList_path: %s : %s"),sPath,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch(...)
	{
		CString msg;
		msg.Format(_T("CProcPil_POCR_SaveFontList_path: %s ..."),sPath);
		g_pMPTI->AddLog_Dev(msg);
	}

	return true;
}

bool CProcPil_POCR::SetPOCRData(POCRInfoParam *pPOCRInfoParam)
{
	for (int i=0; i<POCRFont::Num; i++)
		m_POCRInfoParam[i] = pPOCRInfoParam[i];
	return true;
}


bool CProcPil_POCR::POCRInspTeach(POCRInspParam &sPOCRInspParam, int nImageCnt, CString ModePath, POCRFontFile *ImageFiles, CString ModelFont, int PsrListIndex)
{
	std::shared_ptr<PsrModel> fntMd= std::shared_ptr<PsrModel> (new PsrModel);
	fntMd->Load(ModePath,1,m_resolX,m_resolY);

	POCRInitResultVal(nImageCnt);

	for (int i = 0; i < nImageCnt; i++)
	{
		cv::Mat buf(1, ImageFiles[i].nFileSize, CV_8UC1, ImageFiles[i].m_FontData);
		cv::Mat userSrc = cv::imdecode(buf, cv::IMREAD_UNCHANGED);

		std::vector<std::shared_ptr<rstPSRModel>> rst = fntMd->SearchSingleFont(userSrc, ModelFont, PsrListIndex, sPOCRInspParam);

#ifdef _DEBUG
// 		CString msg;
// 		msg.Format(_T("D:\\testimage\\POCRInspTeach%d.bmp"),i);
// 		cv::imwrite(std::string(CT2A(msg)),userSrc);
#endif
		int rstsz = rst.size();

		if(rstsz == 0)
			return false;

		m_rstPOCRInspResult->vPOCRRstInspTeach[i].nRstScore = rst[0]->score;
		for (int j = 0; j < MAX_POCR_DIVISITION_CNTS; j++)
		{
			m_rstPOCRInspResult->vPOCRRstInspTeach[i].nDivisionScore[j] = rst[0]->div_score[j];
		}
	}

	return true;
}

int CProcPil_POCR::POCRInitResultVal(int nImageCnt)
{
	if (m_rstPOCRInspResult != NULL)
		m_rstPOCRInspResult->Destroy();

	m_rstPOCRInspResult = NULL;

	if (nImageCnt <= 0)
		return FALSE;

	//m_rstPOCRInspResult = new POCRInspResult();
	m_rstPOCRInspResult = g_pMManager->pem_new<POCRInspResult>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	memset(m_rstPOCRInspResult, 0, sizeof(POCRInspResult));

	m_rstPOCRInspResult->nInspCnt = nImageCnt;

	//m_rstPOCRInspResult->vPOCRRstInspTeach = new POCRRstInspTeach[nImageCnt];
	m_rstPOCRInspResult->vPOCRRstInspTeach = g_pMManager->pem_new<POCRRstInspTeach>(true, nImageCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(m_rstPOCRInspResult->vPOCRRstInspTeach, 0, (sizeof(POCRRstInspTeach) * nImageCnt));

	return TRUE;
}

bool CProcPil_POCR::POCRGetInspResult(POCRInspResult *rstPOCRInspResult)
{
	if (m_rstPOCRInspResult != NULL)
		*rstPOCRInspResult = *m_rstPOCRInspResult;
	else
		rstPOCRInspResult = NULL;

	return true;
}

bool CProcPil_POCR::LoadFontList(CString sPath,int fontColor, CString readfonts, CString destPath,bool bOrg)
{
	try
	{
		FontModel = std::make_shared<PsrModel>();
		bool flag = FontModel->Load(sPath, fontColor, m_resolX,m_resolY,readfonts, destPath,bOrg);

		if(!flag)
			return false;
	}
	catch (CMemoryException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_LoadFontList_path: %s : %s"),sPath,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_LoadFontList_path: %s : %s"),sPath,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_LoadFontList_path: %s : %s"),sPath,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch(...)
	{
		CString msg;
		msg.Format(_T("CProcPil_POCR_LoadFontList_path: %s ..."),sPath);
		g_pMPTI->AddLog_Dev(msg);
	}

	return true;
}

bool CProcPil_POCR::DeleteFont(CString ch,int index)
{
	if(FontModel==nullptr)
		return true;

	try
	{
		if(!ch.IsEmpty())
			FontModel->Load(ch, m_resolX,m_resolY);

		bool flag = FontModel->deleteFontList(index);

		if(!flag)
			return false;
	}
	catch (CMemoryException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_DeleteFont_ch: %s : %s"),ch,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_DeleteFont_ch: %s : %s"),ch,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_DeleteFont_ch: %s : %s"),ch,szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch(...)
	{
		CString msg;
		msg.Format(_T("CProcPil_POCR_DeleteFont_ch: %s ..."),ch);
		g_pMPTI->AddLog_Dev(msg);
	}

	return true;
}

bool CProcPil_POCR::SearchPOCR(AlgoPOCR *InspAlgoPOCR)
{
	if(FontModel==nullptr)
		return false;

	bool re = e_NG;
	try
	{
		Cand_POCRresult.clear();
		POCRresult.reset();

		algoPOCR_ParamContainer param(InspAlgoPOCR);
		//		bool flag = FontModel.deleteFont(ch,index);


//#if ImageSave
// 
// 		cv::Mat srcImg(InspImgheight, InspImgwidth, CV_8UC1);
// 		for (int y=0; y<InspImgheight; y++)
// 			memcpy(srcImg.ptr(y), &InspImg[y*InspImgwidth], InspImgwidth);
//#endif
		//std::vector<std::shared_ptr<rstPSRModel>> rst = FontModel->SearchFont(ImgProc, InspAlgoPOCR->m_sTargetFont, m_POCRInfoParam,InspAlgoPOCR->m_sSaveParam,InspAlgoPOCR->m_dStdCharScore);
		std::vector<std::shared_ptr<rstPSRModel>> rst;
		if(InspAlgoPOCR->m_bIAllMatch)
			rst = FontModel->MatchingFont(ImgProc, InspAlgoPOCR->m_sTargetFont, m_POCRInfoParam, InspAlgoPOCR->m_sSaveParam, InspAlgoPOCR->m_dStdCharScore, InspAlgoPOCR->m_nClrForeGround, &param);
		else
			rst = FontModel->SearchFont(ImgProc, InspAlgoPOCR->m_sTargetFont, m_POCRInfoParam, InspAlgoPOCR->m_sSaveParam, InspAlgoPOCR->m_dStdCharScore, InspAlgoPOCR->m_nClrForeGround, &param);

		int rstsz = rst.size();

		for(int fontIdx = 0;fontIdx<InspAlgoPOCR->m_nCharMaxCount;fontIdx++)	
		{
			POCRresult.m_cArrStr[fontIdx] = '?';
			POCRresult.charAngle[fontIdx] = 0;
			POCRresult.charX[fontIdx] = 0;
			POCRresult.charY[fontIdx] = 0;
			POCRresult.charWidth[fontIdx] = 0;
			POCRresult.charHeight[fontIdx] = 0;
			POCRresult.m_dArrCharScore[fontIdx] = 0;
			POCRresult.dContrastScore[fontIdx] = 0;
			POCRresult.dUPScore[fontIdx] = 0;
			POCRresult.dOPScore[fontIdx] = 0;
			POCRresult.dBlobSizeValue[fontIdx] = 0;
		}
		POCRresult.m_nCharCount = InspAlgoPOCR->m_nCharMaxCount;

		if(rstsz == 0)
			return false;
		arr.clear();

		m_AlgoPOCR = *InspAlgoPOCR;
		arr.resize(InspAlgoPOCR->m_nCharMaxCount);
		if (InspAlgoPOCR->m_nCharMaxCount < 5 && !InspAlgoPOCR->m_bIAllMatch)
		{

			for (int fontIdx = 0; fontIdx < InspAlgoPOCR->m_nCharMaxCount; fontIdx++)		//2017.05.11 shkim // 문자열로 분류
			{

				for (int i = 0; i < rstsz; i++)
				{
					if (rst[i]->ch == InspAlgoPOCR->m_sTargetFont[fontIdx])
					{
						arr[fontIdx].push_back(rst[i]);
					}
				}
			}

			// 		// YJS 추가 시작 : 추가된 폰트과 좌표가 너무 같은 폰트가 있으면 		
			// 		for(int i=0;i<rstsz;i++)
			// 		{
			// 			for(int fontIdx = 0;fontIdx<InspAlgoPOCR->m_nCharMaxCount;fontIdx++)		//2017.05.11 shkim // 문자열로 분류
			// 			{
			// 				if(rst[i]->ch == InspAlgoPOCR->m_sTargetFont[fontIdx])
			// 					continue;
			// 
			// 				for(int k=0; k<arr[fontIdx].size(); k++)
			// 				{
			// 					if( fabs(rst[i]->x - arr[fontIdx][k]->x) <= 2.0 &&
			// 						fabs(rst[i]->y - arr[fontIdx][k]->y) <= 2.0 )
			// 					{
			// 						arr[fontIdx].push_back(rst[i]);
			// 						break;
			// 					}
			// 				}
			// 			}
			// 		}
					// YJS 추가 끝

					//2021.09.28 shkim remove by position
			for (int fontIdx = 0; fontIdx + 1 < InspAlgoPOCR->m_nCharMaxCount; fontIdx++)
			{
				if (arr[fontIdx].size() > 0)//for (int j = 0; j < arr[fontIdx].size(); j++)
				{
					int j = 0;
					bool bRemove = false;
					int nRemIdx = j;
					int nRemFontIdx = 0;
					for (int k = 0; k < arr[fontIdx + 1].size(); k++)
					{
						bRemove = true;

						int minOff = (arr[fontIdx][j]->nCharWidth / 2 + arr[fontIdx + 1][k]->nCharWidth / 2)*0.8;
						int curDiff = arr[fontIdx + 1][k]->x - arr[fontIdx][j]->x;
						if (curDiff < 0)
						{
							for (int nIdx = fontIdx; nIdx > 0; nIdx--)
							{
								for (int z = 0; z < arr[nIdx].size(); z++)
									if (arr[fontIdx + 1][k]->x == arr[nIdx][z]->x)//shkim 2022.04.05 앞에있는 문자가 있는지 확인.
									{
										nRemFontIdx++;
										nRemIdx = k;
										break;
									}
								if (nRemFontIdx != 0)
								{
									break;
								}
							}
							if (nRemFontIdx != 0)
							{
								break;
							}
						}
						else if (minOff < curDiff || arr[fontIdx + 1][k]->ch != arr[fontIdx][j]->ch)
						{
							bRemove = false;
							break;
						}
					}
					if (bRemove)
					{
						arr[fontIdx + nRemFontIdx].erase(arr[fontIdx + nRemFontIdx].begin() + nRemIdx);
					}
				}
			}

			for (int fontIdx = 0; fontIdx < InspAlgoPOCR->m_nCharMaxCount; fontIdx++)		//2017.05.11 shkim // 점수기준 내림차순 정렬
			{
				int cnt = arr[fontIdx].size();
				for (int i = 0; i < cnt; i++)
				{
					for (int j = 0; j < cnt; j++)
					{
						if (arr[fontIdx][i]->score > arr[fontIdx][j]->score)
						{
							std::shared_ptr<rstPSRModel> tmp = arr[fontIdx][i];
							arr[fontIdx][i] = arr[fontIdx][j];
							arr[fontIdx][j] = tmp;
						}
					}
				}

			}

			bool SearchOK = true;

			for (int fontIdx = 0; fontIdx < InspAlgoPOCR->m_nCharMaxCount; fontIdx++)
			{
				if (arr[fontIdx].size() <= 0)
					SearchOK = false;
			}
			if (SearchOK)
			{
				if (InspAlgoPOCR->m_nCharMaxCount > 1)
				{
					for (int nRankIdx = 0; nRankIdx < arr[0].size(); nRankIdx++)
					{
						if (Decision(0, InspAlgoPOCR, arr[0][nRankIdx]))
						{
							Cand_POCRresult.push_back(POCRresult);
						}
					}
				}
				else
				{
					SingleDecision();
				}
			}
			else
			{
				Division_fail();
			}
		}
		else
		{

			for (int i = 0; i < rstsz; i++)
			{
				int fontIdx = i % InspAlgoPOCR->m_nCharMaxCount;
				if (rst[i]->ch == InspAlgoPOCR->m_sTargetFont[fontIdx])
				{
					arr[fontIdx].push_back(rst[i]);
				}
			}

			for (int fontIdx = 0; fontIdx < InspAlgoPOCR->m_nCharMaxCount; fontIdx++)		//2017.05.11 shkim // 점수기준 내림차순 정렬
			{
				int cnt = arr[fontIdx].size();
				for (int i = 0; i < cnt; i++)
				{
					for (int j = 0; j < cnt; j++)
					{
						if (arr[fontIdx][i]->score > arr[fontIdx][j]->score)
						{
							std::shared_ptr<rstPSRModel> tmp = arr[fontIdx][i];
							arr[fontIdx][i] = arr[fontIdx][j];
							arr[fontIdx][j] = tmp;
						}
					}
				}

			}

			bool SearchOK = true;

			for (int fontIdx = 0; fontIdx < InspAlgoPOCR->m_nCharMaxCount; fontIdx++)
			{
				if (arr[fontIdx].size() <= 0)
					SearchOK = false;
			}
			//if (SearchOK)
			{
				if (InspAlgoPOCR->m_nCharMaxCount > 1)
				{
					Decision(InspAlgoPOCR);
				}
				else
				{
					SingleDecision();
				}
			}
			//else
			//{
			//	Division_fail();
			//}
		}

		re = Confirm(InspAlgoPOCR);
// 
// 		CString sSaveParam;
// 		sSaveParam.Format(_T("%s"),InspAlgoPOCR->m_sSaveParam);
// 
// 		if(rotate)
// 		{
// 			POCRresult.ModelX = InspImgwidth - POCRresult.ModelX;
// 			POCRresult.ModelY = InspImgheight - POCRresult.ModelY;
// 
// 			for(int i=0;i<POCRresult.m_nCharCount;i++)
// 			{
// 				POCRresult.charX[i] = InspImgwidth - POCRresult.charX[i];
// 				POCRresult.charY[i] = InspImgheight - POCRresult.charY[i];
// 			}
// 		}
// 
// 		if(sSaveParam.IsEmpty() ==false)
// 		{
// 			if(rotate)
// 			{
// 				Angle-=180;
// 
// 				if(Angle<0)
// 					Angle = Angle + 360;
// 
// 				RotateImg_ipp(userSrc, width, height, Angle, &InspImg, &InspImgwidth, &InspImgheight);
// 
// 			}
// 
// 			cv::Mat srcImg(InspImgheight, InspImgwidth, CV_8UC1);
// 			for (int y=0; y<InspImgheight; y++)
// 				memcpy(srcImg.ptr(y), &InspImg[y*InspImgwidth], InspImgwidth);
// 			cv::Rect MoRect;
// 
// 			MoRect.x = POCRresult.ModelX-POCRresult.ModelWidth/2;
// 			MoRect.y = POCRresult.ModelY-POCRresult.ModelHeight/2;
// 			MoRect.width=POCRresult.ModelWidth;
// 			MoRect.height=POCRresult.ModelHeight;
// 
// 			cv::rectangle(srcImg,MoRect,cv::Scalar(255));
// 
// 			for(int i=0;i<POCRresult.m_nCharCount;i++)
// 			{
// 				cv::Rect re;
// 
// 				re.x = POCRresult.charX[i]-POCRresult.charWidth[i]/2;
// 				re.y = POCRresult.charY[i]-POCRresult.charHeight[i]/2;
// 				re.width=POCRresult.charWidth[i];
// 				re.height=POCRresult.charHeight[i];
// 
// 				cv::rectangle(srcImg,re,cv::Scalar(255-50));
// 
// 				CString msg;
// 				msg.Format(_T("ch: %c  , score:%.2f , rectangle[%d] x:%d, y:%d, w:%d, h:%d"),POCRresult.m_cArrStr[i],POCRresult.m_dStringScore,i,re.x,re.y,re.width,re.height);
// 				g_pMPTI->AddLog_Dev(msg);
// 				for(int j=0;j<MAX_POCR_DIVISITION_CNTS;j++)
// 				{
// 					msg.Format(_T("divScore[%d][%d]: %.2f"),i,j,POCRresult.div_score[i][j]);
// 					g_pMPTI->AddLog_Dev(msg);
// 				}
// 			}
// 
// 			cv::imwrite("D:\\testimage\\workimage\\POCRResult.bmp",srcImg);
// 		}

		arr.clear();
	}
	catch (CMemoryException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"),szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"),szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"),szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch(...)
	{
		CString msg;
		msg.Format(_T("CProcPil_POCR_SearchPOCR ..."));
		g_pMPTI->AddLog_Dev(msg);
	}

	return re;
}
// 
// bool CProcPil_POCR::Confirm(std::vector<std::shared_ptr<rstPSRModel>> rst,AlgoPOCR *InspAlgoPOCR)
// {
// 	bool flag = true;
// 
// 	int rstsz = rst.size();
// 	
// 	POCRresult.m_nCharCount = rstsz;
// 
// 	for(int i=0;i<rstsz;i++)
// 	{
// 		if(POCRresult.m_dStringScore>rst[i]->score)
// 		{
// 			POCRresult.m_dStringScore = rst[i]->score;
// 		}
// 		POCRresult.charAngle[i] = rst[i]->angle;
// 		POCRresult.charX[i] = rst[i]->x;
// 		POCRresult.charY[i] = rst[i]->y;
// 		POCRresult.m_dArrCharScore[i] = rst[i]->score;
// 		POCRresult.m_cArrStr[i] = rst[i]->ch;
// 	}
// 	return flag;
// }

double GapY = 30;

bool CProcPil_POCR::Decision(int fontIndex,AlgoPOCR *InspAlgoPOCR,std::shared_ptr<rstPSRModel> font)
{
	bool flag=true;
	bool result = false;
	if (arr.size() <= fontIndex)
		return false;
	int arSize = arr[fontIndex].size();
//	GapY = tan(5*3.14159265/180.0);
	for(int i=0;i<arSize;i++)
	{
		if(fontIndex+1<InspAlgoPOCR->m_nCharMaxCount)
		{
			flag = Decision(fontIndex+1,InspAlgoPOCR,arr[fontIndex][i]);
		}

		//2017.05.11 shkim
		if(flag)
		{
			int minOff = (int)((std::round(font->nCharWidth / 2) + std::round(arr[fontIndex][i]->nCharWidth / 2))*0.65);
			int curDiff = arr[fontIndex][i]->x - font->x;
			if (minOff < curDiff || fontIndex == 0)
			{
				if (abs(font->y - arr[fontIndex][i]->y) <= arr[fontIndex][i]->nCharHeight)
				{
					POCRresult.charAngle[fontIndex] = arr[fontIndex][i]->angle;
					POCRresult.charX[fontIndex] = arr[fontIndex][i]->x;
					POCRresult.charY[fontIndex] = arr[fontIndex][i]->y;
					POCRresult.charWidth[fontIndex] = arr[fontIndex][i]->nCharWidth;
					POCRresult.charHeight[fontIndex] = arr[fontIndex][i]->nCharHeight;
					POCRresult.m_dArrCharScore[fontIndex] = arr[fontIndex][i]->score;
					POCRresult.m_cArrStr[fontIndex] = arr[fontIndex][i]->ch;
					//POCRresult.dRstShiftX = arr[fontIndex][i]->ShiftX;
					//POCRresult.dRstShiftX = arr[fontIndex][i]->ShiftY;
					POCRresult.dContrastScore[fontIndex] = arr[fontIndex][i]->dContrastScore;
					POCRresult.dUPScore[fontIndex] = arr[fontIndex][i]->dUPScore;
					POCRresult.dOPScore[fontIndex] = arr[fontIndex][i]->dOPScore;
					POCRresult.dBlobSizeValue[fontIndex] = arr[fontIndex][i]->dBlobSizeValue;
					// 					for (int j=0; j<MAX_POCR_DIVISITION_CNTS; j++)
					// 						POCRresult.div_score[fontIndex][j] = arr[fontIndex][i]->div_score[j];

					result = true;
				}
				else
				{
					result = false;
				}
			}
		}
		if(result == true)
		{
			break;
		}
	}
	if(result == false)
	{
		flag =false;
	}

	return flag;
}

bool CProcPil_POCR::Decision(AlgoPOCR *InspAlgoPOCR)
{
	bool flag = false;
	int nStartIdx = 0;
	if (arr.size() > 0)
	{
		for (int nstIdx = 0; nstIdx < arr.size(); nstIdx++)
			if (arr[nstIdx].size() > 0)
			{
				nStartIdx = nstIdx;
				break;
			}
		if (arr[nStartIdx].size() > 0)
		{
			if (POCRresult.m_dStringScore > arr[nStartIdx][0]->score)
		{
				POCRresult.m_dStringScore = arr[nStartIdx][0]->score;
		}
		POCRresult.m_bOKString = true;
			POCRresult.charAngle[nStartIdx] = arr[nStartIdx][0]->angle;
			POCRresult.charX[nStartIdx] = arr[nStartIdx][0]->x;
			POCRresult.charY[nStartIdx] = arr[nStartIdx][0]->y;
			POCRresult.charWidth[nStartIdx] = arr[nStartIdx][0]->nCharWidth;
			POCRresult.charHeight[nStartIdx] = arr[nStartIdx][0]->nCharHeight;
			POCRresult.m_dArrCharScore[nStartIdx] = arr[nStartIdx][0]->score;
			POCRresult.m_cArrStr[nStartIdx] = arr[nStartIdx][0]->ch;
		// 		POCRresult.dRstShiftX = arr[0][0]->ShiftX;
		// 		POCRresult.dRstShiftY = arr[0][0]->ShiftY;
		bool bDecFail = true;
			for (int nFont = 0; nFont < arr[nStartIdx].size() && bDecFail; nFont++)
		{
				POCRresult.charAngle[nStartIdx] = arr[nStartIdx][nFont]->angle;
				POCRresult.charX[nStartIdx] = arr[nStartIdx][nFont]->x;
				POCRresult.charY[nStartIdx] = arr[nStartIdx][nFont]->y;
				POCRresult.charWidth[nStartIdx] = arr[nStartIdx][nFont]->nCharWidth;
				POCRresult.charHeight[nStartIdx] = arr[nStartIdx][nFont]->nCharHeight;
				POCRresult.m_dArrCharScore[nStartIdx] = arr[nStartIdx][nFont]->score;
				POCRresult.m_cArrStr[nStartIdx] = arr[nStartIdx][nFont]->ch;
				POCRresult.dContrastScore[nStartIdx] = arr[nStartIdx][nFont]->dContrastScore;
				POCRresult.dUPScore[nStartIdx] = arr[nStartIdx][nFont]->dUPScore;
				POCRresult.dOPScore[nStartIdx] = arr[nStartIdx][nFont]->dOPScore;
				POCRresult.dBlobSizeValue[nStartIdx] = arr[nStartIdx][nFont]->dBlobSizeValue;
				for (int i = nStartIdx + 1; i < InspAlgoPOCR->m_nCharMaxCount; i++)
			{
				bool result = false;
				for (int j = 0; j < arr[i].size(); j++)
				{
					int minOff = (int)((std::round(arr[i][j]->nCharWidth / 2) + std::round(POCRresult.charWidth[i - 1] / 2))*0.65);
					int curDiff = arr[i][j]->x - POCRresult.charX[i - 1];
					if (minOff < curDiff && abs(POCRresult.charY[i - 1] - arr[i][j]->y) <= arr[i][j]->nCharHeight)
					{
						POCRresult.charAngle[i] = arr[i][j]->angle;
						POCRresult.charX[i] = arr[i][j]->x;
						POCRresult.charY[i] = arr[i][j]->y;
							POCRresult.charWidth[i] = arr[i][j]->nCharWidth;
							POCRresult.charHeight[i] = arr[i][j]->nCharHeight;
							POCRresult.m_dArrCharScore[i] = arr[i][j]->score;
							POCRresult.m_cArrStr[i] = arr[i][j]->ch;
							POCRresult.dContrastScore[i] = arr[i][j]->dContrastScore;
							POCRresult.dUPScore[i] = arr[i][j]->dUPScore;
							POCRresult.dOPScore[i] = arr[i][j]->dOPScore;
							POCRresult.dBlobSizeValue[i] = arr[i][j]->dBlobSizeValue;
							//POCRresult.dRstShiftX = arr[fontIndex][i]->ShiftX;
						//POCRresult.dRstShiftX = arr[fontIndex][i]->ShiftY;
						// 					for (int j=0; j<MAX_POCR_DIVISITION_CNTS; j++)
						// 						POCRresult.div_score[fontIndex][j] = arr[fontIndex][i]->div_score[j];
						result = true;
						break;
					}
				}
				bDecFail &= result;
			}
			bDecFail != bDecFail;
				Cand_POCRresult.push_back(POCRresult);
		}
		flag != bDecFail;
		}
	}
	else
	{
		for (int i = 0; i < InspAlgoPOCR->m_nCharMaxCount; i++)
		{
			POCRresult.m_dStringScore = 0;
			POCRresult.charAngle[i] = 0;
			POCRresult.charX[i] = 0;
			POCRresult.charY[i] = 0;
			POCRresult.charWidth[i] = 0;
			POCRresult.charHeight[i] = 0;
			POCRresult.m_dArrCharScore[i] = 0;
			POCRresult.m_nCharScoreResult[i] = 1;
			POCRresult.m_cArrStr[i] = '?';
			POCRresult.dRstShiftX = 0;
			POCRresult.dRstShiftY = 0;
			POCRresult.dContrastScore[i] = 0;
			POCRresult.dUPScore[i] = 0;
			POCRresult.dOPScore[i] = 0;
			POCRresult.dBlobSizeValue[i] = 0;
		}
		Cand_POCRresult.push_back(POCRresult);
	}

	return flag;
}

bool CProcPil_POCR::SingleDecision()
{
	bool flag = false;
	if(arr.size()>0&&arr[0].size()>0)
	{
		if(POCRresult.m_dStringScore>arr[0][0]->score)
		{
			POCRresult.m_dStringScore = arr[0][0]->score;
		}
		POCRresult.m_bOKString = true;
		POCRresult.charAngle[0] = arr[0][0]->angle;
		POCRresult.charX[0] = arr[0][0]->x;
		POCRresult.charY[0] = arr[0][0]->y;
		POCRresult.charWidth[0] = arr[0][0]->nCharWidth;
		POCRresult.charHeight[0] = arr[0][0]->nCharHeight;
		POCRresult.m_dArrCharScore[0] = arr[0][0]->score;
		POCRresult.m_cArrStr[0] = arr[0][0]->ch;
// 		POCRresult.dRstShiftX = arr[0][0]->ShiftX;
// 		POCRresult.dRstShiftY = arr[0][0]->ShiftY;
		POCRresult.dContrastScore[0] = arr[0][0]->dContrastScore;
		POCRresult.dUPScore[0] = arr[0][0]->dUPScore;
		POCRresult.dOPScore[0] = arr[0][0]->dOPScore;
		POCRresult.dBlobSizeValue[0] = arr[0][0]->dBlobSizeValue;
		flag = true;
	}
	else 
	{
		POCRresult.m_dStringScore = 0;
		POCRresult.charAngle[0] = 0;
		POCRresult.charX[0] = 0;
		POCRresult.charY[0] = 0;
		POCRresult.charWidth[0] = 0;
		POCRresult.charHeight[0] = 0;
		POCRresult.m_dArrCharScore[0] = 0;
		POCRresult.m_nCharScoreResult[0] = 1;
		POCRresult.m_cArrStr[0] = '?';
		POCRresult.dRstShiftX = 0;
		POCRresult.dRstShiftY = 0;
		POCRresult.dContrastScore[0] = 0;
		POCRresult.dUPScore[0] = 0;
		POCRresult.dOPScore[0] = 0;
		POCRresult.dBlobSizeValue[0] = 0;
	}
	Cand_POCRresult.push_back(POCRresult);

	return flag;
}

bool CProcPil_POCR::Division_fail()
{
	bool flag = false;

	if(arr.size()>0 && arr[0].size()>0)
	{
		POCRresult.ModelX=arr[0][0]->Parent_x;
		POCRresult.ModelY=arr[0][0]->Parent_y;
		POCRresult.ModelWidth=arr[0][0]->Parent_Width;
		POCRresult.ModelHeight=arr[0][0]->Parent_Height;

		for(int fontIdx = 0;fontIdx<arr.size();fontIdx++)	
		{
			if(arr[fontIdx].size()<=0)
				continue;
			POCRresult.charAngle[fontIdx] = arr[fontIdx][0]->angle;
			POCRresult.charX[fontIdx] = arr[fontIdx][0]->x;
			POCRresult.charY[fontIdx] = arr[fontIdx][0]->y;
			POCRresult.charWidth[fontIdx] = arr[fontIdx][0]->nCharWidth;
			POCRresult.charHeight[fontIdx] = arr[fontIdx][0]->nCharHeight;
			POCRresult.m_dArrCharScore[fontIdx] = arr[fontIdx][0]->score;
			POCRresult.m_cArrStr[fontIdx] = arr[fontIdx][0]->ch;
			POCRresult.dContrastScore[fontIdx] = arr[fontIdx][0]->dContrastScore;
			POCRresult.dUPScore[fontIdx] = arr[fontIdx][0]->dUPScore;
			POCRresult.dOPScore[fontIdx] = arr[fontIdx][0]->dOPScore;
			POCRresult.dBlobSizeValue[fontIdx] = arr[fontIdx][0]->dBlobSizeValue;
			flag = true;
		}
// 		POCRresult.dRstShiftX = arr[0][0]->ShiftX;
// 		POCRresult.dRstShiftY = arr[0][0]->ShiftY;
	}
	else 
	{
		for(int fontIdx = 0;fontIdx<arr.size();fontIdx++)	
		{
			if(arr[fontIdx].size()<=0)
				continue;

			POCRresult.ModelX=0;
			POCRresult.ModelY=0;
			POCRresult.ModelWidth=0;
			POCRresult.ModelHeight=0;

			POCRresult.m_dStringScore = 0;
			POCRresult.charAngle[fontIdx] = 0;
			POCRresult.charX[fontIdx] = 0;
			POCRresult.charY[fontIdx] = 0;
			POCRresult.charWidth[fontIdx] = 0;
			POCRresult.charHeight[fontIdx] = 0;
			POCRresult.m_dArrCharScore[fontIdx] = 0;
			POCRresult.m_nCharScoreResult[fontIdx] = 1;
			POCRresult.m_cArrStr[fontIdx] = '?';
			POCRresult.dContrastScore[fontIdx] = 0;
			POCRresult.dUPScore[fontIdx] = 0;
			POCRresult.dOPScore[fontIdx] = 0;
			POCRresult.dBlobSizeValue[fontIdx] = 0;
			flag = true;
			POCRresult.dRstShiftX = 0;
			POCRresult.dRstShiftY = 0;
		}
	}

	return flag;
}

bool CProcPil_POCR::Confirm(AlgoPOCR *InspAlgoPOCR)
{
	bool flag = false;



	for(int canIdx=0; canIdx < Cand_POCRresult.size();canIdx++)
	{
		for(int i=0;i<Cand_POCRresult[canIdx].m_nCharCount;i++)
		{
			if(Cand_POCRresult[canIdx].m_dArrCharScore[i]<0)
			{
				Cand_POCRresult[canIdx].m_dArrCharScore[i] = 0;
			}
			if(Cand_POCRresult[canIdx].m_dStringScore>Cand_POCRresult[canIdx].m_dArrCharScore[i])
			{
				Cand_POCRresult[canIdx].m_dStringScore = Cand_POCRresult[canIdx].m_dArrCharScore[i];
			}
			if(Cand_POCRresult[canIdx].m_dArrCharScore[i]<m_AlgoPOCR.m_dStdCharScore[i])
			{
				Cand_POCRresult[canIdx].m_nCharScoreResult[i] = 1;
				Cand_POCRresult[canIdx].m_bOKScore = FALSE;
			}
			if(Cand_POCRresult[canIdx].m_cArrStr[i]!=m_AlgoPOCR.m_sTargetFont[i])
			{
				Cand_POCRresult[canIdx].m_bOKString = FALSE;
			}
		}
		if (Cand_POCRresult[canIdx].m_dStringScore < 0)
			Cand_POCRresult[canIdx].m_dStringScore = 0;
	}


	for(int i=0; i < Cand_POCRresult.size();i++)		//2017.06.19 shkim // 점수기준 내림차순 정렬
	{
		for(int j=0;j<Cand_POCRresult.size();j++)
		{
			if(Cand_POCRresult[i].m_dStringScore>Cand_POCRresult[j].m_dStringScore)
			{
				RstAlgoPOCR tmp = Cand_POCRresult[i];
				Cand_POCRresult[i] = Cand_POCRresult[j];
				Cand_POCRresult[j] = tmp;
			}
		}

	}
	bool bisAdditionalCheckOK = true;
	bool bisFontAngleOK = true;
	if(Cand_POCRresult.size()>0)
	{

		POCRresult = Cand_POCRresult[0];

		//if(arr.size()>0)
		for (int i = 0; i < arr.size(); i++)
		{
			if (arr[i].size() < 1)
				continue;
			POCRresult.ModelX = arr[i][0]->Parent_x;
			POCRresult.ModelY = arr[i][0]->Parent_y;
			POCRresult.ModelWidth = arr[i][0]->Parent_Width;
			POCRresult.ModelHeight = arr[i][0]->Parent_Height;
// 			POCRresult.dRstShiftX = arr[0][0]->ShiftX * m_resolX;
// 			POCRresult.dRstShiftY = arr[0][0]->ShiftY * m_resolY;
// 			if (InspAlgoPOCR->m_bUseShift)
// 			{
// 				if (InspAlgoPOCR->m_fShiftX <= std::abs(POCRresult.dRstShiftX) )
// 					POCRresult.bRstShiftX = FALSE;
// 				if (InspAlgoPOCR->m_fShiftY <= std::abs(POCRresult.dRstShiftY))
// 					POCRresult.bRstShiftY = FALSE;
// 			}
			if (InspAlgoPOCR->m_bUseBlobSize)
			{
				for (int fontIdx = 0; fontIdx < POCRresult.m_nCharCount; fontIdx++)
				{
					if (InspAlgoPOCR->m_dBlobSize < POCRresult.dBlobSizeValue[fontIdx])
						bisAdditionalCheckOK = POCRresult.bBlobSizeOK[fontIdx] = FALSE;
					else
						POCRresult.bBlobSizeOK[fontIdx] = TRUE;
				}
			}
			if (InspAlgoPOCR->m_bUseUP)
			{
				for (int fontIdx = 0; fontIdx < POCRresult.m_nCharCount; fontIdx++)
				{
					if (InspAlgoPOCR->m_dUP > POCRresult.dUPScore[fontIdx])
						bisAdditionalCheckOK = POCRresult.bUPOK[fontIdx] = FALSE;
					else
						POCRresult.bUPOK[fontIdx] = TRUE;
				}
			}
			if (InspAlgoPOCR->m_bUseOP)
			{
				for (int fontIdx = 0; fontIdx < POCRresult.m_nCharCount; fontIdx++)
				{
					if (InspAlgoPOCR->m_dOP > POCRresult.dOPScore[fontIdx])
						bisAdditionalCheckOK = POCRresult.bOPOK[fontIdx] = FALSE;
					else
						POCRresult.bOPOK[fontIdx] = TRUE;
				}
			}
			if (InspAlgoPOCR->m_bUseContrast)
			{
				for (int fontIdx = 0; fontIdx < POCRresult.m_nCharCount; fontIdx++)
				{
					if (InspAlgoPOCR->m_dContrast < POCRresult.dContrastScore[fontIdx])
						bisAdditionalCheckOK = POCRresult.bContrastOK[fontIdx] = FALSE;
					else
						POCRresult.bContrastOK[fontIdx] = TRUE;
				}
			}
			if (InspAlgoPOCR->m_bUseFontAngle)
			{
				for (int fontIdx = 0; fontIdx < POCRresult.m_nCharCount; fontIdx++)
				{
					if (InspAlgoPOCR->m_fStdFontAngle < POCRresult.charAngle[fontIdx])
						bisFontAngleOK = POCRresult.bFontAngleOK[fontIdx] = FALSE;
					else
						POCRresult.bFontAngleOK[fontIdx] = TRUE;
				}
			}
			break;

		}
	}
	else
	{
		POCRresult.ModelX=0;
		POCRresult.ModelY=0;
		POCRresult.ModelWidth=0;
		POCRresult.ModelHeight=0;
		POCRresult.dRstShiftX = 0;
		POCRresult.dRstShiftY = 0;

		for(int fontIdx = 0;fontIdx<POCRresult.m_nCharCount;fontIdx++)	
		{
			POCRresult.m_dStringScore = 0;
			POCRresult.charAngle[fontIdx] = 0;
			POCRresult.charX[fontIdx] = 0;
			POCRresult.charY[fontIdx] = 0;
			POCRresult.m_dArrCharScore[fontIdx] = 0;
			POCRresult.m_nCharScoreResult[fontIdx] = 1;
			POCRresult.m_cArrStr[fontIdx] = '?';
			POCRresult.dContrastScore[fontIdx] = 0;
			POCRresult.dUPScore[fontIdx] = 0;
			POCRresult.dOPScore[fontIdx] = 0;
			POCRresult.dBlobSizeValue[fontIdx] = 0;

			if(arr.size()>fontIdx && arr[fontIdx].size()>0)
			{
				POCRresult.charX[fontIdx] = arr[fontIdx][0]->x;
				POCRresult.charY[fontIdx] = arr[fontIdx][0]->y;
				POCRresult.charWidth[fontIdx] = arr[fontIdx][0]->nCharWidth;
				POCRresult.charHeight[fontIdx] = arr[fontIdx][0]->nCharHeight;
				POCRresult.charAngle[fontIdx] = arr[fontIdx][0]->angle;
			}

		}
		POCRresult.m_bOKScore = FALSE;
		POCRresult.m_bOKString = FALSE;
		POCRresult.bRstShiftX = POCRresult.bRstShiftY = FALSE;
	}
	if(POCRresult.m_bOKString && POCRresult.m_bOKScore && POCRresult.bRstShiftX&& POCRresult.bRstShiftY && bisAdditionalCheckOK && bisFontAngleOK)
		flag=true;
	else
		flag = false;

	return flag;
}

bool CProcPil_POCR::ClearFont()
{
	if(FontModel==nullptr)
		return true;
	FontModel.reset();

	return true;
}

int CProcPil_POCR::ModelList_Load(CString strPath)
{

	int ret = ePART_SUCCESS;
	if (!strPath.IsEmpty())
	{
#if BlidErrCode
		if (!m_ModelMng->LoadFileList(strPath, m_resolX, m_resolY))
			ret = ePART_FAIL;
#endif
	}
	else
	{
		if (!m_ModelMng->ExtLoadFileList(m_resolX, m_resolY))
			ret = ePART_FAIL;
	}
	return ret;
}


int CProcPil_POCR::ModelList_Check(CString strPath)
{

	int ret = ePART_SUCCESS;
	if (!strPath.IsEmpty())
	{
#if BlidErrCode
		if (!m_ModelMng->CheckFileList(strPath, m_resolX, m_resolY))
			ret = ePART_FAIL;
#endif
	}
	else
	{
		if (!m_ModelMng->ExtCheckFileList(m_resolX, m_resolY))
			ret = ePART_FAIL;
	}
	return ret;
}


int CProcPil_POCR::ModelList_Clear(int nLane)
{

	int ret = ePART_SUCCESS;

	ret = m_ModelMng->Clear_File(nLane);

	return ret;
}

int CProcPil_POCR::ModelLoad(CString strPath)
{

	int ret = ePART_SUCCESS;

	ClearFont();


	CString strModelPath = _T("");
	strModelPath = strPath + _T(".mod");
	strPath.MakeUpper();
	m_ModelFile = m_ModelMng->GetModel(strPath);

	if(m_ModelFile.get() == NULL)
	{
		CString strLog;
		strLog.Format(_T("Memory Load Fail - %s"), strPath);
		ext::Log::add(strLog);
		return ePART_FAIL;
	}


	FontModel = m_ModelFile->milModel;
	if(FontModel == nullptr || FontModel->nfontListCnt<=0)
		ret = ePART_FAIL;

	return ret;
}

int CProcPil_POCR::ModelFileLoad(CString strPath)
{
	int ret = ePART_SUCCESS;

	ClearFont();

	CString strModelPath = _T("");
	strModelPath = strPath;// + _T(".psr");

	if(FileExists(strModelPath))
	{
		m_ModelFile = std::shared_ptr<ModelFile_POCR>(new ModelFile_POCR(eAlgoPOCR));
		m_ModelFile->LoadFile(strModelPath,m_resolX,m_resolY);

		if(m_ModelFile->milModel == nullptr)
			return ePART_FAIL;
	}
	else
	{
		ret = ePART_FAIL;
		return ret;
	}

	FontModel = m_ModelFile->milModel;
	if(FontModel == nullptr || FontModel->nfontListCnt<=0)
		ret = ePART_FAIL;

	return ret;
}


bool CProcPil_POCR::SearchPOCRDraw(UCHAR* userSrc,int width,int height,AlgoPOCR *InspAlgoPOCR,UCHAR * ucArrDstImgMask, int ModelIndex)
{
	if(FontModel==nullptr)
		return false;

	bool re = e_NG;
	try
	{
		POCRresult.reset();

		cv::Mat srcImg(height,width, CV_8UC1);

		//srcImg.data = userSrc;
		memcpy(srcImg.data,userSrc,width * height);

//		RotateImg_ipp(userSrc, width, height, Angle, &InspImg, &InspImgwidth, &InspImgheight);
//		RotateImg_ipp(MaskImg, width, height, Angle, &ucArrDstImgMask, &InspImgwidth, &InspImgheight);

#if ImageSave
		cv::imwrite("D:\\testimage\\workimage\\RotateImg_ipp.bmp",srcImg);
#endif
		FontModel->SearchFontDraw(srcImg, InspAlgoPOCR->m_bUsePolarity,ucArrDstImgMask,ModelIndex);


#if ImageSave
		cv::imwrite("D:\\testimage\\workimage\\POCRDrawResult.bmp",srcImg);
#endif
	}
	catch (CMemoryException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"),szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CFileException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"),szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch (CException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = {0, }; e->GetErrorMessage(szMsg, 1024); 

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"),szMsg);
		g_pMPTI->AddLog_Dev(msg);
		e->Delete();
	}
	catch(...)
	{
		CString msg;
		msg.Format(_T("CProcPil_POCR_SearchPOCR ..."));
		g_pMPTI->AddLog_Dev(msg);
	}

	return re;
}

bool CProcPil_POCR::SearchPOCRDraw3Ch(UCHAR* userSrc, int width, int height, AlgoPOCR *InspAlgoPOCR, UCHAR * ucArrDstImgMask, int ModelIndex)
{
	bool re = e_NG;
	try
	{
		POCRresult.reset();

		cv::Mat srcImg(height, width, CV_8UC1);

		memcpy(srcImg.data, userSrc, width * height);
#if ImageSave
		cv::imwrite("D:\\testimage\\workimage\\RotateImg_ipp.bmp", srcImg);
#endif
		//FontModel->SearchFontDraw3Ch(srcImg, InspAlgoPOCR->m_bUsePolarity, ucArrDstImgMask, ModelIndex);

		GeoModel gmo;
		gmo.Alloc(srcImg);
		gmo.setMatchAlgo(GeoModel::enmMatchAlgo::agDefault);
		gmo.setAngleRange(false, 0, 0, 0);
		gmo.setAccuracy(GeoModel::acHigh);
		gmo.setPrecision(GeoModel::enmPrecision::pcHigh);
		gmo.Preprocess();


		GeoMatch gMatch;
		GeoResult res(1), rotRes(1);
		cv::Mat Edge_pyr, Edge_half, Edge_img = srcImg.clone();
		int nPyrDownStep(0);
		do
		{
			Edge_pyr.release();
			cv::resize(Edge_img, Edge_pyr, cv::Size(Edge_img.cols*0.5, Edge_img.rows*0.5));
			Edge_img = Edge_pyr;
			nPyrDownStep++;
			if (nPyrDownStep == 1)
				Edge_half = Edge_pyr;

		} while (nPyrDownStep < gmo._pyrDownStep);

		std::shared_ptr<ImgProcessing> ImgProc;
		ImgProc = std::make_shared<ImgProcessing>();
		ImgProc->_ImgProcess(srcImg, Edge_half, Edge_pyr, gmo._pyrDownStep, (GeoModel::enmMatchAlgo)gmo.getMatchAlgo());
		gMatch.SetImgProcess(ImgProc);
		gMatch.FindModel(gmo, res, false);


		std::vector<std::shared_ptr<rstPSRModel>> rst;
		cv::Mat draw(srcImg.rows, srcImg.cols, CV_8UC3, ucArrDstImgMask);
		for (int r = 0; r < srcImg.rows; r++)
		{
			uchar* sPtr = srcImg.ptr(r);
			uchar* dPtr = draw.ptr(r);
			for (int c = 0; c < srcImg.cols; c++)
			{
				dPtr[c * 3] = sPtr[c];
				dPtr[c * 3 + 1] = sPtr[c];
				dPtr[c * 3 + 2] = sPtr[c];
			}
		}
		gMatch.DrawContours(draw, gmo, res, cv::Scalar(0, 0, 255), 1,0);

#if ImageSave
		cv::imwrite("D:\\testimage\\workimage\\POCRDrawResult.bmp", draw);
#endif
	}
	catch (CMemoryException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"), szMsg);
		g_pMPTI->AddLog_Dev(msg);
	}
	catch (CFileException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"), szMsg);
		g_pMPTI->AddLog_Dev(msg);
	}
	catch (CException* e)
	{
		CString msg;
		TCHAR szMsg[1024] = { 0, }; e->GetErrorMessage(szMsg, 1024);

		msg.Format(_T("CProcPil_POCR_SearchPOCR: %s"), szMsg);
		g_pMPTI->AddLog_Dev(msg);
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("CProcPil_POCR_SearchPOCR ..."));
		g_pMPTI->AddLog_Dev(msg);
	}

	return re;
}

void CProcPil_POCR::SetImgProc(UCHAR* userSrc,int width,int height,std::shared_ptr<ImgProcessing_POCR> ProcImg,AlgoPOCR *InspAlgoPOCR, bool rotate)
{
	ImgProc = ProcImg;
	if(!ProcImg->bIsImgProcessed)
	{
		UCHAR* InspImg = NULL;
		int  InspImgwidth, InspImgheight;

		int wndAngle = ((int)InspAlgoPOCR->m_dWndAngle);
		if(wndAngle%90 != 0)
			wndAngle = 0;

		double Angle = InspAlgoPOCR->m_dFontAngle-wndAngle;
		if(rotate)
			Angle = Angle + 180;

		if(Angle>360)
			Angle = Angle - 360;
		else if(Angle<0)
			Angle = Angle + 360;

		RotateImg_ipp(userSrc, width, height, Angle, &InspImg, &InspImgwidth, &InspImgheight);

		cv::Mat Img(InspImgheight , InspImgwidth,CV_8UC1,InspImg);

		if(FontModel->_sfList.size() <=0)
		{
			g_pMPTI->AddLog_OCR(_T("ProcPil_OCV::_ImgpProcess : ModelData is Empty"));

			if (InspImg != NULL)
			{
				g_pMManager->pem_delete(InspImg, true);
				InspImg = NULL;
			}

			return;
		}
		int nPyrDStep = FontModel->_sfList[0]->_OrgImage->_pyrDownStep;
		for each (std::shared_ptr<sFontList> sfList in FontModel->_sfList)
		{
			if (nPyrDStep > sfList->_OrgImage->_pyrDownStep)
				nPyrDStep = sfList->_OrgImage->_pyrDownStep;
		}

		ImgProc->_ImgProcess(Img.clone(), nPyrDStep,(GeoModel::enmMatchAlgo)FontModel->_sfList[0]->_OrgImage->getMatchAlgo());

		ProcImg = ImgProc;

#ifdef _DEBUG
		orgImage = Img.clone();
#endif
		orgImage = Img.clone();
		if(InspImg!=NULL)
		{
			//delete [] InspImg;
			g_pMManager->pem_delete(InspImg, true);
			InspImg = NULL;
		}
	}
}
int CProcPil_POCR::ModelCnt_Check(CString strPath)
{

	int ret = 0;

	CString strModelPath = _T("");
	strModelPath = strPath + _T(".mod");
	strPath.MakeLower();
	m_ModelFile = m_ModelMng->GetModel(strPath);

	if(m_ModelFile.get() == NULL)
	{
		return ret;
	}


	FontModel = m_ModelFile->milModel;

	if(FontModel.get() == NULL)
	{
		return ret;
	}
	ret = FontModel->nfontListCnt;

	return ret;
}
//int CProcPil_POCR::GetModelStr(int idx, SPOCRFontList* sStr)
//{
//
//	int ret = 0;
//
//	CString strModelStr = _T("");
//	if (FontModel.get() == NULL)
//	{
//		return ret;
//	}
//	if (FontModel->nfontListCnt > idx)
//	{
//		strModelStr = FontModel->GetModelStr(idx);
//		memcpy(sStr->m_sFontList, strModelStr.GetBuffer(), strModelStr.GetLength() * 2);
//	}
//
//	return ret;
//}
void CProcPil_POCR::SetExtPOCRFontInfo()
{
	ext::ExtPOCRFont stInfo = ext::irc::get()->_CtrlServer.First()->stPOCRInfo;

	for (int i = 0; i < POCRFont::Num; i++)
	{
		m_POCRInfoParam[i].eFontKey = (POCRFont)stInfo.stFontInfo[i].nFontKey;
		m_POCRInfoParam[i].nPOCRInspCnt = stInfo.stFontInfo[i].nPOCRInspCnt;
		if (m_POCRInfoParam[i].vPOCRInspPtr == NULL)
		{
			m_POCRInfoParam[i].vPOCRInspPtr = g_pMManager->pem_new<POCRInspPtr>(true, m_POCRInfoParam[i].nPOCRInspCnt, (PCHAR)__FUNCTION__, __LINE__);
			for (int j = 0; j < m_POCRInfoParam[i].nPOCRInspCnt; j++)
			{
				m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr = g_pMManager->pem_new<POCRInspParam>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
			}
		}

		for (int j = 0; j < m_POCRInfoParam[i].nPOCRInspCnt; j++)
		{
			m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr->eFontType = (POCRFontType)stInfo.stFontInfo[i].vPOCRInspParam[j].eFontType;
			m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr->eDivisionArea = (DivisionArea)stInfo.stFontInfo[i].vPOCRInspParam[j].eDivisionArea;
			m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr->nDivisionCnt = stInfo.stFontInfo[i].vPOCRInspParam[j].nDivisionCnt;
			m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr->nRequiredSocre = stInfo.stFontInfo[i].vPOCRInspParam[j].nRequiredSocre;

			for (int cnt = 0; cnt < MAX_POCR_DIVISITION_CNTS; cnt++)
			{
				m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr->ePOCRAlgoType[cnt] = (POCRAlgoType)stInfo.stFontInfo[i].vPOCRInspParam[j].ePOCRAlgoType[cnt];
				m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr->nDivisionScore[cnt] = stInfo.stFontInfo[i].vPOCRInspParam[j].nDivisionScore[cnt];
				m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr->eRequiredArea[cnt] = (RequiredType)stInfo.stFontInfo[i].vPOCRInspParam[j].eRequiredArea[cnt];
			}
		}
	}

	for (int i = 0; i < POCRFont::Num; i++)
	{
		m_POCRInfoParam[i].eFontKey = (POCRFont)stInfo.stFontInfo[i].nFontKey;
		m_POCRInfoParam[i].nPOCRInspCnt = stInfo.stFontInfo[i].nPOCRInspCnt;
		if (m_POCRInfoParam[i]._vPOCRInspPtr == NULL)
		{
			m_POCRInfoParam[i]._vPOCRInspPtr = g_pMManager->pem_new<cPOCRInspPtr>(true, m_POCRInfoParam[i].nPOCRInspCnt, (PCHAR)__FUNCTION__, __LINE__);
			for (int j = 0; j < m_POCRInfoParam[i].nPOCRInspCnt; j++)
			{
				m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr = g_pMManager->pem_new<cPOCRInspParam>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
			}
		}

		for (int j = 0; j < m_POCRInfoParam[i].nPOCRInspCnt; j++)
		{
			m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr->eFontType = (POCRFontType)stInfo.stFontInfo[i].vPOCRInspParam[j].eFontType;
			m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr->eDivisionArea = (DivisionArea)stInfo.stFontInfo[i].vPOCRInspParam[j].eDivisionArea;
			m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr->nDivisionCnt = stInfo.stFontInfo[i].vPOCRInspParam[j].nDivisionCnt;
			m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr->nRequiredSocre = stInfo.stFontInfo[i].vPOCRInspParam[j].nRequiredSocre;

			for (int cnt = 0; cnt < MAX_POCR_DIVISITION_CNTS; cnt++)
			{
				m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr->ePOCRAlgoType[cnt] = (POCRAlgoType)stInfo.stFontInfo[i].vPOCRInspParam[j].ePOCRAlgoType[cnt];
				m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr->nDivisionScore[cnt] = stInfo.stFontInfo[i].vPOCRInspParam[j].nDivisionScore[cnt];
				m_POCRInfoParam[i]._vPOCRInspPtr[j]._vPOCRInspPtr->eRequiredArea[cnt] = (RequiredType)stInfo.stFontInfo[i].vPOCRInspParam[j].eRequiredArea[cnt];
			}
		}
	}
}
void CProcPil_POCR::ClearExtPOCRFontInfo()
{
	for (int i = 0; i < POCRFont::Num; i++)
	{
		if (m_POCRInfoParam[i].vPOCRInspPtr != NULL)
		{
			for (int j = 0; j < m_POCRInfoParam[i].nPOCRInspCnt; j++)
			{
				if(m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr != NULL)
					g_pMManager->pem_delete(m_POCRInfoParam[i].vPOCRInspPtr[j].vPOCRInspPtr, false);
			}
			g_pMManager->pem_delete(m_POCRInfoParam[i].vPOCRInspPtr, true);
		}
	}
}