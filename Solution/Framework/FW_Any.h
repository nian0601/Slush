#pragma once

class FW_Any
{
public:
	template <typename T>
	void Set(T& someData)
	{
		FW_ASSERT(sizeof(T) < sizeof(char) * 24, "Object of size %i does not fit in FW_Any", sizeof(T));
		memcpy(myData, &someData, sizeof(someData));
	}

	template <typename T>
	T& Get()
	{
		return reinterpret_cast<T&>(myData[0]);
	}

private:
	char myData[24];
	
};
