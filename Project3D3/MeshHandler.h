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

#define VERTEXSIZE (sizeof(Float3D) + sizeof(Float2D) + sizeof(Float3D) + sizeof(Float3D) + sizeof(Float3D))
#define HEAP_SIZE (VERTEXSIZE * 3000)

struct RenderData
{
	void* data = nullptr;
	UINT size = 0;
	UINT nrOfTriangles = 0;

	UINT nrOfIndices = 0;
	UINT* indexBuffer = 0;

	D3D12_VERTEX_BUFFER_VIEW vBufferView;
	D3D12_INDEX_BUFFER_VIEW iBufferView;

};

class MeshHandler
{
private:
	ID3D12Device* device = nullptr;
	OBJLoader* objLoader = nullptr;

	ID3D12Resource* vertexBufferUploader = nullptr;
	ID3D12Resource* indexBufferUploader = nullptr;

	std::vector<MeshData> meshes;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vBufferViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW> iBufferViews;

	void CreateUploadHeap();

public:
	MeshHandler(ID3D12Device* dev);
	~MeshHandler();

	INT8 LoadMesh(std::string fileName);
	RenderData GetMeshAsRawData(INT8 meshID);

	ID3D12Resource* GetVertexUploadBuffer();
	ID3D12Resource* GetIndexUploadBuffer();

};

#endif
