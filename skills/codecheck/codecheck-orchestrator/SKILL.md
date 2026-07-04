---
name: codecheck-orchestrator
description: |
  代码检视总编排器：对指定路径或 Kit，自动选择并调度 codecheck 下各子 skill（deep-scan、external-input-audit、api-audit 等），
  执行后把各维度产出跨维度去重、合并为一份统一的 codecheck_report_<scope>_<YYYYMMDD>.md 报告。
  当用户表达通用代码检视意图时触发："检视一下代码"、"帮我审一下"、"做一次 code review"、"代码审查"、
  "review 一下"、"审一下这块代码"、"codecheck"、"代码检视"、"全面检视"、"生成检视报告"。
  当用户指定单一维度（纯安全/纯 API/纯外部输入）时，不触发本 skill，直接调对应子 skill。
  当用户明确说"深度扫描"时，优先触发 deep-scan 而非本 skill（deep-scan 是三层专用编排）。
---

# Codecheck Orchestrator — 代码检视总编排器

## 概述

本 skill 是 [`skills/codecheck/`](.) 工作台的**总调度与报告合并层**。它解决一个问题：codecheck 下有 5 个检视维度 skill + 1 个三层编排器（deep-scan），但用户说"检视一下代码"时，没有一个统一入口来**按范围选维度、并行/串行调度、最后合并成一份报告**。本 skill 补齐这一层。

> 导航中枢是 [README.md](README.md)。本 skill 是 README 中"检视流程"的可执行版本——当用户表达通用检视意图时，按本 skill 的流程走；当意图是单一维度时，让位给对应子 skill。

## 与子 skill 的关系

| 子 skill | 本 skill 何时调用 |
|----------|------------------|
| `deep-scan` | 默认：通用检视必调（一次拿 bug+logic+security 三维度） |
| `external-input-audit` | 目标路径在服务侧（`services/`）或 IPC/DB/文件/配置密集区时调 |
| `api-audit` | 目标涉及对外 API（`interfaces/kits/`、`frameworks/*/napi|ani|c/`、或用户指定 Kit）时调 |
| `high-impact-bug-audit` / `logic_analyzer` / `security_review` | **不单独调**——已被 `deep-scan` 包含，避免重复 |

原则：**不与 deep-scan 重复调度其已含的三个维度**。本 skill 只在 deep-scan 之上**补** external-input-audit 和 api-audit，并做合并。

## 工作流

### Step 1：界定范围（缺则向用户确认）

必明确两项：
1. **目标**：路径（如 `services/abilitymgr/src/`）或 Kit 名（如 `abilityKit`）。
2. **检视重点**：未指定时默认"通用检视"。

若用户只说"检视一下代码"未给范围，**必须先问**，不要默认全仓。

### Step 2：选择 skill 组合

按目标位置决策：

| 目标特征 | 调用组合 |
|---------|---------|
| 通用路径（未特化） | `deep-scan {path}` |
| `services/` 下、IPC/持久化密集 | `deep-scan {path}` + `external-input-audit {path}` |
| `interfaces/kits/`、NAPI/ANI/C 绑定、或指定 Kit | `api-audit {kit}` + `deep-scan` 覆盖实现侧路径 |
| 接口+服务侧都涉（最大覆盖） | `deep-scan` + `external-input-audit` + `api-audit` |

子 skill 之间无数据依赖，**并行调度**（多个 Agent 或多个工具调用并发），各自产出原始文件，不互相等待。

### Step 3：执行子 skill

逐个读对应目录的 `SKILL.md` 按其工作流执行。**保留各 skill 原始产出**（md/csv/excel），不在中间改写。记录每个 skill 的：
- 产出文件路径
- 发现总数与分级（P0/P1/P2 或 Confirmed/Likely/Suspicious）

### Step 4：合并为统一报告

汇总到 `codecheck_report_<scope>_<YYYYMMDD>.md`（`<scope>` 用路径简写或 Kit 名）。结构：

```markdown
# 代码检视报告 — <scope>
> 范围：<路径/Kit>   日期：<YYYY-MM-DD>   检视维度：<调用的 skill 列表>

## 1. 总览
- 执行的 skill 与各自发现数
- P0/P1/P2 统计（如适用）

## 2. 高优先级发现（P0/P1，跨维度去重后）
每条：标题 / 维度来源 / 位置(file:line) / 触发路径 / 影响 / 建议 / 证据
（同一 file:line 被多 skill 命中 → 合并为一条，维度来源列多值）

## 3. 分维度明细
### 3.1 high-impact-bug-audit（经 deep-scan）
### 3.2 logic_analyzer（经 deep-scan）
### 3.3 security_review（经 deep-scan）
### 3.4 external-input-audit
### 3.5 api-audit

## 4. 待跟进（Suspicious / 需进一步确认）
## 5. 附录：各 skill 原始产出文件路径
```

### Step 5：交付

向用户交付：
1. 统一报告路径。
2. 一段摘要：调用 了哪些 skill、共多少发现、Top 高危项。
3. 各子 skill 原始产出路径（便于深入查阅）。

不直接改源码——修复由用户确认后另起任务。

## 约定

- **静态语言实现默认排除**：`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp`，除非用户明确要求包含。
- **证据要求**：每条发现可追溯到 `file:line` + 触发路径，不收无证据代码气味项。
- **去重**：跨维度去重以 `file:line` 为键；同位置多维度命中合并为一条，标注全部维度来源。
- **让位**：用户指定单一维度（纯安全/纯 API/纯外部输入/纯逻辑）时，不触发本 skill，直接路由到对应子 skill。
