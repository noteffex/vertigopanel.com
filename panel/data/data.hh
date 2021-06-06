#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <utils/json.hpp>

struct acc_info {
	std::string user	= "";
	std::string pass	= "";
	std::string code	= "";
	std::string s_id	= "";
	int			w_id	= 00;
	int			t_id	= 00;
	int			idx		= 00;
	bool   is_banned	= false;
};

class cdata {
public:
	cdata();
	void imp_from_txt();
	void exp();
public:
	acc_info get(int idx);
	acc_info get(std::string user);
	void delete_accs_from_db(int team_id);
	void set_code(int idx, std::string code);
	void set_steam(int idx, size_t code);
	std::vector<acc_info> get_accs_team(int t_id, int leader_swap = -1);
	std::vector<acc_info> get_accs_wingman(int w_id);
	std::vector<acc_info> accs;
	std::vector<std::string> match_ids;
	std::map<std::string, HWND> hwnds;
private:
	nlohmann::json obj;
};

inline cdata data;