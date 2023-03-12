// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>

#include <functional>
#include <thread>
#include "Graphics.h"
constexpr static int bordersize = 26;
constexpr static int bordersizex = 2;
Vector2 GetWindowPos(HWND hwnd) {
    RECT rect = { NULL };
    if (GetWindowRect(hwnd, &rect))
        return { static_cast<float>(rect.left), static_cast<float>(rect.top) };
}
Vector3 _WorldToScreen(Vector3 pos, View_Matrix matrix) {
    Vector3 screen;
    Vector4 clipCoords;
    clipCoords.x = pos.x * matrix.matrix[0] + pos.y * matrix.matrix[4] + pos.z * matrix.matrix[8] + matrix.matrix[12];
    clipCoords.y = pos.x * matrix.matrix[1] + pos.y * matrix.matrix[5] + pos.z * matrix.matrix[9] + matrix.matrix[13];
    clipCoords.z = pos.x * matrix.matrix[2] + pos.y * matrix.matrix[6] + pos.z * matrix.matrix[10] + matrix.matrix[14];
    clipCoords.w = pos.x * matrix.matrix[3] + pos.y * matrix.matrix[7] + pos.z * matrix.matrix[11] + matrix.matrix[15];

    if (clipCoords.w < 0.1f)
        screen.z = 0.1f;

    Vector3 NDC;
    NDC.x = clipCoords.x / clipCoords.w;
    NDC.y = clipCoords.y / clipCoords.w;
    NDC.z = clipCoords.z / clipCoords.w;
    
    screen.x = (global::Game.size.x / 2 * NDC.x) + (NDC.x + global::Game.size.x / 2);
    screen.y = -(global::Game.size.y / 2 * NDC.y) + (NDC.y + global::Game.size.y / 2);
    return screen;
}

Vector2 CalcAngle(Vector3 pos, Vector3 dst);
Entity* Entity_Inti(DWORD BASE, int i)
{
    Entity* Ent = new Entity;
        for (int i = 0; i < 16; i++)
            Ent->Name[i] = Read<char>(BASE + 0x225 + i); //Inti player names
        Ent->place_number = i;
    Ent->Health = max(Read<int>(BASE + 0xF8), 0);
    Ent->Armour = Read<int>(BASE + 0xFC);
    Ent->Team_Number = Read<int>(BASE + 0x32C);
    Ent->Position = Read<Vector3>(BASE + 0x34);//Beginning of x
    Ent->AngleToAimbot = CalcAngle(global::player->Position, Ent->Position);
    for (int i = 0; i < 16; i++)
        Ent->Equipped.weapon_name[i] = Read<char>((Read<DWORD>(Read<DWORD>(BASE + 0x374) + 0x0C)+0x0) + i);//Pointer->Pointer->int
    Ent->Equipped.weapon_ammo.loaded = Read<int>(Read<DWORD>(Read<DWORD>(BASE + 0x374) + 0x14));
    Ent->Equipped.weapon_ammo.inv = Read<int>(Read<DWORD>(Read<DWORD>(BASE + 0x374) + 0x10));
    return Ent;
}

void Player_Inti()
{
    for (int i = 0; i < 16; i++)
    {
        global::player->Name[i] = Read<char>(global::player_p + 0x225 + i); //Inti player names
        global::player->Last_Target[i] = Read<char>(global::client + 0x101C38 + i);
    }
    global::player->view_matrix = Read<View_Matrix>(0x501AE8);
    global::player->view_angles = Read<Vector2>(global::player_p + 0x40);
    global::player->Health = max(Read<int>(global::player_p + 0xF8), 0);
    global::player->Armour = Read<int>(global::player_p + 0xFC);
    global::player->Team_Number = Read<int>(global::player_p + 0x32C);
    global::player->Position = Read<Vector3>(global::player_p + 0x34);//Beginning of x
    for (int i = 0; i < 16; i++)
        global::player->Equipped.weapon_name[i] = Read<char>((Read<DWORD>(Read<DWORD>(global::player_p + 0x374) + 0x0C) + 0x0) + i);//Pointer->Pointer->int
    global::player->Equipped.weapon_ammo.loaded = Read<int>(Read<DWORD>(Read<DWORD>(global::player_p + 0x374) + 0x14));
    global::player->Equipped.weapon_ammo.inv = Read<int>(Read<DWORD>(Read<DWORD>(global::player_p + 0x374) + 0x10));
}
std::vector<Entity*>* Entity_List()
{
    std::vector<Entity*>* List = new std::vector<Entity*>;
    for (int i = 1; i < global::player_count; i++)
        List->push_back(Entity_Inti(Read<DWORD>(global::entity_list + (i * 4)), i));
    return List;
 }
class Entity_Specific {
public:
    bool Get_Pos_Of_S_Ent(int p, Vector3* Pos)
    {
        auto L = Entity_List();
        bool V = false;
        for (auto S : *L)
        {
            if (S->place_number == p)
            {
                V = true;
                *Pos = S->Position;
            }
            delete S;
        }
        delete L;
        return V;
    }
};

Vector2 CalcAngle(Vector3 pos, Vector3 dst)
{
    float turnX = static_cast<float>(atan((pos.x - dst.x) / (pos.y - dst.y)) * (180 / 3.14159f));
    float turnY = static_cast<float>(-atan((pos.z - dst.z) / sqrt(pow(pos.y - dst.y, 2) + pow(pos.x - dst.x, 2))) * (180 / 3.14159f));
    turnX = 360 - turnX;

    /*Flip*/
    if (pos.y < dst.y)
        turnX = turnX - 180;
   
    /*Convert*/
    if (turnX > 360)
        turnX -= 360;
    else if (turnX < 0)
        turnX += 360;

    return { turnX ,turnY };
}

void Teleport(DWORD Entity_Base, Vector3 Pos)
{
    Write<Vector3>(Entity_Base + 0x34, Pos);
}
void Thread_Control()
{
    while (global::THREAD_ON)
    {
        if (GetKeyState(VK_NUMPAD2) & 0x8000)
        {
            global::selected++;
            if (global::selected >= global::player_count)
                global::selected = 1;
            Sleep(170);
        }
        else if (GetKeyState(VK_NUMPAD8) & 0x8000)
        {
            global::selected--;
            if (global::selected <= 0)
                global::selected = global::player_count - 1;
            Sleep(170);
        }
        else if (GetKeyState(VK_NUMPAD9) & 0x8000)
        {
            Entity_Specific* s = new Entity_Specific();
            Vector3 P;
            if (s->Get_Pos_Of_S_Ent(global::selected, &P))
                Teleport(global::player_p, P);
            delete s;
            Sleep(170);
        }
        else if (GetKeyState(VK_NUMPAD7) & 0x8000)
        {
            global::Show_Info = !global::Show_Info;
        }
        else if (GetKeyState(VK_NUMPAD5) & 0x8000)
        {
            global::AimBot = !global::AimBot;
            Sleep(170);
        }
        else if (GetKeyState(VK_NUMPAD4) & 0x8000)
        {
            global::Esp = !global::Esp;
            Sleep(170);
        }
        Sleep(5);
    }
}
void Hack_Thread()
{
    while (global::THREAD_ON)
    {
        global::Game.size = { static_cast<float>(Read<int>(global::client + 0x110C94)) ,static_cast<float>(Read<int>(global::client + 0x110C98)) };
        global::Game.position = GetWindowPos(global::Game.hwnd);  
        global::Game.position.y += bordersize;
        global::Game.position.x += bordersizex;
        global::player_count = Read<int>(global::client + 0x10F500);
        margin = { 0, 0, static_cast<int>(global::Game.size.x), static_cast<int>(global::Game.size.y) };
        global::Game.focused = (GetForegroundWindow() == global::Game.hwnd);
        Player_Inti();
        std::vector<Entity*>* List = Entity_List();
        for (Entity* Ent : *List)
        {
            if (Ent->place_number == global::selected)
            {
                /*   Aim Bot   */
                if (global::AimBot && Ent->Health != 0 && global::player->Health > 0)
                {
                    if(!(global::player->Position.x == Ent->Position.x && global::player->Position.y == Ent->Position.y && global::player->Position.z == Ent->Position.z))
                    Write<Vector2>(global::player_p + 0x40, Ent->AngleToAimbot);
                }
                /*           */
            }
            delete Ent;
        }
        delete List;
        Sleep(1);
    }
}
DWORD WINAPI ThreadProc();
void Thread()
{
    global::client = (DWORD)GetModuleHandle(global::ac_client);
    global::entity_list = Read<DWORD>(global::client + 0x10F4F8);
    global::player_p = Read<DWORD>(global::client + 0x109B74);
    if (global::Debug)
    {
        AllocConsole();
        SetWindowText(GetConsoleWindow(), L"Debug");
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    }
    /*GAME HWND STUFF*/
    global::Game.hwnd = FindWindow(NULL, L"AssaultCube");
    global::Game.size = { static_cast<float>(Read<int>(global::client + 0x110C94)) ,static_cast<float>(Read<int>(global::client + 0x110C98)) };
    global::Game.position = GetWindowPos(global::Game.hwnd);
    global::Game.position.y += bordersize;
    global::Game.position.x += bordersizex;
    /*               */
     
    std::thread Controls(Thread_Control);
    std::thread Hax(Hack_Thread);
    /*SET UP OVERLAY*/
    std::thread overlay(ThreadProc);
   /*              */
    
    while (global::THREAD_ON && global::Debug)
    {
        
        std::cout << "Player:\n\t"<<global::player->Name<<"\n\t\tStatus:\n\t\t\tHealth: " << global::player->Health << " / 100\n\t\t\tArmour: " << global::player->Armour << "\n\t\t\tTeam #: " << global::player->Team_Number << "\n\t\t\tPosition:  X: " << global::player->Position.x << ", Y: " << global::player->Position.y << ", Z: " << global::player->Position.z << "\n\t\tWeapon (equipped):\n\t\t\tName: " << global::player->Equipped.weapon_name << "\n\t\t\tAmmo: " << global::player->Equipped.weapon_ammo.loaded << " / " << global::player->Equipped.weapon_ammo.inv << "\n" << std::endl;
        
        std::vector<Entity*>* List =  Entity_List();
        for (Entity* Ent : *List)
        {
            
            if (global::Show_Info && Ent->place_number == global::selected)
                std::cout << Ent->Name << "\n\tStatus:\n\t\tHealth: " << Ent->Health << " / 100\n\t\tArmour: " << Ent->Armour << "\n\t\tTeam #: " << Ent->Team_Number << "\n\tPosition:  X: " << Ent->Position.x << ", Y: " << Ent->Position.y << ", Z: " << Ent->Position.z << "\n\tWeapon (equipped):\n\t\tName: " << Ent->Equipped.weapon_name << "\n\t\tAmmo: " << Ent->Equipped.weapon_ammo.loaded << " / " << Ent->Equipped.weapon_ammo.inv << "\n" << std::endl;
            else
                std::cout << Ent->Name<< "\n" << std::endl;
            delete Ent;//Remebmer to delete!!!!! 
        }
        delete List;//Not a vector array
        Sleep(100);
        system("cls");
    }
    Controls.join();
    Hax.join();
    overlay.join();
    delete global::player;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        
        global::Main = std::thread(Thread);
        

        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        global::THREAD_ON = false;
        global::Main.join();
        break;
    }
    return TRUE;
}

LRESULT CALLBACK DLLWindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL RegisterDLLWindowClass()
{
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DLLWindowProc;
    wc.hInstance = global::inj_hModule;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)RGB(0, 0, 0);
    wc.lpszClassName = L"WindowClass";

    if (!RegisterClassEx(&wc))
        return 0;
}
DWORD WINAPI ThreadProc() {
    MSG msg;
    RegisterDLLWindowClass();
    global::overlay = CreateWindowEx(0,
        L"WindowClass",
        L"",
        WS_EX_TOPMOST | WS_POPUP,
        global::Game.position.x, global::Game.position.y,
        global::Game.size.x, global::Game.size.y,
        NULL,
        NULL,
        global::inj_hModule,
        NULL);
    SetWindowLong(global::overlay, GWL_EXSTYLE, (int)GetWindowLong(global::overlay, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    SetLayeredWindowAttributes(global::overlay, RGB(0, 0, 0), 0, ULW_COLORKEY);
    SetLayeredWindowAttributes(global::overlay, 0, 255, LWA_ALPHA);
    
    ShowWindow(global::overlay, SW_SHOWDEFAULT);
    SetWindowPos(global::overlay, HWND_NOTOPMOST, global::Game.position.x, global::Game.position.y, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    initD3D();
    SetForegroundWindow(global::overlay);//Focus this.
    while (global::THREAD_ON)
    {
        SetWindowPos(global::overlay, HWND_TOPMOST, global::Game.position.x, global::Game.position.y, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        
        RENDER();

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            //275 - WM_TIMER???
            if (msg.message == WM_TIMER)
                SetForegroundWindow(global::Game.hwnd);//So it shows / begins the drawing
            TranslateMessage(&msg);
            
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {

                global::THREAD_ON = false;
                pFontS->Release();
                pFontL->Release();
                pFontM->Release();
                Arrow->Release();
                d3d->Release();
                d3ddev->Release();
            }
        }
        
    }
    return 1;

}
LRESULT CALLBACK DLLWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        DwmExtendFrameIntoClientArea(global::overlay, &margin);
        break;
    case WM_DESTROY:
     PostQuitMessage(0);
     return 0;
     break;

    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}