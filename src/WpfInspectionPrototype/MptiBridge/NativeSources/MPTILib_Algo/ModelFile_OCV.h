#pragma once

//#include "pinsp.h"
#include "MModelManager.h"
#include "PsrFileManager.h"


class ModelFile_POCR
{
public:
	InspAlgoType m_nAlgoType;

	CString sOcrText;
	wchar_t ModelName[MAX_STRLEN];
	ULONGLONG lSize;

	CString getFileName() { return ModelName; }

	void LoadFile(CString sFilePath, double resX, double resY);
	void ExtLoadFile(int nIndex, double resX, double resY);
	void SetFileAlgoPath(fileAlgoPath* sFilePath);
	void SetFileAlgoPath(void* sFilePath);
	std::shared_ptr<PsrModel> GetAngleModel(int nAngle);

	bool isAlloced() { return milModel != nullptr; }

	fileAlgoPath* m_sFilePath;

	std::shared_ptr<PsrModel> milModel;

public:
	ModelFile_POCR(InspAlgoType nAlgoType)
	{
		m_nAlgoType = nAlgoType;
		m_sFilePath = nullptr;
		//ModelName = "";
		_tcscpy(ModelName, _T(""));
		lSize = 0;

		milModel = nullptr;
		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	virtual ~ModelFile_POCR()
	{
		g_pMManager->pem_delete_check(this);
	}
};

