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
#include "Pipeline.h"
#include "EntityHandler.h"

#define NUM_SWAP_BUFFERS 2
#define NUM_OBJECTS_TO_RENDER_BATCH 500

template<class Interface>
inline void SafeRelease(
	Interface **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

class D3D12Wrapper
{
private:
	HWND window;

	UINT16 windowHeight;
	UINT16 windowWidth;

	//ID3D12Device* device;
	ID3D12GraphicsCommandList* commandList;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12CommandQueue* commandQueue;

	IDXGISwapChain* swapChain;

	ID3D12Fence* fence;
	UINT64 fenceValue;
	HANDLE eventHandle;

	ID3D12DescriptorHeap* renderTargetsHeap;

	void* textureBufferCPU_mappedPtr;
	UINT renderTargetDescriptorSize;
	ID3D12Resource* renderTargets[NUM_SWAP_BUFFERS];
	ID3D12DescriptorHeap* depthStencileHeap = nullptr;
	ID3D12Resource* depthstencil = nullptr;

	Pipeline* pipelineHandler;

	//maybe
	ID3D12DescriptorHeap* samplerHeap;
	ID3D12DescriptorHeap* textureHeap;
	ID3D12Resource* bufferResource = nullptr;
	ID3D12Resource* textureResource = nullptr;
	ID3D12DescriptorHeap* CBDescriptorHeap;
	ID3D12Resource* constantBufferResource;
	void* constantBufferCPU_mappedPtr;

	D3D12_VIEWPORT vp;
	D3D12_RECT scissorRect;


	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	int frameIndex = 0;

	void InitializeWindow(HINSTANCE hInstance, int nCmdShow);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void CreateDirect3DDevice();
	void CreateCommandInterfacesAndSwapChain();
	void CreateFenceAndEventHandle();
	void CreateRenderTargets();
	void CreateViewportAndScissorRect();
	void CreateRenderHeap();
	//void CreateConstantBufferHeap();
	void CreateDepthStencil();

	void WaitForGPU();

	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	int initialize(HINSTANCE hInstance, int nCmdShow);
	int Shutdown();

	void ClearBuffer();
	void Present();
	D3D12Wrapper();

public:
	D3D12Wrapper(HINSTANCE hInstance, int nCmdShow, UINT16 width, UINT16 height);
	~D3D12Wrapper();

	void Render(EntityHandler* handler);
	ID3D12Device* device;

	UINT8 testPipelineID = -1;
	


};


#endif // !D3D12Wrapper
