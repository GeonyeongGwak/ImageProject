#pragma once
#include "StdAfx.h"
#include "CNN_Inference.h"
#include <fstream>
#include <iomanip>
#include "MemoryManager.h"

//#define DEBUG_AI
//#define DEBUG_AI_DB
namespace csml
{
	void _GaussianLength(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch, std::vector<double>& rstD);

	bool Inference::LoadModel(CString path)
	{
		totalGpuCount = GPUCount();

		load_model_chk = false;
		if (totalGpuCount > 0) // GPU가 존재하는 경우
		{
			if (!(inference_mode == 2 && _SessionCls != nullptr && _SessionSeg == nullptr))
				ReleaseModel();

			// session 객체 생성 & path 경로의 .cs2 파일 로드
			if ((inference_mode == 0 || inference_mode == 2) && (_SessionCls == nullptr || _SessionCls->_Nets.size() == 0))
			{
				modelPath[0] = path;
				_SessionCls = csml::session::ptr(new csml::session(modelPath[0], _gpuId, _gpuCnt));

				if (_SessionCls->_Nets.size() == 0)
				{
					_SessionCls.reset();
					load_model_chk = false;
				}
				else
					load_model_chk = true;
			}
			else if (inference_mode > 0 && (_SessionSeg == nullptr || _SessionSeg->_Nets.size() == 0))
			{
				modelPath[1] = path;
				_SessionSeg = csml::session::ptr(new csml::session(modelPath[1], _gpuId, _gpuCnt));

				if (_SessionSeg->_Nets.size() == 0)
				{
					_SessionSeg.reset();
					load_model_chk = false;
				}
				else
					load_model_chk = true;
			}
		}

		if (load_model_chk)
		{
			//// Input Image 배율 = 2^(downSampling 횟수)
			int ClsReductionRate(1), SegReductionRate(1);
			if (_SessionCls != nullptr && _SessionCls->_Nets.size() > 0)
				ClsReductionRate = _SessionCls->DownSamplingRate();
			if (_SessionSeg != nullptr && _SessionSeg->_Nets.size() > 0)
				SegReductionRate = _SessionSeg->DownSamplingRate();
			downSamplingRate = ClsReductionRate > SegReductionRate ? ClsReductionRate : SegReductionRate;
			_block = 30;
		}
		else
		{
			padded_width = 0;
			padded_height = 0;
			gap_width = 0;
			gap_height = 0;
			downSamplingRate = 1;
		}

		return load_model_chk;
	}
	bool Inference::LoadModel(std::vector<CString> path)
	{
		totalGpuCount = GPUCount();

		load_model_chk = true;
		if (totalGpuCount > 0 && path.size() == 2) // GPU가 존재하는 경우
		{
			ReleaseModel();

			modelPath = path;
			_SessionCls = csml::session::ptr(new csml::session(modelPath[0], _gpuId, _gpuCnt));
			if (_SessionCls->_Nets.size() == 0)
			{
				_SessionCls.reset();
				load_model_chk = false;
			}

			_SessionSeg = csml::session::ptr(new csml::session(modelPath[1], _gpuId, _gpuCnt));
			if (_SessionSeg->_Nets.size() == 0)
			{
				_SessionSeg.reset();
				load_model_chk = false;
			}
		}

		if (load_model_chk)
		{
			//// Input Image 배율 = 2^(downSampling 횟수)
			int ClsReductionRate(1), SegReductionRate(1);
			if (_SessionCls != nullptr && _SessionCls->_Nets.size() > 0)
				ClsReductionRate = _SessionCls->DownSamplingRate();
			if (_SessionSeg != nullptr && _SessionSeg->_Nets.size() > 0)
				SegReductionRate = _SessionSeg->DownSamplingRate();
			downSamplingRate = ClsReductionRate > SegReductionRate ? ClsReductionRate : SegReductionRate;
		}
		else
		{
			padded_width = 0;
			padded_height = 0;
			gap_width = 0;
			gap_height = 0;
			downSamplingRate = 1;
		}

		return load_model_chk;
	}

	CString Inference::ModelID()
	{
		if (inference_mode == 0 && _SessionCls != nullptr)
			return _SessionCls->model_id();
		else if (inference_mode == 1 && _SessionSeg != nullptr)
			return _SessionSeg->model_id();
		else
			return _T("Fusion Model");
	}
	CString Inference::ModelID(int mode)
	{
		if (mode == 0 && _SessionCls != nullptr)
			return _SessionCls->model_id();
		else if (mode == 1 && _SessionSeg != nullptr)
			return _SessionSeg->model_id();
		else
			return _T("Wrong input of inference mode");
	}

	void Inference::ReleaseModel()
	{
		if (_SessionCls)
			_SessionCls.reset();
		if (_SessionSeg)
			_SessionSeg.reset();
	}
	void Inference::ReleaseModel(int netID)
	{
		if (netID == 0)
			_SessionCls.reset();
		else if (netID == 1)
			_SessionSeg.reset();
	}

	bool Inference::EmptyModel()
	{
		if (inference_mode == 0 && _SessionCls == nullptr)
			return true;
		else if (inference_mode == 1 && _SessionSeg == nullptr)
			return true;
		else if (inference_mode == 2 && _SessionCls == nullptr && _SessionSeg == nullptr)
			return true;

		return false;
	}
	bool Inference::EmptyModel(int netID)
	{
		if (netID == 0 && _SessionCls == nullptr)
			return true;
		else if (netID == 1 && _SessionSeg == nullptr)
			return true;

		return false;
	}

	int Inference::GPUCount()
	{
		return runtime::gpu_count();
	}

	void Inference::init()
	{
		downSamplingRate = 1;
		padded_width = 0;
		padded_height = 0;
		gap_width = 0;
		gap_height = 0;

		init_time = 0.0;
		preprocess_time = 0.0;
		inference_time = 0.0;
		postprocess_time = 0.0;
		res_save_time = 0.0;
		total_infer_time = 0.0;
	}
	cv::Mat Inference::preprocess(cv::Mat ROI, int mode)
	{
		if (mode == 0)
		{
			return ROI;
		}
		else if (mode == 1)
		{
			cv::Mat Normalized_ROI;
			float avg = cv::mean(ROI).val[0];
			Normalized_ROI = ROI / (avg + FLT_EPSILON);
			return Normalized_ROI;
		}
	}
	cv::Mat Inference::ZeroPaddingImg(cv::Mat FOV)
	{
		//// Input Image 배율 = 2^(downSampling 횟수)
		int ClsReductionRate(1), SegReductionRate(1);
		if (_SessionCls != nullptr && _SessionCls->_Nets.size() > 0)
			ClsReductionRate = _SessionCls->DownSamplingRate();
		if (_SessionSeg != nullptr && _SessionSeg->_Nets.size() > 0)
			SegReductionRate = _SessionSeg->DownSamplingRate();
		int downSamplingRate = ClsReductionRate > SegReductionRate ? ClsReductionRate : SegReductionRate;

		//// zero-padding 넣은 사이즈
		gap_width = downSamplingRate - (FOV.cols % downSamplingRate) % downSamplingRate;
		gap_height = downSamplingRate - (FOV.rows % downSamplingRate) % downSamplingRate;

		if (gap_width > 0 || gap_height > 0)
		{
			padded_width = FOV.cols + gap_width;
			padded_height = FOV.rows + gap_height;

			cv::Mat paddedInputImg(padded_height, padded_width, FOV.type(), cv::Scalar(0));
			FOV.copyTo(paddedInputImg(cv::Rect(gap_width / 2, gap_height / 2, FOV.cols, FOV.rows)));

			return paddedInputImg;
		}
		else
			return FOV;
	}
	cv::Mat Inference::LabelMaker(cv::Mat InFOV, CString SaveNum)
	{
		////// Check Model Load
		if (_SessionSeg == NULL)
		{
			if (modelPath[1] != "")
			{
				if (totalGpuCount > 0)
					_SessionSeg = csml::session::ptr(new csml::session(modelPath[1], 0));
			}
			else
			{
				//AfxMessageBox(_T("Model is NOT loaded"));
				return cv::Mat();
			}
		}

		if (_SessionSeg == NULL)
			return cv::Mat();

		////// Initialize
		csml::tick tk, tk1, tk2, tk3, tk4, total_tk;
		total_tk.start();
		tk.start();
		init();
		csml::cuByte::alloc(inImg, 1, 1, InFOV.rows, InFOV.cols);
		inImg->upload(InFOV.ptr());

		ZeroPaddingImg(InFOV);

		csml::cuFloat::ptr inMat = csml::cuFloat::alloc(1, 1, padded_height, padded_width);
		if (postprocessing_buffer == nullptr)
			postprocessing_buffer = csml::cuFloat::alloc(1, 1, InFOV.rows, InFOV.cols);
		init_time = tk.end();

		////// Input Matrix
		tk1.start();
		csml::Mat::fov2mat(inImg, InFOV.step, inMat, 0, 0, gap_width / 2, gap_height / 2);
		csml::runtime::thread_sync();
		preprocess_time = tk1.end();

		////// Inference
		tk2.start();
		csml::cuFloat::ptr result = _SessionSeg->Inference(inMat, half_mode);
		auto hres = result->download();
		inference_time = tk2.end();

		////// Post-process
		tk3.start();
		csml::Mat::mat2fov(result, result->size(3), result->size(2), result->size(3), gap_width / 2, gap_height / 2, inImg, InFOV.rows, InFOV.cols, postprocessing_buffer, binary_threshold); // Out Image Size 관련 Offset 수치 수정
		cv::Mat Label(InFOV.rows, InFOV.cols, CV_8UC1);
		inImg->download(Label.ptr<uchar>());
		postprocess_time = tk3.end();
		total_infer_time = total_tk.end();

		////// Save Debug Info
		if (save_chk)
		{
			tk4.start();
			if (SaveNum == _T("") || SaveNum == _T("-1"))
			{
				cv::imwrite((std::string)(CT2CA)BubbleNGImgPath + "\\Inferenced_FOV.bmp", InFOV);
				cv::imwrite((std::string)(CT2CA)BubbleNGImgPath + "\\Inferenced_Label.bmp", Label);
			}
			else
			{
				cv::imwrite((std::string)(CT2CA)BubbleNGImgPath + "\\FOV_" + (std::string)(CT2CA)SaveNum + "_UV.bmp", InFOV);
				cv::imwrite((std::string)(CT2CA)BubbleNGImgPath + "\\Label_" + (std::string)(CT2CA)SaveNum + "_UV.bmp", Label);
			}
			res_save_time = tk4.end();

			SaveReport(BubbleNGImgPath, SaveNum);
		}

		return Label;
	}
	void Inference::SaveReport(CString report_save_path, CString AddedSaveName)
	{
		// Test 관련 정보 - 파일로 저장
		std::ofstream o_f;
		CString folderName;
		if (AddedSaveName == _T(""))
			folderName = BubbleNGImgPath.Right(BubbleNGImgPath.GetLength() - BubbleNGImgPath.ReverseFind('\\') - 1);
		else
			folderName = AddedSaveName;
		o_f.open((std::string)(CT2CA)report_save_path + "\\TestReport(" + (std::string)(CT2CA)folderName + ").csv", std::ios::app);
		o_f << "Path :" << "," << (std::string)(CT2CA)report_save_path << std::endl;
		o_f << "Folder" << "," << (std::string)(CT2CA)folderName << "," << std::endl;
		o_f << "Model" << "," << "classification : " << (std::string)(CT2CA)modelPath[0] << "," << "segmentation : " << (std::string)(CT2CA)modelPath[1] << std::endl;
		o_f << "Src Img Size" << "," << "width" << "," << std::to_string((long double)_imgWid) << "," << "height" << "," << std::to_string((long double)_imgLen) << std::endl;
		o_f << "Input Img Size" << "," << "width" << "," << std::to_string((long double)padded_width) << "," << "height" << "," << std::to_string((long double)padded_height) << std::endl;

		o_f << "Init Time" << "," << std::to_string((long double)init_time) << std::endl;
		o_f << "Load Time" << "," << std::to_string((long double)preprocess_time) << std::endl;
		o_f << "Inference Time" << "," << std::to_string((long double)inference_time) << std::endl;
		o_f << "Post-process Time" << "," << std::to_string((long double)postprocess_time) << std::endl;
		o_f << "Result Save Time" << "," << std::to_string((long double)res_save_time) << std::endl;
		o_f << "Total Infer Time" << "," << std::to_string((long double)total_infer_time) << std::endl << std::endl;

		o_f.close();
	}

	cv::Mat InferOCR::OCRInputMatrix(cv::Mat srcImg)
	{
		// Histogram Equalization
		cv::Mat histImg;
		cv::equalizeHist(srcImg, histImg);

		// Resize (가로 세로 비율 유지 : ROI_w = ROI_h 가정)
		cv::Mat tempMat, ResizeMat;
		int offset = 0.5 * abs(histImg.cols - histImg.rows);
		if (histImg.cols > histImg.rows)
		{
			tempMat = cv::Mat(histImg.cols, histImg.cols, histImg.type(), cv::Scalar::all(0));
			histImg.copyTo(tempMat(cv::Rect(0, offset, histImg.cols, histImg.rows)));
		}
		else
		{
			tempMat = cv::Mat(histImg.rows, histImg.rows, histImg.type(), cv::Scalar::all(0));
			histImg.copyTo(tempMat(cv::Rect(offset, 0, histImg.cols, histImg.rows)));
		}

		// ROI 이미지 - 신경망 입력 크기로 변환
		resize(tempMat, ResizeMat, cv::Size(_imgWid, _imgLen), 0.0, 0.0, cv::INTER_NEAREST);

		// Blur
		cv::Mat dst;
		cv::GaussianBlur(ResizeMat, dst, cv::Size(blurFilterSz, blurFilterSz), 0.0);

		return dst;
	}
	cv::Mat InferOCR::OCRInputMatrix(std::vector<cv::Mat> srcBuffer, int stPos, int nCnt)
	{
		// Input Matrix 초기화
		cv::Mat _inMat;
		if (_inMat.empty() == true ||
			_inMat.size[0] != nCnt || _inMat.size[1] != _imgCh ||
			_inMat.size[2] != _imgLen || _inMat.size[3] != _imgWid)
		{
			std::vector<int> ImgDims(4);
			ImgDims[0] = nCnt;
			ImgDims[1] = _imgCh;
			ImgDims[2] = _imgLen;
			ImgDims[3] = _imgWid;
			_inMat = cv::Mat(4, &ImgDims[0], CV_32FC1, cv::Scalar(0));
		}

		// Input Matrix 생성
		for (int j = 0; j < nCnt; j++)
		{
			// 이미지 형변환
			cv::Mat srcImg = OCRInputMatrix(srcBuffer[stPos + j]);

			cv::Mat src(_imgWid, _imgLen, CV_32F);
			srcImg.convertTo(src, CV_32F);

			// 저장 위치 지정
			std::vector<cv::Range> dstRange(_inMat.dims);
			dstRange[0] = cv::Range(j, j + 1);
			for (int d = 1; d < dstRange.size(); d++)
				dstRange[d] = cv::Range::all();

			cv::Mat dst(_inMat, &dstRange[0]);

			// 복사
			memcpy(dst.ptr(), src.ptr(), src.total() * sizeof(float));
		}

		return _inMat;
	}
	UINT16 InferOCR::ClassNumToAscii(int i)
	{
		if (i < 10) // 숫자
			return UINT16(i + 48);
		else if (i < 37) // 대문자
			return UINT16(i + 55);
		else if (i < 39) // a, b
			return UINT16(i + 60);
		else if (i < 50) // d,e,f,g,h,i,j,k,l,m,n
			return UINT16(i + 61);
		else if (i < 53) //p,q,r
			return UINT16(i + 62);
		else if (i < 55) //t,u
			return UINT16(i + 63);
		else //x,y
			return UINT16(i + 65);
	}
	std::vector<cv::Mat> InferOCR::OCRInference(std::vector<cv::Mat> ImgBuffer)
	{
		////// Check Model Load
		if (_SessionCls == nullptr && modelPath[0] != "")
			if (totalGpuCount > 0)
				_SessionCls = csml::session::ptr(new csml::session(modelPath[0], 0));

		if (_SessionCls == nullptr)
			return std::vector<cv::Mat>();

		tick tk, tk1, tk2, tk3, total_tk;
		total_tk.start();

		////// 초기화
		tk.start();
		init();
		int stPos = 0, nCnt = 0;
		std::vector<cv::Mat> Res;
		array<float>::ptr _inData;
		csml::array<float>::alloc(_inData, _block, _imgCh, _imgLen, _imgWid);
		int inspCount = (int)ceil((double)ImgBuffer.size() / (double)_block);
		init_time = tk.end();

		for (int inspCnt = 0; inspCnt < inspCount; inspCnt++)
		{
			// Batch 세팅
			tk.start();
			stPos += nCnt;
			cuFloat::ptr _subInData;
			if (ImgBuffer.size() < stPos + _block)
			{
				nCnt = ImgBuffer.size() - stPos;
				_subInData = _inData->roi(0, nCnt);
			}
			else
			{
				nCnt = _block;
				_subInData = _inData;
			}
			init_time += tk.end();

			// Input Matrix 생성
			tk1.start();
			cv::Mat _inMat = OCRInputMatrix(ImgBuffer, stPos, nCnt);
			preprocess_time += tk1.end();

			// Inference 수행
			tk2.start();
			_subInData->upload(_inMat.ptr<float>());
			cuFloat::ptr res = _SessionCls->Inference(_subInData, half_mode);
			std::vector<float> vals(res->total());
			res->download(&vals[0]);
			inference_time += tk2.end();

			// 샘플단위 결과 저장
			tk3.start();
			int nCls = vals.size() / nCnt;
			for (int n = 0; n < nCnt; n++)
			{
				// Net 결과 저장
				std::vector<std::pair<int, float>> val;
				for (int cls = 0; cls < nCls; cls++)
					val.emplace_back(make_pair(cls, vals[n * nCls + cls]));

				// 정렬
				std::sort(val.begin(), val.end(), compare_pair_second<std::greater>());

				// 반환 결과 저장 (상위 N개 데이터)
				cv::Mat subRes(_nOCRRes, 2, CV_16U, cv::Scalar::all(0)); // unsigned short
				for (int i = 0; i < _nOCRRes; i++)
				{
					subRes.at<ushort>(i, 0) = ushort(ClassNumToAscii(val[i].first));
					subRes.at<ushort>(i, 1) = ushort(10000 * val[i].second);
				}
				Res.emplace_back(subRes);
			}
			res_save_time += tk3.end();
		}
		total_infer_time = total_tk.end();

		if (save_chk)
			SaveReport(Res, OCRNGImgPath, ImgBuffer);

		return Res;
	}
	void InferOCR::SaveReport(std::vector<cv::Mat> res, CString report_save_path, std::vector<cv::Mat> ImgBuffer)
	{
		// Test 관련 정보 - 파일로 저장
		std::ofstream o_f;
		CString folderName = OCRNGImgPath.Right(OCRNGImgPath.GetLength() - OCRNGImgPath.ReverseFind('\\') - 1);
		o_f.open((std::string)(CT2CA)report_save_path + "\\TestReport(" + (std::string)(CT2CA)folderName + ").csv", std::ios::app);

		CString TempBuf;
		TempBuf.Format(_T("[OCR Test Environment]\nResult Folder,%s\nModel Path,%s\nModel ID,%s\n\n"), OCRNGImgPath, modelPath[0], _SessionCls->model_id());
		o_f << (std::string)(CT2CA)TempBuf;
		TempBuf.Format(_T("[Inference Time]\nhalf Precision,%s\nBatch Size,%d\nInit Time,%lf\nImage Load Time,%lf\nInference Time,%lf\nResult Save Time,%lf\nTotal Infer Time,%lf\n\n"), (half_mode ? _T("True") : _T("False")), _block, init_time, preprocess_time, inference_time, res_save_time, total_infer_time);
		o_f << (std::string)(CT2CA)TempBuf;
		TempBuf.Format(_T("[Sample Info]\nInput Image Size,%d*%d\n# of Input Image,%d\nIndex,Class,Prob,Top-%d Probs of Each Class\n"), _imgWid, _imgLen, res.size(), _nOCRRes);
		o_f << (std::string)(CT2CA)TempBuf;

		////// 분류결과
		for (int n = 0; n < res.size(); n++)
		{
			char resCls = res[n].at<ushort>(0, 0);
			float maxProp = res[n].at<ushort>(0, 1) / 100.f;

			if (ImgBuffer.size() > 0)
				imwrite(std::string(CT2A(OCRNGImgPath)) + cv::format("\\Image_%d(%c_%.2f).bmp", n, resCls, maxProp), ImgBuffer[n]);

			std::string sampleRes = cv::format("%d,%c,%f", n, resCls, maxProp);
			for (int c = 0; c < _nOCRRes; c++)
				sampleRes += cv::format(",%c[%f]", res[n].at<ushort>(c, 0), res[n].at<ushort>(c, 1) / 100.f);
			o_f << sampleRes << "," << std::endl;
		}

		o_f.close();
	}

	void InferCoil::Init(int nDefects)
	{
		//_clsTh = 0.5f;
		//_segTh = 50;
		_blobLimitH = 0;
		_blobLimitW = 0;
		_blobLimitA = 0;
		_blobLimitSzD = 0;
		_nSerialNumber = 0;
		_rcCoilDefects.resize(nDefects);
		memset(&_rcCoilDefects[0], 0, nDefects * sizeof(RECT));

		CStdioFile readers;
		CString str = _T("");

		//_numSelectedGPU = 0;
		//_bUseHalf = false;

		_curSequence = -999;
		srand(time(NULL));
		if (_SessionCoilSeg)
		{
			Mem_freeAll();
		}
		if (_SessionCoilSeg == false)
		{
			bool loadRes = LoadModel(_T("D:\\AI_NET_COIL_F1.cs2"), 3);
			if (loadRes == false)
			{
				return;
			}
		}

		if (_SessionCoilCls == false)
		{
			bool loadRes = LoadModel(_T("D:\\AI_NET_COIL_F2.cs2"), 3);
			if (loadRes == false)
			{
				return;
			}
		}

		//if (readers.Open(_T("D:\\coilConfig.txt"), CFile::modeRead))
		//{
		//	readers.ReadString(str);
		//	readers.ReadString(str);
		//	if (str != "")
		//	{
		//		_bUseHalf = _ttoi(str);
		//	}
		//	readers.ReadString(str);
		//	if (str != "")
		//	{
		//		_numSelectedGPU = _ttoi(str);
		//	}
		//	readers.Close();
		//}

		_bInit = true;
		_imgW = 0;
		_imgH = 0;
		_mskW = 0;
		_mskH = 0;
		_mskX = 0;
		_mskY = 0;
	}

	int InferCoil::LoadModel(CString path, int nCh)
	{		//0 1 2
		//0과 1은 각 번호로
		//2는 rtx가 있는 번호로
		int gpu_id = _numSelectedGPU;

		size_t szLimit(0);
#ifdef _OFFLINE
		// 2GB
		szLimit = 2147483648;
#else
		// 6GB
		szLimit = 6442450944;
#endif // _OFFLINE			
		if (gpu_id == 2)
		{
			size_t memsz0, memsz1;
			runtime::set_gpu(0);
			CString gpu_name0 = runtime::gpu_name(0, &memsz0);
			runtime::set_gpu(1);
			CString gpu_name1 = runtime::gpu_name(1, &memsz1);
			bool b0Rtx = gpu_name0.MakeLower().Find(_T("rtx")) != -1 ? true : false;
			bool b1Rtx = gpu_name1.MakeLower().Find(_T("rtx")) != -1 ? true : false;

			_numSelectedGPU = (memsz0 > szLimit && memsz1 > szLimit) ? (b0Rtx == true && b1Rtx == false) ? 0 : (b0Rtx == false && b1Rtx == true) ? 1 : (memsz0 > memsz1) ? 0 : 1 : memsz0 > szLimit ? 0 : memsz1 > szLimit ? 1 : _numSelectedGPU;
			_bUseHalf = _numSelectedGPU == 0 ? b0Rtx : _numSelectedGPU == 1 ? b1Rtx : false;
			_gpuName = _numSelectedGPU == 0 ? gpu_name0 : _numSelectedGPU == 1 ? gpu_name1 : _T("");
			if (memsz0 < szLimit&&memsz1 < szLimit)
			{
				if (_bMemoryWarned == false)
				{
					AfxMessageBox(_T("Gpu Setting Not Appropriate for AI mode(Memory insufficient. Require:6GB). Check Again!!"), MB_TOPMOST);
					_bMemoryWarned = true;
				}
				return false;
			}
		}

		if (gpu_id == -1)
			return false;
		else
		{
			if (gpu_id < 2)
			{
				runtime::set_gpu(gpu_id);
				CString gpu_name = runtime::gpu_name(gpu_id);
				size_t memsz;
				CString gpuName = runtime::gpu_name(gpu_id, &memsz);
				if (memsz < szLimit)
				{
					if (_bMemoryWarned == false)
					{
						AfxMessageBox(_T("Gpu Setting Not Appropriate for AI mode(Memory insufficient. Require:6GB). Check Again!!"), MB_TOPMOST);
						_bMemoryWarned = true;
					}
					return false;
				}
				else
				{
					_gpuName = gpuName;
					_bUseHalf = _gpuName.MakeLower().Find(_T("rtx")) != -1 ? true : false;
				}
			}

			{
				runtime::set_gpu(_numSelectedGPU);
				_Session = session::ptr(new session(path, _numSelectedGPU));
				if (_Session->_Nets.size() == 0)
					return false;
				else
				{
					csml::layer::nets netAll = _Session->_Nets[0]->net();
					for (int iterN = 0; iterN < netAll.size(); iterN++)
					{
						CString netName = netAll[iterN]->name();
						if (netName.MakeLower().Find(_T("convtranspose")) != -1)
						{
							_SessionCoilSeg = session::ptr(new session(path, _numSelectedGPU));
							_Session.reset();
							return true;
						}
						else if (netName.MakeLower().Find(_T("affine")) != -1 || netName.MakeLower().Find(_T("softmax")) != -1)
						{
							_SessionCoilCls = session::ptr(new session(path, _numSelectedGPU));
							csml::params::ptr ptrParam = _SessionCoilCls->_Nets[0]->param();
							_nModelCh = ptrParam->_params[_T("conv1_f")].cuVal->size(1);
							_Session.reset();
							_clsModelImg = _SessionCoilCls->_CoilResultView.clone();
							if (_clsModelImg.empty() == true)
							{
								_clsModelImg = cv::Mat::ones(606, 300, CV_8UC1);
							}
							//_clsModelImg = cv::imread("D:\\work\\pemtoFrameworkAll\\MPTILib\\CoilModelImg.bmp", CV_8UC1);
							//_clsModelImg = _SessionCoilCls->
							_modelH = _clsModelImg.rows;
							_modelW = _clsModelImg.cols;
							_modelRealW = _clsModelImg.cols;
							int nn = _modelRealW % 4;
							_modelRealW += nn == 0 ? 0 : (4 - nn);
							_2ndFiltModelData.resize(3);
							_2ndFiltModelData[0].resize(_modelRealW*_modelH);
							_2ndFiltModelData[1].resize(_modelRealW*_modelH);
							_2ndFiltModelData[2].resize(_modelRealW*_modelH);
							RefreshFiltImg();
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	void InferCoil::Mem_freeAll()
	{
		if (_Session)
		{
			_Session.reset();
		}
		if (_SessionCoilSeg)
		{
			_SessionCoilSeg.reset();
		}
		if (_SessionCoilCls)
		{
			_SessionCoilCls.reset();
		}
		//if (_inMatCoil)
		//{
		//	_inMatCoil.reset();
		//}
	}
	void InferCoil::Inference()
	{

	}
	void InferCoil::Save_floatImg(cv::Mat src, cv::Mat dstImg)
	{
		cv::Mat floatMat;
		if (src.channels() == 3)
		{
			src.convertTo(floatMat, CV_32F);
			std::vector<cv::Range> srcRange(dstImg.dims);

			std::vector<cv::Mat> chList;
			cv::split(floatMat, chList);

			for (int c = 0; c < chList.size(); c++)
			{
				for (int d = 0; d < srcRange.size(); d++)
					srcRange[d] = cv::Range::all();
				srcRange[1] = cv::Range(c, c + 1);
				cv::Mat Line(dstImg, &srcRange[0]);
				preprocess(chList[c], 0);
				memcpy(Line.ptr(), chList[c].ptr(), chList[c].total() * sizeof(float));
			}
		}
		else
		{
			_tprintf(_T("channels error.\n"));
		}
	}
	float InferCoil::GetSigmoidVal(float in)
	{
		if (in > 7)
			return 1;
		else if (in < -7)
			return 0;
		else
			return 1 / (1 + exp(-in));
	}
	bool InferCoil::CoilResult(uchar* srcBR, uchar* srcTB, uchar* srcTW, uchar* mask, int srcW, int srcH, cv::Rect maskInfo)
	{
		_tBit = _tAdjust = _tBounding = _tSeg = _tCls = _tSegCopy = _tSegPadding = _tClsCopy = _tClsPadding = _tBoundaryCheck = _tSegMat = _tClsImgPre = 0.f;
		float ttime_segPrepare(0.f), t_upload(0.f);
		tick tk2, tk_in;
		tk2.start();
		tk_in.start();
		_curSequence = CoilRand();
		_imgW = srcW;
		_imgH = srcH;

		_offX = 0;
		_offY = 0;
		_mskX = 0;
		_mskY = 0;

		SYSTEMTIME cur_time;
		CString cPrefix;	//y	//m	//d  //h // m // s // ms

#ifdef DEBUG_AI
		GetLocalTime(&cur_time);
		cPrefix = _T("D:\\tem");
		_bsPath.Format(_T("%s\\Coil_%04d%02d%02d%02d%02d%02d%03d"), cPrefix, cur_time.wYear, cur_time.wMonth, cur_time.wDay, cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);
#endif

#ifdef DEBUG_AI_DB
		GetLocalTime(&cur_time);
		cPrefix = _T("D:\\tem");
		_bsPath.Format(_T("%s\\Coil_%04d%02d%02d%02d%02d%02d%03d"), cPrefix, cur_time.wYear, cur_time.wMonth, cur_time.wDay, cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);
#endif
		if (maskInfo.x < 0)	_offX = -maskInfo.x;
		if (maskInfo.y < 0)	_offY = -maskInfo.y;
		_mskX = maskInfo.x;
		_mskY = maskInfo.y;
		_mskW = _offX + _mskX + maskInfo.width > srcW ? srcW - (_mskX + _offX) : maskInfo.width - _offX;
		_mskH = _offY + _mskY + maskInfo.height > srcH ? srcH - (_mskY + _offY) : maskInfo.height - _offY;
		_curImg = cv::Mat(maskInfo.height, maskInfo.width, CV_8UC1, mask, maskInfo.width);

		_tBounding = tk_in.end();
		tk_in.start();
		if (_3chImg.size() > 0)
			_3chImg.clear();

		bool bOnce = true;
		//bOnce : 파트크기만큼 한번에 진행할지, 256*256크기로 크롭하여 진행할지 여부

		_3chImg.resize(3);
		cv::Mat sBR = cv::Mat(srcH, srcW, CV_8UC1, srcBR, srcW);
		cv::Mat sTB = cv::Mat(srcH, srcW, CV_8UC1, srcTB, srcW);
		cv::Mat sTW = cv::Mat(srcH, srcW, CV_8UC1, srcTW, srcW);
		_3chImg[0] = cv::Mat::zeros(_curImg.size(), CV_8UC1);
		_3chImg[1] = cv::Mat::zeros(_curImg.size(), CV_8UC1);
		_3chImg[2] = cv::Mat::zeros(_curImg.size(), CV_8UC1);

		cv::Rect srcRect = cv::Rect(_mskX + _offX, _mskY + _offY, _mskW - _offX, _mskH - _offY);
		cv::Rect dstRect = cv::Rect(_offX, _offY, _mskW - _offX, _mskH - _offY);

#ifdef DEBUG_AI
		imwrite(std::string(CT2A(_bsPath + _T("_MaskInfo.bmp"))), _curImg);
		imwrite(std::string(CT2A(_bsPath + _T("_BR.bmp"))), sBR);
		imwrite(std::string(CT2A(_bsPath + _T("_TB.bmp"))), sTB);
		imwrite(std::string(CT2A(_bsPath + _T("_TW.bmp"))), sTW);

		imwrite(std::string(CT2A(_bsPath + _T("_ROI_MaskInfo.bmp"))), _curImg(dstRect));
		imwrite(std::string(CT2A(_bsPath + _T("_ROI_BR.bmp"))), sBR(srcRect));
#endif

		cv::bitwise_and(sBR(srcRect), _curImg(dstRect), _3chImg[0](dstRect));
		cv::bitwise_and(sTB(srcRect), _curImg(dstRect), _3chImg[1](dstRect));
		cv::bitwise_and(sTW(srcRect), _curImg(dstRect), _3chImg[2](dstRect));
		_tBit = tk_in.end();

		tk_in.start();
		//BR이미지의 평균, 표준편차를 심사하여 이미지 보정 진행
		cv::Mat tempMat;
		_3chImg[0].convertTo(tempMat, CV_32FC1);
		cv::Scalar m_1, std_1;
		cv::meanStdDev(tempMat, m_1, std_1);
		bool bReBright = false;

		if (m_1[0] < 60 || std_1[0] < 60)
		{
			bReBright = true;
			float adjust_std = 72.f / std_1[0];
			float adjust_mean = 74.f - adjust_std * m_1[0];

			tempMat *= adjust_std;
			tempMat += adjust_mean;

			tempMat.convertTo(_3chImg[0], CV_8UC1);
			tempMat = _3chImg[0].clone();
#ifdef DEBUG_AI
			imwrite(std::string(CT2A(_bsPath + _T("_in1__.bmp"))), _3chImg[0]);
#endif
			_3chImg[0].setTo(0);
			cv::bitwise_and(tempMat(dstRect), _curImg(dstRect), _3chImg[0](dstRect));
#ifdef DEBUG_AI
			imwrite(std::string(CT2A(_bsPath + _T("_in1__light.bmp"))), _3chImg[0]);
#endif
		}
		_tAdjust = tk_in.end();
		if (CoilDeviceImg(_3chImg, bOnce))
		{
			return false;
		}

		_coilSegInput = _inDataMat.clone();
		csml::runtime::set_gpu(_numSelectedGPU);

		tk_in.start();
		cuFloat::ptr inFloat = cuFloat::alloc(_inDataMat.size[0], _inDataMat.size[1], _inDataMat.size[2], _inDataMat.size[3]);
		_tSeg = tk_in.end();

		//tk0.start();
		//cuFloat::alloc(_inMatCoil, _inDataMat.size[0], _inDataMat.size[1], _fovH, _fovW);
		//ft_1 = tk0.end();

		ttime_segPrepare = tk2.end();

#ifdef DEBUG_AI_DB
		imwrite(std::string(CT2A(_bsPath + _T("_BR_UV.bmp"))), _3chImg[0]);
		imwrite(std::string(CT2A(_bsPath + _T("_TB_UV.bmp"))), _3chImg[1]);
		imwrite(std::string(CT2A(_bsPath + _T("_TW_UV.bmp"))), _3chImg[2]);
#endif

		tk2.start();
		inFloat->upload(_inDataMat.ptr<float>());
		t_upload = tk2.end();

		float ttime(0.f), ttimePost(0.f);
		tk_in.start();
		//csml::cuFloat::ptr res = _SessionCoilSeg->Inference(_inMatCoil, _bUseHalf);
		csml::cuFloat::ptr res = _SessionCoilSeg->Inference(inFloat, _bUseHalf);
		ttime = tk_in.end();

		tk_in.start();
		CoilPostProcess(res);
		ttimePost = tk_in.end();

		CString totmsg0;
		totmsg0.Format(_T(",%d,%d,%.3f,%.3f,%.3f,%d,%.3f"), _inDataMat.size[2], _inDataMat.size[3], ttime, ttime_segPrepare, t_upload, (int)bReBright, ttimePost);
		_tempMsg_Seg = totmsg0;
		return _bCoilGood;
	}
	void InferCoil::CoilPostProcess(csml::cuFloat::ptr res)
	{
		std::vector<int> dims(4);
		dims[0] = res->size(0);
		dims[1] = res->size(1);
		dims[2] = res->size(2);
		dims[3] = res->size(3);
		cv::Mat resultImg = cv::Mat(4, &dims[0], CV_32FC1);
		res->download(resultImg.ptr<float>());

		cv::Mat fullImg = cv::Mat::zeros(_3chImg[0].size(), CV_8UC1);
		cv::Mat partImg, dataTemp;

		std::vector<cv::Range> srcRange1(4);
		double dMin, dMax;
		srcRange1[0] = cv::Range(0, 1);
		srcRange1[1] = cv::Range(0, 1);
		srcRange1[2] = cv::Range::all();
		srcRange1[3] = cv::Range::all();
		partImg = cv::Mat(resultImg, &srcRange1[0]);
		dataTemp = cv::Mat(resultImg.size[2], resultImg.size[3], CV_32FC1, resultImg.ptr<float>());
		cv::minMaxLoc(dataTemp, &dMin, &dMax);
		dataTemp -= dMin;
		dataTemp /= (dMax - dMin);
		dataTemp *= 255.0;
		dataTemp.convertTo(partImg, CV_8UC1);
		partImg(cv::Rect(gap_width / 2, gap_height / 2, fullImg.cols, fullImg.rows)).copyTo(fullImg);

		_tempResult = fullImg.clone();
		GetRangeCutImg(_segTh, _segTh, rangeUpper);
	}
	bool InferCoil::CoilDeviceImg(std::vector<cv::Mat> vMat, bool bOnce)
	{
		std::vector<cv::Mat> curImg;
		int imgCh = 3;
		curImg.resize(imgCh);
		CString sPath;
		tick tkk;
		tkk.start();
		_bPaddedSeg = false;
		_bPadded = false;
		curImg = ZeroPaddingImg(vMat);
		_bPaddedSeg = _bPadded;
		_tSegPadding = tkk.end();
		_tSegMat = 0.f;
		tkk.start();
		if (curImg[0].rows == 0 || curImg[0].cols == 0)
		{
			return true;
		}
		std::vector<int> szImg;
		szImg.resize(4);
		if (bOnce)
		{
			szImg[0] = 1;
			szImg[1] = imgCh;
			szImg[2] = curImg[0].rows;
			szImg[3] = curImg[0].cols;
		}
		else
		{
			int cropSize = 256;
			szImg[0] = (curImg[0].rows / cropSize + (curImg[0].rows%cropSize == 0 ? 0 : 1))*(curImg[0].cols / cropSize + (curImg[0].cols%cropSize == 0 ? 0 : 1));
			szImg[1] = imgCh;
			szImg[2] = cropSize;
			szImg[3] = cropSize;
		}
		_inDataMat = cv::Mat(4, &szImg[0], CV_32FC1, cv::Scalar(0));
		cv::Mat fMat;

		_tSegMat = tkk.end();
		if (bOnce)
		{
			_tSegCopy = 0.f;

			tkk.start();

			for (int iters = 0; iters < imgCh; iters++)
			{
				std::vector<cv::Range> srcRange(_inDataMat.dims);
				for (int d = 0; d < srcRange.size(); d++)
					srcRange[d] = cv::Range::all();
				srcRange[0] = cv::Range(0, 1);
				srcRange[1] = cv::Range(iters, iters + 1);
				cv::Mat LinetoCopy(_inDataMat, &srcRange[0]);
				curImg[iters].convertTo(fMat, CV_32F);
				memcpy(LinetoCopy.ptr(), fMat.ptr(), fMat.total() * sizeof(float));
			}
			_tSegCopy = tkk.end();
		}
		else
		{
			int tileX = curImg[0].cols / 256 + (curImg[0].cols % 256 == 0 ? 0 : 1);
			int tileY = curImg[0].rows / 256 + (curImg[0].rows % 256 == 0 ? 0 : 1);
			for (int iters = 0; iters < imgCh; iters++)
			{
				for (int iterH = 0; iterH < tileY; iterH++)
				{
					for (int iterW = 0; iterW < tileX; iterW++)
					{
						fMat = cv::Mat::zeros(256, 256, CV_8UC1);
						std::vector<cv::Range> srcRange(_inDataMat.dims);
						for (int d = 0; d < srcRange.size(); d++)
							srcRange[d] = cv::Range::all();
						int init_pt = tileX * iterH + iterW;
						srcRange[0] = cv::Range(init_pt, init_pt + 1);
						srcRange[1] = cv::Range(iters, iters + 1);
						cv::Mat LinetoCopy(_inDataMat, &srcRange[0]);
						cv::Rect rct(iterW * 256, iterH * 256, 256, 256);
						if (iterH * 256 + 256 > curImg[iters].rows)
						{
							rct.height = curImg[iters].rows - iterH * 256;
						}
						if (iterW * 256 + 256 > curImg[iters].cols)
						{
							rct.width = curImg[iters].cols - iterW * 256;
						}
						cv::Rect copyRect(0, 0, rct.width, rct.height);
						curImg[iters](rct).copyTo(fMat(copyRect));
						fMat.convertTo(fMat, CV_32FC1);
						memcpy(LinetoCopy.ptr(), fMat.ptr(), fMat.total() * sizeof(float));
					}
				}
			}
		}
		return false;
	}
	cv::Mat InferCoil::ZeroPaddingImg(cv::Mat FOV)
	{
		//// Input Image 배율 = 2^(downSampling 횟수)
		int ClsReductionRate(1), SegReductionRate(1);
		if (_SessionCoilSeg != nullptr && _SessionCoilSeg->_Nets.size() > 0)
			ClsReductionRate = _SessionCoilSeg->DownSamplingRate();
		int downSamplingRate = ClsReductionRate > SegReductionRate ? ClsReductionRate : SegReductionRate;

		//// zero-padding 넣은 사이즈
		gap_width = (downSamplingRate - (FOV.cols % downSamplingRate)) % downSamplingRate;
		gap_height = (downSamplingRate - (FOV.rows % downSamplingRate)) % downSamplingRate;

		if (gap_width > 0 || gap_height > 0)
		{
			padded_width = FOV.cols + gap_width;
			padded_height = FOV.rows + gap_height;

			cv::Mat paddedInputImg(padded_height, padded_width, CV_32FC1, cv::Scalar(0));
			int startx = gap_width / 2;
			int starty = gap_height / 2;
			if ((startx + FOV.cols) >= paddedInputImg.cols)
			{
				if ((starty + FOV.rows) >= paddedInputImg.rows)
				{
					startx--;
					starty--;
				}
				else
				{
					startx--;
				}
			}

			FOV.copyTo(paddedInputImg(cv::Rect(startx, starty, FOV.cols, FOV.rows)));

			return paddedInputImg;
		}
		else
			return FOV;
	}
	std::vector<cv::Mat> InferCoil::ZeroPaddingImg(std::vector<cv::Mat> FOV)
	{
		//같은 사이즈의 벡터 이미지일 경우 하나의 이미지에 한 번 계산 후 전체 이미지에 적용
		int ClsReductionRate(1), SegReductionRate(1);
		if (_SessionCoilSeg != nullptr && _SessionCoilSeg->_Nets.size() > 0)
			ClsReductionRate = _SessionCoilSeg->DownSamplingRate();
		int downSamplingRate = ClsReductionRate > SegReductionRate ? ClsReductionRate : SegReductionRate;

		//// zero-padding 넣은 사이즈
		gap_width = (downSamplingRate - (FOV[0].cols % downSamplingRate)) % downSamplingRate;
		gap_height = (downSamplingRate - (FOV[0].rows % downSamplingRate)) % downSamplingRate;

		if (gap_width > 0 || gap_height > 0)
		{
			_bPadded = true;
			padded_width = FOV[0].cols + gap_width;
			padded_height = FOV[0].rows + gap_height;

			int startx = gap_width / 2;
			int starty = gap_height / 2;
			if ((startx + FOV[0].cols) > padded_width)
			{
				if ((starty + FOV[0].rows) > padded_height)
				{
					startx--;
					starty--;
				}
				else
				{
					startx--;
				}
			}
			else if ((starty + FOV[0].rows) > padded_height)
			{
				starty--;
			}
			std::vector<cv::Mat> paddedInputImg;
			for (int iters = 0; iters < FOV.size(); iters++)
			{
				paddedInputImg.emplace_back(cv::Mat(padded_height, padded_width, CV_32FC1, cv::Scalar(0)));
				FOV[iters].copyTo(paddedInputImg[iters](cv::Rect(startx, starty, FOV[iters].cols, FOV[iters].rows)));
			}
			return paddedInputImg;
		}
		else
			return FOV;
	}
	int InferCoil::CoilRand()
	{
		int ran_val = abs(rand());
		return ran_val;
	}
	void InferCoil::GetRangeCutImg(int L, int R, int type)
	{
		_segTh = R;
		if (_tempResult.empty() == true)
		{
			_thresholdedImg.release();
			_rcCoilDefects.clear();
			return;
		}
		if (_tempResult.cols <= 0 || _tempResult.rows <= 0)
			return;
		cv::Mat src = _tempResult.clone();
		cv::Mat dst = cv::Mat(src.size(), src.type(), cv::Scalar(0));

		cv::Mat mid1 = cv::Mat(src.size(), src.type(), cv::Scalar(0));
		cv::Mat mid2 = cv::Mat(src.size(), src.type(), cv::Scalar(0));

		switch (type)
		{
		case rangeIn:
			cv::threshold(src, mid1, L, 255, cv::THRESH_BINARY_INV);
			cv::threshold(src, mid2, R, 255, cv::THRESH_BINARY);
			cv::bitwise_and(mid1, mid2, dst);
			break;
		case rangeOut:
			cv::threshold(src, mid1, R, 255, cv::THRESH_BINARY);
			cv::threshold(src, mid2, L, 255, cv::THRESH_BINARY_INV);
			cv::bitwise_or(mid1, mid2, dst);
			break;
		case rangeUpper:
			cv::threshold(src, dst, R, 255, cv::THRESH_BINARY);
			break;

		case rangeLower:
			cv::threshold(src, dst, L, 255, cv::THRESH_BINARY_INV);
			break;
		}


#ifdef DEBUG_AI
		imwrite(std::string(CT2A(_bsPath + _T("_in1.bmp"))), _3chImg[0]);
		imwrite(std::string(CT2A(_bsPath + _T("_in2.bmp"))), _3chImg[1]);
		imwrite(std::string(CT2A(_bsPath + _T("_in3.bmp"))), _3chImg[2]);
		imwrite(std::string(CT2A(_bsPath + _T("_src.bmp"))), src);
		imwrite(std::string(CT2A(_bsPath + _T("_dst.bmp"))), dst);
		////cv::dilate(dst, dst, cv::Mat(), cv::Point(-1, -1), 5);
		////imwrite(std::string(CT2A(_bsPath + _T("_dst2.bmp"))), dst);
#endif
		cv::Rect dstRect = cv::Rect(_offX, _offY, _mskW - _offX, _mskH - _offY);
		_thresholdedImg = cv::Mat(dst.size(), dst.type(), cv::Scalar::all(0));
		cv::bitwise_and(dst(dstRect), _curImg(dstRect), _thresholdedImg(dstRect));
		dst.setTo(0);
		dst = _thresholdedImg.clone();
		//_thresholdedImg(dstRect) = dst(dstRect).clone();
		//cv::Mat tmpMat = dst(dstRect).clone();
#ifdef DEBUG_AI
		imwrite(std::string(CT2A(_bsPath + _T("_dstPost.bmp"))), _thresholdedImg);
#endif
		//imwrite(std::string(CT2A(_bsPath + _T("_dstPost_roi.bmp"))), tmpMat);
		//_thresholdedImg = dst.clone();
		//small size blob 제거
		int nDefects = RemoveBlobs(dst);
		_bCoilGood = nDefects == 0 ? true : false;
		RefreshCoilResultImg(dst);
	}
	void InferCoil::ClearImgs()
	{
		if (_thresholdedImg.empty() == false)
			_thresholdedImg.release();
		if (_tempResult.empty() == false)
			_tempResult.release();
		_imgW = 0;
		_imgH = 0;
		_mskW = 0;
		_mskH = 0;
	}
	void InferCoil::SavePartResultImg(CString cstFilePath, CString cstFileName, bool bSaveOK, bool bSaveNG)
	{
		if (_3chImg.size() < 3)
			return;
		CString logPath;
		logPath.Format(_T("D:\\Eagle3D_data\\log\\zLog_Coil_") + (cstFilePath.Right(14)).Left(8) + _T(".csv"));
		bool bNeedReset = false;
		if (PathFileExists(logPath) && !_3chImg[0].empty() && !_3chImg[1].empty() && !_3chImg[2].empty() && !_tempResult.empty() && !_thresholdedImg.empty() && !_tempResultBlob.empty())
		{
			CStdioFile opener;
			if (opener.Open(logPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate))
			{
				opener.SeekToEnd();
				opener.WriteString(cstFileName + _T("\n"));
				opener.Close();
			}
			bNeedReset = true;
		}

		if (bSaveOK == false && bSaveNG == false)
		{
			if (bNeedReset)
			{
				_tempResult.release();
				_tempResultBlob.release();
				_tempResultBlob_RECTS.release();
			}
			return;
		}

		_bPath = cstFilePath.Left(cstFilePath.GetLength() - 14);
		CString dp_temp = (cstFilePath.Right(14)).Left(8) + _T("_Coil");

		if (_bInit == false || cstFilePath == _T("") || cstFileName == _T("") || _3chImg.size() < 3)
			return;
		else
		{
			if (PathFileExists(_bPath + dp_temp + _T("\\CoilData\\") + cstFilePath.Right(14)) == false)
			{
				_tmkdir(_bPath);
				_tmkdir(_bPath + dp_temp);
				_tmkdir(_bPath + dp_temp + _T("\\CoilData"));
				_tmkdir(_bPath + dp_temp + _T("\\CoilData\\") + cstFilePath.Right(14));
			}
		}
		_dPath = dp_temp + _T("\\CoilData\\") + cstFilePath.Right(14);

		if (!_3chImg[0].empty() && !_3chImg[1].empty() && !_3chImg[2].empty() && !_tempResult.empty() && !_thresholdedImg.empty() && !_tempResultBlob.empty())
		{
			CString lbPath;
			CString fname;
			//NG일 경우 저장
			if (cv::mean(_tempResultBlob)[0] != 0)
			{
				if (bSaveNG == true)
				{
					fname = _bPath + _dPath + _T("\\DefectCoil");
				}
			}
			//OK일 경우 저장 안함 - 추후 변경 가능
			else
			{
				if (bSaveOK == true)
				{
					fname = _bPath + _dPath + _T("\\GoodCoil");
				}
				//return;	//OK일 경우 저장 안함
			}
			if ((GetFileAttributes(fname) != INVALID_FILE_ATTRIBUTES) == false)
				_tmkdir(fname);
			lbPath = fname;

			//입력 이미지 저장
			fname = fname + _T("\\") + cstFileName + _T("_BR.bmp");
			imwrite(std::string(CT2A(fname)), _3chImg[0]);
			fname.Replace(_T("_BR.bmp"), _T("_TB.bmp"));
			imwrite(std::string(CT2A(fname)), _3chImg[1]);
			fname.Replace(_T("_TB.bmp"), _T("_TW.bmp"));
			imwrite(std::string(CT2A(fname)), _3chImg[2]);

#if _OFFLINE || _DEBUG
			fname.Replace(_T("_TW.bmp"), _T("_zResult.bmp"));
			imwrite(std::string(CT2A(fname)), _tempResult);
#endif
			//결과 라벨 저장
			lbPath = lbPath + _T("\\Label_") + cstFileName + _T(".bmp");
			cv::imwrite(std::string(CT2A(lbPath)), _tempResultBlob);


			lbPath.Replace(_T(".bmp"), _T("_zz.bmp"));
			if (_tempResultBlob_RECTS.empty())
			{
				_tempResultBlob_RECTS = _tempResultBlob.clone();
			}
			cv::imwrite(std::string(CT2A(lbPath)), _tempResultBlob_RECTS);
			lbPath.Replace(_T("_zz.bmp"), _T(".bmp"));

		}

		_tempResult.release();
		_tempResultBlob.release();
		_tempResultBlob_RECTS.release();

	}
	void InferCoil::MoveUserOKPart(CString cstFilePath, CString cstFileName)
	{
		//cstFilePath와 cstFileName에서 받는것은 
		//_coil이 빠진 D:\\Eagle3D_data\\AI_Sample\\20201008251342

		//if(_bInit == false || cstFilePath == _T("") || cstFileName == _T(""))
		if (cstFilePath == _T("") || cstFileName == _T("") || cstFilePath.GetLength() < 14)
			return;

		CString cstBasePath = cstFilePath.Left(cstFilePath.GetLength() - 6);	//D:\\Eagle3D_data\\AI_Sample\\20201008
		CString cstDateTime = cstFilePath.Right(14);	//20201008251342
		CString cstDate = cstDateTime.Left(8);	//20201008



		//defectviewer에서의 이동은 되지만
		//Repairstation에서는 _bPath, _dPath가 설정이 안되어있어서 동작 안하므로 
		//FilePath와 FileName을 이용하여 변경
		{
			CString fname = cstBasePath + _T("_Coil\\CoilData\\") + cstDateTime + _T("\\UserOK"); //D:\\Eagle3D_data\\AI_Sample\\20201008\\CoilData\\20201008251342\\UserOK
			if ((GetFileAttributes(fname) != INVALID_FILE_ATTRIBUTES) == false)
				_tmkdir(fname);
			CString src, dst;

			// BR 이미지 이동
			fname.Replace(_T("UserOK"), _T("DefectCoil\\"));
			src = fname + cstFileName + _T("_BR.bmp");
			dst = src;
			dst.Replace(_T("DefectCoil"), _T("UserOK"));
			MoveFile(src, dst);

			// TB 이미지 이동
			src.Replace(_T("_BR.bmp"), _T("_TB.bmp"));
			dst.Replace(_T("_BR.bmp"), _T("_TB.bmp"));
			MoveFile(src, dst);

			// TW 이미지 이동
			src.Replace(_T("_TB.bmp"), _T("_TW.bmp"));
			dst.Replace(_T("_TB.bmp"), _T("_TW.bmp"));
			MoveFile(src, dst);

#if _OFFLINE || _DEBUG
			// Coil Result Gray 이미지 이동
			src.Replace(_T("_TW.bmp"), _T("_zResult.bmp"));
			dst.Replace(_T("_TW.bmp"), _T("_zResult.bmp"));
			MoveFile(src, dst);
#endif

			// 라벨 이동
			src = fname + _T("Label_") + cstFileName + _T(".bmp");
			dst = src;
			dst.Replace(_T("DefectCoil"), _T("UserOK"));
			MoveFile(src, dst);

			src.Replace(_T(".bmp"), _T("_zz.bmp"));
			dst.Replace(_T(".bmp"), _T("_zz.bmp"));
			MoveFile(src, dst);
		}
	}
	void InferCoil::FlushDir(CString inspTime)
	{
		if (inspTime.GetLength() == 0 || _bPath == _T("") || _dPath == _T(""))
			return;

		RemoveFolder(_bPath + _dPath);
		RemoveDirectory(_bPath + _dPath);

		_bPath = _T("");
		_dPath = _T("");
	}
	bool InferCoil::CoilClsResult()
	{
		_bCoilGood = true;
		_tempMsg_Cls = _T("");
		int dSz = _rcCoilDefects.size();
		if (dSz > 0)
		{
			tick tk_;
			float t_imgprep(0.f);
			tk_.start();

			bool prep_img = Coil2ndClsImg();
			t_imgprep = tk_.end();
			if (prep_img == true)
			{
				float tAllocUpload(0.f);
				tk_.start();
				cuFloat::ptr inFloat;

				tick tk_0;
				float ftNew(0.f), ftOld(0.f);

				if (_rcCoilDefects.size() > 0)
				{
					tk_0.start();
					inFloat = cuFloat::alloc(_inDataMat.size[0], _inDataMat.size[1], _inDataMat.size[2], _inDataMat.size[3]);
					_tCls = tk_0.end();

					//tk_0.start();
					//cuFloat::alloc(_inMatCoil, _inDataMat.size[0], _inDataMat.size[1], _fovH, _fovW);
					//ftNew = tk_0.end();

					inFloat->upload(_inDataMat.ptr<float>());
					//_inMatCoil->upload(_inDataMat.ptr<float>());
				}
				tAllocUpload = tk_.end();
				float inferCls(0.f), inferPost(0.f);
				tick tktk;
				cuFloat::ptr res;
				tktk.start();
				if (_rcCoilDefects.size() > 0)
				{
					res = _SessionCoilCls->Inference(inFloat, _bUseHalf);
				}
				//CString totmsg0;
				inferCls = tktk.end();

				tktk.start();
				if (_rcCoilDefects.size() > 0)
				{
					Coil2ndFilterProc(res);
				}
				inferPost = tktk.end();
				//if(dSz==0)
				//	AfxMessageBox(_T("strange"));

				//,Cls_Reset,Upload,Infer,Post

				//_tempMsg_Cls.Format(_T("Cls_Rois,%d\nCls_ImgReset,%.3f\nCls_ImgUpload,%.3f\nCls_Infer,%.3f\nCls_Postwork,%.3f\n"), dSz, t_imgprep, tAllocUpload, inferCls, inferPost);
				_tempMsg_Cls.Format(_T(",%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%3f,%d,%.3f,%.3f,%.3f,%.3f,%d,%.3f,%.3f,"), dSz, t_imgprep, tAllocUpload, inferCls, inferPost, _gpuId, (int)_bUseHalf, _tBit, _tAdjust, _tBounding, _tSegMat, _tSeg, _tSegCopy, (int)_bPaddedSeg, _tSegPadding, _tCls, _tClsImgPre, _tClsCopy, (int)_bPaddedCls, _tClsPadding, _tBoundaryCheck);
				_tempMsg_Cls = _tempMsg_Cls + _gpuName + _T(",");
				//= totmsg0;

			   //WriteCSV(_T("D:\\Eagle3D_data\\log\\zLog_Coil.csv"), _tempMsg_Cls);
			}
		}
		else
		{
			//,gpuNum,gpuName,HalfModeOn
			_tempMsg_Cls.Format(_T(",%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%3f,%d,%.3f,%.3f,%d,%.3f,%.3f,"), dSz, 0, 0, 0, 0, _gpuId, (int)_bUseHalf, _tBit, _tAdjust, _tBounding, _tSegMat, _tSeg, _tCls, _tSegCopy, (int)_bPaddedSeg, _tSegPadding, _tClsImgPre, _tClsCopy, (int)_bPaddedCls, _tClsPadding, _tBoundaryCheck);
			_tempMsg_Cls = _tempMsg_Cls + _gpuName + _T(",");
			//_tempMsg_Cls.Format(_T("Cls_Rois,%d\nCls_ImgReset,%.3f\nCls_ImgUpload,%.3f\nCls_Infer,%.3f\nCls_Postwork,%.3f\n"), _rcCoilDefects.size(), 0, 0, 0, 0);
		}
		return _bCoilGood;
	}
	bool InferCoil::Coil2ndClsImg()
	{
		if (_rcCoilDefects.size() == 0 && _minx.size() == 0)
			return false;
		if (_coilSegInput.empty() == true)
			return false;
		if (_2ndClsImg.empty() == false)
			_2ndClsImg.release();

		tick tk222;
		_tClsImgPre = 0.f;
		tk222.start();
		_inDataMat = _coilSegInput.clone();
		//Removeblobs 진행하며 rcCoilDefects생성해둠

		//_rcCoilDefects.clear();
		////_rcCoilDefects.resize(minx.size());
		//RECT tempRct;
		//for(int iters = 0 ; iters < _minx.size(); iters++)
		//{
		//	tempRct.left = _minx[iters];
		//	tempRct.right = _maxx[iters];
		//	tempRct.top = _miny[iters];
		//	tempRct.bottom = _maxy[iters];
		//	_rcCoilDefects.emplace_back(tempRct);
		//}

		//defect Rect 중심으로부터 224*224 input Image 생성
		std::vector<int> szNewImg;
		szNewImg.resize(4);
		szNewImg[0] = _rcCoilDefects.size();
		szNewImg[1] = _nModelCh;
		szNewImg[2] = 224;
		szNewImg[3] = 224;
		cv::Mat newInput = cv::Mat(4, &szNewImg[0], CV_32FC1, cv::Scalar(0));

		int cx(0), cy(0), o_x, o_y;
		std::vector<cv::Range> srcRange(_inDataMat.dims);
		std::vector<cv::Range> dstRange(_inDataMat.dims);

		std::vector<cv::Mat> srcIn;
		srcIn.emplace_back(_tempResultBlob.clone());
		_tClsImgPre = tk222.end();

		tk222.start();
		_bPaddedCls = false;
		_bPadded = false;
		_2ndClsImg = ZeroPaddingImg(srcIn)[0].clone();
		_bPaddedCls = _bPadded;
		_tClsPadding = tk222.end();

		_tBoundaryCheck = 0.f;
		_tClsCopy = 0.f;
		for (int loop = 0; loop < _rcCoilDefects.size(); loop++)
		{
			tk222.start();
			RECT cropRect;
			cx = gap_width / 2 + (_rcCoilDefects[loop].left + _rcCoilDefects[loop].right) / 2;
			cy = gap_height / 2 + (_rcCoilDefects[loop].top + _rcCoilDefects[loop].bottom) / 2;
			o_x = 0; o_y = 0;
			cropRect.left = cx - 112;
			cropRect.right = cx + 112;;
			cropRect.top = cy - 112;
			cropRect.bottom = cy + 112;;

			//Out of range error exception
			if (cropRect.left < 0)
			{
				o_x = -cropRect.left;
				cropRect.left = 0;
			}
			if (cropRect.right > _inDataMat.size[3])
			{
				cropRect.right = _inDataMat.size[3];
			}
			if (cropRect.top < 0)
			{
				o_y = -cropRect.top;
				cropRect.top = 0;
			}
			if (cropRect.bottom > _inDataMat.size[2])
			{
				cropRect.bottom = _inDataMat.size[2];
			}
			//Out of range error exception

			_tBoundaryCheck += tk222.end();

			tk222.start();
			srcRange[0] = cv::Range(0, 1);
			srcRange[2] = cv::Range(cropRect.top, cropRect.bottom);
			srcRange[3] = cv::Range(cropRect.left, cropRect.right);
			dstRange[0] = cv::Range(loop, loop + 1);
			dstRange[2] = cv::Range(o_y, cropRect.bottom - cropRect.top + o_y);
			dstRange[3] = cv::Range(o_x, cropRect.right - cropRect.left + o_x);

			for (int iters = 0; iters < 3; iters++)
			{
				srcRange[1] = cv::Range(iters, iters + 1);
				dstRange[1] = cv::Range(iters, iters + 1);
				cv::Mat srcCopy(_inDataMat, &srcRange[0]);
				cv::Mat dstCopy(newInput, &dstRange[0]);
				srcCopy.copyTo(dstCopy);
			}
			if (_nModelCh == 4)
			{
				dstRange[1] = cv::Range(3, 4);
				dstRange[2] = cv::Range::all();
				dstRange[3] = cv::Range::all();
				cv::Mat dstCopy(newInput, &dstRange[0]);
				cv::Mat cropArea = cv::Mat(224, 224, _tempResultBlob.type());
				cropArea.setTo(0);
				cv::Mat fMat;
				int wid = cropRect.right - cropRect.left;
				int len = cropRect.bottom - cropRect.top;
				_2ndClsImg(cv::Rect(cropRect.left, cropRect.top, wid, len)).copyTo(cropArea(cv::Rect(o_x, o_y, wid, len)));
				cropArea.convertTo(fMat, CV_32F);
				memcpy(dstCopy.ptr(), fMat.ptr(), fMat.total() * sizeof(float));
			}

			_tClsCopy += tk222.end();
			//}
		}
		_inDataMat.release();
		_inDataMat = newInput.clone();
		return true;
	}
	void InferCoil::WriteCSV(CString path, CString msg)
	{
		CString logPath = path;
		CString extReplace;

		SYSTEMTIME cur_time;
		GetLocalTime(&cur_time);
		CString strTime;
		strTime.Format(_T("%02d%02d%02d%03d,"), cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);
		extReplace.Format(_T("_%02d%02d%02d.csv"), cur_time.wYear, cur_time.wMonth, cur_time.wDay);

		logPath.Replace(_T(".csv"), extReplace);
		bool bExist = PathFileExists(logPath);
		CStdioFile opener;
		if (opener.Open(logPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate))
		{
			opener.SeekToEnd();
			if (bExist == false)
			{
				//totmsg0.Format(_T("serial,%d\nimgw,%d\nimgh,%d\nSeg_Infer,%.3f\nSeg_Prepare,%.3f\nSeg_Postwork,%.3f\n"), _curSequence, _inDataMat.size[2], _inDataMat.size[3], ttime, ttime_segPrepare, ttimePost);
				//_tempMsg_Cls.Format(_T("Cls_Rois,%d\nCls_ImgReset,%.3f\nCls_ImgUpload,%.3f\nCls_Infer,%.3f\nCls_Postwork,%.3f\n"), dSz, t_imgprep, tAllocUpload, inferCls, inferPost);

				opener.WriteString(_T("DateTime,AI_All,AI_prep,Seg,th,Cls,th,PostProc,Wid,Len,Seg_Infer,Pre,Upload,Adjust_Bright,Post,Cls_rois,ImgCopy,Upload,Infer,Post,gpuNum,HalfMode,BitOp,BrightAdjust,mskRangeChk,makeMat,SegAlloc,SegCopy,bPadding,SegPad,ClsAlloc,ClsPre,ClsCopy,bPadding,ClsPad,ClsSizeChk,gpuName,PartName\n"));
				opener.SeekToEnd();
			}
			opener.WriteString(strTime + msg);
			opener.Close();
		}
	}
	void InferCoil::Coil2ndFilterProc(csml::cuFloat::ptr res)
	{
		if (_rcCoilDefects.size() > 0)
		{
			auto resAuto = res->download();
			_clsRes.resize(resAuto.size());
			_clsRes = resAuto;
			//_clsRectList.clear();
			//_clsColorList.clear();
			_tempResultBlob_RECTS = _tempResultBlob.clone();
			for (int iters = 0; iters < resAuto.size(); iters++)
			{
				RECT curRect = _rcCoilDefects[iters];
				CString cstScore;
				cstScore.Format(_T("%.2f"), (float)resAuto[iters]);
				putText(_tempResultBlob_RECTS, std::string(CT2A(cstScore)), cv::Point((curRect.left + curRect.right) / 2, (curRect.top + curRect.bottom) / 2), cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(150, 150, 150), 2);

				if (resAuto[iters] > _clsTh)
				{
					_bCoilGood = false;
				}
				else
				{
					_coilBlob.RemoveLabel(_blobLabelNum[iters]);
					//결함리스트에서 제거
				}
			}
			_tempResultBlob.setTo(0);
			_coilBlob.DrawLabelContours(_tempResultBlob.ptr(), _tempResultBlob.cols, _tempResultBlob.rows, _tempResultBlob.step, 255);

			int nBlobCnt = _coilBlob.m_nBlobRealCount;

			_rctArea.clear(); _rctArea.resize(nBlobCnt);
			_minx.clear(); _minx.resize(nBlobCnt);
			_miny.clear(); _miny.resize(nBlobCnt);
			_maxx.clear(); _maxx.resize(nBlobCnt);
			_maxy.clear(); _maxy.resize(nBlobCnt);
			_rcCoilDefects.resize(nBlobCnt);
			if (nBlobCnt > 0)
			{
				_coilBlob.GetMinMax(&_minx[0], &_miny[0], &_maxx[0], &_maxy[0]);
				_coilBlob.GetArea(&_rctArea[0]);
				_bCoilGood = false;

				for (int iters = 0; iters < nBlobCnt; iters++)
				{
					_rcCoilDefects[iters].left = _minx[iters];
					_rcCoilDefects[iters].right = _maxx[iters];
					_rcCoilDefects[iters].top = _miny[iters];
					_rcCoilDefects[iters].bottom = _maxy[iters];
				}
			}
#ifdef DEBUG_AI
			imwrite(std::string(CT2A(_bsPath + _T("_rmvBlob_Cls.bmp"))), _tempResultBlob_RECTS);
#endif

			RefreshCoilResultImg(_tempResultBlob);
		}
	}
	void InferCoil::RemoveFolder(LPCTSTR pstr)
	{
		TCHAR				curPath[_MAX_PATH];
		WIN32_FIND_DATA		ffd;
		BOOL				bRet = TRUE;

		if (!_tgetcwd(curPath, _MAX_PATH))	return;
		if (_tchdir(pstr))	return;
		HANDLE hFind = ::FindFirstFile(_T("*.*"), &ffd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (_tcscmp(ffd.cFileName, _T(".")) != 0 && _tcscmp(ffd.cFileName, _T("..")) != 0)
				{
					if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						RemoveFolder(ffd.cFileName);
						RemoveDirectory(ffd.cFileName);
					}
					else
					{
						if (!::SetFileAttributes(ffd.cFileName, FILE_ATTRIBUTE_NORMAL))
						{
							bRet = FALSE;
							break;
						}
						if (!::DeleteFile(ffd.cFileName))
						{
							bRet = FALSE;
							break;
						}
					}
				}
				if (!::FindNextFile(hFind, &ffd))
				{
					bRet = TRUE;
					break;
				}
			} while (1);
		}
		::FindClose(hFind);
		_tchdir(curPath);

		if (bRet)
		{
			if (!::SetFileAttributes(pstr, FILE_ATTRIBUTE_NORMAL))
				return;
			Sleep(2);
			if (!::RemoveDirectory(pstr))
			{
				CString		strMsg;
				strMsg.Format(_T("폴더 삭제 실패 - %d"), GetLastError());
				OutputDebugString(strMsg);
			}
		}
	}
	int InferCoil::RemoveBlobs(cv::Mat src)
	{
		_coilBlob.Alloc(src.cols, src.rows);
		int nBlobCnt = _coilBlob.Calculate(src.ptr(), src.cols, src.rows, src.step);
		_coilBlob.CalculateFeatures(true);

		std::vector<int> bLbRemoved;
		bLbRemoved.resize(nBlobCnt);

		//Remove Blobs which do not satisfy wid,len,area criteria
		_coilBlob.RemoveMinArea(_blobLimitA);
		_coilBlob.RemoveSmallObjects(_blobLimitW, _blobLimitH);
		//Remove Blobs which do not satisfy wid,len,area criteria

		nBlobCnt = _coilBlob.m_nBlobRealCount;

		if (nBlobCnt > 0)
		{
			//get roi minmax
			_minx.clear(); _minx.resize(nBlobCnt);
			_miny.clear(); _miny.resize(nBlobCnt);
			_maxx.clear(); _maxx.resize(nBlobCnt);
			_maxy.clear(); _maxy.resize(nBlobCnt);
			_coilBlob.GetMinMax(&_minx[0], &_miny[0], &_maxx[0], &_maxy[0]);
			//get roi minmax

			//Remove Blobs which do not satisfy size dist criteria
			//save original removeList
			memcpy(&bLbRemoved[0], _coilBlob.m_remove + 1, sizeof(int)*bLbRemoved.size());

			cv::Mat imgSpecificLb, imgRoi;
			std::vector<double> szDistList;
			_blobLabelNum.clear();

			//sizeDistance Filtering
			_rstD.clear();
			for (int iter_Outer = 0, iters = 0; iter_Outer < bLbRemoved.size(); iter_Outer++)
			{
				if (bLbRemoved[iter_Outer] != 0)
					continue;
				else if (_blobLimitSzD == 0)
				{// If not process filtering,
				 // save blob label info
					_blobLabelNum.emplace_back(iter_Outer + 1);
					continue;
				}
				//szDist limit > 0 && blobcount > 0
				imgSpecificLb = cv::Mat(src.size(), src.type(), cv::Scalar(0));
				//delete label inverse objects

				_coilBlob.RemoveLabelinv(iter_Outer + 1);
				_coilBlob.DrawLabelContours(imgSpecificLb.ptr(), imgSpecificLb.cols, imgSpecificLb.rows, imgSpecificLb.step, 255);


				//copy and calculate target roi's size dist
				imgRoi = imgSpecificLb(cv::Rect(cv::Point(_minx[iters], _miny[iters]), cv::Point(_maxx[iters], _maxy[iters]))).clone();

				_GaussianLength(imgRoi.ptr(), imgRoi.cols, imgRoi.rows, imgRoi.step, _rstD);

				//compare to cirteria
				if (_rstD[iters] < _blobLimitSzD)
					//set removeflag to true (small object)
					bLbRemoved[iter_Outer] = 1;
				else
				{
					//save size dist defect length list
					szDistList.emplace_back(_rstD[iters]);
					_blobLabelNum.emplace_back(iter_Outer + 1);
				}
				iters++;

				//restore original removedlist
				memcpy(_coilBlob.m_remove + 1, &bLbRemoved[0], sizeof(int)*bLbRemoved.size());
			}
			if (_rstD.size() > 0)
				_coilBlob.m_nBlobRealCount = _rstD.size();
			if (szDistList.size() > 0)
			{
				_rstD.resize(szDistList.size());
				memcpy(&_rstD[0], &szDistList[0], szDistList.size() * sizeof(double));
				szDistList.clear();
			}
			//sizeDistance Filtering
			//refresh info

			nBlobCnt = _coilBlob.m_nBlobRealCount;
			_rctArea.clear(); _rctArea.resize(nBlobCnt);
			_minx.clear(); _minx.resize(nBlobCnt);
			_miny.clear(); _miny.resize(nBlobCnt);
			_maxx.clear(); _maxx.resize(nBlobCnt);
			_maxy.clear(); _maxy.resize(nBlobCnt);
			_bCoilGood = true;
			if (nBlobCnt > 0)
			{
				_coilBlob.GetMinMax(&_minx[0], &_miny[0], &_maxx[0], &_maxy[0]);
				_coilBlob.GetArea(&_rctArea[0]);
				_bCoilGood = false;
			}
			//int mIdx = max_element(_rctArea.begin(), _rctArea.end()) - _rctArea.begin();

		}
		_rcCoilDefects.clear();
		_rcCoilDefects.resize(nBlobCnt);
		for (int iters = 0; iters < nBlobCnt; iters++)
		{
			_rcCoilDefects[iters].left = _minx[iters];
			_rcCoilDefects[iters].right = _maxx[iters];
			_rcCoilDefects[iters].top = _miny[iters];
			_rcCoilDefects[iters].bottom = _maxy[iters];
		}
		src.setTo(0);
		_coilBlob.DrawLabelContours(src.ptr(), src.cols, src.rows, src.step, 255);
		_tempResultBlob = src.clone();
#ifdef DEBUG_AI
		imwrite(std::string(CT2A(_bsPath + _T("_rmvBlob.bmp"))), _tempResultBlob);
#endif
		//그려진 blob의 
		//_coilBlob.Free();
		return nBlobCnt;
	}
	void InferCoil::RefreshCoilResultImg(cv::Mat srcImg)
	{
		cv::Mat tempMat = cv::Mat::zeros(_imgH, _imgW, CV_8UC1);
		srcImg(cv::Rect(_offX, _offY, _mskW - _offX, _mskH - _offY)).clone().copyTo(tempMat(cv::Rect(_mskX + _offX, _mskY + _offY, _mskW - _offX, _mskH - _offY)));

		int nImgHeight = tempMat.rows;
		int nImgWidth = tempMat.cols;
		int nRealWid = tempMat.cols;
		int nn = nRealWid % 4;
		nRealWid += nn == 0 ? 0 : (4 - nn);

		_gaugedImg.clear();
		_gaugedImg.resize(nRealWid*nImgHeight);
		//&pDstData[yy*nRealWid]

		for (int yy = 0; yy < nImgHeight; yy++)
			memcpy(&_gaugedImg[yy*nRealWid], &tempMat.data[yy*nImgWidth], nImgWidth);
	}
	void InferCoil::RefreshFiltImg()
	{
		if (_clsModelImg.empty() == true)
			return;
		cv::Mat tempMat_R = _clsModelImg.clone();
		cv::Mat tempMat_G = _clsModelImg.clone();
		cv::Mat tempMat_B = _clsModelImg.clone();
		int nStep = (tempMat_R.cols / 101);
		int nPos = (int)(_clsTh * 100) * nStep + nStep / 2;
		cv::line(tempMat_R, cv::Point(nPos, 0), cv::Point(nPos, tempMat_R.rows - 1), cv::Scalar(255, 255, 255), 1);
		cv::line(tempMat_G, cv::Point(nPos, 0), cv::Point(nPos, tempMat_R.rows - 1), cv::Scalar(0, 0, 0), 1);
		cv::line(tempMat_B, cv::Point(nPos, 0), cv::Point(nPos, tempMat_R.rows - 1), cv::Scalar(0, 0, 0), 1);
		for (int yy = 0; yy < _modelH; yy++)
		{
			memcpy(&(_2ndFiltModelData[0])[yy*_modelRealW], &tempMat_R.data[yy*_modelW], _modelW);
			memcpy(&(_2ndFiltModelData[1])[yy*_modelRealW], &tempMat_G.data[yy*_modelW], _modelW);
			memcpy(&(_2ndFiltModelData[2])[yy*_modelRealW], &tempMat_B.data[yy*_modelW], _modelW);
		}
		//_modelW = _modelRealW;
	}
	void _GaussianLength(UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nPitch, std::vector<double>& rstD)
	{
		jsl::Blob m_blob;
		m_blob.Alloc(nImgWidth, nImgHeight);
		cv::Mat src(nImgHeight, nImgWidth, CV_8UC1, pByImgSrc, nPitch);
		cv::Mat temp_src;
		src.copyTo(temp_src);
		int width = 0;
		cv::Mat thre_img;
		cv::Mat Mask_img(src.size(), CV_16SC1, cv::Scalar(0));
		m_blob.Calculate(temp_src.ptr(), temp_src.cols, temp_src.rows, temp_src.cols);
		cv::Mat result_img(temp_src.size(), CV_8UC1, cv::Scalar(0));
		m_blob.CalcPerimeter(result_img.ptr(), Mask_img.ptr<USHORT>(), temp_src.cols, temp_src.rows, temp_src.step, width);

		int blobCount = m_blob.m_nBlobRealCount;
		Mask_img.setTo(0);
		thre_img.setTo(0);
		result_img.setTo(0);

		//예외 처리
		//Circle (계산된 Width가 Blob Width, Height의 90% 이상)이면 Width(지름) 리턴
		if (width > 1 && width > nImgWidth*0.9 && width > nImgHeight *0.9)
		{
			rstD.push_back(width);
			return;
		}
		if (width % 2 == 0)width++;

		Mask_img = cv::Mat::zeros(src.rows + width * 2 + 4, src.cols + width * 2 + 4, CV_16SC1);
		thre_img = cv::Mat::zeros(src.rows + width * 2 + 4, src.cols + width * 2 + 4, CV_8UC1);
		result_img = cv::Mat::zeros(src.rows + width * 2 + 4, src.cols + width * 2 + 4, CV_8UC1);

		cv::Mat Extend = cv::Mat::zeros(src.rows + width * 2 + 4, src.cols + width * 2 + 4, CV_8UC1);
		cv::Mat ExtendROI = Extend(cv::Rect(width + 2, width + 2, src.cols, src.rows));
		src.copyTo(ExtendROI);

		cv::GaussianBlur(Extend, Extend, cv::Size(width, width), 0);
		cv::threshold(Extend, thre_img, 0, 255, cv::THRESH_BINARY);
		int tempWidth = width;
		m_blob.Calculate(thre_img.ptr(), thre_img.cols, thre_img.rows, thre_img.cols);
		m_blob.CalcPerimeter(result_img.ptr(), Mask_img.ptr<USHORT>(), thre_img.cols, thre_img.rows, thre_img.step, width);
		//float* n_Lengths = new float[blobCount];
		float* n_Lengths = g_pMManager->pem_new<float>(true, blobCount, (PCHAR)__FUNCTION__, __LINE__);
		m_blob.GetPerimeter(n_Lengths);

		rstD.push_back(n_Lengths[0] - tempWidth);
		m_blob.Free();
		//delete [] n_Lengths;
		g_pMManager->pem_delete(n_Lengths, true);
	}

	void CRAFT_AI_AutoModule::ModuleInit(int nBatch, std::vector<cv::Mat> input_image_vector)
	{
		this->_Core->_batch_size = nBatch;
		this->nBatch = nBatch;
		this->_image_vector = input_image_vector;
		this->batch_result.clear();
		this->batch_result.resize(this->_image_vector.size());
		this->_image_resize_ratio.clear();
		this->_image_center_points_vectors.clear();
	}
	bool CRAFT_AI_AutoModule::LoadModel(CString model_path, int nGpu, std::function<int(int)> select_gpu_device, bool use_option)
	{
		bool beLoaded = false;
		if (use_option) {
			this->beLoaded = this->_Core->LoadModel(model_path, [&](int modelIdx)->int
			{
				// 동일한 GPU로 로드
				return nGpu;
			});
		}
		beLoaded = this->beLoaded;
		return beLoaded;
	}
	void CRAFT_AI_AutoModule::ImageToBatchBuffer(int batch_start_index, int input_batch_size)
	{
		this->_batch_vector.clear();
		this->_batch_vector.resize((size_t)(input_batch_size));
		std::copy(this->_image_vector.begin() + batch_start_index, this->_image_vector.begin() + batch_start_index + input_batch_size, this->_batch_vector.begin());
	}
	void CRAFT_AI_AutoModule::AI_Infer() {
		int batch_start_index = 0;
		int numberOfInputImages = this->_image_vector.size();
		int nBatchLoop = int(std::ceil((float)numberOfInputImages / (float)(this->nBatch)));
		int _input_batch_size;
		this->_Inference_result.clear();
		for (int batch_index = 0; batch_index < nBatchLoop; batch_index++)
		{
			if (batch_index == nBatchLoop - 1)
				_input_batch_size = numberOfInputImages - batch_index * this->nBatch;
			else
				_input_batch_size = this->nBatch;
			batch_start_index = batch_index * this->nBatch;
			this->ImageToBatchBuffer(batch_start_index, _input_batch_size);
			if (this->_batch_vector.size() > 0)
			{
				std::vector<cv::Mat> result_Matrix = this->_Core->Infer(this->_batch_vector);
				for (int i = 0; i < _input_batch_size; i++)
				{
					this->_Inference_result.emplace_back(result_Matrix[i]);
				}
			}
		}
	}
	std::vector<std::vector<cv::Point2f>> CRAFT_AI_AutoModule::CRAFT_AI_returnROI(int set_nBatch, std::vector<cv::Mat> input_image_vector)
	{
		int canvas_size = 640;
		int interpolation = cv::INTER_LINEAR;
		float mag_ratio = 1.5;
		if (this->beLoaded) //모델이 로드 되었을때
		{
			this->ModuleInit(set_nBatch, input_image_vector);
			this->resize_aspect_ratio(this->_image_vector, canvas_size, interpolation, mag_ratio);
			this->normalize_MeanVariance(this->_image_vector);
			AI_Infer();
			this->getCenterPoint(this->_Inference_result, 0.7, 0.6);
			this->adjustResultCoordinates(this->_image_center_points_vectors, this->_image_resize_ratio);

			return this->_image_center_points_vectors;
		}
		else // 모델이 로드되지 않았을떄
		{
			std::vector<std::vector<cv::Point2f>> temp;
			return temp;
		}
	}
	void CRAFT_AI_AutoModule::resize_aspect_ratio(std::vector<cv::Mat> input_image_vector, int canvas_size, int interpolation, float mag_ratio)
	{
		int number_of_images = input_image_vector.size();
		this->_image_vector.clear();
		for (int image_index = 0; image_index < number_of_images; image_index++)
		{
			cv::Mat temp_resized_image;
			cv::Mat temp_resized_image2;
			int img_height = input_image_vector[image_index].rows;
			int img_width = input_image_vector[image_index].cols;
			int img_channel = 3;
			int target_size = mag_ratio * MAX(img_height, img_width);
			if (target_size > canvas_size)
			{
				target_size = canvas_size;
			}
			float ratio = float(target_size) / float(MAX(img_height, img_width));
			this->_image_resize_ratio.emplace_back(ratio);
			int target_h, target_w;
			target_h = int(img_height * ratio);
			target_w = int(img_width * ratio);
			cv::resize(input_image_vector[image_index], temp_resized_image, cv::Size(target_w, target_h), (0, 0), (0, 0), interpolation);
			int target_h32, target_w32;
			target_h32 = target_h;
			target_w32 = target_w;
			if (target_h % 32 != 0) {
				target_h32 = target_h + (32 - target_h % 32);
			}
			if (target_w % 32 != 0) {
				target_w32 = target_w + (32 - target_w % 32);
			}
			temp_resized_image2 = cv::Mat::zeros(cv::Size(target_w32, target_h32), CV_8UC3);
			cv::Mat ImageROI(temp_resized_image2, cv::Rect(0, 0, target_w, target_h));
			temp_resized_image.copyTo(ImageROI);
			this->_image_vector.emplace_back(temp_resized_image2);
		}
	}
	void CRAFT_AI_AutoModule::normalize_MeanVariance(std::vector<cv::Mat> input_image_vector)
	{
		float PreProc_mean_value[] = { 0.485,0.456,0.406 };
		float PreProc_Variance_value[] = { 0.229,0.224,0.225 };
		int number_of_images = input_image_vector.size();
		this->_image_vector.clear();
		for (int image_index = 0; image_index < number_of_images; image_index++) {
			cv::Mat bgr[3];
			cv::Mat rgb[3];
			cv::split(input_image_vector[image_index], bgr);
			for (int i = 0; i < 3; i++)
			{
				bgr[2 - i].convertTo(rgb[i], CV_32F);
				rgb[i] -= 255.0 * PreProc_mean_value[i];
				rgb[i] /= 255.0 * PreProc_Variance_value[i];
			}
			cv::Mat MergeImage;
			cv::merge(rgb, 3, MergeImage);
			this->_image_vector.emplace_back(MergeImage);
		}
	}
	void CRAFT_AI_AutoModule::getCenterPoint(std::vector<cv::Mat> score_map_image_vector, float text_threshold, float low_text)
	{
		int number_of_images = score_map_image_vector.size();
		std::vector<std::vector<cv::Point2f>> image_center_points_vectors;
		jsl::Blob blob;
		for (int image_index = 0; image_index < number_of_images; image_index++)
		{
			cv::Mat _score_map_image;
			std::vector<cv::Point2f> center_point_vector;
			center_point_vector.clear();
			score_map_image_vector[image_index].convertTo(_score_map_image, CV_32FC1);
			_score_map_image /= 255.0;
			int image_height = _score_map_image.rows;
			int image_width = _score_map_image.cols;
			cv::Mat text_score;
			cv::threshold(_score_map_image, text_score, low_text, 1.0, 0);
			text_score.convertTo(text_score, CV_8UC1);
			cv::Mat labels, stats, centroids;
			int nLabels;
			cv::Mat text_score_copy = text_score.clone();
			text_score_copy *= 255;
			blob.Alloc(text_score_copy.cols, text_score_copy.rows);
			nLabels = blob.Calculate(text_score_copy.data, text_score_copy.cols, text_score_copy.rows, text_score_copy.step);
			cv::Mat label_maps = cv::Mat::zeros(text_score_copy.size(), CV_8UC1);
			blob.DrawLabelContours(label_maps.data, label_maps.cols, label_maps.rows, label_maps.step, 255);
			std::vector<float> label_areas;
			std::vector<RECT> label_rects;
			label_areas.resize((int)nLabels);
			label_rects.resize((int)nLabels);
			label_rects = blob.GetRects();
			if (label_areas.size() < 1)
				return;
			blob.GetArea(&label_areas[0]);
			for (int label_index = 1; label_index <= nLabels; label_index++)
			{
				int _Area_size = label_areas[label_index - 1];
				int _left = label_rects[label_index - 1].left;
				int _top = label_rects[label_index - 1].top;
				int _width = label_rects[label_index - 1].right - label_rects[label_index - 1].left + 1;
				int _height = label_rects[label_index - 1].bottom - label_rects[label_index - 1].top + 1;
				float _center_x = float(_left) + float(_width) / 2.0;
				float _center_y = float(_top) + float(_height) / 2.0;
				cv::Mat _AREA_ROI(_score_map_image, cv::Rect(_left, _top, _width, _height));
				double min, max;
				cv::minMaxIdx(_AREA_ROI, &min, &max);
				if (_Area_size < 10)
					continue;
				if (max < text_threshold)
					continue;
				cv::Point2f _center_point(_center_x, _center_y);
				//center_point_vector.emplace_back(_center_point);


				int center_point_vectors_size = center_point_vector.size();

				if (center_point_vectors_size == 0) {
					center_point_vector.emplace_back(_center_point);
				}
				else {
					for (int j = 0; j < center_point_vectors_size; j++) {
						if (center_point_vector[j].x > _center_point.x) {
							center_point_vector.insert(center_point_vector.begin() + j, _center_point);
							break;
						}
						else if (j == center_point_vectors_size - 1) {
							center_point_vector.emplace_back(_center_point);
						}
					}
				}
			}
			image_center_points_vectors.emplace_back(center_point_vector);
		}
		this->_image_center_points_vectors = image_center_points_vectors;
	}
	void CRAFT_AI_AutoModule::adjustResultCoordinates(std::vector<std::vector<cv::Point2f>> image_center_points_vectors, std::vector<float> image_resize_ratio)
	{
		int number_of_images = image_center_points_vectors.size();
		for (int image_index = 0; image_index < number_of_images; image_index++)
		{
			int number_of_boxes = image_center_points_vectors[image_index].size();
			float image_ratio = 1.0 / image_resize_ratio[image_index];
			for (int box_index = 0; box_index < number_of_boxes; box_index++)
			{
				this->_image_center_points_vectors[image_index][box_index] = image_center_points_vectors[image_index][box_index] * image_ratio * 2;
			}
		}
	}

	bool PyOCR_AI_AutoModule::LoadModel(CString model_path, int select_gpu_device, bool use_option)
	{
		bool beLoaded = false;
		if (use_option) {
			this->beLoaded = this->_Core->LoadModel(model_path, select_gpu_device);
		}
		beLoaded = this->beLoaded;
		return beLoaded;
	}

	void PyOCR_AI_AutoModule::ModuleInit(int nBatch, std::vector<cv::Mat> input_images)
	{
		this->_batch_vector.clear();
		this->_Inference_result.clear();
		this->_image_vector.clear();
		this->_original_image_vector.clear();

		this->nBatch = nBatch;
		this->_original_image_vector = input_images;
	}

	std::vector<cv::Mat> PyOCR_AI_AutoModule::OCRInference(std::vector<cv::Mat> input_image_vector, int set_nBatch)
	{
		std::vector<cv::Mat> no_result;

		if (this->beLoaded)
		{
			this->ModuleInit(set_nBatch, input_image_vector);
			this->OCRPreProcess(input_image_vector);
			this->_Infer();

			return this->_Inference_result;
		}
		return no_result;
	}

	void PyOCR_AI_AutoModule::ImageToBatchBuffer(int batch_start_index, int input_batch_size)
	{
		this->_batch_vector.clear();
		this->_batch_vector.resize((size_t)(input_batch_size));
		std::copy(this->_image_vector.begin() + batch_start_index, this->_image_vector.begin() + batch_start_index + input_batch_size, this->_batch_vector.begin());
	}

	void PyOCR_AI_AutoModule::_Infer()
	{
		int batch_start_index = 0;
		int NumOfInputImages = this->_image_vector.size();
		int nBatchLoop = int(ceil(NumOfInputImages / (1.0*this->nBatch)));
		int _input_batch_size;

		for (int batch_index = 0; batch_index < nBatchLoop; batch_index++)
		{
			if (batch_index == nBatchLoop - 1)
				_input_batch_size = NumOfInputImages - batch_index * this->nBatch;
			else
				_input_batch_size = this->nBatch;

			batch_start_index = batch_index * this->nBatch;
			this->ImageToBatchBuffer(batch_start_index, _input_batch_size);

			if (this->_batch_vector.size() > 0)
			{
				std::vector<cv::Mat> result_Matrix = this->_Core->Infer(this->_batch_vector);

				for (int i = 0; i < _input_batch_size; i++)
				{
					this->_Inference_result.emplace_back(result_Matrix[i]);
				}
			}
		}
	}

	void PyOCR_AI_AutoModule::OCRPreProcess(std::vector<cv::Mat> input_image_vector)
	{
		this->_image_vector.clear();
		for (int i = 0; i < input_image_vector.size(); i++)
		{
			this->_image_vector.emplace_back(this->AdjustPreProcess(input_image_vector[i]));
		}
	}

	cv::Mat PyOCR_AI_AutoModule::AdjustPreProcess(cv::Mat input_image)
	{
		cv::Mat histImg;
		cv::equalizeHist(input_image, histImg);

		cv::Mat tempMat, ResizeMat;
		int offset = 0.5 * abs(histImg.cols - histImg.rows);
		if (histImg.cols > histImg.rows)
		{
			tempMat = cv::Mat(histImg.cols, histImg.cols, histImg.type(), cv::Scalar::all(0));
			histImg.copyTo(tempMat(cv::Rect(0, offset, histImg.cols, histImg.rows)));
		}
		else
		{
			tempMat = cv::Mat(histImg.rows, histImg.rows, histImg.type(), cv::Scalar::all(0));
			histImg.copyTo(tempMat(cv::Rect(offset, 0, histImg.cols, histImg.rows)));
		}

		cv::resize(tempMat, ResizeMat, cv::Size(this->_image_width, this->_image_height), 0.0, 0.0, cv::INTER_NEAREST);

		cv::Mat Dst;
		cv::GaussianBlur(ResizeMat, Dst, cv::Size(this->_blur_filter_size, this->_blur_filter_size), 0.0);
		return Dst;
	}

	bool PyOCR_AI_AutoModule::EmptyModel(int netID)
	{
		//only netID = 0
		if (netID == 0)
		{
			return this->_Core->EmptyModel();
		}
		else {
			return false;
		}

	}

	CString PyOCR_AI_AutoModule::ModelID()
	{
		return _Core->get_model_id();
	}
}
