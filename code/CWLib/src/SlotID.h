#pragma once

#include "SlotEnums.inl"

class CSlotID {
public:
    inline bool operator<(CSlotID const& rhs) const 
    {
        if (Type == rhs.Type)
            return SlotNumber < rhs.SlotNumber;
        
        return Type < rhs.Type;
    }

    inline bool operator==(CSlotID const& rhs) const
    {
        return Type == rhs.Type && SlotNumber == rhs.SlotNumber;
    }

    inline bool operator!=(CSlotID const& rhs) const
    {
        return Type != rhs.Type && SlotNumber != rhs.SlotNumber;
    }

    inline bool Empty() const
    {
        return SlotNumber == 0;
    }

    inline bool IsGroup() const
    {
        return Type == LEVEL_TYPE_LOCAL_GROUP || Type == LEVEL_TYPE_DEVELOPER_GROUP;
    }

    inline bool IsDLC() const
    {
        return Type == LEVEL_TYPE_DLC_LEVEL || Type == LEVEL_TYPE_DLC_PACK;
    }
public:
    u32 Type;
    u32 SlotNumber;
};