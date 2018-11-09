ALTER TABLE `dmh_file` CHANGE `md5` `md5` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_session` CHANGE `name` `name` VARCHAR(255) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;

ALTER TABLE `dmh_file` DROP COLUMN IDh;
ALTER TABLE `dmh_file` ADD `IDh` VARCHAR(36) AS (insert( insert( insert( insert(hex(ID),9,0,'-'), 14,0,'-'), 19,0,'-'), 24,0,'-')) VIRTUAL AFTER `ID`;
ALTER TABLE `dmh_area` DROP COLUMN IDh;
ALTER TABLE `dmh_area` ADD `IDh` VARCHAR(36) AS (insert( insert( insert( insert(hex(ID),9,0,'-'), 14,0,'-'), 19,0,'-'), 24,0,'-')) VIRTUAL AFTER `ID`;
