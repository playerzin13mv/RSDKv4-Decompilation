#include "RetroEngine.hpp"

#if RETRO_USE_MOD_LOADER

#include <ctype.h>
#include <string.h>

#define MODSMENU_MAX_MODS (128)

static bool modsPending[MODSMENU_MAX_MODS];

static int ModsMenu_ModCount()
{
    int n = (int)modList.size();
    if (n > MODSMENU_MAX_MODS)
        n = MODSMENU_MAX_MODS;
    return n;
}

static int ModsMenu_PerPage(int n) { return n <= MODSMENU_ROWS ? MODSMENU_ROWS : MODSMENU_ROWS - 1; }

static int ModsMenu_PageCount(int n)
{
    if (n <= 0)
        return 1;
    int per = ModsMenu_PerPage(n);
    return (n + per - 1) / per;
}

static int ModsMenu_RowCount(int n)
{
    if (n <= 0)
        return 2;
    if (n <= MODSMENU_ROWS)
        return n;
    return MODSMENU_ROWS;
}

// Monta o texto de um botao: "ON NOME" ou "OFF NOME" (so letras maiusculas, numeros, espaco, - e .)
static void ModsMenu_BuildText(char *out, int index)
{
    const std::string &src = modList[index].name.empty() ? modList[index].folder : modList[index].name;
    const char *prefix     = modsPending[index] ? "ON " : "OFF ";
    int pos                = 0;
    for (const char *p = prefix; *p; ++p) out[pos++] = *p;

    int limit = 17;
    for (size_t i = 0; i < src.size() && pos < limit; ++i) {
        unsigned char c = (unsigned char)toupper((unsigned char)src[i]);
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == ' ')
            out[pos++] = (char)c;
        else
            out[pos++] = ' ';
    }
    out[pos] = 0;
}

static void ModsMenu_Refresh(NativeEntity_ModsMenu *self)
{
    char buf[0x40];
    int n     = ModsMenu_ModCount();
    int per   = ModsMenu_PerPage(n);
    int pages = ModsMenu_PageCount(n);

    for (int r = 0; r < self->rowCount; ++r) {
        if (n == 0) {
            strcpy(buf, r == 0 ? "NO MODS FOUND" : "SEE MODS FOLDER");
        }
        else if (n > MODSMENU_ROWS && r == MODSMENU_ROWS - 1) {
            sprintf(buf, "NEXT PAGE %d OF %d", self->page + 1, pages);
        }
        else {
            int idx = self->page * per + r;
            if (idx < n)
                ModsMenu_BuildText(buf, idx);
            else
                strcpy(buf, "---");
        }
        SetStringToFont8(self->buttons[r]->text, buf, FONT_LABEL);
    }
}

// Grava o modconfig.ini com o estado escolhido, sem alterar o motor em execucao
static void ModsMenu_Save()
{
    int n = ModsMenu_ModCount();
    bool old[MODSMENU_MAX_MODS];
    for (int m = 0; m < n; ++m) {
        old[m]             = modList[m].active;
        modList[m].active  = modsPending[m];
    }
    SaveMods();
    for (int m = 0; m < n; ++m) modList[m].active = old[m];
}

// Retorna true se um mod foi ligado/desligado
static bool ModsMenu_Activate(NativeEntity_ModsMenu *self, int row)
{
    int n = ModsMenu_ModCount();
    if (n == 0)
        return false;

    if (n > MODSMENU_ROWS && row == MODSMENU_ROWS - 1) {
        self->page = (self->page + 1) % ModsMenu_PageCount(n);
        ModsMenu_Refresh(self);
        return false;
    }

    int idx = self->page * ModsMenu_PerPage(n) + row;
    if (idx >= n)
        return false;

    modsPending[idx] = !modsPending[idx];
    ModsMenu_Save();
    ModsMenu_Refresh(self);
    return true;
}

void ModsMenu_Create(void *objPtr)
{
    RSDK_THIS(ModsMenu);

    int n = ModsMenu_ModCount();
    for (int m = 0; m < n; ++m) modsPending[m] = modList[m].active;

    self->menuControl      = (NativeEntity_MenuControl *)GetNativeObject(0);
    self->labelPtr         = CREATE_ENTITY(TextLabel);
    self->labelPtr->fontID = FONT_HEADING;
    self->labelPtr->scale  = 0.2;
    self->labelPtr->alpha  = 0;
    self->labelPtr->z      = 0;
    self->labelPtr->state  = TEXTLABEL_STATE_IDLE;
    char title[8]          = "MODS";
    SetStringToFont8(self->labelPtr->text, title, FONT_HEADING);
    self->labelPtr->alignOffset = 512.0;
    self->labelRotateY          = DegreesToRad(22.5);
    MatrixRotateYF(&self->labelPtr->renderMatrix, self->labelRotateY);
    MatrixTranslateXYZF(&self->matrix1, -128.0, 80.0, 160.0);
    MatrixMultiplyF(&self->labelPtr->renderMatrix, &self->matrix1);
    self->labelPtr->useRenderMatrix = true;

    self->rowCount = ModsMenu_RowCount(n);
    self->page     = 0;
    self->changed  = false;

    float y = 48.0;
    for (int i = 0; i < self->rowCount; ++i) {
        self->buttons[i] = CREATE_ENTITY(SubMenuButton);

        self->buttons[i]->matXOff = 512.0;
        self->buttons[i]->textY   = -4.0;
        self->buttons[i]->matZ    = 0.0;
        self->buttons[i]->scale   = 0.1;
        MatrixRotateYF(&self->buttons[i]->matrix, DegreesToRad(16.0));
        MatrixTranslateXYZF(&self->matrix1, -128.0, y, 160.0);
        MatrixMultiplyF(&self->buttons[i]->matrix, &self->matrix1);
        self->buttons[i]->useMatrix = true;
        y -= 30.0;
    }

    ModsMenu_Refresh(self);
}

void ModsMenu_Main(void *objPtr)
{
    RSDK_THIS(ModsMenu);
    switch (self->state) {
        case MODSMENU_STATE_SETUP: {
            self->timer += Engine.deltaTime;
            if (self->timer > 1.0) {
                self->timer = 0.0;
                self->state = MODSMENU_STATE_ENTER;
            }
            break;
        }
        case MODSMENU_STATE_ENTER: {
            self->labelPtr->alignOffset /= (1.125 * (60.0 * Engine.deltaTime));

            float div = (60.0 * Engine.deltaTime) * 16.0;
            for (int i = 0; i < self->rowCount; ++i) self->buttons[i]->matXOff += ((-176.0 - self->buttons[i]->matXOff) / div);

            self->timer += (Engine.deltaTime + Engine.deltaTime);
            self->labelPtr->alpha = (256.0 * self->timer);
            if (self->timer > 1.0) {
                self->timer    = 0.0;
                self->state    = MODSMENU_STATE_MAIN;
                keyPress.start = false;
                keyPress.A     = false;
            }
            break;
        }
        case MODSMENU_STATE_MAIN: {
            if (self->menuControl->state == MENUCONTROL_STATE_EXITSUBMENU) {
                self->state = MODSMENU_STATE_EXIT;
            }
            else if (usePhysicalControls) {
                if (touches > 0) {
                    usePhysicalControls = false;
                }
                else {
                    if (keyPress.up) {
                        PlaySfxByName("Menu Move", false);
                        self->selectedButton--;
                        if (self->selectedButton < 0)
                            self->selectedButton = self->rowCount - 1;
                    }
                    else if (keyPress.down) {
                        PlaySfxByName("Menu Move", false);
                        self->selectedButton++;
                        if (self->selectedButton >= self->rowCount)
                            self->selectedButton = 0;
                    }
                    for (int i = 0; i < self->rowCount; ++i) self->buttons[i]->b = 0xFF;
                    self->buttons[self->selectedButton]->b = 0x00;

                    if (self->buttons[self->selectedButton]->g > 0x80 && (keyPress.start || keyPress.A)) {
                        PlaySfxByName("Menu Select", false);
                        self->buttons[self->selectedButton]->state = SUBMENUBUTTON_STATE_FLASHING2;
                        self->buttons[self->selectedButton]->b     = 0xFF;
                        self->timer                                = 0.0;
                        self->state                                = MODSMENU_STATE_ACTION;
                    }
                }
            }
            else {
                float y = 48.0;
                for (int i = 0; i < self->rowCount; ++i) {
                    if (touches > 0) {
                        if (CheckTouchRect(-64.0, y, 96.0, 12.0) >= 0)
                            self->buttons[i]->b = 0x00;
                        else
                            self->buttons[i]->b = 0xFF;
                    }
                    else if (!self->buttons[i]->b) {
                        self->selectedButton = i;
                        PlaySfxByName("Menu Select", false);
                        self->buttons[i]->state = SUBMENUBUTTON_STATE_FLASHING2;
                        self->buttons[i]->b     = 0xFF;
                        self->timer             = 0.0;
                        self->state             = MODSMENU_STATE_ACTION;
                        break;
                    }
                    y -= 30.0;
                }

                if (self->state == MODSMENU_STATE_MAIN && (keyDown.up || keyDown.down)) {
                    self->selectedButton = 0;
                    usePhysicalControls  = true;
                }
            }
            break;
        }
        case MODSMENU_STATE_ACTION: {
            self->menuControl->state = MENUCONTROL_STATE_NONE;
            self->timer += Engine.deltaTime;

            if (!self->buttons[self->selectedButton]->state || self->timer > 1.0) {
                self->timer = 0.0;
                bool toggled = ModsMenu_Activate(self, self->selectedButton);

                if (toggled && !self->changed) {
                    self->changed = true;
                    self->dialog  = CREATE_ENTITY(DialogPanel);
                    self->dialog->buttonCount = DLGTYPE_OK;
                    char msg[0x40]            = "RESTART THE GAME TO APPLY MODS";
                    SetStringToFont8(self->dialog->text, msg, FONT_TEXT);
                    self->state = MODSMENU_STATE_DIALOGWAIT;
                }
                else {
                    self->state              = MODSMENU_STATE_MAIN;
                    self->menuControl->state = MENUCONTROL_STATE_SUBMENU;
                }
            }
            break;
        }
        case MODSMENU_STATE_DIALOGWAIT: {
            self->menuControl->state = MENUCONTROL_STATE_NONE;
            if (self->dialog->selection == DLG_OK) {
                self->state              = MODSMENU_STATE_MAIN;
                self->menuControl->state = MENUCONTROL_STATE_SUBMENU;
            }
            break;
        }
        case MODSMENU_STATE_EXIT: {
            self->labelPtr->alignOffset = (10.0 * (60.0 * Engine.deltaTime)) + self->labelPtr->alignOffset;
            self->timer += (Engine.deltaTime + Engine.deltaTime);

            for (int i = 0; i < self->rowCount; ++i) self->buttons[i]->matXOff += (12.0 * (60.0 * Engine.deltaTime));

            if (self->timer > 1.0) {
                self->timer = 0.0;
                RemoveNativeObject(self->labelPtr);
                for (int i = 0; i < self->rowCount; ++i) RemoveNativeObject(self->buttons[i]);
                RemoveNativeObject(self);
            }
            break;
        }
        default: break;
    }
}
#endif
