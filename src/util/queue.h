#pragma once

#include <string>

#include "concurrentqueue.h"

using Queue = moodycamel::ConcurrentQueue<std::string>;
