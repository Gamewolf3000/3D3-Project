#include "Pipeline.h"

void Pipeline::CreateRootSignature(PipelineData * data, RootSignatureData rootData)
{

	if (rootData.type.size() != rootData.visibility.size())
	{
		// SHITS GONE HORRIBLY WRONG
		return;
	}

	CD3DX12_DESCRIPTOR_RANGE* descRanges = new CD3DX12_DESCRIPTOR_RANGE[rootData.type.size()];
	for (int i = 0; i < rootData.type.size(); i++)
	{
		if (rootData.type[i] == ResourceType::CBV)
		{
			descRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		}
		else if (rootData.type[i] == ResourceType::SRV)
		{
			descRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		}
		else if (rootData.type[i] == ResourceType::SAMPLER)
		{
			descRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
		}
	}

	CD3DX12_ROOT_PARAMETER* rootParameters = new CD3DX12_ROOT_PARAMETER[rootData.visibility.size()];
	for (int i = 0; i < rootData.visibility.size(); i++)
	{
		if (rootData.visibility[i] == ShaderVisibility::VERTEX)
		{
			rootParameters[i].InitAsDescriptorTable(1, &descRanges[i], D3D12_SHADER_VISIBILITY_VERTEX);
		}
		else if (rootData.visibility[i] == ShaderVisibility::PIXEL)
		{
			rootParameters[i].InitAsDescriptorTable(1, &descRanges[i], D3D12_SHADER_VISIBILITY_PIXEL);
		}
	}

	CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Init((UINT)rootData.type.size(), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* sBlob;
	HRESULT hr = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr);
	auto a = sBlob->GetBufferSize();

	hr = device->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&data->rootSignature));

	sBlob->Release();

	delete descRanges;
	delete rootParameters;
}

void Pipeline::CreatePipelineStateObject(PipelineData * data, std::string vs, std::string ps, std::vector<InputLayoutData> layoutData)
{
	std::wstring temp(vs.begin(), vs.end());
	LPCWSTR fileName = temp.c_str();
	////// Shader Compiles //////
	ID3DBlob* vertexBlob;
	HRESULT hr = D3DCompileFromFile(
		fileName, // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",			// entry point
		"vs_5_1",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&vertexBlob,	// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);

	ID3DBlob* pixelBlob;
	temp = std::wstring(ps.begin(), ps.end());
	fileName = temp.c_str();
	hr = D3DCompileFromFile(
		fileName, // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",			// entry point
		"ps_5_1",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&pixelBlob,		// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);

	int totalLayoutSize = 0;

	for (auto input : layoutData)
	{
		totalLayoutSize = input.arraySize;
	}

	D3D12_INPUT_ELEMENT_DESC* inputElementDesc = new D3D12_INPUT_ELEMENT_DESC[totalLayoutSize];
	int nrOfSet = 0;

	for (int i = 0; i < layoutData.size(); i++)
	{
		for (UINT j = 0; j < layoutData[i].arraySize; j++)
		{
			if (layoutData[i].dataType == InputDataType::FLOAT32_4)
			{
				inputElementDesc[nrOfSet] = {(LPCSTR)layoutData[i].inputName.c_str(), j, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
			}
			else if (layoutData[i].dataType == InputDataType::FLOAT32_3)
			{
				inputElementDesc[nrOfSet] = { (LPCSTR)layoutData[i].inputName.c_str(), j, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			}
			else if (layoutData[i].dataType == InputDataType::FLOAT32_2)
			{
				inputElementDesc[nrOfSet] = { (LPCSTR)layoutData[i].inputName.c_str(), j, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			}

			nrOfSet++;
		}

		
		// Set all the things in here
	}

	////// Input Layout //////
	//D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "POSITION", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16 ,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "POSITION", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32 ,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL"	, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL"	, 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "UV" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 96 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "UV" , 1, DXGI_FORMAT_R32G32_FLOAT, 0, 104 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "UV" , 2, DXGI_FORMAT_R32G32_FLOAT, 0, 112 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	//};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = totalLayoutSize; // ARRAYSIZE(inputElementDesc);

	////// Pipline State //////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	//Specify pipeline stages:
	gpsd.pRootSignature = data->rootSignature;
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vertexBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = vertexBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(pixelBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = pixelBlob->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	//DEPTH STENCIL CHEAT!
	gpsd.DepthStencilState.DepthEnable = true;
	gpsd.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsd.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpsd.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	hr = device->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&data->pipelineState));
	data->pipelineState->SetName(L"TestState");
	vertexBlob->Release();
	pixelBlob->Release();
}

Pipeline::Pipeline(ID3D12Device * dev)
{
	device = dev;
}

Pipeline::~Pipeline()
{
	for (auto i : pipelines)
	{
		delete i;
	}
}

UINT8 Pipeline::CreatePipeline(RootSignatureData rootData, std::string vs, std::string ps, std::vector<InputLayoutData> layoutData)
{
	PipelineData* data = new PipelineData();

	CreateRootSignature(data, rootData);
	CreatePipelineStateObject(data, vs, ps, layoutData);

	pipelines.push_back(data);

	return (UINT8)pipelines.size() - 1;
}

void Pipeline::SetPipelineState(UINT8 pipelineID, ID3D12GraphicsCommandList * cmdList)
{
	cmdList->SetGraphicsRootSignature(pipelines[pipelineID]->rootSignature);
	cmdList->SetPipelineState(pipelines[pipelineID]->pipelineState);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
