#ifndef C_MATH_UTIL
#define C_MATH_UTIL
#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <immintrin.h>

static const float pi = 3.141592653f;
static const float deg_to_rad = pi / 180.f; 
static const float rad_to_deg = 180.f / pi;

#ifndef MATH_ERROR
#define MATH_ERROR(error)
#endif

typedef struct 
{
	float x,y;
} vec2;

typedef struct 
{
	float x,y,z;
} vec3;

typedef struct 
{
	float x,y,z,w;
} vec4;

typedef vec4 quat;

#define DEFINE_VECTOR_FUNCS(n)\
	static inline vec##n  add_vec##n(const vec##n lhv,const vec##n rhv) { \
		const float* l = (float*)&lhv; \
		const float* r = (float*)&rhv; \
		float res[n];\
		for(int i = 0; i < n; i++,l++,r++) \
		{\
			res[i] = *l + *r;\
		}\
		return *(vec##n*)res;\
	} \
\
static inline vec##n neg_vec##n(const vec##n lhv,const vec##n rhv) { \
	const float* l = (float*)&lhv; \
	const float* r = (float*)&rhv; \
	float res[n];\
	for(int i = 0; i < n; i++,l++,r++) \
	{\
		res[i] = *l - *r;\
	}\
	return *(vec##n*)res;\
} \
\
static inline vec##n scale_vec##n(const vec##n vec,const float scale) { \
	const float* v = (float*)&vec; \
	float res[n];\
	for(int i = 0; i < n; i++,v++) \
	{\
		res[i] = *v * scale;\
	}\
	return *(vec##n*)res;\
} \
\
static inline float lenght_vec##n(const vec##n vec)\
{\
	float* v = (float*)&vec; \
	float res = 0; \
	for(int i = 0; i < n;i++,v++)\
	{\
		res += (*v) * (*v);\
	}\
	return sqrtf(res);\
}\
\
static inline float fast_lenght_vec##n(const vec##n vec)\
{\
	float* v = (float*)&vec; \
	float res = 0; \
	for(int i = 0; i < n;i++,v++)\
	{\
		res += (*v) * (*v);\
	}\
	return res;\
}\
static inline vec##n normalize_vec##n(const vec##n vec)\
{\
	float lenght = lenght_vec##n(vec);\
	MATH_ERROR(lenght < 0); \
	if(lenght == 0) return vec; \
	float* v = (float*)&vec; \
	float res[n]; \
	float invMag = 1.f / lenght;\
	for(int i = 0; i < n;i++,v++)\
	{\
		res[i] = (*v) * invMag;\
	}\
	return *(vec##n*)res;\
}\
static inline void normalize_inside_vec##n(const vec##n *vec)\
{\
	float lenght = lenght_vec##n(*vec);\
	MATH_ERROR(lenght < 0); \
	if(lenght == 0) return ; \
	float* res = (float*)vec; \
	float invMag = 1.f / lenght;\
	for(int i = 0; i < n;i++)\
	{\
		res[i] *= invMag;\
	}\
}

	DEFINE_VECTOR_FUNCS(2)
	DEFINE_VECTOR_FUNCS(3)
DEFINE_VECTOR_FUNCS(4)


static inline quat quat_from_axis(const vec3 axis,const float theata)
{
	float halfTheata = theata / 2.f;
	float s = sinf(halfTheata);
	return (quat) {.x = axis.x * s, .y = axis.y * s, .z = axis.z * s , .w = cosf(halfTheata)};
}

static inline quat quat_mult(const quat lhv, const quat rhv)
{
	return (quat){
		.x = lhv.w * rhv.x + lhv.x *	rhv.w + lhv.y * rhv.z -	lhv.z * rhv.y,
			.y = lhv.w * rhv.y - lhv.x *	rhv.z + lhv.y * rhv.w +	lhv.z * rhv.x,
			.z = lhv.w * rhv.z + lhv.x *	rhv.y - lhv.y * rhv.x +	lhv.z * rhv.w,
			.w = lhv.w * rhv.w - lhv.x *	rhv.x - lhv.y * rhv.y -	lhv.z * rhv.z,
	};
}


static inline quat quat_lerp(const quat start,const quat end,const float delta)
{
	//calc cosine and theata
	float cosom = start.x * end.x + start.y * end.y + start.z * end.z + start.w * end.w; 
	quat rend = end;
	if(cosom < 0.0f)
	{
		cosom = -cosom;
		rend.x = -rend.x;
		rend.y = -rend.y;
		rend.z = -rend.z;
		rend.w = -rend.w;
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

	return (quat)
	{
		.x = sclp * start.x + sclq * rend.x,	
			.y = sclp * start.y + sclq * rend.y,	
			.z = sclp * start.z + sclq * rend.z,	
			.w = sclp * start.w + sclq * rend.w,	
	};
}

#define normalize_quat(q) nomalize_vec4(*(vec4*)&q); 

typedef struct 
{
	float mat[4][4];
} mat4;

typedef struct 
{
	float mat[3][3];
} mat3;

#define CREATE_MATRIX_FUNCTIONS(n)\
	static inline void identify_mat##n(mat##n *m)\
{\
	*m = (mat##n){{{0}}};\
	for(uint32_t i = 0; i < n; i++)\
	{\
		m->mat[i][i] = 1.f;\
	}\
}

	CREATE_MATRIX_FUNCTIONS(3)
CREATE_MATRIX_FUNCTIONS(4)


static inline vec3 cross_product(const vec3 lhv, const vec3 rhv)
{
	vec3 result;
	result.x = lhv.y * rhv.z - rhv.y * lhv.z;
	result.y = -1 * (lhv.x * rhv.z - rhv.x * lhv.z);
	result.z = lhv.x * rhv.y - rhv.x * lhv.y;	
	return result;
}

static inline void mat4_from_quat(mat4* res,const quat q)
{
	float a = q.w;	
	float b = q.x;
	float c = q.y;
	float d = q.z;

	float a2 = a * a;	
	float b2 = b * b;
	float c2 = c * c;
	float d2 = d * d;

	res->mat[0][0] = a2 + b2 - c2 - d2;
	res->mat[0][1] = 2.f*(b*c + a*d);
	res->mat[0][2] = 2.f*(b*d - a*c);
	res->mat[0][3] = 0.f;

	res->mat[1][0] = 2*(b*c - a*d);
	res->mat[1][1] = a2 - b2 + c2 - d2;
	res->mat[1][2] = 2.f*(c*d + a*b);
	res->mat[1][3] = 0.f;

	res->mat[2][0] = 2.f*(b*d + a*c);
	res->mat[2][1] = 2.f*(c*d - a*b);
	res->mat[2][2] = a2 - b2 - c2 + d2;
	res->mat[2][3] = 0.f;

	res->mat[3][0] = res->mat[3][1] = res->mat[3][2] = 0.f;
	res->mat[3][3] = 1.f;
}

static inline vec4 mat4_mult_vec4(const mat4* lhv,const vec4 rhv)
{
	return (vec4) {
		.x = lhv->mat[0][0] * rhv.x + lhv->mat[1][0] * rhv.y + lhv->mat[2][0] * rhv.z + lhv->mat[3][0] * rhv.w,
			.y = lhv->mat[0][1] * rhv.x + lhv->mat[1][1] * rhv.y + lhv->mat[2][1] * rhv.z + lhv->mat[3][1] * rhv.w,
			.z = lhv->mat[0][2] * rhv.x + lhv->mat[1][2] * rhv.y + lhv->mat[2][2] * rhv.z + lhv->mat[3][2] * rhv.w,
			.w = lhv->mat[0][3] * rhv.x + lhv->mat[1][3] * rhv.y + lhv->mat[2][3] * rhv.z + lhv->mat[3][3] * rhv.w
	};
}

static inline void mat4_mult_mat4 (mat4* res,const mat4* lhv,const mat4* rhv)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			res->mat[x][y] = 0;
			for (int n = 0; n < 4; n++)
			{
				res->mat[x][y] += lhv->mat[n][y] * rhv->mat[x][n];
			}
		}
	}	
}

static inline void mat4_mult_mat4_inside(mat4* lhv,const mat4* rhv)
{
	mat4 temp = *lhv;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			lhv->mat[x][y] = 0;
			for (int n = 0; n < 4; n++)
			{
				lhv->mat[x][y] += temp.mat[n][y] * rhv->mat[x][n];
			}
		}
	}	
}


typedef __m128 float4_sse;
#define load_4float_sse(v) _mm_load_ps(v)
#define load_2double_sse(v) _mm_load_pd(v)

#define mult_4float_sse(v) _mm_mul_ps(v)


#if 0
static inline void mat4_mult_mat4_inside_fast(mat4* lhv,const mat4* rhv)
{
	//shuffle transpose
	float4_sse row1 = load_4float_sse((float*)lhv);
	float4_sse row2 = load_4float_sse((float*)&lhv->mat[1][0]);
	float4_sse row3 = load_4float_sse((float*)&lhv->mat[2][0]);
	float4_sse row4 = load_4float_sse((float*)&lhv->mat[3][0]);

	__m256 t0, t1, t2, t3, t4, t5, n0, n1;
    mat4 result;

	n0 = &lhv

	n1 = in.n[1];                                               // n1 =  9, 10, 11, 12, 13, 14, 15, 16
	t0 = _mm256_unpacklo_ps(n0, n1);                            // t0 =  1,  9,  2, 10,  5, 13,  6, 14
	t1 = _mm256_unpackhi_ps(n0, n1);                            // t1 =  3, 11,  4, 12,  7, 15,  8, 16

	t2 = _mm256_permute2f128_ps(t0, t1, 0x20);                  // t2 =  1,  9,  2, 10,  3, 11,  4, 12 
	t3 = _mm256_permute2f128_ps(t0, t1, 0x31);                  // t3 =  5, 13,  6, 14,  7, 15,  8, 16

	t4 = _mm256_unpacklo_ps(t2, t3);                            // t2 =  1,  5,  9, 13,  3,  7, 11, 15
	t5 = _mm256_unpackhi_ps(t2, t3);                            // t3 =  2,  6, 10, 14,  4,  8, 12, 16

	result.n[0] = _mm256_permute2f128_ps(t4, t5, 0x20);         // t6 =  1,  5,  9, 13,  2,  6, 10, 14
	result.n[1] = _mm256_permute2f128_ps(t4, t5, 0x31);         // t7 =  3,  7, 11, 15,  4,  8, 12, 16



	mat4 temp = *lhv;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			lhv->mat[x][y] = 0;
			for (int n = 0; n < 4; n++)
			{
				lhv->mat[x][y] += temp.mat[n][y] * rhv->mat[x][n];
			}
		}
	}	
}
#endif


static void inline scale_mat4(mat4* mat,float scale)
{
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 3; y++)
			mat->mat[x][y] *= scale;		
} 

static inline void transpose_mat4(mat4* ret,const mat4* m)
{
	int i, j;
	for(j=0; j<4; ++j)
		for(i=0; i<4; ++i)
			ret->mat[i][j] = m->mat[j][i];
}

static inline void transpose_mat4_inside(mat4* m)
{
	int i, j;
	for(j=0; j<4; ++j)
		for(i=0; i<4; ++i)
			m->mat[i][j] = m->mat[j][i];
}
static void create_scaling_mat4(mat4* m,const vec3 v)
{
	identify_mat4(m);
	m->mat[0][0] = v.x;
	m->mat[1][1] = v.y;
	m->mat[2][2] = v.z;
}

static inline void translate_mat4(mat4* m,const vec3 v)
{
	m->mat[3][0] += v.x;	
	m->mat[3][1] += v.y;	
	m->mat[3][2] += v.z;	
}

static inline void orthomat(mat4* m, float left, float right, float bottom, float top, 
		float Near, float Far)
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
static inline void create_translation_mat_inside(mat4* result, const vec3 v)
{
	result->mat[0][0] = 1; result->mat[1][0] = 0; result->mat[2][0] = 0; result->mat[3][0] = v.x;
	result->mat[0][1] = 0; result->mat[1][1] = 1; result->mat[2][1] = 0; result->mat[3][1] = v.y;
	result->mat[0][2] = 0; result->mat[1][2] = 0; result->mat[2][2] = 1; result->mat[3][2] = v.z;
	result->mat[0][3] = 0; result->mat[1][3] = 0; result->mat[2][3] = 0; result->mat[3][3] = 1;
}
static inline void create_lookat_mat4(mat4* Result, const vec3 eye,
		const vec3 target, const vec3 up)
{
	vec3 D = neg_vec3(eye,target);
	normalize_inside_vec3(&D);
	vec3 R = cross_product(up, D);

	mat4 trans;
	vec3 camPos = { -eye.x,-eye.y , -eye.z };
	create_translation_mat_inside(&trans, camPos);

	Result->mat[0][0] = R.x; Result->mat[0][1] = up.x; Result->mat[0][2] = D.x; Result->mat[0][3] = 0.f;
	Result->mat[1][0] = R.y; Result->mat[1][1] = up.y; Result->mat[1][2] = D.y; Result->mat[1][3] = 0.f;
	Result->mat[2][0] = R.z; Result->mat[2][1] = up.z; Result->mat[2][2] = D.z; Result->mat[2][3] = 0.f;
	Result->mat[3][0] = 0.f; Result->mat[3][1] = 0.f; Result->mat[3][2] = 0.f; Result->mat[3][3] = 1.f;

	mat4_mult_mat4_inside(Result,&trans);
}

static inline void inverse_mat4(mat4* res, mat4* m)//assumes that matrix is invertable, 
	//implementation similar to linmath and glu
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

	float idet = 1.0f / (s[0] * c[5] - s[1] * c[4] + s[2] * c[3] + s[3] * c[2] - s[4] * c[1]
			+ s[5] * c[0]);

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

#endif //C_MATH_UTIL
