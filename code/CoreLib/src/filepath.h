#pragma once

#include <GuidHash.h>
#include <vector.h>
#include <TextRange.h>
#include <MMString.h>

#ifdef PS3
    #include <cell/fs/cell_fs_file_api.h>
    #define MAX_PATH (255)
    #define INVALID_FILE_HANDLE (-1)

    #define FILE_BEGIN (CELL_FS_SEEK_SET)
    #define FILE_CURRENT (CELL_FS_SEEK_CUR)
    #define FILE_END (CELL_FS_SEEK_END)
    
    typedef int FileHandle;
    typedef int DirHandle;
#endif

#ifdef WIN32
    #include <Windows.h>
    #define INVALID_FILE_HANDLE INVALID_HANDLE_VALUE
    typedef HANDLE FileHandle;
    typedef HANDLE DirHandle;
#endif

enum EFilePathRootDir 
{
	FPR_GAMEDATA,
	FPR_BLURAY,
	FPR_SYSCACHE,
    FPR_MODLOADER,
    FPR_PLUGIN,

    FPR_MAX
};

enum EOpenMode 
{
	OPEN_READ,
	OPEN_WRITE,
	OPEN_APPEND,
	OPEN_RDWR,
	OPEN_SECURE
};

class CFilePath {
public:
    static CFilePath Empty;
public:
    CFilePath();
    CFilePath(const char* filename);
    CFilePath(EFilePathRootDir root_dir, const char* filename);
    CFilePath(const CFilePath& rhs);
    
    void Assign(const char* filename);
    void Assign(EFilePathRootDir root_dir, const char* filename);

    void Append(const char* str);
    void AppendRaw(const char* str);

    const char* GetExtension() const;
    const char* GetFilename() const;
    
    void FixSlashesAndCase();
    void StripExtension();
    void StripTrailingSlash();
    void Clear();

    
    operator char const*() const { return Filepath; }
    
    inline bool IsEmpty() const { return *Filepath == '\0'; }
    inline int Length() const { return strlen(Filepath); }

    inline bool operator==(const CFilePath& rhs) { return strcmp(Filepath, rhs.Filepath) == 0; }
    inline bool operator!=(const CFilePath& rhs) { return strcmp(Filepath, rhs.Filepath) != 0; }

    inline bool IsValid() const { return !Invalid; }

    CFilePath& operator=(const CFilePath& rhs);
    CFilePath& operator=(const char* rhs);
    
    inline const char* c_str() const { return Filepath; }
private:
    char Filepath[MAX_PATH];
    bool Invalid;
};

extern CFilePath gGameDataPath;
extern CFilePath gBaseDir;
extern CFilePath gSysCachePath;
extern CFilePath gModLoaderPath;
extern CFilePath gPluginPath;

bool FileExists(const CFilePath& fp);
bool FileStat(FileHandle h, u64& modtime, u64& size);
bool FileStat(const CFilePath& fp, u64& modtime, u64& size);
u64 FileSize(const CFilePath& fp);
u64 FileSize(FileHandle h);

void FileClose(FileHandle& h);
bool FileOpen(const CFilePath& fp, FileHandle& fd, EOpenMode mode);
u64 FileRead(FileHandle h, void* out, u64 count);
u64 FileWrite(FileHandle h, const void* bin, u64 count);
u64 FileSeek(FileHandle h, s64 newpos, u32 whence);
bool FileResize(FileHandle h, u32 newsize);
bool FileResizeNoZeroFill(const CFilePath& fp, u32 newsize);
bool FileSync(FileHandle h);

bool DirectoryOpen(const CFilePath& fp, DirHandle& fd);
void DirectoryClose(DirHandle& fd);
bool DirectoryRead(DirHandle fd, char* out, u32 outsize);

int FileAttributes(const CFilePath& fp);

typedef bool (*ParseFn)(TextRange<char>&);
bool FileLoad(const CFilePath& f, ByteArray& out, CHash* out_hash);
bool FileLoad(const CFilePath& path, CVector<MMString<char> >& out, ParseFn parsefunc);

bool StripAndIgnoreFileHash(TextRange<char>& range);