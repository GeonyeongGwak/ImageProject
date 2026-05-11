// ReadWriteFile.cpp: implementation of the CReadWriteFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReadWriteFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReadWriteFile::CReadWriteFile()
{

}

CReadWriteFile::~CReadWriteFile()
{
	RemoveAll();
}

BOOL CReadWriteFile::Open(LPCTSTR szPath)
{
//	if( ( szPath == NULL ) || ( strlen( szPath ) <= 0 ) ) return FALSE;
	if( ( szPath == NULL ) || ( _tcslen(szPath) <= 0 ) ) return FALSE;

	CFile file;
	BOOL fRet = file.Open( szPath, CFile::modeRead | CFile::shareDenyWrite);
	if( !fRet ) return fRet;
	
	UINT file_size = (UINT)file.GetLength();
	if( file_size == 0 ) return FALSE;

	//char* pBuffer = new char[ file_size + 1 ];
	char* pBuffer = g_pMManager->pem_new<char>(true, file_size + 1, (PCHAR)__FUNCTION__, __LINE__);
	//TCHAR* pBuffer = new TCHAR[ file_size + 1 ];
	register DWORD offset = 0;
	bool carry = false;

	while( file.Read( pBuffer + offset , 1 ) > 0 )
	{
		if( pBuffer[offset] == 0x0d )
		{
			carry = true;
			offset++;
			continue;
		}

		if( carry && ( pBuffer[offset] == 0x0a)  )
		{
			pBuffer[offset -1] = '\0';
			Add( pBuffer );
			offset = 0;
			carry = false;
		}
		else{
			offset++;
		}
	}

	if( offset > 0 )
	{
		pBuffer[offset] = '\0';
		Add( pBuffer );
	}
	
	file.Close();

	if(pBuffer) 
		//delete [] pBuffer;
		g_pMManager->pem_delete(pBuffer, true);

	return TRUE;
}

void CReadWriteFile::Add(const char *str)
{
	size_t size;
	//char* newString = new char[ strlen( str )  + sizeof( char ) ];
	char* newString = g_pMManager->pem_new<char>(true, strlen(str) + sizeof(char), (PCHAR)__FUNCTION__, __LINE__);
	size = strlen(str) + 1;
	strcpy_s( newString, size, str );
// 	TCHAR* newString = new TCHAR[ _tcslen( str )  + sizeof( TCHAR ) ];
// 	size = _tcslen(str) + 1;
// 	_tcscpy_s( newString, size, str );

	if( m_StringList.IsEmpty() )
		m_StringList.AddHead( newString );
	else m_StringList.AddTail( newString );
}

int CReadWriteFile::GetCount()
{
	return m_StringList.GetCount();
}

char* CReadWriteFile::operator [](int index)
{
	if( index >= GetCount() ) return NULL;

	POSITION pos = m_StringList.FindIndex( index );
	if( pos == NULL ) return NULL;
	return m_StringList.GetAt( pos );
}

void CReadWriteFile::RemoveAll()
{
	while( !m_StringList.IsEmpty() )
	{
		char* pTemp = m_StringList.RemoveHead();
		//TCHAR* pTemp = m_StringList.RemoveHead();
		if( pTemp )
			//delete pTemp;

			// MemoryLeak Fix -wjlee
			//g_pMManager->pem_delete(pTemp, false);
			g_pMManager->pem_delete(pTemp, true);
	}
}

BOOL CReadWriteFile::Open2(LPCTSTR szPath)
{
//	if( ( szPath == NULL ) || ( strlen( szPath ) <= 0 ) ) return FALSE;
	if( ( szPath == NULL ) || ( _tcslen( szPath ) <= 0 ) ) return FALSE;

	CFile file;
	BOOL fRet = file.Open( szPath, CFile::modeRead | CFile::shareDenyWrite);
	if( !fRet ) return fRet;
	
	UINT file_size = (UINT)file.GetLength();
	if( file_size == 0 ) return FALSE;

	//char* pBuffer = new char[ file_size + 1];
	char* pBuffer = g_pMManager->pem_new<char>(true, file_size + 1, (PCHAR)__FUNCTION__, __LINE__);
	//TCHAR* pBuffer = new TCHAR[ file_size + 1];
	register DWORD offset = 0;
	bool carry = false;
	while( file.Read( pBuffer + offset , 1 ) > 0 )
	{
/*
				if( pBuffer[offset] == 0x0d )
				{
					carry = true;
					offset++;
					continue;
				}*/
		

		if(/* carry &&*/ ( pBuffer[offset] == 0x0a ))
		{
			offset++;
			pBuffer[offset -1] = '\0';
			Add( pBuffer );
			offset = 0;
			carry = false;

		}
		else{
			offset++;
		}
	}

	if( offset > 0 )
	{
		pBuffer[offset] = '\0';
		Add( pBuffer );
	}
	
	file.Close();

	if(pBuffer) 
		//delete [] pBuffer;
		g_pMManager->pem_delete(pBuffer, true);

	return TRUE;
}
