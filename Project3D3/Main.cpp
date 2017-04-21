#include <windows.h>

#include "D3D12Wrapper.h"
#include "EntityHandler.h"
#include "ConstantBuffer.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>

#define roomSize 10
unsigned int stefanRotValue = 0;
void CreateEntities(EntityHandler& handler);
std::vector<Entity*> entities;

Float3D cameraPos(0, 0, 0);
float cameraRot = 0;
void Update(D3D12Wrapper& graphics, EntityHandler & handler);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{

	MSG msg = { 0 };

	/*Open debug console*/
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	/*Create the graphics wrapper*/
	D3D12Wrapper graphics(hInstance, nCmdShow, 1280, 720);
	EntityHandler entityHandler;
	//CreateEntities(entityHandler);

	Entity* testEnt = entityHandler.CreateEntity();
	entityHandler.BindMesh(testEnt, "Jockes_Mystic_Cube.obj");
	//Entity* testEnt2 = entityHandler.CreateEntity();
	//entityHandler.BindMesh(testEnt2, "sphere.obj");
	entityHandler.BindTexture(testEnt, "dungeonTex.jpg");
	//entityHandler.BindTexture(testEnt2, "StefanMega.jpg");

	//float pos[3] = { 2.0f, 0.0f, 0.0f };
	//float rot[3] = { 0.0f, 0.0f, 0.0f };
	//entityHandler.SetTransform(testEnt, pos, rot, 5.0f);

	//pos[0] = -2.0f;
	//entityHandler.SetTransform(testEnt2, pos, rot, 0.5f);
	

	
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
			Update(graphics, entityHandler);
			graphics.Render(&entityHandler);
		}
	}

	FreeConsole();

}

void CreateEntities(EntityHandler & handler)
{
	Entity* wallsAndFloors[6];
	Entity* stefanTablet = handler.CreateEntity();

	float position[3] = { 0.0f, -3.0f, 0.0f };
	float rotation[3] = { 0.0f, 0.0f, 0.0f };

	handler.SetTransform(stefanTablet, position, rotation, 1.0f);
	handler.BindMesh(stefanTablet, "The_Mighty_Cube2.obj");
	handler.BindTexture(stefanTablet, "smileCeption.png");

	entities.push_back(stefanTablet);

	for (int i = 0; i < 6; i++)
	{
		wallsAndFloors[i] = handler.CreateEntity();
		float pos[3] = { 0.0f, 0.0f, 0.0f };
		float rot[3] = { 0.0f, 0.0f, 0.0f };

		if (i == 0)
		{
			pos[0] = -roomSize;
		}
		else if (i == 1)
		{
			pos[0] = roomSize;
		}
		else if (i == 2)
		{
			pos[1] = -roomSize;
		}
		else if (i == 3)
		{
			pos[1] = roomSize;
		}
		else if (i == 4)
		{
			pos[2] = -roomSize;
		}
		else if (i == 5)
		{
			pos[2] = roomSize;
		}

		handler.SetTransform(wallsAndFloors[i], pos, rot, roomSize);
		handler.BindMesh(wallsAndFloors[i], "The_Mighty_Cube2.obj");
		handler.BindTexture(wallsAndFloors[i], "dungeonTex.jpg");

		entities.push_back(wallsAndFloors[i]);
	}
}

void Update(D3D12Wrapper& graphics, EntityHandler & handler)
{
	if ((GetKeyState('A') & 0x80))
	{
		cameraPos.x -= 0.1f;
	}
	else if ((GetKeyState('D') & 0x80))
	{
		cameraPos.x += 0.1f;
	}
	
	if ((GetKeyState('W') & 0x80))
	{
		cameraPos.z += 0.1f;
	}
	else if ((GetKeyState('S') & 0x80))
	{
		cameraPos.z -= 0.1f;
	}

	if ((GetKeyState('U') & 0x80))
	{
		cameraPos.y += 0.1f;
	}
	else if ((GetKeyState('N') & 0x80))
	{
		cameraPos.y -= 0.1f;
	}

	if ((GetKeyState('U') & 0x80))
	{
		cameraPos.y += 0.1f;
	}
	else if ((GetKeyState('N') & 0x80))
	{
		cameraPos.y -= 0.1f;
	}

	if ((GetKeyState('Q') & 0x80))
	{
		cameraRot -= 0.1f;
	}
	else if ((GetKeyState('E') & 0x80))
	{
		cameraRot += 0.1f;
	}
	
	graphics.MoveCamera(cameraPos, cameraRot);

}