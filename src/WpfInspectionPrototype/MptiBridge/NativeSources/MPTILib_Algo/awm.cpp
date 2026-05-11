#include "stdafx.h"
#include "awm.h"

#ifdef _MPTI_EXPORT
#include "MemoryManager.h"
#endif

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

awm * awm::s_obj = nullptr;

awm & awm::get()
{
	if (s_obj == nullptr)
#ifdef _MPTI_EXPORT
		s_obj = g_pMManager->pem_new<awm>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
#else
		s_obj = new awm();
#endif
	return *s_obj;
}

void awm::remove()
{
#ifdef _MPTI_EXPORT
	g_pMManager->pem_delete(s_obj, false);
#else
	delete s_obj;
#endif
	s_obj = nullptr;
}

void awm::check_modules(CString libPath)
{
	if (m_ShowNoRequireModuleMsg == false)
	{
		double idx = 0;
		double k = 2;
#if _Mercury
#if _DEBUG
		!PathFileExists(libPath + _T("18d.dll")) ? idx += pow(k, 0) :
			!PathFileExists(libPath + _T("asm18r.dll")) ? idx += pow(k, 2) :
			!PathFileExists(libPath + _T("cublas64_11.dll")) ? idx += pow(k, 7) :
			!PathFileExists(libPath + _T("cublasLt64_11.dll")) ? idx += pow(k, 8) :
			!PathFileExists(libPath + _T("fbg18r.dll")) ? idx += pow(k, 11) :
			!PathFileExists(libPath + _T("nvrtc-builtins64_111.dll")) ? idx += pow(k, 12) :
			!PathFileExists(libPath + _T("nvrtc64_111_0.dll")) ? idx += pow(k, 13) :
			!PathFileExists(libPath + _T("torch_18d.dll")) ? idx += pow(k, 16) :
			!PathFileExists(libPath + _T("vcruntime140_1d.dll")) ? idx += pow(k, 20) :
			!PathFileExists(libPath + _T("cudnn_cnn_infer64_8.dll")) ? idx += pow(k, 21) :
			!PathFileExists(libPath + _T("cudnn_ops_infer64_8.dll")) ? idx += pow(k, 22) :
			!PathFileExists(libPath + _T("libiomp5md.dll")) ? idx += pow(k, 24) :
			!PathFileExists(libPath + _T("myelin64_1.dll")) ? idx += pow(k, 25) :
			!PathFileExists(libPath + _T("nvinfer.dll")) ? idx += pow(k, 26) :
			!PathFileExists(libPath + _T("nvinfer_plugin.dll")) ? idx += pow(k, 27) :
			!PathFileExists(libPath + _T("nvonnxparser.dll")) ? idx += pow(k, 28) : idx += 0;
#else
		!PathFileExists(libPath + _T("18r.dll")) ? idx += pow(k, 0) :
			!PathFileExists(libPath + _T("asm18r.dll")) ? idx += pow(k, 2) :
			!PathFileExists(libPath + _T("cublas64_11.dll")) ? idx += pow(k, 7) :
			!PathFileExists(libPath + _T("cublasLt64_11.dll")) ? idx += pow(k, 8) :
			!PathFileExists(libPath + _T("fbg18r.dll")) ? idx += pow(k, 11) :
			!PathFileExists(libPath + _T("nvrtc-builtins64_111.dll")) ? idx += pow(k, 12) :
			!PathFileExists(libPath + _T("nvrtc64_111_0.dll")) ? idx += pow(k, 13) :
			!PathFileExists(libPath + _T("torch_18r.dll")) ? idx += pow(k, 16) :
			!PathFileExists(libPath + _T("vcruntime140_1.dll")) ? idx += pow(k, 20) :
			!PathFileExists(libPath + _T("cudnn_cnn_infer64_8.dll")) ? idx += pow(k, 21) :
			!PathFileExists(libPath + _T("cudnn_ops_infer64_8.dll")) ? idx += pow(k, 22) :
			!PathFileExists(libPath + _T("libiomp5md.dll")) ? idx += pow(k, 24) :
			!PathFileExists(libPath + _T("myelin64_1.dll")) ? idx += pow(k, 25) :
			!PathFileExists(libPath + _T("nvinfer.dll")) ? idx += pow(k, 26) :
			!PathFileExists(libPath + _T("nvinfer_plugin.dll")) ? idx += pow(k, 27) :
			!PathFileExists(libPath + _T("nvonnxparser.dll")) ? idx += pow(k, 28) : idx += 0;
#endif
#else
#if _DEBUG
		!PathFileExists(libPath + _T("16d.dll")) ? idx += pow(k, 0) :
			!PathFileExists(libPath + _T("asm16r.dll")) ? idx += pow(k, 2) :
			!PathFileExists(libPath + _T("c10_c16d.dll")) ? idx += pow(k, 4) :
			!PathFileExists(libPath + _T("caffe2_nvrtc.dll")) ? idx += pow(k, 5) :
			!PathFileExists(libPath + _T("cufft64_10.dll")) ? idx += pow(k, 6) :
			!PathFileExists(libPath + _T("cublas64_10.dll")) ? idx += pow(k, 7) :
			!PathFileExists(libPath + _T("cublasLt64_10.dll")) ? idx += pow(k, 8) :
			!PathFileExists(libPath + _T("curand64_10.dll")) ? idx += pow(k, 9) :
			!PathFileExists(libPath + _T("cusparse64_10.dll")) ? idx += pow(k, 10) :
			!PathFileExists(libPath + _T("fbg16r.dll")) ? idx += pow(k, 11) :
			!PathFileExists(libPath + _T("nvrtc-builtins64_102.dll")) ? idx += pow(k, 12) :
			!PathFileExists(libPath + _T("nvrtc64_102_0.dll")) ? idx += pow(k, 13) :
			!PathFileExists(libPath + _T("nvToolsExt64_1.dll")) ? idx += pow(k, 14) :
			!PathFileExists(libPath + _T("torch_16d.dll")) ? idx += pow(k, 16) :
			!PathFileExists(libPath + _T("torch_c16d.dll")) ? idx += pow(k, 18) :
			!PathFileExists(libPath + _T("vcruntime140_1d.dll")) ? idx += pow(k, 20) :
			!PathFileExists(libPath + _T("cudnn_cnn_infer64_8.dll")) ? idx += pow(k, 21) :
			!PathFileExists(libPath + _T("cudnn_ops_infer64_8.dll")) ? idx += pow(k, 22) :
			!PathFileExists(libPath + _T("libiomp5md.dll")) ? idx += pow(k, 24) :
			!PathFileExists(libPath + _T("myelin64_1.dll")) ? idx += pow(k, 25) :
			!PathFileExists(libPath + _T("nvinfer.dll")) ? idx += pow(k, 26) :
			!PathFileExists(libPath + _T("nvinfer_plugin.dll")) ? idx += pow(k, 27) :
			!PathFileExists(libPath + _T("nvonnxparser.dll")) ? idx += pow(k, 28) :
			!PathFileExists(libPath + _T("nvparsers.dll")) ? idx += pow(k, 29) : idx += 0;
#else
		!PathFileExists(libPath + _T("16r.dll")) ? idx += pow(k, 1) :
			!PathFileExists(libPath + _T("asm16r.dll")) ? idx += pow(k, 2) :
			!PathFileExists(libPath + _T("c10_c16r.dll")) ? idx += pow(k, 4) :
			!PathFileExists(libPath + _T("caffe2_nvrtc.dll")) ? idx += pow(k, 5) :
			!PathFileExists(libPath + _T("cufft64_10.dll")) ? idx += pow(k, 6) :
			!PathFileExists(libPath + _T("cublas64_10.dll")) ? idx += pow(k, 7) :
			!PathFileExists(libPath + _T("cublasLt64_10.dll")) ? idx += pow(k, 8) :
			!PathFileExists(libPath + _T("curand64_10.dll")) ? idx += pow(k, 9) :
			!PathFileExists(libPath + _T("cusparse64_10.dll")) ? idx += pow(k, 10) :
			!PathFileExists(libPath + _T("fbg16r.dll")) ? idx += pow(k, 11) :
			!PathFileExists(libPath + _T("nvrtc-builtins64_102.dll")) ? idx += pow(k, 12) :
			!PathFileExists(libPath + _T("nvrtc64_102_0.dll")) ? idx += pow(k, 13) :
			!PathFileExists(libPath + _T("nvToolsExt64_1.dll")) ? idx += pow(k, 14) :
			!PathFileExists(libPath + _T("torch_16r.dll")) ? idx += pow(k, 17) :
			!PathFileExists(libPath + _T("torch_c16r.dll")) ? idx += pow(k, 19) :
			!PathFileExists(libPath + _T("vcruntime140_1.dll")) ? idx += pow(k, 21) :
			!PathFileExists(libPath + _T("cudnn_cnn_infer64_8.dll")) ? idx += pow(k, 21) :
			!PathFileExists(libPath + _T("cudnn_ops_infer64_8.dll")) ? idx += pow(k, 22) :
			!PathFileExists(libPath + _T("libiomp5md.dll")) ? idx += pow(k, 24) :
			!PathFileExists(libPath + _T("myelin64_1.dll")) ? idx += pow(k, 25) :
			!PathFileExists(libPath + _T("nvinfer.dll")) ? idx += pow(k, 26) :
			!PathFileExists(libPath + _T("nvinfer_plugin.dll")) ? idx += pow(k, 27) :
			!PathFileExists(libPath + _T("nvonnxparser.dll")) ? idx += pow(k, 28) :
			!PathFileExists(libPath + _T("nvparsers.dll")) ? idx += pow(k, 29) : idx += 0;
#endif
#endif

		if (idx > 0)
		{
			CString msg;
			msg.Format(_T("%22d : "), idx);
			MessageBox(NULL, msg + _T("Insufficient AI DLLs !\n\nPlease download <Pemtron AI inference package setup> and install first"), _T("Warning"), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
			m_ShowNoRequireModuleMsg = true;
		}
	}
}

awm::awm(void)
{
	m_ShowNoRequireModuleMsg = false;
	m_bLoaded = false;
	m_hAiWrapper = nullptr;
	ai_make_session_p = nullptr;
	ai_get_file_info_p = nullptr;
	ai_load_file_p = nullptr;
	ai_load_file_raw_p = nullptr;
	ai_save_file_p = nullptr;
	ai_remove_file_p = nullptr;

	//	LoadDLL();
}
awm::~awm(void)
{
	ExitDLL();
}

bool awm::LoadDLL()
{
	TCHAR path[4096];
	GetModuleFileName(nullptr, path, sizeof(path));
	CString strPath = path;
	int i = strPath.ReverseFind('\\');
	strPath = strPath.Left(i + 1);

#if defined(_RENEWAL) && defined(_DEBUG)
	CString sDLLName = _T("AI.Wrapper_Rd.dll");
#elif defined(_RENEWAL) && defined(NDEBUG)
	CString sDLLName = _T("AI.Wrapper_R.dll");
#elif !defined(_RENEWAL) && defined(_DEBUG)
	CString sDLLName = _T("AI.Wrapper_d.dll");
#elif !defined(_RENEWAL) && defined(NDEBUG)
	CString sDLLName = _T("AI.Wrapper.dll");
#endif

	CString dllPath = strPath + sDLLName;

	if (m_hAiWrapper == nullptr)
		m_hAiWrapper = ::LoadLibrary(dllPath);

	if (m_hAiWrapper == nullptr)
	{
		if (GetFileAttributes((LPCTSTR)_T("C:\\Program Files\\Pemtron System")) == 0xFFFFFFFF)
			check_modules(strPath); // 실행파일 폴더 내 탐색
		else
			check_modules(_T("C:\\Program Files\\Pemtron System\\")); // AOI 장비 DLL 배포용 위치 탐색

		return false;
	}

	VERIFY(ai_make_session_p = (ai_make_session_t)GetProcAddress(m_hAiWrapper, "ai_make_session"));

	VERIFY(ai_get_file_info_p = (ai_get_file_info_t)GetProcAddress(m_hAiWrapper, "ai_get_file_info"));

	VERIFY(ai_load_file_p = (ai_load_file_t)GetProcAddress(m_hAiWrapper, "ai_load_file"));
	VERIFY(ai_load_file_raw_p = (ai_load_file_raw_t)GetProcAddress(m_hAiWrapper, "ai_load_file_raw"));
	VERIFY(ai_save_file_p = (ai_save_file_t)GetProcAddress(m_hAiWrapper, "ai_save_file"));
	VERIFY(ai_remove_file_p = (ai_remove_file_t)GetProcAddress(m_hAiWrapper, "ai_remove_file"));

	return true;
}

void awm::ExitDLL()
{
	if (m_hAiWrapper)
	{
		::FreeLibrary(m_hAiWrapper);
		m_hAiWrapper = NULL;

		ai_make_session_p = nullptr;
		ai_get_file_info_p = nullptr;
		ai_load_file_p = nullptr;
		ai_load_file_raw_p = nullptr;
		ai_save_file_p = nullptr;
		ai_remove_file_p = nullptr;
	}
}

isession * awm::ai_make_session(AiInfoHeader & infoHead, int nGpuID)
{
	if (m_hAiWrapper == nullptr)
		LoadDLL();
	if (ai_make_session_p == nullptr)
		return nullptr;

	return ai_make_session_p(infoHead, nGpuID);
}

bool awm::ai_get_file_info(LPCTSTR path, AiFileHeader & header)
{
	if (m_hAiWrapper == nullptr)
		LoadDLL();
	if (ai_get_file_info_p == nullptr)
		return nullptr;

	return ai_get_file_info_p(path, header);
}

bool awm::ai_load_file(LPCTSTR path, int nStIdx, int Count, int * nGpuID, isession ** sess)
{
	if (m_hAiWrapper == nullptr)
		LoadDLL();
	if (ai_load_file_p == nullptr)
		return false;
	else
	{
		return ai_load_file_p(path, nStIdx, Count, nGpuID, sess);
	}
}
void awm::ai_load_file_raw(LPCTSTR path, AiFileHeader & fileHead, AiInfoHeader ** info)
{
	if (m_hAiWrapper == nullptr)
		LoadDLL();
	if (ai_load_file_raw_p == nullptr)
		return;

	ai_load_file_raw_p(path, fileHead, info);
}
void awm::ai_save_file(LPCTSTR path, AiFileHeader & fileHead, AiInfoHeader ** infos)
{
	if (m_hAiWrapper == nullptr)
		LoadDLL();
	if (ai_save_file_p == nullptr)
		return;

	ai_save_file_p(path, fileHead, infos);
}
void awm::ai_remove_file(isession * sess)
{
	if (m_hAiWrapper == nullptr)
		LoadDLL();
	if (ai_remove_file_p == nullptr)
		return;

	ai_remove_file_p(sess);
}