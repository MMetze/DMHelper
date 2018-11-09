CREATE TABLE `dmh_file` (
  `ID` binary(16) NOT NULL,
  `md5` varchar(32) NOT NULL,
  `user` binary(16) NOT NULL,
  `name` varchar(32) CHARACTER SET utf8 NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `data` mediumblob NOT NULL,
  PRIMARY KEY (`ID`,`user`),
  UNIQUE KEY `ID` (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1