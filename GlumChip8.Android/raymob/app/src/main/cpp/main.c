#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <jni.h>
#include <math.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <android_native_app_glue.h>

#include "raymob.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "Chip8System.h"
#include "Chip8CPU.h"
#include "Chip8Display.h"
#include "Chip8RAM.h"
#include "Chip8Keyboard.h"
#include "Chip8TouchControls.h"

#define CPU_HZ 700.0
#define TIMER_HZ 60.0
#define MAX_CYCLES 20

#define LOG_TAG "Chip8Emu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//======================================================================================
// GLOBALS
//======================================================================================
static Word *bufferBytes = NULL;
static double _lastCpuTime = 0.0;
static double _lastTimerTime = 0.0;
static RenderTexture2D target;

//======================================================================================
// CHIP8 WRAPPERS
//======================================================================================
void Chip8Keyboard_UpdateKeys(unsigned char keys[16], int lastPressed)
{
    for (int i = 0; i < 16; i++)
        g_chip8SystemInstance._keyboard->_keys[i] = keys[i];
    g_chip8SystemInstance._keyboard->_lastPressed = lastPressed;
}

void Chip8_Decrement_Timers()
{
    if (g_chip8SystemInstance._cpu->_DT > 0)
        g_chip8SystemInstance._cpu->_DT--;
    if (g_chip8SystemInstance._cpu->_ST > 0)
        g_chip8SystemInstance._cpu->_ST--;
}

void Chip8_CPU_Step(void)
{
    Chip8CPU_ExecuteCurrent(g_chip8SystemInstance._cpu);
}

bool Chip8Display_IsHighRes(void)
{
    return g_chip8SystemInstance._display->_isHighRes;
}

bool Chip8Display_IsPlaneActive(int p)
{
    if (p < 0 || p > 1)
        return false;
    return (g_chip8SystemInstance._cpu->_activePlanes & (1 << p)) != 0;
}

Word *Chip8Display_GetPlane(int plane)
{
    if (!g_chip8SystemInstance._display || plane < 0 || plane > 1)
        return NULL;
    return g_chip8SystemInstance._display->_planes[plane];
}

//======================================================================================
// INITIALIZATION & ROM LOADING
//======================================================================================
void Init_Chip8Emulator(void)
{
    srand((unsigned int)time(NULL));

    static Chip8CPU cpu = {0};
    cpu = Chip8CPU_Create();

    static Chip8Display display = {0};
    display = Chip8Display_Create();

    static Chip8RAM ram = {0};
    memset(&ram, 0, sizeof(ram));
    cpu._ram = &ram;
    cpu._activePlanes = 0b01;

    static Chip8Keyboard keyboard = {0};

    Chip8CPU_ResetRegisters(&cpu);
    Chip8System_Init(&cpu, &display, &ram, &keyboard);
}

void LoadChip8RomFromMemory(Word *buffer, size_t length)
{
    Init_Chip8Emulator();
    if (!buffer || length == 0)
        return;

    _lastCpuTime = GetTime();
    _lastTimerTime = GetTime();

    Chip8CPU_LoadProgram(g_chip8SystemInstance._cpu, buffer, length);
    g_chip8SystemInstance.g_romLoaded = true;
    g_chip8SystemInstance._cpu->_running = true;
    LOGI("ROM loaded: %zu bytes", length);
}

//======================================================================================
// UPDATE & RENDER
//======================================================================================
void Update_Chip8_Logic(void)
{
    if (!g_chip8SystemInstance.g_romLoaded)
        return;

    Chip8Keyboard_UpdateKeys(g_chip8SystemInstance._keyboard->_keys,
                             g_chip8SystemInstance._keyboard->_lastPressed);
    double now = GetTime();
    int cycles = 0;

    // Catch up on CPU Cycles
    while ((now - _lastCpuTime) >= (1.0 / CPU_HZ) && cycles < MAX_CYCLES)
    {
        Chip8_CPU_Step();
        _lastCpuTime += 1.0 / CPU_HZ;
        cycles++;
    }

    // Catch up on Timer Cycles (60Hz)
    while ((now - _lastTimerTime) >= (1.0 / TIMER_HZ))
    {
        Chip8_Decrement_Timers();
        _lastTimerTime += 1.0 / TIMER_HZ;
    }
}

void Draw_Chip8_To_Texture(RenderTexture2D texTarget)
{
    BeginTextureMode(texTarget);
    ClearBackground(BLACK);

    if (!g_chip8SystemInstance.g_romLoaded)
    {
        DrawText("Please restart Emulator to open another ROM!", 10, 10, 5, WHITE);
        EndTextureMode();
        return;
    }

    int simW = Chip8Display_IsHighRes() ? 128 : 64;
    int simH = Chip8Display_IsHighRes() ? 64 : 32;
    int scale = Chip8Display_IsHighRes() ? 1 : 2;

    // This is where the screen rendering ends
    int screenDisplayWidth = simW * scale;

    Word *plane;
    Color color; // Gray for plane 2 for visibility

    for (int p = 0; p < 2; p++)
    {
        if (!Chip8Display_IsPlaneActive(p))
            continue;

        plane = Chip8Display_GetPlane(p);
        color = (p == 0) ? WHITE : GRAY;

        for (int y = 0; y < simH; y++)
        {
            for (int x = 0; x < simW; x++)
            {
                if (plane[y * 128 + x])
                {
                    DrawRectangle(x * scale, y * scale, scale, scale, color);
                }
            }
        }
    }
    const char *msg = TextFormat("ROM: %4X | FPS: %d",
                                 Chip8RAM_ReadByte(g_chip8SystemInstance._cpu->_ram, g_chip8SystemInstance._cpu->_PC),
                                 GetFPS());

    DrawText(msg, 10, 10, 10, GREEN);

    Chip8TouchControls_Render(g_chip8SystemInstance._keyboard->_keys, screenDisplayWidth);
    EndTextureMode();
}

//======================================================================================
// JNI CALLBACKS
//======================================================================================
JNIEXPORT void JNICALL
Java_com_raylib_raymob_NativeLoader_sendBytesToNative(JNIEnv *env, jobject thiz, jbyteArray buffer)
{
    jsize len = (*env)->GetArrayLength(env, buffer);
    jbyte *bytes = (*env)->GetByteArrayElements(env, buffer, NULL);
    LoadChip8RomFromMemory((Word *)bytes, len);
    (*env)->ReleaseByteArrayElements(env, buffer, bytes, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_com_raylib_raymob_NativeLoader_sendUriToNative(JNIEnv *env, jobject thiz, jstring uri)
{
    const char *path = (*env)->GetStringUTFChars(env, uri, NULL);
    if (!path)
        return;

    FILE *f = fopen(path, "rb");
    if (!f)
    {
        LOGI("Failed to open ROM: %s\n", path);
        (*env)->ReleaseStringUTFChars(env, uri, path);
        return;
    }

    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    rewind(f);

    if (bufferBytes)
        free(bufferBytes);
    bufferBytes = (Word *)malloc(len);
    if (bufferBytes)
    {
        fread(bufferBytes, 1, len, f);
        LoadChip8RomFromMemory(bufferBytes, len);
    }

    fclose(f);
    (*env)->ReleaseStringUTFChars(env, uri, path);
}

//======================================================================================
// JNI REQUESTS
//======================================================================================

extern struct android_app *GetAndroidApp(void);

void Request_Android_FilePicker(void)
{
    struct android_app *app = GetAndroidApp();
    if (!app)
        return;

    JavaVM *jvm = app->activity->vm;
    JNIEnv *env = NULL;

    // Get the env for the current thread
    (*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_6);
    (*jvm)->AttachCurrentThread(jvm, &env, NULL);

    jclass nativeLoaderClass = (*env)->FindClass(env, "com/raylib/raymob/NativeLoader");
    jmethodID openPickerMethod = (*env)->GetStaticMethodID(env, nativeLoaderClass, "openFilePicker", "()V");

    if (openPickerMethod)
    {
        (*env)->CallStaticVoidMethod(env, nativeLoaderClass, openPickerMethod);
    }
}

//======================================================================================
// MAIN LOOP
//======================================================================================
int main(void)
{
    InitWindow(0, 0, "GlumChip8");

    // 128 (Chip8 HighRes Width) + 120 (Keypad width + padding) = 248
    target = LoadRenderTexture(248, 64);

    // Use POINT filter to keep that chunky pixel look on mobile
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        Update_Chip8_Logic();
        // Chip8Display_Render(g_chip8SystemInstance._display, g_chip8SystemInstance._activePlanes);
        Draw_Chip8_To_Texture(target);
        // Draw the internal buffer scaled up to fill the actual Android screen
        // Source rect height is negative because OpenGL textures are Y-flipped
        DrawTexturePro(target.texture,
                       (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height},
                       (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                       (Vector2){0, 0}, 0.0f, WHITE);
        EndDrawing();

        // Android back button or key escape resets the ROM
        if (IsKeyPressed(KEY_ESCAPE))
        {
            LOGI("ROM reset requested");
            g_chip8SystemInstance.g_romLoaded = false;
            g_chip8SystemInstance._cpu->_running = false;
        }
    }

    UnloadRenderTexture(target);
    CloseWindow();
    if (bufferBytes)
        free(bufferBytes);

    return 0;
}