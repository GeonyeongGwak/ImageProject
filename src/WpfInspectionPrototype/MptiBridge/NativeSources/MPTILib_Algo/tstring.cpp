#include "StdAfx.h"
#include "tstring.h"
	
namespace std
{
	//=================================================================

	enmFileType _pfencode_t(CString sPath)
	{
		unsigned char ucHeader[3];
		enmFileType eFileType(eFtFail);

		if(_taccess_s(sPath, 00)!=0)
			return eFileType;

		CFile pFile;
		CFileStatus fs;
		CFile::GetStatus(sPath, fs);
		if(fs.m_size<2)
		{
			eFileType = eANSI;
			return eFileType;
		}
		if(pFile.Open(sPath, CFile::modeRead)==FALSE)
			return eFileType;

		pFile.Read(ucHeader, 2);
		if (ucHeader[0] == 0xff && ucHeader[1] == 0xfe)		// Check for Unicode
			eFileType = eUnicode;
		else if (ucHeader[0] == 0xfe && ucHeader[1] == 0xff)	// Check for Unicode Big Endian
			eFileType = eUnicodeLE;
		else if (ucHeader[0] == 0xef && ucHeader[1] == 0xbb && fs.m_size>=3)	// Check for UTF-8
		{
			pFile.Read(&ucHeader[2], 1);		// UTF-8 has a three char header
			if (ucHeader[2] == 0xbf)
				eFileType = eUTF8;
		}
		else
			eFileType = eANSI;

		pFile.Close();
		return eFileType;
	}
	FILE * _pfopen(CString sPath, CString mode, errno_t * fe)
	{
		enmFileType eft = _pfencode_t(sPath);

		FILE * fp=nullptr;
		CString modeExt;
		if(eft == eUnicode)
			modeExt=_T(",ccs=UNICODE");
		else if(eft == eUnicodeLE)
			modeExt=_T(",ccs=UTF-16LE");
		else if(eft == eUTF8)
			modeExt=_T(",ccs=UTF-8");
// 		else if(eft == eFtFail)			// TO_THINK : 유니코드로 저장하고 싶을 때 풀면 됨.
// 			modeExt=_T(",ccs=UNICODE");
		else
		{
			TCHAR buf[4096];
			GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, buf, 4096);
			setlocale(LC_ALL, CT2A(buf));
		}
		fp = _tfopen(sPath, mode+modeExt);

		if(fe)
			*fe = GetLastError();

		return fp;
	}
	errno_t _pfopen_s(FILE ** fp, CString sPath, CString mode)
	{
		errno_t err(0);
		*fp = std::_pfopen(sPath, mode, &err);
		return err;
	}

	//=================================================================

	ftencode_t::ftencode_t(CString sPath)
		: m_eft(eFtFail)
	{
		m_eft = _pfencode_t(sPath);
	}
	locale ftencode_t::get()
	{
		return get(m_eft);
	}
	locale ftencode_t::get(enmFileType eft)
	{
		if(eft == eUnicode)
		{
			const locale empty_locale = locale::empty();
			return locale(empty_locale, new codecvt_utf16<TCHAR, 0x10ffff, little_endian>);
		}
		else if(eft == eUnicodeLE)
		{
			const locale empty_locale = locale::empty();
			return locale(empty_locale, new codecvt_utf16<TCHAR>);
		}
		else if(eft == eUTF8)
		{
			const locale empty_locale = locale::empty();
			return locale(empty_locale, new codecvt_utf8<TCHAR>);
		}
		else
		{
			TCHAR buf[4096];
			GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, buf, 4096);
			return locale( string(CT2A(buf)) );
		}
	}

	//=================================================================

	stdio_file::stdio_file()
		: m_fp(nullptr)
		, m_bIsOpened(FALSE)
	{

	}
	stdio_file::stdio_file(CString path, int mode, CFileException * fe)
		: m_fp(nullptr)
		, m_bIsOpened(FALSE)
	{
		m_bIsOpened = Open(path, mode, fe);
	}
	stdio_file::~stdio_file()
	{
		Close();
	}
	BOOL stdio_file::Open(CString path, int mode, CFileException * fe)
	{
		m_fp = _pfopen(path, GetMode(mode));

		if(m_fp)
		{
			m_bIsOpened = TRUE;
			m_File = std::shared_ptr<CStdioFile>( new CStdioFile(m_fp) );
			return TRUE;
		}
		else
			return FALSE;
	}
	BOOL stdio_file::ReadString(CString & str)
	{
		if(m_File==nullptr)
			return FALSE;
		return m_File->ReadString(str);
	}
	void stdio_file::WriteString(LPCTSTR str)
	{
		if(m_File==nullptr)
			return;
		return m_File->WriteString(str);
	}
	void stdio_file::Close()
	{
		m_File.reset();
		if(m_fp)
			fclose(m_fp);
		m_fp=nullptr;
		m_bIsOpened = FALSE;
	}
	ULONGLONG stdio_file::GetLength()
	{
		if(m_File==nullptr)
			return 0;
		return m_File->GetLength();
	}
	CString stdio_file::GetMode(int mode)
	{
		CString strMode;

		if((mode&CFile::modeCreate) == CFile::modeCreate)
		{
			if((mode&CFile::modeNoTruncate) == CFile::modeNoTruncate)
				strMode = _T("a");
			else
				strMode = _T("w");

			if((mode&CFile::typeText)== CFile::typeText)
				strMode += _T("t");

			if( (mode&CFile::modeRead) == CFile::modeRead || 
				(mode&CFile::modeReadWrite) == CFile::modeReadWrite)
				strMode += _T("+");
		}
		else 
		{
			strMode = _T("r");
			if((mode&CFile::typeText)== CFile::typeText)
				strMode += _T("t");

			if( (mode&CFile::modeWrite) == CFile::modeWrite || 
				(mode&CFile::modeReadWrite) == CFile::modeReadWrite)
				strMode += _T("+");
		}

		return strMode;
	}

	//=================================================================
};