#include "filesystem.h"

#include <iostream>
#include <fstream>

bool read_file(const std::string &path, std::string& out_contents) {
    std::ifstream file(path);  // Open for reading
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
    std::ofstream file(path);  // Open for writing (truncates by default)
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << path << "\n";
        return false;
    }

    file << contents;

    if (!file) {  // Checks for write errors
        std::cerr << "Failed to write to file: " << path << "\n";
        return false;
    }

    return true;
}
