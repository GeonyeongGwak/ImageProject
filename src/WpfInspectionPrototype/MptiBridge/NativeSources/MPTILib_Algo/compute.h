// Compute.h: interface for the CCompute class.
//
// Create function
//
// KSH 2009/03/20	Interpolate()
// KSH 2009/06/17	Interpolate()
//////////////////////////////////////////////////////////////////////
#pragma once
#include "MemoryManager.h"
#define PI 3.141592
inline int RounD(double val)
{
	if (val < 0)
	{
		return (int)(val - 0.5005);
	} 
	else
	{
		return (int)(val + 0.5005);
	}
}
inline int RounDF(float val)
{
	if (val < 0)
	{
		return (int)(val - 0.5005);
	} 
	else
	{
		return (int)(val + 0.5005);
	}
}

#include <math.h>

template <class T>
void	Make_3DArray(T**** array, int x, int y, int z)
{
	if(x >0 && y >0 && z > 0)
	{
		*array = new T**[x];

		for(int i=0; i<x; i++)
		{
			(*array)[i] = new T*[y];

			for(int j=0; j<y; j++)
			{
				(*array)[i][j] = new T[z];
			}
		}
	}
}

template <class T>
void Delete_3DArray(T**** array, int x, int y, int z)
{
    if(*array)
    {           
        // 배열 메모리 지우기
        for(int i=0; i<x; i++ )
        {
            for(int j=0; j<y; j++)
            {
                delete []((*array)[i][j]);
            }        
        }

        // 포인터 배열 메모리 지우기
        for(i=0; i<x; i++)
        {
            delete []((*array)[i]);
        }

        // 이중 포인터 배열 메모리 지우기
        delete [] (*array); 
    }

	(*array) = NULL;
}

template <class T>
void	Make_2DArray(T*** array, int m, int n)
{
	if(m > 0 && n > 0)
	{
		*array = new T*[m];

		for(int i=0; i<m; i++)
		{
			(*array)[i] = new T[n];
		}
	}
}

template <class T>
void Delete_2DArray(T*** array, int m, int n)
{
    if(*array)
    {           
        // 포인터 배열 메모리 지우기
        for(int i=0; i<m; i++)
        {
            delete []((*array)[i]);
        }

        // 이중 포인터 배열 메모리 지우기
        delete [] (*array); 
    }
	(*array) = NULL;
}

template <class T>
void	Make_1DArray(T** array, __int64 sizeX)
{
	if(*array != NULL)
		delete [] (*array); 

    if(sizeX>0)
	*array = new T[sizeX];
}

template <class T>
void	Make_1DArray(PCHAR FN, UINT LN, T** array, __int64 sizeX)
{
	if (*array != NULL)
		delete [] (*array); 
		//g_pMManager->pem_delete(*array, true);

	if (sizeX > 0)
		*array = new T[sizeX];
		//*array = g_pMManager->pem_new<T>(true, sizeX, FN, LN);
}

template <class T>
void Delete_1DArray(T** array)
{
	if(*array == NULL)
		return;
	delete [] (*array); 
	(*array) = NULL;
}

template <class T>
void Swap(T& _first, T& _second)
{
	T temp = _first;
	_first = _second;
	_second = temp;
}

template <class T>
void Sort(T *Array, int Count)
{
	int i,j,half,minpos;
	T minval;

	half=Count/2;
	for(i=0;i<=half;i++)
	{
		minpos=i;
		minval=Array[minpos];
		for(j=i+1;j<Count;j++)
		{
			if(Array[j]<minval)
			{
				minpos=j;
				minval=Array[j];
			}
		}
		Array[minpos]=Array[i];
		Array[i]=minval;
	}
}

template <class T>
void Median(T** Src, T** Dst, int sizeX, int sizeY, int nfilterScale)
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
void Gaussian(T **Src, T **Dst, T **svd_filter, int sizeX, int sizeY, int order, int sigma)
{
/*
	#define row 640
	#define column 480
*/
	float **m_V_Convol;
	float **svd_buff;
	Make_2DArray(&m_V_Convol, sizeY, sizeX);
	Make_2DArray(&svd_buff, sizeY, sizeX);


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

	for( y=c;y<sizeY;y++){
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
	for( j=0;j<sizeY;j++){		
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

	for(i=0;i<sizeY;i++){
		for(int j=0;j<sizeX;j++){
			Dst[i][j]=unsigned char(svd_buff[i][j]);
		}
	}

	Delete_2DArray(&m_V_Convol, sizeY, sizeX);
	Delete_2DArray(&svd_buff, sizeY, sizeX);
}

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}
template <class T>
void gaussj(T **a, T **b)
{
	int i,icol,irow,j,k,l,ll;
	T big,dum,pivinv,temp;

	int indxr[]={0,0,0,0};
	int indxc[]={0,0,0,0};
	int ipiv[]={0,0,0,0};
			
	for (i=0;i<4;i++) 
	{	//This is the main loop over the columns to be
							//reduced. 
		big=0.0;
		for (j=0;j<4;j++)	//This is the outer loop of the search for a pivot
							//element. 
		{
			if (ipiv[j] != 1)
			{
				for (k=0;k<4;k++) 
				{
					if (ipiv[k] == 0) 
					{
						if (fabs(a[j][k]) >= big) 
						{
							big=fabs(a[j][k]);
							irow=j;
							icol=k;
						}
					} else if (ipiv[k] > 1) 
					{
						//AfxMessageBox("gaussj: Singular Matrix-1");
					}
				}
			}
		}
		++(ipiv[icol]);

		if (irow != icol) 
		{
			for (l=0;l<4;l++) SWAP(a[irow][l],a[icol][l])
			for (l=0;l<1;l++) SWAP(b[irow][l],b[icol][l])
		}

		indxr[i]=irow;					//We are now ready to divide the pivot row by the
										//pivot element, located at irow and icol. 
		indxc[i]=icol;
		
		if (a[icol][icol] == 0.0)
		{
			//AfxMessageBox("gaussj: Singular Matrix-2");
		}
		pivinv=(float)( 1.0/a[icol][icol] );
		a[icol][icol]=1.0;
		for (l=0;l<4;l++) a[icol][l] *= pivinv;
		for (l=0;l<1;l++) b[icol][l] *= pivinv;
		for (ll=0;ll<4;ll++)			//Next, we reduce the rows...
		{
			if (ll != icol) 
			{			//...except for the pivot one, of course.
				dum=a[ll][icol];
				a[ll][icol]=0.0;
				for (l=0;l<4;l++) a[ll][l] -= a[icol][l]*dum;
				for (l=0;l<1;l++) b[ll][l] -= b[icol][l]*dum;
			}
		}
	}
}

// KSH 2009/06/17
template <class T>
float Interpolate(float pt_x, float pt_y, T **src, int row, int col, int inc=1)
{
	int minX = 0;
	int minY = 0;


//	if(pt_x > 0.) 
		minX = (int)floor( (pt_x) ) - (inc -1);
//	if(pt_y  > 0.) 
		minY = (int)floor( (pt_y) ) - (inc -1);

	if(minX<0) 
		minX = 0;
	else if(minX >= col-1) 
		minX = col-2;

	if(minY<0) 
		minY = 0;
	else if(minY >= row-1) 
		minY = row-2;


//	if(pt_x > 1)
//		minX = (int)floor( pt_x ) - 1; // 소수점이하 값을 제거한 좌표
//	else
//		minX = (int)floor( pt_x ); // 소수점이하 값을 제거한 좌표
//	
//	if(pt_y > 1)
//		minY = (int)floor( pt_y ) - 1;
//	else
//		minY = (int)floor( pt_y );
	
	int maxX = minX+inc; // 소수점 좌표를 올림한 좌표값
	int maxY = minY+inc;

	float ** fInterpolation;
	fInterpolation = new float*[4];
	int i;
    for(i=0; i<4; i++)
    {
        fInterpolation[i] = new float[4];
    }

	float **h;
	Make_2DArray(&h, 4,1);

	float ResultData;	

	fInterpolation[0][0] = 1;
	fInterpolation[0][1] = static_cast<float>(minX);
	fInterpolation[0][2] = static_cast<float>(minY);
	fInterpolation[0][3] = static_cast<float>(minX * minY);

	fInterpolation[1][0] = 1;
	fInterpolation[1][1] = static_cast<float>(minX);
	fInterpolation[1][2] = static_cast<float>(maxY);
	fInterpolation[1][3] = static_cast<float>(minX * maxY);

	fInterpolation[2][0] = 1;
	fInterpolation[2][1] = static_cast<float>(maxX);
	fInterpolation[2][2] = static_cast<float>(maxY);
	fInterpolation[2][3] = static_cast<float>(maxX * maxY);

	fInterpolation[3][0] = 1;
	fInterpolation[3][1] = static_cast<float>(maxX);
	fInterpolation[3][2] = static_cast<float>(minY);
	fInterpolation[3][3] = static_cast<float>(maxX * minY);

	h[0][0] = src[minY][minX];
	h[1][0] = src[maxY][minX];
	h[2][0] = src[maxY][maxX];
	h[3][0] = src[minY][maxX];
	
	gaussj(fInterpolation, h);
//	Gaussj(fInterpolation, 4, h, 1);

	ResultData = h[0][0] + h[1][0] * pt_x + h[2][0] * pt_y + h[3][0] * pt_x * pt_y;

	for(i = 0; i < 4; i++)
	{
		delete [] (fInterpolation[i]);
    }
	delete [] fInterpolation;
	Delete_2DArray(&h, 4,1);
	return ResultData;
}

/*
void dludcmp(double **a, int n, int *indx, double *d);
{
	int i,imax,j,k;
	double big,dum,sum,temp;
	double *vv;						//vv stores the implicit scaling of each row.

	vv = new double[3];
//	Make_1DArray(&vv, n);

	*d=1.0;							//No row interchanges yet.
	for (i=0;i<n;i++) {			//Loop over rows to get the implicit scaling informa-
									//tion. 
		big=0.0;
		for (j=0;j<n;j++)
		if ((temp=fabs(a[i][j])) > big) big=temp;
			if (big == 0.0) return;//AfxMessageBox("Singular matrix in routine ludcmp");
		//No nonzero largest element.
		vv[i]=1.0/big;				//Save the scaling.
	}
	for (j=0;j<n;j++) {			//This is the loop over columns of Crout's method.
		for (i=0;i<j-1;i++) {			//This is equation (2.3.12) except for i = j.
			sum=a[i][j];
			for (k=0;k<i-1;k++) sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
	}
	big=0.0;						//Initialize for the search for largest pivot element.
	for (i=0;i<n;i++) {			//This is i = j of equation (2.3.12) and i = j +1 : : : N
									//of equation (2.3.13). 
		sum=a[i][j];
		for (k=0;k<j-1;k++)
			sum -= a[i][k]*a[k][j];
		a[i][j]=sum;
		if ( (dum=vv[i]*fabs(sum)) >= big) {
		//Is the gure of merit for the pivot better than the best so far?
			big=dum;
			imax=i;
		}
	}
	if (j != imax) {					//Do we need to interchange rows?
		for (k=0;k<n;k++) {			//Yes, do so...
			dum=a[imax][k];
			a[imax][k]=a[j][k];
			a[j][k]=dum;
		}
		*d = -(*d);							//...and change the parity of d.
		vv[imax]=vv[j];						//Also interchange the scale factor.
	}
	indx[j]=imax;
	if (a[j][j] == 0.0) a[j][j]=double(TINY);
	//If the pivot element is zero the matrix is singular (at least to the precision of the
	//algorithm). For some applications on singular matrices, it is desirable to substitute
	//TINY for zero.
	if (j != n) {							//Now, nally, divide by the pivot element.
		dum=1.0/(a[j][j]);
		for (i=j;i<n;i++) a[i][j] *= dum;
		}
	}										//Go back for the next column in the reduction.
//	Delete_1DArray(&vv);
	delete[] vv;
}

void dlubksb(double **a, int n, int *indx, double b[]);
{
	int i,ii=0,ip,j;
	double sum;

	for (i=0;i<n;i++) {			//When ii is set to a positive value, it will become the
									//index of the rst nonvanishing element of b. We now
									//do the forward substitution, equation (2.3.6). The
									//only new wrinkle is to unscramble the permutation
									//as we go.
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii)
		for (j=ii-1;j<i-1;j++) sum -= a[i][j]*b[j];
		else if (sum) ii=i;			//A nonzero element was encountered, so from now on we
									//will have to do the sums in the loop above. 
		b[i]=sum;
	}
	for (i=n-1;i>=0;i--) {			//Now we do the backsubstitution, equation (2.3.7).
		sum=b[i];
		for (j=i;j<n;j++) sum -= a[i][j]*b[j];
		b[i]=sum/a[i][i];			//Store a component of the solution vector X.
	}								//All done!

}*/