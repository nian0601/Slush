#include "stdafx.h"

#include "LevelHud.h"

#include "Level/Level.h"
#include "TopDownGameGlobals.h"
#include "TowerBuildMenu.h"

LevelHud::LevelHud(Level& aLevel)
	: myLevel(aLevel)
{
}

void LevelHud::Update()
{
	myRenderCommands.RemoveAll();
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	uiBuilder.BeginPanel().SetAnchor(Slush::UIElementStyle::AnchorPoint::BOTTOM_LEFT, { 0, -TowerBuildMenu::ourBarHeight })
		.SetXSizing(Slush::UIElementStyle::FIXED, 240).SetYSizing(Slush::UIElementStyle::FIT)
		.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM).SetChildGap(4);

	FW_String lives = "Lives: ";
	lives += myLevel.GetLives();
	uiBuilder.Text(lives.GetBuffer());

	FW_String resources = "Resources: ";
	resources += myLevel.GetResources();
	uiBuilder.Text(resources.GetBuffer());

	const int totalWaves = myLevel.GetTotalWaveCount();
	int currentWave = myLevel.IsInGracePeriod() ? 0 : myLevel.GetCurrentWaveIndex() + 1;
	if (currentWave > totalWaves)
		currentWave = totalWaves;
	FW_String wave = "Wave ";
	wave += currentWave;
	wave += " / ";
	wave += totalWaves;
	uiBuilder.Text(wave.GetBuffer());

	FW_String countdown;
	if (myLevel.IsCountdownActive())
	{
		const float remainingSeconds = myLevel.GetSecondsUntilNextWave();
		int seconds = static_cast<int>(remainingSeconds);
		if (remainingSeconds > seconds)
			++seconds;
		countdown = "Next wave: ";
		countdown += seconds;
	}
	else
		countdown = myLevel.HasNextWave() ? "Next wave: --" : "Final wave";
	uiBuilder.Text(countdown.GetBuffer());

	uiBuilder.EndPanel();
	uiBuilder.Finish(myRenderCommands);
	myIsMouseOverUI = uiBuilder.IsMouseOverUI();
}
