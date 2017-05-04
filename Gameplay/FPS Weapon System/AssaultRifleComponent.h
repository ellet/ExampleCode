#pragma once
#include "WeaponComponent.h"
#include "../../../../Client/Client/CommonIncludes.h"

namespace SB
{
	class PointLightComponent;
}

class AssaultRifleComponent : public SB::ComponentInheritance<WeaponComponent>
{
public:
	AssaultRifleComponent();
	~AssaultRifleComponent();

	virtual void PreInitialize() override;


	virtual void Update(const SB::Time & aDeltaTime) override;


	virtual void DebugRender(const SB::Camera & aCamera, const uint32_t aDebugRenderMode) const override;


	virtual void SetAsActiveWeapon(SB::ObjectPtr & aWeaponObject) override;

	void SetMuzzleFlashOffset(SB::ObjectPtr aWeaponObject, const SB::Vector3f & aOffset, const float aRotationAmount);

	void SpecificActivationPub();
protected:
	virtual void FirePrimary() override;


	virtual void FireSecondary() override;

	virtual void SetupWeaponData() override;

	virtual void SpecificActivation(SB::ObjectPtr & aWeaponObject) override;

private:
	void CreateHitParticles(const SB::Vector3f & aHitPosition, const SB::Vector3f & aHitNormal);
	void CreateAmmoShell(const SB::Vector3f & aAmmoScale, const SB::Color & aAmmoColor);
	void DealDamage(SB::GameObject & aHitObject);

	SB::Stopwatch myParticleCooldown;
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
	unsigned int myTimesFired;
	bool myDisplayTip;
};

