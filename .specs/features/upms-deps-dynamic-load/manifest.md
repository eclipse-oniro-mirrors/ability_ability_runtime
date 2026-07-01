# Manifest — UPMS 依赖库动态加载/卸载

| 字段 | 值 |
|------|-----|
| id | upms-deps-dynamic-load |
| title | UPMS (SA 183) 依赖库动态加载/卸载降内存 |
| type | feature |
| complexity | 标准（2026-07-01 范围裁剪：原复杂层 7 插件 → 只 media+storage 2 插件，fileuri/broker/identity/sandbox/udmf DESCOPED） |
| profile | (none — 无匹配的子系统 profile) |
| target_release | 待确认（OQ-1，见 proposal §3.8） |
| language | zh-CN |
| created | 2026-06-28 |
| owner | wangzhen |

## Lineage / 相关链接
- **废弃前置方案**：`upms-dynamic-unload`（SA 实体级延时自卸载）— 经验证 SA 级动态卸载不可行，已废弃。本地分支 `upmsop1` 已 reset 抹除其代码与 .specs；reflog `6d069578a0` 可恢复；远端 `own/upmsop1` 留作备份。本项是其**替代方向**。
- **前置分析**（本地笔记，未入库，故无链接）：SA 卸载可行性评估（结论：不建议）+ UPMS 内存优化建议（依赖裁剪/数据层/代理冗余）。需求方据其结论转向依赖库粒度。
- **目标 SA**：Uri Permission Manager Service，SA 183，库 `libupms.z.so`，运行于 `foundation` 进程，`run-on-create: false`

## Stage 状态
- [x] **Stage 1 Define** — Approved 2026-06-28
- [x] **Stage 2 Specify** — Approved 2026-06-28（复杂层 design+spec+gates/specify 13/13；阶段：media→P1、broker→P2）
- [~] **Stage 3 Implement** — Phase 1(media) + Phase 2(storage) + task-03 + UT mock 代码完成 + **编译验证通过**（libupms✓ + media/storage 两插件✓ + 死依赖清理 13 项✓ + 两 UT target 编译通过✓，见 review.md §4/§5/§8/§9/§10/§11）；**fileuri/broker/Phase 3 DESCOPED**（2026-07-01 范围调整）；待设备跑 UT + AC-10 实测
- [ ] Stage 4 Release

## 基线关键约束（一句话）
UPMS 本体常驻，建通用依赖插件框架（interface 基类 + 子类在独立 .so + 管理器 dlopen 调 C 工厂 + 加载状态 + 空闲 dlclose），**范围：只把 media + storage 两个独占候选依赖抽进插件 .so**（media→libupms_media_ext、storage→libupms_storage_ext），fileuri/broker/identity/sandbox/udmf DESCOPED（暂不插件化，libupms 直链）+ 清理 13 死依赖；授权状态强一致，定性验收（证 PSS 释放即可），单仓。
