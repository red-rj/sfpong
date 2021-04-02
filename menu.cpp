#include "menu.h"
#include "convert.h"
#include "game.h"
#include "imgui_inc.h"

#include <algorithm>
#include <optional>
#include <vector>

#include <imgui-SFML.h>
#include <fmt/ostream.h>
#include <SFML/Window/Window.hpp>
#include <boost/version.hpp>

using namespace std::literals;

static auto scan_kb() noexcept -> std::optional<sf::Keyboard::Key>
{
	using sf::Keyboard;

	for (int i = 0; i < Keyboard::KeyCount; i++)
	{
		auto key = Keyboard::Key(i);
		if (Keyboard::isKeyPressed(key)) {
			return key;
		}
	}

	return {};
}

#if 0
static auto scan_mouse_btn() noexcept -> std::optional<sf::Mouse::Button>
{
	using sf::Mouse;

	for (int i = 0; i < Mouse::ButtonCount; i++)
	{
		auto btn = Mouse::Button(i);
		if (Mouse::isButtonPressed(btn))
			return btn;
	}

	return {};
}

static auto scan_joy_btn() noexcept
{
	using sf::Joystick;

	for (unsigned id = 0; id < Joystick::Count; id++)
	{
		const auto btnCount = Joystick::getButtonCount(id);
		for (unsigned btn = 0; btn < btnCount; btn++)
		{
			if (Joystick::isButtonPressed(id, btn))
			{
				return std::make_tuple(btn, id);
			}
		}
	}
}
#endif

using namespace pong;
using win = pong::menu::win::Id;

namespace
{
	pong::game_settings work_settings;
	pong::game_settings *settings;

	std::array<std::string, sf::Joystick::Count> _joystick_names;
	int _joystick_count;

	// font ids
	namespace ft { enum Id {
		default_,
		default_large,
		monospace,
		section_title,

		Count
	};}

	// show flags
	bool isVisible[win::Count] = {};

	//fonts
	ImFont* fonts[ft::Count] = {};

}


// windows
static void optionsWin(game& ctx, sf::Window& window);
static void gameStatsWin(game& ctx);
static void aboutSfPongWin();

// ui
static void controlsUi();
static int joystickCombobox(int current_joyid);

static void refresh_joysticks()
{
	using sf::Joystick;

	for (int i=0; i < Joystick::Count; i++)
	{
		if (Joystick::isConnected(i)) {
			auto info = Joystick::getIdentification(i);
			_joystick_names[i] = info.name;
			_joystick_count++;
		}
	}
}
static void clear_joysticks()
{
	for_each_n(_joystick_names.begin(), _joystick_count, [](std::string& n) { n.clear(); });
	_joystick_count = 0;
}


void pong::menu::update(game& ctx, sf::Window& window)
{
	using namespace ImGui;
	using namespace ImScoped;
	using namespace win;

	if (isVisible[game_stats])
		gameStatsWin(ctx);
	
	if (ctx.is_paused()) {
		// windows
		if (isVisible[options])
			optionsWin(ctx, window);
		if (isVisible[about])
			aboutSfPongWin();
		if (isVisible[imgui_demo])
			ImGui::ShowDemoWindow(&isVisible[imgui_demo]);
		if (isVisible[imgui_about])
			ImGui::ShowAboutWindow(&isVisible[imgui_about]);

		// main menu
		if (auto mm = MainMenuBar()) {
			TextDisabled("sfPong");

			if (auto m = Menu("Jogo")) {
				auto currentMode = ctx.mode();
				bool singleplayer = currentMode == gamemode::singleplayer,
					multiplayer = currentMode == gamemode::multiplayer;

				if (MenuItem("Continuar", "ESC"))
					ctx.unpause();
				if (auto m1 = Menu("Novo")) {

					if (MenuItem("1 jogador", nullptr, singleplayer)) {
						ctx.newGame(gamemode::singleplayer);
						ctx.unpause();
					}
					if (MenuItem("2 jogadores", nullptr, multiplayer)) {
						ctx.newGame(gamemode::multiplayer);
						ctx.unpause();
					}
				}
				if (MenuItem("Reiniciar")) {
					ctx.restart();
					ctx.unpause();
				}
				Separator();
				MenuItem("Sobre", nullptr, &isVisible[about]);
			}

			MenuItem(u8"Opções", nullptr, &isVisible[options]);

			if (auto m = Menu("Extra")) {
				MenuItem("Stats", "F12", &isVisible[game_stats]);
			}

			StyleColor _s_[] = {
				{ImGuiCol_Button, sf::Color::Transparent},
				{ImGuiCol_ButtonHovered, sf::Color::Red},
				{ImGuiCol_ButtonActive, sf::Color(255, 50, 50)}
			};

			if (Button("Sair")) {
				log::info("Tchau! :)");
				window.close();
			}
		}
	}
}

void pong::menu::init(game_settings* gs)
{
	settings = gs;
	work_settings = *gs;
	refresh_joysticks();

	auto* atlas = ImGui::GetIO().Fonts;
	const auto ui_font_size = 18.f;

	atlas->Clear();
	fonts[ft::default_] = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size);
	fonts[ft::default_large] = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size * 2);
	fonts[ft::section_title] = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size * 1.25f);
	//fonts[ft::monospace] = atlas->AddFontFromFileTTF(pong::files::mono_tff, ui_font_size);
	ImGui::SFML::UpdateFontTexture();
}

void pong::menu::processEvent(sf::Event& event)
{
	using sf::Event;

	switch (event.type)
	{
	case Event::JoystickConnected:
	case Event::JoystickDisconnected:
		clear_joysticks();
		refresh_joysticks();
		break;
	case Event::KeyPressed:
		if (event.key.code == sf::Keyboard::F12) {
			isVisible[win::game_stats] = !isVisible[win::game_stats];
		}
		break;
	}
}

bool pong::menu::is_open(win::Id id) noexcept
{
	return isVisible[id];
}


void optionsWin(game&, sf::Window& window)
{
	namespace gui = ImScoped;

	const bool isDirty = work_settings != *settings;
	auto wflags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (isDirty) wflags |= ImGuiWindowFlags_UnsavedDocument;

	gui::Window guiwindow(u8"Opções", &isVisible[win::options], wflags);
	if (!guiwindow)
		return;

{
	auto guiwindowsize = ImGui::GetWindowSize();
	gui::Child _content_{ "conteudo opções", { guiwindowsize.x - 10, guiwindowsize.y - 100 } };

	if (auto tabbar = gui::TabBar("##Tabs"))
	{
		if (auto tab = gui::TabBarItem("Game"))
		{
			auto& vidModes = sf::VideoMode::getFullscreenModes();
			auto curVidMode = sf::VideoMode(work_settings.resolution().x, work_settings.resolution().y);
			auto preview = fmt::format("{} x {}", curVidMode.width, curVidMode.height);

			if (auto cb = gui::Combo(u8"Resolução", preview.c_str())) {
				for (auto& vid : vidModes) {
					preview = fmt::format("{} x {}", vid.width, vid.height);
					auto isSelected = vid == curVidMode;
					if (ImGui::Selectable(preview.c_str(), isSelected)) {
						work_settings.resolution().x = vid.width;
						work_settings.resolution().y = vid.height;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
			}
		}
		if (auto tab = gui::TabBarItem("Controles"))
		{
			controlsUi();
		}
		if (auto tab = gui::TabBarItem("DEV"))
		{
			using ImGui::Button;

			if (Button("game stats"))
			{
				isVisible[win::game_stats] = true;
			}
			if (Button("ImGui demo window"))
			{
				isVisible[win::imgui_demo] = true;
			}
		}
	}
}

	ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::Button("Descartar")) {
		work_settings = *settings;
	}
	ImGui::SameLine();
	if (ImGui::Button("Salvar") && isDirty)
	{
		*settings = work_settings;
		window.setSize(settings->resolution());
	}
}

void gameStatsWin(game& ctx)
{
	namespace ims = ImScoped;

	ImGuiIO& io = ImGui::GetIO();
	pos winpos = { io.DisplaySize.x - 10.f, 15.f };
	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(winpos, ImGuiCond_Always, { 1.f, 0 });

	auto const wflags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings 
						| ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ims::Window overlay("Stats", &isVisible[win::game_stats], wflags);

	auto players = ctx.get_players();
	auto& Ball = ctx.get_ball();

#define VEC2 "[{:.2f},{:.2f}]"

	enum { P1, P2, B };

	point Pos[] = {
		players.first.getPosition(),
		players.second.getPosition(),
		Ball.getPosition()
	};

	auto text = fmt::format("P1: " VEC2 "\n" "P2: " VEC2 "\n" "Ball: " VEC2 "",
		Pos[P1].x, Pos[P1].y, Pos[P2].x, Pos[P2].y, Pos[B].x, Pos[B].y);

	ImGui::Text("Positions:\n%s", text.c_str());

#define VEC2 "[{:.3f},{:.3f}]"

	text = fmt::format("P1: {:.3f}\nP2: {:.3f}\nBall: " VEC2 "", 
		players.first.velocity, players.second.velocity, Ball.velocity.x, Ball.velocity.y);

	ImGui::Text("Velocity:\n%s", text.c_str());

#undef VEC2
}

void aboutSfPongWin()
{
	using namespace ImGui;
	namespace gui = ImScoped;

	auto win = gui::Window("Sobre sfPong", &isVisible[win::about], ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
	if (!win) return;

	Text("sfPong %s", pong::version);
	Text("Criado por Pedro Oliva Rodrigues.");
	Separator();
{
	using ImScoped::StyleColor;
	StyleColor _a_{ ImGuiCol_Button, sf::Color::Transparent };

	if (SmallButton("ImGui: " IMGUI_VERSION))
		isVisible[win::imgui_about] = true;
}

	const auto libver = " %s: %d.%d.%d";
	Text(libver, "SFML", SFML_VERSION_MAJOR, SFML_VERSION_MINOR, SFML_VERSION_PATCH);
	Text(libver, "Boost",
		BOOST_VERSION / 100000,
		BOOST_VERSION / 100 % 1000,
		BOOST_VERSION % 100
	);
	Text(libver, "fmtlib",
		FMT_VERSION / 10000,
		FMT_VERSION / 100 % 100,
		FMT_VERSION % 100
	);
	Text(libver, "spdlog", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
}

void controlsUi()
{
	namespace gui = ImScoped;

	{
		gui::Font _f_ = fonts[ft::section_title];
		ImGui::Text("Teclado:");
	}

	auto inputKbKey = [id = 0](const char* label, sf::Keyboard::Key& curKey) mutable
	#pragma region Block
	{
		using namespace ImGui;

		gui::ID _id_ = id++;
		auto constexpr popup_id = "Rebind popup";
		auto keystr = fmt::format("{}", curKey);

		// butão
		Text("%5s:", label);
		SameLine(75);
		if (Button(keystr.c_str())) {
			OpenPopup(popup_id);
			isVisible[win::rebiding_popup] = true;
		}

		// popup
		const auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav;
		if (auto popup = gui::PopupModal(popup_id, &isVisible[win::rebiding_popup], flags))
		{
			using Key = sf::Keyboard::Key;

			gui::Font _f_{ fonts[ft::default_large] };
			Text("Pressione uma nova tecla para '%s', ou Esc para cancelar.", label);

			auto key = scan_kb();
			if (key)
			{
				if (key.value() != Key::Escape)
					curKey = key.value();

				CloseCurrentPopup();
			}
		}
	};
	#pragma endregion

	auto inputKbCtrls = [&](pong::playerid pl)
	{
		auto title = to_string(pl);
		gui::GroupID _grp_ = title;

		ImGui::Text("%s:", title);

		auto& player_ctrls = work_settings.keyboard_keys(pl);

		inputKbKey("Up", player_ctrls.up);
		inputKbKey("Down", player_ctrls.down);
		inputKbKey("Fast", player_ctrls.fast);
	};

	inputKbCtrls(playerid::one);
	ImGui::SameLine();
	inputKbCtrls(playerid::two);

	ImGui::Spacing();

	{
		gui::Font _f_ = fonts[ft::section_title];
		ImGui::Text("Joystick:");
	}

	auto inputJoystickSettings = [&](playerid pid) mutable {
		auto title = to_string(pid);

		gui::GroupID _grp_ = title;
		ImGui::Text(title);

		auto joyid = work_settings.get_joystick(pid);
		auto selected = joystickCombobox(joyid);

		if (selected != joyid) {
			work_settings.set_joystick(pid, selected);
		}

		auto& deadzone = work_settings.joystick_deadzone(pid);
		ImGui::SliderFloat("Deadzone", &deadzone, 0, 50, "%.1f%%");
	};

	inputJoystickSettings(playerid::one);
	ImGui::NewLine();
	inputJoystickSettings(playerid::two);
}

int joystickCombobox(int current_joyid)
{
	using namespace ImGui;
	namespace gui = ImScoped;
	using sf::Joystick;

	const auto npos = game_settings::njoystick;
	const auto nitem = "Nenhum";

	auto previewItem = current_joyid == npos ? nitem : _joystick_names[current_joyid].c_str();

	if (auto cb = gui::Combo("", previewItem))
	{
		bool is_selected = current_joyid == npos;
		if (Selectable(nitem, is_selected)) {
			current_joyid = npos;
		}

		for (int s{}; s < _joystick_count; s++)
		{
			is_selected = s == current_joyid;
			auto& name = _joystick_names[s];

			if (Selectable(name.c_str(), is_selected))
				current_joyid = s;

			if (is_selected)
				SetItemDefaultFocus();
		}
	}

	return current_joyid;
}
