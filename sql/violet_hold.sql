-- Update ichoron's gates
UPDATE gameobject_spawns SET orientation3 = 0.928246, orientation4 = 0.371966 WHERE entry = 191722 AND map = 608;

-- Removed not needed npcs (they're spawned by script)
delete from creature_spawns where entry in (31011, 31010, 31009, 31008, 31007, 30659, 30658) and map = 608;

-- Update guards faction
update creature_properties set faction = 1718 where entry = 30659;

-- Remove worldstate entries, thei're handled via instance script
delete from worldstate_templates where map = 608;

-- Update repop location
update worldmap_info set repopx = '5847.220215', repopy = '761.578979', repopz = '640.950745' where entry = 608;

-- Make prisoners cells closed
update gameobject_spawns set state = 1 where entry in (191556, 191566, 191722, 191565, 191564, 191563, 191562, 191606);

-- update outside trigger position
update gameobject_spawns set orientation3=0.430511, orientation4 = 0.902586 where entry=193608;

-- Add welcome message
update worldmap_info set flags = flags | 2 where entry=608;

-- Fixed invisible creatures in heroic mode
UPDATE `creature_properties` SET `invisibility_type` = '0' WHERE `entry` in (
    -- guards
    31505, 
    -- intro npcs
    31499, 31485, 31489, 31496,
    -- portal guardians
    31487, 31494, 31483, 31497, 31486, 31493, 31490, 32192,
    -- bosses and their adds
    31512, 31518,   -- Zuramat
    31507, 32549,   -- Erekem
    31510,          -- Moragg
    31508,          -- Ichonor (TODO: adds)
    31511,          -- Xevozz (TODO: adds)
    31509,          -- Lavanthor
    31134           -- Cyanigosa
);
