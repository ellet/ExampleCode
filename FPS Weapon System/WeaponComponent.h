#pragma once

enum class AttackTypes
{
	ePrimary,
	eSeconday,
	enumLength
};

enum class IconTypes
{
	eBig,
	eBigGrey,
	eSmall,
	eSmallGrey
};

namespace SB
{
	template <typename T>
	class ConstantBuffer;

	class Effect;

	namespace ConstantBuffers
	{
		struct WriteRefValueData;
	}
}
class WeaponComponent : public SB::BaseComponent
{
public:
	

	struct AttackData
	{
		SB::Vector3f FirePosition;
		SB::Time FireRate;
		float WeaponDamage;
		std::string WeaponFireSound = MISSING_SOUND;
		uint16_t CurrentAmmoCount;
		uint16_t maxAmmoCount = 100;
		SB::Time LastFiredTimer;
		SB::Time LastTryTimer;
		uint16_t AmmoCost = 0;
		float Recoil = 0.f;
		float effectForce = 10.f;

		void LoadAttackData(SB::DataNode aProperties);
	};

	struct WeaponData
	{
		std::shared_ptr<SB::AssimpModel> WeaponModel;
		SB::Vector3f WeaponPosition;
		SB::Color WeaponColor;
		
		AttackData PrimaryAttack;
		AttackData SecondaryAttack;
	};

	virtual void PreInitialize() override;

	SB::Vector3f GetShotDirection();
	void FireWeapon(const AttackTypes aAttackTypeToFire);
	void Unlock();
	bool GetUnlockState() const;

	virtual void SetAsActiveWeapon(SB::ObjectPtr & aWeaponObject) = 0;

	bool AddAmmo(const AttackTypes aFiringType, const uint16_t aAmmoAmountToAdd);

	uint16_t GetPrimaryAmmoCount() const { return myWeaponData.PrimaryAttack.CurrentAmmoCount; }
	uint16_t GetSecondaryAmmoCount() const { return myWeaponData.SecondaryAttack.CurrentAmmoCount; }

	const WeaponData & GetWeaponData() const
	{
		return myWeaponData;
	}


	virtual const SB::ComponentSerializer & GetSerializer() const override;

	void RecoilGuiElements(const std::string &aBaseGUIName)const;
	virtual void LoadData(SB::DataNode aProperties) override;
	void SetupAmmoIcons(const unsigned short aAmmoAmount);
	void UpdateAmmoIcons();

	virtual void Update(const SB::Time & aDeltaTime) override;

protected:
	WeaponComponent();
	~WeaponComponent();
	SB::Randomizer myRandomizer;
	static const float ourMaxRecoil;

	SB::Vector3f GetFireDirection() const;
	SB::Quaternion GetPlayerOrieantion() const;

	virtual void SpecificActivation(SB::ObjectPtr & aWeaponObject) = 0;

	virtual void SetupWeaponData() = 0;

	virtual void FirePrimary() = 0;
	virtual void FireSecondary() = 0;
	void CreateAmmoIcons(const std::string & aSprite, const std::string & aParentName, const std::string & weaponName, const unsigned short aPositionIndex, const float aScale, IconTypes aIconType);
	AttackData & GetAttackData(const AttackTypes aAttackType);

	WeaponData myWeaponData;

	SB::GUIElement * myAmmoContainer;
	std::unordered_map<IconTypes, SB::GrowingArray<SB::SpriteGUIElement*>> myIcons;
	float myCurrentRecoil;
private:

	bool myIsUnlocked;
	void ReceiveTryShootCallback(const AttackTypes aAttackType, const SB::Vector3f & aPosition);
	SB::EventRegistration < AttackTypes, SB::Vector3f> Event_TryShoot;
};
