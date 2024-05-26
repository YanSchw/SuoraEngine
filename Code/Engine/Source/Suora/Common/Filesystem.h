#pragma once
#include <filesystem>
#include "Suora/Common/StringUtils.h"
#include <vector>
#include <algorithm>


namespace Suora
{
    using Path = std::filesystem::path;
    using DirectoryEntry = std::filesystem::directory_entry;

    struct FileUtils
    {
        static Path NormalizedTrimed(const Path& p)
        {
            auto r = p.lexically_normal();
            if (r.has_filename()) return r;
            return r.parent_path();
        }

        static bool IsPathSubpathOf(const Path& base, const Path& sub)
        {
            auto b = NormalizedTrimed(base);
            auto s = NormalizedTrimed(sub).parent_path();
            auto m = std::mismatch(b.begin(), b.end(),
                s.begin(), s.end());

            return m.first == b.end();
        }
        static inline std::vector<DirectoryEntry> GetAllAbsoluteEntriesOfPath(const Path& Path)
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

        static String GetFileExtension(const DirectoryEntry& File)
        {
            return File.path().extension().string();
        }
    };

}