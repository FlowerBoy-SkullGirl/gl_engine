#include <iostream>
#include <stdlib.h>
#include <cstddef>
#include "string.h"
#include <cstdio>
#include "headers/database.h"

#define FILE_IO_BUFFER_SIZE 1

/*
 * File IO Helper Functions
 */

// Write data into a buffer, checking if the specified size and offset will write outside of the bounds of the buffer
// size_t is unsigned, so we do not worry about negative offset or obj size arguments
size_t write_to_buffer(char *buffer, char *data, size_t offset, size_t size_obj, size_t buf_len)
{
	if (offset + size_obj > buf_len)
		return 0;
	memcpy(buffer + offset, data, size_obj);
	return size_obj;
}

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
 * Serialization
 */ 

// Takes a data type list and number of elements argument and converts it to a null-terminated, delimited string
// Allocates memory for the string
char *type_list_to_string(enum DB_TYPES *type_list, int elements)
{
	// Allocate a string that can hold each 1-digit element, a 1-char delimiter for each element except the last, and the 1-byte null terminator
	size_t buffer_size = elements * 2;
	char *buffer = (char *) malloc(buffer_size);

	// For each element of type_list, write the element to the string
	for (int i = 0; i < elements; i++){
		snprintf(buffer + (i*2), buffer_size - (i*2), "%1d", *(type_list + i));
		// If it is the last element, continue
		if (i + 1 == elements)
			continue;
		// Otherwise, write the delimiter character
		*(buffer + (i*2) + 1) = DB_DELIMITER;
	}
	// Write the null terminator at the last byte
	*(buffer + buffer_size - 1) = '\0';

	return buffer;
}

// Take a row_object struct and convert it to a null-terminated, delimited string
// Allocates memory for the string
char *serial_to_string(struct row_object *ro)
{
	// Tracks the offset of the data_list where the next element is to be read
	size_t offset = 0;
	size_t buffer_size = 0;
	size_t chars_written = 0;

	// Data type sizes
	size_t size_int = sizeof(int);
	size_t size_float = sizeof(float);

	// Pointers used to manipulate the row object buffer
	char *char_pointer = (char *) ro->data_list;
	int *int_pointer = NULL;
	float *float_pointer = NULL;
	char *str_pointer = NULL;

	// String where the serialized data will be stored, this value will be returned
	char *buffer = NULL;

	// Find the size needed to hold the new string
	int x = 0;
	for (int i = 0; i < ro->column_count; i++){
		switch (*((ro)->data_type_list + i)){
			case DB_INT:
				// Use an integer pointer at the address of the next element
				int_pointer = (int *) (char_pointer + offset);

				// Find the number of digits of that integer
				buffer_size += num_digits_int(*int_pointer);

				// Increment the offset where the next element is found
				offset += size_int;
				break;
			case DB_FLOAT:
				// Use a float pointer at the address of the next element
				float_pointer = (float *) (char_pointer + offset);

				// Cast the float as an integer to determine how many digits precede the decimal point
				x = *float_pointer;

				// Find the number of digits for the cast integer + 1 for a decimal character + number of digits of precision
				buffer_size += num_digits_int(x) + 1 + DB_FLOAT_PRECISION;

				// Increment the offset where the next element is found
				offset += size_float;
				break;
			// All strings are stored as max size and filled with null-terminator characters until end of string
			case DB_STRING:
				// For ease of use, address a second char pointer at the beginning of the string
				str_pointer = (char_pointer + offset);
				// Find the length of the string up to the first null terminator
				buffer_size += strlen(str_pointer);

				// Increment the offset where the next element is found
				offset += DB_MAX_SIZE_STRING;
				break;
			default:
				break;

		}
	}

	// Allocate memory for the buffer, with an additional byte for each delimiter(column-count - 1, since the last element will not have a delimiter)
	// and an additional byte for the null-terminator
	buffer_size += (ro->column_count);
	buffer = (char *) malloc(buffer_size);
	
	// Reset offset
	offset = 0;

	// Read data into string
	size_t str_size = 0;
	for (int i = 0; i < ro->column_count; i++){
		switch (*((ro)->data_type_list + i)){
			case DB_INT:
				// Use an integer pointer at the address of the next element
				int_pointer = (int *) (char_pointer + offset);

				// Write the integer to the string at the correct offset
				snprintf(buffer + chars_written, buffer_size - chars_written, "%d", (*int_pointer));
				
				// Increment the number of characters written
				chars_written += num_digits_int(*int_pointer);
				// Increment the offset where the next element is found
				offset += size_int;
				break;
			case DB_FLOAT:
				// Use a float pointer at the address of the next element
				float_pointer = (float *) (char_pointer + offset);

				// Write the float to the string at the correct offset, with DB_FLOAT_PRECISION decimal places
				snprintf(buffer + chars_written, buffer_size - chars_written, "%.*f", DB_FLOAT_PRECISION, (*float_pointer));
				// Increment the number of characters written
				x = *float_pointer;
				chars_written += num_digits_int(x) + 1 + DB_FLOAT_PRECISION;
				// Increment the offset where the next element is found
				offset += size_float;
				break;
			// All strings are stored as max size and filled with null-terminator characters until end of string
			case DB_STRING:
				// For ease of use, address a second char pointer at the beginning of the string
				str_pointer = (char_pointer + offset);
				// Find the length of the string up to the first null terminator
				str_size = strlen(str_pointer);
				// Write the integer to the string at the correct offset
				snprintf(buffer + chars_written, buffer_size - chars_written, "%s", str_pointer);

				// Increment the offset where the next element is found
				chars_written += str_size;
				offset += DB_MAX_SIZE_STRING;
				break;
			default:
				break;

		}
		// For each element except the last, write a delimiter to the string
		if (i + 1 == (ro->column_count))
			continue;
		*(buffer + chars_written) = DB_DELIMITER;
		chars_written += 1;
	}
	// Null terminate the string
	*(buffer + chars_written) = '\0';

	return buffer;
}

// Take a delimited string and convert it to a row_object struct
// Expects null-terminated strings
// Allocates memory for the row_object
struct row_object *string_to_serial(char *data_string, char *types_string)
{
	// Allocate memory for the struct
	struct row_object *ro = (struct row_object *) malloc(sizeof(struct row_object));

	// Data type sizes
	size_t size_int = sizeof(int);
	size_t size_float = sizeof(float);

	// Find the number of elements in the list, strings should be null-terminated
	ro->column_count = 0;

	size_t len_types_string = strlen(types_string);
	// If first character is not the null-terminator, there is at least 1 element
	if (len_types_string > 0)
		(ro->column_count) += 1;

	// DB_TYPES is never greater than 1 digit, see database.h definition
	// There is an additional element for each delimiter found
	for (int i = 0; i < len_types_string; i++){
		if (*(types_string + i) == DB_DELIMITER)
			(ro->column_count) += 1;
	}

	// Allocate appropriate memory for data_type_list based off of number of elements found
	(ro->data_type_list) = (enum DB_TYPES *) malloc(sizeof(enum DB_TYPES) * (ro->column_count));

	// Read values from the string into the data_type_list
	// Each 1-byte width element in the string will be followed by a 1-byte delimiter
	// Read 1 char as an integer into the type list, then increment the offset by 2
	// The type list index will always be offset / 2
	for (int i = 0; i < len_types_string; i += 2){
		sscanf((types_string + i), "%1d", ((ro->data_type_list) + (i / 2)));
	}

	// Iterate the newly formed data_type_list and determine the size in bytes of the data_list
	ro->data_list_size = 0;
	for (int i = 0; i < ro->column_count; i++){
		switch(*((ro->data_type_list) + i)){
			case DB_INT:
				(ro->data_list_size) += size_int;
				break;
			case DB_FLOAT:
				(ro->data_list_size) += size_float;
				break;
			case DB_STRING:
				break;
				(ro->data_list_size) += DB_MAX_SIZE_STRING;
			default:
				break;
		}
	}

	// Allocate the appropriate memory for the data_list
	ro->data_list = malloc((ro->data_list_size));

	// Read data from the data_string into the data_list
	size_t len_data_string = strlen(data_string);

	size_t data_list_offset = 0;
	size_t data_string_offset = 0;

	char *char_pointer = (char *)(ro->data_list);
	int *int_pointer = NULL;
	float *float_pointer = NULL;
	char *str_pointer = NULL;

	for (int i = 0; i < ro->column_count; i++){
		switch(*((ro->data_type_list) + i)){
			case DB_INT:
				// Read an int into the data_list
				int_pointer = (int *) (char_pointer + data_list_offset);
				sscanf((data_string + data_string_offset), "%d", int_pointer);
				// Increment the data_list_offset by size of int
				data_list_offset += size_int;
				break;
			case DB_FLOAT:
				// Read a float into the data_list
				float_pointer = (float *) (char_pointer + data_list_offset);
				sscanf((data_string + data_string_offset), "%f", float_pointer);
				// Increment the data_list_offset by size of float
				data_list_offset += size_float;
				break;
			case DB_STRING:
				// Fill the space allotted to the string with null characters
				str_pointer = (char_pointer + data_list_offset);
				memset(str_pointer, '\0', DB_MAX_SIZE_STRING);
				// Copy each character from the current position in data_string_offset up to the next delimiter
				for (int j = data_string_offset; j < (len_data_string - data_string_offset); j++){
					if (*(data_string + j) == DB_DELIMITER)
						break;
					*(str_pointer + j) = *(data_string + j);
				}
				data_list_offset += DB_MAX_SIZE_STRING;
				break;
			default:
				break;
		}
		// If this is the last element, there will not be an additional delimiter
		if (i + 1 == (ro->column_count))
			continue;
		// Otherwise, find the next delimiter and increment the data_string_offset past it
		for (int j = data_string_offset; j < len_data_string; j++){
			if (*(data_string + j) == DB_DELIMITER){
				data_string_offset = j + 1;
				break;
			}
		}
	}

	return ro;
}

// Checks that all members of the struct have been freed, then frees the memory for the struct
// Returns null
struct row_object *free_serialized_data(struct row_object *ro)
{
	if(ro == NULL)
		return NULL;
	if(ro->data_list != NULL){
		free(ro->data_list);
		ro->data_list == NULL;
	}
	if(ro->data_type_list != NULL){
		free(ro->data_type_list);
		ro->data_type_list == NULL;
	}

	free(ro);
	ro = NULL;
	return NULL;
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
