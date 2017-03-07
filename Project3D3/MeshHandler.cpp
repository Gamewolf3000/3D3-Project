#include "MeshHandler.h"

RenderData MeshHandler::GetMeshAsRawData(INT8 meshID)
{
	//MeshData temp = meshes[meshID];
	//int sizeOfVertex = (sizeof(Float3D) + sizeof(Float2D) + sizeof(Float3D));
	//int offsetPosition = 0;
	//int offsetUV = sizeof(Float3D);
	//int offsetNormal = sizeof(Float3D) + sizeof(Float2D);

	//void* returnData = new char[temp.nrOfIndices * sizeOfVertex];

	// This might be unnecessary, we might be able to just use the vertexbuffer ptr interpteted as a void*, needs testing, cause this is probably very expensive to do multiple times
	// every frame
	//for (int i = 0; i < temp.nrOfIndices; i++)
	//{
	//	memcpy(((char*)returnData + i * sizeOfVertex + offsetPosition), &temp.vertexBuffer[temp.indexBuffer[i]].position, sizeof(Float3D));
	//	memcpy(((char*)returnData + i * sizeOfVertex + offsetUV), &temp.vertexBuffer[temp.indexBuffer[i]].uv, sizeof(Float2D));
	//	memcpy(((char*)returnData + i * sizeOfVertex + offsetNormal), &temp.vertexBuffer[temp.indexBuffer[i]].normal, sizeof(Float3D));
	//}

	MeshData temp = meshes[meshID];
	int sizeOfVertex = (sizeof(Float3D) + sizeof(Float2D) + sizeof(Float3D));

	RenderData returnData;

	returnData.data = temp.vertexBuffer;
	returnData.size = temp.nrOfIndices * sizeOfVertex;
	returnData.nrOfTriangles = temp.nrOfIndices / 3;
	returnData.nrOfIndices = temp.nrOfIndices;
	returnData.indexBuffer = temp.indexBuffer;

	return returnData;
}

MeshHandler::MeshHandler(ID3D12Device * dev)
{
	objLoader = new OBJLoader(L"");
}

MeshHandler::~MeshHandler()
{
	delete objLoader;

	for (auto i : meshes)
	{
		delete i.vertexBuffer;
		delete i.indexBuffer;
	}
}

INT8 MeshHandler::LoadMesh(std::string fileName)
{
	meshes.push_back(objLoader->LoadOBJFile(fileName));

	return meshes.size() - 1;
}
