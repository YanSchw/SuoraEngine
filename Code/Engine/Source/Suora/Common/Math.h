#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <inttypes.h>
#include <random>

namespace Suora
{

	struct Math
	{
		// Global Consts
		inline static constexpr float PI = 3.1415926535f;


		inline static float Remap(float value, float in1, float in2, float out1, float out2)
		{
			return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
		}

		inline static float Clamp(float value, float min, float max)
		{
			if (min <= value && value <= max) return value;
			if (value < min) return min;
			return max;
		}
		inline static int Clamp(int value, int min, int max)
		{
			if (min <= value && value <= max) return value;
			if (value < min) return min;
			return max;
		}

		template<class T, class V>
		inline static T Lerp(const T& a, const T& b, const V v)
		{
			return a + ((b - a) * v);
		}
		template<class T, class V>
		inline static T LerpTowards(const T& a, const T& b, const V v)
		{
			return Lerp<T, V>(a, b, glm::clamp<V>(v, (V)0, (V)1));
		}

		inline static float Abs(float value)
		{
			return value < 0 ? value * -1.0f : value;
		}

		inline static float Sin(float value)
		{
			return glm::sin(value);
		}

		inline static bool IsPointInRect(const Vec2& point, float x, float y, uint32_t width, uint32_t height)
		{
			return point.x > x && point.x < x + width && point.y > y && point.y < y + height;
		}

		inline static int RoundToInt(float f)
		{
			return std::round(f);
		}

		inline static float RandomFloat()
		{
			std::random_device rd;
			static std::mt19937 mt(rd());
			std::uniform_real_distribution<float> dt(0.0f, 1.0f);
			return dt(mt);
		}

	};

}