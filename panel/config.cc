#include "config.hh"
#include <utils/utils.hh>

c_config::c_config() {
	if (utils::exists("C:\\Program Files (x86)\\Steam\\Steam.exe"))
		g_cfg.steam_directory = "C:\\Program Files (x86)\\Steam";

	if (utils::exists("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\csgo.exe"))
		g_cfg.csgo_directory = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive";
}