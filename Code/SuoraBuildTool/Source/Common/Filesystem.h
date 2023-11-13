#pragma once
#include "Common/StringUtils.h"
#include <filesystem>
#include <vector>
#include <algorithm>

using FilePath = std::filesystem::path;
using DirectoryEntry = std::filesystem::directory_entry;

namespace File
{
    static FilePath NormalizedTrimed(const FilePath& p)
    {
        auto r = p.lexically_normal();
        if (r.has_filename()) return r;
        return r.parent_path();
    }

    static bool IsPathSubpathOf(const FilePath& base, const FilePath& sub)
    {
        auto b = NormalizedTrimed(base);
        auto s = NormalizedTrimed(sub).parent_path();
        auto m = std::mismatch(b.begin(), b.end(),
            s.begin(), s.end());

        return m.first == b.end();
    }
    static inline std::vector<DirectoryEntry> GetAllAbsoluteEntriesOfPath(const FilePath& Path)
    {
        std::vector<DirectoryEntry> Entries;
        for (auto file : std::filesystem::directory_iterator(Path))
        {
            if (file.is_directory())
            {
                std::vector<DirectoryEntry> SubEntries = GetAllAbsoluteEntriesOfPath(file.path());
                for (auto sub : SubEntries)
                {
                    Entries.push_back(sub);
                }
            }
            else
            {
                Entries.push_back(file);
            }
        }

        return Entries;
    }

    static std::string GetFileExtension(const DirectoryEntry& File)
    {
        return File.path().extension().string();
    }

}