#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "game.h"

#if defined(_WIN32)
// Windows entry point
hime2d::Framework* app;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (app != NULL) {
        app->handleMessages(hWnd, uMsg, wParam, lParam);
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    for (size_t i = 0; i < __argc; i++) {
        hime2d::Framework::args.push_back(__argv[i]);
    };
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    app = new Hime2DExperimentGame();
    app->initVulkan();
    app->setupWindow(hInstance, WndProc);
    app->prepare();
    app->renderLoop();
    delete (app);
    return 0;
}

#elif defined(__ANDROID__)
// Android entry point
hime2d::Framework* app;
void android_main(android_app* state)
{
    app = new Hime2DExperimentGame();
    state->userData = app;
    state->onAppCmd = hime2d::Framework::handleAppCommand;
    state->onInputEvent = hime2d::Framework::handleAppInput;
    androidApp = state;
    app->renderLoop();
    delete (app);
}
#endif
