#pragma once
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <atlstr.h>
#include <vector>
#include <stdio.h>

#include <random>
#include "tick.h"
#include <vector>
#include <string>
#include "InferCore.h"

#ifdef _AITester
#include "../AIWrapper/pSession.h"
#else
#include "./CSML/pSession.h"
#include "./CSML/AI_TypeDef.h"
#include "csml_header.h"

class InferBubbleCSML : public InferBase
{
public:
	
	InferBubbleCSML() {
		////// Inspection Initial Setting [***** User Setting *****]
		_gpuId = 0;
		_gpuCnt = 1;
	
		////// AI Model 학습환경 관련 세팅 (default)
		_block = 1;
		_imgCh = 1;

		////// Inference 관련 내부 인자
		_binary_threshold = 200;
		half_mode = false;
		_bLoadModel = false;
	};
	~InferBubbleCSML() {
	
	}
	
public:
	int _gpuId;
	int _gpuCnt;
	int totalGpuCount; // 장비 GPU 개수

	bool half_mode;  // Inference 시, 내부적으로 half precision으로 변경하여 진행
	bool _bLoadModel;

	int _binary_threshold;

public:
	std::vector<int> GetDevice() override;
	int GetDevice(int sessionIdx) override;

	CString ModelID(int mode) override;
	bool LoadModel(CString path, std::function<int(int)> selDev = nullptr) override; // CPU 모드(nullptr)

	void ReleaseModel();
	void ReleaseModel(int netID);

	bool EmptyModel() override;
	bool EmptyModel(int netID) override;

	int GPUCount();

	void init() override;
	void init(std::shared_ptr<InferInit> ptr) override;
	cv::Mat ZeroPaddingImg(cv::Mat FOV);
	std::shared_ptr<InferResult> Infer(std::vector<cv::Mat> imgBuffer, int sessionIdx = 0) override;
	void SaveReport(CString report_save_path, int AddedSaveName);

protected:
	csml::cuByte::ptr inImg;
	csml::cuFloat::ptr inMat;	// 입력 이미지 버퍼
	csml::cuFloat::ptr postprocessing_buffer; // 후처리용 버퍼

	csml::session::ptr _session;

	CString modelPath;

	int downSamplingCnt;
	int padded_width, padded_height;
	int gap_width, gap_height;
	
	std::shared_ptr<InferInitBubble> init_prop;
};
#endif

class InferBubbleRT : public InferCore
{
public:
	InferBubbleRT()
	{
		_imgCh = 1;
		_binary_threshold = 180;
	}

public:
	int _binary_threshold;

	void init() override;
	void init(std::shared_ptr<InferInit> ptr) override;
	cv::Mat Preprocess(cv::Mat srcImg);
	void Postprocess(cv::Mat res, cv::Mat &dst, bool bAuto = false);
	std::shared_ptr<InferResult> Infer(std::vector<cv::Mat> imgBuffer, int sessionIdx = 0) override;
	void SaveReport(CString report_save_path, int AddedSaveName);

private:
	int offset_x, offset_y;
};

class InferBubble : public InferCore
{
public:
	InferBubble()
	{
		_imgCh = 1;
		_block = 1;
		_binary_threshold = 170;
	}

public:
	int _binary_threshold;

	cv::Mat Preprocess(cv::Mat srcImg);
	//cv::Mat BubbleInputMatrix(std::vector<cv::Mat> srcBuffer, int stPos, int nCnt);
	//std::vector<cv::Mat> BubbleInference(std::vector<cv::Mat> ImgBuffer);
	void Postprocess(cv::Mat res, cv::Mat &dst, bool bAuto = true);
	std::shared_ptr<InferResult> Infer(std::vector<cv::Mat> imgBuffer, int sessionIdx = 0) override;
	void SaveReport(CString report_save_path, int inspCount);

private:
	int offset_x, offset_y;
};