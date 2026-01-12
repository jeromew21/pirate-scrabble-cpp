#include "tween.h"

Tween::Tween(float *ptr, float start, float end, float dur, EasingFunc ease): target(ptr), startValue(start), endValue(end), duration(dur),
                                                                              elapsed(0.0f), easing(ease), finished(false) {
    *target = startValue;
}

void Tween::Update(float dt) {
    if (finished) return;

    elapsed += dt;
    float t = elapsed / duration;

    if (t >= 1.0f) {
        t = 1.0f;
        finished = true;
        *target = endValue;
        if (onComplete) onComplete();
    } else {
        float easedT = easing(t);
        *target = startValue + (endValue - startValue) * easedT;
    }
}

void Tween::SetOnComplete(std::function<void()> callback) {
    onComplete = callback;
}
