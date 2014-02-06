#ifndef VECTOR_4_H
#define VECTOR_4_H

#include <math.h>
#include <iostream>
#include "Vec3f.h"

#define VEC4F(v)	(*reinterpret_cast<Vec4f*>(v))

class Vec4f
{
	public:
		float V[4];

		Vec4f(){V[0] = 0; V[1] = 0; V[2] = 0; V[3] = 0;}
		Vec4f(float x,float y,float z, float w){V[0] = x; V[1] = y; V[2] = z; V[3] = w;}
		Vec4f(Vec3f v){ V[0] = v.V[0]; V[1] = v.V[1]; V[2] = v.V[2]; V[3] = 1.0;}
		
		inline operator Vec3f() const
		{
			/*
			float s;
			if(V[3] == 0.0) s = 1000000;
			else s = 1.0/V[3];
			return Vec3f(V[0]*s,V[1]*s,V[2]*s);
			*/
			return Vec3f(V[0],V[1],V[2]);
		}

		inline bool operator == (const Vec4f &v) const {return (V[0] == v.V[0]) && (V[1] == v.V[1]) && (V[2] == v.V[2]) && (V[3] == v.V[3]);}

		inline bool operator != (const Vec3f &v) const {return (V[0] != v.V[0]) || (V[1] != v.V[1]) || (V[2] != v.V[2]) || (V[3] != v.V[3]);}

		inline float& X() { return V[0];}
		inline float& Y() { return V[1];}
		inline float& Z() { return V[2];}
		inline float& W() { return V[3];}
		
		inline float X() const{ return V[0];}
		inline float Y() const{ return V[1];}
		inline float Z() const{ return V[2];}
		inline float W() const{ return V[3];}

		inline  Vec4f operator * (const Vec4f &v) const
		{
			return Vec4f(V[0] * v.V[0], V[1] * v.V[1], V[2] * v.V[2], V[3] * v.V[3]);
		}
		inline  Vec4f& operator *= (const Vec4f &v)
		{
			V[0] *= v.V[0];
			V[1] *= v.V[1];
			V[2] *= v.V[2];
			V[3] *= v.V[3];
			return *this;
		}

		inline const Vec4f operator * (const float &s) const
		{
			return Vec4f(V[0] * s,V[1] * s,V[2] * s, V[3] * s);
		}


		inline Vec4f& operator *= (float s) 
		{
			V[0] *= s;
			V[1] *= s;
			V[2] *= s;
			V[3] *= s;
			return *this;
		}

		inline const Vec4f operator + (const Vec4f &v) const
		{
			return Vec4f(V[0] + v.V[0],V[1] + v.V[1],V[2] + v.V[2], V[3] + v.V[3]);
		}

		inline const Vec4f operator + (float &s) const
		{
			return Vec4f(V[0] + s,V[1] + s,V[2] + s,V[3] + s);
		}

		inline Vec4f& operator += (const Vec4f &v)
		{
			V[0] += v.V[0];
			V[1] += v.V[1];
			V[2] += v.V[2];
			V[3] += v.V[3];
			return *this;
		}

		inline Vec4f& operator += (const float &s)
		{
			V[0] += s;
			V[1] += s;
			V[2] += s;
			V[3] += s;
			return *this;
		}

		inline const Vec4f operator - (const Vec4f &v) const
		{
			return Vec4f(V[0] - v.V[0],V[1] - v.V[1],V[2] - v.V[2],V[3] - v.V[3]);
		}

		inline const Vec4f operator - (float &s) const
		{
			return Vec4f(V[0] - s,V[1] - s,V[2] - s,V[3] - s);
		}

		inline const Vec4f operator - ()
		{
			 return Vec4f(-V[0],-V[1],-V[2],-V[3]);
		}

		inline Vec4f& operator -= (const Vec4f &v)
		{
			V[0] -= v.V[0];
			V[1] -= v.V[1];
			V[2] -= v.V[2];
			V[3] -= v.V[3];
			return *this;
		}

		inline Vec4f& operator -= (float &s)
		{
			V[0] -= s;
			V[1] -= s;
			V[2] -= s;
			V[3] -= s;
			return *this;
		}

		inline const Vec4f operator / (const Vec4f &v) const
		{
			return Vec4f(V[0]/v.V[0], V[1]/v.V[1], V[2]/v.V[2], V[3]/v.V[3]);
		}
		inline const Vec4f operator /= (const Vec4f &v)
		{
			V[0] /= v.V[0];
			V[1] /= v.V[1];
			V[2] /= v.V[2];
			V[3] /= v.V[3];
			return *this;
		}
		inline const Vec4f operator / (float &s)
		{
			return Vec4f(V[0] / s,V[1] / s,V[2] / s, V[3]/s);
		}

		inline Vec4f& operator /= (float &s)
		{
			V[0] /= s;
			V[1] /= s;
			V[2] /= s;
			V[3] /= s;
			return *this;
		}
		inline Vec4f DivideByW() const
		{
			float s;
			if(V[3] == 0.0) s = 1.0;
			else s = 1.0/V[3];

			return Vec4f(V[0]*s,V[1]*s,V[2]*s,1.0);
		}
		/*
		inline float Length()const
		{
			return sqrtf(V[0] * V[0]+V[1] * V[1]+V[2] * V[2]);
		}

		inline float LengthSqr() const
		{
			return V[0] * V[0] + V[1] * V[1] + V[2] * V[2];
		}

		inline Vec3f& normalize()
		{
			float length = Length();
			if (length > 0)
			{
				float inv = 1.0f/length;
				V[0] *= inv;
				V[1] *= inv;
				V[2] *= inv;
			}
			return *this;
		}
		
		
		inline float dot(const Vec3f &v) const
		{
			return (V[0] * v.V[0]) + (V[1] * v.V[1]) + (V[2] * v.V[2]);
		}
		inline const Vec3f cross(const Vec3f &v) const
		{
			return Vec3f(V[1] * v.V[2] - V[2] * v.V[1],
					V[2] * v.V[0] - V[0] * v.V[2],
					V[0] * v.V[1] - V[1] * v.V[0]);
		}
		inline Vec3f project(const Vec3f &v) const
		{
			float tmp = (V[0]*v.V[0] + V[1]*v.V[1] + V[2]*v.V[2])/(V[0]*V[0] + V[1]*V[1] + V[2]*V[2]);
			return Vec3f(V[0]*tmp, V[1]*tmp, V[2]*tmp);
		}
		*/
		friend std::ostream& operator<< (std::ostream &lhs,Vec4f &rhs);
		friend std::istream& operator>> (std::istream &lhs,Vec4f &rhs);
};

#endif
