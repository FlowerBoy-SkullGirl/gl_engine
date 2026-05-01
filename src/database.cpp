#include "headers/database.h"
#include <iostream>
#include <stdlib.h>


/*
 * Database managment
 */
// Create a database file with a path/filename, returns gl_db with a null value in db_file if failed
struct gl_db create_database(const char *filen)
{
	struct gl_db db;
	db.db_file = NULL;

	// Check if the database already exists, if so, return db with a null filepointer
	FILE *fp = fopen(filen, "r");
	if (fp != NULL){
		return db;
	}

	// fp is NULL, so no file is open, we may create a new file for writing and reading
	fp = fopen(filen, "w+");
	// If creating the file failed, return db with a null filepointer
	if (fp == NULL){
		return db;
	}

	//File was created successfully and is open for writing and reading
	db.db_file = fp;
	return db;
}

// Open a database from a path/filename
struct gl_db open_database(const char *filen)
{
	struct gl_db db;

	// Attempt to open the file for reading and writing
	FILE *fp = fopen(filen, "r+");
	// If opening the file failed, return db with a null filepointer
	if (fp == NULL){
		return db;
	}

	// If fp is not null, the file is open, return
	db.db_file = fp;
	return db;
}

// Close a database
void gl_db close_database(struct gl_db db)
{
	FILE *fp = db.db_file;
	fclose(fp
}

// Delete a database
void gl_db delete_database(struct gl_db);

// Get number of tables from a database
int get_table_count(struct gl_db);

/*
 *Table management
 */
// Create a table in the database with a string name and specify database, returns table id
int add_table_to_db(const char*, struct gl_db);

// Get table id from string name
int get_table_id(const char *, struct gl_db);

// Get count of columns in table
int get_column_count(int, struct gl_db);

// Get row count from table
int get_row_count(int, struct gl_db);

// Get table column data types
DB_TYPES *get_data_types_list(int, struct gl_db);

// Remove a table from the database
void remove_table_from_db(int, struct gl_db);

/*
 *Column management
 */
// Add a column in a table with a string identifier, specify table id and database
int add_column_to_table(const char*, int, struct gl_db);

// Get the index of a column from the string name, table id, and database
int get_column_index(const char*, int, struct gl_db);

// Remove a column from a table using the string name, table id, and database
void remove_column_from_table(const char*, int, struct gl_db);

// Remove a column from a table using the column index, table id, and database
void remove_column_from_table_by_index(int, int, struct gl_db);

/*
 *Row management
 */
// Add a row to a table by providing a serialized object, the table id, and the database
int add_row_to_table(struct row_object, int, struct gl_db);

// Remove a row from the table by providing the row index/id, the table id, and the database
int remove_row_from_table(int, int, struct gl_db);

/*
 *Data management
 */
// Return the data from an entire row, provide the row index, table id, and database
struct row_object *get_row_data(int, int, struct gl_db);

// Update row by providing struct object, row index, table id, and database
void update_row_data_at_index(struct row_object, int, int, struct gl_db);

// Query an int value by indices, table id, and database
int get_int_from_database_index(int, int, int, struct gl_db);

// Query a float value by indices, table id, and database
float get_float_from_database_index(int, int, int, struct gl_db);

// Query a string value by indices, table id, and database
const char *get_string_from_database_index(int, int, int, struct gl_db);

// Query an int value by row index and column name, table id, and database
int get_int_from_database(int, const char *, int, struct gl_db);

// Query a float value by row index and column name, table id, and database
float get_float_from_database(int, const char *, int, struct gl_db);

// Query a string value by row index and column name, table id, and database
const char *get_string_from_database(int, const char *, int, struct gl_db);
