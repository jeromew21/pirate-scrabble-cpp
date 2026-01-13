#pragma once

#include <algorithm>
#include <vector>
#include <functional>
#include <cmath>

// Easing functions
namespace Easing {
    inline float Linear(const float t) { return t; }

    inline float EaseInQuad(const float t) { return t * t; }

    inline float EaseOutQuad(const float t) { return t * (2.0f - t); }

    inline float EaseInOutQuad(const float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }

    inline float EaseInCubic(const float t) { return t * t * t; }

    inline float EaseOutCubic(const float t) {
        return 1.0f + (t - 1.0f) * (t - 1.0f) * (t - 1.0f);;
    }

    inline float EaseInOutCubic(const float t) {
        return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
    }

    inline float EaseInSine(const float t) { return 1.0f - cosf(t * 1.57079632679f); }

    inline float EaseOutSine(const float t) { return sinf(t * 1.57079632679f); }

    inline float EaseInOutSine(const float t) { return -(cosf(3.14159265359f * t) - 1.0f) / 2.0f; }
}

using EasingFunc = std::function<float(float)>;

class Tween {
public:
    float *target;

    float start_value;

    float end_value;

    float duration;

    float elapsed;

    EasingFunc easing;

    std::function<void()> on_complete;

    bool finished;

    Tween(float *ptr, float start, float end, float dur, EasingFunc ease = Easing::Linear);

    void Update(float dt);

    void SetOnComplete(const std::function<void()> &callback);
};

class TweenManager {
    std::vector<Tween> tween_list;

public:
    Tween *CreateTween(float *target, float end_value, float duration,
                       EasingFunc easing = Easing::Linear);

    Tween *CreateTweenFromTo(float *target, float start_value, float end_value,
                             float duration, EasingFunc easing = Easing::Linear);

    void Update(float delta_time);

    void Clear();

    [[nodiscard]] size_t GetActiveTweenCount() const;

    static TweenManager& instance();
};

// Example usage:
/*
int main() {
    TweenManager tweenManager;

    float playerX = 0.0f;
    float playerY = 0.0f;
    float alpha = 0.0f;

    // Tween from current value to 100 over 2 seconds with ease-out
    tweenManager.CreateTween(&playerX, 100.0f, 2.0f, Easing::EaseOutQuad);

    // Tween from specific start to end value
    auto* yTween = tweenManager.CreateTweenFromTo(&playerY, 0.0f, 50.0f, 1.5f, Easing::EaseInOutCubic);
    yTween->SetOnComplete([]() {
        // Do something when tween completes
    });

    // Fade in alpha
    tweenManager.CreateTween(&alpha, 1.0f, 1.0f, Easing::EaseInSine);

    // Game loop
    float deltaTime = 0.016f; // 60 FPS
    while (true) {
        tweenManager.Update(deltaTime);
        // Render using playerX, playerY, alpha values
    }

    return 0;
}
*/
