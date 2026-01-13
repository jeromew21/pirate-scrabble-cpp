#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

#ifdef __EMSCRIPTEN__
inline const fs::path FS_ROOT = "/";
#else
inline const fs::path FS_ROOT = ".";
#endif

bool read_file(const std::string &path, std::string& out_contents);

bool write_file(const std::string &path, const std::string &contents);

fs::path executable_path();

fs::path executable_dir();


