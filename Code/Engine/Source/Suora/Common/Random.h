#pragma once
#include <random>

class Random
{
    std::mt19937 _randomNumberGenerator;
    std::uniform_real_distribution<double_t> _realDistribution;
    std::uniform_int_distribution<int32_t> _bytedistribution;
public:
    Random(uint_least32_t seed)
    {
        this->_randomNumberGenerator = std::mt19937(seed);
        this->_realDistribution = std::uniform_real_distribution<double_t>();
        this->_bytedistribution = std::uniform_int_distribution<int32_t>(0, 256);
    }
    Random() : Random(std::_Random_device()) {};
    ~Random() {}
    int32_t Next()
    {
        return this->Next(0, INT32_MAX);
    }
    int32_t Next(int32_t maxValue)
    {
        return this->Next(0, maxValue);
    }
    int32_t Next(int32_t minValue, int32_t maxValue)
    {
        std::uniform_int_distribution<int32_t> distribution(minValue, maxValue);
        return distribution(this->_randomNumberGenerator);
    }
    double_t NextDouble()
    {
        return this->_realDistribution(this->_randomNumberGenerator);
    }
    double_t NextDouble(double_t minValue, double_t maxValue)
    {
        std::uniform_real_distribution<double_t> distribution(minValue, minValue);
        return this->_realDistribution(this->_randomNumberGenerator);
    }
    void NextBytes(std::vector<uint8_t>& buffer)
    {
        for (auto& i : buffer)
        {
            i = static_cast<uint8_t>(_bytedistribution(this->_randomNumberGenerator));
        }
    }

};