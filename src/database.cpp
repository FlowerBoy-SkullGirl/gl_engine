#include <iostream>
#include <stdlib.h>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include "headers/database.h"

#define FILE_IO_BUFFER_SIZE 1

/*
 * File IO Helper Functions
 */

// Truncate a file after an offset and return the number of bytes truncated
off_t truncate_file_after(FILE *fp, off_t offset)
{
	// Return early if file pointer is null
	if (fp == NULL)
		return 0;

	// Return early if file is already smaller than or equal to requested offset
	off_t file_size = 0;
	fseeko(fp, 0, SEEK_END);

	file_size = ftello(fp);

	if (file_size <= offset)
		return 0;

	// Return file position to offset
	fseek(fp, offset + 1, SEEK_SET);

	// Find the remaining size that needs to be written
	off_t truncate_size = file_size - (offset + 1);

	// Fill the remaining space with whitespace characters
	for(int i = 0; i < truncate_size; i++){
		fputc(DB_WHITESPACE, fp);
	}

	return truncate_size;
}

// Create a temporary file with a specified name and return the file pointer
FILE *create_temp_file(const char *filen)
{
	FILE *fp = fopen(filen, "w+");
	return fp;
}

// Delete temp file, must be closed prior
int remove_temp_file(const char *filen)
{
	return remove(filen);
}

// Write data from first file to second file, starting from first offset to second offset
// Both file * must point to an open file
// Return number of bytes written
off_t f_copy_between(FILE *src, FILE *dest, off_t start, off_t end)
{
	// Used to check bounds of file
	off_t current = 0;
	off_t sum = 0;

	if (src == NULL || dest == NULL)
		return 0;

	fseeko(src, 0, SEEK_END);
	current = ftello(src);

	// Do not write any data if start is past file bounds
	if (start >= current)
		return 0;

	// Only write to the end of the file if end is past the file bounds
	if (end >= current)
		end = current;

	// How many times to iterate loop
	off_t loop_iterations = (end - start) / FILE_IO_BUFFER_SIZE;

	// How many bytes to read after loop finishes
	// This value is less than buffer size, so is guaranteed to fit into buffer
	off_t remainder = (end - start) % FILE_IO_BUFFER_SIZE;

	// Allocate memory for a buffer to read the src file
	unsigned char *buffer = (unsigned char *) malloc(FILE_IO_BUFFER_SIZE);

	if (buffer == NULL)
		return 0;
	
	// Set the file position to the start offset
	fseeko(src, start, SEEK_SET);

	// Begin reading from src and writing to dest
	for(int i = 0; i < loop_iterations; i++){
		fread(buffer, FILE_IO_BUFFER_SIZE, 1, src);
		sum += fwrite(buffer, FILE_IO_BUFFER_SIZE, 1, dest);
	}

	// If there are no more bytes to write, return
	if (remainder == 0){
		free(buffer);
		return sum;
	}

	// Clear buffer
	memset(buffer, 0, FILE_IO_BUFFER_SIZE);

	// Write the remaining bytes
	current = fread(buffer, 1, remainder, src);
	// Too many bytes have been read, abandon
	if (current != remainder){
		return sum;
	}

	sum += fwrite(buffer, 1, remainder, dest);

	// Free buffer memory
	free(buffer);

	// Return the size of bytes written
	return sum;
}

// Replace data in file between two offsets with buffer of specified size, preserve file contents before and after offsets
// Return number of bytes written
off_t f_replace_between(FILE *fp, void *data, size_t size, off_t start, off_t end, DBenum overwrite)
{
	// Return early if there is nothing to write
	// f_copy_between will perform other bounds checking for offsets given
	if (fp == NULL || data == NULL)
		return 0;

	off_t sum = 0;
	const char *temp_filen = "database/dbwrite.temp";
	FILE *temp = create_temp_file(temp_filen);

	// Find end of database file
	fseeko(fp, 0, SEEK_END);
	off_t file_size = ftello(fp);


	// If overwrite flag is not set, copy up to position start + 1
	if (overwrite != DB_IO_OVERWRITE)
		start += 1;
	// If overwrite flag is set, copy only after position end + 1
	if (overwrite == DB_IO_OVERWRITE && end < file_size)
		end += 1;

	// Write copy of database file to temp file, excluding the bytes between start and end
	f_copy_between(fp, temp, 0, start);
	f_copy_between(fp, temp, end, file_size);

	// Set file position for writing new data
	fseeko(fp, start, SEEK_SET);

	// Write the data
	sum = fwrite(data, size, 1, fp);

	// Copy the contents after replaced data back into the file
	fseeko(temp, 0, SEEK_END);
	file_size = ftello(temp);
	f_copy_between(temp, fp, start, file_size);

	// If file has remaining characters, truncate
	// File pointer position is currently 1 char after what has been written
	off_t new_size = ftello(fp);
	if (new_size > file_size)
		truncate_file_after(fp, new_size - 1);

	// Close the temporary file
	fclose(temp);

	// Delete the temp file
	remove_temp_file(temp_filen);

	// All file IO has been completed
	return sum;
}

// Insert data in a file from a buffer of specified size after the given offset, preserving(do not overwrite) data after the offset
// Return number of bytes written
off_t f_insert_after(FILE *fp, void *data, size_t size, off_t offset)
{
	// Return early if there is nothing to write
	// f_copy_between will perform other bounds checking for offsets given
	if (fp == NULL || data == NULL)
		return 0;

	off_t sum = 0;
	const char *temp_filen = "database/dbwrite.temp";
	FILE *temp = create_temp_file(temp_filen);

	// Find end of database file
	fseeko(fp, 0, SEEK_END);
	off_t file_size = ftello(fp);

	// Write copy of database file to temp file
	f_copy_between(fp, temp, 0, file_size);

	// Set the file position
	fseeko(fp, offset + 1, SEEK_SET);

	// Write the buffer data
	sum = fwrite(data, size, 1, fp);

	// Copy the data after offset back from the temp file
	f_copy_between(temp, fp, offset + 1, file_size);

	// Close the temporary file
	fclose(temp);

	// Delete the temp file
	remove_temp_file(temp_filen);

	return sum;
}

int num_digits_int(int x)
{
	if (x == 0)
		return 1;
	int digits = 0;
	for(; x > 0; digits++){
		x /= 10;
	}
	return digits;
}


/*
 * Database managment
 */
// Create a database file with a path/filename, returns NULL if failed
struct gl_db *create_database(const char *filen)
{
	char delimiter_char = DB_DELIMITER;
	char start_index_char = START_INDEX;

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

	size_t char_size = sizeof(char);
	size_t num_elements = 3;
	// An empty database will contain metadata: number of tables, and count of used table id's
	// Build metadata string
	char *metadata_string = (char *) malloc(char_size * num_elements);
	snprintf(metadata_string, char_size * num_elements, "%c%c%c", start_index_char, delimiter_char, start_index_char);
	// Write string to file
	fwrite(metadata_string, char_size, num_elements, fp);
	
	// Free string
	free(metadata_string);

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
struct database_metadata get_database_metadata(struct gl_db *db)
{
	struct database_metadata db_md;
	db_md.num_tables = 0;
	db_md.newest_table_id = 0;

	char c;
	char *buffer;
	// Return early if database is not open
	if(db->db_file == NULL)
		return db_md;

	// Determine the maximum number of digits to be read
	/* There will always be at least 1 digit needed to read a value
	 * of 0. Each time the maximum value can be divided by 10,
	 * add an additional digit 
	 */
	int max = DB_MAX_TABLES;
	int digits = num_digits_int(max);
	size_t digit_chars = digits * sizeof(char);

	// Allocate a buffer that can hold the maximum number of plain text digits required
	// Sizeof(char) should be 1, but is added in case this is different
	buffer = (char *) malloc(digit_chars);

	// Return 0 if buffer cannot be allocated
	if (buffer == NULL)
		return db_md;

	// Place file pointer at beginning of database file, where table count is guaranteed to be
	fseeko(db->db_file, 0, SEEK_SET);

	// Table count will be the first metadata element
	int buffer_offset = 0;
	while((c = fgetc(db->db_file)) != EOF){
		// If we are reading further than the size of our buffer, return 0
		if (buffer_offset > digit_chars)
			return db_md;
		// If a delimiter is encountered, break. Table count is first value in database
		if (c == DB_DELIMITER)
			break;
		// All characters preceding the first delimiter should be the integer table count
		*(buffer + buffer_offset) = c;
		buffer_offset++;
	}
	// Use sscanf to parse an integer from the buffer, then return it
	sscanf(buffer, "%d", &(db_md.num_tables));

	// Reset buffer offset
	buffer_offset = 0;
	// Set buffer to 0
	memset(buffer, 0, digit_chars);

	// Begin reading next element, which will be table id count 
	while((c = fgetc(db->db_file)) != EOF){
		// If we are reading further than the size of our buffer, return 0
		if (buffer_offset > digit_chars)
			return db_md;
		// If a delimiter is encountered, there is an error in the database, break
		if (c == DB_DELIMITER){
			return db_md;
			break;
		}
		// If table start is encountered, all database metadata has been read
		if (c == DB_TABLE_START)
			break;
		// All characters preceding the first delimiter should be the integer table count
		*(buffer + buffer_offset) = c;
		buffer_offset++;
	}
	// Use sscanf to parse an integer from the buffer, then return it
	sscanf(buffer, "%d", &(db_md.newest_table_id));

	// Free the memory allocated to the buffer
	free(buffer);

	return db_md;
}

// Write the data from database_metadata struct into the db file
int write_database_metadata(struct database_metadata db_md, struct gl_db *db)
{
	char delimiter_char = DB_DELIMITER;
	char whitespace_char = DB_WHITESPACE;

	// Return an error if database cannot be accessed
	if (db == NULL)
		return 1;
	// Used for convenience, do not close this file, as it is meant to stay open
	FILE *fp = db->db_file;
	size_t char_size = sizeof(char);

	// Return an error if database file cannot be accessed
	if (fp == NULL)
		return 1;

	// Ensure we are at the beginning of the file
	fseeko(fp, 0, SEEK_SET);

	// Find size needed to hold string + null terminator
	size_t buf_size = num_digits_int(db_md.num_tables) + num_digits_int(db_md.newest_table_id) + (char_size * 2);
	// Write values from metadata struct into string buffer
	char *metadata_string = (char *) malloc(buf_size);
	snprintf(metadata_string, buf_size, "%d%c%d", db_md.num_tables, delimiter_char, db_md.newest_table_id);

	// Find start of database tables and ensure not to write past it
	char c;
	off_t table_start_pos = 0;

	while((c = fgetc(fp)) != EOF){
		if (c == DB_TABLE_START)
			break;
	}
	table_start_pos = ftello(fp);
	// Decrement table_start_pos due to ftell giving the position after the fgetc has been called
	table_start_pos -= 1;

	// Write string buffer to file
	// Decrement table_start_pos since overwrite flag is set
	// Decrement buf_size by 1 to exclude the null terminator
	f_replace_between(fp, metadata_string, buf_size - 1, 0, table_start_pos - 1, DB_IO_OVERWRITE);
	
	// Free string buffer
	free(metadata_string);

	return 0;
}


/*
 *Table management
 */
// Create a table in the database with a string name and specify database, returns table id
int add_table_to_db(const char *name, struct gl_db *db)
{
	char delimiter_char = DB_DELIMITER;
	char table_start_char = DB_TABLE_START;
	char row_start_char = DB_ROW_START;
	char row_end_char = DB_ROW_END;

	// Determine the table's id and index
	struct database_metadata db_md = get_database_metadata(db);
	size_t char_size = sizeof(char);
	// Used for convenience, do not close this file, as it is meant to stay open
	FILE *fp = db->db_file;

	// Append table to the end of the database file
	fseeko(fp, 0, SEEK_END);

	// If the maximum number of tables already exist, return
	if (db_md.num_tables >= DB_MAX_TABLES)
		return 1;
	
	// If there are no other tables, do not write a delimiter
	if (db_md.num_tables != 0){
		fwrite(&delimiter_char, char_size, 1,fp);
	}

	// Increment the database metadata
	db_md.num_tables += 1;
	db_md.newest_table_id += 1;
	// Write the new database metadata
	if(write_database_metadata(db_md, db))
		return 1;

	// Write the table start character
	fwrite(&table_start_char, char_size, 1,fp);

	// Write the table's id, row id count, row count, column count, and name
	fprintf(fp, "%d,%d,%d,%d,%s,", db_md.newest_table_id, 0, 0, 0, name);

	// Write the row start and end delimiters for the column name list
	fwrite(&delimiter_char, char_size, 1,fp);
	fwrite(&row_start_char, char_size, 1,fp);
	fwrite(&row_end_char, char_size, 1,fp);

	// Repeat the process above for the column data type list
	fwrite(&delimiter_char, char_size, 1,fp);
	fwrite(&row_start_char, char_size, 1,fp);
	fwrite(&row_end_char, char_size, 1,fp);

	// Each table has a default 'id' column with type DB_INT
	add_column_to_table(DB_KEY_NAME, DB_INT, db_md.newest_table_id, db);

	return 0;
}

//TODO: Write/get table_metadata

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
// Add a column in a table with a string identifier and type specification, specify table id and database
int add_column_to_table(const char *name, enum DB_TYPES type,int table_id, struct gl_db *db)
{
	return 0;
}

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
