#include <iostream>

#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")

#include <render/imgui/imgui.h>
#include <render/imgui/imgui_impl_dx9.h>
#include <render/imgui/imgui_impl_win32.h>

#include "ui/ui.hh"
#include "render.hh"

#include <config.hh>
#include <includes.hh>

HWND main_hwnd = nullptr;
LPDIRECT3DDEVICE9        g_pd3dDevice;
D3DPRESENT_PARAMETERS    g_d3dpp;
LPDIRECT3D9              g_pD3D;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;
    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

ImWchar* get_font_glyph_ranges(ImGuiIO& io)
{
    static ImVector<ImWchar> ranges;
    if (ranges.empty()) {
        ImFontGlyphRangesBuilder builder;
        constexpr ImWchar base_ranges[]{ 0x0100, 0x024F, 0x0370, 0x03FF, 0x0600, 0x06FF, 0x0E00, 0x0E7F, 0 };
        builder.AddRanges(base_ranges);
        builder.AddRanges(io.Fonts->GetGlyphRangesJapanese()); // Add one of the default ranges
        builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic()); // Add one of the default ranges
        builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull()); // Add one of the default ranges
        builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon()); // Add one of the default ranges
        builder.BuildRanges(&ranges);
    }
    return ranges.Data;
}

void crender::init() {
    wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, g_cfg.title, NULL };
    RegisterClassEx(&wc);
    main_hwnd = LI_FN(CreateWindowExA).cached()(0, wc.lpszClassName, g_cfg.title, WS_POPUP, 0, 0, 5, 5, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(main_hwnd)) {
        CleanupDeviceD3D();
        LI_FN(UnregisterClassA).cached()(wc.lpszClassName, wc.hInstance);
        return;
    }

    // Show the window
    LI_FN(ShowWindow).cached()(main_hwnd, SW_HIDE);
    LI_FN(UpdateWindow).cached()(main_hwnd);

    // Setup Dear ImGui context
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; //crutial for not leaving the imgui.ini file
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

    io.Fonts->AddFontFromFileTTF("assets\\panel\\arial-unicode-ms.ttf", 20, NULL, get_font_glyph_ranges(io));

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(main_hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);
}

void crender::loop() {
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (LI_FN(PeekMessageA).cached()(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            LI_FN(TranslateMessage).cached()(&msg);
            LI_FN(DispatchMessageA).cached()(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            g_cfg.active_windows = 0;
            static bool inited = false;
            if (!inited) {
                ui.init(g_pd3dDevice);
                inited = true;
            }
            ui.render();
        }
        ImGui::EndFrame();

        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        // Update and Render additional Platform Windows
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ResetDevice();
        }

        if (!g_cfg.active_windows) msg.message = WM_QUIT;
    }
}

void crender::exit() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(main_hwnd);
    LI_FN(UnregisterClassA).cached()(wc.lpszClassName, wc.hInstance);
}