#pragma once

#include <memory>

namespace csml
{
	class AFX_CLASS_EXPORT cuStream
	{
	public:
		class Aimpl;

		typedef std::shared_ptr<cuStream> ptr;
		static ptr alloc();
	public:
		cuStream();
	public:
		void query();
		void wait(size_t cnt = 0);
		size_t count();

		std::shared_ptr<Aimpl> _pimpl;
	};


	class AFX_EXT_CLASS runtime
	{
	public:
		static int gpu_count();
		static void set_gpu(int id);
		static int get_gpu();
		static bool tensor_core(int id); // 해당 id의 gpu의 tensor core 존재여부

		static void * malloc(size_t size);
		static void free(void * data);
		static void * mapped(size_t size);
		static void unmapped(void * data);
		static void * unified(size_t size);
		static void * pinned(size_t size); // temp memory 생성부, 메모리 해제 시, unmapped() 사용하기

		static void upload(void * dst, void * src, size_t size, bool bSync=false);
		static void upload2d(void * dst, size_t dpit, void * src, size_t spit, size_t wid, size_t len, bool bSync = false);
		static void download(void * dst, void * src, size_t size, bool bSync = false);
		static void memcpy(void * dst, void * src, size_t size, bool bSync = false, cuStream::ptr stream=nullptr);
		static void memcpy_uva(void * dst, void * src, size_t size, bool bSync = false);

		static void thread_sync();
		static void device_sync();

		static void debug_call();

		static int is_accessable(int from_gpu_id, int to_gpu_id);
		static void atatch_to(int target_gpu_id);
		static void detach_from(int target_gpu_id);

		static CString gpu_name(int id, size_t * totalGlobalMem=nullptr);
	};
}

