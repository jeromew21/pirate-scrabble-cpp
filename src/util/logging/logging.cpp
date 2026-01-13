#include "logging.h"

#include <chrono>
#include <memory>

std::unique_ptr<Logger> instance_;

void Logger::Initialize(const char* output_file) {
    instance_ = std::make_unique<Logger>(output_file);
}

Logger &Logger::instance() {
    return *instance_;
}

const std::vector<std::string> & Logger::entries() const {
    return entries_;
}

Logger::Logger(const char *output_file): output_file(output_file) {
    file_.open(output_file, std::ios::out | std::ios::app);
}

Logger::~Logger() {
    if (file_.is_open()) file_.close();
}

std::string Logger::timestamp() {
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    std::time_t t = clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}
