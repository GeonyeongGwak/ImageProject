#include "StdAfx.h"
#include "Blob.h"
#include <map>
#include <set>
#include "opencv2/opencv.hpp"
#include <ppl.h>

namespace jsl
{
// 	UCHAR whiteCharBuf[4096];
// 	UCHAR blockCharBuf[4096];
	Blob::Blob(void)
		: bufSz_x(0)
		, BufSz_y(0)
		, size_x(0)
		, size_y(0)
		, LabelTmp(0)
		, Label(0)
		, RLE_f(0)
		, RLE_b(0)
		, yList_b(0)
		, yList_f(0)
		, RleCnt_f(0)
		, RleCnt_b(0)
		, m_nBlobCount(0)
		, m_nBlobRealCount(0)
		, AllocSz(0)
		, m_label(0)
		, m_remove(0)
		, m_area(0)
		, m_min_x(0)
		, m_min_y(0)
		, m_max_x(0)
		, m_max_y(0)
		, m_cx(0)
		, m_cy(0)
		, m_min_x_y(0)
		, m_min_y_x(0)
		, m_max_x_y(0)
		, m_max_y_x(0)
		, m_perimeter(0)
		, m_size_x(0)
		, m_size_y(0)
		, ptHoleLabel(0)
	{
// 		memset(whiteCharBuf,255,sizeof(UCHAR)*4096);
// 		memset(blockCharBuf,0,sizeof(UCHAR)*4096);
	}

	Blob::~Blob(void)
	{
		Free();
	}

	bool Blob::Alloc(long sz_x, long sz_y)
	{
		Free();

		bufSz_x = sz_x;
		BufSz_y = sz_y;

		int nx = sz_x % 16;
		int ny = sz_y % 16;

		int nMapSz = (sz_x+nx) * (sz_y+ny);
		AllocSz = nMapSz;

		sz_y += 2;

		/*yList_f   =	(ptRLE_Y *)_aligned_malloc(sz_y*sizeof(ptRLE_Y), 16);
		yList_b   =	(ptRLE_Y *)_aligned_malloc(sz_y*sizeof(ptRLE_Y), 16);
		LabelTmp  =	(label_t *)_aligned_malloc(nMapSz*sizeof(label_t), 16);
		Label	  =	(label_t *)_aligned_malloc(nMapSz*sizeof(label_t), 16);
		RLE_f	  =	(ptRLE *)_aligned_malloc(nMapSz*sizeof(ptRLE), 16);
		RLE_b     =	(ptRLE *)_aligned_malloc(nMapSz*sizeof(ptRLE), 16);
		ptHoleLabel = (int *)_aligned_malloc(nMapSz*sizeof(int), 16);

		m_label		= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_remove	= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_area		= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_min_x		= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_min_y		= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_max_x		= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_max_y		= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_cx		= (float *)_aligned_malloc(PTR_BLOB_MAX*sizeof(float), 16);
		m_cy		= (float *)_aligned_malloc(PTR_BLOB_MAX*sizeof(float), 16);
		m_min_x_y	= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_min_y_x	= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_max_x_y	= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_max_y_x	= (int *)_aligned_malloc(PTR_BLOB_MAX*sizeof(int), 16);
		m_perimeter	= (float *)_aligned_malloc(PTR_BLOB_MAX*sizeof(float), 16);
		m_size_x	= (float *)_aligned_malloc(PTR_BLOB_MAX*sizeof(float), 16);
		m_size_y	= (float *)_aligned_malloc(PTR_BLOB_MAX*sizeof(float), 16);*/

		yList_f = (ptRLE_Y *)g_pMManager->pem_aligned_malloc(sz_y * sizeof(ptRLE_Y), 16, (PCHAR)__FUNCTION__, __LINE__);
		yList_b = (ptRLE_Y *)g_pMManager->pem_aligned_malloc(sz_y * sizeof(ptRLE_Y), 16, (PCHAR)__FUNCTION__, __LINE__);
		LabelTmp = (label_t *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(label_t), 16, (PCHAR)__FUNCTION__, __LINE__);
		Label = (label_t *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(label_t), 16, (PCHAR)__FUNCTION__, __LINE__);
		RLE_f = (ptRLE *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(ptRLE), 16, (PCHAR)__FUNCTION__, __LINE__);
		RLE_b = (ptRLE *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(ptRLE), 16, (PCHAR)__FUNCTION__, __LINE__);
		ptHoleLabel = (int *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);

		m_label = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_remove = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_area = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_min_x = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_min_y = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_max_x = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_max_y = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_cx = (float *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_cy = (float *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_min_x_y = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_min_y_x = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_max_x_y = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_max_y_x = (int *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_perimeter = (float *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_size_x = (float *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);
		m_size_y = (float *)g_pMManager->pem_aligned_malloc(PTR_BLOB_MAX * sizeof(float), 16, (PCHAR)__FUNCTION__, __LINE__);

		memset(&RLE_f[0], 0, sizeof(ptRLE) * nMapSz);
		memset(&RLE_b[0], 0, sizeof(ptRLE) * nMapSz);
		memset(&LabelTmp[0], 0, sizeof(label_t) * nMapSz);
		memset(&Label[0], 0, sizeof(label_t) * nMapSz);
		memset(&ptHoleLabel[0], 0, sizeof(label_t) * nMapSz);
			
		return true;
	}
	bool Blob::ReAlloc(long sz_x, long sz_y)
	{
		if(yList_f)
		{
			//_aligned_free(yList_f);
			g_pMManager->pem_aligned_free(yList_f);
			yList_f=NULL;
		}
		if(yList_b)
		{
			//_aligned_free(yList_b);
			g_pMManager->pem_aligned_free(yList_b);
			yList_b=NULL;
		}
		/*yList_f = (ptRLE_Y *)_aligned_malloc(sz_y*sizeof(ptRLE_Y), 16);
		yList_b = (ptRLE_Y *)_aligned_malloc(sz_y*sizeof(ptRLE_Y), 16);*/
		yList_f = (ptRLE_Y *)g_pMManager->pem_aligned_malloc(sz_y * sizeof(ptRLE_Y), 16, (PCHAR)__FUNCTION__, __LINE__);
		yList_b = (ptRLE_Y *)g_pMManager->pem_aligned_malloc(sz_y * sizeof(ptRLE_Y), 16, (PCHAR)__FUNCTION__, __LINE__);

		bufSz_x = sz_x;
		BufSz_y = sz_y;

		int nx = sz_x % 16;
		int ny = sz_y % 16;
		int nMapSz = (sz_x + nx) * (sz_y + ny);
		AllocSz = nMapSz;
		if (LabelTmp)
		{
			//_aligned_free(LabelTmp);
			g_pMManager->pem_aligned_free(LabelTmp);
			LabelTmp = NULL;
		}
		if (Label)
		{
			//_aligned_free(Label);
			g_pMManager->pem_aligned_free(Label);
			Label = NULL;
		}
		if (RLE_f)
		{
			//_aligned_free(RLE_f);
			g_pMManager->pem_aligned_free(RLE_f);
			RLE_f = NULL;
		}
		if (RLE_b)
		{
			//_aligned_free(RLE_b);
			g_pMManager->pem_aligned_free(RLE_b);
			RLE_b = NULL;
		}
		if (ptHoleLabel)
		{
			//_aligned_free(ptHoleLabel);
			g_pMManager->pem_aligned_free(ptHoleLabel);
			ptHoleLabel = NULL;
		}
// 
// 		bufSz_x = sz_x;
// 		BufSz_y = sz_y;
// 
// 		int nx = sz_x % 16;
// 		int ny = sz_y % 16;
// 
// 		int nMapSz = (sz_x + nx) * (sz_y + ny);
// 		AllocSz = nMapSz;
// 
		LabelTmp = (label_t *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(label_t), 16, (PCHAR)__FUNCTION__, __LINE__);
		Label = (label_t *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(label_t), 16, (PCHAR)__FUNCTION__, __LINE__);
		RLE_f = (ptRLE *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(ptRLE), 16, (PCHAR)__FUNCTION__, __LINE__);
		RLE_b = (ptRLE *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(ptRLE), 16, (PCHAR)__FUNCTION__, __LINE__);
		ptHoleLabel = (int *)g_pMManager->pem_aligned_malloc(nMapSz * sizeof(int), 16, (PCHAR)__FUNCTION__, __LINE__);
// 
        memset(&RLE_f[0], 0, sizeof(ptRLE) * nMapSz);
		memset(&RLE_b[0], 0, sizeof(ptRLE) * nMapSz);
		memset(&LabelTmp[0], 0, sizeof(label_t) * nMapSz);
		memset(&Label[0], 0, sizeof(label_t) * nMapSz);
		memset(&ptHoleLabel[0], 0, sizeof(label_t) * nMapSz);
		return true;
	}

	void Blob::Free()
	{
		if(yList_f)
		{
			//_aligned_free(yList_f);
			g_pMManager->pem_aligned_free(yList_f);
			yList_f=NULL;
		}
		if(yList_b)
		{
			//_aligned_free(yList_b);
			g_pMManager->pem_aligned_free(yList_b);
			yList_b=NULL;
		}
		if(LabelTmp)
		{
			//_aligned_free(LabelTmp);
			g_pMManager->pem_aligned_free(LabelTmp);
			LabelTmp=NULL;
		}
		if(Label)
		{
			//_aligned_free(Label);
			g_pMManager->pem_aligned_free(Label);
			Label=NULL;
		}
		if(RLE_f)
		{
			//_aligned_free(RLE_f);
			g_pMManager->pem_aligned_free(RLE_f);
			RLE_f=NULL;
		}
		if(RLE_b)
		{
			//_aligned_free(RLE_b);
			g_pMManager->pem_aligned_free(RLE_b);
			RLE_b=NULL;
		}


		if(m_label)
		{
			//_aligned_free(m_label);
			g_pMManager->pem_aligned_free(m_label);
			m_label=NULL;
		}
		if(m_remove)
		{
			//_aligned_free(m_remove);
			g_pMManager->pem_aligned_free(m_remove);
			m_remove=NULL;
		}
		if(m_area)
		{
			//_aligned_free(m_area);
			g_pMManager->pem_aligned_free(m_area);
			m_area=NULL;
		}
		if(m_min_x)
		{
			//_aligned_free(m_min_x);
			g_pMManager->pem_aligned_free(m_min_x);
			m_min_x=NULL;
		}
		if(m_min_y)
		{
			//_aligned_free(m_min_y);
			g_pMManager->pem_aligned_free(m_min_y);
			m_min_y=NULL;
		}
		if(m_max_x)
		{
			//_aligned_free(m_max_x);
			g_pMManager->pem_aligned_free(m_max_x);
			m_max_x=NULL;
		}
		if(m_max_y)
		{
			//_aligned_free(m_max_y);
			g_pMManager->pem_aligned_free(m_max_y);
			m_max_y=NULL;
		}
		if(m_cx)
		{
			//_aligned_free(m_cx);
			g_pMManager->pem_aligned_free(m_cx);
			m_cx=NULL;
		}
		if(m_cy)
		{
			//_aligned_free(m_cy);
			g_pMManager->pem_aligned_free(m_cy);
			m_cy=NULL;
		}
		if(m_min_x_y)
		{
			//_aligned_free(m_min_x_y);
			g_pMManager->pem_aligned_free(m_min_x_y);
			m_min_x_y=NULL;
		}
		if(m_min_y_x)
		{
			//_aligned_free(m_min_y_x);
			g_pMManager->pem_aligned_free(m_min_y_x);
			m_min_y_x=NULL;
		}
		if(m_max_x_y)
		{
			//_aligned_free(m_max_x_y);
			g_pMManager->pem_aligned_free(m_max_x_y);
			m_max_x_y=NULL;
		}
		if(m_max_y_x)
		{
			//_aligned_free(m_max_y_x);
			g_pMManager->pem_aligned_free(m_max_y_x);
			m_max_y_x=NULL;
		}
		if(ptHoleLabel)
		{
			//_aligned_free(ptHoleLabel);
			g_pMManager->pem_aligned_free(ptHoleLabel);
			ptHoleLabel=NULL;
		}
			if(m_perimeter)
		{
			//_aligned_free(m_perimeter);
			g_pMManager->pem_aligned_free(m_perimeter);
			m_perimeter=NULL;
	}
		if(m_size_x)
		{
			//_aligned_free(m_size_x);
			g_pMManager->pem_aligned_free(m_size_x);
			m_size_x=NULL;
		}
		if(m_size_y)
		{
			//_aligned_free(m_size_y);
			g_pMManager->pem_aligned_free(m_size_y);
			m_size_y=NULL;
		}
	}
	void Blob::GetPolygon(UCHAR * uArrDst, int nWidth, int nHeight, int nStartX, int nStartY, vector<vector<POINTF>>* vPolygons, vector<POINTF>* vPolyCenter)
	{
		int BoarderSize = 1;
		cv::Mat imgResult(nHeight, nWidth, CV_8UC1, uArrDst);
		int nBlobIdxLength = m_nBlobCount + 1;
		//cv::Mat testMat2; - 확인용
		//cv::cvtColor(imgResult, testMat2, cv::COLOR_GRAY2BGR);- 확인용
		for (int i = 1; i < nBlobIdxLength; i++)
		{
			if (m_remove[i] != 0)
				continue;
			int minX = m_min_x[i];
			int maxX = m_max_x[i] + 1;
			int minY = m_min_y[i];
			int maxY = m_max_y[i] + 1;
			cv::Mat imgRoi = imgResult(cv::Range(minY, maxY), cv::Range(minX, maxX));
			cv::Mat imgRoiBoarder;
			cv::copyMakeBorder(imgRoi, imgRoiBoarder, BoarderSize, BoarderSize, BoarderSize, BoarderSize, cv::BORDER_CONSTANT, cv::Scalar(0));
			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
			// RETR_EXTERNAL : 가장 자리 외곽선만, CHAIN_APPROX_SIMPLE : 좌표 개수 혹은 정확도를 조절하는 알고리즘 선택 중 1
			cv::findContours(imgRoiBoarder, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			std::vector<cv::Point> targetcontours;
			for (int j = 0; j < contours.size(); j++)
			{
				if (targetcontours.size() < contours[j].size())// 폴리곤좌표가 가장 길게 나온게 원하는 PolyGon이라고 가정
					targetcontours = contours[j];
			}
			int nPolyLength = targetcontours.size();
			if (nPolyLength > 0) 
			{
				vector<POINTF> vPolygon;
				//vector<cv::Point> testVect; - 확인용
				float fPolyCenterX = 0, fPolyCenterY = 0;
				for (int idx = 0; idx < nPolyLength; idx++) 
				{
					cv::Point vVector = targetcontours[idx];
					POINTF ptfVector;
					ptfVector.x = vVector.x + minX + nStartX - BoarderSize;
					ptfVector.y = vVector.y + minY + nStartY - BoarderSize;
					vPolygon.push_back(ptfVector);
					fPolyCenterX += ptfVector.x;
					fPolyCenterY += ptfVector.y;
					//cv::Point ptTest(ptfVector.x - nStartX, ptfVector.y - nStartY);
					//testVect.push_back(ptTest);- 확인용
				}
				//cv::polylines(testMat2, testVect, true, cv::Scalar(0, 0, 255), 1, 8); - 확인용
				POINTF ptfPolyCenter;
				ptfPolyCenter.x = fPolyCenterX / nPolyLength;
				ptfPolyCenter.y = fPolyCenterY / nPolyLength;
				vPolyCenter->push_back(ptfPolyCenter);
				vPolygons->push_back(vPolygon);
			}
		}
	}
	int Blob::RunLengthEncoding(BYTE * bImage, long sz_x, long sz_y, long pitch)
	{
		m_nBlobRealCount = m_nBlobCount = 0;

		if(bImage==NULL || sz_x<=0 || sz_y<=0 || pitch<=0)
			return 0;

		size_x = sz_x;
		size_y = sz_y;


		// 변수 타입과 반복횟수를 int타입으로 변환
		int sumCnt(sizeof(RLE_t));
		int nIntSzX = sz_x / sumCnt;
		int nIntPit = pitch / sumCnt;
		RLE_t * bImg = (RLE_t *)bImage;

		// 0: background
		// 1: foreground
		int v[2]={1, 1};
		ptRLE * rle[2] = {&RLE_b[0], &RLE_f[0]};
		ptRLE_Y * yList[2] = {&yList_b[0], &yList_f[0]};

		for(int y=0, idx=0, idx2=0; y<sz_y; y++, idx2+=pitch)
		{
			bImg = (RLE_t *)&bImage[idx2];
			register RLE_t curVal = bImg[0]&0x01;

			rle[curVal][v[curVal]].st = 0;

			yList[curVal][y].st = v[curVal];
			yList[1-curVal][y].st = v[1-curVal];
			int x(0), rx(0);

			for(; x<nIntSzX; x++, rx+=sumCnt)
			{
				RLE_t val = bImg[x];

				if(curVal != (val&0x01))
				{
					int n = curVal;
					rle[n][v[n]++].ed = rx-1;
					rle[1-n][v[1-n]].st = rx;
					curVal = val&0x01;
				}
				val>>=8;
				if(curVal != (val&0x01))
				{
					int n = curVal;
					rle[n][v[n]++].ed = rx;
					rle[1-n][v[1-n]].st = rx+1;
					curVal = val&0x01;
				}
				val>>=8;
				if(curVal != (val&0x01))
				{
					int n = curVal;
					rle[n][v[n]++].ed = rx+1;
					rle[1-n][v[1-n]].st = rx+2;
					curVal = val&0x01;
				}
				val>>=8;
				if(curVal != (val&0x01))
				{
					int n = curVal;
					rle[n][v[n]++].ed = rx+2;
					rle[1-n][v[1-n]].st = rx+3;
					curVal = val&0x01;
				}
			}

			// 4byte 단위가 아닌 데이터 처리
			if(rx < sz_x)
			{
				int nSum = sz_x - rx;
				RLE_t val = bImg[x];
				if(curVal != (val&0x01))
				{
					int n = curVal;
					rle[n][v[n]++].ed = rx-1;
					rle[1-n][v[1-n]].st = rx;
					curVal = val&0x01;
				}
				val>>=8;
				rx++;

				if(rx < sz_x && curVal != (val&0x01))
				{
					int n = curVal;
					rle[n][v[n]++].ed = rx-1;
					rle[1-n][v[1-n]].st = rx;
					curVal = val&0x01;
				}
				val>>=8;
				rx++;

				if(rx < sz_x && curVal != (val&0x01))
				{
					int n = curVal;
					rle[n][v[n]++].ed = rx-1;
					rle[1-n][v[1-n]].st = rx;
					curVal = val&0x01;
				}
				val>>=8;
				rx++;
			}

			int n = curVal;
			rle[n][v[n]].ed = sz_x-1;
			yList[n][y].ed = v[n]++;
			yList[1-n][y].ed = v[1-n]-1;
		}

		RleCnt_b = v[0];
		RleCnt_f = v[1];

		return RleCnt_f;
	}

	int Blob::ConnectedComponentLabeling()
	{
#ifdef USE_CONNECTIVITY_NET
		Construct_Connectivity_Net();
#else
		m_nBlobRealCount = m_nBlobCount = MarkLabel(RLE_f, RleCnt_f, yList_f, size_y);
#endif
		return m_nBlobCount;
	}

	int Blob::MarkLabel(ptRLE * RLE, int nEncCnt, ptRLE_Y * yList, int sz_y)
	{
		// RLE		: RLE 영상 시작 포인터
		// nEncCnt	: RLE 데이터 개수
		// yList	: RLE데이터를 y축으로 나눠주는 데이터(멤버변수)
		// sz_y		: y축 크기

		int curLabel=0;

		label_t * nLabelTmp = &LabelTmp[0];
		for(int i=yList[0].st; i<=yList[0].ed; i++)
		{
			RLE[i].label = ++curLabel;
			nLabelTmp[curLabel] = curLabel;
		}

		int idxBuf[1000];
		int nStPreY(0);
		for(int y = 1; y < sz_y; y++)
		{
			int nStY = yList[y].st;
			int nEdy = yList[y].ed;

			nStPreY = yList[y-1].st;
			int nEdPrey = yList[y-1].ed;
			int nMaxInd(nStPreY);
			for(int i=nStY, n=0; i<=nEdy; i++, n++)
			{
				int nSt = RLE[i].st - 1;
				int nEd = RLE[i].ed + 1;

				if(nSt>nEd)
					continue;

				int nCmpCnt(0);
				int nMin(0x7fffffff), MinInd(0);

				// Y_ToThink(2013-10-24):
				// Merge-Part에서 같은 라벨이 Merge되는 경우는 Hole이다.
				// 여기서 홀의 개수가 나오고 아울러 FillHole이 가능할지 모른다.

				for(int j=nMaxInd; j<=nEdPrey; j++)
				{
					int nSt_c = RLE[j].st;
					int nEd_c = RLE[j].ed;
					if(nEd_c < nSt || nSt_c > nEd_c)
					{
						nMaxInd = j+1;
						continue;
					}

					if( (nSt >= nSt_c && nSt <= nEd_c) ||
						(nEd >= nSt_c && nEd <= nEd_c) ||
						(nSt <  nSt_c && nEd >  nEd_c) )
					{
						if(nMaxInd < j)
							nMaxInd = j;
						int lb = (*(RLE+j)).label;

						idxBuf[nCmpCnt] = lb;
						nCmpCnt++;

						int b = lb;
						while(nLabelTmp[b] != b)
							b = nLabelTmp[b];

						if(nMin > b)
						{
							nMin = b;
							MinInd = lb;
						}
					}
					else if(nSt_c > nEd)
						break;
				}

				if(nMin != 0x7fffffff)
				{
					for(int j=0; j<nCmpCnt; j++)
					{
						int b = idxBuf[j];
						while(nLabelTmp[b] != b)
						{
							int t = nLabelTmp[b];
							nLabelTmp[b] = nMin;
							b = t;
						}
						nLabelTmp[b] = nMin;
					}
					RLE[i].label = nMin;
				}
				else
				{
					RLE[i].label = ++curLabel;
					nLabelTmp[curLabel] = curLabel;
				}
			}
		}

		// flatten
		int cntLabel(1);
		for (int i=1; i<=curLabel; i++)
			nLabelTmp[i] = nLabelTmp[nLabelTmp[i]];

		for(int i=1; i<nEncCnt; i++)
			RLE[i].label = nLabelTmp[RLE[i].label];

		// re-labeling
		int n(1);
		for(int i=1; i<=curLabel; i++)
		{
			if(nLabelTmp[i]>0 && nLabelTmp[i] == i)
			{
				Label[n] = -1;
				nLabelTmp[i]=n++;
			}
		}

		if(n==1)
			return 0;

		for(int i=1; i<nEncCnt; i++)
		{
			int idx = nLabelTmp[RLE[i].label];
			RLE[i].label = idx;

			if(Label[idx]<0)
				Label[idx] = i;
		}

		return n-1;
	}

	void Blob::CalculateFeatures(bool bCalcBlobSize)
	{
		label_t * nLabelTmp = &LabelTmp[0];

		std::set<int> LabelSet;

		std::vector<int> minX, minY, maxX, maxY;
		if(bCalcBlobSize)
		{
			minX.resize(m_nBlobCount+1, size_x);
			minY.resize(m_nBlobCount+1, size_y);
			maxX.resize(m_nBlobCount+1, 0);
			maxY.resize(m_nBlobCount+1, 0);
		}
		for(int y = 0; y < size_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int n = RLE_f[i].label;

				if(n<0 || n>=AllocSz)
					continue;
				if(n>=PTR_BLOB_MAX)
				{

					m_nBlobCount = PTR_BLOB_MAX;
					return;
				}

				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				int cnt = ed - st + 1;

				if(bCalcBlobSize)
				{
					minX[n] = st < minX[n] ? st : minX[n];
					minY[n] = y < minY[n] ? y : minY[n];
					maxX[n] = ed > maxX[n] ? ed : maxX[n];
					maxY[n] = y > maxY[n] ? y : maxY[n];
				}
				//	if(i != Label[n])
				if(LabelSet.find(n)!=LabelSet.end())
				{
					m_area[n]+=cnt;
					m_cx[n] += (cnt * (ed + st))/2;
					m_cy[n] += cnt * y;

					if(m_min_x[n] > st)
					{
						m_min_x[n] = st;
						m_min_x_y[n]= y;
					}
					if(m_max_x[n] < ed)
					{
						m_max_x[n] = ed;
						m_max_x_y[n] = y;
					}

					if(m_min_y[n] > y)
					{
						m_min_y[n] = y;
						m_min_y_x[n] = st;
					}
					if(m_max_y[n] < y)
					{
						m_max_y[n] = y;
						m_max_y_x[n] = ed;
					}
				}
				else
				{
					LabelSet.insert(n);
					m_label[n]= n;
					m_remove[n]= 0;
					m_area[n]	=cnt;
					m_cx	[n]= (cnt * (ed + st))/2;
					m_cy	[n]= cnt * y;
					m_min_x[n]= st;
					m_max_x[n]= ed;
					m_min_y[n]= y;
					m_max_y[n]= y;
					m_perimeter[n]=0;
					m_size_x[n]=0;
					m_size_y[n]=0;
				}
			}
		}
		if(bCalcBlobSize)
		{
			for(int i=1; i<=m_nBlobCount; i++)
			{
				m_size_x[i] = maxX[i] - minX[i] + 1;
				m_size_y[i] = maxY[i] - minY[i] + 1;
			}
		}

		// 		for(int i=1; i<=m_nBlobCount; i++)
		// 		{
		// 			nResult[i].cx /= (double)(Result[i].area);
		// 			nResult[i].cy /= (double)(Result[i].area);
		// 		}
	}



	void Blob::FillHoles(BYTE * bImage, long sz_x, long sz_y, long pitch)
	{
// 		Construct_Connectivity_Net();
// 		Recur_Fill_Holes( bImage, sz_x, sz_y, pitch);
		if(sz_y>BufSz_y || sz_x > bufSz_x)
			ReAlloc(sz_x,sz_y);
		RunLengthEncoding(bImage, sz_x, sz_y, pitch);
		int number=ConnectedComponentLabeling_both();
		CalculateFeatures();
		
		std::set<int> setLabels;
		for (int i = 0; i < ptHoleLabelCnt; i++)
		{
			setLabels.insert((long)ptHoleLabel[i]);
		}
		DrawHoleLabels((UCHAR*)bImage, (long)sz_x, (long)sz_y, (long)pitch, setLabels, 255);
		setLabels.clear();

	}
	void Blob::DrawContours(BYTE * bImage, long sz_x, long sz_y, long pitch, long nVal)
	{
		/*std::map<int, CPoint> stList;

		for(int y=0; y<sz_y; y++)
		{
		int st_b = yList_f[y].st;
		int ed_b = yList_f[y].ed;

		for(int i=st_b; i<=ed_b; i++)
		{
		int st = RLE_f[i].st;
		int ed = RLE_f[i].ed;

		if(m_remove[RLE_f[i].label]==0)
		{
		if(stList.find(RLE_f[i].label)==stList.end() && ed>=st)
		{
		stList.insert(std::make_pair(RLE_f[i].label, CPoint(st,y)));
		}

		for(int x=st; x<=ed; x++)
		{
		bImage[y*pitch+x] = 255;
		}
		}
		}
		}

		std::map<int, CPoint>::iterator itPtList;
		for(itPtList=stList.begin(); itPtList!=stList.end(); ++itPtList)
		{
		Contour(itPtList->second.y, itPtList->second.x, bImage, sz_x, sz_y, pitch, nVal);
		}*/
		std::map<int, CPoint> stList;

		cv::Mat lbImg = cv::Mat::zeros(sz_y, sz_x, CV_MAKE_TYPE(sizeof(label_t), 1));
		label_t * pLabel = lbImg.ptr<label_t>();

		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(PTR_BLOB_MAX > RLE_f[i].label&&m_remove[RLE_f[i].label]==0)
				{
					if(stList.find(RLE_f[i].label)==stList.end() && ed>=st)
					{
						stList.insert(std::make_pair(RLE_f[i].label, CPoint(st,y)));
					}

					for(int x=st; x<=ed; x++)
					{
						pLabel[y*pitch+x] = RLE_f[i].label;
					}
				}
			}
		}


		cv::Mat ResImg(sz_y, sz_x, CV_8UC1, bImage, pitch);
		ResImg = 0;

		std::map<int, CPoint>::iterator itPtList;
		for(itPtList=stList.begin(); itPtList!=stList.end(); ++itPtList)
		{
			Contour(itPtList->second.y, itPtList->second.x, pLabel, ResImg.ptr(), ResImg.cols, ResImg.rows, ResImg.cols, itPtList->first);
		}

		cv::threshold(ResImg, ResImg, 0, nVal, cv::THRESH_BINARY);
	}
	void Blob::DrawLabelContours(BYTE * bImage, long sz_x, long sz_y, long pitch, long nVal)
	{
		std::map<int, CPoint> stList;

		cv::Mat lbImg = cv::Mat::zeros(sz_y, sz_x, CV_MAKE_TYPE(sizeof(label_t), 1));
		label_t * pLabel = lbImg.ptr<label_t>();

		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(PTR_BLOB_MAX > RLE_f[i].label&&m_remove[RLE_f[i].label]==0)
				{
					if(stList.find(RLE_f[i].label)==stList.end() && ed>=st)
					{
						stList.insert(std::make_pair(RLE_f[i].label, CPoint(st,y)));
					}

					for(int x=st; x<=ed; x++)
					{
						pLabel[y*pitch+x] = RLE_f[i].label;
						bImage[y*pitch+x] = 255;
					}
				}
			}
		}
	

		cv::Mat EdgeImg(sz_y, sz_x, CV_8UC1, cv::Scalar(0));

		std::map<int, CPoint>::iterator itPtList;
		for(itPtList=stList.begin(); itPtList!=stList.end(); ++itPtList)
		{
			Contour(itPtList->second.y, itPtList->second.x, pLabel, EdgeImg.ptr(), EdgeImg.cols, EdgeImg.rows, EdgeImg.cols, itPtList->first);
		}

		for (int y=0; y<sz_y; y++)
		{
			int resIdx = y*pitch;
			BYTE * edgePtr = EdgeImg.ptr(y);
			for (int x=0; x<sz_x; x++)
			{
				if(edgePtr[x] != 0)
					bImage[resIdx+x] = nVal;
			}
		}
	}
	void Blob::CalcPerimeter(BYTE * bImage, USHORT * bLabel, long sz_x, long sz_y, long pitch, int& width)
	{
		std::map<int, CPoint> stList;

		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(PTR_BLOB_MAX > RLE_f[i].label&&m_remove[RLE_f[i].label]==0)
				{
					for(int x=st; x<=ed; x++)
					{
						//LabelTmp[y*pitch+x] = RLE_f[i].label;
						bLabel[y*pitch+x] = RLE_f[i].label;
					}

					if(stList.find(RLE_f[i].label)==stList.end() && ed>=st)
					{
						stList.insert(std::make_pair(RLE_f[i].label, CPoint(st,y)));
					}
				}
			}
		}


		std::map<int, CPoint>::iterator itPtList;

		long label = 1;

		for(itPtList=stList.begin(); itPtList!=stList.end(); ++itPtList)
		{			
			//PerimeterContour(itPtList->second.y, itPtList->second.x, /*bLabel*/ LabelTmp ,bImage, sz_x, sz_y, pitch, label);
			PerimeterContour(itPtList->second.y, itPtList->second.x, bLabel ,bImage, sz_x, sz_y, pitch, label, width);
			label++;
		}
	}
	void Blob::PerimeterContour(int sy, int sx, USHORT * bLabel, BYTE * bImage, long sz_x, long sz_y, long pitch, long label, int& width)
	{
		bool bFirst(false), bFoundNew(false);
		int cx(sx), cy(sy), dir(0), px(sx), py(sy);
		TCHAR prev(0);
		int nChainCodeMap[8][2] = { {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1} };
	
		int n_dirCount(0);

		float perimeter = 0.0;

		std::vector<cv::Point> pt;

		cv::Point tmpPt;

		std::vector<std::pair<TCHAR, cv::Point>> stPtr;
		std::pair<TCHAR, cv::Point> tmpPir;

		std::vector<float> dist;

		bool sameDir(false);

		float nWidth = 0.0;

		do
		{
			cx=px;
			cy=py;
			bFoundNew = false;
			for (int i = 0; i < 7; i++)
			{
				int y = cy + nChainCodeMap[dir][0];
				int x = cx + nChainCodeMap[dir][1];

				if(x<0 || y<0 || x>=sz_x || y>=sz_y)
					dir = (dir + 1) % 8;
				else if(bLabel[y*pitch+x] == label)
				{
					bFirst=true;
					TCHAR cur = dir;

					if(bImage[y*pitch+x]==0)
					{
						bFoundNew=true;
						perimeter+= (dir==0 || dir==4 || dir==2 || dir==6) ? 1 : 1.414;

						tmpPt.x = x;
						tmpPt.y = y;;
						pt.push_back(tmpPt);
					}
					// 이전 엣지로 돌아갈 때
					if(((prev+4)%8)==cur)
					{
						bImage[cy*pitch+cx] = 0x0f;
					}
					// 이전 엣지로 돌아갈 때
					else if(bImage[cy*pitch+cx]>0)
					{
						// 두번 이상 참조 될 때에는 자신으로 온  체인코드를 가지고
						// 반시계방향으로 돌면서 이전 엣지를 다시 찾는다.
						// 다시 찾은 inner-code가 outer-code와 같으면 지운다.
						// 다르면 해당 방향을 기록한다.
						// 다시 돌아가서 찾은 체인코드는 반대방향의 체인코드(prev+4 처럼)로 전환하지 않아야 한다.
						
						perimeter+= (dir==0 || dir==4 || dir==2 || dir==6) ? 1 : 1.414;
						
						if((((prev+4)%8)-cur)>=0)
						{
							bImage[cy*pitch+cx] |= (0x40 | 0x1f);
						}
						else
						{
							bImage[cy*pitch+cx] |= ((((prev+4)%8)-cur)&0x80) | 0x1f;
						}
						sameDir = false;
					}
					else
					{
						// 이전 체인코드에서 현재 체인코드를 빼서
						// 음수가 나오면 블랍의 시작점이다.
						//
						// 음수라는 것을 알리는 사인비트를 에지 코드에 or연산으로 남긴다.
						// 
						// (prev+4)는 이전 엣지에서 계산된 현재 엣지의 체인코드를 가지고
						// 현재 엣지에서 이전엣지 위치의 체인코드를 역으로 계산한다.
						//
						// cur은 현재 엣지에서 다음 엣지의 체인코드
						//
						// 어디에서 왔고 어디로 가느냐를 계산
						// 어디에서 왔냐에서 어디로 가느냐를 뺀다.
						// 음수가 나오면 시작엣지

						if((((prev+4)%8)-cur)>=0)
						{
							bImage[cy*pitch+cx] |= (0x40 | 0x1f);
						}
						else
						{
							bImage[cy*pitch+cx] |= ((((prev+4)%8)-cur)&0x80) | 0x1f;
							sameDir =false;
						}


						//외곽선 찾으면서 벡터 추출
						if(cur == prev)
						{
							sameDir=true;
							n_dirCount++;
					}
						else
						{
							sameDir = false;
							n_dirCount = 0;
						}
						
						if(n_dirCount >= 2 && sameDir == true)
						{
							cv::Point ptr;
							ptr.x = px;
							ptr.y = py;
							tmpPir = std::make_pair(prev, ptr);
							stPtr.push_back(tmpPir);
							sameDir = false;
							n_dirCount = 0;
						}
						
					}
					prev = cur;
					px = x;
					py = y;
					break;
				}
				else
					dir = (dir + 1) % 8;
			}
			dir = (prev+6)%8;	// 다음 시작점

			// 			nSearchCnt++;
			// 			if(nSearchCnt >= nMaxCnt)
			// 				break;
		} while ((cx != sx) || (cy != sy) || (bFoundNew==true));

		// 한 픽셀일 때는 끝 엣지로 한다.
		if(bFirst==false)
		{
			bImage[sy*pitch+sx]=0x0f;
			perimeter++;

			tmpPt.x = sx;
			tmpPt.y = sy;;
			pt.push_back(tmpPt);
		}

		if(perimeter >= 2.0)
			perimeter /= 2.0;

		if(pt.size() > 0)
		{
			cv::RotatedRect rt = cv::minAreaRect(pt);
			cv::Size2f rtSize = rt.size;

			m_size_x[label] = rtSize.width;
			m_size_y[label] = rtSize.height;
			
			pt.clear();
		}

		//일정한 벡터가없다고 판단, 1/2 Perimeter return
		if(stPtr.size() == 0)
		{
			m_perimeter[label] = perimeter;
			return;
	}

		int searchDir[8] = {0}; 

		for(int i =0; i < stPtr.size(); i++)
	{
			int tmp = stPtr[i].first;
			searchDir[tmp]++;
		}

		int maxValue = searchDir[0];
		int maxDir = 0;
		for(int j=0; j < 8; j++)
		{
			if(maxValue < searchDir[j])
			{
				maxValue = searchDir[j];
				maxDir= j;
			}
		}

		std::vector<int> selectDir;
		for(int i=0; i< stPtr.size(); i++)
		{
			if(stPtr[i].first == maxDir) selectDir.push_back(i);
		}

		for(int i=0; i<selectDir.size(); i++)
		{
			int y = stPtr[selectDir[i]].second.y;
			int x = stPtr[selectDir[i]].second.x;
			int reDir = (maxDir +2) % 8 ;
			px = x;
			py = y;
			do 
			{				
				cy = py + nChainCodeMap[reDir][0];
				cx = px + nChainCodeMap[reDir][1];

				if(cx >= sz_x || cx < 0 ) break;
				if(cy >= sz_y || cy < 0 ) break;
				

				int leftv = reDir-1;
				if(leftv < 0)leftv=7;

				//직선방향에 대응되는 픽셀이 없을 경우
				//-1방향 픽셀
				int ly = py + nChainCodeMap[leftv][0];
				int lx = px + nChainCodeMap[leftv][1];

				//+1방향 픽셀
				int ry = py + nChainCodeMap[(reDir + 1)%8][0];
				int rx = px + nChainCodeMap[(reDir + 1)%8][1];

				//각 방향 경계처리
				(lx < 0) ? 0 : lx;
				(rx < 0) ? 0 : rx;
				(ly < 0) ? 0 : ly;
				(ry < 0) ? 0 : ry;
				(lx > sz_x-1) ? sz_x-1 : lx;
				(rx > sz_x-1) ? sz_x-1 : rx;
				(ly > sz_y-1) ? sz_y-1 : ly;
				(ry > sz_y-1) ? sz_y-1 : ry;

				if(bImage[cy * pitch + cx] > 14 && bLabel[cy * pitch + cx]==label)
				{	
					float tmpDist = sqrt(std::pow((x - cx),2.0) + std::pow((y - cy), 2.0));
					dist.push_back(tmpDist);
					break;

				}else if(bImage[ly * pitch + lx] > 14 && bLabel[ly * pitch + lx]==label)
				{
					float tmpDist = sqrt(std::pow((x - lx),2.0) + std::pow((y - ly), 2.0));
					if(tmpDist > 2.0)
					{
						dist.push_back(tmpDist);
						break;
					}
				}
				else if(bImage[ry * pitch + rx] > 14 && bLabel[ry * pitch + rx]==label)
				{
					float tmpDist = sqrt(std::pow((x - rx),2.0) + std::pow((y - ry), 2.0));
					if(tmpDist > 2.0)
					{
						dist.push_back(tmpDist);
						break;
					}
				}
				px = cx;
				py = cy;
			} while ((sz_x-1 >= cx) && (sz_y-1 >= cy) && (cx >= 0)&& (cy >= 0));

		}

		if(dist.size() > 0)
		{
			std::sort(dist.begin(), dist.end());
			nWidth = dist[(dist.size() / 2)];
			m_perimeter[label] = perimeter - nWidth ;
		}
		else
		{
			m_perimeter[label] = perimeter;
		}
		width = nWidth;
		dist.clear();
		stPtr.clear();
		selectDir.clear();
	}


	void Blob::Contour(int sy, int sx, label_t * bLabel, BYTE * bImage, long sz_x, long sz_y, long pitch, long label)
	{
		bool bFirst(false), bFoundNew(false);
		int cx(sx), cy(sy), dir(0), px(sx), py(sy);
		TCHAR prev(0);
		int nChainCodeMap[8][2] = { {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1} };
		//		int nMaxCnt = Result[label].area;
		int nSearchCnt(0);

		do
		{
			cx=px;
			cy=py;
			bFoundNew = false;
			for (int i = 0; i < 7; i++)
			{
				int y = cy + nChainCodeMap[dir][0];
				int x = cx + nChainCodeMap[dir][1];

				if(x<0 || y<0 || x>=sz_x || y>=sz_y)
					dir = (dir + 1) % 8;
				else if(bLabel[y*pitch+x] == label)
				{
					bFirst=true;
					TCHAR cur = dir;

					if(bImage[y*pitch+x]==0)
						bFoundNew=true;

					// 이전 엣지로 돌아갈 때
					if(((prev+4)%8)==cur)
						bImage[cy*pitch+cx] = 0x0f;
					// 이전 엣지로 돌아갈 때
					else if(bImage[cy*pitch+cx]>0)
					{
						// 두번 이상 참조 될 때에는 자신으로 온  체인코드를 가지고
						// 반시계방향으로 돌면서 이전 엣지를 다시 찾는다.
						// 다시 찾은 inner-code가 outer-code와 같으면 지운다.
						// 다르면 해당 방향을 기록한다.
						// 다시 돌아가서 찾은 체인코드는 반대방향의 체인코드(prev+4 처럼)로 전환하지 않아야 한다.
						if((((prev+4)%8)-cur)>=0)
						{
							bImage[cy*pitch+cx] |= (0x40 | 0x1f);
						}
						else
						{
							bImage[cy*pitch+cx] |= ((((prev+4)%8)-cur)&0x80) | 0x1f;
						}
					}
					else
					{
						// 이전 체인코드에서 현재 체인코드를 빼서
						// 음수가 나오면 블랍의 시작점이다.
						//
						// 음수라는 것을 알리는 사인비트를 에지 코드에 or연산으로 남긴다.
						// 
						// (prev+4)는 이전 엣지에서 계산된 현재 엣지의 체인코드를 가지고
						// 현재 엣지에서 이전엣지 위치의 체인코드를 역으로 계산한다.
						//
						// cur은 현재 엣지에서 다음 엣지의 체인코드
						//
						// 어디에서 왔고 어디로 가느냐를 계산
						// 어디에서 왔냐에서 어디로 가느냐를 뺀다.
						// 음수가 나오면 시작엣지

						if((((prev+4)%8)-cur)>=0)
						{
							bImage[cy*pitch+cx] |= (0x40 | 0x1f);
						}
						else
						{
							bImage[cy*pitch+cx] |= ((((prev+4)%8)-cur)&0x80) | 0x1f;
						}
					}
					prev = cur;
					px = x;
					py = y;
					break;
				}
				else
					dir = (dir + 1) % 8;
			}
			dir = (prev+6)%8;	// 다음 시작점

			// 			nSearchCnt++;
			// 			if(nSearchCnt >= nMaxCnt)
			// 				break;
		} while ((cx != sx) || (cy != sy) || (bFoundNew==true));

		// 한 픽셀일 때는 끝 엣지로 한다.
		if(bFirst==false)
		{
			bImage[sy*pitch+sx]=0x0f;
		}
	}


	

//	void Blob::Contour(int sy, int sx, BYTE * bImage, long sz_x, long sz_y, long pitch, long val)
// 	{
// 		bool bFirst(false), bFoundNew(false);
// 		int cx(sx), cy(sy), dir(0), px(sx), py(sy);
// 		TCHAR prev(0);
// 		int nChainCodeMap[8][2] = { {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1} };
// 
// 		do
// 		{
// 			cx=px;
// 			cy=py;
// 			bFoundNew = false;
// 			for (int i = 0; i < 7; i++)
// 			{
// 				int y = cy + nChainCodeMap[dir][0];
// 				int x = cx + nChainCodeMap[dir][1];
// 
// 				if(x<0 || y<0 || x>=sz_x || y>=sz_y)
// 					dir = (dir + 1) % 8;
// 				else if(bImage[y*pitch+x] > 0)
// 				{
// 					bFirst=true;
// 					TCHAR cur = dir;
// 
// 					if(bImage[y*pitch+x]==255)
// 						bFoundNew=true;
// 
// 					bImage[cy*pitch+cx] = val;
// 
// 					px = x;
// 					py = y;
// 					break;
// 				}
// 				else
// 					dir = (dir + 1) % 8;
// 			}
// 			dir = (dir+6)%8;	// 다음 시작점
// 
// 		} while ((cx != sx) || (cy != sy) || (bFoundNew==true));
// 
// 		// 한 픽셀일 때는 끝 엣지로 한다.
// 		if(bFirst==false)
// 		{
// 			bImage[sy*pitch+sx]=val;
// 		}	
// 	}
// 	void Blob::Contour(int sy, int sx, USHORT * bLabel, BYTE * bImage, long sz_x, long sz_y, long pitch, long label, long val)
// 	{
// 		int cx(sx), cy(sy), dir(0);
// 		int nChainCodeMap[8][2] = { {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1} };
// 		do
// 		{
// 			for (int i = 0; i < 7; i++)
// 			{
// 				int y = cy + nChainCodeMap[dir][0];
// 				int x = cx + nChainCodeMap[dir][1];
// 
// 				if(x<0 || y<0 || x>=sz_x || y>=sz_y)
// 					dir = (dir + 1) % 8;
// 				else if(bLabel[y*pitch+x] == label)
// 				{
// 					bImage[y*pitch+x] = val;
// 					cy = y;
// 					cx = x;
// 					break;
// 				}
// 				else
// 					dir = (dir + 1) % 8;
// 			}
// 			dir = (dir + 6) % 8;
// 		} while ((cx != sx) || (cy != sy));
// 	}
// 	void Blob::FillHoleByContour(USHORT * bLabel, UCHAR * bImage, long sz_x, long sz_y, long pitch, int nLabel)
// 	{
// 		int nChainCodeMap[8][2] = { {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1} };
// 
// 		for(int y=1,idx=pitch; y<(sz_y-1); y++,idx+=pitch)
// 		{
// 			for(int x=1; x<(sz_x-1); x++)
// 			{
// 				if(bLabel[idx+x]!=nLabel && (bLabel[idx+x-1]==nLabel && (bImage[idx+x-1]==0x9f || bImage[idx+x-1]==0)))
// 				{
// 					bImage[y*pitch+x] = 0;
// 					bLabel[y*pitch+x] = nLabel;
// 				}
// 			}
// 		}
// 	}

	void Blob::DrawLabels(int * bImage, long sz_x, long sz_y, long pitch, long min_x, long min_y, long label)
	{
		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(RLE_f[i].label == label)
				{
					for(int x=st; x<=ed; x++)
					{
						bImage[(y-min_y)*pitch+(x-min_x)] = RLE_f[i].label;
					}
				}
			}
		}
	}

	void Blob::DrawLabels(USHORT * bImage, long sz_x, long sz_y, long pitch)
	{
		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(PTR_BLOB_MAX > RLE_f[i].label && m_remove[RLE_f[i].label] == 0)
				{
					for(int x=st; x<=ed; x++)
					{
						bImage[y*pitch+x] = RLE_f[i].label;
					}
				}
			}
		}
	}

	void Blob::DrawLabels(USHORT * bImage, long sz_x, long sz_y, long pitch, long label)
	{
		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(RLE_f[i].label == label)
				{
					for(int x=st; x<=ed; x++)
					{
						bImage[y*pitch+x] = label;
					}
				}
			}
		}
	}

	void Blob::DrawLabels(UCHAR * bImage, long sz_x, long sz_y, long pitch, long label, long val)
	{
		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(RLE_f[i].label == label)
				{
					for(int x=st; x<=ed; x++)
					{
						bImage[y*pitch+x] = val;
					}
				}
			}
		}
	}

	void Blob::DrawLabels(UCHAR * bImage, long sz_x, long sz_y, long pitch)
	{
		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(PTR_BLOB_MAX > RLE_f[i].label && m_remove[RLE_f[i].label] == 0)
				{
					for(int x=st; x<=ed; x++)
					{
						bImage[y*pitch+x] = RLE_f[i].label;
					}
				}
			}
		}
	}

	void Blob::DrawRemoves(BYTE * bImage, long sz_x, long sz_y, long pitch)
	{
		int label(0);

		std::vector<BYTE> rem(sz_x);

		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(PTR_BLOB_MAX > RLE_f[i].label&&m_remove[RLE_f[i].label] == 1)
				{
					memcpy(&bImage[y*pitch+st], &rem[0], sizeof(BYTE)*(ed-st+1));
// 					for(int x=st; x<=ed; x++)
// 					{
// 						bImage[y*pitch+x] = 0;
// 					}
				}
			}
		}
	}

	void Blob::Draws(BYTE * bImage, long sz_x, long sz_y, long pitch)
	{
		int label(0);


		for(int y=0; y<sz_y; y++)
		{
			int st_b = yList_f[y].st;
			int ed_b = yList_f[y].ed;

			for(int i=st_b; i<=ed_b; i++)
			{
				int st = RLE_f[i].st;
				int ed = RLE_f[i].ed;

				if(PTR_BLOB_MAX > RLE_f[i].label && m_remove[RLE_f[i].label] == 0)
				{
					for(int x=st; x<=ed; x++)
					{
						bImage[y*pitch+x] = 255;
					}
				}
			}
		}
	}

	void Blob::RemoveMinArea(int minArea)
	{
		for (int i=1; i<=m_nBlobCount; i++)
		{
			if(m_area[i]<minArea && m_remove[i]==0)
			{
				m_remove[i] = 1;
				m_nBlobRealCount--;
			}
		}
	}

	void Blob::RemoveMaxArea(int maxArea)
	{
		for (int i=1; i<=m_nBlobCount; i++)
		{
			if(m_area[i]>maxArea && m_remove[i]==0)
			{
				m_remove[i] = 1;
				m_nBlobRealCount--;
			}
		}
	}

	void Blob::RemoveLabelinv(int label)
	{
		for (int i=1; i<=m_nBlobCount; i++)
		{
			if(m_label[i]!=label && m_remove[i]==0)
			{
				m_remove[i] = 1;
				m_nBlobRealCount--;
			}
		}
	}

	void Blob::RemoveLabel(int label)
	{
		for (int i=1; i<=m_nBlobCount; i++)
		{
			if(m_label[i]==label && m_remove[i]==0)
			{
				m_remove[i] = 1;
				m_nBlobRealCount--;
			}
		}
	}
	
	void Blob::RemoveSmallObjects(int minw, int minh)
	{
		for (int i=1; i<=m_nBlobCount; i++)
		{
			if(m_size_x[i]<=minw && m_size_y[i]<=minh && m_remove[i]==0)
			{
				m_remove[i] = 1;
				m_nBlobRealCount--;
			}
		}
	}
	// 
	// 	void Blob::GetCenter(float * x, float * y)
	// 	{
	// 		if(x==NULL || x==NULL)
	// 			return;
	// 
	// 		int LabelCnt = m_nBlobCount;
	// 
	// 		for(int i=1, n=0; i<=LabelCnt; i++)
	// 		{
	// 			if(m_remove[i]==0)
	// 			{
	// 				float area = m_area[i];
	// 				if(area > 0)
	// 				{
	// 					x[n] = (float)m_cx[i] / area;
	// 					y[n] = (float)m_cy[i] / area;
	// 				}
	// 				n++;
	// 			}
	// 		}
	// 	}
	// 
	// 	void Blob::GetMinMax(long * min_x, long * min_y, long * max_x, long * max_y)
	// 	{
	// 		if(min_x==NULL || min_y==NULL || max_x==NULL || max_y==NULL)
	// 			return;
	// 
	// 		int LabelCnt = m_nBlobCount;
	// 
	// 		for(int i=1, n=0; i<=LabelCnt; i++)
	// 		{
	// 			if(m_remove[i]==0)
	// 			{
	// 				min_x[n] = m_min_x[i];
	// 				min_y[n] = m_min_y[i];
	// 				max_x[n] = m_max_x[i];
	// 				max_y[n] = m_max_y[i];
	// 				n++;
	// 			}
	// 		}
	// 	}

	void Blob::GetDiameter(double * dia)
	{
		if(dia==NULL)
			return;

		int LabelCnt = m_nBlobCount;

		for(int i=1, n=0; i<=LabelCnt; i++)
		{
			if(m_remove[i]==0)
			{
				dia[n] = sqrt(4.0 * (double)(m_area[i]) / 3.14159265359);
				n++;
			}
		}
	}

	long Blob::GetLabel(long x, long y, long * label)
	{
		if(label==NULL)
			return false;

		int ret(0);
		int st_b = yList_f[y].st;
		int ed_b = yList_f[y].ed;

		for(int i=st_b; i<=ed_b; i++)
		{
			int lb = RLE_f[i].label;
			int st = RLE_f[i].st;
			int ed = RLE_f[i].ed;

			if(m_remove[lb] == 0 && x>=st && x<=ed)
			{
				ret=lb;
				if(label)
					*label = lb;
				return ret;
			}
		}

		return ret;
	}	

#ifdef USE_CONNECTIVITY_NET
	// one-pass 방식으로 연결정보를 찾는다.
	void Blob::Construct_Connectivity_Net()
	{
		int label(0);
		for(int y=0; y<size_y; y++)
		{
			yList_f[y].front = yList_f[y].st;
			yList_f[y].rear = -1;
		}
		for(int y=0; y<size_y;)
		{
			if(yList_f[y].front>0 && yList_f[y].ed>=yList_f[y].st)
			{
				Recur_Connect_Left(yList_f[y].front, y, label);
				if(yList_f[y].front<=0)
					y++;
			}
			else
				y++;
		}

		for(int y=size_y-1; y>=0;y--)
		{
			yList_f[y].front = yList_f[y].ed;
			yList_f[y].rear = -1;
		}
		for(int y=size_y-1; y>=0;)
		{
			if(yList_f[y].front>0 && yList_f[y].ed>=yList_f[y].st)
			{
				Recur_Connect_Right(yList_f[y].front, y, label);
				if(yList_f[y].front<=0)
					y--;
			}
			else
				y--;
		}

		for(int i=1; i<=label; i++)
			LabelTmp[i] = LabelTmp[LabelTmp[i]];

		for(int i=1; i<=RleCnt_f; i++)
		{
			int idx = LabelTmp[RLE_f[i].label];
			RLE_f[i].label = idx;

		}

		m_nBlobCount = label;
	}

	void Blob::Recur_Connect_Left(int StartNode, int k, int & label)
	{
		if(StartNode <=0)
			return;
		int C_Node = StartNode;
		int lb_c(0);
		RLE_f[C_Node].label = 0;
		int nSt_c = RLE_f[C_Node].st;
		int nEd_c = RLE_f[C_Node].ed;

		int kn = k+1;
		int F_Node(-1), nSt_f(0), nEd_f(0);
		if(kn<size_y && yList_f[kn].ed>=yList_f[kn].st)
		{
			F_Node= yList_f[kn].front;
		}

		while (F_Node>0 && nSt_f<=nEd_c) 
		{
			nSt_f = RLE_f[F_Node].st;
			nEd_f = RLE_f[F_Node].ed;

			if(nEd_f < nSt_c)
			{
				Recur_Connect_Left(F_Node, kn, label);
			}
			else if((nSt_f >= nSt_c && nSt_f <= nEd_c) ||
				(nEd_f >= nSt_c && nEd_f <= nEd_c) ||
				(nSt_f <  nSt_c && nEd_f >  nEd_c) )
			{
				RLE_f[C_Node].stLink = F_Node;
				//	RLE_f[C_Node].Link[0] = F_Node;
				yList_f[k].Left(RLE_f, C_Node);
				C_Node = F_Node;

				k = kn;
				nSt_c = RLE_f[C_Node].st;
				nEd_c = RLE_f[C_Node].ed;
			}

			kn = k+1;
			if(kn>=size_y || yList_f[kn].ed<yList_f[kn].st)
				F_Node = -1;
			else
				F_Node = yList_f[kn].front;
		}

		kn = k+1;
		int R_Node(-1), nSt_r(0), nEd_r(0), nLb_r(0);
		if(kn<size_y && yList_f[kn].ed>=yList_f[kn].st && yList_f[kn].rear>0)
		{
			R_Node = yList_f[kn].rear;
			//	nLb_r = RLE_f[R_Node].label;
			nSt_r = RLE_f[R_Node].st;
			nEd_r = RLE_f[R_Node].ed;
		}
		if(R_Node<=0 || nEd_r < nSt_c)
		{
			RLE_f[C_Node].stLink = C_Node;
			//	RLE_f[C_Node].Link[0] = C_Node;
			yList_f[k].Left(RLE_f, C_Node);
		}
		else if((nSt_r >= nSt_c && nSt_r <= nEd_c) ||
			(nEd_r >= nSt_c && nEd_r <= nEd_c) ||
			(nSt_r <  nSt_c && nEd_r >  nEd_c) )
		{
			if(yList_f[k].st <= (C_Node-1))
			{
				RLE_f[C_Node].stLink = C_Node-1;
				//	RLE_f[C_Node].Link[0] = C_Node-1;
				yList_f[k].Left(RLE_f, C_Node);
			}
		}
	}

	void Blob::Recur_Connect_Right(int StartNode, int k, int & label)
	{
		if(StartNode <=0)
			return;
		int C_Node = StartNode;
		int nSt_c = RLE_f[C_Node].st;
		int nEd_c = RLE_f[C_Node].ed;

		int kn = k-1;
		int F_Node(-1), nSt_f(0), nEd_f(0);
		if(kn>=0 && yList_f[kn].ed>=yList_f[kn].st)
		{
			F_Node= yList_f[kn].front;
		}

		while (F_Node>0 && nEd_f>=nSt_c) 
		{
			nSt_c = RLE_f[C_Node].st;
			nEd_c = RLE_f[C_Node].ed;

			if(nEd_c < nSt_f)
			{
				Recur_Connect_Right(F_Node, kn, label);
			}
			else if((nSt_f >= nSt_c && nSt_f <= nEd_c) ||
				(nEd_f >= nSt_c && nEd_f <= nEd_c) ||
				(nSt_f <  nSt_c && nEd_f >  nEd_c) )
			{
				RLE_f[C_Node].edLink = F_Node;
				//	RLE_f[C_Node].Link[1] = F_Node;
				yList_f[k].Right(RLE_f, C_Node);
				C_Node = F_Node;

				k = kn;
			}

			kn = k-1;
			if(kn<0 || yList_f[kn].ed<yList_f[kn].st)
				F_Node=-1;
			else
				F_Node = yList_f[kn].front;
		}

		kn = k-1;
		int R_Node(-1), nSt_r(0), nEd_r(0), nLb_r(0);
		if(kn<size_y && yList_f[kn].ed>=yList_f[kn].st)
			if(kn>=0 && yList_f[kn].ed>=yList_f[kn].st && yList_f[kn].rear>0)
			{
				R_Node = yList_f[kn].rear;
				nSt_r = RLE_f[R_Node].st;
				nEd_r = RLE_f[R_Node].ed;
				//	nLb_r = RLE_f[R_Node].label;
			}
			if(nSt_r > nEd_c || R_Node<=0)
			{
				RLE_f[C_Node].edLink = C_Node;
				//	RLE_f[C_Node].Link[1] = C_Node;
				yList_f[k].Right(RLE_f, C_Node);
			}
			else if((nSt_r >= nSt_c && nSt_r <= nEd_c) ||
				(nEd_r >= nSt_c && nEd_r <= nEd_c) ||
				(nSt_r <  nSt_c && nEd_r >  nEd_c) )
			{
				if(yList_f[k].ed >= (C_Node+1))
				{
					RLE_f[C_Node].edLink = C_Node+1;
					//	RLE_f[C_Node].Link[1] = C_Node+1;
					yList_f[k].Right(RLE_f, C_Node);
				}
			}
	}

	void Blob::Recur_Fill_Holes()
	{

	}
#endif

	void Blob::Construct_Connectivity_Net()
	{
		_NodeYList.clear();
		_FrontNodeList.clear();
		_ReartNodeList.clear();

		//shkim setting blob node
		for(int y=0; y<size_y; y++)
		{
			std::vector<std::shared_ptr<BlobNode>> tempList;
			for(int i = yList_f[y].st;i<=yList_f[y].ed;i++)
			{
				std::shared_ptr<BlobNode> tmpNode_left(new BlobNode);
				tmpNode_left->IsLeft = true;
				tmpNode_left->row = y;
				tmpNode_left->col = RLE_f[i].st;
				if(tempList.size()>0)
					tmpNode_left->prev = tempList.back();
				tempList.push_back(tmpNode_left);
				if(tempList.size()>1)
					tmpNode_left->prev.lock()->next =tempList.back();

				std::shared_ptr<BlobNode> tmpNode_right(new BlobNode);
				tmpNode_right->IsLeft = false;
				tmpNode_right->row = y;
				tmpNode_right->col = RLE_f[i].ed;
				if(tempList.size()>0)
					tmpNode_right->prev = tempList.back();
				tempList.push_back(tmpNode_right);
				if(tempList.size()>1)
					tmpNode_right->prev.lock()->next =tempList.back();
			}
			_NodeYList.push_back(tempList);
			if(tempList.size()>0)
			{
				_FrontNodeList.push_back(tempList.front());
				_ReartNodeList.push_back(tempList.back());
			}
			else
			{
//				std::shared_ptr<BlobNode> tmpNode(new BlobNode);
				_FrontNodeList.push_back(nullptr);
				_ReartNodeList.push_back(nullptr);
			}
			
		}
		std::vector<std::shared_ptr<BlobNode>> FrontNodeListtmp = _FrontNodeList;

		//shkim connect contours
		//left
		for(int y=0; y<size_y;)
		{
			if(_FrontNodeList[y] != nullptr)
			{
				Recur_Connect_Left(_FrontNodeList[y]);
				if(_FrontNodeList[y] == nullptr)
					y++;
			}
			else
				y++;
		}

		_FrontNodeList = FrontNodeListtmp;
		//right
		for(int y=size_y-1; y>=0;)
		{
			if(_ReartNodeList[y] != nullptr)
			{
				Recur_Connect_Right(_ReartNodeList[y]);
				if(_ReartNodeList[y] == nullptr)
					y--;
			}
			else
				y--;
		}
	}

	inline	void Blob::Recur_Connect_Left(std::shared_ptr<BlobNode> StartNode)
	{
		std::shared_ptr<BlobNode> C_Node = StartNode;
		std::shared_ptr<BlobNode> F_Node;

		int k = C_Node->row;
		if(k+1<_FrontNodeList.size())
			F_Node = _FrontNodeList[k+1];

		while(F_Node != nullptr && F_Node->col<=C_Node->next.lock()->col)
		{

			if(F_Node->next.lock()->col<C_Node->col)
				Recur_Connect_Left(F_Node);
			else if((F_Node->col>=C_Node->col && F_Node->col<=C_Node->next.lock()->col)
				||(C_Node->col>=F_Node->col && C_Node->col<=F_Node->next.lock()->col))
			{
				C_Node->link = F_Node;
				ListFrontSet(C_Node,k);
				C_Node = F_Node;
				k = C_Node->row;
			}

			k = C_Node->row;
			if(k+1>=_FrontNodeList.size())
			{
				F_Node = nullptr;
				continue;
			}

			F_Node = _FrontNodeList[k+1];

		}

		k = C_Node->row;
		std::shared_ptr<BlobNode> R_Node = nullptr;
		if(k+1<_ReartNodeList.size()&&_ReartNodeList[k+1] != nullptr && _ReartNodeList[k+1]->prev.lock() != nullptr)
			R_Node = _ReartNodeList[k+1]->prev.lock();
		while(C_Node->link.lock() == nullptr)
		{
			if( R_Node == nullptr|| R_Node->next.lock()->col<C_Node->col)
			{
				C_Node->link = C_Node->next;
				ListFrontSet(C_Node,k);
				break;
			}
			else if((R_Node->col>=C_Node->col && R_Node->col<=C_Node->next.lock()->col)
				||(C_Node->col>=R_Node->col && C_Node->col<=R_Node->next.lock()->col))
			{
				C_Node->link = C_Node->prev;
				ListFrontSet(C_Node,k);
				break;
			}
			if(R_Node->prev.lock() == nullptr)
			{
				R_Node = nullptr;
				continue;
			}
			R_Node = R_Node->prev.lock()->prev.lock();
		}

	}

	inline	void Blob::Recur_Connect_Right(std::shared_ptr<BlobNode> StartNode)
	{
		std::shared_ptr<BlobNode> C_Node = StartNode;
		std::shared_ptr<BlobNode> F_Node;

		int k = C_Node->row;
		if(k>0)
			F_Node = _ReartNodeList[k-1];

		while (F_Node != nullptr && F_Node->col>=C_Node->prev.lock()->col)
		{
			if(F_Node->prev.lock()->col>C_Node->col)
				Recur_Connect_Right(F_Node);
			else if((F_Node->col<=C_Node->col && F_Node->col>=C_Node->prev.lock()->col)
				||(C_Node->col<=F_Node->col && C_Node->col>=F_Node->prev.lock()->col))
			{
				C_Node->link = F_Node;
				ListRearSet(C_Node,k);
				C_Node = F_Node;
				k = C_Node->row;
			}

			k = C_Node->row;

			if(k<1)
			{
				F_Node = nullptr;
				continue;
			}
			F_Node = _ReartNodeList[k-1];
		}

		k = C_Node->row;
		std::shared_ptr<BlobNode> R_Node = nullptr;
		if(k>0 && _FrontNodeList[k-1] != nullptr &&_FrontNodeList[k-1]->next.lock() != nullptr)
			R_Node = _FrontNodeList[k-1]->next.lock();

		while(C_Node->link.lock() ==nullptr)
		{
			if(R_Node == nullptr || R_Node->prev.lock()->col>C_Node->col)
			{
				C_Node->link = C_Node->prev;
				ListRearSet(C_Node,k);
				break;
			}
			else if((R_Node->col<=C_Node->col && R_Node->col>=C_Node->prev.lock()->col)
				||(C_Node->col<=R_Node->col && C_Node->col>=R_Node->prev.lock()->col))
			{
				C_Node->link = C_Node->next;
				ListRearSet(C_Node,k);
				break;
			}
			if(R_Node->next.lock() == nullptr)
			{
				R_Node = nullptr;
				continue;
			}
			R_Node = R_Node->next.lock()->next.lock();
		}

	}

	void Blob::ListFrontSet(std::shared_ptr<BlobNode> Node,int index)
	{
		if(Node->next.lock()->next.lock() != nullptr)
			_FrontNodeList[index] = Node->next.lock()->next.lock();
		else 
			_FrontNodeList[index]=nullptr;
	}
	void Blob::ListRearSet(std::shared_ptr<BlobNode> Node,int index)
	{
		if(Node->prev.lock()->prev.lock() != nullptr)
			_ReartNodeList[index] = Node->prev.lock()->prev.lock();
		else 
			_ReartNodeList[index]=nullptr;
	}

	///////////////////////////////////////
	void Blob::Recur_Fill_Holes(UCHAR * bImage, long sz_x, long sz_y, long pitch)
	{
		for(int y=0; y<size_y; y++)
		{
			std::vector<std::shared_ptr<BlobNode>> tempList;
			tempList = _NodeYList[y];

			for(int i = 1;(i+1)<tempList.size();i=i+2)
			{
				std::shared_ptr<BlobNode> rightNode = tempList[i];
				std::shared_ptr<BlobNode> leftNode = tempList[i+1];

				if(rightNode->ishole != true && rightNode->link.lock()->row == leftNode->row && rightNode->link.lock()->col == leftNode->col)
				{
					bool flag = true;
					while(rightNode->row != leftNode->row || rightNode->col != leftNode->col || leftNode->IsLeft != rightNode->IsLeft)
					{
						leftNode->IsSearched = true;
						leftNode = leftNode->link.lock();
						if(leftNode->IsSearched||(rightNode->prev.lock()->row == leftNode->row && rightNode->prev.lock()->col == leftNode->col && leftNode->IsLeft == rightNode->prev.lock()->IsLeft))
						{
							flag = false;
							break;
						}
					}
					if(flag)
					while(rightNode->ishole != true)
					{
						rightNode->ishole = true;
						rightNode = rightNode->link.lock();
					}

				}
			}
		}

		for(int y=0; y<size_y; y++)
		{
			std::vector<std::shared_ptr<BlobNode>> tempList;
			tempList = _NodeYList[y];

			for(int i = 1;(i+1)<tempList.size();i=i+2)
			{
				std::shared_ptr<BlobNode> & rightNode = tempList[i];
				std::shared_ptr<BlobNode> & leftNode = tempList[i+1];
				if(rightNode->ishole == true && leftNode->ishole == true)
				{
					UCHAR* ptr = &bImage[y*pitch + rightNode->col];
					int length = (leftNode->col-rightNode->col);
					for(int x = rightNode->col;x<length;x++)
					{
						bImage[y*pitch + x] = 255;
					}
//					memcpy(ptr,whiteCharBuf,sizeof(UCHAR)*(leftNode->col-rightNode->col));
				}

			}
		}
	}

	int Blob::ConnectedComponentLabeling_both()
	{
#ifdef USE_CONNECTIVITY_NET
		Construct_Connectivity_Net();
#else
		ptHoleLabelCnt = 0;
		MarkBackg_Label(RLE_b, RleCnt_b, yList_b, size_y);
		m_nBlobRealCount = m_nBlobCount = MarkLabel_NHole(RLE_f, RleCnt_f, yList_f, size_y);
#endif
		return m_nBlobCount;
	}

	int Blob::MarkBackg_Label(ptRLE * RLE, int nEncCnt, ptRLE_Y * yList, int sz_y)
	{
		// RLE		: RLE 영상 시작 포인터
		// nEncCnt	: RLE 데이터 개수
		// yList	: RLE데이터를 y축으로 나눠주는 데이터(멤버변수)
		// sz_y		: y축 크기

		int curLabel=0;

		label_t * nLabelTmp = &LabelTmp[0];
		for(int i=yList[0].st; i<=yList[0].ed; i++)
		{
			RLE[i].label = ++curLabel;
			nLabelTmp[curLabel] = curLabel;
		}

		int idxBuf[1000];
		int nStPreY(0);
		for(int y = 1; y < sz_y; y++)
		{
			int nStY = yList[y].st;
			int nEdy = yList[y].ed;

			nStPreY = yList[y-1].st;
			int nEdPrey = yList[y-1].ed;
			int nMaxInd(nStPreY);
			for(int i=nStY, n=0; i<=nEdy; i++, n++)
			{
				int nSt = RLE[i].st;
				int nEd = RLE[i].ed;

				if(nSt>nEd)
					continue;

				int nCmpCnt(0);
				int nMin(0x7fffffff), MinInd(0);

				// Y_ToThink(2013-10-24):
				// Merge-Part에서 같은 라벨이 Merge되는 경우는 Hole이다.
				// 여기서 홀의 개수가 나오고 아울러 FillHole이 가능할지 모른다.

				for(int j=nMaxInd; j<=nEdPrey; j++)
				{
					int nSt_c = RLE[j].st;
					int nEd_c = RLE[j].ed;
					if(nEd_c < nSt || nSt_c > nEd_c)
					{
						nMaxInd = j+1;
						continue;
					}

					if( (nSt >= nSt_c && nSt <= nEd_c) ||
						(nEd >= nSt_c && nEd <= nEd_c) ||
						(nSt <  nSt_c && nEd >  nEd_c) )
					{
						if(nMaxInd < j)
							nMaxInd = j;
						int lb = (*(RLE+j)).label;

						idxBuf[nCmpCnt] = lb;
						nCmpCnt++;

						int b = lb;
						while(nLabelTmp[b] != b)
							b = nLabelTmp[b];

						if(nMin > b)
						{
							nMin = b;
							MinInd = lb;
						}
					}
					else if(nSt_c > nEd)
						break;
				}

				if(nMin != 0x7fffffff)
				{
					for(int j=0; j<nCmpCnt; j++)
					{
						int b = idxBuf[j];
						while(nLabelTmp[b] != b)
						{
							int t = nLabelTmp[b];
							nLabelTmp[b] = nMin;
							b = t;
						}
						nLabelTmp[b] = nMin;
					}
					RLE[i].label = nMin;
				}
				else
				{
					RLE[i].label = ++curLabel;
					nLabelTmp[curLabel] = curLabel;
				}
			}
		}

		// flatten
		int cntLabel(1);
		for (int i=1; i<=curLabel; i++)
			nLabelTmp[i] = nLabelTmp[nLabelTmp[i]];

		for(int i=1; i<nEncCnt; i++)
			RLE[i].label = nLabelTmp[RLE[i].label];

		// re-labeling
		int n(1);
		for(int i=1; i<=curLabel; i++)
		{
			if(nLabelTmp[i]>0 && nLabelTmp[i] == i)
			{
				Label[n] = -1;
				nLabelTmp[i]=n++;
			}
		}

		if(n==1)
			return 0;

		for(int i=1; i<nEncCnt; i++)
		{
			int idx = nLabelTmp[RLE[i].label];
			RLE[i].label = idx;

			if(Label[idx]<0)
				Label[idx] = i;
		}

		return n-1;
	}
	int Blob::MarkLabel_NHole(ptRLE * RLE, int nEncCnt, ptRLE_Y * yList, int sz_y)
	{
		// RLE		: RLE 영상 시작 포인터
		// nEncCnt	: RLE 데이터 개수
		// yList	: RLE데이터를 y축으로 나눠주는 데이터(멤버변수)
		// sz_y		: y축 크기


		int curLabel=0;

		label_t * nLabelTmp = &LabelTmp[0];
		for(int i=yList[0].st; i<=yList[0].ed; i++)
		{
			RLE[i].label = ++curLabel;
			nLabelTmp[curLabel] = curLabel;
		}

		int idxBuf[1000];
		int LBidxBuf[1000];
		int nStPreY(0);
		for(int y = 1; y < sz_y; y++)
		{
			int nStY = yList[y].st;
			int nEdy = yList[y].ed;

			nStPreY = yList[y-1].st;
			int nEdPrey = yList[y-1].ed;
			int nMaxInd(nStPreY);
			for(int i=nStY, n=0; i<=nEdy; i++, n++)
			{
				int nSt = RLE[i].st - 1;
				int nEd = RLE[i].ed + 1;

				if(nSt>nEd)
					continue;

				int nCmpCnt(0);
				int nMin(0x7fffffff), MinInd(0);

				// Y_ToThink(2013-10-24):
				// Merge-Part에서 같은 라벨이 Merge되는 경우는 Hole이다.
				// 여기서 홀의 개수가 나오고 아울러 FillHole이 가능할지 모른다.
				for(int j=nMaxInd; j<=nEdPrey; j++)
				{
					int nSt_c = RLE[j].st;
					int nEd_c = RLE[j].ed;
					if(nEd_c < nSt || nSt_c > nEd_c)
					{
						nMaxInd = j+1;
						continue;
					}

					if( (nSt >= nSt_c && nSt <= nEd_c) ||
						(nEd >= nSt_c && nEd <= nEd_c) ||
						(nSt <  nSt_c && nEd >  nEd_c) )
					{
						if(nMaxInd < j)
							nMaxInd = j;
						int lb = (*(RLE+j)).label;

						idxBuf[nCmpCnt] = lb;
						LBidxBuf[nCmpCnt] = j;
						nCmpCnt++;
						
						int b = lb;
						while(nLabelTmp[b] != b)
							b = nLabelTmp[b];

						if(nMin > b)
						{
							nMin = b;
							MinInd = lb;
						}
					}
					else if(nSt_c > nEd)
						break;
				}

				if(nCmpCnt >= 2)
				{
					int LbBufCnt = 0;
					bool bFlag = false;
					for(int z = 0; z<(nCmpCnt-1); z++)
					{
						int zLb = idxBuf[z];
						while(nLabelTmp[zLb] != zLb)
							zLb = nLabelTmp[zLb];

						for(int zs = z+1; zs<nCmpCnt; zs++)
						{
							int zsLb = idxBuf[zs];
							while(nLabelTmp[zsLb] != zsLb)
								zsLb = nLabelTmp[zsLb];
							if(zLb == zsLb) //if(zLb == idxBuf[zs])
							{
								int Back_st = yList_b[y-1].st;
								int Back_ed = yList_b[y-1].ed;
								for(int backIndex=Back_st; backIndex<=Back_ed; backIndex++)
								{
									if((RLE[LBidxBuf[z]].ed+1) == RLE_b[backIndex].st)
									{
										ptHoleLabel[ptHoleLabelCnt]=RLE_b[backIndex].label;
										ptHoleLabelCnt++;
									}
								}
							}
						}
					}
				}

				if(nMin != 0x7fffffff)
				{
					for(int j=0; j<nCmpCnt; j++)
					{
						int b = idxBuf[j];
						while(nLabelTmp[b] != b)
						{
							int t = nLabelTmp[b];
							nLabelTmp[b] = nMin;
							b = t;
						}
						nLabelTmp[b] = nMin;
					}
					RLE[i].label = nMin;
				}
				else
				{
					RLE[i].label = ++curLabel;
					nLabelTmp[curLabel] = curLabel;
				}
			}
		}

		// flatten
		int cntLabel(1);
		for (int i=1; i<=curLabel; i++)
			nLabelTmp[i] = nLabelTmp[nLabelTmp[i]];

		for(int i=1; i<nEncCnt; i++)
			RLE[i].label = nLabelTmp[RLE[i].label];

		// re-labeling
		int n(1);
		for(int i=1; i<=curLabel; i++)
		{
			if(nLabelTmp[i]>0 && nLabelTmp[i] == i)
			{
				Label[n] = -1;
				nLabelTmp[i]=n++;
			}
		}

		if(n==1)
			return 0;

		for(int i=1; i<nEncCnt; i++)
		{
			int idx = nLabelTmp[RLE[i].label];
			RLE[i].label = idx;

			if(Label[idx]<0)
				Label[idx] = i;
		}

		return n-1;
	}

	void Blob::DrawHoleLabels(UCHAR * bImage, long sz_x, long sz_y, long pitch, int* labels, long val, int labelcount)
	{
		for (int y = 0; y < sz_y; y++)
		{
#pragma omp parallel for
			for (int i = yList_b[y].st; i <= yList_b[y].ed; i++)
			{
#pragma omp parallel for
				for (int a = 0; a < labelcount; a++)
				{
					if (RLE_b[i].label != labels[a])
						continue;
					int nS = (y * pitch) + RLE_b[i].st;
					int nE = nS + (RLE_b[i].ed - RLE_b[i].st) + 1;
					std::fill(bImage + nS, bImage + nE, val);
					break;
				}
			}
		}
	}

	void Blob::DrawHoleLabels(UCHAR* bImage, long sz_x, long sz_y, long pitch, std::set<int>& Labels, long val)
	{
		for (int y = 0; y < sz_y; y++)
		{
			int st_b = yList_b[y].st;
			int ed_b = yList_b[y].ed;

			for (int i = st_b; i <= ed_b; i++)
			{
				int st = RLE_b[i].st;
				int ed = RLE_b[i].ed;

				if (Labels.find(RLE_b[i].label) != Labels.end())
				{
					for (int x = st; x <= ed; x++)
					{
						bImage[y * pitch + x] = val;
					}
				}
			}
		}

	}

	bool Blob::isHaveHoleLabel(int Label)
	{
		bool flag = true;
		for(int ptIdx = 0;ptIdx<ptHoleLabelCnt;ptIdx++)
		{
			if(ptHoleLabel[ptIdx]==Label)
				flag = false;
		}

		return flag;
	}
	std::vector<RECT> Blob::GetRects()
	{
		std::vector<RECT> rct;
		int nRects = m_nBlobCount;
		RECT addingRect;
		for (int i = 1, n = 0; i <= nRects; i++)
		{
			if (m_remove[i] == 0)
			{
				addingRect.left = m_min_x[i];
				addingRect.right = m_max_x[i];
				addingRect.top = m_min_y[i];
				addingRect.bottom = m_max_y[i];
				rct.emplace_back(addingRect);
			}
		}
		return rct;
	}
}
