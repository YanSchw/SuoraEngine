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

    bool StringUtil::ReplaceSequence(String& InString, const String& InFrom, const String& InTo)
    {
        size_t start_pos = InString.find(InFrom);
        if (start_pos == String::npos) return false;
        InString.replace(start_pos, InFrom.length(), InTo);
        return true;
    }

    String StringUtil::FloatToString(float InFloat, size_t InFloatingPointDigits)
    {
        String str = std::to_string(InFloat);

        for (int i = 0; str.size(); i++)
        {
            if (str.at(i) == '.')
            {
                return str.substr(0, (size_t)(i + InFloatingPointDigits >= str.size() ? str.size() : i + InFloatingPointDigits));
            }
        }

        return std::to_string(InFloat);
    }
    String StringUtil::Int32ToString(int32_t InInt32)
    {
        return FloatToString((float)InInt32);
    }
    float StringUtil::StringToFloat(const String& InString)
    {
        static Array<char> numericChars = { '-', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
        bool invalidString = false;
        for (char c : InString)
        {
            if (!numericChars.Contains(c))
            {
                String STR = InString;
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

        return InString.size() > 0 ? std::stof(InString) : 0.0f;
    }
    int32_t StringUtil::StringToInt32(const String& InString)
    {
        return (int32_t)StringToFloat(InString);
    }

    String StringUtil::ToLower(String InString)
    {
        for (char& c : InString)
        {
            c = tolower(c);
        }

        return InString;
    }
    String StringUtil::ToUpper(String InString)
    {
        for (char& c : InString)
        {
            c = toupper(c);
        }

        return InString;
    }
    String StringUtil::SmartToUpperCase(String InString, bool InAllToUpper)
    {
        if (InString.size() < 1) return InString;

        for (int64_t it = InString.size() - 1; it >= 1; it--)
        {
            if (InString[it] >= 'A' && InString[it] <= 'Z' && InString[it - 1] >= 'a' && InString[it - 1] <= 'z' && InString[it - 1] != ' ')
            {
                InString.insert(it, " ");
            }
        }

        while (InString[0] == ' ') InString.erase(0, 1);

        {
            char& c = InString[0];
            if (c >= 'a' && c <= 'z') c += ('A' - 'a');
        }

        if (InAllToUpper)
        {
            for (char& c : InString)
            {
                if (c >= 'a' && c <= 'z') c += ('A' - 'a');
            }
        }

        return InString;
    }

    void StringUtil::RemoveCommentsFromString(String& OutString)
    {
        while (OutString.find("/*") != String::npos)
        {
            size_t start = OutString.find("/*");
            size_t end = OutString.find("*/", start);
            OutString.erase(start, end - start + 2);
        }
        while (OutString.find("//") != String::npos)
        {
            size_t pos = OutString.find("//");
            while (OutString[pos] != '\n' && pos < OutString.size())
            {
                OutString.erase(pos, 1);
            }
        }
    }


}