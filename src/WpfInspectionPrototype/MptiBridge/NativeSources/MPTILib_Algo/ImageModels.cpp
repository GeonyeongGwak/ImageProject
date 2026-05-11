#include "StdAfx.h"
#include "ImageModels.h"
#include <opencv2/opencv.hpp>
#include <set>
#include <math.h>
#include <fstream>
#include <functional>


ImgModels::ImgModels()
{
	cv::Mat _mergedModel;
	ModelCnt = 0;

	_sLoadedPath = _T("");

	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
}
ImgModels::~ImgModels()
{
	g_pMManager->pem_delete_check(this);
}

bool ImgModels::SaveFile(CString sPath)
{
	CString extension = sPath.Mid(sPath.GetLength()-4, 4).MakeLower();
	if(extension != _T(".pim"))
		return false;

	CFile file;
	CFileException fe;
	if(file.Open(sPath, CFile::modeCreate | CFile::modeWrite, NULL, &fe)==FALSE)
		return false;

	CArchive ar(&file, CArchive::store);

	SaveFile(ar);

	ar.Close();
	file.Close();

	return true;
}
bool ImgModels::LoadFile(CString sPath,double resX,double resY)
{
	CString extension = sPath.Mid(sPath.GetLength()-4, 4).MakeLower();
	if(extension != _T(".pim"))
		return false;

	_sLoadedPath = sPath;

	CFile file;
	CFileException fe;
	if(file.Open(sPath, CFile::modeRead, NULL, &fe)==FALSE)
		return false;


	CArchive ar(&file, CArchive::load, file.GetLength());

	LoadFile(ar, resX, resY);

	ar.Close();
	file.Close();


	return true;
}


bool ImgModels::SaveFile(CArchive& ar)
{
	try
	{
		// 헤더
		CString sName(_T("pemtron"));
		int version(3);

		ar << sName;
		ar << version;

		ar << ModelCnt;

		// 기본 정보
		if(_mergedModel.empty())
		{
			ar << 0;
			ar << 0;
		}
		else
		{
			ar << _mergedModel.cols;
			ar << _mergedModel.rows;

			for (int y=0; y<_mergedModel.rows; y++)
			{
				uchar * ptr = _mergedModel.ptr(y);
				for (int x=0; x<_mergedModel.cols; x++)
					ar << ptr[x];
			}
		}
		SaveFile_v2( ar,version);
		SaveFile_v3( ar,version);
	}
	catch(...)
	{

		AfxMessageBox(_T("ImgModels, SaveFile fail!!"));
		return false;
	}

	return true;
}


bool ImgModels::LoadFile(CArchive& ar,double resX,double resY)
{
	try
	{
		bool rst=true;;

		// 헤더
		CString sName(_T("pemtron")), sReadName;

		ar >> sReadName;
		if(sReadName != sName)
			return false;
		int version(0);

		ar >> version;

		if (version > 3)
			return false;

		ar >> ModelCnt;

		// 기본 정보 
		int Wid(0), Len(0);
		ar >> Wid;
		ar >> Len;
		if(Wid>0 && Len>0)
		{
			_mergedModel = cv::Mat(Len, Wid, CV_8UC1);
			for (int y=0; y<_mergedModel.rows; y++)
			{
				uchar * ptr = _mergedModel.ptr(y);
				for (int x=0; x<_mergedModel.cols; x++)
					ar >> ptr[x];
			}
		}
		rst = LoadFile_v1(ar,version);
		rst = LoadFile_v2(ar,version);
		rst = LoadFile_v3(ar,version,resX,resY);

		return rst;
	}
	catch(...)
	{

		AfxMessageBox(_T("ImgModels, LoadFile fail!!"));
		return false;
	}
}

bool ImgModels::SaveFile_v1(CArchive& ar,int version)
{
	//shkim version0->1: img x&y&cols&rows -2pixel
	return true;
}

bool ImgModels::LoadFile_v1(CArchive& ar,int version)
{
	try
	{
		if(version == 0 && !_mergedModel.empty() && _mergedModel.cols>4 && _mergedModel.rows>4)
		{
//			imwrite("D:\\testimage\\Pim\\_version0.bmp",_mergedModel);
			cv::Rect re(_PIXEL_MARGIN,_PIXEL_MARGIN,_mergedModel.cols-(_PIXEL_MARGIN*2),_mergedModel.rows-(_PIXEL_MARGIN*2));
			_mergedModel = _mergedModel(re);
//			imwrite("D:\\testimage\\Pim\\_version1.bmp",_mergedModel);
		}
		else if(version == 0)
			return false;

		return true;
	}
	catch(...)
	{

		AfxMessageBox(_T("ImgModels, LoadFile fail!!"));
		return false;
	}
}

bool ImgModels::SaveFile_v2(CArchive& ar,int version)
{
	//shkim version1->2: matching image save
	if(_MatchingImg.empty())
	{
		ar << 0;
		ar << 0;
	}
	else
	{
		ar << _MatchingImg.cols;
		ar << _MatchingImg.rows;

		for (int y=0; y<_MatchingImg.rows; y++)
		{
			uchar * ptr = _MatchingImg.ptr(y);
			for (int x=0; x<_MatchingImg.cols; x++)
				ar << ptr[x];
		}
	}
	return true;
}

bool ImgModels::LoadFile_v2(CArchive& ar,int version)
{
	try
	{
		if(version>1)
		{
			int Wid(0), Len(0);
			ar >> Wid;
			ar >> Len;
			if(Wid>0 && Len>0)
			{
				_MatchingImg = cv::Mat(Len, Wid, CV_8UC1);
				for (int y=0; y<_MatchingImg.rows; y++)
				{
					uchar * ptr = _MatchingImg.ptr(y);
					for (int x=0; x<_MatchingImg.cols; x++)
						ar >> ptr[x];
				}
			}
		}
		else
		{
			_MatchingImg = _mergedModel;
		}

	}
	catch(...)
	{

		AfxMessageBox(_T("ImgModels, LoadFile fail!!"));
		return false;
	}
}
bool ImgModels::SaveFile_v3(CArchive& ar,int version)
{
	ar << _resolX;
	ar << _resolY;
	return true;
} 

bool ImgModels::LoadFile_v3(CArchive & ar, int version,double resX,double resY)
{
	if(version>2)
	{
		ar >> _resolX;
		ar >> _resolY;

		reSizeModel(resX,resY);
	}
	else
	{
		_resolX = 0;
		_resolY = 0;
	}

	return true;
}