#include "StdAfx.h"
#include "BubbleThreadMng.h"
#include "MemoryManager.h"

namespace csml
{
	void BubbleThreadMng::buffer_alloc(int cnt)
	{
		buffer_cnt = cnt;

		//infer_buffer = new OutBub[buffer_cnt];
		infer_buffer = g_pMManager->pem_new<OutBub>(true, buffer_cnt, (PCHAR)__FUNCTION__, __LINE__);
		
		//infer_save_flag = new int[buffer_cnt];
		infer_save_flag = g_pMManager->pem_new<int>(true, buffer_cnt, (PCHAR)__FUNCTION__, __LINE__);
		flag_init();
	}
	
	void BubbleThreadMng::buffer_init(int fovid)
	{
		for(int i = 0; i < buffer_cnt; i++)
		{
			if(!infer_buffer[i].empty())
			{
				if(infer_buffer[i].FOV_Idx == fovid)
				{
					infer_buffer[i].release();
					infer_buffer[i].FOV = cv::Mat();
					infer_buffer[i].FOV_Idx = -1;
					infer_save_flag[i] = 0;
				}
			}
		}
	}

	cv::Mat BubbleThreadMng::buffer_get(int fovid)
	{ 
		for(int i = 0; i < buffer_cnt; i++)
		{
			if(!infer_buffer[i].empty())
			{
				if(infer_buffer[i].FOV_Idx == fovid)
					return infer_buffer[i].FOV;
			}
		}
		
		return cv::Mat();
	}

	int BubbleThreadMng::status_get(int fovid)
	{
		int nRetStat = 0;

		for(int i = 0; i < buffer_cnt; i++)
		{
			if(!infer_buffer[i].empty())
			{
				if(infer_buffer[i].FOV_Idx == fovid)
					nRetStat = infer_save_flag[i];
			}
		}

		return nRetStat;
	}

	BOOL BubbleThreadMng::check_empty_buffer()
	{
		BOOL bIsEmpty = FALSE;

		for(int i = 0; i < buffer_cnt; i++)
		{
			if(!infer_buffer[i].empty())
			{
				if(infer_save_flag[i] == 0)
					bIsEmpty = TRUE;				
			}
		}

		return bIsEmpty;
	}

	void BubbleThreadMng::buffer_release()
	{
		if(buffer_cnt > 0)
			//delete [] infer_buffer;
			g_pMManager->pem_delete(infer_buffer, true);

		if(infer_save_flag != NULL)
			g_pMManager->pem_delete(infer_save_flag, true);
	}

	void BubbleThreadMng::flag_init()
	{
		for(int i = 0; i < buffer_cnt; i++)
			infer_save_flag[i] = 0; 
	}

	UINT BubbleThreadMng::_InferProc(csml::InBub In)
	{
		if(infer_buffer != nullptr)
		{
			if(fov_rows == 0 || fov_cols == 0)
			{
				fov_rows = In.FOV.rows;
				fov_cols = In.FOV.cols;
			}

			CString SaveNum;
			SaveNum.Format(_T("%d"), In.FOV_Idx);

			inference.half_mode = csml::runtime::tensor_core(csml::runtime::get_gpu());

			cv::Mat Label = inference.LabelMaker(In.FOV, SaveNum);

			csml::OutBub Res;
			Res.FOV = Label;
			Res.FOV_Idx = In.FOV_Idx;

			// 결과 저장할 버퍼 위치 탐색
			int idx = 0;
			while(1)
			{
				if(infer_save_flag[idx % buffer_cnt] == 0)
					break;
				else
					idx++;

				Sleep(10);
			}

			// 결과 저장 & flag(0->1)
			infer_buffer[idx] = Res;
			infer_save_flag[idx] = 1;

			return 0;
		}
		return 1;
	}
}