/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Violet hold during whole encounters has total 18 portals summoned
// one portal group is equal to 6 portals, every 6th portal unlocks boss
enum DataIndex : uint8_t
{
    INDEX_INSTANCE_PROGRESS = 0,

    // First two bosses are random
    DATA_GROUP1_BOSS_ENTRY,         // This INDEX value will contain first boss entry
    INDEX_GROUP1_BOSS_PROGRESS,     // First group boss progress data
    DATA_GROUP2_BOSS_ENTRY,         // This INDEX value will contain second boss entry
    INDEX_GROUP2_BOSS_PROGRESS,     // Second group boss progress data
    INDEX_CYANIGOSA,                // Last boss index

    // Portal data
    DATA_CURRENTLY_USED_PORTAL_ID,  // This INDEX value will contain randomly generated portal id
    DATA_PERVIOUS_PORTAL_ID,        // This INDEX value will contain perviously used portal id
    DATA_PORTAL_COUNT,              // This INDEX value will increased every time when portal is summoned
    DATA_SEAL_HEALTH,               // This INDEX value will contain health percent of "doors"
    INDEX_PORTAL_PROGRESS,          // This index declares portal progress state

    INDEX_MAX
};

enum CreatureEntry : uint32_t
{
    // Portal event
    CN_PORTAL_GUARDIAN                      = 30660,
    CN_PORTAL_KEEPER                        = 30695,

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
    CN_AZURE_SORCERER                       = 30667,
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

// General dungeon timers
enum VHTimers : uint32_t
{
    // Timers in milliseconds
    VH_UPDATE_TIMER                 = 1000,
    VH_TIMER_SPAWN_INTRO_MOB_MIN    = 15000,
    VH_TIMER_SPAWN_INTRO_MOB_MAX    = 20000,

    // Every timer below contains strict values in seconds (not milliseconds)
    VH_INITIAL_PORTAL_TIME          = 5,    // only used for first portal
    VH_NEXT_PORTAL_SPAWN_TIME       = 10
};

// Our custom state used by instance script
const uint32_t State_Failed = 5;

enum VH_achievements : uint32_t
{
    // event achiev - 1816
    ACHIEV_CRIT_DEFENSELES      = 6803,

    // Ichoron achiev - 2041
    ACHIEV_CRIT_DEHYDRATATION   = 7320,

    // Zuramat achiev - 2153
    ACHIEV_CRIT_VOID_DANCE      = 7587
};

enum VH_GameObjects : uint32_t
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

enum VH_Texts : uint32_t
{
    // Sinclari's gossips
    GOSSIP_SINCLARI_ACTIVATE    = 600,      // "Activate the cystals when we get in trouble, right."
    GOSSIP_SINCLARI_GET_SAFETY  = 601,      // "Get your people to safety, we'll keep the Blue Dragonblight's forces at bay."
    GOSSIP_SINCLARI_SEND_ME_IN  = 602,      // "I'm not fighting, so send me in now!"

    // Sinclari's texts
    YELL_SINCLARI_LEAVING       = 4522,     // "Prison guards, we are leaving! These adventurers are taking over! Go go go!"
    SAY_SINCLARI_CLOSING_GATES  = 8797,     // "I'm locking the door. Good luck, and thank you for doing this."
    SAY_SINCLARI_INSTANCE_DONE  = 8798      // "You did it! You held the Blue Dragonflight back and defeated their commander. Amazing work!"
};

// Worldstate entries used in instance
enum VH_WorldStateIds
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

enum VH_PORTAL_TYPE : uint8_t
{
    VH_PORTAL_TYPE_NONE         = 0,
    VH_PORTAL_TYPE_GUARDIAN,
    VH_PORTAL_TYPE_SQUAD,
    VH_PORTAL_TYPE_BOSS,
};

// Location used to move guards and liutenant
const Movement::Location introMoveLoc = { 1817.377930f, 803.973633f, 44.363861f, 0 };

const uint8_t maxVHBosses = 5;
const uint32_t randomVHBossArray[maxVHBosses] =
{
    CN_MORAGG,
    CN_ICHORON,
    CN_XEVOZZ,
    CN_LAVANTHOR,
    CN_CYANIGOSA
};

const Movement::Location DefenseSystemLocation  = { 1888.146f, 803.382f,  58.60389f, 3.071779f };

const Movement::Location SinclariSpawnLoc = { 1830.95f, 799.463f, 44.418f, 2.3911f };

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
const Movement::Location sealAttackLoc = { 1843.567017f, 804.288208f, 44.139091f, 0};

const Movement::Location SinclariPositions[] =
{
    { 1829.142f, 798.219f,  44.36212f, 0.122173f }, // 0 - Crystal
    { 1820.12f,  803.916f,  44.36466f, 0.0f      }, // 1 - Outside (before closing doors)
    { 1816.185f, 804.0629f, 44.44799f, 3.176499f }, // 2 - Second Spawn Point (after closing doors)
    { 1827.886f, 804.0555f, 44.36467f, 0.0f      }  // 3 - Outro (instance finish event)
};

const uint32_t MaxPortalPositions = 8;
const Movement::Location PortalPositions[MaxPortalPositions] =
{
    // Portal with guardians
    { 1877.523f, 850.1788f, 45.36822f, 4.34587f   }, // 0, left side, near Erekem boss
    { 1890.679f, 753.4202f, 48.771f,   1.675516f  }, // 1, right side, near Moragg boss
    { 1936.09f,  803.1875f, 54.09715f, 3.054326f  }, // 2, top edge
    { 1858.243f, 770.2379f, 40.42146f, 0.9075712f }, // 3, right side, near Lavanthor boss
    { 1907.288f, 831.1111f, 40.22015f, 3.560472f  }, // 4, left side, near Xevozz boss

    // Portal with elites
    { 1911.281f, 800.9722f, 39.91673f, 3.01942f  }, // 5, right side, near Ichonor boss
    { 1926.516f, 763.6616f, 52.35725f, 2.251475f }, // 6, right side, near Moragg boss
    { 1922.464f, 847.0699f, 48.50161f, 3.961897f }  // 7, left side, near Zuramat boss
};

struct VHPortalInfo
{
    uint8_t id;                                     // id should be equal to PortalPositions array index
    uint32_t portalLowGuid;                         // contains portal guid (for spell chanelling/despawning events)
    VH_PORTAL_TYPE type;                            // see VH_PORTAL_TYPE enum
    uint32_t guardianEntry;                         // contains gurdian/keeper
    uint32_t bossEntry;                             // only used for VH_PORTAL_TYPE_BOSS
    std::list<uint32_t> summonsList;                // contains summon lists - used for squads
    VHPortalInfo() : id(0), type(VH_PORTAL_TYPE_NONE), guardianEntry(0), bossEntry(0)
    {
    }
};

const uint8 maxPortalGuardians = 8;
const uint32_t portalGuardians[maxPortalGuardians] =
{
    CN_AZURE_INVADER,
    CN_AZURE_SPELLBREAKER,
    CN_AZURE_BINDER,
    CN_AZURE_MAGE_SLAYER,
    CN_AZURE_CAPTAIN,
    CN_AZURE_SORCEROR,
    CN_AZURE_RAIDER,
    CN_AZURE_STALKER
};
