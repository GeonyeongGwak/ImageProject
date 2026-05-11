#pragma once
//MIL 

#include "ProcMil.h"

#include "MModelManager.h"
#include <memory>

enum milprocOCR_result
{
	eSTR_SUCCESS = 0,
	eSTR_FAIL,
	eSTR_OVERWRITE
};

enum stringreader_control
{
	eSTR_CONTEXT = 0,
	eSTR_FONT,
	eSTR_STRING
};

enum stringreader_background
{
	eSTR_BLACK = 0,
	eSTR_WHITE
};

enum stringreader_width
{
	eSTR_MEAN = 0,
	eSTR_QUARTER,	
	eSTR_MAX,
	eSTR_MIN,
	eSTR_INFINITE,
	eSTR_USER
};

enum stringreader_type
{
	eSTR_ANY = 0,
	eSTR_DIGIT,
	eSTR_LETTER,
	eSTR_LOWERCASE,
	eSTR_UPPERCASE,
};

enum stringreader_threshmode
{
	eSTR_LOCAL  = 0,
	eSTR_LOCAL_WITH_RESEG ,
	eSTR_USER_DEFINED,
	eSTR_THRESMODE_COUNT
};


class CProcMil_OCR : public CProcMil
{
public:
	CProcMil_OCR(void);
	virtual ~CProcMil_OCR(void);

private:
	//Im::PIL_ID m_milFontContext; //mil string reader context (for Alloc Font)
	Im::PIL_ID m_milStrContext_Temp;
	Im::PIL_ID m_milStrContext;  //mil string reader context (for Search String)
	Im::PIL_ID m_milStrResult;   //mil string reader result
	Im::PIL_ID m_milStrResult_OLD;  

// 	Im::PIL_ID m_milBlobFeature;	// YJS 2016/11/04 : Blob관련 오류 창 원인 제거
// 	Im::PIL_ID m_milBlobResult;

	std::shared_ptr<tagModelFile> m_ModelFile;

	std::shared_ptr<CMModelManager<tagModelFile>> m_ModelMng;
public:
	double doubeZeroBuf[MAX_STRLEN];
public:
	int m_nUsecontinue;
public:
	int ImgWidth;
	int ImgHeight;
//mil init , free
public:
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int FreeMil();


//string reader
private:
	template<typename T>
	int StrInquire(int index, int type, T * controlValue);
	int StrControl(int index, int type, double controlValue);
	int StrEditFont(int index, int operation, int operationMode, MIL_INT param1, const void *param2);

public:

	int AddNewFont();   //Add a new empty user defined font to the context.
	int AddFont();
	int DeleteFont(int fontIndex = 0);
	int SetEditFont(UCHAR* srcImg, int sizeX, int sizeY, double angle, int foreGround, CString font, int fontIndex, 
							int rmLineCnt, int* rmLinePos, int* rmLineWid, int threshMode, int threshVal, int* retThresh = NULL);
	int EditFont(Im::PIL_ID srcImg, CString font, int fontIndex, int foreGround);
	int SetNomalizeFont(int refer, int size, int fontIndex = 0);  // Normalize the characters of the font to an appropriate size.
	int SetSpaceWidthFont(int mode, double width=0.);

	int AddString();   //Add a new empty user defined strings models to the context.
	int DeleteString();
	int SetCharMaxCount(int count);   //Set number of expected characters for all string models to 'count' characters.
	int SetCharMinCount(int count);
	int SetStringCount(int count);   //Set number of strings to read.
	int SetSpaceChar(int space);
	int SetStrAcceptScore(double score);  //set string  acceptance score...   ( value : 0.0 ~ 100.0 )
	int SetStrTargetAcceptScore(double score);
	int SetCharAcceptScore(double score);  //set character  acceptance score...   ( value : 0.0 ~ 100.0 )
	int SetCharSimilAcceptScore(double score);
	int SetCharHomoAcceptScore(double score);
	int SetCharAspectRatio(double maxVal, double minVal);
	int SetCharScaleFactor(double maxVal, double minVal);
	int SetStringScaleFactor(double maxVal, double minVal);
	int SetEnableCharAngle(bool val);
	int SetConsecutiveSpace(int space);	
	int SetThreshHoldMode(int threshMode , int value = -1);
	int SetStrForeGoundColor(int foreGround);
	int	SetThickenChar(int thickenChar);

	int GetCharMinCount();
	int GetCharMaxCount();
	double GetCharSimilAcceptScore();
	double GetCharHomoAcceptScore();
	int GetStrForeGoundColor();

	bool IsPreprocess();
	void Preprocess();
	void UnPreprocess();
	int GetStringModelCount();

	int StringReadStart(UCHAR* userBuff, int sizeX, int sizeY);
	int StringReadStart(void* milBuffID, int cX, int cY, int sizeX, int sizeY, double angle, int foreGround);

	int SaveStrFont(CString path);
	int LoadStrFont(CString path, int* retFontCount, long * retFontSize = NULL, int nInspectionMode = eMI_WindowTeaching);
	int GetFontCount(Im::PIL_ID milContext = M_NULL);

	int SetConstraint(int charPos, int type, wchar_t charList = NULL);

	Im::PIL_ID GetContextID();
	BOOL ExistContext();
	BOOL ExistFont();
	int GetStringCount(int* count);
	int GetCharCount(int* count);
	CString GetString(int index = 0);
	int GetStringScore(double* score,int index = 0);
	int GetCharScore(double* score,int index = 0);
	int GetStringAngle(double* angle,int index = 0);
	int GetStringPos(double* x, double* y,int index = 0);
	BOOL IsFoundStr();


	int GetBuffDrawFont(Im::PIL_ID dst, MIL_DOUBLE color, int fontIndex = 0);
	int GetBuffDrawString(Im::PIL_ID dst, MIL_DOUBLE color);
	int GetBuffDrawBox(Im::PIL_ID dst, MIL_DOUBLE color);

	int GetBuffDrawFont(UCHAR* dst, int sizeX, int sizeY, MIL_DOUBLE color, int fontIndex = 0);
	int GetBuffDrawDefineFont(UCHAR* dst, int sizeX, int sizeY, MIL_DOUBLE color, int fontIndex,CString OKFont);
	int GetBuffDrawString(void* milSrc, UCHAR* dst, int cx, int cy, int srcWidth, int srcHeight, int dstWidth, int dstHeight, double angle, MIL_DOUBLE color);

	int DrawString(UCHAR* srcImg, int srcSizeX, int srcSizeY, UCHAR* dstImg = NULL);
	//int GetBuffDrawString(UCHAR* dst, int sizeX, int sizeY, MIL_DOUBLE color);
	int GetBuffDrawBox(UCHAR* dst, int sizeX, int sizeY, MIL_DOUBLE color);

	
	CString GetFontCharValue(int fontIndex);
	void GetFontImgSize(int fontIndex,int* retOffsetX, int* retOffsetY, int* retWidth, int* retHeight);
	void GetStringImgSize(int* retOffsetX, int* retOffsetY, int* retWidth, int* retHeight);
	

	//CString CreatePath(CString path);


	
	//int GetThreshold(Im::PIL_ID milSrc);

	//int FontHighlight(Im::PIL_ID src, Im::PIL_ID dst, int foreGround, BOOL mode, int charCount, int threshold = -1);
	int FontHighlight(Im::PIL_ID src, Im::PIL_ID dst, int foreGround, BOOL mode, int charCount, int threshold, BOOL bAllchar);
	void MakeBlob(Im::PIL_ID src, Im::PIL_ID dst, int foreground, int fontCount);
	
	// LMJ 2013/11/06	 : 배경색으로 라인을 그려 붙은 글자 분리,,
	void SetDrawLine(void * milBuff, int lineCnt, int * linePos, int * lineWidth, int foreGround);


	void MakeImg(void* srcT_R, void* srcT_G, void* srcT_B, void* srcM_R, void* srcM_B, void* srcB_R, void* srcB_B);

	//shk 20140421
	void TopHatFiltering(UCHAR* srcImg,UCHAR* dstImg,int sizeX,int sizeY,int foreGround);
	void MakeBlob2(Im::PIL_ID src, Im::PIL_ID dst, int foreground, int fontCount,CMilBlobResult** blobResult);
	void StrAllocResult();

	int ModelList_Load(CString strPath);
	int ModelList_Check(CString strPath);
	int ModelLoad(CString strPath);
	int ModelList_Clear(int nLane);
	void ModelRemove(CString strPath);
	void SetFontText(CString sFontText);
	CString GetFontText();

	int GetCharPos(double* x, double* y,double* width, double* height,double* angle,int index,bool m_bReRotate = false);
	int GetCharSIMILARITY(double* score, int index);
	double GetFontWidth();
	void SetImgSize(int width, int height);
};

