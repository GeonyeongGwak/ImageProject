#pragma once

#include "mat_factory.h"
#include <memory>
#include <map>
#include <set>
#include <opencv2\opencv.hpp>
#include "definitions.h"

namespace csml
{
	class AFX_EXT_CLASS params
	{
	public:
		typedef std::shared_ptr<params> ptr;
		typedef std::weak_ptr<params> wtr;
		class Items
		{
		public:
			Items()
			{

			}
			Items(cuFloat::ptr cval)
				: cuVal(cval)
			{

			}
			Items(cuShort::ptr cval_s)
				: cuVal_s(cval_s)
			{

			}
			cuFloat::ptr cuVal;
			cuFloat::ptr cuGrad;
			cuShort::ptr cuVal_s;
			cuShort::ptr cuGrad_s;
			cv::Mat cvVal;
			cv::Mat cvGrad;
			cv::Mat cvVal_s;
			cv::Mat cvGrad_s;
		};

	public:
		params() {}
		params(archive & ar, bool bUseHalf=false) { Serialize(ar, bUseHalf); }
		virtual ~params() {}

		static params::ptr alloc() { return ptr(new params()); }
		static params::ptr alloc(archive & ar, bool bUseHalf = false) { return ptr(new params(ar, bUseHalf)); }
	public:
		void copyTo(ptr obj);
		void add(ptr obj, float ahpha = 1.f, float beta=1.f);
		void clear()
		{
			_params.clear();
		}
		size_t size()
		{
			return _params.size();
		}
		bool empty()
		{
			return _params.empty();
		}
		void erase(CString key)
		{
			_params.erase(key);
		}
		void insert(CString key, cuFloat::ptr val)
		{			
			_params.insert(std::make_pair(key, Items(val)));
		}
		void insert_s(CString key, cuShort::ptr val)
		{	
			_params.insert(std::make_pair(key, Items(val)));
		}
		Items find(CString key)
		{
			auto iter = _params.find(key);
			return iter != _params.end() ? iter->second : Items();
		}
		Items & operator[](CString key)
		{
			return _params[key];
		}
		void down_val(CString key)
		{
			int szDims[4] = { _params[key].cuVal->size(0), _params[key].cuVal->size(1), _params[key].cuVal->size(2), _params[key].cuVal->size(3) };
			_params[key].cvVal = cv::Mat(4, szDims, CV_32FC1);
			_params[key].cuVal->download(_params[key].cvVal.ptr<float>());
		}
		void down_grad(CString key)
		{
			int szDims[4] = { _params[key].cuGrad->size(0) , _params[key].cuGrad->size(1), _params[key].cuGrad->size(2), _params[key].cuGrad->size(3) };
			_params[key].cvGrad = cv::Mat(4, szDims, CV_32FC1);
			_params[key].cuGrad->download(_params[key].cvGrad.ptr<float>());
		}
		void down_val_s(CString key)
		{
			int szDims[4] = { _params[key].cuVal_s->size(0), _params[key].cuVal_s->size(1), _params[key].cuVal_s->size(2), _params[key].cuVal_s->size(3) };
			_params[key].cvVal_s = cv::Mat(4, szDims, CV_16SC1);
			_params[key].cuVal_s->download(_params[key].cvVal.ptr<float>());
		}
		void down_grad_s(CString key)
		{
			int szDims[4] = { _params[key].cuGrad_s->size(0) , _params[key].cuGrad_s->size(1), _params[key].cuGrad_s->size(2), _params[key].cuGrad_s->size(3) };
			_params[key].cvGrad_s = cv::Mat(4, szDims, CV_16SC1);
			_params[key].cuGrad_s->download(_params[key].cvGrad.ptr<float>());
		}

		void Serialize(archive & ar, bool bUseHalf = false);

		//	protected:
		std::map<CString, Items> _params;
		cuFloat::ptr _Workspace;
		cuShort::ptr _hWorkspace;
	};

	class AFX_EXT_CLASS buffers
	{
	public:
		class item
		{
		public:
			class link_item
			{
			public:
				link_item(CString nm, int idx) :sName(nm), buf_idx(idx) {}
				link_item():buf_idx(0){}

			public:
				bool operator < (const link_item & lhs) const
				{
					return sName < lhs.sName;
				}
			public:
				CString sName;
				int buf_idx;
			};
		public:	
			typedef std::shared_ptr<item> ptr;
		public:
			item():loss(0.f) {}
		public:
			cuFloat::ptr & link(CString caller, int idx = 0)
			{
				refer.insert(link_item(caller, idx));
				return out;
			}
			cuShort::ptr & link_s(CString caller, int idx = 0)
			{
				refer.insert(link_item(caller, idx));
				return out_s;
			}
			void link_n(CString caller, int idx = 0)
			{
				refer.insert(link_item(caller, idx));
			}
			size_t refer_count()
			{
				return refer.size();
			}

			cuFloat::ptr in;
			cuFloat::ptr out;
			cuShort::ptr in_s;
			cuShort::ptr out_s;
			std::vector<cuFloat::ptr> grad;
			std::vector<cuShort::ptr> grad_s;
			cuFloat::ptr share[_maxGpuNum];
			cuShort::ptr share_s[_maxGpuNum];
			std::set<link_item> refer;
			float loss;
		};

	public:
		typedef std::shared_ptr<buffers> ptr;
		typedef std::weak_ptr<buffers> wtr;

	public:
		buffers() { sync = cuStream::ptr(new cuStream()); }
		virtual ~buffers() {}
		static buffers::ptr alloc() { return ptr(new buffers()); }

	public:
		void clear()
		{
			_items.clear();
		}
		size_t size()
		{
			return _items.size();
		}
		bool empty()
		{
			return _items.empty();
		}
		void insert(CString key, item val)
		{
			_items.insert(std::make_pair(key, val));
		}
		bool find(CString key)
		{
			return _items.find(key) != _items.end();

			//	if (iter == _items.end()) 
			//		throw std::exception("buffers out of range");
			//
			//	return iter->second ;
		}

		item & operator[](CString key)
		{
			return _items[key];
		}

		cuByte::ptr _Workspace;
		cuStream::ptr sync;

	protected:
		std::map<CString, item> _items;
	};
}
