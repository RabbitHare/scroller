#ifndef MATRIX_4X4_H
#define MATRIX_4X4_H

#ifndef PI
#define PI 3.1415926535
#endif

#include <math.h>
#include <sstream>
#include <Vec3f.h>
#include <Vec4f.h>
#include <Mat3f.h>


class Mat4f
{
	public:
		float M[4][4];

		Mat4f()
		{
			/*
			M[0][0] = 1;M[1][0] = 0;M[2][0] = 0;M[3][0] = 0;
			M[0][1] = 0;M[1][1] = 1;M[2][1] = 0;M[3][1] = 0;
			M[0][2] = 0;M[1][2] = 0;M[2][2] = 1;M[3][2] = 0;
			M[0][3] = 0;M[1][3] = 0;M[2][3] = 0;M[3][3] = 1;
			*/
		}
		Mat4f(float m11,float m21,float m31,float m41,
				float m12,float m22,float m32,float m42,
				float m13,float m23,float m33,float m43,
				float m14,float m24,float m34,float m44)
		{
			M[0][0] = m11;M[1][0] = m21;M[2][0] = m31;M[3][0] = m41;
			M[0][1] = m12;M[1][1] = m22;M[2][1] = m32;M[3][1] = m42;
			M[0][2] = m13;M[1][2] = m23;M[2][2] = m33;M[3][2] = m43;
			M[0][3] = m14;M[1][3] = m24;M[2][3] = m34;M[3][3] = m44;
		}
		Mat4f(const Vec3f& v)
		{
			M[0][0]=v.V[0];	M[1][0] = 0;	M[2][0] = 0;	M[3][0] = 0;
			M[0][1] = 0;	M[1][1]=v.V[1];	M[2][1] = 0;	M[3][1] = 0;
			M[0][2] = 0;	M[1][2] = 0;	M[2][2]=v.V[2];	M[3][2] = 0;
			M[0][3] = 0;	M[1][3] = 0;	M[2][3] = 0;	M[3][3] = 1.0f;
		}
		Mat4f(const Mat4f &m)
		{
			M[0][0] = m.M[0][0];M[1][0] = m.M[1][0];M[2][0] = m.M[2][0];M[3][0] = m.M[3][0];
			M[0][1] = m.M[0][1];M[1][1] = m.M[1][1];M[2][1] = m.M[2][1];M[3][1] = m.M[3][1];
			M[0][2] = m.M[0][2];M[1][2] = m.M[1][2];M[2][2] = m.M[2][2];M[3][2] = m.M[3][2];
			M[0][3] = m.M[0][3];M[1][3] = m.M[1][3];M[2][3] = m.M[2][3];M[3][3] = m.M[3][3];
		}
		Mat4f(const Mat3f &m)
		{
			M[0][0] = m.M[0][0];M[1][0] = m.M[1][0];M[2][0] = m.M[2][0];M[3][0] = 0.0;
			M[0][1] = m.M[0][1];M[1][1] = m.M[1][1];M[2][1] = m.M[2][1];M[3][1] = 0.0;
			M[0][2] = m.M[0][2];M[1][2] = m.M[1][2];M[2][2] = m.M[2][2];M[3][2] = 0.0;
			M[0][3] =     0.0  ;M[1][3] =	  0.0  ;M[2][3] =     0.0  ;M[3][3] = 1.0;
		}

		inline operator Mat3f() const
		{
			return Mat3f(M[0][0],M[1][0],M[2][0],
					M[0][1],M[1][1],M[2][1],
					M[0][2],M[1][2],M[2][2]);
		}

		inline Mat4f& operator = (const Mat4f& m)
		{
			M[0][0] = m.M[0][0];M[1][0] = m.M[1][0];M[2][0] = m.M[2][0];M[3][0] = m.M[3][0];
			M[0][1] = m.M[0][1];M[1][1] = m.M[1][1];M[2][1] = m.M[2][1];M[3][1] = m.M[3][1];
			M[0][2] = m.M[0][2];M[1][2] = m.M[1][2];M[2][2] = m.M[2][2];M[3][2] = m.M[3][2];
			M[0][3] = m.M[0][3];M[1][3] = m.M[1][3];M[2][3] = m.M[2][3];M[3][3] = m.M[3][3];
			return *this;
		}



		void Set(float m11,float m21,float m31,float m41,
				float m12,float m22,float m32,float m42,
				float m13,float m23,float m33,float m43,
				float m14,float m24,float m34,float m44)
		{
			M[0][0] = m11;M[1][0] = m21;M[2][0] = m31;M[3][0] = m41;
			M[0][1] = m12;M[1][1] = m22;M[2][1] = m32;M[3][1] = m42;
			M[0][2] = m13;M[1][2] = m23;M[2][2] = m33;M[3][2] = m43;
			M[0][3] = m14;M[1][3] = m24;M[2][3] = m34;M[3][3] = m44;
		}

		const bool operator == (const Mat4f &m) const
		{
			if ((M[0][0] != m.M[0][0]) || (M[1][0] != m.M[1][0]) || (M[2][0] != m.M[2][0]) || (M[3][0] != m.M[3][0]) ||
					(M[0][1] != m.M[0][1]) || (M[1][1] != m.M[1][1]) || (M[2][1] != m.M[2][1]) || (M[3][1] != m.M[3][1]) ||
					(M[0][2] != m.M[0][2]) || (M[1][2] != m.M[1][2]) || (M[2][2] != m.M[2][2]) || (M[3][2] != m.M[3][2]) ||
					(M[0][3] != m.M[0][3]) || (M[1][3] != m.M[1][3]) || (M[2][3] != m.M[2][3]) || (M[3][3] != m.M[3][3]))
				return false;
			else 
				return true;
		}
		bool operator != (const Mat4f &m) const
		{
			if ((M[0][0] != m.M[0][0]) || (M[1][0] != m.M[1][0]) || (M[2][0] != m.M[2][0]) || (M[3][0] != m.M[3][0]) ||
					(M[0][1] != m.M[0][1]) || (M[1][1] != m.M[1][1]) || (M[2][1] != m.M[2][1]) || (M[3][1] != m.M[3][1]) ||
					(M[0][2] != m.M[0][2]) || (M[1][2] != m.M[1][2]) || (M[2][2] != m.M[2][2]) || (M[3][2] != m.M[3][2]) ||
					(M[0][3] != m.M[0][3]) || (M[1][3] != m.M[1][3]) || (M[2][3] != m.M[2][3]) || (M[3][3] != m.M[3][3]))
				return true;
			else 
				return false;
		}


		inline const Mat4f operator + (const Mat4f &m) const
		{
			return Mat4f(M[0][0] + m.M[0][0],M[1][0] + m.M[1][0],
					M[2][0] + m.M[2][0],M[3][0] + m.M[3][0],
					M[0][1] + m.M[0][1],M[1][1] + m.M[1][1],
					M[2][1] + m.M[2][1],M[3][1] + m.M[3][1],
					M[0][2] + m.M[0][2],M[1][2] + m.M[1][2],
					M[2][2] + m.M[2][2],M[3][2] + m.M[3][2],
					M[0][3] + m.M[0][3],M[1][3] + m.M[1][3],
					M[2][3] + m.M[2][3],M[3][3] + m.M[3][3]);
		}

		inline Mat4f& operator += (const Mat4f &m)
		{

			M[0][0] += m.M[0][0];M[1][0] += m.M[1][0]; M[2][0] += m.M[2][0];M[3][0] += m.M[3][0];
			M[0][1] += m.M[0][1];M[1][1] += m.M[1][1]; M[2][1] += m.M[2][1];M[3][1] += m.M[3][1];
			M[0][2] += m.M[0][2];M[1][2] += m.M[1][2]; M[2][2] += m.M[2][2];M[3][2] += m.M[3][2];
			M[0][3] += m.M[0][3];M[1][3] += m.M[1][3]; M[2][3] += m.M[2][3];M[3][3] += m.M[3][3];
			return *this;
		}

		inline const Mat4f operator - (const Mat4f &m) const
		{
			return Mat4f(M[0][0] - m.M[0][0],M[1][0] - m.M[1][0],
					M[2][0] - m.M[2][0],M[3][0] - m.M[3][0],
					M[0][1] - m.M[0][1],M[1][1] - m.M[1][1],
					M[2][1] - m.M[2][1],M[3][1] - m.M[3][1],
					M[0][2] - m.M[0][2],M[1][2] - m.M[1][2],
					M[2][2] - m.M[2][2],M[3][2] - m.M[3][2],
					M[0][3] - m.M[0][3],M[1][3] - m.M[1][3],
					M[2][3] - m.M[2][3],M[3][3] - m.M[3][3]);
		}

		inline Mat4f& operator -= (const Mat4f &m)
		{

			M[0][0] -= m.M[0][0];M[1][0] -= m.M[1][0]; M[2][0] -= m.M[2][0];M[3][0] -= m.M[3][0];
			M[0][1] -= m.M[0][1];M[1][1] -= m.M[1][1]; M[2][1] -= m.M[2][1];M[3][1] -= m.M[3][1];
			M[0][2] -= m.M[0][2];M[1][2] -= m.M[1][2]; M[2][2] -= m.M[2][2];M[3][2] -= m.M[3][2];
			M[0][3] -= m.M[0][3];M[1][3] -= m.M[1][3]; M[2][3] -= m.M[2][3];M[3][3] -= m.M[3][3];
			return *this;
		}

		/*Multiply matrix this.M * m.M*/
		const Mat4f operator * (const Mat4f &m)const
		{
			Mat4f p;

			for(int r = 0;r < 4;r++)
				for(int c = 0;c < 4;c++)
					p.M[c][r] = (M[0][r]*m.M[c][0]) + (M[1][r]*m.M[c][1]) + (M[2][r]*m.M[c][2]) + (M[3][r]*m.M[c][3]);
			return p;
		}

		inline const Mat4f operator * (float s) const
		{
			return Mat4f(M[0][0] * s,M[1][0] * s,
					M[2][0] * s,M[3][0] * s,
					M[0][1] * s,M[1][1] * s,
					M[2][1] * s,M[3][1] * s,
					M[0][2] * s,M[1][2] * s,
					M[2][2] * s,M[3][2] * s,
					M[0][3] * s,M[1][3] * s,
					M[2][3] * s,M[3][3] * s);
		}

		inline Mat4f& operator *= (float s)
		{

			M[0][0] *= s;M[1][0] *= s; M[2][0] *= s;M[3][0] *= s;
			M[0][1] *= s;M[1][1] *= s; M[2][1] *= s;M[3][1] *= s;
			M[0][2] *= s;M[1][2] *= s; M[2][2] *= s;M[3][2] *= s;
			M[0][3] *= s;M[1][3] *= s; M[2][3] *= s;M[3][3] *= s;
			return *this;
		}

		Mat4f& operator *= (const Mat4f &m)
		{
			float tmp[4][4];
			for(int r = 0;r < 4;r++)
				for(int c = 0;c < 4;c++)
					tmp[c][r] = (M[0][r]*m.M[c][0]) + (M[1][r]*m.M[c][1]) + (M[2][r]*m.M[c][2]) + (M[3][r]*m.M[c][3]);

			memcpy (M,tmp,64);
			return *this;
		}
		inline Vec3f operator* (const Vec3f& param) const
		{
			Vec3f ret;
			ret.V[0] = M[0][0]*param.V[0] + M[1][0]*param.V[1] + M[2][0]*param.V[2] + M[3][0]; 
			ret.V[1] = M[0][1]*param.V[0] + M[1][1]*param.V[1] + M[2][1]*param.V[2] + M[3][1];
			ret.V[2] = M[0][2]*param.V[0] + M[1][2]*param.V[1] + M[2][2]*param.V[2] + M[3][2];
			return ret;
		}   
		inline Vec4f operator* (const Vec4f& param) const
		{
			Vec4f ret;
			ret.V[0] = M[0][0]*param.V[0] + M[1][0]*param.V[1] + M[2][0]*param.V[2] + M[3][0]*param.V[3]; 
			ret.V[1] = M[0][1]*param.V[0] + M[1][1]*param.V[1] + M[2][1]*param.V[2] + M[3][1]*param.V[3];
			ret.V[2] = M[0][2]*param.V[0] + M[1][2]*param.V[1] + M[2][2]*param.V[2] + M[3][2]*param.V[3];
			ret.V[3] = M[0][3]*param.V[0] + M[1][3]*param.V[1] + M[2][3]*param.V[2] + M[3][3]*param.V[3];
			return ret;
		}   
		


		inline void MakeIdentity()
		{
			M[0][0] = 1;M[1][0] = 0;M[2][0] = 0;M[3][0] = 0;
			M[0][1] = 0;M[1][1] = 1;M[2][1] = 0;M[3][1] = 0;
			M[0][2] = 0;M[1][2] = 0;M[2][2] = 1;M[3][2] = 0;
			M[0][3] = 0;M[1][3] = 0;M[2][3] = 0;M[3][3] = 1;
		}
		inline void Transpose()
		{
			float tmp[4][4];
			for(int32_t r=0; r < 4; r++)
			{
				tmp[0][r] = M[r][0];
				tmp[1][r] = M[r][1];
				tmp[2][r] = M[r][2];
				tmp[3][r] = M[r][3];
			}
			memcpy(M,tmp,64);
		}

		inline Vec3f GetTranslation() const
		{
			return Vec3f(M[3][0],M[3][1],M[3][2]);
		}
		inline Vec3f GetOrigin () const
		{
			return GetTranslation ();
		}

		inline void SetTranslation(const Vec3f &v)
		{
			M[3][0] = v.V[0];
			M[3][1] = v.V[1];
			M[3][2] = v.V[2];
		}
		inline void SetOrigin (const Vec3f &v)
		{
			SetTranslation (v);
		}
		inline void SetScale(const Vec3f& v)
		{
			M[0][0]*=v.V[0]; M[1][0]*=v.V[1]; M[2][0]*=v.V[2];
			M[0][1]*=v.V[0]; M[1][1]*=v.V[1]; M[2][1]*=v.V[2];
			M[0][2]*=v.V[0]; M[1][2]*=v.V[1]; M[2][2]*=v.V[2];
		}

		inline void SetRotationT(const Mat3f& rot)
		{

			M[0][0] = rot.M[0][0]; M[1][0] = rot.M[0][1];M[2][0] = rot.M[0][2];
			M[0][1] = rot.M[1][0]; M[1][1] = rot.M[1][1];M[2][1] = rot.M[1][2];
			M[0][2] = rot.M[2][0]; M[1][2] = rot.M[2][1];M[2][2] = rot.M[2][2];
		}
		inline void Merge (const Mat3f& m)
		{
			M[0][0]=m.M[0][0]; M[1][0]=m.M[1][0]; M[2][0]=m.M[2][0];
			M[0][1]=m.M[0][1]; M[1][1]=m.M[1][1]; M[2][1]=m.M[2][1];
			M[0][2]=m.M[0][2]; M[1][2]=m.M[1][2]; M[2][2]=m.M[2][2];
		}

		const char* Debug() const
		{
			std::ostringstream oss;

			for(int r = 0;r < 4;r++)
				oss<<"|"<<M[0][r]<<","<<M[1][r]<<","<<M[2][r]<<","<<M[3][r]<<"|\n";

			return oss.str().c_str();
		}
		friend std::ostream& operator<< (std::ostream &lhs,Mat4f &rhs);

		friend std::istream& operator>> (std::istream &lhs,Mat4f &rhs);

};

#endif
