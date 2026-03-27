#include "Chip8TouchControls.h"
#include "raylib.h"
#include "raygui.h"

const int buttonSize = 12;
const int spacing = 2;
const int padding = 5;

static Vector2 mouse;
static float scaleX;
static float drawHeight;
static float offsetY;

static Vector2 virtualMouse;

void Chip8TouchControls_Render_KeyPad(Word keys[16], int offsetX)
{
    const char *hexLabels[16] = {
        "1", "2", "3", "C",
        "4", "5", "6", "D",
        "7", "8", "9", "E",
        "A", "0", "B", "F"};

    int keyMap[16] = {1, 2, 3, 0xC, 4, 5, 6, 0xD, 7, 8, 9, 0xE, 0xA, 0, 0xB, 0xF};

    for (int i = 0; i < 16; i++)
    {
        int column = i % 4;
        int row = i / 4;

        Rectangle btnRec = {
            (float)(offsetX + padding + (column * (buttonSize + spacing))),
            (float)(padding + (row * (buttonSize + spacing))),
            (float)buttonSize,
            (float)buttonSize};

        // Inject the virtual mouse position into raygui
        // This trick makes raygui 'think' the mouse is at the virtual coord
        GuiSetState(STATE_NORMAL); // Reset state

        // Check collision manually for the key array
        if (CheckCollisionPointRec(virtualMouse, btnRec))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                keys[keyMap[i]] = 1;
                GuiSetState(STATE_PRESSED); // Force visual 'pressed' look
            }
            else
            {
                GuiSetState(STATE_FOCUSED); // Force visual 'hover' look
            }
        }
        else
        {
            keys[keyMap[i]] = 0;
        }

        // Now draw the button with the forced state
        GuiButton(btnRec, hexLabels[i]);
        GuiSetState(STATE_NORMAL); // Reset for next button
    }
}

void Chip8TouchControls_Render_CloseRom(int offsetX)
{
    Rectangle btnRec = {
        (float)(offsetX + 5 * (buttonSize + spacing)),
        (float)(padding),
        (float)buttonSize * 2,
        (float)buttonSize};

    GuiSetState(STATE_NORMAL); // Reset state

    if (CheckCollisionPointRec(virtualMouse, btnRec))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            GuiSetState(STATE_PRESSED);
            g_chip8SystemInstance._cpu->_running = false;
            g_chip8SystemInstance.g_romLoaded = false;
        }
        else
        {
            GuiSetState(STATE_FOCUSED);
        }
    }

    // Now draw the button with the forced state
    GuiButton(btnRec, "Exit");
    GuiSetState(STATE_NORMAL); // Reset for next button
}

void Chip8TouchControls_Render(Word keys[16], int offsetX)
{
    Vector2 mouse = GetMousePosition();
    scaleX = (float)GetScreenWidth() / 248.0f;
    drawHeight = (float)GetScreenWidth() * (64.0f / 248.0f);
    offsetY = (GetScreenHeight() - drawHeight) / 2.0f;
    virtualMouse = (Vector2){
        mouse.x / scaleX,
        (mouse.y - offsetY) / scaleX // Use scaleX here too to maintain aspect ratio
    };
    Chip8TouchControls_Render_KeyPad(keys, offsetX);
    Chip8TouchControls_Render_CloseRom(offsetX);
}
