#include "StdAfx.h"
#include "MModelManager.h"
#include "GeoMatch_Pattern.h"
#include <opencv2/opencv.hpp>

void tagModelFile::LoadFile(CString sFilePath,double resX,double resY)
{
// 	if (m_nAlgoType == eAlgoPattern)
// 	{
// 		int nStrLen = sFilePath.GetLength();
// 		CString sNewPath =  sFilePath.Mid(0, sFilePath.GetLength()-4) + _T(".mod");
// 		CString sExt = sFilePath.Mid(nStrLen-4, 4);
// 
// 		if(sExt == _T(".mod"))
// 			MpatRestore(M_DEFAULT_HOST, sFilePath, &milModel);
// 		else if(sExt == _T(".ppd"))
// 		{
// 			GeoModel_Pattern gm;
// 			if(gm.LoadFile(sFilePath,resX,resY)==true)
// 			{
// 				cv::Mat img = gm.Image();
// #ifdef _DEBUG
// 				cv::imwrite(std::string("d:\\testimage\\workimage\\PilPatLoaded.bmp"), img);
// #endif
// 				if(img.empty()==false)
// 				{
// 					m_bAllocedInternally = true;
// 
// 					MIL_ID imgBuf = MbufCreate2d(M_DEFAULT_HOST, img.cols, img.rows, 8+M_UNSIGNED, M_IMAGE+M_PROC, M_DEFAULT, img.step1()*img.elemSize1(), img.ptr(), M_NULL);
// #ifdef _DEBUG
// 					MbufExport(_T("d:\\testimage\\workimage\\PilPatTransformed.bmp"), M_BMP, imgBuf);
// #endif
// 					MpatAllocModel(M_DEFAULT_HOST, imgBuf, 0, 0, img.cols, img.rows, M_NORMALIZED, &milModel);
// 					
// 					MpatSetAngle(milModel, M_SEARCH_ANGLE_MODE, M_ENABLE);
// 					MpatSetAngle(milModel, M_SEARCH_ANGLE_INTERPOLATION_MODE, M_BILINEAR);
// 					MpatSetAngle(milModel, M_SEARCH_ANGLE_DELTA_NEG, 5.0);
// 					MpatSetAngle(milModel, M_SEARCH_ANGLE_DELTA_POS, 5.0);
// 					MpatSetAngle(milModel, M_SEARCH_ANGLE_ACCURACY, 0.5);
// 
// 					MpatSetAngle(milModel, M_SEARCH_ANGLE, 0.0);
// 
// 					MpatSetAcceptance(milModel, 40);	// ProcMIL_Pattern의 PAT_ATCSCORE이다. 나중에 합치는 방안을 생각해보자
// 
// 					MpatSetSpeed(milModel, M_MEDIUM);
// 					MpatSetAccuracy(milModel, M_MEDIUM);
// 
// 					MpatPreprocModel(M_NULL, milModel, M_DEFAULT);
// 					MpatSave(sNewPath, milModel);
// 					DeleteFile(sFilePath);
// 
// 					MbufFree(imgBuf);
// 				}
// 			}
// 		}	
// 	}
// 	else
// 		MstrRestore(sFilePath, M_DEFAULT_HOST, M_DEFAULT, &milModel);
}
void tagModelFile::ExtLoadFile(int nIndex, double resX, double resY)
{
	// 관리안함
}
void tagModelFile::SetFileAlgoPath(fileAlgoPath* sPath)
{
	//Mil관리 안함.
}

// CMModelManager::CMModelManager(void)
// {
// 	m_nAlgoType = eAlgoNum;
// }
// 
// CMModelManager::CMModelManager(InspAlgoType nAlgoType)
// {
// 	m_nAlgoType = nAlgoType;
// }
// 
// CMModelManager::~CMModelManager(void)
// {
// 	m_ModelList.RemoveAll();
// 	m_ModelList_R.RemoveAll();
// }
// 
// bool CMModelManager::LoadFileList(CString sFolderPath)
// {
// 	typModelArray *ModelList=nullptr;
// 
// 	if (sFolderPath.Find(_T("TEMP_REAR")) == -1)
// 	{
// 		m_ModelList.RemoveAll();
// 		ModelList = &m_ModelList;
// 	}
// 	else
// 	{
// 		m_ModelList_R.RemoveAll();
// 		ModelList = &m_ModelList_R;
// 	}
// 
// 	bool ret = true;
// 
// 	CFileFind Finder;
// 
// 	CString curFile = _T("");
// 	CString folderPath = sFolderPath + _T("\\*.*");
// 	CString extension = _T("");
// 
// 	int fileExist = Finder.FindFile(folderPath);
// 	if(fileExist)
// 	{
// 		while(fileExist)
// 		{
// 			fileExist = Finder.FindNextFile();
// 			if(Finder.IsDots())
// 			{//continue;
// 			}
// 			else if(Finder.IsDirectory())
// 			{// directory : cell ID
// 			}
// 			else
// 			{// file : complete.txt
// 
// 				curFile = Finder.GetFileName();
// 				curFile.MakeLower();
// 				extension = curFile.Mid(curFile.GetLength()-4,4);
// 				if(extension == _T(".mod") || extension == _T(".msr"))
// 				{
// 					typModel _ModelFile = typModel(new ModelFile(m_nAlgoType));
// 
// 					_ModelFile->ModelName = Finder.GetFileName();
// 					_ModelFile->lSize = Finder.GetLength();
// 					CString sFilePath = Finder.GetFilePath();
// 
// 					MIL_ID temp = M_NULL;
// 
// 					if (m_nAlgoType == eAlgoPattern)
// 						temp = MpatRestore(M_DEFAULT_HOST, sFilePath, &_ModelFile->milModel);
// 					else
// 						temp = MstrRestore(sFilePath, M_DEFAULT_HOST, M_DEFAULT, &_ModelFile->milModel);
// 
// 					if(temp == M_NULL)
// 						return false;
// 					ModelList->SetAt(_ModelFile->ModelName, typModel(_ModelFile));
// 				}
// 			}
// 		}
// 	}
// }
// 
// bool CMModelManager::CheckFileList(CString sFolderPath)
// {
// 	typModelArray *ModelList=nullptr;
// 
// 	if (sFolderPath.Find(_T("TEMP_REAR")) == -1)
// 		ModelList = &m_ModelList;
// 	else
// 		ModelList = &m_ModelList_R;
// 
// 	bool ret = true;
// 
// 	CFileFind Finder;
// 
// 	CString curFile = _T("");
// 	CString folderPath = sFolderPath + _T("\\*.*");
// 	CString extension = _T("");
// 
// 	int fileExist = Finder.FindFile(folderPath);
// 	if(fileExist)
// 	{
// 		while(fileExist)
// 		{
// 			fileExist = Finder.FindNextFile();
// 			if(Finder.IsDots())
// 			{//continue;
// 			}
// 			else if(Finder.IsDirectory())
// 			{// directory : cell ID
// 			}
// 			else
// 			{// file : complete.txt
// 
// 				curFile = Finder.GetFileName();
// 				curFile.MakeLower();
// 				extension = curFile.Mid(curFile.GetLength()-4,4);
// 				if(extension == _T(".mod") || extension == _T(".msr"))
// 				{
// 					typModel iter;
// 					if (ModelList->Lookup(Finder.GetFileName(), iter) == true)
// 					{
// 						ULONGLONG FileSize = 0;
// 						FileSize = Finder.GetLength();
// 
// 						if (iter->lSize == FileSize)
// 							continue;
// 					}
// 
// 					typModel _ModelFile = typModel(new ModelFile(m_nAlgoType));
// 
// 					_ModelFile->ModelName = Finder.GetFileName();
// 					_ModelFile->lSize = Finder.GetLength();
// 					CString sFilePath = Finder.GetFilePath();
// 
// 					MIL_ID temp = M_NULL;
// 
// 					if (m_nAlgoType == eAlgoPattern)
// 						temp = MpatRestore(M_DEFAULT_HOST, sFilePath, &_ModelFile->milModel);
// 					else
// 						temp = MstrRestore(sFilePath, M_DEFAULT_HOST, M_DEFAULT, &_ModelFile->milModel);
// 
// 					if(temp == M_NULL)
// 						return false;
// 
// 					ModelList->SetAt(_ModelFile->ModelName, typModel(_ModelFile));
// 				}
// 			}
// 		}
// 	}
// }
// 
// CMModelManager::typModel CMModelManager::GetModel(CString sFilePath)
// {
// 	typModelArray *ModelList;
// 
// 	if (sFilePath.Find(_T("TEMP_REAR")) == -1)
// 	{
// 		ModelList = &m_ModelList;
// 	}
// 	else
// 	{
// 		ModelList = &m_ModelList_R;
// 	}
// 
// 	CString strModelName = sFilePath.Right(sFilePath.GetLength() - sFilePath.ReverseFind('\\') - 1);
// 	
// 	typModel val;
// 	ModelList->Lookup(strModelName, val);
// 
// 	return val;
// }
// 
// void CMModelManager::Remove(CString sFilePath)
// {
// 	typModelArray *ModelList;
// 
// 	if (sFilePath.Find(_T("TEMP_REAR")) == -1)
// 	{
// 		ModelList = &m_ModelList;
// 	}
// 	else
// 	{
// 		ModelList = &m_ModelList_R;
// 	}
// 
// 	CString strModelName = sFilePath.Right(sFilePath.GetLength() - sFilePath.ReverseFind('\\') - 1);
// 
// 	ModelList->RemoveKey(sFilePath);
// }
// 
// bool CMModelManager ::Clear_File(int nLane)
// {
// 	if (nLane == 0)// Front
// 	{
// 		m_ModelList.RemoveAll();
// 	}
// 	else if (nLane == 1)// Rear
// 	{
// 		m_ModelList_R.RemoveAll();
// 	}
// 	else // All
// 	{
// 		m_ModelList.RemoveAll();
// 		m_ModelList_R.RemoveAll();
// 	}
// 
// 	return true;
// }
