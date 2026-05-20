#include "stdafx.h"


#include "ActionGameGlobals.h"
#include "AnimationComponent.h"
#include "DamageDealerComponent.h"
#include "PhysicsComponent.h"
#include "SpriteComponent.h"
#include "StatsComponent.h"
#include "TargetingComponent.h"
#include "WeaponComponent.h"

#include "EntitySystem/EntityManager.h"

#include <Graphics\BaseSprite.h>
#include <Graphics\Texture.h>

#include <Physics\PhysicsWorld.h>
#include "UI/UIManager.h"

void WeaponData::RankData::OnParse(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseFloatField("basecooldown", myBaseCooldown);
	aRootHandle.ParseIntField("basedamage", myBaseDamage);

	Slush::AssetParser::Handle projectileHandle = aRootHandle.ParseChildElement("projectileData");
	projectileHandle.ParseOptionalBoolField("enable", myProjectileData.myEnable, true);
	projectileHandle.ParseOptionalFloatField("baseprojectilespeed", myProjectileData.myBaseProjectileSpeed, myProjectileData.myEnable);
	projectileHandle.ParseOptionalIntField("baseprojectilcount", myProjectileData.myBaseProjectileCount, myProjectileData.myEnable);
	projectileHandle.ParseOptionalStringField("projectilePrefab", myProjectileData.myProjectilePrefab, myProjectileData.myEnable);
}

WeaponData::WeaponData(const char* aName, unsigned int aAssetID)
	: Slush::DataAsset(aName, aAssetID)
{
	myRanks.Add();
}

void WeaponData::OnParse(Slush::AssetParser::Handle aRootHandle)
{
	int numRanks = myRanks.Count();
	if (aRootHandle.IsReading())
	{
		numRanks = aRootHandle.GetNumChildElements();

		Slush::AssetParser::Handle firstChild = aRootHandle.GetChildElementAtIndex(0);
		if (firstChild.IsValid() && firstChild.GetName() != "rankdata")
		{
			myRanks[0].OnParse(aRootHandle);
			return;
		}
	}

	myRanks.Reserve(numRanks);
	for (int i = 0; i < numRanks; ++i)
	{
		Slush::AssetParser::Handle rankHandle;
		if (aRootHandle.IsReading())
			rankHandle = aRootHandle.GetChildElementAtIndex(i);
		else
			rankHandle = aRootHandle.ParseChildElement("rankdata");

		myRanks[i].OnParse(rankHandle);
	}
}

void WeaponData::BuildUI()
{
	for (int i = 0; i < myRanks.Count(); ++i)
	{
		FW_String rankHeader = "Rank ";
		rankHeader += i;

		ImGui::PushID(i);

		bool keepRank = true;
		if (ImGui::CollapsingHeader(rankHeader.GetBuffer(), i == 0 ? nullptr : &keepRank))
		{
			ImGui::Indent();
			RankData& rankData = myRanks[i];
			if (ImGui::CollapsingHeader("Base Data", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent();
				ImGui::SetNextItemWidth(150.f);
				ImGui::InputFloat("Base Cooldown", &rankData.myBaseCooldown, 0.05f, 0.1f, "%.2f");

				ImGui::SetNextItemWidth(150.f);
				ImGui::InputInt("Base Damage", &rankData.myBaseDamage);
				ImGui::Unindent();
			}

			if (ImGui::CollapsingHeader("Projectile Data"))
			{
				ImGui::Indent();
				ImGui::SetNextItemWidth(150.f);
				ImGui::Checkbox("Enable", &rankData.myProjectileData.myEnable);

				if (rankData.myProjectileData.myEnable)
				{
					ImGui::SetNextItemWidth(150.f);
					ImGui::InputFloat("Base Projectile Speed", &rankData.myProjectileData.myBaseProjectileSpeed, 1.f, 100.f, "%.2f");

					ImGui::SetNextItemWidth(150.f);
					ImGui::InputInt("Base Projectile Count", &rankData.myProjectileData.myBaseProjectileCount);
					rankData.myProjectileData.myBaseProjectileCount = FW_Clamp(rankData.myProjectileData.myBaseProjectileCount, 1, 20);

					ImGui::SetNextItemWidth(150.f);
					ImGui::InputText("ProjectilePrefab", &rankData.myProjectileData.myProjectilePrefab);
					if (ImGui::BeginDragDropTarget())
					{
						if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<EntityPrefab>()))
							rankData.myProjectileData.myProjectilePrefab = asset->GetAssetName();

						ImGui::EndDragDropTarget();
					}
				}
				ImGui::Unindent();
			}

			ImGui::Unindent();
		}

		if (!keepRank)
		{
			myRanks.RemoveNonCyclicAtIndex(i);
			--i;
		}

		ImGui::PopID();
	}

	if (ImGui::Button("Add Rank"))
		myRanks.Add(myRanks.GetLast());
}

//////////////////////////////////////////////////////////////////////////

Weapon::Weapon(Entity& anEntity, const WeaponData& aWeaponData)
	: myEntity(anEntity)
	, myWeaponData(aWeaponData)
{
	myRankData = &myWeaponData.myRanks[0];
}

void Weapon::Update()
{
	if (!myActivationCooldown.IsStarted() || myActivationCooldown.HasExpired())
	{
		float cooldown = myRankData->myBaseCooldown;
		if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
		{
			cooldown /= stats->GetCooldownReduction();
		}

		myActivationCooldown.Start(cooldown);

		if (myRankData->myProjectileData.myEnable)
			RunProjectileLogic();
	}
}

void Weapon::Upgrade()
{
	++myRank;
	myRank = FW_Clamp(myRank, 0, myWeaponData.myRanks.Count() - 1);

	myRankData = &myWeaponData.myRanks[myRank];
}

bool Weapon::CanBeUpgraded() const
{
	return myRank + 1 < myWeaponData.myRanks.Count();
}

void Weapon::RunProjectileLogic()
{
	TargetingComponent* targeting = myEntity.GetComponent<TargetingComponent>();
	if (!targeting)
	{
		SLUSH_ERROR("Entity with 'WeaponComponent' is missing a 'TargetingComponent'");
		return;
	}

	EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	const WeaponData::RankData::ProjectileData& projectileData = myRankData->myProjectileData;

	const bool oddNumProjectiles = (projectileData.myBaseProjectileCount % 2) == 1;
	int projectilesToSpawn = projectileData.myBaseProjectileCount;

	Vector2f direction = GetNormalized(target.Get()->myPosition - myEntity.myPosition);

	if (oddNumProjectiles)
	{
		ShootProjectile(direction);
		--projectilesToSpawn;
	}

	for (int i = 0; i < projectilesToSpawn; i += 2)
	{
		float spread = 0.1f * i + 0.1f;
		ShootProjectile(Rotate(direction, FW_PI * spread));
		ShootProjectile(Rotate(direction, -FW_PI * spread));
	}
}

void Weapon::ShootProjectile(const Vector2f& aDirection)
{
	Entity* projectile = myEntity.myEntityManager.CreateEntity(myEntity.myPosition + aDirection * 35.f, myRankData->myProjectileData.myProjectilePrefab.GetBuffer());
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * myRankData->myProjectileData.myBaseProjectileSpeed;
	projectile->GetComponent<SpriteComponent>()->GetSprite().SetRotation(FW_SignedAngle(aDirection));
	if (DamageDealerComponent* projDamage = projectile->GetComponent<DamageDealerComponent>())
	{
		int damage = myRankData->myBaseDamage;
		if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
			damage = static_cast<int>(damage * stats->GetDamageModifier());

		projDamage->SetDamage(damage);
	}

	if (AnimationComponent* anim = myEntity.GetComponent<AnimationComponent>())
		anim->PlaySpritesheetAnimation();
}

//////////////////////////////////////////////////////////////////////////

void WeaponComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseStringField("weapondata", myWeaponDataAsset);
}

void WeaponComponent::Data::OnBuildUI()
{
	ImGui::InputText("WeaponData", &myWeaponDataAsset);
	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<WeaponData>()))
			myWeaponDataAsset = asset->GetAssetName();

		ImGui::EndDragDropTarget();
	}
}


WeaponComponent::WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	WeaponData* startingWeapon = assets.GetAsset<WeaponData>(anEntityPrefab.GetWeaponData().myWeaponDataAsset.GetBuffer());

	myWeapons.Add(new Weapon(myEntity, *startingWeapon));

	const FW_GrowingArray<Slush::Asset*>& allWeapons = assets.GetAllAssets<WeaponData>();
	for (const Slush::Asset* asset : allWeapons)
	{
		const WeaponData* weapon = static_cast<const WeaponData*>(asset);
		if (weapon == startingWeapon)
			continue;

		myAvailableNewWeapons.Add(weapon);
	}
}

WeaponComponent::~WeaponComponent()
{
	myWeapons.DeleteAll();
}

void WeaponComponent::Update()
{
	for (Weapon* weapon : myWeapons)
		weapon->Update();
}

void WeaponComponent::AddPendingUpgrade()
{
	myHasPendingUpgrade = true;

	myUpgradeSelection.RemoveAll();

	const FW_GrowingArray<Weapon*>& activeWeapons = GetWeapons();
	for (int i = 0; i < activeWeapons.Count(); ++i)
	{
		if (!activeWeapons[i]->CanBeUpgraded())
			continue;

		float chance = FW_RandFloat();
		if (chance > 0.5f)
			myUpgradeSelection.Add(&activeWeapons[i]->GetWeaponData());
	}

	for (const Slush::Asset* asset : myAvailableNewWeapons)
	{
		float chance = FW_RandFloat();
		if (chance > 0.5f)
			myUpgradeSelection.Add(static_cast<const WeaponData*>(asset));
	}
}

void WeaponComponent::UpgradeWeapon(const WeaponData& someData)
{
	if (Weapon* weapon = GetWeapon(&someData))
		weapon->Upgrade();
	else
	{
		myWeapons.Add(new Weapon(myEntity, someData));
		myAvailableNewWeapons.RemoveCyclic(&someData);
	}
}

void WeaponComponent::HandleUpgrading(Slush::DynamicUIBuilder& aUIBuilder)
{
	if (myUpgradeSelection.IsEmpty())
	{
		FinishUpgrade();
		return;
	}

	aUIBuilder.Start();

	aUIBuilder.TextHeader(
		"Weapon Upgrade!", 
		ActionGameGlobals::GetInstance().GetFont(), 
		32, 
		0xAA121212, 
		0xFFFFFFFF);

	aUIBuilder.VerticalSpacing(20);

	aUIBuilder.OpenElement()
		.SetPadding(16, 16)
		.SetChildGap(16)
		.SetColor(0xAA121212)
		.SetAlingment(Slush::UIElementStyle::CENTER);

	for (const WeaponData* potentialUpgrade : myUpgradeSelection)
	{
		Weapon* weapon = GetWeapon(potentialUpgrade);

		Slush::UIElementStyle& style = aUIBuilder.OpenElement(potentialUpgrade->GetAssetName().GetBuffer());
		style.SetXSizing(Slush::UIElementStyle::FIXED, 250);
		style.SetYSizing(Slush::UIElementStyle::FIXED, 75);
		style.SetAlingment(Slush::UIElementStyle::CENTER);
		style.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		style.SetChildGap(8);

		if (weapon)
			style.SetColor(0xFF33FF33);
		else
			style.SetColor(0xFFFF3333);

		style.SetOutlineColor(0xFF000000);
		style.SetOutlineThickness(-1.f);
		style.EnableButtonInteraction(0xFFDDDDDD);

		aUIBuilder.Text(potentialUpgrade->GetAssetName().GetBuffer(), ActionGameGlobals::GetInstance().GetFont(), 25, 0xFF000000);

		if (Weapon* weapon = GetWeapon(potentialUpgrade))
			aUIBuilder.Text("Upgrade", ActionGameGlobals::GetInstance().GetFont(), 20, 0xFF000000);
		else
			aUIBuilder.Text("NEW", ActionGameGlobals::GetInstance().GetFont(), 20, 0xFF000000);

		aUIBuilder.CloseElement();
	}

	aUIBuilder.CloseElement(); 
	aUIBuilder.Finish();


	for (const WeaponData* potentialWeapon : myUpgradeSelection)
	{
		if (aUIBuilder.WasClicked(potentialWeapon->GetAssetName().GetBuffer()))
		{
			UpgradeWeapon(*potentialWeapon);
			FinishUpgrade();
		}
	}
}

Weapon* WeaponComponent::GetWeapon(const WeaponData* aWeaponData)
{
	for (Weapon* weapon : myWeapons)
	{
		if (&weapon->GetWeaponData() == aWeaponData)
			return weapon;
	}

	return nullptr;
}
