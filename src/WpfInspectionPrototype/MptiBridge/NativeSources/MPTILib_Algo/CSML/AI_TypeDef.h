#pragma once
#include "opencv2/opencv.hpp"

namespace Toolset
{
#define tlst_csml		_T("CSML")
#define tlst_pytorch	_T("Pytorch")
#define tlst_tensorrt	_T("TensorRT")
#define tlst_tensorrt_trt	_T("TensorRT_trt")

	typedef enum _TS_TYPE { vCSML, vPytorch, vTensorRT, vTensorRT_trt, numTSType }TS_TYPE;
}
namespace Identity
{
#define typ_classification	_T("Classification")
#define typ_pix2pix			_T("Pix2Pix")
#define typ_detection		_T("Detection")
#define type_Craft			_T("Craft")
#define type_clsTorch		_T("clsTorch")
	typedef enum _ID_TYPE { vClassification, vPix2Pix, vDetection, vCraft, vClsTorch ,numIDType }ID_TYPE;
}

typedef enum _DB_TYPE { vBubb, vCad, vOCR, vCnt, vFor, vXRay, vXRay_5CH, numDBType }DB_TYPE;

typedef struct _stToolsetType
{
	//CString ToolsetStr[Toolset::TS_TYPE::num] = { _T("CSML") , _T("Pytorch") , _T("TensorRT") };

	_stToolsetType()
	{
		ToolsetStr[Toolset::TS_TYPE::vCSML] = _T("CSML");
		ToolsetStr[Toolset::TS_TYPE::vPytorch] = _T("Pytorch");
		ToolsetStr[Toolset::TS_TYPE::vTensorRT] = _T("TensorRT");
		ToolsetStr[Toolset::TS_TYPE::vTensorRT_trt] = _T("TensorRT_trt");
	}

	CString ToolsetStr[Toolset::TS_TYPE::numTSType];

}stToolsetType;

typedef struct _stWorkType
{
	//CString WorkStr[DB_TYPE::num] = { _T("Bubble") , _T("CadGerber") , _T("OCR") , _T("PartsCnter") , _T("ForeignCls")};

	_stWorkType()
	{
		WorkStr[DB_TYPE::vBubb] = _T("Bubble");
		WorkStr[DB_TYPE::vCad] = _T("CadGerber");
		WorkStr[DB_TYPE::vOCR] = _T("OCR");
		WorkStr[DB_TYPE::vCnt] = _T("PartsCnter");
		WorkStr[DB_TYPE::vFor] = _T("ForeignCls");
		WorkStr[DB_TYPE::vXRay] = _T("XRay");
		WorkStr[DB_TYPE::vXRay_5CH] = _T("XRay_5CH");

	}

	CString WorkStr[DB_TYPE::numDBType];

}stWorkType;

typedef struct _stIdentityType
{
	//CString IdentityStr[Identity::ID_TYPE::num] = { _T("Classification") , _T("Pix2Pix") , _T("Detection"), _T("Craft") };

	_stIdentityType()
	{
		IdentityStr[Identity::ID_TYPE::vClassification] = _T("Classification");
		IdentityStr[Identity::ID_TYPE::vPix2Pix] = _T("Pix2Pix");
		IdentityStr[Identity::ID_TYPE::vDetection] = _T("Detection");
		IdentityStr[Identity::ID_TYPE::vCraft] = _T("Craft");
		IdentityStr[Identity::ID_TYPE::vClsTorch] = _T("ClsTorch");
	}

	CString IdentityStr[Identity::ID_TYPE::numIDType];

}stIdentityType;

class AiFileHeader
{
public:
	CString sFileVer;
	CString sDatetime;
	size_t nFileCount;
	CString sModelInfo;
public:
	CString Work(int ew)
	{
		stWorkType wt;
		return wt.WorkStr[ew];
	}
	DB_TYPE Work(CString etStr)
	{
		stWorkType tt;

		for (int i = 0; i < DB_TYPE::numDBType; i++)
		{
			if (tt.WorkStr[i] == etStr)
				return DB_TYPE(i);
		}
		return DB_TYPE::numDBType;
	}

	CString Identity(int ew)
	{
		stIdentityType wt;
		return wt.IdentityStr[ew];
	}
	Identity::ID_TYPE Identity(CString etStr)
	{
		stIdentityType ti;

		for (int i = 0; i < Identity::ID_TYPE::numIDType; i++)
		{
			if (ti.IdentityStr[i] == etStr)
				return Identity::ID_TYPE(i);
		}
		return Identity::ID_TYPE::numIDType;
	}

	AiFileHeader()
	{
		nFileCount = 0;
	}
	AiFileHeader(size_t nInfoCount)
	{
		Make(nInfoCount);
	}
	void Make(size_t nInfoCount)
	{
		CTime tm = CTime::GetCurrentTime();
		sFileVer = _T("1.0.1");
		sDatetime = tm.Format(_T("%Y-%m-%d %H:%M:%S"));
		nFileCount = nInfoCount;
	}
};

class AiInfoGeneral
{
public:
	CString sWork;
	CString sType;
	CString sDataset;
	CString sAuthor;
	CString sModelPath;
	CString sGPUType;	// .trt Model file
	unsigned __int64 nSizeBytes;
	unsigned __int64 nInputSizeN;	// .trt Model file
	unsigned __int64 nInputSizeC;	// .trt Model file
	unsigned __int64 nInputSizeH;	// .trt Model file
	unsigned __int64 nInputSizeW;	// .trt Model file

public:
	AiInfoGeneral()
	{
		nSizeBytes = 0;
		nInputSizeN = 0;
		nInputSizeC = 0;
		nInputSizeH = 0;
		nInputSizeW = 0;
	}
	AiInfoGeneral(unsigned __int64 szBytes)
	{
		nSizeBytes = szBytes;
		nInputSizeN = 0;
		nInputSizeC = 0;
		nInputSizeH = 0;
		nInputSizeW = 0;
	}
};

class AiInfoTrain
{
public:
	int nEpoch;
	int nEpochTotal;
	float fLearnRate;
	float fDecay;
	CString sDecayMethod;
	CString sOptimizer;
	CString sDescription;
public:
	AiInfoTrain()
	{
		nEpoch = 0;
		nEpochTotal = 0;
		fLearnRate = 0;
		fDecay = 0;
	}
};

class AiInfoToolset
{
public:
	CString Toolset(int et)
	{
		stToolsetType tt;
		return tt.ToolsetStr[et];
	}
	Toolset::TS_TYPE Toolset(CString etStr)
	{
		stToolsetType tt;

		for (int i = 0; i < Toolset::TS_TYPE::numTSType; i++)
		{
			if (tt.ToolsetStr[i] == etStr)
				return Toolset::TS_TYPE(i);
		}
		return Toolset::TS_TYPE::numTSType;
	}
	CString sFramework;
	CString sVersion;
};

class AiInfoHeader
{
public:
	AiInfoGeneral general;
	AiInfoTrain train;
	AiInfoToolset toolset;
	BYTE * data;
	void SetBuffer(unsigned __int64 sz)
	{
		Release();
		data = new BYTE[sz];
	}
	void Release()
	{
		if (data)
		{
			delete[] data;

			data = nullptr;
		}
	}

	const AiInfoHeader & operator =(const AiInfoHeader & rhs)
	{
		general = rhs.general;
		train = rhs.train;
		toolset = rhs.toolset;
		if (general.nSizeBytes > 0)
		{
			SetBuffer(general.nSizeBytes);
			memcpy(data, rhs.data, general.nSizeBytes);
		}
		return *this;
	}
public:
	AiInfoHeader()
		:data(nullptr)
	{

	}
	~AiInfoHeader()
	{
		Release();
	}
};

class InferInit
{
public:
	InferInit()
	{
		saveDebugImg = false;
		AINGImgPath = _T("D:\\AITest");

		block = 100;
		imgWid = 112;
		imgLen = 112;
		imgCh = 1;

		init_time = 0;
		preprocess_time = 0;
		inference_time = 0;
		postprocess_time = 0;
		res_save_time = 0;
		total_infer_time = 0;
	}

public:
	bool saveDebugImg; // 디버그 이미지 저장 여부

	CString AINGImgPath; // 디버그 이미지 저장 경로

	int block;
	int imgWid;
	int imgLen;
	int imgCh;

	double init_time;
	double preprocess_time;
	double inference_time;
	double postprocess_time;
	double res_save_time;
	double total_infer_time;
};

class InferInitCore : public InferInit
{
public:
	InferInitCore()
	{
		bIndependent = false;
		blurFilterSz = 61;
		block = 100;
		imgWid = 256;
		imgLen = 256;
		imgCh = 1;
	}

public:
	bool bIndependent;
	int blurFilterSz;
};


class InferInitBubble : public InferInit
{
public:
	InferInitBubble()
	{
		binary_threshold = 200;
		block = 1;
		imgWid = 256;
		imgLen = 256;
		imgCh = 1;
	}

public:
	int binary_threshold;
	int block;
};

class InferInitPartsCnter : public InferInit
{
public:
	InferInitPartsCnter()
	{
		bAutoNorm = true;
		bSingleMode = true;
		preprocessThreshold = 14000;
		block = 1;
		imgWid = 512;
		imgLen = 512;
		imgCh = 1;
	}

public:
	bool bAutoNorm;
	bool bSingleMode;
	int preprocessThreshold;
	int block;
};

class InferResult
{
public:
	cv::Mat netRes;
};

class InferResVec : public InferResult
{
public:
	std::vector<cv::Mat> netResBuf;
};

class InferResBool : public InferResult
{
public:
	bool bRes;
};