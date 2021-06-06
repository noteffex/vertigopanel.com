#include <string>
#include <thread>
#include <includes.hh>
#include <filesystem>
#include <TlHelp32.h>
#include <config.hh>

#include <windows.h>
#include <vector>

struct acc_info;
class c_lobby;

struct cMonitorsVec
{
	std::vector<int>       iMonitors;
	std::vector<HMONITOR>  hMonitors;
	std::vector<HDC>       hdcMonitors;
	std::vector<RECT>      rcMonitors;

	cMonitorsVec();
	static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData);
};

// console stuff
namespace utils {
	inline cMonitorsVec monitors;
	void attach_console();
	void detach_console();
	void clear_console();
	bool cout(const char* fmt, ...);
	void log_to_site(const char* what, int lobby_index = 0);
	void webhook(const char* x);
	bool cout_lobby(c_lobby lobby, const char* fmt, ...);
}

namespace utils {
	bool is_profile_banned(const char* url);
	void no_update_notice();
	bool is_profile_banned(long long steam_id_64);
	bool is_profile_banned(std::string steam_id_64);
}

namespace utils {
	std::vector<acc_info> get_all_accs_wingman(c_lobby lobby);
	std::vector<acc_info> get_all_accs(c_lobby lobby, int leader_swap = -1);
	void change_map_all(c_lobby lobby);
	void change_map(std::string steam_id, int map);
	void change_map_wingman(std::string steam_id, int map);
	bool login_to_acc(std::string username, std::string password);
	bool launch_file(const char* exe_path, const char* exe_name, const char* params);
	bool start_cs(int x, int y, std::string username, std::string password, std::string steam_id);
}

namespace utils {
	int64_t epoch_time();
	void sleep(int time);
	int rename(const char* current, const char* new_name);
	void run_mouse_in_circles(HWND hw, int radius = 16, int times = 1);
}

// checks
namespace utils {
	bool exists(std::string file);
	bool does_acc_have_string(std::string steamid, std::string to_find);
	bool does_file_contain_string(std::string path, std::string to_find);
	void set_process_priority(DWORD pid, int priority);
}

namespace utils {
	void place_prerequisites(std::string steam_id);
	void place_all_prerequisites(c_lobby lobby);
	void copy_file(std::string input_path, std::string output_path);
	void copy_folder(std::string input_path, std::string output_path);
}

namespace utils {
	void send_char(HWND hw, UINT virtual_key);
	void send_key(HWND hw, UINT virtual_key);
	void left_click(HWND hw, int x, int y);
	void right_click(HWND hw, int x, int y);

	// Middle mouse button, currently only used to disconnect!
	void send_middlemouse(HWND hw, int x, int y);
	bool create_logs_folder();
}

namespace utils {
	void eternal_sleep();
	void kill_process_by_exe(const char* exe_name);
	BOOL terminate_pid(DWORD pid);
	DWORD get_pid(std::string exe_name);
	MODULEENTRY32 get_module_by_name(const char* module_name, DWORD pid);
	void messagebox(const char* description, const char* title = g_cfg.title);
}

namespace utils {
	void kill_all_cs();
	void kill_steam_stock();
	void kill_pid(DWORD pid);
	void kill_all_steam();
}

namespace utils {
	void clear_file(std::string path);
	void clear_logfile(std::string steam_id);
	void clear_every_log(c_lobby lobby);
}