#include <thread>
#include <vector>

#include <fstream>
#include <config.hh>
#include <lobbies.hh>
#include <utils/utils.hh>

#include "boosting.hh"
#include <data/data.hh>
#include <clip/clip.hh>
#include <anti debug/anti dbg main.hh>

void boosting::derank_auto_accept(c_lobby& lobby) {
	lobby.found_a_match = false;

	auto accs = data.get_accs_team(lobby.team_selected_for_derank);
	while (!boosting::check_if_connected(accs[0].s_id)) {
		int64_t match_id = boosting::find_match_id(accs[0].s_id);

		if (match_id) {
			boosting::long_sleep(lobby, 5000);
			std::vector<int64_t> match_ids;

			bool should_accept = true;
			bool is_other_lobby_on_derank_as_well = false;

			for (int i = 0; i < MAX_LOBBIES; i++) {
				if (i == lobby.lobby_idx) continue;
				if (lobbies[i].current_state != lobby_state::searching_for_game) continue;
				if (lobbies[i].game_type != lobby.game_type) continue;

				if (is_other_lobby_on_derank_as_well) {
					auto team = data.get_accs_team(lobbies[i].team_selected_for_derank);

					if (!team.empty()) {
						auto mid = boosting::find_match_id(team[0].s_id);
						if (mid == match_id) {
							should_accept = false;
							break;
						}
					}
				}
				else {
					auto team1 = data.get_accs_team(lobbies[i].team_1);
					auto team2 = data.get_accs_team(lobbies[i].team_2);

					if (!team1.empty()) {
						auto mid = boosting::find_match_id(team1[0].s_id);
						if (mid == match_id) {
							should_accept = false;
							break;
						}
					}

					if (!team2.empty()) {
						auto mid = boosting::find_match_id(team2[0].s_id);
						if (mid == match_id) {
							should_accept = false;
							break;
						}
					}
				}
			}

			if (should_accept) {
				// accept
			}
			else {

			}
		}
		else {

		}
	}
}

void boosting::auto_accept_wingman(c_lobby& lobby) {
	lobby.found_a_match = false;

	auto accs = utils::get_all_accs_wingman(lobby);
	auto team1 = data.get_accs_wingman(lobby.team_1);
	auto team2 = data.get_accs_wingman(lobby.team_2);

	while (!boosting::check_if_connected(team1[0].s_id)) {
		int asdasdasdas = int(g_cfg.time_to_wait_for_match * 60);
		if ((lobby.searching_started_epoch + asdasdasdas) < utils::epoch_time()) {
			// match not found for 15 minutes switch accs
			utils::cout_lobby(lobby, "Team %i and team %i didn't find a match in 15 minutes", lobby.team_1, lobby.team_2);
			cant_find_a_match_in_15_minutes(lobby);
			break;
		}

		boosting::terminate_if_needed(lobby);
		utils::sleep(1000);

		int64_t matchid1 = boosting::find_match_id(team1[0].s_id);
		int64_t matchid2 = boosting::find_match_id(team2[0].s_id);

		utils::sleep(1000);

		matchid1 = boosting::find_match_id(team1[0].s_id);
		matchid2 = boosting::find_match_id(team2[0].s_id);

		if (!matchid1 && !matchid2) {
			boosting::keep_searching(team1[0].s_id, team2[0].s_id);
		}

		else if (!matchid1 && matchid2) {
			utils::cout_lobby(lobby, "Team %i got a match but team %i didnt, match id: %s", lobby.team_2, lobby.team_1, std::to_string(matchid2).c_str());
			utils::left_click(data.hwnds[team1[0].s_id], 192, 144);

			boosting::update_state(lobby, lobby_state::mismatch);
			int z = 0;
			while (!boosting::did_accept_button_appear(team2[0].s_id)) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(100);
				z++;
				if (z > 50) {
					utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
					break;
				}
			}
			utils::sleep(1000);
			utils::clear_every_log(lobby);
			while (!boosting::did_accept_button_disappear(team2[0].s_id)) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(1000);
			}
			utils::sleep(1000);
			if (!boosting::did_lobby_stop_searching(team2[0].s_id)) {
				utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			}
			utils::clear_every_log(lobby);
			boosting::update_state(lobby, lobby_state::lobby_search_on_cooldown);
			boosting::long_sleep(lobby, 60000);
			utils::clear_every_log(lobby);
			utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			boosting::update_state(lobby, lobby_state::searching_for_game);
		}

		else if (matchid1 && !matchid2) {
			utils::cout_lobby(lobby, "Team %i got a match but team %i didnt, match id: %s", lobby.team_1, lobby.team_2, std::to_string(matchid1).c_str());
			utils::left_click(data.hwnds[team2[0].s_id], 192, 144);

			boosting::update_state(lobby, lobby_state::mismatch);
			int z = 0;
			while (!boosting::did_accept_button_appear(team1[0].s_id)) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(100);
				z++;
				if (z > 50) {
					utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
					break;
				}
			}
			utils::sleep(1000);
			utils::clear_every_log(lobby);
			while (!boosting::did_accept_button_disappear(team1[0].s_id)) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(1000);
			}
			utils::sleep(1000);
			if (!boosting::did_lobby_stop_searching(team1[0].s_id)) {
				utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			}
			utils::clear_every_log(lobby);
			boosting::update_state(lobby, lobby_state::lobby_search_on_cooldown);
			boosting::long_sleep(lobby, 60000);
			utils::clear_every_log(lobby);
			utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			utils::sleep(2000);
			boosting::update_state(lobby, lobby_state::searching_for_game);
		}

		else if (matchid1 != matchid2) {
			//utils::cout_lobby(lobby, "Both team %i & team %i got a match but they're not in the same game, match ids: %s, %s", lobby.team_1, lobby.team_2, std::to_string(matchid1), std::to_string(matchid2));
			boosting::update_state(lobby, lobby_state::mismatch);
			while ((!boosting::did_accept_button_appear(team1[0].s_id)) && (!boosting::did_accept_button_appear(team2[0].s_id))) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(500);
			}
			utils::sleep(1000);
			utils::clear_every_log(lobby);
			while ((!boosting::did_accept_button_disappear(team1[0].s_id)) && (!boosting::did_accept_button_disappear(team2[0].s_id))) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(500);
			}
			utils::sleep(1000);
			if (!boosting::did_lobby_stop_searching(team1[0].s_id)) {
				utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			}
			if (!boosting::did_lobby_stop_searching(team2[0].s_id)) {
				utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			}
			utils::clear_every_log(lobby);
			boosting::update_state(lobby, lobby_state::lobby_search_on_cooldown);
			boosting::long_sleep(lobby, 60000);
			utils::cout_lobby(lobby, "Re initialized search");
			utils::clear_every_log(lobby);
			utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			boosting::update_state(lobby, lobby_state::searching_for_game);
		}

		else if (matchid1 == matchid2) {
			lobby.found_a_match = true;
			utils::cout_lobby(lobby, "Found a match");
			while (!boosting::did_accept_button_appear(team1[0].s_id)) {
				utils::sleep(500);
			}
			utils::sleep(4000);
			boosting::update_state(lobby, lobby_state::match_found);

			std::thread ac1(boosting::hit_accept, team1);
			std::thread ac2(boosting::hit_accept, team2);
			ac1.join();
			ac2.join();

			utils::clear_every_log(lobby);
		}
	}
}

void boosting::alternate_wins_wingman(c_lobby& lobby) {
	team_win_wingman(lobby, lobby.start_with_team2);

	utils::cout_lobby(lobby, "Inviting accounts");
	boosting::update_state(lobby, lobby_state::making_lobby);

	invite_accounts_lobby(lobby);
	utils::clear_every_log(lobby);

	boosting::update_state(lobby, lobby_state::in_lobby);
	utils::cout_lobby(lobby, "Every account joined the lobby");
	boosting::terminate_if_needed(lobby);

	utils::clear_every_log(lobby);

	std::thread init_search_one_thread(initialize_search, lobby.game_type == 0 ? data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id] : data.hwnds[data.get_accs_wingman(lobby.team_1)[0].s_id], true);
	std::thread init_search_two_thread(initialize_search, lobby.game_type == 0 ? data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id] : data.hwnds[data.get_accs_wingman(lobby.team_2)[0].s_id], true);

	init_search_one_thread.join();
	init_search_two_thread.join();

	boosting::terminate_if_needed(lobby);

	utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
	boosting::update_state(lobby, lobby_state::searching_for_game);
	boosting::update_time(lobby, which_time::time_since_started_searching);

	auto_accept_wingman(lobby);
	boosting::update_state(lobby, lobby_state::joining_game);

	if (lobby.found_a_match) {
		boosting::update_state(lobby, lobby_state::in_warm_up);

		boosting::long_sleep(lobby, 69000);
		boosting::update_state(lobby, lobby_state::in_game);
		utils::clear_every_log(lobby);

		team_win_wingman(lobby, !lobby.start_with_team2);
	}
}

void boosting::team_win_wingman(c_lobby& lobby, bool team_2) {
	auto accs = data.get_accs_wingman(team_2 ? lobby.team_2 : lobby.team_1);

	// 2nd team initial disconnection ( get one round )
	{
		utils::cout_lobby(lobby, "Initializing win logic for Team %i", lobby.team_2);
		boosting::dc(accs[0].s_id);
		boosting::dc(accs[1].s_id);

		boosting::dc_check(lobby, accs[0].s_id);
		boosting::dc_check(lobby, accs[1].s_id);

		update_round(lobby, true);
	}

	// get rest of the rounds ( 8 )
	for (int i = 0; i < 8; i++) {
		boosting::rc(accs[i % 2 == 0].s_id);
		boosting::rc_check(lobby, accs[i % 2 == 0].s_id);

		utils::sleep(1500);

		boosting::dc(accs[i % 2 == 0].s_id);
		boosting::dc_check(lobby, accs[i % 2 == 0].s_id);

		update_round(lobby, true);

		if (i == 6) { // half time
			utils::clear_every_log(lobby);
			boosting::long_sleep(lobby, 7000);
		}
	}

	boosting::long_sleep(lobby, 10000);

	utils::send_key(data.hwnds[accs[0].s_id], VK_F6);
	utils::send_key(data.hwnds[accs[1].s_id], VK_F6);

	auto accs2 = data.get_accs_wingman(!team_2 ? lobby.team_2 : lobby.team_1);
	utils::send_key(data.hwnds[accs2[0].s_id], VK_F6);
	utils::send_key(data.hwnds[accs2[1].s_id], VK_F6);

	boosting::long_sleep(lobby, 15000);
}

void boosting::draw_wingman(c_lobby& lobby) {
	auto v1 = data.get_accs_wingman(lobby.team_1);
	auto v2 = data.get_accs_wingman(lobby.team_2);
	
	// initial disconnection ( get one round )
	utils::cout_lobby(lobby, "Initializing draw logic for Team %i", lobby.team_1);
	{
		boosting::dc(v1[0].s_id);
		boosting::dc(v1[1].s_id);

		boosting::dc_check(lobby, v1[0].s_id);
		boosting::dc_check(lobby, v1[1].s_id);

		update_round(lobby);
	}

	// get rest of the rounds ( 7 )
	for (int i = 0; i < 7; i++) {
		boosting::rc(v1[i % 2 == 0].s_id);
		boosting::rc_check(lobby, v1[i % 2 == 0].s_id);

		utils::sleep(1500);

		boosting::dc(v1[i % 2 == 0].s_id);
		boosting::dc_check(lobby, v1[i % 2 == 0].s_id);

		update_round(lobby);
	}

	// 8 rounds complete now reconnect
	{
		boosting::rc(v1[0].s_id);
		boosting::rc(v1[1].s_id);

		boosting::rc_check(lobby, v1[0].s_id);
		boosting::rc_check(lobby, v1[1].s_id);
	}

	// half time
	{
		utils::clear_every_log(lobby);
		boosting::long_sleep(lobby, 7000);
	}

	// 2nd team initial disconnection ( get one round )
	{
		utils::cout_lobby(lobby, "Initializing draw logic for Team %i", lobby.team_2);
		boosting::dc(v2[0].s_id);
		boosting::dc(v2[1].s_id);

		boosting::dc_check(lobby, v2[0].s_id);
		boosting::dc_check(lobby, v2[1].s_id);

		update_round(lobby, true);
	}

	// get rest of the rounds ( 7 )
	for (int i = 0; i < 7; i++) {
		boosting::rc(v2[i % 2 == 0].s_id);
		boosting::rc_check(lobby, v2[i % 2 == 0].s_id);

		utils::sleep(1500);

		boosting::dc(v2[i % 2 == 0].s_id);
		boosting::dc_check(lobby, v2[i % 2 == 0].s_id);

		update_round(lobby, true);
	}

	// re connect all accounts just incase
	{
		boosting::rc(v1[0].s_id);
		boosting::rc(v1[1].s_id);

		boosting::rc(v2[0].s_id);
		boosting::rc(v2[1].s_id);
	}

	// wait 5 seconds
	{
		boosting::long_sleep(lobby, 5000);
	}

	// disconnect
	{
		utils::send_key(data.hwnds[v1[0].s_id], VK_F6);
		utils::send_key(data.hwnds[v1[1].s_id], VK_F6);

		utils::send_key(data.hwnds[v2[0].s_id], VK_F6);
		utils::send_key(data.hwnds[v2[1].s_id], VK_F6);
	}
	utils::cout_lobby(lobby, "Finished draw logic for Team %i & %i", lobby.team_1, lobby.team_2);
}

bool getting_friend_codes = false;
void boosting::boost_wingman(c_lobby& lobby) {
	if (should_launch_accounts(lobby.current_state)) {
		boosting::update_state(lobby, lobby_state::setting_up);

		if (g_cfg.key_type == 1)
			get_teams(lobby);

		// copy needed files
		utils::place_all_prerequisites(lobby);

		// clear old logs as you dont want old log files to interfere
		utils::clear_every_log(lobby);

		// change config file so we get the map we need
		utils::change_map_all(lobby);

		while (locks["launching_account"]) {
			utils::sleep(2000);
		}

		locks["launching_account"] = true;

		// login to all the accounts
		utils::cout_lobby(lobby, "Team %i & team %i are launching accounts", lobby.team_1, lobby.team_2);

		launch_accounts(lobby);
	}

	boosting::update_state(lobby, lobby_state::waiting_for_cs_to_launch);

	wait_for_windows(lobby);
	boosting::update_state(lobby, lobby_state::verifying_handles);

	verify_handles(lobby);
	if (g_cfg.key_type == 1) {
		boosting::update_state(lobby, lobby_state::got_handles);
	}

	locks["launching_account"] = false;

	utils::sleep(3000);

	if (g_cfg.key_type == 2) {
		while (lobbies[lobby.lobby_idx].current_state == lobby_state::verifying_handles) {
			utils::sleep(1000);
		}
	}

	while (getting_friend_codes) {
		utils::sleep(1000);
	}

	getting_friend_codes = true;
	boosting::update_state(lobby, lobby_state::getting_friend_codes);
	boosting::get_friend_codes(lobby);
	getting_friend_codes = false;

	while (lobbies[lobby.lobby_idx].times_boosted < lobby.repetition)
	{
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		utils::clear_every_log(lobby);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");
		boosting::terminate_if_needed(lobby);

		utils::clear_every_log(lobby);

		utils::clear_every_log(lobby);
		std::thread init_search_one_thread(initialize_search, lobby.game_type == 0 ? data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id] : data.hwnds[data.get_accs_wingman(lobby.team_1)[0].s_id], true);
		std::thread init_search_two_thread(initialize_search, lobby.game_type == 0 ? data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id] : data.hwnds[data.get_accs_wingman(lobby.team_2)[0].s_id], true);

		init_search_one_thread.join();
		init_search_two_thread.join();

		boosting::terminate_if_needed(lobby);

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);
		boosting::update_time(lobby, which_time::time_since_started_searching);

		auto_accept_wingman(lobby);
		boosting::update_state(lobby, lobby_state::joining_game);

		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);

			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			utils::clear_every_log(lobby);
			if (g_cfg.key_type == 1) {
				switch (lobby.match_mode) {
				case 0:
					boosting::draw_wingman(lobby);
					break;
				case 1:
					boosting::alternate_wins_wingman(lobby);
					break;
				case 2:
					boosting::team_win_wingman(lobby, false);
					break;
				case 3:
					boosting::team_win_wingman(lobby, true);
					break;
				default:
					boosting::draw_wingman(lobby);
					break;
				}
			}
			else {
				boosting::draw_wingman(lobby);
			}

			utils::sleep(5000);
			update_completion(lobby);
		}
		else {
			break;
		}
	}

	reset_data(lobby);
	boosting::kill_cs_lobby(lobby);
	utils::sleep(10000);
}

void boosting::keep_searching(std::string leader_1_steam, std::string leader_2_steam) {
	if (boosting::did_lobby_stop_searching(leader_1_steam)) {
		utils::clear_logfile(leader_1_steam);
		utils::left_click(data.hwnds[leader_1_steam], 192, 144);
	}
	if (boosting::did_lobby_stop_searching(leader_2_steam)) {
		utils::clear_logfile(leader_2_steam);
		utils::left_click(data.hwnds[leader_2_steam], 192, 144);
	}
}

void boosting::dc(std::string steam_id) {
	utils::send_key(data.hwnds[steam_id], VK_F6);
}

void boosting::dcall(c_lobby lobby) {
	auto accs = utils::get_all_accs(lobby);
	for (size_t i{}; i < accs.size(); i++) {
		boosting::dc(accs[i].s_id);
	}
}

void boosting::rc(std::string steam_id) {
	utils::left_click(data.hwnds[steam_id], 194, 7);
}

void boosting::dc_check(c_lobby lobby, std::string steam_id) {
check:
	int y = 0;
	while (!boosting::check_if_disconnected(steam_id)) {
		boosting::terminate_if_needed(lobby);
		utils::sleep(1000);
		y++;
		if (y > 5) {
			boosting::dc(steam_id);
			goto check;
		}
	}
}

void boosting::rc_check(c_lobby lobby, std::string steam_id) {
check:
	int x = 0;
	while (!boosting::check_if_loading(steam_id)) {
		boosting::terminate_if_needed(lobby);
		utils::sleep(1000);
		x++;
		if (x > 10) {
			boosting::rc(steam_id);
			goto check;
		}
	}

	while (!boosting::check_if_connected(steam_id)) {
		boosting::terminate_if_needed(lobby);
		utils::sleep(1000);
	}
	utils::sleep(1500);
	utils::clear_logfile(steam_id);
}

size_t boosting::get_current_steam_id()
{
	size_t k;
	DWORD charSize = sizeof(size_t) * 255;
	(RegGetValueA)(HKEY_CURRENT_USER, "Software\\Valve\\Steam\\ActiveProcess", "ActiveUser", RRF_RT_ANY, nullptr, &k, &charSize);
	return k;
}

void wait_for_steam_id_update(size_t current) {
	auto k = utils::epoch_time();
	while (current == boosting::get_current_steam_id() || boosting::get_current_steam_id() == 0 || boosting::get_current_steam_id() < 1000000000) {
		utils::sleep(200);

		if (utils::epoch_time() > k + 30 && boosting::get_current_steam_id() > 1000000000) {
			break;
		}
	}
}

int num_of_accs_parsed{ 0 };
void boosting::get_steam_id(int acc_idx) {
	acc_info acc = data.get(acc_idx);
	while (locks["get_steam_id_lock"]) utils::sleep(500);

	locks["get_steam_id_lock"] = true;
	{
		if (acc.s_id.empty() || acc.s_id.size() != 10) {
			int old_steam_id = boosting::get_current_steam_id();

			utils::login_to_acc(acc.user, acc.pass);
			wait_for_steam_id_update(old_steam_id);

			data.set_steam(acc_idx, boosting::get_current_steam_id());

			data.exp();

			utils::kill_all_steam();
			num_of_accs_parsed++;
		}
		//place_files(data.accs[acc_idx].s_id);
	}
	locks["get_steam_id_lock"] = false;
}

void boosting::get_all_steam_ids() {
	for (size_t i{}; i < data.accs.size(); i++) {
		if (num_of_accs_parsed >= 5) {
			if (!g_cfg.is_parsing_on_cooldown)
			{
				g_cfg.last_steam_id_parse = utils::epoch_time();
				g_cfg.is_parsing_on_cooldown = true;
				utils::sleep(30000); // 2 minutes
				num_of_accs_parsed = 0;
				g_cfg.is_parsing_on_cooldown = false;
			}
		}
		g_cfg.last_steam_id_parse = utils::epoch_time();
		boosting::get_steam_id(i);
	}
}

void boosting::boost_competitive(c_lobby& lobby) {
	if (should_launch_accounts(lobby.current_state)) {
		boosting::update_state(lobby, lobby_state::setting_up);

		if (g_cfg.key_type == 1)
			get_teams(lobby);

		// copy needed files
		utils::place_all_prerequisites(lobby);

		// clear old logs as you dont want old log files to interfere
		utils::clear_every_log(lobby);

		// change config file so we get the map we need
		utils::change_map_all(lobby);

		while (locks["launching_account"]) {
			utils::sleep(2000);
		}

		locks["launching_account"] = true;

		// login to all the accounts
		utils::cout_lobby(lobby, "Team %i & team %i are launching accounts", lobby.team_1, lobby.team_2);

		launch_accounts(lobby);
	}

	boosting::update_state(lobby, lobby_state::waiting_for_cs_to_launch);

	wait_for_windows(lobby);
	boosting::update_state(lobby, lobby_state::verifying_handles);

	verify_handles(lobby);
	if (g_cfg.key_type == 1) {
		boosting::update_state(lobby, lobby_state::got_handles);
	}

	locks["launching_account"] = false;

	utils::sleep(3000);

	if (g_cfg.key_type == 2) {
		while (lobbies[lobby.lobby_idx].current_state == lobby_state::verifying_handles) {
			utils::sleep(1000);
		}
	}

	while (getting_friend_codes) {
		utils::sleep(1000);
	}

	getting_friend_codes = true;
	boosting::update_state(lobby, lobby_state::getting_friend_codes);
	boosting::get_friend_codes(lobby);
	getting_friend_codes = false;

	if (lobby.match_mode == 6) {
		lobby.repetition = 1;
	}
	while (lobbies[lobby.lobby_idx].times_boosted < lobby.repetition)
	{
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		utils::clear_every_log(lobby);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");
		boosting::terminate_if_needed(lobby);

		utils::clear_every_log(lobby);
		std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
		std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

		init_search_one_thread.join();
		init_search_two_thread.join();

		boosting::terminate_if_needed(lobby);

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);
		boosting::update_time(lobby, which_time::time_since_started_searching);

		auto_accept(lobby);
		boosting::update_state(lobby, lobby_state::joining_game);

		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);

			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);

			if (g_cfg.key_type == 1) {
				switch (lobby.match_mode) {
				case 0:
					boosting::draw(lobby);
					break;
				case 1:
					boosting::win_lose(lobby, lobby.start_with_team2);
					break;
				case 2:
					boosting::winwin_loselose(lobby, lobby.start_with_team2);
					break;
				case 4:
					boosting::team1_win(lobby, true);
					utils::sleep(5000);
					boosting::dcall(lobby);
					break;
				case 5:
					boosting::team2_win(lobby, true);
					utils::sleep(5000);
					boosting::dcall(lobby);
					break;
				case 6:
					boosting::rank_unlocker(lobby, lobby.start_with_team2);
					break;
				default:
					boosting::draw(lobby);
					break;
				}
			}
			else {
				boosting::draw(lobby);
			}

			utils::sleep(5000);
			update_completion(lobby);
		}
		else {
			break;
		}
	}

	reset_data(lobby);
	boosting::kill_cs_lobby(lobby);
	utils::sleep(10000);
}

void boosting::worker_thread(c_lobby lobby) {
	for (size_t i{}; i < 500; i++) {
		lobbies[lobby.lobby_idx].didnt_find_a_match[i] = {};
		lobbies[lobby.lobby_idx].has_team_been_boosted[i] = {};
	}

	anti_debug::hide_thread_from_debugger();

	for (int i{}; i < lobby.valid_lobbies; i++) {
		switch (lobby.game_type) {
		default:
		case 0: boost_competitive(lobby); break;
		case 1: boost_wingman(lobby); break;
		}
	}
	full_reset_data(lobby);
}

void boosting::update_time(c_lobby& lobby, which_time thonk) {
	lobby.searching_started_epoch = utils::epoch_time();
	lobbies[lobby.lobby_idx].searching_started_epoch = lobby.searching_started_epoch;
}

void boosting::leave_lobby(std::vector<acc_info> accs) {
	for (int r{}; r < 5; r++) {
		utils::sleep(1250);

		for (size_t i{ 0 }; i < accs.size(); i++) {
			utils::left_click(data.hwnds[accs[i].s_id], 243, 11);
		}
	}
}

void boosting::disconnection_check(c_lobby lobby, bool* lock_to_dispell) {
	bool disconnected = false;
	auto accs = utils::get_all_accs(lobby);

	for (size_t i{}; i < accs.size(); i++) {
		if (utils::does_acc_have_string(accs[i].s_id, "CClientSteamContext OnSteamServersDisconnected logged on = 0")) {
			disconnected = true;
			break;
		}
	}

	if (disconnected)
	{
		boosting::reset_rounds(lobby);
		utils::cout_lobby(lobby, "Disconnected");

		for (size_t i{}; i < accs.size(); i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}


		for (size_t i{}; i < accs.size(); i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}

		if (lock_to_dispell != nullptr) {
			*lock_to_dispell = false;
		}

		boosting::update_state(lobby, lobby_state::waiting_for_network_reconnection);

		bool reconnected = false;
		while (1) {
			for (size_t i{}; i < accs.size(); i++) {
				if (utils::does_acc_have_string(accs[i].s_id, "CClientSteamContext OnSteamServersConnected logged on = 1")) {
					reconnected = true;
					break;
				}
			}

			if (reconnected) {
				for (size_t i{}; i < accs.size(); i++) {
					utils::send_key(data.hwnds[accs[i].s_id], VK_ESCAPE);
					utils::send_key(data.hwnds[accs[i].s_id], VK_ESCAPE);
					utils::send_key(data.hwnds[accs[i].s_id], VK_ESCAPE);
					utils::send_key(data.hwnds[accs[i].s_id], VK_ESCAPE);
					utils::sleep(200);

					utils::left_click(data.hwnds[accs[i].s_id], 159, 83);
					utils::left_click(data.hwnds[accs[i].s_id], 159, 83);
					utils::sleep(200);

					utils::left_click(data.hwnds[accs[i].s_id], 147, 83);
					utils::left_click(data.hwnds[accs[i].s_id], 147, 83);
					utils::sleep(200);

					utils::left_click(data.hwnds[accs[i].s_id], 120, 80);
					utils::left_click(data.hwnds[accs[i].s_id], 120, 80);
					utils::sleep(200);

					SendMessageA(data.hwnds[accs[i].s_id], WM_MOUSEMOVE, 0, MAKELPARAM(243, 100));
					utils::sleep(200);
				}

				leave_lobby(accs);
				utils::cout_lobby(lobby, "Reconnected");
				utils::clear_every_log(lobby);
				break;
			}
			utils::sleep(2000);
		}

		boosting::long_sleep(lobby, 60000);

		utils::cout_lobby(lobby, "Restarting boosting");
		boosting::start_boosting(lobby);

		utils::eternal_sleep();
	}
}

void boosting::stop_boosting(c_lobby lobby, bool* lock_to_dispell) {
	auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);
	for (size_t i{}; i < accs.size(); i++) {
		utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
	}

	if (lock_to_dispell != nullptr) {
		*lock_to_dispell = false;
	}

	if (lobby.current_state == lobby_state::in_lobby || lobby.current_state == lobby_state::searching_for_game || lobby.current_state == lobby_state::mismatch || lobby.current_state == lobby_state::lobby_search_on_cooldown) {
		leave_lobby(accs);
	}

	for (size_t i{}; i < 500; i++) {
		lobbies[lobby.lobby_idx].has_team_been_boosted[i] = {};
	}

	boosting::update_state(lobby, lobby_state::terminated);
	boosting::reset_termination(lobby);
	utils::eternal_sleep();
}


void boosting::stop_boosting_and_kill(c_lobby lobby, bool* lock_to_dispell) {
	boosting::kill_cs_lobby(lobby);
	boosting::update_state(lobby, lobby_state::hard_terminated);

	if (lock_to_dispell != nullptr) {
		*lock_to_dispell = false;
	}

	for (size_t i{}; i < 500; i++) {
		lobbies[lobby.lobby_idx].didnt_find_a_match[i] = {};
		lobbies[lobby.lobby_idx].has_team_been_boosted[i] = {};
	}

	boosting::reset_termination(lobby);
	utils::eternal_sleep();
}

void boosting::terminate_if_needed(c_lobby lobby, bool* lock_to_dispell) {
	switch (lobbies[lobby.lobby_idx].termination_request) {
	case 1: boosting::stop_boosting(lobby, lock_to_dispell); break;
	case 2: boosting::stop_boosting_and_kill(lobby, lock_to_dispell); break;
	default: boosting::disconnection_check(lobby, lock_to_dispell); break;
	}
}

bool boosting::should_launch_accounts(lobby_state state) {
	switch (state) {
	default: return false;
	case lobby_state::zero:
	case lobby_state::finished:
	case lobby_state::fully_finished:
	case lobby_state::hard_terminated:
		return true;
	}
}

void boosting::clicker1(c_lobby lobby, int team) { // for lem gets 15 rounds and stops for user to take mvp 
	auto v3 = data.get_accs_team(lobby.team_2);
	for (size_t i{};i < v3.size();i++) {
		boosting::dc(v3[i].s_id);
	}
	update_round(lobby);

	utils::sleep(1000);

	for (size_t i{};i < v3.size();i++) {
		boosting::rc(v3[i].s_id);
	}

	for (size_t i{};i < v3.size();i++) {
		boosting::rc_check(lobby, v3[i].s_id);
	}

	utils::sleep(1000);
	for (size_t i{};i < v3.size();i++) {
		boosting::dc(v3[i].s_id);
	}
	update_round(lobby);

	utils::sleep(1000);

	for (int i{};i < 2;i++) {
		for (size_t i{};i < v3.size();i++) {
			boosting::rc(v3[i].s_id);
			boosting::rc_check(lobby, v3[i].s_id);
			utils::sleep(1000);
			boosting::dc(v3[i].s_id);
			utils::sleep(1000);
			update_round(lobby);
		}
	}

	for (int i{};i < 2;i++) {
		boosting::rc(v3[i].s_id);
		boosting::rc_check(lobby, v3[i].s_id);
		utils::sleep(1000);
		boosting::dc(v3[i].s_id);
		utils::sleep(1000);
		update_round(lobby);
	}

	utils::sleep(1000);

	for (size_t i{};i < v3.size();i++) {
		boosting::rc(v3[i].s_id);
	}

	for (size_t i{};i < v3.size();i++) {
		boosting::rc_check(lobby, v3[i].s_id);
	}

	update_round(lobby);

}

void boosting::clicker2(c_lobby lobby, int team) { // for lem gets 14 rounds first stops for 20 secs dc's to get 15 rounds used when user already took mvp
	auto v3 = data.get_accs_team(lobby.team_2);
	for (size_t i{};i < v3.size();i++) {
		boosting::dc(v3[i].s_id);
	}
	update_round(lobby);

	utils::sleep(1000);

	for (size_t i{};i < v3.size();i++) {
		boosting::rc(v3[i].s_id);
	}

	for (size_t i{};i < v3.size();i++) {
		boosting::rc_check(lobby, v3[i].s_id);
	}

	utils::sleep(1000);
	for (size_t i{};i < v3.size();i++) {
		boosting::dc(v3[i].s_id);
	}
	update_round(lobby);

	utils::sleep(1000);

	for (int i{};i < 2;i++) {
		for (size_t i{};i < v3.size();i++) {
			boosting::rc(v3[i].s_id);
			boosting::rc_check(lobby, v3[i].s_id);
			utils::sleep(1000);
			boosting::dc(v3[i].s_id);
			utils::sleep(1000);
			update_round(lobby);
		}
	}

	for (int i{};i < 1;i++) {
		boosting::rc(v3[i].s_id);
		boosting::rc_check(lobby, v3[i].s_id);
		utils::sleep(1000);
		boosting::dc(v3[i].s_id);
		utils::sleep(1000);
		update_round(lobby);
	}

	utils::sleep(1000);

	for (size_t i{};i < v3.size();i++) {
		boosting::rc(v3[i].s_id);
	}

	for (size_t i{};i < v3.size();i++) {
		boosting::rc_check(lobby, v3[i].s_id);
	}

	update_round(lobby);

	boosting::long_sleep(lobby, 20000);

	for (size_t i{};i < v3.size();i++) {
		boosting::dc(v3[i].s_id);
	}

	utils::sleep(1000);
}

void boosting::invite_team1(c_lobby lobby) {
	auto team1 = data.get_accs_team(lobby.team_1);
	for (size_t i{ 1 }; i < team1.size(); i++) {
		boosting::paste_code(data.hwnds[team1[0].s_id], team1[i].code, 0);
	}
	utils::sleep(2000);
	for (size_t i{ 1 }; i < team1.size(); i++) {
		utils::left_click(data.hwnds[team1[i].s_id], 145, 103);
		utils::sleep(200);
		SendMessageA(data.hwnds[team1[i].s_id], WM_MOUSEMOVE, 0, MAKELPARAM(243, 89));
		utils::sleep(250);
		SendMessageA(data.hwnds[team1[i].s_id], WM_MOUSEMOVE, 0, MAKELPARAM(243, 89));
		utils::sleep(250);
		utils::left_click(data.hwnds[team1[i].s_id], 242, 39);
	}
}

void boosting::invite_team2(c_lobby lobby) {
	auto team1 = data.get_accs_team(lobby.team_1);
	for (size_t i{ 1 }; i < team1.size(); i++) {
		boosting::paste_code(data.hwnds[team1[0].s_id], team1[i].code, 0);
	}
	utils::sleep(2000);
	for (size_t i{ 1 }; i < team1.size(); i++) {
		utils::left_click(data.hwnds[team1[i].s_id], 145, 103);
		utils::sleep(200);
		SendMessageA(data.hwnds[team1[i].s_id], WM_MOUSEMOVE, 0, MAKELPARAM(243, 89));
		utils::sleep(250);
		SendMessageA(data.hwnds[team1[i].s_id], WM_MOUSEMOVE, 0, MAKELPARAM(243, 89));
		utils::sleep(250);
		utils::left_click(data.hwnds[team1[i].s_id], 242, 39);
	}
}

void boosting::wait_for_all_accounts_to_connect(c_lobby lobby) {
	auto accs = utils::get_all_accs(lobby);
	for (size_t i{};i < accs.size(); i++) {
		while (!boosting::check_if_connected(accs[i].s_id)) {
			boosting::terminate_if_needed(lobby);
			utils::sleep(100);
		}
	}
	utils::clear_every_log(lobby);
}

void boosting::update_round(c_lobby& lobby, bool team_2) {
	lobby.rounds_won[team_2]++;
	lobbies[lobby.lobby_idx].rounds_won[team_2]++;
	boosting::terminate_if_needed(lobby);
}

void boosting::reset_rounds(c_lobby& lobby) {
	lobby.rounds_won[0] = 0;
	lobby.rounds_won[1] = 0;
	lobbies[lobby.lobby_idx].rounds_won[0] = 0;
	lobbies[lobby.lobby_idx].rounds_won[1] = 0;
}

void boosting::click_on_a_coord_on_every_acc(c_lobby lobby, int x, int y) {
	auto accs = utils::get_all_accs(lobby);

	for (size_t i{}; i < accs.size(); i++) {
		utils::left_click(data.hwnds[accs[i].s_id], x, y);
	}
}

void boosting::cant_find_a_match_in_15_minutes(c_lobby& lobby) {
	lobby.didnt_find_a_match[lobby.team_1] = true;
	lobby.didnt_find_a_match[lobby.team_2] = true;

	lobbies[lobby.lobby_idx].didnt_find_a_match[lobby.team_1] = lobby.didnt_find_a_match[lobby.team_1];
	lobbies[lobby.lobby_idx].didnt_find_a_match[lobby.team_2] = lobby.didnt_find_a_match[lobby.team_2];

	lobbies[lobby.lobby_idx].times_boosted = lobby.repetition;

	lobby.found_a_match = false;
}

void boosting::auto_accept(c_lobby& lobby) {
	lobby.found_a_match = false;

	auto accs = utils::get_all_accs(lobby);
	auto team1 = data.get_accs_team(lobby.team_1);
	auto team2 = data.get_accs_team(lobby.team_2);

	while (!boosting::check_if_connected(team1[0].s_id)) {
		int asdasdasdas = int(g_cfg.time_to_wait_for_match * 60);
		if ((lobby.searching_started_epoch + asdasdasdas) < utils::epoch_time()) {
			// match not found for 15 minutes switch accs
			utils::cout_lobby(lobby, "Team %i and team %i didn't find a match in 15 minutes", lobby.team_1, lobby.team_2);
			cant_find_a_match_in_15_minutes(lobby);
			break;
		}

		boosting::terminate_if_needed(lobby);
		utils::sleep(1000);

		int64_t matchid1 = boosting::find_match_id(team1[0].s_id);
		int64_t matchid2 = boosting::find_match_id(team2[0].s_id);

		utils::sleep(1000);

		matchid1 = boosting::find_match_id(team1[0].s_id);
		matchid2 = boosting::find_match_id(team2[0].s_id);

		if (!matchid1 && !matchid2) {
			boosting::keep_searching(team1[0].s_id, team2[0].s_id);
		}

		else if (!matchid1 && matchid2) {
			utils::cout_lobby(lobby, "Team %i got a match but team %i didnt, match id: %s", lobby.team_2, lobby.team_1, std::to_string(matchid2).c_str());
			utils::left_click(data.hwnds[team1[0].s_id], 192, 144);

			boosting::update_state(lobby, lobby_state::mismatch);
			int z = 0;
			while (!boosting::did_accept_button_appear(team2[0].s_id)) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(100);
				z++;
				if (z > 50) {
					utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
					break;
				}
			}
			utils::sleep(1000);
			utils::clear_every_log(lobby);
			while (!boosting::did_accept_button_disappear(team2[0].s_id)) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(1000);
			}
			utils::sleep(1000);
			if (!boosting::did_lobby_stop_searching(team2[0].s_id)) {
				utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			}
			utils::clear_every_log(lobby);
			boosting::update_state(lobby, lobby_state::lobby_search_on_cooldown);
			boosting::long_sleep(lobby, 60000);
			utils::clear_every_log(lobby);
			utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			boosting::update_state(lobby, lobby_state::searching_for_game);
		}

		else if (matchid1 && !matchid2) {
			utils::cout_lobby(lobby, "Team %i got a match but team %i didnt, match id: %s", lobby.team_1, lobby.team_2, std::to_string(matchid1).c_str());
			utils::left_click(data.hwnds[team2[0].s_id], 192, 144);

			boosting::update_state(lobby, lobby_state::mismatch);
			int z = 0;
			while (!boosting::did_accept_button_appear(team1[0].s_id)) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(100);
				z++;
				if (z > 50) {
					utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
					break;
				}
			}
			utils::sleep(1000);
			utils::clear_every_log(lobby);
			while (!boosting::did_accept_button_disappear(team1[0].s_id)) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(1000);
			}
			utils::sleep(1000);
			if (!boosting::did_lobby_stop_searching(team1[0].s_id)) {
				utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			}
			utils::clear_every_log(lobby);
			boosting::update_state(lobby, lobby_state::lobby_search_on_cooldown);
			boosting::long_sleep(lobby, 60000);
			utils::clear_every_log(lobby);
			utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			utils::sleep(2000);
			boosting::update_state(lobby, lobby_state::searching_for_game);
		}

		else if (matchid1 != matchid2) {
			//utils::cout_lobby(lobby, "Both team %i & team %i got a match but they're not in the same game, match ids: %s, %s", lobby.team_1, lobby.team_2, std::to_string(matchid1), std::to_string(matchid2));
			boosting::update_state(lobby, lobby_state::mismatch);
			while ((!boosting::did_accept_button_appear(team1[0].s_id)) && (!boosting::did_accept_button_appear(team2[0].s_id))) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(500);
			}
			utils::sleep(1000);
			utils::clear_every_log(lobby);
			while ((!boosting::did_accept_button_disappear(team1[0].s_id)) && (!boosting::did_accept_button_disappear(team2[0].s_id))) {
				boosting::terminate_if_needed(lobby);
				utils::sleep(500);
			}
			utils::sleep(1000);
			if (!boosting::did_lobby_stop_searching(team1[0].s_id)) {
				utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			}
			if (!boosting::did_lobby_stop_searching(team2[0].s_id)) {
				utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			}
			utils::clear_every_log(lobby);
			boosting::update_state(lobby, lobby_state::lobby_search_on_cooldown);
			boosting::long_sleep(lobby, 60000);
			utils::cout_lobby(lobby, "Re initialized search");
			utils::clear_every_log(lobby);
			utils::left_click(data.hwnds[team1[0].s_id], 192, 144);
			utils::left_click(data.hwnds[team2[0].s_id], 192, 144);
			boosting::update_state(lobby, lobby_state::searching_for_game);
		}

		else if (matchid1 == matchid2) {
			lobby.found_a_match = true;
			utils::cout_lobby(lobby, "Found a match");
			while (!boosting::did_accept_button_appear(team1[0].s_id)) {
				utils::sleep(500);
			}
			utils::sleep(4000);
			boosting::update_state(lobby, lobby_state::match_found);

			std::thread ac1(boosting::hit_accept, team1);
			std::thread ac2(boosting::hit_accept, team2);
			ac1.join();
			ac2.join();

			utils::clear_every_log(lobby);
		}
	}
}

void boosting::long_sleep(c_lobby lobby, long long ms) {
	int parts = static_cast<int>(ms / 1000);
	int remainder = ms % 1000;

	utils::sleep(remainder);
	boosting::terminate_if_needed(lobby);

	for (int i{}; i < parts; i++) {
		utils::sleep(1000);
		boosting::terminate_if_needed(lobby);
	}
}

void boosting::draw(c_lobby lobby) {
	utils::cout_lobby(lobby, "Initializing draw logic for team %i", lobby.team_1);

	boosting::team1_win(lobby, false);
	boosting::long_sleep(lobby, 5000);

	utils::cout_lobby(lobby, "Initializing draw logic for team %i", lobby.team_2);
	boosting::team2_win(lobby, false);
	boosting::long_sleep(lobby, 5000);

	auto accs = utils::get_all_accs(lobby);
	for (size_t i{};i < accs.size();i++) {
		utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
	}

	boosting::terminate_if_needed(lobby);
}

void boosting::win_lose(c_lobby lobby, bool start) {
	if (start) {
		boosting::team2_win(lobby, true);
		reset_rounds(lobby);
		utils::sleep(5000);
		auto accs = utils::get_all_accs(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}

		utils::clear_every_log(lobby);
		utils::sleep(5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		utils::sleep(5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");

		utils::clear_every_log(lobby);
		std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
		std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

		init_search_one_thread.join();
		init_search_two_thread.join();

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);
		boosting::update_time(lobby, which_time::time_since_started_searching);

		auto_accept(lobby);

		boosting::update_state(lobby, lobby_state::joining_game);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team1_win(lobby, true);
		}
		reset_rounds(lobby);
		utils::sleep(5000);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		utils::sleep(1000);
	}
	else {
		boosting::team1_win(lobby, true);
		reset_rounds(lobby);
		utils::sleep(5000);
		auto accs = utils::get_all_accs(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}

		utils::clear_every_log(lobby);
		utils::sleep(5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		utils::sleep(5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");

		utils::clear_every_log(lobby);
		std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
		std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

		init_search_one_thread.join();
		init_search_two_thread.join();

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);
		boosting::update_time(lobby, which_time::time_since_started_searching);
		auto_accept(lobby);
		boosting::update_state(lobby, lobby_state::joining_game);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team2_win(lobby, true);
		}
		reset_rounds(lobby);
		utils::sleep(5000);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		utils::sleep(1000);
	}
}

void boosting::winwin_loselose(c_lobby lobby, bool start) {
	if (start) {
		boosting::team2_win(lobby, true);
		reset_rounds(lobby);
		utils::sleep(5000);
		auto accs = utils::get_all_accs(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		boosting::long_sleep(lobby, 5000);
		utils::clear_every_log(lobby);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		boosting::long_sleep(lobby, 5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");

		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);

		auto_accept(lobby);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team2_win(lobby, true);
		}
		utils::sleep(5000);
		reset_rounds(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}

		utils::clear_every_log(lobby);
		boosting::long_sleep(lobby, 5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		boosting::long_sleep(lobby, 5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");

		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);

		auto_accept(lobby);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team2_win(lobby, true);
		}
		reset_rounds(lobby);
		utils::sleep(5000);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		utils::clear_every_log(lobby);
		boosting::long_sleep(lobby, 5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		boosting::long_sleep(lobby, 5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");

		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);

		auto_accept(lobby);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team1_win(lobby, true);
		}
		utils::sleep(5000);
		reset_rounds(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		boosting::long_sleep(lobby, 5000);
		utils::clear_every_log(lobby);
	}
	else {
		boosting::team1_win(lobby, true);
		reset_rounds(lobby);
		utils::sleep(5000);
		auto accs = utils::get_all_accs(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		boosting::long_sleep(lobby, 5000);
		utils::clear_every_log(lobby);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		boosting::long_sleep(lobby, 5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");

		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);

		auto_accept(lobby);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team1_win(lobby, true);
		}
		utils::sleep(5000);
		reset_rounds(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}

		utils::clear_every_log(lobby);
		boosting::long_sleep(lobby, 5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		boosting::long_sleep(lobby, 5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");

		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);

		auto_accept(lobby);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team2_win(lobby, true);
		}
		reset_rounds(lobby);
		utils::sleep(5000);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		utils::clear_every_log(lobby);
		boosting::long_sleep(lobby, 5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		boosting::long_sleep(lobby, 5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");

		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);

		auto_accept(lobby);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team2_win(lobby, true);
		}
		utils::sleep(5000);
		reset_rounds(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		boosting::long_sleep(lobby, 5000);
		utils::clear_every_log(lobby);
	}
}

void boosting::rank_unlocker(c_lobby lobby, bool start) {
	if (start) {
		boosting::team2_win(lobby, true);
		reset_rounds(lobby);
		utils::sleep(5000);
		auto accs = utils::get_all_accs(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}

		utils::clear_every_log(lobby);
		utils::sleep(5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		utils::sleep(5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");
		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);
		boosting::update_time(lobby, which_time::time_since_started_searching);
		auto_accept(lobby);
		boosting::update_state(lobby, lobby_state::joining_game);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team1_win(lobby, true);
		}
		utils::sleep(5000);
		reset_rounds(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		utils::sleep(5000);
		utils::clear_every_log(lobby);
		utils::sleep(5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		utils::sleep(5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");
		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);
		boosting::update_time(lobby, which_time::time_since_started_searching);
		auto_accept(lobby);
		boosting::update_state(lobby, lobby_state::joining_game);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team2_win(lobby, true);
		}
		utils::sleep(5000);
		reset_rounds(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
	}
	else {
		boosting::team1_win(lobby, true);
		reset_rounds(lobby);
		utils::sleep(5000);
		auto accs = utils::get_all_accs(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}

		utils::clear_every_log(lobby);
		utils::sleep(5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		utils::sleep(5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");
		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);
		boosting::update_time(lobby, which_time::time_since_started_searching);
		auto_accept(lobby);
		boosting::update_state(lobby, lobby_state::joining_game);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team2_win(lobby, true);
		}
		utils::sleep(5000);
		reset_rounds(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
		utils::sleep(5000);
		utils::clear_every_log(lobby);
		utils::sleep(5000);
		utils::cout_lobby(lobby, "Inviting accounts");
		boosting::update_state(lobby, lobby_state::making_lobby);

		invite_accounts_lobby(lobby);
		utils::sleep(5000);

		boosting::update_state(lobby, lobby_state::in_lobby);
		utils::cout_lobby(lobby, "Every account joined the lobby");
		{
			utils::clear_every_log(lobby);
			std::thread init_search_one_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], false);
			std::thread init_search_two_thread(initialize_search, data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], false);

			init_search_one_thread.join();
			init_search_two_thread.join();
		}

		utils::cout_lobby(lobby, "Started searching for a match with Team %i & Team %i", lobby.team_1, lobby.team_2);
		boosting::update_state(lobby, lobby_state::searching_for_game);
		boosting::update_time(lobby, which_time::time_since_started_searching);
		auto_accept(lobby);
		boosting::update_state(lobby, lobby_state::joining_game);
		if (lobby.found_a_match) {
			boosting::update_state(lobby, lobby_state::in_warm_up);
			boosting::long_sleep(lobby, 69000);
			boosting::update_state(lobby, lobby_state::in_game);
			boosting::team1_win(lobby, true);
		}
		utils::sleep(5000);
		reset_rounds(lobby);
		for (size_t i{};i < accs.size();i++) {
			utils::send_key(data.hwnds[accs[i].s_id], VK_F6);
		}
	}
}

void boosting::team1_win(c_lobby lobby, bool rounds) {
	utils::clear_every_log(lobby);
	auto v3 = data.get_accs_team(lobby.team_2);
	//auto v3 = data.get_accs_team(lobby.team_2, lobby.team_1); // team 2's players but team 1's leader
	if (rounds) {
		for (size_t i{};i < v3.size();i++) {
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
		}
		update_round(lobby);
		utils::sleep(500);

		for (int i{};i < 2;i++) {
			for (size_t i{};i < v3.size();i++) {
				boosting::rc(v3[i].s_id);
				boosting::rc_check(lobby, v3[i].s_id);
				boosting::dc(v3[i].s_id);
				boosting::dc_check(lobby, v3[i].s_id);
				update_round(lobby);
			}
		}

		for (int i{};i < 4;i++) {
			boosting::rc(v3[i].s_id);
			boosting::rc_check(lobby, v3[i].s_id);
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
			update_round(lobby);
		}

		utils::sleep(1000);

		for (size_t i{};i < v3.size();i++) {
			boosting::rc(v3[i].s_id);
		}

		for (size_t i{};i < v3.size();i++) {
			boosting::rc_check(lobby, v3[i].s_id);
		}

		boosting::long_sleep(lobby, 5000);
		for (size_t i{};i < v3.size();i++) {
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
		}
		update_round(lobby); // 16th round

		utils::sleep(1000);

	}
	else {
		for (size_t i{};i < v3.size();i++) {
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
		}
		update_round(lobby);
		utils::sleep(500);

		for (int i{};i < 2;i++) {
			for (size_t i{};i < v3.size();i++) {
				boosting::rc(v3[i].s_id);
				boosting::rc_check(lobby, v3[i].s_id);
				boosting::dc(v3[i].s_id);
				boosting::dc_check(lobby, v3[i].s_id);
				update_round(lobby);
			}
		}

		for (int i{};i < 4;i++) {
			boosting::rc(v3[i].s_id);
			boosting::rc_check(lobby, v3[i].s_id);
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
			update_round(lobby);
		}

		utils::sleep(1000);

		for (size_t i{};i < v3.size();i++) {
			boosting::rc(v3[i].s_id);
		}

		for (size_t i{};i < v3.size();i++) {
			boosting::rc_check(lobby, v3[i].s_id);
		}

	}
}

void boosting::team2_win(c_lobby lobby, bool rounds) {
	utils::clear_every_log(lobby);
	auto v3 = data.get_accs_team(lobby.team_1);
	//auto v3 = data.get_accs_team(lobby.team_2, lobby.team_1); // team 2's players but team 1's leader
	if (rounds) {
		for (size_t i{};i < v3.size();i++) {
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
		}
		update_round(lobby, true);
		utils::sleep(500);

		for (int i{};i < 2;i++) {
			for (size_t i{};i < v3.size();i++) {
				boosting::rc(v3[i].s_id);
				boosting::rc_check(lobby, v3[i].s_id);
				boosting::dc(v3[i].s_id);
				boosting::dc_check(lobby, v3[i].s_id);
				update_round(lobby, true);
			}
		}

		for (int i{};i < 4;i++) {
			boosting::rc(v3[i].s_id);
			boosting::rc_check(lobby, v3[i].s_id);
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
			update_round(lobby, true);
		}

		utils::sleep(1000);

		for (size_t i{};i < v3.size();i++) {
			boosting::rc(v3[i].s_id);
		}

		for (size_t i{};i < v3.size();i++) {
			boosting::rc_check(lobby, v3[i].s_id);
		}



		boosting::long_sleep(lobby, 5000);
		for (size_t i{};i < v3.size();i++) {
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
		}
		update_round(lobby, true); // 16th round

		utils::sleep(1000);

	}
	else {
		for (size_t i{};i < v3.size();i++) {
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
		}
		update_round(lobby, true);
		utils::sleep(500);

		for (int i{};i < 2;i++) {
			for (size_t i{};i < v3.size();i++) {
				boosting::rc(v3[i].s_id);
				boosting::rc_check(lobby, v3[i].s_id);
				boosting::dc(v3[i].s_id);
				boosting::dc_check(lobby, v3[i].s_id);
				update_round(lobby, true);
			}
		}

		for (int i{};i < 4;i++) {
			boosting::rc(v3[i].s_id);
			boosting::rc_check(lobby, v3[i].s_id);
			boosting::dc(v3[i].s_id);
			boosting::dc_check(lobby, v3[i].s_id);
			update_round(lobby, true);
		}

		utils::sleep(1000);
	}
}

void boosting::hit_accept(std::vector<acc_info> accs) {
	for (size_t i{}; i < accs.size(); i++) {
		utils::left_click(data.hwnds[accs[i].s_id], 123, 83);
	}
}

void boosting::launch_accounts(c_lobby lobby) {
	if (lobby.match_mode == 3) {
		boosting::update_manual_state(lobby, manual_lobby_state::launching_accounts);
	}
	else {
		boosting::update_state(lobby, lobby_state::launching_accounts);
	}

	auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);
	for (size_t i{}; i < accs.size(); i++) {
		int pos_x = 0, pos_y = 0;
		if (lobby.game_type == 0) {
			pos_x = 251 * ((i > 4) ? (i - 5) : (i));
			pos_y = 176 * lobby.lobby_idx * 2;
			if (i > 4) pos_y += 176;
		}
		else {
			pos_x = 251 * i;
			pos_y = 176 * lobby.lobby_idx;
		}

		utils::start_cs(pos_x, pos_y, accs[i].user, accs[i].pass, accs[i].s_id);
		utils::sleep(1000);
	}

	if (lobby.match_mode == 3) {
		boosting::update_manual_state(lobby, manual_lobby_state::waiting_for_cs_to_launch);

		wait_for_windows(lobby);
		boosting::update_manual_state(lobby, manual_lobby_state::verifying_handles);

		verify_handles(lobby);
		boosting::update_manual_state(lobby, manual_lobby_state::got_handles);
	}
}

void restart_cs_unique(int x, int y, std::string user, std::string pass, std::string s_id) {
	utils::start_cs(x, y, user, pass, s_id);
}

void boosting::wait_for_windows(c_lobby& lobby) {
	auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);
	boosting::terminate_if_needed(lobby);

	for (size_t i{}; i < accs.size(); i++) {
		auto& d = accs[i];
		boosting::terminate_if_needed(lobby);

		int to_wait = 90;
		int wait_for_account_launch = 0;
		while (!data.hwnds[d.s_id]) {
			boosting::terminate_if_needed(lobby);
			data.hwnds[d.s_id] = FindWindowA(NULL, d.s_id.c_str());

			if (wait_for_account_launch >= to_wait) {
				wait_for_account_launch = 0;

				if (to_wait > 90) {
					utils::webhook("Accounts are failing to launch please come check for any problems.");
				}
				to_wait *= (5 / 3);
				to_wait = std::clamp(to_wait, 0, 150);

				for (size_t k{ 0 }; k < accs.size(); k++) {
					if (data.hwnds[accs[k].s_id]) continue;
					data.hwnds[accs[k].s_id] = FindWindowA(NULL, accs[k].s_id.c_str());
				}

				for (size_t j{ 0 }; j < accs.size(); j++) {
					if (data.hwnds[accs[j].s_id]) continue;

					int pos_x = 251 * ((j > 4) ? (j - 5) : (j));
					int pos_y = 176 * lobby.lobby_idx * 2;
					if (j > 4) pos_y += 176;

					auto str = std::string("steam_").append(accs[i].s_id).append(".exe");
					if (utils::get_pid(str)) {
						utils::kill_process_by_exe(str.c_str());
					}

					utils::sleep(2000);
					utils::cout_lobby(lobby, "Relaunched csgo for %s, ID:%i", accs[j].s_id.c_str(), j);
					utils::start_cs(pos_x, pos_y, accs[j].user, accs[j].pass, accs[j].s_id);
				}
			}

			utils::sleep(2000);
			wait_for_account_launch++;
		}
	}
}

bool check_for_modules(DWORD pid) {
	auto engine = utils::get_module_by_name("engine.dll", pid).modBaseAddr > 0;
	auto client = utils::get_module_by_name("client.dll", pid).modBaseAddr > 0;
	auto sv = utils::get_module_by_name("serverbrowser.dll", pid).modBaseAddr > 0;
	auto panorama = utils::get_module_by_name("panorama.dll", pid).modBaseAddr > 0;
	auto panoramauiclient = utils::get_module_by_name("panoramauiclient.dll", pid).modBaseAddr > 0;
	auto steamclient = utils::get_module_by_name("steamclient.dll", pid).modBaseAddr > 0;

	return engine && client && sv && panorama && panoramauiclient && steamclient;
}

void boosting::verify_handles(c_lobby& lobby) {
	auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);

	for (size_t i{}; i < accs.size(); i++) {
		auto& d = accs[i];
		do {
			boosting::terminate_if_needed(lobby);
			data.hwnds[d.s_id] = FindWindowA(NULL, d.s_id.c_str());
			(GetWindowThreadProcessId)(data.hwnds[accs[i].s_id], &lobby.pid[i]);

			for (size_t times_checked{ 0 }; times_checked < 10; times_checked++) {
				lobby.fully_loaded[i] = check_for_modules(lobby.pid[i]);
			}
			boosting::terminate_if_needed(lobby);
		} while (!lobby.fully_loaded[i]);
		boosting::terminate_if_needed(lobby);
		utils::set_process_priority(lobby.pid[i], BELOW_NORMAL_PRIORITY_CLASS);
	}
}

void boosting::kill_cs_lobby(c_lobby lobby) {
	while (locks["is_killing_cs"]) {
		utils::sleep(500);
	}

	locks["is_killing_cs"] = true;
	auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);
	for (size_t i = 0; i < accs.size(); i++) {
		if (accs[i].s_id == "nil" || accs[i].s_id.empty()) continue;

		utils::kill_pid(lobby.pid[i]);
		auto str = std::string("steam_").append(accs[i].s_id).append(".exe");
		if (!utils::get_pid(str)) continue;
		utils::kill_process_by_exe(str.c_str());
		utils::sleep(500);
	}
	utils::kill_process_by_exe("gameoverlayui.exe");
	locks["is_killing_cs"] = false;
}

void boosting::invite_accounts_team(c_lobby lobby, std::vector<acc_info> accounts) {
	for (size_t i = 1; i < accounts.size(); i++) {
		boosting::terminate_if_needed(lobby);
		if (lobby.match_mode == 0)
			boosting::paste_code(data.hwnds[accounts[((i > 4) ? 5 : 0)].s_id], accounts[i].code, 0); // 0 cuz we wanna paste on to the leader's hwnd
		else
			boosting::paste_code(data.hwnds[accounts[((i > 1) ? 2 : 0)].s_id], accounts[i].code, 0); // 0 cuz we wanna paste on to the leader's hwnd
	}
}

void boosting::join_lobby(HWND hwnd, bool wingman) {
	utils::left_click(hwnd, 145, 103);
	utils::sleep(200);
	SendMessageA(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(243, 89));
	utils::sleep(250);
	SendMessageA(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(243, 89));
	utils::sleep(250);
	if (wingman) {
		utils::left_click(hwnd, 212, 40);
	}
	else {
		utils::left_click(hwnd, 242, 40);
	}
	utils::sleep(200);
	utils::left_click(hwnd, 123, 100);
}

void boosting::invite_accounts_lobby(c_lobby lobby) {
	if (lobby.game_type == 0 && lobby.match_mode == 3) {
		boosting::update_manual_state(lobby, manual_lobby_state::inviting);
	}
	else {
		boosting::update_state(lobby, lobby_state::inviting);
	}
	auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);

	for (size_t i{};i < accs.size();i++) {
		utils::left_click(data.hwnds[accs[i].s_id], 161, 83);
		utils::left_click(data.hwnds[accs[i].s_id], 161, 83);
	}

	std::thread i1(invite_accounts_team, lobby, lobby.game_type == 0 ? data.get_accs_team(lobby.team_1) : data.get_accs_wingman(lobby.team_1));
	std::thread i2(invite_accounts_team, lobby, lobby.game_type == 0 ? data.get_accs_team(lobby.team_2) : data.get_accs_wingman(lobby.team_2));
	i1.join();
	i2.join();

	utils::sleep(2000);

	std::vector<std::thread> join_lobby_threads{};
	join_lobby_threads.clear();

	for (size_t i{ 1 }; i < accs.size(); i++) {
		if (lobby.game_type == 0) {
			if (i == 5) continue;
			join_lobby_threads.push_back(std::thread(join_lobby, data.hwnds[accs[i].s_id], false));
		}
		else {
			if (i == 2) continue;
			join_lobby_threads.push_back(std::thread(join_lobby, data.hwnds[accs[i].s_id], true));
		}
	}

	for (size_t i{}; i < join_lobby_threads.size(); i++) {
		join_lobby_threads[i].join();
	}
	boosting::terminate_if_needed(lobby);

	utils::sleep(3000);

	utils::cout_lobby(lobby, "Making sure every account joins the lobby");

	if (lobby.game_type == 0)
		make_sure_every_acc_joined(lobby);
	else
		make_sure_every_acc_joined_wingman(lobby);

	if (lobby.game_type == 0 && lobby.match_mode == 3) {
		boosting::update_manual_state(lobby, manual_lobby_state::in_lobby);
	}
	else {
		boosting::update_state(lobby, lobby_state::in_lobby);
	}
}

void boosting::start_boosting(c_lobby lobby) {
	threads.push_back(std::thread(worker_thread, lobby));
}

void boosting::update_state(c_lobby& lobby, lobby_state state) {
	lobby.current_state = state;
	lobbies[lobby.lobby_idx].current_state = state;
}

void boosting::update_manual_state(c_lobby& lobby, manual_lobby_state state) {
	lobby.manual_state = state;
	lobbies[lobby.lobby_idx].manual_state = state;
}

void boosting::reset_data(c_lobby& lobby) {
	for (size_t i{}; i < 2; i++) {
		lobbies[lobby.lobby_idx].rounds_won[i] = 0;
	}

	for (size_t i{}; i < 10; i++) {
		lobbies[lobby.lobby_idx].pid[i] = {};
		lobbies[lobby.lobby_idx].fully_loaded[i] = {};
	}

	lobby.times_boosted = 0;
	lobbies[lobby.lobby_idx].times_boosted = 0;
	lobbies[lobby.lobby_idx].boosting_started_epoch = utils::epoch_time();
	boosting::update_state(lobby, lobby_state::finished);
}

void boosting::paste_code(HWND hw, std::string friend_code, int offset) {
	SendMessageA(hw, WM_MOUSEMOVE, 0, MAKELPARAM(243, 78));
	utils::sleep(200);

	utils::left_click(hw, 241, 34 + offset);
	utils::sleep(200);

	utils::left_click(hw, 241, 34 + offset);
	utils::sleep(200);

	utils::left_click(hw, 204, 43 + offset);
	utils::sleep(200);

	utils::left_click(hw, 204, 43 + offset);
	utils::sleep(200);

	utils::left_click(hw, 118, 73);
	utils::sleep(200);

	utils::left_click(hw, 118, 73);
	utils::sleep(200);

	for (size_t i{};i < friend_code.size();i++) {
		if (friend_code[i] == '-') {
			utils::send_char(hw, 45);
		}
		else {
			utils::send_char(hw, VkKeyScan(friend_code[i]));
		}
	}

	utils::send_key(hw, VK_RETURN);
	//utils::left_click(hw, 139, 72);
	utils::sleep(200);

	utils::left_click(hw, 115, 75);
	utils::sleep(200);

	utils::left_click(hw, 194, 60);
	utils::sleep(200);

	utils::left_click(hw, 146, 90);
	utils::sleep(200);
}

std::string boosting::parse_friend_code(HWND window) {
	utils::left_click(window, 125, 40);

	utils::sleep(300);
	utils::left_click(window, 125, 40);

	utils::sleep(300);
	SendMessageA(window, WM_MOUSEMOVE, WM_MOUSEMOVE, MAKELPARAM(242, 80));

	utils::sleep(300);
	SendMessageA(window, WM_MOUSEMOVE, WM_MOUSEMOVE, MAKELPARAM(242, 35));
	utils::left_click(window, 242, 35);

	utils::sleep(300);
	utils::left_click(window, 242, 35);

	utils::sleep(300);
	utils::left_click(window, 205, 44);

	utils::sleep(300);
	utils::left_click(window, 205, 44);

	utils::sleep(300);
	utils::left_click(window, 130, 84);

	utils::sleep(300);
	utils::left_click(window, 130, 84);

	utils::sleep(300);
	utils::left_click(window, 146, 84);

	utils::sleep(300);
	utils::left_click(window, 146, 84);

	utils::sleep(500);

	std::string k;
	clip::get_text(k);

	return k;
}

void boosting::get_friend_codes(c_lobby lobby) {
	for (int make_sure{}; make_sure < 5; make_sure++) {
		auto lobby_accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);

		for (size_t i{}; i < lobby_accs.size(); i++) {
			boosting::terminate_if_needed(lobby);
			if (!(lobby_accs[i].code == "nil" || lobby_accs[i].code.length() != 10 || lobby_accs[i].code[5] != '-')) continue;

			for (size_t j{}; j < data.accs.size(); j++) {
				if (lobby_accs[i].user != data.accs[j].user) continue;

				std::string code = "";
				bool is_valid = true;
				do {
					is_valid = true;
					code = parse_friend_code(data.hwnds[lobby_accs[i].s_id]);
					utils::cout("got friend code %s for %s.\n", code.c_str(), lobby_accs[i].user.c_str());

					utils::sleep(500);
					if (code.length() != 10 || code[5] != '-') {
						is_valid = false;
					}
					else {
						if (i == 0) continue;
						for (size_t k{}; k < data.accs.size(); k++) {
							if (lobby_accs[i - 1].user != data.accs[k].user) continue;
							if (code != data.accs[k].code) continue;
							is_valid = false;
							break;
						}
					}
					if (!is_valid) {
						utils::sleep(1000);
					}
				} while (!is_valid);

				data.set_code(j, code);
				data.exp();
				utils::sleep(100);
			}
		}
	}
}

void boosting::full_reset_data(c_lobby& lobby) {
	reset_data(lobby);
	boosting::update_state(lobby, lobby_state::fully_finished);
}

void boosting::update_completion(c_lobby& lobby) {
	lobby.times_boosted++;
	lobbies[lobby.lobby_idx].times_boosted++;
	reset_rounds(lobby);
}

void boosting::reset_termination(c_lobby& lobby) {
	lobby.termination_request = 0;
	lobbies[lobby.lobby_idx].termination_request = 0;
	reset_rounds(lobby);
}

void boosting::get_teams(c_lobby& lobby) {
	int team_1 = 0;
	for (int i{}; i < 500; i++) {
		if (lobby.has_team_been_boosted[i]) continue;
		if (team_1 == 0) {
			if (lobby.team_to_boost[i]) {
				team_1 = i;
			}
		}
		else {
			if (lobby.team_to_boost[i]) {
				lobby.team_1 = team_1;
				lobby.team_2 = i;

				lobby.has_team_been_boosted[lobby.team_1] = true;
				lobby.has_team_been_boosted[lobby.team_2] = true;

				lobbies[lobby.lobby_idx].team_1 = lobby.team_1;
				lobbies[lobby.lobby_idx].team_2 = lobby.team_2;

				lobbies[lobby.lobby_idx].has_team_been_boosted[lobby.team_1] = lobby.has_team_been_boosted[lobby.team_1];
				lobbies[lobby.lobby_idx].has_team_been_boosted[lobby.team_2] = lobby.has_team_been_boosted[lobby.team_2];
				team_1 = 0;
				break;
			}
		}
	}
	utils::cout_lobby(lobby, "Team ready");
}

bool boosting::did_acc_join_lobby(std::string steam_id) {
	return utils::does_acc_have_string(steam_id, "CS_App_Lifetime_Gamestats: Recording PartyBrowserJoinInvited");
}

bool boosting::check_if_connected(std::string steam_id) {
	return utils::does_acc_have_string(steam_id, "ChangeGameUIState: CSGO_GAME_UI_STATE_INGAME -> CSGO_GAME_UI_STATE_INGAME");
}

bool boosting::check_if_disconnected(std::string steam_id) {
	return utils::does_acc_have_string(steam_id, "ChangeGameUIState: CSGO_GAME_UI_STATE_INGAME -> CSGO_GAME_UI_STATE_MAINMENU");
}

bool boosting::check_if_loading(std::string steam_id) {
	if (utils::does_acc_have_string(steam_id, "ChangeGameUIState: CSGO_GAME_UI_STATE_MAINMENU -> CSGO_GAME_UI_STATE_LOADINGSCREEN")) {
		return true;
	}
	return false;
}

bool boosting::did_accept_button_appear(std::string steam_id) {
	return utils::does_acc_have_string(steam_id, "lock mmqueue");
}

bool boosting::did_accept_button_disappear(std::string steam_id) {
	bool x = utils::does_acc_have_string(steam_id, "Matchmaking update: 3");
	bool y = utils::does_acc_have_string(steam_id, "Matchmaking message: Failed to ready up");

	if (x || y) {
		return true;
	}
	return false;
}

bool boosting::did_enemies_accept(std::string steam_id) {
	return utils::does_acc_have_string(steam_id, "Matchmaking message: Failed to ready up");
}

bool boosting::did_lobby_start_searching(std::string steam_id) {
	return utils::does_acc_have_string(steam_id, "Matchmaking update: 1");
}

bool boosting::did_lobby_stop_searching(std::string steam_id) {
	return utils::does_acc_have_string(steam_id, "Matchmaking update: 0");
}

bool boosting::check_session_error(std::string steam_id) {
	return utils::does_acc_have_string(steam_id, "CSysSessionClient: Unable to get session information from host");
}

int boosting::get_number_of_members_in_lobby(std::string steam_id) {
	int return_val{ 0 };

	std::ifstream file(g_cfg.csgo_directory + "\\csgo\\log\\" + steam_id + ".log");
	if (!file.is_open()) return return_val;

	size_t offset{ 0 };
	std::string line{ "" }, best_line{ "" };

	while (std::getline(file, line)) {
		const size_t found = line.find("LobbySetData: 'members:numMachines' = '");
		if (found == std::string::npos) continue;

		best_line = line;
		offset = found + 39; // 9 is for match_id:
	}

	std::string ret{};
	if (best_line != "")
	{
		for (int i = 0; i < 1; i++) {
			ret += best_line[i + offset];
		}

		try {
			return_val = std::stoi(ret);
		}
		catch (int e) {
			utils::cout("error on check lobby %i\n", e);
		}
	}

	return return_val;
}

int64_t boosting::find_match_id(std::string steam_id) {
	int64_t return_val{ 0 };

	std::ifstream file(g_cfg.csgo_directory + "\\csgo\\log\\" + steam_id + ".log");
	if (!file.is_open()) return return_val;

	size_t offset{ 0 };
	std::string line{ "" }, best_line{ "" };

	while (std::getline(file, line)) {
		const size_t found = line.find("match_id");
		if (found == std::string::npos) continue;

		best_line = line;
		offset = found + 9; // 9 is for match_id:
	}

	std::string ret{};
	if (best_line != "")
	{
		for (int i = 0; i < 19; i++) {
			ret += best_line[i + offset];
		}

		try {
			return_val = std::stoll(ret);
		}
		catch (int e) {
			utils::cout("error on check lobby %i\n", e);
		}
	}

	return return_val;
}

void boosting::re_invite_account(HWND leader_hwnd, std::string friend_code, std::string steam_id, int offset) {
	boosting::paste_code(leader_hwnd, friend_code, offset);

	utils::sleep(2000);

	join_lobby(data.hwnds[steam_id], false);
}

void boosting::re_invite_account_wingman(HWND leader_hwnd, std::string friend_code, std::string steam_id) {
	boosting::paste_code(leader_hwnd, friend_code, 0);

	utils::sleep(2000);

	join_lobby(data.hwnds[steam_id], true);
}

void boosting::make_sure_every_acc_joined(c_lobby lobby) {
	while (boosting::get_number_of_members_in_lobby(data.get_accs_team(lobby.team_1)[0].s_id) != 5) {
		boosting::terminate_if_needed(lobby);
		for (int i{}; i < 2; i++) {
		check_again1:
			auto lobby_1_numbers = boosting::get_number_of_members_in_lobby(data.get_accs_team(lobby.team_1)[0].s_id);
			switch (lobby_1_numbers) {
			case 1: {
				for (int i = 1; i < 5; i++) {
					if (!boosting::did_acc_join_lobby(data.get_accs_team(lobby.team_1)[i].s_id)) {
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], data.get_accs_team(lobby.team_1)[i].code, data.get_accs_team(lobby.team_1)[i].s_id, 0);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again1;
					}
					else if (boosting::check_session_error(data.get_accs_team(lobby.team_1)[i].s_id)) {
						utils::sleep(1000);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], 148, 82);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], 148, 82);
						utils::sleep(200);
						utils::clear_logfile(data.get_accs_team(lobby.team_1)[i].s_id);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], data.get_accs_team(lobby.team_1)[i].code, data.get_accs_team(lobby.team_1)[i].s_id, 0);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again1;
					}
				}
				break;
			}
			case 2: {
				for (int i = 1; i < 5; i++) {
					if (!boosting::did_acc_join_lobby(data.get_accs_team(lobby.team_1)[i].s_id)) {
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], data.get_accs_team(lobby.team_1)[i].code, data.get_accs_team(lobby.team_1)[i].s_id, 3);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again1;
					}
					else if (boosting::check_session_error(data.get_accs_team(lobby.team_1)[i].s_id)) {
						utils::sleep(1000);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], 148, 82);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], 148, 82);
						utils::sleep(200);
						utils::clear_logfile(data.get_accs_team(lobby.team_1)[i].s_id);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], data.get_accs_team(lobby.team_1)[i].code, data.get_accs_team(lobby.team_1)[i].s_id, 3);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again1;
					}
				}
				break;
			}
			case 3: {
				for (int i = 1; i < 5; i++) {
					if (!boosting::did_acc_join_lobby(data.get_accs_team(lobby.team_1)[i].s_id)) {

						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], data.get_accs_team(lobby.team_1)[i].code, data.get_accs_team(lobby.team_1)[i].s_id, 12);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again1;
					}
					else if (boosting::check_session_error(data.get_accs_team(lobby.team_1)[i].s_id)) {
						utils::sleep(1000);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], 148, 82);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], 148, 82);
						utils::sleep(200);
						utils::clear_logfile(data.get_accs_team(lobby.team_1)[i].s_id);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], data.get_accs_team(lobby.team_1)[i].code, data.get_accs_team(lobby.team_1)[i].s_id, 12);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again1;
					}
				}
				break;
			}
			case 4: {
				for (int i = 1; i < 5; i++) {
					if (!boosting::did_acc_join_lobby(data.get_accs_team(lobby.team_1)[i].s_id)) {
						boosting::terminate_if_needed(lobby);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], data.get_accs_team(lobby.team_1)[i].code, data.get_accs_team(lobby.team_1)[i].s_id, 22);
						utils::sleep(2000);
						goto check_again1;
					}
					else if (boosting::check_session_error(data.get_accs_team(lobby.team_1)[i].s_id)) {
						utils::sleep(1000);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], 148, 82);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], 148, 82);
						utils::sleep(200);
						utils::clear_logfile(data.get_accs_team(lobby.team_1)[i].s_id);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_1)[0].s_id], data.get_accs_team(lobby.team_1)[i].code, data.get_accs_team(lobby.team_1)[i].s_id, 22);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again1;
					}
				}
				break;
			}
			}
		}
	}
	while (boosting::get_number_of_members_in_lobby(data.get_accs_team(lobby.team_2)[0].s_id) != 5) {
		boosting::terminate_if_needed(lobby);
		for (int i{}; i < 2; i++) {
		check_again2:
			auto lobby_2_numbers = boosting::get_number_of_members_in_lobby(data.get_accs_team(lobby.team_2)[0].s_id);
			switch (lobby_2_numbers) {
			case 1: {
				for (int i = 1; i < 5; i++) {
					if (!boosting::did_acc_join_lobby(data.get_accs_team(lobby.team_2)[i].s_id)) {
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], data.get_accs_team(lobby.team_2)[i].code, data.get_accs_team(lobby.team_2)[i].s_id, 0);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again2;
					}
					else if (boosting::check_session_error(data.get_accs_team(lobby.team_2)[i].s_id)) {
						utils::sleep(1000);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], 148, 82);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], 148, 82);
						utils::sleep(200);
						utils::clear_logfile(data.get_accs_team(lobby.team_2)[i].s_id);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], data.get_accs_team(lobby.team_2)[i].code, data.get_accs_team(lobby.team_2)[i].s_id, 0);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again2;
					}
				}
				break;
			}
			case 2: {
				for (int i = 1; i < 5; i++) {
					if (!boosting::did_acc_join_lobby(data.get_accs_team(lobby.team_2)[i].s_id)) {
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], data.get_accs_team(lobby.team_2)[i].code, data.get_accs_team(lobby.team_2)[i].s_id, 3);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again2;
					}
					else if (boosting::check_session_error(data.get_accs_team(lobby.team_2)[i].s_id)) {
						utils::sleep(1000);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], 148, 82);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], 148, 82);
						utils::sleep(200);
						utils::clear_logfile(data.get_accs_team(lobby.team_2)[i].s_id);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], data.get_accs_team(lobby.team_2)[i].code, data.get_accs_team(lobby.team_2)[i].s_id, 3);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again2;
					}
				}
				break;
			}
			case 3: {
				for (int i = 1; i < 5; i++) {
					if (!boosting::did_acc_join_lobby(data.get_accs_team(lobby.team_2)[i].s_id)) {

						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], data.get_accs_team(lobby.team_2)[i].code, data.get_accs_team(lobby.team_2)[i].s_id, 12);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again2;
					}
					else if (boosting::check_session_error(data.get_accs_team(lobby.team_2)[i].s_id)) {
						utils::sleep(1000);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], 148, 82);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], 148, 82);
						utils::sleep(200);
						utils::clear_logfile(data.get_accs_team(lobby.team_2)[i].s_id);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], data.get_accs_team(lobby.team_2)[i].code, data.get_accs_team(lobby.team_2)[i].s_id, 12);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again2;
					}
				}
				break;
			}
			case 4: {
				for (int i = 1; i < 5; i++) {
					if (!boosting::did_acc_join_lobby(data.get_accs_team(lobby.team_2)[i].s_id)) {
						boosting::terminate_if_needed(lobby);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], data.get_accs_team(lobby.team_2)[i].code, data.get_accs_team(lobby.team_2)[i].s_id, 22);
						utils::sleep(2000);
						goto check_again2;
					}
					else if (boosting::check_session_error(data.get_accs_team(lobby.team_2)[i].s_id)) {
						utils::sleep(1000);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], 148, 82);
						utils::left_click(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], 148, 82);
						utils::sleep(200);
						utils::clear_logfile(data.get_accs_team(lobby.team_2)[i].s_id);
						re_invite_account(data.hwnds[data.get_accs_team(lobby.team_2)[0].s_id], data.get_accs_team(lobby.team_2)[i].code, data.get_accs_team(lobby.team_2)[i].s_id, 22);
						boosting::terminate_if_needed(lobby);
						utils::sleep(2000);
						goto check_again2;
					}
				}
				break;
			}
			}
		}
	}
	utils::clear_every_log(lobby);
}

void boosting::make_sure_every_acc_joined_wingman(c_lobby lobby) {
	auto team1 = data.get_accs_wingman(lobby.team_1);
	auto team2 = data.get_accs_wingman(lobby.team_2);

	while (boosting::get_number_of_members_in_lobby(team1[0].s_id) != 2) {
		boosting::terminate_if_needed(lobby);
		for (int i{}; i < 2; i++) {
		check_again1:
			auto lobby_1_numbers = boosting::get_number_of_members_in_lobby(team1[0].s_id);
			for (int i = 1; i < 2; i++) {
				if (!boosting::did_acc_join_lobby(team1[i].s_id)) {
					re_invite_account_wingman(data.hwnds[team1[0].s_id], team1[i].code, team1[i].s_id);
					boosting::terminate_if_needed(lobby);
					utils::sleep(2000);
					goto check_again1;
				}
				else if (boosting::check_session_error(team1[i].s_id)) {
					utils::sleep(1000);
					utils::left_click(data.hwnds[team1[0].s_id], 148, 82);
					utils::left_click(data.hwnds[team1[0].s_id], 148, 82);
					utils::sleep(200);
					utils::clear_logfile(team1[i].s_id);
					re_invite_account_wingman(data.hwnds[team1[0].s_id], team1[i].code, team1[i].s_id);
					boosting::terminate_if_needed(lobby);
					utils::sleep(2000);
					goto check_again1;
				}
			}
		}
	}

	while (boosting::get_number_of_members_in_lobby(team2[0].s_id) != 2) {
		boosting::terminate_if_needed(lobby);
		for (int i{}; i < 2; i++) {
		check_again2:
			auto lobby_2_numbers = boosting::get_number_of_members_in_lobby(team2[0].s_id);
			for (int i = 1; i < 2; i++) {
				if (!boosting::did_acc_join_lobby(team2[i].s_id)) {
					re_invite_account_wingman(data.hwnds[team2[0].s_id], team2[i].code, team2[i].s_id);
					boosting::terminate_if_needed(lobby);
					utils::sleep(2000);
					goto check_again2;
				}
				else if (boosting::check_session_error(team2[i].s_id)) {
					utils::sleep(1000);
					utils::left_click(data.hwnds[team2[0].s_id], 148, 82);
					utils::left_click(data.hwnds[team2[0].s_id], 148, 82);
					utils::sleep(200);
					utils::clear_logfile(team2[i].s_id);
					re_invite_account_wingman(data.hwnds[team2[0].s_id], team2[i].code, team2[i].s_id);
					boosting::terminate_if_needed(lobby);
					utils::sleep(2000);
					goto check_again2;
				}
			}
		}
	}
	utils::clear_every_log(lobby);
}

void boosting::toggle_windows(c_lobby lobby) {
	std::vector<HWND> hwnds{};
	hwnds.clear();

	auto accs = lobby.game_type == 0 ? utils::get_all_accs(lobby) : utils::get_all_accs_wingman(lobby);

	for (size_t i{}; i < accs.size(); i++)
		hwnds.push_back(data.hwnds[accs[i].s_id]);

	for (size_t i{}; i < hwnds.size(); i++) {
		if (lobby.visible) {
			ShowWindow(hwnds[i], SW_HIDE);
		}
		else {
			ShowWindow(hwnds[i], SW_SHOW);
		}

		lobby.visible = !lobby.visible;
		lobbies[lobby.lobby_idx].visible = lobby.visible;
	}
}

void boosting::initialize_search(HWND hwnd, bool wingman) {
	utils::left_click(hwnd, 4, 18);
	utils::sleep(1000);
	if (wingman) {
		utils::left_click(hwnd, 38, 25);
		utils::sleep(350);
		utils::left_click(hwnd, 41, 25);
		utils::sleep(250);
		utils::left_click(hwnd, 45, 25);
	}
	utils::left_click(hwnd, 192, 144);
}