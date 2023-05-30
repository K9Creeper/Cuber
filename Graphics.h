#pragma once
#include <dwmapi.h>
#include "Structure.h"

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
	D3DXCreateFont(d3ddev, 20, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &pFontM);
	D3DXCreateFont(d3ddev, 10, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &pFontS);
	D3DXCreateFont(d3ddev, 30, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH| FF_DONTCARE, L"Arial", &pFontL);
	D3DXCreateFont(d3ddev, 10, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH| FF_DONTCARE, L"Ariall", &Arrow);
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
void DrawString(int x, int y, DWORD color, LPD3DXFONT g_pFont, const char* fmt)
{
	RECT FontPos = { x, y, x + 120, y + 16 };
	g_pFont->DrawTextA(NULL, fmt, -1, &FontPos, DT_NOCLIP, color);
}
void RENDER()
{
	int color1 = rand() % (255 - 1) + 1;
	int color2 = rand() % (255 - 1) + 1;
	int color3 = rand() % (255 - 1) + 1;
	// clear the window alpha
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();
	
	if (global::Game.focused)
	{
		/*



		*/
		/*HACKS*/
		std::vector<Entity*>* List = Entity_List();
		for (Entity* Ent : *List)
		{
			if (global::Esp && Ent->Team_Number != global::player->Team_Number)
			{
				Vector3 low = Ent->Position;
				Ent->Position.z += 5.5f;
				low.z -= .4f;
				Vector3 wts = _WorldToScreen(Ent->Position, global::player->view_matrix);
				Vector3 wtsl = _WorldToScreen(low, global::player->view_matrix);
				if (wts.x <= global::Game.size.x && wts.x >= 0 && wts.y <= global::Game.size.y && wts.y >= 0 && wts.z != 0.1f && Ent->Health > 0 && global::player->Health > 0)
				{
					int height = abs(wts.y - wtsl.y);
					int width = height / 1.7f;
					
					DrawFilledBox(wts.x - (width / 2), wts.y, width, 1, D3DCOLOR_ARGB(255, 255, 0, 150));
					DrawFilledBox(wts.x - (width / 2), wtsl.y, width, 1, D3DCOLOR_ARGB(255, 255, 0, 150));
					DrawFilledBox(wts.x - (width / 2), wts.y, 1, height + 1, D3DCOLOR_ARGB(255, 255, 0, 150));
					DrawFilledBox(wts.x + (width / 2), wts.y, 1, height + 1, D3DCOLOR_ARGB(255, 255, 0, 150));
					/*
					DRAW INFO
					*/
					std::string s = "Health: " + std::to_string(Ent->Health) + " / 100  Armour: " + std::to_string(Ent->Armour);
					std::string m = "Team #: " + std::to_string(Ent->Team_Number);
					std::string p = "Position:   X: " + std::to_string(Ent->Position.x) + " Y: " + std::to_string(Ent->Position.y) + " Z: " + std::to_string(Ent->Position.z);
					std::string w = "Weapon Equiped: " + (std::string)Ent->Equipped.weapon_name + " " + std::to_string(Ent->Equipped.weapon_ammo.loaded) + " / " + std::to_string(Ent->Equipped.weapon_ammo.inv);
					std::vector<const char*> stuf = { Ent->Name, s.c_str(), m.c_str(), p.c_str(), w.c_str() };
					auto font = pFontM;
					int diffs = 14;
					if (width >= 40)
					{
						font = pFontL;
						diffs = 24;
					}
					else if (width <= 15)
					{
						font = pFontS;
						diffs = 9;
					}
					for (int i = 0; i < stuf.size(); i++)
						DrawString(wts.x - (width * 2), wtsl.y + 3 + (diffs*i), D3DCOLOR_ARGB(255, 255, 0, 150), font, stuf.at(i));
				}
			}
				delete Ent;
		}
		delete List;
		/*     */
		/*  MENU  */
		menu->DrawMenu();

		/*          */
		/*



		*/
	}
	
	d3ddev->EndScene();    // ends the 3D scene
	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}