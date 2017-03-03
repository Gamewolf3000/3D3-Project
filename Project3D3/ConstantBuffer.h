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
		VERTEX_SHADER_WORLD,
		VERTEX_SHADER_VIEWPROJECTION,
		COMPUTE_LIGHT_DATA
	};

	INT8 CreateConstantBuffer(void* data, size_t dataSize, ConstantBufferType bufferType);

	void CreateHeap(ConstantBufferType bufferType);

	void BindBuffer(INT8 ID, UINT offset);

	ConstantBufferHandler();
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

	std::map<ConstantBufferType, void*> cpu_MappedPtrs;
	std::vector<ConstantBuffer*> bufferVector;
	UINT8 nrOfBuffers = 0;



};





#endif