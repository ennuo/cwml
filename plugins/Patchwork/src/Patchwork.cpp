#include <cwml/cwml.h>
#include <GuidHash.h>
#include <DebugLog.h>

const char* kRefreshServerURL = "http://lbp.lbpbonsai.com/lbp";
const char* kCustomServerDigest = "CustomServerDigest";
const char* kUserAgent = "PatchworkLBP2 1.2\0\0\0";

cwml::ConfigOption<bool> gAutoDiscover("Autodiscover", "autodiscover", false);
cwml::ConfigOption<const char*> gServerURL("Server URL", "url", kRefreshServerURL);
cwml::ConfigOption<const char*> gServerDigest("Server Digest", "digest", NULL);
cwml::ConfigOption<const char*> gLobbyPassword("Lobby Password", "password", NULL);

#define LBP2_HTTPS_URL_OFFSET   0x00c51b68
#define LBP2_HTTP_URL_OFFSET    0x00c51c08
#define LBP2_DIGEST_OFFSET      0x00c361d0
#define LBP2_USER_AGENT_OFFSET  0x00C36320
#define LBP2_NETWORK_KEY_OFFSET 0x00C248CC

class Patchwork : public cwml::Plugin {
public:
    const char* GetName() const { return "Patchwork"; }
    bool OnAttach()
    {
        if (gAutoDiscover)
        {
            MMLog("patchwork: autodiscover is unimplemented!\n");
            return false;
        }

        Ib_WriteNoCache(LBP2_HTTPS_URL_OFFSET, gServerURL.c_str(), gServerURL.aligned_size());
        Ib_WriteNoCache(LBP2_HTTP_URL_OFFSET, gServerURL.c_str(), gServerURL.aligned_size());
        if (gServerDigest.IsSet())
        {
            char digest[16];
            strncpy(digest, gServerDigest.c_str(), 16);
            Ib_WriteNoCache(LBP2_DIGEST_OFFSET, gServerDigest.c_str(), 16);
        }

        Ib_WriteNoCache(LBP2_USER_AGENT_OFFSET, kUserAgent, 0x14);
        
        if (gLobbyPassword.IsSet())
        {
            MMLog("patchwork: using private lobbies!\n");
            
            CHash hash(gLobbyPassword.c_str(), gLobbyPassword.size());
            Ib_WriteNoCache(LBP2_NETWORK_KEY_OFFSET, hash.GetBuf(), 0x10);
        }
        else
        {
            MMLog("patchwork: using public lobbies!\n");
        }

        MMLog("patchwork: patched server url to %s\n", gServerURL.c_str());

        return true;
    }
};

REGISTER_PLUGIN(Patchwork);