#include "stdafx.hpp"

#include "App.hpp"

#include <spdlog/common.h>

int main() {
	spdlog::set_level(spdlog::level::info);
	PhoenixEngine::App app;
	app.run();

	return 0;
}
