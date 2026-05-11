#pragma once

#include "pinsp.h"
#include "procmil.h"
#include <memory>
#include "ExtInspLogic.h"

//typedef struct tagModelFile
class tagModelFile
{
public:
	InspAlgoType m_nAlgoType;

	CString sOcrText;
	wchar_t ModelName[MAX_STRLEN];
	ULONGLONG lSize;

	fileAlgoPath* m_sFilePath;

	CString getFileName() { return ModelName; }

	void LoadFile(CString sFilePath,double resX,double resY);
	void ExtLoadFile(int nIndex , double resX, double resY);
	void SetFileAlgoPath(fileAlgoPath* sPath);
	bool isAllocedInternally() { return m_bAllocedInternally; }

	bool isAlloced() { return milModel!=M_NULL; }

	Im::PIL_ID milModel;

	Im::PIL_ID milModel_90;
	Im::PIL_ID milModel_180;
	Im::PIL_ID milModel_270;

public:
	tagModelFile(InspAlgoType nAlgoType)
	{
		m_bAllocedInternally  = false;
		m_nAlgoType = nAlgoType;

		//ModelName = "";


//		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);

		_tcscpy(ModelName, _T(""));
		lSize = 0;

		milModel = M_NULL;
		milModel_90 = M_NULL;
		milModel_180 = M_NULL;
		milModel_270 = M_NULL;

		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	virtual ~tagModelFile()
	{
		Free();

		g_pMManager->pem_delete_check(this);
	}

	virtual void Free()
	{
		if(milModel)
		{
			switch(m_nAlgoType)
			{
			case eAlgoOCR: 
				{
					if (milModel != M_NULL){ MstrFree(milModel); milModel = M_NULL; }
				}
				break;
			case eAlgoPattern :
				{
					if (milModel != M_NULL){ MpatFree(milModel); milModel = M_NULL; }
					if (milModel_90 != M_NULL){ MpatFree(milModel_90); milModel_90 = M_NULL; }
					if (milModel_180 != M_NULL){ MpatFree(milModel_180); milModel_180 = M_NULL; }
					if (milModel_270 != M_NULL){ MpatFree(milModel_270); milModel_270 = M_NULL; }
				}
				break;
			}
		}
	}

protected:
	bool m_bAllocedInternally;
};//ModelFile;



template<typename T>
class CMModelManager
{
public:
	CMModelManager(InspAlgoType nAlgoType, CString sMasterExt, void (* fPoint)(CString))
	{
		m_sMasterExt = sMasterExt.MakeLower();
		m_nAlgoType = nAlgoType;
		m_fPoint = fPoint;

		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	virtual ~CMModelManager(void)
	{
// 		m_ModelList.RemoveAll();
// 		m_ModelList_R.RemoveAll();

		m_ModelList.clear();
		m_ModelList_R.clear();

		g_pMManager->pem_delete_check(this);
	}

public:
	typedef std::shared_ptr<T> typModel;
//	typedef CAtlMap<CString, typModel, CStringElementTraits<CString>> typModelArray;
	typedef std::map<CString, typModel> typModelArray;

private:
	InspAlgoType m_nAlgoType;
	typModelArray m_ModelList;
	typModelArray m_ModelList_R;
	CString m_sMasterExt;
	void(*m_fPoint)(CString);

public:
	virtual bool LoadFileList(CString sFolderPath,double resX,double resY);
	bool ExtLoadFileList(double resX, double resY);
	int ExtCheckFileList(double resX, double resY,int nModelType = 0 );
	virtual int LoadFileList(fileAlgoPath* sPath, int ptrFileAlgoPathCnt,double resX,double resY);
	int ExtLoadFileList_P(double resX, double resY);
	//virtual int LoadFileList(SPOCRfileAlgoPath* sPath, int cnt, double resX, double resY);
	virtual int CheckFileList(CString sFolderPath,double resX,double resY);
	virtual int CheckFileList(fileAlgoPath* sPath, int ptrFileAlgoPathCnt, double resX, double resY);
	virtual int CheckFileList(CString sFolderPath, double resX, double resY, CString s2D3D);
	virtual int CheckFileList(SPOCRfileAlgoPath* sPath, int cnt, double resX, double resY);

	int CheckFileList2(CString sFolderPath, double resX, double resY, BOOL& bSizeDiff);

	virtual bool LoadFileList(CString sFolderPath, double resX, double resY, CString s2D3D);
	bool _LoadFileListFoot(CString sFolderPath, CString sExt, double resX, double resY, CString s2D3D);
	int GetSimilarModel(CString sPartcode,bool rear, std::vector<typModel> & SimmilarModelList);

	int GetModelCount(bool rear)
	{
		typModelArray *ModelList=nullptr;

		if (!rear)
			ModelList = &m_ModelList;
		else
			ModelList = &m_ModelList_R;

		return ModelList->size();
	}
	typModel GetModel(CString sFilePath)
	{
		typModelArray *ModelList;

		bool bFrontData = sFilePath.Find(_T("TEMP_REAR")) == -1;
		if (bFrontData && m_ModelList.size() > 0)
			ModelList = &m_ModelList;
		else if (bFrontData == false && m_ModelList_R.size() > 0)
			ModelList = &m_ModelList_R;
		else
			return nullptr;// __asm int 3;

		if (sFilePath.GetLength() < 6)
			return nullptr;// __asm int 3;

		CString strModelName = sFilePath.Right(sFilePath.GetLength() - sFilePath.ReverseFind('\\') - 1);

		CString strName;
		int nIdx = strModelName.ReverseFind('.');
		if(nIdx >= (strModelName.GetLength()-5))
			strName = strModelName.Mid(0, nIdx);
		else
			strName = strModelName;
		strName.MakeLower();
		typModel val;
	//	ModelList->Lookup(strName, val);
		auto iter = ModelList->find(strName);
		if(iter != ModelList->end())
			val = iter->second;

		return val;
	}

	typModel GetPartSearchModel(CString sFilePath)
	{
		typModelArray *ModelList;

		bool bFrontData = sFilePath.Find(_T("TEMP_REAR")) == -1;

		if (bFrontData && m_ModelList.size() > 0)
			ModelList = &m_ModelList;
		else if (bFrontData == false && m_ModelList_R.size() > 0)
			ModelList = &m_ModelList_R;
		else
			return nullptr;// __asm int 3;

		if (sFilePath.GetLength() < 6)
			return nullptr;// __asm int 3;

		CString strModelName = sFilePath.Right(sFilePath.GetLength() - sFilePath.ReverseFind('\\') - 1);

		CString strName;
		int nIdx = strModelName.ReverseFind('.');
		if(nIdx >= (strModelName.GetLength()-5))
			strName = strModelName.Mid(0, nIdx);
		else
			strName = strModelName;
		strName.MakeLower();
		typModel val;
		//	ModelList->Lookup(strName, val);
		auto iter = ModelList->find(strName);
		for (typModelArray::iterator iter=ModelList->begin(); iter!=ModelList->end(); iter++)
		{
			if(iter->first.Find(strName)>=0)
			{
				val = iter->second;
				break;
			}
		}

		return val;
	}

	virtual bool Clear_File(int nLane);
	virtual void Remove(CString sFilePath);

	

protected:
	void LoadModelFile(CString sFilePath, typModel modelfile,double resX,double resY);
	bool _LoadFileList(CString sFolderPath, CString sExt,double resX,double resY);
	bool _ExtLoadFileList(ext::ModelCtrl * pCtrl, CString sExt, double resX, double resY, int nIndex);
	int _LoadFileList(fileAlgoPath* sPath, CString sExt, int ptrFileAlgoPathCnt,double resX,double resY);
	int _CheckFileList(fileAlgoPath* sPath, CString sExt, int ptrFileAlgoPathCnt, double resX, double resY);
	int _LoadFileList(SPOCRfileAlgoPath* sPath, CString sExt, int ptrFileAlgoPathCnt, double resX, double resY);
	int _CheckFileList(SPOCRfileAlgoPath* sPath, CString sExt, int ptrFileAlgoPathCnt, double resX, double resY);
};

template<typename T>
bool CMModelManager<T>::LoadFileList(CString sFolderPath,double resX,double resY)
{
	bool ret(false);
// 	if(m_nAlgoType == eAlgoPattern)
// 	{
// 		if(m_sMasterExt == _T(".ppd"))
// 		{
// 			ret |= _LoadFileList(sFolderPath, m_sMasterExt);
// 			ret |= _LoadFileList(sFolderPath, _T(".mod"));
// 
// 			return ret;
// 		}
// 		else if(m_sMasterExt == _T(".mod"))
// 		{
// 			ret |= _LoadFileList(sFolderPath, m_sMasterExt);
// 			ret |= _LoadFileList(sFolderPath, _T(".ppd"));
// 
// 			return ret;
// 		}
// 		else
// 		{
// 			ret |= _LoadFileList(sFolderPath, _T(".ppd"));
// 			ret |= _LoadFileList(sFolderPath, _T(".mod"));
// 
// 			return ret;
// 		}
// 	}
// 	else 
		if(m_nAlgoType == eAlgoOCR)
		return _LoadFileList(sFolderPath, m_sMasterExt,resX,resY);
	else if(m_nAlgoType == eAlgoPOCR)
		return _LoadFileList(sFolderPath, m_sMasterExt,resX,resY);
	else if(m_nAlgoType == eAlgoBody_Blob)
		return _LoadFileList(sFolderPath, m_sMasterExt,resX,resY);
	else if (m_nAlgoType == eAlgoFoot)
			return _LoadFileListFoot(sFolderPath, m_sMasterExt, resX, resY,_T(""));
	return ret;
}
template<typename T>
bool CMModelManager<T>::ExtLoadFileList( double resX, double resY)
{
	bool ret(false);
	ext::ModelCtrl * pCtrl; 
	pCtrl = &ext::InspRoot_Server::get()->_POCRModelCtrl;
// 	if (m_nAlgoType == eAlgoOCR)
// 		return _LoadFileList(sFolderPath, m_sMasterExt, resX, resY);
/*	else */if (m_nAlgoType == eAlgoPOCR)
		return _ExtLoadFileList( pCtrl, m_sMasterExt, resX, resY,ext::InspRoot_Server::get()->_InspStatus.First()->nModelIndex_POCR);
// 	else if (m_nAlgoType == eAlgoBody_Blob)
// 		return _LoadFileList(sFolderPath, m_sMasterExt, resX, resY);
// 	else if (m_nAlgoType == eAlgoFoot)
// 		return _LoadFileListFoot(sFolderPath, m_sMasterExt, resX, resY, _T(""));
	return ret;
}
template<typename T>
bool CMModelManager<T>::LoadFileList(CString sFolderPath, double resX, double resY, CString s2D3D)
{
	bool ret(false);
	if (m_nAlgoType == eAlgoOCR)
		return _LoadFileList(sFolderPath, m_sMasterExt, resX, resY);
	else if (m_nAlgoType == eAlgoPOCR)
		return _LoadFileList(sFolderPath, m_sMasterExt, resX, resY);
	else if (m_nAlgoType == eAlgoBody_Blob)
		return _LoadFileList(sFolderPath, m_sMasterExt, resX, resY);
	else if (m_nAlgoType == eAlgoFoot)
		return _LoadFileListFoot(sFolderPath, m_sMasterExt, resX, resY, s2D3D);
	return ret;
}

template<typename T>
int CMModelManager<T>::LoadFileList(fileAlgoPath* sPath, int cnt,double resX,double resY)
{
	int ret(0);
	if(m_nAlgoType == eAlgoPattern)
	{
		if(m_sMasterExt == _T(".ppd"))
		{
			ret += _LoadFileList(sPath, m_sMasterExt,cnt,resX,resY);
			ret += _LoadFileList(sPath, _T(".mod"),cnt,resX,resY);

			return ret;
		}
		else if(m_sMasterExt == _T(".mod"))
		{
// 			ret |= _LoadFileList(sPath, m_sMasterExt,cnt,resX,resY);
// 			ret |= _LoadFileList(sPath, _T(".ppd"),cnt,resX,resY);

			return ret;
		}
		else
		{
			ret += _LoadFileList(sPath, _T(".ppd"),cnt,resX,resY);
			//ret |= _LoadFileList(sPath, _T(".mod"),cnt,resX,resY);

			return ret;
		}
	}
// 	else if(m_nAlgoType == eAlgoOCR)
// 		return _LoadFileList(sPath, m_sMasterExt,cnt);
// 	else if(m_nAlgoType == eAlgoPOCR)
// 		return _LoadFileList(sPath, m_sMasterExt,cnt);
	else if(m_nAlgoType == eAlgoBlob)
		return _LoadFileList(sPath, m_sMasterExt,cnt,resX,resY);
	return ret;
}
template<typename T>
int CMModelManager<T>::ExtLoadFileList_P(double resX, double resY)
{
	int ret(0);
	ext::ModelCtrl * pCtrl;
	pCtrl = &ext::InspRoot_Server::get()->_PatternModelCtrl;
	if (m_nAlgoType == eAlgoPattern)
	{
		if (m_sMasterExt == _T(".ppd"))
		{


			ret += _ExtLoadFileList(pCtrl, m_sMasterExt, resX, resY, ext::InspRoot_Server::get()->_InspStatus.First()->nModelIndex_Pattern);
			//ret += _ExtLoadFileList(pCtrl, _T(".mod"), resX, resY, ext::InspRoot_Server::get()->_InspStatus.First()->nModelIndex_Pattern);

			return ret;
		}
		else if (m_sMasterExt == _T(".mod"))
		{
			// 			ret |= _LoadFileList(sPath, m_sMasterExt,cnt,resX,resY);
			// 			ret |= _LoadFileList(sPath, _T(".ppd"),cnt,resX,resY);

			return ret;
		}
		else
		{
			ret += _ExtLoadFileList(pCtrl, _T(".ppd"),  resX, resY, ext::InspRoot_Server::get()->_InspStatus.First()->nModelIndex_Pattern);
			//ret |= _LoadFileList(sPath, _T(".mod"),cnt,resX,resY);

			return ret;
		}
	}
	// 	else if(m_nAlgoType == eAlgoOCR)
	// 		return _LoadFileList(sPath, m_sMasterExt,cnt);
	// 	else if(m_nAlgoType == eAlgoPOCR)
	// 		return _LoadFileList(sPath, m_sMasterExt,cnt);
// 	else if (m_nAlgoType == eAlgoBlob)
// 		return _LoadFileList(sPath, m_sMasterExt, cnt, resX, resY);
	return ret;
}

template<typename T>
bool CMModelManager<T>::_LoadFileListFoot(CString sFolderPath, CString sExt, double resX, double resY,CString s2D3D)
{
	typModelArray *ModelList = nullptr;

	if (sFolderPath.Find(_T("TEMP_REAR")) == -1)
	{
		//	m_ModelList.RemoveAll();
		if (sExt == m_sMasterExt)
			m_ModelList.clear();
		ModelList = &m_ModelList;
	}
	else
	{
		//	m_ModelList_R.RemoveAll();
		if (sExt == m_sMasterExt)
			m_ModelList_R.clear();
		ModelList = &m_ModelList_R;
	}

	bool ret = true;

	CFileFind Finder;

	CString curFile = _T("");
	CString folderPath;
	CString extension = _T("");

	folderPath.Format(_T("%s\\*.*"), sFolderPath);
	int fileExist = Finder.FindFile(folderPath);

	while (fileExist)
	{

		fileExist = Finder.FindNextFile();

		if (Finder.IsDots())
		{//continue;
		}
		else if (Finder.IsDirectory())
		{// directory : cell ID
		}
		else
		{// file : complete.txt

			curFile = Finder.GetFileName();
			curFile.MakeLower();
			if (curFile.GetLength() < 6)
				continue;
			extension = curFile.Mid(curFile.GetLength() - 4, 4);
			if (extension == sExt)
			{
				if (sExt != m_sMasterExt)
				{
					CString sNewPath = curFile.Mid(0, curFile.GetLength() - 4) + m_sMasterExt;
					if (_taccess(sNewPath, 0) != -1)
					{
						DeleteFile(curFile);
						return false;
					}
				}
				int nFndIndex = curFile.Find(s2D3D.MakeLower());
				if (nFndIndex == -1)
					continue;
				typModel iter;
				CString sModelKey = curFile.Mid(0, curFile.GetLength() - 4);

				//	if (ModelList->Lookup(sModelKey, iter) == true)		// 중복된 파일명이 있으면 
				auto iterp = ModelList->find(sFolderPath);
				if (iterp != ModelList->end())
					return false;

				typModel _ModelFile = typModel(new T(m_nAlgoType));

				CString ModelName = Finder.GetFileName();
				wchar_t* wcharArr = ModelName.GetBuffer();
				memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
				//_ModelFile->ModelName = Finder.GetFileName();
				_ModelFile->lSize = Finder.GetLength();
				CString sFilePath = Finder.GetFilePath();

				_ModelFile->LoadFile(sFilePath, resX, resY);

				// 예외처리
				if (_ModelFile->isAlloced() == true)
					ModelList->insert(std::make_pair(sModelKey, typModel(_ModelFile)));
				//	ModelList[sModelKey] = typModel(_ModelFile);
				//	ModelList->SetAt(sModelKey, typModel(_ModelFile));
			}
		}
	}

	return ret;
}

template<typename T>
bool CMModelManager<T>::_LoadFileList(CString sFolderPath, CString sExt,double resX,double resY)
{
	typModelArray *ModelList=nullptr;

	if (sFolderPath.Find(_T("TEMP_REAR")) == -1)
	{
	//	m_ModelList.RemoveAll();
		if(sExt == m_sMasterExt)
			m_ModelList.clear();
		ModelList = &m_ModelList;
	}
	else
	{
	//	m_ModelList_R.RemoveAll();
		if(sExt == m_sMasterExt)
			m_ModelList_R.clear();
		ModelList = &m_ModelList_R;
	}

	bool ret = true;

	CFileFind Finder;

	CString curFile = _T("");
	CString folderPath;
	CString extension = _T("");

	folderPath.Format(_T("%s\\*.*"),sFolderPath);
	int fileExist = Finder.FindFile(folderPath);

	while(fileExist)
	{

		fileExist = Finder.FindNextFile();

		if(Finder.IsDots())
		{//continue;
		}
		else if(Finder.IsDirectory())
		{// directory : cell ID
		}
		else
		{// file : complete.txt

			curFile = Finder.GetFileName();
			curFile.MakeLower();
			if (curFile.GetLength() < 6)
				continue;
			extension = curFile.Mid(curFile.GetLength()-4,4);
			if(extension == sExt)
			{
				if(sExt != m_sMasterExt)
				{
					CString sNewPath =  curFile.Mid(0, curFile.GetLength()-4) + m_sMasterExt;
					if(_taccess(sNewPath, 0)!=-1)
					{
						DeleteFile(curFile);
						return false;
					}
				}

				typModel iter;
				CString sModelKey = curFile.Mid(0, curFile.GetLength()-4);

				//	if (ModelList->Lookup(sModelKey, iter) == true)		// 중복된 파일명이 있으면 
				auto iterp = ModelList->find(sFolderPath);
				if (iterp != ModelList->end())
					return false;

				typModel _ModelFile = typModel(new T(m_nAlgoType));

				CString ModelName = Finder.GetFileName();
				wchar_t* wcharArr = ModelName.GetBuffer();
				memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
				memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
				//_ModelFile->ModelName = Finder.GetFileName();
				_ModelFile->lSize = Finder.GetLength();
				CString sFilePath = Finder.GetFilePath();

				_ModelFile->LoadFile(sFilePath,resX,resY);

				// 예외처리
				if(_ModelFile->isAlloced() == true)
					ModelList->insert(std::make_pair(sModelKey, _ModelFile));
				//	ModelList[sModelKey] = typModel(_ModelFile);
				//	ModelList->SetAt(sModelKey, typModel(_ModelFile));
			}
		}
	}

	return ret;
}

template<typename T>
bool CMModelManager<T>::_ExtLoadFileList(ext::ModelCtrl * pCtrl, CString sExt, double resX, double resY,int nIndex)
{
	typModelArray *ModelList = nullptr;
	m_ModelList.clear(); // 공유메모리사용 해야하니 먼저 클리어
	m_ModelList_R.clear();
	CString sLog;
	sLog.Format(_T("[EXT_Model_Start %d ]Load Cnt/ F:%d, R:%d"), (int)m_nAlgoType, m_ModelList.size(), m_ModelList_R.size());
	ext::Log::add(sLog);

	bool ret = true;
	for (int nLoop = 0; nLoop < nIndex; nLoop++)
	{
		CString sFolderPath(pCtrl->First()[nLoop].sFilePath);
		ext::Log::add(sFolderPath);
		if (sFolderPath.Find(_T("TEMP_REAR")) == -1)
		{
			if (sExt == m_sMasterExt)
			//	m_ModelList.clear();
			ModelList = &m_ModelList;
		}
		else
		{
			if (sExt == m_sMasterExt)
			//	m_ModelList_R.clear();
			ModelList = &m_ModelList_R;
		}

		CString CurrentFile = sFolderPath;
		CurrentFile.MakeLower();
		CString extension = CurrentFile.Mid(CurrentFile.GetLength() - 4, 4);
		if (extension == sExt)
		{
			typModel iter;
			typModel _ModelFile = typModel(new T(m_nAlgoType));
			CString ModelName = PathFindFileName(CurrentFile);
			wchar_t* wcharArr = ModelName.GetBuffer();
			memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
			memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
			CString sModelKey = ModelName.Mid(0, ModelName.GetLength() - 4);
			_ModelFile->lSize = pCtrl->First()[nLoop].img.imgSz;//Finder.GetLength();
			CString sFilePath = CurrentFile;
			_ModelFile->ExtLoadFile(nLoop, resX, resY);
		//	g_pMPTI->CheckPOCRFontVersion(((ext::ModelStatus)pCtrl->First()[nLoop]).sFilePath);
			if (_ModelFile->isAlloced() == true)
				ModelList->insert(std::make_pair(sModelKey, typModel(_ModelFile)));
		}
	}

	sLog.Format(_T("[EXT_Model_End %d]Load Cnt/ F:%d, R:%d"), (int)m_nAlgoType, m_ModelList.size(), m_ModelList_R.size());
	ext::Log::add(sLog);
	return ret;
}
template<typename T>
int CMModelManager<T>::_LoadFileList(fileAlgoPath* sPath, CString sExt,int cnt,double resX,double resY)
{
	typModelArray *ModelList=nullptr;
	CString sPathModelTeach;
	sPathModelTeach.Format(_T("%s"), sPath[0].m_sPathModelTeach);
	if (sPathModelTeach.Find(_T("TEMP_REAR")) == -1)
	{
		//	m_ModelList.RemoveAll();
		if(sExt == m_sMasterExt)
			m_ModelList.clear();
		ModelList = &m_ModelList;
	}
	else
	{
		//	m_ModelList_R.RemoveAll();
		if(sExt == m_sMasterExt)
			m_ModelList_R.clear();
		ModelList = &m_ModelList_R;
	}

	bool ret = true;

	

	for(int i=0;i<cnt;i++)
	{
	CString curFile = _T("");
	CString folderPath; //= sPath[0].m_sPathModelTeach + _T(".*");
	CString extension = _T("");

		CFileFind Finder;


		folderPath.Format(_T("%s.*"),sPath[i].m_sPathModelTeach);
		int fileExist = Finder.FindFile(folderPath);
		if(fileExist == 0)
			continue;
		Finder.FindNextFile();
		curFile = Finder.GetFileName();
		curFile.MakeLower();
		if (curFile.GetLength() < 6)
			continue;
		extension = curFile.Mid(curFile.GetLength()-4,4);
		if(extension == sExt)
		{
			if(sExt != m_sMasterExt)
			{
				CString sNewPath =  curFile.Mid(0, curFile.GetLength()-4) + m_sMasterExt;
				if(_taccess(sNewPath, 0)!=-1)
				{
					DeleteFile(curFile);
					return false;
				}
			}

			typModel iter;
			CString sModelKey = curFile.Mid(0, curFile.GetLength()-4);

			//	if (ModelList->Lookup(sModelKey, iter) == true)		// 중복된 파일명이 있으면 
			auto iterp = ModelList->find(sPath[i].m_sPathModelTeach);
			if (iterp != ModelList->end())
				return false;

			typModel _ModelFile = typModel(new T(m_nAlgoType));

			CString ModelName = Finder.GetFileName();
			wchar_t* wcharArr = ModelName.GetBuffer();
			memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
			memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
			//_ModelFile->ModelName = Finder.GetFileName();
			_ModelFile->lSize = Finder.GetLength();
			CString sFilePath = Finder.GetFilePath();

			_ModelFile->LoadFile(sFilePath,resX,resY);
			_ModelFile->SetFileAlgoPath(&sPath[i]);

			// 예외처리
			if(_ModelFile->isAlloced() == true)
				ModelList->insert(std::make_pair(sModelKey, typModel(_ModelFile)));
			//	ModelList[sModelKey] = typModel(_ModelFile);
			//	ModelList->SetAt(sModelKey, typModel(_ModelFile));
		}
		else if(_T(".mod") == extension)
		{
			continue;
			//return 100;
		}
	}
	return ret;
}
template<typename T>
int CMModelManager<T>::ExtCheckFileList(double resX, double resY, int nModelType)
{
	ext::ModelCtrl * pCtrl;
	int nIndex = 0;
	switch (nModelType)
	{
	case ext::ModelBufferFlag::eExtModel_POCR:
		pCtrl = &ext::InspRoot_Server::get()->_POCRModelCtrl;
		nIndex = ext::InspRoot_Server::get()->_InspStatus.First()->nModelIndex_POCR;
		break;
	case ext::ModelBufferFlag::eExtModel_Pattern:
		pCtrl = &ext::InspRoot_Server::get()->_PatternModelCtrl;
		nIndex = ext::InspRoot_Server::get()->_InspStatus.First()->nModelIndex_Pattern;
		break;
		
	default:
		break;
	}
	

	CString sLog;
	sLog.Format(_T("[EXT_Model_StartCheck %d ]Check Cnt/ F:%d, R:%d"), (int)m_nAlgoType, m_ModelList.size(), m_ModelList_R.size());
	ext::Log::add(sLog);

	
	typModelArray *ModelList = nullptr;
	CString lane;
	bool ret = true;
	CString extension = _T("");
	int cnt = 0;
	//bool bReLoad = true; 
	for (int nLoop = 0; nLoop < nIndex; nLoop++)
	{// file : complete.txt
		CString CurrentFile(pCtrl->First()[nLoop].sFilePath);
		
		if (CurrentFile.Find(_T("TEMP_REAR")) == -1)
		{
			ModelList = &m_ModelList;
			lane.Format(_T("Front"));
		}
		else
		{
			ModelList = &m_ModelList_R;
			lane.Format(_T("Rear"));
		}
		CurrentFile.MakeLower();
		CString extension = CurrentFile.Mid(CurrentFile.GetLength() - 4, 4);
		if ((extension == _T(".ppd") || extension == _T(".psr") || extension == _T(".pim") || extension == _T(".ppd")))
		{
			cnt++;
			if (ret)
			{
				typModel iter;

				CString sNameTemp = PathFindFileName(CurrentFile);
				CString sName = sNameTemp.Mid(0, sNameTemp.GetLength() - 4);
				auto iterP = ModelList->find(sName);
				if (iterP != ModelList->end())
				{
					CString sNameP = iterP->second->getFileName();
					CString sExtP = sNameP.Mid(sNameP.GetLength() - 4, 4);
					iter = iterP->second;
					if (extension != sExtP)
						continue;
					if (iter->lSize == pCtrl->First()[nLoop].img.imgSz)
						continue;
					else
						ret = false;
					CString sLog;
					sLog.Format(_T("src %d != dst %d"), iter->lSize, pCtrl->First()[nLoop].img.imgSz);
					ext::Log::add(sLog);
				}
			}
	
			typModel _ModelFile = typModel(new T(m_nAlgoType));

			CString ModelName = PathFindFileName(CurrentFile);
			wchar_t* wcharArr = ModelName.GetBuffer();
			memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
			memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
			CString sModelKey = ModelName.Mid(0, ModelName.GetLength() - 4);
			_ModelFile->lSize = pCtrl->First()[nLoop].img.imgSz;//Finder.GetLength();
			_ModelFile->ExtLoadFile(nLoop, resX, resY);
			CString sLog; 
			sLog.Format(_T("Check_Fail_Load : %s"), sModelKey);
			ext::Log::add(sLog);
			(*ModelList)[sModelKey] = typModel(_ModelFile);
		}
	}
	CString Str;
	Str.Format(_T("%s PatternCount : %d"), lane, cnt);

	sLog.Format(_T("[EXT_Model_End %d]Check Cnt/ F:%d, R:%d"), (int)m_nAlgoType, m_ModelList.size(), m_ModelList_R.size());
	ext::Log::add(sLog);

	//if (sFolderPath.Find(_T("PatternModel")) > 0)
	//{
	//	m_fPoint(Str);
	//}

	return ret;
}

template<typename T>
int CMModelManager<T>::CheckFileList(CString sFolderPath,double resX,double resY)
{
	typModelArray *ModelList=nullptr;
	CString lane;
	if (sFolderPath.Find(_T("TEMP_REAR")) == -1)
	{
		ModelList = &m_ModelList;
		lane.Format(_T("Front"));
	}
	else
	{
		ModelList = &m_ModelList_R;
		lane.Format(_T("Rear"));
	}
	bool ret = true;

	CFileFind Finder;

	CString curFile = _T("");
	CString folderPath = sFolderPath + _T("\\*.*");
	CString extension = _T("");

	int cnt = 0;

	int fileExist = Finder.FindFile(folderPath);
	if(fileExist)
	{
		while(fileExist)
		{
			fileExist = Finder.FindNextFile();
			if(Finder.IsDots())
			{//continue;
			}
			else if(Finder.IsDirectory())
			{// directory : cell ID
			}
			else
			{// file : complete.txt

				curFile = Finder.GetFileName();
				curFile.MakeLower();
				if (curFile.GetLength() < 6)
					continue;
				CString sName = curFile.Mid(0,curFile.GetLength()-4);
				extension = curFile.Mid(curFile.GetLength()-4,4);
				if(extension == _T(".ppd") /*|| extension == _T(".mod") || extension == _T(".msr") */||extension == _T(".psr") ||extension == _T(".pim") )
				{
					cnt++;
					
					typModel iter;
				//	if (ModelList->Lookup(Finder.GetFileName(), iter) == true)
				//	if(ModelList->Lookup(sName, iter) == true)
					auto iterP = ModelList->find(sName);
					if(iterP != ModelList->end())
					{
						CString sNameP = iterP->second->getFileName();
						CString sExtP = sNameP.Mid(sNameP.GetLength()-4,4);

						if(extension != sExtP)
							continue;

						iter = iterP->second;
						ULONGLONG FileSize = 0;
						FileSize = Finder.GetLength();

						if (iter->lSize == FileSize)
							continue;
						else
							ret = false;
						typModel _ModelFile = typModel(new T(m_nAlgoType));
  
						CString ModelName = Finder.GetFileName();
						wchar_t* wcharArr = ModelName.GetBuffer();
						memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
						memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength()*sizeof(wchar_t));
						//_ModelFile->ModelName = Finder.GetFileName();
						_ModelFile->lSize = Finder.GetLength();
						CString sFilePath = Finder.GetFilePath();

						_ModelFile->LoadFile(sFilePath, resX, resY);

						int nStrLen = curFile.GetLength();
						// 					ModelList->SetAt(sName, typModel(_ModelFile));
						// 					ModelList[sName] = typModel(_ModelFile);
						iterP->second = typModel(_ModelFile);
						continue;
					}
					
					typModel _ModelFile = typModel(new T(m_nAlgoType));

					CString ModelName = Finder.GetFileName();
					wchar_t* wcharArr = ModelName.GetBuffer();
					memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
					memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
					//_ModelFile->ModelName = Finder.GetFileName();
					_ModelFile->lSize = Finder.GetLength();
					CString sFilePath = Finder.GetFilePath();

					_ModelFile->LoadFile(sFilePath,resX,resY);

					int nStrLen = curFile.GetLength();
// 					ModelList->SetAt(sName, typModel(_ModelFile));
// 					ModelList[sName] = typModel(_ModelFile);
					(*ModelList)[sName] = typModel(_ModelFile);
// 					ModelList->erase(sName);
// 					ModelList->insert(std::make_pair(sName, typModel(_ModelFile)));
				}
// 				else if(extension == _T(".mod"))
// 				{
// 					//AfxMessageBox(_T("MIL pattern file detect. please convert file"));
// 					//throw;
// 					return 100;
// 				}
			}
			
		}
		
	}
// 	CString Str;
// 	Str.Format(_T("%s PatternCount : %d"), lane, cnt);
// 
// 	if(sFolderPath.Find(_T("PatternModel")) > 0)
// 	{
// 		m_fPoint(Str);
// 	}
	
	return ret;
}

template<typename T>
int CMModelManager<T>::CheckFileList(CString sFolderPath, double resX, double resY,CString s2D3D)
{
	typModelArray *ModelList = nullptr;
	CString lane;
	if (sFolderPath.Find(_T("TEMP_REAR")) == -1)
	{
		ModelList = &m_ModelList;
		lane.Format(_T("Front"));
	}
	else
	{
		ModelList = &m_ModelList_R;
		lane.Format(_T("Rear"));
	}
	bool ret = true;

	CFileFind Finder;

	CString curFile = _T("");
	CString folderPath = sFolderPath + _T("\\*.*");
	CString extension = _T("");

	int cnt = 0;

	int fileExist = Finder.FindFile(folderPath);
	if (fileExist)
	{
		while (fileExist)
		{
			fileExist = Finder.FindNextFile();
			if (Finder.IsDots())
			{//continue;
			}
			else if (Finder.IsDirectory())
			{// directory : cell ID
			}
			else
			{// file : complete.txt

				curFile = Finder.GetFileName();
				curFile.MakeLower();
				CString sName = curFile.Mid(0, curFile.GetLength() - 4);
				extension = curFile.Mid(curFile.GetLength() - 4, 4);
				if (extension == _T(".ppd") /*|| extension == _T(".mod") || extension == _T(".msr") */ || extension == _T(".psr") || extension == _T(".pim"))
				{
					cnt++;

					int nFndIndex = curFile.Find(s2D3D.MakeLower());
					if (nFndIndex == -1)
						continue;

					typModel iter;
					//	if (ModelList->Lookup(Finder.GetFileName(), iter) == true)
					//	if(ModelList->Lookup(sName, iter) == true)
					auto iterP = ModelList->find(sName);
					if (iterP != ModelList->end())
					{
						CString sNameP = iterP->second->getFileName();
						CString sExtP = sNameP.Mid(sNameP.GetLength() - 4, 4);

						if (extension != sExtP)
							continue;

						iter = iterP->second;
						ULONGLONG FileSize = 0;
						FileSize = Finder.GetLength();

						if (iter->lSize == FileSize)
							continue;
					}

					typModel _ModelFile = typModel(new T(m_nAlgoType));

					CString ModelName = Finder.GetFileName();
					wchar_t* wcharArr = ModelName.GetBuffer();
					memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
					//_ModelFile->ModelName = Finder.GetFileName();
					_ModelFile->lSize = Finder.GetLength();
					CString sFilePath = Finder.GetFilePath();

					_ModelFile->LoadFile(sFilePath, resX, resY);

					int nStrLen = curFile.GetLength();
					// 					ModelList->SetAt(sName, typModel(_ModelFile));
					// 					ModelList[sName] = typModel(_ModelFile);
					(*ModelList)[sName] = typModel(_ModelFile);
					// 					ModelList->erase(sName);
					// 					ModelList->insert(std::make_pair(sName, typModel(_ModelFile)));
				}
				// 				else if(extension == _T(".mod"))
				// 				{
				// 					//AfxMessageBox(_T("MIL pattern file detect. please convert file"));
				// 					//throw;
				// 					return 100;
				// 				}
			}

		}

	}
	CString Str;
	Str.Format(_T("%s PatternCount : %d"), lane, cnt);

	if (sFolderPath.Find(_T("PatternModel")) > 0)
	{
		m_fPoint(Str);
	}

	return ret;
}

template<typename T>
int CMModelManager<T>::CheckFileList(fileAlgoPath* sPath, int cnt, double resX, double resY)
{
	int ret(0);
	if (m_nAlgoType == eAlgoPattern)
	{
		if (m_sMasterExt == _T(".ppd"))
		{
			ret += _CheckFileList(sPath, m_sMasterExt, cnt, resX, resY);
			ret += _CheckFileList(sPath, _T(".mod"), cnt, resX, resY);

			return ret;
		}
		else if (m_sMasterExt == _T(".mod"))
		{
			// 			ret |= _LoadFileList(sPath, m_sMasterExt,cnt,resX,resY);
			// 			ret |= _LoadFileList(sPath, _T(".ppd"),cnt,resX,resY);

			return ret;
		}
		else
		{
			ret += _CheckFileList(sPath, _T(".ppd"), cnt, resX, resY);
			//ret |= _LoadFileList(sPath, _T(".mod"),cnt,resX,resY);

			return ret;
		}
	}
	return ret;
}
template<typename T>
int CMModelManager<T>::_CheckFileList(fileAlgoPath* sPath, CString sExt, int cnt, double resX, double resY)
{
	typModelArray *ModelList = nullptr;
	CString lane;
	CString sPathModelTeach;
	sPathModelTeach.Format(_T("%s"), sPath[0].m_sPathModelTeach);
	if (sPathModelTeach.Find(_T("TEMP_REAR")) == -1)
	{
		//	m_ModelList.RemoveAll();
		ModelList = &m_ModelList;
		lane.Format(_T("Front"));
	}
	else
	{
		//	m_ModelList_R.RemoveAll();
		ModelList = &m_ModelList_R;
		lane.Format(_T("Rear"));
	}

	bool ret = true;

	for (int i = 0; i < cnt; i++)
	{
		CString curFile = _T("");
		CString folderPath; //= sPath[0].m_sPathModelTeach + _T(".*");
		CString extension = _T("");

		CFileFind Finder;
		folderPath.Format(_T("%s.*"), sPath[i].m_sPathModelTeach);
		int fileExist = Finder.FindFile(folderPath);
		if (fileExist == 0)
			continue;
		Finder.FindNextFile();
		curFile = Finder.GetFileName();
		curFile.MakeLower();
		if (curFile.GetLength() < 6)
			continue;
		extension = curFile.Mid(curFile.GetLength() - 4, 4);
		if (extension == sExt)
		{
			if (sExt != m_sMasterExt)
			{
				CString sNewPath = curFile.Mid(0, curFile.GetLength() - 4) + m_sMasterExt;
				if (_taccess(sNewPath, 0) != -1)
				{
					DeleteFile(curFile);
					return false;
				}
			}

			typModel iter;
			CString sModelKey = curFile.Mid(0, curFile.GetLength() - 4);
			auto iterP = ModelList->find(sModelKey);
			if (iterP != ModelList->end())
			{
				CString sNameP = iterP->second->getFileName();
				CString sExtP = sNameP.Mid(sNameP.GetLength() - 4, 4);

				if (extension != sExtP)
					continue;

				iter = iterP->second;
				ULONGLONG FileSize = 0;
				FileSize = Finder.GetLength();

				if (iter->lSize == FileSize)
					continue;
				typModel _ModelFile = typModel(new T(m_nAlgoType));
				CString ModelName = Finder.GetFileName();
				wchar_t* wcharArr = ModelName.GetBuffer();
				memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
				memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
				//_ModelFile->ModelName = Finder.GetFileName();
				_ModelFile->lSize = Finder.GetLength();
				CString sFilePath = Finder.GetFilePath();

				_ModelFile->LoadFile(sFilePath, resX, resY);

				int nStrLen = curFile.GetLength();
				// 					ModelList->SetAt(sName, typModel(_ModelFile));
				// 					ModelList[sName] = typModel(_ModelFile);
				iterP->second = typModel(_ModelFile);
				continue;
			}

			//	if (ModelList->Lookup(sModelKey, iter) == true)		// 중복된 파일명이 있으면 

			typModel _ModelFile = typModel(new T(m_nAlgoType));

			CString ModelName = Finder.GetFileName();
			wchar_t* wcharArr = ModelName.GetBuffer();
			memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
			memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
			//_ModelFile->ModelName = Finder.GetFileName();
			_ModelFile->lSize = Finder.GetLength();
			CString sFilePath = Finder.GetFilePath();

			_ModelFile->LoadFile(sFilePath, resX, resY);
			_ModelFile->SetFileAlgoPath(&sPath[i]);

			// 예외처리
			if (_ModelFile->isAlloced() == true)
				ModelList->insert(std::make_pair(sModelKey, typModel(_ModelFile)));
			//	ModelList[sModelKey] = typModel(_ModelFile);
			//	ModelList->SetAt(sModelKey, typModel(_ModelFile));
		}
		else if (_T(".mod") == extension)
		{
			continue;
			//return 100;
		}
	}

	return ret;
}
template<typename T>
int CMModelManager<T>::CheckFileList(SPOCRfileAlgoPath* sPath, int cnt, double resX, double resY)
{
	int ret(0);
	if (m_nAlgoType == eAlgoPOCR)
	{
		if (m_sMasterExt == _T(".psr"))
		{
			return _CheckFileList(sPath, m_sMasterExt, cnt, resX, resY);
		}
	}
	return ret;
}
template<typename T>
int CMModelManager<T>::_CheckFileList(SPOCRfileAlgoPath* sPath, CString sExt, int cnt, double resX, double resY)
{
	typModelArray *ModelList = nullptr;
	CString lane;
	CString sPathModelTeach;
	sPathModelTeach.Format(_T("%s"), sPath[0].m_sPathModelPath);
	if (sPathModelTeach.Find(_T("TEMP_REAR")) == -1)
	{
		//	m_ModelList.RemoveAll();
		ModelList = &m_ModelList;
		lane.Format(_T("Front"));
	}
	else
	{
		//	m_ModelList_R.RemoveAll();
		ModelList = &m_ModelList_R;
		lane.Format(_T("Rear"));
	}

	bool ret = true;

	for (int i = 0; i < cnt; i++)
	{
		CString curFile = _T("");
		CString folderPath; //= sPath[0].m_sPathModelPath + _T(".*");
		CString extension = _T("");

		CFileFind Finder;
		folderPath.Format(_T("%s"), sPath[i].m_sPathModelPath);
		int fileExist = Finder.FindFile(folderPath);
		if (fileExist == 0)
			continue;
		Finder.FindNextFile();
		curFile = Finder.GetFileName();
		curFile.MakeLower();
		if (curFile.GetLength() < 6)
			continue;
		extension = curFile.Mid(curFile.GetLength() - 4, 4);
		if (extension == sExt)
		{
			if (sExt != m_sMasterExt)
			{
				CString sNewPath = curFile.Mid(0, curFile.GetLength() - 4) + m_sMasterExt;
				if (_taccess(sNewPath, 0) != -1)
				{
					DeleteFile(curFile);
					return false;
				}
			}

			typModel iter;
			CString sModelKey = curFile.Mid(0, curFile.GetLength() - 4);
			auto iterP = ModelList->find(sModelKey);
			if (iterP != ModelList->end())
			{
				CString sNameP = iterP->second->getFileName();
				CString sExtP = sNameP.Mid(sNameP.GetLength() - 4, 4);

				if (extension != sExtP)
					continue;

				iter = iterP->second;
				ULONGLONG FileSize = 0;
				FileSize = Finder.GetLength();

				if (iter->lSize == FileSize)
					continue;
				typModel _ModelFile = typModel(new T(m_nAlgoType));
				CString ModelName = Finder.GetFileName();
				wchar_t* wcharArr = ModelName.GetBuffer();
				memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
				memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
				//_ModelFile->ModelName = Finder.GetFileName();
				_ModelFile->lSize = Finder.GetLength();
				CString sFilePath = Finder.GetFilePath();

				_ModelFile->LoadFile(sFilePath, resX, resY);

				int nStrLen = curFile.GetLength();
				// 					ModelList->SetAt(sName, typModel(_ModelFile));
				// 					ModelList[sName] = typModel(_ModelFile);
				iterP->second = typModel(_ModelFile);
				continue;
			}

			//	if (ModelList->Lookup(sModelKey, iter) == true)		// 중복된 파일명이 있으면 

			typModel _ModelFile = typModel(new T(m_nAlgoType));

			CString ModelName = Finder.GetFileName();
			wchar_t* wcharArr = ModelName.GetBuffer();
			memset(_ModelFile->ModelName, 0, MAX_STRLEN * sizeof(wchar_t));
			memcpy(_ModelFile->ModelName, wcharArr, ModelName.GetLength() * sizeof(wchar_t));
			//_ModelFile->ModelName = Finder.GetFileName();
			_ModelFile->lSize = Finder.GetLength();
			CString sFilePath = Finder.GetFilePath();

			_ModelFile->LoadFile(sFilePath, resX, resY);

			// 예외처리
			if (_ModelFile->isAlloced() == true)
				ModelList->insert(std::make_pair(sModelKey, typModel(_ModelFile)));
			//	ModelList[sModelKey] = typModel(_ModelFile);
			//	ModelList->SetAt(sModelKey, typModel(_ModelFile));
		}
	}

	return ret;
}
template<typename T>
void CMModelManager<T>::Remove(CString sFilePath)
{
	typModelArray *ModelList;

	if (sFilePath.GetLength() < 6)
		return;

	if (sFilePath.Find(_T("TEMP_REAR")) == -1 && m_ModelList.size()>0)
	{
		ModelList = &m_ModelList;
	}
	else
	{
		ModelList = &m_ModelList_R;
	}

	int idx = sFilePath.ReverseFind('\\');

	if (idx < 0)
		return;

	CString strModelName = sFilePath.Right(sFilePath.GetLength() - idx - 1);
	strModelName.MakeLower();
//	ModelList->RemoveKey(sFilePath);
	ModelList->erase(sFilePath);
}

template<typename T>
bool CMModelManager<T>::Clear_File(int nLane)
{
	if (nLane == 0)// Front
	{
	//	m_ModelList.RemoveAll();
		m_ModelList.clear();
	}
	else if (nLane == 1)// Rear
	{
	//	m_ModelList_R.RemoveAll();
		m_ModelList_R.clear();
	}
	else // All
	{
	//	m_ModelList.RemoveAll();
	//	m_ModelList_R.RemoveAll();

		m_ModelList.clear();
		m_ModelList_R.clear();
	}

	return true;
}

template<typename T>
int CMModelManager<T>::GetSimilarModel(CString sPartcode,bool rear,std::vector<typModel> & SimmilarModelList)
{
	typModelArray *ModelList=nullptr;

	int simCount = 0;

	if (!rear)
		ModelList = &m_ModelList;
	else
		ModelList = &m_ModelList_R;
	//ModelList->Lookup(strName, val);
	sPartcode.MakeLower();
	for (typModelArray::iterator iter=ModelList->begin(); iter!=ModelList->end(); iter++)
	{
		if(iter->first.Find(sPartcode)>=0)
		{
		//	SimmilarModelList[simCount] = iter->second;
			SimmilarModelList.push_back(iter->second);
			simCount++;
		}
	}
	return simCount;
}
