void QInsertChars(char* InsertSource, char* InsertDest, b32 AddNull) {
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
    b32 found = 0;
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

void QCopyString(char *CopySource, char **CopyDest)
{
    char *DestPtr = *CopyDest;
    while (*CopySource != 0)
    {
        *DestPtr++ = *CopySource++;
    }
}

b32 QStrEqual(char *a, char *b)
{
  while (*a == *b && *a != 0 && *b != 0)
  {
    ++a;
    ++b;
  };
  return *a == *b;
}
