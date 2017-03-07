#pragma once

#include "JEXMath.h"

#include <map>


class LightHandler
{
public:
	struct PointLight
	{
		Float4D position;
		Float4D lightColour;
		Float4D rangeInXRestPadding;
	};

private:
	typedef signed char INT8;

	
	struct PointLightEntity
	{
		bool active;
		PointLight* lightPtr;
	};

	std::map<INT8, PointLightEntity*> pointLightMap;

	INT8 maximumNumberOFLights = 0;
	INT8 addedLights = 0;
	void* returnData;
	bool changeMade;

public:
	LightHandler(INT8 maximumNumberOfLights = 10);
	~LightHandler();
	void AddLight(INT8 entityID, float colour[4], float position[4], float range);
	void ToggleLightActive(INT8 entityID, bool active);
	void* GatherLightJobs();

	

};