/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2008 WEmu Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"

class SpiritScreeches : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr)
    {
        if (!plr)
            return;

        GossipMenu* Menu;
        Creature* spirit = static_cast<Creature*>(pObject);
        if (spirit == NULL)
            return;

        if (plr->HasQuest(3520))
        {
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2039, plr);
            Menu->AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(495), 1);     // Goodbye

            Menu->SendTo(plr);
        }
    }

    void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* EnteredCode)
    {
        if (!plr)
            return;

        Creature* spirit = static_cast<Creature*>(pObject);
        if (spirit == NULL)
            return;

        switch (IntId)
        {
            case 0:
                GossipHello(pObject, plr);
                break;

            case 1:
            {
                QuestLogEntry* en = plr->GetQuestLogForEntry(3520);
                if (en && en->GetMobCount(0) < en->GetQuest()->required_mob_or_go_count[0])
                {
                    en->SetMobCount(0, en->GetMobCount(0) + 1);
                    en->SendUpdateAddKill(0);
                    en->UpdatePlayerFields();
                }

                spirit->Despawn(1, 0);
                return;

            }
        }
    }

};

class ScreecherSpirit : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(ScreecherSpirit);

    ScreecherSpirit(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        if (!_unit)
            return;

        Creature* cialo = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 5307);
        if (!cialo)
            return;

        if (!cialo->isAlive())
            cialo->Despawn(1, 6 * 60 * 1000);

        _unit->Despawn(60 * 1000, 0);
    }
};

class StewardOfTime : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr)
    {
        GossipMenu* Menu;
        if (plr->HasQuest(10279) || plr->HasFinishedQuest(10279))
        {
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9978, plr);
            Menu->AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(496), 1);     // Please take me to the Master's Lair
            Menu->SendTo(plr);
        }
    }

    void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
    {
        Creature* creat = static_cast<Creature*>(pObject);
        switch (IntId)
        {
            case 1:
                creat->CastSpell(plr, sSpellCustomizations.GetSpellInfo(34891), true);
                break;
        }
    }
};


void SetupTanaris(ScriptMgr* mgr)
{
    GossipScript* Screeches = new SpiritScreeches();
    mgr->register_gossip_script(8612, Screeches);

    mgr->register_creature_script(8612, &ScreecherSpirit::Create);

    GossipScript* StewardOfTimeGossip = new StewardOfTime();
    mgr->register_gossip_script(20142, StewardOfTimeGossip);
}
