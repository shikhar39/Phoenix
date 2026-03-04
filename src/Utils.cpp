#include "Utils.hpp"

std::filesystem::path Utils::getExecutableDir()
{
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();

#elif defined(__linux__)
		char buffer[4096];
		ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		if (len != -1)
		{
			buffer[len] = '\0';
			return std::filesystem::path(buffer).parent_path();
		}
		return std::filesystem::current_path();

#else
		return std::filesystem::current_path();
#endif
}

std::filesystem::path Utils::getProjectRoot()
{
	auto dir = std::filesystem::weakly_canonical(getExecutableDir());

	for (auto current = dir; !current.empty(); current = current.parent_path())
	{
		if (isProjectRoot(current))
			return current;

		// Stop at filesystem root (parent == self)
		if (current == current.parent_path())
			break;
	}

	throw std::runtime_error("Could not find project root starting from: " + dir.string());
}

std::filesystem::path Utils::assetPath(const std::filesystem::path& relative)
{
	return getProjectRoot() / relative;
}

bool Utils::isProjectRoot(const std::filesystem::path& p)
{
	// Pick 1–3 things that only exist at the repo root.
	return existsFile(p / "premake5.lua")
		|| existsDir(p / ".git")
		|| existsDir(p / "assets");
}
