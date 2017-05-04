#pragma once
#include "Game/Score/Pickup/BasePickup.h"

class HealthPickup : public BasePickup
{
public:
	HealthPickup();
	~HealthPickup();

	virtual void OnPickup(SB::GameObject & aPlayerObject) override;

};
