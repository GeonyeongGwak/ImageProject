#include "StdAfx.h"
#include "PInsp.h"
#include "MPTI.h"
#include "MemoryManager.h"
#include <math.h>

#include "ippi.h"
#include "ipps.h"
#pragma comment(lib, "ippi.lib")
#pragma comment(lib, "ipps.lib")


#define _TILTANGLE_USE_PCA 1
#define SQR(x) ((x)*(x))

CPInsp::CPInsp(void)
{
	m_className = _T("CPInsp");

// 	m_milSys = M_NULL;
// 	m_milApp = M_NULL;

	m_pProcMilAlgo = NULL;

	// SHKang 2017/07/24
	Init_PseudoMap();
	Set_PseudoResol(-50.f, 1230.f);
}


CPInsp::~CPInsp(void)
{
// 	m_milSys = M_NULL;
// 	m_milApp = M_NULL;
	CloseDevice();
}

int CPInsp::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int foxWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{	
	m_fovWidth = foxWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	CPInsp::CloseDevice();

	//m_pProcMilAlgo = new CProcMil();
	m_pProcMilAlgo = g_pMManager->pem_new<CProcMil>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_pProcMilAlgo->InitMil(milApp, milSys,bUseImagePilLib);

	return 0;
}

int CPInsp::CloseDevice()
{
	if(m_pProcMilAlgo != NULL)
	{
		m_pProcMilAlgo->FreeMil();
		
		//delete m_pProcMilAlgo;
		g_pMManager->pem_delete(m_pProcMilAlgo, false);
		m_pProcMilAlgo = NULL;
	}

	return 0;
}

double CPInsp::_pixel2mm_x(double val)
{
	double ret = 0;
	ret = val * m_resolX;
	return ret;		
}

double CPInsp::_pixel2mm_y(double val)
{
	double ret = 0;
	ret = val * m_resolY;
	return ret;		
}

double CPInsp::_mm2pixel_x(double val)
{
	double ret = 0;
	ret = val / m_resolX;
	return ret;
}

double CPInsp::_mm2pixel_y(double val)
{
	double ret = 0;
	ret = val / m_resolY;
	return ret;
}


double CPInsp::_GetColor(int color)
{
	double ret = 0.0;

	switch(color)
	{
	case eCOLOR_BLACK:	
		ret = COLOR_BLACK;
		break;
	case eCOLOR_RED:	
		ret = COLOR_RED;
		break;
	case eCOLOR_GREEN:
		ret = COLOR_GREEN;
		break;
	case eCOLOR_BLUE:	
		ret = COLOR_BLUE;
		break;
	case eCOLOR_YELLOW:	
		ret = COLOR_YELLOW;
		break;
	case eCOLOR_MAGENTA:
		ret = COLOR_MAGENTA;
		break;
	case eCOLOR_CYAN:	
		ret = COLOR_CYAN;
		break;
	case eCOLOR_WHITE:	
		ret = COLOR_WHITE;
		break;
	case eCOLOR_GRAY:
		ret = COLOR_GRAY;
		break;
	case eCOLOR_BRIGHT_GRAY:
		ret = COLOR_BRIGHT_GRAY;
		break;
	case eCOLOR_LIGHT_GRAY:	
		ret = COLOR_LIGHT_GRAY;
		break;
	case eCOLOR_LIGHT_GREEN:
		ret = COLOR_LIGHT_GREEN;
		break;
	case eCOLOR_LIGHT_BLUE:	
		ret = COLOR_LIGHT_BLUE;
		break;
	case eCOLOR_LIGHT_WHITE:	
		ret = COLOR_LIGHT_WHITE;
		break;
	case eCOLOR_DARK_RED:
		ret = COLOR_DARK_RED;
		break;
	case eCOLOR_DARK_GREEN:	
		ret = COLOR_DARK_GREEN;
		break;
	case eCOLOR_DARK_BLUE:	
		ret = COLOR_DARK_BLUE;
		break;
	case eCOLOR_DARK_YELLOW:
		ret = COLOR_DARK_YELLOW;
		break;
	case eCOLOR_DARK_MAGENTA:
		ret = COLOR_DARK_MAGENTA;
		break;
	case eCOLOR_DARK_CYAN:
		ret = COLOR_DARK_CYAN;
		break;
	case eCOLOR_ORANGE:
		ret = COLOR_ORANGE;
		break;
	}

	return ret;
}

void CPInsp::CvtPixelToBoard(double ctFovCx, double ctFovCy, double ctRoiCx, double ctRoiCy, double* retX, double* retY, int mode) 
{

	double fovWidth = ((m_fovWidth) * m_resolX);  //unit : pixel -> mm
	double fovLength = ((m_fovLength) * m_resolY);  //unit : pixel -> mm

	double m = ctRoiCx * fovWidth / (m_fovWidth);
	double n = ctRoiCy * fovLength / (m_fovLength);

	double x = 0;
	double y = 0;

	x = ctFovCx - (fovWidth / 2.0) + m;
	y = ctFovCy + (fovLength / 2.0) - n;

	*retX = x;
	*retY = y;
}


int CPInsp::GetLightIndex(int channelIndex)
{
	int ret = 0;

	switch(channelIndex)
	{
	case eM2C_TR:
		{
			ret = eImg_Top_R;
		}
		break;
	case eM2C_TG:
		{
			ret = eImg_Top_G;
		}
		break;
	case eM2C_TB:
		{
			ret = eImg_Top_B;
		}
		break;
	case eM2C_MR:
		{
			ret = eImg_Middle_R;
		}
		break;
	case eM2C_MB:
		{
			ret = eImg_Middle_B;
		}
		break;
	case eM2C_BR:
		{
			ret = eImg_Bottom_R;
		}
		break;
	case eM2C_BB:
		{
			ret = eImg_Bottom_B;
		}
		break;
	}
	
	return ret;
}

BOOL CPInsp::IsAnyAngle(double angle)
{
	BOOL ret = FALSE;

	double temp = 0;
	temp = angle / 90.0;
	temp = angle - (int)temp * 90;

	if(temp != 0)
		ret = TRUE;

	return ret;
}

double CPInsp::CalcRotateAngle(double orgAngle)
{
	double retAngle = 0.0;

	double tempAngle = (orgAngle - (360.0 * (int)(orgAngle / 360.0)));
	if(tempAngle >= 0)
		retAngle = 360.0 - tempAngle;
	else
		retAngle = abs(tempAngle);

	return  retAngle;
}
void CPInsp::Binarize(const UCHAR *pByImgSrc, int nImgWidth, int nImgHeight, int nTypeRange, 
	int threshLow, int threshHigh, BOOL bInvert, UCHAR* &pByImgDst)
{
	if(pByImgDst)
	{
		//delete [] pByImgDst;
		g_pMManager->pem_delete(pByImgDst, true);
		pByImgDst = NULL;
	}
	if(!m_pProcMilAlgo || !pByImgSrc)
		return;

	Im::PIL_ID milSrc = M_NULL;
	milSrc = m_pProcMilAlgo->AllocBuff(nImgWidth, nImgHeight);
	Im::Buf::Put(milSrc, pByImgSrc);

	m_pProcMilAlgo->Binarize(milSrc, milSrc, nTypeRange, (double)threshLow, (double)threshHigh, bInvert);

	//pByImgDst = new UCHAR[nImgWidth * nImgHeight];
	pByImgDst = g_pMManager->pem_new<UCHAR>(true, nImgWidth * nImgHeight, (PCHAR)__FUNCTION__, __LINE__);
	Im::Buf::Get(milSrc, pByImgDst);

	m_pProcMilAlgo->FreeMilImageBuff(&milSrc);
}

void CPInsp::Binarize(const float *pfImgSrc, int nImgWidth, int nImgHeight, int nTypeRange, 
	float fThreshLow, float fThreshHigh, BOOL bInvert, UCHAR* &pByImgDst, BOOL bUseExceptHeight, float fExceptHeightMin, float fExceptHeightMax, bool bExceptZero)
{
	if(pByImgDst)
	{
		g_pMManager->pem_delete(pByImgDst, true);
		pByImgDst = NULL;
	}
	if(!m_pProcMilAlgo || !pfImgSrc)
		return;
	int nSizeImg = nImgHeight * nImgWidth;
	pByImgDst = g_pMManager->pem_new<UCHAR>(true, nSizeImg, (PCHAR)__FUNCTION__, __LINE__);
	memset(pByImgDst, 0, nSizeImg * sizeof(UCHAR));
	int64 milTypeRange = m_pProcMilAlgo->GetTypeRange(nTypeRange, bInvert);
	float fL = (bInvert && (milTypeRange == M_LESS_OR_EQUAL || milTypeRange == M_GREATER_OR_EQUAL)) ? fThreshHigh : fThreshLow;
	float fH = (bInvert && (milTypeRange == M_LESS_OR_EQUAL || milTypeRange == M_GREATER_OR_EQUAL)) ? fThreshLow : fThreshHigh;

	#pragma omp parallel for
	for (int i = 0; i < nSizeImg; ++i)
	{
		float fValue = pfImgSrc[i];
		if ((bExceptZero && fValue == 0.0f) ||
			(bUseExceptHeight && (fValue >= fExceptHeightMax || fValue <= fExceptHeightMin)))
			continue;
		if ((milTypeRange == M_IN_RANGE && (fL <= fValue && fValue <= fH)) ||
			(milTypeRange == M_OUT_RANGE && (fL >= fValue || fValue >= fH)) ||
			(milTypeRange == M_LESS_OR_EQUAL && fL >= fValue) ||
			(milTypeRange == M_GREATER_OR_EQUAL && fValue >= fH))
			pByImgDst[i] = 255;
	}
}

void CPInsp::GetGrayROI2D( UCHAR *pucImgSrc, UCHAR *pucImgDst, int nOrgWidth, RECT rcROISize )
{
	if((pucImgSrc == NULL) || (pucImgDst == NULL))
		return;
	if((rcROISize.bottom <= 0) || (rcROISize.top < 0) || (nOrgWidth <= 0))
		return;
	if((rcROISize.left < 0) || (rcROISize.right <= 0))
		return;

	int nWidth = rcROISize.right - rcROISize.left;
	int nHeight = rcROISize.bottom - rcROISize.top;
	if((nWidth <= 0) || (nHeight <= 0))
	{
		nWidth = 1;
		nHeight = 1;
	}

	IppiSize ipROISize = {nWidth, nHeight};
	int offX = rcROISize.left;
	int offY = rcROISize.top;

	ippiCopy_8u_C1R(pucImgSrc+offX+(offY*nOrgWidth), nOrgWidth, pucImgDst, nWidth, ipROISize);
}

void CPInsp::GetHeighROI3D( float *pfImgSrc, float *pfImgDst, int nOrgWidth, RECT rcROISize )
{
	if((pfImgSrc == NULL) || (pfImgDst == NULL))
		return;
	if((rcROISize.bottom <= 0) || (rcROISize.top < 0) || (nOrgWidth <= 0))
		return;
	if((rcROISize.left < 0) || (rcROISize.right <= 0))
		return;

	int nWidth = rcROISize.right - rcROISize.left;
	int nHeight = rcROISize.bottom - rcROISize.top;
	if((nWidth <= 0) || (nHeight <= 0))
	{
		nWidth = 1;
		nHeight = 1;
	}

	IppiSize ipROISize = {nWidth, nHeight};
	int offX = rcROISize.left;
	int offY = rcROISize.top;
	ippiCopy_32f_C1R(pfImgSrc+offX+offY*nOrgWidth, nOrgWidth*4, pfImgDst, nWidth*4, ipROISize);
}

void CPInsp::GetHeightMinMax( float *pfArrImgData, int nImgWidth, int nImgHeight, float *fHeightMin, float *fHeightMax)
{
	*fHeightMin = 0.0F;
	*fHeightMax = 0.0F;
	if(pfArrImgData == NULL || nImgWidth <= 0 || nImgHeight <= 0)
		return;

	ippsMinMax_32f(pfArrImgData, nImgWidth * nImgHeight, fHeightMin, fHeightMax);
}

float CPInsp::GetHeightMean( float *pfArrImgData, int nImgWidth, int nImgHeight, int nExceptCount, bool bOneMore)
{
	if(pfArrImgData == NULL)
		return 0.0f;

	int nTotal = nImgWidth * nImgHeight;
	if (nExceptCount < 0) nExceptCount = 0;
	if (nExceptCount >= nTotal)
		return 0.0f;

	float isum = 0.0f;
	double inf = std::numeric_limits<double>::infinity();
	int nCnt = 0;
	for (int a = 0; a < nTotal; a++)
	{
		if (_isnan(pfArrImgData[a]) || pfArrImgData[a] == inf || pfArrImgData[a] == inf * -1.0)
			continue;
		isum += pfArrImgData[a];
		nCnt++;
	}
	float fReturnRate = 0;
	if (nCnt > 0 && nExceptCount >= 0 && nCnt - nExceptCount > 0)
		fReturnRate = isum / (nCnt - nExceptCount);

	if (bOneMore)
	{
		isum = 0.0f;
		inf = std::numeric_limits<double>::infinity();
		nCnt = 0;
		for (int a = 0; a < nTotal; a++)
		{
			if (_isnan(pfArrImgData[a]) || pfArrImgData[a] == inf || pfArrImgData[a] == inf * -1.0)
				continue;

			if (fReturnRate > pfArrImgData[a])
				continue;

			isum += pfArrImgData[a];
			nCnt++;
		}

		fReturnRate = 0;
		if (nCnt > 0 && nExceptCount >= 0 && nCnt - nExceptCount > 0)
			fReturnRate = isum / (nCnt - nExceptCount);
	}

	return fReturnRate;
}

float CPInsp::GetHeightAvgMask(float *pfArrImgData, UCHAR * ucMask, int nImgWidth, int nImgHeight, int nStX, int nStY, int roiSizeX, int roiSizeY)
{
	if (pfArrImgData == NULL)
		return 0.0f;

	int nCnt = 0;

	int nEdX = __min(nStX + roiSizeX, nImgWidth);
	int nEdY = __min(nStY + roiSizeY, nImgHeight);

	float isum = 0.0f;
	double inf = std::numeric_limits<double>::infinity();
	for (int r = nStY; r < nEdY; r++)
	{
		for (int c = nStX; c < nEdX; c++)
		{
			if (ucMask[r * nImgWidth + c] != 0)
			{
				isum += pfArrImgData[r*nImgWidth + c];
				nCnt++;
			}
		}
	}
	
	if(nCnt != 0)
		isum = isum / nCnt;

	return isum;
}

float CPInsp::GetMinMaxImg( float *pfArrImgData, int nImgWidth, int nImgHeight,int Min)
{
	float fReturnRate = 0.0F;
	if(pfArrImgData == NULL)
		return fReturnRate;

	Ipp64f Mean;
	IppiSize roiSize = {nImgWidth, nImgHeight};
	Ipp32f rst;

	switch(Min)
	{
	case 1:
		ippiMin_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &rst);
		fReturnRate = rst;
		break;
	case 2:
		ippiMax_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &rst);
		fReturnRate = rst;
		break;
	case 0:
	default:
		ippiMean_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &Mean, ippAlgHintNone);
		fReturnRate = Mean;
		break;
	}

	return fReturnRate;
}
#include "HistogramAnalysis_New.h"
float CPInsp::GetHeightMaxHist( float *pfArrImgData, int nImgWidth, int nImgHeight )
{
	float fReturnRate = 0.0F;
	if(pfArrImgData == NULL)
		return fReturnRate;

	float fMaxH = 0.0F;
	float fMinH = 0.0F;
	IppiSize roiSize = {nImgWidth, nImgHeight};
	Ipp32f rst;
	ippiMin_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &rst);
	fMinH = rst;
	ippiMax_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &rst);
	fMaxH = rst;

	int diff = fMaxH - fMinH;
	std::vector<int> Hist;
	Hist.assign(diff+1,0.0f);
	for(int r=0;r<nImgHeight;r++)
	{
		for(int c=0;c<nImgWidth;c++)
		{
			float fHeight = pfArrImgData[r*nImgWidth+c];
			Hist[fHeight - fMinH]++;
		}
	}

	int MaxVal = 0;
	float MaxHight = 0.0f;
	for(int i=0;i<Hist.size();i++)
	{
		if(MaxVal<Hist[i])
		{
			MaxVal = Hist[i];
			MaxHight = i+fMinH;
		}

	}
	fReturnRate = MaxHight;

	double dMIn = fReturnRate-10;
	double dMax = fReturnRate+10;

	double dSum = 0.0f;
	int nCnt(0);

	for(int r=0;r<nImgHeight;r++)
	{
		for(int c=0;c<nImgWidth;c++)
		{
			if(pfArrImgData[r*nImgWidth+c]>dMIn && pfArrImgData[r*nImgWidth+c]<dMax)
			{
				dSum += pfArrImgData[r*nImgWidth+c];
				nCnt++;
			}
		}
	}
	fReturnRate = dSum/nCnt;
// 	CHistogramAnalysis_New ha;
// 	cv::Mat hData(1, Hist.size(), CV_32SC1, &Hist[0]);
// 	ha.Run(hData);


	return fReturnRate;
}
float CPInsp::GetHeightMaxHist2( float *pfArrImgData, int nImgWidth, int nImgHeight )
{
	float fReturnRate = 0.0F;
	if(pfArrImgData == NULL)
		return fReturnRate;

	float fMaxH = 0.0F;
	float fMinH = 0.0F;
	IppiSize roiSize = {nImgWidth, nImgHeight};
	Ipp32f rst;
	ippiMin_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &rst);
	fMinH = rst;
	ippiMax_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &rst);
	fMaxH = rst;

	int diff = fMaxH - fMinH;
	std::vector<float> Hist;
	Hist.assign(diff+1,0.0f);
	for(int r=0;r<nImgHeight;r++)
	{
		for(int c=0;c<nImgWidth;c++)
		{
			float fHeight = pfArrImgData[r*nImgWidth+c];
			Hist[fHeight - fMinH]++;
		}
	}

	int MaxVal = 0;
	float MaxHight = 0.0f;
	for(int i=0;i<Hist.size();i++)
	{
		if(MaxVal<Hist[i])
		{
			MaxVal = Hist[i];
			MaxHight = i+fMinH;
		}

	}
	fMinH = MaxHight;
	fMinH -= 20;
	fMaxH = MaxHight;
	fMinH += 20;
	double dHeightSum(0.0f);
	int nHeightCnt(0);
	for(int r=1;r<nImgHeight;r++)
	{
		for(int c=1;c<nImgWidth;c++)
		{
			float fDiffX = std::abs(pfArrImgData[r*nImgWidth+c]-pfArrImgData[r*nImgWidth+c-1]);
			float fDiffY = std::abs(pfArrImgData[r*nImgWidth+c]-pfArrImgData[(r-1)*nImgWidth+c]);
			if(fDiffX<20 &&fDiffY<20 && fMaxH < pfArrImgData[r*nImgWidth+c])
			{
				dHeightSum += pfArrImgData[r*nImgWidth+c];
				nHeightCnt++;
			}
		}
	}
#if _DEBUG
	cv::Mat DiffImg(nImgHeight,nImgWidth,CV_8UC1);
	DiffImg.setTo(0);
	for(int r=0;r<nImgHeight;r++)
	{
		UCHAR* imPtr = DiffImg.ptr(r);
		for(int c=1;c<nImgWidth;c++)
		{
			float fDiff = std::abs(pfArrImgData[r*nImgWidth+c]-pfArrImgData[r*nImgWidth+c-1]);
			if(fDiff<20 && fMaxH < pfArrImgData[r*nImgWidth+c])
			{
				imPtr[c] = 255;
			}
		}
	}
#endif

	fReturnRate = dHeightSum/nHeightCnt;

	return fReturnRate;
}

float CPInsp::GetHeightAvgMask( float *pfArrImgData, int nImgWidth, int nImgHeight ,UCHAR* BinImg)
{
	float fReturnRate = 0.0F;
	if(pfArrImgData == NULL)
		return fReturnRate;

	double dSum = 0.0f;
	int nCnt(0);

	for(int r=0;r<nImgHeight;r++)
	{
		for(int c=0;c<nImgWidth;c++)
		{
			if(BinImg[r * nImgWidth + c]!=0)
			{
				dSum += pfArrImgData[r*nImgWidth+c];
				nCnt++;
			}
		}
	}
	if(nCnt == 0)
		return -HUGE_VAL;
	fReturnRate = dSum/nCnt;


	return fReturnRate;
}

float CPInsp::GetHeightMean_rate( float *pfArrImgData, int nImgWidth, int nImgHeight, float stdMeanHeight, double rate)
{
	float fReturnRate = 0.0F;
	if(pfArrImgData == NULL || nImgWidth <= 0 || nImgHeight <= 0)
		return fReturnRate;

	int nROIWidth[4] = {0,};
	int nStartX[4] = {0,};
	int nROIHeight[4] = {0,};
	int nStartY[4] = {0,};
	int nEndX[4] = {0,};
	int nEndY[4] = {0,};

	int nExceptCnt = 0;
	float fSum = 0;

	float heightmn = stdMeanHeight*(1-rate/100);
	float heightmx = stdMeanHeight*(1+rate/100);

	for(int j = 0; j < nImgHeight; j++)
	{
		for(int i = 0; i < nImgWidth; i++)
		{
			if(pfArrImgData[(j * nImgWidth) + i]<heightmn || pfArrImgData[(j * nImgWidth) + i]>heightmx)
			{
				nExceptCnt++;
				continue;
			}

			fSum += pfArrImgData[(j * nImgWidth) + i];			
		}
	}

	int nTotalCnt = ((nImgWidth * nImgHeight) - nExceptCnt);

	if(nTotalCnt != 0)
		fReturnRate = fSum / nTotalCnt;
	else
		fReturnRate = 0.f;

	return fReturnRate;
}


double CPInsp::GetBWImageStruct(AlgoBaseBW &pInspAlgoBW, UCHAR *pucImgSrc, float *pfImgSrc, int nWidth, int nHeight, UCHAR*& ucArrDstImg, BOOL bUseColor, UCHAR* ucArrColorImg, int nDir, int nInspAreaPer)
{
	double dResult = 0.0;
	int nLine = __LINE__;
	try
	{
		UCHAR *pucInsp2DData = NULL;
		UCHAR *pucInsp3DData = NULL;
		UCHAR *ucArrTemp = NULL;
		double dDataBW = 0.0;
		int nArea = nWidth * nHeight;
		bool bNewTemp = false;
		if(ucArrDstImg)
			ucArrTemp = ucArrDstImg;
		else
		{
			ucArrTemp = g_pMManager->pem_new<UCHAR>(true, nArea, (PCHAR)__FUNCTION__, __LINE__);
			bNewTemp = true;
		}
		nLine = __LINE__;
		if(pInspAlgoBW.m_b2dCheck==FALSE && pInspAlgoBW.m_b3dCheck==FALSE && bUseColor==FALSE)
			return dResult;
		bool bIn = false;
		nLine = __LINE__;
		if ((pInspAlgoBW.m_b2dCheck == TRUE && (pucImgSrc == NULL || pInspAlgoBW.m_nMinValue < 0 || pInspAlgoBW.m_nMaxValue < 0)) ||
			(pInspAlgoBW.m_b3dCheck == TRUE && pfImgSrc == NULL) ||
			(bUseColor == TRUE && ucArrColorImg == NULL))
		{
			if (pucInsp2DData)
			{
				g_pMManager->pem_delete(pucInsp2DData, true);
				pucInsp2DData = NULL;
			}
			if (pucInsp3DData)
			{
				g_pMManager->pem_delete(pucInsp3DData, true);
				pucInsp3DData = NULL;
			}
			if (ucArrTemp && bNewTemp)
			{
				g_pMManager->pem_delete(ucArrTemp, true);
				ucArrTemp = NULL;
			}
			return dResult;
		}
		if(pInspAlgoBW.m_b2dCheck==TRUE)
		{
			m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nWidth, nHeight, _T("Origin2D.bmp"));
			nLine = __LINE__;
			CPInsp::Binarize(pucImgSrc, nWidth, nHeight, pInspAlgoBW.m_nRange, pInspAlgoBW.m_nMinValue, pInspAlgoBW.m_nMaxValue, pInspAlgoBW.m_bInvertCheck, pucInsp2DData);
			nLine = __LINE__;
			m_pProcMilAlgo->SaveWorkImg(pucInsp2DData, nWidth, nHeight, _T("Bin2D.bmp"));
			if (pInspAlgoBW.m_b3dCheck == FALSE && (bUseColor == FALSE || ucArrColorImg == NULL))
			{
				memcpy(ucArrTemp, pucInsp2DData, nArea * sizeof(UCHAR));
				bIn = true;
			}
		}
		nLine = __LINE__;
		if(pInspAlgoBW.m_b3dCheck==TRUE)
		{
			m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("Origin3D.bmp"));
			nLine = __LINE__;
			CPInsp::Binarize(pfImgSrc, nWidth, nHeight, pInspAlgoBW.m_n3dRange, pInspAlgoBW.m_d3dHeightMin, pInspAlgoBW.m_d3dHeightMax, pInspAlgoBW.m_bInvertCheck, pucInsp3DData);
			nLine = __LINE__;
			m_pProcMilAlgo->SaveWorkImg(pucInsp3DData, nWidth, nHeight, _T("Bin3D.bmp"));
			if (pInspAlgoBW.m_b2dCheck == FALSE && (bUseColor == FALSE || ucArrColorImg == NULL))
			{
				memcpy(ucArrTemp, pucInsp3DData, nArea * sizeof(UCHAR));
				bIn = true;
			}
		}
		nLine = __LINE__;
		if(bUseColor == TRUE)
		{
			if (pInspAlgoBW.m_b2dCheck == FALSE && pInspAlgoBW.m_b3dCheck == FALSE)
			{
				memcpy(ucArrTemp, ucArrColorImg, nArea * sizeof(UCHAR));
				bIn = true;
			}
		}
		nLine = __LINE__;
		if (bIn == false)
		{
			memset(ucArrTemp, 0, nArea * sizeof(UCHAR));
			for (int nIndex = 0; nIndex < nArea; nIndex++)
			{
				UCHAR uc2dValue = (pInspAlgoBW.m_b2dCheck == TRUE && pucInsp2DData) ? pucInsp2DData[nIndex] : 255;
				UCHAR uc3dValue = (pInspAlgoBW.m_b3dCheck == TRUE && pucInsp3DData) ? pucInsp3DData[nIndex] : 255;
				UCHAR ucColorValue = (bUseColor == TRUE && ucArrColorImg) ? ucArrColorImg[nIndex] : 255;
				if ((pInspAlgoBW.m_bInvertCheck == TRUE && (uc2dValue == 255 || uc3dValue == 255 || ucColorValue == 255)) ||
					(pInspAlgoBW.m_bInvertCheck == FALSE && (uc2dValue == 255 && uc3dValue == 255 && ucColorValue == 255)))
				{
					ucArrTemp[nIndex] = 255;
					dDataBW += 1.0;
				}
			}
		}
		else
			dDataBW = std::count(ucArrTemp, ucArrTemp + nArea, 255);
		if (ucArrDstImg)
			memcpy(ucArrDstImg, ucArrTemp, nArea * sizeof(UCHAR));
		nLine = __LINE__;
		m_pProcMilAlgo->SaveWorkImg(ucArrTemp, nWidth, nHeight, _T("Bin2D3D.jpg"));		//jpg 읽어오기
		if(nDir >= 0 && nDir <= 3 && nInspAreaPer < 100)
		{
			int nVal = 0;
			dDataBW = CalcBWInspectArea(ucArrTemp, nWidth, nHeight, nDir, nInspAreaPer, nVal);
		}
		if(pucInsp2DData)
		{
			g_pMManager->pem_delete(pucInsp2DData, true);
			pucInsp2DData = NULL;
		}
		if(pucInsp3DData)
		{
			g_pMManager->pem_delete(pucInsp3DData, true);
			pucInsp3DData = NULL;
		}
		if(ucArrTemp && bNewTemp)
		{
			g_pMManager->pem_delete(ucArrTemp, true);
			ucArrTemp = NULL;
		}
		nLine = __LINE__;
		if(dDataBW > 0 && nArea > 0)
			dResult = (dDataBW / (double)nArea) * 100.0;
	}
	catch(...)
	{
		if(g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp::GetBWImageStruct(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
	return dResult;
}

double CPInsp::CalcBWInspectArea(UCHAR *pucBWSrc, int &nImgW, int &nImgH, int nDir, int nInspectionAreaPer, int &pnROIValue)
{
	double dReturn = 0;
	if(nInspectionAreaPer < 0)
		nInspectionAreaPer = 1;
	if(nInspectionAreaPer > 100)
		nInspectionAreaPer = 100;
	int nOrgW = nImgW;
	int nOrgH = nImgH;
	int nW = nImgW;
	int nH = nImgH;
	int nX = 0;
	int nY = 0;
	int nW2 = nImgW;
	int nH2 = nImgH;
	if(nDir == 0 || nDir == 1)
	{
		nW = nImgW * (nInspectionAreaPer / 100.0);
		nW2 = nImgW * (nInspectionAreaPer / 100.0);
		if(nDir == 0)
		{
			nX = nImgW - nW;
			nW2 = nImgW;
		}
	}
	else
	{
		nH = nImgH * (nInspectionAreaPer / 100.0);
		nH2 = nImgH * (nInspectionAreaPer / 100.0);
		if(nDir == 2)
		{
			nY = nImgH - nH;
			nH2 = nImgH;
		}
	}
	for(int x = 0; x < nOrgW; x++)
	{
		for(int y = 0; y < nOrgH; y++)
		{
			int nIndex = (y * nOrgW) + x;
			if (x >= nX && x < nW2 && y >= nY && y < nH2)
			{
				if(pucBWSrc[nIndex] == 255)
					dReturn += 1.0;
				if(pucBWSrc[nIndex] == 1)
					pnROIValue += 1;
			}
			else
				pucBWSrc[nIndex] = 0;
		}
	}
	nImgW = nW;
	nImgH = nH;
	return dReturn;
}
void CPInsp::GetInspectArea(UCHAR *pucSrc, int nImgW, int nImgH, int nDir, int nInspectionAreaPer, int nBand)
{
	if (pucSrc == NULL || nImgW <= 0 || nImgH <= 0)
		return;
	if(nInspectionAreaPer < 0)		nInspectionAreaPer = 1;
	if(nInspectionAreaPer > 100)	nInspectionAreaPer = 100;
	int nX = 0;
	int nY = 0;
	int nW = nImgW;
	int nH = nImgH;
	if(nDir == 0 || nDir == 1)
	{
		nW = nImgW * (nInspectionAreaPer / 100.0);
		if(nDir == 0)
		{
			nX = nImgW - (nImgW * (nInspectionAreaPer / 100.0));
			nW = nImgW;
		}
	}
	else
	{
		nH = nImgH * (nInspectionAreaPer / 100.0);
		if(nDir == 2)
		{
			nY = nImgH - (nImgH * (nInspectionAreaPer / 100.0));
			nH = nImgH;
		}
	}
	for(int x = 0; x < nImgW; x++)
	{
		for(int y = 0; y < nImgH; y++)
		{
			int nIndex = (y * nImgW) + x;
			if (x >= nX && x < nW && y >= nY && y < nH)
				continue;
			else
			{
				if (nBand == 3)
				{
					pucSrc[(nIndex * 3)] = 0;
					pucSrc[(nIndex * 3) + 1] = 0;
					pucSrc[(nIndex * 3) + 2] = 0;
				}
				else
					pucSrc[nIndex] = 0;
			}
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pucSrc, nImgW, nImgH, _T("GetInspectArea.bmp"), nBand);
}

double CPInsp::GetGradient(double *ptrnLineX, double *ptrnLineY, double nTotalCnt, double &pdA, double &pdB, bool bHorizon)
{
	double dGradient = 0.0;
	if(nTotalCnt == 0)
		return dGradient;
	// 1: -[Sig]xy + a[Sig]x^2 + b[Sig]x = 0
	// 2: -[Sig]y + a[Sig]x + bn = 0
	// n = nTotalCnt
	// a, b를 계산
	int nStartIndex = 0;
	double dSigmaXY = 0;
	double dSigmaXSquare = 0;

	double dSigmaX = 0;
	double dSigmaY = 0;
	while(nStartIndex < nTotalCnt)
	{
		double dXPos = ptrnLineX[nStartIndex];
		double dYPos = ptrnLineY[nStartIndex];
		//if(dXPos < 0 || dYPos < 0)
		//{
		//	nStartIndex++;
		//	continue;
		//}

		if(bHorizon == true)
		{
			dSigmaXY = dSigmaXY + (dXPos * dYPos);
			dSigmaXSquare = dSigmaXSquare + (dXPos * dXPos);
			dSigmaX = dSigmaX + dXPos;
			dSigmaY = dSigmaY + dYPos;
		}
		else
		{
			dSigmaXY = dSigmaXY + (dXPos * dYPos);
			dSigmaXSquare = dSigmaXSquare + (dYPos * dYPos);
			dSigmaX = dSigmaX + dYPos;
			dSigmaY = dSigmaY + dXPos;
		}

		nStartIndex++;
	}
// 	pdB = (nSigmaXY - nSigmaY) / (nSigmaX - nTotalCnt);
// 	pdA = (nSigmaXY + (pdB * nSigmaX)) / nSigmaXSquare;
	double dValue = (dSigmaX * dSigmaX) - (dSigmaXSquare * nTotalCnt);
	double dValue1 = (dSigmaX * dSigmaXY) - (dSigmaXSquare * dSigmaY);
	if(dValue != 0 && dValue1 != 0)
		pdB = dValue1 / dValue;
	else
		pdB = 0;
	dValue = dSigmaXSquare;
	dValue1 = dSigmaXY - (dSigmaX * pdB);
	if(dValue != 0 && dValue1 != 0)
		pdA = dValue1 / dValue;
	else
		pdA = 0;
	
	//Gradient = Atan(pnA)
	dGradient = atan((double)pdA) * 180 / PI;

	return dGradient;
}

POINTF CPInsp::GetIntersectionOfTwoStraightLines(double *ptrdLineDA, double *ptrdLineDB, BOOL *ptrbHorizon)
{
	POINTF poCrossLinePoint;
	// y = A1x + B1
	// x = A2y + B2
	// y = A1(A2y + B2) + B1
	// y = A1A2y + A1B2 + B1
	// (1 - A1A2)y = A1B2 + B1
	// y = (A1B2 + B1) / (1 - A1A2)
	double dA1 = ptrdLineDA[0];
	double dA2 = ptrdLineDA[1];
	double dB1 = ptrdLineDB[0];
	double dB2 = ptrdLineDB[1];
	if(ptrbHorizon[1] == TRUE)
	{
		dA2 = ptrdLineDA[0];
		dA1 = ptrdLineDA[1];
		dB2 = ptrdLineDB[0];
		dB1 = ptrdLineDB[1];
	}

	double dCrossLinePointY = ((dA1 * dB2) + dB1) / (1 - (dA1 * dA2));
	double dCrossLinePointX = (dA2 * dCrossLinePointY) + dB2;

	poCrossLinePoint.x = dCrossLinePointX;
	poCrossLinePoint.y = dCrossLinePointY;

	return poCrossLinePoint;
}

bool CPInsp::PtInPolygon(POINT pt, POINT *pts, int ptNum)
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

bool CPInsp::PtInPolygon(POINT pt, POINTF *pts, int ptNum)
{
	POINTF spt, ept;				// start, end point
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

int CPInsp::FillOutOfInspArea(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue, BOOL bConvertExceptROI,int nChanel)
{
	if(nUsedInspPolygon < 1 || ptArrInspPolygon == NULL || !ucArrSrcImg)
		return 0;
	BOOL bCheckIngFALSE = bConvertExceptROI ? TRUE : FALSE;

	/*POINT* ptDstPoint = new POINT[nUsedInspPolygon];
	POINTF* ptfDstPoint = new POINTF[nUsedInspPolygon];*/
	POINT* ptDstPoint = g_pMManager->pem_new<POINT>(true, nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);
	POINTF* ptfDstPoint = g_pMManager->pem_new<POINTF>(true, nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);
	if(dAngle == 90 || dAngle == 270)
	{
		dAngle -= 360;
		if(dAngle < 0)
			dAngle += 180;
	}
	if (dAngle != 0) 
	{
		for (int a = 0; a < nUsedInspPolygon; a++) 
		{
			AnglePosChange(dAngle, nWidth, nHeight, ptArrInspPolygon[a], &ptfDstPoint[a]);
			if (ptfDstPoint[a].x < 0)
				ptfDstPoint[a].x = 0;
			if (ptfDstPoint[a].y < 0)
				ptfDstPoint[a].y = 0;
			if (ptfDstPoint[a].x > nWidth)
				ptfDstPoint[a].x = nWidth;
			if (ptfDstPoint[a].y > nHeight)
				ptfDstPoint[a].y = nHeight;
			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}
	}
	else 
	{
		for (int a = 0; a < nUsedInspPolygon; a++) 
		{
			ptfDstPoint[a].x = ptArrInspPolygon[a].x + (nWidth / 2.);
			ptfDstPoint[a].y = ptArrInspPolygon[a].y + (nHeight / 2.);
			if (ptfDstPoint[a].x < 0)
				ptfDstPoint[a].x = 0;
			if (ptfDstPoint[a].y < 0)
				ptfDstPoint[a].y = 0;
			if (ptfDstPoint[a].x > nWidth)
				ptfDstPoint[a].x = nWidth;
			if (ptfDstPoint[a].y > nHeight)
				ptfDstPoint[a].y = nHeight;

			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}
	}
	int cnt = 0;

	//UCHAR* ReturnImg = new UCHAR[nWidth*nHeight];
	UCHAR* ReturnImg = g_pMManager->pem_new<UCHAR>(true, nWidth*nHeight*nChanel, (PCHAR)__FUNCTION__, __LINE__);
	//int *ZeroCnt = (int*)malloc(sizeof(int));   
	int *ZeroCnt = (int*)g_pMManager->pem_malloc(sizeof(int), (PCHAR)__FUNCTION__, __LINE__);
	*ZeroCnt = 0;
	int nColorType = nChanel==3?0:1;
	cv::Mat src =m_pProcMilAlgo->BytesToMat(ucArrSrcImg,nWidth,nHeight,nColorType);
	src = FillPolygon(src,nWidth,nHeight,ptDstPoint,bConvertExceptROI, nUsedInspPolygon, ZeroCnt, ucArrOverlapImg, nChanel);
	memcpy(ReturnImg,m_pProcMilAlgo->MatToBytes(src,ReturnImg),nWidth*nHeight* sizeof(byte)*nChanel);
	memcpy(ucArrSrcImg,ReturnImg,nWidth*nHeight* sizeof(byte)*nChanel);
	if(ptDstPoint)
		//delete[] ptDstPoint;
		g_pMManager->pem_delete(ptDstPoint, true);
	ptDstPoint = NULL;
	if(ptfDstPoint)
		//delete[] ptfDstPoint;
		g_pMManager->pem_delete(ptfDstPoint, true);
	ptfDstPoint = NULL;
	if(ReturnImg)
		//delete[] ReturnImg;
		g_pMManager->pem_delete(ReturnImg, true);
	ReturnImg = NULL;
	src.release();
	//free(ZeroCnt);
	g_pMManager->pem_free(ZeroCnt);
	return cnt;
}
int CPInsp::FillOutOfInspArea(int nWidth, int nHeight, double dAngle, float *fArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue, BOOL bConvertExceptROI, int nChanel)
{
	if(nUsedInspPolygon < 1 || ptArrInspPolygon == NULL || fArrSrcImg == NULL)
		return 0;
	/*POINT* ptDstPoint = new POINT[nUsedInspPolygon];
	POINTF* ptfDstPoint = new POINTF[nUsedInspPolygon];*/
	POINT* ptDstPoint = g_pMManager->pem_new<POINT>(true, nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);
	POINTF* ptfDstPoint = g_pMManager->pem_new<POINTF>(true, nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);
	for (int a = 0; a < nUsedInspPolygon; a++)
	{
		ptfDstPoint[a].x = ptArrInspPolygon[a].x + (nWidth/2.);
		ptfDstPoint[a].y = ptArrInspPolygon[a].y + (nHeight/2.);

		if(ptfDstPoint[a].x < 0)
			ptfDstPoint[a].x = 0;
		if(ptfDstPoint[a].y < 0)
			ptfDstPoint[a].y = 0;
		if(ptfDstPoint[a].x > nWidth)
			ptfDstPoint[a].x = nWidth;
		if(ptfDstPoint[a].y > nHeight)
			ptfDstPoint[a].y = nHeight;

		ptDstPoint[a].x = ptfDstPoint[a].x;
		ptDstPoint[a].y = ptfDstPoint[a].y;		
	}
	int cnt = 0;
	for(int i = 0; i < nHeight; i++)
	{
		for(int j = 0; j < nWidth; j++)
		{
			POINT cur;
			cur.x = j;
			cur.y = i;
			int nIDX = (i * nWidth) + j;
			if(ucArrOverlapImg[nIDX] == 0 && PtInPolygon(cur, ptDstPoint, nUsedInspPolygon) == bConvertExceptROI)
			{
				fArrSrcImg[nIDX] = nFillValue;
				if(ucArrOverlapImg) ucArrOverlapImg[nIDX] = 1;
				cnt++;
			}
		}
	}
	Delete_1DArray(&ptDstPoint);
	Delete_1DArray(&ptfDstPoint);
	return cnt;
}
//2020.10.21 shkim Color window polygon 영역, expt_polygon, expt_roi 추가
int CPInsp::FillOutOfInspAreaCombine_Color(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, TotalInspExceptArea stTieArea)
{
	// case 1 : window polygon 연산
	// case 2 : except polygon 연산
	// case 3 : ExceptROI 
	// CPInsp::FillMaskingROI(nImgWidth, nImgHeight, sWndAlgoImg.dAngle, pUcImgSrc, stTieArea.m_nUsedMaskingValue, stTieArea.m_rcArrMaskingROI, 0, pUcOverlapImg);
	int ExceptCnt = 0;
	BOOL bCheckIngFALSE = FALSE;
	CString str;

	if(!ucArrSrcImg)
		return 0;
	if (stTieArea.m_nUsedWndPolygon < 1 && stTieArea.m_nUsedInspPolygon < 1 && stTieArea.m_nUsedMaskingValue < 1)
		return 0;

	cv::Mat srcOverlapImg;
	cv::Mat OverlapImg;

	if(ucArrOverlapImg != NULL)
		srcOverlapImg =m_pProcMilAlgo->BytesToMat(ucArrOverlapImg,nWidth,nHeight,0);
	// case 1 
	if(!(stTieArea.m_nUsedWndPolygon < 1) && (stTieArea.m_ptArrWndPolygon != NULL))
	{
		FillOutOfInspArea(nWidth, nHeight, dAngle, ucArrSrcImg, ucArrOverlapImg, stTieArea.m_nUsedWndPolygon, stTieArea.m_ptArrWndPolygon, 0, FALSE,3);
	}
	else
	{
		if(ucArrOverlapImg != NULL)
		{
			bitwise_not(srcOverlapImg, srcOverlapImg);
			memcpy(ucArrOverlapImg,m_pProcMilAlgo->MatToBytes(srcOverlapImg,ucArrOverlapImg),nWidth*nHeight* sizeof(byte)*3);
		}
	}
	// case 2 
	if(!(stTieArea.m_nUsedInspPolygon < 1) && (stTieArea.m_ptArrInspPolygon != NULL))
	{
		FillOutOfInspArea(nWidth, nHeight, dAngle, ucArrSrcImg, ucArrOverlapImg, stTieArea.m_nUsedInspPolygon, stTieArea.m_ptArrInspPolygon, 0, true,3);
	}
	// case 3 
	if(!(stTieArea.m_nUsedMaskingValue < 1) && (stTieArea.m_rcArrMaskingROI != NULL))
	//if(!(stTieArea.m_nUsedMaskingValue < 1) && !(stTieArea.m_rcArrMaskingROI.size() < 1))
	{
		FillMaskingROIUsingOpenCV(nWidth, nHeight, 0, ucArrSrcImg, stTieArea.m_nUsedMaskingValue, stTieArea.m_rcArrMaskingROI, 0, ucArrOverlapImg,3);	   
	}
	if(ucArrOverlapImg != NULL)
	{		
		cv::Mat OvImg3ch =m_pProcMilAlgo->BytesToMat(ucArrOverlapImg,nWidth,nHeight,0);
		cv::cvtColor(OvImg3ch,OverlapImg,cv::COLOR_BGR2GRAY);
		int cnt = countNonZero(OverlapImg);

		if(cnt > 0)
			ExceptCnt = (nWidth * nHeight) - cnt;
	}

	srcOverlapImg.release();
	OverlapImg.release();

	return ExceptCnt;
}
// KIY : 2020/04/22 : 기존 window polygon 영역, expt_polygon, expt_roi 연산을 위해 call하던 부분 정리
int CPInsp::FillOutOfInspAreaCombine(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, TotalInspExceptArea stTieArea)
{
	// case 1 : window polygon 연산
	// case 2 : except polygon 연산
	// case 3 : ExceptROI 
	// CPInsp::FillMaskingROI(nImgWidth, nImgHeight, sWndAlgoImg.dAngle, pUcImgSrc, stTieArea.m_nUsedMaskingValue, stTieArea.m_rcArrMaskingROI, 0, pUcOverlapImg);
	if (!ucArrSrcImg)
		return 0;
	int ExceptCnt = 0;
	BOOL bCheckIngFALSE = FALSE;
	cv::Mat srcOverlapImg;
	cv::Mat OverlapImg;

	if(ucArrOverlapImg != NULL)
		srcOverlapImg =m_pProcMilAlgo->BytesToMat(ucArrOverlapImg,nWidth,nHeight,1);
	// case 1 
	if(!(stTieArea.m_nUsedWndPolygon < 1) && (stTieArea.m_ptArrWndPolygon != NULL))
		FillOutOfInspArea(nWidth, nHeight, dAngle, ucArrSrcImg, ucArrOverlapImg, stTieArea.m_nUsedWndPolygon, stTieArea.m_ptArrWndPolygon, 0, FALSE);
	else
	{
		if(ucArrOverlapImg != NULL)
		{
			bitwise_not(srcOverlapImg, srcOverlapImg);
			memcpy(ucArrOverlapImg,m_pProcMilAlgo->MatToBytes(srcOverlapImg,ucArrOverlapImg),nWidth*nHeight* sizeof(byte));
		}
	}
	// case 2 
	if(!(stTieArea.m_nUsedInspPolygon < 1) && (stTieArea.m_ptArrInspPolygon != NULL))
		FillOutOfInspArea(nWidth, nHeight, dAngle, ucArrSrcImg, ucArrOverlapImg, stTieArea.m_nUsedInspPolygon, stTieArea.m_ptArrInspPolygon, 0, true);
	// case 3 
	if(!(stTieArea.m_nUsedMaskingValue < 1) && (stTieArea.m_rcArrMaskingROI != NULL))
	//if(!(stTieArea.m_nUsedMaskingValue < 1) && !(stTieArea.m_rcArrMaskingROI.size() < 1))
		FillMaskingROIUsingOpenCV(nWidth, nHeight, 0, ucArrSrcImg, stTieArea.m_nUsedMaskingValue, stTieArea.m_rcArrMaskingROI, 0, ucArrOverlapImg);	   

	if(ucArrOverlapImg != NULL)
	{		
		OverlapImg =m_pProcMilAlgo->BytesToMat(ucArrOverlapImg,nWidth,nHeight,1);
		int cnt = countNonZero(OverlapImg);

		if(cnt > 0)
			ExceptCnt = (nWidth * nHeight) - cnt;
	}

	srcOverlapImg.release();
	OverlapImg.release();

	return ExceptCnt;
}
int CPInsp::FillMaskingROI(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue, UCHAR *ucArrOverlapImg)
{
	if(nUsedMaskingValue < 1)
		return 0 ;

	if(rcArrMaskingROI == NULL)
		return 0;

	UCHAR* pUcArrTemp = NULL;
	if(ucArrSrcImg)
		pUcArrTemp = ucArrSrcImg;
	else
		return 0;	
	
	BOOL bUseOverlapCnt = FALSE;
	if(ucArrOverlapImg != NULL)
		bUseOverlapCnt = TRUE;

	int cnt = 0;

	for (int a = 0; a < nUsedMaskingValue; a++)
	{
		RECT rcDstRect;
		
		if(AngleRectChange(dAngle, nWidth, nHeight, rcArrMaskingROI[a], &rcDstRect) == FALSE)
			continue;

		int nROIWidth = rcDstRect.right - rcDstRect.left;
		int nStartX = rcDstRect.left;
		if(nROIWidth <= 0)
			continue;

		int nROIHeight = rcDstRect.bottom - rcDstRect.top;
		int nStartY = rcDstRect.top;
		if(nROIHeight <= 0)
			continue;

		int nEndX = nROIWidth + nStartX;
		int nEndY = nROIHeight + nStartY;

		if(nEndX > nWidth)
			nEndX = nWidth;
		if(nEndY > nHeight)
			nEndY = nHeight;

		for (int y = nStartY; y < nEndY; y++)
		{
			for (int x = nStartX; x < nEndX; x++)
			{
				pUcArrTemp[(y * nWidth) + x] = nFillValue;
				if(ucArrOverlapImg)
					ucArrOverlapImg[(y * nWidth) + x] = nFillValue;			
				if(bUseOverlapCnt && ucArrOverlapImg[(y * nWidth) + x] == 0)
					cnt++;

				/*if(bUseOverlapCnt)
					ucArrOverlapImg[(y * nWidth) + x] = 1;*/
			}
		}
	}

	return cnt;
}
int CPInsp::FillMaskingROI(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const std::vector<RECT> rcArrMaskingROI, int nFillValue, UCHAR *ucArrOverlapImg)
{
	if (nUsedMaskingValue < 1)
		return 0;

	if (rcArrMaskingROI.size() < 1)
		return 0;

	UCHAR* pUcArrTemp = NULL;
	if (ucArrSrcImg)
		pUcArrTemp = ucArrSrcImg;
	else
		return 0;

	BOOL bUseOverlapCnt = FALSE;
	if (ucArrOverlapImg != NULL)
		bUseOverlapCnt = TRUE;

	int cnt = 0;

	for (int a = 0; a < nUsedMaskingValue; a++)
	{
		RECT rcDstRect;

		if (AngleRectChange(dAngle, nWidth, nHeight, rcArrMaskingROI[a], &rcDstRect) == FALSE)
			continue;

		int nROIWidth = rcDstRect.right - rcDstRect.left;
		int nStartX = rcDstRect.left;
		if (nROIWidth <= 0)
			continue;

		int nROIHeight = rcDstRect.bottom - rcDstRect.top;
		int nStartY = rcDstRect.top;
		if (nROIHeight <= 0)
			continue;

		int nEndX = nROIWidth + nStartX;
		int nEndY = nROIHeight + nStartY;

		if (nEndX > nWidth)
			nEndX = nWidth;
		if (nEndY > nHeight)
			nEndY = nHeight;

		for (int y = nStartY; y < nEndY; y++)
		{
			for (int x = nStartX; x < nEndX; x++)
			{
				pUcArrTemp[(y * nWidth) + x] = nFillValue;
				if (ucArrOverlapImg)
					ucArrOverlapImg[(y * nWidth) + x] = nFillValue;
				if (bUseOverlapCnt && ucArrOverlapImg[(y * nWidth) + x] == 0)
					cnt++;

				/*if(bUseOverlapCnt)
					ucArrOverlapImg[(y * nWidth) + x] = 1;*/
			}
		}
	}

	return cnt;
}
int CPInsp::FillMaskingROI(int nWidth, int nHeight, double dAngle, float *fArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue, UCHAR *ucArrOverlapImg)
{
	if(nUsedMaskingValue < 1 || rcArrMaskingROI == NULL || fArrSrcImg == NULL)
		return 0;
	int cnt = 0;
	for (int a = 0; a < nUsedMaskingValue; a++)
	{
		RECT rcDstRect;
		if(AngleRectChange(dAngle, nWidth, nHeight, rcArrMaskingROI[a], &rcDstRect) == FALSE)
			continue;
		int nStartX = rcDstRect.left;
		int nStartY = rcDstRect.top;
		int nROIWidth = rcDstRect.right - rcDstRect.left;
		int nROIHeight = rcDstRect.bottom - rcDstRect.top;
		if(nROIWidth <= 0 || nROIHeight <= 0)
			continue;
		int nEndX = nROIWidth + nStartX;
		int nEndY = nROIHeight + nStartY;
		if(nEndX > nWidth) nEndX = nWidth;
		if(nEndY > nHeight) nEndY = nHeight;
		for (int y = nStartY; y < nEndY; y++)
		{
			for (int x = nStartX; x < nEndX; x++)
			{
				int nIDX = (y * nWidth) + x;
				/*if (ucArrOverlapImg[nIDX] == 1)
					continue;*/
				fArrSrcImg[nIDX] = nFillValue;
				/*if(ucArrOverlapImg)
					ucArrOverlapImg[(y * nWidth) + x] = nFillValue;	*/	
				if(ucArrOverlapImg) ucArrOverlapImg[nIDX] = 1;
				cnt++;
			}
		}
	}
	return cnt;
}
int CPInsp::FillMaskingROI(int nWidth, int nHeight, double dAngle, float *fArrSrcImg, int nUsedMaskingValue, const std::vector<RECT> rcArrMaskingROI, int nFillValue, UCHAR *ucArrOverlapImg)
{
	if (nUsedMaskingValue < 1 || rcArrMaskingROI.size() < 1 || fArrSrcImg == NULL)
		return 0;
	int cnt = 0;
	for (int a = 0; a < nUsedMaskingValue; a++)
	{
		RECT rcDstRect;
		if (AngleRectChange(dAngle, nWidth, nHeight, rcArrMaskingROI[a], &rcDstRect) == FALSE)
			continue;
		int nStartX = rcDstRect.left;
		int nStartY = rcDstRect.top;
		int nROIWidth = rcDstRect.right - rcDstRect.left;
		int nROIHeight = rcDstRect.bottom - rcDstRect.top;
		if (nROIWidth <= 0 || nROIHeight <= 0)
			continue;
		int nEndX = nROIWidth + nStartX;
		int nEndY = nROIHeight + nStartY;
		if (nEndX > nWidth) nEndX = nWidth;
		if (nEndY > nHeight) nEndY = nHeight;
		for (int y = nStartY; y < nEndY; y++)
		{
			for (int x = nStartX; x < nEndX; x++)
			{
				int nIDX = (y * nWidth) + x;
				/*if (ucArrOverlapImg[nIDX] == 1)
					continue;*/
				fArrSrcImg[nIDX] = nFillValue;
				/*if(ucArrOverlapImg)
					ucArrOverlapImg[(y * nWidth) + x] = nFillValue;	*/
				if (ucArrOverlapImg) ucArrOverlapImg[nIDX] = 1;
				cnt++;
			}
		}
	}
	return cnt;
}
int CPInsp::FillMaskingROIUsingOpenCV(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue, UCHAR *ucArrOverlapImg, int nChanel)
{
	if(nUsedMaskingValue < 1 || rcArrMaskingROI == NULL || !ucArrSrcImg)
		return 0 ;
	UCHAR* pUcArrTemp = NULL;
	BOOL bUseOverlapCnt = ucArrOverlapImg != NULL ? TRUE : FALSE;
	int cnt = 0;
	int nUsedPointCnt = nUsedMaskingValue*4;
	
	/*POINT* ptDstPoint = new POINT[nUsedPointCnt];
	POINT* ROIPoint = new POINT[nUsedPointCnt];
	POINTF* ptfDstPoint = new POINTF[nUsedPointCnt];*/
	POINT* ptDstPoint = g_pMManager->pem_new<POINT>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);
	POINT* ROIPoint = g_pMManager->pem_new<POINT>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);
	POINTF* ptfDstPoint = g_pMManager->pem_new<POINTF>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);

	if(dAngle == 90 || dAngle == 270)
	{
		dAngle -= 360;
		if(dAngle<0)
			dAngle +=180;
	}

	for (int a = 0; a < nUsedMaskingValue; a ++)
	{
		int nROIWidth = rcArrMaskingROI[a].right - rcArrMaskingROI[a].left;
		if (nROIWidth <= 0)
			continue;
		int nROIHeight = rcArrMaskingROI[a].bottom - rcArrMaskingROI[a].top;
		if (nROIHeight <= 0)
			continue;
		int nStartX = rcArrMaskingROI[a].left;
		int nStartY = rcArrMaskingROI[a].top;
		int index = a * 4;
		ROIPoint[index].x = nStartX;
		ROIPoint[index++].y = nStartY;
		ROIPoint[index].x = nStartX + nROIWidth;
		ROIPoint[index++].y = nStartY;
		ROIPoint[index].x = nStartX + nROIWidth;
		ROIPoint[index++].y = nStartY + nROIHeight;
		ROIPoint[index].x = nStartX;
		ROIPoint[index++].y = nStartY + nROIHeight;
	}
	

	for (int a = 0; a < nUsedPointCnt; a++)
	{
		ptfDstPoint[a].x = ROIPoint[a].x;
		ptfDstPoint[a].y = ROIPoint[a].y;
		if(dAngle!=0)
		{			
			AnglePosChange(dAngle, nWidth, nHeight, ptfDstPoint[a], &ptfDstPoint[a]);
			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}
		else
		{
			ptDstPoint[a].x = ptfDstPoint[a].x + (nWidth/2.);
			ptDstPoint[a].y = ptfDstPoint[a].y + (nHeight/2.);
		}
		if(ptDstPoint[a].x < 0)
			ptDstPoint[a].x = 0;
		if(ptDstPoint[a].y < 0)
			ptDstPoint[a].y = 0;
		if(ptDstPoint[a].x > nWidth)
			ptDstPoint[a].x = nWidth;
		if(ptDstPoint[a].y > nHeight)
			ptDstPoint[a].y = nHeight;
	}

	//UCHAR* ReturnImg = new UCHAR[nWidth*nHeight];
	UCHAR* ReturnImg = g_pMManager->pem_new<UCHAR>(true, nWidth*nHeight*nChanel, (PCHAR)__FUNCTION__, __LINE__);
	//int *ZeroCnt = (int*)malloc(sizeof(int));
	int *ZeroCnt = (int*)g_pMManager->pem_malloc(sizeof(int), (PCHAR)__FUNCTION__, __LINE__);
	*ZeroCnt = 1;

	int nColorTye = nChanel==3?0:1;
	cv::Mat src = m_pProcMilAlgo->BytesToMat(ucArrSrcImg,nWidth,nHeight,nColorTye);
	src = FillPolygon(src,nWidth,nHeight,ptDstPoint,true, nUsedPointCnt, ZeroCnt, ucArrOverlapImg,nChanel);
	memcpy(ucArrSrcImg,m_pProcMilAlgo->MatToBytes(src,ReturnImg),nWidth*nHeight* sizeof(byte)*nChanel);

	cnt = *ZeroCnt;

	if(ReturnImg)
		g_pMManager->pem_delete(ReturnImg, true);
	ReturnImg = NULL;
	if (ptDstPoint)
		g_pMManager->pem_delete(ptDstPoint, true);
	ptDstPoint = NULL;
	if (ROIPoint)
		g_pMManager->pem_delete(ROIPoint, true);
	ROIPoint = NULL;
	if (ptfDstPoint)
		g_pMManager->pem_delete(ptfDstPoint, true);
	ptfDstPoint = NULL;

	src.release();
	g_pMManager->pem_free(ZeroCnt);
	ZeroCnt = NULL;

	return cnt;
}
int CPInsp::FillMaskingROIUsingOpenCV(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const std::vector<RECT>rcArrMaskingROI, int nFillValue, UCHAR *ucArrOverlapImg, int nChanel)
{
	if (nUsedMaskingValue < 1 || rcArrMaskingROI.size() < 1 || !ucArrSrcImg)
		return 0;
	UCHAR* pUcArrTemp = NULL;
	BOOL bUseOverlapCnt = ucArrOverlapImg != NULL ? TRUE : FALSE;
	int cnt = 0;
	int nUsedPointCnt = nUsedMaskingValue * 4;

	/*POINT* ptDstPoint = new POINT[nUsedPointCnt];
	POINT* ROIPoint = new POINT[nUsedPointCnt];
	POINTF* ptfDstPoint = new POINTF[nUsedPointCnt];*/
	POINT* ptDstPoint = g_pMManager->pem_new<POINT>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);
	POINT* ROIPoint = g_pMManager->pem_new<POINT>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);
	POINTF* ptfDstPoint = g_pMManager->pem_new<POINTF>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);

	if (dAngle == 90 || dAngle == 270)
	{
		dAngle -= 360;
		if (dAngle < 0)
			dAngle += 180;
	}

	for (int a = 0; a < nUsedMaskingValue; a++)
	{
		int nROIWidth = rcArrMaskingROI[a].right - rcArrMaskingROI[a].left;
		if (nROIWidth <= 0)
			continue;
		int nROIHeight = rcArrMaskingROI[a].bottom - rcArrMaskingROI[a].top;
		if (nROIHeight <= 0)
			continue;
		int nStartX = rcArrMaskingROI[a].left;
		int nStartY = rcArrMaskingROI[a].top;
		int index = a * 4;
		ROIPoint[index].x = nStartX;
		ROIPoint[index++].y = nStartY;
		ROIPoint[index].x = nStartX + nROIWidth;
		ROIPoint[index++].y = nStartY;
		ROIPoint[index].x = nStartX + nROIWidth;
		ROIPoint[index++].y = nStartY + nROIHeight;
		ROIPoint[index].x = nStartX;
		ROIPoint[index++].y = nStartY + nROIHeight;
	}


	for (int a = 0; a < nUsedPointCnt; a++)
	{
		ptfDstPoint[a].x = ROIPoint[a].x;
		ptfDstPoint[a].y = ROIPoint[a].y;
		if (dAngle != 0)
		{
			AnglePosChange(dAngle, nWidth, nHeight, ptfDstPoint[a], &ptfDstPoint[a]);
			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}
		else
		{
			ptDstPoint[a].x = ptfDstPoint[a].x + (nWidth / 2.);
			ptDstPoint[a].y = ptfDstPoint[a].y + (nHeight / 2.);
		}
		if (ptDstPoint[a].x < 0)
			ptDstPoint[a].x = 0;
		if (ptDstPoint[a].y < 0)
			ptDstPoint[a].y = 0;
		if (ptDstPoint[a].x > nWidth)
			ptDstPoint[a].x = nWidth;
		if (ptDstPoint[a].y > nHeight)
			ptDstPoint[a].y = nHeight;
	}

	//UCHAR* ReturnImg = new UCHAR[nWidth*nHeight];
	UCHAR* ReturnImg = g_pMManager->pem_new<UCHAR>(true, nWidth*nHeight*nChanel, (PCHAR)__FUNCTION__, __LINE__);
	//int *ZeroCnt = (int*)malloc(sizeof(int));
	int *ZeroCnt = (int*)g_pMManager->pem_malloc(sizeof(int), (PCHAR)__FUNCTION__, __LINE__);
	*ZeroCnt = 1;

	int nColorTye = nChanel == 3 ? 0 : 1;
	cv::Mat src = m_pProcMilAlgo->BytesToMat(ucArrSrcImg, nWidth, nHeight, nColorTye);
	src = FillPolygon(src, nWidth, nHeight, ptDstPoint, true, nUsedPointCnt, ZeroCnt, ucArrOverlapImg, nChanel);
	memcpy(ucArrSrcImg, m_pProcMilAlgo->MatToBytes(src, ReturnImg), nWidth*nHeight * sizeof(byte)*nChanel);

	cnt = *ZeroCnt;

	if (ReturnImg)
		g_pMManager->pem_delete(ReturnImg, true);
	ReturnImg = NULL;
	if (ptDstPoint)
		g_pMManager->pem_delete(ptDstPoint, true);
	ptDstPoint = NULL;
	if (ROIPoint)
		g_pMManager->pem_delete(ROIPoint, true);
	ROIPoint = NULL;
	if (ptfDstPoint)
		g_pMManager->pem_delete(ptfDstPoint, true);
	ptfDstPoint = NULL;

	src.release();
	g_pMManager->pem_free(ZeroCnt);
	ZeroCnt = NULL;

	return cnt;
}


const int nBWFillValue = 1;

double CPInsp::GetBWImageStruct(AlgoBlackWhite &pInspAlgoBW, WndAlgoImg &sWndAlgo, UCHAR *ucArrDstImg, TotalInspExceptArea stTieArea, UCHAR* ucArrColorImg, int nDir, int nInspAreaPer, UCHAR *ucArrDstImgAI)
{
	double dResult = 0.0;
	int nLine = __LINE__;
	try
	{
		// 폴리곤 검사 영역 사용시에는 nisnpareaper 를 사용 할 수 없음.
		if(stTieArea.m_nUsedInspPolygon > 0)
			nInspAreaPer = 100;

		// 원래 이미지 2진화
		UCHAR *pucImgSrc	= sWndAlgo.m_ucArr2D;
		float *pfImgSrc		= sWndAlgo.m_fArr3D;
		int nWidth	= sWndAlgo.m_nWidth;
		int nHeight	= sWndAlgo.m_nHeight;
		double dAngle = sWndAlgo.dAngle;
		if(nWidth <= 0 || nHeight <= 0)
			return dResult;
		if (sWndAlgo.m_nWidth3D > 0 && sWndAlgo.m_nWidth3D < nWidth)
			return dResult;
		if (sWndAlgo.m_nHeight3D > 0 && sWndAlgo.m_nHeight3D < nHeight)
			return dResult;
		nLine = __LINE__;
		UCHAR* pUcArrTemp = NULL;
		if(ucArrDstImg)
			pUcArrTemp = ucArrDstImg;
		else
			//pUcArrTemp = new UCHAR[nWidth * nHeight];
			pUcArrTemp = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
		nLine = __LINE__;

		if (ucArrDstImgAI)
		{
			double dDataBW = std::count(ucArrDstImgAI, ucArrDstImgAI + nWidth * nHeight, 255);
			if (ucArrDstImg)
				memcpy(ucArrDstImg, ucArrDstImgAI, nWidth * nHeight * sizeof(UCHAR));
			nLine = __LINE__;
			if (nDir >= 0 && nDir <= 3 && nInspAreaPer < 100)
			{
				int nVal = 0;
				dDataBW = CalcBWInspectArea(ucArrDstImgAI, nWidth, nHeight, nDir, nInspAreaPer, nVal);
			}
			nLine = __LINE__;
			if (dResult > 0 && nWidth * nHeight > 0)
				dResult = (dDataBW / (double)(nWidth * nHeight)) * 100.0;
		}
		else
			dResult = GetBWImageStruct(pInspAlgoBW, pucImgSrc, pfImgSrc, nWidth, nHeight, pUcArrTemp, pInspAlgoBW.m_sAlgoColorBase.m_bUseColor, ucArrColorImg, nDir, nInspAreaPer);
		nLine = __LINE__;
		int nOrgWhiteValue = 0;
		if(stTieArea.m_nUsedMaskingValue != 0 || stTieArea.m_nUsedInspPolygon != 0 || stTieArea.m_nUsedWndPolygon != 0 || pInspAlgoBW.m_bUseTeachingRate == TRUE)
		{
			nLine = __LINE__;

			UCHAR *ucArrOverlapImg = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
			memset(ucArrOverlapImg, 0, sizeof(UCHAR) * nWidth * nHeight);
			m_pProcMilAlgo->SaveWorkImg(ucArrOverlapImg, nWidth, nHeight, _T("Masking_Src.bmp"));

			// KIY 2020/04/23 : 폴리곤 통합
			int nRoiBWValueTemp = CPInsp::FillOutOfInspAreaCombine(sWndAlgo.m_nWidth, sWndAlgo.m_nHeight, 0, pUcArrTemp, ucArrOverlapImg, stTieArea);

			m_pProcMilAlgo->SaveWorkImg(ucArrOverlapImg, nWidth, nHeight, _T("Masking_Dst.bmp"));
			Delete_1DArray(&ucArrOverlapImg);

			cv::Mat src = m_pProcMilAlgo->BytesToMat(pUcArrTemp,nWidth,nHeight,1);
			nLine = __LINE__;
			m_pProcMilAlgo->SaveWorkImg(pUcArrTemp, nWidth, nHeight, _T("Masking_After.bmp"));
			int nRoiBWValue = 0;
			nOrgWhiteValue = 0;
			nLine = __LINE__;
			for(int y = 0; y < nHeight; y++)
			{
				for(int x = 0; x < nWidth; x++)
				{
					if(pUcArrTemp[y * nWidth + x] == nBWFillValue)
						nRoiBWValue += 1;
					else if(pUcArrTemp[y * nWidth + x] == 255)
						nOrgWhiteValue += 1;
				}
			}
			nLine = __LINE__;
			if(nDir >= 0 && nDir <= 3 && nInspAreaPer < 100)
			{
				nRoiBWValue = 0;
				nOrgWhiteValue = CalcBWInspectArea(pUcArrTemp, nWidth, nHeight, nDir, nInspAreaPer, nRoiBWValue);
			}
			pInspAlgoBW.m_dAreaCurrent = nOrgWhiteValue;
			if(nOrgWhiteValue > 0 && nWidth * nHeight > 0)
				dResult = ((double)nOrgWhiteValue / (double)((nWidth * nHeight) - nRoiBWValue - nRoiBWValueTemp)) * 100.0;
			else
				dResult = 0;
			nLine = __LINE__;
		}
		nLine = __LINE__;
		if(!ucArrDstImg && pUcArrTemp)
		{
			//delete pUcArrTemp;
			g_pMManager->pem_delete(pUcArrTemp, false);
			pUcArrTemp = NULL;
		}
	}
	catch(...)
	{
		if(g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp::GetBWImageStruct() _1, Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return dResult;
}

void CPInsp::GetBlobArea(UCHAR *pucImgSrc, int nSrcWidth, int nSrcHeight, double dSrcCx, double dSrcCy, double dBlobW, double dBlobH, double dWndAngle, double *rstW, double *rstH, int nOkPercent /*= 50*/)
{
	if(pucImgSrc == NULL || nSrcWidth <= 0 || nSrcHeight <= 0)
		return;
	if(nOkPercent <= 0)
		nOkPercent = 50;
	UCHAR * ucArrBlobClipDst = NULL;
	int nBlobClipW = nSrcWidth;
	int nBlobClipL = (double)nSrcHeight * 0.25;
	if(nBlobClipL <= 0)
		nBlobClipL = 1;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobClipDst, nBlobClipW * nBlobClipL);
	int nClipX = 0;
	int nClipY = (dSrcCy - nBlobClipL / 2.0);
	if(nClipY < 0)
		nClipY = 0;
	m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nSrcWidth, nSrcHeight, ucArrBlobClipDst, nClipX, nClipY, nBlobClipW, nBlobClipL);
	m_pProcMilAlgo->SaveWorkImg(ucArrBlobClipDst, nBlobClipW, nBlobClipL, _T("ClipBlob1.bmp"));

	// left X
	RECT rcClip = {0,0,0,0};
	double dWhiteValue = 0.;
	double dWhiteRate = 0.;
	int nStdWidth = nBlobClipW;
	int nStdHeight = nBlobClipL;
	for (int x = 0; x < nBlobClipW; x++)
	{
		dWhiteValue = 0.0;
		for(int y = 0; y < nBlobClipL; y++)
		{
			if(ucArrBlobClipDst[x + (y*nBlobClipW)] == 255)
				dWhiteValue += 1;
		}
		dWhiteRate = (dWhiteValue / (double)nStdHeight) * 100.0;
		if(dWhiteRate >= nOkPercent)
		{
			rcClip.left = x;	// top left
			break;
		}
	}
	for (int x = nBlobClipW-1; x >= 0; x--)
	{
		dWhiteValue = 0.0;
		for(int y = 0; y < nBlobClipL; y++)
		{
			if(ucArrBlobClipDst[x + (y*nBlobClipW)] == 255)
				dWhiteValue += 1;
		}
		dWhiteRate = (dWhiteValue / (double)nStdHeight) * 100.0;
		if(dWhiteRate > nOkPercent)
		{
			rcClip.right = x;	// top left
			break;
		}
	}
	Delete_1DArray(& ucArrBlobClipDst);
	UCHAR * ucArrBlobClipDst2 = NULL;
	nBlobClipW = (double)nSrcWidth * 0.25;
	if(nBlobClipW <= 0)
		nBlobClipW = 1;
	nBlobClipL = nSrcHeight;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucArrBlobClipDst2, nBlobClipW * nBlobClipL);
	nStdWidth = nBlobClipW;
	nStdHeight = nBlobClipL;
	nClipX = (dSrcCx - nBlobClipW / 2.0);
	nClipY = 0;
	if(nClipX < 0)
		nClipX = 0;
	m_pProcMilAlgo->GetClipImage_LT(pucImgSrc, nSrcWidth, nSrcHeight, ucArrBlobClipDst2, nClipX, nClipY, nBlobClipW, nBlobClipL);
	m_pProcMilAlgo->SaveWorkImg(ucArrBlobClipDst2, nBlobClipW, nBlobClipL, _T("ClipBlob2.bmp"));

	// top Y
	for (int y = 0; y < nBlobClipL; y++)
	{
		dWhiteValue = 0.0;
		for(int x = 0; x < nBlobClipW; x++)
		{
			if(ucArrBlobClipDst2[x + (y*nBlobClipW)] == 255)
				dWhiteValue += 1;
		}
		dWhiteRate = (dWhiteValue / (double)nStdWidth) * 100.0;
		if(dWhiteRate >= nOkPercent)
		{
			rcClip.top = y;	// top left
			break;
		}
	}

	for (int y = nBlobClipL-1; y >= 0; y--)
	{
		dWhiteValue = 0.0;
		for(int x = 0; x < nBlobClipW; x++)
		{
			if(ucArrBlobClipDst2[x + (y*nBlobClipW)] == 255)
				dWhiteValue += 1;
		}
		dWhiteRate = (dWhiteValue / (double)nStdWidth) * 100.0;
		if(dWhiteRate >= nOkPercent)
		{
			rcClip.bottom = y;	// bottom left
			break;
		}
	}
	Delete_1DArray(& ucArrBlobClipDst2);
	//각도에따른 w와 h 변환..
	if(rcClip.left < 0)
		rcClip.left = 0;
	if(rcClip.right < 0)
		rcClip.right = 0;
	if(rcClip.top < 0)
		rcClip.top = 0;
	if(rcClip.bottom < 0)
		rcClip.bottom = 0;

	// 2020 07/16 bjy : 불량이 1픽셀로 검출되었는데 right - left = 0 해서 0.000mm나오는 문제때문에 수정
	double drstW, drstH;
	if (rcClip.right == 0 && rcClip.left == 0 && rcClip.top == 0 && rcClip.bottom == 0)
	{
		//불량 없음
		drstW = rcClip.right - rcClip.left;
		drstH = rcClip.bottom - rcClip.top;
	}
	else
	{
		//불량 있음
		drstW = rcClip.right - rcClip.left;
		drstH = rcClip.bottom - rcClip.top;
		if (drstH == 0) drstH++;
		if (drstW == 0) drstW++;
	}

	if(dBlobW > drstW)
		drstW = dBlobW;
	if(dBlobH > drstH)
		drstH = dBlobH;

	if(dWndAngle == 90 || dWndAngle == 270)
	{
		double dBuf = drstW;
		drstW = drstH;
		drstH = dBuf;
	}
	*rstW = drstW;
	*rstH = drstH;
}

int CPInsp::BlobImageStruct(AlgoBlob &algoBlob, UCHAR *pucImgSrc, float *pfImgSrc, UCHAR *ucArrColorImg, int nWidth, int nHeight, int nMinBlobArea, 
								double *dArea, double *dCx, double *dCy, CRect *rcBlob, UCHAR *ucArrDstImg, TotalInspExceptArea stTieArea,
								BOOL bApplyFillHole/*=TRUE*/, BOOL bAlignAlgo/*= FALSE*/, int nTeachX/* = -1*/, int nTeachY/* = -1*/, BOOL eraseBorderBlob, double dAreaPix, double dAreaWPix, double dAreaHPix, int nAreaCnt)
{
	if (nWidth <= 0 || nHeight <= 0)
		return 0;

	BOOL bInsp3D = algoBlob.m_bInsp3D;
	if (g_pMPTI->m_bSideOriginalSize == true)
		bInsp3D = FALSE;

	if (algoBlob.m_bInsp2D == FALSE && bInsp3D == FALSE && algoBlob.m_sAlgoColorBase.m_bUseColor == FALSE)
		return 0;

	if (algoBlob.m_bInsp2D)
	{
		if (pucImgSrc == NULL)
			return 0;
	}
	if (bInsp3D)
	{
		if (pfImgSrc == NULL)
			return 0;
	}

	int nArea = nWidth * nHeight;
	UCHAR *pUcImgDst2D = NULL;
	UCHAR *pUcImgDst3D = NULL;
	UCHAR *pUcImgBinary = g_pMManager->pem_new<UCHAR>(true, nArea, (PCHAR)__FUNCTION__, __LINE__);
	memset(pUcImgBinary, 0, nArea * sizeof(UCHAR));
#if _DEBUG
	//cv::Mat imgpUcImgDst2D(nHeight, nWidth, CV_8UC1, pucImgSrc);
	//cv::Mat imgpUcImgDst3D(nHeight, nWidth, CV_32FC1, pfImgSrc);
	//cv::Mat imgpUcImgBinary(nHeight, nWidth, CV_8UC1, pUcImgBinary);
	//cv::Mat imgucArrColorImg(nHeight, nWidth, CV_8UC1, ucArrColorImg);
#endif
	bool bIn = false;
	// 1. Binarize
	if(algoBlob.m_bInsp2D)
	{
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nWidth, nHeight, _T("Origin2D.bmp"));
		CPInsp::Binarize(pucImgSrc, nWidth, nHeight, algoBlob.m_nTypeRange2D, algoBlob.m_nMinBinary, algoBlob.m_nMaxBinary, algoBlob.m_bInvertCheck, pUcImgDst2D);
		m_pProcMilAlgo->SaveWorkImg(pUcImgDst2D, nWidth, nHeight, _T("Bin2D_Org.bmp"));
		if (bInsp3D == FALSE && ucArrColorImg == NULL)
		{
			memcpy(pUcImgBinary, pUcImgDst2D, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if(bInsp3D)
	{
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("Origin3D.bmp"));
		CPInsp::Binarize(pfImgSrc, nWidth, nHeight, algoBlob.m_nTypeRange3D, algoBlob.m_dHeightRateMin, algoBlob.m_dHeightRateMax, algoBlob.m_bInvertCheck, pUcImgDst3D);
		m_pProcMilAlgo->SaveWorkImg(pUcImgDst3D, nWidth, nHeight, _T("Bin3D.bmp"));
		if (algoBlob.m_bInsp2D == FALSE && ucArrColorImg == NULL)
		{
			memcpy(pUcImgBinary, pUcImgDst3D, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (ucArrColorImg != NULL)
	{
		m_pProcMilAlgo->SaveWorkImg(ucArrColorImg, nWidth, nHeight, _T("BinColor_Org.bmp"));
		if (algoBlob.m_bInsp2D == FALSE && bInsp3D == FALSE)
		{
			memcpy(pUcImgBinary, ucArrColorImg, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (bIn == false)
	{
		for (int nIndex = 0; nIndex < nArea; nIndex++)
		{
			UCHAR uc2dValue = (algoBlob.m_bInsp2D == TRUE && pUcImgDst2D) ? pUcImgDst2D[nIndex] : 255;
			UCHAR uc3dValue = (bInsp3D == TRUE && pUcImgDst3D) ? pUcImgDst3D[nIndex] : 255;
			UCHAR ucColorValue = (ucArrColorImg) ? ucArrColorImg[nIndex] : 255;
			if (uc2dValue == 255 && uc3dValue == 255 && ucColorValue == 255)
				pUcImgBinary[nIndex] = 255;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pUcImgBinary, nWidth, nHeight, _T("Bin2D3DColor.bmp"));
	if (pUcImgDst2D)
		Delete_1DArray(&pUcImgDst2D);
	if (pUcImgDst3D)
		Delete_1DArray(&pUcImgDst3D);	
	// KIY 2020/04/23 : 폴리곤 통합
	CPInsp::FillOutOfInspAreaCombine(nWidth, nHeight, 0, pUcImgBinary, NULL, stTieArea);
	// 2. Blob
	// 3. Select Blob
	int nFilter = (algoBlob.m_bFilterIsUse) ? algoBlob.m_nFilterStepNarrow : 0;
	bool bMultiMaxBlob = !algoBlob.m_bUseBlobNG && !algoBlob.m_bInspCoil && algoBlob.m_byNGCnt > 0;
	
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pUcImgBinary, ucArrDstImg, nWidth, nHeight, nMinBlobArea, eraseBorderBlob, bApplyFillHole, nFilter, algoBlob.m_nTypeSelectBlob, nTeachX, nTeachY, -1, dAreaPix, dAreaWPix, dAreaHPix, nAreaCnt, bMultiMaxBlob, algoBlob.m_byNGCnt);
	if (nCntBlob > 0)
	{
		m_pProcMilAlgo->GetBlobResult_Renewal(dArea, dCx, dCy, rcBlob, bAlignAlgo);
		if (ucArrDstImg)
			m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nWidth, nHeight, _T("Blob.bmp"));
	}

	// PYJ 2019/05/23
	double dCirle_x = 0, dCirle_y = 0;

	//blob Circle Fitting NYJ 2020/12/19
	double dErrRate = 0;
	if (ucArrDstImg != nullptr && algoBlob.m_bCircleOpt)
	{
		double dRadi = 0, area;
		CRect rcCircle = *rcBlob;
		cv::Mat cvSrcImg(nHeight, nWidth, CV_8UC1, ucArrDstImg);
		bool ret;
		//ret = BlobCircleCenter(cvSrcImg, dCirle_x, dCirle_y, &dRadi, &dErrRate, true);		//true = None
		ret = BlobCircleCenter_Fit(cvSrcImg, &rcCircle, dCirle_x, dCirle_y, &dRadi, &dErrRate, &area, 5, false, false, 0, algoBlob.m_nTypeSelectBlob);		//true = None
		if (ret == false) dErrRate = 1.;
		if (dErrRate < 0.5) *rcBlob = rcCircle;
	}

	if (ucArrDstImg != nullptr && algoBlob.m_bCircleOpt && dErrRate > 0.5)
	{
		CString sLog;
		cv::RotatedRect FiduRect;
		cv::Mat testImage( cv::Size(nWidth, nHeight), CV_8UC1,ucArrDstImg);
		cv::Mat Ecircle = testImage.clone();

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		cv::findContours(Ecircle,contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE );		//CHAIN_APPROX_NONE: apply all edge point

		if(contours.size()>0)
		{
			std::vector<cv::Point> vPTmp;// = contours[0];
			cv::Mat Points;
			cv::Mat conTemp;// = cv::Mat(vPTmp).clone();
			cv::Point pCenterPoint(0,0);
			cv::Size2f cicleSize(0,0);

			std::vector<cv::Point> selectedCon;
			for(int cCnt = 0;cCnt<contours.size();cCnt++)
			{
				if(contours[cCnt].size()<6)
					continue;

				if(contours[cCnt].size()<selectedCon.size())
					continue;
				selectedCon = contours[cCnt];
			}

			conTemp = cv::Mat(selectedCon);
			conTemp.convertTo(Points,CV_32F);
				
			try
			{
				if(selectedCon.size()>6)
					FiduRect = cv::fitEllipse(cv::Mat(Points));
			}
			catch (CMemoryException* e)
			{
				sLog.Format(_T("Func_InspAlignEdge, fitEllipse_CMemoryException_Points:%d_Points:%d"),Points.cols,Points.rows);
				g_pMPTI->AddLog_Dev(sLog);
					
			}
			catch (CFileException* e)
			{
				sLog.Format(_T("Func_InspAlignEdge, fitEllipse_CFileException_Points:%d_Points:%d"),Points.cols,Points.rows);
				g_pMPTI->AddLog_Dev(sLog);
			}
			catch (CException* e)
			{
				sLog.Format(_T("Func_InspAlignEdge, fitEllipse_CException_Points:%d_Points:%d"),Points.cols,Points.rows);
				g_pMPTI->AddLog_Dev(sLog);
			}
				
			//pCenterPoint.x = FiduRect.center.x;
			//pCenterPoint.y = FiduRect.center.y;
			//cicleSize.width = FiduRect.size.width;
			//cicleSize.height = FiduRect.size.height;
#if _DEBUG
			CString msg;
			msg.Format(_T("D:\\Blob%d.bmp"),0);
			cv::imwrite(std::string(CT2A(msg)),testImage);

			cv::Mat Drawcircle = testImage.clone();
			cv::ellipse(Drawcircle,FiduRect,cv::Scalar(126));
			msg.Format(_T("D:\\Blob%d_EllipseCircle.bmp"),0);
			cv::imwrite(std::string(CT2A(msg)),Drawcircle);

#endif
			dCirle_x = FiduRect.center.x;
			dCirle_y = FiduRect.center.y;
		}
	}

	// PYJ 2019/05/23
	if(algoBlob.m_bCircleOpt)
	{
		 *dCx = dCirle_x;
		 *dCy = dCirle_y;
	}

	if(pUcImgBinary)
	{
		//delete pUcImgBinary;

		// MemoryLeak Fix -wjlee
		//g_pMManager->pem_delete(pUcImgBinary, false);
		g_pMManager->pem_delete(pUcImgBinary, true);

		pUcImgBinary = NULL;
	}

	return nCntBlob;
}

void CPInsp::InitExceptMatchingMaps(int *pnExceptMatchingMaps, int nCntBlob)
{
	for(int i = 0; i< nCntBlob; i++)
		pnExceptMatchingMaps[i] = eNotExistSolderExcept;
}

void CPInsp::SetForeignResultRect(CRect *rcRectSource, CRect *rcRectDes, int nSrcCnt, int nDesCnt)
{								
	rcRectDes[nDesCnt].left = rcRectSource[nSrcCnt].left;
	rcRectDes[nDesCnt].top = rcRectSource[nSrcCnt].top;
	rcRectDes[nDesCnt].right = rcRectSource[nSrcCnt].right;
	rcRectDes[nDesCnt].bottom = rcRectSource[nSrcCnt].bottom;	
}

void CPInsp::SetForeignResultValue(double *dAreaSrc, double *dCxSrc,double *dCySrc, double *dAreaDes, double *dCxDes,double *dCyDes, int nSrcCnt, int nDesCnt)
{								
	dAreaDes[nDesCnt]= dAreaSrc[nSrcCnt];
	dCxDes[nDesCnt] = dCxSrc[nSrcCnt];
	dCyDes[nDesCnt] = dCySrc[nSrcCnt];

}

void CPInsp::SetForeignResultRect(CRect rcRectSource, CRect *rcRectDes, int nDesCnt)
{								
	rcRectDes[nDesCnt].left = rcRectSource.left;
	rcRectDes[nDesCnt].top = rcRectSource.top;
	rcRectDes[nDesCnt].right = rcRectSource.right;
	rcRectDes[nDesCnt].bottom = rcRectSource.bottom;	
}

void CPInsp::SetForeignResultValue(double dAreaSrc, double dCxSrc,double dCySrc, double *dAreaDes, double *dCxDes,double *dCyDes, int nDesCnt)
{								
	dAreaDes[nDesCnt] = dAreaSrc;
	dCxDes[nDesCnt] = dCxSrc;
	dCyDes[nDesCnt] = dCySrc;

}


int CPInsp::BlobImageStruct_BW(AlgoBaseBW &pInspAlgoBW, UCHAR *pucImgSrc, float *pfImgSrc, int nWidth, int nHeight, int nMinBlobArea, 
	double *dArea, double *dCx, double *dCy, CRect *rcBlob, UCHAR *ucArrDstImg, int nTypeSelectBlob, bool bFilterIsUse, int nFilterStepNarrow,
	TotalInspExceptArea stTieArea, BOOL bApplyFillHole, BOOL bAlignAlgo, UCHAR *ucArrColorImg)
{
	if ((pInspAlgoBW.m_b2dCheck == TRUE && pucImgSrc == NULL) || (pInspAlgoBW.m_b3dCheck == TRUE && pfImgSrc == NULL) ||
		nWidth <= 0 || nHeight <= 0 || (!pInspAlgoBW.m_b2dCheck && !pInspAlgoBW.m_b3dCheck && ucArrDstImg == NULL))
		return 0;
	int nArea = nWidth * nHeight;
	UCHAR *pUcImgDst2D = NULL;
	UCHAR *pUcImgDst3D = NULL;
	UCHAR *pUcImgBinary = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(pUcImgBinary, 0, nArea * sizeof(UCHAR));
	bool bIn = false;
	// 1. Binarize
	if(pInspAlgoBW.m_b2dCheck)
	{
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nWidth, nHeight, _T("Origin2D.bmp"));
		CPInsp::Binarize(pucImgSrc, nWidth, nHeight, pInspAlgoBW.m_nRange, 
			pInspAlgoBW.m_nMinValue, pInspAlgoBW.m_nMaxValue, pInspAlgoBW.m_bInvertCheck, pUcImgDst2D);
		m_pProcMilAlgo->SaveWorkImg(pUcImgDst2D, nWidth, nHeight, _T("Bin2D.bmp"));
		if (pInspAlgoBW.m_b3dCheck == FALSE && ucArrColorImg == NULL)
		{
			memcpy(pUcImgBinary, pUcImgDst2D, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if(pInspAlgoBW.m_b3dCheck)
	{
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("Origin3D.bmp"));
		CPInsp::Binarize(pfImgSrc, nWidth, nHeight, pInspAlgoBW.m_n3dRange, 
			pInspAlgoBW.m_d3dHeightMin, pInspAlgoBW.m_d3dHeightMax, pInspAlgoBW.m_bInvertCheck, pUcImgDst3D);
		m_pProcMilAlgo->SaveWorkImg(pUcImgDst3D, nWidth, nHeight, _T("Bin3D.bmp"));
		if (pInspAlgoBW.m_b2dCheck == FALSE && ucArrColorImg == NULL)
		{
			memcpy(pUcImgBinary, pUcImgDst3D, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (ucArrColorImg)
	{
		m_pProcMilAlgo->SaveWorkImg(ucArrColorImg, nWidth, nHeight, _T("OriginColor.bmp"));
		if (pInspAlgoBW.m_b2dCheck == FALSE && pInspAlgoBW.m_b3dCheck == FALSE)
		{
			memcpy(pUcImgBinary, ucArrColorImg, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (bIn == false)
	{
		for (int nIndex = 0; nIndex < nArea; nIndex++)
		{
			UCHAR uc2DVal = (pInspAlgoBW.m_b2dCheck && pUcImgDst2D) ? pUcImgDst2D[nIndex] : 255;
			UCHAR uc3DVal = (pInspAlgoBW.m_b3dCheck && pUcImgDst3D) ? pUcImgDst3D[nIndex] : 255;
			UCHAR ucColorVal = (ucArrColorImg) ? ucArrColorImg[nIndex] : 255;
			if (uc2DVal == 255 && uc3DVal == 255 && ucColorVal == 255)
				pUcImgBinary[nIndex] = 255;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pUcImgBinary, nWidth, nHeight, _T("Bin2D3D.bmp"));
	if (pUcImgDst2D)
	{
		g_pMManager->pem_delete(pUcImgDst2D, true);
		pUcImgDst2D = NULL;
	}
	if (pUcImgDst3D)
	{
		g_pMManager->pem_delete(pUcImgDst3D, true);
		pUcImgDst3D = NULL;
	}
	// KIY 2020/04/23 : 폴리곤 통합
	CPInsp::FillOutOfInspAreaCombine(nWidth, nHeight, 0, pUcImgBinary, NULL, stTieArea);
	// 2. Blob
	// 3. Select Blob
	int nFilter = bFilterIsUse ? nFilterStepNarrow : 0;
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pUcImgBinary, ucArrDstImg, nWidth, nHeight, nMinBlobArea, FALSE, bApplyFillHole, nFilter, nTypeSelectBlob);
	if(nCntBlob > 0)
	{
		m_pProcMilAlgo->GetBlobResult(dArea, dCx, dCy, rcBlob, bAlignAlgo);
		m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nWidth, nHeight, _T("Blob.bmp"));
	}
	if(pUcImgBinary)
	{
		g_pMManager->pem_delete(pUcImgBinary, true);
		pUcImgBinary = NULL;
	}
	return nCntBlob;
}

int CPInsp::BlobImageStruct_BW2(AlgoBaseBW &pInspAlgoBW, UCHAR *pucImgSrc, float *pfImgSrc, int nWidth, int nHeight, int nMinBlobArea,
	double *dArea, double *dCx, double *dCy, CRect *rcBlob, UCHAR *ucArrDstImg, int nTypeSelectBlob, bool bFilterIsUse, int nFilterStepNarrow,
	TotalInspExceptArea stTieArea, BOOL bApplyFillHole, int nDir, int nInspAreaPer, UCHAR *ucArrColorImg)
{
	if ((pInspAlgoBW.m_b2dCheck == TRUE && pucImgSrc == NULL) || (pInspAlgoBW.m_b3dCheck == TRUE && pfImgSrc == NULL) ||
		nWidth <= 0 || nHeight <= 0 || (!pInspAlgoBW.m_b2dCheck && !pInspAlgoBW.m_b3dCheck && ucArrDstImg == NULL))
		return 0;

	// 폴리곤 검사 영역 사용시에는 nisnpareaper 를 사용 할 수 없음.
	if (stTieArea.m_nUsedInspPolygon > 0)
		nInspAreaPer = 100;

	int nArea = nWidth * nHeight;
	UCHAR *pUcImgDst2D = NULL;
	UCHAR *pUcImgDst3D = NULL;
	UCHAR *pUcImgBinary = g_pMManager->pem_new<UCHAR>(true, nWidth * nHeight, (PCHAR)__FUNCTION__, __LINE__);
	memset(pUcImgBinary, 0, nArea * sizeof(UCHAR));
	bool bIn = false;
	// 1. Binarize
	if (pInspAlgoBW.m_b2dCheck)
	{
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nWidth, nHeight, _T("Origin2D.bmp"));
		CPInsp::Binarize(pucImgSrc, nWidth, nHeight, pInspAlgoBW.m_nRange,
			pInspAlgoBW.m_nMinValue, pInspAlgoBW.m_nMaxValue, pInspAlgoBW.m_bInvertCheck, pUcImgDst2D);
		m_pProcMilAlgo->SaveWorkImg(pUcImgDst2D, nWidth, nHeight, _T("Bin2D.bmp"));
		if (pInspAlgoBW.m_b3dCheck == FALSE && ucArrColorImg == NULL)
		{
			memcpy(pUcImgBinary, pUcImgDst2D, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (pInspAlgoBW.m_b3dCheck)
	{
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("Origin3D.bmp"));
		CPInsp::Binarize(pfImgSrc, nWidth, nHeight, pInspAlgoBW.m_n3dRange,
			pInspAlgoBW.m_d3dHeightMin, pInspAlgoBW.m_d3dHeightMax, pInspAlgoBW.m_bInvertCheck, pUcImgDst3D);
		m_pProcMilAlgo->SaveWorkImg(pUcImgDst3D, nWidth, nHeight, _T("Bin3D.bmp"));
		if (pInspAlgoBW.m_b2dCheck == FALSE && ucArrColorImg == NULL)
		{
			memcpy(pUcImgBinary, pUcImgDst3D, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (ucArrColorImg)
	{
		m_pProcMilAlgo->SaveWorkImg(ucArrColorImg, nWidth, nHeight, _T("OriginColor.bmp"));
		if (pInspAlgoBW.m_b2dCheck == FALSE && pInspAlgoBW.m_b3dCheck == FALSE)
		{
			memcpy(pUcImgBinary, ucArrColorImg, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (bIn == false)
	{
		for (int nIndex = 0; nIndex < nArea; nIndex++)
		{
			UCHAR uc2DVal = (pInspAlgoBW.m_b2dCheck && pUcImgDst2D) ? pUcImgDst2D[nIndex] : 255;
			UCHAR uc3DVal = (pInspAlgoBW.m_b3dCheck && pUcImgDst3D) ? pUcImgDst3D[nIndex] : 255;
			UCHAR ucColorVal = (ucArrColorImg) ? ucArrColorImg[nIndex] : 255;
			if (uc2DVal == 255 && uc3DVal == 255 && ucColorVal == 255)
				pUcImgBinary[nIndex] = 255;
		}
	}
	if (nDir >= 0 && nDir <= 3 && nInspAreaPer < 100)
	{
		int nVal = 0;
		double dDataBW = CalcBWInspectArea(pUcImgBinary, nWidth, nHeight, nDir, nInspAreaPer, nVal);
	}
	m_pProcMilAlgo->SaveWorkImg(pUcImgBinary, nWidth, nHeight, _T("Bin2D3D.bmp"));
	if (pUcImgDst2D)
	{
		g_pMManager->pem_delete(pUcImgDst2D, true);
		pUcImgDst2D = NULL;
	}
	if (pUcImgDst3D)
	{
		g_pMManager->pem_delete(pUcImgDst3D, true);
		pUcImgDst3D = NULL;
	}
	// KIY 2020/04/23 : 폴리곤 통합
	CPInsp::FillOutOfInspAreaCombine(nWidth, nHeight, 0, pUcImgBinary, NULL, stTieArea);
	// 2. Blob
	// 3. Select Blob
	int nFilter = bFilterIsUse ? nFilterStepNarrow : 0;
	int nCntBlob = CalcBlob(pUcImgBinary, ucArrDstImg, nWidth, nHeight, nMinBlobArea, bApplyFillHole, nFilter, nTypeSelectBlob, dArea, dCx, dCy, rcBlob);
	if (nCntBlob > 0)
	{
		m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nWidth, nHeight, _T("Blob.bmp"));
	}
	if (pUcImgBinary)
	{
		g_pMManager->pem_delete(pUcImgBinary, true);
		pUcImgBinary = NULL;
	}
	return nCntBlob;
}

int CPInsp::CalcBlob(UCHAR* uArrBinSrc, UCHAR* uArrDst, int nWidth, int nHeight, int nMinArea, BOOL bFillHole, int nFilter, int nSelectType, double* dArea, double* dCx, double* dCy, CRect* rcRect, int nTeachX, int nTeachY, double dSArea)
{
	cv::Mat imgSrc(nHeight, nWidth, CV_8UC1, uArrBinSrc);
	cv::Mat imgDst(nHeight, nWidth, CV_8UC1, uArrDst);
	std::vector<std::vector<cv::Point>> allRst;
	std::vector<std::vector<cv::Point>> allcontour;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(imgSrc, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int nCnt = allcontour.size();
	cv::Mat imgTemp(imgSrc.rows, imgSrc.cols, CV_8UC1, cv::Scalar(0));
	cv::Mat imgTempA(imgSrc.rows, imgSrc.cols, CV_8UC1, cv::Scalar(0));
	imgDst.setTo(0);
	int nMAX_A = -1;
	int nMAX = -1;
	int nMIN_GAP_A = -1;
	double dMIN_GAP = -1;
	int nCntBlob = 0;
	for (int a = 0; a < nCnt; a++)
	{
		int nArea = cv::contourArea(allcontour[a]);
		if (nMinArea >= nArea)
			continue;

		imgTemp.setTo(0);
		cv::drawContours(imgTemp, allcontour, a, cv::Scalar(255), cv::FILLED);
		if (bFillHole == FALSE)
		{
			imgTempA.setTo(0);
			cv::bitwise_and(imgTemp, imgSrc, imgTempA);
			nArea = cv::countNonZero(imgTempA);
			if (nMinArea >= nArea)
				continue;
		}

		if (nSelectType == eSelectBigger)
		{
			if (nMAX_A >= nArea)
				continue;
			nMAX_A = nArea;
			nMAX = a;

			imgDst.setTo(0);
			allRst.clear();
			allRst.push_back(allcontour[nMAX]);
			if (bFillHole)
				imgTemp.copyTo(imgDst);
			else
				imgTempA.copyTo(imgDst);

			nCntBlob = 1;
		}
		else if (nSelectType == eSelectMix)
		{
			if (bFillHole)
				cv::bitwise_or(imgTemp, imgDst, imgDst);
			else
				cv::bitwise_or(imgTempA, imgDst, imgDst);
			allRst.push_back(allcontour[a]);

			nCntBlob = allRst.size();
		}
		else if (nSelectType == eSelectCenter || nSelectType == eSelectPosition)
		{
			double dPosX = nTeachX;
			double dPosY = nTeachY;
			if (nSelectType == eSelectCenter)
			{
				dPosX = nWidth / 2.;
				dPosY = nHeight / 2.;
			}
			if (dPosX < 0 || dPosY < 0)
				continue;

			cv::Moments m = cv::moments(allcontour[a]);
			double dCxTemp = m.m10 / m.m00;
			double dCyTemp = m.m01 / m.m00;
			double dDist = sqrt(pow(dPosX - dCxTemp, 2) + pow(dPosY - dCyTemp, 2));
			if (dMIN_GAP < 0 || dDist < dMIN_GAP)
			{
				dMIN_GAP = dDist;
				imgDst.setTo(0);
				allRst.clear();
				allRst.push_back(allcontour[a]);
				if (bFillHole)
					imgTemp.copyTo(imgDst);
				else
					imgTempA.copyTo(imgDst);
			}

			nCntBlob = 1;
		}
		else if (nSelectType == eSimilarArea)
		{
			int nTempGap = abs(nArea - dSArea);
			if (nMIN_GAP_A < 0 || nTempGap < nMIN_GAP_A)
			{
				nMIN_GAP_A = nTempGap;
				imgDst.setTo(0);
				allRst.clear();
				allRst.push_back(allcontour[a]);
				if (bFillHole)
					imgTemp.copyTo(imgDst);
				else
					imgTempA.copyTo(imgDst);
			}

			nCntBlob = 1;
		}
	}

	if (nFilter > 0)
	{
		cv::morphologyEx(imgDst, imgDst, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), nFilter);
		cv::morphologyEx(imgDst, imgDst, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), nFilter);
	}

	if (nCntBlob > 0)
	{
		GetBlobResult(imgDst, dArea, dCx, dCy, rcRect);
	}

	return nCntBlob;
}

int CPInsp::GetBlobResult(cv::Mat imgBlob, double* dArea, double* dCx, double* dCy, CRect* rcRect)
{
	if (dArea != NULL)
		*dArea = cv::countNonZero(imgBlob);
	if (dCx != NULL || dCy != NULL || rcRect != NULL)
	{
		cv::Moments m = cv::moments(imgBlob, true);
		if (dCx != NULL)
			*dCx = m.m10 / m.m00;
		if (dCy != NULL)
			*dCy = m.m01 / m.m00;
		if (rcRect != NULL)
		{
			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(imgBlob, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			if (contours.size() > 0)
			{
				cv::Rect rcTemp = cv::boundingRect(contours[0]);
				for (int a = 1; a < contours.size(); a++)
				{
					rcTemp |= cv::boundingRect(contours[a]);
				}
				*rcRect = CRect(rcTemp.x, rcTemp.y, rcTemp.x + rcTemp.width, rcTemp.y + rcTemp.height);
			}
		}
	}

	return M_SUCCESS;
}

int CPInsp::GetBlobResult_Rects(cv::Mat imgBlob, std::vector<CRect>& rcRect)
{
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(imgBlob, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int nCntBlob = contours.size();
	if (nCntBlob > 0)
	{
		rcRect.resize(nCntBlob);

		for (int a = 0; a < nCntBlob; a++)
		{
			cv::Rect rcTemp = cv::boundingRect(contours[a]);
			rcRect[a] = CRect(rcTemp.x, rcTemp.y, rcTemp.x + rcTemp.width, rcTemp.y + rcTemp.height);
		}
	}
	else
		return M_FAILURE;

	return M_SUCCESS;
}

void CPInsp::MappingExceptSolderArea(UCHAR *ucArrDstImg, int nWidth, int nHeight, int *pnExceptMatchingMaps,CRect *rcBlob, CRect *rtSolderFillRect, double *dSolderHeight, double dUserSolderMin, int nCntBlob, int nSolderCnt, BOOL bUseInit)
{
	CRect rtDes = NULL;

	//int *pnTempExceptMatchingMaps = new int[nCntBlob];
	int *pnTempExceptMatchingMaps = g_pMManager->pem_new<int>(true, nCntBlob, (PCHAR)__FUNCTION__, __LINE__);

	if(bUseInit)
	{
		for(int i = 0; i< nCntBlob; i++)
			pnTempExceptMatchingMaps[i] = eNotExistSolderExcept;
	}
	else
	{
		// 위에서 맵핑 한걸 복사 해 온다.
		for(int i = 0; i< nCntBlob; i++)
			pnTempExceptMatchingMaps[i] = pnExceptMatchingMaps[i];
	}

	// 블랍한 영역과 유저 솔더 영역을 교차 비교해서 완벽히 일치하는 영역일때 위에서 구한 높이와 비교한다.
	for(int i = 0; i< nCntBlob; i++)
	{
		for(int j = 0; j < nSolderCnt; j++)
		{
			BOOL bRetInSect = rtDes.IntersectRect(rcBlob[i], rtSolderFillRect[j]);
			if(bRetInSect && rtDes == rcBlob[i])	// perfect matching
			{
				if(dSolderHeight[j] >= dUserSolderMin)	// if It is upper area
				{
					pnTempExceptMatchingMaps[i] = eSolderExceptUpper;	// 실제 저장할때 이 영역은 solder 영역 제외 영역 높이 NG로 처리하기 위해 map 에 2로 marking 해 둔다.
					break;
				}
				else
				{
					if(pnTempExceptMatchingMaps[i] != eSolderExceptUpper)
						pnTempExceptMatchingMaps[i] = eSolderExceptLower;	// 실제 저장할때 이 영역은 solder 영역 제외 영역 제외로 처리 하기 위해 1로 marking 한다.

					g_pInspMng->FillExceptWndArea(ucArrDstImg, rtSolderFillRect[j], nWidth, nHeight, 0);

					if(ucArrDstImg && nWidth > 0 && nHeight > 0)
						m_pProcMilAlgo->SaveWorkImg(ucArrDstImg, nWidth, nHeight, _T("03_Foreign_SolderROIZero.bmp"));		

					break;
				}

			}			
		}				
	}	

	for(int i = 0; i< nCntBlob; i++)
		pnExceptMatchingMaps[i] = pnTempExceptMatchingMaps[i];
	
	if(pnTempExceptMatchingMaps != NULL)
		//delete []pnTempExceptMatchingMaps;
		g_pMManager->pem_delete(pnTempExceptMatchingMaps, true);
	pnTempExceptMatchingMaps = NULL;
}

bool CPInsp::ExtractBody(UCHAR *pucImgSrc, int nWidth, int nHeight, CRect rcBlobSrc, CRect &rcBlobDst, int nOkPercent)
{
	bool bRet = false;
	if((pucImgSrc == NULL) || (nWidth <= 0) ||(nHeight <= 0) || (rcBlobSrc == NULL))
		return bRet;

	rcBlobDst = CRect(0,0,0,0);
	double dWhiteValue = 0.;
	double dWhiteRate = 0.;
	POINT pt[4];
	int nRectW = rcBlobSrc.right - rcBlobSrc.left;
	int nRectH = rcBlobSrc.bottom - rcBlobSrc.top;
	
	// top Y
	for (int y = 0; y < nHeight; y++)
	{
		dWhiteValue = 0.0;
		for(int x = 0; x < nWidth; x++)
		{
			if(pucImgSrc[x + (y*nWidth)] == 255)
				dWhiteValue += 1;
		}
		dWhiteRate = (dWhiteValue / (double)nRectW) * 100.0;
		if(dWhiteRate >= nOkPercent)
		{
			pt[0].y = y;	// top left
			pt[3].y = y;	// top right
			y = nHeight;
		}
	}
	// left X
	for (int x = 0; x < nWidth; x++)
	{
		dWhiteValue = 0.0;
		for(int y = 0; y < nHeight; y++)
		{
			if(pucImgSrc[x + (y*nWidth)] == 255)
				dWhiteValue += 1;
		}
		dWhiteRate = (dWhiteValue / (double)nRectH) * 100.0;
		if(dWhiteRate >= nOkPercent)
		{
			pt[0].x = x;	// top left
			pt[1].x = x;	// bottom left
			x = nWidth;
		}
	}
	// bottom Y
	for (int y = nHeight-1; y >= 0; y--)
	{
		dWhiteValue = 0.0;
		for(int x = 0; x < nWidth; x++)
		{
			if(pucImgSrc[x + (y*nWidth)] == 255)
				dWhiteValue += 1;
		}
		dWhiteRate = (dWhiteValue / (double)nRectW) * 100.0;
		if(dWhiteRate >= nOkPercent)
		{
			pt[1].y = y;	// bottom left
			pt[2].y = y;	// bottom right
			y = -1;
		}
	}
	// right X
	for (int x = nWidth-1; x >= 0; x--)
	{
		dWhiteValue = 0.0;
		for(int y = 0; y < nHeight; y++)
		{
			if(pucImgSrc[x + (y*nWidth)] == 255)
				dWhiteValue += 1;
		}
		dWhiteRate = (dWhiteValue / (double)nRectH) * 100.0;
		if(dWhiteRate > nOkPercent)
		{
			pt[2].x = x;	// bottom right
			pt[3].x = x;	// top right
			x = -1;
		}
	}

	LONG nLeft = (LONG)((pt[0].x <= pt[1].x)?		pt[0].x : pt[1].x);
	LONG nRight = (LONG)((pt[2].x <= pt[3].x)?		pt[3].x : pt[2].x);
	LONG nTop = (LONG)((pt[0].y <= pt[3].y)?		pt[0].y : pt[3].y);
	LONG nBottom = (LONG)((pt[1].y <= pt[2].y)?		pt[2].y : pt[1].y);

	if(rcBlobDst != NULL)
	{
		rcBlobDst.left = nLeft;
		rcBlobDst.right = nRight;
		rcBlobDst.top = nTop;
		rcBlobDst.bottom = nBottom;
	}

	return bRet = true;
}

BOOL CPInsp::AngleStartChange(const int nAngle, const int nWidth, const int nHeight, const int nDir, const RECT rcSrcRect, int *ptrnOutStartX, int *ptrnOutStartY)
{
	BOOL bResult = FALSE;

	if((nAngle < 0) && (nWidth <= 0) && (nHeight <= 0))
		return bResult;

	if(nAngle == 0 || nAngle == 180)
	{
		*ptrnOutStartY = rcSrcRect.top + (nHeight/2.0);
		*ptrnOutStartX = rcSrcRect.left + (nWidth/2.0);
	}
	if(nAngle == 90 || nAngle == 270)
	{
		if(nDir == 0 || nDir == 1)
		{
			*ptrnOutStartX = -rcSrcRect.right + (nWidth/2.0);
			*ptrnOutStartY = -rcSrcRect.bottom + (nHeight/2.0);
		}
		else
		{
			*ptrnOutStartX = rcSrcRect.left + (nWidth/2.0);
			*ptrnOutStartY = rcSrcRect.top + (nHeight/2.0);
		}
	}

	if(*ptrnOutStartY < 0)
		*ptrnOutStartY = 0;
	if(*ptrnOutStartX < 0)
		*ptrnOutStartX = 0;

	if((*ptrnOutStartY <= 0) && (*ptrnOutStartX <= 0))
		bResult = FALSE;
	else
		bResult = TRUE;

	return bResult;
}

BOOL CPInsp::AngleRectChange(const int nAngle, const int nWidth, const int nHeight, const RECT rcSrcRect, RECT *rcDstRect)
{
	BOOL bResult = FALSE;

	if((nAngle < 0) && (nWidth <= 0) && (nHeight <= 0))
		return bResult;

	rcDstRect->bottom = rcSrcRect.bottom + (nHeight/2.0);
	rcDstRect->top = rcSrcRect.top + (nHeight/2.0);
	rcDstRect->right = rcSrcRect.right + (nWidth/2.0);
	rcDstRect->left = rcSrcRect.left + (nWidth/2.0);

	if(rcDstRect->left < 0)
		rcDstRect->left = 0;
	if(rcDstRect->right > nWidth)
		rcDstRect->right = nWidth;
	if(rcDstRect->top < 0)
		rcDstRect->top = 0;
	if(rcDstRect->bottom > nHeight)
		rcDstRect->bottom = nHeight;

	if((rcDstRect->bottom <= 0) && (rcDstRect->top <= 0) && (rcDstRect->right <= 0) && (rcDstRect->left <= 0))
		bResult = FALSE;
	if((rcDstRect->right <= rcDstRect->left) || (rcDstRect->bottom <= rcDstRect->top))
		bResult = FALSE;
	else
		bResult = TRUE;

	
	return bResult;
}

BOOL CPInsp::AngleRectChange2(const double dAngle, const int nWidth, const int nHeight, const RECT rcSrcRect, RECT *rcDstRect)
{
	BOOL bResult = FALSE;
	int nRelativeAngle = ((int)dAngle + 360) % 360;
	if ((dAngle < 0) && (nWidth <= 0) && (nHeight <= 0))
		return bResult;
	if (nRelativeAngle == 90)
	{
		rcDstRect->bottom = -rcSrcRect.left;
		rcDstRect->top = -rcSrcRect.right;
		rcDstRect->right = rcSrcRect.bottom;
		rcDstRect->left = rcSrcRect.top;
	}
	else if (nRelativeAngle == 180)
	{
		rcDstRect->bottom = -rcSrcRect.top;
		rcDstRect->top = -rcSrcRect.bottom;
		rcDstRect->right = -rcSrcRect.left;
		rcDstRect->left = -rcSrcRect.right;
	}
	else if (nRelativeAngle == 270)
	{
		rcDstRect->bottom = rcSrcRect.right;
		rcDstRect->top = rcSrcRect.left;
		rcDstRect->right = -rcSrcRect.top;
		rcDstRect->left = -rcSrcRect.bottom;
	}
	else
	{
		rcDstRect->bottom = rcSrcRect.bottom;
		rcDstRect->top = rcSrcRect.top;
		rcDstRect->right = rcSrcRect.right;
		rcDstRect->left = rcSrcRect.left;
	}
	rcDstRect->bottom += (nHeight / 2.0);
	rcDstRect->top += (nHeight / 2.0);
	rcDstRect->right += (nWidth / 2.0);
	rcDstRect->left += (nWidth / 2.0);

	if (rcDstRect->left < 0)
		rcDstRect->left = 0;
	if (rcDstRect->right > nWidth)
		rcDstRect->right = nWidth;
	if (rcDstRect->top < 0)
		rcDstRect->top = 0;
	if (rcDstRect->bottom > nHeight)
		rcDstRect->bottom = nHeight;

	if ((rcDstRect->bottom <= 0) && (rcDstRect->top <= 0) && (rcDstRect->right <= 0) && (rcDstRect->left <= 0))
		bResult = FALSE;
	if ((rcDstRect->right <= rcDstRect->left) || (rcDstRect->bottom <= rcDstRect->top))
		bResult = FALSE;
	else
		bResult = TRUE;
	return bResult;
}

BOOL CPInsp::AnglePointChange(const int nWidth, const int nHeight, const POINTF poSrcPoint, POINTF *poDstPoint)
{
	BOOL bResult = FALSE;

	if((nWidth <= 0) && (nHeight <= 0))
		return bResult;

	poDstPoint->x = poSrcPoint.x + (nWidth/2.);
	poDstPoint->y = poSrcPoint.y + (nHeight/2.);

	if((poDstPoint->x <= 0) && (poDstPoint->y <= 0))
		bResult = FALSE;
	else
		bResult = TRUE;

	return bResult;
}

BOOL CPInsp::AnglePointChange_mm(const float fWidth, const float fHeight, const POINTF poSrcPoint, POINTF *poDstPoint)
{
	BOOL bResult = FALSE;

	if((fWidth <= 0) && (fHeight <= 0))
		return bResult;

	poDstPoint->x = poSrcPoint.x + (fWidth/2.0f);
	poDstPoint->y = poSrcPoint.y + (fHeight/2.0f);

	if((poDstPoint->x < 0) && (poDstPoint->y < 0))
		bResult = FALSE;
	else
		bResult = TRUE;

	return bResult;
}

BOOL CPInsp::AnglePosChange(const int nAngle, const int nWidth, const int nHeight, const POINTF poSrcPoint, POINTF *poDstPoint)
{
	BOOL bResult = FALSE;
	if((nWidth <= 0) && (nHeight <= 0))
		return bResult;

	float fBufX = 0.0;
	float fBufY = 0.0;
	if(nAngle == 270)
	{
		fBufX = poSrcPoint.y * -1;
		fBufY = poSrcPoint.x;
	}
	else if(nAngle == 180)
	{
		fBufX = poSrcPoint.x * -1;
		fBufY = poSrcPoint.y * -1;
	}
	else if(nAngle == 90)
	{
		fBufX = poSrcPoint.y;
		fBufY = poSrcPoint.x * -1;
	}
	else
	{
		fBufX = poSrcPoint.x;
		fBufY = poSrcPoint.y;
	}
	poDstPoint->x = fBufX + (nWidth/2.);
	poDstPoint->y = fBufY + (nHeight/2.);
	bResult = poDstPoint->x < 0 && poDstPoint->y < 0 ? FALSE : TRUE;
	return bResult;
}

int CPInsp::CalcLeadSolderViewAngle(int nPosition, double dAngle)
{
	if (nPosition < 0)
		return nPosition;

	int nVal = 0;
	int outposition = nPosition;
	switch(nPosition)
	{
	case 0:
		nVal = 90;
		break;
	case 1:
		nVal = 270;
		break;
	case 2:
		nVal = 360;
		break;
	case 3:
		nVal = 180;
		break;
	}
	nVal += (int)dAngle;
	if (nVal >= 360)
		nVal -= 360;

	switch(nVal)
	{
	case 0:
		outposition = 2;
		break;
	case 90:
		outposition = 0;
		break;
	case 180:
		outposition = 3;
		break;
	case 270:
		outposition = 1;
		break;
	}
	return outposition;
}

// SHKang 2017/07/24
void CPInsp::Init_PseudoMap() // cmap_1280.txt
{
	int nTempB=128;
	int nTempG= 0;
	int nTempR= 0;

	for(int i=0; i < CR_MAP_SIZE; i++)
	{
		if(nTempB>255) nTempB = 255; else if(nTempB<0) nTempB = 0;
		if(nTempG>255) nTempG = 255; else if(nTempG<0) nTempG = 0;
		if(nTempR>255) nTempR = 255; else if(nTempR<0) nTempR = 0;

		m_crPseudo[i].r = nTempR;
		m_crPseudo[i].g = nTempG;
		m_crPseudo[i].b = nTempB;

		if(i < 382) nTempB++;
		else nTempB--;

		if(i < 127) nTempG--;
		else if(i < 892) nTempG++;
		else nTempG--;

		if(i < 637) nTempR--;
		else if(i < 1147) nTempR++;
		else nTempR--;
	}
}

// SHKang 2017/07/24
void CPInsp::Set_PseudoResol(float fMin, float fMax)
{
	fMax *= 1.15;
	m_fMinZ = fMin;
	m_fMaxZ = fMax;
	float fDiv = CR_MAP_SIZE;
	m_fDivZ = (fMax - fMin) / fDiv;
}

// SHKang 2017/07/24
int CPInsp::GetRGB(float fHei, UCHAR * ucArrDstImg, int nIndex)
{
	float fMin = m_fMinZ;
	float fMax = m_fMaxZ;
	float fDiv = m_fDivZ;

	int crId = 0;
	if (fDiv > 0)
	{
		crId = (fHei - fMin) / fDiv;
		if (crId < 0) crId = 0;
		else if (crId >= CR_MAP_SIZE) crId = CR_MAP_SIZE - 1;
	}

	ucArrDstImg[nIndex] = m_crPseudo[crId].b;
	ucArrDstImg[nIndex + 1] = m_crPseudo[crId].g;
	ucArrDstImg[nIndex + 2] = m_crPseudo[crId].r;

	return 1;
}
int CPInsp::SetRGB(UCHAR * ucArrDstImg, int nIndex, float fH, float fMin, float fMax)
{
	float fMaxTemp = fMax * 1.15f;
	float fDivTemp = CR_MAP_SIZE;
	float fDivZ = (fMaxTemp - fMin) / fDivTemp;
	if (fDivZ <= 0)
		return 0;

	int crId = (fH - fMin) / fDivZ;
	if (crId < 0) crId = 0;
	if (crId >= CR_MAP_SIZE) crId = CR_MAP_SIZE - 1;

	ucArrDstImg[nIndex] = m_crPseudo[crId].b;
	ucArrDstImg[nIndex + 1] = m_crPseudo[crId].g;
	ucArrDstImg[nIndex + 2] = m_crPseudo[crId].r;

	return 1;
}
int CPInsp::SetRGB(UCHAR * ucArrDstImg, long long nIndex, float fH, float fMin, float fMax)
{
	float fMaxTemp = fMax * 1.15f;
	float fDivTemp = CR_MAP_SIZE;
	float fDivZ = (fMaxTemp - fMin) / fDivTemp;
	if (fDivZ <= 0)
		return 0;

	int crId = (fH - fMin) / fDivZ;
	if (crId < 0) crId = 0;
	if (crId >= CR_MAP_SIZE) crId = CR_MAP_SIZE - 1;

	ucArrDstImg[nIndex] = m_crPseudo[crId].b;
	ucArrDstImg[nIndex + 1] = m_crPseudo[crId].g;
	ucArrDstImg[nIndex + 2] = m_crPseudo[crId].r;

	return 1;
}
// SHKang 2017/07/24
int CPInsp::Get_PseudoColorMap(float * fZmapSrc, UCHAR * ucMask, int width, int widthstep, int length, UCHAR* ucArrDstImg)
{
	if (m_fMinZ == -100.0f && m_fMaxZ == -100.0f)
	{
		float fMin=0.f, fMax=0.f;
		GetHeightMinMax(fZmapSrc, width, length, &fMin, &fMax);
		Set_PseudoResol(fMin, fMax);
	}	
	int nRet = 0;
	int nStep = 0;
	int nIdx = 0;
	#pragma omp parallel for
	for(int j=0; j < length; j++)
	{
		for(int i=0; i < width; i++)
		{
			int nZmapId = width * j + i;
			int nStepId = j * widthstep + i;
			int nDstId = nStepId * 3;

			if(ucMask == NULL || ucMask[nStepId] == 255)
			{
				GetRGB(fZmapSrc[nZmapId], ucArrDstImg, nDstId);
			}
			else
			{
				ucArrDstImg[nDstId] = 0;
				ucArrDstImg[nDstId + 1] = 0;
				ucArrDstImg[nDstId + 2] = 0;
			}
		}
	}

	nRet = 1;
	return nRet;
}

float CPInsp::GetImageROIHeightAvg(float *pfImgSrc, int nWidth, int nHeight, RECT rcROI, float fDefaultHeight)
{
	float fRstAvg = 0.0f;
	int nL = rcROI.left;
	int nR = rcROI.right;
	int nT = rcROI.top;
	int nB = rcROI.bottom;
	if(nL < 0) nL = 0;
	if(nR < 0) nR = 0;
	if(nT < 0) nT = 0;
	if(nB < 0) nB = 0;
	if(nL > nWidth) nL = nWidth;
	if(nR > nWidth) nR = nWidth;
	if(nT > nHeight) nT = nHeight;
	if(nB > nHeight) nB = nHeight;
	if(nL > nR)	nL = nR;
	if(nT > nB)	nT = nB;
	float fSum = 0.0f;
	float fCnt = 0.0f;
	for (int y = nT; y < nB; y++)
	{
		for (int x = nL; x < nR; x++)
		{
			int nIndex = (y * nWidth) + x;
			if(pfImgSrc[nIndex] > fDefaultHeight)
			{
				fSum += pfImgSrc[nIndex];
				fCnt++;
			}
		}
	}
	if(fCnt > 0)
		fRstAvg = fSum / fCnt;
	return fRstAvg;
}

float CPInsp::CalcTiltAngle( float *pfArrImgData,int nImgWidth, int nImgHeight ,double* A, double* B, double* C)
{
#if _TILTANGLE_USE_PCA
	float fReturnRate = 0.0F;
	if(pfArrImgData == NULL)
		return fReturnRate;

	Ipp64f Mean;
	IppiSize roiSize = {nImgWidth, nImgHeight};
	ippiMean_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &Mean,  ippAlgHintNone);
	fReturnRate = Mean;

	cv::Mat matrixInsp(nImgHeight*nImgWidth,3,CV_32F);

	for(int i=0;i<nImgHeight;i++)
	{
		for(int j=0;j<nImgWidth;j++)
		{
			matrixInsp.ptr<float>(i)[0] = j*m_resolX*1000;		matrixInsp.ptr<float>(i)[1] =i*m_resolY*1000;		matrixInsp.ptr<float>(i)[2] = pfArrImgData[nImgWidth*i+j];
		}
	}
	cv::Mat covar, comeam,eigenVal,eigenMt;
	cv::calcCovarMatrix(matrixInsp, covar, comeam, cv::COVAR_NORMAL | cv::COVAR_ROWS,CV_32F);
	covar /= (matrixInsp.rows-1);
	cv::eigen(covar,eigenVal,eigenMt);

	float* fptrV = eigenVal.ptr<float>(0);
	float* fptrM = eigenMt.ptr<float>(0);
	*A = fptrM[6];
	*B = fptrM[7];
	*C = fptrM[8];

	return fReturnRate;
#else

	float fReturnRate = 0.0F;
	int nLine = __LINE__;
	try
	{
		nLine = __LINE__;
		if(pfArrImgData == NULL || nImgHeight*nImgWidth<1)
			return fReturnRate;
		cv::Mat matrixX(nImgHeight*nImgWidth,3,CV_32F),matrixXT,matrixY(nImgHeight*nImgWidth,1,CV_32F);
		// 	cv::Mat srcData(nImgHeight,nImgWidth,CV_32F,pfArrImgData);
		// 
		// 	cv::Size knel(5,5);
		// 
		// 	cv::blur(srcData,srcData,knel);
		nLine = __LINE__;
		for(int i=0;i<nImgHeight;i++)
		{
			for(int j=0;j<nImgWidth;j++)
			{
				float* fptr = matrixX.ptr<float>(nImgWidth*i+j);
				float* fptry = matrixY.ptr<float>(nImgWidth*i+j);

				fptr[0] = j*m_resolX*1000;
				fptr[1] = i*m_resolY*1000;
				fptr[2] = 1;
				// 			fptr[2] = i*j;
				// 			fptr[3] = 1;

				fptry[0] = pfArrImgData[nImgWidth*i+j];
			}
		}
		nLine = __LINE__;

		cv::Mat Xt= matrixX.t();
		cv::Mat XX = Xt * matrixX;
		cv::Mat XY = Xt * matrixY;
		cv::Mat res = XX.inv() * XY;;
		nLine = __LINE__;
		float* XXptr = XX.ptr<float>(0);
		float* XTptr = Xt.ptr<float>(0);
		float* XYptr = XY.ptr<float>(0);
		float* Reptr = res.ptr<float>(0);
		*A = Reptr[0];
		*B = Reptr[1];
		*C = -1;
		//	*C = Reptr[2];
		nLine = __LINE__;
		// 	Ipp64f Mean;
		// 	IppiSize roiSize = {nImgWidth, nImgHeight};
		// 	ippiMean_32f_C1R(pfArrImgData, nImgWidth*sizeof(float), roiSize, &Mean,  ippAlgHintNone);
		// 	fReturnRate = Mean;
	}

	catch(...)
	{

		if(g_pMPTI)
		{
			CString sLog = _T("");
			sLog.Format(_T("CPInsp_Algo::CalcTiltAngle(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);

			throw nLine;
		}
	}
	return fReturnRate;

#endif
}

float CPInsp::LeastSquarePlane( int cnt,float *height,POINTF* pint, double* A, double* B, double* C)
{
	if(cnt <= 1)
		return 0.0f;
//	cv::Mat matrixInsp(8,3,CV_32F),MatrixStd(8,3,CV_32F);
// 	matrixInsp.ptr<float>(0)[0] = 6131.55;		matrixInsp.ptr<float>(0)[1] = 6287.4;		matrixInsp.ptr<float>(0)[2] = 3153.089;
// 	matrixInsp.ptr<float>(1)[0] = 5503.44;		matrixInsp.ptr<float>(1)[1] = 7265.44;		matrixInsp.ptr<float>(1)[2] = 3151.533;
// 	matrixInsp.ptr<float>(2)[0] = 5543.32;		matrixInsp.ptr<float>(2)[1] = 9406.15;		matrixInsp.ptr<float>(2)[2] = 3160.676;
// 	matrixInsp.ptr<float>(3)[0] = 6036.835;		matrixInsp.ptr<float>(3)[1] = 10229.5;		matrixInsp.ptr<float>(3)[2] = 3156.181;
// 	matrixInsp.ptr<float>(4)[0] = 10518.35;		matrixInsp.ptr<float>(4)[1] = 6357.26;		matrixInsp.ptr<float>(4)[2] = 3155.109;
// 	matrixInsp.ptr<float>(5)[0] = 10862.315;	matrixInsp.ptr<float>(5)[1] = 6916.14;		matrixInsp.ptr<float>(5)[2] = 3156.075;
// 	matrixInsp.ptr<float>(6)[0] = 10976.97;		matrixInsp.ptr<float>(6)[1] = 9505.95;		matrixInsp.ptr<float>(6)[2] = 3170.098;
// 	matrixInsp.ptr<float>(7)[0] = 10428.62;		matrixInsp.ptr<float>(7)[1] = 10334.29;		matrixInsp.ptr<float>(7)[2] = 3165.117;
// 
// 	MatrixStd.ptr<float>(0)[0] = 4356.89;		MatrixStd.ptr<float>(0)[1] = 5219.54;		MatrixStd.ptr<float>(0)[2] = 3963.427;
// 	MatrixStd.ptr<float>(1)[0] = 12322.92;		MatrixStd.ptr<float>(1)[1] = 5429.12;		MatrixStd.ptr<float>(1)[2] = 3980.454;
// 	MatrixStd.ptr<float>(2)[0] = 12243.16;		MatrixStd.ptr<float>(2)[1] = 11486.98;		MatrixStd.ptr<float>(2)[2] = 3992.131;
// 	MatrixStd.ptr<float>(3)[0] = 4227.28;		MatrixStd.ptr<float>(3)[1] = 11327.3;		MatrixStd.ptr<float>(3)[2] = 3983.531;

	float fAvg = 0.0f;

#if _TILTANGLE_USE_PCA

 	cv::Mat matrixInsp(cnt,3,CV_32F);
 	for (int i = 0; i < cnt; i++)
 	{
 		matrixInsp.ptr<float>(i)[0] = pint[i].x*m_resolX*1000;		matrixInsp.ptr<float>(i)[1] =pint[i].y*m_resolY*1000;	matrixInsp.ptr<float>(i)[2] = height[i]; fAvg += height[i];
	}
	cv::Mat covar, comeam,eigenVal,eigenMt;
	cv::calcCovarMatrix(matrixInsp, covar, comeam, cv::COVAR_NORMAL | cv::COVAR_ROWS,CV_32F);
	covar /= (matrixInsp.rows-1);
	cv::eigen(covar,eigenVal,eigenMt);

	float* fptrV = eigenVal.ptr<float>(0);
	float* fptrM = eigenMt.ptr<float>(0);
	*A = fptrM[6];
	*B = fptrM[7];
	*C = fptrM[8];

// 	CString sLog;
// 	CStdioFile file;
// 	if(file.Open(_T("D:\\Eagle3D_data\\TiltAngle.txt"), CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate)==TRUE)
// 	{
// 		file.SeekToEnd();
// 
// 		for (int i = 0; i < cnt; i++)
// 		{
// 			sLog.Format(_T("%.3f, %.3f, %.3f \n"), pint[i].x*m_resolX*1000,pint[i].y*m_resolY*1000,height[i]);
// 			file.WriteString(sLog);
// 		}
// 		sLog.Format(_T("================\n"));
// 		file.WriteString(sLog);
// 		file.Close();
// 	}

#else
	cv::Mat matrixX(cnt,3,CV_32F),matrixXT,matrixY(cnt,1,CV_32F);
	for(int i=0;i<cnt;i++)
	{
		float* fptr = matrixX.ptr<float>(i);
		float* fptry = matrixY.ptr<float>(i);

		fptr[0] = pint[i].x*m_resolX*1000;
		fptr[1] = pint[i].y*m_resolY*1000;
		fptr[2] = 1;

		fptry[0] = height[i];
	}
	cv::Mat Xt= matrixX.t();
	cv::Mat XX = Xt * matrixX;
	cv::Mat XY = Xt * matrixY;
	cv::Mat res = XX.inv() * XY;;

	float* XXptr = XX.ptr<float>(0);
	float* XTptr = Xt.ptr<float>(0);
	float* XYptr = XY.ptr<float>(0);

	float* Reptr = res.ptr<float>(0);
	*A = Reptr[0];
	*B = Reptr[1];
	*C = -1;
#endif

	return fAvg/cnt;
}

AlgoBlob CPInsp::SetAlgoBlob(const InspAlgo &sInspAlgo, int nIdx)
{
	AlgoBlob algoBlob;

	if(sInspAlgo.m_eAlgoType == eAlgoAlign)
	{
		AlgoAlign *pInspAlgo = (AlgoAlign *)sInspAlgo.m_ptrInspAlgoParam;
		algoBlob.m_bInsp2D = pInspAlgo->m_bInsp2D;
		algoBlob.m_nMinBinary = pInspAlgo->m_nMinBinary;
		algoBlob.m_nMaxBinary = pInspAlgo->m_nMaxBinary;
		algoBlob.m_bInsp3D = pInspAlgo->m_bInsp3D;
		algoBlob.m_dHeightRateMin = pInspAlgo->m_dHeightRateMin;
		algoBlob.m_dHeightRateMax = pInspAlgo->m_dHeightRateMax;
		algoBlob.m_nTypeRange2D = pInspAlgo->m_nTypeRange2D;
		algoBlob.m_nTypeRange3D = pInspAlgo->m_nTypeRange3D;
		algoBlob.m_nTypeSelectBlob = eSelectMix;
		algoBlob.m_bFilterIsUse = FALSE;
		algoBlob.m_sAlgoColorBase = pInspAlgo->m_sAlgoColorBase;
		algoBlob.m_bFillHole = pInspAlgo->m_bFillHole;
	}
	else if(sInspAlgo.m_eAlgoType == eAlgoLine)
	{
		AlgoLine *pInspAlgo = (AlgoLine *)sInspAlgo.m_ptrInspAlgoParam;
		algoBlob.m_bUseIPC = pInspAlgo->m_bUseIPC;
		algoBlob.m_byIPCClass = pInspAlgo->m_byIPCClass;
		algoBlob.m_bInvertCheck = pInspAlgo->m_bInvertCheck;

		algoBlob.m_bFilterIsUse = pInspAlgo->m_bFilterIsUse;
		algoBlob.m_nFilterStepNarrow = pInspAlgo->m_nFilterStepNarrow;

		algoBlob.m_bShiftIsUse = pInspAlgo->m_bShiftIsUse;
		algoBlob.m_dShiftX = pInspAlgo->m_dShiftX;
		algoBlob.m_dShiftY = pInspAlgo->m_dShiftY;

		algoBlob.m_bAreaIsUse = pInspAlgo->m_bAreaIsUse;
		algoBlob.m_dAreaMin = pInspAlgo->m_dAreaMin;
		algoBlob.m_dAreaMax = pInspAlgo->m_dAreaMax;
		algoBlob.m_dAreaCurrent = pInspAlgo->m_dAreaCurrent;

		algoBlob.m_nTypeSelectBlob = pInspAlgo->m_nTypeSelectBlob;

		algoBlob.m_bInsp2D = pInspAlgo->m_bInsp2D;
		algoBlob.m_nTypeRange2D = pInspAlgo->m_nTypeRange2D;
		algoBlob.m_nMinBinary = pInspAlgo->m_nMinBinary;
		algoBlob.m_nMaxBinary = pInspAlgo->m_nMaxBinary;

		algoBlob.m_bInsp3D = pInspAlgo->m_bInsp3D;
		algoBlob.m_nTypeRange3D = pInspAlgo->m_nTypeRange3D;
		algoBlob.m_dHeightRateMin = pInspAlgo->m_dHeightRateMin;
		algoBlob.m_dHeightRateMax = pInspAlgo->m_dHeightRateMax;
		algoBlob.m_fHeightAvg = pInspAlgo->m_fHeightAvg;

		algoBlob.m_dTechCenterX = pInspAlgo->m_dTechCenterX;
		algoBlob.m_dTechCenterY = pInspAlgo->m_dTechCenterY;

		algoBlob.m_bTeachWidthUse = pInspAlgo->m_bTeachWidthUse;
		algoBlob.m_dTeachWidth = pInspAlgo->m_dTeachWidth;
		algoBlob.m_dTeachWidthRateMin = pInspAlgo->m_dTeachWidthRateMin;
		algoBlob.m_dTeachWidthRateMax = pInspAlgo->m_dTeachWidthRateMax;

		algoBlob.m_bTeachLengthUse = pInspAlgo->m_bTeachLengthUse;
		algoBlob.m_dTeachLength = pInspAlgo->m_dTeachLength;
		algoBlob.m_dTeachLengthRateMin = pInspAlgo->m_dTeachLengthRateMin;
		algoBlob.m_dTeachLengthRateMax = pInspAlgo->m_dTeachLengthRateMax;

		algoBlob.m_bShiftXUse = pInspAlgo->m_bShiftXUse;
		algoBlob.m_bShiftYUse = pInspAlgo->m_bShiftYUse;

		algoBlob.m_bUseBlobNG = pInspAlgo->m_bUseBlobNG;
		algoBlob.m_dBlobSizeWidth = pInspAlgo->m_dBlobSizeWidth;
		algoBlob.m_dBlobSizeLength = pInspAlgo->m_dBlobSizeLength;
		algoBlob.m_bUseBlobSizeWidth = pInspAlgo->m_bUseBlobSizeWidth;
		algoBlob.m_bUseBlobSizeLength = pInspAlgo->m_bUseBlobSizeLength;

		algoBlob.m_bUseHeight = pInspAlgo->m_bUseHeight;
		algoBlob.m_dTeachHeight = pInspAlgo->m_dTeachHeight;
		algoBlob.m_dTeachHeightMax = pInspAlgo->m_dTeachHeightMax;
		algoBlob.m_dTeachHeightMin = pInspAlgo->m_dTeachHeightMin;

		algoBlob.m_sAlgoColorBase = pInspAlgo->m_sAlgoColorBase;

		algoBlob.m_bIsModelSubImg = pInspAlgo->m_bIsModelSubImg;
		algoBlob.m_bUsePattern = pInspAlgo->m_bUsePattern;
		memcpy(algoBlob.m_sModelPath, pInspAlgo->m_sModelPath, MAX_STRLEN * sizeof(wchar_t));
		algoBlob.m_bUseFPBW = false;
		algoBlob.m_byFPMargin = 30;
		algoBlob.m_bFillHole = pInspAlgo->m_bFillHole;
		algoBlob.m_bCircleOpt = false;
	}
	else if(sInspAlgo.m_eAlgoType == eAlgoEdge)
	{
		AlgoEdge *pInspAlgo = (AlgoEdge *)sInspAlgo.m_ptrInspAlgoParam;
		algoBlob.m_bUseIPC = pInspAlgo->m_bUseIPC;
		algoBlob.m_byIPCClass = pInspAlgo->m_byIPCClass;
		algoBlob.m_bInvertCheck = pInspAlgo->m_bInvertCheck;

		algoBlob.m_bFilterIsUse = pInspAlgo->m_bFilterIsUse;
		algoBlob.m_nFilterStepNarrow = pInspAlgo->m_nFilterStepNarrow;

		algoBlob.m_bShiftIsUse = pInspAlgo->m_bShiftIsUse;
		algoBlob.m_dShiftX = pInspAlgo->m_dShiftX;
		algoBlob.m_dShiftY = pInspAlgo->m_dShiftY;

		algoBlob.m_bAreaIsUse = pInspAlgo->m_bAreaIsUse;
		algoBlob.m_dAreaMin = pInspAlgo->m_dAreaMin;
		algoBlob.m_dAreaMax = pInspAlgo->m_dAreaMax;
		algoBlob.m_dAreaCurrent = pInspAlgo->m_dAreaCurrent;

		algoBlob.m_nTypeSelectBlob = pInspAlgo->m_nTypeSelectBlob;

		algoBlob.m_bInsp2D = pInspAlgo->m_bInsp2D;
		algoBlob.m_nTypeRange2D = pInspAlgo->m_nTypeRange2D;
		algoBlob.m_nMinBinary = pInspAlgo->m_nMinBinary;
		algoBlob.m_nMaxBinary = pInspAlgo->m_nMaxBinary;

		algoBlob.m_bInsp3D = pInspAlgo->m_bInsp3D;
		algoBlob.m_nTypeRange3D = pInspAlgo->m_nTypeRange3D;
		algoBlob.m_dHeightRateMin = pInspAlgo->m_dHeightRateMin;
		algoBlob.m_dHeightRateMax = pInspAlgo->m_dHeightRateMax;
		algoBlob.m_fHeightAvg = pInspAlgo->m_fHeightAvg;

		algoBlob.m_dTechCenterX = pInspAlgo->m_dTechCenterX;
		algoBlob.m_dTechCenterY = pInspAlgo->m_dTechCenterY;

		algoBlob.m_bTeachWidthUse = pInspAlgo->m_bTeachWidthUse;
		algoBlob.m_dTeachWidth = pInspAlgo->m_dTeachWidth;
		algoBlob.m_dTeachWidthRateMin = pInspAlgo->m_dTeachWidthRateMin;
		algoBlob.m_dTeachWidthRateMax = pInspAlgo->m_dTeachWidthRateMax;

		algoBlob.m_bTeachLengthUse = pInspAlgo->m_bTeachLengthUse;
		algoBlob.m_dTeachLength = pInspAlgo->m_dTeachLength;
		algoBlob.m_dTeachLengthRateMin = pInspAlgo->m_dTeachLengthRateMin;
		algoBlob.m_dTeachLengthRateMax = pInspAlgo->m_dTeachLengthRateMax;

		algoBlob.m_bShiftXUse = pInspAlgo->m_bShiftXUse;
		algoBlob.m_bShiftYUse = pInspAlgo->m_bShiftYUse;

		algoBlob.m_bUseBlobNG = pInspAlgo->m_bUseBlobNG;
		algoBlob.m_dBlobSizeWidth = pInspAlgo->m_dBlobSizeWidth;
		algoBlob.m_dBlobSizeLength = pInspAlgo->m_dBlobSizeLength;
		algoBlob.m_bUseBlobSizeWidth = pInspAlgo->m_bUseBlobSizeWidth;
		algoBlob.m_bUseBlobSizeLength = pInspAlgo->m_bUseBlobSizeLength;

		algoBlob.m_bUseHeight = pInspAlgo->m_bUseHeight;
		algoBlob.m_dTeachHeight = pInspAlgo->m_dTeachHeight;
		algoBlob.m_dTeachHeightMax = pInspAlgo->m_dTeachHeightMax;
		algoBlob.m_dTeachHeightMin = pInspAlgo->m_dTeachHeightMin;

		algoBlob.m_sAlgoColorBase = pInspAlgo->m_sAlgoColorBase;

		algoBlob.m_bIsModelSubImg = pInspAlgo->m_bIsModelSubImg;
		algoBlob.m_bUsePattern = pInspAlgo->m_bUsePattern;
		memcpy(algoBlob.m_sModelPath, pInspAlgo->m_sModelPath, MAX_STRLEN * sizeof(wchar_t));
		algoBlob.m_bUseFPBW = false;
		algoBlob.m_byFPMargin = 30;
		algoBlob.m_bFillHole = pInspAlgo->m_bFillHole;
		algoBlob.m_bCircleOpt = false;
	}
	else if(sInspAlgo.m_eAlgoType == eAlgoAlignEdge)
	{
		AlgoAlignEdge *pInspAlgo = (AlgoAlignEdge *)sInspAlgo.m_ptrInspAlgoParam;
		algoBlob.m_bUseIPC = pInspAlgo->sArrAlgoEdge[nIdx].m_bUseIPC;
		algoBlob.m_byIPCClass = pInspAlgo->sArrAlgoEdge[nIdx].m_byIPCClass;
		algoBlob.m_bInvertCheck = pInspAlgo->sArrAlgoEdge[nIdx].m_bInvertCheck;

		algoBlob.m_bFilterIsUse = pInspAlgo->sArrAlgoEdge[nIdx].m_bFilterIsUse;
		algoBlob.m_nFilterStepNarrow = pInspAlgo->sArrAlgoEdge[nIdx].m_nFilterStepNarrow;

		algoBlob.m_bShiftIsUse = pInspAlgo->sArrAlgoEdge[nIdx].m_bShiftIsUse;
		algoBlob.m_dShiftX = pInspAlgo->sArrAlgoEdge[nIdx].m_dShiftX;
		algoBlob.m_dShiftY = pInspAlgo->sArrAlgoEdge[nIdx].m_dShiftY;

		algoBlob.m_bAreaIsUse = pInspAlgo->sArrAlgoEdge[nIdx].m_bAreaIsUse;
		algoBlob.m_dAreaMin = pInspAlgo->sArrAlgoEdge[nIdx].m_dAreaMin;
		algoBlob.m_dAreaMax = pInspAlgo->sArrAlgoEdge[nIdx].m_dAreaMax;
		algoBlob.m_dAreaCurrent = pInspAlgo->sArrAlgoEdge[nIdx].m_dAreaCurrent;

		algoBlob.m_nTypeSelectBlob = pInspAlgo->sArrAlgoEdge[nIdx].m_nTypeSelectBlob;

		algoBlob.m_bInsp2D = pInspAlgo->sArrAlgoEdge[nIdx].m_bInsp2D;
		algoBlob.m_nTypeRange2D = pInspAlgo->sArrAlgoEdge[nIdx].m_nTypeRange2D;
		algoBlob.m_nMinBinary = pInspAlgo->sArrAlgoEdge[nIdx].m_nMinBinary;
		algoBlob.m_nMaxBinary = pInspAlgo->sArrAlgoEdge[nIdx].m_nMaxBinary;

		algoBlob.m_bInsp3D = pInspAlgo->sArrAlgoEdge[nIdx].m_bInsp3D;
		algoBlob.m_nTypeRange3D = pInspAlgo->sArrAlgoEdge[nIdx].m_nTypeRange3D;
		algoBlob.m_dHeightRateMin = pInspAlgo->sArrAlgoEdge[nIdx].m_dHeightRateMin;
		algoBlob.m_dHeightRateMax = pInspAlgo->sArrAlgoEdge[nIdx].m_dHeightRateMax;
		algoBlob.m_fHeightAvg = pInspAlgo->sArrAlgoEdge[nIdx].m_fHeightAvg;

		algoBlob.m_dTechCenterX = pInspAlgo->sArrAlgoEdge[nIdx].m_dTechCenterX;
		algoBlob.m_dTechCenterY = pInspAlgo->sArrAlgoEdge[nIdx].m_dTechCenterY;

		algoBlob.m_bTeachWidthUse = pInspAlgo->sArrAlgoEdge[nIdx].m_bTeachWidthUse;
		algoBlob.m_dTeachWidth = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachWidth;
		algoBlob.m_dTeachWidthRateMin = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachWidthRateMin;
		algoBlob.m_dTeachWidthRateMax = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachWidthRateMax;

		algoBlob.m_bTeachLengthUse = pInspAlgo->sArrAlgoEdge[nIdx].m_bTeachLengthUse;
		algoBlob.m_dTeachLength = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachLength;
		algoBlob.m_dTeachLengthRateMin = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachLengthRateMin;
		algoBlob.m_dTeachLengthRateMax = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachLengthRateMax;

		algoBlob.m_bShiftXUse = pInspAlgo->sArrAlgoEdge[nIdx].m_bShiftXUse;
		algoBlob.m_bShiftYUse = pInspAlgo->sArrAlgoEdge[nIdx].m_bShiftYUse;

		algoBlob.m_bUseBlobNG = pInspAlgo->sArrAlgoEdge[nIdx].m_bUseBlobNG;
		algoBlob.m_dBlobSizeWidth = pInspAlgo->sArrAlgoEdge[nIdx].m_dBlobSizeWidth;
		algoBlob.m_dBlobSizeLength = pInspAlgo->sArrAlgoEdge[nIdx].m_dBlobSizeLength;
		algoBlob.m_bUseBlobSizeWidth = pInspAlgo->sArrAlgoEdge[nIdx].m_bUseBlobSizeWidth;
		algoBlob.m_bUseBlobSizeLength = pInspAlgo->sArrAlgoEdge[nIdx].m_bUseBlobSizeLength;

		algoBlob.m_bUseHeight = pInspAlgo->sArrAlgoEdge[nIdx].m_bUseHeight;
		algoBlob.m_dTeachHeight = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachHeight;
		algoBlob.m_dTeachHeightMax = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachHeightMax;
		algoBlob.m_dTeachHeightMin = pInspAlgo->sArrAlgoEdge[nIdx].m_dTeachHeightMin;

		algoBlob.m_sAlgoColorBase = pInspAlgo->sArrAlgoEdge[nIdx].m_sAlgoColorBase;

		algoBlob.m_bIsModelSubImg = pInspAlgo->sArrAlgoEdge[nIdx].m_bIsModelSubImg;
		algoBlob.m_bUsePattern = pInspAlgo->sArrAlgoEdge[nIdx].m_bUsePattern;
		memcpy(algoBlob.m_sModelPath, pInspAlgo->sArrAlgoEdge[nIdx].m_sModelPath, MAX_STRLEN * sizeof(wchar_t));
		algoBlob.m_bUseFPBW = false;
		algoBlob.m_byFPMargin = 30;
		algoBlob.m_bFillHole = pInspAlgo->sArrAlgoEdge[nIdx].m_bFillHole;
		algoBlob.m_bCircleOpt = false;
	}
	else if(sInspAlgo.m_eAlgoType == eAlgoBody_Blob)
	{
		AlgoBodyBlob *pInspAlgo = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
		algoBlob.m_bUseIPC = pInspAlgo->m_bUseIPC;
		algoBlob.m_byIPCClass = pInspAlgo->m_byIPCClass;
		algoBlob.m_bInvertCheck = pInspAlgo->m_bInvertCheck;

		algoBlob.m_bFilterIsUse = pInspAlgo->m_bFilterIsUse;
		algoBlob.m_nFilterStepNarrow = pInspAlgo->m_nFilterStepNarrow;

		algoBlob.m_bShiftIsUse = pInspAlgo->m_bShiftIsUse;
		algoBlob.m_dShiftX = pInspAlgo->m_dShiftX;
		algoBlob.m_dShiftY = pInspAlgo->m_dShiftY;

		algoBlob.m_bAreaIsUse = pInspAlgo->m_bAreaIsUse;
		algoBlob.m_dAreaMin = pInspAlgo->m_dAreaMin;
		algoBlob.m_dAreaMax = pInspAlgo->m_dAreaMax;
		algoBlob.m_dAreaCurrent = pInspAlgo->m_dAreaCurrent;

		algoBlob.m_nTypeSelectBlob = pInspAlgo->m_nTypeSelectBlob;

		algoBlob.m_bInsp2D = pInspAlgo->m_bInsp2D;
		algoBlob.m_nTypeRange2D = pInspAlgo->m_nTypeRange2D;
		algoBlob.m_nMinBinary = pInspAlgo->m_nMinBinary;
		algoBlob.m_nMaxBinary = pInspAlgo->m_nMaxBinary;

		algoBlob.m_bInsp3D = pInspAlgo->m_bInsp3D;
		algoBlob.m_nTypeRange3D = pInspAlgo->m_nTypeRange3D;
		algoBlob.m_dHeightRateMin = pInspAlgo->m_dHeightRateMin;
		algoBlob.m_dHeightRateMax = pInspAlgo->m_dHeightRateMax;
		algoBlob.m_fHeightAvg = pInspAlgo->m_fHeightAvg;

		algoBlob.m_dTechCenterX = pInspAlgo->m_dTechCenterX;
		algoBlob.m_dTechCenterY = pInspAlgo->m_dTechCenterY;

		algoBlob.m_bTeachWidthUse = pInspAlgo->m_bTeachWidthUse;
		algoBlob.m_dTeachWidth = pInspAlgo->m_dTeachWidth;
		algoBlob.m_dTeachWidthRateMin = pInspAlgo->m_dTeachWidthRateMin;
		algoBlob.m_dTeachWidthRateMax = pInspAlgo->m_dTeachWidthRateMax;

		algoBlob.m_bTeachLengthUse = pInspAlgo->m_bTeachLengthUse;
		algoBlob.m_dTeachLength = pInspAlgo->m_dTeachLength;
		algoBlob.m_dTeachLengthRateMin = pInspAlgo->m_dTeachLengthRateMin;
		algoBlob.m_dTeachLengthRateMax = pInspAlgo->m_dTeachLengthRateMax;

		algoBlob.m_bShiftXUse = pInspAlgo->m_bShiftXUse;
		algoBlob.m_bShiftYUse = pInspAlgo->m_bShiftYUse;

		algoBlob.m_bUseBlobNG = pInspAlgo->m_bUseBlobNG;
		algoBlob.m_dBlobSizeWidth = pInspAlgo->m_dBlobSizeWidth;
		algoBlob.m_dBlobSizeLength = pInspAlgo->m_dBlobSizeLength;
		algoBlob.m_bUseBlobSizeWidth = pInspAlgo->m_bUseBlobSizeWidth;
		algoBlob.m_bUseBlobSizeLength = pInspAlgo->m_bUseBlobSizeLength;

		algoBlob.m_bUseHeight = pInspAlgo->m_bUseHeight;
		algoBlob.m_dTeachHeight = pInspAlgo->m_dTeachHeight;
		algoBlob.m_dTeachHeightMax = pInspAlgo->m_dTeachHeightMax;
		algoBlob.m_dTeachHeightMin = pInspAlgo->m_dTeachHeightMin;

		algoBlob.m_sAlgoColorBase = pInspAlgo->m_sAlgoColorBase;

		algoBlob.m_bIsModelSubImg = pInspAlgo->m_bIsModelSubImg;
		algoBlob.m_bUsePattern = FALSE;// pInspAlgo->m_bUsePattern;
		memcpy(algoBlob.m_sModelPath, pInspAlgo->m_sModelPath, MAX_STRLEN * sizeof(wchar_t));
		algoBlob.m_bUseFPBW = false;
		algoBlob.m_byFPMargin = 30;
		algoBlob.m_bFillHole = pInspAlgo->m_bFillHole;
		algoBlob.m_bCircleOpt = false;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoFoot)
	{
		AlgoFoot *pInspAlgo = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;
		algoBlob.m_bUseIPC = false;
		algoBlob.m_bInvertCheck = false;
		algoBlob.m_bFilterIsUse = false;
		algoBlob.m_nTypeSelectBlob = eSelectMix;
		algoBlob.m_bInsp3D = true;

		//if (nIdx == (int)m_eFootBin::m_eFootBin_Find || nIdx == (int)m_eFootBin::m_eFootBin_Shift)
		//	algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;
		//else
		//	algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectBigger;

		algoBlob.m_nTypeSelectBlob = ETypeBlob::eSelectMix;

		int Range3D = 0;
		if ((pInspAlgo->m_sArrBin[nIdx].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
			algoBlob.m_nTypeRange3D = 1;
		else if ((pInspAlgo->m_sArrBin[nIdx].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
			algoBlob.m_nTypeRange3D = 2;
		else if ((pInspAlgo->m_sArrBin[nIdx].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			algoBlob.m_nTypeRange3D = 3;
		algoBlob.m_bInsp3D = true;
		algoBlob.m_dHeightRateMin = pInspAlgo->m_sArrBin[nIdx].m_fArrH[m_eMMD::eMMD_Min];
		algoBlob.m_dHeightRateMax = pInspAlgo->m_sArrBin[nIdx].m_fArrH[m_eMMD::eMMD_Max];


		algoBlob.m_bInsp2D = (pInspAlgo->m_sArrBin[nIdx].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) > 0 ? true : false;
		if ((pInspAlgo->m_sArrBin[nIdx].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
			algoBlob.m_nTypeRange2D = 1;
		else if ((pInspAlgo->m_sArrBin[nIdx].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
			algoBlob.m_nTypeRange2D = 2;
		else if ((pInspAlgo->m_sArrBin[nIdx].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			algoBlob.m_nTypeRange2D = 3;
		algoBlob.m_nMinBinary = pInspAlgo->m_sArrBin[nIdx].m_byArrValue[m_eBin::m_eBin_Min2D];
		algoBlob.m_nMaxBinary = pInspAlgo->m_sArrBin[nIdx].m_byArrValue[m_eBin::m_eBin_Max2D];

		algoBlob.m_bFillHole = true;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoTab)
	{
		AlgoTab *pInspAlgo = (AlgoTab *)sInspAlgo.m_ptrInspAlgoParam;
	
		algoBlob.m_bInsp2D = ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2D) == m_enBlobBase_Data::m_enBlobBase_Data_e2D );
		algoBlob.m_nMinBinary = pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Min2D];
		algoBlob.m_nMaxBinary = pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Max2D];

		if ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeIN) != 0)
			algoBlob.m_nTypeRange2D = eTypeRangeIn;
		else if ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeUP) != 0)
			algoBlob.m_nTypeRange2D = eTypeRangeUpper;
		else if ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeLo) != 0)
			algoBlob.m_nTypeRange2D = eTypeRangeLower;
		else
			algoBlob.m_nTypeRange2D = eTypeRangeOut;

		algoBlob.m_bInsp3D = (pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3D) == m_enBlobBase_Data::m_enBlobBase_Data_e3D ? 1 : 0;
		algoBlob.m_dHeightRateMin = pInspAlgo->m_sBlobBase_TabArea.m_fArrValue[m_efBlobBase::m_efBlobBase_Min3D];
		algoBlob.m_dHeightRateMax = pInspAlgo->m_sBlobBase_TabArea.m_fArrValue[m_efBlobBase::m_efBlobBase_Max3D];
		if ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeIN) != 0)
			algoBlob.m_nTypeRange3D = eTypeRangeIn;
		else if ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeUP) != 0)
			algoBlob.m_nTypeRange3D = eTypeRangeUpper;
		else if ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeLo) != 0)
			algoBlob.m_nTypeRange3D = eTypeRangeLower;
		else
			algoBlob.m_nTypeRange3D = eTypeRangeOut;

		algoBlob.m_bFillHole = ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_eFillHole) == m_enBlobBase_Data::m_enBlobBase_Data_eFillHole);
		algoBlob.m_bFilterIsUse = (pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_eFilter) == m_enBlobBase_Data::m_enBlobBase_Data_eFilter;
		algoBlob.m_nFilterStepNarrow = (pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Filter]);
// 		
// 		if ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_eMaxBlob) != 0)
// 			algoBlob.m_nTypeSelectBlob = eSelectBigger;
// 		else
// 			algoBlob.m_nTypeSelectBlob = eSelectMix;
		algoBlob.m_nTypeSelectBlob = eSelectBigger;
		algoBlob.m_sAlgoColorBase.m_bUseColor = pInspAlgo->m_sBlobBase_TabArea.m_sAlgoColorBase.m_bUseColor;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoShapeX)
	{
		AlgoShapeX *pInspAlgo = (AlgoShapeX *)sInspAlgo.m_ptrInspAlgoParam;

		algoBlob.m_bInsp2D = ((pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2D) == m_enBlobBase_Data::m_enBlobBase_Data_e2D);
		algoBlob.m_nMinBinary = pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Min2D];
		algoBlob.m_nMaxBinary = pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Max2D];

		if ((pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeIN) != 0)
			algoBlob.m_nTypeRange2D = eTypeRangeIn;
		else if ((pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeUP) != 0)
			algoBlob.m_nTypeRange2D = eTypeRangeUpper;
		else if ((pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e2DRangeLo) != 0)
			algoBlob.m_nTypeRange2D = eTypeRangeLower;
		else
			algoBlob.m_nTypeRange2D = eTypeRangeOut;

		algoBlob.m_bInsp3D = (pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3D) == m_enBlobBase_Data::m_enBlobBase_Data_e3D ? 1 : 0;
		algoBlob.m_dHeightRateMin = pInspAlgo->m_sBlobBase_ShapeArea.m_fArrValue[m_efBlobBase::m_efBlobBase_Min3D];
		algoBlob.m_dHeightRateMax = pInspAlgo->m_sBlobBase_ShapeArea.m_fArrValue[m_efBlobBase::m_efBlobBase_Max3D];
		if ((pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeIN) != 0)
			algoBlob.m_nTypeRange3D = eTypeRangeIn;
		else if ((pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeUP) != 0)
			algoBlob.m_nTypeRange3D = eTypeRangeUpper;
		else if ((pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_e3DRangeLo) != 0)
			algoBlob.m_nTypeRange3D = eTypeRangeLower;
		else
			algoBlob.m_nTypeRange3D = eTypeRangeOut;

		algoBlob.m_bFillHole = ((pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_eFillHole) == m_enBlobBase_Data::m_enBlobBase_Data_eFillHole);
		algoBlob.m_bFilterIsUse = (pInspAlgo->m_sBlobBase_ShapeArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_eFilter) == m_enBlobBase_Data::m_enBlobBase_Data_eFilter;
		// 		
		// 		if ((pInspAlgo->m_sBlobBase_TabArea.m_nArrValue[m_enBlobBase::m_enBlobBase_Dt] & m_enBlobBase_Data::m_enBlobBase_Data_eMaxBlob) != 0)
		// 			algoBlob.m_nTypeSelectBlob = eSelectBigger;
		// 		else
		// 			algoBlob.m_nTypeSelectBlob = eSelectMix;
		algoBlob.m_nTypeSelectBlob = eSelectBigger;
		algoBlob.m_sAlgoColorBase.m_bUseColor = pInspAlgo->m_sBlobBase_ShapeArea.m_sAlgoColorBase.m_bUseColor;
	}

	return algoBlob;
}

int CPInsp::GetCorrectIDX(int nCnt, double* dArrSrc, int* nArrIDX, bool bMax)
{
	if (nCnt <= 0 || dArrSrc == NULL || nArrIDX == NULL) return 0;
	for (int a = 0; a < nCnt; a++)
		nArrIDX[a] = -1;
	for (int a = 0; a < nCnt; a++)
	{
		double dValue = 0;
		int nIdx = -1;
		for (int b = 0; b < nCnt; b++)
		{
			bool bCheck = true;
			for (int c = 0; c < nCnt; c++)
			{
				if (nArrIDX[c] == b)
				{
					bCheck = false;
					break;
				}
			}
			if (bCheck == false)
				continue;
			if ((!bMax && (nIdx == -1 || (nIdx > -1 && dValue > dArrSrc[b]))) ||
				(bMax && (nIdx == -1 || (nIdx > -1 && dValue < dArrSrc[b]))))
			{
				dValue = dArrSrc[b];
				nIdx = b;
			}
		}
		if (nIdx == -1) return 0;
		nArrIDX[a] = nIdx;
	}
	for (int a = 0; a < nCnt; a++)
	{
		bool bReset = true;
		for (int b = 0; b < nCnt; b++)
		{
			if (nArrIDX[b] == a)
			{
				bReset = false;
				break;
			}
		}
		if (bReset)
		{
			for (int b = 0; b < nCnt; b++)
				nArrIDX[b] = b;
			break;
		}
	}
	return 1;
}
void CPInsp::GetLabelImage(long lLabel, USHORT *LabelImage, int nW, int nH, CString sFileName)
{
#if _DEBUG
	if (lLabel < 0 || LabelImage == NULL || nW <= 0 || nH <= 0)
		return;
	UCHAR *ucImage = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ucImage, nW * nH);
	memset(ucImage, 0, nW * nH * sizeof(UCHAR));
	for (int a = 0; a < nW * nH; a++)
	{
		if (lLabel == LabelImage[a])
			ucImage[a] = 255;
	}
	m_pProcMilAlgo->SaveWorkImg(ucImage, nW, nH, sFileName);
	Delete_1DArray(&ucImage);
#endif
}
RECT CPInsp::GetBlobRect(long *pLebel, USHORT* LabelImage, int nCntBlob, int nW, int nH, int nL, int nR, int nT, int nB)
{
	RECT rcArea;
	rcArea.left = -1;
	rcArea.right = -1;
	rcArea.top = -1;
	rcArea.bottom = -1;
	if (pLebel == NULL || LabelImage == NULL || nCntBlob <= 0 || nW <= 0 || nH <= 0 ||
		nL < 0 || nT < 0 || nR < 0 || nB < 0 || nL >= nW || nT >= nH || nR >= nW || nB >= nH)
		return rcArea;
	for (int a = 0; a < nCntBlob; a++)
	{
		RECT rcLabel;
		rcLabel.left = -1;
		rcLabel.right = -1;
		rcLabel.top = -1;
		rcLabel.bottom = -1;
		for (int y = 0; y < nH; y++)
		{
			for (int x = 0; x < nW; x++)
			{
				if (pLebel[a] != LabelImage[y * nW + x])
					continue;
				if (rcLabel.left == -1 || x < rcLabel.left) rcLabel.left = x;
				if (x > rcLabel.right) rcLabel.right = x;
			}
		}
		for (int x = 0; x < nW; x++)
		{
			for (int y = 0; y < nH; y++)
			{
				if (pLebel[a] != LabelImage[y * nW + x])
					continue;
				if (rcLabel.top == -1 || y < rcLabel.top) rcLabel.top = y;
				if (y > rcLabel.bottom) rcLabel.bottom = y;
			}
		}
		if (rcLabel.left == -1 || rcLabel.right == -1 || rcLabel.top == -1 || rcLabel.bottom == -1)
			continue;
		if (nL >= rcLabel.right || nR <= rcLabel.left || nT >= rcLabel.bottom || nB <= rcLabel.top)
			continue;
		if (rcArea.left == -1 || rcArea.left > rcLabel.left)
			rcArea.left = rcLabel.left;
		if (rcArea.right < rcLabel.right)
			rcArea.right = rcLabel.right;
		if (rcArea.top == -1 || rcArea.top > rcLabel.top)
			rcArea.top = rcLabel.top;
		if (rcArea.bottom < rcLabel.bottom)
			rcArea.bottom = rcLabel.bottom;
	}
	return rcArea;
}

void CPInsp::ExceptPointed(cv::Mat* binImg,float* HSrc)
{
	cv::Mat HImg(binImg->rows,binImg->cols,CV_32FC1,HSrc);
	cv::Mat hX,hY;
	cv::Sobel(HImg,hX,CV_32FC1,1,0);
	cv::Sobel(HImg,hY,CV_32FC1,0,1);

	for(int r=0;r<HImg.rows;r++)
	{
		float* XPtr = hX.ptr<float>(r);
		float* YPtr = hY.ptr<float>(r);
		UCHAR* ImgPtr = binImg->ptr(r);
		for(int c=0;c<HImg.cols;c++)
		{
			if(XPtr[c]>20||YPtr[c]>20)
			{
				ImgPtr[c] = 0;
			}
		}
	}
}
cv::Mat CPInsp::FillPolygon(cv::Mat binImg, int width, int height, POINT* ptDstPoint, bool isExceptROI, int nUsedInspPolygon, int* cnt, UCHAR* ucArrOverlapImg,int nChanel)
{
	cv::Scalar black(0, 0, 0);
	cv::Scalar white(255, 255, 255);

	cv::Mat img;
	cv::Mat OverlapImg;
	vector<cv::Point> contour;
	for(int i=0; i < nUsedInspPolygon; i++)
	{
		if((ptDstPoint[i].x >=0 && ptDstPoint[i].y>=0) && (ptDstPoint[i].x <=width && ptDstPoint[i].y<=height))
			contour.push_back(cv::Point(ptDstPoint[i].x,ptDstPoint[i].y));
	}

	const cv::Point *pts3 = (cv::Point*) cv::Mat(contour).data;
	int npts3 = cv::Mat(contour).rows;
	int ZeroCnt = 0; 
	bool isExceptROIRect = false;
	if(*cnt == 1)
	{
		isExceptROIRect = true;
		npts3 = 4;
	}
	if(ucArrOverlapImg != NULL)
	{
		int nColorTye = nChanel==3?0:1;
		OverlapImg = m_pProcMilAlgo->BytesToMat(ucArrOverlapImg,width,height,nColorTye);
	}

	if(isExceptROI)
	{
		if(!isExceptROIRect)
		{
			img = cv::Mat(height, width, CV_8UC(nChanel), white);
			fillPoly(img, &pts3, &npts3, 1,black);		
			if(ucArrOverlapImg != NULL)
				fillPoly(OverlapImg, &pts3, &npts3, 1,black);
		}
		else
		{
			img = cv::Mat(height, width, CV_8UC(nChanel), white);
			for(int i =0; i < nUsedInspPolygon ; i +=4)
			{				
				cv::rectangle(img, pts3[i], pts3[i+2],black,-1);		
				if(ucArrOverlapImg != NULL)
					cv::rectangle(OverlapImg, pts3[i], pts3[i+2], black,-1);
			}
		}
	}
	else
	{
		img = cv::Mat(height, width, CV_8UC(nChanel), black);
		fillPoly(img, &pts3, &npts3, 1,white);	
		if(ucArrOverlapImg != NULL)
			fillPoly(OverlapImg, &pts3, &npts3, 1,white);
	}	

	if(binImg.size == img.size)
	{
		cv::bitwise_and(img, binImg,img);
	}

	if(ucArrOverlapImg != NULL)
		memcpy(ucArrOverlapImg,m_pProcMilAlgo->MatToBytes(OverlapImg,ucArrOverlapImg),width*height* sizeof(byte)*nChanel);

	*cnt = ZeroCnt;
	binImg = img;
	pts3 = nullptr;

	img.release();
	OverlapImg.release();

	return binImg;
}
void CPInsp::ROIAnglePointChange(double dAngle, const POINTF rcSrc, POINTF *rcDst)
{
	double temp = 0;
	temp = dAngle / 90.0;
	temp = dAngle - (int)temp * 90;
	if (temp != 0)   // 일반각
	{
		rcDst->x = rcSrc.x;
		rcDst->y = rcSrc.y;
		return;
	}
	int nRelativeAngle = ((int)dAngle + 360) % 360;
	switch (nRelativeAngle)
	{
	case 0:
		{
			rcDst->x = rcSrc.x;
			rcDst->y = rcSrc.y;
		}
		break;

	case 90:
		{
			rcDst->x = rcSrc.y;
			rcDst->y = -rcSrc.x;
		}
		break;
	case 180:
		{
			rcDst->x = -rcSrc.x;
			rcDst->y = -rcSrc.y;
		}
		break;
	case 270:
		{
			rcDst->x = -rcSrc.y;
			rcDst->y = rcSrc.x;
		}
		break;
	default:
		{
			rcDst->x = rcSrc.x;
			rcDst->y = rcSrc.y;
		}
		break;
	}
}

int CPInsp::BlobImageStruct_Base(AlgoBlobBase &algo, UCHAR *pucImgSrc, float *pfImgSrc, UCHAR *ucArrColorImg, int nWidth, int nHeight, TotalInspExceptArea stTieArea, UCHAR *pucImgDst)
{
	bool b2D = ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2D) == m_enBlobBase_Data_e2D);
	bool b3D = ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3D) == m_enBlobBase_Data_e3D);
	bool bColor = algo.m_sAlgoColorBase.m_bUseColor;
	if ((pucImgSrc == NULL && b2D) || (pfImgSrc == NULL && b3D) || (nWidth <= 0) || (nHeight <= 0) || (pucImgDst == NULL))
		return 0;
	if (!b2D && (!b3D && g_pMPTI->m_bSideOriginalSize != true) && !bColor)
		return 0;
	int nArea = nWidth * nHeight;
	UCHAR *puc2D = NULL;
	UCHAR *puc3D = NULL;
	UCHAR *pucBin = g_pMManager->pem_new<UCHAR>(true, nArea, (PCHAR)__FUNCTION__, __LINE__);
	memset(pucBin, 0, nArea * sizeof(UCHAR));
	bool bIn = false;
	bool bInvert = ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eInvert) == m_enBlobBase_Data_eInvert);
	if(b2D)
	{
		int nMin = algo.m_nArrValue[m_enBlobBase_Min2D];
		int nMax = algo.m_nArrValue[m_enBlobBase_Max2D];
		int nRange = (int)ETypeInspRange::eTypeRangeOut;
		if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2DRangeUP) == m_enBlobBase_Data_e2DRangeUP)
			nRange = (int)ETypeInspRange::eTypeRangeUpper;
		else if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2DRangeLo) == m_enBlobBase_Data_e2DRangeLo)
			nRange = (int)ETypeInspRange::eTypeRangeLower;
		else if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2DRangeIN) == m_enBlobBase_Data_e2DRangeIN)
			nRange = (int)ETypeInspRange::eTypeRangeIn;
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nWidth, nHeight, _T("Org_2D.bmp"));
		CPInsp::Binarize(pucImgSrc, nWidth, nHeight, nRange, nMin, nMax, bInvert, puc2D);
		m_pProcMilAlgo->SaveWorkImg(puc2D, nWidth, nHeight, _T("Bin_2D.bmp"));
		if (b3D == false && ucArrColorImg == NULL)
		{
			memcpy(pucBin, puc2D, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if(b3D)
	{
		float fMin = algo.m_fArrValue[m_efBlobBase_Min3D];
		float fMax = algo.m_fArrValue[m_efBlobBase_Max3D];
		int nRange = (int)ETypeInspRange::eTypeRangeOut;
		if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3DRangeUP) == m_enBlobBase_Data_e3DRangeUP)
			nRange = (int)ETypeInspRange::eTypeRangeUpper;
		else if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3DRangeLo) == m_enBlobBase_Data_e3DRangeLo)
			nRange = (int)ETypeInspRange::eTypeRangeLower;
		else if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3DRangeIN) == m_enBlobBase_Data_e3DRangeIN)
			nRange = (int)ETypeInspRange::eTypeRangeIn;
		m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nWidth, nHeight, _T("Org_3D.bmp"));
		CPInsp::Binarize(pfImgSrc, nWidth, nHeight, nRange, fMin, fMax, bInvert, puc3D);
		m_pProcMilAlgo->SaveWorkImg(puc3D, nWidth, nHeight, _T("Bin_3D.bmp"));
		if (b2D == false && ucArrColorImg == NULL)
		{
			memcpy(pucBin, puc3D, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (bColor)
	{
		if (b2D == false && b3D == false)
		{
			memcpy(pucBin, ucArrColorImg, nArea * sizeof(UCHAR));
			bIn = true;
		}
	}
	if (bIn == false)
	{
		for (int nIndex = 0; nIndex < nArea; nIndex++)
		{
			UCHAR uc2dValue = (b2D == TRUE && puc2D) ? puc2D[nIndex] : 255;
			UCHAR uc3dValue = (b3D == TRUE && puc3D) ? puc3D[nIndex] : 255;
			UCHAR ucColorValue = (bColor == TRUE && ucArrColorImg) ? ucArrColorImg[nIndex] : 255;
			if ((bInvert == TRUE && (uc2dValue == 255 || uc3dValue == 255 || ucColorValue == 255)) ||
				(bInvert == FALSE && (uc2dValue == 255 && uc3dValue == 255 && ucColorValue == 255)))
				pucBin[nIndex] = 255;
		}
	}
	m_pProcMilAlgo->SaveWorkImg(pucBin, nWidth, nHeight, _T("Bin2D3DColor.bmp"));
	if (puc2D) Delete_1DArray(&puc2D);
	if (puc3D) Delete_1DArray(&puc3D);
	CPInsp::FillOutOfInspAreaCombine(nWidth, nHeight, 0, pucBin, NULL, stTieArea);
	int nFilter = 0;
	if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eFilter) == m_enBlobBase_Data_eFilter)
		nFilter = algo.m_nArrValue[m_enBlobBase_Filter];
	int nMinArea = algo.m_nArrValue[m_enBlobBase_MinArea];
	if (nMinArea < 4) nMinArea = 4;
	BOOL bFillHole = ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eFillHole) == m_enBlobBase_Data_eFillHole);
	int nType = eSelectMix;
	if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eMaxBlob) == m_enBlobBase_Data_eMaxBlob)
		nType = eSelectBigger;
	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select(pucBin, pucImgDst, nWidth, nHeight, nMinArea, FALSE, bFillHole, nFilter, nType);
	if(puc2D) Delete_1DArray(&puc2D);
	if(puc3D) Delete_1DArray(&puc3D);
	if(pucBin) Delete_1DArray(&pucBin);
	return nCntBlob;
}
int CPInsp::BlobImageStruct_Base(AlgoBlobBase &algo, cv::Mat img2D, cv::Mat img3D, cv::Mat imgColor, int nWidth, int nHeight, TotalInspExceptArea stTieArea, cv::Mat imgRst, bool bBin)
{
	bool b2D = ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2D) == m_enBlobBase_Data_e2D);
	bool b3D = ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3D) == m_enBlobBase_Data_e3D);
	bool bColor = algo.m_sAlgoColorBase.m_bUseColor;
	bool bInvert = ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eInvert) == m_enBlobBase_Data_eInvert);

	if (!b2D && (!b3D && g_pMPTI->m_bSideOriginalSize != true) && !bColor)
		return 0;

	if ((img2D.empty() && b2D) || (img3D.empty() && b3D) || (imgColor.empty() && bColor) || (nWidth <= 0) || (nHeight <= 0) || (imgRst.empty()))
		return 0;

	cv::Mat imgBin(nHeight, nWidth, CV_8UC1, cv::Scalar(255));
	if (b2D)
	{
		int nMin = algo.m_nArrValue[m_enBlobBase_Min2D];
		int nMax = algo.m_nArrValue[m_enBlobBase_Max2D];
		int nRange = (int)ETypeInspRange::eTypeRangeOut;
		if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2DRangeUP) == m_enBlobBase_Data_e2DRangeUP)
			nRange = (int)ETypeInspRange::eTypeRangeUpper;
		else if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2DRangeLo) == m_enBlobBase_Data_e2DRangeLo)
			nRange = (int)ETypeInspRange::eTypeRangeLower;
		else if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e2DRangeIN) == m_enBlobBase_Data_e2DRangeIN)
			nRange = (int)ETypeInspRange::eTypeRangeIn;

		UCHAR *pucTemp = NULL;
		CPInsp::Binarize(img2D.data, nWidth, nHeight, nRange, nMin, nMax, bInvert, pucTemp);

		cv::Mat imgTemp(nHeight, nWidth, CV_8UC1, pucTemp);
		cv::bitwise_and(imgTemp, imgBin, imgBin);
		if (pucTemp) Delete_1DArray(&pucTemp);
	}
	if (b3D)
	{
		float fMin = algo.m_fArrValue[m_efBlobBase_Min3D];
		float fMax = algo.m_fArrValue[m_efBlobBase_Max3D];
		int nRange = (int)ETypeInspRange::eTypeRangeOut;
		if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3DRangeUP) == m_enBlobBase_Data_e3DRangeUP)
			nRange = (int)ETypeInspRange::eTypeRangeUpper;
		else if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3DRangeLo) == m_enBlobBase_Data_e3DRangeLo)
			nRange = (int)ETypeInspRange::eTypeRangeLower;
		else if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_e3DRangeIN) == m_enBlobBase_Data_e3DRangeIN)
			nRange = (int)ETypeInspRange::eTypeRangeIn;

		UCHAR *pucTemp = NULL;
		CPInsp::Binarize(img3D.ptr<float>(), nWidth, nHeight, nRange, fMin, fMax, bInvert, pucTemp);

		cv::Mat imgTemp(nHeight, nWidth, CV_8UC1, pucTemp);
		cv::bitwise_and(imgTemp, imgBin, imgBin);
		if (pucTemp) Delete_1DArray(&pucTemp);
	}
	if (bColor)
		cv::bitwise_and(imgColor, imgBin, imgBin);

	CPInsp::FillOutOfInspAreaCombine(nWidth, nHeight, 0, imgBin.data, NULL, stTieArea);

	int nType = eSelectMix;
	if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eMaxBlob) == m_enBlobBase_Data_eMaxBlob)
		nType = eSelectBigger;

	if (bBin && nType == eSelectMix)
	{
		memcpy(imgRst.data, imgBin.data, nWidth*nHeight * sizeof(UCHAR));
		return 1;
	}

	int nFilter = 0;
	if ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eFilter) == m_enBlobBase_Data_eFilter)
		nFilter = algo.m_nArrValue[m_enBlobBase_Filter];

	int nMinArea = algo.m_nArrValue[m_enBlobBase_MinArea];
	if (nMinArea < 4) nMinArea = 4;

	BOOL bFillHole = ((algo.m_nArrValue[m_enBlobBase_Dt] & m_enBlobBase_Data_eFillHole) == m_enBlobBase_Data_eFillHole);

	int nCntBlob = m_pProcMilAlgo->CalcBlob_Select_new(imgBin, imgRst, nWidth, nHeight, nMinArea, FALSE, bFillHole, nFilter, nType);
	return nCntBlob;
}
void CPInsp::GetBlobPolygon(UCHAR * uArrDst, int nWidth, int nHeight, int nStartX, int nStartY, vector<vector<POINTF>>* vPolygons, vector<POINTF>* vPolyCenter)
{
	m_pProcMilAlgo->GetPolygon(uArrDst, nWidth, nHeight, nStartX, nStartY, vPolygons, vPolyCenter);
}
int CPInsp::BlobFillOutImage(AlgoBlob &algoBlob, UCHAR *pucImgSrc, float *pfImgSrc, UCHAR *ucArrColorImg, int nWidth, int nHeight, int nMinBlobArea, 
	double *dArea, double *dCx, double *dCy, CRect *rcBlob, UCHAR *ucArrDstImg, TotalInspExceptArea stTieArea,
	BOOL bApplyFillHole/*=TRUE*/, BOOL bAlignAlgo/*= FALSE*/, int nTeachX/* = -1*/, int nTeachY/* = -1*/, BOOL eraseBorderBlob, double dAreaPix, double dAreaWPix, double dAreaHPix, int nAreaCnt, UCHAR* ucArrFillOutImg, UCHAR* ucBinImg, UCHAR* ucMask)
{
	if((pucImgSrc == NULL) || (pfImgSrc == NULL && g_pMPTI->m_bSideOriginalSize != true) || (nWidth <= 0) ||(nHeight <= 0) || (!algoBlob.m_bInsp2D && (!algoBlob.m_bInsp3D && g_pMPTI->m_bSideOriginalSize != true) && !algoBlob.m_sAlgoColorBase.m_bUseColor))
		return 1;

	UCHAR *pUcImgDst2D = NULL;
	UCHAR *pUcImgDst3D = NULL;
	UCHAR *pUcImgBinary = NULL;

	// 1. Binarize
	if(algoBlob.m_bInsp2D)
	{
		m_pProcMilAlgo->SaveWorkImg(pucImgSrc, nWidth, nHeight, _T("Origin2D.bmp"));

		CPInsp::Binarize(pucImgSrc, nWidth, nHeight, algoBlob.m_nTypeRange2D, 
			algoBlob.m_nMinBinary, algoBlob.m_nMaxBinary, algoBlob.m_bInvertCheck, pUcImgDst2D);

		if(algoBlob.m_bInsp2D && !algoBlob.m_bInsp3D)
			pUcImgBinary = pUcImgDst2D;
		m_pProcMilAlgo->SaveWorkImg(pUcImgBinary, nWidth, nHeight, _T("Bin2D.bmp"));
	}
	if(algoBlob.m_bInsp3D)
	{
		//m_pProcMilAlgo->SaveWorkImg_float(pfImgSrc, nImgWidth, nImgHeight, _T("Origin3D.bmp"), 3);
		float fMinHeight = algoBlob.m_dHeightRateMin;
		float fMaxHeight = algoBlob.m_dHeightRateMax;

		CPInsp::Binarize(pfImgSrc, nWidth, nHeight, algoBlob.m_nTypeRange3D, 
			fMinHeight, fMaxHeight, algoBlob.m_bInvertCheck, pUcImgDst3D);
		if(!algoBlob.m_bInsp2D && algoBlob.m_bInsp3D)
			pUcImgBinary = pUcImgDst3D;
		m_pProcMilAlgo->SaveWorkImg(pUcImgBinary, nWidth, nHeight, _T("Bin3D.bmp"));
	}

	if(pUcImgDst2D && pUcImgDst3D)
	{
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pUcImgBinary, nWidth * nHeight);
		for(int y = 0; y < nHeight; y++)
		{
			for(int x = 0; x < nWidth; x++)
			{
				int index = y * nWidth + x;

				pUcImgBinary[index] = ((pUcImgDst2D[index] == 255) && (pUcImgDst3D[index] == 255)) ? 255 : 0;
			}
		}
		m_pProcMilAlgo->SaveWorkImg(pUcImgBinary, nWidth, nHeight, _T("Bin2D3D.bmp"));
		if(ucArrColorImg == NULL)
		{
			Delete_1DArray(&pUcImgDst2D);
			Delete_1DArray(&pUcImgDst3D);
		}
	}
	if(ucArrColorImg != NULL)
	{
		if(pUcImgBinary == NULL)
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pUcImgBinary, nWidth * nHeight);
		for(int y = 0; y < nHeight; y++)
		{
			for(int x = 0; x < nWidth; x++)
			{
				int index = y * nWidth + x;
				if((algoBlob.m_bInsp2D && pUcImgDst2D) || (algoBlob.m_bInsp3D && pUcImgDst3D))
					pUcImgBinary[index] = ((pUcImgBinary[index] == 255) && (ucArrColorImg[index] == 255)) ? 255 : 0;
				else
					pUcImgBinary[index] = ucArrColorImg[index];
			}
		}
		m_pProcMilAlgo->SaveWorkImg(pUcImgBinary, nWidth, nHeight, _T("Bin2D3DColor.bmp"));
		if(pUcImgDst2D && pUcImgDst3D)
		{
			Delete_1DArray(&pUcImgDst2D);
			Delete_1DArray(&pUcImgDst3D);
		}
	}

	// KIY 2020/04/23 : 폴리곤 통합
	int nFillOut = 0;
	nFillOut = CPInsp::FillOutOfInspAreaCombineImg(nWidth, nHeight, 0, pUcImgBinary, NULL, stTieArea, ucArrFillOutImg, ucBinImg, ucMask);
	Delete_1DArray(&pUcImgBinary);

	return nFillOut;
}

int CPInsp::FillOutOfInspAreaCombineImg(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, TotalInspExceptArea stTieArea, UCHAR* ucArrFillOutImg, UCHAR* ucBinImg, UCHAR* ucMask)
{
	// case 1 : window polygon 연산
	// case 2 : except polygon 연산
	// case 3 : ExceptROI 
	// CPInsp::FillMaskingROI(nImgWidth, nImgHeight, sWndAlgoImg.dAngle, pUcImgSrc, stTieArea.m_nUsedMaskingValue, stTieArea.m_rcArrMaskingROI, 0, pUcOverlapImg);
	if (!ucArrSrcImg)
		return 0;
	if (stTieArea.m_nUsedWndPolygon < 1 && stTieArea.m_nUsedInspPolygon < 1 && stTieArea.m_nUsedMaskingValue < 1)
		return 0;
	int ExceptCnt = 1;
	BOOL bCheckIngFALSE = FALSE;
	//CString str;
	cv::Mat srcOverlapImg;
	cv::Mat OverlapImg;
	if (ucArrOverlapImg != NULL)
		srcOverlapImg = m_pProcMilAlgo->BytesToMat(ucArrOverlapImg, nWidth, nHeight, 1);
	// case 1 
	if (!(stTieArea.m_nUsedWndPolygon < 1) && (stTieArea.m_ptArrWndPolygon != NULL))
		FillOutOfInspAreaImg(nWidth, nHeight, dAngle, ucArrSrcImg, ucArrOverlapImg, stTieArea.m_nUsedWndPolygon, stTieArea.m_ptArrWndPolygon, 0, FALSE, 1, ucArrFillOutImg, ucBinImg, ucMask, 0);
	else
	{
		if (ucArrOverlapImg != NULL)
		{
			bitwise_not(srcOverlapImg, srcOverlapImg);
			memcpy(ucArrOverlapImg, m_pProcMilAlgo->MatToBytes(srcOverlapImg, ucArrOverlapImg), nWidth*nHeight * sizeof(byte));
		}
	}
	// case 2 
	if (!(stTieArea.m_nUsedInspPolygon < 1) && (stTieArea.m_ptArrInspPolygon != NULL))
		FillOutOfInspAreaImg(nWidth, nHeight, dAngle, ucArrSrcImg, ucArrOverlapImg, stTieArea.m_nUsedInspPolygon, stTieArea.m_ptArrInspPolygon, 0, FALSE, 1, ucArrFillOutImg, ucBinImg, ucMask);
	// case 3 
	if(!(stTieArea.m_nUsedMaskingValue < 1) && (stTieArea.m_rcArrMaskingROI != NULL))
	//if(!(stTieArea.m_nUsedMaskingValue < 1) && !(stTieArea.m_rcArrMaskingROI.size() < 1))
		FillMaskingROIUsingOpenCVImg(nWidth, nHeight, 0, ucArrSrcImg, stTieArea.m_nUsedMaskingValue, stTieArea.m_rcArrMaskingROI, 0, ucArrOverlapImg, 1, ucArrFillOutImg, ucMask);
	return 1;
}
void CPInsp::FillMaskingROIUsingOpenCVImg(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const RECT *rcArrMaskingROI, int nFillValue, UCHAR *ucArrOverlapImg, int nChanel, UCHAR *ucArrFillOutImg, UCHAR *ucMask)
{
	if(nUsedMaskingValue < 1 || rcArrMaskingROI == NULL || !ucArrSrcImg)
		return  ;
	UCHAR* pUcArrTemp = NULL;

	BOOL bUseOverlapCnt = ucArrOverlapImg != NULL ? TRUE : FALSE;
	int cnt = 0;
	int nUsedPointCnt = nUsedMaskingValue*4;

	/*POINT* ptDstPoint = new POINT[nUsedPointCnt];
	POINT* ROIPoint = new POINT[nUsedPointCnt];
	POINTF* ptfDstPoint = new POINTF[nUsedPointCnt];*/
	POINT* ptDstPoint = g_pMManager->pem_new<POINT>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);
	POINT* ROIPoint = g_pMManager->pem_new<POINT>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);
	POINTF* ptfDstPoint = g_pMManager->pem_new<POINTF>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);

	if(dAngle == 90 || dAngle == 270)
	{
		dAngle -= 360;
		if(dAngle<0)
			dAngle +=180;
	}

	for (int a = 0; a < nUsedMaskingValue; a ++)
	{
		int nROIWidth = rcArrMaskingROI[a].right - rcArrMaskingROI[a].left;
		if (nROIWidth <= 0)
			continue;
		int nROIHeight = rcArrMaskingROI[a].bottom - rcArrMaskingROI[a].top;
		if (nROIHeight <= 0)
			continue;
		int nStartX = rcArrMaskingROI[a].left;
		int nStartY = rcArrMaskingROI[a].top;
		int index = a * 4;
		ROIPoint[index].x = nStartX;
		ROIPoint[index++].y = nStartY;
		ROIPoint[index].x = nStartX + nROIWidth;
		ROIPoint[index++].y = nStartY;
		ROIPoint[index].x = nStartX + nROIWidth;
		ROIPoint[index++].y = nStartY + nROIHeight;
		ROIPoint[index].x = nStartX;
		ROIPoint[index++].y = nStartY + nROIHeight;
	}


	for (int a = 0; a < nUsedPointCnt; a++)
	{
		ptfDstPoint[a].x = ROIPoint[a].x;
		ptfDstPoint[a].y = ROIPoint[a].y;
		if(dAngle!=0)
		{			
			AnglePosChange(dAngle, nWidth, nHeight, ptfDstPoint[a], &ptfDstPoint[a]);
			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}
		else
		{
			ptDstPoint[a].x = ptfDstPoint[a].x + (nWidth/2.);
			ptDstPoint[a].y = ptfDstPoint[a].y + (nHeight/2.);
		}
		if(ptDstPoint[a].x < 0)
			ptDstPoint[a].x = 0;
		if(ptDstPoint[a].y < 0)
			ptDstPoint[a].y = 0;
		if(ptDstPoint[a].x > nWidth)
			ptDstPoint[a].x = nWidth;
		if(ptDstPoint[a].y > nHeight)
			ptDstPoint[a].y = nHeight;
	}

	//UCHAR* ReturnImg = new UCHAR[nWidth*nHeight*nChanel];
	UCHAR* ReturnImg = g_pMManager->pem_new<UCHAR>(true, nWidth*nHeight*nChanel, (PCHAR)__FUNCTION__, __LINE__);
	//int *ZeroCnt = (int*)malloc(sizeof(int));
	int *ZeroCnt = (int*)g_pMManager->pem_malloc(sizeof(int), (PCHAR)__FUNCTION__, __LINE__);
	*ZeroCnt = 1;

	int nColorTye = nChanel==3?0:1;
	cv::Mat src = m_pProcMilAlgo->BytesToMat(ucArrSrcImg,nWidth,nHeight,nColorTye);
	cv::Mat ucMask2 = m_pProcMilAlgo->BytesToMat(ucMask, nWidth, nHeight, nColorTye);

	src = FillPolygonImg(ucMask2,src,nWidth,nHeight,ptDstPoint,true, nUsedPointCnt, ZeroCnt, ucArrOverlapImg,nChanel);
	memcpy(ucMask, ucMask2.data, nWidth*nHeight * sizeof(byte)*nChanel);
	memcpy(ucArrSrcImg,m_pProcMilAlgo->MatToBytes(src,ReturnImg),nWidth*nHeight* sizeof(byte)*nChanel);

	cnt = *ZeroCnt;

	/*if(ReturnImg)
		delete[] ReturnImg;
	ReturnImg = NULL;
	if(ptDstPoint)
		delete [] ptDstPoint;
	ptDstPoint = NULL;
	if(ROIPoint)
		delete [] ROIPoint;
	ROIPoint = NULL;
	if(ptfDstPoint)
		delete [] ptfDstPoint;
	ptfDstPoint = NULL;*/
	if(ReturnImg)
		g_pMManager->pem_delete(ReturnImg, true);
	ReturnImg = NULL;
	if(ptDstPoint)
		g_pMManager->pem_delete(ptDstPoint, true);
	ptDstPoint = NULL;
	if(ROIPoint)
		g_pMManager->pem_delete(ROIPoint, true);
	ROIPoint = NULL;
	if(ptfDstPoint)
		g_pMManager->pem_delete(ptfDstPoint, true);
	ptfDstPoint = NULL;

	memcpy(ucArrFillOutImg,src.data,nWidth*nHeight* sizeof(byte)*1);

	//MaskImg2.release();
	//GrayImg.release();
	src.release();

	//free(ZeroCnt);
	g_pMManager->pem_free(ZeroCnt);

	ZeroCnt = NULL;
}
void CPInsp::FillMaskingROIUsingOpenCVImg(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, int nUsedMaskingValue, const std::vector<RECT> rcArrMaskingROI, int nFillValue, UCHAR *ucArrOverlapImg, int nChanel, UCHAR *ucArrFillOutImg, UCHAR *ucMask)
{
	if (nUsedMaskingValue < 1 || rcArrMaskingROI.size() < 1 || !ucArrSrcImg)
		return;
	UCHAR* pUcArrTemp = NULL;

	BOOL bUseOverlapCnt = ucArrOverlapImg != NULL ? TRUE : FALSE;
	int cnt = 0;
	int nUsedPointCnt = nUsedMaskingValue * 4;

	/*POINT* ptDstPoint = new POINT[nUsedPointCnt];
	POINT* ROIPoint = new POINT[nUsedPointCnt];
	POINTF* ptfDstPoint = new POINTF[nUsedPointCnt];*/
	POINT* ptDstPoint = g_pMManager->pem_new<POINT>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);
	POINT* ROIPoint = g_pMManager->pem_new<POINT>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);
	POINTF* ptfDstPoint = g_pMManager->pem_new<POINTF>(true, nUsedPointCnt, (PCHAR)__FUNCTION__, __LINE__);

	if (dAngle == 90 || dAngle == 270)
	{
		dAngle -= 360;
		if (dAngle < 0)
			dAngle += 180;
	}

	for (int a = 0; a < nUsedMaskingValue; a++)
	{
		int nROIWidth = rcArrMaskingROI[a].right - rcArrMaskingROI[a].left;
		if (nROIWidth <= 0)
			continue;
		int nROIHeight = rcArrMaskingROI[a].bottom - rcArrMaskingROI[a].top;
		if (nROIHeight <= 0)
			continue;
		int nStartX = rcArrMaskingROI[a].left;
		int nStartY = rcArrMaskingROI[a].top;
		int index = a * 4;
		ROIPoint[index].x = nStartX;
		ROIPoint[index++].y = nStartY;
		ROIPoint[index].x = nStartX + nROIWidth;
		ROIPoint[index++].y = nStartY;
		ROIPoint[index].x = nStartX + nROIWidth;
		ROIPoint[index++].y = nStartY + nROIHeight;
		ROIPoint[index].x = nStartX;
		ROIPoint[index++].y = nStartY + nROIHeight;
	}


	for (int a = 0; a < nUsedPointCnt; a++)
	{
		ptfDstPoint[a].x = ROIPoint[a].x;
		ptfDstPoint[a].y = ROIPoint[a].y;
		if (dAngle != 0)
		{
			AnglePosChange(dAngle, nWidth, nHeight, ptfDstPoint[a], &ptfDstPoint[a]);
			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}
		else
		{
			ptDstPoint[a].x = ptfDstPoint[a].x + (nWidth / 2.);
			ptDstPoint[a].y = ptfDstPoint[a].y + (nHeight / 2.);
		}
		if (ptDstPoint[a].x < 0)
			ptDstPoint[a].x = 0;
		if (ptDstPoint[a].y < 0)
			ptDstPoint[a].y = 0;
		if (ptDstPoint[a].x > nWidth)
			ptDstPoint[a].x = nWidth;
		if (ptDstPoint[a].y > nHeight)
			ptDstPoint[a].y = nHeight;
	}

	//UCHAR* ReturnImg = new UCHAR[nWidth*nHeight*nChanel];
	UCHAR* ReturnImg = g_pMManager->pem_new<UCHAR>(true, nWidth*nHeight*nChanel, (PCHAR)__FUNCTION__, __LINE__);
	//int *ZeroCnt = (int*)malloc(sizeof(int));
	int *ZeroCnt = (int*)g_pMManager->pem_malloc(sizeof(int), (PCHAR)__FUNCTION__, __LINE__);
	*ZeroCnt = 1;

	int nColorTye = nChanel == 3 ? 0 : 1;
	cv::Mat src = m_pProcMilAlgo->BytesToMat(ucArrSrcImg, nWidth, nHeight, nColorTye);
	cv::Mat ucMask2 = m_pProcMilAlgo->BytesToMat(ucMask, nWidth, nHeight, nColorTye);

	src = FillPolygonImg(ucMask2, src, nWidth, nHeight, ptDstPoint, true, nUsedPointCnt, ZeroCnt, ucArrOverlapImg, nChanel);
	memcpy(ucMask, ucMask2.data, nWidth*nHeight * sizeof(byte)*nChanel);
	memcpy(ucArrSrcImg, m_pProcMilAlgo->MatToBytes(src, ReturnImg), nWidth*nHeight * sizeof(byte)*nChanel);

	cnt = *ZeroCnt;

	/*if(ReturnImg)
		delete[] ReturnImg;
	ReturnImg = NULL;
	if(ptDstPoint)
		delete [] ptDstPoint;
	ptDstPoint = NULL;
	if(ROIPoint)
		delete [] ROIPoint;
	ROIPoint = NULL;
	if(ptfDstPoint)
		delete [] ptfDstPoint;
	ptfDstPoint = NULL;*/
	if (ReturnImg)
		g_pMManager->pem_delete(ReturnImg, true);
	ReturnImg = NULL;
	if (ptDstPoint)
		g_pMManager->pem_delete(ptDstPoint, true);
	ptDstPoint = NULL;
	if (ROIPoint)
		g_pMManager->pem_delete(ROIPoint, true);
	ROIPoint = NULL;
	if (ptfDstPoint)
		g_pMManager->pem_delete(ptfDstPoint, true);
	ptfDstPoint = NULL;

	memcpy(ucArrFillOutImg, src.data, nWidth*nHeight * sizeof(byte) * 1);

	//MaskImg2.release();
	//GrayImg.release();
	src.release();

	//free(ZeroCnt);
	g_pMManager->pem_free(ZeroCnt);

	ZeroCnt = NULL;
}
int CPInsp::FillOutOfInspAreaImg(int nWidth, int nHeight, double dAngle, UCHAR *ucArrSrcImg, UCHAR *ucArrOverlapImg, int nUsedInspPolygon, const POINTF *ptArrInspPolygon, int nFillValue, BOOL bConvertExceptROI, int nChanel, UCHAR *ucArrFillOutImg, UCHAR *ucBinImg, UCHAR *ucMask, int nExcept)
{
	if (nUsedInspPolygon < 1 || ptArrInspPolygon == NULL || !ucArrSrcImg)
		return 0;
	BOOL bCheckIngFALSE = bConvertExceptROI ? TRUE : FALSE;
	/*POINT* ptDstPoint = new POINT[nUsedInspPolygon];
	POINTF* ptfDstPoint = new POINTF[nUsedInspPolygon];*/
	POINT* ptDstPoint = g_pMManager->pem_new<POINT>(true, nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);
	POINTF* ptfDstPoint = g_pMManager->pem_new<POINTF>(true, nUsedInspPolygon, (PCHAR)__FUNCTION__, __LINE__);
	cv::Mat ucMask2;
	if (dAngle == 90 || dAngle == 270)
	{
		dAngle -= 360;
		if (dAngle < 0)
			dAngle += 180;
	}
	if (dAngle != 0)
	{
		for (int a = 0; a < nUsedInspPolygon; a++)
		{
			AnglePosChange(dAngle, nWidth, nHeight, ptArrInspPolygon[a], &ptfDstPoint[a]);
			if (ptfDstPoint[a].x < 0)
				ptfDstPoint[a].x = 0;
			if (ptfDstPoint[a].y < 0)
				ptfDstPoint[a].y = 0;
			if (ptfDstPoint[a].x > nWidth)
				ptfDstPoint[a].x = nWidth;
			if (ptfDstPoint[a].y > nHeight)
				ptfDstPoint[a].y = nHeight;
			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}
	}
	else
	{
		for (int a = 0; a < nUsedInspPolygon; a++)
		{
			ptfDstPoint[a].x = ptArrInspPolygon[a].x + (nWidth / 2.);
			ptfDstPoint[a].y = ptArrInspPolygon[a].y + (nHeight / 2.);
			if (ptfDstPoint[a].x < 0)
				ptfDstPoint[a].x = 0;
			if (ptfDstPoint[a].y < 0)
				ptfDstPoint[a].y = 0;
			if (ptfDstPoint[a].x > nWidth)
				ptfDstPoint[a].x = nWidth;
			if (ptfDstPoint[a].y > nHeight)
				ptfDstPoint[a].y = nHeight;

			ptDstPoint[a].x = ptfDstPoint[a].x;
			ptDstPoint[a].y = ptfDstPoint[a].y;
		}
	}
	int cnt = 0;
	//UCHAR* ReturnImg = new UCHAR[nWidth*nHeight*nChanel];
	UCHAR* ReturnImg = g_pMManager->pem_new<UCHAR>(true, nWidth*nHeight*nChanel, (PCHAR)__FUNCTION__, __LINE__);
	//int *ZeroCnt = (int*)malloc(sizeof(int));
	int *ZeroCnt = (int*)g_pMManager->pem_malloc(sizeof(int), (PCHAR)__FUNCTION__, __LINE__);
	*ZeroCnt = 0;
	int nColorType = nChanel == 3 ? 0 : 1;
	cv::Mat src = m_pProcMilAlgo->BytesToMat(ucArrSrcImg, nWidth, nHeight, nColorType);
	ucMask2 = m_pProcMilAlgo->BytesToMat(ucMask, nWidth, nHeight, nColorType);
	src = FillPolygonImg(ucMask2, src, nWidth, nHeight, ptDstPoint, bConvertExceptROI, nUsedInspPolygon, ZeroCnt, ucArrOverlapImg, nChanel, nExcept);
	memcpy(ucMask, ucMask2.data, nWidth*nHeight * sizeof(byte)*nChanel);
	memcpy(ReturnImg, m_pProcMilAlgo->MatToBytes(src, ReturnImg), nWidth*nHeight * sizeof(byte)*nChanel);
	memcpy(ucArrSrcImg, ReturnImg, nWidth*nHeight * sizeof(byte)*nChanel);
	/*if (ptDstPoint)
		delete[] ptDstPoint;
	ptDstPoint = NULL;
	if (ptfDstPoint)
		delete[] ptfDstPoint;
	ptfDstPoint = NULL;
	if (ReturnImg)
		delete[] ReturnImg;
	ReturnImg = NULL;*/
	if (ReturnImg)
		g_pMManager->pem_delete(ReturnImg, true);
	ReturnImg = NULL;
	if (ptDstPoint)
		g_pMManager->pem_delete(ptDstPoint, true);
	ptDstPoint = NULL;
	if (ptfDstPoint)
		g_pMManager->pem_delete(ptfDstPoint, true);
	ptfDstPoint = NULL;
	
	memcpy(ucArrFillOutImg, src.data, nWidth*nHeight * sizeof(byte) * 1);
	if(ucBinImg != NULL)
		memcpy(ucBinImg, src.data, nWidth*nHeight * sizeof(byte) * 1);

	//MaskImg2.release();
	//GrayImg.release();
	src.release();
	//free(ZeroCnt);
	g_pMManager->pem_free(ZeroCnt);
	return cnt;
}
bool CPInsp::BlobCircleCenter(cv::Mat src, double& dCirle_x, double& dCirle_y, double* dRadius, double* dErrorRate, bool bDetail)
{
	cv::Mat testImage = src;
	cv::Mat Ecircle = testImage.clone();

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	if (bDetail)
		cv::findContours(Ecircle, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	else
		cv::findContours(Ecircle, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	if (contours.size() > 0)
	{
		std::vector<cv::Point> selectedCon;
		for (int cCnt = 0; cCnt < contours.size(); cCnt++)
		{
			if (contours[cCnt].size() < 6)
				continue;

			if (contours[cCnt].size() < selectedCon.size())
				continue;

			selectedCon = contours[cCnt];
		}

		if (selectedCon.size() > 6)
		{
			cv::Point2f ct;
			double radi;
			double dErr;

			GetFiduPos_LSQ(selectedCon, &dCirle_x, &dCirle_y, &radi, &dErr);

			if (dRadius)
				*dRadius = radi;

			if (dErrorRate)
				*dErrorRate = dErr;
		}

		else
			return false;

		dCirle_x += 0.5f;
		dCirle_y += 0.5f;

		return true;
	}
	return false;

}

void CPInsp::GetFiduPos_LSQ(std::vector<cv::Point>&  Points, double *centX, double *centY, double *radius, double *err)
{
	int nBorderCnt = Points.size();

	double theta;
	double s_cos = 0, s_sin = 0;
	double xy = 0, x = 0, y = 0;
	int cnt = nBorderCnt;

	double err2 = 0;

	double ds_Xn2 = 0, ds_Yn2 = 0, ds_XnYn = 0, ds_Xn = 0, ds_Yn = 0;
	double dsub1 = 0, dsub2 = 0, dsub3 = 0;

	//?? ???
	for (int i = 0; i < nBorderCnt; i++)
	{
		double x, y;
		x = Points[i].x;
		y = Points[i].y;

		ds_Xn2 += x * x;
		ds_Yn2 += y * y;
		ds_XnYn += x * y;
		ds_Xn += x;
		ds_Yn += y;

		dsub1 += x * (x*x + y * y);
		dsub2 += y * (x*x + y * y);
		dsub3 += x * x + y * y;
	}

	int matrixSize = 3;
	int matrixSize2 = matrixSize * matrixSize;

	/*double* org_m = (double*)malloc(sizeof(double)*matrixSize2);
	double* inv_m = (double*)malloc(sizeof(double)*matrixSize2);
	double* sub_m = (double*)malloc(sizeof(double)*matrixSize);
	double* pBuff = (double*)malloc(sizeof(double)*matrixSize2);*/
	double* org_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);
	double* inv_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);
	double* sub_m = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize, (PCHAR)__FUNCTION__, __LINE__);
	double* pBuff = (double*)g_pMManager->pem_malloc(sizeof(double)*matrixSize2, (PCHAR)__FUNCTION__, __LINE__);

	org_m[0] = ds_Xn2; org_m[1] = ds_XnYn; org_m[2] = ds_Xn;
	org_m[3] = ds_XnYn; org_m[4] = ds_Yn2; org_m[5] = ds_Yn;
	org_m[6] = ds_Xn; org_m[7] = ds_Yn; org_m[8] = cnt;

	sub_m[0] = dsub1; sub_m[1] = dsub2; sub_m[2] = dsub3;

	//IppStatus ans = ippmInvert_ma_64f(org_m, (int)36 * 2, (int)12 * 2, (int)4 * 2, pBuff, inv_m,
	//	(int)36 * 2, (int)12 * 2, (int)4 * 2, (int)3, (int)1);
	//ans = ippmMul_mav_64f(inv_m, 36 * 2, 12 * 2, 4 * 2, 3, 3, sub_m, 4 * 2, 3, org_m, 12 * 2, 4 * 2, 1);

	//*centX = (float)org_m[0] / 2;
	//*centY = (float)org_m[1] / 2;
	//*radius = (float)((*centX**centX) + (*centY**centY) + org_m[2]);
	//*radius = sqrt(*radius);

	cv::Mat org(3, 3, CV_64FC1, org_m);
	cv::Mat sub(3, 1, CV_64FC1, sub_m);
	cv::Mat dst = org.inv() * sub;

	*centX = (float)dst.at<double>(0, 0) / 2;
	*centY = (float)dst.at<double>(1, 0) / 2;
	*radius = (float)((*centX**centX) + (*centY**centY) + dst.at<double>(2, 0));
	*radius = sqrt(*radius);

	g_pMManager->pem_free(org_m);
	g_pMManager->pem_free(inv_m);
	g_pMManager->pem_free(sub_m);
	g_pMManager->pem_free(pBuff);
	//delete[] org_m;
	//delete[] inv_m;
	//delete[] sub_m;
	//delete[] pBuff;

	if (*radius < 0) *radius = -*radius;


	int m_borderCnt = 0;
	for (int i = 0; i < nBorderCnt; i++)
	{
		m_borderCnt++;
		cv::Point pt = Points[i];

		double dist = sqrt(SQR(pt.x - *centX) + SQR(pt.y - *centY));
		double distdeviate = fabs(dist - *radius) / *radius;

		if (distdeviate > 0.07)
			err2++;
	}
	*err = err2 / nBorderCnt;

	return;
}
cv::Mat CPInsp::FillPolygonImg(cv::Mat ucMask,cv::Mat binImg, int width, int height, POINT* ptDstPoint, bool isExceptROI, int nUsedInspPolygon, int* cnt, UCHAR* ucArrOverlapImg, int nChanel, int nExcept)
{
	cv::Scalar black(0, 0, 0);
	cv::Scalar white(255, 255, 255);

	cv::Mat img;
	cv::Mat mask;
	cv::Mat OverlapImg;
	vector<cv::Point> contour;

	CString str;

	for (int i = 0; i < nUsedInspPolygon; i++)
	{
		if ((ptDstPoint[i].x >= 0 && ptDstPoint[i].y >= 0) && (ptDstPoint[i].x <= width && ptDstPoint[i].y <= height))
			contour.push_back(cv::Point(ptDstPoint[i].x, ptDstPoint[i].y));
	}

	const cv::Point *pts3 = (cv::Point*) cv::Mat(contour).data;
	int npts3 = cv::Mat(contour).rows;
	int ZeroCnt = 0;
	bool isExceptROIRect = false;
	if (*cnt == 1)
	{
		isExceptROIRect = true;
		npts3 = 4;
	}
	if (ucArrOverlapImg != NULL)
	{
		int nColorTye = nChanel == 3 ? 0 : 1;
		OverlapImg = m_pProcMilAlgo->BytesToMat(ucArrOverlapImg, width, height, nColorTye);
	}

	if (isExceptROI)
	{
		if (!isExceptROIRect)
		{
			img = cv::Mat(height, width, CV_8UC(nChanel), white);
			cv::fillPoly(img, &pts3, &npts3, 1, black);
			if (ucArrOverlapImg != NULL)
				cv::fillPoly(OverlapImg, &pts3, &npts3, 1, black);
		}
		else
		{
			img = cv::Mat(height, width, CV_8UC(nChanel), white);
			for (int i = 0; i < nUsedInspPolygon; i += 4)
			{
				cv::rectangle(img, pts3[i], pts3[i + 2], black, -1);
				if (ucArrOverlapImg != NULL)
					cv::rectangle(OverlapImg, pts3[i], pts3[i + 2], black, -1);
			}
		}
		// 		str.Format(_T("isExceptROI"));
		// 		g_pMPTI->AddLog_Dev(str);
	}
	else
	{
		img = cv::Mat(height, width, CV_8UC(nChanel), black);
		fillPoly(img, &pts3, &npts3, 1, white);
		if (ucArrOverlapImg != NULL)
			fillPoly(OverlapImg, &pts3, &npts3, 1, white);
		// 		str.Format(_T("polygon region"));
		// 		g_pMPTI->AddLog_Dev(str);
	}
	//ucMask = img.clone();
	if (ucMask.size == img.size)
	{
		mask = cv::Mat(height, width, CV_8UC(nChanel), black);
		mask = img.clone();
		if(nExcept == 1)
			cv::bitwise_and(img, ucMask, mask);
		// 		str.Format(_T("m_nUsedWndPolygon : %d, m_ptArrWndPolygon : %lf,%lf"), nUsedInspPolygon, ptDstPoint[0].x, ptDstPoint[0].y);
		// 		g_pMPTI->AddLog_Dev(str);
	}
	memcpy(ucMask.data, mask.data, width*height * sizeof(byte)*nChanel);

	if (binImg.size == img.size)
	{
		cv::bitwise_and(img, binImg, img);
		// 		str.Format(_T("m_nUsedWndPolygon : %d, m_ptArrWndPolygon : %lf,%lf"), nUsedInspPolygon, ptDstPoint[0].x, ptDstPoint[0].y);
		// 		g_pMPTI->AddLog_Dev(str);
	}
	if (ucArrOverlapImg != NULL)
		memcpy(ucArrOverlapImg, m_pProcMilAlgo->MatToBytes(OverlapImg, ucArrOverlapImg), width*height * sizeof(byte)*nChanel);
	*cnt = ZeroCnt;
	binImg = img;
	pts3 = nullptr;

	img.release();
	OverlapImg.release();

	return binImg;
}

BOOL CPInsp::InspectionLine(int nWndWidth, int nWndHeight, UCHAR *ucArrDstWnd, bool bIsHorizon, int nMeasureDirection, double* dAngle, POINTF* poDrawLine, int* nLineTotalLength, double* dAValue, double* dBValue, int nLineFindType, double dLineFindRate, int nInspOption)
{
	BOOL bReturn = FALSE;

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat source(nWndHeight, nWndWidth, CV_8UC1, ucArrDstWnd);
	cv::Mat imageLine = source.clone();

	cv::findContours(imageLine, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	*dAValue = 0; *dBValue = 0;
	*dAngle = 0;
	if (contours.size() > 0)
	{
		int nMaxBlobidx = 0;
		int nMaxAreaSize = 0;
		for (int i = 0; i < contours.size(); i++)
		{
			int area = contours[i].size();
			if (area > nMaxAreaSize)
			{
				nMaxAreaSize = area;
				nMaxBlobidx = i;
			}
		}

		std::vector<std::vector<cv::Point>> conPoly(contours.size());
		float peri = cv::arcLength(contours[nMaxBlobidx], true);
		cv::approxPolyDP(contours[nMaxBlobidx], conPoly[nMaxBlobidx], 0.02 * peri, true);

		double dAreaRatio = 0.0;
		if (conPoly[nMaxBlobidx].size() == 8)
		{
			cv::Point2f center;
			float fR;

			cv::minEnclosingCircle(conPoly[nMaxBlobidx], center, fR);
			double dContourArea = cv::contourArea(contours[nMaxBlobidx]);
			double dCircleArea = PI * fR * fR;
			dAreaRatio = dContourArea / dCircleArea;
		}

		cv::Point2f points[4];
		if (dAreaRatio > 0.8)	// Circle
		{
			cv::Rect boundRect = cv::boundingRect(contours[nMaxBlobidx]);
			points[0] = cv::Point2f(boundRect.tl().x, boundRect.br().y);
			points[1] = boundRect.tl();
			points[2] = cv::Point2f(boundRect.br().x, boundRect.tl().y);
			points[3] = boundRect.br();

			if (nMeasureDirection == Center_Line)
			{
				poDrawLine[0].x = (bIsHorizon == TRUE) ? (points[0].x + points[1].x) / 2 : (points[2].x + points[1].x) / 2;
				poDrawLine[0].y = (bIsHorizon == TRUE) ? (points[0].y + points[1].y) / 2 : (points[2].y + points[1].y) / 2;
				poDrawLine[1].x = (bIsHorizon == TRUE) ? (points[3].x + points[2].x) / 2 : (points[3].x + points[0].x) / 2;
				poDrawLine[1].y = (bIsHorizon == TRUE) ? (points[3].y + points[2].y) / 2 : (points[3].y + points[0].y) / 2;
			}
			else if (nMeasureDirection == Left_Line)
			{
				poDrawLine[0].x = points[1].x;
				poDrawLine[0].y = points[1].y;
				poDrawLine[1].x = (bIsHorizon == TRUE) ? points[2].x : points[0].x;
				poDrawLine[1].y = (bIsHorizon == TRUE) ? points[2].y : points[0].y;
			}
			else if (nMeasureDirection == Right_Line)
			{
				poDrawLine[0].x = (bIsHorizon == TRUE) ? points[0].x : points[2].x;
				poDrawLine[0].y = (bIsHorizon == TRUE) ? points[0].y : points[2].y;
				poDrawLine[1].x = points[3].x;
				poDrawLine[1].y = points[3].y;
			}

			*dAngle = 0;
			*dAValue = tan(*dAngle * PI / 180);
			double dB_H = ((poDrawLine[0].y - (*dAValue * poDrawLine[0].x)) + (poDrawLine[1].y - (*dAValue * poDrawLine[1].x))) / 2;
			double dB_V = ((poDrawLine[0].x - (*dAValue * poDrawLine[0].y)) + (poDrawLine[1].x - (*dAValue * poDrawLine[1].y))) / 2;
			*dBValue = (bIsHorizon == TRUE) ? dB_H : dB_V;
			bReturn = TRUE;
		}
		else
		{
			bool bExceptAngle = false;
			if ((nInspOption & m_eEdgeData_UseExceptAngle) == m_eEdgeData_UseExceptAngle)
				bExceptAngle = true;

			cv::RotatedRect rotRect = cv::minAreaRect(contours[nMaxBlobidx]);
			float angle = rotRect.angle;
			if (rotRect.angle <= -45)
				angle = (90 + rotRect.angle);

			if (nMeasureDirection == -1)
			{
				*dAngle = angle * -1;
				return TRUE;
			}

			cv::Mat M = cv::getRotationMatrix2D(rotRect.center, angle, 1);
			cv::Mat imageLine2 = source.clone();
			cv::Mat imageRotated;
			cv::warpAffine(imageLine2, imageRotated, M, cv::Size(nWndWidth, nWndHeight));

			UCHAR* pucInspectedBin = NULL;
			Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pucInspectedBin, nWndWidth * nWndHeight);
			memset(pucInspectedBin, 0, sizeof(UCHAR) * nWndWidth * nWndHeight);

			if (bExceptAngle)
			{
				memcpy(pucInspectedBin, imageLine2.data, sizeof(UCHAR) * nWndWidth * nWndHeight);
			}
			else
			{
				memcpy(pucInspectedBin, imageRotated.data, sizeof(UCHAR) * nWndWidth * nWndHeight);
				if (pucInspectedBin)
				{
					int nWndIDX = 0;
					for (int y = 0; y < nWndHeight; y++)
					{
						for (int x = 0; x < nWndWidth; x++)
						{
							if (pucInspectedBin[nWndIDX] > 0)
								pucInspectedBin[nWndIDX] = 255;
							nWndIDX++;
						}
					}
				}
			}

			bReturn = InspectionLine2(nWndWidth, nWndHeight, pucInspectedBin, bIsHorizon, nMeasureDirection, dAngle, poDrawLine, nLineTotalLength, dAValue, dBValue, nLineFindType, dLineFindRate);

			if (bExceptAngle == false)
			{
				angle *= PI / 180;

				for (int i = 0; i < 2; i++)
				{
					float x2 = cos(angle)*(poDrawLine[i].x - rotRect.center.x) - sin(angle)*(poDrawLine[i].y - rotRect.center.y) + rotRect.center.x;
					float y2 = sin(angle)*(poDrawLine[i].x - rotRect.center.x) + cos(angle)*(poDrawLine[i].y - rotRect.center.y) + rotRect.center.y;
					poDrawLine[i].x = x2;
					poDrawLine[i].y = y2;
				}
			}

			double dArrX[2] = { poDrawLine[0].x , poDrawLine[1].x };
			double dArrY[2] = { poDrawLine[0].y , poDrawLine[1].y };
			*dAngle = GetGradient(dArrX, dArrY, 2, *dAValue, *dBValue, bIsHorizon);
			if (bIsHorizon) *dAngle *= -1.0;

			if (pucInspectedBin)
				Delete_1DArray(&pucInspectedBin);
		}
	}
	return bReturn;
}

BOOL CPInsp::InspectionLine2(int nWndWidth, int nWndHeight, UCHAR *ucArrDstWnd, bool bIsHorizon, int nMeasureDirection, double* dAngle, POINTF* poDrawLine, int* nLineTotalLength, double* dAValue, double* dBValue, int nLineFindType, double dLineFindRate)
{
	BOOL bReturn = FALSE;

	double *ptrdLineX = NULL;
	double *ptrdLineY = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrdLineX, nWndWidth * nWndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrdLineY, nWndWidth * nWndHeight);
	memset(ptrdLineX, 0, sizeof(double) * nWndWidth * nWndHeight);
	memset(ptrdLineY, 0, sizeof(double) * nWndWidth * nWndHeight);
	//Center Buf
	int *ptrnLeftX = NULL;
	int *ptrnLeftY = NULL;
	int *ptrnRightX = NULL;
	int *ptrnRightY = NULL;
	int nLeftLength = 0;
	int nRightLength = 0;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrnLeftX, nWndWidth * nWndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrnLeftY, nWndWidth * nWndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrnRightX, nWndWidth * nWndHeight);
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrnRightY, nWndWidth * nWndHeight);
	memset(ptrnLeftX, 0, sizeof(int) * nWndWidth * nWndHeight);
	memset(ptrnLeftY, 0, sizeof(int) * nWndWidth * nWndHeight);
	memset(ptrnRightX, 0, sizeof(int) * nWndWidth * nWndHeight);
	memset(ptrnRightY, 0, sizeof(int) * nWndWidth * nWndHeight);

	int nEndPos_Search = 0;
	int nEndPos_Line = 0;
	if (bIsHorizon)	// -
	{
		nEndPos_Search = nWndHeight;
		nEndPos_Line = nWndWidth;
	}
	else	// |
	{
		nEndPos_Search = nWndWidth;
		nEndPos_Line = nWndHeight;
	}
	for (int nStartPos_Line = 0; nStartPos_Line < nEndPos_Line; nStartPos_Line++)
	{
		if (nMeasureDirection == Center_Line || nMeasureDirection == Left_Line)	// 0: Center 1: Left
		{
			// Center _ Left
			for (int nLeftPos_Search = 0; nLeftPos_Search < nEndPos_Search; nLeftPos_Search++)
			{
				if (bIsHorizon)
				{
					if (ucArrDstWnd[nStartPos_Line + (nLeftPos_Search * nWndWidth)] == 255)
					{
						ptrnLeftX[nLeftLength] = nStartPos_Line;
						ptrnLeftY[nLeftLength] = nLeftPos_Search;
						nLeftLength++;
						break;
					}
				}
				else
				{
					if (ucArrDstWnd[nLeftPos_Search + (nStartPos_Line * nWndWidth)] == 255)
					{
						ptrnLeftX[nLeftLength] = nLeftPos_Search;
						ptrnLeftY[nLeftLength] = nStartPos_Line;
						nLeftLength++;
						break;
					}
				}
			}
		}
		if (nMeasureDirection == Center_Line || nMeasureDirection == Right_Line)	// 0: Center 2: Right
		{
			for (int nRightPos_Search = nEndPos_Search - 1; nRightPos_Search > 0; nRightPos_Search--)
			{
				if (bIsHorizon)
				{
					if (ucArrDstWnd[nStartPos_Line + (nRightPos_Search * nWndWidth)] == 255)
					{
						ptrnRightX[nRightLength] = nStartPos_Line;
						ptrnRightY[nRightLength] = nRightPos_Search;
						nRightLength++;
						break;
					}
				}
				else
				{
					if (ucArrDstWnd[nRightPos_Search + (nStartPos_Line * nWndWidth)] == 255)
					{
						ptrnRightX[nRightLength] = nRightPos_Search;
						ptrnRightY[nRightLength] = nStartPos_Line;
						nRightLength++;
						break;
					}
				}
			}
		}
	}
	double dCenterX = 0;
	double dCenterY = 0;
	int nLineLength = 0;
	int nCnt = 0;
	double dSearchLimite_L = 0.0;
	double dSearchLimite_R = 0.0;
	for (int n = 0; n < 60; n++)
	{
		if (nLeftLength <= 0 && nRightLength <= 0)
			break;
		double dRateTemp = dLineFindRate + (n * 1);
		double dSearchPer = dRateTemp / 100.0;
		if (dSearchPer < 0) dSearchPer = 1.0;
		if (nLeftLength > 0)
		{
			double dValueX = (nLineFindType == 1) ? nWndWidth : 0.0;
			double dValueY = (nLineFindType == 1) ? nWndHeight : 0.0;
			for (int a = 0; a < nLeftLength; a++)
			{
				if (nLineFindType == 1)
				{
					if (ptrnLeftX[a] < dValueX)
						dValueX = ptrnLeftX[a];
					if (ptrnLeftY[a] < dValueY)
						dValueY = ptrnLeftY[a];
				}
				else
				{
					dValueX += ptrnLeftX[a];
					dValueY += ptrnLeftY[a];
				}
			}
			if (nLineFindType == 0)
			{
				dValueX = dValueX / nLeftLength;
				dValueY = dValueY / nLeftLength;
			}
			dSearchLimite_L = (bIsHorizon == true) ? dValueY * dSearchPer : dValueX * dSearchPer;
		}
		if (nRightLength > 0)
		{
			double dValueX = 0.0;
			double dValueY = 0.0;
			for (int a = 0; a < nRightLength; a++)
			{
				if (nLineFindType == 1)
				{
					if (ptrnRightX[a] > dValueX)
						dValueX = ptrnRightX[a];
					if (ptrnRightY[a] > dValueY)
						dValueY = ptrnRightY[a];
				}
				else
				{
					dValueX += ptrnRightX[a];
					dValueY += ptrnRightY[a];
				}
			}
			if (nLineFindType == 0)
			{
				dValueX = dValueX / nRightLength;
				dValueY = dValueY / nRightLength;
			}
			dSearchLimite_R = (bIsHorizon == true) ? dValueY / dSearchPer : dValueX / dSearchPer;
		}
		if (nMeasureDirection == Center_Line)
		{
			if (nLeftLength > nRightLength)
				nLineLength = nRightLength;
			else
				nLineLength = nLeftLength;
			for (int a = 0; a < nLineLength; a++)
			{
				int nLeftX = ptrnLeftX[a];
				int nLeftY = ptrnLeftY[a];
				int nRigthX = ptrnRightX[a];
				int nRigthY = ptrnRightY[a];
				bool bInclude = false;
				if (bIsHorizon == true)
				{
					if (nLeftY <= dSearchLimite_L || nRigthY >= dSearchLimite_R)
						bInclude = true;
				}
				else
				{
					if (nLeftX <= dSearchLimite_L || nRigthX >= dSearchLimite_R)
						bInclude = true;
				}
				if (bInclude == true)
				{
					ptrdLineX[nCnt] = (ptrnLeftX[a] + ptrnRightX[a]) / 2.0;
					ptrdLineY[nCnt] = (ptrnLeftY[a] + ptrnRightY[a]) / 2.0;
					nCnt++;
				}
			}
		}
		else if (nMeasureDirection == Left_Line)
		{
			nLineLength = nLeftLength;
			for (int a = 0; a < nLineLength; a++)
			{
				int nLeftX = ptrnLeftX[a];
				int nLeftY = ptrnLeftY[a];
				bool bInclude = false;
				if (bIsHorizon == true)
				{
					if (nLeftY <= dSearchLimite_L)
						bInclude = true;
				}
				else
				{
					if (nLeftX <= dSearchLimite_L)
						bInclude = true;
				}
				if (bInclude == true)
				{
					ptrdLineX[nCnt] = ptrnLeftX[a];
					ptrdLineY[nCnt] = ptrnLeftY[a];
					nCnt++;
				}
			}
		}
		else if (nMeasureDirection == Right_Line)
		{
			nLineLength = nRightLength;
			for (int a = 0; a < nLineLength; a++)
			{
				int nRigthX = ptrnRightX[a];
				int nRigthY = ptrnRightY[a];
				bool bInclude = false;
				if (bIsHorizon == true)
				{
					if (nRigthY >= dSearchLimite_R)
						bInclude = true;
				}
				else
				{
					if (nRigthX >= dSearchLimite_R)
						bInclude = true;
				}
				if (bInclude == true)
				{
					ptrdLineX[nCnt] = nRigthX;
					ptrdLineY[nCnt] = nRigthY;
					nCnt++;
				}
			}
		}
		int nBreak = (bIsHorizon == true) ? nWndWidth : nWndHeight;
		nBreak *= 0.05;
		if (nCnt >= 30 || nCnt >= nBreak)
			break;
		else
		{
			nCnt = 0;
			for (int i = 0; i < nCnt; i++)
				ptrdLineX[i] = ptrdLineY[i] = 0;
		}
	}
	*nLineTotalLength = nLineLength = nCnt;
	*dAValue = 0; *dBValue = 0;
	*dAngle = 0;
	if (nCnt > 0)
	{
		*dAngle = GetGradient(ptrdLineX, ptrdLineY, (double)nLineLength, *dAValue, *dBValue, bIsHorizon);
		bReturn = TRUE;
		poDrawLine[0].x = (bIsHorizon == TRUE) ? ptrdLineX[0] : (*dAValue * ptrdLineY[0]) + *dBValue;
		poDrawLine[0].y = (bIsHorizon == TRUE) ? (*dAValue * ptrdLineX[0]) + *dBValue : ptrdLineY[0];
		poDrawLine[1].x = (bIsHorizon == TRUE) ? ptrdLineX[nLineLength - 1] : (*dAValue * ptrdLineY[nLineLength - 1]) + *dBValue;
		poDrawLine[1].y = (bIsHorizon == TRUE) ? (*dAValue * ptrdLineX[nLineLength - 1]) + *dBValue : ptrdLineY[nLineLength - 1];

		if (nCnt == 1)
		{
			if ((poDrawLine[0].x == poDrawLine[1].x) && (poDrawLine[0].y, poDrawLine[1].y))
			{
				poDrawLine[1].x = poDrawLine[0].x + 1;
				poDrawLine[1].y = poDrawLine[1].y + 1;
			}
		}
	}
	else
	{
		for (int a = 0; a < 2; a++)
		{
			poDrawLine[a].x = 0;
			poDrawLine[a].y = 0;
		}
	}
	Delete_1DArray(&ptrdLineX);
	Delete_1DArray(&ptrdLineY);
	Delete_1DArray(&ptrnLeftX);
	Delete_1DArray(&ptrnLeftY);
	Delete_1DArray(&ptrnRightX);
	Delete_1DArray(&ptrnRightY);
	return bReturn;
}

// Circle - Center Detection 옵션 함수
bool CPInsp::BlobCircleCenter_Fit(cv::Mat src, CRect *rcBlob, double& dCircle_x, double& dCircle_y, double* dRadius, double* dErrorRate, double *dArea, int nMinBlobArea, BOOL eraseBorderBlob, BOOL fillHole, int nFilter, int nSelectType,
	int nTeachX/*= -1*/, int nTeachY/*= -1*/, int nBlobType, double dAreaPix, double dAreaWPix, double dAreaHPix, int nAreaCnt)
{
	int nCntBlob = 0;
	try
	{
		std::vector<cv::Point> selectedInlier, contour;
		std::vector<std::vector<cv::Point>> allcontour;

		cv::Mat morp1 = src.clone(), morp2 = src.clone(), mask1, mask2;

		// 		int nCnt = m_pProcMilAlgo->CalcBlob_Select(morp1.data, morp1.data, morp1.cols, morp1.rows, 6, FALSE, FALSE, 0, eSelectBigger);
		// 		if (nCnt == 0)
		// 		{
		// 			dCircle_x = dCircle_y = 0.;
		// 			if (dRadius)
		// 				*dRadius = 0.;
		// 			if (dErrorRate)
		// 				*dErrorRate = 1.;
		// 			return false;
		// 		}

		std::vector<cv::Vec4i> hierarchy;

		cv::findContours(morp1, allcontour, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

		for (int cCnt = 0; cCnt < allcontour.size(); cCnt++)
		{
			if (allcontour[cCnt].size() < 10)
				continue;

			if (allcontour[cCnt].size() < contour.size())
				continue;

			contour = allcontour[cCnt];
		}

		// contour 수가 적으면 검출 불가하다고 판단.
		if (contour.size() < 30)
		{
			dCircle_x = dCircle_y = 0.;
			if (dRadius)
				*dRadius = 0.;
			if (dErrorRate)
				*dErrorRate = 1.;
			return false;
		}

		mask1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
		mask2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));

		//==========================모폴로지
		// 원이 포함된 Blob이 충분히 작아질 때까지 모폴로지-침식 반복실행해서
		// 작아진 Blob의 중심을 원의 중심이라 가정한다.

		cv::Point anchor(-1, -1);
		uchar *tData = NULL;

		// 모폴로지 연산 중에 Blob이 사라져버리는 경우를 대비해서, 이전의 Blob 이미지를 가지고 있어야 함.
		// before = 이전(모포롤지 이전) blob 이미지 주소, after = 현재(모폴로지 이후) blob 이미지 주소
		cv::Mat *before = &morp1, *after = &morp2;

		// 작은 노이즈에 대비하여 모폴로지-팽창 1회 사전에 실행
		cv::dilate(*before, *before, mask2, anchor, 1, 0, cv::Scalar::all(0));
		while (true)
		{
			cv::erode(*before, *after, mask1, anchor, 1, 0, cv::Scalar::all(0));
			tData = (*after).data;

			nCntBlob = m_pProcMilAlgo->CalcBlob_Select(tData, NULL, morp1.cols, morp1.rows, nMinBlobArea, eraseBorderBlob, fillHole, nFilter, eSelectBigger, nTeachX, nTeachY, nBlobType, dAreaPix, dAreaWPix, dAreaHPix, nAreaCnt);

			double area, dcx, dcy;
			CRect rect(0, 0, 0, 0);
			if (nCntBlob > 0)
				m_pProcMilAlgo->GetBlobResult_Renewal(&area, &dCircle_x, &dCircle_y, &rect);

			if (rect.Width() < 15 || rect.Height() < 15)
				break;

			// blobdl 아직 남아있고 모폴로지를 다시 실행하는 경우,
			// before와 after를 바꿔준다. (지금의 after가 다음 루프의 before)
			cv::Mat *tmpPMat = before;
			before = after;
			after = tmpPMat;
		}

		// 모폴로지 연산 도중에 blob이 사라진 경우를 대비하기 위해 before 이미지로 다시 blob 계산을 해본다.
		if (nCntBlob == 0)
		{
			nCntBlob = m_pProcMilAlgo->CalcBlob_Select((*before).data, NULL, morp1.cols, morp1.rows, nMinBlobArea, eraseBorderBlob, fillHole, nFilter, eSelectBigger, nTeachX, nTeachY, nBlobType, dAreaPix, dAreaWPix, dAreaHPix, nAreaCnt);

			double area, dcx, dcy;
			CRect rect;
			if (nCntBlob > 0)
				m_pProcMilAlgo->GetBlobResult_Renewal(&area, &dCircle_x, &dCircle_y, &rect);
			else
			{
				dCircle_x = dCircle_y = 0.;
				if (dRadius)
					*dRadius = 0.;
				if (dErrorRate)
					*dErrorRate = 1.;
				return false;
			}
		}
		//==========================모폴로지

		//==========================히스토그램
		// 후보 중심으로부터 모든 contour 점들까지의 거리를 히스토그램으로 생성하고
		// 거리가 비슷한 점의 개수가 가장 많은 거리를 반지름이라 가정한다.
		int minDist(src.rows + src.cols), maxDist(0);
		cv::Mat hist(1, minDist, CV_32SC1);
		hist.setTo(0);
		uchar *data1 = morp1.data, *data2 = morp2.data;
		double radi;

		int* pHist = (int*)hist.data;

		// 히스토그램 생성
		for (int i = 0; i < contour.size(); i++)
		{
			int dist = sqrt(SQR(contour[i].x - dCircle_x) + SQR(contour[i].y - dCircle_y));
			pHist[dist]++;
			if (dist < minDist) minDist = dist;
			if (dist > maxDist) maxDist = dist;
		}

		// 중심-외각점 간 거리 중 최빈값 탐색
		int localSum(0), maxSum(0), maxIdx(0);
		minDist = minDist > 2 ? minDist : 2;
		for (int i = minDist; i <= maxDist; i++)
		{
			localSum += pHist[i] - pHist[i - 2];
			if (maxSum < localSum)
			{
				maxSum = localSum;
				maxIdx = pHist[i] > pHist[i - 1] ? (pHist[i] > pHist[i - 2] ? i : i - 2) : (pHist[i - 1] > pHist[i - 2] ? i - 1 : i - 2);
			}
		}
		//==========================히스토그램

		//==========================1차 원 검출
		// 위에서 얻은 후보 중심과 반지름을 활용해서 contour 중 inlier 후보들을 수집하고
		// 해당 inlier들로 LSQ를 실행해 1차로 원을 검출한다.
		radi = maxIdx;
		maxDist = maxIdx = 0;
		int beforeX = 0, beforeY = 0, gap = 0;

		double dErr;
		for (int i = 0; i < contour.size(); i++)
		{
			if (contour[i].x == 0 || contour[i].x == src.cols - 1 || contour[i].y == 0 || contour[i].y == src.rows - 1)
				continue;

			double dist = sqrt(SQR(contour[i].x - dCircle_x) + SQR(contour[i].y - dCircle_y));
			double distGap = fabs(dist - radi);
			if (distGap <= 3)
			{
				selectedInlier.emplace_back(contour[i]);

				if (beforeX == 0 && beforeY == 0)
				{
					beforeX = contour[i].x;
					beforeY = contour[i].y;
					continue;
				}

				gap = SQR(contour[i].x - beforeX) + SQR(contour[i].y - beforeY);
				if (maxDist < gap)
				{
					maxDist = gap;
					maxIdx = selectedInlier.size() - 1;
				}
				beforeX = contour[i].x;
				beforeY = contour[i].y;
			}
		}
		if (selectedInlier.size() > 0)
		{
			gap = SQR(selectedInlier[0].x - beforeX) + SQR(selectedInlier[0].y - beforeY);
			if (maxDist < gap)
			{
				maxDist = gap;
				maxIdx = 0;
			}

			if (maxDist / (radi * radi) >= 0.5)
			{
				int skipCnt = selectedInlier.size() / 5;
				if (maxIdx < skipCnt)
				{
					int tempSkipCnt = skipCnt - maxIdx;
					selectedInlier.erase(selectedInlier.begin(), selectedInlier.begin() + skipCnt + maxIdx);
					selectedInlier.erase(selectedInlier.end() - tempSkipCnt, selectedInlier.end());
				}
				else if (maxIdx + skipCnt > selectedInlier.size())
				{
					int tempSkipCnt = skipCnt + maxIdx - selectedInlier.size();
					selectedInlier.erase(selectedInlier.begin(), selectedInlier.begin() + tempSkipCnt);
					selectedInlier.erase(selectedInlier.end() - (skipCnt * 2 - tempSkipCnt), selectedInlier.end());
				}
				else
				{
					selectedInlier.erase(selectedInlier.begin() + maxIdx - skipCnt, selectedInlier.begin() + maxIdx + skipCnt);
				}
			}

			GetFiduPos_LSQ(selectedInlier, &dCircle_x, &dCircle_y, &radi, &dErr);
			selectedInlier.clear();
		}
		//==========================1차 원 검출

		//==========================2차 원 검출
		// 1차로 검출된 원의 중심과 반지름을 통해 다시 한번 inlier들을 수집하고
		// 2차로 LSQ를 실행해 최종 원을 검출한다.

		maxDist = maxIdx = 0;
		beforeX = beforeY = gap = 0;
		for (int i = 0; i < contour.size(); i++)
		{
			if (contour[i].x == 0 || contour[i].x == src.cols - 1 || contour[i].y == 0 || contour[i].y == src.rows - 1)
				continue;

			double dist = sqrt(SQR(contour[i].x - dCircle_x) + SQR(contour[i].y - dCircle_y));
			double distGap = fabs(dist - radi);
			if (distGap <= 3)
			{
				selectedInlier.emplace_back(contour[i]);

				if (beforeX == 0 && beforeY == 0)
				{
					beforeX = contour[i].x;
					beforeY = contour[i].y;
					continue;
				}

				gap = SQR(contour[i].x - beforeX) + SQR(contour[i].y - beforeY);
				if (maxDist < gap)
				{
					maxDist = gap;
					maxIdx = selectedInlier.size() - 1;
				}
				beforeX = contour[i].x;
				beforeY = contour[i].y;
			}
		}
		if (selectedInlier.size() > 0)
		{
			gap = SQR(selectedInlier[0].x - beforeX) + SQR(selectedInlier[0].y - beforeY);
			if (maxDist < gap)
			{
				maxDist = gap;
				maxIdx = 0;
			}
			if (maxDist / (radi * radi) >= 0.5)
			{
				int skipCnt = selectedInlier.size() / 5;
				if (maxIdx < skipCnt)
				{
					int tempSkipCnt = skipCnt - maxIdx;
					selectedInlier.erase(selectedInlier.begin(), selectedInlier.begin() + skipCnt + maxIdx);
					selectedInlier.erase(selectedInlier.end() - tempSkipCnt, selectedInlier.end());
				}
				else if (maxIdx + skipCnt > selectedInlier.size())
				{
					int tempSkipCnt = skipCnt + maxIdx - selectedInlier.size();
					selectedInlier.erase(selectedInlier.begin(), selectedInlier.begin() + tempSkipCnt);
					selectedInlier.erase(selectedInlier.end() - (skipCnt * 2 - tempSkipCnt), selectedInlier.end());
				}
				else
				{
					selectedInlier.erase(selectedInlier.begin() + maxIdx - skipCnt, selectedInlier.begin() + maxIdx + skipCnt);
				}
			}

			GetFiduPos_LSQ(selectedInlier, &dCircle_x, &dCircle_y, &radi, &dErr);
		}
		//==========================2차 원 검출

#ifdef DEBUG
		cv::Mat tmp(src.rows, src.cols, CV_8UC3);
		uchar *ptrT = tmp.data, *ptrS = src.data;
		tmp.setTo(0);
		cv::ellipse(tmp, cv::RotatedRect(cv::Point2f(dCircle_x, dCircle_y), cv::Size2f(2 * radi, 2 * radi), 0), cv::Scalar(0, 0, 255));
		for (int i = 0; i < src.rows * src.cols; i++)
		{
			if (ptrS[i] == 0) continue;

			if (ptrT[i * 3 + 2] > 0)
				ptrT[i * 3] = ptrT[i * 3 + 1] = 180;
			else
				ptrT[i * 3] = ptrT[i * 3 + 1] = ptrT[i * 3 + 2] = 255;
		}
		int centeridx = ((int)(dCircle_x + 0.5) + (int)(dCircle_y + 0.5) * src.cols) * 3;
		ptrT[centeridx] = ptrT[centeridx + 1] = 0;
		// 		cv::imwrite("D:\\123.png", tmp);
#endif

		if (dRadius)
			*dRadius = radi;

		if (dErrorRate)
		{
			int cnt = 0;
			for (int i = 0; i < contour.size(); i++)
			{
				double dist = sqrt(SQR(contour[i].x - dCircle_x) + SQR(contour[i].y - dCircle_y));
				double distGap = fabs(dist - radi);
				if (distGap <= 2)
					cnt++;
			}

			*dErrorRate = 1. - (double)cnt / (radi * 6.283185307179586); // <- 2 * Pi (3.14...)
		}

		rcBlob->left = dCircle_x - radi + 0.5 > 0 ? dCircle_x - radi + 0.5 : 0;
		rcBlob->top = dCircle_y - radi + 0.5 > 0 ? dCircle_y - radi + 0.5 : 0;
		rcBlob->right = dCircle_x + radi + 0.5 < src.cols ? dCircle_x + radi + 0.5 : src.cols;
		rcBlob->bottom = dCircle_y + radi + 0.5 < src.rows ? dCircle_y + radi + 0.5 : src.rows;

		if (rcBlob->left >= rcBlob->right || rcBlob->top >= rcBlob->bottom)
		{
			dCircle_x = dCircle_y = 0.;
			if (dRadius)
				*dRadius = 0.;
			if (dErrorRate)
				*dErrorRate = 1.;
			return false;
		}
	}
	catch (cv::Exception ex)
	{
		dCircle_x = dCircle_y = 0.;
		if (dRadius)
			*dRadius = -1.;
		if (dErrorRate)
			*dErrorRate = 1.;

		return false;
	}
	catch (exception ex)
	{
		dCircle_x = dCircle_y = 0.;
		if (dRadius)
			*dRadius = -1.;
		if (dErrorRate)
			*dErrorRate = 1.;

		return false;
	}
	catch (...)
	{
		dCircle_x = dCircle_y = 0.;
		if (dRadius)
			*dRadius = -1.;
		if (dErrorRate)
			*dErrorRate = 1.;

		return false;
	}

	return nCntBlob > 0;
}

bool CPInsp::CrossLineDetect(uchar *src, int nWidth, int nHeight, int nCntBlob, double *dArea, double *dCenterX, double *dCenterY, RECT *rcRect, double dMaxThickness, double dMaxRatio /* ~ INPUT*/, /*OUTPUT ~ */int *arrLabelVert, int *arrLabelHorz, int *arrLabelVertCnt, int *arrLabelHorzCnt, double *arrLabelArea, RECT *arrLabelRect)
{
	bool bReturn = false;
	int nLine = __LINE__, nloopIdx = -1;

	if (src == NULL || dArea == NULL || dCenterX == NULL || dCenterY == NULL || rcRect == NULL
		|| arrLabelHorz == NULL || arrLabelVert == NULL || arrLabelArea == NULL || arrLabelRect == NULL
		|| arrLabelHorzCnt == NULL || arrLabelVertCnt == NULL)
		return bReturn;

	try
	{
		cv::Mat Img(nHeight, nWidth, CV_8UC1, src);

		// 다음에 세팅할 라벨 번호
		int curLabel = 0;

		// 수직, 수평 방향 라벨 저장하는 벡터. Index는 Blob의 Index. 값은 라벨이 없는 Blob은 -1, 있으면 라벨 번호.
		// Vert와 Horz에 중복되는 라벨 번호는 없음. (curLabel 변수 하나로 라벨 번호를 매기기 때문)

		// 각 라벨 번호에 할당된 Blob의 개수. Index는 라벨 번호 (arrLabelVert 또는 arrLabelHorz)

		// 각 라벨 번호에 할당된 Blob 전체의 Rect 범위. Index는 라벨 번호 (arrLabelVert 또는 arrLabelHorz)

		// 각 라벨 번호에 할당된 Blob 전체의 면적. Index는 라벨 번호 (arrLabelVert 또는 arrLabelHorz)

		////// 수직, 수평 방향 직선 성분으로 묶을 때, Blob의 라벨링을 위한 변수들 --- 끝


		// CrossLine Detect 옵션 관련 변수.
		// Blob이 수직, 수평 방향이 교차되는 지점의 모양일 때 ('+', 'L', 'T', 'ㄱ' 등등),
		// 수직방향 성분과 수평방향 성분으로 분리하여 각각의 중심좌표를 따로 저장하는 벡터
		std::vector<std::vector<double>> crossedBlobCenter(nCntBlob);

		nLine = __LINE__;

		for (int i = 0; i < nCntBlob; i++)
		{
			arrLabelVert[i] = arrLabelHorz[i] = -1;

			// 수직, 수평방향으로 교차되는 Blob인지 파악하고 중심을 분리하는 과정
			// Blob Rect의 면적과 실제 Blob의 면적의 비율이 1/2 보다 작으면 + 모양일 가능성이 있다고 본다.
			if (dArea[i] / double((rcRect[i].right - rcRect[i].left)* (rcRect[i].bottom - rcRect[i].top)) < 1. / 2.)
			{
				RECT cRect;

				// 수직방향으로 뻗은 성분 제거 (수평방향으로 긴 마스크로 모폴로지 열림 연산)
				cv::Mat ROIDstImg = Img(cv::Rect(rcRect[i].left, rcRect[i].top, (rcRect[i].right - rcRect[i].left), (rcRect[i].bottom - rcRect[i].top))).clone();
				cv::Mat kernel3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size((rcRect[i].right - rcRect[i].left) / 2, 1));
				cv::erode(ROIDstImg, ROIDstImg, kernel3);
				cv::dilate(ROIDstImg, ROIDstImg, kernel3);

				int nCntBlob2 = m_pProcMilAlgo->CalcBlob_Select(ROIDstImg.ptr(), ROIDstImg.ptr(), ROIDstImg.cols, ROIDstImg.rows, 1, false, false, 0, eSelectMix);
				if (nCntBlob2 != 1)	// 정상적으로 + 모양 또는 T 등의 모양이라면 수직방향으로 뻗은 걸 제거하면 수평방향 blob 한 개만 남아야 한다고 판단
					continue;

				m_pProcMilAlgo->GetBlobResult_Rects(&cRect, 1);
				int tW = cRect.right - cRect.left;
				int tH = cRect.bottom - cRect.top;
				double tRatio = tW < tH ? (double)tW / tH : (double)tH / tW;
				if (tRatio >= dMaxRatio)	// 수평방향의 마스크로 모폴로지 실시해서 수평 성분만 남으면 길고 얇게 남아야 한다고 판단
					continue;

				double t1CenterX, t1CenterY;
				m_pProcMilAlgo->GetBlobResult_Center(&t1CenterX, &t1CenterY);	// 중심좌표 임시 저장. 수직 성분에 대해서도 똑같이 OK 인 경우에만 중심좌표 저장

				nLine = __LINE__;
				nloopIdx = i;

				// 수평방향으로 뻗은 성분 제거 (수직방향으로 긴 마스크로 모폴로지 열림 연산)
				cv::Mat ROIDstImg2 = Img(cv::Rect(rcRect[i].left, rcRect[i].top, (rcRect[i].right - rcRect[i].left), (rcRect[i].bottom - rcRect[i].top))).clone();
				cv::Mat kernel4 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, (rcRect[i].bottom - rcRect[i].top) / 2));
				cv::erode(ROIDstImg2, ROIDstImg2, kernel4);
				cv::dilate(ROIDstImg2, ROIDstImg2, kernel4);

				nCntBlob2 = m_pProcMilAlgo->CalcBlob_Select(ROIDstImg2.ptr(), ROIDstImg2.ptr(), ROIDstImg2.cols, ROIDstImg2.rows, 1, false, false, 0, eSelectMix);
				if (nCntBlob2 != 1)	// 수평방향으로 뻗은 걸 제거하면 수직방향 blob 한 개만 남아야 한다고 판단
					continue;

				m_pProcMilAlgo->GetBlobResult_Rects(&cRect, 1);
				tW = cRect.right - cRect.left;
				tH = cRect.bottom - cRect.top;
				tRatio = tW < tH ? (double)tW / tH : (double)tH / tW;
				if (tRatio >= dMaxRatio)	// 수직방향의 마스크로 모폴로지 실시해서 수직 성분만 남으면 길고 얇게 남아야 한다고 판단
					continue;

				double t2CenterX, t2CenterY;
				m_pProcMilAlgo->GetBlobResult_Center(&t2CenterX, &t2CenterY);	// 중심좌표 임시 저장

				dCenterX[i] = dCenterY[i] = -1;
				crossedBlobCenter[i].resize(4);
				crossedBlobCenter[i][0] = t1CenterX;
				crossedBlobCenter[i][1] = t1CenterY;
				crossedBlobCenter[i][2] = t2CenterX;
				crossedBlobCenter[i][3] = t2CenterY;
			}
		}

		nLine = __LINE__;
		nloopIdx = -1;

		// 수직, 수평 방향으로 묶는 과정
		for (int i = 0; i < nCntBlob - 1; i++)
		{
			double cX_I = dCenterX[i], cY_I = dCenterY[i];
			bool isCross_I = false;
			if (cX_I < 0 && cY_I < 0)	// + 모양이라 중심 검출 다시 했는지 여부 파악
				isCross_I = true;

			for (int j = i + 1; j < nCntBlob; j++)
			{
				double cX_J = dCenterX[j], cY_J = dCenterY[j];
				bool isCross_J = false;
				if (cX_J < 0 && cY_J < 0)	// + 모양이라 중심 검출 다시 했는지 여부 파악
					isCross_J = true;

				if (isCross_I)
				{
					cX_I = crossedBlobCenter[i][2];
					cY_I = crossedBlobCenter[i][3];
				}
				if (isCross_J)
				{
					cX_J = crossedBlobCenter[j][2];
					cY_J = crossedBlobCenter[j][3];
				}

				// Blob Rect의 수직방향 거리
				double dist = cY_I > cY_J ? (rcRect[i].top - rcRect[j].bottom) : (rcRect[j].top - rcRect[i].bottom);

				// 중심의 X 좌표가 거의 동일하고 Rect 간의 거리가 가까울 때, 또는 Blob Rect의 left 나 right가 거의 동일하고 거리가 매우 가까울 때 수직 성분으로 묶는다
				if ((abs(cX_I - cX_J) <= 1.5 && dist < nHeight * 0.2) || ((abs(rcRect[i].left - rcRect[j].left) <= 1 || abs(rcRect[i].right - rcRect[j].right) <= 1) && dist < 7))
				{
					float width_I = rcRect[i].right - rcRect[i].left;
					float width_J = rcRect[j].right - rcRect[j].left;
					float widthRatio = width_I > width_J ? width_J / width_I : width_I / width_J;

					// 두께 차이가 너무 많이 나면 묶지 않는다.
					// + 모양은 두께를 측정하기 어려워서 그냥 묶고, 거리가 매우 가까우면 두께 차이가 조금 심해도 묶는다.
					if (!isCross_I && !isCross_J && (dist >= 5 && widthRatio <= 0.4 || widthRatio <= 0.15))
						continue;

					// 수직방향 라벨링 작업

					if (arrLabelVert[i] < 0)
					{
						arrLabelVert[i] = curLabel++;
						arrLabelVertCnt[curLabel - 1]++;

						// Blob이 충분히 길면 2개로 카운트한다.
						if (double(rcRect[i].right - rcRect[i].left) / double(rcRect[i].bottom - rcRect[i].top) < dMaxRatio)
							arrLabelVertCnt[curLabel - 1]++;
					}

					int label_I = arrLabelVert[i];
					int label_J = arrLabelVert[j];

					if (label_I == label_J)
						continue;

					if (label_J >= 0)
					{
						arrLabelVertCnt[label_I] += arrLabelVertCnt[label_J];
						arrLabelVertCnt[label_J] = 0;

						for (int a = 0; a < nCntBlob; a++)
						{
							if (arrLabelVert[a] == label_J)
								arrLabelVert[a] = label_I;
						}
					}
					else
					{
						arrLabelVert[j] = label_I;
						arrLabelVertCnt[label_I]++;

						if (double(rcRect[j].right - rcRect[j].left) / double(rcRect[j].bottom - rcRect[j].top) < dMaxRatio)
							arrLabelVertCnt[label_I]++;
					}

					// 수직방향 라벨링 끝

					continue;
				}

				nLine = __LINE__;
				nloopIdx = i;

				if (isCross_I)
				{
					cX_I = crossedBlobCenter[i][0];
					cY_I = crossedBlobCenter[i][1];
				}
				if (isCross_J)
				{
					cX_J = crossedBlobCenter[j][0];
					cY_J = crossedBlobCenter[j][1];
				}

				// 여기부터 수평방향 직선 성분 묶는 시퀀스. 수직방향과 동일함

				dist = cX_I < cX_J ? (rcRect[j].left - rcRect[i].right) : (rcRect[i].left - rcRect[j].right);

				if ((abs(cY_I - cY_J) <= 1.5 && dist < nWidth * 0.15) || ((abs(rcRect[i].bottom - rcRect[j].bottom) <= 1 || abs(rcRect[i].top - rcRect[j].top) <= 1) && dist < 10))
				{
					float height_I = rcRect[i].bottom - rcRect[i].top;
					float height_J = rcRect[j].bottom - rcRect[j].top;
					float heightRatio = height_I > height_J ? height_J / height_I : height_I / height_J;

					if (!isCross_I && !isCross_J && (dist >= 7 && heightRatio <= 0.4 || heightRatio <= 0.15))
						continue;

					if (arrLabelHorz[i] < 0)
					{
						arrLabelHorz[i] = curLabel++;
						arrLabelHorzCnt[curLabel - 1]++;

						// Blob이 충분히 길면 2개로 카운트한다.
						if (double(rcRect[i].bottom - rcRect[i].top) / double(rcRect[i].right - rcRect[i].left) < dMaxRatio)
							arrLabelHorzCnt[curLabel - 1]++;
					}

					int label_I = arrLabelHorz[i];
					int label_J = arrLabelHorz[j];

					if (label_I == label_J)
						continue;

					if (label_J >= 0)
					{
						arrLabelHorzCnt[label_I] += arrLabelHorzCnt[label_J];
						arrLabelHorzCnt[label_J] = 0;

						for (int a = 0; a < nCntBlob; a++)
						{
							if (arrLabelHorz[a] == label_J)
								arrLabelHorz[a] = label_I;
						}
					}
					else
					{
						arrLabelHorz[j] = label_I;
						arrLabelHorzCnt[label_I]++;

						// Blob이 충분히 길면 2개로 카운트한다.
						if (double(rcRect[j].bottom - rcRect[j].top) / double(rcRect[j].right - rcRect[j].left) < dMaxRatio)
							arrLabelHorzCnt[label_I]++;
					}
				}
			}
		}

		nLine = __LINE__;
		nloopIdx = -1;

		// 아래 For문은 각 라벨마다 직선성분인 Blob들의 전체 Rect와 Area를 계산하는 과정
		for (int i = 0; i < nCntBlob; i++)
		{
			// 같은 라벨로 묶인 blob 개수가 3개 이상이어야 직선 성분으로 판단한다.
			bool bHorz = arrLabelHorz[i] >= 0 && arrLabelHorzCnt[arrLabelHorz[i]] > 2;
			bool bVert = arrLabelVert[i] >= 0 && arrLabelVertCnt[arrLabelVert[i]] > 2;

			if (bHorz)
			{
				int label = arrLabelHorz[i];

				if (arrLabelArea[label] == 0)
				{
					arrLabelRect[label] = rcRect[i];
				}
				else
				{
					if (arrLabelRect[label].left > rcRect[i].left)
						arrLabelRect[label].left = rcRect[i].left;
					if (arrLabelRect[label].top > rcRect[i].top)
						arrLabelRect[label].top = rcRect[i].top;
					if (arrLabelRect[label].right < rcRect[i].right)
						arrLabelRect[label].right = rcRect[i].right;
					if (arrLabelRect[label].bottom < rcRect[i].bottom)
						arrLabelRect[label].bottom = rcRect[i].bottom;

					arrLabelArea[label] += dArea[i];
				}
			}

			nLine = __LINE__;
			nloopIdx = i;

			if (bVert)
			{
				int label = arrLabelVert[i];

				if (arrLabelArea[label] == 0)
				{
					arrLabelRect[label] = rcRect[i];
				}
				else
				{
					if (arrLabelRect[label].left > rcRect[i].left)
						arrLabelRect[label].left = rcRect[i].left;
					if (arrLabelRect[label].top > rcRect[i].top)
						arrLabelRect[label].top = rcRect[i].top;
					if (arrLabelRect[label].right < rcRect[i].right)
						arrLabelRect[label].right = rcRect[i].right;
					if (arrLabelRect[label].bottom < rcRect[i].bottom)
						arrLabelRect[label].bottom = rcRect[i].bottom;
				}

				arrLabelArea[label] += dArea[i];
			}
		}

		nLine = __LINE__;
		nloopIdx = -1;

		bReturn = true;
	}
	catch (CMemoryException* e)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			if(nloopIdx >= 0)
				sLog.Format(_T("CPInsp::CrossLineDetect(), Line : %d, loopIndex : %d / %d Pass!!!"), nLine, nloopIdx, nCntBlob);
			else
				sLog.Format(_T("CPInsp::CrossLineDetect(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
	catch (CFileException* e)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			if (nloopIdx >= 0)
				sLog.Format(_T("CPInsp::CrossLineDetect(), Line : %d, loopIndex : %d / %d Pass!!!"), nLine, nloopIdx, nCntBlob);
			else
				sLog.Format(_T("CPInsp::CrossLineDetect(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}
	catch (CException* e)
	{
		if (g_pMPTI)
		{
			CString sLog = _T("");
			if (nloopIdx >= 0)
				sLog.Format(_T("CPInsp::CrossLineDetect(), Line : %d, loopIndex : %d / %d Pass!!!"), nLine, nloopIdx, nCntBlob);
			else
				sLog.Format(_T("CPInsp::CrossLineDetect(), Line : %d Pass!!!"), nLine);
			g_pMPTI->AddLog_Dev(sLog);
			throw nLine;
		}
	}

	return bReturn;
}

BOOL CPInsp::ImageThining(int nWndWidth, int nWndHeight, UCHAR *ucArrSrcWnd, UCHAR *ucArrDstWnd, bool bnot)
{
	cv::Mat SrcImg(nWndHeight, nWndWidth, CV_8UC1, ucArrSrcWnd);
	cv::Mat Img = SrcImg.clone();
	//cv::Mat ReadImg = cv::imread(std::string(CT2A(imgPath)));
	//ReadImg.convertTo(Img,CV_8UC1);
	//cv::threshold(Img, Img, 1, 255, cv::THRESH_OTSU);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(3,3));
	cv::erode(Img, Img, kernel);
	cv::dilate(Img, Img, kernel);

	if (bnot)
	{
		Img = ~Img;
	}

	//cv::imwrite(std::string(CT2A(BinPath)),Img);

	cv::Mat tmp, temp1;

	//	cv::Canny(Img,Img,125,225);

	Img.copyTo(tmp);
	Img.copyTo(temp1);
	//	Img.copySize(tmp);
	//	tmp.setTo(0);
	bool flag = true;

	int cnt(0);
	int nStep(0);
	int nPoint[8] = { 0,0,0,0,0,0,0,0 };
	int ntempPoint[8] = { 0,0,0,0,0,0,0,0 };

	while (flag)
	{
		flag = false;
#if 0
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (temp1.data[r*Img.cols + c] == 0)
					continue;

				nPoint[0] = Img.data[(r - 1)*Img.cols + c - 1];
				nPoint[1] = Img.data[(r - 1)*Img.cols + c];
				nPoint[2] = Img.data[(r - 1)*Img.cols + c + 1];

				nPoint[3] = Img.data[(r)*Img.cols + c + 1];
				nPoint[4] = Img.data[(r + 1)*Img.cols + c + 1];

				nPoint[5] = Img.data[(r + 1)*Img.cols + c];
				nPoint[6] = Img.data[(r + 1)*Img.cols + c - 1];

				nPoint[7] = Img.data[(r)*Img.cols + c - 1];

				ntempPoint[0] = temp1.data[(r - 1)*Img.cols + c - 1];
				ntempPoint[1] = temp1.data[(r - 1)*Img.cols + c];
				ntempPoint[2] = temp1.data[(r - 1)*Img.cols + c + 1];
				ntempPoint[3] = temp1.data[(r)*Img.cols + c + 1];
				ntempPoint[4] = temp1.data[(r + 1)*Img.cols + c + 1];
				ntempPoint[5] = temp1.data[(r + 1)*Img.cols + c];
				ntempPoint[6] = temp1.data[(r + 1)*Img.cols + c - 1];
				ntempPoint[7] = temp1.data[(r)*Img.cols + c - 1];
				int nConnect(0);
				// 				int nL(0),nR(0),nT(0),nB(0),BR(0);
				// 				nL = nPoint[0]&nPoint[6]&nPoint[7]&1;
				// 				nR = nPoint[2]&nPoint[3]&nPoint[4]&1;
				// 				nT = nPoint[0]&nPoint[1]&nPoint[2]&1;
				// 				nB = nPoint[4]&nPoint[5]&nPoint[6]&1;
				// 				BR = nPoint[5]&nPoint[3]&nPoint[4]&1;
				// 				int nBCnt = nL+nR+nT+nB + BR;

				int nContin(0);
				bool bContin = false;

				for (int i = 1; i < 8; i++)
				{
					if (nPoint[i - 1] == 0 && nPoint[i] == 255)
						nConnect++;

					if (ntempPoint[i] == 255)
						nContin++;
					else
						nContin = 0;

					if (nContin > 2)
						bContin = true;
				}
				if (nPoint[7] == 0 && nPoint[0] == 255)
					nConnect++;

				if (ntempPoint[0] == 255)
					nContin++;
				else
					nContin = 0;

				if (nContin > 2)
					bContin = true;

				if (nConnect == 1 && bContin/*nBCnt>0*/)
				{
					temp1.data[r*Img.cols + c] = 0;
					flag = true;

					// 					CString SavePath;
					// 					SavePath.Format(_T("%s_step_%d.bmp"),sDirPath,nStep);
					// 					cv::imwrite(std::string(CT2A(SavePath)),temp1);
					// 					SavePath.Format(_T("%s_step_%d_Img.bmp"),sDirPath,nStep);
					// 					cv::imwrite(std::string(CT2A(SavePath)),Img);
					nStep++;
				}
			}
		}
		temp1.copyTo(Img);
		CString SavePath;
		SavePath.Format(_T("%s_temp_%d.bmp"), sDirPath, cnt);
		cv::imwrite(std::string(CT2A(SavePath)), temp1);
		cnt++;
#endif
#if 1

		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (temp1.data[r*Img.cols + c] == 0)
					continue;

				nPoint[0] = Img.data[(r - 1)*Img.cols + c - 1];
				nPoint[1] = Img.data[(r - 1)*Img.cols + c];
				nPoint[2] = Img.data[(r - 1)*Img.cols + c + 1];

				nPoint[3] = Img.data[(r)*Img.cols + c + 1];
				nPoint[4] = Img.data[(r + 1)*Img.cols + c + 1];

				nPoint[5] = Img.data[(r + 1)*Img.cols + c];
				nPoint[6] = Img.data[(r + 1)*Img.cols + c - 1];

				nPoint[7] = Img.data[(r)*Img.cols + c - 1];

				ntempPoint[0] = temp1.data[(r - 1)*Img.cols + c - 1];
				ntempPoint[1] = temp1.data[(r - 1)*Img.cols + c];
				ntempPoint[2] = temp1.data[(r - 1)*Img.cols + c + 1];
				ntempPoint[3] = temp1.data[(r)*Img.cols + c + 1];
				ntempPoint[4] = temp1.data[(r + 1)*Img.cols + c + 1];
				ntempPoint[5] = temp1.data[(r + 1)*Img.cols + c];
				ntempPoint[6] = temp1.data[(r + 1)*Img.cols + c - 1];
				ntempPoint[7] = temp1.data[(r)*Img.cols + c - 1];
				int nConnect(0);
				int nPixCnt(0);
				int nCndi1(0), nCndi2(0);

				for (int i = 1; i < 8; i++)
				{
					if (nPoint[i - 1] == 0 && nPoint[i] == 255)
						nConnect++;
					if (nPoint[i] == 255)
						nPixCnt++;
				}
				if (nPoint[7] == 0 && nPoint[0] == 255)
					nConnect++;
				if (nPoint[0] == 255)
					nPixCnt++;

				nCndi1 = nPoint[1] & nPoint[7] & nPoint[5];
				nCndi2 = nPoint[3] & nPoint[7] & nPoint[5];

				if (nConnect == 1 && (nPixCnt > 1 && nPixCnt < 7) && nCndi1 == 0 && nCndi2 == 0)
				{
					temp1.data[r*Img.cols + c] = 0;
					flag = true;

					nStep++;
				}
			}
		}
		temp1.copyTo(Img);
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (temp1.data[r*Img.cols + c] == 0)
					continue;

				nPoint[0] = Img.data[(r - 1)*Img.cols + c - 1];
				nPoint[1] = Img.data[(r - 1)*Img.cols + c];
				nPoint[2] = Img.data[(r - 1)*Img.cols + c + 1];

				nPoint[3] = Img.data[(r)*Img.cols + c + 1];
				nPoint[4] = Img.data[(r + 1)*Img.cols + c + 1];

				nPoint[5] = Img.data[(r + 1)*Img.cols + c];
				nPoint[6] = Img.data[(r + 1)*Img.cols + c - 1];

				nPoint[7] = Img.data[(r)*Img.cols + c - 1];
				int nConnect(0);
				int nPixCnt(0);
				int nCndi1(0), nCndi2(0);

				for (int i = 1; i < 8; i++)
				{
					if (nPoint[i - 1] == 0 && nPoint[i] == 255)
						nConnect++;
					if (nPoint[i] == 255)
						nPixCnt++;
				}
				if (nPoint[7] == 0 && nPoint[0] == 255)
					nConnect++;
				if (nPoint[0] == 255)
					nPixCnt++;

				nCndi1 = nPoint[1] & nPoint[3] & nPoint[5];
				nCndi2 = nPoint[1] & nPoint[3] & nPoint[7];

				if (nConnect == 1 && (nPixCnt > 1 && nPixCnt < 7) && nCndi1 == 0 && nCndi2 == 0)
				{
					temp1.data[r*Img.cols + c] = 0;
					flag = true;
					nStep++;
				}
			}
		}
		temp1.copyTo(Img);
		cnt++;
#else
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (Img.data[r*Img.cols + c] == 255)
				{
					int up(0), down(0), side(0), nLeft(0), nRight(0), Lt(0), Rt(0), Lb(0), Rb(0), upside(0), Line1(0), Line2(0), Line3(0), Line4(0);
					up += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					up += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					up += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					side += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					side += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;

					nLeft += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					nLeft += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					nLeft += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					nRight += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					nRight += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					nRight += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					upside += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					upside += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Lt += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Lt += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					Lt += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;

					Rt += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					Rt += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					Rt += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;

					Lb += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Lb += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					Lb += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Rb += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					Rb += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					Rb += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Line1 += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Line1 += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;

					Line2 += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					Line2 += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Line3 += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Line3 += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;

					Line4 += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Line4 += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;

					if ((up >= 1 && down >= 1 && side >= 1) || (nLeft >= 1 && nRight >= 1 && upside >= 1))
						if (up + down + side > 2 && up + down + side < 7)
						{
							int backg = Img.data[r*Img.cols + c - 1] * Img.data[(r + 1)*Img.cols + c] > 0 ? 1 : 0;
							if (Img.data[(r - 1)*Img.cols + c] * backg == 0)
							{
								if (Img.data[r*Img.cols + c + 1] * backg == 0)
								{
									temp1.data[r*Img.cols + c] = 0;
									flag = true;
								}
							}
						}
				}

			}
		}
		temp1.copyTo(Img);
		for (int r = 1; r < Img.rows - 1; r++)
		{
			for (int c = 1; c < Img.cols - 1; c++)
			{
				if (Img.data[r*Img.cols + c] == 255)//현재 픽셀 == edge
				{
					int up(0), down(0), side(0), nLeft(0), nRight(0), Lt(0), Rt(0), Lb(0), Rb(0), upside(0);
					up += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					up += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					up += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;
					down += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					side += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					side += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;

					nLeft += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					nLeft += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					nLeft += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					nRight += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					nRight += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					nRight += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					upside += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;
					upside += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;


					Lt += Img.data[(r - 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Lt += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					Lt += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;

					Rt += Img.data[(r - 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					Rt += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					Rt += Img.data[(r - 1)*Img.cols + c] == 255 ? 1 : 0;

					Lb += Img.data[(r + 1)*Img.cols + c - 1] == 255 ? 1 : 0;
					Lb += Img.data[r*Img.cols + c - 1] == 255 ? 1 : 0;
					Lb += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					Rb += Img.data[(r + 1)*Img.cols + c + 1] == 255 ? 1 : 0;
					Rb += Img.data[r*Img.cols + c + 1] == 255 ? 1 : 0;
					Rb += Img.data[(r + 1)*Img.cols + c] == 255 ? 1 : 0;

					if ((up >= 1 && down >= 1 && side >= 1) || (nLeft >= 1 && nRight >= 1 && upside >= 1))
						if (up + down + side > 2 && up + down + side < 7)
						{
							int backg = Img.data[(r - 1)*Img.cols + c] * Img.data[r*Img.cols + c + 1] > 0 ? 1 : 0;
							if (Img.data[(r + 1)*Img.cols + c] * backg == 0)
							{
								if (Img.data[r*Img.cols + c - 1] * backg == 0)
								{
									temp1.data[r*Img.cols + c] = 0;
									flag = true;
								}
							}
						}
				}
			}
		}

		temp1.copyTo(Img);
		CString SavePath;
		SavePath.Format(_T("%s_temp_%d.bmp"), imgPath, cnt);
		cv::imwrite(std::string(CT2A(SavePath)), temp1);
		cnt++;
#endif
	}

	cv::Mat DstImg(nWndHeight, nWndWidth, CV_8UC1, ucArrDstWnd);
	temp1.copyTo(DstImg);
	//cv::imwrite(std::string(CT2A(DstPath)), temp1);
	return TRUE;
}
void CPInsp::CircleIntersection(UCHAR* pucBin, int nW, int nH, double *centX, double *centY, double *radius, double *err, double ini_radius, int nFindPer, float fMin, float fMax)
{
	cv::Mat imgBin(nH, nW, CV_8UC1, pucBin);

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(imgBin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	if (contours.size() < 1 || contours[0].size() == 0)
		return;

	std::vector<cv::Point> contour = contours[0];
	int nBorderCnt = contour.size();
	double dRatioPer = (double)nFindPer / 100.0; // 0.00 ~ 1.00
	if (dRatioPer < 0.0)
		dRatioPer = 0.0;
	else if (dRatioPer > 1.0)
		dRatioPer = 1.0;

	int nPointCount = nBorderCnt * dRatioPer;
	if (nPointCount <= 0)
		nPointCount = 1;
	size_t nRatio = nBorderCnt / nPointCount;

	std::vector<cv::Point2f> intersections;

	for (size_t i = 0; i < nBorderCnt; i += nRatio)
	{
		cv::Point2f center1 = contour[i];
		for (size_t j = 0; j < nBorderCnt; j += nRatio)
		{
			if (i == j)
				continue;

			cv::Point2f center2 = contour[j];
			float distance = cv::norm(center1 - center2);

			if (distance <= 2 * ini_radius)
			{
				float angle = acos(distance / (2 * ini_radius));
				cv::Point2f direction = (center2 - center1) / distance;
				cv::Point2f intersection1 = center1 + ini_radius * (direction * cos(angle) + cv::Point2f(-direction.y, direction.x) * sin(angle));
				cv::Point2f intersection2 = center1 + ini_radius * (direction * cos(angle) - cv::Point2f(-direction.y, direction.x) * sin(angle));

				// 교차점이 Contour 내부에 있는지 확인
				if (cv::pointPolygonTest(contour, intersection1, false) >= 0)
				{
					intersections.push_back(intersection1);
				}
				if (cv::pointPolygonTest(contour, intersection2, false) >= 0)
				{
					intersections.push_back(intersection2);
				}
			}
		}
	}

	cv::Point2f sum(0, 0);
	for (const auto& intersection : intersections)
	{
		sum += intersection;
	}

	if (intersections.empty())
	{
		*radius = 0.;
		*err = 101.;
		return; // 교차점이 없는 경우
	}

	// cv::Mat 형식으로 변환
	cv::Mat data(intersections.size(), 2, CV_32F);
	for (size_t i = 0; i < intersections.size(); i++)
	{
		data.at<float>(i, 0) = intersections[i].x;
		data.at<float>(i, 1) = intersections[i].y;
	}

	// k-means 알고리즘 적용
	cv::Mat labels, centers;
	int k = intersections.size() > 9 ? 9 : intersections.size() - 1;

	if (k > 0)
	{
		cv::kmeans(data, k, labels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0), 3, cv::KMEANS_RANDOM_CENTERS, centers);

		// 가장 큰 군집 찾기
		std::vector<int> cluster_sizes(k, 0);
		for (int i = 0; i < labels.rows; i++)
		{
			int cluster_idx = labels.at<int>(i);
			cluster_sizes[cluster_idx]++;
		}

		int max_cluster_idx = std::distance(cluster_sizes.begin(), std::max_element(cluster_sizes.begin(), cluster_sizes.end()));

		// 가장 큰 군집의 중심점 반환
		*centX = centers.at<float>(max_cluster_idx, 0);
		*centY = centers.at<float>(max_cluster_idx, 1);

		float dMinRadius = (float)ini_radius * fMin / 100.f;
		float dMaxRadius = (float)ini_radius * fMax / 100.f;
		int nOKCnt = 0;
		double theta;
		double dSumRadius = 0.;
		double dError = 100.;

		for (int i = 0; i < nBorderCnt; i++)
		{
			CPoint pt(contour[i].x, contour[i].y);

			theta = atan2(-(pt.y - *centY), (pt.x - *centX));
			double coth = cos(theta);
			double sith = -sin(theta);

			double dCothR = (1 / coth) * (pt.x - *centX);
			double dSithR = (1 / sith) * (pt.y - *centY);
			double dRadius = (dCothR + dSithR) / 2.0;

			if (dMinRadius <= dRadius && dRadius <= dMaxRadius)
			{
				dSumRadius += dRadius;
				nOKCnt++;
			}
		}

		*radius = dSumRadius / (double)nOKCnt;
		dError = 1. - (double)nOKCnt / (double)nBorderCnt;
		*err = dError * 100.0;
	}
	else
	{
		*radius = 0.;
		*err = 101.;
	}
}
bool CPInsp::GetROICenter(int nSelectBlob, cv::Mat img, POINTF* poArrDstPoint)
{
	bool bFindLRTB = false;
	if (nSelectBlob != eSelectBigger)
		return bFindLRTB;

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int nTotal = contours.size();
	if (nTotal < 1)
		return bFindLRTB;

	int nFindPos = 4;
	int nCnt = contours[0].size();
	if (nCnt < nFindPos)
		return bFindLRTB;

	int nMinID = -1;
	int nMinCnt = nCnt;
	for (int nRe = 0; nRe < 2; nRe++)
	{
		if (nRe > 0)
			nCnt = 1000;
		for (int a = 0; a < nCnt; a++)
		{
			std::vector<cv::Point> arr;
			double dVal = a;
			if (nRe > 0)
				dVal = nMinID + (0.001 * a);
			if (a == 0)
			{
				if (nRe > 0) continue;
				arr = contours[0];
			}
			else
				cv::approxPolyDP(contours[0], arr, dVal, true);
			if (arr.size() == nFindPos)
			{
				// LT
				poArrDstPoint[0].x = arr[0].x;
				poArrDstPoint[0].y = arr[0].y;
				// RT
				poArrDstPoint[1].x = arr[3].x;
				poArrDstPoint[1].y = arr[3].y;
				// LB
				poArrDstPoint[2].x = arr[1].x;
				poArrDstPoint[2].y = arr[1].y;
				// RB
				poArrDstPoint[3].x = arr[2].x;
				poArrDstPoint[3].y = arr[2].y;
				bFindLRTB = true;
				break;
			}
			if (arr.size() < nFindPos)
				break;
			if (nRe == 0)
			{
				nMinCnt = arr.size();
				nMinID = a;
			}
		}
		if (bFindLRTB)
			break;
	}
	return bFindLRTB;
}
bool CPInsp::BinCheck(float fVal, int nRange, double dL, double dH)
{
	switch (nRange)
	{
	case eTypeRangeIn:
		if (dL <= fVal && fVal <= dH)
			return true;
		break;
	case eTypeRangeOut:
		if (dL >= fVal || fVal >= dH)
			return true;
		break;
	case eTypeRangeUpper:
		if (fVal >= dH)
			return true;
		break;
	case eTypeRangeLower:
		if (dL >= fVal)
			return true;
		break;
	}
	return false;
}
void CPInsp::Binarize(cv::Mat imgSrc, cv::Mat imgRst, int nRange, double dL, double dH, BOOL b3D)
{
	if (imgSrc.cols != imgRst.cols || imgSrc.rows != imgRst.rows)
		return;

	int nSizeImg = imgSrc.cols * imgSrc.rows;
	memset(imgRst.data, 0, nSizeImg * sizeof(UCHAR));
#pragma omp parallel for
	for (int i = 0; i < nSizeImg; ++i)
	{
		float fVal = 0;
		if (b3D)
		{
			float* pf = imgSrc.ptr<float>();
			fVal = pf[i];
		}
		else
			fVal = imgSrc.data[i];

		bool bBin = BinCheck(fVal, nRange, dL, dH);
		if (bBin)
			imgRst.data[i] = 255;
	}
}
void CPInsp::Binarize(cv::Mat img2D, cv::Mat img3D, cv::Mat imgRst, int n2DRange, double d2DL, double d2DH, int n3DRange, double d3DL, double d3DH)
{
	if (img2D.cols != imgRst.cols || img2D.cols != img3D.cols ||
		img2D.rows != imgRst.rows || img2D.rows != img3D.rows)
		return;

	int nSizeImg = img2D.cols * img2D.rows;
	memset(imgRst.data, 0, nSizeImg * sizeof(UCHAR));
	float* pf = img3D.ptr<float>();
#pragma omp parallel for
	for (int i = 0; i < nSizeImg; i++)
	{
		bool b2D = BinCheck(img2D.data[i], n2DRange, d2DL, d2DH);
		bool b3D = BinCheck(pf[i], n3DRange, d3DL, d3DH);
		if (b2D && b3D)
			imgRst.data[i] = 255;
	}
}
POINTF CPInsp::GetIntersection(POINTF A1, POINTF A2, POINTF B1, POINTF B2)
{
	POINTF poC;
	poC.x = -1;
	poC.y = -1;
	// 직선 A: (x1, y1), (x2, y2)
	double a1 = A2.y - A1.y;
	double b1 = A1.x - A2.x;
	double c1 = a1 * A1.x + b1 * A1.y;

	// 직선 B: (x3, y3), (x4, y4)
	double a2 = B2.y - B1.y;
	double b2 = B1.x - B2.x;
	double c2 = a2 * B1.x + b2 * B1.y;

	double determinant = a1 * b2 - a2 * b1;

	if (std::abs(determinant) < 1e-10) {
		// 두 직선이 평행함
		return poC;
	}

	poC.x = (b2 * c1 - b1 * c2) / determinant;
	poC.y = (a1 * c2 - a2 * c1) / determinant;

	return poC;
}
double CPInsp::GetAngleBetweenLines(POINTF poA, POINTF poB, POINTF poC)
{
	double dLimitAng1 = atan2(poA.y - poC.y, poA.x - poC.x);
	double dLimitAng2 = atan2(poB.y - poC.y, poB.x - poC.x);
	double dLimitAng1_A = (dLimitAng1 / M_PI * 180.0);
	double dLimitAng2_A = (dLimitAng2 / M_PI * 180.0);
	double dLimitAng1_B = Mod(dLimitAng1_A, 360.0);
	double dLimitAng2_B = Mod(dLimitAng2_A, 360.0);
	double dArcAngle = 0;
	if (abs(dLimitAng2_B - dLimitAng1_B) < 180)
		dArcAngle = dLimitAng2_B - dLimitAng1_B;
	else if (abs((360 - (dLimitAng2_B - dLimitAng1_B))) < 360)
		dArcAngle = -(360 - (dLimitAng2_B - dLimitAng1_B));
	else
		dArcAngle = (dLimitAng2_B - dLimitAng1_B) + 360;

	if (dArcAngle < 0)
		dArcAngle *= -1.0;

	return dArcAngle;
}
double CPInsp::Mod(double dA, double dN)
{
	return fmod(fmod(dA, dN) + dN, dN);
}