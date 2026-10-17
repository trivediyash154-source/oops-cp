#ifndef STATCARD_H
#define STATCARD_H

#include <string>
#include "Widget.h"
#include "Theme.h"
using namespace std;

// A small card that shows one live number, for example "WPM 42.5"
class StatCard : public Widget
{
private:
    sf::Text title;
    sf::Text value;
    sf::Color accent;

public:
    StatCard(const sf::Font& titleFont, const sf::Font& valueFont, string name,
             sf::Color color, sf::Vector2f position, sf::Vector2f size)
        : Widget(position, size, 16), title(titleFont, name, 12), value(valueFont, "0", 30)
    {
        accent = color;

        box.setFillColor(Theme::Surface);
        box.setOutlineThickness(1);
        box.setOutlineColor(Theme::Border);

        title.setLetterSpacing(1.8f);
        title.setFillColor(Theme::TextMuted);
        placeText(title, position + sf::Vector2f(34, 26), 0);
        setValue("0");
    }

    void setValue(string text)
    {
        value.setString(text);
        placeText(value, box.getPosition() + sf::Vector2f(20, 62), 0);
    }

    void setAccent(sf::Color color)
    {
        accent = color;
    }

    void draw(sf::RenderWindow& window) override
    {
        sf::Vector2f position = box.getPosition();
        sf::Vector2f size = box.getSize();

        window.draw(box);

        // soft coloured light inside the card and a dot next to the title
        Theme::drawGlow(window, position + size / 2.f, size / 2.f - sf::Vector2f(2, 2),
                        Theme::fade(accent, 0.13f));

        sf::CircleShape dot(4);
        dot.setFillColor(accent);
        dot.setPosition(position + sf::Vector2f(18, 22));
        window.draw(dot);

        value.setFillColor(accent);
        window.draw(title);
        window.draw(value);
    }
};

#endif
