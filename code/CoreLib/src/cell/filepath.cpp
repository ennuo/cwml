#include <filepath.h>

#include <cell/fs/cell_fs_file_api.h>
#include <sys/return_code.h>

#include <DebugLog.h>
#include <thread.h>
#include <StringUtil.h>

bool FileOpen(const CFilePath& fp, FileHandle& fd, EOpenMode mode) 
{
	fd = INVALID_FILE_HANDLE;
	int ret;
	switch (mode) {
		case OPEN_READ: {
			ret = cellFsOpen(fp.c_str(), CELL_FS_O_RDONLY, &fd, NULL, 0); 
			break;
		}
		case OPEN_WRITE: {
			ret = cellFsOpen(fp.c_str(), CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, NULL, 0);
			break;
		}
		case OPEN_APPEND: {
			ret = cellFsOpen(fp.c_str(), CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_APPEND, &fd, NULL, 0);
			break;
		}
		case OPEN_RDWR: {
			ret = cellFsOpen(fp.c_str(), CELL_FS_O_RDWR |  CELL_FS_O_CREAT, &fd, NULL, 0);
			break;
		}
		default: return false;
	}

	if (ret != CELL_FS_OK) 
		MMLogCh(DC_RESOURCE, "Failed cellFsOpen(%s) %d\n", fp.c_str(), ret);

	return ret == CELL_FS_OK;

}

u64 FileRead(FileHandle h, void* out, u64 count) 
{
	u64 n;
	int ret = cellFsRead(h, out, count, &n);
	if (ret != CELL_FS_OK) 
    {
		MMLogCh(DC_RESOURCE, "Failed cellFsRead %d\n", ret);
		return 0;
	}
	return n;
}

u64 FileWrite(FileHandle h, const void* bin, u64 count)
{
	u64 n;
	cellFsWrite(h, bin, count, &n);
	return n;
}

u64 FileSeek(FileHandle h, s64 newpos, u32 whence)
{
	u64 p;
	cellFsLseek(h, newpos, whence, &p);
	return p;
}

bool FileStat(FileHandle h, u64& modtime, u64& size) 
{
	modtime = 0;
	size = 0;
	if (h != INVALID_FILE_HANDLE) 
	{
		CellFsStat status;
		if (cellFsFstat(h, &status) == CELL_FS_OK) 
		{
			modtime = status.st_mtime;
			size = status.st_size;
			return true;
		}
	}
	return false;
}

bool FileStat(const CFilePath& fp, u64& modtime, u64& size)
{
	modtime = 0;
	size = 0;
    CellFsStat status;
    if (cellFsStat(fp.c_str(), &status) == CELL_FS_OK) 
	{
        modtime = status.st_mtime;
        size = status.st_size;
        return true;
    }
	return false;
}

u64 FileSize(const CFilePath& fp)
{
	u64 modtime, size;
	if (!FileStat(fp, modtime, size)) return 0;
	return size;
}

bool FileExists(const CFilePath& fp)
{
    u64 modtime, size;
    return FileStat(fp, modtime, size);
}

bool FileSync(FileHandle h)
{
	int ret = cellFsFsync(h);
	if (ret != CELL_FS_OK)
		MMLogCh(DC_RESOURCE, "Failed cellFsFsync %d\n", ret);
	return ret == CELL_FS_OK;
}

bool FileResize(FileHandle h, u32 newsize)
{
	return cellFsFtruncate(h, newsize) == CELL_FS_OK;
}

bool FileResizeNoZeroFill(const CFilePath& fp, u32 newsize)
{
	u64 size = FileSize(fp);
	if (size == 0)
	{
		int fd;
		if (!FileOpen(fp, fd, OPEN_WRITE)) return false;
		FileClose(fd);
	}
	
	while (size < newsize)
	{
		size += 30000000;
		if (size > newsize)
			size = newsize;

		if (cellFsAllocateFileAreaWithoutZeroFill(fp, size) != CELL_FS_OK)
			return false;

		ThreadSleep(5);
	}

	return true;
}


void FileClose(FileHandle& h) 
{
	if (h != 0) cellFsClose(h);
	h = INVALID_FILE_HANDLE;
}

bool DirectoryOpen(const CFilePath& fp, DirHandle& fd)
{
	return cellFsOpendir(fp.c_str(), &fd) == CELL_FS_OK;
}

void DirectoryClose(DirHandle& fd)
{
	cellFsClosedir(fd);
	fd = INVALID_FILE_HANDLE;
}

bool DirectoryRead(DirHandle fd, char* out, u32 outsize)
{
	CellFsDirent dir;
	u64 n;

	if (cellFsReaddir(fd, &dir, &n) != CELL_FS_OK || n == 0) return false;

	strcpy(out, dir.d_name);
	out[dir.d_namlen] = '\0';

	return true;
}

bool DirectoryCreate(const char* dirname)
{
	int ret = cellFsMkdir(dirname, 0755);
	if (ret == CELL_FS_OK) return true;

	if (ret == EEXIST || ret == EISDIR)
	{
		CellFsStat sb;
		if (cellFsStat(dirname, &sb) == CELL_FS_OK)
			return (sb.st_mode & CELL_FS_S_IFMT) == CELL_FS_S_IFDIR;
	}
	
	return false;
}

int FileAttributes(const CFilePath& fp)
{
	CellFsStat st;
	if (cellFsStat(fp, &st) == CELL_FS_OK)
		return st.st_mode;
	return 0;
}