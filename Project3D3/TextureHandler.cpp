#include "TextureHandler.h"

TextureHandler::TextureHandler(ID3D12Device * dev)
{
	HRESULT hr;
	device = dev;

	D3D12_DESCRIPTOR_HEAP_DESC textureDesc = {};
	textureDesc.NumDescriptors = 1;
	textureDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	textureDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = device->CreateDescriptorHeap(&textureDesc, IID_PPV_ARGS(&textureHeap));
}

TextureHandler::~TextureHandler()
{
	for (auto i : textureResources)
	{
		i.textureResource->Release();
		i.textureUploadHeap->Release();
	}

	textureHeap->Release();
}

INT8 TextureHandler::LoadTextureFromFile(std::string fileName, ID3D12GraphicsCommandList* commandList)
{
	HRESULT hr;
	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subresource;

	std::wstring convertedString = std::wstring(fileName.begin(), fileName.end());
	ID3D12Resource* wicTemp = nullptr;
	ID3D12Resource* texture = nullptr;
	ID3D12Resource* upload = nullptr;

	hr = DirectX::LoadWICTextureFromFile(device, convertedString.c_str(), &wicTemp, decodedData, subresource);

	int sizeOfTexture = GetRequiredIntermediateSize(wicTemp, 0, 1);
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = wicTemp->GetDesc().Format;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = wicTemp->GetDesc().MipLevels;
	desc.Texture2D.ResourceMinLODClamp = 0.0f;

	//----------------------------------------------------------------------------------------------------

	D3D12_RESOURCE_DESC texDesc = wicTemp->GetDesc();

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&texture)
	);

	if (FAILED(hr))
	{
		// THINGS GONE HORRIBLY WRONG, PLEASE SEND HELP
	}
	else
	{
		const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture, 0, num2DSubresources);

		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&upload));
		if (FAILED(hr))
		{
			//SOMETHING WENT HORRIBLY WRONG HERE AS WELL, PLEASE SEND MORE HELP, OR DONUTS AT LEAST
		}
		else
		{
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture,
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

			// Use Heap-allocating UpdateSubresources implementation for variable number of subresources (which is the case for textures).
			UpdateSubresources(commandList, texture, upload, 0, 0, num2DSubresources, &subresource);

			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture,
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}
	}

	texture->SetName(std::wstring(L"Texture " + convertedString).c_str());

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(textureHeap->GetCPUDescriptorHandleForHeapStart());
	
	device->CreateShaderResourceView(texture, &desc, hDescriptor);

	TextureData data;
	data.desc = desc;
	data.size = sizeOfTexture;
	data.textureResource = texture;
	data.textureUploadHeap = upload;

	textureResources.push_back(data);

	wicTemp->Release();

	//commandList->Close();

	return textureResources.size() - 1;
}