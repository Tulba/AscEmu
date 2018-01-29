/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Violet hold during whole encounters has total 18 portals summoned
// one portal group is equal to 6 portals, every 6th portal unlocks boss
enum DataIndex : uint8_t
{
    // Main instance progress index
    INDEX_INSTANCE_PROGRESS = 0,

    // MAIN EVENT DATA
    // First two bosses are random
    // There is total 3 bosses, if players wipes on first two bosses, ghost will replace boss
    // These DATA indexes NEVER CONTAINS BOSS STATE!
    // These two data indexes are saved to database
    DATA_GROUP1_BOSS_ENTRY,         // This INDEX value will contain first boss entry
    DATA_GROUP2_BOSS_ENTRY,         // This INDEX value will contain second boss entry

    // Portal data
    // These three data indexes aren't saved to database
    DATA_PERVIOUS_PORTAL_ID,        // This INDEX value will contain perviously used portal id
    DATA_PORTAL_COUNT,              // This INDEX value will increased every time when portal is summoned
    DATA_SEAL_HEALTH,               // This INDEX value will contain health percent of "doors"
    INDEX_PORTAL_PROGRESS,          // This INDEX declares portal progress state
    DATA_ARE_SUMMONS_MADE,          // This INDEX identifies if portal summoned adds successfuly (0 - false, 1 - true)

    // Achievements
    INDEX_ACHIEV_DEFENSELESS,
    INDEX_ACHIEV_VOID_DANCE,

    // BOSS DATA
    INDEX_MORAGG,
    INDEX_ICHORON,
    INDEX_ZURAMAT,
    INDEX_EREKEM,
    INDEX_LAVANTHOR,
    INDEX_XEVOZZ,

    INDEX_MAX
};

enum CreatureEntry : uint32_t
{
    // Portal event
    CN_PORTAL_GUARDIAN                      = 30660,
    CN_PORTAL_KEEPER                        = 30695,
    CN_PORTAL                               = 30679,
    CN_DOOR_SEAL                            = 30896,
    CN_AZURE_SABOTEUR                       = 31079,

    // Defense AI
    CN_DEFENSE_SYSTEM                       = 30837,
    CN_DEFENSE_SYSTEM_TRIGGER               = 30857,    // Used for lightning effect

    // Sinclari event
    CN_LIEUTNANT_SINCLARI                   = 30658,

    // Intro event
    CN_VIOLET_HOLD_GUARD                    = 30659,
    CN_INTRO_AZURE_BINDER_ARCANE            = 31007,
    CN_INTRO_AZURE_INVADER_ARMS             = 31008,
    CN_INTRO_AZURE_SPELLBREAKER_ARCANE      = 31009,
    CN_INTRO_AZURE_MAGE_SLAYER_MELEE        = 31010,
    CN_PORTAL_INTRO                         = 31011,

    // Portal fighters (not keepers/guardians)
    CN_AZURE_INVADER                        = 30661,
    CN_AZURE_SPELLBREAKER                   = 30662,
    CN_AZURE_BINDER                         = 30663,
    CN_AZURE_MAGE_SLAYER                    = 30664,
    CN_AZURE_CAPTAIN                        = 30666,
    CN_AZURE_SORCERER                       = 30667,
    CN_AZURE_RAIDER                         = 30668,
    CN_AZURE_STALKER                        = 32191,
    CN_VETERAN_MAGE_HUNTER                  = 30665,

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
    CN_CYANIGOSA                            = 31134,

    // Ghostly replacements
    CN_ARAKKOA                              = 32226,
    CM_ARAKKOA_GUARD                        = 32228,
    CN_VOID_LORD                            = 32230,
    CN_ETHERAL                              = 32231,
    CN_SWIRLING                             = 32234,
    CN_WATCHER                              = 32235,
    CN_LAVA_HOUND                           = 32237
};

// General dungeon timers
enum VHTimers : uint32_t
{
    // Timers in milliseconds
    VH_UPDATE_TIMER                 = 1000,
    VH_TIMER_SPAWN_INTRO_MOB_MIN    = 15000,
    VH_TELE_PORTAL_WAVE_TIMER       = 15000,
    VH_TELE_PORTAL_BOSS_SPAWN_TIME  = 2000,
    VH_TIMER_SPAWN_INTRO_MOB_MAX    = 20000,

    VH_INITIAL_PORTAL_TIME          = 1000,    // only used for first portal
    VH_NEXT_PORTAL_SPAWN_TIME       = 10000,
    VH_TIMER_AFTER_BOSS             = 60000    // Used to spawn next portal after boss fight (timer needs correction)
};

// Custom state used by instance script
// This state indicates that dungeon event state is failed
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

    // Bosses cells
    GO_XEVOZZ_CELL              = 191556,
    GO_LAVANTHOR_CELL           = 191566,
    GO_ICHORON_CELL             = 191722,
    GO_ZURAMAT_CELL             = 191565,
    GO_EREKEM_CELL              = 191564,
    GO_EREKEM_GUARD_CELL1       = 191563,
    GO_EREKEM_GUARD_CELL2       = 191562,
    GO_MORAGG_CELL              = 191606
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
    SAY_SINCLARI_INSTANCE_DONE  = 8798,     // "You did it! You held the Blue Dragonflight back and defeated their commander. Amazing work!"

    // Xevozz texts
    YELL_XEVOZZ_AGROO           = 4541,     // "It seems my freedom must be bought with blood!"
    YELL_XEVOZZ_TARGET_DEATH1   = 4542,     // "Nothing personal."
    YELL_XEVOZZ_TARGET_DEATH2   = 4543,     // "Business concluded."
    YELL_XEVOZZ_TARGET_DEATH3   = 4544,     // "Profit!"
    YELL_XEVOZZ_DEATH           = 4545,     // "This is an... unrecoverable... loss."
    YELL_XEVOZZ_RELEASE         = 4546,     // "Back in business! Now, to execute an exit strategy."
    YELL_XEVOZZ_CHARGE          = 4547,     // "It would seem that a renegotiation is in order."
    YELL_XEVOZZ_ARCANE_SUMMON1  = 4548,     // "The air teems with latent energy... quite the harvest!"
    YELL_XEVOZZ_ARCANE_SUMMON2  = 4549,     // "Plentiful, exploitable resources... primed for acquisition!"
    YELL_XEVOZZ_ARCANE_SUMMON3  = 4550      // "Intriguing... a high quantity of arcane energy is near. Time for some prospecting..."
};

// Worldstate entries used in instance
enum VH_WorldStateIds : uint32_t
{
    WORLD_STATE_VH_SHOW                 = 3816,
    WORLD_STATE_VH_PRISON_STATE         = 3815,
    WORLD_STATE_VH_PORTAL_COUNT         = 3810,
};

enum VH_Spells : uint32_t
{
    SPELL_VH_PORTAL_PERIODIC            = 58008,
    SPELL_VH_PORTAL_CHANNEL             = 58012,

    SPELL_VH_CRYSTAL_ACTIVATION         = 57804,
    SPELL_VH_DEFENSE_SYSTEM_VISUAL      = 57887,
    SPELL_VH_DEFENSE_SYSTEM_SPAWN       = 57886,
    SPELL_VH_LIGHTNING_INTRO            = 60038,
    SPELL_VH_ARCANE_DAMAGE              = 57912,

    SPELL_VH_TELEPORT_PLAYER            = 62138,
    SPELL_VH_TELEPORT_PLAYER_EFFECT     = 62139,

    SPELL_VH_DESTROY_DOOR_SEAL          = 58040,

    SPELL_VH_SHIELD_DISRUPTION          = 58291,
    SPELL_VH_SIMPLE_TELEPORT            = 12980,
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
    CN_EREKEM
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

const uint8_t MaxSinclariWps = 5;
const Movement::Location SinclariWps[MaxSinclariWps] =
{
    { 1829.142f, 798.219f,  44.36212f, 0.122173f }, // 0 - Crystal
    { 1820.12f,  803.916f,  44.36466f, 0.0f      }, // 1 - Outside (before closing doors)
    { 1816.185f, 804.0629f, 44.44799f, 3.176499f }, // 2 - Second Spawn Point (after closing doors)
    { 1827.886f, 804.0555f, 44.36467f, 0.0f      },  // 3 - Outro (instance finish event)
    {}
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

const Movement::Location SaboteurPortalLoc = { 1890.73f, 803.309f, 38.4001f, 2.4139f };  // center

// Saboteur waypoints
const uint8_t MaxWpToMoragg = 6;
const Movement::Location SaboteurMoraggPath[MaxWpToMoragg] =
{
    { 1886.251f, 803.0743f, 38.42326f, 0 }, // 0
    { 1885.71f,  799.8929f, 38.37241f, 0 }, // 1
    { 1889.505f, 762.3288f, 47.66684f, 0 }, // 2
    { 1894.542f, 742.1829f, 47.66684f, 0 }, // 3
    { 1894.603f, 739.9231f, 47.66684f, 0 }, // 4
    {}                                      // 5
};

const uint8_t MaxWpToErekem = 6;
const Movement::Location SaboteurErekemPath[MaxWpToErekem] =
{
    { 1886.251f, 803.0743f, 38.42326f, 0 },
    { 1881.047f, 829.6866f, 38.64856f, 0 },
    { 1877.585f, 844.6685f, 38.49014f, 0 },
    { 1876.085f, 851.6685f, 42.99014f, 0 },
    { 1873.747f, 864.1373f, 43.33349f, 0 },
    {}
};

const uint8_t MaxWpToIchonor = 4;
const Movement::Location SaboteurIchoronPath[MaxWpToIchonor] =
{
    { 1886.251f, 803.0743f, 38.42326f, 0 },
    { 1888.672f, 801.2348f, 38.42305f, 0 },
    { 1901.987f, 793.3254f, 38.65126f, 0 },
    {}
};

const uint8_t MaxWpToLavanthor = 4;
const Movement::Location SaboteurLavanthorPath[MaxWpToLavanthor] =
{
    { 1886.251f, 803.0743f, 38.42326f, 0 },
    { 1867.925f, 778.8035f, 38.64702f, 0 },
    { 1853.304f, 759.0161f, 38.65761f, 0 },
    {}
};

const uint8_t MaxWpToXevozz = 4;
const Movement::Location SaboteurXevozzPath[MaxWpToXevozz] =
{
    { 1889.096f, 810.0487f, 38.43871f, 0 },
    { 1896.547f, 823.5473f, 38.72863f, 0 },
    { 1906.666f, 842.3111f, 38.63351f, 0 },
    {}
};

const uint8_t MaxWpToZuramat = 8;
const Movement::Location SaboteurZuramatPath[MaxWpToZuramat] =
{
    { 1886.251f, 803.0743f, 38.42326f, 0 },
    { 1889.69f,  807.0032f, 38.39914f, 0 },
    { 1906.91f,  818.2574f, 38.86596f, 0 },
    { 1929.03f,  824.2713f, 46.09165f, 0 },
    { 1928.441f, 842.8891f, 47.15078f, 0 },
    { 1927.454f, 851.6091f, 47.19094f, 0 },
    { 1927.947f, 852.2986f, 47.19637f, 0 },
    {}
};

struct VHPortalInfo
{
    uint8_t id;                                     // id should be equal to PortalPositions array index
    VH_PORTAL_TYPE type;                            // see VH_PORTAL_TYPE enum
    uint32_t guardianEntry;                         // contains gurdian/keeper
    uint32_t bossEntry;                             // only used for VH_PORTAL_TYPE_BOSS
    std::list<uint32_t> summonsList;                // contains summon lists - used for squads
    VHPortalInfo()
    {
        ResetData();
    }

    void RemoveSummonByGuid(uint32_t guid)
    {
        if (summonsList.empty())
            return;

        for (std::list<uint32>::iterator itr = summonsList.begin(); itr != summonsList.end(); ++itr)
        {
            if ((*itr) == guid)
            {
                summonsList.erase(itr);
                break;
            }
        }
    }

    void ResetData()
    {
        id = 0;
        type = VH_PORTAL_TYPE_NONE;
        guardianEntry = 0;
        bossEntry = 0;
    }
};

const uint8 maxPortalGuardians = 9;
const uint32_t portalGuardians[maxPortalGuardians] =
{
    CN_AZURE_INVADER,
    CN_AZURE_SPELLBREAKER,
    CN_AZURE_BINDER,
    CN_AZURE_MAGE_SLAYER,
    CN_AZURE_CAPTAIN,
    CN_AZURE_SORCERER,
    CN_AZURE_RAIDER,
    CN_AZURE_STALKER,
    CN_VETERAN_MAGE_HUNTER
};

// Keepers/guardians/squad groups announce
const char* GUARDIAN_ANNOUNCE   = "A Portal Gurdian defends the new portal!";
const char* KEEPER_ANNOUNCE     = "A Portal Keeper emerges from the portal!";
const char* SQUAD_ANNOUNCE      = "An elite Blue Dragonflight squad appears from the portal!";

// Sinclari's warning
const char* SEAL_PCT_75     = "Adventurers, the door is beinning to weaken!";
const char* SEAL_PCT_50     = "Only half of the door seal's strength remains! You must fight on!";
const char* SEAL_PCT_5      = "The door seal is about to collapse! All is lost if the Blue Dragonflight breaks through the door!";

// Boss replacements
struct VHBossReplaceStruct
{
    uint32_t bossEntry;
    uint32_t ghostEntry;
};

const uint8_t MaxBossReplacements = 6;
const VHBossReplaceStruct BossReplacements [MaxBossReplacements] =
{
    { CN_EREKEM,    CN_ARAKKOA    },
    { CN_ZURAMAT,   CN_VOID_LORD  },
    { CN_XEVOZZ,    CN_ETHERAL    },
    { CN_ICHORON,   CN_SWIRLING   },
    { CN_LAVANTHOR, CN_LAVA_HOUND },
    { CN_MORAGG,    CN_WATCHER    }
};

// Boss intro waypoints
const uint8_t MoraggPathSize = 4;
const Movement::Location MoraggPath[MoraggPathSize] =
{
    { 1893.895f, 728.1261f, 47.75016f, 0 },
    { 1892.997f, 738.4987f, 47.66684f, 0 },
    { 1889.76f,  758.1089f, 47.66684f, 0 },
    {}
};

const uint8_t ErekemPathSize = 4;
const Movement::Location ErekemPath[ErekemPathSize] =
{
    { 1871.456f, 871.0361f, 43.41524f, 0 },
    { 1874.948f, 859.5452f, 43.33349f, 0 },
    { 1877.245f, 851.967f,  43.3335f,  0 },
    {}
};

const uint8_t ErekemGuardLeftPathSize = 4;
const Movement::Location ErekemGuardLeftPath[ErekemGuardLeftPathSize] =
{
    { 1853.752f, 862.4528f, 43.41614f, 0 },
    { 1866.931f, 854.577f,  43.3335f,  0 },
    { 1872.973f, 850.7875f, 43.3335f,  0 },
    {}
};

const uint8_t ErekemGuardRightPathSize = 4;
const Movement::Location ErekemGuardRightPath[ErekemGuardRightPathSize] =
{
    { 1892.418f, 872.2831f, 43.41563f, 0 },
    { 1885.639f, 859.0245f, 43.3335f, 0 },
    { 1882.432f, 852.2423f, 43.3335f, 0 },
    {}
};

const uint8_t IchoronPathSize = 6;
const Movement::Location IchoronPath[IchoronPathSize] =
{
    { 1942.041f, 749.5228f, 30.95229f, 0 },
    { 1930.571f, 762.9065f, 31.98814f, 0 },
    { 1923.657f, 770.6718f, 34.07256f, 0 },
    { 1910.631f, 784.4096f, 37.09015f, 0 },
    { 1906.595f, 788.3828f, 37.99429f, 0 },
    {}
};

const uint8_t LavanthorPathSize = 4;
const Movement::Location LavanthorPath[LavanthorPathSize] =
{
    { 1844.557f, 748.7083f, 38.74205f, 0 },
    { 1854.618f, 761.5295f, 38.65631f, 0 },
    { 1862.17f,  773.2255f, 38.74879f, 0 },
    {}
};

const uint8_t XevozzPathSize = 4;
const Movement::Location XevozzPath[XevozzPathSize] =
{
    { 1908.417f, 845.8502f, 38.71947f, 0 },
    { 1905.557f, 841.3157f, 38.65529f, 0 },
    { 1899.453f, 832.533f,  38.70752f, 0 },
    {}
};

const uint8_t ZuramatPathSize = 4;
const Movement::Location ZuramatPath[ZuramatPathSize] =
{
    { 1934.151f, 860.9463f, 47.29499f, 0 },
    { 1927.085f, 852.1342f, 47.19214f, 0 },
    { 1923.226f, 847.3297f, 47.15541f, 0 },
    {}
};

const Movement::Location CyanigosaSpawnLocation = { 1922.109f, 804.4493f, 52.49254f, 3.176499f };
const Movement::Location CyanigosaJumpLocation  = { 1888.32f,  804.473f,  38.3578f,  0.0f      };

// Waypoints used by portal summons
// Portal near Erekem boss waypoints
const uint8_t MaxFirstPortalWPS = 7;
const Movement::Location FirstPortalWPs[MaxFirstPortalWPS] =
{
    {1877.670288f, 842.280273f, 43.333591f, 0},
    {1877.338867f, 834.615356f, 38.762287f, 0},
    {1872.161011f, 823.854309f, 38.645401f, 0},
    {1864.860474f, 815.787170f, 38.784843f, 0},
    {1858.953735f, 810.048950f, 44.008759f, 0},
    {1843.707153f, 805.807739f, 44.135197f, 0},
    //{1825.736084f, 807.305847f, 44.363785f},
    {}
};

// Portal near Zuramat boss waypoints (left side)
const uint8_t MaxSecondPortalLeftWPS = 10;
const Movement::Location SecondPortalFirstWPs[MaxSecondPortalLeftWPS] =
{
    {1902.561401f, 853.334656f, 47.106117f, 0},
    {1895.486084f, 855.376404f, 44.334591f, 0},
    {1882.805176f, 854.993286f, 43.333591f, 0},
    {1877.670288f, 842.280273f, 43.333591f, 0},
    {1877.338867f, 834.615356f, 38.762287f, 0},
    {1872.161011f, 823.854309f, 38.645401f, 0},
    {1864.860474f, 815.787170f, 38.784843f, 0},
    {1858.953735f, 810.048950f, 44.008759f, 0},
    {1843.707153f, 805.807739f, 44.135197f, 0},
    //{1825.736084f, 807.305847f, 44.363785f},
    {}
};

// Portal near Zuramat boss waypoints (right side)
const uint8_t MaxSecondPortalRightWPS = 9;
const Movement::Location SecondPortalSecondWPs[MaxSecondPortalRightWPS] =
{
    {1929.392212f, 837.614990f, 47.136166f, 0},
    {1928.290649f, 824.750427f, 45.474411f, 0},
    {1915.544922f, 826.919373f, 38.642811f, 0},
    {1900.933960f, 818.855652f, 38.801647f, 0},
    {1886.810547f, 813.536621f, 38.490490f, 0},
    {1869.079712f, 808.701538f, 38.689003f, 0},
    {1860.843384f, 806.645020f, 44.008789f, 0},
    {1843.707153f, 805.807739f, 44.135197f, 0},
    //{1825.736084f, 807.305847f, 44.363785f},
    {}
};

// Top edge portal waypoints
const uint8_t MaxThirdPortalWPS = 9;
const Movement::Location ThirdPortalWPs[MaxThirdPortalWPS] =
{
    {1934.049438f, 815.778503f, 52.408699f, 0},
    {1928.290649f, 824.750427f, 45.474411f, 0},
    {1915.544922f, 826.919373f, 38.642811f, 0},
    {1900.933960f, 818.855652f, 38.801647f, 0},
    {1886.810547f, 813.536621f, 38.490490f, 0},
    {1869.079712f, 808.701538f, 38.689003f, 0},
    {1860.843384f, 806.645020f, 44.008789f, 0},
    {1843.707153f, 805.807739f, 44.135197f, 0},
    //{1825.736084f, 807.305847f, 44.363785f},
    {}
};

// Squad group spawn location near Moragg boss
const uint8_t MaxFourthPortalWPS = 10;
const Movement::Location FourthPortalWPs[MaxFourthPortalWPS] =
{
    {1921.658447f, 761.657043f, 50.866741f, 0},
    {1910.559814f, 755.780457f, 47.701447f, 0},
    {1896.664673f, 752.920898f, 47.667004f, 0},
    {1887.398804f, 763.633240f, 47.666851f, 0},
    {1879.020386f, 775.396973f, 38.705990f, 0},
    {1872.439087f, 782.568604f, 38.808292f, 0},
    {1863.573364f, 791.173584f, 38.743660f, 0},
    {1857.811890f, 796.765564f, 43.950329f, 0},
    {1845.577759f, 800.681152f, 44.104248f, 0},
    //{1827.100342f, 801.605957f, 44.363358f},
    {}
};

// Portal near Moragg boss
const uint8_t MaxFifthPortalWPS = 7;
const Movement::Location FifthPortalWPs[MaxFifthPortalWPS] =
{
    {1887.398804f, 763.633240f, 47.666851f, 0},
    {1879.020386f, 775.396973f, 38.705990f, 0},
    {1872.439087f, 782.568604f, 38.808292f, 0},
    {1863.573364f, 791.173584f, 38.743660f, 0},
    {1857.811890f, 796.765564f, 43.950329f, 0},
    {1845.577759f, 800.681152f, 44.104248f, 0},
    //{1827.100342f, 801.605957f, 44.363358f},
    {}
};

// Portal near Ichonor boss waypoints
const uint8_t MaxSixhtPortalWPS = 5;
const Movement::Location SixthPoralWPs[MaxSixhtPortalWPS] =
{
    {1888.861084f, 805.074768f, 38.375790f, 0},
    {1869.793823f, 804.135804f, 38.647018f, 0},
    {1861.541504f, 804.149780f, 43.968292f, 0},
    {1843.567017f, 804.288208f, 44.139091f, 0},
    //{1826.889648f, 803.929993f, 44.363239f},
    {}
};

// Default location used for attackers
const Movement::Location DefaultPortalWPs = { 1843.567017f, 804.288208f, 44.139091f, 0 };

// Erekem guards location
const Movement::Location leftErekemGuardPosition = { 1853.75f, 862.453f, 43.4161, 0 };
const Movement::Location rightErekemGuardPosition = { 1892.42f, 872.283f, 43.4156f, 0 };

// Common instance events tasks are handled by SetInstanceData function
class TheVioletHoldInstance : public InstanceScript
{
    uint32_t m_VHencounterData[INDEX_MAX];

    // Low guids of creatures
    uint32_t m_sinclariGUID;
    uint32_t m_ErekemGUID;
    uint32_t m_MoraggGUID;
    uint32_t m_IchoronGUID;
    uint32_t m_XevozzGUID;
    uint32_t m_LavanthorGUID;
    uint32_t m_ZuramatGUID;

    // Guid lists
    std::vector<uint32_t> m_guardsGuids;      // Guards at entrance guids
    std::vector<uint32_t> m_crystalGuids;     // Activation crystal guids
    std::vector<uint32_t> m_introSpawns;      // Intro creatures guids
    std::vector<uint32_t> m_defenseTriggers;  // Used for visual effect in defense npc AI
    std::vector<uint32_t> m_eventSpawns;      // Portal event spawns (attackers)

    // Portal summoning event
    uint32_t m_portalSummonTimer;
    VHPortalInfo m_activePortal;
    uint32_t m_portalGUID;
    uint32_t m_portalGuardianGUID;

    // Booleans used to control sinclari reports about seal hp
    bool emote75pct;
    bool emote50pct;
    bool emote5pct;

    // Friend classes which will able to use private instance data
    friend class VH_DefenseAI;
    friend class TeleportationPortalAI;
    friend class AzureSaboteurAI;

public:

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheVioletHoldInstance(pMapMgr); }
    TheVioletHoldInstance(MapMgr* pMapMgr);

    // Sets instance data - used for events handling
    void SetInstanceData(uint32_t pIndex, uint32_t pData);

    // Gets instance data - used for events handling
    uint32_t GetInstanceData(uint32_t pIndex);

    // Removes all dead intro npcs
    void RemoveIntroNpcs(bool deadOnly);

    // Removed intro npcs by low guid
    void RemoveIntroNpcByGuid(uint32_t guid);

    // Call guards out
    void CallGuardsOut();

private:
    // Sets base instance data
    void ResetInstanceData();

    // Generate very basic portal info
    void GeneratePortalInfo(VHPortalInfo & newPortal);

    // SpawnPortal
    void SpawnPortal();

    // Activates crystal event
    void DoCrystalActivation();

    // Resets whole dungeon and starts intro npcs summoning
    void ResetIntro();

    // Updates world states based on instance data
    void UpdateWorldStates();

    // Send worldstates packet to players
    void UpdateInstanceWorldState(uint32_t field, uint32_t value);

    // Spawn instance intro
    void SpawnIntro();

    // Resets activation crystals
    void ResetCrystals(bool isSelectable);

    // Update achievement criteria for all players by id
    void UpdateAchievCriteriaForPlayers(uint32_t id, uint32_t criteriaCount);

    // Returns ghostly replacement entry for boss
    uint32_t GetGhostlyReplacement(uint32_t bossEntry);

    // Returns boss entry by ghost entry
    uint32_t GetBossEntryByGhost(uint32_t ghostEntry);

    // Spawns ghostly replacement for boss entry
    void SpawnGhostlyReplacement(uint32_t bossEntry, float x, float y, float z, float o);

    // Virtual event functions
    void OnCreaturePushToWorld(Creature* pCreature) override;
    void OnCreatureDeath(Creature* pCreature, Unit* pKiller) override;
    void OnPlayerEnter(Player* plr) override;
    void UpdateEvent() override;
    void OnLoad() override;
    void OnGameObjectPushToWorld(GameObject* pGo) override;
    void OnGameObjectActivate(GameObject* pGo, Player* plr) override;
};