# Task 03 — 死依赖清理（Phase 1）

> item: `upms-deps-dynamic-load` | depends on: task-01（可与 task-02 并行） | mode: 直接实现
> 规则来源：[design.md](../design.md) ADR-6、§5；[spec.md](../spec.md) AC-7。
> 取证：全量依赖审计（4 路 Explore）确认以下依赖在 UPMS 源码零直接调用。

## 目标
从 `ohos_shared_library("libupms")` 的 `external_deps` 移除零调用死依赖，逐个 build 验证，永久减链接/可能减运行时映射（条件性收益，作观测项）。

## 待移除清单（external_deps，`services/uripermmgr/BUILD.gn`）
| # | 依赖 | 取证（零调用） | 验证注意 |
|---|------|------|------|
| 1 | `relational_store:native_rdb` | grep 0 include/0 API（亲验） | — |
| 2 | `relational_store:native_dataability` | 同上 | — |
| 3 | `data_share:datashare_consumer` | 同上 | — |
| 4 | `init:libbeget_proxy` | 0 直接调用 | 防传递需要 |
| 5 | `init:libbegetutil` | 0 直接调用 | 防传递需要 |
| 6 | `common_event_service:cesfwk_core` | 0（不订阅公共事件） | 防传递 |
| 7 | `common_event_service:cesfwk_innerkits` | 0 | 防传递 |
| 8 | `background_task_mgr:bgtaskmgr_innerkits` | 0（不申请后台任务） | 防传递 |
| 9 | `i18n:intl_util` | 0（graphics-gate） | flag 关时本就不链 |
| 10 | `ability_base:configuration` | 0（只用 want/zuri） | 防传递 |
| 11 | `appkit_manager_helper`（内部 deps） | 0 | 防传递 |

> **`eventhandler`**：零调用，但 task-01 已 +`ffrt:libffrt` 走 ffrt 延时 → eventhandler 可一并移除。若实现期发现 EventHandler 仍被间接需要则保留。

## 执行步骤（逐个，防一次性断裂）
1. 每次移除**一个**依赖。
2. `./build.sh --product-name rk3568 --build-target libupms` 验证通过。
3. 若失败（传递需要）：回退该项，在完成标准注明「保留，原因=被 X 传递需要」。
4. 全部处理完跑 `run -t UT -tp uripermmgr` 确认无回归。

## 完成标准
- [ ] 清单 11 项逐个 build 验证，记录「移除成功」或「保留+原因」。
- [ ] libupms 编译通过、UT 全绿（AC-7）。
- [ ] 运行时收益观测：`/proc/<pid>/maps` 对比移除前后 foundation 内这些 .so 是否去映射（条件性，非硬门槛）。
- [ ] 完成标准表填回实际结果（哪些真移除、哪些因传递保留）。

## 风险
- **R8 传递链接断裂**：逐个验证 + 失败即回退，风险可控。
- eventhandler 与 task-01 ffrt 选型耦合：确认 task-01 用 ffrt 后再移除 eventhandler。
