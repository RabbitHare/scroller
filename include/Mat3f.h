
#ifndef __MATRIX_3X3_H__
#define __MATRIX_3X3_H__

#ifndef PI
#define PI 3.1415926535
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sstream>
#include "Vec3f.h"
#include "Vec4f.h"


class Mat3f
{
	public:
		float M[3][3];

		Mat3f()
		{
			/*
			M[0][0] = 1;M[1][0] = 0;M[2][0] = 0;
			M[0][1] = 0;M[1][1] = 1;M[2][1] = 0;
			M[0][2] = 0;M[1][2] = 0;M[2][2] = 1;
			*/
		}
		Mat3f(float m11,float m21,float m31,
				float m12,float m22,float m32,
				float m13,float m23,float m33)
		{
			M[0][0] = m11;M[1][0] = m21;M[2][0] = m31;
			M[0][1] = m12;M[1][1] = m22;M[2][1] = m32;
			M[0][2] = m13;M[1][2] = m23;M[2][2] = m33;
		}
		Mat3f(const Vec3f& v)
		{
			M[0][0]=v.V[0];	M[1][0] = 0;	M[2][0] = 0;	
			M[0][1] = 0;	M[1][1]=v.V[1];	M[2][1] = 0;	
			M[0][2] = 0;	M[1][2] = 0;	M[2][2]=v.V[2];	
		}
		Mat3f(const Mat3f &m)
		{
			M[0][0] = m.M[0][0];M[1][0] = m.M[1][0];M[2][0] = m.M[2][0];
			M[0][1] = m.M[0][1];M[1][1] = m.M[1][1];M[2][1] = m.M[2][1];
			M[0][2] = m.M[0][2];M[1][2] = m.M[1][2];M[2][2] = m.M[2][2];
		}

		inline Mat3f& operator = (const Mat3f& m)
		{
			M[0][0] = m.M[0][0];M[1][0] = m.M[1][0];M[2][0] = m.M[2][0];
			M[0][1] = m.M[0][1];M[1][1] = m.M[1][1];M[2][1] = m.M[2][1];
			M[0][2] = m.M[0][2];M[1][2] = m.M[1][2];M[2][2] = m.M[2][2];
			return *this;
		}



		void Set(float m11,float m21,float m31,
				float m12,float m22,float m32,
				float m13,float m23,float m33)
		{
			M[0][0] = m11;M[1][0] = m21;M[2][0] = m31;
			M[0][1] = m12;M[1][1] = m22;M[2][1] = m32;
			M[0][2] = m13;M[1][2] = m23;M[2][2] = m33;
		}

		const bool operator == (const Mat3f &m) const
		{
			if ((M[0][0] != m.M[0][0]) || (M[1][0] != m.M[1][0]) || (M[2][0] != m.M[2][0]) ||
					(M[0][1] != m.M[0][1]) || (M[1][1] != m.M[1][1]) || (M[2][1] != m.M[2][1]) ||
					(M[0][2] != m.M[0][2]) || (M[1][2] != m.M[1][2]) || (M[2][2] != m.M[2][2]))
				return false;
			else 
				return true;
		}

		const bool operator != (const Mat3f &m) const
		{
			if ((M[0][0] != m.M[0][0]) || (M[1][0] != m.M[1][0]) || (M[2][0] != m.M[2][0]) ||
					(M[0][1] != m.M[0][1]) || (M[1][1] != m.M[1][1]) || (M[2][1] != m.M[2][1]) ||
					(M[0][2] != m.M[0][2]) || (M[1][2] != m.M[1][2]) || (M[2][2] != m.M[2][2]))
				return true;
			else 
				return false;
		}


		inline const Mat3f operator + (const Mat3f &m) const
		{
			return Mat3f(M[0][0] + m.M[0][0],M[1][0] + m.M[1][0],M[2][0] + m.M[2][0],
					M[0][1] + m.M[0][1],M[1][1] + m.M[1][1],M[2][1] + m.M[2][1],
					M[0][2] + m.M[0][2],M[1][2] + m.M[1][2],M[2][2] + m.M[2][2]);
		}

		inline Mat3f& operator += (const Mat3f &m)
		{

			M[0][0] += m.M[0][0];M[1][0] += m.M[1][0]; M[2][0] += m.M[2][0];
			M[0][1] += m.M[0][1];M[1][1] += m.M[1][1]; M[2][1] += m.M[2][1];
			M[0][2] += m.M[0][2];M[1][2] += m.M[1][2]; M[2][2] += m.M[2][2];
			return *this;
		}

		inline const Mat3f operator - (const Mat3f &m) const
		{
			return Mat3f(M[0][0] - m.M[0][0],M[1][0] - m.M[1][0],M[2][0] - m.M[2][0],
					M[0][1] - m.M[0][1],M[1][1] - m.M[1][1],M[2][1] - m.M[2][1],
					M[0][2] - m.M[0][2],M[1][2] - m.M[1][2],M[2][2] - m.M[2][2]);
		}

		inline Mat3f& operator -= (const Mat3f &m)
		{

			M[0][0] -= m.M[0][0];M[1][0] -= m.M[1][0]; M[2][0] -= m.M[2][0];
			M[0][1] -= m.M[0][1];M[1][1] -= m.M[1][1]; M[2][1] -= m.M[2][1];
			M[0][2] -= m.M[0][2];M[1][2] -= m.M[1][2]; M[2][2] -= m.M[2][2];
			return *this;
		}

		/*Multiply matrix this.M * m.M*/
		const Mat3f operator * (const Mat3f &m)const
		{
			Mat3f p;

			for(int r = 0;r < 3;r++)
				for(int c = 0;c < 3;c++)
					p.M[c][r] = (M[0][r]*m.M[c][0]) + (M[1][r]*m.M[c][1]) + (M[2][r]*m.M[c][2]);
			return p;
		}

		inline const Mat3f operator * (float s) const
		{
			return Mat3f(M[0][0] * s,M[1][0] * s,M[2][0] * s,
					M[0][1] * s,M[1][1] * s,M[2][1] * s,
					M[0][2] * s,M[1][2] * s,M[2][2] * s);
		}

		inline Mat3f& operator *= (float s)
		{

			M[0][0] *= s;M[1][0] *= s; M[2][0] *= s;
			M[0][1] *= s;M[1][1] *= s; M[2][1] *= s;
			M[0][2] *= s;M[1][2] *= s; M[2][2] *= s;
			return *this;
		}

		Mat3f& operator *= (const Mat3f &m)
		{
			float tmp[3][3];
			for(int r = 0;r < 4;r++)
				for(int c = 0;c < 4;c++)
					tmp[c][r] = (M[0][r]*m.M[c][0]) + (M[1][r]*m.M[c][1]) + (M[2][r]*m.M[c][2]) + (M[3][r]*m.M[c][3]);
			memcpy (M,tmp,36);
			return *this;
		}
		inline Vec3f operator* (const Vec3f& param) const
		{
			Vec3f ret;
			ret.V[0] = M[0][0]*param.V[0] + M[1][0]*param.V[1] + M[2][0]*param.V[2]; 
			ret.V[1] = M[0][1]*param.V[0] + M[1][1]*param.V[1] + M[2][1]*param.V[2];
			ret.V[2] = M[0][2]*param.V[0] + M[1][2]*param.V[1] + M[2][2]*param.V[2];
			return ret;
		}   
		

		inline void MakeIdentity()
		{
			M[0][0] = 1;M[1][0] = 0;M[2][0] = 0;
			M[0][1] = 0;M[1][1] = 1;M[2][1] = 0;
			M[0][2] = 0;M[1][2] = 0;M[2][2] = 1;
		}
		inline void Transpose()
		{
			float tmp[3][3];
			for(int32_t r=0; r < 3; r++)
			{
				tmp[0][r] = M[r][0];
				tmp[1][r] = M[r][1];
				tmp[2][r] = M[r][2];
			}
			memcpy(M,tmp,36);
		}


		inline void SetScale(const Vec3f& v)
		{
			M[0][0]*=v.V[0]; M[1][0]*=v.V[1]; M[2][0]*=v.V[2];
			M[0][1]*=v.V[0]; M[1][1]*=v.V[1]; M[2][1]*=v.V[2];
			M[0][2]*=v.V[0]; M[1][2]*=v.V[1]; M[2][2]*=v.V[2];
		}

		const char* Debug()
		{
			std::ostringstream oss;

			for(int r = 0;r < 4;r++)
				oss<<"|"<<M[0][r]<<","<<M[1][r]<<","<<M[2][r]<<","<<M[3][r]<<"|\n";

			return oss.str().c_str();
		}
		friend std::ostream& operator<< (std::ostream &lhs,Mat3f &rhs);

		friend std::istream& operator>> (std::istream &lhs,Mat3f &rhs);


};

#endif
