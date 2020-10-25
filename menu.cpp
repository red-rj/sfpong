#include "menu.h"
#include "convert.h"
#include "game.h"
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
	ImFont *font_rebinding_popup,
		*font_sect_title,
		*font_default,
		*font_monospace
		;

	std::vector<std::string> _joystick_list;

	constexpr auto nameof(pong::playerid pl)
	{
		auto* title = "Player ???";
		switch (pl)
		{
		case pong::playerid::one: title = "Player 1"; break;
		case pong::playerid::two: title = "Player 2"; break;
		}
		return title;
	}
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

void pong::menu_t::init()
{
	input_settings[(int)playerid::one] = get_input_cfg(playerid::one);
	input_settings[(int)playerid::two] = get_input_cfg(playerid::two);
	refresh_joystick_list();

	auto* atlas = ImGui::GetIO().Fonts;
	const auto ui_font_size = 18.f;

	atlas->Clear();
	font_default = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size);
	font_rebinding_popup = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size * 2);
	font_sect_title = atlas->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size * 1.25f);
	font_monospace = atlas->AddFontFromFileTTF(pong::files::mono_tff, ui_font_size);

	ImGui::SFML::UpdateFontTexture();
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

	bool isDirty = input_settings != active_settings;
	auto wflags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (isDirty) wflags |= ImGuiWindowFlags_UnsavedDocument;

	gui::Window guiwindow(u8"Opções", &show.options, wflags);
	if (!guiwindow)
		return;

	{
		auto guiwindowsize = ImGui::GetWindowSize();
		gui::Child _content_{ "conteudo opções", { guiwindowsize.x - 10, guiwindowsize.y - 100 } };

		if (auto tabbar = gui::TabBar("##Tabs"))
		{
			if (auto tab = gui::TabBarItem("Controles"))
			{
				controlsUi();
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
	}

	ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::Button("Descartar")) {
		input_settings = active_settings;
	}
	ImGui::SameLine();
	if (ImGui::Button("Salvar") && isDirty)
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

	auto win = gui::Window("Sobre sfPong", &show.about, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
	if (!win) return;

	Text("sfPong %s", pong::version);
	Text("Criado por Pedro Oliva Rodrigues.");
	Separator();
	
	Text("%s: %5s", "ImGui", ImGui::GetVersion()); SameLine();
	if (SmallButton("about")) {
		show.imgui_about = true;
	}
	if (IsItemHovered())
		SetTooltip("ImGui about window");

	constexpr auto libver = "%s: %5d.%d.%d";
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

void pong::menu_t::controlsUi()
{
	namespace gui = ImScoped;
	{
		gui::Font title = font_sect_title;
		ImGui::Text("Teclado:");
	}

	auto inputKbKey = [id = 0, this](const char* label, sf::Keyboard::Key& curKey) mutable
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

			gui::Font sansBig{ font_rebinding_popup };
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

	auto inputKbCtrls = [&](pong::playerid pl) mutable
	{
		auto title = nameof(pl);
		gui::ID _id_ = title;
		gui::Group _g_;

		ImGui::Text("%s:", title);
		//gui::Indent _ind_{ 5.f };

		auto& settings = input_settings[int(pl)];
		auto& player_ctrls = settings.keyboard_controls;

		inputKbKey("Up", player_ctrls.up);
		inputKbKey("Down", player_ctrls.down);
		inputKbKey("Fast", player_ctrls.fast);
	};

	// ---
	inputKbCtrls(playerid::one);
	ImGui::SameLine();
	inputKbCtrls(playerid::two);

	ImGui::Spacing();

	{
		gui::Font title = font_sect_title;
		ImGui::Text("Joystick:");
	}

	auto inputJoystickSettings = [&](playerid pid) mutable {
		const auto other_pid = pid == playerid::one ? playerid::two : playerid::one;

		auto& joyid = input_settings[int(pid)].joystickId;
		auto& deadzone = input_settings[int(pid)].joystick_deadzone;
		auto title = nameof(pid);

		gui::GroupID _grp_ = title;
		ImGui::Text(title);
		//gui::ItemWidth _iw_ = ImGui::GetWindowWidth() * 0.5f;

		auto selected = joystickCombobox("", joyid);

		if (selected != -1) {
			auto& other_joyid = input_settings[int(other_pid)].joystickId;

			if (selected == other_joyid) {
				std::swap(joyid, other_joyid);
			}
		}

		joyid = selected;

		ImGui::SliderFloat("Deadzone", &deadzone, 0, 50, "%.1f%%");
	};

	inputJoystickSettings(playerid::one);
	ImGui::NewLine();
	inputJoystickSettings(playerid::two);
}

int pong::menu_t::joystickCombobox(const char* label, int current_joyid)
{
	using namespace ImGui;
	namespace gui = ImScoped;
	using sf::Joystick;

	const auto npos = -1;
	const auto nitem = "Nenhum";

	auto& joynames = _joystick_list;
	auto previewItem = current_joyid == npos ? nitem : joynames[current_joyid];

	if (auto cb = gui::Combo(label, previewItem.c_str()))
	{
		bool is_selected = current_joyid == npos;
		if (Selectable(nitem, is_selected)) {
			current_joyid = npos;
		}

		auto s = 0u;
		for (auto& name : joynames)
		{
			is_selected = s == current_joyid;
			if (Selectable(name.c_str(), is_selected))
				current_joyid = s;

			if (is_selected)
				SetItemDefaultFocus();

			s++;
		}
	}

	return current_joyid;
}
