#pragma once
#ifndef D3D12Wrapper

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_5.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>
#include "JEXMath.h"
#include "TimerClass.h"

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include "CD3DX12Helper.h"
#include "Pipeline.h"
#include "EntityHandler.h"
#include "ConstantBuffer.h"
#include "MeshHandler.h"
#include "TextureHandler.h"
#include "LightHandler.h"

#include <sstream>

#define NUM_SWAP_BUFFERS 2

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

struct ConstantBufferStruct
{
	Float4x4 worldMatrix;
};

struct ViewProjectionStruct
{
	Float4x4 projectionMatrix;
	Float4x4 viewMatrix;
};

class D3D12Wrapper
{
private:
	HWND window;

	UINT16 windowHeight;
	UINT16 windowWidth;

	ID3D12Device* device;
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
	ID3D12DescriptorHeap* computeShaderResourceHeapSRV = nullptr;
	ID3D12DescriptorHeap* computeShaderResourceHeapUAV = nullptr;
	ID3D12Resource* computeShaderResourceOutput = nullptr;
	ID3D12Resource* computeShaderResourceInput = nullptr;

	Pipeline* pipelineHandler;
	MeshHandler* meshHandler;
	TextureHandler* textureHandler;
	LightHandler* lightHandler;
	UINT8 constantBufferID;
	UINT8 vpID;
	ConstantBufferStruct *cbStruct;
	ViewProjectionStruct *vpStruct;
	Float3D camPos;

	D3D12_VIEWPORT vp;
	D3D12_RECT scissorRect;

	TimerClass* timer;

	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	int frameIndex = 0;

	void InitializeWindow(HINSTANCE hInstance, int nCmdShow);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void CreateDirect3DDevice();
	void CreateCommandInterfacesAndSwapChain();
	void CreateFenceAndEventHandle();
	void CreateRenderTargets();
	void CreateViewportAndScissorRect();
	void CreateDepthStencil();
	void SetupComputeShader();

	void CreatePipelines();
	void DisplayFps();
	void DispatchComputeShader();
	void CopyDepthBuffer();

	void WaitForGPU();

	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	int initialize(HINSTANCE hInstance, int nCmdShow);
	int Shutdown();

	void ClearBuffer();
	void Present();
	D3D12Wrapper();


	/*-----------------------------------------------------
	Deferred Rendering Specifics Starts Right Here
	-------------------------------------------------------*/

	/*Variables*/
	UINT8 deferredPipelineID[2] = { (UINT8)-1, (UINT8)-1 };
	ID3D12DescriptorHeap* GBufferHeapRendering;
	ID3D12DescriptorHeap* GBufferHeapLightning;
	ID3D12Resource* GBuffers[3] = { nullptr, nullptr, nullptr };
	enum GBuffers {
		GBUFFER_NORMAL,
		GBUFFER_COLOUR,
		GBUFFER_POS
	};

	/*Functions*/
	void InitializeDeferredRendering();
	void SetupMeshRendering();
	void FinishMeshRendering();
	void LightPass();


public:
	D3D12Wrapper(HINSTANCE hInstance, int nCmdShow, UINT16 width, UINT16 height);
	~D3D12Wrapper();

	void Render(EntityHandler* handler);

	UINT8 testPipelineID = -1;
	UINT8 meshPipelineID = -1;
	UINT8 computePipelineID = -1;
	UINT8 viewProjID = 127;
	UINT8 camPosID = 126;
	UINT8 lightID = 125;
	
	
	ConstantBufferHandler *constantBufferHandler;

	void MoveCamera(Float3D position, float rotation);
	


};


#endif // !D3D12Wrapper
