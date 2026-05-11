#pragma once

#include <opencv2/opencv.hpp>
#include <math.h>
#include "InspParamDef.h"
#include <memory>
#include <map>

#define _PIXEL_MARGIN 30


#define GeoCharDivC			2
#define GeoCharDivR			2
#define GeoCharDivCnt		GeoCharDivC * GeoCharDivR
#define GeoCharLenByRow		12
#define	GeoCharRow			1
#define GeoCharCount		GeoCharLenByRow * GeoCharRow
#define GeoEpsilon			0.0000001
#define EPSILON 0.000001

class ImgModels
{
public:
	ImgModels();
	virtual ~ImgModels();
	cv::Mat _mergedModel;
	int ModelCnt;

	CString _sLoadedPath;

	cv::Mat _MatchingImg;

	double _resolX;
	double _resolY;

	bool SaveFile(CString sPath);
	bool LoadFile(CString sPath,double resX,double resY);

	bool SaveFile(CArchive& ar);
	bool LoadFile(CArchive& ar,double resX,double resY);

	bool LoadFile_v1(CArchive& ar,int version);
	bool SaveFile_v1(CArchive& ar,int version);

	bool LoadFile_v2(CArchive& ar,int version);
	bool SaveFile_v2(CArchive& ar,int version);

	bool LoadFile_v3(CArchive & ar, int version,double resX,double resY);
	bool SaveFile_v3(CArchive& ar,int version);

	void setRes(double resX,double resY)
	{
		_resolX = resX;
		_resolY = resY;
	}
	void getRes(double* resX, double* resY)
	{
		*resX = _resolX;
		*resY = _resolY;
	}

	void reSizeModel(double resX,double resY)
	{
		if(resX==_resolX&&resY==_resolY)
			return;
		if(resX == 0 ||_resolX == 0 ||resY == 0 ||_resolY == 0 )
			return;

		int nSizeX = (int)(_mergedModel.cols*_resolX/resX);
		int nSizeY = (int)(_mergedModel.rows*_resolY/resY);

		cv::resize(_mergedModel,_mergedModel,cv::Size(nSizeX,nSizeY));
		cv::resize(_MatchingImg,_MatchingImg,cv::Size(nSizeX,nSizeY));
	}
};