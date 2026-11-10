#ifndef NATIVE_MODSMENU_H
#define NATIVE_MODSMENU_H

#if RETRO_USE_MOD_LOADER

#define MODSMENU_ROWS (5)

enum ModsMenuStates {
    MODSMENU_STATE_SETUP,
    MODSMENU_STATE_ENTER,
    MODSMENU_STATE_MAIN,
    MODSMENU_STATE_EXIT,
    MODSMENU_STATE_ACTION,
    MODSMENU_STATE_DIALOGWAIT
};

struct NativeEntity_MenuControl;

struct NativeEntity_ModsMenu : NativeEntityBase {
    ModsMenuStates state;
    float timer;
    NativeEntity_MenuControl *menuControl;
    NativeEntity_TextLabel *labelPtr;
    float labelRotateY;
    MatrixF matrix1;
    NativeEntity_SubMenuButton *buttons[MODSMENU_ROWS];
    int rowCount;
    int selectedButton;
    int page;
    int changed;
    NativeEntity_DialogPanel *dialog;
};

void ModsMenu_Create(void *objPtr);
void ModsMenu_Main(void *objPtr);
#endif

#endif // !NATIVE_MODSMENU_H
