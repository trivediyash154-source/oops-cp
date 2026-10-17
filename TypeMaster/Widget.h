#ifndef WIDGET_H
#define WIDGET_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include "RoundedRect.h"

// Base class for every box on the screen (buttons and stat cards).
// It is an abstract class because draw() is a pure virtual function.
class Widget
{
protected:
    RoundedRect box;

public:
    Widget(sf::Vector2f position, sf::Vector2f size, float radius)
        : box(size, radius)
    {
        box.setPosition(position);
    }

    bool contains(sf::Vector2f point)
    {
        return box.getGlobalBounds().contains(point);
    }

    // Called every frame. Widgets that animate override it.
    virtual void update(sf::Vector2f mouse, float deltaTime)
    {
    }

    virtual void draw(sf::RenderWindow& window) = 0;

    virtual ~Widget() {}

    // Puts a text at 'point', centred vertically on point.y.
    // alignX: 0 = text starts at point.x, 0.5 = centred, 1 = text ends at point.x
    static void placeText(sf::Text& text, sf::Vector2f point, float alignX = 0.5f)
    {
        sf::FloatRect bounds = text.getLocalBounds();

        // use the height of a capital letter, so words with and without
        // letters like 'g' or 'p' are lined up the same way
        sf::FloatRect capital =
            text.getFont().getGlyph('H', text.getCharacterSize(), false).bounds;

        float x = bounds.position.x + bounds.size.x * alignX;
        float y = text.getCharacterSize() + capital.position.y + capital.size.y / 2;

        text.setOrigin({std::round(x), std::round(y)});
        text.setPosition({std::round(point.x), std::round(point.y)});
    }
};

#endif
