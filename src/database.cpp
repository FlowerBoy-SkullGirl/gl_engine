#include "headers/database.h"
#include <iostream>
#include <stdlib.h>


/*
 * Database managment
 */
// Create a database file with a path/filename, returns NULL if failed
struct gl_db *create_database(const char *filen)
{
	struct gl_db *db =(struct gl_db *)malloc(sizeof(struct gl_db));
	if (db == NULL)
		return NULL;

	db->db_file = NULL;

	// Check if the database already exists, if so, return NULL
	FILE *fp = fopen(filen, "r");
	if (fp != NULL){
		return NULL;
	}

	// fp is NULL, so no file is open, we may create a new file for writing and reading
	fp = fopen(filen, "w+");
	// If creating the file failed, return NULL
	if (fp == NULL){
		return NULL;
	}

	//File was created successfully and is open for writing and reading
	db->db_file = fp;
	return db;
}

// Open a database from a path/filename
struct gl_db *open_database(const char *filen)
{
	struct gl_db *db =(struct gl_db *)malloc(sizeof(struct gl_db));
	if (db == NULL)
		return NULL;

	// Attempt to open the file for reading and writing
	FILE *fp = fopen(filen, "r+");
	// If opening the file failed, return NULL
	if (fp == NULL){
		return NULL;
	}

	// If fp is not null, the file is open, return
	db->db_file = fp;
	return db;
}

// Close a database
void close_database(struct gl_db *db)
{
	// Return if database is not open
	if (db == NULL)
		return;

	FILE *fp = db->db_file;
	// Close the database file if the filepointer is not null
	if (fp != NULL)
		fclose(fp);

	// Free db, which is not null
	free(db);
	return;
}

// Delete a database
// Wrapper function for remove()
int delete_database(const char* filen)
{
	return remove(filen);
}

// Get number of tables from a database, returns 0 if database is not open
int get_table_count(struct gl_db *db)
{
	int table_count = 0;
	char c;
	char *buffer;
	// Return early if database is not open
	if(db->db_file == NULL)
		return 0;

	// Determine the maximum number of digits to be read
	/* There will always be at least 1 digit needed to read a value
	 * of 0. Each time the maximum value can be divided by 10,
	 * add an additional digit 
	 */
	int digits = 1;
	int max = DB_MAX_TABLES;
	for(; max > 0; digits++){
		max /= 10;
	}
	// Allocate a buffer that can hold the maximum number of plain text digits required
	// Sizeof(char) should be 1, but is added in case this is different
	buffer = (char *) malloc((sizeof(char))*digits);

	// Return 0 if buffer cannot be allocated
	if (buffer == NULL)
		return 0;

	// Place file pointer at beginning of database file, where table count is guaranteed to be
	fseek(db->db_file, 0, SEEK_SET);

	int buffer_offset = 0;
	while((c = fgetc(db->db_file)) != EOF){
		// If we are reading further than the size of our buffer, return 0
		if (buffer_offset > (sizeof(char))*digits)
			return 0;
		// If the beginning of a table is encountered, break from loop
		if (c == DB_TABLE_START)
			break;
		// All characters preceding the first table should be the integer table count
		*(buffer + buffer_offset) = c;
		buffer_offset++;
	}
	// Use sscanf to parse an integer from the buffer, then return it
	sscanf(buffer, "%d", &table_count);
	return table_count;
}


/*
 *Table management
 */
// Create a table in the database with a string name and specify database, returns table id
int add_table_to_db(const char*, struct gl_db *);

// Get table id from string name
int get_table_id(const char *, struct gl_db *);

// Get count of columns in table
int get_column_count(int, struct gl_db *);

// Get row count from table
int get_row_count(int, struct gl_db *);

// Get table column data types
DB_TYPES *get_data_types_list(int, struct gl_db *);

// Remove a table from the database
void remove_table_from_db(int, struct gl_db *);

/*
 *Column management
 */
// Add a column in a table with a string identifier, specify table id and database
int add_column_to_table(const char*, int, struct gl_db *);

// Get the index of a column from the string name, table id, and database
int get_column_index(const char*, int, struct gl_db *);

// Remove a column from a table using the string name, table id, and database
void remove_column_from_table(const char*, int, struct gl_db *);

// Remove a column from a table using the column index, table id, and database
void remove_column_from_table_by_index(int, int, struct gl_db *);

/*
 *Row management
 */
// Add a row to a table by providing a serialized object, the table id, and the database
int add_row_to_table(struct row_object, int, struct gl_db *);

// Remove a row from the table by providing the row index/id, the table id, and the database
int remove_row_from_table(int, int, struct gl_db *);

/*
 *Data management
 */
// Return the data from an entire row, provide the row index, table id, and database
struct row_object *get_row_data(int, int, struct gl_db *);

// Update row by providing struct object, row index, table id, and database
void update_row_data_at_index(struct row_object, int, int, struct gl_db *);

// Query an int value by indices, table id, and database
int get_int_from_database_index(int, int, int, struct gl_db *);

// Query a float value by indices, table id, and database
float get_float_from_database_index(int, int, int, struct gl_db *);

// Query a string value by indices, table id, and database
const char *get_string_from_database_index(int, int, int, struct gl_db *);

// Query an int value by row index and column name, table id, and database
int get_int_from_database(int, const char *, int, struct gl_db *);

// Query a float value by row index and column name, table id, and database
float get_float_from_database(int, const char *, int, struct gl_db *);

// Query a string value by row index and column name, table id, and database
const char *get_string_from_database(int, const char *, int, struct gl_db *);
