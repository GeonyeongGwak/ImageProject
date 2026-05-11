/********************************************************************
// Create
             Date :    2008/1/25

             author:    LMJ

             purpose:   

 

// function Modify list

*********************************************************************/
// ReadWriteFile.h: interface for the CReadWriteFile class.
//GFC_C_11
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_READWRITEFILE_H__7DD36E87_DF52_4287_A7EC_21FCEC0C58A1__INCLUDED_)
#define AFX_READWRITEFILE_H__7DD36E87_DF52_4287_A7EC_21FCEC0C58A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxtempl.h>

typedef CTypedPtrList<CPtrList, char*> pStrList;
//typedef CTypedPtrList<CPtrList, TCHAR*> pStrList;

class CReadWriteFile  
{
protected:
	pStrList		m_StringList;
public:
	BOOL Open2(LPCTSTR szPath);
	void RemoveAll();
	int GetCount();
	char* operator [](int index);
	void Add(const char *str);
	BOOL Open(LPCTSTR szPath);
	CReadWriteFile();
	virtual ~CReadWriteFile();

};

#endif // !defined(AFX_READWRITEFILE_H__7DD36E87_DF52_4287_A7EC_21FCEC0C58A1__INCLUDED_)
