# codecheck 跨模块公约

> 将散落在多个 SKILL.md 中的共同约定集中管理。所有 scanner 和 orchestrator 必须遵守。
>
> **Path B 统一合约**：单 scanner 直接被调用时不再只产原始发现清单，必须额外按本规程 §5–§13 生成符合 [`codecheck_report_TEMPLATE.md`](codecheck_report_TEMPLATE.md) 的统一报告。本文件从"摘要"升级为**操作规程**，各 scanner 只需声明"遵循本规程"。

---

## 1. 扫描范围

- **静态语言实现默认排除**：`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp` 等 static 侧不在扫描范围（与 `api-scanner` 一致），除非用户明确要求包含。
- **包含**：`.cpp`、`.cc`、`.h`、`.hpp`、`.js`、`.ts` 等源文件。
- **api-scanner 额外规则**：JS API 按目标语言分两套——static（静态语言接口，ArkTS/TS）直接跳过；dynamic（动态语言接口，JS）必须扫描。判定口诀：`js_` 前缀 + NAPI 签名（`napi_env`/`napi_callback_info`）= 动态（扫）；`ets_`/`cj_` 前缀或位于 `ets/`、`cj/` 目录 = 静态（跳过）。

---

## 2. 证据要求

- 每条发现必须可追溯到 `file:line` + 触发路径。
- 不收"代码气味"级别的无证据项。
- 引用代码使用格式：`path/to/file.cpp:123`。
- 无证据项不得计入 P0/P1。

---

## 3. 不动代码

- 检视阶段只产出报告与建议，不直接改源码。
- 修复由用户确认后另起任务，并回到 AGENTS.md 的 Constraints/Verification 章节执行。

---

## 4. 去重规则

- 跨 scanner 去重以 `file:line` 为第一键。
- 同 `file:line` 多 scanner 命中 → 合并为一条，标注维度来源列表。
- 不同 `file:line` 但同根因 → 由 refuter 在 orchestrator Step 5 合并（基于修复方案反推）。
- **单 scanner 自去重**：同一 `file:line` 被同一 scanner 多条规则命中时合并为一条，标注全部命中的规则/Pattern 编号；单 scanner 报告第 4 节"高优先级发现"需做此去重。

---

## 5. 单 scanner 报告生成义务（统领）

> 本节是 Path B 的核心合约：scanner 直接被调用时不再只产原始发现清单，必须额外生成统一报告。

### 5.1 触发条件

- scanner **未经 orchestrator 直接被用户调用**（如"做安全审查"→ security-scanner，"扫一下 abilityKit"→ api-scanner）
- 经 orchestrator 调度时，scanner 只产原始发现清单，统一报告由 orchestrator 合并生成（本节不适用）

### 5.2 双产出义务

单 scanner 直接被调用时，必须产出两份：

| 产出 | 格式 | 用途 |
|------|------|------|
| 原始发现清单 | scanner 自有格式（Excel / md / csv，见各 SKILL.md） | 供深入查阅、供 orchestrator 合并 |
| 统一报告 | `codecheck_report_<scope>_<YYYYMMDD>.md`，遵循 `codecheck_report_TEMPLATE.md` | 门禁管控、跨报告对比 |

### 5.3 生成规程

统一报告按 §6–§13 规程生成，执行顺序：

1. §6 采集版本信息
2. §7 严重等级归一化到 P0–P3
3. §8 必检维度自检（判定是否 insufficient）
4. §9 评分与门禁计算
5. §10 填充 gate_blockers/must_fix/followups
6. §11 填写 YAML 元数据块（17 字段）
7. §12 按 7 章节固定顺序组装报告
8. §13 P0 项 refute（仅 P0）

### 5.4 各 scanner SKILL.md 声明

每个 scanner SKILL.md 顶部需声明："本 scanner 直接被调用时，除产出下述原始发现清单外，必须按 [`conventions.md`](../../conventions.md) §5–§13 规程生成统一报告，其中 P0 项必须做 refute（见 §13.2）。"

---

## 6. 版本信息采集

```bash
# commit_id（完整 40 位）
git rev-parse HEAD
# change_id（I 开头 40 位十六进制）
git log -1 --format="%b" | grep -o 'Change-Id: I[0-9a-f]\{40\}'
# commit_subject（第一行）
git log -1 --format="%s"
```

- 非 git 仓库或无 Change-Id：对应字段填 `"N/A"`，并在报告"1. 基本信息"节注明。
- 日期 `date` 取当日 `YYYY-MM-DD`。
- `round`：单 scanner 首次执行 = 1；同范围重检递增。

---

## 7. 严重等级归一化操作规程

> 各 scanner 内部可保留自有等级做检查，但**进入统一报告前必须归一化到 P0–P3**。归一化映射以**问题实际影响**为准，不以 scanner 内部措辞为准；同一问题被多规则标注不同等级时取最高。

### 7.1 统一等级

| 统一等级 | 含义 | 门禁含义 |
|---------|------|---------|
| **P0 致命** | 崩溃/UAF/OOM/死锁/权限绕过/数据损坏/敏感数据泄漏等必现或易触发 | **阻断上库** |
| **P1 严重** | 影响正确性/安全边界，低概率触发或需组合条件 | 需修复或人工裁决 |
| **P2 一般** | 逻辑缺陷/资源小泄漏/健壮性问题 | 建议修复，登记跟进 |
| **P3 提示** | 风格/潜在风险/观察项，当前不可达 | 不阻塞，登记跟进 |

### 7.2 各 scanner 映射表

| scanner | 内部等级 | → P0 | → P1 | → P2 | → P3 |
|---------|---------|------|------|------|------|
| security-scanner | 9-10/7-8/5-6/3-4/1-2 五档 | 9-10 | 7-8 | 5-6 | 1-4 |
| logic-scanner | 🔴/🟠/🟡/🟢 emoji | 🔴 | 🟠 | 🟡 | 🟢 |
| input-scanner | P0/P1/P2/P3（已对齐） | P0 | P1 | P2 | P3（恒等映射） |
| api-scanner | ❌/⚠️/✅ 三态 | ❌（含 E4 描述与实现不符、安全绕过） | ❌（其他：E1/E2/E3 致调用失败、版本/错误码不一致） | ⚠️（E5/E6/E7 不导致误用） | ✅ 不报告 |

> **api-scanner P3 补定义**：原无 P3 概念。新增"资料冗余/过时（E5）且不导致误用"作为观察项——`✅` 不报告，如需记录可在第 6 节"待跟进"列出但不计入 `findings_total`。

### 7.3 操作要求

- 每条发现进入统一报告时，YAML `findings_by_severity` 与第 3 节"问题统计"表必须用归一化后的等级。
- 各 scanner 自有原始产出可保留内部等级（如 Excel"风险等级"列、api md 的 ❌/⚠️），但统一报告第 4/5 节每条发现的"严重等级"列必须填 P0–P3。

---

## 8. 必检维度自检

> 单 scanner 必须先判定 `dimensions_required`，决定是否 `insufficient`。此步骤决定 `gate_decision` 是否直接判 `insufficient`，优先级高于评分。

### 8.1 路径特征 → 必检维度表

| 路径特征 | dimensions_required |
|---------|---------------------|
| 通用路径/提交 | `security-scanner` + `logic-scanner` |
| `services/` 下、IPC/DB/文件/配置密集区 | `security-scanner` + `logic-scanner` + `input-scanner` |
| `interfaces/kits/`、NAPI/ANI/C 绑定、指定 Kit | `api-scanner` + `security-scanner` + `logic-scanner` |

路径特征探测信号（同 orchestrator Step 2）：

```bash
# IPC 信号
rg "Parcel::Read|OnRemoteRequest|WriteRemoteObject" --type cpp <path>
# 持久化信号
rg "Insert\s*\(|ExecuteSql|fopen|write\s*\(" --type cpp <path>
# API 信号：检查 interfaces/kits/ 或 frameworks/js/napi/ 下是否有 .d.ts/.h
ls <path>/interfaces/kits/ 2>/dev/null
ls <path>/frameworks/js/napi/ 2>/dev/null
```

### 8.2 自检逻辑

- `dimensions_executed` = {自身维度名}
- 计算差集 `missing = dimensions_required − dimensions_executed`
- 若 `missing ≠ ∅` 且 `missing` 未全部在 `waived_dimensions` 中 → `gate_decision = insufficient`、`risk_level = unknown`
- 若 `missing = ∅` 或 `missing` 全部已豁免 → 按第 9 节评分矩阵正常判定
- **必须向用户提示**未执行的必检维度，询问是否豁免；用户不豁免则出 `insufficient` 报告并说明补齐方式（哪些维度需补扫）
- `waived_dimensions` 只记录用户明确书面豁免的维度，scanner 不得自行豁免

---

## 9. 评分与门禁计算规程

### 9.1 评分公式

```
score = max(0, 100 − (30×P0 + 12×P1 + 5×P2 + 2×P3))
```

P0/P1/P2/P3 为第 7 节归一化后的计数。

### 9.2 决策矩阵（自上而下判定，命中即止）

| 序 | 条件 | risk_level | gate_decision |
|----|------|-----------|---------------|
| 0 | 必检维度未执行且未豁免（见 §8） | `unknown` | `insufficient` |
| 1 | 存在 ≥1 项 P0 | `high` | `block` |
| 2 | 存在 ≥1 项 P1（无 P0） | `medium` | `conditional` |
| 3 | score ≥ 90 | `low` | `approve` |
| 4 | score ≥ 70 | `medium` | `conditional` |
| 5 | score < 70 | `high` | `block` |

### 9.3 操作步骤

1. 统计归一化后的 P0/P1/P2/P3 计数
2. 计算 `score`
3. 按决策矩阵自上而下判定 `risk_level` + `gate_decision`（`insufficient` 优先级最高，即便有 P0 也判 `insufficient`）
4. YAML 块（§11）与第 2.1 节表格的 `score`/`risk_level`/`gate_decision` 三值必须一致

### 9.4 决策语义

- `approve`：可上库，P2/P3 登记 `followups` 跟踪
- `conditional`：可上库但附条件——必须处理所有 `must_fix`（P1 项）或门禁复核人书面裁决后放行
- `block`：禁止上库，必须修复 `gate_blockers`（P0 项）后进入下一轮重检
- `insufficient`：无法评估——必检维度缺失，补齐扫描后重出报告

---

## 10. gate 字段填充规则

| 字段 | 填充规则 |
|------|---------|
| `gate_blockers` | 所有 P0 项 ID 列表；`gate_decision = block` 时非空，否则 `[]` |
| `must_fix` | 所有 P1 项 ID 列表；仅 `gate_decision = conditional` 时非空，否则 `[]` |
| `followups` | 所有 P2/P3 项 ID 列表；始终填，允许 `[]` |

**ID 编号约定**（单 scanner 场景）：

| scanner | ID 前缀 | 示例 |
|---------|---------|------|
| security-scanner | `SEC-` | `SEC-01` |
| logic-scanner | `LOG-` | `LOG-01` |
| input-scanner | `INP-` | `INP-01` |
| api-scanner | `API-` | `API-01` |

经 orchestrator 合并时，各 scanner 原始 ID 保留，最终报告第 4 节合并项标注全部来源 ID。

---

## 11. YAML 元数据块填写规则

> 门禁脚本只读取此 YAML 块。字段名与取值域为固定合约，禁止改名、增删或自定义取值。

```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "<路径简写或 Kit 名，与标题一致>"
  round: <正整数>
  commit_id: "<40 位完整 SHA，来自 §6>"
  change_id: "<Change-Id，I 开头 40 位十六进制，来自 §6>"
  commit_subject: "<commit message 第一行，来自 §6>"
  date: "<YYYY-MM-DD>"
  dimensions_required: ["<§8 自检得到的必检维度>"]
  dimensions_executed: ["<单 scanner 场景：仅自身维度>"]
  waived_dimensions: ["<用户明确书面豁免的必检维度，无则 []>"]
  findings_total: <归一化后发现总数>
  findings_by_severity: {P0: <n>, P1: <n>, P2: <n>, P3: <n>}
  score: <§9 计算结果，0-100>
  risk_level: "<low|medium|high|unknown，来自 §9>"
  gate_decision: "<approve|conditional|block|insufficient，来自 §9>"
  gate_blockers: ["<P0 项 ID / 缺失的必检维度，来自 §10>"]
  must_fix: ["<conditional 时填 P1 项 ID，来自 §10>"]
  followups: ["<P2/P3 项 ID，来自 §10>"]
```

**取值域**（唯一合法值）：`risk_level ∈ {low, medium, high, unknown}`；`gate_decision ∈ {approve, conditional, block, insufficient}`。

---

## 12. 固定章节组装规则

> 章节顺序固定，门禁脚本做结构化校验。详见 [`codecheck_report_TEMPLATE.md`](codecheck_report_TEMPLATE.md)。

### 12.1 章节顺序

1. 基本信息（含提交内容核对）
2. 总体评价（2.1 上库质量评估结论 / 2.2 各维度通过率 / 2.3 评分扣分明细）
3. 问题统计（按维度 × P0–P3 交叉表）
4. 高优先级发现（P0/P1，跨维度去重后，单 scanner 即自去重后）
5. 分维度明细（单 scanner 只一个子节，即自身维度）
6. 待跟进（P2/P3 + Suspicious）
7. 附录（7.1 文件清单 / 7.2 检视轨迹 / 7.3 各 skill 原始产出）
- 附录 A：评分与门禁规则（从模板复制，不得改写）

### 12.2 各节填充要点（单 scanner 场景）

| 节 | 单 scanner 填充要点 |
|----|---------------------|
| 1 基本信息 | 检视维度填自身一项；提交内容核对照常 |
| 2.1 上库决策 | 三值与 YAML 一致；阻塞项填 P0 ID 或"无"；上库条件填 P1 处置要求或"无" |
| 2.2 各维度通过率 | 单行：自身维度 n/m |
| 2.3 评分扣分明细 | 按 P0–P3 计数填扣分 |
| 3 问题统计 | 单行：自身维度；总计行 |
| 4 高优先级发现 | 表头 9 列：ID/维度来源/位置/严重等级/概述/影响/触发路径/建议/状态；单 scanner 自去重 |
| 5 分维度明细 | 单子节 5.1，保留 scanner 原始结论（可精简字段，不可改判等级） |
| 6 待跟进 | P2/P3 + Suspicious 列表 |
| 7.1 文件清单 | 检视对象文件 + 状态 |
| 7.2 检视轨迹 | 单 scanner 首次为 Round 1 |
| 7.3 原始产出 | scanner 自有格式产出路径（Excel/md/csv） |
| 附录 A | 从模板原样复制 |

### 12.3 格式一致性要求（门禁校验项）

1. 章节编号与顺序固定
2. YAML 字段名/取值域合法；`score`/`risk_level`/`gate_decision` 三者与 2.1 表格一致
3. 每条发现可追溯到 `file:line` + 触发路径；无证据项不得计入 P0/P1
4. 严重等级只允许 P0–P3
5. 多轮重检保留 7.2 检视轨迹

---

## 13. Refute 公约

> 单 scanner 场景的 refute 义务见 §13.2（仅 P0）；orchestrator 场景见 §13.1（不变）。

### 13.1 经 orchestrator 调度时（不变）

- **只质疑，不发现**：refuter 不能新增任何 scanner 未报告的发现
- **质疑必须有代码证据**：推翻或降级必须引用具体 file:line
- **被推翻的发现保留在附录**：最终报告 §7 附录增加"已排除发现"小节
- **P0 推翻需更严格证据**：不能仅凭"极难触发"推翻 P0，必须是"确认不可达"
- **refute_log.md 与最终报告一并交付**
- 审查范围与三层质疑规则详见 [`orchestrator/refute-rules.md`](orchestrator/refute-rules.md)

### 13.2 单 scanner 直接产出时（Path B 新增）

- **跨 scanner 根因去重（第三层）跳过**：单 scanner 无跨维度去重需求
- **仅 P0 必须做 refute**：所有 P0 项必须经第一层（触发路径证伪）+ 第二层（影响夸大证伪）自审，产出 `refute_log.md` 与统一报告一并交付
- **P1/P2/P3 跳过 refute**：不强制自审，直接进入统一报告
- **P0 推翻需"确认不可达"级证据**：不能仅凭"极难触发"推翻 P0，必须是上游硬 guard 或编译器保证
- **被推翻的 P0 保留在附录 §7"已排除发现"小节**，便于人工复核捞回
- 用户可显式要求"P1 也做 refute"，此时按 §13.1 orchestrator 标准（P0/P1 全审）执行
- 单 scanner 的 `refute_log.md` 可精简：仅记录 P0 项审查结论（✅ 维持 / ⬇️ 降级 / ❌ 推翻），无需第三层去重
