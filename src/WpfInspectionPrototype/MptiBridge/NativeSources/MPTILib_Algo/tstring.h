#pragma once

#include <string>
#include <locale>
#include <codecvt>
#include <memory>

namespace std
{
#ifndef  _LLFMT
	#define _LLFMT	_T("%I64")
#endif

#ifndef _MAX_SIG_DIG
	#define _MAX_SIG_DIG	36
#endif
	//=================================================================

	typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstring;
	typedef basic_istringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > itstringstream;
	typedef basic_ostringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > otstringstream;
	typedef basic_stringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstringstream;
	typedef basic_ifstream<TCHAR, char_traits<TCHAR> > iftstream;
	typedef basic_ofstream<TCHAR, char_traits<TCHAR> > oftstream;
	typedef basic_fstream<TCHAR, char_traits<TCHAR> > ftstream;

	inline tstring to_tstring(INT64 _Val)
	{
		TCHAR _Buf[2 * _MAX_INT_DIG];

		_CSTD _tprintf_s(_Buf, sizeof (_Buf), _LLFMT, _T("d"), _Val);
		return (tstring(_Buf));
	}

	inline tstring to_tstring(UINT64 _Val)
	{
		TCHAR _Buf[2 * _MAX_INT_DIG];

		_CSTD _tprintf_s(_Buf, sizeof (_Buf), _LLFMT, _T("u"), _Val);
		return (tstring(_Buf));
	}

	inline tstring to_tstring(long double _Val)
	{
		TCHAR _Buf[_MAX_EXP_DIG + _MAX_SIG_DIG + 64];

		_CSTD _tprintf_s(_Buf, sizeof (_Buf), _T("%Lg"), _Val);
		return (tstring(_Buf));
	}

	//=================================================================

#ifdef UNICODE
	#define TEOF WEOF
#else
	#define TEOF EOF
#endif
	
	typedef enum _ENM_FILE_TYPE { eFtFail, eANSI, eUnicode, eUnicodeLE, eUTF8 }enmFileType;
	enmFileType _pfencode_t(CString sPath);

	FILE * _pfopen(CString sPath, CString mode, errno_t * fe=nullptr);
	errno_t _pfopen_s(FILE ** fp, CString sPath, CString mode);

	//=================================================================

	class ftencode_t
	{
	public:
		ftencode_t(CString sPath);
		locale get();
		locale get(enmFileType eft);
	protected:
		enmFileType m_eft;
	};

	//=================================================================

	class stdio_file
	{
	public:
		stdio_file();
		stdio_file(CString path, int mode, CFileException * fe=nullptr);
		virtual ~stdio_file();
	public:
		BOOL Open(CString path, int mode, CFileException * fe=nullptr);
		BOOL ReadString(CString & str);
		void WriteString(LPCTSTR str);
		void Close();
		ULONGLONG GetLength();
	public:
		FILE * m_fp;
		BOOL m_bIsOpened;
	protected:
		CString GetMode(int mode);
		std::shared_ptr<CStdioFile> m_File;
	};

	//=================================================================
};