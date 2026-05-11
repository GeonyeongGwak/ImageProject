#pragma once
#include "PI_Buff.h"
#include "BlobBoundary.h"

namespace PIAL
{

	enum Img_Channel
	{
		Top_R
		, Top_G
		, Top_B
		, Top_W

		, Middle_R
		, Middle_B

		, Bottom_R
		, Bottom_B

		, Side1_R
		, Side1_G
		, Side1_B

		, Side2_R
		, Side2_G
		, Side2_B

		, Side3_R
		, Side3_G
		, Side3_B

		, Side4_R
		, Side4_G
		, Side4_B

		, _3D
	};

	class PINSPALGO_API Insp_ROIImg
	{
	public:
		Insp_ROIImg();
		~Insp_ROIImg();

		int nImageSizeX;
		int nImageSizeY;

		RECT m_rcSubRect; // Window Rect

		std::map <Img_Channel, PI_Buff*> m_ImgBuf;

		bool AddBuff(Img_Channel ch, PI_Buff* buff);
		void Clear();
		PI_Buff* GetBuff(Img_Channel ch);
		UCHAR* GetRawBuff(Img_Channel ch);

		Insp_ROIImg* ClipBuff_LT(int nSX, int nSY, int roiSizeX, int roiSizeY, bool bClone, bool bref);
	};

	class PINSPALGO_API Insp_Base
	{
	public:
		Insp_Base();
		~Insp_Base();

		int nImgSizeX; // 이미지 사이즈 (단위 px), 파트, 윈도우사이즈 일 수 있음.
		int nImgSizeY;

		PI_Buff* m_p3D;

		Insp_ROIImg* m_ImageBuffer;
	};


	class PINSPALGO_API _InspRstPolyAlgo
	{
	public:
		int m_ndX; // Teaching 창에서 검사시 빼주기 위해 가지고있음
		int m_ndY; // Teaching 창에서 검사시 빼주기 위해 가지고있음
		int m_nWindID;
		int m_nAlgoID;
		int m_nRoiID;
		int m_nCount; // ROI내에 Blob 몇개 잡혔는지 나타내는 Count
		POINTF m_ptCenter;
		std::vector<POINTF> m_vPolyPoints;
	public:
		_InspRstPolyAlgo(int ndX, int ndY, int nWindID, int nAlgoID, int nRoiID, int nCount, POINTF ptCenter, std::vector<POINTF> vPolyPoints);

		_InspRstPolyAlgo(int nRoiID, std::vector<POINT>& vPolyPoints, int nOffsetX = 0, int nOffsetY = 0);

		RECT GetBoundary();
	};

	class PINSPALGO_API Insp_Wnd : public Insp_Base
	{
	public:
		Insp_Wnd();
		~Insp_Wnd();

		int nInspWindowType;
		int nAlgoIndex;

		int nStartX;
		int nStartY;

		float fWndCX;
		float fWndCY;

		BlobBoundary* m_BlobBoundary;
		BlobBoundary* m_NGBlobBoundary;		
		std::set<int> m_BridgealgoIndex;
		float m_fGV_AVG;
	};

	class PINSPALGO_API Insp_Part : public Insp_Base
	{
	public:
		Insp_Part();
		~Insp_Part();

		float m_Angle;
		float m_fPartRoundingErrX;
		float m_fPartRoundingErrY;
	};

	class PINSPALGO_API Insp_BGA_Wnd : public Insp_Wnd
	{
	public:
		Insp_BGA_Wnd();
		~Insp_BGA_Wnd();

		std::set<int> m_BridgealgoIndex;
	};

	class PINSPALGO_API Insp_Xray
	{
	public:
		Insp_Xray();
		~Insp_Xray();

		int nImgSizeX;
		int nImgSizeY;
		UCHAR* org2D; // 2D 기본영상, 여기에 사용할지 상위에서 사용할지 정해야함.
		std::map <int, UCHAR*> m_ImgBuf; // 원본 이미지, 가공해서는 안됌. 일반적으로 PartSize로 들어와야함..
		std::set<UCHAR*> m_SelectedSliceImage; // 원본이미지 중에 유저가 선택해서 사용하는 이미지

		bool AddBuff(int nIdx, UCHAR* buff);
		UCHAR* GetBuff(int nIdx);
		void Clear();

		void AddSelectedBuffer(UCHAR* buff);
	};

	class PINSPALGO_API Insp_Image
	{
	public:
		Insp_Image();
		~Insp_Image();

		Insp_Part* inspPartImage;
		Insp_Wnd* inspWndImage;
		Insp_Xray* InspXrayImage; // Slice Image

		PI_Buff* m_p2D; // 조합영상
		PI_Buff* m_p2D_Mix[2]; // 다른 조명 이미지
		Insp_ROIImg* m_AngleColorImageBuffer; // AngleColor 이미지
		void ClearWindowData();

		//DSI Result
		bool bUseDSI;
		int nWindowID;
		int nAlgoID;
		std::vector<_InspRstPolyAlgo> vecDSI;
		PI_Buff* m_pAI;
	};

	class PINSPALGO_API AlgoSet
	{
	public:
		AlgoSet();
		~AlgoSet();

		int nAlgoindex;
		int nAlgoType;
		bool bRst;
		bool bChanged;
		int nAlgoID;
	};

	class PINSPALGO_API AlgoSetBGA: public AlgoSet
	{
	public:
		AlgoSetBGA();
		AlgoSetBGA(int algoindex, _AlgoBGA algo, _RstAlgoBGA rst);
		~AlgoSetBGA();
       
		_AlgoBGA m_Algo;
		_RstAlgoBGA m_Rst;

	};

	class PINSPALGO_API AlgoSetBump : public AlgoSet
	{
	public:
		AlgoSetBump();
		AlgoSetBump(int algoindex, _AlgoBump algo, _RstAlgoBump rst);
		AlgoSetBump(int algoindex, int algoID, _AlgoBump algo, _RstAlgoBump rst);
		~AlgoSetBump();

		_AlgoBump m_Algo;
		_RstAlgoBump m_Rst;

	};

	class PINSPALGO_API BumpLocation
	{
	public:
		BumpLocation();
		BumpLocation(int nAlgoIndex, POINTF ptTeach);
		BumpLocation(int nAlgoIndex, POINTF ptTeach, float fWidth_mm, float fLength_mm);

		int m_nAlgoIndex;
		POINTF m_ptTeach;
		POINTF m_ptInsp;
		float m_fWidth_mm;
		float m_fLength_mm;
	};
}

