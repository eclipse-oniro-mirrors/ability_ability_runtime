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

在子 skill 执行完毕后，对变更范围做两项补充检查，结果记入统一报告第 3.6/3.7 节。

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

汇总到 `codecheck_report_<scope>_<YYYYMMDD>.md`（`<scope>` 用路径简写或 Kit 名）。使用以下增强模板：

```markdown
# 代码检视报告 — <scope>

> 范围：<路径/Kit>   日期：<YYYY-MM-DD>   检视维度：<调用的 skill 列表>
> commit-id: <40 位完整 SHA>   Change-Id: <I 开头 40 位十六进制>

---

## 📋 执行摘要 (Executive Summary)

### 基本信息
| 项目 | 值 |
|------|-----|
| 检视范围 | <路径/Kit> |
| commit-id | <40 位完整 SHA> |
| Change-Id | <I 开头 40 位十六进制> |
| 日期 | <YYYY-MM-DD> |
| 检视维度 | <调用的 skill 列表> |
| commit message | <提交信息第一行> |

### ✅ 检查项目清单

每个维度按检查项逐条标记状态：✅ = 通过 ｜ ❌ = 有问题（附问题ID） ｜ ⚠️ = 需要注意

#### 🔐 Security & Bug (deep-scan)
- ✅ 内存安全: <无问题>
- ❌ 反序列化: <问题简述>（S-<ID>）
- ⚠️ 并发安全: <检查未覆盖场景>
**小计**: ✅ <通过数>/<总数> ｜ ❌ <问题数>

#### 🔍 Logic Analyzer (deep-scan)
- ❌ 控制流: <问题简述>（L-<ID>）
- ✅ 状态机: <无问题>
- ⚠️ 错误处理: <部分路径未覆盖>
**小计**: ✅ <通过数>/<总数> ｜ ❌ <问题数>

#### 📡 External Input Audit
- ❌ 持久化链路: <问题简述>（E-<ID>）
- ✅ 读取侧防护: <无问题>
**小计**: ✅ <通过数>/<总数> ｜ ❌ <问题数>

#### 🧪 测试覆盖度
- 🔴 未覆盖: <数量> 个修改点
- 🟢 已覆盖: <数量> 个修改点
- ⚠️ 仅间接覆盖: <数量> 个修改点
**小计**: 🟢 <直接覆盖数> 🟡 <间接覆盖数> 🔴 <未覆盖数>

#### 📐 编码规范
- ✅ PAC 防护: <无问题>
- ❌ 不安全类型转换: <问题简述>（F-<ID>）
- ⚠️ 函数式宏: <数量> 处
**小计**: ⚠️ <违规数> 处发现问题

---

### 🎯 总体评价

| 维度 | 通过率 | 等级 | 评价 |
|------|--------|------|------|
| Security & Bug | <百分比>% (<通过>/<总数>) | 🟢 良好 / 🟡 及格 / 🔴 差 | <评价> |
| Logic Analyzer | <百分比>% (<通过>/<总数>) | 🟢 良好 / 🟡 及格 / 🔴 差 | <评价> |
| External Input | <百分比>% (<通过>/<总数>) | 🟢 良好 / 🟡 及格 / 🔴 差 | <评价> |
| 测试覆盖度 | <百分比>% (<通过>/<总数>) | 🟢 良好 / 🟡 及格 / 🔴 差 | <评价> |
| 编码规范 | <百分比>% (<通过>/<总数>) | 🟢 良好 / 🟡 及格 / 🔴 差 | <评价> |

**整体评分**: <分数>/100（≥90 🟢优秀 / ≥75 🟢良好 / ≥60 🟡及格 / <60 🔴差）
**风险等级**: 🔴 **高风险** / 🟠 **中风险** / 🟡 **低风险** / 🟢 **安全**
**上库决策**: ❌ **不建议上库** / ⚠️ **修复 P0/P1 后上库** / ✅ **可以上库**

**阻塞原因**:
- <原因 1>
- <原因 2>

---

### 📊 问题统计

| 维度 | 总数 | 🔴 致命 | 🟠 严重 | 🟡 警告 |
|------|------|---------|---------|---------|
| Security & Bug | <N> | <N> | <N> | <N> |
| Logic Analyzer | <N> | <N> | <N> | <N> |
| External Input | <N> | <N> | <N> | <N> |
| 测试覆盖度 | <N> | <N> | <N> | <N> |
| 编码规范 | <N> | <N> | <N> | <N> |
| **总计** | **<N>** | **<N>** | **<N>** | **<N>** |

---

## 🔴 高优先级发现（P0/P1，跨维度去重后）

### <ID> 🔴 <标题>（P0）
- **维度来源**：<skill 列表>
- **位置**：`<file>:<line>`
- **触发路径**：<给定输入 X 通过入口 Y，经路径 Z，造成结果 R>
- **影响**：<影响描述>
- **建议**：<修复方案>
- **证据**：<代码引用>
- **等级**：🔴 致命

（同一 file:line 被多 skill 命中 → 合并为一条，维度标注多值）

---

## 📑 分维度明细

### 3.1 🔐 Security & Bug (high-impact-bug-audit)
| ID | 位置 | 类型 | 概述 | 证据 | 等级 |
|----|------|------|------|------|------|
| S-01 | `file.cpp:123` | 🔴 内存安全 | 空指针解引用 | ... | 🔴 |
| S-02 | `file.cpp:456` | 🟠 输入验证 | 外部数据未校验 | ... | 🟠 |

### 3.2 🔍 Logic Analyzer
| ID | 位置 | 问题 | 影响 | 等级 |
|----|------|------|------|------|
| L-01 | `file.cpp:789` | 状态机不完整 | 缺少 SUSPENDED 处理 | 🟠 |

### 3.3 🔒 Security Review
（与 S/Bug 表共用，此处仅补充 deep-scan 中 security 层的独立发现）

### 3.4 📡 External Input Audit
| ID | 链路 | 外部输入源 | 持久化目标 | 校验状态 | 攻击面 | 等级 |
|----|------|------------|------------|----------|--------|------|
| E-01 | CHAIN-001 | IPC Parcel | RDB | ❌ 无校验 | SQL注入 | P0 |

### 3.5 📖 API Audit
| ID | API | 框架 | 服务 | 测试 | 等级 |
|----|-----|------|------|------|------|
| A-01 | startAbility | ✅一致 | ❌行为不符 | 🟡间接覆盖 | 🟠 |

### 3.6 🧪 测试覆盖度

| 修改点 | 文件 | 已有覆盖 | 新增用例 | 完备度 | 等级 |
|--------|------|----------|----------|--------|------|
| Init() BOPD 分支 | `file.cpp:103` | 🔴 无 | ❌ 未补充 | ⚠️ 仅主路径 | 🔴 |
| QueryData() | `file.cpp:130` | 🟢 有 | ✅ 已补充 | 🟢 关键分支+边界 | 🟢 |

### 3.7 📐 编码规范

| ID | 位置 | 违规类型 | 问题 | 等级 |
|----|------|----------|------|------|
| F-01 | `BUILD.gn:15` | 🔴 PAC 缺失 | `ohos_shared_library` 缺 `branch_protector_ret` | 🔴 |
| F-02 | `foo.cpp:42` | 🟠 类型转换 | `reinterpret_cast` 不安全使用 | 🟠 |

---

## ⏳ 待跟进（Suspicious / 需进一步确认）

| # | 发现问题 | 触发路径 | 需要行动 |
|---|----------|----------|---------|
| 1 | 竞态条件疑似 | 触发路径未完全确认 | 补充调用链分析 |

## 📎 附录

### 变更文件清单
| 文件 | 状态 | 新增 | 删除 |
|------|------|------|------|
| `services/abilitymgr/src/ability_manager_service.cpp` | ✏️ 修改 | N | M |
| `test/unittest/modular_object_rdb_data_mgr_test/BUILD.gn` | ➕ 新增 | N | 0 |

### 各 skill 原始产出路径
| Skill | 产出文件 |
|-------|---------|
| deep-scan | `services_abilitymgr_deep_scan_issues.xlsx` |
| external-input-audit | `services_abilitymgr_external_input_audit.xlsx` |

```

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
