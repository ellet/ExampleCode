#pragma once

namespace SB
{
	struct OnTriggerEnterEvent;
	struct OnCollisionEvent;
}



class ClusterExplosiveProjectileComponent : public SB::BaseComponent, public SB::EventSubscriber<SB::OnTriggerEnterEvent>, public SB::EventSubscriber<SB::OnCollisionEvent>
{
public:
	ClusterExplosiveProjectileComponent();
	~ClusterExplosiveProjectileComponent();
	void Initialize() override;

	void SetPlayTravelSound(bool aValue);
	void SetDamage(const float aDamage);

	void SetForceAmount(const float aForce);

	void SetStartSpeed(const float aSpeed);

	void SetBounceLife(const uint8_t aBouneLifeAmount);
	void SetRadius(const float aRadius);

	void SetShellAmount(uint16_t aShellAmount) { myShellAmount = aShellAmount; }
	void SetShooterObject(const SB::GameObject * aShooterObject);
private:

	void Explode(const SB::Vector3f & aUpvector);
	void ReallyExplode();

	void RecieveEvent(const SB::OnTriggerEnterEvent& aEvent) override;
	void RecieveEvent(const SB::OnCollisionEvent& aEvent) override;

	SB::Vector3f myDirectionWithSpeed;
	float myDamage;
	float myRadius;
	float myForce;
	float myStartSpeed;
	uint16_t myShellAmount;
	const SB::GameObject * myShooterObject;
	uint8_t myLifeCounter;
	SB::Stopwatch myActivationTimer;

	bool myPlayTravelSound;
};

