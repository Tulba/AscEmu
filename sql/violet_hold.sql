-- Update ichoron's gates
UPDATE gameobject_spawns SET orientation3 = 0.928246, orientation4 = 0.371966 WHERE entry = 191722 AND map = 608;

-- Removed not needed npcs (they're spawned by script)
delete from creature_spawns where entry in (31011, 31010, 31009, 31008, 31007, 30659, 30658) and map = 608;

-- Update guards faction
update creature_properties set faction = 1718 where entry = 30659;

-- Worldstate entries
delete from worldstate_templates where map = 608;
insert into `worldstate_templates` (`map`, `zone`, `field`, `value`) values
('608','0','3810','0'),
('608','0','3815','100'),
('608','0','3816','0');

-- Update repop location
update worldmap_info set repopx = '5847.220215', repopy = '761.578979', repopz = '640.950745' where entry = 608;

-- Make prisoners cells closed
update gameobject_spawns set state = 1 where entry in (191556, 191566, 191722, 191565, 191564, 191563, 191562, 191606);
