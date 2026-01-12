#pragma once

#include <algorithm>
#include "raylib.h"

// Draw a filled rounded rectangle
void DrawRoundedRectangle(float x, float y, float width, float height, float radius, Color color);

// Draw a rounded rectangle outline/stroke
void DrawRoundedRectangleLines(float x, float y, float width, float height, float radius, float strokeWidth, Color color);
