/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheVioletHold.h"
#include "Spell/SpellAuras.h"

class VH_DefenseAI;
class TeleportationPortalAI;

class TheVioletHoldScript : public InstanceScript
{
    uint32_t m_VHencounterData[INDEX_MAX];

    // Achievements
    bool m_isZuramatAchievFailed;
    bool m_isDefAchievFailed;

    // Low guids of gameobjects
    uint32_t mainGatesGUID;

    // Low guids of creatures
    uint32_t m_sinclariGUID;

    // Guid lists
    std::list<uint32_t> m_guardsGuids;      // Guards at entrance guids
    std::list<uint32_t> m_crystalGuids;     // Activation crystal guids
    std::list<uint32_t> m_introSpawns;      // intro creatures guids
    std::list<uint32_t> m_defenseTriggers;  // Used for visual effect in defense npc AI
    std::list<uint32_t> m_eventSpawns;      // Portal event spawns (it won't contain main portal guardians)

    // Portal summoning event
    uint32_t portalSummonTimer;
    VHPortalInfo m_activePortal;
    uint32_t portalGUID;

    // Friend classes which will able to use private instance data
    friend class VH_DefenseAI;
    friend class TeleportationPortalAI;

    public:

        static InstanceScript* Create(MapMgr* pMapMgr) { return new TheVioletHoldScript(pMapMgr); }
        TheVioletHoldScript(MapMgr* pMapMgr) :
            InstanceScript(pMapMgr),
            m_isZuramatAchievFailed(false),
            m_isDefAchievFailed(false),
            mainGatesGUID(0),
            m_sinclariGUID(0),
            portalSummonTimer(0),
            portalGUID(0)
        {
            //TODO: this should be redone by checking actual saved data for heroic mode
            memset(m_VHencounterData, State_NotStarted, sizeof(m_VHencounterData));
            pMapMgr->pInstance = sInstanceMgr.GetInstanceByIds(MAP_VIOLET_HOLD, pMapMgr->GetInstanceID());
            generateBossDataState();
        }

        void SetInstanceData(uint32_t /*pType*/, uint32_t pIndex, uint32_t pData)
        {
            if (pIndex >= INDEX_MAX)
            {
                return;
            }

            m_VHencounterData[pIndex] = pData;

            switch (pIndex)
            {
                case INDEX_INSTANCE_PROGRESS:
                {
                    if (pData == State_Performed)
                    {
                        DoCrystalActivation();
                    }

                    if (pData == State_PreProgress)
                    {
                        RemoveIntroNpcs(false);
                        // Close the gates
                        if (GameObject* pGO = GetInstance()->GetGameObject(mainGatesGUID))
                        {
                            pGO->SetState(GO_STATE_CLOSED);
                        }

                        // Update crystals
                        for (std::list<uint32_t>::iterator itr = m_crystalGuids.begin(); itr != m_crystalGuids.end(); ++itr)
                        {
                            if (GameObject* pGO = GetInstance()->GetGameObject(*itr))
                            {
                                pGO->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                            }
                        }
                    }

                    if (pData == State_InProgress)
                    {
                        SetInstanceData(0, DATA_SEAL_HEALTH, 100);
                        portalSummonTimer = VH_INITIAL_PORTAL_TIME;
                    }

                    if (pData == State_Failed)
                    {
                        ResetIntro();
                        ResetCrystals(false);
                        SetInstanceData(0, INDEX_INSTANCE_PROGRESS, State_NotStarted);
                    }

                    if (pData == State_Finished)
                    {
                        // Open gates
                        GameObject* pGates = GetInstance()->GetGameObject(mainGatesGUID);
                        if (pGates && pGates->GetState() == GO_STATE_CLOSED)
                        {
                            pGates->SetState(GO_STATE_OPEN);
                        }

                        // Start her outro event
                        Creature* pSinclari = GetInstance()->GetCreature(m_sinclariGUID);
                        if (pSinclari && pSinclari->GetScript())
                        {
                            pSinclari->GetScript()->RegisterAIUpdateEvent(1000);
                        }
                    }
                }break;
                case INDEX_PORTAL_PROGRESS:
                {
                    if (pData == State_Finished)
                    {
                        if (Creature* pPortal = GetInstance()->GetCreature(portalGUID))
                        {
                            pPortal->Despawn(1000, 0);
                        }
                        portalGUID = 0;

                        // Lets reset event
                        SetInstanceData(0, INDEX_PORTAL_PROGRESS, State_NotStarted);
                        SetInstanceData(0, DATA_ARE_SUMMONS_MADE, 0);
                        m_activePortal.ResetData();
                    }
                }break;
                default:
                    break;
            }
        }

        uint32_t GetInstanceData(uint32_t /*pType*/, uint32_t pIndex)
        {
            if (pIndex >= INDEX_MAX)
            {
                return 0;
            }

            return m_VHencounterData[pIndex];
        }

        void OnLoad()
        {
            // For most creatures movements mmaps is needed
            if (!sWorld.settings.terrainCollision.isPathfindingEnabled)
            {
                LOG_ERROR("Violet Hold: dungeon requires pathfinding support.");
            }

            // Spawn intro
            if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_NotStarted)
            {
                ResetIntro();
            }

            RegisterUpdateEvent(VH_UPDATE_TIMER);
        }

        void OnGameObjectPushToWorld(GameObject* pGo)
        {
            switch(pGo->GetEntry())
            {
                case GO_PRISON_SEAL:
                {
                    mainGatesGUID = pGo->GetLowGUID();
                    if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress && pGo->GetState() == GO_STATE_OPEN)
                    {
                        pGo->SetState(GO_STATE_CLOSED);
                    }
                }break;
                case GO_ACTIVATION_CRYSTAL:
                {
                    m_crystalGuids.push_back(pGo->GetLowGUID());
                }break;
                default:
                    break;
            }
        }

        void OnGameObjectActivate(GameObject* pGo, Player* plr)
        {
            if (pGo->GetEntry() == GO_ACTIVATION_CRYSTAL)
            {
                // Mark achiev as failed
                if (!m_isDefAchievFailed)
                {
                    m_isDefAchievFailed = true;
                }

                // Make object not selectable
                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                pGo->SetState(GO_STATE_OPEN);

                // Spell actually sends script event 20001 but theres no handling for such effects
                // plr->CastSpell(plr, SPELL_VH_CRYSTAL_ACTIVATION, true);

                DoCrystalActivation();
            }
        }

        void OnCreaturePushToWorld(Creature* pCreature)
        {
            // Make sure all spawned npcs are in phase 1
            pCreature->Phase(PHASE_SET, 1);

            switch(pCreature->GetEntry())
            {
                case CN_DOOR_SEAL:
                {
                    // HACKY INVISIBLE
                    // invisible display id
                    // this is required to make visual effect working perfectly
                    if (pCreature->GetDisplayId() != 11686)
                        pCreature->SetDisplayId(11686);
                }break;
                case CN_VIOLET_HOLD_GUARD:
                {
                    m_guardsGuids.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                    pCreature->setByteFlag(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
                    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLUS_MOB | UNIT_FLAG_UNKNOWN_16);
                    pCreature->setMoveDisableGravity(true);
                    pCreature->GetAIInterface()->setSplineRun();
                }break;
                case CN_LIEUTNANT_SINCLARI:
                {
                    m_sinclariGUID = GET_LOWGUID_PART(pCreature->GetGUID());
                    pCreature->setByteFlag(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
                    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLUS_MOB | UNIT_FLAG_UNKNOWN_16);
                }break;
                case CN_PORTAL_INTRO:
                {
                     m_introSpawns.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                     pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN_16);
                }break;
                case CN_INTRO_AZURE_BINDER_ARCANE:
                case CN_INTRO_AZURE_INVADER_ARMS:
                case CN_INTRO_AZURE_MAGE_SLAYER_MELEE:
                case CN_INTRO_AZURE_SPELLBREAKER_ARCANE:
                {
                    m_introSpawns.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                    pCreature->setByteFlag(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
                    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN_16);
                }break;
                case CN_DEFENSE_SYSTEM_TRIGGER:
                {
                    m_defenseTriggers.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                    pCreature->Phase(PHASE_SET, 1);
                }break;
                case CN_PORTAL:
                {
                    portalGUID = GET_LOWGUID_PART(pCreature->GetGUID());
                }break;
                // Main portal event related
                case CN_AZURE_INVADER:
                case CN_AZURE_SPELLBREAKER:
                case CN_AZURE_BINDER:
                case CN_AZURE_MAGE_SLAYER:
                case CN_AZURE_CAPTAIN:
                case CN_AZURE_SORCERER:
                case CN_AZURE_RAIDER:
                case CN_AZURE_STALKER:
                {
                    m_eventSpawns.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                }break;
                default:
                    break;
            }
        }

        void SaveInstanceData(uint32_t spawnId)
        {
            if (GetInstance()->iInstanceMode != MODE_HEROIC)
                return;

            if (GetInstance()->pInstance->m_killedNpcs.find(spawnId) == GetInstance()->pInstance->m_killedNpcs.end())
            {
                GetSavedInstance()->m_killedNpcs.insert(spawnId);
                GetSavedInstance()->SaveToDB();
            }
        }

        Instance* GetSavedInstance()
        {
            return GetInstance()->pInstance;
        }

        void OnCreatureDeath(Creature* pCreature, Unit* /*pKiller*/)
        {
            switch (pCreature->GetEntry())
            {
                case CN_ZURAMAT:
                {
                    if (!m_isZuramatAchievFailed)
                    {
                        UpdateAchievCriteriaForPlayers(ACHIEV_CRIT_VOID_DANCE, 1);
                    }
                }break;
                case CN_VOID_SENTRY:
                {
                    m_isZuramatAchievFailed = true;
                }break;
                case CN_PORTAL_INTRO:
                case CN_INTRO_AZURE_BINDER_ARCANE:
                case CN_INTRO_AZURE_INVADER_ARMS:
                case CN_INTRO_AZURE_MAGE_SLAYER_MELEE:
                case CN_INTRO_AZURE_SPELLBREAKER_ARCANE:
                {
                    pCreature->Despawn(4000, 0);
                }break;
                case CN_CYANIGOSA:
                {
                    if (!m_isDefAchievFailed)
                    {
                        UpdateAchievCriteriaForPlayers(ACHIEV_CRIT_DEFENSELES, 1);
                    }
                    setData(pCreature->GetEntry(), Finished);
                    SaveInstanceData(pCreature->GetSQL_id());
                }break;
                case CN_VIOLET_HOLD_GUARD:
                {
                    pCreature->Despawn(1000, 1000);
                }break;
                case CN_MORAGG:
                case CN_ICHORON:
                case CN_XEVOZZ:
                case CN_LAVANTHOR:
                {
                    setData(pCreature->GetEntry(), Finished);
                    SaveInstanceData(pCreature->GetSQL_id());
                }break;
                // Main portal event related
                case CN_AZURE_INVADER:
                case CN_AZURE_SPELLBREAKER:
                case CN_AZURE_BINDER:
                case CN_AZURE_MAGE_SLAYER:
                case CN_AZURE_CAPTAIN:
                case CN_AZURE_SORCERER:
                case CN_AZURE_RAIDER:
                case CN_AZURE_STALKER:
                {
                    if (m_activePortal.type == VH_PORTAL_TYPE_SQUAD && GetInstanceData(0, INDEX_PORTAL_PROGRESS) == State_InProgress)
                    {
                        m_activePortal.DelSummonDataByGuid(pCreature->GetGUID());
                        if (m_activePortal.summonsList.empty())
                        {
                            SetInstanceData(0, INDEX_PORTAL_PROGRESS, State_Finished);
                        }
                    }
                }break;
                case CN_PORTAL_GUARDIAN:
                case CN_PORTAL_KEEPER:
                {
                    if (m_activePortal.type == VH_PORTAL_TYPE_GUARDIAN)
                    {
                        SetInstanceData(0, INDEX_PORTAL_PROGRESS, State_Finished);
                    }
                }break;
                default:
                    break;
            }
        }

        void OnPlayerEnter(Player* plr)
        {
            UpdateWorldStates();
        }

        void UpdateEvent()
        {
            if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) != State_InProgress || GetInstanceData(0, INDEX_INSTANCE_PROGRESS) != State_Finished)
            {
                RemoveIntroNpcs(true);
                UpdateGuards();
            }

            if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress)
            {
                if (GetInstanceData(0, INDEX_PORTAL_PROGRESS) == State_NotStarted)
                {
                    if (portalSummonTimer == 0)
                    {
                        portalSummonTimer = VH_NEXT_PORTAL_SPAWN_TIME;
                        SpawnPortal();
                        SetInstanceData(0, INDEX_PORTAL_PROGRESS, State_InProgress);
                    }
                    else
                        --portalSummonTimer;
                }
            }

            // Erase non existing summons from lists
            if (GetInstanceData(0, INDEX_PORTAL_PROGRESS) == State_InProgress && GetInstanceData(0, DATA_ARE_SUMMONS_MADE) == 1 && m_activePortal.type == VH_PORTAL_TYPE_SQUAD)
            {
                if (!m_activePortal.summonsList.empty())
                {
                    for (std::list<uint32>::iterator itr = m_activePortal.summonsList.begin(); itr != m_activePortal.summonsList.end();)
                    {
                        Creature* pCreature = GetInstance()->GetCreature(*itr);
                        if (pCreature && !pCreature->isAlive())
                        {
                            itr = m_activePortal.summonsList.erase(itr);
                            continue;
                        }
                        ++itr;
                    }
                }
                else
                {
                    SetInstanceData(0, INDEX_PORTAL_PROGRESS, State_Finished);
                }
            }

            // Erase non existing main event summons guids
            if (!m_eventSpawns.empty())
            {
                for (std::list<uint32_t>::iterator itr = m_eventSpawns.begin(); itr != m_eventSpawns.end();)
                {
                    Creature* pSummon = GetInstance()->GetCreature(*itr);
                    if (!pSummon || !pSummon->isAlive())
                    {
                        // Let players get their skinning loots
                        pSummon->Despawn(3 * 60 * 1000, 0); // 3 mins
                        itr = m_eventSpawns.erase(itr);
                        continue;
                    }
                    ++itr;
                }
            }
        }

        // Generate very basic portal info
        void GenerateRandomPortal(VHPortalInfo & newPortal)
        {
            uint8_t currentPortalCount = GetInstanceData(0, DATA_PORTAL_COUNT);
            uint8_t perviousPortal = currentPortalCount != 0 ? GetInstanceData(0, DATA_PERVIOUS_PORTAL_ID) : RandomUInt(MaxPortalPositions - 1);
            uint8_t newPortalId = RandomUInt(MaxPortalPositions - 1);

            if ((currentPortalCount + 1) != 6 && (currentPortalCount + 1) != 12 && (currentPortalCount + 1) != 18)
            {
                // Generate new portal id which doesn't match to pervious portal
                while (newPortalId == perviousPortal)
                {
                    newPortalId = RandomUInt(MaxPortalPositions - 1);
                }
                newPortal.id = newPortalId;

                // if portal id is between 0 and 4, its guardian/keeper type
                if (newPortal.id > 4)
                {
                    newPortal.type = VH_PORTAL_TYPE_SQUAD;
                }
                else
                {
                    newPortal.guardianEntry = RandomUInt(1) ? CN_PORTAL_GUARDIAN : CN_PORTAL_KEEPER;
                    newPortal.type = VH_PORTAL_TYPE_GUARDIAN;
                    // summon list data will published on spawn event
                }
            }
            // boss portal
            else
            {
                newPortal.type = VH_PORTAL_TYPE_BOSS;
                // Generate random boss entry
                while (newPortal.bossEntry == 0 || getData(newPortal.bossEntry) == Finished)
                {
                    newPortal.bossEntry = randomVHBossArray[RandomUInt(maxVHBosses - 1)];
                }
            }
        }

        // SpawnPortal
        void SpawnPortal()
        {
            uint8_t portalCount = GetInstanceData(0, DATA_PORTAL_COUNT);
            if (portalCount + 1 >= 18)
                return;

            ++portalCount;

            GenerateRandomPortal(m_activePortal);
            float x, y, z, o;
            if (m_activePortal.type != VH_PORTAL_TYPE_BOSS)
            {
                x = PortalPositions[m_activePortal.id].x;
                y = PortalPositions[m_activePortal.id].y;
                z = PortalPositions[m_activePortal.id].z;
                o = PortalPositions[m_activePortal.id].o;
            }
            else
            {
                x = BossPortalLoc.x;
                y = BossPortalLoc.y;
                z = BossPortalLoc.z;
                o = BossPortalLoc.o;
            }

            if (!spawnCreature(CN_PORTAL, x, y, z, o))
            {
                LOG_ERROR("Violet Hold: error spawning main event portal");
            }
            SetInstanceData(0, DATA_PORTAL_COUNT, portalCount);
            SetInstanceData(0, DATA_PERVIOUS_PORTAL_ID, m_activePortal.id);
            UpdateWorldStates();
        }
        /////////////////////////////////////////////////////////
        /// Helper functions
        ///

        void DoCrystalActivation()
        {
            spawnCreature(CN_DEFENSE_SYSTEM, DefenseSystemLocation.x, DefenseSystemLocation.y, DefenseSystemLocation.z, DefenseSystemLocation.o);
        }

        void ResetIntro()
        {
            SpawnIntro();

            // Return sinclari to spawn location
            if (Creature* pCreature = GetInstance()->GetCreature(m_sinclariGUID))
            {
                // Despawn and respawn her in 1 sec (2 seconds)
                pCreature->Despawn(1000, 1000);
            }
            else
            {
                // GUID will be set at OnCreaturePushToWorld event
                spawnCreature(CN_LIEUTNANT_SINCLARI, SinclariSpawnLoc.x, SinclariSpawnLoc.y, SinclariSpawnLoc.z, SinclariSpawnLoc.o);
            }
        }

        // Removes all dead intro npcs
        void RemoveIntroNpcs(bool deadOnly)
        {
            if (!m_introSpawns.empty())
            {
                for (std::list<uint32_t>::iterator itr = m_introSpawns.begin(); itr != m_introSpawns.end();)
                {
                    Creature* pIntroSummon = GetInstance()->GetCreature(*itr);
                    if (pIntroSummon && pIntroSummon->IsInInstance())
                    {
                        if (deadOnly)
                        {
                            if (!pIntroSummon->isAlive())
                            {
                                pIntroSummon->Despawn(4000, 0);
                                itr = m_introSpawns.erase(itr);
                                continue;
                            }
                        }
                        else
                        {
                            pIntroSummon->Despawn(4000, 0);
                            itr = m_introSpawns.erase(itr);
                            continue;
                        }
                    }
                    ++itr;
                }
            }
        }

        void RemoveIntroNpcByGuid(uint32_t guid)
        {
            if (!m_introSpawns.empty())
            {
                for (std::list<uint32_t>::iterator itr = m_introSpawns.begin(); itr != m_introSpawns.end(); ++itr)
                {
                    if ((*itr) == guid)
                    {
                        m_introSpawns.erase(itr);
                        break;
                    }
                }
            }
        }

        void UpdateWorldStates()
        {
            UpdateInstanceWorldState(WORLD_STATE_VH_SHOW, GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress ? 1 : 0);
            UpdateInstanceWorldState(WORLD_STATE_VH_PRISON_STATE, GetInstanceData(0, DATA_SEAL_HEALTH));
            UpdateInstanceWorldState(WORLD_STATE_VH_WAVE_COUNT, GetInstanceData(0, DATA_PORTAL_COUNT));
        }

        void UpdateInstanceWorldState(uint32_t field, uint32_t value)
        {
            WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
            data << field;
            data << value;
            GetInstance()->SendPacketToAllPlayers(&data);
        }

        // Spawn instance intro
        void SpawnIntro()
        {
            // Spawn guards
            for (uint8_t i = 0; i < guardsCount; i++)
            {
                Creature* pSummon = spawnCreature(CN_VIOLET_HOLD_GUARD, guardsSpawnLoc[i].x, guardsSpawnLoc[i].y, guardsSpawnLoc[i].z, guardsSpawnLoc[i].o);
                if (!pSummon)
                {
                    LOG_ERROR("Violet Hold: error occured while spawning creature entry %u", CN_VIOLET_HOLD_GUARD);
                }
            }

            // Spawn portals
            for (uint8_t i = 0; i < introPortalCount; i++)
            {
                Creature* pSummon = spawnCreature(CN_PORTAL_INTRO, introPortals[i].x, introPortals[i].y, introPortals[i].z, introPortals[i].o);
                if (!pSummon)
                {
                    LOG_ERROR("Violet Hold: error occured while spawning creature entry %u", CN_PORTAL_INTRO);
                }
            }
        }

        // Resets activation crystals
        void ResetCrystals(bool isSelectable)
        {
            for (std::list<uint32_t>::iterator itr = m_crystalGuids.begin(); itr != m_crystalGuids.end(); ++itr)
            {
                if (GameObject* pCrystal = GetInstance()->GetGameObject(*itr))
                {
                    pCrystal->SetState(GO_STATE_CLOSED);
                    if (!isSelectable)
                    {
                        pCrystal->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                    }
                    else
                    {
                        pCrystal->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                    }
                }
            }
        }

        /// Update achievement criteria for all players by id
        void UpdateAchievCriteriaForPlayers(uint32_t id, uint32_t criteriaCount)
        {
            for (PlayerStorageMap::iterator itr = GetInstance()->m_PlayerStorage.begin(); itr != GetInstance()->m_PlayerStorage.end(); ++itr)
            {
                Player* plr = (*itr).second;
                if (plr && plr->IsInWorld() && !plr->GetAchievementMgr().HasCompleted(id))
                {
                    plr->GetAchievementMgr().UpdateAchievementCriteria(plr, id, criteriaCount);
                    plr->SaveToDB(false);   // Make player happy ^^
                }
            }
        }

        // Calls guards out
        void CallGuardsOut()
        {
            if (m_guardsGuids.empty())
                return;

            for (std::list<uint32_t>::iterator itr = m_guardsGuids.begin(); itr != m_guardsGuids.end(); itr = m_guardsGuids.erase(itr))
            {
                if (Creature* pGuard = GetInstance()->GetCreature(*itr))
                {
                    pGuard->GetAIInterface()->setSplineRun();
                    pGuard->GetAIInterface()->MoveTo(introMoveLoc.x, introMoveLoc.y, introMoveLoc.z);
                    pGuard->Despawn(4500, 0);
                }
            }
        }

        void UpdateGuards()
        {
            if (m_guardsGuids.empty())
                return;

            for (std::list<uint32_t>::iterator itr = m_guardsGuids.begin(); itr != m_guardsGuids.end(); ++itr)
            {
                if (Creature* pGuard = GetInstance()->GetCreature(*itr))
                {
                    if (!pGuard->isAlive())
                    {
                        pGuard->Despawn(1000, 1000);
                    }
                    // hack fix to set their original facing
                    if (!pGuard->getcombatstatus()->IsInCombat() && pGuard->GetAIInterface()->MoveDone())
                    {
                        if (pGuard->GetOrientation() != pGuard->GetSpawnO())
                            pGuard->SetFacing(pGuard->GetSpawnO());
                    }
                }
            }
        }
};

/// ESCORT/GOSSIP EVENT

class SinclariAI : public CreatureAIScript
{
    public:

        static CreatureAIScript* Create(Creature* c) { return new SinclariAI(c); }
        SinclariAI(Creature* pCreature) : CreatureAIScript(pCreature), m_Step(0)
        {
            VH_instance = static_cast<TheVioletHoldScript*>(pCreature->GetMapMgr()->GetScript());
            pCreature->GetAIInterface()->setWalkMode(WALKMODE_WALK);
        }

        void StartEvent()
        {
            GetUnit()->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            RegisterAIUpdateEvent(1000);
        }

        void AIUpdate()
        {
            if (VH_instance && VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) != State_InProgress)
            {
                switch(m_Step)
                {
                    // Walk to crystal
                    case 0:
                    {
                        ModifyAIUpdateEvent(3000);
                        moveTo(SinclariPositions[0].x, SinclariPositions[0].y, SinclariPositions[0].z, false);
                    }break;
                    // Do emote and spawn defense system
                    case 1:
                    {
                        ModifyAIUpdateEvent(6000);
                        GetUnit()->EventAddEmote(EMOTE_ONESHOT_USESTANDING, 3000);
                        VH_instance->SetInstanceData(0, INDEX_INSTANCE_PROGRESS, State_Performed);
                    }break;
                    // Face to guards
                    case 2:
                    {
                        ModifyAIUpdateEvent(1000);
                        GetUnit()->SetFacing(SinclariPositions[0].o);
                    }break;
                    case 3:
                    {
                        ModifyAIUpdateEvent(3000);
                        sendDBChatMessage(YELL_SINCLARI_LEAVING);
                        GetUnit()->EventAddEmote(EMOTE_ONESHOT_SHOUT, 2000);
                        VH_instance->CallGuardsOut();
                    }break;
                    // Move her outside
                    case 4:
                    {
                        ModifyAIUpdateEvent(5500);
                        moveTo(SinclariPositions[1].x, SinclariPositions[1].y, SinclariPositions[1].z, false);
                    }break;
                    // Face her to gates and say goodbye
                    case 5:
                    {
                        GetUnit()->SetOrientation(SinclariPositions[1].o);
                        ModifyAIUpdateEvent(3500);
                        sendDBChatMessage(SAY_SINCLARI_CLOSING_GATES);
                    }break;
                    // Emote mimic
                    case 6:
                    {
                        GetUnit()->EventAddEmote(EMOTE_ONESHOT_TALK, 2000);
                        ModifyAIUpdateEvent(3000);
                    }break;
                    // Close the gates
                    case 7:
                    {
                        VH_instance->SetInstanceData(0, INDEX_INSTANCE_PROGRESS, State_PreProgress);
                        ModifyAIUpdateEvent(1000);
                    }break;
                    // Move her further
                    case 8:
                    {
                        ModifyAIUpdateEvent(1602);
                        moveTo(SinclariPositions[2].x, SinclariPositions[2].y, SinclariPositions[2].z, false);
                    }break;
                    case 9:
                    {
                        GetUnit()->SetFacing(SinclariPositions[2].o);
                        GetUnit()->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        VH_instance->SetInstanceData(0, INDEX_INSTANCE_PROGRESS, State_InProgress);
                        RemoveAIUpdateEvent();
                    }break;
                    default:
                        break;
                }

                ++m_Step;
            }

            // Outro
            if (VH_instance && VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_Finished)
            {
                switch(m_Step)
                {
                    case 10:
                    {
                        moveTo(SinclariPositions[3].x, SinclariPositions[3].y, SinclariPositions[3].z, false);
                        ModifyAIUpdateEvent(4000);
                    }break;
                    case 11:
                    {
                        sendDBChatMessage(SAY_SINCLARI_INSTANCE_DONE);
                        ModifyAIUpdateEvent(10000);
                    }break;
                    case 12:
                    {
                        GetUnit()->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    }break;
                    default:
                        break;
                }
                ++m_Step;
            }
        }

    protected:

        uint32_t aiUpdateFreq;
        uint8_t m_Step;
        TheVioletHoldScript* VH_instance;
};

class SinclariGossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* pPlayer)
        {
            InstanceScript* pInstance = pObject->GetMapMgr()->GetScript();
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 1);
            if (pInstance)
            {
                // If instance is not finished, show escort gossip option
                if (pInstance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_NotStarted)
                {
                    menu.setTextID(13853);
                    menu.AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_SINCLARI_ACTIVATE), 0);
                }
                // Show option to port player to dungeon
                else if (pInstance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress)
                {
                    menu.setTextID(14271);
                    menu.AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_SINCLARI_SEND_ME_IN), 1);
                }
                menu.Send(pPlayer);
            }
        }

        void OnSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/)
        {
            switch (Id)
            {
                // Start escort event
                case 0:
                {
                    Arcemu::Gossip::Menu menu(pObject->GetGUID(), 13854);
                    menu.AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_SINCLARI_GET_SAFETY), 2);
                    menu.Send(pPlayer);
                }break;
                // Teleport player in
                case 1:
                {
                    Arcemu::Gossip::Menu::Complete(pPlayer);
                    static_cast<Creature*>(pObject)->CastSpell(pPlayer, SPELL_VH_TELEPORT_PLAYER, true);
                }break;
                // Start instance event
                case 2:
                {
                    if (Creature* pCreature = static_cast<Creature*>(pObject))
                    {
                        if (SinclariAI *pScript = static_cast<SinclariAI*>(pCreature->GetScript()))
                        {
                            // Remove gossip flag
                            pScript->StartEvent();
                        }
                    }
                    Arcemu::Gossip::Menu::Complete(pPlayer);
                }break;
                default:
                    break;
            }
        }
};

class IntroPortalAI : public CreatureAIScript
{
    TheVioletHoldScript* VH_instance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new IntroPortalAI(c); }
        IntroPortalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            VH_instance = static_cast< TheVioletHoldScript*>(pCreature->GetMapMgr()->GetScript());
            if (VH_instance)
            {
            }
        }

        void OnDespawn()
        {
            // Make sure guid is removed from list
            if (VH_instance)
            {
                VH_instance->RemoveIntroNpcByGuid(GET_LOWGUID_PART(GetUnit()->GetLowGUID()));
            }
        }

        void OnLoad()
        {
            setRooted(true);
            GetUnit()->m_canRegenerateHP = false;
            setCanEnterCombat(false);
            GetUnit()->Phase(PHASE_SET, 1);
            // Register AIUpdate event in 2 sec
            RegisterAIUpdateEvent(2000);
        }

        void OnDied(Unit* pKiller)
        {
            despawn();
        }

        void AIUpdate()
        {
            // Summon adds every 15 or 20 seconds
            if (VH_instance && !(VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress || VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_Performed))
            {
                ModifyAIUpdateEvent(RandomUInt(1) ? VH_TIMER_SPAWN_INTRO_MOB_MIN : VH_TIMER_SPAWN_INTRO_MOB_MAX);
                spawnCreature(VHIntroMobs[RandomUInt(VHIntroMobCount - 1)], GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), GetUnit()->GetPositionZ(), GetUnit()->GetOrientation());
            }
       }
};

class VHAttackerAI : public CreatureAIScript
{
    public:

        static CreatureAIScript* Create(Creature* c) { return new VHAttackerAI(c); }
        VHAttackerAI(Creature* pCreature) : CreatureAIScript(pCreature), isMoveSet(false)
        {
            RegisterAIUpdateEvent(2000);
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        }

        void OnCombatStop(Unit* /*pEnemy*/)
        {
            // Stop any movement
            GetUnit()->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            GetUnit()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
            stopMovement();

            // Run to gates
            GetUnit()->GetAIInterface()->setSplineRun();
            moveTo(sealAttackLoc.x, sealAttackLoc.y, sealAttackLoc.z);

            // Cast spell to seal
            Creature* pTriggerTarget = getNearestCreature(CN_DOOR_SEAL);
            if (pTriggerTarget && !GetUnit()->GetAIInterface()->isCreatureState(MOVING))
            {
                GetUnit()->GetAIInterface()->setFacing(M_PI_FLOAT);
                GetUnit()->SetChannelSpellId(SPELL_VH_DESTROY_DOOR_SEAL);
                GetUnit()->SetChannelSpellTargetGUID(pTriggerTarget->GetGUID());
                RemoveAIUpdateEvent();
                //GetUnit()->CastSpellAoF(pTriggerTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_DESTROY_DOOR_SEAL), false);
            }
        }

        void OnDied(Unit* /*pKiller*/)
        {
            // Stop channelling (this can happen if player will use crystal)
            GetUnit()->SetChannelSpellId(0);
            GetUnit()->SetChannelSpellTargetGUID(0);
            RemoveAIUpdateEvent();
        }

        void OnCombatStart(Unit* /*pKiller*/)
        {
            // Stop channeling and fight...
            GetUnit()->SetChannelSpellId(0);
            GetUnit()->SetChannelSpellTargetGUID(0);
            RegisterAIUpdateEvent(1000);
        }

        void AIUpdate()
        {
            if (!GetUnit()->getcombatstatus()->IsInCombat())
            {
                if (!isMoveSet)
                {
                    _unit->GetAIInterface()->setSplineRun();
                    moveTo(sealAttackLoc.x, sealAttackLoc.y, sealAttackLoc.z);
                    isMoveSet = true;
                }
                InstanceScript* pInstance = GetUnit()->GetMapMgr()->GetScript();
                if (pInstance && pInstance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress && !GetUnit()->GetAIInterface()->isCreatureState(MOVING))
                {
                    Creature* pTriggerTarget = getNearestCreature(CN_DOOR_SEAL);
                    if (pTriggerTarget)
                    {
                        GetUnit()->GetAIInterface()->setFacing(M_PI_FLOAT);
                        GetUnit()->SetChannelSpellId(SPELL_VH_DESTROY_DOOR_SEAL);
                        GetUnit()->SetChannelSpellTargetGUID(pTriggerTarget->GetGUID());
                        RemoveAIUpdateEvent();
                        //GetUnit()->CastSpellAoF(pTriggerTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_DESTROY_DOOR_SEAL), false);
                    }
                }
            }
        }

    protected:

        bool isMoveSet;
};

class VH_DefenseAI : public CreatureAIScript
{
        uint32_t counter;
    public:

        static CreatureAIScript* Create(Creature* c) { return new VH_DefenseAI(c); }
        VH_DefenseAI(Creature* pCreature) : CreatureAIScript(pCreature), counter(0)
        {
            RegisterAIUpdateEvent(1000);
            despawn(7000);
        }

        void AIUpdate()
        {
            GetUnit()->CastSpell(GetUnit(), SPELL_VH_DEFENSE_SYSTEM_SPAWN, true);
            GetUnit()->CastSpell(GetUnit(), SPELL_VH_DEFENSE_SYSTEM_VISUAL, true);
            if (TheVioletHoldScript* pInstance = static_cast<TheVioletHoldScript*>(GetUnit()->GetMapMgr()->GetScript()))
            {
                // Intro spawns
                if (!pInstance->m_introSpawns.empty())
                {
                    for (std::list<uint32_t>::iterator itr = pInstance->m_introSpawns.begin(); itr != pInstance->m_introSpawns.end(); ++itr)
                    {
                        if (counter == 3)
                        {
                            if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                            {
                                GetUnit()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_ARCANE_LIGHTNING_INSTAKILL), true);
                                // Make sure they all dies
                                pTarget->Die(pTarget, pTarget->GetHealth(), 0);
                                pTarget->Despawn(1000, 0);
                            }
                        }
                        else
                        {
                            if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                            {
                                // Despawn portals
                                if (pTarget->GetEntry() == CN_PORTAL_INTRO)
                                {
                                    pTarget->Despawn(1000, 0);
                                }
                                GetUnit()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                            }
                        }
                    }
                }

                // Main event spawns
                if (!pInstance->m_eventSpawns.empty())
                {
                    for (std::list<uint32_t>::iterator itr = pInstance->m_eventSpawns.begin(); itr != pInstance->m_eventSpawns.end(); ++itr)
                    {
                        if (counter == 3)
                        {
                            if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                            {
                                GetUnit()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_ARCANE_LIGHTNING_INSTAKILL), true);
                                // Make sure they all dies
                                pTarget->Die(pTarget, pTarget->GetHealth(), 0);
                            }
                        }
                        else
                        {
                            if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                            {
                                GetUnit()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                            }
                        }
                    }
                }

                // Defense triggers
                if (!pInstance->m_defenseTriggers.empty() && counter < 3)
                {
                    for (std::list<uint32_t>::iterator itr = pInstance->m_defenseTriggers.begin(); itr != pInstance->m_defenseTriggers.end(); ++itr)
                    {
                        if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                        {
                            GetUnit()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                        }
                    }
                }
                ++counter;
            }
        }
};

class TeleportationPortalAI : public CreatureAIScript
{
        bool isGuardianSpawned;
        TheVioletHoldScript* pInstance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new TeleportationPortalAI(c); }
        TeleportationPortalAI(Creature* pCreature) : CreatureAIScript(pCreature), isGuardianSpawned(false)
        {
            pInstance = static_cast<TheVioletHoldScript*>(GetUnit()->GetMapMgr()->GetScript());
            if (pInstance)
            {
                if (pInstance->m_activePortal.type != VH_PORTAL_TYPE_BOSS)
                    RegisterAIUpdateEvent(VH_TELE_PORTAL_SPAWN_TIME);
                else
                    RegisterAIUpdateEvent(VH_TELE_PORTAL_BOSS_SPAWN_TIME);
            }
        }

        void AIUpdate()
        {
            if (!pInstance)
            {
                printf("ERROR \n");
                return;
            }
            switch (pInstance->m_activePortal.type)
            {
                case VH_PORTAL_TYPE_NONE:
                    break;
                case VH_PORTAL_TYPE_GUARDIAN:
                {
                    float landHeight = GetUnit()->GetMapMgr()->GetLandHeight(GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), GetUnit()->GetPositionZ());
                    if (!isGuardianSpawned)
                    {
                        GetUnit()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, pInstance->m_activePortal.guardianEntry == CN_PORTAL_GUARDIAN ? GUARDIAN_ANNOUNCE : KEEPER_ANNOUNCE);
                        Creature* pGuardian = spawnCreature(pInstance->m_activePortal.guardianEntry, GetUnit()->GetPositionX() + RandomFloat(3), GetUnit()->GetPositionY() + RandomFloat(3), landHeight, GetUnit()->GetOrientation());
                        if (pGuardian)
                        {
                            isGuardianSpawned = true;
                            GetUnit()->SetChannelSpellId(SPELL_VH_PORTAL_CHANNEL);
                            GetUnit()->SetChannelSpellTargetGUID(pGuardian->GetGUID());
                        }
                    }
                    else
                    {
                        // Spawn 3 random portal guardians
                        for(uint8 i = 0; i < 3; i++)
                        {
                            Creature* pSummon = spawnCreature(portalGuardians[RandomUInt(maxPortalGuardians - 1)], GetUnit()->GetPositionX() + RandomFloat(3), GetUnit()->GetPositionY() + RandomFloat(3), landHeight, GetUnit()->GetOrientation());
                            if (pSummon)
                            {
                                // In case if OnLoad event will fail, insert guid anyway
                                //TODO: replace this with cleaner solution
                                pInstance->m_eventSpawns.push_back(GET_LOWGUID_PART(pSummon->GetGUID()));
                            }
                        }
                    }
                }break;
                case VH_PORTAL_TYPE_SQUAD:
                {
                    GetUnit()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, SQUAD_ANNOUNCE);
                    //TODO: This count needs to be corrected
                    for(uint8 i = 0; i < 5; i++)
                    {
                        Creature* pSummon = spawnCreature(portalGuardians[RandomUInt(maxPortalGuardians - 1)], GetUnit()->GetPositionX() + RandomFloat(3), GetUnit()->GetPositionY() + RandomFloat(3), GetUnit()->GetPositionZ(), GetUnit()->GetOrientation());
                        if (pSummon)
                        {
                            //TODO: replace this with cleaner solution
                            pInstance->m_activePortal.summonsList.push_back(GET_LOWGUID_PART(pSummon->GetGUID()));
                            pInstance->m_eventSpawns.push_back(GET_LOWGUID_PART(pSummon->GetGUID()));
                        }
                    }
                    despawn(2000, 0);
                }break;
                case VH_PORTAL_TYPE_BOSS:
                {

                }break;
            }
            pInstance->SetInstanceData(0, DATA_ARE_SUMMONS_MADE, 1);
        }
};

// Spells
bool TeleportPlayerInEffect(uint32 /*i*/, Spell* pSpell)
{
    if (pSpell->u_caster == nullptr || !pSpell->u_caster->IsCreature() || !pSpell->GetPlayerTarget())
    {
        return false;
    }

    if (Player* pTarget = pSpell->GetPlayerTarget())
    {
        return pTarget->CastSpell(pTarget, SPELL_VH_TELEPORT_PLAYER_EFFECT, true) ==  SPELL_CANCAST_OK ? true : false;
    }

    return false;
}

bool DestroyDoorSealDummy(uint32 i, Aura* pAura, bool apply)
{
    if (!apply)
        return false;

    Unit* pCaster = pAura->GetUnitCaster();
    if (!pCaster)
        return false;

    InstanceScript* pInstance = pCaster->GetMapMgr()->GetScript();
    if (!pInstance)
        return false;

    pInstance->SetInstanceData(0, DATA_SEAL_HEALTH, pInstance->GetInstanceData(0, DATA_SEAL_HEALTH) - 1);
    return true;
}

void SetupTheVioletHold(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_VIOLET_HOLD, &TheVioletHoldScript::Create);

    //Sinclari gossip/escort event
    mgr->register_creature_script(CN_LIEUTNANT_SINCLARI, &SinclariAI::Create);
    mgr->register_creature_gossip(CN_LIEUTNANT_SINCLARI, new SinclariGossip);

    // Intro event
    mgr->register_creature_script(CN_PORTAL_INTRO, &IntroPortalAI::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_BINDER_ARCANE, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_INVADER_ARMS, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_MAGE_SLAYER_MELEE, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_SPELLBREAKER_ARCANE, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_DEFENSE_SYSTEM, &VH_DefenseAI::Create);

    // Main portal event
    mgr->register_creature_script(CN_PORTAL, &TeleportationPortalAI::Create);
    mgr->register_creature_script(CN_AZURE_INVADER, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_AZURE_SPELLBREAKER, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_AZURE_BINDER, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_AZURE_MAGE_SLAYER, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_AZURE_CAPTAIN, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_AZURE_SORCERER, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_AZURE_RAIDER, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_AZURE_STALKER, &VHAttackerAI::Create);

    // Spells
    mgr->register_script_effect(SPELL_VH_TELEPORT_PLAYER, &TeleportPlayerInEffect);
    mgr->register_dummy_aura(SPELL_VH_DESTROY_DOOR_SEAL, &DestroyDoorSealDummy);
}
