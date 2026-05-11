#include "StdAfx.h"
#include "GeoMatch_Pattern.h"
#include <opencv2/opencv.hpp>
#include <ppl.h>

#include "MPTI.h"


GeoModel_Pattern::Item_Pattern::Item_Pattern(GeoModel * parent) : GeoModel::Item(parent)
{

}

void GeoModel_Pattern::Item_Pattern::CalcMask(cv::Mat mask, float cogX, float cogY, float imgAngle)
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
void GeoModel_Pattern::Item_Pattern::DrawMask(cv::Mat mask, float cogX, float cogY, float imgAngle)
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


GeoResult_Pat::GeoResult_Pat()
{
}
GeoResult_Pat::GeoResult_Pat(int cnt)
	:GeoResult(cnt)
{
}
GeoResult_Pat::~GeoResult_Pat()
{
}

void GeoResult_Pat::Alloc(int cnt)
{
	if(cnt<0)
		return;
	GeoResult::Alloc(cnt);

	_Score.resize(cnt);
}
void GeoResult_Pat::Free()
{
	GeoResult::Free();

	_Score.clear();
}



// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


GeoModel_Pattern::GeoModel_Pattern(void)
{
	g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
}
GeoModel_Pattern::~GeoModel_Pattern(void)
{
	g_pMManager->pem_delete_check(this);
}

void GeoModel_Pattern::SetDivisionParam(fileAlgoPath* sfileAlgoPath)
{
	_DParam.SetDivParam(sfileAlgoPath);
}
void GeoModel_Pattern::CopyOf(GeoModel_Pattern & md, float fImgAngle)
{
	_DParam.CopyOf(md._DParam);

	GeoModel::CopyOf(md, fImgAngle);

	if(md._Mask.empty()==false)
	{
		_Mask.release();
		cv::Mat mask = _GetAngle(md._Mask, fImgAngle, false);
		SetMaskImage(mask);
	}
}
void GeoModel_Pattern::Alloc(cv::Mat src, float fImgAngle)
{
	GeoModel::Alloc(src, fImgAngle);
	
	_Mask = cv::Mat::zeros(_srcModel.rows, _srcModel.cols, CV_8UC1);
}
void GeoModel_Pattern::SetMaskImage(cv::Mat mask)
{
	if(getMatchAlgo() == GeoModel::enmMatchAlgo::agImage)
		return ;

	if(_Mask.empty()==true)
		_Mask = cv::Mat::zeros(_srcModel.rows, _srcModel.cols, _srcModel.type());
	if(_srcModel.rows != mask.rows || _srcModel.cols != mask.cols || _srcModel.type() != mask.type())
		mask = cv::Mat::zeros(_srcModel.rows, _srcModel.cols, _srcModel.type());

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
		((GeoModel_Pattern::Item_Pattern *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
	for (auto pos=_Models[enmDepth::spZoom].begin(); pos!=_Models[enmDepth::spZoom].end() && osoaOrgItem!=nullptr; pos++)
		((GeoModel_Pattern::Item_Pattern *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
	for (auto pos=_Models[enmDepth::spHalf].begin(); pos!=_Models[enmDepth::spHalf].end() && osoaOrgItem!=nullptr; pos++)
		((GeoModel_Pattern::Item_Pattern *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
}
cv::Mat GeoModel_Pattern::GetMaskImage()
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
		((GeoModel_Pattern::Item_Pattern *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
	for (auto pos=_Models[enmDepth::spZoom].begin(); pos!=_Models[enmDepth::spZoom].end(); pos++)
		((GeoModel_Pattern::Item_Pattern *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);
	for (auto pos=_Models[enmDepth::spHalf].begin(); pos!=_Models[enmDepth::spHalf].end(); pos++)
		((GeoModel_Pattern::Item_Pattern *)(pos->second.get()))->CalcMask(_Mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);

	

	if(osoaOrgItem!=nullptr)
		((GeoModel_Pattern::Item_Pattern *)(osoaOrgItem.get()))->DrawMask(mask, osoaOrgItem->_CogX, osoaOrgItem->_CogY, _fImgAngle);

	return mask;
}

bool GeoModel_Pattern::_SaveFile_v3(CArchive & ar)
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
bool GeoModel_Pattern::_LoadFile_v3(CArchive & ar, int ver)
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

bool GeoModel_Pattern::_SaveFile_v5(CArchive & ar)
{
	if(_srcModel_color.empty() == false)
	{
		ar<<_srcModel_color.cols;
		ar<<_srcModel_color.rows;
		ar<<_srcModel_color.channels();
	
		for (int y=0; y<_srcModel_color.rows; y++)
		{
			uchar * ptr = _srcModel_color.ptr(y);
			for (int x=0; x<_srcModel_color.cols * _srcModel_color.channels(); x++)
				ar << ptr[x];
		}
		ar<<_LightNumber;
	}
	else
	{
		ar<<0;
		ar<<0;
		ar<<1;
		ar<<_LightNumber;
	}

	return true;
}

bool GeoModel_Pattern::_LoadFile_v5(CArchive & ar, int version)
{
	int nchannel = 1;
	int width = 0;
	int height = 0;
	int nLight_num = 0;

	if(version >= 5 && ar.IsBufferEmpty() == FALSE)
		ar>>width;
	if(version >= 5 && ar.IsBufferEmpty() == FALSE)
		ar>>height;
	if(version >= 5 && ar.IsBufferEmpty() == FALSE)
		ar>>nchannel;
	//4 이상은 파일이 깨진것으로 판단.
	if (nchannel == 3)
	{
		setModelnChannel(nchannel);

		if (width > 0 && height > 0 && !_srcModel.empty() && width == _srcModel.cols && height == _srcModel.rows)
		{
			_srcModel_color = cv::Mat::zeros(height, width, CV_MAKETYPE(CV_8U, nchannel));
			for (int y = 0; y < _srcModel_color.rows && ar.IsBufferEmpty() == FALSE; y++)
			{
				uchar * ptr = _srcModel_color.ptr(y);
				for (int x = 0; x < _srcModel_color.cols*nchannel && ar.IsBufferEmpty() == FALSE; x++)
				{
					if (version >= 5 && ar.IsBufferEmpty() == FALSE)
						ar >> ptr[x];
				}
			}
		}
	}
	else
		setModelnChannel(1);
	if(version >= 5 && ar.IsBufferEmpty() == FALSE)
	{
		ar>>_LightNumber;
		setLightbt(_LightNumber);
	}
	return true;
}
std::shared_ptr<GeoModel::Item> GeoModel_Pattern::CreateItem()
{
	return std::shared_ptr<Item>(new GeoModel_Pattern::Item_Pattern(this));
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =


void GeoMatch_Pattern::FindModel_Div(GeoModel_Pattern & model, GeoMatchParam param, GeoResult_Pat & result)
{
	_SearchParam = std::shared_ptr<GeoMatchParam>( new GeoMatchParam(param) );

	FindModel( model, result,_SearchParam->_nAreaCnt_C<2);

	if (_SearchParam->_nAreaCnt_C > 1 || _SearchParam->_nAreaCnt_R > 1 || _SearchParam->_bUseCharDiv)
	//if (model.getMatchAlgo() != GeoModel::enmMatchAlgo::agImage)
	{
		if(_SearchParam->_bDivDetailSearch)
			_CalcDivScore(*_ProcImg->_OrgImage, model._Models[GeoModel::enmDepth::spOrg], result, model.getMatchAlgo(),_SearchParam->_bDivDetailSearch);
		else
			CalcDivScore_detail(*_ProcImg->_OrgImage, model, result, model.getMatchAlgo(),_SearchParam->_bDivDetailSearch);
	}

	_SearchParam.reset();
}
void GeoMatch_Pattern::_FindModel_DivSearch(cv::Mat src, GeoModel::typItemPtr ModelItem, int idx, cv::Point SStart, double minScore, double greediness, GeoResult_Pat & Result, int use_algo_model, bool DitailSearch, bool searchDiv)
{
	if(ModelItem != nullptr)
	{
		int noOfCordinates = ModelItem->_noOfCordinates;
		if (use_algo_model == GeoModel::enmMatchAlgo::agImage)
			noOfCordinates = 1;
		if(noOfCordinates <= 0)
		{
			_DivResBuf.clear();
			return;
		}
		cv::Mat Sdl = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdl;
		cv::Mat Sdx = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdx;
		cv::Mat Sdy = _ProcImg->_ProcBuf[ModelItem->_PyrDown].Sdy;
		float * matGradMag = _ProcImg->_ProcBuf[ModelItem->_PyrDown].matGradMag;

		float normMinScore = minScore / noOfCordinates;
		float normGreediness = ((1-greediness * minScore) / (1-greediness)) / noOfCordinates;

		_DivResBuf.clear();
		_DivResBuf.resize(noOfCordinates);

		int Id(1);
		cv::Point pStart;
		pStart.x = SStart.x;
		pStart.y = SStart.y;
		int nMarginX(0);

		//shkim inspection
		if(_SearchParam->_nAreaCnt_C>1 || _SearchParam->_nAreaCnt_R == 1)
		{
			if(!DitailSearch)
			{
				if(_SearchParam->_nAreaCnt_C > 1&& _SearchParam->_ScoreList[1]==0.0)
					nMarginX = _SearchParam->_AreaList[1].right - _SearchParam->_AreaList[1].left;
			}
			//for(int iy = 0;iy<_SearchParam->_nAreaCnt_C; iy++)
			Concurrency::parallel_for((size_t)0, (size_t)_SearchParam->_nAreaCnt_C, [&](size_t iy)
			{
				int Id_Local = iy +1;
				int nMarginX_Local =0;
				if(!DitailSearch)
				{
					nMarginX_Local = nMarginX;
					if(iy > 0 && iy == _SearchParam->_nAreaCnt_C-1 && _SearchParam->_ScoreList[iy-1]==0.0)
						nMarginX_Local = _SearchParam->_AreaList[iy-1].right - _SearchParam->_AreaList[iy-1].left;
				}
				//Result._Score[i]._DivArea
				if(use_algo_model==GeoModel::enmMatchAlgo::agIntaglio || use_algo_model==GeoModel::enmMatchAlgo::agRelief )
					Result._Score[idx]._Offset[iy] = _CalcCoefficientImg_DivSearch_Divide_sse(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, pStart, ModelItem, normMinScore, normGreediness, Id_Local, nMarginX_Local,Result._Score[idx].Score[iy], Result._Score[idx].Count[iy],DitailSearch);
				else if (use_algo_model == GeoModel::enmMatchAlgo::agImage)
					Result._Score[idx]._Offset[iy] = _CalcImageMatching(src, Result._Score[idx]._DivArea[iy], Result._Score[idx]._DivSubArea[iy], pStart, ModelItem, nMarginX_Local, Result._Score[idx].Score[iy], Result._Score[idx].Count[iy], DitailSearch);
				else
					Result._Score[idx]._Offset[iy] = _CalcCoefficient_DivSearch_Divide(Sdx, Sdy, matGradMag, pStart, ModelItem, normMinScore, normGreediness, Id_Local, nMarginX_Local,DitailSearch);
			}
			);
			Id = _SearchParam->_nAreaCnt_C;
		}


//		for(int iy = 0; iy<_SearchParam->_nAreaCnt_C; iy++)
//		{
//// 			if(searchDiv)//shkim 
//// 			{
//// 				pStart.x = SStart.x + Result._Score[idx]._Offset[iy].y;
//// 				pStart.y = SStart.y + Result._Score[idx]._Offset[iy].x;
//// 			}
//// 			else 
//			if(!DitailSearch)
//			{
//				if(iy == 0 && _SearchParam->_ScoreList[iy+1]==0.0)
//					nMarginX = _SearchParam->_AreaList[iy+1].right - _SearchParam->_AreaList[iy+1].left;
//				else if(iy == _SearchParam->_nAreaCnt_C-1 && _SearchParam->_ScoreList[iy-1]==0.0)
//					nMarginX = _SearchParam->_AreaList[iy-1].right - _SearchParam->_AreaList[iy-1].left;
//			}
//			
//			if(use_algo_model==GeoModel::enmMatchAlgo::agIntaglio || use_algo_model==GeoModel::enmMatchAlgo::agRelief)
//			{
//				//Result._Score[idx]._Offset[iy] = _CalcCoefficientImg_DivSearch_Divide(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, pStart, ModelItem, normMinScore, normGreediness, Id, nMarginX, Result._Score[idx].Score[iy], Result._Score[idx].Count[iy],DitailSearch);
//				Result._Score[idx]._Offset[iy] = _CalcCoefficientImg_DivSearch_Divide_sse(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, pStart, ModelItem, normMinScore, normGreediness, Id, nMarginX, Result._Score[idx].Score[iy], Result._Score[idx].Count[iy],DitailSearch);
//			}
//			else
//				Result._Score[idx]._Offset[iy] = _CalcCoefficient_DivSearch_Divide(Sdx, Sdy, matGradMag, pStart, ModelItem, normMinScore, normGreediness, Id, nMarginX,DitailSearch);
//			Id++;
//		}

		//row 
		if(_SearchParam->_nAreaCnt_R>1)
		{
			if (!DitailSearch)
			{
				if (_SearchParam->_nAreaCnt_R > 1 && _SearchParam->_ScoreList[1] == 0.0)
					nMarginX = _SearchParam->_AreaList[1].right - _SearchParam->_AreaList[1].left;
			}
			//for (int iy = 0; iy < _SearchParam->_nAreaCnt_R; iy++)
			Concurrency::parallel_for((size_t)0, (size_t)_SearchParam->_nAreaCnt_R, [&](size_t iy)
			{
				int Id_Local = iy + 1;
				int nMarginX_Local = 0;
				if (!DitailSearch)
				{
					nMarginX_Local = nMarginX;
					if (iy > 0 && iy == _SearchParam->_nAreaCnt_R - 1 && _SearchParam->_ScoreList[iy - 1] == 0.0)
						nMarginX_Local = _SearchParam->_AreaList[iy - 1].right - _SearchParam->_AreaList[iy - 1].left;
				}

				if (use_algo_model == GeoModel::enmMatchAlgo::agIntaglio || use_algo_model == GeoModel::enmMatchAlgo::agRelief)
					Result._Score[idx]._Offset[iy] = _CalcCoefficientImg_DivSearch_Divide_sse(Sdl, _ProcImg->_ProcBuf[ModelItem->_PyrDown].Integral, pStart, ModelItem, normMinScore, normGreediness, Id_Local, nMarginX_Local, Result._Score[idx].Score[iy], Result._Score[idx].Count[iy], DitailSearch);
				else if (use_algo_model == GeoModel::enmMatchAlgo::agImage)
					Result._Score[idx]._Offset[iy] = _CalcImageMatching(src, Result._Score[idx]._DivArea[iy], Result._Score[idx]._DivSubArea[iy], pStart, ModelItem, nMarginX_Local, Result._Score[idx].Score[iy], Result._Score[idx].Count[iy], DitailSearch);
				else
					Result._Score[idx]._Offset[iy] = _CalcCoefficient_DivSearch_Divide(Sdx, Sdy, matGradMag, pStart, ModelItem, normMinScore, normGreediness, Id_Local, nMarginX_Local, DitailSearch);
			}
			);
			Id = _SearchParam->_nAreaCnt_R;
		}

	}
}

cv::Point2i GeoMatch_Pattern::_CalcCoefficient_DivSearch_Divide(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, bool DitailSearch)
{
	if(ModelItem->isProcessed() == false)
		return cv::Point2i();

	float minScore(0.0f);

	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	int * pId = ModelItem->_DivisionId;
	BYTE * pFlags = ModelItem->_Flags;


	float * pResBuf = &_DivResBuf[0];

	int ImgWid = Sdx.cols;
	int ImgLen = Sdx.rows;

	int j = SStart.x;
	int i = SStart.y;

	int nMargin(1);
	nMargin = DitailSearch?1:3;

	int sy = ModelItem->_modelWidth / 80 + nMargin + nMarginX;
	int sx = ModelItem->_modelHeight / 80 + nMargin;
	int Cols = sy*2+1;
	int Rows = sx*2+1;
	int bufSz = Rows * Cols;

	//std::vector<float> * DivResSearch = new std::vector<float>[bufSz];
	//float * SubScoreList = new float[bufSz];
	std::vector<float> * DivResSearch = g_pMManager->pem_new<std::vector<float>>(true, bufSz, (PCHAR)__FUNCTION__, __LINE__);
	float * SubScoreList = g_pMManager->pem_new<float>(true, bufSz, (PCHAR)__FUNCTION__, __LINE__);

	double dMaxScore(-HUGE_VAL);
	int nMaxYIdx(0), nMaxXIdx(0), nMaxYOfs(0), nMaxXOfs(0); 
	for (int ix=-sx, ixn=0; ix<=sx; ix++, ixn++)
	{
		for (int iy=-sy, iyn=0; iy<=sy; iy++, iyn++)
		{
			int nCalcCnt(0);
			SubScoreList[ixn*Cols+iyn] = 0;
			DivResSearch[ixn*Cols+iyn].resize(noOfCordinates);

			float * pDivResBuf = &DivResSearch[ixn*Cols+iyn][0];
			double partialSum(0);

			for(int m=0;m<noOfCordinates;m++)
			{
				if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;

				if(pId[m] != Id)
					continue;

				nCalcCnt++;

				int curX = ix + i + pCoordX[m];
				int curY = iy + j + pCoordY[m];
				float iTx = pEdgeX[m];
				float iTy = pEdgeY[m];
				float iMag = pMag[m];

				if(curX<0 || curY<0 || curX>ImgLen-1 || curY>ImgWid-1)
					continue;

				short *_Sdx = Sdx.ptr<short>(curX);
				short *_Sdy = Sdy.ptr<short>(curX);

				float iSx =_Sdx[curY];
				float iSy =_Sdy[curY];

				double partial(0);
				if((iSx!=0 || iSy!=0) && (iTx!=0 || iTy!=0))
					partial = ((iSx*iTx)+(iSy*iTy)) * (iMag*matGradMag[curX*ImgWid+curY]);
				partialSum += partial;
				pDivResBuf[m] = partial;
			}

			SubScoreList[ixn*Cols+iyn] = partialSum / (double)(nCalcCnt);

			if(dMaxScore < SubScoreList[ixn*Cols+iyn])
			{
				dMaxScore = SubScoreList[ixn*Cols+iyn];
				nMaxYIdx = iyn;
				nMaxXIdx = ixn;
				nMaxYOfs = iy;
				nMaxXOfs = ix;
				
			}


		}
	}

	for(int m=0;m<noOfCordinates;m++)
	{
		if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			continue;

		if(pId[m] != Id)
			continue;
		pResBuf[m] = DivResSearch[nMaxXIdx*Cols+nMaxYIdx][m];
	}

	/*delete[] DivResSearch;
	delete[] SubScoreList;*/
	g_pMManager->pem_delete(DivResSearch, true);
	g_pMManager->pem_delete(SubScoreList, true);

	return cv::Point2i(nMaxXOfs, nMaxYOfs);
}
cv::Point2i GeoMatch_Pattern::_CalcCoefficientLap_DivSearch_Divide(cv::Mat & Sdl, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, bool DitailSearch )
{
	if(ModelItem->isProcessed() == false)
		return cv::Point2i();
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	int * pId = ModelItem->_DivisionId;
	BYTE * pFlags = ModelItem->_Flags;
	float * pResBuf = &_DivResBuf[0];
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int j = SStart.x;
	int i = SStart.y;

	int nMargin(1);
	nMargin = DitailSearch?1:3;
	int sy = ModelItem->_modelWidth / 80 + nMargin + nMarginX;
	int sx = ModelItem->_modelHeight / 80 + nMargin;
	int Cols = sy*2+1;
	int Rows = sx*2+1;
	int bufSz = Rows * Cols;
	/*std::vector<float> * DivResSearch = new std::vector<float>[bufSz];
	float * SubScoreList = new float[bufSz];*/
	std::vector<float> * DivResSearch = g_pMManager->pem_new<std::vector<float>>(true, bufSz, (PCHAR)__FUNCTION__, __LINE__);
	float * SubScoreList = g_pMManager->pem_new<float>(true, bufSz, (PCHAR)__FUNCTION__, __LINE__);
	double dMaxScore(-HUGE_VAL);
	int nMaxYIdx(0), nMaxXIdx(0), nMaxYOfs(0), nMaxXOfs(0); 
	for (int ix=-sx, ixn=0; ix<=sx; ix++, ixn++)
	{
		for (int iy=-sy, iyn=0; iy<=sy; iy++, iyn++)
		{
			int nCalcCnt(0);
			SubScoreList[ixn*Cols+iyn] = 0;
			DivResSearch[ixn*Cols+iyn].resize(noOfCordinates);
			float * pDivResBuf = &DivResSearch[ixn*Cols+iyn][0];
			double partialSum(0);
			for(int m=0;m<noOfCordinates;m++)
			{
				if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
					continue;
				if(pId[m] != Id)
					continue;
				int curX = ix + i + pCoordX[m];
				int curY = iy + j + pCoordY[m];
				float iTl = pEdgeX[m];
				float iMag = pMag[m];

				nCalcCnt++;
				if(curX<0 || curY<0 || curX>ImgLen-1 || curY>ImgWid-1)
					continue;

				BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
				float iSl =_Sdl[curY];
				double partial(0);
				if((iSl!=0 ) && (iTl!=0))
					partial = ((iSl*iTl)) * (iMag*matGradMag[curX*ImgWid+curY]);
				partialSum += partial;
				pDivResBuf[m] = partial;
			}
			SubScoreList[ixn*Cols+iyn] = partialSum / (double)(nCalcCnt);
			if(dMaxScore < SubScoreList[ixn*Cols+iyn])
			{
				dMaxScore = SubScoreList[ixn*Cols+iyn];
				nMaxYIdx = iyn;
				nMaxXIdx = ixn;
				nMaxYOfs = iy;
				nMaxXOfs = ix;
			}
		}
	}
	for(int m=0;m<noOfCordinates;m++)
	{
		if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
			continue;
		if(pId[m] != Id)
			continue;
		pResBuf[m] = DivResSearch[nMaxXIdx*Cols+nMaxYIdx][m];
	}
	/*delete[] DivResSearch;
	delete[] SubScoreList;*/
	g_pMManager->pem_delete(DivResSearch, true);
	g_pMManager->pem_delete(SubScoreList, true);

	return cv::Point2i(nMaxXOfs, nMaxYOfs);
}
cv::Point2i GeoMatch_Pattern::_CalcCoefficientImg_DivSearch_Divide(cv::Mat & Sdl, cv::Mat & Integ, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, float * dRetScore, int * dRetCount , bool DitailSearch)
{
	if(ModelItem->isProcessed() == false)
		return cv::Point2i();
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	int * pId = ModelItem->_DivisionId;
	int * pSubId = ModelItem->_SubDivisionId;
	BYTE * pFlags = ModelItem->_Flags;
	float * pResBuf = &_DivResBuf[0];
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int j = SStart.x;
	int i = SStart.y;

	int nMargin;
	nMargin = DitailSearch?1:3;

	int sy = ModelItem->_modelWidth / 80 + nMargin + nMarginX;
	int sx = ModelItem->_modelHeight / 80 + nMargin;
	int Cols = sy*2+1;
	int Rows = sx*2+1;
	int bufSz = Rows * Cols;

	bool bUseDivChar = _SearchParam->_bUseCharDiv;

	// 분할검사라 해도 평균은 모델 전체의 평균이어야 한다
	cv::Point2i MinPos=ModelItem->_MinPos, MaxPos=ModelItem->_MaxPos;
	int nMinX = j+MinPos.x;
	int nMaxX = j+MaxPos.x+1;
	int nMinY = i+MinPos.y;
	int nMaxY = i+MaxPos.y+1;

	if(nMinX<0)
		nMinX = 0;
	if(nMinY<0)
		nMinY = 0;
	if(nMaxX>=Integ.cols)
		nMaxX = Integ.cols-1;
	if(nMaxY>=Integ.rows)
		nMaxY = Integ.rows-1;

	if (nMaxX < 1 || nMaxY < 1 || nMinX > Integ.cols - 1 || nMinY >Integ.rows - 1)
	{
		for(int m=0;m<noOfCordinates;m++)
		{
			if(pId[m] != Id)
				continue;
			pResBuf[m] = 0;
		}

		return cv::Point2i();
	}

	int SzBox = (nMaxX-nMinX) * (nMaxY-nMinY);
	int * nMinIteg = Integ.ptr<int>(nMinY);
	int * nMaxIteg = Integ.ptr<int>(nMaxY);
	float ImgMean = (float)(nMaxIteg[nMaxX] - nMinIteg[nMaxX] - nMaxIteg[nMinX] + nMinIteg[nMinX]) / (float)(SzBox);
	float fModStdev = ModelItem->_Stdev;

	/*std::vector<float> * DivResSearch = new std::vector<float>[bufSz];
	float * SubScoreList = new float[bufSz];*/
	std::vector<float> * DivResSearch = g_pMManager->pem_new<std::vector<float>>(true, bufSz, (PCHAR)__FUNCTION__, __LINE__);
	float * SubScoreList = g_pMManager->pem_new<float>(true, bufSz, (PCHAR)__FUNCTION__, __LINE__);

	double dMaxScore(-HUGE_VAL);
	int nMaxYIdx(0), nMaxXIdx(0), nMaxYOfs(0), nMaxXOfs(0); 

	if(bUseDivChar == false)
	{
		for (int ix=-sx, ixn=0; ix<=sx; ix++, ixn++)
		{
			for (int iy=-sy, iyn=0; iy<=sy; iy++, iyn++)
			{
				int nCalcCnt(0);
				SubScoreList[ixn*Cols+iyn] = 0;
				DivResSearch[ixn*Cols+iyn].resize(noOfCordinates);
				float * pDivResBuf = &DivResSearch[ixn*Cols+iyn][0];
				double modelStdev(0);
				double partialStdev(0);
				double partialSum(0);
				for(int m=0;m<noOfCordinates;m++)
				{
					if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
						continue;
					if(pId[m] != Id)
						continue;

					nCalcCnt++;

					int curX = ix + i + pCoordX[m];
					int curY = iy + j + pCoordY[m];
					float iTl = pEdgeX[m];
					float iMag = pMag[m];

					if(curX<0 || curY<0 || curX>ImgLen-1 || curY>ImgWid-1)
					{
						pDivResBuf[m] = 0;
						continue;
					}


					BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
					float iSl = (float)(_Sdl[curY]) - ImgMean;

					double partial = ((iSl+GeoEpsilon) * (iTl+GeoEpsilon));

					partialSum += partial;
					partialStdev += (iSl * iSl);
					modelStdev += (iTl * iTl);

					pDivResBuf[m] = partial;
				}

				if(nCalcCnt > 0)
				{
					partialStdev /= nCalcCnt;
					modelStdev /= nCalcCnt;
					double DivVal = (sqrt(partialStdev)+GeoEpsilon) * (sqrt(modelStdev)+GeoEpsilon);
					SubScoreList[ixn*Cols+iyn] = (partialSum / DivVal) / nCalcCnt;
				}
				else
				{
					partialStdev = 0;
					modelStdev = 0;
					SubScoreList[ixn*Cols+iyn] = 0;
				}
				

				if(dMaxScore < SubScoreList[ixn*Cols+iyn])
				{
					if(dRetScore)
					{
						dRetCount[0] = nCalcCnt;
						dRetScore[0] = SubScoreList[ixn*Cols+iyn] * 100.0;
						if(dRetScore[0]>100.0)
						{
							dRetScore[0] = 100;
						}
					}

					dMaxScore = SubScoreList[ixn*Cols+iyn];
					nMaxYIdx = iyn;
					nMaxXIdx = ixn;
					nMaxYOfs = iy;
					nMaxXOfs = ix;
				}
			}
		}
	}
	else
	{
		for (int ix=-sx, ixn=0; ix<=sx; ix++, ixn++)
		{
			for (int iy=-sy, iyn=0; iy<=sy; iy++, iyn++)
			{
				double dMinSubScore(HUGE_VAL);
				double dSubScore[GeoCharDivCnt] = { 0.0, };
				int dSubCount[GeoCharDivCnt] = { 0, };
				for (int ic=0; ic<GeoCharDivCnt; ic++)
				{
					int nCalcCnt(0);
					SubScoreList[ixn*Cols+iyn] = 0;
					DivResSearch[ixn*Cols+iyn].resize(noOfCordinates);
					float * pDivResBuf = &DivResSearch[ixn*Cols+iyn][0];
					double modelStdev(0);
					double partialStdev(0);
					double partialSum(0);
					for(int m=0;m<noOfCordinates;m++)
					{
						if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
							continue;
						if((pId[m] != Id || pSubId[m] != (ic+1)))
							continue;

						int curX = ix + i + pCoordX[m];
						int curY = iy + j + pCoordY[m];
						float iTl = pEdgeX[m];
						float iMag = pMag[m];

						nCalcCnt++;

						if(curX<0 || curY<0 || curX>ImgLen-1 || curY>ImgWid-1)
						{
							continue;
						}


						BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
						float iSl =_Sdl[curY] - ImgMean;
						double partial = ((iSl+GeoEpsilon)*(iTl+GeoEpsilon));
						partialSum = partialSum + partial;
						partialStdev = partialStdev + (iSl * iSl);
						modelStdev = modelStdev + (iTl * iTl);
					}

					if(nCalcCnt>0)
					{
						partialStdev = partialStdev / nCalcCnt;
						modelStdev = modelStdev / nCalcCnt;
						double DivVal = (sqrt(partialStdev)+GeoEpsilon) * (sqrt(modelStdev)+GeoEpsilon);
						dSubScore[ic] = (partialSum / DivVal) / nCalcCnt;
						dSubCount[ic] = nCalcCnt;
						if(dMinSubScore > dSubScore[ic])
							dMinSubScore = dSubScore[ic];
					}
					else
					{
						dSubScore[ic] = 0;
						dSubCount[ic] = 0;
					}
				}

				SubScoreList[ixn*Cols+iyn] = dMinSubScore * 100.0;

				if(dMaxScore < SubScoreList[ixn*Cols+iyn])
				{
					if(dRetScore)
					{
						dSubCount;
						for (int iic=0; iic<GeoCharDivCnt; iic++)
						{
							dRetCount[iic] = dSubCount[iic];
							dRetScore[iic] = dSubScore[iic] * 100.0;
						}
					}
					dMaxScore = SubScoreList[ixn*Cols+iyn];
					nMaxYIdx = iyn;
					nMaxXIdx = ixn;
					nMaxYOfs = iy;
					nMaxXOfs = ix;
				}
			}
		}
	}

	/*delete[] DivResSearch;
	delete[] SubScoreList;*/
	g_pMManager->pem_delete(DivResSearch, true);
	g_pMManager->pem_delete(SubScoreList, true);

	return cv::Point2i(nMaxXOfs, nMaxYOfs);
}
cv::Point2i GeoMatch_Pattern::_CalcCoefficientImg_DivSearch_Divide_sse(cv::Mat & Sdl, cv::Mat & Integ, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, float * dRetScore, int * dRetCount , bool DitailSearch)
{
	if(ModelItem->isProcessed() == false)
		return cv::Point2i();
	float minScore(0.0f);
	int noOfCordinates = ModelItem->_noOfCordinates;
	float * pCoordX = ModelItem->_coordX;
	float * pCoordY = ModelItem->_coordY;
	float * pEdgeX = ModelItem->_edgeDerivativeX;
	float * pEdgeY = ModelItem->_edgeDerivativeY;
	float * pMag = ModelItem->_edgeMagnitude;
	int * pId = ModelItem->_DivisionId;
	int * pSubId = ModelItem->_SubDivisionId;
	BYTE * pFlags = ModelItem->_Flags;
	float * pResBuf = &_DivResBuf[0];
	int ImgWid = Sdl.cols;
	int ImgLen = Sdl.rows;
	int j = SStart.x;
	int i = SStart.y;

	int nMargin;
	nMargin = DitailSearch?1:3;

	int sy = ModelItem->_modelWidth / 80 + nMargin + nMarginX;;
	int sx = ModelItem->_modelHeight / 80 + nMargin;
	int Cols = sy*2+1;
	int Rows = sx*2+1;
	int bufSz = Rows * Cols;

	bool bUseDivChar = _SearchParam->_bUseCharDiv;

	// 분할검사라 해도 평균은 모델 전체의 평균이어야 한다
	cv::Point2i MinPos=ModelItem->_MinPos, MaxPos=ModelItem->_MaxPos;
	int nMinX = j+MinPos.x;
	int nMaxX = j+MaxPos.x+1;
	int nMinY = i+MinPos.y;
	int nMaxY = i+MaxPos.y+1;

	if(nMinX<0)
		nMinX = 0;
	if(nMinY<0)
		nMinY = 0;
	if(nMaxX>=Integ.cols)
		nMaxX = Integ.cols-1;
	if(nMaxY>=Integ.rows)
		nMaxY = Integ.rows-1;

	if(nMaxX < 1 || nMaxY < 1 || nMinX > Integ.cols - 1 || nMinY >Integ.rows - 1)
	{
		for(int m=0;m<noOfCordinates;m++)
		{
			if(pId[m] != Id)
				continue;
			pResBuf[m] = 0;
		}

		return cv::Point2i();
	}

	int SzBox = (nMaxX-nMinX) * (nMaxY-nMinY);
	int * nMinIteg = Integ.ptr<int>(nMinY);
	int * nMaxIteg = Integ.ptr<int>(nMaxY);
	float ImgMean = (float)(nMaxIteg[nMaxX] - nMinIteg[nMaxX] - nMaxIteg[nMinX] + nMinIteg[nMinX]) / (float)(SzBox);
	float fModStdev = ModelItem->_Stdev;

    //float * SubScoreList = new float[bufSz];
	float * SubScoreList = g_pMManager->pem_new<float>(true, bufSz, (PCHAR)__FUNCTION__, __LINE__);

	double dMaxScore(-HUGE_VAL);
	int nMaxYIdx(0), nMaxXIdx(0), nMaxYOfs(0), nMaxXOfs(0); 

	if(bUseDivChar == false)
	{
		std::vector<float> pEdgeX_ID;
		std::vector<std::pair<float,float>> pCoord_ID;

		int noOfCordinates_ID =0;

		int minModelX(INT_MAX) , minModelY(INT_MAX), maxModelX(INT_MIN), MaxModelY(INT_MIN);

		for(int m=0;m<noOfCordinates;m++)
		{	
			if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
				continue;
			if(pId[m] != Id)
				continue;

			noOfCordinates_ID++;

			if(pCoordX[m] < minModelX) minModelX = pCoordX[m];
			if(pCoordX[m] > maxModelX) maxModelX = pCoordX[m];
			if(pCoordY[m] < minModelY) minModelY = pCoordY[m];
			if(pCoordY[m] > MaxModelY) MaxModelY = pCoordY[m];

			pCoord_ID.emplace_back(std::make_pair(pCoordX[m],pCoordY[m])); 
			pEdgeX_ID.emplace_back(pEdgeX[m]);
		}

		//std::sort(pCoord_ID.begin(), pCoord_ID.end(), compPoint);

		for (int ix=-sx, ixn=0; ix<=sx; ix++, ixn++)
		{
			for (int iy=-sy, iyn=0; iy<=sy; iy++, iyn++)
			{
				int nCalcCnt(0);
				SubScoreList[ixn*Cols+iyn] = 0;

				double modelStdev(0);
				double partialStdev(0);
				double partialSum(0);
				
				if(true)
				//if(ix+minModelX<0 || iy+minModelY<0 || ix+maxModelX>ImgLen-1 || iy+MaxModelY>ImgWid-1) //기존
				{
					for(int m=0;m<noOfCordinates_ID;m++)
					{
						nCalcCnt++;

						int curX = ix + i + pCoord_ID[m].first;
						int curY = iy + j + pCoord_ID[m].second;
						float iTl = pEdgeX_ID[m];
					
						if(curX<0 || curY<0 || curX>ImgLen-1 || curY>ImgWid-1)
						{
							continue;
						}
					
						BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
						float iSl = (float)(_Sdl[curY]) - ImgMean;
						double partial = ((iSl+GeoEpsilon) * (iTl+GeoEpsilon));

						partialSum += partial;
						partialStdev += (iSl * iSl);			
						modelStdev += (iTl * iTl);
					}

				}
				else //sse 적용
				{
					//register __m128 _partialStdev=_mm_setzero_ps();
					//register __m128 _partialSum=_mm_setzero_ps();
					//register __m128 _modelStdev=_mm_setzero_ps();
				
					//__m128 _vMean=_mm_set_ps1(ImgMean);
					//__m128 vGeoEpsilon= _mm_set_ps1((float)GeoEpsilon);

					//int nNum =noOfCordinates_ID - (noOfCordinates_ID%4);
					//int m=0;
					//for(;m<nNum;m+=4) //noOfCordinates_ID
					//{
					//	nCalcCnt +=4;
					//	 __m128i _vSdI_=_mm_setzero_si128 ();
					//	 __m128  _vSdI=_mm_setzero_ps();
					//	 __m128  _viTl=_mm_setzero_ps();

					//	for(int n=0;n<4;n++)
					//	{
					//		int curX = ix + i + pCoord_ID[m+n].first;
					//		int curY = iy + j + pCoord_ID[m+n].second;
					//		BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
					//		_vSdI_.m128i_u8[n*4] = _Sdl[curY];

					//	}

					//	_vSdI = _mm_cvtepi32_ps(_vSdI_);
					//	_viTl = _mm_loadu_ps(&pEdgeX_ID[m]);
				
					//	_partialSum = _mm_add_ps(
					//		_partialSum,

					//		_mm_mul_ps(
					//			_mm_add_ps(
					//			_vSdI = _mm_sub_ps(
					//				_vSdI,_vMean), vGeoEpsilon),
					//			_mm_add_ps(_viTl, vGeoEpsilon)
					//		)
					//	);

					//	_partialStdev = _mm_add_ps(_partialStdev, _mm_mul_ps(_vSdI, _vSdI));
					//	_modelStdev = _mm_add_ps(_modelStdev, _mm_mul_ps(_viTl, _viTl));
				
					//}
				
					////나머지
					//int nIndex = 0;
					//for(;m<noOfCordinates_ID;m++)
					//{
					//	nCalcCnt++;

					//	int curX = ix + i + pCoord_ID[m].first;
					//	int curY = iy + j + pCoord_ID[m].second;
					//	float iTl = pEdgeX_ID[m];
					//	BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
					//	float iSl = (float)(_Sdl[curY]) - ImgMean;
					//	double partial = ((iSl+GeoEpsilon) * (iTl+GeoEpsilon));
					//	_partialSum.m128_f32[nIndex] = partial;
					//	_partialStdev.m128_f32[nIndex] = (iSl * iSl);			
					//	_modelStdev.m128_f32[nIndex] = (iTl * iTl);
					//	nIndex++;
					//}

					//for(int n=0;n<4;n++)
					//{
					//	partialStdev += _partialStdev.m128_f32[n];
					//	modelStdev += _modelStdev.m128_f32[n];
					//	partialSum += _partialSum.m128_f32[n];
					//}
				
				}


				if(nCalcCnt > 0)
				{
					partialStdev /= nCalcCnt;
					modelStdev /= nCalcCnt;
					double DivVal = (sqrt(partialStdev)+GeoEpsilon) * (sqrt(modelStdev)+GeoEpsilon);
					SubScoreList[ixn*Cols+iyn] = (partialSum / DivVal) / nCalcCnt;
				}
				else
				{
					partialStdev = 0;
					modelStdev = 0;
					SubScoreList[ixn*Cols+iyn] = 0;
				}
				

				if(dMaxScore < SubScoreList[ixn*Cols+iyn])
				{
					if(dRetScore)
					{
						dRetCount[0] = nCalcCnt;
						dRetScore[0] = SubScoreList[ixn*Cols+iyn] * 100.0;
						if(dRetScore[0]>100.0)
						{
							dRetScore[0] = 100;
						}
					}

					dMaxScore = SubScoreList[ixn*Cols+iyn];
					nMaxYIdx = iyn;
					nMaxXIdx = ixn;
					nMaxYOfs = iy;
					nMaxXOfs = ix;

				}
			}
		}
	}
	else
	{
		double dMinSubScore(HUGE_VAL);
		double dSubScore[GeoCharDivCnt] = { 0.0, };
		int dSubCount[GeoCharDivCnt] = { 0, };

		/*std::vector<std::pair<float,float>>* pCoord_SubID = new std::vector<std::pair<float,float>>[GeoCharDivCnt];
		std::vector<float> * pEdgeX_SubID = new std::vector<float>[GeoCharDivCnt];*/
		std::vector<std::pair<float, float>>* pCoord_SubID = g_pMManager->pem_new<std::vector<std::pair<float, float>>>(true, GeoCharDivCnt, (PCHAR)__FUNCTION__, __LINE__);
		std::vector<float> * pEdgeX_SubID = g_pMManager->pem_new<std::vector<float>>(true, GeoCharDivCnt, (PCHAR)__FUNCTION__, __LINE__);
		std::vector<int> noOfCordinates_SubID(GeoCharDivCnt);

		for(int m=0;m<noOfCordinates;m++)
		{	
			if((pFlags[m]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
				continue;
			if(pId[m] != Id)
				continue;

		    int indexID = pSubId[m]-1;
			if(indexID > GeoCharDivCnt-1) continue;
			if(indexID < 0) continue;
			
			pCoord_SubID[indexID].emplace_back(std::make_pair(pCoordX[m],pCoordY[m])); 
			pEdgeX_SubID[indexID].emplace_back(pEdgeX[m]);
		    ++noOfCordinates_SubID[indexID];
		}


		for (int ix=-sx, ixn=0; ix<=sx; ix++, ixn++)
		{
			for (int iy=-sy, iyn=0; iy<=sy; iy++, iyn++)
			{
				dMinSubScore = HUGE_VAL;
				int pos1 = ix + i;
				int pos2 = iy + j;

				SubScoreList[ixn*Cols+iyn] = 0;
				for (int ic=0; ic<GeoCharDivCnt; ic++)
				{
					dSubScore[ic] =0;
					dSubCount[ic] =0;

					int nCalcCnt(0);
					
					double modelStdev(0);
					double partialStdev(0);
					double partialSum(0);

					int noOfCordinates= noOfCordinates_SubID[ic];
					for(int m=0;m<noOfCordinates;m++)
					{
						
						int curX = pos1 + pCoord_SubID[ic][m].first;
						int curY = pos2 + pCoord_SubID[ic][m].second;
					
						nCalcCnt++;

						if(curX<0 || curY<0 || curX>ImgLen-1 || curY>ImgWid-1)
						{
							continue;
						}

						float iTl = pEdgeX_SubID[ic][m];
						
						BYTE *_Sdl = Sdl.ptr<BYTE>(curX);
						float iSl =(float)_Sdl[curY] - ImgMean;
						double partial = ((iSl+GeoEpsilon)*(iTl+GeoEpsilon));
						partialSum += partial;

						partialStdev += (iSl * iSl);
						modelStdev = modelStdev + (iTl * iTl);
					}

					if(nCalcCnt>0)
					{
						partialStdev = partialStdev / nCalcCnt;
						modelStdev = modelStdev / nCalcCnt;
						double DivVal = (sqrt(partialStdev)+GeoEpsilon) * (sqrt(modelStdev)+GeoEpsilon);
						dSubScore[ic] = (partialSum / DivVal) / nCalcCnt;
						dSubCount[ic] = nCalcCnt;
						if(dMinSubScore > dSubScore[ic])
							dMinSubScore = dSubScore[ic];
					}

				}

				SubScoreList[ixn*Cols+iyn] = dMinSubScore * 100.0;

				if(dMaxScore < SubScoreList[ixn*Cols+iyn])
				{
					if(dRetScore)
					{
						dSubCount;
						for (int iic=0; iic<GeoCharDivCnt; iic++)
						{
							dRetCount[iic] = dSubCount[iic];
							dRetScore[iic] = dSubScore[iic] * 100.0;
						}
					}
					dMaxScore = SubScoreList[ixn*Cols+iyn];
					nMaxYIdx = iyn;
					nMaxXIdx = ixn;
					nMaxYOfs = iy;
					nMaxXOfs = ix;

				}
			}
		}

		/*delete[] pCoord_SubID;
		delete[] pEdgeX_SubID;*/
		g_pMManager->pem_delete(pCoord_SubID, true);
		g_pMManager->pem_delete(pEdgeX_SubID, true);

	}

	//delete[] SubScoreList;
	g_pMManager->pem_delete(SubScoreList, true);


	return cv::Point2i(nMaxXOfs, nMaxYOfs);
}
cv::Point2i GeoMatch_Pattern::_CalcImageMatching(cv::Mat srcImage, cv::Point2d _DivArea[4], cv::Point2d _DivSubArea[4][4], cv::Point SStart, GeoModel::typItemPtr ModelItem, int nMarginX, float * dRetScore, int * dRetCount, bool DitailSearch)
{

#ifndef GeoNoUseWriteTimeLog
	CQTimer qtm;
	qtm.StartTick();
#endif
	GeoModel model = *ModelItem->_ParentPtr;

	int ImgWid = srcImage.cols;
	int ImgLen = srcImage.rows;
	int j = SStart.x;
	int i = SStart.y;

	int nMargin;
	nMargin = DitailSearch ? 1 : 3;

	int sy = ModelItem->_modelWidth / 80 + nMargin + nMarginX;;
	int sx = ModelItem->_modelHeight / 80 + nMargin;
	int Cols = sy * 2 + 1;
	int Rows = sx * 2 + 1;
	int bufSz = Rows * Cols;

	cv::Mat mdOrgImg = srcImage.channels() == 3 ? model.color_Image() : model.Image(), mdHalf, half;
	bool bUseDivChar = _SearchParam->_bUseCharDiv;
	cv::Point2i retPoint = cv::Point2i();
	if (bUseDivChar)//charDiv
	{
		int nPoCnt(0);
		for (int chIdx = 0; chIdx < GeoCharDivCnt; chIdx++)
		{
			cv::Mat RotmdImgIpp;
			unsigned char* userSrc = mdOrgImg.data;
			unsigned char* userDst = NULL;
			int dstSizeX = 0;
			int dstSizeY = 0;
			double dangle = ModelItem->_Angle;
			int orgSizeX = mdOrgImg.cols;
			int orgSizeY = mdOrgImg.rows;
			bool bLinear = true;
			bool bColor = mdOrgImg.channels() == 3;
			model.RotateImg_ipp2020(userSrc, &userDst, dangle, orgSizeX, orgSizeY, &dstSizeX, &dstSizeY, bColor, bLinear, false);

			if (mdOrgImg.channels() != 3)
				RotmdImgIpp = cv::Mat(dstSizeY, dstSizeX, CV_8UC1, userDst).clone();
			else
				RotmdImgIpp = cv::Mat(dstSizeY, dstSizeX, CV_8UC3, userDst).clone();

			if (userDst != NULL)
				g_pMManager->pem_delete(userDst, true);
			//mdOrgImg.size()
			int harfX = RounD(RotmdImgIpp.cols / 2);
			int harfY = RounD(RotmdImgIpp.rows / 2);
			int MnX = _DivSubArea[chIdx][0].x > _DivSubArea[chIdx][2].x ? _DivSubArea[chIdx][2].x + harfX : _DivSubArea[chIdx][0].x + harfX;
			int MnY = _DivSubArea[chIdx][0].y > _DivSubArea[chIdx][2].y ? _DivSubArea[chIdx][2].y + harfY : _DivSubArea[chIdx][0].y + harfY;
			int MxX = _DivSubArea[chIdx][0].x < _DivSubArea[chIdx][2].x ? _DivSubArea[chIdx][2].x + harfX : _DivSubArea[chIdx][0].x + harfX;
			int MxY = _DivSubArea[chIdx][0].y < _DivSubArea[chIdx][2].y ? _DivSubArea[chIdx][2].y + harfY : _DivSubArea[chIdx][0].y + harfY;
			if (MxX > RotmdImgIpp.cols)
				MxX = RotmdImgIpp.cols;
			if (MxY > RotmdImgIpp.rows)
				MxY = RotmdImgIpp.rows;
			if (MnX < 0)
				MnX = 0;
			if (MnY < 0)
				MnY = 0;
			int mdWidth = MxX - MnX;
			int mdHeight = MxY - MnY;
			if (mdWidth < 2 || mdHeight < 2)
				continue;
			cv::Mat mdImg = RotmdImgIpp(cv::Rect(MnX, MnY, mdWidth, mdHeight)).clone();//(cv::Rect(SStart.x, SStart.y, Cols+ ModelItem->_modelWidth, Rows + ModelItem->_modelHeight));
			
			//_GetPyrDown(mdImg, mdHalf);
			int stpointX = SStart.x - harfX - sx + MnX > 0 ? SStart.x - harfX - sx + MnX : 0;
			int stpointY = SStart.y - harfY - sy + MnY > 0 ? SStart.y - harfY - sy + MnY : 0;
			int width = stpointX + Cols + mdWidth < srcImage.cols ? Cols + mdWidth : srcImage.cols - stpointX;
			int height = stpointY + Rows + mdHeight < srcImage.rows ? Rows + mdHeight : srcImage.rows - stpointY;
			cv::Mat TargetImg = srcImage(cv::Rect(stpointX, stpointY, width, height)).clone();
			double scaleVal = 0.5;
			mdHalf = cv::Mat(mdImg.rows*scaleVal, mdImg.cols*scaleVal, CV_MAKETYPE(CV_8U, mdImg.channels()));
			cv::resize(mdImg, mdHalf, cv::Size(mdImg.cols*scaleVal, mdImg.rows*scaleVal));
			half = cv::Mat(TargetImg.rows*scaleVal, TargetImg.cols*scaleVal, CV_MAKETYPE(CV_8U, TargetImg.channels()));
			cv::resize(TargetImg, half, cv::Size(TargetImg.cols*scaleVal, TargetImg.rows*scaleVal));

#ifdef GeoSaveDebug
			cv::imwrite(std::string("d:\\target.bmp"), src);
			cv::imwrite(std::string("d:\\model.bmp"), mdImg);

			cv::imwrite(std::string("d:\\model_org.bmp"), src.channels() == 3 ? model.color_Image() : model.Image());
#endif
			// 
			cv::Mat scoreMat;
			if (half.cols < mdHalf.cols || half.rows < mdHalf.rows)
				continue;
			cv::matchTemplate(TargetImg, mdImg, scoreMat, cv::TM_CCOEFF_NORMED);

			cv::Point	ptMin, ptMax;
			double		Min(0.0), Max(0.0);

			cv::minMaxLoc(scoreMat, &Min, &Max, &ptMin, &ptMax);
			Min *= 100.0;
			Max *= 100.0;
			if (dRetScore)
			{
				dRetCount[chIdx] = half.cols*half.rows;
				dRetScore[chIdx] = Max;
				if (dRetScore[chIdx] > 100.0)
				{
					dRetScore[chIdx] = 100;
				}
			}
			retPoint.x += ((double)(ptMax.x) + (double)(mdHalf.cols) * 0.5) * 2.0;
			retPoint.y += ((double)(ptMax.y) + (double)(mdHalf.rows) * 0.5) * 2.0;
			nPoCnt++;
		}
		retPoint.x = retPoint.x / nPoCnt;
		retPoint.y = retPoint.y / nPoCnt;
	}
	else
	{
		int harfX = RounD(mdOrgImg.cols / 2);
		int harfY = RounD(mdOrgImg.rows / 2);
		int MnX = _DivArea[0].x > _DivArea[2].x ? _DivArea[2].x + harfX : _DivArea[0].x + harfX;
		int MnY = _DivArea[0].y > _DivArea[2].y ? _DivArea[2].y + harfY : _DivArea[0].y + harfY;
		int MxX = _DivArea[0].x < _DivArea[2].x ? _DivArea[2].x + harfX : _DivArea[0].x + harfX;
		int MxY = _DivArea[0].y < _DivArea[2].y ? _DivArea[2].y + harfY : _DivArea[0].y + harfY;
		if (MxX > mdOrgImg.cols)
			MxX = mdOrgImg.cols;
		if (MxY > mdOrgImg.rows)
			MxY = mdOrgImg.rows;
		if (MnX < 0)
			MnX = 0;
		if (MnY < 0)
			MnY = 0;
		int mdWidth = MxX - MnX;
		int mdHeight = MxY - MnY;
		if (mdWidth < 2 || mdHeight < 2)
			return retPoint;
		cv::Mat mdImg = mdOrgImg(cv::Rect(MnX, MnY, mdWidth, mdHeight)).clone();//(cv::Rect(SStart.x, SStart.y, Cols+ ModelItem->_modelWidth, Rows + ModelItem->_modelHeight));
		//_GetPyrDown(mdImg, mdHalf);
		int stpointX = SStart.x - harfX - sx + MnX > 0 ? SStart.x - harfX - sx + MnX : 0;
		int stpointY = SStart.y - harfY - sy + MnY > 0 ? SStart.y - harfY - sy + MnY : 0;
		int width = stpointX + Cols + mdWidth < srcImage.cols ? Cols + mdWidth : srcImage.cols - stpointX;
		int height = stpointY + Rows + mdHeight < srcImage.rows ? Rows + mdHeight : srcImage.rows - stpointY;
		cv::Mat TargetImg = srcImage(cv::Rect(stpointX, stpointY, width, height)).clone();
		double scaleVal = 0.5;
		mdHalf = cv::Mat(mdImg.rows*scaleVal, mdImg.cols*scaleVal, CV_MAKETYPE(CV_8U, mdImg.channels()));
		cv::resize(mdImg, mdHalf, cv::Size(mdImg.cols*scaleVal, mdImg.rows*scaleVal));
		half = cv::Mat(TargetImg.rows*scaleVal, TargetImg.cols*scaleVal, CV_MAKETYPE(CV_8U, TargetImg.channels()));
		cv::resize(TargetImg, half, cv::Size(TargetImg.cols*scaleVal, TargetImg.rows*scaleVal));

#ifdef GeoSaveDebug
		cv::imwrite(std::string("d:\\target.bmp"), src);
		cv::imwrite(std::string("d:\\model.bmp"), mdImg);

		cv::imwrite(std::string("d:\\model_org.bmp"), src.channels() == 3 ? model.color_Image() : model.Image());
#endif
		// 
		cv::Mat scoreMat;
		if (half.cols < mdHalf.cols || half.rows < mdHalf.rows)
			return retPoint;
		cv::matchTemplate(half, mdHalf, scoreMat, cv::TM_CCOEFF_NORMED);

		cv::Point	ptMin, ptMax;
		double		Min(0.0), Max(0.0);

		cv::minMaxLoc(scoreMat, &Min, &Max, &ptMin, &ptMax);
		Min *= 100.0;
		Max *= 100.0;
		if (dRetScore)
		{
			dRetCount[0] = 0;
			dRetScore[0] = Max;
			if (dRetScore[0] > 100.0)
			{
				dRetScore[0] = 100;
			}
		}
		retPoint.x = ((double)(ptMax.x) + (double)(mdHalf.cols) * 0.5) * 2.0;
		retPoint.y = ((double)(ptMax.y) + (double)(mdHalf.rows) * 0.5) * 2.0;
	}
	return retPoint;
}



void GeoMatch_Pattern::CalcDivScore(cv::Mat src, GeoModel::typItemList & model, GeoMatchParam param, GeoResult_Pat & Result, int use_algo_model)
{
	_SearchParam = std::shared_ptr<GeoMatchParam>( new GeoMatchParam(param) );
	_CalcDivScore(src, model, Result, use_algo_model);
	_SearchParam.reset();
}
bool GeoMatch_Pattern::CalcDivScore_detail(cv::Mat src, GeoModel & model, GeoResult_Pat & Result, int use_algo_model, bool searchDiv)
{
	GeoResult_Pat ResultTemp;
	bool isOK = true;
	cv::Mat orgImg,srcImg;
	src.copyTo(orgImg);
	orgImg.copyTo(srcImg);
	int ACnt = _SearchParam->_nAreaCnt_C>_SearchParam->_nAreaCnt_R?_SearchParam->_nAreaCnt_C:_SearchParam->_nAreaCnt_R;

	if (&Result == NULL)
		return false;

	ResultTemp.Alloc(Result.Count());
	_ProcImg->_ImgProcess(srcImg, model);
	if (_ProcImg_E[0]->_fSrc->rows!= srcImg.rows || _ProcImg_E[0]->_fSrc->cols != srcImg.cols)
	{
		cv::Mat MorphImg;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1 + 1 * 2, 1 + 1 * 2), cv::Point(2, 2));
		cv::erode(srcImg, MorphImg, kernel);
		_ProcImg_E[0]->_ImgProcess(MorphImg, model);
		cv::dilate(srcImg, MorphImg, kernel);
		_ProcImg_D[0]->_ImgProcess(MorphImg, model);

		kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1 + 2 * 2, 1 + 2 * 2), cv::Point(2, 2));
		cv::erode(srcImg, MorphImg, kernel);
		_ProcImg_E[1]->_ImgProcess(MorphImg, model);
		cv::dilate(srcImg, MorphImg, kernel);
		_ProcImg_D[1]->_ImgProcess(MorphImg, model);

		kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1 + 3 * 2, 1 + 3 * 2), cv::Point(2, 2));
		cv::erode(srcImg, MorphImg, kernel);
		_ProcImg_E[2]->_ImgProcess(MorphImg, model);
		cv::dilate(srcImg, MorphImg, kernel);
		_ProcImg_D[2]->_ImgProcess(MorphImg, model);
	}
	_CalcDivScore(orgImg, model._Models[GeoModel::enmDepth::spOrg], Result, use_algo_model,_SearchParam->_bDivDetailSearch);
	int nResultScore = 100;
	for(int i=0;i<ACnt;i++)
	{

		if(Result._Score[0].Score[i][0]<_SearchParam->_ScoreList[i])
		{
			isOK = false;
		}
		if(nResultScore>Result._Score[0].Score[i][0])
			nResultScore = Result._Score[0].Score[i][0];
		if(_SearchParam->_bUseCharDiv)
		{

			if(Result._Score[0].Score[i][1]<_SearchParam->_ScoreList[i])
			{
				isOK = false;
			}
			if(Result._Score[0].Score[i][2]<_SearchParam->_ScoreList[i])
			{
				isOK = false;
			}
			if(Result._Score[0].Score[i][3]<_SearchParam->_ScoreList[i])
			{
				isOK = false;
			}
			if(nResultScore>Result._Score[0].Score[i][1])
				nResultScore = Result._Score[0].Score[i][1];
			if(nResultScore>Result._Score[0].Score[i][2])
				nResultScore = Result._Score[0].Score[i][2];
			if(nResultScore>Result._Score[0].Score[i][3])
				nResultScore = Result._Score[0].Score[i][3];
		}
	}
	ResultTemp = Result;

	int k=0;
	while(!isOK && k<3)
	{
		isOK = true;
		_ProcImg = _ProcImg_E[k];

		_CalcDivScore(srcImg, model._Models[GeoModel::enmDepth::spOrg], Result, use_algo_model,_SearchParam->_bDivDetailSearch);
		int MnSc = 100;
		for(int i=0;i<ACnt;i++)
		{

			if(Result._Score[0].Score[i][0]<_SearchParam->_ScoreList[i])
			{
				isOK = false;
			}
			if(MnSc<Result._Score[0].Score[i][0])
			{
				MnSc = Result._Score[0].Score[i][0];
			}
			if(_SearchParam->_bUseCharDiv)
			{

				if(Result._Score[0].Score[i][1]<_SearchParam->_ScoreList[i])
				{
					isOK = false;
				}
				if(Result._Score[0].Score[i][2]<_SearchParam->_ScoreList[i])
				{
					isOK = false;
				}
				if(Result._Score[0].Score[i][3]<_SearchParam->_ScoreList[i])
				{
					isOK = false;
				}
				if(MnSc<Result._Score[0].Score[i][1])
				{
					MnSc = Result._Score[0].Score[i][1];
				}
				if(MnSc<Result._Score[0].Score[i][2])
				{
					MnSc = Result._Score[0].Score[i][2];
				}
				if(MnSc<Result._Score[0].Score[i][3])
				{
					MnSc = Result._Score[0].Score[i][3];
				}
			}
		}
		if(nResultScore<MnSc)
		{
			nResultScore = MnSc;
			ResultTemp = Result;
		}
		if(isOK)
		{
			k++;
			break;
		}
		MnSc = 100;
		isOK = true;

		_ProcImg = _ProcImg_D[k];

		_CalcDivScore(srcImg, model._Models[GeoModel::enmDepth::spOrg], Result, use_algo_model,_SearchParam->_bDivDetailSearch);
		for(int i=0;i<ACnt;i++)
		{

			if(Result._Score[0].Score[i][0]<_SearchParam->_ScoreList[i])
			{
				isOK = false;
			}
			if(MnSc<Result._Score[0].Score[i][0])
			{
				MnSc = Result._Score[0].Score[i][0];
			}
			if(_SearchParam->_bUseCharDiv)
			{

				if(Result._Score[0].Score[i][1]<_SearchParam->_ScoreList[i])
				{
					isOK = false;
				}
				if(Result._Score[0].Score[i][2]<_SearchParam->_ScoreList[i])
				{
					isOK = false;
				}
				if(Result._Score[0].Score[i][3]<_SearchParam->_ScoreList[i])
				{
					isOK = false;
				}
				if(MnSc<Result._Score[0].Score[i][1])
				{
					MnSc = Result._Score[0].Score[i][1];
				}
				if(MnSc<Result._Score[0].Score[i][2])
				{
					MnSc = Result._Score[0].Score[i][2];
				}
				if(MnSc<Result._Score[0].Score[i][3])
				{
					MnSc = Result._Score[0].Score[i][3];
				}
			}
		}
		if(nResultScore<MnSc)
		{
			nResultScore = MnSc;
			ResultTemp = Result;
		}
		k++;
	}

	Result = ResultTemp;

	_ProcImg = _ProcImg_Org;
	return true;
}
void GeoMatch_Pattern::_CalcDivScore(cv::Mat src, GeoModel::typItemList & model, GeoResult_Pat & Result, int use_algo_model, bool searchDiv)
{
	if (&Result == NULL)
		return;

	for (int i=0; i<Result.Count(); i++)
	{
		if(Result._Equality[i] <= 0.0)
		{
			Result._Score[i].Clear();
			continue;
		}

		auto fndRes = model.find( Result._AngleStepId[i] );
		if(fndRes == model.end())
		{
			Result._Score[i].Clear();
			continue;
		}

		GeoModel::typItemPtr item = fndRes->second;

		if(item == nullptr)
		{
			Result._Score[i].Clear();
			continue;
		}

		Result._Score[i].Clear();
		bool bDivSearch(false);
		if(_SearchParam!=nullptr)
		{
			int nCharCnt(1);
			if(_SearchParam->_bUseCharDiv==true)
				nCharCnt = GeoCharDivC * GeoCharDivR;
			int nBufCnt = _SearchParam->_nAreaCnt_C *_SearchParam->_nAreaCnt_R * nCharCnt;

			bDivSearch = true;

			double AngRad = item->_Angle / 180.0 * M_PI;
			double radSin = sin(AngRad);
			double radCos = cos(AngRad);

			std::vector<CRect> AreaList(_SearchParam->_AreaList.begin(), _SearchParam->_AreaList.end());

			if(_SearchParam->_bUseCharDiv)
				Result._Score[i].bSubArea = true;
			cv::Point2d DivArea[GeoCharCount][4];
			cv::Point2d DivSubArea[GeoCharCount][GeoCharDivCnt][4];

			for (int j=0; j<item->_noOfCordinates; j++)
			{
				item->_DivisionId[j] = 0;
				item->_SubDivisionId[j] = 0;
			}

			int Id=1;
			for (auto iter=AreaList.begin(); iter!=AreaList.end(); iter++, Id++)
			{
				cv::Point2d ptList[4];

				CRect rcArea = *iter;

				Result._Score[i]._DivArea[Id-1][0].x = ptList[0].x = (double)( rcArea.left ) * radCos + (double)(rcArea.top   ) * radSin;
				Result._Score[i]._DivArea[Id-1][0].y = ptList[0].y = (double)(-rcArea.left ) * radSin + (double)(rcArea.top   ) * radCos;
				Result._Score[i]._DivArea[Id-1][1].x = ptList[1].x = (double)( rcArea.right) * radCos + (double)(rcArea.top   ) * radSin;
				Result._Score[i]._DivArea[Id-1][1].y = ptList[1].y = (double)(-rcArea.right) * radSin + (double)(rcArea.top   ) * radCos;
				Result._Score[i]._DivArea[Id-1][2].x = ptList[2].x = (double)( rcArea.right) * radCos + (double)(rcArea.bottom) * radSin;
				Result._Score[i]._DivArea[Id-1][2].y = ptList[2].y = (double)(-rcArea.right) * radSin + (double)(rcArea.bottom) * radCos;
				Result._Score[i]._DivArea[Id-1][3].x = ptList[3].x = (double)( rcArea.left ) * radCos + (double)(rcArea.bottom) * radSin;
				Result._Score[i]._DivArea[Id-1][3].y = ptList[3].y = (double)(-rcArea.left ) * radSin + (double)(rcArea.bottom) * radCos;

				float minCoordX(HUGE_VAL), minCoordY(HUGE_VAL), maxCoordX(-HUGE_VAL), maxCoordY(-HUGE_VAL);
				for (int j=0; j<item->_noOfCordinates; j++)
				{
					if(item->_DivisionId[j]) continue; 

					cv::Point2d pt(item->_coordY[j], item->_coordX[j]);
					if(_PtInPoly(ptList, 4, pt)==true)
					{
						item->_DivisionId[j] = Id;

						if(minCoordX > item->_coordY[j])
							minCoordX = item->_coordY[j];

						if(minCoordY > item->_coordX[j])
							minCoordY = item->_coordX[j];

						if(maxCoordX < item->_coordY[j])
							maxCoordX = item->_coordY[j];

						if(maxCoordY < item->_coordX[j])
							maxCoordY = item->_coordX[j];
					}
				}

				float ptct_x = (minCoordX + maxCoordX) / 2.0;
				float ptct_y = (minCoordY + maxCoordY) / 2.0;

				double AngRadBack = (-item->_Angle) / 180.0 * M_PI;
				double radSinBack = sin(AngRadBack);
				double radCosBack = cos(AngRadBack);

				float ptct_org_x = (double)( ptct_x) * radCosBack + (double)(ptct_y) * radSinBack;
				float ptct_org_y = (double)(-ptct_x) * radSinBack + (double)(ptct_y) * radCosBack;

				ptct_x = ptct_org_x;
				ptct_y = ptct_org_y;

				if(_SearchParam->_bUseCharDiv)
				{
					if(use_algo_model != GeoModel::enmMatchAlgo::agImage)
					{

						if (model.find(Result._AngleStepId[0])->second->_Angle == 90 || model.find(Result._AngleStepId[0])->second->_Angle == 270)
						{
							if (rcArea.left < item->_MinPos.y)
								rcArea.left = item->_MinPos.y;
							if (rcArea.top < item->_MinPos.x)
								rcArea.top = item->_MinPos.x;
							if (rcArea.right > item->_MaxPos.y)
								rcArea.right = item->_MaxPos.y;
							if (rcArea.bottom > item->_MaxPos.x)
								rcArea.bottom = item->_MaxPos.x;
						}
						else if (model.find(Result._AngleStepId[0])->second->_Angle == 0 || model.find(Result._AngleStepId[0])->second->_Angle == 180)
						{
							if (rcArea.left < item->_MinPos.x)
								rcArea.left = item->_MinPos.x;
							if (rcArea.top < item->_MinPos.y)
								rcArea.top = item->_MinPos.y;
							if (rcArea.right > item->_MaxPos.x)
								rcArea.right = item->_MaxPos.x;
							if (rcArea.bottom > item->_MaxPos.y)
								rcArea.bottom = item->_MaxPos.y;
						}
					}

					double ImgCols = _SearchParam->_ImgWid;
					double ImgRows = _SearchParam->_ImgLen;

					cv::Point2d chList[4], subList[4];
					cv::Point2d ct, ctrt;

					//					ct.x = ctrt.x = ptct_x;
					//					ct.y = ctrt.y = ptct_y;
					ct.x = (double)(rcArea.left + rcArea.right) / 2.0;
					ct.y = (double)(rcArea.top + rcArea.bottom) / 2.0;

					ctrt.x = (double)( ct.x) * radCos + (double)(ct.y) * radSin;
					ctrt.y = (double)(-ct.x) * radSin + (double)(ct.y) * radCos;

					chList[0].x = (double)( ct.x) * radCos + (double)(rcArea.top   ) * radSin;
					chList[0].y = (double)(-ct.x) * radSin + (double)(rcArea.top   ) * radCos;
					chList[1].x = (double)( ct.x) * radCos + (double)(rcArea.bottom) * radSin;
					chList[1].y = (double)(-ct.x) * radSin + (double)(rcArea.bottom) * radCos;

					chList[2].x = (double)( rcArea.left ) * radCos + (double)(ct.y) * radSin;
					chList[2].y = (double)(-rcArea.left ) * radSin + (double)(ct.y) * radCos;
					chList[3].x = (double)( rcArea.right) * radCos + (double)(ct.y) * radSin;
					chList[3].y = (double)(-rcArea.right) * radSin + (double)(ct.y) * radCos;

					Result._Score[i]._DivSubArea[Id-1][1-1][0] = subList[0] = ptList[0];// 1사분면
					Result._Score[i]._DivSubArea[Id-1][1-1][1] = subList[1] = chList[0];
					Result._Score[i]._DivSubArea[Id-1][1-1][2] = subList[2] = ctrt;
					Result._Score[i]._DivSubArea[Id-1][1-1][3] = subList[3] = chList[2];
					/*for (int j=0; j<item->_noOfCordinates; j++)
					{
						if(item->_DivisionId[j] != Id)
							continue;
						cv::Point2d pt(item->_coordY[j], item->_coordX[j]);
						if(_PtInPoly(subList, 4, pt)==true)
							item->_SubDivisionId[j] = 1;
					}*/

					Result._Score[i]._DivSubArea[Id-1][2-1][0] = subList[0] = chList[0];// 2사분면
					Result._Score[i]._DivSubArea[Id-1][2-1][1] = subList[1] = ptList[1];
					Result._Score[i]._DivSubArea[Id-1][2-1][2] = subList[2] = chList[3];
					Result._Score[i]._DivSubArea[Id-1][2-1][3] = subList[3] = ctrt;

					/*for (int j=0; j<item->_noOfCordinates; j++)
					{
						if(item->_DivisionId[j] != Id)
							continue;
						cv::Point2d pt(item->_coordY[j], item->_coordX[j]);
						if(_PtInPoly(subList, 4, pt)==true)
							item->_SubDivisionId[j] = 2;
					}*/

					Result._Score[i]._DivSubArea[Id-1][3-1][0] = subList[0] = chList[2];// 3사분면
					Result._Score[i]._DivSubArea[Id-1][3-1][1] = subList[1] = ctrt;
					Result._Score[i]._DivSubArea[Id-1][3-1][2] = subList[2] = chList[1];
					Result._Score[i]._DivSubArea[Id-1][3-1][3] = subList[3] = ptList[3];

					/*for (int j=0; j<item->_noOfCordinates; j++)
					{
						if(item->_DivisionId[j] != Id)
							continue;
						cv::Point2d pt(item->_coordY[j], item->_coordX[j]);
						if(_PtInPoly(subList, 4, pt)==true)
							item->_SubDivisionId[j] = 3;
					}*/

					Result._Score[i]._DivSubArea[Id-1][4-1][0] = subList[0] = ctrt;		// 4사분면
					Result._Score[i]._DivSubArea[Id-1][4-1][1] = subList[1] = chList[3];
					Result._Score[i]._DivSubArea[Id-1][4-1][2] = subList[2] = ptList[2];
					Result._Score[i]._DivSubArea[Id-1][4-1][3] = subList[3] = chList[1];

					/*for (int j=0; j<item->_noOfCordinates; j++)
					{
						if(item->_DivisionId[j] != Id)
							continue;
						cv::Point2d pt(item->_coordY[j], item->_coordX[j]);
						if(_PtInPoly(subList, 4, pt)==true)
							item->_SubDivisionId[j] = 4;
					}*/
					for (int j=0; j<item->_noOfCordinates; j++)
					{
						if(item->_DivisionId[j] != Id)
							continue;

						cv::Point2d pt(item->_coordY[j], item->_coordX[j]);
						if(_PtInPoly(Result._Score[i]._DivSubArea[Id-1][1-1], 4, pt)==true) //1사분면
							item->_SubDivisionId[j] = 1;
						else if(_PtInPoly(Result._Score[i]._DivSubArea[Id-1][2-1], 4, pt)==true) //2사분면
							item->_SubDivisionId[j] = 2;
						else if(_PtInPoly(Result._Score[i]._DivSubArea[Id-1][3-1], 4, pt)==true) //3사분면
							item->_SubDivisionId[j] = 3;
						else if(_PtInPoly(Result._Score[i]._DivSubArea[Id-1][4-1], 4, pt)==true) //4사분면
							item->_SubDivisionId[j] = 4;

					}
				}
				else
				{
					for (int j=0; j<item->_noOfCordinates; j++)
					{
						if(item->_DivisionId[j] < 0)
							continue;

						item->_SubDivisionId[j] = 1;
					}
				}
			}
				

			Result._Score[i].Clear();
// 			if(!searchDiv)
// 				_FindModel_DivSearch(src, item, i, cv::Point(Result._Center_y[i],Result._Center_x[i]), 0.0, 0.9, Result, use_algo_model,searchDiv, false);
// 			int xOffset(0),yOffset(0);
// 			if(_SearchParam->_nAreaCnt_C>1)
// 			{
// 				for(int iy = 0; iy<_SearchParam->_nAreaCnt_C; iy++)
// 				{
// 					xOffset += Result._Score[i]._Offset[iy].x;
// 					yOffset += Result._Score[i]._Offset[iy].y;
// 				}
// 				xOffset = xOffset / _SearchParam->_nAreaCnt_C;
// 				yOffset = yOffset / _SearchParam->_nAreaCnt_C;
// 			}
// 			if(_SearchParam->_nAreaCnt_R>1)
// 			{
// 				for(int iy = 0; iy<_SearchParam->_nAreaCnt_R; iy++)
// 				{
// 					xOffset += Result._Score[i]._Offset[iy].x;
// 					yOffset += Result._Score[i]._Offset[iy].y;
// 				}
// 				xOffset = xOffset / _SearchParam->_nAreaCnt_R;
// 				yOffset = yOffset / _SearchParam->_nAreaCnt_R;
// 			}
// 			Result._Center_y[i] += (float)xOffset;
// 			Result._Center_x[i] += (float)yOffset;
			_FindModel_DivSearch(src, item, i, cv::Point(Result._Center_y[i],Result._Center_x[i]), 0.0, 0.9, Result, use_algo_model,searchDiv);


			if(item->_ParentPtr->getMatchAlgo() == GeoModel::enmMatchAlgo::agDefault || item->_ParentPtr->getMatchAlgo() == GeoModel::enmMatchAlgo::agLowContrast)
			{
				if(item->_noOfCordinates > 0)
				{
					for (int j=0; j<item->_noOfCordinates; j++)
					{
						if((item->_Flags[j]&GeoModel::enmPtDataFlag::PdfDelete) == GeoModel::enmPtDataFlag::PdfDelete)
							continue;

						int Idx = item->_DivisionId[j]-1;
						int SubIdx = item->_SubDivisionId[j]-1;
						int nLimitDivIdx = GeoCharCount;
						int nLimitSubDivIdx = GeoCharDivCnt;
						if(Idx>=0 && Idx<nLimitDivIdx && SubIdx>=0 && SubIdx<nLimitSubDivIdx)
						{
							Result._Score[i].Score[Idx][SubIdx] += _DivResBuf[j];
							Result._Score[i].Count[Idx][SubIdx]++;
						}
						else
						{
							int a = 0;
						}
					}

					if(_SearchParam->_nAreaCnt_C>1 || _SearchParam->_nAreaCnt_R == 1)
					for(int x=0; x<_SearchParam->_nAreaCnt_C; x++)
					{
						for(int c=0; c<nCharCnt; c++)
						{
							Result._Score[i].Score[x][c] = (double)(Result._Score[i].Score[x][c]) / (double)(Result._Score[i].Count[x][c]) * 100.0;
						}
					}

					//row
					if(_SearchParam->_nAreaCnt_R>1)
					for(int x=0; x<_SearchParam->_nAreaCnt_R; x++)
					{
						for(int c=0; c<nCharCnt; c++)
						{
							Result._Score[i].Score[x][c] = (double)(Result._Score[i].Score[x][c]) / (double)(Result._Score[i].Count[x][c]) * 100.0;
						}
					}
				}
				else
				{
					if(_SearchParam->_nAreaCnt_C>1 || _SearchParam->_nAreaCnt_R == 1)
					for(int x=0; x<_SearchParam->_nAreaCnt_C; x++)
					{
						for(int c=0; c<nCharCnt; c++)
						{
							Result._Score[i].Score[x][c] = 0;
							Result._Score[i].Count[x][c] = 0;
						}
					}

					//row
					if(_SearchParam->_nAreaCnt_R>1)
					for(int x=0; x<_SearchParam->_nAreaCnt_R; x++)
					{
						for(int c=0; c<nCharCnt; c++)
						{
							Result._Score[i].Score[x][c] = 0;
							Result._Score[i].Count[x][c] = 0;
						}
					}
				}
			}
			else  if(item->_ParentPtr->getMatchAlgo() == GeoModel::enmMatchAlgo::agRelief || item->_ParentPtr->getMatchAlgo() == GeoModel::enmMatchAlgo::agIntaglio)
			{
				CString  str;if(g_pMPTI->m_LogLevel == 8)
				{
					str.Format(_T("Start Stdev Calc"));
					g_pMPTI->AddLog_OCR(str);
				}
				int Imgwid = _SearchParam->_ImgWid;
				int ImgLen = _SearchParam->_ImgLen;
				if(((int)(Result._Angle[i]+5)/90)%2 == 1)
				{
					Imgwid = _SearchParam->_ImgLen;
					ImgLen = _SearchParam->_ImgWid;
				}
				cv::Rect reRect(Result._Center_y[i]-(Imgwid/2)+1,Result._Center_x[i]-(ImgLen/2)+1, Imgwid-2,ImgLen-2);
				cv::Mat cropImg,Intg,Dev;
				if(reRect.x<0)
				{
					reRect.width = _SearchParam->_ImgWid;
					reRect.x = 0;
				}
				if(reRect.y<0)
				{
					reRect.height = _SearchParam->_ImgLen;
					reRect.y = 0;
				}
				if(reRect.x+reRect.width>src.cols)
					reRect.width = src.cols-reRect.x-1;
				if(reRect.y+reRect.height>src.rows)
					reRect.height = src.rows-reRect.y-1;
				
				if(g_pMPTI->m_LogLevel == 8)
				{
					str.Format(_T("Before Create Cropimg, Rect.x = %d, Rect.y = %d, Rect.width = %d, Rect.height = %d"), reRect.x, reRect.y ,reRect.width, reRect.height);
					g_pMPTI->AddLog_OCR(str);
				}

				if(reRect.x < 0 || reRect.y < 0 || reRect.width <= 0 || reRect.height <= 0)
				{
					if(g_pMPTI->m_LogLevel == 8)
					{
						str.Format(_T("Create Cropimg[1], src.witdh = %d, src.height = %d"), src.cols, src.rows);
						g_pMPTI->AddLog_OCR(str);
					}
					cropImg = src.clone();
				}
				else
				{
					if(g_pMPTI->m_LogLevel == 8)
					{
						str.Format(_T("Create Cropimg[3], src.witdh = %d, src.height = %d"), src.cols, src.rows);
						g_pMPTI->AddLog_OCR(str);
					}
					cropImg = src(reRect).clone();
				}

				
				if(g_pMPTI->m_LogLevel == 8)
				{
					str.Format(_T("End Cropimg, croimg.width = %d, cropimg.height = %d"), cropImg.cols, cropImg.rows);
					g_pMPTI->AddLog_OCR(str);
				}

				//cropImg = src(reRect).clone();
				cv::integral(cropImg,Intg,Dev);

				double srcf = cropImg.ptr<uchar>((cropImg.rows-1), (cropImg.cols-1))[0];
				double d = Intg.ptr<int>((Intg.rows-1), (Intg.cols-1))[0];

				double dAvg = d/((Intg.rows-1)*(Intg.cols-1));
				double dPow = pow(dAvg,2);

				d =Dev.ptr<double>((Dev.rows-1), (Dev.cols-1))[0];
				double sqAvg = d/((Dev.rows-1)*(Dev.cols-1));

				double Div =  sqAvg-dPow;
				double sqDiv = sqrt(Div);

				if(item->_ParentPtr->getMatchAlgo() == GeoModel::enmMatchAlgo::agRelief && sqDiv < 30 && dAvg > 240)
				{
						if (_SearchParam->_nAreaCnt_C == 1 && _SearchParam->_nAreaCnt_R == 1)	//Division 없을때
						{
							Result._Equality[0] = 0;
							for (int c = 0; c < nCharCnt; c++)
							{
								Result._Score[i].Score[0][c] = 0;
								Result._Score[i].Count[0][c] = 0;
							}
						}
						else																	//Division 있을때
						{
							//cols
							if(_SearchParam->_nAreaCnt_C>1 || _SearchParam->_nAreaCnt_R == 1)
								for(int x=0; x<_SearchParam->_nAreaCnt_C; x++)
								{
									for(int c=0; c<nCharCnt; c++)
									{
										Result._Score[i].Score[x][c] = 0;
										Result._Score[i].Count[x][c] = 0;
									}
								}

								//row
								if(_SearchParam->_nAreaCnt_R>1)
									for(int x=0; x<_SearchParam->_nAreaCnt_R; x++)
									{
										for(int c=0; c<nCharCnt; c++)
										{
											Result._Score[i].Score[x][c] = 0;
											Result._Score[i].Count[x][c] = 0;
										}
									}
						}
					
				}
				else if (item->_ParentPtr->getMatchAlgo() == GeoModel::enmMatchAlgo::agIntaglio && sqDiv < 30 && dAvg < 55)
				{
					if (_SearchParam->_nAreaCnt_C == 1 && _SearchParam->_nAreaCnt_R == 1)	//Division 없을때
					{
						Result._Equality[0] = 0;
						for (int c = 0; c < nCharCnt; c++)
						{
							Result._Score[i].Score[0][c] = 0;
							Result._Score[i].Count[0][c] = 0;
						}
					}
					else																	//Division 있을때
					{
						//cols
						if(_SearchParam->_nAreaCnt_C>1 || _SearchParam->_nAreaCnt_R == 1)
							for(int x=0; x<_SearchParam->_nAreaCnt_C; x++)
							{
								for(int c=0; c<nCharCnt; c++)
								{
									Result._Score[i].Score[x][c] = 0;
									Result._Score[i].Count[x][c] = 0;
								}
							}

							//row
							if(_SearchParam->_nAreaCnt_R>1)
								for(int x=0; x<_SearchParam->_nAreaCnt_R; x++)
								{
									for(int c=0; c<nCharCnt; c++)
									{
										Result._Score[i].Score[x][c] = 0;
										Result._Score[i].Count[x][c] = 0;
									}
								}
					}
				}
				//if(sqDiv < 30 && dAvg > 240)
				//{
				//	if ( || dAvg < 55)
				//	{
				//		if (_SearchParam->_nAreaCnt_C == 1 && _SearchParam->_nAreaCnt_R == 1)	//Division 없을때
				//		{
				//			Result._Equality[0] = 0;
				//		}
				//		else																	//Division 있을때
				//		{
				//			//cols
				//			if(_SearchParam->_nAreaCnt_C>1)
				//				for(int x=0; x<_SearchParam->_nAreaCnt_C; x++)
				//				{
				//					for(int c=0; c<nCharCnt; c++)
				//					{
				//						Result._Score[i].Score[x][c] = 0;
				//						Result._Score[i].Count[x][c] = 0;
				//					}
				//				}

				//				//row
				//				if(_SearchParam->_nAreaCnt_R>1)
				//					for(int x=0; x<_SearchParam->_nAreaCnt_R; x++)
				//					{
				//						for(int c=0; c<nCharCnt; c++)
				//						{
				//							Result._Score[i].Score[x][c] = 0;
				//							Result._Score[i].Count[x][c] = 0;
				//						}
				//					}
				//		}
				//	}
				//	
				//}
				if(g_pMPTI->m_LogLevel == 8)
				{
					str.Format(_T("Stdev Calc end"));
					g_pMPTI->AddLog_OCR(str);
				}
			}

		}
	}
}

void GeoMatch_Pattern::SetImgProcess(std::shared_ptr<ImgProcessing> ProcImg)
{
	_ProcImg = ProcImg;
}

void GeoMatch_Pattern::SetImgProcess(std::shared_ptr<ImgProcessing> ProcImg,
	std::shared_ptr<ImgProcessing> ProcImg_E1,
	std::shared_ptr<ImgProcessing> ProcImg_E2,
	std::shared_ptr<ImgProcessing> ProcImg_E3,
	std::shared_ptr<ImgProcessing> ProcImg_D1,
	std::shared_ptr<ImgProcessing> ProcImg_D2,
	std::shared_ptr<ImgProcessing> ProcImg_D3)
{
	_ProcImg = ProcImg;
	_ProcImg_Org = ProcImg;
	_ProcImg_E[0] = ProcImg_E1;
	_ProcImg_E[1] = ProcImg_E2;
	_ProcImg_E[2] = ProcImg_E3;
	_ProcImg_D[0] = ProcImg_D1;
	_ProcImg_D[1] = ProcImg_D2;
	_ProcImg_D[2] = ProcImg_D3;
}