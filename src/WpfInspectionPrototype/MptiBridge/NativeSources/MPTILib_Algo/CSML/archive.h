#pragma once
#include "afx.h"
#include <vector>
#include <memory>

namespace csml
{
	typedef CArchive  archive;

	class AFX_EXT_CLASS ar_item
	{
	public:
		ar_item(int mode=archive::Mode::store)
		{
			_mfile = std::shared_ptr<CMemFile>(new CMemFile(52428800));
			_ar = std::shared_ptr<archive>(new archive(_mfile.get(), mode));
		}
		ar_item(ar_item & ar)
		{
			size_t sz(0);
			ar >> sz;
			if (sz > 0)
			{
				_buf.resize(sz);
				ar.get().Read(&_buf[0], (UINT)sz);
				_mfile = std::shared_ptr<CMemFile>(new CMemFile(52428800));
				_mfile->Attach(&_buf[0], (UINT)_buf.size());
				_ar = std::shared_ptr<archive>(new archive(_mfile.get(), archive::load, (int)_buf.size()));
			}
		}
		ar_item(archive & ar)
		{
			size_t sz(0);
			ar >> sz;
			if(sz > 0)
			{
				_buf.resize(sz);
				ar.Read(&_buf[0], (UINT)sz);
				_mfile = std::shared_ptr<CMemFile>(new CMemFile(52428800));
				_mfile->Attach(&_buf[0], (UINT)_buf.size());
				_ar = std::shared_ptr<archive>(new archive(_mfile.get(), archive::load, (int)_buf.size()));
			}			
		}
		virtual ~ar_item() 
		{
			if(_ar!=nullptr && _ar->GetFile() != nullptr)
				_ar->Close();

			if(_mfile != nullptr)
				_mfile->Close();
		}

	public:
		// insertion operations
		template < typename T>
		ar_item& operator<<(T by) 
		{ 
			(*_ar) << by; 
			return *this; 
		}
		template < typename T>
		ar_item& operator>>(T& by)
		{
			if (_mfile->GetPosition() == 0 || _ar->IsBufferEmpty()==FALSE)
				(*_ar.get()) >> by; 
			return *this; 
		}

// 		archive& operator<<(BYTE by) { (*_ar) << by; return get(); }
// 		archive& operator<<(WORD w) { (*_ar) << w; return get(); }
// 		archive& operator<<(LONG l) { (*_ar) << l; return get(); }
// 		archive& operator<<(DWORD dw) { (*_ar) << dw; return get(); }
// 		archive& operator<<(float f) { (*_ar) << f; return get(); }
// 		archive& operator<<(double d) { (*_ar) << d; return get(); }
// 		archive& operator<<(LONGLONG dwdw) { (*_ar) << dwdw; return get(); }
// 		archive& operator<<(ULONGLONG dwdw) { (*_ar) << dwdw; return get(); }
// 
// 		archive& operator<<(int i) { (*_ar) << i; return get(); }
// 		archive& operator<<(short w) { (*_ar) << w; return get(); }
// 		archive& operator<<(char ch) { (*_ar) << ch; return get(); }
// #ifdef _NATIVE_WCHAR_T_DEFINED
// 		archive& operator<<(wchar_t ch) { (*_ar) << ch; return get(); }
// #endif
// 		archive& operator<<(unsigned u) { (*_ar) << u; return get(); }
// 
// 		template < typename BaseType, bool t_bMFCDLL>
// 		archive& operator<<(const ATL::CSimpleStringT<BaseType, t_bMFCDLL>& str) { (*_ar) << str; return get(); }
// 
// 		template< typename BaseType, class StringTraits >
// 		archive& operator<<(const ATL::CStringT<BaseType, StringTraits>& str) { (*_ar) << str; return get(); }
// 
// 		template < typename BaseType, bool t_bMFCDLL>
// 		archive& operator >> (ATL::CSimpleStringT<BaseType, t_bMFCDLL>& str) { if(_ar->IsBufferEmpty()==false) (*_ar) >> str; return get(); }
// 
// 		template< typename BaseType, class StringTraits >
// 		archive& operator >> (ATL::CStringT<BaseType, StringTraits>& str) { if (_ar->IsBufferEmpty() == false) (*_ar) >> str; return get(); }
// 
// 		archive& operator<<(bool b) { (*_ar) << b; return get(); }
// 
// 		// extraction operations
// 		archive& operator >> (BYTE& by) { if (_ar->IsBufferEmpty() == false) (*_ar) >> by; return get(); }
// 		archive& operator >> (WORD& w) { if (_ar->IsBufferEmpty() == false) (*_ar) >> w; return get(); }
// 		archive& operator >> (DWORD& dw) { if (_ar->IsBufferEmpty() == false) (*_ar) >> dw; return get(); }
// 		archive& operator >> (LONG& l) { if (_ar->IsBufferEmpty() == false) (*_ar) >> l; return get(); }
// 		archive& operator >> (float& f) { if (_ar->IsBufferEmpty() == false) (*_ar) >> f; return get(); }
// 		archive& operator >> (double& d) { if (_ar->IsBufferEmpty() == false) (*_ar) >> d; return get(); }
// 		archive& operator >> (LONGLONG& dwdw) { if (_ar->IsBufferEmpty() == false) (*_ar) >> dwdw; return get(); }
// 		archive& operator >> (ULONGLONG& dwdw) { if (_ar->IsBufferEmpty() == false) (*_ar) >> dwdw; return get(); }
// 
// 		archive& operator >> (int& i) { if (_ar->IsBufferEmpty() == false) (*_ar) >> i; return get(); }
// 		archive& operator >> (short& w) { if (_ar->IsBufferEmpty() == false) (*_ar) >> w; return get(); }
// 		archive& operator >> (char& ch) { if (_ar->IsBufferEmpty() == false) (*_ar) >> ch; return get(); }
// #ifdef _NATIVE_WCHAR_T_DEFINED
// 		archive& operator >> (wchar_t& ch) { if (_ar->IsBufferEmpty() == false) (*_ar) >> ch; return get(); }
// #endif
// 		archive& operator >> (unsigned& u) { if (_ar->IsBufferEmpty() == false) (*_ar) >> u; return get(); }
// 		archive& operator >> (bool& b) { if (_ar->IsBufferEmpty() == false) (*_ar) >> b; return get(); }

	public:
		void WriteTo(archive & ar)
		{
			_ar->Close();
			size_t szFile = _mfile->GetLength();
			ar << szFile;
			if(szFile > 0)
			{
				BYTE * buf = _mfile->Detach();
				ar.Write(buf, (UINT)szFile);
				free(buf);
			}
		}
		void WriteTo(ar_item & ar)
		{
			WriteTo(ar.get());
		}
		archive& get() { return (*_ar); }
		
	private:
		std::shared_ptr<archive> _ar;
		std::shared_ptr<CMemFile> _mfile;
		std::vector<BYTE> _buf;
	};

}


