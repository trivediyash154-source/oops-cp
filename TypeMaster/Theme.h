#ifndef THEME_H
#define THEME_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdint>

// All colours of the app are kept in one place,
// so the whole look can be changed from this file.
namespace Theme
{
    const sf::Color BackgroundTop(11, 13, 32);
    const sf::Color BackgroundBottom(28, 15, 52);

    const sf::Color Surface(255, 255, 255, 12);       // glass cards
    const sf::Color SurfaceHover(255, 255, 255, 28);
    const sf::Color Border(255, 255, 255, 26);

    const sf::Color Accent(124, 77, 255);              // violet
    const sf::Color AccentLight(167, 139, 250);
    const sf::Color Cyan(34, 211, 238);
    const sf::Color Green(52, 211, 153);
    const sf::Color Red(251, 113, 133);
    const sf::Color Amber(251, 191, 36);
    const sf::Color Pink(236, 72, 153);

    const sf::Color TextBright(241, 245, 249);
    const sf::Color TextMuted(148, 163, 184);
    const sf::Color TextDim(96, 104, 142);             // passage not typed yet

    // Returns a colour between a and b (t = 0 gives a, t = 1 gives b)
    inline sf::Color mix(sf::Color a, sf::Color b, float t)
    {
        return sf::Color(
            static_cast<std::uint8_t>(a.r + (b.r - a.r) * t),
            static_cast<std::uint8_t>(a.g + (b.g - a.g) * t),
            static_cast<std::uint8_t>(a.b + (b.b - a.b) * t),
            static_cast<std::uint8_t>(a.a + (b.a - a.a) * t)
        );
    }

    // Same colour, but more transparent (amount 0 = invisible, 1 = unchanged)
    inline sf::Color fade(sf::Color color, float amount)
    {
        color.a = static_cast<std::uint8_t>(color.a * amount);
        return color;
    }

    // Soft light: full colour in the middle, fully transparent at the edge
    inline void drawGlow(sf::RenderTarget& target, sf::Vector2f center,
                         sf::Vector2f radius, sf::Color color)
    {
        const int points = 40;
        sf::VertexArray glow(sf::PrimitiveType::TriangleFan, points + 2);

        glow[0].position = center;
        glow[0].color = color;

        for(int i = 1; i < points + 2; i++)
        {
            float angle = (i - 1) * 2 * 3.14159265f / points;

            glow[i].position = center + sf::Vector2f(std::cos(angle) * radius.x,
                                                     std::sin(angle) * radius.y);
            glow[i].color = sf::Color(color.r, color.g, color.b, 0);
        }

        target.draw(glow, sf::BlendAdd);
    }
}

#endif
