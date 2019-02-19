CREATE TABLE `dmh_vsessioncode` (
  `code` varchar(12) NOT NULL,
  `sessionid` binary(16) NOT NULL,
  `created` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8