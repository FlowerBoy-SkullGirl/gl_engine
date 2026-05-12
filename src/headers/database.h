#ifndef DATABASE_H
#define DATABASE_H

/*
 *  Structure of the database:
 *  The first value in the database is a count of tables, the second value is a count of assigned table id's, 
 *  every preceding value is a table
 *  Each table has by default an integer id, a row id count, a row count, a column count, 
 *    a name, a list of column names, and a list of column data types
 *  Tables are enclosed in braces {}
 *  Tables are separated by commas ,
 *  A row is occupied by 1 or more columns, the first column being an integer id, which is a key value
 *  Rows are enclosed in braces {} with column values being separated by commas within ,
 *  Rows are separated by commas ,
 *  Column name/data type lists are enclosed in braces {} and appear similar to the first row in a table, 
 *    however they do not contain an id value, and only store string values
 *  Newlines are optional and all whitespace is ignored unless inside a column value or name string
 *  A table may be visualized like so:
 *  {Table: id_val,row_id_count,row_count,col_count,"name",{"col_names"},{col1_data_type,col2_data_type,},{row: id_val,col2_val,col3_val,...}}
 *  Or like so:
 *  {Table: id_val, row_id_count, row_count, col_count, "name", {"col_names"}, {col1_data_type, col2_data_type, ...},
 *      {row: id_val, col2_val, col3_val},
 *      {row: id_val, col2_val, col3_val}
 *  }
 */

// Max size allowed for different elements
#define DB_MAX_TABLES 16
#define DB_MAX_ROWS 256
#define DB_MAX_COLS 256
#define DB_MAX_SIZE_STRING 256
/*
 * A table is constrained to size (256 ^ 3)(max string, in max rows, in max columns)
 *                              + 2 (table braces) + 512(row braces) 
 *                              + (256 * 255) + 255 + 5(maximum number of commas)
 * which is 2 ^ 24 characters + 514 + ~(2^16), which is a size that can be stored in any data type which can represent 25 bits
 * Multiplied by 16 (2^4), at least 29 bits are needed to store the size of a file with 16 tables
 * Stdlib functions typically use a long value as an offset count from the beginning of a file, so
 * alternative functions like fseeko should be used where possible to avoid overflow,
 * since long is signed and is only guaranteed to hold 32 bit values
 */

// Delimiters and boundary markers
#define DB_TABLE_START '{'
#define DB_TABLE_END '}'
#define DB_ROW_START '{'
#define DB_ROW_END '}'
#define DB_DELIMITER ','
#define START_INDEX '0'
#define DB_WHITESPACE ' '
#define DB_KEY_NAME "id"

struct gl_db{
	FILE *db_file;
};

// An enum for allowed data types in the database
enum DB_TYPES {DB_INT, DB_FLOAT, DB_STRING};

// Abstract serialization of object data into a database row
// Implementation of moving data into the struct will be up to the program using the API
struct row_object{
	int column_count;
	void *data_list;
	size_t data_list_size;
	enum DB_TYPES *data_type_list;
};

// Database containers metadata structs to help manage metadata queries
struct database_metadata{
	int num_tables;
	int newest_table_id;
};

struct table_metadata{
	int id;
	int num_rows;
	int newest_row_id;
	int num_cols;
};

/*
 * Database managment
 */
// Create a database file with a path/filename
struct gl_db *create_database(const char*);

// Open a database from a path/filename
struct gl_db *open_database(const char*);

// Close a database
void close_database(struct gl_db *);

// Delete a database
int delete_database(const char *);

// Get number of tables and table id count from a database
struct database_metadata get_database_metadata(struct gl_db *);

// Write the number of tables and table id count into the database file
int write_database_metadata(struct database_metadata, struct gl_db *db)

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
enum DB_TYPES *get_data_types_list(int, struct gl_db *);

// Remove a table from the database
void remove_table_from_db(int, struct gl_db *);

/*
 *Column management
 */
// Add a column in a table with a string identifier and type specification ,specify table id and database
int add_column_to_table(const char*, enum DB_TYPES, int, struct gl_db *);

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

/*
 *Helper functions
 */

#endif
