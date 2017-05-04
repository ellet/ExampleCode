#pragma once

namespace SB
{
	struct OnTriggerEnterEvent;
	struct OnCollisionEvent;
}

class ExplosiveProjectileComponent : public SB::BaseComponent, public SB::EventSubscriber<SB::OnTriggerEnterEvent>, public SB::EventSubscriber<SB::OnCollisionEvent>
{
public:
	ExplosiveProjectileComponent();
	~ExplosiveProjectileComponent();

	void Initialize() override;

	void Update(const SB::Time& aDeltaTime) override;


	void SetRadius(const float aRadius) { myRadius = aRadius; }
	void SetDamage(const float aDamageValue) { myDamage = aDamageValue; }
	void SetShooterObject(const SB::GameObject * aShooterObject);
	void SetExplosionForce(const float aExplosionForce);

	virtual void PreInitialize() override;

	void SetTravelSound(const std::string &aSound);
	void SetExplosionSound(const std::string &aSound);
private:

	void Explode();

	void RecieveEvent(const SB::OnTriggerEnterEvent& aEvent) override;
	void RecieveEvent(const SB::OnCollisionEvent& aEvent) override;

	std::string myExplosionSound;
	std::string myTravelSound;

	float myDamage;
	float myRadius;
	float myExplosionForce;
	const SB::GameObject * myShooterObject;
	
	
};

