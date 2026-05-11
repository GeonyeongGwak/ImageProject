#pragma once
#include "PI_Buff.h"

namespace PIAL
{
    typedef INT_PTR BlobPtr;

	//enum  ETypeBlob {
	//	eSelectBigger,		// 가장 큰 Area Blob 1개
	//	eSelectMix,			// All Blob
	//	eSelectCenter,		// Center에 가까운 Blob 1개
	//	eSelectPosition,	// 지정된 좌표에 가까운 Blob 1개
	//	eSimilarArea,		// 면적이 가까운
	//};

#define B_INCLUDE                  1L
#define B_EXCLUDE                  2L
#define B_LABEL_VALUE              1L
#define B_AREA                     2L
#define B_GREATER                  5L
#define B_LESS                     6L
#define B_NOT_EQUAL                4L
#define	B_NULL                     0L

#define B_INCLUDED_BLOBS      0x08000000L
#define B_EXCLUDED_BLOBS      0x20000000L

	static INT_PTR pMainBlob = 0;

	class PINSPALGO_API PI_Blob_Res
	{
	public:
		PI_Blob_Res();
		~PI_Blob_Res();

		int  m_label;
		int  m_area;
		float  m_cx;
		float  m_cy;
		CRect m_rcRect;
		float m_cirlerate;   //UnCoining 검사시 높이로 사용
		float  m_circle_cx;
		float  m_circle_cy;
		float  m_radius;
		int  m_NumUse;
		bool m_IsRemove;
		void* m_ref; // delete 하지 마삼..
		int m_refIndex;
		float m_area_covexhull;
		int  m_nType;
		float m_MajorAxis;
		float m_MinorAxis;
		float m_GV; //Contrast도 사용
		float m_ColorContrast;
		float m_Focus1_x;
		float m_Focus1_y;
		float m_Focus2_x;
		float m_Focus2_y;
		float m_fHeightMin;
		float m_fHeightMax;
		//std::vector<PIAL::_RstEachVoid> m_VoidResult;
		
	};

	class PINSPALGO_API PI_Blob_Res_Detail //SubPixel 용
	{
	public:
		PI_Blob_Res_Detail();
		~PI_Blob_Res_Detail();

		int  m_label;
		float  m_area;
		float  m_cx;
		float  m_cy;

		float  m_cx2;
		float  m_cy2;

		float  m_width;
		float  m_length;

		float  m_left;
		float  m_top;
		float  m_right;
		float  m_bottom;
		float  m_Pixelvalue;
		 
	};


	class PINSPALGO_API PI_Blob
	{
	public:
		PI_Blob();
		~PI_Blob();

	
	
		static BlobPtr BlobAlloc();
		static void BlobReAlloc(int sz_x, int sz_y);
		static void BlobFree(BlobPtr Blobptr);

		static void AllocGlobalBlob();
		static void FreeGlobalBlob();

		static void BlobSelect(BlobPtr Blobptr, INT64 Operation, INT64 Feature, INT64 Condition, double CondLow, double CondHigh);
		static int BlobSelect_Area(BlobPtr Blobptr, PI_Buff* dst, double dArea, double dAreaWPix, double dAreaHPix, int nAreaCnt);
		static int BlobSelect_HiddenArea(BlobPtr Blobptr, PI_Buff* dst, int nTeachX, int nTeachY);
		static int BlobSelect_MaxArea(BlobPtr Blobptr, PI_Buff* dst);
		static int BlobSelect_Inner(BlobPtr Blobptr, PI_Buff* dst, double center_x, double center_y);
		static int BlobSelect_Center(BlobPtr Blobptr, PI_Buff* dst, double center_x, double center_y);
		static int BlobSelect_Center(PI_Buff* dst, double center_x, double center_y);

		static int BlobSelect(BlobPtr Blobptr, PI_Buff* dst, std::vector<int> vecID);
		static int BlobSelect(PI_Buff* dst, std::vector<int> vecID);

		static int BlobSelect_Label(BlobPtr Blobptr, PI_Buff* dst, std::vector<int> vecID);
		static int BlobSelect_Label(PI_Buff* dst, std::vector<int> vecID);

		static int BlobRemove(BlobPtr Blobptr, PI_Buff* dst, std::vector<int> vecID);
		static int BlobRemove(PI_Buff* dst, std::vector<int> vecID);

		static void BlobFill(BlobPtr Blobptr, PI_Buff* dst, INT64 Mode);
		static void BlobFill(PI_Buff* dst, INT64 Mode);
		static int BlobGetNumber(BlobPtr Blobptr);
		static int BlobGetNumber();
		
		static int BlobGetNumber_WithRemove(BlobPtr Blobptr);
		static int BlobGetNumber_WithRemove();

		static void FillHoleBlob(BlobPtr Blobptr, PI_Buff* src, PI_Buff* dst);
		static int CalcBlobDirect(BlobPtr Blobptr, PI_Buff* src);
		static int CalcBlobDirect(PI_Buff* src);
		static int CalcBlobDirect(BlobPtr Blobptr, PI_Buff* SrcDst, int MinArea);
		static int CalcBlobDirect(PI_Buff* SrcDst, int MinArea);

		static int CalcBlobHoleDirect(BlobPtr Blobptr, PI_Buff* src);
		static int CalcBlobHoleDirect(PI_Buff* src);
		static int CalcBlobHoleDirect(BlobPtr Blobptr, PI_Buff* SrcDst, int MinArea);
		static int CalcBlobHoleDirect(PI_Buff* SrcDst, int MinArea);

		static int CalcBlob(BlobPtr Blobptr, PI_Buff* src, int minArea, int maxArea, bool fillHole = false, PI_Buff* dst = nullptr, int nFilter = 0);
		static int CalcBlob(PI_Buff* src, int minArea = 0, int maxArea =0, bool fillHole = false, PI_Buff* dst = nullptr, int nFilter = 0);

		static int CalcBlobSelf(BlobPtr Blobptr, PI_Buff* srcdst, int minArea, int maxArea, bool fillHole = false, int nFilter = 0);
		static int CalcBlobSelf(PI_Buff* srcdst, int minArea = 0, int maxArea = 0, bool fillHole = false, int nFilter = 0);

		static void BlobFilter(BlobPtr Blobptr, PI_Buff* srcdst, bool fillHole = false, int nFilter = 0);
		static void BlobFilter(PI_Buff* srcdst, bool fillHole = false, int nFilter = 0);

		static int CalcBlob_Select(BlobPtr Blobptr, PI_Buff * src, PI_Buff * dst, int nWidth, int nHeight, int minArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType
			, int nTeachX = -1, int nTeachY = -1, int nBlobType = -1, double dArea = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = -1);
		static int CalcBlob_Select(PI_Buff * src, PI_Buff * dst, int nWidth, int nHeight, int minArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType
			, int nTeachX = -1, int nTeachY = -1, int nBlobType = -1, double dArea = -1, double dAreaWPix = -1, double dAreaHPix = -1, int nAreaCnt = -1);


		static void BlobGet_BOX(BlobPtr Blobptr, double* minx, double* miny, double* maxx, double* maxy);
		static void BlobGet_BOX(BlobPtr Blobptr, int* minx, int* miny, int* maxx, int* maxy);

		static void BlobGet_Center(BlobPtr Blobptr, double* cx, double* cy);
		static void BlobGet_Center(double* cx, double* cy);
		static void BlobGet_Center(BlobPtr Blobptr, int* cx, int* cy);

		static int BlobGet_AllCount();
		static void BlobGet_Label(BlobPtr Blobptr, int* pLabel);
		static void BlobGet_Area(BlobPtr Blobptr, double* pLabel);

		static int GetBlobLabel(BlobPtr Blobptr, long *pLebel, USHORT* LabelImage, int nWidth, int nHeight);
		static int GetBlobLabel(long *pLebel, USHORT* LabelImage, int nWidth, int nHeight);
		static int GetBlobLabel(BlobPtr Blobptr, long *pLebel, int* LabelImage, int nWidth, int nHeight);
		static int GetBlobLabel(long *pLebel, int* LabelImage, int nWidth, int nHeight);
		static int GetBlobLabel_ConvertLabel(BlobPtr Blobptr, USHORT* LabelImage, int nWidth, int nHeight, std::vector<int> &converter);
		static int GetBlobLabel_ConvertLabel(USHORT* LabelImage, int nWidth, int nHeight, std::vector<int> &converter);
		static int GetBlobLabel_ConvertLabel(BlobPtr Blobptr, int* LabelImage, int nWidth, int nHeight, std::vector<int> &converter);
		static int GetBlobLabel_ConvertLabel(int* LabelImage, int nWidth, int nHeight, std::vector<int> &converter);

		static int GetBlobLabel(BlobPtr Blobptr, int nLebel, UCHAR* LabelImage, int nWidth, int nHeight, int nVal);
		static int GetBlobLabel(int nLebel, UCHAR* LabelImage, int nWidth, int nHeight, int nVal = 255);

		static void DrawRectLabel(BlobPtr Blobptr, PI_Buff* dst, RECT rect, int LabelID);
		static void DrawRectLabel(PI_Buff* dst, RECT rect, int LabelID);

		//bAllBlob -
		//   false:  blob 전체에 대한 결과(All blob)
		//   true :  max Area 기준 최대 3개까지
		static bool BlobGet_Result(BlobPtr Blobptr, double *dArea, double *dCx, double *dCy, CRect *rcRect, bool bAllBlob = false);
		static bool BlobGet_Result(double *dArea, double *dCx, double *dCy, CRect *rcRect, bool bAllBlob = false);

		static bool BlobGet_Result(BlobPtr Blobptr, std::vector<PI_Blob_Res>& results);
		static bool BlobGet_Result(std::vector<PI_Blob_Res>& results);
		static bool BlobGet_Result(BlobPtr Blobptr,int nLabelID, PI_Blob_Res* result);
		static bool BlobGet_Result(int nLabelID, PI_Blob_Res* result);
		static bool BlobGet_Result(BlobPtr Blobptr, CRect rect, PI_Blob_Res* result);
		static bool BlobGet_Result(CRect rect, PI_Blob_Res* result);

		static int GetContour(BlobPtr Blobptr, CRect rect, std::vector<POINT>& contour, int nLabelID = -1);
		static int GetContour(CRect rect, std::vector<POINT>& contour, int nLabelID = -1);

		static int BlobGet_Rects(BlobPtr Blobptr, RECT *rcRect, int nRectCnt);
		static int BlobGet_Rects(RECT *rcRect, int nRectCnt);

		static int BlobGet_Rects(BlobPtr Blobptr, std::vector<CRect> & rcRect);
		static int BlobGet_Rects(std::vector<CRect> & rcRect);

		static bool BlobGet_Result_SubPixel(PI_Buff * binImage, PI_Blob_Res_Detail* result, int Factor = 1);


		static void SortDistanceBytPT(float cx, float cy, std::vector< PI_Blob_Res*>& blobs);

		static void DrawHoleLabels(BlobPtr Blobptr, PI_Buff* binImage, std::set<int>& Labels, long val);
		static void DrawHoleLabels(PI_Buff * binImage, std::set<int>& Labels, long val);
	};

}

