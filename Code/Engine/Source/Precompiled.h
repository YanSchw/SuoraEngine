#pragma once

#include "Suora/Core/PlatformDetection.h"

#ifdef SUORA_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <memory>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Suora/Core/Base.h"

#include "Suora/Core/Log.h"

#ifdef SUORA_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
