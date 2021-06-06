#pragma once
#include <vector>

class c_lobby;
struct acc_info;
class std::thread;

namespace boosting {
	void click_on_a_coord_on_every_acc(c_lobby lobby, int x, int y);
	void cant_find_a_match_in_15_minutes(c_lobby& lobby);
	int64_t find_match_id(std::string steam_id);
	void toggle_windows(c_lobby lobby);
	void initialize_search(HWND hwnd, bool wingman = false);
	bool check_if_connected(std::string steam_id);
	bool check_if_disconnected(std::string steam_id);
	bool check_if_loading(std::string steam_id);
	bool did_accept_button_appear(std::string steam_id);
	bool did_accept_button_disappear(std::string steam_id);
	bool did_enemies_accept(std::string steam_id);
	bool did_lobby_start_searching(std::string steam_id);
	bool check_session_error(std::string steam_id);
	int get_number_of_members_in_lobby(std::string steam_id);
	void re_invite_account(HWND leader_hwnd, std::string friend_code, std::string steam_id, int offset);
	void re_invite_account_wingman(HWND leader_hwnd, std::string friend_code, std::string steam_id);
	void make_sure_every_acc_joined(c_lobby lobby);
	void make_sure_every_acc_joined_wingman(c_lobby lobby);
	bool did_lobby_stop_searching(std::string steam_id);
	bool did_acc_join_lobby(std::string steam_id);
}

namespace boosting {
	inline std::vector<std::thread> threads;

	void get_teams(c_lobby& lobby);
	void verify_handles(c_lobby& lobby);
	void draw(c_lobby lobby);
	void win_lose(c_lobby lobby, bool start = false);
	void winwin_loselose(c_lobby lobby, bool start = false);
	void rank_unlocker(c_lobby lobby, bool start = false);
	void team1_win(c_lobby lobby, bool rounds);
	void team2_win(c_lobby lobby, bool rounds);
	void update_round(c_lobby& lobby, bool team_2 = false);
	void reset_rounds(c_lobby& lobby);
	void disconnection_check(c_lobby lobby, bool* lock_to_dispell = nullptr);
	void stop_boosting(c_lobby lobby, bool* lock_to_dispell = nullptr);
	void stop_boosting_and_kill(c_lobby lobby, bool* lock_to_dispell = nullptr);
	void terminate_if_needed(c_lobby lobby, bool* lock_to_dispell = nullptr);
	bool should_launch_accounts(lobby_state state);
	void clicker1(c_lobby lobby, int team);
	void clicker2(c_lobby lobby, int team);
	void invite_team1(c_lobby lobby);
	void invite_team2(c_lobby lobby);
	void leave_lobby(std::vector<acc_info> accs);
	void update_time(c_lobby& lobby, which_time thonk);
	
	void keep_searching(std::string leader_1_steam, std::string leader_2_steam);

	void dc(std::string steam_id);
	void dcall(c_lobby lobby);
	void rc(std::string steam_id);
	void dc_check(c_lobby, std::string steam_id);
	void rc_check(c_lobby, std::string steam_id);
	size_t get_current_steam_id();
	void get_steam_id(int acc_idx);
	void get_all_steam_ids();
	void draw_wingman(c_lobby& lobby);
	void derank_auto_accept(c_lobby& lobby);
	void alternate_wins_wingman(c_lobby& lobby);
	void team_win_wingman(c_lobby& lobby, bool team_2);
	void boost_wingman(c_lobby& lobby);
	void boost_competitive(c_lobby& lobby);
	void worker_thread(c_lobby lobby);
	void wait_for_all_accounts_to_connect(c_lobby lobby);

	void auto_accept(c_lobby& lobby);
	void auto_accept_wingman(c_lobby& lobby);

	void long_sleep(c_lobby lobby, long long ms);
	void hit_accept(std::vector<acc_info> accs);
	void kill_cs_lobby(c_lobby lobby);
	void invite_accounts_team(c_lobby lobby, std::vector<acc_info> team);
	void join_lobby(HWND hwnd, bool wingman = false);
	void invite_accounts_lobby(c_lobby lobby);
	void start_boosting(c_lobby lobby);
	void launch_accounts(c_lobby lobby);
	void wait_for_windows(c_lobby& lobby);
}

namespace boosting {
	std::string parse_friend_code(HWND window);
	void get_friend_codes(c_lobby lobby);
	void paste_code(HWND hw, std::string friend_code, int offset);
}

namespace boosting {
	void reset_data(c_lobby& lobby);
	void full_reset_data(c_lobby& lobby);
}

namespace boosting {
	void update_state(c_lobby& lobby, lobby_state state);
	void update_manual_state(c_lobby& lobby, manual_lobby_state state);
	void reset_termination(c_lobby& lobby);
	void update_completion(c_lobby& lobby);
}
