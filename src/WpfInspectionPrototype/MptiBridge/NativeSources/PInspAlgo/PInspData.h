#pragma once

namespace PIAL
{
#define BODYEDGE_MAXCOUNT 8

	class PINSPALGO_API BodyInfo // Body
	{
	public:
		BodyInfo();
		~BodyInfo();

		bool Passive;										// Passive Part여부, 이물데이터와 Body2D값이 겹치면 NG치기 위함.
		float BodyAngle;									// 검사된 파트의 Angle 값. 0도 기준으로 본다.
		POINT* BodyStartPos;								// Body 시작점 (파트 이미지 기준)
		SIZE* BodySize;										// Body 크기 (이미지좌표)
		POINT* Shift;										// Passive 가 아닌 Body에서는 AlignResult값 따라가도 될 것으로 보임.. 나중에 삭제될 가능성이 큼..
		POINT BodyCenter;									// Body이미지 중심좌표
		POINTF* EdgePoint;									// Body의 EdgePoint, 최대 8개까지
		PI_Buff* Body2D;									// Body를 이진화 한 이미지 <Angle 보정하기 전>
		std::vector<POINT> EntireBodyEdge;
	};

	class PINSPALGO_API MaskInfo							// Masking 이미지, Clip된 데이터가 들어와야 한다. 
	{
	public :
		MaskInfo();
		~MaskInfo();
		POINT StartPos;										// 파트이미지 기준에서의 시작좌표.
		int Width;											// ExceptROI일 경우에 사용함.
		int Length;											// ExceptROI일 경우에 사용함.
		PI_Buff* MaskImage;									// 파트이미지에 붙일 Mask이미지
	};

	class PINSPALGO_API ForeignMaskInfo						//   
	{
	public:
		ForeignMaskInfo();
		~ForeignMaskInfo();
		cv::Rect2f vForeignMaskData;
		float fAngle;
	};

	class PINSPALGO_API PInspData //Window
	{
	private:
		std::vector<MaskInfo*> MaskData;					// Masking 이미지 버퍼, +
															// Key = AlgoID(숫자만), Value = Mask데이터

		// std::vector<BodyInfo*> BodyData;					// Body정보가 여러개 필요 할 수 있음. 
		// 													// Key = AlgoID(숫자만), Value = Body데이터
		BodyInfo* BodyData;
		
	public:
		PInspData();
		~PInspData();

		int WindowID;										// 저장된 윈도우ID
		int AlgoType;
		std::vector<MaskInfo*> GetMaskData();
		void SetMaskData(MaskInfo* NewMaskData);

		BodyInfo* GetBodyData();
		void SetBodyData(BodyInfo* NewBodyData);

		bool GetBodyContour(int nOffsetX, int nOffexY, std::vector<POINTF>& CornerEdge, std::vector<POINT>& Contour);
		int m_nArrPointsCnt;
		POINTF m_poArrPoints[200];
		int m_nArrMatchingIdx[200];
		float m_fArrMatchingSc[200];
		void SetArrPoints(int nROICnt, POINTF* poArrCenter, int* nArrMatchingIdx, float* fArrMatchingSc);
		PI_Buff* PadBWMask;
		PI_Buff* ShapeXCurrentMask;  // CurrentShape 모드용 16bit 레이블 마스크 (CV_16UC1)
		cv::Mat* PadBWDefualtECC;
	};


	class PINSPALGO_API PInspDataSet
	{
	private:
		double m_nResultHeightOffset; // ReferencePlane ~ UserSet BottomPlane
		double m_nPickerOffset; 
		double m_nThickestValue;
		RECT* m_rcThickestPosition;

		std::vector<PInspData*> InspDataSet;
		static cv::Rect ResizeRect(cv::Rect rtMaskROI, cv::Rect rtWndROI);
		double GetPickerOffset();
		double GetThickestValue();
		std::vector<ForeignMaskInfo*> ForeignMaskData;
	public:
		PInspDataSet();
		~PInspDataSet();

		std::vector<MaskInfo*> GetMaskData();
		std::vector<PInspData*> GetInspDataSet();
		std::vector<ForeignMaskInfo*> GetForeignMaskData();
		void SetForeignMaskData(ForeignMaskInfo* NewForeignMaskData);

		PInspData* GetInspData(int WndID);
		void SetInspData(PInspData* InspData);
		static cv::Mat SetMaskToImage(cv::Mat partMat, cv::Mat maskMat, int nX, int nY, int nWidth, int nHeight);
		static cv::Mat SetMaskToImage(cv::Mat src, PInspDataSet* InspDataSet, std::vector<MaskInfo*> maskList, bool IsPart, int nWndStartX, int nWndStartY);
		static cv::Mat SetMaskToImage(PIAL::Insp_Image* pImg_buf, PInspDataSet* InspDataSet, std::vector<MaskInfo*> maskList, bool IsPart = true, int nWndStartX = 0, int nWndStartY = 0);
		//신규
		void SetMaskToImage(PIAL::PI_Buff* pBuffMask, std::vector<_AlignResult>& vecAlign, std::vector<MaskInfo*> VecVolatileMask, bool IsPart = true, int nWndStartX = 0, int nWndStartY = 0, int AlgoType = 0);

		void RemoveMask(PIAL::PI_Buff* pBuffMask, bool IsPart = true, int nWndStartX = 0, int nWndStartY = 0);

		double GetHeightOffset();
		double GetPackageThickness();
		RECT* GetThickestPosition();
		void SetHeightOffset(double Offset);
		void SetPickerOffset(double dThickness);
		void SetThickestPosition(double dThickness, RECT* rc);

		static PInspData* GetCurrentBodyData(std::vector<PInspData*>& Inspset, std::vector<_AlignResult>& vecAlign, int AlgoType = 0);
		static void SetModuleForeignRect(PIAL::PInspDataSet* InspData, wchar_t* sForeignRect);
		//std::vector<cv::Rect2f> ForeignMaskData;
	};
}
