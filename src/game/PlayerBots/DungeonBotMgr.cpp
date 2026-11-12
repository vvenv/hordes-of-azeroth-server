/*
 * Party bots that join on their own in five-player dungeons. See
 * DungeonBotMgr.h for the rules.
 */

#include "DungeonBotMgr.h"
#include "PartyBotAI.h"
#include "PlayerBotMgr.h"
#include "Player.h"
#include "Group.h"
#include "Map.h"
#include "World.h"
#include "ObjectAccessor.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "Util.h"
#include "Utilities/Random.h"
#include <algorithm>
#include <vector>

uint8 SelectRandomRaceForClass(uint8 playerClass, Team playerTeam); // PlayerBotMgr.cpp

namespace
{
    uint32 const UPDATE_EVERY_MS = 500;
    // The client finishes loading the dungeon before the bots appear.
    uint32 const FILL_DELAY_MS = 3000;
    // A fill that waits for a ghost to come back to life gives up after this.
    uint32 const FILL_GIVE_UP_MS = 120000;
    // A bot that never shows up in the group is forgotten after this.
    uint32 const BOT_LOGIN_GRACE_MS = 30000;
    uint32 const PARTY_SIZE = 5;

    enum : uint32
    {
        SPELL_SHIELD_SLAM      = 23922,
        SPELL_HOLY_SHIELD      = 20925,
        SPELL_HOLY_SHOCK       = 20473,
        SPELL_SHADOWFORM       = 15473,
        SPELL_MOONKIN_FORM     = 24858,
        SPELL_SWIFTMEND        = 18562,
        SPELL_NATURES_SWIFT_DR = 17116,
        SPELL_MANA_TIDE        = 16190,
        SPELL_STORMSTRIKE      = 17364,
    };

    bool IsDps(CombatBotRoles role)
    {
        return role == ROLE_MELEE_DPS || role == ROLE_RANGE_DPS || role == ROLE_INVALID;
    }

    bool HasShield(Player const* pPlayer)
    {
        Item* pItem = pPlayer->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        return pItem && pItem->GetProto()->InventoryType == INVTYPE_SHIELD;
    }

    // The client's language. GetSessionDbcLocale() is the server's DBC locale
    // (enUS here), so the client's own locale comes from its text index.
    bool IsZh(WorldSession const* pSession)
    {
        if (!pSession)
            return false;
        int const index = pSession->GetSessionDbLocaleIndex();
        return index >= 0 && sObjectMgr.GetLocaleForIndex(index) == LOCALE_zhCN;
    }

    // The party bot AI that runs a player, if it is one.
    PartyBotAI* GetPartyBotAI(Player* pPlayer)
    {
        if (!pPlayer || !pPlayer->IsBot() || !pPlayer->AI())
            return nullptr;
        return dynamic_cast<PartyBotAI*>(pPlayer->AI());
    }
}

DungeonBotMgr& DungeonBotMgr::Instance()
{
    static DungeonBotMgr instance;
    return instance;
}

char const* DungeonBotMgr::RoleName(CombatBotRoles role, bool zh)
{
    switch (role)
    {
        case ROLE_TANK:      return zh ? "坦克" : "tank";
        case ROLE_HEALER:    return zh ? "治疗" : "healer";
        case ROLE_MELEE_DPS: return zh ? "近战输出" : "melee dps";
        case ROLE_RANGE_DPS: return zh ? "远程输出" : "ranged dps";
        default:             return zh ? "自动" : "auto";
    }
}

CombatBotRoles DungeonBotMgr::GuessRole(Player const* pPlayer)
{
    switch (pPlayer->GetClass())
    {
        case CLASS_WARRIOR:
            return (pPlayer->HasSpell(SPELL_SHIELD_SLAM) || HasShield(pPlayer)) ? ROLE_TANK : ROLE_MELEE_DPS;
        case CLASS_PALADIN:
            if (pPlayer->HasSpell(SPELL_HOLY_SHIELD))
                return ROLE_TANK;
            if (pPlayer->HasSpell(SPELL_HOLY_SHOCK))
                return ROLE_HEALER;
            return ROLE_MELEE_DPS;
        case CLASS_PRIEST:
            return pPlayer->HasSpell(SPELL_SHADOWFORM) ? ROLE_RANGE_DPS : ROLE_HEALER;
        case CLASS_DRUID:
            if (pPlayer->HasSpell(SPELL_SWIFTMEND) || pPlayer->HasSpell(SPELL_NATURES_SWIFT_DR))
                return ROLE_HEALER;
            return pPlayer->HasSpell(SPELL_MOONKIN_FORM) ? ROLE_RANGE_DPS : ROLE_MELEE_DPS;
        case CLASS_SHAMAN:
            if (pPlayer->HasSpell(SPELL_MANA_TIDE))
                return ROLE_HEALER;
            return pPlayer->HasSpell(SPELL_STORMSTRIKE) ? ROLE_MELEE_DPS : ROLE_RANGE_DPS;
        case CLASS_ROGUE:
            return ROLE_MELEE_DPS;
        default:
            return ROLE_RANGE_DPS;
    }
}

// ---------------------------------------------------------------- settings --

DungeonBotMgr::Settings& DungeonBotMgr::GetSettings(Player* pPlayer)
{
    uint32 const guid = pPlayer->GetGUIDLow();
    auto itr = m_settings.find(guid);
    if (itr != m_settings.end())
        return itr->second;

    Settings& s = m_settings[guid];
    if (std::unique_ptr<QueryResult> result = CharacterDatabase.PQuery(
            "SELECT `role`, `auto_fill` FROM `character_party_bots` WHERE `guid` = %u", guid))
    {
        Field* fields = result->Fetch();
        uint8 const role = fields[0].GetUInt8();
        s.role = role <= ROLE_HEALER ? CombatBotRoles(role) : ROLE_INVALID;
        s.autoFill = fields[1].GetUInt8() != 0;
    }
    return s;
}

void DungeonBotMgr::SaveSettings(Player* pPlayer)
{
    Settings const& s = GetSettings(pPlayer);
    CharacterDatabase.PExecute("REPLACE INTO `character_party_bots` (`guid`, `role`, `auto_fill`) VALUES (%u, %u, %u)",
        pPlayer->GetGUIDLow(), uint32(s.role), s.autoFill ? 1u : 0u);
}

CombatBotRoles DungeonBotMgr::GetChosenRole(Player* pPlayer)
{
    return GetSettings(pPlayer).role;
}

CombatBotRoles DungeonBotMgr::GetPlayerRole(Player* pPlayer)
{
    CombatBotRoles const chosen = GetChosenRole(pPlayer);
    return chosen != ROLE_INVALID ? chosen : GuessRole(pPlayer);
}

void DungeonBotMgr::SetChosenRole(Player* pPlayer, CombatBotRoles role)
{
    GetSettings(pPlayer).role = role;
    SaveSettings(pPlayer);
}

bool DungeonBotMgr::IsAutoFill(Player* pPlayer)
{
    return GetSettings(pPlayer).autoFill;
}

void DungeonBotMgr::SetAutoFill(Player* pPlayer, bool on)
{
    GetSettings(pPlayer).autoFill = on;
    SaveSettings(pPlayer);
}

// -------------------------------------------------------------- fill/dismiss --

void DungeonBotMgr::ForgetGoneBots(uint32 leaderLow)
{
    auto itr = m_autoBots.find(leaderLow);
    if (itr == m_autoBots.end())
        return;

    for (auto bot = itr->second.begin(); bot != itr->second.end();)
    {
        // A bot between two maps (following its leader) is briefly not in the
        // world, so only one gone for a while is forgotten.
        if (ObjectAccessor::FindPlayerNotInWorld(ObjectGuid(HIGHGUID_PLAYER, bot->first)))
            bot->second.lastSeen = m_now;
        if (m_now - bot->second.lastSeen > BOT_LOGIN_GRACE_MS)
            bot = itr->second.erase(bot);
        else
            ++bot;
    }
    if (itr->second.empty())
        m_autoBots.erase(itr);
}

uint32 DungeonBotMgr::Fill(Player* pPlayer, std::string& why)
{
    bool const zh = IsZh(pPlayer->GetSession());

    if (pPlayer->IsTaxiFlying() || pPlayer->InBattleGround() || pPlayer->IsBeingTeleported())
    {
        why = zh ? "现在不能叫机器人（飞行中、战场里或传送中）。" : "Bots cannot join now (flying, in a battleground or teleporting).";
        return 0;
    }
    Map* pMap = pPlayer->GetMap();
    if (!pMap || pMap->IsRaid())
    {
        why = zh ? "团队副本里不会补机器人。" : "Bots do not join in raids.";
        return 0;
    }

    ForgetGoneBots(pPlayer->GetGUIDLow());

    // Who is in the party already, and as what.
    std::vector<CombatBotRoles> roles;
    std::vector<uint8> classes;
    Group* pGroup = pPlayer->GetGroup();
    if (pGroup)
    {
        if (pGroup->isRaidGroup())
        {
            why = zh ? "团队里不会补机器人。" : "Bots do not join a raid group.";
            return 0;
        }
        if (pGroup->GetLeaderGuid() != pPlayer->GetObjectGuid())
        {
            why = zh ? "只有队长能叫机器人。" : "Only the party leader can call bots.";
            return 0;
        }
        for (auto const& slot : pGroup->GetMemberSlots())
        {
            Player* pMember = sObjectAccessor.FindPlayer(slot.guid);
            if (!pMember)
            {
                roles.push_back(ROLE_INVALID);
                continue;
            }
            classes.push_back(pMember->GetClass());
            if (PartyBotAI* pAI = GetPartyBotAI(pMember))
                roles.push_back(pAI->GetRole());
            else
                roles.push_back(GetPlayerRole(pMember));
        }
    }
    else
    {
        roles.push_back(GetPlayerRole(pPlayer));
        classes.push_back(pPlayer->GetClass());
    }

    // Bots still logging in count too, or a second fill would double them.
    auto& mine = m_autoBots[pPlayer->GetGUIDLow()];
    for (auto const& bot : mine)
    {
        ObjectGuid const guid(HIGHGUID_PLAYER, bot.first);
        if (pGroup && pGroup->IsMember(guid))
            continue;
        roles.push_back(bot.second.role);
        classes.push_back(bot.second.botClass);
    }

    if (roles.size() >= PARTY_SIZE)
    {
        why = zh ? "队伍已经满了。" : "The party is already full.";
        return 0;
    }

    bool needTank = std::find(roles.begin(), roles.end(), ROLE_TANK) == roles.end();
    bool needHealer = std::find(roles.begin(), roles.end(), ROLE_HEALER) == roles.end();
    uint32 melee = uint32(std::count(roles.begin(), roles.end(), ROLE_MELEE_DPS));

    auto unused = [&classes](std::vector<uint8> pool)
    {
        std::vector<uint8> fresh;
        for (uint8 c : pool)
            if (std::find(classes.begin(), classes.end(), c) == classes.end())
                fresh.push_back(c);
        return fresh.empty() ? pool : fresh;
    };

    Team const team = pPlayer->GetTeam();
    uint32 const slots = PARTY_SIZE - uint32(roles.size());
    uint32 added = 0;
    for (uint32 i = 0; i < slots; ++i)
    {
        CombatBotRoles role;
        uint8 botClass;
        if (needTank)
        {
            role = ROLE_TANK;
            botClass = CLASS_WARRIOR;
            needTank = false;
        }
        else if (needHealer)
        {
            role = ROLE_HEALER;
            std::vector<uint8> pool = unused({ CLASS_PRIEST, CLASS_DRUID, uint8(team == ALLIANCE ? CLASS_PALADIN : CLASS_SHAMAN) });
            // Priests heal best of the four; take one when there is none yet.
            botClass = std::find(pool.begin(), pool.end(), CLASS_PRIEST) != pool.end() ? CLASS_PRIEST : SelectRandomContainerElement(pool);
            needHealer = false;
        }
        else
        {
            // At most two melee, so the tank is not crowded and cleaves hit fewer.
            std::vector<uint8> pool = melee >= 2
                ? std::vector<uint8>{ CLASS_MAGE, CLASS_HUNTER, CLASS_WARLOCK }
                : std::vector<uint8>{ CLASS_MAGE, CLASS_HUNTER, CLASS_WARLOCK, CLASS_ROGUE, CLASS_WARRIOR };
            botClass = SelectRandomContainerElement(unused(pool));
            role = (botClass == CLASS_ROGUE || botClass == CLASS_WARRIOR) ? ROLE_MELEE_DPS : ROLE_RANGE_DPS;
            if (role == ROLE_MELEE_DPS)
                ++melee;
        }

        uint8 const botRace = SelectRandomRaceForClass(botClass, team);
        if (!botRace)
            continue;

        float x, y, z;
        pPlayer->GetNearPoint(pPlayer, x, y, z, 0, 3.0f, frand(0.0f, 2 * M_PI_F));
        PartyBotAI* ai = new PartyBotAI(pPlayer, nullptr, role, botRace, botClass, pPlayer->GetLevel(),
            pPlayer->GetMapId(), pMap->GetInstanceId(), x, y, z, pPlayer->GetOrientation());
        if (!sPlayerBotMgr.AddBot(ai))
        {
            delete ai;
            continue;
        }
        AutoBot& bot = mine[uint32(ai->botEntry->playerGUID)];
        bot.role = role;
        bot.botClass = botClass;
        bot.lastSeen = m_now;
        classes.push_back(botClass);
        ++added;
    }

    if (!added)
        why = zh ? "机器人没能生成。" : "No bot could be spawned.";
    return added;
}

uint32 DungeonBotMgr::Dismiss(Player* pPlayer, bool autoOnly)
{
    uint32 count = 0;

    // The bots this manager brought, wherever they are. One that is between
    // two maps is not found now; it stays listed and goes on a later call.
    auto mine = m_autoBots.find(pPlayer->GetGUIDLow());
    if (mine != m_autoBots.end())
    {
        for (auto bot = mine->second.begin(); bot != mine->second.end();)
        {
            PartyBotAI* pAI = GetPartyBotAI(sObjectAccessor.FindPlayer(ObjectGuid(HIGHGUID_PLAYER, bot->first)));
            if (pAI && pAI->botEntry)
            {
                pAI->botEntry->requestRemoval = true;
                ++count;
                bot = mine->second.erase(bot);
            }
            else
                ++bot;
        }
        if (mine->second.empty())
            m_autoBots.erase(mine);
    }

    // Everything else this player leads: bots added with .partybot.
    if (!autoOnly)
    {
        if (Group* pGroup = pPlayer->GetGroup())
        {
            for (auto const& slot : pGroup->GetMemberSlots())
            {
                PartyBotAI* pAI = GetPartyBotAI(sObjectAccessor.FindPlayer(slot.guid));
                if (!pAI || pAI->m_leaderGuid != pPlayer->GetObjectGuid() || !pAI->botEntry || pAI->botEntry->requestRemoval)
                    continue;
                pAI->botEntry->requestRemoval = true;
                ++count;
            }
        }
    }
    return count;
}

// ------------------------------------------------------------------- update --

void DungeonBotMgr::Update(uint32 diff)
{
    m_now += diff;
    m_timer += diff;
    if (m_timer < UPDATE_EVERY_MS)
        return;
    m_timer = 0;

    std::vector<ObjectGuid> players;
    {
        HashMapHolder<Player>::ReadGuard guard(HashMapHolder<Player>::GetLock());
        for (auto const& itr : sObjectAccessor.GetPlayers())
            if (itr.second && !itr.second->IsBot())
                players.push_back(itr.first);
    }

    for (ObjectGuid const& guid : players)
    {
        Player* pPlayer = sObjectAccessor.FindPlayer(guid);
        if (!pPlayer || !pPlayer->IsInWorld() || pPlayer->IsBeingTeleported())
            continue;
        Map* pMap = pPlayer->GetMap();
        if (!pMap)
            continue;

        Watch& w = m_watch[guid.GetCounter()];
        bool const inDungeon = pMap->IsNonRaidDungeon();
        if (pMap->GetId() != w.lastMap)
        {
            w.lastMap = pMap->GetId();
            w.fillAt = (inDungeon && IsAutoFill(pPlayer)) ? m_now + FILL_DELAY_MS : 0;
        }

        if (!inDungeon)
        {
            w.fillAt = 0;
            // Out of the dungeon on their feet: the dungeon's bots go home.
            // A ghost on its corpse run keeps them.
            auto mine = m_autoBots.find(guid.GetCounter());
            if (pPlayer->IsAlive() && mine != m_autoBots.end() && !mine->second.empty())
            {
                ForgetGoneBots(guid.GetCounter());
                if (uint32 gone = Dismiss(pPlayer, true))
                {
                    bool const zh = IsZh(pPlayer->GetSession());
                    ChatHandler(pPlayer).PSendSysMessage(zh ? "离开副本，%u 个机器人队友解散了。" : "Left the dungeon: %u bot(s) went home.", gone);
                }
            }
            continue;
        }

        if (!w.fillAt || m_now < w.fillAt)
            continue;
        if (!pPlayer->IsAlive() || pPlayer->IsInCombat())
        {
            if (m_now - w.fillAt > FILL_GIVE_UP_MS)
                w.fillAt = 0;
            continue;
        }
        w.fillAt = 0;

        std::string why;
        bool const zh = IsZh(pPlayer->GetSession());
        if (uint32 added = Fill(pPlayer, why))
            ChatHandler(pPlayer).PSendSysMessage(zh ? "%u 个机器人队友加入了队伍（你是%s）。输入 /gm 打开队伍面板。" : "%u bot(s) joined your party (you play %s). Type /gm for the party panel.",
                added, RoleName(GetPlayerRole(pPlayer), zh));
        else if (!why.empty())
            ChatHandler(pPlayer).SendSysMessage(why.c_str());
    }

    // Players who logged out: forget where they were, so logging back in
    // inside a dungeon counts as entering it (their bots left with them).
    // (A player between two maps is not in the world but still online.)
    for (auto itr = m_watch.begin(); itr != m_watch.end();)
    {
        if (!ObjectAccessor::FindPlayerNotInWorld(ObjectGuid(HIGHGUID_PLAYER, itr->first)))
            itr = m_watch.erase(itr);
        else
            ++itr;
    }

    // Leaders who logged out: their bots leave on their own (PartyBotAI).
    for (auto itr = m_autoBots.begin(); itr != m_autoBots.end();)
    {
        if (!ObjectAccessor::FindPlayerNotInWorld(ObjectGuid(HIGHGUID_PLAYER, itr->first)))
            itr = m_autoBots.erase(itr);
        else
            ++itr;
    }
}

// ----------------------------------------------------------------- commands --

static bool BotsZh(ChatHandler* handler)
{
    WorldSession* s = handler->GetSession();
    return IsZh(s);
}

bool ChatHandler::HandleBotsStatusCommand(char* args)
{
    // `.bots status quiet`: only the machine line, for the GMBox addon.
    bool const quiet = args && std::string(args) == "quiet";
    Player* pPlayer = m_session ? m_session->GetPlayer() : nullptr;
    if (!pPlayer)
        return false;
    bool const zh = BotsZh(this);
    CombatBotRoles const chosen = sDungeonBotMgr.GetChosenRole(pPlayer);
    CombatBotRoles const playing = sDungeonBotMgr.GetPlayerRole(pPlayer);
    bool const autoFill = sDungeonBotMgr.IsAutoFill(pPlayer);
    // The first line is also read by the GMBox addon: keep its shape.
    PSendSysMessage("[bots] role=%s playing=%s auto=%u", DungeonBotMgr::RoleName(chosen, false),
        DungeonBotMgr::RoleName(playing, false), autoFill ? 1u : 0u);
    if (quiet)
        return true;
    if (zh)
        PSendSysMessage("你的职责：%s%s；进五人副本自动补机器人：%s。",
            DungeonBotMgr::RoleName(playing, true), chosen == ROLE_INVALID ? "（自动判断）" : "", autoFill ? "开" : "关");
    else
        PSendSysMessage("You play %s%s; five-player dungeons fill with bots: %s.",
            DungeonBotMgr::RoleName(playing, false), chosen == ROLE_INVALID ? " (guessed)" : "", autoFill ? "on" : "off");
    return true;
}

bool ChatHandler::HandleBotsRoleCommand(char* args)
{
    Player* pPlayer = m_session ? m_session->GetPlayer() : nullptr;
    if (!pPlayer)
        return false;
    std::string const arg = args ? args : "";
    CombatBotRoles role;
    if (arg == "tank")
        role = ROLE_TANK;
    else if (arg == "healer" || arg == "heal")
        role = ROLE_HEALER;
    else if (arg == "dps")
        role = CombatBotBaseAI::IsMeleeDamageClass(pPlayer->GetClass()) && pPlayer->GetClass() != CLASS_DRUID &&
               pPlayer->GetClass() != CLASS_SHAMAN ? ROLE_MELEE_DPS : ROLE_RANGE_DPS;
    else if (arg == "melee")
        role = ROLE_MELEE_DPS;
    else if (arg == "ranged")
        role = ROLE_RANGE_DPS;
    else if (arg == "auto")
        role = ROLE_INVALID;
    else
    {
        SendSysMessage(BotsZh(this) ? "用法：.bots role tank|healer|dps|auto" : "Usage: .bots role tank|healer|dps|auto");
        SetSentErrorMessage(true);
        return false;
    }
    sDungeonBotMgr.SetChosenRole(pPlayer, role);
    return HandleBotsStatusCommand(nullptr);
}

bool ChatHandler::HandleBotsAutoCommand(char* args)
{
    Player* pPlayer = m_session ? m_session->GetPlayer() : nullptr;
    if (!pPlayer)
        return false;
    bool on;
    if (!ExtractOnOff(&args, on))
    {
        SendSysMessage(BotsZh(this) ? "用法：.bots auto on|off" : "Usage: .bots auto on|off");
        SetSentErrorMessage(true);
        return false;
    }
    sDungeonBotMgr.SetAutoFill(pPlayer, on);
    return HandleBotsStatusCommand(nullptr);
}

bool ChatHandler::HandleBotsFillCommand(char* /*args*/)
{
    Player* pPlayer = m_session ? m_session->GetPlayer() : nullptr;
    if (!pPlayer)
        return false;
    std::string why;
    uint32 const added = sDungeonBotMgr.Fill(pPlayer, why);
    if (!added)
    {
        SendSysMessage(why.c_str());
        SetSentErrorMessage(true);
        return false;
    }
    PSendSysMessage(BotsZh(this) ? "%u 个机器人队友正在加入。" : "%u bot(s) on their way.", added);
    return true;
}

bool ChatHandler::HandleBotsDismissCommand(char* /*args*/)
{
    Player* pPlayer = m_session ? m_session->GetPlayer() : nullptr;
    if (!pPlayer)
        return false;
    uint32 const gone = sDungeonBotMgr.Dismiss(pPlayer, false);
    PSendSysMessage(BotsZh(this) ? "%u 个机器人队友解散了。" : "%u bot(s) went home.", gone);
    return true;
}
