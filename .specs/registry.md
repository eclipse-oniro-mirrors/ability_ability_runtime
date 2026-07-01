# .specs Registry

> SDD 需求项索引。每行一个 item，登记 id / 类型 / 复杂度 / 当前阶段。
> 文件存在 = 草案存在；阶段通过 = gate 勾选 + 审批证据完整 + 状态字段已更新。

| id | type | complexity | stage | title |
|----|------|-----------|-------|-------|
| features/upms-deps-dynamic-load | feature | 复杂 | Stage 3 (Phase 2 + task-03 死依赖清理13项完成+编译通过；待 UT+AC-10+Phase 3) | UPMS (SA 183) 依赖插件框架动态加载/卸载降内存 |

## Stage 图例
- **Stage 1 Define** — proposal.md 基线
- **Stage 2 Specify** — design.md + spec.md
- **Stage 3 Implement** — execution-plan + task.md + 代码
- **Stage 4 Release** — 验证 + 合入 + 复盘

## 废弃记录
- `features/upms-dynamic-unload`（SA 实体级延时自卸载）— **已废弃**：经验证 SA 级动态卸载不可行（见 `upms-deps-dynamic-load` proposal §一）。本地分支 `upmsop1` 已 `git reset --hard HEAD~2` 抹除其代码（commit `e06439095f`）与旧 .specs（commit `6d069578a0`）；reflog 可恢复；远端 `own/upmsop1` 仍留作备份（未 force-push）。本项 `upms-deps-dynamic-load` 是其替代方向。
