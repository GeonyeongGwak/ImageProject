#include "stdafx.h"
#include "InferBubble.h"

cv::Mat InferBubble::Preprocess(cv::Mat srcImg)
{
	std::vector<int> dims(4);
	cv::Mat tempMat, floatMat, ResizeMat;

	// Padded Image
	offset_x = (32 - srcImg.cols % 32) % 32;
	offset_y = (32 - srcImg.rows % 32) % 32;

	ResizeMat = cv::Mat(_imgLen + offset_y, _imgWid + offset_x, CV_8UC1, cv::Scalar(0));
	srcImg.copyTo(ResizeMat(cv::Rect(offset_x / 2, offset_y / 2, _imgWid, _imgLen)));

	// Convert uint8 to float32
	ResizeMat.convertTo(floatMat, CV_32FC1);
	floatMat /= 255.f;

	// Memory Copy
	dims[0] = 1;
	dims[1] = 1;
	dims[2] = _imgLen + offset_y;
	dims[3] = _imgWid + offset_x;

	cv::Mat dst = cv::Mat(4, &dims[0], CV_32FC1, cv::Scalar(0));
	memcpy(dst.ptr(), floatMat.ptr(), floatMat.total() * sizeof(float));

	return dst;
}
void InferBubble::Postprocess(cv::Mat res, cv::Mat &dst, bool bAuto)
{
	cv::Mat partImg, dataTemp, biImg;

	dataTemp = res;

	if (bAuto)
	{
		double dMin(-1), dMax(1);
		cv::minMaxLoc(res, &dMin, &dMax);
		dataTemp = (res - dMin) / (dMax - dMin + 0.0000001);
	}

	dataTemp *= 255.0;
	dataTemp.convertTo(partImg, CV_8UC1);
	cv::threshold(partImg, biImg, _binary_threshold, 255, cv::THRESH_BINARY);
	cv::Rect rect(offset_x / 2, offset_y / 2, _imgWid, _imgLen);
	biImg(rect).copyTo(dst);
}
std::shared_ptr<InferResult> InferBubble::Infer(std::vector<cv::Mat> ImgBuffer, int sessionIdx)
{
	if (sessionIdx >= _Session.size())
		return nullptr;

	////// Check Model Load
	if (EmptyModel() || ImgBuffer.size() == 0)
		return nullptr;

	tick tk, tk1, tk2, tk3, tk4, total_tk;
	total_tk.start();
	_saveDebugImg = true;

	////// 초기화
	tk.start();

	std::shared_ptr<InferResult> Res(new InferResult());
	int _imgCnt = ImgBuffer.size();
	init();
	_imgWid = ImgBuffer[0].cols;
	_imgLen = ImgBuffer[0].rows;
	_imgCh = 1;
	_Session[sessionIdx]->init();
	int stPos = 0, nCnt = 0;
	int inspCount = (int)ceil((double)_imgCnt / (double)_block);

	std::vector<int> bufSz(4);
	bufSz[0] = _imgCnt;
	bufSz[1] = _imgCh;
	bufSz[2] = _imgLen;
	bufSz[3] = _imgWid;
	Res->netRes = cv::Mat(4, &bufSz[0], CV_8U);

	_init_time = tk.end();

	cv::Mat output;
	for (int inspCnt = 0; inspCnt < inspCount; inspCnt++)
	{
		// Batch 세팅
		tk.start();
		stPos += nCnt;
		if (ImgBuffer.size() < stPos + _block)
			nCnt = ImgBuffer.size() - stPos;
		else
			nCnt = _block;
		_init_time += tk.end();

		// Input Matrix 생성
		tk1.start();
		cv::Mat _inMat = Preprocess(ImgBuffer[inspCnt]);
		_preprocess_time += tk1.end();

		// Inference 수행
		tk2.start();
		cv::Mat res = _Session[sessionIdx]->Inference(_inMat);
		_inference_time += tk2.end();

		tk3.start();
		// Split to Single Image for Save
		std::vector<cv::Range> range(4);
		range[0] = cv::Range(inspCnt, inspCnt + 1);
		range[1] = cv::Range::all();
		range[2] = cv::Range::all();
		range[3] = cv::Range::all();
		cv::Mat dstdata = cv::Mat(Res->netRes, &range[0]);
		output = cv::Mat(_imgLen, _imgWid, CV_8U, dstdata.ptr());

		// Convert 4D to 2D
		std::vector<cv::Range> srcRange(4);
		srcRange[0] = cv::Range::all();
		srcRange[1] = cv::Range::all();
		srcRange[2] = cv::Range::all();
		srcRange[3] = cv::Range::all();
		cv::Mat data = cv::Mat(res, &srcRange[0]);
		cv::Mat res2D = cv::Mat(_imgLen + offset_y, _imgWid + offset_x, CV_32F, data.ptr());

		// Unpadded Image
		Postprocess(res2D, output, false);
		_postprocess_time += tk3.end();

		if (_saveDebugImg)
		{
			tk4.start();
			CString savePath;
			CTime tm = CTime::GetCurrentTime();
			CString sDatetime = tm.Format(_T("%y%m%d%H%M%S"));

			savePath.Format(_T("%s\\FOV_%d_%s_UV.bmp"), _AINGImgPath, inspCnt, sDatetime);
			cv::imwrite((std::string)(CT2CA)savePath, ImgBuffer[inspCnt]);
			savePath.Format(_T("%s\\Label_%d_%s_UV.bmp"), _AINGImgPath, inspCnt, sDatetime);
			cv::imwrite((std::string)(CT2CA)savePath, output);
			_res_save_time += tk4.end();
		}
	}

#ifndef _AITester
	if (inspCount == 1)
		Res->netRes = output;
#endif

	_total_infer_time += total_tk.end();

	if (_saveDebugImg)
		SaveReport(_AINGImgPath, inspCount);

	return Res;
}
void InferBubble::SaveReport(CString report_save_path, int inspCount)
{
	// Test 관련 정보 - 파일로 저장
	std::ofstream o_f;
	o_f.open((std::string)(CT2CA)report_save_path + "\\TestReport.csv", std::ios::app);
	o_f << "Path :" << "," << (std::string)(CT2CA)report_save_path << std::endl;
	o_f << "Model" << "," << (std::string)(CT2CA)modelPath[0] << std::endl;
	o_f << "Src Img Size,width," << std::to_string((long double)_imgWid) << ",height," << std::to_string((long double)_imgLen) << std::endl;
	o_f << "Input Img Size,width," << std::to_string((long double)_imgWid + offset_x) << ",height," << std::to_string((long double)_imgLen + offset_y) << std::endl;
	o_f << "Insp Img Count," << std::to_string((long double)inspCount) << std::endl << std::endl;

	o_f << "Init Time" << "," << std::to_string((long double)_init_time) << std::endl;
	o_f << "Load Time" << "," << std::to_string((long double)_preprocess_time) << std::endl;
	o_f << "Inference Time" << "," << std::to_string((long double)_inference_time) << std::endl;
	o_f << "Post-process Time" << "," << std::to_string((long double)_postprocess_time) << std::endl;
	o_f << "Result Save Time" << "," << std::to_string((long double)_res_save_time) << std::endl;
	o_f << "Total Infer Time" << "," << std::to_string((long double)_total_infer_time) << std::endl << std::endl;

	o_f.close();
}

#ifndef _AITester
std::vector<int> InferBubbleCSML::GetDevice()
{
	std::vector<int> nGpuId;
	nGpuId.emplace_back(_gpuId);

	return nGpuId;
}
int InferBubbleCSML::GetDevice(int sessionIdx)
{
	return _gpuId;
}

bool InferBubbleCSML::LoadModel(CString path, std::function<int(int)> selDev)
{
	if (path.Right(4) != _T(".cs2"))
		return false;

	totalGpuCount = GPUCount();

	_bLoadModel = false;
	if (totalGpuCount > 0) // GPU가 존재하는 경우
	{
		// session 객체 생성 & path 경로의 .cs2 파일 로드		
		modelPath = path;

		if (selDev)
			_gpuId = -1;
		else
			_gpuId = selDev(0);

		if (_gpuId < 0) _gpuId = 0; // CSML -> CPU 지원하지 않는 AI 모듈

		_session = csml::session::ptr(new csml::session(modelPath, _gpuId, _gpuCnt));

		if (_session->_Nets.size() == 0)
		{
			_session.reset();
			_bLoadModel = false;
		}
		else
			_bLoadModel = true;
	}

	if (_bLoadModel)
	{
		//// Input Image 배율 = 2^(downSampling 횟수)
		if (_session != nullptr && _session->_Nets.size() > 0)
			downSamplingCnt = _session->DownSamplingRate();
	}
	else
	{
		padded_width = 0;
		padded_height = 0;
		gap_width = 0;
		gap_height = 0;
		downSamplingCnt = 1;
	}

	return _bLoadModel;
}

//CString InferBubble::ModelID()
//{
//	if (_SessionSeg != nullptr)
	//	return _SessionSeg->model_id();
	//else
	//	return _T("");
//}
CString InferBubbleCSML::ModelID(int mode)
{
	if (_session != nullptr)
		return _session->model_id();
	else
		return _T("");
}

void InferBubbleCSML::ReleaseModel()
{
	if (_session)
		_session.reset();
}
void InferBubbleCSML::ReleaseModel(int netID)
{
	ReleaseModel();
}

bool InferBubbleCSML::EmptyModel()
{
	if (_session == nullptr)
		return true;
	else if (_session->_Nets.size() == 0)
		return true;
	else
	{
		for (int cnt = 0; cnt < _session->_Nets.size(); cnt++)
			if ((_session->_Nets[cnt]->net()).size() == 0)
				return true;
	}

	return false;
}
bool InferBubbleCSML::EmptyModel(int netID)
{
	return EmptyModel();
}

int InferBubbleCSML::GPUCount()
{
	return csml::runtime::gpu_count();
}

void InferBubbleCSML::init()
{
	if (init_prop == nullptr)
		init_prop = std::shared_ptr<InferInitBubble>(new InferInitBubble());

	init(init_prop);
}
void InferBubbleCSML::init(std::shared_ptr<InferInit> ptr)
{
	InferBase::init(ptr);

	std::shared_ptr<InferInitBubble> prop = std::static_pointer_cast<InferInitBubble, InferInit>(ptr);

	_binary_threshold = prop->binary_threshold;
	_block = prop->block;

	padded_width = 0;
	padded_height = 0;
	gap_width = 0;
	gap_height = 0;
}

cv::Mat InferBubbleCSML::ZeroPaddingImg(cv::Mat FOV)
{
	//// zero-padding 넣은 사이즈
	gap_width = downSamplingCnt - (FOV.cols % downSamplingCnt) % downSamplingCnt;
	gap_height = downSamplingCnt - (FOV.rows % downSamplingCnt) % downSamplingCnt;

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
std::shared_ptr<InferResult> InferBubbleCSML::Infer(std::vector<cv::Mat> In, int SaveNum)
{
	std::shared_ptr<InferResult> Res(new InferResult());

	////// Check Model Load
	if (_session == NULL)
	{
		if (modelPath != "")
		{
			if (totalGpuCount > 0)
				_session = csml::session::ptr(new csml::session(modelPath, 0));
		}
		else
		{
			//AfxMessageBox(_T("Model is NOT loaded"));
			return Res;
		}
	}

	if (_session == NULL)
		return Res;

	////// Initialize
	cv::Mat InFOV;
	csml::tick tk, tk1, tk2, tk3, tk4, total_tk;
	total_tk.start();
	tk.start();
	init();
	csml::cuByte::alloc(inImg, 1, 1, In[0].rows, In[0].cols); // Input FOV »çÀÌÁî °íÁ¤ ÇÊ¿ä

	for (int i = 0; i < 1; i++)    // Only Use First Image
	{
		InFOV = In[i];
		inImg->upload(InFOV.ptr());

		ZeroPaddingImg(InFOV);

		csml::cuFloat::ptr inMat = csml::cuFloat::alloc(1, 1, padded_height, padded_width);
		if (postprocessing_buffer == nullptr)
			postprocessing_buffer = csml::cuFloat::alloc(1, 1, InFOV.rows, InFOV.cols);
		_init_time = tk.end();

		////// Input Matrix
		tk1.start();
		csml::Mat::fov2mat(inImg, InFOV.step, inMat, 0, 0, gap_width / 2, gap_height / 2);
		csml::runtime::thread_sync();
		_preprocess_time = tk1.end();

		////// Inference
		tk2.start();
		csml::cuFloat::ptr result = _session->Inference(inMat, half_mode);
		auto hres = result->download();
		_inference_time = tk2.end();

		////// Post-process
		tk3.start();
		csml::Mat::mat2fov(result, result->size(3), result->size(2), result->size(3), gap_width / 2, gap_height / 2, inImg, InFOV.rows, InFOV.cols, postprocessing_buffer, _binary_threshold); // Out Image Size °ü·Ã Offset ¼öÄ¡ ¼öÁ¤
		cv::Mat Label(InFOV.rows, InFOV.cols, CV_8UC1);
		inImg->download(Label.ptr<uchar>());
		Res->netRes = Label;
		_postprocess_time = tk3.end();
		_total_infer_time = total_tk.end();

		////// Save Debug Info
		if (_saveDebugImg)
		{
			tk4.start();
			if (SaveNum > -1)
			{
				cv::imwrite((std::string)(CT2CA)_AINGImgPath + "\\Inferenced_FOV.bmp", InFOV);
				cv::imwrite((std::string)(CT2CA)_AINGImgPath + "\\Inferenced_Label.bmp", Label);
			}
			else
			{
				CString _sNum;
				_sNum.Format(_T("%d"), SaveNum);
				cv::imwrite((std::string)(CT2CA)_AINGImgPath + "\\FOV_" + (std::string)(CT2CA)_sNum + "_UV.bmp", InFOV);
				cv::imwrite((std::string)(CT2CA)_AINGImgPath + "\\Label_" + (std::string)(CT2CA)_sNum + "_UV.bmp", Label);
			}
			_res_save_time = tk4.end();

			SaveReport(_AINGImgPath, SaveNum);
		}
	}

	return Res;
}
void InferBubbleCSML::SaveReport(CString report_save_path, int AddedSaveName)
{
	// Test 관련 정보 - 파일로 저장
	std::ofstream o_f;
	CString folderName;
	if (AddedSaveName < 0)
		folderName = _AINGImgPath.Right(_AINGImgPath.GetLength() - _AINGImgPath.ReverseFind('\\') - 1);
	else
		folderName.Format(_T("%d"), AddedSaveName);
	o_f.open((std::string)(CT2CA)report_save_path + "\\TestReport(" + (std::string)(CT2CA)folderName + ").csv", std::ios::app);
	o_f << "Path :" << "," << (std::string)(CT2CA)report_save_path << std::endl;
	o_f << "Folder" << "," << (std::string)(CT2CA)folderName << "," << std::endl;
	o_f << "Model Path" << "," << "" << (std::string)(CT2CA)modelPath << std::endl;
	o_f << "Src Img Size" << "," << "width" << "," << std::to_string((long double)_imgWid) << "," << "height" << "," << std::to_string((long double)_imgLen) << std::endl;
	o_f << "Input Img Size" << "," << "width" << "," << std::to_string((long double)padded_width) << "," << "height" << "," << std::to_string((long double)padded_height) << std::endl;

	o_f << "Init Time" << "," << std::to_string((long double)_init_time) << std::endl;
	o_f << "Load Time" << "," << std::to_string((long double)_preprocess_time) << std::endl;
	o_f << "Inference Time" << "," << std::to_string((long double)_inference_time) << std::endl;
	o_f << "Post-process Time" << "," << std::to_string((long double)_postprocess_time) << std::endl;
	o_f << "Result Save Time" << "," << std::to_string((long double)_res_save_time) << std::endl;
	o_f << "Total Infer Time" << "," << std::to_string((long double)_total_infer_time) << std::endl << std::endl;

	o_f.close();
}
#endif

void InferBubbleRT::init()
{
	if (init_prop == nullptr)
		init_prop = std::shared_ptr<InferInit>(new InferInitBubble());

	init(init_prop);
}
void InferBubbleRT::init(std::shared_ptr<InferInit> ptr)
{
	InferBase::init(ptr);

	std::shared_ptr<InferInitBubble> prop = std::static_pointer_cast<InferInitBubble, InferInit>(ptr);

	_binary_threshold = prop->binary_threshold;
	_block = prop->block;
}
cv::Mat InferBubbleRT::Preprocess(cv::Mat srcImg)
{
	// Padded Image
	offset_x = (32 - srcImg.cols % 32) % 32;
	offset_y = (32 - srcImg.rows % 32) % 32;

	if (offset_x == 0 && offset_y == 0)
		return srcImg;

	cv::Mat ResizeMat = cv::Mat(_imgLen + offset_y, _imgWid + offset_x, CV_8UC1, cv::Scalar(0));
	//	srcImg.copyTo(ResizeMat(cv::Rect(offset_x / 2, offset_y / 2, _imgWid, _imgLen)));

	int srcPitch = srcImg.step1();
	int dstPitch = ResizeMat.step1();
	BYTE * srcPtr = srcImg.ptr();
	BYTE * dstPtr = ResizeMat.ptr();
	for (size_t y = 0; y < _imgLen; y++)
	{
		int dst_i = (y + offset_y / 2) * dstPitch;
		int src_i = y * srcPitch;
		memcpy(&dstPtr[dst_i + offset_x / 2], &srcPtr[src_i], _imgWid);
	}

	return ResizeMat;
}
void InferBubbleRT::Postprocess(cv::Mat res, cv::Mat &dst, bool bAuto)
{
	if (offset_x != 0 || offset_y != 0)
	{
		cv::Mat dataTemp, biImg;
		dataTemp = res;

		//	cv::threshold(res, biImg, _binaryThreshold, 255, cv::THRESH_BINARY);
		//	cv::Rect rect(offset_x / 2, offset_y / 2, _imgWid, _imgLen);
		//	res(rect).copyTo(dst);

		cv::Mat srcImg = res;
		dst = cv::Mat(_imgLen, _imgWid, CV_8UC1, cv::Scalar(0));
		//cv::Mat DstMat = dst;

		int srcPitch = srcImg.step1();
		int dstPitch = dst.step1();
		BYTE * srcPtr = srcImg.ptr();
		BYTE * dstPtr = dst.ptr();
		for (size_t y = 0; y < _imgLen; y++)
		{
			int dst_i = y * dstPitch;
			int src_i = (y + offset_y / 2) * srcPitch;
			memcpy(&dstPtr[dst_i], &srcPtr[src_i + offset_x / 2], _imgWid);
		}
	}
}
std::shared_ptr<InferResult> InferBubbleRT::Infer(std::vector<cv::Mat> imgBuffer, int sessionIdx)
{
	BubbleItems BubItem;
	BubItem.threshold = _binary_threshold;
	_imgWid = imgBuffer[0].cols;
	_imgLen = imgBuffer[0].rows;
	_Session[0]->SetPreperenceItems(&BubItem);

#ifdef _AITester
	std::shared_ptr<InferResVec> result = std::shared_ptr<InferResVec>(new InferResVec());;
	for (int i = 0; i < imgBuffer.size(); i++) {
		Preprocess(imgBuffer[i]).copyTo(imgBuffer[i]);

		cv::Mat tmpMat = _Session[0]->Inference(imgBuffer[i]);

		Postprocess(tmpMat, tmpMat, false);
		result->netResBuf.emplace_back(tmpMat);
	}
#else	
	std::shared_ptr<InferResult> result = std::shared_ptr<InferResult>(new InferResult());
	Preprocess(imgBuffer[0]).copyTo(imgBuffer[0]);

	cv::Mat tmpMat = _Session[0]->Inference(imgBuffer[0]);

	Postprocess(tmpMat, result->netRes, false);

	////// Save Debug Info
	if (_saveDebugImg)
	{
		CString _sNum;
		_sNum.Format(_T("%d"), 0);
		cv::imwrite((std::string)(CT2CA)_AINGImgPath + "\\FOV_" + (std::string)(CT2CA)_sNum + "_UV.bmp", imgBuffer[0]);
		cv::imwrite((std::string)(CT2CA)_AINGImgPath + "\\Label_" + (std::string)(CT2CA)_sNum + "_UV.bmp", result->netRes);

		SaveReport(_AINGImgPath, 0);
	}
#endif
	return result;
}

void InferBubbleRT::SaveReport(CString report_save_path, int AddedSaveName)
{
	// Test
	std::ofstream o_f;
	CString folderName;
	if (AddedSaveName < 0)
		folderName = _AINGImgPath.Right(_AINGImgPath.GetLength() - _AINGImgPath.ReverseFind('\\') - 1);
	else
		folderName.Format(_T("%d"), AddedSaveName);
	o_f.open((std::string)(CT2CA)report_save_path + "\\TestReport(" + (std::string)(CT2CA)folderName + ").csv", std::ios::app);
	o_f << "Path," << (std::string)(CT2CA)report_save_path << std::endl;
	o_f << "Folder," << (std::string)(CT2CA)folderName << "," << std::endl;
	o_f << "Model ID," << (std::string)(CT2CA)ModelID(0) << std::endl;
	o_f << "Src Img Size,width," << std::to_string((long double)_imgWid) << ",height," << std::to_string((long double)_imgLen) << std::endl;
	o_f << "Input Img Size,width," << std::to_string((long double)_imgWid + offset_x) << ",height," << std::to_string((long double)_imgLen + offset_y) << std::endl;

	/*o_f << "Init Time" << "," << std::to_string((long double)_init_time) << std::endl;
	o_f << "Load Time" << "," << std::to_string((long double)_preprocess_time) << std::endl;
	o_f << "Inference Time" << "," << std::to_string((long double)_inference_time) << std::endl;
	o_f << "Post-process Time" << "," << std::to_string((long double)_postprocess_time) << std::endl;
	o_f << "Result Save Time" << "," << std::to_string((long double)_res_save_time) << std::endl;
	o_f << "Total Infer Time" << "," << std::to_string((long double)_total_infer_time) << std::endl << std::endl;*/

	o_f.close();
}