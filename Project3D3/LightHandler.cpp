#include "LightHandler.h"

LightHandler::LightHandler(INT8 maximumNumberOfLights)
{
	this->maximumNumberOFLights = maximumNumberOfLights;
	this->addedLights = 0;
	this->changeMade = true;
	this->returnData = nullptr;
}

LightHandler::~LightHandler()
{
	if (this->returnData)
		delete returnData;
}

void LightHandler::AddLight(INT8 entityID, float colour[4], float position[4], float range)
{
	if (maximumNumberOFLights != addedLights)
	{
		PointLightEntity *newLight = new PointLightEntity;
		newLight->active = true;
		newLight->lightPtr = new PointLight;
		newLight->lightPtr->lightColour = Float4D(colour);
		newLight->lightPtr->position = Float4D(position);
		newLight->lightPtr->rangeInXRestPadding = Float4D(range, 0.0f, 0.0f, 0.0f);
		addedLights++;
		pointLightMap[entityID] = newLight;
		changeMade = true;
	}
}

void LightHandler::ToggleLightActive(INT8 entityID, bool active)
{
	changeMade = true;
	pointLightMap[entityID]->active = active;
}

void * LightHandler::GatherLightJobs()
{
	if (changeMade)
	{
		if (returnData)
			delete returnData;
		size_t activeLights = 0;
		for (auto &data : pointLightMap)
		{
			if (data.second->active)
				activeLights++;
		}
		size_t size = sizeof(PointLight);
		returnData = new char[activeLights * size];
		size_t offset = 0;
		for (auto &data : pointLightMap)
		{
			if (data.second->active)
			{
				memcpy((char*)(returnData)+offset, data.second->lightPtr, size);
				offset += size;
			}
		}
		changeMade = false;
	}


	return returnData;
}