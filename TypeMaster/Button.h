#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include <algorithm>
#include "Widget.h"
#include "Theme.h"
using namespace std;

class Button : public Widget
{
private:
    sf::Text label;
    bool primary;   // main action, always filled with the accent colour
    bool active;    // used by the mode buttons to show the selected mode
    float hover;    // 0 = normal, 1 = mouse is on it (changes smoothly)

public:
    Button(const sf::Font& font, string text, sf::Vector2f position,
           sf::Vector2f size, bool isPrimary = false)
        : Widget(position, size, 12), label(font, text, 16)
    {
        primary = isPrimary;
        active = false;
        hover = 0;

        placeText(label, position + size / 2.f);
    }

    void setActive(bool value)
    {
        active = value;
    }

    void update(sf::Vector2f mouse, float deltaTime) override
    {
        float target = contains(mouse) ? 1.f : 0.f;
        hover += (target - hover) * min(1.f, deltaTime * 12);
    }

    void draw(sf::RenderWindow& window) override
    {
        sf::Vector2f size = box.getSize();
        sf::Vector2f center = box.getPosition() + size / 2.f;

        if(primary || active)
        {
            Theme::drawGlow(window, center, {size.x * 0.8f, size.y * 1.3f},
                            Theme::fade(Theme::Accent, 0.30f + 0.35f * hover));

            box.setFillColor(Theme::mix(Theme::Accent, Theme::AccentLight, hover * 0.6f));
            box.setOutlineThickness(0);
            label.setFillColor(sf::Color::White);
        }
        else
        {
            box.setFillColor(Theme::mix(Theme::Surface, Theme::SurfaceHover, hover));
            box.setOutlineThickness(1);
            box.setOutlineColor(Theme::mix(Theme::Border, Theme::AccentLight, hover));
            label.setFillColor(Theme::mix(Theme::TextMuted, Theme::TextBright, hover));
        }

        window.draw(box);
        window.draw(label);
    }
};

#endif
