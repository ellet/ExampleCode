#pragma once

namespace SB
{
	struct OnTriggerEnterEvent;
	struct OnCollisionEvent;
}

class ShellComponent : public SB::BaseComponent, public SB::EventSubscriber<SB::OnTriggerEnterEvent>, public SB::EventSubscriber<SB::OnCollisionEvent>
{
public:
	ShellComponent();
	~ShellComponent();

	void Initialize() override;

	void SetDamage(const float aDamageValue);

	void Update(const SB::Time& aDeltaTime) override;
	void SetShooterObject(const SB::GameObject * aShooterObject);

private:
	void RecieveEvent(const SB::OnCollisionEvent & aEvent) override;
	void RecieveEvent(const SB::OnTriggerEnterEvent& aEvent) override;
	const SB::GameObject * myShooterObject;

	SB::Stopwatch myCollidedTimer;

	float myDamage;
	uint8_t myBounceCounter;
	bool myCollidedThisFrame;
};

