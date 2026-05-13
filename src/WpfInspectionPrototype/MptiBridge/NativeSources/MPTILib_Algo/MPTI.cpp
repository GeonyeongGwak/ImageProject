// MPTILib_Algo.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "MPTI.h"

#include "alp_file.h"
#include "alp_alloc.h"
#include "alp_proc.h"
#include "Minidump/PemMinidump.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <io.h>
#include <math.h>

#include "InspParamDef.h"
#include "LeastSquare.h"

//#include "ippCore.h" 
#include "MemoryManager.h"
#include "ExtInspLogic.h"
#include "ProcMil_Pattern.h"
#include "PInsp_OCV.h"
//#ifdef _DEBUG
//
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

#define	_limit_value(val, _min, _max) (val < _min) ? _min : ((val > _max) ? _max : val)

CMPTI *g_pMPTI = NULL;



CMPTI::CMPTI(void)
{
	PemMinidump::Begin();
	m_LogLevel = 0;
	m_digitSizeX = 0;
	m_digitSizeY = 0;

	m_factorR = 1.f;
	m_factorG = 1.f;
	m_factorB = 1.f;
	m_factorBR = 1.f;
	m_factorBB = 1.f;

	m_fCompoBtmR = 0.f;
	m_fCompoBtmB = 0.f;

	m_pfZmapData = NULL;
	m_nZmapWidth = 0;
	m_nZmapHeight = 0;

	for (int i = 0; i < (int)eM2C_NUM; i++)
	{
		m_bufRawDataFov[i] = M_NULL;
		m_parrFovBuf[i] = NULL;
		m_parrPartBuf[i] = NULL;
	}

	for (int k = 0; k < 4; k++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_pArrPartBuf_Side[k][j] = NULL;
			m_bufRawDataFov_Side[k][j] = M_NULL;
		}
	}

	pUcAlgoImgLight = NULL;

	m_bUseImagePilLib = true;

	for (int i = 0; i < eM2C_NUM; i++)
	{
		m_nSizeX_parrFovBuf[i] = m_nSizeY_parrFovBuf[i] = 0;
	}
	m_nSizeXRawData = m_nSizeYRawData = 0;
	m_nCompositeLightMode = 0;

	DeletePttArray();
	DeletePotArray();
	DeletePstArray();
	
	Im::Factory(Im::PIL);

	m_milApp = nullptr;
	m_milSys = nullptr;

	m_InspMng = new InspManager;
	//m_InspMng->mpInsp = &m_InspAlgo;
	//m_InspMng->m_InspAlgo = m_InspAlgo;

	MPTI_SetRawDataMachineType(eRawDataNone);

	m_nRedLgtPos_2DImg = (int)eColor2DImg_Light_Top_R;
	m_nGreenLgtPos_2DImg = (int)eColor2DImg_Light_Top_G;
	m_nBlueLgtPos_2DImg = (int)eColor2DImg_Light_Top_B;
}

CMPTI::~CMPTI(void)
{
	for (int i = 0; i < (int)eM2C_NUM; i++)
	{
		if (m_parrFovBuf[i] != NULL)
		{
			delete m_parrFovBuf[i];
			m_parrFovBuf[i] = NULL;
		}

		if (m_bufRawDataFov[i] != M_NULL)
		{
			Im::Buf::Free(m_bufRawDataFov[i]);
			m_bufRawDataFov[i] = M_NULL;
		}
	}

	for (int j = 0; j < 4; j++)
	{
		for (int k = 0; k < 3; k++)
		{
			if (m_bufRawDataFov_Side[j][k] != M_NULL)
			{
				Im::Buf::Free(m_bufRawDataFov_Side[j][k]);
				m_bufRawDataFov_Side[j][k] = M_NULL;
			}
		}
	}

	DeletePttArray();
	DeletePotArray();
	DeletePstArray();
	m_InspAlgo.CloseDevice();
	//delete m_pWireBondingInsp;
	delete m_InspMng;
	

	//Im::Factory_Free();
	//Im::Blob::Free();


	PemMinidump::End();

}

int CMPTI::GetPttFileLoad_SPI(CString strPath, int* nWidth, int* nHeight, bool bCallSPI)
{
	CString pszFileName = strPath;
	CFile myFile;
	CFileException fileException;

	if (!myFile.Open(pszFileName, CFile::modeRead, &fileException))
	{
		return eMR_FAIL;
	}

	CArchive ar(&myFile, CArchive::load);

	UINT nRow = 0;
	UINT nColumn = 0;

	float fResol_x;
	float fResol_y;
	UINT nBit;
	UINT nGap_x;
	UINT nGap_y;
	UINT nUnit;

	ar >> nRow;
	ar >> nColumn;
	ar >> fResol_x;
	ar >> fResol_y;
	ar >> nBit;
	ar >> nGap_x;
	ar >> nGap_y;
	ar >> nUnit;

	bool rodd = FALSE;
	bool codd = FALSE;
	UINT nNumRow = 0;
	UINT nNumColumn = 0;
	int NumOfRangeY = 0;
	int NumOfRangeX = 0;
	int Increase = 1;

	int nPlusMargin = 0;

	if ((nRow % 2) != 0)
	{
		nNumRow = nRow - 1;
		rodd = TRUE;
	}
	else
		nNumRow = nRow;

	if ((nColumn % 2) != 0)
	{
		nNumColumn = nColumn - 1;
		codd = TRUE;
	}
	else
		nNumColumn = nColumn;

	if ((nNumColumn % 4) != 0)
	{
		nPlusMargin = nNumColumn % 4;
	}

	*nWidth = nNumColumn + nPlusMargin;
	*nHeight = nNumRow;

	NumOfRangeY = (int)nNumRow / 2;
	NumOfRangeX = (int)nNumColumn / 2;

	int	totalData = 4 * NumOfRangeX * NumOfRangeY;

	if (m_pfZmapData != NULL)
	{
		delete[] m_pfZmapData;
		m_pfZmapData = NULL;
	}

	m_pfZmapData = new float[(*nWidth) * (*nHeight)];
	m_nZmapWidth = *nWidth;
	m_nZmapHeight = *nHeight;

	int	xrange = nNumColumn;
	int bit = nBit;
	int count = 0;

	if (bit == 16)
	{
		short temp;

		for (int y = 0; y < m_nZmapHeight; y += Increase)
		{
			for (int x = 0; x < m_nZmapWidth; x += Increase)
			{
				if (x >= m_nZmapWidth - nPlusMargin)
				{
					m_pfZmapData[count] = (float)0;
				}
				else
				{
					ar >> temp;

					m_pfZmapData[count] = (float)temp;

					if (codd)
						if ((count % xrange) == 0)
							ar >> temp;
				}
				count++;
			}
		}


		if (rodd)
		{
			if (codd)
			{
				for (int i = 0; i < xrange + 1; i++)
					ar >> temp;
			}
			else
			{
				for (int i = 0; i < xrange; i++)
					ar >> temp;
			}
		}
	}

	else if (bit == 32)
	{
		float temp;
		for (int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase)
		{
			for (int x = -NumOfRangeX; x < NumOfRangeX; x += Increase)
			{
				ar >> temp;

				m_pfZmapData[count] = (float)temp;

				count++;

				if (codd)
					if ((count % xrange) == 0)
						ar >> temp;
			}
		}

		if (rodd)
		{
			if (codd)
			{
				for (int i = 0; i < xrange + 1; i++)
					ar >> temp;
			}
			else
			{
				for (int i = 0; i < xrange; i++)
					ar >> temp;
			}
		}
	}

	else if (bit == 64)
	{
		double temp;
		for (int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase)
		{
			for (int x = -NumOfRangeX; x < NumOfRangeX; x += Increase)
			{
				ar >> temp;

				m_pfZmapData[count] = (float)temp;

				count++;

				if (codd)
					if ((count % xrange) == 0)
						ar >> temp;
			}
		}

		if (rodd)
		{
			if (codd)
			{
				for (int i = 0; i < xrange + 1; i++)
					ar >> temp;
			}
			else
			{
				for (int i = 0; i < xrange; i++)
					ar >> temp;
			}
		}
	}
	else
	{
		short temp;
		for (int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase)
		{
			for (int x = -NumOfRangeX; x < NumOfRangeX; x += Increase)
			{
				ar >> temp;

				m_pfZmapData[count] = (float)temp;

				count++;

				if (codd)
					if ((count % xrange) == 0)
						ar >> temp;
			}
		}

		if (rodd)
		{
			if (codd)
			{
				for (int i = 0; i < xrange + 1; i++)
					ar >> temp;
			}
			else
			{
				for (int i = 0; i < xrange; i++)
					ar >> temp;
			}
		}
	}

	// 2D Data Load
	float fRow = 0;
	float fColumn = 0;
	float fSt_Col = 0;
	float fSt_Row = 0;
	float fShiftX = 0;
	float fShiftY = 0;
	float fWidth = 0;
	float fLength = 0;
	float fDEPTH = 0;

	ar >> fRow;
	ar >> fColumn;
	ar >> fSt_Col;
	ar >> fSt_Row;
	ar >> fShiftX;
	ar >> fShiftY;
	ar >> fWidth;
	ar >> fLength;
	ar >> fResol_x;
	ar >> fResol_y;
	ar >> fDEPTH;

	float fNumRow = 0;
	float fNumColumn = 0;

	if (((int)fRow % 2) != 0)
	{
		fNumRow = fRow - 1;
		rodd = TRUE;
	}

	if (((int)fColumn % 2) != 0)
	{
		fNumColumn = fColumn - 1;
		codd = TRUE;
	}

	for (int i = (int)eM2C_TB; i < (int)eM2C_TW; i++)
	{
		if (m_parrPartBuf[i] != NULL)
		{
			delete[] m_parrPartBuf[i];
			m_parrPartBuf[i] = NULL;
		}
		m_parrPartBuf[i] = new UCHAR[(*nWidth) * (*nHeight)];
	}

	xrange = (int)fNumColumn;
	float DEPTH = fDEPTH;

	float fitTR = m_factorR;
	float fitTG = m_factorG;
	float fitTB = m_factorB;
	float fitBR = m_factorBR;
	float fitBB = m_factorBB;
	float compoBtmR = m_fCompoBtmR;
	float compoBtmG = m_fCompoBtmG;
	float compoBtmB = m_fCompoBtmB;

	float fBGR = 0.4f;
	float fBGB = 0.8f;

	if (abs(DEPTH - 8.) < 0.1f)
	{
		int	nIndex = 0;
		UCHAR tempData;
		for (int j = 0; j < *nHeight; j++)
		{
			for (int i = 0; i < *nWidth; i++)
			{
				if (i >= *nWidth - nPlusMargin)
				{
					m_parrPartBuf[eM2C_TR][nIndex] = 0;
					m_parrPartBuf[eM2C_TG][nIndex] = 0;
					m_parrPartBuf[eM2C_TB][nIndex] = 0;
				}
				else
				{
					ar >> tempData;

					if (m_nCompositeLightMode == 0)
					{
						m_parrPartBuf[eM2C_TR][nIndex] = tempData;
						m_parrPartBuf[eM2C_TG][nIndex] = tempData;
						m_parrPartBuf[eM2C_TB][nIndex] = tempData;
					}
					else
					{
						m_parrPartBuf[eM2C_TR][nIndex] = (UCHAR)(tempData - ((m_parrPartBuf[eM2C_BR][nIndex] * fitBR) * compoBtmR)) / fitTR;
						m_parrPartBuf[eM2C_TG][nIndex] = (UCHAR)(tempData - (((m_parrPartBuf[eM2C_BR][nIndex] * fitBR * fBGR) + (m_parrPartBuf[eM2C_BB][nIndex] * fitBB * fBGB)) * compoBtmG)) / fitTG;
						m_parrPartBuf[eM2C_TB][nIndex] = (UCHAR)(tempData - ((m_parrPartBuf[eM2C_BB][nIndex] * fitBB) * compoBtmB)) / fitTB;
					}

					if (codd)
						if (((i + 1) % xrange) == 0)
							ar >> tempData;
				}
				nIndex++;
			}
		}
	}
	else if (abs(DEPTH - 16) < 0.1f)
	{
		int	nIndex = 0;
		short tempData;
		for (int j = 0; j < *nHeight; j++)
		{
			for (int i = 0; i < *nWidth; i++)
			{
				if (i >= *nWidth - nPlusMargin)
				{
					m_parrPartBuf[eM2C_TR][nIndex] = 0;
					m_parrPartBuf[eM2C_TG][nIndex] = 0;
					m_parrPartBuf[eM2C_TB][nIndex] = 0;
				}
				else
				{
					ar >> tempData;

					UCHAR red = tempData >> 8;
					UCHAR blue = (UCHAR)tempData;
					UINT green = UINT((float)red * 0.4f + (float)blue * 0.8f);

					if (green > 255) green = 255;

					if (m_nCompositeLightMode == 0)
					{
						m_parrPartBuf[eM2C_TR][nIndex] = red;
						m_parrPartBuf[eM2C_TG][nIndex] = green;
						m_parrPartBuf[eM2C_TB][nIndex] = blue;
					}
					else
					{
						m_parrPartBuf[eM2C_TR][nIndex] = (UCHAR)(tempData - ((m_parrPartBuf[eM2C_BR][nIndex] * fitBR) * compoBtmR)) / fitTR;
						m_parrPartBuf[eM2C_TG][nIndex] = (UCHAR)(tempData - (((m_parrPartBuf[eM2C_BR][nIndex] * fitBR * fBGR) + (m_parrPartBuf[eM2C_BB][nIndex] * fitBB * fBGB)) * compoBtmG)) / fitTG;
						m_parrPartBuf[eM2C_TB][nIndex] = (UCHAR)(tempData - ((m_parrPartBuf[eM2C_BB][nIndex] * fitBB) * compoBtmB)) / fitTB;
					}

					if (codd)
						if (((i + 1) % xrange) == 0)
							ar >> tempData;
				}
				nIndex++;
			}
		}
	}
	else if (abs(DEPTH - 32) < 0.1f)
	{
		int	nIndex = 0;
		UINT tempData;
		for (int j = 0; j < *nHeight; j++)
		{
			for (int i = 0; i < *nWidth; i++)
			{
				if (i >= *nWidth - nPlusMargin)
				{
					m_parrPartBuf[eM2C_TR][nIndex] = 0;
					m_parrPartBuf[eM2C_TG][nIndex] = 0;
					m_parrPartBuf[eM2C_TB][nIndex] = 0;
				}
				else
				{
					ar >> tempData;

					UCHAR red = tempData >> 16;
					UCHAR green = tempData >> 8;
					UCHAR blue = (UCHAR)tempData;

					if (m_nCompositeLightMode == 0 || bCallSPI == true)
					{
						m_parrPartBuf[eM2C_TR][nIndex] = red;
						m_parrPartBuf[eM2C_TG][nIndex] = green;
						m_parrPartBuf[eM2C_TB][nIndex] = blue;
						bCallSPI = false;
					}
					else
					{
						int nTemp = (int)((red - ((m_parrPartBuf[eM2C_BR][nIndex] * fitBR) * compoBtmR)) / fitTR);
						m_parrPartBuf[eM2C_TR][nIndex] = _limit_value(nTemp, 0, 255);

					    int nTempG = (int)((green - (((m_parrPartBuf[eM2C_BR][nIndex] * fitBR * compoBtmR * fBGR) + (m_parrPartBuf[eM2C_BB][nIndex] * fitBB * compoBtmB * fBGB)) * compoBtmG)) / fitTG);
						m_parrPartBuf[eM2C_TG][nIndex] = _limit_value(nTempG, 0, 255);

						int nTempB = (int)((blue - ((m_parrPartBuf[eM2C_BB][nIndex] * fitBB) * compoBtmB)) / fitTB);
						m_parrPartBuf[eM2C_TB][nIndex] = _limit_value(nTempB, 0, 255);
					}

					if (codd)
						if (((i + 1) % xrange) == 0)
							ar >> tempData;
				}
				nIndex++;
			}

		}

		if (rodd)
		{
			if (codd)
			{
				for (int i = 0; i < xrange + 1; i++)
					ar >> tempData;
			}
			else
			{
				for (int i = 0; i < xrange; i++)
					ar >> tempData;
			}
		}
	}
	else
	{
		AfxMessageBox(_T("DEPTH Error"));
		return eMR_FAIL;
	}

	ar.Close();
	myFile.Close();

	return eMR_SUCCESS;
}

int CMPTI::GetPotFileLoad_SPI(CString strPath, int* nWidth, int* nHeight, float* fPixelResX, float* fPixelResY)
{
	CString pszFileName = strPath;
	CFile myFile;
	CFileException fileException;

	if (!myFile.Open(pszFileName, CFile::modeRead, &fileException))
	{
		//AfxMessageBox(_T("Pot file open Error"));
		return eMR_FAIL;
	}

	CArchive ar(&myFile, CArchive::load);

	float fCol = 0;
	float fRow = 0;
	float fResX = 0;
	float fResY = 0;
	float fDepthBit = 0;

	int nRow, nCol;
	int nj = 0;
	int ni = 0;
	int nNumRow, nNumColumn;
	int nPlusMargin = 0;

	BOOL codd = FALSE;
	BOOL rodd = FALSE;

	ar >> fCol;
	ar >> fRow;
	ar >> fResX;
	ar >> fResY;
	ar >> fDepthBit;

	nRow = (int)fRow;
	nCol = (int)fCol;

	*fPixelResX = fResX;
	*fPixelResY = fResY;

	nNumRow = nRow;
	nNumColumn = nCol;

	if ((nRow % 2) != 0)
	{
		nNumRow = nRow - 1;
		rodd = TRUE;
	}
	else
		nNumRow = nRow;

	if ((nCol % 2) != 0)
	{
		nNumColumn = nCol - 1;
		codd = TRUE;
	}
	else
		nNumColumn = nCol;

	if ((nNumColumn % 4) != 0)
	{
		nPlusMargin = nNumColumn % 4;
	}

	*nWidth = nNumColumn + nPlusMargin;
	*nHeight = nNumRow;

	int nIndex = 0;
	UCHAR tempData = 0;


	int	xrange = nNumColumn;

	for (int i = 0; i < eM2C_NUM; i++)
	{
		if (i == eM2C_TR || i == eM2C_TG || i == eM2C_TB)
			continue;

		nIndex = 0;

		if (m_parrPartBuf[i] != NULL)
		{
			delete[] m_parrPartBuf[i];
			m_parrPartBuf[i] = NULL;
		}
		m_parrPartBuf[i] = new UCHAR[(*nHeight) * (*nWidth)];

		for (nj = 0; nj < nNumRow; nj++)
		{
			for (ni = 0; ni < *nWidth; ni++)
			{
				if (ni >= *nWidth - nPlusMargin)
				{
					m_parrPartBuf[i][nIndex] = 0;
				}
				else
				{
					ar >> tempData;
					m_parrPartBuf[i][nIndex] = tempData;

					if (codd)
						if (((ni + 1) % nNumColumn) == 0)
							ar >> tempData;
				}

				nIndex++;
			}
		}

		if (rodd)
		{
			if (codd)
			{
				for (int i = 0; i < xrange + 1; i++)
					ar >> tempData;
			}
			else
			{
				for (int i = 0; i < xrange; i++)
					ar >> tempData;
			}
		}
	}

	return eMR_SUCCESS;
}

int CMPTI::GetPstFileLoad_RawData(CString strPath, int* nWidth, int* nHeight)
{
	//CString pszFileName = strPath;
	CString pszFileName[4];
	CFile myFile[4];
	CFileException fileException;

	//CArchive ar( &myFile,CArchive::load );

	float fCol = 0;
	float fRow = 0;
	float fResX = 0;
	float fResY = 0;
	float fDepthBit = 0;

	int nRow, nCol;
	int nj = 0;
	int ni = 0;
	int nNumRow, nNumColumn;
	//int nPlusMargin = 0;

	BOOL codd = FALSE;
	BOOL rodd = FALSE;

	int nIndex = 0;
	UCHAR tempData = 0;
	CFileFind fn;

	for (int i = 0; i < 4; i++)	// Side Camera Index
	{
		pszFileName[i].Format(_T("%s@S%d.pst"), strPath, i + 1);
		if (fn.FindFile(pszFileName[i]) && myFile[i].Open(pszFileName[i], CFile::modeRead, &fileException))
		{
			CArchive ar(&myFile[i], CArchive::load);

			ar >> fCol;
			ar >> fRow;
			ar >> fResX;
			ar >> fResY;
			ar >> fDepthBit;

			nRow = (int)fRow;
			nCol = (int)fCol;

			*nWidth = nCol;
			*nHeight = nRow;

			nNumRow = nRow;
			nNumColumn = nCol;

			for (int j = 0; j < 3; j++)	// Channel R.G.B
			{
				nIndex = 0;

				if (m_pArrPartBuf_Side[i][j] != NULL)
				{
					delete[] m_pArrPartBuf_Side[i][j];
					m_pArrPartBuf_Side[i][j] = NULL;
				}
				m_pArrPartBuf_Side[i][j] = new UCHAR[nNumRow * nNumColumn];

				for (nj = 0; nj < nNumRow; nj++)
				{
					for (ni = 0; ni < nNumColumn; ni++)
					{
						ar >> tempData;
						m_pArrPartBuf_Side[i][j][nIndex] = tempData;

						nIndex++;
					}
				}
			}
		}
		else
		{
			for (int j = 0; j < 3; j++)	// Channel R.G.B
			{
				if (m_pArrPartBuf_Side[i][j] != NULL)
				{
					delete[] m_pArrPartBuf_Side[i][j];
					m_pArrPartBuf_Side[i][j] = NULL;
				}
			}
		}
	}

	return eMR_SUCCESS;
}

int CMPTI::GetPotFileLoad_RawData(CString strPath, int* nWidth, int* nHeight, float* fPixelResX, float* fPixelResY)
{
	CString pszFileName = strPath;
	CFile myFile;
	CFileException fileException;

	if (!myFile.Open(pszFileName, CFile::modeRead, &fileException))
	{
		//AfxMessageBox(_T("Pot file open Error"));
		return eMR_FAIL;
	}

	CArchive ar(&myFile, CArchive::load);

	float fCol = 0;
	float fRow = 0;
	float fResX = 0;
	float fResY = 0;
	float fDepthBit = 0;

	int nRow, nCol;
	int nj = 0;
	int ni = 0;
	int nNumRow, nNumColumn;
	//int nPlusMargin = 0;

	BOOL codd = FALSE;
	BOOL rodd = FALSE;

	ar >> fCol;
	ar >> fRow;
	ar >> fResX;
	ar >> fResY;
	ar >> fDepthBit;

	nRow = (int)fRow;
	nCol = (int)fCol;

	*nWidth = nCol;
	*nHeight = nRow;
	*fPixelResX = fResX;
	*fPixelResY = fResY;

	nNumRow = nRow;
	nNumColumn = nCol;

	int nIndex = 0;
	UCHAR tempData = 0;

	for (int i = 0; i < eM2C_NUM; i++)
	{
		if (i == eM2C_TR || i == eM2C_TG || i == eM2C_TB)
			continue;

		nIndex = 0;

		if (m_parrPartBuf[i] != NULL)
		{
			delete [] m_parrPartBuf[i];
			m_parrPartBuf[i] = NULL;
		}
		m_parrPartBuf[i] = new UCHAR[nNumRow * nNumColumn];

		for (nj = 0; nj < nNumRow; nj++)
		{
			for (ni = 0; ni < nNumColumn; ni++)
			{
				ar >> tempData;
				m_parrPartBuf[i][nIndex] = tempData;

				nIndex++;
			}
		}
	}

	// 	CString sFilePath;
	// 
	// 	int nSize = strPath.GetLength();
	// 	strPath.Mid(0, nSize-4);
	// 	strPath += _T("BB.tif");
	// 
	// 	CMInspManager *pMInspManager = m_pInspSys->m_pInsopMng;
	// 	if(!pMInspManager)
	// 		return eMR_FAIL;
	// 	if(!pMInspManager->GetProcMil())
	// 		return eMR_FAIL;
	// 	CProcMil *procMil = pMInspManager->GetProcMil();
	// 
	// 	procMil->SaveImage(m_parrPartBuf[eM2C_BR], nNumColumn, nNumRow, 1, strPath);
	// 	strPath.Mid(0, nSize-6);
	// 	strPath += _T("BR.tif");
	// 	procMil->SaveImage(m_parrPartBuf[], nNumColumn, nNumRow, 1, strPath);
	// 	strPath.Mid(0, nSize-6);
	// 	strPath += _T("MB.tif");
	// 	procMil->SaveImage(m_parrPartBuf[], nNumColumn, nNumRow, 1, strPath);
	// 	strPath.Mid(0, nSize-6);
	// 	strPath += _T("MR.tif");
	// 	procMil->SaveImage(m_parrPartBuf[], nNumColumn, nNumRow, 1, strPath);
	// 	strPath.Mid(0, nSize-6);
	// 	strPath += _T("TR.tif");
	// 	procMil->SaveImage(m_parrPartBuf[], nNumColumn, nNumRow, 1, strPath);

	return eMR_SUCCESS;
}

int CMPTI::GetPttFileLoad_RawData(CString strPath, int* nWidth, int* nHeight)
{
	CString pszFileName = strPath;
	CFile myFile;
	CFileException fileException;

	if (!myFile.Open(pszFileName, CFile::modeRead, &fileException))
	{
		//AfxMessageBox(_T("Ptt file open Error"));
		return eMR_FAIL;
	}

	CArchive ar(&myFile, CArchive::load);

	//3D Data File ¾È¾¸ ±×·¯³ª CArchive ar ·Î•û¿Ã·Á¸é ¼ø¼­»ó ¸ðµÎ Â÷·Ê·Î •û¿Í¾ßÇÔ
	UINT nRow = 0;
	UINT nColumn = 0;

	float fResol_x;
	float fResol_y;
	UINT nBit;
	UINT nGap_x;
	UINT nGap_y;
	UINT nUnit;

	ar >> nRow;
	ar >> nColumn;
	ar >> fResol_x;
	ar >> fResol_y;
	ar >> nBit;
	ar >> nGap_x;
	ar >> nGap_y;
	ar >> nUnit;

	bool rodd = FALSE;
	bool codd = FALSE;
	UINT nNumRow = 0;
	UINT nNumColumn = 0;
	int NumOfRangeY = 0;
	int NumOfRangeX = 0;
	int Increase = 1;

	// 	if( (nRow % 2) != 0 )
	// 	{
	// 		nNumRow = nRow - 1;
	// 		rodd = TRUE;
	// 	}
	// 	else
	// 		nNumRow = nRow;
	// 
	// 	if( (nColumn % 2) != 0 )
	// 	{
	// 		nNumColumn = nColumn - 1;
	// 		codd = TRUE;
	// 	}
	// 	else
	//		nNumColumn = nColumn;

	// 	if((nNumColumn%4) != 0)
	// 	{
	// 		nPlusMargin = nNumColumn%4;
	// 	}

	// 	*nWidth = nNumColumn + nPlusMargin;
	// 	*nHeight = nNumRow;

	*nWidth = (int)nColumn;
	*nHeight = (int)nRow;

	NumOfRangeY = (int)nNumRow / 2;
	NumOfRangeX = (int)nNumColumn / 2;

	int	totalData = 4 * NumOfRangeX * NumOfRangeY;

	if (m_pfZmapData != NULL)
	{
		delete [] m_pfZmapData;
		m_pfZmapData = NULL;
	}

	m_pfZmapData = new float[(*nWidth) * (*nHeight)];
	m_nZmapWidth = *nWidth;
	m_nZmapHeight = *nHeight;

	int	xrange = nNumColumn;
	int bit = nBit;
	int count = 0;

	if (bit == 16)
	{
		short temp;
		for (int y = 0; y < (int)nRow; y++)
		{
			for (int x = 0; x < (int)nColumn; x++)
			{
				ar >> temp;

				m_pfZmapData[count] = (float)temp;

				count++;

				// 				if( codd )
				// 					if( (count % xrange) == 0 )
				// 						ar >> temp;
			}
		}
	}
	else if (bit == 32)
	{
		float temp;
		for (int y = 0; y < (int)nRow; y++)
		{
			for (int x = 0; x < (int)nColumn; x++)
			{
				ar >> temp;

				m_pfZmapData[count] = temp;

				count++;
			}
		}
	}
	else if (bit == 64)
	{
		double temp;
		for (int y = 0; y < (int)nRow; y++)
		{
			for (int x = 0; x < (int)nColumn; x++)
			{
				ar >> temp;

				m_pfZmapData[count] = (float)temp;

				count++;
			}
		}
	}
	else
	{
		short temp;
		for (int y = 0; y < (int)nRow; y++)
		{
			for (int x = 0; x < (int)nColumn; x++)
			{
				ar >> temp;

				m_pfZmapData[count] = (float)temp;

				count++;

				// 				if( codd )
				// 					if( (count % xrange) == 0 )
				// 						ar >> temp;
			}
		}
	}

	// 2D Data Load
	float fRow = 0;
	float fColumn = 0;
	float fSt_Col = 0;
	float fSt_Row = 0;
	float fShiftX = 0;
	float fShiftY = 0;
	float fWidth = 0;
	float fLength = 0;
	float fDEPTH = 0;

	ar >> fRow;
	ar >> fColumn;
	ar >> fSt_Col;
	ar >> fSt_Row;
	ar >> fShiftX;
	ar >> fShiftY;
	ar >> fWidth;
	ar >> fLength;
	ar >> fResol_x;
	ar >> fResol_y;
	ar >> fDEPTH;

	float fNumRow = 0;
	float fNumColumn = 0;

	if (((int)fRow % 2) != 0)
	{
		fNumRow = fRow - 1;
		rodd = TRUE;
	}

	if (((int)fColumn % 2) != 0)
	{
		fNumColumn = fColumn - 1;
		codd = TRUE;
	}

	for (int i = (int)eM2C_TB; i < (int)eM2C_TW; i++)
	{
		if (m_parrPartBuf[i] != NULL)
		{
			delete [] m_parrPartBuf[i];
			m_parrPartBuf[i] = NULL;
		}
		m_parrPartBuf[i] = new UCHAR[(*nWidth) * (*nHeight)];
	}

	xrange = (int)fNumColumn;
	float DEPTH = fDEPTH;

	float fitTR = m_factorR;
	float fitTG = m_factorG;
	float fitTB = m_factorB;
	float fitBR = m_factorBR;
	float fitBB = m_factorBB;
	float compoBtmR = m_fCompoBtmR;
	float compoBtmG = m_fCompoBtmG;
	float compoBtmB = m_fCompoBtmB;

	float fBGR = 0.4f;
	float fBGB = 0.8f;

	if (abs(DEPTH - 8) < 0.1f)
	{
		int	nIndex = 0;
		UCHAR tempData;
		for (int j = 0; j < *nHeight; j++)
		{
			for (int i = 0; i < *nWidth; i++)
			{
				ar >> tempData;

				if (m_nCompositeLightMode == 0)
				{
					m_parrPartBuf[eM2C_TR][nIndex] = tempData / fitTR;
					m_parrPartBuf[eM2C_TG][nIndex] = tempData / fitTG;
					m_parrPartBuf[eM2C_TB][nIndex] = tempData / fitTB;
				}
				else
				{
					m_parrPartBuf[eM2C_TR][nIndex] = (UCHAR)(tempData - ((m_parrPartBuf[eM2C_BR][nIndex] * fitBR) * compoBtmR)) / fitTR;
					m_parrPartBuf[eM2C_TG][nIndex] = (UCHAR)(tempData - (((m_parrPartBuf[eM2C_BR][nIndex] * fitBR * fBGR) + (m_parrPartBuf[eM2C_BB][nIndex] * fitBB * fBGB)) * compoBtmG)) / fitTG;
					m_parrPartBuf[eM2C_TB][nIndex] = (UCHAR)(tempData - ((m_parrPartBuf[eM2C_BB][nIndex] * fitBB) * compoBtmB)) / fitTB;
				}

				nIndex++;
			}
		}
	}
	else if (abs(DEPTH - 16) < 0.1f)
	{
		int	nIndex = 0;
		short tempData;
		for (int j = 0; j < *nHeight; j++)
		{
			for (int i = 0; i < *nWidth; i++)
			{
				ar >> tempData;

				UCHAR red = tempData >> 8;
				UCHAR blue = (UCHAR)tempData;
				UINT green = UINT((float)red * 0.4f + (float)blue * 0.8f);

				if (green > 255) green = 255;

				if (m_nCompositeLightMode == 0)
				{
					m_parrPartBuf[eM2C_TR][nIndex] = red / fitTR;
					m_parrPartBuf[eM2C_TG][nIndex] = green / fitTG;
					m_parrPartBuf[eM2C_TB][nIndex] = blue / fitTB;
				}
				else
				{
					m_parrPartBuf[eM2C_TR][nIndex] = (UCHAR)(red - ((m_parrPartBuf[eM2C_BR][nIndex] * fitBR) * compoBtmR)) / fitTR;
					m_parrPartBuf[eM2C_TG][nIndex] = (UCHAR)(green - (((m_parrPartBuf[eM2C_BR][nIndex] * fitBR * fBGR) + (m_parrPartBuf[eM2C_BB][nIndex] * fitBB * fBGB)) * compoBtmG)) / fitTG;
					m_parrPartBuf[eM2C_TB][nIndex] = (UCHAR)(blue - ((m_parrPartBuf[eM2C_BB][nIndex] * fitBB) * compoBtmB)) / fitTB;
				}

				nIndex++;
			}
		}
	}
	else if (abs(DEPTH - 32) < 0.1f)
	{
		int	nIndex = 0;
		UINT tempData;
		for (int j = 0; j < *nHeight; j++)
		{
			for (int i = 0; i < *nWidth; i++)
			{
				ar >> tempData;

				UCHAR red = tempData >> 16;
				UCHAR green = tempData >> 8;
				UCHAR blue = (UCHAR)tempData;

				if (m_nCompositeLightMode == 0)
				{
					m_parrPartBuf[eM2C_TR][nIndex] = red / fitTR;
					m_parrPartBuf[eM2C_TG][nIndex] = green / fitTG;
					m_parrPartBuf[eM2C_TB][nIndex] = blue / fitTB;
				}
				else
				{
					m_parrPartBuf[eM2C_TR][nIndex] = (UCHAR)(red - ((m_parrPartBuf[eM2C_BR][nIndex] * fitBR) * compoBtmR)) / fitTR;
					m_parrPartBuf[eM2C_TG][nIndex] = (UCHAR)(green - (((m_parrPartBuf[eM2C_BR][nIndex] * fitBR * fBGR) + (m_parrPartBuf[eM2C_BB][nIndex] * fitBB * fBGB)) * compoBtmG)) / fitTG;
					m_parrPartBuf[eM2C_TB][nIndex] = (UCHAR)(blue - ((m_parrPartBuf[eM2C_BB][nIndex] * fitBB) * compoBtmB)) / fitTB;
				}

				nIndex++;
			}
		}
	}
	else
	{
		AfxMessageBox(_T("DEPTH Error"));
		return eMR_FAIL;
	}

	cv::Mat tr = cv::Mat(nRow, nColumn, CV_8UC1, m_parrPartBuf[eM2C_TR]);
	cv::Mat tg = cv::Mat(nRow, nColumn, CV_8UC1, m_parrPartBuf[eM2C_TG]);
	cv::Mat tb = cv::Mat(nRow, nColumn, CV_8UC1, m_parrPartBuf[eM2C_TB]);
	cv::imwrite("D:\\ColorImage\\TR_1.bmp", tr);
	cv::imwrite("D:\\ColorImage\\TG_1.bmp", tg);
	cv::imwrite("D:\\ColorImage\\TB_1.bmp", tb);
	ar.Close();
	myFile.Close();

	return eMR_SUCCESS;
}

int CMPTI::SetRawDataFovInfo(CString filePathPtt, CString filePathPot, int nX, int nY, bool bSeparatedPart, int* nWidth, int* nHeight, int nFovSizeX, int nFovSizeY )
{
	int nFovX, nFovY;
	int nRawSizeX, nRawSizeY;
	float fPixelResX, fPixelResY;

	int nRst = eMR_FAIL;
	nRst = GetPotFileLoad_RawData(filePathPot, &nRawSizeX, &nRawSizeY, &fPixelResX, &fPixelResY);
	//nRst = GetPotFileLoad_SPI(filePathPot, &nRawSizeX, &nRawSizeY, &fPixelResX, &fPixelResY);

	if (nRst != eMR_SUCCESS)
		return nRst;

	nRst = GetPttFileLoad_RawData(filePathPtt, &nRawSizeX, &nRawSizeY);
	//nRst = GetPttFileLoad_SPI(filePathPtt, &nRawSizeX, &nRawSizeY, false);

	if (nRst != eMR_SUCCESS)
		return nRst;

	*nWidth = m_nSizeXRawData = nRawSizeX;
	*nHeight = m_nSizeYRawData = nRawSizeY;

	nFovX = nFovSizeX;
	nFovY = nFovSizeY;
// 
// 	if (bSeparatedPart)
// 	{
// 		nFovX = m_nSizeXRawData;
// 		nFovY = m_nSizeYRawData;
// 	}

	for (int i = 0; i < (int)eM2C_NUM; i++)
	{
		if (m_bufRawDataFov[i] != M_NULL)
		{
			Im::Buf::Free(m_bufRawDataFov[i]);
			m_bufRawDataFov[i] = M_NULL;
		}

		Im::Buf::Alloc2d(*m_milSys, nFovX, nFovY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &m_bufRawDataFov[i]);
		Im::Buf::Clear(m_bufRawDataFov[i], 0);
		Im::Buf::Put2d(m_bufRawDataFov[i], nX, nY, nRawSizeX, nRawSizeY, m_parrPartBuf[i]);

	}


	CString strPstFilePath = filePathPot;
	int nLeng = strPstFilePath.GetLength();

	strPstFilePath.Delete(nLeng - 4, 4);

	nRst = GetPstFileLoad_RawData(strPstFilePath, &nRawSizeX, &nRawSizeY);

	for (int j = 0; j < 4; j++)
	{
		for (int k = 0; k < 3; k++)
		{
			if (m_bufRawDataFov_Side[j][k] != M_NULL)
			{
				Im::Buf::Free(m_bufRawDataFov_Side[j][k]);
				m_bufRawDataFov_Side[j][k] = M_NULL;
			}

			Im::Buf::Alloc2d(*m_milSys, nFovX, nFovY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &m_bufRawDataFov_Side[j][k]);
			Im::Buf::Clear(m_bufRawDataFov_Side[j][k], 0);

			if (m_pArrPartBuf_Side[j][k] != NULL)
				Im::Buf::Put2d(m_bufRawDataFov_Side[j][k], nX, nY, nRawSizeX, nRawSizeY, m_pArrPartBuf_Side[j][k]);
		}
	}

	SetResolution(nFovX, nFovY, fPixelResX, fPixelResY);

	return eMR_SUCCESS;
}
void CMPTI::SetMachineOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax)
{
	SetFootOption(nFootPadTopWGrayLevel, nFootPatternMatchScore, bMatch2D, bMatch3D, nSobelmin, nSobelmax);
}

UCHAR* CMPTI::GetPttFileChannel(int nChannel)
{
	if (nChannel == 0)
		return m_parrPartBuf[eM2C_TR];
	else if (nChannel == 1)
		return m_parrPartBuf[eM2C_TG];
	else if (nChannel == 2)
		return m_parrPartBuf[eM2C_TB];
	else if (nChannel == 3)
		return m_parrPartBuf[eM2C_BR];
	else if (nChannel == 4)
		return m_parrPartBuf[eM2C_BB];
	else if (nChannel == 5)
		return m_parrPartBuf[eM2C_MR];
	else if (nChannel == 6)
		return m_parrPartBuf[eM2C_MB];
	else if (nChannel == 7)
		return m_parrPartBuf[eM2C_TW];
}

UCHAR* CMPTI::GetRawDataFovBuffer(int nChannel)
{
	bool bNeedNewBuffer = false;

	int sizeX = m_digitSizeX;
	int sizeY = m_digitSizeY;
	if (m_nSizeXRawData > m_digitSizeX || m_nSizeYRawData > m_digitSizeY)
	{
		sizeX = m_nSizeXRawData;
		sizeY = m_nSizeYRawData;
	}

	for (int i = 0; i < eM2C_NUM; i++)
	{
		if (nChannel == i)
		{
			if (m_parrFovBuf != NULL && m_nSizeX_parrFovBuf[i] * m_nSizeY_parrFovBuf[i] != sizeX * sizeY)
				bNeedNewBuffer = true;

			m_nSizeX_parrFovBuf[i] = sizeX;
			m_nSizeY_parrFovBuf[i] = sizeY;

			if (bNeedNewBuffer == true)
			{
				if (m_parrFovBuf[i] != NULL)
				{
					delete[] m_parrFovBuf[i];
					m_parrFovBuf[i] = NULL;
				}
				m_parrFovBuf[i] = new UCHAR[sizeX * sizeY];
			}

			Im::Buf::Get2d(m_bufRawDataFov[nChannel], 0, 0, sizeX, sizeY, m_parrFovBuf[i]);

			return m_parrFovBuf[i];
		}
	}
}

UCHAR* CMPTI::GetRawDataFovBuffer_Side(int nChannel, int nCamIdx)
{
	if (m_pArrPartBuf_Side[nCamIdx][nChannel] != NULL)
	{
		delete[] m_pArrPartBuf_Side[nCamIdx][nChannel];
		m_pArrPartBuf_Side[nCamIdx][nChannel] = NULL;
	}

	m_pArrPartBuf_Side[nCamIdx][nChannel] = new UCHAR[m_digitSizeX * m_digitSizeY];
	Im::Buf::Get2d(m_bufRawDataFov_Side[nCamIdx][nChannel], 0, 0, m_digitSizeX, m_digitSizeY, m_pArrPartBuf_Side[nCamIdx][nChannel]);

	return m_pArrPartBuf_Side[nCamIdx][nChannel];

}

float* CMPTI::GetRawDataZmap()
{
	return m_pfZmapData;
}

int CMPTI::GetRawDataZmapHeight()
{
	return m_nZmapHeight;
}

int CMPTI::GetRawDataZmapWidth()
{
	return m_nZmapWidth;
}

int CMPTI::SetColorFactor(float factorR, float factorG, float factorB, float factorBR, float factorBB)
{
	// m_factorR = factorR;
	// m_factorG = factorG;
	// m_factorB = factorB;
	m_factorR = 1.f;
	m_factorG = 1.f;
	m_factorB = 1.f;
	m_factorBR = factorBR;
	m_factorBB = factorBB;
	PInspAlgoWrapper* wrapper = g_pMPTI->GetPInspWrapper();
	wrapper->SetFittingValue(m_factorR, m_factorG, m_factorB, m_factorBR, m_factorBB);
	return eMR_SUCCESS;
}

void CMPTI::SetCompositeLightMode(int LightMode, float compoBtmR, float compoBtmG, float compoBtmB)
{
	PInspAlgoWrapper* wrapper = g_pMPTI->GetPInspWrapper();
	m_nCompositeLightMode = LightMode;

	m_fCompoBtmR = compoBtmR;
	m_fCompoBtmG = compoBtmG;
	m_fCompoBtmB = compoBtmB;
	wrapper->SetCompositeLightMode(LightMode, compoBtmR, compoBtmG, compoBtmB);
}

int CMPTI::DeletePttArray()
{
	for (int i = (int)eM2C_TB; i < (int)eM2C_TW; i++)
	{
		if (m_parrPartBuf[i] != NULL)
		{
			delete[] m_parrPartBuf[i];
			m_parrPartBuf[i] = NULL;
		}
	}

	if (m_pfZmapData != NULL)
	{
		delete[] m_pfZmapData;
		m_pfZmapData = NULL;
	}

	m_nZmapWidth = 0;
	m_nZmapHeight = 0;

	return eMR_SUCCESS;
}

int CMPTI::DeletePotArray()
{
	for (int i = 0; i < eM2C_NUM; i++)
	{
		if (i == eM2C_TR || i == eM2C_TG || i == eM2C_TB)
			continue;

		if (m_parrPartBuf[i] != NULL)
		{
			delete[] m_parrPartBuf[i];
			m_parrPartBuf[i] = NULL;
		}
	}
	return eMR_SUCCESS;
}

int CMPTI::DeletePstArray()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (m_pArrPartBuf_Side[i][j] != NULL)
			{
				delete[] m_pArrPartBuf_Side[i][j];
				m_pArrPartBuf_Side[i][j] = NULL;
			}
		}
	}

	return eMR_SUCCESS;
}
#include <fstream>
void CMPTI::AddLog(CString sMessge)
{
	//AddLog(sMessge.GetBuffer(0));
	AddLog_Dev(sMessge);
// 	CTime tm(CTime::GetCurrentTime());
// 
// 	CString sTime;
// 	sTime.Format(_T("D:\\Eagle3D_data\\Log\\AlgoTool_Part_%s.csv"),  tm.Format(_T("%Y-%m-%d")));
// 
// 	std::wfstream file(sTime, std::ios::app);
// 	if (file.is_open())
// 	{
// 		file << (LPCTSTR)(tm.Format(_T("%Y-%m-%d %H:%M:%S"))) << _T(",") << (LPCTSTR)(sMessge) << std::endl;
// 		file.close();
// 	}
}

void CMPTI::AddLog_Dev(CString sMessge, int nLogLv)
{
	if (CheckLogLv(nLogLv) == false)
		return;

	SYSTEMTIME Systime;
	GetLocalTime(&Systime);
	
	CString sSystemTime;
	sSystemTime.Format(_T("%04d-%02d-%02d"), Systime.wYear, Systime.wMonth, Systime.wDay);
	CString sTime;
	sTime.Format(_T("D:\\Eagle3D_data\\Log\\AlgoTool_%d_%s.csv"), ext::env::nTool_id , sSystemTime);

	std::wfstream file(sTime, std::ios::app);
	if (file.is_open())
	{
		sSystemTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d:%03d"), Systime.wYear, Systime.wMonth, Systime.wDay, Systime.wHour, Systime.wMinute, Systime.wSecond, Systime.wMilliseconds);
		file << (LPCTSTR)sSystemTime << _T(",") << (LPCTSTR)(sMessge) << std::endl;
		file.close();
	}
}

void CMPTI::AddLog_OCR(wchar_t  szMessage[])
{
	/*if (m_cbAddLog_OCR != NULL)
	{
		BSTR	bstrMessage = SysAllocString(szMessage);
		m_cbAddLog_OCR(bstrMessage);
		SysFreeString(bstrMessage);
	}*/
}

void CMPTI::AddLog_OCR(CString sMessge)
{
	AddLog_OCR(sMessge.GetBuffer(0));
}

bool CMPTI::CheckLogLv(int nLogLv)
{
	if (nLogLv == 0)
		return true;

	if (GetLogLv() < nLogLv)
		return false;
}

int CMPTI::nCalcWidthStep(bool bUseWidthStep, int OrgImgWidth)       //NYJ 2020/10/19
{
	if (bUseWidthStep)
	{
		int nWidthStep = OrgImgWidth;     //widthStep : bytes per a line
		int nSizeX = OrgImgWidth % 4;
		if (nSizeX != 0)
			nWidthStep += 4 - nSizeX;
		return nWidthStep;   //4 바이트의 배수가 되도록 영상 너비 정렬 시, width 대신 widthStep 값 사용
	}
	else
		return OrgImgWidth;
}
CString CMPTI::GetWireDebugDataFullPath(int nfootKind, CString csAddSubPath, CString csAddName, int nFormat)
{
	CString csWireDebugDataFullPath = _T("");
	CString csRootPath = _T("D:\\testimage\\workimage\\wire");
	CString csMainPath = _T("WIREINSP_");
	CString csFormat = _T("bmp");

	CreateDirectory(csRootPath, NULL);

	if (csAddSubPath.IsEmpty() != TRUE)
	{
		csRootPath = csRootPath + _T("\\") + csAddSubPath;
		CreateDirectory(csRootPath, NULL);
	}

	CString csInspType = _T("UNKNOWN_");

	if (nfootKind == (int)m_eFootBin::m_eFootBin_Pad)
		csInspType = _T("Pad_");
	else if (nfootKind == (int)m_eFootBin::m_eFootBin_Foot)
		csInspType = _T("Foot_");
	else if (nfootKind == (int)m_eFootBin::m_eFootBin_Wing)
		csInspType = _T("Wing_");
	else if (nfootKind == (int)m_eFootBin::m_eFootBin_Wedge)
		csInspType = _T("Wedge_");

	csMainPath = _T("WIREINSP_") + csInspType;

	if (nFormat == 1)
		csFormat = _T("ptr");	//3D 

	CString csFullName = _T("");

	csFullName.Format(_T("%s_%s"), csMainPath, csAddName);

	csWireDebugDataFullPath.Format(_T("%s\\%s.%s"), csRootPath, csFullName, csFormat);

	return csWireDebugDataFullPath;
}

void CMPTI::SetResolution(int fovWidth, int fovLength, double resolX, double resolY)
{

	m_InspAlgo.InitDevice(m_milApp, m_milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	m_pWireBondingInsp->InitDevice(m_milApp, m_milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	m_InspMng->GetPatternInsp()->InitDevice(m_milApp, m_milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	//m_InspMng->InitDevice(m_milApp, m_milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
	m_InspMng->SetResolution(fovWidth, fovLength, resolX, resolY);

}
void CMPTI::SetFootOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax)
{
	m_InspMng->SetFootOption(nFootPadTopWGrayLevel, nFootPatternMatchScore, bMatch2D, bMatch3D, nSobelmin, nSobelmax);
}
void CMPTI::SetExtInspInit(int fovWidth, int fovLength, double resolX, double resolY)
{
	m_InspMng->GetPatternInsp()->InitDevice(m_milApp, m_milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());


	//((CPInsp_Pattern *)m_pPatternInsp)->InitDevice(m_milApp, m_milSys, fovWidth, fovLength, resolX, resolY, g_pMPTI->isUseImagePilLib());
}
SIZE CMPTI::GetSizeBoard()
{
	//double dBoardSizeX;
	//double dBoardSizeY;

	//m_pMCoordi->GetBoardSize(&dBoardSizeX, &dBoardSizeY);

	SIZE szBoardSize;
	szBoardSize.cx = m_sizeBoardX;
	szBoardSize.cy = m_sizeBoardY;

	return szBoardSize;
}

int CMPTI::SetSizeBoard(double sizeX, double sizeY)
{
	m_sizeBoardX = sizeX;
	m_sizeBoardY = sizeY;
	//m_pMCoordi->SetSizeBoard(sizeX, sizeY);

	//if (m_pMConv)
	//	m_pMConv->SetPcbLength((float)sizeX);

	return 0;
}
int MPTI_Init()
{
	//AfxSetAllocStop(519);
	//AfxSetAllocStop(515);
	//BCD_Open(0, 0, TRUE, 1);

	if (g_pMPTI)
	{
		delete g_pMPTI;
		g_pMPTI = NULL;
	}
	g_pMPTI = new CMPTI;

	g_pMManager = new MemoryManager;

	g_pMPTI->m_milApp = (Im::PIL_ID*)g_pMPTI;
	g_pMPTI->m_milSys = (Im::PIL_ID*)g_pMPTI;

	if (g_pMPTI == NULL)	return 0;

	return 1;

}

 
 int MPTI_Dispose()
 {
 	BCD_Close();
 	if (g_pMPTI)
 	{
 		delete g_pMPTI;
 		g_pMPTI = NULL;
 	}
 
 	delete g_pMManager;
 	g_pMManager = NULL;
 
 	return 0;
 }



int MPTI_SetInspParam(InspPartInfo* boardInfo, InspPartParam *pParamArray, int nParamArraySize)
{
	if (g_pMPTI == NULL)
		return 0;

	//get Shared - Sturct 연결 
	ext::CopyParamToExt(boardInfo, pParamArray, nParamArraySize);


	boardInfo->partZmapData.zmapSizeX = g_pMPTI->m_nZmapWidth;
	boardInfo->partZmapData.zmapSizeY = g_pMPTI->m_nZmapHeight;
	boardInfo->partZmapData.data = g_pMPTI->m_pfZmapData;

	boardInfo->partImgBuf.imgTop_R = g_pMPTI->m_parrPartBuf[eM2C_TR];
	boardInfo->partImgBuf.imgTop_G = g_pMPTI->m_parrPartBuf[eM2C_TG];
	boardInfo->partImgBuf.imgTop_B = g_pMPTI->m_parrPartBuf[eM2C_TB];

	boardInfo->partImgBuf.imgTop_W = g_pMPTI->m_parrPartBuf[eM2C_TW];
	boardInfo->partImgBuf.imgMiddle_R = g_pMPTI->m_parrPartBuf[eM2C_MR];
	boardInfo->partImgBuf.imgMiddle_B = g_pMPTI->m_parrPartBuf[eM2C_MB];
	boardInfo->partImgBuf.imgBottom_R = g_pMPTI->m_parrPartBuf[eM2C_BR];
	boardInfo->partImgBuf.imgBottom_B = g_pMPTI->m_parrPartBuf[eM2C_BB];

	boardInfo->partImgBuf.nImageSizeX = g_pMPTI->m_nSizeXRawData;
	boardInfo->partImgBuf.nImageSizeY = g_pMPTI->m_nSizeYRawData;

	g_pMPTI->m_InspMng->SetInspParam(boardInfo, pParamArray, nParamArraySize);
	g_pMPTI->m_InspMng->SortingParamater(pParamArray, nParamArraySize);

	return 1;
}
int MPTI_InspProc()
{
	if (g_pMPTI == NULL)
		return 0;

	return g_pMPTI->m_InspMng->InspProc();
}
int MPTI_GetInspectionResult(InspectionResult* retResult)
{
	if (g_pMPTI == NULL)	
		return eMR_FAIL;

	return g_pMPTI->m_InspMng->GetInspectionResult(retResult);
}

int MPTI_GetPttFileLoad(BSTR filePath, int* nWidth, int* nHeight, bool bCallSPI)
{
	if (g_pMPTI == NULL)	return eMR_NULL;

	// return g_pMPTI->GetPttFileLoad_SPI(filePath, nWidth, nHeight, bCallSPI);
	return g_pMPTI->GetPttFileLoad_RawData(filePath, nWidth, nHeight);
}

BOOL MPTI_ColorMapSetValue(lightData* pRedLightData)
{
	BOOL bRet = FALSE;
	for (int n = 0; n < 3; n++)
	{
		PIAL::PInspAlgo_Lib::m_sLightData[n].m_byLightCnt = pRedLightData[n].m_byLightCnt;
		for (int a = 0; a < LIGHT_DATA_CNT; a++)
		{
			for (int b = 0; b < LIGHT_CNT; b++)
			{
				byte byData = pRedLightData[n].m_byArrLightData[a][b];
				PIAL::PInspAlgo_Lib::m_sLightData[n].m_byArrLightData[a][b] = byData;
			}
		}
	}
	bRet = TRUE;
	return bRet;
}

UCHAR* MPTI_GetPttFileChannel(int nChannel)
{
	if (g_pMPTI == NULL)	return eMR_NULL;

	return g_pMPTI->GetPttFileChannel(nChannel);
}

UCHAR* MPTI_GetRawDataFovBuffer(int nChannel)
{
	if (g_pMPTI == NULL)	return eMR_NULL;

	return g_pMPTI->GetRawDataFovBuffer(nChannel);
}

UCHAR* MPTI_GetRawDataFovBuffer_Side(int nChannel, int nCamIdx)
{
	if (g_pMPTI == NULL)	return eMR_NULL;

	return g_pMPTI->GetRawDataFovBuffer_Side(nChannel, nCamIdx);
}

float* MPTI_GetRawDataZmap()
{
	if (g_pMPTI == NULL)	return eMR_NULL;

	return g_pMPTI->GetRawDataZmap();
}

int MPTI_GetRawDataZmapHeight()
{
	if (g_pMPTI == NULL)	return eMR_NULL;

	return g_pMPTI->GetRawDataZmapHeight();
}

int MPTI_GetRawDataZmapWidth()
{
	if (g_pMPTI == NULL)	return eMR_NULL;

	return g_pMPTI->GetRawDataZmapWidth();
}

void  MPTI_FULLImgCompose(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR *ptrbyWhiteBuff,
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	double dROICenterX, double dROICenterY, int nRoiWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage)
{
	if (g_pMPTI == NULL)
		return;
	CPInsp_Algo &InspAlgo = g_pMPTI->GetInspAlgo();
	InspAlgo.FullImageCompose(ptrbyRedBuffer, ptrbyGreenBuffer, ptrbyBlueBuffer, ptrbyWhiteBuff,
		nFullImageSize, nFullImageWidth, nFullImageHeight,
		dROICenterX, dROICenterY, nRoiWidth, nROIHeight,
		nRedValue, nGreenValue, nBlueValue, nWhiteValue, ptrbyResultImage);
}

void  MPTI_FullImageClaculCompose(const LightTypeBuf &sLightImgBuf, UCHAR* ptrbyResultImage)
{
	if (g_pMPTI == NULL)
		return;
	CPInsp_Algo &InspAlgo = g_pMPTI->GetInspAlgo();
	InspAlgo.FullImageClaculCompose(sLightImgBuf, ptrbyResultImage);
}



int MPTI_SetColorFactor(float factorR, float factorG, float factorB, float factorBR, float factorBB)
{
	if (g_pMPTI == NULL) return eMR_FAIL;

	return g_pMPTI->SetColorFactor(factorR, factorG, factorB, factorBR, factorBB);
}

void MPTI_SetCompositeLightMode(int LightMode, float compoBtmR, float compoBtmG, float compoBtmB)
{
	if (g_pMPTI == NULL) return;
	return g_pMPTI->SetCompositeLightMode(LightMode, compoBtmR, compoBtmG, compoBtmB);
}

int MPTI_SetRawDataFovInfo(BSTR filePathPtt, BSTR filePathPot, int nX, int nY, bool bSeparatedPart, int* nWidth, int* nHeight)
{
	if (g_pMPTI == NULL)	return eMR_NULL;

	return g_pMPTI->SetRawDataFovInfo(filePathPtt, filePathPot, nX, nY, bSeparatedPart, nWidth, nHeight);
}
void MPTI_SetMachineOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax)
{
	if (g_pMPTI == NULL)	return;
	g_pMPTI->SetMachineOption(nFootPadTopWGrayLevel, nFootPatternMatchScore, bMatch2D, bMatch3D, nSobelmin, nSobelmax);
}
void MPTI_SetWireOption(int nFootPadTopWGrayLevel, int nFootPatternMatchScore, bool bMatch2D, bool bMatch3D, int nSobelmin, int nSobelmax, float fDiffHigh, float fDiffLow, float fDiameterWire, int nFootPatternMatchStopScore)
{

	CPInsp_WireBonding* PInspWireBonding = g_pMPTI->GetWireBondingInsp();

	if (PInspWireBonding == NULL)
		return;
	PInspWireBonding->SetWireOption(nFootPadTopWGrayLevel, nFootPatternMatchScore, bMatch2D, bMatch3D, nSobelmin, nSobelmax, nFootPatternMatchStopScore);

	PInspWireBonding->m_fDiffHigh = fDiffHigh;//15;//40;
	PInspWireBonding->m_fDiffLow = fDiffLow;// 15;//40;
	PInspWireBonding->m_fDiameterWire = fDiameterWire;

}
BOOL MPTI_TeachFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, int nSelectArea, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	if (ucArrDstImg == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	int nStartX, nStartY;
	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}
	///////////////////////////////////////////////////////////////////////////////
	UCHAR * puImgDst = NULL;
	if (ucArrDstImg)
	{
		Make_1DArray(&puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

		//memset(ucArrDstImg, 0, sizeof(UCHAR) * sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		//memset(puImgDst, 0, sizeof(UCHAR) * sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		ippsSet_8u(0, puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		//PInspAlgo.Update_fromCV(&sWndAlgoImg, ucArrDstImg, puImgDst);
		sWndClipAlgo->m_nWidth3D = sWndAlgoImg.m_nWidth;
		sWndClipAlgo->m_nHeight3D = sWndAlgoImg.m_nHeight;
	}
	CPInsp_WireBonding* PInspWireBonding = g_pMPTI->GetWireBondingInsp();
	bRet = PInspWireBonding->TeachFoot(sInspAlgo, *sWndClipAlgo, nStartX, nStartY, ptrAlgoColorOpt, nSelectArea, puImgDst);
	//////////////////////////////////////////
	/*AlgoFoot *pAlgoFoot = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;
	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	PIAL::_TotalInspExceptArea stTieArea2;

	PIAL::_AlgoFoot algo2;
	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->ConvertAlgo(pAlgoFoot, algo2);
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pwnd_buf_ROI);
	pwnd_buf_ROI->m_rcSubRect = ptrAlgoColorOpt.m_rcImageRect;
	PIAL::PInspAlgo_WireBonding* PInspWireBonding = pInspWarpper->m_PInspWire;


	int nPartWidth = sWndAlgoImg.m_nWidth;
	int nPartHeight = sWndAlgoImg.m_nHeight;

	pImg_buf->inspPartImage->nImgSizeX = nPartWidth;
	pImg_buf->inspPartImage->nImgSizeY = nPartHeight;

	PIAL::PI_Buff puImgDst(ucArrDstImg,nPartWidth, nPartHeight, nPartWidth);
	
	pImg_buf->inspPartImage->nImgSizeX = nStartX;
	pImg_buf->inspPartImage->nImgSizeY = nStartY;
	PIAL::Foot_Model* model = PInspWireBonding->MakeFoot(&algo2, *pImg_buf, nStartX, nStartY, *pwnd_buf_ROI, nSelectArea != m_eFootBin_Pad);
	bRet = pInspWarpper->m_PInspWire->TeachFoot(model,nSelectArea, &puImgDst);*/
	/////////////////////////////////////////

	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}

	/*delete model;
	delete pImg_buf;
	delete pwnd_buf_ROI;*/

	return bRet;
}
CString CMPTI::GetForeignDebugDataFullPath(int nFovCnt, CString csAddName, int nMode, int nFormat, bool bDEBUG)
{
	CString csForeignDebugDataFullPath = _T("");
	CString csRootPath = _T("");
	CString csMakeFile = _T("");
	csMakeFile.Format(_T("D:\\Eagle3D_data\\ForeignDebugData_M"));
	csRootPath.Format(_T("D:\\Eagle3D_data\\ForeignDebugData_M\\%d"), nFovCnt);
#if _DEBUG
	if (bDEBUG)
		csRootPath.Format(_T("D:\\Eagle3D_data\\ForeignDebugData\\DEBUG\\%d"), nFovCnt);
#endif
	CreateDirectory(csMakeFile, NULL);
	CreateDirectory(csRootPath, NULL);
	CString csMode = _T("");
	if (nMode == m_eForeignInsp_Foreign)
		csMode = _T("Fre");
	else if (nMode == m_eForeignInsp_Foreign2D)
		csMode = _T("Fre2D");
	else if (nMode == m_eForeignInsp_Gray)
		csMode = _T("Sol");
	else if (nMode == m_eForeignInsp_GrayBub)
		csMode = _T("Bub");
	else if (nMode == m_eForeignInsp_Copper)
		csMode = _T("Cop");
	else if (nMode == m_eForeignInsp_Warpage)
		csMode = _T("War");
	else if (nMode == m_eForeignInsp_Total)
		csMode = _T("BTM");
	CString csFormat = _T("bmp");
	if (nFormat == 1) csFormat = _T("ptt");
	else if (nFormat == 2) csFormat = _T("ptr");
	CString csFullName = _T("");
	csFullName.Format(_T("%03d_%s_%s"), nFovCnt, csAddName, csMode);
	csForeignDebugDataFullPath.Format(_T("%s\\%s.%s"), csRootPath, csFullName, csFormat);
	return csForeignDebugDataFullPath;
}

BOOL MPTI_HistogramFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, TotalInspExceptArea stTieArea, int nSelectArea, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt)
{
	BOOL bRet = FALSE;

	//if (ucArrDstImg == NULL) return bRet;

	////0.005
	//if (g_pMPTI == NULL)
	//	return bRet;

	//CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	//if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
	//	return bRet;

	//if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
	//	return bRet;

	////0.007
	//int nStartX, nStartY;
	//WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	//if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	//{
	//	if (sWndClipAlgo)
	//	{
	//		delete  sWndClipAlgo;
	//		sWndClipAlgo = NULL;
	//	}
	//	return eMR_FAIL;
	//}

	////0.018
	//CPInsp_WireBonding* PInspWireBonding = g_pMPTI->GetWireBondingInsp();

	//UCHAR * puImgDst = NULL;
	//if (ucArrDstImg)
	//{
	//	Make_1DArray(&puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

	//	//memset(ucArrDstImg, 0, sizeof(UCHAR) * sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
	//	//memset(puImgDst, 0, sizeof(UCHAR) * sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
	//	ippsSet_8u(0, puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
	//	PInspWireBonding->Update_fromCV(&sWndAlgoImg, ucArrDstImg, puImgDst);
	//	sWndClipAlgo->m_nWidth3D = sWndAlgoImg.m_nWidth;
	//	sWndClipAlgo->m_nHeight3D = sWndAlgoImg.m_nHeight;
	//}

	//RstAlgoFoot sRstAlgo;
	//CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, *sWndClipAlgo, nStartX, nStartY, ptrAlgoColorOpt);
	//bRet = PInspWireBonding->InspFoot(pFoot, *sWndClipAlgo, &sRstAlgo, stTieArea, ptrAlgoColorOpt, nSelectArea, puImgDst);

	//
	////UCHAR * puImgDst = NULL;
	//bRet = PInspWireBonding->DrawHistogram_GrayLevelImage(pFoot, puImgDst);

	//if (pFoot)
	//{
	//	delete  pFoot;
	//	pFoot = NULL;
	//}


	return bRet;
}

BOOL MPTI_InspFoot(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, RstAlgoFoot  &sRstAlgo, TotalInspExceptArea stTieArea, int nSelectArea, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;
	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	int nStartX, nStartY;
	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}

	/*AlgoFoot *pAlgoBlob = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;

	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	PIAL::_AlgoFoot algo2;

	PIAL::_RstAlgoFoot RstFoot2;
	PIAL::_TotalInspExceptArea stTieArea2;
	
	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->ConvertAlgo(pAlgoBlob, algo2);
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pwnd_buf_ROI);
	pwnd_buf_ROI->m_rcSubRect = ptrAlgoColorOpt.m_rcImageRect;
	PIAL::PInspAlgo_WireBonding* PInspWireBonding = pInspWarpper->m_PInspWire;*/

	UCHAR * puImgDst = NULL;
	if (ucArrDstImg)
	{
		Make_1DArray(&puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		ippsSet_8u(0, puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);

		sWndClipAlgo->m_nWidth3D = sWndAlgoImg.m_nWidth;
		sWndClipAlgo->m_nHeight3D = sWndAlgoImg.m_nHeight;
	}
	////////////////////////////////////////////////////////////////////////////
	CPInsp_WireBonding* PInspWireBonding = g_pMPTI->GetWireBondingInsp();
	cv::Mat cvimg(0, 0, CV_32FC1);
	cv::Mat cvDBCPolygonImg2(sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth, CV_8UC1);
	cvDBCPolygonImg2.setTo(0);

	bool bUsePadAreaAutoteach = ((AlgoFoot*)sInspAlgo.m_ptrInspAlgoParam)->m_bUsePadAreaAutoTeach;
	CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, sWndAlgoImg, nStartX, nStartY, ptrAlgoColorOpt, &cvimg, &cvDBCPolygonImg2, true, bUsePadAreaAutoteach);
	//CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, *sWndClipAlgo, nStartX, nStartY, ptrAlgoColorOpt, &cvimg, true, bUsePadAreaAutoteach);
	//bRet = PInspWireBonding->InspFoot(pFoot, *sWndClipAlgo, &sRstAlgo, stTieArea, ptrAlgoColorOpt, nSelectArea, puImgDst);
	bRet = PInspWireBonding->InspFoot(pFoot, *sWndClipAlgo, (RstAlgoFoot *)&sRstAlgo, stTieArea, ptrAlgoColorOpt, &cvimg, nSelectArea, puImgDst, NULL, false);
	if (pFoot)
	{
		delete  pFoot;
		pFoot = NULL;
	}

	////////////////////////////////////////////////////////////////////////////
	/*PIAL::Foot_Model* model = PInspWireBonding->MakeFoot(&algo2, *pImg_buf, nStartX, nStartY, *pwnd_buf_ROI, nSelectArea != m_eFootBin_Pad);
	bRet = PInspWireBonding->InspFoot(model, &RstFoot2, stTieArea2, *pwnd_buf_ROI, nSelectArea, puImgDst,false);

	pInspWarpper->ConvertRstAlgo(RstFoot2, &sRstAlgo);

	if (model)
	{
		delete  model;
		model = NULL;
	}*/
	////////////////////////////////////////////////////////////////////////////

	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, puImgDst, ucArrDstImg);
	//LJH
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}
	return bRet;
}

BOOL MPTI_InspFoot_Color(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, RstAlgoFoot &sRstAlgo, TotalInspExceptArea stTieArea, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, int nTabIndex)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;
	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	int nStartX, nStartY;
	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}
	UCHAR * puImgDst = NULL;
	if (ucArrDstImg)
	{
		Make_1DArray(&puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight*3);
		ippsSet_8u(0, puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight * 3);
		sWndClipAlgo->m_nWidth3D = sWndAlgoImg.m_nWidth;
		sWndClipAlgo->m_nHeight3D = sWndAlgoImg.m_nHeight;
	}

// 	CPInsp_WireBonding* PInspWireBonding = g_pMPTI->GetWireBondingInsp();
// 	CFoot_Model* pFoot = PInspWireBonding->MakeFoot(sInspAlgo, *sWndClipAlgo, nStartX, nStartY, ptrAlgoColorOpt);

// 	if (pFoot == nullptr)
// 	{
// 		AfxMessageBox(_T("Teaching Window out of Range."));
// 		return -1;
// 	}
// 		
// 	pFoot->m_rst_Color_Image = true;
// 	pFoot->m_fcBackground = fcBackground;
// 	pFoot->m_fcFoot = fcFoot;
// 	pFoot->m_fcWing = fcWing;
// 	pFoot->m_fcWedge = fcWedge;
// 
// 	bRet = PInspWireBonding->InspFoot(pFoot, *sWndClipAlgo, &sRstAlgo, stTieArea, ptrAlgoColorOpt, nTabIndex, puImgDst);
// 
// 	if (pFoot)
// 	{
// 		delete  pFoot;
// 		pFoot = NULL;
// 	}

	//PInspAlgo.Update_Image_TOCV_3Channel(&sWndAlgoImg, puImgDst, ucArrDstImg);
	////LJH
	//for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
	//	Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	//Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	//Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	//if (sWndClipAlgo)
	//{
	//	delete  sWndClipAlgo;
	//	sWndClipAlgo = NULL;
	//}
	return bRet;
}

BOOL MPTI_TeachFoot_Color(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt, FootColor fcBackground, FootColor fcFoot, FootColor fcWing, FootColor fcWedge, int nTabIndex, cv::Mat *cv3DAvgFilter)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	if (ucArrDstImg == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	int nStartX, nStartY;
	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}

	UCHAR * puImgDst = NULL;
	if (ucArrDstImg)
	{
		Make_1DArray(&puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight * 3);
		ippsSet_8u(0, puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight * 3);
		sWndClipAlgo->m_nWidth3D = sWndAlgoImg.m_nWidth;
		sWndClipAlgo->m_nHeight3D = sWndAlgoImg.m_nHeight;
	}

	CPInsp_WireBonding* PInspWireBonding = g_pMPTI->GetWireBondingInsp();

	//bRet = PInspWireBonding->TeachFootColor(sInspAlgo, *sWndClipAlgo, nStartX, nStartY, ptrAlgoColorOpt, nTabIndex, fcBackground, fcFoot, fcWing, fcWedge, puImgDst, cv3DAvgFilter);
	//bool ret2 = TeachFootColor((AlgoFoot *)sInspAlgo, nfootKind, fcBackground, fcFoot, fcWing, fcWedge, ucArrDstImg, nStartX, nStartY, cv3DAvgFilter);

	//PInspAlgo.Update_Image_TOCV_3Channel(&sWndAlgoImg, puImgDst, ucArrDstImg);

	//////////////////////////////////////////
	//AlgoFoot *pAlgoFoot = (AlgoFoot *)sInspAlgo.m_ptrInspAlgoParam;
	//PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	//PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	//PIAL::_TotalInspExceptArea stTieArea2;

	//PIAL::_AlgoFoot algo2;
	//PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	//pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	//pInspWarpper->ConvertAlgo(pAlgoFoot, algo2);
	//pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pwnd_buf_ROI);
	//pwnd_buf_ROI->m_rcSubRect = ptrAlgoColorOpt.m_rcImageRect;
	//PIAL::PInspAlgo_WireBonding* PInspWireBonding = pInspWarpper->m_PInspWire;


	//int nPartWidth = sWndAlgoImg.m_nWidth;
	//int nPartHeight = sWndAlgoImg.m_nHeight;

	//pImg_buf->inspPartImage->nImgSizeX = nPartWidth;
	//pImg_buf->inspPartImage->nImgSizeY = nPartHeight;

	//PIAL::PI_Buff puImgDst(ucArrDstImg,nPartWidth, nPartHeight, nPartWidth*3,1,3);

	//pImg_buf->inspWndImage->nStartX = nStartX;
	//pImg_buf->inspWndImage->nStartY = nStartY;

	//PIAL::_FootColor ColorPad;
	//PIAL::_FootColor ColorFoot;
	//PIAL::_FootColor ColorWing;
	//PIAL::_FootColor ColorWedge;
	//memcpy(&ColorPad, &fcBackground, sizeof(FootColor));
	//memcpy(&ColorFoot, &fcFoot, sizeof(FootColor));
	//memcpy(&ColorWing, &fcWing, sizeof(FootColor));
	//memcpy(&ColorWedge, &fcWedge, sizeof(FootColor));

	//PIAL::Foot_Model* model = PInspWireBonding->MakeFoot(&algo2, *pImg_buf, nStartX, nStartY, *pwnd_buf_ROI, nTabIndex != m_eFootBin_Pad);
	//nStartX = model->mImageRect.left - (ptrAlgoColorOpt.m_sFovImg.nImageSizeX / 2 - ptrAlgoColorOpt.m_sImgBuf.nImageSizeX);
	//nStartY = model->mImageRect.top - (ptrAlgoColorOpt.m_sFovImg.nImageSizeY / 2 - ptrAlgoColorOpt.m_sImgBuf.nImageSizeY);
	//if (ptrAlgoColorOpt.m_sImgBuf.nImageSizeX <= 0) nStartX = model->mImageRect.left;
	//if (ptrAlgoColorOpt.m_sImgBuf.nImageSizeY <= 0)nStartY = model->mImageRect.top;
	//bRet = pInspWarpper->m_PInspWire->TeachFootColor(model, nTabIndex, ColorPad, ColorFoot, ColorWing, ColorWedge, &puImgDst, nStartX, nStartY);
	/////////////////////////////////////////
	
	//LJH
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}
	/////////////////////////////////////////
	/*delete model;
	delete pImg_buf;
	delete pwnd_buf_ROI;*/
	/////////////////////////////////////////
	return bRet;
}

BOOL MPTI_InspWire(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, RstAlgoWire  &sRstAlgo, TotalInspExceptArea stTieArea, int nSelectArea, float fTeachH, UCHAR* ucArrDstImg, AlgoColorOpt &ptrAlgoColorOpt)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;
	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;
	//0.002
	int nStartX, nStartY;
	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}
	
	///신규////////////////////////////////////////////////////////////////////////////////////////////////////

	/*AlgoWire *pAlgowire = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	PIAL::_TotalInspExceptArea stTieArea2;

	PIAL::_AlgoWire algo2;
	PIAL::_RstAlgoWire Rstwire2;
	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->ConvertAlgo(pAlgowire, algo2);
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pwnd_buf_ROI);
	pwnd_buf_ROI->m_rcSubRect = ptrAlgoColorOpt.m_rcImageRect;
	PIAL::PInspAlgo_WireBonding* PInspWireBonding = pInspWarpper->m_PInspWire;


	int nPartWidth = sWndAlgoImg.m_nWidth;
	int nPartHeight = sWndAlgoImg.m_nHeight;

	pImg_buf->inspPartImage->nImgSizeX = nPartWidth;
	pImg_buf->inspPartImage->nImgSizeY = nPartHeight;

	PIAL::PI_Buff puImgDst(ucArrDstImg, nPartWidth, nPartHeight, nPartWidth);

	pImg_buf->inspWndImage->nStartX = nStartX;
	pImg_buf->inspWndImage->nStartY = nStartY;

	POINTF poArrInspFoot[2];
	AlgoWire *pAlgo = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
	poArrInspFoot[0].x = pAlgo->m_sArrPoint[0].x + (sWndClipAlgo->m_nWidth / 2.);
	poArrInspFoot[0].y = pAlgo->m_sArrPoint[0].y + (sWndClipAlgo->m_nHeight / 2.);
	poArrInspFoot[1].x = pAlgo->m_sArrPoint[1].x + (sWndClipAlgo->m_nWidth / 2.);;
	poArrInspFoot[1].y = pAlgo->m_sArrPoint[1].y + (sWndClipAlgo->m_nHeight / 2.);
	PIAL::Wire_Model* model = PInspWireBonding->MakeWire(algo2, *pImg_buf, nStartX, nStartY, *pwnd_buf_ROI);
	bRet = pInspWarpper->m_PInspWire->InspWire(model, &Rstwire2, poArrInspFoot, nSelectArea, &puImgDst);
	pInspWarpper->ConvertRstAlgo(Rstwire2, &sRstAlgo);*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// 기존
	CPInsp_WireBonding * PInspWireBonding = g_pMPTI->GetWireBondingInsp();

	UCHAR * puImgDst = NULL;
	if (ucArrDstImg)
	{
		Make_1DArray(&puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		//memset(ucArrDstImg, 0, sizeof(UCHAR) * sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		//memset(puImgDst, 0, sizeof(UCHAR) * sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		//PInspAlgo.Update_fromCV(&sWndAlgoImg, ucArrDstImg, puImgDst);
		sWndClipAlgo->m_nWidth3D = sWndAlgoImg.m_nWidth;
		sWndClipAlgo->m_nHeight3D = sWndAlgoImg.m_nHeight;
	}

	POINTF poArrInspFoot[2];
	AlgoWire *pAlgo = (AlgoWire *)sInspAlgo.m_ptrInspAlgoParam;
	poArrInspFoot[0].x = pAlgo->m_sArrPoint[0].x + (sWndClipAlgo->m_nWidth/2.);
	poArrInspFoot[0].y = pAlgo->m_sArrPoint[0].y + (sWndClipAlgo->m_nHeight / 2.);
	poArrInspFoot[1].x = pAlgo->m_sArrPoint[1].x + (sWndClipAlgo->m_nWidth / 2.);;
	poArrInspFoot[1].y = pAlgo->m_sArrPoint[1].y + (sWndClipAlgo->m_nHeight / 2.);
	bRet = PInspWireBonding->InspWire(sInspAlgo, *sWndClipAlgo, &sRstAlgo, stTieArea, nStartX, nStartY, ptrAlgoColorOpt, poArrInspFoot, nSelectArea, fTeachH, puImgDst);

	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, puImgDst, ucArrDstImg);
	
	//LJH
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/*delete model;
	delete pImg_buf;
	delete pwnd_buf_ROI;*/

	///////////////////////////////////////////////////////////////////////////////////////////////////////


	return bRet;
}


BOOL MPTI_WireAutoSearch(AlgoFoot& foot1, AlgoFoot& foot2, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, int rect1Num, RECT*  pRect1, int Rect2Num, RECT* pRect2, WireSearch& footPos, int nSearchType)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	CPInsp_WireBonding * PInspWireBonding = g_pMPTI->GetWireBondingInsp();

	int nStartX, nStartY;
	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}

	sWndClipAlgo->m_nWidth3D = sWndAlgoImg.m_nWidth;
	sWndClipAlgo->m_nHeight3D = sWndAlgoImg.m_nHeight;

	
	CSearchWire_Model* pModel = new CSearchWire_Model;
// 	pModel->m_pFoot1 = PInspWireBonding->MakeFoot(&foot1, *sWndClipAlgo, nStartX, nStartY, ptrAlgoColorOpt,NULL,true,foot1.m_bUsePadAreaAutoTeach);
// 	pModel->m_pFoot2 = PInspWireBonding->MakeFoot(&foot2, *sWndClipAlgo, nStartX, nStartY, ptrAlgoColorOpt, NULL,true,foot2.m_bUsePadAreaAutoTeach);
	pModel->m_nWidth = sWndClipAlgo->m_nWidth3D;
	pModel->m_nHeight = sWndClipAlgo->m_nHeight3D;
	pModel->m_pPart3D = sWndAlgoImg.m_fArr3D_part;
	pModel->m_pPartImageBuffer = &ptrAlgoColorOpt.m_sFovImg;

	for (size_t i = 0; i < rect1Num; i++)
		pModel->m_ROI_Foot1.push_back(pRect1[i]);
	
	for (size_t i = 0; i < Rect2Num; i++)
		pModel->m_ROI_Foot2.push_back(pRect2[i]);

	if (PInspWireBonding->SearchWire(pModel, footPos, nSearchType))
		bRet = TRUE;
	else
		bRet = FALSE;

	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	
	delete pModel;

	if(sWndAlgoImg.m_ucArr2D)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	if(sWndAlgoImg.m_fArr3D)
		Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}

	return bRet;
}


BOOL MPTI_InspWireShort(int nWireNum, WireRst* Wirerst, WndAlgoImg &sWndAlgoImg, UCHAR* ucArrDstImg)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	CPInsp_WireBonding * PInspWireBonding = g_pMPTI->GetWireBondingInsp();

	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	

	UCHAR * puImgDst = NULL;
	if (ucArrDstImg)
	{
		Make_1DArray(&puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
		//PInspAlgo.Update_fromCV(&sWndAlgoImg, ucArrDstImg, puImgDst);
	}

	/*PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->InspWireShort(nWireNum, Wirerst, puImgDst, sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);*/

	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, puImgDst, ucArrDstImg);
	//LJH
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	return bRet;
}

BOOL MPTI_LoadFootModelList(wchar_t* FolderPath)
{

	bool bRet = false;
	if (g_pMPTI == NULL)
		return bRet;
	CString sFolderPath = _T("");
	sFolderPath.Format(L"%s", FolderPath);
	/*PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	PIAL::PInspAlgo_WireBonding* PInspWireBonding = pInspWarpper->m_PInspWire;

	bRet = PInspWireBonding->LoadFootModelList(sFolderPath);*/
	CPInsp_WireBonding * PInspWireBonding = g_pMPTI->GetWireBondingInsp(); //우선 모비스 기준으로
	bRet = PInspWireBonding->LoadFootModelList(sFolderPath);
	return bRet == true ? TRUE : FALSE;
}
BOOL MPTI_CheckModelList(wchar_t* FolderPath)
{

	bool bRet = false;
	if (g_pMPTI == NULL)
		return bRet;
	CString sFolderPath = _T("");
	sFolderPath.Format(L"%s", FolderPath);
	/*PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	PIAL::PInspAlgo_WireBonding* PInspWireBonding = pInspWarpper->m_PInspWire;

	bRet = PInspWireBonding->CheckModelList(sFolderPath);*/
	CPInsp_WireBonding * PInspWireBonding = g_pMPTI->GetWireBondingInsp();//우선 모비스 기준으로
	bRet = PInspWireBonding->CheckModelList(sFolderPath);

	return bRet == true ? TRUE : FALSE;
}

BOOL MPTI_InspBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, RstAlgoBlob &sRstAlgo, TotalInspExceptArea stTieArea, bool bDisplayInvert)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	int WndImgCV = eMR_FAIL;
	if (sInspAlgo.m_nMixCount == 0)
		WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);
	if (WndImgCV == eMR_FAIL)
		return bRet;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}

	
	AlgoCoordinate sAlgoCoodi;
	//if (PInspAlgo.GetCoordinate(sWndAlgoImg, sWndInfo, &sAlgoCoodi) == false)
	//{
	//	if (sWndClipAlgo)
	//	{
	//		delete  sWndClipAlgo;
	//		sWndClipAlgo = NULL;
	//	}
	//	return eMR_FAIL;
	//}

	AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;

	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	PIAL::_TotalInspExceptArea stTieArea2;
	PIAL::_AlgoBlob algo2;
	PIAL::_RstAlgoBlob rstAlgo2;
	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->ConvertAlgo(pAlgoBlob, algo2);

	PIAL::PI_Buff* puImgDst = nullptr;
	if (pUcImgBlob)
		puImgDst = new PIAL::PI_Buff(pUcImgBlob,sWndAlgoImg.m_nWidth,sWndAlgoImg.m_nHeight, sWndAlgoImg.m_nWidth);

	pImg_buf->inspWndImage->nStartX = nStartX;
	pImg_buf->inspWndImage->nStartY = nStartY;

	pInspWarpper->m_PInspAlgo->InspBlob(algo2, *pImg_buf,*pwnd_buf_ROI, &rstAlgo2, stTieArea2, puImgDst);

	pInspWarpper->ConvertRstAlgo(rstAlgo2, &sRstAlgo);

	delete pImg_buf;
	delete pwnd_buf_ROI;
	ptrAlgoColorOpt.Destroy();

	if (puImgDst)
	{

		delete puImgDst;
	}

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	//LJH
	for (int iLoopCount = sInspAlgo.m_nMixCount; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}
	return bRet;
}



BOOL MPTI_InspBlob2(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, RstAlgoBlob  &sRstAlgo, TotalInspExceptArea TieArea, bool bDisplayIvert)
{
	BOOL bRet = FALSE;
	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();


	//if(PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
	//	return bRet;

	//if(PInspAlgo.WndAlgoImg_Update_fromCV_MixImg(&sWndAlgoImg) == eMR_FAIL)
	//	return bRet;
	int WndImgCV = eMR_FAIL;
	if (sInspAlgo.m_nMixCount == 0)
		WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);
	

	if (WndImgCV == eMR_FAIL)
		return bRet;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
// 	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
// 	{
// 		if (sWndClipAlgo)
// 		{
// 			delete  sWndClipAlgo;
// 			sWndClipAlgo = NULL;
// 		}
// 		return eMR_FAIL;
// 	}
	UCHAR * puImgDst = NULL;
	if (pUcImgBlob)
		Make_1DArray(&puImgDst, sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);

	AlgoCoordinate sAlgoCoodi;

	//if (ptrAlgoColorOpt.m_sFovImg.nImageSizeX > 0 && ptrAlgoColorOpt.m_sFovImg.nImageSizeY > 0)
	//{
	//	if (PInspAlgo.WndAlgoImg_Update_fromCV_Color(&ptrAlgoColorOpt) == eMR_FAIL)
	//	{
	//		ptrAlgoColorOpt.Destroy();
	//		return bRet;
	//	}
	//}
	AlgoBlob *pAlgoBlob = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
	/////////////////////////////////////////////////

	/////////////////////////////////////////////////

	//bRet = PInspAlgo.InspBlob(sInspAlgo, sWndAlgoImg, *sWndClipAlgo, ptrAlgoColorOpt.m_sImgBuf, sAlgoCoodi, sWndInfo, puImgDst, &sRstAlgo, TieArea, 0);


	ptrAlgoColorOpt.Destroy();

	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, puImgDst, pUcImgBlob);
	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	//LJH
	for (int iLoopCount = sInspAlgo.m_nMixCount; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}
	return bRet;
}
UCHAR* CMPTI::GetGrabOutputBuf_Color(int nCycleIdx, int nMode, int nChannel)
{

// 	nCycleIdx = nCycleIdx % 4;	// 4순환 10 * 4 , Mode 0 :  Top R.G.B (Channel 1,2,3) , Mode 1 : BB 3 (UV)  BR 1 ,  Mode 2 : Sub UV Channel
// 
// 	if (nMode == 1)
// 		return m_outputCycleUV[nCycleIdx][nChannel - 1];
// 	else if (nMode == 0)
// 		return m_outputCycleRGB[nCycleIdx][nChannel - 1];
// 	else
// 		return m_outputCycleSubUV[nCycleIdx][nChannel - 1];
	TRACE(_T("MPTI::GetGrabOutputBuf_Color"));
	return 0;//m_pMGrab->GetGrabOutputBuf_Color(nCycleIdx, nMode, nChannel);
}
BOOL CMPTI::IsMachineTypeUV()
{
	BOOL bIsCIMachine = FALSE;
// 	if (m_nDIffMachine == DiffMachineUV || m_nDIffMachine == DiffMachineDualsideCI)
// 		bIsCIMachine = TRUE;

	return bIsCIMachine;
}
void* CMPTI::GetSeqGrabBufID(int sequence, int fovNo, int channel2D, int projectionmode, int nGen2D)
{
// 	int channelIndex = GetSeqGrabIndex(sequence, fovNo, channel2D, projectionmode, nGen2D);
// 	return m_pMGrab->GetContinueGrabBufID(channelIndex);
	int nTest = 0 ; 
	return (void *)nTest;
}
BOOL MPTI_TeachBGA(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, PIAL::TeachBGA &sTeachBGA, TotalInspExceptArea stTieArea, bool bDisplayInvert)
{

	/* setNumThreads(5);
	 int x = getNumThreads(5);*/
	/*int num2 = 0;
	ippGetNumThreads(&num2);*/

	//IppStatus is =  ippSetNumThreads(5);
	//int num = 0;
	//ippGetNumThreads(&num);
//	ippSetNumThreads
	/*UCHAR* ptr = new UCHAR[4096* 3072];

	alpf_load_buk(L"D:\\Test\\sfg4_t_g.buk", ptr);

	UCHAR* ptr2 = new UCHAR[4096 * 3072];


	for (size_t i = 0; i < 4096 * 3072; i++)
	{
		if (ptr[i] >= 128)
			ptr2[i] = 256;
	}

	jsl::Blob* blob = new jsl::Blob;
	blob->Alloc(4096, 3072);
	blob->Calculate(ptr2, 4092, 3072, 4092);

	delete[] ptr;
	delete[] ptr2;
	delete blob;

	return false;*/

	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();

	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;
	
	int WndImgCV = eMR_FAIL;
	//if (sInspAlgo.m_nMixCount == 0)
		WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);
	if (WndImgCV == eMR_FAIL)
		return bRet;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}


	//AlgoCoordinate sAlgoCoodi;
	//if (PInspAlgo.GetCoordinate(sWndAlgoImg, sWndInfo, &sAlgoCoodi) == false)
	//{
	//	if (sWndClipAlgo)
	//	{
	//		delete  sWndClipAlgo;
	//		sWndClipAlgo = NULL;
	//	}
	//	return eMR_FAIL;
	//}

	AlgoBGA *pAlgoBlob = (AlgoBGA *)sInspAlgo.m_ptrInspAlgoParam;
	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	PIAL::_TotalInspExceptArea stTieArea2;
	PIAL::_AlgoBGA algo2;
	//PIAL::_RstAlgoBGA rstAlgo2;
	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->ConvertAlgo(pAlgoBlob, algo2);

	PIAL::PI_Buff* puImgDst = nullptr;
	if (pUcImgBlob)
		puImgDst = new PIAL::PI_Buff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);

	pImg_buf->inspWndImage->nStartX = nStartX;
	pImg_buf->inspWndImage->nStartY = nStartY;

	pInspWarpper->m_PInspAlgo->TeachBGA(algo2, *pImg_buf, &sTeachBGA, stTieArea2, puImgDst);

	if (sTeachBGA.m_numBump > 0 && sTeachBGA.pBump != NULL)
	{
		for (size_t i = 0; i < sTeachBGA.m_numBump; i++)
		{
			sTeachBGA.pBump[i].m_cx += nStartX;
			sTeachBGA.pBump[i].m_cy += nStartY;

			sTeachBGA.pBump[i].m_rcRect_I.left += nStartX;
			sTeachBGA.pBump[i].m_rcRect_I.top += nStartY;
			sTeachBGA.pBump[i].m_rcRect_I.right += nStartX;
			sTeachBGA.pBump[i].m_rcRect_I.bottom += nStartY;
		}

	}

	//Test Code
	/*for (size_t i = 0; i < 8710; i++)
	{
	pInspWarpper->m_PInspAlgo->TeachBGA(algo2, *pWnd_buf, *pwnd_buf_ROI, &sTeachBGA, stTieArea2, puImgDst);

	if (sTeachBGA.m_numBump > 0 && sTeachBGA.pBump != NULL)
	{
		for (size_t i = 0; i < sTeachBGA.m_numBump; i++)
		{
			sTeachBGA.pBump[i].m_cx += nStartX;
			sTeachBGA.pBump[i].m_cy += nStartY;

			sTeachBGA.pBump[i].m_rcRect_I.left += nStartX;
			sTeachBGA.pBump[i].m_rcRect_I.top += nStartY;
			sTeachBGA.pBump[i].m_rcRect_I.right += nStartX;
			sTeachBGA.pBump[i].m_rcRect_I.bottom += nStartY;
		}

	}
	  if(i < 8709)
		delete[] sTeachBGA.pBump;

	}*/

	UCHAR* pbuffer = new UCHAR[sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight];
	memset(pbuffer, 0, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
	memcpy(pbuffer, puImgDst->m_pData, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
	
	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, pbuffer, pUcImgBlob);

	ptrAlgoColorOpt.Destroy();
	delete pImg_buf;
	delete pwnd_buf_ROI;
	
	if (puImgDst)
		delete puImgDst;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	//LJH
	for (int iLoopCount = sInspAlgo.m_nMixCount; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}

	return bRet;
}
BOOL MPTI_DrawBGABinary(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, UCHAR *pUcImgBlob)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	bool bIsXray = MPTI_GetRawDataMachineType() == (int)eRawDataAXI;
	bool bAbnormalImage = bIsXray
		? (sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0)
		: (sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0);
	if (bAbnormalImage)
		return bRet;

	int WndImgCV = eMR_FAIL;
	if (sInspAlgo.m_nMixCount == 0)
		WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);
	if (WndImgCV == eMR_FAIL)
		return bRet;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}

	AlgoBGA *pAlgoBlob = (AlgoBGA *)sInspAlgo.m_ptrInspAlgoParam;
	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	PIAL::_AlgoBGA algo2;
	//PIAL::_RstAlgoBGA rstAlgo2;
	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->ConvertAlgo(pAlgoBlob, algo2);

	PIAL::PI_Buff* puImgDst = nullptr;
	if (pUcImgBlob)
		puImgDst = new PIAL::PI_Buff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);

	pImg_buf->inspWndImage->nStartX = nStartX;
	pImg_buf->inspWndImage->nStartY = nStartY;

	//pInspWarpper->m_PInspAlgo->TeachBGA(algo2, *pImg_buf, *pwnd_buf_ROI, &sTeachBGA, stTieArea2, puImgDst);
	pInspWarpper->m_PInspAlgo->DrawBGABinary(algo2, *pImg_buf, *pwnd_buf_ROI, puImgDst);

	UCHAR* pbuffer = new UCHAR[sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight];
	memset(pbuffer, 0, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
	memcpy(pbuffer, puImgDst->m_pData, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);

	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, pbuffer, pUcImgBlob);

	delete pImg_buf;
	delete pwnd_buf_ROI;

	if (puImgDst)
		delete puImgDst;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	//LJH
	for (int iLoopCount = sInspAlgo.m_nMixCount; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}

	return bRet;
}

void MPTI_DeleteBumps(struct tagTeachBump* pBump)
{
	delete[] pBump;
}

void MPTI_DeleteUnits(struct tagTeachUnit* pUnit)
{
	delete[] pUnit;
}

int MPTI_GetCIEMapAboutImage(UCHAR* rstCIEMap, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, UCHAR* rstBin, AlgoColorOpt& ColorOpt, bool bUseColorMap2)
{
	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();	
	if (&PInspAlgo == NULL)
		return eMR_FAIL;

	int dstCieSizeX = _SIZE_CLRX;
	int dstCieSizeY = _SIZE_CLRY;

	BOOL bRet = FALSE;	
	RECT ROIRect = RECT();
	ROIRect.left = ColorOpt.m_rcImageRect.left;
	ROIRect.right = ColorOpt.m_rcImageRect.right;
	ROIRect.top = ColorOpt.m_rcImageRect.top;
	ROIRect.bottom = ColorOpt.m_rcImageRect.bottom;
	
	int nWidth = ROIRect.right - ROIRect.left + 1;
	int nLength = ROIRect.bottom - ROIRect.top + 1;

	PIAL::PI_Buff* bufTR = NULL;
	PIAL::PI_Buff* bufTG = NULL;
	PIAL::PI_Buff* bufTB = NULL;
	PIAL::PI_Buff* bufBR = NULL;
	PIAL::PI_Buff* bufBB = NULL;

	int nPartWidth = g_pMPTI->m_nSizeXRawData;
	int nPartLength = g_pMPTI->m_nSizeYRawData;
	UCHAR* mAC_Red = new UCHAR[nPartWidth * nPartLength];
	UCHAR* mAC_Green = new UCHAR[nPartWidth * nPartLength];
	UCHAR* mAC_Blue = new UCHAR[nPartWidth * nPartLength];

	if (bUseColorMap2)
	{
		MPTI_GetColorBuffer_Part(mAC_Red, mAC_Green, mAC_Blue, nPartWidth, nPartLength, true, true);
		bufTR = new PIAL::PI_Buff(mAC_Red, nPartWidth, nPartLength, nPartWidth);
		bufTG = new PIAL::PI_Buff(mAC_Green, nPartWidth, nPartLength, nPartWidth);
		bufTB = new PIAL::PI_Buff(mAC_Blue, nPartWidth, nPartLength, nPartWidth);
	}
	else
	{
		bufTR = new PIAL::PI_Buff(g_pMPTI->m_parrPartBuf[eM2C_TR], nPartWidth, nPartLength, nPartWidth);
		bufTG = new PIAL::PI_Buff(g_pMPTI->m_parrPartBuf[eM2C_TG], nPartWidth, nPartLength, nPartWidth);
		bufTB = new PIAL::PI_Buff(g_pMPTI->m_parrPartBuf[eM2C_TB], nPartWidth, nPartLength, nPartWidth);

		if (g_pMPTI->m_parrPartBuf[eM2C_BR] != NULL)
			bufBR = new PIAL::PI_Buff(g_pMPTI->m_parrPartBuf[eM2C_BR], nPartWidth, nPartLength, nPartWidth);
		if (g_pMPTI->m_parrPartBuf[eM2C_BB] != NULL)
			bufBB = new PIAL::PI_Buff(g_pMPTI->m_parrPartBuf[eM2C_BB], nPartWidth, nPartLength, nPartWidth);
	}
	PIAL::PI_Buff* bufCloneTR = bufTR->ClipBuff_LT(ROIRect, true);
	PIAL::PI_Buff* bufCloneTG = bufTG->ClipBuff_LT(ROIRect, true);
	PIAL::PI_Buff* bufCloneTB = bufTB->ClipBuff_LT(ROIRect, true);
	PIAL::PI_Buff* bufCloneBR = NULL;
	PIAL::PI_Buff* bufCloneBB = NULL;

	if (bufBR != NULL)
		bufCloneBR = bufBR->ClipBuff_LT(ROIRect, true);
	if (bufBB != NULL)
		bufCloneBB = bufBB->ClipBuff_LT(ROIRect, true);

//	pInspWarpper->m_PTeaching->GetColorCIEView(rstCIEMap, PIAL::PInspAlgo_Lib::CIEMapOriginal, bufCloneTR, bufCloneTG, bufCloneTB, nWidth, nLength);

	if (bufTR != NULL)	delete bufTR;
	if (bufTG != NULL)	delete bufTG;
	if (bufTB != NULL)	delete bufTB;
	if (bufBR != NULL)	delete bufBR;
	if (bufBB != NULL)	delete bufBB;

	if (bufCloneTR != NULL)	delete bufCloneTR;
	if (bufCloneTG != NULL)	delete bufCloneTG;
	if (bufCloneTB != NULL)	delete bufCloneTB;
	if (bufCloneBR != NULL)	delete bufCloneBR;
	if (bufCloneBB != NULL)	delete bufCloneBB;

	delete mAC_Red;
	delete mAC_Green;
	delete mAC_Blue;
}

void MPTI_GetColorBuffer_Part(UCHAR* buf_R, UCHAR* buf_G, UCHAR* buf_B, int nWidth, int nHeight, bool bTeach, int nInspAC)
{
	if (g_pMPTI == NULL)	return;
	PInspAlgoWrapper* wrapper = g_pMPTI->GetPInspWrapper();

	if (bTeach && (/*bAngleColor || */((nInspAC & _eDefaultAC_Data_Use) == _eDefaultAC_Data_Use)))
	{
		PIAL::_LightTypeBuf sLightBuf;
		sLightBuf.m_pucTRed = g_pMPTI->m_parrPartBuf[eM2C_TR];
		sLightBuf.m_pucTGreen = g_pMPTI->m_parrPartBuf[eM2C_TG];
		sLightBuf.m_pucTBlue = g_pMPTI->m_parrPartBuf[eM2C_TB];
		sLightBuf.m_pucTWhite = g_pMPTI->m_parrPartBuf[eM2C_TW];
		sLightBuf.m_pucMRed = g_pMPTI->m_parrPartBuf[eM2C_MR];
		sLightBuf.m_pucMGreen = NULL;
		sLightBuf.m_pucMBlue = g_pMPTI->m_parrPartBuf[eM2C_MB];
		sLightBuf.m_pucMWhite = NULL;
		sLightBuf.m_pucBRed = g_pMPTI->m_parrPartBuf[eM2C_BR];
		sLightBuf.m_pucBGreen = NULL;
		sLightBuf.m_pucBBlue = g_pMPTI->m_parrPartBuf[eM2C_BB];
		sLightBuf.m_pucBWhite = NULL;
		sLightBuf.m_nImgWidth = nWidth;
		sLightBuf.m_nImgHeight = nHeight;
		sLightBuf.m_nROIImgWidth = nWidth;
		sLightBuf.m_nROIImgHeight = nHeight;
		sLightBuf.m_dROIX = 0;
		sLightBuf.m_dROIY = 0;

		sLightBuf.m_pnRedValue = new int[_LIGHT_CNT] {true, };
		sLightBuf.m_pnGreenValue = new int[_LIGHT_CNT] {true, };
		sLightBuf.m_pnBlueValue = new int[_LIGHT_CNT] {true, };
		sLightBuf.m_pnWhiteValue = new int[_LIGHT_CNT] {true, };
		sLightBuf.m_pnPosition = new int[_LIGHT_CNT] {true, };
		sLightBuf.m_pnCalculation = new int[_LIGHT_CNT] {true, };

		memset(sLightBuf.m_pnRedValue, 0, _LIGHT_CNT * sizeof(int));
		memset(sLightBuf.m_pnGreenValue, 0, _LIGHT_CNT * sizeof(int));
		memset(sLightBuf.m_pnBlueValue, 0, _LIGHT_CNT * sizeof(int));
		memset(sLightBuf.m_pnWhiteValue, 0, _LIGHT_CNT * sizeof(int));
		memset(sLightBuf.m_pnPosition, 0, _LIGHT_CNT * sizeof(int));
		memset(sLightBuf.m_pnCalculation, 0, _LIGHT_CNT * sizeof(int));

		for (int a = 0; a < 3; a++)
		{
			sLightBuf.m_nImgCnt = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byLightCnt;
			memset(sLightBuf.m_pnRedValue, 0, _LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnGreenValue, 0, _LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnBlueValue, 0, _LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnWhiteValue, 0, _LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnPosition, 0, _LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnCalculation, 0, _LIGHT_CNT * sizeof(int));
			for (int b = 0; b < _LIGHT_CNT; b++)
			{
				sLightBuf.m_pnRedValue[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[_eLightData_Red][b];
				sLightBuf.m_pnGreenValue[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[_eLightData_Green][b];
				sLightBuf.m_pnBlueValue[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[_eLightData_Blue][b];
				sLightBuf.m_pnWhiteValue[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[_eLightData_White][b];
				sLightBuf.m_pnPosition[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[_eLightData_Position][b];
				sLightBuf.m_pnCalculation[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[_eLightData_Calculation][b];
			}
			if (a == 0)
				PIAL::PInspAlgo_Lib::ROIImageClaculCompose(sLightBuf, buf_R);
			else if (a == 1)
				PIAL::PInspAlgo_Lib::ROIImageClaculCompose(sLightBuf, buf_G);
			else if (a == 2)
				PIAL::PInspAlgo_Lib::ROIImageClaculCompose(sLightBuf, buf_B);
		}

		delete[] sLightBuf.m_pnRedValue;
		delete[] sLightBuf.m_pnGreenValue;
		delete[] sLightBuf.m_pnBlueValue;
		delete[] sLightBuf.m_pnWhiteValue;
		delete[] sLightBuf.m_pnPosition;
		delete[] sLightBuf.m_pnCalculation;
	}
	else
	{
		buf_R = g_pMPTI->m_parrPartBuf[eM2C_TR];
		buf_G = g_pMPTI->m_parrPartBuf[eM2C_TG];
		buf_B = g_pMPTI->m_parrPartBuf[eM2C_TB];
	}
#ifdef IMAGESAVE
	if (img_R != NULL)
	{
		cv::Mat mat_R = cv::Mat(nHeight, nWidth, CV_8UC1, img_R);
		cv::imwrite("D:\\ColorImage\\img_R.bmp", mat_R);
	}
	if (img_G != NULL)
	{
		cv::Mat mat_G = cv::Mat(nHeight, nWidth, CV_8UC1, img_G);
		cv::imwrite("D:\\ColorImage\\img_G.bmp", mat_G);
	}
	if (img_B != NULL)
	{
		cv::Mat mat_B = cv::Mat(nHeight, nWidth, CV_8UC1, img_B);
		cv::imwrite("D:\\ColorImage\\img_B.bmp", mat_B);
	}
#endif
	return;
}

BOOL MPTI_SetAngleColor(byte* byData)
{
	BOOL bRet = FALSE;
	for (int a = 0; a < _eDefaultAC_Total; a++)
		PIAL::PInspAlgo_Lib::m_byDefaultAC[a] = byData[a];

	bRet = TRUE;
	return bRet;
}

void MPTI_SetRawDataMachineType(int nType)
{
	PIAL::PInspAlgo_Lib::m_MachineType = nType;
}

int MPTI_GetRawDataMachineType()
{
	return PIAL::PInspAlgo_Lib::m_MachineType;
}

BOOL MPTI_InspMeanGray(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, double &dResult, byte insptype, int &nReviewMin, int &nReviewMax, TotalInspExceptArea stTieArea)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();

	//if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
	//	return bRet;

	int WndImgCV = eMR_FAIL;
	if (sInspAlgo.m_nMixCount == 0)
		WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);
	if (WndImgCV == eMR_FAIL)
		return bRet;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}

	AlgoGrayMean *pAlgoBlob = (AlgoGrayMean *)sInspAlgo.m_ptrInspAlgoParam;
	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	PIAL::_TotalInspExceptArea stTieArea2;
	PIAL::_AlgoGrayMean algo2;
	PIAL::_RstAlgoGrayMean* rstGrayMean = new PIAL::_RstAlgoGrayMean();

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->ConvertAlgo(pAlgoBlob, algo2);
	//(algoGrayMean, *pImg_buf, rstGrayMean, stTieArea,m_pParamArray[nWndIndex].WndInspType);
	BOOL bReturn = pInspWarpper->m_PInspAlgo->InspGrayMean(algo2, *pImg_buf, rstGrayMean, stTieArea2, insptype);
	
}

BOOL MPTI_InspPadBW(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR* pUcPadBWDst, RstAlgoPadBW &sRstAlgo, double dStartX, double dStartY)
{
	bool bRet = FALSE;

	//성광테크 branch 코드//////////////////////////////////////////////////////////////////
// 	CPInsp_Algo &PInspAlgo = g_pInspMng->GetInspAlgo();
// 
// 
	LightTypeBuf sLightBuf;
	sLightBuf.m_pucTRed = ptrAlgoColorOpt.m_sFovImg.imgTop_R;
	sLightBuf.m_pucTGreen = ptrAlgoColorOpt.m_sFovImg.imgTop_G;
	sLightBuf.m_pucTBlue = ptrAlgoColorOpt.m_sFovImg.imgTop_B;
	sLightBuf.m_pucTWhite = ptrAlgoColorOpt.m_sFovImg.imgTop_W;

	sLightBuf.m_pucMRed = ptrAlgoColorOpt.m_sFovImg.imgMiddle_R;
	sLightBuf.m_pucMGreen = NULL;
	sLightBuf.m_pucMBlue = ptrAlgoColorOpt.m_sFovImg.imgMiddle_B;
	sLightBuf.m_pucMWhite = NULL;

	sLightBuf.m_pucBRed = ptrAlgoColorOpt.m_sFovImg.imgBottom_R;
	sLightBuf.m_pucBGreen = NULL;
	sLightBuf.m_pucBBlue = ptrAlgoColorOpt.m_sFovImg.imgBottom_B;
	sLightBuf.m_pucBWhite = NULL;

	sLightBuf.m_nImgWidth = sWndInfo.m_dPartWidth;
	sLightBuf.m_nImgHeight = sWndInfo.m_dPartHeight;
	sLightBuf.m_nROIImgWidth = sWndInfo.dWidth;
	sLightBuf.m_nROIImgHeight = sWndInfo.dLength;

	sLightBuf.m_dROIX = dStartX;
	sLightBuf.m_dROIY = dStartY;

	sLightBuf.m_nImgCnt = sInspAlgo.m_nLightCnt;
	sLightBuf.m_pnRedValue = sInspAlgo.m_nArrRedValue;
	sLightBuf.m_pnGreenValue = sInspAlgo.m_nArrGreenValue;
	sLightBuf.m_pnBlueValue = sInspAlgo.m_nArrBlueValue;
	sLightBuf.m_pnWhiteValue = sInspAlgo.m_nArrWhiteValue;
	sLightBuf.m_pnPosition = sInspAlgo.m_nArrLightPosition;
	sLightBuf.m_pnCalculation = sInspAlgo.m_nArrCalculation;
	// 
	// 	bRet = PInspAlgo.InspPadBW(sInspAlgo, sWndAlgoImg, sLightBuf, &sRstAlgo);
	// 
	// 	int nWidth = sWndAlgoImg.m_nWidth;
	// 	int nHeight = sWndAlgoImg.m_nHeight;
	// 	int nFovWidth = g_pInspMng->GetFovWidth();
	// 	int nFovHeight = g_pInspMng->GetFovLength();
	// 
	// 	UCHAR* ptr_Dst = NULL;
	// 	cv::Mat mDst(nFovHeight, nFovWidth, CV_8UC1, g_pInspMng->m_ucPadRstBuffer[MAX_CNT_PAD_COMPOSED_LIGHT]);
	// 	cv::Mat mRoi = mDst(cv::Rect(dStartX, dStartY, nWidth, nHeight));
	// 
	// 	memcpy(&pUcPadBWDst, mRoi.data, nWidth * nHeight * sizeof(UCHAR));
	/////////////////////////////////////////////////////////////////////////////////////////////

		//여기서 부터 padBW 알고리즘 검사함수
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();

	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	int WndImgCV = eMR_FAIL;
	if (sInspAlgo.m_nMixCount == 0)
		WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);
	if (WndImgCV == eMR_FAIL)
		return bRet;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}

	//AlgoCoordinate sAlgoCoodi;
	//if (PInspAlgo.GetCoordinate(sWndAlgoImg, sWndInfo, &sAlgoCoodi) == false)
	//{
	//	if (sWndClipAlgo)
	//	{
	//		delete  sWndClipAlgo;
	//		sWndClipAlgo = NULL;
	//	}
	//	return eMR_FAIL;
	//}

	AlgoPadBW *pAlgoPadBW = (AlgoPadBW *)sInspAlgo.m_ptrInspAlgoParam;

	//PIAL::Insp_Wnd* pWnd_buf = new PIAL::Insp_Wnd;
	PIAL::Insp_Image* pWnd_buf = new PIAL::Insp_Image;

	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
	PIAL::_TotalInspExceptArea stTieArea2;

	PIAL::_AlgoPadBW algo2;
	PIAL::_RstAlgoPadBW rstAlgo2;

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pWnd_buf);
	//pInspWarpper->ConvertAlgo(pAlgoPadBW, algo2);

	PIAL::PI_Buff* puImgDst = nullptr;
	//if (pUcImgBlob)
	puImgDst = new PIAL::PI_Buff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);

	pWnd_buf->inspWndImage->nStartX = nStartX;
	pWnd_buf->inspWndImage->nStartY = nStartY;

	//Test를 위해 Local 변수로 버퍼 할당
	UCHAR** m_FOVImgComposed_buf = NULL;	// = g_pMPTI->m_bUseImagePilLib;//5개
	UCHAR** m_ImgCalc_buf = NULL;			// = m_ucPadCalcBuffer;

	m_FOVImgComposed_buf = g_pMPTI->m_InspMng->m_ucPadRstBuffer; //gm_ucPadRstBuffer;
	m_ImgCalc_buf = g_pMPTI->m_InspMng->m_ucPadCalcBuffer;;

	//UCHAR* m_ImgMaskBuf = m_ucMaskBuf;		//거버 마스크로 위치 및 형상검사(1.기본검사)

	int nFOVBufferSizeW = pInspWarpper->m_fovWidth;
	int nFOVBufferSizeH = pInspWarpper->m_fovLength;

	//조명합성용 채널별 이미지 데이터 세팅
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pwnd_buf_ROI);

	//bAlgoOK = m_PInspWrapper->m_PInspAlgo->InspPadBW(algo2, *pWnd_buf, *pwnd_buf_ROI, &rstAlgo2, nFOVBufferSizeW, nFOVBufferSizeH, m_ImgCalc_buf, m_FOVImgComposed_buf/*, stTieArea2, pPreImgDst*/);
	bRet = pInspWarpper->m_PInspAlgo->InspPadBW(algo2, *pWnd_buf, *pwnd_buf_ROI, &rstAlgo2, nFOVBufferSizeW, nFOVBufferSizeH, m_ImgCalc_buf, m_FOVImgComposed_buf);

	int nWidth = sWndAlgoImg.m_nWidth;
	int nHeight = sWndAlgoImg.m_nHeight;

	UCHAR* ptr_Dst = NULL;
	//cv::Mat mDst(nFOVBufferSizeH, nFOVBufferSizeW, CV_8UC1, m_ucPadRstBuffer[MAX_CNT_PAD_COMPOSED_LIGHT]);
	//cv::Mat mRoi = mDst(cv::Rect(dStartX, dStartY, nWidth, nHeight));

	//memcpy(&pUcPadBWDst, mRoi.data, nWidth * nHeight * sizeof(UCHAR));


// 	UCHAR* pbuffer = new UCHAR[sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight];
// 	memset(pbuffer, 0, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
// 	memcpy(pbuffer, puImgDst->m_pData, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
// 
// 	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, pbuffer, pUcImgBlob);
// 
// 	ptrAlgoColorOpt.Destroy();
	delete pWnd_buf;
	delete pwnd_buf_ROI;

	if (puImgDst)
		delete puImgDst;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	//LJH
	for (int iLoopCount = sInspAlgo.m_nMixCount; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}

	return bRet;
}

BOOL MPTI_BinalizeBodyBlob(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* ucArrDstImg, int nMaskViewingMode, AlgoColorOpt &ptrAlgoColorOpt, RstAlgoBodyBlob *pRstAlgo, TotalInspExceptArea stTieArea, bool bIsLoadFovRawDataImage)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;
	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
	

	AlgoBodyBlob *pAlgoBodyBlob = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;

	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::_AlgoBodyBlob algo2;
	PIAL::_RstAlgoBodyBlob rstAlgo2;
	PIAL::_TotalInspExceptArea stTieArea2;

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->ConvertAlgo(pAlgoBodyBlob, algo2);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspPartImage->nImgSizeX = sWndAlgoImg.m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sWndAlgoImg.m_nHeight3D;


	if (sWndAlgoImg.m_ucArrCV)
	{
		pImg_buf->inspWndImage->nImgSizeX = nImgWidth;
		pImg_buf->inspWndImage->nImgSizeY = nImgHeight;
		pImg_buf->m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArrCV, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	}
	if (sWndAlgoImg.m_fArr3D_part)
	{
		if (pImg_buf->inspWndImage->m_p3D)
			delete pImg_buf->inspWndImage->m_p3D;

		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
		pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
	}
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pImg_buf->inspPartImage->m_ImageBuffer);

	PIAL::PI_Buff dst(ucArrDstImg, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);

	bRet = pInspWarpper->m_PInspAlgo->BinalizeBodyBlob(algo2, *pImg_buf, &rstAlgo2, stTieArea2, &dst);
	pInspWarpper->ConvertRstAlgo(rstAlgo2 ,pRstAlgo);
	
	delete pImg_buf;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	ptrAlgoColorOpt.Destroy();
	//LJH
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	return bRet;
}

BOOL MPTI_InspBodyBlob(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, BOOL bTeach, AlgoColorOpt &ptrAlgoColorOpt,
	RstAlgoBodyBlob *pRstAlgo, CRect &rcBlobBody, UCHAR *pUcImgBlob, TotalInspExceptArea stTieArea, bool bIsLoadFovRawDataImage)
{
	BOOL bRet = FALSE;

	if (sInspAlgo.m_eAlgoType != eAlgoBody_Blob)
		return bRet;
	AlgoBodyBlob *pAlgoBodyBlob = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pAlgoBodyBlob)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();

	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	if (pUcImgBlob)
	{
		memset(pUcImgBlob, 0, sizeof(UCHAR)* sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
	}

	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::_AlgoBodyBlob algo2;
	PIAL::_RstAlgoBodyBlob rstAlgo2;
	PIAL::_TotalInspExceptArea stTieArea2;

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->ConvertAlgo(pAlgoBodyBlob, algo2);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspPartImage->nImgSizeX = sWndAlgoImg.m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sWndAlgoImg.m_nHeight3D;


	if (sWndAlgoImg.m_ucArr2D)
	{
		pImg_buf->inspWndImage->nImgSizeX = nImgWidth;
		pImg_buf->inspWndImage->nImgSizeY = nImgHeight;
		pImg_buf->m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArr2D, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	}
	if (sWndAlgoImg.m_fArr3D_part)
	{
		if (pImg_buf->inspWndImage->m_p3D)
			delete pImg_buf->inspWndImage->m_p3D;

		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
		pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
	}
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pImg_buf->inspPartImage->m_ImageBuffer);

	PIAL::PI_Buff dst(pUcImgBlob, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);

	POINTF ptWndCenter;
	ptWndCenter.x = 0.f;
	ptWndCenter.y = 0.f;

	if(bTeach)
		bRet = pInspWarpper->m_PInspAlgo->TeachBodyBlob(algo2, *pImg_buf, &rstAlgo2, ptWndCenter, stTieArea2, &dst);
	else
	{
		std::vector<PIAL::_AlignResult> vecAlignRst;
		// double HeightOffset = 0; // 넘겨받은 값 입력해야함..
		bRet = pInspWarpper->m_PInspAlgo->InspBodyBlob(algo2, *pImg_buf, rcBlobBody, &rstAlgo2, ptWndCenter, stTieArea2, vecAlignRst, /*HeightOffset, */&dst);
	}


	pInspWarpper->ConvertRstAlgo(rstAlgo2, pRstAlgo);

	delete pImg_buf;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	ptrAlgoColorOpt.Destroy();
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	return bRet;
}

BOOL MPTI_DrawNGBlob(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, TotalInspExceptArea stTieArea) // LSJ
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();

	bool bIsXray = MPTI_GetRawDataMachineType() == (int)eRawDataAXI;
	bool bAbnormalImage = bIsXray
		? (sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0)
		: (sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0);
	if (bAbnormalImage)
		return bRet;

	if (sInspAlgo.m_eAlgoType != eAlgoNGBlob)
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}
	if (ptrAlgoColorOpt.m_sFovImg.nImageSizeX > 0 && ptrAlgoColorOpt.m_sFovImg.nImageSizeY > 0)
	{
		if (PInspAlgo.WndAlgoImg_Update_fromCV_Color(&ptrAlgoColorOpt, false) == eMR_FAIL)
		{
			ptrAlgoColorOpt.Destroy();
			return bRet;
		}
	}

	AlgoNGBlob *pAlgoNGBlob = (AlgoNGBlob *)sInspAlgo.m_ptrInspAlgoParam;
	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::_TotalInspExceptArea stTieArea2;
	PIAL::_AlgoNGBlob algo2;
	PIAL::_RstAlgoNGBlob rstAlgo2;
	std::vector<PIAL::_AlignResult> vrstAlign = std::vector<PIAL::_AlignResult>();


	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt, pImg_buf->inspWndImage->m_ImageBuffer);
	pInspWarpper->ConvertAlgo(pAlgoNGBlob, algo2);
	pInspWarpper->ConvertExceptROI(&sInspAlgo, &stTieArea2, 0);

	PIAL::PI_Buff* puImgDst = nullptr;
	if (pUcImgBlob)
		puImgDst = new PIAL::PI_Buff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspWndImage->nStartX = nStartX;
	pImg_buf->inspWndImage->nStartY = nStartY;
	//Body,Bump Mask 추가 JHJ
	RECT* arrBumpMask = NULL;
	RECT* arrBodyMask = NULL;
	RECT bodyRect = { 0 };
	int nArrRect[2] = { 0,0 };
	std::vector<PIAL::PInspData*> pDataSet;
	PIAL::PInspData pInspData;
	PIAL::BodyInfo* bodyData = new PIAL::BodyInfo();
	bodyData->BodyStartPos = new POINT();
	bodyData->Passive = false;
	bodyData->BodyStartPos->x = bodyRect.left;
	bodyData->BodyStartPos->y = bodyRect.top;
	bodyData->BodySize = new SIZE();
	bodyData->BodySize->cx = bodyRect.right - bodyRect.left;
	bodyData->BodySize->cy = bodyRect.bottom - bodyRect.top;
	pInspData.SetBodyData(bodyData);
	pDataSet.push_back(&pInspData);
	if (algo2.narrdata[_NGBlob_by_inspType] == _m_eNGBlob_InspType::Scratch)//Scratch
		pInspWarpper->m_PInspAlgo->DrawScratch(algo2, *pImg_buf, puImgDst, stTieArea2, arrBumpMask, arrBodyMask, nArrRect);
	else
	{
		pInspWarpper->m_PInspAlgo->DrawNGBlob_renewal(algo2, *pImg_buf, puImgDst, stTieArea2, bodyRect, pDataSet, arrBumpMask, arrBodyMask, nArrRect);
		//pInspWarpper->m_PInspAlgo->DrawNGBlob(algo2, *pImg_buf, puImgDst, stTieArea2, bodyRect, pDataSet);
	}
	UCHAR* pbuffer = new UCHAR[sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight];
	memset(pbuffer, 0, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
	memcpy(pbuffer, puImgDst->m_pData, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);

	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, pbuffer, pUcImgBlob);

	delete pImg_buf;

	ptrAlgoColorOpt.Destroy();

	if (puImgDst)
		delete puImgDst;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	ptrAlgoColorOpt.Destroy();
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}

	return bRet;
}

BOOL MPTI_NGBlobHistogram(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, TotalInspExceptArea stTieArea, UCHAR *retHistoImg, int *nRetRangeMin, int *nRetRangeMax, int *nRetRangeAvg, int* nRetMaxFreqValue)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();

	if (sInspAlgo.m_eAlgoType != eAlgoNGBlob)
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
	int nStartX, nStartY;
	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
	{
		if (sWndClipAlgo)
		{
			delete  sWndClipAlgo;
			sWndClipAlgo = NULL;
		}
		return eMR_FAIL;
	}
	if (ptrAlgoColorOpt.m_sFovImg.nImageSizeX > 0 && ptrAlgoColorOpt.m_sFovImg.nImageSizeY > 0)
	{
		if (PInspAlgo.WndAlgoImg_Update_fromCV_Color(&ptrAlgoColorOpt, false) == eMR_FAIL)
		{
			ptrAlgoColorOpt.Destroy();
			return bRet;
		}
	}

	AlgoNGBlob *pAlgoNGBlob = (AlgoNGBlob *)sInspAlgo.m_ptrInspAlgoParam;
	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::_TotalInspExceptArea stTieArea2;
	PIAL::_AlgoNGBlob algo2;
	PIAL::_RstAlgoNGBlob rstAlgo2;
	std::vector<PIAL::_AlignResult> vrstAlign = std::vector<PIAL::_AlignResult>();

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt, pImg_buf->inspWndImage->m_ImageBuffer);
	pInspWarpper->ConvertAlgo(pAlgoNGBlob, algo2);
	pInspWarpper->ConvertExceptROI(&sInspAlgo, &stTieArea2, 0);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspWndImage->nStartX = nStartX;
	pImg_buf->inspWndImage->nStartY = nStartY;
	//Body,Bump Mask 추가 JHJ
	RECT* arrBumpMask = NULL;
	RECT* arrBodyMask = NULL;
	RECT bodyRect = { 0 };
	int nArrRect[2] = { 0,0 };
	std::vector<PIAL::PInspData*> pDataSet;
	PIAL::PInspData pInspData;
	PIAL::BodyInfo* bodyData = new PIAL::BodyInfo();
	bodyData->BodyStartPos = new POINT();
	bodyData->Passive = false;
	bodyData->BodyStartPos->x = bodyRect.left;
	bodyData->BodyStartPos->y = bodyRect.top;
	bodyData->BodySize = new SIZE();
	bodyData->BodySize->cx = bodyRect.right - bodyRect.left;
	bodyData->BodySize->cy = bodyRect.bottom - bodyRect.top;
	pInspData.SetBodyData(bodyData);
	pDataSet.push_back(&pInspData);

	pInspWarpper->m_PInspAlgo->Histogram_NGBlob(algo2, *pImg_buf, stTieArea2, bodyRect, pDataSet, arrBumpMask, arrBodyMask, nArrRect,retHistoImg,nRetRangeMin,nRetRangeMax, nRetRangeAvg, nRetMaxFreqValue);
	delete pImg_buf;

	ptrAlgoColorOpt.Destroy();
	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	ptrAlgoColorOpt.Destroy();
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	if (sWndClipAlgo)
	{
		delete  sWndClipAlgo;
		sWndClipAlgo = NULL;
	}

	return bRet;

}

BOOL MPTI_BinalizeBodyEdge(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* ucArrDstImg, int nMaskViewingMode, AlgoColorOpt &ptrAlgoColorOpt, bool bIsLoadFovRawDataImage)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;
	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();


	AlgoBodyEdge *pAlgo = (AlgoBodyEdge *)sInspAlgo.m_ptrInspAlgoParam;

	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::_AlgoBodyEdge algo2;

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->ConvertAlgo(pAlgo, algo2);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspPartImage->nImgSizeX = sWndAlgoImg.m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sWndAlgoImg.m_nHeight3D;


	if (sWndAlgoImg.m_ucArrCV)
	{
		pImg_buf->inspWndImage->nImgSizeX = nImgWidth;
		pImg_buf->inspWndImage->nImgSizeY = nImgHeight;
		pImg_buf->m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArrCV, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	}
	if (sWndAlgoImg.m_fArr3D_part)
	{
		if (pImg_buf->inspWndImage->m_p3D)
			delete pImg_buf->inspWndImage->m_p3D;

		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
		pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
	}
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pImg_buf->inspPartImage->m_ImageBuffer);

	PIAL::PI_Buff dst(ucArrDstImg, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);

	bRet = pInspWarpper->m_PInspAlgo->PInspAlgo::BinalizeBodyEdge(algo2, *pImg_buf, &dst);

	delete pImg_buf;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	ptrAlgoColorOpt.Destroy();
	//LJH
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	return bRet;

}

BOOL MPTI_InspBodyEdge(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, BOOL bTeach, AlgoColorOpt &ptrAlgoColorOpt,
	RstAlgoBodyEdge *pRstAlgo, UCHAR *pUcImgBlob, bool bIsLoadFovRawDataImage)
{
	BOOL bRet = FALSE;

	if (sInspAlgo.m_eAlgoType != eAlgoBodyEdge)
		return bRet;
	AlgoBodyEdge *pAlgo = (AlgoBodyEdge *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pAlgo)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();

	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	if (pUcImgBlob)
	{
		memset(pUcImgBlob, 0, sizeof(UCHAR)* sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
	}

	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::_AlgoBodyEdge algo2;
	PIAL::_RstAlgoBodyEdge rstAlgo2;
	PIAL::_TotalInspExceptArea stTieArea2;

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	pInspWarpper->ConvertAlgo(pAlgo, algo2);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspPartImage->nImgSizeX = sWndAlgoImg.m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sWndAlgoImg.m_nHeight3D;


	if (sWndAlgoImg.m_ucArr2D)
	{
		pImg_buf->inspWndImage->nImgSizeX = nImgWidth;
		pImg_buf->inspWndImage->nImgSizeY = nImgHeight;
		pImg_buf->m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArr2D, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	}
	if (sWndAlgoImg.m_fArr3D_part)
	{
		if (pImg_buf->inspWndImage->m_p3D)
			delete pImg_buf->inspWndImage->m_p3D;

		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
		pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
	}
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pImg_buf->inspPartImage->m_ImageBuffer);

	

	POINTF ptWndCenter;
	ptWndCenter.x = 0.f;
	ptWndCenter.y = 0.f;

	if (bTeach)
	{
		PIAL::PI_Buff dst(pUcImgBlob, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
		bRet = pInspWarpper->m_PInspAlgo->TeachBodyEdge(algo2, *pImg_buf, &rstAlgo2, &dst);
	}
	else
	{
		std::vector<PIAL::_AlignResult> vecAlignRst;
		bRet = pInspWarpper->m_PInspAlgo->InspBodyEdge(algo2, *pImg_buf, &rstAlgo2, vecAlignRst);
	}


	pInspWarpper->ConvertRstAlgo(rstAlgo2, pRstAlgo);

	delete pImg_buf;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	ptrAlgoColorOpt.Destroy();
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	return bRet;
}

//BOOL MPTI_TeachQFN(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo sWndInfo, AlgoColorOpt &ptrAlgoColorOpt, UCHAR *pUcImgBlob, PIAL::TeachQFN &sTeachQFN, TotalInspExceptArea stTieArea, bool bDisplayInvert)
//{
//	BOOL bRet = FALSE;
//	if (g_pMPTI == NULL)
//		return bRet;
//
//	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();
//
//	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
//		return bRet;
//
//	int WndImgCV = eMR_FAIL;
//	WndImgCV = PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg);
//	if (WndImgCV == eMR_FAIL)
//		return bRet;
//
//	WndAlgoImg *sWndClipAlgo = new WndAlgoImg;
//	int nStartX, nStartY;
//	if (PInspAlgo.GetWndClip_3(sWndAlgoImg, *sWndClipAlgo, sWndInfo, &nStartX, &nStartY) == false)
//	{
//		if (sWndClipAlgo)
//		{
//			delete  sWndClipAlgo;
//			sWndClipAlgo = NULL;
//		}
//		return eMR_FAIL;
//	}
//
//	AlgoQFN *pAlgoBlob = (AlgoQFN *)sInspAlgo.m_ptrInspAlgoParam;
//
//	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
//	PIAL::Insp_ROIImg* pwnd_buf_ROI = new PIAL::Insp_ROIImg;
//	PIAL::_TotalInspExceptArea stTieArea2;
//
//	PIAL::_AlgoQFN algo2;
//	//PIAL::_RstAlgoQFN rstAlgo2;
//	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
//	pInspWarpper->MakeBuffWnd(sWndClipAlgo, sWndInfo, pImg_buf);
//	pInspWarpper->ConvertAlgo(pAlgoBlob, algo2);
//
//	PIAL::PI_Buff* puImgDst = nullptr;
//	if (pUcImgBlob)
//		puImgDst = new PIAL::PI_Buff(sWndAlgoImg.m_nWidth, sWndAlgoImg.m_nHeight);
//
//	pImg_buf->inspWndImage->nStartX = nStartX;
//	pImg_buf->inspWndImage->nStartY = nStartY;
//
//	////////////////////////////////
//	//pInspWarpper->m_PInspAlgo->TeachBGA(algo2, *pImg_buf, &sTeachBGA, stTieArea2, puImgDst);
//
//	//pInspWarpper->m_PInspAlgo->TeachQFN(algo2, *pImg_buf, &sTeachQFN, stTieArea2, puImgDst);
//	//pInspWarpper->m_PInspAlgo->TeachQFNSecVer(algo2, *pImg_buf, &sTeachQFN, stTieArea2, puImgDst);
//	pInspWarpper->m_PInspAlgo->TeachQFN_Test(algo2, *pImg_buf, &sTeachQFN, stTieArea2, puImgDst);
//
//	if (sTeachQFN.m_numUnit > 0 && sTeachQFN.pUnit != NULL)
//	{
//		for (size_t i = 0; i < sTeachQFN.m_numUnit; i++)
//		{
//			sTeachQFN.pUnit[i].m_cx += nStartX;
//			sTeachQFN.pUnit[i].m_cy += nStartY;
//
//			sTeachQFN.pUnit[i].m_rcRect_I.left += nStartX;
//			sTeachQFN.pUnit[i].m_rcRect_I.top += nStartY;
//			sTeachQFN.pUnit[i].m_rcRect_I.right += nStartX;
//			sTeachQFN.pUnit[i].m_rcRect_I.bottom += nStartY;
//		}
//
//	}
//	////////////////////
//
//	UCHAR* pbuffer = new UCHAR[sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight];
//	memset(pbuffer, 0, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
//	memcpy(pbuffer, puImgDst->m_pData, sWndAlgoImg.m_nWidth*sWndAlgoImg.m_nHeight);
//
//	PInspAlgo.Update_Image_TOCV(&sWndAlgoImg, pbuffer, pUcImgBlob);
//
//	ptrAlgoColorOpt.Destroy();
//	delete pImg_buf;
//	delete pwnd_buf_ROI;
//
//	if (puImgDst)
//		delete puImgDst;
//
//	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
//	//LJH
//	for (int iLoopCount = sInspAlgo.m_nMixCount; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
//		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
//	Delete_1DArray(&sWndAlgoImg.m_fArr3D);
//	if (sWndClipAlgo)
//	{
//		delete  sWndClipAlgo;
//		sWndClipAlgo = NULL;
//	}
//
//	return bRet;
//}

BOOL MPTI_InspGWire(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, WndInfo &sWndInfo, BOOL bTeach, AlgoColorOpt &ptrAlgoColorOpt,
	RstAlgoGWire *pRstAlgo, UCHAR *pUcImgBlob, bool bIsLoadFovRawDataImage)
{
	BOOL bRet = FALSE;

	if (sInspAlgo.m_eAlgoType != eAlgoGWire)
		return bRet;
	AlgoGWire *pAlgo = (AlgoGWire *)sInspAlgo.m_ptrInspAlgoParam;
	if (!pAlgo)
		return bRet;

	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();

	if ((sWndAlgoImg.m_ucArrCV == NULL) || (sWndAlgoImg.m_fArr3D_part == NULL) || (sWndAlgoImg.m_nWidth <= 0) || (sWndAlgoImg.m_nHeight <= 0))
		return bRet;

	if (PInspAlgo.WndAlgoImg_Update_fromCV(&sWndAlgoImg) == eMR_FAIL)
		return bRet;

	if (pUcImgBlob)
	{
		memset(pUcImgBlob, 0, sizeof(UCHAR)* sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight);
	}

	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::_AlgoGWire algo2;
	PIAL::_RstAlgoGWire rstAlgo2;
	PIAL::_TotalInspExceptArea stTieArea2;

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	//pInspWarpper->ConvertAlgo(pAlgo, algo2);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspPartImage->nImgSizeX = sWndAlgoImg.m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sWndAlgoImg.m_nHeight3D;


	if (sWndAlgoImg.m_ucArr2D)
	{
		pImg_buf->inspWndImage->nImgSizeX = nImgWidth;
		pImg_buf->inspWndImage->nImgSizeY = nImgHeight;
		pImg_buf->m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArr2D, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	}
	if (sWndAlgoImg.m_fArr3D_part)
	{
		if (pImg_buf->inspWndImage->m_p3D)
			delete pImg_buf->inspWndImage->m_p3D;

		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
		pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
	}
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pImg_buf->inspPartImage->m_ImageBuffer);



	POINTF ptWndCenter;
	ptWndCenter.x = 0.f;
	ptWndCenter.y = 0.f;

	if (bTeach)
	{
		PIAL::PI_Buff dst(pUcImgBlob, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
		bRet = pInspWarpper->m_PInspAlgo->TeachGWire(algo2, *pImg_buf, &rstAlgo2, &dst);
	}
	else
	{
		std::vector<PIAL::_AlignResult> vecAlignRst;
		bRet = pInspWarpper->m_PInspAlgo->InspGWire(algo2, *pImg_buf, &rstAlgo2, vecAlignRst);
	}


	/*pInspWarpper->ConvertRstAlgo(rstAlgo2, pRstAlgo);*/

	delete pImg_buf;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	ptrAlgoColorOpt.Destroy();
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	return bRet;
}
BOOL CMPTI::IsMachineTypeDualSide()
{
	BOOL bIsDualsideMachine = FALSE;
	if (m_nDIffMachine == DiffMachineDualside || m_nDIffMachine == DiffMachineDualsideCI || m_nDIffMachine == DiffMachineMars)
		bIsDualsideMachine = TRUE;

	return bIsDualsideMachine;
}
BOOL MPTI_BinalizeGWire(InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, UCHAR* ucArrDstImg, int nMaskViewingMode, AlgoColorOpt &ptrAlgoColorOpt, bool bIsLoadFovRawDataImage)
{
	BOOL bRet = FALSE;
	if (g_pMPTI == NULL)
		return bRet;
	CPInsp_Algo &PInspAlgo = g_pMPTI->GetInspAlgo();


	AlgoGWire *pAlgo = (AlgoGWire *)sInspAlgo.m_ptrInspAlgoParam;

	PIAL::Insp_Image* pImg_buf = new PIAL::Insp_Image;
	PIAL::_AlgoBodyEdge algo2;

	PInspAlgoWrapper* pInspWarpper = g_pMPTI->GetPInspWrapper();
	//pInspWarpper->ConvertAlgo(pAlgo, algo2);

	int nImgWidth = sWndAlgoImg.m_nWidth;
	int nImgHeight = sWndAlgoImg.m_nHeight;
	pImg_buf->inspPartImage->nImgSizeX = sWndAlgoImg.m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sWndAlgoImg.m_nHeight3D;


	if (sWndAlgoImg.m_ucArrCV)
	{
		pImg_buf->inspWndImage->nImgSizeX = nImgWidth;
		pImg_buf->inspWndImage->nImgSizeY = nImgHeight;
		pImg_buf->m_p2D = new PIAL::PI_Buff(sWndAlgoImg.m_ucArrCV, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
	}
	if (sWndAlgoImg.m_fArr3D_part)
	{
		if (pImg_buf->inspWndImage->m_p3D)
			delete pImg_buf->inspWndImage->m_p3D;

		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
		pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(sWndAlgoImg.m_fArr3D_part, nImgWidth, nImgHeight, true);
	}
	pInspWarpper->MakeROIImg(&ptrAlgoColorOpt.m_sFovImg, pImg_buf->inspPartImage->m_ImageBuffer);

	PIAL::PI_Buff dst(ucArrDstImg, nImgWidth, nImgHeight, nImgWidth, 1, 1, true);
#ifdef _DEBUG
	cv::Mat tmp = pImg_buf->m_p2D->Mat();
#endif
	bRet = pInspWarpper->m_PInspAlgo->PInspAlgo::BinalizeBodyEdge(algo2, *pImg_buf, &dst);

	delete pImg_buf;

	Delete_1DArray(&sWndAlgoImg.m_ucArr2D);
	ptrAlgoColorOpt.Destroy();
	//LJH
	for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)
		Delete_1DArray(&sWndAlgoImg.m_ucArr2D_Mix[iLoopCount]);
	Delete_1DArray(&sWndAlgoImg.m_fArr3D);

	return bRet;
}

BOOL MPTI_SharedFlagCheck()
{

	return FALSE;
}
void CMPTI::SharedInit(int nAlgoToolIndex, int nMachineType, int * nToolType)
{
	ext::EnvVariable::nTool_id = m_InspMng->m_nToolIndex = nAlgoToolIndex;
	// Init 시 공유 메모리 할당 
	ext::StartInit::get()->Init();
	ext::ExtStartInitStruct * st = new ext::ExtStartInitStruct();
	ext::StartInit::get()->GetData(st);
	
	/*const int num = st->nForeignTool;*/
	unsigned long long bufSize = 0 ;
	bool bEnv = ext::EnvVariable::Init(nAlgoToolIndex, st->nToolCnt, _T("C:\\Eagle3D_64x\\PROGRAM\\AlgoTool\\AlgorithmTool.exe"),2,3,nMachineType, st->nBigPartTool, st->nForeignTool);
	bool bRoot = ext::irc::get()->Init();
	bool bClient = ext::ClientCtrl::get()->Init(nAlgoToolIndex, nMachineType);
	bool bLogic = ext::logic::get()->Init();
	bool bSer = ext::irs::get()->Init(nMachineType);
	*nToolType = ext::ClientCtrl::get()->m_nToolType;
	ext::MatDatas::get()->Init();
	//std::vector<int> groups[numGroups];
	//// 1~40 순회하며 그룹에 분배
	//for (int i = 0; i < EXT_BUFFER_FOREIGN_CNT; ++i) 
	//{
	//	int groupIndex = (i - 1) % numGroups;  // 0 ~ 3
	//	groups[groupIndex].push_back(i);
	//}

	bool bInit = bEnv == true && bClient == true && bLogic == true && bRoot  == true ? true : false;
	m_nExtMachineType = nMachineType;
	//init 실패시 서브프로세스 종료
	if (bInit == false)
	{
		ext::ClientCtrl::get()->_ctrl.SetSignal(ext::eSlvSignal::eSlvAlive, false);
		exit(0);
	}
}
BOOL MPTI_SharedInit(int nAlgoToolIndex ,int nMachineType , int * nToolType) //1 부터 시작할 Algo Index
{
	//g_pMPTI->m_InspMng->m_nToolIndex = nAlgoToolIndex;
	// ToTal Cnt Inspection Part 
	// 10 
	g_pMPTI->SharedInit(nAlgoToolIndex, nMachineType, nToolType);

	BCD_Open_Module(2048, 2048, true, 1, nAlgoToolIndex);
	//pixel Res X  , Y   FOV Size X , Y ; 
	//g_pMPTI->SetResolution(fovWidth, fovLength, resolX, resolY);


	return FALSE;
}
void MPTI_SharedExit()
{
	ext::logic::get()->Exit();
	ext::irc::get()->Exit();
	ext::EnvVariable::Exit();
	ext::StartInit::get()->Exit();
	ext::ClientCtrl::get()-> Exit();
}
BOOL MPTI_SharedInspection() //1 부터 시작할 Algo Index
{
	if(g_pMPTI->m_InspMng != NULL)
		g_pMPTI->m_InspMng->ShaerdInspection();
	return FALSE;
}
int CMPTI::SetSideFovMilAlloc(int* nFOVSizeX, int* nFOVSizeY)
{
	Im::PIL_ID *milsys; //= (Im::PIL_ID*)m_pMGrab->GetGrabSystem();
	int nFovX, nFovY;

	nFovX = m_digitSizeX;
	nFovY = m_digitSizeY;

	*nFOVSizeX = nFovX;
	*nFOVSizeY = nFovY;

	for (int i = 0; i < (int)3; i++)
	{
		if (m_bufSideDataFov[i] != M_NULL)
		{
			Im::Buf::Free(m_bufSideDataFov[i]);
			m_bufSideDataFov[i] = M_NULL;
		}

		Im::Buf::Alloc2d(*milsys, m_digitSizeX, m_digitSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &m_bufSideDataFov[i]);
		Im::Buf::Clear(m_bufSideDataFov[i], 0);
	}

	return eMR_SUCCESS;
}
bool CMPTI::IsForeignEnable(int nType)
{
	InspForeignInfo sFR = g_pInspMng->GetForeignData();
	if (nType >= m_eForeignInsp_Foreign && nType < m_eForeignInsp_Total)
	{
		return sFR.IsForeignEnable(nType);
	}
	else if (nType == m_eForeignInsp_Calc3D)
	{
		if (sFR.UseData(ZMAPFOV_TYPE_CALC_WARPAGE, true))
			return true;
		if (sFR.UseData(ZMAPFOV_TYPE_FOREIGN_PART, true))
			return true;
		if (sFR.UseData(ZMAPFOV_TYPE_INTERPOSER, true))
			return true;

		for (int nMode = m_eForeignInsp_Foreign; nMode < m_eForeignInsp_Total; nMode++)
		{
			if (sFR.IsForeignEnable(nMode) == false)
				continue;

			if (sFR.IsForeignEnable(nMode, m_eForeignInsp_3D))
				return true;
		}

		return false;
	}
	else
	{
		for (int nMode = m_eForeignInsp_Foreign; nMode < m_eForeignInsp_Total; nMode++)
		{
			if (sFR.IsForeignEnable(nMode) == false)
				continue;

			if (nType == m_eForeignInsp_Total)
				return true;
			else
			{
				if (sFR.IsForeignEnable(nMode, nType))
					return true;
			}
		}
	}
	return false;
}
void MPTI_AddLog(wchar_t* s)
{
	CString sLog;


	CString temp(s);
	sLog = temp;

	CString asd;
	asd.Format(_T("%s"), sLog);
	ext::Log::add(asd);

}
CProcMil* CMPTI::GetProcMil()
{
	InspManager *pMInspManager = GetMInspManager();

	if (!pMInspManager)
		return NULL;
	if (!pMInspManager->m_procMil)
		return NULL;
	return pMInspManager->m_procMil;
}
InspManager* CMPTI::GetMInspManager()
{
	if (!g_pMPTI)
		return NULL;
	return g_pMPTI->m_InspMng;
}
void CMPTI::RoiImageCompose_LT(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage, double dAngle)
{
	CProcMil *procMil = GetProcMil();
	if (!procMil)
		return;

	float fRedValue = (float)nRedValue / 100.0;
	float fGreenValue = (float)nGreenValue / 100.0;
	float fBlueValue = (float)nBlueValue / 100.0;
	float fWhiteValue = (float)nWhiteValue / 100.0;

	int nValueBuf = 0;
	float fRedImageValue = 0;
	float fGreenImageValue = 0;
	float fBlueImageValue = 0;
	float fWhiteImageValue = 0;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	int nMaxValue = nFullImageSize / (nFullImageWidth*nFullImageHeight);
	if (nMaxValue <= 0)	nMaxValue = 1;

	nRedValue = (int)(fRedValue * 128.0);
	nGreenValue = (int)(fGreenValue * 128.0);
	nBlueValue = (int)(fBlueValue * 128.0);
	nWhiteValue = (int)(fWhiteValue * 128.0);

	__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
	__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
	__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
	__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);
	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);


	for (int i = 0; i < nMaxValue; i++)
	{
		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * iptrbyResultImage = (&ptrbyResultImage[(y - nStartY)*nROIWidth]);
			int x = 0;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue;
				register __m128i fGreenImageValue;
				register __m128i fBlueImageValue;
				register __m128i fWhiteImageValue;

				if (!ptrbyRedBuffer)
					fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
				else
					fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyRedBuffer[FImgIdx]);
				if (!ptrbyGreenBuffer)
					fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
				else
					fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyGreenBuffer[FImgIdx]);
				if (!ptrbyBlueBuffer)
					fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
				else
					fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBlueBuffer[FImgIdx]);
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
				else
					fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyWhiteBuffer[FImgIdx]);

				_mm_storeu_si128(
					(__m128i *)(&iptrbyResultImage[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (!ptrbyRedBuffer)
					fRedImageValue = 0;
				else
					fRedImageValue = ptrbyRedBuffer[FImgIdx];
				if (!ptrbyGreenBuffer)
					fGreenImageValue = 0;
				else
					fGreenImageValue = ptrbyGreenBuffer[FImgIdx];
				if (!ptrbyBlueBuffer)
					fBlueImageValue = 0;
				else
					fBlueImageValue = ptrbyBlueBuffer[FImgIdx];
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = 0;
				else
					fWhiteImageValue = ptrbyWhiteBuffer[FImgIdx];

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				iptrbyResultImage[(x - nStartX)] = nValueBuf;
			}
		}
	}
	if (dAngle == 0)
		return;

	UCHAR* ptrSrc = ptrbyResultImage;
	UCHAR* ptrbyBuffer_ro = NULL;
	int retDstSizeX(0), retDstSizeY(0);
	int GapX = 0;
	int GapY = 0;
	if (ptrbyResultImage != NULL)
	{
		procMil->RotateImg_ipp(ptrbyResultImage, nROIWidth, nROIHeight, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);

		GapX = RounD((retDstSizeX - nROIWidth) / 2);
		GapY = RounD((retDstSizeY - nROIHeight) / 2);
		if ((GapX + nROIWidth) > retDstSizeX)
			GapX = 0;
		if ((GapY + nROIHeight) > retDstSizeY)
			GapY = 0;
		for (int r = 0; r < nROIHeight; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];
			UCHAR* dstPtr = &ptrSrc[r*nROIWidth];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*nROIWidth);
		}
		procMil->SaveWorkImg(ptrbyBuffer_ro, retDstSizeX, retDstSizeY, _T("ptrbyBuffer_ro Red.bmp"));
		procMil->SaveWorkImg(ptrSrc, nROIWidth, nROIHeight, _T("ptrbyRedBuffer.bmp"));

		if (ptrbyBuffer_ro != NULL)
			//delete [] ptrbyBuffer_ro;
			g_pMManager->pem_delete(ptrbyBuffer_ro, true);
	}

}
void CMPTI::FullImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage)
{
	CProcMil *procMil = g_pMPTI->GetProcMil();
	if (!procMil)
		return;

	int nWidth = sLightImg.m_nImgWidth;
	int nHeight = sLightImg.m_nImgHeight;
	int nImgCnt = sLightImg.m_nImgCnt;
	if ((nWidth <= 0) || (nHeight <= 0) || (nImgCnt <= 0))
		return;
	int nImageSize = nWidth * nHeight;

	UCHAR* ptrbyTopRedBuffer = sLightImg.m_pucTRed;
	UCHAR* ptrbyTopGreenBuffer = sLightImg.m_pucTGreen;
	UCHAR* ptrbyTopBlueBuffer = sLightImg.m_pucTBlue;
	UCHAR* ptrbyTopWhiteBuffer = sLightImg.m_pucTWhite;

	UCHAR* ptrbyMidRedBuffer = sLightImg.m_pucMRed;
	UCHAR* ptrbyMidGreenBuffer = sLightImg.m_pucMGreen;
	UCHAR* ptrbyMidBlueBuffer = sLightImg.m_pucMBlue;
	UCHAR* ptrbyMidWhiteBuffer = sLightImg.m_pucMWhite;

	UCHAR* ptrbyBotRedBuffer = sLightImg.m_pucBRed;
	UCHAR* ptrbyBotGreenBuffer = sLightImg.m_pucBGreen;
	UCHAR* ptrbyBotBlueBuffer = sLightImg.m_pucBBlue;
	UCHAR* ptrbyBotWhiteBuffer = sLightImg.m_pucBWhite;

	int* pnRedValue = sLightImg.m_pnRedValue;
	int* pnGreenValue = sLightImg.m_pnGreenValue;
	int* pnBlueValue = sLightImg.m_pnBlueValue;
	int* pnWhiteValue = sLightImg.m_pnWhiteValue;

	int *ptrnPosition = sLightImg.m_pnPosition;
	int *nCalculation = sLightImg.m_pnCalculation;
	if ((nCalculation == NULL) || (ptrbyResultImage == NULL))
		return;

	int nValueBuf = 0;
	float fRedImageValue = 0.0;
	float fGreenImageValue = 0.0;
	float fBlueImageValue = 0.0;
	float fWhiteImageValue = 0.0;

	float fRedValue = 0.0;
	float fGreenValue = 0.0;
	float fBlueValue = 0.0;
	float fWhiteValue = 0.0;

	UCHAR** ppucComposeImageBuf = NULL;
	//ppucComposeImageBuf = new UCHAR*[nImgCnt];
	ppucComposeImageBuf = g_pMManager->pem_new<UCHAR*>(true, nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	for (int i = 0; i < nImgCnt; i++)
	{
		//ppucComposeImageBuf[i] = new UCHAR[nImageSize];
		ppucComposeImageBuf[i] = g_pMManager->pem_new<UCHAR>(true, nImageSize, (PCHAR)__FUNCTION__, __LINE__);
	}


	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);

	for (int a = 0; a < nImgCnt; a++)
	{
		nValueBuf = 0;

		fRedValue = (double)pnRedValue[a] / 100.0;
		fGreenValue = (double)pnGreenValue[a] / 100.0;
		fBlueValue = (double)pnBlueValue[a] / 100.0;
		fWhiteValue = (double)pnWhiteValue[a] / 100.0;

		int nRedValue = (int)(fRedValue * 128.0);
		int nGreenValue = (int)(fGreenValue * 128.0);
		int nBlueValue = (int)(fBlueValue * 128.0);
		int nWhiteValue = (int)(fWhiteValue * 128.0);

		__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
		__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
		__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
		__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);

		for (int y = 0; y < nHeight; y++)
		{
			UCHAR * ippucComposeImageBuf = (UCHAR *)(&ppucComposeImageBuf[a][(y)*nWidth]);
			int x;
			for (x = 0; (x + 16) < nWidth; x += 16)
			{


				int FImgIdx = y * nWidth + x;

				register __m128i fRedImageValue = zeroBuf;
				register __m128i fGreenImageValue = zeroBuf;
				register __m128i fBlueImageValue = zeroBuf;
				register __m128i fWhiteImageValue = zeroBuf;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopRedBuffer[FImgIdx]);
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopGreenBuffer[FImgIdx]);
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopBlueBuffer[FImgIdx]);
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidRedBuffer[FImgIdx]);
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidGreenBuffer[FImgIdx]);
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidBlueBuffer[FImgIdx]);
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotRedBuffer[FImgIdx]);
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotGreenBuffer[FImgIdx]);
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotBlueBuffer[FImgIdx]);
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotWhiteBuffer[FImgIdx]);
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				_mm_storeu_si128(
					(__m128i *)(&ippucComposeImageBuf[(x)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < nWidth; x++)
			{
				int FImgIdx = y * nWidth + x;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyTopRedBuffer[FImgIdx];
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyTopGreenBuffer[FImgIdx];
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyTopBlueBuffer[FImgIdx];
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyTopWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyMidRedBuffer[FImgIdx];
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyMidGreenBuffer[FImgIdx];
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyMidBlueBuffer[FImgIdx];
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyMidWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyBotRedBuffer[FImgIdx];
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyBotGreenBuffer[FImgIdx];
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyBotBlueBuffer[FImgIdx];
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyBotWhiteBuffer[FImgIdx];
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				if (nValueBuf < 0)
					nValueBuf = 0;
				ippucComposeImageBuf[x] = nValueBuf;
			}

		}
		procMil->SaveWorkImg(ppucComposeImageBuf[a], nWidth, nHeight, _T("ComposeImageBuf.bmp"));
	}

	//UCHAR *pnImageValue = new UCHAR[nImageSize];
	UCHAR *pnImageValue = g_pMManager->pem_new<UCHAR>(true, nImageSize, (PCHAR)__FUNCTION__, __LINE__);
	int nImgValue = 0;
	for (int a = 0; a < nImgCnt; a++)
	{
		for (int b = 0; b < nImageSize; b++)
		{
			if (a == 0)
				nImgValue = ppucComposeImageBuf[a][b];
			else
			{
				if (nCalculation[a - 1] == 1)
					nImgValue = pnImageValue[b] + ppucComposeImageBuf[a][b];
				else if (nCalculation[a - 1] == 2)
					nImgValue = pnImageValue[b] - ppucComposeImageBuf[a][b];
			}

			if (nImgValue > 255)
				nImgValue = 255;
			if (nImgValue < 0)
				nImgValue = 0;

			pnImageValue[b] = nImgValue;
		}
		procMil->SaveWorkImg(pnImageValue, nWidth, nHeight, _T("ImageValue.bmp"));
	}
	for (int i = 0; i < nImageSize; i++)
	{
		ptrbyResultImage[i] = pnImageValue[i];
	}
	procMil->SaveWorkImg(ptrbyResultImage, nWidth, nHeight, _T("ResultImage.bmp"));
	if (pnImageValue)
	{
		//delete [] pnImageValue;
		g_pMManager->pem_delete(pnImageValue, true);
		pnImageValue = NULL;
	}
	if (ppucComposeImageBuf)
	{
		for (int i = 0; i < nImgCnt; i++)
		{
			//delete ppucComposeImageBuf[i];

			// MemoryLeak Fix -wjlee
			//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
			g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

			ppucComposeImageBuf[i] = NULL;
		}
		//delete [] ppucComposeImageBuf;
		g_pMManager->pem_delete(ppucComposeImageBuf, true);
		ppucComposeImageBuf = NULL;
	}
}
void CMPTI::ROIImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage, double dAngle)
{
	CProcMil *procMil = GetProcMil();
	if (!procMil)
		return;

	int nFullImageWidth = sLightImg.m_nImgWidth;
	int nFullImageHeight = sLightImg.m_nImgHeight;
	int nROIWidth = sLightImg.m_nROIImgWidth;
	int nROIHeight = sLightImg.m_nROIImgHeight;
	int nImgCnt = sLightImg.m_nImgCnt;
	if ((nFullImageWidth <= 0) || (nFullImageHeight <= 0) || (nImgCnt <= 0) || (nROIWidth <= 0) || (nROIHeight <= 0))
		return;
	double dROIX = sLightImg.m_dROIX;
	double dROIY = sLightImg.m_dROIY;

	UCHAR* ptrbyTopRedBuffer = sLightImg.m_pucTRed;
	UCHAR* ptrbyTopGreenBuffer = sLightImg.m_pucTGreen;
	UCHAR* ptrbyTopBlueBuffer = sLightImg.m_pucTBlue;
	UCHAR* ptrbyTopWhiteBuffer = sLightImg.m_pucTWhite;

	UCHAR* ptrbyMidRedBuffer = sLightImg.m_pucMRed;
	UCHAR* ptrbyMidGreenBuffer = sLightImg.m_pucMGreen;
	UCHAR* ptrbyMidBlueBuffer = sLightImg.m_pucMBlue;
	UCHAR* ptrbyMidWhiteBuffer = sLightImg.m_pucMWhite;

	UCHAR* ptrbyBotRedBuffer = sLightImg.m_pucBRed;
	UCHAR* ptrbyBotGreenBuffer = sLightImg.m_pucBGreen;
	UCHAR* ptrbyBotBlueBuffer = sLightImg.m_pucBBlue;
	UCHAR* ptrbyBotWhiteBuffer = sLightImg.m_pucBWhite;

	int* pnRedValue = sLightImg.m_pnRedValue;
	int* pnGreenValue = sLightImg.m_pnGreenValue;
	int* pnBlueValue = sLightImg.m_pnBlueValue;
	int* pnWhiteValue = sLightImg.m_pnWhiteValue;

	int *ptrnPosition = sLightImg.m_pnPosition;
	int *nCalculation = sLightImg.m_pnCalculation;
	if ((nCalculation == NULL) || (ptrbyResultImage == NULL))
		return;

	int nValueBuf = 0;
	float fRedImageValue = 0.0;
	float fGreenImageValue = 0.0;
	float fBlueImageValue = 0.0;
	float fWhiteImageValue = 0.0;

	float fRedValue = 0.0;
	float fGreenValue = 0.0;
	float fBlueValue = 0.0;
	float fWhiteValue = 0.0;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	int nImageSize = nROIWidth * nROIHeight;
	UCHAR** ppucComposeImageBuf = NULL;
	//ppucComposeImageBuf = new UCHAR*[nImgCnt];
	ppucComposeImageBuf = g_pMManager->pem_new<UCHAR*>(true, nImgCnt, (PCHAR)__FUNCTION__, __LINE__);
	for (int i = 0; i < nImgCnt; i++)
	{
		//ppucComposeImageBuf[i] = new UCHAR[nImageSize];
		ppucComposeImageBuf[i] = g_pMManager->pem_new<UCHAR>(true, nImageSize, (PCHAR)__FUNCTION__, __LINE__);
	}

	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);

	for (int a = 0; a < nImgCnt; a++)
	{
		nROIIndex = 0;
		nValueBuf = 0;

		fRedValue = (double)pnRedValue[a] / 100.0;
		fGreenValue = (double)pnGreenValue[a] / 100.0;
		fBlueValue = (double)pnBlueValue[a] / 100.0;
		fWhiteValue = (double)pnWhiteValue[a] / 100.0;

		int nRedValue = (int)(fRedValue * 128.0);
		int nGreenValue = (int)(fGreenValue * 128.0);
		int nBlueValue = (int)(fBlueValue * 128.0);
		int nWhiteValue = (int)(fWhiteValue * 128.0);

		__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
		__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
		__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
		__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);

		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * ippucComposeImageBuf = (UCHAR *)(&ppucComposeImageBuf[a][(y - nStartY)*nROIWidth]);
			int x;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue = zeroBuf;
				register __m128i fGreenImageValue = zeroBuf;
				register __m128i fBlueImageValue = zeroBuf;
				register __m128i fWhiteImageValue = zeroBuf;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopRedBuffer[FImgIdx]);
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopGreenBuffer[FImgIdx]);
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopBlueBuffer[FImgIdx]);
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidRedBuffer[FImgIdx]);
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidGreenBuffer[FImgIdx]);
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidBlueBuffer[FImgIdx]);
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotRedBuffer[FImgIdx]);
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotGreenBuffer[FImgIdx]);
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotBlueBuffer[FImgIdx]);
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotWhiteBuffer[FImgIdx]);
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				_mm_storeu_si128(
					(__m128i *)(&ippucComposeImageBuf[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyTopRedBuffer[FImgIdx];
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyTopGreenBuffer[FImgIdx];
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyTopBlueBuffer[FImgIdx];
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyTopWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyMidRedBuffer[FImgIdx];
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyMidGreenBuffer[FImgIdx];
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyMidBlueBuffer[FImgIdx];
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyMidWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyBotRedBuffer[FImgIdx];
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyBotGreenBuffer[FImgIdx];
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyBotBlueBuffer[FImgIdx];
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyBotWhiteBuffer[FImgIdx];
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						for (int i = 0; i < nImgCnt; i++)
						{
							//delete ppucComposeImageBuf[i];

							// MemoryLeak Fix -wjlee
							//g_pMManager->pem_delete(ppucComposeImageBuf[i], false);
							g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

							ppucComposeImageBuf[i] = NULL;
						}
						//delete [] ppucComposeImageBuf;
						g_pMManager->pem_delete(ppucComposeImageBuf, true);
						ppucComposeImageBuf = NULL;
					}
					return;
				}

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				if (nValueBuf < 0)
					nValueBuf = 0;
				ippucComposeImageBuf[x - nStartX] = nValueBuf;
			}

		}
		procMil->SaveWorkImg(ppucComposeImageBuf[a], nROIWidth, nROIHeight, _T("ComposeImageBuf.bmp"));
	}

	int nImgValue = 0;
	for (int b = 0; b < nImageSize; b++)
	{
		for (int a = 0; a < nImgCnt; a++)
		{
			if (a == 0)
				nImgValue = ppucComposeImageBuf[a][b];
			else
			{
				if (nCalculation[a - 1] == 1)
					nImgValue = ptrbyResultImage[b] + ppucComposeImageBuf[a][b];
				else if (nCalculation[a - 1] == 2)
					nImgValue = ptrbyResultImage[b] - ppucComposeImageBuf[a][b];
			}

			if (nImgValue > 255)
				nImgValue = 255;
			if (nImgValue < 0)
				nImgValue = 0;

			ptrbyResultImage[b] = nImgValue;
		}
	}
	procMil->SaveWorkImg(ptrbyResultImage, nROIWidth, nROIHeight, _T("pnImageValue.bmp"));

	if (ppucComposeImageBuf)
	{
		for (int i = 0; i < nImgCnt; i++)
		{
			// MemoryLeak Fix -wjlee
			g_pMManager->pem_delete(ppucComposeImageBuf[i], true);

			ppucComposeImageBuf[i] = NULL;
		}
		//delete [] ppucComposeImageBuf;
		g_pMManager->pem_delete(ppucComposeImageBuf, true);
		ppucComposeImageBuf = NULL;
	}
	if (dAngle == 0)
		return;

	//shkim rotate
	UCHAR* ptrSrc = ptrbyResultImage;
	UCHAR* ptrbyBuffer_ro = NULL;
	int retDstSizeX(0), retDstSizeY(0);
	int GapX = 0;
	int GapY = 0;
	if (ptrbyResultImage != NULL)
	{
		procMil->RotateImg_ipp(ptrbyResultImage, nROIWidth, nROIHeight, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);

		GapX = RounD((retDstSizeX - nROIWidth) / 2);
		GapY = RounD((retDstSizeY - nROIHeight) / 2);
		if ((GapX + nROIWidth) > retDstSizeX)
			GapX = 0;
		if ((GapY + nROIHeight) > retDstSizeY)
			GapY = 0;
		for (int r = 0; r < nROIHeight; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];
			UCHAR* dstPtr = &ptrSrc[r*nROIWidth];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*nROIWidth);
		}
		procMil->SaveWorkImg(ptrbyBuffer_ro, retDstSizeX, retDstSizeY, _T("ptrbyBuffer_ro Red.bmp"));
		procMil->SaveWorkImg(ptrSrc, nROIWidth, nROIHeight, _T("ptrbyRedBuffer.bmp"));

		if (ptrbyBuffer_ro != NULL)
			//delete [] ptrbyBuffer_ro;
			g_pMManager->pem_delete(ptrbyBuffer_ro, true);
	}
}

void CMPTI::FrameWriterInit()
{
	ext::FrameFlagWriter::get()->Init();
	ext::FrameWriters::get()->Init();
	bool bLogic = ext::logic::get()->Init(500); 

}
void MPTI_FrameWriterInit()
{
	g_pMPTI->FrameWriterInit();
}
//void CMPTI::FrameWriter()
//{
//
//}
//void MPTI_FrameWriter()
//{
//
//}

void MPTI_Set2DChannel_LightPos(int nR, int nG, int nB)
{
	if (g_pMPTI == NULL) return;

	g_pMPTI->m_nRedLgtPos_2DImg = nR;
	g_pMPTI->m_nGreenLgtPos_2DImg = nG;
	g_pMPTI->m_nBlueLgtPos_2DImg = nB;
}

void CMPTI::POCR_UseAI_Set(int nInspAIIdx, bool bInspOne, bool bUseAIModule, int nAIOption, int nThreshold)
{
	if (g_pMPTI)
	{
		g_pMPTI->m_nlnspIndexAI = nInspAIIdx;
		g_pMPTI->bInspOne = bInspOne;
		g_pMPTI->m_bUseAIModule = bUseAIModule;
		g_pMPTI->m_nAIOption = nAIOption;
		g_pMPTI->m_SegThreshold = nThreshold;
	}
}
