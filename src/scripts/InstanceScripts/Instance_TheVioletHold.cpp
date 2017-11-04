/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheVioletHold.h"
#include "Spell/SpellAuras.h"
#include "Objects/Faction.h"

// Helper functions

uint32_t GenerateWaitTime(Creature* pCreature, float newX, float currentX, float newY, float currentY)
{
    if (!pCreature)
        return 0;

    float distanceX = (newX - currentX) * (newX - currentX);
    float distanceY = (newY - currentY) * (newY - currentY);
    float distance = sqrt(distanceX + distanceY);
    return  static_cast<uint32_t>((1000 * std::abs(distance / pCreature->GetCreatureProperties()->run_speed)) - 1000);
}

Movement::WayPoint* CreateWaypoint(uint32_t pId, uint32_t waitTime, Movement::Location pCoords)
{
    Movement::WayPoint* wp = new Movement::WayPoint();
    wp->id = pId;
    wp->x = pCoords.x;
    wp->y = pCoords.y;
    wp->z = pCoords.z;
    wp->o = pCoords.o;
    wp->waittime = waitTime;
    wp->flags = Movement::WP_MOVE_TYPE_RUN;
    wp->forwardemoteoneshot = false;
    wp->forwardemoteid = 0;
    wp->backwardemoteoneshot = false;
    wp->backwardemoteid = 0;
    wp->forwardskinid = 0;
    wp->backwardskinid = 0;
    return wp;
}

/// ESCORT/GOSSIP EVENT
class SinclariAI : public CreatureAIScript
{
    public:

        static CreatureAIScript* Create(Creature* c) { return new SinclariAI(c); }
        SinclariAI(Creature* pCreature) : CreatureAIScript(pCreature), m_Step(0)
        {
            VH_instance = static_cast<TheVioletHoldInstance*>(pCreature->GetMapMgr()->GetScript());
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
        TheVioletHoldInstance* VH_instance;
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
    TheVioletHoldInstance* VH_instance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new IntroPortalAI(c); }
        IntroPortalAI(Creature* pCreature) : CreatureAIScript(pCreature), portalId(-1)
        {
            VH_instance = static_cast< TheVioletHoldInstance*>(pCreature->GetMapMgr()->GetScript());
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
            if (VH_instance && !(VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress || VH_instance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_Performed) && portalId != -1)
            {
                float landHeight = GetUnit()->GetMapMgr()->GetLandHeight(GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), GetUnit()->GetPositionZ());
                ModifyAIUpdateEvent(RandomUInt(1) ? VH_TIMER_SPAWN_INTRO_MOB_MIN : VH_TIMER_SPAWN_INTRO_MOB_MAX);
                if (Creature* pAttacker = spawnCreature(VHIntroMobs[RandomUInt(VHIntroMobCount - 1)], GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), landHeight, GetUnit()->GetOrientation()))
                {
                    switch (portalId)
                    {
                        case 0: // left
                        {
                            for (uint8_t i = 0; i < MaxFirstPortalWPS; i++)
                            {
                                uint32_t waitTime = 1000;
                                if (i == 0)
                                    waitTime += GenerateWaitTime(GetUnit(), FirstPortalWPs[i].x, GetUnit()->GetPositionX(), FirstPortalWPs[i].y, GetUnit()->GetPositionY());
                                else
                                    waitTime += GenerateWaitTime(GetUnit(), FirstPortalWPs[i].x, FirstPortalWPs[i - 1].x, FirstPortalWPs[i].y, FirstPortalWPs[i - 1].y);

                                pAttacker->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, FirstPortalWPs[i]));
                            }
                        }break;
                        case 1: // right
                        {
                            for (uint8_t i = 0; i < MaxFifthPortalWPS; i++)
                            {
                                uint32_t waitTime = 1000;
                                if (i == 0)
                                    waitTime += GenerateWaitTime(GetUnit(), FifthPortalWPs[i].x, GetUnit()->GetPositionX(), FifthPortalWPs[i].y, GetUnit()->GetPositionY());
                                else
                                    waitTime += GenerateWaitTime(GetUnit(), FifthPortalWPs[i].x, FifthPortalWPs[i - 1].x, FifthPortalWPs[i].y, FifthPortalWPs[i - 1].y);

                                pAttacker->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, FifthPortalWPs[i]));
                            }
                        }break;
                        case 2: // middle
                        {
                            for (uint8_t i = 0; i < MaxThirdPortalWPS; i++)
                            {
                                uint32_t waitTime = 1000;
                                if (i == 0)
                                    waitTime += GenerateWaitTime(GetUnit(), ThirdPortalWPs[i].x, GetUnit()->GetPositionX(), ThirdPortalWPs[i].y, GetUnit()->GetPositionY());
                                else
                                    waitTime += GenerateWaitTime(GetUnit(), ThirdPortalWPs[i].x, ThirdPortalWPs[i - 1].x, ThirdPortalWPs[i].y, ThirdPortalWPs[i - 1].y);

                                pAttacker->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, ThirdPortalWPs[i]));
                            }
                        }break;
                        default:
                            break;
                    }
                }
                else
                {
                    printf("NO INTRO ATTACKER\n");
                }
            }
        }

        int8_t portalId;
};

class VHAttackerAI : public CreatureAIScript
{
    InstanceScript* pInstance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new VHAttackerAI(c); }
        VHAttackerAI(Creature* pCreature) : CreatureAIScript(pCreature), isMoveSet(false)
        {
            pInstance = pCreature->GetMapMgr()->GetScript();
        }

        void OnLoad()
        {
            RegisterAIUpdateEvent(2000);
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/)
        {
            if (iWaypointId + 1 <= GetUnit()->GetAIInterface()->getWayPointsCount())
            {
                GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                GetUnit()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
            }

            if (pInstance->GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress && iWaypointId == GetUnit()->GetAIInterface()->getWayPointsCount() - 1)
            {
                if (Creature* pTriggerTarget = getNearestCreature(1823.696045f, 803.604858f, 44.895786f, CN_DOOR_SEAL))
                {
                    GetUnit()->GetAIInterface()->setFacing(M_PI_FLOAT);
                    GetUnit()->SetChannelSpellId(SPELL_VH_DESTROY_DOOR_SEAL);
                    GetUnit()->SetChannelSpellTargetGUID(pTriggerTarget->GetGUID());
                    RegisterAIUpdateEvent(6000);
                    //GetUnit()->CastSpellAoF(pTriggerTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_DESTROY_DOOR_SEAL), false);
                }
                else
                {
                    printf("NO SEAL TRIGGER\n");
                }

            }
        }

        void OnCombatStop(Unit* /*pEnemy*/)
        {
            if (GetUnit()->isAlive())
            {
                GetUnit()->GetAIInterface()->setCurrentAgent(AGENT_NULL);
                GetUnit()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                GetUnit()->GetAIInterface()->StopMovement(0);
                // Move to last waypoint id
                GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                GetUnit()->GetAIInterface()->setWayPointToMove(GetUnit()->GetAIInterface()->getCurrentWayPointId() != 0 ? GetUnit()->GetAIInterface()->getCurrentWayPointId() : 1);
            }
        }

        void OnDied(Unit* /*pKiller*/)
        {
            // Stop channelling (this can happen if player will use crystal)
            GetUnit()->SetChannelSpellId(0);
            GetUnit()->SetChannelSpellTargetGUID(0);
            RemoveAIUpdateEvent();
            despawn();
        }

        void OnCombatStart(Unit* /*pKiller*/)
        {
            // Stop channeling and fight...
            GetUnit()->SetChannelSpellId(0);
            GetUnit()->SetChannelSpellTargetGUID(0);
            RemoveAIUpdateEvent();  // Remove pervious event timer
            RegisterAIUpdateEvent(1000);
        }

        void AIUpdate()
        {
            if (GetUnit()->GetAIInterface()->getCurrentWayPointId() < GetUnit()->GetAIInterface()->getWayPointsCount() && !isMoveSet)
            {
                GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                GetUnit()->GetAIInterface()->setWayPointToMove(1);
                isMoveSet = true;
                RemoveAIUpdateEvent();
            }
        }

    protected:
        Creature* pTriggerTarget;
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
            pCreature->SetScale(5.0f);  // Temp, this will moved to db
            GetUnit()->CastSpell(GetUnit(), SPELL_VH_DEFENSE_SYSTEM_VISUAL, true);
            despawn(4000);
        }

        void AIUpdate()
        {

            if (TheVioletHoldInstance* pInstance = static_cast<TheVioletHoldInstance*>(GetUnit()->GetMapMgr()->GetScript()))
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
                                // HACK
                                GetUnit()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
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
                                //HACK
                                GetUnit()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
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

                // Defense triggers (ONLY animation)
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

                // Damage guardians too
                if (pInstance->m_activePortal.guardianEntry != 0)
                {
                    Creature* pCreature = pInstance->GetInstance()->GetCreature(pInstance->m_portalGuardianGUID);
                    if (pCreature && pCreature->isAlive())
                    {
                        if (counter == 3)
                        {
                            GetUnit()->CastSpellAoF(pCreature->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                            //pCreature->Die(pCreature, pCreature->GetHealth(), 0);
                        }
                        else
                        {
                            GetUnit()->CastSpellAoF(pCreature->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                        }
                    }
                }

                if (counter >= 3)
                {
                    GetUnit()->CastSpell(GetUnit(), SPELL_VH_DEFENSE_SYSTEM_SPAWN, true);
                }
                ++counter;
            }
        }
};

class TeleportationPortalAI : public CreatureAIScript
{
        bool isGuardianSpawned;
        TheVioletHoldInstance* pInstance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new TeleportationPortalAI(c); }
        TeleportationPortalAI(Creature* pCreature) : CreatureAIScript(pCreature), isGuardianSpawned(false)
        {
            pInstance = static_cast<TheVioletHoldInstance*>(GetUnit()->GetMapMgr()->GetScript());
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
                        Creature* pGuardian = spawnCreature(pInstance->m_activePortal.guardianEntry, GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), landHeight, GetUnit()->GetOrientation());
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
                                AddWaypoint(pSummon, pInstance->m_activePortal.id, 0);

                            }
                        }
                    }
                }break;
                case VH_PORTAL_TYPE_SQUAD:
                {
                    GetUnit()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, SQUAD_ANNOUNCE);
                    float landHeight = GetUnit()->GetMapMgr()->GetLandHeight(GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), GetUnit()->GetPositionZ());
                    //TODO: This count needs to be corrected
                    for(uint8 i = 0; i < 5; i++)
                    {
                        Creature* pSummon = spawnCreature(portalGuardians[RandomUInt(maxPortalGuardians - 1)], GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), landHeight, GetUnit()->GetOrientation());
                        if (pSummon)
                        {
                            //TODO: replace this with cleaner solution
                            pInstance->m_activePortal.summonsList.push_back(GET_LOWGUID_PART(pSummon->GetGUID()));
                            pInstance->m_eventSpawns.push_back(GET_LOWGUID_PART(pSummon->GetGUID()));
                            pInstance->m_eventSpawns.push_back(GET_LOWGUID_PART(pSummon->GetGUID()));
                            AddWaypoint(pSummon, pInstance->m_activePortal.id, 0);
                        }
                    }
                    RemoveAIUpdateEvent();
                    despawn(1000, 0);
                }break;
                case VH_PORTAL_TYPE_BOSS:
                {
                    float landHeight = GetUnit()->GetMapMgr()->GetLandHeight(GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), GetUnit()->GetPositionZ());
                    Creature* pSaboteur = spawnCreature(CN_AZURE_SABOTEUR, GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), landHeight, GetUnit()->GetOrientation());
                    if (pSaboteur)
                    {
                        AddWaypoint(pSaboteur, 0, pInstance->m_activePortal.bossEntry);
                        pSaboteur->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        pSaboteur->GetAIInterface()->setWayPointToMove(1);
                    }
                    RemoveAIUpdateEvent();
                    despawn(1000, 0);
                }break;
            }
            pInstance->SetInstanceData(0, DATA_ARE_SUMMONS_MADE, 1);
        }

        void AddWaypoint(Creature* pCreature, uint8_t portalId, uint32_t bossEntry)
        {
            if (!pCreature)
                return;

            // not a boss encounter
            if (bossEntry == 0)
            {
                switch (portalId)
                {
                    case 0: // Guardian portal near Erekem
                    {
                        for (uint8_t i = 0; i < MaxFirstPortalWPS; i++)
                        {
                            uint32_t waitTime = 1000;
                            if (i == 0)
                                waitTime += GenerateWaitTime(pCreature, FirstPortalWPs[i].x, pCreature->GetPositionX(), FirstPortalWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(pCreature, FirstPortalWPs[i].x, FirstPortalWPs[i - 1].x, FirstPortalWPs[i].y, FirstPortalWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, FirstPortalWPs[i]));
                        }
                    }break;
                    case 7: // Squad group near Zuramat
                    {
                        if (RandomUInt(1))
                        {
                            for (uint8_t i = 0; i < MaxSecondPortalLeftWPS; i++)
                            {
                                uint32_t waitTime = 1000;
                                if (i == 0)
                                    waitTime += GenerateWaitTime(pCreature, SecondPortalFirstWPs[i].x, pCreature->GetPositionX(), SecondPortalFirstWPs[i].y, pCreature->GetPositionY());
                                else
                                    waitTime += GenerateWaitTime(pCreature, SecondPortalFirstWPs[i].x, SecondPortalFirstWPs[i - 1].x, SecondPortalFirstWPs[i].y, SecondPortalFirstWPs[i - 1].y);

                                pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SecondPortalFirstWPs[i]));
                            }
                        }
                        else
                        {
                            for (uint8_t i = 0; i < MaxSecondPortalRightWPS; i++)
                            {
                                uint32_t waitTime = 1000;
                                if (i == 0)
                                    waitTime += GenerateWaitTime(pCreature, SecondPortalSecondWPs[i].x, pCreature->GetPositionX(), SecondPortalSecondWPs[i].y, pCreature->GetPositionY());
                                else
                                    waitTime += GenerateWaitTime(pCreature, SecondPortalSecondWPs[i].x, SecondPortalSecondWPs[i - 1].x, SecondPortalSecondWPs[i].y, SecondPortalSecondWPs[i - 1].y);

                                pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SecondPortalSecondWPs[i]));
                            }
                        }

                    }break;
                    case 2: // top edge
                    {
                        for (uint8_t i = 0; i < MaxThirdPortalWPS; i++)
                        {
                            uint32_t waitTime = 1000;
                            if (i == 0)
                                waitTime += GenerateWaitTime(pCreature, ThirdPortalWPs[i].x, pCreature->GetPositionX(), ThirdPortalWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(pCreature, ThirdPortalWPs[i].x, ThirdPortalWPs[i - 1].x, ThirdPortalWPs[i].y, ThirdPortalWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, ThirdPortalWPs[i]));
                        }
                    }break;
                    case 6: // near moragg squad location
                    {
                        for (uint8_t i = 0; i < MaxFourthPortalWPS; i++)
                        {
                            uint32_t waitTime = 1000;
                            if (i == 0)
                                waitTime += GenerateWaitTime(pCreature, FourthPortalWPs[i].x, pCreature->GetPositionX(), FourthPortalWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(pCreature, FourthPortalWPs[i].x, FourthPortalWPs[i - 1].x, FourthPortalWPs[i].y, FourthPortalWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, FourthPortalWPs[i]));
                        }
                    }break;
                    case 4: // morag loc
                    {
                        for (uint8_t i = 0; i < MaxFifthPortalWPS; i++)
                        {
                            uint32_t waitTime = 1000;
                            if (i == 0)
                                waitTime += GenerateWaitTime(pCreature, FifthPortalWPs[i].x, pCreature->GetPositionX(), FifthPortalWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(pCreature, FifthPortalWPs[i].x, FifthPortalWPs[i - 1].x, FifthPortalWPs[i].y, FifthPortalWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, FifthPortalWPs[i]));
                        }
                    }break;
                    case 5:
                    {
                        for (uint8_t i = 0; i < MaxSixhtPortalWPS; i++)
                        {
                            uint32_t waitTime = 1000;
                            if (i == 0)
                                waitTime += GenerateWaitTime(pCreature, SixthPoralWPs[i].x, pCreature->GetPositionX(), SixthPoralWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(pCreature, SixthPoralWPs[i].x, SixthPoralWPs[i - 1].x, SixthPoralWPs[i].y, SixthPoralWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SixthPoralWPs[i]));
                        }
                    }break;
                    default:
                    {
                        uint32_t waitTime = 1000 + GenerateWaitTime(pCreature, DefaultPortalWPs.x, pCreature->GetPositionX(), DefaultPortalWPs.y, pCreature->GetPositionY());
                        pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(1, waitTime, DefaultPortalWPs));
                        // Unused waypoint
                        Movement::Location emptyLoc = { 0, 0, 0, 0 };
                        pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(2, waitTime, emptyLoc));

                    }break;
                }
            }
            else
            {
                switch (bossEntry)
                {
                    case CN_MORAGG:
                    {
                        for (uint8_t i = 0; i < MaxWpToMoragg; i++)
                        {
                            uint32_t waitTime = 1000;
                            // First wp
                            if (i == 0)
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurMoraggPath[i].x, GetUnit()->GetPositionX(), SaboteurMoraggPath[i].y, GetUnit()->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurMoraggPath[i].x, SaboteurMoraggPath[i - 1].x, SaboteurMoraggPath[i].y, SaboteurMoraggPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SaboteurMoraggPath[i]));
                        }
                    }break;
                    case CN_ICHORON:
                    {
                        for (uint8_t i = 0; i < MaxWpToIchonor; i++)
                        {
                            uint32_t waitTime = 1000;
                            // First wp
                            if (i == 0)
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurIchoronPath[i].x, GetUnit()->GetPositionX(), SaboteurIchoronPath[i].y, GetUnit()->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurIchoronPath[i].x, SaboteurIchoronPath[i - 1].x, SaboteurIchoronPath[i].y, SaboteurIchoronPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SaboteurIchoronPath[i]));
                        }
                    }break;
                    case CN_XEVOZZ:
                    {
                        for (uint8_t i = 0; i < MaxWpToXevozz; i++)
                        {
                            uint32_t waitTime = 1000;
                            // First wp
                            if (i == 0)
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurXevozzPath[i].x, GetUnit()->GetPositionX(), SaboteurXevozzPath[i].y, GetUnit()->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurXevozzPath[i].x, SaboteurXevozzPath[i - 1].x, SaboteurXevozzPath[i].y, SaboteurXevozzPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SaboteurXevozzPath[i]));
                        }
                    }break;
                    case CN_LAVANTHOR:
                    {
                        for (uint8_t i = 0; i < MaxWpToLavanthor; i++)
                        {
                            uint32_t waitTime = 1000;
                            // First wp
                            if (i == 0)
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurLavanthorPath[i].x, GetUnit()->GetPositionX(), SaboteurLavanthorPath[i].y, GetUnit()->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurLavanthorPath[i].x, SaboteurLavanthorPath[i - 1].x, SaboteurLavanthorPath[i].y, SaboteurLavanthorPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SaboteurLavanthorPath[i]));
                        }
                    }break;
                    case CN_EREKEM:
                    {
                        for (uint8_t i = 0; i < MaxWpToErekem; i++)
                        {
                            uint32_t waitTime = 1000;
                            // First wp
                            if (i == 0)
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurErekemPath[i].x, GetUnit()->GetPositionX(), SaboteurErekemPath[i].y, GetUnit()->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurErekemPath[i].x, SaboteurErekemPath[i - 1].x, SaboteurErekemPath[i].y, SaboteurErekemPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SaboteurErekemPath[i]));
                        }
                    }break;
                    case CN_ZURAMAT:
                    {
                        for (uint8_t i = 0; i < MaxWpToZuramat; i++)
                        {
                            uint32_t waitTime = 1000;
                            // First wp
                            if (i == 0)
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurZuramatPath[i].x, GetUnit()->GetPositionX(), SaboteurZuramatPath[i].y, GetUnit()->GetPositionY());
                            else
                                waitTime += GenerateWaitTime(GetUnit(), SaboteurZuramatPath[i].x, SaboteurZuramatPath[i - 1].x, SaboteurZuramatPath[i].y, SaboteurZuramatPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, SaboteurZuramatPath[i]));
                        }
                    }break;
                    default:
                    {
                        LOG_ERROR("Violet Hold: Unhandled boss entry %u used for Saboteur", bossEntry);
                    }break;
                }
            }
        }
};

class AzureSaboteurAI : public CreatureAIScript
{
    TheVioletHoldInstance* pInstance;
    uint8_t mStep;
    public:

        static CreatureAIScript* Create(Creature* c) { return new AzureSaboteurAI(c); }
        AzureSaboteurAI(Creature* pCreature) : CreatureAIScript(pCreature), mStep(0)
        {
            pInstance = static_cast<TheVioletHoldInstance* >(pCreature->GetMapMgr()->GetScript());
            setCanEnterCombat(false);
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/)
        {
            if (pInstance)
            {
                switch (pInstance->m_activePortal.bossEntry)
                {
                case CN_MORAGG:
                {
                    if (iWaypointId == (MaxWpToMoragg - 1))
                    {
                        GetUnit()->SetFacing(4.689994f);
                        RegisterAIUpdateEvent(1000);
                    }
                    else
                    {
                        GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        GetUnit()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                    }
                }break;
                case CN_ICHORON:
                {
                    if (iWaypointId == (MaxWpToIchonor - 1))
                    {
                        GetUnit()->SetFacing(5.411396f);
                        RegisterAIUpdateEvent(1000);
                    }
                    else
                    {
                        GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        GetUnit()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                    }
                }break;
                case CN_XEVOZZ:
                {
                    if (iWaypointId == (MaxWpToXevozz - 1))
                    {
                        GetUnit()->SetFacing(1.060255f);
                        RegisterAIUpdateEvent(1000);
                    }
                    else
                    {
                        GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        GetUnit()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                    }
                }break;
                case CN_LAVANTHOR:
                {
                    if (iWaypointId == (MaxWpToLavanthor - 1))
                    {
                        GetUnit()->SetFacing(4.025167f);
                        RegisterAIUpdateEvent(1000);
                    }
                    else
                    {
                        GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        GetUnit()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                    }
                }break;
                case CN_EREKEM:
                {
                    if (iWaypointId == (MaxWpToErekem - 1))
                    {
                        GetUnit()->SetFacing(1.955642f);
                        RegisterAIUpdateEvent(1000);
                    }
                    else
                    {
                        GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        GetUnit()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                    }
                }break;
                case CN_ZURAMAT:
                {
                    if (iWaypointId == (MaxWpToZuramat - 1))
                    {
                        GetUnit()->SetFacing(0.942478f);
                        RegisterAIUpdateEvent(1000);
                    }
                    else
                    {
                        GetUnit()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        GetUnit()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                    }
                }break;
                }
            }
        }

        void AIUpdate()
        {
            switch (mStep)
            {
                // Do same action 3 times
                case 0:
                case 1:
                case 2:
                {
                    GetUnit()->CastSpell(GetUnit(), SPELL_VH_SHIELD_DISRUPTION, true);
                }break;
                case 3:
                {
                    GetUnit()->CastSpell(GetUnit(), SPELL_VH_SIMPLE_TELEPORT, true);
                    despawn(1000, 0);

                    // Let instance prepare boss event
                    if (pInstance)
                    {
                        switch(pInstance->m_activePortal.bossEntry)
                        {
                            case CN_MORAGG:
                            {
                                pInstance->SetInstanceData(0, INDEX_MORAGG, State_PreProgress);
                            }break;
                            case CN_ICHORON:
                            {
                                pInstance->SetInstanceData(0, INDEX_ICHONOR, State_PreProgress);
                            }break;
                            case CN_XEVOZZ:
                            {
                                pInstance->SetInstanceData(0, INDEX_XEVOZZ, State_PreProgress);
                            }break;
                            case CN_LAVANTHOR:
                            {
                                pInstance->SetInstanceData(0, INDEX_LAVANTHOR, State_PreProgress);
                            }break;
                            case CN_EREKEM:
                            {
                                pInstance->SetInstanceData(0, INDEX_EREKEM, State_PreProgress);
                            }break;
                            case CN_ZURAMAT:
                            {
                                pInstance->SetInstanceData(0, INDEX_ZURAMAT, State_PreProgress);
                            }break;
                        }
                    }
                }break;
                default:
                    break;
            }
            ++mStep;
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

TheVioletHoldInstance::TheVioletHoldInstance(MapMgr* pMapMgr) :
    InstanceScript(pMapMgr),
    m_isZuramatAchievFailed(false),
    m_isDefAchievFailed(false),
    m_mainGatesGUID(0),
    m_sinclariGUID(0),
    m_portalSummonTimer(0),
    m_portalGUID(0),
    m_portalGuardianGUID(0),
    emote75pct(false),
    emote50pct(false),
    emote5pct(false)
{
    ResetInstanceData();
    pMapMgr->pInstance = sInstanceMgr.GetInstanceByIds(MAP_VIOLET_HOLD, pMapMgr->GetInstanceID());
    generateBossDataState();
}

void TheVioletHoldInstance::SetCellForcedStates(float pMinX, float pMaxX, float pMinY, float pMaxY, bool pActivate)
{
    if (pMinX == pMaxX || pMinY == pMaxY)
        return;

    float Y = pMinY;
    while (pMinX < pMaxX)
    {
        while (pMinY < pMaxY)
        {
            MapCell* CurrentCell = mInstance->GetCellByCoords(pMinX, pMinY);
            if (pActivate && CurrentCell == nullptr)
            {
                CurrentCell = mInstance->CreateByCoords(pMinX, pMinY);
                if (CurrentCell != nullptr)
                    CurrentCell->Init(mInstance->GetPosX(pMinX), mInstance->GetPosY(pMinY), mInstance);
            }

            if (CurrentCell != nullptr)
            {
                if (pActivate)
                {
                    mInstance->AddForcedCell(CurrentCell);
                }
                else
                    mInstance->RemoveForcedCell(CurrentCell);
            }

            pMinY += 40.0f;
        }

        pMinY = Y;
        pMinX += 40.0f;
    }
}

//TODO: this should be redone by checking actual saved data for heroic mode
void TheVioletHoldInstance::ResetInstanceData()
{
    memset(m_VHencounterData, State_NotStarted, sizeof(m_VHencounterData));
    m_activePortal.ResetData();
    m_activePortal.summonsList.clear(); // listed spawns are already removed by using m_eventSpawns container
}

void TheVioletHoldInstance::SetInstanceData(uint32_t /*pType*/, uint32_t pIndex, uint32_t pData)
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
            if (GameObject* pGO = GetInstance()->GetGameObject(m_mainGatesGUID))
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
            // Set HP
            SetInstanceData(0, DATA_SEAL_HEALTH, 100);
        }

        if (pData == State_InProgress)
        {
            m_portalSummonTimer = VH_INITIAL_PORTAL_TIME;
        }

        if (pData == State_Failed)
        {
            ResetIntro();
            ResetInstanceData();
        }

        if (pData == State_Finished)
        {
            // Open gates
            GameObject* pGates = GetInstance()->GetGameObject(m_mainGatesGUID);
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
            if (Creature* pPortal = GetInstance()->GetCreature(m_portalGUID))
            {
                pPortal->Despawn(1000, 0);
            }
            m_portalGUID = 0;

            // Lets reset event
            SetInstanceData(0, INDEX_PORTAL_PROGRESS, State_NotStarted);
            SetInstanceData(0, DATA_ARE_SUMMONS_MADE, 0);
            m_activePortal.ResetData();
        }
    }break;
    case DATA_SEAL_HEALTH:
    {
        if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress)
        {
            if (pData <= 75 && !emote75pct)
            {
                if (Creature* pCreature = GetInstance()->GetCreature(m_sinclariGUID))
                {
                    pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SEAL_PCT_75);
                }
                emote75pct = true;
            }

            if (pData <= 50 && !emote50pct)
            {
                if (Creature* pCreature = GetInstance()->GetCreature(m_sinclariGUID))
                {
                    pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SEAL_PCT_50);
                }
                emote50pct = true;
            }

            if (pData <= 5 && !emote5pct)
            {
                if (Creature* pCreature = GetInstance()->GetCreature(m_sinclariGUID))
                {
                    pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SEAL_PCT_5);
                }
                emote5pct = true;
            }

            if (pData == 0)
            {
                SetInstanceData(0, INDEX_INSTANCE_PROGRESS, State_Failed);
            }
            UpdateWorldStates();
        }
    }break;
    default:
        break;
    }
}

uint32_t TheVioletHoldInstance::GetInstanceData(uint32_t /*pType*/, uint32_t pIndex)
{
    if (pIndex >= INDEX_MAX)
    {
        return 0;
    }

    return m_VHencounterData[pIndex];
}

void TheVioletHoldInstance::OnLoad()
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
    SetCellForcedStates(1700.0f, 2100.0f, 500.0f, 1000.0f, true);
    RegisterUpdateEvent(VH_UPDATE_TIMER);
}

void TheVioletHoldInstance::OnGameObjectPushToWorld(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
    case GO_PRISON_SEAL:
    {
        m_mainGatesGUID = pGo->GetLowGUID();
    }break;
    case GO_ACTIVATION_CRYSTAL:
    {
        m_crystalGuids.push_back(pGo->GetLowGUID());
    }break;
    case GO_XEVOZZ_CELL:
    {
        m_XevozzCellGUID = pGo->GetLowGUID();
    }break;
    case GO_LAVANTHOR_CELL:
    {
        m_LavanthorCellGUID = pGo->GetLowGUID();
    }break;
    case GO_ICHORON_CELL:
    {
        m_IchnonorCellGUID = pGo->GetLowGUID();
    }break;
    case GO_ZURAMAT_CELL:
    {
        m_ZuramatCellGUID = pGo->GetLowGUID();
    }break;
    case GO_EREKEM_CELL:
    {
        m_ErekemCellGUID = pGo->GetLowGUID();
    }break;
    case GO_EREKEM_GUARD_CELL1:
    {
        m_ErekemGuardCellGUID[0] = pGo->GetLowGUID();
    }break;
    case GO_EREKEM_GUARD_CELL2:
    {
        m_ErekemGuardCellGUID[1] = pGo->GetLowGUID();
    }break;
    case GO_MORAGG_DOOR:
    {
        m_MorrogCellGUID = pGo->GetLowGUID();
    }break;
    default:
        break;
    }
}

void TheVioletHoldInstance::OnGameObjectActivate(GameObject* pGo, Player* plr)
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
        DoCrystalActivation();
    }
}


/*void TheVioletHoldInstance::SaveInstanceData(uint32_t spawnId)
{
    if (GetInstance()->iInstanceMode != MODE_HEROIC)
        return;

    if (GetInstance()->pInstance->m_killedNpcs.find(spawnId) == GetInstance()->pInstance->m_killedNpcs.end())
    {
        GetSavedInstance()->m_killedNpcs.insert(spawnId);
        GetSavedInstance()->SaveToDB();
    }


Instance* GetSavedInstance()
{
    return GetInstance()->pInstance;
}
*/

void TheVioletHoldInstance::OnCreaturePushToWorld(Creature* pCreature)
{
    // Make sure all spawned npcs are in phase 1
    pCreature->Phase(PHASE_SET, 1);

    switch (pCreature->GetEntry())
    {
    case CN_DOOR_SEAL:
    {
        // HACKY INVISIBLE
        // invisible display id
        // this is required to make visual effect
        if (pCreature->GetDisplayId() != 11686)
            pCreature->SetDisplayId(11686);
    }break;
    case CN_VIOLET_HOLD_GUARD:
    {
        m_guardsGuids.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
        pCreature->setByteFlag(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLUS_MOB | UNIT_FLAG_UNKNOWN_16);
        pCreature->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ENABLE_POWER_REGEN);
        pCreature->setUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
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
    }break;
    case CN_PORTAL:
    {
        m_portalGUID = GET_LOWGUID_PART(pCreature->GetGUID());
        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
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
    case CN_PORTAL_GUARDIAN:
    case CN_PORTAL_KEEPER:
    {
        m_portalGuardianGUID = GET_LOWGUID_PART(pCreature->GetGUID());
    }break;
    case CN_MORAGG:
    case CN_ICHORON:
    case CN_XEVOZZ:
    case CN_LAVANTHOR:
    case CN_ZURAMAT:
    default:
        break;
    }
}

void TheVioletHoldInstance::OnCreatureDeath(Creature* pCreature, Unit* pKiller)
{
    switch (pCreature->GetEntry())
    {
    case CN_ZURAMAT:
    {
        if (!m_isZuramatAchievFailed)
        {
            UpdateAchievCriteriaForPlayers(ACHIEV_CRIT_VOID_DANCE, 1);
        }
        setData(pCreature->GetEntry(), Finished);
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
        //TODO: replace this with basic despawn
        if (pCreature->IsInWorld())
        {
            pCreature->RemoveFromWorld(true);
        }
    }break;
    case CN_CYANIGOSA:
    {
        if (!m_isDefAchievFailed)
        {
            UpdateAchievCriteriaForPlayers(ACHIEV_CRIT_DEFENSELES, 1);
        }
        setData(pCreature->GetEntry(), Finished);
        // SaveInstanceData(pCreature->GetSQL_id());
    }break;
    case CN_VIOLET_HOLD_GUARD:
    {
        pCreature->Despawn(0, 1000);
    }break;
    case CN_MORAGG:
    case CN_ICHORON:
    case CN_XEVOZZ:
    case CN_LAVANTHOR:
    {
        setData(pCreature->GetEntry(), Finished);
        //SaveInstanceData(pCreature->GetSQL_id());
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
            m_activePortal.DelSummonDataByGuid(GET_LOWGUID_PART(pCreature->GetGUID()));
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
        m_portalGuardianGUID = 0;
    }break;
    default:
    {
        LOG_ERROR("UNHANDLED CREATURE %u", pCreature->GetEntry());

    }break;
    }
}

void TheVioletHoldInstance::OnPlayerEnter(Player* plr)
{
    UpdateWorldStates();
}

void TheVioletHoldInstance::UpdateEvent()
{
    if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) != State_InProgress || GetInstanceData(0, INDEX_INSTANCE_PROGRESS) != State_Finished)
    {

        //RemoveIntroNpcs(true);
        //UpdateGuards();

    }
    if (GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress)
    {
        if (GetInstanceData(0, INDEX_PORTAL_PROGRESS) == State_NotStarted)
        {
            if (m_portalSummonTimer == 0)
            {
                m_portalSummonTimer = VH_NEXT_PORTAL_SPAWN_TIME;
                SpawnPortal();
                SetInstanceData(0, INDEX_PORTAL_PROGRESS, State_InProgress);
            }
            else
                --m_portalSummonTimer;
        }
    }

    //HACK: Erase non existing summons from lists, they should be removed OnDied events
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

    if (!m_eventSpawns.empty())
    {
        for (std::list<uint32_t>::iterator itr = m_eventSpawns.begin(); itr != m_eventSpawns.end();)
        {
            Creature* pSummon = GetInstance()->GetCreature(*itr);
            if (!pSummon || !pSummon->isAlive())
            {
                // Let players get their skinning loots
                pSummon->Despawn(2 * 60 * 1000, 0); // 2 mins
                itr = m_eventSpawns.erase(itr);
                continue;
            }
            ++itr;
        }
    }
}

// Generate very basic portal info
void TheVioletHoldInstance::GenerateRandomPortal(VHPortalInfo & newPortal)
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

        // First boss
        if ((currentPortalCount + 1) == 6)
            SetInstanceData(0, DATA_GROUP1_BOSS_ENTRY, newPortal.bossEntry);

        // Second boss
        if ((currentPortalCount + 1) == 12)
            SetInstanceData(0, DATA_GROUP2_BOSS_ENTRY, newPortal.bossEntry);
    }
}

// SpawnPortal
void TheVioletHoldInstance::SpawnPortal()
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

void TheVioletHoldInstance::DoCrystalActivation()
{
    spawnCreature(CN_DEFENSE_SYSTEM, DefenseSystemLocation.x, DefenseSystemLocation.y, DefenseSystemLocation.z, DefenseSystemLocation.o);
}

void TheVioletHoldInstance::ResetIntro()
{
    // Despawn event spawns
    if (!m_eventSpawns.empty())
    {
        for (std::list<uint32_t>::iterator itr = m_eventSpawns.begin(); itr != m_eventSpawns.end(); itr = m_eventSpawns.erase(itr))
        {
            if (Creature* pSummon = GetInstance()->GetCreature(*itr))
            {
                pSummon->Despawn(1000, 0);
            }
        }
    }

    // Despawn last portal guardian
    if (m_portalGuardianGUID != 0)
    {
        if (Creature* pGuardian = GetInstance()->GetCreature(m_portalGuardianGUID))
        {
            pGuardian->Despawn(1000, 0);
            m_portalGuardianGUID = 0;
        }
    }

    // Despawn last portal
    if (m_portalGUID != 0)
    {
        if (Creature* pGuardian = GetInstance()->GetCreature(m_portalGUID))
        {
            pGuardian->Despawn(1000, 0);
            m_portalGUID = 0;
        }
    }

    // Open the gates
    if (m_mainGatesGUID != 0)
    {
        if (GameObject* pGates = GetInstance()->GetGameObject(m_mainGatesGUID))
        {
            pGates->SetState(GO_STATE_OPEN);
        }
    }

    // Return sinclari to spawn location
    // There is possible issue where you can get two sinclaris :D
    if (m_sinclariGUID != 0)
    {
        if (Creature* pCreature = GetInstance()->GetCreature(m_sinclariGUID))
        {
            // Despawn and respawn her in 1 sec (2 seconds)
            pCreature->Despawn(1000, 0);
            spawnCreature(CN_LIEUTNANT_SINCLARI, SinclariSpawnLoc.x, SinclariSpawnLoc.y, SinclariSpawnLoc.z, SinclariSpawnLoc.o);
        }
    }
    else
    {
        // GUID will be set at OnCreaturePushToWorld event
        spawnCreature(CN_LIEUTNANT_SINCLARI, SinclariSpawnLoc.x, SinclariSpawnLoc.y, SinclariSpawnLoc.z, SinclariSpawnLoc.o);
    }
    SpawnIntro();
    ResetCrystals(false);
}

void TheVioletHoldInstance::RemoveIntroNpcs(bool deadOnly)
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

void TheVioletHoldInstance::RemoveIntroNpcByGuid(uint32_t guid)
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

void TheVioletHoldInstance::UpdateWorldStates()
{
    UpdateInstanceWorldState(WORLD_STATE_VH_SHOW, GetInstanceData(0, INDEX_INSTANCE_PROGRESS) == State_InProgress ? 1 : 0);
    UpdateInstanceWorldState(WORLD_STATE_VH_PRISON_STATE, GetInstanceData(0, DATA_SEAL_HEALTH));
    UpdateInstanceWorldState(WORLD_STATE_VH_WAVE_COUNT, GetInstanceData(0, DATA_PORTAL_COUNT));
}

void TheVioletHoldInstance::UpdateInstanceWorldState(uint32_t field, uint32_t value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
    data << field;
    data << value;
    GetInstance()->SendPacketToAllPlayers(&data);
}

void TheVioletHoldInstance::SpawnIntro()
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
        else
        {
            if (IntroPortalAI* pPortal = static_cast<IntroPortalAI*>(pSummon->GetScript()))
            {
                pPortal->portalId = i;
            }
        }
    }
}

void TheVioletHoldInstance::ResetCrystals(bool isSelectable)
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

void TheVioletHoldInstance::UpdateAchievCriteriaForPlayers(uint32_t id, uint32_t criteriaCount)
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
void TheVioletHoldInstance::CallGuardsOut()
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

    // Huge HACK
#ifdef ENABLE_VH_HACKS
void TheVioletHoldInstance::UpdateGuards()
{
    if (m_guardsGuids.empty())
        return;

    for (std::list<uint32_t>::iterator itr = m_guardsGuids.begin(); itr != m_guardsGuids.end(); ++itr)
    {
        if (Creature* pGuard = GetInstance()->GetCreature(*itr))
        {
            // hack fix to get his respawn properly
            if (pGuard->IsDead())
            {
                pGuard->Despawn(1000, 1000);
                pGuard->setDeathState(ALIVE);   // This prevents him from respawn/despawn loop
            }

            // hack fix to set their original facing
            if (pGuard->IsInInstance() && pGuard->isAlive() && !pGuard->getcombatstatus()->IsInCombat() && pGuard->GetAIInterface()->MoveDone())
            {
                if (pGuard->GetOrientation() != pGuard->GetSpawnO())
                    pGuard->SetFacing(pGuard->GetSpawnO());
            }
        }
    }
}
#endif //#ifdef ENABLE_VH_HACKS

void SetupTheVioletHold(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_VIOLET_HOLD, &TheVioletHoldInstance::Create);

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
    mgr->register_creature_script(CN_AZURE_SABOTEUR, &AzureSaboteurAI::Create);

    // Spells
    mgr->register_script_effect(SPELL_VH_TELEPORT_PLAYER, &TeleportPlayerInEffect);
}
