#pragma once

#include <concurrentqueue.h>

using Queue = moodycamel::ConcurrentQueue<std::string>;

bool read_file(const std::string &path, std::string& outContents);

bool write_file(const std::string &path, const std::string &contents);
