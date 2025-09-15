#pragma nce

#include <MMString.h>
#include <new>

class CTextState;
typedef void (TextStateCallback)(CTextState*, u32);

class CTextState {
public:
    CTextState(int icon_mode, int title_key, int text_key, int button_mode, int progress_mode, void* userdata);

    inline CTextState()
    {
        memset(this, 0, sizeof(CTextState));
        new (&title) MMString<tchar_t>();
        new (&text) MMString<tchar_t>();
        active = true;
        pad = -1;
    }

public:
#ifdef LBP1
    u32 title_key;
    MMString<tchar_t> title;
    u32 text_key;
    MMString<tchar_t> text;
#else
    MMString<tchar_t> title;
    MMString<tchar_t> text;
    u32 a, b;
    u32 title_key;
    u32 text_key;
    u32 c;
#endif
    int button_mode;
    int icon_mode;
    int progress_mode;
    void* userdata;
    TextStateCallback* callback;
    int pad;
    float progress;
#ifndef LBP1
    bool active;
#endif
};

Ib_DeclarePort(SetActiveLoadingScreen, void*, CTextState const* text, TextStateCallback* callback, bool allow_if_other_active_screens);
Ib_DeclarePort(CancelActiveLoadingScreen, void, void* handle, bool call_callback, u32 result);