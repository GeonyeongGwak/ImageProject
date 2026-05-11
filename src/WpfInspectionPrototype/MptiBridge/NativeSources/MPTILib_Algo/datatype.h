#pragma once

namespace jsl
{
	template<typename T>
	class Point2d
	{
	public:
		Point2d():x(0.0), y(0.0) {}
		Point2d(T tmpX, T tmpY):x(tmpX), y(tmpY) {}
		virtual ~Point2d(){}
	public:
		bool operator<(Point2d<T> & rhs)
		{
			if(y<rhs.y)
				return true;
			else if(y==rhs.y)
			{
				if(x<rhs.x)
					return true;
				else
					return false;
			}
			else
				return false;
		}
		
		bool operator==(Point2d<T> & rhs)
		{
			if(y==rhs.y && x==rhs.x)
				return true;
			else
				return false;
		}
		
		Point2d<T> & operator=(Point2d<T> & rhs)
		{
			x = rhs.x;
			y = rhs.y;
			return *this;
		}
		T x;
		T y;
	};

	template<typename TX,typename TY,typename TZ>
	class Point3d
	{
	public:
		Point3d():x(0.0), y(0.0), z(0.0) {}
		Point3d(TX tmpX, TY tmpY, TZ tmpZ):x(tmpX), y(tmpY), z(tmpZ) {}
		virtual ~Point3d(){}
	public:
		bool operator<(Point3d<TX,TY,TZ> & rhs)
		{
			if(z<rhs.z)
				return true;
			else if(z==rhs.z)
			{
				if(y<rhs.y)
					return true;
				else if(y==rhs.y)
				{
					if(x<rhs.x)
						return true;
					else
						return true;
				}
				else
					return false;
			}
			else
				return false;
		}

		bool operator==(Point3d<TX,TY,TZ> & rhs)
		{
			if(z<rhs.z && y<rhs.y && x<rhs.x)
				return true;
			else
				return false;
		}
		
		Point3d<TX,TY,TZ> & operator=(Point3d<TX,TY,TZ> & rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			return *this;
		}

		TX x;
		TY y;
		TZ z;
	};

	template<typename T>
	class Rect
	{
	public:
		Rect():left(0.0), top(0.0), right(0.0), bottom(0.0) {}
		Rect(T l, T t, T r, T b):left(l), top(t), right(l), bottom(b) {}
		virtual ~Rect(){}
	public:
		T width()	{ return fabs(right - left); }
		T height()	{ return fabs(bottom - top); }
		bool operator<(Rect<T> & rhs)
		{
			if(top<rhs.top)
				return true;
			else if(top==rhs.top)
			{
				if(left<rhs.left)
					return true;
				else
					return false;
			}
			else
				return false;
		}
		T left;
		T right;
		T top;
		T bottom;
	};

	template<typename T>
	class Size
	{
	public:
		Size():cx(0.0), cy(0.0) {}
		Size(T tmpX, T tmpY):cx(tmpX), cy(tmpY) {}
		virtual ~Size(){}
	public:
		bool operator<(Size<T> & rhs)
		{
			if(cy<rhs.cy)
				return true;
			else if(cy==rhs.cy)
			{
				if(cx<rhs.cx)
					return true;
				else
					return false;
			}
			else
				return false;
		}

		T cx;
		T cy;
	};
}
