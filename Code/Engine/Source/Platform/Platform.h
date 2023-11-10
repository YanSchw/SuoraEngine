#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <optional>

namespace Suora
{

	struct Platform
	{
		static float GetTime();

		static void WriteToFile(const String& filePath, const String& content);
		static String ReadFromFile(const String& filePath);
		static String ReadFromFile(const std::filesystem::path& path) { return ReadFromFile(path.string()); }
		static void RenameFile(const std::filesystem::path& filePath, const String& rename);

		static std::optional<String> OpenFileDialog();
		static std::optional<String> OpenFileDialog(const std::vector<String>& filter);
		static std::optional<String> ChoosePathDialog();

		static void WriteToClipboard(const String& str);
		static String ReadFromClipboard();

		static void ShowInExplorer(const std::filesystem::path& folderPath);
		static void CreateDirectory(const std::filesystem::path& folderPath);
		static void CopyDirectory(const std::filesystem::path& src, const std::filesystem::path& dest);
		static void RemoveFile(const std::filesystem::path& filePath);

		static void OpenFileExternally(const String& filePath);
	};


}