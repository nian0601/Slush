#pragma once
#include "FW_Vector2.h"

namespace FW_Intersection
{
	static float sign(const Vector2f& aV1, const Vector2f& aV2, const Vector2f& aV3)
	{
		return (aV1.x - aV3.x) * (aV2.y - aV3.y) - (aV2.x - aV3.x) * (aV1.y - aV3.y);
	}

	inline bool PointInsideTriangle(const Vector2f& aPoint, const Vector2f& aV1, const Vector2f& aV2, const Vector2f& aV3)
	{
		bool b1 = sign(aPoint, aV1, aV2) < 0.f;
		bool b2 = sign(aPoint, aV2, aV3) < 0.f;
		bool b3 = sign(aPoint, aV3, aV1) < 0.f;

		return (b1 == b2) && (b1 == b3);
	}

	struct LineSegment
	{
		Vector2f myStart;
		Vector2f myEnd;
	};

	struct Line
	{
		void FromSegment(const LineSegment& aSegment)
		{
			Vector2f dir = aSegment.myEnd - aSegment.myStart;
			myNormal = Vector2f(-dir.y, dir.x);
			Normalize(myNormal);
			myDist = Dot(aSegment.myStart, myNormal);
		}
		Vector2f myNormal;
		float myDist;
	};

	inline bool CheckOverlap(const Vector2f& aDir, float aLenght, const Vector2f& aStart, const Vector2f& aEnd, Vector2f* aIntersectionPoint = nullptr)
	{
		float projection = Dot(aDir, aEnd - aStart);
		if (projection < aLenght && projection > 0)
		{
			if (aIntersectionPoint)
				*aIntersectionPoint = aStart;

			return true;
		}

		return false;
	}

	inline bool LineSegmentVsLineSegment(const LineSegment& aFirst, const LineSegment& aSecond, Vector2f* aIntersectionPoint = nullptr)
	{
		if (Length2(aFirst.myEnd - aFirst.myStart) == 0.f)
			return false;
		if (Length2(aSecond.myEnd - aSecond.myStart) == 0.f)
			return false;

		Line line1;
		line1.FromSegment(aFirst);
		Line line2;
		line2.FromSegment(aSecond);

		float denomX = line1.myNormal.x * line2.myNormal.y - line2.myNormal.x * line1.myNormal.y;
		float denomY = line1.myNormal.x * line2.myNormal.y - line2.myNormal.x * line1.myNormal.y;
		if (denomX == 0 || denomY == 0)
		{
			//Lines are parallell, if dist are different then ther is 0 overlap
			if (line1.myDist != line2.myDist)
				return false;

			//Lines are parallell and overlapping, solve the overlap
			Vector2f dir1 = aFirst.myEnd - aFirst.myStart;
			float length1 = Length(dir1);
			dir1 /= length1;

			Vector2f dir2 = aSecond.myEnd - aSecond.myStart;
			float length2 = Length(dir2);
			dir2 /= length2;

			if (CheckOverlap(dir1, length1, aSecond.myEnd, aFirst.myStart, aIntersectionPoint))
				return true;
			if (CheckOverlap(dir1, length1, aSecond.myStart, aFirst.myStart, aIntersectionPoint))
				return true;
			if (CheckOverlap(dir2, length2, aFirst.myEnd, aSecond.myStart, aIntersectionPoint))
				return true;
			if (CheckOverlap(dir2, length2, aFirst.myStart, aSecond.myStart, aIntersectionPoint))
				return true;

			return false;
		}

		
		float numX = line2.myNormal.y * line1.myDist - line1.myNormal.y * line2.myDist;
		float numY = line1.myNormal.x * line2.myDist - line2.myNormal.x * line1.myDist;
		const Vector2f intersection = { numX / denomX, numY / denomY };

		Vector2f dir1 = aFirst.myEnd - aFirst.myStart;
		Vector2f dir1Intersect = intersection - aFirst.myStart;

		if (Dot(dir1, dir1Intersect) < 0 || Length2(dir1Intersect) > Length2(dir1))
			return false;

		Vector2f dir2 = aSecond.myEnd - aSecond.myStart;
		Vector2f dir2Intersect = intersection - aSecond.myStart;

		if (Dot(dir2, dir2Intersect) < 0 || Length2(dir2Intersect) > Length2(dir2))
			return false;

		if (aIntersectionPoint)
			*aIntersectionPoint = intersection;
		
		return true;
	}
}
