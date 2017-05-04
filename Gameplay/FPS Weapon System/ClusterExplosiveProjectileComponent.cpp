#include "stdafx.h"
#include "ClusterExplosiveProjectileComponent.h"
#include "Engine\Physics\Colliders\SphereColliderComponent.h"
#include "Engine\Physics\Actors\DynamicActorComponent.h"
#include "Engine\Streak\StreakLogic.h"
#include "Engine\Streak\StreakComponent.h"
#include "Engine\Model\ModelComponent.h"
#include "Engine\GameObject\TimedRemovalComponent.h"
#include "ShellComponent.h"
#include "Game\Enemy\EnemyComponent.h"
#include "Engine\Physics\Events\OnTriggerEnterEvent.h"
#include "Engine\Physics\Events\OnCollisionEvent.h"
#include "Game\Triggers\DamageTriggers\AttackExplosionComponent.h"
#include "Engine\Lighting\PointLightComponent.h"
#include "Engine\Texture\Texture.h"
#include "Engine\Feedback\VisualEffect\VisualEffectComponent.h"
#include "Game\EffectUtility\ElectricExplosionEffectComponent.h"
#include "Utilities\Tweening\Tween.h"
#include <Engine\Resources\ResourceManager.h>

SB::Time ActivationTime = 0.1f;

ClusterExplosiveProjectileComponent::ClusterExplosiveProjectileComponent()
{
	myLifeCounter = 0;
	myForce = 10;
	myStartSpeed = 10.f;
	myPlayTravelSound = false;
}


ClusterExplosiveProjectileComponent::~ClusterExplosiveProjectileComponent()
{
}

void ClusterExplosiveProjectileComponent::Initialize()
{
	GetGameObject().AddEventSubscriber<SB::OnTriggerEnterEvent>(*this);
	GetGameObject().AddEventSubscriber<SB::OnCollisionEvent>(*this);

	if (myPlayTravelSound == true && IsOnClient())
	{
		GetGameObject().GetComponent<SB::SpeakerComponent>()->PostSoundEvent("Play_electricityhum");
	}
}

void ClusterExplosiveProjectileComponent::SetPlayTravelSound(bool aValue)
{
	myPlayTravelSound = aValue;
}

void ClusterExplosiveProjectileComponent::SetDamage(const float aDamage)
{
	myDamage = aDamage;
}

void ClusterExplosiveProjectileComponent::SetForceAmount(const float aForce)
{
	myForce = aForce;
}

void ClusterExplosiveProjectileComponent::SetStartSpeed(const float aSpeed)
{
	myStartSpeed = aSpeed;
}

void ClusterExplosiveProjectileComponent::SetBounceLife(const uint8_t aBouneLifeAmount)
{
	myLifeCounter = aBouneLifeAmount;
}

void ClusterExplosiveProjectileComponent::SetRadius(const float aRadius)
{
	myRadius = aRadius;
}

void ClusterExplosiveProjectileComponent::SetShooterObject(const SB::GameObject * aShooterObject)
{
	myShooterObject = aShooterObject;
}

void ClusterExplosiveProjectileComponent::Explode(const SB::Vector3f & aUpvector)
{
	if (myActivationTimer.GetElapsedTime() < ActivationTime)
	{
		return;
	}

	unsigned short shells = myShellAmount;
	
	ReallyExplode();

	if (myLifeCounter > 0)
	{
		SB::Randomizer randomer;

		const float xDir = 0.35f;
		const float yDir = 0.75f;
		const float zDir = 0.35f;

		SB::Vector3f startDirection(xDir, yDir, zDir);
		startDirection.Normalize();

		SB::Quaternion xRotation;

		if (abs(aUpvector.x) > 0.9999f)
		{
			xRotation.RotateAroundLocalZ(HalfPi);
		}
		else
		{
			xRotation.RotateAroundLocalX(HalfPi);
		}

		SB::Vector3f forward = aUpvector * xRotation;
		SB::Quaternion upQuaterion = SB::Quaternion::LookRotation(forward, aUpvector);
		
		if (isnan(upQuaterion.GetXYZW().x) == true)
		{
			Error("Cluster explosion component quaterion is invalid");
		}

		//SB::Matrix44f rotation = SB::Matrix44f::CreateRotateAroundY(FullRotation / 3.f);

		uint8_t currentTurn = 0;
		while (shells-- > 0)
		{
			upQuaterion.RotateAlongAxis(aUpvector, FullRotation / myShellAmount);
			SB::Vector3f direction = startDirection * upQuaterion;
			direction.Normalize();
		
			const SB::Vector3f speed = direction * myStartSpeed;

			auto obj = GetScene().CreateLocalGameObject("Shot");
			obj->SetPosition(GetGameObject().GetWorldPosition());
			auto triggershape = obj->CreateLocalComponent<SB::SphereColliderComponent>();
			auto sound = obj->CreateLocalComponent<SB::SpeakerComponent>();
			auto shape = obj->CreateLocalComponent<SB::SphereColliderComponent>();
			auto actor = obj->CreateLocalComponent<SB::DynamicActorComponent>();

			if (SB::Engine::HasRenderer())
			{
				SB::GrowingArray<float> sizes;
				SB::GrowingArray<SB::Vector4f> colors;

				sizes.Add(8.f);
				colors.Add(SB::Color::White);
				colors.Add(SB::Color::Cyan);
				colors.Add(SB::Color::Black);

				SB::UnityStreakLogic * streakData = new SB::UnityStreakLogic(colors, sizes);
				auto streaky = obj->CreateLocalComponent<SB::StreakComponent>();
				streaky->Setup(streakData, 0.03f, 10);

				streaky->AddTexture(SB::Engine::GetInstance().GetResourceManager().Get<SB::Texture>("Assets/Textures/ElectricFlakTextures/electric01.dds"));

				auto spherey = obj->CreateLocalComponent<SB::VisualEffectComponent>();
				spherey->Setup(false, SB::eVisualShape::eSphere);
				spherey->SetColor(SB::Color::White);
				spherey->SetLocalScale(SB::Vector3f::One * 3.f);

				auto lighty = obj->CreateLocalComponent<SB::PointLightComponent>();
				lighty->SetColor(SB::Color::White);
				lighty->Setup(SB::Color::Cyan, 1.f, 70.f);
			}

			obj->CreateLocalComponent<SB::TimedRemovalComponent>()->NotifyRemove(5.f);
			auto projectileComp = obj->CreateLocalComponent<ClusterExplosiveProjectileComponent>();
			projectileComp->SetBounceLife(myLifeCounter - 1);
			projectileComp->SetShellAmount(3);
			projectileComp->SetShooterObject(myShooterObject);
			projectileComp->SetRadius(myRadius / 2.f);
			projectileComp->SetDamage(myDamage / 2.f);
			projectileComp->SetForceAmount(myForce / 2.f);
			projectileComp->SetStartSpeed(myStartSpeed * 0.85f);

			obj->Initialize();

			actor->EnableGravity();
			actor->ActivateSweepChecking();
			actor->SetVelocity(speed);
			actor->EnableGravity();
			actor->SetRotation(SB::Quaternion::LookRotation(direction));

			actor->SetFilterData(0 | CollisionFilter::ePlayerShot, 0 | CollisionFilter::eEnvironment | CollisionFilter::eEnemy);
		
		}	
	}
	{
		const SB::Vector3f speed = aUpvector * myStartSpeed;

		const SB::Time ExplosionTime = 1.7f;
		SB::ObjectPtr explosionEffectObject = myObject->GetScene().CreateLocalGameObject("Explosion");
		explosionEffectObject->SetPosition(myObject->GetWorldPosition());
		explosionEffectObject->CreateLocalComponent<ElectricExplosionEffectComponent>();
		explosionEffectObject->CreateLocalComponent<SB::TimedRemovalComponent>()->NotifyRemove(ExplosionTime);
		explosionEffectObject->GetComponent<ElectricExplosionEffectComponent>()->Setup(speed, static_cast<unsigned short>(myLifeCounter + 1));
		
		auto vis = explosionEffectObject->CreateLocalComponent<SB::VisualEffectComponent>();
		vis->Setup(false , SB::eVisualShape::eSphere);
		float moddedTime = ExplosionTime.InSeconds() * 0.5f;
		vis->SetColor(SB::Color::BrightCyan);
		{
			SB::TweenSettings sizeTween;
			sizeTween.myDuration = moddedTime;
			sizeTween.myStart = 0.f;
			sizeTween.myEnd = 40.f * static_cast<float>(myLifeCounter + 1);
			sizeTween.myLoopType = SB::LoopType::Once;
			sizeTween.myMod = SB::TweenMod::EaseOut;
			sizeTween.myType = SB::TweenType::Circular;
			vis->SetupSizeTween(sizeTween);
		}
		{
			SB::TweenSettings alphaTween;
			alphaTween.myDuration = moddedTime;
			alphaTween.myStart = 1.f;
			alphaTween.myEnd = 0.f;
			alphaTween.myLoopType = SB::LoopType::Once;
			alphaTween.myMod = SB::TweenMod::EaseInOut;
			alphaTween.myType = SB::TweenType::Cubic;
			vis->Fade(alphaTween);
		}
		explosionEffectObject->Initialize();
	}

	GetGameObject().Remove();
}

void ClusterExplosiveProjectileComponent::ReallyExplode()
{
	if (IsOnClient() == true)
	{
		GetGameObject().GetComponent<SB::SpeakerComponent>()->PostSoundEvent("Stop_electricityhum");

		GetGameObject().GetComponent<SB::SpeakerComponent>()->PostSoundEvent("Play_electricitybounce");
	}
	
	//GetGameObject().Play3DSound("Play_" + myExplosionSound);
	{
		SB::ObjectPtr explosionObject = myObject->GetScene().CreateLocalGameObject("Explosion");

		auto explosionComp = explosionObject->CreateLocalComponent<AttackExplosionComponent>();
		explosionComp->SetDamage(myDamage);
		explosionComp->SetEndRadius(myRadius);
		explosionObject->SetPosition(myObject->GetWorldPosition());
		explosionObject->GetComponent<AttackExplosionComponent>()->SetShooterObject(myShooterObject);
		explosionObject->GetComponent<AttackExplosionComponent>()->SetEffectForce(myForce);
		explosionObject->Initialize();
	}


}

void ClusterExplosiveProjectileComponent::RecieveEvent(const SB::OnTriggerEnterEvent& aEvent)
{
	if (aEvent.GameObjectCollidedWith->GetComponentCount<EnemyComponent>() > 0)
	{
		Explode(SB::Vector3f::UnitY);
	}
}

void ClusterExplosiveProjectileComponent::RecieveEvent(const SB::OnCollisionEvent& aEvent)
{
	Explode(aEvent.collisionNormal);
}
