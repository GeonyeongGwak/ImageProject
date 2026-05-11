#pragma once
#include "../IPInsp_Algo.h"

//★★ 코드 작성 필요
class IPINSP_ALGO;
class CPInsp_AlgoPatternDiff : public IPINSP_ALGO
{
private:

public:
	CPInsp_AlgoPatternDiff(void);
	virtual ~CPInsp_AlgoPatternDiff(void);
	virtual void InitAlgo();
	
	virtual unsigned long long GetInspAlgoData();
	virtual int AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect);
	virtual bool InspWindowArea(int nType = m_eInspWindowArea_Type_None);
	virtual BOOL InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg = nullptr);

	virtual BOOL InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf = nullptr);
	virtual BOOL SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle = true);
	virtual int UseColorImage(const InspAlgo &sInspAlgo, int nIndex = -1);
	virtual bool SetAIImage(InspPartInfo *pInspBoardInfo, int inspType);
	typedef struct stPatternDiffModels 
	{
		QWORD dwPDI;
		QWORD dwPPD;

	}PatternDiffModels;
	typedef struct stMultiAlignArea
	{
		int stX;
		int stY;
		int nAreaWidth;
		int nAreaLength;
		GeoModel geoMaster;
		stMultiAlignArea()
		{
		}
	}MultiAlignArea;
	typedef struct stPatternDiffModel
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
				
		BOOL bUseMultiAlign;
		int nMultiAlignCnt;
		MultiAlignArea stAlignArea[8];
		stPatternDiffModel()
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

	}PatternDiffModel;
	std::map<CString, PatternDiffModel *> m_MapModel;
	std::map<CString, PatternDiffModel *> m_MapModelExc;
	PatternDiffModel m_stModel;
	PatternDiffModels  m_stModels;
	std::map<CString, cv::Mat> m_vLayerMask[2];
	//Last OK Model Index for Auto Insp Seq
	std::map<CString, int>  m_vArrModelLastOKIdx[2];

	void ClearAutoInspParam(int nCurLane);
	void AddLayerMask(CString sPath, cv::Mat& Mask, int nCurLane);
	BOOL GetLayerMask(CString sPath, cv::Mat& Mask, double dAngle = 0);
	void MapClear();
	BOOL SaveModel(cv::Mat ModelImg, CString sPath, RECT rcMatchingArea = { 0,0,0,0 }, int nMultiAlignCnt = 0, RECT* rtArrAlignArea = nullptr);
	BOOL SaveModelFile(CArchive& ar, cv::Mat ModelImg, RECT rcMatchingArea, int nMultiAlignCnt = 0, RECT* rtArrAlignArea = nullptr);
	BOOL LoadModel(CString sPath, BOOL bCreateImgFile, BOOL bAutoInspLoad, BOOL bUseMultiAlign = FALSE);
	BOOL LoadExtModel(ext::ModelStatus pCtrl, BYTE * Data, BOOL bCreateImgFile);
	BOOL LoadModelFile(CArchive& ar, double dResolX, double dResolY);
	BOOL LoadExtModelSetting(CString sPath , void * Model, CArchive& ar, double dResolX, double dResolY);
	void ReSizeModel(double dResolX, double dResolY);
	BOOL LoadMatchInfo(CArchive& ar, double m_resolX, double m_resolY);
	void SetMatchingArea(CString sPath, RECT rcMatchingArea, int nMultiAlignCnt = 0, RECT* rtArrAlignArea = nullptr);

	short LabelValue(int nLabelIdx);

	POINTF SelectOffset(int nTargetAlignID, std::vector<PIAL::_AlignResult> vecAlignResult, double& dAlignTheta, POINTF& ptMatchingCenter);
	void SetLayerMask(const InspAlgo& InspAlgo, cv::Mat& DstMask, int nWidth, int nHeight);
	BOOL IsPointInPolygon(POINT p, POINTF* PointList, int size, int nImgWidth, int nImgHeight, int dX, int dY);
	BOOL MatchNewModel(cv::Mat& Srcimg, CString sRstModelPath, RECT rcMatchingArea, POINT& ptRst);

	//For AutoRun OK Model index
	void ClearModelIdx(int nLane);
	void SetOKModelIdx(CString sModelKey, int nSuccessIdx);
	int GetLastOKIdx(CString sCurKey);
	void LoadExceptModel(CString sPathpCtrl, BOOL bAutoInspSeq = FALSE);
	bool CheckModelList(CString sPath, int nLaneNum);
	bool IsMainProcessCheck();
};