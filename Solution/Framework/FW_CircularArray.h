#pragma once
#include "FW_Math.h"

template<typename ObjectType, int ElementCount>
class FW_CircularArray
{
public:
	FW_CircularArray() { myData = new ObjectType[ElementCount]; }
	~FW_CircularArray() { delete[] myData; }

	ObjectType& Add()
	{
		ObjectType& object = myData[myCurrentIndex];
		++myCurrentIndex;

		myTotalCount = FW_Min(++myTotalCount, ElementCount);
		if (myCurrentIndex >= ElementCount)
			myCurrentIndex = 0;

		return object;
	}

	void Add(const ObjectType& anObject)
	{
		ObjectType& addedObject = Add();
		addedObject = anObject;
	}

	void RemoveAll()
	{
		myCurrentIndex = 0;
		myTotalCount = 0;
	}

	void DeleteAll()
	{
		for (int i = 0; i < myTotalCount; ++i)
		{
			delete myData[i];
			myData[i] = nullptr;
		}

		myCurrentIndex = 0;
		myTotalCount = 0;
	}

	ObjectType& operator[](const int& aIndex)
	{
		int realIndex = (myCurrentIndex + aIndex) % myTotalCount;
		return myData[realIndex];
	}

	const ObjectType& operator[](const int& aIndex) const
	{
		int realIndex = (myCurrentIndex + aIndex) % myTotalCount;
		return myData[realIndex];
	}

	typedef ObjectType* iterator;
	typedef const ObjectType* const_iterator;
	iterator begin() { return &myData[0]; }
	const_iterator begin() const { return &myData[0]; }
	iterator end() { return &myData[myTotalCount]; }
	const_iterator end() const { return &myData[myTotalCount]; }

	ObjectType* myData = nullptr;
	int myCurrentIndex = 0;
	int myTotalCount = 0;
};