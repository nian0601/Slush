#include "stdafx.h"

#include "StatsComponent.h"
#include "Graphics\Texture.h"
#include "Core\Assets\AssetStorage.h"

void StatsUpgradeData::OnParse(Slush::AssetParser::Handle aRootHandle)
{
	const char* statTypeNames[] = { "cooldown", "damage", "experiencegain"};
	static_assert(IM_ARRAYSIZE(statTypeNames) == StatType::NUM_STATS);

	for (int i = 0; i < IM_ARRAYSIZE(statTypeNames); ++i)
	{
		Slush::AssetParser::Handle statHandle = aRootHandle.ParseChildElement(statTypeNames[i]);

		StatData& statData = myStatDatas[i];
		statHandle.ParseIntField("maxupgrades", statData.myMaxUpgrades);
		statHandle.ParseFloatField("increaseperupgrade", statData.myIncreasePerUpgrade);
		statHandle.ParseStringField("icon", statData.myIconTextureID);
		Slush::AssetParser::Handle textureRectHandle = statHandle.ParseChildElement("iconTextureRect");
		if (textureRectHandle.IsValid())
		{
			textureRectHandle.ParseIntField("x", statData.myIconTextureRect.myTopLeft.x);
			textureRectHandle.ParseIntField("y", statData.myIconTextureRect.myTopLeft.y);
			textureRectHandle.ParseIntField("width", statData.myIconTextureRect.myExtents.x);
			textureRectHandle.ParseIntField("height", statData.myIconTextureRect.myExtents.y);

			if (aRootHandle.IsReading())
				statData.myIconTextureRect = MakeRectFromTopLeft(statData.myIconTextureRect.myTopLeft, statData.myIconTextureRect.myExtents);
		}
	}
}

void StatsUpgradeData::BuildUI()
{
	const char* statTypeNames[] = { "Cooldown", "Damage", "Experience Gain" };
	static_assert(IM_ARRAYSIZE(statTypeNames) == StatType::NUM_STATS);

	for (int i = 0; i < IM_ARRAYSIZE(statTypeNames); ++i)
	{
		ImGui::PushID(i);

		if (ImGui::CollapsingHeader(statTypeNames[i]))
		{
			ImGui::Indent();

			StatData& statData = myStatDatas[i];

			if (statData.myIconTextureID.Empty())
			{
				ImGui::BeginDisabled(true);
				ImGui::Button("Icon", { 64, 64 });
				ImGui::EndDisabled();
			}
			else
			{
				Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
				if (const Slush::Texture* texture = assets.GetAsset<Slush::Texture>(statData.myIconTextureID))
				{
					sf::FloatRect texRect;
					texRect.position.x = static_cast<float>(statData.myIconTextureRect.myTopLeft.x);
					texRect.position.y = static_cast<float>(statData.myIconTextureRect.myTopLeft.y);
					texRect.size.x = static_cast<float>(statData.myIconTextureRect.myExtents.x);
					texRect.size.y = static_cast<float>(statData.myIconTextureRect.myExtents.y);

					sf::Vector2f size;
					size.x = static_cast<float>(FW_Max(statData.myIconTextureRect.myExtents.x, 48));
					size.y = static_cast<float>(FW_Max(statData.myIconTextureRect.myExtents.y, 48));

					ImGui::Image(*texture->GetSFMLTexture(), size, texRect);
				}
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* imguiPayload = ImGui::AcceptDragDropPayload("TextureDragPayload"))
				{
					Slush::TextureDragPayload* payload = static_cast<Slush::TextureDragPayload*>(imguiPayload->Data);
					statData.myIconTextureID = payload->myTexture->GetAssetName();
					statData.myIconTextureRect = payload->myTextureRect;
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::SetNextItemWidth(100.f);
			ImGui::InputInt("Max Upgrades", &statData.myMaxUpgrades);

			ImGui::SetNextItemWidth(100.f);
			ImGui::InputFloat("Increase Per Upgrade", &statData.myIncreasePerUpgrade, 0.05f, 1.f, "%.2f");

			ImGui::Unindent();
		}

		ImGui::PopID();
	}
}

//////////////////////////////////////////////////////////////////////////

void StatsComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle;
}

void StatsComponent::Data::OnBuildUI()
{
}

//////////////////////////////////////////////////////////////////////////

void StatsComponent::RuntimeStat::Upgrade(float aValue)
{
	myStatModifierValue += aValue;
	++myNumberOfUpgrades;
}

void StatsComponent::OnEnterWorld()
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	myUpgradeData = assets.GetAsset<StatsUpgradeData>("statupgrades");
}

bool StatsComponent::CanUpgradeAnyStat() const
{
	for (int i = 0; i < StatType::NUM_STATS; ++i)
	{
		if (CanUpgradeStat(StatType(i)))
			return true;
	}

	return false;
}

void StatsComponent::UpgradeStat(StatType aStat)
{
	RuntimeStat& runtime = myRuntimeStats[aStat];
	const StatsUpgradeData::StatData& upgradeData = myUpgradeData->myStatDatas[aStat];

	runtime.Upgrade(upgradeData.myIncreasePerUpgrade);
}

bool StatsComponent::CanUpgradeStat(StatType aStat) const
{
	const RuntimeStat& runtime = myRuntimeStats[aStat];
	const StatsUpgradeData::StatData& upgradeData = myUpgradeData->myStatDatas[aStat];

	return runtime.myNumberOfUpgrades < upgradeData.myMaxUpgrades;
}

float StatsComponent::GetStatValue(StatType aStat) const
{
	const RuntimeStat& runtime = myRuntimeStats[aStat];
	return runtime.myStatModifierValue;
}

int StatsComponent::GetStatRank(StatType aStat) const
{
	const RuntimeStat& runtime = myRuntimeStats[aStat];
	return runtime.myNumberOfUpgrades;
}
