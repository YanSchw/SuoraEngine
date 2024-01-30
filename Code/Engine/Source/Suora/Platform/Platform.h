#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <optional>

namespace Suora
{
	/** Platform Utility Struct */
	struct Platform
	{
		/** Get Time in Seconds. Should only be used to calculate time deltas, since the absolute Time may differ
		*   depending on the Platform.                                                                            */
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

		/** Open the given file in the OS default application for the file format.
		*   e.g. A .txt file on Windows may be opened in Notepad.                  */
		static void OpenFileExternally(const String& filePath);

		/** Execute a given Input in the OS Command Line */
		static void CommandLine(const String& input);
	};


}