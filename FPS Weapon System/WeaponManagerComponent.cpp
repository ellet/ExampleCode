#include "stdafx.h"
#include "WeaponManagerComponent.h"
#include "FlakCannonComponent.h"
#include "Engine\Input\GameInputListener.h"
#include "..\PlayerControllerComponent.h"
#include "Engine\Model\ModelComponent.h"
#include "Engine\Model\AssimpModel.h"
#include "AssaultRifleComponent.h"
#include "Engine\Text\Text.h"
#include "Engine\Engine.h"
#include "Engine\Resources\ResourceManager.h"
#include "Game/Game.h"
#include "InstinctWeaponComponent.h"
#include "Engine/Rendering/CommonRenderData.h"
#include "Game/Player/WeaponAnimation/WeaponAnimationController.h"
#include "Engine\Effect\StandardEffect.h"
#include "Game/Player/PlayerComponent.h"
#include <Engine/Camera/Camera.h>
#include <Engine/Rendering/DXRenderer.h>
#include "Engine\Rendering\RenderingEventGroup.h"


WeaponManagerComponent::WeaponManagerComponent()
{
	myFlakCannon = nullptr;
	myAssaultRifle = nullptr;
	myLastWeapon = nullptr;
	myActiveWeapon = nullptr;
	myLockAllActions = false;
	myInstinctActivated = false;
	Event_SwitchWeapon = RegisterEvent<const SB::ComponentPtr<WeaponComponent> &, const SB::ObjectPtr &>(SB::MessageDelivery::eReliableNonOrdered, true, &WeaponManagerComponent::SwitchWeaponCallback, this);

	Event_UnlockedWeapon = RegisterEvent<const uint8_t, const bool>(SB::MessageDelivery::eReliableNonOrdered, false, &WeaponManagerComponent::RecieveUnlockedWeapon, this);
	if (IsOnClient())
	{
		myWriteTypeValueEffect = std::make_shared<SB::StandardEffect>("shaders/pbr/vertex.fx", "VShader", "shaders/pbr/writeRefPixel.fx", "PShader");
		myWriteRefValueBufferData = std::make_shared<SB::ConstantBuffer<SB::ConstantBuffers::WriteRefValueData>>();
	}
	myOutlineRefValue = 60.f;
}


WeaponManagerComponent::~WeaponManagerComponent()
{
}

void WeaponManagerComponent::PreInitialize()
{
	if (myPlayer == nullptr)
	{
		Error("SetPlayer must be called before WeaponManagerComponent is initialized!");
	}

	if (Game::GetInstance().IsClient())
	{
		//myAmmoText = std::make_unique<SB::Text>();
		//myAmmoText->SetFont(SB::Engine::GetResourceManager().Get<SB::Font>("Assets/Text/Calibri"));
	}

	myWeaponObject = GetScene().CreateLocalGameObject("weaponObject");
	myWeaponObject->SetPosition(myObject->GetWorldPosition());
	myWeaponObject->CreateLocalComponent<WeaponAnimationController>()->SetPlayer(*myPlayer);
	myWeaponObject->SetParent(GetGameObject().GetComponent<PlayerControllerComponent>()->GetVisionObject());

	myWeaponObject->SetRotation(SB::Quaternion::LookRotation({ 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }));
	myWeaponObject->Initialize();

	myFlakCannon = &*GetGameObject().RequireGlobalComponent<FlakCannonComponent>();
	myAssaultRifle = &*GetGameObject().RequireGlobalComponent<AssaultRifleComponent>();	

	LoadWeaponData(*myAssaultRifle, "Assets/Data/Weapons/AssaultRifle.json");
	LoadWeaponData(*myFlakCannon, "Assets/Data/Weapons/FlakCannon.json");

	myAssaultRifle->Unlock();
	

	myInstinctWeapon = &*GetGameObject().RequireGlobalComponent<InstinctWeaponComponent>();
	myInstinctWeapon->Unlock();
}

void WeaponManagerComponent::Initialize()
{
	SwitchWeapon(*myAssaultRifle);

	myObject->AddEventSubscriber<ObjectOutOfHealthMessage>(*this);
	myObject->AddEventSubscriber<ObjectRespawnedMessage>(*this);
}

void WeaponManagerComponent::Update(const SB::Time & aDeltaTime)
{
	if (myLockAllActions == true)
	{
		return;
	}

	if (myInstinctActivated == false)
	{
		if (GetInput().GetActionPressed(GameActions::eWeapon_Assault) == true && GetInput().GetActionHeld(GameActions::eShoot) == false)
		{
			if (IsOnClient() && myPlayer->GetComponent<PlayerControllerComponent>()->IsOurPlayer())
			{
				Event_SwitchWeapon(myAssaultRifle->AsPointer<WeaponComponent>(), myObject->AsPointer());
			}
		}
		else if (GetInput().GetActionPressed(GameActions::eWeapon_Flak) == true && GetInput().GetActionHeld(GameActions::eShoot) == false)
		{
			if (IsOnClient() && myPlayer->GetComponent<PlayerControllerComponent>()->IsOurPlayer())
			{
				Event_SwitchWeapon(myFlakCannon->AsPointer<WeaponComponent>(), myObject->AsPointer());
			}
		}
	}

	if (GetInput().GetActionHeld(GameActions::eAlternateShoot) && myInstinctActivated == false)
	{
		if (IsOnClient() && myPlayer->GetComponent<PlayerControllerComponent>()->IsOurPlayer())
		{
			myActiveWeapon->FireWeapon(AttackTypes::eSeconday);
			UpdateGUIIcons(true);
		}
	}
	else if (GetInput().GetActionHeld(GameActions::eShoot))
	{
		if (IsOnClient() && myPlayer->GetComponent<PlayerControllerComponent>()->IsOurPlayer())
		{
			myActiveWeapon->FireWeapon(AttackTypes::ePrimary);

		}
	}
	

	//if (myAmmoText != nullptr && myPlayer->GetComponent<PlayerControllerComponent>()->IsOurPlayer() == true)
	//{
	//	myAmmoText->SetText("Primary ammo: " + std::to_string(myActiveWeapon->GetPrimaryAmmoCount()) +
	//		"\n Secondary ammo: " + std::to_string(myActiveWeapon->GetSecondaryAmmoCount()));
	//}
}



void WeaponManagerComponent::RenderGUI(const SB::GuiRenderTarget & aRenderTarget) const
{
	//if (myAmmoText != nullptr)
	//{
	//	myAmmoText->Render(aRenderTarget);
	//}
}

bool WeaponManagerComponent::AddAmmo(const AmmoData & aAmmoData)
{
	bool didAddAmmo = GetWeapon(aAmmoData.WeaponType).AddAmmo(aAmmoData.FireModeType, aAmmoData.AmmoAmount);
	UpdateGUIIcons();
	
	return didAddAmmo;
}

void WeaponManagerComponent::RecieveEvent(const ObjectOutOfHealthMessage & aEvent)
{
	myLockAllActions = true;
}

void WeaponManagerComponent::RecieveEvent(const ObjectRespawnedMessage & aEvent)
{
	myLockAllActions = false;
}

void WeaponManagerComponent::SetUseInstinctWeapon(const bool aValue)
{ 
	myInstinctActivated = aValue;
	if (IsMine())
	{
		if (myInstinctActivated == true)
		{
			if (myActiveWeapon != myInstinctWeapon)
			{
				myLastWeapon = myActiveWeapon;
			}
			
			Event_SwitchWeapon(myInstinctWeapon->AsPointer<WeaponComponent>(), myObject->AsPointer());
		}
		else
		{
			Event_SwitchWeapon(myLastWeapon == nullptr ? nullptr : myLastWeapon->AsPointer<WeaponComponent>(), myObject->AsPointer());
		}
	}
}

void WeaponManagerComponent::SetPlayer(SB::GameObject& aPlayer)
{
	myPlayer = &aPlayer;
}

void WeaponManagerComponent::UnlockWeapon(const WeaponTypes aWeaponTypeToUnlock, const bool aShouldSwithTo /*= true*/)
{
	if (IsMine() == true)
	{
		Event_UnlockedWeapon(static_cast<uint8_t>(aWeaponTypeToUnlock), aShouldSwithTo);
	}
}

void WeaponManagerComponent::EndRender(const SB::Camera & aCamera) const
{
	if (myObject->GetComponent<PlayerComponent>()->ShouldRenderThirdPerson(aCamera.GetRenderingBindMode()) == false)
	{
		SB::RenderingEventGroup g(L"Weapon Rendering");

		SB::Engine::GetRenderer().DisableDepthRead();
		SB::ConstantBuffers::WriteRefValueData data;
		data.gWriteRefValue.myRefValue = myOutlineRefValue;
		data.gWriteRefValue.myPercentageShown = 0.f;
		myWriteRefValueBufferData->UpdateData(data);
		myWriteRefValueBufferData->BindToPS(3);
		myWeaponObject->GetComponent<SB::ModelComponent>()->SetEffect(myWriteTypeValueEffect);
		myWeaponObject->GetComponent<SB::ModelComponent>()->InstantRender(aCamera);

;
		SB::Engine::GetRenderer().EnableDepth();
		myWeaponObject->GetComponent<SB::ModelComponent>()->InstantRender(aCamera);
	}
}

void WeaponManagerComponent::SwitchWeapon(WeaponComponent & aWeaponToSwitchTo)
{
	if (myLockAllActions == true)
	{
		return;
	}

	if (aWeaponToSwitchTo.GetUnlockState() == true && &aWeaponToSwitchTo != myActiveWeapon)
	{
		myActiveWeapon = &aWeaponToSwitchTo;
		aWeaponToSwitchTo.SetAsActiveWeapon(myWeaponObject);
		myWeaponObject->GetComponent<SB::ModelComponent>()->DisableAutomaticRender();
		UpdateGUIIcons();
	}
}

void WeaponManagerComponent::SwitchWeaponCallback(const SB::ComponentPtr<WeaponComponent> & aWeaponToSwitchTo, const SB::ObjectPtr & aObject)
{
	if (aWeaponToSwitchTo != nullptr)
	{
		if (myWeaponObject->GetComponent<WeaponAnimationController>()->IsPlayingMove() && aObject->GetComponent<PlayerControllerComponent>()->IsOurPlayer() == true)
		{
			SwitchWeapon(*aWeaponToSwitchTo);
		}
	}
}

void WeaponManagerComponent::LoadWeaponData(WeaponComponent & aWeaponToLoad, const std::string & aFilePath)
{
	SB::DataDocument weaponDoc;
	SB::Data::Open(aFilePath, weaponDoc);

	aWeaponToLoad.LoadData(weaponDoc["WeaponData"]);
}

void WeaponManagerComponent::RecieveUnlockedWeapon(const uint8_t aWeapon, const bool aShouldSwitchToo)
{
	GetWeapon(static_cast<WeaponTypes>(aWeapon)).Unlock();
	if (SB::Engine::HasRenderer())
	{
		myActiveWeapon->SetupAmmoIcons(GetWeapon(static_cast<WeaponTypes>(aWeapon)).GetSecondaryAmmoCount());
	}
	if (aShouldSwitchToo == true)
	{
		if (GetInput().GetActionHeld(GameActions::eShoot) == false && GetInput().GetActionHeld(GameActions::eAlternateShoot) == false)
		{
			SwitchWeapon(GetWeapon(static_cast<WeaponTypes>(aWeapon)));
		}
	}
}

WeaponComponent & WeaponManagerComponent::GetWeapon(const WeaponTypes aWeaponType)
{
	if (aWeaponType == WeaponTypes::eFlak)
	{
		return *myFlakCannon;
	}
	else if (aWeaponType == WeaponTypes::eAssault)
	{
		return *myAssaultRifle;
	}
	else
	{
		Error("Weapon Type not found");
	}
}

void WeaponManagerComponent::UpdateGUIIcons(const bool aShouldOnlyUpdateBigIcons)
{
	if (myActiveWeapon == nullptr)
	{
		if (myFlakCannon->GetUnlockState() == true)
		{
			myAssaultRifle->SetupAmmoIcons(myFlakCannon->GetSecondaryAmmoCount());
		}
		else
		{
			myAssaultRifle->SetupAmmoIcons(0);
		}
	}
	else
	{
		if (aShouldOnlyUpdateBigIcons == true)
		{
			myActiveWeapon->UpdateAmmoIcons();
		}
		else
		{
			if (myActiveWeapon == myAssaultRifle)
			{
				myAssaultRifle->SetupAmmoIcons(myFlakCannon->GetSecondaryAmmoCount());
			}
			else
			{
				myFlakCannon->SetupAmmoIcons(myAssaultRifle->GetSecondaryAmmoCount());
			}
		}	
	}
}

