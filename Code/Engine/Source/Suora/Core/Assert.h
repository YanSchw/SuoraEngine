#pragma once

#include "Suora/Core/Base.h"
#include "Suora/Core/Log.h"
#include <filesystem>

#if defined(SUORA_ENABLE_ASSERTS) || defined(SUORA_ENABLE_VERIFY)

	#define SUORA_INTERNAL_ASSERT_IMPL(check, msg, ...) { if(!(check)) { SUORA_ERROR(::Suora::LogCategory::Assert, msg, __VA_ARGS__); SUORA_DEBUGBREAK(); } }
	#define SUORA_INTERNAL_ASSERT_WITH_MSG(check, ...) SUORA_INTERNAL_ASSERT_IMPL(check, "Assertion failed: {0}", __VA_ARGS__)
	#define SUORA_INTERNAL_ASSERT_NO_MSG(check) SUORA_INTERNAL_ASSERT_IMPL(check, "Assertion '{0}' failed at {1}:{2}", SUORA_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define SUORA_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define SUORA_INTERNAL_ASSERT_GET_MACRO(...) SUORA_EXPAND_MACRO( SUORA_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, SUORA_INTERNAL_ASSERT_WITH_MSG, SUORA_INTERNAL_ASSERT_NO_MSG) )

#endif


#ifdef SUORA_ENABLE_ASSERTS
	#define SUORA_ASSERT(...) SUORA_EXPAND_MACRO( SUORA_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
	#define SUORA_ASSERT(...)
#endif

#ifdef SUORA_ENABLE_VERIFY
	#define SUORA_VERIFY(...) SUORA_EXPAND_MACRO( SUORA_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
	#define SUORA_VERIFY(...)
#endif

#define SuoraAssert(...) SUORA_ASSERT(__VA_ARGS__)
#define SuoraVerify(...) SUORA_VERIFY(__VA_ARGS__)