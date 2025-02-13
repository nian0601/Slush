#pragma once

#include <cmath>
#include "FW_Assert.h"

template<typename T>
class FW_Vector2
{
public:
	FW_Vector2() { x = 0; y = 0; }
	FW_Vector2(T aValue) { x = aValue; y = aValue; }
	FW_Vector2(T aX, T aY) { x = aX; y = aY; }
	~FW_Vector2() {};

	FW_Vector2<T> operator-()
	{
		FW_Vector2<T> tmp = *this;
		tmp.x = -x;
		tmp.y = -y;
		return tmp;
	}

	T x;
	T y;
};

template <typename T>
T Length(const FW_Vector2<T>& aVector)
{
	return T(sqrt(Length2(aVector)));
}

template <typename T>
T Length2(const FW_Vector2<T>& aVector)
{
	return Dot(aVector, aVector);
}

template <typename T>
void Normalize(FW_Vector2<T>& aVector)
{
	aVector = GetNormalized(aVector);
}

template <typename T>
FW_Vector2<T> GetNormalized(const FW_Vector2<T>& aVector)
{
	T length = Length(aVector);
	if (length == 0)
	{
		return FW_Vector2<T>();
	}
	return FW_Vector2<T>(aVector / length);
}

template <typename T>
FW_Vector2<T> ClampLenght(const FW_Vector2<T>& aVector, float aLenght)
{
	FW_Vector2<T> result = aVector;

	float lenght = Length(result);
	if (lenght > aLenght)
	{
		result /= lenght;
		result *= aLenght;
	}

	return result;
}

template <typename T>
T Dot(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	return aFirstVector.x * aSecondVector.x + aFirstVector.y * aSecondVector.y;
}

template <typename T>
T Cross(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	return aFirstVector.x * aSecondVector.y - aFirstVector.y * aSecondVector.x;
}

template <typename T>
FW_Vector2<T> Cross(const FW_Vector2<T>& aVector, T aScalar)
{
	return FW_Vector2<T>(aScalar * aVector.y, -aScalar * aVector.x);
}

template <typename T>
FW_Vector2<T> Cross(T aScalar, const FW_Vector2<T>& aVector)
{
	return FW_Vector2<T>(-aScalar * aVector.y, aScalar * aVector.x);
}

template <typename T>
FW_Vector2<T> ReflectAround(const FW_Vector2<T>& aVector, const FW_Vector2<T>& aReflectionVector)
{
	return aVector - static_cast<T>(2) * aReflectionVector * Dot(aReflectionVector, aVector);
}

template <typename T>
FW_Vector2<T> Rotate(const FW_Vector2<T>& aVector, float aRadians)
{
	float c = cos(aRadians);
	float s = sin(aRadians);

	FW_Vector2<T> result;
	result.x = (c * aVector.x) + (-s * aVector.y);
	result.y = (s * aVector.x) + (c * aVector.y);
	return result;
}

// operator with vector
template <typename T>
bool operator==(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	return aFirstVector.x == aSecondVector.x && aFirstVector.y == aSecondVector.y;
}

template <typename T>
bool operator!=(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	return !(aFirstVector == aSecondVector);
}

template <typename T>
FW_Vector2<T> operator+(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_Vector2<T> result(aFirstVector);
	result += aSecondVector;
	return result;
}
template <typename T>
FW_Vector2<T> operator+=(FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	aFirstVector.x += aSecondVector.x;
	aFirstVector.y += aSecondVector.y;
	return aFirstVector;
}

template <typename T>
FW_Vector2<T> operator-(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_Vector2<T> result(aFirstVector);
	result -= aSecondVector;
	return result;
}
template <typename T>
FW_Vector2<T> operator-=(FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	aFirstVector.x -= aSecondVector.x;
	aFirstVector.y -= aSecondVector.y;
	return aFirstVector;
}

template <typename T>
FW_Vector2<T> operator*(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_Vector2<T> result(aFirstVector);
	result *= aSecondVector;
	return result;
}
template <typename T>
FW_Vector2<T> operator*=(FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	aFirstVector.x *= aSecondVector.x;
	aFirstVector.y *= aSecondVector.y;
	return aFirstVector;
}

template <typename T>
FW_Vector2<T> operator/(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_Vector2<T> result(aFirstVector);
	result /= aSecondVector;
	return result;
}
template <typename T>
FW_Vector2<T> operator/=(FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_ASSERT(aSecondVector.x != 0 && aSecondVector.y != 0, "Division by zero.");
	aFirstVector.x /= aSecondVector.x;
	aFirstVector.y /= aSecondVector.y;
	return aFirstVector;
}

// operator with scalar
template <typename T>
FW_Vector2<T> operator+(const FW_Vector2<T>& aVector, T aScalar)
{
	FW_Vector2<T> result(aVector);
	result += aScalar;
	return result;
}
template <typename T>
FW_Vector2<T> operator+(T aScalar, const FW_Vector2<T>& aVector)
{
	return aVector + aScalar;
}
template <typename T>
FW_Vector2<T> operator+=(FW_Vector2<T>& aVector, T aScalar)
{
	aVector.x += aScalar;
	aVector.y += aScalar;
	return aVector;
}

template <typename T>
FW_Vector2<T> operator-(const FW_Vector2<T>& aVector, T aScalar)
{
	FW_Vector2<T> result(aVector);
	result -= aScalar;
	return result;
}

template <typename T>
FW_Vector2<T> operator-=(FW_Vector2<T>& aVector, T aScalar)
{
	aVector.x -= aScalar;
	aVector.y -= aScalar;
	return aVector;
}

template <typename T>
FW_Vector2<T> operator*(const FW_Vector2<T>& aVector, T aScalar)
{
	FW_Vector2<T> result(aVector);
	result *= aScalar;
	return result;
}
template <typename T>
FW_Vector2<T> operator*(T aScalar, const FW_Vector2<T>& aVector)
{
	return aVector * aScalar;
}
template <typename T>
FW_Vector2<T> operator*=(FW_Vector2<T>& aVector, T aScalar)
{
	aVector.x *= aScalar;
	aVector.y *= aScalar;
	return aVector;
}

template <typename T>
FW_Vector2<T> operator/(const FW_Vector2<T>& aVector, T aScalar)
{
	FW_Vector2<T> result(aVector);
	result /= aScalar;
	return result;
}
template <typename T>
FW_Vector2<T> operator/=(FW_Vector2<T>& aVector, T aScalar)
{
	FW_ASSERT(aScalar != 0, "Division by zero.");
	aVector.x /= aScalar;
	aVector.y /= aScalar;
	return aVector;
}

template <typename T>
FW_Vector2<T> operator-(const FW_Vector2<T>& aVector)
{
	return FW_Vector2<T>(-aVector.x, -aVector.y);
}

//typedefs
typedef FW_Vector2<float> Vector2f;
typedef FW_Vector2<int> Vector2i;

inline Vector2i ToInt(const Vector2f& aVector)
{
	Vector2i result;
	result.x = static_cast<int>(aVector.x);
	result.y = static_cast<int>(aVector.y);
	return result;
}