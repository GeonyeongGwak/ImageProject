#pragma once

#include "pinsp.h"
#include "MModelManager.h"
#include "GeoMatch_Pattern.h"

class ModelFile_Pat
{
public:
	InspAlgoType m_nAlgoType;

	CString sOcrText;
	wchar_t ModelName[MAX_STRLEN];
	//CString ModelName;
	ULONGLONG lSize;

	CString getFileName() { return ModelName; }

	void LoadFile(CString sFilePath,double resX,double resY);
	void ExtLoadFile(int nIndex, double resX, double resY);
	void SetFileAlgoPath(fileAlgoPath* sFilePath);
	std::shared_ptr<GeoModel_Pattern> GetAngleModel(int nAngle);

	bool isAlloced() { return milModel!=nullptr; }

	fileAlgoPath* m_sFilePath;

	std::shared_ptr<GeoModel_Pattern> milModel;

	std::shared_ptr<GeoModel_Pattern> milModel_90;
	std::shared_ptr<GeoModel_Pattern> milModel_180;
	std::shared_ptr<GeoModel_Pattern> milModel_270;
public:
	ModelFile_Pat(InspAlgoType nAlgoType)
	{
		m_nAlgoType = nAlgoType;
		m_sFilePath = nullptr;
		//ModelName = "";
		_tcscpy(ModelName, _T(""));
		lSize = 0;
		milModel = nullptr;
		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	virtual ~ModelFile_Pat()
	{
		g_pMManager->pem_delete_check(this);
	}
};

