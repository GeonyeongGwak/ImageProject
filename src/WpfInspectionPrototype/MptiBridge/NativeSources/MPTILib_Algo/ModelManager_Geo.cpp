#include "StdAfx.h"
#include "ModelManager_Geo.h"
#include "MPTI.h"


void ModelFile_Pat::LoadFile(CString sFilePath,double resX,double resY)
{
	int nLine = __LINE__;
	try
	{
		if(milModel==nullptr)
			milModel = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());

		nLine = __LINE__;
		if (m_nAlgoType == eAlgoPattern || m_nAlgoType == eAlgoBody_Blob || m_nAlgoType == eAlgoFoot)
		{
			nLine = __LINE__;
			if(milModel->LoadFile(sFilePath,resX,resY)==false)
			{
				CString sNewPath =  sFilePath.Mid(0, sFilePath.GetLength()-4) + _T(".ppd");
				CString sImgPath =  sFilePath.Mid(0, sFilePath.GetLength()-4) + _T(".tif");
				CString extension = sFilePath.Mid(sFilePath.GetLength()-4, 4).MakeLower();
// 				if(extension != _T(".mod"))
// 					return;
// 
// 				if(_taccess(sNewPath, 0) != -1)
// 				{
// 					DeleteFile(sFilePath);
// 					return;
// 				}

				nLine = __LINE__;
				if(_taccess(sImgPath, 0) == 0)
				{
					nLine = __LINE__;
					cv::Mat srcImg = cv::imread(std::string(CT2A(sImgPath)),0);

					nLine = __LINE__;
					milModel->Alloc(srcImg);
					milModel->Preprocess();
					milModel->setRes(resX,resY);
					milModel->SaveFile(sNewPath, false);
					if(extension == _T(".mod"))
						DeleteFile(sFilePath);

					nLine = __LINE__;
				}
				else 
				{
					milModel = nullptr;
				}
			}
		}
	}
	catch(...)
	{
		if(g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("ModelManager_Geo::LoadFile(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
}
void ModelFile_Pat::ExtLoadFile(int nIndex, double resX, double resY)
{
	int nLine = __LINE__;
	try
	{
		if (milModel == nullptr)
			milModel = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());

		nLine = __LINE__;
		if (m_nAlgoType == eAlgoPattern || m_nAlgoType == eAlgoBody_Blob || m_nAlgoType == eAlgoFoot)
		{
			nLine = __LINE__;
			if (milModel->ExtLoadFile(nIndex, resX, resY) == false)
			{
				ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_PatternModelCtrl.First()[nIndex];
				CString sFilePath(pCtrl.sFilePath);
				CString sNewPath = sFilePath.Mid(0, sFilePath.GetLength() - 4) + _T(".ppd");
				CString sImgPath = sFilePath.Mid(0, sFilePath.GetLength() - 4) + _T(".tif");
				CString extension = sFilePath.Mid(sFilePath.GetLength() - 4, 4).MakeLower();
				nLine = __LINE__;
				if (_taccess(sImgPath, 0) == 0)
				{
					nLine = __LINE__;
					cv::Mat srcImg = cv::imread(std::string(CT2A(sImgPath)), 0);

					nLine = __LINE__;
					milModel->Alloc(srcImg);
					milModel->Preprocess();
					milModel->setRes(resX, resY);
					milModel->SaveFile(sNewPath, false);
					if (extension == _T(".mod"))
						DeleteFile(sFilePath);

					nLine = __LINE__;
				}
			}
		}
	}
	catch (...)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("ModelManager_Geo::LoadFile(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
}
void ModelFile_Pat::SetFileAlgoPath(fileAlgoPath* sFilePath)
{
	milModel->SetDivisionParam(sFilePath);
}

std::shared_ptr<GeoModel_Pattern> ModelFile_Pat::GetAngleModel(int nAngle)
{
	int AngleList[3] = { 90, 180, 270 };

	std::shared_ptr<GeoModel_Pattern> * ModelList[3] = { &milModel_90, &milModel_180, &milModel_270 };

	for (int i=0; i<3; i++)
	{
		if(AngleList[i] != nAngle)
			continue;

		if(ModelList[i][0] == nullptr)
		{
			ModelList[i][0] = std::make_shared<GeoModel_Pattern>();
			ModelList[i][0]->CopyOf(*milModel, AngleList[i]);
			//ModelList[i]->setAngle(AngleList[i]);
		}

		if(ModelList[i][0]->isProcessed()==false)
			ModelList[i][0]->Preprocess();

		return ModelList[i][0];
	}

	if(milModel->isProcessed()==false)
		milModel->Preprocess();

	return milModel;
}