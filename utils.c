void QInsertChars(char* InsertSource, char* InsertDest, B32 AddNull) {
    // NOTE(talha): insert characters at source pointer location
    while (*InsertSource != 0) {
        *InsertDest++ = *InsertSource++;
    }
    if (AddNull) *InsertDest++ = 0;
};

void QCopyUntilString(const char* CopySource, char **CopyDest, char* ToFind)
{
    // NOTE(talha): copies target to source up till and including ToFind
    char *copy = *CopyDest;
    const char *i = CopySource;
    char *t = ToFind; 
    B32 found = 0;
    while(*i != 0 && !found) { 
        *copy++ = *i;
        while(*i++ == *t) {
            if (*++t == 0) {
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

B32 QFindSubStr(char *pattern, char **src)
{
  char *srcPtr = *src;
  char *patternPtr = pattern;
  B8 found = 0;
  while (*srcPtr != 0 && found == 0)
  {
    if (*srcPtr == *patternPtr)
    {
      while (*srcPtr == *patternPtr && *srcPtr != 0 && *patternPtr != 0)
      {
        ++srcPtr;
        ++patternPtr;
      }
      if (*patternPtr == 0)
      {
        found = 1;
      }
      else if (*srcPtr != 0)
      {
        patternPtr = pattern;
      }
      else
      {
        break;
      }
    }
    else
    {
      ++srcPtr;
    }
  }
  return found;
}

I32 AppendToPath(char **subPath, char *path, char *toAppend, B8 isDir)
{
  char *_dir_ptr = *subPath;
  I32 bytesWritten = 0;
  bytesWritten += QCopyStringMoveDest(path, &_dir_ptr); 
  if (*(_dir_ptr-1) != '/') 
  {
    *_dir_ptr++ = '/';
    ++bytesWritten;
  }
  bytesWritten += QCopyStringMoveDest(toAppend, &_dir_ptr);
  /**
   * a case occurs where data from a previous loop iteration still exists
   * at a memory location. 
   * So, im just null terminating my strings so their data remains correct regardless
   * of if memory assigned is empty of not
   */
  if (isDir) 
  {
    if (*(_dir_ptr-1) != '/') 
    {
      *_dir_ptr++ = '/';
      ++bytesWritten;
    }
  }
  *_dir_ptr = 0; // prevents buggy memory
  return bytesWritten;
}
