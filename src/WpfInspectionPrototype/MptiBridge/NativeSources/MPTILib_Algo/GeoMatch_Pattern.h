#pragma once
#include "GeoMatch.h"


class GeoModel_Pattern : public GeoModel
{
public:
	class Item_Pattern : public GeoModel::Item
	{
	public:
		Item_Pattern(GeoModel * parent);
	public:
		void CalcMask(cv::Mat mask, float cogX, float cogY, float imgAngle);
		void DrawMask(cv::Mat mask, float cogX, float cogY, float imgAngle);
	};
public:
	class DivParam
	{
	public:
		DivParam()
		{
			m_bsetData = false;

			DiviLnR = 0;
			DiviLnC = 0;

			memset(GapLnR,0,sizeof(GapLnR));
			memset(GapLnC,0,sizeof(GapLnC));
			memset(DiviScore,0,sizeof(DiviScore));

			DetailSearch = true;

			bUseCharDiv = false;
		}
		void CopyOf(DivParam & dp)
		{
			dp.DiviLnR = DiviLnR;
			dp.DiviLnC;

			memcpy(dp.GapLnR,GapLnR,sizeof(GapLnR));
			memcpy(dp.GapLnC,GapLnC,sizeof(GapLnC));
			memcpy(dp.DiviScore,DiviScore,sizeof(DiviScore));

			dp.DetailSearch = DetailSearch;
			dp.bUseCharDiv = bUseCharDiv;
			dp.m_bsetData = m_bsetData;
		}
		void SetDivParam(fileAlgoPath* sfileAlgoPath)
		{
			if(sfileAlgoPath)
			{
				CString sPathModelTeach;
				sPathModelTeach.Format(_T("%s"), sfileAlgoPath->m_sPathModelTeach);
				int rIndex = sPathModelTeach.ReverseFind(_T('@'));
				CString sIdx = _T("");

				for(int i=rIndex+1;i< sPathModelTeach.GetLength();i++)
				{
					sIdx+= sPathModelTeach.GetAt(i);
				}

				int nModelIdx = _ttoi(sIdx);nModelIdx--;
				DiviLnR = sfileAlgoPath->DiviLnR[nModelIdx];
				DiviLnC = sfileAlgoPath->DiviLnC[nModelIdx];

				for(int j=0;j<CNT_PATTERN_DIVISION_R;j++)
					GapLnR[j]=sfileAlgoPath->GapLnR[nModelIdx][j];

				for(int j=0;j<CNT_PATTERN_DIVISION_C;j++)
					GapLnC[j]=sfileAlgoPath->GapLnC[nModelIdx][j];

				for(int j=0;j<CNT_PATTERN_SCORE;j++)
					DiviScore[j]=sfileAlgoPath->DiviScore[nModelIdx][j];

				DetailSearch = sfileAlgoPath->DetailSearch[nModelIdx];

				bUseCharDiv = sfileAlgoPath->m_bUseCharacter;
				m_bsetData = true;
			}
		}
		//shkim model divide area
		int DiviLnR;
		int DiviLnC;

		double GapLnR[CNT_PATTERN_DIVISION_R];
		double GapLnC[CNT_PATTERN_DIVISION_C];

		double DiviScore[CNT_PATTERN_SCORE];

		bool DetailSearch;
		bool bUseCharDiv;
		bool m_bsetData;
	};

	typedef enum { PdmBackground=0x00, PdmMask=0x01, PdmAlive=0x02 } enmPtDataModify;
	
public:
	GeoModel_Pattern(void);
	virtual ~GeoModel_Pattern(void);

public:
	void SetDivisionParam(fileAlgoPath* sfileAlgoPath);

	virtual void CopyOf(GeoModel_Pattern & md, float fImgAngle=0.0f);
	virtual void Alloc(cv::Mat src, float fImgAngle=0.0f);
	void SetMaskImage(cv::Mat mask);
	cv::Mat GetMaskImage();

	virtual bool _SaveFile_v3(CArchive & ar);
	virtual bool _LoadFile_v3(CArchive & ar, int ver);

	virtual bool _SaveFile_v5(CArchive & ar);
	virtual bool _LoadFile_v5(CArchive & ar, int version);

	virtual std::shared_ptr<Item> CreateItem();
public:
	DivParam		_DParam;					// 분할 검사 파라메터(파일에 저장하지 않는다)
	cv::Mat		_Mask;
};

class GeoMatchParam
{
public:
	GeoMatchParam()
	{
		_ImgWid = _ImgLen = 0;
		_nAreaCnt_R = _nAreaCnt_C = _nCharAreaCnt = 0;
		_bUseCharDiv = false;
		_bDivDetailSearch = false;
		_modelRect.left = _modelRect.top = _modelRect.right = _modelRect.bottom = 0;
		_nFiler = 0;

		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}

	virtual ~GeoMatchParam()
	{
		g_pMManager->pem_delete_check(this);
	}

public:
	void SetParam(std::shared_ptr<GeoModel_Pattern> model, bool bUseCharDiv, int nDivLnCnt_C, int nDivLnCnt_R, double * lnPosC, double * lnPosR, double * score, int scorePitch, bool bDetailSearch, RECT modelRect)
	{
		int Angle = (int)model->getImageAngle();
		cv::Mat & Img = model->Image();

		_ImgWid = Img.cols;
		_ImgLen = Img.rows;

		_bUseCharDiv = bUseCharDiv;
		_bDivDetailSearch = bDetailSearch;
		_modelRect = modelRect;

		_nCharAreaCnt = 1;
		if(_bUseCharDiv==true)
			_nCharAreaCnt = GeoCharDivR * GeoCharDivC;

		_nAreaCnt_R = nDivLnCnt_R + 1;
		_nAreaCnt_C = nDivLnCnt_C + 1;

		_AreaList.clear();
		_AreaList.resize(_nAreaCnt_R * _nAreaCnt_C);

		_ScoreList.clear();
		_ScoreList.resize(_nAreaCnt_R * _nAreaCnt_C);

		for (int y=0; y<_nAreaCnt_R; y++)
		{
			for (int x=0; x<_nAreaCnt_C; x++)
			{
				CRect rcArea;

				if(_ImgWid >= _ImgLen)
				{
					int iy = y;
					int ix = x;
					int cntR = nDivLnCnt_R;
					int cntC = nDivLnCnt_C;
					CalcRect(Img, lnPosR, lnPosC, ix, iy, cntC, cntR, rcArea);

					int szX = _nAreaCnt_C;
					_ScoreList[y*_nAreaCnt_C+x] = score[iy*scorePitch+ix];
				}
				else if(_ImgWid < _ImgLen)
				{
					int iy = y;
					int ix = x;
					int cntR = nDivLnCnt_R;
					int cntC = nDivLnCnt_C;
					CalcRect(Img, lnPosC, lnPosR, iy, ix, cntR, cntC, rcArea);

					int szX = _nAreaCnt_C;
					_ScoreList[y*_nAreaCnt_C+x] = score[iy*scorePitch+ix];
				}

				// 영상의 center를 기준으로 좌표 변환
				cv::Point ctPos;
				ctPos.x = Img.cols / 2;
				ctPos.y = Img.rows / 2;

				rcArea.top = rcArea.top - ctPos.y;
				rcArea.bottom = rcArea.bottom - ctPos.y;

				rcArea.left = rcArea.left - ctPos.x;
				rcArea.right = rcArea.right - ctPos.x;

				_AreaList[y*_nAreaCnt_C+x] = rcArea;
			}
		}
	}

	int _ImgWid;
	int _ImgLen;

	int _nAreaCnt_R;
	int _nAreaCnt_C;
	int _nCharAreaCnt;

	std::vector<CRect> _AreaList;
	std::vector<float> _ScoreList;

	bool _bUseCharDiv;

	bool _bDivDetailSearch;

	CRect _modelRect;
	int _nFiler;

protected:
	void CalcRect(cv::Mat & Img, double * lnPosR, double * lnPosC, int ix, int iy, int cntC, int cntR, CRect & rcArea)
	{
		if(cntR>0)
		{
			if(iy>0 && iy<cntR)
			{
				rcArea.top = lnPosR[iy-1];
				rcArea.bottom = lnPosR[iy];
			}
			else if(iy==0)
			{
				rcArea.top = 0;
				rcArea.bottom = lnPosR[iy];
			}
			else if(iy == cntR)
			{
				rcArea.top = lnPosR[iy-1];
				rcArea.bottom = Img.rows;
			}
			else
			{
				rcArea.top = 0;
				rcArea.bottom = Img.rows;
			}
		}
		else
		{
			rcArea.top = 0;
			rcArea.bottom = Img.rows;
		}

		if(cntC>0)
		{
			if(ix>0 && ix<cntC)
			{
				rcArea.left = lnPosC[ix-1];
				rcArea.right = lnPosC[ix];
			}
			else if(ix==0)
			{
				rcArea.left = 0;
				rcArea.right = lnPosC[ix];
			}
			else if(ix == cntC)
			{
				rcArea.left = lnPosC[ix-1];
				rcArea.right = Img.cols;
			}
			else
			{
				rcArea.left = 0;
				rcArea.right = Img.cols;
			}
		}
		else
		{
			rcArea.left = 0;
			rcArea.right = Img.cols;
		}
	}
};

class GeoCharResult
{
public:
	GeoCharResult()
	{
		Clear();
	}

	void Clear()
	{
		bSubArea = false;
		for (int i=0; i<GeoCharCount; i++)
		{
			_Offset[i] = cv::Point2i();
			for (int w=0; w<GeoCharDivCnt; w++)
			{
				ScoreImg[i][w] = 0;
				Score[i][w] = 0;
				Count[i][w] = 0;
			}
		}
	}

public:
	int Count[GeoCharCount][GeoCharDivCnt];
	float Score[GeoCharCount][GeoCharDivCnt];
	float ScoreImg[GeoCharCount][GeoCharDivCnt];
	bool bSubArea;
	cv::Point2d _DivArea[GeoCharCount][4];
	cv::Point2d _DivSubArea[GeoCharCount][GeoCharDivCnt][4];
	cv::Point2i _Offset[GeoCharCount];
};

class GeoResult_Pat : public GeoResult
{
public:
	GeoResult_Pat();
	GeoResult_Pat(int cnt);
	virtual ~GeoResult_Pat();
public:
	virtual void Alloc(int cnt);
	virtual void Free();

	std::vector<GeoCharResult> _Score;
};

class GeoMatch_Pattern : public GeoMatch
{
public:
	void FindModel_Div(GeoModel_Pattern & model, GeoMatchParam param, GeoResult_Pat & result);

	// 분할 검사
	void CalcDivScore(cv::Mat src, GeoModel::typItemList & model, GeoMatchParam param, GeoResult_Pat & Result, int use_algo_model);
	void _CalcDivScore(cv::Mat src, GeoModel::typItemList & model, GeoResult_Pat & Result, int use_algo_model, bool searchDiv = true);
	bool CalcDivScore_detail(cv::Mat src, GeoModel & model, GeoResult_Pat & Result, int use_algo_model, bool searchDiv);
	void _FindModel_DivSearch(cv::Mat src, GeoModel::typItemPtr ModelItem, int idx, cv::Point SStart, double minScore, double greediness, GeoResult_Pat & Result, int use_algo_model, bool DitailSearch = true, bool searchDiv = true);

	cv::Point2i _CalcCoefficient_DivSearch_Divide(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX,bool DitailSearch = true);
	cv::Point2i _CalcCoefficientLap_DivSearch_Divide(cv::Mat & Sdl, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, bool DitailSearch = true);
	cv::Point2i _CalcCoefficientImg_DivSearch_Divide(cv::Mat & Sdl, cv::Mat & Integ, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, float * dRetScore=nullptr, int * dRetCount=nullptr, bool DitailSearch = true);
	cv::Point2i _CalcCoefficientImg_DivSearch_Divide_sse(cv::Mat & Sdl, cv::Mat & Integ, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness, int Id,int nMarginX, float * dRetScore=nullptr, int * dRetCount=nullptr, bool DitailSearch = true);
	cv::Point2i _CalcImageMatching(cv::Mat srcImage, cv::Point2d _DivArea[4], cv::Point2d _DivSubArea[4][4], cv::Point SStart, GeoModel::typItemPtr ModelItem, int nMarginX, float * dRetScore, int * dRetCount, bool DitailSearch);
	
	void SetImgProcess(std::shared_ptr<ImgProcessing> ProcImg);
	void SetImgProcess(std::shared_ptr<ImgProcessing> ProcImg,std::shared_ptr<ImgProcessing> ProcImg_E1,std::shared_ptr<ImgProcessing> ProcImg_E2,std::shared_ptr<ImgProcessing> ProcImg_E3,std::shared_ptr<ImgProcessing> ProcImg_D1,std::shared_ptr<ImgProcessing> ProcImg_D2,std::shared_ptr<ImgProcessing> ProcImg_D3);
	std::shared_ptr<GeoMatchParam> _SearchParam;

private:
	std::shared_ptr<ImgProcessing> _ProcImg_Org;
	std::shared_ptr<ImgProcessing> _ProcImg_E[3];
	std::shared_ptr<ImgProcessing> _ProcImg_D[3];
};