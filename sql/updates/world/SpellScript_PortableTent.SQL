-- This is the only required script name for ALL portable objects. --
INSERT INTO `world`.`spell_script_names` (`spell_id`, `ScriptName`) VALUES ('90018', 'SpellScript_PortableTent');

ALTER TABLE `world`.`gameobject` 
ADD COLUMN `owner` INT(10) UNSIGNED NULL DEFAULT '0' AFTER `size`;

UPDATE world.gameobject SET owner=0;

ALTER TABLE `world`.`gameobject`   
  ADD COLUMN `creationtime` BIGINT(10) DEFAULT 0 NULL AFTER `owner`;
