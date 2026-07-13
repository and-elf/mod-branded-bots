#include "mod_branded_bots_loader.h"
#include "BrandedBotsMgr.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace BrandedBots;

// Loads/refreshes the bot-branding config on startup and on `.reload config`.
class BrandedBotsWorldScript : public WorldScript
{
public:
    BrandedBotsWorldScript() : WorldScript("BrandedBotsWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sBrandedBotsMgr->LoadConfig();
    }
};

// Assigns a brand to random bots as they log in (proc-based; no bot-AI involvement).
class BrandedBotsPlayerScript : public PlayerScript
{
public:
    BrandedBotsPlayerScript() : PlayerScript("BrandedBotsPlayerScript") { }

    void OnPlayerLogin(Player* player) override
    {
        sBrandedBotsMgr->OnBotLogin(player);
    }
};

void AddBrandedBotsScripts()
{
    new BrandedBotsWorldScript();
    new BrandedBotsPlayerScript();
}
