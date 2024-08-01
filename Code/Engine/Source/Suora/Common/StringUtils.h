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
        static bool ReplaceSequence(String& InString, const String& InFrom, const String& InTo);
        static String FloatToString(float InFloat, size_t InFloatingPointDigits = 3);
        static String Int32ToString(int32_t InInt32);
        static float StringToFloat(const String& InString);
        static int32_t StringToInt32(const String& InString);

        static String ToLower(String InString);
        static String ToUpper(String InString);
        static String SmartToUpperCase(String InString, bool InAllToUpper);

        static void RemoveCommentsFromString(String& OutString);
    };

}