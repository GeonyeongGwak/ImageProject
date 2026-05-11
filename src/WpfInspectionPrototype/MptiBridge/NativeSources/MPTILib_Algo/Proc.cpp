#include "StdAfx.h"
#include "Proc.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

#define Use_Memcpy	1

CProc::CProc(void)
{
	m_className = _T("CProc");

	m_resolX = 0;
	m_resolY = 0;
}


CProc::~CProc(void)
{

}

void CProc::SetResol(double resolX, double resolY, int fovWidth)
{
	m_resolX = resolX;
	m_resolY = resolY;
	m_fovWidth = fovWidth;
}

UCHAR* CProc::AllocUserBuff(int sizeX, int sizeY, int band)
{
	if(!(band == 1 || band == 3))
		band = 1;

	UCHAR* temp = NULL;
	temp = new UCHAR[sizeX * sizeY * band];
	memset(temp, 0, sizeof(UCHAR) * sizeX * sizeY * band);

	return temp;
}

void CProc::DeleteUserBuff(UCHAR** userBuf)
{
	if(*userBuf != NULL)
	{
		delete *userBuf;
		*userBuf = NULL;
	}
}



double CProc::_pixel2mm_x(double val)
{
	double ret = 0;
	ret = val * m_resolX;
	return ret;		
}

double CProc::_pixel2mm_y(double val)
{
	double ret = 0;
	ret = val * m_resolY;
	return ret;		
}

double CProc::_mm2pixel_x(double val)
{
	double ret = 0;
	ret = val / m_resolX;
	return ret;
}

double CProc::_mm2pixel_y(double val)
{
	double ret = 0;
	ret = val / m_resolY;
	return ret;
}


BOOL CProc::FileExists(CString strFilePath)
{
	CFileFind aFile;
	BOOL IsExist = aFile.FindFile(strFilePath);
	aFile.Close();
	return IsExist;
}

int CProc::CorrectCoordinate(double pos_x, double pos_y, double ct_x, double ct_y, double theta, double delta_x, double delta_y, double *corr_x, double *corr_y)
{
	if(theta == 0 && delta_x == 0 && delta_y == 0)
	{
		*corr_x = pos_x;
		*corr_y = pos_y;
		return 0;
	}
	double cosTheta = cos(theta * (PI / 180.0));	// degree to radian
	double sinTheta = sin(theta * (PI / 180.0));

	// move to standard origin position
	double mX = pos_x - ct_x;
	double mY = pos_y - ct_y;

	// correct rotate theta
	double rX =  mX * cosTheta - mY * sinTheta;
	double rY =  mX * sinTheta + mY * cosTheta;

	// correct shift delta
	mX = rX + delta_x;
	mY = rY + delta_y;

	// move to current origin position
	*corr_x = mX + ct_x;
	*corr_y = mY + ct_y;

	return 1;
}

int CProc::_CorrectCoordinate(double pos_x, double pos_y, double ct_x, double ct_y, double theta, double delta_x, double delta_y, double *corr_x, double *corr_y)
{
	if(theta == 0 && delta_x == 0 && delta_y == 0)
	{
		*corr_x = ct_x;
		*corr_y = ct_y;
		return 0;
	}
	double cosTheta = cos(theta * (PI / 180.0));	// degree to radian
	double sinTheta = sin(theta * (PI / 180.0));

	// move to standard origin position
	double mX = pos_x - ct_x;
	double mY = pos_y - ct_y;

	// correct rotate theta
	double rX =  mX * cosTheta + mY * sinTheta;
	double rY =  -mX * sinTheta + mY * cosTheta;

	// correct shift delta
	mX = rX + delta_x;
	mY = rY + delta_y;

	// move to current origin position
	*corr_x = mX + ct_x;
	*corr_y = mY + ct_y;

	return 0;
}

void CProc::CopyBuffer(UCHAR* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, UCHAR* pUcSrcBuffer, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY)
{
	if(!pUcDstBuffer || !pUcSrcBuffer)
		return;

	#pragma omp parallel for /*collapse(2)*/
	for(int y = 0; y < nSrcSizeY;y++)
	{
		if((y + nOffsetY) > nDstSizeY)
			continue;

		int nSizeNewX(nSrcSizeX);
		if((nSrcSizeX + nOffsetX) > nDstSizeX)
			nSizeNewX -= (nSrcSizeX + nOffsetX) - nDstSizeX;
#ifdef Use_Memcpy
		memcpy(&pUcDstBuffer[nDstSizeX * (y + nOffsetY) +  nOffsetX], &pUcSrcBuffer[nSrcSizeX * y], nSizeNewX * sizeof(UCHAR));
#else
 		for(int x = 0; x < nSrcSizeX; x++)
 		{
 			if((y + nOffsetY) > nDstSizeY || (x + nOffsetX) > nDstSizeX)
 				continue;
 			pUcDstBuffer[nDstSizeX * (y + nOffsetY) + x + nOffsetX] = pUcSrcBuffer[nSrcSizeX * y + x];
 		}
#endif
	}
}

void CProc::CopyBuffer_OFOV(UCHAR* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, UCHAR* pUcSrcBuffer, int nOverlapSizeX, int nOverlapSizeY, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY)
{
	if (!pUcDstBuffer || !pUcSrcBuffer)
		return;
	int nRightFovCutStartPos = 0;
	int nBottomFovCutStartPos = 0;
	if (nOffsetX > 0)
	{
		nRightFovCutStartPos = nOffsetX - nOverlapSizeX;
		nOffsetX = nOverlapSizeX;
	}
	if (nOffsetY > 0)
	{
		nBottomFovCutStartPos = nOffsetY - nOverlapSizeY;
		nOffsetY = nOverlapSizeY;
		nSrcSizeY -= nBottomFovCutStartPos;
	}
#pragma omp parallel for /*collapse(2)*/
	for (int y = 0; y < nSrcSizeY; y++)
	{
		if ((y + nOffsetY) > nDstSizeY)
			continue;
		int nSizeNewX(nSrcSizeX);
		if ((nSrcSizeX + nOffsetX) > nDstSizeX)
			nSizeNewX -= (nSrcSizeX + nOffsetX) - nDstSizeX;
#ifdef Use_Memcpy
		memcpy(&pUcDstBuffer[nDstSizeX * (y + nOffsetY) + nOffsetX], &pUcSrcBuffer[nSrcSizeX * (y + nBottomFovCutStartPos) + nRightFovCutStartPos], nSizeNewX * sizeof(UCHAR));
#else
		for (int x = 0; x < nSrcSizeX; x++)
		{
			if ((y + nOffsetY) > nDstSizeY || (x + nOffsetX) > nDstSizeX)
				continue;
			pUcDstBuffer[nDstSizeX * (y + nOffsetY) + x + nOffsetX] = pUcSrcBuffer[nSrcSizeX * y + x];
		}
#endif
	}
}
void CProc::CopyBuffer( float* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, float* pUcSrcBuffer, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY )
{
	if(!pUcDstBuffer || !pUcSrcBuffer)
		return;

	#pragma omp parallel for /*collapse(2)*/
	for(int y = 0; y < nSrcSizeY;y++)
	{
		if((y + nOffsetY) > nDstSizeY)
			continue;

		int nSizeNewX(nSrcSizeX);
		if((nSrcSizeX + nOffsetX) > nDstSizeX)
			nSizeNewX -= (nSrcSizeX + nOffsetX) - nDstSizeX;
#ifdef Use_Memcpy
		memcpy(&pUcDstBuffer[nDstSizeX * (y + nOffsetY) +  nOffsetX], &pUcSrcBuffer[nSrcSizeX * y], nSizeNewX * sizeof(float));
#else
 		for(int x = 0; x < nSrcSizeX; x++)
 		{
 			if((y + nOffsetY) > nDstSizeY || (x + nOffsetX) > nDstSizeX)
 				continue;
 			pUcDstBuffer[nDstSizeX * (y + nOffsetY) + x + nOffsetX] = pUcSrcBuffer[nSrcSizeX * y + x];
 		}
#endif
	}
}

void CProc::CopyCropBuffer( UCHAR* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, int nOffsetX, int nOffsetY, UCHAR* pUcSrcBuffer, int nSrcSizeX, int nSrcSttX, int nSrcSttY, int nSrcEndX, int nSrcEndY )
{
	if(!pUcDstBuffer || !pUcSrcBuffer)
		return;

	#pragma omp parallel for /*collapse(2)*/
	for(int y = nSrcSttY; y < nSrcEndY;y++)
		for(int x = nSrcSttX; x < nSrcEndX; x++)
		{
			int nDstIdxX = ((x-nSrcSttX) + nOffsetX);
			int nDstIdxY = ((y-nSrcSttY) + nOffsetY);

			if( (nDstIdxY > nDstSizeY) || (nDstIdxX > nDstSizeX) )
				continue;
			else if (nDstIdxY < 0 || nDstIdxX < 0)
				continue;
			pUcDstBuffer[nDstSizeX * nDstIdxY + nDstIdxX] = pUcSrcBuffer[nSrcSizeX * y + x];
		}
}

void CProc::CopyCropBuffer( float* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, int nOffsetX, int nOffsetY, float* pUcSrcBuffer, int nSrcSizeX, int nSrcSttX, int nSrcSttY, int nSrcEndX, int nSrcEndY )
{
	if(!pUcDstBuffer || !pUcSrcBuffer)
		return;

	#pragma omp parallel for /*collapse(2)*/
	for(int y = nSrcSttY; y < nSrcEndY;y++)
		for(int x = nSrcSttX; x < nSrcEndX; x++)
		{
			int nDstIdxX = ((x-nSrcSttX) + nOffsetX);
			int nDstIdxY = ((y-nSrcSttY) + nOffsetY);

			if( (nDstIdxY > nDstSizeY) || (nDstIdxX > nDstSizeX) )
				continue;
			else if (nDstIdxX < 0 || nDstIdxY < 0)
				continue;
			pUcDstBuffer[nDstSizeX * nDstIdxY + nDstIdxX] = pUcSrcBuffer[nSrcSizeX * y + x];
		}
}

void CProc::CopyCropBuffer(UCHAR* &pUcDstBuffer, int nDstStX, int nDstStY, int nDstSizeX, int nDstSizeY, UCHAR* pUcSrcBuffer, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY)
{
	if(!pUcDstBuffer || !pUcSrcBuffer)
		return;

	int stX = 0;
	int stY = 0;
	int stOpX = 0;
	int stOpY = 0;

	if(nOffsetX == 0)
		stX = nDstStX;
	else
		stOpX = nDstStX;

	if(nOffsetY == 0)
		stY = nDstStY;
	else
		stOpY = nDstStY;

	int nPartWidthTemp = nDstSizeX;
	int nPartHeightTemp = nDstSizeY;

	if (nOffsetX + nSrcSizeX < nDstSizeX)  // 4080 + 2040 < 5000
		nPartWidthTemp = nOffsetX + nSrcSizeX;
	else
		nPartWidthTemp = nDstSizeX;

	if (nOffsetY + nSrcSizeY < nDstSizeY)  // 4080 + 2040 < 5000
		nPartHeightTemp = nOffsetY + nSrcSizeY;
	else
		nPartHeightTemp = nDstSizeY;


	for(int i = 0; i < nDstSizeY;i++)
	{
		for(int j = 0; j < nDstSizeX; j++)
		{
			if((i + nOffsetY) > nDstSizeY || (j + nOffsetX) > nDstSizeX)
				continue;

// 			if(i + nOffsetY > nPartHeightTemp || j + nOffsetX > nPartWidthTemp - stX)
// 				continue;

			pUcDstBuffer[(nDstSizeX * (i + nOffsetY)) + (j + (nOffsetX - stOpX))] = pUcSrcBuffer[(nSrcSizeX * (i + stY)) + (j + stX)];
		}
	}
}

// 위 코드 완료 후 붙여넣기
void CProc::CopyCropBuffer( float* &pUcDstBuffer, int nDstStX, int nDstStY, int nDstSizeX, int nDstSizeY, float* pUcSrcBuffer, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY )
{
	if(!pUcDstBuffer || !pUcSrcBuffer)
		return;

	int stX = 0;
	int stY = 0;
	if(nOffsetX == 0)
		stX = nDstStX;

	if(nOffsetY == 0)
		stY = nDstStY;

	for(int i = 0; i < nDstSizeY;i++)
	{
		for(int j = 0; j < nDstSizeX; j++)
		{
// 			if((i + nOffsetY) > nDstSizeY || (j + nOffsetX) > nDstSizeX)
// 				continue;

			pUcDstBuffer[nDstSizeX * (i + nOffsetY) + j + nOffsetX] = pUcSrcBuffer[nSrcSizeX * (i + stY) + (j + stX)];
		}
	}
}
void CProc::CopyBuffer_Portion(float* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, float* pUcSrcBuffer, int nCutLeftSize, int nCutTopSize, 
	int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY, bool bLastMerge)
{
	if (!pUcDstBuffer || !pUcSrcBuffer)
		return;
	if (nDstSizeX <= 0 || nDstSizeY <= 0)
		return;

	int nSizeNewX = nSrcSizeX - nCutLeftSize;
	int nNewSizeY = nSrcSizeY - nCutTopSize;
	if (nSizeNewX <= 0 || nNewSizeY <= 0)
		return;

	if ((nSizeNewX + nOffsetX) > nDstSizeX)
		nSizeNewX -= (nSizeNewX + nOffsetX) - nDstSizeX;
	if (nSizeNewX <= 0)
		return;

#pragma omp parallel for /*collapse(2)*/
	for (int y = 0; y < nNewSizeY; y++)
	{
		// 기존 소스처럼 적용하면 마지막 메모리 때문에 문제발생됨 -LWJ ModifyInsp
		if (y + nOffsetY >= nDstSizeY)
			continue;
		if (y + nCutTopSize >= nSrcSizeY)
			continue;

		long long lSrc = nSrcSizeX * (y + nCutTopSize) + nCutLeftSize;
		long long lRst = nDstSizeX * (y + nOffsetY) + nOffsetX;
		if (lSrc < 0 || lRst < 0)
			continue;

		memcpy(&pUcDstBuffer[lRst], &pUcSrcBuffer[lSrc], nSizeNewX * sizeof(float));
	}
}

void CProc::CopyBuffer_Portion(UCHAR* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, UCHAR* pUcSrcBuffer, int nCutLeftSize, int nCutTopSize, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY)
{
	if (!pUcDstBuffer || !pUcSrcBuffer)
		return;
	if (nDstSizeX <= 0 || nDstSizeY <= 0)
		return;

	int nSizeNewX = nSrcSizeX - nCutLeftSize;
	int nNewSizeY = nSrcSizeY - nCutTopSize;
	if (nSizeNewX <= 0 || nNewSizeY <= 0)
		return;

	if ((nSizeNewX + nOffsetX) > nDstSizeX)
		nSizeNewX -= (nSizeNewX + nOffsetX) - nDstSizeX;
	if (nSizeNewX <= 0)
		return;
#pragma omp parallel for /*collapse(2)*/
	for (int y = 0; y < nNewSizeY; y++)
	{
		// 기존 소스처럼 적용하면 마지막 메모리 때문에 문제발생됨 -LWJ ModifyInsp
		if (y + nOffsetY >= nDstSizeY)
			continue;
		if (y + nCutTopSize >= nSrcSizeY)
			continue;

		long long lSrc = nSrcSizeX * (y + nCutTopSize) + nCutLeftSize;
		long long lRst = nDstSizeX * (y + nOffsetY) + nOffsetX;
		if (lSrc < 0 || lRst < 0)
			continue;

		memcpy(&pUcDstBuffer[lRst], &pUcSrcBuffer[lSrc], nSizeNewX * sizeof(UCHAR));
	}
}