#ifndef  AMR_STRINGS_H
#define AMR_STRINGS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @todo: switch to using enums maybe
#define AMRS_OK 0
#define AMRS_INVALID_INSERT_LENGTH 1
#define AMRS_INVALID_CAPACITY_LENGTH 2
#define AMRS_INSUFFICIENT_SPACE 3
#define AMRS_DOUBLE_FREE 4
#define AMRS_ALLOCATED_TRUE 5
#define AMRS_ALLOCATED_FALSE 6
#define AMRS_DOUBLE_INIT 7
#define AMRS_NO_MATCH_FOUND 8
#define AMRS_INVALID_REPLACEMENT_INDEX 9
#define AMRS_INVALID_INDEX 10
#define AMRS_INVALID_INDEX_RANGE 11

// @note: 
// Relatively safe string library. Built for learning and performance (not great right now)
//
// @important:
// - all operations check if the string is allocated
// - in case of initialisation, Amrs_Init_Empty checks if the string is not already
// allocated


// strings will be represented as such
struct amrs_string {
    uint32_t capacity;
    uint32_t len;
    uint8_t is_allocated;
    char *last_element;
    char *buffer;
};

struct amrs_result_u32 {
    uint32_t val;
    uint8_t status;
};

struct amrs_result_char {
    char val;
    uint8_t status;
};

// @note: not sure I need the const str methods
// especially since str methods function the same
// I'll still keep them I guess, will look into removing them later on if I find no 
// issues with the normal method

uint8_t Amrs_Is_Allocated(struct amrs_string str);
uint8_t Amrs_Init_Empty(struct amrs_string *str, uint32_t capacity);

// initialises an empty string and transfers a buffer
// allows the user to manage their own memory and does not rely on
// the internal memory allocations of the library
uint8_t Amrs_Init_Empty_Pass_Buffer(struct amrs_string *str, char *buffer, uint32_t capacity);
uint8_t Amrs_Free(struct amrs_string *str);

// Initialise const char* based strings. 
// Since we can control the size of the input string and this makes it safer
uint8_t Amrs_Init_Const_Str_Raw(struct amrs_string *str, uint32_t capacity, const char *raw_str, uint32_t raw_str_len);

// Initialise raw char* based strings.
// These need to be null-terminated, and their size is then compared with
// total capacity of the string to allow any operations
uint8_t Amrs_Init_Str_Raw(struct amrs_string *str, uint32_t capacity, char *raw_str);

// Safe way to index the string
// adds length and allocation checks
// other than that, is a simple index after this
// more so a convenience function to bypass having to check before access
struct amrs_result_char Amrs_Index(struct amrs_string str, uint32_t index);

// Find a character `to_find` starting at `start_index`
struct amrs_result_u32 
Amrs_Find_Char_From(
        struct amrs_string str, 
        uint32_t start_index,
        char to_find);

// append and copy operations use memcpy on the buffer
// though the operation is trivial and can be done with a loop
// meant to be able to be replaced by a simd operation

// Append a const char* string to existing string.
uint8_t Amrs_Append_Const_Str_Raw(struct amrs_string *str, const char *raw_str, uint32_t raw_str_len);

// Append a raw char* string to existing string.
// Needs to be a null-terminated string.
// Size is calculated and then checked with capacity to allow any operation
uint8_t Amrs_Append_Str_Raw(struct amrs_string *str, char *raw_str);

// NOTE(talha): string copying methods
// perform explicit length checks
// since it uses amrs_string which defines length explicitly
// it is relatively safe
uint8_t Amrs_Append_Str(struct amrs_string *str, struct amrs_string *to_append);
uint8_t Amrs_Copy_Str(struct amrs_string *copy_from, struct amrs_string *copy_to);

// @note: append `to_append` to `str` in the range
// `start_index` - `end_index`
uint8_t Amrs_Append_Str_In_Range(
        struct amrs_string *str, struct amrs_string to_append,
        uint32_t start_index, uint32_t end_index);


// finds in str the given raw_substr 
// returns an amrs_result
// containing index of raw_substr
// and status code indictaing whether the operation was a success
struct amrs_result_u32
Amrs_Find_Const_Substring_Raw(struct amrs_string *str, const char *raw_substr, uint32_t raw_substr_len);

uint8_t Amrs_Replace_Const_Str_Raw(struct amrs_string *str, uint32_t replace_at,
        const char *replace_with, uint32_t replace_with_len);

// TODO(talha): implement this to get detailed errors from a status code
const char *Amrs_Get_Status_Code_Info(uint8_t status_code);

#endif
