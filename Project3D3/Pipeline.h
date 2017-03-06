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
	SAMPLER
};

enum ShaderVisibility
{
	VERTEX,
	PIXEL
};

struct RootSignatureData
{
	std::vector<ResourceType> type;
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
	std::wstring inputName = L"";
	InputDataType dataType;
	UINT8 arraySize;

};

class Pipeline
{
private:
	Pipeline();

	ID3D12Device* device;

	std::vector<PipelineData*> pipelines;

	void CreateRootSignature(PipelineData* data, RootSignatureData rootData);
	void CreatePipelineStateObject(PipelineData* data, std::string vs, std::string ps, std::vector<InputLayoutData> layoutData);


public:
	Pipeline(ID3D12Device* dev);

	UINT8 CreatePipeline(RootSignatureData rootData, std::string vs, std::string ps, std::vector<InputLayoutData> layoutData);
	void SetPipelineState(UINT8 pipelineID, ID3D12GraphicsCommandList * cmdList);

};


#endif // !D3D12Wrapper
