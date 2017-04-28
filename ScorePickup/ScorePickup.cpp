#include "stdafx.h"
#include "ScorePickup.h"
#include "Game\Score\ScorePickupedMessage.h"
#include "..\Engine\Engine\GameObject\GameObject.h"


ScorePickup::ScorePickup() : BasePickup("Assets/Models/Objects/Pickups/score.fbx", "ScorePickup")
{
}


ScorePickup::~ScorePickup()
{
}

void ScorePickup::OnPickup(SB::GameObject & aPlayerObject)
{
	SB::ObjectPtr emitter = aPlayerObject.GetScene().CreateGameObject("Pickup Particles");
	SB::ComponentPtr<SB::ParticleComponent> particleComponent = emitter->CreateComponent<SB::ParticleComponent>();

	emitter->SetPosition(aPlayerObject.GetWorldPosition());
	particleComponent->SetEmitter("ScorePickup", true, false, SB::Vector3f::Zero);
	emitter->Initialize();

	SB::PMPostMaster::Post(ScorePickedUpMessage());
}
