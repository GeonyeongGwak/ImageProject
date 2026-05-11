#pragma once

#include "pinsp.h"
#include "MModelManager.h"
#include "ImageModels.h"


class ModelFile_Foreign
{
public:
	InspAlgoType m_nAlgoType;

	CString sOcrText;
	wchar_t ModelName[MAX_STRLEN];
	ULONGLONG lSize;

	CString getFileName() { return ModelName; }

	void LoadFile(CString sFilePath,double resX,double resY);
	void SetFileAlgoPath(fileAlgoPath* sFilePath);
	std::shared_ptr<ImgModels> GetAngleModel(int nAngle);

	bool isAlloced() { return milModel!=nullptr; }

	fileAlgoPath* m_sFilePath;

	std::shared_ptr<ImgModels> milModel;

public:
	ModelFile_Foreign(InspAlgoType nAlgoType)
	{
		m_nAlgoType = nAlgoType;
		m_sFilePath = nullptr;
		//ModelName = "";
		_tcscpy(ModelName, _T(""));
		lSize = 0;

		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	virtual ~ModelFile_Foreign()
	{
		g_pMManager->pem_delete_check(this);
	}
};
