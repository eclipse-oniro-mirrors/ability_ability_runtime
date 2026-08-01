# codecheck — 代码检视技能集

本目录是 ability_runtime 仓库的**代码检视工作台**。当用户说"检视一下代码""帮我审一下""做一次代码 review"时，从这里出发：先读本 README 选定要调用的子 skill，逐个执行扫描，最后把各 skill 的产出合并为一份统一报告。

> 入口指引：`AGENTS.md` 的「代码检视」章节把"检视代码"这一意图路由到本 README。本 README 只负责**导航与编排**，每个子 skill 的具体方法论在其各自目录的 `SKILL.md` 中。

---

## 子 skill 一览

| 目录 | 检视维度 | 触发场景 | 输出 |
|------|---------|---------|------|
| [`high-impact-bug-audit/`](high-impact-bug-audit/SKILL.md) | 高影响缺陷：崩溃、挂死、OOM、UAF、死锁、数据损坏、资源泄漏、状态污染、权限绕过 | "查高危 bug""P0/P1 风险排查""崩溃/挂死审计" | 按 `影响×可触发性×波及面` 排序的缺陷清单（Confirmed/Likely/Suspicious 分级） |
| [`logic_analyzer/`](logic_analyzer/SKILL.md) | 逻辑影响：修改的波及路径、逻辑一致性、状态机转换、边界条件、错误处理 | "逻辑分析""这段改动会影响什么""状态机/数据流/控制流检查" | 逻辑影响范围 + 不一致/边界遗漏清单 |
| [`security-review/`](security-review/SKILL.md) | 商用前安全审查：内存安全、注入、权限、敏感数据、并发、合规 | "安全审查""漏洞扫描""安全审计""商用前 review" | 详尽 `report.md`，按漏洞类型分组 |
| [`external-input-audit/`](external-input-audit/SKILL.md) | "外部输入 → 持久化"全链路健壮性：IPC/HTTP/CLI/配置/网络 → DB/文件/缓存/日志 | "外部输入排查""输入接口审计""持久化安全检查""接口健壮性" | 按 P0/P1/P2 排序的 Excel 风险清单（写入侧+读取侧双维度） |
| [`api-audit/`](api-audit/SKILL.md) | 对外 API 全量一致性：资料文档×接口定义×框架实现×测试用例完备度，三轮扫描 | "接口审计""API 一致性""测试用例完备度""扫一下 xxxKit" | `<kit>_api_audit.md` + `<kit>_api_audit.csv` 双格式 |
| [`deep-scan/`](deep-scan/SKILL.md) | **三层编排器**：high-impact-bug-audit → logic_analyzer → security-review | `/deep-scan {path}` 或"对某某路径做深度扫描/全面排查" | 按 P0/P1/P2 排序的 Excel 问题汇总 |
| [`codecheck-orchestrator/`](codecheck-orchestrator/SKILL.md) | **总编排器**：按范围选 skill 组合（deep-scan + external-input-audit + api-audit），执行后跨维度去重合并为统一报告 | "检视一下代码""帮我审一下""做一次 code review""生成检视报告" | `codecheck_report_<scope>_<YYYYMMDD>.md` 统一报告 |

### 维度关系图

```
                 用户："检视一下代码"
                          │
                          ▼
              ┌─────────────────────────┐
              │  codecheck-orchestrator  │  总编排：选组合 + 合并报告
              └────────────┬─────────────┘
                           │ 调度（按范围选）
        ┌──────────────────┼──────────────────┐
        ▼                  ▼                  ▼
  ┌───────────┐   ┌──────────────────┐   ┌───────────┐
  │ deep-scan │   │external-input-   │   │ api-audit │
  │ (三层串联) │   │audit             │   │           │
  └─────┬─────┘   └──────────────────┘   └───────────┘
        │ 内含三层
   ┌────┴────┬────────────┬──────────────┐
   ▼         ▼            ▼              
high-impact  logic_      security-       
-bug-audit   analyzer    review          
```

> 三层关系：
> - `codecheck-orchestrator` 是**总编排器**，按范围调度 deep-scan + external-input-audit + api-audit，并合并统一报告。通用"检视代码"的默认入口。
> - `deep-scan` 是**三层编排器**，只串联 bug→logic→security，不纳入 external-input-audit / api-audit。
> - 单一维度（纯安全/纯 API/纯外部输入/纯逻辑）时直接调对应子 skill，不走 orchestrator。

---

## 检视流程（"检视一下代码"时怎么走）

### Step 1：界定范围
明确两点，缺则向用户确认：
1. **目标路径或 Kit**：如 `services/abilitymgr/src/`、`frameworks/native/ability/native/`、或 `abilityKit`。
2. **检视重点**：通用 review / 安全 / 高危 bug / API 兼容 / 外部输入健壮性。未指定时走"通用检视"。

### Step 2：按场景选择 skill 组合

| 场景 | 调用组合 |
|------|---------|
| 通用代码检视（最常见） | `codecheck-orchestrator`（按范围自动选 deep-scan ± external-input-audit ± api-audit，并合并报告） |
| 安全/商用前专项 | `security-review` 单独深入 + `external-input-audit` |
| 接口/SDK 变更 | `api-audit {kit}` + `deep-scan` 覆盖实现侧 |
| 服务侧健壮性（IPC/持久化密集） | `deep-scan` + `external-input-audit` |
| 仅逻辑变更影响评估 | `logic_analyzer` 单独 |

### Step 3：逐个执行
读对应目录的 `SKILL.md`，按其工作流执行。每个 skill 的输出格式不一致（md/csv/excel），**保留各 skill 原始产出**，不要在中间改写。

### Step 4：合并为统一报告
把各 skill 的产出汇总到一份 `codecheck_report_<scope>_<YYYYMMDD>.md`。**统一报告用于门禁管控，必须严格套用权威模板**：

> 📄 **权威模板：** [`codecheck_report_TEMPLATE.md`](codecheck_report_TEMPLATE.md)
> 模板固定了章节顺序（基本信息 → 总体评价 → 问题统计 → 高优先级发现 → 分维度明细 → 待跟进 → 附录）、头部机器可读的 YAML 报告元数据块，以及文末"附录 A"的**评分公式与门禁决策矩阵**。所有 codecheck 报告（编排、deep-scan、单维度）合并后格式保持一致，便于门禁脚本解析与历史对比。

生成要求（门禁校验项）：
1. 头部 YAML 报告元数据块字段名/取值域固定，`score`/`risk_level`/`gate_decision` 必须按附录 A 的公式与决策矩阵计算，不得自创分值。
2. 严重等级统一归一化为 P0–P3（各 skill 的 critical/high/medium/low、致命/严重/一般/提示 一律映射后汇总）。
3. 跨维度去重：同一 `file:line` 被多个 skill 命中时在"高优先级发现"合并为一条，标注维度来源列表。
4. 必检维度缺失时，门禁决策为 `insufficient`（无法评估），需补齐扫描后重出报告。

简略结构（完整字段见模板）：

```
# 代码检视报告 — <scope>（Round <N> / 最新提交）

~~~yaml   ← 报告元数据块（机器可读，固定字段）
codecheck_report: { schema_version, scope, round, commit_id, change_id,
                    commit_subject, date, dimensions_required,
                    dimensions_executed, waived_dimensions, findings_total,
                    findings_by_severity, score, risk_level,
                    gate_decision, gate_blockers, must_fix, followups }
~~~

## 1. 基本信息
## 2. 总体评价     ← 整体评分 / 风险等级 / 上库决策 + 决策依据 + 扣分明细
## 3. 问题统计     ← 按维度 × P0/P1/P2/P3 汇总
## 4. 高优先级发现（P0/P1，跨维度去重）
## 5. 分维度明细   ← 5.1..5.n 对应实际执行维度
## 6. 待跟进（P2/P3 + Suspicious）
## 7. 附录         ← 变更文件清单 / 检视轨迹 / 原始产出
## 附录 A          ← 评分公式与门禁决策矩阵（权威定义）
```

---

## 约定

- **静态语言实现默认排除**：`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp` 等 static 侧不在扫描范围（与 `api-audit` 一致），除非用户明确要求包含。
- **证据要求**：每条发现必须可追溯到 `file:line` + 触发路径，不收"代码气味"级别的无证据项。
- **不动代码**：检视阶段只产出报告与建议，不直接改源码；修复由用户确认后另起任务。
