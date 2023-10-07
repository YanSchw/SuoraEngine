#pragma once

#define SUORA_VERSION_MAJOR 0
#define SUORA_VERSION_MINOR 7
#define SUORA_VERSION_PATCH 0

#include <memory>
#include "Suora/Core/PlatformDetection.h"

#ifdef SUORA_DEBUG
	#define SUORA_ENABLE_ASSERTS
#endif

#ifdef SUORA_RELEASE

#endif

#ifdef SUORA_DIST

#endif

// Verifies are always enabled!
#define SUORA_ENABLE_VERIFY

// Debugbreak
#if defined(SUORA_ENABLE_ASSERTS) || defined(SUORA_ENABLE_VERIFY)
	#if defined(SUORA_PLATFORM_WINDOWS)
		#define SUORA_DEBUGBREAK() __debugbreak()
	#elif defined(SUORA_PLATFORM_LINUX)
		#include <signal.h>
		#define SUORA_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
#else
	#define SUORA_DEBUGBREAK()
#endif

#define SUORA_EXPAND_MACRO(x) x
#define SUORA_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define SUORA_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define SUORA_DEPRECATED(Version, Message) [[deprecated(Message "Please update your code to the new API before upgrading to the next release, otherwise your project will no longer compile.")]]

namespace Suora 
{

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using WeakRef = std::weak_ptr<T>;

}

#include "Suora/Core/Log.h"
#include "Suora/Core/Assert.h"
