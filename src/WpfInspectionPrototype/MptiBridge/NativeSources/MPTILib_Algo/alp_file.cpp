#include "stdafx.h"
#include "alp_file.h"
#include "SharedMemory.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

#ifndef _ALP_ALLOC_H_
#include "alp_alloc.h"
#endif


#define	LMTI(i)	(i < 0.f) ? 0.f : ((i > 255.f) ? 255.f : i)
#define MPTT(r,g,b,rf,gf,bf)	( ((int)(LMTI((float)r*rf)) << 16) | ((int)(LMTI((float)g*gf)) << 8) | ((int)(LMTI((float)b*bf))) )
#define MPTT_COMPOBTM(r,g,b,rf,gf,bf, br, bg, bb)	( ((int)(LMTI((float)r*rf+br)) << 16) | ((int)(LMTI((float)g*gf+bg)) << 8) | ((int)(LMTI((float)b*bf+bb))) )


int alpf_get_buk_info(CString filePath, LP_HEADER_BUK pHeader)
{
	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".buk")) == FALSE)
	{
		filePath += _T(".buk");
	}

	CFile file;
	file.Open(filePath, CFile::modeRead, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::load);

		ar >> pHeader->startX;
		ar >> pHeader->startY;
		ar >> pHeader->sizeX;
		ar >> pHeader->sizeY;
		ar >> pHeader->camFovX;
		ar >> pHeader->camFovY;
		ar >> pHeader->sizeBit;

		ar.Close();
		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}

// SHKang 2017/10/18 : 
int alpf_load_fbuk(CString sFilePath, int nFovX, int nFovY, int nImgCnt, PUINT8 *pArrDest)
{
	int nRet = 0;
	if(pArrDest == NULL) return nRet; // return nRet;

	CString tmpPath;
	tmpPath = sFilePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(5) == _T(".fbuk")) == FALSE)
	{
		sFilePath += _T(".fbuk");
	}

//	DWORD dwTick1 = GetTickCount();

	BOOL bShareMemory = FALSE;
	if(bShareMemory)
	{
		CSharedMemory sm;
		if(sm.OpenFile(sFilePath, _T("read"), OPEN_EXISTING, FILE_FLAG_OVERLAPPED|FILE_FLAG_SEQUENTIAL_SCAN)==true)
		{
//			DWORD dwTick2 = GetTickCount();
			sm.MapToRead(0, sm.m_nBytes);
//			DWORD dwTick3 = GetTickCount();
			BYTE * ptr = sm.Byte();
			int nOffset(0);

			HEADER_BUK header;
			memcpy(&header.startX	, &ptr[nOffset], sizeof(header.startX	));		nOffset += sizeof(header.startX	);
			memcpy(&header.startY	, &ptr[nOffset], sizeof(header.startY	));		nOffset += sizeof(header.startY	);
			memcpy(&header.sizeX	, &ptr[nOffset], sizeof(header.sizeX	));		nOffset += sizeof(header.sizeX	);
			memcpy(&header.sizeY	, &ptr[nOffset], sizeof(header.sizeY	));		nOffset += sizeof(header.sizeY	);
			memcpy(&header.camFovX	, &ptr[nOffset], sizeof(header.camFovX	));		nOffset += sizeof(header.camFovX);
			memcpy(&header.camFovY	, &ptr[nOffset], sizeof(header.camFovY	));		nOffset += sizeof(header.camFovY);
			memcpy(&header.sizeBit	, &ptr[nOffset], sizeof(header.sizeBit	));		nOffset += sizeof(header.sizeBit);

/*			DWORD dwTick4 = GetTickCount();*/
			int nFovSize = nFovX * nFovY;

			//  		if(sizeX > 0 && sizeY > 0)
			//  			memcpy(pDest , &ptr[nOffset], sizeX * sizeY);	nOffset += sizeX * sizeY;
			for(int i=0; i < nImgCnt; i++) 
			{
				UCHAR * pDest = (UCHAR *) pArrDest[i];
				memcpy(pDest , &ptr[nOffset], nFovSize);	nOffset += nFovSize;
			}
// 			DWORD dwTick5 = GetTickCount();
// 
// 			sRet.Format(_T("OpenFile : %.03f [s], MapToRead : %.03f [s], MemCpy(Head) : %.03f [s], MemCpy(Body) : %.03f [s]"), 
// 				(dwTick2- dwTick1)/1000., (dwTick3 -dwTick2)/1000., (dwTick4 -dwTick3)/1000., (dwTick5 -dwTick4)/1000.);
		}

	}
	else
	{
		CFile file;
		file.Open(sFilePath, CFile::modeRead, NULL);
//		DWORD dwTick2 = GetTickCount();
		if((HANDLE)file != INVALID_HANDLE_VALUE)
		{
			CArchive ar(&file, CArchive::load);

//			DWORD dwTick3 = GetTickCount();

			HEADER_BUK header;
			ar >> header.startX;
			ar >> header.startY;
			ar >> header.sizeX;
			ar >> header.sizeY;
			ar >> header.camFovX;
			ar >> header.camFovY;
			ar >> header.sizeBit;

//			DWORD dwTick4 = GetTickCount();
			int nFovSize = nFovX * nFovY;

			for(int i=0; i < nImgCnt; i++) 
			{
				UCHAR * pDest = (UCHAR *) pArrDest[i];
				ar.Read(pDest, nFovSize);
			}

//			DWORD dwTick5 = GetTickCount();
			ar.Close();
			file.Close();

// 			sRet.Format(_T("Open : %.03f [s], CArchive::load : %.03f [s], MemCpy(Head) : %.03f [s], MemCpy(Body) : %.03f [s]"), 
// 				(dwTick2- dwTick1)/1000., (dwTick3 -dwTick2)/1000., (dwTick4 -dwTick3)/1000., (dwTick5 -dwTick4)/1000.);
		}
	}

	return nRet;
}

// SHKang 2017/10/18 : 
int alpf_save_fbuk(CString filePath, LP_HEADER_BUK pHeader, int nImgCnt, PUINT8 *pArrSrc)
{
	if(pArrSrc == NULL) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(5) == _T(".fbuk")) == FALSE)
	{
		filePath += _T(".fbuk");
	}

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file == INVALID_HANDLE_VALUE) return -1;

	{
		CArchive ar(&file, CArchive::store);

		ar << pHeader->startX;
		ar << pHeader->startY;
		ar << pHeader->sizeX;
		ar << pHeader->sizeY;
		ar << pHeader->camFovX;
		ar << pHeader->camFovY;
		ar << pHeader->sizeBit;

		int sizeX = (int)pHeader->sizeX;
		int sizeY = (int)pHeader->sizeY / nImgCnt;

		for(int i=0; i< nImgCnt; i++)
		{
			UCHAR * pSrc = (UCHAR *)pArrSrc[i];
			ar.Write(pSrc, sizeX * sizeY);
		}

		ar.Close();
		file.Close();
	}
	return 0;
}

int alpf_load_buk(CString filePath, UCHAR *pDest, int nDstW, int nDstH, int &nSizex, int &nSizey)
{
	if(pDest == NULL) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".buk")) == FALSE)
	{
		filePath += _T(".buk");
	}

	CSharedMemory sm;
	if (sm.OpenFile(filePath, _T("read"), OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN) == false)
		return -1;

		sm.MapToRead(0, sm.m_nBytes);
		BYTE * ptr = sm.Byte();
		int nOffset(0);

		HEADER_BUK header;
		memcpy(&header.startX	, &ptr[nOffset], sizeof(header.startX	));		nOffset += sizeof(header.startX	);
		memcpy(&header.startY	, &ptr[nOffset], sizeof(header.startY	));		nOffset += sizeof(header.startY	);
		memcpy(&header.sizeX	, &ptr[nOffset], sizeof(header.sizeX	));		nOffset += sizeof(header.sizeX	);
		memcpy(&header.sizeY	, &ptr[nOffset], sizeof(header.sizeY	));		nOffset += sizeof(header.sizeY	);
		memcpy(&header.camFovX	, &ptr[nOffset], sizeof(header.camFovX	));		nOffset += sizeof(header.camFovX);
		memcpy(&header.camFovY	, &ptr[nOffset], sizeof(header.camFovY	));		nOffset += sizeof(header.camFovY);
		memcpy(&header.sizeBit	, &ptr[nOffset], sizeof(header.sizeBit	));		nOffset += sizeof(header.sizeBit);

		DWORD dwTick4 = GetTickCount();
		int sizeX = (int)header.sizeX;
		int sizeY = (int)header.sizeY;

	if (sizeX <= 0 || sizeX != nDstW || sizeY <= 0 || sizeY != nDstH)
		return -1;
  			memcpy(pDest , &ptr[nOffset], sizeX * sizeY);	nOffset += sizeX * sizeY;
	nSizex = sizeX;
	nSizey = sizeY;
	return 0;

/*

 	CFile file;
	file.Open(filePath, CFile::modeRead, NULL);
	DWORD dwTick2 = GetTickCount();
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::load);

		DWORD dwTick3 = GetTickCount();

		HEADER_BUK header;
		ar >> header.startX;
		ar >> header.startY;
		ar >> header.sizeX;
		ar >> header.sizeY;
		ar >> header.camFovX;
		ar >> header.camFovY;
		ar >> header.sizeBit;

		DWORD dwTick4 = GetTickCount();
		int sizeX = (int)header.sizeX;
		int sizeY = (int)header.sizeY;

		//for(int y=0 ; y<sizeY ; y++)
		//{
		//	for(int x=0 ; x<sizeX ; x++)
		//	{
		//		ar >> pDest[x+(y*sizeX)];
		//	}
		//}

 		int nFrameCnt = 56;
 		for(int i=0; i < nFrameCnt; i++) 
 		{
 			int size = sizeX * sizeY/nFrameCnt;
 			ar.Read(pDest, size);
 		}
//  		 if(sizeX > 0 && sizeY > 0 )
//  			ar.Read(pDest[I], (sizeX * sizeY));
//		ar.Read(pDest, (sizeX * sizeY));
		DWORD dwTick5 = GetTickCount();
		ar.Close();
		file.Close();

		sRet.Format(_T("Open : %.03f [s], CArchive::load : %.03f [s], MemCpy(Head) : %.03f [s], MemCpy(Body) : %.03f [s]"), 
			(dwTick2- dwTick1)/1000., (dwTick3 -dwTick2)/1000., (dwTick4 -dwTick3)/1000., (dwTick5 -dwTick4)/1000.);
	}
	else
	{
		return sRet;
		// return -1;
	}

	return sRet;
	// return 0;
*/
}


int alpf_save_buk(CString filePath, LP_HEADER_BUK pHeader, UCHAR *pSrc)
{
	if(pSrc == NULL) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".buk")) == FALSE)
	{
		filePath += _T(".buk");
	}

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		ar << pHeader->startX;
		ar << pHeader->startY;
		ar << pHeader->sizeX;
		ar << pHeader->sizeY;
		ar << pHeader->camFovX;
		ar << pHeader->camFovY;
		ar << pHeader->sizeBit;

		int sizeX = (int)pHeader->sizeX;
		int sizeY = (int)pHeader->sizeY;
		/*
		for(int y=0 ; y<sizeY ; y++)
		{
			for(int x=0 ; x<sizeX ; x++)
			{
				ar << pSrc[x+(y*sizeX)];
			}
		}
		*/
		if(sizeX > 0 && sizeY > 0 )
			ar.Write(pSrc, sizeX * sizeY);

		ar.Close();
		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}

int alpf_get_nphi_info(CString filePath, LP_HEADER_NPHI pHeader)
{
	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(5) == _T(".nphi")) == FALSE)
	{
		filePath += _T(".nphi");
	}

	OFSTRUCT of;
	CStringA filePathA = (CStringA)filePath;
	if(OpenFile(filePathA, &of, OF_EXIST) == HFILE_ERROR)
		return -1;

	CFile file;
	file.Open(filePath, CFile::modeRead, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::load);

		ar >> pHeader->width;
		ar >> pHeader->height;

		ar.Close();
		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}


int alpf_load_nphi(CString filePath, float *pDest)
{
	if(pDest == NULL) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(5) == _T(".nphi")) == FALSE)
	{
		filePath += _T(".nphi");
	}

	CFile file;
	file.Open(filePath, CFile::modeRead, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::load);

		HEADER_NPHI header;
		ar >> header.width;
		ar >> header.height;

// SHKang 2017/05/23		
#ifdef _NPHI_INT
		int temp = 0;
		for(int y=0 ; y<header.height ; y++)
		{
			for(int x=0 ; x<header.width ; x++)
			{
				// ATW 2012.12.24 change : prevent change in data belong type casting
				ar >> temp;
				pDest[x+(y*header.width)] = (float)temp / 10000.f;
			}
		}
#else
		ar.Read(pDest, header.width * header.height);
#endif
		ar.Close();
		file.Close();

// 		int temp = 0;
// 		for(int y=0 ; y<header.height ; y++)
// 		{
// 			for(int x=0 ; x<header.width ; x++)
// 			{
// #ifdef _NPHI_INT
// 				// ATW 2012.12.24 change : prevent change in data belong type casting
// 				ar >> temp;
// 				pDest[x+(y*header.width)] = (float)temp / 10000.f;
// #else
// 				ar >> pDest[x+(y*header.width)];
// #endif		
// 			}
// 		}
// 		ar.Close();
// 		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}


int alpf_save_nphi(CString filePath, LP_HEADER_NPHI pHeader, float *pSrc)
{
	if(pSrc == NULL) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(5) == _T(".nphi")) == FALSE)
	{
		filePath += _T(".nphi");
	}

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		ar << pHeader->width;
		ar << pHeader->height;

		int len = (pHeader->width*pHeader->height);

// SHKang 2017/05/23
#ifdef _NPHI_INT
		for(int i=0 ; i<len ; i++)
		{
			// ATW 2012.12.24 change : prevent change in data belong type casting
			ar << (int)(pSrc[i] * 10000.f);
		}
#else
		ar.Write(pSrc, len);
#endif

		ar.Close();
		file.Close();	

// 		for(int i=0 ; i<len ; i++)
// 		{
// #ifdef _NPHI_INT
// 			// ATW 2012.12.24 change : prevent change in data belong type casting
// 			ar << (int)(pSrc[i] * 10000.f);
// #else
// 			ar << pSrc[i];
// #endif
// 		}
// 		ar.Close();
// 		file.Close();	
	}
	else
	{
		return -1;
	}

	return 0;
}


int alpf_get_phi_info(CString filePath, LP_HEADER_PHI pHeader)
{
	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".phi")) == FALSE)
	{
		filePath += _T(".phi");
	}

	CFile file;
	file.Open(filePath, CFile::modeRead, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::load);

		ar >> pHeader->left;
		ar >> pHeader->top;
		ar >> pHeader->width;
		ar >> pHeader->height;
		ar >> pHeader->res0;
		ar >> pHeader->res1;

		ar.Close();
		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}


int alpf_load_phi(CString filePath, float *pDest)
{
	if(pDest == NULL) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".phi")) == FALSE)
	{
		filePath += _T(".phi");
	}

	CFile file;
	file.Open(filePath, CFile::modeRead, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::load);

		HEADER_PHI header;
		ar >> header.left;
		ar >> header.top;
		ar >> header.width;
		ar >> header.height;
		ar >> header.res0;
		ar >> header.res1;
		
		int right, bottom;
		right = header.left + header.width;
		bottom = header.top + header.height;

		float **pTemp = NULL;
		alpa_make_array_2d(&pTemp, header.height, header.width);

		int temp;
		for(int x=header.left ; x<right ; x++)
		{
			for(int y=header.top ; y<bottom ; y++)
			{
				ar >> temp;
				pTemp[y-header.top][x-header.left] = (float)temp / 10000.f;

				/*
				temp = (float)temp/10000;
				pTemp[y][x]=(int)((temp+PI)*1000);
				*/
			}
		}

		for(int y=0 ; y<header.height ; y++)
		{
			for(int x=0 ; x<header.width ; x++)
			{
				pDest[x+(y*header.width)] = pTemp[y][x];
			}
		}

		alpa_delete_array_2d(&pTemp, header.height);

		ar.Close();
		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}


int alpf_convert_phi_to_nphi(CString filePath)
{
	int ret = 0;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".phi")) == FALSE)
	{
		filePath += _T(".phi");
	}

	CFile loadFile;
	loadFile.Open(filePath, CFile::modeRead, NULL);
	if(loadFile)
	{
		CArchive ar(&loadFile, CArchive::load);

		HEADER_PHI header;
		ar >> header.left;
		ar >> header.top;
		ar >> header.width;
		ar >> header.height;
		ar >> header.res0;
		ar >> header.res1;
		
		int right, bottom;
		right = header.left + header.width;
		bottom = header.top + header.height;

		int **pTemp = NULL;
#ifdef _NPHI_INT
		int *pTemp2 = NULL;	// ATW 2012.12.24 change : type casting from int to float
#else
		float *pTemp2 = NULL;
#endif
		alpa_make_array_2d(&pTemp, header.height, header.width);
		alpa_make_array_1d(&pTemp2, header.width*header.height);

		for(int x=header.left ; x<right ; x++)
		{
			for(int y=header.top ; y<bottom ; y++)
			{
				ar >> pTemp[y-header.top][x-header.left];
			}
		}

		for(int y=0 ; y<header.height ; y++)
		{
			for(int x=0 ; x<header.width ; x++)
			{
#ifdef _NPHI_INT
				pTemp2[x+(y*header.width)] = pTemp[y][x];
#else
				// ATW 2012.12.24 change : type casting from int to float
				pTemp2[x+(y*header.width)] = pTemp[y][x] / 10000.f;
#endif
			}
		}

		CString nphiFilePath;
		filePath.Delete(filePath.GetLength()-4, 4);
		nphiFilePath = filePath;
		nphiFilePath += _T(".nphi");

		CFile saveFile;
		saveFile.Open(nphiFilePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
		if(saveFile)
		{
			CArchive ar(&saveFile, CArchive::store);

			ar << header.width;
			ar << header.height;

			int lenData = header.width * header.height;
			for(int n=0 ; n<lenData ; n++)
			{
				ar << pTemp2[n];
			}

			ar.Close();
			saveFile.Close();	
		}
		else
		{
			ret = -1;
		}
		
		alpa_delete_array_2d(&pTemp, header.height);
		alpa_delete_array_1d(&pTemp2);

		ar.Close();
		loadFile.Close();
	}
	else
	{
		ret = -1;
	}

	return ret;
}


int alpf_save_cw(CString filePath, LP_HEADER_CW pHeader, float *pZmap)
{
	if(pZmap == NULL) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(3) == _T(".cw")) == FALSE)
	{
		filePath += _T(".cw");
	}

	DeleteFile(filePath);

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		//ar << pHeader->width;
		//ar << pHeader->height;
		ar << pHeader->height;
		ar << pHeader->width;

		for (int y=0 ; y<pHeader->height ; y++)
		{
			for (int x=0 ; x<pHeader->width ; x++)
			{
				ar << pZmap[(y * pHeader->width) + x];
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


int alpf_save_cw(CString filePath, LP_HEADER_CW pHeader, RECT roi, int zmapWidth, float *pZmap)
{
	if(pZmap == NULL) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(3) == _T(".cw")) == FALSE)
	{
		filePath += _T(".cw");
	}

	DeleteFile(filePath);

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);
		
		//ar << pHeader->width;
		//ar << pHeader->height;
		ar << pHeader->height;
		ar << pHeader->width;
		
		int idx = 0;
		for (int y=roi.top ; y<roi.bottom ; y++)
		{
			for (int x=roi.left ; x<roi.right ; x++)
			{
				idx = (y * zmapWidth) + x;

				ar << pZmap[idx];
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


int alpf_save_ptt(CString filePath, LP_HEADER_PTT pHeader, float *pZmap, float *pImg32f)
{
	if((pZmap == NULL) || (pImg32f == NULL)) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".ptt")) == FALSE)
	{
		filePath += _T(".ptt");
	}

	DeleteFile(filePath);

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
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
					ar << (float)pZmap[x + (y*sizeX)];
				else
					ar << (short)pZmap[x + (y*sizeX)];
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

		for(int y=0 ; y<sizeY ; y++)
		{
			for(int x=0 ; x<sizeX ; x++)
			{
				ar << (unsigned int)pImg32f[x+(y*sizeX)];
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

int alpf_save_ptr(CString filePath, LP_HEADER_PTR pHeader, float *pZmap)
{
	if ((pZmap == NULL)) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if ((tmpPath.Right(4) == _T(".ptr")) == FALSE)
	{
		filePath += _T(".ptr");
	}

	DeleteFile(filePath);

	CFile file;
	file.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate, NULL);
	if ((HANDLE)file != INVALID_HANDLE_VALUE)
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

		for (int y = 0; y < sizeY; y++)
		{
			for (int x = 0; x < sizeX; x++)
			{
				if(pHeader->sizeBit == 16)
					ar << (short)pZmap[x + (y*sizeX)];
				else
					ar << pZmap[x + (y*sizeX)];
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

int alpf_save_ptt(CString filePath, LP_HEADER_PTT pHeader, RECT roi, int zmapWidth, float *pZmap, float *pImg32f)
{
	if((pZmap == NULL) || (pImg32f == NULL)) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".ptt")) == FALSE)
	{
		filePath += _T(".ptt");
	}

	DeleteFile(filePath);

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		int idx = 0;

		ar << pHeader->uiNumCol;
		ar << pHeader->uiNumRow;
		//ar << pHeader->uiNumCol;
		ar << pHeader->zResolX;
		ar << pHeader->zResolY;
		ar << pHeader->sizeBit;
		ar << pHeader->gabX;
		ar << pHeader->gabY;
		ar << pHeader->sizeUnit;

		for (int y=roi.top ; y<roi.bottom ; y++)
		{
			for (int x=roi.left ; x<roi.right ; x++)
			{
				idx = (y * zmapWidth) + x;
				if (pHeader->sizeBit == 32)
					ar << (float)pZmap[idx];
				else
				ar << (short)pZmap[idx];
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

		for (int y=roi.top ; y<roi.bottom ; y++)
		{
			for (int x=roi.left ; x<roi.right ; x++)
			{
				idx = (y * zmapWidth) + x;

				ar << (unsigned int)pImg32f[idx];
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


int alpf_make_img32f(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, float factorR, float factorG, float factorB, int dstWidthStep, float *pDest)
{
	if((pImgR == NULL) || (pImgG == NULL) || (pImgB == NULL) || (pDest == NULL)) return -1;

	int idxSrc = 0;
	int idxDst = 0;

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			idxSrc = (y * imgWidthStep) + x;
			idxDst = (y * dstWidthStep) + x;

			pDest[idxDst] = (float)(MPTT(pImgR[idxSrc], pImgG[idxSrc], pImgB[idxSrc], factorR, factorG, factorB));
		}
	}

	return 0;
}

int alpf_make_img32f_CompositeBtm(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, UCHAR *pImgBR, UCHAR *pImgBB,
	float factorR, float factorG, float factorB, float factorBR, float factorBB, float CompoBtmR, float CompoBtmG, float CompoBtmB, int dstWidthStep, float *pDest)
{
	if((pImgR == NULL) || (pImgG == NULL) || (pImgB == NULL) || (pImgBR == NULL) || (pImgBB == NULL) || (pDest == NULL)) return -1;

	int idxSrc = 0;
	int idxDst = 0;

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			idxSrc = (y * imgWidthStep) + x;
			idxDst = (y * dstWidthStep) + x;

			float fBR = (float)(pImgBR[idxSrc]*factorBR) * CompoBtmR;
			LMTI(fBR);

			float fBB = (float)(pImgBR[idxSrc]*factorBB) * CompoBtmB;
			LMTI(fBB);

			float fBGR = 0.4f;
			float fBGB = 0.8f;

			float fBG = ((fBR * fBGR) + (fBB * fBGB)) * CompoBtmG;
			LMTI(fBG);

			pDest[idxDst] = (float)(MPTT_COMPOBTM(pImgR[idxSrc], pImgG[idxSrc], pImgB[idxSrc], factorR, factorG, factorB, fBR, fBG, fBB));
		}
	}

	return 0;
}

// SHKang 2017/07/20
int alpf_make_img24n32f(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, float factorR, float factorG, float factorB, int dstWidthStep, float *pDest32, UCHAR* pDest24, int dstDst24WidthStep)
{
	if((pImgR == NULL) || (pImgG == NULL) || (pImgB == NULL) || (pDest32 == NULL)) return -1;
	if(dstDst24WidthStep==0) dstDst24WidthStep = dstWidthStep;
// 	int idxSrc = 0;
// 	int idxDst = 0;
// 	int idxRoiDst = 0;
// 
// 	int roiW = (int)(roi.right - roi.left);
// 	float data = 0;
	int roiW = (int)(roi.right - roi.left);

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			float data = 0;
			int idxSrc = (y * imgWidthStep) + x;
			int idxDst = (y * dstWidthStep) + x;

//			int idxRoiDst = (int)((((y - roi.top) * roiW) + (x - roi.left)) * 3);
			int idxRoiDst = (int)((((y - roi.top) * dstDst24WidthStep) + (x - roi.left)) * 3);	// SHKang 2017/07/20 : 이건 버그네....헐

			if(pDest32 != NULL)
				pDest32[idxDst] = (float)(MPTT(pImgR[idxSrc], pImgG[idxSrc], pImgB[idxSrc], factorR, factorG, factorB));

			if(pDest24 != NULL)
			{
				data = pImgB[idxSrc] * factorB;	//blue
				if(data > 255)
					pDest24[idxRoiDst] = 255;
				else
					pDest24[idxRoiDst] = (UCHAR)data;


				data = pImgG[idxSrc] * factorG;	//green
				if(data > 255)
					pDest24[idxRoiDst + 1] = 255;
				else
					pDest24[idxRoiDst + 1] = (UCHAR)data;


				data = pImgR[idxSrc] * factorR;	//red
				if(data > 255)
					pDest24[idxRoiDst + 2] = 255;
				else
					pDest24[idxRoiDst + 2] = (UCHAR)data;
			}
		}
	}

	return 0;
}

int alpf_make_img24n32f_CompositeBtm(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, UCHAR *pImgBR, UCHAR *pImgBB, float factorR, float factorG, float factorB, float factorBR, float factorBB, float compoBtmR, float compoBtmG, float compoBtmB, int dstWidthStep, float *pDest32, UCHAR* pDest24, int dstDst24WidthStep)	// SHKang 2017/07/20
{
	//if((pImgR == NULL) || (pImgG == NULL) || (pImgB == NULL) || (pDest32 == NULL)) return -1;
	if((pImgR == NULL) || (pImgG == NULL) || (pImgB == NULL) || (pImgBR == NULL) || (pImgBB == NULL) || (pDest32 == NULL)) return -1;
	if(dstDst24WidthStep==0) dstDst24WidthStep = dstWidthStep;
	int idxSrc = 0;
	int idxDst = 0;
	int idxRoiDst = 0;
	float data = 0;

	int roiW = (int)(roi.right - roi.left);

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			int idxSrc = 0;
			int idxDst = 0;
			int idxRoiDst = 0;
			float data = 0;

			idxSrc = (y * imgWidthStep) + x;
			idxDst = (y * dstWidthStep) + x;
			// idxRoiDst = (int)((((y - roi.top) * roiW) + (x - roi.left)) * 3);
			idxRoiDst = (int)((((y - roi.top) * dstDst24WidthStep) + (x - roi.left)) * 3);	// SHKang 2017/07/20 : 이건 버그네....헐

			float fBR = (float)(pImgBR[idxSrc]*factorBR) * compoBtmR;
			LMTI(fBR);

			float fBB = (float)(pImgBB[idxSrc]*factorBB) * compoBtmB;
			LMTI(fBB);

			float fBGR = 0.4f;
			float fBGB = 0.8f;

			float fBG = ((fBR * fBGR) + (fBB * fBGB)) * compoBtmG;
			LMTI(fBG);

			if(pDest32 != NULL)
				pDest32[idxDst] = (float)(MPTT_COMPOBTM(pImgR[idxSrc], pImgG[idxSrc], pImgB[idxSrc], factorR, factorG, factorB, fBR, fBG, fBB));
				//pDest32[idxDst] = (float)(MPTT(pImgR[idxSrc], pImgG[idxSrc], pImgB[idxSrc], factorR, factorG, factorB));

			if(pDest24 != NULL)
			{
				data = (pImgB[idxSrc] * factorB) + fBB;	//blue
				if(data > 255)
					pDest24[idxRoiDst] = 255;
				else
					pDest24[idxRoiDst] = (UCHAR)data;


				data = (pImgG[idxSrc] * factorG) + fBG;
				if(data > 255)
					pDest24[idxRoiDst + 1] = 255;
				else
					pDest24[idxRoiDst + 1] = (UCHAR)data;


				data = (pImgR[idxSrc] * factorR) + fBR;	//red
				if(data > 255)
					pDest24[idxRoiDst + 2] = 255;
				else
					pDest24[idxRoiDst + 2] = (UCHAR)data;
			}
		}
	}

	return 0;
}

int alpf_make_img24(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, float factorR, float factorG, float factorB, int dstWidthStep, UCHAR* pDest24, int dstDst24WidthStep)
{
	if((pImgR == NULL) || (pImgG == NULL) || (pImgB == NULL) || (pDest24 == NULL)) return -1;
	if(dstDst24WidthStep==0) dstDst24WidthStep = dstWidthStep;

	int idxSrc = 0;
	int idxDst = 0;
	int idxRoiDst = 0;
	float data = 0;

	int roiW = (int)(roi.right - roi.left);

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			int idxSrc = 0;
			int idxDst = 0;
			int idxRoiDst = 0;
			float data = 0;

			idxSrc = (y * imgWidthStep) + x;
			idxDst = (y * dstWidthStep) + x;
			//idxRoiDst = (int)((((y - roi.top) * roiW) + (x - roi.left)) * 3);			
			idxRoiDst = (int)((((y - roi.top) * dstDst24WidthStep) + (x - roi.left)) * 3);	// SHKang 2017/07/20 : 이건 버그네....헐

			if(pDest24 != NULL)
			{
				data = pImgB[idxSrc] * factorB;	//green

				if(data > 255)
					pDest24[idxRoiDst] = 255;
				else
					pDest24[idxRoiDst] = (UCHAR)data;


				data = pImgG[idxSrc] * factorG;	//green
				if(data > 255)
					pDest24[idxRoiDst + 1] = 255;
				else
					pDest24[idxRoiDst + 1] = (UCHAR)data;


				data = pImgR[idxSrc] * factorR;	//red
				if(data > 255)
					pDest24[idxRoiDst + 2] = 255;
				else
					pDest24[idxRoiDst + 2] = (UCHAR)data;
			}
		}
	}

	return 0;
}

int alpf_make_Miximg24_S4(RECT roi, int imgWidthStep, UCHAR *pImgR, UCHAR *pImgG, UCHAR *pImgB, UCHAR *pImgUV, float factorR, float factorG, float factorB, int dstWidthStep, UCHAR* pDest24, int dstDst24WidthStep)
{
	if((pImgR == NULL) || (pImgG == NULL) || (pImgB == NULL) || (pDest24 == NULL)) return -1;
	if(dstDst24WidthStep==0) dstDst24WidthStep = dstWidthStep;

	int idxSrc = 0;
	int idxDst = 0;
	int idxRoiDst = 0;
	float data = 0;

	int roiW = (int)(roi.right - roi.left);

	for (int y=roi.top ; y<roi.bottom ; y++)
	{
		for (int x=roi.left ; x<roi.right ; x++)
		{
			int idxSrc = 0;
			int idxDst = 0;
			int idxRoiDst = 0;
			float data = 0;

			idxSrc = (y * imgWidthStep) + x;
			idxDst = (y * dstWidthStep) + x;
			//idxRoiDst = (int)((((y - roi.top) * roiW) + (x - roi.left)) * 3);			
			idxRoiDst = (int)((((y - roi.top) * dstDst24WidthStep) + (x - roi.left)) * 3);	// SHKang 2017/07/20 : 이건 버그네....헐

			if(pDest24 != NULL)
			{
				float b1 = (pImgB[idxSrc]*(factorB*0.3));
				float b2 = (pImgUV[idxSrc]*(factorB*0.7));
				data = b1+b2;

				if(data > 255)
					pDest24[idxRoiDst] = 255;
				else
					pDest24[idxRoiDst] = (UCHAR)data;


				data = pImgG[idxSrc] * factorG;	//green
				if(data > 255)
					pDest24[idxRoiDst + 1] = 255;
				else
					pDest24[idxRoiDst + 1] = (UCHAR)data;


				data = pImgR[idxSrc] * factorR;	//red
				if(data > 255)
					pDest24[idxRoiDst + 2] = 255;
				else
					pDest24[idxRoiDst + 2] = (UCHAR)data;
			}
		}
	}

	return 0;
}


int alpf_save_pot(CString filePath, LP_HEADER_POT pHeader, UCHAR *pImgBB, UCHAR *pImgBR, UCHAR *pImgMB, UCHAR *pImgMR, UCHAR *pImgTW)
{
	if(pImgBB == NULL || pImgBR == NULL || pImgMB == NULL || pImgMR == NULL || pImgTW == NULL)
		return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".pot")) == FALSE)
	{
		filePath += _T(".pot");
	}

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		ar << pHeader->fNumCol;
		ar << pHeader->fNumRow;
		ar << pHeader->fPixelResX;
		ar << pHeader->fPixelResY;
		ar << pHeader->fDepthBit;

		int sizeX = (int)pHeader->fNumCol;
		int sizeY = (int)pHeader->fNumRow;
		int y = 0;
		int x = 0;

		// SHKang 2017/05/23
		ar.Write(pImgBB, sizeX * sizeY);
		ar.Write(pImgBR, sizeX * sizeY);
		ar.Write(pImgMB, sizeX * sizeY);
		ar.Write(pImgMR, sizeX * sizeY);
		ar.Write(pImgTW, sizeX * sizeY);

		ar.Close();
		file.Close();
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgBB[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgBR[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgMB[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgMR[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgTW[x+(y*sizeX)];
// 			}
// 		}
// 		ar.Close();
// 		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}

int alpf_save_put(CString filePath, LP_HEADER_POT pHeader, UCHAR *pImgTR, UCHAR *pImgTG, UCHAR *pImgTB, UCHAR *pImgBB, UCHAR *pImgBR)
{
	if(pImgTR == NULL || pImgTG == NULL || pImgTB == NULL || pImgBB == NULL)
		return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".put")) == FALSE)
	{
		filePath += _T(".put");
	}

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		ar << pHeader->fNumCol;
		ar << pHeader->fNumRow;
		ar << pHeader->fPixelResX;
		ar << pHeader->fPixelResY;
		ar << pHeader->fDepthBit;

		int sizeX = (int)pHeader->fNumCol;
		int sizeY = (int)pHeader->fNumRow;
		int y = 0;
		int x = 0;


		// SHKang 2017/05/23
		ar.Write(pImgBB, sizeX * sizeY);
		if(pImgBR != NULL)
			ar.Write(pImgBR, sizeX * sizeY);
		ar.Write(pImgTB, sizeX * sizeY);
		ar.Write(pImgTR, sizeX * sizeY);
		ar.Write(pImgTG, sizeX * sizeY);

		ar.Close();
		file.Close();


// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgBB[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgTB[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgTR[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgTG[x+(y*sizeX)];
// 			}
// 		}
// 
// 		ar.Close();
// 		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}

int alpf_save_pst(CString filePath, LP_HEADER_POT pHeader, UCHAR *pImgSR, UCHAR *pImgSG, UCHAR *pImgSB)
{
	if(pImgSR == NULL || pImgSG == NULL || pImgSB == NULL)
		return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".pst")) == FALSE)
	{
		filePath += _T(".pst");
	}

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		ar << pHeader->fNumCol;
		ar << pHeader->fNumRow;
		ar << pHeader->fPixelResX;
		ar << pHeader->fPixelResY;
		ar << pHeader->fDepthBit;

		int sizeX = (int)pHeader->fNumCol;
		int sizeY = (int)pHeader->fNumRow;
		int y = 0;
		int x = 0;


		// SHKang 2017/05/23
		ar.Write(pImgSR, sizeX * sizeY);
		ar.Write(pImgSG, sizeX * sizeY);
		ar.Write(pImgSB, sizeX * sizeY);

		ar.Close();
		file.Close();
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgSR[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgSG[x+(y*sizeX)];
// 			}
// 		}
// 		for(y=0 ; y<sizeY ; y++)
// 		{
// 			for(x=0 ; x<sizeX ; x++)
// 			{
// 				ar << pImgSB[x+(y*sizeX)];
// 			}
// 		}
// 
// 		ar.Close();
// 		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}

// SHKang 2017/03/30 : R,G,B Factor 값을 적용하여 PTT 파일을 생성한다. JPG 메모리로도 리턴한다.
int alpf_save_ptt(CString filePath, LP_HEADER_PTT pHeader, float *pZmap, BYTE *pImgR, BYTE *pImgG, BYTE *pImgB, float fFactorR, float fFactorG, float fFactorB, UCHAR* pDest24)
{
	if((pZmap == NULL) || (pImgR == NULL) || (pImgG == NULL) || (pImgB == NULL) || (pDest24 == NULL)) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".ptt")) == FALSE)
	{
		filePath += _T(".ptt");
	}

	DeleteFile(filePath);

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
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

		int dataR = 0.f;
		int dataG = 0.f;
		int dataB = 0.f;

		int nIdx = 0;
		int idxRoiDst = 0;
		for(int y=0 ; y<sizeY ; y++)
		{
			for(int x=0 ; x<sizeX ; x++)
			{
				//				ar << (unsigned int)pImg32f[x+(y*sizeX)];
				nIdx = x+(y*sizeX);

				dataR = (int)(LMTI((float)pImgR[nIdx]*fFactorR));
				dataG = (int)(LMTI((float)pImgG[nIdx]*fFactorG));
				dataB = (int)(LMTI((float)pImgG[nIdx]*fFactorB));

				unsigned int unRGB =  (dataR << 16) | (dataG << 8) | dataR;

				ar << unRGB;

				// jpg 를 위한 24 bit
				idxRoiDst = nIdx*3;

				pDest24[idxRoiDst] = (UCHAR)dataB;
				pDest24[idxRoiDst + 1] = (UCHAR)dataG;
				pDest24[idxRoiDst + 2] = (UCHAR)dataR;
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

//shkim Load
int alpf_load_buk_to_savebmp(CString filePath, UCHAR **pDest,int* sizeX,int *sizeY)
{
	if(*pDest != NULL) delete [] *pDest;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".buk")) == FALSE)
	{
		filePath += _T(".buk");
	}

	CSharedMemory sm;
	if(sm.OpenFile(filePath, _T("read"), OPEN_EXISTING, FILE_FLAG_OVERLAPPED|FILE_FLAG_SEQUENTIAL_SCAN)==false) return -1;

	{
		sm.MapToRead(0, sm.m_nBytes);
		BYTE * ptr = sm.Byte();
		int nOffset(0);

		HEADER_BUK header;
		memcpy(&header.startX	, &ptr[nOffset], sizeof(header.startX	));		nOffset += sizeof(header.startX	);
		memcpy(&header.startY	, &ptr[nOffset], sizeof(header.startY	));		nOffset += sizeof(header.startY	);
		memcpy(&header.sizeX	, &ptr[nOffset], sizeof(header.sizeX	));		nOffset += sizeof(header.sizeX	);
		memcpy(&header.sizeY	, &ptr[nOffset], sizeof(header.sizeY	));		nOffset += sizeof(header.sizeY	);
		memcpy(&header.camFovX	, &ptr[nOffset], sizeof(header.camFovX	));		nOffset += sizeof(header.camFovX);
		memcpy(&header.camFovY	, &ptr[nOffset], sizeof(header.camFovY	));		nOffset += sizeof(header.camFovY);
		memcpy(&header.sizeBit	, &ptr[nOffset], sizeof(header.sizeBit	));		nOffset += sizeof(header.sizeBit);

		DWORD dwTick4 = GetTickCount();
		* sizeX = (int)header.sizeX;
		* sizeY = (int)header.sizeY;
		if(sizeX > 0 && sizeY > 0)
		{
			*pDest=new UCHAR[(*sizeX) *( *sizeY)];
			memcpy(*pDest , &ptr[nOffset], (*sizeX) *( *sizeY));	nOffset += (*sizeX) *( *sizeY);
		}
	}
	return 0;
}

// .p3d file 저장 (3d 디버깅용 Part ROI data)
int alpf_save_p3d(CString filePath, size_t szSaveBuf, BYTE *pSrc)
{
	if(pSrc == NULL || szSaveBuf <= 0) return -1;

	CString tmpPath;
	tmpPath = filePath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".p3d")) == FALSE)
	{
		filePath += _T(".p3d");
	}

	CFile file;
	file.Open(filePath, CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate, NULL);
	if((HANDLE)file != INVALID_HANDLE_VALUE)
	{
		CArchive ar(&file, CArchive::store);

		ar.Write(pSrc, szSaveBuf);

		ar.Close();
		file.Close();
	}
	else
	{
		return -1;
	}

	return 0;
}