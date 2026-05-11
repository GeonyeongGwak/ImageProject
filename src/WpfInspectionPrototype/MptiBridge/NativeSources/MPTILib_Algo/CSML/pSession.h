#pragma once

#include "opencv2/opencv.hpp"
#include "AI_TypeDef.h"
#include "iSession.h"
#include "pyTorchAPI.h"		//ºôµå ÈÄ ÀÌº¥Æ®¿¡¼­ copy µÊ
#include "TensorRTAPI.h"	//ºôµå ÈÄ ÀÌº¥Æ®¿¡¼­ copy µÊ

#ifdef _AI_WRAPPER
	#define  DECL_DLL extern "C" __declspec(dllexport)
#else
	#define  DECL_DLL extern "C" __declspec(dllimport)
#endif // _AI_WRAPPER


typedef enum _MAIN_VER_{ Ver2=2, Ver4=4 }eMainVer;

//DECL_DLL void ai_init(int nMainVer);
//DECL_DLL void ai_exit();

DECL_DLL isession * ai_make_session(AiFileHeader & fileHead, AiInfoHeader & infoHead, int nGpuID);

DECL_DLL bool ai_get_file_info(LPCTSTR path, AiFileHeader & header);

DECL_DLL bool ai_load_file(LPCTSTR path, int nStIdx, int Count, int * nGpuID, isession ** sess);
DECL_DLL void ai_load_file_raw(LPCTSTR path, AiFileHeader & fileHead, AiInfoHeader ** info);
DECL_DLL void ai_save_file(LPCTSTR path, AiFileHeader & fileHead, AiInfoHeader ** infos);
DECL_DLL void ai_remove_file(isession * sess);

class SessMng
{
public:
	SessMng()
	{
		_sess = nullptr;
	}
	SessMng(isession * ss)
		: _sess(ss)
	{
	}
	~SessMng()
	{
		_sess = nullptr;
	}

	isession * operator ->()
	{
		return _sess;
	}
	isession * get()
	{
		return _sess;
	}
	operator isession *()
	{
		return _sess;
	}
	void reset()
	{
		if (_sess)
		{
			ai_remove_file(_sess);
			_sess = nullptr;
		}
	}

protected:
	isession *_sess;
};