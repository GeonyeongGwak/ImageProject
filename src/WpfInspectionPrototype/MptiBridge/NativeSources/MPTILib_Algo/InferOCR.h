#pragma once
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <atlstr.h>
#include <vector>
#include <stdio.h>

#include <random>
#include <vector>
#include <string>
#include "InferCore.h"

#ifdef _AITester
#include "../AIWrapper/pSession.h"
#else
#include "./CSML/pSession.h"
#include "./CSML/AI_TypeDef.h"
#endif

class CRAFT_InferCore
{
public:
	CRAFT_InferCore() //积己磊
	{

	}
	~CRAFT_InferCore() //家戈磊
	{
		for (int session_idx = 0; session_idx < this->Sessions.size(); session_idx++)
		{
			// Session 昏力
			awm::get().ai_remove_file(this->Sessions[session_idx]);
		}
	}

	int GPUCount();
	int GetDevice(int session_idx);
	bool LoadModel(CString ai_file_path, std::function<int(int)> selectDev);
	bool EmptyModel();
	void init();
	std::vector<cv::Mat> Infer(std::vector<cv::Mat> imgBuffer);

	std::vector<isession *> Sessions;

	int _batch_size;
};

class InferOcrCmp : public InferCore
{
public:
	InferOcrCmp()
	{
		_imgWid = 224;
		_imgLen = 224;
		_saveDebugImg = false;
		_imgCh = 1;
	}

public:
	cv::Mat Preprocess(cv::Mat srcImg);
	//void Postprocess(cv::Mat res, cv::Mat &dst, bool bAuto = false);
	std::shared_ptr<InferResult> Infer(std::vector<cv::Mat> imgBuffer, int sessionIdx = 0) override;
	void SaveReport(CString report_save_path);

private:
	int offset_x, offset_y;
};

class PyOCR_InferCore
{
public:
	PyOCR_InferCore()
	{

	}
	~PyOCR_InferCore()
	{
		for (int session_idx = 0; session_idx < this->Sessions.size(); session_idx++)
		{
			awm::get().ai_remove_file(this->Sessions[session_idx]);
		}
	}
public:
	int GetDevice(int session_idx);
	bool LoadModel(CString ai_file_path, int SelectGPUDevice);
	bool EmptyModel();
	void Init();
	std::vector<cv::Mat> Infer(std::vector<cv::Mat> imgBuffer);
	CString get_model_id();
	void set_model_id(CString ai_file_path);
private:
	std::vector<isession*> Sessions;
	CString model_id;
};