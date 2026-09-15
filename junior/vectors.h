#pragma once

#include <Windows.h>
#include <corecrt_math_defines.h>
#include <random>
#include <xmmintrin.h>
#include <immintrin.h>

struct D3DMATRIX {
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};
};

__forceinline int __wcslen( const wchar_t* s )
{
	int len = 0;

	while (s[len] != L'\0')
	{
		if (s[++len] == L'\0')
			return len;
		if (s[++len] == L'\0')
			return len;
		if (s[++len] == L'\0')
			return len;
		++len;
	}

	return len;
}

#define u uintptr_t
class fvector2d
{
public:
	fvector2d( ) : x( 0.f ), y( 0.f )
	{

	}

	fvector2d( double _x, double _y ) : x( _x ), y( _y )
	{

	}
	~fvector2d( )
	{

	}

	fvector2d operator-( fvector2d v )
	{
		return fvector2d( x - v.x, y - v.y );
	}


	bool is_zero( ) const
	{
		constexpr double epsilon = 1e-6;
		return std::abs( x ) < epsilon && std::abs( y ) < epsilon;
	}

	double x;
	double y;
};

class fvector3d
{
public:
	fvector3d( ) : i( 0.0 ), j( 0.0 ), k( 0.0 )
	{
	}

	fvector3d( double _i, double _j, double _k ) : i( _i ), j( _j ), k( _k )
	{
	}

	~fvector3d( )
	{
	}

	double i;
	double j;
	double k;

	inline double dot( fvector3d v ) const
	{
		return i * v.i + j * v.j + k * v.k;
	}

	inline double length( ) const
	{
		return std::sqrt( i * i + j * j + k * k );
	}

	inline double distance( const fvector3d& v ) const
	{
		return std::sqrt( std::pow( v.i - i, 2.0 ) + std::pow( v.j - j, 2.0 ) + std::pow( v.k - k, 2.0 ) );
	}

	fvector3d operator+( const fvector3d& v ) const
	{
		return fvector3d( i + v.i, j + v.j, k + v.k );
	}

	fvector3d operator-( const fvector3d& v ) const
	{
		return fvector3d( i - v.i, j - v.j, k - v.k );
	}

	fvector3d operator*( double scalar ) const
	{
		return fvector3d( i * scalar, j * scalar, k * scalar );
	}

	fvector3d operator/( double scalar ) const
	{
		return fvector3d( i / scalar, j / scalar, k / scalar );
	}
};

class fvector
{
public:
	fvector( ) : x( 0.f ), y( 0.f ), z( 0.f )
	{

	}

	fvector( double _x, double _y, double _z ) : x( _x ), y( _y ), z( _z )
	{

	}
	~fvector( )
	{

	}

	double x;
	double y;
	double z;

	inline double dot( fvector v )
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline double distance( fvector v )
	{
		return double( sqrtf( powf( v.x - x, 2.0 ) + powf( v.y - y, 2.0 ) + powf( v.z - z, 2.0 ) ) );
	}

	inline double length( ) {
		return sqrt( x * x + y * y + z * z );
	}

	fvector operator+( fvector v )
	{
		return fvector( x + v.x, y + v.y, z + v.z );
	}

	fvector operator-( fvector v )
	{
		return fvector( x - v.x, y - v.y, z - v.z );
	}

	fvector operator/( double flNum )
	{
		return fvector( x / flNum, y / flNum, z / flNum );
	}

	fvector& operator+=(const fvector& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	fvector operator*( double flNum ) { return fvector( x * flNum, y * flNum, z * flNum ); }
	fvector operator*(const fvector& o) const { return { x * o.x, y * o.y, z * o.z }; }
};
struct fquat
{
	double x;
	double y;
	double z;
	double w;
};
struct frotator
{
	double Pitch;
	double Yaw;
	double Roll;
};

struct FTransform
{
	fquat rot;
	fvector translation;
	char pad[4];
	fvector scale;
	char pad1[4];
	D3DMATRIX ToMatrixWithScale( )
	{
		fvector scale_3d(
			(scale.x == 0.0) ? 1.0 : scale.x,
			(scale.y == 0.0) ? 1.0 : scale.y,
			(scale.z == 0.0) ? 1.0 : scale.z
		);

		double x2 = rot.x + rot.x;
		double y2 = rot.y + rot.y;
		double z2 = rot.z + rot.z;

		double xx2 = rot.x * x2;
		double yy2 = rot.y * y2;
		double zz2 = rot.z * z2;
		double yz2 = rot.y * z2;
		double wx2 = rot.w * x2;
		double xy2 = rot.x * y2;
		double wz2 = rot.w * z2;
		double xz2 = rot.x * z2;
		double wy2 = rot.w * y2;

		D3DMATRIX matrix;

		matrix._41 = translation.x;
		matrix._42 = translation.y;
		matrix._43 = translation.z;

		matrix._11 = (1.0 - (yy2 + zz2)) * scale_3d.x;
		matrix._22 = (1.0 - (xx2 + zz2)) * scale_3d.y;
		matrix._33 = (1.0 - (xx2 + yy2)) * scale_3d.z;

		matrix._32 = (yz2 - wx2) * scale_3d.z;
		matrix._23 = (yz2 + wx2) * scale_3d.y;

		matrix._21 = (xy2 - wz2) * scale_3d.y;
		matrix._12 = (xy2 + wz2) * scale_3d.x;

		matrix._31 = (xz2 + wy2) * scale_3d.z;
		matrix._13 = (xz2 - wy2) * scale_3d.x;

		matrix._14 = 0.0f;
		matrix._24 = 0.0f;
		matrix._34 = 0.0f;
		matrix._44 = 1.0f;

		return matrix;
	}
};
inline D3DMATRIX MatrixMultiplication( D3DMATRIX pM1, D3DMATRIX pM2 )
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}
#define PI 3.14159265358979323846f
struct _MATRIX {
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float m[4][4];
	};
};
inline _MATRIX Matrix( fvector Vec4, fvector origin = fvector( 0, 0, 0 ) )
{
	double radPitch = (Vec4.x * double( PI ) / 180.f);
	double radYaw = (Vec4.y * double( PI ) / 180.f);
	double radRoll = (Vec4.z * double( PI ) / 180.f);

	double SP = sinf( radPitch );
	double CP = cosf( radPitch );
	double SY = sinf( radYaw );
	double CY = cosf( radYaw );
	double SR = sinf( radRoll );
	double CR = cosf( radRoll );

	_MATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

static float powf_( float _X, float _Y ) {
	return (_mm_cvtss_f32( _mm_pow_ps( _mm_set_ss( _X ), _mm_set_ss( _Y ) ) ));
}

static float sqrtf_( float _X ) {
	return (_mm_cvtss_f32( _mm_sqrt_ps( _mm_set_ss( _X ) ) ));
}

static double get_cross_distance( double x1, double y1, double x2, double y2 ) {
	return sqrtf( powf( (x2 - x1), 2 ) + powf_( (y2 - y1), 2 ) );
}

struct alignas(16) matrix_elements {
	double m11, m12, m13, m14;
	double m21, m22, m23, m24;
	double m31, m32, m33, m34;
	double m41, m42, m43, m44;

	matrix_elements( ) : m11( 0 ), m12( 0 ), m13( 0 ), m14( 0 ),
		m21( 0 ), m22( 0 ), m23( 0 ), m24( 0 ),
		m31( 0 ), m32( 0 ), m33( 0 ), m34( 0 ),
		m41( 0 ), m42( 0 ), m43( 0 ), m44( 0 ) {
	}
};

struct alignas(16) dbl_matrix {
	union {
		matrix_elements elements;
		double m[4][4];
	};

	dbl_matrix( ) : elements( ) {}

	double& operator()( size_t row, size_t col ) { return m[row][col]; }
	const double& operator()( size_t row, size_t col ) const { return m[row][col]; }
};

struct fplane : fvector {
	double w = 0.0;
};

struct alignas(16) fmatrix : public dbl_matrix {
	fplane x_plane;
	fplane y_plane;
	fplane z_plane;
	fplane w_plane;

	fmatrix( ) : dbl_matrix( ), x_plane( ), y_plane( ), z_plane( ), w_plane( ) {}
};

struct fmatrix3x3 {
	float m[3][3];
};

static inline fvector operator*(const fmatrix3x3& M, const fvector& v)
{
	return fvector{
		M.m[0][0] * v.x + M.m[0][1] * v.y + M.m[0][2] * v.z,
		M.m[1][0] * v.x + M.m[1][1] * v.y + M.m[1][2] * v.z,
		M.m[2][0] * v.x + M.m[2][1] * v.y + M.m[2][2] * v.z
	};
}

static inline float deg2rad(float deg) { return deg * 0.017453292519943295f; } 

static inline fmatrix3x3 build_rotation_matrix(float yawDeg, float pitchDeg, float rollDeg)
{
	float Yaw = deg2rad(yawDeg);
	float Pitch = deg2rad(pitchDeg);
	float Roll = deg2rad(rollDeg);

	float cy = cosf(Yaw), sy = sinf(Yaw);
	float cp = cosf(Pitch), sp = sinf(Pitch);
	float cr = cosf(Roll), sr = sinf(Roll);

	fmatrix3x3 R;

	R.m[0][0] = cy * cp;
	R.m[0][1] = cy * sp * sr - sy * cr;
	R.m[0][2] = cy * sp * cr + sy * sr;

	R.m[1][0] = sy * cp;
	R.m[1][1] = sy * sp * sr + cy * cr;
	R.m[1][2] = sy * sp * cr - cy * sr;

	R.m[2][0] = -sp;
	R.m[2][1] = cp * sr;
	R.m[2][2] = cp * cr;

	return R;
}

inline int screen_w = GetSystemMetrics( SM_CXSCREEN );
inline int screen_h = GetSystemMetrics( SM_CYSCREEN );

constexpr int key_binds[] = {
	VK_RBUTTON,
	VK_LBUTTON,
	VK_MBUTTON,
	VK_XBUTTON1,
	VK_XBUTTON2
};