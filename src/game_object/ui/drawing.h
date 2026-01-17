#pragma once

struct Color;

// Draw a filled rounded rectangle
void DrawRoundedRectangle(float x, float y, float width, float height, float radius, const Color &color);

// Draw a rounded rectangle outline/stroke
void DrawRoundedRectangleLines(float x, float y, float width, float height, float radius, float strokeWidth, const Color &color);
