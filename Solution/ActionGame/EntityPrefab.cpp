#include "EntityPrefab.h"
#include <imgui\imgui.h>
#include <FW_Math.h>

EntityPrefab::EntityPrefab(const char* aName)
	: myName(aName)
{
}

void EntityPrefab::BuildUI()
{
	if (BaseComponentUI(mySprite.myEnabled, "Sprite", "Add Sprite"))
	{
		ImGui::ColorEdit4("Color", mySprite.myFloatColor);
		ImGui::InputFloat("Radius", &mySprite.myRadius, 1.f, 10.f, "%.2f");
		ImGui::InputFloat2("Size", &mySprite.mySize.x, "%.2f");

		mySprite.myColor = FW_Float_To_ARGB(mySprite.myFloatColor[3], mySprite.myFloatColor[0], mySprite.myFloatColor[1], mySprite.myFloatColor[2]);

		ImGui::TreePop();
	}

	if (BaseComponentUI(myAnimation.myEnabled, "Animation", "Add Animation"))
	{
		ImGui::TreePop();
	}

	if (BaseComponentUI(myProjectileShooting.myEnabled, "Projectile Shooter", "Add Projectile Shooter"))
	{
		ImGui::InputFloat("Cooldown", &myProjectileShooting.myCooldown, 0.1f, 1.f, "%.2f");
		ImGui::TreePop();
	}

	if (BaseComponentUI(myPlayerController.myEnabled, "Player Controller", "Add Player Controller"))
	{
		ImGui::TreePop();
	}

	if (BaseComponentUI(myNPCController.myEnabled, "NPC Controller", "Add NPC Controller"))
	{
		ImGui::TreePop();
	}

	if (BaseComponentUI(myHealth.myEnabled, "Health", "Add Health"))
	{
		ImGui::InputInt("Max Health", &myHealth.myMaxHealth);
		ImGui::TreePop();
	}

	if (BaseComponentUI(myPhysics.myEnabled, "Physics", "Add Physics"))
	{
		ImGui::Checkbox("Is Static", &myPhysics.myStatic);
		ImGui::Checkbox("Match Sprite", &myPhysics.myMatchSprite);

		ImGui::InputFloat("Radius", &myPhysics.myRadius, 1.f, 10.f, "%.2f");
		ImGui::InputFloat2("Size", &myPhysics.mySize.x, "%.2f");

		ImGui::TreePop();
	}

	if (BaseComponentUI(myRemoveOnCollision.myEnabled, "Remove On Collision", "Add Remove On Collision"))
	{
		ImGui::TreePop();
	}
}

bool EntityPrefab::BaseComponentUI(bool& aEnabledFlag, const char* aComponentLabel, const char* aAddComponentLabel)
{
	if (aEnabledFlag)
	{
		if (ImGui::TreeNode(aComponentLabel))
		{
			if (ImGui::Button("Remove"))
				aEnabledFlag = false;

			return true;
		}
	}
	else
	{
		aEnabledFlag = ImGui::Button(aAddComponentLabel);
	}

	return false;
}
