#pragma once

#include <filesystem>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

class Utils
{
public:
	static bool existsDir(const std::filesystem::path& p) {
		std::error_code ec;
		return std::filesystem::is_directory(p, ec);
	}

	static bool existsFile(const std::filesystem::path& p) {
		std::error_code ec;
		return std::filesystem::is_regular_file(p, ec);
	}

	static std::filesystem::path getExecutableDir();
	static std::filesystem::path getProjectRoot();
	static std::filesystem::path assetPath(const std::filesystem::path& relative);
	static bool isProjectRoot(const std::filesystem::path& p);;
};
