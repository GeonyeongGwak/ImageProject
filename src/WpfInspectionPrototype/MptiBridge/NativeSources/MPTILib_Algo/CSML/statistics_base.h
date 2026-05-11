#pragma once

#include "mat_type.h"
#include "Sampler.h"
#include <memory>
#include <map>
#include <vector>


namespace csml
{
	class AFX_EXT_CLASS statistics_base
	{
	public:
		typedef std::shared_ptr<statistics_base> ptr;

	public:
		virtual void init()=0;
		virtual void batch(int e, int i, Sampler * TrainSampler, float loss, std::vector<float> loss_vals) = 0;
		virtual void epoch(int e, Sampler * TrainSampler)=0;
		virtual void valid_epoch(int e, int i, Sampler * ValidSampler, cuFloat::ptr result, int nCurBatch)=0;
		virtual void valid_epoch(int e, int i, Sampler * ValidSampler, std::vector<cuFloat::ptr> result, int nCurBatch) = 0;
		virtual void infer_res(int i, cuFloat::ptr result, cuFloat::ptr src)=0;
		virtual void SaveToFile(CString sPath) = 0;

		virtual float epoch_result(int e) = 0;
		bool _boxsave;//박스결과 저장할지 여부
		bool _boxAllshow;//라벨과 겹치는 지점 이외에 true라 나온 부분도 같이 그릴지 여부
		CString _sRootPath;

		// 학습 정보 저장용 버퍼
		CString _sProcPath;
		CString _netType;
		CString _modelId;
		CString _loadModelPath;
		CString _loadMoelId;
		std::vector<CString> _sTrainPath;
		std::vector<bool> _sTrainPathChk;
		std::vector<CString> _sValidPath;
		std::vector<bool> _sValidPathChk;
		int _TrainRoiWidTh;  // ROI 사이즈 임계값
		int _TrainRoiLenTh;  // ROI 사이즈 임계값
		int _ValidRoiWidTh;  // ROI 사이즈 임계값
		int _ValidRoiLenTh;  // ROI 사이즈 임계값
		int _nTotalTrainROICnt; // Train DB 내, 사이즈가 임계값 이상인 ROI 개수
		int _nTotalValidROICnt; // Valid DB 내, 사이즈가 임계값 이상인 ROI 개수
		int _nValidROICnt; // Valid DB 내, 실제 Random Sampling된 ROI 개수
		int _width;
		int _height;
		int _gpuId;
		int	_gpuCnt;
		int	_nBatchSize;
		float _fLearnRate;
		float _fWeightDecay;
		std::vector<double> _nMiniBatchRate; // empty 이면 DB 비율로 mini-batch 생성
		bool _bDefaultAugOption; // 코드 상의 default augmentation option 사용여부
		bool _bSamplingValid;
		bool _bSamplingTrain;
	};

	class AFX_EXT_CLASS stat_classification : public statistics_base
	{
	public:
		class result
		{
		public:
			typedef std::shared_ptr<result> ptr;


		public:
			result()
			{
				_loss = 0.f;
				totalRatio = 0.f;
				procTime = 0.f;
				_nTrainItemCnt = 0;
				_nOkCnt = 0;
			}

		public:
			std::map<int, std::vector<int>> _idx;
			std::vector<float> _batchLoss;

			float _loss;
			int _nTrainItemCnt;
			int _nOkCnt;
			float totalRatio;
			float procTime;

			Sampler::classRatio_t classRatio;
			std::map<int,int> classCount;
			std::vector<int> _decision;
		};

	public:
		virtual void init();
		virtual void batch(int e, int i, Sampler * TrainSampler, float loss, std::vector<float> loss_vals);
		virtual void epoch(int e, Sampler * TrainSampler);
		virtual void valid_epoch(int e, int i, Sampler * ValidSampler, cuFloat::ptr result, int nCurBatch);
		virtual void valid_epoch(int e, int i, Sampler * ValidSampler, std::vector<cuFloat::ptr> result, int nCurBatch);
		virtual void infer_res(int i, cuFloat::ptr result, cuFloat::ptr src);
		virtual void SaveToFile(CString sPath);
		virtual float epoch_result(int e);

	public:
		std::map<int, result::ptr> _result;
		cv::Mat _ValidResult;
		std::map<int, int> _SelCnt;
	protected:
		virtual void alloc(int e);
		virtual void alloc(cuFloat::ptr data);
	};
}