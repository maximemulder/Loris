-- Create the REDCap notifications table

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
  `handled_dt` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `i_redcap_notif_received_dt` (`received_dt`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Add the module to the list of modules

INSERT IGNORE INTO `modules` (`Name`, `Active`) VALUES ('redcap', 'Y');

-- Add the module settings

INSERT INTO `ConfigSettings` (`Name`, `Description`, `Visible`, `AllowMultiple`, `DataType`, `Parent`, `Label`, `OrderNumber`)
  VALUES
    ('redcap', 'Settings related to the REDCap integration of LORIS', 1, 0, NULL, NULL, 'REDCap', 13);

SET @redcap_config_id := LAST_INSERT_ID();

INSERT INTO `ConfigSettings` (`Name`, `Description`, `Visible`, `AllowMultiple`, `DataType`, `Parent`, `Label`, `OrderNumber`)
  VALUES
    ('url', 'The API URL provided by REDCap', 1, 0, 'text', @redcap_config_id, 'REDCap API URL', 1),
    ('token', 'The API token provided by REDCap', 1, 0, 'text', @redcap_config_id, 'REDCap API token', 2);
