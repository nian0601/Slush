#include "stdafx.h"

#include "EntitySystem/EntityManager.h"
#include "Level.h"
#include "ActionGameGlobals.h"

#include "Components/ExperienceComponent.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"

#include <Core\Input.h>
#include <UI\UIManager.h>
#include "Tilemap.h"
#include <Graphics\RectSprite.h>
#include <Graphics\Text.h>
#include "LevelData.h"

Level::Level()
	: myEntityManager(ActionGameGlobals::GetInstance().GetEntityManager())
	, myFont(ActionGameGlobals::GetInstance().GetFont())
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	myLevelData = assets.GetAsset<LevelData>("testLevel");
	myEnemyWaveData = &myLevelData->myEnemyWaves[0];
	myWaveTimer.Start(3.f);

	Entity* player = myEntityManager.CreateEntity(myLevelData->myPlayerStartPosition, myLevelData->myPlayerEntityPrefab);
	myPlayerHandle = player->myHandle;

	myTilemap = new Tilemap();

	myUISprite = new Slush::RectSprite();
	myText = new Slush::Text();
	myText->SetFont(myFont);

	myUIBackgroundStyle.SetPadding(16, 16);
	myUIBackgroundStyle.SetChildGap(16);
	myUIBackgroundStyle.SetColor(0xAA121212);
	myUIBackgroundStyle.SetXSizing(Slush::UIElementStyle::FIT);
	myUIBackgroundStyle.SetAlingment(Slush::UIElementStyle::CENTER);

	myUIButtonStyle.SetXSizing(Slush::UIElementStyle::FIXED, 250);
	myUIButtonStyle.SetYSizing(Slush::UIElementStyle::FIXED, 75);
	myUIButtonStyle.SetAlingment(Slush::UIElementStyle::CENTER);
	myUIButtonStyle.SetColor(0xFFAAFFAF);
	myUIButtonStyle.EnableButtonInteraction(0xFFDDDDDD);
}

Level::~Level()
{
	FW_SAFE_DELETE(myTilemap);
	myEntityManager.DeleteAllEntities();

	FW_SAFE_DELETE(myUISprite);
	FW_SAFE_DELETE(myText);
}

void Level::Update()
{
	Entity* player = myPlayerHandle.Get();
	if (!player)
		return;

	ExperienceComponent* expComp = player->GetComponent<ExperienceComponent>();

	Slush::Engine& engine = Slush::Engine::GetInstance();
	const Slush::Input& input = engine.GetInput();
	if (input.WasKeyReleased(Slush::Input::E))
	{
		expComp->AddExperience(1);
	}

	myIsLevelingUp = expComp->NeedsLevelUp();
	if (myIsLevelingUp)
	{
		HandleLevlingUp();
	}
	else
	{
		HandleEnemyWaves();
	}
}

void Level::RenderGame()
{
	myTilemap->Render();
}

void Level::RenderUI()
{
	for (Slush::DynamicUIBuilder::RenderCommand& command : myUIRenderCommands)
	{
		if (command.myText.Empty())
		{
			myUISprite->SetOrigin(Slush::RectSprite::Origin::TOP_LEFT);
			myUISprite->SetPosition(command.myPosition.x, command.myPosition.y);
			myUISprite->SetSize(command.mySize.x, command.mySize.y);
			myUISprite->SetFillColor(command.myColor);
			myUISprite->Render();
		}
		else
		{
			myText->SetText(command.myText);
			myText->SetCharacterSize(command.myTextSize);
			myText->SetColor(command.myColor);
			myText->SetPosition(command.myPosition.x, command.myPosition.y);
			myText->Render();
		}
	}
	myUIRenderCommands.RemoveAll();
}

bool Level::IsPlayerDead() const
{
	Entity* player = myPlayerHandle.Get();
	if (!player)
		return true;

	if (HealthComponent* health = player->GetComponent<HealthComponent>())
		return health->IsDead();

	return true;
}

void Level::HandleLevlingUp()
{
	Entity* player = myPlayerHandle.Get();
	ExperienceComponent* expComp = player->GetComponent<ExperienceComponent>();

	if (StatsComponent* stats = player->GetComponent<StatsComponent>())
	{
		if (!stats->CanUpgradeCooldownReduction() && !stats->CanUpgradeDamage() && !stats->CanUpgradeAdditionalProjectiles())
		{
			expComp->LevelUp();
			myIsLevelingUp = false;
			SLUSH_WARNING("No more available upgrades, auto-leveling");
		}
		else
		{
			Slush::DynamicUIBuilder uiBuilder;
			uiBuilder.Start();

			{
				uiBuilder.OpenElement("Title", myUIBackgroundStyle);
				uiBuilder.GetStyle().SetXSizing(Slush::UIElementStyle::GROW);

				uiBuilder.OpenElement("title");
				uiBuilder.SetText("Leveled Up!", myFont, 32);
				uiBuilder.CloseElement();

				uiBuilder.CloseElement(); // Title
			}

			{
				uiBuilder.OpenElement("ButtonBackground", myUIBackgroundStyle);

				if (stats->CanUpgradeCooldownReduction())
				{
					uiBuilder.OpenElement("Cooldown", myUIButtonStyle);
					uiBuilder.GetStyle().SetColor(0xFFFF3333);

					uiBuilder.OpenElement();
					uiBuilder.SetText("Cooldown", myFont, 25);
					uiBuilder.GetStyle().SetColor(0xFF000000);
					uiBuilder.CloseElement();

					uiBuilder.CloseElement();
				}

				if (stats->CanUpgradeDamage())
				{
					uiBuilder.OpenElement("Damage", myUIButtonStyle);
					uiBuilder.GetStyle().SetColor(0xFFFFFF33);

					uiBuilder.OpenElement();
					uiBuilder.SetText("Damage", myFont, 25);
					uiBuilder.GetStyle().SetColor(0xFF000000);
					uiBuilder.CloseElement();

					uiBuilder.CloseElement();
				}

				if (stats->CanUpgradeAdditionalProjectiles())
				{
					uiBuilder.OpenElement("Projectile", myUIButtonStyle);
					uiBuilder.GetStyle().SetColor(0xFF33FF33);

					uiBuilder.OpenElement();
					uiBuilder.SetText("Projectile", myFont, 25);
					uiBuilder.GetStyle().SetColor(0xFF000000);
					uiBuilder.CloseElement();

					uiBuilder.CloseElement();
				}
				uiBuilder.CloseElement(); // ButtonBackground
			}

			uiBuilder.Finish(myUIRenderCommands);


			if (stats->CanUpgradeCooldownReduction() && uiBuilder.WasClicked("Cooldown"))
			{
				stats->AddCooldownReductionUpgrade();
				expComp->LevelUp();
				myIsLevelingUp = false;
			}
			if (stats->CanUpgradeDamage() && uiBuilder.WasClicked("Damage"))
			{
				stats->AddDamageUpgrade();
				expComp->LevelUp();
				myIsLevelingUp = false;
			}
			if (stats->CanUpgradeAdditionalProjectiles() && uiBuilder.WasClicked("Projectile"))
			{
				stats->AddAdditionalProjectilesUpgrade();
				expComp->LevelUp();
				myIsLevelingUp = false;
			}
		}
	}
	else
	{
		SLUSH_ERROR("Player doesnt have a StatsComponent, not able to pick upgrade when Leveling");
		expComp->LevelUp();
		myIsLevelingUp = false;
	}
}

void Level::HandleEnemyWaves()
{
	if (ActionGameGlobals::GetInstance().myDebugSettings.myPauseEnemySpawning)
		return;

	if (myWaveTimer.IsStarted() && !myWaveTimer.HasExpired())
		return;

	myWaveTimer.Start(myEnemyWaveData->myDuration);

	Entity* player = myPlayerHandle.Get();

	const int iterationLimit = 100;
	int iterations = 0;
	Vector2f spawnArea = { 1920.f, 1080.f };
	Vector2f spawnAreaMargin = { 75.f, 75.f };
	const int numEnemies = FW_RandInt(myEnemyWaveData->myMinEnemyCount, myEnemyWaveData->myMaxEnemyCount);
	FW_GrowingArray<Entity*> spawnedEnemies;
	while (spawnedEnemies.Count() < numEnemies && iterations <= iterationLimit)
	{
		++iterations;

		float randX = FW_RandFloat(spawnAreaMargin.x, spawnArea.x - spawnAreaMargin.x);
		float randY = FW_RandFloat(spawnAreaMargin.y, spawnArea.y - spawnAreaMargin.y);
		Vector2f position;
		position.x += randX;
		position.y += randY;

		if (IsTooClose(player->myPosition, position, myPlayerClearanceRadius))
			continue;

		for (Entity* enemy : spawnedEnemies)
		{
			if (IsTooClose(enemy->myPosition, position, myEnemyClearanceRadius))
				continue;
		}

		int enemyPick = FW_RandInt(0, myEnemyWaveData->myEnemyPrefabs.Count() -1);

		Entity* enemy = myEntityManager.CreateEntity(position, myEnemyWaveData->myEnemyPrefabs[enemyPick]);
		if (ProjectileShootingComponent* projShoot = enemy->GetComponent<ProjectileShootingComponent>())
			projShoot->TriggerCooldown();

		spawnedEnemies.Add(enemy);
	}

	++myWaveCounter;
	if (myWaveCounter >= myLevelData->myEnemyWaves.Count())
		myWaveCounter = 0;

	myEnemyWaveData = &myLevelData->myEnemyWaves[myWaveCounter];
}

bool Level::IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance)
{
	float distance2 = Length2(aPosition - aTestPosition);
	return distance2 <= FW_Square(aTestClearance);
}
