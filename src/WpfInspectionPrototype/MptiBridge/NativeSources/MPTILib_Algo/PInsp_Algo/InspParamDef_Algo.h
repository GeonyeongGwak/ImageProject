#pragma once

#include "Align/InspParamDef_Align.h"
#include "AlignEdge/InspParamDef_AlignEdge.h"	//Edge
#include "Barcode/InspParamDef_Barcode.h"
#include "BGA/InspParamDef_BGA.h"
#include "Blob/InspParamDef_Blob.h"
#include "BodyBlob/InspParamDef_BodyBlob.h"
#include "Bridge/InspParamDef_Bridge.h"
#include "Bump/InspParamDef_Bump.h"
#include "ColorXY/InspParamDef_ColorXY.h"
#include "DisColor/InspParamDef_DisColor.h"
#include "Fillet/InspParamDef_Fillet.h"
#include "Foot/InspParamDef_Foot.h"
#include "GrayMean/InspParamDef_GrayMean.h"
#include "GrayDiff/InspParamDef_GrayDiff.h"
#include "Grid/InspParamDef_Grid.h"	//Color, Tab
#include "HeightDiff/InspParamDef_HeightDiff.h"
#include "HeightMean/InspParamDef_HeightMean.h"
#include "LeadLift/InspParamDef_LeadLift.h"
#include "LeadSearch/InspParamDef_LeadSearch.h"
#include "LeadSideSolder/InspParamDef_LeadSideSolder.h"
#include "LeadSolder/InspParamDef_LeadSolder.h"
#include "LeadTip/InspParamDef_LeadTip.h"
#include "Line/InspParamDef_Line.h"
#include "NGBlob/InspParamDef_NGBlob.h"
#include "OCR/InspParamDef_OCR.h"
#include "PadAlign/InspParamDef_PadAlign.h"	// BW
#include "PadBW/InspParamDef_PadBW.h"
#include "Pattern/InspParamDef_Pattern.h"
#include "POCR/InspParamDef_POCR.h"
#include "SolderCone/InspParamDef_SolderCone.h"
#include "Tilt/InspParamDef_Tilt.h"
#include "Volume/InspParamDef_Volume.h"
#include "Width/InspParamDef_Width.h"
#include "Wire/InspParamDef_Wire.h"
#include "BodyEdge/InspParamDef_BodyEdge.h"
#include "Tab/InspParamDef_Tab.h"
#include "Distance/InspParamDef_Distance.h"
#include "GWire/InspParamDef_GWire.h"
#include "PatternDiff/InspParamDef_PatternDiff.h"
#include "PadArray/InspParamDef_PadArray.h"


//shw Delete X#include "NewAlgo1/InspParamDef_NewAlgo1.h"
#include "ShapeX/InspParamDef_ShapeX.h"
 //#include "NewAlgo1/InspParamDef_NewAlgo1.h"

enum InspAlgoType
{
	eAlgoBW,
	eAlgoBlob,
	eAlgoAlign,
	eAlgoBody_Blob,
	eAlgoTilt,
	eAlgoOCR,
	eAlgoPattern,
	eAlgoColor,
	eAlgoGray_Mean,
	eAlgoHeight_Mean,
	eAlgoGray_Diff,
	eAlgoHeight_Diff,
	eAlgoLead_Search,
	eAlgoBridge,
	eAlgoLead_Tip,
	eAlgoLead_Lift,
	eAlgoLead_Solder,
	eAlgoLead_Color,
	eAlgoWidth,
	eAlgoVolume,
	eAlgoLead_SideSolder,
	eAlgoTab_Search,
	eAlgoTab,
	eAlgoColorBand_Search,
	eAlgoGrid,
	eAlgoLine,
	eAlgoEdge,
	eAlgoSolderCone,
	eAlgoColorXY,
	eAlgoAlignEdge,
	eAlgoPadAlign,
	eAlgoPOCR,
	eAlgoDisColor,
	eAlgoWire,
	eAlgoFoot,
	eAlgoBarcode,
	eAlgoFillet,
	eAlgoBGA,
	eAlgoBump,
	eAlgoNGBlob,
	eAlgoPadBW,
	eAlgoForeignOCV,
	eAlgoBodyEdge,
	eAlgoPackageThickness,
	eAlgoDistance,
	eAlgoGWire,
	eAlgoEdgePoint,
	eAlgoPatternDiff,
	eAlgoShapeX,
	eAlgoPadArray,
	eAlgoLQBGA,
	//shw Delete XeAlgoNewAlgo1,
	eAlgoNum,
};
enum InspAlgoType_SPCKind : unsigned long long
{
	eSPCAlgoBW = 2,
	eSPCAlgoBlob = 4,
	eSPCAlgoAlign = 8,
	eSPCAlgoBody_Blob = 16,
	eSPCAlgoTilt = 32,
	eSPCAlgoOCR = 64,
	eSPCAlgoPattern = 128,
	eSPCAlgoColor = 256,
	eSPCAlgoGray_Mean = 512,
	eSPCAlgoHeight_Mean = 1024,
	eSPCAlgoGray_Diff = 2048,
	eSPCAlgoHeight_Diff = 4096,
	eSPCAlgoBridge = 8192,
	eSPCAlgoLead_Tip = 16384,
	eSPCAlgoLead_Lift = 32768,
	eSPCAlgoLead_Solder = 65536,
	eSPCAlgoLead_Color = 131072,
	eSPCAlgoWidth = 262144,
	eSPCAlgoVolume = 524288,
	eSPCAlgoTab = 1048576,
	eSPCAlgoGrid = 2097152,
	eSPCAlgoLine = 4194304,
	eSPCAlgoColorXY = 8388608,
	eSPCAlgoEdge = 16777216,
	eSPCAlgoLeadSideSolder = 33554432,
	eSPCAlgoSolderCone = 67108864,
	eSPCAlgoAlignEdge = 134217728,
	eSPCAlgoPOCR = 268435456,
	eSPCAlgoPadAlign = 536870912,
	eSPCAlgoWire = 1073741824,
	eSPCAlgoFoot = 2147483648,
	eSPCAlgoBarcode = 4294967296,
	eSPCAlgoFillet = 8589934592,
	eSPCAlgoBGA = 17179869184,
	eSPCAlgoBump = 34359738368,
	eSPCAlgoPadBW = 68719476736,
	eSPCAlgoNGBlob = 137438953472,
	eSPCAlgoForeignOCV = 274877906944,
	eSPCAlgoBodyEdge = 549755813888,
	eSPCAlgoPackageThickness = 0x10000000000,
	eSPCAlgoDistance = 0x0000000020000000000,
	eSPCAlgoGWire = 0x0000000040000000000,
	eSPCAlgoEdgePoint = 0x0000000080000000000,
	eSPCAlgoPatternDiff = 0x00000000100000000000,
	eSPCAlgoShapeX = 0x00000000200000000000,
	eSPCAlgoTiltArray = 0x0000000400000000000,  // Tilt ROI Au¢¯e A¢¬AO (¨öCA| ¨úE¡Æi¢¬¢çAo X)           // ShapeX/TiltArr/PadArr
	eSPCAlgoPadArray = 0x0000000800000000000,
	eSPCAlgoBarcodeArray = 0x0000001000000000000,   // Mars Barcode Au¢¯e A¢¬AO (¨öCA| ¨úE¡Æi¢¬¢çAo X)   // PadArr/BarcodeArr/LQBGA
	eSPCAlgoLQBGA = 0x0000002000000000000,
	//shw Delete XeSPCAlgoNewAlgo1 = VALUE,
	eSPCAlgoNum,
};