#ifndef ROUNDEDRECT_H
#define ROUNDEDRECT_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

// SFML has no rounded rectangle, so we make one by inheriting sf::Shape.
// sf::Shape only needs to know how many points the shape has and where
// each point is. Both are virtual functions, so we override them.
class RoundedRect : public sf::Shape
{
private:
    sf::Vector2f size;
    float radius;

    static const int pointsPerCorner = 10;

public:
    RoundedRect(sf::Vector2f rectSize = {0, 0}, float cornerRadius = 0)
    {
        size = rectSize;
        radius = cornerRadius;
        update();   // sf::Shape rebuilds itself using our points
    }

    void setSize(sf::Vector2f rectSize)
    {
        size = rectSize;
        update();
    }

    sf::Vector2f getSize() const
    {
        return size;
    }

    std::size_t getPointCount() const override
    {
        return pointsPerCorner * 4;
    }

    sf::Vector2f getPoint(std::size_t index) const override
    {
        // radius can not be bigger than half of the smaller side
        float r = std::min(radius, std::min(size.x, size.y) / 2);

        // corner 0 = top right, 1 = bottom right, 2 = bottom left, 3 = top left
        int corner = index / pointsPerCorner;
        int step = index % pointsPerCorner;

        float degrees = corner * 90 - 90 + step * 90.0f / (pointsPerCorner - 1);
        float angle = degrees * 3.14159265f / 180;

        sf::Vector2f center;
        center.x = (corner == 0 || corner == 1) ? size.x - r : r;
        center.y = (corner == 1 || corner == 2) ? size.y - r : r;

        return center + sf::Vector2f(std::cos(angle) * r, std::sin(angle) * r);
    }
};

#endif
