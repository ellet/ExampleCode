#pragma once
#include "WeaponComponent.h"

namespace SB
{
	class PointLightComponent;
}
class InstinctWeaponComponent : public SB::ComponentInheritance<WeaponComponent>
{
public:
	InstinctWeaponComponent();
	~InstinctWeaponComponent();

	virtual void PreInitialize() override;


	virtual void Update(const SB::Time & aDeltaTime) override;


	virtual void DebugRender(const SB::Camera & aCamera, const uint32_t aDebugRenderMode) const override;


	virtual void SetAsActiveWeapon(SB::ObjectPtr & aWeaponObject) override;

protected:
	virtual void FirePrimary() override;
	virtual void FireSecondary() override;

	virtual void SetupWeaponData() override;

	virtual void SpecificActivation(SB::ObjectPtr & aWeaponObject) override;

private:
	void DealDamage(SB::GameObject & aHitObject);
	void CreateAmmoShell(const SB::Vector3f & aAmmoScale, const SB::Color & aAmmoColor);
	SB::Vector3f myWeaponPos;
	SB::Vector3f myFirePos;
	SB::ObjectPtr myWeaponObject;

	//SB::ObjectPtr myPositionHitObject;

	SB::Vector3f linestartpos;
	SB::Vector3f lineendpos;

	SB::ObjectPtr myMuzzleFlash;
	SB::Stopwatch myMuzzleTimer;
	SB::Time myMuzzleStayTime;
	SB::PointLightComponent * myMuzzleFlashLight;

	SB::Randomizer myRandomizer;
};

