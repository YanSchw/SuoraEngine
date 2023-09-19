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

		static void WriteToFile(const std::string& filePath, const std::string& content);
		static std::string ReadFromFile(const std::string& filePath);
		static std::string ReadFromFile(const std::filesystem::path& path) { return ReadFromFile(path.string()); }
		static void RenameFile(const std::filesystem::path& filePath, const std::string& rename);

		static std::optional<std::string> OpenFileDialog();
		static std::optional<std::string> OpenFileDialog(const std::vector<std::string>& filter);
		static std::optional<std::string> ChoosePathDialog();

		static void WriteToClipboard(const std::string& str);
		static std::string ReadFromClipboard();

		static void ShowInExplorer(const std::filesystem::path& folderPath);
		static void CreateDirectory(const std::filesystem::path& folderPath);
		static void CopyDirectory(const std::filesystem::path& src, const std::filesystem::path& dest);
		static void RemoveFile(const std::filesystem::path& filePath);

		static void OpenFileExternally(const std::string& filePath);
	};


}