#pragma once
#include "ExtInspRoot.h"

namespace ext
{
	class ImageBuffer_VM_Tools
	{
	public:
		ImageBuffer_VM_Tools();
		virtual ~ImageBuffer_VM_Tools();
		virtual bool Create(QWORD sizebytes);
		virtual bool Init();
		virtual void Exit();

		BYTE * Lock(ImgData pos);
		void Unlock(ImgData pos);


		QWORD _sizebytes;
		BYTE * _pBuffer;
	};

	class ImageBuffer_VM_Eagle3D
	{
	public:
		ImageBuffer_VM_Eagle3D();
		virtual ~ImageBuffer_VM_Eagle3D();

		virtual bool Init(int nTool_id, Ctrlee * ctrl);
		virtual void Exit();

		ImgData Push(BYTE * ptr, int Wid, int Len, int Ch, int widthbytes);

		void Clear();

		QWORD _offset;
		QWORD _sizebytes;
		Ctrlee * _ctrl;
		int _nToolId;
		DWORD _procId;
	};
};

