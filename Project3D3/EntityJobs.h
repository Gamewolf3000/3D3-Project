#pragma once
#ifndef ENTITYJOBS

#include <Windows.h>
#include <string>

// entityID represents the specific id of that thing, for example the entityId in transformJob is the transform id of a entity
// some name change might be good for clarity

struct TransformJob
{
private:
	TransformJob();
public:
	TransformJob(INT8 entID)
	{
		entityID = entID;
	}

	INT8 entityID = -1;
	float position[3] = { 0.0f, 0.0f, 0.0f };
	float rotation[3] = { 0.0f, 0.0f, 0.0f };

};

struct MeshJob
{
private:
	MeshJob();
public:
	MeshJob(INT8 entID, std::string nameOfFile)
	{
		entityID = entID;
		fileName = nameOfFile;
	}

	INT8 entityID = -1;
	std::string fileName = "";
};

struct TextureJob
{
private:
	TextureJob();
public:
	TextureJob(INT8 entID, std::string nameOfFile)
	{
		entityID = entID;
		fileName = nameOfFile;
	}

	INT8 entityID = -1;
	std::string fileName = "";
};

struct LightJob
{
private:
	LightJob();
public:
	LightJob(INT8 entID, float lightColour[3], float lightRange)
	{
		entityID = entID;

		colour[0] = lightColour[0];
		colour[1] = lightColour[1];
		colour[2] = lightColour[2];

		range = lightRange;
	}

	INT8 entityID = -1;
	float colour[3] = { 1.0f, 1.0f, 1.0f };
	float range = -1;
};

struct PipelineJob
{
private:
	PipelineJob();
public:
	PipelineJob(INT8 entID, std::string nameOfVS, std::string nameOfPS)
	{
		entityID = entID;

		vsName = nameOfVS;
		psName = nameOfPS;
	}

	INT8 entityID = -1;
	std::string vsName = "";
	std::string psName = "";
};



#endif // !ENTITYJOBS
