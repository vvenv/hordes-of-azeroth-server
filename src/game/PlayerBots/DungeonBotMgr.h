/*
 * Fills a player's party with party bots when they enter a five-player
 * dungeon, and sends the bots home when they leave it alive.
 *
 * The manager polls the online players from the world thread (it runs from
 * World::Update next to sPlayerBotMgr), so no teleport or map code needs a
 * hook and nothing touches a map from a map thread.
 *
 * Each character keeps two settings in `character_party_bots`: the role the
 * player plays (auto = guessed from class and talents) and whether dungeons
 * fill on their own. `.bots` (open to every account) changes them, fills or
 * dismisses by hand, and the GMBox addon drives it from its Party tab.
 */

#ifndef MANGOS_DUNGEON_BOT_MGR_H
#define MANGOS_DUNGEON_BOT_MGR_H

#include "Common.h"
#include "SharedDefines.h"
#include <map>
#include <unordered_map>

class Player;

class DungeonBotMgr
{
public:
    static DungeonBotMgr& Instance();

    void Update(uint32 diff);

    // The role the player plays: their own choice, else a guess.
    CombatBotRoles GetPlayerRole(Player* pPlayer);
    CombatBotRoles GetChosenRole(Player* pPlayer);
    void SetChosenRole(Player* pPlayer, CombatBotRoles role);
    bool IsAutoFill(Player* pPlayer);
    void SetAutoFill(Player* pPlayer, bool on);

    // Adds bots for the missing roles up to a party of five. Returns how many
    // were added; `why` says why none were when it returns 0.
    uint32 Fill(Player* pPlayer, std::string& why);
    // Sends away the bots this player leads: only the automatic ones, or all.
    uint32 Dismiss(Player* pPlayer, bool autoOnly);

    static CombatBotRoles GuessRole(Player const* pPlayer);
    static char const* RoleName(CombatBotRoles role, bool zh);

private:
    struct Settings
    {
        CombatBotRoles role = ROLE_INVALID;
        bool autoFill = true;
    };
    struct Watch
    {
        uint32 lastMap = UINT32_MAX;
        uint32 fillAt = 0;          // world ms; 0 = no fill pending
    };
    struct AutoBot
    {
        CombatBotRoles role = ROLE_INVALID;
        uint8 botClass = 0;
        uint32 lastSeen = 0;        // world ms: spawned, or last found in the world
    };

    Settings& GetSettings(Player* pPlayer);
    void SaveSettings(Player* pPlayer);
    void ForgetGoneBots(uint32 leaderLow);

    std::unordered_map<uint32, Settings> m_settings;            // by character guid
    std::unordered_map<uint32, Watch> m_watch;                  // by character guid
    std::unordered_map<uint32, std::map<uint32, AutoBot>> m_autoBots; // leader -> bot guid -> bot
    uint32 m_timer = 0;
    uint32 m_now = 0;
};

#define sDungeonBotMgr DungeonBotMgr::Instance()

#endif
