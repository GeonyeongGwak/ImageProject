#include "stdafx.h"

#include "InspManager.h"
#include "ImgLoadSave.h"

//#include "mil.h"
//#pragma comment(lib, "mil.lib")
#include "ProcMil.h"
//#include "MGrab.h"


#ifndef _ALP_ALLOC_H_
#include "alp_alloc.h"
#endif

#include <math.h>
#include "MemoryManager.h"


#define	LMTI(i)	(i < 0.f) ? 0.f : ((i > 255.f) ? 255.f : i)
#define MPTT(r,g,b,rf,gf,bf)	( ((int)(LMTI((float)r*rf)) << 16) | ((int)(LMTI((float)g*gf)) << 8) | ((int)(LMTI((float)b*bf))) )

CImgLoadSave *g_pImageFileLoadSave = NULL;


BOOL ImageInit(wchar_t* filePath, UINT nFovSizeX, UINT nFovSizeY)
{
	if(g_pImageFileLoadSave)
		ImageDestroy();

	//g_pImageFileLoadSave = new CImgLoadSave;
	g_pImageFileLoadSave = g_pMManager->pem_new<CImgLoadSave>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	return g_pImageFileLoadSave->Initial(filePath, nFovSizeX, nFovSizeY);
}
BOOL ImageDestroy()
{
	//delete g_pImageFileLoadSave;
	g_pMManager->pem_delete(g_pImageFileLoadSave, false);
	g_pImageFileLoadSave = NULL;

	return TRUE;
}

int ImgLoadBmp(UCHAR* &pDest, wchar_t* filePath, int &nWidth, int &nHeight )
{
	//if(pDest == NULL) return -1;

	if(g_pImageFileLoadSave == NULL)
		return -1;
	MIL_ID *pMilApp = (MIL_ID*)g_pImageFileLoadSave->GetGrabApplication();
	MIL_ID *pMilSys = (MIL_ID*)g_pImageFileLoadSave->GetGrabSystem();
	if((pMilApp == M_NULL) || (pMilSys == M_NULL))
		return -1;

	CString csFilePath;
	csFilePath.Format(_T("%s"),filePath);

	CString tmpPath;
	tmpPath = csFilePath;
	tmpPath.MakeLower();

	if((tmpPath.Right(4) == _T(".bmp")) == FALSE)
	{
		csFilePath += _T(".bmp");
	}

	csFilePath.Format(_T("%s"),filePath);


	MIL_ID idMilImage = M_NULL;

	//idMilImage = m_milProc.AllocBuff(nWidth, nHeight, 0);
	BOOL bAlloc = FALSE;
	if(pDest == NULL)
	{
		//pDest= new UCHAR[(nWidth * nHeight)*3];
		  //pDest = (UCHAR*)LocalAlloc(LPTR, sizeof(UCHAR)*(nWidth)*(nHeight)*3);
		pDest = (UCHAR*)g_pMManager->pem_LocalAlloc(LPTR, sizeof(UCHAR)*(nWidth)*(nHeight) * 3, (PCHAR)__FUNCTION__, __LINE__);
		  bAlloc = TRUE;
	}

	//MbufLoad(csFilePath.GetBuffer(0), (MIL_ID)idMilImage);
	//m_milProc.LoadImgProcBuf(&idMilImage, csFilePath.GetBuffer(0));
	MbufRestore(csFilePath, *pMilSys, &idMilImage);
	MbufGet2d((MIL_ID)idMilImage, 0, 0, nWidth, nHeight, pDest);

	MbufExport(_T("D:\\test.bmp"), M_BMP, idMilImage);
	
	if(idMilImage != M_NULL)
		MbufFree(idMilImage);
	idMilImage = M_NULL;

	/*if(bAlloc)
		LocalFree(pDest);*/

	return 0;
}

int ImgLoadTif(UCHAR* &pDest, wchar_t* filePath, int &nWidth, int &nHeight )
{
	if(g_pImageFileLoadSave == NULL)
		return -1;
	MIL_ID *pMilApp = (MIL_ID*)g_pImageFileLoadSave->GetGrabApplication();
	MIL_ID *pMilSys = (MIL_ID*)g_pImageFileLoadSave->GetGrabSystem();
	if((pMilApp == M_NULL) || (pMilSys == M_NULL))
		return -1;

	CString csFilePath;
	csFilePath.Format(_T("%s"),filePath);

	CString tmpPath;
	tmpPath = csFilePath;
	tmpPath.MakeLower();

	//if((tmpPath.Right(4) == _T(".tif")) == FALSE)
	if((tmpPath.Right(4) == _T(".jpg")) == FALSE)		//jpg 읽어오기
	{
		//csFilePath += _T(".tif");
		csFilePath += _T(". jpg");
	}

	csFilePath.Format(_T("%s"),filePath);


	MIL_ID idMilImage = NULL;
	//MbufRestore(csFilePath, *pMilSys, &idMilImage);
	//TCHAR* tmp = new TCHAR[ csFilePath.GetLength() + 1 ];
	TCHAR* tmp = g_pMManager->pem_new<TCHAR>(true, csFilePath.GetLength() + 1, (PCHAR)__FUNCTION__, __LINE__);
	_tcscpy( tmp, csFilePath.GetBuffer() );
	tmp[ csFilePath.GetLength() ] = 0;

	MbufImport(tmp, M_DEFAULT, M_RESTORE, *pMilSys,&idMilImage);
	//delete[] tmp;
	g_pMManager->pem_delete(tmp, true);

	nWidth	= MbufInquire(idMilImage, M_SIZE_X, M_NULL);
	nHeight = MbufInquire(idMilImage, M_SIZE_Y, M_NULL);

	if(pDest == NULL)
		//pDest = (UCHAR*)LocalAlloc(LPTR, sizeof(UCHAR)*(nWidth)*(nHeight));
		pDest = (UCHAR*)g_pMManager->pem_LocalAlloc(LPTR, sizeof(UCHAR)*(nWidth)*(nHeight), (PCHAR)__FUNCTION__, __LINE__);

	//MbufLoad(csFilePath, idMilImage);
	//MbufLoad(csFilePath.GetBuffer(0), (MIL_ID)idMilImage);
	//milProc.LoadImgProcBuf(&idMilImage, csFilePath.GetBuffer(0));

	MbufGet2d((MIL_ID)idMilImage, 0, 0, nWidth, nHeight, pDest);

	if(idMilImage != M_NULL)
		MbufFree(idMilImage);
	idMilImage = M_NULL;

	return M_SUCCESS;
}

int ImgSaveTif(UCHAR *pDest, wchar_t* filePath, int nWidth, int nHeight )
{
	if(pDest == NULL) return -1;
	if((nWidth <= 0) || (nHeight <= 0))
		return -1;

	if(g_pImageFileLoadSave == NULL)
		return -1;
	MIL_ID *pMilApp = (MIL_ID*)g_pImageFileLoadSave->GetGrabApplication();
	MIL_ID *pMilSys = (MIL_ID*)g_pImageFileLoadSave->GetGrabSystem();
	if((pMilApp == M_NULL) || (pMilSys == M_NULL))
		return -1;

	//MIL_ID idMilImage = M_NULL;
	//idMilImage = milProc.AllocBuff(nWidth, nHeight, 0);

	//milProc.SaveBuff(idMilImage,filePath);
	int nBand = 1;


	//if(idMilImage != M_NULL)
	//	MbufFree(idMilImage);
	//idMilImage = M_NULL;

	return M_SUCCESS;
}


int ImgLoadBuk(UCHAR* &pDest, wchar_t* filePath, int &nWidth, int &nHeight)
{
	//return alpf_load_buk(filePath, pDest);

	//if(pDest == NULL) return -1;

	CString csFilePath;
	csFilePath.Format(_T("%s"),filePath);


	CString tmpPath;
	tmpPath = csFilePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".buk")) == FALSE)
	{
		csFilePath += _T(".buk");
	}

	CFile file;
	file.Open(csFilePath, CFile::modeRead, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::load);

		HEADER_BUK header;
		ar >> header.startX;
		ar >> header.startY;
		ar >> header.sizeX;
		ar >> header.sizeY;
		ar >> header.camFovX;
		ar >> header.camFovY;
		ar >> header.sizeBit;

		nWidth = (int)header.sizeX;
		nHeight = (int)header.sizeY;

		// 할당
		if(pDest == NULL)
		{
			//pDest = new UCHAR[nWidth * nHeight];
			//pDest = (UCHAR*)LocalAlloc(LPTR, sizeof(UCHAR)*(nWidth)*(nHeight));
			pDest = (UCHAR*)g_pMManager->pem_LocalAlloc(LPTR, sizeof(UCHAR)*(nWidth)*(nHeight), (PCHAR)__FUNCTION__, __LINE__);
		}

		for(int y=0 ; y<nHeight ; y++)
		{
			for(int x=0 ; x<nWidth ; x++)
			{
				ar >> pDest[x+(y*nWidth)];
			}
		}

		ar.Close();
		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}


int ImgSaveBuk(UCHAR *pSrc, wchar_t* filePath, LP_HEADER_BUK pHeader)
{
	return alpf_save_buk(filePath, pHeader, pSrc);
}

//20140901 SHW ptt load 작성
int ImgAllLoadPtt(float* &ptrfVertexColor, float* &ptrfvRawData, wchar_t* filePath, int &nWidth, int &nHeight, LP_HEADER_PTT pHeader )
{
	if(!pHeader)
		return FALSE;

	CString pszFileName;
	pszFileName.Format(_T("%s"),filePath);

	CFile myFile;
	CFileException fileException;
	if ( !myFile.Open( pszFileName, CFile::modeRead, &fileException ) )
	{
		AfxMessageBox(_T("Ptt file open Error"));
		return FALSE;
	}

	CArchive ar( &myFile,CArchive::load );

	UINT nRow = 0;
	UINT nColumn = 0;

	/*CPri_Header	*pPriHeader = new CPri_Header;
	CPtr_Header	*pPtrHeader = new CPtr_Header;*/
	CPri_Header	*pPriHeader = g_pMManager->pem_new<CPri_Header>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	CPtr_Header	*pPtrHeader = g_pMManager->pem_new<CPtr_Header>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	ar >> nRow;
	ar >> nColumn;
	ar >> pPtrHeader->fResol_x;
	ar >> pPtrHeader->fResol_y;
	ar >> pPtrHeader->nBit;
	ar >> pPtrHeader->nGap_x;
	ar >> pPtrHeader->nGap_y;
	ar >> pPtrHeader->nUnit;
	{
		pHeader->uiNumCol	 = nRow				;
		pHeader->uiNumRow	 = nColumn				;
		pHeader->zResolX	 = pPtrHeader->fResol_x;
		pHeader->zResolY	 = pPtrHeader->fResol_y;
		pHeader->sizeBit	 = pPtrHeader->nBit	;
		pHeader->gabX		 = pPtrHeader->nGap_x	;
		pHeader->gabY		 = pPtrHeader->nGap_y	;
		pHeader->sizeUnit	 = pPtrHeader->nUnit	;
	}

	bool rodd = FALSE;
	bool codd = FALSE;

	if( (nRow % 2) != 0 )
	{
		pPtrHeader->nNumRow = nRow - 1;
		rodd = TRUE;
	}
	else
		pPtrHeader->nNumRow = nRow;

	if( (nColumn % 2) != 0 )
	{
		pPtrHeader->nNumColumn = nColumn - 1;
		codd = TRUE;
	}
	else
		pPtrHeader->nNumColumn = nColumn;

	int NumOfRangeY = (int)pPtrHeader->nNumRow / 2;
	int NumOfRangeX = (int)pPtrHeader->nNumColumn / 2;

	int	totalData = 4 * NumOfRangeX * NumOfRangeY;

	if( ptrfvRawData != NULL )
		//delete[] ptrfvRawData;
		g_pMManager->pem_delete(ptrfvRawData, true);

	//ptrfvRawData = new float[totalData];
	//ptrfvRawData = (float*)LocalAlloc(LPTR, sizeof(float)*totalData);
	ptrfvRawData = (float*)g_pMManager->pem_LocalAlloc(LPTR, sizeof(float)*totalData, (PCHAR)__FUNCTION__, __LINE__);

	int index = 0;
	int count = 0;
	int	xrange = pPtrHeader->nNumColumn;
	int bit = pPtrHeader->nBit;
	int Increase = 1;	 // 증가치( 변경 하면 안됨 )

	if( bit == 16)
	{
		short temp;
		for( int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase )
		{
			for( int x = -NumOfRangeX; x < NumOfRangeX; x += Increase )
			{
				ar >> temp;

				ptrfvRawData[count] = (float)temp;

				count++;
				index += 3;

				if( codd )
					if( (count % xrange) == 0 )
						ar >> temp;
			}
		}

		if( rodd )
		{
			if( codd )
			{
				for( int i = 0; i < xrange + 1; i++ )
					ar >> temp;
			}
			else
			{
				for( int i = 0; i < xrange; i++ )
					ar >> temp;
			}
		}
	}
	else if(bit == 32)
	{
		float temp;
		for( int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase )
		{
			for( int x = -NumOfRangeX; x < NumOfRangeX; x += Increase )
			{
				ar >> temp;

				ptrfvRawData[count] = (float)temp;

				count++;
				index += 3;

				if( codd )
					if( (count % xrange) == 0 )
						ar >> temp;
			}
		}

		if( rodd )
		{
			if( codd )
			{
				for( int i = 0; i < xrange + 1; i++ )
					ar >> temp;
			}
			else
			{
				for( int i = 0; i < xrange; i++ )
					ar >> temp;
			}
		}
	}
	else if(bit == 64)
	{
		double temp;
		for( int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase )
		{
			for( int x = -NumOfRangeX; x < NumOfRangeX; x += Increase )
			{
				ar >> temp;

				ptrfvRawData[count] = (float)temp;

				count++;
				index += 3;

				if( codd )
					if( (count % xrange) == 0 )
						ar >> temp;
			}
		}

		if( rodd )
		{
			if( codd )
			{
				for( int i = 0; i < xrange + 1; i++ )
					ar >> temp;
			}
			else
			{
				for( int i = 0; i < xrange; i++ )
					ar >> temp;
			}
		}
	}
	else
	{
		short temp;
		for( int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase )
		{
			for( int x = -NumOfRangeX; x < NumOfRangeX; x += Increase )
			{
				ar >> temp;

				ptrfvRawData[count] = (float)temp;

				count++;
				index += 3;

				if( codd )
					if( (count % xrange) == 0 )
						ar >> temp;
			}
		}

		if( rodd )
		{
			if( codd )
			{
				for( int i = 0; i < xrange + 1; i++ )
					ar >> temp;
			}
			else
			{
				for( int i = 0; i < xrange; i++ )
					ar >> temp;
			}
		}
	}

	float fRow = 0;
	float fColumn = 0;

	ar >> fRow;
	ar >> fColumn;
	ar >> pPriHeader->fSt_Col;
	ar >> pPriHeader->fSt_Row;
	ar >> pPriHeader->fShiftX;
	ar >> pPriHeader->fShiftY;
	ar >> pPriHeader->fWidth;
	ar >> pPriHeader->fLength;
	ar >> pPriHeader->fResol_x;
	ar >> pPriHeader->fResol_y;
	ar >> pPriHeader->fDEPTH;


	{
		pHeader->fNumCol		= fRow				;
		pHeader->fNumRow		= fColumn				;
		pHeader->targetStartX	= pPriHeader->fSt_Col	;
		pHeader->targetStartY	= pPriHeader->fSt_Row	;
		pHeader->targetShiftX	= pPriHeader->fShiftX	;
		pHeader->targetShiftY	= pPriHeader->fShiftY	;
		pHeader->targetWidth	= pPriHeader->fWidth	;
		pHeader->targetHeight	= pPriHeader->fLength	;
		pHeader->pixelResX		= pPriHeader->fResol_x;
		pHeader->pixelResY		= pPriHeader->fResol_y;
		pHeader->depthBit		= pPriHeader->fDEPTH	;
	}

	if( ((int)fRow % 2) != 0 )
	{
		pPriHeader->fNumRow = fRow - 1;
		rodd = TRUE;
	}

	if( ((int)fColumn % 2) != 0 )
	{
		pPriHeader->fNumColumn = fColumn - 1;
		codd = TRUE;
	}

	nWidth	= pPtrHeader->nNumColumn;
	nHeight	= pPtrHeader->nNumRow;

	if( ptrfVertexColor == NULL )
	{
		//ptrfVertexColor = new float[totalData*3];
		//ptrfVertexColor = (float*)LocalAlloc(LPTR, sizeof(float)*totalData*3);
		ptrfVertexColor = (float*)g_pMManager->pem_LocalAlloc(LPTR, sizeof(float)*totalData * 3, (PCHAR)__FUNCTION__, __LINE__);
	}

	xrange = pPriHeader->fNumColumn;
	float DEPTH = pPriHeader->fDEPTH;

	BOOL bLoadComplete = TRUE;
	if( abs(DEPTH - 8) < 0.1f )
	{
		int	nIndex = 0;
		UCHAR tempData;
		for(int j=0;j<totalData;j++)
		{	
			ar >> tempData;

			ptrfVertexColor[nIndex] = (float)tempData / 255;
			ptrfVertexColor[nIndex + 1] = (float)tempData / 255;
			ptrfVertexColor[nIndex + 2] = (float)tempData / 255;

			nIndex += 3;

			if( codd )
				if( ((j+1) % xrange) == 0 )
					ar >> tempData;
		}
	}
	else if( abs(DEPTH - 16) < 0.1f )
	{
		int	nIndex = 0;
		short tempData;
		for(int j=0;j<totalData;j++)
		{	
			ar >> tempData;

			if( codd )
				if( ((j+1) % xrange) == 0 )
					ar >> tempData;

			UCHAR red = tempData>>8;
			UCHAR blue = (UCHAR)tempData;
			UINT green = UINT((float)red * 0.4f + (float)blue * 0.8f);

			if( green > 255 ) green = 255;

			ptrfVertexColor[nIndex + 0] = (float)red / 255;
			ptrfVertexColor[nIndex + 1] = (float)green / 255;
			ptrfVertexColor[nIndex + 2] = (float)blue / 255;

			nIndex += 3;
		}
	}
	else if( abs(DEPTH - 32) < 0.1f )
	{
		int	nIndex = 0;
		UINT tempData;
		for(int j=0;j<totalData;j++)
		{	
			ar >> tempData;

			if( codd )
				if( ((j+1) % xrange) == 0 )
					ar >> tempData;

			UCHAR red	= tempData >> 16;
			UCHAR green	= tempData >> 8; 
			UCHAR blue	= (UCHAR)tempData;

			ptrfVertexColor[nIndex + 0] = (float)red / 255;
			ptrfVertexColor[nIndex + 1] = (float)green / 255;
			ptrfVertexColor[nIndex + 2] = (float)blue / 255;

			nIndex += 3;
		}
	}
	else
	{
		if(pPriHeader)
		{
			//delete pPriHeader;
			g_pMManager->pem_delete(pPriHeader, false);
			pPriHeader = NULL;
		}
		if(pPtrHeader)
		{
			//delete pPtrHeader;
			g_pMManager->pem_delete(pPtrHeader, false);
			pPtrHeader = NULL;
		}

		AfxMessageBox(_T("DEPTH Error"));
		bLoadComplete = FALSE;
	}

	ar.Close();
	myFile.Close();

	if(pPriHeader)
	{
		//delete pPriHeader;
		g_pMManager->pem_delete(pPriHeader, false);
		pPriHeader = NULL;
	}
	if(pPtrHeader)
	{
		//delete pPtrHeader;
		g_pMManager->pem_delete(pPtrHeader, false);
		pPtrHeader = NULL;
	}

	return bLoadComplete;
}


int ImgLoadPtt(float* &ptrfVertexColorRed, float* &ptrfVertexColorGreend, float* &ptrfVertexColorBlue,
												float* &ptrfvRawData, wchar_t* filePath, int &nWidth, int &nHeight )
{
	CString pszFileName;
	pszFileName.Format(_T("%s"),filePath);

	CFile myFile;
	CFileException fileException;
	if ( !myFile.Open( pszFileName, CFile::modeRead, &fileException ) )
	{
		AfxMessageBox(_T("Ptt file open Error"));
		return FALSE;
	}

	CArchive ar( &myFile,CArchive::load );

	UINT nRow = 0;
	UINT nColumn = 0;

	/*CPri_Header	*pPriHeader = new CPri_Header;
	CPtr_Header	*pPtrHeader = new CPtr_Header;*/
	CPri_Header	*pPriHeader = g_pMManager->pem_new<CPri_Header>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	CPtr_Header	*pPtrHeader = g_pMManager->pem_new<CPtr_Header>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

	ar >> nRow;
	ar >> nColumn;
	ar >> pPtrHeader->fResol_x;
	ar >> pPtrHeader->fResol_y;
	ar >> pPtrHeader->nBit;
	ar >> pPtrHeader->nGap_x;
	ar >> pPtrHeader->nGap_y;
	ar >> pPtrHeader->nUnit;

	bool rodd = FALSE;
	bool codd = FALSE;

	if( (nRow % 2) != 0 )
	{
		pPtrHeader->nNumRow = nRow - 1;
		rodd = TRUE;
	}
	else
		pPtrHeader->nNumRow = nRow;

	if( (nColumn % 2) != 0 )
	{
		pPtrHeader->nNumColumn = nColumn - 1;
		codd = TRUE;
	}
	else
		pPtrHeader->nNumColumn = nColumn;

	int NumOfRangeY = (int)pPtrHeader->nNumRow / 2;
	int NumOfRangeX = (int)pPtrHeader->nNumColumn / 2;

	int	totalData = 4 * NumOfRangeX * NumOfRangeY;

	if( ptrfvRawData != NULL )
		//delete[] ptrfvRawData;
		g_pMManager->pem_delete(ptrfvRawData, true);

	//ptrfvRawData = new float[totalData];
	ptrfvRawData = g_pMManager->pem_new<float>(true, totalData, (PCHAR)__FUNCTION__, __LINE__);

	int index = 0;
	int count = 0;
	int	xrange = pPtrHeader->nNumColumn;
	int bit = pPtrHeader->nBit;
	int Increase = 1;	 // 증가치( 변경 하면 안됨 )

	if( bit == 16)
	{
		short temp;
		for( int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase )
		{
			for( int x = -NumOfRangeX; x < NumOfRangeX; x += Increase )
			{
				ar >> temp;

				ptrfvRawData[count] = (float)temp;

				count++;
				index += 3;

				if( codd )
					if( (count % xrange) == 0 )
						ar >> temp;
			}
		}

		if( rodd )
		{
			if( codd )
			{
				for( int i = 0; i < xrange + 1; i++ )
					ar >> temp;
			}
			else
			{
				for( int i = 0; i < xrange; i++ )
					ar >> temp;
			}
		}
	}
	else if(bit == 32)
	{
		float temp;
		for( int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase )
		{
			for( int x = -NumOfRangeX; x < NumOfRangeX; x += Increase )
			{
				ar >> temp;

				ptrfvRawData[count] = (float)temp;

				count++;
				index += 3;

				if( codd )
					if( (count % xrange) == 0 )
						ar >> temp;
			}
		}

		if( rodd )
		{
			if( codd )
			{
				for( int i = 0; i < xrange + 1; i++ )
					ar >> temp;
			}
			else
			{
				for( int i = 0; i < xrange; i++ )
					ar >> temp;
			}
		}
	}
	else if(bit == 64)
	{
		double temp;
		for( int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase )
		{
			for( int x = -NumOfRangeX; x < NumOfRangeX; x += Increase )
			{
				ar >> temp;

				ptrfvRawData[count] = (float)temp;

				count++;
				index += 3;

				if( codd )
					if( (count % xrange) == 0 )
						ar >> temp;
			}
		}

		if( rodd )
		{
			if( codd )
			{
				for( int i = 0; i < xrange + 1; i++ )
					ar >> temp;
			}
			else
			{
				for( int i = 0; i < xrange; i++ )
					ar >> temp;
			}
		}
	}
	else
	{
		short temp;
		for( int y = NumOfRangeY; y > -NumOfRangeY; y -= Increase )
		{
			for( int x = -NumOfRangeX; x < NumOfRangeX; x += Increase )
			{
				ar >> temp;

				ptrfvRawData[count] = (float)temp;

				count++;
				index += 3;

				if( codd )
					if( (count % xrange) == 0 )
						ar >> temp;
			}
		}

		if( rodd )
		{
			if( codd )
			{
				for( int i = 0; i < xrange + 1; i++ )
					ar >> temp;
			}
			else
			{
				for( int i = 0; i < xrange; i++ )
					ar >> temp;
			}
		}
	}

	float fRow = 0;
	float fColumn = 0;

	ar >> fRow;
	ar >> fColumn;
	ar >> pPriHeader->fSt_Col;
	ar >> pPriHeader->fSt_Row;
	ar >> pPriHeader->fShiftX;
	ar >> pPriHeader->fShiftY;
	ar >> pPriHeader->fWidth;
	ar >> pPriHeader->fLength;
	ar >> pPriHeader->fResol_x;
	ar >> pPriHeader->fResol_y;
	ar >> pPriHeader->fDEPTH;


	if( ((int)fRow % 2) != 0 )
	{
		pPriHeader->fNumRow = fRow - 1;
		rodd = TRUE;
	}

	if( ((int)fColumn % 2) != 0 )
	{
		pPriHeader->fNumColumn = fColumn - 1;
		codd = TRUE;
	}

	nWidth	= pPtrHeader->nNumColumn;
	nHeight	= pPtrHeader->nNumRow;

	/*if( ptrfVertexColorRed == NULL )
		ptrfVertexColorRed = new float[totalData];
	if( ptrfVertexColorGreend == NULL )
		ptrfVertexColorGreend = new float[totalData];
	if( ptrfVertexColorBlue == NULL )
		ptrfVertexColorBlue = new float[totalData];*/
	if (ptrfVertexColorRed == NULL)
		ptrfVertexColorRed = g_pMManager->pem_new<float>(true, totalData, (PCHAR)__FUNCTION__, __LINE__);
	if (ptrfVertexColorGreend == NULL)
		ptrfVertexColorGreend = g_pMManager->pem_new<float>(true, totalData, (PCHAR)__FUNCTION__, __LINE__);
	if (ptrfVertexColorBlue == NULL)
		ptrfVertexColorBlue = g_pMManager->pem_new<float>(true, totalData, (PCHAR)__FUNCTION__, __LINE__);

	xrange = pPriHeader->fNumColumn;
	float DEPTH = pPriHeader->fDEPTH;

	BOOL bLoadComplete = TRUE;

	if( abs(DEPTH - 8) < 0.1f )
	{
		int	nIndex = 0;
		UCHAR tempData;
		for(int j=0;j<totalData;j++)
		{	
			ar >> tempData;

			ptrfVertexColorRed[nIndex] = (float)tempData / 255;
			ptrfVertexColorGreend[nIndex] = (float)tempData / 255;
			ptrfVertexColorBlue[nIndex] = (float)tempData / 255;

			++nIndex;

			if( codd )
				if( ((j+1) % xrange) == 0 )
					ar >> tempData;
		}
	}
	else if( abs(DEPTH - 16) < 0.1f )
	{
		int	nIndex = 0;
		short tempData;
		for(int j=0;j<totalData;j++)
		{	
			ar >> tempData;

			if( codd )
				if( ((j+1) % xrange) == 0 )
					ar >> tempData;

			UCHAR red = tempData>>8;
			UCHAR blue = (UCHAR)tempData;
			UINT green = UINT((float)red * 0.4f + (float)blue * 0.8f);

			if( green > 255 ) green = 255;

			ptrfVertexColorRed[nIndex] = (float)red / 255;
			ptrfVertexColorGreend[nIndex] = (float)green / 255;
			ptrfVertexColorBlue[nIndex] = (float)blue / 255;

			++nIndex;
		}
	}
	else if( abs(DEPTH - 32) < 0.1f )
	{
		int	nIndex = 0;
		UINT tempData;
		for(int j=0;j<totalData;j++)
		{	
			ar >> tempData;

			if( codd )
				if( ((j+1) % xrange) == 0 )
					ar >> tempData;

			UCHAR red	= tempData >> 16;
			UCHAR green	= tempData >> 8; 
			UCHAR blue	= (UCHAR)tempData;

			ptrfVertexColorRed[nIndex] = (float)red / 255;
			ptrfVertexColorGreend[nIndex] = (float)green / 255;
			ptrfVertexColorBlue[nIndex] = (float)blue / 255;

			++nIndex;
		}
	}
	else
	{
		AfxMessageBox(_T("DEPTH Error"));
		bLoadComplete = FALSE;
	}

	ar.Close();
	myFile.Close();

	if(pPriHeader)
	{
		//delete pPriHeader;
		g_pMManager->pem_delete(pPriHeader, false);
		pPriHeader = NULL;
	}
	if(pPtrHeader)
	{
		//delete pPtrHeader;
		g_pMManager->pem_delete(pPtrHeader, false);
		pPtrHeader = NULL;
	}

	return bLoadComplete;
}

// int ImageFileLoad_ptt(UCHAR* &ptrfVertexColorRed, UCHAR* &ptrfVertexColorGreend, UCHAR* &ptrfVertexColorBlue,
// 	UCHAR* &ptrfvRawData, wchar_t* filePath, int &nWidth, int &nHeight )
// {
// 	return FALSE;
// }

int ImgAllSavePtt(float *pZmap, float *pImg32f, wchar_t* filePath, LP_HEADER_PTT pHeader)
{
	//return alpf_save_ptt(filePath, pHeader, pZmap, pImg32f);

	if((pZmap == NULL) || (pImg32f == NULL)) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".ptt")) == FALSE)
	{
		tmpPath += _T(".ptt");
	}

	//DeleteFile(filePath);

	CFile file;
	file.Open(tmpPath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		int sizeX = 0;
		int sizeY = 0;

		ar << pHeader->uiNumCol;
		ar << pHeader->uiNumRow;
		//ar << pHeader->uiNumCol;
		ar << pHeader->zResolX;
		ar << pHeader->zResolY;
		ar << pHeader->sizeBit;
		ar << pHeader->gabX;
		ar << pHeader->gabY;
		ar << pHeader->sizeUnit;

		sizeX = (int)pHeader->uiNumRow;
		sizeY = (int)pHeader->uiNumCol;

		for(int y=0 ; y<sizeY ; y++)
		{
			for(int x=0 ; x<sizeX ; x++)
			{
				if (pHeader->sizeBit == 32)
					ar << (float)pZmap[x+(y*sizeX)];
				else
					ar << (short)pZmap[x+(y*sizeX)];
			}
		}

		ar << pHeader->fNumCol;
		ar << pHeader->fNumRow;
		//ar << pHeader->fNumCol;
		ar << pHeader->targetStartX;
		ar << pHeader->targetStartY;
		ar << pHeader->targetShiftX;
		ar << pHeader->targetShiftY;
		ar << pHeader->targetWidth;
		ar << pHeader->targetHeight;
		ar << pHeader->pixelResX;
		ar << pHeader->pixelResY;
		ar << pHeader->depthBit;

		sizeX = (int)pHeader->fNumRow;
		sizeY = (int)pHeader->fNumCol;

		float factorR(1), factorG(1), factorB(1);

		int nIdx = 0;
		for(int y=0 ; y<sizeY ; y++)
		{
			for(int x=0 ; x<sizeX ; x++)
			{
				//int nIdx = x+(y*sizeX);

				UCHAR red	= pImg32f[nIdx]		* 255;
				UCHAR green	= pImg32f[nIdx+1]	* 255; 
				UCHAR blue	= pImg32f[nIdx+2]	* 255;

				float fData = (float)(MPTT(red, green, blue, factorR, factorG, factorB));
				ar << (unsigned int)fData;
				nIdx += 3;
			}
		}

		ar.Close();
		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}

int ImgSavePtt(float *pZmap, float *pImg32f, wchar_t* filePath, LP_HEADER_PTT pHeader, RECT roi, int zmapWidth)
{
	return alpf_save_ptt(filePath, pHeader, roi, zmapWidth, pZmap, pImg32f);
}

CImgLoadSave::CImgLoadSave()
{
	m_pMGrab = NULL;
	m_pInspMng = NULL;
}

CImgLoadSave::~CImgLoadSave()
{
	Destroy();
}

 BOOL CImgLoadSave::Initial( CString sPathDcf, UINT nFovSizeX, UINT nFovSizeY )
 {
 	Destroy();
//  
//  	//m_pMGrab = new CMGrab();
//  	m_pMGrab = g_pMManager->pem_new<CMGrab>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
//  
//  	// SHKang 2017/05/17
//  	int nRet = m_pMGrab->Init(sPathDcf, nFovSizeX, nFovSizeY, true, 3);	
//  	// int nRet = m_pMGrab->Init(sPathDcf, nFovSizeX, nFovSizeY);
//  
 	//int bInit // (nRet == eMR_SUCCESS);
//  #ifndef _DEBUG
//  	if(bInit)
//  #endif // _DEBUG
//  	{
//  		//m_pInspMng = new CMInspManager();
//  		m_pInspMng = g_pMManager->pem_new<CMInspManager>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
//  		CPGrabDev &PGrabDev = m_pMGrab->GetPGrabDev();
//  		m_pInspMng->Init(&PGrabDev.GetMilApp(), &PGrabDev.GetMilSys());
//  		m_pInspMng->SetResolution(PGrabDev.GetResX(), PGrabDev.GetResY(),0.02,0.02);
//  	}
//  
	return false;
 }
 
 void CImgLoadSave::Destroy()
 {
//  	if(m_pInspMng)
//  	{
//  		//delete m_pInspMng;
//  		g_pMManager->pem_delete(m_pInspMng, false);
//  		m_pInspMng = NULL;
//  	}
//  	if(m_pMGrab)
//  	{
//  		m_pMGrab->Exit();
//  		//delete m_pMGrab;
//  		g_pMManager->pem_delete(m_pMGrab, false);
//  		m_pMGrab = NULL;
//  	}
 }
 
 void* CImgLoadSave::GetGrabSystem()
 {
	 return M_NULL;
 	if(!m_pMGrab)
 		return M_NULL;
 
 	//return m_pMGrab->GetGrabSystem();
 }
 
 void* CImgLoadSave::GetGrabApplication()
 {
	 return M_NULL;
 	if(!m_pMGrab)
 		return M_NULL;
 
 	//return m_pMGrab->GetGrabApplication();
 }
