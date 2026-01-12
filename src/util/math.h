#pragma once

struct float2 {
    float x = 0.f;
    float y = 0.f;

    float2 operator+(const float2 &other) const { return {x + other.x, y + other.y}; }
    float2 operator-(const float2 &other) const { return {x - other.x, y - other.y}; }
    float2 operator*(const float scalar) const { return {x * scalar, y * scalar}; }

    float2 &operator+=(const float2 &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    float2 &operator-=(const float2 &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    static float2 min(const float2 &a, const float2 &b) {
        return {std::min(a.x, b.x), std::min(a.y, b.y)};
    }

    static float2 max(const float2 &a, const float2 &b) {
        return {std::max(a.x, b.x), std::max(a.y, b.y)};
    }
};
