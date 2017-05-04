#include "stdafx.h"
#include "PickupComponent.h"
#include "Game/Player/PlayerComponent.h"
#include "HealthPickup.h"
#include "Game/collisionFilters.h"
#include "Game/Player/PlayerTriggerOnContactComponent.h"
#include <Engine/Physics/Actors/DynamicActorComponent.h>
#include <Engine/Physics/Colliders/SphereColliderComponent.h>
#include <Engine/Physics/Events/OnTriggerEnterEvent.h>
#include <Engine/Physics/Actors/CharacterControllerComponent.h>
#include <Engine/Streak/StreakComponent.h>
#include <Engine/Streak/StreakLogic.h>
#include <Engine/Component/TimedRemovalComponent.h>
#include <Engine/Lighting/PointLightComponent.h>

PickupComponent::PickupComponent()
{
	myPickupEffect = nullptr;
	myTargetPlayer = nullptr;
	myEndOfRound = false;
	myAddedTriggerObject = false;
}

PickupComponent::~PickupComponent()
{

}

void PickupComponent::PreInitialize()
{
	myObject->CreateComponent<SB::PointLightComponent>()->Setup(SB::Color::Turquoise, 0.75f, 250.f);
	
}

void PickupComponent::Initialize()
{
	myStayAliveTime = 0.f;
	GetGameObject().AddEventSubscriber<SB::OnTriggerEnterEvent>(*this);
	GetGameObject().AddEventSubscriber<SB::OnCollisionEvent>(*this);
	
	/*#####################################################################################################
	SB::GrowingArray<SB::Vector4f> colors;
	colors.Add(SB::Color::Clouds);
	colors.Add(SB::Color::PeterRiver);
	SB::GrowingArray<float> sizes;
	sizes.Add(5.f);
	sizes.Add(0.f);

	myStreakObject = &*myObject->GetScene().CreateGameObject("Pickup Streaks");
	myStreakObject->SetPosition(myObject->GetWorldPosition());
	SB::UnityStreakLogic * logic = new SB::UnityStreakLogic(colors, sizes);
	myStreakObject->CreateComponent<SB::StreakComponent>()->Setup(logic, SB::Time::FromSeconds(0.1f), 10);
	myStreakObject->CreateComponent<SB::TimedRemovalComponent>();
	myStreakObject->Initialize();
	#####################################################################################################*/
}

void PickupComponent::PostInitialize()
{
	if (myPickupEffect == nullptr)
	{
		Error("Pickup component - pickupeffect not set");
	}
}

void PickupComponent::SetPickup(std::unique_ptr<BasePickup> & aPickup)
{
	myPickupEffect = std::move(aPickup);
}

void PickupComponent::EndUpdate(const SB::Time& aDeltaTime)
{
	if (myStreakObject != nullptr)
	{
		myStreakObject->SetPosition(myObject->GetWorldPosition());
	}
}

void PickupComponent::RecieveEvent(const SB::OnTriggerEnterEvent & aEvent)
{
	if (aEvent.myGameObject->GetComponentCount<PlayerComponent>() > 0)
	{
		/*################################################################################
		if (myStreakObject != nullptr)
		{
			SB::Vector3f pos = aEvent.myGameObject->GetWorldPosition();
			pos.y = myObject->GetPosition().y;
			myStreakObject->SetPosition(pos);
			myStreakObject->GetComponent<SB::TimedRemovalComponent>()->NotifyRemove(2.f);
			myStreakObject = nullptr;
		}
		################################################################################*/

		if (myPickupEffect != nullptr)
		{
			myPickupEffect->OnPickup(*aEvent.myGameObject);
			
			GetGameObject().GetComponent<SB::SpeakerComponent>()->PostSoundEvent("Play_Ting");
		}
		else
		{
			if (myEndOfRound == false)
			{
				Warning("Pickup effect not set on pickupcomponent");
			}
		}
		GetGameObject().Remove();
	}
}

void PickupComponent::RecieveEvent(const SB::OnCollisionEvent & aEvent)
{
	if (GetGameObject().IsRemoved() == false)
	{
		if (myTargetPlayer == nullptr)
		{
			SpawnFollowTriggerObject();
		}
		else
		{
			GetGameObject().GetComponent<SB::DynamicActorComponent>()->ApplyForce(SB::Vector3f::UnitY * 2000.f);
		}
	}
}

void PickupComponent::OnPlayerTrigger(const SB::GameObject & aPlayerObject)
{
	if (aPlayerObject.GetComponentCount<PlayerComponent>() > 0)
	{
		myTargetPlayer = &aPlayerObject;
		GetGameObject().GetComponent<SB::DynamicActorComponent>()->DisableGravity();

		if (myPlayerTriggerObject != nullptr)
		{
			myPlayerTriggerObject->Remove();
			myPlayerTriggerObject = SB::ObjectPtr();
		}
	}
	else
	{
		Warning("Player callback did not recieve a player game object");
	}
}

void PickupComponent::Update(const SB::Time & aDeltaTime)
{
	if (myTargetPlayer != nullptr)
	{
		const SB::Vector3f direction = myTargetPlayer->GetComponent<SB::CharacterControllerComponent>()->GetCenterPosition() - GetGameObject().GetPosition();
		if (myEndOfRound == false)
		{
			const float deltaForceAmount = 400000.f * aDeltaTime.InSeconds();

			GetGameObject().GetComponent<SB::DynamicActorComponent>()->ApplyForce(direction.GetNormalized() * deltaForceAmount);
			SB::Vector3f velocity = GetGameObject().GetComponent<SB::DynamicActorComponent>()->GetVelocity();
			float velocityLength = velocity.Length();
			const float maxVelocity = 20000.f;
			if (velocityLength >= maxVelocity)
			{
				velocity = velocity / velocityLength * maxVelocity;
				GetGameObject().GetComponent<SB::DynamicActorComponent>()->SetVelocity(velocity);
			}
		}
		else
		{
			GetGameObject().GetComponent<SB::DynamicActorComponent>()->SetVelocity(direction.GetNormalized() * 1000.f);
		}
	}
	else
	{
		if (myPlayerTriggerObject != nullptr)
		{
			myPlayerTriggerObject->GetComponent<PlayerTriggerOnContactComponent>()->SetToParentPosition(GetGameObject().GetWorldPosition());
		}

		myStayAliveTime += aDeltaTime.InSeconds();
		const float AliveTime = 5.f;

		if (myStayAliveTime >= AliveTime)
		{
			GetGameObject().Remove();
		}
	}
}



void PickupComponent::SpawnFollowTriggerObject()
{
	if (myAddedTriggerObject == false)
	{
		myAddedTriggerObject = true;
		SB::SphereShape colliderShape;
		colliderShape.myPosition = SB::Vector3f::Zero;
		colliderShape.myRadius = 500.f;

		myPlayerTriggerObject = GetGameObject().GetScene().CreateGameObject("PickupPlayerTrigger");

		auto && collider = myPlayerTriggerObject->CreateComponent<SB::SphereColliderComponent>();
		collider->SetShape(colliderShape);
		collider->SetIsTrigger();

		auto && PlayerTrigger = myPlayerTriggerObject->CreateComponent<PlayerTriggerOnContactComponent>();
		PlayerTrigger->SetOnContactCallback(*this);

		auto && actor = myPlayerTriggerObject->CreateComponent<SB::DynamicActorComponent>();
		myPlayerTriggerObject->Initialize();

		actor->SetFilterData(0 | CollisionFilter::ePickup, 0 | CollisionFilter::ePlayer);
	}
}

void PickupComponent::OnRemoved()
{
	/*###############################################################################
	if (myStreakObject != nullptr)
	{
		myStreakObject->GetComponent<SB::TimedRemovalComponent>()->NotifyRemove(2.f);
		myStreakObject = nullptr;
	}
	###############################################################################*/

	if (myPlayerTriggerObject != nullptr)
	{
		myPlayerTriggerObject->Remove();
		myPlayerTriggerObject = SB::ObjectPtr();
	}
}

SB::ReceiveResult PickupComponent::Receive(const RoundEndMessage & aMessage)
{
	/*
		At the end of round the player should get all the score pickups he/she hasn't had the time to pick up yet, so the pickupeffect is triggered straight away.
	*/

	if (myPickupEffect != nullptr)
	{
		myEndOfRound = true;
		myPickupEffect->OnPickup(*aMessage.PlayerObjectPointer);
		myPickupEffect = nullptr;
		SpawnFollowTriggerObject();
		if (myPlayerTriggerObject != nullptr)
		{
			myPlayerTriggerObject->GetComponent<SB::SphereColliderComponent>()->SetRadius(9999.f);
		}
	}
	else
	{
		Warning(GetGameObject().GetIdentifier() + ": pickupeffect was nullptr on end of round message");
	}

	return SB::ReceiveResult::eContinue;
}


