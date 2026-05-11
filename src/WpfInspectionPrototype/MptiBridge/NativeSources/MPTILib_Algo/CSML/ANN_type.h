#pragma once

#include "mat_type.h"
#include "archive.h"
#include "params.h"
#include <opencv2\opencv.hpp>
#include <vector>
#include <memory>
#include <map>

namespace csml
{
	typedef float		val_type;
	enum eGANmode{
		none = 0,
		Generate = 1,
		Discriminate = 2
	};
	enum eInitializer {
		InitHe, InitXavier, InitStdev
	};
	enum ePool { MaxPool, AvgPool };
	enum ePad { NoPad, Same, Valid };
	enum eOdd { Even, Odd };
	enum eFusedActiMode { SIGMOID, RELU, TANH, CLIPPED_RELU, ELU, IDENTITY, NONE};

	//  CUDNN_ACTIVATION_SIGMOID = 0,
	//	CUDNN_ACTIVATION_RELU = 1,
	//	CUDNN_ACTIVATION_TANH = 2,
	//	CUDNN_ACTIVATION_CLIPPED_RELU = 3,
	//	CUDNN_ACTIVATION_ELU = 4,
	//	CUDNN_ACTIVATION_IDENTITY = 5

	class AFX_EXT_CLASS optimizer
	{
	public:
		typedef std::shared_ptr<optimizer> ptr;
	public:
		virtual void init() = 0;
		virtual void update(cuFloat::ptr grad, cuFloat::ptr & Res);
		virtual optimizer::ptr clone() = 0;
		virtual void Serialize(archive& ar)=0;

	public:
		static float _LearnRate;
		static float _WeightLimit;
		static bool _bExportToFile;
	};
	class AFX_EXT_CLASS OptimizerFactory
	{
	public:
		typedef enum _ENM_OPTIMIZER { OptSGD, OptAdam } enmOptimizer;
	public:
		static optimizer::ptr Alloc();
		static enmOptimizer _SelectOptimizer;
		static bool _bLoadParam; // Load Model 시, Optimizer Param 로드 여부 
	};


	class AFX_EXT_CLASS loss_func
	{
	public:
		typedef std::shared_ptr<loss_func> ptr;
		typedef std::weak_ptr<loss_func> wtr;

	public:
		loss_func(CString sLayerName)
		{
			_sLayerName = sLayerName;
			_lossVal = 0.f;
			_scale = 1.f;
			_labelOffset = 0;
			_bUseMultiClass = false;
			_bUseTensorCore = false;
			_nGANmode = csml::eGANmode::none;
		}
		loss_func(loss_func & obj);
		loss_func(archive & ar)
		{
			_lossVal = 0.f;
			_scale = 1.f;
			_labelOffset = 0;
			_bUseMultiClass = false;
			_bUseTensorCore = false;
			_nGANmode = csml::eGANmode::none;
			Serialize(ar);
		}
		loss_func() {
			_lossVal = 0.f;
			_scale = 1.f;
			_labelOffset = 0;
			_bUseMultiClass = false;
			_bUseTensorCore = false;
			_nGANmode = csml::eGANmode::none;
		}

	public:
		virtual ptr clone() = 0;
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr())=0;
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D = cuFloat::ptr(), cuStream::ptr sync = cuStream::ptr()) = 0;
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void errorsum();
		virtual void Serialize(archive & ar);
		virtual CString name() { return _T("None"); }
		virtual void setL(cv::Mat & L);
		virtual void setL_s(cv::Mat & L);
		virtual void setW(cv::Mat & W);
		virtual void download();
		cuFloat::ptr _E;
		cuFloat::ptr _L;
		cuFloat::ptr _Lsub;
		cuFloat::wtr _Y;
		cuFloat::ptr _Out;
		cuFloat::ptr _W; // Loss 계산 시 적용될 클래스별 Weight
		cuFloat::ptr _ResBuffer;

		cuShort::ptr _Es;
		cuShort::ptr _Ls;
		cuShort::ptr _Lsubs;
		cuShort::wtr _Ys;
		cuShort::ptr _Outs;
		cuShort::ptr _ResBuffers;

		cv::Mat _hL, _hW, _replaceIn, _replaceLabel;
		std::vector<std::vector<cv::Point3d>> _coordis;

		float _scale;
		float _lossVal;

		int _labelOffset; // 실제 Label 값과 학습 클래스 차이 조절 인자 (Loss & Backprop 시 적용)

		bool _bUseFirst;
		bool _bUseMultiClass; // 멀티 클래스 학습 여부 -> Cross Entropy 계산에 활용
		bool _bUseTensorCore; // loss_func::cal_loss 코드 - GPU 종류에 따라 분할(RTX / GTX) 

		int _nGANmode;
	protected:
		cuByte::ptr _CalcBuffer;
		CString _sLayerName;
	};
	class AFX_EXT_CLASS CrossEntropy : public loss_func
	{
	public:
		CrossEntropy(CString sLayerName)
			: loss_func(sLayerName)
		{
		}
		CrossEntropy(CrossEntropy & obj)
			: loss_func(obj)
		{
		}
		CrossEntropy(archive & ar)
			: loss_func(ar)
		{

		}
		loss_func::ptr clone()
		{
			return loss_func::ptr(new CrossEntropy(*this));
		}
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D, cuStream::ptr sync = cuStream::ptr());
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual CString name() { return _T("CrossEntropy"); }
	};
	class AFX_EXT_CLASS FocalLoss : public CrossEntropy
	{
	public:
		FocalLoss(CString sLayerName)
			: CrossEntropy(sLayerName)
		{
		}
		FocalLoss(CrossEntropy & obj)
			: CrossEntropy(obj)
		{
		}
		FocalLoss(archive & ar)
			: CrossEntropy(ar)
		{

		}
		loss_func::ptr clone()
		{
			return loss_func::ptr(new FocalLoss(*this));
		}
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D, cuStream::ptr sync = cuStream::ptr());
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync);
		virtual CString name() { return _T("FocalLoss"); }
	};
	class AFX_EXT_CLASS LogisticLoss : public loss_func
	{
	public:
		LogisticLoss(CString sLayerName, float weight = 1.f)
			: loss_func(sLayerName)
		{
			_weight = weight;
		}
		LogisticLoss(LogisticLoss & obj)
			: loss_func(obj)
		{
		}
		LogisticLoss(archive & ar)
		{
			Serialize(ar);
		}

		loss_func::ptr clone()
		{
			return loss_func::ptr(new LogisticLoss(*this));
		}
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D, cuStream::ptr sync = cuStream::ptr());
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void Serialize(archive& ar);
		virtual CString name() { return _T("LogisticLoss"); }
		float _weight;
	};
	class AFX_EXT_CLASS MeanSquaredError : public loss_func
	{
	public:
		MeanSquaredError(CString sLayerName)
			: loss_func(sLayerName)
		{
		}
		MeanSquaredError(MeanSquaredError & obj)
			: loss_func(obj)
		{
		}
		MeanSquaredError(archive & ar)
			: loss_func(ar)
		{

		}

		loss_func::ptr clone()
		{
			return loss_func::ptr(new MeanSquaredError(*this));
		}
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D, cuStream::ptr sync = cuStream::ptr());
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual CString name() { return _T("MeanSquaredError"); }
	};
	class AFX_EXT_CLASS YoloLoss : public loss_func
	{
	public:
		YoloLoss(CString sLayerName, float ObjectnessLamda, float SizeLamda)
			: loss_func(sLayerName)
		{
			_ObjectnessLamda = 5;
			_SizeLamda = 10;
		}
		YoloLoss(YoloLoss & obj)
			: loss_func(obj)
		{
			_ObjectnessLamda = obj._ObjectnessLamda;
			_SizeLamda = obj._SizeLamda;
		}
		YoloLoss(archive & ar)
		{
			Serialize(ar);
		}
	public:
		virtual loss_func::ptr clone()
		{
			return loss_func::ptr(new YoloLoss(*this));
		}
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D, cuStream::ptr sync = cuStream::ptr());
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void Serialize(archive& ar);
		virtual CString name() { return _T("YoloLoss"); }
	protected:
		float _ObjectnessLamda;
		float _SizeLamda;
	};
	class AFX_EXT_CLASS Yolov2Loss : public loss_func
	{
	public:
		Yolov2Loss(CString sLayerName, float ObjectnessLamda, float SizeLamda, int nBoxesInCell, int nClasses)
			: loss_func(sLayerName)
		{
			_ObjectnessLamda = 5;
			_SizeLamda = 10;
			_nBoxesInCell = 2;
			_nClasses = nClasses;
		}
		Yolov2Loss(Yolov2Loss & obj)
			: loss_func(obj)
		{
			_ObjectnessLamda = obj._ObjectnessLamda;
			_SizeLamda = obj._SizeLamda;
			_nBoxesInCell = obj._nBoxesInCell;
			_nClasses = obj._nClasses;
		}
		Yolov2Loss(archive & ar)
		{
			Serialize(ar);
		}
	public:
		virtual loss_func::ptr clone()
		{
			return loss_func::ptr(new Yolov2Loss(*this));
		}
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D = cuFloat::ptr(), cuStream::ptr sync = cuStream::ptr());
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void Serialize(archive& ar);
		virtual CString name() { return _T("Yolov2Loss"); }
	protected:
		float _ObjectnessLamda;
		float _SizeLamda;
		int _nBoxesInCell;
		int _nClasses;
	};
	class AFX_EXT_CLASS rpnLoss : public loss_func
	{
	public:
		rpnLoss(CString sLayerName, int nAnchors)
			: loss_func(sLayerName)
		{
			_nAnchors = nAnchors;
		}
		rpnLoss(rpnLoss & obj)
			: loss_func(obj)
		{
			_nAnchors = obj._nAnchors;
		}
		rpnLoss(archive & ar)
		{
			Serialize(ar);
		}
	public:
		virtual loss_func::ptr clone()
		{
			return loss_func::ptr(new rpnLoss(*this));
		}
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D = cuFloat::ptr(), cuStream::ptr sync = cuStream::ptr());
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void Serialize(archive& ar);
		virtual CString name() { return _T("rpnLoss"); }
	protected:
		int _nAnchors;
	};

	class AFX_EXT_CLASS LossLinear : public loss_func
	{
	public:
		LossLinear(CString sLayerName)
			: loss_func(sLayerName)
		{
			_sequences = -1;
		}
		LossLinear(LossLinear & obj)
			: loss_func(obj)
		{
			_sequences = -1;
		}
		LossLinear(archive & ar)
		{
			Serialize(ar);
			_sequences = -1;
		}

		loss_func::ptr clone()
		{
			return loss_func::ptr(new LossLinear(*this));
		}
		virtual void predict(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void back_prop(cuFloat::ptr Y, cuFloat::ptr D, cuStream::ptr sync = cuStream::ptr());
		virtual void calc_loss(cuFloat::ptr Y, cuStream::ptr sync = cuStream::ptr());
		virtual void Serialize(archive& ar);
		virtual CString name() { return _T("LossLinear"); }
		int _sequences;
		cuFloat::ptr _Dreal;
		cuFloat::ptr _Dfake;
	};

	// Layers...
	class AFX_EXT_CLASS layer
	{
	public:
		typedef std::shared_ptr<layer> ptr;
		typedef std::weak_ptr<layer> wptr;
		typedef std::vector<ptr> nets;
		typedef std::vector<wptr> wnets;

	public:
		typedef enum _eLayerType { eLtConv, eLtPool, eLtTransform, eLtAffine, eLtActivate, eLtDropout, eLtNorm, eLtOutput, eLtRuntime, eLtScaleChannelwise } eLayerType;
		typedef enum _eLayerDepth { eLdInput, eLdHidden, eLdOutput } eLayerDepth;
		typedef enum _eRunMode { eRmTrain, eRmInference } eRunMode;
		typedef enum _eTrainSpeed { LowMemory, HighSpeed } eTrainSpeed;

	public:
		// calc functions...
		virtual void init() { }
		virtual void init_connection();
		virtual cuFloat::ptr predict(cuFloat::ptr src)
		{
			return forward(src);
		}
		virtual cuShort::ptr predict(cuShort::ptr src)
		{

			return predict_s();
		}
		virtual cuFloat::ptr predict();
		virtual cuShort::ptr predict_s();
		virtual cuFloat::ptr forward(cuFloat::ptr) = 0;
		virtual cuFloat::ptr forward();
		virtual cuFloat::ptr backward(cuFloat::ptr, bool multi_gpu_chk = false) = 0;
		virtual cuFloat::ptr backward(bool multi_gpu_chk = false);
		virtual std::vector<cv::Mat> Visualize();
		virtual void update(float Wdecay_lamda=0) { }

		virtual void erase_param() {}

		virtual layer::ptr clone(params::wtr param= params::wtr(), buffers::wtr buffer = buffers::wtr()) = 0;
		virtual void Serialize(archive& ar)=0;


		// multi-gpu functions...
		//virtual void sum_grad(ptr layer, float fScale, bool bFirst) {}
		virtual void sum_grad(ptr layer) {}
		virtual void update_params(int gpu_cnt) {}

		// properties...
		virtual eLayerType type() = 0;

		virtual CString name() = 0;
		CString lname() { return _sName; }

		std::vector<CString> InputList() { return _sInsSel; }
		void InputList(std::vector<CString> newInput) { _sInsSel = newInput; }

		virtual int down_sampling_rate() { return 1; }

		virtual void loss_ptr(loss_func::ptr val) {  }
		virtual loss_func::ptr loss_ptr() { return loss_func::ptr(); }

		virtual std::vector<CString> ParamList() { return std::vector<CString>(); }
		virtual void setValue(CString name, CString val) {  }
		virtual CString getValue(CString name) { return CString(); }

		void params_ptr(params::wtr val) { _params = val; }
		params::ptr params_ptr() { return _params.lock(); }

		void buffers_ptr(buffers::wtr val) { _buffers = val; }
		buffers::ptr buffers_ptr() { return _buffers.lock(); }

		void update_flag(bool val) { _bUpdateParam = val; }
		bool update_flag() { return _bUpdateParam; }

		void backward_flag(bool val) { _bCalcGradient = val; }
		bool backward_flag() { return _bCalcGradient; }

		void group(int val) { _group = val; }
		int group() { return _group; }

		static eTrainSpeed _TrainSpeed;
	protected:
		layer();
		void IdxSel_Init()
		{
			_sIdxSel.resize(_sInsSel.size());
		}
		cuFloat::ptr _Add;
		cuFloat::ptr _dAdd;
		cuShort::ptr _Add_s;
		cuShort::ptr _dAdd_s;
		params::wtr _params;
		buffers::wtr _buffers;
		int _group;
	public:
		std::vector<CString> _sInsSel;
		std::vector<int> _sIdxSel;
		CString _sName;
		CString _sNameSub;
		bool _bUpdateParam;
		bool _bUseBias;      // 20190125 MSH - ConvTranspose 인자 옮겨옴
		bool _bCalcGradient;
	};
}