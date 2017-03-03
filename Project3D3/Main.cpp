#include <windows.h>

#include "D3D12Wrapper.h"
#include "EntityHandler.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	
	MSG msg = { 0 };
	
	/*Open debug console*/
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	/*Create the graphics wrapper*/
	D3D12Wrapper graphics(hInstance, nCmdShow, 1280, 720);
	EntityHandler entityHandler;


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