# Gate — Stage 1 Define（定义阶段）

> item: `upms-deps-dynamic-load` | 阶段切换：Stage 1 → Stage 2
> 规则依据：SKILL.md「Stage 1 进入 Stage 2 条件」+「审批即状态」+「证据先于声明」。
> 本文件记录 Stage 1 门禁逐项检查结果与审批证据。

## 一、Stage 1 退出 / Stage 2 进入门禁

| # | 检查项 | 结果 | 证据 / 理由 |
|---|--------|------|------------|
| D1 | proposal.md 原始需求已记录 | ✅ 通过 | proposal.md §一，记录 SA 卸载被否决、转向依赖库粒度的动机与降内存目标 |
| D2 | 澄清记录完整（逐轮问答 + 选项 + 决策 + 理由） | ✅ 通过 | proposal.md §二，8 轮澄清（清理/驱动力/维度/目标库/触发/仓范围/验收线/状态一致）均闭环 |
| D3 | 范围 (In scope) 已明确 | ✅ 通过 | proposal.md §3.2，5 项（加载管理器/调用收拢/空闲触发/同步保护/单测） |
| D4 | 排除项 (Out of scope) 已明确 | ✅ 通过 | proposal.md §3.3，5 项排除 + §3.7 不涉及项确认表 |
| D5 | 涉及子系统 / 仓已识别 | ✅ 通过 | proposal.md §3.4，单仓 `ability_runtime`（services/uripermmgr + BUILD.gn） |
| D6 | 复杂度级别已判断 | ✅ 通过 | standard（单仓单特性，依赖库粒度动态加载，无跨 SIG 协调） |
| D7 | 每个 P0/P1 AC 以 WHEN/THEN 写出且可测 | ✅ 通过 | proposal.md §3.5，3 个 P0 + 3 个 P1，每条含验证手段 |
| D8 | 不涉及项确认表已完成（YAGNI） | ✅ 通过 | proposal.md §3.7，7 行 N/A 已填，无留空 |
| D9 | Profile 判定完成 | ✅ 通过 | 无 `.claude/ohos-sdd/profiles/` 目录，无匹配子系统 profile，manifest.profile = none |
| D10 | 待解决问题已登记并指派 owner | ✅ 通过 | proposal.md §3.8，OQ-1~OQ-6，均 Stage 2 处理 |
| D11 | 基线理解已被需求方确认 | ✅ 通过 | 见下方「审批证据」——需求方对理解草稿回复 "Y" |

**结论：Stage 1 门禁全部通过（11/11）。基线草案已写入 proposal.md。**

## 二、审批证据 (Approval Evidence)

- **形式**：理解确认草稿（核心 / 范围含 / 排除项 / 涉及仓 / 复杂度 / P0-P1 AC / Stage 2 开放项）已于本轮对话向需求方完整输出。
- **需求方表态**：2026-06-28，需求方（owner: wangzhen）回复 **"Y"**，确认理解准确、范围/排除项无增删、复杂度判 **standard** 可接受，授权写入基线草案。
- **判定**：Stage 1 门禁 11/11 通过，基线草案已落盘。**等待需求方审阅书面 proposal.md 后，明确批准进入 Stage 2。**

## 三、带入 Stage 2 的移交事项

1. design.md 需决策 OQ-2~OQ-6（封装结构、空闲阈值与判定、依赖库用法调研、dlopen 调用方式、同步原语）。
2. design.md 需给出关键 ADR：依赖加载管理器架构、连接生命周期管理（rdb RdbStore 句柄的关闭/重建）、空闲触发实现、同步保护策略。
3. spec.md 需把 §3.5 的 AC 展开为完整规格 + 兼容性 / 可追溯。
4. `target_release`(OQ-1) 在 Stage 2 期间向 owner 确认。
5. **Stage 2 必须先做"UPMS 对 rdb/data_share 等的实际用法调研"（OQ-4）——这是强一致 AC-2/AC-3 的实现前提，不可跳过。**

> HARD-GATE：design.md / spec.md 在本 gate 通过 + 需求方**明确批准进入 Stage 2** 后方可创建。本 gate 已通过，**等待需求方批准进入 Stage 2**。
