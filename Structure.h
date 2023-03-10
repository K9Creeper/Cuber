#pragma once
#include <vector>
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
namespace global {
    static auto ac_client = L"ac_client.exe";
    std::thread Main;
    bool THREAD_ON = true;

    HINSTANCE  inj_hModule;
    Game_Window Game;
    HWND       overlay;

    int player_count;//0x10F500
    int selected = 1;
    bool AimBot;
    bool Show_Info = true;

    DWORD client;
    DWORD entity_list;//0x10f4f8
    DWORD player_p; //0x109B74
    Player* player = new Player();

};
