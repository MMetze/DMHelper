CREATE TABLE `dmh_file` (
  `ID` binary(16) NOT NULL,
  `user` binary(16) NOT NULL,
  `name` varchar(32) NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `data` mediumblob NOT NULL,
  UNIQUE KEY `ID` (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8