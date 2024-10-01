-- DROP TABLE IF EXISTS `redcap_notification`;

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

INSERT IGNORE INTO `modules` (`Name`, `Active`) VALUES ('redcap', 'Y');

INSERT INTO `ConfigSettings` (`Name`, `Description`, `Visible`, `AllowMultiple`, `DataType`, `Parent`, `Label`, `OrderNumber`) VALUES ('REDCap', 'The token provided by RedCAP to call its API', 0, 0, 'text', NULL, 'RedCAP API Token', NULL);

SET @redcap_config_setting_id := LAST_INSERT_ID();

INSERT INTO `Config` (`ConfigID`, `Value`) VALUES (@redcap_config_setting_id, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ');
