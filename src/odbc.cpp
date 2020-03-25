#include <odbc/odbc.hpp>
#include <type_traits>
#include <cstdlib>
#include <string>
// #define LOG(format, ...) Odbc_Logger::pTrace->P7_TRACE(L"[\n" format L"\n]", __VA_ARGS__)
// #define ERR_LOG(format, ...) Odbc_Logger::pTrace->P7_ERROR(L"[\n" format L"\n]", __VA_ARGS__)
// #define LOG_LINE(format, ...) LOG(L"\t" format, __VA_ARGS__)

#define TRY_SQL_CLASS(h, ht, x, comment, after_err_handler)	\
	{ \
		RETCODE rc = x; \
		if (rc == SQL_SUCCESS_WITH_INFO) \
		{ \
			std_sqltrace_handler(h, ht, x, comment, &sql_status_code);	\
		} \
		if (rc == SQL_ERROR) \
		{ \
			std_sqlerr_handler(h, ht, x, comment, &sql_status_code); \
			after_err_handler; \
		} \
	}

#define TRY_SQL TRY_SQL_CLASS
static SQLHENV hEnv= NULL;

std::string handle_diag_rec(SQLHANDLE hsql, SQLSMALLINT htype, RETCODE ret_code, SQLINTEGER *status_code = nullptr)
{
	if (ret_code == SQL_INVALID_HANDLE)
	{
		// report_err("Invalid handle\n");
		return std::string("\t\t" "Invalid handle\n");
	}
	std::string diag_rec = "";
	diag_rec.reserve(1000);

	SQLSMALLINT rec_i = 1;
	SQLINTEGER error_i;
	CHAR message_sz[1000] = {};
	CHAR state_sz[SQL_SQLSTATE_SIZE + 1] = {};
	SQLSMALLINT read;
	while (SQLGetDiagRec(htype,
						 hsql,
						 rec_i++,
						 (SQLCHAR*)state_sz,
						 &error_i,
						 (SQLCHAR*)message_sz,
						 (SQLSMALLINT)std::size(message_sz),
						 &read) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (strncmp(state_sz, "01004", 5))
		{
			CHAR format_buff[1000] = {};
			std::snprintf(format_buff, std::size(format_buff), "\t\t" "[ %5.5s ] %s (%d)\n", state_sz, message_sz, error_i);
			diag_rec += std::string(format_buff);
			// report_err("[ %5.5s ] %s (%d)\n", state_wsz, message_wsz, error_i);
		}
	}
	if (status_code)
		*status_code = atoi((const char*)state_sz);

	// TODO: replace with more efficient variant
	if (std::size(diag_rec))
		diag_rec.pop_back(); // remove last '\n'
	return diag_rec;
}


void std_sqltrace_handler(SQLHANDLE hsql, SQLSMALLINT htype, RETCODE ret_code, const wchar_t *line_msg_wstr = nullptr, SQLINTEGER *status_code = nullptr)
{
	std::wstring trace_msg;
	trace_msg.reserve(1000);

	if (line_msg_wstr)
	{
		trace_msg += L"\tComment:\n" L"\t\t" + std::wstring(line_msg_wstr) + L'\n';
	}

	std::string diag_rec_str = handle_diag_rec(hsql, htype, ret_code, status_code);
	if (diag_rec_str.length())
	{
		trace_msg += L"\t" L"DiagRec:\n";
		trace_msg.resize(trace_msg.length() + diag_rec_str.length());

		size_t converted;
		mbstowcs_s(&converted, (wchar_t*)trace_msg.data(), trace_msg.length(), diag_rec_str.data(), _TRUNCATE);
	}
	LOG(L"%s", trace_msg.c_str());
}


void std_sqlerr_handler(SQLHANDLE hsql, SQLSMALLINT htype, RETCODE ret_code, const wchar_t *line_msg_wstr = nullptr, SQLINTEGER *status_code = nullptr)
{
	std::wstring error_msg;
	error_msg.reserve(1000);

	if (line_msg_wstr)
	{
		error_msg += L"\tComment:\n" L"\t\t" + std::wstring(line_msg_wstr) + L'\n';
	}

	std::string diag_rec_str = handle_diag_rec(hsql, htype, ret_code, status_code);
	if (diag_rec_str.length())
	{
		error_msg += L"\t" L"DiagRec:\n";
		error_msg.resize(error_msg.length() + diag_rec_str.length());

		size_t converted;
		mbstowcs_s(&converted, (wchar_t*)error_msg.data(), error_msg.length(), diag_rec_str.data(), _TRUNCATE);
	}
	ERR_LOG(L"%s", error_msg.c_str());
}


bool init_env()
{
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR)
		return false;
	else
	{
		if (SQLSetEnvAttr(hEnv,
						  SQL_ATTR_ODBC_VERSION,
						  (SQLPOINTER)SQL_OV_ODBC3,
						  0) == SQL_ERROR)
		{
			return false;
		}
		else
		{
			return Odbc_Logger::init_logger();
		}
		
	}
}


void free_env()
{
	if (hEnv)
		SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
	Odbc_Logger::free_logger();
}


size_t sql_ctype_size(SQLSMALLINT ctype)
{
	switch (ctype)
	{
		case SQL_C_CHAR:		return sizeof(sql_ctype_t<SQL_C_CHAR>);
		case SQL_C_WCHAR:		return sizeof(sql_ctype_t<SQL_C_WCHAR>);
		case SQL_C_SSHORT:		return sizeof(sql_ctype_t<SQL_C_SSHORT>);
		case SQL_C_USHORT:		return sizeof(sql_ctype_t<SQL_C_USHORT>);
		case SQL_C_SLONG:		return sizeof(sql_ctype_t<SQL_C_SLONG>);
		case SQL_C_ULONG:		return sizeof(sql_ctype_t<SQL_C_ULONG>);
		case SQL_C_FLOAT:		return sizeof(sql_ctype_t<SQL_C_FLOAT>);
		case SQL_C_DOUBLE:		return sizeof(sql_ctype_t<SQL_C_DOUBLE>);
		case SQL_C_BIT:			return sizeof(sql_ctype_t<SQL_C_BIT>);
		case SQL_C_STINYINT:	return sizeof(sql_ctype_t<SQL_C_STINYINT>);
		case SQL_C_UTINYINT:	return sizeof(sql_ctype_t<SQL_C_UTINYINT>);
		case SQL_C_SBIGINT:		return sizeof(sql_ctype_t<SQL_C_SBIGINT>);
		case SQL_C_UBIGINT:		return sizeof(sql_ctype_t<SQL_C_UBIGINT>);
		case SQL_C_BINARY:		return sizeof(sql_ctype_t<SQL_C_BINARY>);
		default:
			ERR_LOG(L"Undefined SQL_C_TYPE %X", ctype);
			// throw error
			return 0;
	}
}


SQLSMALLINT sql_type_ctype(SQLSMALLINT type)
{
#define SQLTYPE_CTYPE(sqlt, ct) case sqlt: return ct
	switch (type)
	{
		SQLTYPE_CTYPE(SQL_CHAR, SQL_C_CHAR);
		SQLTYPE_CTYPE(SQL_VARCHAR, SQL_C_CHAR);
		SQLTYPE_CTYPE(SQL_LONGVARCHAR, SQL_C_CHAR);
		SQLTYPE_CTYPE(SQL_WCHAR, SQL_C_WCHAR);
		SQLTYPE_CTYPE(SQL_WVARCHAR, SQL_C_WCHAR);
		SQLTYPE_CTYPE(SQL_WLONGVARCHAR, SQL_C_WCHAR);
		SQLTYPE_CTYPE(SQL_DECIMAL, SQL_C_SBIGINT);
		SQLTYPE_CTYPE(SQL_NUMERIC, SQL_C_SLONG);
		SQLTYPE_CTYPE(SQL_SMALLINT, SQL_C_SSHORT);
		SQLTYPE_CTYPE(SQL_INTEGER, SQL_C_SLONG);
		SQLTYPE_CTYPE(SQL_REAL, SQL_C_FLOAT);
		SQLTYPE_CTYPE(SQL_FLOAT, SQL_C_DOUBLE);
		SQLTYPE_CTYPE(SQL_DOUBLE, SQL_C_DOUBLE);
		SQLTYPE_CTYPE(SQL_BIT, SQL_C_BIT);
		SQLTYPE_CTYPE(SQL_TINYINT, SQL_C_STINYINT);
		SQLTYPE_CTYPE(SQL_BIGINT, SQL_C_SBIGINT);
		SQLTYPE_CTYPE(SQL_BINARY, SQL_C_BINARY);
		SQLTYPE_CTYPE(SQL_VARBINARY, SQL_C_BINARY);
		SQLTYPE_CTYPE(SQL_LONGVARBINARY, SQL_C_BINARY);
		// no support yet
		// SQLTYPE_CTYPE(SQL_TYPE_DATE[6], );
		// SQLTYPE_CTYPE(SQL_TYPE_TIME[6], );
		// SQLTYPE_CTYPE(SQL_TYPE_TIMESTAMP[6], );
		// SQLTYPE_CTYPE(SQL_TYPE_UTCDATETIME, );
		// SQLTYPE_CTYPE(SQL_TYPE_UTCTIME, );
		// SQLTYPE_CTYPE(SQL_INTERVAL_MONTH[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_YEAR[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_YEAR_TO_MONTH[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_DAY[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_HOUR[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_MINUTE[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_SECOND[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_DAY_TO_HOUR[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_DAY_TO_MINUTE[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_DAY_TO_SECOND[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_HOUR_TO_MINUTE[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_HOUR_TO_SECOND[7], );
		// SQLTYPE_CTYPE(SQL_INTERVAL_MINUTE_TO_SECOND[7], );
		// SQLTYPE_CTYPE(SQL_GUID, );
		default:
			// error log
			return 0;
	}
}


/* ------------------------------------------------- */
// struct Stmt
/* ------------------------------------------------- */


Stmt::Stmt(HSTMT hStmt) : hStmt(hStmt) {}


Stmt::Stmt(HSTMT hStmt, const char *query) : hStmt(hStmt), query(query) {}


Stmt::~Stmt()
{
	// last_retcode = SQLFreeStmt(hStmt, SQL_CLOSE);
	// if (last_retcode == SQL_SUCCESS ||
	// 	last_retcode == SQL_SUCCESS_WITH_INFO)
	// {
	// 	// do mb something
	// }
	// else
	// {
	// 	// error log
	// }
	TRY_SQL(hStmt, SQL_HANDLE_STMT,
			SQLFreeStmt(hStmt, SQL_CLOSE), nullptr,
			last_retcode = rc);
}

void Stmt::set_query(const char *query)
{
	this->query = query;
}


RETCODE Stmt::exec()
{
	cols_cnt = 0;
	TRY_SQL(hStmt, SQL_HANDLE_STMT,
			SQLExecDirect(hStmt, (SQLCHAR*)query, SQL_NTS), nullptr,
				last_retcode = rc);
	return last_retcode;
}


SQLSMALLINT Stmt::get_cols_cnt()
{
	if (!cols_cnt)
	{
		TRY_SQL(hStmt, SQL_HANDLE_STMT,
				SQLNumResultCols(hStmt, &cols_cnt), nullptr,
				last_retcode = rc);
	}
	return cols_cnt;
}


SQLLEN Stmt::get_rows_cnt()
{
	if (!rows_cnt)
	{
		TRY_SQL(hStmt, SQL_HANDLE_STMT,
				SQLRowCount(hStmt, &rows_cnt), nullptr,
				last_retcode = rc);
	}
	return rows_cnt;
}


bool Stmt::alloc_odbc_data()
{
	for (SQLSMALLINT col = 1; col <= cols_cnt; ++col)
	{
		SQLLEN sql_type_l;
		SQLLEN data_size = 0;
		TRY_SQL(hStmt, SQL_HANDLE_STMT,
				last_retcode = SQLColAttribute(hStmt,
											   col,
											   SQL_DESC_CONCISE_TYPE,
											   nullptr,
											   0,
											   nullptr,
											   &sql_type_l),
				L"Failed to get SQLColAttribute",
				return false);
		
		// if this is string type then obtain it's length info
		if (sql_type_l == SQL_C_CHAR ||
			sql_type_l == SQL_C_WCHAR ||
			sql_type_l == SQL_C_BINARY)
		{
			TRY_SQL(hStmt, SQL_HANDLE_STMT,
					last_retcode = SQLColAttribute(hStmt,
												   col,
												   SQL_DESC_LENGTH,
												   nullptr,
												   0,
												   nullptr,
												   &data_size),
					L"Failed to get length of string column with SQLColAttribute",
					return false);
			data_size *= sql_type_l == SQL_C_WCHAR ? sizeof(wchar_t) : sizeof(char);
		}
		else
		{
			data_size = sql_ctype_size(sql_type_ctype((SQLSMALLINT)sql_type_l));
		}
		bindings_cols.emplace_back();
		// allocate and bind
		bindings_cols[col - 1].data = calloc(1, data_size);
		bindings_cols[col - 1].ind_ptr = (SQLLEN*)calloc(1, sizeof(SQLLEN));
		bindings_cols[col - 1].type = (SQLSMALLINT)sql_type_l;
		bindings_cols[col - 1].size = data_size;
		TRY_SQL(hStmt, SQL_HANDLE_STMT,
				last_retcode = SQLBindCol(hStmt,
										  col,
										  (SQLSMALLINT)sql_type_l,
										  bindings_cols[col - 1].data,
										  data_size,
										  bindings_cols[col - 1].ind_ptr),
				L"Failed to accomplish bind via SQLBindCol",
				return false);
	}
	return are_bindings_alloc = true;
}


Data_Matrix_t Stmt::get_all_rows()
{
	if (cols_cnt == 0)
	{
		get_cols_cnt();
		if (last_retcode == SQL_ERROR)
		{
			// error log
			return Data_Matrix_t{};
		}
	}

	if (rows_cnt == 0)
	{
		get_rows_cnt();
		if (last_retcode == SQL_ERROR)
		{
			// error log
			return Data_Matrix_t{};
		}		
	}


	Data_Matrix_t data_mx;
	data_mx.reserve(rows_cnt);
	bindings_cols.reserve(cols_cnt);
	if (!are_bindings_alloc)
	{
		alloc_odbc_data();
	}
		// copy every row of data to data_mx and return it
	for (SQLLEN row = 0; row < rows_cnt; ++row)
	{
		last_retcode = SQLFetch(hStmt);

		if (last_retcode == SQL_ERROR)
		{
			// error log
			ERR_LOG(L"SQLFetch failed");
			return data_mx;
		}
		else if (last_retcode == SQL_NO_DATA_FOUND)
		{
			break;
		}
		else
		{
			data_mx.emplace_back(cols_cnt);
			for (SQLSMALLINT col = 0; col < cols_cnt; ++col)
			{
				void *data = calloc(1, bindings_cols[col].size);
				if (!data)
				{
					// error log
					break;
				}
				std::memcpy(data, bindings_cols[col].get_data(), bindings_cols[col].size);
				data_mx[row][col] = Odbc_Data(bindings_cols[col].type, bindings_cols[col].size, data);
			}
		}
	}

	return data_mx;
}


RETCODE Stmt::get_last_retcode() const noexcept
{
	return last_retcode;
}

SQLINTEGER Stmt::get_status_code() const noexcept
{
	return sql_status_code;
}

/* ------------------------------------------------- */
// struct Odbc
/* ------------------------------------------------- */

Odbc::Odbc(const char *connection_string) : connection_string(connection_string) {}


void Odbc::set_connection_string(const char *connection_string)
{
	this->connection_string = connection_string;
}

Odbc::~Odbc()
{
	if (stmt)
	{
		delete stmt;
		stmt = nullptr;
	}
	if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	if (hDbc) SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
}


bool Odbc::connect()
{
	if (connected)
	{
	    ERR_LOG(L"Connetion was already set");
		return false;
	}
	
	if (!connection_string)
	{
		// report no connection string
		ERR_LOG("No connection string provided");
		return false;
	}
	
	TRY_SQL(hEnv, SQL_HANDLE_ENV,
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc),
			L"Failed to alloc SQL_HANDLE_DBC",
			return false);

	TRY_SQL(hDbc, SQL_HANDLE_DBC,
			last_retcode = SQLDriverConnect(hDbc,
											NULL,
											(SQLCHAR*)connection_string,
											SQL_NTS,
											NULL,
											0,
											NULL,
											SQL_DRIVER_NOPROMPT),
			L"Failed to set driver connection",
			return false);			
	connected = true;

	// alloc hStmt
	TRY_SQL(hDbc, SQL_HANDLE_DBC,
			last_retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt),
			L"Failed to allocate SQL_HANDLE_STMT",
			hStmt = NULL);

	return connected;
}


Stmt *Odbc::exec_query(const char *query)
{
	if (stmt)
	{
		delete stmt;
		stmt = nullptr;
	}

	if (hStmt)
	{
		stmt = new Stmt(hStmt, query);
		// TRY_SQL(hStmt, SQL_HANDLE_STMT,
		// 		stmt->exec(),
		// 		L"Failed to execute statement",
		// 		return nullptr);
		last_retcode = stmt->exec();
		sql_status_code = stmt->get_status_code();
		
		if (last_retcode != SQL_ERROR)
			return stmt;
		else
		{
			return nullptr;	
		}
	}
	else
	{
		// if still is not allocated allocate
		return nullptr;			// temporary solution
	}
}


RETCODE Odbc::get_last_retcode() const noexcept
{
	return last_retcode;
}

SQLINTEGER Odbc::get_status_code() const noexcept
{
	return sql_status_code;
}
