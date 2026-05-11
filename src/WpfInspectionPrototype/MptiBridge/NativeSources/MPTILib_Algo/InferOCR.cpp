#include "stdafx.h"
#include "InferOCR.h"
#include "tick.h"

int CRAFT_InferCore::GPUCount()
{
	int gpu_count = -1;
	if (this->Sessions.size() != 0)
	{
		gpu_count = this->Sessions[0]->get_gpu_count();
	}

	return gpu_count;
}
int CRAFT_InferCore::GetDevice(int session_idx)
{
	int gpu_id = this->Sessions[session_idx]->get_device();
	return gpu_id;
}
bool CRAFT_InferCore::LoadModel(CString ai_file_path, std::function<int(int)> selectDev)
{
	bool beResult = false;
	std::vector<int> gpu_id;
	AiFileHeader file_header;

	if (awm::get().ai_get_file_info(ai_file_path, file_header))
	{
		gpu_id.resize(file_header.nFileCount);
		for (int i = 0; i < file_header.nFileCount; i++)
		{
			if (selectDev) {
				gpu_id[i] = selectDev(i);
			}
			else {
				gpu_id[i] = -1;
			}
		}

		this->Sessions.resize(file_header.nFileCount);
		beResult = awm::get().ai_load_file(ai_file_path, 0, file_header.nFileCount, &gpu_id[0], &this->Sessions[0]);
	}

	return beResult;
}
bool CRAFT_InferCore::EmptyModel()
{
	if (this->Sessions.size() == 0) {
		return true;
	}
	else {
		return false;
	}
}
void CRAFT_InferCore::init()
{

}
std::vector<cv::Mat> CRAFT_InferCore::Infer(std::vector<cv::Mat> input_batch_image_buffer)
{
	int input_image_size = input_batch_image_buffer.size();
	std::vector<cv::Mat> infer_result;
	for (int image_index = 0; image_index < input_image_size; image_index++) {
		cv::Mat score_map_result = this->Sessions[0]->Inference(input_batch_image_buffer[image_index]);
		infer_result.emplace_back(score_map_result);
	}
	return infer_result;
}

// resize (224, 224), convert into 4D mat
cv::Mat InferOcrCmp::Preprocess(cv::Mat srcImg)		
{
	std::vector<int> dims(4);
	cv::Mat tempMat, floatMat, ResizeMat;
	int _w, _h, new_w, new_h, x1, y1;

	ResizeMat = cv::Mat(_imgLen, _imgWid, CV_8UC1, cv::Scalar(0));
	_w = srcImg.cols;
	_h = srcImg.rows;
	if (_w > _h)
	{
		new_w = _imgWid;
		new_h = _imgLen * _h / _w;
		x1 = 0;
		y1 = (_imgLen - new_h) / 2;
	}
	else
	{
		new_w = _imgWid * _w / _h;
		new_h = _imgLen;
		x1 = (_imgWid - new_w) / 2;
		y1 = 0;
	}
	
	x1 = std::max(x1, 0);
	y1 = std::max(y1, 0);
	x1 = std::min(x1, _imgWid - new_w);
	y1 = std::min(y1, _imgLen - new_h);

	cv::resize(srcImg, tempMat, cv::Size(new_w, new_h));	// resize (<=224, 224) or (224, <=224)
	tempMat.copyTo(ResizeMat(cv::Rect(x1, y1, new_w, new_h)));	// make 224, 224 image with padding

	//cv::imshow("src", srcImg);
	//cv::imshow("resize", tempMat);
	//cv::imshow("224", ResizeMat);
	//cv::waitKey(0);

	ResizeMat.convertTo(floatMat, CV_32FC1);
	floatMat /= 255.f;

	// Memory Copy
	dims[0] = 1;
	dims[1] = 1;
	dims[2] = _imgLen;
	dims[3] = _imgWid;

	cv::Mat dst = cv::Mat(4, &dims[0], CV_32FC1, cv::Scalar(0));
	memcpy(dst.ptr(), floatMat.ptr(), floatMat.total() * sizeof(float));

	return dst;
}
std::shared_ptr<InferResult> InferOcrCmp::Infer(std::vector<cv::Mat> ImgBuffer, int sessionIdx)
{
	if (sessionIdx >= _Session.size())
		return nullptr;

	////// Check Model Load
	if (EmptyModel() || ImgBuffer.size() == 0)
		return nullptr;

	std::vector<cv::Mat> _inMat;
	
	tick tk, tk1, tk2, total_tk;
	total_tk.start();

	////// 초기화
	tk.start();

	std::shared_ptr<InferResBool> Res(new InferResBool());
	//int _imgCnt = ImgBuffer.size();
	init();
	//_imgWid = ImgBuffer[0].cols;
	//_imgLen = ImgBuffer[0].rows;
	//_imgCh = 1;
	_Session[sessionIdx]->init();
	//int stPos = 0, nCnt = 0;
	//int inspCount = (int)ceil((double)_imgCnt / (double)_block);
	//std::vector<int> bufSz(4);
	//bufSz[0] = _imgCnt;
	//bufSz[1] = _imgCh;
	//bufSz[2] = _imgLen;
	//bufSz[3] = _imgWid;
	// Res->bRes = false;

	_init_time = tk.end();

	//cv::Mat output;

	// Input Matrix 생성
	tk1.start();
	_inMat.emplace_back(Preprocess(ImgBuffer[0]));
	_inMat.emplace_back(Preprocess(ImgBuffer[1]));
	_preprocess_time += tk1.end();

	// Inference 수행
	tk2.start();
	Res->bRes = _Session[sessionIdx]->bInference(_inMat);
	_inference_time += tk2.end();

	_total_infer_time += total_tk.end();

	if (_saveDebugImg)
		SaveReport(_AINGImgPath);

	return Res;
}
void InferOcrCmp::SaveReport(CString report_save_path)
{
	// Test 관련 정보 - 파일로 저장
	std::ofstream o_f;
	o_f.open((std::string)(CT2CA)report_save_path + "\\TestReport.csv", std::ios::app);
	o_f << "Path :" << "," << (std::string)(CT2CA)report_save_path << std::endl;
	o_f << "Model" << "," << (std::string)(CT2CA)modelPath[0] << std::endl;
	o_f << "Src Img Size,width," << std::to_string((long double)_imgWid) << ",height," << std::to_string((long double)_imgLen) << std::endl;
	o_f << "Input Img Size,width," << std::to_string((long double)_imgWid + offset_x) << ",height," << std::to_string((long double)_imgLen + offset_y) << std::endl;

	o_f << "Init Time" << "," << std::to_string((long double)_init_time) << std::endl;
	o_f << "Load Time" << "," << std::to_string((long double)_preprocess_time) << std::endl;
	o_f << "Inference Time" << "," << std::to_string((long double)_inference_time) << std::endl;
	o_f << "Post-process Time" << "," << std::to_string((long double)_postprocess_time) << std::endl;
	o_f << "Result Save Time" << "," << std::to_string((long double)_res_save_time) << std::endl;
	o_f << "Total Infer Time" << "," << std::to_string((long double)_total_infer_time) << std::endl << std::endl;

	o_f.close();
}

int PyOCR_InferCore::GetDevice(int session_idx)
{
	return this->Sessions[session_idx]->get_device();
}
bool PyOCR_InferCore::LoadModel(CString ai_file_path, int SelectGPUDevice)
{
	/*
		SelectGPUDevice 
		-1 : CPUMode
		0~ : GPU Device Id
	*/
	bool beResult = false;
	std::vector<int> gpu_id;
	AiFileHeader file_header;

	if (awm::get().ai_get_file_info(ai_file_path, file_header))
	{	
		gpu_id.emplace_back(SelectGPUDevice);
		this->Sessions.resize(file_header.nFileCount);
		beResult = awm::get().ai_load_file(ai_file_path, 0, file_header.nFileCount, &gpu_id[0], &this->Sessions[0]);
		this->set_model_id(ai_file_path);
	}

	return beResult;
}
CString PyOCR_InferCore::get_model_id()
{
	return this->model_id;
}
void PyOCR_InferCore::set_model_id(CString ai_file_path)
{
	AiFileHeader file_header;
	AiInfoHeader *info_header;

	CString sDateTimeInfo;
	awm::get().ai_load_file_raw(ai_file_path, file_header, &info_header);

	CString sDateTime = file_header.sDatetime;
	CString sWork = info_header->general.sWork;
	int nEpoch = info_header->train.nEpoch;
	CString StringEpoch;

	StringEpoch.Format(_T("%03d"), nEpoch);
	sDateTimeInfo.Format(_T("%s%s_%s%s_%s%s_%s%s"),
		sDateTime.Mid(2, 2),
		sWork,
		sDateTime.Mid(5, 2),
		sDateTime.Mid(8, 2),
		sDateTime.Mid(11, 2),
		sDateTime.Mid(14, 2),
		sDateTime.Mid(17, 2),
		StringEpoch
	);

	this->model_id = sDateTimeInfo;
}
bool PyOCR_InferCore::EmptyModel()
{
	if (this->Sessions.size() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void PyOCR_InferCore::Init()
{

}
std::vector<cv::Mat> PyOCR_InferCore::Infer(std::vector<cv::Mat> imgBuffer)
{

	//image shape 1 x W x H 
	int input_image_size = imgBuffer.size();
	std::vector<cv::Mat> infer_result;
	cv::Mat _result;


	int size[] = { input_image_size,1,224,224 };  // N x 1 x 224 x 224
	// 4d matrix size 
	// dst.size.p[N]

	cv::Mat input_4d_image(4, size, CV_8U);

	for (int i = 0; i < input_image_size; i++) {
		memcpy(input_4d_image.ptr<UINT8>(i), imgBuffer[i].ptr(), imgBuffer[i].total() * sizeof(UINT8));
	}


	//image show 4d image to 2d image
	//for (int i = 0; i < 3; i++) {
	//	cv::Mat show_dst(224,224,CV_8U, input_4d_image.ptr<UINT8>(i,0));
	//	cv::imshow("fgdfg", show_dst);
	//	cv::waitKey(0);
	//	cv::destroyAllWindows();
	//}

	/*for (int image_index = 0; image_index < input_image_size; image_index++)
	{
		_result = this->Sessions[0]->Inference(imgBuffer[image_index]);
		infer_result.emplace_back(_result);
	}*/

	_result = this->Sessions[0]->Inference(input_4d_image);

	for (int i = 0; i < input_image_size; i++)
	{
		cv::Mat _res_mat = _result(cv::Rect(cv::Point(0, 0 + 4 * i), cv::Point(2, 4 + 4 * i))).clone();
		infer_result.emplace_back(_res_mat);
	}
	return infer_result;
}