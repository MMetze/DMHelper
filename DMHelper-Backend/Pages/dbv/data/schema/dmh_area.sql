CREATE TABLE `dmh_area` (
  `ID` binary(16) NOT NULL,
  `IDh` varchar(36) AS (insert(
     insert(
       insert(
         insert(hex(ID),9,0,'-'),
         14,0,'-'),
       19,0,'-'),
     24,0,'-')) VIRTUAL,
  `name` varchar(32) CHARACTER SET utf8 NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1