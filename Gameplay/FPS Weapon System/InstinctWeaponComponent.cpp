#include "stdafx.h"
#include "InstinctWeaponComponent.h"
#include "Engine\Input\GameInputListener.h"
#include "Engine\Physics\Colliders\SphereColliderComponent.h"
#include "Engine\Physics\Actors\DynamicActorComponent.h"
#include "Engine\Streak\StreakLogic.h"
#include "Engine\Streak\StreakComponent.h"
#include "Engine\GameObject\TimedRemovalComponent.h"
#include "ShellComponent.h"
#include "Engine\Model\ModelComponent.h"
#include "..\PlayerControllerComponent.h"
#include "Engine\Model\AssimpModel.h"
#include "Engine\Engine.h"
#include "Engine\Resources\ResourceManager.h"
#include "Utilities\Math\Matrix44.h"
#include "Engine\Physics\System\PhysicsManager.h"
#include "Utilities\Shapes\RayShape.h"
#include "Game/Game.h"
#include "Game\Health\EnemyHealthComponent.h"
#include "Game\Player\Weapons\ExplosiveProjectileComponent.h"
#include <Engine/Debugging/DebugDrawer.h>
#include "Engine\Lighting\PointLightComponent.h"
#include "..\WeaponAnimation\WeaponAnimationController.h"
#include "Engine\Physics\Actors\CharacterControllerComponent.h"
#include "Engine\Physics\Colliders\BoxColliderComponent.h"
#include "Engine\Physics\Actors\RagdollComponent.h"

InstinctWeaponComponent::InstinctWeaponComponent()
{
	myMuzzleStayTime = 0.2f;
	
}


InstinctWeaponComponent::~InstinctWeaponComponent()
{
}

void InstinctWeaponComponent::PreInitialize()
{
	myMuzzleFlash = GetScene().CreateLocalObjectAndAddModel("MuzzleFlash", "Assets/Models/EffectModels/muzzleFlash.fbx", SB::Vector3f::Zero);
	myMuzzleFlash->SetScale({ 7.5f , 7.5f, 7.5f });
	auto && modelComponent = myMuzzleFlash->GetComponent<SB::ModelComponent>();
	modelComponent->DisableAutomaticRender();
	modelComponent->SetColor(SB::Color::PeterRiver);
	modelComponent->SetMatrix(SB::Matrix44f::CreateRotateAroundY(HalfPi) * SB::Matrix44f::CreateTranslation(0.f, 0.f, 0.f));
	myMuzzleFlashLight = &*myMuzzleFlash->CreateLocalComponent<SB::PointLightComponent>();
	myMuzzleFlashLight->Setup(SB::Color::PeterRiver, 0.5f, 250.f);
	myMuzzleFlash->Initialize();

	WeaponComponent::PreInitialize();
}

void InstinctWeaponComponent::Update(const SB::Time & aDeltaTime)
{
	if (myMuzzleTimer.GetElapsedTime() > myMuzzleStayTime)
	{
		myMuzzleFlash->GetComponent<SB::ModelComponent>()->DisableAutomaticRender();
	}
	Super::Update(aDeltaTime);
}

void InstinctWeaponComponent::DebugRender(const SB::Camera & aCamera, const uint32_t aDebugRenderMode) const
{
	SB::Engine::GetDebugDrawer().RenderArrow(aCamera, linestartpos, lineendpos, SB::Color::Red);
}

void InstinctWeaponComponent::SetAsActiveWeapon(SB::ObjectPtr & aWeaponObject)
{
	Super::SetAsActiveWeapon(aWeaponObject);
}

void InstinctWeaponComponent::FirePrimary()
{
	myMuzzleFlash->GetComponent<SB::ModelComponent>()->EnableAutomaticRender();
	myMuzzleTimer.Restart();

	SB::Quaternion muzzleFlashRotation;
	muzzleFlashRotation.RotateAroundLocalZ(myRandomizer.GetRandomValue(0.f, TwoPi));
	myMuzzleFlash->SetRotation(muzzleFlashRotation);

	GetGameObject().TraversingGetComponent<WeaponAnimationController>()->PlayAssaultRiflePrimary();


	SB::Quaternion PlayerOrientation = GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerLookDirection();

	const SB::Vector3f firePos = GetGameObject().GetComponent<PlayerControllerComponent>()->GetVisionObject()->GetWorldPosition();
	const SB::Vector3f forward = PlayerOrientation.GetForward().GetNormalized();

	SB::RaytraceInput rayInput;
	rayInput.startPosition = firePos;
	rayInput.direction = forward;
	rayInput.collisionFilter = 0 | CollisionFilter::eEnvironment | CollisionFilter::eEnemy |CollisionFilter::eRagDoll;

	CreateAmmoShell(SB::Vector3f(0.6f, 0.6f, 1.3f), SB::Color::SunFlower);

	SB::RaytraceOutput rayOutput;

	if (GetScene().GetPhysicsManager().Raytrace(rayInput, rayOutput) == true)
	{

		linestartpos = rayInput.startPosition;
		lineendpos = rayOutput.hitPosition;
		if (rayOutput.collisionIDOfObjectHit == static_cast<uint32_t>(CollisionFilter::eRagDoll))
		{
			rayOutput.gameObjectHit->GetComponent<SB::RagdollComponent>()->ApplyForce(rayOutput.hitPosition, forward * 700000.f);
		}
		else
		if (rayOutput.gameObjectHit->IsMine())
		{
			DealDamage(*rayOutput.gameObjectHit);
		}
	}
}

void InstinctWeaponComponent::SetupWeaponData()
{
	std::shared_ptr<SB::AssimpModel> weaponModel;

	if (Game::GetInstance().IsClient())
	{
		weaponModel = SB::Engine::GetResourceManager().Get<SB::AssimpModel>("Assets/Models/Weapons/AssaultRifle/assaultRifle.fbx");
		weaponModel->MakeReady(false);
	}

	myWeaponPos = { 22.f, -22.f, 35.f };

	myFirePos = myWeaponPos;
	myFirePos.z -= 65.f;
	myFirePos.y += 3.f;

	myWeaponData.WeaponPosition = myWeaponPos;
	myWeaponData.WeaponModel = weaponModel;
	myWeaponData.WeaponColor = SB::Color(2.f,0.5f,0.5f,1.f);

	myWeaponData.PrimaryAttack.FirePosition = myFirePos;
	myWeaponData.PrimaryAttack.FireRate = 1.f / 3.5f;
	myWeaponData.PrimaryAttack.WeaponDamage = 1000000.f;
	myWeaponData.PrimaryAttack.AmmoCost = 0;
	myWeaponData.PrimaryAttack.WeaponFireSound = "Play_AssaultRifleFire";

	myWeaponData.SecondaryAttack.FirePosition = myFirePos;
	myWeaponData.SecondaryAttack.FireRate = 2.f;
	myWeaponData.SecondaryAttack.WeaponDamage = 50.f;
}

void InstinctWeaponComponent::SpecificActivation(SB::ObjectPtr & aWeaponObject)
{
	myMuzzleFlash->SetParent(aWeaponObject);
	SB::Vector3f firePos = myWeaponData.PrimaryAttack.FirePosition - myWeaponPos;
	firePos.z = -firePos.z;
	myMuzzleFlash->SetPosition(firePos);
}

void InstinctWeaponComponent::FireSecondary()
{
	
}

void InstinctWeaponComponent::DealDamage(SB::GameObject & aHitObject)
{
	if (aHitObject.GetComponentCount<EnemyHealthComponent>() > 0)
	{
		aHitObject.GetComponent<EnemyHealthComponent>()->TakeDamage(myWeaponData.PrimaryAttack.WeaponDamage, &GetGameObject());
	}
}

void InstinctWeaponComponent::CreateAmmoShell(const SB::Vector3f & aAmmoScale, const SB::Color & aAmmoColor)
{
	SB::Vector3f position = myObject->GetWorldPosition();
	auto ammoShell = GetScene().CreateLocalObjectAndAddModel("Ammo Shell", "Assets/Models/Objects/Primitives/Cube.fbx", SB::Vector3f::Zero);
	ammoShell->SetScale(aAmmoScale);
	ammoShell->GetComponent<SB::ModelComponent>()->SetColor(aAmmoColor);
	SB::Vector3f pos = GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerCenterPosition() + ((myFirePos + SB::Vector3f(5.f, 5.f, 80.f)) * GetPlayerOrieantion());
	ammoShell->SetPosition(pos);
	auto shape = ammoShell->CreateLocalComponent<SB::BoxColliderComponent>();
	auto actor = ammoShell->CreateLocalComponent<SB::DynamicActorComponent>();
	ammoShell->CreateLocalComponent<SB::TimedRemovalComponent>()->NotifyRemove(2.f);
	ammoShell->Initialize();

	actor->EnableGravity();
	actor->SetVelocity(myObject->GetComponent<SB::CharacterControllerComponent>()->GetVelocity());
	actor->ApplyForce(SB::Vector3f(myRandomizer.GetRandomValue(5000.f, 9000.f), myRandomizer.GetRandomValue(7000.f, 12000.f), 0.f) * GetPlayerOrieantion());
	SB::Quaternion ammoShellRotation;
	ammoShellRotation.RotateAroundLocalX(myRandomizer.GetRandomValue(0.f, TwoPi));
	actor->SetRotation(ammoShellRotation);
	actor->SetFilterData(0 | CollisionFilter::eAmmoShell, 0 | CollisionFilter::eEnvironment);
}

