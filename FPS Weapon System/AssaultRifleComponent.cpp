#include "stdafx.h"
#include "AssaultRifleComponent.h"
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
#include "Game/Player/WeaponAnimation/WeaponAnimationController.h"
#include <Engine/Lighting/PointLightComponent.h>
#include "Engine\ParticleSystem\ParticleComponent.h"
#include "Engine\Physics\Colliders\BoxColliderComponent.h"
#include "Engine\Physics\Actors\CharacterControllerComponent.h"
#include "Engine\Physics\Actors\RagdollComponent.h"
#include "Engine\GUI\SpriteGUIElement.h"
#include "Engine\Texture/Texture.h"

AssaultRifleComponent::AssaultRifleComponent()
{
	myMuzzleStayTime = 0.1f;
	myTimesFired = 0;
	myDisplayTip = true;
	myMuzzleFlashLight = nullptr;
}


AssaultRifleComponent::~AssaultRifleComponent()
{
}

void AssaultRifleComponent::PreInitialize()
{
	myMuzzleFlash = GetScene().CreateLocalObjectAndAddModel("MuzzleFlash", "Assets/Models/EffectModels/muzzleFlash.fbx", SB::Vector3f::Zero);
	myMuzzleFlash->SetScale({7.5f , 7.5f, 7.5f});
	auto && modelComponent = myMuzzleFlash->GetComponent<SB::ModelComponent>();
	modelComponent->SetRenderingFilter(SB::RenderingMode::ePBR);
	modelComponent->DisableAutomaticRender();
	modelComponent->SetColor(SB::Color::PeterRiver);
	modelComponent->SetMatrix(SB::Matrix44f::CreateRotateAroundY(HalfPi) * SB::Matrix44f::CreateTranslation(0.f, 0.f, 0.f));
	myMuzzleFlashLight = &*myMuzzleFlash->CreateLocalComponent<SB::PointLightComponent>();
	myMuzzleFlashLight->Setup(SB::Color::PeterRiver, 0.5f, 250.f);
	myMuzzleFlash->Initialize();

	WeaponComponent::PreInitialize();
}

void AssaultRifleComponent::Update(const SB::Time & aDeltaTime)
{
	std::string baseGuiName = "assaultRifleCrosshair";
	RecoilGuiElements(baseGuiName);


	if (myMuzzleTimer.GetElapsedTime() > myMuzzleStayTime)
	{
		myMuzzleFlash->GetComponent<SB::ModelComponent>()->DisableAutomaticRender();
		myMuzzleFlashLight->SetIntensity(0.f);
	}
	else
	{
		myMuzzleFlashLight->SetIntensity(0.5f - (myMuzzleTimer.GetElapsedTime().InSeconds() / myMuzzleStayTime.InSeconds()) * 0.5f);
	}
	Super::Update(aDeltaTime);
}


void AssaultRifleComponent::DebugRender(const SB::Camera & aCamera, const uint32_t aDebugRenderMode) const
{
	SB::Engine::GetDebugDrawer().RenderArrow(aCamera, linestartpos, lineendpos, SB::Color::Red);
}

void AssaultRifleComponent::SetAsActiveWeapon(SB::ObjectPtr & aWeaponObject)
{
	if (IsOnClient() && GetGameObject().GetComponent<PlayerControllerComponent>()->IsOurPlayer())
	{
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairRight")->SetActive(false);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairLeft")->SetActive(false);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairUp")->SetActive(false);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairDown")->SetActive(false);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairCenter")->SetActive(false);

		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairRight")->SetActive(true);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairLeft")->SetActive(true);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairUp")->SetActive(true);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairDown")->SetActive(true);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairCenter")->SetActive(true);
	}

	Super::SetAsActiveWeapon(aWeaponObject);
}

void AssaultRifleComponent::SetMuzzleFlashOffset(SB::ObjectPtr aWeaponObject, const SB::Vector3f & aOffset, const float aRotationAmount)
{
	myMuzzleFlash->SetParent(aWeaponObject);
	myMuzzleFlash->SetPosition(aOffset);

	SB::Quaternion muzzleFlashRotation;
	muzzleFlashRotation.RotateAroundLocalZ(myRandomizer.GetRandomValue(0.f, TwoPi));

	auto rot = aWeaponObject->GetRotation();
	rot.RotateAroundLocalY(aRotationAmount);
	myMuzzleFlash->SetRotation(rot * muzzleFlashRotation);
}

void AssaultRifleComponent::SpecificActivationPub()
{
	SpecificActivation(myWeaponObject);
}

void CreateSmokeTrail(SB::Scene & aScene, const SB::Vector3f & aFrom, const SB::Vector3f & aTo)
{
	if (SB::Engine::HasRenderer())
	{
		const int numSegments = 20;
		const float timeBetweenSegments = 1.f / static_cast<float>(numSegments);
		const float totalTime = timeBetweenSegments * static_cast<float>(numSegments);

		SB::ObjectPtr obj = aScene.CreateLocalGameObject("Rifle Trail");

		SB::GrowingArray<float> sizes;
		SB::GrowingArray<SB::Vector4f> colors;
		colors.Add(SB::Color::White);

		for (size_t i = 0; i < 1; ++i)
		{
			sizes.Add(10.f);
		}
		sizes.Add(0.f);

		SB::UnityStreakLogic * streakData = new SB::UnityStreakLogic(colors, sizes);
		auto && streaky = obj->CreateLocalComponent<SB::StreakComponent>();
		streaky->Setup(streakData, timeBetweenSegments, numSegments);

		streaky->AddTexture(SB::Engine::GetResourceManager().Get<SB::Texture>("Assets/Textures/ElectricFlakTextures/electric01.dds"));

		obj->CreateLocalComponent<SB::TimedRemovalComponent>()->NotifyRemove(totalTime);

		obj->Initialize();

		for (int i=-1; i<numSegments - 1; ++i)
		{
			float p = static_cast<float>(i) / static_cast<float>(numSegments - 1);
			const SB::Vector3f pos = aFrom + (aTo - aFrom) * p;
			obj->SetPosition(pos);
			streaky->EndUpdate(timeBetweenSegments);
		}
	}
}

void AssaultRifleComponent::FirePrimary()
{
	myMuzzleFlash->GetComponent<SB::ModelComponent>()->EnableAutomaticRender();
	SB::Quaternion muzzleFlashRotation;
	muzzleFlashRotation.RotateAroundLocalZ(myRandomizer.GetRandomValue(0.f, TwoPi));
	myMuzzleFlash->SetRotation(muzzleFlashRotation);
	myMuzzleTimer.Restart();

	GetGameObject().TraversingGetComponent<WeaponAnimationController>()->PlayAssaultRiflePrimary();


	const SB::Vector3f firePos = GetGameObject().GetComponent<PlayerControllerComponent>()->GetVisionObject()->GetWorldPosition();
	CreateAmmoShell(SB::Vector3f(0.6f, 0.6f, 1.3f), SB::Color::SunFlower);

	SB::Vector3f direction = GetShotDirection();

	SB::RaytraceInput rayInput;
	rayInput.startPosition = firePos;
	rayInput.direction = direction;
	rayInput.collisionFilter = 0 | CollisionFilter::eEnvironment | CollisionFilter::eEnemy | CollisionFilter::eRagDoll;

	SB::RaytraceOutput rayOutput;

	if (GetScene().GetPhysicsManager().Raytrace(rayInput, rayOutput) == true)
	{

		linestartpos = rayInput.startPosition;
		lineendpos = rayOutput.hitPosition;
		if (rayOutput.collisionIDOfObjectHit == static_cast<uint32_t>(CollisionFilter::eRagDoll))
		{
			rayOutput.gameObjectHit->GetComponent<SB::RagdollComponent>()->ApplyForce(rayOutput.hitPosition, direction * myWeaponData.PrimaryAttack.effectForce);
		}
		else
		if (rayOutput.gameObjectHit->IsMine())
		{
			DealDamage(*rayOutput.gameObjectHit);
		}
		CreateHitParticles(rayOutput.hitPosition, rayOutput.hitNormal);
	}


	myTimesFired++;

	if (IsOnClient() == true)
	{
		if (myDisplayTip == true && myTimesFired >= 180)
		{
			SmartTipsManager& tipsManager = static_cast<ClientGame&>(ClientGame::GetInstance()).GetSmartTipsManager();

			if (myTimesFired >= 310)
			{
				tipsManager.AddToDisplayQueue(eTipID::eAssaultSecondaryFire);
				myDisplayTip = false;
			}
			else
			{
				tipsManager.AddToDisplayQueue(eTipID::eInfiniteAmmo);
			}
		}
	}
}



void AssaultRifleComponent::SetupWeaponData()
{
	std::shared_ptr<SB::AssimpModel> weaponModel;

	if (Game::GetInstance().IsClient())
	{
		weaponModel = SB::Engine::GetResourceManager().Get<SB::AssimpModel>("Assets/Models/Weapons/AssaultRifle/assaultRifle.fbx");
		for (unsigned short i = 0; i < 5; i++)
		{
			CreateAmmoIcons("Assets/GUI/ingame/rocketLogoGui.dds", "ammo", "big", i + 1, 0.15f, IconTypes::eBig);
			CreateAmmoIcons("Assets/GUI/ingame/rocketLogoGuiEmpty.dds", "ammo", "bigGrey", i + 1, 0.15f, IconTypes::eBigGrey);
			CreateAmmoIcons("Assets/GUI/ingame/flakLogoGui.dds", "otherAmmo", "small", i + 1, 0.1f, IconTypes::eSmall);
			CreateAmmoIcons("Assets/GUI/ingame/flakLogoGuiEmpty.dds", "otherAmmo", "smallGrey", i + 1, 0.1f, IconTypes::eSmallGrey);
		}
	}
	
	myWeaponPos = { 22.f, -22.f, 35.f };

	myFirePos = myWeaponPos;
	myFirePos.z = 101.f;
	myFirePos.y += 3.f;

	myWeaponData.WeaponPosition = myWeaponPos;
	myWeaponData.WeaponModel = weaponModel;
	myWeaponData.WeaponColor = SB::Color::White;
	
	myWeaponData.PrimaryAttack.FirePosition = myFirePos;
	

	myWeaponData.SecondaryAttack.FirePosition = myFirePos;

	SB::Vector3f firePos = myWeaponData.PrimaryAttack.FirePosition - myWeaponPos;
	myMuzzleFlash->SetPosition(firePos);
}

void AssaultRifleComponent::SpecificActivation(SB::ObjectPtr & aWeaponObject)
{
	myMuzzleFlash->SetParent(aWeaponObject);	
}

void AssaultRifleComponent::FireSecondary()
{
	GetGameObject().TraversingGetComponent<WeaponAnimationController>()->PlayAssaultRifleSecondary();

	const SB::Vector3f forward = GetFireDirection();


	SB::Vector3f offset = myFirePos * GetPlayerOrieantion();
	const SB::Vector3f speed = forward * 4500.f;
	

	auto && obj = GetScene().CreateLocalGameObject("Rocket");
	obj->SetPosition(GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerCenterPosition() + offset);
	auto && triggershape = obj->CreateLocalComponent<SB::SphereColliderComponent>();
	auto && shape = obj->CreateLocalComponent<SB::SphereColliderComponent>();
	auto && actor = obj->CreateLocalComponent<SB::DynamicActorComponent>();

	obj->CreateLocalComponent<SB::PointLightComponent>()->Setup(SB::Color::Yellow, 1.f, 300.f);
	obj->CreateLocalComponent<SB::SpeakerComponent>();

	shape->SetRadius(5.f);
	shape->SetIsCollisionShape();

	triggershape->SetRadius(5.f);
	triggershape->SetIsTrigger();

	auto && model = obj->CreateLocalComponent<SB::ModelComponent>();
	model->SetModel("Assets/Models/Objects/Primitives/cube.fbx");
	model->SetMatrix(SB::Matrix44f::CreateScale(5.f, 5.f, 5.f));
	SB::Color c = SB::Vector4f(SB::Color::SunFlower);
	model->SetColor(c);

	obj->CreateLocalComponent<SB::TimedRemovalComponent>()->NotifyRemove(5.f);

	auto && projectileComp = obj->CreateLocalComponent<ExplosiveProjectileComponent>();
	projectileComp->SetDamage(100.f);
	projectileComp->SetRadius(350.f);
	projectileComp->SetShooterObject(&GetGameObject());
	projectileComp->SetExplosionSound("rocketExplosion1");
	projectileComp->SetTravelSound("rocketTravel");
	projectileComp->SetExplosionForce(myWeaponData.SecondaryAttack.effectForce);
	obj->Initialize();

	actor->SetVelocity(speed);
	actor->ActivateSweepChecking();
	actor->SetRotation(SB::Quaternion::LookRotation(forward));

	actor->SetFilterData(0 | CollisionFilter::ePlayerShot, 0 | CollisionFilter::eEnvironment | CollisionFilter::eEnemy | CollisionFilter::ePlayerShot);


	if (IsOnClient() == true && myDisplayTip == true)
	{
		SmartTipsManager& tipsManager = static_cast<ClientGame&>(ClientGame::GetInstance()).GetSmartTipsManager();
		tipsManager.AddToDisplayQueue(eTipID::eAssaultSecondaryFire);
		myDisplayTip = false;
	}
}

void AssaultRifleComponent::CreateHitParticles(const SB::Vector3f & aHitPosition, const SB::Vector3f & aHitNormal)
{
	if (myParticleCooldown.GetElapsedTime().InSeconds() > 1.f / 30.f)
	{
		SB::ObjectPtr particleObject = myObject->GetScene().CreateLocalGameObject("Hit Particles");
		SB::ComponentPtr<SB::ParticleComponent> particleComponent = particleObject->CreateLocalComponent<SB::ParticleComponent>();

		particleObject->CreateLocalComponent<SB::TimedRemovalComponent>()->NotifyRemove(0.1f);
		particleComponent->SetEmitter("AssaultHitParticles", true, false);
		SB::Vector3f up = SB::Vector3f::UnitX;
		if (aHitNormal.GetNormalized() == SB::Vector3f::UnitX)
		{
			up = SB::Vector3f::UnitY;
		}
		particleComponent->SetParticlesSpace(SB::Quaternion::LookRotation(aHitNormal, up).GenerateMatrix());
		particleObject->SetPosition(aHitPosition);
		particleObject->Initialize();
		myParticleCooldown.Restart();
	}
}

void AssaultRifleComponent::CreateAmmoShell(const SB::Vector3f & aAmmoScale, const SB::Color & aAmmoColor)
{
	SB::Vector3f position = myObject->GetWorldPosition();
	auto ammoShell = GetScene().CreateLocalObjectAndAddModel("Ammo Shell", "Assets/Models/Objects/Primitives/Cube.fbx", SB::Vector3f::Zero);
	ammoShell->SetScale(aAmmoScale);
	ammoShell->GetComponent<SB::ModelComponent>()->SetColor(aAmmoColor);
	SB::Vector3f pos = GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerCenterPosition() + ((myFirePos + SB::Vector3f(5.f, 5.f, -45.f)) * GetPlayerOrieantion());
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

void AssaultRifleComponent::DealDamage(SB::GameObject & aHitObject)
{
	if (aHitObject.GetComponentCount<EnemyHealthComponent>() > 0)
	{
		aHitObject.GetComponent<EnemyHealthComponent>()->TakeDamage(myWeaponData.PrimaryAttack.WeaponDamage, &GetGameObject());
	}
}

