ALTER TABLE `dmh_session` ADD `name` VARCHAR(255) NOT NULL AFTER `owner`;
ALTER TABLE `dmh_vusersession` ADD UNIQUE( `session`, `user`);
