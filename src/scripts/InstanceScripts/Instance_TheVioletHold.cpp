/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
    float baseDistance = sqrt(distanceX + distanceY);
    uint32_t waitTime = static_cast<uint32_t>((1000 * std::abs(baseDistance / speed)));
    if (waitTime > 2000)
        waitTime -= 2000;
    return waitTime;
}


// Sinclari's escort event

class SinclariAI : public CreatureAIScript
{
    uint8_t m_eventId;
    uint32_t m_eventTimer;

    TheVioletHoldInstance* VH_instance;

    public:

        static CreatureAIScript* Create(Creature* c) { return new SinclariAI(c); }
        SinclariAI(Creature* pCreature) : CreatureAIScript(pCreature), m_eventId(0), m_eventTimer(0)
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

        void DoEvent()
        {
            switch (m_eventId)
            {
                // Face to guards and call them after 1 second
                case 0:
                {
                    getCreature()->SetFacing(SinclariWps[0].o);
                    _resetTimer(m_eventTimer, 1000);
                }break;
                // Call guards and after 3 seconds start another waypoint movement
                case 1:
                {
                    sendDBChatMessage(YELL_SINCLARI_LEAVING);
                    getCreature()->EventAddEmote(EMOTE_ONESHOT_SHOUT, 2000);
                    VH_instance->CallGuardsOut();
                    _resetTimer(m_eventTimer, 3000);
                }break;
                // Move her outside
                case 2:
                {
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(2);
                    _removeTimer(m_eventTimer);
                }break;
                // Emote mimic
                case 3:
                {
                    getCreature()->EventAddEmote(EMOTE_ONESHOT_TALK, 2000);
                    _resetTimer(m_eventTimer, 3000);
                }break;
                // Close the gates
                case 4:
                {
                    VH_instance->SetInstanceData(INDEX_INSTANCE_PROGRESS, PreProgress);
                    _resetTimer(m_eventTimer, 1000);
                }break;
                // Move her further
                case 5:
                {
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(3);
                    _removeTimer(m_eventTimer);
                    RemoveAIUpdateEvent();
                }break;
                default:
                    break;
            }

            ++m_eventId;
        }

        void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
        {
            switch (iWaypointId)
            {
                // Handle emote and activate crystal
                case 1:
                {
                    if (VH_instance)
                    {
                        VH_instance->SetInstanceData(INDEX_INSTANCE_PROGRESS, Performed);
                    }

                    getCreature()->EventAddEmote(EMOTE_ONESHOT_USESTANDING, 3000);
                    m_eventTimer = _addTimer(6000);
                }break;
                // Say goodbye and call event 3
                case 2:
                {
                    getCreature()->SetFacing(SinclariWps[1].o);
                    sendDBChatMessage(SAY_SINCLARI_CLOSING_GATES);
                    m_eventTimer = _addTimer(3000);
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
            if (VH_instance && VH_instance->GetInstanceData(INDEX_INSTANCE_PROGRESS) != InProgress && _isTimerFinished(m_eventTimer))
            {
                DoEvent();
            }
        }
};

class SinclariGossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* pPlayer) override
        {
            TheVioletHoldInstance* pInstance = static_cast<TheVioletHoldInstance*>(pObject->GetMapMgr()->GetScript());
            // TODO: correct text id
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), sMySQLStore.getGossipTextIdForNpc(pObject->GetEntry()));
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


// Intro event

class IntroPortalAI : public CreatureAIScript
{
    TheVioletHoldInstance* VH_instance;
    uint32_t spawnTimer;
    public:

        static CreatureAIScript* Create(Creature* c) { return new IntroPortalAI(c); }
        IntroPortalAI(Creature* pCreature) : CreatureAIScript(pCreature), portalId(-1), spawnTimer(0)
        {
            VH_instance = static_cast<TheVioletHoldInstance*>(pCreature->GetMapMgr()->GetScript());
        }

        void OnDespawn() override
        {
            // Make sure guid is removed from list
            if (VH_instance)
            {
                VH_instance->RemoveIntroNpcByGuid(GET_LOWGUID_PART(getCreature()->GetGUID()));
            }
        }

        void OnLoad() override
        {
            setRooted(true);
            getCreature()->m_canRegenerateHP = false;
            setCanEnterCombat(false);
            spawnTimer = _addTimer(Util::getRandomUInt(8, 15) * 1000);
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(spawnTimer) && VH_instance && !(VH_instance->GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress || VH_instance->GetInstanceData(INDEX_INSTANCE_PROGRESS) == Performed) && portalId != -1)
            {
                float landHeight = getCreature()->GetMapMgr()->GetLandHeight(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
                if (Creature* pAttacker = spawnCreature(VHIntroMobs[Util::getRandomUInt(VHIntroMobCount - 1)], getCreature()->GetPositionX(), getCreature()->GetPositionY(), landHeight, getCreature()->GetOrientation()))
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
                        {
                            LOG_ERROR("Violet Hold: unhandled intro portal id %u", portalId);
                        }break;
                    }
                }
                _resetTimer(spawnTimer, Util::getRandomUInt(8, 15) * 1000);
            }
        }

// public variables
public:
    int8_t portalId;
};


class VHAttackerAI : public CreatureAIScript
{
    TheVioletHoldInstance* pInstance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new VHAttackerAI(c); }
        VHAttackerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            pInstance = static_cast<TheVioletHoldInstance*>(pCreature->GetMapMgr()->GetScript());
            switch (pCreature->GetEntry())
            {
                case CN_INTRO_AZURE_MAGE_SLAYER_MELEE:
                {
                    addAISpell(58469, 9.0f, TARGET_SELF, 0, 11, false, true);           // Arcane Empowerment
                }break;
                case CN_INTRO_AZURE_SPELLBREAKER_ARCANE:
                {
                    addAISpell(58462, 9.0f, TARGET_ATTACKING, 0, 0, false, true);       // Arcane Blast
                    addAISpell(25603, 8.0f, TARGET_ATTACKING, 0, 0, false, true);       // Slow
                }break;
                case CN_AZURE_INVADER:
                case CN_INTRO_AZURE_INVADER_ARMS:
                {
                    addAISpell(15496, 9.0f, TARGET_ATTACKING, 0, 0, false, true);       // Cleave
                    addAISpell(58459, 8.0f, TARGET_ATTACKING, 0, 13, false, true);      // Impale
                }break;
                case CN_AZURE_SPELLBREAKER:
                {
                    addAISpell(58462, 9.0f, TARGET_ATTACKING, 0, 0, false, true);       // Arcane Blast
                }break;
                case CN_AZURE_BINDER:
                case CN_INTRO_AZURE_BINDER_ARCANE:
                {
                    addAISpell(58456, 9.0f, TARGET_ATTACKING, 0, 2, false, true);       // Arcane Barrage
                    addAISpell(58455, 8.0f, TARGET_SELF, 0, 5, false, false);           // Arcane Explosion
                }break;
                case CN_AZURE_MAGE_SLAYER:
                {
                    addAISpell(60204, 9.0f, TARGET_SELF, 0, 11, false, true);           // Arcane Empowerment
                }break;
                case CN_AZURE_CAPTAIN:
                {
                    addAISpell(32736, 9.0f, TARGET_ATTACKING, 0, 6, false, true);       // Mortal Strike
                    addAISpell(41057, 9.0f, TARGET_ATTACKING, 0, 5, false, true);       // Wirtlwind
                }break;
                case CN_AZURE_SORCERER:
                {
                    addAISpell(60182, 8.0f, TARGET_ATTACKING, 0, 0, false, true);       // Mana Detonation
                    if (pCreature->GetMapMgr()->iInstanceMode == MODE_HEROIC)
                    {
                        addAISpell(60204, 9.0f, TARGET_ATTACKING, 4, 0, false, true);    // Arcane Stream
                    }
                }break;
                case CN_AZURE_RAIDER:
                {
                    addAISpell(60158, 8.0f, TARGET_SELF, 0, 6, false, true);            // Magic Reflection
                }break;
                case CN_AZURE_STALKER:
                {
                    addAISpell(58471, 8.0f, TARGET_ATTACKING, 0, 0, false, true);       // Backstab
                    addAISpell(58470, 7.0f, TARGET_ATTACKING, 0, 0, false, true);       // Tactical Blink
                }break;
                case CN_VETERAN_MAGE_HUNTER:
                {
                    addAISpell(20829, 8.0f, TARGET_ATTACKING, 1, 0, false, true);       // Arcane Bolt
                    addAISpell(20823, 7.0f, TARGET_ATTACKING, 3, 0, false, true);       // Fireball
                    addAISpell(20822, 6.0f, TARGET_ATTACKING, 3, 0, false, true);       // Frostbolt
                }break;
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
                getCreature()->GetAIInterface()->setFacing(M_PI_FLOAT);
                getCreature()->SetChannelSpellId(SPELL_VH_DESTROY_DOOR_SEAL);
                getCreature()->SetChannelSpellTargetGUID(triggerGUID);
            }
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            // Stop channelling
            StartChanneling(0);
            despawn(2000);
            if (pInstance)
            {
                pInstance->RemoveIntroNpcByGuid(GET_LOWGUID_PART(getCreature()->GetGUID()));
            }
        }

        void OnCombatStart(Unit* /*pKiller*/) override
        {
            StartChanneling(0); // Stop channeling
        }

        void OnCombatStop(Unit* /*pEnemy*/) override
        {
            if (getCreature()->isAlive())
            {
                getCreature()->GetAIInterface()->StopMovement(3000);
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                if (getCreature()->GetAIInterface()->getCurrentWayPointId() < getCreature()->GetAIInterface()->getWayPointsCount() - 1)
                {
                    SetWaypointToMove(getCreature()->GetAIInterface()->getCurrentWayPointId() + 1);
                }
                else
                {
                    SetWaypointToMove(getCreature()->GetAIInterface()->getWayPointsCount() - 1);
                }
            }
        }

        void OnLoad() override
        {
            getCreature()->GetAIInterface()->StopMovement(2000);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
            getCreature()->GetAIInterface()->setWayPointToMove(1);
        }

        void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
        {
            /*if (iWaypointId < getCreature()->GetAIInterface()->getWayPointsCount() - 1)
            {
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
            }*/

            if (pInstance->GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress && pInstance->GetInstanceData(DATA_SEAL_HEALTH) != 0
                && iWaypointId == getCreature()->GetAIInterface()->getWayPointsCount() - 1)
            {
                if (Creature* pTriggerTarget = getNearestCreature(1823.696045f, 803.604858f, 44.895786f, CN_DOOR_SEAL))
                {

                    StartChanneling(pTriggerTarget->GetGUID());
                    getCreature()->CastSpell(pTriggerTarget, sSpellCustomizations.GetSpellInfo(SPELL_VH_DESTROY_DOOR_SEAL), true);
                }
            }
        }

        void AIUpdate() override
        {
            // attack closest enemy
            Unit* pEnemy = getBestUnitTarget(TargetFilter_Closest);
            if (pEnemy && getRangeToObject(pEnemy) < 15.0f)
            {
                getCreature()->GetAIInterface()->AttackReaction(pEnemy, 1, 0);
            }
        }
};


// Defense system AI

class VH_DefenseAI : public CreatureAIScript
{
        uint32_t counter;
    public:

        static CreatureAIScript* Create(Creature* c) { return new VH_DefenseAI(c); }
        VH_DefenseAI(Creature* pCreature) : CreatureAIScript(pCreature), counter(0)
        {
            despawn(5000);
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
                    for (std::vector<uint32_t>::iterator itr = pInstance->m_introSpawns.begin(); itr != pInstance->m_introSpawns.end();)
                    {
                        if (counter == 3)
                        {
                            if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                            {
                                // HACK
                                getCreature()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                                pTarget->Die(pTarget, pTarget->GetHealth(), 0);
                            }
                            itr = pInstance->m_introSpawns.erase(itr);
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
                            ++itr;
                        }
                    }
                }

                // Main event spawns
                if (!pInstance->m_eventSpawns.empty())
                {
                    for (std::vector<uint32_t>::iterator itr = pInstance->m_eventSpawns.begin(); itr != pInstance->m_eventSpawns.end(); ++itr)
                    {
                        if (counter == 3)
                        {
                            if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                            {
                                //HACK
                                getCreature()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                                pTarget->Die(pTarget, pTarget->GetHealth(), 0);
                            }
                            itr = pInstance->m_eventSpawns.erase(itr);
                        }
                        else
                        {
                            if (Creature* pTarget = pInstance->GetInstance()->GetCreature(*itr))
                            {
                                getCreature()->CastSpellAoF(pTarget->GetPosition(), sSpellCustomizations.GetSpellInfo(SPELL_VH_LIGHTNING_INTRO), true);
                            }
                            ++itr;
                        }
                    }
                }

                // Defense triggers (ONLY animation)
                if (!pInstance->m_defenseTriggers.empty() && counter < 3)
                {
                    for (std::vector<uint32_t>::iterator itr = pInstance->m_defenseTriggers.begin(); itr != pInstance->m_defenseTriggers.end(); ++itr)
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

                if (counter > 3)
                {
                    getCreature()->CastSpell(getCreature(), SPELL_VH_DEFENSE_SYSTEM_SPAWN, true);
                }
                ++counter;
            }
        }
};


// Base portal AI
// Summons squares, keepers/guardians, saboteur and last boss

class TeleportationPortalAI : public CreatureAIScript
{
        bool isSummonStarted;
        bool isGuardianSpawned;
        uint32_t spawnTimer;
        TheVioletHoldInstance* pInstance;
    public:

        static CreatureAIScript* Create(Creature* c) { return new TeleportationPortalAI(c); }
        TeleportationPortalAI(Creature* pCreature) : 
            CreatureAIScript(pCreature), 
            isSummonStarted(false), 
            isGuardianSpawned(false), 
            spawnTimer(0)
        {
            pInstance = static_cast<TheVioletHoldInstance*>(getCreature()->GetMapMgr()->GetScript());
        }

        void OnLoad() override
        {
            setRooted(true);
            getCreature()->m_canRegenerateHP = false;
            setCanEnterCombat(false);
            switch (pInstance->m_activePortal.type)
            {
                case VH_PORTAL_TYPE_BOSS:
                {
                    spawnTimer = _addTimer(VH_TELE_PORTAL_BOSS_SPAWN_TIME);
                }break;
                case VH_PORTAL_TYPE_SQUAD:
                case VH_PORTAL_TYPE_GUARDIAN:
                {
                    spawnTimer = _addTimer(VH_TELE_PORTAL_WAVE_TIMER);
                }break;
                default:
                    break;
            }
        }

        void AIUpdate() override
        {
            if (!pInstance)
            {
                return;
            }

            if (_isTimerFinished(spawnTimer))
            {
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
                            if (Creature* pGuardian = spawnCreature(pInstance->m_activePortal.guardianEntry, getCreature()->GetPositionX(), getCreature()->GetPositionY(), landHeight, getCreature()->GetOrientation()))
                            {
                                isGuardianSpawned = true;
                                getCreature()->SetChannelSpellId(SPELL_VH_PORTAL_CHANNEL);
                                getCreature()->SetChannelSpellTargetGUID(pGuardian->GetGUID());
                            }
                        }
                        else
                        {
                            // Spawn 3 random portal guardians
                            for (uint8_t i = 0; i < 3; i++)
                            {
                                if (Creature* pSummon = spawnCreature(portalGuardians[Util::getRandomUInt(maxPortalGuardians - 1)], getCreature()->GetPositionX() + Util::getRandomFloat(float(i)) + 1.0f, getCreature()->GetPositionY() + Util::getRandomFloat(float(i)) + 1.0f, landHeight, getCreature()->GetOrientation()))
                                {
                                    AddWaypoint(pSummon, pInstance->m_activePortal.id, 0);
                                }
                            }
                        }
                        _resetTimer(spawnTimer, VH_TELE_PORTAL_WAVE_TIMER);
                    }break;
                    case VH_PORTAL_TYPE_SQUAD:
                    {
                        getCreature()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, SQUAD_ANNOUNCE);
                        float landHeight = getCreature()->GetMapMgr()->GetLandHeight(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
                        //TODO: This count needs to be corrected
                        for (uint8_t i = 0; i < 5; i++)
                        {
                            if (Creature* pSummon = spawnCreature(portalGuardians[Util::getRandomUInt(maxPortalGuardians - 1)], getCreature()->GetPositionX() + Util::getRandomFloat(float(i)) + 1.0f, getCreature()->GetPositionY() + Util::getRandomFloat(float(i)) + 1.0f, landHeight, getCreature()->GetOrientation()))
                            {
                                //TODO: replace this with cleaner solution
                                pInstance->m_activePortal.summonsList.push_back(GET_LOWGUID_PART(pSummon->GetGUID()));
                                pInstance->m_eventSpawns.push_back(GET_LOWGUID_PART(pSummon->GetGUID()));
                                AddWaypoint(pSummon, pInstance->m_activePortal.id, 0);
                            }
                        }
                        _removeTimer(spawnTimer);
                        RemoveAIUpdateEvent();
                        despawn(1000, 0);
                    }break;
                    // Spawn Soboteur which will open boss cell
                    case VH_PORTAL_TYPE_BOSS:
                    {
                        float landHeight = getCreature()->GetMapMgr()->GetLandHeight(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ());
                        if (pInstance->GetInstanceData(DATA_PORTAL_COUNT) != 18)
                        {
                            if (Creature* pSaboteur = spawnCreature(CN_AZURE_SABOTEUR, getCreature()->GetPositionX(), getCreature()->GetPositionY(), landHeight, getCreature()->GetOrientation()))
                            {
                                AddWaypoint(pSaboteur, 0, pInstance->m_activePortal.bossEntry);
                            }
                        }
                        else
                        {
                            spawnCreature(CN_CYANIGOSA, getCreature()->GetPositionX(), getCreature()->GetPositionY(), landHeight, getCreature()->GetOrientation());
                        }
                        _removeTimer(spawnTimer);
                        RemoveAIUpdateEvent();
                        despawn(1000, 0);
                    }break;
                }
                pInstance->SetInstanceData(DATA_ARE_SUMMONS_MADE, 1);
            }
        }

        void AddWaypoint(Creature* pCreature, uint8_t portalId, uint32_t bossEntry)
        {
            if (!pCreature)
                return;

            float runSpeed = pCreature->GetCreatureProperties()->run_speed;
            // Basic portal
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
                        if (Util::getRandomUInt(1))
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
                        uint32_t waitTime = GenerateWPWaitTime(runSpeed, DefaultPortalWPs.x, pCreature->GetPositionX(), DefaultPortalWPs.y, pCreature->GetPositionY());
                        pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(1, waitTime, Movement::WP_MOVE_TYPE_RUN, DefaultPortalWPs));
                        // Unused waypoint
                        Movement::Location emptyLoc = { 0, 0, 0, 0 };
                        pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(2, waitTime, Movement::WP_MOVE_TYPE_RUN, emptyLoc));

                    }break;
                }
            }
            // Boss portal
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
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurMoraggPath[i].x, pCreature->GetPositionX(), SaboteurMoraggPath[i].y, pCreature->GetPositionY());
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
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurIchoronPath[i].x, pCreature->GetPositionX(), SaboteurIchoronPath[i].y, pCreature->GetPositionY());
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
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurXevozzPath[i].x, pCreature->GetPositionX(), SaboteurXevozzPath[i].y, pCreature->GetPositionY());
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
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurLavanthorPath[i].x, pCreature->GetPositionX(), SaboteurLavanthorPath[i].y, pCreature->GetPositionY());
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
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurErekemPath[i].x, pCreature->GetPositionX(), SaboteurErekemPath[i].y, pCreature->GetPositionY());
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
                                waitTime = GenerateWPWaitTime(runSpeed, SaboteurZuramatPath[i].x, pCreature->GetPositionX(), SaboteurZuramatPath[i].y, pCreature->GetPositionY());
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
    uint32_t eventTimer;
    public:

        static CreatureAIScript* Create(Creature* c) { return new AzureSaboteurAI(c); }
        AzureSaboteurAI(Creature* pCreature) : CreatureAIScript(pCreature), mStep(0), eventTimer(0)
        {
            pInstance = static_cast<TheVioletHoldInstance* >(pCreature->GetMapMgr()->GetScript());
            setCanEnterCombat(false);
        }

        void OnLoad() override
        {
            setCanEnterCombat(false);
            getCreature()->GetAIInterface()->StopMovement(2000);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(1);
        }

        void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
        {
            if (pInstance)
            {
                switch (pInstance->m_activePortal.bossEntry)
                {
                    case CN_MORAGG:
                    {
                        if (iWaypointId == getCreature()->GetAIInterface()->getWayPointsCount() - 1)
                        {
                            getCreature()->SetFacing(4.689994f);
                            eventTimer = _addTimer(1000);
                        }
                    }break;
                    case CN_ICHORON:
                    {
                        if (iWaypointId == getCreature()->GetAIInterface()->getWayPointsCount() - 1)
                        {
                            getCreature()->SetFacing(5.411396f);
                            eventTimer = _addTimer(1000);
                        }
                    }break;
                    case CN_XEVOZZ:
                    {
                        if (iWaypointId == getCreature()->GetAIInterface()->getWayPointsCount() - 1)
                        {
                            getCreature()->SetFacing(1.060255f);
                            eventTimer = _addTimer(1000);
                        }
                    }break;
                    case CN_LAVANTHOR:
                    {
                        if (iWaypointId == getCreature()->GetAIInterface()->getWayPointsCount() - 1)
                        {
                            getCreature()->SetFacing(4.025167f);
                            eventTimer = _addTimer(1000);
                        }
                    }break;
                    case CN_EREKEM:
                    {
                        if (iWaypointId == getCreature()->GetAIInterface()->getWayPointsCount() - 1)
                        {
                            getCreature()->SetFacing(1.955642f);
                            eventTimer = _addTimer(1000);
                        }
                    }break;
                    case CN_ZURAMAT:
                    {
                        if (iWaypointId == getCreature()->GetAIInterface()->getWayPointsCount() - 1)
                        {
                            getCreature()->SetFacing(0.942478f);
                            eventTimer = _addTimer(2000);
                        }
                    }break;
                }

                if (iWaypointId != getCreature()->GetAIInterface()->getWayPointsCount() - 1)
                {
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                }
            }
        }

        void OnDespawn() override
        {
            if (pInstance)
            {
                switch (pInstance->m_activePortal.bossEntry)
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
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(eventTimer))
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
                        RemoveAIUpdateEvent();
                    }break;
                }

                ++mStep;
                if (mStep != 4)
                {
                    _resetTimer(eventTimer, 1000);
                }
                else
                {
                    _removeTimer(eventTimer);
                }
            }
        }
};

// BOSSES

// Moragg boss event

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
        addAISpell(SPELL_CORROSIVE_SALIVA, TARGET_ATTACKING, 8.0f, 0, 6);
        if (pCreature->GetMapMgr()->iInstanceMode == MODE_HEROIC)
        {
            addAISpell(SPELL_RAY_OF_PAIN_H, TARGET_RANDOM_SINGLE, 7.0f, 0, 25);
            addAISpell(SPELL_RAY_OF_SUFFERING_H, TARGET_RANDOM_SINGLE, 7.0f, 0, 5);
        }
        else
        {
            addAISpell(SPELL_RAY_OF_PAIN, TARGET_RANDOM_SINGLE, 7.0f, 0, 25);
            addAISpell(SPELL_RAY_OF_SUFFERING, TARGET_RANDOM_SINGLE, 7.0f, 0, 5);
        }
        setCanEnterCombat(false);
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == MoraggPathSize - 1)
        {
            setCanEnterCombat(true);
            getCreature()->GetAIInterface()->AttackReaction(getNearestPlayer(), 1, 0);
        }
    }
};


// Zuramat boss event

class ZuramatAI : public CreatureAIScript
{
    enum Spells
    {
        SPELL_SHROUD_OF_DARKNESS        = 54524,
        SPELL_SUMMON_VOID_SENTRY        = 54369,
        SPELL_VOID_SHIFT                = 54361,
        SPELL_VOID_SHIFTED              = 54343,
        SPELL_ZURAMAT_ADD               = 54341,
        SPELL_ZURAMAT_ADD_2             = 54342,
        SPELL_ZURAMAT_ADD_DUMMY         = 54351,
        SPELL_SUMMON_VOID_SENTRY_BALL   = 58650
    };

    uint32_t mVoidSentryTimer;
    uint32_t mVoidShiftTimer;
    uint32_t mShroudTimer;

public:

    static CreatureAIScript* Create(Creature* c) { return new ZuramatAI(c); }
    ZuramatAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Prepare waypoints
        float walkSpeed = pCreature->GetCreatureProperties()->walk_speed;
        for (uint8_t i = 0; i < ZuramatPathSize; i++)
        {
            uint32_t waitTime = 0;
            // First wp
            if (i == 0)
                waitTime = GenerateWPWaitTime(walkSpeed, ZuramatPath[i].x, pCreature->GetPositionX(), ZuramatPath[i].y, pCreature->GetPositionY());
            else
                waitTime = GenerateWPWaitTime(walkSpeed, ZuramatPath[i].x, ZuramatPath[i - 1].x, ZuramatPath[i].y, ZuramatPath[i - 1].y);

            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_WALK, ZuramatPath[i]));
        }
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        setCanEnterCombat(false);
        mVoidSentryTimer = 0;
        mVoidShiftTimer = 0;
        mShroudTimer = 0;
    }

    void OnCombatStart(Unit* pEnemy) override
    {
        mVoidSentryTimer = _addTimer(4000);
        mVoidShiftTimer = _addTimer(9000);
        mShroudTimer = _addTimer(Util::getRandomUInt(18, 20) * 1000);
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == ZuramatPathSize - 1)
        {
            setCanEnterCombat(true);
            getCreature()->GetAIInterface()->AttackReaction(getNearestPlayer(), 1, 0);
        }
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mVoidSentryTimer))
        {
            getCreature()->CastSpell(getCreature(), SPELL_SUMMON_VOID_SENTRY, true);
            _resetTimer(mVoidSentryTimer, Util::getRandomUInt(7, 10) * 1000);
        }

        if (_isTimerFinished(mVoidShiftTimer))
        {
            Unit* pTarget = getBestPlayerTarget(static_cast<TargetFilter>(TargetFilter_Aggroed | TargetFilter_InRangeOnly), 0, 60.0f);
            if (pTarget && pTarget->isAlive())
            {
                getCreature()->CastSpell(pTarget, SPELL_VOID_SHIFT, false);
                _resetTimer(mVoidShiftTimer, 15000);
            }
        }

        if (_isTimerFinished(mShroudTimer))
        {
            getCreature()->CastSpell(getCreature(), SPELL_SHROUD_OF_DARKNESS, true);
            _resetTimer(mShroudTimer, Util::getRandomUInt(18, 20) * 1000);
        }
    }
};

class VoidSentryAI : public CreatureAIScript
{
    enum Spells
    {
        SPELL_VOID_SHIFTED = 54343,
        SPELL_ZURAMAT_ADD = 54341,
        SPELL_ZURAMAT_ADD_2 = 54342,
        SPELL_ZURAMAT_ADD_DUMMY = 54351,
        SPELL_SUMMON_VOID_SENTRY_BALL = 58650
    };

    uint32_t mVoidSentryTimer;
    uint32_t mVoidShiftTimer;
    uint32_t mShroudTimer;

public:

    static CreatureAIScript* Create(Creature* c) { return new VoidSentryAI(c); }
    VoidSentryAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCreature->GetAIInterface()->setAiScriptType(AI_SCRIPT_PASSIVE);
        setRooted(true);
    }

    void OnLoad() override
    {
        getCreature()->CastSpell(getCreature(), SPELL_ZURAMAT_ADD, true);
        getCreature()->CastSpell(getCreature(), SPELL_ZURAMAT_ADD_DUMMY, true);
        getCreature()->CastSpell(getCreature(), SPELL_SUMMON_VOID_SENTRY_BALL, true);
    }

    void OnDied(Unit* /*pEnemy*/) override
    {
        despawn(2000, 0);
    }
};

bool OnRemoveVoidShift(Aura* aura)
{
    if (aura && aura->GetSpellInfo()->getId() == 54361) // Void Shift
    {
        Unit* pTarget = aura->GetTarget();
        if (pTarget && pTarget->isAlive())
        {
            pTarget->CastSpell(pTarget, 54343, true);   // Void Shifted
        }
    }
}

// Ichonor boss event

class IchoronAI : public CreatureAIScript
{
public:

    static CreatureAIScript* Create(Creature* c) { return new IchoronAI(c); }
    IchoronAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Prepare waypoints
        float walkSpeed = pCreature->GetCreatureProperties()->walk_speed;
        for (uint8_t i = 0; i < IchoronPathSize; i++)
        {
            uint32_t waitTime = 0;
            // First wp
            if (i == 0)
                waitTime = GenerateWPWaitTime(walkSpeed, IchoronPath[i].x, pCreature->GetPositionX(), IchoronPath[i].y, pCreature->GetPositionY());
            else
                waitTime = GenerateWPWaitTime(walkSpeed, IchoronPath[i].x, IchoronPath[i - 1].x, IchoronPath[i].y, IchoronPath[i - 1].y);

            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_WALK, IchoronPath[i]));
        }
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        setCanEnterCombat(false);
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == IchoronPathSize - 1)
        {
            setCanEnterCombat(true);
            getCreature()->GetAIInterface()->AttackReaction(getNearestPlayer(), 1, 0);
        }
    }
};


// Lavanthor boss event

class LavanthorAI : public CreatureAIScript
{
public:

    static CreatureAIScript* Create(Creature* c) { return new LavanthorAI(c); }
    LavanthorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Prepare waypoints
        float walkSpeed = pCreature->GetCreatureProperties()->walk_speed;
        for (uint8_t i = 0; i < LavanthorPathSize; i++)
        {
            uint32_t waitTime = 0;
            // First wp
            if (i == 0)
                waitTime = GenerateWPWaitTime(walkSpeed, LavanthorPath[i].x, pCreature->GetPositionX(), LavanthorPath[i].y, pCreature->GetPositionY());
            else
                waitTime = GenerateWPWaitTime(walkSpeed, LavanthorPath[i].x, LavanthorPath[i - 1].x, LavanthorPath[i].y, LavanthorPath[i - 1].y);

            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_WALK, LavanthorPath[i]));
        }
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        setCanEnterCombat(false);
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == LavanthorPathSize - 1)
        {
            setCanEnterCombat(true);
            getCreature()->GetAIInterface()->AttackReaction(getNearestPlayer(), 1, 0);
        }
    }
};


// Xevozz boss event

class XevozzAI : public CreatureAIScript
{
public:

    static CreatureAIScript* Create(Creature* c) { return new XevozzAI(c); }
    XevozzAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Prepare waypoints
        float walkSpeed = pCreature->GetCreatureProperties()->walk_speed;
        for (uint8_t i = 0; i < XevozzPathSize; i++)
        {
            uint32_t waitTime = 0;
            // First wp
            if (i == 0)
                waitTime = GenerateWPWaitTime(walkSpeed, XevozzPath[i].x, pCreature->GetPositionX(), XevozzPath[i].y, pCreature->GetPositionY());
            else
                waitTime = GenerateWPWaitTime(walkSpeed, XevozzPath[i].x, XevozzPath[i - 1].x, XevozzPath[i].y, XevozzPath[i - 1].y);

            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_WALK, XevozzPath[i]));
        }
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);

        addEmoteForEvent(Event_OnCombatStart, YELL_XEVOZZ_AGROO);
        addEmoteForEvent(Event_OnTargetDied, YELL_XEVOZZ_TARGET_DEATH1);
        addEmoteForEvent(Event_OnTargetDied, YELL_XEVOZZ_TARGET_DEATH2);
        addEmoteForEvent(Event_OnTargetDied, YELL_XEVOZZ_TARGET_DEATH3);
        addEmoteForEvent(Event_OnDied, YELL_XEVOZZ_DEATH);
        setCanEnterCombat(false);
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == XevozzPathSize - 1)
        {
            setCanEnterCombat(true);
            getCreature()->GetAIInterface()->AttackReaction(getNearestPlayer(), 1, 0);
        }
    }
};

// Erekem boss event

class ErekemAI : public CreatureAIScript
{
public:

    static CreatureAIScript* Create(Creature* c) { return new ErekemAI(c); }
    ErekemAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Prepare waypoints
        float walkSpeed = pCreature->GetCreatureProperties()->walk_speed;
        for (uint8_t i = 0; i < ErekemPathSize; i++)
        {
            uint32_t waitTime = 0;
            // First wp
            if (i == 0)
                waitTime = GenerateWPWaitTime(walkSpeed, ErekemPath[i].x, pCreature->GetPositionX(), ErekemPath[i].y, pCreature->GetPositionY());
            else
                waitTime = GenerateWPWaitTime(walkSpeed, ErekemPath[i].x, ErekemPath[i - 1].x, ErekemPath[i].y, ErekemPath[i - 1].y);

            pCreature->GetAIInterface()->addWayPoint(CreateWaypoint(i + 1, waitTime, Movement::WP_MOVE_TYPE_WALK, ErekemPath[i]));
        }
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        setCanEnterCombat(false);
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == ErekemPathSize - 1)
        {
            setCanEnterCombat(true);
            getCreature()->GetAIInterface()->AttackReaction(getNearestPlayer(), 1, 0);
        }
    }
};

// Spell scripts

bool TeleportPlayerInEffect(uint8_t /*i*/, Spell* pSpell)
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
    generateBossDataState();

    m_VHencounterData[DATA_GROUP1_BOSS_ENTRY] = getData(DATA_GROUP1_BOSS_ENTRY) != InvalidState ? getData(DATA_GROUP1_BOSS_ENTRY) : NotStarted;
    m_VHencounterData[DATA_GROUP2_BOSS_ENTRY] = getData(DATA_GROUP2_BOSS_ENTRY) != InvalidState ? getData(DATA_GROUP1_BOSS_ENTRY) : NotStarted;

    // Achievement data
    if (GetInstance()->iInstanceMode == MODE_HEROIC)
    {
        m_VHencounterData[INDEX_ACHIEV_DEFENSELESS] = getData(INDEX_ACHIEV_DEFENSELESS) != InvalidState ? getData(INDEX_ACHIEV_DEFENSELESS) : 0;
        m_VHencounterData[INDEX_ACHIEV_VOID_DANCE] = getData(INDEX_ACHIEV_VOID_DANCE) != InvalidState ? getData(INDEX_ACHIEV_VOID_DANCE) : 0;
    }
}

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
                    setGameObjectStateForEntry(GO_PRISON_SEAL, GO_STATE_CLOSED);
                    ResetCrystals(true);
                    SetInstanceData(DATA_SEAL_HEALTH, 100);
                }break;
                case InProgress:
                {
                    m_portalSummonTimer = addTimer(VH_INITIAL_PORTAL_TIME);
                }break;
                case State_Failed:
                {
                    ResetIntro();
                    ResetInstanceData();
                }break;
                case Finished:
                {
                    // Open gates
                    setGameObjectStateForEntry(GO_PRISON_SEAL, GO_STATE_OPEN);

                    // Start her outro event
                    if (Creature* pSinclari = GetInstance()->GetCreature(m_sinclariGUID))
                    {
                        pSinclari->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        pSinclari->GetAIInterface()->setWayPointToMove(3);
                    }

                    // Hide worldstates
                    UpdateInstanceWorldState(WORLD_STATE_VH_SHOW, GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress ? 1 : 0);
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
                resetTimer(m_portalSummonTimer, (GetInstanceData(DATA_PORTAL_COUNT) == 6 || GetInstanceData(DATA_PORTAL_COUNT) == 12) ? VH_TIMER_AFTER_BOSS : VH_NEXT_PORTAL_SPAWN_TIME);

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
                case PreProgress:
                {
                    setGameObjectStateForEntry(GO_MORAGG_CELL, GO_STATE_OPEN);
                    if (Creature* Moragg = GetCreatureByGuid(m_MoraggGUID))
                    {
                        Moragg->GetAIInterface()->StopMovement(5000);
                        Moragg->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                        Moragg->GetAIInterface()->setWayPointToMove(1);
                    }
                }break;
                case State_Failed:
                {
                    setGameObjectStateForEntry(GO_MORAGG_CELL, GO_STATE_CLOSED);
                }break;
            }
        }break;
        case INDEX_ICHORON:
        {
            switch (pData)
            {
                case PreProgress:
                {
                    setGameObjectStateForEntry(GO_ICHORON_CELL, GO_STATE_OPEN);
                    if (Creature* Ichoron = GetCreatureByGuid(m_IchoronGUID))
                    {
                        Ichoron->GetAIInterface()->StopMovement(5000);
                        Ichoron->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                        Ichoron->GetAIInterface()->setWayPointToMove(1);
                    }
                }break;
                case State_Failed:
                {
                    setGameObjectStateForEntry(GO_ICHORON_CELL, GO_STATE_CLOSED);
                }break;
            }
        }break;
        case INDEX_ZURAMAT:
        {
            switch (pData)
            {
                case PreProgress:
                {
                    setGameObjectStateForEntry(GO_ZURAMAT_CELL, GO_STATE_OPEN);
                    if (Creature* Zuramat = GetCreatureByGuid(m_ZuramatGUID))
                    {
                        Zuramat->GetAIInterface()->StopMovement(5000);
                        Zuramat->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                        Zuramat->GetAIInterface()->setWayPointToMove(1);
                    }
                }break;
                case State_Failed:
                {
                    setGameObjectStateForEntry(GO_ZURAMAT_CELL, GO_STATE_CLOSED);
                }break;
            }
        }break;
        case INDEX_EREKEM:
        {
            switch (pData)
            {
                case PreProgress:
                {
                    setGameObjectStateForEntry(GO_EREKEM_CELL, GO_STATE_OPEN);
                    if (Creature* Erekem = GetCreatureByGuid(m_ErekemGUID))
                    {
                        Erekem->GetAIInterface()->StopMovement(5000);
                        Erekem->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                        Erekem->GetAIInterface()->setWayPointToMove(1);
                    }

                    // Guard on left
                    setGameObjectStateForEntry(GO_EREKEM_GUARD_CELL1, GO_STATE_OPEN);
                    if (Creature* Guard = GetInstance()->GetInterface()->GetCreatureNearestCoords(leftErekemGuardPosition.x, leftErekemGuardPosition.y, leftErekemGuardPosition.z, CN_EREKEM_GUARD))
                    {
                        Guard->GetAIInterface()->StopMovement(5000);
                        Guard->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                        Guard->GetAIInterface()->setWayPointToMove(1);
                    }

                    // Guard on right
                    setGameObjectStateForEntry(GO_EREKEM_GUARD_CELL2, GO_STATE_OPEN);
                    if (Creature* Guard = GetInstance()->GetInterface()->GetCreatureNearestCoords(rightErekemGuardPosition.x, rightErekemGuardPosition.y, rightErekemGuardPosition.z, CN_EREKEM_GUARD))
                    {
                        Guard->GetAIInterface()->StopMovement(5000);
                        Guard->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                        Guard->GetAIInterface()->setWayPointToMove(1);
                    }

                }break;
                case State_Failed:
                {
                    setGameObjectStateForEntry(GO_EREKEM_CELL, GO_STATE_CLOSED);
                    setGameObjectStateForEntry(GO_EREKEM_GUARD_CELL1, GO_STATE_CLOSED);
                    setGameObjectStateForEntry(GO_EREKEM_GUARD_CELL2, GO_STATE_CLOSED);
                }break;
            }
        }break;
        case INDEX_LAVANTHOR:
        {
            switch (pData)
            {
                case PreProgress:
                {
                    setGameObjectStateForEntry(GO_LAVANTHOR_CELL, GO_STATE_OPEN);
                    if (Creature* Lavanthor = GetCreatureByGuid(m_LavanthorGUID))
                    {
                        Lavanthor->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                        Lavanthor->GetAIInterface()->setWayPointToMove(1);
                    }
                }break;
                case State_Failed:
                {
                    setGameObjectStateForEntry(GO_LAVANTHOR_CELL, GO_STATE_CLOSED);
                }break;
            }
        }break;
        case INDEX_XEVOZZ:
        {
            switch (pData)
            {
                case PreProgress:
                {
                    setGameObjectStateForEntry(GO_XEVOZZ_CELL, GO_STATE_OPEN);
                    if (Creature* Xevozz = GetCreatureByGuid(m_XevozzGUID))
                    {
                        Xevozz->SendScriptTextChatMessage(YELL_XEVOZZ_RELEASE);
                        Xevozz->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
                        Xevozz->GetAIInterface()->setWayPointToMove(1);
                    }
                }break;
                case State_Failed:
                {
                    setGameObjectStateForEntry(GO_XEVOZZ_CELL, GO_STATE_CLOSED);
                }break;
            }
        }break;
        case DATA_GROUP1_BOSS_ENTRY:
        case DATA_GROUP2_BOSS_ENTRY:
        case INDEX_ACHIEV_DEFENSELESS:
        case INDEX_ACHIEV_VOID_DANCE:
        {
            setData(pIndex, pData);
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

// Generate very basic portal info
void TheVioletHoldInstance::GeneratePortalInfo(VHPortalInfo & newPortal)
{
    uint8_t currentPortalCount = GetInstanceData(DATA_PORTAL_COUNT);
    uint8_t perviousPortal = currentPortalCount != 0 ? GetInstanceData(DATA_PERVIOUS_PORTAL_ID) : Util::getRandomUInt(MaxPortalPositions - 1);
    uint8_t newPortalId = Util::getRandomUInt(MaxPortalPositions - 1);

    // Clear pervious portal data
    newPortal.ResetData();

    if ((currentPortalCount + 1) != 6 && (currentPortalCount + 1) != 12 && (currentPortalCount + 1) != 18)
    {
        // Generate new portal id which doesn't match to pervious portal
        while (newPortalId == perviousPortal)
        {
            newPortalId = Util::getRandomUInt(MaxPortalPositions - 1);
        }
        newPortal.id = newPortalId;

        // if portal id is between 0 and 4, its guardian/keeper type
        if (newPortal.id > 4)
        {
            newPortal.type = VH_PORTAL_TYPE_SQUAD;
        }
        else
        {
            newPortal.guardianEntry = Util::getRandomUInt(1) ? CN_PORTAL_GUARDIAN : CN_PORTAL_KEEPER;
            newPortal.type = VH_PORTAL_TYPE_GUARDIAN;
            // summon list data will published on spawn event
        }
    }
    // boss portal
    else
    {
        newPortal.type = VH_PORTAL_TYPE_BOSS;

        // Last boss
        if ((currentPortalCount + 1) == 18)
        {
            newPortal.bossEntry = CN_CYANIGOSA;
        }
        // Other bosses
        else
        {
            // First boss
            if ((currentPortalCount + 1) == 6)
            {
                if (GetInstanceData(DATA_GROUP1_BOSS_ENTRY) == 0)
                {
                    // Generate random boss entry
                    while (newPortal.bossEntry == 0 || getData(newPortal.bossEntry) == Finished)
                    {
                        newPortal.bossEntry = randomVHBossArray[Util::getRandomUInt(maxVHBosses - 1)];
                    }
                    SetInstanceData(DATA_GROUP1_BOSS_ENTRY, newPortal.bossEntry);
                }
                else
                {
                    newPortal.bossEntry = GetInstanceData(DATA_GROUP1_BOSS_ENTRY);
                }
            }

            // Second boss
            if ((currentPortalCount + 1) == 12)
            {
                if (GetInstanceData(DATA_GROUP2_BOSS_ENTRY) == 0)
                {
                    // Generate random boss entry
                    while (newPortal.bossEntry == 0 || getData(newPortal.bossEntry) == Finished)
                    {
                        newPortal.bossEntry = randomVHBossArray[Util::getRandomUInt(maxVHBosses - 1)];
                    }
                    SetInstanceData(DATA_GROUP2_BOSS_ENTRY, newPortal.bossEntry);
                }
                else
                {
                    newPortal.bossEntry = GetInstanceData(DATA_GROUP2_BOSS_ENTRY);
                }
            }
        }
    }
}

// SpawnPortal
void TheVioletHoldInstance::SpawnPortal()
{
    uint8_t portalCount = GetInstanceData(DATA_PORTAL_COUNT) + 1;
    if (portalCount > 18)
    {
        return;
    }

    GeneratePortalInfo(m_activePortal);

    float x = 0, y = 0, z = 0, o = 0;

    // Group portals
    if (m_activePortal.type != VH_PORTAL_TYPE_BOSS)
    {
        x = PortalPositions[m_activePortal.id].x;
        y = PortalPositions[m_activePortal.id].y;
        z = PortalPositions[m_activePortal.id].z;
        o = PortalPositions[m_activePortal.id].o;
    }
    // Boss portals
    else
    {
        // Last boss
        if (portalCount == 18)
        {
            // Use top edge location
            x = PortalPositions[2].x;
            y = PortalPositions[2].y;
            z = PortalPositions[2].z;
            o = PortalPositions[2].o;
        }
        // Other bosses
        else
        {
            x = SaboteurPortalLoc.x;
            y = SaboteurPortalLoc.y;
            z = SaboteurPortalLoc.z;
            o = SaboteurPortalLoc.o;
        }
    }

    SetInstanceData(DATA_PORTAL_COUNT, portalCount);
    SetInstanceData(DATA_PERVIOUS_PORTAL_ID, m_activePortal.id);

    if (!spawnCreature(CN_PORTAL, x, y, z, o))
    {
        LOG_ERROR("Violet Hold: error spawning main event portal");
    }

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
        for (std::vector<uint32_t>::iterator itr = m_eventSpawns.begin(); itr != m_eventSpawns.end(); itr = m_eventSpawns.erase(itr))
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
    setGameObjectStateForEntry(GO_PRISON_SEAL, GO_STATE_OPEN);

    // Return sinclari to spawn location
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
        setGameObjectStateForEntry(GO_EREKEM_CELL, GO_STATE_CLOSED);
        setGameObjectStateForEntry(GO_EREKEM_GUARD_CELL1, GO_STATE_CLOSED);
        setGameObjectStateForEntry(GO_EREKEM_GUARD_CELL2, GO_STATE_CLOSED);
    }

    if (GetInstanceData(INDEX_ZURAMAT) != Finished)
    {
        setGameObjectStateForEntry(GO_ZURAMAT_CELL, GO_STATE_CLOSED);
    }

    if (GetInstanceData(INDEX_MORAGG) != Finished)
    {
        setGameObjectStateForEntry(GO_MORAGG_CELL, GO_STATE_CLOSED);
    }

    if (GetInstanceData(INDEX_ICHORON) != Finished)
    {
        setGameObjectStateForEntry(GO_ICHORON_CELL, GO_STATE_CLOSED);
    }

    if (GetInstanceData(INDEX_LAVANTHOR) != Finished)
    {
        setGameObjectStateForEntry(GO_LAVANTHOR_CELL, GO_STATE_CLOSED);
    }

    if (GetInstanceData(INDEX_XEVOZZ) != Finished)
    {
        setGameObjectStateForEntry(GO_XEVOZZ_CELL, GO_STATE_CLOSED);
    }

    // Do base stuff
    SpawnIntro();
    ResetCrystals(false);
}

void TheVioletHoldInstance::RemoveIntroNpcs(bool deadOnly)
{
    if (!m_introSpawns.empty())
    {
        for (std::vector<uint32_t>::iterator itr = m_introSpawns.begin(); itr != m_introSpawns.end();)
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
        for (std::vector<uint32_t>::iterator itr = m_introSpawns.begin(); itr != m_introSpawns.end(); ++itr)
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
    UpdateInstanceWorldState(WORLD_STATE_VH_PORTAL_COUNT, GetInstanceData(DATA_PORTAL_COUNT));
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
        if (!spawnCreature(CN_VIOLET_HOLD_GUARD, guardsSpawnLoc[i].x, guardsSpawnLoc[i].y, guardsSpawnLoc[i].z, guardsSpawnLoc[i].o))
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

void TheVioletHoldInstance::ResetCrystals(bool setSelectable)
{
    for (std::vector<uint32_t>::iterator itr = m_crystalGuids.begin(); itr != m_crystalGuids.end(); ++itr)
    {
        if (GameObject* pCrystal = GetInstance()->GetGameObject(*itr))
        {
            pCrystal->SetState(GO_STATE_CLOSED);
            if (!setSelectable)
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

    for (std::vector<uint32_t>::iterator itr = m_guardsGuids.begin(); itr != m_guardsGuids.end(); itr = m_guardsGuids.erase(itr))
    {
        if (Creature* pGuard = GetInstance()->GetCreature(*itr))
        {
            if (pGuard->isAlive())
            {
                pGuard->GetAIInterface()->setSplineRun();
                pGuard->GetAIInterface()->MoveTo(introMoveLoc.x, introMoveLoc.y, introMoveLoc.z);
                pGuard->Despawn(4500, 0);
            }
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

void TheVioletHoldInstance::OnPlayerEnter(Player* plr)
{
    UpdateWorldStates();
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
}

void TheVioletHoldInstance::OnGameObjectPushToWorld(GameObject* pGo)
{
    if (pGo->GetEntry() == GO_ACTIVATION_CRYSTAL)
    {
        m_crystalGuids.push_back(pGo->GetLowGUID());
    }
}

void TheVioletHoldInstance::OnGameObjectActivate(GameObject* pGo, Player* plr)
{
    if (pGo->GetEntry() == GO_ACTIVATION_CRYSTAL)
    {
        // Mark achiev as failed
        if (GetInstance()->iInstanceMode == MODE_HEROIC)
        {
            SetInstanceData(INDEX_ACHIEV_DEFENSELESS, State_Failed);
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
        }break;
        case CN_LIEUTNANT_SINCLARI:
        {
            m_sinclariGUID = GET_LOWGUID_PART(pCreature->GetGUID());
        }break;
        case CN_PORTAL_INTRO:
        case CN_INTRO_AZURE_BINDER_ARCANE:
        case CN_INTRO_AZURE_INVADER_ARMS:
        case CN_INTRO_AZURE_MAGE_SLAYER_MELEE:
        case CN_INTRO_AZURE_SPELLBREAKER_ARCANE:
        {
            m_introSpawns.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
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
        case CN_EREKEM:
        {
            m_ErekemGUID = GET_LOWGUID_PART(pCreature->GetGUID());
        }break;
        default:
            break;
    }
}

void TheVioletHoldInstance::OnCreatureDeath(Creature* pCreature, Unit* /*pKiller*/)
{
    switch (pCreature->GetEntry())
    {
        case CN_ZURAMAT:
        {
            if (GetInstanceData(INDEX_ACHIEV_VOID_DANCE) != State_Failed)
            {
                UpdateAchievCriteriaForPlayers(ACHIEV_CRIT_VOID_DANCE, 1);
            }
            setData(pCreature->GetEntry(), Finished);
        }break;
        case CN_VOID_SENTRY:
        {
            SetInstanceData(INDEX_ACHIEV_VOID_DANCE, State_Failed);
        }break;
        case CN_CYANIGOSA:
        {
            if (GetInstance()->iInstanceMode == MODE_HEROIC && GetInstanceData(INDEX_ACHIEV_DEFENSELESS) != State_Failed)
            {
                UpdateAchievCriteriaForPlayers(ACHIEV_CRIT_DEFENSELES, 1);
            }
            SetInstanceData(INDEX_INSTANCE_PROGRESS, Finished);
        }break;
        case CN_VIOLET_HOLD_GUARD:
        {
            sEventMgr.RemoveEvents(pCreature);
            pCreature->Despawn(2000, 0);
            for (std::vector<uint32>::iterator itr = m_guardsGuids.begin(); itr != m_guardsGuids.end();)
            {
                if ((*itr) == GET_LOWGUID_PART(pCreature->GetGUID()))
                {
                    m_guardsGuids.erase(itr);
                    break;
                }
                ++itr;
            }
            spawnCreature(CN_VIOLET_HOLD_GUARD, pCreature->GetSpawnX(), pCreature->GetSpawnY(), pCreature->GetSpawnZ(), pCreature->GetSpawnO());
            // Modify corpse despawn event to make it well suitible
            sEventMgr.AddEvent(pCreature, &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }break;
        case CN_MORAGG:
        {
            SetInstanceData(INDEX_MORAGG, Finished);
            SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
        }break;
        case CN_ICHORON:
        {
            SetInstanceData(INDEX_ICHORON, Finished);
            SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
        }break;
        case CN_XEVOZZ:
        {
            SetInstanceData(INDEX_XEVOZZ, Finished);
            SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
        }break;
        case CN_LAVANTHOR:
        {
            SetInstanceData(INDEX_LAVANTHOR, Finished);
            SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
        }break;
        case CN_EREKEM:
        {
            SetInstanceData(INDEX_EREKEM, Finished);
            SetInstanceData(INDEX_PORTAL_PROGRESS, Finished);
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
        case CN_VETERAN_MAGE_HUNTER:
        {
            if (m_activePortal.type == VH_PORTAL_TYPE_SQUAD && GetInstanceData(INDEX_PORTAL_PROGRESS) == InProgress)
            {
                m_activePortal.RemoveSummonByGuid(GET_LOWGUID_PART(pCreature->GetGUID()));
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
        }break;
    }
}

void TheVioletHoldInstance::UpdateEvent()
{
    if (GetInstanceData(INDEX_INSTANCE_PROGRESS) == InProgress && GetInstanceData(INDEX_PORTAL_PROGRESS) == NotStarted)
    {
        if (isTimerFinished(m_portalSummonTimer))
        {
            SpawnPortal();
            SetInstanceData(INDEX_PORTAL_PROGRESS, InProgress);
            // Timer is resetted in SetInstanceData
        }
    }
}

void SetupTheVioletHold(ScriptMgr* mgr)
{
    // Sinclari gossip/escort event
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
    mgr->register_creature_script(CN_VETERAN_MAGE_HUNTER, &VHAttackerAI::Create);
    mgr->register_creature_script(CN_AZURE_SABOTEUR, &AzureSaboteurAI::Create);

    // Bosses
    mgr->register_creature_script(CN_MORAGG, &MoraggAI::Create);
    mgr->register_creature_script(CN_ICHORON, &IchoronAI::Create);

    mgr->register_creature_script(CN_ZURAMAT, &ZuramatAI::Create);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_AURA_REMOVE, (void*)OnRemoveVoidShift);
    mgr->register_creature_script(CN_LAVANTHOR, &LavanthorAI::Create);
    mgr->register_creature_script(CN_EREKEM, &ErekemAI::Create);
    mgr->register_creature_script(CN_XEVOZZ, &XevozzAI::Create);

    // Instance
    mgr->register_instance_script(MAP_VIOLET_HOLD, &TheVioletHoldInstance::Create);

    // Spells
    mgr->register_script_effect(SPELL_VH_TELEPORT_PLAYER, &TeleportPlayerInEffect);
}
