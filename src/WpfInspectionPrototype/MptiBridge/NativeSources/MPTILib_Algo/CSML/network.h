#pragma once
#include "ANN_factory.h"
#include <algorithm>

namespace csml
{
	class AFX_EXT_CLASS network
	{
	public:
		typedef std::shared_ptr<network> ptr;
		typedef std::weak_ptr<network> wptr;

	public:
		network()
		{
			_nSelectGroup = 2;
		}
		network(archive & ar, loss_func::ptr loss)
		{
			_nSelectGroup = 2;
			loss_ptr(loss);
			Serialize(ar);
		}
		virtual ~network()
		{

		}

	public:
		virtual void init();
		virtual void init_connection();
		virtual cuFloat::ptr predict(cuFloat::ptr batch);
		virtual cuFloat::ptr forward(cuFloat::ptr batch);
		virtual cuFloat::ptr backward(bool multi_gpu_chk = false);
		virtual cuShort::ptr predict(cuShort::ptr batch);
		//virtual void sum_grad(std::vector<ptr> & nets, int nCurIdx, int st_gpu_id);
		virtual void sum_grad(std::vector<ptr> & nets, int nCurIdx);
		virtual void update_params(int gpu_cnt);
		virtual void update(float decay);

		virtual void loss_func_auto_create(loss_func::ptr & loss); // 이전버전과 호환성을 가지기 위한 함수

	public:
		network::ptr clone();
		void clear()
		{
			_nets.clear();
		}
		size_t size()
		{
			return _nets.size();
		}
		bool empty()
		{
			return _nets.empty();
		}
		void add(layer::ptr l)
		{
			_nets.emplace_back(l);
		}
		void insert(size_t index, layer::ptr l)
		{
			_nets.insert(_nets.begin() + index, l);
		}
		void erase(size_t index)
		{
			_nets[index]->erase_param();
			_nets.erase(_nets.begin() + index);
		}
		bool erase(CString layerName, CString newLinkLayerName = _T(""))
		{
			bool exist_name = false;
			int idx = 0;
			for(; idx < _nets.size(); idx++)
			{
				if(_nets[idx]->lname() == layerName)
				{
					exist_name = true;
					break;
				}
			}

			if(exist_name)
			{
				erase(idx);
				
				// 삭제된 Layer와 연결된 부분 link_layer_name으로 재링크
				if(newLinkLayerName != _T(""))
					replaceLinks(layerName, newLinkLayerName);

				return true;
			}
			else
				return false;
		}
		void replaceLinks(CString targetLayerName, CString newLayerName)
		{
			for(int idx = 0; idx < _nets.size(); idx++)
			{
				auto InNames = _nets[idx]->InputList();
				for(int n = 0; n < InNames.size(); n++)
				{
					if(InNames[n] == targetLayerName)
					{
						InNames[n] = newLayerName;
						_nets[idx]->InputList(InNames);
					}
				}
			}
		}
		int searchIndex(CString layerName)
		{
			for(int idx = 0; idx < _nets.size(); idx++)
				if(_nets[idx]->lname() == layerName)
					return idx;

			return -1;
		}
		void replace(size_t index, layer::ptr l)
		{
			_nets[index]->erase_param();
			_nets[index] = l;
			l->buffers_ptr(buffer());
			l->params_ptr(param());
		}
		void cutNonUnitConnection(CString layerName)
		{
			for(int idx = 0; idx < _nets.size(); idx++)
			{
				auto InNames = _nets[idx]->InputList();
				if(InNames.size() > 1)
				{
					for(int n = 0; n < InNames.size(); n++)
					{
						if(InNames[n] == layerName)
						{
							InNames.erase(InNames.begin() + n);
							_nets[idx]->InputList(InNames);
						}
					}
				}
			}
		}
		std::map<CString, int> linkCheck(CString layerName)
		{
			std::map<CString, int> linkList;
			for(int idx = 0; idx < _nets.size(); idx++)
			{
				auto InNames = _nets[idx]->InputList();
				for(int n = 0; n < InNames.size(); n++)
				{
					if(InNames[n] == layerName)
					{
						linkList[_nets[idx]->lname()] = n;
					}
				}
			}
			return linkList;
		}
		layer::ptr & last()
		{
			return _nets[_nets.size() - 1];
		}
		layer::ptr & first()
		{
			return _nets[0];
		}
		layer::ptr & operator[](size_t index)
		{
			return _nets[index];
		}

		void params_ptr(params::ptr val)
		{
			_params = val;
			std::for_each(_nets.begin(), _nets.end(), [val](layer::ptr & l)
			{
				l->params_ptr(val);
			});
		}
		void buffers_ptr(buffers::ptr val) 
		{
			_buffers = val;
			std::for_each(_nets.begin(), _nets.end(), [val](layer::ptr & l)
			{
				l->buffers_ptr(val);
			});
		}
		void loss_ptr(loss_func::ptr val)
		{
			_loss = val;
			for(int iters = 0 ; iters < _nets.size(); iters++)
			{
				_nets[iters]->loss_ptr(val);
			}
			//last()->loss_ptr(val);
			//first()->loss_ptr(val);
		}
		void Serialize(archive & ar);

		int down_sampling_rate();
		
		void select_group(int GroupId);
		int select_group() { return _nSelectGroup; }
		int group_count();

		params::ptr param() { return _params; }
		buffers::ptr buffer() { return _buffers; }
		loss_func::ptr loss() { return _loss; }
		layer::nets net(){return _nets;}
	protected:
		layer::nets _nets;
		params::ptr _params;
		buffers::ptr _buffers;
		loss_func::ptr _loss;
		int _nSelectGroup;
	public:
		bool _bGradCamReady;
	};
}