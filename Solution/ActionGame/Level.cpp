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
#include <Graphics\Text.h>

Level::Level(Slush::Font& aFont, Slush::AssetStorage<Slush::UILayout>& someUILayouts)
	: myEntityManager(ActionGameGlobals::GetInstance().GetEntityManager())
	, myPhysicsWorld(ActionGameGlobals::GetInstance().GetPhysicsWorld())
	, myFont(aFont)
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
	myText = new Slush::Text();
	myText->SetFont(aFont);

	
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
	FW_SAFE_DELETE(myUIManager);
	FW_SAFE_DELETE(myNPCWave);
	myEntityManager.DeleteAllEntities();

	FW_SAFE_DELETE(myUISprite);
	FW_SAFE_DELETE(myText);
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

					{
						uiBuilder.OpenElement("Cooldown", myUIButtonStyle);
						uiBuilder.GetStyle().SetColor(0xFFFF3333);

						uiBuilder.OpenElement();
						uiBuilder.SetText("Cooldown", myFont, 25);
						uiBuilder.GetStyle().SetColor(0xFF000000);
						uiBuilder.CloseElement();

						uiBuilder.CloseElement();
					}

					{
						uiBuilder.OpenElement("Damage", myUIButtonStyle);
						uiBuilder.GetStyle().SetColor(0xFFFFFF33);

						uiBuilder.OpenElement();
						uiBuilder.SetText("Damage", myFont, 25);
						uiBuilder.GetStyle().SetColor(0xFF000000);
						uiBuilder.CloseElement();

						uiBuilder.CloseElement();
					}

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