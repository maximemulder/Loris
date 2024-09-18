-- DROP TABLE IF EXISTS `redcap_notification`;

INSERT IGNORE INTO modules (Name, Active) VALUES ('redcap', 'Y');

CREATE TABLE `redcap_notification` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `complete` char(1) NOT NULL,
  `project_id` varchar(50) NOT NULL,
  `record` varchar(20) NOT NULL COMMENT 'PSCID',
  `redcap_event_name` varchar(50) NOT NULL COMMENT 'Visit_label',
  `instrument` varchar(150) NOT NULL COMMENT 'Test_name',
  `username` varchar(100) NOT NULL,
  `redcap_url` varchar(255) NOT NULL,
  `project_url` varchar(255) NOT NULL,
  `received_dt` datetime NOT NULL,
  PRIMARY KEY (`id`),
  KEY `i_redcap_notif_received_dt` (`received_dt`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

