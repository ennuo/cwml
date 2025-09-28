#include <filepath.h>

#include <DebugLog.h>
#include <StringUtil.h>

CFilePath CFilePath::Empty;

CFilePath gGameDataPath;
CFilePath gBaseDir;
CFilePath gSysCachePath;
CFilePath gModLoaderPath;
CFilePath gPluginPath;

char* PrependPath(char* dst, const char* filename, const char* path)
{
	if (*filename == '/') filename += 1;

	int len = StringLength(path);
	bool append_slash = len == 0 || path[len - 1] != '/';

	strcpy(dst + len + (append_slash ? 1 : 0), filename);
	if (path != dst)
		strncpy(dst, path, len);
	if (append_slash)
		dst[len] = '/';
	
	return dst;
}

CFilePath::CFilePath()
{
	*Filepath = '\0';
	Invalid = true;
}

CFilePath::CFilePath(const char* filename)
{
	Assign(filename);
}

CFilePath::CFilePath(EFilePathRootDir root_dir, const char* filename)
{
	Assign(root_dir, filename);
}

CFilePath::CFilePath(const CFilePath& rhs)
{
	*this = rhs;
}

CFilePath& CFilePath::operator=(const CFilePath& rhs)
{
	Assign(rhs.Filepath);
	return *this;
}

CFilePath& CFilePath::operator=(const char* rhs)
{
	Assign(rhs);
	return *this;
}

void CFilePath::Append(const char* str)
{
	if (IsEmpty())
	{
		AppendRaw(str);
		return;
	}

	char c = Filepath[Length() - 1];
	if (c == '/') AppendRaw(*str == '/' ? str + 1 : str);
	else
	{
		if (*str != '/') AppendRaw("/");
		AppendRaw(str);
	}
}

void CFilePath::AppendRaw(const char* str)
{
	int len = StringAppend(Filepath, str);
	Invalid = len > MAX_PATH - 1;
}

void CFilePath::Clear()
{
	Assign("");
}

void CFilePath::Assign(EFilePathRootDir root_dir, const char* filename)
{
	Invalid = false;
	switch (root_dir)
	{
		case FPR_GAMEDATA:
		{
			PrependPath(Filepath, filename, gGameDataPath.c_str());
			break;
		}
		case FPR_BLURAY:
		{
			PrependPath(Filepath, filename, gBaseDir.c_str());
			break;
		}
		case FPR_SYSCACHE:
		{
			PrependPath(Filepath, filename, gSysCachePath.c_str());
			break;
		}
		case FPR_MODLOADER:
		{
			PrependPath(Filepath, filename, gModLoaderPath.c_str());
			break;
		}
		case FPR_PLUGIN:
		{
			PrependPath(Filepath, filename, gPluginPath.c_str());
			break;
		}
		default:
		{
			Invalid = true;
			break;
		}
	}
}

void CFilePath::Assign(const char* filename)
{
	int len = StringCopy<char, MAX_PATH>(Filepath, filename);
	Invalid = len > MAX_PATH - 1;
}

const char* CFilePath::GetExtension() const
{
	const char* s = strrchr(Filepath, '.');
	return s != NULL ? s : "";
}

const char* CFilePath::GetFilename() const
{
	const char* s = strrchr(Filepath, '/');
	return s != NULL ? s + 1 : "";
}

void CFilePath::FixSlashesAndCase()
{
	for (char* it = Filepath; *it != '\0'; ++it)
	{
		if (*it == '\\') *it = '/';
		else *it = tolower(*it);
	}
}

void CFilePath::StripExtension()
{
	char* s = strrchr(Filepath, '.');
	if (s != NULL)
		*s = '\0';
}

void CFilePath::StripTrailingSlash()
{
	int len = strlen(Filepath);
	if (len != 0 && Filepath[len - 1] == '/' || Filepath[len - 1] == '\\')
		Filepath[len - 1] = '\0';
}

bool FileLoad(const CFilePath& f, ByteArray& out, CHash* out_hash)
{
	FileHandle h;
	if (!FileOpen(f, h, OPEN_READ)) return false;

	int size = FileSize(f);
	out.resize(size);

	if (size == 0)
	{
		FileClose(h);
		return true;
	}

	if (FileRead(h, out.begin(), size) != size)
	{
		FileClose(h);
		return false;
	}

	FileClose(h);
	if (out_hash != NULL)
		*out_hash = CHash((const uint8_t*)out.begin(), out.size());

	return true;
}

bool StripAndIgnoreFileHash(TextRange<char>& range)
{
    const char* c = range.Begin;
    while (c != range.End && *c != '#') c++;
    range.End = c;
    range.TrimWhite();
    return range.Valid();
}

u32 GetLine(const ByteArray& bytes, u32 start_offs, TextRange<char>& range)
{
	const char* s = bytes.begin() + start_offs;
	range.Begin = s;
	while (s != bytes.end() && *s != '\n' && *s != '\r') s++;
	range.End = s;
	while (*s == '\n' || *s == '\r') s++;
	return s - bytes.begin();
}

bool LinesLoad(const ByteArray& bytes, CVector<MMString<char> >& out, ParseFn parsefunc)
{
	u32 offset = 0;
	while (offset < bytes.size())
	{
		TextRange<char> range;
		offset = GetLine(bytes, offset, range);
		if (range.Valid() && (parsefunc == NULL || parsefunc(range)))
		{
			out.resize(out.size() + 1);
			out.back().assign(range.Begin, range.Length());
		}
	}

	return true;
}

bool FileLoad(const CFilePath& path, CVector<MMString<char> >& out, ParseFn parsefunc)
{
	ByteArray bytes;
	if (!FileLoad(path, bytes, NULL)) return false;
	return LinesLoad(bytes, out, parsefunc);
}