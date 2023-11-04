#pragma once
#include <vector>
#include <string>
#include <sstream>
#include "Array.h"

namespace Suora
{
	namespace Util
	{

        static std::vector<std::string> SplitString(const std::string& s, char delim) 
        {
            std::vector<std::string> result;
            std::stringstream ss(s);
            std::string item;

            while (std::getline(ss, item, delim)) 
            {
                result.push_back(item);
            }

            return result;
        }
        static bool ReplaceSequence(std::string& str, const std::string& from, const std::string& to) 
        {
            size_t start_pos = str.find(from);
            if (start_pos == std::string::npos) return false;
            str.replace(start_pos, from.length(), to);
            return true;
        }
        static std::string FloatToString(float f, size_t floatingPointDigits = 3)
        {
            std::string str = std::to_string(f);
            
            for (int i = 0; str.size(); i++)
            {
                if (str.at(i) == '.')
                {
                    return str.substr(0, (size_t) (i + floatingPointDigits >= str.size() ? str.size() : i + floatingPointDigits));
                }
            }

            return std::to_string(f);
        }
        static std::string Int32ToString(int32_t i)
        {
            return FloatToString((float)i);
        }
        static float StringToFloat(const std::string& str)
        {
            static Array<char> numericChars = {'-', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
            bool invalidString = false;
            for (char c : str)
            {
                if (!numericChars.Contains(c))
                {
                    std::string STR = str;
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
        static int32_t StringToInt32(const std::string& str)
        {
            return (int32_t)StringToFloat(str);
        }

        static std::string ToLower(std::string str)
        {
            for (char& c : str)
            {
                c = tolower(c);
            }

            return str;
        }
        static std::string ToUpper(std::string str)
        {
            for (char& c : str)
            {
                c = toupper(c);
            }

            return str;
        }

        static void RemoveCommentsFromString(std::string& str)
        {
            while (str.find("/*") != std::string::npos)
            {
                size_t start = str.find("/*");
                size_t end = str.find("*/", start);
                str.erase(start, end - start + 2);
            }
            while (str.find("//") != std::string::npos)
            {
                size_t pos = str.find("//");
                while (str[pos] != '\n' && pos < str.size())
                {
                    str.erase(pos, 1);
                }
            }
        }

        static std::string SmartToUpperCase(std::string str, bool allToUpper)
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

	}
}