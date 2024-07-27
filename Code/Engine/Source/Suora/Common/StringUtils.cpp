#include "Precompiled.h"
#include "StringUtils.h"
#include "Array.h"
#include <sstream>

namespace Suora
{

    Array<String> StringUtil::SplitString(const String& InString, char InDelimiter)
    {
        Array<String> result;
        std::stringstream ss(InString);
        String item;

        while (std::getline(ss, item, InDelimiter))
        {
            result.Add(item);
        }

        return result;
    }

    bool StringUtil::ReplaceSequence(String& str, const String& from, const String& to)
    {
        size_t start_pos = str.find(from);
        if (start_pos == String::npos) return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    String StringUtil::FloatToString(float f, size_t floatingPointDigits)
    {
        String str = std::to_string(f);

        for (int i = 0; str.size(); i++)
        {
            if (str.at(i) == '.')
            {
                return str.substr(0, (size_t)(i + floatingPointDigits >= str.size() ? str.size() : i + floatingPointDigits));
            }
        }

        return std::to_string(f);
    }
    String StringUtil::Int32ToString(int32_t i)
    {
        return FloatToString((float)i);
    }
    float StringUtil::StringToFloat(const String& str)
    {
        static Array<char> numericChars = { '-', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
        bool invalidString = false;
        for (char c : str)
        {
            if (!numericChars.Contains(c))
            {
                String STR = str;
                for (int64_t i = STR.size() - 1; i >= 0; i--)
                {
                    if (!numericChars.Contains(STR[i]))
                    {
                        STR.erase(i, 1);
                    }
                }
                return STR.size() > 0 ? std::stof(STR) : 0.0f;
            }
        }

        return str.size() > 0 ? std::stof(str) : 0.0f;
    }
    int32_t StringUtil::StringToInt32(const String& str)
    {
        return (int32_t)StringToFloat(str);
    }

    String StringUtil::ToLower(String str)
    {
        for (char& c : str)
        {
            c = tolower(c);
        }

        return str;
    }
    String StringUtil::ToUpper(String str)
    {
        for (char& c : str)
        {
            c = toupper(c);
        }

        return str;
    }
    String StringUtil::SmartToUpperCase(String str, bool allToUpper)
    {
        if (str.size() < 1) return str;

        for (int64_t it = str.size() - 1; it >= 1; it--)
        {
            if (str[it] >= 'A' && str[it] <= 'Z' && str[it - 1] >= 'a' && str[it - 1] <= 'z' && str[it - 1] != ' ')
            {
                str.insert(it, " ");
            }
        }

        while (str[0] == ' ') str.erase(0, 1);

        {
            char& c = str[0];
            if (c >= 'a' && c <= 'z') c += ('A' - 'a');
        }

        if (allToUpper)
        {
            for (char& c : str)
            {
                if (c >= 'a' && c <= 'z') c += ('A' - 'a');
            }
        }

        return str;
    }

    void StringUtil::RemoveCommentsFromString(String& str)
    {
        while (str.find("/*") != String::npos)
        {
            size_t start = str.find("/*");
            size_t end = str.find("*/", start);
            str.erase(start, end - start + 2);
        }
        while (str.find("//") != String::npos)
        {
            size_t pos = str.find("//");
            while (str[pos] != '\n' && pos < str.size())
            {
                str.erase(pos, 1);
            }
        }
    }


}