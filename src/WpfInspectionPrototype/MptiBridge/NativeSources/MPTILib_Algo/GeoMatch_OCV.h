#pragma once

#include "geomatch.h"
#include "OCVParamDef.h"


	class GeoModel_POCR : public GeoModel
	{
		class Item_POCR : public GeoModel::Item
		{
		public:
			Item_POCR(GeoModel * parent);

		public:
			cv::Mat algo_3(cv::Mat srcP, cv::Scalar mean, cv::Scalar stddev, int width, int height); //노이즈 심할때 사용
			bool Calculate(cv::Mat & src, cv::Mat mask, int nPreProvLv, float maxContrast, float minContrast, bool _bEdgeEnhance, int use_algo = GeoModel::enmMatchAlgo::agDefault);
		};

	public:
		GeoModel_POCR(void);
		virtual ~GeoModel_POCR(void);
		virtual std::shared_ptr<Item> CreateItem();

	public:

	protected:
	};

	class GeoPOCRResult
	{
	public:
		GeoPOCRResult()
		{
			Clear();
		}

		void Clear()
		{
			_ResScore = 0.0;
			_AreaCount = 0;
			for (int i = 0; i < MAX_POCR_DIVISITION_CNTS; i++)
			{
				_DivArea[i] = cv::Point2d();
				_Score[i] = 0;
				_Count[i] = 0;
			}
		}

	public:
		int _AreaCount;
		cv::Point2d _DivArea[MAX_POCR_DIVISITION_CNTS];
		float _Score[MAX_POCR_DIVISITION_CNTS];
		int _Count[MAX_POCR_DIVISITION_CNTS];
		cv::Rect _Roi[MAX_POCR_DIVISITION_CNTS];
		double _ResScore;
	};

	class GeoResult_POCR : public GeoResult
	{
	public:
		GeoResult_POCR();
		GeoResult_POCR(int cnt);
		virtual ~GeoResult_POCR();
	public:
		virtual void Alloc(int cnt);
		virtual void Free();

		std::vector<std::vector<GeoPOCRResult>> _Score;	// 엔진의 폰트 수에 따라 증가한다.
		std::vector<std::vector<float>> _ResEdge;
		std::vector<std::vector<float>> _ResDiff;
		std::vector<int> _SelEngine;
	};

	class ImgProcessing_POCR : ImgProcessing
	{
	public:

		ImgProcessing_POCR()
		{
			_OrgImage = std::make_shared<cv::Mat>();
			// 		_fSrc= std::make_shared<cv::Mat>();
			// 		_half= std::make_shared<cv::Mat>();
			// 		_pyr = std::make_shared<cv::Mat>();
			edge = std::make_shared<ImgProcessing>();
			diff = std::make_shared<ImgProcessing>();
			Img = std::make_shared<ImgProcessing>();
			bIsImgProcessed = false;
		}
		ImgProcessing_POCR(ImgProcessing_POCR &lhs)
		{
			Free();
			copy(lhs);
		}

		void copy(ImgProcessing_POCR &lhs)
		{
			Free();

			edge = lhs.edge;
			diff = lhs.diff;
			Img = lhs.Img;

			_fSrc = lhs._fSrc;
			_half = lhs._half;
			_pyr = lhs._pyr;

			_OrgImage = lhs._OrgImage;
			_ProcBuf[GeoModel::enmDepth::spOrg] = lhs._ProcBuf[GeoModel::enmDepth::spOrg];
			_ProcBuf[GeoModel::enmDepth::spHalf] = lhs._ProcBuf[GeoModel::enmDepth::spHalf];
			_ProcBuf[GeoModel::enmDepth::spZoom] = lhs._ProcBuf[GeoModel::enmDepth::spZoom];

		}
		void Free()
		{
			for (int i = 0; i < GeoModel::enmDepth::spNum; i++)
				_ProcBuf[i].Free();
		}

		void _ImgProcess(cv::Mat src, int modelStep, GeoModel::enmMatchAlgo MatchAlgo = GeoModel::enmMatchAlgo::agDefault);
		//virtual void _ImgProcess(cv::Mat src,GeoModel &model);
		std::shared_ptr<ImgProcessing> edge;
		std::shared_ptr<ImgProcessing> diff;
		std::shared_ptr<ImgProcessing> Img;

		bool bIsImgProcessed;
	};

	class GeoMatch_POCR : public GeoMatch
	{
	public:
		GeoMatch_POCR(void);
		virtual ~GeoMatch_POCR(void);

	public:
		virtual void _GetResult(GeoResult_POCR * resData, int nModelCount, int nResCnt, GeoResult_POCR & result);

		void FindModel_Char(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoResult_POCR & result);
		void FindModel_Char(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, cv::Point2f OfsChar, GeoResult_POCR & result);
		void FindModel_Char_Angle(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoModel * StringModel, GeoResult * StringResult, cv::Point2f OfsChar, GeoResult_POCR & result);
		void FindModel_Char_Angle_AllAngle(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoModel * StringModel, GeoResult * StringResult, cv::Point2f OfsChar, GeoResult_POCR & result);
		void FindModel_Char_AngleFunc(int nCharAngleType, TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoModel * StringModel, GeoResult * StringResult, cv::Point2f OfsChar, GeoResult_POCR & result);

		void CalcDivScore(GeoModel & model, GeoResult_POCR & Result, int nThreshold);
		void CalcResScore(GeoResult_POCR & Result, bool isSmallFont);
		void DrawContours(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth, bool wb);

	void(GeoMatch_POCR::*FunPtr_FindModel_Char_Angle[2])(TCHAR ch, GeoModel_POCR & model, cPOCRInfoParam * param, GeoModel * StringModel, GeoResult * StringResult, cv::Point2f OfsChar, GeoResult_POCR & result);
		cPOCRInfoParam * _EngineParam;
		TCHAR _Ch;
		cv::Mat _DestImg;

	protected:
		// 	virtual void ProcIntegral(cv::Mat & src, cv::Mat & Integ, cv::Mat & SqInteg);
		void _CalcDivScore(GeoModel & model, GeoResult_POCR & Result);
		void _CalcDivScore_Img(GeoModel & model, GeoResult_POCR & Result, int nThreshold);
		void _CalcPosSubArea(GeoModel::typItemPtr item, cPOCRInspParam * param);
		void _CalcPosSubArea(GeoModel Model, GeoResult_POCR & Result, cPOCRInspParam * param, int iRes, int iEngine);

		void _setSubAreaId(GeoModel::typItemPtr item, cv::Point2d posList[][4], int nDivCnt);
		void _CalcPosSubArea(GeoModel::typItemPtr item, cPOCRInspParam * param, double angle);

		void _CalcAlgoSubScore(GeoModel::typItemPtr item, GeoResult_POCR & Result, cPOCRInspParam * param, int iRes, int iEngine);
		void _CalcAlgoSubScore(GeoModel model, GeoResult_POCR & Result, cPOCRInspParam * param, int iRes, int iEngine, int nThreshold);


		POCRFont _GetFontType(TCHAR ch);
		bool _CheckFont(TCHAR ch);
		bool _CheckThinFont(TCHAR ch);
		void SetDivCnt2EngineParam(cPOCRInspParam * param);
		void SaveResScore(GeoResult_POCR & Result, bool isSmallFont);

		void _FindModel_DivSearch(GeoModel::typItemPtr ModelItem, int idx, cv::Point SStart, GeoResult_POCR & Result, int use_algo_model);

		void _CalcCoefficient_Edge_Divide(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float * pResBuf);
		void _CalcCoefficient_Diff_Divide(cv::Mat & Sdl, cv::Mat & Integ, cv::Point SStart, GeoModel::typItemPtr ModelItem, float * pResBuf);

		void _DrawContours(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth);
	};

