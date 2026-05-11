#include "stdafx.h"
#include "InferCore.h"
#include "awm.h"

void ProcessWindowMessage()
{
	MSG msg;
	while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		::SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}
}

int InferCore::GPUCount()
{
	int cnt = -1;
	if (_Session.size() != 0)
		cnt = _Session[0]->get_gpu_count();

	return cnt;
}
bool InferCore::LoadModel(CString path, std::function<int(int)> selDev)
{
	bool bRes = false;
	if (path.Right(4) != _T(".ai1"))
		return bRes;

	std::vector<int> gpuId;
	AiFileHeader fh;
	if (awm::get().ai_get_file_info(path, fh) == true)
	{
		gpuId.resize(fh.nFileCount);
		for (int i = 0; i < fh.nFileCount; i++) {
			if (selDev)
				gpuId[i] = selDev(i);
			else
				gpuId[i] = -1;
		}

		modelPath.clear();
		modelPath.emplace_back(path);
		
		_Session.resize(fh.nFileCount);
		isession * pSss;
		bRes = awm::get().ai_load_file(path, 0, fh.nFileCount, &gpuId[0], &pSss);
		_Session[0] = std::shared_ptr<isession>(pSss);

#ifdef _AITester
		assert(bRes == true);
#endif

		if (bRes)
		{
			// Model ID
			CString strID;
			modelIDBuf.resize(fh.nFileCount);
			for (int n = 0; n < fh.nFileCount; n++)
			{
				strID.Format(_T("%s%s%s%s%s%s_%s_%s"), fh.sDatetime.Mid(2, 2), fh.sDatetime.Mid(5, 2), fh.sDatetime.Mid(8, 2), fh.sDatetime.Mid(11, 2), fh.sDatetime.Mid(14, 2), fh.sDatetime.Mid(17, 2), _Session[n]->_cstDB, _Session[n]->_cstID);
				modelIDBuf[n] = strID;
			}
		}
	}
	else
	{

	}

	return bRes;
}
bool InferCore::LoadModelSimple(CString path, int nGpuidx)
{
	bool bRes = false;
	std::vector<int> gpuId;
	gpuId.emplace_back(nGpuidx);
	AiFileHeader fh;
	if (awm::get().ai_get_file_info(path, fh) == true)
	{
		gpuId.resize(fh.nFileCount);
		for (int i = 0; i < fh.nFileCount; i++) {
			gpuId[i] = nGpuidx;
		}

		modelPath.clear();
		modelPath.emplace_back(path);

		_Session.resize(fh.nFileCount);
		isession * pSss;
		bRes = awm::get().ai_load_file(path, 0, fh.nFileCount, &gpuId[0], &pSss);
		_Session[0] = std::shared_ptr<isession>(pSss);

		// Model ID
		CString strID;
		modelIDBuf.resize(fh.nFileCount);
		for (int n = 0; n < fh.nFileCount; n++)
		{
			strID.Format(_T("%s_%s_%s"), fh.sDatetime, _Session[n]->_cstDB, _Session[n]->_cstID);
			modelIDBuf[n] = strID;
		}
	}
	else
	{

	}

	return bRes;
}
bool InferCore::EmptyModel()
{
	if (_Session.size() == 0)
		return true;
	else
		return false;
}
bool InferCore::EmptyModel(int netID)
{
	return EmptyModel();
}
CString InferCore::ModelID(int idx)
{
	CString strID;
	if (modelIDBuf.size() > idx)
	{
		strID = modelIDBuf[idx];
	}

	return strID;
}

void InferCore::init()
{
	if (init_prop == nullptr)
		init_prop = std::shared_ptr<InferInit>(new InferInitCore());

	init(init_prop);
}
void InferCore::init(std::shared_ptr<InferInit> ptr)
{
	InferBase::init(ptr);

	std::shared_ptr<InferInitCore> prop = std::static_pointer_cast<InferInitCore, InferInit>(ptr);
	_bIndependent = prop->bIndependent;
	_blurFilterSz = prop->blurFilterSz;	

	szImg.resize(4);
	_srcInfo.resize(4);
}
void InferCore::UpdateDevice(int gpuIdx, int sessionIdx)
{
	if (sessionIdx < 0)
	{
		for (int n = 0; n < _Session.size(); n++)
			_Session[n]->set_device(gpuIdx);
	}
	else
		_Session[sessionIdx]->set_device(gpuIdx);
}
std::vector<int> InferCore::GetDevice()
{
	std::vector<int> nGpuId;
	for (int n = 0; n < _Session.size(); n++)
	{
		int gpuId = _Session[n]->get_device();
		nGpuId.emplace_back(gpuId);
	}

	return nGpuId;
}
int InferCore::GetDevice(int sessionIdx)
{
	int gpuId = _Session[sessionIdx]->get_device();
	return gpuId;
}
std::shared_ptr<InferResult> InferCore::Infer(std::vector<cv::Mat> imgBuffer, int sessionIdx)
{
	if (sessionIdx >= _Session.size())
		return nullptr;

	std::shared_ptr<InferResult> Res(new InferResult());
	cv::Mat ResMat, tmpMat, tmp2;
	std::vector<cv::Mat> vBatchMatBuff;
	int realBatch = _block;
	int ndim(0), nSeq(0);
	int nStride = _bIndependent ? _imgCh : 1;
	int nImgAmount = (int)imgBuffer.size() / nStride;
	std::vector<int> ranges;
	_Session[sessionIdx]->init();

	//배치보다 작으면 그대로 진행
	if (nImgAmount <= _block)
	{
		MakeBatchFromVector(imgBuffer, _bIndependent);
		Res->netRes = _Session[sessionIdx]->Inference(_inDataMat);
		return Res;
	}
	//배치보다 커지면 나누어 진행
	else
	{
		int nAmounts = (int)ceil((double)nImgAmount / (double)_block);
		CString cstMsg;
		for (int rep = 0; rep < nAmounts; rep++)
		{
			vBatchMatBuff.clear();
			realBatch = (rep + 1)*_block > nImgAmount ? nImgAmount - rep * _block : _block;
			vBatchMatBuff.resize(realBatch * nStride);
			std::copy(imgBuffer.begin() + (rep * _block) * nStride, imgBuffer.begin() + (rep * _block + realBatch) * nStride, vBatchMatBuff.begin());
			MakeBatchFromVector(vBatchMatBuff, _bIndependent);
			tmpMat = _Session[sessionIdx]->Inference(_inDataMat);
			ndim = tmpMat.dims;

			///Alloc
			if (ResMat.empty())
			{
				ranges.emplace_back(nImgAmount);
				for (int iters = 1; iters < ndim; iters++)
					ranges.emplace_back(tmpMat.size[iters]);
				ResMat = cv::Mat(ndim, &ranges[0], tmpMat.type());
			}

			///Copy
			std::vector<cv::Range> srcRange(ndim);
			srcRange[0] = cv::Range(rep*_block, rep*_block + realBatch);
			for (int d = 1; d < srcRange.size(); d++)
				srcRange[d] = cv::Range::all();
			cv::Mat Line(ResMat, &srcRange[0]);
			memcpy(Line.ptr(), tmpMat.ptr(), tmpMat.total() * sizeof(float));
#ifdef _AITester
			cstMsg.Format(_T("Infer. Rep %d/%d"), rep + 1, nAmounts);
			_CstStatusMsg->Val = cstMsg;
#endif
			ProcessWindowMessage();
		}
		Res->netRes = ResMat;
		return Res;
	}
}
std::vector<std::vector<cv::Mat>> InferCore::vInfer(std::vector<cv::Mat> imgBuffer, bool bIndependent, int sessionIdx)
{
	std::vector<std::vector<cv::Mat>> ResMat;

	if (sessionIdx >= _Session.size())
		return ResMat;
	std::vector<cv::Mat> vBatchMatBuff;
	int realBatch = _block;
	int ndim(0), nSeq(0);
	int nStride = bIndependent ? _imgCh : 1;
	int nImgAmount = (int)imgBuffer.size() / nStride;
	std::vector<int> ranges;
	_Session[sessionIdx]->init();

	//배치보다 작으면 그대로 진행
	if (nImgAmount <= _block)
	{
		MakeBatchFromVector(imgBuffer, bIndependent);
		ResMat.emplace_back(_Session[sessionIdx]->vInference(_inDataMat));
		return ResMat;
	}
	//배치보다 커지면 나누어 진행
	else
	{
		for (int rep = 0; rep < (int)ceil((double)nImgAmount / (double)_block); rep++)
		{
			vBatchMatBuff.clear();
			realBatch = (rep + 1)*_block > nImgAmount ? nImgAmount - rep * _block : _block;
			vBatchMatBuff.resize(realBatch * nStride);
			std::copy(imgBuffer.begin() + (rep * _block) * nStride, imgBuffer.begin() + (rep * _block + realBatch) * nStride, vBatchMatBuff.begin());
			MakeBatchFromVector(vBatchMatBuff, bIndependent);
			ResMat.emplace_back(_Session[sessionIdx]->Inference(_inDataMat));
		}
		return ResMat;
	}
}


#ifdef _AITester
std::shared_ptr<InferResult> InferCore::Infer(std::vector<CString> cstPaths, int sessionIdx)
{
	if (sessionIdx >= _Session.size())
		return nullptr;

	std::shared_ptr<InferResult> Res(new InferResult());
	cv::Mat ResMat, tmpMat, tmp2;
	std::vector<CString> vBatchPaths;
	int realBatch = _block;
	int ndim(0), nSeq(0);
	int nStride = _bIndependent ? _imgCh : 1;
	int nImgAmount = (int)cstPaths.size() / nStride;
	std::vector<int> ranges;
	_Session[sessionIdx]->init();

	//배치보다 작으면 그대로 진행
	if (nImgAmount <= _block)
	{
		MakeBatchFromVectorPaths(cstPaths, _bIndependent);
		Res->netRes = _Session[sessionIdx]->Inference(_inDataMat);
		return Res;
	}
	//배치보다 커지면 나누어 진행
	else
	{
		int nAmounts = (int)ceil((double)nImgAmount / (double)_block);
		CString cstMsg;
		for (int rep = 0; rep < nAmounts; rep++)
		{
			vBatchPaths.clear();
			realBatch = (rep + 1)*_block > nImgAmount ? nImgAmount - rep * _block : _block;
			vBatchPaths.resize(realBatch * nStride);
			std::copy(cstPaths.begin() + (rep * _block) * nStride, cstPaths.begin() + (rep * _block + realBatch) * nStride, vBatchPaths.begin());
			MakeBatchFromVectorPaths(vBatchPaths, _bIndependent);
			tmpMat = _Session[sessionIdx]->Inference(_inDataMat);
			ndim = tmpMat.dims;

			///Alloc
			if (ResMat.empty())
			{
				ranges.emplace_back(nImgAmount);
				for (int iters = 1; iters < ndim; iters++)
					ranges.emplace_back(tmpMat.size[iters]);
				ResMat = cv::Mat(ndim, &ranges[0], tmpMat.type());
			}

			///Copy
			std::vector<cv::Range> srcRange(ndim);
			srcRange[0] = cv::Range(rep*_block, rep*_block + realBatch);
			for (int d = 1; d < srcRange.size(); d++)
				srcRange[d] = cv::Range::all();
			cv::Mat Line(ResMat, &srcRange[0]);
			memcpy(Line.ptr(), tmpMat.ptr(), tmpMat.total() * sizeof(float));
#ifdef _AITester
			cstMsg.Format(_T("Infer. Rep %d/%d"), rep + 1, nAmounts);
			_CstStatusMsg->Val = cstMsg;
#endif
			ProcessWindowMessage();
		}
		Res->netRes = ResMat;
		return Res;
	}
}
std::vector<std::vector<cv::Mat>> InferCore::vInfer(std::vector<CString> cstPaths, bool bIndependent, int sessionIdx)
{
	std::vector<std::vector<cv::Mat>> ResMat;

	if (sessionIdx >= _Session.size())
		return ResMat;
	std::vector<CString> vBatchPaths;
	int realBatch = _block;
	int ndim(0), nSeq(0);
	int nStride = bIndependent ? _imgCh : 1;
	int nImgAmount = (int)cstPaths.size() / nStride;
	std::vector<int> ranges;
	_Session[sessionIdx]->init();

	//배치보다 작으면 그대로 진행
	if (nImgAmount <= _block)
	{
		MakeBatchFromVectorPaths(cstPaths, bIndependent);
		ResMat.emplace_back(_Session[sessionIdx]->vInference(_inDataMat));
		return ResMat;
	}
	//배치보다 커지면 나누어 진행
	else
	{
		for (int rep = 0; rep < (int)ceil((double)nImgAmount / (double)_block); rep++)
		{
			vBatchPaths.clear();
			realBatch = (rep + 1)*_block > nImgAmount ? nImgAmount - rep * _block : _block;
			vBatchPaths.resize(realBatch * nStride);
			std::copy(cstPaths.begin() + (rep * _block) * nStride, cstPaths.begin() + (rep * _block + realBatch) * nStride, vBatchPaths.begin());
			MakeBatchFromVectorPaths(vBatchPaths, bIndependent);
			ResMat.emplace_back(_Session[sessionIdx]->Inference(_inDataMat));
		}
		return ResMat;
	}
}
void InferCore::MakeBatchFromVectorPaths(std::vector<CString> vInputList, bool bInputChannelsSplitted)
{
	std::vector<cv::Mat> vInMat;
	for (int iters = 0; iters < vInputList.size(); iters++)
	{
		vInMat.emplace_back(cv::imread(std::string(CT2A(vInputList[iters])), cv::IMREAD_UNCHANGED));
	}
	//vInMat 벡터 이미지에 맞춰서 똑같은 크기 구성으로
	//AI용 4D 입력 이미지를 만든다.
	//int _imgWid, _imgLen, _imgCh;
	int nImgs = bInputChannelsSplitted ? (int)vInputList.size() / _imgCh : (int)vInputList.size();
	if (_inDataMat.empty() || _srcInfo[0] != nImgs || _srcInfo[1] != _imgCh || _srcInfo[2] != _imgLen || _srcInfo[3] != _imgWid)
	{
		_srcInfo[0] = nImgs;
		_srcInfo[1] = _imgCh;
		_srcInfo[2] = _imgLen;
		_srcInfo[3] = _imgWid;
		_inDataMat = cv::Mat(4, &_srcInfo[0], CV_32FC1, cv::Scalar::all(0));
	}
	cv::Mat colorImg, floatMat;
	std::vector<cv::Mat> chImg;
	chImg.resize(_imgCh);
	std::vector<cv::Range> srcRange(_inDataMat.dims);
	for (int j = 0; j < nImgs; j++)
	{
		if (bInputChannelsSplitted)
		{
			for (int nIdx = 0; nIdx < _imgCh; nIdx++)
				chImg[nIdx] = vInMat[j*_imgCh + nIdx];
		}
		else
		{
			cv::split(vInMat[j], &chImg[0]);
		}
		srcRange[0] = cv::Range(j, j + 1);
		for (int d = 2; d < srcRange.size(); d++)
			srcRange[d] = cv::Range::all();
		for (int iters = 0; iters < _imgCh; iters++)
		{
			chImg[iters].convertTo(floatMat, CV_32FC1);
			//Preprocess(floatMat);
			srcRange[1] = cv::Range(iters, iters + 1);// _imgCh - iters - 1, _imgCh - iters);

			cv::Mat Line(_inDataMat, &srcRange[0]);
			memcpy(Line.ptr(), floatMat.ptr(), floatMat.total() * sizeof(float));
		}
	}
}
#endif


cv::Mat InferCore::RestoreImg(cv::Mat res, int n, int nCh, bool bDenormalizeAuto)
{
	cv::Mat partImg, dataTemp;
	std::vector<cv::Range> srcRange1(4);
	double dMin(-1), dMax(1);
	srcRange1[0] = cv::Range(n, n + 1);
	srcRange1[1] = cv::Range(nCh, nCh + 1);
	srcRange1[2] = cv::Range::all();
	srcRange1[3] = cv::Range::all();
	partImg = cv::Mat(res, &srcRange1[0]);
	dataTemp = cv::Mat(res.size[2], res.size[3], CV_32FC1, partImg.ptr<float>());
	if (bDenormalizeAuto)
		cv::minMaxLoc(dataTemp, &dMin, &dMax);
	dataTemp -= dMin;
	dataTemp /= (dMax - dMin + 0.0000001);
	dataTemp *= 255.0;
	dataTemp.convertTo(partImg, CV_8UC1);
	return partImg;
}
void InferCore::Preprocess(cv::Mat inFloat)
{
	float norm_m(0.5f), norm_std(0.5f);
	cv::Scalar sMean = cv::mean(inFloat);
	double dMinInner, dMaxInner;
	cv::minMaxLoc(inFloat, &dMinInner, &dMaxInner);
	if (sMean[0] != 0.f && sMean[0] != 0.f)
	{
		inFloat -= dMinInner;
		inFloat /= (dMaxInner - dMinInner);
	}
	cv::Scalar outMean, outStd;

	inFloat -= norm_m;
	inFloat /= norm_std;

}
void InferCore::MakeBatchFromVector(std::vector<cv::Mat> vInMat, bool bInputChannelsSplitted)
{
	//vInMat 벡터 이미지에 맞춰서 똑같은 크기 구성으로
	//AI용 4D 입력 이미지를 만든다.
	//int _imgWid, _imgLen, _imgCh;
	int nImgs = bInputChannelsSplitted ? (int)vInMat.size() / _imgCh : (int)vInMat.size();
	if (_inDataMat.empty() || _srcInfo[0] != nImgs || _srcInfo[1] != _imgCh || _srcInfo[2] != _imgLen || _srcInfo[3] != _imgWid)
	{
		_srcInfo[0] = nImgs;
		_srcInfo[1] = _imgCh;
		_srcInfo[2] = _imgLen;
		_srcInfo[3] = _imgWid;
		_inDataMat = cv::Mat(4, &_srcInfo[0], CV_32FC1, cv::Scalar::all(0));
	}
	cv::Mat colorImg, floatMat;
	std::vector<cv::Mat> chImg;
	chImg.resize(_imgCh);
	std::vector<cv::Range> srcRange(_inDataMat.dims);
	for (int j = 0; j < nImgs; j++)
	{
		if (bInputChannelsSplitted)
		{
			for (int nIdx = 0; nIdx < _imgCh; nIdx++)
				chImg[nIdx] = vInMat[j*_imgCh + nIdx];
		}
		else
		{
			cv::split(vInMat[j], &chImg[0]);
		}
		srcRange[0] = cv::Range(j, j + 1);
		for (int d = 2; d < srcRange.size(); d++)
			srcRange[d] = cv::Range::all();
		for (int iters = 0; iters < _imgCh; iters++)
		{
			chImg[iters].convertTo(floatMat, CV_32FC1);
			//Preprocess(floatMat);
			srcRange[1] = cv::Range(iters, iters + 1);// _imgCh - iters - 1, _imgCh - iters);

			cv::Mat Line(_inDataMat, &srcRange[0]);
			memcpy(Line.ptr(), floatMat.ptr(), floatMat.total() * sizeof(float));
		}
	}
}


CString InferCore::GetCurrentDebugImgPath()
{
	return _AINGImgPath;
}
