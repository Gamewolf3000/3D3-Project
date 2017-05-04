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

#define NUM_FRAMES 1000
#define NUM_EXTRA_LOOPS 100
#define NUM_SWAP_BUFFERS 2
#define NUM_TIME_STAMPS 5

enum TimeStampIdentifiers
{
	TOTAL_TIME,
	LIGHT_TIME,
	COMPUTE_TIME,
	GEOMETRY_TIME,
	PREPASS_TIME
};

enum TimeStampStartEnd
{
	TOTAL_TIME_START,
	TOTAL_TIME_END,
	LIGHT_TIME_START,
	LIGHT_TIME_END,
	COMPUTE_TIME_START,
	COMPUTE_TIME_END,
	GEOMETRY_TIME_START,
	GEOMETRY_TIME_END,
	PREPASS_TIME_START,
	PREPASS_TIME_END
};

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

struct ComputeShaderStruct
{
	Float4x4 revProjMat;
	Float4x4 revViewMat;

	int nrOfTriangles;
	Float3D camPos;

	unsigned int windowWidth;
	unsigned int windowHeight;

	Float2D pad;

	Float4x4 viewMat;
};

struct MeshRelatedData
{
	Float4x4 worldMatrix;
	int nrOfTrianglesMeshHas;
	int padding[3] = { 3, 4, 5 };
};

class D3D12Wrapper
{
private:
	HWND window;

	UINT32 windowHeight;
	UINT32 windowWidth;

	ID3D12Device* device;
	ID3D12GraphicsCommandList* commandList;
	ID3D12GraphicsCommandList* commandListPrePass;
	ID3D12GraphicsCommandList* commandListComputePass;
	ID3D12GraphicsCommandList* commandListGeometryPass;
	ID3D12GraphicsCommandList* commandListPostPass;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12CommandAllocator* computeAllocator;
	ID3D12CommandQueue* computeQueue;
	ID3D12CommandQueue* commandQueue;

	IDXGISwapChain* swapChain;

	ID3D12Fence* fence;
	UINT64 fenceValue;
	HANDLE eventHandle;

	ID3D12Fence* prePassFence;
	UINT64 prePassFenceValue;
	HANDLE prePassEventHandle;

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
	ID3D12Resource* computeShaderResourceMeshes = nullptr;
	ID3D12Resource* computeShaderResourceFrameData = nullptr;
	ID3D12Resource* computeShaderResourceLightData = nullptr;

	Pipeline* pipelineHandler;
	MeshHandler* meshHandler;
	TextureHandler* textureHandler;
	LightHandler* lightHandler;
	UINT8 constantBufferID;
	UINT8 vpID;
	ConstantBufferStruct *cbStruct;
	ViewProjectionStruct *vpStruct;
	ViewProjectionStruct computeCamera;

	Float3D camPos;
	float rotation;

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

	void RenderPrePass(EntityHandler* handler);
	void RenderGeometryPass(EntityHandler* handler);

	void StartTimer();
	void EndTimer();

	void WaitForGPU();
	void WaitForCompute();

	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	int initialize(HINSTANCE hInstance, int nCmdShow);
	int Shutdown();

	void ClearBuffer(ID3D12GraphicsCommandList* cmdList);
	void Present();
	D3D12Wrapper();

	/*-----------------------------------------------------
	Timing stuff
	-------------------------------------------------------*/

	UINT64 timestampFrequency[2] = { 0,0 };
	ID3D12QueryHeap* queryHeap;
	ID3D12Resource* heapData;

	double graphicsDeltaTime = 0.0;;
	double prePassTime = 0.0;;
	double computeTime = 0.0;
	double geometryTime = 0.0;
	double lightTime = 0.0;
	UINT64 frames = 0;

	UINT64 CPUCalibration[2];
	UINT64 GPUCalibration[2];

	struct GraphicTimeStamps
	{
		struct times
		{
			UINT64 start;
			UINT64 end;
		} timeStamps[NUM_TIME_STAMPS];
	} *data;
	void InitializePerformanceVariables();


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
	UINT8 prePassPipelineID = -1;
	UINT8 computePipelineID = -1;
	bool renderPrePass = true;
	UINT8 viewProjID = 127;
	UINT8 camPosID = 126;
	UINT8 lightID = 125;
	
	
	ConstantBufferHandler *constantBufferHandler;

	void MoveCamera(Float3D position, float rotation);
	


};


#endif // !D3D12Wrapper
