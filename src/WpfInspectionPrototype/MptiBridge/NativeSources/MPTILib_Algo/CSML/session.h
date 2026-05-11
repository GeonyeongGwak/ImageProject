#pragma once
#include "network.h"
#include "Sampler.h"
#include "statistics_base.h"
#include <memory>
#include <functional>
#include <map>

namespace csml
{
	enum eTrainingObject
	{
		None = 0, Classficiation, Generation, Segmentation, BoxDetection, RPN, SegCoil, ClsCoil, ClsOCR, GAN
	};
	typedef enum _eLossCompareMethod
	{
		Equal = 0, Lower, Upper
	}eLossCompareMethod;

	typedef struct _stGroupTrainingItem
	{
		int nGroupId;
		float loss_thres;
		int nMethod;
	}stGroupTrainingItem;

	class AFX_EXT_CLASS session
	{
	public:
		typedef std::shared_ptr<session>	ptr;
		std::vector<cuFloat::ptr> validResults;
		std::vector<cuShort::ptr> validResults_s;
		typedef enum _eType { Normal, MultiGpu } eType;
		cv::Mat _CoilResultView;
		csml::eTrainingObject _trainObj;
		

	public:
		session(std::vector<network::ptr> nets, std::vector<params::ptr> params, std::vector<buffers::ptr> buffers, std::vector<loss_func::ptr> loss, float fWDecay=0.0001, int gpuId = 0, int gpuCount = 1, bool bAdversarial=false);
		session(std::vector<network::ptr> nets, std::vector<loss_func::ptr> loss, float fWDecay=0.0001, int gpuId = 0, int gpuCount = 1, bool bAdversarial=false);
		session(network::ptr nets, loss_func::ptr loss, float fWDecay=0.0001, int gpuId = 0, int gpuCount = 1, bool bAdversarial=false);
		session(CString sFilePath, int gpuId, int gpuCount = 1, bool bChangeOptimizer = true, bool bAdversarial = false);
		session(ptr lhs);
		virtual ~session();

	public:
		virtual void Run(Sampler * spTrain, Sampler * spTest, int nBatch=100, int nEpochSz=20, layer::eTrainSpeed speedMode = layer::eTrainSpeed::LowMemory);
		virtual void Test(Sampler * spTest, int nBatch=100, layer::eTrainSpeed speedMode = layer::eTrainSpeed::LowMemory, bool bUseHalf = false);
		virtual void Stop();
		virtual cuFloat::ptr Inference(cuFloat::ptr inData, bool half_mode = false);
		virtual cuShort::ptr Inference(cuShort::ptr inData);

		int DownSamplingRate(){ return _Nets[0]->down_sampling_rate(); }
		bool LoadFromFile(CString sPath);
		bool SaveToFile(CString sPath, bool bExportOptimizerParam=true);
		void NetAttach(network::ptr Nets, loss_func::ptr loss);

		void ChangeLossFunc(int i, loss_func::ptr NewLossFunc)
		{
			// seesion
			if(i==_LossFunc.size())
			_LossFunc.emplace_back(NewLossFunc);
			else
			_LossFunc[i] = NewLossFunc;

			// network
			_Nets[i]->loss_ptr(NewLossFunc);
		}

		void use_gpu(int id, int cnt) { _st_gpu_id = id; _use_gpu_cnt = cnt; runtime::set_gpu(_st_gpu_id); }
		int gpu_id() { return _st_gpu_id; }
		int gpu_cnt() { return _use_gpu_cnt; }

		void model_id(CString InspObject, CTime st_time)
		{
			CString sDateNTimeInfo;
			sDateNTimeInfo.Format(_T("%s%s_%s_%s_%s"), st_time.Format(_T("%y")), InspObject, st_time.Format(_T("%m%d")), st_time.Format(_T("%H%M")), st_time.Format(_T("%S")));
			_model_id = sDateNTimeInfo;
		}
		CString model_id(){ return _model_id; }
		void model_id_clear() { _model_id = _T(""); }

		float WeightDecay() { return _fWDecayLamda; }
		void WeightDecay(float val) { _fWDecayLamda = val; }
		float LearnRate() { return optimizer::_LearnRate; }
		int BatchSize() { return _nBatch; }
		int TrainingCount() { return _nTrainingEpoch; }
		int CurrentTrainEpoch() { return _trainedEpoch; }
		void UseUniformClassRate(bool _bUse) { _bUniformClsRate = _bUse; }
		bool UseUniformClassRate() {return _bUniformClsRate; }
		void ClassRate(std::vector<double> rate) { _classRate = rate; }
		std::vector<double> ClassRate() { return _classRate; }
		bool UseSamplingThread() {return _bUseSamplingThread; }
		void UseSamplingThread(bool _bUse) { _bUseSamplingThread = _bUse; }
		bool TrainMultiClass() {return _bTrainMultiClass; }
		void TrainMultiClass(bool _bUse) { _bTrainMultiClass = _bUse; }
		int LabelOffset() {return _LabelOffset;}
		void LabelOffset(int offset) { _LabelOffset = offset; }
		
		statistics_base::ptr _stats;
		std::function<void(int, float, std::vector<float> losses) > _Loss;
		std::function<void(int, float)> _Result;
		std::function<void(void)> _Finish;

		std::vector<network::ptr> _Nets;
		bool _bSaveImg, m_bUseEnsemble, _bDrawAllboxes;
		bool _bUseHalf;
		bool _bAdversarial;
		std::map<int, stGroupTrainingItem> _ATItems;

	protected:
		session()
		{

		}
		virtual void Serialize(archive & ar);

		void _CheckAdversarial(float loss, int nBatchIdx);

		static UINT _TrainProcF(void * pThis);
		virtual UINT _TrainProc();
		virtual void _Train();

		static UINT _ValidProcF(void * pThis);
		virtual UINT _ValidProc(int e);
		virtual UINT _ValidProc_s(int e);
		virtual cuFloat::ptr _Valid(Sampler * sampler);
		virtual cuFloat::ptr _Valid_s(Sampler * sampler);

		virtual void AllocBuf(int nBatch, Sampler * sampler);
		static UINT _GetBatchProcF(void * pThis);
		virtual UINT _GetBatchProc();
		static UINT _GetBatchValidF(void * pThis);
		virtual UINT _GetBatchValid();

		CWinThread * _pTrainThread, * _pBatchGenThread;
		bool _bTrainRunFlag;
		bool _bNeedNewBatch;
		int _st_gpu_id;
		int _use_gpu_cnt;

		std::vector<params::ptr> _params;
		std::vector<buffers::ptr> _buffers;
		std::vector<loss_func::ptr> _LossFunc;
		std::vector<cuFloat::ptr> _inMat, _inSubMat;
		std::vector<cuShort::ptr> _inMat_s, _inSubMat_s;

		CString _model_id;

		float _fWDecayLamda;		
		int _nBatch;
		int _nTrainingEpoch;
		int _trainedEpoch; // 현재 학습중인 에폭 - Serialize 시, model ID의 epoch part 기록용
		bool _bUniformClsRate; // Mini-batch 샘플링 비율 지정 여부 
		std::vector<double> _classRate; // Mini-batch 샘플링 비율
		int _nTrainBatches; // Train Set의 batch 단위 크기
		int _nTestBatches; // Valid Set의 batch 단위 크기
		int _LabelOffset; // 실제 라벨값과 학습 클래스 차이 조절 인자 (Loss & Backprop 시 적용)

		bool _bChangeOptimizer; // Model Load 시, 저장된 Optimizer 종류 로드 여부
		bool _bTrainMultiClass; // 학습 대상의 멀티클래스 여부
		bool _bUseSamplingThread; // 샘플링스레드 사용여부
		bool _bGenTrainBatch; // 학습용 (Train / Valid) Batch 우선 생성 모드
		int _samplingBatches[2]; // 샘플링된 batch 수
		std::vector<int> _spCntBuffer;
		std::vector<Sampler *> _spTrainBuffer, _spTestBuffer;
		std::vector<bool> _spTrainBufferFlag, _spTestBufferFlag;

		Sampler * _spTrain, * _spTest;
		cuFloat::ptr _errMat;
		cuShort::ptr _errMat_s;
	};
}