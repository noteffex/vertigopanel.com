#define NOMINMAX

#include <string>
#include <fstream>
#include <tchar.h>
#include <Windows.h>
#include "json.hpp"

std::string steam_directory;
BOOL launch_file(const char* exe_path, const char* exe_name, const char* params)
{
    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = _T("open");
    ShExecInfo.lpFile = _T(exe_name);
    ShExecInfo.lpParameters = params;
    ShExecInfo.lpDirectory = exe_path;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    return ShellExecuteExA(&ShExecInfo);
}

std::string decode(std::string in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
    int val = 0, valb = -8;
    for (size_t jj = 0; jj < in.size(); jj++) {
        char c = in[jj];
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

void read_config() {
    nlohmann::json object{};
    std::ifstream in("config.dat");
    if (!in.good()) {
        return;
    }

    std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    object = nlohmann::json::parse(decode(str), nullptr, false);

    if (object.is_discarded()) {
        in.close();
        return;
    }

    if (object.contains("steam_directory")) object["steam_directory"].get_to(steam_directory);
    in.close();
}

int __stdcall APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    read_config();
    std::string input = lpCmdLine;
    return launch_file(steam_directory.c_str(), std::string("steam_").append(input.substr(input.size() - 10)).append(".exe").c_str(), input.c_str());
}