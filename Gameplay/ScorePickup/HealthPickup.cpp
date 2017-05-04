#include "stdafx.h"
#include "HealthPickup.h"
#include "Game\Health\PlayerHealthComponent.h"


HealthPickup::HealthPickup() : BasePickup("Assets/Models/Objects/Pickups/health.fbx", "HealthPickup")
{
}


HealthPickup::~HealthPickup()
{
}

void HealthPickup::OnPickup(SB::GameObject & aPlayerObject)
{
	if (aPlayerObject.GetComponentCount<PlayerHealthComponent>() > 0)
	{
		aPlayerObject.GetComponent<PlayerHealthComponent>()->RecoverHealth(5.f, false);
	}
	else
	{
		Warning("Health pickup did not find playerhealthcomponent");
	}
}
