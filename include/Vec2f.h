#ifndef VECTOR_2_FLOAT_H
#define VECTOR_2_FLOAT_H

#include <cmath>
#include <iostream>

class Vec2f
{
	public:
		union
		{
			float V[2];
			struct
			{
				float x,y;
			};
		};

		Vec2f()
		{
			x = 0;
			y = 0;
		}
		Vec2f(float X,float Y)
		{
			x = X;
			y = Y;
		}
		Vec2f(float s)
		{
			x = s;
			y = s;
		}


		inline Vec2f& operator=(const Vec2f &rhs)
		{
			x = rhs.x;
			y = rhs.y;
			return *this;
		}
		inline Vec2f& operator*=(const Vec2f &param)
		{
			x *= param.x;
			y *= param.y;
			return *this;
		}
		inline Vec2f operator+ (const Vec2f& v) const
		{
			return Vec2f(x + v.x, y + v.y);
		}
		inline Vec2f operator- (const Vec2f& v) const
		{
			return Vec2f(x - v.x, y - v.y);
		}
		inline Vec2f operator* (const Vec2f& v) const
		{
			return Vec2f(x*x, y*y);
		}
		inline Vec2f operator* (float s) const
		{
			return Vec2f(x*s, y*s);
		}
		inline Vec2f operator/ (float s) const
		{
			return Vec2f(x/s, y/s);
		}
		inline Vec2f operator/ (const Vec2f& v) const
		{
			return Vec2f(x/v.x, y/v.y);
		}
		inline bool operator== (const Vec2f& v) const
		{
			return (x == v.x && y == v.y);
		}
		inline bool operator != (const Vec2f& v) const
		{
			return (x != v.x && y != v.y);
		}
		inline float Normalize()
		{
			float len = sqrtf(x*x + y*y);
			if (len > 0.0)
			{
				x /= len;
				y /= len;
			}
			return len;
		}
		inline float Dot(const Vec2f& v) const
		{
			return (x*v.x + y*v.y);
		}
		// STATIC FUNCTIONS
		static inline Vec2f Abs(const Vec2f& v)
		{
			return Vec2f (fabs(v.x), fabs(v.y));
		}


		friend std::ostream& operator<< (std::ostream &lhs,Vec2f &rhs) ;
		friend std::istream& operator>> (std::istream &lhs,Vec2f &rhs);
};

#endif
