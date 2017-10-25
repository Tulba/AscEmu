-- Update ichoron's gates
UPDATE gameobject_spawns SET orientation3 = 0.928246, orientation4 = 0.371966 WHERE entry = 191722 AND map = 608;

-- Removed not needed npcs (they're spawned by script)
delete from creature_spawns where entry in (31011, 31010, 31009, 31008, 31007, 30659) and map = 608;

-- Update guards faction
update creature_properties set faction = 1718 where entry = 30659;
