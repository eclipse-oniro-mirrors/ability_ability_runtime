# G8 配置级提权

> 来源：known-defect-patterns.md G8。

## 特征信号 / grep 线索

- SELinux 策略中服务数据库/目录标签过宽；CLI/bin 无独立标签（shell 可直接执行）。
- 权限映射表（如 cli_permission_map.json）中单个权限映射多个系统权限。
- PermissionDefinition.json 与对外文档的 `availableType`/ACL 使能方式不一致。
- 系统公共事件已定义但未加入系统公共事件列表（历史多次重犯）。
- 系统应用声明与功能无关的权限（违反最小化，如弹窗应用申请联网）。

## 历史案例

- `auto_startup_service.db` / `ability_manager_service.db` / `bmsdb.db` 标签过大 → 攻破低权限服务即可篡改开机自启、MDM 保活。
- `ohos.permission.cli.START_ABILITY` 含 `START_INVISIBLE_ABILITY` → 调起任意应用非暴露组件。
- 小艺 Claw CLI 无独立标签 → shell 无权限执行任意 claw 指令。

## 检查点

- 审查不只看代码：SELinux 策略、权限定义、映射表、事件列表均纳入范围。
- 发现"权限/标签比功能需要的宽"即上报。
