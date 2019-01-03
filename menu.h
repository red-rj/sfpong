#pragma once

#include <vector>
#include <string>

#include "SFML/Graphics.hpp"

namespace red::pong
{
    class menu : public sf::Drawable, public sf::Transformable
    {
    public:
        menu(float width, float height, std::initializer_list<sf::String> texts_ = {})
            : menu({width, height}, texts_) {}

        explicit menu(const sf::Vector2f& size, std::initializer_list<sf::String> texts_ = {});

        void moveUp();
        void moveDown();

        void select(int idx);
        int selected() const noexcept { return selectedIndex; }
        void deselect();

        sf::Color const& getColor() const noexcept { return color_; }
        void setColor(sf::Color const& c);

        sf::Color const& getSelectedColor() const noexcept { return selectedColor_; }
        void setSelectedColor(sf::Color const& c);

        sf::Font const& getFont() const noexcept { return font_; }
        void setFont(sf::Font const& value);

        sf::Vector2f const& getSize() const noexcept { return size_; }
        void setSize(sf::Vector2f const& value);

        auto begin() { return texts.begin(); }
        auto end() { return texts.end(); }

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        void reformat() noexcept;

        int selectedIndex = 0;
        sf::Color color_ = sf::Color::White, selectedColor_ = sf::Color::Red;
        sf::Font font_;
        sf::Vector2f size_;
        std::vector<sf::Text> texts;
    };
}
