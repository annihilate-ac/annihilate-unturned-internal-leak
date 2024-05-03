#pragma once

#include <cmath>
#include <cstdint>

#define M_PI 3.14159265358979323846f
#define M_RADPI	57.295779513082f
#define M_PI_F ((float)(M_PI))
#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))
#define atan2(a, b) ((float)atan2((double)(a), (double)(b)))

class vec2_t
{
public:

	float x, y;

	vec2_t() : x(0.f), y(0.f)
	{ }

	vec2_t(float _x, float _y) : x(_x), y(_y)
	{ }

	~vec2_t()
	{ }

	float distance(vec2_t v)
	{ return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0))); }

	float length()
	{ return sqrt(x * x + y * y); }

	float magnitude() 
	{ return sqrt(x * x + y * y); }

	vec2_t normalized() 
	{

		float mag = magnitude();
		if (mag == 0) return { 0, 0 };
		return { x / mag, y / mag };

	}

	bool operator==(const vec2_t& in)
	{ return (this->x == in.x && this->y == in.y); }

	vec2_t operator*(float s)
	{ return vec2_t(x * s, y * s); }

	vec2_t operator+(const vec2_t& v)
	{ return vec2_t(x + v.x, y + v.y); }

	vec2_t operator-(const vec2_t& v)
	{ return vec2_t(x - v.x, y - v.y); }

	vec2_t operator+=(const vec2_t& v)
	{ return vec2_t(x += v.x, y += v.y); }

	vec2_t operator/=(const vec2_t& v)
	{ return vec2_t(x /= v.x, y /= v.y); }

	vec2_t& operator/(float fl) 
	{

		x / fl;
		y / fl;
		return *this;

	}

};

class vec3_t
{
public:

	float x, y, z;

	vec3_t() : x(0.f), y(0.f), z(0.f)
	{ }

	vec3_t(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{ }

	~vec3_t()
	{ }

	float length() const
	{ return sqrt((x * x) + (y * y) + (z * z)); }

	float dot(const vec3_t& v)
	{ return x * v.x + y * v.y + z * v.z; }

	bool non_zero() 
	{ return x && y && z; }

	float distance(const vec3_t& b)
	{

		float num = this->x - b.x;
		float num2 = this->y - b.y;
		float num3 = this->z - b.z;

		return (float)sqrt((double)(num * num + num2 * num2 + num3 * num3));

	}

	float length_2d() const
	{
		return sqrt(x * x + y * y);
	}

	float magnitude()
	{ return (float)sqrt((double)(x * x + y * y + z * z)); }

	vec3_t normalized()
	{

		float mag = magnitude();

		vec3_t value;

		value.x /= mag;
		value.y /= mag;
		value.z /= mag;

		return value;

	}

	vec3_t cross(const vec3_t& v2)
	{

		vec3_t cross_product;
		cross_product.x = y * v2.z - z * v2.y;
		cross_product.y = z * v2.x - x * v2.z;
		cross_product.z = x * v2.y - y * v2.x;

		return cross_product;

	}

	void clamp() 
	{

		if (x > 75.f)
		{
			x = 75.f;
		}
		else if (x < -75.f)
		{
			x = -75.f;
		}
		if (z < -180)
		{
			z += 360.0f;
		}
		else if (z > 180)
		{
			z -= 360.0f;
		}
		y = 0.f;

	}

	bool almost_equal(const vec3_t& other, float epsilon = 0.0001f) const 
	{

		float diffX = std::abs(x - other.x);
		float diffY = std::abs(y - other.y);
		float diffZ = std::abs(z - other.z);

		return (diffX <= epsilon) && (diffY <= epsilon) && (diffZ <= epsilon);

	}

	bool operator==(const vec3_t& in) const
	{ return (x == in.x && y == in.y && z == in.z); }

	bool operator!=(const vec3_t& in) const
	{ return (x != in.x || y != in.y || z != in.z); }

	vec3_t operator+(const vec3_t& v)
	{ return vec3_t(x + v.x, y + v.y, z + v.z); }

	vec3_t operator-(const vec3_t& v) const
	{ return vec3_t(x - v.x, y - v.y, z - v.z); }

	vec3_t operator*(float number) const
	{ return vec3_t(x * number, y * number, z * number); }

	vec3_t& operator/(float fl)
	{

		x / fl;
		y / fl;
		z / fl;
		return *this;

	}

	vec3_t& operator/=(float fl)
	{

		x /= fl;
		y /= fl;
		z /= fl;
		return *this;

	}

	vec3_t& operator+=(const vec3_t& v)
	{

		x += v.x;
		y += v.y;
		z += v.z;
		return *this;

	}

	vec3_t& operator-=(const vec3_t& v)
	{

		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;

	}

	inline vec3_t operator*(const vec3_t& rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
	inline vec3_t operator/(const vec3_t& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }

};

//void rotate_triangle(std::array<ImVec2, 3>& points, float rotation)
//{
//
//	const auto pointsCenter = (points.at(0) + points.at(1) + points.at(2)) / 3;
//
//	for (auto& point : points)
//	{
//
//		point -= pointsCenter;
//
//		const auto tempX = point.x;
//		const auto tempY = point.y;
//
//		const auto theta = DEG2RAD(rotation);
//		const auto c = cos(theta);
//		const auto s = sin(theta);
//
//		point.x = tempX * c - tempY * s;
//		point.y = tempX * s + tempY * c;
//
//		point += pointsCenter;
//
//	}
//
//}

ImVec2 RotateVertex(const ImVec2& p, const ImVec2& v, float angle) {
	// convert theta angle to sine and cosine representations.
	float c = std::cos(DEG2RAD(angle));
	float s = std::sin(DEG2RAD(angle));

	return {
		p.x + (v.x - p.x) * c - (v.y - p.y) * s,
		p.y + (v.x - p.x) * s + (v.y - p.y) * c
	};
}

vec3_t calculate_angle(const vec3_t& src, const vec3_t& dst)
{

	vec3_t angles;

	vec3_t delta = src - dst;
	float hyp = delta.length_2d();

	angles.y = std::atanf(delta.y / delta.x) * M_RADPI;
	angles.x = std::atanf(-delta.z / hyp) * -M_RADPI;
	angles.z = 0.0f;

	if (delta.x >= 0.0f)
		angles.y += 180.0f;

	return angles;

}