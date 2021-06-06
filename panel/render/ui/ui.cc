#include "ui.hh"
#include <fstream>
#include <config.hh>
#include <includes.hh>
#include <data/data.hh>
#include <utils/utils.hh>
#include <server/server.hh>
#include <linguistics/linguistics.hh>
#include <render/imgui/imgui_stdlib.h>
#include <anti debug/anti dbg main.hh>

#include "arrays.hh"
#include <lobbies.hh>
#include <boosting/boosting.hh>
#include <anti debug/xorstr.hpp>

void get_config();
void update_config();

#include <cmath>
#include <deque>

float dot_color[4]{ 1.f, 1.f, 1.f, 1.f };
float connect_color[4]{ 1.f, 1.f, 1.f, 1.f };
size_t dots_amount = 80;
int dots_connect_distance = 180;
float dot_radius = 3;
float alpha = 0.f;
int seed_count = 0;

float distance(float x0, float y0, float x1, float y1)
{
	return std::sqrtf(std::powf((x1 - x0), 2) + std::powf((y1 - y0), 2));
}

int random_int(int min, int max)
{
	auto seed = rand() % (max - min + 1) + min;
	return rand() % (max - min + 1) + min;
}

float random_float(float min, float max)
{
	int mi = int(min * 100);
	int ma = int(max * 100);

	auto seed = rand() % (ma - mi + 1) + mi;
	return float(rand() % (ma - mi + 1) + mi) / 100;
}

struct dot {
	float x, y, velocity_x, velocity_y;
};
std::deque< dot > dots;

namespace ImGui {
	void partices()
	{
		while (dots.size() > dots_amount) {
			dots.pop_back();
		}
		while (dots.size() < dots_amount) {
			dot temp;
			{
				temp.x = float(random_int(0, 1920));
				temp.y = float(random_int(0, 1080));
				constexpr float max = 12, min = 4;

				if (random_int(0, 1)) temp.velocity_x = random_float(-max, -min);
				else temp.velocity_x = random_float(min, max);

				if (random_int(0, 1)) temp.velocity_y = random_float(-max, -min);
				else temp.velocity_y = random_float(min, max);
			}
			dots.emplace_front(temp);
		}

		for (auto& cdot : dots) {
			for (auto& cdot2 : dots) {
				auto dist = distance(cdot.x, cdot.y, cdot2.x, cdot2.y);

				if (dist <= dots_connect_distance) {
					float a_connect_multiplier = 1.f;
					if (dist > dots_connect_distance * 0.7f) {
						a_connect_multiplier = (dots_connect_distance - dist) / (dots_connect_distance * 0.1f);
					}
					ImGui::GetWindowDrawList()->AddLine
					(
						ImVec2(cdot.x, cdot.y),
						ImVec2(cdot2.x, cdot2.y),
						ImColor(connect_color[0], connect_color[1], connect_color[2], connect_color[3] * a_connect_multiplier * alpha)
					);
				}
			}
		}

		for (size_t i = 0; i < dots.size(); i++) {
			auto& cdot = dots[i];
			ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cdot.x, cdot.y), dot_radius, ImColor(dot_color[0], dot_color[1], dot_color[2], dot_color[3] * alpha));

			cdot.x += cdot.velocity_x * float(2 * ImGui::GetIO().DeltaTime);
			cdot.y += cdot.velocity_y * float(2 * ImGui::GetIO().DeltaTime);
			if (cdot.x < -dots_connect_distance || cdot.y < -dots_connect_distance || cdot.x > 1920 + dots_connect_distance || cdot.y > 1080 + dots_connect_distance)
			{
				dots.erase(dots.cbegin() + i);
			}
		}

		alpha += ImGui::GetIO().DeltaTime / 10.f;
		alpha = std::clamp(alpha, 0.f, 0.5f);
	}

	bool Spinner(ImVec2 pos, float radius, float thickness, const ImU32& color) {
		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		ImGuiWindow* window = GetCurrentWindow();

		constexpr float num_segments = 30;
		float start = fabsf(sinf(float(g.Time) * 1.8f) * (num_segments - 5));

		const float a_min = IM_PI * 2.0f * (start) / num_segments;
		const float a_max = IM_PI * 2.0f * (num_segments - 3) / num_segments;

		const ImVec2 centre = ImVec2(pos.x, pos.y + style.FramePadding.y);

		window->DrawList->PathClear();
		for (int i = 0; i < num_segments; i++) {
			const float a = a_min + (i / num_segments) * (a_max - a_min);
			window->DrawList->PathLineTo(ImVec2(centre.x + cosf(a + float(g.Time) * 8) * radius, centre.y + sinf(a + float(g.Time) * 8) * radius));
		}
		window->DrawList->PathStroke(color, false, thickness);

		return true;
	}
}

auto get_search_name = [&](int team_id)
{
	return team_id == 0 ? std::string(strings("None")) : team_id == 1 ? std::string(strings("Unassigned")) : std::string(strings("Team")).append(" ").append(std::to_string(team_id - 1));
};

void accounts_page() {
	static int team_filter = 0;
	static char user_filter[255];
	ImGui::InputText(strings("User Filter"), user_filter, IM_ARRAYSIZE(user_filter));

	if (ImGui::BeginCombo(strings("Team Filter"), get_search_name(team_filter).c_str())) {
		if (ImGui::Selectable(strings("None"), team_filter == 0)) {
			team_filter = 0;
		}
		if (ImGui::Selectable(strings("Unassigned"), team_filter == 1)) {
			team_filter = 1;
		}
		for (size_t j = 2; j < 502; j++) {
			if (ImGui::Selectable(std::string(strings("Team")).append(" ").append(std::to_string(j - 1)).c_str(), team_filter == j)) {
				team_filter = j;
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginChild(do_xor("accs"), { 0, -30 })) {
		ImGui::SetWindowFontScale(0.85f);
		if (ImGui::BeginTable(do_xor("table1"), 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
		{
			ImGui::TableSetupColumn(strings("ID"), ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(strings("Username"), ImGuiTableColumnFlags_WidthFixed);
			//ImGui::TableSetupColumn(strings("Friend Code"), ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(strings("Team ID"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(strings("Wingman Team ID"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(strings("Steam ID"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(strings("Ban"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			for (size_t i = 0; i < data.accs.size(); i++) {
				auto& d = data.accs[i];

				if (d.user.find(user_filter) == std::string::npos) continue;
				if (team_filter > 0 && d.t_id != team_filter - 1) continue;
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				//if (ImGui::Button("Login"))
				//{
				//	//utils::start_cs(0, 0, d.user, d.pass, d.s_id);
				//	auto v2 = data.get_accs_team(1, 2); // team 1's players but team 2's leader
				//	for (size_t aaa{}; aaa < v2.size(); aaa++) {
				//		utils::cout_lobby(lobbies[0], "%s\n", v2[aaa].user.c_str());
				//	}
				//}
				//if (ImGui::Button("Login2"))
				//{
				//	auto hwnd = FindWindow(NULL, d.s_id.c_str());
				//	utils::send_key(hwnd, 45);//prolly
				//}
				ImGui::Text(std::to_string(d.idx + 1).c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", d.user.c_str());
				ImGui::TableSetColumnIndex(2);
				{
					ImGui::PushItemWidth(-1);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 4, 4 });
					if (ImGui::BeginCombo(std::string(do_xor("##team_id")).append(std::to_string(i)).c_str(), d.t_id == 0 ? strings("Unassigned") : std::string(strings("Team")).append(" ").append(std::to_string(d.t_id)).c_str())) {
						if (ImGui::Selectable(strings("Unassigned"), d.t_id == 0)) {
							d.t_id = 0;
						}
						for (size_t j = 1; j < 500; j++) {
							if (ImGui::Selectable(std::string(strings("Team")).append(" ").append(std::to_string(j)).c_str(), d.t_id == j)) {
								d.t_id = j;
							}
						}
						ImGui::EndCombo();
					}
					ImGui::PopStyleVar();
					ImGui::PopItemWidth();
				}
				ImGui::TableSetColumnIndex(3);
				{
					ImGui::PushItemWidth(-1);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 4, 4 });
					if (ImGui::BeginCombo(std::string(do_xor("##wingman_team_id")).append(std::to_string(i)).c_str(), d.w_id == 0 ? strings("Unassigned") : std::string(strings("Team")).append(" ").append(std::to_string(d.w_id)).c_str())) {
						if (ImGui::Selectable(strings("Unassigned"), d.w_id == 0)) {
							d.w_id = 0;
						}
						for (size_t j = 1; j < 500; j++) {
							if (ImGui::Selectable(std::string(strings("Team")).append(" ").append(std::to_string(j)).c_str(), d.w_id == j)) {
								d.w_id = j;
							}
						}
						ImGui::EndCombo();
					}
					ImGui::PopStyleVar();
					ImGui::PopItemWidth();
				}
				ImGui::TableSetColumnIndex(4);
				ImGui::Text("%s", d.s_id.c_str());
				ImGui::TableSetColumnIndex(5);
				if (d.is_banned) {
					ImGui::Text("y");
				}
				else {
					ImGui::Text("n");
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
		ImGui::SetWindowFontScale(1.f);
	}
	if (ImGui::Button(strings("Import"))) {
		data.imp_from_txt();
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(do_xor("assets\\accounts.txt"));
	}
	ImGui::SameLine();
	if (ImGui::Button(strings("Auto assign teams"))) {
		int team_to_assign = 1;
		for (size_t i{}; i < data.accs.size(); i++) {
			if (data.accs[i].t_id != 0) continue;

			while (data.get_accs_team(team_to_assign).size() >= 5) {
				team_to_assign++;
			}

			data.accs[i].t_id = team_to_assign;
		}
		team_to_assign = 1;
		for (size_t i{}; i < data.accs.size(); i++) {
			if (data.accs[i].w_id != 0) continue;

			while (data.get_accs_wingman(team_to_assign).size() >= 2) {
				team_to_assign++;
			}

			data.accs[i].w_id = team_to_assign;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(strings("Get SteamIDs"))) {
		//g_cfg.last_steam_id_parse = utils::epoch_time();
		LI_FN(CreateThread).cached()(0, 0, (LPTHREAD_START_ROUTINE)boosting::get_all_steam_ids, NULL, NULL, NULL);
		update_config();
	}
	if (g_cfg.last_steam_id_parse != 0 && ImGui::IsItemHovered()) {
		if (g_cfg.is_parsing_on_cooldown) {
			ImGui::SetTooltip(do_xor("Steam id parsing on cooldown\nEnds after %u seconds"), 30 - (utils::epoch_time() - g_cfg.last_steam_id_parse));
		}
		else {
			ImGui::SetTooltip(do_xor("You last parsed steam ids %u seconds ago"), utils::epoch_time() - g_cfg.last_steam_id_parse);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(strings("Save"))) {
		data.exp();
		utils::messagebox(do_xor("Saved the database"), do_xor("Information"));
	}
	static int temp{ 0 };
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::InputInt("##Team to pwn", &temp);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(do_xor("Team to delete"));
	}
	ImGui::SameLine();
	if (ImGui::Button(strings("Delete"))) {
		data.delete_accs_from_db(temp);
	}
}

auto all_accs_have_steam_id = [&](int team_id, bool wingman = false)
{
	bool ret = true;
	auto accs = wingman ? data.get_accs_wingman(team_id) : data.get_accs_team(team_id);
	for (size_t i{}; i < accs.size(); i++) {
		if (accs[i].s_id.length() != 10) {
			ret = false;
			break;
		}
	}
	return ret;
};

bool is_selected_by_other_lobby (int team_id)
{
	bool ret = false;
	for (short i{}; i < MAX_LOBBIES; i++) {
		if (lobbies[i].team_1 == team_id || lobbies[i].team_2 == team_id) {
			ret = true;
		}
	}
	return ret;
};

bool is_selected_by_other_lobby(int team_id, int local_lobby)
{
	bool ret = false;
	for (short i{}; i < MAX_LOBBIES; i++) {
		if (i == local_lobby) continue;
		for (size_t j{}; j < 500; j++) {
			if (lobbies[i].team_to_boost[team_id]) {
				ret = true;
				break;
			}
		}
	}
	return ret;
};

auto get_team_name = [&](int team_id)
{
	return team_id == 0 ? std::string(strings("Select")) : std::string(strings("Team")).append(" ").append(std::to_string(team_id));
};

std::string seconds_to_dhms(int sec) {
	int days = int(sec * 1.15741E-5);
	int hours = int(sec * 0.000277778) % 24;
	int minutes = int(sec * 0.0166666667) % 60;
	int seconds = int(sec % 60);

	if (days) {
		return std::to_string(days).append(do_xor(" day(s), ")).append(std::to_string(hours)).append(do_xor(" hour(s), ")).append(std::to_string(minutes)).append(do_xor("minute(s), ")).append(std::to_string(seconds)).append(do_xor(" second(s)"));
	}
	else if (hours) {
		return std::to_string(hours).append(do_xor(" hour(s), ")).append(std::to_string(minutes)).append((do_xor("minute(s), "))).append(std::to_string(seconds)).append(do_xor(" second(s)"));
	}
	else if (minutes) {
		return std::to_string(minutes).append((do_xor("minute(s), "))).append(std::to_string(seconds)).append(do_xor(" second(s)"));
	}
	return std::to_string(seconds % 60).append(do_xor(" second(s)"));
}

void manual_boost_builder(c_lobby& lobby) {
	lobby.valid_lobbies = 1;
	if (lobby.manual_state == manual_lobby_state::zero) {
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		ImGui::Combo(do_xor("##Match mode"), &lobby.match_mode, strings("Draw\0Win Lose\0Win win lose lose\0Manual\0Team1 win\0Team2 win\0Rank unlocker\0"));

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		if (ImGui::BeginCombo(strings("Team 1"), get_team_name(lobby.team_1).c_str()))
		{
			for (int i = 1; i < 500; i++) {
				bool should_disable = false;
				if (data.get_accs_team(i).size() == 0) continue;
				if (data.get_accs_team(i).size() < 5 || data.get_accs_team(i).size() > 5) should_disable = true;
				if (!all_accs_have_steam_id(i)) should_disable = true;
				if (is_selected_by_other_lobby(i)) should_disable = true;
				if (ImGui::Selectable(get_team_name(i).c_str(), lobby.team_1 == i, should_disable ? ImGuiSelectableFlags_Disabled : 0)) {
					lobby.team_1 = i;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		if (ImGui::BeginCombo(strings("Team 2"), get_team_name(lobby.team_2).c_str()))
		{
			for (int i = 1; i < 500; i++) {
				bool should_disable = false;
				if (data.get_accs_team(i).size() == 0) continue;
				if (data.get_accs_team(i).size() < 5 || data.get_accs_team(i).size() > 5) should_disable = true;
				if (!all_accs_have_steam_id(i)) should_disable = true;
				if (is_selected_by_other_lobby(i)) should_disable = true;
				if (ImGui::Selectable(get_team_name(i).c_str(), lobby.team_2 == i, should_disable ? ImGuiSelectableFlags_Disabled : 0)) {
					lobby.team_2 = i;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		ImGui::Checkbox(strings("Auto accept"), &lobby.auto_accept);

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		if (ImGui::Button(strings("Launch accounts"), ImVec2(ImGui::GetWindowWidth() * 0.649f, 40)))
		{
			if (lobby.team_1 && lobby.team_2)
			{
				lobby.boosting_started_epoch = utils::epoch_time();
				utils::create_logs_folder();
				boosting::threads.push_back(std::thread(boosting::launch_accounts, lobby));
			}
		}
	}
	else if (lobby.manual_state == manual_lobby_state::launching_accounts) {
		ImGui::Text(do_xor("Launching accounts"));
	}
	else if (lobby.manual_state == manual_lobby_state::waiting_for_cs_to_launch) {
		ImGui::Text(do_xor("Waiting for CS:GO to launch"));
	}
	else if (lobby.manual_state == manual_lobby_state::verifying_handles) {
		ImGui::Text(do_xor("Verifying handles"));
	}
	else if (lobby.manual_state >= manual_lobby_state::got_handles && lobby.manual_state < manual_lobby_state::finished) {

		if (lobby.manual_state == manual_lobby_state::got_handles) {
			if (ImGui::Button(strings("Invite accounts"), ImVec2(ImGui::GetWindowWidth() * 0.649f, 40))) {
				boosting::threads.push_back(std::thread(boosting::invite_accounts_lobby, lobby));
			}
		}
	}
	ImGui::Combo(do_xor("##Match mode"), &lobby.match_mode, strings("Draw\0Win Lose\0Win win lose lose\0Manual\0Team1 win\0Team2 win\0Rank unlocker\0"));
}

void something_builder(c_lobby& lobby) {
	if ((lobby.current_state >= lobby_state::hard_terminated && lobby.current_state <= lobby_state::zero) || lobby.current_state == lobby_state::fully_finished) {
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		ImGui::Combo(do_xor("##Match mode"), &lobby.match_mode, strings("Draw\0Win Lose\0Win win lose lose\0Manual\0Team1 win\0Team2 win\0Rank unlocker\0"));

		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(strings("Match mode"));
		}

		if (lobby.match_mode == 1 || lobby.match_mode == 2 || lobby.match_mode == 6)
		{
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
			ImGui::Checkbox(strings("Start with team 2"), &lobby.start_with_team2);
		}

		if (lobby.match_mode != 6) {
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
			if (ImGui::InputInt(do_xor("##Repetition"), &lobby.repetition)) {
				lobby.repetition = std::clamp(lobby.repetition, 1, 500);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(strings("Repetition"));
			}
		}

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		ImGui::Combo(strings("Map"), &lobby.map, map_names, IM_ARRAYSIZE(map_names));

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		if (ImGui::BeginCombo(do_xor("##Select accounts"), strings("Select accounts"))) {
			for (int i = 1; i < 500; i++) {
				bool should_disable = false;
				if (data.get_accs_team(i).size() == 0) continue;
				if (data.get_accs_team(i).size() < 5 || data.get_accs_team(i).size() > 5) should_disable = true;
				if (!all_accs_have_steam_id(i)) should_disable = true;
				if (is_selected_by_other_lobby(i, lobby.lobby_idx)) should_disable = true;
				ImGui::Selectable(std::string(strings("Team")).append(" ").append(std::to_string(i)).c_str(), &lobby.team_to_boost[i], should_disable ? ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_DontClosePopups);
			}
			ImGui::EndCombo();
		}

		ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - 50);

		if (ImGui::ImageButton(ui.textures["play"], ImVec2(50, 50)))
		{
			int got_lobby = false;
			lobby.valid_lobbies = 0;
			for (int l{}; l < 500; l++) {
				if (!got_lobby) {
					if (lobby.team_to_boost[l]) got_lobby = true;
				}
				else {
					if (lobby.team_to_boost[l]) {
						lobby.valid_lobbies++;
						got_lobby = false;
					}
				}
			}

			if (lobby.valid_lobbies >= 1) {
				lobby.boosting_started_epoch = utils::epoch_time();
				utils::create_logs_folder();
				boosting::start_boosting(lobby);
			}
		}
	}

	if (lobby.current_state >= lobby_state::got_handles && lobby.current_state < lobby_state::finished) {
		//ImGui::Checkbox("Stop CS:GO", &lobby.restart_cs);
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (200 - (ImGui::GetStyle().ItemSpacing.x * 2))) * 0.5f);
		if (ImGui::ImageButton(ui.textures["stop"], ImVec2(50, 50))) {
			lobby.termination_request = 1;
			utils::cout_lobby(lobby, do_xor("\"Stop boosting\" requested"));
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(ui.textures["stop_csgo"], ImVec2(50, 50))) {
			lobby.termination_request = 2;
			utils::cout_lobby(lobby, do_xor("\"Stop boosting and kill CS:GO\" requested"));
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(lobby.visible ? ui.textures["hide_windows"] : ui.textures["visible"], ImVec2(50, 50))) {
			boosting::toggle_windows(lobby);
		}
		ImGui::Text(std::string(strings("Currently boosting")).append(": ").append(strings("Team")).append(" %i ").append(strings("and")).append(" ").append(strings("Team")).append(" %i ").c_str(), lobby.team_1, lobby.team_2);
	}

	switch (lobby.current_state) {
	case lobby_state::terminated: break;
	case lobby_state::hard_terminated: break;
	case lobby_state::waiting_for_network_reconnection:
		ImGui::Text(do_xor("Waiting for network reconnection."));
		break;
	case lobby_state::setting_up:
		ImGui::Text(do_xor("Setting up lobby."));
		break;
	case lobby_state::launching_accounts:
		ImGui::Text(do_xor("Launching accounts."));
		break;
	case lobby_state::waiting_for_cs_to_launch:
		ImGui::Text(do_xor("Waiting for CS:GO to start."));
		break;
	case lobby_state::verifying_handles:
		ImGui::Text(do_xor("Waiting for CS:GO to load."));
		break;
	case lobby_state::got_handles:
		ImGui::Text(do_xor("Got handle to every CS:GO."));
		break;
	case lobby_state::getting_friend_codes:
		ImGui::Text(do_xor("Getting friend code"));
		break;
	case lobby_state::making_lobby:
		ImGui::Text(do_xor("Inviting accounts"));
		break;
	case lobby_state::in_lobby:
		ImGui::Text(do_xor("In lobby"));
		break;
	case lobby_state::mismatch:
		ImGui::Text(do_xor("Mismatch"));
		[[fallthrough]];
	case lobby_state::searching_for_game:
		ImGui::Text(do_xor("Searching for a match since the last %s"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.searching_started_epoch))).c_str());
		break;
	case lobby_state::match_found:
		ImGui::Text(do_xor("Match Found"));
		break;
	case lobby_state::lobby_search_on_cooldown:
		ImGui::Text(do_xor("Lobby search on cooldown due to mismatch"));
		break;
	case lobby_state::joining_game:
		ImGui::Text(do_xor("Waiting for all accounts to connect"));
		break;
	case lobby_state::in_warm_up:
		ImGui::Text(do_xor("Waiting for warmup to end"));
		break;
	case lobby_state::in_game:
		ImGui::Text(do_xor("In game"));
		ImGui::Text(do_xor("Score: %i / %i"), lobby.rounds_won[0], lobby.rounds_won[1]);
		[[fallthrough]];
	case lobby_state::finished:
		ImGui::Text(do_xor("Time elapsed: %s"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.boosting_started_epoch))).c_str());
		break;
	case lobby_state::fully_finished:
		ImGui::Text(do_xor("Finished boosting: %s ago"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.boosting_started_epoch))).c_str());
		break;
	}
	ImGui::Text(std::string(strings("Times boosted")).append(": %i").c_str(), lobby.times_boosted);

	std::string o__o{ "" };
	for (size_t i{}; i < 500; i++) {
		if (lobby.has_team_been_boosted[i] && lobby.team_1 != i && lobby.team_2 != i && !lobby.didnt_find_a_match[i]) {
			o__o += "Team ";
			o__o += std::to_string(i);
			o__o += "\n";
		}
	}
	if (!o__o.empty()) {
		ImGui::InputTextMultiline("##teams_completed", &o__o, { 130, 0 }, ImGuiInputTextFlags_ReadOnly);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(do_xor("Teams completed"));
		}
	}
	std::string o___o = { "" };
	for (size_t i{}; i < 500; i++) {
		if (lobby.didnt_find_a_match[i]) {
			o___o += "Team ";
			o___o += std::to_string(i);
			o___o += "\n";
		}
	}
	if (!o___o.empty()) {
		if (!o__o.empty()) {
			ImGui::SameLine();
		}
		ImGui::InputTextMultiline("##teams_skipped", &o___o, { 130, 0 }, ImGuiInputTextFlags_ReadOnly);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(do_xor("Teams skipped"));
		}
	}
}

void wingman_builder(c_lobby& lobby) {
	if ((lobby.current_state >= lobby_state::hard_terminated && lobby.current_state <= lobby_state::zero) || lobby.current_state == lobby_state::fully_finished) {
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		ImGui::Combo(do_xor("##Match mode"), &lobby.match_mode, strings("Draw\0Alternate\0Team1 win\0Team2 win\0"));

		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(strings("Match mode"));
		}

		if (lobby.match_mode == 1)
		{
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
			ImGui::Checkbox(strings("Start with team 2"), &lobby.start_with_team2);
		}

		if (lobby.match_mode != 6) {
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
			if (ImGui::InputInt(do_xor("##Repetition"), &lobby.repetition)) {
				lobby.repetition = std::clamp(lobby.repetition, 1, 500);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(strings("Repetition"));
			}
		}

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		ImGui::Combo(strings("Map"), &lobby.map_wingman, wingman_map_names, IM_ARRAYSIZE(wingman_map_names));

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		//this needs to be changed
		if (ImGui::BeginCombo(do_xor("##Select accounts"), strings("Select accounts"))) {
			for (int i = 1; i < 500; i++) {
				bool should_disable = false;
				if (data.get_accs_wingman(i).size() == 0) continue;
				if (data.get_accs_wingman(i).size() < 2 || data.get_accs_wingman(i).size() > 2) should_disable = true;
				if (!all_accs_have_steam_id(i, 1)) should_disable = true;
				if (is_selected_by_other_lobby(i, lobby.lobby_idx)) should_disable = true;
				ImGui::Selectable(std::string(strings("Team")).append(" ").append(std::to_string(i)).c_str(), &lobby.team_to_boost[i], should_disable ? ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_DontClosePopups);
			}
			ImGui::EndCombo();
		}

		ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - 50);

		if (ImGui::ImageButton(ui.textures["play"], ImVec2(50, 50)))
		{
			int got_lobby = false;
			lobby.valid_lobbies = 0;
			for (int l{}; l < 500; l++) {
				if (!got_lobby) {
					if (lobby.team_to_boost[l]) got_lobby = true;
				}
				else {
					if (lobby.team_to_boost[l]) {
						lobby.valid_lobbies++;
						got_lobby = false;
					}
				}
			}

			if (lobby.valid_lobbies >= 1) {
				lobby.boosting_started_epoch = utils::epoch_time();
				utils::create_logs_folder();
				boosting::start_boosting(lobby);
			}
		}
	}

	if (lobby.current_state >= lobby_state::got_handles && lobby.current_state < lobby_state::finished) {
		//ImGui::Checkbox("Stop CS:GO", &lobby.restart_cs);
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (200 - (ImGui::GetStyle().ItemSpacing.x * 2))) * 0.5f);
		if (ImGui::ImageButton(ui.textures["stop"], ImVec2(50, 50))) {
			lobby.termination_request = 1;
			utils::cout_lobby(lobby, do_xor("\"Stop boosting\" requested"));
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(ui.textures["stop_csgo"], ImVec2(50, 50))) {
			lobby.termination_request = 2;
			utils::cout_lobby(lobby, do_xor("\"Stop boosting and kill CS:GO\" requested"));
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(lobby.visible ? ui.textures["hide_windows"] : ui.textures["visible"], ImVec2(50, 50))) {
			boosting::toggle_windows(lobby);
		}
		ImGui::Text(std::string(strings("Currently boosting")).append(": ").append(strings("Team")).append(" %i ").append(strings("and")).append(" ").append(strings("Team")).append(" %i ").c_str(), lobby.team_1, lobby.team_2);
	}

	switch (lobby.current_state) {
	case lobby_state::terminated: break;
	case lobby_state::hard_terminated: break;
	case lobby_state::waiting_for_network_reconnection:
		ImGui::Text(do_xor("Waiting for network reconnection."));
		break;
	case lobby_state::setting_up:
		ImGui::Text(do_xor("Setting up lobby."));
		break;
	case lobby_state::launching_accounts:
		ImGui::Text(do_xor("Launching accounts."));
		break;
	case lobby_state::waiting_for_cs_to_launch:
		ImGui::Text(do_xor("Waiting for CS:GO to start."));
		break;
	case lobby_state::verifying_handles:
		ImGui::Text(do_xor("Waiting for CS:GO to load."));
		break;
	case lobby_state::got_handles:
		ImGui::Text(do_xor("Got handle to every CS:GO."));
		break;
	case lobby_state::getting_friend_codes:
		ImGui::Text(do_xor("Getting friend code"));
		break;
	case lobby_state::making_lobby:
		ImGui::Text(do_xor("Inviting accounts"));
		break;
	case lobby_state::in_lobby:
		ImGui::Text(do_xor("In lobby"));
		break;
	case lobby_state::mismatch:
		ImGui::Text(do_xor("Mismatch"));
		[[fallthrough]];
	case lobby_state::searching_for_game:
		ImGui::Text(do_xor("Searching for a match since the last %s"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.searching_started_epoch))).c_str());
		break;
	case lobby_state::match_found:
		ImGui::Text(do_xor("Match Found"));
		break;
	case lobby_state::lobby_search_on_cooldown:
		ImGui::Text(do_xor("Lobby search on cooldown due to mismatch"));
		break;
	case lobby_state::joining_game:
		ImGui::Text(do_xor("Waiting for all accounts to connect"));
		break;
	case lobby_state::in_warm_up:
		ImGui::Text(do_xor("Waiting for warmup to end"));
		break;
	case lobby_state::in_game:
		ImGui::Text(do_xor("In game"));
		ImGui::Text(do_xor("Score: %i / %i"), lobby.rounds_won[0], lobby.rounds_won[1]);
		[[fallthrough]];
	case lobby_state::finished:
		ImGui::Text(do_xor("Time elapsed: %s"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.boosting_started_epoch))).c_str());
		break;
	case lobby_state::fully_finished:
		ImGui::Text(do_xor("Finished boosting: %s ago"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.boosting_started_epoch))).c_str());
		break;
	}
	ImGui::Text(std::string(strings("Times boosted")).append(": %i").c_str(), lobby.times_boosted);

	std::string o__o{ "" };
	for (size_t i{}; i < 500; i++) {
		if (lobby.has_team_been_boosted[i] && lobby.team_1 != i && lobby.team_2 != i && !lobby.didnt_find_a_match[i]) {
			o__o += "Team ";
			o__o += std::to_string(i);
			o__o += "\n";
		}
	}
	if (!o__o.empty()) {
		ImGui::InputTextMultiline("##teams_completed", &o__o, { 130, 0 }, ImGuiInputTextFlags_ReadOnly);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(do_xor("Teams completed"));
		}
	}
	std::string o___o = { "" };
	for (size_t i{}; i < 500; i++) {
		if (lobby.didnt_find_a_match[i]) {
			o___o += "Team ";
			o___o += std::to_string(i);
			o___o += "\n";
		}
	}
	if (!o___o.empty()) {
		if (!o__o.empty()) {
			ImGui::SameLine();
		}
		ImGui::InputTextMultiline("##teams_skipped", &o___o, { 130, 0 }, ImGuiInputTextFlags_ReadOnly);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(do_xor("Teams skipped"));
		}
	}
}

void lobby_builder_lite(c_lobby& lobby) {
	lobby.match_mode = 0;
	lobby.valid_lobbies = 1;

	if ((lobby.current_state >= lobby_state::hard_terminated && lobby.current_state <= lobby_state::zero) || lobby.current_state == lobby_state::fully_finished) {
		if (ImGui::BeginCombo(strings("Team 1"), get_team_name(lobby.team_1).c_str()))
		{
			for (int i = 1; i < 500; i++) {
				bool should_disable = false;
				if (data.get_accs_team(i).size() == 0) continue;
				if (data.get_accs_team(i).size() < 5 || data.get_accs_team(i).size() > 5) should_disable = true;
				if (!all_accs_have_steam_id(i)) should_disable = true;
				if (is_selected_by_other_lobby(i)) should_disable = true;
				if (ImGui::Selectable(get_team_name(i).c_str(), lobby.team_1 == i, should_disable ? ImGuiSelectableFlags_Disabled : 0)) {
					lobby.team_1 = i;
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginCombo(strings("Team 2"), get_team_name(lobby.team_2).c_str()))
		{
			for (int i = 1; i < 500; i++) {
				bool should_disable = false;
				if (data.get_accs_team(i).size() == 0) continue;
				if (data.get_accs_team(i).size() < 5 || data.get_accs_team(i).size() > 5) should_disable = true;
				if (!all_accs_have_steam_id(i)) should_disable = true;
				if (is_selected_by_other_lobby(i)) should_disable = true;
				if (ImGui::Selectable(get_team_name(i).c_str(), lobby.team_2 == i, should_disable ? ImGuiSelectableFlags_Disabled : 0)) {
					lobby.team_2 = i;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		if (ImGui::InputInt(do_xor("##Repetition"), &lobby.repetition)) {
			lobby.repetition = std::clamp(lobby.repetition, 1, 500);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(strings("Repetition"));
		}
		ImGui::Combo(strings("Map"), &lobby.map, map_names, IM_ARRAYSIZE(map_names));

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		if (ImGui::Button(strings("Start"), ImVec2(ImGui::GetWindowWidth() * 0.5f, 40)))
		{
			if (lobby.team_1 && lobby.team_2) {
				lobby.boosting_started_epoch = utils::epoch_time();
				utils::create_logs_folder();
				boosting::start_boosting(lobby);
			}
		}
	}

	if (lobby.current_state == lobby_state::verifying_handles) {
		if (ImGui::ButtonEx2(strings("Continue"), ImVec2(ImGui::GetWindowWidth() * 0.84f, 40)))
		{
			lobby.current_state = lobby_state::got_handles;
		}
	}

	if (lobby.current_state >= lobby_state::got_handles && lobby.current_state < lobby_state::finished) {
		//ImGui::Checkbox("Stop CS:GO", &lobby.restart_cs);
		if (ImGui::Button(strings("Stop boosting"), ImVec2(ImGui::GetWindowWidth() * 0.649f, 40))) {
			lobby.termination_request = 1;
			utils::cout_lobby(lobby, do_xor("\"Stop boosting\" requested"));
		}
		if (ImGui::Button(strings("Stop boosting and kill CS:GO"), ImVec2(ImGui::GetWindowWidth() * 0.649f, 40))) {
			lobby.termination_request = 2;
			utils::cout_lobby(lobby, do_xor("\"Stop boosting and kill CS:GO\" requested"));
		}
		if (ImGui::Button(strings("Toggle window visibility"), ImVec2(ImGui::GetWindowWidth() * 0.649f, 40))) {
			boosting::toggle_windows(lobby);
		}
		ImGui::Text(std::string(strings("Currently boosting")).append(": ").append(strings("Team")).append(" %i ").append(strings("and")).append(" ").append(strings("Team")).append(" %i ").c_str(), lobby.team_1, lobby.team_2);
	}

	switch (lobby.current_state) {
	case lobby_state::terminated: break;
	case lobby_state::hard_terminated: break;
	case lobby_state::waiting_for_network_reconnection:
		ImGui::Text(do_xor("Waiting for network reconnection."));
		break;
	case lobby_state::setting_up:
		ImGui::Text(do_xor("Setting up lobby."));
		break;
	case lobby_state::launching_accounts:
		ImGui::Text(do_xor("Launching accounts."));
		break;
	case lobby_state::waiting_for_cs_to_launch:
		ImGui::Text(do_xor("Waiting for CS:GO to start."));
		break;
	case lobby_state::verifying_handles:
		ImGui::Text(do_xor("Waiting for CS:GO to load."));
		break;
	case lobby_state::got_handles:
		ImGui::Text(do_xor("Got handle to every CS:GO."));
		break;
	case lobby_state::getting_friend_codes:
		ImGui::Text(do_xor("Getting friend code"));
		break;
	case lobby_state::making_lobby:
		ImGui::Text(do_xor("Inviting accounts"));
		break;
	case lobby_state::in_lobby:
		ImGui::Text(do_xor("In lobby"));
		break;
	case lobby_state::mismatch:
		ImGui::Text(do_xor("Mismatch"));
		[[fallthrough]];
	case lobby_state::searching_for_game:
		ImGui::Text(do_xor("Searching for a match since the last %s"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.searching_started_epoch))).c_str());
		break;
	case lobby_state::match_found:
		ImGui::Text(do_xor("Match Found"));
		break;
	case lobby_state::lobby_search_on_cooldown:
		ImGui::Text(do_xor("Lobby search on cooldown due to mismatch"));
		break;
	case lobby_state::joining_game:
		ImGui::Text(do_xor("Waiting for all accounts to connect"));
		break;
	case lobby_state::in_warm_up:
		ImGui::Text(do_xor("Waiting for warmup to end"));
		break;
	case lobby_state::in_game:
		ImGui::Text(do_xor("In game"));
		ImGui::Text("Score: %i / %i", lobby.rounds_won[0], lobby.rounds_won[1]);
		[[fallthrough]];
	case lobby_state::finished:
		ImGui::Text(do_xor("Time elapsed: %s"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.boosting_started_epoch))).c_str());
		break;
	case lobby_state::fully_finished:
		ImGui::Text(do_xor("Finished boosting: %s ago"), seconds_to_dhms(static_cast<int>(abs(utils::epoch_time() - lobby.boosting_started_epoch))).c_str());
		break;
	}
	ImGui::Text(std::string(strings("Times boosted")).append(": %i").c_str(), lobby.times_boosted);

	std::string o__o{ "" };
	for (size_t i{}; i < 500; i++) {
		if (lobby.has_team_been_boosted[i] && lobby.team_1 != i && lobby.team_2 != i) {
			o__o += "Team ";
			o__o += std::to_string(i);
			o__o += "\n";
		}
	}
	if (!o__o.empty()) {
		ImGui::InputTextMultiline("##teams_completed", &o__o, {}, ImGuiInputTextFlags_ReadOnly);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(do_xor("Teams completed"));
		}
	}
}

void lobby_builder(c_lobby& lobby) {
	if ((lobby.current_state >= lobby_state::hard_terminated && lobby.current_state <= lobby_state::zero) || lobby.current_state == lobby_state::fully_finished) {
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
		ImGui::Combo(do_xor("##Game Type"), &lobby.game_type, strings("Competitive\0Wingman\0"));
	}
	if (lobby.game_type == 0) {
		if (lobby.match_mode == 3) {
			manual_boost_builder(lobby);
		}
		else {
			something_builder(lobby);
		}
	}
	else {
		wingman_builder(lobby);
	}
}

void boost_page() {
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	static int lobby_id{ 1 };
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderInt(do_xor("##lobby_id"), &lobby_id, 1, MAX_LOBBIES, do_xor("Lobby %d"));
	ImGui::Dummy({ 20, 20 });
	ImGui::BeginChild(do_xor("bottom"), { ImGui::GetWindowWidth() * 0.92f , ImGui::GetWindowHeight() * 0.92f - 80 });
	{
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		switch (g_cfg.key_type) {
			case 1: lobby_builder(lobbies[lobby_id - 1]); break;
			case 2: lobby_builder_lite(lobbies[lobby_id - 1]); break;
			case 3: ImGui::Text(do_xor("Banned!")); break;
			default: ImGui::Text(do_xor(" Unknown error!")); break;
		}
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	ImGui::PopItemWidth();
}

constexpr long long steam3_64_differnce = 76561199023777455 - 1063511727;

static int banned = 0;
void check_for_bans() {
	if (locks["checking for bans"]) {
		utils::messagebox("Ban checker in progress");
		return;
	}
	else {
		locks["checking for bans"] = true;
		utils::messagebox("Ban checker starts after clicking ok");
	}

	banned = 0;
	for (size_t i = 0; i < data.accs.size(); i++) {
		auto& d = data.accs[i];
		long long k = std::stoll(d.s_id) + steam3_64_differnce;
		d.is_banned = utils::is_profile_banned(k);

		if (d.is_banned) {
			banned++;
			utils::messagebox((d.user + std::string(" is banned.")).c_str());
		}
	}
	locks["checking for bans"] = false;
}

void settings_page() {
	ImGui::Text(do_xor("Number of accounts: %u"), data.accs.size());
	ImGui::Combo(strings("Language"), &g_cfg.language, do_xor("Chinese\0English\0"));

	if (ImGui::Button(strings("Check for bans"))) {
		boosting::threads.push_back(std::thread(check_for_bans));
	}
	ImGui::SameLine();
	ImGui::Text("Number of bans: %i", banned);

	static int lang{ g_cfg.language };
	static int log_prefrence{ g_cfg.log_to };
	static float wait_time{ g_cfg.time_to_wait_for_match };
	static std::string steam{ g_cfg.steam_directory };
	static std::string csgo{ g_cfg.csgo_directory };
	static std::string webhook{ g_cfg.webhook_url };

	ImGui::Combo("Log to", &g_cfg.log_to, "Discord\0vertigopanel.com/logs.php\0");
	ImGui::SliderFloat("##Time to wait for a match", &g_cfg.time_to_wait_for_match, 5, 60, "Time to wait for a match: %.2f");
	g_cfg.time_to_wait_for_match = std::clamp(g_cfg.time_to_wait_for_match, 5.f, 60.f);
	ImGui::InputTextWithHint("##steam directory", strings("Steam directory"), &g_cfg.steam_directory);
	ImGui::InputTextWithHint(do_xor("##csgo directory"), strings("CS:GO directory"), &g_cfg.csgo_directory);

	if (g_cfg.log_to == 0) {
		ImGui::InputTextWithHint(do_xor("##webhook url"), strings("Webhook url"), &g_cfg.webhook_url);
		if (!g_cfg.webhook_url.empty()) {
			if (ImGui::Button(strings("Test webhook"))) {
				utils::webhook(strings("Hello"));
			}
		}
	}

	if (g_cfg.webhook_url != webhook && !g_cfg.webhook_url.empty()) {
		update_config();
		webhook = g_cfg.webhook_url;
	}
	else if (g_cfg.csgo_directory != csgo && !g_cfg.csgo_directory.empty()) {
		update_config();
		csgo = g_cfg.csgo_directory;
	}
	else if (g_cfg.steam_directory != steam && !g_cfg.steam_directory.empty()) {
		update_config();
		steam = g_cfg.steam_directory;
	}
	else if (g_cfg.language != lang) {
		update_config();
		lang = g_cfg.language;
	}
	else if (g_cfg.log_to != log_prefrence) {
		update_config();
		log_prefrence = g_cfg.log_to;
	}
	else if (g_cfg.time_to_wait_for_match != wait_time) {
		update_config();
		wait_time = g_cfg.time_to_wait_for_match;
	}
}

void c_ui::render() {
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.192156862745098f, 0.2941176470588235f, 0.6235294117647059f, 1.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	{
		if (g_cfg.logged_in) {
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.2f, 1.f));
			panel();
			ImGui::PopStyleColor();
		}
		else if (g_cfg.has_spare_key && !g_cfg.logged_in) {
			static bool once = false;
			if (!once) {
				window_darkness = 0;
				once = true;
			}

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.09f, 0.29f, 1.f));
			register_key();
			ImGui::PopStyleColor();
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.09f, 0.29f, 1.f));
			login();
			ImGui::PopStyleColor();
		}
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	//msg.render();
}

void c_ui::panel() {
	if (!g_cfg.active) return;

	ImGui::SetNextWindowPos(panel_page_pos, ImGuiCond_Once);
	ImGui::SetNextWindowSize(panel_page_size, ImGuiCond_Once);
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::Begin(g_cfg.title, &g_cfg.active, window_flags);
	{
		ImGui::BeginChild(do_xor("##panel"), ImVec2(0, 0), false);
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.15f, 1.f));
			ImGui::BeginChild(do_xor("##panel_left"), ImVec2(200, 0), false);
			{
				panel_left();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::SameLine(200);
			ImGui::BeginChild(do_xor("##panel_right"), ImVec2(0, 0), false);
			{
				panel_right();
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
		g_cfg.active_windows++;
	}
	ImGui::End();
}

void add_tab(const char* title, int idx)
{
	/*if (g_cfg.page == idx) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.2f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.2f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.2f, 1.f));
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.15f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.1f, 0.15f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.15f, 1.f));
	}*/

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));

	if (ImGui::Button(title, { 200, 40 })) {
		g_cfg.page = idx;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void c_ui::panel_right() {
	static const ImU32 color_528ad0 = ImColor(82, 138, 208, 255);
	static const ImU32 color_1d2481 = ImColor(29, 36, 129, 255);

	ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), color_528ad0, color_1d2481, color_1d2481, color_528ad0);
	if (!g_cfg.csgo_directory.length() || !g_cfg.steam_directory.length()) {
		g_cfg.page = 2;
	}
	if (g_cfg.page == 2 || !g_cfg.csgo_directory.length() || !g_cfg.steam_directory.length()) {
		ImGui::SetCursorPosX(1 + ImGui::GetCursorPosX() + ImGui::GetWindowWidth() * 0.04f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetWindowHeight() * 0.04f);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 20, 20 });
		ImGui::BeginChild(do_xor("##idk"), ImVec2(ImGui::GetWindowWidth() * 0.92f, ImGui::GetWindowHeight() * 0.92f));
		{
			settings_page();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}
	else if (g_cfg.page == 0) {
		ImGui::SetCursorPosX(1 + ImGui::GetCursorPosX() + ImGui::GetWindowWidth() * 0.04f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetWindowHeight() * 0.04f);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 20, 20 });
		ImGui::BeginChild(do_xor("##idk"), ImVec2(ImGui::GetWindowWidth() * 0.92f, ImGui::GetWindowHeight() * 0.92f));
		{
			accounts_page();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}
	else if (g_cfg.page == 1) {
		ImGui::SetCursorPosX(1 + ImGui::GetCursorPosX() + ImGui::GetWindowWidth() * 0.04f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetWindowHeight() * 0.04f);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 20, 20 });
		//ImGui::BeginChild("##idk", ImVec2(ImGui::GetWindowWidth() * 0.92f, ImGui::GetWindowHeight() * 0.92f), true);
		ImGui::BeginGroup();
		{
			boost_page();
		}
		ImGui::EndGroup();
		//ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}
	top_right_close_minimize_buttons();
}

void c_ui::panel_left() {
	ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImColor(93, 152, 226, 255), ImColor(93, 152, 226, 255), 0x5d98e2ff, 0x5d98e2ff);
	ImGui::GetWindowDrawList()->AddImageRounded(textures["logo"], ImGui::GetWindowPos() + ImVec2(40, 40), ImGui::GetWindowPos() + ImVec2(160, 160), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, 1.f), 16.f);
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(g_cfg.username.c_str()).x) * 0.5f);
	ImGui::SetCursorPosY(175);
	ImGui::Text(g_cfg.username.c_str());

	//ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 1, 0), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 0), ImColor(1.f, 1.f, 1.f, 1.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

	add_tab(strings("Accounts"), 0);
	add_tab(strings("Boost"), 1);
	add_tab(strings("Settings"), 2);

	ImGui::PopStyleVar();

	//ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 25);
}

void c_ui::login() {
	if (!g_cfg.active) return;

	ImGui::SetNextWindowPos(login_page_pos, ImGuiCond_Once);
	ImGui::SetNextWindowSize(login_page_size, ImGuiCond_Once);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin(g_cfg.title, &g_cfg.active, window_flags);
	{
		ImGui::partices();
		ImGui::BeginChild(do_xor("login"), ImVec2(0, 0), false, g_cfg.should_login ? ImGuiWindowFlags_NoInputs : 0);
		{
			ImGui::SetCursorPos(ImVec2(8, 4));

			ImGui::Text(strings("Vertigo Panel"));

			{
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2931442357583202f);
				{
					ImGui::SetCursorPosY((ImGui::GetWindowHeight() - 128) * 0.5f);

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					ImGui::SetCursorPosX(303);
					if (ImGui::InputTextWithHint(do_xor("##Username"), strings("Username"), &g_cfg.username, ImGuiInputTextFlags_EnterReturnsTrue)) g_cfg.should_login = true;

					ImGui::SetCursorPosX(303);
					if (ImGui::InputTextWithHint(do_xor("##Password"), strings("Password"), &g_cfg.password, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue)) g_cfg.should_login = true;
					ImGui::PopStyleVar();

					ImGui::SetCursorPosX(303);
					if (ImGui::ButtonEx2(strings("Login"), ImVec2(248, 40))) g_cfg.should_login = true;

					if (g_cfg.should_login) {
						if (std::string(g_cfg.username).length() == 0) {
							g_cfg.should_login = false;
							utils::messagebox(strings("Username empty"), strings("Error"));
						}
						else if (std::string(g_cfg.password).length() == 0) {
							g_cfg.should_login = false;
							utils::messagebox(do_xor("Password empty"), do_xor("Error"));
						}
						else if (std::string(g_cfg.username).length() < 2) {
							g_cfg.should_login = false;
							utils::messagebox(do_xor("Username too small!"), do_xor("Error"));
						}
						else if (std::string(g_cfg.password).length() < 8) {
							g_cfg.should_login = false;
							utils::messagebox(do_xor("Password too small!"), do_xor("Error"));
						}
						else if (!g_cfg.login_thread_created) {
							update_config();
							LI_FN(CreateThread).cached()(0, 0, (LPTHREAD_START_ROUTINE)login_thread, NULL, NULL, NULL);
							g_cfg.login_thread_created = true;
						}
					}
				}
				ImGui::PopItemWidth();
			}

			top_right_close_minimize_buttons();

			ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImColor(0.f, 0.f, 0.f, window_darkness));
			ImGui::Spinner(ImGui::GetWindowPos() + ImGui::GetWindowSize() * 0.5f, 32.f, 3.f, ImColor(1.f, 1.f, 1.f, window_darkness));

			if (g_cfg.should_login && !g_cfg.logged_in) {
				window_darkness += ImGui::GetIO().DeltaTime;
				window_darkness = std::clamp(window_darkness, 0.f, 0.75f);
			}
		}
		ImGui::EndChild();
		g_cfg.active_windows++;
	}
	ImGui::End();
}

void c_ui::top_right_close_minimize_buttons() {
	ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 60, 0 });
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

		if (ImGui::Button("-", { 25, 25 })) {
			PostMessage(FindWindow(NULL, g_cfg.title), WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		ImGui::SameLine();
		if (ImGui::Button("x", { 25, 25 })) {
			g_cfg.active = false;
			if (g_cfg.logged_in) {
				utils::kill_all_cs();
				utils::kill_all_steam();
				anti_debug::erase_portable_executable_header_from_memory();
			}
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}
}

void c_ui::register_key() {
	if (!g_cfg.active) return;

	ImGui::SetNextWindowPos(login_page_pos, ImGuiCond_Once);
	ImGui::SetNextWindowSize(login_page_size, ImGuiCond_Once);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin(g_cfg.title, &g_cfg.active, window_flags);
	{
		ImGui::partices();
		ImGui::BeginChild(do_xor("register key"), ImVec2(0, 0), false, g_cfg.should_register_key ? ImGuiWindowFlags_NoInputs : 0);
		{
			ImGui::SetCursorPos(ImVec2(8, 4));
			ImGui::Text(g_cfg.title);

			{
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2931442357583202f);
				{
					ImGui::SetCursorPosY((ImGui::GetWindowHeight() - 85) * 0.5f);

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					ImGui::SetCursorPosX(303);
					if (ImGui::InputTextWithHint(do_xor("##Product Key"), do_xor("Product Key"), &g_cfg.product_key, ImGuiInputTextFlags_EnterReturnsTrue)) g_cfg.should_register_key = true;
					ImGui::PopStyleVar();

					ImGui::SetCursorPosX(303);
					if (ImGui::ButtonEx2(do_xor("Register"), ImVec2(248, 40))) g_cfg.should_register_key = true;

					if (g_cfg.should_register_key) {
						if (g_cfg.product_key.length() == 0) {
							g_cfg.should_register_key = false;
							utils::messagebox(do_xor("Key empty"), do_xor("Error"));
						}
						else if (g_cfg.product_key.length() < 64) {
							g_cfg.should_register_key = false;
							utils::messagebox(do_xor("Key too small!"), do_xor("Error"));
						}
						else if (g_cfg.product_key.length() > 64) {
							g_cfg.should_register_key = false;
							utils::messagebox(do_xor("Key too large!"), do_xor("Error"));
						}
						else if (!g_cfg.set_key_thread_created) {
							LI_FN(CreateThread).cached()(0, 0, (LPTHREAD_START_ROUTINE)set_key_thread, NULL, NULL, NULL);
							g_cfg.set_key_thread_created = true;
						}
					}
				}
				ImGui::PopItemWidth();
			}

			top_right_close_minimize_buttons();

			ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImColor(0.f, 0.f, 0.f, window_darkness));
			ImGui::Spinner(ImGui::GetWindowPos() + ImGui::GetWindowSize() * 0.5f, 32.f, 3.f, ImColor(1.f, 1.f, 1.f, window_darkness));

			if (g_cfg.should_register_key && !g_cfg.logged_in) {
				window_darkness += ImGui::GetIO().DeltaTime;
				window_darkness = std::clamp(window_darkness, 0.f, 0.75f);
			}
		}
		ImGui::EndChild();
		g_cfg.active_windows++;
	}
	ImGui::End();
}

#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")

#include "../texture/logo.h"
#include "../texture/stop.h"
#include "../texture/play.h"
#include "../texture/visible.h"
#include "../texture/stop_csgo.h"
#include "../texture/hide_windows.h"

void c_ui::init(LPDIRECT3DDEVICE9 d) {
	dev = d;
	ImGui::StyleColorsDark();
	if (panel_page_pos.x == 0) {
		RECT screen_rect{};
		LI_FN(GetWindowRect).cached()(LI_FN(GetDesktopWindow).cached()(), &screen_rect);
		screen_res = ImVec2(float(screen_rect.right), float(screen_rect.bottom));
		login_page_pos = (screen_res - login_page_size) * 0.5f;
		panel_page_pos = (screen_res - panel_page_size) * 0.5f;

		D3DXCreateTextureFromFileInMemory(ui.dev, logo, sizeof(logo), &textures["logo"]);
		D3DXCreateTextureFromFileInMemory(ui.dev, stop, sizeof(stop), &textures["stop"]);
		D3DXCreateTextureFromFileInMemory(ui.dev, play, sizeof(play), &textures["play"]);
		D3DXCreateTextureFromFileInMemory(ui.dev, visible, sizeof(visible), &textures["visible"]);
		D3DXCreateTextureFromFileInMemory(ui.dev, stop_csgo, sizeof(stop_csgo), &textures["stop_csgo"]);
		D3DXCreateTextureFromFileInMemory(ui.dev, hide_windows, sizeof(hide_windows), &textures["hide_windows"]);

		get_config();
	}
}

#include <encryption/encryption.hh>
void get_config() {
	nlohmann::json object;
	object.clear();

	std::ifstream in(g_cfg.assets_folder + std::string(do_xor("\\panel\\config.dat")));

	if (in.good()) {
		std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		object = nlohmann::json::parse(encryption::base4::decode(str), nullptr, false);

		if (object.is_discarded()) {
			in.close();
			return;
		}
	}
	else { return; }
	in.close();

	if (object.contains(do_xor("login"))) object[do_xor("login")].get_to(g_cfg.username);
	if (object.contains(do_xor("pass"))) object[do_xor("pass")].get_to(g_cfg.password);
	if (object.contains(do_xor("webhook_url"))) object[do_xor("webhook_url")].get_to(g_cfg.webhook_url);
	if (object.contains(do_xor("csgo_directory"))) object[do_xor("csgo_directory")].get_to(g_cfg.csgo_directory);
	if (object.contains(do_xor("steam_directory"))) object[do_xor("steam_directory")].get_to(g_cfg.steam_directory);
	if (object.contains(do_xor("language"))) object[do_xor("language")].get_to(g_cfg.language);
	if (object.contains(do_xor("log_to"))) object[do_xor("log_to")].get_to(g_cfg.log_to);
	if (object.contains(do_xor("time_to_wait_for_match"))) object[do_xor("time_to_wait_for_match")].get_to(g_cfg.time_to_wait_for_match);
}

void update_config() {
	std::ofstream o(g_cfg.assets_folder + std::string(do_xor("\\panel\\config.dat")));
	if (!o.is_open()) {
		return;
	}

	nlohmann::json object;
	object.clear();

	object[do_xor("login")] = g_cfg.username;
	object[do_xor("pass")] = g_cfg.password;
	object[do_xor("csgo_directory")] = g_cfg.csgo_directory;
	object[do_xor("steam_directory")] = g_cfg.steam_directory;
	object[do_xor("last_steam_id_parse")] = g_cfg.last_steam_id_parse;
	object[do_xor("webhook_url")] = g_cfg.webhook_url;
	object[do_xor("key")] = g_cfg.product_key;
	object[do_xor("language")] = g_cfg.language;
	object[do_xor("log_to")] = g_cfg.log_to;
	object[do_xor("time_to_wait_for_match")] = g_cfg.time_to_wait_for_match;

	o << encryption::base4::encode(object.dump());
	o.close();
}