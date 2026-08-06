#include "stdafx.h"

#include "TimedRemovalComponent.h"
#include <imgui\ImGuiWidgets.h>

void TimedRemovalComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseFloatField("duration", myDuration);
}

void TimedRemovalComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputFloat("Duration", &myDuration);
}

//////////////////////////////////////////////////////////////////////////

TimedRemovalComponent::TimedRemovalComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	const TimedRemovalComponent::Data* data = static_cast<const TimedRemovalComponent::Data*>(anEntityPrefab.GetComponentBaseData<TimedRemovalComponent>());
	myTimer.Start(data->myDuration);
}

void TimedRemovalComponent::Update()
{
	if (myTimer.HasExpired())
		myEntity.myIsMarkedForRemoval = true;
}