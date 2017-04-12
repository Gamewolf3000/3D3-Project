#include "ConstantBuffer.h"

void ConstantBufferHandler::CreateConstantBuffer(INT8 ID, void * data, ConstantBufferType bufferType)
{
	ConstantBuffer* buffer = new ConstantBuffer;
	size_t dataSize = constantBufferSizes[bufferType];
	buffer->rawData = new char[dataSize];
	memcpy(buffer->rawData, data, dataSize);
	constantBufferSizes[bufferType];
	buffer->dataSize = dataSize;
	buffer->type = bufferType;

	if (cpu_MappedPtrs.find(bufferType) == cpu_MappedPtrs.end())
	{
		/*Taking for granted that the buffers are created during initialization, NOT during rendering*/
		CreateHeap(bufferType);
	}

	bufferVector[ID][bufferType] = (buffer);

}

void ConstantBufferHandler::CreateHeap(ConstantBufferType bufferType)
{
	if (!devicePtr)
	{
		//OH GOD END IT NOW
		return;
	}
	/*Heap creation goes here, if needed?*/
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	ID3D12DescriptorHeap* heapPtr = nullptr;
	
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = maximumNumberOfBuffersBoundAtOnce;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HRESULT hr = devicePtr->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapPtr));
	if (FAILED((hr)))
	{
		//THROW!
		return;
	}
	constantBufferHeapMap[bufferType] = heapPtr;

	D3D12_HEAP_PROPERTIES constantHeapProperties = {};
	constantHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	constantHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	constantHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	constantHeapProperties.VisibleNodeMask = 0;
	constantHeapProperties.CreationNodeMask = 0;

	D3D12_RESOURCE_DESC constanstBufferDesc = {};
	constanstBufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	constanstBufferDesc.DepthOrArraySize = 1;
	constanstBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	constanstBufferDesc.Height = 1;
	constanstBufferDesc.MipLevels = 1;
	constanstBufferDesc.SampleDesc.Count = 1;
	constanstBufferDesc.SampleDesc.Quality = 0;
	constanstBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	constanstBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	constanstBufferDesc.Width = ((constantBufferSizes[bufferType] + 255) & ~255) * maximumNumberOfBuffersBoundAtOnce;

	ID3D12Resource* resourcePtr;
	hr = devicePtr->CreateCommittedResource(&constantHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&constanstBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resourcePtr));
	if (FAILED(hr))
	{
		//ERRORS!
		return;
	}
	resourcePtr->SetName(L"ConstantBuffer " + bufferType);

	D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc[512] = {};
		
		auto CPUHandle = heapPtr->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < maximumNumberOfBuffersBoundAtOnce; i++)
	{
		viewDesc[i].SizeInBytes = ((constantBufferSizes[bufferType] + 255) & ~255);
		viewDesc[i].BufferLocation = resourcePtr->GetGPUVirtualAddress();
		viewDesc[i].BufferLocation += i*((constantBufferSizes[bufferType] + 255) & ~255);
		devicePtr->CreateConstantBufferView(&viewDesc[i], CPUHandle );
		CPUHandle.ptr += devicePtr->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	constantBufferResourceMap[bufferType] = resourcePtr;
	void* cpuPtr;
	D3D12_RANGE range = { 0,0 };
	resourcePtr->Map(0, &range, &cpuPtr);
	cpu_MappedPtrs[bufferType] = cpuPtr;

}

void ConstantBufferHandler::BindBuffer(UINT8 ID, ConstantBufferType bufferType, UINT index)
{
	/*No error checking yet!*/
	ConstantBuffer* buffer = bufferVector[ID][bufferType];
	char* pointerWithOffset = (char*)cpu_MappedPtrs.find(buffer->type)->second;
	pointerWithOffset += index*((constantBufferSizes[bufferType] + 255) & ~255);
	memcpy(pointerWithOffset, buffer->rawData, buffer->dataSize);

}

void ConstantBufferHandler::UpdateBuffer(UINT8 ID, ConstantBufferType bufferType, void * newData)
{
	/*Time complexity for map.find might become a problem...*/
	if (bufferVector.find(ID) == bufferVector.end())
		CreateConstantBuffer(ID, newData, bufferType);
	else if(bufferVector.find(ID)->second.find(bufferType) == bufferVector.find(ID)->second.end())
		CreateConstantBuffer(ID, newData, bufferType);			
	else
		memcpy(bufferVector[ID][bufferType]->rawData, newData, bufferVector[ID][bufferType]->dataSize);
}

void ConstantBufferHandler::SetDescriptorHeap(ConstantBufferType bufferType, ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetDescriptorHeaps(1, &constantBufferHeapMap[bufferType]);
}

void ConstantBufferHandler::SetGraphicsRoot(ConstantBufferType bufferType, UINT index, UINT offset, ID3D12GraphicsCommandList* cmdList)
{
	auto GPUHandle = constantBufferHeapMap[bufferType]->GetGPUDescriptorHandleForHeapStart();
	GPUHandle.ptr += offset;
	cmdList->SetGraphicsRootDescriptorTable(index, GPUHandle);
}

void * ConstantBufferHandler::GetBufferData(UINT8 ID, ConstantBufferType bufferType)
{
	return bufferVector[ID][bufferType]->rawData;
}

ConstantBufferHandler::ConstantBufferHandler(ConstantBufferSizes sizes, UINT16 maximumNumberOfBindings, ID3D12Device* deviceRef)
{
	this->maximumNumberOfBuffersBoundAtOnce = maximumNumberOfBindings;
	this->constantBufferSizes = sizes;
	this->devicePtr = deviceRef;

	CreateHeap(ConstantBufferType::VERTEX_SHADER_PER_OBJECT_DATA);
	CreateHeap(ConstantBufferType::VERTEX_SHADER_PER_FRAME_DATA);
	CreateHeap(ConstantBufferType::PIXEL_SHADER_LIGHT_DATA);
	CreateHeap(ConstantBufferType::COMPUTE_LIGHT_DATA);

}

ConstantBufferHandler::~ConstantBufferHandler()
{
	for (auto &resource : constantBufferResourceMap)
		resource.second->Release();
	for (auto &heap : constantBufferHeapMap)
		heap.second->Release();
	for (auto &buffer : bufferVector)
	{
		for (auto &innerBuffer : buffer.second)
		{
			delete innerBuffer.second->rawData;
			delete innerBuffer.second;
		}
	}
}
