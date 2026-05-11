#include "StdAfx.h"
#include "PILibrary.h"
#include "Binarize.h"
#include "ippi.h"
using namespace Im;

CImgLib* Blob::Imgptr = NULL;

CImgLib::CImgLib(void)
{
}
CImgLib::~CImgLib(void)
{
}

////////////////////////////////////////////////////////////////////PIL//////////////////////////////////////////////////////////////////////////////////////////////
iSys * Im::_pSys = nullptr;
iApp * Im::_pApp = nullptr;
iDig * Im::_pDig = nullptr;
iBuf * Im::_pBuf = nullptr;
iIm * Im::_pIm = nullptr;
iGra * Im::_pGra = nullptr;
iCal * Im::_pCal = nullptr;


CPilImg::CPilImg(void)
{
	_LibType = Im::eType::PIL;
	m_blob.Alloc(1024, 1024);
}
CPilImg::~CPilImg(void)
{
	m_blob.Free();
}
//blob
void CPilImg::blobFree(int64 mil_id)
{
	if(mil_id != 0)
	{
		jsl::Blob * pb = (jsl::Blob *)mil_id;

		delete pb;
		mil_id = 0;
	}
//	MblobFree(mil_id);
}
void CPilImg::blobReconstruct(int64 srce_image_id, int64 BlobResId,int64 dest_image_id, MIL_INT operation, MIL_INT mode)
{
	int src_sz_x = Im::Buf::Inquire(srce_image_id, M_SIZE_X, M_NULL);
	int src_sz_y = Im::Buf::Inquire(srce_image_id, M_SIZE_Y, M_NULL);
	int srcPitch = Im::Buf::Inquire(srce_image_id, M_PITCH, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Im::Buf::Inquire(srce_image_id, M_HOST_ADDRESS, M_NULL);
	switch(operation)				//shkim operation ÀÌ 
	{
	case M_FILL_HOLES:
		if (BlobResId == NULL)
		{
		m_blob.Calculate(srcPtr, src_sz_x, src_sz_y, srcPitch);
		m_blob.FillHoles(srcPtr, src_sz_x, src_sz_y, srcPitch);
		}
		else
		{
			((jsl::Blob *)BlobResId)->Calculate(srcPtr, src_sz_x, src_sz_y, srcPitch);
			((jsl::Blob *)BlobResId)->FillHoles(srcPtr, src_sz_x, src_sz_y, srcPitch);
		}
		break;
	case M_ERASE_BORDER_BLOBS:
		//MblobReconstruct(srce_image_id, M_NULL, dest_image_id, M_ERASE_BORDER_BLOBS, M_BINARY + M_8_CONNECTED);
		break;
	case M_RECONSTRUCT_FROM_SEED:
		//MblobReconstruct(srce_image_id, seed_image_id, dest_image_id, M_RECONSTRUCT_FROM_SEED, M_8_CONNECTED);
		break;
	}
}
void CPilImg::graColor(int64 GraphContextId,MIL_DOUBLE ForegroundColor)
{
	
	if(M_COLOR_BLACK == ForegroundColor)
	{
		reverse = TRUE;
	}
	else
	{
		reverse = FALSE;
	}
}
void CPilImg::blobDraw(int64 GraphContId, int64 ResultId, int64 DestImageId,int64 Operation, int64 Label, int64 ControlFlag)
{
	if(ResultId == 0)
		return;
	if(!reverse)// blob: white , background: black
	{
		int src_sz_x = Im::Buf::Inquire(DestImageId, M_SIZE_X, M_NULL);
		int src_sz_y = Im::Buf::Inquire(DestImageId, M_SIZE_Y, M_NULL);
		int srcPitch = Im::Buf::Inquire(DestImageId, M_PITCH, M_NULL);
		UCHAR * srcPtr = (UCHAR *)Im::Buf::Inquire(DestImageId, M_HOST_ADDRESS, M_NULL);

		((jsl::Blob *)ResultId)->DrawLabels(srcPtr, src_sz_x, src_sz_y, srcPitch, Label, M_COLOR_WHITE);

	}
	else// blob: black , background: white
	{	
		MbufClear(DestImageId, M_COLOR_WHITE);
		int src_sz_x = Im::Buf::Inquire(DestImageId, M_SIZE_X, M_NULL);
		int src_sz_y = Im::Buf::Inquire(DestImageId, M_SIZE_Y, M_NULL);
		int srcPitch = Im::Buf::Inquire(DestImageId, M_PITCH, M_NULL);
		UCHAR * srcPtr = (UCHAR *)Im::Buf::Inquire(DestImageId, M_HOST_ADDRESS, M_NULL);

		((jsl::Blob *)ResultId)->DrawLabels(srcPtr, src_sz_x, src_sz_y, srcPitch, Label, M_COLOR_BLACK);
	}
}
void CPilImg::blobControl(int64 BlobResId, int64 ProcMode, double Value)
{
//	MblobControl(BlobResId, ProcMode, Value);
}
void CPilImg::blobCalculate(int64 BlobIdentImageId, int64 GreyImageId,int64 FeatureListId, int64 BlobResId)
{
	if(BlobResId == 0)
		return;
	int Dst_sz_x = Im::Buf::Inquire(BlobIdentImageId, M_SIZE_X, M_NULL);
	int Dst_sz_y = Im::Buf::Inquire(BlobIdentImageId, M_SIZE_Y, M_NULL);
	int DstPitch = Im::Buf::Inquire(BlobIdentImageId, M_PITCH, M_NULL);
	UCHAR * DstPtr = (UCHAR *)Im::Buf::Inquire(BlobIdentImageId, M_HOST_ADDRESS, M_NULL);
	((jsl::Blob *)BlobResId)->Calculate(DstPtr, Dst_sz_x, Dst_sz_y, DstPitch);

}
void CPilImg::blobSelect(int64 BlobResId, int64 Operation, int64 Feature,int64 Condition, double CondLow, double CondHigh)
{
	if(BlobResId == 0)
		return;
	if(Feature == M_LABEL_VALUE )
	{
//	case M_NOT_EQUAL:
		((jsl::Blob *)BlobResId)->RemoveLabelinv(CondLow);
	}
	else if(Feature == M_AREA)
	{
		switch(Condition)
		{
		case M_LESS:
			((jsl::Blob *)BlobResId)->RemoveMinArea(CondLow);
			break;
		case M_GREATER:
			((jsl::Blob *)BlobResId)->RemoveMaxArea(CondLow);
			break;
		case M_NOT_EQUAL:
			((jsl::Blob *)BlobResId)->RemoveMinArea(CondLow);
			((jsl::Blob *)BlobResId)->RemoveMaxArea(CondHigh);
			break;
		}
	}
	
	
//	MblobSelect(BlobResId, Operation, Feature,Condition, CondLow, CondHigh);
}
void CPilImg::blobFill(int64 BlobResId, int64 TargetImageId, int64 Mode,int64 Value)
{
	if(BlobResId == 0)
		return;
	int src_sz_x = Im::Buf::Inquire(TargetImageId, M_SIZE_X, M_NULL);
	int src_sz_y = Im::Buf::Inquire(TargetImageId, M_SIZE_Y, M_NULL);
	int srcPitch = Im::Buf::Inquire(TargetImageId, M_PITCH, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Im::Buf::Inquire(TargetImageId, M_HOST_ADDRESS, M_NULL);
	switch(Mode)
	{
	case M_EXCLUDED_BLOBS:
		((jsl::Blob *)BlobResId)->DrawRemoves(srcPtr, src_sz_x, src_sz_y, srcPitch);	//redraw excluded blob (value : 0) -> erase
		break;
	case M_INCLUDED_BLOBS:
		((jsl::Blob *)BlobResId)->Draws(srcPtr, src_sz_x, src_sz_y, srcPitch);		//redraw included blob (value : 255) -> draw
		break;
	}
}
int64 CPilImg::blobGetNumber(int64 BlobResId, MIL_INT *CountPtr)
{
	if(BlobResId == 0)
		return 0;
	if(CountPtr == NULL)
	{
		return (int64)((jsl::Blob *)BlobResId)->m_nBlobRealCount;
	}
	*CountPtr = (int64)((jsl::Blob *)BlobResId)->m_nBlobRealCount;
	return *CountPtr;
}
int64 CPilImg::blobAllocFeatureList(int64 SystemId,MIL_INT *FeatureListPtr)
{
	*FeatureListPtr = M_NULL;
	return *FeatureListPtr;//MblobAllocFeatureList(SystemId, FeatureListPtr);
}
int64 CPilImg::blobAllocResult(int64 SystemId, MIL_INT *BlobResIdPtr)
{
	//jsl::Blob * pb = new jsl::Blob();
	jsl::Blob * pb = g_pMManager->pem_new<jsl::Blob>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	pb->Alloc(1024, 1024);
	if(BlobResIdPtr)
	{
		jsl::Blob * ptrb = (jsl::Blob *)(*BlobResIdPtr);

		//delete ptrb;
		g_pMManager->pem_delete(ptrb, false);
		*BlobResIdPtr = (int64)pb;
	}
	return (int64)pb;//MblobAllocResult(SystemId, BlobResIdPtr);
}
void CPilImg::blobGetResult(int64 BlobResId,int64 Feature, double  *TargetArrayPtr)
{
	TBolbResult(BlobResId,Feature, TargetArrayPtr);
}
void CPilImg::blobGetResult(int64 BlobResId,int64 Feature, int64  *TargetArrayPtr)
{

	TBolbResult(BlobResId,Feature, TargetArrayPtr);
}
void CPilImg::blobGetResult(int64 BlobResId,int64 Feature, MIL_INT32  *TargetArrayPtr)
{

	TBolbResult(BlobResId,Feature, TargetArrayPtr);
}
template<typename T>
void CPilImg::TBolbResult(int64 BlobResId,int64 Feature, T*  TargetArrayPtr)
{
	if(BlobResId == 0)
		return;
	int64 Operater = 0xffffL;
	Operater= Operater&Feature;

	T *ptrf;

	switch(Operater)
	{
	case M_AREA:
		((jsl::Blob *)BlobResId)->GetArea(TargetArrayPtr);
		break;
	case M_CENTER_OF_GRAVITY_X:
		//ptrf = new T[((jsl::Blob *)BlobResId)->m_nBlobCount];
		ptrf = g_pMManager->pem_new<T>(true, ((jsl::Blob *)BlobResId)->m_nBlobCount, (PCHAR)__FUNCTION__, __LINE__);
		((jsl::Blob *)BlobResId)->GetCenter(TargetArrayPtr, ptrf);
		//delete [] ptrf;
		g_pMManager->pem_delete(ptrf, true);
		break;
	case M_CENTER_OF_GRAVITY_Y:
		//ptrf = new T[((jsl::Blob *)BlobResId)->m_nBlobCount];
		ptrf = g_pMManager->pem_new<T>(true, ((jsl::Blob *)BlobResId)->m_nBlobCount, (PCHAR)__FUNCTION__, __LINE__);
		((jsl::Blob *)BlobResId)->GetCenter(ptrf, TargetArrayPtr);
		//delete [] ptrf;
		g_pMManager->pem_delete(ptrf, true);
		break;
	case M_LABEL_VALUE:
		((jsl::Blob *)BlobResId)->GetLabel(TargetArrayPtr);
		break;
	case M_FERET_MIN_DIAMETER:
	case M_FERET_MIN_ANGLE:
		//MblobGetResult(BlobResId,Feature, TargetArrayPtr);
		*TargetArrayPtr = (T)0;
		break;
	case M_BOX_X_MIN:
		((jsl::Blob *)BlobResId)->GetMinX(TargetArrayPtr);
		break;
	case M_BOX_X_MAX:
		((jsl::Blob *)BlobResId)->GetMaxX(TargetArrayPtr);
		break;
	case M_BOX_Y_MIN:
		((jsl::Blob *)BlobResId)->GetMinY(TargetArrayPtr);
		break;
	case M_BOX_Y_MAX:
		((jsl::Blob *)BlobResId)->GetMaxY(TargetArrayPtr);
		break;
	}
}
int64 CPilImg::blobGetLabel(int64 BlobResId, int64 XPos, int64 YPos,MIL_INT *BlobLabelPtr)
{
	if(BlobResId == 0)
		return 0;
	long label(0);
	if(BlobLabelPtr == NULL){

		return ((jsl::Blob *)BlobResId)->GetLabel(XPos,YPos,&label);
	}

	((jsl::Blob *)BlobResId)->GetLabel(XPos, YPos, &label);
	*BlobLabelPtr = (MIL_INT)label;
	return *BlobLabelPtr;
}
void CPilImg::blobLabel(int64 BlobResId, int64 TargetImageId, int64 Mode)
{
	if (BlobResId == 0 || TargetImageId == 0)
		return;
	int src_sz_x = Im::Buf::Inquire(TargetImageId, M_SIZE_X, M_NULL);
	int src_sz_y = Im::Buf::Inquire(TargetImageId, M_SIZE_Y, M_NULL);
	int srcPitch = Im::Buf::Inquire(TargetImageId, M_PITCH, M_NULL);
	USHORT * srcPtr = (USHORT *)Im::Buf::Inquire(TargetImageId, M_HOST_ADDRESS, M_NULL);
	((jsl::Blob *)BlobResId)->DrawLabels(srcPtr, src_sz_x, src_sz_y, srcPitch);

//	MblobLabel(BlobResId, TargetImageId, Mode);
}
void CPilImg::blobSelectFeature(int64 FeatureListId, int64 Feature)
{
	//MblobSelectFeature(FeatureListId, Feature);
}

void CPilImg::blobGet_BOX(int64 BlobResId, char  *minx, char  *miny, char  *maxx, char  *maxy)
{
	TblobGet_BOX(BlobResId,minx,miny,maxx,maxy);
}
void CPilImg::blobGet_BOX(int64 BlobResId, double  *minx, double  *miny, double  *maxx, double  *maxy)
{
	TblobGet_BOX(BlobResId,minx,miny,maxx,maxy);
}
//	virtual void blobGet_BOX(int64 BlobResId,int64 Feature, float  *minx, float  *miny, float  *maxx, float  *maxy);
void CPilImg::blobGet_BOX(int64 BlobResId, int64  *minx, int64  *miny, int64  *maxx, int64  *maxy)
{
	TblobGet_BOX(BlobResId,minx,miny,maxx,maxy);
}
void CPilImg::blobGet_BOX(int64 BlobResId, MIL_INT32  *minx, MIL_INT32  *miny, MIL_INT32  *maxx, MIL_INT32  *maxy)
{
	TblobGet_BOX(BlobResId,minx,miny,maxx,maxy);
}
template<typename T>
void CPilImg::TblobGet_BOX(int64 BlobResId,T*  minx, T  *miny, T  *maxx, T  *maxy)
{
	if(BlobResId == 0)
		return;
	((jsl::Blob *)BlobResId)->GetMinMax(minx,miny,maxx,maxy);
}

void CPilImg::blobGet_Center(int64 BlobResId,double    *cx, double  *cy)
{
	if(BlobResId == 0)
		return;
	((jsl::Blob *)BlobResId)->GetCenter(cx, cy);
}
void CPilImg::blobGet_Center(int64 BlobResId,int64  *cx, int64  *cy)
{
	if(BlobResId == 0)
		return;
	((jsl::Blob *)BlobResId)->GetCenter(cx, cy);
}
void CPilImg::blobGet_Center(int64 BlobResId,MIL_INT32  *cx, MIL_INT32  *cy)
{
	if(BlobResId == 0)
		return;
	((jsl::Blob *)BlobResId)->GetCenter(cx, cy);
}



// CMilImg
CMilImg::CMilImg(void)
{
	_LibType = Im::eType::MIL;
}
CMilImg::~CMilImg(void)
{
}
void CMilImg::blobFree(int64 mil_id)
{
	MblobFree(mil_id);
}
void CMilImg::blobReconstruct(int64 srce_image_id, int64 BlobResId,int64 dest_image_id, MIL_INT operation, MIL_INT mode)
{
	MblobReconstruct(srce_image_id, BlobResId, dest_image_id, operation, mode);
}
void CMilImg::graColor(int64 GraphContextId,MIL_DOUBLE ForegroundColor)
{
	MgraColor(GraphContextId, ForegroundColor);
}
void CMilImg::blobDraw(int64 GraphContId, int64 ResultId, int64 DestImageId,int64 Operation, int64 Label, int64 ControlFlag)
{
	MblobDraw(GraphContId , ResultId, DestImageId, Operation, Label, ControlFlag);
}
void CMilImg::blobControl(int64 BlobResId, int64 ProcMode, double Value)
{
	MblobControl(BlobResId, ProcMode, Value);
}
void CMilImg::blobCalculate(int64 BlobIdentImageId, int64 GreyImageId,int64 FeatureListId, int64 BlobResId)
{
	MblobCalculate(BlobIdentImageId, GreyImageId,FeatureListId, BlobResId);
}
void CMilImg::blobSelect(int64 BlobResId, int64 Operation, int64 Feature,int64 Condition, double CondLow, double CondHigh)
{
	MblobSelect(BlobResId, Operation, Feature,Condition, CondLow, CondHigh);
}
void CMilImg::blobFill(int64 BlobResId, int64 TargetImageId, int64 Mode,int64 Value)
{
	MblobFill(BlobResId, TargetImageId, Mode,Value);
}
int64 CMilImg::blobGetNumber(int64 BlobResId, MIL_INT *CountPtr)
{
	return MblobGetNumber(BlobResId, CountPtr);
}
int64 CMilImg::blobAllocFeatureList(int64 SystemId,MIL_INT *FeatureListPtr)
{
	return MblobAllocFeatureList(SystemId, FeatureListPtr);
}
int64 CMilImg::blobAllocResult(int64 SystemId, MIL_INT *BlobResIdPtr)
{
	return MblobAllocResult(SystemId, BlobResIdPtr);
}
void CMilImg::blobGetResult(int64 BlobResId,int64 Feature, double  *TargetArrayPtr)
{

	TBolbResult(BlobResId,Feature, TargetArrayPtr);
}
void CMilImg::blobGetResult(int64 BlobResId,int64 Feature, int64  *TargetArrayPtr)
{

	TBolbResult(BlobResId,Feature, TargetArrayPtr);
}
void CMilImg::blobGetResult(int64 BlobResId,int64 Feature, MIL_INT32  *TargetArrayPtr)
{

	TBolbResult(BlobResId,Feature, TargetArrayPtr);
}
template<typename T>
void CMilImg::TBolbResult(int64 BlobResId,int64 Feature, T*  TargetArrayPtr)
{
	MblobGetResult(BlobResId,Feature, TargetArrayPtr);
}
int64 CMilImg::blobGetLabel(int64 BlobResId, int64 XPos, int64 YPos,MIL_INT *BlobLabelPtr)
{
	return MblobGetLabel(BlobResId, XPos, YPos,BlobLabelPtr);
}
void CMilImg::blobLabel(int64 BlobResId, int64 TargetImageId, int64 Mode)
{
	MblobLabel(BlobResId, TargetImageId, Mode);
}
void CMilImg::blobSelectFeature(int64 FeatureListId, int64 Feature)
{
	MblobSelectFeature(FeatureListId, Feature);
}
void CMilImg::blobGet_BOX(int64 BlobResId, char  *minx, char  *miny, char  *maxx, char  *maxy)
{
	MblobGetResult(BlobResId,M_BOX_X_MIN+M_TYPE_CHAR, minx);
	MblobGetResult(BlobResId,M_BOX_X_MAX+M_TYPE_CHAR, maxx);
	MblobGetResult(BlobResId,M_BOX_Y_MIN+M_TYPE_CHAR, miny);
	MblobGetResult(BlobResId,M_BOX_Y_MAX+M_TYPE_CHAR, maxy);
}
void CMilImg::blobGet_BOX(int64 BlobResId,double  *minx, double  *miny, double  *maxx, double  *maxy)
{
	MblobGetResult(BlobResId,M_BOX_X_MIN+M_TYPE_DOUBLE, minx);
	MblobGetResult(BlobResId,M_BOX_X_MAX+M_TYPE_DOUBLE, maxx);
	MblobGetResult(BlobResId,M_BOX_Y_MIN+M_TYPE_DOUBLE, miny);
	MblobGetResult(BlobResId,M_BOX_Y_MAX+M_TYPE_DOUBLE, maxy);
}
//	virtual void blobGet_BOX(int64 BlobResId,int64 Feature, float  *minx, float  *miny, float  *maxx, float  *maxy);
void CMilImg::blobGet_BOX(int64 BlobResId,int64  *minx, int64  *miny, int64  *maxx, int64  *maxy)
{
	MblobGetResult(BlobResId,M_BOX_X_MIN+M_TYPE_MIL_INT64, minx);
	MblobGetResult(BlobResId,M_BOX_X_MAX+M_TYPE_MIL_INT64, maxx);
	MblobGetResult(BlobResId,M_BOX_Y_MIN+M_TYPE_MIL_INT64, miny);
	MblobGetResult(BlobResId,M_BOX_Y_MAX+M_TYPE_MIL_INT64, maxy);
}
void CMilImg::blobGet_BOX(int64 BlobResId, MIL_INT32  *minx, MIL_INT32  *miny, MIL_INT32  *maxx, MIL_INT32  *maxy)
{
	MblobGetResult(BlobResId,M_BOX_X_MIN+M_TYPE_MIL_INT32, minx);
	MblobGetResult(BlobResId,M_BOX_X_MAX+M_TYPE_MIL_INT32, maxx);
	MblobGetResult(BlobResId,M_BOX_Y_MIN+M_TYPE_MIL_INT32, miny);
	MblobGetResult(BlobResId,M_BOX_Y_MAX+M_TYPE_MIL_INT32, maxy);
}
void CMilImg::blobGet_Center(int64 BlobResId,double    *cx, double  *cy)
{
	MblobGetResult(BlobResId,M_CENTER_OF_GRAVITY_X + M_TYPE_DOUBLE, cx);
	MblobGetResult(BlobResId,M_CENTER_OF_GRAVITY_Y + M_TYPE_DOUBLE, cy);
}
void CMilImg::blobGet_Center(int64 BlobResId,int64  *cx, int64  *cy)
{
	MblobGetResult(BlobResId,M_CENTER_OF_GRAVITY_X + M_TYPE_MIL_INT64, cx);
	MblobGetResult(BlobResId,M_CENTER_OF_GRAVITY_Y + M_TYPE_MIL_INT64, cy);
}
void CMilImg::blobGet_Center(int64 BlobResId,MIL_INT32  *cx, MIL_INT32  *cy)
{
	MblobGetResult(BlobResId,M_CENTER_OF_GRAVITY_X + M_TYPE_MIL_INT32, cx);
	MblobGetResult(BlobResId,M_CENTER_OF_GRAVITY_Y + M_TYPE_MIL_INT32, cy);
}


// Sys
Im::PIL_ID Im::MILSys::Alloc(MIL_CONST_TEXT_PTR SystemDescriptor, int64 SystemNum, int64 InitFlag, Im::PIL_ID *SystemIdPtr)
{
	MIL_ID* mil_id = (MIL_ID*)SystemIdPtr;
	return MsysAlloc(SystemDescriptor, SystemNum, InitFlag, mil_id);
}
void Im::MILSys::Free(Im::PIL_ID SystemId)
{
	MsysFree((MIL_ID)SystemId);
}
int64 Im::MILSys::Inquire(Im::PIL_ID SystemId, int64 InquireType, void *UserVarPtr)
{
	int64 nRet = 0;
	nRet = MsysInquire(SystemId, InquireType, M_NULL);
	int64 * Var = (int64 *)UserVarPtr;
	if(Var != M_NULL)
	{
		*Var = nRet;
	}
	return nRet;
}

// App
Im::PIL_ID Im::MILApp::Alloc(int64 InitFlag, Im::PIL_ID *ApplicationIdPtr)
{
	MIL_ID* mil_id = (MIL_ID*)ApplicationIdPtr;
	return MappAlloc(InitFlag, mil_id);
}
void Im::MILApp::Free(Im::PIL_ID ApplicationId)
{
	MappFree((MIL_ID)ApplicationId);
}
void Im::MILApp::Control(int64 ControlType, int64 ControlValue)
{
	MappControl(ControlType, ControlValue);
}
void Im::MILApp::Timer(int64 Mode, double *Time)
{
	MappTimer(Mode, Time);
}
int64 Im::MILApp::Inquire(int64 InquireType, void *UserVarPtr)
{
	int64 nRet = 0;
	int64 * Var = (int64 *)UserVarPtr;
	nRet = MappInquire(InquireType, M_NULL);

	if(Var != NULL)
	{
		*Var = nRet;
	}
	return nRet;
}

// iBuf
UCHAR* Im::iBuf::Load(CString strFilename)
{
	Im::PILBuf pPB;

	return pPB.Load(strFilename);
}
// Dig
void Im::MILDig::Free(PIL_ID DevId)
{
	MdigFree((MIL_ID)DevId);
}

// Buf
PIL_ID Im::PILBuf::Alloc2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *IdVarPtr)
{
	int cvType = CV_8UC1;
	if(Type & M_FLOAT)
	{
		cvType = CV_32FC1;
	}
	else if(Type & M_SIGNED)
	{
		if(Type & 32)
			cvType = CV_32SC1;
		else if(Type & 16)
			cvType = CV_16SC1;
		else if(Type & 8)
			cvType = CV_8SC1;
	}
	else //if(Type & M_UNSIGNED)
	{
		if(Type & 32)
			cvType = CV_32FC1;
		else if(Type & 16)
			cvType = CV_16UC1;
		else if(Type & 8)
			cvType = CV_8UC1;
	}

	cv::Mat *img = new cv::Mat(cv::Size(SizeX, SizeY), cvType);
	g_pMManager->pem_new_check(img, (PCHAR)__FUNCTION__, __LINE__);
	PIL_ID id = (PIL_ID)img;
	if(IdVarPtr != nullptr)
		*IdVarPtr = id;
	return id;
}
PIL_ID Im::PILBuf::Create2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void *DataPtr, PIL_ID *IdVarPtr)
{
	int cvType = 0;
	if(Type & M_FLOAT)
	{
		cvType = CV_32FC1;
	}
	else if(Type & M_SIGNED)
	{
		if(Type & 32)
			cvType = CV_32SC1;
		else if(Type & 16)
			cvType = CV_16SC1;
		else if(Type & 8)
			cvType = CV_8SC1;
	}
	else //if(Type & M_UNSIGNED)
	{
		if(Type & 16)
			cvType = CV_16UC1;
		else if(Type & 8)
			cvType = CV_8UC1;
	}
	
	if (Pitch == M_DEFAULT)
	{
		Pitch = 0;
	}

	cv::Mat *img = new cv::Mat(cv::Size(SizeX, SizeY), cvType, DataPtr, Pitch);
	g_pMManager->pem_new_check(img, (PCHAR)__FUNCTION__, __LINE__);
	PIL_ID id = (PIL_ID)img;
	if(IdVarPtr != nullptr)
		*IdVarPtr = id;
	return id;
}
void Im::PILBuf::Free(Im::PIL_ID BufId)
{
	if(BufId == 0)
		return;
	cv::Mat *img = (cv::Mat*)BufId;
	//delete img;
	g_pMManager->pem_delete(img, false);
}
void Im::PILBuf::Load(CString Filename, PIL_ID BufId)
{
	cv::Mat img = cv::imread(std::string(CT2A((LPCTSTR)Filename)), cv::IMREAD_UNCHANGED);
	//cv::Mat * pImg = new cv::Mat();
	cv::Mat * pImg = g_pMManager->pem_new<cv::Mat>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	img.copyTo(*pImg);
	BufId = (PIL_ID)pImg;
}
UCHAR* Im::PILBuf::Load(CString strFilename)
{
	cv::Mat img = cv::imread(std::string(CT2A((LPCTSTR)strFilename)), cv::IMREAD_COLOR);
	//cv::Mat * pImg = new cv::Mat();
	cv::Mat * pImg = g_pMManager->pem_new<cv::Mat>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	img.copyTo(*pImg);

	//CString str = _T("D:\\Convert2.bmp");
	//cv::imwrite(std::string(CT2A((LPCTSTR)str)), *pImg);

	PIL_ID BufId = (PIL_ID)pImg;

	int src_sz_x = Inquire(BufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(BufId, M_SIZE_Y, M_NULL);
	int srcPitch = Inquire(BufId, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Inquire(BufId, M_HOST_ADDRESS, M_NULL);

	return srcPtr;
	// 	{
	// 		memcpy(&bufPtr[y*sizeof(UCHAR)*src_sz_x*3 + sizeof(UCHAR)*OffX*3], &srcPtr[yi*srcPitch], sizeof(UCHAR)*SizeX*3);
	// 	}
}
PIL_ID Im::PILBuf::Import(CString Filename, int64 FileFornmat, int64 Operation, PIL_ID SystemId, PIL_ID *BufIdPtr)
{
	cv::Mat img = cv::imread(std::string(CT2A((LPCTSTR)Filename)), cv::IMREAD_UNCHANGED);
	//cv::Mat * pImg = new cv::Mat();
	cv::Mat * pImg = g_pMManager->pem_new<cv::Mat>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	img.copyTo(*pImg);

	PIL_ID id = (PIL_ID)pImg;
	if(BufIdPtr != nullptr)
		*BufIdPtr = id;
	return id;
}
void Im::PILBuf::Export(CString FileName, int64 FileFormat, PIL_ID srcBufId)
{
	cv::Mat *img = (cv::Mat*)srcBufId;
	ImSave(FileName,img);
	//cv::imwrite(std::string(CT2A((LPCTSTR)FileName)), *img); 
}
void Im::PILBuf::Save(CString FileName, PIL_ID BufId)
{
	cv::Mat *img = (cv::Mat*)BufId;
	ImSave(FileName,img);
	//cv::imwrite(std::string(CT2A((LPCTSTR)FileName)), *img);
}
int64 Im::PILBuf::Inquire(PIL_ID BufId, int64 InquireType, void *ResultPtr)
{
	cv::Mat *img = (cv::Mat*)BufId;

	int64 ret = 0;
	if (img == nullptr)
	{
		assert(img != nullptr);
		if (ResultPtr != NULL)	ResultPtr = (void*)ret;
		return 0;
	}
		
	switch(InquireType)
	{
	case M_SIZE_X:
		ret = img->cols;
		if(ResultPtr != NULL)	ResultPtr = (void*)ret;
		break;
	case M_SIZE_Y:
		ret = img->rows;
		if(ResultPtr != NULL)	ResultPtr = (void*)ret;
		break;
	case M_PITCH:
		ret = img->step1();
		if(ResultPtr != NULL)	ResultPtr = (void*)ret;
		break;
	case M_PITCH_BYTE:
		ret = img->step;
		if(ResultPtr != NULL)	ResultPtr = (void*)ret;
		break;
	case M_HOST_ADDRESS:
		{
			int64 * Var = (int64 *)ResultPtr;
			UCHAR * _ptr = img->ptr();
			ret = (int64)(_ptr);
			if(Var != NULL)
				*Var = ret;
			break;
		}
	case M_SIZE_BAND:
		{
			ret = img->channels();
			if(ResultPtr != NULL)	ResultPtr = (void*)ret;
			break;
		}
	default:
		break;
	}
	return ret;
}
template<typename T>
void Im::PILBuf::Put(T* src, T* dst, int size_x, int size_y)
{
	for (int y = 0; y < size_y; y++)
	{
		memcpy(&src[y*size_x], &dst[y*size_x], size_x*sizeof(T));
	}
}
void Im::PILBuf::Put(PIL_ID DestBufId, const void *UserArrayPtr)
{
	int src_sz_x = Inquire(DestBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(DestBufId, M_SIZE_Y, M_NULL);
	int srcPitch = Inquire(DestBufId, M_PITCH, M_NULL);
	int srcPitchByte = Inquire(DestBufId, M_PITCH_BYTE, M_NULL);

	int type = srcPitchByte / src_sz_x;
	if(type == 4)
	{
		long * srcPtr = (long *)Inquire(DestBufId, M_HOST_ADDRESS, M_NULL);
		long * bufPtr = (long *)UserArrayPtr;

		Put<long>(srcPtr, bufPtr, src_sz_x, src_sz_y);
	}
	else //if(type == 1)
	{
		UCHAR * srcPtr = (UCHAR *)Inquire(DestBufId, M_HOST_ADDRESS, M_NULL);
		UCHAR * bufPtr = (UCHAR *)UserArrayPtr;

		Put<UCHAR>(srcPtr, bufPtr, src_sz_x, src_sz_y);
	}
	

	/*
	UCHAR * srcPtr = (UCHAR *)Inquire(DestBufId, M_HOST_ADDRESS, M_NULL);

	UCHAR* bufPtr = (UCHAR*)UserArrayPtr;

	//for (int y = 0; y < src_sz_y; y++)
	//{
	//	memcpy(&srcPtr[y*srcPitch], &bufPtr[y*srcPitch], srcPitch);
	//}
	memcpy(&srcPtr[0], &bufPtr[0], src_sz_y*srcPitchByte);
	*/
}

void Im::PILBuf::Put2d(PIL_ID DestBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr)
{
	int src_sz_x = Inquire(DestBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(DestBufId, M_SIZE_Y, M_NULL);
	int srcPitch = Inquire(DestBufId, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Inquire(DestBufId, M_HOST_ADDRESS, M_NULL);

	UCHAR* bufPtr = (UCHAR*)BufferPtr;

	for (int y = Offy, yi = 0; yi < Sizey; y++, yi++)
	{
		memcpy(&srcPtr[y*srcPitch + Offx], &bufPtr[yi*Sizex], sizeof(UCHAR) * Sizex);
	}
}
void Im::PILBuf::Get(PIL_ID SrcBufId, void *UserArrayPtr)
{
	int src_sz_x = Inquire(SrcBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(SrcBufId, M_SIZE_Y, M_NULL);
	int srcPitch = Inquire(SrcBufId, M_PITCH, M_NULL);
	int srcPitchByte = Inquire(SrcBufId, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Inquire(SrcBufId, M_HOST_ADDRESS, M_NULL);

	UCHAR* bufPtr = (UCHAR*)UserArrayPtr;

	memcpy(&bufPtr[0], &srcPtr[0], src_sz_y*srcPitchByte);
}
void Im::PILBuf::Get2d(PIL_ID SourceBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr)
{
	cv::Mat* SrcMat = (cv::Mat*)SourceBufId;
	if (SourceBufId <= 0)
		return;
	int src_sz_x = Inquire(SourceBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(SourceBufId, M_SIZE_Y, M_NULL);
	int srcPitch = Inquire(SourceBufId, M_PITCH, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Inquire(SourceBufId, M_HOST_ADDRESS, M_NULL);

	UCHAR* bufPtr = (UCHAR*)BufferPtr;

	for (int y = Offy, yi = 0; yi < Sizey; y++, yi++)
	{
		memcpy(&bufPtr[yi*Sizex], &srcPtr[y*srcPitch + Offx], sizeof(UCHAR) * Sizex);
	}
}
PIL_ID Im::PILBuf::CreateColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void **ArrayOfDataPtr, PIL_ID *BufIdPtr)
{
	int cvType = 0;
	if(Type & M_FLOAT)
	{
		cvType = CV_32FC3;
	}
	else if(Type & M_SIGNED)
	{
		if(Type & 32)
			cvType = CV_32SC3;
		else if(Type & 16)
			cvType = CV_16SC3;
		else if(Type & 8)
			cvType = CV_8SC3;
	}
	else //if(Type & M_UNSIGNED)
	{
		if(Type & 16)
			cvType = CV_16UC3;
		else if(Type & 8)
			cvType = CV_8UC3;
	}

	if (Pitch == M_DEFAULT)
	{
		Pitch = 0;
	}

	cv::Mat *img = nullptr;
	img = new cv::Mat(cv::Size(SizeX, SizeY), cvType, *ArrayOfDataPtr, Pitch);

	g_pMManager->pem_new_check(img, (PCHAR)__FUNCTION__, __LINE__);
	PIL_ID id = (PIL_ID)img;
	if(BufIdPtr != nullptr)
		*BufIdPtr = id;
	return id;
}
PIL_ID Im::PILBuf::AllocColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *BufIdPtr)
{
	int cvType = 0;
	if(Type & M_FLOAT)
	{
		cvType = CV_32FC3;
	}
	else if(Type & M_SIGNED)
	{
		if(Type & 32)
			cvType = CV_32SC3;
		else if(Type & 16)
			cvType = CV_16SC3;
		else if(Type & 8)
			cvType = CV_8SC3;
	}
	else //if(Type & M_UNSIGNED)
	{
		if(Type & 16)
			cvType = CV_16UC3;
		else if(Type & 8)
			cvType = CV_8UC3;
	}

	cv::Mat *img = new cv::Mat(cv::Size(SizeX, SizeY), cvType);
	g_pMManager->pem_new_check(img, (PCHAR)__FUNCTION__, __LINE__);
	PIL_ID id = (PIL_ID)img;
	if(BufIdPtr != nullptr)
		*BufIdPtr = id;
	return id;
}
void Im::PILBuf::PutColor(PIL_ID DestBufId, int64 DataFormat, int64 Band, const void *UserArrayPtr) 
{
	int src_sz_x = Inquire(DestBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(DestBufId, M_SIZE_Y, M_NULL);
	int srcPitchByte = Inquire(DestBufId, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Inquire(DestBufId, M_HOST_ADDRESS, M_NULL);

	UCHAR* bufPtr = (UCHAR*)UserArrayPtr;

	for (int y = 0; y < src_sz_y; y++)
	{
		memcpy(&srcPtr[y*sizeof(UCHAR)*src_sz_x*3], &bufPtr[y*sizeof(UCHAR)*src_sz_x*3], sizeof(UCHAR)*src_sz_x*3);
	}
}
void Im::PILBuf::PutColor2d(PIL_ID DestBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, const void *UserArrayPtr) 
{
	int src_sz_x = Inquire(DestBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(DestBufId, M_SIZE_Y, M_NULL);
	int srcPitch = Inquire(DestBufId, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Inquire(DestBufId, M_HOST_ADDRESS, M_NULL);

	UCHAR* bufPtr = (UCHAR*)UserArrayPtr;

	for (int y = OffY, yi = 0; yi < SizeY; y++, yi++)
	{
		memcpy(&srcPtr[y*sizeof(UCHAR)*src_sz_x*3 + sizeof(UCHAR)*OffX*3], &bufPtr[yi*sizeof(UCHAR)*SizeX*3], sizeof(UCHAR)*SizeX*3);
	}
}
void Im::PILBuf::GetColor(PIL_ID SrcBufId, int64 DataFormat, int64 Band, void *UserArrayPtr)
{
	int src_sz_x = Inquire(SrcBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(SrcBufId, M_SIZE_Y, M_NULL);
	int srcPitchByte = Inquire(SrcBufId, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Inquire(SrcBufId, M_HOST_ADDRESS, M_NULL);

	UCHAR* bufPtr = (UCHAR*)UserArrayPtr;

	for (int y = 0; y < src_sz_y; y++)
	{
		memcpy(&bufPtr[y*srcPitchByte], &srcPtr[y*srcPitchByte], srcPitchByte);
	}
}
void Im::PILBuf::GetColor2d(PIL_ID SrcBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, void *UserArrayPtr)
{
	int src_sz_x = Inquire(SrcBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Inquire(SrcBufId, M_SIZE_Y, M_NULL);
	int srcPitch = Inquire(SrcBufId, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Inquire(SrcBufId, M_HOST_ADDRESS, M_NULL);

	UCHAR* bufPtr = (UCHAR*)UserArrayPtr;

	for (int y = OffY, yi = 0; yi < SizeY; y++, yi++)
	{
		memcpy(&bufPtr[y*sizeof(UCHAR)*src_sz_x*3 + sizeof(UCHAR)*OffX*3], &srcPtr[yi*srcPitch], sizeof(UCHAR)*SizeX*3);
	}
}
void Im::PILBuf::Copy(PIL_ID SrcBufId, PIL_ID DestBufId)
{
	cv::Mat* src = (cv::Mat*)SrcBufId;
	cv::Mat* dest = (cv::Mat*)DestBufId;
	src->copyTo(*dest);
}
void Im::PILBuf::CopyColor(PIL_ID SrcBufId, PIL_ID DestBufId, int64 Band) 
{
	cv::Mat* src = (cv::Mat*)SrcBufId;
	cv::Mat* dest = (cv::Mat*)DestBufId;
	int src_ch = src->channels();
	int dest_ch = dest->channels();
	if (src_ch == dest_ch)
	{
		src->copyTo(*dest);
	}
	else if(src_ch == 3 && dest_ch == 1)
	{
		cv::cvtColor(*src, *dest, cv::COLOR_RGB2GRAY);
	}
	else if(src_ch == 1 && dest_ch == 3)
	{
		cv::cvtColor(*src, *dest, cv::COLOR_GRAY2RGB);
	}
}
void Im::PILBuf::CopyClip(PIL_ID SrcBufId, PIL_ID DestBufId, int64 DestOffX, int64 DestOffY)
{
	cv::Mat* src = (cv::Mat*)SrcBufId;
	cv::Mat* dest = (cv::Mat*)DestBufId;
	int w = dest->cols;
	int h = dest->rows;

	cv::Mat img;
	cv::Rect rc(DestOffX, DestOffY, w, h);
	img = (*src)(rc);
	img.copyTo(*dest);
}
void Im::PILBuf::Clear(PIL_ID DestImageBufId, double Color)
{
	cv::Mat* dest = (cv::Mat*)DestImageBufId;
	dest->setTo(cv::Scalar(Color));
}
PIL_ID Im::PILBuf::Child2d(PIL_ID ParentBufId, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, PIL_ID *BufIdPtr)
{
	cv::Mat* ParentBuf = (cv::Mat*)ParentBufId;
	cv::Rect rc(OffX, OffY, SizeX, SizeY);
	
	cv::Mat *img = new cv::Mat(cv::Size(SizeX, SizeY), ParentBuf->type());
	g_pMManager->pem_new_check(img, (PCHAR)__FUNCTION__, __LINE__);
	(*ParentBuf)(rc).copyTo(*img);
	PIL_ID id = (PIL_ID)img;
	if(BufIdPtr != nullptr)
		*BufIdPtr = id;
	return id;
}
void Im::PILBuf::ImSave(CString sPath,cv::Mat *img)
{
	CString extension = sPath.Mid(sPath.GetLength()-4, 4).MakeLower();
	std::vector<uchar> vBuf;
	cv::imencode(std::string(CT2A((LPCTSTR)extension)), *img,vBuf);

	CFile file;
	CFileException fe;
	if(file.Open(sPath, CFile::modeCreate | CFile::modeWrite /*| CFile::shareDenyRead || CFile::shareDenyWrite*/, NULL, &fe)==FALSE)
		return;
	file.Write(&vBuf[0], vBuf.size());
// 	CArchive ar(&file, CArchive::store);
// 
// 	try
// 	{
// 		for(int i=0;i<vBuf.size();i++)
// 			ar<<vBuf[i];
// 	}
// 	catch(...)
// 	{
// 	}
// 
// 	ar.Close();
	file.Close();
}

PIL_ID Im::MILBuf::Alloc2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *IdVarPtr)
{
	return MbufAlloc2d(SystemId, SizeX, SizeY, Type, Attribute, (MIL_ID *)IdVarPtr);
}
PIL_ID Im::MILBuf::Create2d(PIL_ID SystemId, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void *DataPtr, PIL_ID *IdVarPtr)
{
	return MbufCreate2d(SystemId, SizeX, SizeY, Type, Attribute, ControlFlag, Pitch, DataPtr, (MIL_ID *)IdVarPtr);
}
void Im::MILBuf::Free(Im::PIL_ID BufId)
{
	MbufFree((MIL_ID)BufId);
}
void Im::MILBuf::Load(CString Filename, PIL_ID BufId)
{
	MbufLoad(Filename, BufId);
}
PIL_ID Im::MILBuf::Import(CString Filename, int64 FileFormat, int64 Operation, PIL_ID SystemId, PIL_ID *BufIdPtr)
{
	return MbufImport(Filename.GetBuffer(), FileFormat, Operation, SystemId, (MIL_ID *)BufIdPtr);
}
void Im::MILBuf::Export(CString FileName, int64 FileFormat, PIL_ID srcBufId)
{
	MbufExport(FileName, FileFormat, srcBufId);
}
void Im::MILBuf::Save(CString FileName, PIL_ID BufId)
{
	MbufSave(FileName, BufId);
}
int64 Im::MILBuf::Inquire(PIL_ID BufId, int64 InquireType, void *ResultPtr)
{
	int64 nRet = 0;
	int64 * Var = (int64 *)ResultPtr;
	nRet = MbufInquire((MIL_ID)BufId, InquireType, M_NULL);
	if(Var != M_NULL)
	{
		*Var = nRet;
	}
	return nRet;
}
void Im::MILBuf::Put(PIL_ID DestBufId, const void *UserArrayPtr)
{
	MbufPut(DestBufId, UserArrayPtr);
}

void Im::MILBuf::Put2d(PIL_ID DestBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr)
{
	MbufPut2d(DestBufId, Offx, Offy, Sizex, Sizey, BufferPtr);
}
void Im::MILBuf::Get(PIL_ID SrcBufId, void *UserArrayPtr)
{
	MbufGet(SrcBufId, UserArrayPtr);
}
void Im::MILBuf::Get2d(PIL_ID SourceBufId, int64 Offx, int64 Offy, int64 Sizex, int64 Sizey, void *BufferPtr)
{
	MbufGet2d(SourceBufId, Offx, Offy, Sizex, Sizey, BufferPtr);
}
PIL_ID Im::MILBuf::CreateColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, int64 ControlFlag, int64 Pitch, void **ArrayOfDataPtr, PIL_ID *BufIdPtr) 
{
	return MbufCreateColor(SystemId, SizeBand, SizeX, SizeY, Type, Attribute, ControlFlag, Pitch, ArrayOfDataPtr, (MIL_ID*)BufIdPtr);
}
PIL_ID Im::MILBuf::AllocColor(PIL_ID SystemId, int64 SizeBand, int64 SizeX, int64 SizeY, int64 Type, int64 Attribute, PIL_ID *BufIdPtr)
{
	return MbufAllocColor(SystemId, SizeBand, SizeX, SizeY, Type, Attribute, (MIL_ID*)BufIdPtr);
}
void Im::MILBuf::PutColor(PIL_ID DestBufId, int64 DataFormat, int64 Band, const void *UserArrayPtr) 
{
	MbufPutColor(DestBufId, DataFormat, Band, UserArrayPtr);
}
void Im::MILBuf::PutColor2d(PIL_ID DestBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, const void *UserArrayPtr) 
{
	MbufPutColor2d(DestBufId, DataFormat, Band, OffX, OffY, SizeX, SizeY, UserArrayPtr);
}
void Im::MILBuf::GetColor(PIL_ID SrcBufId, int64 DataFormat, int64 Band, void *UserArrayPtr)
{
	MbufGetColor(SrcBufId, DataFormat, Band, UserArrayPtr);
}
void Im::MILBuf::GetColor2d(PIL_ID SrcBufId, int64 DataFormat, int64 Band, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, void *UserArrayPtr)
{
	MbufGetColor2d(SrcBufId, DataFormat, Band, OffX, OffY, SizeX, SizeY, UserArrayPtr);
}
void Im::MILBuf::Copy(PIL_ID SrcBufId, PIL_ID DestBufId)
{
	MbufCopy(SrcBufId, DestBufId);
}
void Im::MILBuf::CopyColor(PIL_ID SrcBufId, PIL_ID DestBufId, int64 Band) 
{
	MbufCopyColor(SrcBufId, DestBufId, Band);
}
void Im::MILBuf::CopyClip(PIL_ID SrcBufId, PIL_ID DestBufId, int64 DestOffX, int64 DestOffY)
{
	MbufCopyClip(SrcBufId, DestBufId, DestOffX, DestOffY);
}
void Im::MILBuf::Clear(PIL_ID DestImageBufId, double Color)
{
	MbufClear(DestImageBufId, Color);
}
PIL_ID Im::MILBuf::Child2d(PIL_ID ParentBufId, int64 OffX, int64 OffY, int64 SizeX, int64 SizeY, PIL_ID *BufIdPtr)
{
	return MbufChild2d(ParentBufId, OffX, OffY, SizeX, SizeY, (MIL_ID*)BufIdPtr);
}

//im

void Im::PILIm::RotateImg_ipp(cv::Mat userSrc, double angle, cv::Mat* userDst)
{
	unsigned char* ucuserDst;
	int dstSizeX = 0;
	int dstSizeY = 0;

	if (userSrc.channels() != 3)
	{
		//RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize)
		unsigned char* ucuserSrc;

		ucuserSrc = userSrc.data;

		double dangle = angle;
		int orgSizeX;
		int orgSizeY;
		orgSizeX = userSrc.cols;
		orgSizeY = userSrc.rows;

		bool bLinear = false;
		bool bColor = false;

		RotateImg_ipp2020(ucuserSrc, &ucuserDst, dangle, orgSizeX, orgSizeY, &dstSizeX, &dstSizeY, bColor, bLinear, false);

	}
	else
	{
		//RotateImg_ipp_color(src, angle, &dst, bInterpolation);
		unsigned char* ucuserSrc;

		ucuserSrc = userSrc.data;

		double dangle = angle;
		int orgSizeX;
		int orgSizeY;
		orgSizeX = userSrc.cols;
		orgSizeY = userSrc.rows;

		bool bLinear = false;
		bool bColor = true;

		RotateImg_ipp2020(ucuserSrc, &ucuserDst, dangle, orgSizeX, orgSizeY, &dstSizeX, &dstSizeY, bColor, bLinear, false);
	}

	if (userSrc.channels() != 3)
	{
		*userDst = cv::Mat(dstSizeY, dstSizeX, CV_8UC1);
		memcpy(userDst->data, ucuserDst, sizeof(UCHAR)*dstSizeY*dstSizeX);
		delete[] ucuserDst;
		ucuserDst = NULL;
	}
	else
	{
		*userDst = cv::Mat(dstSizeY, dstSizeX, CV_8UC3);
		memcpy(userDst->data, ucuserDst, sizeof(UCHAR)*dstSizeY*dstSizeX * 3);
		delete[] ucuserDst;
		ucuserDst = NULL;
	}
}

bool Im::PILIm::RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize)
{
	IppiSize srcSize = { orgSizeX,  orgSizeY };
	double angle = dangle;
	if (angle == 0)
	{
		if (dstSizeX)
			*dstSizeX = orgSizeX;
		if (dstSizeY)
			*dstSizeY = orgSizeY;

		if (bColor)
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * 3 * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C3R(userSrc, orgSizeX * 3, *userDst, orgSizeX * 3, srcSize);
		}
		else
		{
			if (*userDst == NULL)
				*userDst = g_pMManager->pem_new<uchar>(true, orgSizeX * orgSizeY, (PCHAR)__FUNCTION__, __LINE__);
			ippiCopy_8u_C1R(userSrc, orgSizeX, *userDst, orgSizeX, srcSize);
		}

		return true;
	}

	double coeffs[2][3] = { 0 };	//affine transform coefficients

	IppStatus status = ippStsNoErr;

	//1. Source
	int numChannels = 1;

	if (bColor)
		numChannels = 3;


	int srcStep = srcSize.width * numChannels;
	//Memory allocation for the intermediate images
	Ipp8u* pSrc = userSrc;
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };


	//2. Dst
	double xShift = 0.0, yShift = 0.0;
	ippiGetRotateShift(srcSize.width / 2, srcSize.height / 2, angle, &xShift, &yShift);

	//compute affine transform coefficients by angle and x- and y-shifts
	if (status >= ippStsNoErr) status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);

	//affine transform bounds
	double bound[2][2] = { 0 };

	//get bounds of transformed image
	if (status >= ippStsNoErr) status = ippiGetAffineBound(srcRoi, bound, coeffs);

	//set image step for rotated image
	IppiSize dstSize = { srcSize.width , srcSize.height };

	//??? ?? ??
	if (bUseOrgSize == false)
	{
		/*dstSize.width = (int)((int)(bound[1][0] + 0.5) - (int)(bound[0][0] - 0.5) + 1.0);
		dstSize.height = (int)((int)(bound[1][1] + 0.5) - (int)(bound[0][1] - 0.5) + 1.0);*/
		dstSize.width = (int)(bound[1][0] - bound[0][0] + 1.0);
		dstSize.height = (int)(bound[1][1] - bound[0][1] + 1.0);
	}

	//Memory allocation for the intermediate images
	//*userDst = new uchar[dstSize.width * numChannels * dstSize.height];
	*userDst = g_pMManager->pem_new<uchar>(true, dstSize.width * numChannels * dstSize.height, (PCHAR)__FUNCTION__, __LINE__);
	Ipp8u* pDstRoi = *userDst;

	//set offset of the processing destination ROI
	IppiPoint dstOffset = { 0, 0 };

	if (bUseOrgSize == false)
	{
		//??? ?? ?? Shift
		coeffs[0][2] -= bound[0][0];
		coeffs[1][2] -= bound[0][1];
	}

	//Affine
	//sizes for WarpAffine data structure, initialization buffer, work buffer
	int specSize = 0, initSize = 0, bufSize = 0;

	//pointer to work buffer
	Ipp8u* pBuffer = NULL;

	IppiBorderType borderType = ippBorderConst;
	IppiWarpDirection direction = ippWarpForward;

	//border value to extend the source image
	Ipp64f pBorderValue[3];

	//pointer to WarpAffine data structure
	IppiWarpSpec* pSpec = NULL;
	//IppStatus status = ippStsNoErr;

	//set border value to extend the source image
	for (int i = 0; i < numChannels; ++i) pBorderValue[i] = 0;


	if (bLinear)
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippLinear, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing

		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineLinear_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		// Spec and init buffer sizes
		status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippNearest, direction, borderType,
			&specSize, &initSize);

		pSpec = (IppiWarpSpec*)ippsMalloc_8u(specSize);

		//Affine transform data initialization
		if (status >= ippStsNoErr) status = ippiWarpAffineNearestInit(srcSize, dstSize, ipp8u, coeffs, direction, numChannels, borderType, pBorderValue, 0, pSpec);

		//Get work buffer size
		if (status >= ippStsNoErr) status = ippiWarpGetBufferSize(pSpec, dstSize, &bufSize);

		//allocate memory for work buffer
		pBuffer = ippsMalloc_8u(bufSize);

		//Affine transform processing
		if (numChannels == 1)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C1R(pSrc, srcStep, pDstRoi, dstSize.width, dstOffset, dstSize, pSpec, pBuffer);
		}
		else if (numChannels == 3)
		{
			if (status >= ippStsNoErr) status = ippiWarpAffineNearest_8u_C3R(pSrc, srcStep, pDstRoi, dstSize.width * 3, dstOffset, dstSize, pSpec, pBuffer);
		}
		else
		{
			assert(false);
		}
	}

	if (dstSizeX)
		* dstSizeX = dstSize.width;
	if (dstSizeY)
		* dstSizeY = dstSize.height;

	//Free memory
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return true;
}


void Im::PILIm::RotateImg_ipp_color(cv::Mat userSrc, double angle, cv::Mat* userDst)
{
	unsigned char* ucuserDst;
	int dstSizeX = 0;
	int dstSizeY = 0;

	if (userSrc.channels() != 3)
	{
		//RotateImg_ipp2020(unsigned char* userSrc, unsigned char** userDst, double dangle, int orgSizeX, int orgSizeY, int *dstSizeX, int *dstSizeY, bool bColor, bool bLinear, bool bUseOrgSize)
		unsigned char* ucuserSrc;

		ucuserSrc = userSrc.data;

		double dangle = angle;
		int orgSizeX;
		int orgSizeY;
		orgSizeX = userSrc.cols;
		orgSizeY = userSrc.rows;

		bool bLinear = false;
		bool bColor = false;

		RotateImg_ipp2020(ucuserSrc, &ucuserDst, dangle, orgSizeX, orgSizeY, &dstSizeX, &dstSizeY, bColor, bLinear, false);

	}
	else
	{
		//RotateImg_ipp_color(src, angle, &dst, bInterpolation);
		unsigned char* ucuserSrc;

		ucuserSrc = userSrc.data;

		double dangle = angle;
		int orgSizeX;
		int orgSizeY;
		orgSizeX = userSrc.cols;
		orgSizeY = userSrc.rows;

		bool bLinear = false;
		bool bColor = true;

		RotateImg_ipp2020(ucuserSrc, &ucuserDst, dangle, orgSizeX, orgSizeY, &dstSizeX, &dstSizeY, bColor, bLinear, false);
	}

	if (userSrc.channels() != 3)
	{
		*userDst = cv::Mat(dstSizeY, dstSizeX, CV_8UC1);
		memcpy(userDst->data, ucuserDst, sizeof(UCHAR)*dstSizeY*dstSizeX);
		delete[] ucuserDst;
		ucuserDst = NULL;
	}
	else
	{
		*userDst = cv::Mat(dstSizeY, dstSizeX, CV_8UC3);
		memcpy(userDst->data, ucuserDst, sizeof(UCHAR)*dstSizeY*dstSizeX * 3);
		delete[] ucuserDst;
		ucuserDst = NULL;
	}
}
void Im::PILIm::Free(PIL_ID ImResultId)
{
	if(ImResultId == 0)
		return;
	cv::Mat *img = (cv::Mat*)ImResultId;
	//delete img;
	g_pMManager->pem_delete(img, false);
}
void Im::PILIm::Rotate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double Angle, double SrcCenX, double SrcCenY, double DstCenX, double DstCenY, int64 InterpolationMode) 
{
	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;

	if(SrcImg->channels()==1)
		RotateImg_ipp(*SrcImg, Angle, DestImg);
	else
		RotateImg_ipp_color(*SrcImg, Angle, DestImg);
}
PIL_ID Im::PILIm::Binarize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 ConditionAndThreshMode, double LowParam, double HighParam)
{
	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;

	int Low = (int)LowParam;
	int High = (int)HighParam;
	switch(ConditionAndThreshMode)
	{
	case M_IN_RANGE:
		jsl::Binarize::InRange(*SrcImg, *DestImg, Low, High);
		break;
	case M_OUT_RANGE:
		jsl::Binarize::OutRange(*SrcImg, *DestImg, Low, High);
		break;
	case M_EQUAL:
		//
		break;
	case M_NOT_EQUAL:
		//
		break;
	case M_GREATER:
		jsl::Binarize::Greater(*SrcImg, *DestImg, Low);
		break;
	case M_LESS:
		jsl::Binarize::Less(*SrcImg, *DestImg, Low);
		break;
	case M_GREATER_OR_EQUAL:
		jsl::Binarize::GreaterEqual(*SrcImg, *DestImg, Low);
		break;
	case M_LESS_OR_EQUAL:
		jsl::Binarize::LessEqual(*SrcImg, *DestImg, Low);
		break;
	}

	return DestImageBufId;
}
void Im::PILIm::Close(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
{
	if(NbIteration < 1)
		return ;

	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;

	cv::morphologyEx(*SrcImg, *DestImg, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), NbIteration);
}
void Im::PILIm::Open(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode) 
{
	if(NbIteration < 1)
		return ;

	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;
	int morph_size = 2 * NbIteration + 1;

	cv::morphologyEx(*SrcImg, *DestImg, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), NbIteration);
}
void Im::PILIm::Erode(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
{
	if(NbIteration < 1)
		return ;

	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;
	int morph_size = 2 * NbIteration + 1;

	//cv::morphologyEx(*SrcImg, *DestImg, cv::MORPH_ERODE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), NbIteration);
	cv::erode(*SrcImg, *DestImg, cv::Mat(), cv::Point(-1, -1), NbIteration);
}
void Im::PILIm::Dilate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
{
	if(NbIteration < 1)
		return ;

	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;
	int morph_size = 2 * NbIteration + 1;

	//cv::morphologyEx(*SrcImg, *DestImg, cv::MORPH_DILATE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), NbIteration);
	cv::dilate(*SrcImg, *DestImg, cv::Mat(), cv::Point(-1, -1), NbIteration);
}
void Im::PILIm::Morphic(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Operation, int64 NbIterationOrArea, int64 ProcMode)
{
	if(NbIterationOrArea < 1)
		return ;

	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;
	cv::Mat* StructElem = (cv::Mat*)StructElemBufId;

	cv::Mat StructElem_temp;

	StructElem->convertTo(StructElem_temp, CV_8UC1);

	int nMode = 0;
	if(Operation & M_DILATE)
		nMode = cv::MORPH_DILATE;
	else if(Operation & M_ERODE)
		nMode = cv::MORPH_ERODE;
	
	if(ProcMode & M_GRAYSCALE)
	{
		cv::morphologyEx(*SrcImg, *DestImg, nMode, StructElem_temp, cv::Point(-1, -1), NbIterationOrArea);
	}	
	else	// M_BINARY
	{
		cv::morphologyEx(*SrcImg, *DestImg, nMode, StructElem_temp, cv::Point(-1, -1), NbIterationOrArea);
	}
}
void Im::PILIm::Rank(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Rank, int64 ProcMode)
{
	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;

	if(ProcMode & M_GRAYSCALE)
	{
		cv::medianBlur(*SrcImg, *DestImg, 3);	//M_3X3_RECT
	}
	else	// M_BINARY
	{
		cv::medianBlur(*SrcImg, *DestImg, 3);	//M_3X3_RECT
	}
}
void Im::PILIm::Arith(double Src1ImageBufIdOrConst, double Src2ImageBufIdOrConst, PIL_ID DestImageBufId, int64 Operation)
{
	PIL_ID src1 = Src1ImageBufIdOrConst;
	PIL_ID src2 = Src2ImageBufIdOrConst;
	cv::Mat* Src1Img = (cv::Mat*)src1;
	cv::Mat* Src2Img = (cv::Mat*)src2;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;

	UINT oper = Operation; 
	if(Operation & M_SATURATION)
		oper -= M_SATURATION;
	if(M_NEG == oper)
	{
		cv::Mat sub_mat = cv::Mat::ones(DestImg->size(), DestImg->type())*255;
		cv::subtract(sub_mat, *Src2Img, *DestImg);
	}
	else if(M_OR == oper)
	{
		cv::bitwise_or(*Src1Img, *Src2Img, *DestImg);
	}
	else if(M_SUB == oper)
	{
		cv::subtract(*Src1Img, *Src2Img, *DestImg);
	}
	else if(M_MULT == oper)
	{
		cv::multiply(*Src1Img, *Src2Img, *DestImg);
	}
	else	//if(M_ADD & Operation)
	{
		cv::add(*Src1Img, *Src2Img, *DestImg);
	}
	//MimArith(Src1ImageBufIdOrConst, Src2ImageBufIdOrConst, DestImageBufId, Operation);
}
void Im::PILIm::Resize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double ScaleFactorX, double ScaleFactorY, int64 InterpolationMode)
{
	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;

	cv::resize(*SrcImg, *DestImg, cv::Size(DestImg->cols, DestImg->rows));
}
void Im::PILIm::Translate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double XDisplacement, double YDisplacement, int64 InterpolationMode)
{
	if(XDisplacement == 0 && YDisplacement == 0)
		return;

	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* DestImg = (cv::Mat*)DestImageBufId;

	cv::Mat M = (cv::Mat_<float>(2,3) << 1, 0, XDisplacement, 0, 1, YDisplacement);
	cv::warpAffine(*SrcImg, *DestImg, M, cv::Size(DestImg->cols, DestImg->rows));

	//MimTranslate(SrcImageBufId, DestImageBufId, XDisplacement, YDisplacement, InterpolationMode);
}
void Im::PILIm::EdgeDetect(PIL_ID SrcImageBufId, PIL_ID DestIntensityImageBufId, PIL_ID DestAngleImageBufId, PIL_ID KernelId, int64 ControlFlag, int64 Threshold)
{
	// »ç¿ë ¾ÈÇÔ
	//MimEdgeDetect (SrcImageBufId, DestIntensityImageBufId, DestAngleImageBufId, KernelId, ControlFlag, Threshold);
}
PIL_ID Im::PILIm::AllocResult(PIL_ID SystemId, int64 NbEntries, int64 ResultType, PIL_ID *ImResultIdPtr) 	
{
	PIL_ID id;
	if(ResultType == M_HIST_LIST)
	{
		cv::Mat *img = new cv::Mat(NbEntries, 1, CV_32FC1);
		g_pMManager->pem_new_check(img, (PCHAR)__FUNCTION__, __LINE__);
		id = (PIL_ID)img;
	}
	else if(ResultType == M_PROJ_LIST)
	{
		cv::Mat *img = new cv::Mat(NbEntries, 1, CV_32FC1);
		g_pMManager->pem_new_check(img, (PCHAR)__FUNCTION__, __LINE__);
		id = (PIL_ID)img;
	}
	if(ImResultIdPtr != nullptr)
		*ImResultIdPtr = id;

	return 0;
}
void Im::PILIm::Histogram(PIL_ID SrcImageBufId, PIL_ID HistImResultId)
{
	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* HistResultImg = (cv::Mat*)HistImResultId;
	int histsize = 256; 
	float range[2] = { 0, 256 } ;
	const float* histRange = { range };
	int channels = { 0 };
	bool uniform = true;
	bool accumulate = false;

	cv::Mat hist;
	cv::calcHist(SrcImg, 1, 0, cv::Mat(), hist, 1, &histsize, &histRange, uniform, accumulate);

	hist.copyTo(*HistResultImg);
}
void Im::PILIm::GetResult(PIL_ID ImResultId, int64 ResultType, void *UserArrayPtr) 
{
	cv::Mat* ImResult = (cv::Mat*)ImResultId;
	float *ptr = ImResult->ptr<float>();
	int nRow = ImResult->rows;

	if(ResultType & M_TYPE_MIL_INT32)
	{
		int *UserPtr = (int *)UserArrayPtr;
		for(int i=0; i<nRow; i++)
		{
			UserPtr[i] = ptr[i];
		}
	}
	else if(ResultType & M_TYPE_LONG)
	{
		long *UserPtr = (long *)UserArrayPtr;
		for(int i=0; i<nRow; i++)
		{
			UserPtr[i] = ptr[i];
		}
	}
	else if(ResultType & M_TYPE_FLOAT)
	{
		memcpy(UserArrayPtr, ptr, sizeof(float) * ImResult->rows);
	}
}
void Im::PILIm::Project(PIL_ID SrcImageBufId, PIL_ID ProjImResultId, double ProjAngle) 
{
	cv::Mat* SrcImg = (cv::Mat*)SrcImageBufId;
	cv::Mat* ProjResult = (cv::Mat*)ProjImResultId;

	int src_sz_x = Buf::Inquire(SrcImageBufId, M_SIZE_X, M_NULL);
	int src_sz_y = Buf::Inquire(SrcImageBufId, M_SIZE_Y, M_NULL);
	int srcPitch = Buf::Inquire(SrcImageBufId, M_PITCH_BYTE, M_NULL);
	UCHAR * srcPtr = (UCHAR *)Buf::Inquire(SrcImageBufId, M_HOST_ADDRESS, M_NULL);

	float* ProRes = ProjResult->ptr<float>();

	if (ProjAngle == M_90_DEGREE)
	{
		for(int x=0; x<src_sz_x; x++)
		{
			for(int y=0; y<src_sz_y; y++)
			{
				ProRes[x] += srcPtr[y*srcPitch + x];
			}
		}
	}
	else	//M_0_DEGREE
	{
		for(int y=0; y<src_sz_y; y++)
		{
			for(int x=0; x<src_sz_x; x++)
			{
				ProRes[y] += srcPtr[y*srcPitch + x];
			}
		}
	}
}

void Im::MILIm::Free(PIL_ID ImResultId)
{
	MimFree(ImResultId);
}
void Im::MILIm::Rotate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double Angle, double SrcCenX, double SrcCenY, double DstCenX, double DstCenY, int64 InterpolationMode) 
{
	MimRotate(SrcImageBufId, DestImageBufId, Angle, SrcCenX, SrcCenY, DstCenX, DstCenY, InterpolationMode);
}
PIL_ID Im::MILIm::Binarize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 ConditionAndThreshMode, double LowParam, double HighParam)
{
	return MimBinarize(SrcImageBufId, DestImageBufId, ConditionAndThreshMode, LowParam, HighParam);
}
void Im::MILIm::Close(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
{
	MimClose(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
}
void Im::MILIm::Open(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode) 
{
	MimOpen(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
}
void Im::MILIm::Erode(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
{
	MimErode(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
}
void Im::MILIm::Dilate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, int64 NbIteration, int64 ProcMode)
{
	MimDilate(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
}
void Im::MILIm::Morphic(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Operation, int64 NbIterationOrArea, int64 ProcMode)
{
	MimMorphic((MIL_ID)SrcImageBufId, (MIL_ID)DestImageBufId, (MIL_ID)StructElemBufId, Operation, NbIterationOrArea, ProcMode);
}
void Im::MILIm::Rank(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, PIL_ID StructElemBufId, int64 Rank, int64 ProcMode)
{
	MimRank(SrcImageBufId, DestImageBufId, StructElemBufId, Rank, ProcMode);
}
void Im::MILIm::Arith(double Src1ImageBufIdOrConst, double Src2ImageBufIdOrConst, PIL_ID DestImageBufId, int64 Operation)
{
	MimArith(Src1ImageBufIdOrConst, Src2ImageBufIdOrConst, DestImageBufId, Operation);
}
void Im::MILIm::Resize(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double ScaleFactorX, double ScaleFactorY, int64 InterpolationMode)
{
	MimResize(SrcImageBufId, DestImageBufId, ScaleFactorX, ScaleFactorY, InterpolationMode);
}
void Im::MILIm::Translate(PIL_ID SrcImageBufId, PIL_ID DestImageBufId, double XDisplacement, double YDisplacement, int64 InterpolationMode)
{
	MimTranslate(SrcImageBufId, DestImageBufId, XDisplacement, YDisplacement, InterpolationMode);
}
void Im::MILIm::EdgeDetect (PIL_ID SrcImageBufId, PIL_ID DestIntensityImageBufId, PIL_ID DestAngleImageBufId, PIL_ID KernelId, int64 ControlFlag, int64 Threshold)
{
	MimEdgeDetect(SrcImageBufId, DestIntensityImageBufId, DestAngleImageBufId, KernelId, ControlFlag, Threshold);
}
PIL_ID Im::MILIm::AllocResult(PIL_ID SystemId, int64 NbEntries, int64 ResultType, PIL_ID *ImResultIdPtr) 	
{
	return MimAllocResult(SystemId, NbEntries, ResultType, (MIL_ID*)ImResultIdPtr);
}
void Im::MILIm::Histogram(PIL_ID SrcImageBufId, PIL_ID HistImResultId)
{
	MimHistogram(SrcImageBufId, HistImResultId);
}
void Im::MILIm::GetResult(PIL_ID ImResultId, int64 ResultType, void *UserArrayPtr) 
{
	if(ResultType & M_TYPE_MIL_INT32)
	{
		MimGetResult(ImResultId, ResultType, (MIL_UINT32*)UserArrayPtr);
	}
	else if(ResultType & M_TYPE_DOUBLE)
	{
		MimGetResult(ImResultId, ResultType, (double*)UserArrayPtr);
	}
	else if(ResultType & M_TYPE_FLOAT)
	{
		MimGetResult(ImResultId, ResultType, (float*)UserArrayPtr);
	}
	else if(ResultType & M_TYPE_LONG)
	{
		MimGetResult(ImResultId, ResultType, (long*)UserArrayPtr);
	}
}
void Im::MILIm::Project(PIL_ID SrcImageBufId, PIL_ID ProjImResultId, double ProjAngle) 
{
	MimProject(SrcImageBufId, ProjImResultId, ProjAngle);
}


// gra

Im::PILGra::PILGra(void)
{
	color = 0;
}
Im::PILGra::~PILGra(void)
{
}
void Im::PILGra::Rect(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
{
	cv::Mat* GraphCont = (cv::Mat*)GraphContId;
	cv::Mat* DestImageBufIdOrGraList = (cv::Mat*)DestImageBufIdOrGraListId;

	cv::Rect rect(cv::Point((int)XStart,(int)YStart), cv::Point((int)XEnd,(int)YEnd));
	cv::rectangle(*DestImageBufIdOrGraList, rect, color, 1);
}
void Im::PILGra::Color(PIL_ID GraphContId, double ForegroundColor)
{
	if(ForegroundColor == M_COLOR_BLACK)
		color = cv::Scalar(0,0,0);
	else if(ForegroundColor == M_COLOR_WHITE)
		color = cv::Scalar(255,255,255);
	else if(ForegroundColor == M_COLOR_GRAY)
		color = cv::Scalar(128,128,128);
	else if(ForegroundColor == M_COLOR_DARK_CYAN)
		color = cv::Scalar(169,169,169);
	else if(ForegroundColor == M_COLOR_RED)
		color = cv::Scalar(0,0,255);
	else if(ForegroundColor == M_COLOR_GREEN)
		color = cv::Scalar(0,128,0);
	else if(ForegroundColor == M_COLOR_BLUE)
		color = cv::Scalar(255,0,0);

}
void Im::PILGra::Lines(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, int64 NumberOfLinesOrVertices, const int64 *XStartArray, const int64 *YStartArray, const int64 *XEndArray, const int64 *YEndArray, int64 ControlFlag)
{
	cv::Mat* GraphCont = (cv::Mat*)GraphContId;
	cv::Mat* DestImageBufIdOrGraList = (cv::Mat*)DestImageBufIdOrGraListId;

	for(int i=0; i<NumberOfLinesOrVertices; i++)
	{
		cv::Point pt1((int)XStartArray[i], (int)YStartArray[i]);
		cv::Point pt2((int)XEndArray[i], (int)YEndArray[i]);
		cv::line(*DestImageBufIdOrGraList, pt1, pt2, color);
	}
}
void Im::PILGra::Line(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
{
	cv::Mat* GraphCont = (cv::Mat*)GraphContId;
	cv::Mat* DestImageBufIdOrGraList = (cv::Mat*)DestImageBufIdOrGraListId;

	cv::Point pt1((int)XStart, (int)YStart);
	cv::Point pt2((int)XEnd, (int)YEnd);
	cv::line(*DestImageBufIdOrGraList, pt1, pt2, color);
}
void Im::PILGra::RectFill(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
{
	cv::Mat* GraphCont = (cv::Mat*)GraphContId;
	cv::Mat* DestImageBufIdOrGraList = (cv::Mat*)DestImageBufIdOrGraListId;

	cv::Rect rect(cv::Point((int)XStart,(int)YStart), cv::Point((int)XEnd,(int)YEnd));

	cv::rectangle(*DestImageBufIdOrGraList, rect, color, -1);
}
void Im::PILGra::Text(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, CString StringPtr)
{
	// XStart , YStart : ¿ÞÂÊ »ó´Ü ÁÂÇ¥
	cv::Mat* GraphCont = (cv::Mat*)GraphContId;
	cv::Mat* DestImageBufIdOrGraList = (cv::Mat*)DestImageBufIdOrGraListId;

	std::string str = CT2A((LPCTSTR)StringPtr);
	int offset = 10;
	cv::Point pt((int)XStart, (int)YStart+offset);	// ¿ÞÂÊ ÇÏ´Ü ÁÂÇ¥
	cv::putText(*DestImageBufIdOrGraList, str, pt, 2, 0.5, cv::Scalar::all(255));
}
void Im::PILGra::Dot(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XPos, double YPos)
{
	cv::Mat* GraphCont = (cv::Mat*)GraphContId;
	cv::Mat* DestImageBufIdOrGraList = (cv::Mat*)DestImageBufIdOrGraListId;
	cv::Point pt((int)XPos, (int)YPos);
	if(DestImageBufIdOrGraList->channels() == 1)
	{
		DestImageBufIdOrGraList->at<UCHAR>(pt) = 255;
	}
}

void Im::MILGra::Rect(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
{
	MgraRect(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
}
void Im::MILGra::Color(PIL_ID GraphContId, double ForegroundColor)
{
	MgraColor(GraphContId, ForegroundColor);
}
void Im::MILGra::Lines(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, int64 NumberOfLinesOrVertices, const int64 *XStartArray, const int64 *YStartArray, const int64 *XEndArray, const int64 *YEndArray, int64 ControlFlag)
{
	MgraLines(GraphContId, DestImageBufIdOrGraListId, NumberOfLinesOrVertices, (double*)XStartArray, (double*)YStartArray, (double*)XEndArray, (double*)YEndArray, ControlFlag);
}
void Im::MILGra::Line(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
{
	MgraLine(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
}
void Im::MILGra::RectFill(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, double XEnd, double YEnd)
{
	MgraRectFill(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
}

void Im::MILGra::Text(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XStart, double YStart, CString StringPtr)
{
	// XStart , YStart : ¿ÞÂÊ »ó´Ü ÁÂÇ¥
	MgraText(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, StringPtr);
}
void Im::MILGra::Dot(PIL_ID GraphContId, PIL_ID DestImageBufIdOrGraListId, double XPos, double YPos)
{
	MgraDot(GraphContId, DestImageBufIdOrGraListId, XPos, YPos);
}

//cal
void Im::PILCal::Free(PIL_ID CalibrationId)
{
	if(CalibrationId == 0)
		return;
	cv::Mat *img = (cv::Mat*)CalibrationId;
	//delete img;
	g_pMManager->pem_delete(img, false);
}
void Im::PILCal::List(PIL_ID CalibrationId, const double *XPixArray, const double *YPixArray, const double *XWorldArray, const double *YWorldArray, const double *ZWorldArray, int64 NumPoint, int64 Operation, int64 ControlFlag)
{
	cv::Mat* calMat = (cv::Mat*)CalibrationId;

	int nSizeX = calMat->cols;
	int nSizeY = calMat->rows;
	double* srcPtr = (double *)calMat->ptr();

	double Pix_x=0, Pix_y=0;
	double World_x=0, World_y=0;
	double Avg_Pix_x=0, Avg_Pix_y=0;
	double Avg_World_x=0, Avg_World_y=0;

	for(int i=0; i<NumPoint; i++)
	{
// 		Pix_x += abs(XPixArray[i]);
// 		Pix_y += abs(YPixArray[i]);
// 		World_x += abs(XWorldArray[i]);
// 		World_y += abs(YWorldArray[i]);

		Pix_x += XPixArray[i];
		Pix_y += YPixArray[i];
		World_x += XWorldArray[i];
		World_y += YWorldArray[i];
	}
	Avg_Pix_x = Pix_x / NumPoint;
	Avg_Pix_y = Pix_y / NumPoint;
	Avg_World_x = World_x / NumPoint;
	Avg_World_y = World_y / NumPoint;

	Pix_x=0, Pix_y=0;
	World_x=0, World_y=0;
	for(int j=0; j<NumPoint; j++)
	{
		Pix_x += abs(Avg_Pix_x - XPixArray[j]);
		Pix_y += abs(Avg_Pix_y - YPixArray[j]);
		World_x += abs(Avg_World_x - XWorldArray[j]);
		World_y += abs(Avg_World_y - YWorldArray[j]);
	}

	double result_x = World_x / Pix_x;
	double result_y = World_y / Pix_y;

	srcPtr[0] = result_x;
	srcPtr[1] = result_y;
}
PIL_ID Im::PILCal::Alloc(PIL_ID SystemId, int64 Mode, int64 ModeFlag, PIL_ID *CalibrationIdPtr)
{
	cv::Mat *img = new cv::Mat(cv::Size(1, 2), CV_64FC1);
	g_pMManager->pem_new_check(img, (PCHAR)__FUNCTION__, __LINE__);
	img->setTo(cv::Scalar(0));
	PIL_ID id = (PIL_ID)img;
	if(CalibrationIdPtr != nullptr)
		*CalibrationIdPtr = id;
	return id;
}
int64 Im::PILCal::Inquire(PIL_ID CalibrationOrMilId, int64 InquireType, void *UserVarPtr)
{
	cv::Mat *calMat = (cv::Mat*)CalibrationOrMilId;

	int64 ret = 0;
	int64 type = InquireType;// & 0xFF;

	if(type == M_CALIBRATION_SUCCESSFUL)
	{
		double* _ptr = calMat->ptr<double>();
		if(_ptr[0] != 0 && _ptr[1] != 0)
		{
			ret = M_TRUE;
		}
		if(UserVarPtr != NULL)
		{
			int64* Var = (int64*)UserVarPtr;
			*Var = ret;
		}
	}
	else if(type == M_PIXEL_SIZE_X)
	{
		double* _ptr = calMat->ptr<double>();
		double var_temp;
		if(_ptr[0] != 0)
		{
			var_temp = _ptr[0];
		}
		if(UserVarPtr != NULL)
		{
			double* Var = (double*)UserVarPtr;
			*Var = var_temp;
		}
		ret = var_temp;
	}
	else if(type == M_PIXEL_SIZE_Y)
	{
		double* _ptr = calMat->ptr<double>();
		double var_temp;
		if(_ptr[1] != 0)
		{
			var_temp = _ptr[1];
		}
		if(UserVarPtr != NULL)
		{
			double* Var = (double*)UserVarPtr;
			*Var = var_temp;
		}
		ret = var_temp;
	}

	return ret;
}

void Im::MILCal::Free(PIL_ID CalibrationId)
{
	McalFree(CalibrationId);
}
void Im::MILCal::List(PIL_ID CalibrationId, const double *XPixArray, const double *YPixArray, const double *XWorldArray, const double *YWorldArray, const double *ZWorldArray, int64 NumPoint, int64 Operation, int64 ControlFlag)
{
	McalList(CalibrationId, XPixArray, YPixArray, XWorldArray, YWorldArray, ZWorldArray, NumPoint, Operation, ControlFlag);
}
PIL_ID Im::MILCal::Alloc(PIL_ID SystemId, int64 Mode, int64 ModeFlag, PIL_ID *CalibrationIdPtr)
{
	return McalAlloc(SystemId, Mode, ModeFlag, (MIL_ID*)CalibrationIdPtr);
}
int64 Im::MILCal::Inquire(PIL_ID CalibrationOrMilId, int64 InquireType, void *UserVarPtr)
{
	int64 nRet = 0;
	int64 * Var = (int64 *)UserVarPtr;
	nRet = McalInquire((MIL_ID)CalibrationOrMilId, InquireType, M_NULL);
	if(Var != M_NULL)
	{
		*Var = nRet;
	}
	return nRet;
}