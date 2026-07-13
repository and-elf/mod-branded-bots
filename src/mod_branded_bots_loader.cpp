#include "mod_branded_bots_loader.h"

// Single entrypoint the modules loader calls (Add<dir-with-underscores>Scripts). Fans out to the
// feature registration so the adapter stays in its own translation unit.
void Addmod_branded_botsScripts()
{
    AddBrandedBotsScripts();
}
