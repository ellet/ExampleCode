#pragma once
#include "PickupTypes.h"

namespace SB
{
	class GameObject;
}

class BasePickup
{
public:
	virtual ~BasePickup();

	virtual void OnPickup(SB::GameObject & aPlayerObject) = 0;

	const std::string & GetModelPath() const;
	const std::string & GetPickupName() const;

protected:
	BasePickup(const std::string & aModelPath, const std::string aPickupName);

	std::string myModelPath;
	std::string myPickupName;
};

