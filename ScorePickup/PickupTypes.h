#pragma once

enum class PickupTypes
{
	eHealth,
	eScore,
	enumLength
};

inline PickupTypes GetEnum(const std::string & aEnumName)
{
	if (aEnumName == "HealthPickup")
	{
		return PickupTypes::eHealth;
	}
	else if (aEnumName == "ScorePickup")
	{
		return PickupTypes::eScore;
	}
	else
	{
		Error((aEnumName + ": is not a valid pickup name").c_str());
	}
}
