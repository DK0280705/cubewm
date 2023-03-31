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
};

enum class direction : unsigned char {
    left,
    up,
    right,
    down
};