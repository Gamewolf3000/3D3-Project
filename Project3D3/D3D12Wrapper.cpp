#include "D3D12Wrapper.h"
#include <iostream>
#include "LightHandler.h"

D3D12Wrapper::D3D12Wrapper(HINSTANCE hInstance, int nCmdShow, UINT16 width, UINT16 height)
{
	CoInitialize(NULL);

	windowHeight = height;
	windowWidth = width;
	
	initialize(hInstance, nCmdShow);

	pipelineHandler = new Pipeline(device);
	CreatePipelines();
	lightHandler = new LightHandler(MAXNROFLIGHTS);
	meshHandler = new MeshHandler(device);
	textureHandler = new TextureHandler(device);

	auto sizes = ConstantBufferHandler::ConstantBufferSizes();
	sizes.VERTEX_SHADER_PER_OBJECT_DATA_SIZE = sizeof(ConstantBufferStruct);
	sizes.VERTEX_SHADER_PER_FRAME_DATA_SIZE = sizeof(ViewProjectionStruct);
	sizes.PIXEL_SHADER_LIGHT_DATA_SIZE = sizeof(LightHandler::PointLight)*MAXNROFLIGHTS;
	sizes.COMPUTE_LIGHT_DATA_SIZE = sizeof(float); /*To be edited!*/
	sizes.COMPUTE_CAMERA_POS_SIZE = sizeof(Float4D);
	constantBufferHandler = new ConstantBufferHandler(sizes, 512, device);

	vpStruct = new ViewProjectionStruct;

	MatrixToFloat4x4(vpStruct->viewMatrix, MatrixTranspose(MatrixViewLH(VecCreate(0, 0, -1, 1), VecCreate(0, 0, 0, 1), VecCreate(0, 1, 0, 0))));
	MatrixToFloat4x4(vpStruct->projectionMatrix, MatrixTranspose(MatrixProjectionLH(JEX_PI/2, 1280.0/720.0, 0.1f, 100.0f)));
	camPos = Float3D(0.0f, 0.0f, -1.0f);

	float lightColour[4] = { .5f, .5f, 0.5f, 1.0f };
	float position[4] = { .0f, .0f, -4.0f, 1.0f };
	for (int i = 0; i < MAXNROFLIGHTS; i++)
	{
		position[0] = 3.5f*cos(i * 2.0 * JEX_PI / MAXNROFLIGHTS);
		//position[1] = 3.5f*sinf(i * 2 * JEX_PI / MAXNROFLIGHTS);
		position[2] = 3.5f*sin(i * 2.0 * JEX_PI / MAXNROFLIGHTS);
		lightColour[0] = abs(2.5f*cos(i * 2.0 * JEX_PI / MAXNROFLIGHTS))*0.15f*(1.0f / MAXNROFLIGHTS);
		lightColour[1] = abs(3.5f*sinf(i * 2 * JEX_PI / MAXNROFLIGHTS))*0.15f*(1.0f / MAXNROFLIGHTS);
		lightColour[2] = abs(2.5f*sin(i * 2.0 * JEX_PI / MAXNROFLIGHTS))*.15f*(1.0f / MAXNROFLIGHTS);
		lightHandler->AddLight(i, lightColour, position, 4.f);
	}

	MatrixToFloat4x4(computeCamera.projectionMatrix, MatrixProjectionLH(JEX_PI / 2, 1280.0 / 720.0, 0.1f, 100.0f));

	//Matrix test = MatrixProjectionLH(JEX_PI / 2, 1280.0 / 720.0, 0.1f, 100.0f);

	//constantBufferHandler->CreateConstantBuffer(127, vpStruct, ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA);
	SetupComputeShader();

	timer = new TimerClass;
	timer->Reset();

	std::cout << "We think it worked to create the Wrapper. We don't check that." << std::endl;
}

void D3D12Wrapper::InitializePerformanceVariables()
{

	HRESULT hr;
	commandQueue->GetTimestampFrequency(&timestampFrequency[0]);

	D3D12_QUERY_HEAP_DESC qDesc = {};
	qDesc.Count = 2*NUM_TIME_STAMPS;
	qDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

	hr = device->CreateQueryHeap(&qDesc, IID_PPV_ARGS(&queryHeap));


	D3D12_HEAP_PROPERTIES hProps = {};
	hProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	hProps.Type = D3D12_HEAP_TYPE_READBACK;
	hProps.CreationNodeMask = 0;
	hProps.VisibleNodeMask = 0;
	hProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;


	D3D12_RESOURCE_DESC desc = {};

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = 16*NUM_TIME_STAMPS;
	desc.Height = desc.DepthOrArraySize = desc.MipLevels = 1;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;



	hr = device->CreateCommittedResource(&hProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&heapData));


}

D3D12Wrapper::~D3D12Wrapper()
{
	Shutdown();

	delete pipelineHandler;
	delete meshHandler;
	delete textureHandler;
	delete constantBufferHandler;
}

void D3D12Wrapper::Render(EntityHandler* handler)
{
	DisplayFps();
	timer->Tick();

	//const std::vector<Entity*> &entities = handler->GetEntityVector();
	
	StartTimer();
	WaitForGPU();

	RenderPrePass(handler);	
	WaitForGPU();

	DispatchComputeShader();
	RenderGeometryPass(handler);

	WaitForGPU();
	WaitForCompute();

	LightPass();
	WaitForGPU();
	//commandList->Close();

	Present();
	WaitForGPU();

	EndTimer();
	WaitForGPU();
}

void D3D12Wrapper::MoveCamera(Float3D position, float rotation)
{
	Vec lookDir = VecCreate(0, 0, 1, 0);
	Vec positionVec = VecCreate(position.x, position.y, position.z, 1.0f);
	camPos = position;
	this->rotation = rotation;

	camPos = position;

	Matrix rotMatrix = MatrixRotationAroundAxis(VecCreate(0.0f, 1.0f, 0.0f, 0.0f), rotation);

	lookDir = VecMultMatrix3D(lookDir, rotMatrix);

	MatrixToFloat4x4(vpStruct->viewMatrix, MatrixTranspose(MatrixViewLH(positionVec, VecAdd(lookDir, positionVec), VecCreate(0.0f, 1.0f, 0.0f, 0.0f))));
}

void D3D12Wrapper::InitializeWindow(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"3D3_PROJECT";
	if (!RegisterClassEx(&wcex))
	{
		//THROW HERE
		return;
	}

	RECT rc = { 0, 0, windowWidth, windowHeight};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		L"3D3_PROJECT",
		L"AWESOME LIGHTING IN D12 THAT MIGHT OR MIGHT NOT WORK INCLUDING SOME KIND OF DEFFERED STUFF FOR PERFOMANCE NOOBS",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	ShowWindow(window, nCmdShow);
}

LRESULT CALLBACK D3D12Wrapper::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void D3D12Wrapper::CreateDirect3DDevice()
{
#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	ID3D12Debug* debugController = nullptr;

#ifdef STATIC_LINK_DEBUGSTUFF
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease(debugController);
#else
	HMODULE mD3D12 = GetModuleHandle(L"D3D12.dll");
	PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
	if (SUCCEEDED(f(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease(&debugController);
#endif
#endif

	//dxgi1_5 is only needed for the initialization process using the adapter.
	IDXGIFactory4* factory = nullptr;
	IDXGIAdapter1* adapter = nullptr;
	//First a factory is created to iterate through the adapters available.

	HRESULT hr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	for (UINT adapterIndex = 0;; ++adapterIndex)
	{
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter))
		{
			break; //No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), nullptr)))
		{
			break;
		}

		SafeRelease(&adapter);
	}
	if (adapter)
	{
		//Create the actual device.
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device));

		SafeRelease(&adapter);
	}
	else
	{

		//Create warp device if no adapter was found.
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
	}




	SafeRelease(&factory);
}

void D3D12Wrapper::CreateCommandInterfacesAndSwapChain()
{
	HRESULT hr;
	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	hr = device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&commandQueue));

	cqd.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

	hr = device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&computeQueue));

	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&computeAllocator));


	//Create command list.
	hr = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator,
		nullptr,
		IID_PPV_ARGS(&commandList));

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	commandList->Close();

	hr = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator,
		nullptr,
		IID_PPV_ARGS(&commandListPrePass));

	commandListPrePass->Close();

	hr = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		computeAllocator,
		nullptr,
		IID_PPV_ARGS(&commandListComputePass));

	commandListComputePass->Close();

	hr = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator,
		nullptr,
		IID_PPV_ARGS(&commandListGeometryPass));

	commandListGeometryPass->Close();

	hr = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator,
		nullptr,
		IID_PPV_ARGS(&commandListPostPass));

	commandListPostPass->Close();



	IDXGIFactory4*	factory = nullptr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));



	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = NUM_SWAP_BUFFERS;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	 hr = factory->CreateSwapChainForHwnd(
		commandQueue,
		window,
		&scDesc,
		nullptr,
		nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&swapChain)
	);

	SafeRelease(&factory);
}

void D3D12Wrapper::CreateFenceAndEventHandle()
{
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	fenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	eventHandle = CreateEvent(0, false, false, 0);

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&prePassFence));
	prePassFenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	prePassEventHandle = CreateEvent(0, false, false, 0);
}

void D3D12Wrapper::CreateRenderTargets()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	HRESULT hr = device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&renderTargetsHeap));

	//Create resources for the render targets.
	renderTargetDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		hr = swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]));
		device->CreateRenderTargetView(renderTargets[n], nullptr, cdh);
		cdh.ptr += renderTargetDescriptorSize;
		switch (n)
		{
		case 0:
			renderTargets[n]->SetName(L"RenderTarget: 1");
			break;
		case 1:
			renderTargets[n]->SetName(L"RenderTarget: 2");
			break;


		}
	}


}

void D3D12Wrapper::CreateViewportAndScissorRect()
{
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)windowWidth;
	vp.Height = (float)windowHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	scissorRect.left = (long)0;
	scissorRect.right = (long)windowWidth;
	scissorRect.top = (long)0;
	scissorRect.bottom = (long)windowHeight;
}

void D3D12Wrapper::CreateDepthStencil()
{
	HRESULT hr;

	D3D12_RESOURCE_DESC depthDesc = {};

	depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthDesc.Alignment = 0;
	depthDesc.Width = windowWidth;
	depthDesc.Height = windowHeight;
	depthDesc.DepthOrArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.MipLevels = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D32_FLOAT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&depthStencileHeap));

	hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_COMMON,
		&optClear, IID_PPV_ARGS(&depthstencil));

	device->CreateDepthStencilView(depthstencil, nullptr, depthStencileHeap->GetCPUDescriptorHandleForHeapStart());

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthstencil, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

void D3D12Wrapper::SetupComputeShader()
{
	HRESULT hr;

	RootSignatureData rsData;

	ResourceDescription UAV;
	UAV.type = ResourceType::UAV;
	UAV.shaderRegister = 0;
	rsData.type.push_back(UAV);

	ResourceDescription SRV;
	SRV.type = ResourceType::SRV;
	SRV.shaderRegister = 0;
	rsData.type.push_back(SRV);

	SRV.type = ResourceType::SRV;
	SRV.shaderRegister = 1;
	SRV.rType = SRV_ROOT;
	rsData.type.push_back(SRV);

	ResourceDescription CBV;
	CBV.type = ResourceType::CBV;
	CBV.shaderRegister = 0;
	CBV.rType = CBV_ROOT;
	rsData.type.push_back(CBV);

	CBV.type = ResourceType::CBV;
	CBV.shaderRegister = 1;
	CBV.rType = CBV_ROOT;
	rsData.type.push_back(CBV);

	computePipelineID = pipelineHandler->CreateComputePipeline(rsData, "TestComputeShader.hlsl");

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 2;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
	uavHeapDesc.NumDescriptors = 1;
	uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&computeShaderResourceHeapSRV));
	hr = device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&computeShaderResourceHeapUAV));

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = windowWidth;
	texDesc.Height = windowHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&computeShaderResourceOutput));

	hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&computeShaderResourceInput));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptor(computeShaderResourceHeapSRV->GetCPUDescriptorHandleForHeapStart());

	device->CreateShaderResourceView(computeShaderResourceOutput, &srvDesc, descriptor);
	descriptor.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	device->CreateShaderResourceView(computeShaderResourceInput, &srvDesc, descriptor);
	device->CreateUnorderedAccessView(computeShaderResourceOutput, nullptr, &uavDesc, computeShaderResourceHeapUAV->GetCPUDescriptorHandleForHeapStart());

	//---------------------------------------------------------------------------------------------------------------------

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
	hr = device->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&computeShaderResourceMeshes));

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ComputeShaderStruct)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&computeShaderResourceFrameData)
	);

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(LightHandler::PointLight) * MAXNROFLIGHTS + sizeof(int) * 4),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&computeShaderResourceLightData)
	);

}

void D3D12Wrapper::InitializeDeferredRendering()
{
	RootSignatureData rootData[2];
	ResourceDescription CBV;

	/*Rasterizing Stage*/
	CBV.shaderRegister = 0;
	CBV.type = ResourceType::CBV;
	rootData[0].type.push_back(CBV);
	rootData[0].visibility.push_back(VERTEX);
	CBV.shaderRegister = 1;
	rootData[0].type.push_back(CBV);
	rootData[0].visibility.push_back(VERTEX);
	CBV.shaderRegister = 0;
	CBV.type = ResourceType::CBV;
	rootData[0].type.push_back(CBV);
	rootData[0].visibility.push_back(PIXEL);

	ResourceDescription SRV;
	SRV.shaderRegister = 0;
	SRV.type = ResourceType::SRV;
	rootData[0].type.push_back(SRV);
	rootData[0].visibility.push_back(PIXEL);

	SRV.shaderRegister = 1;
	SRV.type = ResourceType::SRV;
	rootData[0].type.push_back(SRV);
	rootData[0].visibility.push_back(PIXEL);

	ResourceDescription SAMP;
	SAMP.shaderRegister = 0;
	SAMP.type = ResourceType::SAMPLER;
	rootData[0].type.push_back(SAMP);
	rootData[0].visibility.push_back(PIXEL);

	std::vector<InputLayoutData> layoutData;
	InputLayoutData tempLayout;
	tempLayout.inputName = "POSITION";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_3;
	layoutData.push_back(tempLayout);
	tempLayout.inputName = "TEXCOORDS";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_2;
	layoutData.push_back(tempLayout);
	tempLayout.inputName = "NORMAL";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_3;
	layoutData.push_back(tempLayout);
	tempLayout.inputName = "TANGENT";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_3;
	layoutData.push_back(tempLayout);
	tempLayout.inputName = "BITANGENT";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_3;
	layoutData.push_back(tempLayout);

	deferredPipelineID[0] = pipelineHandler->CreatePipeline(rootData[0], "RasterizingStageVS.hlsl", "RasterizingStagePS.hlsl", layoutData, true);


	/*Rendering Resources*/

	/*Texture heap (Lightning)*/
	D3D12_DESCRIPTOR_HEAP_DESC textureDesc = {};
	textureDesc.NumDescriptors = 3; // maximum number of textures, bad things will probably happen if we have more
	textureDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	textureDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	HRESULT hr = device->CreateDescriptorHeap(&textureDesc, IID_PPV_ARGS(&GBufferHeapLightning));

	/*Render target heap (Rendering)*/
	textureDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	textureDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&textureDesc, IID_PPV_ARGS(&GBufferHeapRendering));

	D3D12_HEAP_PROPERTIES hProperties;
	hProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	hProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	hProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	hProperties.CreationNodeMask = 0;
	hProperties.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC rDesc;

	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Width = windowWidth;
	rDesc.Height = windowHeight; 
	rDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
	rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_SHADER_RESOURCE_VIEW_DESC sRVDesc;
	sRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	sRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	sRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	sRVDesc.Texture2D.MipLevels = 1;
	sRVDesc.Texture2D.MostDetailedMip = 0;
	sRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	sRVDesc.Texture2D.PlaneSlice = 0;


	D3D12_CLEAR_VALUE clear;
	clear.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	clear.Color[0] = 0.0f;
	clear.Color[1] = 0.0f;
	clear.Color[2] = 1.0f;
	clear.Color[3] = 1.0f;
	
	LPCWSTR names[3] = { L"GBuffer: Normal", L"GBuffer: Colour", L"GBuffer: Pos" };

	auto rtvHandle = GBufferHeapRendering->GetCPUDescriptorHandleForHeapStart();
	auto rtvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto shaderViewHandle = GBufferHeapLightning->GetCPUDescriptorHandleForHeapStart();
	auto shaderViewIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int i = 0; i < 3; i++)
	{
		hr = device->CreateCommittedResource(&hProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&rDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clear,
			IID_PPV_ARGS(&GBuffers[i]));
		GBuffers[i]->SetName(names[i]);

		device->CreateShaderResourceView(GBuffers[i], &sRVDesc, shaderViewHandle);
		device->CreateRenderTargetView(GBuffers[i], nullptr, rtvHandle);

		shaderViewHandle.ptr += shaderViewIncrementSize;
		rtvHandle.ptr += rtvIncrementSize;
		
	}

	/*Create upload heap, make it a shader resource?*/

	/*Light-Pass*/
	
	layoutData.clear();
	CBV.shaderRegister = 0;
	CBV.type = ResourceType::CBV;
	rootData[1].type.push_back(CBV);
	rootData[1].visibility.push_back(PIXEL);

	layoutData.clear();
	CBV.shaderRegister = 1;
	CBV.type = ResourceType::CBV;
	rootData[1].type.push_back(CBV);
	rootData[1].visibility.push_back(PIXEL);

	SRV.shaderRegister = 0;
	SRV.type = ResourceType::SRV;
	rootData[1].type.push_back(SRV);
	rootData[1].visibility.push_back(PIXEL);

	SRV.shaderRegister = 1;
	SRV.type = ResourceType::SRV;
	rootData[1].type.push_back(SRV);
	rootData[1].visibility.push_back(PIXEL);

	SRV.shaderRegister = 2;
	SRV.type = ResourceType::SRV;
	rootData[1].type.push_back(SRV);
	rootData[1].visibility.push_back(PIXEL);

	SRV.shaderRegister = 3;
	SRV.type = ResourceType::SRV;
	rootData[1].type.push_back(SRV);
	rootData[1].visibility.push_back(PIXEL);

	deferredPipelineID[1] = pipelineHandler->CreatePipeline(rootData[1], "LightningStageVS.hlsl", "LightningStagePS.hlsl", layoutData, false);
	
	/*Set the Render targets as shared resources*/
	/*Creating a texture heap for the light pass*/
	

	D3D12_SHADER_RESOURCE_VIEW_DESC rvDesc = {};
	rvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	rvDesc.Texture2D.MipLevels = 1;
	rvDesc.Texture2D.MostDetailedMip = 0;
	rvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


}

void D3D12Wrapper::SetupMeshRendering()
{
	pipelineHandler->SetPipelineState(deferredPipelineID[0], commandListGeometryPass);
	SetResourceTransitionBarrier(commandListGeometryPass,
		GBuffers[GBUFFER_NORMAL],
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);
	SetResourceTransitionBarrier(commandListGeometryPass,
		GBuffers[GBUFFER_COLOUR],
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);
	SetResourceTransitionBarrier(commandListGeometryPass,
		GBuffers[GBUFFER_POS],
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);
	commandListGeometryPass->OMSetRenderTargets(3, &GBufferHeapRendering->GetCPUDescriptorHandleForHeapStart(), true, &depthStencileHeap->GetCPUDescriptorHandleForHeapStart());
	commandListGeometryPass->ClearRenderTargetView(GBufferHeapRendering->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);

	commandListGeometryPass->ClearDepthStencilView(depthStencileHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

}

void D3D12Wrapper::FinishMeshRendering()
{
	SetResourceTransitionBarrier(commandListGeometryPass,
		GBuffers[GBUFFER_NORMAL],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE		//state after
	);
	SetResourceTransitionBarrier(commandListGeometryPass,
		GBuffers[GBUFFER_COLOUR],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE	//state after
	);
	SetResourceTransitionBarrier(commandListGeometryPass,
		GBuffers[GBUFFER_POS],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE	//state after
	);
	
}

void D3D12Wrapper::LightPass()
{
	commandAllocator->Reset();
	HRESULT hr = commandListPostPass->Reset(commandAllocator, nullptr);
	commandListPostPass->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, LIGHT_TIME_START);
	
	ClearBuffer(commandListPostPass);

	commandListPostPass->RSSetViewports(1, &vp);
	commandListPostPass->RSSetScissorRects(1, &scissorRect);

	pipelineHandler->SetPipelineState(deferredPipelineID[1], commandListPostPass);

	constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::COMPUTE_CAMERA_POS, commandListPostPass);
	constantBufferHandler->UpdateBuffer(camPosID, ConstantBufferHandler::ConstantBufferType::COMPUTE_CAMERA_POS, &camPos);
	constantBufferHandler->BindBuffer(camPosID, ConstantBufferHandler::ConstantBufferType::COMPUTE_CAMERA_POS, 0);
	constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::COMPUTE_CAMERA_POS, 0, 0, commandListPostPass);

	constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::PIXEL_SHADER_LIGHT_DATA, commandListPostPass);
	constantBufferHandler->UpdateBuffer(lightID, ConstantBufferHandler::ConstantBufferType::PIXEL_SHADER_LIGHT_DATA, lightHandler->GatherLightJobs());
	constantBufferHandler->BindBuffer(lightID, ConstantBufferHandler::ConstantBufferType::PIXEL_SHADER_LIGHT_DATA, 0);
	constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::PIXEL_SHADER_LIGHT_DATA, 1, 0, commandListPostPass);

	auto handle = GBufferHeapLightning->GetGPUDescriptorHandleForHeapStart();
	auto incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	commandListPostPass->SetDescriptorHeaps(1, &GBufferHeapLightning);
	commandListPostPass->SetGraphicsRootDescriptorTable(2, handle);
	handle.ptr += incrementSize;
	commandListPostPass->SetGraphicsRootDescriptorTable(3, handle);
	handle.ptr += incrementSize;
	commandListPostPass->SetGraphicsRootDescriptorTable(4, handle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(computeShaderResourceHeapSRV->GetGPUDescriptorHandleForHeapStart());
	commandListPostPass->SetDescriptorHeaps(1, &computeShaderResourceHeapSRV);
	commandListPostPass->SetGraphicsRootDescriptorTable(5, texHandle);

	commandListPostPass->DrawInstanced(6, 1, 0, 0);

	SetResourceTransitionBarrier(commandListPostPass,
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	commandListPostPass->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, LIGHT_TIME_END);
	commandListPostPass->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { commandListPostPass };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

}

void D3D12Wrapper::CreatePipelines()
{
	RootSignatureData rootData;
	ResourceDescription CBV;
	CBV.shaderRegister = 0;
	CBV.type = ResourceType::CBV;
	rootData.type.push_back(CBV);
	rootData.visibility.push_back(VERTEX);
	CBV.shaderRegister = 1;
	rootData.type.push_back(CBV);
	rootData.visibility.push_back(VERTEX);
	CBV.shaderRegister = 0;
	CBV.type = ResourceType::CBV;
	rootData.type.push_back(CBV);
	rootData.visibility.push_back(PIXEL);

	std::vector<InputLayoutData> layoutData;

	testPipelineID = pipelineHandler->CreatePipeline(rootData, "TriangleTestVS.hlsl", "TriangleTestPS.hlsl", layoutData);


	RootSignatureData rootData2;
	ResourceDescription CBV2;
	CBV2.shaderRegister = 0;
	CBV2.type = ResourceType::CBV;
	rootData2.type.push_back(CBV2);
	rootData2.visibility.push_back(VERTEX);
	CBV2.shaderRegister = 1;
	rootData2.type.push_back(CBV2);
	rootData2.visibility.push_back(VERTEX);
	CBV2.shaderRegister = 0;
	CBV2.type = ResourceType::CBV;
	rootData2.type.push_back(CBV2);
	rootData2.visibility.push_back(PIXEL);

	ResourceDescription SRV;
	SRV.shaderRegister = 0;
	SRV.type = ResourceType::SRV;
	rootData2.type.push_back(SRV);
	rootData2.visibility.push_back(PIXEL);

	SRV.shaderRegister = 1;
	SRV.type = ResourceType::SRV;
	rootData2.type.push_back(SRV);
	rootData2.visibility.push_back(PIXEL);

	ResourceDescription SAMP;
	SAMP.shaderRegister = 0;
	SAMP.type = ResourceType::SAMPLER;
	rootData2.type.push_back(SAMP);
	rootData2.visibility.push_back(PIXEL);

	std::vector<InputLayoutData> layoutData2;
	InputLayoutData tempLayout;
	tempLayout.inputName = "POSITION";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_3;
	layoutData2.push_back(tempLayout);
	tempLayout.inputName = "TEXCOORDS";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_2;
	layoutData2.push_back(tempLayout);
	tempLayout.inputName = "NORMAL";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_3;
	layoutData2.push_back(tempLayout);
	tempLayout.inputName = "TANGENT";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_3;
	layoutData2.push_back(tempLayout);
	tempLayout.inputName = "BITANGENT";
	tempLayout.arraySize = 1;
	tempLayout.dataType = FLOAT32_3;
	layoutData2.push_back(tempLayout);

	meshPipelineID = pipelineHandler->CreatePipeline(rootData2, "MeshTestVS.hlsl", "MeshTestPS.hlsl", layoutData2);


	RootSignatureData rootData3;
	ResourceDescription CBV3;
	CBV3.shaderRegister = 0;
	CBV3.type = ResourceType::CBV;
	rootData3.type.push_back(CBV3);
	rootData3.visibility.push_back(VERTEX);
	CBV3.shaderRegister = 1;
	rootData3.type.push_back(CBV3);
	rootData3.visibility.push_back(VERTEX);

	// Same layout as before so reusing

	prePassPipelineID = pipelineHandler->CreatePipeline(rootData3, "PrePassVs.hlsl", "PrePassPS.hlsl", layoutData2);

	InitializeDeferredRendering();
}

void D3D12Wrapper::DisplayFps()
{
	static int frameCount = 0;
	static float timeElapsed = 0.0f;

	frameCount++;

	if ((timer->time() - timeElapsed) > 1.0f)
	{
		float fps = (float)frameCount;
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << L"AWESOME LIGHTING IN D12 THAT MIGHT OR MIGHT NOT WORK INCLUDING SOME KIND OF DEFFERED STUFF FOR PERFOMANCE NOOBS" << L"    " << L"FPS:  " << fps << L"    " << L"Frame Time: " << mspf
			<< L"  (ms)";
		SetWindowText(window, outs.str().c_str());

		frameCount = 0;
		timeElapsed += 1.0f;
	}
}

void D3D12Wrapper::DispatchComputeShader()
{
	computeAllocator->Reset();
	HRESULT hr = commandListComputePass->Reset(computeAllocator, nullptr);
	commandListComputePass->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, COMPUTE_TIME_START);
	
	//CopyDepthBuffer();
	commandListComputePass->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(computeShaderResourceOutput, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	pipelineHandler->SetComputePipelineState(computePipelineID, commandListComputePass);

	//ID3D12DescriptorHeap* ppHeaps[] = { m_srvUavHeap.Get() };
	//pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_srvUavHeap->GetGPUDescriptorHandleForHeapStart(), srvIndex + threadIndex, m_srvUavDescriptorSize);
	//CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(m_srvUavHeap->GetGPUDescriptorHandleForHeapStart(), uavIndex + threadIndex, m_srvUavDescriptorSize);

	//pCommandList->SetComputeRootConstantBufferView(ComputeRootCBV, m_constantBufferCS->GetGPUVirtualAddress());
	//pCommandList->SetComputeRootDescriptorTable(ComputeRootSRVTable, srvHandle);
	//pCommandList->SetComputeRootDescriptorTable(ComputeRootUAVTable, uavHandle);
	commandListComputePass->SetDescriptorHeaps(1, &computeShaderResourceHeapUAV);
	commandListComputePass->SetComputeRootDescriptorTable(0, computeShaderResourceHeapUAV->GetGPUDescriptorHandleForHeapStart());


	CD3DX12_GPU_DESCRIPTOR_HANDLE depthDescriptor(computeShaderResourceHeapSRV->GetGPUDescriptorHandleForHeapStart());
	depthDescriptor.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	commandListComputePass->SetDescriptorHeaps(1, &computeShaderResourceHeapSRV);
	commandListComputePass->SetComputeRootDescriptorTable(1, depthDescriptor);

	commandListComputePass->SetComputeRootShaderResourceView(2, computeShaderResourceMeshes->GetGPUVirtualAddress());
	commandListComputePass->SetComputeRootConstantBufferView(3, computeShaderResourceFrameData->GetGPUVirtualAddress());
	commandListComputePass->SetComputeRootConstantBufferView(4, computeShaderResourceLightData->GetGPUVirtualAddress());
	commandListComputePass->Dispatch(80, 45, 1); // Threads matching a resolution of 1280x720 together with the partitioning in the compute shader
	commandListComputePass->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(computeShaderResourceOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	commandListComputePass->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, COMPUTE_TIME_END);
	commandListComputePass->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { commandListComputePass };
	computeQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);


}

void D3D12Wrapper::CopyDepthBuffer()
{
	HRESULT hr;

	//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	hr = commandQueue->Signal(prePassFence, fence);
	prePassFenceValue++;
	commandListPrePass->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthstencil, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE));
	commandListPrePass->CopyResource(computeShaderResourceInput, depthstencil);
	commandListPrePass->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthstencil, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

void D3D12Wrapper::RenderPrePass(EntityHandler* handler)
{
	const std::vector<Entity*> &entities = handler->GetEntityVector();

	commandAllocator->Reset();
	HRESULT hr = commandListPrePass->Reset(commandAllocator, nullptr);
	commandListPrePass->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, PREPASS_TIME_START);

	for (auto &textureJobs : handler->GetTextureJobs())
	{
		//Handle the job
		entities[textureJobs.entityID]->textureID = textureHandler->LoadTextureFromFile(textureJobs.fileName, commandListPrePass);

		commandListPrePass->Close();
		ID3D12CommandList* listsToExecute[] = { commandListPrePass };
		commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

		WaitForGPU();

		commandAllocator->Reset();
		HRESULT hr = commandListPrePass->Reset(commandAllocator, nullptr);
		WaitForGPU();

	}
	handler->GetTextureJobs().clear();

	//Set necessary states.
	commandListPrePass->RSSetViewports(1, &vp);
	commandListPrePass->RSSetScissorRects(1, &scissorRect);

	//Indicate that the back buffer will be used as render target.
	//SetResourceTransitionBarrier(commandListPrePass,
	//	renderTargets[frameIndex],
	//	D3D12_RESOURCE_STATE_PRESENT,		//state before
	//	D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	//);

	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(commandListPrePass,
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);

	ClearBuffer(commandListPrePass);

	pipelineHandler->SetPipelineState(prePassPipelineID, commandListPrePass);
	constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, commandListPrePass);
	constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, 1, 0, commandListPrePass);
	constantBufferHandler->UpdateBuffer(127, ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, vpStruct);
	constantBufferHandler->BindBuffer(127, ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, 0);

	for (auto &transformJobs : handler->GetTransformJobs())
	{
		/*Get the constant buffer that contains the transform job -> Update the buffer and bind it*/
		/*Fetch the data here*/
		Float4x4 finalMatrix;

		Matrix scaling = MatrixScaling(transformJobs.scale, transformJobs.scale, transformJobs.scale);
		Matrix translation = MatrixTranslation(transformJobs.position[0], transformJobs.position[1], transformJobs.position[2]);
		MatrixToFloat4x4(finalMatrix, MatrixTranspose(scaling * (MatrixRotationAroundAxis(VecCreate(1.0f, 0.0f, 0.0f, 0.0f), transformJobs.rotation[0])*MatrixRotationAroundAxis(VecCreate(0.0f, 1.0f, 0.0f, 0.0f), transformJobs.rotation[1])*MatrixRotationAroundAxis(VecCreate(0.0f, 0.0f, 1.0f, 0.0f), transformJobs.rotation[2]))*translation));

		constantBufferHandler->UpdateBuffer(transformJobs.entityID, ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, &finalMatrix);

		//Handle the job
	}
	handler->GetTransformJobs().clear();

	for (auto &meshJobs : handler->GetMeshJobs())
	{
		//Handle the job
		entities[meshJobs.entityID]->meshID = meshHandler->LoadMesh(meshJobs.fileName);
	}
	handler->GetMeshJobs().clear();

	for (auto &lightJobs : handler->GetLightJobs())
	{
		//HANDLE IT
	}

	for (auto &pipelineJobs : handler->GetPipelineJobs())
	{
		//Handle the job
	}

	int nrOfTriangles = 0;
	UINT vertexOffset = 0;
	D3D12_RANGE range = { 0, 0 };
	void* dataPtr;
	for (int i = 0; i < entities.size(); i++) // send all the mesh data to the compute shader
	{
		if (entities[i]->render)
		{

			RenderData rData = meshHandler->GetMeshAsRawData(entities[i]->meshID);

			if (vertexOffset + rData.nrOfIndices * VERTEXSIZE > HEAP_SIZE)
			{
				// in here we should take care of sending what we have so far, potentially we should send what we can from this mesh as well so as to maximize the usage
				// but for now just a breakpoint, thats an easy solution
				int a = 0;
				a++;
			}

			void* bufferData = rData.data;
			computeShaderResourceMeshes->Map(0, &range, &dataPtr);
			memcpy((char*)dataPtr + vertexOffset, bufferData, rData.size);
			computeShaderResourceMeshes->Unmap(0, nullptr);

			vertexOffset += rData.nrOfIndices * VERTEXSIZE;
			nrOfTriangles += rData.nrOfTriangles;
		}
	}
	void* dataPtr2;
	ViewProjectionStruct tempVP;
	ComputeShaderStruct uploadData = {};

	float temptest = DirectX::XMMatrixDeterminant(Float4x4ToMatrix(vpStruct->projectionMatrix)).m128_f32[0];
	DirectX::XMMATRIX testInvert = MatrixInvert(MatrixTranspose(Float4x4ToMatrix(vpStruct->projectionMatrix)));

	MatrixToFloat4x4(uploadData.revProjMat, MatrixTranspose(MatrixInvert(Float4x4ToMatrix(computeCamera.projectionMatrix))));
	//MatrixToFloat4x4(tempVP.viewMatrix, MatrixTranspose(MatrixInvert(MatrixTranspose(Float4x4ToMatrix(vpStruct->viewMatrix)))));
	//MatrixToFloat4x4(tempVP.viewMatrix, MatrixTranspose(MatrixInvert(MatrixTranspose(MatrixIdentity))));

	Matrix rotMatrix = MatrixRotationAroundAxis(VecCreate(0.0f, 1.0f, 0.0f, 0.0f), rotation);

	Vec lookDir = VecCreate(0.0f, 0.0f, 1.0f, 0.0f);
	lookDir = VecMultMatrix3D(lookDir, rotMatrix);

	Vec positionVec = VecCreate(camPos.x, camPos.y, camPos.z, 1.0f);
	MatrixToFloat4x4(uploadData.revViewMat, MatrixTranspose(MatrixInvert(MatrixViewLH(positionVec, VecAdd(lookDir, positionVec), VecCreate(0.0f, 1.0f, 0.0f, 0.0f)))));

	Float4x4 vpToSendUp;
	memcpy(&uploadData.viewMat, &vpStruct->viewMatrix, sizeof(Float4x4));

	uploadData.nrOfTriangles = nrOfTriangles;
	uploadData.camPos = camPos;
	uploadData.windowWidth = windowWidth;
	uploadData.windowHeight = windowHeight;
	uploadData.pad;

	computeShaderResourceFrameData->Map(0, &range, &dataPtr2);
	memcpy(dataPtr2, &uploadData, sizeof(uploadData));
	computeShaderResourceFrameData->Unmap(0, nullptr);

	void* dataPtr3;
	computeShaderResourceLightData->Map(0, &range, &dataPtr3);
	memcpy(dataPtr3, lightHandler->GatherLightJobs(), sizeof(LightHandler::PointLight) * lightHandler->GetNrOfActiveLights() + sizeof(int) * 4);
	computeShaderResourceLightData->Unmap(0, nullptr);


	vertexOffset = 0;
	UINT indexOffset = 0;
	ID3D12Resource* vUpload = meshHandler->GetVertexUploadBuffer();
	ID3D12Resource* iUpload = meshHandler->GetIndexUploadBuffer();
	void* dataBegin;
	UINT index = 0;
	for (int i = 0; i < entities.size(); i++) // handle pre pass rendering of meshes
	{
		if (entities[i]->render)
		{

			RenderData rData = meshHandler->GetMeshAsRawData(entities[i]->meshID);

			if (vertexOffset + rData.nrOfIndices * VERTEXSIZE > HEAP_SIZE)
			{
				// in here we should take care of sending what we have so far, potentially we should send what we can from this mesh as well so as to maximize the usage
				// but for now just a breakpoint, thats an easy solution
				int a = 0;
				a++;
			}

			void* bufferData = rData.data;
			vUpload->Map(0, &range, &dataBegin);
			memcpy((char*)dataBegin + vertexOffset, bufferData, rData.size);
			vUpload->Unmap(0, nullptr);

			rData.vBufferView.BufferLocation += vertexOffset;

			commandListPrePass->IASetVertexBuffers(0, 1, &rData.vBufferView);

			void* bufferData2 = rData.indexBuffer;
			void* dataBegin2;
			iUpload->Map(0, &range, &dataBegin2);
			memcpy((char*)dataBegin2 + indexOffset, bufferData2, rData.nrOfIndices * sizeof(UINT));
			iUpload->Unmap(0, nullptr);

			rData.iBufferView.BufferLocation += indexOffset;

			commandListPrePass->IASetIndexBuffer(&rData.iBufferView);

			//constantBufferHandler->BindBuffer(0/*REPLACE 0 with ID!*/, ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, index);
			constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, commandListPrePass);
			constantBufferHandler->BindBuffer(entities[i]->entityID, ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, index);
			constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, 0, index*device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), commandListPrePass);

			//commandList->DrawInstanced(rData.nrOfIndices, 1, 0, 0);
			commandListPrePass->DrawIndexedInstanced(rData.nrOfIndices, 1, 0, 0, 0);

			vertexOffset += rData.nrOfIndices * VERTEXSIZE;
			indexOffset += rData.nrOfIndices * sizeof(UINT);

			index++;
		}
	}

	//SetResourceTransitionBarrier(commandListPrePass,
	//	renderTargets[frameIndex],
	//	D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
	//	D3D12_RESOURCE_STATE_PRESENT		//state after
	//);

	CopyDepthBuffer();

	commandListPrePass->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, PREPASS_TIME_END);
	commandListPrePass->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { commandListPrePass };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

}

void D3D12Wrapper::RenderGeometryPass(EntityHandler * handler)
{
	const std::vector<Entity*> &entities = handler->GetEntityVector();

	commandAllocator->Reset();
	HRESULT hr = commandListGeometryPass->Reset(commandAllocator, nullptr);

	commandListGeometryPass->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, GEOMETRY_TIME_START);
	//Set necessary states.
	commandListGeometryPass->RSSetViewports(1, &vp);
	commandListGeometryPass->RSSetScissorRects(1, &scissorRect);


	//pipelineHandler->SetPipelineState(meshPipelineID, commandListGeometryPass);


	UINT index = 0;

	SetupMeshRendering();

	constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, commandListGeometryPass);
	constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, 1, 0, commandListGeometryPass);
	constantBufferHandler->UpdateBuffer(viewProjID, ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, vpStruct);
	constantBufferHandler->BindBuffer(viewProjID, ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, 0);
	

	D3D12_RANGE range = { 0, 0 };
	UINT vertexOffset = 0;
	UINT indexOffset = 0;
	ID3D12Resource* vUpload = meshHandler->GetVertexUploadBuffer();
	ID3D12Resource* iUpload = meshHandler->GetIndexUploadBuffer();
	void* dataBegin;
	
		index = 0;

		for (int i = 0; i < entities.size(); i++)
		{
			if (entities[i]->render)
			{

				RenderData rData = meshHandler->GetMeshAsRawData(entities[i]->meshID);

				if (vertexOffset + rData.nrOfIndices * VERTEXSIZE > HEAP_SIZE)
				{
					// in here we should take care of sending what we have so far, potentially we should send what we can from this mesh as well so as to maximize the usage
					// but for now just a breakpoint, thats an easy solution
					int a = 0;
					a++;
				}

				void* bufferData = rData.data;
				vUpload->Map(0, &range, &dataBegin);
				memcpy((char*)dataBegin + vertexOffset, bufferData, rData.size);
				vUpload->Unmap(0, nullptr);

				rData.vBufferView.BufferLocation += vertexOffset;

				commandListGeometryPass->IASetVertexBuffers(0, 1, &rData.vBufferView);

				//Should work, but is unnecessary

				void* bufferData2 = rData.indexBuffer;
				void* dataBegin2;
				iUpload->Map(0, &range, &dataBegin2);
				memcpy((char*)dataBegin2 + indexOffset, bufferData2, rData.nrOfIndices * sizeof(UINT));
				iUpload->Unmap(0, nullptr);

				rData.iBufferView.BufferLocation += indexOffset;

				commandListGeometryPass->IASetIndexBuffer(&rData.iBufferView);


				if (entities[i]->textureID != -1)
				{
					CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(textureHandler->GetTextureHeap()->GetGPUDescriptorHandleForHeapStart());
					texHandle.Offset(entities[i]->textureID, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

					commandListGeometryPass->SetDescriptorHeaps(1, &textureHandler->GetTextureHeap());
					commandListGeometryPass->SetGraphicsRootDescriptorTable(3, texHandle);
				}
				else
				{
					CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(textureHandler->GetTextureHeap()->GetGPUDescriptorHandleForHeapStart());

					commandListGeometryPass->SetDescriptorHeaps(1, nullptr);
					commandListGeometryPass->SetGraphicsRootDescriptorTable(3, texHandle);
				}

				CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(computeShaderResourceHeapSRV->GetGPUDescriptorHandleForHeapStart());

				commandListGeometryPass->SetDescriptorHeaps(1, &computeShaderResourceHeapSRV);
				commandListGeometryPass->SetGraphicsRootDescriptorTable(4, texHandle);

				//constantBufferHandler->BindBuffer(0/*REPLACE 0 with ID!*/, ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, index);
				constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, commandListGeometryPass);
				constantBufferHandler->BindBuffer(entities[i]->entityID, ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, index);
				constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, 0, index*device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), commandListGeometryPass);

				constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::PIXEL_SHADER_LIGHT_DATA, commandListGeometryPass);
				constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::PIXEL_SHADER_LIGHT_DATA, 2, 0, commandListGeometryPass);

				//commandList->DrawInstanced(rData.nrOfIndices, 1, 0, 0);
				commandListGeometryPass->DrawIndexedInstanced(rData.nrOfIndices, 1, 0, 0, 0);

				vertexOffset += rData.nrOfIndices * VERTEXSIZE;
				indexOffset += rData.nrOfIndices * sizeof(UINT);

				index++;
			}
		}
	
	//SetResourceTransitionBarrier(commandListGeometryPass,
	//	renderTargets[frameIndex],
	//	D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
	//	D3D12_RESOURCE_STATE_PRESENT		//state after
	//);

	FinishMeshRendering();

	commandListGeometryPass->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, GEOMETRY_TIME_END);
	commandListGeometryPass->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { commandListGeometryPass };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

}

void D3D12Wrapper::StartTimer()
{
	commandAllocator->Reset();
	HRESULT hr = commandList->Reset(commandAllocator, nullptr);

	commandList->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, TOTAL_TIME_START);

	commandList->Close();

	ID3D12CommandList* listsToExecute[] = { commandList };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

}

void D3D12Wrapper::EndTimer()
{

	commandAllocator->Reset();
	HRESULT hr = commandList->Reset(commandAllocator, nullptr);


	commandList->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, TOTAL_TIME_END);

	commandList->ResolveQueryData(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, 2 * NUM_TIME_STAMPS, heapData, 0);

	commandList->Close();

	ID3D12CommandList* listsToExecute[] = { commandList };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	WaitForGPU();


	D3D12_RANGE range = { 0, 0 };
	data = nullptr;
	
	UINT64 CPUCalibration[2];
	UINT64 GPUCalibration[2];


	commandQueue->GetTimestampFrequency(&timestampFrequency[0]);
	commandQueue->GetClockCalibration(&GPUCalibration[0], &CPUCalibration[0]);
	computeQueue->GetClockCalibration(&GPUCalibration[1], &CPUCalibration[1]);
	
	//Change this shit
	auto difference = GPUCalibration[1] - GPUCalibration[0] - (CPUCalibration[1] - CPUCalibration[0]);


	computeQueue->GetTimestampFrequency(&timestampFrequency[1]);
	heapData->Map(0, &range, (void**)&data);


	graphicsDeltaTime += (data->timeStamps[TOTAL_TIME].end - data->timeStamps[TOTAL_TIME].start) / (timestampFrequency[0]*1.0);
	prePassTime += (data->timeStamps[PREPASS_TIME].end - data->timeStamps[PREPASS_TIME].start) / (timestampFrequency[0]*1.0);
	computeTime += ((data->timeStamps[COMPUTE_TIME].end - data->timeStamps[COMPUTE_TIME].start)-difference) / (timestampFrequency[1]*1.0);
	geometryTime += (data->timeStamps[GEOMETRY_TIME].end - data->timeStamps[GEOMETRY_TIME].start) / (timestampFrequency[0]*1.0);
	lightTime += (data->timeStamps[LIGHT_TIME].end - data->timeStamps[LIGHT_TIME].start) / (timestampFrequency[0]*1.0);

	frames++;
	if ((frames % 100) == 0)
	{
		printf("\n\n\n");
		printf("Test Results for Frame %I64d:\n", frames);
		printf("GraphicsDeltaTime: %lf PrePassTime: %lf ComputeTime: %lf GeometryTime: %lf LightTime: %lf\n",
			(graphicsDeltaTime*1000.0) / frames, (prePassTime*1000.0) / frames, (computeTime*1000.0) / frames, (geometryTime*1000.0) / frames, (lightTime*1000.0) / frames);
		printf("\n");
		printf("Start PrePass: %I64d \t End PrePass: %I64d\n", data->timeStamps[PREPASS_TIME].start, data->timeStamps[PREPASS_TIME].end);
		printf("Start ComputePass: %I64d \t End ComputePass: %I64d\n", data->timeStamps[COMPUTE_TIME].start, data->timeStamps[COMPUTE_TIME].end);
		printf("Start GeometryPass: %I64d \t End GeometryPass: %I64d\n", data->timeStamps[GEOMETRY_TIME].start, data->timeStamps[GEOMETRY_TIME].end);
		printf("Start LightPass: %I64d \t End LightPass: %I64d\n", data->timeStamps[LIGHT_TIME].start, data->timeStamps[LIGHT_TIME].end);
		if (frames == NUM_FRAMES)
		{
			printf("Done");
			getchar();
			getchar();
			getchar();
		}
	}
	heapData->Unmap(0, &range);
}

void D3D12Wrapper::WaitForGPU()
{
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//This is code implemented as such for simplicity. The cpu could for example be used
	//for other tasks to prepare the next frame while the current one is being rendered.
	

	//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	commandQueue->Signal(this->fence, fence);
	fenceValue++;

	//Wait until command queue is done.
	if (this->fence->GetCompletedValue() < fence)
	{
		this->fence->SetEventOnCompletion(fence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
	}
	
}

void D3D12Wrapper::WaitForCompute()
{
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//This is code implemented as such for simplicity. The cpu could for example be used
	//for other tasks to prepare the next frame while the current one is being rendered.

	//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	computeQueue->Signal(this->fence, fence);
	fenceValue++;

	//Wait until command queue is done.
	if (this->fence->GetCompletedValue() < fence)
	{
		this->fence->SetEventOnCompletion(fence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
	}

}

void D3D12Wrapper::SetResourceTransitionBarrier(ID3D12GraphicsCommandList * commandList, ID3D12Resource * resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{

	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);

}

int D3D12Wrapper::initialize(HINSTANCE hInstance, int nCmdShow)
{
	InitializeWindow(hInstance, nCmdShow);	//1. Create Window

	CreateDirect3DDevice();					//2. Create Device
	CreateCommandInterfacesAndSwapChain();	//3. Create CommandQueue and SwapChain
	commandAllocator->Reset();
	HRESULT hr = commandList->Reset(commandAllocator, nullptr);
	CreateFenceAndEventHandle();			//4. Create Fence and Event handle
	CreateRenderTargets();					//5. Create render targets for backbuffer
	CreateViewportAndScissorRect();
	CreateDepthStencil();
	InitializePerformanceVariables();

	commandList->Close();
	ID3D12CommandList* listsToExecute[] = { commandList };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	WaitForGPU();

	return 0;
}

void D3D12Wrapper::Present()
{
	
	//Execute the command list.
	//ID3D12CommandList* listsToExecute[] = { commandList };
	//commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//Present the frame.
	swapChain->Present(0, 0);

	//Swap frame index for next frame. 
	frameIndex = (frameIndex + 1) % NUM_SWAP_BUFFERS;


}

int D3D12Wrapper::Shutdown()
{
	WaitForGPU();


	CloseHandle(eventHandle);
	SafeRelease(&device);
	SafeRelease(&commandQueue);
	SafeRelease(&computeQueue);
	SafeRelease(&commandAllocator);
	SafeRelease(&computeAllocator);
	SafeRelease(&commandList);
	SafeRelease(&swapChain);

	SafeRelease(&fence);

	SafeRelease(&renderTargetsHeap);
	for(int i = 0; i < 3; i++)
		SafeRelease(&GBuffers[i]);

	SafeRelease(&GBufferHeapRendering);
	SafeRelease(&GBufferHeapLightning);
	
	/*SafeRelease(&samplerHeap);*/
	//SafeRelease(&textureHeap);
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		SafeRelease(&renderTargets[i]);
	}

	/*SafeRelease(&constantBufferResource);
	SafeRelease(&CBDescriptorHeap);*/

	//SafeRelease(&constantDiffuseBufferResource);
	//SafeRelease(&DiffuseDescriptorHeap);

	SafeRelease(&depthStencileHeap);
	SafeRelease(&depthstencil);

	SafeRelease(&computeShaderResourceOutput);
	SafeRelease(&computeShaderResourceInput);
	SafeRelease(&computeShaderResourceHeapSRV);
	SafeRelease(&computeShaderResourceHeapUAV);
	SafeRelease(&computeShaderResourceMeshes);
	SafeRelease(&computeShaderResourceFrameData);
	SafeRelease(&computeShaderResourceLightData);
	SafeRelease(&prePassFence);

	SafeRelease(&commandListPrePass);
	SafeRelease(&commandListComputePass);
	SafeRelease(&commandListGeometryPass);
	SafeRelease(&commandListPostPass);

	SafeRelease(&queryHeap);
	SafeRelease(&heapData);

	return 0;
}

void D3D12Wrapper::ClearBuffer(ID3D12GraphicsCommandList* cmdList)
{
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += renderTargetDescriptorSize * frameIndex;

	cmdList->OMSetRenderTargets(1, &cdh, true, &depthStencileHeap->GetCPUDescriptorHandleForHeapStart());
	cmdList->ClearRenderTargetView(cdh, clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(depthStencileHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

}
