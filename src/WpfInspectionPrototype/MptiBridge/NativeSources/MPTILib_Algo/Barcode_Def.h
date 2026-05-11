#ifdef _USRDLL
#define IESTATE dllexport
#else
#define IESTATE dllimport
#endif

#ifdef __cplusplus
#define USEDLL extern "C" __declspec(IESTATE)
#else
#define USEDLL __declspec(IESTATE)
#endif

//BCD_Open								: dll open(init)할때 사용 bDTKOn(SPI : FALSE, AOI : TRUE), MachineType(AOI : 1, SPI : 2)
//BCD_EncoderType_Order					: nType : SPI :  -1만사용(CItoP.dll.dat)  AOI : DTK : 0 ,  MIL order : 1, CItoP.dll.dat : -1
//BCD_Inspection						: insp
//BCD_Close								: 바코드 해제
//BCD_Get_License						: DTK, MIL 라이센스 확인(SPI 에서 사용)
//BCD_Inspect_Stop						: 검사 중지.
//BCD_PPDB_Alloc						: DataMatrixConfigWizard.exe 에서 필요.(SPI, AOI 사용 중지-> BCD_Set_PPBD 으로 경로 전달)
//BCD_Use_NEW_Barcode_Set				: 신규기능(PPDB) 사용 
//BCD_Use_NEW_Barcode_Get				: 신규기능(PPDB) 사용 확인
//BCD_Get_Mil_License					: mil license 확인
//BCD_RESULT_CENTER_POINT				: DataMatrixConfigWizard.exe 에서 필요.
//BCD_Insp_Return_PPDB					: 결과값으로 신규기능 사용 확인

#include <vector>
#include <map>

USEDLL void BCD_Open(int FovSizeX, int FovSizeY, BOOL bDTKOn, int MachineType); 

USEDLL int BCD_EncoderType_Order(int nInspType, int nFlipX, int nFlipY, int nCheckSum, int nCount);	// SHKang 2017/10/27

// nThresholdType : AOI에서 DTK 파라메터 중 ThresholdMode를 선택할 수 있도록 변경 하여 추가함.
USEDLL int BCD_Inspection(UCHAR * Src, int imgSizeX, int imgSizeY, POINT st, POINT ed, int type, int color,	CString *codeString, int nFlipX, int nFlipY, int nCheckSum, int nCount, int nThresholdType, int nUseAngle, double *dAngle, int nStdAngle, bool *bIsOrientationOK);


//USEDLL int BCD_ModuleProcessInput(UCHAR * Src, int imgSizeX, int imgSizeY, POINT st, POINT ed, int type, int color,	wchar_t* sModuleBarcodeNum, int nFlipX, int nFlipY, int nCheckSum, int nCount, int nThresholdType);

//USEDLL int BCD_ModuleProcessResult(CString *sBarcodeModuleNumber, CString *codeString);

//USEDLL int BCD_ModuleProcessResult_Vector(std::vector<std::vector<CString>> &Result_Id_CodeString);

//USEDLL int BCD_ModuleProcessIDSelectResult(CString sBarcodeModuleNumber, CString *codeString);

//USEDLL int BCD_ModuleProcessDataMatrixConfigWizardLoad(CString sJobPath, UCHAR * Src, int imgSizeX, int imgSizeY, POINT st, POINT ed);

USEDLL void BCD_Close();

USEDLL void BCD_MIL_Decode_Close();

USEDLL int BCD_Get_License();//SPI만 사용

USEDLL void BCD_Inspect_Stop();

//제거 예정
USEDLL int BCD_PPDB_Alloc(int ManualMode, int Direction, int Cols, int Rows, double ImageAngle, int DotShape);// LWW 2017/08/24


USEDLL int BCD_Set_PPBD(CString Job_Path);
//job 경로 : 받아서 진행.

USEDLL long BCD_Get_Mil_License();

USEDLL BOOL  BCD_Use_NEW_Barcode_Set(BOOL bUse);

USEDLL BOOL  BCD_Use_NEW_Barcode_Get();

USEDLL BOOL BCD_RESULT_CENTER_POINT(CPoint **section_centerpoint, int cols, int rows);

USEDLL BOOL BCD_Insp_Return_PPDB();

USEDLL BOOL BCD_Usable_PPDBFunction();

BOOL KillProcess();

USEDLL void BCD_Check_ParamOptimization(bool bCheck);

USEDLL BOOL BCD_OptiFileApply();

USEDLL void Delete_OptiFile();