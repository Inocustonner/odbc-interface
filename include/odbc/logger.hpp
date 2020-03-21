#pragma once
#include <P7_Trace.h>
// include windows

namespace Odbc_Logger
{
	bool init_logger();
	void free_logger();

	void log(const wchar_t *format, ...);
	void log_err(const wchar_t *format, ...);
};
