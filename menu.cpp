#include "menu.h"
#include "convert.h"
#include "game.h"
#include "imgui_inc.h"

#include <algorithm>
#include <optional>
#include <vector>
#include <sstream>

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

static auto scan_joy_btn() noexcept -> std::optional<std::tuple<unsigned,unsigned>>
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

	return std::nullopt;
}


namespace
{
	std::vector<std::string> _joystick_names;
	// std::vector<sf::Joystick::Identification> _joysticks;

	void refresh_joysticks()
	{
		using sf::Joystick;
	
		_joystick_names.clear();
	
		for (int i=0; i < Joystick::Count; i++)
		{
			if (Joystick::isConnected(i)) {
				auto info = Joystick::getIdentification(i);
				_joystick_names.push_back(info.name);
			}
		}
	}
}

using namespace pong;

void themenu::init()
{
	work_settings = game.settings;

	if (!ImGui::SFML::Init(game.window, false)) {
		spdlog::error("ImGui::SFML::Init failed!");
		std::terminate();
	}
	
	refresh_joysticks();
	auto* atlas = ImGui::GetIO().Fonts;

	atlas->Clear();
	fonts[font_normal] = atlas->AddFontFromFileTTF(pong::files::sans_tff, font_size);
	fonts[font_larger] = atlas->AddFontFromFileTTF(pong::files::sans_tff, font_size * 2);
	fonts[font_title] = atlas->AddFontFromFileTTF(pong::files::sans_tff, font_size * 1.25f);
	fonts[font_monospace] = atlas->AddFontFromFileTTF(pong::files::mono_tff, font_size);

	if (!ImGui::SFML::UpdateFontTexture()) {
		spdlog::error("ImGui::SFML::UpdateFontTexture failed!");
		std::terminate();
	}
}

themenu::themenu(pong::game& gameref, float fontsize)
	: game(gameref)
	, font_size(fontsize)
{
	visible.fill(false);
}

themenu::~themenu()
{
	ImGui::SFML::Shutdown(game.window);
}

bool themenu::isOpen(menuid mid)
{
    return visible[mid];
}

void themenu::processEvent(sf::Event &event)
{
	using sf::Event;

	switch (event.type)
	{
	case Event::JoystickConnected:
	case Event::JoystickDisconnected:
		refresh_joysticks();
		break;
	case Event::KeyPressed:
		if (event.key.code == sf::Keyboard::F12) {
			visible[ui_game_stats] = !visible[ui_game_stats];
		}
		break;
	}
}


void themenu::update()
{
	using namespace ImGui;
	using namespace ImScoped;

	if (visible[ui_game_stats])
		gameStatsUi();
	
	if (game.paused) {
		// windows
		if (visible[ui_options])
			optionsUi();
		if (visible[ui_about])
			aboutUi();
		if (visible[ui_imgui_demo])
			ImGui::ShowDemoWindow(&visible[ui_imgui_demo]);
		if (visible[ui_imgui_about])
			ImGui::ShowAboutWindow(&visible[ui_imgui_about]);

		// main menu
		if (auto mm = MainMenuBar()) {
			TextDisabled("sfPong");

			if (auto m = Menu("Jogo")) {
				if (MenuItem("Continuar", "ESC"))
					game.paused = false;
				if (auto m1 = Menu("Novo")) {

					if (MenuItem("1 jogador", nullptr, game.mode == gamemode::singleplayer)) {
						game.newGame(gamemode::singleplayer);
					}
					if (MenuItem("2 jogadores", nullptr, game.mode == gamemode::multiplayer)) {
						game.newGame(gamemode::multiplayer);
					}
				}
				if (MenuItem("Reiniciar")) {
					game.reset();
					game.paused = false;
				}
				Separator();
				MenuItem("Sobre", nullptr, &visible[ui_about]);
			}

			MenuItem("Opções", nullptr, &visible[ui_options]);

			if (auto m = Menu("Extra")) {
				MenuItem("Stats", "F12", &visible[ui_game_stats]);
			}

			StyleColor _s_[] = {
				{ImGuiCol_Button, sf::Color::Transparent},
				{ImGuiCol_ButtonHovered, sf::Color::Red},
				{ImGuiCol_ButtonActive, sf::Color(255, 50, 50)}
			};

			if (Button("Sair")) {
				spdlog::info("Tchau! :)");
				game.window.close();
			}
		}
	}
}

void themenu::optionsUi()
{
	namespace gui = ImScoped;

	const bool isDirty = work_settings != game.settings;
	auto wflags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (isDirty) wflags |= ImGuiWindowFlags_UnsavedDocument;

	gui::Window guiwindow("Opções", &visible[ui_options], wflags);
	if (!guiwindow)
		return;

{ // child
	auto guiwindowsize = ImGui::GetWindowSize();
	gui::Child _content_{ "conteudo opções", { guiwindowsize.x - 10, guiwindowsize.y - 100 } };
	
	if (auto tabbar = gui::TabBar("##Tabs"))
	{
		if (auto tab = gui::TabBarItem("Game"))
		{
			auto& vidModes = sf::VideoMode::getFullscreenModes();
			auto curVidMode = sf::VideoMode(work_settings.resolution.x, work_settings.resolution.y);
			auto preview = fmt::format("{} x {}", curVidMode.width, curVidMode.height);

			if (auto cb = gui::Combo("Resolução", preview.c_str())) {
				for (auto& vid : vidModes) {
					preview = fmt::format("{} x {}", vid.width, vid.height);
					auto isSelected = vid == curVidMode;
					if (ImGui::Selectable(preview.c_str(), isSelected)) {
						work_settings.resolution.x = vid.width;
						work_settings.resolution.y = vid.height;
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
				visible[ui_game_stats] = true;
			}
			if (Button("ImGui demo window"))
			{
				visible[ui_imgui_demo] = true;
			}
		}
	}
}

	ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::Button("Descartar")) {
		work_settings = game.settings;
	}
	ImGui::SameLine();
	if (ImGui::Button("Salvar") && isDirty)
	{
		game.settings = work_settings;
		// TODO: resolução
	}
}

void themenu::gameStatsUi()
{
	namespace ims = ImScoped;

	ImGuiIO& io = ImGui::GetIO();
	pos winpos = { io.DisplaySize.x - 10.f, 15.f };
	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(winpos, ImGuiCond_Always, { 1.f, 0 });

	auto const wflags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings 
						| ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ims::Window overlay("Stats", &visible[ui_game_stats], wflags);

	point Pos[] = {
		game.player1.shape.getPosition(),
		game.player2.shape.getPosition(),
		game.ball.shape.getPosition()
	};

	auto text = fmt::format("P1: [{:.2f}]\n" "P2: [{:.2f}]\n" "Ball: [{:.2f}]", Pos[0], Pos[1], Pos[2]);

	ImGui::Text("Positions:\n%s", text.c_str());

	text = fmt::format("P1: {:.3f}\nP2: {:.3f}\nBall: [{:.2f}]", 
		game.player1.velocity, game.player2.velocity, game.ball.velocity);

	ImGui::Text("Velocity:\n%s", text.c_str());
}

void themenu::aboutUi()
{
	using namespace ImGui;
	namespace gui = ImScoped;

	auto window = gui::Window("Sobre sfPong", &visible[ui_about], ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
	if (!window) return;

	Text("sfPong %s", pong::version);
	Text("Criado por Pedro Oliva Rodrigues.");
	Separator();

	gui::Font _f_ = fonts[font_monospace];

	const auto libver = "%-7s %d.%d.%d";
	Text(libver, "SFML", SFML_VERSION_MAJOR, SFML_VERSION_MINOR, SFML_VERSION_PATCH);

{ // imgui
	Text(libver, "ImGui", 
		IMGUI_VERSION_NUM / 10000,
		IMGUI_VERSION_NUM / 100 % 100,
		IMGUI_VERSION_NUM % 100
	);

	SameLine();

	gui::Font _ff_ = fonts[font_normal];
	if (SmallButton(" + "))
		visible[ui_imgui_about] = true;
}

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

void themenu::controlsUi()
{
	namespace gui = ImScoped;

	{
		gui::Font _f_ = fonts[font_title];
		ImGui::Text("Teclado:");
	}

	auto inputKbKey = [&,id = 0](const char* label, sf::Keyboard::Key& curKey) mutable
	{
		using namespace ImGui;

		gui::ID _id_ = id++;
		auto constexpr popup_id = "Rebind popup";
		auto keyname = conv::to_string_view(curKey);

		// botão
		Text("%5s:", label);
		SameLine(75);
		// TODO: Isso assume que keyname tem terminador nulo
		if (Button(keyname.data())) {
			OpenPopup(popup_id);
			visible[ui_rebiding_popup] = true;
		}

		// popup
		const auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav;
		if (auto popup = gui::PopupModal(popup_id, &visible[ui_rebiding_popup], flags))
		{
			using Key = sf::Keyboard::Key;

			gui::Font _f_{ fonts[font_larger] };
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

	auto inputKbCtrls = [&](pong::playerid pl)
	{
		auto title = conv::to_string_view(pl).data();
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
		gui::Font _f_ = fonts[font_title];
		ImGui::Text("Joystick:");
	}

	auto inputJoystickSettings = [&](playerid pid) mutable {
		auto title = conv::to_string_view(pid).data();

		gui::GroupID _grp_ = title;
		ImGui::Text(title);

		auto joyid = work_settings.get_joystick(pid);
		int selected = joyid;

		// joystick combobox
		{
			using sf::Joystick;
			const auto npos = game_settings::njoystick;
			const auto nitem = "Nenhum";

			auto previewItem = joyid == npos ? nitem : _joystick_names[joyid].c_str();

			if (auto cb = gui::Combo("", previewItem))
			{
				bool is_selected = joyid == npos;
				if (ImGui::Selectable(nitem, is_selected)) {
					selected = npos;
				}

				for (int s{}; s < _joystick_names.size(); s++)
				{
					is_selected = s == joyid;
					auto& name = _joystick_names[s];

					if (ImGui::Selectable(name.c_str(), is_selected))
						selected = s;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			}
		}

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
