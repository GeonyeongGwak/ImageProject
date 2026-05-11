#pragma once
#include <Windows.h>
#include "csml_header.h"
#include <opencv2/opencv.hpp>
#include <atlstr.h>
#include <vector>
#include <stdio.h>
#include <random>
#include "Blob.h"
#include "awm.h"
#include "InferOCR.h"

#define _nOCRRes 4 // OCR 결과 반환 개수

typedef enum _eRangeType_ { rangeIn = 0, rangeOut = 1, rangeUpper = 2, rangeLower = 3 } rangeType;
typedef enum _eNetworkType_ { Classification, Generation, Segmentation, BoxDetection, RPN, SegCoil, ClsCoil, ClsOCR, GANCoil } netType;
typedef enum _eObjType_ { Bubble, CadGerber, Coil, OCR } objType;

namespace csml
{
	struct RoiInfo
	{
		int left;
		int top;
		int width;
		int height;
		int classNum;
	};

	class Inference
	{
	public:

		Inference() {
			////// Inspection Initial Setting [***** User Setting *****]
			_gpuId = 0;
			_gpuCnt = 1;

			_block = 100;
			binary_threshold = 240;

			save_chk = true;	// FOV, Label, test report 저장여부	
			BubbleNGImgPath = _T("D:\\BubbleTest");	// 결과 저장 Root 경로				

			////// AI Model 학습환경 관련 세팅 (default)
			_imgCh = 1;
			_imgLen = 112;
			_imgWid = 112;
			preprocess_mode = 0;	// 0 - 전처리 없음, 1 - 평균으로 나눔
			inference_mode = 1;		// 0 - Classification , 1 - Segmentation, 2 - Cls & Seg

			////// Inference 관련 내부 인자
			half_mode = false;
			load_model_chk = false;

			init();

			modelPath.clear();
			modelPath.emplace_back("");
			modelPath.emplace_back("");
		};
		~Inference() {

		}

	public:
		int _gpuId;
		int _gpuCnt;

		int _block;

		std::vector<CString> modelPath;
		CString BubbleNGImgPath; // save_path 인자 이름 변경 

		int totalGpuCount; // 장비 GPU 개수

		bool half_mode;  // Inference 시, 내부적으로 half precision으로 변경하여 진행

		bool load_model_chk;
		bool save_chk;

		int binary_threshold;

		int inference_mode;
		int preprocess_mode;

		double init_time;
		double preprocess_time;
		double inference_time;
		double postprocess_time;
		double res_save_time;
		double total_infer_time;

	public:
		bool LoadModel(CString path);
		bool LoadModel(std::vector<CString> path);

		CString ModelID();
		CString ModelID(int mode);

		void ReleaseModel();
		void ReleaseModel(int netID); // 0 - Cls , 1 - Seg

		bool EmptyModel();
		bool EmptyModel(int netID);

		int GPUCount();

		void init();
		cv::Mat preprocess(cv::Mat ROI, int mode);
		cv::Mat ZeroPaddingImg(cv::Mat FOV);
		cv::Mat LabelMaker(cv::Mat FOV, CString SaveNum = _T(""));
		virtual void SaveReport(CString report_save_path, CString AddedSaveName);

	protected:

		int _imgCh;
		int _imgLen;
		int _imgWid;

		csml::cuByte::ptr inImg;
		csml::cuFloat::ptr inMat;	// 입력 이미지 버퍼
		csml::cuFloat::ptr postprocessing_buffer; // 후처리용 버퍼

		csml::session::ptr _SessionCls;
		csml::session::ptr _SessionSeg;

		int downSamplingRate;
		int padded_width, padded_height;
		int gap_width, gap_height;
	};

	class InferOCR : public Inference
	{
	public:
		InferOCR()
		{
			inference_mode = 0; // 분류모델
			preprocess_mode = 0; // 전처리 없음

			_imgLen = 224;
			_imgWid = 224;

			blurFilterSz = 61;

			save_chk = false;

			modelPath.clear();	// AI Net Model 경로
			modelPath.emplace_back("");

			OCRNGImgPath = _T("D:\\OCRTest"); // 결과 저장 Root 경로
		};
		~InferOCR()
		{

		};

	public:
		template<template <typename> class P = std::greater >
		struct compare_pair_second {
			template<class T1, class T2> bool operator()(const std::pair<T1, T2>&left, const std::pair<T1, T2>&right) {
				return P<T2>()(left.second, right.second);
			}
		};

		cv::Mat OCRInputMatrix(cv::Mat srcImg);
		cv::Mat OCRInputMatrix(std::vector<cv::Mat> srcBuffer, int stPos, int nCnt);
		UINT16 ClassNumToAscii(int i);
		std::vector<cv::Mat> OCRInference(std::vector<cv::Mat> ImgBuffer); // 샘플단위 클래스별 확률값 ([클래스 0~9] : 숫자 0~9, [클래스 10~35]: 대문자 A~Z)
		void SaveReport(std::vector<cv::Mat> res, CString report_save_path, std::vector<cv::Mat> ImgBuffer = std::vector<cv::Mat>());

		int blurFilterSz;
		CString OCRNGImgPath;
	};

	class InferCoil : Inference
	{
	public:
		InferCoil(int gpuSelected)
		{
			_bInit = false;
			_segTh = 0;
			_numSelectedGPU = gpuSelected;
			_bUseHalf = false;
			_bMemoryWarned = false;

			g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
		}
		~InferCoil()
		{
			Mem_freeAll();
			_coilBlob.Free();

			g_pMManager->pem_delete_check(this);
		}

	public:
		bool Check_Classification(int nCh);//네트워크 로드 여부 확인
		bool Check_Mixed();			//네트워크 로드 여부 확인
		bool Check_Segmentation();	//네트워크 로드 여부 확인
		void ClearImgs();
		//3ch 입력 segmentation, classification 각기 진행
		bool CoilMixResult(uchar* srcBR, uchar* srcTB, uchar* srcTW, uchar* mask, int srcW, int srcH, cv::Rect maskInfo, std::vector<uchar> & aiResultImg);
		int CoilRand();//랜덤번호
		//3가지 기능 내포한 코일검사//1. 3ch 입력 classification//2. 3ch 입력 segmentation//3. 3ch 입력 segmentation 이후 4ch classification 연계 진행
		bool CoilResult(uchar* srcBR, uchar* srcTB, uchar* srcTW, uchar* mask, int srcW, int srcH, cv::Rect maskInfo);
		bool CoilClsResult();
		void FlushDir(CString inspTime);
		void GetRangeCutImg(int L, int R, int type);//타입에 따라 min, max구간 임계값으로 이미지 이진화
		void Inference();
		void Init(int nDefects);//초기화
		int LoadModel(CString path, int nCh);//모델로드
		void Mem_freeAll();//기존메모리 해제
		void MoveUserOKPart(CString cstFilePath, CString cstFileName);
		cv::Mat RestoreImg(cv::Mat res, int n, int nCh = 0);//4채널 이미지로부터 n번째의 이미지 복원
		void ProcessWindowMessage();
		void RefreshFiltImg();
		int RemoveBlobs(cv::Mat src);//10,10보다 작은 블랍 제거
		void SavePartResultImg(CString cstFilePath, CString cstFileName, bool bSaveOK, bool bSaveNG);
		void WriteCSV(CString path, CString msg);//csv파일 작성

	protected:
		bool CoilDeviceImg(std::vector<cv::Mat> vMat, bool bOnce);//조명이미지로 4ch 입력이미지 생성(_inDataMat)
		void CoilPostProcess(csml::cuFloat::ptr res);//Segmentation 망통과 후처리작업
		void CoilClsPostProc(csml::cuFloat::ptr res);//Classification 망통과 후처리작업
		bool Coil2ndClsImg();//segmentation 이후 결함추정roi를 이용하여 분류연계용 입력 이미지 생성
		void Coil2ndFilterProc(csml::cuFloat::ptr res);//연계이후 후처리작업
		void RefreshCoilResultImg(cv::Mat srcImg);
		float GetSigmoidVal(float in);//시그모이드 값 계산
		void RemoveFolder(LPCTSTR pstr);
		void SaveGoodDB();
		void SaveSegLabel();
		void SaveSegResultImgs(cuFloat::ptr res, cv::Mat &dst);
		void Save_floatImg(cv::Mat src, cv::Mat dstImg);
		cv::Mat ZeroPaddingImg(cv::Mat FOV);
		std::vector<cv::Mat> ZeroPaddingImg(std::vector<cv::Mat> FOV);

	public:

		//ImgData
		cv::Mat _coilSegInput;
		cv::Mat _inDataMat;		//코일검사 입력 이미지
		cv::Mat _tempResult;	//코일검사 임계값 미적용 이미지;
		cv::Mat _tempResultBlob;//코일검사 적용, 작은블랍 제거 이미지;
		cv::Mat _tempResultBlob_RECTS;
		cv::Mat _thresholdedImg;//임계값 적용 이미지
		cv::Mat _LabelImg;		// 굿DB 생성시 비교대상 라벨이미지
		cv::Mat _2ndClsImg;		//2차 필터 결과 이진 이미지
		cv::Mat _densityMap;	//분포 확인용 map
		cv::Mat _curImg;
		cv::Mat _clsModelImg;
		//bool
		bool _bInit;		//코일 추론 모델 초기화 되었는지 여부
		bool _bMemoryWarned;	// 망 로드 메모리 체크하여 경고메시지 부여 여부(최초 1회만 메시지 띄우기)
		bool _bCoilGood;	// 코일검사 결과 양불판정 결과
		bool _bOnce;		//sliding할지 통 이미지로 진행할지 여부
		bool _bSegLabelOnly;// segmentation 결과저장여부
		bool _bGoodDB;		//good DB생성 여부
		bool _bLabelling;	//라벨이미지 저장 여부
		bool _bClassification;//classification여부
		bool _bUseHalf;		//하프플롯 사용여부
		bool _bSaveResultImg;
		bool _bPaddedSeg, _bPaddedCls, _bPadded;
		//int float double..
		int _curSequence;	//코일 현재 검사된 고유번호
		int _imgW, _imgH;	//입력윈도우크기
		int _mskX, _mskY;
		int _offX, _offY;
		int _mskW, _mskH;	//신경망결과크기
		int _nModelCh;		//입력채널수
		int _numSelectedGPU;//코일검사 진행하는 gpu번호
		int	_segTh;		//Segmentation 임계값
		int _blobLimitW, _blobLimitH, _blobLimitA, _blobLimitSzD; // Width, Height, Area, SizeDistance Limit
		float _clsTh;	//Classification 임계값
		float _tBit, _tAdjust, _tBounding, _tSeg, _tCls, _tSegCopy, _tSegPadding, _tClsCopy, _tClsPadding, _tBoundaryCheck, _tSegMat, _tClsImgPre;
		int _nSerialNumber;
		double rstWid;
		int _modelW, _modelH, _modelRealW;
		int _fovW, _fovH;
		//vector
		std::vector<cv::Mat> _3chImg;			//조명이미지
		std::vector<CString> _moduleNo, _refID;
		std::vector<RECT> _rcCoilDefects;		//결함roi정보
		std::vector<std::vector<cv::Mat>> _inImgs;//코일검사 입력 채널 이미지
		std::vector<csml::RoiInfo> _RoiLists;	// good DB생성시 신경망 통과 결과의 블랍 정보
		std::vector<int> _minx, _maxx, _miny, _maxy;	//defect Rect정보
		std::vector<double> _rctArea;
		std::vector<double> _rstD;//size distance lists
		std::vector<int> _blobLabelNum;//blob label number lists
		std::vector<uchar> _gaugedImg;
		std::vector<std::vector<uchar>> _2ndFiltModelData;
		std::vector<float> _clsRes;
		//std::vector<string> _mText;
		//std::vector<cv::Point> _mPoint;
		//std::vector<cv::Rect> _clsRectList;
		//std::vector<cv::Scalar> _clsColorList;

		//Path
		CString _bsPath;
		CString _mPath;//모델 경로
		CString _imgBasePath; // 찾은 이미지 기본경로
		CString _imgExtraPath;//추가경로(폴더)
		CString _imgName;// 이미지 이름
		CString _resultPath;//결과 저장경로
		CString _tempPath;//임시경로
		CString _bPath, _dPath;
		CString _tempMsg_Cls, _tempMsg_Seg;
		CString _gpuName;

	protected:
		session::ptr _Session, _SessionCoilSeg, _SessionCoilCls;
		//cuFloat::ptr _inMatCoil;
		//cuFloat::ptr _inFloat;
		jsl::Blob _coilBlob;
	};
	class CRAFT_AI_AutoModule
	{
	public:
		CRAFT_AI_AutoModule()
		{
			_Core = std::shared_ptr<CRAFT_InferCore>(new CRAFT_InferCore());
			beLoaded = false;
			nBatch = 10;
		}
		~CRAFT_AI_AutoModule()
		{
			_Core.reset();
			beLoaded = false;
		}
	public:
		void ModuleInit(int nBatch, std::vector<cv::Mat> input_image_vector);
		bool LoadModel(CString model_path, int nGpu, std::function<int(int)> select_gpu_device, bool use_option);
		void AI_Infer();
		std::vector<std::vector<cv::Point2f>> CRAFT_AI_returnROI(int set_nBatch, std::vector<cv::Mat> input_image_vector);
		void ImageToBatchBuffer(int batch_start_index, int input_batch_size);
		void resize_aspect_ratio(std::vector<cv::Mat> input_image_vector, int canvas_size, int interpolation, float mag_ratio);
		void normalize_MeanVariance(std::vector<cv::Mat> input_image_vector);
		void getCenterPoint(std::vector<cv::Mat> score_map_image_vector, float text_threshold, float low_text);
		void adjustResultCoordinates(std::vector<std::vector<cv::Point2f>> image_rect_info, std::vector<float> image_resize_ratio);
	public:
		int nBatch;
		bool beLoaded;
		std::vector<float> batch_result;
	private:
		std::shared_ptr<CRAFT_InferCore> _Core;
		std::vector<cv::Mat> _image_vector; //들어온 이미지 버퍼.
		std::vector<cv::Mat> _batch_vector; //배치 버퍼
		std::vector<cv::Mat> _Inference_result; //추론 결과 score map 이미지 버퍼
		std::vector<float> _image_resize_ratio; //resize_aspect_ratio에 의해 변형된 이미지 배율
		std::vector<std::vector<cv::Point2f>> _image_center_points_vectors;
	};

	class PyOCR_AI_AutoModule
	{
	public:
		PyOCR_AI_AutoModule()
		{
			this->_Core = std::shared_ptr<PyOCR_InferCore>(new PyOCR_InferCore());
			this->beLoaded = false;
			this->nBatch = 10;
			this->_image_height = 224;
			this->_image_width = 224;
			this->_blur_filter_size = 61;
		}
		~PyOCR_AI_AutoModule()
		{
			this->_Core.reset();
			this->beLoaded = false;
		}
	public:
		void ModuleInit(int nBatch, std::vector<cv::Mat> input_images);
		bool LoadModel(CString model_path, int select_gpu_device, bool use_option);
		std::vector<cv::Mat> OCRInference(std::vector<cv::Mat> input_image_vector, int set_nBatch = 10);
		void ImageToBatchBuffer(int batch_start_index, int input_batch_size);
		cv::Mat AdjustPreProcess(cv::Mat input_image);
		void OCRPreProcess(std::vector<cv::Mat> input_image_vector);
		void _Infer();
		bool EmptyModel(int netID);
		CString ModelID();

	public:
		bool beLoaded;
		int nBatch;

	private:
		std::shared_ptr<PyOCR_InferCore> _Core;
		std::vector<cv::Mat> _image_vector;
		std::vector<cv::Mat> _original_image_vector;
		std::vector<cv::Mat> _batch_vector;
		std::vector<cv::Mat> _Inference_result;
		int _image_height;
		int _image_width;
		int _blur_filter_size;
	};
}