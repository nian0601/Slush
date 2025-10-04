#pragma once

class Entity;

class EntityHandle
{
	friend class EntityManager;
	struct ProxyObject
	{
		Entity* myObject = nullptr;
		int myRefCount = 0;
	};

public:
	EntityHandle()
		: myProxy(nullptr)
	{}

	EntityHandle(ProxyObject* aProxy)
		: myProxy(aProxy)
	{
		myProxy->myRefCount++;
	}

	~EntityHandle()
	{
		if (myProxy)
		{
			myProxy->myRefCount--;
			myProxy = nullptr;
		}
	}

	EntityHandle(const EntityHandle& aOther)
	{
		if (myProxy)
			myProxy->myRefCount--;

		myProxy = aOther.myProxy;

		if (myProxy)
			myProxy->myRefCount++;
	}

	EntityHandle& operator=(const EntityHandle& aOther)
	{
		if (this == &aOther)
			return *this;

		if (myProxy)
			myProxy->myRefCount--;

		myProxy = aOther.myProxy;
		myProxy->myRefCount++;
		return *this;
	}

	bool IsValid() const
	{
		return myProxy != nullptr && myProxy->myObject != nullptr;
	}

	Entity* Get() const
	{
		if (!IsValid())
			return nullptr;

		return myProxy->myObject;
	}

	void Clear()
	{
		if (myProxy)
		{
			myProxy->myRefCount--;
			myProxy = nullptr;
		}
	}

	bool operator==(const EntityHandle& aOther)
	{
		if (!IsValid() || !aOther.IsValid())
			return false;

		return myProxy->myObject == aOther.myProxy->myObject;
	}

	bool operator!=(const EntityHandle& aOther)
	{
		return !(*this == aOther);
	}

private:
	ProxyObject* myProxy = nullptr;
};
