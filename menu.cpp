#include "menu.h"
#include "convert.h"
#include "game.h"
#include "input.h"
#include "imgui_ext.h"

#include <algorithm>
#include <optional>
#include <tuple>
#include <vector>

#include <imgui-SFML.h>
#include <fmt/ostream.h>
#include <SFML/Window/Window.hpp>
#include <boost/range/algorithm.hpp>


static auto scan_kb() noexcept -> std::optional<sf::Keyboard::Key>
{
	using sf::Keyboard;
	
	auto& io = ImGui::GetIO();
	auto const begin = io.KeysDown;
	auto const end = begin + Keyboard::KeyCount;
	auto it = std::find(begin, end, true);

	if (it != end) {
		return Keyboard::Key(it - begin);
	}
	else return {};
}

static auto scan_mouse_btn() noexcept -> std::optional<sf::Mouse::Button>
{
	using sf::Mouse;
	constexpr auto Count = Mouse::ButtonCount;

	auto& io = ImGui::GetIO();
	auto const begin = io.MouseDown;
	auto const end = begin + Count;
	auto it = std::find(begin, end, true);

	if (it != end) {
		return Mouse::Button(it - begin);
	}
	else return {};
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


namespace
{
	ImFont* ui_default_font, *ui_bigger_font;

	std::vector<std::string> _joystick_list;
}

pong::menu_t pong::game_menu;

void pong::menu_t::draw(game& ctx, sf::Window& window)
{
	using namespace ImGui;
	using namespace ImScoped;

	if (!ctx.paused) return;

	// update

	// draw
	if (show.options)
		guiOptions(ctx);

	if (show.game_stats)
		guiStats(ctx);

	if (show.about)
		aboutSfPong();

	if (show.imgui_demo)
		ShowDemoWindow(&show.imgui_demo);
	if (show.imgui_about)
		ImGui::ShowAboutWindow(&show.imgui_about);

	MainMenuBar mmb;
	if (!mmb) return;
	TextDisabled("sfPong");

	if (auto m = Menu("Jogo")) {
		bool singleplayer = ctx.currentMode == game::mode::singleplayer,
			 multiplayer = ctx.currentMode == game::mode::multiplayer;

		if (MenuItem("Continuar", "ESC"))
			ctx.paused = false;
		if (auto m1 = Menu("Novo")) {
			auto size = static_cast<size2d>(window.getSize());

			if (MenuItem("1 jogador", nullptr, false, !singleplayer)) {
				ctx = game(size, game::mode::singleplayer);
				ctx.paused = false;
			}
			if (MenuItem("2 jogadores", nullptr, false, !multiplayer)) {
				ctx = game(size, game::mode::multiplayer);
				ctx.paused = false;
			}
		}
		Separator();
		MenuItem("Sobre", nullptr, &show.about);
	}

	MenuItem(u8"Opções", nullptr, &show.options);

	{
		auto _s_ = {
			StyleColor(ImGuiCol_Button, sf::Color::Transparent),
			StyleColor(ImGuiCol_ButtonHovered, sf::Color::Red),
			StyleColor(ImGuiCol_ButtonActive, sf::Color::Red)
		};

		if (Button("Sair")) {
			log::info("Tchau! :)");
			window.close();
		}
	}
}

void pong::menu_t::init()
{
	input_settings[(int)playerid::one] = get_input_cfg(playerid::one);
	input_settings[(int)playerid::two] = get_input_cfg(playerid::two);
	refresh_joystick_list();

	auto* fontAtlas = ImGui::GetIO().Fonts;
	ui_default_font = fontAtlas->Fonts.front();
	ui_bigger_font = fontAtlas->Fonts[1];
}

void pong::menu_t::refresh_joystick_list() const
{
	using sf::Joystick;
	_joystick_list.clear();
	_joystick_list.reserve(Joystick::Count);

	for (unsigned id = 0; id < Joystick::Count; id++)
	{
		if (Joystick::isConnected(id)) {
			auto info = Joystick::getIdentification(id);
			_joystick_list.emplace_back(info.name);
		}
	}
}

void pong::menu_t::guiOptions(game&)
{
	namespace gui = ImScoped;
	std::array<player_input_cfg, 2> active_settings = {
		get_input_cfg(playerid::one),
		get_input_cfg(playerid::two)
	};

	auto wflags = input_settings != active_settings ? ImGuiWindowFlags_UnsavedDocument : 0;
	gui::Window guiwindow(u8"Opções", &show.options, wflags);
	if (!guiwindow)
		return;

	if (auto tabbar = gui::TabBar("##Tabs"))
	{
		if (auto tab = gui::TabBarItem("Controles"))
		{

			auto InputControl = [id = 0, this](const char* label, sf::Keyboard::Key& curKey) mutable
			#pragma region Block
			{
				using namespace ImGui;

				gui::ID _id_ = id++;
				auto constexpr popup_id = "Rebind popup";
				auto keystr = fmt::format("{}", curKey);

				Text("%5s:", label);
				SameLine(75);
				if (Button(keystr.c_str())) {
					OpenPopup(popup_id);
					rebinding = true;
				}

				const auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav;
				if (auto popup = gui::PopupModal(popup_id, &rebinding, flags))
				{
					using Key = sf::Keyboard::Key;

					gui::Font sansBig{ ui_bigger_font };
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

			auto InputPlayerCtrls = [&](pong::playerid pl) mutable
			{
				gui::ID _id_ = int(pl);
				auto* title = "Player ???";
				switch (pl)
				{
				case pong::playerid::one: title = "Player 1"; break;
				case pong::playerid::two: title = "Player 2"; break;
				}

				ImGui::Text("%s:", title);
				gui::Indent _ind_{ 5.f };

				auto& settings = input_settings[int(pl)];
				auto& player_ctrls = settings.keyboard_controls;

				InputControl("Up", player_ctrls.up);
				InputControl("Down", player_ctrls.down);
				InputControl("Fast", player_ctrls.fast);

				joystickCombobox(settings.joystickId);

				ImGui::SliderFloat("Joystick deadzone", &settings.joystick_deadzone, 0, 50, "%.1f%%");
			};

			auto rollback_if_eq = [&](playerid lhs, playerid rhs) mutable {
				if (input_settings[int(lhs)].joystickId != -1 
					&& input_settings[int(lhs)].joystickId == input_settings[int(rhs)].joystickId)
				{
					input_settings[int(rhs)].joystickId = -1;
				}
			};

			// ---
			InputPlayerCtrls(playerid::one);
			rollback_if_eq(playerid::one, playerid::two);
			
			InputPlayerCtrls(playerid::two);
			rollback_if_eq(playerid::two, playerid::one);
		}
		if (auto tab = gui::TabBarItem("DEV"))
		{
			using ImGui::Button;

			if (Button("game stats"))
			{
				show.game_stats = true;
			}
			if (Button("ImGui demo window"))
			{
				show.imgui_demo = true;
			}
		}
	}

	ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::Button("Descartar")) {
		input_settings = active_settings;
	}
	ImGui::SameLine();
	if (ImGui::Button("Salvar") && (wflags & ImGuiWindowFlags_UnsavedDocument) != 0)
	{
		for (auto player : { playerid::one, playerid::two })
		{
			set_input_cfg(input_settings[int(player)], player);
		}
	}
}

void pong::menu_t::guiStats(game& ctx)
{
	using namespace ImScoped;

	ImGuiIO& io = ImGui::GetIO();
	pos winpos = { io.DisplaySize.x - 10.f, 15.f };
	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(winpos, ImGuiCond_FirstUseEver, { 1.f, 0 });

	auto constexpr wflags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
	Window overlay("Stats", &show.game_stats, wflags);

	auto const& P1 = ctx.Player1;
	auto const& P2 = ctx.Player2;
	auto const& Ball = ctx.Ball;

	auto p1_pos = P1.getPosition();
	auto p2_pos = P2.getPosition();
	auto ball_pos = Ball.getPosition();

	auto text = fmt::format("P1: [{:.2f}, {:.2f}]\nP2: [{:.2f}, {:.2f}]\nBall: [{:.2f}, {:.2f}]",
		p1_pos.x, p1_pos.y, p2_pos.x, p2_pos.y, ball_pos.x, ball_pos.y);

	ImGui::Text("Positions:\n%s", text.c_str());


	text = fmt::format("P1: {:.3f}\nP2: {:.3f}\nBall: [{:.3f}, {:.3f}]", 
		P1.velocity.y, P2.velocity.y, Ball.velocity.x, Ball.velocity.y);

	ImGui::Text("Velocity:\n%s", text.c_str());
}

void pong::menu_t::aboutSfPong()
{
	using namespace ImGui;
	namespace gui = ImScoped;

	auto win = gui::Window("Sobre sfPong", &show.about, ImGuiWindowFlags_AlwaysAutoResize);
	if (!win) return;

	Text("sfPong %s", "0.0.0");
	Text("Criado por Pedro Oliva Rodrigues.");
	Separator();
	
	Text("%10s %s", "ImGui", ImGui::GetVersion()); SameLine();
	if (SmallButton("about")) {
		show.imgui_about = true;
	}
	if (IsItemHovered())
		SetTooltip("ImGui about window");

	constexpr auto libver = "%10s: %d.%d.%d";
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

void pong::menu_t::joystickCombobox(int& joyid)
{
	using namespace ImGui;
	namespace gui = ImScoped;
	using sf::Joystick;

	const auto npos = -1;
	const auto nitem = "Nenhum";

	auto& joynames = _joystick_list;
	auto previewItem = joyid == npos ? nitem : joynames[joyid];

	if (auto cb = gui::Combo("Joystick", previewItem.c_str()))
	{
		if (Selectable(nitem, joyid==npos)) {
			joyid = npos;
		}

		auto s = 0u;
		for (auto& name : joynames)
		{
			bool is_selected = s == joyid;
			if (Selectable(name.c_str(), is_selected))
				joyid = s;

			if (is_selected)
				SetItemDefaultFocus();

			s++;
		}
	}
}
