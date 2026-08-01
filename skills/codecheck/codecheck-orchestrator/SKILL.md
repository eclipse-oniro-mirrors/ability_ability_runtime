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
| `high-impact-bug-audit` / `logic_analyzer` / `security-review` | **不单独调**——已被 `deep-scan` 包含，避免重复 |

原则：**不与 deep-scan 重复调度其已含的三个维度**。本 skill 只在 deep-scan 之上**补** external-input-audit 和 api-audit，并做合并。

## 工作流

### Step 1：界定范围（缺则向用户确认）

必明确三项：
1. **目标**：路径（如 `services/abilitymgr/src/`）或 Kit 名（如 `abilityKit`）。
2. **检视重点**：未指定时默认"通用检视"。
3. **版本信息**：从 git 获取完整 commit-id（full SHA, 40 位十六进制）和 Change-Id，记入报告头部。

```bash
# 获取 commit-id（完整 40 位）
git rev-parse HEAD
# 或
git log -1 --format="%H"

# 获取 Change-Id
git log -1 --format="%b" | grep -o 'Change-Id: I[0-9a-f]\{40\}'

# 同时获取
echo "commit-id: $(git rev-parse HEAD)"
echo "Change-Id: $(git log -1 --format='%b' | grep -o 'Change-Id: I[0-9a-f]\{40\}')"

# 获取 commit message 第一行（subject）
git log -1 --format="%s"
```

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

### Step 3.5：补充检查（测试覆盖度 + 编码规范快速检视）

在子 skill 执行完毕后，对变更范围做两项补充检查，结果记入统一报告第 5.6（测试覆盖度）/5.7（编码规范）节。

#### 3.5A 测试覆盖度检查

对变更文件对应的测试目录，检查以下要点：

1. **修改点识别**：将 diff 分解为离散修改点（新增函数/修改分支/删除逻辑/接口变更）
2. **已有覆盖分析**：搜索 test 目录中是否已有用例覆盖被修改的函数
3. **新增覆盖检查**：本次 diff 中是否包含测试文件变更，新增用例的 `@tc.desc` 是否描述了覆盖场景
4. **完备度评价**：关键分支/边界条件/错误路径是否有直接覆盖；仅间接覆盖的路径标注为 🟡

判定标准：

| 覆盖程度 | 符号 | 判定 |
|---------|------|------|
| 直接覆盖 | 🟢 | 存在测试直接调用被修改函数并验证行为 |
| 间接覆盖 | 🟡 | 存在测试经过该路径（如通过上层接口间接调用），但无直接断言 |
| 未覆盖 | 🔴 | 无任何测试经过该代码路径 |

#### 3.5B 编码规范快速检视

对变更文件做以下快速检查，不需要逐行扫描，聚焦高频违规模式：

| 检查项 | 检查方法 | 严重等级 |
|--------|---------|---------|
| **BUILD.gn PAC 防护** | 检查 `ohos_shared_library` 等目标是否含 `branch_protector_ret = "pac_ret"` | 🔴 致命 |
| **reinterpret_cast / const_cast** | grep 搜索变更文件中是否新增不安全的类型转换 | 🟠 严重 |
| **裸指针 → sptr 隐式转换** | 搜索 `sptr<` 附近是否有裸指针未经过 `new` 直接赋值 | 🟠 严重 |
| **函数式宏** | 搜索 `#define` 中是否含函数式宏（日志宏除外） | 🟡 警告 |
| **空函数体风格** | `if/for/while` 是否缺少大括号 | 🟡 警告 |
| **日志红线** | 搜索日志中是否含 `challenge`/`token`/`cmdLine`/文件路径等敏感信息 | 🔴 致命 |

### Step 4：合并为统一报告

汇总到 `codecheck_report_<scope>_<YYYYMMDD>.md`（`<scope>` 用路径简写或 Kit 名）。

**必须使用权威模板**：`skills/codecheck/codecheck_report_TEMPLATE.md`。它是唯一格式基准，保证所有统一报告结构一致、可供门禁脚本解析。模板固定了：
- 头部 **YAML 报告元数据块**（机器可读，字段名/取值域固定，`risk_level` 仅 `low|medium|high|unknown`，`gate_decision` 仅 `approve|conditional|block|insufficient`）。
- **固定章节**：1 基本信息 → 2 总体评价（整体评分 / 风险等级 / 上库决策 + 决策依据 + 扣分明细）→ 3 问题统计 → 4 高优先级发现（P0/P1）→ 5 分维度明细 → 6 待跟进 → 7 附录 → 附录 A 门禁规则。
- **附录 A 评分与决策矩阵**：`评分 = max(0, 100 − (30×P0 + 12×P1 + 5×P2 + 2×P3))`；存在 P0 → block，存在 P1 → conditional，评分 ≥90 → approve，≥70 → conditional，否则 block；必检维度缺失 → insufficient。严重等级统一归一化为 P0–P3。

执行合并时：
1. 跨维度去重以 `file:line` 为键；同位置多维度命中合并为一条，标注全部维度来源。
2. 按附录 A 计算 `score` / `risk_level` / `gate_decision`，同时写入 YAML 元数据块与 2.1 表格（两者必须一致）。
3. 必检维度缺失时，决策为 `insufficient` 并在 `waived_dimensions` 记录用户豁免项。

```markdown
# 代码检视报告 — <scope>（Round <N> / 最新提交）

~~~yaml   ← 报告元数据块（机器可读，字段按模板）
codecheck_report: { schema_version, scope, round, commit_id, change_id,
                    commit_subject, date, dimensions_required,
                    dimensions_executed, waived_dimensions, findings_total,
                    findings_by_severity, score, risk_level,
                    gate_decision, gate_blockers, must_fix, followups }
~~~

## 1. 基本信息
## 2. 总体评价        ← 整体评分 / 风险等级 / 上库决策 + 决策依据 + 扣分明细
## 3. 问题统计        ← 按维度 × P0/P1/P2/P3
## 4. 高优先级发现（P0/P1，跨维度去重后）
## 5. 分维度明细       ← 5.1 high-impact-bug-audit（经 deep-scan）
                        5.2 logic_analyzer（经 deep-scan）
                        5.3 security-review（经 deep-scan）
                        5.4 external-input-audit
                        5.5 api-audit
                        5.6 测试覆盖度  5.7 编码规范
## 6. 待跟进（P2/P3 + Suspicious）
## 7. 附录            ← 变更文件清单 / 检视轨迹 / 原始产出
## 附录 A             ← 评分与门禁规则（权威定义，勿改）
```

完整字段与填写说明见 [`skills/codecheck/codecheck_report_TEMPLATE.md`](../codecheck_report_TEMPLATE.md)。

### Step 5：交付

向用户交付：
1. 统一报告路径。
2. Executive Summary 摘要：各维度通过率、总分、上库决策。
3. Top 高危项（P0 问题）及其阻塞原因。
4. 各子 skill 原始产出路径（便于深入查阅）。

不直接改源码——修复由用户确认后另起任务。

## 约定

- **静态语言实现默认排除**：`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp`，除非用户明确要求包含。
- **证据要求**：每条发现可追溯到 `file:line` + 触发路径，不收无证据代码气味项。
- **去重**：跨维度去重以 `file:line` 为键；同位置多维度命中合并为一条，标注全部维度来源。
- **让位**：用户指定单一维度（纯安全/纯 API/纯外部输入/纯逻辑）时，不触发本 skill，直接路由到对应子 skill。
