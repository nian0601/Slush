#include "stdafx.h"

#include "EntityPrefab.h"
#include "TimedRemovalComponent.h"

void TimedRemovalComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("duration", myDuration);
}

void TimedRemovalComponent::Data::OnBuildUI()
{
	ImGui::InputFloat("Duration", &myDuration);
}

//////////////////////////////////////////////////////////////////////////

TimedRemovalComponent::TimedRemovalComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	const TimedRemovalComponent::Data* data = static_cast<const TimedRemovalComponent::Data*>(anEntityPrefab.GetComponentBaseData<TimedRemovalComponent>());
	myTimer.Start(data->myDuration);
}

void TimedRemovalComponent::Update()
{
	if (myTimer.HasExpired())
		myEntity.myIsMarkedForRemoval = true;
}