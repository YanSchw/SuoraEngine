#pragma once
#include <string>
#include "Suora/Core/Base.h"

namespace Suora
{
	using String = std::string;

    struct StringUtil
    {
        static std::vector<String> SplitString(const String& s, char delim);
        static bool ReplaceSequence(String& str, const String& from, const String& to);
        static String FloatToString(float f, size_t floatingPointDigits = 3);
        static String Int32ToString(int32 i);
        static float StringToFloat(const String& str);
        static int32 StringToInt32(const String& str);

        static String ToLower(String str);
        static String ToUpper(String str);
        static String SmartToUpperCase(String str, bool allToUpper);

        static void RemoveCommentsFromString(String& str);
    };

}