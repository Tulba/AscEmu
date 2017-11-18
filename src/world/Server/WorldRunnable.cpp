/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
 *
 */

#include "StdAfx.h"
#include "WorldRunnable.h"
#include <CrashHandler.h>
#include "World.h"
#include "World.Legacy.h"
#include "ServerState.h"

#define WORLD_UPDATE_DELAY 50

WorldRunnable::WorldRunnable() : CThread()
{
}

bool WorldRunnable::runThread()
{
    SetThreadName("WorldRunnable (non-instance/logon)");
    uint32 LastWorldUpdate = Util::getMSTime();
    uint32 LastSessionsUpdate = Util::getMSTime();

    THREAD_TRY_EXECUTION
        while (GetThreadState() != THREADSTATE_TERMINATE)
        {
            // Provision for pausing this thread.
            if (GetThreadState() == THREADSTATE_PAUSED)
            {
                while (GetThreadState() == THREADSTATE_PAUSED)
                {
                    Arcemu::Sleep(200);
                }
            }
            if (GetThreadState() == THREADSTATE_TERMINATE)
                break;

            ThreadState = THREADSTATE_BUSY;

            ServerState::instance()->update();

            uint32 diff;
            //calc time passed
            uint32 now, execution_start;
            now = Util::getMSTime();
            execution_start = now;

            if (now < LastWorldUpdate) //overrun
                diff = WORLD_UPDATE_DELAY;
            else
                diff = now - LastWorldUpdate;

            LastWorldUpdate = now;
            sWorld.Update(diff);

            now = Util::getMSTime();

            if (now < LastSessionsUpdate) //overrun
                diff = WORLD_UPDATE_DELAY;
            else
                diff = now - LastSessionsUpdate;

            LastSessionsUpdate = now;
            sWorld.updateGlobalSession(diff);

            now = Util::getMSTime();
            //we have to wait now

            if (execution_start > now)//overrun
                diff = WORLD_UPDATE_DELAY - now;

            else
                diff = now - execution_start; //time used for updating

            if (GetThreadState() == THREADSTATE_TERMINATE)
                break;

            ThreadState = THREADSTATE_SLEEPING;

            /*This is execution time compensating system
                if execution took more than default delay
                no need to make this sleep*/
            if (diff < WORLD_UPDATE_DELAY)
                Arcemu::Sleep(WORLD_UPDATE_DELAY - diff);
        }

        THREAD_HANDLE_CRASH
    return true;
}
