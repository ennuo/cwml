#include <Directory.h>

void DirectoryCreate(const CFilePath& src)
{
    if (src.IsEmpty()) return;

    char temp[1024];
    const char* s = src;
    const char* e = NULL;
    
    while ((e = strchr(s, '\\')) != NULL || (e = strchr(s, '/')) != NULL)
    {
        strcpy(temp, src);
        temp[e - src.c_str()] = '\0';
        DirectoryCreate(temp);
        
        s = e + 1;
        if (s == NULL || *s == '\0') 
            return;
    }

}