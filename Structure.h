#pragma once
#include <vector>
#include <string>
template <typename T>
T Read(DWORD addressToRead)
{
    return *(T*)addressToRead;
}

template <typename T> /* https://stackoverflow.com/questions/23555730/reading-and-writing-with-a-dll-injection-c */
void Write(DWORD addressToWrite, T value)
{
    T* ptr = (T*)addressToWrite;
    *ptr = value;
    //CANNOT FREE THE POINTER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}
struct Vector3 {
    float x, y, z;
    //x : 0x34 
    //y: 0x38
    // z : 0x3C
};
struct Vector2 {
    float x, y;
};
struct Vector4 {
    float x, y, z, w;
};
struct View_Matrix
{
    float matrix[16];
};

struct ammo {
    int loaded/*14*/, inv/*10*/;
};
struct Weapon //0x374
{
    char weapon_name[16]/*0x0C*/;
    ammo weapon_ammo;
};
struct Game_Window {
    char name[16];
    bool focused;
    HWND hwnd;
    Vector2 position;
    Vector2 size;
};
class Entity {
public:
    char Name[16];//0x225
    int Team_Number /*0x32C*/, Health/*0xf8*/, Armour /*0xFC*/, place_number;
    Vector3 Position;

    Vector2 AngleToAimbot;
    Weapon Equipped;
};
class Player {
public:
    char Name[16];//0x225
    int Team_Number /*0x32C*/, Health/*0xf8*/, Armour /*0xFC*/;
    char Last_Target[16];//0x101C38
    View_Matrix view_matrix;//0x501AE8
    Vector2 view_angles;
    Vector3 Position;
    Weapon Equipped;
};
struct Option {
    int option_type;/* 1- Submenu */
    char Option_Name[16];
    std::function<void*> Function = nullptr;
    Sub_Menu* SubMenu = nullptr;
};

struct Options_Class {
    std::vector<Option*>* Option_List;
    int selected_option_index;
};
class Sub_Menu {
private:
    char* Menu_Name;
    Sub_Menu* Parent_Menu = nullptr;//NULLPTR REFFERS TO THE MAIN MENU WHERE EVERYTHING IS CONNECTED TO
    Options_Class* Options;
public:
    Sub_Menu(char* Menu_Name)
    {
        this->Menu_Name = Menu_Name;
        this->Options = new Options_Class;
    }
    std::vector<Option*>* GetOptionsList()
    {
        return Options->Option_List;
    }
    Option* GetSelectedOption()
    {
        return this->Options->Option_List->at(this->Options->selected_option_index);
    }
    Sub_Menu* GetParentMenu()
    {
        return Parent_Menu;
    }
    char* GetMenuName()
    {
        return this -> Menu_Name;
    }
    
};
class Menu {
    Sub_Menu* Selected = nullptr;
    void Set_Menu(Sub_Menu* Selected)
    {this->Selected = Selected;}
    void Release()
    {
        Sub_Menu* PLoop = Selected;
        while (PLoop != nullptr)
        {
            PLoop = PLoop->GetParentMenu();//Reach the main menu!
        }
        /* ^^^^^ First stage ^^^^^  */
        Sub_Menu* CLoop = PLoop;
        bool loop = true;
        while (loop)
        {
            bool empyu = true;
            for (Option * Op : *CLoop->GetOptionsList()) {
                if (Op->option_type == 1 && Op->SubMenu != nullptr /*Just in case*/)
                {
                    empyu = false;
                    CLoop = Op->SubMenu;
                        break;
                }
                else
                {
                    delete Op->SubMenu;   
                    //Delete other stuff....DO LATER
                }
                delete Op;
            }
            if (empyu)
            {
                delete PLoop->GetOptionsList();
            }
        }
        /*
        Loop through options. Delete contents of vector. Once done delete vector. Delete Everything else in the menu. Then delete menu.
        */
        delete PLoop;//LAST STAGE
    }
};
namespace global {
    static auto ac_client = L"ac_client.exe";
    std::thread Main;
    bool THREAD_ON = true;
    bool Debug = false;

    HINSTANCE  inj_hModule;
    Game_Window Game;
    HWND       overlay;

    int player_count;//0x10F500
    int selected = 1;
    bool AimBot;
    bool Esp;
    bool Show_Info = true;

    DWORD client;
    DWORD entity_list;//0x10f4f8
    DWORD player_p; //0x109B74
    Player* player = new Player();

};
