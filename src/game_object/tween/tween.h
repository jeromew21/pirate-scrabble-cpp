#pragma once

#include <algorithm>
#include <vector>
#include <functional>
#include <cmath>

// Easing functions
namespace Easing {
    inline float Linear(float t) { return t; }
    inline float EaseInQuad(float t) { return t * t; }
    inline float EaseOutQuad(float t) { return t * (2.0f - t); }

    inline float EaseInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }

    inline float EaseInCubic(float t) { return t * t * t; }

    inline float EaseOutCubic(float t) {
        return 1.0f + (t - 1.0f) * (t - 1.0f) * (t - 1.0f);;
    }

    inline float EaseInOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
    }

    inline float EaseInSine(float t) { return 1.0f - cosf(t * 1.57079632679f); }
    inline float EaseOutSine(float t) { return sinf(t * 1.57079632679f); }
    inline float EaseInOutSine(float t) { return -(cosf(3.14159265359f * t) - 1.0f) / 2.0f; }
}

using EasingFunc = std::function<float(float)>;

class Tween {
public:
    float *target;
    float startValue;
    float endValue;
    float duration;
    float elapsed;
    EasingFunc easing;
    std::function<void()> onComplete;
    bool finished;

    Tween(float *ptr, float start, float end, float dur, EasingFunc ease = Easing::Linear);

    void Update(float dt);

    void SetOnComplete(std::function<void()> callback);
};

class TweenManager {
private:
    std::vector<Tween> tweens;

public:
    Tween *CreateTween(float *target, float endValue, float duration,
                       EasingFunc easing = Easing::Linear) {
        tweens.emplace_back(target, *target, endValue, duration, easing);
        return &tweens.back();
    }

    Tween *CreateTweenFromTo(float *target, float startValue, float endValue,
                             float duration, EasingFunc easing = Easing::Linear) {
        tweens.emplace_back(target, startValue, endValue, duration, easing);
        return &tweens.back();
    }

    void Update(float deltaTime) {
        tweens.erase(
            std::remove_if(tweens.begin(), tweens.end(),
                           [deltaTime](Tween &tween) {
                               tween.Update(deltaTime);
                               return tween.finished;
                           }),
            tweens.end()
        );
    }

    void Clear() {
        tweens.clear();
    }

    [[nodiscard]] size_t GetActiveTweenCount() const {
        return tweens.size();
    }
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
