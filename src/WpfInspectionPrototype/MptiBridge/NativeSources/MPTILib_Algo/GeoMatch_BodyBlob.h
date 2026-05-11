#pragma once

#include "geomatch_pattern.h"

class GeoMatch_BodyBlob : public GeoMatch_Pattern
{
public:
	GeoMatch_BodyBlob(void);
	virtual ~GeoMatch_BodyBlob(void);

public:
	virtual void FindModel(GeoModel & model, GeoResult & result, int x, int y);
	virtual void FindModelAtPos(GeoModel & model, GeoResult & result, float x, float y);
};

