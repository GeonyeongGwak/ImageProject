#include "stdafx.h"
#include "extVM.h"
#include "ntos.h"
#include "ExtLog.h"
#include "ExtTools.h"

namespace ext
{
	ImageBuffer_VM_Tools::ImageBuffer_VM_Tools()
		: _sizebytes(0)
		, _pBuffer(nullptr)
	{

	}
	ImageBuffer_VM_Tools::~ImageBuffer_VM_Tools()
	{
		Exit();
	}
	bool ImageBuffer_VM_Tools::Create(QWORD sizebytes)
	{
		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
		_pBuffer = (BYTE *)NtAllocateVirtualMemory(hProc, nullptr, sizebytes, 0, MEM_COMMIT, PAGE_READWRITE);
		if (_pBuffer == nullptr)
		{
			DWORD dwErrCode = GetLastError();
			ext::Log::add(ext::Format(_T("VirtualAlloc Error Code = %d"), dwErrCode));
		}
		CloseHandle(hProc);
		return _pBuffer != nullptr;
	}
	bool ImageBuffer_VM_Tools::Init()
	{
		ext::Log::add(_T("ImageBuffer::Init() - Start"));

		bool bInit = Create(env::nImgBufSize);

		ext::Log::add(_T("ImageBuffer_VM_Tools::Init() - End"));
		return bInit;
	}
	void ImageBuffer_VM_Tools::Exit()
	{
		if (_pBuffer == nullptr)
			return;
		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
		SIZE_T dwBufSz = env::nImgBufSize;
		void * pBuf = _pBuffer;
		if (NtFreeVirtualMemory(hProc, &pBuf, &dwBufSz, MEM_RELEASE) == FALSE)
		{
			DWORD dwErrCode = GetLastError();
			ext::Log::add(ext::Format(_T("VirtualFree Error Code = %d"), dwErrCode));
		}
		CloseHandle(hProc);
		_pBuffer = nullptr;
	}
	BYTE * ImageBuffer_VM_Tools::Lock(ImgData pos)
	{
		return _pBuffer + pos.imgStIdx;
	}
	void ImageBuffer_VM_Tools::Unlock(ImgData pos)
	{

	}






	ImageBuffer_VM_Eagle3D::ImageBuffer_VM_Eagle3D()
		: _offset(0)
		, _sizebytes(0)
		, _ctrl(nullptr)
		, _nToolId(-1)
		, _procId(0)
	{

	}
	ImageBuffer_VM_Eagle3D::~ImageBuffer_VM_Eagle3D()
	{
		Exit();
	}

	bool ImageBuffer_VM_Eagle3D::Init(int nTool_id, Ctrlee * ctrl)
	{
		_nToolId = nTool_id;
		_ctrl = ctrl;
		return true;
	}
	void ImageBuffer_VM_Eagle3D::Exit()
	{

	}

	ImgData ImageBuffer_VM_Eagle3D::Push(BYTE * ptr, int Wid, int Len, int Ch, int widthbytes)
	{
		DWORD dwProcId = _ctrl->First()->dwProcessID;
		HANDLE hTool = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _ctrl->First()->dwProcessID);
		if (hTool == INVALID_HANDLE_VALUE || hTool == nullptr)
			return ImgData();

		ImgData ret;
		if (_offset >= _sizebytes || (_offset + (Wid*Len)) >= _sizebytes)
			_offset = 0;

		QWORD qwImgBufAddr(_offset);	// 원래 주소반영되어야 함.

		ret.imgStIdx = _offset;
		QWORD ImgSz(0);
		BYTE * buf = (BYTE *)qwImgBufAddr;
		if ((Wid*Ch) == widthbytes)
		{
			SIZE_T nWritten(0);
			int nTryCnt(0);
			do {
				NTSTATUS nRes = NtWriteVirtualMemory(hTool, buf + nWritten, ptr, (Wid*Ch*Len)- nWritten, &nWritten);
				nTryCnt++;
			} while (nWritten < (Wid*Ch*Len) && nTryCnt < Len);
			assert(nWritten == (Wid*Ch*Len));
		}
		else
		{
			for (int y = 0; y < Len; y++)
			{
				int nLineSz = Wid * Ch;
				SIZE_T nWritten(0);
				int nTryCnt(0);
				do {
					NTSTATUS nRes = NtWriteVirtualMemory(hTool, buf + Wid * Ch*y + nWritten, ptr + widthbytes * y, nLineSz-nWritten, &nWritten);
					nTryCnt++;
				} while (nWritten < nLineSz && nTryCnt < 2);
				ImgSz += nLineSz;
			}
		}


		ret.imgStIdx = _offset;
		ret.imgSz = ImgSz;

		_offset += ImgSz;
		CloseHandle(hTool);
	}

	void ImageBuffer_VM_Eagle3D::Clear()
	{
		_offset = 0;
	}
}