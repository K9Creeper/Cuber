#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <dwmapi.h>
#include "Structure.h"

LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;
LPD3DXFONT pFont;
LPD3DXFONT Arrow;
MARGINS  margin;

void initD3D()
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = global::overlay;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;//D3DFMT_X8R8G8B8;     // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = global::Game.size.x;    // set the width of the buffer
	d3dpp.BackBufferHeight = global::Game.size.y;    // set the height of the buffer

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// create a device class using this information and the info from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		global::overlay,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);
	D3DXCreateFont(d3ddev, 20, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &pFont);
	D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Ariall", &Arrow);
}
void DrawFilledBox(int x, int y, int w, int h, DWORD color)
{
	D3DRECT rect = { x, y, x + w, y + h };
	d3ddev->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, color, 0, 0);
}
void DrawLinedBox(int x, int y, int w, int h, int p, DWORD color)
{
	DrawFilledBox(x, y, w, p, color);

	DrawFilledBox(x, y, p, h, color);

	DrawFilledBox(x + w, y, p, h, color);

	DrawFilledBox(x, y + h, w + p, p, color);
}


void RENDER()
{

	int color1 = rand() % (255 - 1) + 1;
	int color2 = rand() % (255 - 1) + 1;
	int color3 = rand() % (255 - 1) + 1;
	// clear the window alpha
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();    // begins the 3D scene
	/*	
	
	*/
	
	DrawFilledBox(0, 0, 50, 50, D3DCOLOR_ARGB(255, 255,0,0));

	/*
	
	
	*/
	d3ddev->EndScene();    // ends the 3D scene
	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}