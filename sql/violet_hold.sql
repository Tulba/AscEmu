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
UPDATE creature_properties SET minlevel=81, maxlevel=81, faction=1720 WHERE entry IN (SELECT difficulty_1 FROM creature_difficulty WHERE entry IN(30695, 30660));

-- update Cyanigosa stats
update creature_properties set minlevel=77, maxlevel=77, faction=16 where entry = 31134;
UPDATE creature_properties SET minlevel=82, maxlevel=82, faction=16 WHERE entry IN (SELECT difficulty_1 FROM creature_difficulty WHERE entry=31134);

-- Remove Xevozz npc_monstersay data
delete from npc_monstersay where entry = 31511;

-- Portal keeper/guardian texts
delete from `npc_script_text` where `entry` between 8946 and 8956;
insert into `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`) values
('8947','I will defend this portal with my life!','0','0','14','0','100','0','0','0','0'),
('8948','More portals will take this one\'s place!','0','0','14','0','100','0','0','0','0'),
('8949','My death will not stop this invasion!','0','0','14','0','100','0','0','0','0'),
('8950','The destruction of Dalaran is inevitable!','0','0','14','0','100','0','0','0','0'),
('8951','The portal has stabilized! Attack!','0','0','14','0','100','0','0','0','0'),
('8952','The way into Dalaran has been opened!','0','0','14','0','100','0','0','0','0'),
('8953','Why do you defend the Kirin Tor...','0','0','14','0','100','0','0','0','0'),
('8954','You shall not disrupt this portal!','0','0','14','0','100','0','0','0','0'),
('8955','Your pathetic defense ends here!','0','0','14','0','100','0','0','0','0');

-- update azure spellbreaker level
UPDATE `creature_properties` SET `minlevel` = '75' , `maxlevel` = '75' WHERE `entry` = '30662'; 
UPDATE creature_properties SET minlevel=80, maxlevel=80 WHERE entry IN (SELECT difficulty_1 FROM creature_difficulty WHERE entry=30662);

-- update azure saboteur level
UPDATE `creature_properties` SET `minlevel` = '75' , `maxlevel` = '75' WHERE `entry` = '31079'; 
UPDATE creature_properties SET minlevel=80, maxlevel=80 WHERE entry IN (SELECT difficulty_1 FROM creature_difficulty WHERE entry=31079);

-- update azure mage slayer level
UPDATE `creature_properties` SET `minlevel` = '75' , `maxlevel` = '75' WHERE `entry` = '30664'; 
UPDATE creature_properties SET minlevel=80, maxlevel=80 WHERE entry IN (SELECT difficulty_1 FROM creature_difficulty WHERE entry=30664);

-- Attackers texts
delete from `npc_script_text` where `entry` between 8955 and 8963;
insert into `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`) values
('8956','Dalaran must fall!','0','0','12','0','100','0','0','0','0'),
('8957','Destroy all who stand against us!','0','0','12','0','100','0','0','0','0'),
('8958','For the Spellweaver!','0','0','12','0','100','0','0','0','0'),
('8959','Magic must be... contained...','0','0','12','0','100','0','0','0','0'),
('8960','The Kirin Tor must be stopped!','0','0','12','0','100','0','0','0','0'),
('8961','The Nexus War will not be stopped!','0','0','12','0','100','0','0','0','0'),
('8962','You cannot stop us all!','0','0','12','0','100','0','0','0','0');
