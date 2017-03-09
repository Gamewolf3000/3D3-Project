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

	Entity* testEnt2 = entityHandler.CreateEntity();
	entityHandler.BindMesh(testEnt2, "sphere.obj");
	Entity* testEnt = entityHandler.CreateEntity();
	entityHandler.BindMesh(testEnt, "The_Mighty_Cube2.obj");

	
	/*Looping the shit out of it*/
	while (WM_QUIT != msg.message && !(GetKeyState(VK_ESCAPE) & 0x80))
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			graphics.Render(&entityHandler);
		}
	}

	FreeConsole();

}