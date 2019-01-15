#ifndef PAKKI_MATH_UTIL
#define PAKKI_MATH_UTIL
#include <math.h>
#include <stdlib.h>
/*namespace Math
  {
  static inline void cross_product(vec3* result, const vec3* lhv, const vec3*rhv);
  static inline vec3 cross_product(const vec3* lhv, const vec3*rhv);
  void perspective(mat4* m,float y_fov,float aspect,float n,float f);
  static constexpr float pi = 3.141592653f;
  static constexpr float deg_to_rad = pi / 180.f; 
  static constexpr float red_to_deg = 180.f / pi;
  static inline void create_translation_matrix(mat4* result, const vec3 v);
  static inline void create_lookat_mat4(mat4* Result, const vec3* eye, const vec3* target, const vec3* up);
  }*/
namespace MATH
{

	static constexpr float pi = 3.141592653f;
	static constexpr float deg_to_rad = pi / 180.f; 
	static constexpr float rad_to_deg = 180.f / pi;


	struct  vec2
	{
		vec2() : x(0),y(0) {}
		vec2(float _x,float _y) : x(_x),y(_y) {}

		vec2 operator-(const vec2& rhv)const {
			return vec2(this->x - rhv.x,this->y - rhv.y);
		}
		void operator*=(const float rhv){
			x *= rhv;
			y *= rhv; 
		}				
		union
		{
			struct
			{
				float x,y;	
			};
			struct
			{
				float 	s, t;	
			};
		};
	};

	struct  vec3
	{
		vec3() : x(0),y(0),z(0) {}
		vec3(float _x,float _y,float _z) : x(_x),y(_y),z(_z) {}
		union
		{
			struct
			{
				float x,y,z;	
			};
			struct
			{
				float 	a, b ,c;	
			};
		};
		vec3 operator+(const vec3& rhv)
		{
			return vec3(this->x + rhv.x,this->y + rhv.y,this->z + rhv.z);
		}
		vec3 operator-(const vec3& rhv) const
		{
			return vec3(this->x - rhv.x,this->y - rhv.y,this->z - rhv.z);
		}

		void operator+=(const vec3& rhv)
		{
			this->x += rhv.x;
			this->y += rhv.y;
			this->z += rhv.z;
		}
		void operator-=(const vec3& rhv)
		{
			this->x -= rhv.x;
			this->y -= rhv.y;
			this->z -= rhv.z;
		}

		float operator*(const vec3& rhv) const 
		{
			return x * rhv.x + y* rhv.y + z * rhv.z;
		}
		vec3 operator*(const float rhv) const
		{
			return vec3(x * rhv, y* rhv, z * rhv);
		}

	};
	vec3 operator*(const float rhv, const vec3& lhv) 
	{
		return vec3(lhv.x * rhv, lhv.y* rhv, lhv.z * rhv);
	}

	struct  vec4
	{
		vec4() : x(0),y(0),z(0) {}
		vec4(float _x,float _y,float _z,float _w) : x(_x),y(_y),z(_z),w(_w) {}

		union
		{
			struct
			{
				float x,y,z,w;	
			};
			struct
			{
				float scalar,i,j,k;	
			};
			struct
			{
				float 	a, b ,c, d;	
			};
		};
	};
	static inline vec3 cross_product(const vec3& lhv, const vec3& rhv);
	static inline float lenght_fast(const vec3& v);
	static inline vec3 get_scaled(const vec3& v,float scale);
#define QtoV3(Q) (*((vec3*)(&(Q))))
	struct quaternion
	{
		float scalar,i,j,k;
		quaternion(float _scalar,float _i,float _j,float _k): scalar(_scalar),i(_i),j(_j),k(_k) {};
		//https://ipfs.io/ipfs/QmXoypizjW3WknFiJnKLwHCnL72vedxjQkDDP1mXWo6uco/wiki/Conversion_between_quaternions_and_Euler_angles.html
#if 0
		quaternion(const vec3& v)
		{
#if 0
			float t0 = cosf(v.y / 2.f); // yaw
			float t1 = sinf(v.y / 2.f); // yaw
			float t2 = cosf(v.z / 2.f); // roll
			float t3 = sinf(v.z / 2.f); // roll
			float t4 = cosf(v.x / 2.f); // pitch
			float t5 = sinf(v.x / 2.f); // pitch
			scalar = t0 * t2 * t4 + t1 * t3 * t5;
			i = t0 * t3 * t4 - t1 * t2 * t5;
			j = t0 * t2 * t5 + t1 * t3 * t4;
			k = t1 * t2 * t4 - t0 * t3 * t5;
#endif
#if 1
			float c1 = cosf(v.y / 2.f); // yaw / heading
			float s1 = sinf(v.y / 2.f); // yaw  / heading
			float c2 = cosf(v.x / 2.f); // pitch / attitude
			float s2 = sinf(v.x/ 2.f); // pitch / attitude
			float c3 = cosf(v.z / 2.f); // roll / bank
			float s3 = sinf(v.z / 2.f); // roll / bank
			float c1c2 = c1*c2;
			float s1s2 = s1*s2;
			scalar =c1c2*c3 - s1s2*s3;
			i =c1c2*s3 + s1s2*c3;
			j =s1*c2*c3 + c1*s2*s3;
			k =c1*s2*c3 - s1*c2*s3;
#endif

		}
#endif
		// Given a rotation vector of form unitRotationAxis * angle,
		// returns the equivalent quaternion (unitRotationAxis * sin(angle), cos(Angle)).
		quaternion (const vec3& v) {
#if 1
			vec3 scaled = get_scaled(v,0.5f);
			vec3 c(cosf(scaled.x),cosf(scaled.y ),cosf(scaled.z ));
			vec3 s(sinf(scaled.x ),sinf(scaled.y),sinf(scaled.z));
			//float c = (eulerAngle * T(0.5));
			//float s = glm::sin(eulerAngle * T(0.5));


			this->scalar = c.x * c.y * c.z + s.x * s.y * s.z;
			this->i = s.x * c.y * c.z - c.x * s.y * s.z;
			this->j = c.x * s.y * c.z + s.x * c.y * s.z;
			this->k = c.x * c.y * s.z - s.x * s.y * c.z;
#endif
#if 0
			float angleSquared = lenght_fast(v);//v.LengthSq();
			float s = 0;
			float c = 1;
			if (angleSquared > 0) {
				float angle = sqrt(angleSquared);
				s = sin(angle * 0.5f) / angle; // normalize
				c = cos(angle * 0.5f);
			}
			i = s * v.x;
			j = s * v.y;
			k = s * v.z;
			scalar = c;
			//return quaternion(s * v.x, s * v.y, s * v.z, c);
#endif
		}
		quaternion(const vec3& v,float theata ) // axis angle rotation
		{
			scalar = cosf(theata / 2.f);
			float s = sinf(theata / 2.f);
			i = v.x * s;
			j = v.y * s;
			k = v.z * s;
		}
		quaternion():scalar(1), i(0),j(0),k(0){};
		quaternion operator *(const quaternion& rhv) const
		{

#if 1
			return quaternion(
					scalar * rhv.scalar - i *	rhv.i - j * rhv.j -			k * rhv.k,
					scalar * rhv.i		+ i *	rhv.scalar + j * rhv.k -			k * rhv.j,
					scalar * rhv.j		- i *	rhv.k + j * rhv.scalar +	k * rhv.i,
					scalar * rhv.k		+ i *	rhv.j - j * rhv.i +			k * rhv.scalar);
#endif
#if 0
			//vec3 k = QtoV3(*this);	
			float nScalar = this->scalar * rhv.scalar - QtoV3(*this) * QtoV3(rhv);// *((vec3*)&i) * *((vec3*)&rhv.i) ;
			vec3 retVec = QtoV3(*this) * rhv.scalar + QtoV3(rhv) * this->scalar + cross_product(QtoV3(*this),QtoV3(rhv));

			quaternion qret;
			qret.scalar = nScalar;
			QtoV3(qret) = retVec;

			return qret;

#endif
		}
	};
	quaternion interpolate_q(quaternion start,quaternion end,float delta)
	{
		//calc cosine and theata
		float cosom = start.i * end.i + start.j * end.j + start.k * end.k + start.scalar * end.scalar; 
		quaternion rend = end;
		if(cosom < 0.0f)
		{
			cosom = -cosom;
			rend.i = -rend.i;
			rend.j = -rend.j;
			rend.k = -rend.k;
			rend.scalar = -rend.scalar;
		}
		float sclp = 0, sclq = 0;
		if((1.f - cosom) > 0.0001)
		{
			float omega,sinom;
			omega = acosf(cosom);
			sinom = sinf(omega);
			sclp = sinf((1.0f - delta) *  omega) / sinom;
			sclq = sinf(delta * omega) / sinom;
		}
		else
		{
			sclp = 1.0f - delta;
			sclq = delta;
		}

		return quaternion
			(
			 sclp * start.scalar + sclq * rend.scalar,	
			 sclp * start.i + sclq * rend.i,	
			 sclp * start.j + sclq * rend.j,	
			 sclp * start.k + sclq * rend.k	
			);
	}
	//static inline rotate_quaternion(quaternion* g,const vec3& axis,float angle)
	//{

	//}

	static inline float lenght(const vec3& v)
	{
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z );		
	}
	static inline float lenght(const vec4& v)
	{
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);		
	}
	static inline float lenght(const vec2& v)
	{
		return sqrtf(v.x * v.x + v.y * v.y);		
	}
	static inline float lenght(const quaternion& q)
	{
		return sqrtf(q.scalar * q.scalar + q.i * q.i + q.j * q.j + q.k * q.k);
	}
	static inline float lenght_fast(const vec3& v)
	{
		return (v.x * v.x + v.y * v.y + v.z * v.z );		
	}
	static inline float lenght_fast(const vec4& v)
	{
		return (v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);		
	}
	static inline float lenght_fast(const vec2& v)
	{
		return (v.x * v.x + v.y * v.y);		
	}
	static inline vec4 normalized(const vec4& v)
	{
		float l = lenght(v);
		if(!l) return vec4();

		return vec4 (v.x / l,v.y / l,v.z / l,v.w / l);
	}

	static inline vec3 normalized(const vec3& v)
	{
		float l = lenght(v);
		if(!l) return vec3();

		return vec3 (v.x / l,v.y / l,v.z / l);
	}

	static inline vec2 normalized(const vec2& v)
	{
		float l = lenght(v);
		if(!l) return vec2();

		return vec2 (v.x / l,v.y / l);
	}


	static inline void normalize(vec3* v)
	{
		float l = lenght(*v);
		if(!l) return;
		v->x /= l;
		v->y /= l;
		v->z /= l;
	}
	static inline void normalize(vec2* v)
	{
		float l = lenght(*v);
		if(!l) return;
		v->x /= l;
		v->y /= l;
	}
	static inline void normalize(quaternion* q)
	{
		float l = lenght(*q);
		if(!l) return;
		//assimp inv
		float invMag = 1.0f / l;
		q->scalar *= invMag;
		q->i *= invMag;
		q->j *= invMag;
		q->k *= invMag;
	}

	static inline void normalize(vec4* v)
	{
		float l = lenght(*v);
		if(!l) return;
		v->x /= l;
		v->y /= l;
		v->z /= l;
		v->w /= l;
	}
	static inline vec2 get_scaled(const vec2& v,float scale)
	{
		vec2 ret;
		ret.x = v.x * scale;
		ret.y = v.y * scale;
		return ret;
	}

	static inline vec3 get_scaled(const vec3& v,float scale)
	{
		vec3 ret;
		ret.x = v.x * scale;
		ret.y = v.y * scale;
		ret.z = v.z * scale;
		return ret;
	}

	static inline vec4 get_scaled(const vec4& v,float scale)
	{
		vec4 ret;
		ret.x = v.x * scale;
		ret.y = v.y * scale;
		ret.z = v.z * scale;
		ret.w = v.w * scale;
		return ret;
	}



	static inline void scale(vec2* v,float scale)
	{
		v->x *= scale;v->y *= scale;
	}

	static inline void scale(vec3* v,float scale)
	{
		v->x *= scale;v->y *= scale;v->z *= scale;
	}
	static inline void scale(vec4* v,float scale)
	{
		v->x *= scale;v->y *= scale;v->z *= scale;v->w *= scale;
	}

	struct  mat3
	{
		float mat[3][3];
	};
	struct  mat4
	{
		mat4() 
		{
			for(int y = 0; y < 4; y++)
				for(int x = 0; x < 4; x++)
					mat[y][x] = 0;
		};
		mat4(float k[16]) 
		{
			memcpy(mat,k,sizeof(float) * 16);
		};
		mat4(const quaternion& q)
		{
			float a = q.scalar;	
			float b = q.i;
			float c = q.j;
			float d = q.k;

			float a2 = a * a;	
			float b2 = b * b;
			float c2 = c * c;
			float d2 = d * d;

			mat[0][0] = a2 + b2 - c2 - d2;
			mat[0][1] = 2.f*(b*c + a*d);
			mat[0][2] = 2.f*(b*d - a*c);
			mat[0][3] = 0.f;

			mat[1][0] = 2*(b*c - a*d);
			mat[1][1] = a2 - b2 + c2 - d2;
			mat[1][2] = 2.f*(c*d + a*b);
			mat[1][3] = 0.f;

			mat[2][0] = 2.f*(b*d + a*c);
			mat[2][1] = 2.f*(c*d - a*b);
			mat[2][2] = a2 - b2 - c2 + d2;
			mat[2][3] = 0.f;

			mat[3][0] = mat[3][1] = mat[3][2] = 0.f;
			mat[3][3] = 1.f;

#if 0
			float ww = q.w * q.w;
			float xx = q.x * q.x;
			float yy = q.y * q.y;
			float zz = q.z * q.z;

			M[0][0] = ww + xx - yy - zz;
			M[0][1] = 2 * (q.x * q.y - q.w * q.z);
			M[0][2] = 2 * (q.x * q.z + q.w * q.y);
			M[0][3] = T(0);

			M[1][0] = 2 * (q.x * q.y + q.w * q.z);
			M[1][1] = ww - xx + yy - zz;
			M[1][2] = 2 * (q.y * q.z - q.w * q.x);
			M[1][3] = T(0);

			M[2][0] = 2 * (q.x * q.z - q.w * q.y);
			M[2][1] = 2 * (q.y * q.z + q.w * q.x);
			M[2][2] = ww - xx - yy + zz;
			M[2][3] = T(0);

			M[3][0] = T(0);
			M[3][1] = T(0);
			M[3][2] = T(0);
			M[3][3] = T(1);
#endif
		}
		float mat[4][4];

		vec4 operator* (const vec4& rhv) const 
		{
			vec4 Result;
			Result.x = this->mat[0][0] * rhv.x + this->mat[1][0] * rhv.y + this->mat[2][0] * rhv.z + this->mat[3][0] * rhv.w;
			Result.y = this->mat[0][1] * rhv.x + this->mat[1][1] * rhv.y + this->mat[2][1] * rhv.z + this->mat[3][1] * rhv.w;
			Result.z = this->mat[0][2] * rhv.x + this->mat[1][2] * rhv.y + this->mat[2][2] * rhv.z + this->mat[3][2] * rhv.w;
			Result.w = this->mat[0][3] * rhv.x + this->mat[1][3] * rhv.y + this->mat[2][3] * rhv.z + this->mat[3][3] * rhv.w;	
			return Result;
		}

		mat4 operator* (const mat4& rhv) const
		{
			mat4 temp; //*this;
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					temp.mat[x][y] = 0;
					for (int n = 0; n < 4; n++)
					{
						temp.mat[x][y] += this->mat[n][y] * rhv.mat[x][n];
					}
				}
			}	
			return temp;
		}
		void operator*=(const mat4& rhv) 
		{
			mat4 temp = *this;
			*this = temp * rhv;
		}
		//static inline mat4 rotationMatY(float angle);
	};

	static void scale_mat4(mat4* mat,float scale)
	{
		for (int x = 0; x < 3; x++)
			for (int y = 0; y < 3; y++)
				mat->mat[x][y] *= scale;		
	} 
	static void scale_mat4(mat4* mat,const vec3 scale)
	{
		//[x][y]
		mat->mat[0][0] *= scale.x;mat->mat[1][0] *= scale.y;mat->mat[2][0] *= scale.z;
		mat->mat[0][1] *= scale.x;mat->mat[1][1] *= scale.y;mat->mat[2][1] *= scale.z;
		mat->mat[0][2] *= scale.x;mat->mat[1][2] *= scale.y;mat->mat[2][2] *= scale.z;
	} 


	static inline void identify(mat4* m)
	{
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				m->mat[x][y] = x == y ? 1.f : 0.f;
			}
		}	
	}

	static inline void transpose(mat4* ret,mat4* m)
	{
		int i, j;
		for(j=0; j<4; ++j)
			for(i=0; i<4; ++i)
				ret->mat[i][j] = m->mat[j][i];
	}

	static void create_scaling_matrix(mat4* m,const vec3& v)
	{
		identify(m);
		m->mat[0][0] = v.x;
		m->mat[1][1] = v.y;
		m->mat[2][2] = v.z;
	}

	static inline void identify(mat3* m)
	{
		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				m->mat[x][y] = x == y ? 1.f : 0.f;
			}
		}	
	}


	//static const float identy[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//static const mat4 IdentityMatrix(identy);
	static inline void translate(mat4* m,const vec3& v)
	{
		m->mat[3][0] += v.x;	
		m->mat[3][1] += v.y;	
		m->mat[3][2] += v.z;	
	}

	static inline void orthomat(mat4* m, float left, float right, float bottom, float top, float Near, float Far)
	{
		m->mat[0][0] = (2.f / (right - left));
		m->mat[0][1] = 0.f;
		m->mat[0][2] = 0.f;
		m->mat[0][3] = 0.f;

		m->mat[1][1] = (2.f / (top - bottom));
		m->mat[1][0] = 0.f;
		m->mat[1][2] = 0.f;
		m->mat[1][3] = 0.f;

		m->mat[2][2] = (-2.f / (Far - Near));
		m->mat[2][0] = 0.f;
		m->mat[2][1] = 0.f;
		m->mat[2][3] = 0.f;


		m->mat[3][0] = -((right + left) / (right - left));
		m->mat[3][1] = -((top + bottom) / (top - bottom));
		m->mat[3][2] = -((Far + Near) / (Far - Near));
		m->mat[3][3] = 1.f;
	}
	void perspective(mat4* m,float y_fov,float aspect,float n,float f)
	{
		float const a = (float)(1.f / tanf(y_fov / 2.f));
		m->mat[0][0] = a / aspect;
		m->mat[0][1] = 0.f;
		m->mat[0][2] = 0.f;
		m->mat[0][3] = 0.f;

		m->mat[1][0] = 0.f;
		m->mat[1][1] = a;
		m->mat[1][2] = 0.f;
		m->mat[1][3] = 0.f;

		m->mat[2][0] = 0.f;
		m->mat[2][1] = 0.f;
		m->mat[2][2] = -((f + n) / (f - n));
		m->mat[2][3] = -1.f;

		m->mat[3][0] = 0.f;
		m->mat[3][1] = 0.f;
		m->mat[3][2] = -((2.f * f * n) / (f - n));
		m->mat[3][3] = 0.f;	
	}
	/*
	   static inline void cross_product(vec3* result, const vec3* lhv, const vec3*rhv)
	   {
	   vec3 left = *lhv;
	   result->x = left.y * rhv->z - rhv->y * left.z;
	   result->y = -1 * (left.x * rhv->z - rhv->x * left.z);
	   result->z = left.x * rhv->y - rhv->x * left.y;	
	   }
	   */
	static inline vec3 cross_product(const vec3& lhv, const vec3& rhv)
	{
		vec3 result;
		result.x = lhv.y * rhv.z - rhv.y * lhv.z;
		result.y = -1 * (lhv.x * rhv.z - rhv.x * lhv.z);
		result.z = lhv.x * rhv.y - rhv.x * lhv.y;	
		return result;
	}
	static inline mat4 rotationMatY(float angle)
	{	
		float s = sinf(angle);
		float c = cosf(angle);

		mat4 ret;
		ret.mat[0][0] = c;

		ret.mat[0][2] = s;
		ret.mat[1][1] = 1.f;
		ret.mat[2][0] = -s;
		ret.mat[2][2] = c;
		ret.mat[3][3] = 1.f;
		return ret;
	}
	static inline mat4 rotationMatX(float angle)
	{	
		float s = sinf(angle);
		float c = cosf(angle);

		mat4 ret;
		ret.mat[0][0] = 1.f;

		ret.mat[1][1] = c;
		ret.mat[1][2] = s;
		ret.mat[2][1] = -s;
		ret.mat[2][2] = c;
		ret.mat[3][3] = 1.f;
		return ret;
	} 
	static inline mat4 rotationMatZ(float angle)
	{	

		float s = sinf(angle);
		float c = cosf(angle);

		mat4 ret;
		ret.mat[0][0] = c;

		ret.mat[0][1] = s;
		ret.mat[1][0] = -s;
		ret.mat[1][1] = c;
		ret.mat[2][2] = 1.f;
		ret.mat[3][3] = 1.f;



		//float s = sinf(angle);
		//float c = cosf(angle);
		//mat4x4 R = {
		//		{   c, 0.f,   s, 0.f},
		//		{ 0.f, 1.f, 0.f, 0.f},
		//		{  -s, 0.f,   c, 0.f},
		//		{ 0.f, 0.f, 0.f, 1.f}
		//	};




		return ret;
	}

	static inline void create_translation_matrix(mat4* result, const vec3 v);
	static inline void create_lookat_mat4(mat4* Result, const vec3& eye,
			const vec3& target, const vec3& up)
	{
		vec3 D(eye.x - target.x,eye.y - target.y ,eye.z - target.z);
		normalize(&D);
		//normalize_vec3(&D); // needed?

		//vec3 R = { 0 , 0, 0};
		vec3 R = cross_product(up, D);


		mat4 trans;
		vec3 camPos = { -eye.x,-eye.y , -eye.z };
		create_translation_matrix(&trans, camPos);


		Result->mat[0][0] = R.x; Result->mat[0][1] = up.x; Result->mat[0][2] = D.x; Result->mat[0][3] = 0.f;
		Result->mat[1][0] = R.y; Result->mat[1][1] = up.y; Result->mat[1][2] = D.y; Result->mat[1][3] = 0.f;
		Result->mat[2][0] = R.z; Result->mat[2][1] = up.z; Result->mat[2][2] = D.z; Result->mat[2][3] = 0.f;
		Result->mat[3][0] = 0.f; Result->mat[3][1] = 0.f; Result->mat[3][2] = 0.f; Result->mat[3][3] = 1.f;


		(*Result) =  (*Result) * trans; 
		//(*Result) =  (*Result) * trans; 
		//mult_mat4(Result, Result, &trans);


	}
	static inline void create_translation_matrix(mat4* result, const vec3 v)
	{
		result->mat[0][0] = 1; result->mat[1][0] = 0; result->mat[2][0] = 0; result->mat[3][0] = v.x;
		result->mat[0][1] = 0; result->mat[1][1] = 1; result->mat[2][1] = 0; result->mat[3][1] = v.y;
		result->mat[0][2] = 0; result->mat[1][2] = 0; result->mat[2][2] = 1; result->mat[3][2] = v.z;
		result->mat[0][3] = 0; result->mat[1][3] = 0; result->mat[2][3] = 0; result->mat[3][3] = 1;
	}
	void rotateY(mat4* m,float angle) 
	{
		if(angle == 0) return;	
		mat4 r = rotationMatY(angle);
		*m *=  r; 
	}
	void rotateX(mat4* m,float angle) 
	{
		if(angle == 0) return;	
		mat4 r = rotationMatX(angle);

		*m *=  r; 
	}
	void rotateZ(mat4* m,float angle) 
	{
		if(angle == 0) return;	
		mat4 r = rotationMatZ(angle);
		*m *=  r; 
	}

	//TODO custom rand
	static inline void seed_rand(unsigned int seed)
	{
		srand(seed);
	}
	static inline int irand_between(int start, int end)//inclusive and exclusive
	{
		int realRange = end - start;
		int inrange = rand() % realRange;
		return start + inrange;
	}
	static inline int irand_range(int range)//inclusive and exclusive
	{
		int inrange = rand() % range;
		return inrange;
	}
	static float lerp(float v0, float v1, float t)
	{
		return (1 - t) * v0 + t * v1;
	}
	static inline int max_val(int lhv,int rhv)
	{
		return lhv > rhv ? lhv : lhv;	
	}
	static inline int min_val(int lhv,int rhv)
	{
		return lhv < rhv ? lhv : lhv;	
	}

	static inline void inverse_mat4(mat4* res, mat4* m)//assumes that matrix is invertable, implementation similar to linmath and glu
	{
		float s[6];
		float c[6];
		s[0] = m->mat[0][0] * m->mat[1][1] - m->mat[1][0] * m->mat[0][1];
		s[1] = m->mat[0][0] * m->mat[1][2] - m->mat[1][0] * m->mat[0][2];
		s[2] = m->mat[0][0] * m->mat[1][3] - m->mat[1][0] * m->mat[0][3];
		s[3] = m->mat[0][1] * m->mat[1][2] - m->mat[1][1] * m->mat[0][2];
		s[4] = m->mat[0][1] * m->mat[1][3] - m->mat[1][1] * m->mat[0][3];
		s[5] = m->mat[0][2] * m->mat[1][3] - m->mat[1][2] * m->mat[0][3];

		c[0] = m->mat[2][0] * m->mat[3][1] - m->mat[3][0] * m->mat[2][1];
		c[1] = m->mat[2][0] * m->mat[3][2] - m->mat[3][0] * m->mat[2][2];
		c[2] = m->mat[2][0] * m->mat[3][3] - m->mat[3][0] * m->mat[2][3];
		c[3] = m->mat[2][1] * m->mat[3][2] - m->mat[3][1] * m->mat[2][2];
		c[4] = m->mat[2][1] * m->mat[3][3] - m->mat[3][1] * m->mat[2][3];
		c[5] = m->mat[2][2] * m->mat[3][3] - m->mat[3][2] * m->mat[2][3];


		float idet = 1.0f / (s[0] * c[5] - s[1] * c[4] + s[2] * c[3] + s[3] * c[2] - s[4] * c[1] + s[5] * c[0]);

		res->mat[0][0] = (m->mat[1][1] * c[5] - m->mat[1][2] * c[4] + m->mat[1][3] * c[3]) * idet;
		res->mat[0][1] = (-m->mat[0][1] * c[5] + m->mat[0][2] * c[4] - m->mat[0][3] * c[3]) * idet;
		res->mat[0][2] = (m->mat[3][1] * s[5] - m->mat[3][2] * s[4] + m->mat[3][3] * s[3]) * idet;
		res->mat[0][3] = (-m->mat[2][1] * s[5] + m->mat[2][2] * s[4] - m->mat[2][3] * s[3]) * idet;

		res->mat[1][0] = (-m->mat[1][0] * c[5] + m->mat[1][2] * c[2] - m->mat[1][3] * c[1]) * idet;
		res->mat[1][1] = (m->mat[0][0] * c[5] - m->mat[0][2] * c[2] + m->mat[0][3] * c[1]) * idet;
		res->mat[1][2] = (-m->mat[3][0] * s[5] + m->mat[3][2] * s[2] - m->mat[3][3] * s[1]) * idet;
		res->mat[1][3] = (m->mat[2][0] * s[5] - m->mat[2][2] * s[2] + m->mat[2][3] * s[1]) * idet;

		res->mat[2][0] = (m->mat[1][0] * c[4] - m->mat[1][1] * c[2] + m->mat[1][3] * c[0]) * idet;
		res->mat[2][1] = (-m->mat[0][0] * c[4] + m->mat[0][1] * c[2] - m->mat[0][3] * c[0]) * idet;
		res->mat[2][2] = (m->mat[3][0] * s[4] - m->mat[3][1] * s[2] + m->mat[3][3] * s[0]) * idet;
		res->mat[2][3] = (-m->mat[2][0] * s[4] + m->mat[2][1] * s[2] - m->mat[2][3] * s[0]) * idet;

		res->mat[3][0] = (-m->mat[1][0] * c[3] + m->mat[1][1] * c[1] - m->mat[1][2] * c[0]) * idet;
		res->mat[3][1] = (m->mat[0][0] * c[3] - m->mat[0][1] * c[1] + m->mat[0][2] * c[0]) * idet;
		res->mat[3][2] = (-m->mat[3][0] * s[3] + m->mat[3][1] * s[1] - m->mat[3][2] * s[0]) * idet;
		res->mat[3][3] = (m->mat[2][0] * s[3] - m->mat[2][1] * s[1] + m->mat[2][2] * s[0]) * idet;
	}

#if 0
	template<typename T>
		static inline T max(T lhv,T rhv)
		{
			return lhv > rhv ? lhv : lhv;	
		}
	template<typename T>
		static inline T min(T lhv,T rhv)
		{
			return lhv < rhv ? lhv : lhv;	
		}
#endif
}
//TODO SIMD versions 
//TODO 2D Math utils
#endif //PAKKI_MATHUTIL
