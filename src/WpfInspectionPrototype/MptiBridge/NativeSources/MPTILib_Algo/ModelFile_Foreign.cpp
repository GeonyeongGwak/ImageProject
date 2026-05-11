#include "StdAfx.h"
#include "ModelFile_Foreign.h"
#include "MPTI.h"

void ModelFile_Foreign::LoadFile(CString sFilePath,double resX,double resY)
{
	int nLine = __LINE__;
	try
	{
		if(milModel==nullptr)
			milModel = std::shared_ptr<ImgModels>(new ImgModels());

		nLine = __LINE__;
		milModel->LoadFile(sFilePath,resX,resY);
	}
	catch(...)
	{
		if(g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("ModelFile_Foreign::LoadFile(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
}

void ModelFile_Foreign::SetFileAlgoPath(fileAlgoPath* sFilePath)
{

}

std::shared_ptr<ImgModels> ModelFile_Foreign::GetAngleModel(int nAngle)
{
	return milModel;
}