/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/Gossip.h"
#include "Server/Script/ScriptMgr.h"

class MasterHammersmith : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7245);
            menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_HAMMERSMITH_LEARN), 1);
            menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_HAMMERSMITH_UNLEARN), 2);

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            uint32 textid = 0;
            if (1 == Id)
            {
                if (!plr->_HasSkillLine(164) || plr->_GetSkillLineCurrent(164, false) < 300)
                    textid = 20001;
                else if (!plr->HasSpell(9787))
                    textid = 20002;
                else if (plr->HasSpell(17040))
                    textid = 20003;
                else if (plr->HasSpell(17041) || plr->HasSpell(17039) || plr->HasSpell(9788))
                    textid = 20004;
                else
                {
                    if (!plr->HasGold(600))
                        textid = 20005;
                    else
                    {
                        //pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Make good use of this knowledge." );
                        textid = 20006;
                        static_cast<Creature*>(pObject)->CastSpell(plr, 39099, true);
                        plr->ModGold(-600);
                    }
                }
            }
            else
            {
                if (!plr->HasSpell(17040))
                    textid = 20007;

                else if ((!plr->HasGold(250000) && plr->getLevel() <= 50) || (!plr->HasGold(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65)
                         || (!plr->HasGold(1000000) && plr->getLevel() > 65))
                         textid = 20008;

                else
                {
                    int32 unlearnGold = 0;
                    if (plr->getLevel() <= 50)
                        unlearnGold = 250000;
                    if (plr->getLevel() > 50 && plr->getLevel() <= 65)
                        unlearnGold = 500000;
                    if (plr->getLevel() > 65)
                        unlearnGold = 1000000;

                    plr->ModGold(-unlearnGold);
                    plr->removeSpell(17040, false, false, 0);
                    textid = 20009;
                }
            }
            Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), textid, plr);
        }

        void Destroy() override { delete this; }

};

class MasterSwordsmith : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7247);
            menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_SWORDSMITH_LEARN), 1);
            menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_SWORDSMITH_UNLEARN), 2);
            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            uint32 textid = 0;
            if (1 == Id)
            {
                if (!plr->_HasSkillLine(164) || plr->_GetSkillLineCurrent(164, false) < 300)
                    textid = 20001;

                else if (!plr->HasSpell(9787))
                    textid = 20002;

                else if (plr->HasSpell(17039))
                    textid = 20003;

                else if (plr->HasSpell(17041) || plr->HasSpell(17040) || plr->HasSpell(9788))
                    textid = 20004;
                else
                {
                    if (!plr->HasGold(600))
                        textid = 20005;
                    else
                    {
                        //pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Make good use of this knowledge." );
                        textid = 20006;
                        static_cast<Creature*>(pObject)->CastSpell(plr, 39097, true);
                        plr->ModGold(-600);
                    }
                }
            }
            else
            {
                if (!plr->HasSpell(17039))
                    textid = 20007;

                else if ((!plr->HasGold(250000) && plr->getLevel() <= 50) || (!plr->HasGold(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65)
                         || (!plr->HasGold(1000000) && plr->getLevel() > 65))
                         textid = 20008;
                else
                {
                    int32 unlearnGold = 0;
                    if (plr->getLevel() <= 50)
                        unlearnGold = 250000;
                    if (plr->getLevel() > 50 && plr->getLevel() <= 65)
                        unlearnGold = 500000;
                    if (plr->getLevel() > 65)
                        unlearnGold = 1000000;

                    plr->ModGold(-unlearnGold);
                    plr->removeSpell(17039, false, false, 0);
                    textid = 20009;
                }
            }
            Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), textid, plr);
        }

};

class MasterAxesmith : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7243);
            menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_AXESMITH_LEARN), 1);
            menu.AddItem(GOSSIP_ICON_TRAINER, plr->GetSession()->LocalizedGossipOption(GI_T_AXESMITH_UNLEARN), 2);
            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            uint32 textid = 0;
            if (1 == Id)
            {
                if (!plr->_HasSkillLine(164) || plr->_GetSkillLineCurrent(164, false) < 300)
                    textid = 20001;

                else if (!plr->HasSpell(9787))
                    textid = 20002;

                else if (plr->HasSpell(17041))
                    textid = 20003;

                else if (plr->HasSpell(17039) || plr->HasSpell(17040) || plr->HasSpell(9788))
                    textid = 20004;
                else
                {
                    if (!plr->HasGold(600))
                        textid = 20005;

                    else
                    {
                        //pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Make good use of this knowledge." );
                        textid = 20006;
                        static_cast<Creature*>(pObject)->CastSpell(plr, 39098, true);
                        plr->ModGold(-600);
                    }
                }
            }
            else
            {
                if (!plr->HasSpell(17041))
                    textid = 20007;

                else if ((!plr->HasGold(250000) && plr->getLevel() <= 50) || (!plr->HasGold(500000) && plr->getLevel() > 50 && plr->getLevel() <= 65)
                         || (!plr->HasGold(1000000) && plr->getLevel() > 65))
                         textid = 20008;
                else
                {
                    int32 unlearnGold = 0;
                    if (plr->getLevel() <= 50)
                        unlearnGold = 250000;
                    if (plr->getLevel() > 50 && plr->getLevel() <= 65)
                        unlearnGold = 500000;
                    if (plr->getLevel() > 65)
                        unlearnGold = 1000000;

                    plr->ModGold(-unlearnGold);
                    plr->removeSpell(17041, false, false, 0);
                    textid = 20009;
                }
            }
            Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), textid, plr);
        }

};

void SetupTrainerScript(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(11191, new MasterHammersmith);    // Lilith the Lithe
    mgr->register_creature_gossip(11193, new MasterSwordsmith);     // Seril Scourgebane
    mgr->register_creature_gossip(11192, new MasterAxesmith);       // Kilram
}