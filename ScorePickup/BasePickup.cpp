#include "stdafx.h"
#include "BasePickup.h"



BasePickup::BasePickup(const std::string & aModelPath, const std::string aPickupName) : myModelPath(aModelPath), myPickupName(aPickupName)
{
	
}

BasePickup::~BasePickup()
{
}

const std::string & BasePickup::GetModelPath() const
{
	return myModelPath;
}

const std::string & BasePickup::GetPickupName() const
{
	return myPickupName;
}

