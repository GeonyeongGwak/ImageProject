
#ifdef DLLEXPORT
#define DLLTYPE_OCR __declspec(dllexport)
#else
#define DLLTYPE_OCR __declspec(dllimport)
#endif

extern "C" DLLTYPE_OCR void OCR_Dll_CreateInterface();
extern "C" DLLTYPE_OCR void OCR_Dll_DestroyInterface();
extern "C" DLLTYPE_OCR CString OCR_Dll_ReadFileOCR(CString sPathOCR);
extern "C" DLLTYPE_OCR CString OCR_Dll_ReadOCR(UCHAR* ptrFontImg, int nSizeX, int nSizeY);
