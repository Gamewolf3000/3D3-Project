#include <windows.h>

#include "D3D12Wrapper.h"
#include "EntityHandler.h"
#include "ConstantBuffer.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>

Float3D cameraPos(0, 0, -10);
float cameraRot = 0;
void Update(D3D12Wrapper& graphics);

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

	Entity* testEnt = entityHandler.CreateEntity();
	entityHandler.BindMesh(testEnt, "The_Mighty_Cube2.obj");
	Entity* testEnt2 = entityHandler.CreateEntity();
	entityHandler.BindMesh(testEnt2, "sphere.obj");
	entityHandler.BindTexture(testEnt, "smileCeption.png");
	entityHandler.BindTexture(testEnt2, "StefanMega.jpg");

	float pos[3] = { 1000.0f, 0.0f, 0.0f };
	float rot[3] = { 0.0f, 0.0f, 0.0f };

	entityHandler.SetTransform(testEnt, pos, rot);
	

	
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
			Update(graphics);
			graphics.Render(&entityHandler);
		}
	}

	FreeConsole();

}

void Update(D3D12Wrapper& graphics)
{
	if ((GetKeyState('A') & 0x80))
	{
		cameraPos.x -= 0.01f;
	}
	else if ((GetKeyState('D') & 0x80))
	{
		cameraPos.x += 0.01f;
	}
	
	if ((GetKeyState('W') & 0x80))
	{
		cameraPos.z += 0.01f;
	}
	else if ((GetKeyState('S') & 0x80))
	{
		cameraPos.z -= 0.01f;
	}

	if ((GetKeyState('Q') & 0x80))
	{
		cameraRot -= 0.0015f;
	}
	else if ((GetKeyState('E') & 0x80))
	{
		cameraRot += 0.0015f;
	}

	graphics.MoveCamera(cameraPos, cameraRot);
}