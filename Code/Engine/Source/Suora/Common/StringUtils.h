#pragma once
#include <string>
#include <vector>
#include "Array.h"

namespace Suora
{
	using String = std::string;

    struct StringUtil
    {
        static Array<String> SplitString(const String& InString, char InDelimiter);
        static bool ReplaceSequence(String& str, const String& from, const String& to);
        static String FloatToString(float f, size_t floatingPointDigits = 3);
        static String Int32ToString(int32_t i);
        static float StringToFloat(const String& str);
        static int32_t StringToInt32(const String& str);

        static String ToLower(String str);
        static String ToUpper(String str);
        static String SmartToUpperCase(String str, bool allToUpper);

        static void RemoveCommentsFromString(String& str);
    };

}