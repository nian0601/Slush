#include "stdafx.h"

#include "TopDownGameGlobals.h"

#include "Level/Level.h"

TopDownGameGlobals* TopDownGameGlobals::ourInstance = nullptr;

TopDownGameGlobals& TopDownGameGlobals::GetInstance()
{
	if (!ourInstance)
	{
		ourInstance = new TopDownGameGlobals();
	}

	return *ourInstance;
}

void TopDownGameGlobals::Destroy()
{
	FW_SAFE_DELETE(ourInstance);
}

Level& TopDownGameGlobals::GetLevel()
{
	FW_ASSERT(myLevel != nullptr, "Need to set a Level");
	return *myLevel;
}
