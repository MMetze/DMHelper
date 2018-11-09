CREATE TABLE `dmh_settings` (
  `settings` varchar(32) CHARACTER SET utf8 NOT NULL,
  `value` varchar(32) CHARACTER SET utf8 NOT NULL,
  UNIQUE KEY `setting` (`settings`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1