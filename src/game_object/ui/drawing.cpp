#include "drawing.h"

// Draw a filled rounded rectangle
void DrawRoundedRectangle(float x, float y, float width, float height, float radius, Color color)
{
    // Clamp radius to half the smallest dimension
    radius = std::min(radius, std::min(width, height) / 2.0f);

    if (radius <= 0) {
        DrawRectangle(x, y, width, height, color);
        return;
    }

    // Draw the main rectangles (cross shape)
    DrawRectangle(x + radius, y, width - 2 * radius, height, color);  // Horizontal
    DrawRectangle(x, y + radius, radius, height - 2 * radius, color); // Left vertical
    DrawRectangle(x + width - radius, y + radius, radius, height - 2 * radius, color); // Right vertical

    // Draw the four corner circles
    DrawCircle(x + radius, y + radius, radius, color);                          // Top-left
    DrawCircle(x + width - radius, y + radius, radius, color);                  // Top-right
    DrawCircle(x + radius, y + height - radius, radius, color);                 // Bottom-left
    DrawCircle(x + width - radius, y + height - radius, radius, color);         // Bottom-right
}

// Draw a rounded rectangle outline/stroke
void DrawRoundedRectangleLines(float x, float y, float width, float height, float radius, float strokeWidth, Color color)
{
    // Clamp radius to half the smallest dimension
    radius = std::min(radius, std::min(width, height) / 2.0f);

    if (radius <= 0) {
        DrawRectangleLinesEx({x, y, width, height}, strokeWidth, color);
        return;
    }

    // Draw the four straight edges
    DrawRectangle(x + radius, y, width - 2 * radius, strokeWidth, color);  // Top
    DrawRectangle(x + radius, y + height - strokeWidth, width - 2 * radius, strokeWidth, color); // Bottom
    DrawRectangle(x, y + radius, strokeWidth, height - 2 * radius, color); // Left
    DrawRectangle(x + width - strokeWidth, y + radius, strokeWidth, height - 2 * radius, color); // Right

    // Draw the four corner arcs
    DrawRing({x + radius, y + radius}, radius - strokeWidth, radius, 180, 270, 32, color); // Top-left
    DrawRing({x + width - radius, y + radius}, radius - strokeWidth, radius, 270, 360, 32, color); // Top-right
    DrawRing({x + radius, y + height - radius}, radius - strokeWidth, radius, 90, 180, 32, color); // Bottom-left
    DrawRing({x + width - radius, y + height - radius}, radius - strokeWidth, radius, 0, 90, 32, color); // Bottom-right
}
