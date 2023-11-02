#include "Precompiled.h"
#include "VectorUtils.h"
#include "Suora/Common/Common.h"

namespace Suora
{
    template<>
    std::string Vec::ToString<Vec2>(const Vec2& vec)
    {
        return (std::to_string(vec.x) + "/" + std::to_string(vec.y));
    }
    template<>
    std::string Vec::ToString<Vec3>(const Vec3& vec)
    {
        return (std::to_string(vec.x) + "/" + std::to_string(vec.y) + "/" + std::to_string(vec.z));
    }
    template<>
    std::string Vec::ToString<Vec4>(const Vec4& vec)
    {
        return (std::to_string(vec.x) + "/" + std::to_string(vec.y) + "/" + std::to_string(vec.z) + "/" + std::to_string(vec.w));
    }
    template<>
    std::string Vec::ToString<Quat>(const Quat& quat)
    {
        return (std::to_string(quat.x) + "/" + std::to_string(quat.y) + "/" + std::to_string(quat.z) + "/" + std::to_string(quat.w));
    }
    template<>
    std::string Vec::ToString<glm::mat4>(const glm::mat4& m)
    {
        std::string str;
        str += ToString<Vec4>(m[0]) + ";";
        str += ToString<Vec4>(m[1]) + ";";
        str += ToString<Vec4>(m[2]) + ";";
        str += ToString<Vec4>(m[3]) + "";
        return str;
    }

    template<>
    Vec2 Vec::FromString<Vec2>(const std::string& str)
    {
        if (str == "") return Vec::Zero;
        const std::vector<std::string> xyz = Util::SplitString(str, '/');
        return Vec2(std::stof(xyz[0]), std::stof(xyz[1]));
    }
    template<>
    Vec3 Vec::FromString<Vec3>(const std::string& str)
    {
        if (str == "") return Vec::Zero;
        const std::vector<std::string> xyz = Util::SplitString(str, '/');
        return Vec3(std::stof(xyz[0]), std::stof(xyz[1]), std::stof(xyz[2]));
    }
    template<>
    Vec4 Vec::FromString<Vec4>(const std::string& str)
    {
        if (str == "") return Vec4();
        const std::vector<std::string> xyzw = Util::SplitString(str, '/');
        return Vec4(std::stof(xyzw[0]), std::stof(xyzw[1]), std::stof(xyzw[2]), std::stof(xyzw[3]));
    }
    template<>
    Quat Vec::FromString<Quat>(const std::string& str)
    {
        if (str == "") return glm::identity<Quat>();
        const std::vector<std::string> xyzw = Util::SplitString(str, '/');
        return Quat(std::stof(xyzw[0]), std::stof(xyzw[1]), std::stof(xyzw[2]), std::stof(xyzw[3]));
    }
    template<>
    glm::mat4 Vec::FromString<glm::mat4>(const std::string& str)
    {
        if (str == "") return glm::mat4();
        glm::mat4 m = glm::mat4();
        std::vector<std::string> rows = Util::SplitString(str, ';');

        m[0] = FromString<Vec4>(rows[0]);
        m[1] = FromString<Vec4>(rows[1]);
        m[2] = FromString<Vec4>(rows[2]);
        m[3] = FromString<Vec4>(rows[3]);

        return m;
    }

    const Vec3 Vec::Zero = Vec3();
    const Vec3 Vec::Right = Vec3(1, 0, 0);
    const Vec3 Vec::Up = Vec3(0, 1, 0);
    const Vec3 Vec::Forward = Vec3(0, 0, 1);
    const Vec3 Vec::One = Vec3(1);
}