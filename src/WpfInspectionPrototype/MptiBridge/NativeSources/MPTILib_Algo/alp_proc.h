//////////////////////////////////////////////////////////////////////////
//
//	alp_proc.h/cpp
//	- create by ATW
//
//	< history >
//	01. ATW 2013/09/23 create below
//		<template> void alpp_swap(T& _first, T& _second);
//		<template> void alpp_sort_insertion(T arr[], int len);
//		<template> void alpp_sort_quick_range(T *Array, int si, int ei);
//		<template> void alpp_sort_quick(T *Array, int len);
//		<template> T alpp_get_mean_range(T *Array, int si, int ei);
//		<template> T alpp_get_mean(T *Array, int len);
//		<template> T alpp_get_max(T pArray[], int len);
//		<template> T alpp_get_min(T pArray[], int len);
//		<template> void alpp_median_2d(T** Src, T** Dst, int sizeX, int sizeY, int nfilterScale);
//		<template> void alpp_gaussian_2d(T **Src, T **Dst, T **svd_filter, int sizeX, int sizeY, int order, int sigma);
//		<template> void alpp_median_1d(T *pSrc, T *pDst, int width, int height, int kernelSizeX, int kernelSizeY);
//		<template> void alpp_gaussian_1d(T *pSrc, T *pDst, int width, int height, int maskSize);
//		<template> int alpp_median_hybrid_1d(T* src, T* dst, RECT roi, int width, int sizeKernel);
//		int alpp_alpha_trimmed_mean(float* src, float* dst, int roiWidth, int roiHeight, int xIdxStSrc, int yIdxStSrc, int srcStride, int xIdxStDst, int yIdxStDst, int dstStride, int alpha);
//	02. ATW 2013/10/17
//		alpp_alpha_trimmed_mean_horizontal/vertical func argument separate from alpha to alpha low and high
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "math.h"

#ifndef D_PI
#define D_PI	3.141592653589793238462643383279502884197169399375105820974944592308
#endif


template <class T>
void alpp_swap(T& _first, T& _second)
{
	T temp = _first;
	_first = _second;
	_second = temp;
}

template <class T>
void alpp_sort_insertion(T arr[], int len)
{
	int i, j;
	T tmp;

	for (i=1 ; i<len ; i++)
	{
		j = i;
		while (j > 0 && arr[j - 1] > arr[j])
		{
			tmp = arr[j];
			arr[j] = arr[j - 1];
			arr[j - 1] = tmp;
			j--;
		}
	}
}

template <class T>
void alpp_sort_insertion2(T arr[], T arr2[], int len)
{
	int i, j;
	T tmp;
	T tmp2;

	for (i=1 ; i<len ; i++)
	{
		j = i;
		while (j > 0 && arr[j - 1] > arr[j])
		{
			tmp = arr[j];
			tmp2 = arr2[j];
			arr[j] = arr[j - 1];
			arr2[j] = arr2[j - 1];
			arr[j - 1] = tmp;
			arr2[j - 1] = tmp2;
			j--;
		}
	}
}

template <class T>
void alpp_sort_quick_range(T *Array, int si, int ei)
{
	int lHold = si;
	int rHold = ei;
	int idxPivot = si;

	T valPivot = Array[idxPivot];

	while (si < ei)
	{
		while ((Array[ei] >= valPivot) && (si < ei))
		{
			ei--;
		}

		if (si != ei)
		{
			Array[si] = Array[ei];
		}

		while ((Array[si] <= valPivot) && (si < ei))
		{
			si++;
		}

		if (si != ei)
		{
			Array[ei] = Array[si];
			ei--;
		}
	}

	Array[si] = valPivot;
	idxPivot = si;
	si = lHold;
	ei = rHold;

	if (si < idxPivot)
	{
		alpp_sort_quick_range(Array, si, idxPivot-1);
	}

	if (ei > idxPivot)
	{
		alpp_sort_quick_range(Array, idxPivot+1, ei);
	}
}

template <class T>
void alpp_sort_quick(T *Array, int len)
{
	alpp_sort_quick_range(Array, 0, len-1);
}

template <class T>
T alpp_get_mean_range(T *Array, int si, int ei)
{
	T sumVal = 0.f;
	T divVal = 0.f;
	for (int i=si ; i<=ei ; i++)
	{
		sumVal += Array[i];
		divVal += 1.f;
	}

	return sumVal / divVal;
}

template <class T>
T alpp_get_mean(T *Array, int len)
{
	return alpp_get_mean_range(Array, 0, len-1);
}

template <class T>
T alpp_get_max(T pArray[], int len)
{
	int idxMax = 0;

	for (int i=1 ; i<len ; ++i)
	{
		if (pArray[idxMax] < pArray[i])
		{
			idxMax = i;
		}
	}

	return pArray[idxMax];
}

template <class T>
int alpp_get_max_index(T pArray[], int len)
{
	int idxMax = 0;

	for (int i=1 ; i<len ; ++i)
	{
		if (pArray[idxMax] < pArray[i])
		{
			idxMax = i;
		}
	}

	return idxMax;
}

template <class T>
T alpp_get_min(T pArray[], int len)
{
	int idxMin = 0;

	for (int i=1 ; i<len ; ++i)
	{
		if (pArray[idxMin] > pArray[i])
		{
			idxMin = i;
		}
	}
	return pArray[idxMin];
}

template <class T>
int alpp_get_min_index(T pArray[], int len)
{
	int idxMin = 0;

	for (int i=1 ; i<len ; ++i)
	{
		if (pArray[idxMin] > pArray[i])
		{
			idxMin = i;
		}
	}
	return idxMin;
}

template <class T>
void alpp_median_2d(T** Src, T** Dst, int sizeX, int sizeY, int nfilterScale)
{
	int i,x,y,dx,dy;
	int Xhalf,Yhalf;
	T Array[1000];

	/* Read input image */
	/*
	T **Data1;
	Data1 = new T *[sizeY];
	for(int j=0; j<sizeY; j++)
	{
		Data1[j] = new T[sizeX];
	}
	for(y=0;y<sizeY;y++){
		for(x=0;x<sizeX;x++){
			Data1[y][x]=Src[y][x];
		}
	}
	*/	

	/* Perform median filtering */
	int m_iMsizex = nfilterScale;
	int m_iMsizey = nfilterScale;
	Xhalf=m_iMsizex/2;
	Yhalf=m_iMsizey/2;
	
	for(y=0;y<sizeY;y++)
	{
		for(x=0;x<sizeX;x++)
		{
			/* Load array with nearby pixel values */
			i=0;
			for(dy=-Yhalf;dy<=(m_iMsizey-Yhalf-1);dy++)
			{
				for(dx=-Xhalf;dx<=(m_iMsizex-Xhalf-1);dx++)
				{
					if((x+dx>=0) && (y+dy>=0) && (x+dx<sizeX) && (y+dy<sizeY)) 
						//Array[i++]=Src[(y+dy)*rr + (x+dx)];
						Array[i++]=Src[y+dy][x+dx];
				}
			}

			/* Select median value */
			Sort(Array,i);

			Dst[y][x]=Array[i/2];
		}
	}
	/*	
	for(j=0; j<sizeY; j++)
	{
		delete [] Data1[j];
	}
	delete [] Data1;
	*/	
}

template <class T>
void alpp_gaussian_2d(T **Src, T **Dst, T **svd_filter, int sizeX, int sizeY, int order, int sigma)
{
	/*
	#define row 640
	#define column 480
	*/
	T **m_V_Convol;
	T **svd_buff;
	alpa_make_array_2d(&m_V_Convol, sizeY, sizeX);
	alpa_make_array_2d(&svd_buff, sizeY, sizeX);

	for(int i=0;i<sizeY;i++){
		for(int j=0;j<sizeX;j++){
			svd_buff[i][j]=Src[i][j];
		}
	}

	T sum=0;
//	float filter[filter_n];
//	DWORD s;
	int r=sizeX-order,c=sizeY-order;

//	for(int x=0;x<nOrder;x++)filter[x]=(float)svd_filter[usm_sigma][x];

	for(int y=0;y<order-1;y++){
		for(int x=0;x<sizeX;x++){
			m_V_Convol[y][x]=svd_buff[y][x];
		}
	}

	for(int y=c;y<sizeY;y++){
		for(int x=0;x<sizeX;x++){
			m_V_Convol[y][x]=svd_buff[y][x];
		}
	}  
	
	//row convolution
	for(int j=order-1;j<(c+1);j++){
		for(int i=0;i<(sizeX);i++){
			for(int k=0;k<order;k++)
			{
				if((order-k-1)==0)sum+=svd_buff[j][i]*svd_filter[sigma][k];
				else sum+=(svd_buff[j-(order-k-1)][i]+svd_buff[j+(order-k-1)][i])*svd_filter[sigma][k];
			}
			m_V_Convol[j][i]=sum;
			sum=0;
		}
	}

	//column convolution
	for(int j=0;j<sizeY;j++){		
		for(int i=order-1;i<(r+1);i++)
		{
			for(int k=0;k<order;k++)
			{
				if((order-k-1)==0)sum+=m_V_Convol[j][i]*svd_filter[sigma][k];
				else sum+=(m_V_Convol[j][i-(order-k-1)]+m_V_Convol[j][i+(order-k-1)])*svd_filter[sigma][k];
			}
	
			svd_buff[j][i]=sum;
			sum=0;
		}
	}

	for(int i=0;i<sizeY;i++){
		for(int j=0;j<sizeX;j++){
			Dst[i][j]=unsigned char(svd_buff[i][j]);
		}
	}

	alpa_delete_array_2d(&m_V_Convol, sizeY, sizeX);
	alpa_delete_array_2d(&svd_buff, sizeY, sizeX);
}

template <class T>
void alpp_median_1d(T *pSrc, T *pDst, int width, int height, int kernelSizeX, int kernelSizeY)
{// ATW 2012.10.25 insert

	int xMid, yMid;
	T bufSort[1000];

	xMid= kernelSizeX / 2;
	yMid= kernelSizeY / 2;

	int i = 0;
	for(int y=0 ; y<height ; y++)
	{
		for(int x=0 ; x<width ; x++)
		{
			i = 0;
			for(int ky=-yMid ; ky<=(kernelSizeY-yMid-1) ; ky++)
			{
				for(int kx=-xMid ; kx<=(kernelSizeX-xMid-1) ; kx++)
				{
					if((x+kx >= 0) && (y+ky >= 0) && (x+kx < width) && (y+ky < height)) 
					{
						bufSort[i++] = pSrc[(y+ky)*width + (x+kx)];
					}
				}
			}

			/* Select median value */
			Sort(bufSort, i);

			pDst[y*width+x] = bufSort[i/2];
		}
	}
}

template <class T>
void alpp_gaussian_1d(T *pSrc, T *pDst, int width, int height, int maskSize)
{// ATW 2012.10.25 insert

	memcpy(pDst, pSrc, sizeof(T)*width*height);

	// get sigma from mask size
	//double sigma = (double)(maskSize - 1) / 6.0;
	double sigma = 1.0;

	// get Gaussian mask
	double *pMask = NULL;
	alpa_make_array_1d(&pMask, maskSize*maskSize);

	int maskCenter = (maskSize / 2);
	int nx, ny;
	for(int y=0 ; y<maskSize ; y++)
	{
		for(int x=0 ; x<maskSize ; x++)
		{
			nx = x - maskCenter;
			ny = y - maskCenter;
			pMask[x+(maskSize*y)] = 1.0 / (2.0*D_PI*sigma*sigma) * exp(-1*((nx*nx)+(ny*ny))/(2.0*sigma*sigma));
		}
	}

	double sumVal;
//	int centerPos;
	
	for(int y=maskCenter ; y<height-maskCenter ; y++)
	{
		for(int x=maskCenter ; x<width-maskCenter ; x++)
		{
			sumVal = 0;
			for(int my=0 ; my<maskSize ; my++)
			{
				for(int mx=0 ; mx<maskSize ; mx++)
				{
					sumVal += (double)pSrc[(x-maskCenter+mx) + (width * (y-maskCenter+my))] * pMask[mx + (maskSize * my)];
				}
			}
			pDst[x + (width * y)] = (T)sumVal;
		}
	}

	// delete mask
	alpa_delete_array_1d(&pMask);
}

template <class T>
int alpp_median_hybrid_1d(T* src, T* dst, RECT roi, int width, int sizeKernel)
{
	if (src == NULL) return -1;
	if (dst == NULL) return -1;
	if (sizeKernel < 3) return -1;
	
	int halfKernel = sizeKernel / 2;

	roi.left += halfKernel;
	roi.right -= halfKernel;
	roi.top += halfKernel;
	roi.bottom -= halfKernel;

	T k[3] = {0.f};	// +, x and center

	int sampleCnt;
	int sampleNum = (sizeKernel * 2 - 1);
	T* samples = NULL;
	alpa_make_array_1d(&samples, sampleNum);

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			int idx = y * width + x;
			k[0] = src[idx];

			int m, n;
			sampleCnt = 0;
			for (m=(-halfKernel) ; m<=halfKernel ; m++)
			{					
				int ki = y * width + (x + m);
				samples[sampleCnt++] = src[ki];
			}
			for (n=(-halfKernel) ; n<=halfKernel ; n++)
			{
				if (n)
				{
					int ki = (y + n) * width + x;
					samples[sampleCnt++] = src[ki];
				}
			}
			alpp_sort_insertion(samples, sampleCnt);
			k[1] = samples[sampleCnt / 2];

			sampleCnt = 0;
			
			for (m=(-halfKernel) ; m<=halfKernel ; m++)
			{
				int ki = (y + m) * width + (x + m);
				samples[sampleCnt++] = src[ki];
			}
			
			for (m=(-halfKernel) ; m<=halfKernel ; m++)
			{
				int ki = (y + m) * width + (x + m - sizeKernel);
				samples[sampleCnt++] = src[ki];
			}

			alpp_sort_insertion(samples, sampleCnt);
			k[2] = samples[sampleCnt / 2];

			alpp_sort_insertion(k, 3);
			dst[idx] = k[1];
		}
	}

	alpa_delete_array_1d(&samples);

	return 0;
}


int alpp_alpha_trimmed_mean_5x5(float* src, float* dst, int roiWidth, int roiHeight, int xIdxStSrc, int yIdxStSrc, int srcStride, int xIdxStDst, int yIdxStDst, int dstStride, int alpha);


int alpp_alpha_trimmed_mean_horizontal(float* src, float* dst, RECT roi, int widthStep, int alpha_h, int alpha_l);


int alpp_alpha_trimmed_median_horizontal(float* src, float* dst, RECT roi, int widthStep);


int alpp_alpha_trimmed_median_horizontal(float* src, float* dst, RECT roi, int widthStep, float marking, float markOverwrite);


int alpp_alpha_trimmed_mean_vertical(float* src, float* dst, RECT roi, int widthStep, int alpha_h, int alpha_l);


int alpp_alpha_trimmed_median_vertical(float* src, float* dst, RECT roi, int widthStep);


int alpp_alpha_trimmed_median_vertical(float* src, float* dst, RECT roi, int widthStep, float marking, float markOverwrite);