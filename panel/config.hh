#pragma once
#include <string>
#include <unordered_map>

class c_config {
public:
	c_config();
	int log_to{ 0 };
	std::string server_version{ "69420" };
	std::string username{ "" };
	std::string password{ "" };
public:
	int page{ 0 };
	int language{ 1 };
public:
	bool active{ true };
	int active_windows{ 0 };
public:
	bool logged_in = false;
	bool should_login = false;
	bool should_register_key = false;
public:
	const char* title = "Vertigo Panel";
	bool login_thread_created = false;
	bool set_key_thread_created = false;
public:
	int login_epoch{ 0 };
	int key_type{ 3 };
	int key_validity{ 86400 };
	int key_first_use{ 86400 };
	bool has_spare_key{ false };
public:
	std::string product_key{ "" };
	std::string webhook_url{ "" };
	std::string assets_folder{ "" };
	std::string csgo_directory{ "" };
	std::string steam_directory{ "" };
public:
	int64_t last_steam_id_parse{ 0 };
	bool is_parsing_on_cooldown{ false };
	std::string app_version{ "1.0.5" };
	float time_to_wait_for_match{ 15 };
};

inline c_config g_cfg;
inline std::unordered_map<std::string, bool> locks;