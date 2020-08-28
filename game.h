#pragma once
#include <utility>
#include <SFML/Graphics.hpp>
#include "game_config.h"

namespace pong
{
	// position type
	using pos = sf::Vector2f;
	// velocity type
	using vel = sf::Vector2f;

	using bounds_t = sf::FloatRect;


	// game entities
	struct net_shape : public sf::Drawable, public sf::Transformable
	{
        explicit net_shape(float pieceSize_ = 20.f, int pieceCount_ = 25);

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform *= getTransform();
			target.draw(m_net, states);
		}

		
		float m_piece_size;
		int m_piece_count;
		sf::VertexArray m_net{ sf::Quads };
	};
	
	struct score : public sf::Drawable
	{
		score() = default;

		score(bounds_t playarea, std::filesystem::path const& fontfile, unsigned size)
		{
			create(playarea, fontfile, size);
		}

		void create(bounds_t playarea, std::filesystem::path const& fontfile, unsigned size)
		{
			font.loadFromFile(fontfile.string());
			text = sf::Text("", font, size);
			text.setPosition(playarea.width / 2 - 100, 50);
			update();
		}

		void update() {
			text.setString(fmt::format("{}    {}", val.first, val.second));
		}

		// increment score
		void add(short p1, short p2) {
			val.first += p1;
			val.second += p2;
			update();
		}

		void set(std::pair<short, short> newscore) noexcept { val = newscore; }
		constexpr auto& get() const { return val; }


	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(text, states);
		}

		std::pair<short, short> val = {0,0};
		sf::Text text;
		sf::Font font;
	};

	struct court : public sf::Drawable
	{
		court() = default;

		court(bounds_t playarea, float heigth, sf::Vector2f margin)
		{
			top = bottom = sf::RectangleShape({ playarea.width - margin.x * 2, heigth });
			top.setPosition(margin);
			bottom.setOrigin(0, heigth);
			bottom.setPosition(margin + sf::Vector2f(0, playarea.height - margin.y * 2));
			net.setPosition(playarea.width / 2, 20);

			m_center = playarea;
			auto H = margin.y + heigth;
			m_center.top += H;
			m_center.height -= H;
		}

		auto getCenter() const noexcept { return m_center; }

		sf::RectangleShape top, bottom;
		net_shape net;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(top, states);
			target.draw(bottom, states);
			target.draw(net, states);
		}

		bounds_t m_center;
	};

	
	struct game;


	template<typename T>
	using pair_of = std::pair<T,T>;

	struct physics
	{
		physics(const pos& position, const vel& velocity)
			: m_position(position), m_velocity(velocity) {}

		void update();
		void update(vel const& accel);

		auto& position() const { return m_position; }
		auto& velocity() const { return m_velocity; }

	private:
		pong::pos m_position;
		pong::vel m_velocity;
	};

	struct controllable_physics
	{
		controllable_physics(const physics& phy, vel acc)
			: m_phy(phy), m_accel(acc)
		{}

		void update();

		void move(float amount) {
			m_accel.y = amount;
		}

		auto& position() const { return m_phy.position(); }
		auto& velocity() const { return m_phy.velocity(); }
		auto& acceleration() const { return m_accel; }

	private:
		physics m_phy;
		vel m_accel;
	};


	struct paddle : sf::RectangleShape
	{
		bool ai = false;
		int id = -1;
		const config_t* pcfg = nullptr;
		vel velocity;

		void update();
		void update(const bounds_t& ball_bounds);
	};

	struct ball : sf::CircleShape
	{
		explicit ball(float radius = 0) : CircleShape(radius)
		{
			setOrigin(radius, radius);
			setFillColor(sf::Color::Red);
		}

		void update();
		void update(const paddle& player);

		const config_t* pcfg = nullptr;
		vel velocity;
	};


    bool check_collision(const sf::Shape* a, const sf::Shape* b);
	
    bool check_collision(const sf::Shape* a, const court* court);


	struct menu_state
	{
		void draw(game* ctx);

		// options
		bool show_options = false, rebinding = false;
		config_t config;
		bool show_stats = false;

	private:
		void guiOptions(game* ctx);
		void guiStats(game* ctx);
	};
	

	enum class dir { left, right };


	struct game
	{
		friend menu_state;

		game(config_t cfg);

		void run();
		void serve(dir direction);
		//void update();

		void swap(game& other) noexcept;

	private:

		void resetState();
		void pollEvents();

		bool paused = true;
		sf::FloatRect playable_area;
		uint64_t tickcount = 0;

		sf::RenderWindow Window;

		score Score;
		court Court;

		config_t Config;
		paddle Player1, Player2;
		ball Ball;

		// menu
		sf::Event lastEvent;
	};

}
