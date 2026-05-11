
#include "stdafx.h"

#include <io.h>
#include <direct.h>
#include "PemMinidump.h"
#include "ExtInspLogic.h"

LPTOP_LEVEL_EXCEPTION_FILTER PreviousExceptionFilter = NULL;

BOOL PemMinidump::Begin( VOID )
{
	PreviousExceptionFilter = SetUnhandledExceptionFilter(UnHandledExceptionFilter);
	return true;
}

BOOL PemMinidump::End( VOID )
{
	SetUnhandledExceptionFilter(PreviousExceptionFilter);
	return true;
}

LONG WINAPI UnHandledExceptionFilter( struct _EXCEPTION_POINTERS *exceptionInfo )
{
	HMODULE	DllHandle		= NULL;

	// Windows 2000 이전에는 따로 DBGHELP를 배포해서 설정해 주어야 한다.
	DllHandle				= LoadLibrary(_T("DBGHELP.DLL"));

	if (DllHandle)
	{
		MINIDUMPWRITEDUMP Dump = (MINIDUMPWRITEDUMP) GetProcAddress(DllHandle, "MiniDumpWriteDump");

		if (Dump)
		{
			CaptureScreenShot();

			TCHAR		DumpPath[MAX_PATH] = {0,};
			SYSTEMTIME	SystemTime;

			GetLocalTime(&SystemTime);
			TCHAR tszBuff[MAX_PATH];
			VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), tszBuff, MAX_PATH));

			CString strTemp, strFilePath;

			strTemp.Format(_T("%s"), tszBuff);
			strTemp = strTemp.Left(strTemp.ReverseFind('\\'));
#ifdef _DEBUG
			strTemp = strTemp.Left(strTemp.ReverseFind('\\'));
#endif
			strTemp = strTemp + _T("\\DumpMessage");
			
			if (GetFileAttributes(_T("C:\\Temp")) == -1)
			{
				CreateDirectory(_T("C:\\Temp"), NULL);
			}
 			FILE * fp = _tfopen(_T("C:\\Temp\\dump.txt"), _T("w"));
 			fclose(fp);

			char MBBuffer[1024];
			memset(MBBuffer,0,sizeof(MBBuffer));

#ifdef _UNICODE
			WideCharToMultiByte(CP_ACP, 0, strTemp.GetBuffer(0), -1, MBBuffer, 1024, 0, 0);
#else
			sprintf(MBBuffer, _T("%s"), strTemp);
#endif

			if (_access(MBBuffer,0)==-1)
				_mkdir(MBBuffer);

			//MessageBoxA(NULL, MBBuffer, "", MB_OK );

			_sntprintf(DumpPath, MAX_PATH, _T("%s\\%04d%02d%02d_%02d%02d%02d.dmp"), strTemp,
				SystemTime.wYear,
				SystemTime.wMonth,
				SystemTime.wDay,
				SystemTime.wHour,
				SystemTime.wMinute,
				SystemTime.wSecond);

			HANDLE FileHandle = CreateFile(
				DumpPath, 
				GENERIC_WRITE, 
				FILE_SHARE_WRITE, 
				NULL, CREATE_ALWAYS, 
				FILE_ATTRIBUTE_NORMAL, 
				NULL);

			if (FileHandle != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION MiniDumpExceptionInfo;

				MiniDumpExceptionInfo.ThreadId			= GetCurrentThreadId();
				MiniDumpExceptionInfo.ExceptionPointers	= exceptionInfo;
				MiniDumpExceptionInfo.ClientPointers	= NULL;

				MINIDUMP_TYPE dumpType = MiniDumpNormal;

				int nFullDump = GetPrivateProfileInt(_T("DevelopOption"), _T("GetFullDump"),  
					0, _T("C:\\PEMTRON\\DevelopOption.ini"));

				if (nFullDump == 1)
					dumpType = MiniDumpWithFullMemory;

				BOOL Success = Dump(
					GetCurrentProcess(), 
					GetCurrentProcessId(), 
					FileHandle, 
					dumpType,
					&MiniDumpExceptionInfo, 
					NULL, 
					NULL);

				if (Success)
				{
					CloseHandle(FileHandle);
					return EXCEPTION_CONTINUE_SEARCH;
				}
			}
			CloseHandle(FileHandle);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}



bool WriteDIB( LPCTSTR szFile, HANDLE hDIB )
{
	BITMAPFILEHEADER    hdr;
	LPBITMAPINFOHEADER  lpbi;

	if(!hDIB)
		return FALSE;

	CFile file;

	if(!file.Open( szFile, CFile::modeWrite|CFile::modeCreate) )
		return FALSE;
	lpbi = (LPBITMAPINFOHEADER)hDIB;
	int nColors = 1 << lpbi->biBitCount;

	// Fill in the fields of the file header 
	hdr.bfType      = ((WORD) ('M' << 8) | 'B');    // is always "BM"
	hdr.bfSize      = GlobalSize (hDIB) + sizeof(hdr);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	hdr.bfOffBits   = (DWORD) (sizeof( hdr ) + lpbi->biSize + nColors * sizeof(RGBQUAD));

	// Write the file header 
	file.Write(&hdr, sizeof(hdr));

	// Write the DIB header and the bits 
	file.Write(lpbi, GlobalSize(hDIB));

	return TRUE;
}

void CaptureScreenShot()
{
	CClientDC dc( NULL );
	CRect rc(0, 0, GetSystemMetrics(SM_CXMAXTRACK), GetSystemMetrics(SM_CYMAXTRACK) );

	CBitmap captureBmp;
	captureBmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height() );

	CDC dcMem;
	dcMem.CreateCompatibleDC( &dc );
	CBitmap* pOldBitmap = dcMem.SelectObject(&captureBmp);

	dcMem.BitBlt(0, 0, rc.Width(), rc.Height(), &dc, 0, 0, SRCCOPY );
	dcMem.SelectObject( pOldBitmap );

	HANDLE hDIB = DDBToDIB(captureBmp, BI_RGB, NULL);
	if( hDIB != NULL )
	{
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);

		if (_access("..\\ScreenShot",0)==-1)
			_mkdir("..\\ScreenShot");

		CString strPath;
		strPath.Format( _T("..\\ScreenShot\\%04d%02d%02d_%02d%02d%02d%03d.bmp"),
			csTime.wYear, csTime.wMonth, csTime.wDay,
			csTime.wHour, csTime.wMinute, csTime.wSecond, csTime.wMilliseconds );

		WriteDIB((LPCTSTR)strPath, hDIB );
		//		CloseHandle(hDIB );
	}

	captureBmp.DeleteObject();
}

HANDLE DDBToDIB( CBitmap &bitmap, DWORD dwCompression, CPalette *pPal )
{
	BITMAP   bm;
	BITMAPINFOHEADER bi;
	LPBITMAPINFOHEADER  lpbi;
	DWORD   dwLen;
	HANDLE   hDIB;
	HANDLE   handle;
	HDC    hDC;
	HPALETTE  hPal;


	ASSERT( bitmap.GetSafeHandle() );

	// The function has no arg for bitfields
	if( dwCompression == BI_BITFIELDS )
		return NULL;

	// If a palette has not been supplied use defaul palette
	hPal = (HPALETTE) pPal->GetSafeHandle();
	if (hPal==NULL)
		hPal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

	// Get bitmap information
	bitmap.GetObject(sizeof(bm),(LPSTR)&bm);

	// Initialize the bitmapinfoheader
	bi.biSize  = sizeof(BITMAPINFOHEADER);
	bi.biWidth  = bm.bmWidth;
	bi.biHeight   = bm.bmHeight;
	bi.biPlanes   = 1;
	bi.biBitCount  = bm.bmPlanes * bm.bmBitsPixel;
	bi.biCompression = dwCompression;
	bi.biSizeImage  = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed  = 0;
	bi.biClrImportant = 0;

	// Compute the size of the  infoheader and the color table
	int nColors = (1 << bi.biBitCount);
	if( nColors > 256 )
		nColors = 0;
	dwLen  = bi.biSize + nColors * sizeof(RGBQUAD);

	// We need a device context to get the DIB from
	hDC = ::GetDC(NULL);
	hPal = SelectPalette(hDC,hPal,FALSE);
	RealizePalette(hDC);

	// Allocate enough memory to hold bitmapinfoheader and color table
	hDIB = GlobalAlloc(GMEM_FIXED,dwLen);
	

	if (!hDIB){
		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	lpbi = (LPBITMAPINFOHEADER)hDIB;

	*lpbi = bi;

	// Call GetDIBits with a NULL lpBits param, so the device driver 
	// will calculate the biSizeImage field 
	GetDIBits(hDC, (HBITMAP)bitmap.GetSafeHandle(), 0L, (DWORD)bi.biHeight,
		(LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

	bi = *lpbi;

	// If the driver did not fill in the biSizeImage field, then compute it
	// Each scan line of the image is aligned on a DWORD (32bit) boundary
	if (bi.biSizeImage == 0){
		bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8)
			* bi.biHeight;

		// If a compression scheme is used the result may infact be larger
		// Increase the size to account for this.
		if (dwCompression != BI_RGB)
			bi.biSizeImage = (bi.biSizeImage * 3) / 2;
	}

	// Realloc the buffer so that it can hold all the bits
	dwLen += bi.biSizeImage;
	if (handle = GlobalReAlloc(hDIB, dwLen, GMEM_MOVEABLE))
		hDIB = handle;
	else{
		GlobalFree(hDIB);

		// Reselect the original palette
		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	// Get the bitmap bits
	lpbi = (LPBITMAPINFOHEADER)hDIB;

	// FINALLY get the DIB
	BOOL bGotBits = GetDIBits( hDC, (HBITMAP)bitmap.GetSafeHandle(),
		0L,    // Start scan line
		(DWORD)bi.biHeight,  // # of scan lines
		(LPBYTE)lpbi    // address for bitmap bits
		+ (bi.biSize + nColors * sizeof(RGBQUAD)),
		(LPBITMAPINFO)lpbi,  // address of bitmapinfo
		(DWORD)DIB_RGB_COLORS);  // Use RGB for color table

	if( !bGotBits )
	{
		GlobalFree(hDIB);

		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	SelectPalette(hDC,hPal,FALSE);
	::ReleaseDC(NULL,hDC);
	return hDIB;
}
