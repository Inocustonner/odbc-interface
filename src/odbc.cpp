#include <odbc/odbc.hpp>
#include <type_traits>
#include <cstdlib>

#define TRY_SQL_NATIVE(x, handle_err, after_handler) \
	{ \
		RETCODE rc = x; \
		if (rc != SQL_SUCCESS) \
		{ \
		} \
		if (rc == SQL_ERROR) \
		{ \
			handle_err; \
			after_handler; \
		} \
	}

#define TRY_SQL TRY_SQL_NATIVE
static SQLHENV hEnv= NULL;

void handle_diag_rec(SQLHANDLE hsql, SQLSMALLINT htype, RETCODE ret_code)
{

	SQLSMALLINT rec_i = 0;
	SQLINTEGER error_i;
	CHAR message_sz[1000] = {};
	CHAR state_sz[SQL_SQLSTATE_SIZE + 1] = {};
	
	if (ret_code == SQL_INVALID_HANDLE)
	{
		// report_err("Invalid handle\n");
		return;
	}
	
	while (SQLGetDiagRec(htype,
						 hsql,
						 rec_i++,
						 (SQLCHAR*)state_sz,
						 &error_i,
						 (SQLCHAR*)message_sz,
						 (SQLSMALLINT)std::size(message_sz),
						 nullptr) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (strncmp(state_sz, "01004", 5))
		{
			// report_err("[ %5.5s ] %s (%d)\n", state_wsz, message_wsz, error_i);
		}
	}	
}


void std_sqlerr_handler(SQLHANDLE hsql, SQLSMALLINT htype, RETCODE ret_code, const char *msg = nullptr)
{
	
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
			return true;		
		}
		
	}
}


void free_env()
{
	if (hEnv)
		SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
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
			// error log
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
	TRY_SQL(SQLFreeStmt(hStmt, SQL_CLOSE), handle_diag_rec(hStmt, SQL_HANDLE_STMT, rc), last_retcode = rc);
}

void Stmt::set_query(const char *query)
{
	this->query = query;
}


RETCODE Stmt::exec()
{
	cols_cnt = 0;
	last_retcode = SQLExecDirect(hStmt, (SQLCHAR*)query, SQL_NTS);
	return last_retcode;
}


SQLSMALLINT Stmt::get_cols_cnt()
{
	if (!cols_cnt)
		last_retcode = SQLNumResultCols(hStmt, &cols_cnt); // if error occ sets cols to 0
	return cols_cnt;
}


SQLLEN Stmt::get_rows_cnt()
{
	if (!rows_cnt)
		last_retcode = SQLRowCount(hStmt, &rows_cnt);
	return rows_cnt;
}


bool Stmt::alloc_odbc_data()
{
	for (SQLSMALLINT col = 1; col <= cols_cnt; ++col)
	{
		SQLSMALLINT sql_type_l;
		SQLLEN data_size = 0;
		last_retcode = SQLColAttribute(hStmt,
									   col,
									   SQL_DESC_CONCISE_TYPE,
									   nullptr,
									   0,
									   nullptr,
									   &sql_type_l);

		if (last_retcode == SQL_SUCCESS
			|| last_retcode == SQL_SUCCESS_WITH_INFO)
		{
			// if this is string type then obtain it's length info
			if (sql_type_l == SQL_C_CHAR ||
				sql_type_l == SQL_C_WCHAR ||
				sql_type_l == SQL_C_BINARY)
			{
				last_retcode = SQLColAttribute(hStmt,
											   col,
											   SQL_DESC_LENGTH,
											   nullptr,
											   0,
											   nullptr,
											   &data_size);
				if (last_retcode == SQL_SUCCESS ||
					last_retcode == SQL_SUCCESS_WITH_INFO)
				{
					// data_size *= sql_ctype_size(sql_type_l);
					data_size *= sql_type_l == SQL_C_WCHAR ? sizeof(wchar_t) : sizeof(char);
				}
				else
				{
					// error log
					return false;
				}
			}
			else
			{
				data_size = sql_ctype_size(sql_type_ctype(sql_type_l));
			}
			bindings_cols.emplace_back();
			// allocate and bind
			bindings_cols[col - 1].data = calloc(1, data_size);
			bindings_cols[col - 1].ind_ptr = (SQLLEN*)calloc(1, sizeof(SQLLEN));
			bindings_cols[col - 1].type = sql_type_l;
			bindings_cols[col - 1].size = data_size;

			last_retcode = SQLBindCol(hStmt,
									  col,
									  sql_type_l,
									  bindings_cols[col - 1].data,
									  data_size,
									  bindings_cols[col - 1].ind_ptr);
			if (last_retcode == SQL_SUCCESS ||
				last_retcode == SQL_SUCCESS_WITH_INFO)
			{
				// may be display info if sql success with info
			}
			else
			{
				// error log
				return false;
			}
		}
		else
		{
			// error log
			return false;
		}
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


RETCODE Stmt::get_last_retcode()
{
	return last_retcode;
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
	if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	if (hDbc) SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
}


bool Odbc::connect()
{
	if (connected)
	{
		// report already conneted
		return false;
	}
	
	if (!connection_string)
	{
		// report no connection string
		return false;
	}

	if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) == SQL_ERROR)
	{
		// error log
		return false;
	}

	last_retcode = SQLDriverConnect(hDbc,
									NULL,
									(SQLCHAR*)connection_string,
									SQL_NTS,
									NULL,
									0,
									NULL,
									SQL_DRIVER_NOPROMPT); // make prompt option possible

	if (last_retcode == SQL_SUCCESS ||
		last_retcode == SQL_SUCCESS_WITH_INFO)
	{
		connected = true;
	}
	else
	{
		return false;
	}

	// alloc hStmt
	last_retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
	if (last_retcode == SQL_ERROR)
		hStmt = NULL;

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
		stmt->exec();
		if (stmt->get_last_retcode() == SQL_SUCCESS ||
			stmt->get_last_retcode() == SQL_SUCCESS_WITH_INFO)
		{
			// handle log if with_info
			return stmt;
		}
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
