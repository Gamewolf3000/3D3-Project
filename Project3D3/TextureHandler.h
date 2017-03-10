#ifndef TEXTURE_HANDLER_H
#define TEXTURE_HANDLER_H
#pragma once
#include "JEXMath.h"

#include <d3d12.h>
#include <dxgi1_5.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include "DXTK12\WICTextureLoader.h"

#include <string>
#include <vector>

#include "CD3DX12Helper.h"

struct TextureData
{
	ID3D12Resource* textureResource = nullptr;
	ID3D12Resource* textureUploadHeap = nullptr;
	UINT size = 0;
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
};

class TextureHandler
{
private:
	ID3D12Device* device = nullptr;

	std::vector<TextureData> textureResources;
	ID3D12DescriptorHeap* textureHeap;

public:
	TextureHandler(ID3D12Device* dev);
	~TextureHandler();


	INT8 LoadTextureFromFile(std::string fileName, ID3D12GraphicsCommandList* commandList);



};

#endif
