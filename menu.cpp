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

using namespace pong;

namespace
{
	pong::game_settings work_settings;
	pong::game_settings *settings;

	std::array<std::string, sf::Joystick::Count> _joystick_names;
	int _joystick_count;

	// window ids
namespace win {
	enum Id
	{
		options,
		game_stats,
		about,
		imgui_demo,
		imgui_about,
		rebiding_popup,

		Count
	};
}

	enum fontId 
	{
		font_default_,
		font_default_large,
		font_monospace,
		font_section_title,

		font_Count
	};

	// show flags
	bool isVisible[win::Count] = {};

	//fonts
	ImFont* fonts[font_Count] = {};
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

	int i = 0;
	for (; i < Joystick::Count; i++)
	{
		if (Joystick::isConnected(i)) {
			auto info = Joystick::getIdentification(i);
			_joystick_names[i] = info.name;
		}
	}
	_joystick_count = i;
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

	if (isVisible[options])
		optionsWin(ctx, window);

	if (isVisible[game_stats])
		gameStatsWin(ctx);

	if (isVisible[about])
		aboutSfPongWin();

	if (isVisible[imgui_demo])
		ShowDemoWindow(&isVisible[imgui_demo]);
	if (isVisible[imgui_about])
		ImGui::ShowAboutWindow(&isVisible[imgui_about]);

	MainMenuBar mmb;
	if (!mmb) return;

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

	MenuItem(u8"Op��es", nullptr, &isVisible[options]);

	{
		auto _styles_ = {
			StyleColor(ImGuiCol_Button, sf::Color::Transparent),
			StyleColor(ImGuiCol_ButtonHovered, sf::Color::Red),
			StyleColor(ImGuiCol_ButtonActive, sf::Color(255, 50, 50))
		};

		if (Button("Sair")) {
			log::info("Tchau! :)");
			window.close();
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
	fonts[font_default_] = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size);
	fonts[font_default_large] = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size * 2);
	fonts[font_section_title] = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size * 1.25f);
	fonts[font_monospace] = atlas->AddFontFromFileTTF(pong::files::mono_tff, ui_font_size);
	ImGui::SFML::UpdateFontTexture();
}

void pong::menu::processEvent(sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::JoystickConnected:
	case sf::Event::JoystickDisconnected:
		clear_joysticks();
		refresh_joysticks();
		break;
	}
}

bool pong::menu::rebinding_popup_open() noexcept
{
	return isVisible[win::rebiding_popup];
}


void optionsWin(game&, sf::Window& window)
{
	namespace gui = ImScoped;

	const bool isDirty = work_settings != *settings;
	auto wflags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (isDirty) wflags |= ImGuiWindowFlags_UnsavedDocument;

	gui::Window guiwindow(u8"Op��es", &isVisible[win::options], wflags);
	if (!guiwindow)
		return;

	{
		auto guiwindowsize = ImGui::GetWindowSize();
		gui::Child _content_{ "conteudo op��es", { guiwindowsize.x - 10, guiwindowsize.y - 100 } };

		if (auto tabbar = gui::TabBar("##Tabs"))
		{
			if (auto tab = gui::TabBarItem("Game"))
			{
				auto& vidModes = sf::VideoMode::getFullscreenModes();
				auto curVidMode = sf::VideoMode(work_settings.resolution().x, work_settings.resolution().y);
				auto preview = fmt::format("{} x {}", curVidMode.width, curVidMode.height);

				if (auto cb = gui::Combo(u8"Resolu��o", preview.c_str())) {
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
	using namespace ImScoped;

	ImGuiIO& io = ImGui::GetIO();
	pos winpos = { io.DisplaySize.x - 10.f, 15.f };
	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(winpos, ImGuiCond_FirstUseEver, { 1.f, 0 });

	auto constexpr wflags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
	Window overlay("Stats", &isVisible[win::game_stats], wflags);

	auto players = ctx.get_players();

	auto& P1 = players.first;
	auto& P2 = players.second;
	auto& Ball = ctx.get_ball();

	auto p1_pos = P1.getPosition();
	auto p2_pos = P2.getPosition();
	auto ball_pos = Ball.getPosition();

	auto text = fmt::format("P1: [{:.2f}, {:.2f}]\nP2: [{:.2f}, {:.2f}]\nBall: [{:.2f}, {:.2f}]",
		p1_pos.x, p1_pos.y, p2_pos.x, p2_pos.y, ball_pos.x, ball_pos.y);

	ImGui::Text("Positions:\n%s", text.c_str());


	text = fmt::format("P1: {:.3f}\nP2: {:.3f}\nBall: [{:.3f}, {:.3f}]", 
		P1.velocity, P2.velocity, Ball.velocity.x, Ball.velocity.y);

	ImGui::Text("Velocity:\n%s", text.c_str());
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
	
	Text("%s: %5s", "ImGui", ImGui::GetVersion()); SameLine();
	if (SmallButton("about")) {
		isVisible[win::imgui_about] = true;
	}
	if (IsItemHovered())
		SetTooltip("ImGui about window");

	const auto libver = "%s: %5d.%d.%d";
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
		gui::Font _f_ = fonts[font_section_title];
		ImGui::Text("Teclado:");
	}

	auto inputKbKey = [id = 0](const char* label, sf::Keyboard::Key& curKey) mutable
	#pragma region Block
	{
		using namespace ImGui;

		gui::ID _id_ = id++;
		auto constexpr popup_id = "Rebind popup";
		auto keystr = fmt::format("{}", curKey);

		// but�o
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

			gui::Font _f_{ fonts[font_default_large] };
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
		gui::Font _f_ = fonts[font_section_title];
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

	const auto npos = -1;
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
