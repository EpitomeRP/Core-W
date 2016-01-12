ALTER TABLE `characters`.`characters` 
ADD COLUMN `displayid` INT(10) UNSIGNED NULL DEFAULT 0 AFTER `deleteDate`,
ADD COLUMN `objectscale` FLOAT UNSIGNED NOT NULL DEFAULT 1 AFTER `displayid`;
