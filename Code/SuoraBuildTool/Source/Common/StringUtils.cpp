#include "StringUtils.h"
#include <vector>
#include <sstream>

namespace Suora
{


    std::vector<std::string> StringUtil::SplitString(const std::string& s, char delim)
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

    bool StringUtil::ReplaceSequence(std::string& str, const std::string& from, const std::string& to)
    {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos) return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    std::string StringUtil::ToLower(std::string str)
    {
        for (char& c : str)
        {
            c = tolower(c);
        }

        return str;
    }
    std::string StringUtil::ToUpper(std::string str)
    {
        for (char& c : str)
        {
            c = toupper(c);
        }

        return str;
    }
    std::string StringUtil::SmartToUpperCase(std::string str, bool allToUpper)
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

    void StringUtil::RemoveCommentsFromString(std::string& str)
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


}