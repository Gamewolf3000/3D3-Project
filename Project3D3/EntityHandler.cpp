#include "EntityHandler.h"
#include "ConstantBuffer.h"
Entity * EntityHandler::CreateEntity()
{
	entityVec.push_back(new Entity(entityVec.size()));

	// Always create a transform job, there is no point in an entity without a transformation anyway
	// and by doing it at this point we reduce the work needed to be done on the outside
	transformJobs.push_back(entityVec[entityVec.size() - 1]->entityID);
	float data[6] = { 0 };

	
	return entityVec[entityVec.size() - 1];
}

void EntityHandler::BindMesh(Entity * entity, std::string fileName)
{
	meshJobs.push_back(MeshJob(entity->entityID, fileName));
	entity->render = true;
}

void EntityHandler::BindTexture(Entity * entity, std::string fileName)
{
	textureJobs.push_back(TextureJob(entity->entityID, fileName));
}

void EntityHandler::BindLight(Entity * entity, float lightColour[3], float lightRange)
{
	lightJobs.push_back(LightJob(entity->entityID, lightColour, lightRange));
}

void EntityHandler::BindPipeline(Entity * entity, std::string nameOfVS, std::string nameOfPS)
{
	pipelineJobs.push_back(PipelineJob(entity->entityID, nameOfVS, nameOfPS));
}

void EntityHandler::SetTransform(Entity * entity, float pos[3], float rot[3], float scale)
{
	TransformJob temp(entity->entityID);

	for (int i = 0; i < 3; i++)
	{
		temp.position[i] = pos[i];
		temp.rotation[i] = rot[i];
	}

	temp.scale = scale;

	transformJobs.push_back(temp);

}

const std::vector<Entity*>& EntityHandler::GetEntityVector()
{
	return entityVec;
}

std::vector<TransformJob>& EntityHandler::GetTransformJobs()
{
	return transformJobs;
}

std::vector<MeshJob>& EntityHandler::GetMeshJobs()
{
	return meshJobs;
}

std::vector<TextureJob>& EntityHandler::GetTextureJobs()
{
	return textureJobs;
}

std::vector<LightJob>& EntityHandler::GetLightJobs()
{
	return lightJobs;
}

std::vector<PipelineJob>& EntityHandler::GetPipelineJobs()
{
	return pipelineJobs;
}

EntityHandler::EntityHandler()
{
}

EntityHandler::~EntityHandler()
{
	for (auto i : entityVec)
	{
		delete i;
	}
}
