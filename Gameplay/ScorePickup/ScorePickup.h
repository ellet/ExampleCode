#pragma once
#include "BasePickup.h"


class ScorePickup : public BasePickup
{
public:
	ScorePickup();
	~ScorePickup();

	virtual void OnPickup(SB::GameObject & aPlayerObject) override;

};

