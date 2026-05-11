#pragma once

typedef struct _PreperenceItems
{

}PreperenceItems;

typedef struct _BubbleItems : public PreperenceItems
{
public:
	_BubbleItems()
	{
		threshold = -1;
	}

	int threshold;
}BubbleItems;

typedef struct _PartsCnterItems : public PreperenceItems
{
public:
	_PartsCnterItems()
	{
		th_min = 0;
		th_max = 65535;
	}

	int th_min, th_max;
}PartsCnterItems;

class isession
{
public:
	isession()
	{
		_bLoadModel = false;
		_bGPU = true;
		_bHalfMode = false;

		_nGpuId = -1;
		_cstDB = _T("");
		_cstID = _T("");
	}
	virtual ~isession()
	{

	}

	virtual void init() {}
	virtual void exit() {}
	virtual void SetPreperenceItems(PreperenceItems * pItems) {  }
	virtual cv::Mat Inference(cv::Mat In) { return cv::Mat(); }

	virtual std::vector<cv::Mat> vInference(cv::Mat src) { return std::vector<cv::Mat>(); }
	virtual bool bInference(std::vector<cv::Mat> src) { return false; }		// OCR binary classification

	virtual void set_device(int gpuIdx = -1) { _bGPU = gpuIdx >= 0 ? true : false; _nGpuId = gpuIdx; }
	virtual void set_precision(bool bHalfMode) { _bHalfMode = bHalfMode; }
	virtual bool check_model_ready() { return _bLoadModel; }
	virtual size_t get_gpu_count() { return 0; }
	virtual int get_device() { return _nGpuId; }
	virtual bool check_half_precision() { return _bHalfMode; }

	CString _cstDB;
	CString _cstID;
	
protected:
	bool _bLoadModel; // Check Model Load Success
	bool _bGPU;
	bool _bHalfMode;
	int _nGpuId; // CPU(-1), GPU(0~)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region CA(CountAccessory)
public:
	virtual int CA_GetCnt() { return 0; }
	virtual void CA_GetItems(float* hostFloatSrc, int* hostPeakSrcX, int*hostPeakSrcY, int* hostPeakDstX, int* hostPeakDstY,
		int* hostDist, float* hostSobelX, float* hostSobelY, unsigned char* hostHeat, /*unsigned char* hostGrad,*/
		unsigned char* hostPeak/*, unsigned char* hostHeatGradSub*/) { return; }
	//virtual void CA_ABSSUB_HeatGrad(uchar* dst) { return; }
	virtual void CA_FindPeaks(uchar* heat, uchar* dst, uchar* src,int threshold, int offset) { return; }
	virtual int CA_FindNearestPT(float* dist, int &nPts, int* srcx, int* srcy, int* usage, int searchRange, int wid, int len, bool single_mode) { return 0; }
	virtual void CA_DrawPT(int nPts, int* srcx, int* srcy, int* usage, uchar* dst = nullptr) { return; }
	virtual void CA_Sobel_Bidirectional(float* imgData, int wid, int len) { return; }	
#pragma endregion
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};