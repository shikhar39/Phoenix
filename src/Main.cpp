#include <spdlog/common.h>

#include "App.h"
#include "stdafx.h"

int main() {
	spdlog::set_level(spdlog::level::info);
	PhoenixEngine::App app;
	app.run();

	return 0;
}
