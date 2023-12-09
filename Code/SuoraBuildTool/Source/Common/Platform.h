#pragma once

#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>

struct Platform
{
	static double GetTime()
	{
		auto currentTime = std::chrono::steady_clock::now();
		return currentTime.time_since_epoch().count() / 1000000000.0;
	}
	static void WriteToFile(const std::string& filePath, const std::string& content)
	{
		std::ofstream writer;
		writer.open(filePath);

		writer << content;

		writer.close();
	}
	static std::string ReadFromFile(const std::string& filePath)
	{
		std::ifstream reader(filePath);
		std::string str((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
		reader.close();

		return str;
	}
};