#pragma once

struct Point2D
{
    int x = 0;
    int y = 0;
};

struct Vector2D
{
    Point2D pos;
    Point2D size;
    static constexpr bool contains(const Vector2D& vec, Point2D pos)
    {
        return (vec.pos.x <= pos.x
            && vec.pos.y <= pos.y
            && vec.size.x >= pos.x
            && vec.size.y >= pos.y);
    }
};

enum class Direction : unsigned char {
    Left,
    Up,
    Right,
    Down
};

inline constexpr const char* direction_to_str(Direction dir)
{
    switch (dir) {
    case Direction::Left:
        return "left";
    case Direction::Up:
        return "up";
    case Direction::Right:
        return "right";
    case Direction::Down:
        return "down";
    }
}