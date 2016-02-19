INSERT INTO `world`.`spell_script_names` (`spell_id`, `ScriptName`) VALUES ('90018', 'SpellScript_PortableTent');
INSERT INTO `world`.`spell_script_names` (`spell_id`, `ScriptName`) VALUES ('90019', 'SpellScript_PortableBench');

ALTER TABLE `world`.`gameobject` 
ADD COLUMN `owner` INT(10) UNSIGNED NULL DEFAULT '0' AFTER `size`;

UPDATE world.gameobject SET owner=0;