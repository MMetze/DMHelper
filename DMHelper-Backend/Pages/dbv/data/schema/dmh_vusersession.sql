CREATE TABLE `dmh_vusersession` (
  `session` binary(16) NOT NULL,
  `user` binary(16) NOT NULL,
  `payload` varchar(32) CHARACTER SET utf8 NOT NULL,
  `time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' ON UPDATE CURRENT_TIMESTAMP,
  KEY `session` (`session`),
  KEY `user` (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
