#pragma once
#include "../Platform.h"

#include "Suora/Core/Window.h"
#include "WindowsWindow.h"
#include "Suora/Core/Application.h"

#include "Windows.h"
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>

#include <istream>
#include <ostream>
#include <fstream>
#include <chrono>
#include <locale>
#include <codecvt>

std::wstring string_to_wstring(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

namespace Suora
{

	float Platform::GetTime()
	{
		auto currentTime = std::chrono::steady_clock::now();
		return currentTime.time_since_epoch().count() / 1000000000.0f;
	}
	void Platform::WriteToFile(const String& filePath, const String& content)
	{
		std::ofstream writer;
		writer.open(filePath);

		writer << content;

		writer.close();
	}
	String Platform::ReadFromFile(const String& filePath)
	{
		std::ifstream reader(filePath);
		String str((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
		reader.close();

		return str;
	}
	void Platform::RenameFile(const std::filesystem::path& filePath, const String& rename)
	{
		if (std::filesystem::exists(filePath))
		{
			std::filesystem::rename(filePath, filePath.parent_path() / (rename + filePath.extension().string()));
		}
	}

	std::optional<String> Platform::OpenFileDialog()
	{
		LPCSTR filter = "All\0*.*\0Text\0*.TXT\0";

		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Window::s_CurrentFocusedWindow->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		return std::nullopt;
	}
	std::optional<String> Platform::OpenFileDialog(const std::vector<String>& filter)
	{
		String allFilters;
		for (int i = 0; i < filter.size(); i++)
		{
			allFilters += filter[i];
			if (i + 1 < filter.size())
			{
				allFilters += "\0";
			}
		}
		LPCSTR FILTER = allFilters.c_str();

		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Window::s_CurrentFocusedWindow->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = FILTER;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		return std::nullopt;
	}

	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
	{
		if (uMsg == BFFM_INITIALIZED)
		{
			String tmp = (const char*)lpData;
			//std::cout << "path: " << tmp << std::endl;
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		}

		return 0;
	}
	std::optional<String> Platform::ChoosePathDialog()
	{
		const String saved_path = std::filesystem::current_path().string();
		TCHAR path[MAX_PATH];

		const char* path_param = saved_path.c_str();

		BROWSEINFO bi = { 0 };
		bi.lpszTitle = L"Browse for folder...";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM)path_param;

		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

		if (pidl != 0)
		{
			//get the name of the folder and put it in path
			SHGetPathFromIDList(pidl, path);

			//free memory used
			IMalloc* imalloc = 0;
			if (SUCCEEDED(SHGetMalloc(&imalloc)))
			{
				imalloc->Free(pidl);
				imalloc->Release();
			}

			std::wstring ws = std::basic_string<TCHAR>(path);
			String str(ws.begin(), ws.end());
			return str;
		}

		return std::nullopt;
	}
	
	void Platform::WriteToClipboard(const String& str)
	{
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, str.length());
		memcpy(GlobalLock(hMem), str.c_str(), str.length());
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
		GlobalFree(hMem);
	}
	String Platform::ReadFromClipboard()
	{
		if (OpenClipboard(NULL))
		{
			const HANDLE hglb = GetClipboardData(CF_TEXT);

			if (hglb != NULL)
			{
				const char* lptstr = (const char*)GlobalLock(hglb);

				if (lptstr != NULL)
				{
					GlobalUnlock(hglb);
					CloseClipboard();
					return lptstr;
				}
			}

			CloseClipboard();
		}

		return "";
	}

	void Platform::ShowInExplorer(const std::filesystem::path& folderPath)
	{
		if (std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath))
		{
			ShellExecuteA(NULL, "open", folderPath.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
		}
	}

	void Platform::CreateDirectory(const std::filesystem::path& folderPath)
	{
		std::filesystem::create_directories(folderPath);
	}

	void Platform::CopyDirectory(const std::filesystem::path& src, const std::filesystem::path& dest)
	{
		if (std::filesystem::exists(src))
		{
			CreateDirectory(dest);
			std::filesystem::copy(src, dest, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
		}
	}

	void Platform::RemoveFile(const std::filesystem::path& filePath)
	{
		if (std::filesystem::exists(filePath))
		{
			std::filesystem::remove(filePath);
		}
	}

	void Platform::OpenFileExternally(const String& filePath)
	{
		wchar_t wtext[512];
		mbstowcs(wtext, filePath.c_str(), filePath.length());//includes null
		LPWSTR ptr = wtext;
		ShellExecute(0, 0, ptr, 0, 0, SW_SHOW);
	}

	void Platform::CommandLine(const String& input)
	{
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE; // Hide the console window

		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));

		wchar_t  cmd[1024];
		swprintf(cmd, 1024, L"C:\\Windows\\System32\\cmd.exe /c %s", string_to_wstring(input).c_str());

		if (!CreateProcess(NULL,   // No module name (use command line)
			cmd,    // Command line
			NULL,   // Process handle not inheritable
			NULL,   // Thread handle not inheritable
			FALSE,  // Set handle inheritance to FALSE
			0,      // No creation flags
			NULL,   // Use parent's environment block
			NULL,   // Use parent's starting directory 
			&si,    // Pointer to STARTUPINFO structure
			&pi))   // Pointer to PROCESS_INFORMATION structure
		{
			SUORA_LOG(LogCategory::Core, LogLevel::Error, "CreateProcess failed ({0}).", GetLastError());
			return;
		}

		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}