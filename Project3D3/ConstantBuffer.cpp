#include "ConstantBuffer.h"

INT8 ConstantBufferHandler::CreateConstantBuffer(void * data, size_t dataSize, ConstantBufferType bufferType)
{
	ConstantBuffer* buffer = new ConstantBuffer;
	buffer->rawData = new char[dataSize];
	memcpy(buffer->rawData, data, dataSize);
	buffer->dataSize = dataSize;
	buffer->type = bufferType;

	if (cpu_MappedPtrs.find(bufferType) == cpu_MappedPtrs.end())
	{
		/*Taking for granted that the buffers are created during initialization, NOT during rendering*/
		CreateHeap(bufferType);
	}



	bufferVector.push_back(buffer);

	return nrOfBuffers++;
}

void ConstantBufferHandler::CreateHeap(ConstantBufferType bufferType)
{
	if (!devicePtr)
	{
		//OH GOD END IT NOW
		return;
	}
	/*Heap creation goes here, if needed?*/
	//D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//ID3D12Heap* heapPtr = nullptr;
	//
	//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//heapDesc.NodeMask = 0;
	//heapDesc.NumDescriptors = maximumNumberOfBuffersBoundAtOnce;
	//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//HRESULT hr = devicePtr->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapPtr));
	//if (FAILED((hr)))
	//{
	//	//THROW!
	//	return;
	//}
	//constantBufferHeapMap[bufferType] = heapPtr;

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

	switch (bufferType)
	{
	case VERTEX_SHADER_VIEWPROJECTION:
		constanstBufferDesc.Width = maximumNumberOfBuffersBoundAtOnce * constantBufferSizes.VERTEX_SHADER_VIEWPROJECTION_SIZE;
		break;

	case VERTEX_SHADER_WORLD:
		constanstBufferDesc.Width = maximumNumberOfBuffersBoundAtOnce * constantBufferSizes.VERTEX_SHADER_WORLD_SIZE;
		break;

	case COMPUTE_LIGHT_DATA:
		constanstBufferDesc.Width = maximumNumberOfBuffersBoundAtOnce * constantBufferSizes.COMPUTE_LIGHT_DATA_SIZE;
		break;
	}
	ID3D12Resource* resourcePtr;
	HRESULT hr = devicePtr->CreateCommittedResource(&constantHeapProperties,
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
	constantBufferResourceMap[bufferType] = resourcePtr;
	void* cpuPtr;
	D3D12_RANGE range = { 0,0 };
	resourcePtr->Map(0, &range, &cpuPtr);
	cpu_MappedPtrs[bufferType] = cpuPtr;

}

void ConstantBufferHandler::BindBuffer(INT8 ID, UINT offset)
{
	/*No error checking yet!*/
	ConstantBuffer* buffer = bufferVector[ID];
	char* pointerWithOffset = (char*)cpu_MappedPtrs.find(buffer->type)->second;
	pointerWithOffset += offset;
	memcpy(pointerWithOffset, buffer->rawData, buffer->dataSize);

}

ConstantBufferHandler::ConstantBufferHandler(ConstantBufferSizes sizes, UINT16 maximumNumberOfBindings, ID3D12Device* deviceRef)
{
	this->maximumNumberOfBuffersBoundAtOnce = maximumNumberOfBindings;
	this->constantBufferSizes = sizes;
	this->devicePtr = deviceRef;
}

ConstantBufferHandler::~ConstantBufferHandler()
{/*
	for (auto &ptr : cpu_MappedPtrs)
		delete ptr.second;*/
	for (auto &resource : constantBufferResourceMap)
		resource.second->Release();
	for (auto &heap : constantBufferHeapMap)
		heap.second->Release();
	for (auto &buffer : bufferVector)
	{
		delete buffer->rawData;
		delete buffer;
	}
}
