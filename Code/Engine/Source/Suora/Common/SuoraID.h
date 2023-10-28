#pragma once

#include <string>
#include <random>
#include <sstream>
#include <inttypes.h>

namespace uuid 
{
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    static std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}

namespace Suora
{

    struct SuoraID
    {
    private:
        std::string str;
    public:
        SuoraID()
        {
            str = "INVALID";
        }
        SuoraID(const std::string& id)
        {
            str = id;
        }

        inline bool operator==(const SuoraID& other) const
        {
            return str == other.str;
        }

        const std::string& GetString() const
        {
            return str;
        }

        static SuoraID Generate()
        {
            return SuoraID(uuid::generate_uuid_v4());
        }


        static const SuoraID Invalid;

    };

}