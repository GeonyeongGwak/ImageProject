#include "StdAfx.h"
#include "ModelFile_OCV.h"



void ModelFile_POCR::LoadFile(CString sFilePath, double resX, double resY)
{
	int nLine = __LINE__;
	try
	{
		if (milModel == nullptr)
			milModel = std::shared_ptr<PsrModel>(new PsrModel());

		nLine = __LINE__;
		if (milModel->Load(sFilePath, resX, resY) == false)
		{
			milModel = nullptr;
		}
	}
	catch(...)
	{
	/*	if(g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("ModelManager_Geo::LoadFile(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}*/
	}
}
void ModelFile_POCR::ExtLoadFile(int nIndex, double resX, double resY)
{
	int nLine = __LINE__;
	try
	{
		if (milModel == nullptr)
			milModel = std::shared_ptr<PsrModel>(new PsrModel());

		nLine = __LINE__;
		milModel->ExtLoad(nIndex, resX, resY);
	}
	catch (...)
	{
	}
}
void ModelFile_POCR::SetFileAlgoPath(fileAlgoPath* sFilePath)
{

}


std::shared_ptr<PsrModel> ModelFile_POCR::GetAngleModel(int nAngle)
{
	// 	int AngleList[3] = { 90, 180, 270 };
	// 
	// 	std::shared_ptr<GeoModel_POCR> * ModelList[3] = { &milModel_90, &milModel_180, &milModel_270 };
	// 
	// 	for (int i=0; i<3; i++)
	// 	{
	// 		if(AngleList[i] != nAngle)
	// 			continue;
	// 
	// 		if(ModelList[i][0] == nullptr)
	// 		{
	// 			ModelList[i][0] = std::make_shared<GeoModel_POCR>();
	// 			ModelList[i][0]->CopyOf(*milModel, AngleList[i]);
	// 			//ModelList[i]->setAngle(AngleList[i]);
	// 		}
	// 
	// 		if(ModelList[i][0]->isProcessed()==false)
	// 			ModelList[i][0]->Preprocess();
	// 
	// 		return ModelList[i][0];
	// 	}
	// 
	// 	if(milModel->isProcessed()==false)
	// 		milModel->Preprocess();

	return milModel;
}
