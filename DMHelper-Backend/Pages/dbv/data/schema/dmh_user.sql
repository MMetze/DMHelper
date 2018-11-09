CREATE TABLE `dmh_user` (
  `ID` binary(16) NOT NULL,
  `IDh` varchar(36) AS (insert(
     insert(
       insert(
         insert(hex(ID),9,0,'-'),
         14,0,'-'),
       19,0,'-'),
     24,0,'-')) VIRTUAL,
  `username` varchar(32) CHARACTER SET utf8 NOT NULL,
  `email` varchar(32) CHARACTER SET utf8 NOT NULL,
  `surname` varchar(32) CHARACTER SET utf8 NOT NULL,
  `forename` varchar(32) CHARACTER SET utf8 NOT NULL,
  `created` datetime DEFAULT CURRENT_TIMESTAMP,
  `lastlogin` datetime DEFAULT NULL,
  `pass` varchar(60) CHARACTER SET utf8 NOT NULL,
  `disabled` datetime DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1