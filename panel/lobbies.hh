#pragma once
#include <array>
#include <cstdint>
#include <Windows.h>

enum class lobby_state {
	waiting_for_network_reconnection = -3,
	hard_terminated = -2,
	terminated = -1,
	zero,
	setting_up,
	launching_accounts,
	waiting_for_cs_to_launch,
	verifying_handles,
	got_handles,
	getting_friend_codes,
	making_lobby,
	inviting,
	in_lobby,
	searching_for_game,
	mismatch,
	lobby_search_on_cooldown,
	match_found,
	joining_game,
	in_warm_up,
	in_game,
	finished,
	fully_finished
};

enum class manual_lobby_state {
	zero,
	launching_accounts,
	waiting_for_cs_to_launch,
	verifying_handles,
	got_handles,
	making_lobby,
	inviting,
	in_lobby,
	searching_for_game,
	mismatch,
	lobby_search_on_cooldown,
	match_found,
	joining_game,
	in_warm_up,
	in_game,
	finished
};

enum class which_time {
	time_since_started_searching
};

class c_lobby {
public:
	bool visible{ true };
	int lobby_idx{ 0 };
	int termination_request{ 0 };
public:
	int map{ 10 };
	int map_wingman{ 5 };
	int match_mode{ 0 };
	int repetition{ 2 };
	bool start_with_team2{ false };
public:
	int team_1{ 0 };
	int team_2{ 0 };
	int team_selected_for_derank{ 0 };
	int valid_lobbies{ 0 };
	int times_boosted{ 0 };
	int rounds_won[2]{ 0 };
	int wait_for_account_launch;
public:
	bool team_to_boost[500]{ false };
	bool didnt_find_a_match[500]{ false };
	bool has_team_been_boosted[500]{ false };
public:
	DWORD pid[10];
	bool fully_loaded[10]{ false };
	lobby_state current_state{ lobby_state::zero };
	manual_lobby_state manual_state{ manual_lobby_state::zero };
public:
	bool found_a_match{ false };
	int64_t boosting_started_epoch{ 0 };
	int64_t searching_started_epoch{ 0 };
public:
	// stuff for manual boost
	bool auto_accept{ true };
	int game_type{ 1 }; // 0 = comp, 1= wingman
};

inline constexpr int MAX_LOBBIES = 5;
inline std::array<c_lobby, MAX_LOBBIES> lobbies;