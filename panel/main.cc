#include <config.hh>
#include <utils/utils.hh>
#include <render/render.hh>

#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

#include <lobbies.hh>
#include <linguistics/linguistics.hh>

#include <server/server.hh>
#include <boosting/boosting.hh>
#include <anti debug/xorstr.hpp>
#include <encryption/encryption.hh>
#include <anti debug/anti dbg main.hh>

void common_tasks() {
    TCHAR pwd[MAX_PATH]{ "" };
    LI_FN(GetCurrentDirectoryA).cached()(MAX_PATH, pwd);
    g_cfg.assets_folder = std::string(pwd).append("\\assets");

    for (size_t i{}; i < lobbies.size(); i++) lobbies[i].lobby_idx = i;
    build_strings();
    boosting::threads.push_back(std::thread(get_version_info));
    boosting::threads.push_back(std::thread(check_versions));
}

void init() {
    common_tasks();
#ifdef _DEBUG
    utils::attach_console();
#else
    anti_debug::init();
#endif
    render.init();
    render.loop();
}

void exit() {
    utils::kill_all_cs();
    utils::kill_all_steam();
    render.exit();
    anti_debug::erase_portable_executable_header_from_memory();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    init();
    exit();
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, msg, wParam, lParam);
}