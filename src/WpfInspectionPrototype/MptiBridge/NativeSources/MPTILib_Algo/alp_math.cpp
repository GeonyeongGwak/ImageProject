#include "stdafx.h"
#include "alp_math.h"

#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

int gauss_elimination(const double* matrixA, const double* matrixI, float* matrixB, int len);


int alpm_lsm_line(int order, int len, float *points, float *values, float *coef)
{
	if (order < 1) return -1;
	if (len <= order) return -1;	
	if ((points==NULL) || (values==NULL) || (coef==NULL)) return -1;
	
	int numC = order + 1;
	double mul = 1.0;

	double *sigmaXX = NULL;
	double *sigmaYX = NULL;
	double *matU = NULL;
	double *imatU = NULL;
	double *matV = NULL;

	sigmaXX = new double[(order*2+1)];
	sigmaYX = new double[(order+1)];
	matU = new double[numC*numC];
	imatU = new double[numC*numC];
	matV = new double[numC];

	memset(sigmaXX, 0, sizeof(double)*(order*2+1));
	memset(sigmaYX, 0, sizeof(double)*(order+1));
	memset(matU, 0, sizeof(double)*numC*numC);
	memset(imatU, 0, sizeof(double)*numC*numC);
	memset(matV, 0, sizeof(double)*numC);

	// ¢²[Xi^n]	(n: 0 ~ orderx2)
	for(int n=0 ; n<=(order*2) ; n++)
	{
		for(int m=0 ; m<len ; m++)
		{
			mul = 1.0;
			for(int l=0 ; l<n ; l++)
			{
				mul *= (double)points[m];
			}
			sigmaXX[n] += mul;
		}
	}

	// ¢²[Yi x Xi^n]
	for(int n=0 ; n<=order ; n++)
	{
		for(int m=0 ; m<len ; m++)
		{
			mul = 1.0;
			for(int l=0 ; l<n ; l++)
			{
				mul *= (double)points[m];
			}
			sigmaYX[n] += (double)values[m] * mul;
		}
	}

	// Fill matrix U
	for(int n=0 ; n<numC ; n++)
	{
		for(int m=0 ; m<numC ; m++)
		{
			matU[(n*numC)+m] = sigmaXX[(order*2)-n-m];
		}
	}

	// Fill matrix V
	for(int n=0 ; n<=order ; n++)
	{
		matV[n] = sigmaYX[order-n];
	}

	/*// IPP support

	Ipp32f *pIppBuff;
	pIppBuff = NULL;
	pIppBuff = ippsMalloc_32f(numC*numC);

	// Calculate inverse matrix U'
	int srcWidth = numC;
	int srcStride2 = sizeof(float);
	int srcStride1 = srcStride2 * srcWidth;
	ippmInvert_m_32f(matU, srcStride1, srcStride2, pIppBuff, imatU, srcStride1, srcStride2, srcWidth);

	// Coefficients(Result) matrix = imatU' * matV
	int src1Width = numC;
	int src1Height = numC;
	int src1Stride2 = sizeof(float);
	int src1Stride1 = src1Stride2 * src1Width;
	int src2Len = numC;
	int src2Stride2 = sizeof(float);
	int dstStride2 = sizeof(float);
	ippmMul_mv_32f(imatU, src1Stride1, src1Stride2, src1Width, src1Height, matV, src2Stride2, src2Len, retC, dstStride2);

	// Return
	memcpy(coefficients, retC, sizeof(float)*numC);

	if(pIppBuff)
	{
		ippsFree(pIppBuff);
		pIppBuff = NULL;
	}
	*/

	int res = gauss_elimination(matU, matV, coef, numC);

	delete [] sigmaXX;
	delete [] sigmaYX;
	delete [] matU;
	delete [] imatU;
	delete [] matV;

	return res;
}


int alpm_lsm_plane(int order, int len, float* ax, float* ay, float* az, float* coef)
{
	if (order < 1) return -1;
	if (len <= 2) return -1;
	if ((ax==NULL) || (ay==NULL) || (az==NULL)) return -1;

	int coefCnt = 0;
	for (int i=order ; i>=0 ; i--)
		coefCnt += (i + 1);

	double *C = new double [coefCnt];
	double *U = new double [coefCnt * coefCnt];
	double *V = new double [coefCnt];
	
	memset(U, 0, sizeof(double) * coefCnt * coefCnt);
	memset(V, 0, sizeof(double) * coefCnt);

	const double scale = 1000.0;
	for (int n=0 ; n<len ; n++)
	{
		memset(C, 0, sizeof(double) * coefCnt);

		int i = 0;
		for (int x=order ; x>=0 ; x--)
		{
			for (int y=(order-x) ; y>=0 ; y--)
			{
				C[i++] = (double)(pow(ax[n], x) * pow(ay[n], y));
			}
		}

		for (int y=0 ; y<coefCnt ; y++)
		{
			for (int x=0 ; x<coefCnt ; x++)
			{
				U[y * coefCnt + x] += (C[x] * C[y]) / scale;
			}
			V[y] += ((double)az[n] * C[y]) / scale;
		}
	}

	int res = gauss_elimination(U, V, coef, coefCnt);

	delete [] C;
	delete [] U;
	delete [] V;

	return res;
}


bool alpm_pt_in_polygon(POINT pt, const POINT *pts, int ptNum)
{
	POINT spt, ept;				// start, end point
	bool oddNodes = false;		// false: even(out), true: odd(in)

	for(int i=0 ; i<ptNum ; i++)
	{
		spt = pts[i];
		ept = pts[(i + 1) % ptNum];		// % prevent over index

		if(((spt.y <= pt.y) && (ept.y > pt.y))			// an upward crossing
			|| ((spt.y > pt.y) && (ept.y <=  pt.y)))	// a downward crossing
		{
			// pt.x is in left of edge
			oddNodes ^= ((float)pt.x < ((float)spt.x + ((float)(pt.y - spt.y) / (float)(ept.y - spt.y) * (float)(ept.x - spt.x))));
		}
	}

	return oddNodes;
}


#pragma region _inside_function_

int gauss_elimination(const double* matrixA, const double* matrixI, float* matrixB, int len)
{
	//////////////////////////////////////////////////////////////////////////
	//
	// GaussElimination : Gauss elimination for calculate coefficients
	// Made by ATW. 2013/09/12
	// len is N
	// matrixA : N x N
	// matrixI : 1 x N
	// matrixB : 1 x N
	//
	//////////////////////////////////////////////////////////////////////////

	if (matrixA == NULL) return -1;
	if (matrixI == NULL) return -1;
	if (matrixB == NULL) return -1;

	int row = len;
	int rowMax;
	double valMax, valCur, tmp;
	double* mA = NULL;
	double* mI = NULL;
	//
	// memory allocation
	//
	mA = (double*)malloc(sizeof(double) * row * len);
	memcpy(mA, matrixA, sizeof(double) * row * len);
	mI = (double*)malloc(sizeof(double) * row);
	memcpy(mI, matrixI, sizeof(double) * row);
	//
	// forward elimination with pivoting
	//
	for (int r=0 ; r<row-1 ; r++)
	{
		// find max pivot
		valMax = abs(mA[r * row + r]);
		rowMax = r;
		for (int nr=r/*+1*/ ; nr<row ; nr++)
		{
			valCur = abs(mA[nr * row + r]);
			if (valCur > valMax)
			{
				valMax = valCur;
				rowMax = nr;
			}
		}
		// if there something bigger, change row
		if (rowMax != r)	//if (abs(mA[r][r]) != valMax)
		{
			for (int c=0 ; c<len ; c++)
			{
				tmp = mA[r * row + c];
				mA[r * row + c] = mA[rowMax * row + c];
				mA[rowMax * row + c] = tmp;
			}
			tmp = mI[r];
			mI[r] = mI[rowMax];
			mI[rowMax] = tmp;
		}
		// elimination
		for (int nr=r+1 ; nr<row ; nr++)
		{
			tmp = mA[nr * row + r] / mA[r * row + r];
			for (int c=0 ; c<len ; c++)
			{
				mA[nr * row + c] -= (tmp * mA[r * row + c]);
			}
			mI[nr] -= (tmp * mI[r]);
		}
	}
	//
	// backward substitution
	//
	for (int r=row-1 ; r>=0 ; r--)
	{
		for (int c=r+1 ; c<row ; c++)
		{
			mI[r] -= (mA[r * row + c] * mI[c]);
		}
		mI[r] /= mA[r * row + r];
	}
	//
	// return result
	//
	for (int r=0 ; r<row ; r++)
	{
		matrixB[r] = (float)mI[r];
	}
	//
	// memory free
	//
	free(mA);
	free(mI);

	return 0;
}

#pragma endregion _inside_function_
