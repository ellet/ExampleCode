#pragma once
#include "Game/Player/Weapons/WeaponComponent.h"


namespace SB
{
	class RandomDistributionSphere;
}

class FlakCannonComponent : public SB::ComponentInheritance<WeaponComponent>
{
public:
	FlakCannonComponent();
	~FlakCannonComponent();


	virtual void LoadData(SB::DataNode aProperties) override;


	virtual void SetAsActiveWeapon(SB::ObjectPtr & aWeaponObject) override;
	virtual void Update(const SB::Time & aDeltaTime) override;

protected:
	virtual void SetupWeaponData() override;

	virtual void FirePrimary() override;
	virtual void FireSecondary() override;

	virtual void SpecificActivation(SB::ObjectPtr & aWeaponObject) override;
	

private:
	float myStartExplosionSize;
	uint8_t myStartBouncesAmount;
	uint16_t myShrapnelAmount;

	SB::Vector3f myWeaponPos;
	SB::Vector3f myFirePos;
	SB::ObjectPtr myWeaponObject;
	std::unique_ptr< SB::RandomDistributionSphere> myDistribution;
	unsigned int myTimesFired;
	bool myDisplayTip;
};

