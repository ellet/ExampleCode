#include "stdafx.h"
#include "FlakCannonComponent.h"
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
#include "Game/Game.h"
#include "ExplosiveProjectileComponent.h"
#include "ClusterExplosiveProjectileComponent.h"
#include "Utilities\Randomizer\DistributionShapes\RandomDistributionSphere.h"
#include "Game/Player/WeaponAnimation/WeaponAnimationController.h"
#include "Engine\Texture\Texture.h"
#include "Engine\Feedback\VisualEffect\VisualEffectComponent.h"
#include "Engine\Lighting\PointLightComponent.h"
#include <Engine\Resources\ResourceManager.h>

FlakCannonComponent::FlakCannonComponent()
{
	myDistribution = std::make_unique<SB::RandomDistributionSphere>(SB::Vector3f::Zero, 7.f);
	myTimesFired = 0;
	myDisplayTip = true;
}


FlakCannonComponent::~FlakCannonComponent()
{
}

void FlakCannonComponent::LoadData(SB::DataNode aProperties)
{
	Super::LoadData(aProperties);

	myShrapnelAmount = 3;
	myStartExplosionSize = 175.f;
	myStartBouncesAmount = 2;

	if (aProperties.HasMember("StartExplosionSize") == true)
	{
		myStartExplosionSize = aProperties["StartExplosionSize"].GetFloat();
	}
	
	if (aProperties.HasMember("AmountOfBounces") == true)
	{
		myStartBouncesAmount = static_cast<uint8_t>(aProperties["AmountOfBounces"].GetUnsignedShort());
	}

	if (aProperties.HasMember("ShellsWithEachExplosion") == true)
	{
		myShrapnelAmount = aProperties["ShellsWithEachExplosion"].GetUnsignedShort();
	}
}

void FlakCannonComponent::SetAsActiveWeapon(SB::ObjectPtr & aWeaponObject)
{
	if (IsOnClient() && GetGameObject().GetComponent<PlayerControllerComponent>()->IsOurPlayer())
	{
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairRight")->SetActive(true);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairLeft")->SetActive(true);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairUp")->SetActive(true);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairDown")->SetActive(true);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("flakCannonCrosshairCenter")->SetActive(true);

		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairRight")->SetActive(false);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairLeft")->SetActive(false);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairUp")->SetActive(false);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairDown")->SetActive(false);
		GetScene().GetGUI()->GetGUIElement<SB::SpriteGUIElement>("assaultRifleCrosshairCenter")->SetActive(false);
	}
	Super::SetAsActiveWeapon(aWeaponObject);
}

void FlakCannonComponent::SetupWeaponData()
{
	if (Game::GetInstance().IsClient())
	{
		std::shared_ptr<SB::AssimpModel> weaponModel = SB::Engine::GetResourceManager().Get<SB::AssimpModel>("Assets/Models/Weapons/FlakCannon/flakCannon.fbx");

		myWeaponPos = { 30.f, -30.f, 0.f };
		weaponModel->MakeReady(false);
		const float sizeInZ = weaponModel->GetBoundingBox().GetSize().z;

		myFirePos = myWeaponPos;
		myFirePos.z += (sizeInZ / 2.f) + 37.f;
		myFirePos.y += 10.5f;
	
		myWeaponData.WeaponPosition = myWeaponPos;
		myWeaponData.WeaponModel = weaponModel;
		myWeaponData.WeaponColor = SB::Color::White;//SB::Color::Carrot;

		myWeaponData.PrimaryAttack.FirePosition = myFirePos;
		myWeaponData.SecondaryAttack.FirePosition = myFirePos;

		for (unsigned short i = 0; i < 5; i++)
		{
			CreateAmmoIcons("Assets/GUI/ingame/flakLogoGui.dds", "ammo", "big", i + 1, 0.15f, IconTypes::eBig);
			CreateAmmoIcons("Assets/GUI/ingame/flakLogoGuiEmpty.dds", "ammo", "bigGrey", i + 1, 0.15f, IconTypes::eBigGrey);
			CreateAmmoIcons("Assets/GUI/ingame/rocketLogoGui.dds", "otherAmmo", "small", i + 1, 0.1f, IconTypes::eSmall);
			CreateAmmoIcons("Assets/GUI/ingame/rocketLogoGuiEmpty.dds", "otherAmmo", "smallGrey", i + 1, 0.1f, IconTypes::eSmallGrey);
		}
	}
}

void FlakCannonComponent::Update(const SB::Time & aDeltaTime)
{
	std::string baseGuiName = "flakCannonCrosshair";
	RecoilGuiElements(baseGuiName);
	Super::Update(aDeltaTime);
}

void FlakCannonComponent::FirePrimary()
{
	GetGameObject().TraversingGetComponent<WeaponAnimationController>()->PlayFlakCannonPrimary();

	uint16_t shootsToShot = 0;

	const float shotsPerSecond = 50.f;
	const float timeBetweenShot = 1.0f / shotsPerSecond;

	shootsToShot = 15;
	while (shootsToShot-- > 0)
	{
		float recoilMultiplier = 1.f + myCurrentRecoil / ourMaxRecoil;

		const float rX = RandomGenerator.GetRandomValue(-0.025f, 0.025f) * recoilMultiplier;
		const float rY = RandomGenerator.GetRandomValue(-0.045f, 0.045f) * recoilMultiplier;
		const float rZ = RandomGenerator.GetRandomValue(0.975f, 1.0f) * recoilMultiplier;



		SB::Vector3f direction = SB::Vector3f(rX, rY, rZ).GetNormalized();
		direction = direction * GetPlayerOrieantion();
		
		SB::Vector3f randPos = myDistribution->GetRandomPoint();
		randPos.z = 0.f;
		randPos = randPos * GetPlayerOrieantion();

		SB::Vector3f offset = myFirePos * GetPlayerOrieantion();
		const SB::Vector3f speed = direction * 4000;//5500.f;

		auto && obj = GetScene().CreateLocalGameObject("Shot");
		obj->SetPosition(GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerCenterPosition() + offset + randPos);
		auto && triggershape = obj->CreateLocalComponent<SB::SphereColliderComponent>();
		auto && shape = obj->CreateLocalComponent<SB::SphereColliderComponent>();
		auto && actor = obj->CreateLocalComponent<SB::DynamicActorComponent>();

		if (SB::Engine::HasRenderer())
		{
			SB::GrowingArray<float> sizes;
			SB::GrowingArray<SB::Vector4f> colors;

			sizes.Add(7.f);
			colors.Add(SB::Color::White);
			colors.Add(SB::Color::Cyan);
			colors.Add(SB::Color::Black);

			SB::UnityStreakLogic * streakData = new SB::UnityStreakLogic(colors, sizes);
			auto streaky = obj->CreateLocalComponent<SB::StreakComponent>();
			streaky->Setup(streakData, 0.03f, 7);
			
			streaky->AddTexture(SB::Engine::GetInstance().GetResourceManager().Get<SB::Texture>("Assets/Textures/ElectricFlakTextures/electric01.dds"));

			auto spherey = obj->CreateLocalComponent<SB::VisualEffectComponent>();
			spherey->Setup(false, SB::eVisualShape::eSphere);
			spherey->SetColor(SB::Color::White);
			spherey->SetLocalScale(SB::Vector3f::One * 3.f);

			auto lighty = obj->CreateLocalComponent<SB::PointLightComponent>();
			lighty->SetColor(SB::Color::White);
			lighty->Setup(SB::Color::Cyan, 0.2f, 100.f);
		}

		shape->SetRadius(5.f);
		shape->SetIsCollisionShape();

		triggershape->SetRadius(5.f);
		triggershape->SetIsTrigger();

		obj->CreateLocalComponent<SB::TimedRemovalComponent>()->NotifyRemove(5.f);
		obj->CreateLocalComponent<ShellComponent>()->SetShooterObject(&GetGameObject());
		obj->GetComponent<ShellComponent>()->SetDamage(myWeaponData.PrimaryAttack.WeaponDamage);

		obj->Initialize();

		actor->ActivateSweepChecking();
		actor->SetVelocity(speed);
		actor->SetRotation(SB::Quaternion::LookRotation(direction));
		actor->SetMass(25.f);

		actor->SetFilterData(0 | CollisionFilter::ePlayerShot, 0 | CollisionFilter::eEnvironment | CollisionFilter::eEnemy | CollisionFilter::eRagDoll);
	}

	
	myTimesFired++;

	if (IsOnClient() == true)
	{
		if (myDisplayTip == true && myTimesFired >= 20)
		{
			SmartTipsManager& tipsManager = static_cast<ClientGame&>(ClientGame::GetInstance()).GetSmartTipsManager();
			tipsManager.AddToDisplayQueue(eTipID::eFlakSecondaryFire);
			myDisplayTip = false;
		}
	}
}

void FlakCannonComponent::FireSecondary()
{
	GetGameObject().TraversingGetComponent<WeaponAnimationController>()->PlayFlakCannonSecondary();

	SB::Quaternion playerOrientation = GetPlayerOrieantion();
	SB::Vector3f offset = myFirePos * playerOrientation;

	SB::Vector3f aimDirection(0.f, 0.23f, .77f);
	aimDirection = playerOrientation.GetForward();
	const SB::Vector3f speed = aimDirection * 2235.f;

	auto && obj = GetScene().CreateLocalGameObject("Shot");
	obj->SetPosition(GetGameObject().GetComponent<PlayerControllerComponent>()->GetPlayerCenterPosition() + offset);
	auto && triggershape = obj->CreateLocalComponent<SB::SphereColliderComponent>();
	auto && shape = obj->CreateLocalComponent<SB::SphereColliderComponent>();
	auto && actor = obj->CreateLocalComponent<SB::DynamicActorComponent>();
	auto && sound = obj->CreateLocalComponent<SB::SpeakerComponent>();

	shape->SetRadius(5.f);
	shape->SetIsCollisionShape();

	triggershape->SetRadius(5.f);
	triggershape->SetIsTrigger();

	if (SB::Engine::HasRenderer())
	{
		SB::GrowingArray<float> sizes;
		SB::GrowingArray<SB::Vector4f> colors;

		sizes.Add(15.f);
		colors.Add(SB::Color::White);
		colors.Add(SB::Color::Cyan);
		colors.Add(SB::Color::Black);

		SB::UnityStreakLogic * streakData = new SB::UnityStreakLogic(colors, sizes);
		auto streaky = obj->CreateLocalComponent<SB::StreakComponent>();
		streaky->Setup(streakData, 0.02f, 12);

		streaky->AddTexture(SB::Engine::GetInstance().GetResourceManager().Get<SB::Texture>("Assets/Textures/ElectricFlakTextures/electric01.dds"));

		auto spherey = obj->CreateLocalComponent<SB::VisualEffectComponent>();
		spherey->Setup(false, SB::eVisualShape::eSphere);
		spherey->SetColor(SB::Color::White);
		spherey->SetLocalScale(SB::Vector3f::One * 8.f);

		auto lighty = obj->CreateLocalComponent<SB::PointLightComponent>();
		lighty->SetColor(SB::Color::White);
		lighty->Setup(SB::Color::Cyan, 0.7f, 100.f);
	}


	auto model = obj->CreateLocalComponent<SB::ModelComponent>();
	model->SetModel("Assets/Models/Objects/Primitives/cube.fbx");
	model->SetMatrix(SB::Matrix44f::CreateScale(5.f, 5.f, 5.f));
	SB::Color c = SB::Vector4f(SB::Color::SunFlower);
	model->SetColor(c);

	obj->CreateLocalComponent<SB::TimedRemovalComponent>()->NotifyRemove(5.f);
	auto && projectileComp = obj->CreateLocalComponent<ClusterExplosiveProjectileComponent>();
	projectileComp->SetPlayTravelSound(true);
	projectileComp->SetBounceLife(myStartBouncesAmount);
	projectileComp->SetRadius(myStartExplosionSize);
	projectileComp->SetShellAmount(myShrapnelAmount);
	projectileComp->SetShooterObject(&GetGameObject());
	projectileComp->SetDamage(myWeaponData.SecondaryAttack.WeaponDamage);
	projectileComp->SetForceAmount(myWeaponData.SecondaryAttack.effectForce);
	projectileComp->SetStartSpeed(450.f);
	
	//projectileComp->SetRadius(350.f);

	obj->Initialize();

	actor->EnableGravity();
	actor->ActivateSweepChecking();
	actor->SetVelocity(speed);
	actor->SetMass(1.f);
	actor->SetRotation(SB::Quaternion::LookRotation(aimDirection));

	actor->SetFilterData(0 | CollisionFilter::ePlayerShot, 0 | CollisionFilter::eEnvironment | CollisionFilter::eEnemy);


	if (IsOnClient() == true && myDisplayTip == true)
	{
		SmartTipsManager& tipsManager = static_cast<ClientGame&>(ClientGame::GetInstance()).GetSmartTipsManager();
		tipsManager.AddToDisplayQueue(eTipID::eFlakSecondaryFire);
		myDisplayTip = false;
	}
}

void FlakCannonComponent::SpecificActivation(SB::ObjectPtr & aWeaponObject)
{
	
}


