#pragma once
#include "PInspAlgo_Lib.h"

namespace PIAL
{
	class PINSPALGO_API Geo_Point
	{

	public:
		double x;
		double y;

	public:
		Geo_Point() : x(0), y(0) {}
		Geo_Point(double a, double b) { x = a; y = b; }

		// basic operations
		inline Geo_Point operator+(const Geo_Point& r) const { return Geo_Point(x + r.x, y + r.y); }
		inline Geo_Point operator-(const Geo_Point& r) const { return Geo_Point(x - r.x, y - r.y); }
		inline Geo_Point operator*(const Geo_Point& r) const { return Geo_Point(x*r.x, y*r.y); }
		inline Geo_Point operator/(const Geo_Point& r) const { return Geo_Point(x / r.x, y / r.y); }
		inline Geo_Point operator*(const double s)  const { return Geo_Point(x*s, y*s); }
		inline Geo_Point operator/(const double s)  const { return Geo_Point(x / s, y / s); }
		inline Geo_Point operator-(const double s)  const { return Geo_Point(x - s, y - s); }
		inline Geo_Point operator+(const double s)  const { return Geo_Point(x + s, y + s); }

		inline bool operator==(const Geo_Point& r)  const { return  (x == r.x && y == r.y) ? true : false; }
		inline bool operator!=(const Geo_Point& r)  const { return  (x != r.x || y != r.y) ? true : false; }

		inline Geo_Point& operator+=(const Geo_Point& r) { x += r.x, y += r.y; return *this; }
		inline Geo_Point& operator-=(const Geo_Point& r) { x -= r.x, y -= r.y; return *this; }
		inline Geo_Point& operator*=(const Geo_Point& r) { x *= r.x, y *= r.y; return *this; }
		inline Geo_Point& operator/=(const Geo_Point& r) { x /= r.x, y /= r.y; return *this; }
		inline Geo_Point& operator*=(const double s) { x *= s, y *= s; return *this; }
		inline Geo_Point& operator/=(const double s) { x /= s, y /= s; return *this; }

	};

}