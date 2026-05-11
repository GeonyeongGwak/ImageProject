#pragma once

#include "pinsp.h"
#include "MModelManager.h"
#include "GeoMatch_Foot.h"


class ModelFile_Foot
{
public:
	InspAlgoType m_nAlgoType;

	wchar_t ModelName[MAX_STRLEN];
	ULONGLONG lSize;
	int FootType;
	cv::Rect WedgeRect;
	int FootDirection;

	CString getFileName() { return ModelName; }

	void LoadFile(CString sFilePath, double resX, double resY);
	void SetFileAlgoPath(fileAlgoPath* sFilePath);
	void ReadWedgeArea();
	std::shared_ptr<GeoModel_Foot> GetAngleModel(int nAngle);
	void SetAngleModel();

	bool isAlloced() { return milModel != nullptr; }

	std::shared_ptr<GeoModel_Foot> milModel;

	std::shared_ptr<GeoModel_Foot> milModel_90;
	std::shared_ptr<GeoModel_Foot> milModel_180;
	std::shared_ptr<GeoModel_Foot> milModel_270;

	fileAlgoPath* m_sFilePath;
public:
	ModelFile_Foot(InspAlgoType nAlgoType)
	{
		m_nAlgoType = nAlgoType;
		//ModelName = "";
		_tcscpy(ModelName, _T(""));
		lSize = 0;
		WedgeRect.x = 0;
		WedgeRect.y = 0;
		WedgeRect.width = 1;
		WedgeRect.height = 1;
	}
	virtual ~ModelFile_Foot()
	{

	}
};
