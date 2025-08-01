#pragma once
#include <stdlib.h>
#include "FW_Vector2.h"
#include "FW_Vector3.h"
#define FW_PI 3.14159265358979323846f
#define FW_EPSILON 0.0001f

inline float FW_DegreesToRadians(float anAngle)
{
	return anAngle * FW_PI / 180.f;
}

inline float FW_RadiansToDegrees(float anAngle)
{
	return anAngle * 180.f / FW_PI;
}

inline float FW_RandFloat()
{
	return static_cast<float>(rand() % 10000) / 10000.f;
}

inline float FW_RandFloat(float aMin, float aMax) 
{
	return aMin + (aMax - aMin) * FW_RandFloat();
}

inline float FW_RandClamped()
{
	return FW_RandFloat() - FW_RandFloat();
}

inline int FW_RandInt(int aMin, int aMax)
{
	return (rand() & (aMax - aMin + 1)) + aMin;
}

inline float FW_SignedAngle(const Vector2f& aVector)
{
	Vector2f normalized = GetNormalized(aVector);
	return atan2f(normalized.y, normalized.x);
}

inline float FW_AngleBetweenVectors(const Vector2f& a, const Vector2f& b)
{
	Vector2f normalizedA = GetNormalized(a);
	Vector2f normalizedB = GetNormalized(b);

	float angle = Dot(normalizedA, normalizedB);
	return acosf(angle);
}

inline float FW_AngleBetweenVectors(const Vector3f& a, const Vector3f& b)
{
	Vector3f normalizedA = GetNormalized(a);
	Vector3f normalizedB = GetNormalized(b);

	float angle = Dot(normalizedA, normalizedB);
	return acosf(angle);
}

inline Vector2f FW_RandomVector2f()
{
	return { FW_RandFloat(), FW_RandFloat() };
}

inline Vector3f FW_RandomVector3f()
{
	return { FW_RandFloat(), FW_RandFloat(), FW_RandFloat() };
}

inline Vector3f FW_RandomVector3f(float aMin, float aMax)
{
	return { FW_RandFloat(aMin, aMax), FW_RandFloat(aMin, aMax), FW_RandFloat(aMin, aMax) };
}

inline Vector3f FW_RandomUnitSphereVector()
{
	while (true)
	{
		Vector3f v = FW_RandomVector3f(-1.f, 1.f);
		if(Length2(v) >= 1.f) continue;

		return v;
	}
}

inline Vector3f FW_RandomUnitVector()
{
	return GetNormalized(FW_RandomUnitSphereVector());
}

inline Vector3f FW_RandomInHemisphereVector(const Vector3f& aNormal)
{
	Vector3f inUnitSphere = FW_RandomUnitSphereVector();

	if (Dot(inUnitSphere, aNormal) > 0.f)
		return inUnitSphere;

	return -inUnitSphere;
}

inline Vector3f FW_RandomInUnitDisk()
{
	while (true)
	{
		Vector3f v = Vector3f(FW_RandFloat(-1.f, 1.f), FW_RandFloat(-1.f, 1.f), 0.f);
		if(Length2(v) >= 1.f) continue;

		return v;
	}
}

inline float FW_RoundToInt(float aFloat)
{
	int intCast = static_cast<int>(aFloat > 0.f ? aFloat + 0.5f : aFloat - 0.5f);
	return static_cast<float>(intCast);
}

inline void FW_RoundToInt(Vector2f& aVector)
{
	aVector.x = FW_RoundToInt(aVector.x);
	aVector.y = FW_RoundToInt(aVector.y);
}

template <typename T>
inline T FW_Min(const T& aFirst, const T& aSecond)
{
	return aFirst < aSecond ? aFirst : aSecond;
}

template <typename T>
inline T FW_Max(const T& aFirst, const T& aSecond)
{
	return aFirst > aSecond ? aFirst : aSecond;
}

inline float FW_Lerp(float aStart, float aEnd, float aAlpha)
{
	return (aStart + (aEnd - aStart) * aAlpha);
}

inline Vector2f FW_Lerp(Vector2f aStart, Vector2f aEnd, float aAlpha)
{
	return (aStart + (aEnd - aStart) * aAlpha);
}

inline float FW_UnLerp(float aStart, float aEnd, float aValue)
{
	return (aValue - aStart) / (aEnd - aStart);
}

inline float FW_Remap(float aOriginalStart, float aOriginalEnd, float aValue, float aNewStart, float aNewEnd)
{
	float lerpValue = FW_UnLerp(aOriginalStart, aOriginalEnd, aValue);
	return FW_Lerp(aNewStart, aNewEnd, lerpValue);
}

template <typename T>
inline T FW_Clamp(const T& aValue, const T& aMin, const T& aMax)
{
	if (aValue < aMin) return aMin;
	if (aValue > aMax) return aMax;
	return aValue;
}

template <typename T>
inline T FW_Square(const T& aValue)
{
	return aValue * aValue;
}

inline float FW_Equal(float A, float B)
{
	return abs(A - B) <= FW_EPSILON;
}

inline int FW_RandomColor()
{
	int color = 0;

	Vector3f randColor = FW_RandomVector3f(0.f, 1.f);

	color |= unsigned char(FW_Clamp(randColor.x, 0.f, 0.999f) * 255.99f) << 0;
	color |= unsigned char(FW_Clamp(randColor.y, 0.f, 0.999f) * 255.99f) << 8;
	color |= unsigned char(FW_Clamp(randColor.z, 0.f, 0.999f) * 255.99f) << 16;
	color |= 255 << 24;

	return color;
}

inline int FW_Float_To_ARGB(float a, float r, float g, float b)
{
	int color = 0;

	color |= unsigned char(FW_Clamp(a, 0.f, 0.999f) * 255.99f) << 24;
	color |= unsigned char(FW_Clamp(r, 0.f, 0.999f) * 255.99f) << 16;
	color |= unsigned char(FW_Clamp(g, 0.f, 0.999f) * 255.99f) << 8;
	color |= unsigned char(FW_Clamp(b, 0.f, 0.999f) * 255.99f) << 0;

	return color;
}

inline void FW_ARGB_To_RGBAFloat(int aARGB, float aFloatArray[4])
{
	short alpha = (aARGB & 0xFF000000) >> 24;
	short red = (aARGB & 0x00FF0000) >> 16;
	short green = (aARGB & 0x0000FF00) >> 8;
	short blue = (aARGB & 0x000000FF) >> 0;

	aFloatArray[0] = red / 255.f;
	aFloatArray[1] = green / 255.f;
	aFloatArray[2] = blue / 255.f;
	aFloatArray[3] = alpha / 255.f;
}

inline Vector3f FW_ARGB_To_Vector(unsigned int aARGB)
{
	short red = (aARGB & 0x00FF0000) >> 16;
	short green = (aARGB & 0x0000FF00) >> 8;
	short blue = (aARGB & 0x000000FF) >> 0;

	return Vector3f(red / 255.f, green / 255.f, blue / 255.f);
}

inline short FW_Interpolate_ColorChannel(short aFirst, short aSecond, float aInterpolator)
{
	return static_cast<short>(FW_Lerp(aFirst / 255.f, aSecond / 255.f, aInterpolator) * 255.f);
}

inline int FW_Interpolate_Color(int aFirst, int aSecond, float aInterpolator)
{
	//short alpha = (aARGB & 0xFF000000) >> 24;
	short red1 = (aFirst& 0x00FF0000) >> 16;
	short green1 = (aFirst & 0x0000FF00) >> 8;
	short blue1 = (aFirst & 0x000000FF) >> 0;

	//short alpha = (aARGB & 0xFF000000) >> 24;
	short red2 = (aSecond & 0x00FF0000) >> 16;
	short green2 = (aSecond & 0x0000FF00) >> 8;
	short blue2 = (aSecond & 0x000000FF) >> 0;

	short lerpedRed = FW_Interpolate_ColorChannel(red1, red2, aInterpolator);
	short lerpedGreen = FW_Interpolate_ColorChannel(green1, green2, aInterpolator);
	short lerpedBlue = FW_Interpolate_ColorChannel(blue1, blue2, aInterpolator);

	int color = 0;

	color |= 255 << 24;
	color |= lerpedRed << 16;
	color |= lerpedGreen << 8;
	color |= lerpedBlue;

	return color;
}