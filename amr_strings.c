uint8_t Amrs_Is_Allocated(struct amrs_string str)
{
    if (str.is_allocated == 1)
    {
        return AMRS_ALLOCATED_TRUE;
    }
    else
    {
        return AMRS_ALLOCATED_FALSE;
    }
}

uint8_t Amrs_Init_Empty(struct amrs_string *str, uint32_t capacity)
{
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_TRUE)
    {
        return AMRS_DOUBLE_INIT;
    }

    if (capacity == 0) return AMRS_INVALID_CAPACITY_LENGTH;

    str->len = 0;
    str->capacity = capacity;
    str->buffer = (char *)calloc(str->capacity, sizeof(str->buffer));
    str->is_allocated = 1;

    return AMRS_OK;
}

uint8_t Amrs_Init_Empty_Pass_Buffer(struct amrs_string *str, char *buffer, uint32_t capacity)
{
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_TRUE)
    {
        return AMRS_DOUBLE_INIT;
    }
    if (capacity ==  0) return AMRS_INVALID_CAPACITY_LENGTH;

    // @todo: implement this
    return AMRS_OK;
}

uint8_t Amrs_Free(struct amrs_string *str)
{
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_FALSE)
    {
        return AMRS_DOUBLE_FREE;
    }
    str->len = 0;
    str->capacity = 0;
    str->is_allocated = 0;
    free(str->buffer);
    return AMRS_OK;
}

uint8_t Amrs_Init_Const_Str_Raw(struct amrs_string *str, uint32_t capacity, const char *init_str, uint32_t init_str_len)
{
    uint32_t status;
    if (init_str_len == 0) return AMRS_INVALID_INSERT_LENGTH;
    if (capacity < init_str_len) return AMRS_INSUFFICIENT_SPACE;

    status = Amrs_Init_Empty(str, capacity);
    if (status != AMRS_OK) return status;

    status = Amrs_Append_Const_Str_Raw(str, init_str, init_str_len);
    return status;
}

uint8_t Amrs_Init_Str_Raw(struct amrs_string *str, uint32_t capacity, char *raw_str)
{
    uint32_t status;
    uint32_t raw_str_len = strlen(raw_str);
    if (raw_str_len == 0) return AMRS_INVALID_INSERT_LENGTH;
    if (capacity < raw_str_len) return AMRS_INSUFFICIENT_SPACE;

    status = Amrs_Init_Empty(str, capacity);
    if (status != AMRS_OK) return status;

    status = Amrs_Append_Str_Raw(str, raw_str);
    return status;
}

struct amrs_result_char Amrs_Index(struct amrs_string str, uint32_t index)
{
    struct amrs_result_char res = {0};
    if (Amrs_Is_Allocated(str) == AMRS_ALLOCATED_FALSE)
    {
        res.status = AMRS_ALLOCATED_FALSE;
        return res;
    }

    if (index >= str.len)
    {
        res.status = AMRS_INVALID_INDEX;
    }

    res.val = str.buffer[index];
    res.status = AMRS_OK;
    return res;
}

struct amrs_result_u32 Amrs_Find_Char_From(struct amrs_string str, uint32_t start_index, char to_find)
{
    struct amrs_result_u32 result = {0};
    if (Amrs_Is_Allocated(str) == AMRS_ALLOCATED_FALSE)
    {
        result.status = AMRS_ALLOCATED_FALSE;
        return result;
    }

    for (uint32_t i = start_index; i < str.len; i++)
    {
        if (str.buffer[i] == to_find)
        {
            result.status = AMRS_OK;
            result.val = i;
            return result;
        }
    }

    result.status = AMRS_NO_MATCH_FOUND;
    return result;
}

uint8_t Amrs_Append_Const_Str_Raw(struct amrs_string *str, const char *to_append, uint32_t to_append_len)
{
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_FALSE)
    {
        return AMRS_ALLOCATED_FALSE;
    }
    uint32_t available_size = str->capacity - str->len;

    if (to_append_len == 0) return AMRS_INVALID_INSERT_LENGTH;                         // invalid insert length
    if (available_size < to_append_len) return AMRS_INSUFFICIENT_SPACE;             // insufficient space in str
    
    memcpy(&(str->buffer[str->len]), to_append, to_append_len);
    str->len = str->len + to_append_len;

    return AMRS_OK;
};

uint8_t Amrs_Append_Str_Raw(struct amrs_string *str, char *to_append)
{
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_FALSE)
    {
        return AMRS_ALLOCATED_FALSE;
    }

    uint32_t available_size = str->capacity - str->len;
    uint32_t to_append_len = strlen(to_append);
    if (to_append_len == 0) return AMRS_INVALID_INSERT_LENGTH;                         // invalid insert length
    if (available_size < to_append_len) return AMRS_INSUFFICIENT_SPACE;             // insufficient space in str
    
    memcpy(&(str->buffer[str->len]), to_append, to_append_len);
    str->len = str->len + to_append_len;

    return AMRS_OK;
};

uint8_t Amrs_Append_Str(struct amrs_string *str, struct amrs_string *to_append)
{
    if (Amrs_Is_Allocated(*to_append) == AMRS_ALLOCATED_FALSE) {
        return AMRS_ALLOCATED_FALSE;
    }
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_FALSE) {
        return AMRS_ALLOCATED_FALSE;
    }
    
    if (to_append->len > str->capacity) {
        return AMRS_INSUFFICIENT_SPACE;
    }
    
    memcpy(&(str->buffer[str->len]), to_append->buffer, to_append->len);
    str->len = str->len + to_append->len;

    return AMRS_OK;
}

// @note: copy str might not be required
// as append_str functions the same way
uint8_t Amrs_Copy_Str(struct amrs_string *copy_from, struct amrs_string *copy_to)
{
    if (Amrs_Is_Allocated(*copy_from) == AMRS_ALLOCATED_FALSE) {
        return AMRS_ALLOCATED_FALSE;
    }
    if (Amrs_Is_Allocated(*copy_to) == AMRS_ALLOCATED_FALSE) {
        return AMRS_ALLOCATED_FALSE;
    }
    
    if (copy_from->len > copy_to->capacity) {
        return AMRS_INSUFFICIENT_SPACE;
    }
    
    memcpy(copy_to->buffer, copy_from->buffer, copy_from->len);
    copy_to->len = copy_from->len;

    return AMRS_OK;
}

uint8_t Amrs_Append_Str_In_Range(
        struct amrs_string *str, struct amrs_string to_append,
        uint32_t start_index, uint32_t end_index)
{
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_FALSE) {
        return AMRS_ALLOCATED_FALSE;
    }
    if (Amrs_Is_Allocated(to_append) == AMRS_ALLOCATED_FALSE) {
        return AMRS_ALLOCATED_FALSE;
    }
    // check if index range is valid
    if (start_index >= end_index) {
        return AMRS_INVALID_INDEX_RANGE;
    }
    // check if indexes fall outside of to_append
    if (to_append.len < start_index || to_append.len < end_index) {
        return AMRS_INVALID_INDEX;
    }
    // check if str has insufficient capacity
    uint32_t available_capacity = to_append.capacity - to_append.len;
    uint32_t to_append_len = end_index - start_index;
    if (available_capacity < to_append_len) {
        return AMRS_INSUFFICIENT_SPACE;
    }

    memcpy(&(str->buffer[str->len]), &(to_append.buffer[start_index]), to_append_len);
    str->len = str->len + to_append_len;

    return AMRS_OK;
}

struct amrs_result_u32 Amrs_Find_Const_Substring_Raw(
    struct amrs_string *str, 
    const char *raw_substr, uint32_t raw_substr_len)
{
    struct amrs_result_u32 result = {0};
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_FALSE)
    {
        result.status = AMRS_ALLOCATED_FALSE;
        return result;
    }
    if (str->len < raw_substr_len) {
        result.status = AMRS_INSUFFICIENT_SPACE;
        return result;
    }

    const char *iter_substr = raw_substr;

    uint8_t is_match_found = 0;
    uint8_t is_match_starting = 0;
    
    uint32_t index_substr = 0;
    uint32_t match_count = 0;

    for (uint32_t i = 0; i < str->len; i++)
    {
        if (str->buffer[i] == *iter_substr)
        {
            if (!is_match_starting) {
                // char matches, beginning matching check
                is_match_starting = 1;
                index_substr = i;
            }
            match_count++;
            iter_substr++;
        }
        else
        {
            // not match, reset substr matching state
            is_match_starting = 0;
            index_substr = 0;
            match_count = 0;
            iter_substr = raw_substr;
        }

        if (match_count == raw_substr_len)
        {
            // is entire word matched
            is_match_found = 1;
            break;
        }
    }

    if (is_match_found) {
        result.val = index_substr;
        result.status = AMRS_OK;
    }
    else
    {
        result.status = AMRS_NO_MATCH_FOUND;
    }
    return result;
}

uint8_t Amrs_Replace_Const_Str_Raw(struct amrs_string *str, uint32_t replace_at,
        const char *replace_with, uint32_t replace_with_len)
{
    if (Amrs_Is_Allocated(*str) == AMRS_ALLOCATED_FALSE)
    {
        return AMRS_ALLOCATED_FALSE;
    }

    if (replace_at > str->len)
    {
        return AMRS_INVALID_REPLACEMENT_INDEX;
    }

    uint32_t available_space = str->capacity - replace_at;
    if (available_space < replace_with_len)
    {
        return AMRS_INSUFFICIENT_SPACE;
    }

    uint32_t replace_with_max_str_index = replace_at + replace_with_len;
    uint32_t str_replace_with_len_increment = 0;
    if (replace_with_max_str_index > str->len)
    {
        str_replace_with_len_increment = replace_with_max_str_index - str->len;
    }

    memcpy(&str->buffer[replace_at], replace_with, replace_with_len);
    str->len = str->len + str_replace_with_len_increment;

    return AMRS_OK;
}
