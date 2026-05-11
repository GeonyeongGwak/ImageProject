#pragma once
#include "../IPInsp_Algo.h"

class IPINSP_ALGO;
class CPInsp_AlgoNGBlob : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoNGBlob(void);
	virtual ~CPInsp_AlgoNGBlob(void);
	void MapClear();
	virtual void InitAlgo();
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);
	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	void LoadExceptModel(CString sPath); //Eagle3D Load
	BOOL LoadExtModel(ext::ModelStatus pCtrl, BYTE * Data, BOOL bCreateImgFile); // EXT Load
	BOOL LoadExtModelSetting(CString sPath, void * Model, CArchive & ar, double dResolX, double dResolY);
	typedef struct stNGBlobModel
	{
		CString sName;
		int nWidth;
		int nHeight;
		cv::Mat Img2D[4];
		double dResolX;
		double dResolY;
		BOOL bUseMatching;
		int nStx;
		int nSty;
		int nAreaWidth;
		int nAreaHeight;
		GeoModel geoMaster;
		GeoModel geoMaster_90;
		GeoModel geoMaster_180;
		GeoModel geoMaster_270;
		stNGBlobModel()
		{
		}
		void ModelClear()
		{
			for (int i = 0; i < 4; i++)
			{
				if (Img2D[i].data != NULL)
					Img2D[i].release();
			}
		}
		cv::Mat GetRotateimg(float dAngle = 0)
		{
			if ((int)dAngle % 90 != 0) dAngle = 0.0;
			if (dAngle == 0.0)
				return Img2D[0].clone();
			else if (dAngle == 90.0)
				return Img2D[1].clone();
			else if (dAngle == 180.0)
				return Img2D[2].clone();
			else if (dAngle == 270.0)
				return Img2D[3].clone();
		}
	}NGBlobModel;
	std::map<CString, NGBlobModel *> m_MapModelExc;
	NGBlobModel m_stModel;
};

