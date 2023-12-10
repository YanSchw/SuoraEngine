#pragma once
#include <string>
#include <vector>

namespace Suora
{

    struct StringUtil
    {
        static std::vector<std::string> SplitString(const std::string& s, char delim);
        static bool ReplaceSequence(std::string& str, const std::string& from, const std::string& to);

        static std::string ToLower(std::string str);
        static std::string ToUpper(std::string str);
        static std::string SmartToUpperCase(std::string str, bool allToUpper);

        static void RemoveCommentsFromString(std::string& str);
    };

}