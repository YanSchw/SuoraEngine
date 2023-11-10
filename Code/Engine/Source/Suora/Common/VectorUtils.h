#pragma once
#include <ostream>
#include "Suora/Common/StringUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Suora
{
    using Vec2 = glm::vec2;
    using iVec2 = glm::ivec2;
    using Vec3 = glm::vec3;
    using Vec3d = glm::dvec3;
    using Vec4 = glm::vec4;
    using Vec4d = glm::dvec4;
    using Color = glm::vec4;

    using Quat = glm::quat;

    struct Vec
    {
        template<class T> inline static T Normalized(const T& a) { return glm::normalize(a); }
        template<class T> inline static void Normalize(T& a) { a = glm::normalize(a); }
        template<class T> inline static float Distance(const T& a, const T& b) { return glm::distance(a, b); }

        // Static const Vectors
        static const Vec3 Zero;
        static const Vec3 Right;
        static const Vec3 Up;
        static const Vec3 Forward;
        static const Vec3 One;
        
        template<class T> static String ToString(const T& vec);
        template<class T> static T FromString(const String& str);
    };

    struct Rotator
    {
        float Pitch = 0.0f;
        float Yaw = 0.0f;
        float Roll = 0.0f;

        Quat ToQuat() const
        {
            const Quat yaw = glm::angleAxis(Yaw, Vec::Up);
            const Quat pitch = glm::angleAxis(Pitch, Vec::Right);
            const Quat roll = glm::angleAxis(Roll, Vec::Forward);

            return (yaw * pitch * roll);
        }
    };

    static std::ostream& operator<<(std::ostream& stream, const Vec2& vec)
    {
        stream << "{" << vec.x << ", " << vec.y << "}";
        return stream;
    }
    static std::ostream& operator<<(std::ostream& stream, const Vec3& vec)
    {
        stream << "{" << vec.x << ", " << vec.y << ", " << vec.z << "}";
        return stream;
    }

}

namespace std
{
    template <>
    struct hash<Suora::Vec3>
    {
        size_t operator()(const Suora::Vec3& k) const
        {
            // Compute individual hash values for first,
            // second and third and combine them using XOR
            // and bit shifting:

            return ((std::hash<float>()(k.x)
                ^ (std::hash<float>()(k.y) << 1)) >> 1)
                ^ (std::hash<float>()(k.z) << 1);
        }
    };

}