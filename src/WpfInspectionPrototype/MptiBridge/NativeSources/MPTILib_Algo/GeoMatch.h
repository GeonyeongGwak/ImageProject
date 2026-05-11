#pragma once

#include <opencv2/opencv.hpp>
#include <math.h>
#include "InspParamDef.h"
#include <memory>
#include <map>

//shkim IPP rotate
//#include "ippm.h"
#include "ipps.h"
#include "ippi.h"

//#pragma comment(lib, "ippm.lib")
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")


#define GeoCharDivC			2
#define GeoCharDivR			2
#define GeoCharDivCnt		GeoCharDivC * GeoCharDivR
#define GeoCharLenByRow		12
#define	GeoCharRow			1
#define GeoCharCount		GeoCharLenByRow * GeoCharRow
#define GeoEpsilon			0.0000001
#define EPSILON 0.000001
#define Min_Meg 25
#define Min_Meg_Inv 0.04// 1/Min_Meg

#ifdef _DEBUG
#define GeoNoUseMultiThread	1
#endif
//#define GeoNoUseSimd			1
#define GeoNoUseWriteTimeLog	1
#define GeoNoUseWriteLog		1

class GeoMatch;

class imgMatch;

class GeoModel
{
public:
	typedef enum { spLow = 0, spMedium, spHigh }enmSpeed;
	typedef enum { acLow = 0, acMedium = 1, acHigh = 2 }enmAccuracy;
	typedef enum { spZoom = 0, spHalf, spOrg, spNum }enmDepth;
	typedef enum { agOriginal = 0, agDefault = 1, agLowContrast, agIntaglio, agRelief, agImage, agValue, agImageMin }enmMatchAlgo;
	typedef enum { PdfDelete = 0x01, PdfDraw = 0x02 } enmPtDataFlag;
	typedef enum { pcLow = 0, pcMedium = 1, pcHigh = 2 }enmPrecision;//shkim searching margin option
	typedef enum { cmPreProc = 0, cmProProc = 1 }enmCalcMode;

public:
	class Item
	{
	public:
		Item(GeoModel * parent);
		Item(GeoModel * parent, cv::Mat & src, cv::Mat mask, int nPreProvLv, float maxContrast, float minContrast, bool _bEdgeEnhance, int nFilter);
		virtual ~Item();

	public:
		virtual bool Calculate(cv::Mat & src, cv::Mat mask, int nPreProvLv, float maxContrast, float minContrast, bool _bEdgeEnhance, int use_algo = GeoModel::enmMatchAlgo::agDefault);
		bool CalculateAngle(int modelWidth, int modelHeight, int nPreProvLv, float maxContrast, float minContrast, bool _bEdgeEnhance, std::shared_ptr<Item> Orgmd, float angle, int use_algo = GeoModel::enmMatchAlgo::agDefault);
		bool ProCalculate(cv::Mat & src, int use_algo = GeoModel::enmMatchAlgo::agDefault);
		bool isProcessed() { return _modelDefined; }


		void CalcMask(cv::Mat mask, float cogX, float cogY, float imgAngle);
		void DrawMask(cv::Mat mask, float cogX, float cogY, float imgAngle);


		int		_modelHeight;
		int		_modelWidth;
		int		_noOfCordinates;	// Number of elements in coordinate array
		float	* _coordX;
		float	* _coordY;
		float	* _edgeMagnitude;	// gradient magnitude
		float	* _edgeDerivativeX;	// gradient in X direction
		float	* _edgeDerivativeY;	// gradient in Y direction	
		int		* _DivisionId;
		int		* _SubDivisionId;
		BYTE	* _Flags;

		float	_CogX;			// 기준 좌표
		float	_CogY;
		float	_Angle;			// 각도
		int		_PyrDown;		// 축소 스텝
		int		_SobelSize;		// 사용안함
		bool	_bOriginal;		// 원본크기의 0도 여부
		float	_AvgEdges;		// 사용 안함

		GeoModel * _ParentPtr;

		cv::Point2i _MinPos;
		cv::Point2i _MaxPos;
		double		_Stdev;
		int _Filter;

	public:
		void SaveFile(CArchive & ar);
		void LoadFile(CArchive & ar);

		void SaveFile_v2(CArchive & ar);
		void LoadFile_v2(CArchive & ar, int ver);

		void SaveFile_v3(CArchive & ar);
		void LoadFile_v3(CArchive & ar, int ver);

	public:

		virtual void _Alloc(int wid, int len);
		virtual void _Free();

		cv::Mat algo_1(cv::Mat srcP, double dLowthre, double dHighthre, cv::Scalar mean, cv::Scalar stddev, cv::Size D_size, cv::Size U_size);
		cv::Mat algo_2(cv::Mat srcP, cv::Scalar mean, cv::Scalar stddev, int width, int height); // 일반적으로 사용
		virtual cv::Mat algo_3(cv::Mat srcP, cv::Scalar mean, cv::Scalar stddev, int width, int height); //노이즈 심할때 사용
		cv::Mat algo_4(cv::Mat srcP); //음각

		void Thinning(cv::Mat src);
		cv::Mat algo_6(cv::Mat srcP); //양각
		cv::Mat noise_remove(cv::Mat srcP, int width, int height);

		bool	_modelDefined;

	protected:
		void _Sobel(cv::Mat & src, cv::Mat & gx, cv::Mat & gy, cv::Mat & mask);
		void _Calc_Dir_Mag(cv::Mat gx, cv::Mat gy, cv::Mat gsx, cv::Mat gsy, float * magMat, int * orients, cv::Size Ssize, float & MaxGradient);
		void _Calc_Dir_Mag(cv::Mat gx, cv::Mat gy, float * magMat, int * orients, cv::Size Ssize, float & MaxGradient);
		void _NonMaximalSuppression(int * orients, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float MaxGradient);
		void _HysterisisThreshold(cv::Mat gx, cv::Mat gy, cv::Mat gsx, cv::Mat gsy, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float maxContrast, float minContrast);
		void _HysterisisThreshold(cv::Mat gx, cv::Mat gy, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float maxContrast, float minContrast);

		void _Laplacian(cv::Mat & src, cv::Mat & gl, cv::Mat & mask);
		void _Calc_Dir_Lap(cv::Mat gx, cv::Mat gy, cv::Mat gl, float * magMat, int * orients, cv::Size Ssize, float & MaxGradient);
		void _NonMaximalSuppression_Lap(int * orients, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float MaxGradient);
		void _HysterisisThreshold_Lap(cv::Mat gl, float * magMat, cv::Mat nmsEdges, cv::Size Ssize, float maxContrast, float minContrast);
	};


	typedef std::shared_ptr<Item>	typItemPtr;
	typedef std::map<int, typItemPtr>	typItemList;

public:
	GeoModel();
	virtual ~GeoModel();

public:

	virtual void CopyOf(GeoModel & md, float fImgAngle = 0.0f);
	virtual void Alloc(cv::Mat src, float fImgAngle = 0.0f);

	cv::Mat Image() { return _srcModel; }
	cv::Mat color_Image() { return _srcModel_color; }
	float getImageAngle() { return _fImgAngle; }
	bool empty() { return _srcModel.empty(); };

	virtual void Preprocess();
	virtual void RePreprocess();
	bool isProcessed() { return _modelDefined; }

	virtual void ReCalculation(cv::Mat src);
	bool SaveFile(CString sPath, bool bSzOptimiz);
	bool LoadFile(CString sPath, double resX, double resY);
	bool ExtLoadFile(int nIndex, double resX, double resY);
	bool SaveFile(CArchive& ar, bool bSzOptimiz, int nVersion = 10);
	bool LoadFile(CArchive& ar, double resX, double resY);



public:
	// 유저가 모델 1개 직접 만들고 싶을 때
	std::shared_ptr<Item> MakeModel(cv::Mat src, cv::Mat mask, float angle, float maxContrast, float minContrast, int nPyrDepth);
	cv::Mat _GetAngle(cv::Mat & src, float angle, bool bInterpolation = false);
	void RotateImg_ipp(cv::Mat& userSrc, double angle, cv::Mat* userDst, bool bInterpolation);

	bool RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize);
	bool RotateImg_ipp2020(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear = true, bool bUseOrgSize = false);

	cv::Point WarpAffine(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent, bool Rev);

	void RotateImg_ipp_color(cv::Mat& userSrc, double angle, cv::Mat* userDst, bool bInterpolation);

	virtual std::shared_ptr<Item> CreateItem();

	typItemList		_Models[enmDepth::spNum];	// 축소Step과 각도 별 모델들 저장 컨테이너
	int				_pyrDownStep;				// 영상 축소 step
	float			_MaxScore;					// 사용 안함

	int _Filter;



public:
	// 각도 파라메터
	float getAngle() { return _fAngleStd; }
	void setAngle(float fAngle) { if (_fAngleStd != fAngle) _modelDefined = false; _fAngleStd = fAngle; _fImgAngle = fAngle; }
	void setAngleRange(bool bEnable, float fDeltaAngNeg, float fDeltaAngPos, float fStepAng);
	void getAngleRange(bool & bEnable, float & fDeltaAngNeg, float & fDeltaAngPos, float & fStepAng);

	int getAngleStep() { return _nAngleSearchStep; }
	void setAngleStep(int nStep) { _nAngleSearchStep = nStep; }

	// 기타 파라메터 get/set
	float getAcceptance() { return _Acceptance; }
	void setAcceptance(float val) { _Acceptance = val; }

	enmSpeed getSpeed() { return _Speed; }
	void setSpeed(enmSpeed val) { _Speed = val; }

	enmAccuracy getAccuracy() { return _Accuracy; }
	void setAccuracy(enmAccuracy val) { _Accuracy = val; }

	int getOrgAngleIdx() { return _OrgModelAngleIdx; }

	bool getFindSubPixel() { return _FindSubPixel; }
	void setFindSubPixel(bool val) { _FindSubPixel = val; }

	bool getEdgeEnhance() { return _bEdgeEnhancement; }
	void setEdgeEnhance(bool val) { if (_bEdgeEnhancement != val) _modelDefined = false; _bEdgeEnhancement = val; }

	cv::Point2i getOverlappedModelDist() { return _overlappedModelDist; }
	void setOverlappedModelDist(cv::Point2i Dist) { _overlappedModelDist = Dist; }

	int getPreProcLv(int nZoomDepth) { return _PreProcLv[nZoomDepth]; }

	int getMatchAlgo() { return _Use_algo_model; }
	void setMatchAlgo(int val) { _Use_algo_model = val; }

	CString getModelPath() { return _sLoadedPath; }

	int getLightbt() { return _LightNumber; }
	void setLightbt(int val) { _LightNumber = val; }

	int getModelnChannel() { return _Use_model_nchannel; }
	void setModelnChannel(int val) { _Use_model_nchannel = val; }

	int getModelFilterType() { return _Filter; }
	void setModelFilterType(int val) { _Filter = val; }

	enmPrecision getPrecision() { return _Precision; }
	void setPrecision(enmPrecision val) { _Precision = val; }

	void getSearchAngle(float & fStAngle, float & fEdAngle)
	{
		fStAngle = _fSearchAngleSt;
		fEdAngle = _fSearchAngleEd;
	}
	void setSearchAngle(float fStAngle, float fEdAngle)
	{
		_fSearchAngleSt = fStAngle;
		_fSearchAngleEd = fEdAngle;
	}

	void setRes(double resX, double resY)
	{
		_resolX = resX;
		_resolY = resY;
	}
	void getRes(double* resX, double* resY)
	{
		*resX = _resolX;
		*resY = _resolY;
	}

	void reSizeModel(double resX, double resY)
	{
		if (resX == _resolX && resY == _resolY)
			return;
		if (resX <= 0.0001 || _resolX <= 0.0001 || resY <= 0.0001 || _resolY <= 0.0001)
			return;
		if (resX >= 1 || _resolX >= 1 || resY >= 1 || _resolY >= 1)
			return;

		//file 저장하는 소수점이하의 값 차이면 예외처리
		if (abs(resX - _resolX) <= 0.00000000000000001 || abs(resY - _resolY) <= 0.00000000000000001)
			return;

		double dRatioX = _resolX / resX;
		double dRatioY = _resolY / resY;

		if (dRatioX >= 10 || dRatioX <= 0.1 || dRatioY >= 10 || dRatioY <= 0.1)
			return;

		cv::Mat* tmp;

		if (_srcModel_color.empty() == false)
			tmp = &_srcModel_color;
		else
			tmp = &_srcModel;

		int nSizeX = (int)(tmp->cols*_resolX / resX);
		int nSizeY = (int)(tmp->rows*_resolY / resY);

		cv::resize(*tmp, *tmp, cv::Size(nSizeX, nSizeY));

		_modelDefined = false;
	}

protected:
	void _MakeBuffer(int wid, int len);
	GeoModel::typItemPtr _MakeModel(cv::Mat src, cv::Mat mask, float angle, int nStep, float maxContrast, float minContrast, std::map<int, GeoModel::typItemPtr> & mdList, int nPyrDepth, bool bOrgImg = false, enmCalcMode cMode = cmPreProc);
	void _MakeModelAngle(GeoModel::typItemPtr orgItem, cv::Mat src, cv::Mat mask, float angle, int nStep, float maxContrast, float minContrast, std::map<int, GeoModel::typItemPtr> & mdList, int nPyrDepth, bool bOrgImg = false);


	virtual void _GetPyrDown(cv::Mat src, cv::Mat & des);
	bool _CheckPyrSz(cv::Mat & src);

	void _PreprocessAngle(cv::Mat src, float maxContrast, float minContrast, std::map<int, GeoModel::typItemPtr> & mdList, int nPyrDepth, enmCalcMode cMode = cmPreProc);


	virtual bool _SaveFile_v2(CArchive & ar);
	virtual bool _LoadFile_v2(CArchive & ar, int ver);


	virtual bool _SaveFile_v3(CArchive & ar);
	virtual bool _LoadFile_v3(CArchive & ar, int ver);

	virtual bool _LoadFile_v4(CArchive & ar, int ver, double resX, double resY);

	virtual cv::Mat _ImgPreProcess(cv::Mat src);

	virtual bool _SaveFile_v5(CArchive & ar);
	virtual bool _LoadFile_v5(CArchive & ar, int version);

	virtual bool _SaveFile_v6(CArchive & ar);
	virtual bool _LoadFile_v6(CArchive & ar, int version);

	virtual bool _SaveFile_v7(CArchive & ar);
	virtual bool _LoadFile_v7(CArchive & ar, int version, double resX, double resY);

	virtual bool _SaveFile_v8(CArchive & ar);
	virtual bool _LoadFile_v8(CArchive & ar, int version);

	virtual bool _SaveFile_v9(CArchive & ar);
	virtual bool _LoadFile_v9(CArchive & ar, int version, double resX, double resY);

	virtual bool _SaveFile_v10(CArchive & ar);
	virtual bool _LoadFile_v10(CArchive & ar, int version);
	virtual bool _SaveFile_v11(CArchive & ar);
	virtual bool _LoadFile_v11(CArchive & ar, int version);
	// 모델 리스트
	cv::Mat		_srcModel;
	cv::Mat		_srcModel_color;

	cv::Mat		_halfModel;
	cv::Mat		_pyrModel;


	float		_fImgAngle;

	// 모델 파라메터
	bool		_modelDefined;
	bool		_bEdgeEnhancement;

	// 검색 파라메터
	float		_Acceptance;
	enmSpeed	_Speed;
	enmAccuracy	_Accuracy;
	cv::Point2i	_overlappedModelDist;
	enmPrecision	_Precision;

	// 각도 파라메터
	bool		_bAngleFlag;
	float		_fAngleNeg;
	float		_fAnglePos;
	float		_fAngleStep;
	float		_fAngleStd;

	float		_fSearchAngleSt;
	float		_fSearchAngleEd;

	// 기타 파라메터
	int			_PreProcLv[enmDepth::spNum];	// 사용 안함
	int			_OrgModelAngleIdx;				// 여러 각도 생성 시 0도 index
	bool		_FindSubPixel;					// 2차 함수를 이용해서 sub-pixel위치를 계산하는 여부 플래그
	int			_Use_algo_model;				// add 시 적용한 모델을 찾기 위한 변수
	int			_LightNumber;					// 현재 조명 버튼 index
	int			_Use_model_nchannel;			// 적용된 모델의 체널

	CString		_sLoadedPath;					// 모델 파일 경로

	bool		_bUseImgPreProc;				// 모델 복사 했을 때는 영상 전처리 안하도록 변경.
	int			_nPyraDownStepLimit;

	int			_nAngleSearchStep;

	//shkim resolution
	double _resolX;
	double _resolY;
};




class GeoResult
{
public:
	GeoResult();
	GeoResult(int cnt);
	virtual ~GeoResult();
public:
	virtual void Alloc(int cnt);
	virtual void Free();
	int getResCnt() { return _ResCnt; }
	void setResCnt(int val) { _ResCnt = val; }
	int Count() { return _Count; }
	std::vector<float> _Center_x;
	std::vector<float> _Center_y;
	std::vector<float> _Equality;
	std::vector<float> _Angle;
	std::vector<int> _AngleStepId;

protected:
	int _Count;
	int _ResCnt;
};

class GeoSortItem
{
public:
	GeoSortItem()
	{

	}
	GeoSortItem(float score, int id, float angle, float x, float y, int i, int j)
		: _Score(score)
		, _Id(id)
		, _Angle(angle)
		, _X(x)
		, _Y(y)
		, _I(i)
		, _J(j)
	{

	}
	virtual ~GeoSortItem()
	{

	}

public:
	float _Score;
	float _Angle;
	float _X;
	float _Y;
	int _Id;
	int _I;
	int _J;
};

bool operator < (const GeoSortItem & lhs, const GeoSortItem & rhs);
bool operator > (const GeoSortItem & lhs, const GeoSortItem & rhs);
class ImgProcBuf
{
public:
	ImgProcBuf()
		: matGradMag(nullptr)
	{

	}
	ImgProcBuf(ImgProcBuf & lhs)
		: matGradMag(nullptr)
	{
		Free();
		copy(lhs);
	}
	virtual ~ImgProcBuf()
	{
		Free();
	}

	void Alloc(cv::Mat src)
	{
		Free();

		if (Sdx.rows != src.rows || Sdx.cols != src.cols)
			Sdx = cv::Mat::zeros(src.rows, src.cols, CV_16SC1);

		if (Sdy.rows != src.rows || Sdy.cols != src.cols)
			Sdy = cv::Mat::zeros(src.rows, src.cols, CV_16SC1);

		if (Sdl.rows != src.rows || Sdl.cols != src.cols)
			Sdl = cv::Mat::zeros(src.rows, src.cols, CV_16SC1);

		if (Integral.rows != src.rows || Integral.cols != src.cols)
			Integral = cv::Mat::zeros(src.rows, src.cols, CV_32SC1);

		if (SqIntegral.rows != src.rows || SqIntegral.cols != src.cols)
			SqIntegral = cv::Mat::zeros(src.rows, src.cols, CV_32SC1);

		_matGradMag = cv::Mat::zeros(src.rows, src.cols, CV_32FC1);

		// 		CString msg;
		// 		msg.Format(_T("Alloc: %d"),(src.rows * src.cols));
		// 		AfxMessageBox(msg);
		matGradMag = _matGradMag.ptr<float>();//std::allocate_shared<float>();
	}
	void Free()
	{
	}
	void copy(ImgProcBuf & right)
	{
		Sdx = right.Sdx;
		Sdy = right.Sdy;
		Sdl = right.Sdl;
		Integral = right.Integral;
		SqIntegral = right.SqIntegral;
		_matGradMag = right._matGradMag;
		matGradMag = _matGradMag.ptr<float>();
	}

	void operator=(ImgProcBuf & right)
	{
		Free();
		copy(right);
	}

public:
	cv::Mat Sdx;
	cv::Mat Sdy;
	cv::Mat Sdl;
	cv::Mat Integral;
	cv::Mat SqIntegral;
	cv::Mat _matGradMag;
	float * matGradMag;
};
class ImgProcessing
{
public:
	ImgProcessing()
	{
		_OrgImage = std::make_shared<cv::Mat>();
		_fSrc = std::make_shared<cv::Mat>();
		_half = std::make_shared<cv::Mat>();
		_pyr = std::make_shared<cv::Mat>();
		bIsImgProcessed = false;

		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);

		bool bUseAVX = false;
		FunPtr_CalcMagnitude_sse = &ImgProcessing::_CalcMagnitude_sse;
		if (_MSC_FULL_VER >= 160040219)
		{
			int cpuInfo[4];
			__cpuid(cpuInfo, 1);

			bool bOsUseXSAVE_XRSTORE = cpuInfo[2] & (1 << 27) || false;
			bool bCpuAVXSuport = cpuInfo[2] & (1 << 28) || false;

			if (bOsUseXSAVE_XRSTORE && bCpuAVXSuport)
			{
				unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
				bUseAVX = (xcrFeatureMask & 0x6) || false;
			}
		}
		if (bUseAVX)
			FunPtr_CalcMagnitude_sse = &ImgProcessing::_CalcMagnitude_AVX;
	}
	virtual ~ImgProcessing()
	{
		Free();

		g_pMManager->pem_delete_check(this);
	}

	void Free()
	{
		_ProcBuf[GeoModel::enmDepth::spOrg].Free();
		_ProcBuf[GeoModel::enmDepth::spHalf].Free();
		_ProcBuf[GeoModel::enmDepth::spZoom].Free();


	}
	void copy(ImgProcessing &lhs)
	{
		Free();

		_fSrc = lhs._fSrc;
		_half = lhs._half;
		_pyr = lhs._pyr;

		_OrgImage = lhs._OrgImage;

		_ProcBuf[GeoModel::enmDepth::spOrg] = lhs._ProcBuf[GeoModel::enmDepth::spOrg];
		_ProcBuf[GeoModel::enmDepth::spHalf] = lhs._ProcBuf[GeoModel::enmDepth::spHalf];
		_ProcBuf[GeoModel::enmDepth::spZoom] = lhs._ProcBuf[GeoModel::enmDepth::spZoom];

		edgeModelStep = lhs.edgeModelStep;
		IntaglioModelStep = lhs.IntaglioModelStep;

	}
	void operator=(ImgProcessing & right)
	{
		copy(right);
	}
	ImgProcBuf _ProcBuf[GeoModel::enmDepth::spNum];

	void _GetPyrDown(cv::Mat src, cv::Mat & des);	// 축소 영상 생성

	virtual cv::Mat _ImgPreprocess(cv::Mat src, GeoModel::enmMatchAlgo MatchAlgo);
	virtual void _ImgProcess(cv::Mat src, GeoModel &model);
	virtual void _ImgProcess(cv::Mat src, cv::Mat half, cv::Mat pyr, int modelStep, GeoModel::enmMatchAlgo MatchAlgo = GeoModel::enmMatchAlgo::agDefault);//for Match POCR
	virtual void ProcIntegral(cv::Mat & src, cv::Mat & Integ, cv::Mat & SqInteg);
	// 에지 세기 계산
	void _CalcMagnitude(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Size Ssize);
	void _CalcMagnitudeLap(cv::Mat & Sdl, float * matGradMag, cv::Size Ssize);
	void _CalcMagnitude_sse(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Size Ssize);
	void _CalcMagnitude_AVX(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Size Ssize);

	std::shared_ptr<cv::Mat> _OrgImage;
	std::shared_ptr<cv::Mat> _fSrc;
	std::shared_ptr<cv::Mat> _half;
	std::shared_ptr<cv::Mat> _pyr;

	int edgeModelStep;
	int IntaglioModelStep;
	bool bIsImgProcessed;

	void(ImgProcessing::*FunPtr_CalcMagnitude_sse)(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Size Ssize);
};

class GeoMatch
{
public:
	class imgMatch
	{
	public:
		cv::Mat m_model;			//모델영상
		cv::Mat m_Dev;				//편차영상
		cv::Mat m_Integ;				//적분영상

		double m_nAvgValue;			//평균
		double m_dstdDevP;			//편차 영상의 표준편차

	public:
		imgMatch()
		{
			m_nAvgValue = 0;
			FunPtr_ImageMatching_sse = &imgMatch::_ImageMatching_sse;

			bool bUseAVX = false;
			if (_MSC_FULL_VER >= 160040219)
			{
				int cpuInfo[4];
				__cpuid(cpuInfo, 1);

				bool bOsUseXSAVE_XRSTORE = cpuInfo[2] & (1 << 27) || false;
				bool bCpuAVXSuport = cpuInfo[2] & (1 << 28) || false;

				if (bOsUseXSAVE_XRSTORE && bCpuAVXSuport)
				{
					unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
					bUseAVX = (xcrFeatureMask & 0x6) || false;
				}
			}
			if (bUseAVX)
				FunPtr_ImageMatching_sse = &imgMatch::_ImageMatching_AVX;
		}
		~imgMatch()
		{
		}
		virtual void copyTo(imgMatch& dst, cv::Rect re);

		virtual void calcValue(cv::Mat model, int nThreshold);

		virtual void setImage(cv::Mat model);

		virtual double ImageMatching(imgMatch inputImg, int nPitch);

		virtual double ImageMatching_sse(imgMatch inputImg, int nPitch);

		double _ImageMatching_sse(imgMatch inputImg, int nPitch);

		double _ImageMatching_AVX(imgMatch inputImg, int nPitch);

		int GetThresHold(UCHAR* src, int nSizeX, int nSizeY, int* thresh);
		int GetHistogram(UCHAR* src, int sizeX, int sizeY, float* histo);

		double(imgMatch::*FunPtr_ImageMatching_sse)(imgMatch inputImg, int nPitch);
	};

public:
	GeoMatch(void);
	~GeoMatch(void);

	// 메인 매칭 함수

	virtual void FindModel(GeoModel & model, GeoResult & result, bool bIsDiv);


	// Drawing 함수
	void DrawContours(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth, int i);
	void DrawContours(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth);
	void DrawContoursAngle(cv::Mat source, GeoModel& model, GeoResult& Res, cv::Scalar color, int lineWidth, float angle);
	void DrawContours(cv::Mat source, GeoModel::typItemPtr ModelItem, GeoResult & Res, cv::Scalar color, int lineWidth);
	cv::Mat DrawContourColor(cv::Mat source, GeoModel & model, GeoResult & Res, cv::Scalar color, int lineWidth);

	// Item 1개 매칭 함수
	void _FindModel(GeoModel::typItemPtr ModelItem, int idx, cv::Point SStart, cv::Size Ssize, float fAngleStd, double minScore, double greediness, GeoResult & result, bool bCalcPrecisePos = false, int use_algo_model = 1);
	// 영상 전처리 함수 set
	void SetImgProcess(std::shared_ptr<ImgProcessing> _ProcImg);


	// for CI Auto Focus
	double grad_1stOrder_focus_algo_1(cv::Mat src);
	double grad_1stOrder_focus_algo_2(cv::Mat src, double a);
	double grad_2ndOrder_focus_algo_2(cv::Mat src, double a);

	CString getProcDataSaveFlag() { return _bSaveProcData; }
	void setProcDataSaveFlag(CString bSave) { _bSaveProcData = bSave; }
	void SaveImg(cv::Mat img, CString funcName, CString imgName);

protected:
	// 영상 탐색 함수
	void _FindModel(cv::Mat src, GeoModel & model, GeoModel::enmDepth depth, double minScore, double greediness, GeoResult & result, int nResCnt, bool bCalcPrecisePos = false);	// 영상 전체 탐색 함수
	void _FindImgModel(cv::Mat src, GeoModel & model, GeoResult & result, cv::Mat& org, cv::Mat& half, cv::Mat& pyr);  // 이미지기반 매칭 함수
	void _TunningRes(cv::Mat src, cv::Mat zoom, GeoModel::typItemList & model, float fAngleStd, int pyrDownStep, double minScore, double greediness, GeoResult & zoomResult, GeoResult & srcResult, bool bCalcPrecisePos = false, int use_algo_model = 1, int Precision = GeoModel::enmPrecision::pcLow);	// 이전 탐색 결과로 Score 계산을 다시하는 함수

	std::shared_ptr<ImgProcessing> _ProcImg;
	//ImgProcBuf _ProcBuf[GeoModel::enmDepth::spNum];


	// 매칭 함수
	void _CalcCoefficient(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);
	void _CalcCoefficient_sse(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);

	inline void _CalcCoefficient_sse_inline(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pEdgeY, float * pMag, BYTE * pFlag, float costh, float sinth);
	void _CalcCoefficientLap(cv::Mat & Sdl, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);
	void _CalcCoefficientImg_sse(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);

	inline void _CalcCoefficientImg_sse_inline(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pMag, BYTE * pFlag, cv::Point2i MinPos, cv::Point2i MaxPos, float stdev);
	void _CalcCoefficientImg(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);

	inline void _CalcCoefficientVal_sse_inline(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pMag, BYTE * pFlag, cv::Point2i MinPos, cv::Point2i MaxPos, float stdev);
	void _CalcCoefficientVal_sse(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);
	void _CalcCoefficientVal(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);

	// 위치 찾기
	void _CalcResult(cv::Mat resBuf, int nModelIdx, GeoModel::typItemPtr ModelItem, cv::Point SStart, cv::Size Ssize, GeoResult & result);
	virtual void _GetResult(GeoResult * resData, int nModelCount, int nResCnt, GeoResult & result);
	void _CalcPrecisePos(cv::Mat resBuf, int nModelIdx, GeoModel::typItemPtr ModelItem, GeoResult & result);

	void _CalcCoefficient_DivSearch(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Point SStart, GeoModel::typItemPtr ModelItem, float normMinScore, float normGreediness);

	std::vector<float> _DivResBuf;

	virtual void _AllResultProcedure(GeoModel::typItemPtr * pmdTmpList, GeoResult * resData, int nModelCount, GeoResult & result, int nResCnt);

	bool _PtInPoly(cv::Point2d * ptArray, int nCount, cv::Point2d point);
	CString _bSaveProcData;

	//AVX
	void _CalcCoefficient_AVX_inline(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pEdgeY, float * pMag, BYTE * pFlag, float costh, float sinth);
	void _CalcCoefficientImg_AVX_inline(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pMag, BYTE * pFlag, cv::Point2i MinPos, cv::Point2i MaxPos, float stdev);
	void _CalcCoefficientVal_AVX_inline(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, int i, int j, int ImgWid, int ImgLen, int noOfCordinates, float * pCoordX, float * pCoordY, float * pEdgeX, float * pMag, BYTE * pFlag, cv::Point2i MinPos, cv::Point2i MaxPos, float stdev);
	void _CalcCoefficient_AVX(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);
	void _CalcCoefficientImg_AVX(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);
	void _CalcCoefficientVal_AVX(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);

	void(GeoMatch::*FunPtr_CalcCoefficient_sse)(cv::Mat & Sdx, cv::Mat & Sdy, float * matGradMag, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);
	void(GeoMatch::*FunPtr_CalcCoefficientImg_sse)(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);
	void(GeoMatch::*FunPtr_CalcCoefficientVal_sse)(cv::Mat & Sdl, cv::Mat & Integ, cv::Mat & IntegSq, cv::Mat & resBuf, cv::Point SStart, cv::Size Ssize, GeoModel::typItemPtr ModelItem, float fAngleStd, float normMinScore, float normGreediness, bool bConcurrency);
};
