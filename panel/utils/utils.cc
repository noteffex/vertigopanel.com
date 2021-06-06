#include "utils.hh"
#include <fstream>
#include <tchar.h>
#include <shellapi.h>

#include <http/http.hh>

#include <TlHelp32.h>
#include <includes.hh>
#include <lobbies.hh>
#include <data/data.hh>
#include <render/ui/arrays.hh>

#include <anti debug/xorstr.hpp>
#include <encryption/encryption.hh>

// Structure that includes all screen hanldes and rectangles
BOOL CALLBACK cMonitorsVec::MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
{
    cMonitorsVec* pThis = reinterpret_cast<cMonitorsVec*>(pData);

    pThis->hMonitors.push_back(hMon);
    pThis->hdcMonitors.push_back(hdc);
    pThis->rcMonitors.push_back(*lprcMonitor);
    pThis->iMonitors.push_back(pThis->hdcMonitors.size());
    return TRUE;
}

cMonitorsVec::cMonitorsVec()
{
    EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
    std::sort(rcMonitors.begin(), rcMonitors.end(), [](RECT a, RECT b) { return a.left < b.left; });
}

HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in  = NULL, _old_in  = NULL;

static const std::string profiles = "profiles/";
static const std::string steam_community_link = "https://steamcommunity.com/";

bool utils::is_profile_banned(long long steam_id_64) {
    return is_profile_banned(std::to_string(steam_id_64));
}

bool utils::is_profile_banned(std::string steam_id_64) {
    return is_profile_banned((steam_community_link + profiles + steam_id_64).c_str());
}

bool utils::is_profile_banned(const char* url) {
    if (!std::string(url)._Starts_with(steam_community_link)) {
        return is_profile_banned(std::string(url));
    }

    auto data = http::get_data(url);
    if (data.second != 200) return false;
    return data.first.find("profile_ban") != std::string::npos;
}

void utils::no_update_notice() {
    std::string line{};
    bool exists = false;

    std::ifstream in("C:\\Windows\\System32\\drivers\\etc\\hosts");
    while (std::getline(in, line)) {
        if (line.find("blog.counter-strike.net") != std::string::npos) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        std::ofstream out;
        out.open("C:\\Windows\\System32\\drivers\\etc\\hosts", std::ios::app);
        if (!out) {
            utils::messagebox("Either File Not Found or Permission Denied\nPlease re-run the program as an admin", "Error");
        }
        else {
            out << "\n127.0.0.1" << "\t" << "blog.counter-strike.net\n";
        }
        out.close();
    }
}

void utils::eternal_sleep() {
    for (double i{}; i < DBL_MAX; i++) {
        utils::sleep(INT_MAX);
        utils::sleep(INT_MAX);
        utils::sleep(INT_MAX);
        utils::sleep(INT_MAX);
        utils::sleep(INT_MAX);
    }
}

void utils::kill_process_by_exe(const char* exe_name)
{
    HANDLE hSnapShot = LI_FN(CreateToolhelp32Snapshot).cached()(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = LI_FN(Process32First).cached()(hSnapShot, &pEntry);
    while (hRes)
    {
        if (strcmp(pEntry.szExeFile, exe_name) == 0)
        {
            HANDLE hProcess = LI_FN(OpenProcess).cached()(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
            if (hProcess != NULL)
            {
                LI_FN(TerminateProcess).cached()(hProcess, 9);
                LI_FN(CloseHandle).cached()(hProcess);
            }
        }
        hRes = LI_FN(Process32Next).cached()(hSnapShot, &pEntry);
    }
    LI_FN(CloseHandle).cached()(hSnapShot);
}

BOOL utils::terminate_pid(DWORD pid)
{
    DWORD dwDesiredAccess = PROCESS_TERMINATE;
    BOOL  bInheritHandle = FALSE;
    HANDLE hProcess = LI_FN(OpenProcess).cached()(dwDesiredAccess, bInheritHandle, pid);

    if (hProcess == NULL) {
        return FALSE;
    }

    BOOL result = LI_FN(TerminateProcess).cached()(hProcess, 1);
    LI_FN(CloseHandle).cached()(hProcess);
    return result;
}

MODULEENTRY32 utils::get_module_by_name(const char* module_name, DWORD pid) {
    MODULEENTRY32 entry = { 0 };
    void* snap = LI_FN(CreateToolhelp32Snapshot).cached()(TH32CS_SNAPMODULE, pid);

    if (!snap) return { 0 };

    entry.dwSize = sizeof(entry);
    bool run_module = LI_FN(Module32First).cached()(snap, &entry);

    while (run_module) {
        if (!strcmp((char*)entry.szModule, module_name)) {
            LI_FN(CloseHandle).cached()(snap);
            return entry;
        }
        run_module = LI_FN(Module32Next).cached()(snap, &entry);
    }

    LI_FN(CloseHandle).cached()(snap);
    return { 0 };
};

void utils::clear_file(std::string path) {
    if (!utils::exists(path)) return;
    std::ofstream ofs(path, std::ofstream::trunc);
    ofs.close();
}

void utils::clear_logfile(std::string steam_id) {
    if (!utils::exists(g_cfg.csgo_directory + "\\csgo\\log\\" + steam_id + ".log")) return;
    std::ofstream ofs(g_cfg.csgo_directory + "\\csgo\\log\\" + steam_id + ".log", std::ofstream::trunc);
    ofs.close();
}

void utils::clear_every_log(c_lobby lobby) {
    auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);
    for (size_t i{}; i < accs.size(); i++)
        utils::clear_logfile(accs[i].s_id);
}

std::vector<acc_info> utils::get_all_accs(c_lobby lobby, int leader_swap)
{
    auto v1 = data.get_accs_team(lobby.team_1, leader_swap);
    auto v2 = data.get_accs_team(lobby.team_2, leader_swap);
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
}

std::vector<acc_info> utils::get_all_accs_wingman(c_lobby lobby)
{
    auto v1 = data.get_accs_wingman(lobby.team_1);
    auto v2 = data.get_accs_wingman(lobby.team_2);
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
}

void utils::change_map_all(c_lobby lobby) {
    auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);
    for (size_t i{}; i < accs.size();i++) {
        if (lobby.game_type == 0)
            utils::change_map(accs[i].s_id, lobby.map);
        else
            utils::change_map_wingman(accs[i].s_id, lobby.map_wingman);
    }
}

void utils::kill_all_cs() {
    if (utils::get_pid("csgo.exe")) {
        utils::kill_process_by_exe("csgo.exe");
    }
}

void utils::kill_steam_stock() {
    if (utils::get_pid("steam.exe")) {
        utils::kill_process_by_exe("steam.exe");
    }
    if (utils::get_pid("steamwebhelper.exe")) {
        utils::kill_process_by_exe("steamwebhelper.exe");
    }
}

void utils::kill_pid(DWORD pid) {
    if (!pid) return;
    utils::terminate_pid(pid);
}

void utils::kill_all_steam() {
    kill_steam_stock();
    for (size_t i{}; i < data.accs.size(); i++) {
        if (data.accs[i].s_id == "nil" || data.accs[i].s_id.empty()) continue;

        auto str = std::string("steam_").append(data.accs[i].s_id).append(".exe");
        if (!utils::get_pid(str)) continue;
        utils::kill_process_by_exe(str.c_str());
    }
}

void game_info_txt(std::string steam) {
    std::string final_product = "";

    std::ifstream file(g_cfg.assets_folder + std::string("\\csgo\\cfg\\gameinfo.txt"));
    if (!file.is_open()) return;

    std::string line;
    int num_of_lines = 0;

    while (std::getline(file, line)) {
        size_t pos{ line.find("00000000") };
        if (pos != std::string::npos)
        {
            for (int i = 0; i < 10; i++) {
                line[i + pos] = steam[i];
            }
        }
        final_product += line + "\n";
    }
    file.close();

    std::ofstream a(g_cfg.csgo_directory + "\\csgo_" + steam + "\\gameinfo.txt");
    a << std::setw(4) << final_product;
    a.close();
}

void userdata_procedure(std::string steam_id) {
    auto path = g_cfg.steam_directory + "\\userdata\\" + steam_id + "\\730";
    std::filesystem::remove_all(path);

    path += "\\local\\cfg";
    utils::copy_folder(g_cfg.assets_folder + std::string("\\userdata"), path);
}

void cs_folder_procedure(std::string steam_id) {
    static std::vector<std::string> vids_list;
    if (vids_list.empty()) {
        vids_list.push_back("acknowledge.webm");
        vids_list.push_back("acknowledge540p.webm");
        vids_list.push_back("acknowledge720p.webm");
        vids_list.push_back("ancient.webm");
        vids_list.push_back("ancient540.webm");
        vids_list.push_back("ancient720.webm");
        vids_list.push_back("anubis.webm");
        vids_list.push_back("anubis540.webm");
        vids_list.push_back("anubis720.webm");
        vids_list.push_back("apollo.webm");
        vids_list.push_back("apollo540.webm");
        vids_list.push_back("apollo720.webm");
        vids_list.push_back("background_movie_op10.webm");
        vids_list.push_back("blacksite.webm");
        vids_list.push_back("blacksite540p.webm");
        vids_list.push_back("blacksite720p.webm");
        vids_list.push_back("blue.webm");
        vids_list.push_back("bonus.webm");
        vids_list.push_back("broken_fang_launch.webm");
        vids_list.push_back("broken_fang_launch540.webm");
        vids_list.push_back("broken_fang_launch720.webm");
        vids_list.push_back("cbble.webm");
        vids_list.push_back("cbble540p.webm");
        vids_list.push_back("cbble720p.webm");
        vids_list.push_back("chlorine.webm");
        vids_list.push_back("chlorine540.webm");
        vids_list.push_back("chlorine720.webm");
        vids_list.push_back("digital_glitch.webm");
        vids_list.push_back("dz_blacksite_preview.webm");
        vids_list.push_back("dz_frostbite_preview.webm");
        vids_list.push_back("dz_junglety_preview.webm");
        vids_list.push_back("dz_sirocco_preview.webm");
        vids_list.push_back("engage.webm");
        vids_list.push_back("engage540.webm");
        vids_list.push_back("engage720.webm");
        vids_list.push_back("guard540.webm");
        vids_list.push_back("guard720.webm");
        vids_list.push_back("intro.webm");
        vids_list.push_back("intro720p.webm");
        vids_list.push_back("intro-perfectworld.webm");
        vids_list.push_back("intro-perfectworld720p.webm");
        vids_list.push_back("mutiny.webm");
        vids_list.push_back("mutiny520.webm");
        vids_list.push_back("mutiny540.webm");
        vids_list.push_back("mutiny720.webm");
        vids_list.push_back("mvp.webm");
        vids_list.push_back("nuke.webm");
        vids_list.push_back("nuke540p.webm");
        vids_list.push_back("nuke720p.webm");
        vids_list.push_back("op9_main.webm");
        vids_list.push_back("op9_main540p.webm");
        vids_list.push_back("op9_main720p.webm");
        vids_list.push_back("op9_mainmenu.webm");
        vids_list.push_back("op9_mainmenu540p.webm");
        vids_list.push_back("op9_mainmenu720p.webm");
        vids_list.push_back("operation_loading.webm");
        vids_list.push_back("pink_ct.webm");
        vids_list.push_back("pink_t.webm");
        vids_list.push_back("purple.webm");
        vids_list.push_back("rankbar.webm");
        vids_list.push_back("search.webm");
        vids_list.push_back("search540.webm");
        vids_list.push_back("search720.webm");
        vids_list.push_back("sirocco.webm");
        vids_list.push_back("sirocco540.webm");
        vids_list.push_back("sirocco720.webm");
        vids_list.push_back("sirocco_night.webm");
        vids_list.push_back("sirocco_night540p.webm");
        vids_list.push_back("sirocco_night720p.webm");
        vids_list.push_back("splat.webm");
        vids_list.push_back("survival_promo.webm");
        vids_list.push_back("survival_winner.webm");
        vids_list.push_back("swamp.webm");
        vids_list.push_back("swamp540.webm");
        vids_list.push_back("swamp720.webm");
        vids_list.push_back("tiers_bg.webm");
        vids_list.push_back("tournament_bg.webm");
        vids_list.push_back("tournament_bg540p.webm");
        vids_list.push_back("tournament_bg720p.webm");
        vids_list.push_back("trailer_0.webm");
        vids_list.push_back("trailer_1.webm");
        vids_list.push_back("vertigo.webm");
        vids_list.push_back("vertigo540.webm");
        vids_list.push_back("vertigo720.webm");
        vids_list.push_back("vs_bg.webm");
        vids_list.push_back("zoo_alpha.webm");
        vids_list.push_back("zoo_no_alpha.webm");
    }
    
    auto csgo_steamid_folder_path = g_cfg.csgo_directory + "\\csgo_" + steam_id;
    if (!utils::exists(csgo_steamid_folder_path)) {
        utils::copy_file(g_cfg.assets_folder + std::string("\\csgo\\cfg\\autoexec.cfg"), csgo_steamid_folder_path + "\\cfg\\autoexec.cfg");

        auto path = csgo_steamid_folder_path + "\\panorama";
        utils::copy_file(g_cfg.assets_folder + std::string("\\csgo\\videos\\anubis.webm"), path + "\\anubis.webm");
        utils::copy_file(g_cfg.assets_folder + std::string("\\csgo\\videos\\gobutton.webm"), path + "\\gobutton.webm");
        for (size_t k = 0; k < vids_list.size(); k++) {
            utils::copy_file(g_cfg.assets_folder + std::string("\\csgo\\videos\\anubis.webm"), path + "\\videos\\" + vids_list[k]);
        }
    }
}

void steam_procedure(std::string steam_id)
{
    if (utils::exists(g_cfg.steam_directory + std::string("\\steam_").append(steam_id).append(".exe"))) return;
    std::filesystem::copy_file(g_cfg.steam_directory + std::string("\\steam.exe"), g_cfg.steam_directory + std::string("\\steam_").append(steam_id).append(".exe"));
}

void utils::place_prerequisites(std::string steam_id) {
    while (locks["file_folders_lock"]) utils::sleep(500);
    locks["file_folders_lock"] = true;
    {
        userdata_procedure(steam_id);
        cs_folder_procedure(steam_id);
        game_info_txt(steam_id);
        steam_procedure(steam_id);
    }
    locks["file_folders_lock"] = false;
}

void utils::place_all_prerequisites(c_lobby lobby) {
    auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);
    for (size_t i{}; i < accs.size(); i++) {
        utils::place_prerequisites(accs[i].s_id);
    }
}

bool utils::does_acc_have_string(std::string steamid, std::string to_find) {
    return utils::does_file_contain_string(g_cfg.csgo_directory + "\\csgo\\log\\" + steamid + ".log", to_find);
}

bool utils::does_file_contain_string(std::string path, std::string to_find) {
    std::ifstream file(path);
    if (!file.is_open()) {
        utils::cout("couldnt open file \n");
        return false;
    }
    std::string line{};
    while (std::getline(file, line)) {
        const size_t found = line.find(to_find);
        if (found == std::string::npos) continue;
        return true;
    }
    return false;
}

void utils::set_process_priority(DWORD pid, int priority)
{
    HANDLE h = LI_FN(OpenProcess).cached()(PROCESS_SET_INFORMATION, TRUE, pid);
    if (!h) return;

    LI_FN(SetPriorityClass).cached()(h, priority);
    LI_FN(CloseHandle).cached()(h);
}

void utils::clear_console() {
    if (!_out) return;
    system("cls");
}

int64_t utils::epoch_time() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void utils::messagebox(const char* desc, const char* title) {
    MessageBoxA(0, desc, title, 0);
}

void utils::change_map(std::string steam_id, int map) {
    std::ofstream out;
    std::string file = g_cfg.steam_directory + "\\userdata\\" + steam_id + "\\730\\local\\cfg\\config.cfg";
    out.open(file, std::ios::app);
    if (!out) {
        messagebox("File not found", "Error");
        return;
    }
    out << std::string("\n	ui_playsettings_mode_official_v20 \"competitive\"\nplayer_competitive_maplist_8_9_0_5C054B51 mg_").append(map_names[map]).append("\n");
}

void utils::change_map_wingman(std::string steam_id, int map) {
    std::ofstream out;
    std::string file = g_cfg.steam_directory + "\\userdata\\" + steam_id + "\\730\\local\\cfg\\config.cfg";
    out.open(file, std::ios::app);
    if (!out) {
        messagebox("File not found", "Error");
        return;
    }
    out << std::string("\nui_playsettings_mode_official_v20 \"scrimcomp2v2\"\nplayer_competitive_maplist_2v2_9_0_24235B08 mg_").append(wingman_map_names[map]).append("\n");
}

bool utils::login_to_acc(std::string username, std::string password)
{
    return utils::launch_file(g_cfg.steam_directory.c_str(), "Steam.exe", std::string("-login ").append(username).append(" ").append(password).c_str());
}

bool utils::start_cs(int x, int y, std::string username, std::string password, std::string steam_id)
{
    static auto params_size = std::string(" -w 640 -h 480");
    static auto params_generic = std::string(" -applaunch 730 +fps_max 49 +fps_max_menu 29 -novid -nosound -nosync -vrdisable -nopreload -limitvsconst -nohltv -softparticlesdefaultoff -noaafonts -nojoy -noshader +violence_hblood 0 +r_dynamic 0 -noubershader -nofbo -nodcaudio -nomsaa +set vid level 0 +sethdmodels 0 +noshaderapi -worldwide -nohltv -noqueuedload -textmode -no-browser");

    auto params_login = std::string(" -login ").append(username).append(" ").append(password);
    auto params_pos = std::string(" -x ").append(std::to_string(x)).append(" -y ").append(std::to_string(y));
    auto params_logging = std::string(" +con_logfile log\\").append(steam_id).append(".log");
    auto params_language = " -language " + steam_id;

    return utils::launch_file((g_cfg.assets_folder + std::string("\\panel")).c_str(), "client.exe", (params_login + params_generic + params_pos + params_size + params_logging + params_language).c_str());
}

bool utils::launch_file(const char* exe_path, const char* exe_name, const char* params)
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
    return LI_FN(ShellExecuteExA).cached()(&ShExecInfo);
}

std::pair<USHORT, bool> VirtualKeyToScanCode(UINT VirtualKey)
{
    USHORT ScanCode = (USHORT)MapVirtualKey(VirtualKey, MAPVK_VK_TO_VSC);
    bool IsExtended = false;

    // because MapVirtualKey strips the extended bit for some keys
    switch (VirtualKey)
    {
    case VK_RMENU: case VK_RCONTROL:
    case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
    case VK_PRIOR: case VK_NEXT: // page up and page down
    case VK_END: case VK_HOME:
    case VK_INSERT: case VK_DELETE:
    case VK_DIVIDE: // numpad slash
    case VK_NUMLOCK:
    {
        IsExtended = true;
        break;
    }
    }

    return std::make_pair(ScanCode, IsExtended);
}

LPARAM CreateLPARAM_KeyUpDown(UINT VirtualKey, USHORT RepeatCount, bool TransitionState, bool PreviousKeyState, bool ContextCode)
{
    std::pair<USHORT, bool> ScanCode = VirtualKeyToScanCode(VirtualKey);
    return (
        (LPARAM(TransitionState) << 31) |
        (LPARAM(PreviousKeyState) << 30) |
        (LPARAM(ContextCode) << 29) |
        (LPARAM(ScanCode.second) << 24) |
        (LPARAM(ScanCode.first) << 16) |
        LPARAM(RepeatCount)
        );
}

LPARAM CreateLPARAM_KeyDown(UINT VirtualKey)
{
    return CreateLPARAM_KeyUpDown(VirtualKey, 1, false, false, false);
}

LPARAM CreateLPARAM_KeyUp(UINT VirtualKey)
{
    return CreateLPARAM_KeyUpDown(VirtualKey, 1, true, true, false);
}

void utils::send_char(HWND hw, UINT virtual_key) {
    SendMessageA(hw, WM_CHAR, virtual_key, 0);
}

void utils::send_key(HWND hw, UINT virtual_key) {
    SendMessageA(hw, WM_KEYDOWN, virtual_key, CreateLPARAM_KeyDown(virtual_key));
    utils::sleep(50);
    SendMessageA(hw, WM_KEYUP, virtual_key, CreateLPARAM_KeyUp(virtual_key));
}

bool utils::cout_lobby(c_lobby lobby, const char* fmt, ...)
{
    char buf[1024];
    va_list va;

    va_start(va, fmt);
    _vsnprintf_s(buf, 1024, fmt, va);
    va_end(va);

    auto k = std::string("[Lobby ").append(std::to_string(lobby.lobby_idx + 1)).append("] ").append(buf);
    if (g_cfg.log_to == 0) {
        utils::webhook(k.c_str());
    }
    else {
        log_to_site(buf, lobby.lobby_idx);
    }
    return cout(k.append("\n").c_str());
}

bool utils::cout(const char* fmt, ...)
{
    if (!_out) return false;

    char buf[1024];
    va_list va;
    
    va_start(va, fmt);
    _vsnprintf_s(buf, 1024, fmt, va);
    va_end(va);
   
    return WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
}

void utils::webhook(const char* x) {
    if (g_cfg.webhook_url.empty() || g_cfg.webhook_url.find("https://discord.com/api/webhooks") == std::string::npos) { // invalid/empty webhook
        return;
    }

    std::string dump = std::string("{\"content\":\"").append(x).append("\"}");

    if (CURL* curl = curl_easy_init(); curl) {
        struct curl_slist* list = NULL;
        list = curl_slist_append(list, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

        curl_easy_setopt(curl, CURLOPT_URL, g_cfg.webhook_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dump.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http::write_callback);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

void utils::attach_console() {
    _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
    _old_err = GetStdHandle(STD_ERROR_HANDLE);
    _old_in  = GetStdHandle(STD_INPUT_HANDLE);

    AllocConsole();
    AttachConsole(GetCurrentProcessId());

    _out = GetStdHandle(STD_OUTPUT_HANDLE);
    _err = GetStdHandle(STD_ERROR_HANDLE);
    _in  = GetStdHandle(STD_INPUT_HANDLE);

    SetConsoleMode(_out, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
    SetConsoleMode(_in, ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
}

void utils::detach_console() {
    if (_out && _err && _in) {
        FreeConsole();
        if (_old_out) SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
        if (_old_err) SetStdHandle(STD_ERROR_HANDLE, _old_err);
        if (_old_in)  SetStdHandle(STD_INPUT_HANDLE, _old_in);
    }
}

void utils::sleep(int time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

int utils::rename(const char* current, const char* new_name) {
    return std::rename(current, new_name);
}

void utils::run_mouse_in_circles(HWND hw, int radius, int times)
{
    constexpr float double_pi = 44 / 7;
    for (int i{}; i < times; i++) {
        for (float j{}; j < double_pi; j += 0.0174533f) {
            float cosine = cosf(j);
            float sine = sinf(j);

            float x = cosine * radius;
            float y = sine * radius;

            SendMessageA(hw, WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
            utils::sleep(1);
        }
    }
}

void utils::left_click(HWND hw, int x, int y)
{
    SetActiveWindow(hw);
    SendMessageA(hw, WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
    utils::sleep(50);
    SendMessageA(hw, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
    utils::sleep(50);
    SendMessageA(hw, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
}

void utils::right_click(HWND hw, int x, int y)
{
    SetActiveWindow(hw);
    SendMessageA(hw, WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
    utils::sleep(50);
    SendMessageA(hw, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(x, y));
    utils::sleep(50);
    SendMessageA(hw, WM_RBUTTONUP, 0, MAKELPARAM(x, y));
}

void utils::send_middlemouse(HWND hw, int x, int y)
{
    SetActiveWindow(hw);
    SendMessageA(hw, WM_MBUTTONDOWN, MK_MBUTTON, MAKELPARAM(x, y));
    utils::sleep(50);
    SendMessageA(hw, WM_MBUTTONUP, 0, MAKELPARAM(x, y));
}

bool utils::create_logs_folder() {
    if (!g_cfg.csgo_directory.empty()) {
        if (!utils::exists(g_cfg.csgo_directory + "\\csgo\\log")) {
            return std::filesystem::create_directories(g_cfg.csgo_directory + "\\csgo\\log");
        }
        return true;
    }
    return false;
}

void utils::copy_folder(std::string input_path, std::string output_path) {
    std::filesystem::path path{ output_path };
    std::filesystem::create_directories(path.parent_path());
    std::filesystem::copy(input_path, output_path);
}

void utils::copy_file(std::string input_path, std::string output_path) {
    std::filesystem::path path{ output_path };
    std::filesystem::create_directories(path.parent_path());
    std::filesystem::copy_file(input_path, output_path);
}

bool utils::exists(std::string file) {
    struct stat buffer;
    return !stat(file.c_str(), &buffer);
}

DWORD utils::get_pid(std::string exe_name)
{
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processSnapshot{ LI_FN(CreateToolhelp32Snapshot).cached()(TH32CS_SNAPPROCESS, NULL) };

    if (processSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    LI_FN(Process32First).cached()(processSnapshot, &processInfo);

    if (!exe_name.compare(processInfo.szExeFile)) {
        LI_FN(CloseHandle).cached()(processSnapshot);
        return processInfo.th32ProcessID;
    }

    while (LI_FN(Process32Next).cached()(processSnapshot, &processInfo)) {
        if (!exe_name.compare(processInfo.szExeFile)) {
            LI_FN(CloseHandle).cached()(processSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    LI_FN(CloseHandle).cached()(processSnapshot);
    return 0;
}