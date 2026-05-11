#pragma once
#include "session.h"

#include <vector>

namespace csml
{
	class AFX_EXT_CLASS session_multi_gpu : public session
	{
	public:
		session_multi_gpu(network::ptr nets, loss_func::ptr loss, float fWDecay = 0.0001f, int gpuId = 0, int gpuCount = 1);
		session_multi_gpu(CString sFilePath, int gpuId, int gpuCount = 1);
		virtual ~session_multi_gpu();

	protected:
		virtual void _Train();
		virtual cuFloat::ptr _Valid(Sampler * sampler);
		virtual void Serialize(archive & ar);
		
		void alloc_event();
		std::vector<HANDLE> _events;
	};
}


