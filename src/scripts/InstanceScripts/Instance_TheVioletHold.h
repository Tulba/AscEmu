/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

const uint32_t VH_UPDATE_TIMER  = 1000;

// Violet hold during whole encounters has total 18 portals summoned
// one portal group is equal to 6 portals
enum DataIndex : uint8_t
{
    // This index is used to identify for event progress
    // This index should be set to "in progress" only after Sinclari's escort event
    INDEX_INSTANCE_PROGRESS = 0,

    // This index identifies portal count
    INDEX_PORTAL_COUNT,

    // First two bosses are random
    INDEX_GROUP1_BOSS,
    INDEX_GROUP2_BOSS,

    // Wave data
    DATA_WAVE_TYPE,
    DATA_LAST_PORTAL_ID,
    INDEX_WAVE_PROGRESS,

    // Final boss event
    INDEX_CYANIGOSA,

    INDEX_MAX
};

// Our custom state used by instance script
const uint32_t State_Failed = 5;

const Movement::Location SinclariPositions[] =
{
    { 1829.142f, 798.219f,  44.36212f, 0.122173f }, // 0 - Crystal
    { 1820.12f,  803.916f,  44.36466f, 0.0f      }, // 1 - Outside
    { 1816.185f, 804.0629f, 44.44799f, 3.176499f }, // 2 - Second Spawn Point
    { 1827.886f, 804.0555f, 44.36467f, 0.0f      }  // 3 - Outro
};

const Movement::LocationWithFlag AttackerWP[] =
{
    { { 1858.077f, 804.8599f, 44.00872f, 0 }, Movement::WP_MOVE_TYPE_RUN }, // Run to middle of entrance platform
    { { 1836.152f, 804.7064f, 44.2534f, 0 }, Movement::WP_MOVE_TYPE_RUN } // Run to door to attack it
};

// Location used to move guards and liutenant
const Movement::Location introMoveLoc = { 1817.377930f, 803.973633f, 44.363861f, 0 };

enum CreatureEntry : uint32_t
{
    //Main event
    CN_PORTAL_GUARDIAN                      = 30660,
    CN_DEFENSE_SYSTEM                       = 30837,
    CN_DEFENSE_SYSTEM_TRIGGER               = 30857,

    // Sinclari event
    CN_LIEUTNANT_SINCLARI                   = 30658,

    // Intro event
    CN_VIOLET_HOLD_GUARD                    = 30659,
    CN_INTRO_AZURE_BINDER_ARCANE            = 31007,
    CN_INTRO_AZURE_INVADER_ARMS             = 31008,
    CN_INTRO_AZURE_SPELLBREAKER_ARCANE      = 31009,
    CN_INTRO_AZURE_MAGE_SLAYER_MELEE        = 31010,
    CN_PORTAL_INTRO                         = 31011,

    // Portal npcs
    CN_PORTAL                               = 30679,
    CN_DOOR_SEAL                            = 30896,

    //Portal Guardians
    CN_AZURE_INVADER                        = 30661,
    CN_AZURE_SPELLBREAKER                   = 30662,
    CN_AZURE_BINDER                         = 30663,
    CN_AZURE_MAGE_SLAYER                    = 30664,
    CN_AZURE_CAPTAIN                        = 30666,
    CN_AZURE_SORCEROR                       = 30667,
    CN_AZURE_RAIDER                         = 30668,
    CN_AZURE_STALKER                        = 32191,

    // NPC with spell arcane spher
    CN_CRYSTAL_SYSTEM                       = 30837,

    // Zurumat event
    CN_ZURAMAT                              = 29314,
    CN_VOID_SENTRY                          = 29364,

    // Erekem event
    CN_EREKEM                               = 29315,
    CN_EREKEM_GUARD                         = 32228,

    //Bosses
    CN_MORAGG                               = 29316,
    CN_ICHORON                              = 29313,
    CN_XEVOZZ                               = 29266,
    CN_LAVANTHOR                            = 29312,
    CN_CYANIGOSA                            = 31134
};

const uint8_t maxVHBosses = 5;
const uint32_t randomVHBossArray[maxVHBosses] =
{
    CN_MORAGG, CN_ICHORON, CN_XEVOZZ, CN_LAVANTHOR, CN_CYANIGOSA
};

enum VH_achievements : uint32_t
{
    // event achiev - 1816
    ACHIEV_CRIT_DEFENSELES      = 6803,

    // Ichoron achiev - 2041
    ACHIEV_CRIT_DEHYDRATATION   = 7320,

    // Zuramat achiev - 2153
    ACHIEV_CRIT_VOID_DANCE      = 7587
};

// Violet hold portal locations
const uint8_t VHPortalLocCount = 6;
const Movement::Location VHPortalLocations[] =
{
    { 1877.51f, 850.104f, 44.6599f, 4.7822f  },
    { 1918.37f, 853.437f, 47.1624f, 4.12294f },
    { 1936.07f, 803.198f, 53.3749f, 3.12414f },
    { 1927.61f, 758.436f, 51.4533f, 2.20891f },
    { 1890.64f, 753.471f, 48.7224f, 1.71042f },
    { 1908.31f, 809.657f, 38.7037f, 3.08701f }
};

const Movement::Location DefenseSystemLocation  = { 1888.146f, 803.382f,  58.60389f, 3.071779f };

// General dungeon timers
enum VHTimers : uint32_t
{
    VH_TIMER_UPDATE             = 100,

    VH_TIMER_GUARD_DESPAWN_TIME = 1500,
    VH_TIMER_GUARD_RESPAWN_TIME = 500,
    VH_TIMER_GUARD_FLEE_DELAY   = 5750,

    VH_TIMER_SPAWN_INTRO_MOB    = 15000,
    VH_TIMER_INTRO_PORTAL_DESPAWN_TIME = VH_TIMER_GUARD_FLEE_DELAY,
};

// Spell used by arcane crystal
const uint32_t SPELL_ARCANE_LIGHTNING = 57930;

enum GameObjects
{
    // Sinclari's escort event
    GO_INTRO_ACTIVATION_CRYSTAL = 193615,

    // Instantly kills summoned adds by portals
    GO_ACTIVATION_CRYSTAL       = 193611,

    // Main gates
    GO_PRISON_SEAL              = 191723,

    GO_XEVOZZ_CELL              = 191556,
    GO_LAVANTHOR_CELL           = 191566,
    GO_ICHORON_CELL             = 191722,
    GO_ZURAMAT_CELL             = 191565,
    GO_EREKEM_CELL              = 191564,
    GO_EREKEM_GUARD_CELL1       = 191563,
    GO_EREKEM_GUARD_CELL2       = 191562,
    GO_MORAGG_DOOR              = 191606
};

//enum CreatureSay
//{
//
//};

/// Sinclari event data
// TODO: check values and move this to cpp generally, it won't be used to multiple scripts
enum SinclariTexts
{
    // Gossip items
    GOSSIP_SINCLARI_ACTIVATE    = 600,
    GOSSIP_SINCLARI_GET_SAFETY  = 601,
    GOSSIP_SINCLARI_SEND_ME_IN  = 602
};

const Movement::Location SinclariSpawnLoc = { 1830.95f, 799.463f, 44.418f, 2.3911f };

// Temporally used constants
const char* SINCLARI_YELL = "Prison guards, we are leaving! These adventurers are taking over! Go go go!";
const char* SINCLARI_SAY = "I'm locking the door. Good luck, and thank you for doing this.";
const char* SINCLARI_GO_OPTION1 = "Activate the cystals when we get in trouble, right.";
const char* SINCLARI_GO_OPTION2 = "Get your people to safety, we'll keep the Blue Dragonblight's forces at bay.";
const char* SINCLARI_GO_OPTION3 = "I'm not fighting, so send me in now!";
const char* SINCLARI_SAY_VICTORY = "You did it! You held the Blue Dragonflight back and defeated their commander. Amazing work!";

// Worldstate entries used in instance
enum VHWorldStateIds
{
    WORLD_STATE_VH_SHOW             = 3816,
    WORLD_STATE_VH_PRISON_STATE     = 3815,
    WORLD_STATE_VH_WAVE_COUNT       = 3810,
};

enum VH_Spells
{
    SPELL_VH_PORTAL_PERIODIC            = 58008,
    SPELL_VH_PORTAL_CHANNEL             = 58012,

    SPELL_VH_CRYSTAL_ACTIVATION         = 57804,
    SPELL_VH_DEFENSE_SYSTEM_VISUAL      = 57887,
    SPELL_VH_DEFENSE_SYSTEM_SPAWN       = 57886,
    SPELL_VH_LIGHTNING_INTRO            = 60038,
    SPELL_VH_ARCANE_LIGHTNING_INSTAKILL = 57930,

    SPELL_VH_TELEPORT_PLAYER            = 62138,
    SPELL_VH_TELEPORT_PLAYER_EFFECT     = 62139
};

const Movement::Location fGuardExitLoc = { 1802.099f, 803.7724f, 44.36466f, 0.0f};

// Guards spawn location
const uint8_t guardsCount = 4;
const Movement::Location guardsSpawnLoc[guardsCount] =
{
    { 1854.54f, 802.147f, 44.0078f, 6.22792f },
    { 1854.82f, 805.873f, 44.0081f, 0.054694f },
    { 1853.95f, 810.462f, 44.0088f, 0.239263f },
    { 1854.32f, 798.053f, 44.0087f, 6.0355f }
};

// Intro portal locations
const uint8_t introPortalCount = 3;
const Movement::Location introPortals[introPortalCount] =
{
    { 1878.363770f, 845.572144f, 43.333664f, 4.825092f },   // Left portal
    { 1890.527832f, 758.085510f, 47.666927f, 1.714914f },   // Right portal
    { 1931.233154f, 802.679871f, 52.410446f, 3.112921f }    // Up portal
};

// Intro npcs entries
const uint8_t VHIntroMobCount = 4;
const uint32_t VHIntroMobs[VHIntroMobCount] =
{
    CN_INTRO_AZURE_BINDER_ARCANE,
    CN_INTRO_AZURE_INVADER_ARMS,
    CN_INTRO_AZURE_MAGE_SLAYER_MELEE,
    CN_INTRO_AZURE_SPELLBREAKER_ARCANE
};

// Location used to move intro and event npcs
const Movement::Location sealAttackLoc = {1858.027f, 804.11f, 44.009f, M_PI_FLOAT};
