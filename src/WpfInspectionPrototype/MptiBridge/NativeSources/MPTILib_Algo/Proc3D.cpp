#include "StdAfx.h"
#include "Proc3D.h"

#include "alp_alloc.h"
#include "alp_file.h"

#include <math.h>
#include<cmath>

#include "ipps.h"
#include "ippi.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

// ATW 2013.10.23
#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")

using namespace std;



#define  HISTO_INTERVAL		1
#define  NODATA				-3000

#define ALLOW_HEIGHT	90.0  //%
#define STD_AREA_SIZE	10	  //pixel
#define SEARCH_PIXEL	10	  //pixel


#define  EDGE_CNTS      50
#define  ST_MARGIN		0

#define PI 3.1415926535


#define HEIGHT_MARGIN		20    //20%

//shk 20140410
#define LEAD_LIFT_MARGIN	8 // 12 --> 8
//shk 20140416
#define LEAD_LIFT_MIN	80
//shk 20140422
#define LEAD_LIFT_HIGH_LIMIT 0.7		//70% //권팀장이 책임 진다고 했는데...
#define LEAD_LIFT_DIV	20

//shk 20140512
#define LEAD_DOWNSTEP	0.35

double g_LTCos[721];
double g_LTSin[721];

CProc3D::CProc3D(void)
{
	m_className = _T("CProc3D");

	m_pZmapHistoData = NULL;
	m_histoSize = 0;

	// JACKY 2013/10/03 JUST TEST CODE
}


CProc3D::~CProc3D(void)
{
	DeleteHistoVal();
}

float CProc3D::ExtractBody(float* zmapData, float* bodyDst, float* boardDst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY, int range)
{
	if(zmapData == NULL  || orgSizeX < 0 || orgSizeY < 0 || cx < 0 || cy < 0 || roiSizeX < 0 || roiSizeY < 0)
		return 0;

	float* pZmapRoiData = NULL;
	float* pZmapRoiNormalData = NULL;
	float* pZmapBodyData = NULL;
	float* pZmapBoardData = NULL;
	int* pZmapHistoData = NULL;
	float roiHmax = 0;
	float roiHmin = 0;

	//pZmapRoiData = new float[roiSizeX * roiSizeY];
	pZmapRoiData = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(pZmapRoiData, 0, (sizeof(float) * roiSizeX * roiSizeY));
	//pZmapRoiNormalData = new float[roiSizeX * roiSizeY];
	pZmapRoiNormalData = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(pZmapRoiNormalData, 0, (sizeof(float) * roiSizeX * roiSizeY));
	//pZmapBodyData = new float[roiSizeX * roiSizeY];
	pZmapBodyData = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(pZmapBodyData, 0, (sizeof(float) * roiSizeX * roiSizeY));
	//pZmapBoardData = new float[roiSizeX * roiSizeY];
	pZmapBoardData = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(pZmapBoardData, 0, (sizeof(float) * roiSizeX * roiSizeY));

	//clip data
	GetClipZmapData(zmapData, pZmapRoiData, orgSizeX, orgSizeY, cx, cy, roiSizeX, roiSizeY);

	//get max height , min height
	GetHlimit(pZmapRoiData, roiSizeX, roiSizeY, &roiHmax, &roiHmin);

	//normalize roi zmap data
	NormalizeData_Zmap(pZmapRoiData, pZmapRoiNormalData, roiSizeX, roiSizeY, roiHmin);

	//histogram
	
	m_histoSize = (int)((roiHmax - roiHmin) / HISTO_INTERVAL) + 1;
	if(m_histoSize < 1)
	{	
		/*delete pZmapRoiData;
		delete pZmapRoiNormalData;
		delete pZmapBodyData;
		delete pZmapBoardData;*/
		g_pMManager->pem_delete(pZmapRoiData, false);
		g_pMManager->pem_delete(pZmapRoiNormalData, false);
		g_pMManager->pem_delete(pZmapBodyData, false);
		g_pMManager->pem_delete(pZmapBoardData, false);

		return 0;
	}

	DeleteHistoVal();
	//m_pZmapHistoData = new int[m_histoSize];
	m_pZmapHistoData = g_pMManager->pem_new<int>(true, m_histoSize, (PCHAR)__FUNCTION__, __LINE__);
	memset(m_pZmapHistoData, 0, sizeof(int) * m_histoSize);

	MakeHistogram(pZmapRoiNormalData, m_pZmapHistoData, roiSizeX, roiSizeY, roiHmax, roiHmin, m_histoSize);

// #ifdef _DEBUG
// 	HistogramSave(m_pZmapHistoData, m_histoSize);
// #endif 
	

	//get max body height
	int bodyRangeMin = 0;
	int bodyRangeMax = 0;
	int boardRangeMin = 0;
	int boardRangeMax = 0;

	int thresh = GetThreshold(m_pZmapHistoData, roiSizeX, roiSizeY, m_histoSize);
	GetZmapObjectRange(m_pZmapHistoData, thresh, m_histoSize, range, &bodyRangeMin, &bodyRangeMax);
	GetZmapObjectRange(m_pZmapHistoData, 0, thresh, 0, &boardRangeMin, &boardRangeMax);
	bodyRangeMin = (int)((bodyRangeMin * HISTO_INTERVAL) + (int)roiHmin);
	bodyRangeMax = (int)((bodyRangeMax * HISTO_INTERVAL) + (int)roiHmin);
	boardRangeMin = (int)((boardRangeMin * HISTO_INTERVAL) + (int)roiHmin);
	boardRangeMax = (int)((boardRangeMax * HISTO_INTERVAL) + (int)roiHmin);

	//extract body data & board data
	ExtractData(pZmapRoiData, pZmapBodyData, roiSizeX, roiSizeY, bodyRangeMin, bodyRangeMax, roiHmin);
	ExtractData(pZmapRoiData, pZmapBoardData, roiSizeX, roiSizeY, boardRangeMin, boardRangeMax, roiHmin);

	//get average height ( real body height = zmap body height average -(+) zmap board height average)
	float bodyHavr = 0;
	float boardHavr = 0;
	float realBodyHavr = 0;

	bodyHavr = GetZmapHavr(pZmapBodyData, roiSizeX, roiSizeY ,roiHmin);
	boardHavr =  GetZmapHavr(pZmapBoardData, roiSizeX, roiSizeY ,roiHmin);

	if((boardHavr) <= 0)
		realBodyHavr = bodyHavr - boardHavr;
	else
		realBodyHavr = bodyHavr + boardHavr;

	//return result
	if(bodyDst != NULL)
		memcpy_s(bodyDst, (sizeof(float) * roiSizeX * roiSizeY), pZmapBodyData,  (sizeof(float) * roiSizeX * roiSizeY));

	if(boardDst != NULL)
		memcpy_s(boardDst, (sizeof(float) * roiSizeX * roiSizeY), pZmapBoardData,  (sizeof(float) * roiSizeX * roiSizeY));


	if(pZmapRoiData != NULL)
		//delete pZmapRoiData;
		g_pMManager->pem_delete(pZmapRoiData, false);
	if(pZmapRoiNormalData != NULL)
		//delete pZmapRoiNormalData;
		g_pMManager->pem_delete(pZmapRoiNormalData, false);
	if(pZmapBodyData != NULL)
		//delete pZmapBodyData;
		g_pMManager->pem_delete(pZmapBodyData, false);
	if(pZmapBoardData != NULL)
		//delete pZmapBoardData;
		g_pMManager->pem_delete(pZmapBoardData, false);
	

	return realBodyHavr;
}

void CProc3D::DeleteHistoVal()
{
	if(m_pZmapHistoData != NULL)
	{
		//delete m_pZmapHistoData;
		g_pMManager->pem_delete(m_pZmapHistoData, false);
		m_pZmapHistoData = NULL;
	}

}

void CProc3D::GetClipZmapData(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int sizeX, int sizeY)
{
	if(src == NULL || dst == NULL || orgSizeX < 0 || orgSizeY < 0 || cx < 0 || cy < 0 || sizeX < 0 || sizeY < 0)
		return;

	float* pSrcTemp = NULL;
	float* pDstTemp = NULL; 

	int stX = 0;
	int stY = 0;
	int endX = 0;
	int endY = 0;
	int width = sizeX;
	int height = sizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	BOOL flagX = FALSE;
	BOOL flagY = FALSE;

	if(stX < 0)
	{
		stX = 0;
		flagX = TRUE;
	}

	if(stY < 0)
	{
		stY = 0;
		flagY = TRUE;
	}

	endX = cx + (width / 2);  // end point x
	endY = cy + (height / 2);  // end point Y

	if(endX > orgSizeX)
	{
		endX = orgSizeX;
		flagX = TRUE;
	}

	if(endY > orgSizeY)
	{
		endY = orgSizeY;
		flagY = TRUE;
	}

	if(flagX == TRUE)
		width = endX - stX;

	if(flagY == TRUE)
		height = endY - stY;
	

// 	if((stX + width) > orgSizeX)
// 	{
// 		width = orgSizeX - stX;
// 	}
// 
// 	if((stY + height) > orgSizeY)
// 	{
// 		height = orgSizeY - stY;
// 	}

	pDstTemp = dst;
	pSrcTemp = src;
	pSrcTemp += stX + (stY * orgSizeX);
	for(int i=0; i<height; i++)
	{
		memcpy_s(pDstTemp, (sizeof(float) * width), pSrcTemp, (sizeof(float) * width));
		pSrcTemp += orgSizeX;
		pDstTemp += width;
	}
}

void CProc3D::GetHlimit(float* src, int sizeX, int sizeY, float* retHmax, float* retHmin)
{
	if(src == NULL || sizeX < 0 || sizeY < 0)
		return;

	float hMax = 0;
	float hMin = 0;
	float temp = 0;

	hMax = src[0];
	hMin = src[0];

	float* srcTemp = NULL;
	srcTemp = src;
	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{
			temp = *srcTemp++;
			if(temp > -10000 && temp != NODATA)
			{	
				if(temp > hMax)
				{
					hMax = temp;
				}
				else if(temp < hMin)
				{
					hMin = temp;
				}
			}		
		}
	}

	*retHmax = hMax;
	*retHmin = hMin;
}

void CProc3D::GetHlimit(int* src, int sizeX, int sizeY, int* retHmax, int* retHmin, BOOL removeData, int rmvVal)
{
	if(src == NULL || sizeX < 0 || sizeY < 0)
		return;

	int hMax = 0;
	int hMin = 0;
	int temp = 0;

	hMax = src[(int)(sizeX/2)];
	hMin = src[(int)(sizeX/2)];

	int* srcTemp = NULL;
	srcTemp = src;
	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{
			temp = *srcTemp++;
		
			if(removeData)
			{
				if(temp != rmvVal)
				{	
					if(temp > hMax)
					{
						hMax = temp;
					}
					else if(temp < hMin)
					{
						hMin = temp;
					}
				}
			}
			else
			{
				if(temp > hMax)
				{
					hMax = temp;
				}
				else if(temp < hMin)
				{
					hMin = temp;
				}
			}
			
		}
	}

	*retHmax = hMax;
	*retHmin = hMin;
}



float CProc3D::GetZmapHavr(float* src, int sizeX, int sizeY, float hMin)
{
	if(src == NULL || sizeX < 0 || sizeY < 0)
		return 1;

	float retAvr = 0;
	double sum = 0;
	int cnts = 0;
	float temp = 0;

	float* srcTemp = NULL;
	srcTemp = src;
	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{
			temp = *srcTemp++;
			if(temp !=  hMin)
			{	
				sum = sum + temp;
				cnts++;
			}
		}
	}

	retAvr = (float)(sum / cnts);

	return retAvr;
}

void CProc3D::GetZmapObjectRange(int* histo, int st, int ed, double range, int* rangeMin, int* rangeMax)
{
	if(histo == NULL)
		return;

	double avr = 0.0;
	double stdev = 0.0;
	int min = 0;
	int max = 0;
	int stdVal = 0;

	if(range == 0)
	{
		GetStdev(histo, st, ed, &avr, &stdev);
 		stdVal = (int)((avr + stdev) - range);
	}
	else
	{
		int maxVal = GetMaxVal(histo, st, ed);
		stdVal = maxVal - (int)(maxVal * (range / 100.0));
	}

	int data = 0;

	int index = 0;
	for(int i = st; i <  ed; i++)
	{
		data = histo[i];
		if(data >= stdVal)
		{
			min = i;
			break;
		}	
	}

	for(int i = ed-1; i > min; i--)
	{
		data = histo[i];
		if(data >= stdVal)
		{
			max = i;
			break;
		}	
	}

	*rangeMin = min;
	*rangeMax = max;
}


void CProc3D::NormalizeData_Zmap(float* src, float* dst, int sizeX, int sizeY, float hMin)
{
	if(src == NULL || dst == NULL || sizeX < 0 || sizeY < 0)
		return;

	float* temp = NULL;
	//temp = new float[sizeX * sizeY];
	temp = g_pMManager->pem_new<float>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, (sizeof(float) * sizeX * sizeY));

	float* srcTemp = NULL;
	float* dstTemp = NULL;
	srcTemp = src;
	dstTemp = temp;
	float data = 0;

	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{
			data = *srcTemp++;
			if(data == NODATA)
				*dstTemp++ = 0;
			else
				*dstTemp++ = data - hMin;
		}
	}

	memcpy_s(dst, (sizeof(float) * sizeX * sizeY), temp, (sizeof(float) * sizeX * sizeY));
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

void CProc3D::NormalizeData_Gray(float* src, UCHAR* dst, int sizeX, int sizeY)
{
	if(src == NULL || dst == NULL || sizeX < 0 || sizeY < 0)
		return;

	float hMax = 0;
	float hMin = 0;
	UCHAR* temp = NULL;
	//temp = new UCHAR[sizeX * sizeY];
	temp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, (sizeof(UCHAR) * sizeX * sizeY));

	GetHlimit(src, sizeX, sizeY, &hMax, &hMin);

	float* srcTemp = NULL;
	UCHAR* dstTemp = NULL;
	srcTemp = src;
	dstTemp = temp;
	float data = 0;
	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{	
			data = *srcTemp++;
			if(data == NODATA)
				*dstTemp++ = 0;
			else
				*dstTemp++ = (UCHAR)(((data - hMin) / (hMax - hMin)) * 255) / 1;
		}
	}

	memcpy_s(dst, (sizeof(UCHAR) * sizeX * sizeY), temp, (sizeof(UCHAR) * sizeX * sizeY));

	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

void CProc3D::MakeHistogram(float* src, int* dstHisto, int sizeX, int sizeY, float hMax, float hMin, int histoSize)
{
	if(src == NULL || dstHisto == NULL || sizeX < 0 || sizeY < 0 || histoSize < 0)
		return;

	int* histoTemp = NULL;
	//int total = (int)((hMax - hMin) / HISTO_INTERVAL) + 1;  //0 포함
	int data = 0;
	int mod = 0;
	int temp = 0;

	//histoTemp = new int[histoSize];
	histoTemp = g_pMManager->pem_new<int>(true, histoSize, (PCHAR)__FUNCTION__, __LINE__);
	memset(histoTemp, 0, sizeof(int) * histoSize);

	float* srcTemp = NULL;
	srcTemp = src;

	for(int y=0; y<sizeY; y++)
	{
		for(int x=0; x<sizeX; x++)
		{
			data = (int)*srcTemp++;
			mod = data % HISTO_INTERVAL;
		
			if(mod == 0)
			{	
				temp = data / HISTO_INTERVAL;
				if(temp < 0 || temp >= (sizeof(int) * histoSize))
				{
					AfxMessageBox(_T("zmap histogram make error!"));
					//delete histoTemp;
					g_pMManager->pem_delete(histoTemp, false);
					return;
				}

				histoTemp[temp]++;
			}
		}
	}

	memcpy_s(dstHisto, (sizeof(int) * histoSize), histoTemp, (sizeof(int) * histoSize));

	if(histoTemp != NULL)
		//delete histoTemp;
		g_pMManager->pem_delete(histoTemp, false);

	return;
}

void CProc3D::GetHistogramSize(int* retSize, int* retMax)
{
	*retSize = m_histoSize;
	*retMax = GetMaxVal(m_pZmapHistoData, 0, m_histoSize);
}

void CProc3D::GetHistogramData(int* dst, int histoSize)
{
	if(dst == NULL)
		return ;

	memcpy_s(dst, (sizeof(int) * histoSize), m_pZmapHistoData, (sizeof(int) * histoSize));
}

void CProc3D::ExtractData(float* src, float* dst, int sizeX, int sizeY, int rangeMin, int rangeMax, float defaultValue)
{
	if(src == NULL || dst == NULL || sizeX < 0 || sizeY < 0)
		return;

// 	float hRealMin = hExrtCenter - range;
// 	float hRealMax = hExrtCenter + range;

	float* temp = NULL;
	//temp = new float[sizeX * sizeY];
	temp = g_pMManager->pem_new<float>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, (sizeof(float) * sizeX * sizeY));

	float data = 0;
	float* srcTemp = NULL;
	float* dstTemp = NULL;
	srcTemp = src;
	dstTemp = temp;

	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{
			data = *srcTemp++;
			if(data >= (float)rangeMin && data <= (float)rangeMax)
			{
				*dstTemp++ = data;
			}
			else
			{
				*dstTemp++ = defaultValue;
			}
		}
	}

	memcpy_s(dst, (sizeof(float) * sizeX * sizeY), temp, (sizeof(float) * sizeX * sizeY));
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

void  CProc3D::ExtractData(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int sizeX, int sizeY, float defaultValue)
{
	if(src == NULL || dst == NULL || orgSizeX < 0 || orgSizeY < 0 || cx < 0 || cy < 0 || sizeX < 0 || sizeY < 0)
		return;

	//memset(dst, (int)defaultValue, (sizeof(float) * orgSizeX * orgSizeY));
	for(int i = 0; i < (orgSizeX * orgSizeY); i++)
	{
		dst[i] = defaultValue;
	}

	float* pSrcTemp = NULL;
	float* pDstTemp = NULL; 

	int stX = 0;
	int stY = 0;
	int width = sizeX;
	int height = sizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	if((stX + width) > orgSizeX)
	{
		width = orgSizeX - stX;
	}

	if((stY + height) > orgSizeY)
	{
		height = orgSizeY - stY;
	}

	pDstTemp = dst;
	pSrcTemp = src;
	pSrcTemp += stX + (stY * orgSizeX);
	pDstTemp += stX + (stY * orgSizeX);
	for(int i=0; i<height; i++)
	{
		memcpy_s(pDstTemp, (sizeof(float) * width), pSrcTemp, (sizeof(float) * width));
		pSrcTemp += orgSizeX;
		pDstTemp += orgSizeX;
	}
}


int CProc3D::GetThreshold(int* histo, int sizeX, int sizeY, int histoSize)
{
	if(histoSize < 0)
		return -1;

	int ret = 0;
	float*  hist;
	//hist = new float[histoSize];
	hist = g_pMManager->pem_new<float>(true, histoSize, (PCHAR)__FUNCTION__, __LINE__);

	int T = 0 ,Told = 0;
	float area =(float)(sizeX * sizeY);
	float sum = 0.f;


	for(int i=0; i<histoSize; i++)
	{
		hist[i] = histo[i]/area;
	}

	for(int i=0; i<histoSize; i++)
	{
		sum += (i*hist[i]);
	}
	T = (int)sum;

	if(T<0)
	{
		//delete hist;
		g_pMManager->pem_delete(hist, false);
		return -1;
	}

	float a1,b1,u1,a2,b2,u2;

	do{
		Told = T;

		a1 = b1 = 0;

		for(int i=0; i<=Told; i++)
		{
			a1 += (i*hist[i]);
			b1 += hist[i];
		}

		u1 = a1/b1;

		a2 = b2 = 0;

		for(int i=Told+1; i<histoSize; i++)
		{
			a2 += (i*hist[i]);
			b2 += hist[i];
		}
		u2 = a2/b2;

		if(b1 == 0)
			b1 = 1.f;
		if(b2 == 0)
			b2 = 1.f;

		T = (int)((u1+u2)/2);

		if(T<0)
		{
			//delete hist;
			g_pMManager->pem_delete(hist, false);
			return 150;
		}
	}while(T != Told);

	ret = T;

	if(ret<=0 || ret>=histoSize)
	{	
		//delete hist;
		g_pMManager->pem_delete(hist, false);
		return -1;
	}



	//delete hist;
	g_pMManager->pem_delete(hist, false);
	return ret;
}

void CProc3D::GetStdev(int* data, int st, int ed, double* retAvr, double* retStd)
{
	int trueCnt = 0;
	double sumVal = 0.0;

	for( int i = st ; i < ed; i++)
	{
		sumVal += data[i];
		trueCnt ++;
	}
	double avgVal = sumVal/ (float)trueCnt;


	double total = 0.f;
	int cnt = 0 ;
	for( int i = st ; i < ed; i++)
	{		
		total += pow((double)data[i] - avgVal, 2);
		cnt ++;
	}
	double stdDev = sqrt((total) / (double)cnt);

	*retAvr = avgVal;
	*retStd = stdDev;
}

void CProc3D::GetStdev(float* data, int st, int ed, double* retAvr, double* retStd)
{
	int trueCnt = 0;
	double sumVal = 0.0;

	for( int i = st ; i < ed; i++)
	{
		sumVal += data[i];
		trueCnt ++;
	}
	double avgVal = sumVal/ (float)trueCnt;


	double total = 0.f;
	int cnt = 0 ;
	for( int i = st ; i < ed; i++)
	{		
		total += pow((double)data[i] - avgVal, 2);
		cnt ++;
	}
	double stdDev = sqrt((total) / (double)cnt);

	*retAvr = avgVal;
	*retStd = stdDev;
}

int CProc3D::GetMaxVal(int* histo, int st, int ed)
{
	if(histo == NULL || st < 0 || st > ed || ed < 0)
		return 0;

	int max = histo[st];
	for(int i = st; i < ed; i++)
	{
		if(max < histo[i])
		{
			max = histo[i];
		}
	}

	return max;
}

void CProc3D::HistogramSave(int* histo, int histoSize)
{
	FILE	*fp;
	wchar_t	fileName[256];
	CString path = _T("D:\\TestImage\\data\\histo.csv");
	wsprintf(fileName, _T("%s"), path);

	DeleteFile(fileName);
	_wfopen_s(&fp,fileName, _T("at"));
	//fp = _wfopen(fileName, _T("at"));
	if (!fp) 
	{
		AfxMessageBox(_T("already opened..."));
		return;
	}
	

	int temp = 0;

	for(int i = 0; i < histoSize ;i++)
	{
		fwprintf(fp, _T("%d,%d\n"),i * HISTO_INTERVAL, histo[i] );			
	}



	fclose(fp);

	//AfxMessageBox(_T("완료!"));
}

void CProc3D::DataSave(float* data, int dataSize, CString path)
{
	FILE	*fp;
	wchar_t	fileName[256];
	wsprintf(fileName, _T("%s"), path);

	DeleteFile(fileName);
	_wfopen_s(&fp,fileName, _T("at"));
	//fp = _wfopen(fileName, _T("at"));
	if (!fp) 
	{
		AfxMessageBox(_T("already opened..."));
		return;
	}


	int temp = 0;

	for(int i = 0; i < dataSize ;i++)
	{
		fwprintf(fp, _T("%d,%f\n"),i , data[i] );			
	}



	fclose(fp);
}

void CProc3D::DataSave(int* data, int dataSize, CString path)
{
	FILE	*fp;
	wchar_t	fileName[256];
	wsprintf(fileName, _T("%s"), path);

	DeleteFile(fileName);
	_wfopen_s(&fp,fileName, _T("at"));
	//fp = _wfopen(fileName, _T("at"));
	if (!fp) 
	{
		AfxMessageBox(_T("already opened..."));
		return;
	}


	int temp = 0;

	for(int i = 0; i < dataSize ;i++)
	{
		fwprintf(fp, _T("%d,%d\n"),i , data[i] );			
	}



	fclose(fp);
}

void CProc3D::DataSave(float* data1, float* data2, int dataSize, CString path)
{
	FILE	*fp;
	wchar_t	fileName[256];
	wsprintf(fileName, _T("%s"), path);

	DeleteFile(fileName);
	_wfopen_s(&fp,fileName, _T("at"));
	//fp = _wfopen(fileName, _T("at"));
	if (!fp) 
	{
		AfxMessageBox(_T("already opened..."));
		return;
	}


	int temp = 0;

	for(int i = 0; i < dataSize ;i++)
	{
		fwprintf(fp, _T("%d,%f,%f\n"),i , data1[i], data2[i] );			
	}



	fclose(fp);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CProc3D::IsHeightOk(float* pZmapData, int roiSizeX, int roiSizeY, float stdH)
{
	BOOL ret = TRUE;

	double sum = 0;
	int cnt = 0;
	int cx = (int)(roiSizeX / 2.0);
	int cy = (int)(roiSizeY / 2.0);
	int width = (int)(roiSizeX * (10.0 / 100.0));
	int length = (int)(roiSizeY * (10.0 / 100.0));
	int stX = (int)(cx - (width / 2.0));
	int stY = (int)(cy - (length / 2.0));
	int pitch = roiSizeX - width;

	float* srcTemp = pZmapData;
	srcTemp += ((stY * roiSizeX) + stX);
	for(int y = 0; y < length; y++)
	{		
		for(int x = 0; x < width; x++)
		{
			sum += *srcTemp++;
			cnt++;
		}
		srcTemp += pitch;
	}

	float hAvr = (float)(sum / cnt);

	if(hAvr < stdH)
		ret = FALSE;

	return ret;
}

BOOL CProc3D::GetRoughGrayImage(float* pZmapRoiData, int roiSizeX, int roiSizeY, double range, float* retThreshold, UCHAR* grayImg_High, UCHAR* grayImg_Low)
{
	BOOL ret = TRUE;
	float stdHighAvrH = 0;
	float stdLowAvrH = 0;
	
// 	CString strMsg;
// 	DWORD st, ed;
// 	double time = 0;
// 	st = GetTickCount();
	GetStdDeltaH(pZmapRoiData, roiSizeX, roiSizeY, range, &stdHighAvrH, &stdLowAvrH);
// 	ed = GetTickCount();
// 	time =  ed-st;
// 	strMsg.Format(_T("%f"),time);
// 	AfxMessageBox(strMsg);

	if(grayImg_High != NULL)
		MakeRoughGrayImg(pZmapRoiData, grayImg_High, roiSizeX, roiSizeY, (int)stdHighAvrH, FALSE);

	ret = IsHeightOk(pZmapRoiData, roiSizeX, roiSizeY, stdHighAvrH);
	

/*	if(!flag)
	{
		int size = roiSizeX * roiSizeY;
		float* zmap = new float[size];
		memset(zmap, 0, sizeof(float) * size);

		float dataMax = 0;
		float dataMin = 0;
		GetHlimit(pZmapRoiData, roiSizeX, roiSizeY, &dataMax, &dataMin);
		RemakeZmap(pZmapRoiData, zmap, roiSizeX, roiSizeY, dataMin, stdHighAvrH, dataMin);

		GetStdDeltaH(zmap, roiSizeX, roiSizeY, range, &stdHighAvrH, &stdLowAvrH);
		MakeRoughGrayImg(zmap, grayImg_High, roiSizeX, roiSizeY, (int)stdHighAvrH, FALSE);

		delete zmap;
	}*/

	if(grayImg_Low != NULL)
		MakeRoughGrayImg(pZmapRoiData, grayImg_Low, roiSizeX, roiSizeY, (int)stdLowAvrH, TRUE);



	*retThreshold = stdHighAvrH;

	return ret;
}

//top과 left는 외부, right와 bottom은 내부에서부터 검색
int CProc3D::SearchEdge(UCHAR** srcProfile, int dataCnt, int dataSize, int pfMargin, CPoint* startCoordi, int position, CPoint** retEdge)
{
	int retEdgCnt = 0;

	CPoint* edgeTemp = NULL;
	//edgeTemp = new CPoint[dataCnt];
	edgeTemp = g_pMManager->pem_new<CPoint>(true, dataCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(edgeTemp, 0, sizeof(CPoint) * dataCnt);

	float temp = 0;
	int index = 0;

	UCHAR stdData = 0;
	UCHAR data = 0;
	for(int i = 0; i < dataCnt; i++)
	{	
		if(srcProfile[i][0] == 0)
			stdData = 255;
		else
			stdData = 0;

		for(int j = 0; j <  dataSize - 1; j++)
		{
			data = srcProfile[i][j];
			if(data == stdData)
			{
				edgeTemp[index] = MakePoint(j, startCoordi[i], position, pfMargin);
				index++;
				break;
			}

		}
	}

	//*retEdge = new CPoint[index];
	*retEdge = g_pMManager->pem_new<CPoint>(true, index, (PCHAR)__FUNCTION__, __LINE__);
	//memset(*retEdge, 0, sizeof(CPoint) * index);
	memcpy_s(*retEdge, sizeof(CPoint) * index, edgeTemp, sizeof(CPoint) * index);


	//delete edgeTemp;
	g_pMManager->pem_delete(edgeTemp, false);

	retEdgCnt = index;
	return retEdgCnt;
}

//모두 외부에서 부터 검색...
int CProc3D::SearchEdge2(UCHAR** srcProfile, int dataCnt, int dataSize, int pfMargin, CPoint* startCoordi, int position, CPoint** retEdge)
{
	int retEdgCnt = 0;

	CPoint* edgeTemp = NULL;
	//edgeTemp = new CPoint[dataCnt];
	edgeTemp = g_pMManager->pem_new<CPoint>(true, dataCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(edgeTemp, 0, sizeof(CPoint) * dataCnt);

	float temp = 0;
	int index = 0;

	UCHAR stdData = 0;
	UCHAR data = 0;

	if(position == e_LEFT || position == e_TOP)
	{
		for(int i = 0; i < dataCnt; i++)
		{	
			if(srcProfile[i][0] == 0)
				stdData = 255;
			else
				stdData = 0;

			for(int j = 0; j <  dataSize - 1; j++)
			{
				data = srcProfile[i][j];
				if(data == stdData)
				{
					edgeTemp[index] = MakePoint(j, startCoordi[i], position, pfMargin);
					index++;
					break;
				}

			}
		}
	}
	else if(position == e_RIGHT || position == e_BOTTOM)
	{	
		for(int i = 0; i < dataCnt; i++)
		{	
			if(srcProfile[i][dataSize-1] == 0)
				stdData = 255;
			else
				stdData = 0;

			for(int j = dataSize - 2; j >= 0; j--)
			{
				data = srcProfile[i][j];
				if(data == stdData)
				{
// 					if(position == e_RIGHT)
// 					{
// 						edgeTemp[index].x = (startCoordi[i].x + dataSize) - j;
// 						edgeTemp[index].y = startCoordi[i].y;
// 
// 						//ret.x = (LONG)(startCoordi.x + calcData);
// 						//ret.y = (LONG)startCoordi.y;
// 						
// 					}
// 					else if(position == e_BOTTOM)
// 					{
// 						edgeTemp[index].x = startCoordi[i].x;
// 						edgeTemp[index].y = (startCoordi[i].y + dataSize) - j;
// 
// 						//ret.x = (LONG)startCoordi.x;
// 						//ret.y = (LONG)(startCoordi.y + calcData);
// 					}
					edgeTemp[index] = MakePoint(j, startCoordi[i], position, pfMargin);
					index++;
					break;
				}

			}
		}
	}

	

	//*retEdge = new CPoint[index];
	*retEdge = g_pMManager->pem_new<CPoint>(true, index, (PCHAR)__FUNCTION__, __LINE__);
	//memset(*retEdge, 0, sizeof(CPoint) * index);
	memcpy_s(*retEdge, sizeof(CPoint) * index, edgeTemp, sizeof(CPoint) * index);


	//delete edgeTemp;
	g_pMManager->pem_delete(edgeTemp, false);

	retEdgCnt = index;
	return retEdgCnt;
}

void CProc3D::EraseBorderLine(UCHAR* userSrc, UCHAR* userDst, int sizeX, int sizeY, int pixelCount, int position)
{
	if(userSrc == NULL || userDst == NULL || sizeX <= 0 || sizeY <= 0 || pixelCount <= 0)
		return;

	int area = sizeX * sizeY;
	memcpy_s(userDst, (sizeof(UCHAR) * area), userSrc, (sizeof(UCHAR) * area));
	
	UCHAR* pDstTemp = NULL;
	
	int size = 0;
	if(position == e_TOP)
	{
		size = sizeX * pixelCount;

		pDstTemp = userDst;
		memset(pDstTemp, 0, sizeof(UCHAR) * size);	
	}
	else if(position == e_BOTTOM)
	{
		size = sizeX * pixelCount;

		pDstTemp = userDst;
		pDstTemp += ((sizeY - pixelCount) - 1) * sizeX;
		memset(pDstTemp, 0, sizeof(UCHAR) * size);	
	}

	else if(position == e_LEFT)
	{
		pDstTemp = userDst;
		for(int i = 0; i < sizeY; i++)
		{
			memset(pDstTemp, 0, sizeof(UCHAR) * pixelCount);	
			pDstTemp += sizeX;
		}
	}
	else if(position == e_RIGHT)
	{
		pDstTemp = userDst;
		pDstTemp += (sizeX - pixelCount) - 1;
		for(int i = 0; i < sizeY; i++)
		{
			memset(pDstTemp, 0, sizeof(UCHAR) * pixelCount);
			pDstTemp += sizeX;
		}
	}
	else
	{
		return;
	}	
}

double CProc3D::ExtractBody4(UCHAR* blobImg, int cx, int cy, int roiSizeX, int roiSizeY, CRect blobRt, double* retCogX, double* retCogY, double* retBodyWidth, double* retBodyHeight, CRect* retRect, POINT* retNodalPoint, UCHAR** retTest1, UCHAR** retTest2, UCHAR** retTest3, UCHAR** retTest4, UCHAR** retTest5, int* retTestSizeX, int* retTestSizeY)
{
	double avrAngle = 0;
	//////////////////////////////////////////////////////////////////////////

	int margin = 10;
	int list1[2] = {1, 9};
	int list2[2] = {2, 3};
	int list3[2] = {4, 6};
	int list4[2] = {8, 12};
	//////////////////////////////////////////////////////////////////////////
	int stX = (int)blobRt.left - margin;
	int stY = (int)blobRt.top - margin;
	int endX = (int)blobRt.right + margin;
	int endY = (int)blobRt.bottom + margin;

	stX = (stX < 1)?	1 : stX;
	stY = (stY < 1)?	1 : stY;
	endX = (endX > roiSizeX-1)?	roiSizeX-1 : endX;
	endY = (endY > roiSizeY-1)?	roiSizeY-1 : endY;

	int rstImgSizeX = endX - stX;
	int rstImgSizeY = endY - stY;
	int rstImgArea = rstImgSizeX * rstImgSizeY;

	//UCHAR* rstImg = new UCHAR[rstImgArea];
	UCHAR* rstImg = g_pMManager->pem_new<UCHAR>(true, rstImgArea, (PCHAR)__FUNCTION__, __LINE__);
	memset(rstImg, 0, sizeof(UCHAR) * rstImgArea);

	int index = 0;
	int rstIndex = 0;
	UCHAR beforData = 0;
	UCHAR data = 0;
	UCHAR afterData = 0;
	int posValue = 255;

	int grpCount[4] = {0,};  // 0 -> 1&9 ,  1 -> 2&3 ,  2 -> 4&6 ,  3 -> 8&12

	for(int y = stY; y < endY; y++)
	{
		for(int x = stX; x < endX; x++)
		{
			index = (y * roiSizeX) + x;
			rstIndex  = ((y - stY)  * rstImgSizeX) + (x - stX);
			
			if(index - 1 > 0 || index + 1 < rstImgArea)
			{	
				//H  ㅡ
				beforData = blobImg[index - 1];
				data = blobImg[index];
				afterData = blobImg[index + 1];

				int subData = beforData - afterData;
				if(data == posValue)
				{
					if(subData == (posValue * -1))
					{
						rstImg[rstIndex] += list2[0];	
						
					}
					else if(subData == posValue)
					{
						rstImg[rstIndex] += list4[0];
					}
				}

				//V	 ㅣ
				beforData = blobImg[index - roiSizeX];
				data = blobImg[index];
				afterData = blobImg[index + roiSizeX];

				subData = beforData - afterData;
				if(data == posValue)
				{
					if(subData == (posValue * -1))
					{						
						rstImg[rstIndex] += list1[0];	
					}
					else if(subData == posValue)
					{					
						rstImg[rstIndex] += list3[0];
					}
				} 

				//count
				data = rstImg[rstIndex];
				if(data == list1[0] || data == list1[1])
					grpCount[0]++;
				else if(data == list2[0] || data == list2[1])
					grpCount[1]++;
				else if(data == list3[0] || data == list3[1])
					grpCount[2]++;
				else if(data == list4[0] || data == list4[1])
					grpCount[3]++;
			}

		}
	}

	//////////////////////////////////////////////////////////////////////////
	//좌표 정리...
	POINT* grpPoint[4] = {NULL,};
	for(int i = 0; i < 4; i++)
	{
		//grpPoint[i] = new POINT[grpCount[i]];
		grpPoint[i] = g_pMManager->pem_new<POINT>(true, grpCount[i], (PCHAR)__FUNCTION__, __LINE__);
		memset(grpPoint[i], 0, sizeof(POINT) * grpCount[i]);
	}


	index = 0;
	data = 0;

	CPoint blobCt = blobRt.CenterPoint();


	int grpIndex1 = 0;
	int grpIndex2 = 0;
	int grpIndex3 = 0;
	int grpIndex4 = 0;
	for(int y = 0; y < rstImgSizeY; y++)
	{
		for(int x = 0; x < rstImgSizeX; x++)
		{
			index = (y * rstImgSizeX) + x;
			data = rstImg[index];

			if(data == list1[0] || data == list1[1])
			{
				if(y + stY < blobCt.y)
				{
					grpPoint[0][grpIndex1].x = (LONG)(x + stX);
					grpPoint[0][grpIndex1].y = (LONG)(y + stY);
					grpIndex1++;
				}
			}
			else if(data == list2[0] || data == list2[1])
			{
				if(x + stX < blobCt.x)				
				{
					grpPoint[1][grpIndex2].x = (LONG)(x + stX);
					grpPoint[1][grpIndex2].y = (LONG)(y + stY);
					grpIndex2++;
				}
			}
			else if(data == list3[0] || data == list3[1])
			{
				if(y + stY > blobCt.y)
				{
					grpPoint[2][grpIndex3].x = (LONG)(x + stX);
					grpPoint[2][grpIndex3].y = (LONG)(y + stY);
					grpIndex3++;
				}
			}
			else if(data == list4[0] || data == list4[1])
			{
				if(x + stX > blobCt.x)
				{
					grpPoint[3][grpIndex4].x = (LONG)(x + stX);
					grpPoint[3][grpIndex4].y = (LONG)(y + stY);
					grpIndex4++;
				}
			}		
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//least square
	float coef1[2];
	float coef2[2];
	float coef3[2];
	float coef4[2];


// 	CalcLineEquation(grpPoint[0], grpCount[0], coef1);//top
// 	CalcLineEquation(grpPoint[1], grpCount[1], coef2);//left
// 	CalcLineEquation(grpPoint[2], grpCount[2], coef3);//bottom
// 	CalcLineEquation(grpPoint[3], grpCount[3], coef4);//right


	//직선 방정식 
	//hough line

	double angle_left =  CalcLineEquation(grpPoint[1], grpIndex2, e_LEFT , roiSizeX, roiSizeY, coef2);
	double angle_right = CalcLineEquation(grpPoint[3], grpIndex4, e_RIGHT , roiSizeX, roiSizeY, coef4); 
	double angle_top = CalcLineEquation(grpPoint[0], grpIndex1, e_TOP , roiSizeX, roiSizeY, coef1); 
	double angle_bottom = CalcLineEquation(grpPoint[2], grpIndex3, e_BOTTOM, roiSizeX, roiSizeY, coef3); 
// 	double angle_left =  CalcLineEquation(grpPoint[1], grpCount[1], e_LEFT , roiSizeX, roiSizeY, coef2);
// 	double angle_right = CalcLineEquation(grpPoint[3], grpCount[3], e_RIGHT , roiSizeX, roiSizeY, coef4); 
// 	double angle_top = CalcLineEquation(grpPoint[0], grpCount[0], e_TOP , roiSizeX, roiSizeY, coef1); 
// 	double angle_bottom = CalcLineEquation(grpPoint[2], grpCount[2], e_BOTTOM, roiSizeX, roiSizeY, coef3); 



	POINT pt[4];
	CalcNodalPoint(coef1, 0, coef2, 90, &pt[0].x, &pt[0].y);	//left_top
	CalcNodalPoint(coef3, 0, coef2, 90, &pt[1].x, &pt[1].y);	//left_bottom
	CalcNodalPoint(coef3, 0, coef4, 90, &pt[2].x, &pt[2].y); //right_bottom
	CalcNodalPoint(coef1, 0, coef4, 90, &pt[3].x, &pt[3].y);		//right_top
	

	if(retNodalPoint != NULL)
	{
		memcpy_s(retNodalPoint, sizeof(POINT) * 4, pt, sizeof(POINT) * 4);
	}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////


	CRect bodyRect;
	bodyRect.left = (LONG)((pt[0].x <= pt[1].x)?		pt[0].x : pt[1].x);
	bodyRect.right = (LONG)((pt[2].x <= pt[3].x)?		pt[3].x : pt[2].x);
	bodyRect.top = (LONG)((pt[0].y <= pt[3].y)?		pt[0].y : pt[3].y);
	bodyRect.bottom = (LONG)((pt[1].y <= pt[2].y)?		pt[2].y : pt[1].y);

	//////////////////////////////////////////////////////////////////////////
	double bodyW_T = 0;
	double bodyW_B = 0;
	double bodyH_L = 0;
	double bodyH_R = 0;


	LONG tempW = abs(pt[3].x - pt[0].x);
	LONG tempH = abs(pt[0].y - pt[3].y);
	bodyW_T = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[2].x - pt[1].x);
	tempH = abs(pt[1].y - pt[2].y);
	bodyW_B = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[0].x - pt[1].x);
	tempH = abs(pt[1].y - pt[0].y);
	bodyH_L = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[2].x - pt[3].x);
	tempH = abs(pt[2].y - pt[3].y);
	bodyH_R = sqrt((double)(tempW * tempW + tempH * tempH));


	double bodyWidth = (bodyW_T + bodyW_B) / 2.0;
	double bodyHeight = (bodyH_L + bodyH_R) / 2.0;

	*retBodyWidth = bodyWidth;
	*retBodyHeight = bodyHeight;
	//////////////////////////////////////////////////////////////////////////


	//ㅡ
/*	double diffAngle_V = 0.;
	double diffAngle_H = 0.;
	double tempAngle1 = angle_top;
	double tempAngle2 = angle_bottom;
	if(angle_top > 90.0)
		tempAngle1 = angle_top - 180.0;
	if(angle_bottom > 90.0)
		tempAngle2 = angle_bottom - 180.0;

	double avrAngle_H = (tempAngle1 + tempAngle2) / 2.0;
	diffAngle_H = abs(tempAngle1 - tempAngle2);

	//ㅣ
	double tempAngle3 = angle_left;
	double tempAngle4 = angle_right;
	double avrAngle_V = ((tempAngle3 + tempAngle4) / 2.0) - 90.0;
	diffAngle_V = abs(tempAngle3 - tempAngle4);

	if(diffAngle_H > diffAngle_V)
		avrAngle = avrAngle_V;
	else
		avrAngle = avrAngle_H;*/

	double tempAngle[4] = {0,};
	double diffAngle_V = 0.;
	double diffAngle_H = 0.;
	tempAngle[0] = angle_top;
	tempAngle[1] = angle_bottom;
	if(angle_top > 90.0)
		tempAngle[0] = angle_top - 180.0;
	if(angle_bottom > 90.0)
		tempAngle[1] = angle_bottom - 180.0;

	//ㅣ
	tempAngle[2] = angle_left - 90;
	tempAngle[3] = angle_right - 90;

	int indexTemp = 0;
	double angleMin = abs(tempAngle[0]);
	for(int i = 0; i < 4; i++)
	{
		if(angleMin > abs(tempAngle[i]))
		{
			angleMin = abs(tempAngle[i]);
			indexTemp = i;
		}
	}

	avrAngle = tempAngle[indexTemp];
	//////////////////////////////////////////////////////////////////////////


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	double bodyCtX = (bodyRect.Width() / 2.0) + bodyRect.left;
	double bodyCtY = (bodyRect.Height() / 2.0) + bodyRect.top;

	double cogX = (cx - (roiSizeX / 2.0)) + bodyCtX;
	double cogY = (cy - (roiSizeY / 2.0)) + bodyCtY;


	*retCogX = cogX;
	*retCogY = cogY;
	*retRect = bodyRect;


	
	//////////////////////////////////////////////////////////////////////////
	//test용	
/*	UCHAR* imgTemp1 = new UCHAR[rstImgArea];
	memset(imgTemp1, 0, sizeof(UCHAR) * rstImgArea);
	UCHAR* imgTemp2 = new UCHAR[rstImgArea];
	memset(imgTemp2, 0, sizeof(UCHAR) * rstImgArea);
	UCHAR* imgTemp3 = new UCHAR[rstImgArea];
	memset(imgTemp3, 0, sizeof(UCHAR) * rstImgArea);
	UCHAR* imgTemp4 = new UCHAR[rstImgArea];
	memset(imgTemp4, 0, sizeof(UCHAR) * rstImgArea);
	UCHAR* imgTemp5= new UCHAR[rstImgArea];
	memset(imgTemp5, 0, sizeof(UCHAR) * rstImgArea);

	int viewIndex = 0;
	for(int i = 0; i < grpCount[viewIndex]; i++)
	{
		index = ((grpPoint[viewIndex][i].y - stY) * rstImgSizeX) + (grpPoint[viewIndex][i].x - stX);
		imgTemp1[index] = 255;
		imgTemp5[index] = 255;
	}

	viewIndex = 1;
	for(int i = 0; i < grpCount[viewIndex]; i++)
	{
		index = ((grpPoint[viewIndex][i].y - stY) * rstImgSizeX) + (grpPoint[viewIndex][i].x - stX);
		imgTemp2[index] = 255;
		imgTemp5[index] = 255;
	}

	viewIndex = 2;
	for(int i = 0; i < grpCount[viewIndex]; i++)
	{
		index = ((grpPoint[viewIndex][i].y - stY) * rstImgSizeX) + (grpPoint[viewIndex][i].x - stX);
		imgTemp3[index] = 255;
		imgTemp5[index] = 255;
	}

	viewIndex = 3;
	for(int i = 0; i < grpCount[viewIndex]; i++)
	{
		index = ((grpPoint[viewIndex][i].y - stY) * rstImgSizeX) + (grpPoint[viewIndex][i].x - stX);
		imgTemp4[index] = 255;
		imgTemp5[index] = 255;
	}


	*retTest1 = new UCHAR[rstImgArea];
	memset(*retTest1, 0, sizeof(UCHAR) * rstImgArea);
	memcpy_s(*retTest1, sizeof(UCHAR) * rstImgArea, imgTemp1, sizeof(UCHAR) * rstImgArea );

	*retTest2 = new UCHAR[rstImgArea];
	memset(*retTest2, 0, sizeof(UCHAR) * rstImgArea);
	memcpy_s(*retTest2, sizeof(UCHAR) * rstImgArea, imgTemp2, sizeof(UCHAR) * rstImgArea );

	*retTest3 = new UCHAR[rstImgArea];
	memset(*retTest3, 0, sizeof(UCHAR) * rstImgArea);
	memcpy_s(*retTest3, sizeof(UCHAR) * rstImgArea, imgTemp3, sizeof(UCHAR) * rstImgArea );

	*retTest4 = new UCHAR[rstImgArea];
	memset(*retTest4, 0, sizeof(UCHAR) * rstImgArea);
	memcpy_s(*retTest4, sizeof(UCHAR) * rstImgArea, imgTemp4, sizeof(UCHAR) * rstImgArea );

	*retTest5 = new UCHAR[rstImgArea];
	memset(*retTest5, 0, sizeof(UCHAR) * rstImgArea);
	memcpy_s(*retTest5, sizeof(UCHAR) * rstImgArea, imgTemp5, sizeof(UCHAR) * rstImgArea );


	*retTestSizeX = rstImgSizeX;
	*retTestSizeY = rstImgSizeY;


	
	delete imgTemp1;
	delete imgTemp2;
	delete imgTemp3;
	delete imgTemp4;
	delete imgTemp5;
	//////////////////////////////////////////////////////////////////////////
	
*/

	//delete rstImg;
	g_pMManager->pem_delete(rstImg, false);
	for(int i = 0; i < 4; i++)
	{
		//delete grpPoint[i];
		g_pMManager->pem_delete(grpPoint[i], false);
	}
	
	return avrAngle;
}

double CProc3D::ExtractBody3(float* pZmapRoiData, int cx, int cy, int roiSizeX, int roiSizeY, double angle, UCHAR* blobImg, UCHAR* blobImg_board, int blobCx, int blobCy, CRect blobRt, double* retCogX, double* retCogY, double* retBodyWidth, double* retBodyHeight, CRect* retRect, BYTE* lutData, UCHAR* img, POINT* retNodalPoint)
{
	double avrAngle = 0;

	int edgeCnts = EDGE_CNTS;
	int stMargin = ST_MARGIN;
	int dataCnts_row = 0;
	int dataCnts_col = 0;
	int roiArea = roiSizeX * roiSizeY;

	if(blobRt.Width() < (edgeCnts + 1))
	{
		dataCnts_col = blobRt.Width() - 1;
	}
	else
	{
		dataCnts_col = edgeCnts;
	}

	if(blobRt.Height() < (edgeCnts + 1))
	{
		dataCnts_row = blobRt.Height() - 1;
	}	
	else
	{
		dataCnts_row = edgeCnts;	
	}

	if(dataCnts_col <= 0 || dataCnts_row <= 0)
		return -1;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	/*CPoint* stRowLeft = new CPoint[dataCnts_row];
	CPoint* stRowRight = new CPoint[dataCnts_row];
	CPoint* stColTop = new CPoint[dataCnts_col];
	CPoint* stColBottom = new CPoint[dataCnts_col];*/
	CPoint* stRowLeft = g_pMManager->pem_new<CPoint>(true, dataCnts_row, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stRowRight = g_pMManager->pem_new<CPoint>(true, dataCnts_row, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColTop = g_pMManager->pem_new<CPoint>(true, dataCnts_col, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColBottom = g_pMManager->pem_new<CPoint>(true, dataCnts_col, (PCHAR)__FUNCTION__, __LINE__);


	CalcStartPoint2(blobCx, blobCy, blobRt, stMargin, dataCnts_row, dataCnts_col, stRowLeft, stRowRight, stColTop, stColBottom);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	UCHAR** profileData_left = NULL;
	int pfSt_left = (int)(blobRt.left / 2.0);
	int sz_left = (int)(stRowLeft[0].x - pfSt_left);
	alpa_make_array_2d(&profileData_left, dataCnts_row, sz_left);

	UCHAR** profileData_right = NULL;
	int pfEd_right = (int)((roiSizeX - blobRt.right) / 2.0);
	int sz_right =(int)((roiSizeX - stRowRight[0].x) - pfEd_right);
	alpa_make_array_2d(&profileData_right, dataCnts_row, sz_right);

	UCHAR** profileData_top = NULL;
	int pfSt_top = (int)(blobRt.top / 2.0);
	int sz_top = (int)(stColTop[0].y - pfSt_top);
	alpa_make_array_2d(&profileData_top, dataCnts_col, sz_top);

	UCHAR** profileData_bottom = NULL;
	int pfEd_bottom = (int)((roiSizeY - blobRt.bottom) / 2.0);
	int sz_bottom = (int)((roiSizeY - stColBottom[0].y) - pfEd_bottom);
	alpa_make_array_2d(&profileData_bottom, dataCnts_col, sz_bottom);

	//extract data (left, right, top, bottom)
	CString fileName = _T("");	
	for(int i = 0; i < dataCnts_row; i++)
	{
		MakeProfileData_H(blobImg, profileData_left[i], roiSizeX, roiSizeY, pfSt_left, (int)stRowLeft[i].y, sz_left);
		MakeProfileData_H(blobImg, profileData_right[i], roiSizeX, roiSizeY, (int)stRowRight[i].x, (int)stRowRight[i].y, sz_right);
	}

	for(int i = 0; i < dataCnts_col; i++)
	{
		MakeProfileData_V(blobImg, profileData_top[i], roiSizeX, roiSizeY, (int)stColTop[i].x, pfSt_top, sz_top);
		MakeProfileData_V(blobImg, profileData_bottom[i], roiSizeX, roiSizeY, (int)stColBottom[i].x, (int)stColBottom[i].y, sz_bottom);
	}
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//



	int deltaMaxCnt_left;
	CPoint* deltaMax_left;

	int deltaMaxCnt_right;
	CPoint* deltaMax_right;

	int deltaMaxCnt_top;
	CPoint* deltaMax_top;

	int deltaMaxCnt_bottom;
	CPoint* deltaMax_bottom;

	deltaMaxCnt_left = SearchEdge2(profileData_left, dataCnts_row, sz_left, pfSt_left, stRowLeft, e_LEFT, &deltaMax_left);
	deltaMaxCnt_right = SearchEdge2(profileData_right, dataCnts_row, sz_right, pfEd_right, stRowRight, e_RIGHT, &deltaMax_right);
	deltaMaxCnt_top = SearchEdge2(profileData_top, dataCnts_col, sz_top, pfSt_top, stColTop, e_TOP, &deltaMax_top);
	deltaMaxCnt_bottom = SearchEdge2(profileData_bottom, dataCnts_col, sz_bottom, pfEd_bottom, stColBottom, e_BOTTOM, &deltaMax_bottom);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	float coef_left[2];
	float coef_right[2];
	float coef_top[2];
	float coef_bottom[2];

	//직선 방정식 
	//hough line
	double angle_left = CalcLineEquation(deltaMax_left, deltaMaxCnt_left, e_LEFT, roiSizeX, roiSizeY, stRowLeft, coef_left);
	double angle_right = CalcLineEquation(deltaMax_right, deltaMaxCnt_right, e_RIGHT, roiSizeX, roiSizeY, stRowRight, coef_right);
	double angle_top = CalcLineEquation(deltaMax_top, deltaMaxCnt_top, e_TOP, roiSizeX, roiSizeY, stColTop, coef_top);
	double angle_bottom = CalcLineEquation(deltaMax_bottom, deltaMaxCnt_bottom, e_BOTTOM, roiSizeX, roiSizeY, stColBottom, coef_bottom);


	//least square
// 	float coef_left2[2];
// 	float coef_right2[2];
// 	float coef_top2[2];
// 	float coef_bottom2[2];
// 
// 
// 	CalcLineEquation(deltaMax_left, deltaMaxCnt_left, coef_left2);
// 	CalcLineEquation(deltaMax_right, deltaMaxCnt_right, coef_right2);
// 	CalcLineEquation(deltaMax_top, deltaMaxCnt_top, coef_top2);
// 	CalcLineEquation(deltaMax_bottom, deltaMaxCnt_bottom, coef_bottom2);

	//////////////////////////////////////////////////////////////////////////
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//교점
	POINT pt[4];
 	CalcNodalPoint(coef_left, angle_left, coef_top, angle_top, &pt[0].x, &pt[0].y);			//left_top
 	CalcNodalPoint(coef_left, angle_left, coef_bottom, angle_bottom, &pt[1].x, &pt[1].y);	//left_bottom
 	CalcNodalPoint(coef_right, angle_right, coef_bottom, angle_bottom, &pt[2].x, &pt[2].y); //right_bottom
 	CalcNodalPoint(coef_right, angle_right, coef_top, angle_top, &pt[3].x, &pt[3].y);		//right_top



	if(retNodalPoint != NULL)
	{
		memcpy_s(retNodalPoint, sizeof(POINT) * 4, pt, sizeof(POINT) * 4);
	}

	CRect bodyRect;
	bodyRect.left = (LONG)((pt[0].x <= pt[1].x)?		pt[0].x : pt[1].x);
	bodyRect.right = (LONG)((pt[2].x <= pt[3].x)?		pt[3].x : pt[2].x);
	bodyRect.top = (LONG)((pt[0].y <= pt[3].y)?		pt[0].y : pt[3].y);
	bodyRect.bottom = (LONG)((pt[1].y <= pt[2].y)?		pt[2].y : pt[1].y);

	//////////////////////////////////////////////////////////////////////////
	double bodyW_T = 0;
	double bodyW_B = 0;
	double bodyH_L = 0;
	double bodyH_R = 0;


	LONG tempW = abs(pt[3].x - pt[0].x);
	LONG tempH = abs(pt[0].y - pt[3].y);
	bodyW_T = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[2].x - pt[1].x);
	tempH = abs(pt[1].y - pt[2].y);
	bodyW_B = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[0].x - pt[1].x);
	tempH = abs(pt[1].y - pt[0].y);
	bodyH_L = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[2].x - pt[3].x);
	tempH = abs(pt[2].y - pt[3].y);
	bodyH_R = sqrt((double)(tempW * tempW + tempH * tempH));


	double bodyWidth = (bodyW_T + bodyW_B) / 2.0;
	double bodyHeight = (bodyH_L + bodyH_R) / 2.0;

	*retBodyWidth = bodyWidth;
	*retBodyHeight = bodyHeight;
	//////////////////////////////////////////////////////////////////////////


	//ㅡ
	double diffAngle_V = 0.;
	double diffAngle_H = 0.;
	double tempAngle1 = angle_top;
	double tempAngle2 = angle_bottom;
	if(angle_top > 90.0)
		tempAngle1 = angle_top - 180.0;
	if(angle_bottom > 90.0)
		tempAngle2 = angle_bottom - 180.0;

	double avrAngle_H = (tempAngle1 + tempAngle2) / 2.0;
	diffAngle_H = abs(tempAngle1 - tempAngle2);

	//ㅣ
	tempAngle1 = angle_left;
	tempAngle2 = angle_right;
	double avrAngle_V = ((tempAngle1 + tempAngle2) / 2.0) - 90.0;
	diffAngle_V = abs(tempAngle1 - tempAngle2);


	/*if(bodyWidth > bodyHeight)
		avrAngle = avrAngle_H;
	else
		avrAngle = avrAngle_V;*/

	if(diffAngle_H > diffAngle_V)
		avrAngle = avrAngle_V;
	else
		avrAngle = avrAngle_H;

	//////////////////////////////////////////////////////////////////////////

	


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	double bodyCtX = (bodyRect.Width() / 2.0) + bodyRect.left;
	double bodyCtY = (bodyRect.Height() / 2.0) + bodyRect.top;


	int widthTemp = 0;
	int lengthTemp = 0;
	double bodyCtX_temp = 0;
	double bodyCtY_temp = 0;

	if(angle ==  90 || angle == 270)
	{
		widthTemp = roiSizeY;
		lengthTemp = roiSizeX;

		bodyCtX_temp = bodyCtY;
		bodyCtY_temp = bodyCtX;
	}
	else
	{
		widthTemp = roiSizeX;
		lengthTemp = roiSizeY;

		bodyCtX_temp = bodyCtX;
		bodyCtY_temp = bodyCtY;
	}
	double cogX = (cx - (widthTemp / 2.0)) + bodyCtX_temp;
	double cogY = (cy - (lengthTemp / 2.0)) + bodyCtY_temp;


	*retCogX = cogX;
	*retCogY = cogY;
	*retRect = bodyRect;



	////////////////////////////////////////////////////////////////////////// make profile image  (debug)
 	if(img != NULL)
 	{
		int index_top = 0;
		int index_bottom = 0;


		int ctX = blobCx;//roiSizeX / 2;
		int ctY = blobCy;//roiSizeY / 2;	

		int distX = (int)(blobRt.Width() / (dataCnts_col + 1.0) + 0.5);
		int distY = (int)(blobRt.Height() / (dataCnts_row + 1.0) + 0.5);

// 		int distX = (roiSizeX ) / (dataCnts_col + 1);
// 		int distY = (roiSizeY ) / (dataCnts_row + 1);



		UCHAR* profileImg = NULL;
		//profileImg = new UCHAR[roiArea];
		profileImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
		memset(profileImg, 0, sizeof(UCHAR) * roiArea);

		UCHAR* pDst = NULL;
		UCHAR* pSrc = NULL;
		for(int i = 0; i < dataCnts_row; i++)
		{		
			pDst = profileImg;
			pDst += (int)((roiSizeX * stRowLeft[i].y) + pfSt_left);
			memcpy_s(pDst, (sizeof(UCHAR) * sz_left), profileData_left[i], (sizeof(UCHAR) * sz_left));	

		}


		for(int i = 0; i < dataCnts_row; i++)
		{				
			pDst = profileImg;
			pDst += ((roiSizeX * stRowRight[i].y) + stRowRight[i].x);
			memcpy_s(pDst, (sizeof(UCHAR) * sz_right), profileData_right[i], (sizeof(UCHAR) * sz_right));		
		}

		index_top = 0;
		index_bottom = 0;

		for(int y = 0; y < roiSizeY; y++)
		{
			if(y < (int)stColTop[0].y && y > pfSt_top)
			{	
				pDst = profileImg;
				pDst += (roiSizeX * y) + stColTop[0].x;
				for(int i = 0; i < dataCnts_col; i++)
				{
					pSrc = profileData_top[i];
					pSrc += index_top;				

					*pDst = *pSrc;
					pDst += distX;
				}
				index_top++;
			}

			if(y >= (int)stColBottom[0].y && y < (roiSizeY - pfEd_bottom))
			{	
				pDst = profileImg;
				pDst += (roiSizeX * y) + stColBottom[0].x;
				for(int i = 0; i < dataCnts_col; i++)
				{
					pSrc = profileData_bottom[i];
					pSrc += index_bottom;

					*pDst = *pSrc;	
					pDst += distX;
				}
				index_bottom++;
			}

		}

		memcpy_s(img, (sizeof(UCHAR) * roiArea), profileImg, (sizeof(UCHAR) * roiArea));




		//delete profileImg;
		g_pMManager->pem_delete(profileImg, false);

	}
	//////////////////////////////////////////////////////////////////////////

	/*delete stRowLeft;
	delete stRowRight;
	delete stColTop;
	delete stColBottom;*/
	g_pMManager->pem_delete(stRowLeft, false);
	g_pMManager->pem_delete(stRowRight, false);
	g_pMManager->pem_delete(stColTop, false);
	g_pMManager->pem_delete(stColBottom, false);

	/*delete deltaMax_left;
	delete deltaMax_right; 
	delete deltaMax_top;
	delete deltaMax_bottom;*/
	g_pMManager->pem_delete(deltaMax_left, false);
	g_pMManager->pem_delete(deltaMax_right, false);
	g_pMManager->pem_delete(deltaMax_top, false);
	g_pMManager->pem_delete(deltaMax_bottom, false);

	alpa_delete_array_2d(&profileData_left, dataCnts_row);
	alpa_delete_array_2d(&profileData_right, dataCnts_row);
	alpa_delete_array_2d(&profileData_top, dataCnts_col);
	alpa_delete_array_2d(&profileData_bottom, dataCnts_col);



	return avrAngle;
}

float CProc3D::ExtractBody2(float* pZmapRoiData, float* bodyDst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY, double angle, double* retCogX, double* retCogY, CRect* retRect, UCHAR* img)
{
	float hAvr = 0;
	if(pZmapRoiData == NULL  || orgSizeX < 0 || orgSizeY < 0 || cx < 0 || cy < 0 || roiSizeX < 0 || roiSizeY < 0)
		return -1;

	int edgeCnts = EDGE_CNTS;
	int stMargin = ST_MARGIN;
	int dataCnts_row = 0;
	int dataCnts_col = 0;

	if(roiSizeX < (edgeCnts + 1))
	{
		dataCnts_col = roiSizeX - 1;
	}
	else
	{
		dataCnts_col = edgeCnts;
	}

	if(roiSizeY < (edgeCnts + 1))
	{
		dataCnts_row = roiSizeY - 1;
	}	
	else
	{
		dataCnts_row = edgeCnts;	
	}

	if(dataCnts_col <= 0 || dataCnts_row <= 0)
		return -1;

	int tempSize = STD_AREA_SIZE;
	int tempCx = roiSizeX / 2;
	int tempCy = roiSizeY / 2;

	float ftH = 0;
	float stdH = GetStdDeltaH(pZmapRoiData, roiSizeX, roiSizeY, 0.2, &ftH);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	/*CPoint* stRowLeft = new CPoint[dataCnts_row];
	CPoint* stRowRight = new CPoint[dataCnts_row];
	CPoint* stColTop = new CPoint[dataCnts_col];
	CPoint* stColBottom = new CPoint[dataCnts_col];*/
	CPoint* stRowLeft = g_pMManager->pem_new<CPoint>(true, dataCnts_row, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stRowRight = g_pMManager->pem_new<CPoint>(true, dataCnts_row, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColTop = g_pMManager->pem_new<CPoint>(true, dataCnts_col, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColBottom = g_pMManager->pem_new<CPoint>(true, dataCnts_col, (PCHAR)__FUNCTION__, __LINE__);


	CalcStartPoint(roiSizeX, roiSizeY, dataCnts_row, dataCnts_col, stMargin, stRowLeft, stRowRight, stColTop, stColBottom);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	float** profileData_left = NULL;
	int sz_left = (int)stRowLeft[0].x;
	alpa_make_array_2d(&profileData_left, dataCnts_row, sz_left);

	float** profileData_right = NULL;
	int sz_right = roiSizeX - (int)stRowRight[0].x;
	alpa_make_array_2d(&profileData_right, dataCnts_row, sz_right);

	float** profileData_top = NULL;
	int sz_top = (int)stColTop[0].y;
	alpa_make_array_2d(&profileData_top, dataCnts_col, sz_top);

	float** profileData_bottom = NULL;
	int sz_bottom = roiSizeY - (int)stColBottom[0].y;
	alpa_make_array_2d(&profileData_bottom, dataCnts_col, sz_bottom);

	//extract data (left, right, top, bottom)
	CString fileName = _T("");	
	for(int i = 0; i < dataCnts_row; i++)
	{
		MakeProfileData_H(pZmapRoiData, profileData_left[i], roiSizeX, roiSizeY, 0, (int)stRowLeft[i].y, sz_left);
		MakeProfileData_H(pZmapRoiData, profileData_right[i], roiSizeX, roiSizeY, (int)stRowRight[i].x, (int)stRowRight[i].y, sz_right);

		// 		#ifdef _DEBUG
		// 		fileName.Format(_T("D:\\TestImage\\data\\left_%d.csv"),i);
		// 		DataSave(profileData_left[i], sz_left, fileName);
		// 		fileName.Format(_T("D:\\TestImage\\data\\right_%d.csv"),i);
		// 		DataSave(profileData_right[i], sz_right, fileName);		
		// 		#endif
	}

	for(int i = 0; i < dataCnts_col; i++)
	{
		MakeProfileData_V(pZmapRoiData, profileData_top[i], roiSizeX, roiSizeY, (int)stColTop[i].x, 0, sz_top);
		MakeProfileData_V(pZmapRoiData, profileData_bottom[i], roiSizeX, roiSizeY, (int)stColBottom[i].x, (int)stColBottom[i].y, sz_bottom);

		// 		#ifdef _DEBUG
		// 		fileName.Format(_T("D:\\TestImage\\data\\top_%d.csv"),i);
		// 		DataSave(profileData_top[i], sz_top, fileName);
		// 		fileName.Format(_T("D:\\TestImage\\data\\bottom_%d.csv"),i);
		// 		DataSave(profileData_bottom[i], sz_bottom, fileName);
		// 		#endif
	}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


	int deltaMaxCnt_left;
	CPoint* deltaMax_left;

	int deltaMaxCnt_right;
	CPoint* deltaMax_right;

	int deltaMaxCnt_top;
	CPoint* deltaMax_top;

	int deltaMaxCnt_bottom;
	CPoint* deltaMax_bottom;



	deltaMaxCnt_left = SearchEdge(profileData_left, dataCnts_row, sz_left, ftH, stdH, SEARCH_PIXEL, stRowLeft, e_LEFT, &deltaMax_left);
	deltaMaxCnt_right = SearchEdge(profileData_right, dataCnts_row, sz_right, ftH, stdH, SEARCH_PIXEL, stRowRight, e_RIGHT, &deltaMax_right);
	deltaMaxCnt_top = SearchEdge(profileData_top, dataCnts_col, sz_top, stdH, ftH, SEARCH_PIXEL, stColTop, e_TOP, &deltaMax_top);
	deltaMaxCnt_bottom = SearchEdge(profileData_bottom, dataCnts_col, sz_bottom, ftH, stdH, SEARCH_PIXEL, stColBottom, e_BOTTOM, &deltaMax_bottom);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	float coef_left[2];
	float coef_right[2];
	float coef_top[2];
	float coef_bottom[2];

	//직선 방정식 
	//hough line
	double angle_left = CalcLineEquation(deltaMax_left, deltaMaxCnt_left, e_LEFT, roiSizeX, roiSizeY, stRowLeft, coef_left);
	double angle_right = CalcLineEquation(deltaMax_right, deltaMaxCnt_right, e_RIGHT, roiSizeX, roiSizeY, stRowRight, coef_right);
	double angle_top = CalcLineEquation(deltaMax_top, deltaMaxCnt_top, e_TOP, roiSizeX, roiSizeY, stColTop, coef_top);
	double angle_bottom = CalcLineEquation(deltaMax_bottom, deltaMaxCnt_bottom, e_BOTTOM, roiSizeX, roiSizeY, stColBottom, coef_bottom);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	//교점
	POINT pt[4];
	CalcNodalPoint(coef_left, angle_left, coef_top, angle_top, &pt[0].x, &pt[0].y);			//left_top
	CalcNodalPoint(coef_left, angle_left, coef_bottom, angle_bottom, &pt[1].x, &pt[1].y);	//left_bottom
	CalcNodalPoint(coef_right, angle_right, coef_bottom, angle_bottom, &pt[2].x, &pt[2].y); //right_bottom
	CalcNodalPoint(coef_right, angle_right, coef_top, angle_top, &pt[3].x, &pt[3].y);		//right_top


	CRect bodyRect;
	bodyRect.left = (long)((pt[0].x + pt[1].x) / 2.0);
	bodyRect.right = (long)((pt[2].x + pt[3].x) / 2.0);
	bodyRect.top = (long)((pt[0].y + pt[3].y) / 2.0);
	bodyRect.bottom = (long)((pt[1].y + pt[2].y) / 2.0);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	BYTE* lutData = NULL;
	//lutData = new BYTE[roiSizeX * roiSizeY];
	lutData = g_pMManager->pem_new<BYTE>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(lutData, 0, (sizeof(BYTE) * roiSizeX * roiSizeY));

	MakeLUT_Poly(lutData, roiSizeX, roiSizeY, pt, 4);

	float dataMax = 0;
	float dataMin = 0;
	GetHlimit(pZmapRoiData, roiSizeX, roiSizeY, &dataMax, &dataMin);
	float avrH = MaskingData(pZmapRoiData, lutData, bodyDst, roiSizeX, roiSizeY, dataMin);

	//delete lutData;
	g_pMManager->pem_delete(lutData, false);

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	double bodyCtX = (bodyRect.Width() / 2.0) + bodyRect.left;
	double bodyCtY = (bodyRect.Height() / 2.0) + bodyRect.top;


	int widthTemp = 0;
	int lengthTemp = 0;
	double bodyCtX_temp = 0;
	double bodyCtY_temp = 0;

	if(angle ==  90 || angle == 270)
	{
		widthTemp = roiSizeY;
		lengthTemp = roiSizeX;

		bodyCtX_temp = bodyCtY;
		bodyCtY_temp = bodyCtX;
	}
	else
	{
		widthTemp = roiSizeX;
		lengthTemp = roiSizeY;

		bodyCtX_temp = bodyCtX;
		bodyCtY_temp = bodyCtY;
	}
	double cogX = (cx - (widthTemp / 2.0)) + bodyCtX_temp;
	double cogY = (cy - (lengthTemp / 2.0)) + bodyCtY_temp;


	*retCogX = cogX;
	*retCogY = cogY;
	*retRect = bodyRect;

	////////////////////////////////////////////////////////////////////////// make profile image  (debug)
	if(img != NULL)
	{
		int index_top = 0;
		int index_bottom = 0;
		UCHAR* gray_left[EDGE_CNTS];
		UCHAR* gray_right[EDGE_CNTS];
		UCHAR* gray_top[EDGE_CNTS];
		UCHAR* gray_bottom[EDGE_CNTS];

		int ctX = roiSizeX / 2;
		int ctY = roiSizeY / 2;	
		int distX = (roiSizeX ) / (dataCnts_col + 1);
		int distY = (roiSizeY ) / (dataCnts_row + 1);

		for(int i = 0; i < dataCnts_row; i++)
		{
			//gray_left[i] = new UCHAR[sz_left];
			gray_left[i] = g_pMManager->pem_new<UCHAR>(true, sz_left, (PCHAR)__FUNCTION__, __LINE__);
			NormalizeData_Gray(profileData_left[i], gray_left[i], sz_left, 1);

			//gray_right[i] = new UCHAR[sz_right];
			gray_right[i] = g_pMManager->pem_new<UCHAR>(true, sz_right, (PCHAR)__FUNCTION__, __LINE__);
			NormalizeData_Gray(profileData_right[i], gray_right[i], sz_right, 1);
		}

		for(int i = 0; i < dataCnts_col; i++)
		{
			//gray_top[i] = new UCHAR[sz_top];
			gray_top[i] = g_pMManager->pem_new<UCHAR>(true, sz_top, (PCHAR)__FUNCTION__, __LINE__);
			NormalizeData_Gray(profileData_top[i], gray_top[i], 1 , sz_top);

			//gray_bottom[i] = new UCHAR[sz_bottom];
			gray_bottom[i] = g_pMManager->pem_new<UCHAR>(true, sz_bottom, (PCHAR)__FUNCTION__, __LINE__);
			NormalizeData_Gray(profileData_bottom[i], gray_bottom[i], 1 , sz_bottom);
		}





		UCHAR* profileImg = NULL;
		//profileImg = new UCHAR[roiSizeX * roiSizeY];
		profileImg = g_pMManager->pem_new<UCHAR>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
		//memset(profileImg, 0, sizeof(UCHAR) * roiSizeX * roiSizeY);

		UCHAR* pDst = NULL;
		UCHAR* pSrc = NULL;
		for(int i = 0; i < dataCnts_row; i++)
		{		
			pDst = profileImg;
			pDst += (roiSizeX * stRowLeft[i].y);
			memcpy_s(pDst, (sizeof(UCHAR) * sz_left), gray_left[i], (sizeof(UCHAR) * sz_left));	

		}


		for(int i = 0; i < dataCnts_row; i++)
		{				
			pDst = profileImg;
			pDst += ((roiSizeX * stRowRight[i].y) + stRowRight[i].x);
			memcpy_s(pDst, (sizeof(UCHAR) * sz_right), gray_right[i], (sizeof(UCHAR) * sz_right));		
		}

		index_top = 0;
		index_bottom = 0;

		for(int y = 0; y < roiSizeY; y++)
		{
			if(y < (int)stColTop[0].y)
			{	
				pDst = profileImg;
				pDst += (roiSizeX * y) + stColTop[0].x;
				for(int i = 0; i < dataCnts_col; i++)
				{
					pSrc = gray_top[i];
					pSrc += index_top;				

					*pDst = *pSrc;
					pDst += distX;
				}
				index_top++;
			}

			if(y >= (int)stColBottom[0].y)
			{	
				pDst = profileImg;
				pDst += (roiSizeX * y) + stColBottom[0].x;
				for(int i = 0; i < dataCnts_col; i++)
				{
					pSrc = gray_bottom[i];
					pSrc += index_bottom;

					*pDst = *pSrc;	
					pDst += distX;
				}
				index_bottom++;
			}

		}

		memcpy_s(img, (sizeof(UCHAR) * roiSizeX * roiSizeY), profileImg, (sizeof(UCHAR) * roiSizeX * roiSizeY));



		for(int i = 0; i < dataCnts_row; i++)
		{
			//delete gray_left[i];
			g_pMManager->pem_delete(gray_left[i], false);
		}
		for(int i = 0; i < dataCnts_row; i++)
		{
			//delete gray_right[i];
			g_pMManager->pem_delete(gray_right[i], false);
		}
		for(int i = 0; i < dataCnts_col; i++)
		{
			//delete gray_top[i];
			g_pMManager->pem_delete(gray_top[i], false);
		}
		for(int i = 0; i < dataCnts_col; i++)
		{
			//delete gray_bottom[i];
			g_pMManager->pem_delete(gray_bottom[i], false);
		}

		//delete profileImg;
		g_pMManager->pem_delete(profileImg, false);

	}


	//////////////////////////////////////////////////////////////////////////

	/*delete deltaMax_left;
	delete deltaMax_right; 
	delete deltaMax_top;
	delete deltaMax_bottom;*/
	g_pMManager->pem_delete(deltaMax_left, false);
	g_pMManager->pem_delete(deltaMax_right, false);
	g_pMManager->pem_delete(deltaMax_top, false);
	g_pMManager->pem_delete(deltaMax_bottom, false);


	/*delete stRowLeft;
	delete stRowRight;
	delete stColTop;
	delete stColBottom;*/
	g_pMManager->pem_delete(stRowLeft, false);
	g_pMManager->pem_delete(stRowRight, false);
	g_pMManager->pem_delete(stColTop, false);
	g_pMManager->pem_delete(stColBottom, false);
	//	delete pZmapRoiData;

	alpa_delete_array_2d(&profileData_left, dataCnts_row);
	alpa_delete_array_2d(&profileData_right, dataCnts_row);
	alpa_delete_array_2d(&profileData_top, dataCnts_col);
	alpa_delete_array_2d(&profileData_bottom, dataCnts_col);

	hAvr = avrH;
	return hAvr;
}

int CProc3D::SearchEdge(float** data, int dataCnt, int dataSize, float filterH, float stdH, int stdLen, CPoint* startCoordi, int position, CPoint** retEdge)
{
	//data[dataCnt][dataSize]  

	int retEdgCnt = 0;

	CPoint* edgeTemp = NULL;
	//edgeTemp = new CPoint[dataCnt * dataSize];
	edgeTemp = g_pMManager->pem_new<CPoint>(true, dataCnt * dataSize, (PCHAR)__FUNCTION__, __LINE__);
	memset(edgeTemp, 0, sizeof(CPoint) * dataCnt * dataSize);

	float temp = 0;
	int index = 0;

	for(int i = 0; i < dataCnt; i++)
	{	
		if(IsBody(data[i], dataSize, stdLen, filterH, position))
		{
			for(int j = 0; j <  dataSize - 1; j++)
			{
				temp = (stdH - data[i][j]) * (stdH - data[i][j + 1]);	

				if(temp < 0)
				{
					edgeTemp[index] = MakePoint(j, startCoordi[i], position);
					index++;
				}
			}
		}
	}

	//*retEdge = new CPoint[index];
	*retEdge = g_pMManager->pem_new<CPoint>(true, index, (PCHAR)__FUNCTION__, __LINE__);
	//memset(*retEdge, 0, sizeof(CPoint) * index);
	memcpy_s(*retEdge, sizeof(CPoint) * index, edgeTemp, sizeof(CPoint) * index);


	//delete edgeTemp;
	g_pMManager->pem_delete(edgeTemp, false);

	retEdgCnt = index;
	return retEdgCnt;
}

BOOL CProc3D::IsBody(float* data, int dataSize, int searchLen, float filterH, int position)
{
	BOOL ret = TRUE;
	if(searchLen > dataSize)
		return FALSE;

	float sum = 0;
	float avr = 0;
	int st = 0;
	int ed = 0;

	if(position == e_LEFT || position == e_TOP)
	{
		ed = dataSize - 1;
		st = ed - searchLen;		
	}
	else if(position == e_RIGHT || position == e_BOTTOM)
	{
		st = 0;
		ed = searchLen;
	}

	for(int i = st; i < ed; i++)
	{
		sum += data[i];
	}
	avr = sum / searchLen;

	double stdHeight = filterH * (ALLOW_HEIGHT / 100.0);

	if(avr < stdHeight)
		ret = FALSE;

	return ret;
}

CPoint CProc3D::MakePoint(int calcData, CPoint startCoordi, int position, int pfMargin)
{
	CPoint ret;
	ret.SetPoint(0,0);

	if(position == e_LEFT ||  position == e_RIGHT || position == e_TOP || position == e_BOTTOM)
	{
		switch(position)
		{
		case e_LEFT:
			{
				ret.x = (LONG)calcData + pfMargin;
				ret.y = (LONG)startCoordi.y;
			}
			break;
		case e_RIGHT:
			{
				ret.x = (LONG)(startCoordi.x + calcData);
				ret.y = (LONG)startCoordi.y;
			}
			break;
		case e_TOP:
			{
				ret.x = (LONG)startCoordi.x;
				ret.y = (LONG)calcData + pfMargin;
			}
			break;
		case e_BOTTOM:
			{
				ret.x = (LONG)startCoordi.x;
				ret.y = (LONG)(startCoordi.y + calcData);
			}
			break;
		}
	}
	

	return ret;
}


float CProc3D::GetStdDeltaH(float* roiZmap, int sizeX, int sizeY, double range, float* retFilterHighH, float* retFilterLowH)
{
	if(roiZmap == NULL || sizeX < 0 || sizeY < 0)
		return -1;

	float retH = 0;	
	double sum = 0;
	float stdAvr1 = 0;
	double area = sizeX * sizeY;

	int index = 0;
	float data = 0;
	float hMax = roiZmap[0]; 
	float hMin = roiZmap[0]; 
	for(int x = 0; x < sizeX; x++)
	{
		for(int y = 0; y < sizeY; y++)
		{
			index = (sizeX * y) + x;
			data = roiZmap[index];

			sum += (double)data;

			if(hMax < data)
				hMax = data;

			if(hMin > data)
				hMin = data;
		}
	}
	stdAvr1 = (float)(sum / area);


	index = 0;
	int highCnt = 0;		
	double highSum = 0;

	int lowCnt = 0;
	double lowSum = 0;

	float stdAvr2 = 0;
	float stdAvr3 = 0;
	for(int x = 0; x < sizeX; x++)
	{
		for(int y = 0; y < sizeY; y++)
		{
			index = (sizeX * y) + x;
			data = roiZmap[index];

			if(data >= stdAvr1)
			{
				highSum += (double)data;
				highCnt++;
			}
			else
			{
				lowSum += (double)data;
				lowCnt++;
			}
		}
	}


	float margin = 0;

	if(range < 0 || range > 100)
		range = HEIGHT_MARGIN;


	range = range / 100.0;
	if(retFilterHighH != NULL)
	{
		stdAvr2 = (float)(highSum / highCnt);
		margin = (float)(stdAvr2 * range);   //20%
		*retFilterHighH = stdAvr2 - margin;
	}


	if(retFilterLowH != NULL)
	{
		stdAvr3 = (float)(lowSum / lowCnt);
		*retFilterLowH = stdAvr3;
	}

	//사용하지 않아 주석처리함....  (ExtractBody2() 에서 사용함..)
/*	double testAvr = 0.;
	double testStdev = 0.;
	GetStdev(roiZmap, 0 , sizeX * sizeY, &testAvr, &testStdev);

	double per = (double)((stdAvr2 / (hMax + hMin)) * 100.0);



	float totalAvr = (float)((stdAvr1 + stdAvr2) / 2.0);
	retH = totalAvr;*/




		

	return retH;
}







//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



float CProc3D::ExtractBody(float* pZmapRoiData, float* bodyDst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY, double angle, double* retCogX, double* retCogY, CRect* retRect, UCHAR* img)
{
	if(pZmapRoiData == NULL  || orgSizeX < 0 || orgSizeY < 0 || cx < 0 || cy < 0 || roiSizeX < 0 || roiSizeY < 0)
		return -1;

	int edgeCnts = EDGE_CNTS;
	int stMargin = ST_MARGIN;
	int dataCnts_row = 0;
	int dataCnts_col = 0;

	if(roiSizeX < (edgeCnts + 1))
	{
		dataCnts_col = roiSizeX - 1;
	}
	else
	{
		dataCnts_col = edgeCnts;
	}

	if(roiSizeY < (edgeCnts + 1))
	{
		dataCnts_row = roiSizeY - 1;
	}	
	else
	{
		dataCnts_row = edgeCnts;	
	}

	if(dataCnts_col <= 0 || dataCnts_row <= 0)
		return -1;


	/*float* pZmapRoiData = NULL;
	pZmapRoiData = new float[roiSizeX * roiSizeY];
	memset(pZmapRoiData, 0, (sizeof(float) * roiSizeX * roiSizeY));
	GetClipZmapData(zmapData, pZmapRoiData, orgSizeX, orgSizeY, cx, cy, roiSizeX, roiSizeY);*/
	

	int tempSize = STD_AREA_SIZE;
	int tempCx = roiSizeX / 2;
	int tempCy = roiSizeY / 2;
	float height = GetFilterHeight(pZmapRoiData, roiSizeX, roiSizeY, tempCx, tempCy, tempSize, tempSize);  // 중심에서 10*10의 평균 높이 
	//float height = ExtractBody(zmapData, NULL, NULL, orgSizeX, orgSizeY, cx, cy, roiSizeX, roiSizeY, 100);	
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	/*CPoint* stRowLeft = new CPoint[dataCnts_row];
	CPoint* stRowRight = new CPoint[dataCnts_row];
	CPoint* stColTop = new CPoint[dataCnts_col];
	CPoint* stColBottom = new CPoint[dataCnts_col];*/
	CPoint* stRowLeft = g_pMManager->pem_new<CPoint>(true, dataCnts_row, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stRowRight = g_pMManager->pem_new<CPoint>(true, dataCnts_row, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColTop = g_pMManager->pem_new<CPoint>(true, dataCnts_col, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColBottom = g_pMManager->pem_new<CPoint>(true, dataCnts_col, (PCHAR)__FUNCTION__, __LINE__);

	
	CalcStartPoint(roiSizeX, roiSizeY, dataCnts_row, dataCnts_col, stMargin, stRowLeft, stRowRight, stColTop, stColBottom);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

	float** profileData_left = NULL;
	int sz_left = (int)stRowLeft[0].x;
	alpa_make_array_2d(&profileData_left, dataCnts_row, sz_left);

	float** profileData_right = NULL;
	int sz_right = roiSizeX - (int)stRowRight[0].x;
	alpa_make_array_2d(&profileData_right, dataCnts_row, sz_right);

	float** profileData_top = NULL;
	int sz_top = (int)stColTop[0].y;
	alpa_make_array_2d(&profileData_top, dataCnts_col, sz_top);

	float** profileData_bottom = NULL;
	int sz_bottom = roiSizeY - (int)stColBottom[0].y;
	alpa_make_array_2d(&profileData_bottom, dataCnts_col, sz_bottom);
	
	//extract data (left, right, top, bottom)
	CString fileName = _T("");	
	for(int i = 0; i < dataCnts_row; i++)
	{
		MakeProfileData_H(pZmapRoiData, profileData_left[i], roiSizeX, roiSizeY, 0, (int)stRowLeft[i].y, sz_left);
		MakeProfileData_H(pZmapRoiData, profileData_right[i], roiSizeX, roiSizeY, (int)stRowRight[i].x, (int)stRowRight[i].y, sz_right);

// 		#ifdef _DEBUG
// 		fileName.Format(_T("D:\\TestImage\\data\\left_%d.csv"),i);
// 		DataSave(profileData_left[i], sz_left, fileName);
// 		fileName.Format(_T("D:\\TestImage\\data\\right_%d.csv"),i);
// 		DataSave(profileData_right[i], sz_right, fileName);		
// 		#endif
	}

	for(int i = 0; i < dataCnts_col; i++)
	{
		MakeProfileData_V(pZmapRoiData, profileData_top[i], roiSizeX, roiSizeY, (int)stColTop[i].x, 0, sz_top);
		MakeProfileData_V(pZmapRoiData, profileData_bottom[i], roiSizeX, roiSizeY, (int)stColBottom[i].x, (int)stColBottom[i].y, sz_bottom);

// 		#ifdef _DEBUG
// 		fileName.Format(_T("D:\\TestImage\\data\\top_%d.csv"),i);
// 		DataSave(profileData_top[i], sz_top, fileName);
// 		fileName.Format(_T("D:\\TestImage\\data\\bottom_%d.csv"),i);
// 		DataSave(profileData_bottom[i], sz_bottom, fileName);
// 		#endif
	}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	/*int* deltaMax_left = new int[dataCnts_row];
	int* deltaMax_right = new int[dataCnts_row];
	int* deltaMax_top = new int[dataCnts_col];
	int* deltaMax_bottom = new int[dataCnts_col];*/
	int* deltaMax_left = g_pMManager->pem_new<int>(true, dataCnts_row, (PCHAR)__FUNCTION__, __LINE__);
	int* deltaMax_right = g_pMManager->pem_new<int>(true, dataCnts_row, (PCHAR)__FUNCTION__, __LINE__);
	int* deltaMax_top = g_pMManager->pem_new<int>(true, dataCnts_col, (PCHAR)__FUNCTION__, __LINE__);
	int* deltaMax_bottom = g_pMManager->pem_new<int>(true, dataCnts_col, (PCHAR)__FUNCTION__, __LINE__);

		
	int ctSerachPixel = SEARCH_PIXEL;
	double stdHeight = height * (ALLOW_HEIGHT / 100.0);

	for(int i = 0; i < dataCnts_row; i++)
	{
		SearchDataEdge(profileData_left[i], sz_left, stdHeight, ctSerachPixel, e_LEFT, &deltaMax_left[i]);
		SearchDataEdge(profileData_right[i], sz_right, stdHeight, ctSerachPixel, e_RIGHT, &deltaMax_right[i]);
	}

	for(int i = 0; i < dataCnts_col; i++)
	{
		SearchDataEdge(profileData_top[i], sz_top, stdHeight, ctSerachPixel, e_TOP, &deltaMax_top[i]);
		SearchDataEdge(profileData_bottom[i], sz_bottom, stdHeight, ctSerachPixel, e_BOTTOM, &deltaMax_bottom[i]);
	}

// #ifdef _DEBUG
// 	DataSave(deltaMax_left, dataCnts_row, _T("D:\\TestImage\\data\\_left.csv"));
// 	DataSave(deltaMax_right, dataCnts_row, _T("D:\\TestImage\\data\\_right.csv"));
// 	DataSave(deltaMax_top, dataCnts_col, _T("D:\\TestImage\\data\\_top.csv"));
// 	DataSave(deltaMax_bottom, dataCnts_col, _T("D:\\TestImage\\data\\_bottom.csv"));
// #endif
	
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	float coef_left[2];
	float coef_right[2];
	float coef_top[2];
	float coef_bottom[2];

	//직선 방정식 
//hough line
	double angle_left = CalcLineEquation(deltaMax_left, dataCnts_row, e_LEFT, roiSizeX, roiSizeY, stRowLeft, coef_left);
	double angle_right = CalcLineEquation(deltaMax_right, dataCnts_row, e_RIGHT, roiSizeX, roiSizeY, stRowRight, coef_right);
	double angle_top = CalcLineEquation(deltaMax_top, dataCnts_col, e_TOP, roiSizeX, roiSizeY, stColTop, coef_top);
	double angle_bottom = CalcLineEquation(deltaMax_bottom, dataCnts_col, e_BOTTOM, roiSizeX, roiSizeY, stColBottom, coef_bottom);

//least square
// 	 totalAvr_left = CalcLineEquation(deltaMax_left, dataCnts_row, e_LEFT, stRowLeft, coef_left);
// 	 totalAvr_right = CalcLineEquation(deltaMax_right, dataCnts_row, e_RIGHT, stRowRight, coef_right);
// 	 totalAvr_top = CalcLineEquation(deltaMax_top, dataCnts_col, e_TOP, stColTop, coef_top);
// 	 totalAvr_bottom = CalcLineEquation(deltaMax_bottom, dataCnts_col, e_BOTTOM, stColBottom, coef_bottom);

	
	//교점
	POINT pt[4];
	CalcNodalPoint(coef_left, angle_left, coef_top, angle_top, &pt[0].x, &pt[0].y);			//left_top
	CalcNodalPoint(coef_left, angle_left, coef_bottom, angle_bottom, &pt[1].x, &pt[1].y);	//left_bottom
	CalcNodalPoint(coef_right, angle_right, coef_bottom, angle_bottom, &pt[2].x, &pt[2].y); //right_bottom
	CalcNodalPoint(coef_right, angle_right, coef_top, angle_top, &pt[3].x, &pt[3].y);		//right_top

	
	CRect bodyRect;
	bodyRect.left = (long)((pt[0].x + pt[1].x) / 2);
	bodyRect.right = (long)((pt[2].x + pt[3].x) / 2);
	bodyRect.top = (long)((pt[0].y + pt[3].y) / 2);
	bodyRect.bottom = (long)((pt[1].y + pt[2].y) / 2);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	
	BYTE* lutData = NULL;
	//lutData = new BYTE[roiSizeX * roiSizeY];
	lutData = g_pMManager->pem_new<BYTE>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(lutData, 0, (sizeof(BYTE) * roiSizeX * roiSizeY));

	MakeLUT_Poly(lutData, roiSizeX, roiSizeY, pt, 4);

	float dataMax = 0;
	float dataMin = 0;
	GetHlimit(pZmapRoiData, roiSizeX, roiSizeY, &dataMax, &dataMin);
	float avrH = MaskingData(pZmapRoiData, lutData, bodyDst, roiSizeX, roiSizeY, dataMin);

	//delete lutData;
	g_pMManager->pem_delete(lutData, false);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	double bodyCtX = (bodyRect.Width() / 2.0) + bodyRect.left;
	double bodyCtY = (bodyRect.Height() / 2.0) + bodyRect.top;
	 

	int widthTemp = 0;
	int lengthTemp = 0;
 	double bodyCtX_temp = 0;
 	double bodyCtY_temp = 0;

	if(angle ==  90 || angle == 270)
	{
		widthTemp = roiSizeY;
		lengthTemp = roiSizeX;

 		bodyCtX_temp = bodyCtY;
 		bodyCtY_temp = bodyCtX;
	}
	else
	{
		widthTemp = roiSizeX;
		lengthTemp = roiSizeY;

 		bodyCtX_temp = bodyCtX;
 		bodyCtY_temp = bodyCtY;
	}
	double cogX = (cx - (widthTemp / 2.0)) + bodyCtX_temp;
	double cogY = (cy - (lengthTemp / 2.0)) + bodyCtY_temp;


	*retCogX = cogX;
	*retCogY = cogY;
	*retRect = bodyRect;

	////////////////////////////////////////////////////////////////////////// make profile image  (debug)
	if(img != NULL)
	{
		int index_top = 0;
		int index_bottom = 0;
		UCHAR* gray_left[EDGE_CNTS];
		UCHAR* gray_right[EDGE_CNTS];
		UCHAR* gray_top[EDGE_CNTS];
		UCHAR* gray_bottom[EDGE_CNTS];

		int ctX = roiSizeX / 2;
		int ctY = roiSizeY / 2;	
		int distX = (roiSizeX ) / (dataCnts_col + 1);
		int distY = (roiSizeY ) / (dataCnts_row + 1);

		for(int i = 0; i < dataCnts_row; i++)
		{
			//gray_left[i] = new UCHAR[sz_left];
			gray_left[i] = g_pMManager->pem_new<UCHAR>(true, sz_left, (PCHAR)__FUNCTION__, __LINE__);
			NormalizeData_Gray(profileData_left[i], gray_left[i], sz_left, 1);

			//gray_right[i] = new UCHAR[sz_right];
			gray_right[i] = g_pMManager->pem_new<UCHAR>(true, sz_right, (PCHAR)__FUNCTION__, __LINE__);
			NormalizeData_Gray(profileData_right[i], gray_right[i], sz_right, 1);
		}

		for(int i = 0; i < dataCnts_col; i++)
		{
			//gray_top[i] = new UCHAR[sz_top];
			gray_top[i] = g_pMManager->pem_new<UCHAR>(true, sz_top, (PCHAR)__FUNCTION__, __LINE__);
			NormalizeData_Gray(profileData_top[i], gray_top[i], 1 , sz_top);

			//gray_bottom[i] = new UCHAR[sz_bottom];
			gray_bottom[i] = g_pMManager->pem_new<UCHAR>(true, sz_bottom, (PCHAR)__FUNCTION__, __LINE__);
			NormalizeData_Gray(profileData_bottom[i], gray_bottom[i], 1 , sz_bottom);
		}





		UCHAR* profileImg = NULL;
		//profileImg = new UCHAR[roiSizeX * roiSizeY];
		profileImg = g_pMManager->pem_new<UCHAR>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
		//memset(profileImg, 0, sizeof(UCHAR) * roiSizeX * roiSizeY);

		UCHAR* pDst = NULL;
		UCHAR* pSrc = NULL;
		for(int i = 0; i < dataCnts_row; i++)
		{		
			pDst = profileImg;
			pDst += (roiSizeX * stRowLeft[i].y);
			memcpy_s(pDst, (sizeof(UCHAR) * sz_left), gray_left[i], (sizeof(UCHAR) * sz_left));	

		}


		for(int i = 0; i < dataCnts_row; i++)
		{				
			pDst = profileImg;
			pDst += ((roiSizeX * stRowRight[i].y) + stRowRight[i].x);
			memcpy_s(pDst, (sizeof(UCHAR) * sz_right), gray_right[i], (sizeof(UCHAR) * sz_right));		
		}

		index_top = 0;
		index_bottom = 0;

		for(int y = 0; y < roiSizeY; y++)
		{
			if(y < (int)stColTop[0].y)
			{	
				pDst = profileImg;
				pDst += (roiSizeX * y) + stColTop[0].x;
				for(int i = 0; i < dataCnts_col; i++)
				{
					pSrc = gray_top[i];
					pSrc += index_top;				

					*pDst = *pSrc;
					pDst += distX;
				}
				index_top++;
			}

			if(y >= (int)stColBottom[0].y)
			{	
				pDst = profileImg;
				pDst += (roiSizeX * y) + stColBottom[0].x;
				for(int i = 0; i < dataCnts_col; i++)
				{
					pSrc = gray_bottom[i];
					pSrc += index_bottom;
				
					*pDst = *pSrc;	
					pDst += distX;
				}
				index_bottom++;
			}

		}

		memcpy_s(img, (sizeof(UCHAR) * roiSizeX * roiSizeY), profileImg, (sizeof(UCHAR) * roiSizeX * roiSizeY));



		for(int i = 0; i < dataCnts_row; i++)
		{
			//delete gray_left[i];
			g_pMManager->pem_delete(gray_left[i], false);
		}
		for(int i = 0; i < dataCnts_row; i++)
		{
			//delete gray_right[i];
			g_pMManager->pem_delete(gray_right[i], false);
		}
		for(int i = 0; i < dataCnts_col; i++)
		{
			//delete gray_top[i];
			g_pMManager->pem_delete(gray_top[i], false);
		}
		for(int i = 0; i < dataCnts_col; i++)
		{
			//delete gray_bottom[i];
			g_pMManager->pem_delete(gray_bottom[i], false);
		}

		//delete profileImg;
		g_pMManager->pem_delete(profileImg, false);

	}

	
	//////////////////////////////////////////////////////////////////////////


	/*delete deltaMax_left;
	delete deltaMax_right; 
	delete deltaMax_top;
	delete deltaMax_bottom;*/
	g_pMManager->pem_delete(deltaMax_left, false);
	g_pMManager->pem_delete(deltaMax_right, false);
	g_pMManager->pem_delete(deltaMax_top, false);
	g_pMManager->pem_delete(deltaMax_bottom, false);


	/*delete stRowLeft;
	delete stRowRight;
	delete stColTop;
	delete stColBottom;*/
	g_pMManager->pem_delete(stRowLeft, false);
	g_pMManager->pem_delete(stRowRight, false);
	g_pMManager->pem_delete(stColTop, false);
	g_pMManager->pem_delete(stColBottom, false);
//	delete pZmapRoiData;

	alpa_delete_array_2d(&profileData_left, dataCnts_row);
	alpa_delete_array_2d(&profileData_right, dataCnts_row);
	alpa_delete_array_2d(&profileData_top, dataCnts_col);
	alpa_delete_array_2d(&profileData_bottom, dataCnts_col);


	return avrH;
}

void CProc3D::CalcNodalPoint(float* pVal1, double angle1, float* pVal2, double angle2, long* retX, long* retY)
{
	float x = 0;
	float y = 0;
	
	x = (pVal1[0] - pVal2[0]) / (pVal2[1] - pVal1[1]);

	//float y2 = 0;
	//y2 = ((pVal2[1] * pVal1[0]) - (pVal1[1] * pVal2[0])) / (pVal2[1] - pVal1[1]); // y값 계산..

	if(angle1 == 90.0)
	{
		y = (pVal2[1] * x) + pVal2[0];
	}
	else if(angle2 == 90.0)
	{
		y = (pVal1[1] * x) + pVal1[0];
	}
	else
	{
		y = (pVal2[1] * x) + pVal2[0];
	}


	*retX = (long)(x + 0.5);
	*retY = (long)(y + 0.5);
}


int CProc3D::CalcThreshold(float* zmapData, int bufSize)
{
	int ret = 0;

	int thresh = 0;
	float profileMax = 0;
	float profileMin = 0;

	GetHlimit(zmapData, bufSize, 1, &profileMax, &profileMin);

	//float* normalizeData = new float[bufSize];
	float* normalizeData = g_pMManager->pem_new<float>(true, bufSize, (PCHAR)__FUNCTION__, __LINE__);
	//normalize roi zmap data
	NormalizeData_Zmap(zmapData, normalizeData, bufSize, 1, profileMin);

	//histogram

	m_histoSize = (int)((profileMax - profileMin) / HISTO_INTERVAL) + 1;
	if(m_histoSize < 1)
	{
		//delete normalizeData;
		g_pMManager->pem_delete(normalizeData, false);
		return 0;
	}

	DeleteHistoVal();
	//m_pZmapHistoData = new int[m_histoSize];
	m_pZmapHistoData = g_pMManager->pem_new<int>(true, m_histoSize, (PCHAR)__FUNCTION__, __LINE__);
	memset(m_pZmapHistoData, 0, sizeof(int) * m_histoSize);

	MakeHistogram(normalizeData, m_pZmapHistoData, bufSize, 1, profileMax, profileMin, m_histoSize);

// #ifdef _DEBUG
// 		HistogramSave(m_pZmapHistoData, m_histoSize);
// #endif 

	thresh = GetThreshold(m_pZmapHistoData, bufSize, 1, m_histoSize);
	ret =  thresh - (int)profileMin;

	double aavr = 0.0;
	double std = 0.0;
	GetStdev(m_pZmapHistoData, thresh, m_histoSize, &aavr, &std);
	

	//delete normalizeData;
	g_pMManager->pem_delete(normalizeData, false);


	return ret;
}

int CProc3D::CalcThreshold(float* zmapData, int sizeX, int sizeY)
{
	int ret = 0;

	int thresh = 0;
	float profileMax = 0;
	float profileMin = 0;

	GetHlimit(zmapData, sizeX, sizeY, &profileMax, &profileMin);

	//float* normalizeData = new float[sizeX * sizeY];
	float* normalizeData = g_pMManager->pem_new<float>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//normalize roi zmap data
	NormalizeData_Zmap(zmapData, normalizeData, sizeX, sizeY, profileMin);

	//histogram

	m_histoSize = (int)((profileMax - profileMin) / HISTO_INTERVAL) + 1;
	if(m_histoSize < 1)
	{
		//delete normalizeData;
		g_pMManager->pem_delete(normalizeData, false);
		return 0;
	}

	DeleteHistoVal();
	//m_pZmapHistoData = new int[m_histoSize];
	m_pZmapHistoData = g_pMManager->pem_new<int>(true, m_histoSize, (PCHAR)__FUNCTION__, __LINE__);
	memset(m_pZmapHistoData, 0, sizeof(int) * m_histoSize);

	MakeHistogram(normalizeData, m_pZmapHistoData, sizeX, sizeY, profileMax, profileMin, m_histoSize);

// #ifdef _DEBUG
// 	HistogramSave(m_pZmapHistoData, m_histoSize);
// #endif 

	thresh = GetThreshold(m_pZmapHistoData, sizeX, sizeY, m_histoSize);
	ret =  thresh - (int)profileMin;


	//delete normalizeData;
	g_pMManager->pem_delete(normalizeData, false);


	return ret;
}

void CProc3D::SearchDataEdge_rev(float* pfData, int dataSize, double stdHeight, int stdPixel, int* retDeltaMax)
{
	int deltaMax = 0;
	float tempSum = 0;
	double tempAvr = 0;
	float sub = 0;
	int ctSerachPixel = stdPixel;
	
	int thresh = CalcThreshold(pfData, dataSize);
	float deltaTemp = pfData[dataSize - 1] - pfData[dataSize - 2];

	for(int i = dataSize - 1; i > 0; i--)
	{
		tempSum += pfData[i];
		if(i == dataSize - (ctSerachPixel + 1))
		{
			tempAvr = tempSum / 10.0;
			if(tempAvr < stdHeight)
			{
				deltaMax = -1;
				break;
			}
		}

		if(pfData[i] > thresh)
		{
			sub = pfData[i] - pfData[i - 1];
			if(deltaTemp < sub)
			{
				deltaTemp = sub;
				deltaMax = i;
			}
		}
	}

	*retDeltaMax = deltaMax;
}

void CProc3D::SearchDataEdge(float* pfData, int dataSize, double stdHeight, int stdPixel, int mode, int* retDeltaMax)
{
	int noInspArea = 5;
	int deltaMax = 0;
	float tempSum = 0;
	double tempAvr = 0;
	float sub = 0;
	int ctSerachPixel = stdPixel;

	float deltaTemp = 0;
	int thresh = CalcThreshold(pfData, dataSize);

	int nextIndex = 0;
	int index = 0;
	int searchCnts = 0;
	int st = 0;
	int ed = 0;
	if(mode == e_LEFT || mode == e_TOP)
	{
		st = dataSize - 1;
		ed = noInspArea;
		deltaTemp = pfData[st] - pfData[st - 1];
		for(int i = st; i > ed; i--)
		{
			index = i;
			nextIndex = index - 1;
			if(pfData[index] <= NODATA)
			{
				index = nextIndex;
				nextIndex = index - 1;
			}
			if(pfData[nextIndex] <= NODATA)
			{ 
				if(i == (ed + 1))
					break;

				nextIndex = i - 2;
			}

			tempSum += pfData[index];
			searchCnts++;
			if(searchCnts == ctSerachPixel)
			{
				tempAvr = tempSum / searchCnts;
				if(tempAvr < stdHeight)
				{
					deltaMax = -1;
					break;
				}
			}

// 			if(pfData[i] > thresh)
// 			{
				sub = pfData[index] - pfData[nextIndex];
				if(deltaTemp < sub)
				{
					deltaTemp = sub;
					deltaMax = index;
				}
			//}
		}

	}
	else if(mode == e_RIGHT || mode == e_BOTTOM)
	{
		searchCnts = 0;
		st = 0;
		ed = (dataSize - 1) - noInspArea;
		deltaTemp = pfData[0] - pfData[1];
		for(int i = st; i < ed; i++)
		{
			index = i;
			nextIndex = index + 1;
			if(pfData[index] <= NODATA)
			{
				index = nextIndex;
				nextIndex = index + 1;
			}
			if(pfData[nextIndex] <= NODATA)
			{ 
				if(i == ed - 1)
					break;

				nextIndex = i + 2;
			}

			tempSum += pfData[index];
			searchCnts++;
			if(searchCnts == ctSerachPixel)
			{
				tempAvr = tempSum / searchCnts;
				if(tempAvr < stdHeight)
				{
					deltaMax = -1;
					break;
				}
			}

// 			if(pfData[i] > thresh)
// 			{
				sub = pfData[index] - pfData[nextIndex];
				if(deltaTemp < sub)
				{
					deltaTemp = sub;
					deltaMax = index;
				}
			//}
		}
	}
	

	*retDeltaMax = deltaMax;
}

void CProc3D::MakeProfileData_H(float* zmapData, float* dst, int orgSizeX, int orgSizeY, int stpX, int stpY, int size) // ㅡ
{
	float* pSrcTemp = zmapData;
	float* pDstTemp = dst;
	int startIndex = (orgSizeX * stpY) + stpX;
	int cpyLength = size;

	if((startIndex + cpyLength) >= (orgSizeX * orgSizeY))
	{
		cpyLength = ((orgSizeX * orgSizeY) - startIndex); - 1;

		if(cpyLength < 0)
			cpyLength = 0;
	}

	pSrcTemp += startIndex;
	memcpy_s(pDstTemp, (sizeof(float) * cpyLength), pSrcTemp, (sizeof(float) * cpyLength));	
}

//#include <omp.h>
void CProc3D::MakeProfileData_V(float* zmapData, float* dst, int orgSizeX, int orgSizeY, int stpX, int stpY, int size) // ㅣ
{
	if(orgSizeX <= 0 || orgSizeY <= 0 || stpX < 0 || stpY < 0 || size <= 0)
		return;

	float* pSrcTemp = zmapData;
	float* pDstTemp = dst;
	int startIndex = (orgSizeX * stpY) + stpX;
	int cpyLength = size;

	if((startIndex + cpyLength) >= (orgSizeX * orgSizeY))
	{
		cpyLength = ((orgSizeX * orgSizeY) - startIndex) - 1;

		if(cpyLength < 0)
			cpyLength = 0;
	}

	pSrcTemp += startIndex;

	//#pragma omp for shared(pDstTemp, pSrcTemp) private(orgSizeX)
	for(int i = 0; i < cpyLength; i++)
	{
		memcpy_s(pDstTemp, (sizeof(float) * 1), pSrcTemp, (sizeof(float) * 1));	
		pSrcTemp += orgSizeX;
		pDstTemp += 1;
	}
}

void CProc3D::MakeProfileData_H(UCHAR* imgData, UCHAR* dst, int orgSizeX, int orgSizeY, int stpX, int stpY, int size) // ㅡ
{
	UCHAR* pSrcTemp = imgData;
	UCHAR* pDstTemp = dst;
	int startIndex = (orgSizeX * stpY) + stpX;
	int cpyLength = size;

	if((startIndex + cpyLength) >= (orgSizeX * orgSizeY))
	{
		cpyLength = ((orgSizeX * orgSizeY) - startIndex); - 1;

		if(cpyLength < 0)
			cpyLength = 0;
	}

	pSrcTemp += startIndex;
	memcpy_s(pDstTemp, (sizeof(UCHAR) * cpyLength), pSrcTemp, (sizeof(UCHAR) * cpyLength));	
}

void CProc3D::MakeProfileData_V(UCHAR* imgData, UCHAR* dst, int orgSizeX, int orgSizeY, int stpX, int stpY, int size) // ㅣ
{
	if(orgSizeX <= 0 || orgSizeY <= 0 || stpX < 0 || stpY < 0 || size <= 0)
		return;

	UCHAR* pSrcTemp = imgData;
	UCHAR* pDstTemp = dst;
	int startIndex = (orgSizeX * stpY) + stpX;
	int cpyLength = size;

	if((startIndex + cpyLength) >= (orgSizeX * orgSizeY))
	{
		cpyLength = ((orgSizeX * orgSizeY) - startIndex) - 1;

		if(cpyLength < 0)
			cpyLength = 0;
	}

	pSrcTemp += startIndex;

	//#pragma omp for shared(pDstTemp, pSrcTemp) private(orgSizeX)
	for(int i = 0; i < cpyLength; i++)
	{
		memcpy_s(pDstTemp, (sizeof(UCHAR) * 1), pSrcTemp, (sizeof(UCHAR) * 1));	
		pSrcTemp += orgSizeX;
		pDstTemp += 1;
	}
}


void CProc3D::CalcStartPoint2(int cx, int cy, CRect rt, int margin, int rowCnts, int colCnts, CPoint* retLeft, CPoint* retRight, CPoint* retTop, CPoint* retBottom)
{
	if(rowCnts <= 0 || colCnts <= 0)
		return ;

	int distX = (int)(rt.Width() / (colCnts + 1.0) + 0.5);
	int distY = (int)(rt.Height() / (rowCnts + 1.0) + 0.5);
	int tempX1 = rt.left;
	int tempY1 = rt.top;
	int tempX2 = rt.left;
	int tempY2 = rt.top;

	/*CPoint* stRowLeft = new CPoint[rowCnts];
	CPoint* stRowRight = new CPoint[rowCnts];
	CPoint* stColTop = new CPoint[colCnts];
	CPoint* stColBottom = new CPoint[colCnts];*/
	CPoint* stRowLeft = g_pMManager->pem_new<CPoint>(true, rowCnts, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stRowRight = g_pMManager->pem_new<CPoint>(true, rowCnts, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColTop = g_pMManager->pem_new<CPoint>(true, colCnts, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColBottom = g_pMManager->pem_new<CPoint>(true, colCnts, (PCHAR)__FUNCTION__, __LINE__);


	for(int i = 0; i < rowCnts; i++)
	{
		tempX1 = cx - margin;
		tempY1 += distY;	
		stRowLeft[i].SetPoint(tempX1, tempY1);

		tempX1 = cx + margin;
		stRowRight[i].SetPoint(tempX1, tempY1);
	}

	for(int i = 0; i < colCnts; i++)
	{
		tempX2 += distX;
		tempY2 = cy - margin;	
		stColTop[i].SetPoint(tempX2, tempY2);

		tempY2 = cy + margin;
		stColBottom[i].SetPoint(tempX2, tempY2);
	}



	memcpy_s(retLeft, (sizeof(CPoint) * rowCnts), stRowLeft, (sizeof(CPoint) * rowCnts));
	memcpy_s(retRight, (sizeof(CPoint) * rowCnts), stRowRight, (sizeof(CPoint) * rowCnts));
	memcpy_s(retTop, (sizeof(CPoint) * colCnts), stColTop, (sizeof(CPoint) * colCnts));
	memcpy_s(retBottom, (sizeof(CPoint) * colCnts), stColBottom, (sizeof(CPoint) * colCnts));


	/*delete stRowLeft;
	delete stRowRight;
	delete stColTop;
	delete stColBottom;*/
	g_pMManager->pem_delete(stRowLeft, false);
	g_pMManager->pem_delete(stRowRight, false);
	g_pMManager->pem_delete(stColTop, false);
	g_pMManager->pem_delete(stColBottom, false);
}

void CProc3D::CalcStartPoint(int sizeX, int sizeY, int rowCnts, int colCnts, int margin, CPoint* retLeft, CPoint* retRight, CPoint* retTop, CPoint* retBottom)
{	
	if(rowCnts <= 0 || colCnts <= 0 || sizeX <= 0 || sizeY <= 0)
		return ;

	int ctX = sizeX / 2;
	int ctY = sizeY / 2;	
	int distX = sizeX / (colCnts + 1);
	int modX = (sizeX ) % (colCnts + 1);
	int distY = sizeY / (rowCnts + 1);
	int modY = (sizeY ) % (rowCnts + 1);

	int stX = modX / 2;//(sizeX - (distX * colCnts)) / 2;
	int stY = modY / 2;//(sizeY - (distY * rowCnts)) / 2;
	int tempX1 = (stX < 0)? (stX * -1) : stX;
	int tempY1 = (stY < 0)? (stY * -1) : stY;
	int tempX2 = (stX < 0)? (stX * -1) : stX;
	int tempY2 = (stY < 0)? (stY * -1) : stY;

	/*CPoint* stRowLeft = new CPoint[rowCnts];
	CPoint* stRowRight = new CPoint[rowCnts];
	CPoint* stColTop = new CPoint[colCnts];
	CPoint* stColBottom = new CPoint[colCnts];*/
	CPoint* stRowLeft = g_pMManager->pem_new<CPoint>(true, rowCnts, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stRowRight = g_pMManager->pem_new<CPoint>(true, rowCnts, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColTop = g_pMManager->pem_new<CPoint>(true, colCnts, (PCHAR)__FUNCTION__, __LINE__);
	CPoint* stColBottom = g_pMManager->pem_new<CPoint>(true, colCnts, (PCHAR)__FUNCTION__, __LINE__);

	for(int i = 0; i < rowCnts; i++)
	{
		tempX1 = ctX - margin;
		tempY1 += distY;	
		stRowLeft[i].SetPoint(tempX1, tempY1);

		tempX1 = ctX + margin;
		stRowRight[i].SetPoint(tempX1, tempY1);
	}

	for(int i = 0; i < colCnts; i++)
	{
		tempX2 += distX;
		tempY2 = ctY - margin;	
		stColTop[i].SetPoint(tempX2, tempY2);

		tempY2 = ctY + margin;
		stColBottom[i].SetPoint(tempX2, tempY2);
	}


	memcpy_s(retLeft, (sizeof(CPoint) * rowCnts), stRowLeft, (sizeof(CPoint) * rowCnts));
	memcpy_s(retRight, (sizeof(CPoint) * rowCnts), stRowRight, (sizeof(CPoint) * rowCnts));
	memcpy_s(retTop, (sizeof(CPoint) * colCnts), stColTop, (sizeof(CPoint) * colCnts));
	memcpy_s(retBottom, (sizeof(CPoint) * colCnts), stColBottom, (sizeof(CPoint) * colCnts));


	/*delete stRowLeft;
	delete stRowRight;
	delete stColTop;
	delete stColBottom;*/
	g_pMManager->pem_delete(stRowLeft, false);
	g_pMManager->pem_delete(stRowRight, false);
	g_pMManager->pem_delete(stColTop, false);
	g_pMManager->pem_delete(stColBottom, false);
}

float CProc3D::GetFilterHeight(float* src, int orgSizeX, int orgSizeY, int cx, int cy, int sizeX, int sizeY)
{
	if(sizeX < 0 || sizeY < 0)
		return -10000;

	float* pZmapTemp = NULL;
	//pZmapTemp = new float[sizeX * sizeY];
	pZmapTemp = g_pMManager->pem_new<float>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(pZmapTemp, 0, (sizeof(float) * sizeX * sizeY));

	//clip data
	GetClipZmapData(src, pZmapTemp, orgSizeX, orgSizeY, cx, cy, sizeX, sizeY);

	float sum = 0;
	float height = 0;
	int area = sizeX * sizeY;
	for(int i = 0; i < area; i++)
	{
		sum += pZmapTemp[i];
	}

	height = sum / area;

	//delete pZmapTemp;
	g_pMManager->pem_delete(pZmapTemp, false);

	return height;
}

//shk 2014/04/10
float CProc3D::GetFilterHeight2(float* src, int orgSizeX, int orgSizeY, int cx, int cy, int sizeX, int sizeY)
{
	if(sizeX < 0 || sizeY < 0)
		return -10000;

	float* pZmapTemp = NULL;
	//pZmapTemp = new float[sizeX * sizeY];
	pZmapTemp = g_pMManager->pem_new<float>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(pZmapTemp, 0, (sizeof(float) * sizeX * sizeY));

	//clip data
	GetClipZmapData(src, pZmapTemp, orgSizeX, orgSizeY, cx, cy, sizeX, sizeY);

	int nthresLow = 10;

	//SHK 20140415 이하 주석처리
	//ippsThreshold_LTVal_32f(pZmapTemp, pZmapTemp, sizeX * sizeY, (Ipp32f)80,  (Ipp32f)80);

	float sum = 0;
	float height = 0;
	int area = sizeX * sizeY;


	float pMin = 0.0;
	float pMax = 0.0;

	ippsMinMax_32f(pZmapTemp,area,&pMin,&pMax);

	int nLevels = 0;

//SHK 20140416
	if (pMax < LEAD_LIFT_MIN)
	{
		//delete pZmapTemp;
		g_pMManager->pem_delete(pZmapTemp, false);
		height = pMax;
		return height;
	}

//SHK 20140415
	if (pMin < LEAD_LIFT_MIN)
	{
		pMin = LEAD_LIFT_MIN;
	}
//////////////////////////////////////////////////////////////////////////

	nLevels = (int)(( pMax - pMin ) / LEAD_LIFT_DIV);

	nLevels = nLevels +3;

	Ipp32f* pLevels = ippsMalloc_32f(nLevels);

	//int* pZmapHistoData = new int[nLevels];
	int* pZmapHistoData = g_pMManager->pem_new<int>(true, nLevels, (PCHAR)__FUNCTION__, __LINE__);
	memset(pZmapHistoData, 0, sizeof(int) * (nLevels));

	for (int x=0;x<nLevels;x++)
	{
		pLevels[x] = pMin+ LEAD_LIFT_DIV*(x-1);
	}

	Ipp32f** ppLevels = &pLevels;

	IppiSize roiSize = {sizeX, sizeY};

// 	for(int i=0;i<area;i++)
// 	{
// 		for (int k=0;k<nLevels;k++)
// 		{
// 			if (pLevels[k] <= pZmapTemp[i] && pLevels[k+1] > pZmapTemp[i])
// 			{
// 				pZmapHistoData[k]++;
// 			}
// 		}
// 	}

	short ippStatus = 0;
	IppiHistogramSpec* histSpec = NULL;
	int sizeHistObj, sizeBuffer;
	ippStatus = ippiHistogramGetBufferSize(ipp32f, roiSize, pZmapHistoData, 1/*nChan*/, 0/*user step*/, &sizeHistObj, &sizeBuffer);
	histSpec = (IppiHistogramSpec*)ippsMalloc_8u(sizeHistObj);
	byte* histBuffer = NULL;
	histBuffer = (byte*)ippsMalloc_8u(sizeBuffer);

	// Initialize spec
	//ippStatus = ippiHistogramInit(ipp32f, &pLevels, pZmapHistoData, 1/*nChan*/, histSpec);

	int levelLength = 0;
	uint hist[255];		//레벨을 어떻게 지정해야 할지 NYJ 2020/10/08
	uint* pHist = &hist[0];

	ippiHistogram_32f_C1R(pZmapTemp, sizeof(float)*sizeX, roiSize, pHist, histSpec, histBuffer);

	ippsFree(histSpec);
	ippsFree(histBuffer);

	int* pZmapHistoDataT = g_pMManager->pem_new<int>(true, nLevels, (PCHAR)__FUNCTION__, __LINE__);
	memcpy_s(pZmapHistoDataT,sizeof(int) * (nLevels),pZmapHistoData, sizeof(int) * (nLevels));

	for (int n=1;n<nLevels-1;n++)
	{
		pZmapHistoDataT[n] = pZmapHistoData[n-1] + pZmapHistoData[n] + pZmapHistoData[n+1];
	}

	Ipp32s pInMax = 0; //max값은 중요하지 않음.
	int pIndex = 0;
	
	ippsMaxIndx_32s(pZmapHistoDataT,nLevels,&pInMax,&pIndex);

//SHK 20140415
	int tempMax = (int)(pZmapHistoDataT[pIndex] * LEAD_LIFT_HIGH_LIMIT);


	for (int i=pIndex;i<nLevels;i++)
	{
		if (pZmapHistoDataT[i] >= tempMax)
		{
			pIndex = i;
		}	
	}

//////////////////////////////////////////////////////////////////////////

	height = pMin+ LEAD_LIFT_DIV*(pIndex);//(pIndex-1);

	/*delete pZmapHistoData;
	delete pZmapHistoDataT;
	delete pZmapTemp;*/
	g_pMManager->pem_delete(pZmapHistoData, false);
	g_pMManager->pem_delete(pZmapHistoDataT, false);
	g_pMManager->pem_delete(pZmapTemp, false);

	ippsFree(pLevels);

	return height;
}

int CProc3D::CalcLineEquation(int* data, int cnts, int mode, CPoint* startCoordi, float* retCoef)
{
	/*float* coordiX = new float[cnts];
	float* coordiY = new float[cnts];*/
	float* coordiX = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
	float* coordiY = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
	float coef[2];


	int temp = 0;
	for(int i = 0; i < cnts; i++)
	{
		if(data[i] != -1)
			temp++;
	}

	//int* tempData = new int[temp];
	int* tempData = g_pMManager->pem_new<int>(true, temp, (PCHAR)__FUNCTION__, __LINE__);
	int index = 0;
	for(int i = 0; i < cnts; i++)
	{
		if(data[i] != -1)
		{
			tempData[index] = data[i];
			index++;
		}
	}
	double avrTemp = 0.0;
	double stdev = 0.0;
	GetStdev(tempData, 0, index, &avrTemp, &stdev);
	//delete tempData;
	g_pMManager->pem_delete(tempData, false);


	int max = 0;
	int min = 0;
	if(stdev < 2)
	{
		max = 1000000;
		min = -1000000;
	}
	else
	{
		max = (int)((avrTemp + (stdev / 2)) + 0.5);
		min = (int)((avrTemp - (stdev / 2)) + 0.5);
	}
	

	int sum = 0;
	int dataCnts = 0;
	int avr = 0;


	for(int i = 0; i < cnts; i++)
	{
		if(data[i] != -1 && data[i] < max && data[i] > min)
		{
			sum += data[i];

			switch(mode)
			{
			case e_LEFT:
				{
					coordiX[dataCnts] = (float)data[i];
					coordiY[dataCnts] = (float)startCoordi[i].y;
				}
				break;
			case e_RIGHT:
				{
					coordiX[dataCnts] = (float)(startCoordi[i].x + data[i]);
					coordiY[dataCnts] = (float)startCoordi[i].y;
				}
				break;
			case e_TOP:
				{
					coordiX[dataCnts] = (float)startCoordi[i].x;
					coordiY[dataCnts] = (float)data[i];
				}
				break;
			case e_BOTTOM:
				{
					coordiX[dataCnts] = (float)startCoordi[i].x;
					coordiY[dataCnts] = (float)(startCoordi[i].y + data[i]);
				}
				break;
			}
			dataCnts++;
		}
	}

	if(sum != 0)
		avr = sum / (dataCnts);
	else
		avr = 0;

	LeastSquareMethod(coordiX, coordiY, 1, dataCnts, coef);
	retCoef[0] = coef[0];
	retCoef[1] = coef[1];

// #ifdef _DEBUG
// 	CString path = _T("");
// 	if(mode == e_LEFT)
// 		path.Format(_T("D:\\TestImage\\data\\r_left.csv"));
// 	else if(mode == e_RIGHT)
// 		path.Format(_T("D:\\TestImage\\data\\r_right.csv"));
// 	else if(mode == e_TOP)
// 		path.Format(_T("D:\\TestImage\\data\\r_top.csv"));
// 	else if(mode == e_BOTTOM)
// 		path.Format(_T("D:\\TestImage\\data\\r_bottom.csv"));
// 
// 	DataSave(coordiX, coordiY, dataCnts, path);
// #endif


	/*delete coordiX;
	delete coordiY;*/
	g_pMManager->pem_delete(coordiX, false);
	g_pMManager->pem_delete(coordiY, false);

	return  avr;
}

void CProc3D::CalcLineEquation(POINT* data, int cnts, float* retCoef)
{
	/*float* coordiX = new float[cnts];
	float* coordiY = new float[cnts];*/
	float* coordiX = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
	float* coordiY = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
	float coef[2];


	for(int i = 0; i < cnts; i++)
	{
		coordiX[i] = (float)data[i].x;
		coordiY[i] = (float)data[i].y;
	}

	LeastSquareMethod(coordiX, coordiY, 1, cnts, coef);
	retCoef[0] = coef[0];
	retCoef[1] = coef[1];


	/*delete coordiX;
	delete coordiY;*/
	g_pMManager->pem_delete(coordiX, false);
	g_pMManager->pem_delete(coordiY, false);
}

double CProc3D::CalcLineEquation(CPoint* data, int cnts, int mode, int width, int height, CPoint* startCoordi, float* retCoef)
{
	/*float* coordiX = new float[cnts];
	float* coordiY = new float[cnts];*/
	float* coordiX = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
	float* coordiY = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);

	int tempAng = 0;
	if(mode == e_LEFT || mode == e_RIGHT)
	{
		tempAng = 90;

	}
	else if(mode == e_TOP || mode == e_BOTTOM)
	{
		tempAng = 0;
	}

	for(int i = 0; i < cnts; i++)
	{
		coordiX[i] = (float)data[i].x;
		coordiY[i] = (float)data[i].y;
	}

	double rstRho = 0.0;
	double rstDegrees = 0.0;
	HoughLine(coordiX, coordiY, cnts, width, height, tempAng, 30, &rstRho, &rstDegrees);


	double radian = rstDegrees * (PI / 180);
	retCoef[0] = (float)(rstRho / cos(radian));
	retCoef[1] = (float)((sin(radian) / cos(radian)) * -1);


	/*delete coordiX;
	delete coordiY;	*/
	g_pMManager->pem_delete(coordiX, false);
	g_pMManager->pem_delete(coordiY, false);

	return  rstDegrees;
}

double CProc3D::CalcLineEquation(int* data, int cnts, int mode, int width, int height, CPoint* startCoordi, float* retCoef)
{
	/*float* coordiX = new float[cnts];
	float* coordiY = new float[cnts];*/
	float* coordiX = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
	float* coordiY = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);


	int dataCnts = 0;
	int tempAng = 0;
	if(mode == e_LEFT || mode == e_RIGHT)
	{
		tempAng = 90;

	}
	else if(mode == e_TOP || mode == e_BOTTOM)
	{
		tempAng = 0;
	}

	
	for(int i = 0; i < cnts; i++)
	{
		if(data[i] != -1)
		{
			switch(mode)
			{
			case e_LEFT:
				{
					coordiX[dataCnts] = (float)data[i];
					coordiY[dataCnts] = (float)startCoordi[i].y;
				}
				break;
			case e_RIGHT:
				{
					coordiX[dataCnts] = (float)(startCoordi[i].x + data[i]);
					coordiY[dataCnts] = (float)startCoordi[i].y;
				}
				break;
			case e_TOP:
				{
					coordiX[dataCnts] = (float)startCoordi[i].x;
					coordiY[dataCnts] = (float)data[i];
				}
				break;
			case e_BOTTOM:
				{
					coordiX[dataCnts] = (float)startCoordi[i].x;
					coordiY[dataCnts] = (float)(startCoordi[i].y + data[i]);
				}
				break;
			}
			dataCnts++;
		}
	}

	double rstRho = 0.0;
	double rstDegrees = 0.0;
	HoughLine(coordiX, coordiY, dataCnts, width, height, tempAng, 30, &rstRho, &rstDegrees);
	//HoughLine2(coordiX, coordiY, cnts, width, height, tempAng, 30, &rstRho, &rstDegrees);

	double radian = rstDegrees * (PI / 180);
	retCoef[0] = (float)(rstRho / cos(radian));
	retCoef[1] = (float)((sin(radian) / cos(radian)) * -1);
	

	/*delete coordiX;
	delete coordiY;	*/
	g_pMManager->pem_delete(coordiX, false);
	g_pMManager->pem_delete(coordiY, false);

	return  rstDegrees;
}

double CProc3D::CalcLineEquation(POINT* data, int cnts, int mode, int width, int height, float* retCoef)
{
	/*float* coordiX = new float[cnts];
	float* coordiY = new float[cnts];*/
	float* coordiX = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
	float* coordiY = g_pMManager->pem_new<float>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);

	int tempAng = 0;
	if(mode == e_LEFT || mode == e_RIGHT)
	{
		tempAng = 90;

	}
	else if(mode == e_TOP || mode == e_BOTTOM)
	{
		tempAng = 0;
	}

	for(int i = 0; i < cnts; i++)
	{
		coordiX[i] = (float)data[i].x;
		coordiY[i] = (float)data[i].y;
	}

	double rstRho = 0.0;
	double rstDegrees = 0.0;
	HoughLine(coordiX, coordiY, cnts, width, height, tempAng, 30, &rstRho, &rstDegrees);
	//HoughLine2(coordiX, coordiY, cnts, width, height, tempAng, 30, &rstRho, &rstDegrees);

	double radian = rstDegrees * (PI / 180);
	retCoef[0] = (float)(rstRho / cos(radian));
	retCoef[1] = (float)((sin(radian) / cos(radian)) * -1);


	/*delete coordiX;
	delete coordiY;	*/
	g_pMManager->pem_delete(coordiX, false);
	g_pMManager->pem_delete(coordiY, false);

	return  rstDegrees;
}

bool CProc3D::LeastSquareMethod(float x[], float y[], UINT nOrder, UINT nDatNum, float Coef[])
{
	// 1-D least square method
	// pX, pY - x, y data points
	// nDatNum - x, y data number of points
	// nOrder - 근사식의 차수
	// pCoef - 근사식의 계수 (y=Coef[0]+Coef[1]x+Coef[2]x^2+...), n차식이면 계수의 개수는 n+1개

	int  i, j, k, l;
	float  w1, w2, w3, pivot, aik, a[21][22], w[42];
	int n=nOrder;
	int number=nDatNum;

	if(n >= number || n < 1 || n > 20) return false;
	for(i = 0; i < n*2; i++) {
		w1 = 0.0;
		for(j = 0; j < number; j++) {
			w2 = w3 = x[j];
			for(k = 0; k < i; k++)	w2 *= w3;
			w1 += w2;
		}
		w[i] = w1;
	}

	// matrix 입력
	for(i = 0; i < n+1; i++) {
		for(j = 0; j < n+1; j++) {
			l = i + j - 1;
			a[i][j] = w[l];
		}
	}
	a[0][0] = (float)number;
	w1 = 0.0;
	for(i = 0; i < number; i++) w1 += y[i];
	a[0][n+1] = w1;

	// sigma(Yi Xi) 계산해서 대입
	for(i = 0; i < n; i++) {
		w1 = 0.0;
		for(j = 0; j < number; j++) {
			w2 = w3 = x[j];
			for(k = 0; k < i; k++) w2 *= w3;
			w1 += y[j] * w2;
		}
		a[i+1][n+1] = w1;
	}

	// matrix 계산
	for(k = 0; k < n+1; k++) {
		pivot = a[k][k];
		for(j = k; j < n+2; j++) a[k][j] /= pivot;
		for(i = 0; i < n+1; i++) {
			if(i != k) {
				aik = a[i][k];
				for(j = k; j < n+2; j++) a[i][j] -= aik * a[k][j];
			}
		}
	}

	// 다항식의 계수를 최종적으로 전달 (pass by reference)
	for(i = 0; i < n+1; i++) Coef[i] = a[i][n+1];

	return true;
}

int CProc3D::LeastSquareMethod_XY(float *arrX, float *arrY, float *arrZ, int len, float* coefX, float* coefY, float* coefC)
{
	if((arrX==NULL) || (arrY==NULL) || (arrZ==NULL)) return -1;
	if(len <= 2) return -1;

	float sigmaXSq = 0.f;
	float sigmaXY = 0.f;
	float sigmaX = 0.f;
	float sigmaYSq = 0.f;
	float sigmaY = 0.f;
	float sigmaM = 0.f;
	float sigmaZX = 0.f;
	float sigmaZY = 0.f;
	float sigmaZ = 0.f;

	const int rowU = 3;
	const int colU = 3;
	const int rowV = 3;
	const int rowC = 3;

	float matU[rowU*colU] = {0.f};
	float imatU[colU*rowU] = {0.f};
	float matV[rowV] = {0.f};
	float matC[rowC] = {0.f};

	float x, y, z;
	for(int m=0 ; m<len ; m++)
	{
		x = arrX[m];
		y = arrY[m];
		z = arrZ[m];

		sigmaXSq += (x * x);
		sigmaXY += (x * y);
		sigmaX += x;
		sigmaYSq += (y * y);
		sigmaY += y;
		sigmaM += 1.f;
		sigmaZX += (z * x);
		sigmaZY += (z * y);
		sigmaZ += z;
	}

	// Fill matrix U
	matU[0] = sigmaXSq;
	matU[1] = sigmaXY;
	matU[2] = sigmaX;
	matU[3] = sigmaXY;
	matU[4] = sigmaYSq;
	matU[5] = sigmaY;
	matU[6] = sigmaX;
	matU[7] = sigmaY;
	matU[8] = sigmaM;

	// Fill matrix V
	matV[0] = sigmaZX;
	matV[1] = sigmaZY;
	matV[2] = sigmaZ;

	// temporary buffer - longer side length x 2
	Ipp32f ippBuff[rowU*rowU];

	// Calculate inverse matrix U'
	/*int srcWidth = rowU;
	int srcStride2 = sizeof(float);
	int srcStride1 = srcStride2 * srcWidth;*/
	//ippmInvert_m_32f(matU, srcStride1, srcStride2, ippBuff, imatU, srcStride1, srcStride2, srcWidth);

	cv::Mat org(rowU, colU, CV_32FC1, matU);
	cv::Mat sub(rowV, 1, CV_32FC1, matV);
	cv::Mat dst = org.inv() * sub;

	matC[0] = dst.at<float>(0, 0);
	matC[1] = dst.at<float>(1, 0);
	matC[2] = dst.at<float>(2, 0);

	// Coefficients(Result) matrix = imatU' * matV
	/*
	int src1Width = colU;
	int src1Height = rowU;
	int src1Stride2 = sizeof(float);
	int src1Stride1 = src1Stride2 * src1Width;
	int src2Len = rowV;
	int src2Stride2 = sizeof(float);
	int dstStride2 = sizeof(float);
	//ippmMul_mv_32f(imatU, src1Stride1, src1Stride2, src1Width, src1Height, matV, src2Stride2, src2Len, matC, dstStride2);
	*/

	*coefX = matC[0];
	*coefY = matC[1];
	*coefC = matC[2];

	return 0;
}


void CProc3D::HoughLine(float* x, float* y, int dataCnts, int width, int height, int ang, int t, double* retRho, double* retAngle)
{	
	
	float fStepAngle = 0.5;
	int num_rho = (int)(sqrt((double)(width * width + height * height)) * 2);
	int num_ang = (int)(180 / fStepAngle);
	float tableFactor = (float)(PI / num_ang);
	double rhoHalf = num_rho / 2.0;

	int st = (int)((ang - t) / fStepAngle);
	int ed = (int)((ang + t) / fStepAngle);

	int index = 0;
	double theta = 0;


	//look up table
/*	double* tsin = new double[num_ang];
	double* tcos = new double[num_ang];

	
	for(int n = st; n < ed; n++)
	{
		if(n < 0)
		{
			index = n + num_ang;
		}
		else if(n >= num_ang)
		{
			index = n - num_ang;
		}
		else
		{
			index = n;
		}
		theta = index * tableFactor;
		tsin[index] = (double)sin(theta);
		tcos[index] = (double)cos(theta);
	}
	
	*/
	//accumulate array
	//short* arr = new short[num_rho * num_ang];
	short* arr = g_pMManager->pem_new<short>(true, num_rho * num_ang, (PCHAR)__FUNCTION__, __LINE__);
	memset(arr, 0, sizeof(short) * num_rho * num_ang );


	index = 0;
	int temp = 0;
	float fx, fy;
	for(int i = 0; i < dataCnts; i++)
	{
		fx = x[i];
		fy = y[i];
		for(int n = st; n < ed; n++)
		{
			if(n < 0)
			{
				index = n + num_ang;
			}
			else if(n >= num_ang)
			{
				index = n - num_ang;
			}
			else
			{
				index = n;
			}
			
			int AngIndex = n+360;
			if(n >= 0) temp = (int)floor(fx * g_LTSin[AngIndex] + fy * g_LTCos[AngIndex] + 0.5);
			else temp = (int)floor(-fx * g_LTSin[AngIndex] - fy * g_LTCos[AngIndex] + 0.5);
			//temp = (int)floor(fx * tsin[index] + fy * tcos[index] + 0.5);
			temp += (int)rhoHalf;    // num_rho / 2

			arr[index + (temp * num_ang)]++;
		}
	}


	// JACKY 2013/10/02 JUST TEST CODE  (time incleased, good result)	
// 	int nMaskSum = 2;
// 	int nWidth = num_ang;
// 	int nHeight = num_rho;
// 	short* arrSum = new short[num_rho * num_ang];
// 	memcpy(arrSum, arr, sizeof(short)*nWidth*nHeight);
// 
// 	for(int i=nMaskSum; i<nHeight-nMaskSum; i++)
// 	{
// 		for(int j=nMaskSum; j<nWidth-nMaskSum; j++)
// 		{
// 			short nSum = 0;
// 			for(int m=-nMaskSum; m<=nMaskSum; m++)
// 			{
// 				for(int n=-nMaskSum; n<=nMaskSum; n++)
// 				{
// 					nSum += arr[ (j+n) + (i+nMaskSum)*nWidth ];
// 				}
// 			}
// 			arrSum[j + i*nWidth] = nSum;
// 		}
// 	}
// 	memcpy(arr, arrSum, sizeof(short)*nWidth*nHeight);	
// 	delete [] arrSum;
	// END OF TEST CODE


	// 	SHK 2013/10/04
// 	float* arrf = new float[sizeof(float)*num_rho*num_ang];
// 	IppiSize ippsize;
// 	ippsize.height = num_rho-5;
// 	ippsize.width = num_ang-5;
// 
// 	ippiSumWindowColumn_16s32f_C1R(arr,num_ang,arrf,num_ang,ippsize,5,2);
// 	ippiConvert_32f16s_C1R(arrf,num_ang,arr,num_ang,ippsize,ippRndNear );
// 	ippiSumWindowRow_16s32f_C1R(arr,num_ang,arrf,num_ang,ippsize,5,2);
// 	ippiConvert_32f16s_C1R(arrf,num_ang,arr,num_ang,ippsize,ippRndNear );
// 
// 	delete arrf;
	// 	// END OF TEST CODE

	//search max value
	double rhoTemp = 0.0;
	double angTemp = 0.0;
	int arr_max = 0;	
//////////////////////////////////////////////////////////////////////////
	// JACKY 2013/9/30
	short MaxVal = 0;
	int MaxIndex = 0;
	ippsMaxIndx_16s(arr, num_rho * num_ang, &MaxVal, &MaxIndex); 
	int nMaxTheta = MaxIndex % num_ang;
	int nMaxRho = (int)((float)MaxIndex / num_ang);

//////////////////////////////////////////////////////////////////////////
	
/*	index = 0;
	int nMaxTheta = 0;
	int nMaxRho = 0;
	for(int i = 0; i < num_rho; i++)
	{
		for(int n = st; n < ed; n++)
		{
			if(n < 0)
			{
				index = n + num_ang;
			}
			else if(n >= num_ang)
			{
				index = n - num_ang;
			}
			else
			{
				index = n;
			}

			if(arr[index + (i * num_ang)] > arr_max)
			{
				arr_max = arr[index + (i * num_ang)];
				rhoTemp = i - rhoHalf;
				angTemp = index * fStepAngle;
				nMaxTheta = index;
				nMaxRho = i;
			}
		}
	}*/
//////////////////////////////////////////////////////////////////////////
	
	

	// index = theta (col)
	// i     = rho   (row)
	int nMask = 3;	//7*7  (3 + 1 + 3)
	int nTotalSumTheta = 0;
	int nTotalSumRho = 0;
	int nValSum = 0;
	int nTarget = 0;
	int temp1 = nMaxRho-nMask;
	int temp2 = nMaxTheta-nMask;

	if(temp1 < 0)
		temp1 = 0;
	
	if(temp2 < 0)
		temp2 = 0;

	short nValTmp = 0;
	for(int i=temp1; i<=nMaxRho+nMask; i++)
	{
		for(int j=temp2; j<=nMaxTheta+nMask; j++)
		{
			nTarget = j + i*num_ang;
			nValTmp = arr[nTarget];			
			nValSum += nValTmp;
			nTotalSumTheta += nValTmp * j;
			nTotalSumRho += nValTmp * i;
		}
	}
	double MaxRho = (float)nTotalSumRho / nValSum;
	double MaxTheta = (float)nTotalSumTheta / nValSum;

	rhoTemp = MaxRho - rhoHalf;
	angTemp = MaxTheta * fStepAngle;
	
	*retRho = rhoTemp;
	*retAngle = angTemp;

// 	delete [] tsin;
// 	delete [] tcos;
	//delete [] arr;
	g_pMManager->pem_delete(arr, true);

}


// void CProc3D::HoughLine(float* x, float* y, int dataCnts, int width, int height, int ang, int t, double* retRho, double* retAngle)
// {	
// 	float fStepAngle = 0.5;
// 	int num_rho = (int)(sqrt((double)(width * width + height * height)) * 2);
// 	int num_ang = (int)(180 / fStepAngle);
// 	float tableFactor = (float)(PI / num_ang);
// 	double rhoHalf = num_rho / 2.0;
// 
// 	int st = (int)((ang - t) / fStepAngle);
// 	int ed = (int)((ang + t) / fStepAngle);
// 
// 	int index = 0;
// 	double theta = 0;
// 
// 	//look up table
// /*	double* tsin = new double[num_ang];
// 	double* tcos = new double[num_ang];
// 
// 	int st = (int)((ang - t) / fStepAngle);
// 	int ed = (int)((ang + t) / fStepAngle);
// 	
// 	for(int n = st; n < ed; n++)
// 	{
// 		if(n < 0)
// 		{
// 			index = n + num_ang;
// 		}
// 		else if(n >= num_ang)
// 		{
// 			index = n - num_ang;
// 		}
// 		else
// 		{
// 			index = n;
// 		}
// 		theta = index * tableFactor;
// 		tsin[index] = (double)sin(theta);
// 		tcos[index] = (double)cos(theta);
// 	}*/
// 
// 	//accumulate array	
// 	short* arr = new short[num_rho * num_ang];
// 	memset(arr, 0, sizeof(short) * num_rho * num_ang );
// 
// 	index = 0;
// 	int temp = 0;
// 	float fx, fy;
// 
// 	for(int i = 0; i < dataCnts; i++)
// 	{
// 		fx = x[i];
// 		fy = y[i];		
// 		for(int n = st; n < ed; n++)
// 		{
// 			if(n < 0)
// 			{
// 				index = n + num_ang;
// 			}
// 			else if(n >= num_ang)
// 			{
// 				index = n - num_ang;
// 			}
// 			else
// 			{
// 				index = n;
// 			}
// 
// 			int AngIndex = n+360;
// 			if(n >= 0) temp = (int)floor(fx * g_LTSin[AngIndex] + fy * g_LTCos[AngIndex] + 0.5);
// 			else temp = (int)floor(-fx * g_LTSin[AngIndex] - fy * g_LTCos[AngIndex] + 0.5);
// 
// 
// 			//temp = (int)floor(fx * tsin[index] + fy * tcos[index] + 0.5);
// 			temp += (int)rhoHalf;    // num_rho / 2
// 
// 
// 		}	
// 	}
// 			arr[index + (temp * num_ang)]++;
// 	// JACKY 2013/10/02 JUST TEST CODE  (time incleased, good result)	
// 	int nMaskSum = 2;
// 	int nWidth = num_ang;
// 	int nHeight = num_rho;
// 	short* arrSum = new short[num_rho * num_ang];
// 	memcpy(arrSum, arr, sizeof(short)*nWidth*nHeight);
// 
// 	for(int i=nMaskSum; i<nHeight-nMaskSum; i++)
// 	{
// 		for(int j=nMaskSum; j<nWidth-nMaskSum; j++)
// 		{
// 			short nSum = 0;
// 			for(int m=-nMaskSum; m<=nMaskSum; m++)
// 			{
// 				for(int n=-nMaskSum; n<=nMaskSum; n++)
// 				{
// 					nSum += arr[ (j+n) + (i+nMaskSum)*nWidth ];
// 				}
// 			}
// 			arrSum[j + i*nWidth] = nSum;
// 		}
// 	}
// 	memcpy(arr, arrSum, sizeof(short)*nWidth*nHeight);	
// 	delete [] arrSum;
// 	// END OF TEST CODE
// 
// 	//search max value
// 	double rhoTemp = 0.0;
// 	double angTemp = 0.0;
// 	int arr_max = 0;
// //	double rhoHalf = num_rho / 2.0;
// //////////////////////////////////////////////////////////////////////////
// 	// JACKY 2013/9/30
// 	short MaxVal = 0;
// 	int MaxIndex = 0;
// 	ippsMaxIndx_16s(arr, num_rho * num_ang, &MaxVal, &MaxIndex); 
// 	int nMaxTheta = MaxIndex % num_ang;
// 	int nMaxRho = (int)((float)MaxIndex / num_ang);
// 
// //////////////////////////////////////////////////////////////////////////
// 	
// 	/*
// 	index = 0;
// 	int nMaxTheta = 0;
// 	int nMaxRho = 0;
// 	for(int i = 0; i < num_rho; i++)
// 	{
// 		for(int n = st; n < ed; n++)
// 		{
// 			if(n < 0)
// 			{
// 				index = n + num_ang;
// 			}
// 			else if(n >= num_ang)
// 			{
// 				index = n - num_ang;
// 			}
// 			else
// 			{
// 				index = n;
// 			}
// 
// 			if(arr[index + (i * num_ang)] > arr_max)
// 			{
// 				arr_max = arr[index + (i * num_ang)];
// 				rhoTemp = i - rhoHalf;
// 				angTemp = index * fStepAngle;
// 				nMaxTheta = index;
// 				nMaxRho = i;
// 			}
// 		}
// 	}*/
// //////////////////////////////////////////////////////////////////////////
// 	
// 	
// 
// 	// index = theta (col)
// 	// i     = rho   (row)
// 	int nMask = 3;	//7*7  (3 + 1 + 3)
// 	int nTotalSumTheta = 0;
// 	int nTotalSumRho = 0;
// 	int nValSum = 0;
// 	int nTarget = 0;
// 	int temp1 = nMaxRho-nMask;
// 	int temp2 = nMaxTheta-nMask;
// 
// 	if(temp1 < 0)
// 		temp1 = 0;
// 	
// 	if(temp2 < 0)
// 		temp2 = 0;
// 
// 	for(int i=temp1; i<=nMaxRho+nMask; i++)
// 	{
// 		for(int j=temp2; j<=nMaxTheta+nMask; j++)
// 		{
// 			nTarget = j + i*num_ang;
// 			nValSum += arr[nTarget];
// 			nTotalSumTheta += arr[nTarget] * j;
// 			nTotalSumRho += arr[nTarget] * i;
// 		}
// 	}
// 	double MaxRho = (float)nTotalSumRho / nValSum;
// 	double MaxTheta = (float)nTotalSumTheta / nValSum;
// 
// 	rhoTemp = MaxRho - rhoHalf;
// 	angTemp = MaxTheta * fStepAngle;
// 	
// 	*retRho = rhoTemp;
// 	*retAngle = angTemp;
// 
// // 	delete [] tsin;
// // 	delete [] tcos;
// 	delete [] arr;
// 
// }


void CProc3D::HoughLine2(float* x, float* y, int dataCnts, int width, int height, int ang, int t, double* retRho, double* retAngle)
{	
	float fStepAngle = 0.5;
	int num_rho = (int)(sqrt((double)(width * width + height * height)) * 2);
	int num_ang = (int)(180 / fStepAngle);
	float tableFactor = (float)(PI / num_ang);
	int rhoHalf = num_rho / 2;

	//look up table
	/*double* tsin = new double[num_ang];
	double* tcos = new double[num_ang];*/
	double* tsin = g_pMManager->pem_new<double>(true, num_ang, (PCHAR)__FUNCTION__, __LINE__);
	double* tcos = g_pMManager->pem_new<double>(true, num_ang, (PCHAR)__FUNCTION__, __LINE__);

	int st = (int)((ang - t) / fStepAngle);
	int ed = (int)((ang + t) / fStepAngle);

	int index = 0;
	double theta = 0;
	for(int n = st; n < ed; n++)
	{
		if(n < 0)
		{
			index = n + num_ang;
		}
		else if(n >= num_ang)
		{
			index = n - num_ang;
		}
		else
		{
			index = n;
		}
		theta = index * tableFactor;
		tsin[index] = (double)sin(theta);
		tcos[index] = (double)cos(theta);
	}

	//accumulate array
	//short* arr = new short[num_rho * num_ang];
	short* arr = g_pMManager->pem_new<short>(true, num_rho * num_ang, (PCHAR)__FUNCTION__, __LINE__);
	memset(arr, 0, sizeof(short) * num_rho * num_ang );

	index = 0;
	int temp = 0;
	float fx, fy;
	for(int i = 0; i < dataCnts; i++)
	{
		fx = x[i];
		fy = y[i];
		for(int n = st; n < ed; n++)
		{
			if(n < 0)
			{
				index = n + num_ang;
			}
			else if(n >= num_ang)
			{
				index = n - num_ang;
			}
			else
			{
				index = n;
			}

			temp = (int)floor(fx * tsin[index] + fy * tcos[index] + 0.5);
			temp += (num_rho / 2);    // num_rho / 2

			arr[index + (temp * num_ang)]++;
		}
	}

	const int max_cnt = 50;
	int max_idx = max_cnt - 1;
	int thresCnt = 10;
	int mv[max_cnt] = {0,};
	float mt[max_cnt] = {0.f,};
	float mr[max_cnt] = {0.f,};

	int cur_idx = 0;
	for (int rho=0 ; rho<num_rho ; rho++)
	{
		for (int theta=0 ; theta<num_ang ; theta++)
		{
			int val = arr[rho * num_ang + theta];

			if (val > thresCnt)
			{
				if (cur_idx != 0)
				{
					for (int m=0 ; m<cur_idx ; m++)
					{
						if (val >= mv[m])
						{
							for (int k=cur_idx-1 ; k>=m ; k--)
							{
								mv[k+1] = mv[k];
								mt[k+1] = mt[k];
								mr[k+1] = mr[k];
							}

							mv[m] = val;
							mt[m] = (float)theta;
							mr[m] = (float)rho;
							cur_idx++;
							if (cur_idx >= max_idx)
								cur_idx = max_idx;

							m = cur_idx;	// break
						}					
					}
				}
				else
				{
					mv[0] = val;
					mt[0] = (float)theta;
					mr[0] = (float)rho;
					cur_idx++;
				}
			}
		}
	}

	float mt_std = 0.f;
	float mr_std = 0.f;
	float mt_sum = 0.f;
	float mr_sum = 0.f;
	float mt_mean = 0.f;
	float mr_mean = 0.f;
	float dev = 0.f;
	int _cnt = 0;
	int data_cnt = cur_idx + 1;

	ippsStdDev_32f(mt, data_cnt, &mt_std, ippAlgHintNone);
	ippsMean_32f(mt, data_cnt, &mt_mean, ippAlgHintNone);

	_cnt = 0;
	for (int m=0 ; m<data_cnt ; m++)
	{
		dev = (float)mt[m] - mt_mean;
		if (fabs(dev) <= mt_std)
		{
			mt_sum += (float)mt[m];
			mr_sum += (float)mr[m];
			_cnt++;
		}
	}

// 	_cnt = 0;
// 	for (int m=0 ; m<data_cnt ; m++)
// 	{
// 		dev = (float)mt[m] - mt_mean;
// 		if (fabs(dev) <= mt_std)
// 		{
// 			mt[_cnt] = mt[m];
// 			mr[_cnt] = mr[m];
// 			_cnt++;
// 		}
// 	}
// 
// 	data_cnt = _cnt;
// 	ippsStdDev_32f(mr, data_cnt, &mr_std, ippAlgHintNone);
// 	ippsMean_32f(mr, data_cnt, &mr_mean, ippAlgHintNone);
// 
// 	_cnt = 0;
// 	for (int m=0 ; m<data_cnt ; m++)
// 	{
// 		dev = (float)mr[m] - mr_mean;
// 		if (fabs(dev) <= mr_mean)
// 		{
// 			mt_sum += (float)mt[m];
// 			mr_sum += (float)mr[m];
// 			_cnt++;
// 		}
// 	}

	mt_mean = mt_sum / (float)_cnt;
	mr_mean = mr_sum / (float)_cnt;

	*retRho = mr_mean - rhoHalf;
	*retAngle = mt_mean * fStepAngle;

	/*delete [] tsin;
	delete [] tcos;
	delete [] arr;*/
	g_pMManager->pem_delete(tsin, true);
	g_pMManager->pem_delete(tcos, true);
	g_pMManager->pem_delete(arr, true);
}


void CProc3D::MakeLUT_Poly(BYTE* dst, int sizeX, int sizeY, const POINT *pts, int ptNum)
{
	BYTE* dstTemp = NULL;
	BYTE* tempBuf = NULL;

	//tempBuf = new BYTE[sizeX * sizeY];
	tempBuf = g_pMManager->pem_new<BYTE>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(tempBuf, 0, (sizeof(BYTE) * sizeX * sizeY));

	POINT tempPt;
	BOOL rst = FALSE;

	dstTemp = tempBuf;
	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			tempPt.x = x;
			tempPt.y = y;
			rst = PtInPolygon(tempPt, pts, ptNum);

			if(rst)
				*dstTemp++ = 255;
			else
				*dstTemp++ = 0;

		}
	}

	memcpy_s(dst, (sizeof(BYTE) * sizeX * sizeY),
		tempBuf, (sizeof(BYTE) * sizeX * sizeY));

	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);
}

bool CProc3D::PtInPolygon(POINT pt, const POINT *pts, int ptNum)
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

float CProc3D::MaskingData(float* src, BYTE* mask, float* dst, int sizeX, int sizeY, float defaultVal)
{
	BYTE data = 0;

	float sum = 0.;
	int cnts = 0;
	float avr = 0.;

	for(int y = 0; y < sizeY; y++)
	{	
		for(int x = 0; x < sizeX; x++)
		{
			data = mask[(y * sizeX) + x];
			if(data >= 1)
			{
				if(dst != NULL)
					dst[(y * sizeX) + x] = src[(y * sizeX) + x];

				sum += src[(y * sizeX) + x];
				cnts++;
			}
			else
			{
				if(dst != NULL)
					dst[(y * sizeX) + x] = defaultVal;
			}
		}
	}

	avr = sum / cnts;
	return avr;
}

float CProc3D::MaskingData_rev(float* src, BYTE* mask, float* dst, int sizeX, int sizeY, float defaultVal)
{
	BYTE data = 0;

	float sum = 0.;
	int cnts = 0;
	float avr = 0.;

	for(int y = 0; y < sizeY; y++)
	{	
		for(int x = 0; x < sizeX; x++)
		{
			data = mask[(y * sizeX) + x];
			if(data >= 1)
			{
				if(dst != NULL)
					dst[(y * sizeX) + x] = defaultVal;

				sum += src[(y * sizeX) + x];
				cnts++;
			}
			else
			{
				if(dst != NULL)
					dst[(y * sizeX) + x] = src[(y * sizeX) + x];				
			}
		}
	}

	avr = sum / cnts;
	return avr;
}


//////////////////////////////////////////////////////////////////////////
void CProc3D::ExtractLeadData(float* zmapData, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY, UCHAR* grayImg)
{
	if(zmapData == NULL  || orgSizeX < 0 || orgSizeY < 0 || cx < 0 || cy < 0 || roiSizeX < 0 || roiSizeY < 0)
		return;

	
	float* pZmapRoiData = NULL;
	//pZmapRoiData = new float[roiSizeX * roiSizeY];
	pZmapRoiData = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(pZmapRoiData, 0, (sizeof(float) * roiSizeX * roiSizeY));
	GetClipZmapData(zmapData, pZmapRoiData, orgSizeX, orgSizeY, cx, cy, roiSizeX, roiSizeY);

//////////////////////////////////////////////////////////////////////////

	MakeRoughGrayImg(pZmapRoiData, grayImg, roiSizeX, roiSizeY);

	//delete pZmapRoiData;
	g_pMManager->pem_delete(pZmapRoiData, false);
}


void CProc3D::MakeRoughGrayImg(float* src, UCHAR* dst, int sizeX, int sizeY)
{
	if(src == NULL || dst == NULL || sizeX < 0 || sizeY < 0)
		return;

	float hMax = 0;
	float hMin = 0;
	UCHAR* temp = NULL;
	//temp = new UCHAR[sizeX * sizeY];
	temp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, (sizeof(UCHAR) * sizeX * sizeY));

	float stdMin = 0;
	float stdMax = 0;
	float avr = GetZmapDataAvr(src, sizeX, sizeY);
	GetHlimit(src, sizeX, sizeY, &hMax, &hMin);

	float val = 0;
	val = GetZmapDataAvr(src, sizeX, sizeY, hMin, avr);
	stdMin = val;
	val =  GetZmapDataAvr(src, sizeX, sizeY, avr, hMax);
	stdMax = val;

	float* srcTemp = NULL;
	UCHAR* dstTemp = NULL;
	srcTemp = src;
	dstTemp = temp;
	float data = 0;
	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{	
			data = *srcTemp++;
			if(data == NODATA)
			{	
				*dstTemp++ = 0;
			}
			else
			{
				if(data >= stdMin && data <= stdMax)
					*dstTemp++ = (UCHAR)(((data - stdMin) / (stdMax - stdMin)) * 255) / 1;
				else if(data < stdMin)
					*dstTemp++ = 0;
				else if(data > stdMax)
					*dstTemp++ = 255;
			}
		}
	}

	memcpy_s(dst, (sizeof(UCHAR) * sizeX * sizeY), temp, (sizeof(UCHAR) * sizeX * sizeY));

	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

void CProc3D::MakeRoughGrayImg(float* src, UCHAR* dst, int sizeX, int sizeY, int minRange, BOOL inverse)
{
	if(src == NULL || dst == NULL || sizeX < 0 || sizeY < 0)
		return;

	float hMax = 0;
	float hMin = 0;
	UCHAR* temp = NULL;
	//temp = new UCHAR[sizeX * sizeY];
	temp = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, (sizeof(UCHAR) * sizeX * sizeY));

	float stdRange = (float)minRange;
	if(minRange < 0)
		stdRange = 0;

	float* srcTemp = NULL;
	UCHAR* dstTemp = NULL;
	srcTemp = src;
	dstTemp = temp;

	float data = 0;
	for (int y=0 ; y<sizeY ; y++)
	{
		for (int x=0 ; x<sizeX ; x++)
		{	
			data = *srcTemp++;
			if(data == NODATA)
			{	
				*dstTemp++ = 0;
			}
			else
			{
				if(inverse)
				{
					if(data > stdRange)
						*dstTemp++ = 0;
					else
						*dstTemp++ = 255;
				}
				else
				{
					if(/*stdRange + 400 > data && */data >= stdRange)
						*dstTemp++ = 255;
					else
						*dstTemp++ = 0;
				}
			}
		}
	}

	memcpy_s(dst, (sizeof(UCHAR) * sizeX * sizeY), temp, (sizeof(UCHAR) * sizeX * sizeY));

	//delete temp;
	g_pMManager->pem_delete(temp, false);
}

float CProc3D::GetZmapDataAvr(float* src, int sizeX, int sizeY)
{
	if(src == NULL || sizeX < 0 || sizeY < 0)
		return 1;

	int cnts = 0;
	double sum = 0;
	float avr = 0;

	for(int i = 0; i < sizeX * sizeY; i++)
	{
		if(src[i] != NODATA)
		{
			sum += src[i];
			cnts++;
		}
	}

	avr = (float)(sum / cnts);

	return avr;
}

float CProc3D::GetZmapDataAvr(float* src, int sizeX, int sizeY, float minRange, float maxRange)
{
	if(src == NULL || sizeX < 0 || sizeY < 0)
		return 1;

	int cnts = 0;
	float sum = 0;
	float avr = 0;

	for(int i = 0; i < sizeX * sizeY; i++)
	{
		if(src[i] >= minRange && src[i] <= maxRange)
		{
			sum += src[i];
			cnts++;
		}
	}

	avr = sum / cnts;

	return avr;
}

BOOL CProc3D::InspLeadArea(float* leadData, int sizeX, int sizeY, int index, UCHAR* grayImage, BOOL* retResult)
{
	BOOL ret = FALSE;

	if(leadData == NULL  || sizeX < 0 || sizeY < 0)
		return ret;

	//UCHAR* tempBuf = new UCHAR[sizeX * sizeY];
	UCHAR* tempBuf = g_pMManager->pem_new<UCHAR>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	//memset(tempBuf, 0, (sizeof(UCHAR) * sizeX * sizeY));
	MakeRoughGrayImg(leadData, tempBuf, sizeX, sizeY);

	memcpy_s(grayImage, (sizeof(UCHAR) * sizeX * sizeY), tempBuf, (sizeof(UCHAR) * sizeX * sizeY));

	int size = 1 * sizeY;
	float* profileData_v = NULL;
	//profileData_v = new float[size];
	profileData_v = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);

	int stX = 0;
	int stY = 0;

	CString fileName = _T("");
	int vtInterval = 3;
	for(int i = 0; i < vtInterval; i++)
	{
		stX = (int)((sizeX / ((vtInterval + 1) * 1.0)) * (i + 1));
		stY = 0;
		MakeProfileData_V(leadData, profileData_v, sizeX, sizeY, stX, stY, size);

		//fileName.Format(_T("D:\\TestImage\\data\\lead(V)_%d_%d.csv"), index, i);
		//DataSave(profileData_v, size, fileName);
	}
	//delete profileData_v;
	g_pMManager->pem_delete(profileData_v, false);


// 	size = sizeX * 1;
// 	float* profileData_h = NULL;
// 	profileData_h = new float[size];
// 
// 	int hrInterval = 5;
// 	for(int i = 0; i < hrInterval; i++)
// 	{
// 		stX = 0;
// 		stY = (int)((sizeY / ((hrInterval + 1) * 1.0)) * (i + 1));
// 		MakeProfileData_H(leadData, profileData_h, sizeX, sizeY, stX, stY, size);
// 
// 		fileName.Format(_T("D:\\TestImage\\data\\lead(H)_%d_%d.csv"), index, i);
// 		DataSave(profileData_h, size, fileName);
// 	}
// 	delete profileData_h;

	//delete tempBuf;
	g_pMManager->pem_delete(tempBuf, false);
	return ret;
}


BOOL CProc3D::InspLeadArea(float* zmapData, int sizeX, int sizeY, CRect* leadRect, int rectCnts, double angle, BOOL* retResult)
{
	BOOL ret = FALSE;
	
	int rotateSizeX = 0;
	int rotateSizeY = 0;
	//CRect* rt = new CRect[rectCnts];
	CRect* rt = g_pMManager->pem_new<CRect>(true, rectCnts, (PCHAR)__FUNCTION__, __LINE__);
	memset(rt, 0, sizeof(CRect) * rectCnts);
	RotateRect(leadRect, rectCnts, angle, sizeX, sizeY, rt, &rotateSizeX, &rotateSizeY);

	//////////////////////////////////////////////////////////////////////////

	int cx = 0;
	int cy = 0;
	int width = 0;
	int height = 0;

	int minProfileSize = 10000000;
	int profileSize = 0;
	int stX = 0;
	int stY = 0;
	int vtInterval = 1;

	float** leadProfileData = NULL;
	//leadProfileData = new float*[rectCnts * vtInterval];
	leadProfileData = g_pMManager->pem_new<float*>(true, rectCnts * vtInterval, (PCHAR)__FUNCTION__, __LINE__);

	CString fileName = _T("");
	float* leadData = NULL;
	int index = 0;
	for(int i = 0; i < rectCnts; i++)
	{
		if(leadData != NULL)
		{
			//delete leadData;
			g_pMManager->pem_delete(leadData, false);
			leadData = NULL;
		}

		cx = (int)(rt[i].left + (rt[i].Width() / 2.0));
		cy = (int)(rt[i].top + (rt[i].Height() / 2.0));
		width = rt[i].Width();
		height = rt[i].Height();
		//leadData = new float[width * height];
		leadData = g_pMManager->pem_new<float>(true, width * height, (PCHAR)__FUNCTION__, __LINE__);
		GetClipZmapData(zmapData, leadData, rotateSizeX, rotateSizeY, cx, cy, width, height);

		profileSize = 1 * height;
		for(int j = 0; j < vtInterval; j++)
		{
			stX = (int)((width / ((vtInterval + 1) * 1.0)) * (j + 1));
			stY = 0;
			index = (i * vtInterval) + j;
			//leadProfileData[index] = new float[profileSize];
			leadProfileData[index] = g_pMManager->pem_new<float>(true, profileSize, (PCHAR)__FUNCTION__, __LINE__);
			MakeProfileData_V(leadData, leadProfileData[index], width, height, stX, stY, profileSize);

 			//fileName.Format(_T("D:\\TestImage\\data\\lead(V)_%d_%d.csv"), i, j);
 			//DataSave(leadProfileData[index], profileSize, fileName);
		}	

		if(minProfileSize > profileSize)
		{
			minProfileSize = profileSize;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	/*float* leadAvrData = new float[minProfileSize];
	float* x = new float[minProfileSize];*/
	float* leadAvrData = g_pMManager->pem_new<float>(true, minProfileSize, (PCHAR)__FUNCTION__, __LINE__);
	float* x = g_pMManager->pem_new<float>(true, minProfileSize, (PCHAR)__FUNCTION__, __LINE__);

	float sum = 0;
	float avr = 0;
	for(int i = 0; i < minProfileSize; i++)
	{
		sum = 0;
		for(int j = 0; j < rectCnts; j++)
		{
			sum += leadProfileData[j][i];
		}
		
		avr =  sum / rectCnts;
		leadAvrData[i] = avr;

		x[i] = (float)i;
	}

 	//fileName.Format(_T("D:\\TestImage\\data\\leadAVR.csv"));
 	//DataSave(leadAvrData, minProfileSize, fileName);

	//////////////////////////////////////////////////////////////////////////
	float coef[2] = {0,};
	LeastSquareMethod(x, leadAvrData, 1, minProfileSize, coef);
	
	float temp = 0;
	//float* compairData = new float[minProfileSize];
	float* compairData = g_pMManager->pem_new<float>(true, minProfileSize, (PCHAR)__FUNCTION__, __LINE__);
	for(int i = 0; i < minProfileSize; i++)
	{
		temp = (coef[1] * x[i]) + coef[0];
		compairData[i] = leadAvrData[i] - temp;
	}

 	//fileName.Format(_T("D:\\TestImage\\data\\compairData.csv"));
 	//DataSave(compairData, minProfileSize, fileName);

	BOOL flag = FALSE;
	temp = compairData[0];
	if(temp >= 0)
		flag = TRUE;
	else 
		flag = FALSE;
	
	index = 0;
	//int* nodeX = new int[minProfileSize];
	int* nodeX = g_pMManager->pem_new<int>(true, minProfileSize, (PCHAR)__FUNCTION__, __LINE__);
	memset(nodeX, -1, sizeof(int) * minProfileSize);

	for(int i = 0; i < minProfileSize; i++)
	{
		temp = compairData[i];
		if(flag)
		{
			if(temp < 0)
			{
				nodeX[index] = i;
				index++;
				flag = FALSE;
			}
		}
		else
		{
			if(temp >= 0)
			{
				flag = TRUE;
			}
		}
	}

	temp = 0;
	int tempCnts = 0;
	int inspPoint = 0;
	float tempMin = 0;
	float tempMax = 0;
	for(int i = 0; i < index; i++)
	{
		temp = leadAvrData[nodeX[i]];

		for(int j = 0; j < minProfileSize - 1; j++)
		{
			if(leadAvrData[j] >= leadAvrData[j + 1])
			{
				tempMax = leadAvrData[j];
				tempMin = leadAvrData[j + 1];
			}
			else
			{
				tempMax = leadAvrData[j + 1];
				tempMin = leadAvrData[j];
			}

			if(tempMin <= temp && tempMax > temp )
				tempCnts++;			
		}

		if((tempCnts > 0) && (tempCnts < 3))
		{
			inspPoint = nodeX[i];
			break;
		}
		tempCnts = 0;
	}
	//////////////////////////////////////////////////////////////////////////

	tempMax = 0;
	tempMin = 0;
	int inspPixelCnts = (int)(minProfileSize * (20.0 / 100.0)); //lead length -> 20%
	int st = (int)(inspPoint - (inspPixelCnts / 2.0));
	float stdData = leadAvrData[inspPoint];

	//BOOL* rst = new int[rectCnts];
	BOOL* rst = g_pMManager->pem_new<int>(true, rectCnts, (PCHAR)__FUNCTION__, __LINE__);
	memset(rst, 0, sizeof(BOOL) * rectCnts);

	for(int i = 0; i < rectCnts; i++)
	{
		for(int j = 0; j < inspPixelCnts; j++)
		{
			if(leadProfileData[i][st + (j)] >= leadProfileData[i][st + (j + 1)])
			{
				tempMax = leadProfileData[i][st + (j)];
				tempMin = leadProfileData[i][st + (j + 1)];
			}
			else
			{
				tempMax = leadProfileData[i][st + (j + 1)];
				tempMin = leadProfileData[i][st + (j)];
			}

			if((tempMax > stdData && tempMin < stdData) || tempMax == stdData || tempMin == stdData) // ok
			{
				rst[i] = FALSE;
				break;
			}
			else //ng
			{
				rst[i] = TRUE;
			}
		}

		if(rst[i])
			ret = TRUE;
	}

	if(retResult != NULL)
		memcpy_s(retResult, (sizeof(BOOL) * rectCnts), rst, (sizeof(BOOL) * rectCnts));		

	//////////////////////////////////////////////////////////////////////////
	alpa_delete_array_2d(&leadProfileData, (rectCnts * vtInterval));
	/*delete leadAvrData;
	delete leadData;
	delete rt;
	delete x;*/
	g_pMManager->pem_delete(leadAvrData, false);
	g_pMManager->pem_delete(leadData, false);
	g_pMManager->pem_delete(rt, false);
	g_pMManager->pem_delete(x, false);

	/*delete compairData;
	delete nodeX;

	delete rst;	*/	
	g_pMManager->pem_delete(compairData, false);
	g_pMManager->pem_delete(nodeX, false);
	g_pMManager->pem_delete(rst, false);

	return ret;
}

float CProc3D::CalcLeadHeight(float* zmapData, int sizeX, int sizeY, CRect* leadRect, int rectCnts, double angle, float* retHeight)
{
	float ret = -1;

	if(rectCnts < 1)
		return ret;

	int stX = 0;
	int stY = 0;
	int profileSize = 10;

	//float* avrH = new float[rectCnts];
	float* avrH = g_pMManager->pem_new<float>(true, rectCnts, (PCHAR)__FUNCTION__, __LINE__);
	memset(avrH, 0, sizeof(float) * rectCnts);

	double sum = 0;
	double sumCnt = 0;
	double totalSum = 0;
	float* leadProfileData = NULL;
	for(int i = 0; i < rectCnts; i++)
	{
		//leadProfileData = new float[profileSize];
		leadProfileData = g_pMManager->pem_new<float>(true, profileSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(leadProfileData, 0, sizeof(float) * profileSize);

		stX = (int)(leadRect[i].left + (leadRect[i].Width() / 2.0));
		stY = (int)(leadRect[i].bottom - profileSize);

		if(stX > 0 && stY > 0 && profileSize > 0)
		{
			MakeProfileData_V(zmapData, leadProfileData, sizeX, sizeY, stX, stY, profileSize);

			sum = 0;
			for(int j = 0; j < profileSize; j++)
			{
				sum += leadProfileData[j];
			}
			avrH[i] = (float)(sum / profileSize);
			totalSum += avrH[i];
		}
		//delete leadProfileData;
		g_pMManager->pem_delete(leadProfileData, false);
	}

	float totalAvrH = 0;
	totalAvrH = (float)(totalSum / rectCnts);
	ret = totalAvrH;

	if(retHeight  != NULL)
		memcpy_s(retHeight, sizeof(float) * rectCnts, avrH, sizeof(float) * rectCnts);


	//delete avrH;
	g_pMManager->pem_delete(avrH, false);

	return ret;
}


float CProc3D::CalcLeadHeight(float* zmapData, int orgSizeX, int orgSizeY, int clipSizeX, int clipSizeY, CRect* leadRect, int rectCnts, float* retHeight)
{
	float ret = -1;

	if(rectCnts < 1 || orgSizeX <= 0 || orgSizeY <= 0 || clipSizeX <= 0 || clipSizeY <= 0)
		return ret;

	//float* avrH = new float[rectCnts];
	float* avrH = g_pMManager->pem_new<float>(true, rectCnts, (PCHAR)__FUNCTION__, __LINE__);
	memset(avrH, 0, sizeof(float) * rectCnts);

	int cx = 0;
	int cy = 0;
	int w = 0;
	int h = 0;

	double totalSum = 0;
	for(int i = 0; i < rectCnts; i++)
	{
		w = clipSizeX;
		h = clipSizeY;

		if(w > leadRect[i].Width())
			w = leadRect[i].Width();
		if(h > leadRect[i].Height())
			h = leadRect[i].Height();

		cx = (int)(leadRect[i].left + (leadRect[i].Width() / 2.0));
		cy = (int)((leadRect[i].bottom - (h / 2.0)) - 1);

		if((int)(h / 2.0) >= cy)
			cy += (int)(h / 2.0);

		
		avrH[i] = GetFilterHeight(zmapData, orgSizeX, orgSizeY, cx, cy, w, h);

// 		w = leadRect[i].Width()- 4;
// 		h = clipSizeY;
// 		cx = (int)(leadRect[i].left + (leadRect[i].Width() / 2.0));
// 		cy = (int)((leadRect[i].bottom - (h / 2.0)) - 1);
// 		avrH[i] = GetFilterHeight(zmapData, orgSizeX, orgSizeY, cx, cy, w, h);


		totalSum += avrH[i];
	}

	float totalAvrH = 0;
	totalAvrH = (float)(totalSum / rectCnts);
	ret = totalAvrH;

	if(retHeight  != NULL)
		memcpy_s(retHeight, sizeof(float) * rectCnts, avrH, sizeof(float) * rectCnts);


	//delete avrH;
	g_pMManager->pem_delete(avrH, false);

	return ret;
}

//shk 2014/04/10 
float CProc3D::CalcLeadHeight2(float* zmapData, int orgSizeX, int orgSizeY, int clipSizeX, int clipSizeY, CRect* leadRect, int rectCnts, float* retHeight)
{
	float ret = -1;

	if(rectCnts < 1 || orgSizeX <= 0 || orgSizeY <= 0 || clipSizeX <= 0 || clipSizeY <= 0)
		return ret;

	//float* avrH = new float[rectCnts];
	float* avrH = g_pMManager->pem_new<float>(true, rectCnts, (PCHAR)__FUNCTION__, __LINE__);
	memset(avrH, 0, sizeof(float) * rectCnts);

	int cx = 0;
	int cy = 0;
	int w = 0;
	int h = 0;

	double totalSum = 0;
	for(int i = 0; i < rectCnts; i++)
	{
		w = leadRect[i].Width();
		h = LEAD_LIFT_MARGIN;

		if(w > leadRect[i].Width())
			w = leadRect[i].Width();
		if(h > leadRect[i].Height())
			h = leadRect[i].Height();

		cx = (int)(leadRect[i].left + (leadRect[i].Width() / 2.0));
		cy = (int)(leadRect[i].bottom - (LEAD_LIFT_MARGIN/2.0));

		avrH[i] = GetFilterHeight2(zmapData, orgSizeX, orgSizeY, cx, cy, (int)(w/2), h);


		totalSum += avrH[i];
	}

	float totalAvrH = 0;
	totalAvrH = (float)(totalSum / rectCnts);
	ret = totalAvrH;

	if(retHeight  != NULL)
		memcpy_s(retHeight, sizeof(float) * rectCnts, avrH, sizeof(float) * rectCnts);


	//delete avrH;
	g_pMManager->pem_delete(avrH, false);

	return ret;
}

void CProc3D::RotateRect(CRect* data, int cnts, double angle, int orgSizeX, int orgSizeY, CRect* retResult, int* retWidth, int* retHeight)
{
	if(cnts < 1)
		return ;

	CRect* tempRect = NULL;
	//tempRect = new CRect[cnts];
	tempRect = g_pMManager->pem_new<CRect>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);

	for(int i = 0; i < cnts; i++)
	{
		if(angle == 90)
		{
			tempRect[i].left = data[i].top;
			tempRect[i].right = tempRect[i].left + data[i].Height();
			tempRect[i].top = orgSizeX - data[i].Width();
			tempRect[i].bottom = orgSizeX;

			*retWidth = orgSizeY;
			*retHeight = orgSizeX;
		}
		else if(angle == 180)
		{
			tempRect[i].left = orgSizeX - data[i].right;
			tempRect[i].right = tempRect[i].left + data[i].Width();
			tempRect[i].top = orgSizeY - data[i].Height();
			tempRect[i].bottom = orgSizeY;

			*retWidth = orgSizeX;
			*retHeight = orgSizeY;
		}
		else if(angle == 270)
		{
			tempRect[i].left = orgSizeY - data[i].bottom;
			tempRect[i].right = tempRect[i].left + data[i].Height();
			tempRect[i].top = data[i].left;
			tempRect[i].bottom = orgSizeX - 1;

			*retWidth = orgSizeY;
			*retHeight = orgSizeX;
		}
		else if(angle == 0)
		{
			tempRect[i].left = data[i].left;
			tempRect[i].right = data[i].right;
			tempRect[i].top = data[i].top;
			tempRect[i].bottom = data[i].bottom;

			*retWidth = orgSizeX;
			*retHeight = orgSizeY;
		}
	}
	

	memcpy_s(retResult, (sizeof(CRect) * cnts) , tempRect, (sizeof(CRect) * cnts));

	//delete tempRect;
	g_pMManager->pem_delete(tempRect, false);
}


void CProc3D::RemakeZmap(float* srcZmap, float* dstZmap, int sizeX, int sizeY, int minRange, int maxRange, float minData)
{
	float* zmapTemp = NULL;
	//zmapTemp = new float[sizeX * sizeY];
	zmapTemp = g_pMManager->pem_new<float>(true, sizeX * sizeY, (PCHAR)__FUNCTION__, __LINE__);
	memset(zmapTemp, 0, sizeof(float) * sizeX * sizeY);

	int index = 0;
	float data = 0;
	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			index = (sizeX * y) + x;
			data = srcZmap[index];

			if(data >= minRange && data <= maxRange)
			{
				zmapTemp[index] = data;
			}
			else
			{
				zmapTemp[index] = (float)minRange;
			}
		}
	}

	memcpy_s(dstZmap, sizeof(float) * sizeX * sizeY, zmapTemp, sizeof(float) * sizeX * sizeY);

	//delete zmapTemp;
	g_pMManager->pem_delete(zmapTemp, false);
}

int CProc3D::RemakeSolderRect(float* pZmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, CRect** retResult)
{
	int area = roiSizeX * roiSizeY;
	int margin = 10;

	CString fileName = _T("");
	int size = 0;
	int index = 0;
	int solderMaxIndex = 0;

	int sizeMax = (int)solderRect[0].Height();
	for(int i = 0; i < solderCnt; i++)
	{
		if(solderRect[i].Height() > sizeMax)
		{
			sizeMax = (int)solderRect[i].Height();
			solderMaxIndex = i;
		}

		if(solderRect[i].Width() <=  0 || solderRect[i].Height() <= 0 || solderRect[i].left < 0 || solderRect[i].right < 0 || solderRect[i].top < 0 || solderRect[i].bottom < 0)
		{
			return 0;
		}
	}

	if((roiSizeY - sizeMax) <= margin)
		margin = (roiSizeY - sizeMax) - 1;

	size = sizeMax + margin;

	if(sizeMax < 0 || size < 0 && size > roiSizeY)
	{
		return 0;
	}

	CRect* rectTemp = NULL;
	//rectTemp = new CRect[solderCnt];
	rectTemp = g_pMManager->pem_new<CRect>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(rectTemp, 0, sizeof(CRect) * solderCnt);

// 	float** profileData = NULL;
// 	profileData = new float*[solderCnt];
	float* profileData = NULL;

	
	int stX = 0;
	int stY = 0;
	float data1 = 0;
	float data2 = 0;
	float sub = 0;
	double sumTemp = 0;
	int sumCnt = 0;

	double sumTemp_bottom = 0;
	int sumCnt_bottom = 0;
	
	for(int i = 0; i < solderCnt; i++)
	{
		//profileData = new float[size];
		profileData = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);
		memset(profileData, 0, sizeof(float) * size);

		stX = (int)(solderRect[i].left + (int)(solderRect[i].Width() / 2.0));
		stY = (int)solderRect[i].top - margin;
		MakeProfileData_V(pZmapData, profileData, roiSizeX, roiSizeY, stX, stY, size);



		float minData = 100000;
		int startPt = 0;
		for(int j = 0; j < size; j++)
		{
			if(j == (size - 1))
			{
				break;
			}

			data1 = profileData[j];
			data2 = profileData[j+1];
			sub = data2-data1;

			if(minData > sub)
			{
				minData = sub;
				startPt = j;
			}
		}


		rectTemp[i].top = -1; //시작점을 못찾았을 경우 -1... (시작점을 못찾았을경우 다른 찾은것들의 평균값을 넣어준다..)
		//솔더 시작점
		
		BOOL startFlag = FALSE;
		for(int j = startPt; j > 0; j--) 
		{
			data1 = profileData[j-1];
			data2 = profileData[j];					
			sub = data2-data1;

			if(abs(sub) < 10)
			{
				rectTemp[i].top = stY + j;
				sumTemp += (double)rectTemp[i].top; //평균 구하기 위함..
				sumCnt++;
				startFlag = TRUE;
				break;
			}
		}

		if(startFlag == FALSE)
		{
			for(int j = startPt; j < size -1; j++) 
			{
				data1 = profileData[j];
				data2 = profileData[j+1];					
				sub = data2-data1;

				if(abs(sub) < 10)
				{
					rectTemp[i].top = stY + (j + 1);
					sumTemp += (double)rectTemp[i].top; //평균 구하기 위함..
					sumCnt++;
					startFlag = TRUE;
					break;
				}
			}
		}

		//솔더 끝점
		float tempMax = 0;
		int startPt2 = 0;
		BOOL flag_1st = TRUE;
		BOOL flag_2st = FALSE;
		for(int j = startPt; j < size -1; j++) //1차
		{
			data1 = profileData[j];
			if(data1 < 5)
			{
				rectTemp[i].bottom = stY + j;
				sumTemp_bottom += rectTemp[i].bottom;
				sumCnt_bottom++;
				break;
			}
			else
			{
				rectTemp[i].bottom = -1;
			}
			/*data1 = profileData[j];
			data2 = profileData[j+1];					
			sub = data2-data1;

			if(flag_1st == TRUE && abs(sub) < 5)
			{
				rectTemp[i].bottom = stY + ((j == 0)? 0 : (j + 1));
				flag_1st = FALSE;
			}

			if(flag_1st == FALSE && abs(sub) > 10)  //2차 시작점 검색..  (1차 이후 10보다 큰부분이 있으면, 그중 제일 큰값이 있는 점이 2차 시작점)
			{
				if(tempMax < abs(sub))
				{
					tempMax = abs(sub);
					startPt2 = j + 1;
					flag_2st = TRUE;
				}
			}*/
		}

		/*if(flag_2st == TRUE)
		{
			for(int j = startPt2; j < size - 1; j++) //2차
			{
				data1 = profileData[j];
				data2 = profileData[j+1];					
				sub = data2-data1;

				if(abs(sub) < 5)
				{
					rectTemp[i].bottom = stY + ((j == 0)? 0 : (j + 1));
					break;
				}
			}
		}*/


		rectTemp[i].left = solderRect[i].left;
		rectTemp[i].right = solderRect[i].right;
 
// 		fileName.Format(_T("D:\\TestImage\\data\\data_%d.csv"),i);
// 		DataSave(profileData, size, fileName);

		index++;


		//delete profileData;
		g_pMManager->pem_delete(profileData, false);
		profileData = NULL;
			
	}

	if(sumCnt != 0)
	{
		//////////////////////////////////////////////////////////////////////////
		//솔더 시작점을 못찾은 부분은 평균값을 넣어준다..(구분은 -1이면 못찾은 것..)
		LONG avrTemp = 0;
		avrTemp = (LONG)(sumTemp / sumCnt);
		if(avrTemp < 0)
			avrTemp = solderRect[solderMaxIndex].top;

		LONG avrTemp_bottom = 0;
		avrTemp_bottom = (LONG)(sumTemp_bottom / sumCnt_bottom);
		if(avrTemp_bottom < 0)
			avrTemp_bottom = solderRect[solderMaxIndex].bottom;
		
		for(int i = 0; i < solderCnt; i++) 
		{
			if(rectTemp[i].top == -1)
				rectTemp[i].top = avrTemp;

			if(rectTemp[i].bottom < 1) 
				rectTemp[i].bottom = avrTemp_bottom;
		}
		//////////////////////////////////////////////////////////////////////////


		//*retResult = new CRect[solderCnt];
		*retResult = g_pMManager->pem_new<CRect>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
		//memset(*retResult, 0, sizeof(CRect) * solderCnt);
		memcpy_s(*retResult, sizeof(CRect) * solderCnt, rectTemp, sizeof(CRect) * solderCnt);
	}
	else
	{
		index = 0;
	}

	//delete rectTemp;
	g_pMManager->pem_delete(rectTemp, false);

	return index;
}

int CProc3D::RemakeSolderRect_Chip(float* pZmapData, int roiSizeX, int roiSizeY, CRect solderRect, CRect* retResult)
{
	int ret = TRUE;

	int area = roiSizeX * roiSizeY;

	CString fileName = _T("");
	int size = (int)solderRect.Height();
	
	if(size < 0 && size > roiSizeY)
	{
		return FALSE;
	}

	CRect rectTemp;

	float* profileData = NULL;

	int stX = 0;
	int stY = 0;
	float data1 = 0;
	float data2 = 0;
	float sub = 0;

	//profileData = new float[size];
	profileData = g_pMManager->pem_new<float>(true, size, (PCHAR)__FUNCTION__, __LINE__);
	memset(profileData, 0, sizeof(float) * size);

	stX = (int)(solderRect.left + (int)(solderRect.Width() / 2.0));
	stY = (int)solderRect.top;
	MakeProfileData_V(pZmapData, profileData, roiSizeX, roiSizeY, stX, stY, size);



	float minData = 100000;
	int startPt = 0;
	for(int j = 0; j < size; j++)
	{
		if(j == (size - 1))
		{
			break;
		}

		data1 = profileData[j];
		data2 = profileData[j+1];
		sub = data2-data1;

		if(minData > sub)
		{
			minData = sub;
			startPt = j;
		}
	}


	rectTemp.top = -1; //시작점을 못찾았을 경우 -1... (시작점을 못찾았을경우 다른 찾은것들의 평균값을 넣어준다..)
	//솔더 시작점

	BOOL startFlag = FALSE;
	for(int j = startPt; j > 0; j--) 
	{
		data1 = profileData[j-1];
		data2 = profileData[j];					
		sub = data2-data1;

		if(abs(sub) < 10)
		{
			rectTemp.top = stY + j;
			startFlag = TRUE;
			break;
		}
	}

	if(startFlag == FALSE)
	{
		for(int j = startPt; j < size -1; j++) 
		{
			data1 = profileData[j];
			data2 = profileData[j+1];					
			sub = data2-data1;

			if(abs(sub) < 10)
			{
				rectTemp.top = stY + (j + 1);
				startFlag = TRUE;
				break;
			}
		}
	}

	//솔더 끝점
	rectTemp.bottom = -1;

	float tempMax = 0;
	int startPt2 = 0;
	BOOL flag_1st = TRUE;
	BOOL flag_2st = FALSE;
	for(int j = startPt; j < size -1; j++) //1차
	{
		data1 = profileData[j];
		if(data1 < 5)
		{
			rectTemp.bottom = stY + j;
			break;
		}
	/*	data1 = profileData[j];
		data2 = profileData[j+1];					
		sub = data2-data1;

		if(flag_1st == TRUE && abs(sub) < 5)
		{
			rectTemp.bottom = stY + ((j == 0)? 0 : (j + 1));
			flag_1st = FALSE;
		}

		if(flag_1st == FALSE && abs(sub) > 10)  //2차 시작점 검색..  (1차 이후 10보다 큰부분이 있으면, 그중 제일 큰값이 있는 점이 2차 시작점)
		{
			if(tempMax < abs(sub))
			{
				tempMax = abs(sub);
				startPt2 = j + 1;
				flag_2st = TRUE;
			}
		}*/
	}

	/*if(flag_2st == TRUE)
	{
		for(int j = startPt2; j < size - 1; j++) //2차
		{
			data1 = profileData[j];
			data2 = profileData[j+1];					
			sub = data2-data1;

			if(abs(sub) < 5)
			{
				rectTemp.bottom = stY + ((j == 0)? 0 : (j + 1));
				break;
			}
		}
	}*/


	rectTemp.left = solderRect.left;
	rectTemp.right = solderRect.right;

	//fileName.Format(_T("D:\\TestImage\\data\\data_Chip.csv"));
	//DataSave(profileData, size, fileName);



//////////////////////////////////////////////////////////////////////////


		if(rectTemp.top < 0 || rectTemp.bottom < 1)
			ret = FALSE;
	
		//////////////////////////////////////////////////////////////////////////
		*retResult = rectTemp;

	//delete profileData;
	g_pMManager->pem_delete(profileData, false);

	return ret;
}

int CProc3D::CalcAreaVolume(float* pZmapData, int roiSizeX, int roiSizeY, CRect* solderRect, int solderCnt, double* retAreaVolume, float* retHlimit)
{
	int ret = 0;

	if(pZmapData == NULL || roiSizeX <= 0 || roiSizeY <= 0 || solderRect == NULL || solderCnt == 0)
		return 0;

	//float* hAvr = new float[solderCnt];   //¼O´o ½AAU¶oAIAC Æo±O ³oAI...(ºICC±¸CO¶§ ³oAI·I ¾²AO, AIAI≫oAC ³oAI´A ¹≪½ACI°i AI°ªA¸·I ´eA¼)
	float* hAvr = g_pMManager->pem_new<float>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(hAvr, 0, sizeof(float) * solderCnt);

	//double* areaVol = new double[solderCnt]; 
	double* areaVol = g_pMManager->pem_new<double>(true, solderCnt, (PCHAR)__FUNCTION__, __LINE__);
	memset(areaVol, 0, sizeof(double) * solderCnt);

	int stX = 0;
	int stY = 0; 
	double sum = 0;
	int profileSize = 0;
	float* profile = NULL; //½AAU¶oAI CA·IÆAAI.. (±æAI´U ´U ´U¸§.. ±×¶§ ±×¶§ ±æAI¸¸A­ ≫y¼º)
	for(int i = 0; i < solderCnt; i++)
	{
		profileSize = solderRect[i].Width();
		//profile = new float[profileSize];
		profile = g_pMManager->pem_new<float>(true, profileSize, (PCHAR)__FUNCTION__, __LINE__);
		memset(profile, 0, sizeof(float) * profileSize);

		stX = solderRect[i].left;
		stY = solderRect[i].top;
		MakeProfileData_H(pZmapData, profile, roiSizeX, roiSizeY, stX, stY, profileSize);

		sum = 0;
		for(int j = 0; j < profileSize; j++)
		{
			sum += profile[j];
		}
		hAvr[i] = (float)(sum / profileSize);
		areaVol[i] = (double)((solderRect[i].Width() * solderRect[i].Height()) * hAvr[i]);

		//delete profile;
		g_pMManager->pem_delete(profile, false);
	}

	//////////////////////////////////////////////////////////////////////////

	double area = 0;
	if(retAreaVolume != NULL)
	{
		memcpy_s(retAreaVolume, sizeof(double) * solderCnt, areaVol, sizeof(double) * solderCnt);
	}


	if(retHlimit != NULL)
	{
		memcpy_s(retHlimit, sizeof(float) * solderCnt, hAvr, sizeof(float) * solderCnt);
	}


	/*delete hAvr;
	delete areaVol;*/
	g_pMManager->pem_delete(hAvr, false);
	g_pMManager->pem_delete(areaVol, false);

	return ret;
}



void CProc3D::MakeZmap2BinImg(float* zmap, int sizeX, int sizeY, float threshold, UCHAR* dstImg)
{
	int index = 0;
	float data = 0;
	for (int y=0 ; y < sizeY ; y++)
	{
		for (int x=0 ; x < sizeX ; x++)
		{	
			index = (y * sizeX) + x;
			data = zmap[index];

			if(data >= threshold)
				dstImg[index] = 255;
			else
				dstImg[index] = 0;
		}	
	}
}

BOOL CProc3D::MakeZmap2BinImg2(float* zmap, int sizeX, int sizeY, float threshold, UCHAR* dstImg)
{
	BOOL ret = FALSE;

	int index = 0;
	float data = 0;
	int count = 0;
	for (int y=0 ; y < sizeY ; y++)
	{
		for (int x=0 ; x < sizeX ; x++)
		{	
			index = (y * sizeX) + x;
			data = zmap[index];

			if(data >= threshold)
			{
				dstImg[index] = 255;
				count++;
			}
			else
			{
				dstImg[index] = 0;
			}
		}	
	}

	if(count > 10)
		ret = TRUE;

	return ret;
}

float CProc3D::GetBinImage(float* roiZmap, int roiSizeX, int roiSizeY, float Threshold, UCHAR* BinImg)
{
	if(roiZmap == NULL || roiSizeX < 0 || roiSizeY < 0)
		return -1;

	int area = roiSizeX * roiSizeY;

	IppStatus sts;

	//float* proiZmap = new float[area];
	float* proiZmap = g_pMManager->pem_new<float>(true, area, (PCHAR)__FUNCTION__, __LINE__);

	sts = ippsThreshold_LTValGTVal_32f(roiZmap, proiZmap, area, Threshold, 0, Threshold, 255);
	sts = ippsConvert_32f8u_Sfs(proiZmap,BinImg,area,ippRndNear,0);
	//delete proiZmap;
	g_pMManager->pem_delete(proiZmap, false);

	return (float)sts;
}

float CProc3D::GetBinImage2(float* roiZmap, int roiSizeX, int roiSizeY, float Threshold_Low, float Threshold_Heigh, UCHAR* BinImg)
{
	if(roiZmap == NULL || roiSizeX < 0 || roiSizeY < 0)
		return -1;

	int area = roiSizeX * roiSizeY;

	IppStatus sts;

	//float* proiZmap = new float[area];
	float* proiZmap = g_pMManager->pem_new<float>(true, area, (PCHAR)__FUNCTION__, __LINE__);


	sts = ippsThreshold_GTVal_32f(roiZmap, proiZmap, area, Threshold_Heigh, 0);
	sts = ippsThreshold_LTValGTVal_32f(proiZmap, proiZmap, area, Threshold_Low, 0, Threshold_Low, 255);
	sts = ippsConvert_32f8u_Sfs(proiZmap,BinImg,area,ippRndNear,0);

	//delete proiZmap;
	g_pMManager->pem_delete(proiZmap, false);

	return (float)sts;
}

//SHK 2013/1107
float CProc3D::GetCropZmap(float* src, float* dst, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY)
{
	if(src == NULL || roiSizeX < 0 || roiSizeY < 0)
		return -1;

	if(roiSizeX > orgSizeX)
		roiSizeX = orgSizeX;
	if(roiSizeY > orgSizeY)
		roiSizeY = orgSizeY;

	//SHK 2013/11/15 offset 예외처리
	int stX = 0;
	int stY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	if(stX < 0)
		stX = 0;

	if(stY < 0)
		stY = 0;

	if(stX + width >= orgSizeX)
		stX = orgSizeX - width;

	if(stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	sts = ippiCopy_32f_C1R(src + offX + offY * orgSizeX, orgSizeX * 4, dst, width * 4, iSize);

	return (float)sts;
}

// LMJ 2014/01/16
float * CProc3D::GetCropZmap(float* src, int orgSizeX, int orgSizeY, int cx, int cy, int roiSizeX, int roiSizeY)
{
	if(src == NULL || roiSizeX < 0 || roiSizeY < 0)
		return NULL;

	if(roiSizeX > orgSizeX)
		roiSizeX = orgSizeX;
	if(roiSizeY > orgSizeY)
		roiSizeY = orgSizeY;

	//float * dst = new float [roiSizeX * roiSizeY];
	float * dst = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	//SHK 2013/11/15 offset ¿¹¿UA³¸®
	int stX = 0;
	int stY = 0;
	int width = roiSizeX;
	int height = roiSizeY;

	stX = cx - (width / 2);  // start point x
	stY = cy - (height / 2);  // start point Y

	if(stX < 0)
		stX = 0;

	if(stY < 0)
		stY = 0;

	if(stX + width >= orgSizeX)
		stX = orgSizeX - width;

	if(stY + height > orgSizeY)
		stY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = stX;
	int offY = stY;

	sts = ippiCopy_32f_C1R(src + offX + offY * orgSizeX, orgSizeX * 4, dst, width * 4, iSize);

	return dst;
}

float * CProc3D::GetCropZmap_LT(float* src, int orgSizeX, int orgSizeY, int nStX, int nStY, int roiSizeX, int roiSizeY)
{
	if(src == NULL || roiSizeX < 0 || roiSizeY < 0)
		return NULL;

	if(roiSizeX > orgSizeX)
		roiSizeX = orgSizeX;
	if(roiSizeY > orgSizeY)
		roiSizeY = orgSizeY;

	//float * dst = new float [roiSizeX * roiSizeY];
	float * dst = g_pMManager->pem_new<float>(true, roiSizeX * roiSizeY, (PCHAR)__FUNCTION__, __LINE__);

	//SHK 2013/11/15 offset ¿¹¿UA³¸®
	int width = roiSizeX;
	int height = roiSizeY;

	if(nStX < 0)
		nStX = 0;

	if(nStY < 0)
		nStY = 0;

	if(nStX + width >= orgSizeX)
		nStX = orgSizeX - width;

	if(nStY + height > orgSizeY)
		nStY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = nStX;
	int offY = nStY;

	sts = ippiCopy_32f_C1R(src + offX + offY * orgSizeX, orgSizeX * 4, dst, width * 4, iSize);

	return dst;
}

//SHW 2014/11/03
float CProc3D::GetCropZmap_LT(float* src, float* dst, int orgSizeX, int orgSizeY, int nStX, int nStY, int roiSizeX, int roiSizeY)
{
	if (src == NULL || dst == NULL)
		return -1;
	if (orgSizeX <= 0 || orgSizeY <= 0 || roiSizeX <= 0 || roiSizeY <= 0)
		return -1;

	int stX = nStX;  // start point x
	int stY = nStY;  // start point Y
	if (stX < 0) stX = 0;
	if (stY < 0) stY = 0;
	if (stX >= orgSizeX || stY >= orgSizeY)
		return -1;

	if (roiSizeX > orgSizeX || roiSizeY > orgSizeY ||
		stX + roiSizeX > orgSizeX || stY + roiSizeY > orgSizeY)
	{
		int nETX = stX + roiSizeX;
		int nETY = stY + roiSizeY;
		if (nETX < 0) nETX = 0;
		if (nETY < 0) nETY = 0;
		if (nETX > orgSizeX) nETX = orgSizeX;
		if (nETY > orgSizeY) nETY = orgSizeY;
		for (int y = stY; y < nETY; y++)
		{
			if (y < 0) continue;
			if (y >= orgSizeY) break;
			for (int x = stX; x < nETX; x++)
			{
				if (x < 0) continue;
				if (x >= orgSizeX) break;
				int nIndex = (y * orgSizeX) + x;

				int nX = x - stX;
				int nY = y - stY;
				if (nX < 0 || nX >= roiSizeX)
					continue;
				if (nY < 0 || nY >= roiSizeY)
					continue;
				int nIndex2 = (nY * roiSizeX) + nX;
				dst[nIndex2] = src[nIndex];
			}
		}
		return 0;
	}

	IppiSize iSize = { roiSizeX, roiSizeY };
	IppStatus sts = ippiCopy_32f_C1R(src + stX + stY * orgSizeX, orgSizeX * 4, dst, roiSizeX * 4, iSize);

	return (float)sts;
}


float CProc3D::GetCropZmap_LT(float* src, float* dst, int orgSizeX, int orgSizeY, int nStX, int nStY, int roiSizeX, int roiSizeY, double dAngle)
{
	if(src == NULL || roiSizeX < 0 || roiSizeY < 0)
		return -1;


	if(dAngle == 0)
		return GetCropZmap_LT( src,  dst,  orgSizeX,  orgSizeY,  nStX,  nStY,  roiSizeX,  roiSizeY);

	//float* ptrSrc = new float[orgSizeX*orgSizeY];
	float* ptrSrc = g_pMManager->pem_new<float>(true, orgSizeX*orgSizeY, (PCHAR)__FUNCTION__, __LINE__);

	for(int r=0;r<orgSizeY;r++)
	{
		float* srcPtr = &src[r*orgSizeX];
		float* dstPtr = &ptrSrc[r*orgSizeX];
		memcpy(dstPtr,srcPtr,sizeof(float)*orgSizeX);
	}

	float* ptrHeight_ro = NULL;
	int retDstSizeX(0), retDstSizeY(0);
	int GapX = 0;
	int GapY = 0;

	if(ptrSrc != NULL)
	{
		RotateZmap_ipp2(ptrSrc, &ptrHeight_ro, orgSizeX, orgSizeY, -dAngle, &retDstSizeX, &retDstSizeY);

		GapX = RounD((retDstSizeX - orgSizeX)/2);
		GapY = RounD((retDstSizeY - orgSizeY)/2);
		if((GapX+orgSizeX)>retDstSizeX)
			GapX = 0;
		if((GapY+orgSizeY)>retDstSizeY)
			GapY = 0;
		for(int r=0;r<orgSizeY;r++)
		{
			float* srcPtr = &ptrHeight_ro[(r+GapY)*retDstSizeX+GapX];
			float* dstPtr = &ptrSrc[r*orgSizeX];
			memcpy(dstPtr,srcPtr,sizeof(float)*orgSizeX);
		}
	}
	if(ptrHeight_ro != NULL)
		//delete [] ptrHeight_ro;
		g_pMManager->pem_delete(ptrHeight_ro, true);

	if(roiSizeX > orgSizeX)
		roiSizeX = orgSizeX;
	if(roiSizeY > orgSizeY)
		roiSizeY = orgSizeY;

	int width = roiSizeX;
	int height = roiSizeY;

	if(nStX < 0)
		nStX = 0;

	if(nStY < 0)
		nStY = 0;

	if(nStX + width >= orgSizeX)
		nStX = orgSizeX - width;

	if(nStY + height > orgSizeY)
		nStY = orgSizeY - height;

	IppStatus sts;

	IppiSize iSize = {width, height};

	int offX = nStX;
	int offY = nStY;

	sts = ippiCopy_32f_C1R(ptrSrc + offX + offY * orgSizeX, orgSizeX * 4, dst, width * 4, iSize);

	//delete [] ptrSrc;
	g_pMManager->pem_delete(ptrSrc, true);

	return (float)sts;
}


//SHK 2013/11/11 
//************************************
// FullName:  CProc3D::ExtractBodyCv
// BodyBlobCN °a°uμ￥AIAI ¹YE?CI´A CO¼o
// Aß½E AAC￥´A ÆAÆ®±aAØA¸·I CN AAC￥.
//************************************
double CProc3D::ExtractBodyCv(UCHAR* blobImg, double cx, double cy, int roiSizeX, int roiSizeY, CRect blobRt, double* retCogX, double* retCogY, double* retBodyWidth, double* retBodyHeight, CRect* retRect, POINT* retNodalPoint)
{
	double avrAngle = 0;

	cv::Mat blobI = cv::Mat(roiSizeY,roiSizeX,CV_8UC1,blobImg);

// 	Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(5, 3));
// 	erode(blobI, blobI, element);

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours( blobI, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

	std::vector<cv::RotatedRect> minRect( contours.size() );

	cv::Point2f rect_points[4];

	if ( contours.size() != 1)
	{
		for( int i = 0; i < contours.size(); i++ )
		{
			minRect[i] = cv::minAreaRect(cv::Mat(contours[contours.size()-1]) );
			minRect[i].points( rect_points );
		}
	}
	else
	{
		for( int i = 0; i < contours.size(); i++ )
		{
			minRect[i] = cv::minAreaRect(cv::Mat(contours[i]) );
			minRect[i].points( rect_points );
		}
	}

	float fCenMinRectX = 0.0;
	float fCenMinRectY = 0.0;
	if(contours.size() > 0)
	{
		fCenMinRectX = minRect[0].center.x;
		fCenMinRectY = minRect[0].center.y;
	}

	POINT pt[4];

#if 0
	for (int i=0;i<4;i++)
	{
		float difX = rect_points[i].x - minRect[0].center.x;
		float difY = rect_points[i].y - minRect[0].center.y;
		
		if(difX < 0 && difY < 0)
		{
			pt[0].x = (LONG)rect_points[i].x;//left_top
			pt[0].y = (LONG)rect_points[i].y;
		}
		else if(difX < 0 && difY > 0) 
		{
			pt[1].x = (LONG)rect_points[i].x;//left_bottom
			pt[1].y = (LONG)rect_points[i].y;
		}
		else if(difX > 0 && difY > 0)
		{
			pt[2].x = (LONG)rect_points[i].x;//right_bottom
			pt[2].y = (LONG)rect_points[i].y;
		}
		else if(difX > 0 && difY < 0)
		{
			pt[3].x = (LONG)rect_points[i].x;//right_top
			pt[3].y = (LONG)rect_points[i].y;
		}
	}
#endif
	LONG topTemp = 0L;
	cv::Point2f top_points = ((float)0,(float)0);
	int topCnt = 0;
	LONG botTemp = 0L;
	cv::Point2f bot_points = ((float)0,(float)0);
	int botCnt = 0;

	for(int i=0;i<4;i++)
	{
		if(rect_points[i].x > roiSizeX)
			rect_points[i].x = roiSizeX;
		if(rect_points[i].y > roiSizeY)
			rect_points[i].y = roiSizeY;
		if(rect_points[i].x < 0)
			rect_points[i].x = 0;
		if(rect_points[i].y < 0)
			rect_points[i].y = 0;

		float difX = rect_points[i].x - fCenMinRectX;
		float difY = rect_points[i].y - fCenMinRectY;

		//Top
		if(difY < 0 )
		{
			topCnt++;
			switch (topCnt)
			{
			case 1:
				topTemp = (LONG)rect_points[i].x; //값이 없으면
				top_points = rect_points[i];
				break;
			case 2:
				{
					if (topTemp < (LONG)rect_points[i].x)
					{
						pt[3].x = (LONG)rect_points[i].x;//right_top
						pt[3].y = (LONG)rect_points[i].y;

						pt[0].x = (LONG)top_points.x; //left_top
						pt[0].y = (LONG)top_points.y;
					}
					else
					{
						pt[0].x = (LONG)rect_points[i].x;//left_top
						pt[0].y = (LONG)rect_points[i].y;

						pt[3].x = (LONG)top_points.x; //right_top
						pt[3].y = (LONG)top_points.y;
					}
				}
				break;
			}
		}
		else
		{
			botCnt++;
			switch (botCnt)
			{
			case 1:
				botTemp = (LONG)rect_points[i].x; //값이 없으면
				bot_points = rect_points[i];
				break;
			case 2:
				{
					if (botTemp < (LONG)rect_points[i].x)
					{
						pt[2].x = (LONG)rect_points[i].x;//right_top
						pt[2].y = (LONG)rect_points[i].y;

						pt[1].x = (LONG)bot_points.x;
						pt[1].y = (LONG)bot_points.y;
					}
					else
					{
						pt[1].x = (LONG)rect_points[i].x;//left_top
						pt[1].y = (LONG)rect_points[i].y;

						pt[2].x = (LONG)bot_points.x;
						pt[2].y = (LONG)bot_points.y;
					}
				}
				break;
			}
		}
	}

	if(retNodalPoint != NULL)
	{
		memcpy_s(retNodalPoint, sizeof(POINT) * 4, pt, sizeof(POINT) * 4);
	}


	//*retBodyWidth = minRect[0].size.width;
	//*retBodyHeight = minRect[0].size.height;

	//////////////////////////////////////////////////////////////////////////
	double bodyW_T = 0;
	double bodyW_B = 0;
	double bodyH_L = 0;
	double bodyH_R = 0;

	LONG tempW = abs(pt[3].x - pt[0].x);
	LONG tempH = abs(pt[0].y - pt[3].y);
	bodyW_T = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[2].x - pt[1].x);
	tempH = abs(pt[1].y - pt[2].y);
	bodyW_B = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[0].x - pt[1].x);
	tempH = abs(pt[1].y - pt[0].y);
	bodyH_L = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[2].x - pt[3].x);
	tempH = abs(pt[2].y - pt[3].y);
	bodyH_R = sqrt((double)(tempW * tempW + tempH * tempH));


	double bodyWidth = (bodyW_T + bodyW_B) / 2.0;
	double bodyHeight = (bodyH_L + bodyH_R) / 2.0;

	CRect bodyRect;
	LONG nLeft = (LONG)((pt[0].x <= pt[1].x)?		pt[0].x : pt[1].x);
	LONG nRight = (LONG)((pt[2].x <= pt[3].x)?		pt[3].x : pt[2].x);
	LONG nTop = (LONG)((pt[0].y <= pt[3].y)?		pt[0].y : pt[3].y);
	LONG nBottom = (LONG)((pt[1].y <= pt[2].y)?		pt[2].y : pt[1].y);


// 	if((bodyWidth < 1) || (bodyHeight < 1) || (nLeft < 0) || (nRight < 0) || (bodyWidth > roiSizeX) || (bodyHeight > roiSizeY))
// 		return avrAngle;
	if((bodyWidth < 1) || (bodyHeight < 1))
		return avrAngle;
	if((nLeft < 0) || (nRight < 0) || (nTop < 0) || (nBottom < 0))
		return avrAngle;
	if((nLeft > roiSizeX) || (nRight > roiSizeX) || (nTop > roiSizeY) || (nBottom > roiSizeY))
		return avrAngle;
	if((bodyWidth > roiSizeX) || (bodyHeight > roiSizeY))
		return avrAngle;


	bodyRect.left = nLeft;
	bodyRect.right = nRight;
	bodyRect.top = nTop;
	bodyRect.bottom = nBottom;

	*retBodyWidth = bodyWidth;
	*retBodyHeight = bodyHeight;
	//////////////////////////////////////////////////////////////////////////

	//avrAngle = GetAngle(pt[3].x,pt[3].y,pt[2].x,pt[2].y);//minRect[0].angle;
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	double cogX = (cx - ((double)roiSizeX / 2.0)) + fCenMinRectX;
	double cogY = (cy - ((double)roiSizeY / 2.0)) + fCenMinRectY;

	*retCogX = cogX;
	*retCogY = cogY;
	*retRect = bodyRect;

	return avrAngle;
}

// SHW 20141114
double CProc3D::ExtractBodyCv(UCHAR* blobImg, double stx, double sty, int roiSizeX, int roiSizeY, CRect blobRt, double* retCogX, double* retCogY, double* retBodyWidth, double* retBodyHeight, CRect* retRect, POINT* retNodalPoint, int nOkPercent, bool bSTD, bool bUnitFind, bool bUNIT_W, int nTeachX, int nTeachY, double* retBodyA)
{
	POINT pt[4];
	for (int i = 0; i < 4; i++)
	{
		pt[i].x = pt[i].y = -1;
	}
	double dWhiteValue = 0.;
	double dWhiteRate = 0.;
	double dStdWidth = roiSizeX;
	double dStdHeight = roiSizeY;
	if (bSTD)
	{
		if (retBodyWidth && *retBodyWidth > 0) dStdWidth = *retBodyWidth;
		if (retBodyHeight && *retBodyHeight > 0) dStdHeight = *retBodyHeight;
	}
	if(nOkPercent != 1 && nOkPercent < 10)
		nOkPercent = 10;
	if (nOkPercent > 100)
		nOkPercent = 100;

	cv::Mat imgBlob(roiSizeY, roiSizeX, CV_8UC1, blobImg);
	//top left
	// top Y
	if (bUnitFind && bUNIT_W == false)
	{
		int nST = nTeachY >= 0 ? nTeachY : roiSizeY / 2;
		if (nST >= roiSizeY) nST = roiSizeY - 1;
		for (int y = nST; y >= 0; y--)
		{
			dWhiteValue = 0.0;
			double dMax = 0;
			for (int x = 0; x < roiSizeX; x++)
			{
				if (imgBlob.data[x + (y*roiSizeX)] == 255)
					dWhiteValue += 1;
				else
				{
					if (dWhiteValue > dMax)
						dMax = dWhiteValue;
					dWhiteValue = 0;
				}
			}
			if (dWhiteValue > dMax) dMax = dWhiteValue;
			dWhiteRate = (dMax / dStdWidth) * 100.0;
			if (dWhiteRate >= nOkPercent)
			{
				pt[0].y = y;	// top left
				pt[3].y = y;	// top right
				if (nOkPercent == 1 && y - 2 >= 0)
				{
					pt[0].y = y - 2;	// top left
					pt[3].y = y - 2;	// bottom left
				}
				else if (nOkPercent == 1 && y - 1 >= 0)
				{
					pt[0].y = y - 1;	// top left
					pt[3].y = y - 1;	// bottom left
				}
			}
			else
				break;
		}
	}
	if (bUnitFind == false || pt[0].y < 0 || pt[3].y < 0)
	{
		for (int y = 0; y < roiSizeY; y++)
		{
			dWhiteValue = 0.0;
			for (int x = 0; x < roiSizeX; x++)
			{
				if (imgBlob.data[x + (y*roiSizeX)] == 255)
					dWhiteValue += 1;
			}
			dWhiteRate = (dWhiteValue / dStdWidth) * 100.0;
			if (dWhiteRate >= nOkPercent)
			{
				pt[0].y = y;	// top left
				pt[3].y = y;	// top right
				if (nOkPercent == 1 && y - 2 >= 0)
				{
					pt[0].y = y - 2;	// top left
					pt[3].y = y - 2;	// top right
				}
				else if (nOkPercent == 1 && y - 1 >= 0)
				{
					pt[0].y = y - 1;	// top left
					pt[3].y = y - 1;	// top right
				}
				break;
			}
		}
	}

	// left X
	if (bUnitFind && bUNIT_W)
	{
		int nST = nTeachX >= 0 ? nTeachX : roiSizeX / 2;
		if (nST >= roiSizeX) nST = roiSizeX - 1;
		for (int x = nST; x >= 0; x--)
		{
			dWhiteValue = 0.0;
			double dMax = 0;
			for (int y = 0; y < roiSizeY; y++)
			{
				if (imgBlob.data[x + (y*roiSizeX)] == 255)
					dWhiteValue += 1;
				else
				{
					if (dWhiteValue > dMax)
						dMax = dWhiteValue;
					dWhiteValue = 0;
				}
			}
			if (dWhiteValue > dMax) dMax = dWhiteValue;
			dWhiteRate = (dMax / dStdHeight) * 100.0;
			if (dWhiteRate >= nOkPercent)
			{
				pt[0].x = x;	// top left
				pt[1].x = x;	// bottom left
				if (nOkPercent == 1 && x - 2 >= 0)
				{
					pt[0].x = x - 2;	// top left
					pt[1].x = x - 2;	// bottom left
				}
				else if (nOkPercent == 1 && x - 1 >= 0)
				{
					pt[0].x = x - 1;	// top left
					pt[1].x = x - 1;	// bottom left
				}
			}
			else
				break;
		}
	}
	if (bUnitFind == false || pt[0].x < 0 || pt[1].x < 0)
	{
		for (int x = 0; x < roiSizeX; x++)
		{
			dWhiteValue = 0.0;
			for (int y = 0; y < roiSizeY; y++)
			{
				if (imgBlob.data[x + (y*roiSizeX)] == 255)
					dWhiteValue += 1;
			}
			dWhiteRate = (dWhiteValue / dStdHeight) * 100.0;
			if (dWhiteRate >= nOkPercent)
			{
				pt[0].x = x;	// top left
				pt[1].x = x;	// bottom left
				if (nOkPercent == 1 && x - 2 >= 0)
				{
					pt[0].x = x - 2;	// top left
					pt[1].x = x - 2;	// bottom left
				}
				else if (nOkPercent == 1 && x - 1 >= 0)
				{
					pt[0].x = x - 1;	// top left
					pt[1].x = x - 1;	// bottom left
				}
				break;
			}
		}
	}
	//bottom Right
	// bottom Y
	int nStart = roiSizeY - 1;
	if (bUnitFind && bUNIT_W == false)
	{
		nStart = nTeachY >= 0 ? nTeachY : roiSizeY / 2;
		if (nStart >= roiSizeY) nStart = roiSizeY - 1;
		for (int y = nStart; y < roiSizeY; y++)
		{
			dWhiteValue = 0.0;
			double dMax = 0;
			for (int x = 0; x < roiSizeX; x++)
			{
				if (imgBlob.data[x + (y*roiSizeX)] == 255)
					dWhiteValue += 1;
				else
				{
					if (dWhiteValue > dMax)
						dMax = dWhiteValue;
					dWhiteValue = 0;
				}
			}
			if (dWhiteValue > dMax) dMax = dWhiteValue;
			dWhiteRate = (dMax / dStdWidth) * 100.0;
			if (dWhiteRate >= nOkPercent)
			{
				pt[1].y = y;	// bottom left
				pt[2].y = y;	// bottom right
				if (nOkPercent == 1 && y + 2 <= roiSizeY)
				{
					pt[1].y = y + 2;		// bottom left
					pt[2].y = y + 2;	// bottom right
				}
				else if ((nOkPercent >= 10 || nOkPercent == 1) && y + 1 <= roiSizeY)
				{
					pt[1].y = y + 1;		// bottom left
					pt[2].y = y + 1;	// bottom right
				}
			}
			else
				break;
		}
	}
	if (bUnitFind == false || pt[1].y < 0 || pt[2].y < 0)
	{
		nStart = roiSizeY - 1;
		/*if (nStart < 20)
			nStart = roiSizeY;*/
		for (int y = nStart; y >= 0; y--)
		{
			dWhiteValue = 0.0;
			for (int x = 0; x < roiSizeX; x++)
			{
				if (imgBlob.data[x + (y*roiSizeX)] == 255)
					dWhiteValue += 1;
			}
			dWhiteRate = (dWhiteValue / dStdWidth) * 100.0;
			if (dWhiteRate >= nOkPercent)
			{
				pt[1].y = y;	// bottom left
				pt[2].y = y;	// bottom right
				if (nOkPercent == 1 && y + 2 <= roiSizeY)
				{
					pt[1].y = y + 2;		// bottom left
					pt[2].y = y + 2;	// bottom right
				}
				else if ((/*nOkPercent >= 10 || */nOkPercent == 1) && y + 1 <= roiSizeY)
				{
					pt[1].y = y + 1;		// bottom left
					pt[2].y = y + 1;	// bottom right
				}
				break;
			}
		}
	}
	// right X
	if (bUnitFind && bUNIT_W)
	{
		nStart = nTeachX >= 0 ? nTeachX : roiSizeX / 2;
		if (nStart >= roiSizeX) nStart = roiSizeX - 1;
		for (int x = nStart; x < roiSizeX; x++)
		{
			dWhiteValue = 0.0;
			double dMax = 0;
			for (int y = 0; y < roiSizeY; y++)
			{
				if (imgBlob.data[x + (y*roiSizeX)] == 255)
					dWhiteValue += 1;
				else
				{
					if (dWhiteValue > dMax)
						dMax = dWhiteValue;
					dWhiteValue = 0;
				}
			}
			if (dWhiteValue > dMax) dMax = dWhiteValue;
			dWhiteRate = (dMax / dStdHeight) * 100.0;
			if (dWhiteRate > nOkPercent)
			{
				pt[2].x = x;	// bottom right
				pt[3].x = x;	// top right
				if (nOkPercent == 1 && x + 2 <= roiSizeX)
				{
					pt[2].x = x + 2;		// bottom left
					pt[3].x = x + 2;	// bottom right
				}
				else if ((nOkPercent >= 10 || nOkPercent == 1) && x + 1 <= roiSizeX)
				{
					pt[2].x = x + 1;		// bottom left
					pt[3].x = x + 1;	// bottom right
				}
			}
			else
				break;
		}
	}
	if (bUnitFind == false || pt[2].x < 0 || pt[3].x < 0)
	{
		nStart = roiSizeX - 1;
		/*if (nStart < 20)
			nStart = roiSizeX;*/
		for (int x = nStart; x >= 0; x--)
		{
			dWhiteValue = 0.0;
			for (int y = 0; y < roiSizeY; y++)
			{
				if (imgBlob.data[x + (y*roiSizeX)] == 255)
					dWhiteValue += 1;
			}
			dWhiteRate = (dWhiteValue / dStdHeight) * 100.0;
			if (dWhiteRate > nOkPercent)
			{
				pt[2].x = x;	// bottom right
				pt[3].x = x;	// top right
				if (nOkPercent == 1 && x + 2 <= roiSizeX)
				{
					pt[2].x = x + 2;		// bottom left
					pt[3].x = x + 2;	// bottom right
				}
				else if ((/*nOkPercent >= 10 || */nOkPercent == 1) && x + 1 <= roiSizeX)
				{
					pt[2].x = x + 1;		// bottom left
					pt[3].x = x + 1;	// bottom right
				}
				break;
			}
		}
	}

	cv::Mat imgBody(roiSizeY, roiSizeX, CV_8UC1, cv::Scalar(0));
	if (bUnitFind)
	{
		std::vector<cv::Point> vRect;
		for (int a = 0; a < 4; a++)
		{
			cv::Point pPo(pt[a].x, pt[a].y);
			vRect.push_back(pPo);
		}
		cv::fillPoly(imgBody, vRect, cv::Scalar(255));
	}
	else
	{
		int nL = (pt[0].x <= pt[1].x) ? pt[0].x : pt[1].x;
		int nR = (pt[2].x <= pt[3].x) ? pt[3].x : pt[2].x;
		int nT = (pt[0].y <= pt[3].y) ? pt[0].y : pt[3].y;
		int nB = (pt[1].y <= pt[2].y) ? pt[2].y : pt[1].y;
		if (nL < 0) nL = 0;
		if (nR < 0) nR = 0;
		if (nT < 0) nT = 0;
		if (nB < 0) nB = 0;
		if (nL >= roiSizeX) nL = roiSizeX - 1;
		if (nR >= roiSizeX) nR = roiSizeX - 1;
		if (nT >= roiSizeY) nT = roiSizeY - 1;
		if (nB >= roiSizeY) nB = roiSizeY - 1;
		if(nR - nL < 0 || nB - nT < 0)
			return 0;
		imgBody(cv::Rect(nL, nT, nR - nL, nB - nT)).setTo(255);
	}
	cv::bitwise_and(imgBody, imgBlob, imgBlob);

	double avrAngle = 0;

	if (retNodalPoint != NULL)
	{
		memcpy_s(retNodalPoint, sizeof(POINT) * 4, pt, sizeof(POINT) * 4);
	}
	//////////////////////////////////////////////////////////////////////////
	double bodyW_T = 0;
	double bodyW_B = 0;
	double bodyH_L = 0;
	double bodyH_R = 0;

	LONG tempW = abs(pt[3].x - pt[0].x); if (tempW != 0) tempW += 1;
	LONG tempH = abs(pt[0].y - pt[3].y); if (tempH != 0) tempH += 1;
	bodyW_T = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[2].x - pt[1].x); if (tempW != 0) tempW += 1;
	tempH = abs(pt[1].y - pt[2].y); if (tempH != 0) tempH += 1;
	bodyW_B = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[0].x - pt[1].x); if (tempW != 0) tempW += 1;
	tempH = abs(pt[1].y - pt[0].y); if (tempH != 0) tempH += 1;
	bodyH_L = sqrt((double)(tempW * tempW + tempH * tempH));

	tempW = abs(pt[2].x - pt[3].x); if (tempW != 0) tempW += 1;
	tempH = abs(pt[2].y - pt[3].y); if (tempH != 0) tempH += 1;
	bodyH_R = sqrt((double)(tempW * tempW + tempH * tempH));


	double bodyWidth = (bodyW_T + bodyW_B) / 2.0;
	double bodyHeight = (bodyH_L + bodyH_R) / 2.0;

	CRect bodyRect;
	LONG nLeft = (LONG)((pt[0].x <= pt[1].x)?		pt[0].x : pt[1].x);
	LONG nRight = (LONG)((pt[2].x <= pt[3].x)?		pt[3].x : pt[2].x);
	LONG nTop = (LONG)((pt[0].y <= pt[3].y)?		pt[0].y : pt[3].y);
	LONG nBottom = (LONG)((pt[1].y <= pt[2].y)?		pt[2].y : pt[1].y);
	if(nLeft < 0) nLeft = 0;
	if(nRight > roiSizeX) nRight = roiSizeX;
	if(nTop < 0) nTop = 0;
	if(nBottom > roiSizeY) nBottom = roiSizeY;

	if((bodyWidth < 1) || (bodyHeight < 1))
		return avrAngle;
	if((nLeft < 0) || (nRight < 0) || (nTop < 0) || (nBottom < 0))
		return avrAngle;
	if((nLeft > roiSizeX) || (nRight > roiSizeX) || (nTop > roiSizeY) || (nBottom > roiSizeY))
		return avrAngle;
	if((bodyWidth > roiSizeX) || (bodyHeight > roiSizeY))
		return avrAngle;


	bodyRect.left = nLeft;
	bodyRect.right = nRight;
	bodyRect.top = nTop;
	bodyRect.bottom = nBottom;

	if (retBodyWidth != NULL)
		*retBodyWidth = bodyWidth;
	if (retBodyHeight != NULL)
		*retBodyHeight = bodyHeight;
	//////////////////////////////////////////////////////////////////////////

	avrAngle = GetBodyAngle(imgBlob.data, roiSizeX, roiSizeY, bodyRect.left+(bodyWidth/2), bodyRect.top+(bodyHeight/2), bodyWidth, bodyHeight);
	if (retBodyA != NULL)
		*retBodyA = cv::countNonZero(imgBlob);

	//avrAngle = GetAngle(pt[3].x,pt[3].y,pt[2].x,pt[2].y);//minRect[0].angle;
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	double cogX = stx + (bodyRect.left + ((bodyWidth - 1) / 2.0));
	double cogY = sty + (bodyRect.top + ((bodyHeight - 1) / 2.0));

	if(retCogX != NULL)
		*retCogX = cogX;
	if(retCogY != NULL)
		*retCogY = cogY;
	if(retRect != NULL)
		*retRect = bodyRect;

	return avrAngle;
}

//SHK 2013/11/11
double CProc3D::GetAngle(int x1,int y1, int x2,int y2){
	int dx = x2 - x1;
	int dy = y2 - y1;

	double rad= atan2((double)dx, (double)dy);
	double degree = (rad*180)/PI ;

	return degree;
}

// SHW 20141127 body blob angle get
// LYS 20141203 Modify
// LMJ 2017/05/26 Modify
//	1. 바디 3/4 길이 내 모든 포인트를 이용해 Angle을 구하고, 추세선을 구한다
// 	2. 모든 포인트의 추세선을 벗어나는 Error값을 구한다
// 	3. Error 평균 +- 2 * Error 표준편차를 벗어나는 포인트를 제외시킨다
// 	4. 나머지 포인트들을 이용해 다시 Angle을 구한다
double CProc3D::GetBodyAngle(UCHAR* ucpBlobImg, int nSizeX, int nSizeY, int nBlobCenterX, int nBlobCenterY, int nBlobWidth, int nBlobHeight)
{
	double dAngle = 0.f;
	if(!ucpBlobImg || (nSizeX <= 0) || (nSizeY <= 0) || (nBlobWidth <= 0) || (nBlobHeight <= 0))
		return dAngle;
	if(nBlobWidth > nSizeX || nBlobHeight > nSizeY)
	{
		CString str;
		str.Format(_T("nSizeX : %d, nSizeY : %d, nBlobWidth : %d, nBlobHeight : %d"), nSizeX, nSizeY, nBlobWidth, nBlobHeight);
		//AfxMessageBox(str);
		return dAngle;
	}

#if _DEBUG
	cv::Mat imgBlobImg(nSizeY, nSizeX, CV_8UC1, ucpBlobImg);
#endif
	int nSize = 0;
	float * pfArrCenterPosX = NULL;
	float * pfArrCenterPosY = NULL;

	float fX1 = 0.f;
	float fX2 = 0.f;
	float fY1 = 0.f;
	float fY2 = 0.f;
	double dRad = 0.0;

	float a, b;
	// 	int nConditionValue = 0;
	// X축이 더 긴 Window의 경우
	if(nSizeX >= nSizeY)
	{
		// 가로가 긴 경우 theta = atan2((y2-y1)/(x2-x1))
		nSize = RounD(nBlobWidth * 0.75);		// Blob SizeAC 3/4 ±æAI
		if(nSize % 2 != 0)
		{
			nSize -= 1;
			if(nSize <= 0)
			{
				if (pfArrCenterPosX != NULL)
				{
					//delete [] pfArrCenterPosX;
					g_pMManager->pem_delete(pfArrCenterPosX, true);
					pfArrCenterPosX = NULL;
				}

				if (pfArrCenterPosY != NULL)
				{
					//delete [] pfArrCenterPosY;
					g_pMManager->pem_delete(pfArrCenterPosY, true);
					pfArrCenterPosY = NULL;
				}
				return dAngle;
			}
		}
		/*pfArrCenterPosX = new float [nSize];
		pfArrCenterPosY = new float [nSize];*/
		pfArrCenterPosX = g_pMManager->pem_new<float>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
		pfArrCenterPosY = g_pMManager->pem_new<float>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
		fX1 = nBlobCenterX - RounD(nSize / 2.);		// 1/8 AoA¡
		fX2 = nSize + fX1 - 1;
		if(fX1 >= nSizeX || fX2 >= nSizeX)
		{
			if (pfArrCenterPosX != NULL)
			{
				//delete [] pfArrCenterPosX;
				g_pMManager->pem_delete(pfArrCenterPosX, true);
				pfArrCenterPosX = NULL;
			}

			if (pfArrCenterPosY != NULL)
			{
				//delete [] pfArrCenterPosY;
				g_pMManager->pem_delete(pfArrCenterPosY, true);
				pfArrCenterPosY = NULL;
			}
			CString str;
			str.Format(_T("nSizeX : %d, fX1 : %.2f, fX2 : %.2f"), nSizeX, fX1, fX2);
			//AfxMessageBox(str);
			return dAngle;
		}

		int nBtmY = 0, nTopY = 0;	// y bottom, y top
		int nIndex = 0;
		// 		nConditionValue = nBlobCenterY * 0.1;

		for(int x = (int)fX1; x <= fX2; x++)
		{
			for(int y = 0; y < nSizeY; y++)
			{
				if(ucpBlobImg[y*nSizeX + x] == 255)
				{
					nTopY = y;
					break;
				}
			}
			for(int y = nSizeY - 1; y > 0; y--)
			{
				if(ucpBlobImg[y*nSizeX + x] == 255)
				{
					nBtmY = y;
					break;
				}
			}

			// 			if(((nTopY + nBtmY) / 2.) <= nBlobCenterY + nConditionValue && ((nTopY + nBtmY) / 2.) >= nBlobCenterY - nConditionValue)
			{
				pfArrCenterPosX[nIndex] = x;
				pfArrCenterPosY[nIndex] = (nTopY + nBtmY) / 2.;
				nIndex++;
			}
		}

		GetAB(pfArrCenterPosX, pfArrCenterPosY, nIndex, &a, &b);
		if(a == 0)
			dRad = 0.0;
		else
		{
			/*float * pfArrErr = new float [nIndex];
			float * pfArrCenterPosY_2 = new float[nIndex];*/
			float * pfArrErr = g_pMManager->pem_new<float>(true, nIndex, (PCHAR)__FUNCTION__, __LINE__);
			float * pfArrCenterPosY_2 = g_pMManager->pem_new<float>(true, nIndex, (PCHAR)__FUNCTION__, __LINE__);
			for(int i = 0; i < nIndex; i++)
			{
				pfArrCenterPosY_2[i] = a * pfArrCenterPosX[i] + b;
				pfArrErr[i] = fabs(pfArrCenterPosY_2[i] - pfArrCenterPosY[i]);
			}

			double err_avg, err_stdev;
			GetStdev(pfArrErr, 0, nIndex, &err_avg, &err_stdev);

			int nLastCount = 0;
			for(int i = 0; i < nIndex; i++)
			{
				if(pfArrErr[i] > err_avg + 2 * err_stdev || pfArrErr[i] < err_avg - 2 * err_stdev)
					continue;

				pfArrCenterPosY[nLastCount] = pfArrCenterPosY[i];
				pfArrCenterPosX[nLastCount] = pfArrCenterPosX[i];
				nLastCount++;
			}
			if(nIndex > nLastCount)
				GetAB(pfArrCenterPosX, pfArrCenterPosY, nLastCount, &a, &b);

			dRad = atan(a);

			//delete [] pfArrErr;
			g_pMManager->pem_delete(pfArrErr, true);
			pfArrErr = NULL;
			//delete [] pfArrCenterPosY_2;
			g_pMManager->pem_delete(pfArrCenterPosY_2, true);
			pfArrCenterPosY_2 = NULL;
		}

		dAngle = ((dRad*180) / PI) * -1 ;
	}
	else	// YAaAI ´o ±a WindowAC °æ¿i
	{
		// ¼¼·I°¡ ±a °æ¿i theta = atan2((x2-x1)/(y2-y1))
		nSize = RounD(nBlobHeight * 0.75);		// Blob SizeAC 3/4 ±æAI
		if(nSize % 2 != 0)
		{
			nSize -= 1;
			if (nSize <= 0)
			{
				if (pfArrCenterPosX != NULL)
				{
					//delete [] pfArrCenterPosX;
					g_pMManager->pem_delete(pfArrCenterPosX, true);
					pfArrCenterPosX = NULL;
				}

				if (pfArrCenterPosY != NULL)
				{
					//delete [] pfArrCenterPosY;
					g_pMManager->pem_delete(pfArrCenterPosY, true);
					pfArrCenterPosY = NULL;
				}
				return dAngle;
			}
		}
		/*pfArrCenterPosX = new float [nSize];
		pfArrCenterPosY = new float [nSize];*/
		pfArrCenterPosX = g_pMManager->pem_new<float>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
		pfArrCenterPosY = g_pMManager->pem_new<float>(true, nSize, (PCHAR)__FUNCTION__, __LINE__);
		fY1 = nBlobCenterY - RounD(nSize / 2.);		// 1/8 AoA¡
		fY2 = nSize + fY1 - 1;
		if(fY1 >= nSizeY || fY2 >= nSizeY)
		{
			if (pfArrCenterPosX != NULL)
			{
				//delete [] pfArrCenterPosX;
				g_pMManager->pem_delete(pfArrCenterPosX, true);
				pfArrCenterPosX = NULL;
			}

			if (pfArrCenterPosY != NULL)
			{
				//delete [] pfArrCenterPosY;
				g_pMManager->pem_delete(pfArrCenterPosY, true);
				pfArrCenterPosY = NULL;
			}
			CString str;
			str.Format(_T("nSizeY : %d, fY1 : %.2f, fY2 : %.2f"), nSizeY, fY1, fY2);
			//AfxMessageBox(str);
			return dAngle;
		}

		int nBtmX = 0, nTopX = 0;	// y bottom, y top
		int nIndex = 0;
		// 		nConditionValue = nBlobCenterX * 0.1;

		for(int y = (int)fY1; y <= fY2; y++)
		{
			for(int x = 0; x < nSizeX; x++)
			{
				if(ucpBlobImg[y*nSizeX + x] == 255)
				{
					nTopX = x;
					break;
				}
			}
			for(int x = nSizeX - 1; x > 0; x--)
			{
				if(ucpBlobImg[y*nSizeX + x] == 255)
				{
					nBtmX = x;
					break;
				}
			}

			// 			if(((nTopX + nBtmX) / 2.) <= nBlobCenterX + nConditionValue && ((nTopX + nBtmX) / 2.) >= nBlobCenterX - nConditionValue)
			// 			{
			pfArrCenterPosX[nIndex] = (nTopX + nBtmX) / 2.;
			pfArrCenterPosY[nIndex] = y;
			nIndex++;
			// 			}

		}

		GetAB(pfArrCenterPosY, pfArrCenterPosX, nIndex, &a, &b);
		if(a == 0)
			dRad = 0.0;
		else
		{
			/*float * pfArrErr = new float [nIndex];
			float * pfArrCenterPosX_2 = new float[nIndex];*/
			float * pfArrErr = g_pMManager->pem_new<float>(true, nIndex, (PCHAR)__FUNCTION__, __LINE__);
			float * pfArrCenterPosX_2 = g_pMManager->pem_new<float>(true, nIndex, (PCHAR)__FUNCTION__, __LINE__);
			for(int i = 0; i < nIndex; i++)
			{
				pfArrCenterPosX_2[i] = a * pfArrCenterPosY[i] + b;
				pfArrErr[i] = fabs(pfArrCenterPosX_2[i] - pfArrCenterPosX[i]);
			}

			double err_avg, err_stdev;
			GetStdev(pfArrErr, 0, nIndex, &err_avg, &err_stdev);

			int nLastCount = 0;
			for(int i = 0; i < nIndex; i++)
			{
				if(pfArrErr[i] > err_avg + 2 * err_stdev || pfArrErr[i] < err_avg - 2 * err_stdev)
					continue;

				pfArrCenterPosY[nLastCount] = pfArrCenterPosY[i];
				pfArrCenterPosX[nLastCount] = pfArrCenterPosX[i];
				nLastCount++;
			}
			if(nIndex > nLastCount)
				GetAB(pfArrCenterPosY, pfArrCenterPosX, nLastCount, &a, &b);

			dRad = atan(a);

			//delete [] pfArrErr;
			g_pMManager->pem_delete(pfArrErr, true);
			pfArrErr = NULL;
			//delete [] pfArrCenterPosX_2;
			g_pMManager->pem_delete(pfArrCenterPosX_2, true);
			pfArrCenterPosX_2 = NULL;
		}

		dAngle = (dRad*180) / PI ;
	}

	// #ifdef _DEBUG
	// 	FILE * file=fopen("d:\\testimage\\workimage\\BodtAngle.txt", "wt");
	// 	if(file!=NULL)
	// 	{
	// 		fprintf(file, "fx1 : %.3f\nfy1 : %.3f\nfx2 : %.3f\nfy2 : %.3f\n\n", fX1, fY1, fX2, fY2);
	// 		fprintf(file, "fAvgLine : %f\ndRad : %f\ndAngle : %f\n", fAvgLine, dRad, dAngle);
	// 		fprintf(file, "BlobCenterX : %d\nBlobCenterY : %d\nConditionValue : %d\n", nBlobCenterX, nBlobCenterY, nConditionValue);
	// 		for (int a=0; a<nSize; a++)
	// 		{
	// 			fprintf(file, "pfArrCenterPosX : %f\tpfArrCenterPosY : %f\n", pfArrCenterPosX[a], pfArrCenterPosY[a]);
	// 		}
	// 
	// 		fclose(file);
	// 	}
	// #endif

	if(pfArrCenterPosX != NULL)
	{
		//delete [] pfArrCenterPosX;
		g_pMManager->pem_delete(pfArrCenterPosX, true);
		pfArrCenterPosX = NULL;
	}

	if(pfArrCenterPosY != NULL)
	{
		//delete [] pfArrCenterPosY;
		g_pMManager->pem_delete(pfArrCenterPosY, true);
		pfArrCenterPosY = NULL;
	}

	return dAngle;
}

void CProc3D::GetAB(float * fArrPosX, float * fArrPosY, int nCount, float * a, float * b)
{
	float matK[2][2] = { 0.0, };
	float inverse[2][2] = { 0.0, };
	float maty[2] = {0.0,};

	for(int i = 0; i < nCount; i++)
	{
		matK[0][0] += pow(fArrPosX[i],2.0f);
		matK[0][1] += fArrPosX[i];
		matK[1][0] += fArrPosX[i];

		maty[0] += fArrPosX[i]*fArrPosY[i];
		maty[1] += fArrPosY[i];
	}
	matK[1][1] = nCount;

	float fTempa;

	if(matK[0][0] * matK[1][1] != matK[0][1] * matK[1][0])
	{

		fTempa = (matK[0][0] * matK[1][1]) - (matK[0][1] * matK[1][0]);

		inverse[0][0] = matK[1][1] / fTempa;
		inverse[0][1] = -matK[0][1] / fTempa;
		inverse[1][0] = -matK[1][0] / fTempa;
		inverse[1][1] = matK[0][0] / fTempa;

		*a = (inverse[0][0] * maty[0]) + (inverse[0][1] * maty[1]);
		*b = (inverse[1][0] * maty[0]) + (inverse[1][1] * maty[1]);
	}
	else
	{
		*a = 0.0;
	}
}

cv::Point CProc3D::WarpAffine(cv::Mat orgImg, cv::Mat& roImg, float Seta, cv::Size sz, cv::Point Center, bool NOtCent, bool Rev)
{
	cv::Point Cent, NCenter, temp;
	cv::Point Margin(0, 0);
	int Maxsz = orgImg.cols > orgImg.rows ? orgImg.cols : orgImg.rows;
	Cent.x = (orgImg.cols - 1) / 2;// MaxI;
	Cent.y = (orgImg.rows - 1) / 2;// MaxI;
	cv::Mat Img = orgImg.clone();
#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_orgImg.bmp", orgImg);
#endif
	Margin.x = Center.x - orgImg.cols / 2;
	Margin.y = Center.y - orgImg.rows / 2;

	NCenter = Center;
	temp.x = 0;
	temp.y = 0;

	if (abs(Center.x - Cent.x) > 2 || abs(Center.y - Cent.y) > 2)
	{
		cv::Point TmpM;
		if (Rev)
		{
			TmpM.x = Center.x - Cent.x;// +Margin.x * 2;
			TmpM.y = Center.y - Cent.y;// +Margin.y * 2;
		}
		else
		{
			TmpM.x = Center.x - Cent.x;
			TmpM.y = Center.y - Cent.y;
		}

		int w = TmpM.x <= 0 ? Img.cols + (Img.cols - Center.x - Center.x) : Img.cols + (Center.x * 2 - Img.cols);
		int h = TmpM.y <= 0 ? Img.rows + (Img.rows - Center.y - Center.y) : Img.rows + (Center.y * 2 - Img.rows);

		cv::Mat TempImage(h, w, orgImg.type());
		TempImage.setTo(0);

		temp.x = TempImage.cols / 2 - Center.x;// +TmpM.x;
		temp.y = TempImage.rows / 2 - Center.y;// +TmpM.y;

		cv::Mat roi = TempImage(cv::Rect(temp.x, temp.y, Img.cols, Img.rows));

		orgImg.copyTo(roi);

		Img = TempImage;
		NCenter.x = Img.cols / 2;
		NCenter.y = Img.rows / 2;

	}
	cv::Mat RotImage;
	cv::Mat rotation = getRotationMatrix2D(NCenter, Seta, 1);

	if (CV_MAT_TYPE(orgImg.type()) == CV_32FC1)
		cv::warpAffine(Img, RotImage, rotation, Img.size(), cv::INTER_LINEAR);
	else
		cv::warpAffine(Img, RotImage, rotation, Img.size(), cv::INTER_NEAREST);

#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_RotImage.bmp", RotImage);
#endif

	int MarX(0), MarY(0);
	if (NOtCent)
	{
		MarX = Margin.x;
		MarY = Margin.y;
		if (Rev)
		{
			MarX += Margin.x*0.5;
			MarY += Margin.y*0.5;
		}
	}

	temp.x = RotImage.cols / 2 - Cent.x - MarX;
	temp.y = RotImage.rows / 2 - Cent.y - MarY;

	//Center가 이미지 중심에서 오른쪽에 있을 경우
	if (Center.x > Cent.x)
	{
		temp.x = RotImage.cols / 2 - Center.x - MarX;
		Margin.x = 0;
	}

	//Center가 이미지 중심에서 아래에 있을 경우
	if (Center.y > Cent.y)
	{
		temp.y = RotImage.rows / 2 - Center.y - MarY;
		Margin.y = 0;
	}

	cv::Mat Crop = RotImage;
	int x(0), y(0), w = orgImg.cols, h = orgImg.rows;
	if (temp.x < 0)
	{
		x = -temp.x;
		w = orgImg.cols + temp.x;

		temp.x = 0;

		if (temp.y < 0)
		{
			y = -temp.y;
			h = orgImg.rows + temp.y;

			temp.y = 0;
			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else if (temp.y + orgImg.rows > RotImage.rows)
		{
			y = temp.y;
			y = 0;
			//int diffh = (orgImg.rows + temp.y) - RotImage.rows;
			//h = (orgImg.rows - diffh);
			h = RotImage.rows - temp.y;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else
		{
			y = temp.y;
			h = orgImg.rows;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, 0, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
	}
	else if (temp.x + orgImg.cols > RotImage.cols)
	{
		x = temp.x;
		x = 0;

		w = RotImage.cols - temp.x;

		if (temp.y < 0)
		{
			y = -temp.y;
			h = orgImg.rows + temp.y;

			temp.y = 0;
			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else if (temp.y + orgImg.rows > RotImage.rows)
		{
			y = temp.y;
			y = 0;

			h = RotImage.rows - temp.y;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, y, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
		else
		{
			y = temp.y;
			h = orgImg.rows;

			cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

			roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
			roImg.setTo(0);
			cv::Mat roi = roImg(cv::Rect(x, 0, w, h));

			cropImg.copyTo(roi);
			return Margin;
		}
	}
	if (temp.y < 0)
	{
		y = -temp.y;
		h = orgImg.rows + temp.y;

		temp.y = 0;
		cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

		roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
		roImg.setTo(0);
		cv::Mat roi = roImg(cv::Rect(0, y, w, h));

		cropImg.copyTo(roi);
		return Margin;
	}
	else if (temp.y + orgImg.rows > RotImage.rows)
	{
		y = temp.y;
		y = 0;

		h = RotImage.rows - temp.y;

		cv::Mat cropImg = RotImage(cv::Rect(temp.x, temp.y, w, h)).clone();

		roImg = cv::Mat(orgImg.rows, orgImg.cols, orgImg.type());
		roImg.setTo(0);
		cv::Mat roi = roImg(cv::Rect(0, y, w, h));

		cropImg.copyTo(roi);
		return Margin;
	}

	roImg = RotImage(cv::Rect(temp.x, temp.y, orgImg.cols, orgImg.rows)).clone();
#if _DEBUG
	cv::imwrite("D:\\FootRst\\Pad_WarpAffine_roImg.bmp", roImg);
#endif
	return Margin;
}

void CProc3D::RotateZmap_ipp2(float* zmapData, float** dst, int orgSizeX, int orgSizeY, double angle, int* retDstSizeX, int* retDstSizeY)
{
	bool bLinear = true;
	bool bUseOrgSize = false;

	bool ret = RotateZmap_ipp2020(zmapData, dst, angle, orgSizeX, orgSizeY, retDstSizeX, retDstSizeY, bLinear, bUseOrgSize);
}

bool CProc3D::RotateZmap_ipp2020(float* userSrc, float** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bLinear, bool bUseOrgSize)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (*userDst == NULL)
			*userDst = g_pMManager->pem_new<float>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
		ippiCopy_32f_C1R(userSrc, orgSizeX * 4, *userDst, orgSizeX * 4, srcSize);

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp32f* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//변화된 크기 만큼
	if (bUseOrgSize == false)
	{
		/*dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
		  dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);*/
		dstSize.width = (int)(bound[1][0] - bound[0][0] + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}

	//Memory allocation for the intermediate images
	//*userDst = new float[dstSize.width * dstSize.height];
	*userDst = g_pMManager->pem_new<float>(true, dstSize.width * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);
	Ipp32f* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//변화된 크기 만큼 Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;

	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (status >= ippStsNoErr) status = ippiWarpAffineLinear_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp32f, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp32f, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (status >= ippStsNoErr) status = ippiWarpAffineNearest_32f_C1R(pSrc, srcStep * sizeof(Ipp32f), pDstRoi, dstSize.width * sizeof(Ipp32f), dstOffset, dstSize, pSpec, pBuffer);
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}

int CProc3D::ProcAnyAngle_Zmap(float* src, float** dst, int orgSizeX, int orgSizeY, int bdryCx, int bdryCy, int bdrySizeX, int bdrySizeY, double angle, int* retDstSizeX, int* retDstSizeY)
{
	int ret = TRUE;
	int bdryRoiArea = bdrySizeX * bdrySizeY;
	float* zmapBoundary = g_pMManager->pem_new<float>(true, bdryRoiArea, (PCHAR)__FUNCTION__, __LINE__);
	GetCropZmap(src, zmapBoundary, orgSizeX, orgSizeY, bdryCx, bdryCy, bdrySizeX, bdrySizeY);
	RotateZmap_ipp2(zmapBoundary, dst, bdrySizeX, bdrySizeY, angle, retDstSizeX, retDstSizeY);

	g_pMManager->pem_delete(zmapBoundary, true);

	return ret;
}

int CProc3D::ProcAnyAngle_Zmap_LT(float* src, float** dst, int orgSizeX, int orgSizeY, int bdry_stX, int bdry_stY, int bdrySizeX, int bdrySizeY, double angle, int* retDstSizeX, int* retDstSizeY)
{
	int ret = TRUE;
	int bdryRoiArea = bdrySizeX * bdrySizeY;
	//float* zmapBoundary = new float[bdryRoiArea];
	float* zmapBoundary = g_pMManager->pem_new<float>(true, bdryRoiArea, (PCHAR)__FUNCTION__, __LINE__);
	GetCropZmap_LT(src, zmapBoundary, orgSizeX, orgSizeY, bdry_stX, bdry_stY, bdrySizeX, bdrySizeY);
	RotateZmap_ipp2(zmapBoundary, dst, bdrySizeX, bdrySizeY, angle, retDstSizeX, retDstSizeY);

	//delete zmapBoundary;
	g_pMManager->pem_delete(zmapBoundary, false);

	return ret;
}

/*
void CProc3D::SaveBuffer(unsigned char* pImageSrc, int nWidth, int nHeight, int nChannel, char *psFileName)
{
	int nDepth = 8;
	
	IplImage* iplImage = cvCreateImageHeader(cv::Size(nWidth,nHeight), nDepth, nChannel);
	cvSetData(iplImage, pImageSrc, iplImage->widthStep);
	cvSaveImage(psFileName, iplImage);
}
*/
bool CProc3D::CheckBody(UCHAR* ucImg, int nW, int nH, CRect rcROI, double dBodyW, double dBodyH, double dStdW, double dStdH, double dPerOK)
{
	int nGap = 5;
	if (dBodyW <= 0 || dBodyH <= 0)
		return true;

	double dPerX = (dBodyW / dStdW * 100.0) - 100.0;
	double dPerY = (dBodyH / dStdH * 100.0) - 100.0;
	if (dPerX < dPerOK && dPerY < dPerOK)
		return true;

	for (int a = 0; a < 2; a++)
	{
		double dTotal = 0;
		double dOUT = 0;
		double dStd = (a == 0) ? dStdW : dStdH;
		double dROI = (a == 0) ? dBodyW : dBodyH;
		double dPer = (dROI / dStd * 100.0) - 100.0;
		if (dPer < dPerOK)
			continue;

		int nXW = (a == 0) ? nW : nH;
		int nSTX = (a == 0) ? rcROI.left - nGap : rcROI.top - nGap;
		int nETX = (a == 0) ? rcROI.right + nGap : rcROI.bottom + nGap;

		int nYW = (a == 0) ? nH : nW;
		int nSTY = (a == 0) ? rcROI.top - nGap : rcROI.left - nGap;
		int nETY = (a == 0) ? rcROI.bottom + nGap : rcROI.right + nGap;
		for (int y = nSTY; y < nETY; y++)
		{
			if (y < 0) continue;
			if (y >= nYW) break;
			int nST = -1;
			int nET = -1;

			for (int x = nSTX; x < nETX; x++)
			{
				if (x < 0) continue;
				if (x >= nXW) break;
				int nIndex = (a == 0) ? (y * nW) + x : (x * nW) + y;
				if (ucImg[nIndex] == 255)
				{
					nST = x;
					break;
				}
			}
			if (nST < 0) continue;

			for (int x = nETX; x > nSTX; x--)
			{
				if (x < 0) break;
				if (x >= nXW) continue;
				int nIndex = (a == 0) ? (y * nW) + x : (x * nW) + y;
				if (ucImg[nIndex] == 255)
				{
					nET = x;
					break;
				}
			}
			if (nET < 0) continue;

			double dROI = nET - nST;
			dPer = (dROI / dStd * 100.0) - 100.0;
			if (dPer >= dPerOK)
				dOUT += 1.0;
			dTotal += 1.0;
		}

		dPer = (dOUT / dTotal * 100.0);
		if (dPer > dPerOK)
			return false;
	}
	return true;
}
