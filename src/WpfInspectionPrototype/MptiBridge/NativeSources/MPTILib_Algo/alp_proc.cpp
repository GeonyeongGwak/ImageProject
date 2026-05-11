#include "stdafx.h"
#include "alp_proc.h"

#include "math.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

#ifndef _ALP_ALLOC_H_
#include "alp_alloc.h"
#endif

// AVX
#include <smmintrin.h>
#include <immintrin.h>
int avx_alpha_trimmed_mean_3x3(float* src, float* dst, int w, int h, int p, int dp, int alpha);
int avx_alpha_trimmed_mean_5x5(float* src, float* dst, int w, int h, int p, int dp, int alpha);


int alpp_alpha_trimmed_mean_5x5(float* src, float* dst, int roiWidth, int roiHeight, int xIdxStSrc, int yIdxStSrc, int srcStride, int xIdxStDst, int yIdxStDst, int dstStride, int alpha)
{
	if (src == NULL) return -1;
	if (dst == NULL) return -1;

	if ((alpha >= 25) 
		|| ((xIdxStSrc + roiWidth) > srcStride) 
		|| ((xIdxStDst + roiWidth) > dstStride))
	{
		if (src != dst)
		{
			for (int y=0 ; y<roiHeight ; y++)
			{
				for (int x=0 ; x<roiWidth ; x++)
				{
					int idxSrc = (yIdxStSrc + y) * srcStride + (xIdxStSrc + x);
					int idxDst = (yIdxStDst + y) * dstStride + (xIdxStDst + x);
					dst[idxDst] = src[idxSrc];
				}
			}
		}

		return -1;
	}

	int si = yIdxStSrc * srcStride + xIdxStSrc;
	int di = yIdxStDst * dstStride + xIdxStDst;
	avx_alpha_trimmed_mean_5x5(&src[si], &dst[di], roiWidth, roiHeight, srcStride, dstStride, alpha);

	return 0;
}


int alpp_alpha_trimmed_mean_horizontal(float* src, float* dst, RECT roi, int widthStep, int alpha_h, int alpha_l)
{
	if (src == NULL) return -1;
	if (dst == NULL) return -1;
	
	const int lenFilter = 11;
	const int halfLenFilter = 5;

	if (((alpha_l + alpha_h) >= lenFilter)
		|| ((roi.right - roi.left) < lenFilter)
		|| ((roi.bottom - roi.top) < lenFilter))
	{
		if (src != dst)
		{
			for (int y=roi.top ; y<roi.bottom ; y++)
			{
				for (int x=roi.left ; x<roi.right ; x++)
				{
					int idx = y * widthStep + x;
					dst[idx] = src[idx];
				}
			}
		}

		return -1;
	}

	float k[lenFilter];
	int cnt = 0;
	float tmp = 0.f;
	float meanVal = 0.f;
	int idx = 0;

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			idx = y * widthStep + x;

			if ((x < (roi.left + halfLenFilter)) || (x >= (roi.right - halfLenFilter)))
			{
				dst[idx] = src[idx];
			}
			else
			{
				cnt = 0;
				tmp = 0.f;
				for (int m=(-halfLenFilter) ; m<=halfLenFilter ; m++)
				{
					k[cnt] = src[y * widthStep + (x + m)];

					// sorting
					for (int c=cnt ; c>=1; c--)
					{
						if (k[c-1] > k[c])
						{
							tmp = k[c];
							k[c] = k[c-1];
							k[c-1] = tmp;
						}
						else
							c = 0;
					}
					cnt++;
				}

				meanVal = alpp_get_mean_range(k, alpha_l, (lenFilter-1-alpha_h));
				dst[idx] = meanVal;
			}
		}
	}

	return 0;
}


int alpp_alpha_trimmed_median_horizontal(float* src, float* dst, RECT roi, int widthStep)
{
	if (src == NULL) return -1;
	if (dst == NULL) return -1;

	const int lenFilter = 11;
	const int halfLenFilter = 5;

	if (((roi.right - roi.left) < lenFilter)
		|| ((roi.bottom - roi.top) < lenFilter))
	{
		if (src != dst)
		{
			for (int y=roi.top ; y<roi.bottom ; y++)
			{
				for (int x=roi.left ; x<roi.right ; x++)
				{
					int idx = y * widthStep + x;
					dst[idx] = src[idx];
				}
			}
		}

		return -1;
	}
	
	float k[lenFilter];	// max length 11
	int cnt = 0;
	float tmp = 0.f;
	float meanVal = 0.f;
	int idx = 0;

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			idx = y * widthStep + x;

			if ((x < (roi.left + halfLenFilter)) || (x >= (roi.right - halfLenFilter)))
			{
				dst[idx] = src[idx];
			}
			else
			{
				cnt = 0;
				tmp = 0.f;
				for (int m=(-halfLenFilter) ; m<=halfLenFilter ; m++)
				{
					k[cnt] = src[y * widthStep + (x + m)];

					// sorting
					for (int c=cnt ; c>=1; c--)
					{
						if (k[c-1] > k[c])
						{
							tmp = k[c];
							k[c] = k[c-1];
							k[c-1] = tmp;
						}
						else
							c = 0;
					}
					cnt++;
				}

				if (cnt)
				{
					dst[idx] = k[cnt/2];
				}
				else
				{
					dst[idx] = src[idx];
				}
			}
		}
	}

	return 0;
}


int alpp_alpha_trimmed_median_horizontal(float* src, float* dst, RECT roi, int widthStep, float marking, float markOverwrite)
{
	if (src == NULL) return -1;
	if (dst == NULL) return -1;

	const int lenFilter = 11;
	const int halfLenFilter = 5;

	if (((roi.right - roi.left) < lenFilter)
		|| ((roi.bottom - roi.top) < lenFilter))
	{
		if (src != dst)
		{
			for (int y=roi.top ; y<roi.bottom ; y++)
			{
				for (int x=roi.left ; x<roi.right ; x++)
				{
					int idx = y * widthStep + x;
					dst[idx] = src[idx];
				}
			}
		}

		return -1;
	}

	float k[lenFilter];	// max length 11
	int cnt = 0;
	float tmp = 0.f;
	float meanVal = 0.f;
	int idx = 0;

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			idx = y * widthStep + x;

			if ((x < (roi.left + halfLenFilter)) || (x >= (roi.right - halfLenFilter)))
			{
				dst[idx] = src[idx];
			}
			else if (src[idx] == marking)
			{
				dst[idx] = markOverwrite;
			}
			else
			{
				cnt = 0;
				tmp = 0.f;
				for (int m=(-halfLenFilter) ; m<=halfLenFilter ; m++)
				{
					float val = src[y * widthStep + (x + m)];
					
					if (val != marking)
					{
						k[cnt] = val;

						// sorting
						for (int c=cnt ; c>=1; c--)
						{
							if (k[c-1] > k[c])
							{
								tmp = k[c];
								k[c] = k[c-1];
								k[c-1] = tmp;
							}
							else
								c = 0;
						}
						cnt++;
					}
				}

				if (cnt)
				{
					dst[idx] = k[cnt/2];
				}
				else
				{
					dst[idx] = src[idx];
				}
			}
		}
	}

	return 0;
}


int alpp_alpha_trimmed_mean_vertical(float* src, float* dst, RECT roi, int widthStep, int alpha_h, int alpha_l)
{
	if (src == NULL) return -1;
	if (dst == NULL) return -1;

	const int lenFilter = 11;
	const int halfLenFilter = 5;

	if (((alpha_l + alpha_h) >= lenFilter)
		|| ((roi.right - roi.left) < lenFilter)
		|| ((roi.bottom - roi.top) < lenFilter))
	{
		if (src != dst)
		{
			for (int y=roi.top ; y<roi.bottom ; y++)
			{
				for (int x=roi.left ; x<roi.right ; x++)
				{
					int idx = y * widthStep + x;
					dst[idx] = src[idx];
				}
			}
		}

		return -1;
	}

	float k[lenFilter];
	int cnt = 0;
	float tmp = 0.f;
	float meanVal = 0.f;
	int idx = 0;

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			idx = y * widthStep + x;

			if ((y < (roi.top + halfLenFilter)) || (y >= (roi.bottom - halfLenFilter)))
			{
				dst[idx] = src[idx];
			}
			else
			{
				cnt = 0;
				tmp = 0.f;
				for (int n=(-halfLenFilter) ; n<=halfLenFilter ; n++)
				{
					k[cnt] = src[(y + n) * widthStep + x];

					// sorting
					for (int c=cnt ; c>=1; c--)
					{
						if (k[c-1] > k[c])
						{
							tmp = k[c];
							k[c] = k[c-1];
							k[c-1] = tmp;
						}
						else
							c = 0;
					}
					cnt++;
				}

				meanVal = alpp_get_mean_range(k, alpha_l, (lenFilter-1-alpha_h));
				dst[idx] = meanVal;
			}
		}
	}

	return 0;
}


int alpp_alpha_trimmed_median_vertical(float* src, float* dst, RECT roi, int widthStep)
{
	if (src == NULL) return -1;
	if (dst == NULL) return -1;

	const int lenFilter = 11;
	const int halfLenFilter = 5;

	if (((roi.right - roi.left) < lenFilter)
		|| ((roi.bottom - roi.top) < lenFilter))
	{
		if (src != dst)
		{
			for (int y=roi.top ; y<roi.bottom ; y++)
			{
				for (int x=roi.left ; x<roi.right ; x++)
				{
					int idx = y * widthStep + x;
					dst[idx] = src[idx];
				}
			}
		}

		return -1;
	}

	float k[lenFilter];	// max length 11
	int cnt = 0;
	float tmp = 0.f;
	float meanVal = 0.f;
	int idx = 0;

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			idx = y * widthStep + x;
			
			if ((y < (roi.top + halfLenFilter)) || (y >= (roi.bottom - halfLenFilter)))
			{
				dst[idx] = src[idx];
			}
			else
			{
				cnt = 0;
				tmp = 0.f;
				for (int n=(-halfLenFilter) ; n<=halfLenFilter ; n++)
				{
					k[cnt] = src[(y + n) * widthStep + x];

					// sorting
					for (int c=cnt ; c>=1; c--)
					{
						if (k[c-1] > k[c])
						{
							tmp = k[c];
							k[c] = k[c-1];
							k[c-1] = tmp;
						}
						else
							c = 0;
					}
					cnt++;
				}

				if (cnt)
				{
					dst[idx] = k[cnt/2];
				}
				else
				{
					dst[idx] = src[idx];
				}
			}
		}
	}

	return 0;
}


int alpp_alpha_trimmed_median_vertical(float* src, float* dst, RECT roi, int widthStep, float marking, float markOverwrite)
{
	if (src == NULL) return -1;
	if (dst == NULL) return -1;

	const int lenFilter = 11;
	const int halfLenFilter = 5;

	if (((roi.right - roi.left) < lenFilter)
		|| ((roi.bottom - roi.top) < lenFilter))
	{
		if (src != dst)
		{
			for (int y=roi.top ; y<roi.bottom ; y++)
			{
				for (int x=roi.left ; x<roi.right ; x++)
				{
					int idx = y * widthStep + x;
					dst[idx] = src[idx];
				}
			}
		}

		return -1;
	}

	float k[lenFilter];	// max length 11
	int cnt = 0;
	float tmp = 0.f;
	float meanVal = 0.f;
	int idx = 0;

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			idx = y * widthStep + x;

			if ((y < (roi.top + halfLenFilter)) || (y >= (roi.bottom - halfLenFilter)))
			{
				dst[idx] = src[idx];
			}
			else if (src[idx] == marking)
			{
				dst[idx] = markOverwrite;
			}
			else
			{
				cnt = 0;
				tmp = 0.f;
				for (int n=(-halfLenFilter) ; n<=halfLenFilter ; n++)
				{
					float val = src[(y + n) * widthStep + x];
					
					if (val != marking)
					{
						k[cnt] = val;
						
						// sorting
						for (int c=cnt ; c>=1; c--)
						{
							if (k[c-1] > k[c])
							{
								tmp = k[c];
								k[c] = k[c-1];
								k[c-1] = tmp;
							}
							else
								c = 0;
						}
						cnt++;
					}
				}

				if (cnt)
				{
					dst[idx] = k[cnt/2];
				}
				else
				{
					dst[idx] = src[idx];
				}
			}
		}
	}

	return 0;
}


#pragma region _inside_function_

int avx_alpha_trimmed_mean_3x3(float* src, float* dst, int w, int h, int p, int dp, int alpha)
{
	if ((src == NULL) || (dst == NULL)) return -1;
	if ((alpha * 2) >= (3 * 3)) return -1;

	const int nValBytes(sizeof(float));
	const int nVecBytes(32);
	const int nVectorSz(nVecBytes/nValBytes);
	const int nBlockSz(3);
	const int nBlockCt(nBlockSz/2);
	const int nFilterLength(nBlockSz*nBlockSz);

	float * buf = (float *)_aligned_malloc((nFilterLength*nVectorSz)*nValBytes, nVecBytes);
	float * buf2 = (float *)_aligned_malloc((nFilterLength*nVectorSz)*nValBytes, nVecBytes);
	float * DivVal = (float *)_aligned_malloc(nVectorSz*nValBytes, nVecBytes);

	float val = (float)(nFilterLength-(alpha*2));
	for(int z=0; z<nVectorSz; z++)
		DivVal[z]=val;

	register __m256 ymm0 = _mm256_load_ps(DivVal), ymm1, ymm2, ymm3, ymm4;

	for(int y=0, si=0, di=nBlockCt*dp; y<(h-5); y++, si+=p, di+=dp)
	{
		for(int x=0; x<(w-5); x+=nVectorSz)
		{
			// 1차 정렬
			// 1열
			ymm1 = _mm256_min_ps(_mm256_loadu_ps(&src[si+x+0]), _mm256_loadu_ps(&src[si+x+1]));
			ymm2 = _mm256_max_ps(_mm256_loadu_ps(&src[si+x+0]), _mm256_loadu_ps(&src[si+x+1]));
			_mm256_storeu_ps(&buf[2*nVectorSz], _mm256_max_ps(ymm2, _mm256_loadu_ps(&src[si+x+2])));
			ymm2 = _mm256_min_ps(ymm2, _mm256_loadu_ps(&src[si+x+2]));
			_mm256_storeu_ps(&buf[0*nVectorSz], _mm256_min_ps(ymm1, ymm2));
			_mm256_storeu_ps(&buf[1*nVectorSz], _mm256_max_ps(ymm1, ymm2));

			// 2열
			ymm1 = _mm256_min_ps(_mm256_loadu_ps(&src[si+p+x+0]), _mm256_loadu_ps(&src[si+p+x+1]));
			ymm2 = _mm256_max_ps(_mm256_loadu_ps(&src[si+p+x+0]), _mm256_loadu_ps(&src[si+p+x+1]));
			_mm256_storeu_ps(&buf[5*nVectorSz], _mm256_max_ps(ymm2, _mm256_loadu_ps(&src[si+p+x+2])));
			ymm2 = _mm256_min_ps(ymm2, _mm256_loadu_ps(&src[si+p+x+2]));
			_mm256_storeu_ps(&buf[3*nVectorSz], _mm256_min_ps(ymm1, ymm2));
			_mm256_storeu_ps(&buf[4*nVectorSz], _mm256_max_ps(ymm1, ymm2));

			// 3열
			ymm1 = _mm256_min_ps(_mm256_loadu_ps(&src[si+p+p+x+0]), _mm256_loadu_ps(&src[si+p+p+x+1]));
			ymm2 = _mm256_max_ps(_mm256_loadu_ps(&src[si+p+p+x+0]), _mm256_loadu_ps(&src[si+p+p+x+1]));
			_mm256_storeu_ps(&buf[8*nVectorSz], _mm256_max_ps(ymm2, _mm256_loadu_ps(&src[si+p+x+2])));
			ymm2 = _mm256_min_ps(ymm2, _mm256_loadu_ps(&src[si+p+p+x+2]));
			_mm256_storeu_ps(&buf[6*nVectorSz], _mm256_min_ps(ymm1, ymm2));
			_mm256_storeu_ps(&buf[7*nVectorSz], _mm256_max_ps(ymm1, ymm2));

			// 2차 정렬
			// min
			ymm1 = _mm256_min_ps(_mm256_loadu_ps(&buf[0*nVectorSz]), _mm256_loadu_ps(&buf[3*nVectorSz]));
			ymm2 = _mm256_max_ps(_mm256_loadu_ps(&buf[0*nVectorSz]), _mm256_loadu_ps(&buf[3*nVectorSz]));
			_mm256_storeu_ps(&buf2[2*nVectorSz], _mm256_max_ps(ymm2, _mm256_loadu_ps(&buf[6*nVectorSz])));
			ymm2 = _mm256_min_ps(ymm2, _mm256_loadu_ps(&buf[6*nVectorSz]));
			_mm256_storeu_ps(&buf2[0*nVectorSz], _mm256_min_ps(ymm1, ymm2));
			_mm256_storeu_ps(&buf2[1*nVectorSz], _mm256_max_ps(ymm1, ymm2));

			// med
			ymm1 = _mm256_min_ps(_mm256_loadu_ps(&buf[1*nVectorSz]), _mm256_loadu_ps(&buf[4*nVectorSz]));
			ymm2 = _mm256_max_ps(_mm256_loadu_ps(&buf[1*nVectorSz]), _mm256_loadu_ps(&buf[4*nVectorSz]));
			_mm256_storeu_ps(&buf2[5*nVectorSz], _mm256_max_ps(ymm2, _mm256_loadu_ps(&buf[7*nVectorSz])));
			ymm2 = _mm256_min_ps(ymm2, _mm256_loadu_ps(&buf[7*nVectorSz]));
			_mm256_storeu_ps(&buf2[3*nVectorSz], _mm256_min_ps(ymm1, ymm2));
			_mm256_storeu_ps(&buf2[4*nVectorSz], _mm256_max_ps(ymm1, ymm2));

			// max
			ymm1 = _mm256_min_ps(_mm256_loadu_ps(&buf[2*nVectorSz]), _mm256_loadu_ps(&buf[5*nVectorSz]));
			ymm2 = _mm256_max_ps(_mm256_loadu_ps(&buf[2*nVectorSz]), _mm256_loadu_ps(&buf[5*nVectorSz]));
			_mm256_storeu_ps(&buf2[8*nVectorSz], _mm256_max_ps(ymm2, _mm256_loadu_ps(&buf[8*nVectorSz])));
			ymm2 = _mm256_min_ps(ymm2, _mm256_loadu_ps(&buf[8*nVectorSz]));
			_mm256_storeu_ps(&buf2[6*nVectorSz], _mm256_min_ps(ymm1, ymm2));
			_mm256_storeu_ps(&buf2[7*nVectorSz], _mm256_max_ps(ymm1, ymm2));

			// 3차정렬
			// 1차
			ymm3 = _mm256_max_ps(_mm256_loadu_ps(&buf2[1*nVectorSz]), _mm256_loadu_ps(&buf2[3*nVectorSz]));
			_mm256_storeu_ps(&buf2[1*nVectorSz], _mm256_min_ps(_mm256_loadu_ps(&buf2[1*nVectorSz]), _mm256_loadu_ps(&buf2[3*nVectorSz])));
			// 2차
			ymm4 = _mm256_min_ps(_mm256_loadu_ps(&buf2[5*nVectorSz]), _mm256_loadu_ps(&buf2[7*nVectorSz]));
			_mm256_storeu_ps(&buf2[7*nVectorSz], _mm256_max_ps(_mm256_loadu_ps(&buf2[5*nVectorSz]), _mm256_loadu_ps(&buf2[7*nVectorSz])));
			// 3차
			ymm1 = _mm256_min_ps(_mm256_loadu_ps(&buf2[2*nVectorSz]), _mm256_loadu_ps(&buf2[4*nVectorSz]));
			ymm2 = _mm256_max_ps(_mm256_loadu_ps(&buf2[2*nVectorSz]), _mm256_loadu_ps(&buf2[4*nVectorSz]));
			_mm256_storeu_ps(&buf2[5*nVectorSz], _mm256_max_ps(ymm2, _mm256_loadu_ps(&buf2[6*nVectorSz])));
			ymm2 = _mm256_min_ps(ymm2, _mm256_loadu_ps(&buf2[6*nVectorSz]));
			_mm256_storeu_ps(&buf2[3*nVectorSz], _mm256_min_ps(ymm1, ymm2));
			_mm256_storeu_ps(&buf2[4*nVectorSz], _mm256_max_ps(ymm1, ymm2));

			// 4차 정렬
			// 1차
			_mm256_storeu_ps(&buf2[2*nVectorSz], _mm256_min_ps(ymm3, _mm256_loadu_ps(&buf2[3*nVectorSz])));
			_mm256_storeu_ps(&buf2[3*nVectorSz], _mm256_max_ps(ymm3, _mm256_loadu_ps(&buf2[3*nVectorSz])));
			// 2차
			_mm256_storeu_ps(&buf2[6*nVectorSz], _mm256_max_ps(ymm4, _mm256_loadu_ps(&buf2[5*nVectorSz])));
			_mm256_storeu_ps(&buf2[5*nVectorSz], _mm256_min_ps(ymm4, _mm256_loadu_ps(&buf2[5*nVectorSz])));

			// 
			ymm1=_mm256_setzero_ps();
			ymm1=_mm256_add_ps(ymm1, _mm256_load_ps(&buf[(alpha)*nVectorSz]));
			for(int i=alpha+1; i<(nFilterLength-alpha); i++)
				ymm1=_mm256_add_ps(ymm1, _mm256_load_ps(&buf[(i)*nVectorSz]));
			_mm256_storeu_ps(&dst[di+x+nBlockCt], _mm256_div_ps(ymm1, ymm0));
		}
	}

	_aligned_free(buf);
	_aligned_free(buf2);
	_aligned_free(DivVal);

	return 0;
}


int avx_alpha_trimmed_mean_5x5(float* src, float* dst, int w, int h, int p, int dp, int alpha)
{
	if ((src == NULL) || (dst == NULL)) return -1;
	if ((alpha * 2) >= (3 * 3)) return -1;

	float * buf = (float *)_aligned_malloc((416)*sizeof(float), 32);
	float * DivVal = (float *)_aligned_malloc(sizeof(float)*8, 32);

	float val = (float)(25-(alpha*2));
	for(int z=0; z<8; z++)
		DivVal[z]=val;

	register __m256 xmm2 = _mm256_load_ps(DivVal);

	for(int y=0, si=0, di=2*dp; y<(h-5); y++, si+=p, di+=dp)
	{
		for(int x=0; x<(w-5); x+=8)
		{
			_mm256_store_ps(&buf[0], _mm256_min_ps(_mm256_loadu_ps(&src[si+x+0]), _mm256_loadu_ps(&src[si+x+1])));
			_mm256_store_ps(&buf[8], _mm256_max_ps(_mm256_loadu_ps(&src[si+x+0]), _mm256_loadu_ps(&src[si+x+1])));

			register __m256 xmm0;
			int ix(2), iy(0), n(2), ii(0);
			while(iy<5)
			{
				xmm0 = _mm256_loadu_ps(&src[si+x+ii+ix]);
				for(int i=0; i<n; i++)
				{
					register __m256 xmm1 = _mm256_load_ps(&buf[i*8]);
					_mm256_store_ps(&buf[i*8], _mm256_min_ps(xmm1, xmm0));
					xmm0 = _mm256_max_ps(xmm1, xmm0);
				}
				_mm256_store_ps(&buf[n*8], xmm0);

				n++;

				ix++;
				if(ix>=5)
				{
					iy++;
					ii+=p;
					ix=0;
				}
			}

			alpha=4;

			xmm0=_mm256_setzero_ps();
			xmm0=_mm256_add_ps(xmm0, _mm256_load_ps(&buf[(alpha)*8]));
			for(int i=alpha+1; i<(25-alpha); i++)
			{
				xmm0=_mm256_add_ps(xmm0, _mm256_load_ps(&buf[(i)*8]));
			}
			_mm256_storeu_ps(&dst[di+x+2], _mm256_div_ps(xmm0, xmm2));
		}
	}

	_aligned_free(buf);
	_aligned_free(DivVal);

	return 0;
}

#pragma endregion _inside_function_
