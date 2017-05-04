#include "stdafx.h"
#include "ShellComponent.h"
#include "Game/Health/EnemyHealthComponent.h"
#include <Engine/Physics/Events/OnTriggerEnterEvent.h>
#include <Engine/Input/GameInputListener.h>

const uint8_t MaxBounces = 4;

ShellComponent::ShellComponent()
{
	myBounceCounter = 0;
	myCollidedThisFrame = false;

	myDamage = 0.f;
}


ShellComponent::~ShellComponent()
{
}

void ShellComponent::Initialize()
{
	myObject->AddEventSubscriber<SB::OnTriggerEnterEvent>(*this);
	myObject->AddEventSubscriber<SB::OnCollisionEvent>(*this);
}

void ShellComponent::SetDamage(const float aDamageValue)
{
	myDamage = aDamageValue;
}

void ShellComponent::Update(const SB::Time& aDeltaTime)
{
	if (GetInput().GetActionPressed(GameActions::eRemoveShots))
	{
		myObject->Remove();
	}
}

void ShellComponent::SetShooterObject(const SB::GameObject * aShooterObject)
{
	myShooterObject = aShooterObject;
}

void ShellComponent::RecieveEvent(const SB::OnTriggerEnterEvent& aEvent)
{
	if (aEvent.GameObjectCollidedWith->IsMine())
	{
		auto health = aEvent.GameObjectCollidedWith->GetComponent<EnemyHealthComponent>();

		if (health != nullptr)
		{
			health->TakeDamage(myDamage, myShooterObject);
			myObject->Remove();
		}
		else if (aEvent.collisionID == (0 | CollisionFilter::eRagDoll))
		{
			GetGameObject().Remove();
		}
	}
}

void ShellComponent::RecieveEvent(const SB::OnCollisionEvent & aEvent)
{
	if (myCollidedTimer.GetElapsedTime().InSeconds() > 0.1f)
	{
		myCollidedTimer.Restart();

		++myBounceCounter;
		if (myBounceCounter >= MaxBounces)
		{
			myObject->Remove();
		}
	}
}
