#pragma once

#include <mutex>
#include <vector>
#include <string>
#include <fstream>

#include <fmt/color.h>

class Logger {
public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static void Initialize(const char* output_file);

    static Logger& instance();

    template<typename... Args>
    void info(fmt::format_string<Args...> fmt_str, Args&&... args) {
        log("INFO", fmt::terminal_color::white, fmt_str, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(fmt::format_string<Args...> fmt_str, Args&&... args) {
        log("WARN", fmt::terminal_color::yellow, fmt_str, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(fmt::format_string<Args...> fmt_str, Args&&... args) {
        log("ERROR", fmt::terminal_color::red, fmt_str, std::forward<Args>(args)...);
    }

    const std::vector<std::string>& entries() const;

    explicit Logger(const char* output_file);

    ~Logger();

private:
    const char* output_file;

    std::mutex mutex_;
    std::vector<std::string> entries_;
    std::ofstream file_;

    static std::string timestamp();

    template<typename... Args>
    void log(const char* level,
             const fmt::terminal_color color,
             fmt::format_string<Args...> fmt_str,
             Args&&... args)
    {
        std::string message = fmt::format(fmt_str, std::forward<Args>(args)...);
        std::string line = fmt::format("[{}] [{}] {}", timestamp(), level, message);

        std::lock_guard lock(mutex_);

        entries_.push_back(line);

        if (level == std::string("ERROR")) {
            fmt::print(stderr, fmt::fg(color), "{}\n", line);
        } else {
            fmt::print("{}\n", line);
        }

        if (file_.is_open()) {
            file_ << line << '\n';
            file_.flush();
        }
    }
};
