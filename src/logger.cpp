#include <cstdio>
#include <cstdlib>
#include <odbc/logger.hpp>
#include <string>
#include <cstdarg>
std::wstring GetExePath()
{
	std::wstring path;
	path.resize(MAX_PATH + 1);
	size_t path_len = GetModuleFileNameW(NULL, path.data(), path.size());
	path.resize(path.find_last_of(L"\\"));
	for (auto it = std::begin(path); it != std::end(path); ++it)
	{
		if (*it == L'\\') *it = L'/';	
	}

	return path;
}


namespace Odbc_Logger
{
	extern IP7_Client *pClient = nullptr;
	extern IP7_Trace *pTrace = nullptr;
	extern IP7_Trace::hModule hModule = NULL;

	bool init_logger()
	{
		std::wstring path = GetExePath();
		// form args for logger
		std::wstring args_str = L"/P7.Sink=FileTxt /P7.Format=\"{%cn}[%tf] %lv:\n%ms\" ";
		args_str += L"P7.Dir=" + path + L"/log";

		// printf("args : %ls\n", args_str.c_str());

		P7_Set_Crash_Handler();

		pClient = P7_Create_Client(args_str.c_str());
		if (!pClient) return false;
		
		pTrace = P7_Create_Trace(pClient, TM("Odbc_Inf"));
		if (!pTrace)
		{
			free_logger();
			return false;
		}
		return true;
	}


	void free_logger()
	{
		if (pTrace) pTrace->Release();
		if (pClient) pClient->Release();
	}


	// void log(const wchar_t *format, ...)
	// {
	// 	va_list args;
	// 	va_start(args, format);
	// 	pTrace->P7_TRACE(hModule, format, args);
	// 	va_end(args);
	// }

	// void err_log(const wchar_t *format, ...)
	// {
	// 	va_list args;
	// 	va_start(args, format);
	// 	pTrace->P7_ERROR(hModule, format, args);
	// 	va_end(args);
	// }
};
