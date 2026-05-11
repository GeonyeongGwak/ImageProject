#pragma once

namespace ext
{
class Log
{
public:
	static void add(CString sLog, int nLogLv = 0);
};
enum m_eLogLv
{
	None = 0,
	Lv1,
	Lv2,
	Lv3,
	Lv4,
	Fiducial,
	Teaching,
	FOV,
	TactTime,
	Foreign,
	Partition,
	AI_Module,
	OffEdit,
	Cali,
	TrendLine,
	ExBarcode,
	ALL,
	OCR,
	Sticker,
	Check3D,
	PolygonGerber,
	MultiProcess,
};
}

