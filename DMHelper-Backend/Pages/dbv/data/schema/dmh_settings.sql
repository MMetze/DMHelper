CREATE TABLE `dmh_settings` (
  `settings` varchar(32) NOT NULL,
  `value` varchar(32) NOT NULL,
  UNIQUE KEY `setting` (`settings`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8