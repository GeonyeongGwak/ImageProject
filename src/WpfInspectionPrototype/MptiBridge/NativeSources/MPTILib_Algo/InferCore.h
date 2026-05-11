#pragma once

#include "awm.h"
#include "tick.h"
#include <vector>
#include <string>
#include <fstream>
#include <functional>
#include "InferBase.h"

#define _nOCRRes 4

class InferCore : public InferBase
{
public:
	template<template <typename> class P = std::greater >
	struct compare_pair_second {
		template<class T1, class T2> bool operator()(const std::pair<T1, T2>&left, const std::pair<T1, T2>&right) {
			return P<T2>()(left.second, right.second);
		}
	};

public:
	InferCore()
	{
		_bIndependent = false;

		_block = 100;
		_blurFilterSz = 61;

		_imgLen = 256;
		_imgWid = 256;
		_imgCh = 1;

#ifdef _AITester
		Variables::aiTesterFile.CstStatusMsg->On = _CstStatusMsg;
#endif
	}
	virtual ~InferCore()
	{
		_Session.clear();
	}

public:
#ifdef _AITester
	typedef InferCore ThisT;
	jsl::Binder<ThisT, CString>::Ptr _CstStatusMsg = jsl::newBinder<ThisT, CString>(this, nullptr);
#endif

public:
	int GPUCount() override;

	bool LoadModel(CString path, std::function<int(int)> selDev = nullptr) override;
	virtual bool LoadModelSimple(CString path, int nGpuidx); // CPU 모드(nullptr)
	bool EmptyModel() override;
	bool EmptyModel(int netID) override;
	CString ModelID(int idx = 0) override;
	void init() override;
	void init(std::shared_ptr<InferInit> ptr) override;
	virtual void UpdateDevice(int gpuIdx = 0, int sessionIdx = -1); // sessionIdx : -1 일 때, 모든 세션의 Device 일괄 변경
	std::vector<int> GetDevice() override;
	int GetDevice(int sessionIdx) override;

	virtual std::shared_ptr<InferResult> Infer(std::vector<cv::Mat> imgBuffer, int sessionIdx = 0);
	virtual std::vector<std::vector<cv::Mat>> vInfer(std::vector<cv::Mat> imgBuffer, bool bIndependent = false, int sessionIdx = 0);
	virtual std::vector<cv::Mat> InferRT(std::vector<cv::Mat> imgBuffer, int sessionIdx = 0) { return std::vector<cv::Mat>(); }
#ifdef _AITester
	virtual std::shared_ptr<InferResult> Infer(std::vector<CString> cstPaths, int sessionIdx = 0);
	virtual std::vector<std::vector<cv::Mat>> vInfer(std::vector<CString> cstPaths, bool bIndependent = false, int sessionIdx = 0);
	virtual std::vector<cv::Mat> InferRT(std::vector<CString> cstPaths, int sessionIdx = 0) { return std::vector<cv::Mat>(); }
	void MakeBatchFromVectorPaths(std::vector<CString> vInputList, bool bInputChannelsSplitted = false);
#endif
	virtual CString GetCurrentDebugImgPath(); // 최근에 사용된 디버그 이미지 저장 경로 출력용

	cv::Mat RestoreImg(cv::Mat res, int n, int nCh, bool bDenormalizeAuto = true);
	void Preprocess(cv::Mat inFloat);
	void MakeBatchFromVector(std::vector<cv::Mat> vInMat, bool bInputChannelsSplitted = false);
	

public:
	bool _bIndependent;

	int _blurFilterSz;
	std::vector<int> szImg, _srcInfo;

	std::vector<CString> modelPath;
	std::vector<CString> modelIDBuf;

	cv::Mat _inDataMat;

	std::shared_ptr<InferInit> init_prop;

	std::vector<std::shared_ptr<isession>> _Session;
};
