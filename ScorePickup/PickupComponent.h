#pragma once
#include "Engine\Component\BaseComponent.h"
#include "Game\Player\PlayerTriggerCallback.h"
#include "..\Engine\Engine\Physics\Events\OnCollisionEvent.h"

namespace SB
{
	struct OnTriggerEnterEvent;
}



class BasePickup;
struct RoundEndMessage;

class PickupComponent : public SB::BaseComponent, public PlayerTriggerCallback, public SB::EventSubscriber<SB::OnTriggerEnterEvent>, 
	public SB::EventSubscriber<SB::OnCollisionEvent>, public SB::PMSubscriber<RoundEndMessage>
{
public:
	PickupComponent();
	~PickupComponent();


	void PreInitialize() override;
	virtual void Initialize() override;
	virtual void PostInitialize() override;

	void SetPickup(std::unique_ptr<BasePickup> & aPickup);

	void EndUpdate(const SB::Time& aDeltaTime) override;
	virtual void Update(const SB::Time & aDeltaTime) override;

	virtual void RecieveEvent(const SB::OnTriggerEnterEvent & aEvent) override;
	virtual void RecieveEvent(const SB::OnCollisionEvent & aEvent) override;

	virtual SB::ReceiveResult Receive(const RoundEndMessage & aMessage) override;
	
	virtual void OnPlayerTrigger(const SB::GameObject & aPlayerObject) override;

	virtual void OnRemoved() override;


private:
	void SpawnFollowTriggerObject();

	float myStayAliveTime;

	bool myEndOfRound;
	bool myAddedTriggerObject;
	std::unique_ptr<BasePickup> myPickupEffect;
	const SB::GameObject * myTargetPlayer;
	SB::GameObject * myStreakObject;
	SB::ObjectPtr myPlayerTriggerObject;
};
