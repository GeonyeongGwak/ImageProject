#pragma once

#ifdef _EXTCLIENT
#include "ExtLog.h"
#include "OCVParamDef.h"
#else
#include "OCV/OCVParamDef.h"
#endif 
#include "ipp.h" //
#define TOTAL_WND_CNT 13

namespace ext {
	typedef struct tagExtPOCRInspParam
	{
		int eFontType;
		int eDivisionArea;
		int nDivisionCnt;
		int nRequiredSocre;

		int ePOCRAlgoType[MAX_POCR_DIVISITION_CNTS];
		int nDivisionScore[MAX_POCR_DIVISITION_CNTS];
		int eRequiredArea[MAX_POCR_DIVISITION_CNTS];
	}ExtPOCRInspParam;
	typedef struct tagExtPOCRInfoParam
	{
		int nFontKey;	//0, 1, 2 ....A, B, C.... Z
		int nPOCRInspCnt;

		ExtPOCRInspParam vPOCRInspParam[MAX_POCR_PARAM_CNT];

	}ExtPOCRInfoParam;

	typedef struct tagExtPOCRFont
	{
		ExtPOCRInfoParam stFontInfo[64];
	}ExtPOCRFont;

	typedef struct tagExtProductionInfo
	{
		int partTotalCount;
		double resolX;
		double resolY;
		BOOL bRunSaveOK; // 0 1 
		BOOL bRunSaveNG; // 0 1 
		BOOL b2DImageOK; // 0 1 
		BOOL b3DImageOK; // 0 1 
		BOOL b2DImageNG; // 0 1 
		BOOL b3DImageNG; // 0 1 
		UINT32 unSaveOption;
		int nKeep3DImageSize;
		int m_nSave_3DRaw;
		int nImgNameRule; // 0 1 2

		//wchar_t sImageRootPath[256];
		//wchar_t cRefID[MAX_STRLEN];
		//int nImgSave2DCheck;
		int n2DSaveQuality; // Quality;
		int nLogLv;			//Log Level
		float fFactor_TR; // Fitting Data -  AOI 장비에서 받아오는 데이터 
		float fFactor_TB;
		float fFactor_TG;
		float fFactor_MR;
		float fFactor_MB;
		float fFactor_MGR;
		float fFactor_MGB;
		float fFactor_BR;
		float fFactor_BB;
		float fFactor_BGR;
		float fFactor_BGB;
		float fFactor_ACR;
		float fFactor_ACG;
		float fFactor_ACB;

		int nFovSizeX;
		int nFovSizeY;
		int nFovTotalCnt;
		BOOL bSaveAINGImage;
		double dPatternDiffMaxSize;
		double dPatternDiffDistance;
		BOOL bPatternDiffGrouping;
		BOOL bPatternDiffUseAxisSize;
		int nBoardSizeX;
		int nBoardSizeY;
		wchar_t sJobPath[256];
		int m_nMachineCode;
		int ColorMode;
		BOOL bSharedInOutImageSave;
		BOOL bMultiProcessThreadSave;
		BOOL bForeignDebugSave;
		int nAngleColor;
		int n2DImageQuality;
		int nUseNGPolygon;
		BOOL bUseAI;
		int nArrPriorityCode[kindofWholeNGTypeTotalNum];
		int nRedLgtPos_2DImg;
		int nGreenLgtPos_2DImg;
		int nBlueLgtPos_2DImg;
		int nSegThreshold;
		BOOL bUseCalibration;
	}ExtProductionInfo;
	typedef struct tagExtStartInitStruct
	{
		int nBigPartTool;
		int nForeignTool;
		int nToolCnt;
	}ExtStartInitStruct;

	typedef struct tagExtPreStartParam
	{
		int nFovSizeX;
		int nFovSizeY;
		double resolX;
		double resolY;

	}ExtPreStartParam;

	typedef struct tagExtMachineInfoBodyBlob
	{
		double dSearchPer_Default;
		double dSearchPer_BodyTip;
		int nBodyBlobOPT;
		int nBodyBlobPoint;
		void init()
		{
			dSearchPer_Default = 90.0;
			dSearchPer_BodyTip = 90.0;
			nBodyBlobOPT = 0;
			nBodyBlobPoint = 0;
		}
	}ExtMachineInfoBodyBlob;

	typedef struct tagExtMachineInfoPattern
	{
		BOOL bUseForceCharDiv;
		BOOL bUseMinScoreSet;
		int nMinScore;
		double dFactor_R;
		double dFactor_G;
		double dFactor_B;
		void init()
		{
			bUseForceCharDiv = false;
			bUseMinScoreSet = false;
			nMinScore = 60;
			dFactor_R = 1.0;
			dFactor_G = 1.0;
			dFactor_B = 1.0;
		}
	}ExtMachineInfoPattern;

	typedef struct tagExtMachineInfoGroupSkipData
	{
		BOOL m_bArrSkipGroupWnd[TOTAL_WND_CNT];
		BOOL m_bArrSkipGroupAlgo[TOTAL_WND_CNT];
		__int64 m_bArrSkipAlgo[TOTAL_WND_CNT];

	}ExtMachineInfoGroupSkipData;

	typedef struct tagExtMachineInfoFootWire
	{
		double WireSpecMarginY;
		double WireSpecMarginX;
		int FootPadTopWGrayLevel;
		int FootPatternMatchScore;
		int FootPatternMatchStopScore;
		int FootSobmn;
		int FootSobmx;
		BOOL FootMatch2D;
		BOOL FootMatch3D;
		float fWireDiffHigh;
		float fWireDiffLow;
		float fDiameterWire;
	}ExtMachineInfoFootWire;
	typedef struct tagExtAlgoMachineInfo
	{
		tagExtMachineInfoBodyBlob  stExtBodyBlob;
		tagExtMachineInfoPattern   stExtPattern;
		tagExtMachineInfoGroupSkipData stExtGroupSkipData;
		tagExtMachineInfoFootWire stExtFootWire;
	}ExtAlgoMachineInfo;
}

namespace ext
{
	class ZstdDll
	{
	public:
		// 기본 API
		typedef size_t(*ZSTD_compressBound_t)(size_t srcSize);
		typedef size_t(*ZSTD_compress_t)(void* dst, size_t dstCapacity, const void* src, size_t srcSize, int compressionLevel);
		typedef unsigned(*ZSTD_isError_t)(size_t code);
		typedef size_t(*ZSTD_decompress_t)(void* dst, size_t dstCapacity, const void* src, size_t compressedSize);
		typedef unsigned long long(*ZSTD_getFrameContentSize_t)(const void* src, size_t srcSize);
		typedef const char* (*ZSTD_getErrorName_t)(size_t code);

		// 고급 API (멀티스레드용)
		typedef void* (*ZSTD_createCCtx_t)();
		typedef size_t(*ZSTD_freeCCtx_t)(void* cctx);
		typedef size_t(*ZSTD_CCtx_setParameter_t)(void* cctx, int param, int value);
		typedef size_t(*ZSTD_compress2_t)(void* cctx, void* dst, size_t dstCapacity, const void* src, size_t srcSize);

		// zstd 파라미터 상수
		enum ZSTD_cParameter {
			ZSTD_c_compressionLevel = 100,
			ZSTD_c_nbWorkers = 400,
		};

		ZSTD_compressBound_t pfnCompressBound;
		ZSTD_compress_t pfnCompress;
		ZSTD_isError_t pfnIsError;
		ZSTD_decompress_t pfnDecompress;
		ZSTD_getFrameContentSize_t pfnGetFrameContentSize;
		ZSTD_getErrorName_t pfnGetErrorName;

		ZSTD_createCCtx_t pfnCreateCCtx;
		ZSTD_freeCCtx_t pfnFreeCCtx;
		ZSTD_CCtx_setParameter_t pfnCCtxSetParameter;
		ZSTD_compress2_t pfnCompress2;

		ZstdDll()
			: m_hModule(NULL)
			, pfnCompressBound(nullptr)
			, pfnCompress(nullptr)
			, pfnIsError(nullptr)
			, pfnDecompress(nullptr)
			, pfnGetFrameContentSize(nullptr)
			, pfnGetErrorName(nullptr)
			, pfnCreateCCtx(nullptr)
			, pfnFreeCCtx(nullptr)
			, pfnCCtxSetParameter(nullptr)
			, pfnCompress2(nullptr)
			, m_bMultiThreadSupport(false)
		{
		}

		~ZstdDll()
		{
			Unload();
		}
		CString GetErrorString(size_t code)
		{
			if (pfnGetErrorName)
			{
				const char* szErr = pfnGetErrorName(code);
				if (szErr)
				{
					CString sErr;
					sErr.Format(_T("%S"), szErr);  // ANSI → TCHAR 변환
					return sErr;
				}
			}
			CString sErr;
			sErr.Format(_T("unknown error (code=%zu)"), code);
			return sErr;
		}

		bool Load(LPCWSTR dllPath = L"C:\\Eagle3D_64x\\PROGRAM\\AlgoTool\\libzstd.dll") // Algorithm Tool 에 경로를 기본으로 설정, 필요시 외부에서 경로 지정 가능
		{
			if (m_hModule != NULL)
				return true;

			m_hModule = ::LoadLibraryW(dllPath);
			if (m_hModule == NULL)
			{
				CString sLog;
				sLog.Format(_T("[ZSTD] LoadLibrary failed: %s (error: %d)"), dllPath, GetLastError());
				ext::Log::add(sLog);
				return false;
			}

			// 필수 API
			pfnCompressBound = (ZSTD_compressBound_t)::GetProcAddress(m_hModule, "ZSTD_compressBound");
			pfnCompress = (ZSTD_compress_t)::GetProcAddress(m_hModule, "ZSTD_compress");
			pfnIsError = (ZSTD_isError_t)::GetProcAddress(m_hModule, "ZSTD_isError");
			pfnDecompress = (ZSTD_decompress_t)::GetProcAddress(m_hModule, "ZSTD_decompress");
			pfnGetFrameContentSize = (ZSTD_getFrameContentSize_t)::GetProcAddress(m_hModule, "ZSTD_getFrameContentSize");
			pfnGetErrorName = (ZSTD_getErrorName_t)::GetProcAddress(m_hModule, "ZSTD_getErrorName");

			if (!pfnCompressBound || !pfnCompress || !pfnIsError)
			{
				ext::Log::add(_T("[ZSTD] GetProcAddress failed for core functions"));
				Unload();
				return false;
			}

			// 고급 API (멀티스레드) - 없어도 기본 압축은 동작
			pfnCreateCCtx = (ZSTD_createCCtx_t)::GetProcAddress(m_hModule, "ZSTD_createCCtx");
			pfnFreeCCtx = (ZSTD_freeCCtx_t)::GetProcAddress(m_hModule, "ZSTD_freeCCtx");
			pfnCCtxSetParameter = (ZSTD_CCtx_setParameter_t)::GetProcAddress(m_hModule, "ZSTD_CCtx_setParameter");
			pfnCompress2 = (ZSTD_compress2_t)::GetProcAddress(m_hModule, "ZSTD_compress2");

			m_bMultiThreadSupport = (pfnCreateCCtx && pfnFreeCCtx && pfnCCtxSetParameter && pfnCompress2);

			CString sLog;
			sLog.Format(_T("[ZSTD] DLL loaded. MultiThread=%s, ErrorName=%s"),
				m_bMultiThreadSupport ? _T("YES") : _T("NO"),
				pfnGetErrorName ? _T("YES") : _T("NO"));
			ext::Log::add(sLog);

			return true;
		}

		void Unload()
		{
			if (m_hModule != NULL)
			{
				::FreeLibrary(m_hModule);
				m_hModule = NULL;
			}
			pfnCompressBound = nullptr;
			pfnCompress = nullptr;
			pfnIsError = nullptr;
			pfnDecompress = nullptr;
			pfnGetFrameContentSize = nullptr;
			pfnGetErrorName = nullptr;
			pfnCreateCCtx = nullptr;
			pfnFreeCCtx = nullptr;
			pfnCCtxSetParameter = nullptr;
			pfnCompress2 = nullptr;
			m_bMultiThreadSupport = false;
		}
		size_t Decompress(void* dst, size_t dstCapacity, const void* src, size_t srcSize)
		{
			if (!pfnDecompress || !pfnIsError) return 0;
			size_t result = pfnDecompress(dst, dstCapacity, src, srcSize);
			if (pfnIsError(result))
			{
				CString sLog;
				sLog.Format(_T("[ZSTD] Decompress error: %s"), GetErrorString(result));
				ext::Log::add(sLog);
				return 0;
			}
			return result;
		}

		// 원본 사이즈 조회 (압축된 데이터에서)
		unsigned long long GetFrameContentSize(const void* src, size_t srcSize)
		{
			if (!pfnGetFrameContentSize) return 0;
			return pfnGetFrameContentSize(src, srcSize);
		}
		bool IsLoaded() const { return m_hModule != NULL; }
		bool HasMultiThread() const { return m_bMultiThreadSupport; }

		size_t CompressBound(size_t srcSize)
		{
			if (!pfnCompressBound) return 0;
			return pfnCompressBound(srcSize);
		}

		// 기본 단일스레드 압축
		size_t Compress(void* dst, size_t dstCapacity, const void* src, size_t srcSize, int level = 3)
		{
			if (!pfnCompress || !pfnIsError) return 0;
			size_t result = pfnCompress(dst, dstCapacity, src, srcSize, level);
			if (pfnIsError(result))
			{
				CString sLog;
				sLog.Format(_T("[ZSTD] Compress error: %s (level=%d, srcSize=%zu)"), GetErrorString(result), level, srcSize);
				ext::Log::add(sLog);
				return 0;
			}
			return result;
		}

		// 멀티스레드 압축 (zstd 내장 nbWorkers 사용)
		// nWorkers=0이면 자동(CPU 코어 수)
		size_t CompressMT(void* dst, size_t dstCapacity, const void* src, size_t srcSize, int level = 5, int nWorkers = 0)
		{
			if (!m_bMultiThreadSupport)
				return Compress(dst, dstCapacity, src, srcSize, level);

			if (nWorkers == 0)
				nWorkers = (int)std::thread::hardware_concurrency(); //최대코어를가져다쓰므로 일단은..사용금지
			if (nWorkers < 1) nWorkers = 1;

			void* cctx = pfnCreateCCtx();
			if (!cctx)
			{
				ext::Log::add(_T("[ZSTD] CompressMT error: ZSTD_createCCtx() returned NULL"));
				return 0;
			}

			// 압축 레벨 설정
			size_t retLevel = pfnCCtxSetParameter(cctx, ZSTD_c_compressionLevel, level);
			if (pfnIsError(retLevel))
			{
				CString sLog;
				sLog.Format(_T("[ZSTD] CompressMT error: setParameter(compressionLevel=%d) failed: %s"), level, GetErrorString(retLevel));
				ext::Log::add(sLog);
			}
			// 멀티스레드 워커 수 설정
			size_t ret = pfnCCtxSetParameter(cctx, ZSTD_c_nbWorkers, nWorkers);
			if (pfnIsError(ret))
			{
				CString sLog;
				sLog.Format(_T("[ZSTD] CompressMT warning: setParameter(nbWorkers=%d) failed: %s - fallback to single thread"), nWorkers, GetErrorString(ret));
				ext::Log::add(sLog);
				// nbWorkers 미지원 시 단일스레드 폴백
				pfnCCtxSetParameter(cctx, ZSTD_c_nbWorkers, 0);
			}

			size_t result = pfnCompress2(cctx, dst, dstCapacity, src, srcSize);
			pfnFreeCCtx(cctx);

			if (pfnIsError(result))
			{
				CString sLog;
				sLog.Format(_T("[ZSTD] CompressMT error: compress2 failed: %s (srcSize=%zu, level=%d, workers=%d)"), GetErrorString(result), srcSize, level, nWorkers);
				ext::Log::add(sLog);
				return 0;
			}
			return result;
		}

	private:
		HMODULE m_hModule;
		bool m_bMultiThreadSupport;
	};

	inline ZstdDll& GetZstdDll()
	{
		static ZstdDll s_zstd;
		return s_zstd;
	}

	// .zst 파일 커스텀 헤더 구조
#pragma pack(push, 1)
	struct ZstdFrameFileHeader
	{
		unsigned long long ullOrigSize;  // 원본 크기 (8바이트)
		int nFrameCnt;                   // 프레임 수 (4바이트)
		int nSizeX;                      // sizeX (4바이트)
		int nSizeY;                      // sizeY (4바이트)
	};
#pragma pack(pop)

	// .zst 파일 디코드 결과
	struct ZstdDecodeResult
	{
		std::vector<BYTE> decompData;    // 압축 해제된 전체 데이터
		int nFrameCnt;
		int nSizeX;
		int nSizeY;
		bool bSuccess;

		ZstdDecodeResult() : nFrameCnt(0), nSizeX(0), nSizeY(0), bSuccess(false) {}

		// 특정 프레임 포인터 획득
		BYTE* GetFrame(int nFrameIndex)
		{
			if (!bSuccess || nFrameIndex < 0 || nFrameIndex >= nFrameCnt)
				return nullptr;
			size_t nFrameSize = (size_t)nSizeX * nSizeY;
			size_t offset = nFrameSize * nFrameIndex;
			if (offset + nFrameSize > decompData.size())
				return nullptr;
			return decompData.data() + offset;
		}

		// 프레임 1장 크기
		size_t GetFrameSize() const
		{
			return (size_t)nSizeX * nSizeY;
		}

		// 전체 프레임 포인터 배열 생성 (alpf_save_fbuk 호환)
		std::vector<PUINT8> GetFramePointers()
		{
			std::vector<PUINT8> vFrames(nFrameCnt);
			size_t nFrameSize = GetFrameSize();
			for (int i = 0; i < nFrameCnt; i++)
				vFrames[i] = decompData.data() + i * nFrameSize;
			return vFrames;
		}
	};

	// .zst 파일 디코드 (파일 경로)
	inline ZstdDecodeResult ZstdDecodeFile(const CString& sZstPath)
	{
		ZstdDecodeResult result;

		// 파일 읽기
		CT2CA szPath(sZstPath);
		std::string strPath(szPath);

		FILE* fp = nullptr;
		fopen_s(&fp, strPath.c_str(), "rb");
		if (!fp)
		{
			CString sLog;
			sLog.Format(_T("[ZSTD] Decode failed - cannot open: %s"), sZstPath);
			ext::Log::add(sLog);
			return result;
		}

		// 파일 크기
		fseek(fp, 0, SEEK_END);
		long fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (fileSize <= (long)sizeof(ZstdFrameFileHeader))
		{
			fclose(fp);
			ext::Log::add(_T("[ZSTD] Decode failed - file too small"));
			return result;
		}

		// 헤더 읽기
		ZstdFrameFileHeader header;
		if (fread(&header, sizeof(header), 1, fp) != 1)
		{
			fclose(fp);
			ext::Log::add(_T("[ZSTD] Decode failed - header read error"));
			return result;
		}

		// 압축 데이터 읽기
		long compDataSize = fileSize - sizeof(ZstdFrameFileHeader);
		std::vector<BYTE> compData(compDataSize);
		if (fread(compData.data(), 1, compDataSize, fp) != (size_t)compDataSize)
		{
			fclose(fp);
			ext::Log::add(_T("[ZSTD] Decode failed - compressed data read error"));
			return result;
		}
		fclose(fp);

		// 압축 해제
		ext::ZstdDll& zstd = ext::GetZstdDll();
		if (!zstd.IsLoaded() && !zstd.Load())
		{
			ext::Log::add(_T("[ZSTD] Decode failed - DLL not loaded"));
			return result;
		}

		result.decompData.resize((size_t)header.ullOrigSize);
		size_t decompSize = zstd.Decompress(
			result.decompData.data(), result.decompData.size(),
			compData.data(), compDataSize);

		if (decompSize == 0 || decompSize != header.ullOrigSize)
		{
			CString sLog;
			sLog.Format(_T("[ZSTD] Decode failed - decompress error (expected=%llu, got=%llu)"),
				header.ullOrigSize, (unsigned long long)decompSize);
			ext::Log::add(sLog);
			result.decompData.clear();
			return result;
		}

		result.nFrameCnt = header.nFrameCnt;
		result.nSizeX = header.nSizeX;
		result.nSizeY = header.nSizeY;
		result.bSuccess = true;

		CString sLog;
		sLog.Format(_T("[ZSTD] Decoded: %dx%d, Frames=%d, Size=%llu bytes"),
			header.nSizeX, header.nSizeY, header.nFrameCnt, header.ullOrigSize);
		ext::Log::add(sLog);

		return result;
	}

	// 메모리 버퍼에서 디코드 (파일 없이 버퍼에서 직접)
	inline ZstdDecodeResult ZstdDecodeBuffer(const BYTE* pCompData, size_t compSize)
	{
		ZstdDecodeResult result;

		if (!pCompData || compSize <= sizeof(ZstdFrameFileHeader))
			return result;

		// 헤더 파싱
		ZstdFrameFileHeader header;
		memcpy(&header, pCompData, sizeof(header));

		const BYTE* pZstdData = pCompData + sizeof(ZstdFrameFileHeader);
		size_t zstdSize = compSize - sizeof(ZstdFrameFileHeader);

		// 압축 해제
		ext::ZstdDll& zstd = ext::GetZstdDll();
		if (!zstd.IsLoaded() && !zstd.Load())
			return result;

		result.decompData.resize((size_t)header.ullOrigSize);
		size_t decompSize = zstd.Decompress(
			result.decompData.data(), result.decompData.size(),
			pZstdData, zstdSize);

		if (decompSize == 0 || decompSize != header.ullOrigSize)
		{
			result.decompData.clear();
			return result;
		}

		result.nFrameCnt = header.nFrameCnt;
		result.nSizeX = header.nSizeX;
		result.nSizeY = header.nSizeY;
		result.bSuccess = true;

		return result;
	}

}