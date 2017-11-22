/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheVioletHold.h"
#include "Spell/SpellAuras.h"
#include "Objects/Faction.h"

// Helper functions

uint32_t GenerateWPWaitTime(float speed, float newX, float currentX, float newY, float currentY)
{
    float distanceX = (newX - currentX) * (newX - currentX);
    float distanceY = (newY - currentY) * (newY - currentY);
    float distance = sqrt(distanceX + distanceY);
    // we dont have flying creatures here, using only walk/run speeds
    return  static_cast<uint32_t>((1000 * std::abs(distance / speed)));
}

/// ESCORT/GOSSIP EVENT
class SinclariAI : public CreatureAIScript
{
    public:

        static CreatureAIScript* Create(Creature* c) { return new SinclariAI(c); }
        SinclariAI(Creature* pCreature) : CreatureAIScript(pCreature), m_Step(0)
        {
            VH_instance = static_cast<TheVioletHoldInstance*>(pCreature->GetMapMgr()->GetScript());
            float runSpeed = pCreature->GetCreatureProperties()->run_speed;
            for (uint8_t i = 0; i < MaxSinclariWps; i++)
            {
                uint32_t waitTime = 0;
                if (i == 0)
                    waitTime = GenerateWPWaitTime(runSpeed, SinclariWps[i].x, pCreature->GetPositionX(), SinclariWps[i].y, pCreature->GetPositionY());
                else
                    waitTime = GenerateWPWaitTime(runSpeed, SinclariWps[i].x, FirstPortalWPs[i - 1].x, SinclariWps[i].y, SinclariWps[i - 1].y);
                pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_WALK, SinclariWps[i]));
                pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            }
        }

        void StartEvent()
        {
            getCreature()->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(1);
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            switch (iWaypointId)
            {
                // Handle emote and activate crystal (via instance script)
                case 1:
                {
                    getCreature()->EventAddEmote(EMOTE_ONESHOT_USESTANDING, 3000);
                    if (VH_instance)
                        VH_instance->SetInstanceData(INDEX_INSTANCE_PROGRESS, Performed);
                    // After 6 seconds call AIUpdate event (its going to be step 0)
                    RegisterAIUpdateEvent(6000);
                }break;
                // Say goodbye and call AIUpdate event (step 3)
                case 2:
                {
                    getCreature()->SetFacing(SinclariWps[1].o);
                    RegisterAIUpdateEvent(3500);
                    sendDBChatMessage(SAY_SINCLARI_CLOSING_GATES);
                }break;
                // Set instance data, update her unit_npc_flags and update facing
                case 3:
                {
                    getCreature()->SetFacing(SinclariWps[2].o);
                    getCreature()->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    if (VH_instance)
                        VH_instance->SetInstanceData(INDEX_INSTANCE_PROGRESS, InProgress);
                }break;
                // Outro event
                case 4:
                {
                    sendDBChatMessage(SAY_SINCLARI_INSTANCE_DONE);
                }break;
            }
        }

        void AIUpdate() override
        {
            if (VH_instance && VH_instance->GetInstanceData(INDEX_INSTANCE_PROGRESS) != InProgress)
            {
                switch(m_Step)
                {
                    // Face to guards and call them after 1 second
                    case 0:
                    {
                        ModifyAIUpdateEvent(1000);
                        getCreature()->SetFacing(SinclariWps[0].o);
                    }break;
                    // Call guards and after 3 seconds start another waypoint movement
                    case 1:
                    {
                        ModifyAIUpdateEvent(3000);
                        sendDBChatMessage(YELL_SINCLARI_LEAVING);
                        getCreature()->EventAddEmote(EMOTE_ONESHOT_SHOUT, 2000);
                        VH_instance->CallGuardsOut();
                    }break;
                    // Move her outside
                    case 2:
                    {
                        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        getCreature()->GetAIInterface()->setWayPointToMove(2);
                        RemoveAIUpdateEvent();
                    }break;
                    // Emote mimic
                    case 3:
                    {
                        getCreature()->EventAddEmote(EMOTE_ONESHOT_TALK, 2000);
                        ModifyAIUpdateEvent(3000);
                    }break;
                    // Close the gates
                    case 4:
                    {
                        VH_instance->SetInstanceData(INDEX_INSTANCE_PROGRESS, PreProgress);
                        ModifyAIUpdateEvent(1000);
                    }break;
                    // Move her further
                    case 5:
                    {
                        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        getCreature()->GetAIInterface()->setWayPointToMove(3);
                        RemoveAIUpdateEvent();
                    }break;
                    default:
                        break;
                }

                ++m_Step;
            }
        }

    protected:

        uint8_t m_Step;
        TheVioletHoldInstance* VH_instance;
};

class SinclariGossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* pPlayer)
        {
            TheVioletHoldInstance* pInstance = static_cast<TheVioletHoldInstance*>(pObject->GetMapMgr()->GetScript());
            // TODO: correct text id
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 1);
            if (pInstance)
            {
                // If instance is not finished, show escort gossip option
                if (pInstance->GetInstanceData(INDEX_INSTANCE_PROGRESS) == NotStarted)
                {
                    menu.setTextID(13853);
                    menu.AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_SINCLARI_ACTIVATE), 0);
                }
                // Show option to port player to dungeon
                else if (pInstance->GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress)
                {
                    menu.setTextID(14271);
                    menu.AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(GOSSIP_SINCLARI_SEND_ME_IN), 1);
                }
                menu.Send(pPlayer);
            }
        }

        void OnSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
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
            VH_instance = static_cast<TheVioletHoldInstance*>(pCreature->GetMapMgr()->GetScript());
        }

        void OnDespawn() override
        {
            // Make sure guid is removed from list
            if (VH_instance)
            {
                VH_instance->RemoveIntroNpcByGuid(GET_LOWGUID_PART(getCreature()->GetLowGUID()));
            }
        }

        void OnLoad() override
        {
            setRooted(true);
            getCreature()->m_canRegenerateHP = false;
            setCanEnterCombat(false);
            // Register AIUpdate event in 2 sec
            RegisterAIUpdateEvent(2000);
        }

        void AIUpdate() override
        {
            // Summon adds every 15 or 20 seconds
            if (VH_instance && !(VH_instance->GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress || VH_instance->GetInstanceData(INDEX_INSTANCE_PROGRESS) == Performed) && portalId != -1)
            {
                float landHeight = getCreature()->GetMapMgr()->GetLandHeight(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
                ModifyAIUpdateEvent(RandomUInt(1) ? VH_TIMER_SPAWN_INTRO_MOB_MIN : VH_TIMER_SPAWN_INTRO_MOB_MAX);
                if (Creature* pAttacker = spawnCreature(VHIntroMobs[RandomUInt(VHIntroMobCount - 1)], getCreature()->GetPositionX(), getCreature()->GetPositionY(), landHeight, getCreature()->GetOrientation()))
                {
                    float runSpeed = pAttacker->GetCreatureProperties()->run_speed;
                    switch (portalId)
                    {
                        case 0: // left
                        {
                            for (uint8_t i = 0; i < MaxFirstPortalWPS; i++)
                            {
                                uint32_t waitTime = 0;
                                if (i == 0)
                                    waitTime = GenerateWPWaitTime(runSpeed, FirstPortalWPs[i].x, getCreature()->GetPositionX(), FirstPortalWPs[i].y, getCreature()->GetPositionY());
                                else
                                    waitTime = GenerateWPWaitTime(runSpeed, FirstPortalWPs[i].x, FirstPortalWPs[i - 1].x, FirstPortalWPs[i].y, FirstPortalWPs[i - 1].y);

                                pAttacker->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, FirstPortalWPs[i]));
                            }
                        }break;
                        case 1: // right
                        {
                            for (uint8_t i = 0; i < MaxFifthPortalWPS; i++)
                            {
                                uint32_t waitTime = 0;
                                if (i == 0)
                                    waitTime = GenerateWPWaitTime(runSpeed, FifthPortalWPs[i].x, getCreature()->GetPositionX(), FifthPortalWPs[i].y, getCreature()->GetPositionY());
                                else
                                    waitTime = GenerateWPWaitTime(runSpeed, FifthPortalWPs[i].x, FifthPortalWPs[i - 1].x, FifthPortalWPs[i].y, FifthPortalWPs[i - 1].y);

                                pAttacker->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, FifthPortalWPs[i]));
                            }
                        }break;
                        case 2: // middle
                        {
                            for (uint8_t i = 0; i < MaxThirdPortalWPS; i++)
                            {
                                uint32_t waitTime = 0;
                                if (i == 0)
                                    waitTime = GenerateWPWaitTime(runSpeed, ThirdPortalWPs[i].x, getCreature()->GetPositionX(), ThirdPortalWPs[i].y, getCreature()->GetPositionY());
                                else
                                    waitTime = GenerateWPWaitTime(runSpeed, ThirdPortalWPs[i].x, ThirdPortalWPs[i - 1].x, ThirdPortalWPs[i].y, ThirdPortalWPs[i - 1].y);

                                pAttacker->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, ThirdPortalWPs[i]));
                            }
                        }break;
                        default:
                            break;
                    }
                }
            }
        }

        int8_t portalId;
};

class VHAttackerAI : public CreatureAIScript
{
    TheVioletHoldInstance* pInstance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new VHAttackerAI(c); }
        VHAttackerAI(Creature* pCreature) : CreatureAIScript(pCreature), isMoveSet(false)
        {
            pInstance = static_cast<TheVioletHoldInstance*>(pCreature->GetMapMgr()->GetScript());
        }

        void OnLoad() override
        {
            RegisterAIUpdateEvent(1000);
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            if (iWaypointId < getCreature()->GetAIInterface()->getWayPointsCount())
            {
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
            }

            if (pInstance->GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress && pInstance->GetInstanceData(DATA_SEAL_HEALTH) != 0 
            && iWaypointId == getCreature()->GetAIInterface()->getWayPointsCount() - 1)
            {
                if (Creature* pTriggerTarget = getNearestCreature(1823.696045f, 803.604858f, 44.895786f, CN_DOOR_SEAL))
                {
                    getCreature()->GetAIInterface()->setFacing(M_PI_FLOAT);
                    StartChanneling(pTriggerTarget->GetGUID());
                    RegisterAIUpdateEvent(6000);
                    //getCreature()->CastSpellAoF(pTriggerTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_DESTROY_DOOR_SEAL), false);
                }
            }
        }

        void OnCombatStop(Unit* /*pEnemy*/) override
        {
            if (getCreature()->isAlive())
            {
                getCreature()->GetAIInterface()->setCurrentAgent(AGENT_NULL);
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                getCreature()->GetAIInterface()->StopMovement(0);
                // Move to last waypoint id
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(getCreature()->GetAIInterface()->getCurrentWayPointId() != 0 ? getCreature()->GetAIInterface()->getCurrentWayPointId() : 1);
            }
        }

        void StartChanneling(uint64_t triggerGUID)
        {
            // Stop channelling
            if (triggerGUID == 0)
            {
                if (getCreature()->GetChannelSpellId() != 0)
                    getCreature()->SetChannelSpellId(0);

                if (getCreature()->GetChannelSpellTargetGUID() != 0)
                    getCreature()->SetChannelSpellTargetGUID(0);
            }
            else
            {
                getCreature()->SetChannelSpellId(SPELL_VH_DESTROY_DOOR_SEAL);
                getCreature()->SetChannelSpellTargetGUID(triggerGUID);
            }
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            // Stop channelling
            StartChanneling(0);
            getCreature()->Despawn(1000, 0);
        }

        void OnCombatStart(Unit* /*pKiller*/) override
        {
            // Stop channeling and fight...
            StartChanneling(0);
            RemoveAIUpdateEvent();  // Remove pervious event timer
            RegisterAIUpdateEvent(1000);
        }

        void AIUpdate() override
        {
            if (getCreature()->GetAIInterface()->getCurrentWayPointId() < 1 && !isMoveSet)
            {
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(1);
                isMoveSet = true;
                RemoveAIUpdateEvent();
            }

            // TODO: this should be handled by periodic aura
            if (!getCreature()->CombatStatus.IsInCombat() && getCreature()->GetChannelSpellTargetGUID() != 0 && getCreature()->GetChannelSpellId() != 0
            && pInstance && pInstance->GetInstanceData(INDEX_INSTANCE_PROGRESS) && pInstance->GetInstanceData(DATA_SEAL_HEALTH) != 0)
            {
                pInstance->SetInstanceData(DATA_SEAL_HEALTH, pInstance->GetInstanceData(DATA_SEAL_HEALTH) - 1);
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
            despawn(4000);
        }

        void OnLoad() override
        {
            getCreature()->CastSpell(getCreature(), SPELL_VH_DEFENSE_SYSTEM_VISUAL, true);
        }

        void AIUpdate() override
        {

            if (TheVioletHoldInstance* pInstance = static_cast<TheVioletHoldInstance*>(getCreature()->GetMapMgr()->GetScript()))
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
                                getCreature()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                                pTarget->Die(pTarget, pTarget->GetHealth(), 0);
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
                                getCreature()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
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
                                getCreature()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                                pTarget->Die(pTarget, pTarget->GetHealth(), 0);
                            }
                        }
                        else
                        {
                            if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                            {
                                getCreature()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
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
                            getCreature()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
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
                            getCreature()->CastSpellAoF(pCreature->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                            //pCreature->Die(pCreature, pCreature->GetHealth(), 0);
                        }
                        else
                        {
                            getCreature()->CastSpellAoF(pCreature->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                        }
                    }
                }

                if (counter >= 3)
                {
                    getCreature()->CastSpell(getCreature(), SPELL_VH_DEFENSE_SYSTEM_SPAWN, true);
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
            pInstance = static_cast<TheVioletHoldInstance*>(getCreature()->GetMapMgr()->GetScript());
            if (pInstance)
            {
                if (pInstance->m_activePortal.type != VH_PORTAL_TYPE_BOSS)
                    RegisterAIUpdateEvent(VH_TELE_PORTAL_SPAWN_TIME);
                else
                    RegisterAIUpdateEvent(VH_TELE_PORTAL_BOSS_SPAWN_TIME);
            }
        }

        void AIUpdate() override
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
                    float landHeight = getCreature()->GetMapMgr()->GetLandHeight(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
                    if (!isGuardianSpawned)
                    {
                        getCreature()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, pInstance->m_activePortal.guardianEntry == CN_PORTAL_GUARDIAN ? GUARDIAN_ANNOUNCE : KEEPER_ANNOUNCE);
                        Creature* pGuardian = spawnCreature(pInstance->m_activePortal.guardianEntry, getCreature()->GetPositionX(), getCreature()->GetPositionY(), landHeight, getCreature()->GetOrientation());
                        if (pGuardian)
                        {
                            isGuardianSpawned = true;
                            getCreature()->SetChannelSpellId(SPELL_VH_PORTAL_CHANNEL);
                            getCreature()->SetChannelSpellTargetGUID(pGuardian->GetGUID());
                        }
                    }
                    else
                    {
                        // Spawn 3 random portal guardians
                        for(uint8_t i = 0; i < 3; i++)
                        {
                            if (Creature* pSummon = spawnCreature(portalGuardians[RandomUInt(maxPortalGuardians - 1)], getCreature()->GetPositionX() + RandomFloat(3), getCreature()->GetPositionY() + RandomFloat(3), landHeight, getCreature()->GetOrientation()))
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
                    getCreature()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, SQUAD_ANNOUNCE);
                    float landHeight = getCreature()->GetMapMgr()->GetLandHeight(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
                    //TODO: This count needs to be corrected
                    for(uint8 i = 0; i < 5; i++)
                    {
                        Creature* pSummon = spawnCreature(portalGuardians[RandomUInt(maxPortalGuardians - 1)], getCreature()->GetPositionX(), getCreature()->GetPositionY(), landHeight, getCreature()->GetOrientation());
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
                    float landHeight = getCreature()->GetMapMgr()->GetLandHeight(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
                    Creature* pSaboteur = spawnCreature(CN_AZURE_SABOTEUR, getCreature()->GetPositionX(), getCreature()->GetPositionY(), landHeight, getCreature()->GetOrientation());
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
            pInstance->SetInstanceData(DATA_ARE_SUMMONS_MADE, 1);
        }

        void AddWaypoint(Creature* pCreature, uint8_t portalId, uint32_t bossEntry)
        {
            if (!pCreature)
                return;

            // not a boss encounter
            float runSpeed = pCreature->GetCreatureProperties()->run_speed;
            if (bossEntry == 0)
            {
                switch (portalId)
                {
                    case 0: // Guardian portal near Erekem
                    {
                        for (uint8_t i = 0; i < MaxFirstPortalWPS; i++)
                        {
                            uint32_t waitTime = 0;
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, FirstPortalWPs[i].x, pCreature->GetPositionX(), FirstPortalWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, FirstPortalWPs[i].x, FirstPortalWPs[i - 1].x, FirstPortalWPs[i].y, FirstPortalWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, FirstPortalWPs[i]));
                        }
                    }break;
                    case 7: // Squad group near Zuramat
                    {
                        if (RandomUInt(1))
                        {
                            for (uint8_t i = 0; i < MaxSecondPortalLeftWPS; i++)
                            {
                                uint32_t waitTime = 0;
                                if (i == 0)
                                    waitTime = GenerateWPWaitTime(runSpeed, SecondPortalFirstWPs[i].x, pCreature->GetPositionX(), SecondPortalFirstWPs[i].y, pCreature->GetPositionY());
                                else
                                    waitTime = GenerateWPWaitTime(runSpeed, SecondPortalFirstWPs[i].x, SecondPortalFirstWPs[i - 1].x, SecondPortalFirstWPs[i].y, SecondPortalFirstWPs[i - 1].y);

                                pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SecondPortalFirstWPs[i]));
                            }
                        }
                        else
                        {
                            for (uint8_t i = 0; i < MaxSecondPortalRightWPS; i++)
                            {
                                uint32_t waitTime = 0;
                                if (i == 0)
                                    waitTime = GenerateWPWaitTime(runSpeed, SecondPortalSecondWPs[i].x, pCreature->GetPositionX(), SecondPortalSecondWPs[i].y, pCreature->GetPositionY());
                                else
                                    waitTime = GenerateWPWaitTime(runSpeed, SecondPortalSecondWPs[i].x, SecondPortalSecondWPs[i - 1].x, SecondPortalSecondWPs[i].y, SecondPortalSecondWPs[i - 1].y);

                                pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SecondPortalSecondWPs[i]));
                            }
                        }

                    }break;
                    case 2: // top edge
                    {
                        for (uint8_t i = 0; i < MaxThirdPortalWPS; i++)
                        {
                            uint32_t waitTime = 0;
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, ThirdPortalWPs[i].x, pCreature->GetPositionX(), ThirdPortalWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, ThirdPortalWPs[i].x, ThirdPortalWPs[i - 1].x, ThirdPortalWPs[i].y, ThirdPortalWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, ThirdPortalWPs[i]));
                        }
                    }break;
                    case 6: // near moragg squad location
                    {
                        for (uint8_t i = 0; i < MaxFourthPortalWPS; i++)
                        {
                            uint32_t waitTime = 0;
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, FourthPortalWPs[i].x, pCreature->GetPositionX(), FourthPortalWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, FourthPortalWPs[i].x, FourthPortalWPs[i - 1].x, FourthPortalWPs[i].y, FourthPortalWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, FourthPortalWPs[i]));
                        }
                    }break;
                    case 4: // morag loc
                    {
                        for (uint8_t i = 0; i < MaxFifthPortalWPS; i++)
                        {
                            uint32_t waitTime = 0;
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, FifthPortalWPs[i].x, pCreature->GetPositionX(), FifthPortalWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, FifthPortalWPs[i].x, FifthPortalWPs[i - 1].x, FifthPortalWPs[i].y, FifthPortalWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, FifthPortalWPs[i]));
                        }
                    }break;
                    case 5:
                    {
                        for (uint8_t i = 0; i < MaxSixhtPortalWPS; i++)
                        {
                            uint32_t waitTime = 0;
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, SixthPoralWPs[i].x, pCreature->GetPositionX(), SixthPoralWPs[i].y, pCreature->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, SixthPoralWPs[i].x, SixthPoralWPs[i - 1].x, SixthPoralWPs[i].y, SixthPoralWPs[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SixthPoralWPs[i]));
                        }
                    }break;
                    default:
                    {
                        uint32_t waitTime = 1000 + GenerateWPWaitTime(runSpeed, DefaultPortalWPs.x, pCreature->GetPositionX(), DefaultPortalWPs.y, pCreature->GetPositionY());
                        pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(1, waitTime, Movement::WP_MOVE_TYPE_RUN, DefaultPortalWPs));
                        // Unused waypoint
                        Movement::Location emptyLoc = { 0, 0, 0, 0 };
                        pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(2, waitTime, Movement::WP_MOVE_TYPE_RUN, emptyLoc));

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
                            uint32_t waitTime = 0;
                            // First wp
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurMoraggPath[i].x, getCreature()->GetPositionX(), SaboteurMoraggPath[i].y, getCreature()->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurMoraggPath[i].x, SaboteurMoraggPath[i - 1].x, SaboteurMoraggPath[i].y, SaboteurMoraggPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SaboteurMoraggPath[i]));
                        }
                    }break;
                    case CN_ICHORON:
                    {
                        for (uint8_t i = 0; i < MaxWpToIchonor; i++)
                        {
                            uint32_t waitTime = 0;
                            // First wp
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurIchoronPath[i].x, getCreature()->GetPositionX(), SaboteurIchoronPath[i].y, getCreature()->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurIchoronPath[i].x, SaboteurIchoronPath[i - 1].x, SaboteurIchoronPath[i].y, SaboteurIchoronPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SaboteurIchoronPath[i]));
                        }
                    }break;
                    case CN_XEVOZZ:
                    {
                        for (uint8_t i = 0; i < MaxWpToXevozz; i++)
                        {
                            uint32_t waitTime = 0;
                            // First wp
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurXevozzPath[i].x, getCreature()->GetPositionX(), SaboteurXevozzPath[i].y, getCreature()->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurXevozzPath[i].x, SaboteurXevozzPath[i - 1].x, SaboteurXevozzPath[i].y, SaboteurXevozzPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SaboteurXevozzPath[i]));
                        }
                    }break;
                    case CN_LAVANTHOR:
                    {
                        for (uint8_t i = 0; i < MaxWpToLavanthor; i++)
                        {
                            uint32_t waitTime = 0;
                            // First wp
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurLavanthorPath[i].x, getCreature()->GetPositionX(), SaboteurLavanthorPath[i].y, getCreature()->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurLavanthorPath[i].x, SaboteurLavanthorPath[i - 1].x, SaboteurLavanthorPath[i].y, SaboteurLavanthorPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SaboteurLavanthorPath[i]));
                        }
                    }break;
                    case CN_EREKEM:
                    {
                        for (uint8_t i = 0; i < MaxWpToErekem; i++)
                        {
                            uint32_t waitTime = 0;
                            // First wp
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurErekemPath[i].x, getCreature()->GetPositionX(), SaboteurErekemPath[i].y, getCreature()->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurErekemPath[i].x, SaboteurErekemPath[i - 1].x, SaboteurErekemPath[i].y, SaboteurErekemPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SaboteurErekemPath[i]));
                        }
                    }break;
                    case CN_ZURAMAT:
                    {
                        for (uint8_t i = 0; i < MaxWpToZuramat; i++)
                        {
                            uint32_t waitTime = 0;
                            // First wp
                            if (i == 0)
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurZuramatPath[i].x, getCreature()->GetPositionX(), SaboteurZuramatPath[i].y, getCreature()->GetPositionY());
                            else
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurZuramatPath[i].x, SaboteurZuramatPath[i - 1].x, SaboteurZuramatPath[i].y, SaboteurZuramatPath[i - 1].y);

                            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_RUN, SaboteurZuramatPath[i]));
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

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            if (pInstance)
            {
                switch (pInstance->m_activePortal.bossEntry)
                {
                    case CN_MORAGG:
                    {
                        if (iWaypointId == (MaxWpToMoragg - 1))
                        {
                            getCreature()->SetFacing(4.689994f);
                            RegisterAIUpdateEvent(1000);
                        }
                        else
                        {
                            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                        }
                    }break;
                    case CN_ICHORON:
                    {
                        if (iWaypointId == (MaxWpToIchonor - 1))
                        {
                            getCreature()->SetFacing(5.411396f);
                            RegisterAIUpdateEvent(1000);
                        }
                        else
                        {
                            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                        }
                    }break;
                    case CN_XEVOZZ:
                    {
                        if (iWaypointId == (MaxWpToXevozz - 1))
                        {
                            getCreature()->SetFacing(1.060255f);
                            RegisterAIUpdateEvent(1000);
                        }
                        else
                        {
                            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                        }
                    }break;
                    case CN_LAVANTHOR:
                    {
                        if (iWaypointId == (MaxWpToLavanthor - 1))
                        {
                            getCreature()->SetFacing(4.025167f);
                            RegisterAIUpdateEvent(1000);
                        }
                        else
                        {
                            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                        }
                    }break;
                    case CN_EREKEM:
                    {
                        if (iWaypointId == (MaxWpToErekem - 1))
                        {
                            getCreature()->SetFacing(1.955642f);
                            RegisterAIUpdateEvent(1000);
                        }
                        else
                        {
                            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                        }
                    }break;
                    case CN_ZURAMAT:
                    {
                        if (iWaypointId == (MaxWpToZuramat - 1))
                        {
                            getCreature()->SetFacing(0.942478f);
                            RegisterAIUpdateEvent(1000);
                        }
                        else
                        {
                            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                        }
                    }break;
                }
            }
        }

        void AIUpdate() override
        {
            switch (mStep)
            {
                // Do same action 3 times
                case 0:
                case 1:
                case 2:
                {
                    getCreature()->CastSpell(getCreature(), SPELL_VH_SHIELD_DISRUPTION, true);
                }break;
                case 3:
                {
                    getCreature()->CastSpell(getCreature(), SPELL_VH_SIMPLE_TELEPORT, true);
                    despawn(1000, 0);

                    // Let instance prepare boss event
                    if (pInstance)
                    {
                        switch(pInstance->m_activePortal.bossEntry)
                        {
                            case CN_MORAGG:
                            {
                                pInstance->SetInstanceData(INDEX_MORAGG, PreProgress);
                            }break;
                            case CN_ICHORON:
                            {
                                pInstance->SetInstanceData(INDEX_ICHORON, PreProgress);
                            }break;
                            case CN_XEVOZZ:
                            {
                                pInstance->SetInstanceData(INDEX_XEVOZZ, PreProgress);
                            }break;
                            case CN_LAVANTHOR:
                            {
                                pInstance->SetInstanceData(INDEX_LAVANTHOR, PreProgress);
                            }break;
                            case CN_EREKEM:
                            {
                                pInstance->SetInstanceData(INDEX_EREKEM, PreProgress);
                            }break;
                            case CN_ZURAMAT:
                            {
                                pInstance->SetInstanceData(INDEX_ZURAMAT, PreProgress);
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

// BOSSES

class MoraggAI : public CreatureAIScript
{
    enum MoraggSpells : uint32_t
    {
        SPELL_CORROSIVE_SALIVA  = 54527,
        SPELL_OPTIC_LINK        = 54396,
        SPELL_RAY_OF_PAIN       = 54438,
        SPELL_RAY_OF_PAIN_H     = 59523,
        SPELL_RAY_OF_SUFFERING  = 54442,
        SPELL_RAY_OF_SUFFERING_H = 59524,

        // Visual

        SPELL_OPTIC_LINK_LEVEL_1 = 54393,
        SPELL_OPTIC_LINK_LEVEL_2 = 54394,
        SPELL_OPTIC_LINK_LEVEL_3 = 54395
    };
public:

    static CreatureAIScript* Create(Creature* c) { return new MoraggAI(c); }
    MoraggAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Prepare waypoints
        float walkSpeed = pCreature->GetCreatureProperties()->walk_speed;
        for (uint8_t i = 0; i < MoraggPathSize; i++)
        {
            uint32_t waitTime = 0;
            // First wp
            if (i == 0)
                waitTime = GenerateWPWaitTime(walkSpeed, MoraggPath[i].x, pCreature->GetPositionX(), MoraggPath[i].y, pCreature->GetPositionY());
            else
                waitTime = GenerateWPWaitTime(walkSpeed, MoraggPath[i].x, MoraggPath[i - 1].x, MoraggPath[i].y, MoraggPath[i - 1].y);

            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_WALK, MoraggPath[i]));
        }
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);

        // Prepare spells
        AddSpell(SPELL_CORROSIVE_SALIVA, Target_Current, 8.0f, 0, 6);
        if (pCreature->GetMapMgr()->iInstanceMode == MODE_HEROIC)
        {
            AddSpell(SPELL_RAY_OF_PAIN_H, Target_RandomPlayer, 7.0f, 0, 25);
            AddSpell(SPELL_RAY_OF_SUFFERING_H, Target_RandomPlayer, 7.0f, 0, 5);
        }
        else
        {
            AddSpell(SPELL_RAY_OF_PAIN, Target_RandomPlayer, 7.0f, 0, 25);
            AddSpell(SPELL_RAY_OF_SUFFERING, Target_RandomPlayer, 7.0f, 0, 5);
        }
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == MoraggPathSize - 1)
        {
            // Make him targetable and able to enter to combat
            getCreature()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9 | UNIT_FLAG_NOT_SELECTABLE);
        }
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
    m_ErekemGUID(0),
    m_MoraggGUID(0),
    m_IchoronGUID(0),
    m_XevozzGUID(0),
    m_LavanthorGUID(0),
    m_ZuramatGUID(0),
    m_portalSummonTimer(0),
    m_portalGUID(0),
    m_portalGuardianGUID(0),
    emote75pct(false),
    emote50pct(false),
    emote5pct(false)
{
    ResetInstanceData();
    //pMapMgr->pInstance = sInstanceMgr.GetInstanceByIds(MAP_VIOLET_HOLD, pMapMgr->GetInstanceID());
    generateBossDataState();
}

//TODO: this should be redone by checking actual saved data for heroic mode
void TheVioletHoldInstance::ResetInstanceData()
{
    memset(m_VHencounterData, NotStarted, sizeof(m_VHencounterData));
    m_activePortal.ResetData();
    m_activePortal.summonsList.clear(); // listed spawns are already removed by using m_eventSpawns container
}

void TheVioletHoldInstance::SetInstanceData(uint32_t pIndex, uint32_t pData)
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
            switch (pData)
            {
                case Performed:
                {
                    DoCrystalActivation();
                }break;
                case PreProgress:
                {
                    //RemoveIntroNpcs(false);
                    // Close the gates
                    if (GameObject* pGO = GetInstance()->GetGameObject(m_mainGatesGUID))
                    {
                        pGO->SetState(GO_STATE_CLOSED);
                    }

                    // Update crystals
                    ResetCrystals(true);

                    // Set HP
                    SetInstanceData(DATA_SEAL_HEALTH, 100);
                }break;
                case InProgress:
                {
                    m_portalSummonTimer = VH_INITIAL_PORTAL_TIME;
                }break;
                case State_Failed:
                {
                    ResetIntro();
                    ResetInstanceData();
                }break;
                case Finished:
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
                }break;
            }
        }break;
        case INDEX_PORTAL_PROGRESS:
        {
            if (pData == Finished)
            {
                if (Creature* pPortal = GetInstance()->GetCreature(m_portalGUID))
                {
                    pPortal->Despawn(1000, 0);
                }
                m_portalGUID = 0;

                // Lets reset event
                SetInstanceData(INDEX_PORTAL_PROGRESS, NotStarted);
                SetInstanceData(DATA_ARE_SUMMONS_MADE, 0);
                m_activePortal.ResetData();
            }
        }break;
        case DATA_SEAL_HEALTH:
        {
            if (GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress)
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

                // Seal is destroyed
                if (pData == 0)
                {
                    SetInstanceData(INDEX_INSTANCE_PROGRESS, State_Failed);
                }
                UpdateWorldStates();
            }
        }break;
        case INDEX_MORAGG:
        {
            switch (pData)
            {
                case Performed:
                {
                    ReleaseBoss(m_MorragCellGUID, m_MoraggGUID);
                    SetInstanceData(INDEX_MORAGG, PreProgress);
                }break;
                case State_Failed:
                {
                    if (GameObject* pGates = GetInstance()->GetGameObject(m_MorragCellGUID))
                    {
                        pGates->SetState(GO_STATE_CLOSED);
                    }

                    if (Creature* pMoragg = GetInstance()->GetCreature(m_MoraggGUID))
                    {
                        SpawnGhostlyReplacement(CN_MORAGG, pMoragg->GetSpawnX(), pMoragg->GetSpawnY(), pMoragg->GetSpawnZ(), pMoragg->GetSpawnO());
                        pMoragg->Despawn(1000, 0);
                    }
                }break;
                case Finished:
                {
                    // Start timer for next portal
                    SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
                }break;
            }
        }break;
        case INDEX_ICHORON:
        {
            switch (pData)
            {
                case Performed:
                {
                    ReleaseBoss(m_IchoronCellGUID, m_IchoronGUID);
                    SetInstanceData(INDEX_ICHORON, PreProgress);
                }break;
                case State_Failed:
                {
                    if (GameObject* pGates = GetInstance()->GetGameObject(m_IchoronCellGUID))
                    {
                        pGates->SetState(GO_STATE_CLOSED);
                    }

                    if (Creature* pIchoron = GetInstance()->GetCreature(m_IchoronGUID))
                    {
                        SpawnGhostlyReplacement(CN_ICHORON, pIchoron->GetSpawnX(), pIchoron->GetSpawnY(), pIchoron->GetSpawnZ(), pIchoron->GetSpawnO());
                        pIchoron->Despawn(1000, 0);
                    }
                }break;
                case Finished:
                {
                    // Start timer for next portal
                    SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
                }break;
            }
        }break;
        case INDEX_ZURAMAT:
        {
            switch (pData)
            {
                case Performed:
                {
                    ReleaseBoss(m_ZuramatCellGUID, m_ZuramatGUID);
                    SetInstanceData(INDEX_ZURAMAT, PreProgress);
                }break;
                case State_Failed:
                {
                    if (GameObject* pGates = GetInstance()->GetGameObject(m_ZuramatCellGUID))
                    {
                        pGates->SetState(GO_STATE_CLOSED);
                    }

                    if (Creature* pZuramat = GetInstance()->GetCreature(m_IchoronGUID))
                    {
                        SpawnGhostlyReplacement(CN_ZURAMAT, pZuramat->GetSpawnX(), pZuramat->GetSpawnY(), pZuramat->GetSpawnZ(), pZuramat->GetSpawnO());
                        pZuramat->Despawn(1000, 0);
                    }
                }break;
                case Finished:
                {
                    // Start timer for next portal
                    SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
                }break;
            }
        }break;
        case INDEX_EREKEM:
        {
            switch (pData)
            {
                case Performed:
                {
                    // TODO: add text and add movement to his guards
                    ReleaseBoss(m_ErekemCellGUID, m_ErekemGUID);
                    SetInstanceData(INDEX_ZURAMAT, PreProgress);
                }break;
                case State_Failed:
                {
                    if (GameObject* pGates = GetInstance()->GetGameObject(m_ZuramatCellGUID))
                    {
                        pGates->SetState(GO_STATE_CLOSED);
                    }

                    if (Creature* pErekem = GetInstance()->GetCreature(m_ErekemGUID))
                    {
                        SpawnGhostlyReplacement(CN_ZURAMAT, pErekem->GetSpawnX(), pErekem->GetSpawnY(), pErekem->GetSpawnZ(), pErekem->GetSpawnO());
                        pErekem->Despawn(1000, 0);
                    }
                }break;
                case Finished:
                {
                    // Start timer for next portal
                    SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
                }break;
            }
        }break;
        case INDEX_LAVANTHOR:
        {
            switch (pData)
            {
                case Performed:
                {
                    // TODO: add sound/emote mimic
                    ReleaseBoss(m_LavanthorCellGUID, m_LavanthorGUID);
                    SetInstanceData(INDEX_LAVANTHOR, PreProgress);
                }break;
                case State_Failed:
                {
                    if (GameObject* pGates = GetInstance()->GetGameObject(m_LavanthorCellGUID))
                    {
                        pGates->SetState(GO_STATE_CLOSED);
                    }

                    if (Creature* pLavanthor = GetInstance()->GetCreature(m_IchoronGUID))
                    {
                        SpawnGhostlyReplacement(CN_ZURAMAT, pLavanthor->GetSpawnX(), pLavanthor->GetSpawnY(), pLavanthor->GetSpawnZ(), pLavanthor->GetSpawnO());
                        pLavanthor->Despawn(1000, 0);
                    }
                }break;
                case Finished:
                {
                    // Start timer for next portal
                    SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
                }break;
            }
        }break;
        case INDEX_XEVOZZ:
        {
            switch (pData)
            {
                case Performed:
                {
                    // TODO: add text
                    ReleaseBoss(m_XevozzCellGUID, m_XevozzGUID);
                    SetInstanceData(INDEX_XEVOZZ, PreProgress);
                }break;
                case State_Failed:
                {
                    if (GameObject* pGates = GetInstance()->GetGameObject(m_XevozzCellGUID))
                    {
                        pGates->SetState(GO_STATE_CLOSED);
                    }

                    if (Creature* pXevozz = GetInstance()->GetCreature(m_XevozzGUID))
                    {
                        SpawnGhostlyReplacement(CN_ZURAMAT, pXevozz->GetSpawnX(), pXevozz->GetSpawnY(), pXevozz->GetSpawnZ(), pXevozz->GetSpawnO());
                        pXevozz->Despawn(1000, 0);
                    }
                }break;
                case Finished:
                {
                    // Start timer for next portal
                    SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
                }break;
            }
        }break;
        default:
            break;
    }
}

uint32_t TheVioletHoldInstance::GetInstanceData(uint32_t pIndex)
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
    if (GetInstanceData(INDEX_INSTANCE_PROGRESS) == NotStarted)
    {
        ResetIntro();
    }
    setCellForcedStates(1700.0f, 2100.0f, 500.0f, 1000.0f, true);
    registerUpdateEvent();  // default timer is 1000 ms
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
            m_IchoronCellGUID = pGo->GetLowGUID();
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
            m_MorragCellGUID = pGo->GetLowGUID();
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
        {
            m_MoraggGUID = GET_LOWGUID_PART(pCreature->GetGUID());
        }break;
        case CN_ICHORON:
        {
            m_IchoronGUID = GET_LOWGUID_PART(pCreature->GetGUID());
        }break;
        case CN_XEVOZZ:
        {
            m_XevozzGUID = GET_LOWGUID_PART(pCreature->GetGUID());
        }break;
        case CN_LAVANTHOR:
        {
            m_LavanthorGUID = GET_LOWGUID_PART(pCreature->GetGUID());
        }break;
        case CN_ZURAMAT:
        {
            m_ZuramatGUID = GET_LOWGUID_PART(pCreature->GetGUID());
        }break;
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
            pCreature->Despawn(1000, 0);
        }break;
        case CN_CYANIGOSA:
        {
            if (!m_isDefAchievFailed)
            {
                UpdateAchievCriteriaForPlayers(ACHIEV_CRIT_DEFENSELES, 1);
            }
            SetInstanceData(INDEX_INSTANCE_PROGRESS, Finished);
        }break;
        case CN_VIOLET_HOLD_GUARD:
        {
        }break;
        case CN_MORAGG:
        {
            SetInstanceData(INDEX_MORAGG, Finished);
        }break;
        case CN_ICHORON:
        {
            SetInstanceData(INDEX_ICHORON, Finished);
        }break;
        case CN_XEVOZZ:
        {
            SetInstanceData(INDEX_XEVOZZ, Finished);
        }break;
        case CN_LAVANTHOR:
        {
            SetInstanceData(INDEX_LAVANTHOR, Finished);
        }break;
        case CN_EREKEM:
        {
            SetInstanceData(INDEX_EREKEM, Finished);
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
            if (m_activePortal.type == VH_PORTAL_TYPE_SQUAD && GetInstanceData(INDEX_PORTAL_PROGRESS) == InProgress)
            {
                m_activePortal.DelSummonDataByGuid(GET_LOWGUID_PART(pCreature->GetGUID()));
                if (m_activePortal.summonsList.empty())
                {
                    SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
                }
            }
        }break;
        case CN_PORTAL_GUARDIAN:
        case CN_PORTAL_KEEPER:
        {
            if (m_activePortal.type == VH_PORTAL_TYPE_GUARDIAN)
            {
                SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
            }
            m_portalGuardianGUID = 0;
        }break;
        // Ghostly replacements
        case CN_ARAKKOA:
        case CN_VOID_LORD:
        case CN_ETHERAL:
        case CN_SWIRLING:
        case CN_LAVA_HOUND:
        case CN_WATCHER:
        {
            switch (GetBossEntryByGhost(pCreature->GetEntry()))
            {
                case CN_MORAGG:
                {
                    SetInstanceData(INDEX_MORAGG, Finished);
                }break;
                case CN_ICHORON:
                {
                    SetInstanceData(INDEX_ICHORON, Finished);
                }break;
                case CN_XEVOZZ:
                {
                    SetInstanceData(INDEX_XEVOZZ, Finished);
                }break;
                case CN_LAVANTHOR:
                {
                    SetInstanceData(INDEX_LAVANTHOR, Finished);
                }break;
                case CN_EREKEM:
                {
                    SetInstanceData(INDEX_EREKEM, Finished);
                }break;
            }
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
    if (GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress)
    {
        if (GetInstanceData(INDEX_PORTAL_PROGRESS) == NotStarted)
        {
            if (m_portalSummonTimer == 0)
            {
                m_portalSummonTimer = VH_NEXT_PORTAL_SPAWN_TIME;
                SpawnPortal();
                SetInstanceData(INDEX_PORTAL_PROGRESS, InProgress);
            }
            else
                --m_portalSummonTimer;
        }
    }

    //HACK: Erase non existing summons from lists, they should be removed OnDied events
    if (GetInstanceData(INDEX_PORTAL_PROGRESS) == InProgress && GetInstanceData(DATA_ARE_SUMMONS_MADE) == 1 && m_activePortal.type == VH_PORTAL_TYPE_SQUAD)
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
            SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
        }
    }
}

// Generate very basic portal info
void TheVioletHoldInstance::GenerateRandomPortal(VHPortalInfo & newPortal)
{
    uint8_t currentPortalCount = GetInstanceData(DATA_PORTAL_COUNT);
    uint8_t perviousPortal = currentPortalCount != 0 ? GetInstanceData(DATA_PERVIOUS_PORTAL_ID) : RandomUInt(MaxPortalPositions - 1);
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
            SetInstanceData(DATA_GROUP1_BOSS_ENTRY, newPortal.bossEntry);

        // Second boss
        if ((currentPortalCount + 1) == 12)
            SetInstanceData(DATA_GROUP2_BOSS_ENTRY, newPortal.bossEntry);
    }
}

// SpawnPortal
void TheVioletHoldInstance::SpawnPortal()
{
    uint8_t portalCount = GetInstanceData(DATA_PORTAL_COUNT);
    if (portalCount + 1 >= 18)
        return;

    ++portalCount;

    float x, y, z, o;
    if (m_activePortal.type != VH_PORTAL_TYPE_BOSS)
    {
        GenerateRandomPortal(m_activePortal);
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
    SetInstanceData(DATA_PORTAL_COUNT, portalCount);
    SetInstanceData(DATA_PERVIOUS_PORTAL_ID, m_activePortal.id);
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

    // Close prison cells depending on instance data

    if (GetInstanceData(INDEX_EREKEM) != Finished)
    {
        if (GameObject* pGo = GetInstance()->GetGameObject(m_ErekemCellGUID))
        {
            if (pGo->GetState() == GO_STATE_OPEN)
            {
                pGo->SetState(GO_STATE_CLOSED);
            }
        }
    }

    if (GetInstanceData(INDEX_ZURAMAT) != Finished)
    {
        if (GameObject* pGo = GetInstance()->GetGameObject(m_ZuramatCellGUID))
        {
            if (pGo->GetState() == GO_STATE_OPEN)
            {
                pGo->SetState(GO_STATE_CLOSED);
            }
        }
    }

    if (GetInstanceData(INDEX_MORAGG) != Finished)
    {
        if (GameObject* pGo = GetInstance()->GetGameObject(m_MorragCellGUID))
        {
            if (pGo->GetState() == GO_STATE_OPEN)
            {
                pGo->SetState(GO_STATE_CLOSED);
            }
        }
    }

    if (GetInstanceData(INDEX_ICHORON) != Finished)
    {
        if (GameObject* pGo = GetInstance()->GetGameObject(m_IchoronCellGUID))
        {
            if (pGo->GetState() == GO_STATE_OPEN)
            {
                pGo->SetState(GO_STATE_CLOSED);
            }
        }
    }

    if (GetInstanceData(INDEX_LAVANTHOR) != Finished)
    {
        if (GameObject* pGo = GetInstance()->GetGameObject(m_LavanthorCellGUID))
        {
            if (pGo->GetState() == GO_STATE_OPEN)
            {
                pGo->SetState(GO_STATE_CLOSED);
            }
        }
    }

    if (GetInstanceData(INDEX_XEVOZZ) != Finished)
    {
        if (GameObject* pGo = GetInstance()->GetGameObject(m_XevozzCellGUID))
        {
            if (pGo->GetState() == GO_STATE_OPEN)
            {
                pGo->SetState(GO_STATE_CLOSED);
            }
        }
    }

    // Do base stuff
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
    UpdateInstanceWorldState(WORLD_STATE_VH_SHOW, GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress ? 1 : 0);
    UpdateInstanceWorldState(WORLD_STATE_VH_PRISON_STATE, GetInstanceData(DATA_SEAL_HEALTH));
    UpdateInstanceWorldState(WORLD_STATE_VH_WAVE_COUNT, GetInstanceData(DATA_PORTAL_COUNT));
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

uint32_t TheVioletHoldInstance::GetGhostlyReplacement(uint32_t bossEntry)
{
    for (uint8_t i = 0; i < MaxBossReplacements; i++)
    {
        if (BossReplacements[i].bossEntry == bossEntry)
        {
            return BossReplacements[i].ghostEntry;
        }
    }

    return 0;
}

// Returns boss entry by ghost entry
uint32_t TheVioletHoldInstance::GetBossEntryByGhost(uint32_t ghostEntry)
{
    for (uint8_t i = 0; i < MaxBossReplacements; i++)
    {
        if (BossReplacements[i].ghostEntry == ghostEntry)
        {
            return BossReplacements[i].bossEntry;
        }
    }

    return 0;
}

void TheVioletHoldInstance::SpawnGhostlyReplacement(uint32_t bossEntry, float x, float y, float z, float o)
{
    uint32_t replacementEntry = GetGhostlyReplacement(bossEntry);
    if (replacementEntry != 0)
    {
        if (!spawnCreature(replacementEntry, x, y, z, o))
        {
            LOG_ERROR("Violet Hold: failed to spawn ghostly replacement for boss entry %u", bossEntry);
        }
    }
    else
    {
        LOG_ERROR("Violet Hold: failed to get ghostly replacement for boss entry %u", bossEntry);
    }
}

void TheVioletHoldInstance::ReleaseBoss(uint32_t gatesGuid, uint32_t bossGuid)
{
    // Open gates
    if (GameObject* pGates = GetInstance()->GetGameObject(gatesGuid))
    {
        pGates->SetState(GO_STATE_OPEN);
    }

    // Start Creature movement
    if (Creature* pBoss = GetInstance()->GetCreature(bossGuid))
    {
        if (pBoss->isAlive())
        {
            pBoss->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            pBoss->GetAIInterface()->setWayPointToMove(1);
        }
    }
}

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

    // Bosses
    mgr->register_creature_script(CN_MORAGG, &MoraggAI::Create);

    // Spells
    mgr->register_script_effect(SPELL_VH_TELEPORT_PLAYER, &TeleportPlayerInEffect);
}
