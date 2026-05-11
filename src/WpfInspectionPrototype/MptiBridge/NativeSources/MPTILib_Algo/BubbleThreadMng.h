#include "CNN_Inference.h"
#include <memory>
#include <map>

namespace csml
{
	class InBub
	{
	public:
		InBub()
		{
			FOV_Idx = -1;
		};
		~InBub()
		{
			FOV.release();
			BCh.release();
		};

		cv::Mat FOV;
		cv::Mat BCh;

		int FOV_Idx;
	};

	class OutBub
	{
	public:
		OutBub()
		{
			FOV_Idx = -1;
		};
		~OutBub()
		{
			release();
		};

		bool empty(){ return FOV.empty(); }
		void release(){ FOV.release(); }

		cv::Mat FOV;
		int FOV_Idx;
	};

	class BubbleThreadMng
	{
	public:
		BubbleThreadMng(void)
		{
			fov_rows = 0;
			fov_cols = 0;
		};
		~BubbleThreadMng(void)
		{
			buffer_release();
		};
		
		void buffer_alloc(int cnt);
		void buffer_init(int fovid);
		cv::Mat buffer_get(int fovid);
		void buffer_release();

		int status_get(int fovid);
		BOOL check_empty_buffer();

		int buffer_count() { return buffer_cnt; }

		int Input_Image_rows() { return fov_rows; }
		int Input_Image_cols() { return fov_cols; }

		void flag_init();
		

		UINT _InferProc(csml::InBub In);


	public:
		Inference inference;
		OutBub* infer_buffer;
		int* infer_save_flag;

	protected:
		int buffer_cnt;
		int fov_rows;
		int fov_cols;
	};
}