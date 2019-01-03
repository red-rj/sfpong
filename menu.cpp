#include "menu.h"

red::pong::menu::menu(const sf::Vector2f & size, std::initializer_list<sf::String> texts_) : size_(size)
{
    for (auto& i : texts_)
    {
        texts.emplace_back(i, font_);
    }
}


void red::pong::menu::moveUp()
{
    if (selectedIndex - 1 >= 0)
    {
        select(selectedIndex - 1);
    }
    else
    {
        select((int)texts.size() - 1);
    }
}

void red::pong::menu::moveDown()
{
    if (selectedIndex + 1 < texts.size())
    {
        select(selectedIndex + 1);
    }
    else
    {
        select(0);
    }
}

void red::pong::menu::select(int idx)
{
    int old = selectedIndex != -1 ? selectedIndex : 0;
    texts.at(old).setFillColor(color_);
    selectedIndex = idx;
    texts.at(selectedIndex).setFillColor(selectedColor_);
}

void red::pong::menu::deselect()
{
    if (selectedIndex != -1)
        texts.at(selectedIndex).setFillColor(color_);
    selectedIndex = -1;
}

void red::pong::menu::setColor(sf::Color const & c)
{
    if (c != color_) {
        color_ = c;
        reformat();
    }
}

void red::pong::menu::setSelectedColor(sf::Color const & c)
{
    if (c != selectedColor_) {
        selectedColor_ = c;
        reformat();
    }
}

void red::pong::menu::setFont(sf::Font const & value)
{
    font_ = value;
    reformat();
}

void red::pong::menu::setSize(sf::Vector2f const & value)
{
    if (value != size_) {
        size_ = value;
        reformat();
    }
}

void red::pong::menu::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    for (auto& t : texts)
    {
        target.draw(t, states);
    }
}

void red::pong::menu::reformat() noexcept
{
    if (texts.empty()) return;
    
    for (int i = 0; i < texts.size(); i++)
    {
        texts[i].setFont(font_);
        if (i == selectedIndex) {
            texts[i].setFillColor(selectedColor_);
        } else {
            texts[i].setFillColor(color_);
        }
        texts[i].setPosition(size_.x / 2, size_.y / texts.size() * (i + 1));
    }
}
