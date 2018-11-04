CREATE TABLE `dmh_varearights` (
  `uID` int(10) unsigned NOT NULL,
  `aID` int(10) unsigned NOT NULL,
  KEY `userID` (`uID`),
  KEY `AreaID` (`aID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8