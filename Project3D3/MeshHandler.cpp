#include "MeshHandler.h"

RenderData MeshHandler::GetMeshAsRawData(INT8 meshID)
{

	MeshData temp = meshes[meshID];
	int sizeOfVertex = (sizeof(Float3D) + sizeof(Float2D) + sizeof(Float3D) + sizeof(Float3D) + sizeof(Float3D));

	RenderData returnData;

	returnData.data = temp.vertexBuffer;
	returnData.size = temp.nrOfIndices * sizeOfVertex;
	returnData.nrOfTriangles = temp.nrOfIndices / 3;
	returnData.nrOfIndices = temp.nrOfIndices;
	returnData.indexBuffer = temp.indexBuffer;
	returnData.vBufferView = vBufferViews[meshID];
	returnData.iBufferView = iBufferViews[meshID];

	return returnData;
}

ID3D12Resource * MeshHandler::GetVertexUploadBuffer()
{
	return vertexBufferUploader;
}

ID3D12Resource * MeshHandler::GetIndexUploadBuffer()
{
	return indexBufferUploader;
}

void MeshHandler::CreateUploadHeap()
{
	//Note: using upload heaps to transfer static data like vert buffers is not 
	//recommended. Every time the GPU needs it, the upload heap will be marshalled 
	//over. Please read up on Default Heap usage. An upload heap is used here for 
	//code simplicity
	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = HEAP_SIZE;
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Creates both a resource and an implicit heap, such that the heap is big enough
	//to contain the entire resource and the resource is mapped to the heap. 
	device->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferUploader));

	vertexBufferUploader->SetName(L"vb heap");

	rd.Width = sizeof(UINT) * 100000;

	device->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUploader));
}

MeshHandler::MeshHandler(ID3D12Device* dev)
{
	objLoader = new OBJLoader(L"");
	device = dev;

	CreateUploadHeap();
}

MeshHandler::~MeshHandler()
{
	delete objLoader;

	for (auto i : meshes)
	{
		delete i.vertexBuffer;
		delete i.indexBuffer;
	}

	vertexBufferUploader->Release();
	indexBufferUploader->Release();
}

INT8 MeshHandler::LoadMesh(std::string fileName)
{

	for (int i = 0; i < meshes.size(); i++)
	{
		if (fileName == meshes[i].identifier)
		{
			return i;
		}
	}

	int sizeOfVertex = (sizeof(Float3D) + sizeof(Float2D) + sizeof(Float3D) + sizeof(Float3D) + sizeof(Float3D));

	meshes.push_back(objLoader->LoadOBJFile(fileName));
	meshes[meshes.size() - 1].identifier = fileName;

	// Create view here
	D3D12_VERTEX_BUFFER_VIEW vBufferView;
	vBufferView.BufferLocation = vertexBufferUploader->GetGPUVirtualAddress();
	vBufferView.SizeInBytes = meshes[meshes.size() - 1].nrOfIndices * sizeOfVertex;
	vBufferView.StrideInBytes = sizeOfVertex;

	vBufferViews.push_back(vBufferView);

	D3D12_INDEX_BUFFER_VIEW iBufferView;
	iBufferView.BufferLocation = indexBufferUploader->GetGPUVirtualAddress();
	iBufferView.SizeInBytes = sizeof(UINT) * meshes[meshes.size() - 1].nrOfIndices;
	iBufferView.Format = DXGI_FORMAT_R32_UINT;

	iBufferViews.push_back(iBufferView);

	return meshes.size() - 1;
}
