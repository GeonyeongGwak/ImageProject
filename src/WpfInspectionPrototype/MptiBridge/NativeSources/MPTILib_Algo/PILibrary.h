#pragma once


#include "MilWrapper.h"
#include "Blob.h"
#include "opencv2/opencv.hpp"

//shkim IPP rotate
#include "ipps.h"
#include "ippi.h"

#pragma comment(lib, "ipps.lib")
#pragma comment(lib, "ippi.lib")


class CImgLib
{
public:
	CImgLib(void);
	virtual ~CImgLib(void);
	
	virtual void blobFree(int64 mil_id) {};
	virtual void blobReconstruct(int64 srce_image_id, int64 BlobResId,int64 dest_image_id, MIL_INT operation, MIL_INT mode) {};
	virtual void graColor(int64 GraphContextId,MIL_DOUBLE ForegroundColor){};
	virtual void blobDraw(int64 GraphContId, int64 ResultId, int64 DestImageId,int64 Operation, int64 Label, int64 ControlFlag) {};
	virtual void blobControl(int64 BlobResId, int64 ProcMode, double Value) {};
	virtual void blobCalculate(int64 BlobIdentImageId, int64 GreyImageId,int64 FeatureListId, int64 BlobResId) {};
	virtual void blobSelect(int64 BlobResId, int64 Operation, int64 Feature,int64 Condition, double CondLow, double CondHigh) {};
	virtual void blobFill(int64 BlobResId, int64 TargetImageId, int64 Mode,int64 Value) {};
	virtual int64 blobGetNumber(int64 BlobResId, MIL_INT *CountPtr) { return 0; };
	virtual int64 blobAllocFeatureList(int64 SystemId,MIL_INT *FeatureListPtr) { return 0; };
	virtual int64 blobAllocResult(int64 SystemId, MIL_INT *BlobResIdPtr) { return 0; };
	virtual void blobGetResult(int64 BlobResId,int64 Feature, float  *TargetArrayPtr){};
	virtual void blobGetResult(int64 BlobResId,int64 Feature, char  *TargetArrayPtr) {};
	virtual void blobGetResult(int64 BlobResId,int64 Feature, double  *TargetArrayPtr) {};
	virtual void blobGetResult(int64 BlobResId,int64 Feature, int64  *TargetArrayPtr) {};
	virtual void blobGetResult(int64 BlobResId,int64 Feature, MIL_INT32  *TargetArrayPtr) {};
	virtual int64 blobGetLabel(int64 BlobResId, int64 XPos, int64 YPos,MIL_INT *BlobLabelPtr) { return 0; };
	virtual void blobLabel(int64 BlobResId, int64 TargetImageId, int64 Mode) {};
	virtual void blobSelectFeature(int64 FeatureListId, int64 Feature) {};
	virtual void blobGet_BOX(int64 BlobResId, char  *minx, char  *miny, char  *maxx, char  *maxy) {};
	virtual void blobGet_BOX(int64 BlobResId, double  *minx, double  *miny, double  *maxx, double  *maxy) {};
	virtual void blobGet_BOX(int64 BlobResId, int64  *minx, int64  *miny, int64  *maxx, int64  *maxy) {};
	virtual void blobGet_BOX(int64 BlobResId, MIL_INT32  *minx, MIL_INT32  *miny, MIL_INT32  *maxx, MIL_INT32  *maxy) {};
	virtual void blobGet_Center(int64 BlobResId,double    *cx, double  *cy){};
	virtual void blobGet_Center(int64 BlobResId,int64  *cx, int64  *cy){};
	virtual void blobGet_Center(int64 BlobResId,MIL_INT32  *cx, MIL_INT32  *cy){};

	int _LibType;
};

class CPilImg:public CImgLib
{
public:
	CPilImg(void);
	virtual ~CPilImg(void);

	virtual void blobFree(int64 mil_id);
	virtual void blobReconstruct(int64 srce_image_id, int64 BlobResId,int64 dest_image_id, MIL_INT operation, MIL_INT mode);
	virtual void graColor(int64 GraphContextId,MIL_DOUBLE ForegroundColor);
	virtual void blobDraw(int64 GraphContId, int64 ResultId, int64 DestImageId,int64 Operation, int64 Label, int64 ControlFlag);
	virtual void blobControl(int64 BlobResId, int64 ProcMode, double Value);
	virtual void blobCalculate(int64 BlobIdentImageId, int64 GreyImageId,int64 FeatureListId, int64 BlobResId);
	virtual void blobSelect(int64 BlobResId, int64 Operation, int64 Feature,int64 Condition, double CondLow, double CondHigh);
	virtual void blobFill(int64 BlobResId, int64 TargetImageId, int64 Mode,int64 Value);
	virtual int64 blobGetNumber(int64 BlobResId, MIL_INT *CountPtr);
	virtual int64 blobAllocFeatureList(int64 SystemId,MIL_INT *FeatureListPtr);
	virtual int64 blobAllocResult(int64 SystemId, MIL_INT *BlobResIdPtr);
	virtual void blobGetResult(int64 BlobResId,int64 Feature, double  *TargetArrayPtr);
	virtual void blobGetResult(int64 BlobResId,int64 Feature, int64  *TargetArrayPtr);
	virtual void blobGetResult(int64 BlobResId,int64 Feature, MIL_INT32  *TargetArrayPtr);
	virtual int64 blobGetLabel(int64 BlobResId, int64 XPos, int64 YPos,MIL_INT *BlobLabelPtr);
	virtual void blobLabel(int64 BlobResId, int64 TargetImageId, int64 Mode);
	virtual void blobSelectFeature(int64 FeatureListId, int64 Feature);
	virtual void blobGet_BOX(int64 BlobResId, char  *minx, char  *miny, char  *maxx, char  *maxy);
	virtual void blobGet_BOX(int64 BlobResId, double  *minx, double  *miny, double  *maxx, double  *maxy);
	virtual void blobGet_BOX(int64 BlobResId, int64  *minx, int64  *miny, int64  *maxx, int64  *maxy);
	virtual void blobGet_BOX(int64 BlobResId, MIL_INT32  *minx, MIL_INT32  *miny, MIL_INT32  *maxx, MIL_INT32  *maxy);
	virtual void blobGet_Center(int64 BlobResId,double    *cx, double  *cy);
	virtual void blobGet_Center(int64 BlobResId,int64  *cx, int64  *cy);
	virtual void blobGet_Center(int64 BlobResId,MIL_INT32  *cx, MIL_INT32  *cy);


	template<typename T>
	void TBolbResult(int64 BlobResId,int64 Feature, T*  TargetArrayPtr);
	template<typename T>
	void TblobGet_BOX(int64 BlobResId,T*  minx, T  *miny, T  *maxx, T  *maxy);

private:
	BOOL reverse;
	jsl::Blob m_blob;

// 
// private:
// 	std::map<int64,cv::Mat> m_ImageList;
// 	std::map<int64,jsl::Blob> m_blobList;
};

class CMilImg:public CImgLib
 {
 public:
 	CMilImg(void);
 	virtual ~CMilImg(void);
 
	virtual void blobFree(int64 mil_id);
	virtual void blobReconstruct(int64 srce_image_id, int64 BlobResId,int64 dest_image_id, MIL_INT operation, MIL_INT mode);
	virtual void graColor(int64 GraphContextId,MIL_DOUBLE ForegroundColor);
	virtual void blobDraw(int64 GraphContId, int64 ResultId, int64 DestImageId,int64 Operation, int64 Label, int64 ControlFlag);
	virtual void blobControl(int64 BlobResId, int64 ProcMode, double Value);
	virtual void blobCalculate(int64 BlobIdentImageId, int64 GreyImageId,int64 FeatureListId, int64 BlobResId);
	virtual void blobSelect(int64 BlobResId, int64 Operation, int64 Feature,int64 Condition, double CondLow, double CondHigh);
	virtual void blobFill(int64 BlobResId, int64 TargetImageId, int64 Mode,int64 Value);
	virtual int64 blobGetNumber(int64 BlobResId, MIL_INT *CountPtr);
	virtual int64 blobAllocFeatureList(int64 SystemId,MIL_INT *FeatureListPtr);
	virtual int64 blobAllocResult(int64 SystemId, MIL_INT *BlobResIdPtr);
	virtual void blobGetResult(int64 BlobResId,int64 Feature, double  *TargetArrayPtr);
	virtual void blobGetResult(int64 BlobResId,int64 Feature, int64  *TargetArrayPtr);
	virtual void blobGetResult(int64 BlobResId,int64 Feature, MIL_INT32  *TargetArrayPtr);
 	virtual int64 blobGetLabel(int64 BlobResId, int64 XPos, int64 YPos,MIL_INT *BlobLabelPtr);
 	virtual void blobLabel(int64 BlobResId, int64 TargetImageId, int64 Mode);
	virtual void blobSelectFeature(int64 FeatureListId, int64 Feature);
	virtual void blobGet_BOX(int64 BlobResId, char  *minx, char  *miny, char  *maxx, char  *maxy);
	virtual void blobGet_BOX(int64 BlobResId, double  *minx, double  *miny, double  *maxx, double  *maxy);
	virtual void blobGet_BOX(int64 BlobResId, int64  *minx, int64  *miny, int64  *maxx, int64  *maxy);
	virtual void blobGet_BOX(int64 BlobResId, MIL_INT32  *minx, MIL_INT32  *miny, MIL_INT32  *maxx, MIL_INT32  *maxy);
	virtual void blobGet_Center(int64 BlobResId,double    *cx, double  *cy);
	virtual void blobGet_Center(int64 BlobResId,int64  *cx, int64  *cy);
	virtual void blobGet_Center(int64 BlobResId,MIL_INT32  *cx, MIL_INT32  *cy);

	template<typename T>
	void TBolbResult(int64 BlobResId,int64 Feature, T*  TargetArrayPtr);
 };


namespace Im
{
	typedef INT_PTR PIL_ID;
	typedef enum { PIL=0, MIL=1, BOTH=2 }eType;

	class iSys
	{
	public:
		virtual PIL_ID Alloc(MIL_CONST_TEXT_PTR SystemDescriptor, int64 SystemNum, int64 InitFlag, PIL_ID *SystemIdPtr) { *SystemIdPtr = 1; return 1; };
		virtual void Free(PIL_ID SystemId) {};
		virtual int64 Inquire(Im::PIL_ID SystemId, int64 InquireType, void *UserVarPtr) { return 0; };
	};
	class PILSys : public iSys
	{
	public:
		PIL_ID Alloc(MIL_CONST_TEXT_PTR SystemDescriptor, int64 SystemNum, int64 InitFlag, PIL_ID *SystemIdPtr) { *SystemIdPtr = 1; return 1; };
		void Free(PIL_ID SystemId) {};
		int64 Inquire(Im::PIL_ID SystemId, int64 InquireType, void *UserVarPtr) { return 0; };
	};
	class MILSys : public iSys
	{
	public:
		PIL_ID Alloc(MIL_CONST_TEXT_PTR SystemDescriptor, int64 SystemNum, int64 InitFlag, PIL_ID *SystemIdPtr);
		void Free(PIL_ID SystemId);
		int64 Inquire(Im::PIL_ID SystemId, int64 InquireType, void *UserVarPtr);
	};


	class iApp
	{
	public:
		virtual PIL_ID Alloc(int64 InitFlag, PIL_ID *ApplicationIdPtr) { *ApplicationIdPtr = 1; return 1; };
		virtual void Free(PIL_ID ApplicationId) {};
		virtual void Control(int64 ControlType, int64 ControlValue) {};
		virtual void Timer(int64 Mode, double *Time) {};
		virtual int64 Inquire(int64 InquireType, void *UserVarPtr) { return 0; };
	};
	class PILApp : public iApp
	{
	public:
		PIL_ID Alloc(int64 InitFlag, PIL_ID *ApplicationIdPtr) { *ApplicationIdPtr = 1; return 1; };
		void Free(PIL_ID ApplicationId) {};
		void Control(int64 ControlType, int64 ControlValue) {};
		void Timer(int64 Mode, double *Time) {};
		int64 Inquire(int64 InquireType, void *UserVarPtr) { return 0; };
	};
	class MILApp : public iApp
	{
	public:
		PIL_ID Alloc(int64 InitFlag, PIL_ID *ApplicationIdPtr);
		void Free(PIL_ID ApplicationId);
		void Control(int64 ControlType, int64 ControlValue);
		void Timer(int64 Mode, double *Time);
		int64 Inquire(int64 InquireType, void *UserVarPtr);
	};

	class iDig
	{
	public:
		virtual void Free(PIL_ID DevId) {};
	};
	class PILDig : public iDig
	{
	public:
		void Free(PIL_ID DevId) {};
	};
	class MILDig : public iDig
	{
	public:
		void Free(PIL_ID DevId);
	};

	class iBuf
	{
	public:
		virtual PIL_ID Alloc2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *IdVarPtr) { return 0; };
		virtual PIL_ID Create2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void *DataPtr, PIL_ID *IdVarPtr) { return 0; };
		virtual void Free(Im::PIL_ID BufId) {};
		virtual void Load(CString Filename, PIL_ID BufId) {};
		UCHAR* Load(CString strFilename);
		virtual PIL_ID Import(CString Filename, int64 FileFormat, int64 Operation, PIL_ID SystemId, PIL_ID *BufIdPtr) { return 0; };
		virtual void Export(CString FileName, int64 FileFormat, PIL_ID srcBufId) {};
		virtual void Save(CString FileName, PIL_ID BufId) {};
		virtual int64 Inquire(PIL_ID BufId, int64 InquireType, void *ResultPtr) { return 0; };
		virtual void Put2d(PIL_ID DestBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr) {};
		virtual void Get2d(PIL_ID SourceBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr) {};
		virtual PIL_ID CreateColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void **ArrayOfDataPtr, PIL_ID *BufIdPtr) { return 0; };
		virtual PIL_ID AllocColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *BufIdPtr) { return 0; };
		virtual void PutColor(PIL_ID DestBufId, int64 DataFormat, int64 Band, const void *UserArrayPtr) {};
		virtual void PutColor2d(PIL_ID DestBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, const void *UserArrayPtr) {};
		virtual void GetColor(PIL_ID SrcBufId, int64 DataFormat, int64 Band, void *UserArrayPtr) {};
		virtual void GetColor2d(PIL_ID SrcBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, void *UserArrayPtr) {};
		virtual void Copy(PIL_ID SrcBufId, PIL_ID DestBufId) {};
		virtual void CopyColor(PIL_ID SrcBufId, PIL_ID DestBufId, int64 Band) {};
		virtual void CopyClip(PIL_ID SrcBufId, PIL_ID DestBufId, int64 DestOffX, int64 DestOffY) {};
		virtual void Clear(PIL_ID DestImageBufId, double Color) {};
		virtual void Put(PIL_ID DestBufId, const void *UserArrayPtr) {};
		virtual void Get(PIL_ID SrcBufId, void *UserArrayPtr) {};
		virtual PIL_ID Child2d(PIL_ID ParentBufId, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, PIL_ID *BufIdPtr) { return 0; };
	};
	class PILBuf : public iBuf
	{
	public:
		 PIL_ID Alloc2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *IdVarPtr) override;
		 PIL_ID Create2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void *DataPtr, PIL_ID *IdVarPtr) override;
		 void Free(Im::PIL_ID BufId) override;
		 void Load(CString Filename, PIL_ID BufId) override;
		 UCHAR* Load(CString strFilename);
		 PIL_ID Import(CString Filename, int64 FileFormat, int64 Operation, PIL_ID SystemId, PIL_ID *BufIdPtr) override;
		 void Export(CString FileName, int64 FileFormat, PIL_ID srcBufId) override;
		 void Save(CString FileName, PIL_ID BufId) override;
		 int64 Inquire(PIL_ID BufId, int64 InquireType, void *ResultPtr) override;
		 void Put2d(PIL_ID DestBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr) override;
		 void Get2d(PIL_ID SourceBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr) override;
		 PIL_ID CreateColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void **ArrayOfDataPtr, PIL_ID *BufIdPtr) override;
		 PIL_ID AllocColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *BufIdPtr) override;
		 void PutColor(PIL_ID DestBufId, int64 DataFormat, int64 Band, const void *UserArrayPtr) override;
		 void PutColor2d(PIL_ID DestBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, const void *UserArrayPtr) override;
		 void GetColor(PIL_ID SrcBufId, int64 DataFormat, int64 Band, void *UserArrayPtr) override;
		 void GetColor2d(PIL_ID SrcBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, void *UserArrayPtr);
		 void Copy(PIL_ID SrcBufId, PIL_ID DestBufId) override;
		 void CopyColor(PIL_ID SrcBufId, PIL_ID DestBufId, int64 Band) override;
		 void CopyClip(PIL_ID SrcBufId, PIL_ID DestBufId, int64 DestOffX, int64 DestOffY) override;
		 void Clear(PIL_ID DestImageBufId, double Color) override;
		 void Put(PIL_ID DestBufId, const void *UserArrayPtr) override;
		 void Get(PIL_ID SrcBufId, void *UserArrayPtr) override;
		 PIL_ID Child2d(PIL_ID ParentBufId, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, PIL_ID *BufIdPtr) override;

		template<typename T>
		void Put(T* src, T* dst, int size_x, int size_y);

		void ImSave(CString sPath,cv::Mat *img);
	};

	class MILBuf : public iBuf
	{
	public:
		 PIL_ID Alloc2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *IdVarPtr) override;
		 PIL_ID Create2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void *DataPtr, PIL_ID *IdVarPtr) override;
		 void Free(Im::PIL_ID BufId) override;
		 void Load(CString Filename, PIL_ID BufId) override;
		 PIL_ID Import(CString Filename, int64 FileFormat, int64 Operation, PIL_ID SystemId, PIL_ID *BufIdPtr) override;
		 void Export(CString FileName, int64 FileFormat, PIL_ID srcBufId) override;
		 void Save(CString FileName, PIL_ID BufId) override;
		 int64 Inquire(PIL_ID BufId, int64 InquireType, void *ResultPtr) override;
		 void Put2d(PIL_ID DestBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr) override;
		 void Get2d(PIL_ID SourceBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr) override;
		 PIL_ID CreateColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void **ArrayOfDataPtr, PIL_ID *BufIdPtr) override;
		 PIL_ID AllocColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *BufIdPtr) override;
		 void PutColor(PIL_ID DestBufId, int64 DataFormat, int64 Band, const void *UserArrayPtr) override;
		 void PutColor2d(PIL_ID DestBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, const void *UserArrayPtr) override;
		 void GetColor(PIL_ID SrcBufId, int64 DataFormat, int64 Band, void *UserArrayPtr) override;
		 void GetColor2d(PIL_ID SrcBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, void *UserArrayPtr);
		 void Copy(PIL_ID SrcBufId, PIL_ID DestBufId) override;
		 void CopyColor(PIL_ID SrcBufId, PIL_ID DestBufId, int64 Band) override;
		 void CopyClip(PIL_ID SrcBufId, PIL_ID DestBufId, int64 DestOffX, int64 DestOffY) override;
		 void Clear(PIL_ID DestImageBufId, double Color) override;
		 void Put(PIL_ID DestBufId, const void *UserArrayPtr) override;
		 void Get(PIL_ID SrcBufId, void *UserArrayPtr) override;
		 PIL_ID Child2d(PIL_ID ParentBufId, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, PIL_ID *BufIdPtr) override;
	};

	class iIm
	{
	public:
		virtual void Free(PIL_ID ImResultId) {};
		virtual void Rotate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double Angle, double SrcCenX, double SrcCenY, double DstCenX, double DstCenY, int64 InterpolationMode) {};
		virtual PIL_ID Binarize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 ConditionAndThreshMode, double LowParam, double HighParam) { return 0; };
		virtual void Close(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode) {};
		virtual void Open(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode) {};
		virtual void Erode(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode) {};
		virtual void Dilate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode) {};
		virtual void Morphic(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Operation, int64 NbIterationOrArea, int64 ProcMode) {};
		virtual void Rank(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Rank, int64 ProcMode) {};
		virtual void Arith(double Src1ImageBufIdOrConst, double Src2ImageBufIdOrConst, PIL_ID DestImageBufId, int64 Operation) {};
		virtual void Resize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double ScaleFactorX, double ScaleFactorY, int64 InterpolationMode) {};
		virtual void Translate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double XDisplacement, double YDisplacement, int64 InterpolationMode) {};
		virtual void EdgeDetect(PIL_ID SrcImageBufId, PIL_ID DestIntensityImageBufId, PIL_ID DestAngleImageBufId, PIL_ID KernelId, int64 ControlFlag, int64 Threshold) {};
		virtual PIL_ID AllocResult(PIL_ID SystemId, int64 NbEntries, int64 ResultType, PIL_ID *ImResultIdPtr) { return 0; };
		virtual void Histogram(PIL_ID SrcImageBufId, PIL_ID HistImResultId) {};
		virtual void GetResult(PIL_ID ImResultId, int64 ResultType, void *UserArrayPtr) {};
		virtual void Project(PIL_ID SrcImageBufId, PIL_ID ProjImResultId, double ProjAngle) {};
	};

	class PILIm : public iIm
	{
	public:
		void Free(PIL_ID ImResultId);
		void Rotate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double Angle, double SrcCenX, double SrcCenY, double DstCenX, double DstCenY, int64 InterpolationMode);
		PIL_ID Binarize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 ConditionAndThreshMode, double LowParam, double HighParam);
		void Close(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode);
		void Open(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode);
		void Erode(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode);
		void Dilate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode);
		void Morphic(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Operation, int64 NbIterationOrArea, int64 ProcMode);
		void Rank(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Rank, int64 ProcMode);
		void Arith(double Src1ImageBufIdOrConst, double Src2ImageBufIdOrConst, PIL_ID DestImageBufId, int64 Operation);
		void Resize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double ScaleFactorX, double ScaleFactorY, int64 InterpolationMode);
		void Translate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double XDisplacement, double YDisplacement, int64 InterpolationMode);
		void EdgeDetect(PIL_ID SrcImageBufId, PIL_ID DestIntensityImageBufId, PIL_ID DestAngleImageBufId, PIL_ID KernelId, int64 ControlFlag, int64 Threshold);
		PIL_ID AllocResult(PIL_ID SystemId, int64 NbEntries, int64 ResultType, PIL_ID *ImResultIdPtr);	
		void Histogram(PIL_ID SrcImageBufId, PIL_ID HistImResultId);
		void GetResult(PIL_ID ImResultId, int64 ResultType, void *UserArrayPtr);
		void Project(PIL_ID SrcImageBufId, PIL_ID ProjImResultId, double ProjAngle);

		void RotateImg_ipp(cv::Mat userSrc, double angle, cv::Mat* userDst);
		void RotateImg_ipp_color(cv::Mat userSrc, double angle, cv::Mat* userDst);
		bool RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize);

	};

	class MILIm : public iIm
	{
	public:
		void Free(PIL_ID ImResultId);
		void Rotate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double Angle, double SrcCenX, double SrcCenY, double DstCenX, double DstCenY, int64 InterpolationMode);
		PIL_ID Binarize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 ConditionAndThreshMode, double LowParam, double HighParam);
		void Close(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode);
		void Open(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode);
		void Erode(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode);
		void Dilate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode);
		void Morphic(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Operation, int64 NbIterationOrArea, int64 ProcMode);
		void Rank(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Rank, int64 ProcMode);
		void Arith(double Src1ImageBufIdOrConst, double Src2ImageBufIdOrConst, PIL_ID DestImageBufId, int64 Operation);
		void Resize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double ScaleFactorX, double ScaleFactorY, int64 InterpolationMode);
		void Translate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double XDisplacement, double YDisplacement, int64 InterpolationMode);
		void EdgeDetect(PIL_ID SrcImageBufId, PIL_ID DestIntensityImageBufId, PIL_ID DestAngleImageBufId, PIL_ID KernelId, int64 ControlFlag, int64 Threshold);
		PIL_ID AllocResult(PIL_ID SystemId, int64 NbEntries, int64 ResultType, PIL_ID *ImResultIdPtr);	
		void Histogram(PIL_ID SrcImageBufId, PIL_ID HistImResultId);
		void GetResult(PIL_ID ImResultId, int64 ResultType, void *UserArrayPtr);
		void Project(PIL_ID SrcImageBufId, PIL_ID ProjImResultId, double ProjAngle);
	};

	class iGra
	{
	public:
		virtual void Rect(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd) {};
		virtual void Color(PIL_ID GraphContId, double ForegroundColor) {};
		virtual void Lines(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, int64 NumberOfLinesOrVertices, const int64 *XStartArray, const int64 *YStartArray, const int64 *XEndArray, const int64 *YEndArray, int64 ControlFlag) {};
		virtual void Line(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd) {};
		virtual void RectFill(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd) {};
		virtual void Text(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, CString StringPtr) {};
		virtual void Dot(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XPos, double YPos) {};
	};

	class PILGra : public iGra
	{
	public:
		cv::Scalar color;
		PILGra();
		~PILGra();
		void Rect(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd);
		void Color(PIL_ID GraphContId, double ForegroundColor);
		void Lines(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, int64 NumberOfLinesOrVertices, const int64 *XStartArray, const int64 *YStartArray, const int64 *XEndArray, const int64 *YEndArray, int64 ControlFlag);
		void Line(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd);
		void RectFill(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd);
		void Text(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, CString StringPtr);
		void Dot(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XPos, double YPos);
	};
	class MILGra : public iGra
	{
	public:
		void Rect(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd);
		void Color(PIL_ID GraphContId, double ForegroundColor);
		void Lines(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, int64 NumberOfLinesOrVertices, const int64 *XStartArray, const int64 *YStartArray, const int64 *XEndArray, const int64 *YEndArray, int64 ControlFlag);
		void Line(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd);
		void RectFill(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd);
		void Text(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, CString StringPtr);
		void Dot(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XPos, double YPos);
	};

	class iCal
	{
	public:
		virtual void Free(PIL_ID CalibrationId) {};
		virtual void List(PIL_ID CalibrationId, const double *XPixArray, const double *YPixArray, const double *XWorldArray, const double *YWorldArray, const double *ZWorldArray, int64 NumPoint, int64 Operation, int64 ControlFlag) {};
		virtual PIL_ID Alloc(PIL_ID SystemId, int64 Mode, int64 ModeFlag, PIL_ID *CalibrationIdPtr) { return 0; };
		virtual int64 Inquire(PIL_ID CalibrationOrMilId, int64 InquireType, void *UserVarPtr) { return 0; };
	};
	class PILCal : public iCal
	{
	public:
		void Free(PIL_ID CalibrationId);
		void List(PIL_ID CalibrationId, const double *XPixArray, const double *YPixArray, const double *XWorldArray, const double *YWorldArray, const double *ZWorldArray, int64 NumPoint, int64 Operation, int64 ControlFlag);
		PIL_ID Alloc(PIL_ID SystemId, int64 Mode, int64 ModeFlag, PIL_ID *CalibrationIdPtr);
		int64 Inquire(PIL_ID CalibrationOrMilId, int64 InquireType, void *UserVarPtr);
	};
	class MILCal : public iCal
	{
	public:
		void Free(PIL_ID CalibrationId);
		void List(PIL_ID CalibrationId, const double *XPixArray, const double *YPixArray, const double *XWorldArray, const double *YWorldArray, const double *ZWorldArray, int64 NumPoint, int64 Operation, int64 ControlFlag);
		PIL_ID Alloc(PIL_ID SystemId, int64 Mode, int64 ModeFlag, PIL_ID *CalibrationIdPtr);
		int64 Inquire(PIL_ID CalibrationOrMilId, int64 InquireType, void *UserVarPtr);
	};

	static int _LibType;
 	
 	extern iSys * _pSys;
 	extern iApp * _pApp;
 	extern iDig * _pDig;
 	extern iBuf * _pBuf;
 	extern iIm * _pIm;
 	extern iGra * _pGra;
	extern iCal * _pCal;

	static void Factory(int nType)
	{
		if (_pSys != nullptr)
			return;
		_LibType = nType;

		if(_LibType == PIL || _LibType == BOTH)
		{
			/*Im::_pSys = new PILSys();
			Im::_pApp = new PILApp();
			Im::_pDig = new PILDig();
			Im::_pBuf = new PILBuf();
			Im::_pIm = new PILIm();
			Im::_pGra = new PILGra();
			Im::_pCal = new PILCal();*/
			Im::_pSys = g_pMManager->pem_new<PILSys>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pApp = g_pMManager->pem_new<PILApp>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pDig = g_pMManager->pem_new<PILDig>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pBuf = g_pMManager->pem_new<PILBuf>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pIm = g_pMManager->pem_new<PILIm>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pGra = g_pMManager->pem_new<PILGra>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pCal = g_pMManager->pem_new<PILCal>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);

			if (_LibType == BOTH)
			{
				BOOL bRet = FALSE;
				bRet = LoadMilDll();
			}
		}
		else
		{
			BOOL bRet = FALSE;
			bRet = LoadMilDll();
			bRet = LoadMilblobDll();
			bRet = LoadMilimDll();
			bRet = LoadMilpatDll();
			bRet = LoadMilstrDll();
			bRet = LoadMilcalDll();

			/*Im::_pSys = new MILSys();
			Im::_pApp = new MILApp();
			Im::_pDig = new MILDig();
			Im::_pBuf = new MILBuf();
			Im::_pIm = new MILIm();
			Im::_pGra = new MILGra();
			Im::_pCal = new MILCal();*/
			Im::_pSys = g_pMManager->pem_new<MILSys>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pApp = g_pMManager->pem_new<MILApp>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pDig = g_pMManager->pem_new<MILDig>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pBuf = g_pMManager->pem_new<MILBuf>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pIm = g_pMManager->pem_new<MILIm>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pGra = g_pMManager->pem_new<MILGra>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			Im::_pCal = g_pMManager->pem_new<MILCal>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		}
	}

	static void Factory_Free()
	{
		if (_LibType == PIL || _LibType == BOTH)
		{
			//delete Im::_pSys;
			g_pMManager->pem_delete(Im::_pSys, false);
			Im::_pSys = nullptr;
			//delete Im::_pApp;
			g_pMManager->pem_delete(Im::_pApp, false);
			Im::_pApp = nullptr;
			//delete Im::_pDig;
			g_pMManager->pem_delete(Im::_pDig, false);
			Im::_pDig = nullptr;
			//delete Im::_pBuf;
			g_pMManager->pem_delete(Im::_pBuf, false);
			Im::_pBuf = nullptr;
			//delete Im::_pIm ;
			g_pMManager->pem_delete(Im::_pIm, false);
			Im::_pIm = nullptr;
			//delete Im::_pGra;
			g_pMManager->pem_delete(Im::_pGra, false);
			Im::_pGra = nullptr;
			//delete Im::_pCal;
			g_pMManager->pem_delete(Im::_pCal, false);
			Im::_pCal = nullptr;

			if (_LibType == BOTH)
			{	
				BOOL bRet = FALSE;
				bRet = FreeMilDll();
			}
		}
		else
		{
			BOOL bRet = FALSE;
			bRet = FreeMilDll();
			bRet = FreeMilblobDll();
			bRet = FreeMilimDll();
			bRet = FreeMilpatDll();
			bRet = FreeMilstrDll();
			bRet = FreeMilcalDll();

			//delete Im::_pSys;
			g_pMManager->pem_delete(Im::_pSys, false);
			Im::_pSys = nullptr;
			//delete Im::_pApp;
			g_pMManager->pem_delete(Im::_pApp, false);
			Im::_pApp = nullptr;
			//delete Im::_pDig;
			g_pMManager->pem_delete(Im::_pDig, false);
			Im::_pDig = nullptr;
			//delete Im::_pBuf;
			g_pMManager->pem_delete(Im::_pBuf, false);
			Im::_pBuf = nullptr;
			//delete Im::_pIm ;
			g_pMManager->pem_delete(Im::_pIm, false);
			Im::_pIm = nullptr;
			//delete Im::_pGra;
			g_pMManager->pem_delete(Im::_pGra, false);
			Im::_pGra = nullptr;
			//delete Im::_pCal;
			g_pMManager->pem_delete(Im::_pCal, false);
			Im::_pCal = nullptr;
		}
	}


	class Sys
	{
	public:
		static PIL_ID Alloc(MIL_CONST_TEXT_PTR SystemDescriptor, int64 SystemNum, int64 InitFlag, PIL_ID *SystemIdPtr) { return _pSys->Alloc(SystemDescriptor, SystemNum, InitFlag, SystemIdPtr); };
		static void Free(PIL_ID SystemId) { _pSys->Free(SystemId); };
		static int64 Inquire(Im::PIL_ID SystemId, int64 InquireType, void *UserVarPtr) { return _pSys->Inquire(SystemId, InquireType, UserVarPtr); };
	};
	class App
	{
	public:
		static PIL_ID Alloc(int64 InitFlag, PIL_ID *ApplicationIdPtr) { return _pApp->Alloc(InitFlag, ApplicationIdPtr); };
		static void Free(PIL_ID ApplicationId) { _pApp->Free(ApplicationId); };
		static void Control(int64 ControlType, int64 ControlValue) { _pApp->Control(ControlType, ControlValue); };
		static void Timer(int64 Mode, double *Time) { _pApp->Timer(Mode, Time); };
		static int64 Inquire(int64 InquireType, void *UserVarPtr) { return _pApp->Inquire(InquireType, UserVarPtr); };
	};
	class Dig
	{
	public:
		static void Free(PIL_ID DevId) { _pDig->Free(DevId); };
	};
	class Buf
	{
	public:
		static PIL_ID Alloc2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *IdVarPtr) {
			return _pBuf->Alloc2d(SystemId, SizeX, SizeY, Type, Attribute, IdVarPtr);
		};
		static PIL_ID Create2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void *DataPtr, PIL_ID *IdVarPtr)
		{ return _pBuf->Create2d(SystemId, SizeX, SizeY, Type, Attribute, ControlFlag, Pitch, DataPtr, IdVarPtr); };
		static void Free(PIL_ID BufId) { _pBuf->Free(BufId); };
		static void Load(CString Filename, PIL_ID BufId) { _pBuf->Load(Filename, BufId); };
		static UCHAR* Load(CString Filename) { return _pBuf->Load(Filename); };
		static PIL_ID Import(CString Filename, int64 FileFormat, int64 Operation, PIL_ID SystemId, PIL_ID *BufIdPtr) {
			return _pBuf->Import(Filename, FileFormat, Operation, SystemId, BufIdPtr); 
		};
		static void Export(CString FileName, int64 FileFormat, PIL_ID srcBufId) { _pBuf->Export(FileName, FileFormat, srcBufId); };
		static void Save(CString FileName, PIL_ID BufId) { _pBuf->Save(FileName, BufId); };
		static int64 Inquire(PIL_ID BufId, int64 InquireType, void *ResultPtr) { return _pBuf->Inquire(BufId, InquireType, ResultPtr); };
		static void Put2d(PIL_ID DestBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr) { _pBuf->Put2d(DestBufId, Offx, Offy, Sizex, Sizey, BufferPtr); };
		static void Get2d(PIL_ID SourceBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr) { _pBuf->Get2d(SourceBufId, Offx, Offy, Sizex, Sizey, BufferPtr); };
		static PIL_ID CreateColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void **ArrayOfDataPtr, PIL_ID *BufIdPtr)
		{
			return _pBuf->CreateColor(SystemId, SizeBand, SizeX, SizeY, Type, Attribute, ControlFlag, Pitch, ArrayOfDataPtr, BufIdPtr);
		};
		static PIL_ID AllocColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *BufIdPtr)
		{
			return _pBuf->AllocColor(SystemId, SizeBand, SizeX, SizeY, Type, Attribute, BufIdPtr);
		}
		static void PutColor(PIL_ID DestBufId, int64 DataFormat, int64 Band, const void *UserArrayPtr) { _pBuf->PutColor(DestBufId, DataFormat, Band, UserArrayPtr); };
		static void PutColor2d(PIL_ID DestBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, const void *UserArrayPtr)
		{
			_pBuf->PutColor2d(DestBufId, DataFormat, Band, OffX, OffY, SizeX, SizeY, UserArrayPtr);
		};
		static void GetColor(PIL_ID SrcBufId, int64 DataFormat, int64 Band, void *UserArrayPtr) { _pBuf->GetColor(SrcBufId, DataFormat, Band, UserArrayPtr); };
		static void GetColor2d(PIL_ID SrcBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, void *UserArrayPtr)
		{
			_pBuf->GetColor2d(SrcBufId, DataFormat, Band, OffX, OffY, SizeX, SizeY, UserArrayPtr);
		};
		static void Copy(PIL_ID SrcBufId, PIL_ID DestBufId) { _pBuf->Copy(SrcBufId, DestBufId); };
		static void CopyColor(PIL_ID SrcBufId, PIL_ID DestBufId, int64 Band) { _pBuf->CopyColor(SrcBufId, DestBufId, Band); };
		static void CopyClip(PIL_ID SrcBufId, PIL_ID DestBufId, int64 DestOffX, int64 DestOffY) { _pBuf->CopyClip(SrcBufId, DestBufId, DestOffX, DestOffY); };
		static void Clear(PIL_ID DestImageBufId, double Color) {
			_pBuf->Clear(DestImageBufId, Color);
		};
		static void Put(PIL_ID DestBufId, const void *UserArrayPtr) {
			_pBuf->Put(DestBufId, UserArrayPtr);
		};
		static void Get(PIL_ID SrcBufId, void *UserArrayPtr) {
			_pBuf->Get(SrcBufId, UserArrayPtr);
		};
		static PIL_ID Child2d(PIL_ID ParentBufId, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, PIL_ID *BufIdPtr)
		{ return _pBuf->Child2d(ParentBufId, OffX, OffY, SizeX, SizeY, BufIdPtr); };
	};
	class im
	{
	public:
		static void Free(PIL_ID ImResultId) { _pIm->Free(ImResultId); };
		static void Rotate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double Angle, double SrcCenX, double SrcCenY, double DstCenX, double DstCenY, int64 InterpolationMode)
		{
			_pIm->Rotate(SrcImageBufId, DestImageBufId, Angle, SrcCenX, SrcCenY, DstCenX, DstCenY, InterpolationMode);
		};
		static PIL_ID Binarize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 ConditionAndThreshMode, double LowParam, double HighParam)
		{
			return _pIm->Binarize(SrcImageBufId, DestImageBufId, ConditionAndThreshMode, LowParam, HighParam);
		};
		static void Close(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
		{
			_pIm->Close(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
		};
		static void Open(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
		{
			_pIm->Open(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
		};
		static void Erode(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
		{
			_pIm->Erode(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
		};
		static void Dilate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
		{
			_pIm->Dilate(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
		};
		static void Morphic(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Operation, int64 NbIterationOrArea, int64 ProcMode)
		{
			_pIm->Morphic(SrcImageBufId, DestImageBufId, StructElemBufId, Operation, NbIterationOrArea, ProcMode);
		};
		static void Rank(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Rank, int64 ProcMode)
		{
			_pIm->Rank(SrcImageBufId, DestImageBufId, StructElemBufId, Rank, ProcMode);
		};
		static void Arith(double Src1ImageBufIdOrConst, double Src2ImageBufIdOrConst, PIL_ID DestImageBufId, int64 Operation)
		{
			_pIm->Arith(Src1ImageBufIdOrConst, Src2ImageBufIdOrConst, DestImageBufId, Operation);
		};
		static void Resize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double ScaleFactorX, double ScaleFactorY, int64 InterpolationMode)
		{
			_pIm->Resize(SrcImageBufId, DestImageBufId, ScaleFactorX, ScaleFactorY, InterpolationMode);
		};
		static void Translate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double XDisplacement, double YDisplacement, int64 InterpolationMode)
		{
			_pIm->Translate(SrcImageBufId, DestImageBufId, XDisplacement, YDisplacement, InterpolationMode);
		};
		static void EdgeDetect(PIL_ID SrcImageBufId, PIL_ID DestIntensityImageBufId, PIL_ID DestAngleImageBufId, PIL_ID KernelId, int64 ControlFlag, int64 Threshold)
		{
			_pIm->EdgeDetect(SrcImageBufId, DestIntensityImageBufId, DestAngleImageBufId, KernelId, ControlFlag, Threshold);
		};
		static PIL_ID AllocResult(PIL_ID SystemId, int64 NbEntries, int64 ResultType, PIL_ID *ImResultIdPtr)
		{
			return _pIm->AllocResult(SystemId, NbEntries, ResultType, ImResultIdPtr);
		};
		static void Histogram(PIL_ID SrcImageBufId, PIL_ID HistImResultId)
		{
			_pIm->Histogram(SrcImageBufId, HistImResultId);
		};
		static void GetResult(PIL_ID ImResultId, int64 ResultType, void *UserArrayPtr)
		{
			_pIm->GetResult(ImResultId, ResultType, UserArrayPtr);
		};
		static void Project(PIL_ID SrcImageBufId, PIL_ID ProjImResultId, double ProjAngle)
		{
			_pIm->Project(SrcImageBufId, ProjImResultId, ProjAngle);
		};
	};
	class Gra
	{
	public:
		static void Rect(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
		{
			_pGra->Rect(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
		};
		static void Color(PIL_ID GraphContId, double ForegroundColor)
		{
			_pGra->Color(GraphContId, ForegroundColor);
		};
		static void Lines(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, int64 NumberOfLinesOrVertices, const int64 *XStartArray, const int64 *YStartArray, const int64 *XEndArray, const int64 *YEndArray, int64 ControlFlag)
		{
			_pGra->Lines(GraphContId, DestImageBufIdOrGraListId, NumberOfLinesOrVertices, XStartArray, YStartArray, XEndArray, YEndArray, ControlFlag);
		};
		static void Line(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
		{
			_pGra->Line(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
		};
		static void RectFill(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
		{
			_pGra->RectFill(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
		};
		static void Text(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, CString StringPtr)
		{
			_pGra->Text(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, StringPtr);
		};
		static void Dot(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XPos, double YPos)
		{
			_pGra->Dot(GraphContId, DestImageBufIdOrGraListId, XPos, YPos);
		}
	};
	class Cal
	{
	public:
		static void Free(PIL_ID CalibrationId)
		{
			_pCal->Free(CalibrationId);
		};
		static void List(PIL_ID CalibrationId, const double *XPixArray, const double *YPixArray, const double *XWorldArray, const double *YWorldArray, const double *ZWorldArray, int64 NumPoint, int64 Operation, int64 ControlFlag)
		{
			_pCal->List(CalibrationId, XPixArray, YPixArray, XWorldArray, YWorldArray, ZWorldArray, NumPoint, Operation, ControlFlag);
		};
		static PIL_ID Alloc(PIL_ID SystemId, int64 Mode, int64 ModeFlag, PIL_ID *CalibrationIdPtr)
		{
			return _pCal->Alloc(SystemId, Mode, ModeFlag, CalibrationIdPtr);
		};
		static int64 Inquire(PIL_ID CalibrationOrMilId, int64 InquireType, void *UserVarPtr)
		{
			return _pCal->Inquire(CalibrationOrMilId, InquireType, UserVarPtr);
		};
	};

	class Blob
	{
	public:
		static CImgLib* Imgptr;
		static void Alloc(eType ty)
		{
			if(ty == eType::PIL)
			{
				if(Imgptr)
				{
					if(_LibType == eType::PIL)
						return;
					else
						//delete Imgptr;
						g_pMManager->pem_delete(Imgptr, false);
				}
				//Imgptr = new CPilImg();
				Imgptr = g_pMManager->pem_new<CPilImg>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			}
			else if(ty == eType::MIL)
			{
				if(Imgptr)
				{
					if(_LibType == eType::MIL)
						return;
					else
					//delete Imgptr;
						g_pMManager->pem_delete(Imgptr, false);
				}
				//Imgptr = new CMilImg();
				Imgptr = g_pMManager->pem_new<CMilImg>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
			}
		};
		static void Free(void)
		{
			if(Imgptr != NULL)
			{
				//delete Imgptr;
				g_pMManager->pem_delete(Imgptr, false);
				Imgptr = NULL;
			}
		}


		static void blobFree(int64 mil_id){Imgptr->blobFree(mil_id);};
		static void blobReconstruct(int64 srce_image_id, int64 BlobResId,int64 dest_image_id, MIL_INT operation, MIL_INT mode){Imgptr->blobReconstruct( srce_image_id, BlobResId, dest_image_id,  operation,  mode);}
		static void graColor(int64 GraphContextId,MIL_DOUBLE ForegroundColor){Imgptr->graColor(GraphContextId, ForegroundColor);}
		static void blobDraw(int64 GraphContId, int64 ResultId, int64 DestImageId,int64 Operation, int64 Label, int64 ControlFlag){Imgptr->blobDraw( GraphContId,  ResultId,  DestImageId, Operation,  Label,  ControlFlag);}
		static void blobControl(int64 BlobResId, int64 ProcMode, double Value){Imgptr->blobControl(BlobResId, ProcMode, Value);}
		static void blobCalculate(int64 BlobIdentImageId, int64 GreyImageId,int64 FeatureListId, int64 BlobResId){Imgptr->blobCalculate(BlobIdentImageId, GreyImageId, FeatureListId, BlobResId);}
		static void blobSelect(int64 BlobResId, int64 Operation, int64 Feature,int64 Condition, double CondLow, double CondHigh){Imgptr->blobSelect(BlobResId, Operation,  Feature, Condition,  CondLow,  CondHigh);}
		static void blobFill(int64 BlobResId, int64 TargetImageId, int64 Mode,int64 Value){Imgptr->blobFill( BlobResId,  TargetImageId,  Mode, Value);}
		static int64 blobGetNumber(int64 BlobResId, MIL_INT *CountPtr){return Imgptr->blobGetNumber(BlobResId, CountPtr);}
		static int64 blobAllocFeatureList(int64 SystemId,MIL_INT *FeatureListPtr){return Imgptr->blobAllocFeatureList(SystemId,FeatureListPtr);}
		static int64 blobAllocResult(int64 SystemId, MIL_INT *BlobResIdPtr){return Imgptr->blobAllocResult(SystemId, BlobResIdPtr);}
		static void blobGetResult(int64 BlobResId,int64 Feature, float  *TargetArrayPtr){Imgptr->blobGetResult(BlobResId,Feature, TargetArrayPtr);}
		static void blobGetResult(int64 BlobResId,int64 Feature, double  *TargetArrayPtr){Imgptr->blobGetResult(BlobResId,Feature, TargetArrayPtr);}
		static void blobGetResult(int64 BlobResId,int64 Feature, int64  *TargetArrayPtr){Imgptr->blobGetResult(BlobResId, Feature, TargetArrayPtr);}
		static void blobGetResult(int64 BlobResId,int64 Feature, MIL_INT32  *TargetArrayPtr){Imgptr->blobGetResult( BlobResId, Feature, TargetArrayPtr);}
		static int64 blobGetLabel(int64 BlobResId, int64 XPos, int64 YPos,MIL_INT *BlobLabelPtr){return Imgptr->blobGetLabel(BlobResId, XPos, YPos,BlobLabelPtr);}
		static void blobLabel(int64 BlobResId, int64 TargetImageId, int64 Mode){Imgptr->blobLabel(BlobResId, TargetImageId, Mode);}
		static void blobSelectFeature(int64 FeatureListId, int64 Feature){Imgptr->blobSelectFeature(FeatureListId, Feature);}
		static void blobGet_BOX(int64 BlobResId, char  *minx, char  *miny, char  *maxx, char  *maxy){Imgptr->blobGet_BOX( BlobResId, minx, miny, maxx, maxy);}
		static void blobGet_BOX(int64 BlobResId, double  *minx, double  *miny, double  *maxx, double  *maxy){Imgptr->blobGet_BOX( BlobResId, minx, miny, maxx, maxy);}
		static void blobGet_BOX(int64 BlobResId, int64  *minx, int64  *miny, int64  *maxx, int64  *maxy){Imgptr->blobGet_BOX( BlobResId, minx, miny, maxx, maxy);}
		static void blobGet_BOX(int64 BlobResId, MIL_INT32  *minx, MIL_INT32  *miny, MIL_INT32  *maxx, MIL_INT32  *maxy){Imgptr->blobGet_BOX( BlobResId, minx, miny, maxx, maxy);}
		static void blobGet_Center(int64 BlobResId,double    *cx, double  *cy){Imgptr->blobGet_Center(BlobResId,cx, cy);}
		static void blobGet_Center(int64 BlobResId,int64  *cx, int64  *cy){Imgptr->blobGet_Center(BlobResId,cx, cy);}
		static void blobGet_Center(int64 BlobResId,MIL_INT32  *cx, MIL_INT32  *cy){Imgptr->blobGet_Center(BlobResId,cx, cy);}
	};
}