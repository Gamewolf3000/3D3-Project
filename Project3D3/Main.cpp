#include <windows.h>

#include "D3D12Wrapper.h"
#include "EntityHandler.h"
#include "ConstantBuffer.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	
	MSG msg = { 0 };
	
	/*Open debug console*/
	AllocConsole();
	FILE* fp;
	freopen_s(&fp,"CONOUT$", "w", stdout);

	/*Create the graphics wrapper*/
	D3D12Wrapper graphics(hInstance, nCmdShow, 1280, 720);
	EntityHandler entityHandler;
	ConstantBufferHandler::ConstantBufferSizes sizes;
	sizes.COMPUTE_LIGHT_DATA_SIZE = (5+255) & ~255;
	sizes.VERTEX_SHADER_PER_FRAME_DATA_SIZE = (5 + 255) & ~255;
	sizes.VERTEX_SHADER_PER_OBJECT_DATA_SIZE = (5 + 255) & ~255;
	ConstantBufferHandler cbH(sizes, 512U, graphics.device);

	float* test = new float(2.5f);

	cbH.CreateConstantBuffer(test, sizeof(float), ConstantBufferHandler::ConstantBufferType::VERTEX_SHADER_PER_FRAME_DATA);

	/*Looping the shit out of it*/
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//Update();
			graphics.Render(&entityHandler);
		}
	}

	FreeConsole();

}