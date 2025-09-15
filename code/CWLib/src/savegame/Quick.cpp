#include <savegame/Quick.h>
#include <FartRO.h>
#include <Serialise.h>

ReflectReturn CheckFartRevision(u32 magic)
{
    const int FART_MAGIC = 0x464152;
    if (magic >> 8 != FART_MAGIC) return REFLECT_INVALID;

    int farc_version = magic & 0xff;
    if (farc_version != 'C')
    {
        farc_version -= '0';
        if (farc_version < 2) return REFLECT_INVALID;
        if (farc_version > 4) return REFLECT_FORMAT_TOO_NEW;
    }

    return REFLECT_OK;
}

u32 GetFartRevision(u32 magic)
{
    switch (magic)
    {
        case FARC: return 0;
        case FAR2: return 1;
        case FAR3: return 2;
        case FAR4: return 3;
    };

    return -1;
}

ReflectReturn CheckCache(FileHandle fd, SSaveKey& key, LocalUserID* user)
{
    ReflectReturn ret;

    FileSeek(fd, -sizeof(Footer), FILE_END);
    Footer footer;
    
    if (FileRead(fd, &footer, sizeof(Footer)) != sizeof(Footer))
        return REFLECT_FILEIO_FAILURE;
    
    if ((ret = CheckFartRevision(footer.magic)) != REFLECT_OK) return ret;
    int farc_version = GetFartRevision(footer.magic);

    bool hasSaveKey = false;
    bool hasBranchDescription = false;
    bool hasHashinate = false;
    bool hasLocalUser = false;

    if (farc_version >= 1)
    {
        hasLocalUser = true;
        hasSaveKey = true;
    }

    if (farc_version >= 2) hasHashinate = true;
    if (farc_version >= 3) hasBranchDescription = true;

    int fat_offset = (footer.count * sizeof(CFartRO::CFAT)) + sizeof(Footer);
    if (hasSaveKey) fat_offset += sizeof(SSaveKey);
    if (hasHashinate) fat_offset += sizeof(CHash);
    if (hasBranchDescription) fat_offset += sizeof(SRevision);
    if (hasLocalUser) fat_offset += sizeof(LocalUserID);

    if (FileSeek(fd, -fat_offset, FILE_END) == 0) return REFLECT_FILEIO_FAILURE;

    LocalUserID local_userid;
    if (user == NULL) user = &local_userid;
    
    return REFLECT_OK;
}

ReflectReturn CheckCache(const CFilePath& fart, SSaveKey& key, LocalUserID* user)
{
    FileHandle fd;
    if (!FileOpen(fart, fd, OPEN_READ)) return REFLECT_COULDNT_OPEN_FILE;

    ReflectReturn ret = CheckCache(fd, key, user);
    FileClose(fd);
    
    return ret;
}