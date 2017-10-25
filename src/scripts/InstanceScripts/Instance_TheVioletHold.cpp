/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheVioletHold.h"

class TheVioletHoldScript : public InstanceScript
{
    uint32_t m_VHencounterData[INDEX_MAX];
    bool introStarted;

    // Achievements
    bool m_isZuramatAchievFailed;
    bool m_isDefAchievFailed;

    // Guards
    std::list<uint32_t /*guid*/> m_guardsGuids;

    // Guids holder for portal waves
    std::vector<uint32_t /*guid*/> m_eventSpawns;

    // Activation crystal guids
    std::list<uint32_t /*guid*/> m_crystalGuids;

    // Low guids of gameobjects
    uint32_t mainGatesGUID;

    // Low guids of creatures
    uint32_t m_sinclariGUID;

    // used for gates seal
    uint32_t sealHP;
    uint32_t portalCount;

    // List holds summoned intro creatures guids before main event
    std::list<uint32_t> intro_spawns;

    public:

        static InstanceScript* Create(MapMgr* pMapMgr) { return new TheVioletHoldScript(pMapMgr); }
        TheVioletHoldScript(MapMgr* pMapMgr) :
            InstanceScript(pMapMgr),
            introStarted(false),
            m_isZuramatAchievFailed(false),
            m_isDefAchievFailed(false),
            mainGatesGUID(0),
            m_sinclariGUID(0),
            sealHP(100),
            portalCount(0)
        {
            //TODO: this should be redone by checking actual saved data for heroic mode
            memset(m_VHencounterData, State_NotStarted, sizeof(m_VHencounterData));
        }

        void SetInstanceData(uint32_t /*pType*/, uint32_t pIndex, uint32_t pData)
        {
            if (pIndex >= INDEX_MAX)
            {
                return;
            }

            switch (pIndex)
            {
                case INDEX_INSTANCE_PROGRESS:
                {
                    if (pData == State_Performed)
                    {
                        spawnCreature(CN_DEFENSE_SYSTEM, DefenseSystemLocation.x, DefenseSystemLocation.y, DefenseSystemLocation.z, DefenseSystemLocation.o);
                    }

                    if (pData == State_InProgress)
                    {
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

                        FillInitialWorldStates();
                    }

                    if (pData == State_Failed)
                    {
                        ResetIntro();
                        ResetCrystals(true);
                        SetInstanceData(0, INDEX_INSTANCE_PROGRESS, State_NotStarted);
                    }
                }break;
                default:
                    break;
            }
            m_VHencounterData[pIndex] = pData;
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
            FillInitialWorldStates();   // Only for testing, this will need to be repalced

            // For most creatures movements mmaps is needed
            if (!sWorld.settings.terrainCollision.isPathfindingEnabled)
            {
                LOG_ERROR("Violet Hold: dungeon requires pathfinding support.");
            }

            // Spawn intro
            if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_NotStarted && !introStarted)
            {
                ResetIntro();
                introStarted = true;
            }
            RegisterUpdateEvent(1000);
        }

        void OnGameObjectPushToWorld(GameObject* pGo)
        {
            switch(pGo->GetEntry())
            {
                case GO_PRISON_SEAL:
                {
                    mainGatesGUID = pGo->GetLowGUID();
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

                // Spell actually sends script event 20001 but theres no handling for such effects
                // Lets cast spell to mimic original event
                plr->CastSpell(plr, SPELL_VH_CRYSTAL_ACTIVATION, true);

                // Defense system will do its event on spawn, nothing else we don't need to do
                spawnCreature(CN_DEFENSE_SYSTEM, DefenseSystemLocation.x, DefenseSystemLocation.y, DefenseSystemLocation.z, DefenseSystemLocation.o);
            }
        }

        void OnCreaturePushToWorld(Creature* pCreature)
        {
            switch(pCreature->GetEntry())
            {
                case CN_VIOLET_HOLD_GUARD:
                {
                    m_guardsGuids.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                    pCreature->Phase(PHASE_SET, 1);
                    pCreature->setByteFlag(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
                }break;
                case CN_LIEUTNANT_SINCLARI:
                {
                    m_sinclariGUID = GET_LOWGUID_PART(pCreature->GetGUID());
                    pCreature->setByteFlag(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
                }break;
                case CN_PORTAL_INTRO:
                case CN_INTRO_AZURE_BINDER_ARCANE:
                case CN_INTRO_AZURE_INVADER_ARMS:
                case CN_INTRO_AZURE_MAGE_SLAYER_MELEE:
                case CN_INTRO_AZURE_SPELLBREAKER_ARCANE:
                {
                    intro_spawns.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                }break;
                case CN_DEFENSE_SYSTEM:
                default:
                    break;
            }
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
                }break;
                default:
                    break;
            }
        }

        void UpdateEvent()
        {
            // If instance currently is performed for main event
            if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) != State_InProgress)
            {
                if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) > State_NotStarted)
                {
                    RemoveIntroNpcs(true);
                }
                else
                {
                    RemoveIntroNpcs(false);
                }
            }
        }

        /////////////////////////////////////////////////////////
        /// Helper functions
        ///

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
                spawnCreature(CN_LIEUTNANT_SINCLARI, SinclariSpawnLoc.x, SinclariSpawnLoc.y, SinclariSpawnLoc.z, SinclariSpawnLoc.o);
            }
        }

        // Removes all dead intro npcs
        void RemoveIntroNpcs(bool checkForDead)
        {
            // In some cases intro npcs aren't despawned on OnDied event
            if (!intro_spawns.empty())
            {
                for (std::list<uint32_t>::iterator itr = intro_spawns.begin(); itr != intro_spawns.end();)
                {
                    if (Creature* pIntroSummon = GetInstance()->GetCreature(*itr))
                    {
                        if (pIntroSummon->IsInInstance())
                        {
                            if (checkForDead && pIntroSummon->isAlive())
                            {
                                pIntroSummon->Despawn(4000, 0);
                                itr = intro_spawns.erase(itr);
                                continue;
                            }
                            else
                            {
                                pIntroSummon->Despawn(4000, 0);
                                itr = intro_spawns.erase(itr);
                                continue;
                            }
                        }
                    }
                    ++itr;
                }
            }
        }

        // Removes intro npc from list by guid
        // ONLY called on despawn event
        void RemoveIntroNpcByGuid(uint32_t guid)
        {
            if (!intro_spawns.empty())
            {
                for (std::list<uint32_t>::iterator itr = intro_spawns.begin(); itr != intro_spawns.end(); ++itr)
                {
                    if ((*itr) == guid)
                    {
                        intro_spawns.erase(itr);
                        break;
                    }
                }
            }
        }

        void FillInitialWorldStates()
        {
            GetInstance()->GetWorldStatesHandler().SetWorldStateForZone(0, VH_DEFAULT_AREA, WORLD_STATE_VH_SHOW, 1);
            GetInstance()->GetWorldStatesHandler().SetWorldStateForZone(0, VH_DEFAULT_AREA, WORLD_STATE_VH_PRISON_STATE, sealHP);
            GetInstance()->GetWorldStatesHandler().SetWorldStateForZone(0, VH_DEFAULT_AREA, WORLD_STATE_VH_WAVE_COUNT, portalCount);
        }

        // Spawn instance intro
        void SpawnIntro()
        {
            // Spawn guards
            for (uint8_t i = 0; i < guardsCount; i++)
            {
                spawnCreature(CN_VIOLET_HOLD_GUARD, guardsSpawnLoc[i].x, guardsSpawnLoc[i].y, guardsSpawnLoc[i].z, guardsSpawnLoc[i].o))
            }

            // Spawn portals
            for (uint8_t i = 0; i < introPortalCount; i++)
            {
                spawnCreature(CN_PORTAL_INTRO, introPortals[i].x, introPortals[i].y, introPortals[i].z, introPortals[i].o);
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

        /// Calls guards out
        void CallGuardsOut()
        {
            for (std::list<uint32_t>::iterator itr = m_guardsGuids.begin(); itr != m_guardsGuids.end();)
            {
                if (Creature* pGuard = GetInstance()->GetCreature(*itr))
                {
                    pGuard->GetAIInterface()->setWalkMode(WALKMODE_RUN);
                    pGuard->GetAIInterface()->MoveTo(introMoveLoc.x, introMoveLoc.y, introMoveLoc.z);
                    pGuard->Despawn(6000, 0); // Despawn in 6 sec
                }
                itr = m_guardsGuids.erase(itr);
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
            GetUnit()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            RegisterAIUpdateEvent(1000);
        }

        void AIUpdate()
        {
            if (VH_instance && (VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_NotStarted || VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_Performed))
            {
                switch(m_Step)
                {
                    // Walk to crystal
                    case 0:
                    {
                        ModifyAIUpdateEvent(2000);
                        moveTo(SinclariPositions[0].x, SinclariPositions[0].y, SinclariPositions[0].z);
                    }break;
                    // Update facing
                    case 1:
                    {
                        ModifyAIUpdateEvent(500);
                        GetUnit()->SetFacing(SinclariPositions[0].o);
                    }break;
                    // Do emote and spawn defense system
                    case 2:
                    {
                        ModifyAIUpdateEvent(3000);
                        GetUnit()->Emote(EMOTE_ONESHOT_USESTANDING);
                    }break;
                    case 3:
                    {
                        ModifyAIUpdateEvent(1000);
                        VH_instance->SetInstanceData(0, INDEX_INSTANCE_PROGRESS, State_Performed);
                    }break;
                    // Face to guards
                    case 4:
                    {
                        GetUnit()->SetFacing(6.239587f);
                        ModifyAIUpdateEvent(2000);
                    }break;
                    // call them out
                    case 5:
                    {
                        GetUnit()->Emote(EMOTE_ONESHOT_SHOUT);
                        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, SINCLARI_YELL);
                        VH_instance->CallGuardsOut();
                        // Call guards out of dungeon
                    }break;
                    // Move her to guards
                    case 6:
                    {
                        ModifyAIUpdateEvent(4000);
                        moveTo(SinclariPositions[1].x, SinclariPositions[1].y, SinclariPositions[1].z, false);
                    }break;
                    // Face her to gates (shes outside of door)
                    case 7:
                    {
                        ModifyAIUpdateEvent(1000);
                        GetUnit()->SetFacing(SinclariPositions[1].o);
                    }break;
                    // Goodbye everyone
                    case 8:
                    {
                        ModifyAIUpdateEvent(6000);
                        sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, SINCLARI_SAY);
                    }break;
                    // Start instance event
                    case 9:
                    {
                        RemoveAIUpdateEvent();
                        VH_instance->SetInstanceData(0, INDEX_INSTANCE_PROGRESS, State_InProgress);
                        moveTo(SinclariPositions[2].x, SinclariPositions[2].y, SinclariPositions[2].z, false);
                        // GetUnit()->SetFacing(M_PI_FLOAT);
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
                    menu.AddItem(GOSSIP_ICON_CHAT, SINCLARI_GO_OPTION1, 0);
                }
                // else show option to port player to dungeon
                else
                {
                    menu.setTextID(14271);
                    menu.AddItem(GOSSIP_ICON_CHAT, SINCLARI_GO_OPTION3, 1);
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
                    menu.AddItem(GOSSIP_ICON_CHAT, SINCLARI_GO_OPTION2, 2);
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
    InstanceScript* VH_instance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new IntroPortalAI(c); }
        IntroPortalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            VH_instance = pCreature->GetMapMgr()->GetScript();
        }

        void OnDespawn()
        {
            // Make sure guid is removed from list
            if (TheVioletHoldScript* pInstance = static_cast<TheVioletHoldScript*>(GetUnit()->GetMapMgr()->GetScript()))
            {
                pInstance->RemoveIntroNpcByGuid(GET_LOWGUID_PART(GetUnit()->GetLowGUID()));
            }
        }

        void OnLoad()
        {
            setRooted(true);
            GetUnit()->m_canRegenerateHP = false;
            setCanEnterCombat(false);
            GetUnit()->Phase(PHASE_SET, 1);
            // Register AIUpdate event in 2 sec
            RegisterAIUpdateEvent(5000);
        }

        void AIUpdate()
        {
            // Summon adds every 40 or 60 seconds
            if (VH_instance && !(VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress || VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_Performed))
            {
                ModifyAIUpdateEvent(RandomUInt(1) ? 40000 : 60000);
                spawnCreature(VHIntroMobs[RandomUInt(VHIntroMobCount - 1)], GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), GetUnit()->GetPositionZ(), M_PI_FLOAT);
            }
       }
};

class VHIntroNpcAI : public CreatureAIScript
{
    public:

        static CreatureAIScript* Create(Creature* c) { return new VHIntroNpcAI(c); }
        VHIntroNpcAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Lets start init event
            RegisterAIUpdateEvent(1000);
        }

        void OnDespawn()
        {
            // Make sure guid is removed from list
            if (TheVioletHoldScript* pInstance = static_cast<TheVioletHoldScript*>(GetUnit()->GetMapMgr()->GetScript()))
            {
                pInstance->RemoveIntroNpcByGuid(GET_LOWGUID_PART(GetUnit()->GetLowGUID()));
            }
        }

        void OnCombatStop(Unit* /*pEnemy*/)
        {
            GetUnit()->GetAIInterface()->MoveTo(sealAttackLoc.x, sealAttackLoc.y, sealAttackLoc.z, sealAttackLoc.o);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            if (!GetUnit()->CombatStatus.IsInCombat())
            {
                GetUnit()->GetAIInterface()->setWalkMode(WALKMODE_RUN);
                GetUnit()->GetAIInterface()->MoveTo(sealAttackLoc.x, sealAttackLoc.y, sealAttackLoc.z, sealAttackLoc.o);
                RemoveAIUpdateEvent();
            }
        }
};

class VH_guardAI : public CreatureAIScript
{
    public:

        static CreatureAIScript* Create(Creature* c) { return new VH_guardAI(c); }
        VH_guardAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* /*pEnemey*/)
        {
            // Correctly set facing
            InstanceScript* pInstance = GetUnit()->GetMapMgr()->GetScript();
            if (pInstance && pInstance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) != State_Performed)
            {
                GetUnit()->SetFacing(GetUnit()->GetSpawnO());
            }
        }
};

class VH_DefenseAI : public CreatureAIScript
{
        uint32_t counter;
    public:

        static CreatureAIScript* Create(Creature* c) { return new VH_DefenseAI(c); }
        VH_DefenseAI(Creature* pCreature) : CreatureAIScript(pCreature), counter(0)
        {
            RegisterAIUpdateEvent(1000);
            GetUnit()->CastSpell(pCreature, sSpellCustomizations.GetSpellInfo(SPELL_VH_DEFENSE_SYSTEM_VISUAL), true);
        }

        void AIUpdate()
        {
            // Make storm animation 3 times
            if (counter < 3)
            {
                GetUnit()->CastSpell(GetUnit(), SPELL_VH_LIGHTNING_INTRO, true);
                ++counter;
            }

            // You were warned 3 times with animation, lets purge xD
            if (counter == 3)
            {
                GetUnit()->CastSpell(GetUnit(), SPELL_VH_ARCANE_LIGHTNING_INSTAKILL, false);
                despawn(1000, 0);
            }
        }
};

// Spells
bool TeleporPlayerInEffect(uint32 /*i*/, Spell* pSpell)
{
    if (pSpell->u_caster == nullptr || !pSpell->u_caster->IsCreature() || !pSpell->GetPlayerTarget())
    {
        return false;
    }

    Player* pTarget = pSpell->GetPlayerTarget();

    pTarget->CastSpell(pTarget, SPELL_VH_TELEPORT_PLAYER_EFFECT, true);
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
    mgr->register_creature_script(CN_INTRO_AZURE_BINDER_ARCANE, &VHIntroNpcAI::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_INVADER_ARMS, &VHIntroNpcAI::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_MAGE_SLAYER_MELEE, &VHIntroNpcAI::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_SPELLBREAKER_ARCANE, &VHIntroNpcAI::Create);
    mgr->register_creature_script(CN_VIOLET_HOLD_GUARD, &VH_guardAI::Create);
    mgr->register_creature_script(CN_DEFENSE_SYSTEM, &VH_DefenseAI::Create);

    // Spells
    mgr->register_script_effect(SPELL_VH_TELEPORT_PLAYER, &TeleporPlayerInEffect);
}
