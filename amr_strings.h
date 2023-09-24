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

struct amrs_result {
    uint32_t val;
    uint8_t status;
};

// TODO(talha): STR_RAW functions are a bit sus since I donot pass a length
// instead I rely on the capacity being the definitive limit, which I guess is fine

uint8_t Amrs_Is_Allocated(struct amrs_string str);
uint8_t Amrs_Init_Empty(struct amrs_string *str, uint32_t capacity);
uint8_t Amrs_Free(struct amrs_string *str);

// Initialise const char* based strings. 
// Since we can control the size of the input string and this makes it safer
uint8_t Amrs_Init_Const_Str_Raw(struct amrs_string *str, uint32_t capacity, const char *raw_str, uint32_t raw_str_len);

// Initialise raw char* based strings.
// These need to be null-terminated, and their size is then compared with
// total capacity of the string to allow any operations
uint8_t Amrs_Init_Str_Raw(struct amrs_string *str, uint32_t capacity, char *raw_str);

// Append a const char* string to existing string.
uint8_t Amrs_Append_Const_Str_Raw(struct amrs_string *str, const char *raw_str, uint32_t raw_str_len);

// Append a raw char* string to existing string.
// Needs to be a null-terminated string.
// Size is calculated and then checked with capacity to allow any operation
uint8_t Amrs_Append_Str_Raw(struct amrs_string *str, char *raw_str);

// NOTE(talha): Copy a string from copy_from to copy_to
// uses memcpy on the buffer
// performs explicit length checks
// since it uses amrs_string which defines length explicitly
// it is relatively safe
uint8_t Amrs_Copy_Str(struct amrs_string *copy_from, struct amrs_string *copy_to);

struct amrs_result Amrs_Find_Const_Substring_Raw(struct amrs_string *str, const char *raw_substr, uint32_t raw_substr_len);

// TODO(talha): implement this to get detailed errors from a status code
const char *Amrs_Get_Status_Code_Info(uint8_t status_code);

#endif
