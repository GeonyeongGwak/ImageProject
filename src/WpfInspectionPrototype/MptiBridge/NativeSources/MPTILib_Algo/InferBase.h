#pragma once

#ifdef _AITester
#include "Valiables.h"  //debugging internal
#include "../AIWrapper/AI_TypeDef.h"
#else
#include "./CSML/AI_TypeDef.h"
#endif

class InferBase // CSML & pyTorch 공용 인터페이스
{
public:
	InferBase()
	{
		_saveDebugImg = false;
		_AINGImgPath = _T("D:\\AITest");
	}

public:
	virtual int GPUCount() = 0;

	virtual std::vector<int> GetDevice() = 0;
	virtual int GetDevice(int sessionIdx) = 0;
	
	virtual bool LoadModel(CString path, std::function<int(int)> selDev = nullptr) = 0; // CPU 모드(nullptr)
	
	virtual bool EmptyModel() = 0;
	virtual bool EmptyModel(int netID) = 0;

	virtual CString ModelID(int idx = 0) = 0;	

	virtual void init() = 0;
	virtual void init(std::shared_ptr<InferInit> ptr)
	{
		_saveDebugImg = ptr->saveDebugImg;
		_AINGImgPath = ptr->AINGImgPath;

		_block = ptr->block;
		_imgLen = ptr->imgLen;
		_imgWid = ptr->imgWid;
		_imgCh = ptr->imgCh;

		_init_time = ptr->init_time;
		_preprocess_time = ptr->preprocess_time;
		_inference_time = ptr->inference_time;
		_postprocess_time = ptr->postprocess_time;
		_res_save_time = ptr->res_save_time;
		_total_infer_time = ptr->total_infer_time;
	};

	virtual std::shared_ptr<InferResult> Infer(std::vector<cv::Mat> imgBuffer, int sessionIdx = 0) = 0;
	
public:
	bool _saveDebugImg; // 디버그 이미지 저장 여부
	CString _AINGImgPath; // 디버그 이미지 저장 경로

	double _init_time;
	double _preprocess_time;
	double _inference_time;
	double _postprocess_time;
	double _res_save_time;
	double _total_infer_time;

	int _block;
	int _imgWid, _imgLen, _imgCh;
};