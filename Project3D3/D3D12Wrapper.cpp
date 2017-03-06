#include "D3D12Wrapper.h"
#include <iostream>

D3D12Wrapper::D3D12Wrapper(HINSTANCE hInstance, int nCmdShow, UINT16 width, UINT16 height)
{
	CoInitialize(NULL);

	windowHeight = height;
	windowWidth = width;
	
	initialize(hInstance, nCmdShow);
	std::cout << "I think it worked to create the Wrapper. We don't check that." << std::endl;
}

D3D12Wrapper::~D3D12Wrapper()
{
	Shutdown();
	int fuckingWork;
	fuckingWork = true;
}

void D3D12Wrapper::Render(EntityHandler* handler)
{
	ClearBuffer();

	for (auto transformJobs : handler->GetTransformJobs())
	{
		//Handle the job
	}

	for (auto meshJobs : handler->GetMeshJobs())
	{
		//Handle the job
	}

	for (auto textureJobs : handler->GetTextureJobs())
	{
		//Handle the job
	}

	for (auto lightJobs : handler->GetLightJobs())
	{
		//Handle the job
	}

	for (auto pipelineJobs : handler->GetPipelineJobs())
	{
		//Handle the job
	}

	for (auto entities : handler->GetEntityVector())
	{
		//get the mesh, texture, pos and other things in here and set them and stuff
	}

	SetResourceTransitionBarrier(commandList,
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	commandList->Close();

	Present();
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

	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&commandQueue));

	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	//Create command list.
	device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator,
		nullptr,
		IID_PPV_ARGS(&commandList));

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	commandList->Close();

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

	HRESULT hr = factory->CreateSwapChainForHwnd(
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

void D3D12Wrapper::CreateRenderHeap()
{
	if (bufferResource)
		bufferResource->Release();

	//Note: using upload heaps to transfer static data like vert buffers is not 
	//recommended. Every time the GPU needs it, the upload heap will be marshalled 
	//over. Please read up on Default Heap usage. An upload heap is used here for 
	//code simplicity and because there are very few vertices to actually transfer.
	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = NUM_OBJECTS_TO_RENDER_BATCH * 3 * 4;;
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
		IID_PPV_ARGS(&bufferResource));

	bufferResource->SetName(L"vb heap");
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
	InitializeWindow(hInstance, nCmdShow);					//1. Create Window

	CreateDirect3DDevice();					//2. Create Device
	CreateCommandInterfacesAndSwapChain();	//3. Create CommandQueue and SwapChain
	commandAllocator->Reset();
	HRESULT hr = commandList->Reset(commandAllocator, nullptr);
	CreateFenceAndEventHandle();			//4. Create Fence and Event handle
	CreateRenderTargets();					//5. Create render targets for backbuffer
	CreateViewportAndScissorRect();
	CreateRenderHeap();
	//CreateConstantBufferHeap();
	CreateDepthStencil();

	commandList->Close();
	ID3D12CommandList* listsToExecute[] = { commandList };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	WaitForGPU();
	commandAllocator->Reset();
	hr = commandList->Reset(commandAllocator, nullptr);



	return 0;
}

void D3D12Wrapper::Present()
{
	
	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { commandList };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//Present the frame.
	swapChain->Present(0, 0);
	WaitForGPU();

	//Swap frame index for next frame.
	frameIndex = (frameIndex + 1) % NUM_SWAP_BUFFERS;


}

int D3D12Wrapper::Shutdown()
{
	WaitForGPU();


	CloseHandle(eventHandle);
	SafeRelease(&device);
	SafeRelease(&commandQueue);
	SafeRelease(&commandAllocator);
	SafeRelease(&commandList);
	SafeRelease(&swapChain);

	SafeRelease(&fence);

	SafeRelease(&renderTargetsHeap);
	/*SafeRelease(&samplerHeap);
	SafeRelease(&textureHeap);*/
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		SafeRelease(&renderTargets[i]);
	}

	SafeRelease(&bufferResource);
	SafeRelease(&textureResource);

	/*SafeRelease(&constantBufferResource);
	SafeRelease(&CBDescriptorHeap);*/

	//SafeRelease(&constantDiffuseBufferResource);
	//SafeRelease(&DiffuseDescriptorHeap);

	SafeRelease(&depthStencileHeap);
	SafeRelease(&depthstencil);

	return 0;
}

void D3D12Wrapper::ClearBuffer()
{
	/*First to be called*/
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += renderTargetDescriptorSize * frameIndex;

	commandAllocator->Reset();
	HRESULT hr = commandList->Reset(commandAllocator, nullptr);

	commandList->ClearRenderTargetView(cdh, clearColor, 0, nullptr);

}
