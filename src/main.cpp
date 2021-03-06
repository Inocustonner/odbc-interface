#include <odbc/odbc.hpp>
#include <iostream>

void atp()
{
	Odbc bd;
	const char *conn_str = "DRIVER={PostgreSQL ANSI}; SERVER=localhost; PORT=5432; DATABASE=mydb; UID=postgres; PWD=root;";
	bd.set_connection_string(conn_str);
	if (bd.connect())
	{
		LOG_LINE(L"Connected");
		auto *stmt = bd.exec_query("SELECT * FROM test;");
		if (stmt == nullptr)
		{
			ERR_LOG(L"\tFailed to exec_query");
			return;
		}

		int cols = stmt->get_cols_cnt();
		int rows = stmt->get_rows_cnt();
		printf("cols : %d\nrows : %d\n", cols, rows);
		
		Data_Matrix_t data_rows = std::move(stmt->get_all_rows());
		
		if (data_rows.size() > 0)
		{
			for (const auto &data_row : data_rows)
			{
				for (auto it = std::cbegin(data_row); it != std::cend(data_row); ++it)
				{
					if (it->type == SQL_C_CHAR)
					{
						printf("%s\n", (const char*)it->get_data());
					}
					else
					{
						printf("%d | ", *(const int*)it->get_data());
					}
				}
			}
		}
		else
		{
			ERR_LOG(L"\tFailed to recive rows of data\n");
		}
	}
	else
	{
		ERR_LOG("\tConnection Failed");
		return;
	}
	LOG_LINE(L"Disconnected");
}

int main()
{
	// initializing
	init_env();

	// test case
	atp();

	// deinitializing
	free_env();
	return 0;
}
