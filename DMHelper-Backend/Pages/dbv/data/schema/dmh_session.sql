CREATE TABLE `dmh_session` (
  `ID` binary(16) NOT NULL,
  `IDh` varchar(36) CHARACTER SET latin1 AS (insert(insert(insert(insert(hex(`ID`),9,0,'-'),14,0,'-'),19,0,'-'),24,0,'-')) VIRTUAL,
  `owner` binary(16) NOT NULL,
  `payload` varchar(1024) NOT NULL,
  `last` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`ID`),
  KEY `owner` (`owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8