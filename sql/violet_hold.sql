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

UPDATE `creature_properties` SET `invisibility_type` = '0' WHERE `entry` in (
SELECT difficulty_1 FROM creature_difficulty WHERE entry in (
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
));

-- Added missing Sinclari's script texts
delete from npc_script_text where entry in (8797, 8798);
insert into `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`) values
('8797','I\'m locking the door. Good luck, and thank you for doing this.','30658','0','12','0','100','0','0','0','0'),
('8798','You did it! You held the Blue Dragonflight back and defeated their commander. Amazing work!','30658','0','12','0','100','0','0','0','0');

-- Update Azure Mage Slayer faction, perviously was 35 (friendly)
update creature_properties set faction=1720 where entry in (30664, 31497);

-- Delete unneeded target for spell 58040
DELETE FROM `spelltargetconstraints` WHERE `SpellID` = '58040';

-- Increased view distance
UPDATE `worldmap_info` SET `viewingDistance` = '1000' WHERE `entry` = '608'; 

-- Update prison seal invisibility, base invisibility is hack done in script
UPDATE `creature_properties` SET `invisibility_type` = '0' WHERE `entry` = 30896;
UPDATE creature_properties SET `invisibility_type` = '0' WHERE entry IN (SELECT difficulty_1 FROM creature_difficulty WHERE entry = 30896);

-- Update keeper/guardian stats
update creature_properties set minlevel=76, maxlevel=76, faction=1720 where entry in (30695, 30660);
UPDATE creature_properties SET minlevel=76, maxlevel=76, faction=1720 WHERE entry IN (SELECT difficulty_1 FROM creature_difficulty WHERE entry IN(30695, 30660));

