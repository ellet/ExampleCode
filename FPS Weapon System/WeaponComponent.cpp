#include "stdafx.h"
#include "WeaponComponent.h"
#include "Engine/Model/ModelComponent.h"
#include "Engine/Model/AssimpModel.h"
#include "../PlayerControllerComponent.h"
#include "Game/Player/WeaponAnimation/WeaponAnimationController.h"

const float WeaponComponent::ourMaxRecoil = 30.f;


WeaponComponent::WeaponComponent()
{

	Event_TryShoot = RegisterEvent<AttackTypes, SB::Vector3f>(SB::MessageDelivery::eReliableNonOrdered, true, &WeaponComponent::ReceiveTryShootCallback, this);
	myIsUnlocked = false;
	myAmmoContainer = nullptr;
	myCurrentRecoil = 0;
}

WeaponComponent::~WeaponComponent()
{

}

void WeaponComponent::PreInitialize()
{	
	if (SB::Engine::GetInstance().HasRenderer())
	{
		myAmmoContainer = myObject->GetScene().GetGUI()->GetGUIElement<SB::GUIElement>("ammoContainer");
	}
	SetupWeaponData();
}

SB::Vector3f WeaponComponent::GetShotDirection()
{
	SB::Quaternion PlayerOrientation = GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerLookDirection();
	SB::Quaternion shotRotation = PlayerOrientation;
	shotRotation.RotateAroundLocalZ(myRandomizer.GetRandomValue(0.f, TwoPi));
	shotRotation.RotateAroundLocalX(myCurrentRecoil / 1000.f);

	return shotRotation.GetForward().GetNormalized();
}

void WeaponComponent::FireWeapon(const AttackTypes aAttackTypeToFire)
{
	AttackData & currentData = GetAttackData(aAttackTypeToFire);	
	currentData.LastTryTimer = 0;

	if (currentData.CurrentAmmoCount >= currentData.AmmoCost)
	{
		while (currentData.LastFiredTimer >= currentData.FireRate)
		{
			currentData.LastFiredTimer -=  currentData.FireRate;
			Event_TryShoot(aAttackTypeToFire, myObject->GetWorldPosition());
			currentData.CurrentAmmoCount -= currentData.AmmoCost;
			GetGameObject().Play3DSound(currentData.WeaponFireSound);
			GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("crosshairHit");

			if (GetGameObject().GetComponent<PlayerControllerComponent>()->IsOurPlayer())
			{
				GetGameObject().GetComponent<PlayerControllerComponent>()->TryLookUp(-currentData.Recoil / 500.f);
			}

			myCurrentRecoil += currentData.Recoil;
			if (myCurrentRecoil > ourMaxRecoil)
				myCurrentRecoil = ourMaxRecoil;

		}
	}
	else
	{
		//Out of ammo click
		if (currentData.LastTryTimer >= currentData.FireRate)
		{
			GetGameObject().Play3DSound("Play_outofammo");
		}
	}
}

void WeaponComponent::Unlock()
{
	myIsUnlocked = true;
}

bool WeaponComponent::GetUnlockState() const
{
	return myIsUnlocked;
}

void WeaponComponent::SetAsActiveWeapon(SB::ObjectPtr & aWeaponObject)
{
	aWeaponObject->GetComponent<WeaponAnimationController>()->SwitchWeapon(myWeaponData.WeaponModel, myWeaponData.WeaponPosition, myWeaponData.WeaponColor);
	// aWeaponObject->GetComponent<SB::ModelComponent>()->SetModel(myWeaponData.WeaponModel);
	// aWeaponObject->SetPosition(myWeaponData.WeaponPosition);
	// aWeaponObject->GetComponent<SB::ModelComponent>()->SetColor(myWeaponData.WeaponColor);

	SpecificActivation(aWeaponObject);
}

bool WeaponComponent::AddAmmo(const AttackTypes aFiringType, const uint16_t aAmmoAmountToAdd)
{
	
	if (GetAttackData(aFiringType).CurrentAmmoCount < GetAttackData(aFiringType).maxAmmoCount)
	{
		GetAttackData(aFiringType).CurrentAmmoCount += aAmmoAmountToAdd;

		if (GetAttackData(aFiringType).CurrentAmmoCount > GetAttackData(aFiringType).maxAmmoCount)
		{
			GetAttackData(aFiringType).CurrentAmmoCount = GetAttackData(aFiringType).maxAmmoCount;
		}
		return true;
	}
	return false;
}

const SB::ComponentSerializer & WeaponComponent::GetSerializer() const
{
	static SB::ComponentSerializer serializer = Super::GetSerializer().Concatenate(SB::ComponentSerializer(
		&WeaponComponent::myIsUnlocked
	));
	return serializer;
}

void WeaponComponent::RecoilGuiElements(const std::string &aBaseGUIName) const
{
	if (IsOnClient() && GetGameObject().GetComponent<PlayerControllerComponent>()->IsOurPlayer())
	{
		std::string guiName;
		SB::SpriteGUIElement * gui;
		guiName = aBaseGUIName + "Up";
		gui = GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>(guiName);
		gui->SetPosition(-SB::Vector2f::UnitY * myCurrentRecoil);

		guiName = aBaseGUIName + "Down";
		gui = GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>(guiName);
		gui->SetPosition(SB::Vector2f::UnitY * myCurrentRecoil);

		guiName = aBaseGUIName + "Right";
		gui = GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>(guiName);
		gui->SetPosition(SB::Vector2f::UnitX * myCurrentRecoil);

		guiName = aBaseGUIName + "Left";
		gui = GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>(guiName);
		gui->SetPosition(-SB::Vector2f::UnitX * myCurrentRecoil);
	}
}

void WeaponComponent::LoadData(SB::DataNode aProperties)
{
	if (aProperties.HasMember("Primary") == true)
	{
		myWeaponData.PrimaryAttack.LoadAttackData(aProperties["Primary"]);
	}

	if (aProperties.HasMember("Secondary") == true)
	{
		myWeaponData.SecondaryAttack.LoadAttackData(aProperties["Secondary"]);
	}


}

void WeaponComponent::SetupAmmoIcons(const unsigned short aAmmoAmount)
{
	if (SB::Engine::GetInstance().HasRenderer())
	{
		int numberOfIcons = myWeaponData.SecondaryAttack.CurrentAmmoCount;
		int numberOfSmallIcons = aAmmoAmount;
		if (numberOfIcons > 5)
			numberOfIcons = 5;
		if (numberOfSmallIcons > 5)
			numberOfSmallIcons = 5;

		int numberOfGreyIcons = numberOfIcons - 1;
		int numberOFGreySmallIcons = numberOfSmallIcons - 1;

		

		myAmmoContainer->SetActiveIncludingChildren(false);
		for (auto it = myIcons.begin(); it != myIcons.end(); ++it)
		{
			if (it->first == IconTypes::eBig)
			{
				for (unsigned short i = 0; i < numberOfIcons; i++)
				{
					it->second[i]->SetActive(true);
				}
			}
			else if (it->first == IconTypes::eBigGrey)
			{
				for (short i = 4; i > numberOfGreyIcons; i--)
				{
					it->second[i]->SetActive(true);
				}
			}
			else if (it->first == IconTypes::eSmall)
			{
				for (unsigned short i = 0; i < numberOfSmallIcons; i++)
				{
					it->second[i]->SetActive(true);
				}
			}
			else
			{		
				for (short i = 4; i > numberOFGreySmallIcons; i--)
				{
					it->second[i]->SetActive(true);
				}
			}
		}
		myAmmoContainer->SetActive(true);
	}
}

void WeaponComponent::UpdateAmmoIcons()
{
	if (SB::Engine::GetInstance().HasRenderer())
	{
		for (auto it = myIcons.begin(); it != myIcons.end(); ++it)
		{
			if (it->first == IconTypes::eBig)
			{
				for (unsigned short i = 0; i < 5; i++)
				{
					if (i < myWeaponData.SecondaryAttack.CurrentAmmoCount)
						it->second[i]->SetActive(true);
					else
						it->second[i]->SetActive(false);
				}
			}
			else if (it->first == IconTypes::eBigGrey)
			{
				for (unsigned short i = 0; i < 5; i++)
				{
					if (i < myWeaponData.SecondaryAttack.CurrentAmmoCount)
						it->second[i]->SetActive(false);
					else
						it->second[i]->SetActive(true);
				}
			}
		}
	}
}

void WeaponComponent::Update(const SB::Time & aDeltaTime)
{
	myCurrentRecoil -= aDeltaTime.InSeconds() * 20.f;
	if (myCurrentRecoil < 0.f)
		myCurrentRecoil = 0.f;

	for (char i = 0; i < static_cast<char>(AttackTypes::enumLength); ++i)
	{
		AttackData & data = GetAttackData(static_cast<AttackTypes>(i));
		data.LastTryTimer += aDeltaTime;
		if (data.LastFiredTimer < data.FireRate)
		{
			data.LastFiredTimer += aDeltaTime;
		}

	}
}


SB::Vector3f WeaponComponent::GetFireDirection() const
{
	SB::Quaternion PlayerOrientation = GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerLookDirection();
	return PlayerOrientation.GetForward();
}

SB::Quaternion WeaponComponent::GetPlayerOrieantion() const
{
	return GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerLookDirection();
}

WeaponComponent::AttackData & WeaponComponent::GetAttackData(const AttackTypes aAttackType)
{
	if (aAttackType== AttackTypes::ePrimary)
	{
		return myWeaponData.PrimaryAttack;
	}
	else if (aAttackType == AttackTypes::eSeconday)
	{
		return myWeaponData.SecondaryAttack;
	}
	else
	{
		Error("Could not find attack type");
	}
}

void WeaponComponent::ReceiveTryShootCallback(const AttackTypes aAttackType, const SB::Vector3f & aPosition)
{
	if (IsMine())
	{
		AttackData & currentData = GetAttackData(aAttackType);

		if (currentData.CurrentAmmoCount >= currentData.AmmoCost)
		{
			if (aAttackType == AttackTypes::ePrimary)
			{
				FirePrimary();
			}
			else
			{
				FireSecondary();
			}
			if (IsOnServer())
			{
				currentData.CurrentAmmoCount -= currentData.AmmoCost;
			}
		}
	}
	else
	{
		if (aAttackType == AttackTypes::ePrimary)
		{
			FirePrimary();
		}
		else
		{
			FireSecondary();
		}
	}
	
}

void WeaponComponent::AttackData::LoadAttackData(SB::DataNode aProperties)
{
	AmmoCost = 0;
	CurrentAmmoCount = 100;
	maxAmmoCount = 100;
	FireRate = 1.f;
	WeaponDamage = 1.f;
	WeaponFireSound = MISSING_SOUND;
	Recoil = 0.f;
	if (aProperties.HasMember("Damage") == true)
	{
		WeaponDamage = aProperties["Damage"].GetFloat();
	}

	if (aProperties.HasMember("AmmoCost") == true)
	{
		AmmoCost = aProperties["AmmoCost"].GetUnsignedShort();

		if (AmmoCost > 0)
		{
			maxAmmoCount = aProperties["MaxAmmo"].GetUnsignedShort();

			if (aProperties.HasMember("StartAmmo") == true)
			{
				CurrentAmmoCount = aProperties["StartAmmo"].GetUnsignedShort();

				if (CurrentAmmoCount > maxAmmoCount)
				{
					Error("Start ammo is higher than maximum ammo");
				}
			}
			else
			{
				CurrentAmmoCount = maxAmmoCount;
			}
		}
	}

	if (aProperties.HasMember("Recoil") == true)
	{
		Recoil = aProperties["Recoil"].GetFloat();
	}

	if (aProperties.HasMember("FireRate") == true)
	{
		FireRate = aProperties["FireRate"].GetFloat();
	}

	if (aProperties.HasMember("AttackSound") == true)
	{
		WeaponFireSound = aProperties["AttackSound"].GetString();
	}

	if (aProperties.HasMember("EffectForce") == true)
	{
		effectForce = aProperties["EffectForce"].GetFloat();
	}
}

void WeaponComponent::CreateAmmoIcons(const std::string & aSprite, const std::string & aParentName, const std::string & weaponName, const unsigned short aPositionIndex, const float aScale, IconTypes aIconType)
{
	std::shared_ptr<SB::SpriteGUIElement> newIcon = std::make_shared<SB::SpriteGUIElement>(aSprite.c_str(), SB::Vector2f::Zero, false, SB::Vector2f::One * aScale, SB::Vector2f(512.f, 512.f));
	newIcon->SetName(weaponName + std::to_string(aPositionIndex));
	newIcon->SetActive(false);
	newIcon->SetAnchorPoint(SB::HorizontalAnchorPoint::eCenter, SB::VerticalAnchorPoint::eCenter);
	std::string parentName = aParentName;
	parentName += std::to_string(aPositionIndex);
	myAmmoContainer->GetGUIElement<SB::GUIElement>(parentName)->AddChild(newIcon);
	myIcons[aIconType].Add(&*newIcon);
}
