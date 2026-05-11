#pragma once
#include "GeoMatch_Pattern.h"


class GeoModel_Foot : public GeoModel
{
public:
	class Item_Foot : public GeoModel::Item
	{
	public:
		Item_Foot(GeoModel * parent);
	public:
		void CalcMask(cv::Mat mask, float cogX, float cogY, float imgAngle);
		void DrawMask(cv::Mat mask, float cogX, float cogY, float imgAngle);
	};
public:

	typedef enum { PdmBackground=0x00, PdmMask=0x01, PdmAlive=0x02 } enmPtDataModify;
	
public:
	GeoModel_Foot(void);
	virtual ~GeoModel_Foot(void);

public:
	void SetDivisionParam(fileAlgoPath* sfileAlgoPath);

	virtual void CopyOf(GeoModel_Foot & md, float fImgAngle=0.0f);
	virtual void Alloc(cv::Mat src, float fImgAngle=0.0f);
	void SetMaskImage(cv::Mat mask);
	cv::Mat GetMaskImage();

	virtual bool _SaveFile_v3(CArchive & ar);
	virtual bool _LoadFile_v3(CArchive & ar, int ver);

	virtual bool _SaveFile_v8(CArchive & ar);
	virtual bool _LoadFile_v8(CArchive & ar, int version);

	virtual bool _SaveFile_v11(CArchive & ar);
	virtual bool _LoadFile_v11(CArchive & ar, int version);
	virtual std::shared_ptr<Item> CreateItem();
public:
	int FootType;
	cv::Mat		_Mask;
	cv::Rect	WedgeArea;
	int FootDirection;
	int FootDBC =0;
};

class GeoResult_Foot : public GeoResult
{
public:
	GeoResult_Foot();
	GeoResult_Foot(int cnt);
	virtual ~GeoResult_Foot();
public:
	virtual void Alloc(int cnt);
	virtual void Free();

	std::vector<GeoCharResult> _Score;
};

class GeoMatch_Foot : public GeoMatch
{
public:
	void FindModel_Div(GeoModel_Foot & model, GeoMatchParam param, GeoResult_Pat & result);

	// 분할 검사
	void CalcDivScore(cv::Mat src, GeoModel::typItemList & model, GeoMatchParam param, GeoResult_Pat & Result, int use_algo_model);
	void _CalcDivScore(cv::Mat src, GeoModel::typItemList & model, GeoResult_Pat & Result, int use_algo_model, bool searchDiv = true);
	bool CalcDivScore_detail(cv::Mat src, GeoModel & model, GeoResult_Pat & Result, int use_algo_model, bool searchDiv);
	void _FindModel_DivSearch(cv::Mat src, GeoModel::typItemPtr ModelItem, int idx, cv::Point SStart, double minScore, double greediness, GeoResult_Pat & Result, int use_algo_model, bool DitailSearch = true, bool searchDiv = true);

	cv::Point2i _CalcCoefficient_DivSearch_Divide(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX,bool DitailSearch = true);
	cv::Point2i _CalcCoefficientLap_DivSearch_Divide(cv::Mat & Sdl, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, bool DitailSearch = true);
	cv::Point2i _CalcCoefficientImg_DivSearch_Divide(cv::Mat & Sdl, cv::Mat & Integ, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, float * dRetScore=nullptr, int * dRetCount=nullptr, bool DitailSearch = true);
	cv::Point2i _CalcCoefficientImg_DivSearch_Divide_sse(cv::Mat & Sdl, cv::Mat & Integ, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, float * dRetScore=nullptr, int * dRetCount=nullptr, bool DitailSearch = true);
	
	void SetImgProcess(std::shared_ptr<ImgProcessing> ProcImg);
	void SetImgProcess(std::shared_ptr<ImgProcessing> ProcImg,std::shared_ptr<ImgProcessing> ProcImg_E1,std::shared_ptr<ImgProcessing> ProcImg_E2,std::shared_ptr<ImgProcessing> ProcImg_E3,std::shared_ptr<ImgProcessing> ProcImg_D1,std::shared_ptr<ImgProcessing> ProcImg_D2,std::shared_ptr<ImgProcessing> ProcImg_D3);
	std::shared_ptr<GeoMatchParam> _SearchParam;

private:
	std::shared_ptr<ImgProcessing> _ProcImg_Org;
	std::shared_ptr<ImgProcessing> _ProcImg_E[3];
	std::shared_ptr<ImgProcessing> _ProcImg_D[3];
};