#include "StdAfx.h"
#include "ModelFile_Foot.h"
#include "MPTI.h"

void ModelFile_Foot::LoadFile(CString sFilePath, double resX, double resY)
{
	int nLine = __LINE__;
	try
	{
		if (milModel == nullptr)
			milModel = std::shared_ptr<GeoModel_Foot>(new GeoModel_Foot());

		nLine = __LINE__;
		if (m_nAlgoType == eAlgoFoot)
		{
			nLine = __LINE__;
			if (milModel->LoadFile(sFilePath, resX, resY) == false)
			{
				CString sNewPath = sFilePath.Mid(0, sFilePath.GetLength() - 4) + _T(".ppd");
				CString sImgPath = sFilePath.Mid(0, sFilePath.GetLength() - 4) + _T(".tif");
				CString extension = sFilePath.Mid(sFilePath.GetLength() - 4, 4).MakeLower();
				if (extension != _T(".mod"))
					return;

				if (_taccess(sNewPath, 0) != -1)
				{
					DeleteFile(sFilePath);
					return;
				}

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
					DeleteFile(sFilePath);

					nLine = __LINE__;
				}
			}
			SetAngleModel();
			ReadWedgeArea();
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

void ModelFile_Foot::ReadWedgeArea()
{
	if (isAlloced())
	{
		FootType = milModel->FootType;
		WedgeRect.x = milModel->WedgeArea.x;
		WedgeRect.y = milModel->WedgeArea.y;
		WedgeRect.width = milModel->WedgeArea.width;
		WedgeRect.height = milModel->WedgeArea.height;
		FootDirection = milModel->FootDirection;
	}
}
void ModelFile_Foot::SetFileAlgoPath(fileAlgoPath* sFilePath)
{
	milModel->SetDivisionParam(sFilePath);
}

std::shared_ptr<GeoModel_Foot> ModelFile_Foot::GetAngleModel(int nAngle)
{
	int AngleList[3] = { 90, 180, 270 };

	std::shared_ptr<GeoModel_Foot> * ModelList[3] = { &milModel_90, &milModel_180, &milModel_270 };

	for (int i = 0; i < 3; i++)
	{
		if (AngleList[i] != nAngle)
			continue;

		if (ModelList[i][0] == nullptr)
		{
			ModelList[i][0] = std::make_shared<GeoModel_Foot>();
			ModelList[i][0]->CopyOf(*milModel, AngleList[i]);
			//ModelList[i]->setAngle(AngleList[i]);
		}

		if (ModelList[i][0]->isProcessed() == false)
			ModelList[i][0]->Preprocess();

		return ModelList[i][0];
	}

	if (milModel->isProcessed() == false)
		milModel->Preprocess();

	return milModel;
}

void ModelFile_Foot::SetAngleModel()
{
#if !_OFFLINE
		if (milModel_90 == nullptr)
		{
			milModel_90 = std::shared_ptr<GeoModel_Foot>(new GeoModel_Foot());
			milModel_90->CopyOf(*milModel, 90.0F);
			if (milModel_90->isProcessed() == false)
				milModel_90->Preprocess();
		}
		if (milModel_180 == nullptr)
		{
			milModel_180 = std::shared_ptr<GeoModel_Foot>(new GeoModel_Foot());
			milModel_180->CopyOf(*milModel, 180.0F);
			if (milModel_180->isProcessed() == false)
				milModel_180->Preprocess();
		}
		if (milModel_270 == nullptr)
		{
			milModel_270 = std::shared_ptr<GeoModel_Foot>(new GeoModel_Foot());
			milModel_270->CopyOf(*milModel, 270.0F);
			if (milModel_270->isProcessed() == false)
				milModel_270->Preprocess();
		}
#endif
}