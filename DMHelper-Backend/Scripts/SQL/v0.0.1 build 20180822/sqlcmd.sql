CREATE TABLE `dmhelper`.`dmh_Settings` ( `setting` VARCHAR(32) NOT NULL , `value` VARCHAR(128) NOT NULL , UNIQUE (`setting`)) ENGINE = InnoDB;
INSERT INTO dmh_Settings ( setting, value) VALUES ('version','DM Helper Backend v0.0.1 build 20180822');

CREATE TABLE `dmhelper`.`dmh_User` ( `ID` INT UNSIGNED NOT NULL AUTO_INCREMENT , `username` VARCHAR(32) NOT NULL , `email` VARCHAR(32) NOT NULL , `surname` VARCHAR(32) NOT NULL , `forename` VARCHAR(32) NOT NULL , `created` DATETIME DEFAULT NOW() , `lastlogin` DATETIME NULL DEFAULT NULL , `pass` VARCHAR(61) NOT NULL , `disabled` DATETIME NULL DEFAULT NULL , PRIMARY KEY (`ID`)) ENGINE = InnoDB;
INSERT INTO dmh_Users ( username, email, surname, forename ) VALUES ( 'mmetze', 'green.t@gmx.net', 'Metze', 'Matthias' );

CREATE TABLE `dmhelper`.`dmh_Area` ( `ID` INT UNSIGNED NOT NULL AUTO_INCREMENT , `name` VARCHAR(32) NOT NULL , PRIMARY KEY (`ID`)) ENGINE = InnoDB;
insert into dmh_area ( name ) values ( 'usermanagement' );

CREATE TABLE `dmhelper`.`dmh_vAreaRights` ( `uID` INT UNSIGNED NOT NULL , `aID` INT UNSIGNED NOT NULL ) ENGINE = InnoDB;
ALTER TABLE `dmh_varearights` ADD CONSTRAINT `userID` FOREIGN KEY (`uID`) REFERENCES `dmh_user`(`ID`) ON DELETE RESTRICT ON UPDATE RESTRICT; ALTER TABLE `dmh_varearights` ADD CONSTRAINT `AreaID` FOREIGN KEY (`aID`) REFERENCES `dmh_area`(`ID`) ON DELETE RESTRICT ON UPDATE RESTRICT;

CREATE TABLE `dmhelper`.`dmh_Session` ( `ID` INT UNSIGNED NOT NULL AUTO_INCREMENT , `owner` INT UNSIGNED NOT NULL , `payload` BLOB NULL , `last` TIMESTAMP NOT NULL , PRIMARY KEY (`ID`), INDEX `owner` (`owner`)) ENGINE = InnoDB;
