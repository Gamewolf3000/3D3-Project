#pragma once
#ifndef D3D12Wrapper

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_5.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include "CD3DX12Helper.h"

#include <string>
#include <vector>

struct PipelineData
{
	ID3D12RootSignature* rootSignature = nullptr;
	ID3D12PipelineState* pipelineState = nullptr;

	~PipelineData()
	{
		rootSignature->Release();
		pipelineState->Release();
	}
};

enum ResourceType
{
	CBV,
	SRV,
	SAMPLER, // not implemented in compute pipeline
	UAV // not implemented in graphics pipeline
};

enum RootType
{
	DESCRIPTOR_TABLE,
	CBV_ROOT,
	SRV_ROOT,
	SAMPLER_ROOT,
	UAV_ROOT
};

struct ResourceDescription
{
	ResourceType type;
	UINT8 shaderRegister;
	RootType rType = DESCRIPTOR_TABLE;
};

enum ShaderVisibility
{
	VERTEX,
	PIXEL
};

struct RootSignatureData
{
	std::vector<ResourceDescription> type;
	std::vector<ShaderVisibility> visibility;
};

enum InputDataType
{
	FLOAT32_4,
	FLOAT32_3,
	FLOAT32_2

};

struct InputLayoutData
{
	LPCSTR inputName = "";
	InputDataType dataType;
	UINT8 arraySize;

};

class Pipeline
{
private:
	Pipeline();

	ID3D12Device* device;

	std::vector<PipelineData*> pipelines;
	std::vector<PipelineData*> computePipelines;

	void CreateRootSignature(PipelineData* data, RootSignatureData rootData);
	void CreatePipelineStateObject(PipelineData* data, std::string vs, std::string ps, std::vector<InputLayoutData> layoutData, bool deferredRendering);

	ID3D12RootSignature* CreateComputeRootSignature(RootSignatureData rootData);


public:
	Pipeline(ID3D12Device* dev);
	~Pipeline();

	UINT8 CreatePipeline(RootSignatureData rootData, std::string vs, std::string ps, std::vector<InputLayoutData> layoutData, bool deferredRendering = false);
	void SetPipelineState(UINT8 pipelineID, ID3D12GraphicsCommandList * cmdList);

	UINT8 CreateComputePipeline(RootSignatureData rsData, std::string cs);
	void SetComputePipelineState(UINT8 pipelineID, ID3D12GraphicsCommandList * cmdList);

};


#endif // !D3D12Wrapper
