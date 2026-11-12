DROP PROCEDURE IF EXISTS add_migration;
DELIMITER ??
CREATE PROCEDURE `add_migration`()
BEGIN
DECLARE v INT DEFAULT 1;
SET v = (SELECT COUNT(*) FROM `migrations` WHERE `id`='20261112160000');
IF v = 0 THEN
INSERT INTO `migrations` VALUES ('20261112160000');
-- Add your query below.

-- Simplified Chinese for the mangos_string rows that had none: the replies a
-- player sees from GM tools (speeds, positions, reputation, skills, stats),
-- bot and battleground notices. Rows that are only a format ("%d - %s") or
-- a table rule keep the English default.
UPDATE `mangos_string` SET `content_loc4` = '当前的每日消息：\n%s' WHERE `entry` = 56;
UPDATE `mangos_string` SET `content_loc4` = '地图：%u（%s） 区域：%u（%s） 地区：%u（%s）\nX：%f Y：%f Z：%f 朝向：%f\ngrid[%u,%u]cell[%u,%u] 副本 ID：%u\n 区域坐标 X：%f Y：%f\n地面高度：%f 地板高度：%f 高度数据（地图：%u 视觉地图：%u）' WHERE `entry` = 101;
UPDATE `mangos_string` SET `content_loc4` = '你把%s的法术修正 %u 改为 %i（法术族位 %u，目标 %s）。' WHERE `entry` = 131;
UPDATE `mangos_string` SET `content_loc4` = '你把 %2$s 的所有速度设为正常值的 %1$2.2f 倍。' WHERE `entry` = 137;
UPDATE `mangos_string` SET `content_loc4` = '你把 %2$s 的速度设为正常值的 %1$2.2f 倍。' WHERE `entry` = 139;
UPDATE `mangos_string` SET `content_loc4` = '你把 %2$s 的游泳速度设为正常值的 %1$2.2f 倍。' WHERE `entry` = 141;
UPDATE `mangos_string` SET `content_loc4` = '你把 %2$s 的飞行路线速度设为正常值的 %1$2.2f 倍。' WHERE `entry` = 145;
UPDATE `mangos_string` SET `content_loc4` = '你从 %2$s 身上拿走了 %1$i 铜币。' WHERE `entry` = 155;
UPDATE `mangos_string` SET `content_loc4` = '找到的地点：\n %s' WHERE `entry` = 168;
UPDATE `mangos_string` SET `content_loc4` = '[机器人] 加入一个 %u 级的临时 |cFF0070DD联盟|r 战场机器人到战场队列 %u' WHERE `entry` = 185;
UPDATE `mangos_string` SET `content_loc4` = '[机器人] 加入一个 %u 级的临时 |cFFFF0000部落|r 战场机器人到战场队列 %u' WHERE `entry` = 186;
UPDATE `mangos_string` SET `content_loc4` = '找不到 %2$s 的路点 %1$u（路径 %3$i，来自 %4$s）' WHERE `entry` = 220;
UPDATE `mangos_string` SET `content_loc4` = '%2$s 的路点 %1$u（路径 %3$i，由 %4$s 保存）' WHERE `entry` = 246;
UPDATE `mangos_string` SET `content_loc4` = '%s 的工单（最后更新：%s）：\n%s ' WHERE `entry` = 290;
UPDATE `mangos_string` SET `content_loc4` = '%3$s 在阵营 %1$s（%2$u）的声望被设为 %4$5d！' WHERE `entry` = 305;
UPDATE `mangos_string` SET `content_loc4` = '回复：\n%s ' WHERE `entry` = 373;
UPDATE `mangos_string` SET `content_loc4` = '%s 被封禁 %s。原因：%s。' WHERE `entry` = 408;
UPDATE `mangos_string` SET `content_loc4` = '%s 已解封。备注：%s' WHERE `entry` = 411;
UPDATE `mangos_string` SET `content_loc4` = 'IP：%s\n封禁日期：%s\n解封日期：%s\n剩余：%s\n原因：%s\n执行人：%s' WHERE `entry` = 423;
UPDATE `mangos_string` SET `content_loc4` = '任务 %u 由物品开始。要正常进行，请先把物品放进背包，再按正常方式接任务：.additem %u' WHERE `entry` = 472;
UPDATE `mangos_string` SET `content_loc4` = '已向 %2$s 发送操作码 %1$u' WHERE `entry` = 478;
UPDATE `mangos_string` SET `content_loc4` = '玩家 %3$s 的技能 %1$u（%2$s）设为 %4$u，当前上限设为 %5$u（不含天赋加成）。' WHERE `entry` = 484;
UPDATE `mangos_string` SET `content_loc4` = '已清除 %2$s 的法术 %1$u 冷却。' WHERE `entry` = 493;
UPDATE `mangos_string` SET `content_loc4` = '已从 %3$s 移除物品 %1$i，数量 %2$i' WHERE `entry` = 496;
UPDATE `mangos_string` SET `content_loc4` = '>> 游戏物件 %s（GUID：%u）位于 %f %f %f，朝向 %f。' WHERE `entry` = 523;
UPDATE `mangos_string` SET `content_loc4` = '选中的物件：\n|cffffffff|Hgameobject:%d|h[%s]|h|r GUID：%u ID：%u\nX：%f Y：%f Z：%f 地图：%u\n朝向：%f' WHERE `entry` = 524;
UPDATE `mangos_string` SET `content_loc4` = '选中的玩家：%s。\n阵营：%u。\nNPC 标记：%u。\n编号：%u。\n模型：%u（原本：%u）。' WHERE `entry` = 539;
UPDATE `mangos_string` SET `content_loc4` = '生命值（基础）：%u。（最大）：%u。（当前）：%u。' WHERE `entry` = 541;
UPDATE `mangos_string` SET `content_loc4` = '单位标记：%u。\n动态标记：%u。\n阵营模板：%u。' WHERE `entry` = 542;
UPDATE `mangos_string` SET `content_loc4` = '事件 %u：%s%s\n开始：%s 结束：%s 间隔：%s 持续：%s\n下次状态变化：%s' WHERE `entry` = 586;
UPDATE `mangos_string` SET `content_loc4` = '你把 %2$s 的无重力模式设为%1$s。' WHERE `entry` = 611;
UPDATE `mangos_string` SET `content_loc4` = '%2$s 把你的无重力模式设为%1$s。' WHERE `entry` = 612;
UPDATE `mangos_string` SET `content_loc4` = '无重力作弊：开' WHERE `entry` = 613;
UPDATE `mangos_string` SET `content_loc4` = '无重力作弊：关' WHERE `entry` = 614;
UPDATE `mangos_string` SET `content_loc4` = '$n占领了%s！如果无人阻止，%s将在 1 分钟后控制它！' WHERE `entry` = 660;
UPDATE `mangos_string` SET `content_loc4` = '你在%s的战斗结束后，我们没能把荣誉印记发给你。现在把它附在信里寄给你。感谢你在%s的奋战！' WHERE `entry` = 666;
UPDATE `mangos_string` SET `content_loc4` = '%s 的排队情况（等级 %u 到 %u）\n联盟排队：%u（至少还需 %u）\n部落排队：%u（至少还需 %u）' WHERE `entry` = 711;
UPDATE `mangos_string` SET `content_loc4` = '人数不足，本场战斗将在 %u 分钟后关闭。' WHERE `entry` = 750;
UPDATE `mangos_string` SET `content_loc4` = '人数不足，本场战斗将在 %u 秒后关闭。' WHERE `entry` = 751;
UPDATE `mangos_string` SET `content_loc4` = '%s遭到攻击！如果无人阻止，%s将摧毁它！' WHERE `entry` = 760;
UPDATE `mangos_string` SET `content_loc4` = '%s遭到攻击！如果无人阻止，%s将占领它！' WHERE `entry` = 764;
UPDATE `mangos_string` SET `content_loc4` = '斯尼维尔来了！斯尼维尔要占领冷齿矿洞！' WHERE `entry` = 791;
UPDATE `mangos_string` SET `content_loc4` = '进攻的时候到了！为了部落！' WHERE `entry` = 792;
UPDATE `mangos_string` SET `content_loc4` = '振作起来，联盟！把这些恶棍赶出奥特兰克山谷！' WHERE `entry` = 793;
UPDATE `mangos_string` SET `content_loc4` = '你把 %s 的近战暴击率改为 %g%%。' WHERE `entry` = 930;
UPDATE `mangos_string` SET `content_loc4` = '%s 把你的近战暴击率改为 %g%%。' WHERE `entry` = 931;
UPDATE `mangos_string` SET `content_loc4` = '你把 %s 的远程暴击率改为 %g%%。' WHERE `entry` = 932;
UPDATE `mangos_string` SET `content_loc4` = '%s 把你的远程暴击率改为 %g%%。' WHERE `entry` = 933;
UPDATE `mangos_string` SET `content_loc4` = '你把 %s 的法术暴击率改为 %g%%。' WHERE `entry` = 934;
UPDATE `mangos_string` SET `content_loc4` = '%s 把你的法术暴击率改为 %g%%。' WHERE `entry` = 935;
UPDATE `mangos_string` SET `content_loc4` = '你把 %s 的格挡率改为 %g%%。' WHERE `entry` = 944;
UPDATE `mangos_string` SET `content_loc4` = '%s 把你的格挡率改为 %g%%。' WHERE `entry` = 945;
UPDATE `mangos_string` SET `content_loc4` = '你把 %s 的躲闪率改为 %g%%。' WHERE `entry` = 946;
UPDATE `mangos_string` SET `content_loc4` = '%s 把你的躲闪率改为 %g%%。' WHERE `entry` = 947;
UPDATE `mangos_string` SET `content_loc4` = '你把 %s 的招架率改为 %g%%。' WHERE `entry` = 948;
UPDATE `mangos_string` SET `content_loc4` = '%s 把你的招架率改为 %g%%。' WHERE `entry` = 949;
UPDATE `mangos_string` SET `content_loc4` = '激活的物件：%d。可见距离系数：%0.2f' WHERE `entry` = 1030;
UPDATE `mangos_string` SET `content_loc4` = '每日消息已改为：\n%s' WHERE `entry` = 1101;
UPDATE `mangos_string` SET `content_loc4` = '（%u/%u +永久 %u +临时 %u）' WHERE `entry` = 1122;
UPDATE `mangos_string` SET `content_loc4` = '无效的骑术类型。可以用以下之一：\n%s' WHERE `entry` = 1172;
UPDATE `mangos_string` SET `content_loc4` = '|cff00ff00角色|r|cffff00ff %s |r|cff00ff00放弃了工单。工单编号：|r|cffff00ff %d。|r' WHERE `entry` = 2002;
UPDATE `mangos_string` SET `content_loc4` = '这个工单已经分配给你了。要改派，先用 .ticket unassign %d 取消分配。' WHERE `entry` = 2013;
UPDATE `mangos_string` SET `content_loc4` = '|cff00ff00角色|r|cffff00ff %s |r|cff00ff00修改了工单。工单编号：|r|cffff00ff %d。|r' WHERE `entry` = 2053;
UPDATE `mangos_string` SET `content_loc4` = '法力值（基础）：%u。（最大）：%u。（当前）：%u。' WHERE `entry` = 11017;

-- End of migration.
END IF;
END??
DELIMITER ;
CALL add_migration();
DROP PROCEDURE IF EXISTS add_migration;
