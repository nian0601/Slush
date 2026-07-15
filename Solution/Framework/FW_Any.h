#pragma once

class FW_Any
{
public:
	template <typename T>
	void Set(T& someData)
	{
		static_assert(sizeof(T) < sizeof(char) * 24, "Object does not fit in FW_Any");
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
