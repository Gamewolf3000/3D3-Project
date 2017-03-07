#ifndef MESH_HANDLER_H
#define MESH_HANDLER_H
#pragma once
#include "JEXMath.h"

#include <d3d12.h>
#include <dxgi1_5.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include <string>
#include <vector>

#include "CD3DX12Helper.h"
#include "OBJLoader.h"

struct RenderData
{
	void* data = nullptr;
	UINT size = 0;
	UINT nrOfTriangles = 0;

	UINT nrOfIndices = 0;
	UINT* indexBuffer = 0;

};

class MeshHandler
{
private:
	ID3D12Device* device;
	OBJLoader* objLoader;

	std::vector<MeshData> meshes;

	RenderData GetMeshAsRawData(INT8 meshID);


public:
	MeshHandler(ID3D12Device* dev);
	~MeshHandler();

	INT8 LoadMesh(std::string fileName);


};

#endif
