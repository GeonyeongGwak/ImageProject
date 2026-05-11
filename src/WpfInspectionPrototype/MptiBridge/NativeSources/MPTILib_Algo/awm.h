#pragma once

#ifdef _AITester
#include "../AIWrapper/pSession.h"
#elif _Mercury
#include "../AIWrapper/pSession.h"
#else
#include "./CSML/pSession.h"
#endif


typedef		isession *	(WINAPI *ai_make_session_t	)(AiInfoHeader & infoHead, int nGpuID);

typedef		bool		(WINAPI *ai_get_file_info_t	)(LPCTSTR path, AiFileHeader & header);

typedef		bool		(WINAPI *ai_load_file_t		)(LPCTSTR path, int nStIdx, int Count, int * nGpuID, isession ** sess);
typedef		void		(WINAPI *ai_load_file_raw_t	)(LPCTSTR path, AiFileHeader & fileHead, AiInfoHeader ** info);
typedef		void		(WINAPI *ai_save_file_t		)(LPCTSTR path, AiFileHeader & fileHead, AiInfoHeader ** infos);
typedef		void		(WINAPI *ai_remove_file_t	)(isession * sess);

class awm
{
public:
	awm(void);
	virtual ~awm(void);

	bool LoadDLL();
	void ExitDLL();

	isession * ai_make_session(AiInfoHeader & infoHead, int nGpuID);

	bool ai_get_file_info(LPCTSTR path, AiFileHeader & header);

	bool ai_load_file(LPCTSTR path, int nStIdx, int Count, int * nGpuID, isession ** sess);
	void ai_load_file_raw(LPCTSTR path, AiFileHeader & fileHead, AiInfoHeader ** info);
	void ai_save_file(LPCTSTR path, AiFileHeader & fileHead, AiInfoHeader ** infos);
	void ai_remove_file(isession * sess);

	void check_modules(CString libPath);

	HMODULE m_hAiWrapper;
	ai_make_session_t		ai_make_session_p;
	ai_get_file_info_t		ai_get_file_info_p;	
	ai_load_file_t			ai_load_file_p;	
	ai_load_file_raw_t		ai_load_file_raw_p;	
	ai_save_file_t			ai_save_file_p;	
	ai_remove_file_t		ai_remove_file_p;

	bool m_ShowNoRequireModuleMsg;
	bool m_bLoaded;
	static awm & get();
	static awm * s_obj;
	static void remove();
};
