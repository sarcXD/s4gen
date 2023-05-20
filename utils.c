void QInsertChars(char* InsertSource, char* InsertDest, B32 AddNull) {
    // NOTE(talha): insert characters at source pointer location
    while (*InsertSource != '\0') {
        *InsertDest++ = *InsertSource++;
    }
    if (AddNull) *InsertDest++ = '\0';
};

void QCopyUntilString(const char* CopySource, char **CopyDest, char* ToFind)
{
    // NOTE(talha): copies target to source up till and including ToFind
    char *copy = *CopyDest;
    const char *i = CopySource;
    char *t = ToFind; 
    B32 found = 0;
    while(*i != '\0' && !found) { 
        *copy++ = *i;
        while(*i++ == *t) {
            if (*++t == '\0') {
                found = 1;
                break;
            }
            *copy++ = *i;
        }
        t = ToFind;
    }
    *CopyDest = copy;
}

int QCopyString(char *CopySource, char **CopyDest)
{
    char *DestPtr = *CopyDest;
    int BytesWritten = 0;
    while (*CopySource != 0)
    {
        *DestPtr++ = *CopySource++;
        ++BytesWritten;
    }
    return BytesWritten;
}

// @description:
// copy source to dest
// move dest ptr so that it is 1 byte after the copied source
// @returns:
// bytes written
int QCopyStringMoveDest(char *CopySource, char **CopyDest)
{
    int BytesWritten = 0;
    char *DestPtr = *CopyDest;
    while (*CopySource != 0)
    {
        *DestPtr++ = *CopySource++;
        ++BytesWritten;
    }
    *CopyDest = DestPtr;
    return BytesWritten;
}

B32 QStrEqual(char *a, char *b)
{
  while (*a == *b && *a != 0 && *b != 0)
  {
    ++a;
    ++b;
  };
  return *a == *b;
}

B32 QFindSubStr(char *pattern, char *src)
{
  char *srcPtr = src;
  char *patternPtr = pattern;
  B8 found = 0;
  while (*srcPtr != 0 && found == 0)
  {
    if (*srcPtr == *patternPtr)
    {
      while (*srcPtr != *patternPtr && *srcPtr != 0 && *patternPtr != 0)
      {
        ++srcPtr;
        ++patternPtr;
      }
      if (*patternPtr == 0)
      {
        found = 1;
      }
      patternPtr = pattern;
      break;
    }
    else
    {
      ++srcPtr;
    }
  }
  return found;
}
