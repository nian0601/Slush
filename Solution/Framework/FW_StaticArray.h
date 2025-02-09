#pragma once
#include <cassert>

template<typename Type, int Size>
class FW_StaticArray
{
public:
	FW_StaticArray() {}
	FW_StaticArray(const FW_StaticArray& aFW_StaticArray);

	FW_StaticArray& operator=(const FW_StaticArray& aFW_StaticArray);

	inline const Type& operator[](const int& aIndex) const;
	inline Type& operator[](const int& aIndex);
	inline int Count() const { return Size; };

	inline void Insert(int aIndex, const Type& aObject);
	inline void Fill(const Type& aObject);

	inline void DeleteAll();

	typedef Type* iterator;
	typedef const Type* const_iterator;
	iterator begin() { return &myData[0]; }
	const_iterator begin() const { return &myData[0]; }
	iterator end() { return &myData[Size]; }
	const_iterator end() const { return &myData[Size]; }

private:
	Type myData[Size];
};

template<typename Type, int Size>
FW_StaticArray<Type, Size>::FW_StaticArray(const FW_StaticArray& aFW_StaticArray)
{
	operator=(aFW_StaticArray);
}

template<typename Type, int Size>
FW_StaticArray<Type, Size>& FW_StaticArray<Type, Size>::operator=(const FW_StaticArray& aFW_StaticArray)
{
	for (int i = 0; i < Size; ++i)
		myData[i] = aFW_StaticArray.myData[i];

	return *this;
}

template<typename Type, int Size>
inline const Type& FW_StaticArray<Type, Size>::operator[](const int& aIndex) const
{
	FW_ASSERT(aIndex >= 0, "Index has to be 0 or more.");
	FW_ASSERT(aIndex < Size, "a index out of bounds!");
	return myData[aIndex];
}

template<typename Type, int Size>
inline Type& FW_StaticArray<Type, Size>::operator[](const int& aIndex)
{
	FW_ASSERT(aIndex >= 0, "Index has to be 0 or more.");
	FW_ASSERT(aIndex < Size, "a index out of bounds!");
	return myData[aIndex];
}

template<typename Type, int Size>
inline void FW_StaticArray<Type, Size>::Insert(int aIndex, const Type& aObject)
{
	FW_ASSERT(aIndex >= 0, "Index has to be 0 or more.");
	FW_ASSERT(aIndex < Size, "a index out of bounds!");
	for (int i = Size - 2; i >= aIndex; --i)
		myData[i + 1] = myData[i];

	myData[aIndex] = aObject;
}

template<typename Type, int Size>
inline void FW_StaticArray<Type, Size>::Fill(const Type& aObject)
{
	for (int i = 0; i < Size; ++i)
		myData[i] = aObject;
}

template<typename Type, int Size>
inline void FW_StaticArray<Type, Size>::DeleteAll()
{
	for (int i = 0; i < Size; ++i)
	{
		delete myData[i];
		myData[i] = nullptr;
	}
}