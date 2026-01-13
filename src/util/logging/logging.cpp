#include "logging.h"

#include <memory>

std::unique_ptr<Logger> instance_;

void Logger::Initialize(const char* output_file) {
    instance_ = std::make_unique<Logger>(output_file);
}

Logger &Logger::instance() {
    return *instance_;
}
