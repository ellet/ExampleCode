#include "stdafx.h"
#include "ExplosiveProjectileComponent.h"
#include "Game/Triggers/DamageTriggers/AttackExplosionComponent.h"
#include "Game/Enemy/EnemyComponent.h"
#include "Engine/Physics/Events/OnTriggerEnterEvent.h"
#include "Engine/Physics/Actors/StaticActorComponent.h"
#include "Engine/Physics/Colliders/SphereColliderComponent.h"
#include "Engine/ParticleSystem/ParticleComponent.h"
#include "Game/EffectUtility/FireExplosionEffectComponent.h"
#include "Engine/Streak/StreakLogic.h"
#include "Engine/Streak/StreakComponent.h"
#include "Engine\Texture\Texture.h"
#include <Engine/Resources/ResourceManager.h>

ExplosiveProjectileComponent::ExplosiveProjectileComponent()
{
	myRadius = 0.f;
	myDamage = 0.f;
}


ExplosiveProjectileComponent::~ExplosiveProjectileComponent()
{
}

void ExplosiveProjectileComponent::Initialize()
{
	GetGameObject().AddEventSubscriber<SB::OnTriggerEnterEvent>(*this);
	GetGameObject().AddEventSubscriber<SB::OnCollisionEvent>(*this);
	GetGameObject().GetComponent<SB::SpeakerComponent>()->PostSoundEvent("Play_" + myTravelSound);

}

void ExplosiveProjectileComponent::Update(const SB::Time& aDeltaTime)
{
}

void ExplosiveProjectileComponent::SetShooterObject(const SB::GameObject * aShooterObject)
{
	myShooterObject = aShooterObject;
}

void ExplosiveProjectileComponent::SetExplosionForce(const float aExplosionForce)
{
	myExplosionForce = aExplosionForce;
}

void ExplosiveProjectileComponent::PreInitialize()
{
	if (SB::Engine::HasRenderer())
	{
		SB::GrowingArray<float> sizes;
		SB::GrowingArray<SB::Vector4f> colors;
		sizes.Add(0.f);
		sizes.Add(20.f);
		sizes.Add(15.f);
		sizes.Add(0.f);
		colors.Add(SB::Color::Black);
		colors.Add(SB::Color::White);
		colors.Add(SB::Color::Yellow);
		colors.Add(SB::Color::Black);
		SB::UnityStreakLogic * streakData = new SB::UnityStreakLogic(colors, sizes);
		auto streaky = myObject->CreateLocalComponent<SB::StreakComponent>();
		streaky->Setup(streakData, 0.03f, 10);
		streaky->AddTexture(SB::Engine::GetResourceManager().Get<SB::Texture>("Assets/Textures/ExplosionTexture/explosion01.dds"));
		//SB::ComponentPtr<SB::ParticleComponent> particles = myObject->CreateLocalComponent<SB::ParticleComponent>();
		//particles->SetEmitter("LargeSmokeRockets", true, false, SB::Vector3f::Zero, 0.5f);
		//particles->SetDynamicParticleSize(0.3f);
	}
}

void ExplosiveProjectileComponent::SetTravelSound(const std::string &aSound)
{
	myTravelSound = aSound;
}
void ExplosiveProjectileComponent::SetExplosionSound(const std::string &aSound)
{
	myExplosionSound = aSound;
}
void ExplosiveProjectileComponent::Explode()
{
	GetGameObject().GetComponent<SB::SpeakerComponent>()->PostSoundEvent("Stop_" + myTravelSound);

	GetGameObject().Play3DSound("Play_" + myExplosionSound);
	GetGameObject().Remove();

	SB::ObjectPtr explosionObject = myObject->GetScene().CreateLocalGameObject("Explosion");

	explosionObject->CreateLocalComponent<FireExplosionEffectComponent>()->Setup(myExplosionForce, static_cast<unsigned short>(RandomGenerator.GetRandomValue(3, 10)));
	explosionObject->CreateLocalComponent<AttackExplosionComponent>()->SetDamage(myDamage);
	explosionObject->SetPosition(myObject->GetWorldPosition());
	//explosionObject->GetComponent<AttackExplosionComponent>()->SetEffectData(SB::Vector3f::UnitY * 1000.f, 10);
	explosionObject->GetComponent<AttackExplosionComponent>()->SetShooterObject(myShooterObject);
	explosionObject->GetComponent<AttackExplosionComponent>()->SetEffectForce(myExplosionForce);

	explosionObject->Initialize();

}

void ExplosiveProjectileComponent::RecieveEvent(const SB::OnTriggerEnterEvent& aEvent)
{
	if (aEvent.GameObjectCollidedWith->GetComponentCount<EnemyComponent>() > 0)
	{
		Explode();
	}
}

void ExplosiveProjectileComponent::RecieveEvent(const SB::OnCollisionEvent& aEvent)
{
	Explode();
}
