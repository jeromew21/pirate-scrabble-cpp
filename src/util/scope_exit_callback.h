#pragma once

#include <utility>
#include <type_traits>

template<typename F>
class ScopeExitCallback {
public:
    explicit ScopeExitCallback(F &&f) noexcept(std::is_nothrow_move_constructible_v<F>)
        : callback(std::forward<F>(f)), active(true) {
    }

    ScopeExitCallback(ScopeExitCallback &&other) noexcept(std::is_nothrow_move_constructible_v<F>)
        : callback(std::move(other.callback)), active(other.active) {
        other.active = false;
    }

    ScopeExitCallback(const ScopeExitCallback &) = delete;

    ScopeExitCallback &operator=(const ScopeExitCallback &) = delete;

    ScopeExitCallback &operator=(ScopeExitCallback &&) = delete;

    ~ScopeExitCallback() noexcept {
        if (active) {
            callback();
        }
    }

    void dismiss() noexcept {
        active = false;
    }

    F callback;

private:
    bool active;
};
