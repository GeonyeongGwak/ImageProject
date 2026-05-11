#pragma once

#include <memory>
#include <vector>
#include <assert.h>
#include "runtime.h"
#include "archive.h"
#include "npps.h"
#include "definitions.h"


#ifdef _AFXEXT
	#define IESTATE dllexport
#else
	#define IESTATE dllimport
#endif

#ifdef __cplusplus
	#define USEDLL extern "C" __declspec(IESTATE)
#else
	#define USEDLL __declspec(IESTATE)
#endif



namespace csml
{
	class AFX_CLASS_EXPORT memoryMng
	{
	public:			
		bool _bUse;
		void * _pBuffer;
		size_t _size;		


	public:

		static void init();
		static void* data(size_t sz);  // 불가능 표기 & 저장할 공간 할당
		static void release(void * pRemove);
		static void releaseAll();
		static int getSize(void * pTarget);

	private:
		static memoryMng _buffer[_maxGpuNum][_maxTempMem];
	};

	template<typename T, typename Tpool=memoryMng> class AFX_CLASS_EXPORT array : public std::enable_shared_from_this<array<T>>
	{
	public:
		typedef std::shared_ptr<array<T>>	ptr;
		typedef std::shared_ptr<array<T>>	wtr;
		typedef enum _Access { Device, Mapped, Unified, Temp } eAccess;


	public:
		
		//hostdata에 데이터 저장한 후 gpu 메모리를 해제
		void backup()
		{
			if (_parent != nullptr)
				return;	// TODO:

			if(_hostData == nullptr)
				_hostData = runtime::pinned(elemSize()*total());
			runtime::download(_hostData, _data, elemSize()*total());
			memoryMng::release(_data_org);
		}
		
		//host memory를 gpu 메모리에 부여
		void restore()
		{
			if (_parent != nullptr)
				return;	// TODO

			_data = memoryMng::data(elemSize() * total());
			runtime::upload(_data, _hostData, elemSize() * total());
			runtime::mapped(elemSize() * total());
		}

	protected:
		array(void)
		{
			init();
		}
		array(archive& ar)
		{
			init();
			Serialize(ar);
		}
		array(size_t n, size_t c, size_t h, size_t w, eAccess access = eAccess::Device)
		{
			init();
			create(n,c,h,w, access);
		}

	public:
		virtual ~array(void)
		{
			release();
		}

		static ptr alloc()
		{
			return ptr(new array<T>());
		}
		static ptr alloc(archive& ar)
		{
			return ptr(new array<T>(ar));
		}
		static ptr alloc(size_t n, size_t c, size_t h=1, size_t w=1, eAccess access = eAccess::Device)
		{
			return ptr(new array<T>(n, c, h, w, access));
		}
		static bool alloc(ptr & dst, ptr std, eAccess access = eAccess::Device)
		{
			size_t n(std->size(0)), c(std->size(1)), h(std->size(2)), w(std->size(3));

			return alloc(dst, n, c, h, w, access);
		}
		static bool alloc(ptr & dst, size_t n, size_t c, size_t h=1, size_t w=1, eAccess access = eAccess::Device)
		{
			ptr tmp = dst;
			while (tmp != nullptr && tmp->parent() != nullptr)
				tmp = tmp->parent();

			bool bModified(false);
			size_t total = n*c*h*w;

			if (dst != nullptr && dst->total() == total && 
				dst->size(0) == n && dst->size(1) == c &&
				dst->size(2) == h && dst->size(3) == w) // 같을 때
				return bModified;

			if (tmp == nullptr || tmp->total() < total)
			{
				dst = alloc(n, c, h, w, access);
				bModified = true;
			}
			else if (tmp != nullptr && tmp->total() > total)
			{
				dst = tmp->reshapeTo(n, c, h, w);
				bModified = true;
			}
			else if (tmp != nullptr && tmp->total() == total)
			{
				if(tmp->size(0) == n && tmp->size(1) == c &&
					tmp->size(2) == h && tmp->size(3) == w) // 같을 때
					dst = tmp;
				else
					dst = tmp->reshapeTo(n, c, h, w);
				bModified = true;
			}

			return bModified;
		}
	public:
		virtual bool create(size_t n, size_t c, size_t h, size_t w, eAccess access = eAccess::Device)
		{
			release();

			_access = access;
			_size = new size_t[5];
			_size[0] = n;
			_size[1] = c;
			_size[2] = h;
			_size[3] = w;
			_size[4] = n*c*h*w;

			if(_access == eAccess::Mapped)
				_data_org = _data = (T *)runtime::mapped(elemSize() * total()); 
			else if(_access == eAccess::Unified)
				_data_org = _data = (T *)runtime::unified(elemSize() * total());
			else if (_access == eAccess::Temp)
				_data_org = _data = (T*)memoryMng::data(elemSize() * total());
			else
				_data_org = _data = (T *)runtime::malloc(elemSize() * total());

			calc_mat_size();

			assert(_data != nullptr);
// 			_refcount = new int(1);
//			runtime::syncronize();
			return _data!=nullptr;
		}
		virtual void release()
		{
			if (_data_org != nullptr && _parent == nullptr)
			{
				if (_access == eAccess::Mapped)
					runtime::unmapped(_data_org); 
				else if (_access == eAccess::Temp)
					memoryMng::release(_data_org);
				else
					runtime::free(_data_org);

				delete[] _size;
				_size = nullptr;
				_data = _data_org = nullptr;
			}
			else if (_size != nullptr)
			{
				delete[] _size;
				_size = nullptr;
				_data = _data_org = nullptr;
			}
		}

		ptr roi(int n, int size)
		{
			ptr nMat(alloc());
			roiTo(nMat);

			nMat->_size[0] = size;
			nMat->_data = data(n);
			nMat->calc_mat_size();

			return nMat;
		}
		ptr roi(int n, int c, int size)
		{
			ptr nMat(alloc());
			roiTo(nMat);
			
			nMat->_size[0] = 1;
			nMat->_size[1] = size;
			nMat->_data = data(n, c);
			nMat->calc_mat_size();

			return nMat;
		}
		ptr roi(int n, int c, int h, int size)
		{
			ptr nMat(alloc());
			roiTo(nMat);

			nMat->_size[0] = 1;
			nMat->_size[1] = 1;
			nMat->_size[2] = size;
			nMat->_data = data(n, c, h);
			nMat->calc_mat_size();

			return nMat;
		}

		ptr reshapeTo(size_t n, size_t c, size_t h, size_t w)
		{
			ptr nMat(alloc());
			roiTo(nMat);

			nMat->_size[0] = n;
			nMat->_size[1] = c;
			nMat->_size[2] = h;
			nMat->_size[3] = w;
			nMat->_size[4] = n*c*h*w;
			nMat->calc_mat_size();

			return nMat;
		}

		virtual size_t size(int i)
		{
			return _size[i];
		}
		virtual size_t total() { return _size[4]; }
		virtual int elemSize() { return sizeof(T); }

		virtual void reshape(int l, int w)
		{
			_size[1] = (_size[1] * _size[2] * _size[3]) / (l * w);
			_size[2] = l;
			_size[3] = w;
		}

		

		virtual T * data()
		{
			return _data;
		}
		virtual T * data(int i)
		{
			return &_data[i * _size[1] * _size[2] * _size[3]];
		}
		virtual T * data(int n, int c)
		{
			return &_data[n * _size[1] * _size[2] * _size[3] + c * _size[2] * _size[3]];
		}
		virtual T * data(int n, int c, int h)
		{
			return &_data[n * _size[1] * _size[2] * _size[3] + c * _size[2] * _size[3] + h * _size[3]];
		}

		template<typename T2>
		T2 * data()
		{
			return (T2*)(_data);
		}

		virtual std::vector<T> download(bool bSync=false)
		{
			std::vector<T> buf(total());
			runtime::download(&buf[0], _data, elemSize()*total(), bSync);
			return buf;
		}
		virtual void download(T * mem, bool bSync = false)
		{
			runtime::download(mem, _data, elemSize()*total(), bSync);
		}
		virtual void upload(T * mem, bool bSync = false)
		{
			runtime::upload(_data, mem, elemSize()*total(), bSync);
		}
		virtual void upload2d(T * mem, size_t pitch, bool bSync = false)
		{
			runtime::upload2d(data(), size(3), mem, pitch, size(3), size(2), bSync);
		}

		ptr clone(cuStream::ptr sync = nullptr)
		{
			ptr res(new array<T>(_size[0], _size[1], _size[2], _size[3], _access));
			copyTo(res, false, sync);

			return res;
		}
		void copyTo(ptr & lhs, bool bSync = false, cuStream::ptr sync=nullptr)
		{
			if (lhs == nullptr)
				lhs = ptr(new array<T>(_size[0], _size[1], _size[2], _size[3], _access));

			if (lhs->total() != total())
				return;

			if(_access != eAccess::Unified)
				runtime::memcpy(lhs->data(), data(), elemSize()*total(), bSync, sync);
			else
				runtime::memcpy_uva(lhs->data(), data(), elemSize()*total(), bSync);
		}

		virtual bool empty()
		{
			return _data_org == nullptr /*|| _refcount == nullptr*/;
		}

		ptr parent() { return _parent; }

		ptr parent_top()
		{
			cuFloat::ptr tmp = _parent;
			while (_parent == nullptr && _parent->parent() != nullptr)
				tmp = _parent->parent();
			return tmp;
		}

		virtual size_t rows()
		{
			return _rows;
		}
		virtual size_t cols()
		{
			return _cols;
		}

		eAccess access() { return _access; }
		void Serialize(archive& ar)
		{
			if (ar.IsStoring())
			{
				ar_item arm, arm2;
				int type = 0;
				arm2 << type;
				arm2 << _size[0];
				arm2 << _size[1];
				arm2 << _size[2];
				arm2 << _size[3];
				arm2 << _size[4];
				arm2.WriteTo(arm);

				auto mem = download();
				arm.get().Write(&mem[0], mem.size()*sizeof(T));

				arm.WriteTo(ar);
			}
			else
			{
				ar_item arm(ar);
				ar_item arm2(arm);

				int type = -1;
				arm2 >> type;
				size_t sz[5];
				arm2 >> sz[0];
				arm2 >> sz[1];
				arm2 >> sz[2];
				arm2 >> sz[3];
				arm2 >> sz[4];

				create(sz[0], sz[1], sz[2], sz[3]);
				if (_size[4] > 0)
				{
					std::vector<T> mem(total());
					arm.get().Read(&mem[0], total()*sizeof(T));
					upload(&mem[0]);
				}
			}
		}

	public:
		static void concat(std::vector<ptr> inputs, ptr & out, int axis=1)
		{
			if (axis == 1)
			{
				ptr tmp = out;

				size_t c_cnt(0);
				for (size_t i=0; i<inputs.size(); i++)
					c_cnt += inputs[i]->size(1);

				size_t n(inputs[0]->size(0)), c(c_cnt), h(inputs[0]->size(2)), w(inputs[0]->size(3));

				size_t total = n * c * h * w;
				if (tmp == nullptr || tmp->total() < total)
					out = alloc(n, c, h, w);
				else
					out = tmp->reshapeTo(n, c, h, w);
			
				size_t c_idx(0);
				for (size_t i = 0; i < inputs.size(); i++)
				{
					size_t c_cnt_r(inputs[i]->size(1));
					
					for (size_t ni = 0; ni < n; ni++)
					{
						ptr src = inputs[i]->roi(ni, 1);
						ptr dst = out->roi(ni, c_idx, c_cnt_r);
						src->copyTo(dst);
					}

					c_idx += c_cnt_r;
				}
			}
			else if (axis == 0)
			{
				ptr tmp = out;

				size_t n_cnt(0);
				for (size_t i = 0; i < inputs.size(); i++)
					n_cnt += inputs[i]->size(0);

				size_t n(n_cnt), c(inputs[0]->size(1)), h(inputs[0]->size(2)), w(inputs[0]->size(3));

				size_t total = n * c * h * w;
				if (tmp == nullptr || tmp->total() < total)
					out = alloc(n, c, h, w);
				else
					out = tmp->reshapeTo(n, c, h, w);

				size_t n_idx(0);
				for (size_t i = 0; i < inputs.size(); i++)
				{
					size_t n_cnt_r(inputs[i]->size(0));
					ptr dst = out->roi(n_idx, n_cnt_r);
					inputs[i]->copyTo(dst);
					n_idx += n_cnt_r;
				}
			}
		}
		static void split(ptr input, std::vector<ptr> &outputs, std::vector<int> sz, int axis = 1)
		{
			if (axis == 1) // c 변형
			{
				if(outputs.size()<sz.size())
					outputs.resize(sz.size());
				size_t c_idx(0);
				for (size_t i = 0; i < sz.size(); i++)
					alloc(outputs[i], input->size(0), sz[i], input->size(2), input->size(3), input->_access);

				for (size_t i = 0; i < sz.size(); i++)
				{
					size_t c_cnt_r(sz[i]);

					for (size_t ni = 0; ni < outputs[0]->size(0); ni++)
					{
						ptr src = input->roi(ni, c_idx, c_cnt_r);
						ptr dst = outputs[i]->roi(ni, 1);
						src->copyTo(dst);
					}

					c_idx += c_cnt_r;
				}
			}
			else if (axis == 0) // n 변형
			{
				if(outputs.size()<sz.size())
					outputs.resize(sz.size());
				size_t n_idx(0);
				for (size_t i = 0; i < sz.size(); i++)
				{
					alloc(outputs[i], sz[i], input->size(1), input->size(2), input->size(3), input->_access);

					size_t n_cnt_r(outputs[i]->size(0));
					ptr src = input->roi(n_idx, n_cnt_r);
					src->copyTo(outputs[i]);
					n_idx += n_cnt_r;
				}
			}
		}

		void set(T constant)
		{
			npps::set(constant, data(), total());
		}
		void operator*=(T val) {
			npps::mulc(val, data(), total());
		}
		void operator+=(T val) {
			npps::addc(val, data(), total());
		}
		void operator-=(T val) {
			npps::subc(val, data(), total());
		}
		void operator/=(T val) {
			npps::divc(val, data(), total());
		}
	protected:
		virtual void init()
		{
			//memoryMng::init();
// 			_refcount = nullptr;
			_rows = _cols = 0;
			_data = _data_org = nullptr;
			_size = nullptr;
		}
		virtual void roiTo(ptr lhs)
		{
			assert(_access != Temp);

			lhs->_size = new size_t[5];
			memcpy(lhs->_size, _size, sizeof(size_t) * 5);
			lhs->_data =_data;
			lhs->_data_org = _data_org;
// 			lhs->_refcount = _refcount;
			lhs->_access = _access;
			lhs->_parent = shared_from_this();
// 			if (_refcount)
// 				(*_refcount)++;

		}
		void calc_mat_size()
		{
			_size[4] = _size[0] * _size[1] * _size[2] * _size[3];
			_cols = _size[0];
			_rows = _size[1] * _size[2] * _size[3];
		}

		T * _data;
		T * _data_org;
		void * _hostData;

		size_t * _size;
// 		int * _refcount;
		eAccess _access;

		size_t _rows;
		size_t _cols;

	private:
		ptr _parent;
	};

	typedef array<BYTE> cuByte;
	typedef array<short> cuShort;
	typedef array<int> cuInt;
	typedef array<float> cuFloat;
	typedef array<double> cuDouble;
}

