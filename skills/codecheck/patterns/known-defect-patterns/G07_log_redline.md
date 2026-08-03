# G7 日志红线

> 来源：known-defect-patterns.md G7。

## 特征信号 / grep 线索

- 日志打印：`udid`、`networkid`、`SN`、`challenge`、`token`、`password`、完整 `cmdLine`、文件路径/名称、已安装应用包名。

## 历史案例

- `distributed_data_storage.cpp` 打印 udid；`ability_record.cpp`/`ui_ability_lifecycle_manager.cpp` 打印 networkid；aa/AMS 打印 SN（多版本复发）；`process_manager.cpp` 打印完整 cmdLine（含会话 challenge）；climgr 多处敏感日志；Context 打印文件名。

## 检查点

- 对红线关键字做全库 grep，逐处确认是否脱敏。
- 凭据类内容出现在日志中：除报代码问题外，提示"凭据视同已泄露，需评估轮换"。
