#ifndef CONSTANT_BUFFER_H
#define CONSTANT_BUFFER_H

#include <d3d12.h>
#include <dxgi1_5.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include <map>
#include <vector>

class ConstantBufferHandler
{
public:
	/*WARNING: Must check the "size" of the buffers and change accordingly. Later point?*/
	enum ConstantBufferType {
		VERTEX_SHADER_PER_OBJECT_DATA,
		VERTEX_SHADER_PER_FRAME_DATA,
		COMPUTE_LIGHT_DATA
	};
	/*MUST be set by the caller. Data to create the "size" of the buffers.
	NOTE: If the above enum gets extra data, so must the struct!*/
	struct ConstantBufferSizes {
		UINT16 VERTEX_SHADER_PER_OBJECT_DATA_SIZE = 0;
		UINT16 VERTEX_SHADER_PER_FRAME_DATA_SIZE = 0;
		UINT16 COMPUTE_LIGHT_DATA_SIZE = 0;

	};


	UINT8 CreateConstantBuffer(void* data, size_t dataSize, ConstantBufferType bufferType);

	void CreateHeap(ConstantBufferType bufferType);

	void BindBuffer(UINT8 ID, UINT offset);

	ConstantBufferHandler(ConstantBufferSizes sizes = ConstantBufferSizes() , UINT16 maximumNumberOfBindings = 512, ID3D12Device* deviceRef = nullptr);
	~ConstantBufferHandler();

	ConstantBufferHandler(const ConstantBufferHandler &original) = delete;
	ConstantBufferHandler operator=(const ConstantBufferHandler &original) = delete;

private:
	
	struct ConstantBuffer
	{
		void *rawData;
		size_t dataSize;
		ConstantBufferType type;
	};

	std::map<ConstantBufferType, ID3D12Heap*> constantBufferHeapMap;
	std::map<ConstantBufferType, ID3D12Resource*> constantBufferResourceMap;
	std::map<ConstantBufferType, void*> cpu_MappedPtrs;
	std::vector<ConstantBuffer*> bufferVector;

	UINT8 nrOfBuffers = 0;
	UINT16 maximumNumberOfBuffersBoundAtOnce = 0;
	ConstantBufferSizes constantBufferSizes;
	ID3D12Device* devicePtr = nullptr;

};





#endif