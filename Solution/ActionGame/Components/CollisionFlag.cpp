#include "stdafx.h"

#include "CollisionFlag.h"

namespace CollisionUtils
{
	namespace
	{
		const char* ourNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
		const char* ourSerializationNames[] = { "environment", "player", "npc", "playerprojectile", "npcprojectile", "pickup" };
		static_assert(IM_ARRAYSIZE(ourNames) == COLLISIONFLAG_COUNT);
		static_assert(IM_ARRAYSIZE(ourSerializationNames) == COLLISIONFLAG_COUNT);
	}

	const char* ToString(CollisionFlag aFlag)
	{
		FW_ASSERT(aFlag >= 0 && aFlag < COLLISIONFLAG_COUNT, "Invalid CollisionFlag");
		return ourNames[aFlag];
	}

	const char* const* GetNames()
	{
		return ourNames;
	}

	const char* const* GetSerializationNames()
	{
		return ourSerializationNames;
	}
}

void GamePhysicsData::OnParseCollisionFlags(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("collisionflag", myCollisionFlag);

	Slush::AssetParser::Handle collidesWithHandle = aComponentHandle.ParseChildElement("collideswith");
	if (collidesWithHandle.IsValid())
	{
		const char* const* serializationNames = CollisionUtils::GetSerializationNames();
		for (int i = 0; i < CollisionUtils::COLLISIONFLAG_COUNT; ++i)
		{
			bool flagSet = (myCollidesWithMask & (1u << i)) != 0;
			collidesWithHandle.ParseBoolField(serializationNames[i], flagSet);

			if (flagSet)
				myCollidesWithMask |= 1u << i;
			else
				myCollidesWithMask &= ~(1u << i);
		}
	}
}

void GamePhysicsData::OnBuildCollisionFlagsUI()
{
	const char* const* names = CollisionUtils::GetNames();
	ImGui::Combo("Collision Flag", &myCollisionFlag, names, CollisionUtils::COLLISIONFLAG_COUNT);

	ImGui::Text("Collides With:");
	for (int i = 0; i < CollisionUtils::COLLISIONFLAG_COUNT; ++i)
	{
		bool flagSet = (myCollidesWithMask & (1u << i)) != 0;
		if (ImGui::Checkbox(names[i], &flagSet))
		{
			if (flagSet)
				myCollidesWithMask |= 1u << i;
			else
				myCollidesWithMask &= ~(1u << i);
		}
	}
}
