#pragma once
#include <vector>
#include <string>
#include <functional>
#include <d3d9.h>
#include <d3dx9.h>
#include <thread>
#include <sstream>

/*GRAPHICS*/
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;
LPD3DXFONT pFontS, pFontM, pFontL, Arrow;
MARGINS  margin;
void DrawFilledBox(int x, int y, int w, int h, DWORD color);
void DrawLinedBox(int x, int y, int w, int h, int p, DWORD color);
void DrawString(int x, int y, DWORD color, LPD3DXFONT g_pFont, const char* fmt);

/*       */
/*       */
#define SubType 1;
#define ToggleType 3;
#define ActionType 2;
template <typename T> /* https://stackoverflow.com/questions/23555730/reading-and-writing-with-a-dll-injection-c */
void Write(DWORD addressToWrite, T value);
template <typename T>
T Read(DWORD addressToRead);
struct Vector3;
struct Vector2;
struct Vector4;
struct View_Matrix;
struct ammo;
struct Weapon;
struct Game_Window;
class Entity;
std::vector<Entity*>* Entity_List();
class Entity_Specific;
class Player;
Vector3 _WorldToScreen(Vector3 pos, View_Matrix matrix);
class Sub_Menu;
struct Option;
struct Options_Class;
class Menu;
Menu* menu = nullptr;
/*      */


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
namespace global {
    static auto ac_client = L"ac_client.exe";
    std::thread Main;
    bool THREAD_ON = true;
    bool Debug = true;

    HINSTANCE  inj_hModule;
    Game_Window Game;
    HWND       overlay;
    MSG msg;

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

struct Option {
    int option_type = NULL;/* 1- Submenu 2- button  3-toggle 4- kinda-array*/
    const char* option_name = NULL;
    std::function<void()> function = NULL;
    Sub_Menu* Sub = nullptr;
    bool* boolean = nullptr;  
    std::vector<const char*>* Array = nullptr;
    int index_array = 0;
};


struct Options_Class {
    std::vector<Option*>* Option_List = new std::vector<Option*>;
    int selected_option_index = 0;
};
class Sub_Menu {
private:
    const char* Menu_Name;
    Sub_Menu* Parent_Menu = nullptr;//NULLPTR REFFERS TO THE MAIN MENU WHERE EVERYTHING IS CONNECTED TO
    Options_Class* Options = nullptr;
public:
    Sub_Menu(const char* Menu_Name) { this->Menu_Name = Menu_Name; this->Options = new Options_Class; }
    std::vector<Option*>* GetOptionsList()
    {
        return this->Options->Option_List;
    }
    Option* GetSelectedOption()
    {
        return this->Options->Option_List->at(this->Options->selected_option_index);
    }
    Options_Class* GetOptionClass()
    {
        return this->Options;
    }
    Sub_Menu* GetParentMenu()
    {
        return this->Parent_Menu;
    }
    const char* GetMenuName()
    {
        return this -> Menu_Name;
    }
    void Add_Sub_Menu(Sub_Menu* Menu)
    {
        Option* Op = new Option;
        Menu->Parent_Menu = this;
        Op->option_name = Menu->GetMenuName();
        Op->Sub = Menu;
        Op->option_type = SubType;
        this->Options->Option_List->push_back(Op);
    }
    void Add_Toggle(const char* Toggle_Name, bool& b)
    {
        Option* Op = new Option;
        Op->option_name = Toggle_Name;
        Op->option_type = ToggleType;
        Op->boolean = &b;
        Op->function = [&] { b = !b; };
        this->Options->Option_List->push_back(Op);
    }
    void Add_Action(const char* Action_Name, std::function<void()>Func)
    {
        Option* Op = new Option;
        Op->option_name = Action_Name;
        Op->option_type = ActionType;
        Op->function = Func;
        this->Options->Option_List->push_back(Op);
    }
    template <typename T>
    void Add_Array(std::vector<const char*>* value, const char* Array_Name, T& change)
    {
        Option* Op = new Option;
        Op->option_name = Array_Name;
        Op->option_type = 4;
        Op->Array = value;
        std::cout << change << std::endl;
        Op->function = [&] {
            std::stringstream strValue;
            strValue << this->GetSelectedOption()->Array->at(this->GetSelectedOption()->index_array);
            strValue >> change;
            std::cout << change << std::endl;
        };
        this->Options->Option_List->push_back(Op);
        Op->function();
    }
};
class Menu {
private:
    Sub_Menu* Selected = nullptr;
    bool Hidden = false;
public:
    void Set_Menu(Sub_Menu* Selected)
    {this->Selected = Selected;}
    Sub_Menu* GetSelectedMenu(){return this->Selected;}
    Menu(Sub_Menu* Selected){this->Set_Menu(Selected);}
    bool IsHidden()
    {
        return this->Hidden;
    }
    int MenuX = 0, MenuY = global::Game.size.y / 3, MenuBorderSize = 2, BoxHeight = global::Game.size.y / 18, BoxWidth = global::Game.size.x / 6;
    void DrawMenu()
    {
        if (!Hidden)
        {
            int IMenux = MenuX + MenuBorderSize;
            int IMenuy = MenuY + MenuBorderSize;
            DWORD BORDERCOLOR = D3DCOLOR_ARGB(255, 120, 10, 240);
            DrawFilledBox(MenuX, MenuY, MenuBorderSize + BoxWidth, MenuBorderSize, BORDERCOLOR);//Border
            for (int i = 0; i < Selected->GetOptionsList()->size(); i++)
            {
                int Y = IMenuy + i * BoxHeight + i * MenuBorderSize;
                DWORD COLOR = D3DCOLOR_ARGB(255, 0, 0, 0);
                DWORD TEXTCOLOR = D3DCOLOR_ARGB(255, 255, 255, 255);
                if (i == Selected->GetOptionClass()->selected_option_index)
                {
                    COLOR = D3DCOLOR_ARGB(235, 40, 40, 40);
                }
                DrawFilledBox(IMenux, Y, BoxWidth, BoxHeight, COLOR);
                DrawFilledBox(MenuX, Y+ BoxHeight, MenuBorderSize + BoxWidth, MenuBorderSize, BORDERCOLOR);//Border
                DrawFilledBox(MenuX, Y, MenuBorderSize, BoxHeight + MenuBorderSize, BORDERCOLOR);
                DrawFilledBox(MenuX+ BoxWidth, Y, MenuBorderSize, BoxHeight + MenuBorderSize, BORDERCOLOR);
                std::string name = this->Selected->GetOptionsList()->at(i)->option_name;
                if (this->Selected->GetOptionsList()->at(i)->option_type == 3)
                    if (*this->Selected->GetOptionsList()->at(i)->boolean)
                        name += " ON";
                    else
                        name += " OFF";
                else if (this->Selected->GetOptionsList()->at(i)->option_type == 1)
                    name += "  >";
                else if (this->Selected->GetOptionsList()->at(i)->option_type == 4)
                {
                    std::string sele = this->Selected->GetOptionsList()->at(i)->Array->at(this->Selected->GetOptionsList()->at(i)->index_array);
                    name += " < ";
                    name += sele + " >";
                }
                DrawString(IMenux, Y + BoxHeight / 3, TEXTCOLOR, pFontS, name.c_str());
            }
            
        }
    }
    void MenuUp()
    {
        int c = this->Selected->GetOptionClass()->selected_option_index;
        if (c - 1 >= 0)
            this->Selected->GetOptionClass()->selected_option_index -= 1;
        else
            this->Selected->GetOptionClass()->selected_option_index = this->Selected->GetOptionsList()->size()-1;
    }
    void MenuDwn()
    {
        int c = this->Selected->GetOptionClass()->selected_option_index;
        if (c + 1 < this->Selected->GetOptionsList()->size())
            this->Selected->GetOptionClass()->selected_option_index += 1;
        else
            this->Selected->GetOptionClass()->selected_option_index = 0;
    }
    void MenuArrayRight() {
        if (this->GetSelectedMenu()->GetSelectedOption()->option_type == 4)
        {

            if (this->GetSelectedMenu()->GetSelectedOption()->index_array + 1 >= this->GetSelectedMenu()->GetSelectedOption()->Array->size())
                this->GetSelectedMenu()->GetSelectedOption()->index_array = 0;
            else
                this->GetSelectedMenu()->GetSelectedOption()->index_array++;
            this->Execute();
        }
    }
    void MenuArrayLeft() {
        if (this->GetSelectedMenu()->GetSelectedOption()->option_type == 4)
        {

            if (this->GetSelectedMenu()->GetSelectedOption()->index_array - 1 < 0)
                this->GetSelectedMenu()->GetSelectedOption()->index_array = this->GetSelectedMenu()->GetSelectedOption()->Array->size() - 1;
            else
                this->GetSelectedMenu()->GetSelectedOption()->index_array--;
            this->Execute();
        }
    }
    void Execute()
    {
        if (this->Selected->GetSelectedOption()->option_type != 1)
            this->Selected->GetSelectedOption()->function();
        else
            this->Set_Menu(this->Selected->GetSelectedOption()->Sub);
    }
    void MenuVisibility(bool b)
    {
        this->Hidden = b;
    }
    void BackMenu()
    {
        if (this->Selected->GetParentMenu() != nullptr)
            this->Selected = this->Selected->GetParentMenu();
        else
            this->Hidden = true;
    }
    void Release()
    {
        Sub_Menu* PLoop = Selected;
        while (PLoop != nullptr){PLoop = PLoop->GetParentMenu();/*Reach the main menu!*/ }
        /* ^^^^^ First stage ^^^^^  */
        Sub_Menu* CLoop = PLoop;
        bool loop = true;
        while (loop)
        {
            bool empyu = true;
            for (Option* Op : *CLoop->GetOptionsList()){
                if (Op->option_type == 1 && Op->Sub != nullptr /*Just in case*/)
                {
                    empyu = false;
                    CLoop = Op->Sub;
                        break;
                }
                else
                {
                    delete Op->Sub;
                    delete Op->boolean;
                    //Delete other stuff....DO LATER
                }
                delete Op;
                CLoop = PLoop;
            }
            if (empyu) { delete PLoop->GetOptionsList(); delete PLoop->GetOptionClass(); }
        }
        /*
        Loop through options. Delete contents of vector. Once done delete vector. Delete Everything else in the menu. Then delete menu.
        */
        //LAST STAGE
        delete PLoop;
        delete this;
    }
};