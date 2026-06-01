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
	off_t new_pos = ftello(fp);
	// File pointer position is currently 1 char after what has been written
	if (((sum < ((end - start) + 1)) && overwrite == DB_IO_OVERWRITE) || ((sum < ((end - start) - 1)) && overwrite == DB_IO_NO_OVERWRITE))
		truncate_file_after(fp, new_pos - 1);

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
	// Add one character for null terminated string
	char *metadata_string = (char *) malloc(char_size * (num_elements + 1));
	snprintf(metadata_string, char_size * (num_elements + 1), "%c%c%c", start_index_char, delimiter_char, start_index_char);
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
		return DB_ERROR;
	// Used for convenience, do not close this file, as it is meant to stay open
	FILE *fp = db->db_file;
	size_t char_size = sizeof(char);

	// Return an error if database file cannot be accessed
	if (fp == NULL)
		return DB_ERROR;

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
	// If there is no table, start_pos will already be at EOF, so do not decrement
	if (c != EOF)
		table_start_pos -= 1;
	

	// Write string buffer to file
	// Decrement table_start_pos since overwrite flag is set
	// Decrement buf_size by 1 to exclude the null terminator
	f_replace_between(fp, metadata_string, buf_size - 1, 0, table_start_pos - 1, DB_IO_OVERWRITE);
	
	// Free string buffer
	free(metadata_string);

	return DB_SUCCESS;
}


/*
 *Table management
 */
// Create a table in the database with a string name and specify database, returns table id
int add_table_to_db(const char *name, struct gl_db *db)
{
	char delimiter_char = DB_DELIMITER;
	char table_start_char = DB_TABLE_START;
	char table_end_char = DB_TABLE_END;
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
		return DB_ERROR;
	
	// If there are no other tables, do not write a delimiter
	if (db_md.num_tables != 0){
		fwrite(&delimiter_char, char_size, 1,fp);
	}

	// Increment the database metadata
	db_md.num_tables += 1;
	db_md.newest_table_id += 1;
	// Write the new database metadata
	if(write_database_metadata(db_md, db))
		return DB_ERROR;

	// Writing metadata may have moved file position cursor
	fseeko(fp, 0, SEEK_END);
	// Write the table start character
	fwrite(&table_start_char, char_size, 1,fp);

	// Write the table's id, row id count, row count, column count, and name
	fprintf(fp, "%d,%d,%d,%d,%s", db_md.newest_table_id, 0, 0, 0, name);

	// Write the row start and end delimiters for the column name list
	fwrite(&delimiter_char, char_size, 1,fp);
	fwrite(&row_start_char, char_size, 1,fp);
	fwrite(&row_end_char, char_size, 1,fp);

	// Repeat the process above for the column data type list
	fwrite(&delimiter_char, char_size, 1,fp);
	fwrite(&row_start_char, char_size, 1,fp);
	fwrite(&row_end_char, char_size, 1,fp);
	fwrite(&table_end_char, char_size, 1, fp);

	return DB_SUCCESS;
}

// Take an id integer and a database and return the position of the desired table
off_t find_table_by_id(int id, struct gl_db *db)
{
	char table_start_char = DB_TABLE_START;
	char table_end_char = DB_TABLE_END;
	char row_start_char = DB_ROW_START;
	char row_end_char = DB_ROW_END;
	char delimiter_char = DB_DELIMITER;
	off_t table_pos = 0;
	int current_id = 0;
	char c;

	FILE *fp = db->db_file;

	// Determine size of buffer needed to hold table id integer as a string
	struct database_metadata db_md = get_database_metadata(db);
	// Add one for the null terminator character
	size_t buffer_size = num_digits_int(db_md.newest_table_id) + 1;
	
	// Allocate a string buffer to read in the table id's
	char *buffer = (char *) malloc(buffer_size);
	size_t buffer_offset = 0;

	int depth = DB_TABLE_DEPTH;

	// Seek to the beginning of the file so we are at depth 0(not inside a container)
	fseeko(fp, 0, SEEK_SET);
	// Iterate through the file until a table start delimiter is found
	while((c = fgetc(fp)) != EOF){
		if (c == table_start_char){
			// Record the position, back one character due to fgetc advancing the position
			table_pos = ftello(fp) - 1;
			// Determine if this table has the desired id
			while ((c = fgetc(fp)) != EOF){
				// We have written more characters than our expected max size
				if (buffer_offset >= buffer_size)
					break;
				// We have reached the end of the id field
				if (c == delimiter_char){
					*(buffer + buffer_offset) = '\0';
					break;
				}
				// Characters between a table start and the first delimiter should be the table id
				*(buffer + buffer_offset) = c;
				buffer_offset++;
			}
			// Read the integer into a variable from the buffer
			sscanf(buffer, "%d", &current_id);
			// If the id is found, exit the loop
			if (current_id == id){
				break;
			}

			// Reset offset for next search
			buffer_offset = 0;

			// If id has not been found, find the end of this table
			while((c = fgetc(fp)) != EOF){
				// If we have reached a container closing character and are at the correct depth, we have reached the correct position to continue to the next table
				if (c == table_end_char && depth == DB_TABLE_DEPTH)
					break;

				// If we encounter a container inside the table, increase depth counter
				if (c == row_start_char)
					depth++;
				if (c == row_end_char)
					depth--;
			}
		}
		// The loop breaks when the correct position is found, set pos to 0 otherwise
		table_pos = 0;
	}
	
	// Free allocated memory
	free(buffer);
	return table_pos;
}

// Take a table_metadata struct, table id, and database pointer and write the metadata to the table
int write_table_metadata(struct table_metadata table_md, int table_id, struct gl_db *db)
{
	// Get the table position
	off_t table_pos = find_table_by_id(table_id, db);
	off_t metadata_end = table_pos;

	FILE *fp = db->db_file;
	char c;

	// Move to the position behind the table start, the first character of the metadata
	fseeko(fp, table_pos + 1, SEEK_SET);

	// Find the end position of the metadata string
	// TODO: Account for table name
	// Excluding the table name, which is not contained in the metadata struct, there are 4 elements in the metadata list
	// Each will be followed by a delimiter. The last delimiter position will mark the end of the needed string
	int num_elements = 4;
	while((c = fgetc(fp)) != EOF){
		// Decrement the number of remaining delimiters each time one is encountered
		if(c == DB_DELIMITER)
			num_elements--;
		// If we have found all the delimiters, return the position, -1 due to fgetc advancing the pos cursor
		if(num_elements == 0){
			metadata_end = ftello(fp) - 1;
			break;
		}
	}

	// Allocate a string large enough to hold the metadata string elements, 1 byte for each delimiter, and 1 byte for the null terminator character
	char delimiter_char = DB_DELIMITER;
	int size_elements = num_digits_int(table_md.id) + num_digits_int(table_md.num_rows) + num_digits_int(table_md.newest_row_id) + num_digits_int(table_md.num_cols);
	int num_delimiters = 4; // There are 4 pieces of metadata to be written
	size_t buffer_size = size_elements + num_delimiters + 1;
	char *buffer = (char *) malloc(buffer_size);
	// Write the metadata to the string
	snprintf(buffer, buffer_size, "%d%c%d%c%d%c%d%c", table_md.id, delimiter_char, table_md.num_rows, delimiter_char, table_md.newest_row_id, delimiter_char, table_md.num_cols, delimiter_char);

	// Replace the current metadata with the newly formed string
	// Start at the first character after the table start, as to not overwrite the delimiter
	f_replace_between(fp, buffer, buffer_size - 1, table_pos + 1, metadata_end, DB_IO_OVERWRITE);

	// Free the memory for the buffer
	free(buffer);

	return DB_SUCCESS;

}

// Take a table id and a database as an argument, find table metadata, //TODO: allocates memory for table name
struct table_metadata get_table_metadata(int id, struct gl_db *db)
{
	// Initialize the struct
	struct table_metadata table_md;
	// Get the table position
	off_t table_pos = find_table_by_id(id, db);
	off_t metadata_end = table_pos;

	FILE *fp = db->db_file;
	char c;

	// Write the id, which is given
	table_md.id = id;

	// Move to the position behind the table start, the first character of the metadata
	fseeko(fp, table_pos + 1, SEEK_SET);

	// Find the length of the string needed to contain the metadata
	// TODO: Account for table name
	// Excluding the table name, which is not contained in the metadata struct, there are 4 elements in the metadata list
	// Each will be followed by a delimiter. The last delimiter position will mark the end of the needed string
	int num_elements = 4;
	while((c = fgetc(fp)) != EOF){
		// Decrement the number of remaining delimiters each time one is encountered
		if(c == DB_DELIMITER)
			num_elements--;
		// If we have found all the delimiters, return the position, -1 due to fgetc advancing the pos cursor
		if(num_elements == 0){
			metadata_end = ftello(fp) - 1;
			break;
		}
	}

	// Allocate a string large enough to hold the metadata substring + the null terminator, we do not read the final delimiter
	size_t buffer_size = metadata_end - table_pos;
	char *buffer = (char *) malloc(buffer_size);

	// Move the cursor back to the start of the metadata
	fseeko(fp, table_pos + 1, SEEK_SET);

	// Read the appropriate number of characters into the buffer
	off_t buffer_offset = 0;
	while((c = fgetc(fp)) != EOF){
		// We have read the desired number of characters, write the null terminator
		if (buffer_offset == (buffer_size - 1)){
			*(buffer + buffer_offset) = '\0';
			break;
		}
		*(buffer + buffer_offset) = c;
		buffer_offset++;
	}
	// Read from the buffer into the metadata struct
	sscanf(buffer, "%d,%d,%d,%d", &(table_md.id), &(table_md.num_rows), &(table_md.newest_row_id), &(table_md.num_cols));

	// Free the memory for the buffer
	free(buffer);

	return table_md;
}

// Get table id from string name
int get_table_id(const char *, struct gl_db *);

// Get count of columns in table
int get_column_count(int, struct gl_db *);

// Get row count from table
int get_row_count(int, struct gl_db *);

// Get table column data types
// Excludes the default key id type
// Allocates memory for the types list
enum DB_TYPES *get_data_types_list(int table_id, struct gl_db *db)
{
	off_t types_container_pos = 0;

	//Find table position
	off_t table_pos = find_table_by_id(table_id, db);
	FILE *fp = db->db_file;

	// Move the file position cursor to one character past the start of the table
	fseeko(fp, table_pos + 1, SEEK_SET);
	// Keep track of containers if we encounter any delimiting characters
	int depth = DB_COLROW_DEPTH;
	int containers_before_types = 1; // The col names container precedes the types container
	char delimiter_char = DB_DELIMITER;
	char c;
	while((c = fgetc(fp)) != EOF){
		if (c == DB_ROW_START){
			// Obtain the cursor position - 1 to account for fgetc advancing the position
			types_container_pos = ftello(fp) - 1;
			// If there are no other containers to skip, break and report the position
			if (containers_before_types == 0 && depth == DB_COLROW_DEPTH)
				break;
			// Decrement the containers counter, since we have encountered a container
			containers_before_types--;
			// Increment the depth counter, since we are entering a container
			depth++;
			// Iterate until we return to desired depth
			while((c = fgetc(fp)) != EOF){
				if (c == DB_ROW_END){
					depth--;
					break;
				}
			}
		}
		// Reset the position if the correct container has not been encountered
		types_container_pos = 0;
	}

	// Find the end of the types container
	off_t types_container_end = types_container_pos;
	while((c = fgetc(fp)) != EOF){
		if (c == DB_ROW_END){
			// Obtain the cursor position - 1 to account for fgetc advancing the position
			types_container_end = ftello(fp) - 1;
			// If end of container symbol has been found break and report the position
			break;
			}
	}

	// Allocate a string which can hold the bytes from types_container_pos to types_container_end EXCLUDING the container delimiter symbols, but INCLUDING a null-terminator character
	size_t buffer_size = types_container_end - types_container_pos;
	char *buffer = (char *) malloc(buffer_size);

	// Return the cursor position to the start of the container to begin reading into the string
	fseeko(fp, types_container_pos + 1, SEEK_SET);

	// Read the characters until the buffer has been filled appropriately
	off_t buffer_offset = 0;
	while((c = fgetc(fp)) != EOF){
		// We have read the desired number of characters, write the null terminator
		if (buffer_offset == (buffer_size - 1)){
			*(buffer + buffer_offset) = '\0';
			break;
		}
		*(buffer + buffer_offset) = c;
		buffer_offset++;
	}

	// Get the table metadata and determine the number of elements in the column type list, excluding the key id
	struct table_metadata table_md = get_table_metadata(table_id, db);
	int num_elements = table_md.num_cols - 1;

	// Allocate a DB_TYPES array with num_elements space
	enum DB_TYPES *types_list = (enum DB_TYPES *) malloc(num_elements * (sizeof(enum DB_TYPES)));

	// First two characters will be key id type and a delimiter character, which will be excluded from the types list that is returned
	char *types_string = buffer + 2;
	// Null terminated, so strlen works appropriately
	size_t len_types_string = strlen(types_string);

	for (int i = 0; i < len_types_string; i += 2){
		sscanf((types_string + i), "%1d", (types_list + (i / 2)));
	}

	// types_string points to the same memory as buffer, do not free both, set types_string as null
	free(buffer);
	buffer = NULL;
	types_string = NULL;

	return types_list;
}

// Remove a table from the database
int remove_table_from_db(int table_id, struct gl_db *db)
{
	FILE *fp = db->db_file;

	// Find the table position
	off_t table_pos = find_table_by_id(table_id, db);
	off_t table_end = 0;

	// If table is not found, return error
	if (!table_pos)
		return DB_ERROR;

	// Find the position of the end of the table
	// Seek to the inside of the table container
	fseeko(fp, table_pos + 1, SEEK_SET);

	// Iterate the file until the table end symbol has been found at the appropriate depth
	int depth = DB_TABLE_DEPTH;
	char c;
	while ((c = fgetc(fp)) != EOF){
		// Mark the current position, moved back one position due to fgetc advancing the offset
		table_end = ftello(fp) - 1;
		// End of the container has been reached
		if (c == DB_TABLE_END && depth == DB_TABLE_DEPTH)
			break;
		if (c == DB_ROW_START)
			depth++;
		if (c == DB_ROW_END)
			depth--;
	}
	// Return error if end of file was reached
	if (c == EOF)
		return DB_ERROR;

	// If the character before the table is a delimiter, include it in the overwrite below
	fseeko(fp, table_pos - 1, SEEK_SET);
	if ((c = fgetc(fp)) == DB_DELIMITER)
		table_pos -= 1;
	// Overwrite the data between the offsets with 0 bytes, effectively erasing the row
	// f_replace_between rejects null data, so we give a pointer to some data and specify size 0 so none is written
	f_replace_between(fp, &c, 0, table_pos, table_end, DB_IO_OVERWRITE);

	// Decrement the number of rows in the table metadata
	struct database_metadata database_md = get_database_metadata(db);
	database_md.num_tables -= 1;

	// Write the new value to the database
	write_database_metadata(database_md, db);

	return DB_SUCCESS;
}

/*
 *Column management
 */
// Take a list of DB_TYPES and number of elements, table id, and database pointer, and write the type list to a string in the database
// Writes an additional default key id of DB_INT type at the beginning of the list
int write_column_data_types_to_table(enum DB_TYPES *types, int num_elements, int table_id, struct gl_db *db)
{
	off_t types_container_pos = 0;
	// Obtain a string of the types list
	char *types_string = type_list_to_string(types, num_elements);
	// String is null-terminated, so strlen will work
	size_t types_string_len = strlen(types_string);

	// Find the position of the desired table
	off_t table_pos = find_table_by_id(table_id, db);
	FILE *fp = db->db_file;

	// Move the file position cursor to one character past the start of the table
	fseeko(fp, table_pos + 1, SEEK_SET);

	// Keep track of containers if we encounter any delimiting characters
	int depth = DB_COLROW_DEPTH;
	int containers_before_types = 1; // The col names container precedes the types container
	char delimiter_char = DB_DELIMITER;
	char c;
	while((c = fgetc(fp)) != EOF){
		if (c == DB_ROW_START){
			// Obtain the cursor position - 1 to account for fgetc advancing the position
			types_container_pos = ftello(fp) - 1;
			// If there are no other containers to skip, break and report the position
			if (containers_before_types == 0 && depth == DB_COLROW_DEPTH)
				break;
			// Decrement the containers counter, since we have encountered a container
			containers_before_types--;
			// Increment the depth counter, since we are entering a container
			depth++;
			// Iterate until we return to desired depth
			while((c = fgetc(fp)) != EOF){
				if (c == DB_ROW_END){
					depth--;
					break;
				}
			}
		}
		// Reset the position if the correct container has not been encountered
		types_container_pos = 0;
	}

	// Find the end of the types container
	off_t types_container_end = types_container_pos;
	while((c = fgetc(fp)) != EOF){
		if (c == DB_ROW_END){
			// Obtain the cursor position - 1 to account for fgetc advancing the position
			types_container_end = ftello(fp) - 1;
			// If end of container symbol has been found break and report the position
			break;
			}
	}

	// Insert the types list string within the container
	f_replace_between(fp, types_string, types_string_len, types_container_pos, types_container_end, DB_IO_NO_OVERWRITE);

	// Insert the key id type string before the remaining types
	// Key id column occupies 1 additional column over the other elements
	// 1 byte for DB_TYPE char, 1 byte for delimiter char, 1 byte for null terminator
	size_t key_id_string_size = 3;
	char *key_id_string = (char *) malloc(key_id_string_size);
	snprintf(key_id_string, key_id_string_size, "%d%c", DB_INT, delimiter_char);

	// The size of the string memory - 1 is the length of the string
	f_insert_after(fp, key_id_string, key_id_string_size - 1, types_container_pos);

	free(types_string);
	free(key_id_string);

	// Update the number of columns in the table metadata
	struct table_metadata table_md = get_table_metadata(table_id, db);

	// The table columns are the elements from the list and 1 additional column for the key id
	table_md.num_cols = num_elements + 1;
	write_table_metadata(table_md, table_id, db);

	return DB_SUCCESS;
}

// Add a column in a table with a string identifier and type specification, specify table id and database
int add_column_to_table(const char *name, enum DB_TYPES type,int table_id, struct gl_db *db)
{
	return DB_SUCCESS;
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
// Will reject non-conforming row objects based on table column data types
// If the table has no current column data types, it will write the row object's data types list
int add_row_to_table(struct row_object *ro, int table_id, struct gl_db *db)
{
	// Create a more convenient pointer for the database file 
	FILE *fp = db->db_file;

	// Table symbols
	char table_start_char = DB_TABLE_START;
	char table_end_char = DB_TABLE_END;
	char row_start_char = DB_ROW_START;
	char row_end_char = DB_ROW_END;
	char delimiter_char = DB_DELIMITER;

	// Find the table to be written to
	off_t table_pos = 0;
	table_pos = find_table_by_id(table_id, db);
	// If table was not found, return
	if (!table_pos)
		return DB_ERROR;

	// Get table metadata
	struct table_metadata table_md = get_table_metadata(table_id, db);

	// Obtain a string for the row data
	char *row_string = serial_to_string(ro);
	
	// Obtain a string for the data types list
	char *row_data_types = type_list_to_string(ro->data_type_list, ro->column_count);

	// If the table has no current data types column data, continue to next procedure
	// Otherwise, compare the current data to the new data and reject row objects that do not conform
	if (table_md.num_cols != 0){

		// Check that the table and row object have the same number of data elements, minus the key id data
		// Otherwise, exit with an error after freeing resources
		if ((table_md.num_cols - 1) != ro->column_count){
			free(row_string);
			free(row_data_types);
			return DB_ERROR;
		}
		// Find the table's existing data type list and convert it to a string
		enum DB_TYPES *table_data_types = get_data_types_list(table_id, db);
		char *table_data_types_string = type_list_to_string(table_data_types, table_md.num_cols - 1); // Subtract 1 for the key id column

		// Compare the string to that of the row object, exit with an error if they are not equal
		if (strcmp(table_data_types_string, row_data_types)){
			free(row_string);
			free(row_data_types);
			free(table_data_types);
			free(table_data_types_string);
			return DB_ERROR;
		}

		// Free the resources used to compare the strings
		free(table_data_types);
		free(table_data_types_string);
	}else{
		// If the table previously had no data types column data, write it now
		write_column_data_types_to_table(ro->data_type_list, ro->column_count, table_id, db);
	}
	// Both cases are now converged with the data types being written to the table, matching the row object
	// Update the locally stored table metadata 
	table_md = get_table_metadata(table_id, db);
	
	// Find the end of the table, where the row will be appended
	off_t table_end = 0;
	
	// Move the file position to the character after the table start delimiter
	fseeko(fp, table_pos + 1, SEEK_SET);

	// We are within the table container, depth is colrow
	int depth = DB_COLROW_DEPTH;
	char c;
	while ((c = fgetc(fp)) != EOF){
		// If we have reached a container closing character and are at the correct depth, we have reached the end of the desired table
		if (c == table_end_char && depth == DB_COLROW_DEPTH){
			// Update the position offset, decremented by 1 due to fgetc advancing the offset
			table_end = ftello(fp) - 1;
			break;
		}

		// If we encounter a container inside the table, increase depth counter
		if (c == row_start_char)
			depth++;
		if (c == row_end_char)
			depth--;
	}
	// If the table end is not found, return with an error
	if(table_end == 0){
		free(row_string);
		free(row_data_types);
		return DB_ERROR;
	}

	// Increment the number of rows
	table_md.num_rows += 1;
	table_md.newest_row_id += 1;

	// Allocate a string buffer that can hold a database delimiter, the row start delimiter (1-byte), the row_id, 
	// the database delimiter character (1-byte), the existing row object string, 
	// a row end delimiter (1-byte), and the null-terminator(1-byte)
	int char_size = sizeof(char);
	size_t buffer_size = (char_size * 5) + num_digits_int(table_md.newest_row_id) + strlen(row_string);
	char *buffer = (char *) malloc(buffer_size);
	snprintf(buffer, buffer_size, "%c%c%d%c%s%c", delimiter_char, row_start_char, table_md.newest_row_id, delimiter_char, row_string, row_end_char);

	// Insert the new string into the file before the table_end character, subtract 1 from the buffer size to exclude the null-terminator
	f_insert_after(fp, buffer, buffer_size - 1, table_end - 1);

	// Update the table metadata
	write_table_metadata(table_md, table_id, db);

	// Free the allocated resources and return success
	free(row_string);
	free(row_data_types);
	free(buffer);
	
	return DB_SUCCESS;
}

// Find the position of a row with a given id, table id, and database
// Return 0 if it is not found
off_t find_row_by_id(int row_id, int table_id, struct gl_db *db)
{
	off_t row_pos = 0;
	FILE *fp = db->db_file;

	// Find the table position
	off_t table_pos = find_table_by_id(table_id, db);

	// Get the table metadata to check number of rows
	struct table_metadata table_md = get_table_metadata(table_id, db);

	// If row id is greater than the greatest row id created, it cannot be in the table, return error
	if (row_id > table_md.newest_row_id)
		return row_pos;

	// Skip the column data containers
	int num_col_containers = 2;

	// Move the file position cursor to the inside of the table
	fseeko(fp, table_pos + 1, SEEK_SET);

	int depth = DB_COLROW_DEPTH;
	char c;
	while((c = fgetc(fp)) != EOF){
		if (c == DB_ROW_START){
			// Obtain the cursor position - 1 to account for fgetc advancing the position
			row_pos = ftello(fp) - 1;
			// If there are no other containers to skip, break and report the position
			if (num_col_containers == 0 && depth == DB_COLROW_DEPTH)
				break;
			// Decrement the containers counter, since we have encountered a container
			num_col_containers--;
			// Increment the depth counter, since we are entering a container
			depth++;
			// Iterate until we return to desired depth
			while((c = fgetc(fp)) != EOF){
				if (c == DB_ROW_END){
					depth--;
					break;
				}
			}
		}
		// Set row_pos to 0 if we did not reach the break statement in this iteration
		row_pos = 0;
	}

	// Return if end of columns could not be found
	if (!row_pos)
		return row_pos;

	// Decrement the row position by 1 to place the offset before the first row_start symbol
	row_pos--;
	fseeko(fp, row_pos, SEEK_SET);

	// Iterate through the file until a row start delimiter is found, check id, then return the position offset
	int current_id = 0;
	depth = DB_COLROW_DEPTH;

	// Allocate a buffer that can hold the id characters plus the null terminator
	size_t buffer_size = num_digits_int(table_md.newest_row_id) + 1;
	size_t buffer_offset = 0;
	char *buffer = (char *) malloc(buffer_size);

	while((c = fgetc(fp)) != EOF){
		if (c == DB_ROW_START){
			// Record the position, back one character due to fgetc advancing the position
			row_pos = ftello(fp) - 1;
			// Determine if this row has the desired id
			while ((c = fgetc(fp)) != EOF){
				// We have written more characters than our expected max size
				if (buffer_offset >= buffer_size)
					break;
				// We have reached the end of the id field
				if (c == DB_DELIMITER){
					*(buffer + buffer_offset) = '\0';
					break;
				}
				// Characters between a table start and the first delimiter should be the table id
				*(buffer + buffer_offset) = c;
				buffer_offset++;
			}
			// Read the integer into a variable from the buffer
			sscanf(buffer, "%d", &current_id);
			// If the id is found, exit the loop
			if (current_id == row_id){
				break;
			}

			// Reset offset for next search
			buffer_offset = 0;

			// If id has not been found, find the end of this table
			while((c = fgetc(fp)) != EOF){
				// If we have reached a container closing character and are at the correct depth, we have reached the correct position to continue to the next row
				if (c == DB_ROW_END && depth == DB_COLROW_DEPTH)
					break;

				// If we encounter a container inside the row, increase depth counter
				if (c == DB_ROW_START)
					depth++;
				if (c == DB_ROW_END)
					depth--;
			}
		}
		// The loop breaks when the correct position is found, set pos to 0 otherwise
		row_pos = 0;
	}

	free(buffer);
	return row_pos;
}

// Remove a row from the table by providing the row index/id, the table id, and the database
int remove_row_from_table(int row_id, int table_id, struct gl_db *db)
{
	FILE *fp = db->db_file;

	// Find the row position
	off_t row_pos = find_row_by_id(row_id, table_id, db);
	off_t row_end = 0;

	// If row is not found, return error
	if (!row_pos)
		return DB_ERROR;

	// Find the position of the end of the row
	// Seek to the inside of the row container
	fseeko(fp, row_pos + 1, SEEK_SET);

	// Iterate the file until the row end symbol has been found at the appropriate depth
	int depth = DB_COLROW_DEPTH;
	char c;
	while ((c = fgetc(fp)) != EOF){
		// Mark the current position, moved back one position due to fgetc advancing the offset
		row_end = ftello(fp) - 1;
		// End of the container has been reached
		if (c == DB_ROW_END && depth == DB_COLROW_DEPTH)
			break;
		if (c == DB_ROW_START)
			depth++;
		if (c == DB_ROW_END)
			depth--;
	}
	// Return error if end of file was reached
	if (c == EOF)
		return DB_ERROR;

	// Overwrite the data between the offsets with 0 bytes, effectively erasing the row
	// f_replace_between rejects null data, so we give a pointer to some data and specify size 0 so none is written
	// Decrement the row_pos offset so that the preceding delimiter is also erased
	f_replace_between(fp, &c, 0, row_pos - 1, row_end, DB_IO_OVERWRITE);

	// Decrement the number of rows in the table metadata
	struct table_metadata table_md = get_table_metadata(table_id, db);
	table_md.num_rows -= 1;

	// Write the new value to the database
	write_table_metadata(table_md, table_id, db);

	return DB_SUCCESS;
}

/*
 *Data management
 */
// Return the data from an entire row, provide the row index, table id, and database
// Allocates memory for a row_object
struct row_object *get_row_data(int row_id, int table_id, struct gl_db *db)
{

	// Value returned
	struct row_object *ro = NULL;

	FILE *fp = db->db_file;

	// Find the row position
	off_t row_pos = find_row_by_id(row_id, table_id, db);
	off_t row_end = 0;

	// If row is not found, return error
	if (!row_pos)
		return ro;

	// Find the position of the end of the row
	// Seek to the inside of the row container
	fseeko(fp, row_pos + 1, SEEK_SET);

	// Iterate the file until the row end symbol has been found at the appropriate depth
	int depth = DB_COLROW_DEPTH;
	char c;
	while ((c = fgetc(fp)) != EOF){
		// Mark the current position, moved back one position due to fgetc advancing the offset
		row_end = ftello(fp) - 1;
		// End of the container has been reached
		if (c == DB_ROW_END && depth == DB_COLROW_DEPTH)
			break;
		if (c == DB_ROW_START)
			depth++;
		if (c == DB_ROW_END)
			depth--;
	}
	// Return error if end of file was reached
	if (c == EOF)
		return ro;

	// Increment the row position by number of digits in the id, plus number of chars in the delimiter char so that we skip
	// The key id data
	row_pos += num_digits_int(row_id) + 1;
	// Seek to the inside of the row container after the key id
	fseeko(fp, row_pos + 1, SEEK_SET);

	// Allocate a buffer large enough to hold the string between the start of the row and the end, plus the null terminator
	size_t buffer_size = row_end - row_pos;
	char *buffer = (char *) malloc(buffer_size);
	
	// Read the appropriate number of characters into the buffer
	off_t buffer_offset = 0;
	while((c = fgetc(fp)) != EOF){
		// We have read the desired number of characters, write the null terminator
		if (buffer_offset == (buffer_size - 1)){
			*(buffer + buffer_offset) = '\0';
			break;
		}
		*(buffer + buffer_offset) = c;
		buffer_offset++;
	}

	// Obtain the table metadata for number of elements
	struct table_metadata table_md = get_table_metadata(table_id, db);

	// Obtain a string for the data types list
	enum DB_TYPES *types_list = get_data_types_list(table_id, db);
	// Subtract 1 from the number of columns to exclude the key id column
	char *types_list_string = type_list_to_string(types_list, table_md.num_cols - 1);

	// Create the row object from the obtained string
	ro = string_to_serial(buffer, types_list_string);

	// Free allocated resources and return the row object
	free(types_list);
	free(types_list_string);
	free(buffer);
	return ro;
}

// Update row by providing struct object, row id, table id, and database
int update_row_data_by_id(struct row_object *ro, int row_id, int table_id, struct gl_db *db)
{
	/*
	 * Manipulate the table metadata to ensure that the row id remains the same when writing a new table
	 * Then set it back to the original values
	 */

	// Get current table_metadata
	struct table_metadata table_md = get_table_metadata(table_id, db);
	int current_row_id = table_md.newest_row_id;

	// Return with an error if the row id cannot be present in the table
	if (table_md.newest_row_id < row_id)
		return DB_ERROR;

	// Remove the current data with the desired row id
	remove_row_from_table(row_id, table_id, db);

	// Set the table newest row id metadata to 1 below the desired id, so that the next row is given that id
	table_md.newest_row_id = row_id - 1;
	// Write the metadata to the database
	write_table_metadata(table_md, table_id, db);

	// Add the new row data
	add_row_to_table(ro, table_id, db);

	// Rewrite the correct table metadata
	table_md.newest_row_id = current_row_id;
	// Write the metadata to the database
	write_table_metadata(table_md, table_id, db);

	return DB_SUCCESS;
}

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
