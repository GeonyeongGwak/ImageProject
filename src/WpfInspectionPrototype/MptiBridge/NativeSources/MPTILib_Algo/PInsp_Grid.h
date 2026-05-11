#pragma once

#include "PInsp_Color.h"
#include "ProcMil_Color.h"

// YJS 2016/11/04 : 생성
class CPInsp_Grid : public CPInsp_Color
{
public:
	class BufferMng
	{
	public:
		void Init(int& Wid, int& Len)
		{
			m_vImg_Stdev.clear();
			m_vImg_CntStdev.clear();
			m_vImg_Avg.clear();
			m_vImg_CntAvg.clear();
			m_vImg_Res.clear();
			m_vMaskBuf.clear();

			if(Wid <= 0)
				Wid = 1;
			if(Len <= 0)
				Len = 1;

			m_vImg_Stdev.resize(Wid * Len);
			m_vImg_CntStdev.resize(Wid * Len);
			m_vImg_Avg.resize(Wid * Len);
			m_vImg_CntAvg.resize(Wid * Len);
			m_vImg_Res.resize(Wid * Len);
			m_vMaskBuf.resize(Wid * Len);
		}

		std::vector<float> m_vImg_Stdev;
		std::vector<int> m_vImg_CntStdev;

		std::vector<float> m_vImg_Avg;
		std::vector<int> m_vImg_CntAvg;

		std::vector<UCHAR> m_vImg_Res;
		std::vector<UCHAR> m_vMaskBuf;

		BOOL bResultOK;
		double dArea;
		double dAreaPer;
	};
public:
	CPInsp_Grid(void);
	virtual ~CPInsp_Grid(void);

public:
	virtual int InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib);
	virtual int SetInspParam(const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, 
						const AlgoCoordinate &coordinateAlgo, const InspPartInfo *pInspBoardInfo);
	virtual int SetInspParam( const InspAlgo &sInspAlgo, const WndAlgoImg &sWndAlgoImg, const AlgoCoordinate &coordinateAlgo, 
							const WndInfo &wndInfoAnyAngle, const InspPartInfo *pInspBoardInfo, bool bIsLoadFovRawDataImage, int projectionmode );

	void CalcInspParam(double * stdev, double * stdevThreshold, bool bIsLoadFovRawDataImage);
	virtual int ProcAuto();
	virtual int ProcStep(int stepID);

	virtual BOOL GetInspRst( RstAlgoGrid *inspAlgoResult );

	void GetBlobImage(std::vector<UCHAR> & vBin);
	void GetMaskImage(std::vector<UCHAR> & vMask);

	void GetWindowSize(int & wid, int & len)
	{
		wid = GetWidthImg(m_inspCoordinate);
		len = GetHeightImg(m_inspCoordinate);
	}

	UCHAR * GetWindowImg(InspImg_Kind kind)
	{
		return &m_img_Mix[kind][0];
	}

	void SetwndAlgoImg(const WndAlgoImg &sWndAlgoImg)
	{
		m_wndAlgoImg = sWndAlgoImg;
	}

protected:
	void MakeMask3d();
	void IgnoreSelectedArea(bool bIsLoadFovRawDataImage = false);
	void CalcAllCell();
	void BlobAnalysis();
	void Decision();
	
	void _CalcParam(double * stdev, double * threshold);
	void _MakeProcImg(UCHAR * img_R, UCHAR * img_G, UCHAR * img_B, UCHAR * imgBR = NULL, UCHAR * imgBB = NULL, bool bIsLoadFovRawDataImage = false);
	void _MakeColorBuf_Mix_Teach(bool bIsLoadFovRawDataImage);
	void _CheckPartAngle();
	int m_nColumn;
	int m_nRow;

	// 검사 파라메터
	AlgoGrid* m_pAlgoGrid;
	BufferMng m_Buffer;
	std::vector<UCHAR> m_img_Mix[InspImg_Kind::eImg_BufCnt];
protected:
	void _CalcCIE(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, float factorR, float factorG, float factorB, 
				int width, int length, POINT * pts, int ptNum, UCHAR * mask);
	void _CalcCIE__CompoBtm(UCHAR* srcR, UCHAR* srcG, UCHAR* srcB, UCHAR* srcBR, UCHAR* srcBB, float factorR, float factorG, float factorB, float factorBR, float factorBB,
				float compoBtmR, float compoBtmG, float compoBtmB, int width, int length, POINT * pts, int ptNum, UCHAR * mask);
	inline bool _Color_GetCxy(float r, float g, float b, POINT * pts, int ptNum);
	inline bool _ThresholdGray(int nRangeMode, int nRangeMin, int nRangeMax, UCHAR nGray);

	inline bool _Threshold3d(int nRangeMode, float nRangeMin, float nRangeMax, float nGray);

	InspPartInfo * m_pInspBoardInfo;
};

