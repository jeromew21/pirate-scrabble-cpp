#include "tween.h"

Tween::Tween(float *ptr, float start, float end, float dur, EasingFunc ease) : target(ptr), start_value(start),
                                                                               end_value(end), duration(dur),
                                                                               elapsed(0.0f), easing(ease),
                                                                               finished(false) {
    *target = start_value;
}

void Tween::Update(float dt) {
    if (finished) return;

    elapsed += dt;
    float t = elapsed / duration;

    if (t >= 1.0f) {
        t = 1.0f;
        finished = true;
        *target = end_value;
        if (on_complete) on_complete();
    } else {
        float easedT = easing(t);
        *target = start_value + (end_value - start_value) * easedT;
    }
}

void Tween::SetOnComplete(std::function<void()> callback) {
    on_complete = callback;
}

Tween *TweenManager::CreateTween(float *target, float end_value, float duration, EasingFunc easing) {
    tween_list.emplace_back(target, *target, end_value, duration, easing);
    return &tween_list.back();
}

Tween *TweenManager::CreateTweenFromTo(float *target, float start_value, float end_value, float duration,
                                       EasingFunc easing) {
    tween_list.emplace_back(target, start_value, end_value, duration, easing);
    return &tween_list.back();
}

void TweenManager::Update(float delta_time) {
    tween_list.erase(
        std::remove_if(tween_list.begin(), tween_list.end(),
                       [delta_time](Tween &tween) {
                           tween.Update(delta_time);
                           return tween.finished;
                       }),
        tween_list.end()
    );
}

void TweenManager::Clear() {
    tween_list.clear();
}

size_t TweenManager::GetActiveTweenCount() const {
    return tween_list.size();
}

TweenManager &TweenManager::instance() {
    static TweenManager inst;
    return inst;
}
