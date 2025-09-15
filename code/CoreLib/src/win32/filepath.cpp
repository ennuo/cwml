#include <filepath.h>
#include <DebugLog.h>

bool FileOpen(const CFilePath& fp, FileHandle& fd, EOpenMode mode)
{
    fd = INVALID_FILE_HANDLE;
    switch (mode)
    {
        case OPEN_READ:
        {
            fd = CreateFile(fp.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            break;
        }
        case OPEN_WRITE:
        {
            fd = CreateFile(fp.c_str(), GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            break;
        }
        case OPEN_APPEND:
        {
            fd = CreateFile(fp.c_str(), GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            break;
        }
        case OPEN_RDWR:
        {
            fd = CreateFile(fp.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            break;
        }
    }

    if (fd == INVALID_FILE_HANDLE)
        MMLogCh(DC_RESOURCE, "Failed CreateFile(%s) %d\n", fp.c_str(), GetLastError());
    
    return fd != INVALID_FILE_HANDLE;
}

u64 FileRead(FileHandle h, void* out, u64 count)
{
    DWORD n;
    if (ReadFile(h, out, count, &n, NULL))
        return n;
    return 0;
}

u64 FileWrite(FileHandle h, const void* bin, u64 count)
{
    DWORD n;
    if (!WriteFile(h, bin, count, &n, NULL))
        MMLogCh(DC_RESOURCE, "Failed FileWrite %d\n", GetLastError());
    return n;
}

u64 FileSeek(FileHandle h, s64 newpos, u32 whence)
{
    DWORD p = SetFilePointer(h, newpos, NULL, whence);
    // MMLog("newpos=%08x, res=%08x\n", (s32)newpos, (s32)p);
    return p;

}

u64 FileSize(const CFilePath& fp)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(fp.c_str(), GetFileExInfoStandard, &fad))
        return 0;
    
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}

u64 FileSize(FileHandle h)
{
    return GetFileSize(h, NULL);
}

bool FileExists(const CFilePath& fp)
{
    DWORD attrib = GetFileAttributes(fp.c_str());
    return attrib != INVALID_FILE_ATTRIBUTES;
}

bool FileResize(FileHandle h, u32 newsize)
{
    if (SetFilePointer(h, newsize, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        return false;
    bool success = SetEndOfFile(h);
    SetFilePointer(h, 0, NULL, FILE_BEGIN);
    return success;
}

void FileClose(FileHandle& h)
{
    CloseHandle(h);
    h = INVALID_FILE_HANDLE;
}

bool FileResizeNoZeroFill(const CFilePath& fp, u32 newsize)
{
    u64 size = FileSize(fp);
    if (size == 0)
    {
        FileHandle fd;
        if (!FileOpen(fp, fd, OPEN_WRITE)) return false;
        FileClose(fd);
    }

    bool res = true;
    if (size < newsize)
    {
        FileHandle fd;
        if (!FileOpen(fp, fd, OPEN_APPEND))
            return false;
        
        res = FileResize(fd, newsize);
        FileClose(fd);

    }

    return res;
}

bool FileSync(FileHandle fd)
{
    return true;
}

bool DirectoryCreate(const char* dirname)
{
    if (CreateDirectory(dirname, NULL)) return true;
    return GetLastError() == ERROR_ALREADY_EXISTS;
}