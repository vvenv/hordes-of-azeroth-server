DROP PROCEDURE IF EXISTS add_migration;
DELIMITER ??
CREATE PROCEDURE `add_migration`()
BEGIN
DECLARE v INT DEFAULT 1;
SET v = (SELECT COUNT(*) FROM `migrations` WHERE `id`='20261112140000');
IF v = 0 THEN
INSERT INTO `migrations` VALUES ('20261112140000');
-- Add your query below.

-- Party bot settings per character (DungeonBotMgr): the role the player
-- plays (0 = guess from class and talents, else CombatBotRoles) and whether
-- five-player dungeons fill the party with bots on entry.
CREATE TABLE IF NOT EXISTS `character_party_bots` (
  `guid` int(11) unsigned NOT NULL COMMENT 'Character guid',
  `role` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `auto_fill` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- End of migration.
END IF;
END??
DELIMITER ;
CALL add_migration();
DROP PROCEDURE IF EXISTS add_migration;
