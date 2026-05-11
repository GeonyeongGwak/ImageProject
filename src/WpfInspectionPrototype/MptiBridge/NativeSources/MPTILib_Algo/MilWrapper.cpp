#include "StdAfx.h"
#include "MilWrapper.h"

HINSTANCE g_hMilDLL = NULL;
HINSTANCE g_hMilblobDLL = NULL;
HINSTANCE g_hMilimDLL = NULL;
HINSTANCE g_hMilpatDLL = NULL;
HINSTANCE g_hMilcodeDLL = NULL;
HINSTANCE g_hMilstrDLL = NULL;
HINSTANCE g_hMilcalDLL = NULL;
BOOL g_bMil10 = FALSE;

typedef MIL_INT(*MsysInquire_Fun)(MIL_ID SystemId, MIL_INT InquireType, void *UserVarPtr);
typedef MIL_ID(*MsysAlloc_Fun)(MIL_CONST_TEXT_PTR SystemDescriptor, MIL_INT SystemNum, MIL_INT InitFlag, MIL_ID *SystemIdPtr);
typedef MIL_ID(*MsysAlloc10_Fun)(MIL_ID ContextAppId, MIL_CONST_TEXT_PTR SystemDescriptor, MIL_INT SystemNum, MIL_INT InitFlag, MIL_ID *SystemIdPtr);
typedef void(*MsysFree_Fun)(MIL_ID SystemId);

typedef void(*MappFree_Fun)(MIL_ID ApplicationId);
typedef MIL_INT(*MappInquire_Fun)(MIL_INT InquireType, void *UserVarPtr);
typedef MIL_INT(*MappInquire10_Fun)(MIL_ID ContextAppId, MIL_INT InquireType, void *UserVarPtr);
typedef MIL_ID(*MappAlloc_Fun)(MIL_INT InitFlag, MIL_ID *ApplicationIdPtr);
typedef MIL_ID(*MappAlloc10_Fun)(MIL_CONST_TEXT_PTR ServerDescription, MIL_INT InitFlag, MIL_ID *ApplicationIdPtr);

typedef void(*MdigHalt_Fun)(MIL_ID DigId);
typedef void(*MdigGrabContinuous_Fun)(MIL_ID DigId, MIL_ID DestImageBufId);
typedef void(*MdigFree_Fun)(MIL_ID DigId);
typedef MIL_ID(*MdigAlloc_Fun)(MIL_ID SystemId, MIL_INT DigNum, MIL_CONST_TEXT_PTR DataFormat, MIL_INT InitFlag, MIL_ID *DigIdPtr);
typedef MIL_ID(*MdigAlloc_Fun)(MIL_ID SystemId, MIL_INT DigNum, MIL_CONST_TEXT_PTR DataFormat, MIL_INT InitFlag, MIL_ID *DigIdPtr);
typedef MIL_INT(*MdigInquire_Fun)(MIL_ID DigId, MIL_INT64 InquireType, void *UserVarPtr);
typedef void(*MdigControl_Fun)(MIL_ID DigId, MIL_INT64 ControlType, MIL_DOUBLE ControlValue);
typedef void(*MdigHookFunction_Fun)(MIL_ID DigId, MIL_INT HookType, MIL_DIG_HOOK_FUNCTION_PTR HookHandlerPtr, void *UserDataPtr);
typedef void(*MdigProcess_Fun)(MIL_ID DigId, const MIL_ID *DestImageArrayPtr, MIL_INT ImageCount, MIL_INT Operation, MIL_INT OperationFlag, MIL_DIG_HOOK_FUNCTION_PTR HookHandlerPtr, void *UserDataPtr);
typedef void(*MdigGrab_Fun)(MIL_ID DigId, MIL_ID DestImageBufId);
typedef void(*MdigGrabWait_Fun)(MIL_ID DigId, MIL_INT ControlFlag);
typedef MIL_DOUBLE(*MappTimer_Fun)(MIL_INT ControlType, MIL_DOUBLE *TimePtr);
typedef MIL_DOUBLE(*MappTimer10_Fun)(MIL_ID ContextAppId, MIL_INT64 ControlType, MIL_DOUBLE *TimePtr);
typedef void(*MappControl_Fun)(MIL_INT ControlType, MIL_INT ControlValue);
typedef void(*MappControl10_Fun)(MIL_ID ContextAppId, MIL_INT ControlType, MIL_INT ControlValue);

typedef MIL_INT(*MbufInquire_Fun)(MIL_ID BufId, MIL_INT InquireType, void *UserVarPtr);
typedef MIL_ID(*MbufAlloc2d_Fun)(MIL_ID SystemId, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT64 Attribute, MIL_ID *BufIdPtr);
typedef void(*MbufCopy_Fun)(MIL_ID SrcBufId, MIL_ID DestBufId);
typedef void(*MbufExport_Fun)(MIL_CONST_TEXT_PTR Filename, MIL_INT FileFormat, MIL_ID SrcBufId);
typedef void(*MbufFree_Fun)(MIL_ID BufId);
typedef void(*MbufClear_Fun)(MIL_ID DestImageBufId, MIL_DOUBLE Color);
typedef void(*MbufPut2d_Fun)(MIL_ID DestBufId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, const void *UserArrayPtr);
typedef void(*MbufGet2d_Fun)(MIL_ID SrcBufId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, void *UserArrayPtr);
typedef MIL_ID(*MbufRestore_Fun)(MIL_CONST_TEXT_PTR Filename, MIL_ID SystemId, MIL_ID *BufIdPtr);
typedef void(*MbufPut_Fun)(MIL_ID DestBufId, const void *UserArrayPtr);
typedef MIL_ID(*MbufImport_Fun)(MIL_TEXT_PTR Filename, MIL_INT FileFormat, MIL_INT Operation, MIL_ID SystemId, MIL_ID *BufIdPtr);
typedef MIL_ID(*MbufChild2d_Fun)(MIL_ID ParentBufId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, MIL_ID *BufIdPtr);
typedef void(*MbufGet_Fun		)(MIL_ID SrcBufId, void *UserArrayPtr);
typedef void(*MbufGetColor_Fun	)(MIL_ID SrcBufId, MIL_INT DataFormat, MIL_INT Band, void *UserArrayPtr);
typedef void(*MbufCopyColor_Fun	)(MIL_ID SrcBufId, MIL_ID DestBufId, MIL_INT Band); 
typedef void(*MbufCopyClip_Fun	)(MIL_ID SrcBufId, MIL_ID DestBufId, MIL_INT DestOffX, MIL_INT DestOffY);
typedef void(*MbufLoad_Fun		)(MIL_CONST_TEXT_PTR Filename, MIL_ID BufId);
typedef void(*MbufSave_Fun		)(MIL_CONST_TEXT_PTR FileName, MIL_ID BufId);
typedef void(*MbufPutColor_Fun	)(MIL_ID DestBufId, MIL_INT DataFormat, MIL_INT Band, const void *UserArrayPtr) ;
typedef void(*MbufPutColor2d_Fun)(MIL_ID DestBufId, MIL_INT DataFormat, MIL_INT Band, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, const void *UserArrayPtr);
typedef MIL_ID(*MbufCreate2d_Fun	)(MIL_ID SystemId, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT64 Attribute, MIL_INT64 ControlFlag, MIL_INT Pitch, void *DataPtr, MIL_ID *BufIdPtr);
typedef MIL_ID(*MbufCreateColor_Fun	)(MIL_ID SystemId, MIL_INT SizeBand, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT64 Attribute, MIL_INT64 ControlFlag, MIL_INT Pitch, void **ArrayOfDataPtr, MIL_ID *BufIdPtr);
typedef MIL_ID(*MbufAllocColor_Fun	)(MIL_ID SystemId, MIL_INT SizeBand, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT64 Attribute, MIL_ID *BufIdPtr);
typedef void(*MbufGetColor2d_Fun	)(MIL_ID SrcBufId, MIL_INT DataFormat, MIL_INT Band, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, void *UserArrayPtr);

typedef void(*MgraColor_Fun)(MIL_ID GraphContId, MIL_DOUBLE ForegroundColor);
typedef void(*MgraRect_Fun)(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XStart, MIL_DOUBLE YStart, MIL_DOUBLE XEnd, MIL_DOUBLE YEnd);
typedef MIL_ID(*MgraAlloc_Fun)(MIL_ID SystemId, MIL_ID *GraphContIdPtr);
typedef void(*MgraFree_Fun)(MIL_ID ObjectId);
typedef void(*MgraRectFill_Fun	)(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XStart, MIL_DOUBLE YStart, MIL_DOUBLE XEnd, MIL_DOUBLE YEnd);
typedef void(*MgraDot_Fun		)(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XPos, MIL_DOUBLE YPos);
typedef void(*MgraLine_Fun		)(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XStart, MIL_DOUBLE YStart, MIL_DOUBLE XEnd, MIL_DOUBLE YEnd);
typedef void(*MgraLines_Fun		)(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_INT NumberOfLinesOrVertices, const MIL_DOUBLE *XStartArray, const MIL_DOUBLE *YStartArray, const MIL_DOUBLE *XEndArray, const MIL_DOUBLE *YEndArray, MIL_INT ControlFlag);
typedef void(*MgraText_Fun		)(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XStart, MIL_DOUBLE YStart, MIL_CONST_TEXT_PTR StringPtr);

typedef void(*MblobGetResultSingle_Fun)(MIL_ID BlobResId, MIL_INT LabelVal, MIL_INT Feature, void *TargetVarPtr);
typedef void(*MblobGetResult_Fun)(MIL_ID BlobResId, MIL_INT Feature, void *TargetArrayPtr);
typedef void(*MblobSelect_Fun)(MIL_ID BlobResId, MIL_INT Operation, MIL_INT SelectionCriterion, MIL_INT Condition, MIL_DOUBLE CondLow, MIL_DOUBLE CondHigh);
typedef void(*MblobFill_Fun)(MIL_ID BlobResId, MIL_ID DestImageBufId, MIL_INT Criterion, MIL_INT Value);
typedef MIL_INT(*MblobGetNumber_Fun)(MIL_ID BlobResId, MIL_INT *CountVarPtr);
typedef void(*MblobCalculate_Fun)(MIL_ID BlobIdentImageId, MIL_ID GrayImageId, MIL_ID FeatureListId, MIL_ID BlobResId);
typedef void(*MblobReconstruct_Fun)(MIL_ID SrcImageBufId, MIL_ID SeedImageBufId, MIL_ID DestImageBufId, MIL_INT Operation, MIL_INT ProcMode);
typedef MIL_INT(*MblobGetLabel_Fun)(MIL_ID BlobResId, MIL_INT XPos, MIL_INT YPos, MIL_INT *LabelVarPtr);
typedef MIL_ID(*MblobAllocFeatureList_Fun)(MIL_ID SystemId, MIL_ID *FeatureListIdPtr);
typedef MIL_ID(*MblobAllocResult_Fun)(MIL_ID SystemId, MIL_ID *BlobResIdPtr);
typedef void(*MblobSelectFeature_Fun)(MIL_ID FeatureListId, MIL_INT Feature);
typedef void(*MblobFree_Fun)(MIL_ID BlobId);
typedef void(*MblobDraw_Fun)(MIL_ID GraphContId, MIL_ID ResultId, MIL_ID DestImageId, MIL_INT Operation, MIL_INT Label, MIL_INT ControlFlag);
typedef void(*MblobLabel_Fun)(MIL_ID BlobResId, MIL_ID DestImageBufId, MIL_INT Mode);
typedef void(*MblobControl_Fun)(MIL_ID BlobResId, MIL_INT ControlType, MIL_DOUBLE ControlValue);

typedef void(*MimDilate_Fun)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT NbIteration, MIL_INT ProcMode);
typedef void(*MimErode_Fun)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT NbIteration, MIL_INT ProcMode);
typedef void(*MimCountDifference_Fun)(MIL_ID Src1ImageBufId, MIL_ID Src2ImageBufId, MIL_ID ImResultId);
typedef void(*MimResize_Fun)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_DOUBLE ScaleFactorX, MIL_DOUBLE ScaleFactorY, MIL_INT InterpolationMode);
typedef MIL_INT(*MimBinarize_Fun)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT ConditionAndThreshMode, MIL_DOUBLE LowParam, MIL_DOUBLE HighParam);
typedef MIL_ID(*MimAllocResult_Fun)(MIL_ID SystemId, MIL_INT NbEntries, MIL_INT64 ResultType, MIL_ID *ImResultIdPtr);
typedef void (*MimFree_Fun)(MIL_ID ImResultId);
typedef MIL_INT(*MimInquire_Fun)(MIL_ID ContextorResultId, MIL_INT InquireType, void *UserVarPtr);
typedef void(*MimGetResult_Fun)(MIL_ID ImResultId, MIL_INT ResultType, void *UserArrayPtr);
typedef void(*MimClose_Fun)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT NbIteration, MIL_INT ProcMode);
typedef void(*MimOpen_Fun)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT NbIteration, MIL_INT ProcMode);
typedef void(*MimArith_Fun)(MIL_DOUBLE Src1ImageBufIdOrConst, MIL_DOUBLE Src2ImageBufIdOrConst, MIL_ID DestImageBufId, MIL_INT Operation);
typedef void(*MimHistogramEqualize_Fun)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT Method, MIL_DOUBLE Alpha, MIL_DOUBLE Min, MIL_DOUBLE Max);
typedef void(*MimRotate_Fun		)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_DOUBLE Angle, MIL_DOUBLE SrcCenX, MIL_DOUBLE SrcCenY, MIL_DOUBLE DstCenX, MIL_DOUBLE DstCenY, MIL_INT InterpolationMode);
typedef void(*MimMorphic_Fun	)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_ID StructElemBufId, MIL_INT Operation, MIL_INT NbIterationOrArea, MIL_INT ProcMode);
typedef void(*MimFlip_Fun		)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT Operation, MIL_INT OpFlag); 
typedef void(*MimRank_Fun		)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_ID StructElemBufId, MIL_INT Rank, MIL_INT ProcMode);
typedef void(*MimTranslate_Fun	)(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_DOUBLE XDisplacement, MIL_DOUBLE YDisplacement, MIL_INT InterpolationMode);
typedef void(*MimEdgeDetect_Fun	)(MIL_ID SrcImageBufId, MIL_ID DestIntensityImageBufId, MIL_ID DestAngleImageBufId, MIL_ID KernelId, MIL_INT ControlFlag, MIL_INT Threshold);
typedef void(*MimHistogram_Fun	)(MIL_ID SrcImageBufId, MIL_ID HistImResultId);
typedef void(*MimProject_Fun	)(MIL_ID SrcImageBufId, MIL_ID ProjImResultId, MIL_DOUBLE ProjAngle);
typedef void(*MimStat_Fun		)(MIL_ID SrcImageId, MIL_ID StatResultId, MIL_INT StatType, MIL_INT Condition, MIL_DOUBLE CondLow, MIL_DOUBLE CondHigh);
typedef void(*MimTransform_Fun	)(MIL_ID SrcImageRBufId, MIL_ID SrcImageIBufId, MIL_ID DestImageRBufId, MIL_ID DestImageIBufId, MIL_INT TransformType, MIL_INT ControlFlag);


typedef void(*MpatGetResult_Fun)(MIL_ID PatResultId, MIL_INT ResultType, void *UserArrayPtr);
typedef MIL_INT(*MpatGetNumber_Fun)(MIL_ID PatResultId, MIL_INT *CountPtr);
typedef void(*MpatFindModel_Fun)(MIL_ID ImageBufId, MIL_ID ModelId, MIL_ID PatResultId);
typedef MIL_ID(*MpatAllocResult_Fun)(MIL_ID SystemId, MIL_INT NbEntries, MIL_ID *PatResultIdPtr);
typedef MIL_ID(*MpatAllocModel_Fun)(MIL_ID SystemId, MIL_ID SrcImageBufId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, MIL_INT ModelType, MIL_ID *ModelIdPtr);
typedef void(*MpatSetAngle_Fun)(MIL_ID ModelId, MIL_INT ControlType, MIL_DOUBLE ControlValue);
typedef void(*MpatSetAccuracy_Fun)(MIL_ID ModelId, MIL_INT Accuracy);
typedef void(*MpatSetAcceptance_Fun)(MIL_ID ModelId, MIL_DOUBLE AcceptanceThreshold);
typedef void(*MpatPreprocModel_Fun)(MIL_ID TypicalImageBufId, MIL_ID ModelId, MIL_INT Mode);
typedef void(*MpatFree_Fun)(MIL_ID PatId);
typedef void(*MpatSetSpeed_Fun)(MIL_ID ModelId, MIL_INT SpeedFactor);
typedef MIL_ID(*MpatAllocRotatedModel_Fun)(MIL_ID SystemId, MIL_ID SrcModelId, MIL_DOUBLE Angle, MIL_INT InterpolationMode, MIL_INT ModelType, MIL_ID *NewModelIdPtr);
typedef MIL_INT(*MpatInquire_Fun)(MIL_ID PatId, MIL_INT InquireType, void *UserVarPtr); 
typedef void(*MpatSetPosition_Fun)(MIL_ID ModelId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY) ;
typedef void(*MpatSave_Fun)(MIL_CONST_TEXT_PTR FileName, MIL_ID ModelId);
typedef void(*MpatCopy_Fun)(MIL_ID ModelId, MIL_ID DestImageBufId, MIL_INT CopyMode);
typedef MIL_ID(*MpatRestore_Fun)(MIL_ID SystemId, MIL_CONST_TEXT_PTR Filename, MIL_ID *ModelIdPtr);

typedef void(*McodeRead_Fun)(MIL_ID CodeContextId, MIL_ID ImageBufId, MIL_ID CodeResultId);
typedef void(*McodeModel_Fun)(MIL_ID CodeContextId, MIL_INT Operation, MIL_INT CodeType, MIL_INT Instance, MIL_INT ControlFlag, MIL_ID *CodeModelIdPtr);
typedef void(*McodeGetResult_Fun)(MIL_ID CodeResultId, MIL_INT ResultType, void *ResultPtr);
typedef void(*McodeFree_Fun)(MIL_ID ObjectId);
typedef void(*McodeControl_Fun)(MIL_ID CodeId, MIL_INT ControlType, MIL_DOUBLE ControlValue);
typedef void(*McodeAllocResult_Fun)(MIL_ID SystemId, MIL_INT ControlFlag, MIL_ID *CodeResultIdPtr);
typedef void(*McodeAlloc_Fun)(MIL_ID SystemId, MIL_INT ContextType, MIL_INT ControlFlag, MIL_ID *CodeContextIdPtr);

typedef void(*MstrFree_Fun			)(MIL_ID ObjectId);
typedef void(*MstrControl_Fun		)(MIL_ID ContextId, MIL_INT Index, MIL_INT ControlType, MIL_DOUBLE ControlValue);
typedef MIL_ID(*MstrAlloc_Fun		)(MIL_ID SystemId, MIL_INT ContextType, MIL_INT ControlFlag, MIL_ID *ObjectIdPtr);
typedef void(*MstrEditFont_Fun		)(MIL_ID ContextId, MIL_INT FontIndex, MIL_INT Operation, MIL_INT OperationMode, MIL_INT Param1, const void *Param2, const void *Param3);
typedef MIL_INT(*MstrInquire_Fun	)(MIL_ID ContextId, MIL_INT Index, MIL_INT InquireType, void *UserVarPtr);
typedef void(*MstrSave_Fun			)(MIL_CONST_TEXT_PTR FileName, MIL_ID ContextId, MIL_INT ControlFlag);
typedef MIL_ID(*MstrRestore_Fun		)(MIL_CONST_TEXT_PTR Filename, MIL_ID SystemId, MIL_INT ControlFlag, MIL_ID *ContextIdPtr);
typedef void(*MstrSetConstraint_Fun	)(MIL_ID ContextId, MIL_INT StringIndex, MIL_INT CharPos, MIL_INT ConstraintType, const void *CharList);
typedef void(*MstrPreprocess_Fun	)(MIL_ID ContextId, MIL_INT ControlFlag); 
typedef void(*MstrRead_Fun			)(MIL_ID ContextId, MIL_ID TargetImageId, MIL_ID ResultId);
typedef void(*MstrGetResult_Fun		)(MIL_ID ResultId, MIL_INT Index, MIL_INT ResultType, void *ResultArrayPtr); 
typedef void(*MstrDraw_Fun			)(MIL_ID GraphContId, MIL_ID ContextOrResultId, MIL_ID DestImageId, MIL_INT Operation, MIL_INT Index, const void *CharList, MIL_INT ControlFlag);
typedef MIL_ID(*MstrAllocResult_Fun	)(MIL_ID SystemId, MIL_INT ControlFlag, MIL_ID *ObjectIdPtr);

typedef void(*McalFree_Fun			)(MIL_ID CalibrationId);
typedef void(*McalList_Fun			)(MIL_ID CalibrationId, const MIL_DOUBLE *XPixArray, const MIL_DOUBLE *YPixArray, const MIL_DOUBLE *XWorldArray, const MIL_DOUBLE *YWorldArray, const MIL_DOUBLE *ZWorldArray, MIL_INT NumPoint, MIL_INT Operation, MIL_INT ControlFlag);
typedef MIL_ID(*McalAlloc_Fun		)(MIL_ID SystemId, MIL_INT Mode, MIL_INT ModeFlag, MIL_ID *CalibrationIdPtr);
typedef MIL_INT(*McalInquire_Fun	)(MIL_ID CalibrationOrMilId, MIL_INT InquireType, void *UserVarPtr);
typedef void(*MdigControlFeature_Fun)(MIL_ID DigId, MIL_INT64 ControlType, MIL_CONST_TEXTW_PTR FeatureName, MIL_INT64 UserVarType, const void* UserVarPtr);
typedef void(*MdigInquireFeature_Fun)(MIL_ID DigId, MIL_INT64 InquireType, MIL_CONST_TEXTW_PTR FeatureName, MIL_INT64 UserVarType, void* UserVarPtr);
typedef MIL_ID(*MdigGetHookInfo_Fun )(MIL_ID EventId, MIL_INT64 InfoType, void *UserVarPtr);
typedef MIL_ID(*MthrAlloc_Fun) (MIL_ID SystemId, MIL_INT64 ObjectType, MIL_INT64 ControlFlag, MIL_THREAD_FUNCTION_PTR ThreadFctPtr, void *UserDataPtr, MIL_ID *ThreadEventOrMutexIdPtr);
typedef void(*MthrControl_Fun) (MIL_ID ThreadEventOrMutexId, MIL_INT64 ControlType, MIL_INT64 ControlValue);
typedef void(*MthrFree_Fun) (MIL_ID ThreadEventorMutexId);
typedef MIL_INT(*MthrWait_Fun)(MIL_ID ThreadOrEventId, MIL_INT64 WaitOption, MIL_INT *StatePtr);
typedef void(*MsysHookFunction_Fun)(MIL_ID SysId, MIL_INT HookType, MIL_SYS_HOOK_FUNCTION_PTR HookHandlerPtr, void *UserDataPtr);

//mil
MsysInquire_Fun	MsysInquire_ptr;
MsysAlloc_Fun	MsysAlloc_ptr;
MsysAlloc10_Fun	MsysAlloc10_ptr;
MsysFree_Fun	MsysFree_ptr;

MappInquire_Fun MappInquire_ptr;
MappInquire10_Fun MappInquire10_ptr;
MappAlloc_Fun MappAlloc_ptr;
MappAlloc10_Fun MappAlloc10_ptr;
MappControl_Fun			MappControl_ptr;
MappControl10_Fun			MappControl10_ptr;
MappTimer_Fun		MappTimer_ptr;
MappTimer10_Fun		MappTimer10_ptr;
MappFree_Fun			MappFree_ptr;

MdigControl_Fun	MdigControl_ptr;
MdigHookFunction_Fun	MdigHookFunction_ptr;
MdigProcess_Fun	MdigProcess_ptr;
MdigGrab_Fun		MdigGrab_ptr;
MdigGrabWait_Fun	MdigGrabWait_ptr;
MdigAlloc_Fun			MdigAlloc_ptr;
MdigInquire_Fun			MdigInquire_ptr;
MdigHalt_Fun			MdigHalt_ptr;
MdigGrabContinuous_Fun	MdigGrabContinuous_ptr;
MdigFree_Fun			MdigFree_ptr;

MbufInquire_Fun MbufInquire_ptr;
MbufAlloc2d_Fun	MbufAlloc2d_ptr;
MbufCopy_Fun	MbufCopy_ptr;
MbufExport_Fun	MbufExport_ptr;
MbufFree_Fun	MbufFree_ptr;
MbufClear_Fun	MbufClear_ptr;
MbufPut2d_Fun	MbufPut2d_ptr;
MbufGet2d_Fun	MbufGet2d_ptr;
MbufRestore_Fun	MbufRestore_ptr;
MbufPut_Fun		MbufPut_ptr;
MbufImport_Fun	MbufImport_ptr;
MbufChild2d_Fun	MbufChild2d_ptr;
MbufGet_Fun				MbufGet_ptr;
MbufGetColor_Fun		MbufGetColor_ptr;
MbufCopyColor_Fun		MbufCopyColor_ptr;
MbufCopyClip_Fun		MbufCopyClip_ptr;
MbufCreate2d_Fun		MbufCreate2d_ptr;
MbufLoad_Fun			MbufLoad_ptr;
MbufSave_Fun			MbufSave_ptr;
MbufCreateColor_Fun		MbufCreateColor_ptr;
MbufAllocColor_Fun		MbufAllocColor_ptr;
MbufPutColor_Fun		MbufPutColor_ptr;
MbufPutColor2d_Fun		MbufPutColor2d_ptr;
MbufGetColor2d_Fun		MbufGetColor2d_ptr;

MgraAlloc_Fun			MgraAlloc_ptr;
MgraFree_Fun			MgraFree_ptr;
MgraColor_Fun		MgraColor_ptr;
MgraRect_Fun		MgraRect_ptr;
MgraRectFill_Fun	MgraRectFill_ptr;
MgraDot_Fun			MgraDot_ptr;
MgraLine_Fun		MgraLine_ptr;
MgraLines_Fun		MgraLines_ptr;
MgraText_Fun		MgraText_ptr;
//blob
MblobGetResult_Fun MblobGetResult_ptr;
MblobSelect_Fun MblobSelect_ptr;
MblobFill_Fun MblobFill_ptr;
MblobGetNumber_Fun MblobGetNumber_ptr;
MblobCalculate_Fun MblobCalculate_ptr;
MblobReconstruct_Fun MblobReconstruct_ptr;
MblobGetLabel_Fun MblobGetLabel_ptr;
MblobAllocFeatureList_Fun MblobAllocFeatureList_ptr;
MblobAllocResult_Fun MblobAllocResult_ptr;
MblobSelectFeature_Fun MblobSelectFeature_ptr;
MblobFree_Fun MblobFree_ptr;
MblobDraw_Fun MblobDraw_ptr;
MblobLabel_Fun MblobLabel_ptr;
MblobGetResultSingle_Fun MblobGetResultSingle_ptr;
MblobControl_Fun	MblobControl_ptr;
//im
MimDilate_Fun MimDilate_ptr;
MimErode_Fun MimErode_ptr;
MimCountDifference_Fun MimCountDifference_ptr;
MimResize_Fun MimResize_ptr;
MimBinarize_Fun MimBinarize_ptr;
MimAllocResult_Fun MimAllocResult_ptr;
MimFree_Fun MimFree_ptr;
MimInquire_Fun MimInquire_ptr;
MimGetResult_Fun MimGetResult_ptr;
MimClose_Fun MimClose_ptr;
MimOpen_Fun MimOpen_ptr;
MimArith_Fun MimArith_ptr;
MimHistogramEqualize_Fun	MimHistogramEqualize_ptr;	
MimRotate_Fun				MimRotate_ptr;
MimMorphic_Fun				MimMorphic_ptr;
MimFlip_Fun					MimFlip_ptr;
MimRank_Fun					MimRank_ptr;
MimTranslate_Fun			MimTranslate_ptr;
MimEdgeDetect_Fun			MimEdgeDetect_ptr;
MimHistogram_Fun			MimHistogram_ptr;
MimProject_Fun				MimProject_ptr;
MimStat_Fun					MimStat_ptr;
MimTransform_Fun			MimTransform_ptr;
//pat
MpatGetResult_Fun MpatGetResult_ptr;
MpatGetNumber_Fun MpatGetNumber_ptr;
MpatFindModel_Fun MpatFindModel_ptr;
MpatAllocResult_Fun MpatAllocResult_ptr;
MpatAllocModel_Fun MpatAllocModel_ptr;
MpatSetAngle_Fun MpatSetAngle_ptr;
MpatSetAccuracy_Fun		MpatSetAccuracy_ptr;
MpatSetAcceptance_Fun	MpatSetAcceptance_ptr;
MpatPreprocModel_Fun	MpatPreprocModel_ptr;
MpatFree_Fun			MpatFree_ptr;
MpatSetSpeed_Fun		MpatSetSpeed_ptr;
MpatAllocRotatedModel_Fun	MpatAllocRotatedModel_ptr;
MpatInquire_Fun			MpatInquire_ptr;
MpatSetPosition_Fun		MpatSetPosition_ptr;
MpatSave_Fun			MpatSave_ptr;
MpatCopy_Fun			MpatCopy_ptr;
MpatRestore_Fun			MpatRestore_ptr;
//code
McodeRead_Fun			McodeRead_ptr;
McodeModel_Fun			McodeModel_ptr;
McodeGetResult_Fun		McodeGetResult_ptr;
McodeFree_Fun			McodeFree_ptr;
McodeControl_Fun		McodeControl_ptr;
McodeAllocResult_Fun	McodeAllocResult_ptr;
McodeAlloc_Fun			McodeAlloc_ptr;
//str
MstrFree_Fun			MstrFree_ptr;
MstrControl_Fun			MstrControl_ptr;
MstrAlloc_Fun			MstrAlloc_ptr;
MstrEditFont_Fun		MstrEditFont_ptr;
MstrInquire_Fun			MstrInquire_ptr;
MstrSave_Fun			MstrSave_ptr;
MstrRestore_Fun			MstrRestore_ptr;
MstrSetConstraint_Fun	MstrSetConstraint_ptr;
MstrPreprocess_Fun		MstrPreprocess_ptr;
MstrRead_Fun			MstrRead_ptr;
MstrGetResult_Fun		MstrGetResult_ptr;
MstrDraw_Fun			MstrDraw_ptr;
MstrAllocResult_Fun		MstrAllocResult_ptr;

McalFree_Fun			McalFree_ptr;
McalAlloc_Fun			McalAlloc_ptr;
McalList_Fun			McalList_ptr;
McalInquire_Fun			McalInquire_ptr;
MdigControlFeature_Fun		MdigControlFeature_ptr;
MdigInquireFeature_Fun		MdigInquireFeature_ptr;
MdigGetHookInfo_Fun			MdigGetHookInfo_ptr;
MthrAlloc_Fun			MthrAlloc_ptr;
MthrControl_Fun			MthrControl_ptr;
MthrFree_Fun			MthrFree_ptr;
MthrWait_Fun			MthrWait_ptr;
MsysHookFunction_Fun		MsysHookFunction_ptr;
#define WIN_MAJOR_MAX	    10    // Windows 10
#define WIN_MAJOR_MIN	    5    // Windows 2000 or XP

BOOL CheckWindowVersion_WIN10()
{
	DWORD dwTypeMask;
	DWORDLONG dwlConditionMask = 0;
	OSVERSIONINFOEX ovi;
	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	BOOL bRet = FALSE;
	for (int mjr = WIN_MAJOR_MAX; mjr >= WIN_MAJOR_MIN; mjr--)
	{
		for (int mnr = 0; mnr <= 3; mnr++)
		{
			memset(&ovi, 0, sizeof(OSVERSIONINFOEX));
			ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			ovi.dwMajorVersion = mjr;
			ovi.dwMinorVersion = mnr;
			dwlConditionMask = 0;

			VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
			VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_EQUAL);

			dwTypeMask = VER_MAJORVERSION | VER_MINORVERSION;
			if (TRUE == VerifyVersionInfo(&ovi, dwTypeMask, dwlConditionMask)) {
				dwMajor = mjr;
				dwMinor = mnr;
				bRet = TRUE;
			}
		}
	}
	BOOL bRetWIN10 = FALSE;
	if (bRet != FALSE)
	{
		switch (dwMajor)
		{
		case 10:	// Windows 10
			bRetWIN10 = TRUE;
			break;
		case 6:	// win8
			if (dwMinor == 3 /*Windows 8.1*/ || dwMinor == 2/*Windows 8.1*/)
			{
				bRetWIN10 = TRUE;
			}
			break;
		}
	}
	return bRetWIN10;
}

BOOL LoadMilDll()
{
	if (g_hMilDLL != NULL)
		return TRUE;

	CString strDll, strMsg;
	strDll.Format(_T("Mil.dll"));

	g_hMilDLL = LoadLibrary(strDll);
	if (g_hMilDLL == NULL)
	{
		strMsg.Format(_T("fail load %s"), strDll);
		AfxMessageBox(strMsg);
		return FALSE;
	}
	g_bMil10 = CheckWindowVersion_WIN10();
	if (g_bMil10 == TRUE)
	{
		VERIFY(MsysAlloc10_ptr = (MsysAlloc10_Fun)GetProcAddress(g_hMilDLL, "MsysAllocW"));								//MsysAlloc_Fun	MsysAlloc_ptr;

		VERIFY(MappInquire10_ptr = (MappInquire10_Fun)GetProcAddress(g_hMilDLL, "MappInquire"));						//MappInquire_Fun MappInquire_ptr;
		VERIFY(MappAlloc10_ptr = (MappAlloc10_Fun)GetProcAddress(g_hMilDLL, "MappAllocW"));								//MappAlloc_Fun MappAlloc_ptr;
		VERIFY(MappTimer10_ptr = (MappTimer10_Fun)GetProcAddress(g_hMilDLL, "MappTimer"));								//MappTimer_Fun		MappTimer_ptr;
		VERIFY(MappControl10_ptr = (MappControl10_Fun)GetProcAddress(g_hMilDLL, "MappControl"));						//MappControl_Fun			MappControl_ptr;
		VERIFY(MbufClear_ptr = (MbufClear_Fun)GetProcAddress(g_hMilDLL, "MbufClearDouble"));								//MbufClear_Fun	MbufClear_ptr;
		VERIFY(MgraColor_ptr = (MgraColor_Fun)GetProcAddress(g_hMilDLL, "MgraColorDouble"));								//MgraColor_Fun	MgraColor_ptr;		
	}
	else
	{
		VERIFY(MsysAlloc_ptr = (MsysAlloc_Fun)GetProcAddress(g_hMilDLL, "MsysAllocW"));								//MsysAlloc_Fun	MsysAlloc_ptr;
		VERIFY(MappInquire_ptr = (MappInquire_Fun)GetProcAddress(g_hMilDLL, "MappInquire"));						//MappInquire_Fun MappInquire_ptr;
		VERIFY(MappAlloc_ptr = (MappAlloc_Fun)GetProcAddress(g_hMilDLL, "MappAllocW"));								//MappAlloc_Fun MappAlloc_ptr;
		VERIFY(MappControl_ptr = (MappControl_Fun)GetProcAddress(g_hMilDLL, "MappControl"));						//MappControl_Fun			MappControl_ptr;
		VERIFY(MappTimer_ptr = (MappTimer_Fun)GetProcAddress(g_hMilDLL, "MappTimer"));								//MappTimer_Fun		MappTimer_ptr;		
		VERIFY(MbufClear_ptr = (MbufClear_Fun)GetProcAddress(g_hMilDLL, "MbufClear"));								//MbufClear_Fun	MbufClear_ptr;
		VERIFY(MgraColor_ptr = (MgraColor_Fun)GetProcAddress(g_hMilDLL, "MgraColor"));								//MgraColor_Fun	MgraColor_ptr;
		
	}
	VERIFY(MdigControlFeature_ptr = (MdigControlFeature_Fun)GetProcAddress(g_hMilDLL, "MdigControlFeatureW"));
	VERIFY(MdigInquireFeature_ptr = (MdigInquireFeature_Fun)GetProcAddress(g_hMilDLL, "MdigInquireFeatureW"));
	VERIFY(MdigGetHookInfo_ptr = (MdigGetHookInfo_Fun)GetProcAddress(g_hMilDLL, "MdigGetHookInfo"));
	VERIFY(MthrAlloc_ptr = (MthrAlloc_Fun)GetProcAddress(g_hMilDLL, "MthrAlloc"));
	VERIFY(MthrControl_ptr = (MthrControl_Fun)GetProcAddress(g_hMilDLL, "MthrControlInt64"));
	VERIFY(MthrFree_ptr = (MthrFree_Fun)GetProcAddress(g_hMilDLL, "MthrFree"));
	VERIFY(MthrWait_ptr = (MthrWait_Fun)GetProcAddress(g_hMilDLL, "MthrWait"));
	VERIFY(MsysHookFunction_ptr = (MsysHookFunction_Fun)GetProcAddress(g_hMilDLL, "MsysHookFunction"));
	VERIFY(MsysInquire_ptr = (MsysInquire_Fun)GetProcAddress(g_hMilDLL, "MsysInquire"));						//MsysInquire_Fun	MsysInquire_ptr;
	VERIFY(MsysFree_ptr = (MsysFree_Fun)GetProcAddress(g_hMilDLL, "MsysFree"));									//MsysFree_Fun	MsysFree_ptr;

	VERIFY(MappFree_ptr = (MappFree_Fun)GetProcAddress(g_hMilDLL, "MappFree"));									//MdigFree_Fun			MdigFree_ptr;
	
	VERIFY(MdigControl_ptr = (MdigControl_Fun)GetProcAddress(g_hMilDLL, "MdigControlDouble"));					//MdigControl_Fun	MdigControl_ptr;
	VERIFY(MdigHookFunction_ptr = (MdigHookFunction_Fun)GetProcAddress(g_hMilDLL, "MdigHookFunction"));			//MdigHookFunction_Fun	MdigHookFunction_ptr;
	VERIFY(MdigProcess_ptr = (MdigProcess_Fun)GetProcAddress(g_hMilDLL, "MdigProcess"));						//MdigProcess_Fun	MdigProcess_ptr;
	VERIFY(MdigGrab_ptr = (MdigGrab_Fun)GetProcAddress(g_hMilDLL, "MdigGrab"));								//MdigGrab_Fun		MdigGrab_ptr;
	VERIFY(MdigGrabWait_ptr = (MdigGrabWait_Fun)GetProcAddress(g_hMilDLL, "MdigGrabWait"));						//MdigGrabWait_Fun	MdigGrabWait_ptr;
	VERIFY(MdigAlloc_ptr = (MdigAlloc_Fun)GetProcAddress(g_hMilDLL, "MdigAllocW"));								//MdigAlloc_Fun			MdigAlloc_ptr;
	VERIFY(MdigInquire_ptr = (MdigInquire_Fun)GetProcAddress(g_hMilDLL, "MdigInquire"));						//MdigInquire_Fun			MdigInquire_ptr;
	VERIFY(MdigHalt_ptr = (MdigHalt_Fun)GetProcAddress(g_hMilDLL, "MdigHalt"));									//MdigHalt_Fun			MdigHalt_ptr;
	VERIFY(MdigGrabContinuous_ptr = (MdigGrabContinuous_Fun)GetProcAddress(g_hMilDLL, "MdigGrabContinuous"));	//MdigGrabContinuous_Fun	MdigGrabContinuous_ptr;
	VERIFY(MdigFree_ptr = (MdigFree_Fun)GetProcAddress(g_hMilDLL, "MdigFree"));									//MappFree_Fun			MappFree_ptr;
	
	VERIFY(MbufInquire_ptr = (MbufInquire_Fun)GetProcAddress(g_hMilDLL, "MbufInquire"));						//MbufInquire_Fun MbufInquire_ptr;
	VERIFY(MbufAlloc2d_ptr = (MbufAlloc2d_Fun)GetProcAddress(g_hMilDLL, "MbufAlloc2d"));						//MbufAlloc2d_Fun	MbufAlloc2d_ptr;
	VERIFY(MbufCopy_ptr = (MbufCopy_Fun)GetProcAddress(g_hMilDLL, "MbufCopy"));									//MbufCopy_Fun	MbufCopy_ptr;
	VERIFY(MbufExport_ptr = (MbufExport_Fun)GetProcAddress(g_hMilDLL, "MbufExportW"));							//MbufExport_Fun	MbufExport_ptr;
	VERIFY(MbufFree_ptr = (MbufFree_Fun)GetProcAddress(g_hMilDLL, "MbufFree"));									//MbufFree_Fun	MbufFree_ptr;
	VERIFY(MbufPut2d_ptr = (MbufPut2d_Fun)GetProcAddress(g_hMilDLL, "MbufPut2d"));								//MbufPut2d_Fun	MbufPut2d_ptr;
	VERIFY(MbufGet2d_ptr = (MbufGet2d_Fun)GetProcAddress(g_hMilDLL, "MbufGet2d"));								//MbufGet2d_Fun	MbufGet2d_ptr;
	VERIFY(MbufRestore_ptr = (MbufRestore_Fun)GetProcAddress(g_hMilDLL, "MbufRestoreW"));						//MbufRestore_Fun	MbufRestore_ptr;
	VERIFY(MbufPut_ptr = (MbufPut_Fun)GetProcAddress(g_hMilDLL, "MbufPut"));									//MbufPut_Fun		MbufPut_ptr;
	VERIFY(MbufImport_ptr = (MbufImport_Fun)GetProcAddress(g_hMilDLL, "MbufImportW"));							//MbufImport_Fun	MbufImport_ptr;
	VERIFY(MbufChild2d_ptr = (MbufChild2d_Fun)GetProcAddress(g_hMilDLL, "MbufChild2d"));						//MbufChild2d_Fun	MbufChild2d_ptr;
	VERIFY(MbufGet_ptr			= (MbufGet_Fun			)GetProcAddress(g_hMilDLL, "MbufGet"		));
	VERIFY(MbufGetColor_ptr		= (MbufGetColor_Fun		)GetProcAddress(g_hMilDLL, "MbufGetColor"	));
	VERIFY(MbufCopyColor_ptr	= (MbufCopyColor_Fun	)GetProcAddress(g_hMilDLL, "MbufCopyColor"	));
	VERIFY(MbufCopyClip_ptr		= (MbufCopyClip_Fun		)GetProcAddress(g_hMilDLL, "MbufCopyClip"	));
	VERIFY(MbufCreate2d_ptr		= (MbufCreate2d_Fun		)GetProcAddress(g_hMilDLL, "MbufCreate2dFunc"	));
	VERIFY(MbufLoad_ptr			= (MbufLoad_Fun			)GetProcAddress(g_hMilDLL, "MbufLoadW"		));
	VERIFY(MbufSave_ptr			= (MbufSave_Fun			)GetProcAddress(g_hMilDLL, "MbufSaveW"		));
	VERIFY(MbufCreateColor_ptr	= (MbufCreateColor_Fun	)GetProcAddress(g_hMilDLL, "MbufCreateColor"));
	VERIFY(MbufAllocColor_ptr	= (MbufAllocColor_Fun	)GetProcAddress(g_hMilDLL, "MbufAllocColor"	));
	VERIFY(MbufPutColor_ptr		= (MbufPutColor_Fun		)GetProcAddress(g_hMilDLL, "MbufPutColor"	));
	VERIFY(MbufPutColor2d_ptr	= (MbufPutColor2d_Fun	)GetProcAddress(g_hMilDLL, "MbufPutColor2d"	));
	VERIFY(MbufGetColor2d_ptr	= (MbufGetColor2d_Fun	)GetProcAddress(g_hMilDLL, "MbufGetColor2d"	));

	VERIFY(MgraRect_ptr		= (MgraRect_Fun		)GetProcAddress(g_hMilDLL, "MgraRectDouble"	));							//MgraRect_Fun	MgraRect_ptr;
	VERIFY(MgraAlloc_ptr	= (MgraAlloc_Fun	)GetProcAddress(g_hMilDLL, "MgraAlloc"		));
	VERIFY(MgraFree_ptr		= (MgraFree_Fun		)GetProcAddress(g_hMilDLL, "MgraFree"		));
	VERIFY(MgraRectFill_ptr	= (MgraRectFill_Fun	)GetProcAddress(g_hMilDLL, "MgraRectFillDouble"	));
	VERIFY(MgraDot_ptr		= (MgraDot_Fun		)GetProcAddress(g_hMilDLL, "MgraDotDouble"		));
	VERIFY(MgraLine_ptr		= (MgraLine_Fun		)GetProcAddress(g_hMilDLL, "MgraLineDouble"		));
	VERIFY(MgraLines_ptr	= (MgraLines_Fun	)GetProcAddress(g_hMilDLL, "MgraLinesDouble"		));
	VERIFY(MgraText_ptr		= (MgraText_Fun		)GetProcAddress(g_hMilDLL, "MgraTextWDouble"		));

	return TRUE;
}
BOOL LoadMilblobDll()
{
	if (g_hMilblobDLL != NULL)
		return TRUE;

	CString strDll, strMsg;
	strDll.Format(_T("MilBlob.dll"));

	g_hMilblobDLL = LoadLibrary(strDll);
	if (g_hMilblobDLL == NULL)
	{
		strMsg.Format(_T("fail load %s"), strDll);
		AfxMessageBox(strMsg);
		return FALSE;
	}

	VERIFY(MblobGetResult_ptr = (MblobGetResult_Fun)GetProcAddress(g_hMilblobDLL, "MblobGetResult"));							//MblobGetResult_Fun MblobGetResult_ptr;
	VERIFY(MblobSelect_ptr = (MblobSelect_Fun)GetProcAddress(g_hMilblobDLL, "MblobSelect"));									//MblobSelect_Fun MblobSelect_ptr;
	VERIFY(MblobFill_ptr = (MblobFill_Fun)GetProcAddress(g_hMilblobDLL, "MblobFill"));											//MblobFill_Fun MblobFill_ptr;
	VERIFY(MblobGetNumber_ptr = (MblobGetNumber_Fun)GetProcAddress(g_hMilblobDLL, "MblobGetNumber"));							//MblobGetNumber_Fun MblobGetNumber_ptr;
	VERIFY(MblobCalculate_ptr = (MblobCalculate_Fun)GetProcAddress(g_hMilblobDLL, "MblobCalculate"));							//MblobCalculate_Fun MblobCalculate_ptr;
	VERIFY(MblobReconstruct_ptr = (MblobReconstruct_Fun)GetProcAddress(g_hMilblobDLL, "MblobReconstruct"));						//MblobReconstruct_Fun MblobReconstruct_ptr;
	VERIFY(MblobGetLabel_ptr = (MblobGetLabel_Fun)GetProcAddress(g_hMilblobDLL, "MblobGetLabel"));								//MblobGetLabel_Fun MblobGetLabel_ptr;
	VERIFY(MblobAllocFeatureList_ptr = (MblobAllocFeatureList_Fun)GetProcAddress(g_hMilblobDLL, "MblobAllocFeatureList"));		//MblobAllocFeatureList_Fun MblobAllocFeatureList_ptr;
	VERIFY(MblobAllocResult_ptr = (MblobAllocResult_Fun)GetProcAddress(g_hMilblobDLL, "MblobAllocResult"));						//MblobAllocResult_Fun MblobAllocResult_ptr;
	VERIFY(MblobSelectFeature_ptr = (MblobSelectFeature_Fun)GetProcAddress(g_hMilblobDLL, "MblobSelectFeature"));				//MblobSelectFeature_Fun MblobSelectFeature_ptr;
	VERIFY(MblobFree_ptr = (MblobFree_Fun)GetProcAddress(g_hMilblobDLL, "MblobFree"));											//MblobFree_Fun MblobFree_ptr;
	VERIFY(MblobDraw_ptr = (MblobDraw_Fun)GetProcAddress(g_hMilblobDLL, "MblobDraw"));											//MblobDraw_Fun MblobDraw_ptr;
	VERIFY(MblobLabel_ptr = (MblobLabel_Fun)GetProcAddress(g_hMilblobDLL, "MblobLabel"));										//MblobLabel_Fun MblobLabel_ptr;
	VERIFY(MblobGetResultSingle_ptr = (MblobGetResultSingle_Fun)GetProcAddress(g_hMilblobDLL, "MblobGetResultSingle"));			//MblobGetResultSingle_Fun MblobGetResultSingle_ptr;
	VERIFY(MblobControl_ptr = (MblobControl_Fun)GetProcAddress(g_hMilblobDLL, "MblobControlDouble"));

	return TRUE;
}
BOOL LoadMilimDll()
{
	if (g_hMilimDLL != NULL)
		return TRUE;

	CString strDll, strMsg;
	strDll.Format(_T("Milim.dll"));

	g_hMilimDLL = LoadLibrary(strDll);
	if (g_hMilimDLL == NULL)
	{
		strMsg.Format(_T("fail load %s"), strDll);
		AfxMessageBox(strMsg);
		return FALSE;
	}
	VERIFY(MimDilate_ptr = (MimDilate_Fun)GetProcAddress(g_hMilimDLL, "MimDilate"));								//MimDilate_Fun MimDilate_ptr;
	VERIFY(MimErode_ptr = (MimErode_Fun)GetProcAddress(g_hMilimDLL, "MimErode"));									//MimErode_Fun MimErode_ptr;
	VERIFY(MimCountDifference_ptr = (MimCountDifference_Fun)GetProcAddress(g_hMilimDLL, "MimCountDifference"));		//MimCountDifference_Fun MimCountDifference_ptr;
	VERIFY(MimResize_ptr = (MimResize_Fun)GetProcAddress(g_hMilimDLL, "MimResize"));								//MimResize_Fun MimResize_ptr;
	VERIFY(MimBinarize_ptr = (MimBinarize_Fun)GetProcAddress(g_hMilimDLL, "MimBinarize"));							//MimBinarize_Fun MimBinarize_ptr;
	VERIFY(MimAllocResult_ptr = (MimAllocResult_Fun)GetProcAddress(g_hMilimDLL, "MimAllocResult"));					//MimAllocResult_Fun MimAllocResult_ptr;
	VERIFY(MimFree_ptr = (MimFree_Fun)GetProcAddress(g_hMilimDLL, "MimFree"));										//MimFree_Fun MimFree_ptr;
	VERIFY(MimInquire_ptr = (MimInquire_Fun)GetProcAddress(g_hMilimDLL, "MimInquire"));								//MimInquire_Fun MimInquire_ptr;
	VERIFY(MimGetResult_ptr = (MimGetResult_Fun)GetProcAddress(g_hMilimDLL, "MimGetResult"));						//MimGetResult_Fun MimGetResult_ptr;
	VERIFY(MimClose_ptr = (MimClose_Fun)GetProcAddress(g_hMilimDLL, "MimClose"));									//MimClose_Fun MimClose_ptr;
	VERIFY(MimOpen_ptr = (MimOpen_Fun)GetProcAddress(g_hMilimDLL, "MimOpen"));										//MimOpen_Fun MimOpen_ptr;
	VERIFY(MimArith_ptr = (MimArith_Fun)GetProcAddress(g_hMilimDLL, "MimArithDouble"));									//MimArith_Fun MimArith_ptr;
	VERIFY(MimHistogramEqualize_ptr = (MimHistogramEqualize_Fun)GetProcAddress(g_hMilimDLL, "MimHistogramEqualize"	));
	VERIFY(MimRotate_ptr		    = (MimRotate_Fun		   )GetProcAddress(g_hMilimDLL, "MimRotate"			));
	VERIFY(MimMorphic_ptr		    = (MimMorphic_Fun		   )GetProcAddress(g_hMilimDLL, "MimMorphic"		));
	VERIFY(MimFlip_ptr			    = (MimFlip_Fun			   )GetProcAddress(g_hMilimDLL, "MimFlip"			));
	VERIFY(MimRank_ptr			    = (MimRank_Fun			   )GetProcAddress(g_hMilimDLL, "MimRank"			));
	VERIFY(MimTranslate_ptr		    = (MimTranslate_Fun		   )GetProcAddress(g_hMilimDLL, "MimTranslate"		));
	VERIFY(MimEdgeDetect_ptr	    = (MimEdgeDetect_Fun	   )GetProcAddress(g_hMilimDLL, "MimEdgeDetect"		));
	VERIFY(MimHistogram_ptr		    = (MimHistogram_Fun		   )GetProcAddress(g_hMilimDLL, "MimHistogram"		));
	VERIFY(MimProject_ptr		    = (MimProject_Fun		   )GetProcAddress(g_hMilimDLL, "MimProject"		));
	VERIFY(MimStat_ptr			    = (MimStat_Fun			   )GetProcAddress(g_hMilimDLL, "MimStatDouble"		));
	VERIFY(MimTransform_ptr			= (MimTransform_Fun		   )GetProcAddress(g_hMilimDLL, "MimTransform"		));
	return TRUE;
}
BOOL LoadMilpatDll()
{
	if (g_hMilpatDLL != NULL)
		return TRUE;

	CString strDll, strMsg;
	strDll.Format(_T("milpat.dll"));

	g_hMilpatDLL = LoadLibrary(strDll);
	if (g_hMilpatDLL == NULL)
	{
		strMsg.Format(_T("fail load %s"), strDll);
		AfxMessageBox(strMsg);
		return FALSE;
	}

	VERIFY(MpatGetResult_ptr = (MpatGetResult_Fun)GetProcAddress(g_hMilpatDLL, "MpatGetResult"));				//MpatGetResult_Fun MpatGetResult_ptr;
	VERIFY(MpatGetNumber_ptr = (MpatGetNumber_Fun)GetProcAddress(g_hMilpatDLL, "MpatGetNumber"));				//MpatGetNumber_Fun MpatGetNumber_ptr;
	VERIFY(MpatFindModel_ptr = (MpatFindModel_Fun)GetProcAddress(g_hMilpatDLL, "MpatFindModel"));				//MpatFindModel_Fun MpatFindModel_ptr;
	VERIFY(MpatAllocResult_ptr = (MpatAllocResult_Fun)GetProcAddress(g_hMilpatDLL, "MpatAllocResult"));			//MpatAllocResult_Fun MpatAllocResult_ptr;
	VERIFY(MpatAllocModel_ptr = (MpatAllocModel_Fun)GetProcAddress(g_hMilpatDLL, "MpatAllocModel"));			//MpatAllocModel_Fun MpatAllocModel_ptr;
	VERIFY(MpatSetAngle_ptr = (MpatSetAngle_Fun)GetProcAddress(g_hMilpatDLL, "MpatSetAngle"));					//MpatSetAngle_Fun MpatSetAngle_ptr;
	VERIFY(MpatSetAccuracy_ptr = (MpatSetAccuracy_Fun)GetProcAddress(g_hMilpatDLL, "MpatSetAccuracy"));			//MpatSetAccuracy_Fun		MpatSetAccuracy_ptr;
	VERIFY(MpatSetAcceptance_ptr = (MpatSetAcceptance_Fun)GetProcAddress(g_hMilpatDLL, "MpatSetAcceptance"));	//MpatSetAcceptance_Fun	MpatSetAcceptance_ptr;
	VERIFY(MpatPreprocModel_ptr = (MpatPreprocModel_Fun)GetProcAddress(g_hMilpatDLL, "MpatPreprocModel"));		//MpatPreprocModel_Fun	MpatPreprocModel_ptr;
	VERIFY(MpatFree_ptr = (MpatFree_Fun)GetProcAddress(g_hMilpatDLL, "MpatFree"));								//MpatFree_Fun			MpatFree_ptr;
	VERIFY(MpatSetSpeed_ptr = (MpatSetSpeed_Fun)GetProcAddress(g_hMilpatDLL, "MpatSetSpeed"));												//MpatSetSpeed_Fun		MpatSetSpeed_ptr;
	VERIFY(MpatAllocRotatedModel_ptr = (MpatAllocRotatedModel_Fun)GetProcAddress(g_hMilpatDLL, "MpatAllocRotatedModel"));
	VERIFY(MpatInquire_ptr		= (MpatInquire_Fun		)GetProcAddress(g_hMilpatDLL, "MpatInquire"		));
	VERIFY(MpatSetPosition_ptr	= (MpatSetPosition_Fun	)GetProcAddress(g_hMilpatDLL, "MpatSetPosition"	));
	VERIFY(MpatSave_ptr			= (MpatSave_Fun			)GetProcAddress(g_hMilpatDLL, "MpatSaveW"		));
	VERIFY(MpatCopy_ptr			= (MpatCopy_Fun			)GetProcAddress(g_hMilpatDLL, "MpatCopy"		));
	VERIFY(MpatRestore_ptr		= (MpatRestore_Fun		)GetProcAddress(g_hMilpatDLL, "MpatRestoreW"		));

//	VERIFY(_ptr = (_Fun)GetProcAddress(g_hMilpatDLL, "MimDilate"));

	return TRUE;
}
BOOL LoadMilcodeDll()
{
	if (g_hMilcodeDLL != NULL)
		return TRUE;

	CString strDll, strMsg;
	strDll.Format(_T("milcode.dll"));

	g_hMilcodeDLL = LoadLibrary(strDll);
	if (g_hMilcodeDLL == NULL)
	{
		strMsg.Format(_T("fail load %s"), strDll);
		AfxMessageBox(strMsg);
		return FALSE;
	}
	VERIFY(McodeRead_ptr = (McodeRead_Fun)GetProcAddress(g_hMilcodeDLL, "McodeRead"));						//McodeRead_Fun			McodeRead_ptr;
	VERIFY(McodeModel_ptr = (McodeModel_Fun)GetProcAddress(g_hMilcodeDLL, "McodeModel"));					//McodeModel_Fun			McodeModel_ptr;
	VERIFY(McodeGetResult_ptr = (McodeGetResult_Fun)GetProcAddress(g_hMilcodeDLL, "McodeGetResult"));		//McodeGetResult_Fun		McodeGetResult_ptr;
	VERIFY(McodeFree_ptr = (McodeFree_Fun)GetProcAddress(g_hMilcodeDLL, "McodeFree"));						//McodeFree_Fun			McodeFree_ptr;
	VERIFY(McodeControl_ptr = (McodeControl_Fun)GetProcAddress(g_hMilcodeDLL, "McodeControlDouble"));				//McodeControl_Fun		McodeControl_ptr;
	VERIFY(McodeAllocResult_ptr = (McodeAllocResult_Fun)GetProcAddress(g_hMilcodeDLL, "McodeAllocResult"));	//McodeAllocResult_Fun	McodeAllocResult_ptr;
	VERIFY(McodeAlloc_ptr = (McodeAlloc_Fun)GetProcAddress(g_hMilcodeDLL, "McodeAlloc"));					//McodeAlloc_Fun			McodeAlloc_ptr;

	return TRUE;
}
BOOL LoadMilstrDll()
{
	if(g_hMilstrDLL != NULL)
		return TRUE;

	CString strDll, strMsg;
	strDll.Format(_T("milstr.dll"));

	g_hMilstrDLL = LoadLibrary(strDll);
	if (g_hMilstrDLL == NULL)
	{
		strMsg.Format(_T("fail load %s"), strDll);
		AfxMessageBox(strMsg);
		return FALSE;
	}

	VERIFY(MstrFree_ptr				= (MstrFree_Fun			)GetProcAddress(g_hMilstrDLL, "MstrFree"			));
	VERIFY(MstrControl_ptr			= (MstrControl_Fun		)GetProcAddress(g_hMilstrDLL, "MstrControlDouble"	));
	VERIFY(MstrAlloc_ptr			= (MstrAlloc_Fun		)GetProcAddress(g_hMilstrDLL, "MstrAlloc"			));
	VERIFY(MstrEditFont_ptr			= (MstrEditFont_Fun		)GetProcAddress(g_hMilstrDLL, "MstrEditFont"		));
	VERIFY(MstrInquire_ptr			= (MstrInquire_Fun		)GetProcAddress(g_hMilstrDLL, "MstrInquire"			));
	VERIFY(MstrSave_ptr				= (MstrSave_Fun			)GetProcAddress(g_hMilstrDLL, "MstrSaveW"			));
	VERIFY(MstrRestore_ptr			= (MstrRestore_Fun		)GetProcAddress(g_hMilstrDLL, "MstrRestoreW"		));
	VERIFY(MstrSetConstraint_ptr	= (MstrSetConstraint_Fun)GetProcAddress(g_hMilstrDLL, "MstrSetConstraint"	));
	VERIFY(MstrPreprocess_ptr		= (MstrPreprocess_Fun	)GetProcAddress(g_hMilstrDLL, "MstrPreprocess"		));
	VERIFY(MstrRead_ptr				= (MstrRead_Fun			)GetProcAddress(g_hMilstrDLL, "MstrRead"			));
	VERIFY(MstrGetResult_ptr		= (MstrGetResult_Fun	)GetProcAddress(g_hMilstrDLL, "MstrGetResult"		));
	VERIFY(MstrDraw_ptr				= (MstrDraw_Fun			)GetProcAddress(g_hMilstrDLL, "MstrDraw"			));
	VERIFY(MstrAllocResult_ptr		= (MstrAllocResult_Fun	)GetProcAddress(g_hMilstrDLL, "MstrAllocResult"		));

	return TRUE;
}
BOOL LoadMilcalDll()
{
	if(g_hMilcalDLL != NULL)
		return TRUE;

	CString strDll, strMsg;
	strDll.Format(_T("milcal.dll"));

	g_hMilcalDLL = LoadLibrary(strDll);
	if (g_hMilcalDLL == NULL)
	{
		strMsg.Format(_T("fail load %s"), strDll);
		AfxMessageBox(strMsg);
		return FALSE;
	}

	VERIFY(McalFree_ptr		= (McalFree_Fun		)GetProcAddress(g_hMilcalDLL, "McalFree"	));
	VERIFY(McalAlloc_ptr	= (McalAlloc_Fun	)GetProcAddress(g_hMilcalDLL, "McalAlloc"	));
	VERIFY(McalList_ptr		= (McalList_Fun		)GetProcAddress(g_hMilcalDLL, "McalList"	));
	VERIFY(McalInquire_ptr	= (McalInquire_Fun	)GetProcAddress(g_hMilcalDLL, "McalInquire"	));

	return TRUE;
}

BOOL FreeMilDll()
{
	if (g_hMilDLL != NULL)
	{
		FreeLibrary(g_hMilDLL);
		g_hMilDLL = NULL;
	}
	return TRUE;
}
BOOL FreeMilblobDll()
{
	if (g_hMilblobDLL != NULL)
	{
		FreeLibrary(g_hMilblobDLL);
		g_hMilblobDLL = NULL;
	}
	return TRUE;
}
BOOL FreeMilimDll()
{
	if (g_hMilimDLL != NULL)
	{
		FreeLibrary(g_hMilimDLL);
		g_hMilimDLL = NULL;
	}
	return TRUE;
}
BOOL FreeMilpatDll()
{
	if (g_hMilpatDLL != NULL)
	{
		FreeLibrary(g_hMilpatDLL);
		g_hMilpatDLL = NULL;
	}
	return TRUE;
}
BOOL FreeMilcodeDll()
{
	if (g_hMilcodeDLL != NULL)
	{
		FreeLibrary(g_hMilcodeDLL);
		g_hMilcodeDLL = NULL;
	}
	return TRUE;
}
BOOL FreeMilstrDll()
{
	if (g_hMilstrDLL != NULL)
	{
		FreeLibrary(g_hMilstrDLL);
		g_hMilstrDLL = NULL;
	}
	return TRUE;
}
BOOL FreeMilcalDll()
{
	if (g_hMilcalDLL != NULL)
	{
		FreeLibrary(g_hMilcalDLL);
		g_hMilcalDLL = NULL;
	}
	return TRUE;
}

MIL_INT MsysInquire(MIL_ID SystemId, MIL_INT InquireType, void *UserVarPtr)
{
	return (*MsysInquire_ptr)(SystemId, InquireType, UserVarPtr);
}
MIL_ID MsysAlloc(MIL_CONST_TEXT_PTR SystemDescriptor, MIL_INT SystemNum, MIL_INT InitFlag, MIL_ID *SystemIdPtr)
{
	if (g_bMil10 == TRUE)
	{
		return (*MsysAlloc10_ptr)(M_DEFAULT, SystemDescriptor, SystemNum, InitFlag, SystemIdPtr);
	}
	else
	{
		return (*MsysAlloc_ptr)(SystemDescriptor, SystemNum, InitFlag, SystemIdPtr);
	}
}
void MsysFree(MIL_ID SystemId)
{
	(*MsysFree_ptr)(SystemId);
}

MIL_INT MappInquire(MIL_INT InquireType, void *UserVarPtr)
{
	if (g_bMil10 == TRUE)
	{
		return (*MappInquire10_ptr)(M_DEFAULT, InquireType, UserVarPtr);
	}
	else
	{
		return (*MappInquire_ptr)(InquireType, UserVarPtr);
	}
}
MIL_ID MappAlloc(MIL_INT InitFlag, MIL_ID *ApplicationIdPtr)
{
	if (g_bMil10 == TRUE)
	{
		return (*MappAlloc10_ptr)(MIL_TEXT("M_DEFAULT"), InitFlag, ApplicationIdPtr);
	}
	else
	{
		return (*MappAlloc_ptr)(InitFlag, ApplicationIdPtr);
	}
}
MIL_DOUBLE MappTimer(MIL_INT ControlType, MIL_DOUBLE *TimePtr)
{
	if (g_bMil10 == TRUE)
	{
		return (*MappTimer10_ptr)(M_DEFAULT , ControlType, TimePtr);
	}
	else
	{
		return (*MappTimer_ptr)(ControlType, TimePtr);
	}
}
void MappFree(MIL_ID ApplicationId)
{
	(*MappFree_ptr)(ApplicationId);
}
void MappControl(MIL_INT ControlType, MIL_INT ControlValue)
{
	if (g_bMil10 == TRUE)
	{
		(*MappControl10_ptr)(M_DEFAULT, ControlType, ControlValue);
	}
	else
	{
		(*MappControl_ptr)(ControlType, ControlValue);
	}
}

void MdigControl(MIL_ID DigId, MIL_INT64 ControlType, MIL_DOUBLE ControlValue)
{
	(*MdigControl_ptr)(DigId, ControlType, ControlValue);
}
void MdigHookFunction(MIL_ID DigId, MIL_INT HookType, MIL_DIG_HOOK_FUNCTION_PTR HookHandlerPtr, void *UserDataPtr)
{
	(*MdigHookFunction_ptr)(DigId, HookType, HookHandlerPtr, UserDataPtr);
}
void MdigProcess(MIL_ID DigId, const MIL_ID *DestImageArrayPtr, MIL_INT ImageCount, MIL_INT Operation, MIL_INT OperationFlag, MIL_DIG_HOOK_FUNCTION_PTR HookHandlerPtr, void *UserDataPtr)
{
	(*MdigProcess_ptr)(DigId, DestImageArrayPtr, ImageCount, Operation, OperationFlag, HookHandlerPtr, UserDataPtr);
}
void MdigGrab(MIL_ID DigId, MIL_ID DestImageBufId)
{
	(*MdigGrab_ptr)(DigId, DestImageBufId);
}
void MdigGrabWait(MIL_ID DigId, MIL_INT ControlFlag)
{
	(*MdigGrabWait_ptr)(DigId, ControlFlag);
}
MIL_ID MdigAlloc(MIL_ID SystemId, MIL_INT DigNum, MIL_CONST_TEXT_PTR DataFormat, MIL_INT InitFlag, MIL_ID *DigIdPtr)
{
	return (*MdigAlloc_ptr)(SystemId, DigNum, DataFormat, InitFlag, DigIdPtr);
}
MIL_INT MdigInquire(MIL_ID DigId, MIL_INT64 InquireType, void *UserVarPtr)
{
	return (*MdigInquire_ptr)(DigId, InquireType, UserVarPtr);
}
void MdigHalt(MIL_ID DigId)
{
	(*MdigHalt_ptr)(DigId);
}
void MdigGrabContinuous(MIL_ID DigId, MIL_ID DestImageBufId)
{
	(*MdigGrabContinuous_ptr)(DigId, DestImageBufId);
}
void MdigFree(MIL_ID DigId)
{
	(*MdigFree_ptr)(DigId);
}

MIL_INT MbufInquire(MIL_ID BufId, MIL_INT InquireType, void *UserVarPtr)
{
	return (*MbufInquire_ptr)(BufId, InquireType, UserVarPtr);
}
MIL_ID MbufAlloc2d(MIL_ID SystemId, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT64 Attribute, MIL_ID *BufIdPtr)
{
	return (*MbufAlloc2d_ptr)(SystemId, SizeX, SizeY, Type, Attribute, BufIdPtr);
}
void MbufCopy(MIL_ID SrcBufId, MIL_ID DestBufId)
{
	(*MbufCopy_ptr)(SrcBufId, DestBufId);
}
void MbufExport(MIL_CONST_TEXT_PTR Filename, MIL_INT FileFormat, MIL_ID SrcBufId)
{
	(*MbufExport_ptr)(Filename, FileFormat, SrcBufId);
}
void MbufFree(MIL_ID BufId)
{
	(*MbufFree_ptr)(BufId);
}
void MbufClear(MIL_ID DestImageBufId, MIL_DOUBLE Color)
{
	(*MbufClear_ptr)(DestImageBufId, Color);
}
void MbufPut2d(MIL_ID DestBufId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, const void *UserArrayPtr)
{
	(*MbufPut2d_ptr)(DestBufId, OffX, OffY, SizeX, SizeY, UserArrayPtr);
}
void MbufGet2d(MIL_ID SrcBufId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, void *UserArrayPtr)
{
	(*MbufGet2d_ptr)(SrcBufId, OffX, OffY, SizeX, SizeY, UserArrayPtr);
}
MIL_ID MbufRestore(MIL_CONST_TEXT_PTR Filename, MIL_ID SystemId, MIL_ID *BufIdPtr)
{
	return (*MbufRestore_ptr)(Filename, SystemId, BufIdPtr);
}
void MbufPut(MIL_ID DestBufId, const void *UserArrayPtr)
{
	(*MbufPut_ptr)(DestBufId, UserArrayPtr);
}
MIL_ID MbufImport(MIL_TEXT_PTR Filename, MIL_INT FileFormat, MIL_INT Operation, MIL_ID SystemId, MIL_ID *BufIdPtr)
{
	return (*MbufImport_ptr)(Filename, FileFormat, Operation, SystemId, BufIdPtr);
}
MIL_ID MbufChild2d(MIL_ID ParentBufId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, MIL_ID *BufIdPtr)
{
	return (*MbufChild2d_ptr)(ParentBufId, OffY, OffY, SizeX, SizeY, BufIdPtr);
}
void MbufGet(MIL_ID SrcBufId, void *UserArrayPtr)
{
	(*MbufGet_ptr)(SrcBufId, UserArrayPtr);
}
void MbufGetColor(MIL_ID SrcBufId, MIL_INT DataFormat, MIL_INT Band, void *UserArrayPtr)
{
	(*MbufGetColor_ptr)(SrcBufId, DataFormat, Band, UserArrayPtr);
}
void MbufCopyColor(MIL_ID SrcBufId, MIL_ID DestBufId, MIL_INT Band)
{
	(*MbufCopyColor_ptr)(SrcBufId, DestBufId, Band);
}
void MbufCopyClip(MIL_ID SrcBufId, MIL_ID DestBufId, MIL_INT DestOffX, MIL_INT DestOffY)
{
	(*MbufCopyClip_ptr)(SrcBufId, DestBufId, DestOffX, DestOffY);
}
void MbufLoad(MIL_CONST_TEXT_PTR Filename, MIL_ID BufId)
{
	(*MbufLoad_ptr)(Filename, BufId);
}
void MbufSave(MIL_CONST_TEXT_PTR FileName, MIL_ID BufId)
{
	(*MbufSave_ptr)(FileName, BufId);
}
void MbufPutColor(MIL_ID DestBufId, MIL_INT DataFormat, MIL_INT Band, const void *UserArrayPtr)
{
	(*MbufPutColor_ptr)(DestBufId, DataFormat, Band, UserArrayPtr);
}
void MbufPutColor2d(MIL_ID DestBufId, MIL_INT DataFormat, MIL_INT Band, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, const void *UserArrayPtr)
{
	(*MbufPutColor2d_ptr)(DestBufId, DataFormat, Band, OffX, OffY, SizeX, SizeY, UserArrayPtr);
}
MIL_ID MbufCreate2d(MIL_ID SystemId, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT64 Attribute, MIL_INT64 ControlFlag, MIL_INT Pitch, void *DataPtr, MIL_ID *BufIdPtr)
{
	return (*MbufCreate2d_ptr)(SystemId, SizeX, SizeY, Type, Attribute, ControlFlag, Pitch, DataPtr, BufIdPtr);
}
MIL_ID MbufCreateColor(MIL_ID SystemId, MIL_INT SizeBand, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT64 Attribute, MIL_INT64 ControlFlag, MIL_INT Pitch, void **ArrayOfDataPtr, MIL_ID *BufIdPtr)
{
	return (*MbufCreateColor_ptr)(SystemId, SizeBand, SizeX, SizeY, Type, Attribute, ControlFlag, Pitch, ArrayOfDataPtr, BufIdPtr);
}
MIL_ID MbufAllocColor(MIL_ID SystemId, MIL_INT SizeBand, MIL_INT SizeX, MIL_INT SizeY, MIL_INT Type, MIL_INT64 Attribute, MIL_ID *BufIdPtr)
{
	return (*MbufAllocColor_ptr)(SystemId, SizeBand, SizeX, SizeY, Type, Attribute, BufIdPtr);
}
void MbufGetColor2d(MIL_ID SrcBufId, MIL_INT DataFormat, MIL_INT Band, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, void *UserArrayPtr)
{
	(*MbufGetColor2d_ptr)(SrcBufId, DataFormat, Band, OffX, OffY, SizeX, SizeY, UserArrayPtr);
}


void MgraColor(MIL_ID GraphContId, MIL_DOUBLE ForegroundColor)
{
	(*MgraColor_ptr)(GraphContId, ForegroundColor);
}
void MgraRect(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XStart, MIL_DOUBLE YStart, MIL_DOUBLE XEnd, MIL_DOUBLE YEnd)
{
	(*MgraRect_ptr)(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
}
MIL_ID MgraAlloc(MIL_ID SystemId, MIL_ID *GraphContIdPtr)
{
	return (*MgraAlloc_ptr)(SystemId, GraphContIdPtr);
}
void MgraFree(MIL_ID ObjectId)
{
	(*MgraFree_ptr)(ObjectId);
}
void MgraRectFill(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XStart, MIL_DOUBLE YStart, MIL_DOUBLE XEnd, MIL_DOUBLE YEnd)
{
	(*MgraRectFill_ptr)(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
}
void MgraDot(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XPos, MIL_DOUBLE YPos)
{
	(*MgraDot_ptr)(GraphContId, DestImageBufIdOrGraListId, XPos, YPos);
}
void MgraLine(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XStart, MIL_DOUBLE YStart, MIL_DOUBLE XEnd, MIL_DOUBLE YEnd)
{
	(*MgraLine_ptr)(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, XEnd, YEnd);
}
void MgraLines(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_INT NumberOfLinesOrVertices, const MIL_DOUBLE *XStartArray, const MIL_DOUBLE *YStartArray, const MIL_DOUBLE *XEndArray, const MIL_DOUBLE *YEndArray, MIL_INT ControlFlag)
{
	(*MgraLines_ptr)(GraphContId, DestImageBufIdOrGraListId, NumberOfLinesOrVertices, XStartArray, YStartArray, XEndArray, YEndArray, ControlFlag);
}
void MgraText(MIL_ID GraphContId, MIL_ID DestImageBufIdOrGraListId, MIL_DOUBLE XStart, MIL_DOUBLE YStart, MIL_CONST_TEXT_PTR StringPtr)
{
	(*MgraText_ptr)(GraphContId, DestImageBufIdOrGraListId, XStart, YStart, StringPtr);
}

void MblobGetResult(MIL_ID BlobResId, MIL_INT Feature, void *TargetArrayPtr)
{
	(*MblobGetResult_ptr)(BlobResId, Feature, TargetArrayPtr);
}
void MblobSelect(MIL_ID BlobResId, MIL_INT Operation, MIL_INT SelectionCriterion, MIL_INT Condition, MIL_DOUBLE CondLow, MIL_DOUBLE CondHigh)
{
	(*MblobSelect_ptr)(BlobResId, Operation, SelectionCriterion, Condition, CondLow, CondHigh);
}
void MblobFill(MIL_ID BlobResId, MIL_ID DestImageBufId, MIL_INT Criterion, MIL_INT Value)
{
	(*MblobFill_ptr)(BlobResId, DestImageBufId, Criterion, Value);
}
MIL_INT MblobGetNumber(MIL_ID BlobResId, MIL_INT *CountVarPtr)
{
	return (*MblobGetNumber_ptr)(BlobResId, CountVarPtr);
}
void MblobCalculate(MIL_ID BlobIdentImageId, MIL_ID GrayImageId, MIL_ID FeatureListId, MIL_ID BlobResId)
{
	(*MblobCalculate_ptr)(BlobIdentImageId, GrayImageId, FeatureListId, BlobResId);
}
void MblobReconstruct(MIL_ID SrcImageBufId, MIL_ID SeedImageBufId, MIL_ID DestImageBufId, MIL_INT Operation, MIL_INT ProcMode)
{
	(*MblobReconstruct_ptr)(SrcImageBufId, SeedImageBufId, DestImageBufId, Operation, ProcMode);
}
MIL_ID MblobGetLabel(MIL_ID BlobResId, MIL_INT XPos, MIL_INT YPos, MIL_INT *LabelVarPtr)
{
	return (*MblobGetLabel_ptr)(BlobResId, XPos, YPos, LabelVarPtr);
}
MIL_ID MblobAllocFeatureList(MIL_ID SystemId, MIL_ID *FeatureListIdPtr)
{
	return (*MblobAllocFeatureList_ptr)(SystemId, FeatureListIdPtr);
}
MIL_ID MblobAllocResult(MIL_ID SystemId, MIL_ID *BlobResIdPtr)
{
	return (*MblobAllocResult_ptr)(SystemId, BlobResIdPtr);
}
void MblobSelectFeature(MIL_ID FeatureListId, MIL_INT Feature)
{
	(*MblobSelectFeature_ptr)(FeatureListId, Feature);
}
void MblobFree(MIL_ID BlobId)
{
	(*MblobFree_ptr)(BlobId);
}
void MblobDraw(MIL_ID GraphContId, MIL_ID ResultId, MIL_ID DestImageId, MIL_INT Operation, MIL_INT Label, MIL_INT ControlFlag)
{
	(*MblobDraw_ptr)(GraphContId, ResultId, DestImageId, Operation, Label, ControlFlag);
}
void MblobLabel(MIL_ID BlobResId, MIL_ID DestImageBufId, MIL_INT Mode)
{
	(*MblobLabel_ptr)(BlobResId, DestImageBufId, Mode);
}
void MblobGetResultSingle(MIL_ID BlobResId, MIL_INT LabelVal, MIL_INT Feature, void *TargetVarPtr)
{
	(*MblobGetResultSingle_ptr)(BlobResId, LabelVal, Feature, TargetVarPtr);
}
void MblobControl(MIL_ID BlobResId, MIL_INT ControlType, MIL_DOUBLE ControlValue)
{
	(*MblobControl_ptr)(BlobResId, ControlType, ControlValue);
}

void MimDilate(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT NbIteration, MIL_INT ProcMode)
{
	(*MimDilate_ptr)(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
}
void MimErode(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT NbIteration, MIL_INT ProcMode)
{
	(*MimErode_ptr)(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
}
void MimCountDifference(MIL_ID Src1ImageBufId, MIL_ID Src2ImageBufId, MIL_ID ImResultId)
{
	(*MimCountDifference_ptr)(Src1ImageBufId, Src2ImageBufId, ImResultId);
}
void MimResize(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_DOUBLE ScaleFactorX, MIL_DOUBLE ScaleFactorY, MIL_INT InterpolationMode)
{
	(*MimResize_ptr)(SrcImageBufId, DestImageBufId, ScaleFactorX, ScaleFactorY, InterpolationMode);
}
MIL_INT MimBinarize(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT ConditionAndThreshMode, MIL_DOUBLE LowParam, MIL_DOUBLE HighParam)
{
	return (*MimBinarize_ptr)(SrcImageBufId, DestImageBufId, ConditionAndThreshMode, LowParam, HighParam);
}
MIL_ID MimAllocResult(MIL_ID SystemId, MIL_INT NbEntries, MIL_INT64 ResultType, MIL_ID *ImResultIdPtr)
{
	return (*MimAllocResult_ptr)(SystemId, NbEntries, ResultType, ImResultIdPtr);
}
void MimFree(MIL_ID ImResultId)
{
	(*MimFree_ptr)(ImResultId);
}
MIL_INT MimInquire(MIL_ID ContextorResultId, MIL_INT InquireType, void *UserVarPtr)
{
	return (*MimInquire_ptr)(ContextorResultId, InquireType, UserVarPtr);
}
void MimGetResult(MIL_ID ImResultId, MIL_INT ResultType, void *UserArrayPtr)
{
	(*MimGetResult_ptr)(ImResultId, ResultType, UserArrayPtr);
}
void MimClose(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT NbIteration, MIL_INT ProcMode)
{
	(*MimClose_ptr)(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
}
void MimOpen(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT NbIteration, MIL_INT ProcMode)
{
	(*MimOpen_ptr)(SrcImageBufId, DestImageBufId, NbIteration, ProcMode);
}
void MimArith(MIL_DOUBLE Src1ImageBufIdOrConst, MIL_DOUBLE Src2ImageBufIdOrConst, MIL_ID DestImageBufId, MIL_INT Operation)
{
	(*MimArith_ptr)(Src1ImageBufIdOrConst, Src2ImageBufIdOrConst, DestImageBufId, Operation);
}
void MimHistogramEqualize(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT Method, MIL_DOUBLE Alpha, MIL_DOUBLE Min, MIL_DOUBLE Max)
{
	(*MimHistogramEqualize_ptr)(SrcImageBufId, DestImageBufId, Method, Alpha, Min, Max);
}
void MimRotate(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_DOUBLE Angle, MIL_DOUBLE SrcCenX, MIL_DOUBLE SrcCenY, MIL_DOUBLE DstCenX, MIL_DOUBLE DstCenY, MIL_INT InterpolationMode)
{
	(*MimRotate_ptr)(SrcImageBufId, DestImageBufId, Angle, SrcCenX, SrcCenY, DstCenX, DstCenY, InterpolationMode);
}
void MimMorphic(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_ID StructElemBufId, MIL_INT Operation, MIL_INT NbIterationOrArea, MIL_INT ProcMode)
{
	(*MimMorphic_ptr)(SrcImageBufId, DestImageBufId, StructElemBufId, Operation, NbIterationOrArea, ProcMode);
}
void MimFlip(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_INT Operation, MIL_INT OpFlag)
{
	(*MimFlip_ptr)(SrcImageBufId, DestImageBufId, Operation, OpFlag);
}
void MimRank(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_ID StructElemBufId, MIL_INT Rank, MIL_INT ProcMode)
{
	(*MimRank_ptr)(SrcImageBufId, DestImageBufId, StructElemBufId, Rank, ProcMode);
}
void MimTranslate(MIL_ID SrcImageBufId, MIL_ID DestImageBufId, MIL_DOUBLE XDisplacement, MIL_DOUBLE YDisplacement, MIL_INT InterpolationMode)
{
	(*MimTranslate_ptr)(SrcImageBufId, DestImageBufId, XDisplacement, YDisplacement, InterpolationMode);
}
void MimEdgeDetect(MIL_ID SrcImageBufId, MIL_ID DestIntensityImageBufId, MIL_ID DestAngleImageBufId, MIL_ID KernelId, MIL_INT ControlFlag, MIL_INT Threshold)
{
	(*MimEdgeDetect_ptr)(SrcImageBufId, DestIntensityImageBufId, DestAngleImageBufId, KernelId, ControlFlag, Threshold);
}
void MimHistogram(MIL_ID SrcImageBufId, MIL_ID HistImResultId)
{
	(*MimHistogram_ptr)(SrcImageBufId, HistImResultId);
}
void MimProject(MIL_ID SrcImageBufId, MIL_ID ProjImResultId, MIL_DOUBLE ProjAngle)
{
	(*MimProject_ptr)(SrcImageBufId, ProjImResultId, ProjAngle);
}
void MimStat(MIL_ID SrcImageId, MIL_ID StatResultId, MIL_INT StatType, MIL_INT Condition, MIL_DOUBLE CondLow, MIL_DOUBLE CondHigh)
{
	(*MimStat_ptr)(SrcImageId, StatResultId, StatType, Condition, CondLow, CondHigh);
}
void MimTransform(MIL_ID SrcImageRBufId, MIL_ID SrcImageIBufId, MIL_ID DestImageRBufId, MIL_ID DestImageIBufId, MIL_INT TransformType, MIL_INT ControlFlag)
{
	(*MimTransform_ptr)(SrcImageRBufId, SrcImageIBufId, DestImageRBufId, DestImageIBufId, TransformType, ControlFlag);
}

void MpatGetResult(MIL_ID PatResultId, MIL_INT ResultType, void *UserArrayPtr)
{
	(*MpatGetResult_ptr)(PatResultId, ResultType, UserArrayPtr);
}
MIL_INT MpatGetNumber(MIL_ID PatResultId, MIL_INT *CountPtr)
{
	return (*MpatGetNumber_ptr)(PatResultId, CountPtr);
}
void MpatFindModel(MIL_ID ImageBufId, MIL_ID ModelId, MIL_ID PatResultId)
{
	(*MpatFindModel_ptr)(ImageBufId, ModelId, PatResultId);
}
MIL_ID MpatAllocResult(MIL_ID SystemId, MIL_INT NbEntries, MIL_ID *PatResultIdPtr)
{
	return (*MpatAllocResult_ptr)(SystemId, NbEntries, PatResultIdPtr);
}
MIL_ID MpatAllocModel(MIL_ID SystemId, MIL_ID SrcImageBufId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY, MIL_INT ModelType, MIL_ID *ModelIdPtr)
{
	return (*MpatAllocModel_ptr)(SystemId, SrcImageBufId, OffX, OffY, SizeX, SizeY, ModelType, ModelIdPtr);
}
void MpatSetAngle(MIL_ID ModelId, MIL_INT ControlType, MIL_DOUBLE ControlValue)
{
	(*MpatSetAngle_ptr)(ModelId, ControlType, ControlValue);
}
void MpatSetAccuracy(MIL_ID ModelId, MIL_INT Accuracy)
{
	(*MpatSetAccuracy_ptr)(ModelId, Accuracy);
}
void MpatSetAcceptance(MIL_ID ModelId, MIL_DOUBLE AcceptanceThreshold)
{
	(*MpatSetAcceptance_ptr)(ModelId, AcceptanceThreshold);
}
void MpatPreprocModel(MIL_ID TypicalImageBufId, MIL_ID ModelId, MIL_INT Mode)
{
	(*MpatPreprocModel_ptr)(TypicalImageBufId, ModelId, Mode);
}
void MpatFree(MIL_ID PatId)
{
	(*MpatFree_ptr)(PatId);
}
void MpatSetSpeed(MIL_ID ModelId, MIL_INT SpeedFactor)
{
	(*MpatSetSpeed_ptr)(ModelId, SpeedFactor);
}
MIL_ID MpatAllocRotatedModel(MIL_ID SystemId, MIL_ID SrcModelId, MIL_DOUBLE Angle, MIL_INT InterpolationMode, MIL_INT ModelType, MIL_ID *NewModelIdPtr)
{
	return (*MpatAllocRotatedModel_ptr)(SystemId, SrcModelId, Angle, InterpolationMode, ModelType, NewModelIdPtr);
}
MIL_INT MpatInquire(MIL_ID PatId, MIL_INT InquireType, void *UserVarPtr)
{
	return (*MpatInquire_ptr)(PatId, InquireType, UserVarPtr); 
}
void MpatSetPosition(MIL_ID ModelId, MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY)
{
	(*MpatSetPosition_ptr)(ModelId, OffX, OffY, SizeX, SizeY);
}
void MpatSave(MIL_CONST_TEXT_PTR FileName, MIL_ID ModelId)
{
	(*MpatSave_ptr)(FileName, ModelId);
}
void MpatCopy(MIL_ID ModelId, MIL_ID DestImageBufId, MIL_INT CopyMode)
{
	(*MpatCopy_ptr)(ModelId, DestImageBufId, CopyMode);
}
MIL_ID MpatRestore(MIL_ID SystemId, MIL_CONST_TEXT_PTR Filename, MIL_ID *ModelIdPtr)
{
	return (*MpatRestore_ptr)(SystemId, Filename, ModelIdPtr);
}

void McodeRead(MIL_ID CodeContextId, MIL_ID ImageBufId, MIL_ID CodeResultId)
{
	(*McodeRead_ptr)(CodeContextId, ImageBufId, CodeResultId);
}
void McodeModel(MIL_ID CodeContextId, MIL_INT Operation, MIL_INT CodeType, MIL_INT Instance, MIL_INT ControlFlag, MIL_ID *CodeModelIdPtr)
{
	(*McodeModel_ptr)(CodeContextId, Operation, CodeType, Instance, ControlFlag, CodeModelIdPtr);
}
void McodeGetResult(MIL_ID CodeResultId, MIL_INT ResultType, void *ResultPtr)
{
	(*McodeGetResult_ptr)(CodeResultId, ResultType, ResultPtr);
}
void McodeFree(MIL_ID ObjectId)
{
	(*McodeFree_ptr)(ObjectId);
}
void McodeControl(MIL_ID CodeId, MIL_INT ControlType, MIL_DOUBLE ControlValue)
{
	(*McodeControl_ptr)(CodeId, ControlType, ControlValue);
}
void McodeAllocResult(MIL_ID SystemId, MIL_INT ControlFlag, MIL_ID *CodeResultIdPtr)
{
	(*McodeAllocResult_ptr)(SystemId, ControlFlag, CodeResultIdPtr);
}
void McodeAlloc(MIL_ID SystemId, MIL_INT ContextType, MIL_INT ControlFlag, MIL_ID *CodeContextIdPtr)
{
	(*McodeAlloc_ptr)(SystemId, ContextType, ControlFlag, CodeContextIdPtr);
}

void MstrFree(MIL_ID ObjectId)
{
	(*MstrFree_ptr)(ObjectId);
}
void MstrControl(MIL_ID ContextId, MIL_INT Index, MIL_INT ControlType, MIL_DOUBLE ControlValue)
{
	(*MstrControl_ptr)(ContextId, Index, ControlType, ControlValue);
}
MIL_ID MstrAlloc(MIL_ID SystemId, MIL_INT ContextType, MIL_INT ControlFlag, MIL_ID *ObjectIdPtr)
{
	return (*MstrAlloc_ptr)(SystemId, ContextType, ControlFlag, ObjectIdPtr);
}
void MstrEditFont(MIL_ID ContextId, MIL_INT FontIndex, MIL_INT Operation, MIL_INT OperationMode, MIL_INT Param1, const void *Param2, const void *Param3)
{
	(*MstrEditFont_ptr)(ContextId, FontIndex, Operation, OperationMode, Param1, Param2, Param3);
}
MIL_INT MstrInquire(MIL_ID ContextId, MIL_INT Index, MIL_INT InquireType, void *UserVarPtr)
{
	return (*MstrInquire_ptr)(ContextId, Index, InquireType, UserVarPtr);
}
void MstrSave(MIL_CONST_TEXT_PTR FileName, MIL_ID ContextId, MIL_INT ControlFlag)
{
	(*MstrSave_ptr)(FileName, ContextId, ControlFlag);
}
MIL_ID MstrRestore(MIL_CONST_TEXT_PTR Filename, MIL_ID SystemId, MIL_INT ControlFlag, MIL_ID *ContextIdPtr)
{
	return (*MstrRestore_ptr)(Filename, SystemId, ControlFlag, ContextIdPtr);
}
void MstrSetConstraint(MIL_ID ContextId, MIL_INT StringIndex, MIL_INT CharPos, MIL_INT ConstraintType, const void *CharList)
{
	(*MstrSetConstraint_ptr)(ContextId, StringIndex, CharPos, ConstraintType, CharList);
}
void MstrPreprocess(MIL_ID ContextId, MIL_INT ControlFlag)
{
	(*MstrPreprocess_ptr)(ContextId, ControlFlag); 
}
void MstrRead(MIL_ID ContextId, MIL_ID TargetImageId, MIL_ID ResultId)
{
	(*MstrRead_ptr)(ContextId, TargetImageId, ResultId);
}
void MstrGetResult(MIL_ID ResultId, MIL_INT Index, MIL_INT ResultType, void *ResultArrayPtr)
{
	(*MstrGetResult_ptr)(ResultId, Index, ResultType, ResultArrayPtr);
}
void MstrDraw(MIL_ID GraphContId, MIL_ID ContextOrResultId, MIL_ID DestImageId, MIL_INT Operation, MIL_INT Index, const void *CharList, MIL_INT ControlFlag)
{
	(*MstrDraw_ptr)(GraphContId, ContextOrResultId, DestImageId, Operation, Index, CharList, ControlFlag);
}
MIL_ID MstrAllocResult(MIL_ID SystemId, MIL_INT ControlFlag, MIL_ID *ObjectIdPtr)
{
	return (*MstrAllocResult_ptr)(SystemId, ControlFlag, ObjectIdPtr);
}

void McalFree(MIL_ID CalibrationId)
{
	(*McalFree_ptr)(CalibrationId);
}
void McalList(MIL_ID CalibrationId, const MIL_DOUBLE *XPixArray, const MIL_DOUBLE *YPixArray, const MIL_DOUBLE *XWorldArray, const MIL_DOUBLE *YWorldArray, const MIL_DOUBLE *ZWorldArray, MIL_INT NumPoint, MIL_INT Operation, MIL_INT ControlFlag)
{
	(*McalList_ptr)(CalibrationId, XPixArray, YPixArray, XWorldArray, YWorldArray, ZWorldArray, NumPoint, Operation, ControlFlag);
}
MIL_ID McalAlloc(MIL_ID SystemId, MIL_INT Mode, MIL_INT ModeFlag, MIL_ID *CalibrationIdPtr)
{
	return (*McalAlloc_ptr)(SystemId, Mode, ModeFlag, CalibrationIdPtr);
}
MIL_INT McalInquire(MIL_ID CalibrationOrMilId, MIL_INT InquireType, void *UserVarPtr)
{
	return (*McalInquire_ptr)(CalibrationOrMilId, InquireType, UserVarPtr);
}
void MdigControlFeature(MIL_ID DigId, MIL_INT64 ControlType, MIL_CONST_TEXTW_PTR FeatureName, MIL_INT64 UserVarType, const void  *UserVarPtr)
{
	(*MdigControlFeature_ptr)(DigId, ControlType, FeatureName, UserVarType, UserVarPtr);
}

void MdigInquireFeature(MIL_ID DigId, MIL_INT64 InquireType, MIL_CONST_TEXTW_PTR FeatureName, MIL_INT64 UserVarType, void *UserVarPtr)
{
 	(*MdigInquireFeature_ptr)(DigId, InquireType, FeatureName, UserVarType, UserVarPtr);
}

MIL_INT MdigGetHookInfo(MIL_ID EventId, MIL_INT64 InfoType, void *UserVarPtr)
{
	return (*MdigGetHookInfo_ptr)(EventId, InfoType, UserVarPtr);
}
MIL_ID MthrAlloc(MIL_ID SystemId, MIL_INT64 ObjectType, MIL_INT64 ControlFlag, MIL_THREAD_FUNCTION_PTR ThreadFctPtr, void *UserDataPtr, MIL_ID *ThreadEventOrMutexIdPtr)
{
	return (*MthrAlloc_ptr)(SystemId, ObjectType, ControlFlag, ThreadFctPtr, UserDataPtr, ThreadEventOrMutexIdPtr);
}
void MthrControl(MIL_ID ThreadEventOrMutexId, MIL_INT64 ControlType, MIL_INT64 ControlValue)
{
	(*MthrControl_ptr)(ThreadEventOrMutexId, ControlType, ControlValue);
}

void MthrFree(MIL_ID ThreadEventorMutexId)
{
	(*MthrFree_ptr)(ThreadEventorMutexId);
}

MIL_INT MthrWait(MIL_ID ThreadOrEventId, MIL_INT64 WaitOption, MIL_INT *StatePtr)
{
	return (*MthrWait_ptr)(ThreadOrEventId, WaitOption, StatePtr);
}

void MsysHookFunction(MIL_ID SysId, MIL_INT HookType, MIL_SYS_HOOK_FUNCTION_PTR HookHandlerPtr, void *UserDataPtr)
{
	(*MsysHookFunction_ptr)(SysId, HookType, HookHandlerPtr, UserDataPtr);
}