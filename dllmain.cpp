// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>

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

    if (clipCoords.w < 0.1f) {
        screen.z = 0.1f;
    }

    Vector3 NDC = { clipCoords.x / clipCoords.w , clipCoords.y / clipCoords.w, clipCoords.z / clipCoords.w };
    
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
        Ent->Equipped.weapon_name[i] = Read<char>((Read<DWORD>(Read<DWORD>(BASE + 0x374) + 0x0C)+0x0) + i);
    Ent->Equipped.weapon_ammo.loaded = Read<int>(Read<DWORD>(Read<DWORD>(BASE + 0x374) + 0x14));
    Ent->Equipped.weapon_ammo.inv = Read<int>(Read<DWORD>(Read<DWORD>(BASE + 0x374) + 0x10));
    return Ent;
}

void Player_Inti()
{
    for (int i = 0; i < 16; i++)
    {
        global::player->Name[i] = Read<char>(global::player_p + 0x225 + i);
        global::player->Last_Target[i] = Read<char>(global::client + 0x101C38 + i);
    }
    global::player->view_matrix = Read<View_Matrix>(0x501AE8);
    global::player->view_angles = Read<Vector2>(global::player_p + 0x40);
    global::player->Health = max(Read<int>(global::player_p + 0xF8), 0);
    global::player->Armour = Read<int>(global::player_p + 0xFC);
    global::player->Team_Number = Read<int>(global::player_p + 0x32C);
    global::player->Position = Read<Vector3>(global::player_p + 0x34);
    for (int i = 0; i < 16; i++)
        global::player->Equipped.weapon_name[i] = Read<char>((Read<DWORD>(Read<DWORD>(global::player_p + 0x374) + 0x0C) + 0x0) + i);
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
int Name_to_Index(std::string selected_name)
{
    int ret = 1;
    std::vector<Entity*>* List = Entity_List();
    for (Entity* Ent : *List)
    {
        if ((std::string)Ent->Name == selected_name)
            ret = Ent->place_number;
        delete Ent;
    }
    delete List;
    return ret;
}
Vector2 CalcAngle(Vector3 pos, Vector3 dst)
{
    float turnX = static_cast<float>(atan((pos.x - dst.x) / (pos.y - dst.y)) * (180 / 3.14159f));
    float turnY = static_cast<float>(-atan((pos.z - dst.z) / sqrt(pow(pos.y - dst.y, 2) + pow(pos.x - dst.x, 2))) * (180 / 3.14159f));/* Rad to Deg */
    turnX = 360 - turnX;

    /*Flip*/
    if (pos.y < dst.y)
        turnX = turnX - 180;
   
    /*Convert*/
    if (turnX > 360)
        turnX -= 360;
    else if (turnX < 0)
        turnX += 360;
   
    return { turnX ,turnY }; /* Return as a Vector 2 */
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
            if (!menu->IsHidden())
            menu->MenuDwn();
            Sleep(170);
        }
        else if (GetKeyState(VK_NUMPAD8) & 0x8000)
        {
            if (!menu->IsHidden())
            menu->MenuUp();
            Sleep(170);
        }
        else if (GetKeyState(VK_NUMPAD5) & 0x8000)
        {
            if (!menu->IsHidden())
            menu->Execute();
            Sleep(170);
        }
        else if (GetKeyState(VK_NUMPAD0) & 0x8000)
        {
            if (!menu->IsHidden())
            menu->BackMenu();
            Sleep(170);
        }
        else if (GetKeyState(VK_MULTIPLY) & 0x8000)
        {
            menu->MenuVisibility(!menu->IsHidden());
            Sleep(170);
        }        
        else if (GetKeyState(VK_NUMPAD4) & 0x8000)
        {
                if(!menu->IsHidden())
                menu->MenuArrayLeft();
                Sleep(170);
        }
        else if (GetKeyState(VK_NUMPAD6) & 0x8000)
        {
            if (!menu->IsHidden())
            menu->MenuArrayRight();
                Sleep(170);
        }

        Sleep(5);
    }
}
template <typename T>
int name_in_array(std::vector<T>arr,T name)
{
    for (int i = 0; i < arr.size(); i++)
    {
        if (arr.at(i) == name)
            return i;
    }
    return -1;
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
        std::vector<std::string>* tracknames = new std::vector<std::string>;
        for (Entity* Ent : *List)
        {  
            global::selected = Name_to_Index(global::selected_name);
            tracknames->push_back(Ent->Name);//this is getting deleted which because the char* is pointing to the Ent->Name! Big problemo
            
            if (Ent->place_number == global::selected)
            {
                /*   Aim Bot    */  
                if (global::AimBot && Ent->Health != 0 && global::player->Health > 0)
                {
                    if(!(global::player->Position.x == Ent->Position.x && global::player->Position.y == Ent->Position.y && global::player->Position.z == Ent->Position.z))
                    Write<Vector2>(global::player_p + 0x40, Ent->AngleToAimbot);
                }
                /*              */  
            }
            delete Ent;
        }
        delete List;

        for (int i = 0; i < tracknames->size(); i++) {
            std::string cname = tracknames->at(i);
            if (name_in_array<std::string>(*global::Entity_Name_List, cname) == -1) {
                global::Entity_Name_List->push_back(cname);
            }
        }
        for (int i = 0; i < global::Entity_Name_List->size(); i++) {
            std::string cname = global::Entity_Name_List->at(i);
            const int index = name_in_array<std::string>(*tracknames, cname);
                if (index == -1) {
                    global::Entity_Name_List->erase(global::Entity_Name_List->begin() + name_in_array<std::string>(*global::Entity_Name_List, cname));
                }
            
        }
        delete tracknames;
    }
}
DWORD WINAPI ThreadProc();
void Thread()
{
    global::client = (DWORD)GetModuleHandle(global::ac_client);
    global::entity_list = Read<DWORD>(global::client + 0x10F4F8);
    global::player_p = Read<DWORD>(global::client + 0x109B74);

    /*GAME HWND STUFF*/
    global::Game.hwnd = FindWindow(NULL, L"AssaultCube");
    global::Game.size = { static_cast<float>(Read<int>(global::client + 0x110C94)) ,static_cast<float>(Read<int>(global::client + 0x110C98)) };
    global::Game.position = GetWindowPos(global::Game.hwnd);
    global::Game.position.y += bordersize;
    global::Game.position.x += bordersizex;
    /*               */
    if (global::Debug)
    {
        AllocConsole();
        SetWindowText(GetConsoleWindow(), L"Debug");
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    }
    /*
    MENU SETUP
    */
    Sub_Menu* Sub1 = new Sub_Menu("Main");
    Sub_Menu* Sub2 = new Sub_Menu("HACKS");
    Sub2->Add_Toggle("ESP", global::Esp);
    Sub2->Add_Toggle("Aimbot", global::AimBot);
    Sub2->Add_Array(global::Entity_Name_List, "Selected", global::selected_name);
    Sub2->Add_Action("Teleport", [&] {Entity_Specific* E = new Entity_Specific; Vector3 V;if(E->Get_Pos_Of_S_Ent(global::selected, &V))Teleport(global::player_p, V); delete E;});
    Sub1->Add_Sub_Menu(Sub2);
    Sub1->Add_Action("Quit / Exit", [&] {PostMessage(global::overlay, WM_DESTROY, NULL, NULL); });
    menu = new Menu(Sub1);

    /*
    
    */
    

     
    std::thread Controls(Thread_Control);
    std::thread Hax(Hack_Thread);
    /*SET UP OVERLAY*/
    std::thread overlay(ThreadProc);
   /*              */
    
    while (global::THREAD_ON && global::Debug && false)
    {
        
        std::cout << "Player:\n\t"<<global::player->Name<<"\n\t\tStatus:\n\t\t\tHealth: " << global::player->Health << " / 100\n\t\t\tArmour: " << global::player->Armour << "\n\t\t\tTeam #: " << global::player->Team_Number << "\n\t\t\tPosition:  X: " << global::player->Position.x << ", Y: " << global::player->Position.y << ", Z: " << global::player->Position.z << "\n\t\tWeapon (equipped):\n\t\t\tName: " << global::player->Equipped.weapon_name << "\n\t\t\tAmmo: " << global::player->Equipped.weapon_ammo.loaded << " / " << global::player->Equipped.weapon_ammo.inv << "\n" << "\n";
        
       std::vector<Entity*>* List = Entity_List();
        for (Entity* Ent : *List)
        {
            if (global::Show_Info && Ent->place_number == global::selected)
                std::cout << Ent->Name << "\n\tStatus:\n\t\tHealth: " << Ent->Health << " / 100\n\t\tArmour: " << Ent->Armour << "\n\t\tTeam #: " << Ent->Team_Number << "\n\tPosition:  X: " << Ent->Position.x << ", Y: " << Ent->Position.y << ", Z: " << Ent->Position.z << "\n\tWeapon (equipped):\n\t\tName: " << Ent->Equipped.weapon_name << "\n\t\tAmmo: " << Ent->Equipped.weapon_ammo.loaded << " / " << Ent->Equipped.weapon_ammo.inv << "\n" << "\n";
            else
                std::cout << Ent->Name<< "\n" << "\n";
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

        while (PeekMessage(&global::msg, NULL, 0, 0, PM_REMOVE))
        {
            //275 - WM_TIMER???
            if (global::msg.message == WM_TIMER)
                SetForegroundWindow(global::Game.hwnd);//So it shows / begins the drawing
            TranslateMessage(&global::msg);
            
            DispatchMessage(&global::msg);
            if (global::msg.message == WM_QUIT)
            {
                std::cout << "Quit" << "\n";
                global::THREAD_ON = false;
                pFontS->Release();
                pFontL->Release();
                pFontM->Release();
                Arrow->Release();
                d3d->Release();
                d3ddev->Release();
                menu->Release();    
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