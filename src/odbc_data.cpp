#include <odbc/odbc_data.hpp>
#include <cstdlib>
#include <cstdio>
Odbc_Data::Odbc_Data(Odbc_Data &&odata)
{
	this->data = odata.data;
	odata.data = nullptr;
}


Odbc_Data::Odbc_Data(SQLSMALLINT type, void *data) : type(type), data(data) {}


Odbc_Data::Odbc_Data(SQLSMALLINT type, SQLLEN size, void *data) : type(type), size(size), data(data) {}

Odbc_Data &Odbc_Data::operator=(Odbc_Data &&odata)
{
	this->data = odata.data;
	this->ind_ptr = odata.ind_ptr;
	this->type = odata.type;
	this->size = odata.size;
	odata.data = nullptr;
	odata.ind_ptr = nullptr;
	return *this;
}


Odbc_Data::~Odbc_Data()
{
#ifdef ODBC_DEBUG_DATA
	printf("odbc_data dctor : data %p, ind_ptr %p\n", data, ind_ptr);
#endif
	if (data) free(data);
	if (ind_ptr) free(ind_ptr);
}


void *Odbc_Data::get_data() const
{
	return data;
}
