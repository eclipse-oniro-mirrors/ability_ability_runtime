# G16 BundleEventCallback 预置应用 OTA 盲区

> 来源：本仓业务约束（预防性提醒）。`RegisterBundleEventCallback` 注册的回调依赖
> `COMMON_EVENT_PACKAGE_REMOVED/ADDED/CHANGED` 公共事件触发，而预置应用 OTA 走独立安装路径，
> 上述公共事件在 OTA 期间不会发布，回调因此不会触发。依赖该回调同步状态的逻辑会出现盲区。

## 特征信号 / grep 线索

- 调用 `RegisterBundleEventCallback` / `RegisterPluginEventCallback` 注册了 `BundleEventCallback` 子类，但未对 OTA/预置应用场景做兜底。
- 回调 `OnReceiveEvent` 仅处理 `COMMON_EVENT_PACKAGE_REMOVED/ADDED/CHANGED` 三类 action，假定这三类事件覆盖全部安装/升级/卸载路径。
- 依赖回调同步跨服务状态：URI 权限清理、模块信息更新、自启动数据、InsightIntent 事件、应用升级完成通知、常驻进程重启等。
- OTA 路径有独立标志位佐证（`ApplicationInfoFlag::FLAG_OTA_INSTALLED` / `FLAG_PREINSTALLED_APP` / `FLAG_PREINSTALLED_APP_UPDATE`），说明 OTA 是与普通安装/升级不同的安装来源。

```bash
rg -n "RegisterBundleEventCallback|RegisterPluginEventCallback" --type cpp
rg -n "COMMON_EVENT_PACKAGE_REMOVED|COMMON_EVENT_PACKAGE_ADDED|COMMON_EVENT_PACKAGE_CHANGED" --type cpp
rg -n "FLAG_OTA_INSTALLED|FLAG_PREINSTALLED_APP" --type cpp
```

## 历史案例 / 已知业务约束

- 本条为预防性业务约束，非单次 crash 事故。
- 现网调用点：
  - `services/abilitymgr/src/ability_manager_service.cpp:4200` —— AMS 注册 `AbilityBundleEventCallback`。
  - `agent_runtime_framework/services/agentmgr/src/agent_manager_service.cpp:183` —— AgentMgr 注册 `AgentBundleEventCallback`。
- `AbilityBundleEventCallback::OnReceiveEvent`（`services/abilitymgr/src/ability_bundle_event_callback.cpp:44-115`）在回调中同步：
  - `HandleRemoveUriPermission`（tokenId 对应 URI 权限未清理）；
  - `HandleUpdatedModuleInfo`（模块信息未刷新）；
  - `DeleteAutoStartupData` / `CheckAutoStartupData`（自启动数据陈旧）；
  - `InsightIntentEventMgr::UpdateInsightIntentEvent` / `DeleteInsightIntentEvent`（意图事件陈旧）；
  - `HandleAppUpgradeCompleted`（应用升级完成未通知 AMS，`AppUpgradeCompleted` 不触发）。
- OTA 期间上述任一未触发都会导致跨服务状态不一致（如已卸载应用的 URI 权限残留、陈旧模块信息被读取、自启动/意图表与实际安装态不符）。

## 检查点

- 调用 `RegisterBundleEventCallback`/`RegisterPluginEventCallback` 处，是否显式注释或处理"预置应用 OTA 期间回调不触发"？
- 是否存在 OTA 兜底链路（如监听 OTA 完成广播/系统参数、启动后扫描比对实际安装态、补偿式全量同步），而非仅依赖这三类公共事件？
- 回调中同步的状态（URI 权限、模块信息、自启动、InsightIntent、升级完成）是否在 OTA 路径有其他入口能保证最终一致？若 OTA 后靠首次调用懒补偿，补偿是否覆盖全部本应由回调处理的字段？
- `OnReceiveEvent` 是否对 action 做"非三类即忽略"的隐式假设？是否会把 OTA 场景静默丢掉？

## 典型后果

预置应用 OTA 期间回调不触发 → 跨服务状态不一致（URI 权限残留、模块/自启动/意图表陈旧、升级完成未通知），P1/P2。
