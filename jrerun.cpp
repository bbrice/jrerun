#include "targetver.h"
#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <atlbase.h>

////////////////////////////////////////////////////////////////////////////////

static const size_t g_max_property_length = 256;

////////////////////////////////////////////////////////////////////////////////

static void execute(const std::wstring& cmdline)
{
	std::unique_ptr<wchar_t[]> p_cmdline = std::make_unique<wchar_t[]>(cmdline.length() + 1);
	wcscpy(p_cmdline.get(), cmdline.c_str());

	STARTUPINFO         si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi = { NULL };
	
	if (CreateProcess(NULL, p_cmdline.get(), NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi) != FALSE)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
}

static bool get_reg_value(CRegKey& key, const std::wstring& value_name, std::wstring& value)
{
	ULONG value_size = 0;
	if (key.QueryStringValue(value_name.c_str(), NULL, &value_size) != ERROR_SUCCESS)
		return false;

	value.resize(value_size);

	if (key.QueryStringValue(value_name.c_str(), &value[0], &value_size) != ERROR_SUCCESS)
		return false;

	if (value.length() > 0 && value[value.length() - 1] == L'\0')
		value.resize(value_size - 1);

	return true;
}

static bool find_jre(std::wstring& jre_path)
{
	CRegKey jre;
	if (jre.Open(HKEY_LOCAL_MACHINE, L"Software\\JavaSoft\\Java Runtime Environment", KEY_READ) != ERROR_SUCCESS)
	{
		BOOL is_64bit_win;
		if (IsWow64Process(GetCurrentProcess(), &is_64bit_win) == FALSE)
			return false;

		const REGSAM wow64_flag = (is_64bit_win ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);
		if (jre.Open(HKEY_LOCAL_MACHINE, L"Software\\JavaSoft\\Java Runtime Environment", KEY_READ | wow64_flag) != ERROR_SUCCESS)
			return false;
	}

	std::wstring jre_version;
	if (!get_reg_value(jre, L"CurrentVersion", jre_version))
		return false;

	CRegKey jre_current;
	if (jre_current.Open(jre, jre_version.c_str(), KEY_READ) != ERROR_SUCCESS)
		return false;

	std::wstring java_home;
	if (!get_reg_value(jre_current, L"JavaHome", jre_path))
		return false;

	if (!jre_path.empty() && jre_path[jre_path.length() - 1] != L'\\')
		jre_path += L'\\';

	jre_path += L"bin\\javaw.exe";
	return true;
}

static bool get_exe_path(std::wstring& exe_path)
{
	const size_t max_size_increment = MAX_PATH;
	std::vector<wchar_t> exe_path_buf(max_size_increment);

	DWORD size = GetModuleFileName(NULL, exe_path_buf.data(), static_cast<DWORD>(exe_path_buf.size()));
	for (DWORD err = GetLastError(); err == ERROR_INSUFFICIENT_BUFFER; err = GetLastError())
	{
		exe_path_buf.resize(exe_path_buf.size() + max_size_increment);
		size = GetModuleFileName(NULL, exe_path_buf.data(), static_cast<DWORD>(exe_path_buf.size()));
	}

	if (size > 0)
		exe_path.assign(exe_path_buf.begin(), exe_path_buf.begin() + size);

	return exe_path.length() > 0;
}

static bool get_property(const std::wstring& properties_filename, const std::wstring& section, const std::wstring& name, std::wstring& value)
{
	const DWORD value_length = GetPrivateProfileString(section.c_str(), name.c_str(), NULL, &value[0], static_cast<DWORD>(value.size()), properties_filename.c_str());
	value.resize(value_length);
	return value.length() > 0;
}

static void get_properties_filename(const std::wstring& exe_path, std::wstring& properties_path)
{
	const size_t dot_pos = exe_path.find_last_of(L".\\/");

	if (dot_pos != std::wstring::npos && exe_path[dot_pos] == L'.')
		properties_path = exe_path.substr(0, dot_pos);
	else
		properties_path = exe_path;

	properties_path += L".ini";
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	std::wstring exe_path;
	if (get_exe_path(exe_path))
	{
		std::wstring properties_path;
		get_properties_filename(exe_path, properties_path);

		std::wstring jre_path;
		if (find_jre(jre_path))
		{
			std::wostringstream cmdline;
			cmdline << L'"' << jre_path << L'"';

			std::wstring args(g_max_property_length, L'\0');
			if (get_property(properties_path, L"app", L"args", args))
				cmdline << L' ' << args;

			std::wstring file(g_max_property_length, L'\0');
			if (get_property(properties_path, L"app", L"file", file))
				cmdline << L' ' << L'"' << file << L'"';

			if (lpCmdLine && lpCmdLine[0] != L'\0')
				cmdline << L' ' << lpCmdLine;

			execute(cmdline.str());
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
