#include "filesystem.h"

#include <iostream>
#include <fstream>

#ifdef __linux__
#include <unistd.h>
#include <climits>
#elif __APPLE__
#include <mach-o/dyld.h>
#elif _WIN32
#include <windows.h>
#endif


bool read_file(const std::string &path, std::string &out_contents) {
    std::ifstream file(path); // Open for reading
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << path << "\n";
        return false;
    }
    out_contents.clear();
    std::string line;
    while (std::getline(file, line)) {
        out_contents += line + "\n";
    }
    if (file.bad()) {
        std::cerr << "I/O error while reading file: " << path << "\n";
        return false;
    }
    return true;
}

bool write_file(const std::string &path, const std::string &contents) {
    std::ofstream file(path); // Open for writing (truncates by default)
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << path << "\n";
        return false;
    }
    file << contents;
    if (!file) {
        // Checks for write errors
        std::cerr << "Failed to write to file: " << path << "\n";
        return false;
    }
    return true;
}

fs::path executable_path() {
#ifdef __linux__
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        auto path = fs::path{buffer};
        return path;
    }
#elif __APPLE__
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        auto path = fs::path{buffer};
        return path;
    }
#elif _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    auto path = fs::path{buffer};
    return path;
#endif
    return "";
}

fs::path executable_dir() {
    return executable_path().parent_path();
}
