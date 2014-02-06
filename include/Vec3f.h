#ifndef VECTOR_3_H
#define VECTOR_3_H

#include <math.h>
#include <iostream>
#include <sstream>
#include <Vec2f.h>

#define VEC3F(v)	(*reinterpret_cast<Vec3f*>(v))

class Vec3f
{
	public:
		Vec3f(){}
		Vec3f(float x,float y,float z){V[0] = x; V[1] = y; V[2] = z;}
		Vec3f(float v[]){ V[0] = v[0]; V[1] = v[1]; V[2] = v[2]; }
		Vec3f (float s){ V[0]=s,V[1]=s; V[2]=s;}
		Vec3f(Vec2f v){ V[0] = v.x; V[1] = v.y; V[2] = 0.0;}
//		Vec3f(Quatf q){ V[0] = q.x; V[1] = q.y; V[2] = q.z;}
		
		union
		{
			float V[3];
			struct
			{
				float x,y,z;
			};
		};

		inline float& operator[] (size_t i)
		{
			return V[i];
		}

		inline float operator[] (size_t i) const
		{
			return V[i];
		}

		inline operator Vec2f() const
		{
			return Vec2f(V[0],V[1]);
		}

		inline bool operator == (const Vec3f &v) const {return (V[0] == v.V[0]) && (V[1] == v.V[1]) && (V[2] == v.V[2]);}

		inline bool operator != (const Vec3f &v) const {return (V[0] != v.V[0]) || (V[1] != v.V[1]) || (V[2] != v.V[2]);}

		inline float& X() { return V[0];}
		inline float& Y() { return V[1];}
		inline float& Z() { return V[2];}

		inline float X() const{ return V[0];}
		inline float Y() const{ return V[1];}
		inline float Z() const{ return V[2];}

		inline void Set( float x, float y, float z)
		{
			V[0] = x;
			V[1] = y;
			V[2] = z;
		}

		inline  Vec3f operator * (const Vec3f &v) const
		{
			return Vec3f(V[0] * v.V[0], V[1] * v.V[1], V[2] * v.V[2]);
		}
		inline  Vec3f& operator *= (const Vec3f &v)
		{
			V[0] *= v.V[0];
			V[1] *= v.V[1];
			V[2] *= v.V[2];
			return *this;
		}

		inline const Vec3f operator * (const float s) const
		{
			return Vec3f(V[0] * s,V[1] * s,V[2] * s);
		}


		inline Vec3f& operator *= (float s) 
		{
			V[0] *= s;
			V[1] *= s;
			V[2] *= s;
			return *this;
		}

		inline const Vec3f operator + (const Vec3f &v) const
		{
			return Vec3f(V[0] + v.V[0],V[1] + v.V[1],V[2] + v.V[2]);
		}

		inline const Vec3f operator + (float s) const
		{
			return Vec3f(V[0] + s,V[1] + s,V[2] + s);
		}

		inline Vec3f& operator += (const Vec3f &v)
		{
			V[0] += v.V[0];
			V[1] += v.V[1];
			V[2] += v.V[2];
			return *this;
		}

		inline Vec3f& operator += (const float s)
		{
			V[0] += s;
			V[1] += s;
			V[2] += s;
			return *this;
		}

		inline const Vec3f operator - (const Vec3f &v) const
		{
			return Vec3f(V[0] - v.V[0],V[1] - v.V[1],V[2] - v.V[2]);
		}

		inline const Vec3f operator - (const float s) const
		{
			return Vec3f(V[0] - s,V[1] - s,V[2] - s);
		}

		inline Vec3f operator - () const
		{
			return Vec3f(-V[0],-V[1],-V[2]);
		}

		inline Vec3f& operator -= (const Vec3f &v)
		{
			V[0] -= v.V[0];
			V[1] -= v.V[1];
			V[2] -= v.V[2];
			return *this;
		}

		inline Vec3f& operator -= (float s)
		{
			V[0] -= s;
			V[1] -= s;
			V[2] -= s;
			return *this;
		}

		inline const Vec3f operator / (const Vec3f &v) const
		{
			return Vec3f(V[0]/v.V[0], V[1]/v.V[1], V[2]/v.V[2]);
		}
		inline const Vec3f operator /= (const Vec3f &v)
		{
			V[0] /= v.V[0];
			V[1] /= v.V[1];
			V[2] /= v.V[2];
			return *this;
		}
		inline const Vec3f operator / (float s)
		{
			return Vec3f(V[0] / s,V[1] / s,V[2] / s);
		}

		inline Vec3f& operator /= (float s)
		{
			V[0] /= s;
			V[1] /= s;
			V[2] /= s;
			return *this;
		}

		inline float Length()const
		{
			return sqrtf(V[0]*V[0] + V[1]*V[1] + V[2]*V[2]);
		}

		inline float LengthSqr() const
		{
			return V[0] * V[0] + V[1] * V[1] + V[2] * V[2];
		}

		inline float Normalize()
		{
			float length = Length();
			if (length > 0)
			{
				float inv = 1.0f/length;
				V[0] *= inv;
				V[1] *= inv;
				V[2] *= inv;
			}
			return length;
		}

		inline float Dot(const Vec3f &v) const
		{
			return (V[0] * v.V[0]) + (V[1] * v.V[1]) + (V[2] * v.V[2]);
		}
		inline const Vec3f Cross(const Vec3f &v) const
		{
			return Vec3f(V[1] * v.V[2] - V[2] * v.V[1],
					V[2] * v.V[0] - V[0] * v.V[2],
					V[0] * v.V[1] - V[1] * v.V[0]);
		}
		inline Vec3f Project(const Vec3f &v) const
		{
			float tmp = (V[0]*v.V[0] + V[1]*v.V[1] + V[2]*v.V[2])/(V[0]*V[0] + V[1]*V[1] + V[2]*V[2]);
			return Vec3f(V[0]*tmp, V[1]*tmp, V[2]*tmp);
		}
		inline std::string ToString() const
		{
			std::stringstream s;
			s << V[0] << " " << V[1] << " " << V[2];
			return s.str();
		}

		friend std::ostream& operator<< (std::ostream &lhs,Vec3f &rhs);
		friend std::istream& operator>> (std::istream &lhs,Vec3f &rhs);
		/* STATIC FUNCTIONS */
		static inline Vec3f Abs(const Vec3f &v)
		{
			return Vec3f(fabs(v.V[0]), fabs(v.V[1]), fabs(v.V[2]));
		}
};

#endif
