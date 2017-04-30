#pragma once
#include "Game\Health\HealthMessages.h"
#include "Engine\Events\EventRegistration.h"
#include "WeaponComponent.h"

//class WeaponComponent;

enum class WeaponTypes
{
	eFlak,
	eAssault,
	enumLength
};

enum class AttackTypes;

struct AmmoData
{
	WeaponTypes WeaponType;
	AttackTypes FireModeType;
	uint16_t AmmoAmount;
};

namespace SB
{
	class Text;
}

class WeaponManagerComponent : public SB::BaseComponent, public SB::EventSubscriber<ObjectOutOfHealthMessage>, public SB::EventSubscriber<ObjectRespawnedMessage>
{
public:
	WeaponManagerComponent();
	~WeaponManagerComponent();

	virtual void PreInitialize() override;
	virtual void Initialize() override;


	virtual void Update(const SB::Time & aDeltaTime) override;

	virtual void RenderGUI(const SB::GuiRenderTarget & aRenderTarget) const override;

	bool AddAmmo(const AmmoData & aAmmoData);

	virtual void RecieveEvent(const ObjectOutOfHealthMessage & aEvent) override;
	virtual void RecieveEvent(const ObjectRespawnedMessage & aEvent) override;

	void SetUseInstinctWeapon(const bool aValue);
	void SetPlayer(SB::GameObject & aPlayer);
	

	void UnlockWeapon(const WeaponTypes aWeaponTypeToUnlock, const bool aShouldSwithTo = true);

	virtual void EndRender(const SB::Camera & aCamera) const override;
	bool GetIsAssaultRifle()
	{
		if (myActiveWeapon == myAssaultRifle)
		{
			return true;
		}
		return false;
	}
	bool GetIsInstinctWeapon()
	{
		if (myActiveWeapon == myInstinctWeapon)
		{
			return true;
		}
		return false;
	}
	WeaponComponent* GetActiveWeapon()
	{
		return myActiveWeapon;
	}

private:
	void SwitchWeapon(WeaponComponent & aWeaponToSwitchTo);
	void SwitchWeaponCallback(const SB::ComponentPtr<WeaponComponent> & aWeaponToSwitchTo, const SB::ObjectPtr & aObject);

	void LoadWeaponData(WeaponComponent & aWeaponToLoad, const std::string & aFilePath);

	void RecieveUnlockedWeapon(const uint8_t aWeapon, const bool aShouldSwitchToo);

	WeaponComponent & GetWeapon(const WeaponTypes aWeaponType);
	void UpdateGUIIcons(const bool aShouldOnlyUpdateBigIcons = false);

	SB::ObjectPtr myWeaponObject;
	std::shared_ptr<SB::Effect> myWriteTypeValueEffect;
	std::shared_ptr<SB::ConstantBuffer<SB::ConstantBuffers::WriteRefValueData>> myWriteRefValueBufferData;
	
	WeaponComponent * myActiveWeapon;
	WeaponComponent * myFlakCannon;
	WeaponComponent * myInstinctWeapon;
	WeaponComponent * myAssaultRifle;
	WeaponComponent * myLastWeapon;

	SB::EventRegistration<const SB::ComponentPtr<WeaponComponent> &, const SB::ObjectPtr &> Event_SwitchWeapon;
	SB::EventRegistration<const uint8_t, const bool> Event_UnlockedWeapon;

	SB::GameObject * myPlayer;

	//std::unique_ptr<SB::Text> myAmmoText;
	float myOutlineRefValue;
	bool myLockAllActions;
	bool myInstinctActivated;
};

