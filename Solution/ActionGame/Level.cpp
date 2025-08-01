#include "stdafx.h"

#include "EntityManager.h"
#include "ExperienceComponent.h"
#include "HealthComponent.h"
#include "Level.h"
#include "NPCWave.h"
#include "StatsComponent.h"
#include "ActionGameGlobals.h"

#include <Core\Input.h>
#include <UI\UIManager.h>
#include <UI\UIButton.h>
#include "Tilemap.h"
#include <Graphics\RectSprite.h>

Level::Level(Slush::Font& aFont, Slush::AssetStorage<Slush::UILayout>& someUILayouts)
	: myEntityManager(ActionGameGlobals::GetInstance().GetEntityManager())
	, myPhysicsWorld(ActionGameGlobals::GetInstance().GetPhysicsWorld())
{
	Entity* player = myEntityManager.CreateEntity({ 400.f, 400.f }, "Player");
	myPlayerHandle = player->myHandle;

	myNPCWave = new NPCWave();

	myUIManager = new Slush::UIManager(aFont);
	myUIManager->SetLayout(someUILayouts.GetAsset("LevelUp"));

	if (Slush::UIWidget* button = myUIManager->FindWidget("DamageUpgrade"))
		myDamageUpgradeButton = static_cast<Slush::UIButton*>(button);

	if (Slush::UIWidget* button = myUIManager->FindWidget("CooldownUpgrade"))
		myCooldownUpgradeButton = static_cast<Slush::UIButton*>(button);

	if (Slush::UIWidget* button = myUIManager->FindWidget("ProjectileUpgrade"))
		myProjectileUpgradeButton = static_cast<Slush::UIButton*>(button);

	myTilemap = new Tilemap();

	myUISprite = new Slush::RectSprite();
}

Level::~Level()
{
	FW_SAFE_DELETE(myTilemap);
	FW_SAFE_DELETE(myUIManager);
	FW_SAFE_DELETE(myNPCWave);
	myEntityManager.DeleteAllEntities();

	FW_SAFE_DELETE(myUISprite);
}

void Level::Update()
{
	myNPCWave->Update();

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

	if (StatsComponent* stats = player->GetComponent<StatsComponent>())
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		if (engine.GetInput().WasKeyPressed(Slush::Input::Q))
			stats->AddAdditionalProjectilesUpgrade();
	}

	myIsLevelingUp = expComp->NeedsLevelUp();
	if (myIsLevelingUp)
	{
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
				Slush::Engine& engine = Slush::Engine::GetInstance();
				myUIManager->Update(engine.GetInput());

				if (stats->CanUpgradeCooldownReduction() && myCooldownUpgradeButton->WasPressed())
				{
					stats->AddCooldownReductionUpgrade();
					expComp->LevelUp();
					myIsLevelingUp = false;
				}
				else if (stats->CanUpgradeDamage() && myDamageUpgradeButton->WasPressed())
				{
					stats->AddDamageUpgrade();
					expComp->LevelUp();
					myIsLevelingUp = false;
				}
				else if (stats->CanUpgradeAdditionalProjectiles() && myProjectileUpgradeButton->WasPressed())
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

	myUIRenderCommands.RemoveAll();

	Slush::DynamicUIBuilder uiBuilder;
	uiBuilder.Start();

	uiBuilder.OpenElement("Middle background");
	uiBuilder.SetPadding(16, 16);
	uiBuilder.SetChildGap(16);
	uiBuilder.SetColor(0xFF3333FF);

	for (int i = 0; i < 3; ++i)
	{
		uiBuilder.OpenElement("Middle button");
		uiBuilder.SetXSizing(Slush::DynamicUIBuilder::FIXED, 250);
		uiBuilder.SetYSizing(Slush::DynamicUIBuilder::FIXED, 75);
		uiBuilder.SetColor(0xFFAAFFAA);
		uiBuilder.CloseElement();
	}

	uiBuilder.CloseElement();

	uiBuilder.Finish(myUIRenderCommands);
}

void Level::RenderGame()
{
	myTilemap->Render();

	for (Slush::DynamicUIBuilder::RenderCommand& command : myUIRenderCommands)
	{
		myUISprite->SetOrigin(Slush::RectSprite::Origin::TOP_LEFT);
		myUISprite->SetPosition(command.myPosition.x, command.myPosition.y);
		myUISprite->SetSize(command.mySize.x, command.mySize.y);
		myUISprite->SetFillColor(command.myColor);
		myUISprite->Render();
	}
}

void Level::RenderUI()
{
	if (myIsLevelingUp)
		myUIManager->Render();
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