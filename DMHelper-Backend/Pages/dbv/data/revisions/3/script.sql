ALTER DATABASE dmhelper CHARACTER SET utf8 COLLATE utf8_general_ci;

SELECT CONCAT("ALTER TABLE", TABLE_SCHEMA, '.', TABLE_NAME," COLLATE utf8_general_ci;") AS ExcuteTheString
FROM INFORMATION_SCHEMA.TABLES
WHERE TABLE_SCHEMA="dmhelper"
AND TABLE_TYPE="BASE TABLE";

ALTER TABLE `dmh_area` CHANGE `name` `name` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_file` CHANGE `name` `name` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_session` CHANGE `payload` `payload` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_settings` CHANGE `setting` `settings` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_settings` CHANGE `value` `value` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_user` CHANGE `username` `username` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_user` CHANGE `email` `email` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_user` CHANGE `surname` `surname` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_user` CHANGE `forename` `forename` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_user` CHANGE `pass` `pass` VARCHAR(60) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;
ALTER TABLE `dmh_vusersession` CHANGE `payload` `payload` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL;

ALTER TABLE `dmh_user` CHANGE `ID` `ID` BINARY(16) NOT NULL;

 alter table dmh_user add IDh varchar(36) generated always as
  (insert(
     insert(
       insert(
         insert(hex(ID),9,0,'-'),
         14,0,'-'),
       19,0,'-'),
     24,0,'-')
  ) virtual after ID;

ALTER TABLE `dmh_session` CHANGE `ID` `ID` BINARY(16) NOT NULL;
alter table dmh_session add IDh varchar(36) generated always as
 (insert(
    insert(
      insert(
        insert(hex(ID),9,0,'-'),
        14,0,'-'),
      19,0,'-'),
    24,0,'-')
 ) virtual after ID;

 ALTER TABLE `dmh_area` CHANGE `ID` `ID` BINARY(16) NOT NULL;
 alter table dmh_area add IDh varchar(36) generated always as
  (insert(
     insert(
       insert(
         insert(hex(ID),9,0,'-'),
         14,0,'-'),
       19,0,'-'),
     24,0,'-')
  ) virtual after ID;

update dmh_area set ID=unhex(replace(uuid(),"-","")) where ID=is not null;

ALTER TABLE `dmh_vusersession` CHANGE `session` `session` BINARY(16) NOT NULL, CHANGE `user` `user` BINARY(16) NOT NULL;
update dmh_vusersession set user=(select id from dmh_user where username="m.metze") where payload='dds';
update dmh_vusersession set user=(select id from dmh_user where username="c.turner") where payload='';
update dmh_vusersession set session=(select id from dmh_session where IDh="7B3AA550-649A-4D51-920E-CAB465616995");

ALTER TABLE `dmh_session` CHANGE `owner` `owner` BINARY(16) NOT NULL;
update dmh_session set owner=unhex(replace('813C343B-B270-11E8-BB35-2C4D5446E8FD','-',''));

ALTER TABLE `dmh_vsessioncode` ADD `created` DATETIME on update CURRENT_TIMESTAMP NOT NULL AFTER `sessionid`;
ALTER TABLE `dmh_vsessioncode` CHANGE `created` `created` DATETIME on update CURRENT_TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP;
