#pragma once
#ifndef ENTITYHANDLER

#include <Windows.h>
#include <vector>

#include "EntityJobs.h"

struct Entity
{
	friend class EntityHandler;
	friend class D3D12Wrapper;

private:
	Entity(INT8 id)
	{
		entityID = id;
	}

	INT8 entityID = -1;

	INT8 transformID = -1;
	INT8 meshID = -1;
	INT8 textureID = -1;
	INT8 pipelineID = -1;

	bool render = false;
};

class EntityHandler
{
	friend class D3D12Wrapper;

private:
	//These vectors should not be accessed directly even though it is possible, do it through the get functions so that they are sent as references and can be handled efficiently

	std::vector<Entity*> entityVec;

	std::vector<TransformJob> transformJobs;
	std::vector<MeshJob> meshJobs;
	std::vector<TextureJob> textureJobs;
	std::vector<LightJob> lightJobs;
	std::vector<PipelineJob> pipelineJobs;

	//Get function even though the classes accessing it are friends so that it can be sent as a const reference for performance and security reasons
	const std::vector<Entity*>& GetEntityVector();

	//Purposely not const so that they can be emptied when the handlers are done with them

	std::vector<TransformJob>& GetTransformJobs();
	std::vector<MeshJob>& GetMeshJobs();
	std::vector<TextureJob>& GetTextureJobs();
	std::vector<LightJob>& GetLightJobs();
	std::vector<PipelineJob>& GetPipelineJobs();

public:
	EntityHandler();
	~EntityHandler();

	Entity* CreateEntity();

	void BindMesh(Entity* entity, std::string fileName);
	void BindTexture(Entity* entity, std::string fileName);
	void BindLight(Entity* entity, float lightColour[3], float lightRange);
	void BindPipeline(Entity* entity, std::string nameOfVS, std::string nameOfPS);

	void SetTransform(Entity* entity, float pos[3], float rot[3]); // This isn't really according to the entity component system but since it is a small project we do it anyways
	

};


#endif // !ENTITYHANDLER
