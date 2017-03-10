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
	lightHandler = new LightHandler(512);
	meshHandler = new MeshHandler(device);
	textureHandler = new TextureHandler(device);

	auto sizes = ConstantBufferHandler::ConstantBufferSizes();
	sizes.VERTEX_SHADER_PER_OBJECT_DATA_SIZE = sizeof(ConstantBufferStruct);
	sizes.VERTEX_SHADER_PER_FRAME_DATA_SIZE = sizeof(ViewProjectionStruct);
	sizes.PIXEL_SHADER_LIGHT_DATA_SIZE = sizeof(LightHandler::PointLight);
	sizes.COMPUTE_LIGHT_DATA_SIZE = sizeof(float); /*To be edited!*/
	constantBufferHandler = new ConstantBufferHandler(sizes, 512, device);

	vpStruct = new ViewProjectionStruct;

	MatrixToFloat4x4(vpStruct->viewMatrix, MatrixTranspose(MatrixViewLH(VecCreate(0, 0, -10, 1), VecCreate(0, 0, 0, 1), VecCreate(0, 1, 0, 0))));
	MatrixToFloat4x4(vpStruct->projectionMatrix, MatrixTranspose(MatrixProjectionLH(JEX_PI/2, 1280.0/720.0, 0.1f, 100.0f)));

	//constantBufferHandler->CreateConstantBuffer(127, vpStruct, ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA);

	std::cout << "We think it worked to create the Wrapper. We don't check that." << std::endl;
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
	const std::vector<Entity*> &entities = handler->GetEntityVector();
	
	commandAllocator->Reset();
	HRESULT hr = commandList->Reset(commandAllocator, nullptr);

	for (auto &textureJobs : handler->GetTextureJobs())
	{
		//Handle the job
		entities[textureJobs.entityID]->textureID = textureHandler->LoadTextureFromFile(textureJobs.fileName, commandList);

		commandList->Close();
		ID3D12CommandList* listsToExecute[] = { commandList };
		commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

		WaitForGPU();

		commandAllocator->Reset();
		HRESULT hr = commandList->Reset(commandAllocator, nullptr);
		WaitForGPU();

	}
	handler->GetTextureJobs().clear();

	//Set necessary states.
	commandList->RSSetViewports(1, &vp);
	commandList->RSSetScissorRects(1, &scissorRect);

	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(commandList,
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);

	ClearBuffer();

	pipelineHandler->SetPipelineState(meshPipelineID, commandList);
	constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, commandList);
	constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, 1, 0, commandList);
	constantBufferHandler->UpdateBuffer(127, ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, vpStruct);
	constantBufferHandler->BindBuffer(127, ConstantBufferHandler::VERTEX_SHADER_PER_FRAME_DATA, 0);


	UINT index = 0;
	for (auto &transformJobs : handler->GetTransformJobs())
	{
		/*Get the constant buffer that contains the transform job -> Update the buffer and bind it*/
		/*Fetch the data here*/
		Float4x4 finalMatrix;

		Matrix translation = MatrixTranslation(transformJobs.position[0], transformJobs.position[1], transformJobs.position[2]);
		MatrixToFloat4x4(finalMatrix, (MatrixRotationAroundAxis(VecCreate(1.0f, 0.0f, 0.0f, 0.0f), transformJobs.rotation[0])*MatrixRotationAroundAxis(VecCreate(0.0f, 1.0f, 0.0f, 0.0f), transformJobs.rotation[1])*MatrixRotationAroundAxis(VecCreate(0.0f, 0.0f, 1.0f, 0.0f), transformJobs.rotation[2]))*translation);

		constantBufferHandler->UpdateBuffer(transformJobs.entityID, ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, &finalMatrix);

		//Handle the job
		index++;
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

			commandList->IASetVertexBuffers(0, 1, &rData.vBufferView);

			//Should work, but is unnecessary

			void* bufferData2 = rData.indexBuffer;
			void* dataBegin2;
			iUpload->Map(0, &range, &dataBegin2);
			memcpy((char*)dataBegin2 + indexOffset, bufferData2, rData.nrOfIndices * sizeof(UINT));
			iUpload->Unmap(0, nullptr);

			rData.iBufferView.BufferLocation += indexOffset;

			commandList->IASetIndexBuffer(&rData.iBufferView);


			if (entities[i]->textureID != -1)
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(textureHandler->GetTextureHeap()->GetGPUDescriptorHandleForHeapStart());
				texHandle.Offset(entities[i]->textureID, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	
				commandList->SetDescriptorHeaps(1, &textureHandler->GetTextureHeap());
				commandList->SetGraphicsRootDescriptorTable(3, texHandle);
			}
			else
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(textureHandler->GetTextureHeap()->GetGPUDescriptorHandleForHeapStart());

				commandList->SetDescriptorHeaps(1, nullptr);
				commandList->SetGraphicsRootDescriptorTable(3, texHandle);
			}

			constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, commandList);
			constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, 0, 0, commandList);
			constantBufferHandler->BindBuffer(entities[i]->entityID, ConstantBufferHandler::VERTEX_SHADER_PER_OBJECT_DATA, index);

			constantBufferHandler->SetDescriptorHeap(ConstantBufferHandler::PIXEL_SHADER_LIGHT_DATA, commandList);
			constantBufferHandler->SetGraphicsRoot(ConstantBufferHandler::PIXEL_SHADER_LIGHT_DATA, 2, 0, commandList);

			//commandList->DrawInstanced(rData.nrOfIndices, 1, 0, 0);
			commandList->DrawIndexedInstanced(rData.nrOfIndices, 1, 0, 0, 0);

			vertexOffset += rData.nrOfIndices * VERTEXSIZE;
			indexOffset += rData.nrOfIndices * sizeof(UINT);

			index++;
		}
	}

	//commandList->DrawInstanced(6, 1, 0, 0);

	SetResourceTransitionBarrier(commandList,
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	commandList->Close();

	Present();
}

void D3D12Wrapper::MoveCamera(Float3D position, float rotation)
{
	Vec lookDir = VecCreate(0, 0, 1, 0);
	Vec positionVec = VecCreate(position.x, position.y, position.z, 1.0f);

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
	InitializeWindow(hInstance, nCmdShow);	//1. Create Window

	CreateDirect3DDevice();					//2. Create Device
	CreateCommandInterfacesAndSwapChain();	//3. Create CommandQueue and SwapChain
	commandAllocator->Reset();
	HRESULT hr = commandList->Reset(commandAllocator, nullptr);
	CreateFenceAndEventHandle();			//4. Create Fence and Event handle
	CreateRenderTargets();					//5. Create render targets for backbuffer
	CreateViewportAndScissorRect();
	CreateDepthStencil();

	commandList->Close();
	ID3D12CommandList* listsToExecute[] = { commandList };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	WaitForGPU();

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
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += renderTargetDescriptorSize * frameIndex;

	commandList->OMSetRenderTargets(1, &cdh, true, &depthStencileHeap->GetCPUDescriptorHandleForHeapStart());
	commandList->ClearRenderTargetView(cdh, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(depthStencileHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

}
