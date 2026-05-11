#include "stdafx.h"
#include "ExtTools.h"
#include <stdarg.h>
namespace ext
{
	CString Format(LPCTSTR sFormat, ...)
	{
		CString str;

		va_list args;
		va_start(args, sFormat);
		str.Format(sFormat, args);
		va_end(args);
		
		return str;
	}
}