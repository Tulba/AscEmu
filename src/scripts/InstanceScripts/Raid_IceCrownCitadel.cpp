/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_IceCrownCitadel.h"
#include "Objects/Faction.h"

//////////////////////////////////////////////////////////////////////////////////////////
//ICC zone: 4812
//Prepared creature entry:
//
//CN_DEATHBRINGER_SAURFANG    37813
//CN_FESTERGUT                36626
//CN_ROTFACE                  36627
//CN_PROFESSOR_PUTRICIDE      36678
//CN_PRINCE_VALANAR           37970
//N_BLOOD_QUEEN_LANATHEL     37955
//CN_SINDRAGOSA               36853
//CN_THE_LICHKING             36597
//
///\todo  start boss scripts
//////////////////////////////////////////////////////////////////////////////////////////
//Event: GunshipBattle
//
//Affects:
//Available teleports. If GunshipBattle done -> Teleportlocation 4 available.
//
//Devnotes:
//Far away from implementing this :(
//////////////////////////////////////////////////////////////////////////////////////////
enum IceCrown_Encounters
{
    DATA_LORD_MARROWGAR,
    DATA_COLDFLAME,
    DATA_BONE_SPIKE,
    DATA_LADY_DEATHWHISPER,
    DATA_VALITHRIA_DREAM,

    ICC_DATA_END
};


//////////////////////////////////////////////////////////////////////////////////////////
//IceCrownCitadel Instance
class IceCrownCitadelScript : public InstanceScript
{
    public:

        IceCrownCitadelScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {
            if (getData(CN_LORD_MARROWGAR) == Finished)
            {
                setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_1, GO_STATE_CLOSED);    // Icewall 1
                setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_2, GO_STATE_CLOSED);    // Icewall 2
                setGameObjectStateForEntry(GO_MARROWGAR_DOOR, GO_STATE_CLOSED);         // Door
            }

            //test timers
            addTimer(75000);
            addTimer(5000);
            addTimer(35000);
        }

        static InstanceScript* Create(MapMgr* pMapMgr) { return new IceCrownCitadelScript(pMapMgr); }

        void UpdateEvent()
        {
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject)
        {
            // Gos which are not visible by killing a boss needs a second check...
            if (getData(CN_LORD_MARROWGAR) == Finished)
            {
                setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_1, GO_STATE_OPEN);    // Icewall 1
                setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_2, GO_STATE_OPEN);    // Icewall 2
                setGameObjectStateForEntry(GO_MARROWGAR_DOOR, GO_STATE_OPEN);         // Door
            }

            switch (pGameObject->GetEntry())
            {
                case GO_TELE_1:
                case GO_TELE_2:
                case GO_TELE_3:
                case GO_TELE_4:
                case GO_TELE_5:
                {
                    pGameObject->SetFlags(0);
                } break;
            }
        }

        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
        {
            switch (pCreature->GetEntry())
            {
                case CN_LORD_MARROWGAR:
                {
                    setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_1, GO_STATE_OPEN);    // Icewall 1
                    setGameObjectStateForEntry(GO_MARROWGAR_ICEWALL_2, GO_STATE_OPEN);    // Icewall 2
                    setGameObjectStateForEntry(GO_MARROWGAR_DOOR, GO_STATE_OPEN);         // Door
                }break;
                default:
                    break;
            }
        }

        void OnPlayerEnter(Player* player)
        {
            if (!spawnsCreated())
            {
                // setup only the npcs with the correct team...
                switch (player->GetTeam())
                {
                    case TEAM_ALLIANCE:
                        for (uint8 i = 0; i < 13; i++)
                            spawnCreature(AllySpawns[i].entry, AllySpawns[i].x, AllySpawns[i].y, AllySpawns[i].z, AllySpawns[i].o, AllySpawns[i].faction);
                        break;
                    case TEAM_HORDE:
                        for (uint8 i = 0; i < 13; i++)
                            spawnCreature(HordeSpawns[i].entry, HordeSpawns[i].x, HordeSpawns[i].y, HordeSpawns[i].z, HordeSpawns[i].o, HordeSpawns[i].faction);
                        break;
                }

                setSpawnsCreated();
            }
        }

};

//////////////////////////////////////////////////////////////////////////////////////////
// IceCrown Teleporter
class ICCTeleporterGossip : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* object, Player* player)
    {
        InstanceScript* pInstance = player->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        Arcemu::Gossip::Menu menu(object->GetGUID(), 15221, player->GetSession()->language);
        menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(515), 0);     // Teleport to Light's Hammer.

        if (pInstance->getData(CN_LORD_MARROWGAR) == Finished)
            menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(516), 1);      // Teleport to Oratory of The Damned.

        if (pInstance->getData(CN_LADY_DEATHWHISPER) == Finished)
            menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(517), 2);      // Teleport to Rampart of Skulls.

        // GunshipBattle has to be finished...
        //menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(518), 3);        // Teleport to Deathbringer's Rise.

        if (pInstance->getData(CN_VALITHRIA_DREAMWALKER) == Finished)
            menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(519), 4);      // Teleport to the Upper Spire.

        if (pInstance->getData(CN_COLDFLAME) == Finished)
            menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(520), 5);      // Teleport to Sindragosa's Lair.

        menu.Send(player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 Id, const char* enteredcode, uint32 gossipId)
    {
        switch (Id)
        {
            case 0:
                player->CastSpell(player, 70781, true);     // Light's Hammer
                break;
            case 1:
                player->CastSpell(player, 70856, true);     // Oratory of The Damned
                break;
            case 2:
                player->CastSpell(player, 70857, true);     // Rampart of Skulls
                break;
            case 3:
                player->CastSpell(player, 70858, true);     // Deathbringer's Rise
                break;
            case 4:
                player->CastSpell(player, 70859, true);     // Upper Spire
                break;
            case 5:
                player->CastSpell(player, 70861, true);     // Sindragosa's Lair
                break;
        }
        Arcemu::Gossip::Menu::Complete(player);
    }
};

class ICCTeleporterAI : public GameObjectAIScript
{
public:

    ICCTeleporterAI(GameObject* go) : GameObjectAIScript(go) {}

    ~ICCTeleporterAI() {}

    static GameObjectAIScript* Create(GameObject* go) { return new ICCTeleporterAI(go); }

    void OnActivate(Player* player)
    {
        ICCTeleporterGossip gossip;
        gossip.OnHello(_gameobject, player);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss: Lord Marrowgar
const uint32 LM_BERSERK = 47008;
const uint32 BONE_SLICE = 69055;
const uint32 BONE_SPIKE = 69057;       // Not shure about this
const uint32 BONE_STORM = 69076;
const uint32 SOUL_FEAST = 71203;       // Needs a script

class LordMarrowgarAI : public CreatureAIScript
{
    public:

        /*bool m_spellcheck[4];
        SP_AI_Spell spells[4];*/

        static CreatureAIScript* Create(Creature* c) { return new LordMarrowgarAI(c); }
        LordMarrowgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            sendDBChatMessage(922);      // This is the beginning AND the end, mortals. None may enter the master's sanctum!

            /*nrspells = 4;*/

            /*spells[0].info = sSpellCustomizations.GetSpellInfo(BONE_SLICE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 10000;*/

            /*SpellDesc* boneslice = AddSpell(BONE_SLICE, Target_Current, 60.0f, 0.0f, 15);
            boneslice->AddEmote("boneslice", CHAT_MSG_MONSTER_YELL, 0);*/

            /*spells[1].info = sSpellCustomizations.GetSpellInfo(BONE_STORM);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 60000;
            spells[1].perctrigger = 75.0f;
            spells[1].attackstoptimer = 60000;*/

            SpellDesc* bonestorm = AddSpell(BONE_STORM, Target_RandomDestination, 75.0f, 0.0f, 60);
            bonestorm->AddEmote("bonestorm", CHAT_MSG_MONSTER_YELL, 0);

            /*spells[2].info = sSpellCustomizations.GetSpellInfo(LM_BERSERK);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 50.0f;
            spells[2].attackstoptimer = 10000;*/

            /*SpellDesc* berserk = AddSpell(LM_BERSERK, Target_Current, 50.0f, 0.0f, 15);
            berserk->AddEmote("berserk", CHAT_MSG_MONSTER_YELL, 0);*/

            //spells[3].info = sSpellCustomizations.GetSpellInfo(SOUL_FEAST);
            //spells[3].targettype = TARGET_RANDOM_SINGLE;
            //spells[3].instant = true;
            //spells[3].cooldown = 20;
            //spells[3].perctrigger = 50.0f;
            //spells[3].attackstoptimer = 12000;

            /*SpellDesc* souldFest = AddSpell(SOUL_FEAST, Target_RandomPlayer, 50.0f, 0.0f, 20);
            souldFest->AddEmote("Your soul is fest", CHAT_MSG_MONSTER_YELL, 0);*/
            /* Testcode - remove me please
            exampleTimer1 = 0;
            exampleTimer2 = 0;
            exampleTimer3 = 0;*/

            /* Testcode - remove me please*/
            /*SpellDesc* WhirlTemp = AddSpell(36175, Target_Self, 50.0f, 0.0f, 60);
            WhirlTemp->AddEmote("Reap the Whirlwind!", CHAT_MSG_MONSTER_YELL, 11089);
            WhirlTemp->AddEmote("I'll cut you to peices!", CHAT_MSG_MONSTER_YELL, 11090);*/
        }

        void AIUpdate()
        {
            /*switch (RandomUInt(1))
            {
                case 0:
                {
                    float val = RandomFloat(100.0f);
                    SpellCast(val);
                }break;
                case 1:
                    BoneSpike();
                    break;
            }*/
        }

        /* Testcode - remove me please
        void OnScriptPhaseChange(uint32_t scriptPhase)
        {
            std::stringstream ss;
            ss << "My scriptPhase is now " << scriptPhase;

            sendAnnouncement(ss.str());
        }*/

        void OnCombatStart(Unit* pTarget)
        {
            sendDBChatMessage(923);      // The Scourge will wash over this world as a swarm of death and destruction!
            RegisterAIUpdateEvent(400);

            /* Testcode - remove me please
            exampleTimer1 = _addTimer(30000);
            exampleTimer2 = _addTimer(120000);

            if (pTarget->IsPlayer())
            {
                static_cast<Player*>(pTarget)->BroadcastMessage("Morrowgar Timer 1 = %u", _getTimeForTimer(exampleTimer1));
                static_cast<Player*>(pTarget)->BroadcastMessage("Morrowgar Timer 2 = %u", _getTimeForTimer(exampleTimer2));
            }*/
        }

        //void BoneSpike()
        //{
        //    switch (RandomUInt(2))
        //    {
        //        case 0:
        //            sendDBChatMessage(925);      // Bound by bone!
        //            break;
        //        case 1:
        //            sendDBChatMessage(926);      // Stick around!
        //            break;
        //        case 2:
        //            sendDBChatMessage(927);      // The only escape is death!
        //            break;
        //    }

        //    std::vector<Player*> TargetTable;
        //    std::set<Object*>::iterator itr = getCreature()->GetInRangePlayerSetBegin();

        //    for (; itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
        //    {
        //        if (isHostile(getCreature(), (*itr)))
        //        {
        //            Player* RandomTarget = NULL;
        //            RandomTarget = static_cast<Player*>(*itr);
        //            if (RandomTarget && RandomTarget->isAlive() && isHostile(getCreature(), RandomTarget))
        //                TargetTable.push_back(RandomTarget);
        //        }
        //    }

        //    if (!TargetTable.size())
        //        return;

        //    auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
        //    auto random_target = TargetTable[random_index];

        //    if (random_target == nullptr)
        //        return;

        //    getCreature()->CastSpell(random_target, sSpellCustomizations.GetSpellInfo(BONE_SPIKE), false);

        //    TargetTable.clear();

        //    spawnCreature(CN_BONE_SPIKE, random_target->GetPosition());

        //    TargetTable.clear();
        //}

        void OnTargetDied(Unit* pTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(928);      // More bones for the offering!
                    break;
                case 1:
                    sendDBChatMessage(929);      // Languish in damnation!
                    break;
            }
        }

        void OnDied(Unit* pTarget)
        {
            sendDBChatMessage(930);      // I see... Only darkness.

            /* Testcode - remove me please
            _cancelAllTimers();*/
        }

        //void SpellCast(float val)
        //{
        //    if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        //    {
        //        float comulativeperc = 0;
        //        Unit* target = NULL;
        //        for (uint8 i = 0; i < nrspells; i++)
        //        {
        //            if (!spells[i].perctrigger)
        //                continue;

        //            if (m_spellcheck[i])
        //            {
        //                target = getCreature()->GetAIInterface()->getNextTarget();
        //                switch (spells[i].targettype)
        //                {
        //                    case TARGET_SELF:
        //                    case TARGET_VARIOUS:
        //                        getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
        //                        break;
        //                    case TARGET_RANDOM_SINGLE:
        //                    case TARGET_ATTACKING:
        //                        getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
        //                        break;
        //                    case TARGET_DESTINATION:
        //                        getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
        //                        break;
        //                }
        //                m_spellcheck[i] = false;
        //                return;
        //            }

        //            if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
        //            {
        //                getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
        //                m_spellcheck[i] = true;
        //            }
        //            comulativeperc += spells[i].perctrigger;
        //        }

        //        RemoveAIUpdateEvent();
        //        RegisterAIUpdateEvent(50000);

        //        /* Testcode - remove me please
        //        exampleTimer3 = _addTimer(50000);*/
        //    }
        //}

    protected:

        uint8 nrspells;

        /* Testcode - remove me please
        uint32_t exampleTimer1;
        uint32_t exampleTimer2;
        uint32_t exampleTimer3;*/
};

const uint32 IMPALED = 69065;

class BoneSpikeAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(BoneSpikeAI, MoonScriptCreatureAI);
        BoneSpikeAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);  // On wowhead they said "kill them not just looking at them".
            getCreature()->Despawn(8000, 0);
        }

};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss: Lady Deathwhisper
// MANA_BARRIER = 70842
// DEATH_AND_DECAY = 71001
// TOUCH_OF_INSIGNIFICANCE = 71204
// SHADOW_BOLT = 71254
// DOMINATE_MIND_H = 71289
// SUMMON_SHADE = 71363
// FROSTBOLT = 71420
// FROSTBOLT_VOLLEY = 72905
// ...

///////////////////////////////////////////////////////
// Boss: Valithria Dreamwalker
// ...
//
//
//
//

///////////////////////////////////////////////////////
// Boss: Cold Flame
// ...
//
//
//
//

void SetupICC(ScriptMgr* mgr)
{
#ifndef UseNewMapScriptsProject
    //Instance
    mgr->register_instance_script(MAP_ICECROWNCITADEL, &IceCrownCitadelScript::Create);

    //Teleporters
    mgr->register_gameobject_script(GO_TELE_1, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_1, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_2, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_2, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_3, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_3, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_4, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_4, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_5, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_5, new ICCTeleporterGossip());

    //Bosses
    mgr->register_creature_script(CN_LORD_MARROWGAR, &LordMarrowgarAI::Create);
    //mgr->register_creature_script(CN_LADY_DEATHWHISPER, &LadyDeathwhisperAI::Create);
    //mgr->register_creature_script(CN_VALITHRIA_DREAMWALKER, &ValithriaDreamwalkerAI::Create);
    //mgr->register_creature_script(CN_COLDFLAME, &ColdFlameAI::Create);

    //Misc
    mgr->register_creature_script(CN_BONE_SPIKE, &BoneSpikeAI::Create);
#endif
}
