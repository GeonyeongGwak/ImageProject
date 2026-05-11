#include "StdAfx.h"
#include "GeoMatch_Foot.h"
#include <opencv2/opencv.hpp>
#include <ppl.h>

#include "MPTI.h"


GeoModel_Foot::Item_Foot::Item_Foot(GeoModel * parent) : GeoModel::Item(parent)
{

}

void GeoModel_Foot::Item_Foot::CalcMask(cv::Mat mask, float cogX, float cogY, float imgAngle)
{
	int pyrStep(0);
	if(_PyrDown == enmDepth::spHalf)
		pyrStep = 1;
	else if(_PyrDown == enmDepth::spZoom)
		pyrStep = _ParentPtr->_pyrDownStep;

	float ang = _Angle - imgAngle;
	int scale = RounDF( pow(2.0, pyrStep) );

	float radian = -ang / 180.0f * M_PI;
	float sinth = sin(radian);
	float costh = cos(radian);

	float ct_x = _CogX;
	float ct_y = _CogY;
	float * coordX = _coordX;
	float * coordY = _coordY;
	BYTE * flags = _Flags;
	for (int m=0; m<_noOfCordinates; m++)
	{
		float newCoordX =-coordX[m] * (float)(scale);
		float newCoordY = coordY[m] * (float)(scale); 

		float curY =  (( newCoordY)*costh - ( newCoordX)*sinth);
		float curX = -(( newCoordY)*sinth + ( newCoordX)*costh);

		int px = RounDF(curX + cogX);
		int py = RounDF(curY + cogY);

		if(px<0 || px>=mask.rows || py<0 || py>=mask.cols)
			continue;

		bool bFound(false);
		for (int x=0; x<scale && bFound==false; x++)
		{
			if((px+x) >= mask.rows)
				continue;
			BYTE * mskPtr = mask.ptr(px+x);
			for (int y=0; y<scale && bFound==false; y++)
			{
				if((py+y) >= mask.cols)
					continue;
				BYTE MaskBits = mskPtr[py+y];
				if((MaskBits & enmPtDataModify::PdmMask) == enmPtDataModify::PdmMask)
					bFound = true;
			}
		}

		if(bFound)
			flags[m] |= enmPtDataFlag::PdfDelete;
		else
			flags[m] &=~enmPtDataFlag::PdfDelete;
	}
}
void GeoModel_Foot::Item_Foot::DrawMask(cv::Mat mask, float cogX, float cogY, float imgAngle)
{
	int pyrStep(0);
	if(_PyrDown == enmDepth::spHalf)
		pyrStep = 1;
	else if(_PyrDown == enmDepth::spZoom)
		pyrStep = _ParentPtr->_pyrDownStep;

	float ang = _Angle - imgAngle;
	int scale = RounDF( pow(2.0, pyrStep) );

	float radian = -ang / 180.0f * M_PI;
	float sinth = sin(radian);
	float costh = cos(radian);

	float ct_x = _CogX;
	float ct_y = _CogY;
	float * coordX = _coordX;
	float * coordY = _coordY;
	BYTE * flags = _Flags;
	for (int m=0; m<_noOfCordinates; m++)
	{
		float newCoordY = coordY[m] * (float)(scale);
		float newCoordX =-coordX[m] * (float)(scale); 

		float curY =  (( newCoordY)*costh - ( newCoordX)*sinth);
		float curX = -(( newCoordY)*sinth + ( newCoordX)*costh);

		int py = RounDF(curY + cogY);
		int px = RounDF(curX + cogX);

		if(px<0 || px>=mask.rows || py<0 || py>=mask.cols)
			continue;

		for (int x=0; x<scale; x++)
		{
			if((px+x) >= mask.rows)
				continue;
			BYTE * mskPtr = mask.ptr(px+x);
			for (int y=0; y<scale; y++)
			{
				if((py+y) >= mask.cols)
					continue;

				mskPtr[py+y] |= enmPtDataModify::PdmAlive;
			}
		}
	}
}


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


GeoResult_Foot::GeoResult_Foot()
{
}
GeoResult_Foot::GeoResult_Foot(int cnt)
	:GeoResult(cnt)
{
}
GeoResult_Foot::~GeoResult_Foot()
{
}

void GeoResult_Foot::Alloc(int cnt)
{
	if(cnt<0)
		return;
	GeoResult::Alloc(cnt);

	_Score.resize(cnt);
}
void GeoResult_Foot::Free()
{
	GeoResult::Free();

	_Score.clear();
}



// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


GeoModel_Foot::GeoModel_Foot(void)
{
}
GeoModel_Foot::~GeoModel_Foot(void)
{
}

void GeoModel_Foot::SetDivisionParam(fileAlgoPath* sfileAlgoPath)
{
}
void GeoModel_Foot::CopyOf(GeoModel_Foot & md, float fImgAngle)
{
	GeoModel::CopyOf(md, fImgAngle);

	if(md._Mask.empty()==false)
	{
		_Mask.release();
		cv::Mat mask = _GetAngle(md._Mask, fImgAngle, false);
		SetMaskImage(mask);
	}
	FootType = md.FootType;
	WedgeArea.x =  md.WedgeArea.x;
	WedgeArea.y = md.WedgeArea.y;
	WedgeArea.width = md.WedgeArea.width;
	WedgeArea.height = md.WedgeArea.height;
	FootDirection = md.FootDirection;
	FootDBC = md.FootDBC;
}
void GeoModel_Foot::Alloc(cv::Mat src, float fImgAngle)
{
	GeoModel::Alloc(src, fImgAngle);
	
	_Mask = cv::Mat::zeros(_srcModel.rows, _srcModel.cols, CV_8UC1);
}
void GeoModel_Foot::SetMaskImage(cv::Mat mask)
{
	if(getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
		return ;

	if(_Mask.empty()==true)
		_Mask = cv::Mat::zeros(_srcModel.rows, _srcModel.cols, _srcModel.type());

	if(_Mask.rows != mask.rows || _Mask.cols != mask.cols || _Mask.type() != mask.type())
		return;

	_Mask = mask & 0x01;

	typItemPtr osoaOrgItem;
	for (auto pos=_Models[enmDepth::spOrg].begin(); pos!=_Models[enmDepth::spOrg].end(); pos++)
	{
		float ang = pos->second->_Angle - _fImgAngle;
		if(fabs(ang) < 0.0001)
		{
			osoaOrgItem = pos->second;
			break;
		}
	}

	for (auto pos=_Models[enmDepth::spOrg].begin(); pos!=_Models[enmDepth::spOrg].end() && osoaOrgItem!=nullptr; pos++)
		((GeoModel_Foot::Item_Foot *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
	for (auto pos=_Models[enmDepth::spZoom].begin(); pos!=_Models[enmDepth::spZoom].end() && osoaOrgItem!=nullptr; pos++)
		((GeoModel_Foot::Item_Foot *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
	for (auto pos=_Models[enmDepth::spHalf].begin(); pos!=_Models[enmDepth::spHalf].end() && osoaOrgItem!=nullptr; pos++)
		((GeoModel_Foot::Item_Foot *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
}
cv::Mat GeoModel_Foot::GetMaskImage()
{
	cv::Mat mask;
	if(_Mask.empty()==true)
		_Mask = cv::Mat::zeros(_srcModel.rows, _srcModel.cols, CV_8UC1);

	if(getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
		return _Mask;

	mask = _Mask.clone();

	typItemPtr osoaOrgItem;
	for (auto pos=_Models[enmDepth::spOrg].begin(); pos!=_Models[enmDepth::spOrg].end(); pos++)
	{
		float ang = pos->second->_Angle - _fImgAngle;
		if(ang == 0.0)
		{
			osoaOrgItem = pos->second;
			break;
		}
	}

	for (auto pos=_Models[enmDepth::spOrg].begin(); pos!=_Models[enmDepth::spOrg].end() && osoaOrgItem!=nullptr; pos++)
		((GeoModel_Foot::Item_Foot *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
	for (auto pos=_Models[enmDepth::spZoom].begin(); pos!=_Models[enmDepth::spZoom].end(); pos++)
		((GeoModel_Foot::Item_Foot *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
	for (auto pos=_Models[enmDepth::spHalf].begin(); pos!=_Models[enmDepth::spHalf].end(); pos++)
		((GeoModel_Foot::Item_Foot *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);

	

	if(osoaOrgItem!=nullptr)
		((GeoModel_Foot::Item_Foot *)(osoaOrgItem.get()))->DrawMask(mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);

	return mask;
}

bool GeoModel_Foot::_SaveFile_v3(CArchive & ar)
{
	if(_Mask.empty()==false)
	{
		ar << _Mask.rows;
		ar << _Mask.cols;
		ar << _Mask.type();

		for (int y=0; y<_Mask.rows; y++)
		{
			uchar * ptr = _Mask.ptr(y);
			for (int x=0; x<_Mask.cols; x++)
			{
				ar << ptr[x];
			}
		}
	}
	else
	{
		int nZero(0); 
		ar << nZero;
		ar << nZero;
		ar << nZero;
	}

	GeoModel::_SaveFile_v3(ar);

	return true;
}
bool GeoModel_Foot::_LoadFile_v3(CArchive & ar, int ver)
{
	int nMaskWid(0), nMaskLen(0), nMaskType(0); 
	if(ver>=3 && ar.IsBufferEmpty()==FALSE)
		ar >> nMaskLen;
	else
		nMaskLen = _srcModel.rows;
	if(ver>=3 && ar.IsBufferEmpty()==FALSE)
		ar >> nMaskWid;
	else
		nMaskWid = _srcModel.cols;
	if(ver>=3 && ar.IsBufferEmpty()==FALSE)
		ar >> nMaskType;
	else
		nMaskType = CV_8UC1;

	if(nMaskLen > 0 && nMaskWid > 0)
	{
		_Mask = cv::Mat::zeros(nMaskLen, nMaskWid, nMaskType);
		for (int y=0; y<_Mask.rows; y++)
		{
			uchar * ptr = _Mask.ptr(y);
			for (int x=0; x<_Mask.cols; x++)
			{
				if(ver>=3 && ar.IsBufferEmpty()==FALSE)
					ar >> ptr[x];
				else
					ptr[x] = 0;
			}
		}
	}

	GeoModel::_LoadFile_v3(ar, ver);

	return true;
}
bool GeoModel_Foot::_SaveFile_v8(CArchive & ar)
{
	ar << FootType;
	ar << WedgeArea.x;
	ar << WedgeArea.y;
	ar << WedgeArea.width;
	ar << WedgeArea.height;
	ar << FootDirection;
	return true;
}

bool GeoModel_Foot::_LoadFile_v8(CArchive & ar, int version)
{
	if (version < 8)
		return false;
	if (ar.IsBufferEmpty() == FALSE)
	{
		ar >> FootType;
		if (ar.IsBufferEmpty() == TRUE)
			return false;
		ar >> WedgeArea.x;
		ar >> WedgeArea.y;
		ar >> WedgeArea.width;
		ar >> WedgeArea.height;
		ar >> FootDirection;
	}
	return true;
}
bool GeoModel_Foot::_SaveFile_v11(CArchive & ar)
{
	ar << FootDBC;
	return true;
}
bool GeoModel_Foot::_LoadFile_v11(CArchive & ar, int version)
{
	if (version < 11) FootDBC = 0;
	else ar >> FootDBC;
	return true;
}
std::shared_ptr<GeoModel::Item> GeoModel_Foot::CreateItem()
{
	return std::shared_ptr<Item>(new GeoModel_Foot::Item_Foot(this));
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
void GeoMatch_Foot::SetImgProcess(std::shared_ptr<ImgProcessing> ProcImg)
{
	_ProcImg = ProcImg;
}