#pragma once
#include <d3d9.h>
#include <string>
#include <unordered_map>

#include <render/imgui/imgui.h>
#include <render/imgui/imgui_internal.h>

class c_ui {
public:
	void init(LPDIRECT3DDEVICE9);
	void render();
private:
	void login();
	void top_right_close_minimize_buttons();
	void panel();
	void register_key();
private:
	void panel_left();
	void panel_right();
public:
	ImVec2 screen_res{ 000, 000 };
private:
	ImVec2 login_page_pos{ 000, 000 };
	ImVec2 panel_page_pos{ 000, 000 };
private:
	ImVec2 login_page_size{ 846, 530 };
	ImVec2 panel_page_size{ 846, 530 };
	DWORD window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
private:
	float window_darkness{ 0.f };
	float window_darkness2{ 0.f };
public:
	LPDIRECT3DDEVICE9 dev;
	std::unordered_map<std::string, LPDIRECT3DTEXTURE9> textures;
};

inline c_ui ui;