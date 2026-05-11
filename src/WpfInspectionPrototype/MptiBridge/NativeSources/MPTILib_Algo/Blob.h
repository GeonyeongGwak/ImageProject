#pragma once

#include <vector>
#include <memory>
#include <set>
//#define  USE_CONNECTIVITY_NET
#define PTR_BLOB_MAX	65534//32767
// 결과 데이터 구조체
typedef struct _BLOB_RESULT_
{
	int label;
	int _tremove;
	int area;
	int min_x;
	int min_y;
	int max_x;
	int max_y;
	int cx;
	int cy;
	int min_x_y;
	int min_y_x;
	int max_x_y;
	int max_y_x;
}tagBlobResult;

// 라벨링시 사용할 데이터 타입
typedef int		label_t;

// RLE데이터 구조체
typedef struct tagPTRLE
{
	int	label;
	USHORT  st;
	USHORT  ed;

#ifdef USE_CONNECTIVITY_NET
	//	int Link[2];
	//	USHORT Lb[2];
	int stLink;
	int edLink;
#endif
} ptRLE;

// line별로 RLE의 시작데이터와 끝 데이터를 가리키는 구조체
class ptRLE_Y
{
public:
	int  st;
	int  ed;
#ifdef USE_CONNECTIVITY_NET
	int front;
	int rear;

	inline	void Left(ptRLE * RLE, int node)
	{
		int i=front;
		for (;i<=ed; i++)
		{
			if(RLE[i].stLink<=0)
				//	if(RLE[i].Link[0]<=0)
				break;
		}

		if(i<=ed)
			front=i;
		else
			front=0;

		if(rear < node)
			rear = node;
	}

	inline	void Right(ptRLE * RLE, int node)
	{
		int i=front;
		for (;i>=st; i--)
		{
			if(RLE[i].edLink<=0)
				//	if(RLE[i].Link[1]<=0)
				break;
		}

		if(i>=st)
			front=i;
		else
			front=0;

		if(rear > node)
			rear = node;
	}
#endif
};

#include "MemoryManager.h"
class BlobNode
{
public:
	bool IsLeft;
	int row;
	int col;
	std::weak_ptr<BlobNode> link;
	std::weak_ptr<BlobNode> prev;
	std::weak_ptr<BlobNode> next;
	bool ishole;
	bool IsSearched;
	BlobNode(void)
	{
		IsLeft = true;
		row = 0;
		col = 0;
		link.reset();// = nullptr;
		prev.reset();// = nullptr;
		next.reset();// = nullptr;
		ishole = false;
		IsSearched = false;
		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	BlobNode(bool flag)
	{
		IsLeft = flag;
		row = 0;
		col = 0;
		link.reset();// = nullptr;
		prev.reset();// = nullptr;
		next.reset();// = nullptr;
		ishole = false;
		IsSearched = false;
		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}
	BlobNode(BlobNode& node)
	{
		Free();
		copy(node);
		g_pMManager->pem_new_check(this, (PCHAR)__FUNCTION__, __LINE__);
	}

	~BlobNode(void)
	{
		Free();
		g_pMManager->pem_delete_check(this);
	}
	void operator=(BlobNode node)
	{
		Free();
		copy(node);
	}
	// 	void operator=(ptRLE Rle)
	// 	{
	// 		Free();
	// 		copy(Rle);
	// 	}
	void Free()
	{
		IsLeft = true;
		row = 0;
		col = 0;
		link.reset();// = nullptr;
		prev.reset();// = nullptr;
		next.reset();// = nullptr;
		ishole = false;
		IsSearched = false;
	}
	void copy(BlobNode node)
	{
		IsLeft = node.IsLeft;
		row = node.row;
		col = node.col;
		link = node.link;
		prev = node.prev;
		next = node.next;
		ishole = node.ishole;
		IsSearched = node.IsSearched;
	}
	void Set(int rowIndex,int colIndex)
	{
		row = rowIndex;
		col = colIndex;;
		link.reset();// = nullptr;
		prev.reset();// = nullptr;
		next.reset();// = nullptr;
		ishole = false;
		IsSearched = false;
	}
};

// RLE 수행시 사용할 데이터 타입
typedef int RLE_t;


namespace jsl
{
	class Blob
	{
	public:
		typedef __int64		res_type;
	public:
		Blob(void);
		virtual ~Blob(void);

	public:
		// 초기화함수 sz:영상의 최대 크기
		bool Alloc(long sz_x, long sz_y);
		bool ReAlloc(long sz_x, long sz_y);
		void Free();


		// ============================================================
		// Mil의 BlobCalculate에 대응되는 함수.
		// ============================================================

		// MblobCalculate
		int Calculate(BYTE * bImage, long sz_x, long sz_y, long pitch)
		{
			if(sz_y>BufSz_y|| sz_x > bufSz_x)
				ReAlloc(sz_x,sz_y);
			RunLengthEncoding(bImage, sz_x, sz_y, pitch);
			int number=ConnectedComponentLabeling();
			CalculateFeatures();

			return number;
		}
		// Polygon
		void GetPolygon(UCHAR * uArrDst, int nWidth, int nHeight, int nStartX, int nStartY, std::vector<std::vector<POINTF>>* vPolygons, vector<POINTF>* vPolyCenter);
		// Run-Length Encoding
		int RunLengthEncoding(BYTE * bImage, long sz_x, long sz_y, long pitch);

		// Connected-Component Labeling
		int ConnectedComponentLabeling();
		int ConnectedComponentLabeling_both(); // back&forground labeling

		// feature 계산
		void CalculateFeatures(bool bCalcBlobSize = false);


		// ============================================================
		// Mil의 Draw 또는 Fill에 대응되는 함수.
		// ============================================================

		// 라벨맵을 그린다.
		void DrawLabels(int * bImage, long sz_x, long sz_y, long pitch, long min_x, long min_y, long label);
		void DrawLabels(USHORT * bImage, long sz_x, long sz_y, long pitch);
		void DrawLabels(USHORT * bImage, long sz_x, long sz_y, long pitch, long label);
		void DrawLabels(UCHAR * bImage, long sz_x, long sz_y, long pitch, long label, long val);
		void DrawLabels(UCHAR * bImage, long sz_x, long sz_y, long pitch);

		// 지워진 블랍을 영상에서 0으로 체운다.
		void DrawRemoves(BYTE * bImage, long sz_x, long sz_y, long pitch);
		void Draws(BYTE * bImage, long sz_x, long sz_y, long pitch);

		// 외곽에지픽셀을 원하는 값으로 체운다.
		void DrawContours(BYTE * bImage, long sz_x, long sz_y, long pitch, long nVal);

		// Blob을 255로 체우고 외곽에지픽셀을 원하는 값으로 체운다.
		void DrawLabelContours(BYTE * bImage, long sz_x, long sz_y, long pitch, long nVal);

		// 외곽 에지픽셀을 구하고 블랍의 길이와 Feret영역을 구한다. 
		void CalcPerimeter(BYTE * bImage, USHORT * bLabel,long sz_x, long sz_y, long pitch, int& width);

		// 외곽 에지픽셀을 구하고 Shift 강도를 리턴 

		// 블랍의 홀을 체운다.
		void FillHoles(BYTE * bImage, long sz_x, long sz_y, long pitch);
		int MarkBackg_Label(ptRLE * RLE, int nEncCnt, ptRLE_Y * yList, int sz_y);
		int MarkLabel_NHole(ptRLE * RLE, int nEncCnt, ptRLE_Y * yList, int sz_y);
		void DrawHoleLabels(UCHAR * bImage, long sz_x, long sz_y, long pitch, int* labels, long val, int labelCount);
		void DrawHoleLabels(UCHAR* bImage, long sz_x, long sz_y, long pitch, std::set<int>& Labels, long val);


		// ============================================================
		// Mil의 MbobSelect에 대응되는 함수
		// ============================================================

		void RemoveMinArea(int minArea);// Area가 입력값보다 미만이면 제거
		void RemoveMaxArea(int maxArea);// Area가 입력값보다 초과면 제거
		void RemoveLabel(int label);	// Label이 입력값과 같으면 제거
		void RemoveLabelinv(int label);	// Label이 입력값과 다르면 제거
		void RemoveSmallObjects(int minw, int minh);// Blob width, height가 minw, minh 이하면 제거

		// ============================================================
		// Mil의 MbobGetResult에 대응되는 함수
		// ============================================================
		std::vector<RECT> GetRects();
		//		void GetCenter(float * x, float * y);
		template<typename T>
		void GetCenter(T * x, T * y);										//shkim add
		//		void GetMinMax(long * min_x, long * min_y, long * max_x, long * max_y);
		template<typename T>
		void GetMinMax(T * min_x, T * min_y, T * max_x, T * max_y);//shkim add

		template<typename T>
		void GetMinX(T * min_x);
		template<typename T>
		void GetMaxX(T * max_x);

		template<typename T>
		void GetMinY(T * min_y);
		template<typename T>
		void GetMaxY(T * max_y);

		template<typename T>
		void GetPerimeter(T * perimeter);

		template<typename T>
		void GetSizeX(T * size_X);

		template<typename T>
		void GetSizeY(T * size_Y);

		void GetDiameter(double * dia);
		long GetLabel(long x, long y, long * label);

		template<typename T>
		void GetLabel(T * label);

		template<typename T>
		void GetArea(T * area);

		// 블랍 개수
		int m_nBlobCount;		// 모든 블랍 개수
		int m_nBlobRealCount;	// _tremove한 블랍을 제외한 개수

		// 계산된 블랍 특징정보가 저장되는 구조체 리스트
		//		std::vector<tagBlobResult> Result;

		int * m_label;
		int * m_remove;
		int * m_area;
		int * m_min_x;
		int * m_min_y;
		int * m_max_x;
		int * m_max_y;
		float * m_cx;
		float * m_cy;
		int * m_min_x_y;
		int * m_min_y_x;
		int * m_max_x_y;
		int * m_max_y_x;
		float * m_perimeter;
		float * m_size_x;
		float * m_size_y;

	protected:
		int MarkLabel(ptRLE * RLE, int nEncCnt, ptRLE_Y * yList, int sz_y);
		//		void FillHoleByContour(USHORT * bLabel, UCHAR * bImage, long sz_x, long sz_y, long pitch, int nLabel);
		bool isHaveHoleLabel(int Label);

		// chain-code알고리즘을 이용해서 blob의 시계방향으로 외곽선을 찾고 Perimeter와 Feret영역을 구한다.
		void PerimeterContour(int sy, int sx, USHORT * bLabel, BYTE * bImage, long sz_x, long sz_y, long pitch, long label, int& width);

		// chain-code알고리즘을 이용해서 blob의 시계방향으로 외곽선을 찾고 Perimeter와 Feret영역을 구한다.

		// chain-code알고리즘을 이용해서 시계방향으로 외곽선을 찾는다.
		void Contour(int sy, int sx, label_t * bLabel, BYTE * bImage, long sz_x, long sz_y, long pitch, long label);
		//		void Contour(int sy, int sx, BYTE * bImage, long sz_x, long sz_y, long pitch, long val);
		//		void Contour(int sy, int sx, USHORT * bLabel, BYTE * bImage, long sz_x, long sz_y, long pitch, long label, long val);

		// 버퍼 크기 정보
		int bufSz_x;
		int BufSz_y;
		int AllocSz;

		// 현재 영상 크기정보
		int size_x;
		int size_y;

		// 라벨링 할 때 사용되는 변수.
		label_t * LabelTmp;
		label_t * Label;

		// RLE데이터 저장소 _f는 foreground, _b는 background
		ptRLE * RLE_f;
		ptRLE * RLE_b;
		int RleCnt_f;
		int RleCnt_b;

		// 라인 별 RLE의 시작과 끝을 저장한다. _f는 foreground, _b는 background
		ptRLE_Y * yList_b;
		ptRLE_Y * yList_f;

		//hole label
		int* ptHoleLabel;
		int ptHoleLabelCnt;

#ifdef USE_CONNECTIVITY_NET
	public:
		// one-pass 방식으로 연결정보를 찾는다. st와 ed를 따로 찾는다.
		void Construct_Connectivity_Net();
	protected:
		// one-pass 방식으로 연결정보 찾기 st:Left, ed:Right
		inline	void Recur_Connect_Left(int StartNode, int k, int & label);
		inline	void Recur_Connect_Right(int StartNode, int k, int & label);
		void Recur_Fill_Holes();
#endif
	public:
		// one-pass 방식으로 연결정보를 찾는다. st와 ed를 따로 찾는다.
		void Construct_Connectivity_Net();
	protected:
		// one-pass 방식으로 연결정보 찾기 st:Left, ed:Right
		inline	void Recur_Connect_Left(std::shared_ptr<BlobNode> StartNode);
		inline	void Recur_Connect_Right(std::shared_ptr<BlobNode> StartNode);
		void Recur_Fill_Holes(UCHAR * bImage, long sz_x, long sz_y, long pitch);
		void ListFrontSet(std::shared_ptr<BlobNode> Node,int index);
		void ListRearSet(std::shared_ptr<BlobNode> Node,int index);
		void DrawHole(std::shared_ptr<BlobNode> leftNode,int maxIndex);

		std::vector<std::vector<std::shared_ptr<BlobNode>>> _NodeYList;
		std::vector<std::shared_ptr<BlobNode>> _FrontNodeList;
		std::vector<std::shared_ptr<BlobNode>> _ReartNodeList;

	};

	template<typename T>
	void Blob::GetArea(T * area)
	{
		int LabelCnt = m_nBlobCount;

		for(int i=1, n=0; i<=LabelCnt; i++)
		{
			if(m_remove[i]==0)
			{
				area[n] = (T)(m_area[i]);
				n++;
			}
		}
	}

	template<typename T>
	void Blob::GetLabel(T * label)
	{
		if(label==NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for(int i=1, n=0; i<=LabelCnt; i++)
		{
			if(m_remove[i]==0)
			{
				label[n] = (T)m_label[i];
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetCenter(T * x, T * y)
	{
		if(x==NULL || x==NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for(int i=1, n=0; i<=LabelCnt; i++)
		{
			if(m_remove[i]==0)
			{
				float area = m_area[i];
				if(area > 0)
				{
					x[n] = (T)m_cx[i] / area;
					y[n] = (T)m_cy[i] / area;
				}
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetMinMax(T * min_x, T * min_y, T * max_x, T * max_y)
	{
		if(min_x==NULL || min_y==NULL || max_x==NULL || max_y==NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for(int i=1, n=0; i<=LabelCnt; i++)
		{
			if(m_remove[i]==0)
			{
				min_x[n] = (T)m_min_x[i];
				min_y[n] = (T)m_min_y[i];
				max_x[n] = (T)m_max_x[i];
				max_y[n] = (T)m_max_y[i];
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetMinX(T * min_x)
	{
		if (min_x == NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for (int i = 1, n = 0; i <= LabelCnt; i++)
		{
			if (m_remove[i] == 0)
			{
				min_x[n] = (T)m_min_x[i];
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetMaxX(T * max_x)
	{
		if (max_x == NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for (int i = 1, n = 0; i <= LabelCnt; i++)
		{
			if (m_remove[i] == 0)
			{
				max_x[n] = (T)m_max_x[i];
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetMinY(T * min_y)
	{
		if (min_y == NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for (int i = 1, n = 0; i <= LabelCnt; i++)
		{
			if (m_remove[i] == 0)
			{
				min_y[n] = (T)m_min_y[i];
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetMaxY(T * max_y)
	{
		if (max_y == NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for (int i = 1, n = 0; i <= LabelCnt; i++)
		{
			if (m_remove[i] == 0)
			{
				max_y[n] = (T)m_max_y[i];
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetPerimeter(T * perimeter)
	{
		if (perimeter == NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for (int i = 1, n = 0; i <= LabelCnt; i++)
		{
			if (m_remove[i] == 0)
			{
				perimeter[n] = (T)m_perimeter[i];
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetSizeX(T * size_X)
	{
		if (size_X == NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for (int i = 1, n = 0; i <= LabelCnt; i++)
		{
			if (m_remove[i] == 0)
			{
				size_X[n] = (T)m_size_x[i];
				n++;
			}
		}
	}
	template<typename T>
	void Blob::GetSizeY(T * size_Y)
	{
		if (size_Y == NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for (int i = 1, n = 0; i <= LabelCnt; i++)
		{
			if (m_remove[i] == 0)
			{
				size_Y[n] = (T)m_size_y[i];
				n++;
			}
		}
	}

	template<typename T>
	class CircularQueue
	{
	public:
		CircularQueue()
			: m_array(NULL)
			, m_nSize(0)
			, m_nStIdx(0)
			, m_nEdIdx(0)
			, m_Offset(0)
		{

		}
		CircularQueue(int nSize, int Ofs = 0)
			: m_array(NULL)
			, m_nSize(0)
			, m_nStIdx(0)
			, m_nEdIdx(0)
			, m_Offset(0)
		{
			Alloc(nSize, Ofs);
		}

		virtual ~CircularQueue()
		{
			Free();
		}

	public:
		bool Alloc(int nSize, int Ofs = 0)
		{
			Free();

			m_Offset = Ofs;
			m_nStIdx = m_nEdIdx = m_Offset;
			m_nSize = nSize;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &m_array, nSize);

			return m_array != NULL;
		}
		void Free()
		{
			if (m_array)
			{
				m_nStIdx = m_nEdIdx = m_Offset;
				Delete_1DArray(&m_array);
			}
		}


		int Push(T val)
		{
			//m_nEdIdx++;
			//m_nEdIdx = m_nEdIdx % m_nSize;
			m_nEdIdx = m_nEdIdx < (m_nSize - 1) ? m_nEdIdx + 1 : m_Offset;

			if (m_nStIdx == m_nEdIdx)
				Pop();
			m_array[m_nEdIdx] = val;
			return m_nEdIdx;
		}

		T * Ptr()
		{
			return m_array;
		}

		int Size()
		{
			return m_nEdIdx >= m_nStIdx ? (m_nEdIdx - m_nStIdx) : (m_nSize - m_nStIdx) + (m_nEdIdx - m_Offset) + 1;
		}

		T & Front()
		{
			return m_array[m_nStIdx];
		}

		bool Pop()
		{
			if (m_nStIdx == m_nEdIdx)
				return false;

			//m_nStIdx++;
			//m_nStIdx = m_nStIdx % m_nSize;
			m_nStIdx = m_nStIdx < (m_nSize - 1) ? m_nStIdx + 1 : m_Offset;

			return true;
		}

		T & operator [](int idx)
		{
			return m_array[idx];
		}

		void Set(int idx, T & val)
		{
			m_array[idx] = val;
		}

	protected:
		int m_nStIdx;
		int m_nEdIdx;

		T * m_array;
		int m_nSize;
		int m_Offset;
	};
};
