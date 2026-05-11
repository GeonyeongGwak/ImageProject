#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <random>
#include <map>
#include <memory>
#include "ImageDeformation.h"

namespace csml
{
	struct __RPNinfo
	{
		int n;
		int c;
		int h;
		int w;
	};
	class AFX_EXT_CLASS Sampler
	{
	public:
		typedef std::map<int, double> classRatio_t;
		typedef std::shared_ptr<Sampler> ptr;

		typedef enum _eType_ { Train, Valid } eType;
		typedef enum _eMode_ { Sequential, Random } eMode;

	public:
		Sampler(eType type)
			:_gen(_rd())
		{
			_imgCount = _imgCh = _imgLen = _imgWid = 0;
			_lbCh = _lbLen = _lbWid = 0;

			_type = type;
			_mode = eMode::Random;

			_SampledValidDB_flag = false;
			_RoiSampling_flag = false;

			_bDefaultProp = true;
		}
		Sampler(Sampler & This, bool _bShareBatchMemoryDB)
		{
			_imgCount = This._imgCount;
			_imgCh = This._imgCh;
			_imgLen = This._imgLen;
			_imgWid = This._imgWid;

			_lbCh = This._lbCh;
			_lbLen = This._lbLen;
			_lbWid = This._lbWid;

			// DB - Copy 없이 같은 메모리 공유
			_AllImg = This._AllImg;
			_Labels = This._Labels;

			if(_bShareBatchMemoryDB)
			{
				// Batch - Copy 없이 같은 메모리 공유
				_BatchImg = This._BatchImg;
				_BatchLbs = This._BatchLbs;

				_BatchImgSub = This._BatchImgSub;
				_BatchLbsSub = This._BatchLbsSub;
			}

			_points = This._points;

			_SampleResult = This._SampleResult;
			rpnidx_T = This.rpnidx_T;
			rpnidx_F = This.rpnidx_F;
			rpnidx_Essential_Good = This.rpnidx_Essential_Good;
			_BatchClassRate = This._BatchClassRate;

			_CountByClass = This._CountByClass;
			_TargetCountByClass = This._TargetCountByClass;

			_SampledValidDB_flag = This._SampledValidDB_flag;
			_RoiSampling_flag = This._RoiSampling_flag;

			_mode = This._mode;
			_type = This._type;
			_BatchSz = This._BatchSz;	

			_zeroSamplingCnt = This._zeroSamplingCnt;

			_RandomOrder = This._RandomOrder;

			prop = This.prop;

			_bDefaultProp = This._bDefaultProp;
		}

	public:
		virtual void init(int nBatch)
		{
			_BatchSz = nBatch;
			allocBatch(nBatch);

			if (_type == eType::Train && _mode == eMode::Sequential)
			{
				// TODO: 섞어주자
				RandomOrderingDB();
			}

			if(_type == eType::Train)
			{
				if(_bDefaultProp)
					AugPropSetting();
			}
		}

		virtual cv::Mat getTrain(int idx, std::vector<double> classRate = std::vector<double>())
		{
			int nCnt(0);
			if (_mode == eMode::Sequential)
				return getMixedEpoch2d(idx, _BatchSz);
			else
				return getMiniBatch2d(_BatchSz, classRate);
		}
		virtual cv::Mat getValid(int idx, int & nCnt)
		{
			return getEpoch2d(idx, _BatchSz, nCnt);
		}

		virtual int get_index(int n);

		virtual Sampler * clone(bool _bCopyMemoryDB);

		int _imgCount;
		int _imgCh;
		int _imgLen;
		int _imgWid;

		int _lbCh;
		int _lbLen;
		int _lbWid;

		cv::Mat _AllImg;
		cv::Mat _Labels;
		std::map<int, double> _CountByClass;
		std::map<int, double> _TargetCountByClass;

		//20.03.17 KYH 배치 고정 사용을 위한 값들
		std::map<int, std::vector<int>> _BatchArray; //배치에 사용된 번호 순서
		bool _bFirstBatch; //초기배치 확인용
		float _FixRate; //고정사용율
		//20.03.17 KYH 배치 고정 사용을 위한 값들

		cv::Mat _BatchImg;
		cv::Mat _BatchLbs;

		cv::Mat _BatchImgSub;
		cv::Mat _BatchLbsSub;
		std::vector<std::vector<cv::Point3d>> _points;

		std::vector<int> _SampleResult;
		std::vector< csml::__RPNinfo> rpnidx_T;
		std::vector< csml::__RPNinfo> rpnidx_F;
		std::vector< csml::__RPNinfo> rpnidx_Essential_Good;
		std::map<int, double> _BatchClassRate;

		eMode _mode;
		eType _type;

		int _BatchSz;

		bool _SampledValidDB_flag; // 이전 학습 시, 샘플링된 이미지를 Test Set으로 사용한 경우 true
		bool _RoiSampling_flag; // Roi DB 정보로 랜덤 샘플링하여 Input을 생성한 경우 true
		int _zeroSamplingCnt; // TestDB 생성 시, Sampling에 실패한 ROI 수

		void setAugFlag(bool aug_chk) { _bDefaultProp = aug_chk; }
		void setAugProp(ImageDeformation::DeformProperty AugProp) { prop = AugProp; }

		struct RoiInfo
		{
			int imgIdx; // _PathList의 인덱스
			int roiIdx; // .lmk 파일 내부의 인덱스 

			int left;
			int right;
			int top;
			int bottom;

			int classNum; // _Folders의 Index에 해당
			int labelNum; // Label Image 상의 클래스(=brightness)

			bool targetChk; // 학습 시, 지속적으로 보여질 DB 내의 ROI
		};

	protected:
		virtual cv::Mat getEpoch2d(int idx, int nBatch, int & nCnt);
		virtual cv::Mat getMiniBatch2d(int nBatch, std::vector<double> classRate = std::vector<double>());

		virtual cv::Mat getMixedEpoch2d(int idx, int nBatch);
		virtual void RandomOrderingDB();
		virtual int gcd(int a, int b);

		virtual void allocBatch(int nBatch);

		virtual void Sampling_Begin(int nBatchSz)
		{

		}
		virtual void Sampling(int idx, int n)
		{

		}
		virtual bool GetImage(int n, cv::Mat dstImg, cv::Mat dstLb, bool _bAugmentation = false);
		virtual void preprocess(cv::Mat dst);

		virtual bool AugImage(int n, std::vector<cv::Mat> dstImg, cv::Mat dstLb = cv::Mat());
		virtual bool AugImageCoil(int n, std::vector<cv::Mat> dstImg, cv::Mat dstLb = cv::Mat(), bool bGANSetting = false);
		virtual bool AugImageOCR(int n, std::vector<cv::Mat> dstImg, cv::Mat dstLb = cv::Mat());
		virtual bool AugImage(int n, std::vector<cv::Mat> dstImg, std::vector<cv::Rect> &coordi_box, std::vector<cv::Rect> &coordi_good, std::vector<int> &nClsBubble, std::vector<int> &nClsGood, cv::Mat dstLb = cv::Mat());
		virtual bool AugImage(int n, std::vector<cv::Mat> dstImg, std::vector<cv::Rect> &coordi_box, std::vector<int> &class_box, cv::Mat dstLb = cv::Mat());

		virtual void AugPropSetting();
		virtual void AugPropSettingforBg();
		virtual void AugPropSettingforBox();
		virtual void AugPropSettingforBoxWithCadGerber();
		virtual void AugPropSettingforOCR();

		// Cad&Gerber Box
		virtual bool RandomSampling(cv::Mat &ROIImg, cv::Mat FoV, RoiInfo &ROIInfo, int ROI_w, int ROI_h, bool bOriScale, bool bg_mix, cv::Mat BgFoV);
		// OCR
		virtual bool RandomSampling(cv::Mat &ROIImg, cv::Mat FoV, cv::Mat Label, RoiInfo ROIInfo, int ROI_w, int ROI_h, bool bFontResize, cv::Mat BgFoV = cv::Mat(), CRect BgRect = CRect());
		// Bubble
		virtual bool RandomSampling(std::vector<cv::Mat> &ROIImg, cv::Mat &ROILb, std::vector<cv::Mat> FoV, cv::Mat Label, CRect ROI_rect, int ROI_w, int ROI_h, bool bg_chk);
		// RPN
		virtual bool RandomSamplingB(std::vector<cv::Mat> &ROIImg, cv::Mat &ROILb, std::vector<cv::Mat> FoV, cv::Mat Label, CRect ROI_rect, int ROI_w, int ROI_h, bool bg_chk, int &roix, int &roiy, int &roir, int &roib, bool bAug);

		std::random_device _rd;
		std::mt19937 _gen;

		std::vector<int> _RandomOrder;

		ImageDeformation deform;
		ImageDeformation::DeformProperty prop;

		bool _bDefaultProp; // 디폴트 설정 값으로의 Augmentation 수행 여부
	};
}